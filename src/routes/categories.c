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

#include "things.h"

#include "controllers/categories.h"
#include "controllers/users.h"

// GET /api/things/categories
// get all the authenticated user's categories
void things_categories_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = NULL;

		if (!things_categories_get_all_by_user (
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
				(void) http_response_send (no_user_categories, http_receive);
			}
		}

		else {
			(void) http_response_send (no_user_categories, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/things/categories
// a user has requested to create a new category
void things_category_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_category_create (
			user, request->body
		);

		switch (error) {
			case THINGS_ERROR_NONE: {
				// return success to user
				(void) http_response_send (
					category_created_success,
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

// GET /api/things/categories/:id/info
// returns information about an existing category that belongs to a user
void things_category_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *category_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		if (category_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!things_category_get_by_id_and_user_to_json (
				category_id->str, &user->oid,
				category_no_user_query_opts,
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
				(void) http_response_send (no_user_category, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// PUT /api/things/categories/:id/update
// a user wants to update an existing category
void things_category_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_category_update (
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

// DELETE /api/things/categories/:id/remove
// deletes an existing user's category
void things_category_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *category_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		switch (things_category_delete (user, category_id)) {
			case THINGS_ERROR_NONE:
				(void) http_response_send (category_deleted_success, http_receive);
				break;

			default:
				(void) http_response_send (category_deleted_bad, http_receive);
				break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}
