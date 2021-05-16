#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include "models/category.h"

#include "controllers/categories.h"

static Pool *categories_pool = NULL;

const bson_t *category_no_user_query_opts = NULL;
DoubleList *category_no_user_select = NULL;

HttpResponse *no_user_categories = NULL;
HttpResponse *no_user_category = NULL;
HttpResponse *category_created_success = NULL;
HttpResponse *category_created_bad = NULL;
HttpResponse *category_deleted_success = NULL;
HttpResponse *category_deleted_bad = NULL;

void things_category_delete (void *category_ptr);

static unsigned int things_categories_init_pool (void) {

	unsigned int retval = 1;

	categories_pool = pool_create (category_delete);
	if (categories_pool) {
		pool_set_create (categories_pool, category_new);
		pool_set_produce_if_empty (categories_pool, true);
		if (!pool_init (categories_pool, category_new, DEFAULT_CATEGORIES_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init categories pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create categories pool!");
	}

	return retval;

}

static unsigned int things_categories_init_query_opts (void) {

	unsigned int retval = 1;

	category_no_user_select = dlist_init (str_delete, str_comparator);
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("title"));
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("amount"));
	(void) dlist_insert_after (category_no_user_select, dlist_end (category_no_user_select), str_new ("date"));

	category_no_user_query_opts = mongo_find_generate_opts (category_no_user_select);

	if (category_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int things_categories_init_responses (void) {

	unsigned int retval = 1;

	no_user_categories = http_response_json_key_value (
		(http_status) 404, "msg", "Failed to get user's categories"
	);

	no_user_category = http_response_json_key_value (
		(http_status) 404, "msg", "User's category was not found"
	);

	category_created_success = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	category_created_bad = http_response_json_key_value (
		(http_status) 400, "error", "Failed to create category!"
	);

	category_deleted_success = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	category_deleted_bad = http_response_json_key_value (
		(http_status) 400, "error", "Failed to delete category!"
	);

	if (
		no_user_categories && no_user_category
		&& category_created_success && category_created_bad
		&& category_deleted_success && category_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int things_categories_init (void) {

	unsigned int errors = 0;

	errors |= things_categories_init_pool ();

	errors |= things_categories_init_query_opts ();

	errors |= things_categories_init_responses ();

	return errors;

}

void things_categories_end (void) {

	bson_destroy ((bson_t *) category_no_user_query_opts);

	http_response_delete (no_user_categories);
	http_response_delete (no_user_category);
	http_response_delete (category_created_success);
	http_response_delete (category_created_bad);
	http_response_delete (category_deleted_success);
	http_response_delete (category_deleted_bad);

	pool_delete (categories_pool);
	categories_pool = NULL;

}

Category *things_category_get_by_id_and_user (
	const String *category_id, const bson_oid_t *user_oid
) {

	Category *category = NULL;

	if (category_id) {
		category = (Category *) pool_pop (categories_pool);
		if (category) {
			bson_oid_init_from_string (&category->oid, category_id->str);

			if (category_get_by_oid_and_user (
				category,
				&category->oid, user_oid,
				NULL
			)) {
				things_category_delete (category);
				category = NULL;
			}
		}
	}

	return category;

}

Category *things_category_create (
	const char *user_id,
	const char *title, const char *description
) {

	Category *category = (Category *) pool_pop (categories_pool);
	if (category) {
		bson_oid_init (&category->oid, NULL);

		bson_oid_init_from_string (&category->user_oid, user_id);

		if (title) (void) strncpy (category->title, title, CATEGORY_TITLE_SIZE - 1);
		if (description) (void) strncpy (category->description, description, CATEGORY_DESCRIPTION_SIZE - 1);
		
		category->date = time (NULL);
	}

	return category;

}

void things_category_return (void *category_ptr) {

	(void) memset (category_ptr, 0, sizeof (Category));
	(void) pool_push (categories_pool, category_ptr);

}