/* MiCO Team
 * Copyright (c) 2017 MXCHIP Information Tech. Co.,Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MICOSOCKET_H__
#define __MICOSOCKET_H__


#include "lwip/sockets.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @defgroup MICO_SOCKET_GROUP_1 MICO BSD-like Socket Functions
  * @brief Provide basic APIs for socket function
  * @{
  */

/**
  * @brief  Create an endpoint for communication
  * @attention  Never doing operations on one socket in different MICO threads
  * @param  domain: Specifies a communication domain; this selects the protocol
  *         family which will be used for communication. 
  *           This parameter can be one value:
  *             @arg AF_INET: IPv4 Internet protocols.
  * @param  type: Specifies the communication semantics.
  *           This parameter can be one of the following values:
  *             @arg SOCK_STREAM: Provides sequenced, reliable, two-way, 
  *                    connection-based byte streams.  An out-of-band data 
  *                    transmission mechanism may be supported. (TCP)
  *             @arg SOCK_DGRAM: Supports datagrams (connectionless, unreliable
  *                    messages of a fixed maximum length).(UDP)
  * @param  protocol: specifies a particular protocol to be used with the socket.
  *            This parameter can be one of the following values:
  *             @arg IPPROTO_TCP: TCP protocol
  *             @arg IPPROTO_UDP: UDP protocol
  * @retval On success, a file descriptor for the new socket is returned.  
            On error, -1 is returned.
  */
int socket(int domain, int type, int protocol);

/**
  * @brief  Set options on sockets
  * @attention  Never doing operations on one socket in different MICO threads
  * @param  socket: A file descriptor
  * @param  level: This parameter can be : IPPROTO_IP, SOL_SOCKET, IPPROTO_TCP, IPPROTO_UDP
  * @param  optname: This parameter is defined in SOCK_OPT_VAL
  * @param  optval: address of buffer in which the value for the requested option(s) 
  *         are to be set.
  * @param  optlen: containing the size of the buffer pointed to by optval
  * @retval On success, zero is returned.  On error, -1 is returned.
  */
int setsockopt (int socket, int level, int optname, void *optval, socklen_t optlen);

/**
  * @brief  Get options on sockets
  * @attention  Never doing operations on one socket in different MICO threads
  * @param  socket: A file descriptor
  * @param  level: This parameter can be : IPPROTO_IP, SOL_SOCKET, IPPROTO_TCP, IPPROTO_UDP
  * @param  optname: This parameter is defined in SOCK_OPT_VAL
  * @param  optval: address of buffer in which the value for the requested option(s)
  *         are to be returned.
  * @param  optlen_ptr: This is a value-result argument, initially containing the size
  *         of the buffer pointed to by optval, and modified on return to indicate 
  *         the actual size of the value returned. 
  * @retval On success, zero is returned.  On error, -1 is returned.
  */
int getsockopt (int socket, int level, int optname, void *optval, socklen_t *optlen_ptr);

/**
  * @brief  bind a name to a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @note   Assigns the address specified by addr to the socket referred to by the file
  *         descriptor socket.
  * @param  socket: A file descriptor
  * @param  addr: Point to the target address to be binded
  * @param  length: This parameter containing the size of the buffer pointed to by addr
  * @retval On success, zero is returned.  On error, -1 is returned.
  */
int bind (int socket, struct sockaddr *addr, socklen_t length);

/**
  * @brief    Initiate a connection on a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @details  The connect() system call connects the socket referred to by the file
  *           descriptor socket to the address specified by addr.
  * @param    socket: A file descriptor
  * @param    addr: Point to the target address to be binded
  * @param    length: This parameter containing the size of the buffer pointed to by addr
  * @retval   On success, zero is returned.  On error, -1 is returned.
  */
int connect (int socket, struct sockaddr *addr, socklen_t length);

