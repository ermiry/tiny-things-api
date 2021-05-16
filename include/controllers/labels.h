#ifndef _THINGS_LABELS_H_
#define _THINGS_LABELS_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include "models/label.h"

#define DEFAULT_LABELS_POOL_INIT			32

struct _HttpResponse;

extern struct _HttpResponse *no_user_labels;
extern struct _HttpResponse *no_user_label;
extern struct _HttpResponse *label_created_success;
extern struct _HttpResponse *label_created_bad;
extern struct _HttpResponse *label_deleted_success;
extern struct _HttpResponse *label_deleted_bad;

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

extern void things_label_return (void *label_ptr);

#endif