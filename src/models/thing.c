#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/thing.h"

static CMongoModel *things_model = NULL;

static void thing_doc_parse (
	void *user_ptr, const bson_t *user_doc
);

unsigned int things_model_init (void) {

	unsigned int retval = 1;

	things_model = cmongo_model_create (THINGS_COLL_NAME);
	if (things_model) {
		cmongo_model_set_parser (things_model, thing_doc_parse);

		retval = 0;
	}

	return retval;

}

void things_model_end (void) {

	cmongo_model_delete (things_model);

}

const char *things_status_to_string (const ThingStatus status) {

	switch (status) {
		#define XX(num, name, string) case THING_STATUS_##name: return #string;
		THING_STATUS_MAP(XX)
		#undef XX
	}

	return things_status_to_string (THING_STATUS_NONE);

}

void *thing_new (void) {

	Thing *thing = (Thing *) malloc (sizeof (Thing));
	if (thing) {
		(void) memset (thing, 0, sizeof (Thing));
	}

	return thing;

}

void thing_delete (void *thing_ptr) {

	if (thing_ptr) free (thing_ptr);

}

void thing_print (const Thing *thing) {

	if (thing) {
		(void) printf ("id: %s\n", thing->id);

		(void) printf ("title: %s\n", thing->title);
		(void) printf ("description: %s\n", thing->description);

		(void) printf ("status: %s\n", things_status_to_string (thing->status));

		char buffer[128] = { 0 };
		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&thing->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}

static void thing_doc_parse (
	void *thing_ptr, const bson_t *thing_doc
) {

	Thing *thing = (Thing *) thing_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, thing_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &thing->oid);
				bson_oid_to_string (&thing->oid, thing->id);
			}

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &thing->user_oid);

			else if (!strcmp (key, "category"))
				bson_oid_copy (&value->value.v_oid, &thing->category_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) {
				(void) strncpy (
					thing->title,
					value->value.v_utf8.str,
					THING_TITLE_SIZE - 1
				);
			}

			else if (!strcmp (key, "date"))
				thing->date = (time_t) bson_iter_date_time (&iter) / 1000;
		}
	}

}

bson_t *thing_query_oid (const bson_oid_t *oid) {

	bson_t *query = NULL;

	if (oid) {
		query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "_id", -1, oid);
		}
	}

	return query;

}

bson_t *thing_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	bson_t *thing_query = bson_new ();
	if (thing_query) {
		(void) bson_append_oid (thing_query, "_id", -1, oid);
		(void) bson_append_oid (thing_query, "user", -1, user_oid);
	}

	return thing_query;

}

u8 thing_get_by_oid (
	Thing *thing, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (thing && oid) {
		bson_t *thing_query = bson_new ();
		if (thing_query) {
			(void) bson_append_oid (thing_query, "_id", -1, oid);
			retval = mongo_find_one_with_opts (
				things_model,
				thing_query, query_opts,
				thing
			);
		}
	}

	return retval;

}

u8 thing_get_by_oid_and_user (
	Thing *thing,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (thing && oid && user_oid) {
		bson_t *thing_query = thing_query_by_oid_and_user (
			oid, user_oid
		);

		if (thing_query) {
			retval = mongo_find_one_with_opts (
				things_model,
				thing_query, query_opts,
				thing
			);
		}
	}

	return retval;

}

u8 thing_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (oid && user_oid) {
		bson_t *thing_query = thing_query_by_oid_and_user (
			oid, user_oid
		);

		if (thing_query) {
			retval = mongo_find_one_with_opts_to_json (
				things_model,
				thing_query, query_opts,
				json, json_len
			);
		}
	}

	return retval;

}

static bson_t *thing_to_bson (const Thing *thing) {

	bson_t *doc = NULL;

	if (thing) {
		doc = bson_new ();
		if (doc) {
			(void) bson_append_oid (doc, "_id", -1, &thing->oid);

			(void) bson_append_oid (doc, "user", -1, &thing->user_oid);

			(void) bson_append_oid (doc, "category", -1, &thing->category_oid);

			(void) bson_append_utf8 (doc, "title", -1, thing->title, -1);

			(void) bson_append_date_time (doc, "date", -1, thing->date * 1000);
		}
	}

	return doc;

}

static bson_t *thing_update_bson (const Thing *thing) {

	bson_t *doc = NULL;

	if (thing) {
		doc = bson_new ();
		if (doc) {
			bson_t set_doc = BSON_INITIALIZER;
			(void) bson_append_document_begin (doc, "$set", -1, &set_doc);
			(void) bson_append_utf8 (&set_doc, "title", -1, thing->title, -1);

			(void) bson_append_oid (&set_doc, "category", -1, &thing->category_oid);

			// (void) bson_append_date_time (&set_doc, "date", -1, thing->date * 1000);
			(void) bson_append_document_end (doc, &set_doc);
		}
	}

	return doc;

}

// get all the things that are related to a user
mongoc_cursor_t *things_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				things_model,
				query, opts
			);
		}
	}

	return retval;

}

unsigned int things_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	char **json, size_t *json_len
) {

	unsigned int retval = 1;

	if (user_oid) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_to_json (
				things_model,
				query, opts,
				"things",
				json, json_len
			);
		}
	}

	return retval;

}

unsigned int thing_insert_one (const Thing *thing) {

	return mongo_insert_one (
		things_model, thing_to_bson (thing)
	);

}

unsigned int thing_update_one (const Thing *thing) {

	return mongo_update_one (
		things_model,
		thing_query_oid (&thing->oid),
		thing_update_bson (thing)
	);

}

unsigned int thing_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	unsigned int retval = 1;

	if (oid && user_oid) {
		bson_t *thing_query = thing_query_by_oid_and_user (
			oid, user_oid
		);

		if (thing_query) {
			retval = mongo_delete_one (
				things_model, thing_query
			);
		}
	}

	return retval;

}
