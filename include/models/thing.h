#ifndef _MODELS_THING_H_
#define _MODELS_THING_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>

#define THING_TITLE_LEN			    1024
#define THING_DESCRIPTION_LEN			4096

extern mongoc_collection_t *things_collection;

// opens handle to things collection
extern unsigned int things_collection_get (void);

extern void things_collection_close (void);

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

extern const char *things_status_to_string (ThingStatus status);

typedef struct Thing {

	bson_oid_t oid;

	bson_oid_t user_oid;

	char title[THING_TITLE_LEN];
	char description[THING_DESCRIPTION_LEN];

	ThingStatus status;

	time_t date;

} Thing;

extern void *thing_new (void);

extern void thing_delete (void *thing_ptr);

extern void thing_print (Thing *thing);

#endif