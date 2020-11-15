#ifndef _THINGS_H_
#define _THINGS_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>
#include <cerver/http/response.h>

#define DEFAULT_PORT					"5001"

#define DEFAULT_THINGS_POOL_INIT		32

#pragma region main

extern const String *PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;

extern HttpResponse *oki_doki;
extern HttpResponse *bad_request;
extern HttpResponse *server_error;
extern HttpResponse *bad_user;
extern HttpResponse *missing_values;

// inits things main values
extern unsigned int things_init (void);

// ends things main values
extern unsigned int things_end (void);

#pragma endregion

#pragma region routes

// GET api/things
extern void things_handler (CerverReceive *cr, HttpRequest *request);

// GET api/things/version
extern void things_version_handler (CerverReceive *cr, HttpRequest *request);

// GET api/things/auth
extern void things_auth_handler (CerverReceive *cr, HttpRequest *request);

// GET api/things/categories
// get all the authenticated user's categories
extern void things_categories_handler (CerverReceive *cr, HttpRequest *request);

// POST api/things/categories
// a user has requested to create a new category
extern void things_category_create_handler (CerverReceive *cr, HttpRequest *request);

// GET api/things/categories/:id
// returns information about an existing category that belongs to a user
extern void things_category_get_handler (CerverReceive *cr, HttpRequest *request);

// POST api/things/categories/:id
// a user wants to update an existing category
extern void things_category_update_handler (CerverReceive *cr, HttpRequest *request);

// DELETE api/things/categories/:id
// deletes an existing user's category
extern void things_category_delete_handler (CerverReceive *cr, HttpRequest *request);

#pragma endregion

#endif