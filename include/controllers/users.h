#ifndef _THINGS_USERS_H_
#define _THINGS_USERS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>

#include "models/user.h"

#define DEFAULT_USERS_POOL_INIT			16

struct _HttpReceive;
struct _HttpResponse;

typedef enum ThingsUserInput {

	THINGS_USER_INPUT_NONE			= 0,
	THINGS_USER_INPUT_NAME			= 1,
	THINGS_USER_INPUT_USERNAME		= 2,
	THINGS_USER_INPUT_EMAIL			= 4,
	THINGS_USER_INPUT_PASSWORD		= 8,
	THINGS_USER_INPUT_CONFIRM		= 16,
	THINGS_USER_INPUT_MATCH			= 32,

} ThingsUserInput;

#define THINGS_USER_ERROR_MAP(XX)					\
	XX(0,	NONE, 				None)				\
	XX(1,	BAD_REQUEST, 		Bad Request)		\
	XX(2,	MISSING_VALUES, 	Missing Values)		\
	XX(3,	REPEATED, 			Existing Email)		\
	XX(4,	NOT_FOUND, 			Not found)			\
	XX(5,	WRONG_PSWD, 		Wrong password)		\
	XX(6,	SERVER_ERROR, 		Server Error)

typedef enum ThingsUserError {

	#define XX(num, name, string) THINGS_USER_ERROR_##name = num,
	THINGS_USER_ERROR_MAP (XX)
	#undef XX

} ThingsUserError;

extern const bson_t *user_login_query_opts;
extern const bson_t *user_transactions_query_opts;
extern const bson_t *user_categories_query_opts;
extern const bson_t *user_places_query_opts;

extern struct _HttpResponse *users_works;
extern struct _HttpResponse *missing_user_values;
extern struct _HttpResponse *wrong_password;
extern struct _HttpResponse *user_not_found;
extern struct _HttpResponse *repeated_email;

extern unsigned int things_users_init (void);

extern void things_users_end (void);

extern User *things_user_create (
	const char *name,
	const char *username,
	const char *email,
	const char *password,
	const bson_oid_t *role_oid
);

extern User *things_user_get (void);

extern User *things_user_get_by_email (const char *email);

extern u8 things_user_check_by_email (
	const char *email
);

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick"
// }
extern void *things_user_parse_from_json (void *user_json_ptr);

extern unsigned int things_user_generate_token (
	const User *user, char *json_token, size_t *json_len
);

extern User *things_user_register (
	const String *request_body, 
	ThingsUserError *error, ThingsUserInput *input
);

extern User *things_user_login (
	const String *request_body, 
	ThingsUserError *error, ThingsUserInput *input
);

extern void things_user_delete (void *user_ptr);

#endif