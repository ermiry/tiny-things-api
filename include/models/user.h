#ifndef _MODELS_USER_H_
#define _MODELS_USER_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#define USERS_COLL_NAME			"users"

#define USER_ID_SIZE				32
#define USER_EMAIL_SIZE			128
#define USER_NAME_SIZE			128
#define USER_USERNAME_SIZE		128
#define USER_PASSWORD_SIZE		128
#define USER_ROLE_SIZE			64

extern unsigned int users_model_init (void);

extern void users_model_end (void);

typedef struct User {

	// user's unique id
	char id[USER_ID_SIZE];
	bson_oid_t oid;

	// main user values
	char email[USER_EMAIL_SIZE];
	char name[USER_NAME_SIZE];
	char username[USER_USERNAME_SIZE];
	char password[USER_PASSWORD_SIZE];

	// the role this user belongs to
	// based on its role, a user can perform different operations
	char role[USER_ROLE_SIZE];
	bson_oid_t role_oid;

	// used to validate JWT expiration
	time_t iat;

} User;

extern void *user_new (void);

extern void user_delete (void *user_ptr);

extern void user_print (User *user);

extern bson_t *user_query_id (const char *id);

extern bson_t *user_query_email (const char *email);

extern u8 user_get_by_id (
	User *user, const char *id, const bson_t *query_opts
);

extern u8 user_check_by_email (const char *email);

// gets a user from the db by its email
extern u8 user_get_by_email (
	User *user, const char *email, const bson_t *query_opts
);

// gets a user from the db by its username
extern u8 user_get_by_username (
	User *user, const String *username, const bson_t *query_opts
);

extern bson_t *user_bson_create (const User *user);

extern unsigned int user_insert_one (const User *user);

#endif