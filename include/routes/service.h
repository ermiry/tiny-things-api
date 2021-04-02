#ifndef _THINGS_ROUTES_SERVICE_H_
#define _THINGS_ROUTES_SERVICE_H_

struct _HttpReceive;
struct _HttpRequest;


// GET /api/things
extern void things_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/things/version
extern void things_version_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/things/auth
extern void things_auth_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET *
extern void things_catch_all_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif