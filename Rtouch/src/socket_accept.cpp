/**
 * @file socket_accept.cpp
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "socket_accept.h"

Socket_accept::Socket_accept()
{
    m_threadPool = new ThreadPool;
}

Socket_accept::~Socket_accept()
{
    // m_thread_accept.join();
    // m_threadPool.~ThreadPool();

    m_TerminateFlag = true;
    delete m_threadPool;
}

void Socket_accept::socket_accept(int listen)
{
    // new thread - accept client
    // m_thread_accept = std::thread(&Socket_accept::accept_client, this, listen);
    m_threadPool->add_Task(&Socket_accept::accept_client, this, listen);

    Log_info("Thread: accept thread");

    while (true)
    {
        // send message or another handle
    }
}

void Socket_accept::accept_client(int listen)
{
    Log_info("START: accept...");
    while (!m_TerminateFlag)
    {
        int connect = accept(listen, (sockaddr *)&m_clientAddr, &m_clientAddrLen);
        if (SOCKET_ERROR == connect)
        {
            Log_error("accept");
            sleep(1);
            continue;
        }

        // get the client
        struct Socket_accept::s_socket_client client;
        client.socket = connect;
        client.socket_Addr = m_clientAddr;

        // get the IP and port
        inet_ntop(AF_INET, &client.socket_Addr.sin_addr, client.ip, INET_ADDRSTRLEN);
        client.port = ntohs(client.socket_Addr.sin_port);

        // save the information
        this->m_client_list.push_back(client);

        // Log_info("Get new client: %s:%d", client.ip, client.port);
        Log_info("START: recevice Message from %s:%d", client.ip, client.port);

        // add task of receive the message
        std::future<bool> res = this->m_threadPool->add_Task(&Socket_accept::recvMessage, this, client.socket, client.ip, client.port);
    }
}

bool Socket_accept::recvMessage(int clientSocket, char *clientIP, int port)
{
    char recvBuff[BUFF_MAX_SIZE];
    memset(recvBuff, 0, BUFF_MAX_SIZE);

    while (true)
    {
        // It will be close when recv return the message's length is 0
        if (recv(clientSocket, recvBuff, sizeof(BUFF_MAX_SIZE), 0) != 0)
        {
            // message processing
            Log_info("%s:%d : %s", clientIP, port, recvBuff);
            memset(recvBuff, 0, BUFF_MAX_SIZE);
        }
        else
        {
            return false;
        }
    }
}
