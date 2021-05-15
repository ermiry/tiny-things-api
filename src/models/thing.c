#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/thing.h"

static CMongoModel *things_model = NULL;

static void user_doc_parse (
	void *user_ptr, const bson_t *user_doc
);

unsigned int things_model_init (void) {

	unsigned int retval = 1;

	things_model = cmongo_model_create (THINGS_COLL_NAME);
	if (things_model) {
		retval = 0;
	}

	return retval;

}

void things_model_end (void) {

	cmongo_model_delete (things_model);

}

const char *things_status_to_string (const ThingStatus status) {

	switch (status) {
		#define XX(num, name, string) case THING_STATUS_##name: return #string;
		THING_STATUS_MAP(XX)
		#undef XX
	}

	return things_status_to_string (THING_STATUS_NONE);

}

void *thing_new (void) {

	Thing *thing = (Thing *) malloc (sizeof (Thing));
	if (thing) {
		(void) memset (thing, 0, sizeof (Thing));
	}

	return thing;

}

void thing_delete (void *thing_ptr) {

	if (thing_ptr) free (thing_ptr);

}

void thing_print (const Thing *thing) {

	if (thing) {
		(void) printf ("id: %s\n", thing->id);

		(void) printf ("title: %s\n", thing->title);
		(void) printf ("description: %s\n", thing->description);

		(void) printf ("status: %s\n", things_status_to_string (thing->status));

		char buffer[128] = { 0 };
		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&thing->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}
