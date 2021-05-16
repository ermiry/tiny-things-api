#ifndef _THINGS_ROUTES_CATEGORIES_H_
#define _THINGS_ROUTES_CATEGORIES_H_

struct _HttpReceive;
struct _HttpRequest;

// GET /api/things/categories
// get all the authenticated user's categories
extern void things_categories_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/things/categories
// a user has requested to create a new category
extern void things_category_create_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/things/categories/:id/info
// returns information about an existing category that belongs to a user
extern void things_category_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/things/categories/:id/update
// a user wants to update an existing category
extern void things_category_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/things/categories/:id/remove
// deletes an existing user's category
extern void things_category_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif