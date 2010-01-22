/**
 *  \file network.h
 *  \brief Handles setting up server and client network connections and network
 *         I/O.
 */

#ifndef _NETWORK_H_
#define _NETWORK_H_

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

#include "messages.h"
#include "msgtypes.h"

/******************************
**
** #defines
**
******************************/

#ifndef STRING_SIZE
#define STRING_SIZE 64
#endif /* STRING_SIZE */

#ifndef MAX_MSG_SIZE
#define MAX_MSG_SIZE 65536
#endif /* MAX_MSG_SIZE */

#ifndef NET_MAX_CLIENTS
#define NET_MAX_CLIENTS 10
#endif /* NET_MAX_CLIENTS */

#ifndef NET_MODES
#define NET_MODES
#define MODE_NAV		1
#define MODE_VISION		2
#define MODE_PLANNER	3
#define MODE_LJ			4
#define MODE_STATUS		5
#define MODE_JOY		6
#define MODE_OPEN		7
#endif /* NET_MODES */


/******************************
**
** Data types
**
******************************/



/******************************
**
** Function prototypes
**
******************************/

//! Creates a handler for catching signals.
//! \param socket File descriptor for server.
void net_sigchld_handler(int socket);

//! Creates a socket for TCP communications.
//! \return A network file descriptor.
int net_socket();

//! Sets TCP socket reuse option.
//! \param fd A pointer to a network file descriptor.
//! \return -1 if error, else 0.
int net_setsockopt(int *fd);

//! Sets TCP socket non-blocking option.
//! \param fd A pointer to a network file descriptor.
//! \return -1 if error, else 0.
int net_setnonblock(int *fd);

//! Binds a TCP socket.
//! \param fd A pointer to a network file descriptor.
//! \param port Port to bind.
void net_bind(int *fd, short port);

//! Listens for incoming TCP connections.
//! \param fd A pointer to a network file descriptor.
void net_listen(int *fd);

//! Sets actions to take based upon signal emitted.
//!
void net_sigaction();

//! Accepts incoming TCP connections.
//! Returns a pointer to a file descriptor.
//! \param fd_orig The original file descriptor.
//! \return The modified file descriptor.
int net_accept(int fd_orig);

//! Closes a TCP socket connection.
//! \param fd A network file descriptor.
void net_close(int fd);

//! Sends data on a TCP socket connection.
//! Returns the number of bytes sent.
//! \param fd A network file descriptor.
//! \param msg The message to send.
//! \param len The message length.
//! \return The number of bytes sent.
int net_send(int fd, const void *msg, int len);

//! Selects a TCP port to communicate with.
//! Returns the value of the select call.
//! \param tv A timeval struct that is modified by select().
//! \return Fills in read_fds struct with file descriptors waiting to be read
//!         from using select() system call.
int net_select(struct timeval tv);

//! Gets the hostname of a specified IP address.
//! \param address A pointer to the an IP address.
void net_gethostbyname(char *address);

//! Connects to an existing TCP socket.
//! \param fd A network file descriptor.
//! \param port Port to connect to.
//! \return 1 on success, -1 on failure.
int net_connect(int fd, short port);

//! Receives data on a TCP socket connection.
//! Returns the number of bytes received.
//! \param fd A file descriptor for the network connection.
//! \param buf A pointer to a buffer for network data.
//! \return The number of bytes received.
int net_recv(int fd, void *buf);

//! Create a TCP server.
//! Returns a file descriptor for the API server.
//! \param port Port to use for server.
//! \return File descriptor for the server.
int net_server_setup(short port);

//! Create a TCP client.
//! \param address A pointer to the IP address of the server.
//! \param port Port to use for client.
//! \return File descriptor for the client.
int net_client_setup(char *address, short port);

//! Send and receive data for a TCP server.
//! \param fd A file descriptor for the server.
//! \param buf A pointer to a buffer for network data.
//! \param msg A pointer to message data.
//! \param mode Mode for the server to act in.
//! \return Number of bytes received.
int net_server(int fd, void *buf, MSG_DATA *msg, int mode);

//! Send and receive data for a TCP client.
//! \param fd A file descriptor for the client.
//! \param buf A pointer to a buffer for network data.
//! \param msg A pointer to message data.
//! \param mode Mode for the client to act in.
//! \return Number of bytes received.
int net_client(int fd, void *buf, MSG_DATA *msg, int mode);


#endif /* _NETWORK_H_ */

