#ifndef _THINGS_LABELS_H_
#define _THINGS_LABELS_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include "errors.h"

#include "models/label.h"
#include "models/user.h"

#define DEFAULT_LABELS_POOL_INIT			32

struct _HttpResponse;

extern const bson_t *label_no_user_query_opts;

extern struct _HttpResponse *no_user_labels;
extern struct _HttpResponse *no_user_label;
extern struct _HttpResponse *label_created_success;
extern struct _HttpResponse *label_created_bad;
extern struct _HttpResponse *label_deleted_success;
extern struct _HttpResponse *label_deleted_bad;

extern unsigned int things_labels_init (void);

extern void things_labels_end (void);

extern Label *things_label_get (void);

extern unsigned int things_labels_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
);

extern Label *things_label_get_by_id_and_user (
	const String *label_id, const bson_oid_t *user_oid
);

extern u8 things_label_get_by_id_and_user_to_json (
	const char *label_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern ThingsError things_label_create (
	const User *user, const String *request_body
);

extern ThingsError things_label_update (
	const User *user, const String *label_id,
	const String *request_body
);

extern ThingsError things_label_delete (
	const User *user, const String *label_id
);

extern void things_label_return (void *label_ptr);

#endif