#ifndef _THINGS_CATEGORIES_H_
#define _THINGS_CATEGORIES_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include "errors.h"

#include "models/category.h"
#include "models/user.h"

#define DEFAULT_CATEGORIES_POOL_INIT			32

struct _HttpResponse;

extern const bson_t *category_no_user_query_opts;

extern struct _HttpResponse *no_user_categories;
extern struct _HttpResponse *no_user_category;
extern struct _HttpResponse *category_created_success;
extern struct _HttpResponse *category_created_bad;
extern struct _HttpResponse *category_deleted_success;
extern struct _HttpResponse *category_deleted_bad;

extern unsigned int things_categories_init (void);

extern void things_categories_end (void);

extern Category *things_category_get (void);

extern unsigned int things_categories_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
);

extern Category *things_category_get_by_id_and_user (
	const String *category_id, const bson_oid_t *user_oid
);

extern u8 things_category_get_by_id_and_user_to_json (
	const char *category_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern ThingsError things_category_create (
	const User *user, const String *request_body
);

extern ThingsError things_category_update (
	const User *user, const String *category_id,
	const String *request_body
);

extern ThingsError things_category_delete (
	const User *user, const String *category_id
);

extern void things_category_return (void *category_ptr);

#endif