/**
  * @brief    Listen for connections on a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @details  listen() marks the socket referred to by socket as a passive socket,
  *           that is, as a socket that will be used to accept incoming connection
  *           requests using accept().
  * @param    socket: a file descriptor.
  * @param    n: Defines the maximum length to which the queue of pending
  *           connections for socket may grow. This parameter is not used in MICO,
  *           use 0 is fine.
  * @retval   On success, zero is returned.  On error, -1 is returned.
  */
int listen (int socket, int n);

/**
  * @brief    Accept a connection on a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @details  The accept() system call is used with connection-based socket types
  *           (SOCK_STREAM).  It extracts the first connection request on the queue
  *           of pending connections for the listening socket, sockfd, creates a 
  *           new connected socket, and returns a new file descriptor referring to 
  *           that socket.  The newly created socket is not in the listening state.  
  *           The original socket socket is unaffected by this call.
  * @param    socket: A file descriptor.
  * @param    addr: Point to the buffer to store the address of the accepted client.
  * @param    length_ptr: This parameter containing the size of the buffer pointed to
  *             by addr.
  * @retval   On success, zero is returned.  On error, -1 is returned.
  */
int accept (int socket, struct sockaddr *addr, socklen_t *length_ptr);

/**
  * @brief      Monitor multiple file descriptors, waiting until one or more of the 
  *             file descriptors become "ready" for some class of I/O operation 
  *             (e.g., input possible). 
  * @attention  Never doing operations on one socket in different MICO threads
  * @note       A file descriptor is considered ready if it is possible to perform 
  *             the corresponding I/O operation (e.g., read()) without blocking.
  * @param      nfds: is the highest-numbered file descriptor in any of the three
  *             sets, plus 1. In MICO, the mount of file descriptors is fewer, so  
  *             MICO use the MAX number of these file descriptors inside, and this 
  *             parameter is cared.
  * @param      readfds: A file descriptor sets will be watched to see if characters 
  *             become available for reading
  * @param      writefds: A file descriptor sets will be watched to see if a write 
  *             will not block.
  * @param      exceptfds: A file descriptor sets will be watched for exceptions.
  * @param      timeout: The timeout argument specifies the interval that select() 
  *             should block waiting for a file descriptor to become ready. 
  *             If timeout is NULL (no timeout), select() can block indefinitely.   
  * @retval     On success, return the number of file descriptors contained in the 
  *             three returned descriptor sets (that is, the total number of bits 
  *             that are set in readfds, writefds, exceptfds) which may be zero if 
  *             the timeout expires before anything interesting happens.  On error, 
  *             -1 is returned, the file descriptor sets are unmodified, and timeout 
  *             becomes undefined.
  */
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);


/**
  * @brief      Send a message on a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @details    The send() call may be used only when the socket is in a connected
  *             state (so that the intended recipient is known). The only difference
  *             between send() and write() is the presence of flags.  With a zero
  *             flags argument, send() is equivalent to write().
  * @note       When the message does not fit into the send buffer of the socket,
  *             send() normally blocks, unless the socket has been placed in
  *             nonblocking I/O mode.  In nonblocking mode it would fail. The select()
  *             call may be used to determine when it is possible to send more data.        
  * @param      socket: A file descriptor.
  * @param      buffer: Point to the send data buffer.
  * @param      size: Length of the send data buffer.
  * @param      flags: Zero in MICO.
  * @retval     On success, these calls return the number of bytes sent.  On error,
  *             -1 is returned,
  */
int send (int socket, const void *buffer, size_t size, int flags);

/**
  * @brief      Send a message on a socket
  * @attention  Never doing operations on one socket in different MICO threads
  * @note       Refer send() for details.
  */
ssize_t write (int filedes, const void *buffer, size_t size);


