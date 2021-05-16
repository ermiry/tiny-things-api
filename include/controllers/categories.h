#ifndef _THINGS_CATEGORIES_H_
#define _THINGS_CATEGORIES_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include "models/category.h"

#define DEFAULT_CATEGORIES_POOL_INIT			32

struct _HttpResponse;

extern struct _HttpResponse *no_user_categories;
extern struct _HttpResponse *no_user_category;
extern struct _HttpResponse *category_created_success;
extern struct _HttpResponse *category_created_bad;
extern struct _HttpResponse *category_deleted_success;
extern struct _HttpResponse *category_deleted_bad;

extern unsigned int things_categories_init (void);

extern void things_categories_end (void);

extern Category *things_category_get_by_id_and_user (
	const String *category_id, const bson_oid_t *user_oid
);

extern Category *things_category_create (
	const char *user_id,
	const char *title, const char *description
);

extern void things_category_return (void *category_ptr);

#endif