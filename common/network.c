/******************************************************************************
 *
 *  Title:        network.c
 *
 *  Description:  Handles setting up server and client network connections and
 *                network I/O.
 *
 *****************************************************************************/


#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>

#include "network.h"
#include "messages.h"


static int fdmax;
static fd_set master;
static fd_set read_fds;

static struct hostent *hent;


/******************************************************************************
 *
 * Title:       int net_server_setup( short port )
 *
 * Description: Creates a TCP server. Establishes a socket with file
 *              descriptor server_fd to be used in subsequent network calls.
 *
 * Input:       port: Port to bind.
 *
 * Output:      fd: File descriptor for the server.
 *
 * Globals:     read_fds, master, fdmax
 *
 *****************************************************************************/

int net_server_setup( short port )
{
    int fd;
    fd = -1;

    /* Zero out the file descriptor sets. Used to keep track of fd's available to
     *read data from. */
    FD_ZERO( &read_fds );
    FD_ZERO( &master );

    /* Make each system call. Error checking is done within the following
     *functions. */
    fd = net_socket( );
    net_setnonblock( &fd );
    net_setsockopt( &fd );
    net_bind( &fd, port );
    net_listen( &fd );
    net_sigaction( );

    /* Set the maximum file descriptor number to look for data to read. Also add
     * server_fd to the master set. The master set is necessary because read_fds
     * gets modified by the FD_ISSET() system call. */
    fdmax = fd;
    FD_SET( fd, &master );

    return fd;
} /* end net_server_setup() */


/******************************************************************************
 *
 * Title:       int net_client_setup( char *address, short port )
 *
 * Description: Creates socket connection between client and server.
 *              Establishes a socket with file descriptor server_fd to be used
 *              in subsequent network calls.
 *
 * Input:       address: A pointer to the IP address of the server.
 *              port: Port to connect to.
 *
 * Output:      fd: File descriptor for the client.
 *
 * Globals:     read_fds, master
 *
 *****************************************************************************/

int net_client_setup( char *address, short port )
{
    int fd;
    fd = -1;

    /* Make each system call. Error checking is done within the following
     * functions. */
    net_gethostbyname( address );
    fd = net_socket( );
    fd = net_connect( fd, port );

    return fd;
} /* end net_client_setup() */


/******************************************************************************
 *
 * Title:       int net_server( int fd,
 *                              void *buf,
 *                              MSG_DATA *msg,
 *                              int mode
 *                              )
 *
 * Description: Sends and receives data on network socket using TCP. Looks for
 *              new connections and adds them to the master fd set.
 *
 * Input:       fd: The file descriptor for the server.
 *              buf: A buffer for network data.
 *              msg: Pointer to message data.
 *              mode: Mode for the server to act in.
 *
 * Output:      recv_bytes: Number of bytes received.
 *
 * Globals:     read_fds, master, fdmax
 *
 *****************************************************************************/

int net_server( int fd, void *buf, MSG_DATA *msg, int mode )
{
    int ii;
    int new_fd;
    int recv_bytes = 0;

    struct timeval tv;

    /* This value is modified by the select() system call and must be zeroed out
     * every time. */
    tv.tv_sec = 0;
    tv.tv_usec = 1000;

    /* Copy the master set into the read_fds set as read_fds gets modified by
     * the FD_ISSET() system call. */
    memcpy( &read_fds, &master, sizeof( master ) );
    net_select( tv );

    /* For each socket with data available read that data and then send UUV
     * IMU data to it. */
    for ( ii = 0; ii <= fdmax; ii++ ) {
        if ( FD_ISSET( ii, &read_fds ) ) { /* Check for data on sockets. */
            if ( ii == fd ) { /* Check if it is remote connection. */
                new_fd = net_accept( fd ); /* Accept new connections. */
                FD_SET( new_fd, &master );
            }
            else {
                /* Get the data from the socket. */
                recv_bytes = net_recv( ii, buf );
                if ( recv_bytes == 0 ) { /* Connection lost. Close socket. */
                    net_close( ii );
                    FD_CLR( ii, &master );
                }
                else {
                    /* Send data to clients. */
                    if ( mode == MODE_NAV ) {
						messages_send( ii, STATUS_MSGID, msg );
                    }
                    else if ( mode == MODE_VISION ) {
                        messages_send( ii, VISION_MSGID, msg );
                    }
                    else if ( mode == MODE_PLANNER ) {
                        messages_send( ii, TARGET_MSGID, msg );
                    }
                    else if ( mode == MODE_LJ ) {
                        messages_send( ii, LJ_MSGID, msg );
                    }
                }
            }
        } /* end FD_ISSET */
    } /* end for() loop */

    return recv_bytes;
} /* end net_server() */


