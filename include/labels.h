#ifndef _THINGS_LABELS_H_
#define _THINGS_LABELS_H_

#include <bson/bson.h>

#include <cerver/collections/dlist.h>
#include <cerver/collections/pool.h>

#include "models/label.h"

#define DEFAULT_LABELS_POOL_INIT			32

extern Pool *labels_pool;

extern const bson_t *label_no_user_query_opts;
extern DoubleList *label_no_user_select;

extern unsigned int things_labels_init (void);

extern void things_labels_end (void);

extern Label *things_label_get_by_id_and_user (
	const String *label_id, const bson_oid_t *user_oid
);

extern Label *things_label_create (
	const char *user_id,
	const char *title, const char *description,
	const char *color
);

extern void things_label_delete (void *label_ptr);

#endif