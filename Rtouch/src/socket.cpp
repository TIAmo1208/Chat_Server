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
#include "socket.h"

Socket::Socket()
{
    LogSystem::instance()->Log_init("Log_Rtouch");

    socket_Init();
}

Socket::Socket(int port, int domain, int type, int protocol)
    : m_port(port), m_domain(domain), m_type(type), m_protocol(protocol)
{
    LogSystem::instance()->Log_init("Log_Rtouch");

    socket_Init();
}

Socket::~Socket()
{
    delete LogSystem::instance();
}

void Socket::socket_accept()
{
    // start accept new client
    this->m_accept = new Socket_accept;
    this->m_accept->socket_accept(m_socket_listen);
}

void Socket::socket_Init()
{
    int ret = 0;

    // socket
    Log_info("START: init");
    ret = socket(m_domain, m_type, m_protocol);
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: init");
        return;
    }
    m_socket_listen = ret;

    // Set non-blocking
    // int flags = fcntl(m_socket_listen, F_GETFL, 0);
    // fcntl(m_socket_listen, F_SETFL, flags | O_NONBLOCK);

    // bind
    Log_info("START: bind");
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_socket_listen, (struct sockaddr *)&addr, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: bind");
        return;
    }

    // listen
    Log_info("START: listen (port:%d) ...", m_port);
    ret = listen(m_socket_listen, 5);
    if (SOCKET_ERROR == ret)
    {
        Log_error("ERROR: listen");
        return;
    }

    Log_info("DONE: socket init 127.0.0.1:%d", m_port);
}
