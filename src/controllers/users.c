#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "mongo.h"
#include "users.h"

#include "models/user.h"

#pragma region main

static Pool *users_pool = NULL;

const bson_t *user_categories_query_opts = NULL;
DoubleList *user_categories_select = NULL;

const bson_t *user_labels_query_opts = NULL;
DoubleList *user_labels_select = NULL;

static unsigned int things_users_init_pool (void) {

	unsigned int retval = 1;

	users_pool = pool_create (user_delete);
	if (users_pool) {
		pool_set_create (users_pool, user_new);
		pool_set_produce_if_empty (users_pool, true);
		if (!pool_init (users_pool, user_new, DEFAULT_USERS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init users pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create users pool!");
	}

	return retval;	

}

static unsigned int things_users_init_query_opts (void) {

	unsigned int retval = 1;

	user_categories_select = dlist_init (str_delete, str_comparator);
	dlist_insert_after (user_categories_select, dlist_end (user_categories_select), str_new ("categoriesCount"));

	user_categories_query_opts = mongo_find_generate_opts (user_categories_select);

	user_labels_select = dlist_init (str_delete, str_comparator);
	dlist_insert_after (user_labels_select, dlist_end (user_labels_select), str_new ("labelsCount"));

	user_labels_query_opts = mongo_find_generate_opts (user_labels_select);

	if (
		user_categories_query_opts && user_labels_query_opts
	) retval = 0;

	return retval;

}

unsigned int things_users_init (void) {

	unsigned int errors = 0;

	errors |= things_users_init_pool ();

	errors |= things_users_init_query_opts ();

	return errors;

}

void things_users_end (void) {

	dlist_delete (user_categories_select);
	bson_destroy ((bson_t *) user_categories_query_opts);

	dlist_delete (user_labels_select);
	bson_destroy ((bson_t *) user_labels_query_opts);

	pool_delete (users_pool);
	users_pool = NULL;

}

// {
//   "email": "erick.salas@ermiry.com",
//   "iat": 1596532954
//   "id": "5eb2b13f0051f70011e9d3af",
//   "name": "Erick Salas",
//   "role": "god",
//   "username": "erick",
// }
void *things_user_parse_from_json (void *user_json_ptr) {

	json_t *user_json = (json_t *) user_json_ptr;

	User *user = user_new ();
	if (user) {
		const char *email = NULL;
		const char *id = NULL;
		const char *name = NULL;
		const char *role = NULL;
		const char *username = NULL;

		if (!json_unpack (
			user_json,
			"{s:s, s:i, s:s, s:s, s:s, s:s}",
			"email", &email,
			"iat", &user->iat,
			"id", &id,
			"name", &name,
			"role", &role,
			"username", &username
		)) {
			(void) strncpy (user->email, email, USER_EMAIL_LEN);
			(void) strncpy (user->id, id, USER_ID_LEN);
			(void) strncpy (user->name, name, USER_NAME_LEN);
			(void) strncpy (user->role, role, USER_ROLE_LEN);
			(void) strncpy (user->username, username, USER_USERNAME_LEN);

			user_print (user);
		}

		else {
			cerver_log_error ("user_parse_from_json () - json_unpack () has failed!");

			(void) pool_push (users_pool, user);
			user = NULL;
		}
	}

	return user;

}

void things_user_delete (void *user_ptr) {

	(void) memset (user_ptr, 0, sizeof (User));
	(void) pool_push (users_pool, user_ptr);

}

#pragma endregion