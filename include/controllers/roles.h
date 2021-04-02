#ifndef _THINGS_ROLES_H_
#define _THINGS_ROLES_H_

#include <bson/bson.h>

#include "models/role.h"

extern const Role *common_role;

extern unsigned int things_roles_init (void);

extern void things_roles_end (void);

extern const Role *things_role_get_by_oid (
	const bson_oid_t *role_oid
);

extern const Role *things_role_get_by_name (
	const char *role_name
);

extern const char *things_role_name_get_by_oid (
	const bson_oid_t *role_oid
);

#endif