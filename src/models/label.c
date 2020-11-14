#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include "mongo.h"

#include "models/label.h"

#define LABELS_COLL_NAME         				"labels"

mongoc_collection_t *labels_collection = NULL;

// opens handle to labels collection
unsigned int labels_collection_get (void) {

	unsigned int retval = 1;

	labels_collection = mongo_collection_get (LABELS_COLL_NAME);
	if (labels_collection) {
		cerver_log_debug ("Opened handle to labels collection!");
		retval = 0;
	}

	else {
		cerver_log_error ("Failed to get handle to labels collection!");
	}

	return retval;

}

void labels_collection_close (void) {

	if (labels_collection) mongoc_collection_destroy (labels_collection);

}

void *label_new (void) {

	Label *label = (Label *) malloc (sizeof (Label));
	if (label) {
		(void) memset (label, 0, sizeof (Label));
	}

	return label;

}

void label_delete (void *label_ptr) {

	if (label_ptr) free (label_ptr);

}

void label_print (Label *label) {

	if (label) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&label->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		bson_oid_to_string (&label->category_oid, buffer);
		(void) printf ("category: %s\n", buffer);

		(void) printf ("title: %s\n", label->title);
		(void) printf ("description: %s\n", label->description);
		(void) printf ("color: %s\n", label->color);

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&label->date));
		(void) printf ("date: %s GMT\n", buffer);
	}

}