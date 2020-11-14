#ifndef _POCKET_CATEGORIES_H_
#define _POCKET_CATEGORIES_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include "models/category.h"

#define DEFAULT_CATEGORIES_POOL_INIT			32

extern Pool *categories_pool;

extern const bson_t *category_no_user_query_opts;
extern DoubleList *category_no_user_select;

extern unsigned int things_categories_init (void);

extern void things_categories_end (void);

extern Category *things_category_get_by_id_and_user (
	const String *category_id, const bson_oid_t *user_oid
);

extern Category *things_category_create (
	const char *user_id,
	const char *title, const char *description
);

extern void things_category_delete (void *category_ptr);

#endif