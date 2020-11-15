#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/version.h>
#include <cerver/cerver.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "handler.h"
#include "things.h"
#include "users.h"
#include "version.h"

static Cerver *things_api = NULL;

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
	HttpRoute *things_route = http_route_create (REQUEST_METHOD_GET, "api/things", things_handler);
	http_cerver_route_register (http_cerver, things_route);

	/* register things children routes */
	// GET api/things/version
	HttpRoute *things_version_route = http_route_create (REQUEST_METHOD_GET, "version", things_version_handler);
	http_route_child_add (things_route, things_version_route);

	// GET api/things/auth
	HttpRoute *things_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", things_auth_handler);
	http_route_set_auth (things_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (things_auth_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (things_route, things_auth_route);

	/*** categories ***/

	// GET api/things/categories
	HttpRoute *categories_route = http_route_create (REQUEST_METHOD_GET, "categories", things_categories_handler);
	http_route_set_auth (categories_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (categories_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (things_route, categories_route);

	// POST api/things/categories
	http_route_set_handler (categories_route, REQUEST_METHOD_POST, things_category_create_handler);

	// GET api/things/categories/:id
	HttpRoute *single_category_route = http_route_create (REQUEST_METHOD_GET, "categories/:id", things_category_get_handler);
	http_route_set_auth (single_category_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (single_category_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (things_route, single_category_route);

	// POST api/things/categories/:id
	http_route_set_handler (single_category_route, REQUEST_METHOD_POST, things_category_update_handler);

	// DELETE api/things/categories/:id
	http_route_set_handler (single_category_route, REQUEST_METHOD_DELETE, things_category_delete_handler);

	/*** labels ***/

	// GET api/things/labels
	HttpRoute *labels_route = http_route_create (REQUEST_METHOD_GET, "labels", things_labels_handler);
	http_route_set_auth (labels_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (labels_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (things_route, labels_route);

	// POST api/things/labels
	http_route_set_handler (labels_route, REQUEST_METHOD_POST, things_label_create_handler);

	// GET api/things/labels/:id
	HttpRoute *single_label_route = http_route_create (REQUEST_METHOD_GET, "labels/:id", things_label_get_handler);
	http_route_set_auth (single_label_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (single_label_route, things_user_parse_from_json, things_user_delete);
	http_route_child_add (things_route, single_label_route);

	// POST api/things/labels/:id
	http_route_set_handler (single_label_route, REQUEST_METHOD_POST, things_label_update_handler);

	// DELETE api/things/labels/:id
	http_route_set_handler (single_label_route, REQUEST_METHOD_DELETE, things_label_delete_handler);

}

static void start (void) {

	things_api = cerver_create (
		CERVER_TYPE_WEB,
		"things-api",
		atoi (PORT->str),
		PROTOCOL_TCP,
		false,
		10,
		1000
	);

	if (things_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (things_api, CERVER_RECEIVE_BUFFER_SIZE);
		cerver_set_thpool_n_threads (things_api, CERVER_TH_THREADS);
		cerver_set_handler_type (things_api, CERVER_HANDLER_TYPE_THREADS);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) things_api->cerver_data;

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		http_cerver_auth_set_jwt_priv_key_filename (http_cerver, "keys/key.key");
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, "keys/key.pub");

		things_set_things_routes (http_cerver);

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
	signal (SIGINT, end);
	signal (SIGTERM, end);

	// to prevent SIGPIPE when writting to socket
	signal (SIGPIPE, SIG_IGN);

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