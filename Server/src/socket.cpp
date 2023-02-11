/**
 * @file socket.cpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
 *
 */
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "../include/socket.h"

#define VERSION (0.2)

/*______ V A R I A B L E _____________________________________________________*/

int m_domain = SOCKET_CONFIG_DOMAIN;     // IPv4
int m_type = SOCKET_CONFIG_TYPE;         // Types of sockets
int m_protocol = SOCKET_CONFIG_PROTOCOL; // protocols
int m_port = SOCKET_CONFIG_PORT;         // port
int m_socket_listen;                     // server socket

Socket_accept *m_accept; // accept client connect

/*______ F U N C T I O N _____________________________________________________*/

Socket::Socket(int port, int domain, int type, int protocol)
{
    m_port = port;
    m_domain = domain;
    m_type = type;
    m_protocol = protocol;

    if (socket_Init() != 1)
    {
        Log_error("Socket Init is fail");
        return;
    }
}

Socket::~Socket()
{
}

int Socket::socket_Init()
{
    Log_info("---- Chat Server\tversion:%0.2f ----", VERSION);
    int ret = 0;

    // socket
    Log_debug("Server: init");
    ret = socket(m_domain, m_type, m_protocol);
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: init");
        return ret;
    }
    m_socket_listen = ret;

    //
    // int opt = 1;
    // setsockopt(m_socket_listen, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Set non-blocking
    int flags = fcntl(m_socket_listen, F_GETFL, 0);
    fcntl(m_socket_listen, F_SETFL, flags | O_NONBLOCK);

    // bind
    Log_debug("Server: bind");
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_socket_listen, (struct sockaddr *)&addr, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: bind");
        return ret;
    }

    // listen
    Log_info("Server: listen (port:%d) ...", m_port);
    ret = listen(m_socket_listen, 5);
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: listen");
        return ret;
    }

    Log_debug("DONE: socket init 127.0.0.1:%d", m_port);

    return 1;
}

void Socket::socket_accept(ThreadPool *threadpool)
{
    // start accept new client
    m_accept = new Socket_accept(threadpool);
    m_accept->SocketAccept_socket_accept(m_socket_listen);
}
