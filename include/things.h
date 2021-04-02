#ifndef _THINGS_H_
#define _THINGS_H_

#include <stdbool.h>

#include "runtime.h"

#define DEFAULT_PORT					"5002"

struct _HttpResponse;

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const String *PRIV_KEY;
extern const String *PUB_KEY;

extern bool ENABLE_USERS_ROUTES;

// inits things main values
extern unsigned int things_init (void);

// ends things main values
extern unsigned int things_end (void);

#endif