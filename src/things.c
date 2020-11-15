#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "categories.h"
#include "labels.h"
#include "mongo.h"
#include "roles.h"
#include "things.h"
#include "users.h"
#include "version.h"

#include "models/action.h"
#include "models/category.h"
#include "models/label.h"
#include "models/role.h"
#include "models/thing.h"
#include "models/user.h"

const String *PORT = NULL;

static const String *MONGO_URI = NULL;
static const String *MONGO_APP_NAME = NULL;
static const String *MONGO_DB = NULL;

unsigned int CERVER_RECEIVE_BUFFER_SIZE = 4096;
unsigned int CERVER_TH_THREADS = 4;

HttpResponse *oki_doki = NULL;
HttpResponse *bad_request = NULL;
HttpResponse *server_error = NULL;
HttpResponse *bad_user = NULL;
HttpResponse *missing_values = NULL;

static HttpResponse *things_works = NULL;
static HttpResponse *current_version = NULL;

static HttpResponse *no_user_categories = NULL;
static HttpResponse *no_user_category = NULL;

static HttpResponse *category_created_success = NULL;
static HttpResponse *category_created_bad = NULL;
static HttpResponse *category_deleted_success = NULL;
static HttpResponse *category_deleted_bad = NULL;

#pragma region env

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static unsigned int things_env_get_port (void) {
	
	unsigned int retval = 1;

	char *port_env = getenv ("PORT");
	if (port_env) {
		PORT = str_new (port_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get PORT from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_app_name (void) {

	unsigned int retval = 1;

	char *mongo_app_name_env = getenv ("MONGO_APP_NAME");
	if (mongo_app_name_env) {
		MONGO_APP_NAME = str_new (mongo_app_name_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_APP_NAME from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_db (void) {

	unsigned int retval = 1;

	char *mongo_db_env = getenv ("MONGO_DB");
	if (mongo_db_env) {
		MONGO_DB = str_new (mongo_db_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_DB from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_uri (void) {

	unsigned int retval = 1;

	char *mongo_uri_env = getenv ("MONGO_URI");
	if (mongo_uri_env) {
		MONGO_URI = str_new (mongo_uri_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_URI from env!");
	}

	return retval;

}

static void gepp_env_get_cerver_receive_buffer_size (void) {

	char *buffer_size = getenv ("CERVER_RECEIVE_BUFFER_SIZE");
	if (buffer_size) {
		CERVER_RECEIVE_BUFFER_SIZE = (unsigned int) atoi (buffer_size);
		cerver_log_success (
			"CERVER_RECEIVE_BUFFER_SIZE -> %d\n", CERVER_RECEIVE_BUFFER_SIZE
		);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_RECEIVE_BUFFER_SIZE from env - using default %d!",
			CERVER_RECEIVE_BUFFER_SIZE
		);
	}
}

static void gepp_env_get_cerver_th_threads (void) {

	char *th_threads = getenv ("CERVER_TH_THREADS");
	if (th_threads) {
		CERVER_TH_THREADS = (unsigned int) atoi (th_threads);
		cerver_log_success ("CERVER_TH_THREADS -> %d\n", CERVER_TH_THREADS);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_TH_THREADS from env - using default %d!",
			CERVER_TH_THREADS
		);
	}

}

#pragma GCC diagnostic pop

static unsigned int things_init_env (void) {

	unsigned int errors = 0;

	errors |= things_env_get_port ();

	errors |= things_env_get_mongo_uri ();

	errors |= things_env_get_mongo_app_name ();

	errors |= things_env_get_mongo_db ();

	return errors;

}

#pragma endregion

#pragma region things

static Pool *thing_pool = NULL;

static const bson_t *thing_no_user_query_opts = NULL;
static DoubleList *thing_no_user_select = NULL;

static unsigned int things_thing_init_pool (void) {

	unsigned int retval = 1;

	thing_pool = pool_create (thing_delete);
	if (thing_pool) {
		pool_set_create (thing_pool, thing_new);
		pool_set_produce_if_empty (thing_pool, true);
		if (!pool_init (thing_pool, thing_new, DEFAULT_THINGS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init things pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create things pool!");
	}

	return retval;

}

static unsigned int things_thing_init_query_opts (void) {

	unsigned int retval = 1;

	thing_no_user_select = dlist_init (str_delete, str_comparator);
	(void) dlist_insert_after (thing_no_user_select, dlist_end (thing_no_user_select), str_new ("title"));
	(void) dlist_insert_after (thing_no_user_select, dlist_end (thing_no_user_select), str_new ("amount"));
	(void) dlist_insert_after (thing_no_user_select, dlist_end (thing_no_user_select), str_new ("date"));

	thing_no_user_query_opts = mongo_find_generate_opts (thing_no_user_select);

	if (thing_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int things_things_init (void) {

	unsigned int errors = 0;

	errors |= things_thing_init_pool ();

	errors |= things_thing_init_query_opts ();

	return errors;

}

static void things_things_end (void) {

	bson_destroy ((bson_t *) thing_no_user_query_opts);

	pool_delete (thing_pool);
	thing_pool = NULL;

}

static void things_thing_delete (void *thing_ptr) {

	(void) memset (thing_ptr, 0, sizeof (Thing));
	(void) pool_push (thing_pool, thing_ptr);

}

#pragma endregion

#pragma region main

static unsigned int things_mongo_connect (void) {

	unsigned int errors = 0;

	bool connected_to_mongo = false;

	mongo_set_uri (MONGO_URI->str);
	mongo_set_app_name (MONGO_APP_NAME->str);
	mongo_set_db_name (MONGO_DB->str);

	if (!mongo_connect ()) {
		// test mongo connection
		if (!mongo_ping_db ()) {
			cerver_log_success ("Connected to Mongo DB!");

			// open handle to actions collection
			errors |= actions_collection_get ();

			// open handle to categories collection
			errors |= categories_collection_get ();

			// open handle to labels collection
			errors |= labels_collection_get ();

			// open handle to roles collection
			errors |= roles_collection_get ();

			// open handle to things collection
			errors |= things_collection_get ();

			// open handle to users collection
			errors |= users_collection_get ();

			connected_to_mongo = true;
		}
	}

	if (!connected_to_mongo) {
		cerver_log_error ("Failed to connect to mongo!");
		errors |= 1;
	}

	return errors;

}

static unsigned int things_mongo_init (void) {

	unsigned int retval = 1;

	if (!things_mongo_connect ()) {
		if (!things_roles_init ()) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to get roles from db!");
		}
	}

	return retval;

}

static unsigned int things_init_responses (void) {

	unsigned int retval = 1;

	oki_doki = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	bad_request = http_response_json_key_value (
		(http_status) 400, "error", "Bad request!"
	);

	server_error = http_response_json_key_value (
		(http_status) 500, "error", "Internal server error!"
	);

	bad_user = http_response_json_key_value (
		(http_status) 400, "error", "Bad user!"
	);

	missing_values = http_response_json_key_value (
		(http_status) 400, "error", "Missing values!"
	);

	things_works = http_response_json_key_value (
		(http_status) 200, "msg", "Things works!"
	);

	char *status = c_string_create ("%s - %s", THINGS_VERSION_NAME, THINGS_VERSION_DATE);
	if (status) {
		current_version = http_response_json_key_value (
			(http_status) 200, "version", status
		);

		free (status);
	}

	/*** categories ****/

	no_user_categories = http_response_json_key_value (
		(http_status) 404, "msg", "Failed to get user's categories"
	);

	no_user_category = http_response_json_key_value (
		(http_status) 404, "msg", "User's category was not found"
	);

	category_created_success = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	category_created_bad = http_response_json_key_value (
		(http_status) 400, "error", "Failed to create category!"
	);

	category_deleted_success = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	category_deleted_bad = http_response_json_key_value (
		(http_status) 400, "error", "Failed to delete category!"
	);

	if (
		oki_doki && bad_request && server_error && bad_user && missing_values
		&& things_works && current_version
		&& no_user_categories && no_user_category
		&& category_created_success && category_created_bad
		&& category_deleted_success && category_deleted_bad
	) retval = 0;

	return retval;

}

// inits things main values
unsigned int things_init (void) {

	unsigned int errors = 0;

	if (!things_init_env ()) {
		errors |= things_mongo_init ();

		errors |= things_users_init ();

		errors |= things_categories_init ();

		errors |= things_labels_init ();

		errors |= things_things_init ();

		errors |= things_init_responses ();
	}

	return errors; 

}

static unsigned int things_mongo_end (void) {

	if (mongo_get_status () == MONGO_STATUS_CONNECTED) {
		actions_collection_close ();

		categories_collection_close ();

		labels_collection_close ();

		roles_collection_close ();

		things_collection_close ();

		users_collection_close ();

		mongo_disconnect ();
	}

	return 0;

}

// ends things main values
unsigned int things_end (void) {

	unsigned int errors = 0;

	errors |= things_mongo_end ();

	things_roles_end ();

	things_users_end ();

	things_categories_end ();

	things_labels_end ();

	things_things_end ();

	http_respponse_delete (oki_doki);
	http_respponse_delete (bad_request);
	http_respponse_delete (server_error);
	http_respponse_delete (bad_user);
	http_respponse_delete (missing_values);

	http_respponse_delete (things_works);
	http_respponse_delete (current_version);

	http_respponse_delete (no_user_categories);
	http_respponse_delete (no_user_category);

	http_respponse_delete (category_created_success);
	http_respponse_delete (category_created_bad);
	http_respponse_delete (category_deleted_success);
	http_respponse_delete (category_deleted_bad);

	str_delete ((String *) MONGO_URI);
	str_delete ((String *) MONGO_APP_NAME);
	str_delete ((String *) MONGO_DB);

	return errors;

}

#pragma endregion

#pragma region routes

// GET api/things
void things_handler (CerverReceive *cr, HttpRequest *request) {

	(void) http_response_send (things_works, cr->cerver, cr->connection);

}

// GET api/things/version
void things_version_handler (CerverReceive *cr, HttpRequest *request) {

	(void) http_response_send (current_version, cr->cerver, cr->connection);

}

// GET api/things/auth
void things_auth_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;

	if (user) {
		#ifdef THINGS_DEBUG
		user_print (user);
		#endif

		(void) http_response_send (oki_doki, cr->cerver, cr->connection);
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

#pragma endregion

#pragma region categories

static char *things_categories_handler_generate_json (
	User *user,
	mongoc_cursor_t *categories_cursor,
	size_t *json_len
) {

	char *retval = NULL;

	bson_t *doc = bson_new ();
	if (doc) {
		(void) bson_append_int32 (doc, "count", -1, user->categories_count);

		bson_t categories_array = { 0 };
		(void) bson_append_array_begin (doc, "categories", -1, &categories_array);
		char buf[16] = { 0 };
		const char *key = NULL;
		size_t keylen = 0;

		int i = 0;
		const bson_t *category_doc = NULL;
		while (mongoc_cursor_next (categories_cursor, &category_doc)) {
			keylen = bson_uint32_to_string (i, &key, buf, sizeof (buf));
			(void) bson_append_document (&categories_array, key, (int) keylen, category_doc);

			bson_destroy ((bson_t *) category_doc);

			i++;
		}
		(void) bson_append_array_end (doc, &categories_array);

		retval = bson_as_relaxed_extended_json (doc, json_len);
	}

	return retval;

}

// GET api/things/categories
// get all the authenticated user's categories
void things_categories_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		// get user's categories from the db
		if (!user_get_by_id (user, user->id, user_categories_query_opts)) {
			mongoc_cursor_t *categories_cursor = categories_get_all_by_user (
				&user->oid, category_no_user_query_opts
			);

			if (categories_cursor) {
				// convert them to json and send them back
				size_t json_len = 0;
				char *json = things_categories_handler_generate_json (
					user, categories_cursor, &json_len
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

				mongoc_cursor_destroy (categories_cursor);
			}

			else {
				(void) http_response_send (no_user_categories, cr->cerver, cr->connection);
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

static void things_category_parse_json (
	json_t *json_body,
	const char **title,
	const char **description
) {

	// get values from json to create a new category
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
		}
	}

}

static Category *things_category_create_handler_internal (
	const char *user_id, const String *request_body
) {

	Category *category = NULL;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			things_category_parse_json (
				json_body,
				&title,
				&description
			);

			category = things_category_create (
				user_id,
				title, description
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

	return category;

}

// POST api/things/categories
// a user has requested to create a new category
void things_category_create_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Category *category = things_category_create_handler_internal (
			user->id, request->body
		);

		if (category) {
			#ifdef THINGS_DEBUG
			category_print (category);
			#endif

			if (!mongo_insert_one (
				categories_collection,
				category_to_bson (category)
			)) {
				// update users values
				(void) mongo_update_one (
					users_collection,
					user_query_id (user->id),
					user_create_update_things_categories ()
				);

				// return success to user
				(void) http_response_send (
					category_created_success,
					cr->cerver, cr->connection
				);
			}

			else {
				(void) http_response_send (
					category_created_bad,
					cr->cerver, cr->connection
				);
			}
			
			things_category_delete (category);
		}

		else {
			(void) http_response_send (
				category_created_bad,
				cr->cerver, cr->connection
			);
		}
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

// GET api/things/categories/:id
// returns information about an existing category that belongs to a user
void things_category_get_handler (CerverReceive *cr, HttpRequest *request) {

}

// POST api/things/categories/:id
// a user wants to update an existing category
void things_category_update_handler (CerverReceive *cr, HttpRequest *request) {

}

// DELETE api/things/categories/:id
// deletes an existing user's category
void things_category_delete_handler (CerverReceive *cr, HttpRequest *request) {

}

#pragma endregion