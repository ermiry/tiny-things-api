#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/cerver.h>
#include <cerver/version.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/log.h>
#include <cerver/utils/utils.h>

#include "things.h"
#include "version.h"

#include "controllers/users.h"

#include "routes/categories.h"
#include "routes/labels.h"
#include "routes/service.h"
#include "routes/things.h"
#include "routes/users.h"

static Cerver *things_api = NULL;
HttpCerver *http_cerver = NULL;

void end (int dummy) {
	
	if (things_api) {
		cerver_stats_print (things_api, false, false);
		printf ("\nHTTP Cerver stats:\n");
		http_cerver_all_stats_print ((HttpCerver *) things_api->cerver_data);
		printf ("\n");
		cerver_teardown (things_api);
	}

	(void) things_end ();

	cerver_end ();

	exit (0);

}

static void things_set_things_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/things
	HttpRoute *main_things_route = http_route_create (REQUEST_METHOD_GET, "api/things", things_handler);
	http_cerver_route_register (http_cerver, main_things_route);

	/* register things children routes */
	// GET api/things/version
	HttpRoute *things_version_route = http_route_create (REQUEST_METHOD_GET, "version", things_version_handler);
	http_route_child_add (main_things_route, things_version_route);

	// GET api/things/auth
	HttpRoute *things_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", things_auth_handler);
	http_route_set_auth (things_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (things_auth_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, things_auth_route);

	/*** things ***/

	// GET api/things/things
	HttpRoute *things_route = http_route_create (REQUEST_METHOD_GET, "things", things_things_handler);
	http_route_set_auth (things_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (things_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, things_route);

	// POST api/things/things
	http_route_set_handler (things_route, REQUEST_METHOD_POST, things_thing_create_handler);

	// GET api/things/things/:id/info
	HttpRoute *thing_info_route = http_route_create (REQUEST_METHOD_GET, "things/:id/info", things_thing_get_handler);
	http_route_set_auth (thing_info_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (thing_info_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, thing_info_route);

	// PUT api/things/things/:id/update
	HttpRoute *thing_update_route = http_route_create (REQUEST_METHOD_PUT, "things/:id/update", things_thing_update_handler);
	http_route_set_auth (thing_update_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (thing_update_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, thing_update_route);

	// DELETE api/things/things/:id/remove
	HttpRoute *thing_remove_route = http_route_create (REQUEST_METHOD_DELETE, "things/:id/remove", things_thing_delete_handler);
	http_route_set_auth (thing_remove_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (thing_remove_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, thing_remove_route);

	/*** categories ***/

	// GET api/things/categories
	HttpRoute *categories_route = http_route_create (REQUEST_METHOD_GET, "categories", things_categories_handler);
	http_route_set_auth (categories_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (categories_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, categories_route);

	// POST api/things/categories
	http_route_set_handler (categories_route, REQUEST_METHOD_POST, things_category_create_handler);

	// GET api/things/categories/:id/info
	HttpRoute *category_info_route = http_route_create (REQUEST_METHOD_GET, "categories/:id/info", things_category_get_handler);
	http_route_set_auth (category_info_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (category_info_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, category_info_route);

	// PUT api/things/categories/:id/update
	HttpRoute *category_update_route = http_route_create (REQUEST_METHOD_PUT, "categories/:id/update", things_category_update_handler);
	http_route_set_auth (category_update_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (category_update_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, category_update_route);
	
	// DELETE api/things/categories/:id/remove
	HttpRoute *category_remove_route = http_route_create (REQUEST_METHOD_DELETE, "categories/:id/remove", things_category_delete_handler);
	http_route_set_auth (category_remove_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (category_remove_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, category_remove_route);

	/*** labels ***/

	// GET api/things/labels
	HttpRoute *labels_route = http_route_create (REQUEST_METHOD_GET, "labels", things_labels_handler);
	http_route_set_auth (labels_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (labels_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, labels_route);

	// POST api/things/labels
	http_route_set_handler (labels_route, REQUEST_METHOD_POST, things_label_create_handler);

	// GET api/things/labels/:id/info
	HttpRoute *label_info_route = http_route_create (REQUEST_METHOD_GET, "labels/:id/info", things_label_get_handler);
	http_route_set_auth (label_info_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (label_info_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, label_info_route);

	// PUT api/things/labels/:id/update
	HttpRoute *label_update_route = http_route_create (REQUEST_METHOD_PUT, "labels/:id/update", things_label_update_handler);
	http_route_set_auth (label_update_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (label_update_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, label_update_route);

	// DELETE api/things/labels/:id/remove
	HttpRoute *label_remove_route = http_route_create (REQUEST_METHOD_DELETE, "labels/:id/remove", things_label_delete_handler);
	http_route_set_auth (label_remove_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (label_remove_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (main_things_route, label_remove_route);

}

static void things_set_users_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/users
	HttpRoute *users_route = http_route_create (REQUEST_METHOD_GET, "api/users", users_handler);
	http_cerver_route_register (http_cerver, users_route);

	/* register users children routes */
	// POST api/users/login
	HttpRoute *users_login_route = http_route_create (REQUEST_METHOD_POST, "login", users_login_handler);
	http_route_child_add (users_route, users_login_route);

	// POST api/users/register
	HttpRoute *users_register_route = http_route_create (REQUEST_METHOD_POST, "register", users_register_handler);
	http_route_child_add (users_route, users_register_route);

}

static void start (void) {

	things_api = cerver_create (
		CERVER_TYPE_WEB,
		"things-api",
		PORT,
		PROTOCOL_TCP,
		false,
		CERVER_CONNECTION_QUEUE
	);

	if (things_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (things_api, CERVER_RECEIVE_BUFFER_SIZE);
		cerver_set_thpool_n_threads (things_api, CERVER_TH_THREADS);
		cerver_set_handler_type (things_api, CERVER_HANDLER_TYPE_THREADS);

		cerver_set_reusable_address_flags (things_api, true);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) things_api->cerver_data;

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		if (ENABLE_USERS_ROUTES) {
			http_cerver_auth_set_jwt_priv_key_filename (http_cerver, PRIV_KEY->str);
		}
		
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, PUB_KEY->str);

		things_set_things_routes (http_cerver);

		if (ENABLE_USERS_ROUTES) {
			things_set_users_routes (http_cerver);
		}

		// add a catch all route
		http_cerver_set_catch_all_route (http_cerver, things_catch_all_handler);

		if (cerver_start (things_api)) {
			cerver_log_error (
				"Failed to start %s!",
				things_api->info->name->str
			);

			cerver_delete (things_api);
		}
	}

	else {
		cerver_log_error ("Failed to create cerver!");

		cerver_delete (things_api);
	}

}

int main (int argc, char const **argv) {

	srand (time (NULL));

	// register to the quit signal
	(void) signal (SIGINT, end);
	(void) signal (SIGTERM, end);

	// to prevent SIGPIPE when writting to socket
	(void) signal (SIGPIPE, SIG_IGN);

	cerver_init ();

	cerver_version_print_full ();

	things_version_print_full ();

	if (!things_init ()) {
		start ();
	}

	else {
		cerver_log_error ("Failed to init things!");
	}

	(void) things_end ();

	cerver_end ();

	return 0;

}