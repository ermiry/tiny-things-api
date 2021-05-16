#ifndef _THINGS_THINGS_H_
#define _THINGS_THINGS_H_

#include <bson/bson.h>

#include <cerver/collections/pool.h>

#include "errors.h"

#include "models/thing.h"
#include "models/user.h"

#define DEFAULT_THINGS_POOL_INIT			32

struct _HttpResponse;

extern Pool *things_pool;

extern const bson_t *thing_no_user_query_opts;

extern struct _HttpResponse *no_user_thing;

extern struct _HttpResponse *thing_created_success;
extern struct _HttpResponse *thing_created_bad;
extern struct _HttpResponse *thing_deleted_success;
extern struct _HttpResponse *thing_deleted_bad;

extern unsigned int thing_things_init (void);

extern void thing_things_end (void);

extern unsigned int thing_thing_get_all_by_user (
	const bson_oid_t *user_oid,
	char **json, size_t *json_len
);

extern Thing *thing_thing_get_by_id_and_user (
	const String *thing_id, const bson_oid_t *user_oid
);

extern u8 thing_thing_get_by_id_and_user_to_json (
	const char *thing_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern ThingsError thing_thing_create (
	const User *user, const String *request_body
);

extern ThingsError thing_thing_update (
	const User *user, const String *thing_id,
	const String *request_body
);

extern ThingsError thing_thing_delete (
	const User *user, const String *thing_id
);

extern void thing_thing_return (void *thing_ptr);

#endif