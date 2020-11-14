#include <cerver/utils/log.h>

#include "version.h"

// print full things version information
void things_version_print_full (void) {

	cerver_log_msg ("\n\nTiny Things Version: %s\n", THINGS_VERSION_NAME);
	cerver_log_msg ("Release Date & time: %s - %s\n", THINGS_VERSION_DATE, THINGS_VERSION_TIME);
	cerver_log_msg ("Author: %s\n\n", THINGS_VERSION_AUTHOR);

}

// print the version id
void things_version_print_version_id (void) {

	cerver_log_msg ("\n\nTiny Things Version ID: %s\n", THINGS_VERSION);

}

// print the version name
void things_version_print_version_name (void) {

	cerver_log_msg ("\n\nTiny Things Version: %s\n", THINGS_VERSION_NAME);

}