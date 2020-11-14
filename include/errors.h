#ifndef _THINGS_ERRORS_H_
#define _THINGS_ERRORS_H_

#include <cerver/handler.h>

#define THINGS_ERROR_MAP(XX)						\
	XX(0,	NONE, 				None)				\
	XX(1,	BAD_REQUEST, 		Bad Request)		\
	XX(2,	MISSING_VALUES, 	Missing Values)		\
	XX(3,	BAD_USER, 			Bad User)			\
	XX(4,	SERVER_ERROR, 		Server Error)

typedef enum ThingsError {

	#define XX(num, name, string) THINGS_ERROR_##name = num,
	THINGS_ERROR_MAP (XX)
	#undef XX

} ThingsError;

extern const char *things_error_to_string (ThingsError type);

extern void things_error_send_response (ThingsError error, CerverReceive *cr);

#endif