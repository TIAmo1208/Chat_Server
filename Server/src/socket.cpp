/**
 * @file socket.cpp
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "socket.h"

#define VERSION (0.2)

/*______ F U N C T I O N _____________________________________________________*/

Socket::Socket()
{
    LogSystem::instance()->Log_init("Log_Chat_Server");

    if (socket_Init() != 1)
    {
        return;
    }
}

Socket::Socket(int port, int domain, int type, int protocol)
    : m_port(port), m_domain(domain), m_type(type), m_protocol(protocol)
{
    LogSystem::instance()->Log_init("Log_server");

    if (socket_Init() != 1)
    {
        Log_error("Socket Init is fail");
        return;
    }
}

Socket::~Socket()
{
    delete LogSystem::instance();
}

void Socket::socket_accept()
{
    // start accept new client
    this->m_accept = new Socket_accept;
    this->m_accept->SocketAccept_socket_accept(m_socket_listen);
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

    // Set non-blocking
    // int flags = fcntl(m_socket_listen, F_GETFL, 0);
    // fcntl(m_socket_listen, F_SETFL, flags | O_NONBLOCK);

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
