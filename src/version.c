#include <cerver/utils/log.h>

#include "version.h"

// print full things version information
void things_version_print_full (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Things Version: %s", THINGS_VERSION_NAME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Release Date & time: %s - %s", THINGS_VERSION_DATE, THINGS_VERSION_TIME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Author: %s\n", THINGS_VERSION_AUTHOR
	);

}

// print the version id
void things_version_print_version_id (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Things Version ID: %s", THINGS_VERSION
	);

}

// print the version name
void things_version_print_version_name (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Tiny Things Version: %s", THINGS_VERSION_NAME
	);

}