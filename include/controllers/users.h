#ifndef _THINGS_USERS_H_
#define _THINGS_USERS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>

#define DEFAULT_USERS_POOL_INIT			16

#pragma region main

extern const bson_t *user_categories_query_opts;
extern DoubleList *user_categories_select;

extern const bson_t *user_labels_query_opts;
extern DoubleList *user_labels_select;

extern unsigned int things_users_init (void);

extern void things_users_end (void);

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick",
// }
extern void *things_user_parse_from_json (void *user_json_ptr);

extern void things_user_delete (void *user_ptr);

#pragma endregion

#endif