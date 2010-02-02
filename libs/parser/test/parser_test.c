#include <stdio.h>
#include <string.h>

#include "parser.h"


#ifndef DEFAULT_FILENAMES
#define DEFAULT_FILENAMES
#define STINGRAY_FILENAME	"../../../conf/nav.conf"
#endif /* DEFAULT_FILENAMES */


int main( int argc, char *argv[] )
{
	/// Declare variables.
	CONF_VARS cf;
	memset(&cf, 0, sizeof(CONF_VARS));

	/// Parse command line arguments.
	printf("MAIN: Trying configuration file %s\n\n", STINGRAY_FILENAME);
	parse_default_config(&cf);
	parse_cla(argc, argv, &cf, STINGRAY, (const char *)STINGRAY_FILENAME);

	/// Print out the configuration values.
	parse_print_config(&cf);

	return 0;
} /* end main() */
