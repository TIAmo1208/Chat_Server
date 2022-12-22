/**
 * @file socket.h
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <stdio.h>

// socket
#include "socket_config.h"
#include "socket_accept.h"

#ifdef LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // set no blocking
#endif

// log
#include "Log.h"
using namespace Log;

/**
 * @brief Socket init,accept,revice msg and send msg
 *
 */
class Socket
{
private:
    /**
     * @brief Init
     *
     * @return int return the init result
     */
    int socket_Init();

public:
    Socket();
    Socket(int port, int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
    ~Socket();

    /**
     * @brief start accept new client
     *
     * @param accept the accept class
     */
    void socket_accept();

private:
    int m_domain = SOCKET_CONFIG_DOMAIN;      // IPv4
    int m_type = SOCKET_CONFIG_TYPE;          // Types of sockets
    int m_protocol = SOCKET_CONFIG__PROTOCOL; // protocols
    int m_port = SOCKET_CONFIG__PORT;         // port
    int m_socket_listen;                      // server socket

    Socket_accept *m_accept; // accept client connect
};

#endif // __SOCKET_H__