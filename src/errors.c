#include <cerver/handler.h>

#include <cerver/http/response.h>

#include "things.h"
#include "errors.h"

const char *things_error_to_string (ThingsError type) {

	switch (type) {
		#define XX(num, name, string) case THINGS_ERROR_##name: return #string;
		THINGS_ERROR_MAP(XX)
		#undef XX
	}

	return things_error_to_string (THINGS_ERROR_NONE);

}

void things_error_send_response (ThingsError error, CerverReceive *cr) {

	switch (error) {
		case THINGS_ERROR_NONE: break;

		case THINGS_ERROR_BAD_REQUEST:
			(void) http_response_send (bad_request, cr->cerver, cr->connection);
			break;

		case THINGS_ERROR_MISSING_VALUES:
			(void) http_response_send (missing_values, cr->cerver, cr->connection);
			break;

		case THINGS_ERROR_BAD_USER:
			(void) http_response_send (bad_user, cr->cerver, cr->connection);
			break;

		case THINGS_ERROR_SERVER_ERROR:
			(void) http_response_send (server_error, cr->cerver, cr->connection);
			break;

		default: break;
	}

}