/******************************************************************************
 *
 * Title:       int net_client( int fd, void *buf, MSG_DATA *msg, int mode )
 *
 * Description: Sends and receives data on network socket using TCP.
 *
 * Input:       fd: A file descriptor for the client.
 *              buf: A buffer for network data.
 *              msg: A pointer to message data.
 *              mode: Mode for the client to act in.
 *
 * Output:      recv_bytes: Number of bytes received.
 *
 * Globals:     read_fds, master.
 *
 *****************************************************************************/

int net_client( int fd, void *buf, MSG_DATA *msg, int mode )
{
    int recv_bytes = 0;

    /* Send and receive data for connected socket. */
    if ( mode == MODE_STATUS ) {
        messages_send( fd, (int)STATUS_MSGID, msg );
    }
    else if ( mode == MODE_JOY ) {
        messages_send( fd, (int)TELEOP_MSGID, msg );
    }
    recv_bytes = net_recv( fd, buf );

    return recv_bytes;
} /* end net_client() */


/******************************************************************************
 *
 * Title:       void net_sigchld_handler( int socket )
 *
 * Description: Looks for a kill signal to reap dead processes.
 *
 * Input:       socket: File descriptor for server.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void net_sigchld_handler( int socket )
{
    while ( waitpid( -1, NULL, WNOHANG ) > 0 );
} /* end net_sigchld_handler() */


/******************************************************************************
 *
 * Title:       int net_socket( )
 *
 * Description: Sets up a socket for a file descriptor.
 *
 * Input:       None.
 *
 * Output:      fd: A network file descriptor.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int net_socket( )
{
    int fd;

    /* Create a socket for network communications. */
    if ( ( fd = socket( AF_INET,
                        SOCK_STREAM,
                        0 ) )
            == -1 ) {
        perror( "socket" );
    }

    return fd;
} /* end net_socket() */


/******************************************************************************
 *
 * Title:       int net_setsockopt( int *fd )
 *
 * Description: Sets the reuse socket option on a file descriptor.
 *
 * Input:       fd: A network file descriptor.
 *
 * Output:      -1 if error, else 0.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int net_setsockopt( int *fd )
{
    int yes = 1;

    /* Allow the socket to be reused for new connections. */
    if ( setsockopt( *fd,
                     SOL_SOCKET,
                     SO_REUSEADDR,
                     &yes,
                     sizeof( int ) )
            == -1 ) {
        perror( "setsockopt" );
        return -1;
    }

    return 0;
} /* end net_setsockopt() */


/******************************************************************************
 *
 * Title:       int net_setnonblock( int *fd )
 *
 * Description: Sets the non-blocking socket option on a file descriptor.
 *
 * Input:       None.
 *
 * Output:      -1 if error, else 0.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int net_setnonblock( int *fd )
{
    //int yes = 1;
    long arg;

    /* Make the socket non-blocking. */
    arg = fcntl( *fd, F_GETFL, NULL );
    arg |= O_NONBLOCK;
    fcntl( *fd, F_SETFL, arg );
    /*
        if( ioctl( *fd,
                   FIONBIO,
                   (char *)&yes )
                   == -1 ) {
            perror( "ioctl" );
            return -1;
        }
    */
    return 0;
} /* end net_setnonblock() */


/******************************************************************************
 *
 * Title:       void net_bind( int *fd, short port )
 *
 * Description: Binds a file descriptor to a port.
 *
 * Input:       fd: A network file descriptor.
 *              port: Port to bind.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void net_bind( int *fd, short port )
{

    struct sockaddr_in my_addr;

    /* Set socket parameters. */
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons( port );
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset( my_addr.sin_zero, '\0', sizeof( my_addr.sin_zero ) );

    /* Bind the socket with specific address and port. */

    if ( bind( *fd,
               ( struct sockaddr * )&my_addr,
               sizeof( struct sockaddr_in ) )
            == -1 ) {
        perror( "bind" );
    }
} /* end net_bind() */


/******************************************************************************
 *
 * Title:       void net_listen( int *fd )
 *
 * Description: Sets up a port for listening for new connections. Don't accept
 *              any more than MAX_CLIENTS.
 *
 * Input:       fd: A pointer to a network file descriptor.
 *
 * Output:      None.
 *
 * Globals:     master, fdmax
 *
 *****************************************************************************/

void net_listen( int *fd )
{
    if ( listen( *fd,
                 MAX_CLIENTS )
            == -1 ) {
        perror( "listen" );
    }

    /* Add new connection to the master set. */
    FD_SET( *fd, &master );

    fdmax = *fd;
} /* end net_listen() */


/******************************************************************************
 *
 * Title:       void net_sigaction( )
 *
 * Description: Sets the action to take when catching a signal. select()
 *              catches the signal.
 *
 * Input:       None.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void net_sigaction( )
{

    struct sigaction sa;

    sa.sa_handler = net_sigchld_handler;
    sigemptyset( &sa.sa_mask );
    sa.sa_flags = SA_RESTART;

    if ( sigaction( SIGCHLD,
                    &sa,
                    NULL )
            == -1 ) {
        perror( "sigaction" );
    }
} /* end net_sigaction() */


