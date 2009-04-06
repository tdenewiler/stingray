#include <stdio.h>
#include <string.h>

#include "parser.h"


#ifndef DEFAULT_FILENAMES
#define DEFAULT_FILENAMES
#define STINGRAY_FILENAME	"../../../conf/stingray.conf"
#define GUI_FILENAME		"../../../conf/client.conf"
#endif /* DEFAULT_FILENAMES */


int main( int argc, char *argv[] )
{
	CONF_VARS config;
	memset( &config, 0, sizeof( CONF_VARS ) );

	/* Parse command line arguments. */
	default_config( &config );
	parse_cla( argc, argv, &config, STINGRAY, ( const char * )STINGRAY_FILENAME );

	printf( "server_IP = %s\n", config.server_IP );
	printf( "api_port = %hd\n", config.api_port );
	printf( "debug_level = %d\n", config.debug_level );
	printf( "pololu_port = %s\n", config.pololu_port );
	printf( "enable_net = %d\n", config.enable_net );
	printf( "net_mode = %d\n", config.net_mode );

	return 0;
} /* end main() */