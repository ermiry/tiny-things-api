#ifndef _MODELS_THING_H_
#define _MODELS_THING_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>

#define THINGS_COLL_NAME         		"things"

#define THING_ID_SIZE					32
#define THING_TITLE_SIZE			    1024
#define THING_DESCRIPTION_SIZE			4096

extern unsigned int things_model_init (void);

extern void things_model_end (void);

#define THING_STATUS_MAP(XX)			\
	XX(0,	NONE, 	    None)		    \
	XX(1,	TODO, 		Todo)			\
	XX(2,	PROGRESS, 	Progress)		\
	XX(3,	DONE, 		Done)

typedef enum ThingStatus {

	#define XX(num, name, string) THING_STATUS_##name = num,
	THING_STATUS_MAP (XX)
	#undef XX

} ThingStatus;

extern const char *things_status_to_string (const ThingStatus status);

typedef struct Thing {

	bson_oid_t oid;
	char id[THING_ID_SIZE];

	bson_oid_t user_oid;

	char title[THING_TITLE_SIZE];
	char description[THING_DESCRIPTION_SIZE];

	ThingStatus status;

	time_t date;

} Thing;

extern void *thing_new (void);

extern void thing_delete (void *thing_ptr);

extern void thing_print (const Thing *thing);

#endif