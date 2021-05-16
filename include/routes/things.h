#ifndef _THINGS_ROUTES_THINGS_H_
#define _THINGS_ROUTES_THINGS_H_

struct _HttpReceive;
struct _HttpResponse;

// GET /api/things/things
// get all the authenticated user's things
extern void things_things_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/things/things
// a user has requested to create a new thing
extern void things_thing_create_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/things/things/:id/info
// returns information about an existing thing that belongs to a user
extern void things_thing_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/things/things/:id/update
// a user wants to update an existing thing
extern void things_thing_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/things/things/:id/remove
// deletes an existing user's thing
extern void things_thing_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif