#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/label.h"

#define LABELS_COLL_NAME         				"labels"

mongoc_collection_t *labels_collection = NULL;

// opens handle to labels collection
unsigned int labels_collection_get (void) {

	unsigned int retval = 1;

	labels_collection = mongo_collection_get (LABELS_COLL_NAME);
	if (labels_collection) {
		cerver_log_debug ("Opened handle to labels collection!");
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to labels collection!");
	}

	return retval;

}

void labels_collection_close (void) {

	if (labels_collection) mongoc_collection_destroy (labels_collection);

}

void *label_new (void) {

	Label *label = (Label *) malloc (sizeof (Label));
	if (label) {
		(void) memset (label, 0, sizeof (Label));
	}

	return label;

}

void label_delete (void *label_ptr) {

	if (label_ptr) free (label_ptr);

}

void label_print (Label *label) {

	if (label) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&label->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		bson_oid_to_string (&label->category_oid, buffer);
		(void) printf ("label: %s\n", buffer);

		(void) printf ("title: %s\n", label->title);
		(void) printf ("description: %s\n", label->description);
		(void) printf ("color: %s\n", label->color);

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&label->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}

static void label_doc_parse (Label *label, const bson_t *label_doc) {

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, label_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id"))
				bson_oid_copy (&value->value.v_oid, &label->oid);

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &label->user_oid);

			else if (!strcmp (key, "category"))
				bson_oid_copy (&value->value.v_oid, &label->category_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) 
				(void) strncpy (label->title, value->value.v_utf8.str, LABEL_TITLE_LEN);

			else if (!strcmp (key, "description")) 
				(void) strncpy (label->description, value->value.v_utf8.str, LABEL_DESCRIPTION_LEN);

			else if (!strcmp (key, "color")) 
				(void) strncpy (label->color, value->value.v_utf8.str, LABEL_COLOR_LEN);

			else if (!strcmp (key, "date")) 
				label->date = (time_t) bson_iter_date_time (&iter) / 1000;
		}
	}

}

bson_t *label_query_oid (const bson_oid_t *oid) {

	bson_t *query = NULL;

	if (oid) {
		query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "_id", -1, oid);
		}
	}

	return query;

}

const bson_t *label_find_by_oid (
	const bson_oid_t *oid, const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *label_query = bson_new ();
	if (label_query) {
		(void) bson_append_oid (label_query, "_id", -1, oid);
		retval = mongo_find_one_with_opts (labels_collection, label_query, query_opts);
	}

	return retval;

}

u8 label_get_by_oid (
	Label *label, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (label) {
		const bson_t *label_doc = label_find_by_oid (oid, query_opts);
		if (label_doc) {
			label_doc_parse (label, label_doc);
			bson_destroy ((bson_t *) label_doc);

			retval = 0;
		}
	}

	return retval;

}

const bson_t *label_find_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	const bson_t *retval = NULL;

	bson_t *label_query = bson_new ();
	if (label_query) {
		(void) bson_append_oid (label_query, "_id", -1, oid);
		(void) bson_append_oid (label_query, "user", -1, user_oid);

		retval = mongo_find_one_with_opts (labels_collection, label_query, query_opts);
	}

	return retval;

}

u8 label_get_by_oid_and_user (
	Label *label,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (label) {
		const bson_t *label_doc = label_find_by_oid_and_user (oid, user_oid, query_opts);
		if (label_doc) {
			label_doc_parse (label, label_doc);
			bson_destroy ((bson_t *) label_doc);

			retval = 0;
		}
	}

	return retval;

}

bson_t *label_to_bson (Label *label) {

    bson_t *doc = NULL;

    if (label) {
        doc = bson_new ();
        if (doc) {
            (void) bson_append_oid (doc, "_id", -1, &label->oid);

			(void) bson_append_oid (doc, "user", -1, &label->user_oid);

			(void) bson_append_utf8 (doc, "title", -1, label->title, -1);
			(void) bson_append_utf8 (doc, "description", -1, label->description, -1);
			(void) bson_append_utf8 (doc, "color", -1, label->color, -1);

			(void) bson_append_date_time (doc, "date", -1, label->date * 1000);
        }
    }

    return doc;

}

bson_t *label_update_bson (Label *label) {

	bson_t *doc = NULL;

    if (label) {
        doc = bson_new ();
        if (doc) {
			bson_t set_doc = { 0 };
			(void) bson_append_document_begin (doc, "$set", -1, &set_doc);
			(void) bson_append_utf8 (&set_doc, "title", -1, label->title, -1);
			(void) bson_append_utf8 (&set_doc, "description", -1, label->description, -1);
			(void) bson_append_utf8 (&set_doc, "color", -1, label->color, -1);
			(void) bson_append_document_end (doc, &set_doc);
        }
    }

    return doc;

}

// get all the labels that are related to a user
mongoc_cursor_t *labels_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				labels_collection,
				query, opts
			);
		}
	}

	return retval;

}