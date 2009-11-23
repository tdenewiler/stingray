/*******************************************************************************
 *
 *  Title:        client.c
 *
 *  Description:  A test program for network functionality.
 *
 ******************************************************************************/

#include "client.h"

/// Global file descriptors. Only global so that client_exit() can close them.
int client_fd;


/*------------------------------------------------------------------------------
 * int client_sigint()
 * Called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void client_sigint( int signal )
{
	exit( 0 );
} /* end client_sigint() */


/*------------------------------------------------------------------------------
 * int client_exit()
 * Exit function for main program. Closes all file descriptors. This function
 * is called when SIGINT (ctrl-c) is invoked.
 *----------------------------------------------------------------------------*/

void client_exit( )
{
	printf("CLIENT_EXIT: Shutting down network test program ... ");
	/// Sleep to let things shut down properly.
	usleep( 200000 );

	/// Close the open file descriptors.
	if (client_fd > 0) {
		close(client_fd);
	}

	printf("<OK>\n\n");
} /* end test_exit() */


/*------------------------------------------------------------------------------
 * int main()
 * Initialize data. Open ports. Run main program loop.
 *----------------------------------------------------------------------------*/

int main( int argc, char *argv[] )
{
	/// Setup exit function. It is called when SIGINT (ctrl-c) is invoked.
	void( *exit_ptr )( void );
	exit_ptr = client_exit;
	atexit( exit_ptr );

	struct sigaction sigint_action;
	sigint_action.sa_handler = client_sigint;
	sigint_action.sa_flags = 0;
	sigaction( SIGINT, &sigint_action, NULL );

	/// Declare variables.
	int recv_bytes = 0;
	char client_buf[MAX_MSG_SIZE];
	MSG_DATA msg;
	
	int server_port = 2222;
	char server_IP[STRING_SIZE] = "127.0.0.1";

	/// Declare timers.
	TIMING timer_client;

	printf("MAIN: Starting Client Test Program ... \n");

	/// Initialize variables.
	client_fd = -1;

	memset( &msg, 0, sizeof(MSG_DATA) );
	messages_init( &msg );

    /// Set up the client network client.
	client_fd = net_client_setup( server_IP, server_port );
	if( client_fd > 0 ) {
		printf("MAIN: Client setup OK.\n");
	}
	else {
		printf("MAIN: WARNING!!! Client setup failed.\n");
	}

	/// Initialize timers.
	timing_set_timer(&timer_client);

	printf("MAIN: Client test running now.\n\n");

	/// Main loop.
	while (1) {
        /// Get client data.
		if (client_fd > 0) {
			recv_bytes = net_client( client_fd, client_buf, &msg, MODE_PLANNER );
			client_buf[recv_bytes] = '\0';
			if (recv_bytes > 0) {
				messages_decode( client_fd, client_buf, &msg, recv_bytes );
				printf("MAIN: Got %d bytes.\n", recv_bytes);
			}
		}

		/// Update the timers.
		timing_set_timer(&timer_client);
	}

	exit( 0 );
} /* end main() */
