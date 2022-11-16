/**
 * @file socket_i.h
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SOCKET_I_H__
#define __SOCKET_I_H__

#define LINUX

// socket config
#define SOCKET_CONFIG_DOMAIN AF_INET;        // IPv4
#define SOCKET_CONFIG_TYPE SOCK_STREAM;      // Types of sockets
#define SOCKET_CONFIG__PROTOCOL IPPROTO_TCP; // protocols
#define SOCKET_CONFIG__PORT 8000;            // port

// socket returned value
#define SOCKET_ERROR -1
#define SOCKET_SUCCESS 0

// get or set Buffer size
#define BUFF_MAX_SIZE 4096

#endif // __SOCKET_I_H__