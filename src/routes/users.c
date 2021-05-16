#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "things.h"

#include "controllers/roles.h"
#include "controllers/users.h"

#include "models/user.h"

#define USER_FIELD_SIZE			128
#define USER_INPUT_ERROR_SIZE	512

// GET /api/users
void users_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (users_works, http_receive);

}

static void users_send_input_error (
	const HttpReceive *http_receive,
	const ThingsUserInput input
) {

	size_t input_error_size = 0;
	char input_error[USER_INPUT_ERROR_SIZE] = { 0 };

	char name_error[USER_FIELD_SIZE] = { "null" };
	char username_error[USER_FIELD_SIZE] = { "null" };
	char email_error[USER_FIELD_SIZE] = { "null" };
	char password_error[USER_FIELD_SIZE] = { "null" };
	char confirm_error[USER_FIELD_SIZE] = { "null" };

	if ((input & THINGS_USER_INPUT_NAME) == THINGS_USER_INPUT_NAME)
		(void) strncpy (name_error, "Name field is required!", USER_FIELD_SIZE - 1);

	if ((input & THINGS_USER_INPUT_USERNAME) == THINGS_USER_INPUT_USERNAME)
		(void) strncpy (username_error, "Username field is required!", USER_FIELD_SIZE - 1);

	if ((input & THINGS_USER_INPUT_EMAIL) == THINGS_USER_INPUT_EMAIL)
		(void) strncpy (email_error, "Email field is required!", USER_FIELD_SIZE - 1);

	if ((input & THINGS_USER_INPUT_PASSWORD) == THINGS_USER_INPUT_PASSWORD)
		(void) strncpy (password_error, "Password field is required!", USER_FIELD_SIZE - 1);

	if ((input & THINGS_USER_INPUT_CONFIRM) == THINGS_USER_INPUT_CONFIRM)
		(void) strncpy (confirm_error, "Password confirm field is required!", USER_FIELD_SIZE - 1);

	if ((input & THINGS_USER_INPUT_MATCH) == THINGS_USER_INPUT_MATCH)
		(void) strncpy (confirm_error, "Passwords do not match!", USER_FIELD_SIZE - 1);

	input_error_size = snprintf (
		input_error, USER_INPUT_ERROR_SIZE - 1,
		"{\"name\": \"%s\", \"username\": \"%s\", \"email\": \"%s\", \"password\": \"%s\", \"confirm\": \"%s\"}",
		name_error, username_error, email_error, password_error, confirm_error
	);

	// TODO: change to only use reference
	HttpResponse *res = http_response_create (
		HTTP_STATUS_BAD_REQUEST, input_error, input_error_size
	);

	if (res) {
		http_response_compile (res);
		// http_response_print (res);
		(void) http_response_send (res, http_receive);
		http_response_delete (res);
	}

}

// POST /api/users/register
void users_register_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	ThingsUserError error = THINGS_USER_ERROR_NONE;
	ThingsUserInput input = THINGS_USER_INPUT_NONE;

	char token[HTTP_JWT_TOKEN_SIZE] = { 0 };
	size_t token_size = 0;

	User *user = things_user_register (
		request->body, &error, &input
	);

	// TODO: handle errors
	switch (error) {
		case THINGS_USER_ERROR_NONE: {
			(void) things_user_generate_token (
				user, token, &token_size
			);

			// return token back to the user
			(void) http_response_render_json (
				http_receive, HTTP_STATUS_OK,
				token, token_size
			);

			things_user_delete (user);
		} break;

		case THINGS_USER_ERROR_BAD_REQUEST:
			users_send_input_error (http_receive, input);
			break;

		case THINGS_USER_ERROR_MISSING_VALUES:
			users_send_input_error (http_receive, input);
			break;

		case THINGS_USER_ERROR_REPEATED: break;
		case THINGS_USER_ERROR_NOT_FOUND: break;
		case THINGS_USER_ERROR_WRONG_PSWD: break;
		case THINGS_USER_ERROR_SERVER_ERROR: break;

		default: break;
	}

}

// POST /api/users/login
void users_login_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	ThingsUserError error = THINGS_USER_ERROR_NONE;
	ThingsUserInput input = THINGS_USER_INPUT_NONE;

	char token[HTTP_JWT_TOKEN_SIZE] = { 0 };
	size_t token_size = 0;

	User *user = things_user_login (
		request->body, &error, &input
	);

	// TODO: handle errors
	switch (error) {
		case THINGS_USER_ERROR_NONE: {
			(void) things_user_generate_token (
				user, token, &token_size
			);

			// return token back to the user
			(void) http_response_render_json (
				http_receive, HTTP_STATUS_OK,
				token, token_size
			);

			things_user_delete (user);
		} break;

		case THINGS_USER_ERROR_BAD_REQUEST:
			users_send_input_error (http_receive, input);
			break;

		case THINGS_USER_ERROR_MISSING_VALUES:
			users_send_input_error (http_receive, input);
			break;

		case THINGS_USER_ERROR_NOT_FOUND: break;
		case THINGS_USER_ERROR_WRONG_PSWD: break;
		case THINGS_USER_ERROR_SERVER_ERROR: break;

		default: break;
	}

}