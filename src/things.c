#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/utils.h>
#include <cerver/utils/log.h>

#include "things.h"
#include "mongo.h"
#include "roles.h"
#include "version.h"

#include "models/action.h"
#include "models/role.h"
#include "models/user.h"

#pragma region main

const String *PORT = NULL;

static const String *MONGO_URI = NULL;
static const String *MONGO_APP_NAME = NULL;
static const String *MONGO_DB = NULL;

unsigned int CERVER_RECEIVE_BUFFER_SIZE = 4096;
unsigned int CERVER_TH_THREADS = 4;

HttpResponse *oki_doki = NULL;
HttpResponse *bad_request = NULL;
HttpResponse *server_error = NULL;
HttpResponse *bad_user = NULL;
HttpResponse *missing_values = NULL;

static HttpResponse *things_works = NULL;
static HttpResponse *current_version = NULL;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

static unsigned int things_env_get_port (void) {
	
	unsigned int retval = 1;

	char *port_env = getenv ("PORT");
	if (port_env) {
		PORT = str_new (port_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get PORT from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_app_name (void) {

	unsigned int retval = 1;

	char *mongo_app_name_env = getenv ("MONGO_APP_NAME");
	if (mongo_app_name_env) {
		MONGO_APP_NAME = str_new (mongo_app_name_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_APP_NAME from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_db (void) {

	unsigned int retval = 1;

	char *mongo_db_env = getenv ("MONGO_DB");
	if (mongo_db_env) {
		MONGO_DB = str_new (mongo_db_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_DB from env!");
	}

	return retval;

}

static unsigned int things_env_get_mongo_uri (void) {

	unsigned int retval = 1;

	char *mongo_uri_env = getenv ("MONGO_URI");
	if (mongo_uri_env) {
		MONGO_URI = str_new (mongo_uri_env);

		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get MONGO_URI from env!");
	}

	return retval;

}

static void gepp_env_get_cerver_receive_buffer_size (void) {

	char *buffer_size = getenv ("CERVER_RECEIVE_BUFFER_SIZE");
	if (buffer_size) {
		CERVER_RECEIVE_BUFFER_SIZE = (unsigned int) atoi (buffer_size);
		cerver_log_success (
			"CERVER_RECEIVE_BUFFER_SIZE -> %d\n", CERVER_RECEIVE_BUFFER_SIZE
		);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_RECEIVE_BUFFER_SIZE from env - using default %d!",
			CERVER_RECEIVE_BUFFER_SIZE
		);
	}
}

static void gepp_env_get_cerver_th_threads (void) {

	char *th_threads = getenv ("CERVER_TH_THREADS");
	if (th_threads) {
		CERVER_TH_THREADS = (unsigned int) atoi (th_threads);
		cerver_log_success ("CERVER_TH_THREADS -> %d\n", CERVER_TH_THREADS);
	}

	else {
		cerver_log_warning (
			"Failed to get CERVER_TH_THREADS from env - using default %d!",
			CERVER_TH_THREADS
		);
	}

}

#pragma GCC diagnostic pop

static unsigned int things_init_env (void) {

	unsigned int errors = 0;

	errors |= things_env_get_port ();

	errors |= things_env_get_mongo_uri ();

	errors |= things_env_get_mongo_app_name ();

	errors |= things_env_get_mongo_db ();

	return errors;

}

static unsigned int things_mongo_connect (void) {

	unsigned int errors = 0;

	bool connected_to_mongo = false;

	mongo_set_uri (MONGO_URI->str);
	mongo_set_app_name (MONGO_APP_NAME->str);
	mongo_set_db_name (MONGO_DB->str);

	if (!mongo_connect ()) {
		// test mongo connection
		if (!mongo_ping_db ()) {
			cerver_log_success ("Connected to Mongo DB!");

			// open handle to actions collection
			errors |= actions_collection_get ();

			// open handle to roles collection
			errors |= roles_collection_get ();

			// open handle to users collection
			errors |= users_collection_get ();

			connected_to_mongo = true;
		}
	}

	if (!connected_to_mongo) {
		cerver_log_error ("Failed to connect to mongo!");
		errors |= 1;
	}

	return errors;

}

static unsigned int things_mongo_init (void) {

	unsigned int retval = 1;

	if (!things_mongo_connect ()) {
		if (!things_roles_init ()) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to get roles from db!");
		}
	}

	return retval;

}

static unsigned int things_init_responses (void) {

	unsigned int retval = 1;

	oki_doki = http_response_json_key_value (
		(http_status) 200, "oki", "doki"
	);

	bad_request = http_response_json_key_value (
		(http_status) 400, "error", "Bad request!"
	);

	server_error = http_response_json_key_value (
		(http_status) 500, "error", "Internal server error!"
	);

	bad_user = http_response_json_key_value (
		(http_status) 400, "error", "Bad user!"
	);

	missing_values = http_response_json_key_value (
		(http_status) 400, "error", "Missing values!"
	);

	things_works = http_response_json_key_value (
		(http_status) 200, "msg", "Pocket works!"
	);

	char *status = c_string_create ("%s - %s", THINGS_VERSION_NAME, THINGS_VERSION_DATE);
	if (status) {
		current_version = http_response_json_key_value (
			(http_status) 200, "version", status
		);

		free (status);
	}

	if (
		oki_doki && bad_request && server_error && bad_user && missing_values
		&& things_works && current_version
	) retval = 0;

	return retval;

}

// inits things main values
unsigned int things_init (void) {

	unsigned int errors = 0;

	if (!things_init_env ()) {
		errors |= things_mongo_init ();

		errors |= things_users_init ();

		errors |= things_init_responses ();
	}

	return errors; 

}

static unsigned int things_mongo_end (void) {

	if (mongo_get_status () == MONGO_STATUS_CONNECTED) {
		actions_collection_close ();

		roles_collection_close ();

		users_collection_close ();

		mongo_disconnect ();
	}

	return 0;

}

// ends things main values
unsigned int things_end (void) {

	unsigned int errors = 0;

	errors |= things_mongo_end ();

	things_roles_end ();

	things_users_end ();

	http_respponse_delete (oki_doki);
	http_respponse_delete (bad_request);
	http_respponse_delete (server_error);
	http_respponse_delete (bad_user);
	http_respponse_delete (missing_values);

	http_respponse_delete (things_works);
	http_respponse_delete (current_version);

	str_delete ((String *) MONGO_URI);
	str_delete ((String *) MONGO_APP_NAME);
	str_delete ((String *) MONGO_DB);

	return errors;

}

#pragma endregion

#pragma region routes

// GET api/things
void things_handler (CerverReceive *cr, HttpRequest *request) {

	(void) http_response_send (things_works, cr->cerver, cr->connection);

}

// GET api/things/version
void things_version_handler (CerverReceive *cr, HttpRequest *request) {

	(void) http_response_send (current_version, cr->cerver, cr->connection);

}

// GET api/things/auth
void things_auth_handler (CerverReceive *cr, HttpRequest *request) {

	User *user = (User *) request->decoded_data;

	if (user) {
		#ifdef POCKET_DEBUG
		user_print (user);
		#endif

		(void) http_response_send (oki_doki, cr->cerver, cr->connection);
	}

	else {
		(void) http_response_send (bad_user, cr->cerver, cr->connection);
	}

}

#pragma endregion