#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/thing.h"

#define THINGS_COLL_NAME         				"things"

mongoc_collection_t *things_collection = NULL;

// opens handle to thing collection
unsigned int things_collection_get (void) {

	unsigned int retval = 1;

	things_collection = mongo_collection_get (THINGS_COLL_NAME);
	if (things_collection) {
		cerver_log_debug ("Opened handle to things collection!");
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to things collection!");
	}

	return retval;

}

void things_collection_close (void) {

	if (things_collection) mongoc_collection_destroy (things_collection);

}

const char *things_status_to_string (ThingStatus status) {

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

void thing_print (Thing *thing) {

	if (thing) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&thing->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		(void) printf ("title: %s\n", thing->title);
		(void) printf ("description: %s\n", thing->description);

		(void) printf ("status: %s\n", things_status_to_string (thing->status));

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&thing->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}