/**
  * @brief      Send a message on a socket to a specific target address.
  * @attention  Never doing operations on one socket in different MICO threads
  * @details    Refer send() for details. If sendto() is used on a connection-mode 
  *             (SOCK_STREAM, SOCK_SEQPACKET) socket, the arguments dest_addr and 
  *             addrlen are ignored. Otherwise, the address of the target is given by
  *             dest_addr with addrlen specifying its size. 
  * @param      socket: Refer send() for details.
  * @param      buffer: Refer send() for details.
  * @param      size: Refer send() for details.
  * @param      flags: Refer send() for details.
  * @param      addr: Point to the target address.
  * @param      length: This parameter containing the size of the buffer pointed to
  *             by addr.
  * @retval     On success, these calls return the number of bytes sent.  On error,
  *             -1 is returned,
  */
int sendto (int socket, const void *buffer, size_t size, int flags, const struct sockaddr *addr, socklen_t length);


/**
  * @brief      Receive a message from a socket.
  * @attention  Never doing operations on one socket in different MICO threads
  * @details    If no messages are available at the socket, the receive calls wait
  *             for a message to arrive, unless the socket is nonblocking, in 
  *             which case the value -1 is returned.  The receive calls normally 
  *             return any data available, up to the requested amount, rather than 
  *             waiting for receipt of the full amount requested.
  * @param      socket: A file descriptor.
  * @param      buffer: Point to the send data buffer.
  * @param      size: Length of the send data buffer.
  * @param      flags: Zero in MICO.
  * @retval     These calls return the number of bytes received, or -1 if an error
  *             occurred.
  *             When a stream socket peer has performed an orderly shutdown, the
  *             return value will be 0 (the traditional "end-of-file" return).
  *             The value 0 may also be returned if the requested number of bytes to
  *             receive from a stream socket was 0.
  */
int recv (int socket, void *buffer, size_t size, int flags);

/**
  * @brief      Receive a message from a socket.
  * @attention  Never doing operations on one socket in different MICO threads
  * @note       Refer recv() for details.
  */
ssize_t read (int filedes, void *buffer, size_t size);


/**
  * @brief      Receive a message from a socket and get the source address.
  * @attention  Never doing operations on one socket in different MICO threads
  * @details    If src_addr is not NULL, and the underlying protocol provides 
  *             the source address of the message, that source address is placed 
  *             in the buffer pointed to by src_addr.  In this case, addrlen is 
  *             a value-result argument.  Before the call, it should be 
  *             initialized to the size of the buffer associated with src_addr. 
  *             Upon return, addrlen is updated to contain the actual size of 
  *             the source address.  The returned address is truncated if the 
  *             buffer provided is too small; in this case, addrlen will return 
  *             a value greater than was supplied to the call.
  *             If the caller is not interested in the source address, src_addr
  *             should be specified as NULL and addrlen should be specified as 0.
  * @param      sockfd: Refer recv() for details.
  * @param      buf: Refer recv() for details.
  * @param      len: Refer recv() for details.
  * @param      flags: Refer recv() for details.
  * @param      src_addr: Point to the buffer to store the source address.
  * @param      addrlen: This parameter containing the size of the buffer pointed to 
  *             by src_addr.
  * @retval     These calls return the number of bytes received, or -1 if an 
  *             error occurred.
  *             When a stream socket peer has performed an orderly shutdown, the
  *             return value will be 0 (the traditional "end-of-file" return).
  *             The value 0 may also be returned if the requested number of bytes to
  *             receive from a stream socket was 0.
  */
int recvfrom (int socket, void *buffer, size_t size, int flags, struct sockaddr *addr, socklen_t *length_ptr);
 
/**
  * @brief      Close a file descriptor.
  * @attention  Never doing operations on one socket in different MICO threads
  * @details    closes a file descriptor, so that it no longer refers to any
  *             file and may be reused.the resources associated with the
  *             open file description are freed.
  * @param      filedes: A file descriptor.
  * @retval     Returns zero on success.  On error, -1 is returned.
  */
int close (int filedes);


#ifdef __cplusplus
}
#endif

#endif /*__MICO_SOCKET_H__*/



