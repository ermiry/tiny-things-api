#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "mongo.h"
#include "labels.h"

#include "models/label.h"

Pool *labels_pool = NULL;

const bson_t *label_no_user_query_opts = NULL;
DoubleList *label_no_user_select = NULL;

void things_label_delete (void *label_ptr);

static unsigned int things_labels_init_pool (void) {

	unsigned int retval = 1;

	labels_pool = pool_create (label_delete);
	if (labels_pool) {
		pool_set_create (labels_pool, label_new);
		pool_set_produce_if_empty (labels_pool, true);
		if (!pool_init (labels_pool, label_new, DEFAULT_LABELS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init labels pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create labels pool!");
	}

	return retval;

}

static unsigned int things_labels_init_query_opts (void) {

	unsigned int retval = 1;

	label_no_user_select = dlist_init (str_delete, str_comparator);
	(void) dlist_insert_after (label_no_user_select, dlist_end (label_no_user_select), str_new ("title"));
	(void) dlist_insert_after (label_no_user_select, dlist_end (label_no_user_select), str_new ("amount"));
	(void) dlist_insert_after (label_no_user_select, dlist_end (label_no_user_select), str_new ("date"));

	label_no_user_query_opts = mongo_find_generate_opts (label_no_user_select);

	if (label_no_user_query_opts) retval = 0;

	return retval;

}

unsigned int things_labels_init (void) {

	unsigned int errors = 0;

	errors |= things_labels_init_pool ();

	errors |= things_labels_init_query_opts ();

	return errors;

}

void things_labels_end (void) {

	bson_destroy ((bson_t *) label_no_user_query_opts);

	pool_delete (labels_pool);
	labels_pool = NULL;

}

Label *things_label_get_by_id_and_user (
	const String *label_id, const bson_oid_t *user_oid
) {

	Label *label = NULL;

	if (label_id) {
		label = (Label *) pool_pop (labels_pool);
		if (label) {
			bson_oid_init_from_string (&label->oid, label_id->str);

			if (label_get_by_oid_and_user (
				label,
				&label->oid, user_oid,
				NULL
			)) {
				things_label_delete (label);
				label = NULL;
			}
		}
	}

	return label;

}

Label *things_label_create (
	const char *user_id,
	const char *title, const char *description,
	const char *color
) {

	Label *label = (Label *) pool_pop (labels_pool);
	if (label) {
		bson_oid_init (&label->oid, NULL);

		bson_oid_init_from_string (&label->user_oid, user_id);

		if (title) (void) strncpy (label->title, title, LABEL_TITLE_LEN);
		if (description) (void) strncpy (label->description, description, LABEL_DESCRIPTION_LEN);

		if (color) (void) strncpy (label->color, color, LABEL_COLOR_LEN);
		
		label->date = time (NULL);
	}

	return label;

}

void things_label_delete (void *label_ptr) {

	(void) memset (label_ptr, 0, sizeof (Label));
	(void) pool_push (labels_pool, label_ptr);

}