/******************************************************************************
 *
 * Title:       int net_accept( int fd_orig )
 *
 * Description: Accepts a new connection and creates a new file descriptor.
 *
 * Input:       fd_orig: The original network file descriptor.
 *
 * Output:      fd_ret: The modified file descriptor.
 *
 * Globals:     fdmax
 *
 *****************************************************************************/

int net_accept( int fd_orig )
{

    struct sockaddr_in their_addr;
    socklen_t addr_len;
    int fd_ret;

    addr_len = sizeof( struct sockaddr_in );
    if ( ( fd_ret = accept( fd_orig,
                            ( struct sockaddr * ) & their_addr,
                            &addr_len ) )
            == -1 ) {
        perror( "accept" );
    }
    else {
        if ( fd_ret > fdmax ) {
            fdmax = fd_ret;
        }
    }

    return fd_ret;
} /* end net_accept() */


/******************************************************************************
 *
 * Title:       void net_close( int fd )
 *
 * Description: Closes a file descriptor.
 *
 * Input:       fd: A network file descriptor.
 *
 * Output:      None.
 *
 * Globals:     None.
 *
 *****************************************************************************/

void net_close( int fd )
{
    close( fd );
} /* end net_close() */


/******************************************************************************
 *
 * Title:       int net_select( struct timeval *tv )
 *
 * Description: Determines whether data is available on network stack. The
 *              global variables read_fds and tv are modified here by the
 *              select() system call.
 *
 * Input:       tv: A timeval struct that is modified by select().
 *
 * Output:      retval: Fills in read_fds struct with file descriptors waiting
 *                      to be read from using select() system call.
 *
 * Globals:     fdmax, read_fds
 *
 *****************************************************************************/

int net_select( struct timeval tv )
{
    int retval = 0;

    if ( ( retval = select( fdmax + 1,
                            &read_fds,
                            NULL,
                            NULL,
                            &tv )
                    == -1 ) ) {
        perror( "select" );
    }

    return retval;
} /* end net_select() */


/******************************************************************************
 *
 * Title:       void net_gethostbyname( char *address )
 *
 * Description: Gets the host network information.
 *
 * Input:       address: A pointer to the an IP address.
 *
 * Output:      None.
 *
 * Globals:     hent
 *
 *****************************************************************************/

void net_gethostbyname( char *address )
{
    if ( ( hent = gethostbyname( address ) ) == NULL ) {
        herror( "gethostbyname" );
    }
} /* end net_gethostbyname() */


/******************************************************************************
 *
 * Title:       int net_connect( int fd, short port )
 *
 * Description: Connects a client TCP port to a server TCP port.
 *
 * Input:       fd: A pointer to a network file descriptor.
 *              port: Port to connect to.
 *
 * Output:      fd on success, -1 on failure.
 *
 * Globals:     hent
 *
 *****************************************************************************/

int net_connect( int fd, short port )
{

    struct sockaddr_in their_addr;

    /* Set socket parameters. */
    memset( &their_addr, 0, sizeof( struct sockaddr_in ) );
    their_addr.sin_family = AF_INET;
    their_addr.sin_addr = *( ( struct in_addr * )hent->h_addr );
    their_addr.sin_port = htons( port );

    if ( connect( fd,
                  ( struct sockaddr * )&their_addr,
                  sizeof( struct sockaddr_in ) )
            == -1 ) {
        perror( "connect" );
        net_close( fd );
        return -1;
    }

    return fd;
} /* end net_connect() */


/******************************************************************************
 *
 * Title:       int net_send( int fd, const void *msg, int len )
 *
 * Description: Sends data on network socket using TCP.
 *
 * Input:       fd: A network file descriptor.
 *              msg: The message to send.
 *              len: The message length.
 *
 * Output:      send_bytes: The number of bytes sent.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int net_send( int fd, const void *msg, int len )
{
    int send_bytes = 0;

    if ( ( send_bytes = send( fd,
                              msg,
                              len,
                              0 ) )
            == -1 ) {
        perror( "send" );
    }

    return send_bytes;
} /* end net_send() */


/******************************************************************************
 *
 * Title:       int net_recv( int fd, void *buf )
 *
 * Description: Receives data on network socket using TCP.
 *
 * Input:       fd: A file descriptor for the network connection.
 *              buf: A buffer for network data.
 *
 * Output:      recv_bytes: The number of bytes received.
 *
 * Globals:     None.
 *
 *****************************************************************************/

int net_recv( int fd, void *buf )
{
    int recv_bytes;

    if ( ( recv_bytes = recv( fd,
                              buf,
                              MAX_MSG_SIZE,
                              0 ) )
            == -1 ) {
        if ( errno != EWOULDBLOCK ) {
            perror( "recv" );
            return 0;
        }
    }

    return recv_bytes;
} /* end net_recv() */
