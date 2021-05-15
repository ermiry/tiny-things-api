#ifndef _MODELS_LABEL_H_
#define _MODELS_LABEL_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>

#define LABELS_COLL_NAME        	"labels"

#define LABEL_ID_SIZE				32
#define LABEL_TITLE_SIZE			1024
#define LABEL_DESCRIPTION_SIZE		2048
#define LABEL_COLOR_SIZE			128

extern unsigned int labels_model_init (void);

extern void labels_model_end (void);

typedef struct Label {

	// label's unique id
	bson_oid_t oid;
	char id[LABEL_ID_SIZE];

	// reference to the owner of this label
	bson_oid_t user_oid;

	bson_oid_t category_oid;

	// how the user defined this transaction
	char title[LABEL_TITLE_SIZE];

	// a description added by the user to give extra information
	char description[LABEL_DESCRIPTION_SIZE];

	// the user can select a color
	// that will be used to display all matching transactions
	// in the mobile app
	char color[LABEL_COLOR_SIZE];

	// the date when the label was created
	time_t date;

} Label;

extern void *label_new (void);

extern void label_delete (void *label_ptr);

extern void label_print (Label *label);

extern bson_t *label_query_oid (const bson_oid_t *oid);

extern bson_t *label_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

extern u8 label_get_by_oid (
	Label *label, const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 label_get_by_oid_and_user (
	Label *label,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 label_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

// get all the labels that are related to a user
extern mongoc_cursor_t *labels_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern unsigned int labels_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
);

extern unsigned int label_insert_one (const Label *label);

extern unsigned int label_update_one (const Label *label);

extern unsigned int label_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif