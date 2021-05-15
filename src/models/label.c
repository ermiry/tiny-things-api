#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/label.h"

static CMongoModel *labels_model = NULL;

static void label_doc_parse (
	void *label_ptr, const bson_t *label_doc
);

unsigned int labels_model_init (void) {

	unsigned int retval = 1;

	labels_model = cmongo_model_create (LABELS_COLL_NAME);
	if (labels_model) {
		cmongo_model_set_parser (labels_model, label_doc_parse);

		retval = 0;
	}

	return retval;

}

void labels_model_end (void) {

	cmongo_model_delete (labels_model);

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

		(void) printf ("title: %s\n", label->title);
		(void) printf ("description: %s\n", label->description);
		(void) printf ("color: %s\n", label->color);

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&label->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}

static void label_doc_parse (
	void *label_ptr, const bson_t *label_doc
) {

	Label *label = (Label *) label_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, label_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &label->oid);
				bson_oid_to_string (&label->oid, label->id);
			}

			else if (!strcmp (key, "user")) {
				bson_oid_copy (&value->value.v_oid, &label->user_oid);
			}

			else if (!strcmp (key, "category")) {
				bson_oid_copy (&value->value.v_oid, &label->category_oid);
			}

			else if (!strcmp (key, "title") && value->value.v_utf8.str) {
				(void) strncpy (
					label->title,
					value->value.v_utf8.str,
					LABEL_TITLE_SIZE - 1
				);
			}

			else if (!strcmp (key, "description")) {
				(void) strncpy (
					label->description,
					value->value.v_utf8.str,
					LABEL_DESCRIPTION_SIZE - 1
				);
			}

			else if (!strcmp (key, "color")) {
				(void) strncpy (
					label->color,
					value->value.v_utf8.str,
					LABEL_COLOR_SIZE - 1
				);
			}

			else if (!strcmp (key, "date")) {
				label->date = (time_t) bson_iter_date_time (&iter) / 1000;
			}
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

bson_t *label_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	bson_t *label_query = bson_new ();
	if (label_query) {
		(void) bson_append_oid (label_query, "_id", -1, oid);
		(void) bson_append_oid (label_query, "user", -1, user_oid);
	}

	return label_query;

}

u8 label_get_by_oid (
	Label *label, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (label && oid) {
		bson_t *label_query = bson_new ();
		if (label_query) {
			(void) bson_append_oid (label_query, "_id", -1, oid);
			retval = mongo_find_one_with_opts (
				labels_model,
				label_query, query_opts,
				label
			);
		}
	}

	return retval;

}

u8 label_get_by_oid_and_user (
	Label *label,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (label && oid && user_oid) {
		bson_t *label_query = label_query_by_oid_and_user (
			oid, user_oid
		);

		if (label_query) {
			retval = mongo_find_one_with_opts (
				labels_model,
				label_query, query_opts,
				label
			);
		}
	}

	return retval;

}

u8 label_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (oid && user_oid) {
		bson_t *label_query = label_query_by_oid_and_user (
			oid, user_oid
		);

		if (label_query) {
			retval = mongo_find_one_with_opts_to_json (
				labels_model,
				label_query, query_opts,
				json, json_len
			);
		}
	}

	return retval;

}

static bson_t *label_to_bson (const Label *label) {

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

static bson_t *label_update_bson (const Label *label) {

	bson_t *doc = NULL;

	if (label) {
		doc = bson_new ();
		if (doc) {
			bson_t set_doc = BSON_INITIALIZER;
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
				labels_model,
				query, opts
			);
		}
	}

	return retval;

}

unsigned int labels_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (user_oid) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_to_json (
				labels_model,
				query, opts,
				"labels",
				json, json_len
			);
		}
	}

	return retval;

}

unsigned int label_insert_one (const Label *label) {

	return mongo_insert_one (
		labels_model, label_to_bson (label)
	);

}

unsigned int label_update_one (const Label *label) {

	return mongo_update_one (
		labels_model,
		label_query_oid (&label->oid),
		label_update_bson (label)
	);

}

unsigned int label_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	unsigned int retval = 1;

	if (oid && user_oid) {
		bson_t *label_query = label_query_by_oid_and_user (
			oid, user_oid
		);

		if (label_query) {
			retval = mongo_delete_one (
				labels_model, label_query
			);
		}
	}

	return retval;

}