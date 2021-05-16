#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "errors.h"
#include "things.h"

#include "controllers/categories.h"
#include "controllers/things.h"
#include "controllers/users.h"

#include "models/category.h"
#include "models/user.h"

// GET /api/things/things
// get all the authenticated user's things
void things_things_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = NULL;

		if (!things_thing_get_all_by_user (
			&user->oid,
			&json, &json_len
		)) {
			if (json) {
				(void) http_response_json_custom_reference_send (
					http_receive,
					HTTP_STATUS_OK,
					json, json_len
				);

				free (json);
			}

			else {
				(void) http_response_send (no_user_thing, http_receive);
			}
		}

		else {
			(void) http_response_send (no_user_thing, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/things/things
// a user has requested to create a new thing
void things_thing_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_thing_create (
			user, request->body
		);

		switch (error) {
			case THINGS_ERROR_NONE: {
				// return success to user
				(void) http_response_send (
					thing_created_success,
					http_receive
				);
			} break;

			default: {
				things_error_send_response (error, http_receive);
			} break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// GET /api/things/things/:id/info
// returns information about an existing thing that belongs to a user
void things_thing_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *thing_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		if (thing_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!things_thing_get_by_id_and_user_to_json (
				thing_id->str, &user->oid,
				thing_no_user_query_opts,
				&json, &json_len
			)) {
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, HTTP_STATUS_OK, json, json_len
					);

					free (json);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (no_user_thing, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// PUT /api/things/things/:id/update
// a user wants to update an existing thing
void things_thing_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_thing_update (
			user, request->params[0], request->body
		);

		switch (error) {
			case THINGS_ERROR_NONE: {
				(void) http_response_send (oki_doki, http_receive);
			} break;

			default: {
				things_error_send_response (error, http_receive);
			} break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// DELETE /api/things/things/:id/remove
// deletes an existing user's thing
void things_thing_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *thing_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		switch (things_thing_delete (user, thing_id)) {
			case THINGS_ERROR_NONE:
				(void) http_response_send (thing_deleted_success, http_receive);
				break;

			default:
				(void) http_response_send (thing_deleted_bad, http_receive);
				break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}