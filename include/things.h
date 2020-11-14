#ifndef _THINGS_H_
#define _THINGS_H_

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/handler.h>

#include <cerver/http/request.h>
#include <cerver/http/response.h>

#define DEFAULT_PORT					"5001"

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

#pragma endregion

#endif