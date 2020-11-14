#ifndef _MODELS_LABEL_H_
#define _MODELS_LABEL_H_

#include <time.h>

#include <mongoc/mongoc.h>
#include <bson/bson.h>

#include <cerver/types/types.h>

#define LABEL_TITLE_LEN			    	512
#define LABEL_DESCRIPTION_LEN			1024
#define LABEL_COLOR_LEN					64

extern mongoc_collection_t *labels_collection;

// opens handle to labels collection
extern unsigned int labels_collection_get (void);

extern void labels_collection_close (void);

typedef struct Label {

	bson_oid_t oid;

	bson_oid_t user_oid;

	bson_oid_t category_oid;

	char title[LABEL_TITLE_LEN];
	char description[LABEL_DESCRIPTION_LEN];
	char color[LABEL_COLOR_LEN];

	time_t date;

} Label;

extern void *label_new (void);

extern void label_delete (void *label_ptr);

extern void label_print (Label *label);

#endif