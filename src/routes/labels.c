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

#include "controllers/labels.h"
#include "controllers/users.h"

// GET /api/things/labels
// get all the authenticated user's labels
void things_labels_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = NULL;

		if (!things_labels_get_all_by_user (
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
				(void) http_response_send (no_user_labels, http_receive);
			}
		}

		else {
			(void) http_response_send (no_user_labels, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/things/labels
// a user has requested to create a new label
void things_label_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_label_create (
			user, request->body
		);

		switch (error) {
			case THINGS_ERROR_NONE: {
				// return success to user
				(void) http_response_send (
					label_created_success,
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

// GET /api/things/labels/:id/info
// returns information about an existing label that belongs to a user
void things_label_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *label_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		if (label_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!things_label_get_by_id_and_user_to_json (
				label_id->str, &user->oid,
				label_no_user_query_opts,
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
				(void) http_response_send (no_user_label, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// PUT /api/things/labels/:id/update
// a user wants to update an existing label
void things_label_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		ThingsError error = things_label_update (
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

// DELETE /api/things/labels/:id/remove
// deletes an existing user's label
void things_label_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *label_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		switch (things_label_delete (user, label_id)) {
			case THINGS_ERROR_NONE:
				(void) http_response_send (label_deleted_success, http_receive);
				break;

			default:
				(void) http_response_send (label_deleted_bad, http_receive);
				break;
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}
