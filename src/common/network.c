/******************************************************************************
 *
 *  Title:        network.c
 *
 *  Description:  Handles setting up server and client network connections and
 *                network I/O.
 *
 *****************************************************************************/

#include "network.h"

static int fdmax;
static fd_set master;
static fd_set read_fds;
static struct hostent *hent;

/* This variable is in here as a hack. Planner is supposed to send 2 messages
 * to GUI but only 1 is making it through most of the time. Rarely the second
 * message gets to GUI as well. This will send STATUS one time, LJ the next.
 * messages_decode() should fix this but it's not working yet. */
static int hack_msg_num;
static int hack_msg_num_client;


/*------------------------------------------------------------------------------
 * int net_server_setup()
 * Creates a TCP server. Establishes a socket with file descriptor server_fd to
 * be used in subsequent network calls.
 *----------------------------------------------------------------------------*/

int net_server_setup( short port )
{
    int fd = -1;

    /// Zero out the file descriptor sets. Used to keep track of fd's available to
    /// read data from. Basically a list of clients that are connected.
    FD_ZERO( &read_fds );
    FD_ZERO( &master );

    /// Make each system call. Error checking is done within the following functions.
    fd = net_socket( );
    net_setnonblock( &fd );
    net_setsockopt( &fd );
    net_bind( &fd, port );
    net_listen( &fd );
    net_sigaction( );

    /// Set the maximum file descriptor number to look for data to read. Also add
    /// fd to the master set. The master set is necessary because read_fds
    /// gets modified by the FD_ISSET() system call inside the net_server() function.
    fdmax = fd;
    FD_SET( fd, &master );

    return fd;
} /* end net_server_setup() */


/*------------------------------------------------------------------------------
 * int net_client_setup()
 * Creates socket connection between client and server. Establishes a socket with
 * file descriptor server_fd to be used in subsequent network calls.
 *----------------------------------------------------------------------------*/

int net_client_setup( char *address, short port )
{
    int fd = -1;

    /// Make each system call. Error checking is done within the following functions.
    net_gethostbyname( address );
    fd = net_socket( );
    fd = net_connect( fd, port );

    return fd;
} /* end net_client_setup() */


/*------------------------------------------------------------------------------
 * int net_server()
 * Sends and receives data on network socket using TCP. Looks for new connections
 * and adds them to the master fd set.
 *----------------------------------------------------------------------------*/

int net_server( int fd, void *buf, MSG_DATA *msg, int mode )
{
	/// Declare variables.
    int ii;
    int new_fd;
    int recv_bytes = 0;

    struct timeval tv;

    /// This value is modified by the select() system call and must be zeroed out every time.
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    /// Copy the master set into the read_fds set as read_fds gets modified by
    /// the FD_ISSET() system call. */
    memcpy( &read_fds, &master, sizeof(master) );
    net_select( tv );

    /// For each socket with data available read that data and then send UUV IMU data to it.
    for (ii = 0; ii <= fdmax; ii++) {
        if (FD_ISSET(ii, &read_fds)) { /// Check for data on sockets.
            if (ii == fd) { /// Check if it is remote connection.
                new_fd = net_accept( fd ); /// Accept new connections.
                FD_SET( new_fd, &master );
            }
            else {
                /// Get the data from the socket.
                recv_bytes = net_recv( ii, buf );
                if (recv_bytes == 0) {
					/// Connection lost. Close socket.
                    net_close(ii);
                    FD_CLR(ii, &master);
                }
                else {
                    /// Send data to clients.
                    if (mode == MODE_STATUS) {
                    	messages_send( ii, STATUS_MSGID, msg );
					}
                    else if (mode == MODE_VISION) {
                        messages_send( ii, VISION_MSGID, msg );
                    }
                    else if (mode == MODE_LJ) {
                        messages_send( ii, LJ_MSGID, msg );
                    }
					/* This is a hack. Both messages should be sent at once.
					 * messages_decode() should fix this but it's not working
					 * yet. */
                    else if (mode == MODE_PLANNER) {
						if (hack_msg_num == 1) {
							messages_send( ii, LJ_MSGID, msg );
							hack_msg_num = 2;
						}
						else {
							messages_send( ii, STATUS_MSGID, msg );
							hack_msg_num = 1;
						}
                    }
                }
            }
        } /// end FD_ISSET
    } /// end for() loop

    return recv_bytes;
} /* end net_server() */


/*------------------------------------------------------------------------------
 * int net_client()
 * Sends and receives data on network socket using TCP.
 *----------------------------------------------------------------------------*/

int net_client( int fd, void *buf, MSG_DATA *msg, int mode )
{
	/// Declare variables.
    int recv_bytes = 0;

    /// Send and receive data for connected socket.
    if (mode == MODE_STATUS) {
        messages_send( fd, (int)STATUS_MSGID, msg );
    }
    else if (mode == MODE_JOY) {
        messages_send( fd, (int)TELEOP_MSGID, msg );
    }
    else if (mode == MODE_PLANNER) {
    	if( hack_msg_num_client == 1 ) {
			messages_send( fd, (int)TARGET_MSGID, msg );
			hack_msg_num_client = 2;
			usleep(1000);
		}
		else if (hack_msg_num_client == 2) {
			messages_send( fd, (int)GAIN_MSGID, msg );
			hack_msg_num_client = 3;
			usleep(1000);
		}
		else {
	    	messages_send( fd, (int)LJ_MSGID, msg );
	    	hack_msg_num_client = 1;
			usleep(1000);
		}
	}
	else if (mode == MODE_OPEN) {
		messages_send( fd, (int)OPEN_MSGID, msg );
	}
    recv_bytes = net_recv( fd, buf );

    return recv_bytes;
} /* end net_client() */


