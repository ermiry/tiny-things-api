#ifndef _THINGS_ROLES_H_
#define _THINGS_ROLES_H_

#include <cerver/types/string.h>

#include <bson/bson.h>

#include "models/role.h"

extern const Role *common_role;

extern unsigned int things_roles_init (void);

extern void things_roles_end (void);

extern const String *things_roles_get_by_oid (const bson_oid_t *role_oid);

#endif