#ifndef _POCKET_LABELS_H_
#define _POCKET_LABELS_H_

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

extern void things_label_delete (void *label_ptr);

#endif