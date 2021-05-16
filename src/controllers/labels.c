#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/label.h"
#include "models/user.h"

#include "controllers/labels.h"

static Pool *labels_pool = NULL;

const bson_t *label_no_user_query_opts = NULL;
static CMongoSelect *label_no_user_select = NULL;

HttpResponse *no_user_labels = NULL;
HttpResponse *no_user_label = NULL;

HttpResponse *label_created_success = NULL;
HttpResponse *label_created_bad = NULL;
HttpResponse *label_deleted_success = NULL;
HttpResponse *label_deleted_bad = NULL;

void things_label_return (void *label_ptr);

static unsigned int things_labels_init_pool (void) {

	unsigned int retval = 1;

	labels_pool = pool_create (label_delete);
	if (labels_pool) {
		pool_set_create (labels_pool, label_new);
		pool_set_produce_if_empty (labels_pool, true);
		if (!pool_init (labels_pool, label_new, DEFAULT_LABELS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init labels pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create labels pool!");
	}

	return retval;

}

static unsigned int things_labels_init_query_opts (void) {

	unsigned int retval = 1;

	label_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (label_no_user_select, "title");
	(void) cmongo_select_insert_field (label_no_user_select, "description");
	(void) cmongo_select_insert_field (label_no_user_select, "color");
	(void) cmongo_select_insert_field (label_no_user_select, "date");

	label_no_user_query_opts = mongo_find_generate_opts (label_no_user_select);

	if (label_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int things_labels_init_responses (void) {

	unsigned int retval = 1;

	no_user_labels = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "Failed to get user's labels"
	);

	no_user_label = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "User's label was not found"
	);

	label_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	label_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create label!"
	);

	label_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	label_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete label!"
	);

	if (
		no_user_labels && no_user_label
		&& label_created_success && label_created_bad
		&& label_deleted_success && label_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int things_labels_init (void) {

	unsigned int errors = 0;

	errors |= things_labels_init_pool ();

	errors |= things_labels_init_query_opts ();

	errors |= things_labels_init_responses ();

	return errors;

}

void things_labels_end (void) {

	cmongo_select_delete (label_no_user_select);
	bson_destroy ((bson_t *) label_no_user_query_opts);

	http_response_delete (no_user_labels);
	http_response_delete (no_user_label);

	http_response_delete (label_created_success);
	http_response_delete (label_created_bad);
	http_response_delete (label_deleted_success);
	http_response_delete (label_deleted_bad);

	pool_delete (labels_pool);
	labels_pool = NULL;

}

Label *things_label_get (void) {

	return (Label *) pool_pop (labels_pool);

}

unsigned int things_labels_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
) {

	return labels_get_all_by_user_to_json (
		user_oid, label_no_user_query_opts,
		json, json_len
	);

}

Label *things_label_get_by_id_and_user (
	const String *label_id, const bson_oid_t *user_oid
) {

	Label *label = NULL;

	if (label_id) {
		label = (Label *) pool_pop (labels_pool);
		if (label) {
			bson_oid_init_from_string (&label->oid, label_id->str);

			if (label_get_by_oid_and_user (
				label,
				&label->oid, user_oid,
				NULL
			)) {
				things_label_return (label);
				label = NULL;
			}
		}
	}

	return label;

}

u8 things_label_get_by_id_and_user_to_json (
	const char *label_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (label_id) {
		bson_oid_t label_oid = { 0 };
		bson_oid_init_from_string (&label_oid, label_id);

		retval = label_get_by_oid_and_user_to_json (
			&label_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static Label *things_label_create_actual (
	const char *user_id,
	const char *title, const char *description,
	const char *color
) {

	Label *label = (Label *) pool_pop (labels_pool);
	if (label) {
		bson_oid_init (&label->oid, NULL);

		bson_oid_init_from_string (&label->user_oid, user_id);

		if (title) (void) strncpy (label->title, title, LABEL_TITLE_SIZE - 1);
		if (description) (void) strncpy (label->description, description, LABEL_DESCRIPTION_SIZE - 1);

		if (color) (void) strncpy (label->color, color, LABEL_COLOR_SIZE - 1);
		
		label->date = time (NULL);
	}

	return label;

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
				#ifdef THINGS_DEBUG
				(void) printf ("title: \"%s\"\n", *title);
				#endif
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("description: \"%s\"\n", *description);
				#endif
			}

			else if (!strcmp (key, "color")) {
				*color = json_string_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("color: \"%s\"\n", *color);
				#endif
			}
		}
	}

}

static ThingsError things_label_create_parse_json (
	Label **label,
	const char *user_id, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	const char *title = NULL;
	const char *description = NULL;
	const char *color = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		things_label_parse_json (
			json_body,
			&title,
			&description,
			&color
		);

		*label = things_label_create_actual (
			user_id,
			title, description,
			color
		);

		json_decref (json_body);
	}

	else {
		cerver_log_error (
			"json_loads () - json error on line %d: %s\n", 
			json_error.line, json_error.text
		);

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

ThingsError things_label_create (
	const User *user, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	if (request_body) {
		Label *label = NULL;

		error = things_label_create_parse_json (
			&label,
			user->id, request_body
		);

		if (error == THINGS_ERROR_NONE) {
			#ifdef THINGS_DEBUG
			label_print (label);
			#endif

			if (!label_insert_one (
				label
			)) {
				// update users values
				(void) user_add_label (user);
			}

			else {
				error = THINGS_ERROR_SERVER_ERROR;
			}
			
			things_label_return (label);
		}
	}

	else {
		#ifdef THINGS_DEBUG
		cerver_log_error ("Missing request body to create label!");
		#endif

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

static ThingsError things_label_update_parse_json (
	Label *label, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	const char *title = NULL;
	const char *description = NULL;
	const char *color = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		things_label_parse_json (
			json_body,
			&title,
			&description,
			&color
		);

		if (title) (void) strncpy (label->title, title, LABEL_TITLE_SIZE - 1);
		if (description) (void) strncpy (label->description, description, LABEL_DESCRIPTION_SIZE - 1);
		if (color) (void) strncpy (label->color, color, LABEL_COLOR_SIZE - 1);

		json_decref (json_body);
	}

	else {
		#ifdef THINGS_DEBUG
		cerver_log_error (
			"json_loads () - json error on line %d: %s\n", 
			json_error.line, json_error.text
		);
		#endif

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

ThingsError things_label_update (
	const User *user, const String *label_id,
	const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	if (request_body) {
		Label *label = things_label_get_by_id_and_user (
			label_id, &user->oid
		);

		if (label) {
			// get update values
			if (things_label_update_parse_json (
				label, request_body
			) == THINGS_ERROR_NONE) {
				// update the label in the db
				if (label_update_one (label)) {
					error = THINGS_ERROR_SERVER_ERROR;
				}
			}

			things_label_return (label);
		}

		else {
			#ifdef THINGS_DEBUG
			cerver_log_error ("Failed to get matching label!");
			#endif

			error = THINGS_ERROR_NOT_FOUND;
		}
	}

	else {
		#ifdef THINGS_DEBUG
		cerver_log_error ("Missing request body to update label!");
		#endif

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

// TODO: handle things that reference the requested label
ThingsError things_label_delete (
	const User *user, const String *label_id
) {

	ThingsError error = THINGS_ERROR_NONE;

	bson_oid_t oid = { 0 };
	bson_oid_init_from_string (&oid, label_id->str);

	if (!label_delete_one_by_oid_and_user (
		&oid, &user->oid
	)) {
		#ifdef THINGS_DEBUG
		cerver_log_debug ("Deleted label %s", label_id->str);
		#endif
	}

	else {
		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

void things_label_return (void *label_ptr) {

	(void) memset (label_ptr, 0, sizeof (Label));
	(void) pool_push (labels_pool, label_ptr);

}