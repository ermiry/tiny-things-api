static char *things_labels_handler_generate_json (
	User *user,
	mongoc_cursor_t *labels_cursor,
	size_t *json_len
) {

	char *retval = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_int32 (doc, "count", -1, user->labels_count);

		bson_t labels_array = { 0 };
		(void) bson_append_array_begin (doc, "labels", -1, &labels_array);
		char buf[16] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		int i = 0;
		const bson_t *label_doc = NULL;
		while (mongoc_cursor_next (labels_cursor, &label_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, sizeof (buf));
			(void) bson_append_document (&labels_array, key, (int) keylen, label_doc);

			bson_destroy ((bson_t *) label_doc);

			i++;
		}
		(void) bson_append_array_end (doc, &labels_array);

		retval = bson_as_relaxed_extended_json (doc, json_len);
	}

	return retval;

}

// GET api/things/labels
// get all the authenticated user's labels
void things_labels_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		// get user's labels from the db
		if (!user_get_by_id (user, user->id, user_labels_query_opts)) {
			mongoc_cursor_t *labels_cursor = labels_get_all_by_user (
				&user->oid, label_no_user_query_opts
			);

			if (labels_cursor) {
				// convert them to json and send them back
				size_t json_len = 0;
				char *json = things_labels_handler_generate_json (
					user, labels_cursor, &json_len
				);

				if (json) {
					(void) http_response_json_custom_reference_send (
						cr,
						200,
						json, json_len
					);

					free (json);
				}

				else {
					(void) http_response_send (server_error, cr->cerver, cr->connection);
				}

				mongoc_cursor_destroy (labels_cursor);
			}

			else {
				(void) http_response_send (no_user_labels, cr->cerver, cr->connection);
			}
		}

		else {
			(void) http_response_send (bad_user, cr->cerver, cr->connection);
		}
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

static void things_label_parse_json (
	json_t *json_body,
	const char **title,
	const char **description,
	const char **color
) {

	// get values from json to create a new label
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "title")) {
				*title = json_string_value (value);
				(void) printf ("title: \"%s\"\n", *title);
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				(void) printf ("description: \"%s\"\n", *description);
			}

			else if (!strcmp (key, "color")) {
				*color = json_string_value (value);
				(void) printf ("color: \"%s\"\n", *color);
			}
		}
	}

}

static Label *things_label_create_handler_internal (
	const char *user_id, const String *request_body
) {

	Label *label = NULL;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;
		const char *color = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			things_label_parse_json (
				json_body,
				&title,
				&description,
				&color
			);

			label = things_label_create (
				user_id,
				title, description,
				color
			);

			json_decref (json_body);
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n", 
				error.line, error.text
			);
		}
	}

	return label;

}

// POST api/things/labels
// a user has requested to create a new label
void things_label_create_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Label *label = things_label_create_handler_internal (
			user->id, request->body
		);

		if (label) {
			#ifdef LABEL_DEBUG
			label_print (label);
			#endif

			if (!mongo_insert_one (
				labels_collection,
				label_to_bson (label)
			)) {
				// update users values
				(void) mongo_update_one (
					users_collection,
					user_query_id (user->id),
					user_create_update_things_labels ()
				);

				// return success to user
				(void) http_response_send (
					label_created_success,
					cr->cerver, cr->connection
				);
			}

			else {
				(void) http_response_send (
					label_created_bad,
					cr->cerver, cr->connection
				);
			}
			
			things_label_delete (label);
		}

		else {
			(void) http_response_send (
				label_created_bad,
				cr->cerver, cr->connection
			);
		}
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

// GET api/things/labels/:id
// returns information about an existing label that belongs to a user
void things_label_get_handler (CerverReceive *cr, HttpRequest *request) {

	const String *label_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		Label *label = (Label *) pool_pop (labels_pool);
		if (label) {
			bson_oid_init_from_string (&label->oid, label_id->str);
			bson_oid_init_from_string (&label->user_oid, user->id);

			const bson_t *label_bson = label_find_by_oid_and_user (
				&label->oid, &label->user_oid,
				label_no_user_query_opts
			);

			if (label_bson) {
				size_t json_len = 0;
				char *json = bson_as_relaxed_extended_json (label_bson, &json_len);
				if (json) {
					(void) http_response_json_custom_reference_send (
						cr, 200, json, json_len
					);

					free (json);
				}

				bson_destroy ((bson_t *) label_bson);
			}

			else {
				(void) http_response_send (no_user_label, cr->cerver, cr->connection);
			}

			things_label_delete (label);
		}
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

static u8 things_label_update_handler_internal (
	Label *label, const String *request_body
) {

	u8 retval = 1;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;
		const char *color = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			things_label_parse_json (
				json_body,
				&title,
				&description,
				&color
			);

			if (title) (void) strncpy (label->title, title, LABEL_TITLE_LEN);
			if (description) (void) strncpy (label->description, description, LABEL_DESCRIPTION_LEN);
			if (color) (void) strncpy (label->color, color, LABEL_COLOR_LEN);

			json_decref (json_body);

			retval = 0;
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n", 
				error.line, error.text
			);
		}
	}

	return retval;

}

// POST api/things/labels/:id
// a user wants to update an existing label
void things_label_update_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_init_from_string (&user->oid, user->id);

		Label *label = things_label_get_by_id_and_user (
			request->params[0], &user->oid
		);

		if (label) {
			// get update values
			if (!things_label_update_handler_internal (
				label, request->body
			)) {
				// update the label in the db
				if (!mongo_update_one (
					labels_collection,
					label_query_oid (&label->oid),
					label_update_bson (label)
				)) {
					(void) http_response_send (oki_doki, cr->cerver, cr->connection);
				}

				else {
					(void) http_response_send (server_error, cr->cerver, cr->connection);
				}
			}

			things_label_delete (label);
		}

		else {
			(void) http_response_send (bad_request, cr->cerver, cr->connection);
		}
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

// DELETE api/things/labels/:id
// deletes an existing user's label
void things_label_delete_handler (CerverReceive *cr, HttpRequest *request) {

	// TODO:

}