#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>

#include <cerver/http/http.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "things.h"

#include "models/user.h"

#include "controllers/service.h"

// GET /api/things
void things_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (things_works, http_receive);

}

// GET /api/things/version
void things_version_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	(void) http_response_send (current_version, http_receive);

}

// GET /api/things/auth
void things_auth_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;

	if (user) {
		#ifdef THINGS_DEBUG
		user_print (user);
		#endif

		(void) http_response_send (oki_doki, http_receive);
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// GET *
void things_catch_all_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	http_response_send (catch_all, http_receive);

}
