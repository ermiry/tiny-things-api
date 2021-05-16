#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/thing.h"
#include "models/user.h"

#include "controllers/things.h"

Pool *things_pool = NULL;

const bson_t *thing_no_user_query_opts = NULL;
static CMongoSelect *thing_no_user_select = NULL;

HttpResponse *no_user_thing = NULL;

HttpResponse *thing_created_success = NULL;
HttpResponse *thing_created_bad = NULL;
HttpResponse *thing_deleted_success = NULL;
HttpResponse *thing_deleted_bad = NULL;

void things_thing_return (void *thing_ptr);

static unsigned int things_things_init_pool (void) {

	unsigned int retval = 1;

	things_pool = pool_create (thing_delete);
	if (things_pool) {
		pool_set_create (things_pool, thing_new);
		pool_set_produce_if_empty (things_pool, true);
		if (!pool_init (things_pool, thing_new, DEFAULT_THINGS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init thing pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create thing pool!");
	}

	return retval;

}

static unsigned int things_things_init_query_opts (void) {

	unsigned int retval = 1;

	thing_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (thing_no_user_select, "title");
	(void) cmongo_select_insert_field (thing_no_user_select, "amount");
	(void) cmongo_select_insert_field (thing_no_user_select, "date");
	(void) cmongo_select_insert_field (thing_no_user_select, "category");

	thing_no_user_query_opts = mongo_find_generate_opts (thing_no_user_select);

	if (thing_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int things_thing_init_responses (void) {

	unsigned int retval = 1;

	no_user_thing = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "Failed to get user's thing(s)"
	);

	thing_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	thing_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create thing!"
	);

	thing_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	thing_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete thing!"
	);

	if (
		no_user_thing
		&& thing_created_success && thing_created_bad
		&& thing_deleted_success && thing_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int things_things_init (void) {

	unsigned int errors = 0;

	errors |= things_thing_init_pool ();

	errors |= things_thing_init_query_opts ();

	errors |= things_thing_init_responses ();

	return errors;

}

void things_things_end (void) {

	cmongo_select_delete (thing_no_user_select);
	bson_destroy ((bson_t *) thing_no_user_query_opts);

	pool_delete (things_pool);
	things_pool = NULL;

	http_response_delete (no_user_thing);

	http_response_delete (thing_created_success);
	http_response_delete (thing_created_bad);
	http_response_delete (thing_deleted_success);
	http_response_delete (thing_deleted_bad);

}

unsigned int things_thing_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
) {

	return things_get_all_by_user_to_json (
		user_oid, thing_no_user_query_opts,
		json, json_len
	);

}

Thing *things_thing_get_by_id_and_user (
	const String *thing_id, const bson_oid_t *user_oid
) {

	Thing *thing = NULL;

	if (thing_id) {
		thing = (Thing *) pool_pop (things_pool);
		if (thing) {
			bson_oid_init_from_string (&thing->oid, thing_id->str);

			if (thing_get_by_oid_and_user (
				thing,
				&thing->oid, user_oid,
				NULL
			)) {
				things_thing_return (thing);
				thing = NULL;
			}
		}
	}

	return thing;

}

u8 things_thing_get_by_id_and_user_to_json (
	const char *thing_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (thing_id) {
		bson_oid_t thing_oid = { 0 };
		bson_oid_init_from_string (&thing_oid, thing_id);

		retval = thing_get_by_oid_and_user_to_json (
			&thing_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static Thing *things_thing_create_actual (
	const char *user_id,
	const char *title,
	const double amount,
	const char *category_id,
	const char *date
) {

	Thing *thing = (Thing *) pool_pop (things_pool);
	if (thing) {
		bson_oid_init (&thing->oid, NULL);

		bson_oid_init_from_string (&thing->user_oid, user_id);

		if (title) (void) strncpy (thing->title, title, THING_TITLE_SIZE - 1);

		// FIXME:
		// if (category_id) {
		// 	bson_oid_init_from_string (&thing->category_oid, category_id);
		// }

		if (date) {
			int y = 0, M = 0, d = 0, h = 0, m = 0;
			float s = 0;
			(void) sscanf (date, "%d-%d-%dT%d:%d:%f", &y, &M, &d, &h, &m, &s);

			struct tm date;
			date.tm_year = y - 1900;	// Year since 1900
			date.tm_mon = M - 1;		// 0-11
			date.tm_mday = d;			// 1-31
			date.tm_hour = h;			// 0-23
			date.tm_min = m;			// 0-59
			date.tm_sec = (int) s;		// 0-61 (0-60 in C++11)

			thing->date = mktime (&date);
		}

		else {
			thing->date = time (NULL);
		}
	}

	return thing;

}

static void things_thing_parse_json (
	json_t *json_body,
	const char **title,
	double *amount,
	const char **category,
	const char **place,
	const char **date
) {

	// get values from json to create a new thing
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

			else if (!strcmp (key, "amount")) {
				*amount = json_real_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("amount: %f\n", *amount);
				#endif
			}

			else if (!strcmp (key, "category")) {
				*category = json_string_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("category: \"%s\"\n", *category);
				#endif
			}

			else if (!strcmp (key, "place")) {
				*place = json_string_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("place: \"%s\"\n", *place);
				#endif
			}

			else if (!strcmp (key, "date")) {
				*date = json_string_value (value);
				#ifdef THINGS_DEBUG
				(void) printf ("date: \"%s\"\n", *date);
				#endif
			}
		}
	}

}

static ThingsError things_thing_create_parse_json (
	Thing **thing,
	const char *user_id, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	const char *title = NULL;
	double amount = 0;
	const char *category_id = NULL;
	const char *place_id = NULL;
	const char *date = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		things_thing_parse_json (
			json_body,
			&title, &amount,
			&category_id, &place_id, &date
		);

		if (title && category_id) {
			*thing = things_thing_create_actual (
				user_id,
				title, amount,
				category_id,
				date
			);

			if (*thing == NULL) error = THINGS_ERROR_SERVER_ERROR;
		}

		else {
			error = THINGS_ERROR_MISSING_VALUES;
		}

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

ThingsError things_thing_create (
	const User *user, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	if (request_body) {
		Thing *thing = NULL;

		error = things_thing_create_parse_json (
			&thing,
			user->id, request_body
		);

		if (error == THINGS_ERROR_NONE) {
			#ifdef THINGS_DEBUG
			thing_print (thing);
			#endif

			if (!thing_insert_one (thing)) {
				// update users values
				(void) user_add_things (user);
			}

			else {
				error = THINGS_ERROR_SERVER_ERROR;
			}

			things_thing_return (thing);
		}
	}

	else {
		#ifdef THINGS_DEBUG
		cerver_log_error ("Missing request body to create thing!");
		#endif

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

static ThingsError things_thing_update_parse_json (
	Thing *thing, const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	const char *title = NULL;
	double amount = 0;
	const char *category_id = NULL;
	const char *place_id = NULL;
	const char *date = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		things_thing_parse_json (
			json_body,
			&title, &amount,
			&category_id, &place_id, &date
		);

		// FIXME:
		// if (title) (void) strncpy (thing->title, title, THING_TITLE_SIZE - 1);
		// if (category_id) (void) bson_oid_init_from_string (&thing->category_oid, category_id);
		// if (place_id) (void) bson_oid_init_from_string (&thing->place_oid, place_id);

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

ThingsError things_thing_update (
	const User *user, const String *thing_id,
	const String *request_body
) {

	ThingsError error = THINGS_ERROR_NONE;

	if (request_body) {
		Thing *thing = things_thing_get_by_id_and_user (
			thing_id, &user->oid
		);

		if (thing) {
			// get update values
			if (things_thing_update_parse_json (
				thing, request_body
			) == THINGS_ERROR_NONE) {
				// update the thing in the db
				if (thing_update_one (thing)) {
					error = THINGS_ERROR_SERVER_ERROR;
				}
			}

			things_thing_return (thing);
		}

		else {
			#ifdef THINGS_DEBUG
			cerver_log_error ("Failed to get matching thing!");
			#endif

			error = THINGS_ERROR_NOT_FOUND;
		}
	}

	else {
		#ifdef THINGS_DEBUG
		cerver_log_error ("Missing request body to update thing!");
		#endif

		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

ThingsError things_thing_delete (
	const User *user, const String *thing_id
) {

	ThingsError error = THINGS_ERROR_NONE;

	bson_oid_t oid = { 0 };
	bson_oid_init_from_string (&oid, thing_id->str);

	if (!thing_delete_one_by_oid_and_user (
		&oid, &user->oid
	)) {
		#ifdef THINGS_DEBUG
		cerver_log_debug ("Deleted thing %s", thing_id->str);
		#endif
	}

	else {
		error = THINGS_ERROR_BAD_REQUEST;
	}

	return error;

}

void things_thing_return (void *thing_ptr) {

	(void) memset (thing_ptr, 0, sizeof (Thing));
	(void) pool_push (things_pool, thing_ptr);

}