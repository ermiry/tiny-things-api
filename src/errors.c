#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/response.h>

#include "things.h"
#include "errors.h"

#include "controllers/service.h"

const char *things_error_to_string (const ThingsError type) {

	switch (type) {
		#define XX(num, name, string) case THINGS_ERROR_##name: return #string;
		THINGS_ERROR_MAP(XX)
		#undef XX
	}

	return things_error_to_string (THINGS_ERROR_NONE);

}

void things_error_send_response (
	const ThingsError error,
	const HttpReceive *http_receive
) {

	switch (error) {
		case THINGS_ERROR_NONE: break;

		case THINGS_ERROR_BAD_REQUEST:
			(void) http_response_send (bad_request_error, http_receive);
			break;

		case THINGS_ERROR_MISSING_VALUES:
			(void) http_response_send (missing_values, http_receive);
			break;

		case THINGS_ERROR_BAD_USER:
			(void) http_response_send (bad_user_error, http_receive);
			break;

		case THINGS_ERROR_NOT_FOUND:
			(void) http_response_send (not_found_error, http_receive);
			break;

		case THINGS_ERROR_SERVER_ERROR:
			(void) http_response_send (server_error, http_receive);
			break;

		default: break;
	}

}