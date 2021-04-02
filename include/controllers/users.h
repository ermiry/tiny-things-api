#ifndef _THINGS_USERS_H_
#define _THINGS_USERS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>

#include "models/user.h"

#define DEFAULT_USERS_POOL_INIT			16

struct _HttpReceive;
struct _HttpResponse;

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

extern void things_user_delete (void *user_ptr);

#endif