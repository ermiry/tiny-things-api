#ifndef _THINGS_ROUTES_LABELS_H_
#define _THINGS_ROUTES_LABELS_H_

struct _HttpReceive;
struct _HttpRequest;

// GET /api/things/labels
// get all the authenticated user's labels
extern void things_labels_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/things/labels
// a user has requested to create a new label
extern void things_label_create_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/things/labels/:id/info
// returns information about an existing label that belongs to a user
extern void things_label_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/things/labels/:id/update
// a user wants to update an existing label
extern void things_label_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/things/labels/:id/remove
// deletes an existing user's label
extern void things_label_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif