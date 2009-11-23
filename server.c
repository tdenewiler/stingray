/*******************************************************************************
 *
 *  Title:        server.c
 *
 *  Description:  A test program for network functionality.
 *
 ******************************************************************************/

#include "server.h"

/// Global file descriptors. Only global so that server_exit() can close them.
int server_fd;


/*------------------------------------------------------------------------------
 * int server_sigint()
 * Called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void server_sigint( int signal )
{
	exit( 0 );
} /* end server_sigint() */


/*------------------------------------------------------------------------------
 * int server_exit()
 * Exit function for main program. Closes all file descriptors. This function
 * is called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void server_exit( )
{
	printf("SERVER_EXIT: Shutting down network test program ... ");
	/// Sleep to let things shut down properly.
	usleep( 200000 );

	/// Close the open file descriptors.
	if (server_fd > 0) {
		close(server_fd);
	}

	printf("<OK>\n\n");
} /* end server_exit() */


/*------------------------------------------------------------------------------
 * int main()
 * Initialize data. Open ports. Run main program loop.
 *----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
	/// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
	void(*exit_ptr)(void);
	exit_ptr = server_exit;
	atexit(exit_ptr);

	struct sigaction sigint_action;
	sigint_action.sa_handler = server_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	/// Declare variables.
	int recv_bytes = 0;
	char server_buf[MAX_MSG_SIZE];
	MSG_DATA msg;
	int server_port = 2222;

	/// Declare timers.
	TIMING timer_server;

	printf("MAIN: Starting Server Test Program ... \n");

	/// Initialize variables.
	server_fd = -1;

	memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

	/// Set up server.
	server_fd = net_server_setup( server_port );
	if( server_fd > 0 ) {
		printf("MAIN: Server setup OK.\n");
	}
	else {
		printf("MAIN: WARNING!!! Server setup failed.\n");
	}

	/// Initialize timers.
	timing_set_timer(&timer_server);

	printf("MAIN: Server test running now.\n\n");

	/// Main loop.
	while (1) {
		/// Get network data.
		if (server_fd > 0) {
			recv_bytes = net_server( server_fd, server_buf, &msg, MODE_PLANNER );
			if (recv_bytes > 0) {
				server_buf[recv_bytes] = '\0';
				messages_decode( server_fd, server_buf, &msg, recv_bytes );
			}
		}

		/// Update the timers.
		timing_set_timer(&timer_server);
	}

	exit( 0 );
} /* end main() */