/*------------------------------------------------------------------------------
 * void net_sigchld_handler()
 * Looks for a kill signal to reap dead processes.
 *----------------------------------------------------------------------------*/

void net_sigchld_handler( int socket )
{
    while (waitpid( -1, NULL, WNOHANG) > 0 );
} /* end net_sigchld_handler() */


/*------------------------------------------------------------------------------
 * int net_socket()
 * Sets up a socket for a file descriptor.
 *----------------------------------------------------------------------------*/

int net_socket( )
{
    int fd;

    /// Create a socket for network communications.
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
    }

    return fd;
} /* end net_socket() */


/*------------------------------------------------------------------------------
 * int net_setsockopt()
 * Sets the reuse socket option on a file descriptor.
 *----------------------------------------------------------------------------*/

int net_setsockopt( int *fd )
{
    int yes = 1;

    /// Allow the socket to be reused for new connections.
    if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        return -1;
    }

    return 0;
} /* end net_setsockopt() */


/*------------------------------------------------------------------------------
 * int net_setnonblock()
 * Sets the non-blocking socket option on a file descriptor.
 *----------------------------------------------------------------------------*/

int net_setnonblock( int *fd )
{
    long arg;

    /// Make the socket non-blocking.
    arg = fcntl(*fd, F_GETFL, NULL);
    arg |= O_NONBLOCK;
    fcntl(*fd, F_SETFL, arg);

    return 0;
} /* end net_setnonblock() */


/*------------------------------------------------------------------------------
 * void net_bind()
 * Binds a file descriptor to a port.
 *----------------------------------------------------------------------------*/

void net_bind( int *fd, short port )
{
    struct sockaddr_in my_addr;

    /// Set socket parameters.
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset( my_addr.sin_zero, '\0', sizeof(my_addr.sin_zero) );

    /// Bind the socket with specific address and port.

    if (bind(*fd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("bind");
    }
} /* end net_bind() */


/*------------------------------------------------------------------------------
 * void net_listen()
 * Sets up a port for listening for new connections. Don't accept any more than NET_MAX_CLIENTS.
 *----------------------------------------------------------------------------*/

void net_listen( int *fd )
{
    if (listen(*fd, NET_MAX_CLIENTS) == -1) {
        perror("listen");
    }

    /// Add new connection to the master set.
    FD_SET(*fd, &master);

    fdmax = *fd;
} /* end net_listen() */


/*------------------------------------------------------------------------------
 * void net_sigaction()
 * Sets the action to take when catching a signal. select() catches the signal.
 *----------------------------------------------------------------------------*/

void net_sigaction()
{
    struct sigaction sa;

    sa.sa_handler = net_sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
    }
} /* end net_sigaction() */


/*------------------------------------------------------------------------------
 * int net_accept()
 * Accepts a new connection and creates a new file descriptor.
 *----------------------------------------------------------------------------*/

int net_accept( int fd_orig )
{
    struct sockaddr_in their_addr;
    socklen_t addr_len;
    int fd_ret;

    addr_len = sizeof(struct sockaddr_in);
    if ((fd_ret = accept(fd_orig, (struct sockaddr *) & their_addr, &addr_len)) == -1) {
        perror("accept");
    }
    else {
        if (fd_ret > fdmax) {
            fdmax = fd_ret;
        }
    }

    return fd_ret;
} /* end net_accept() */


/*------------------------------------------------------------------------------
 * void net_close()
 * Closes a file descriptor.
 *----------------------------------------------------------------------------*/

void net_close( int fd )
{
    close(fd);
} /* end net_close() */


/*------------------------------------------------------------------------------
 * int net_select()
 * Determines whether data is available on network stack. The global variables
 * read_fds and tv are modified here by the select() system call.
 *----------------------------------------------------------------------------*/

int net_select( struct timeval tv )
{
    int retval = 0;

    if ((retval = select(fdmax + 1, &read_fds, NULL, NULL, &tv) == -1)) {
        perror("select");
    }

    return retval;
} /* end net_select() */


/*------------------------------------------------------------------------------
 * void net_gethostbyname()
 * Gets the host network information.
 *----------------------------------------------------------------------------*/

void net_gethostbyname( char *address )
{
    if ((hent = gethostbyname(address)) == NULL) {
        herror("gethostbyname");
    }
} /* end net_gethostbyname() */


/*------------------------------------------------------------------------------
 * int net_connect()
 * Connects a client TCP port to a server TCP port.
 *----------------------------------------------------------------------------*/

int net_connect( int fd, short port )
{
    struct sockaddr_in their_addr;

    /// Set socket parameters.
    memset( &their_addr, 0, sizeof(struct sockaddr_in) );
    their_addr.sin_family = AF_INET;
    their_addr.sin_addr = *((struct in_addr *)hent->h_addr);
    their_addr.sin_port = htons(port);

    if (connect(fd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("connect");
        net_close( fd );
        return -1;
    }

    return fd;
} /* end net_connect() */


/*------------------------------------------------------------------------------
 * int net_send()
 * Sends data on network socket using TCP.
 *----------------------------------------------------------------------------*/

int net_send( int fd, const void *msg, int len )
{
    int send_bytes = 0;

    if ((send_bytes = send(fd, msg, len, 0)) == -1) {
        perror("send");
    }

    return send_bytes;
} /* end net_send() */


/*------------------------------------------------------------------------------
 * int net_recv()
 * Receives data on network socket using TCP.
 *----------------------------------------------------------------------------*/

int net_recv( int fd, void *buf )
{
    int recv_bytes;

    if ((recv_bytes = recv(fd, buf, MAX_MSG_SIZE, 0)) == -1) {
        if (errno != EWOULDBLOCK) {
            perror("recv");
            return 0;
        }
    }

    return recv_bytes;
} /* end net_recv() */
