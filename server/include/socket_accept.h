/**
 * @file socket_accept.h
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __SOCKET_ACCPET_H__
#define __SOCKET_ACCPET_H__

#include <vector>
#include <map>
#include <stdio.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <math.h>
#include <sstream>

// socket
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// socket_config
#include "socket_config.h"

// thread pool
#include "threadPool.h"

// log
#include "Log.h"
using namespace Log;

/**
 * @brief Socket_accept
 *      - be use to accept new client, and Manage the client
 */
class Socket_accept
{
public:
    Socket_accept();
    ~Socket_accept();

    /**
     * @brief start accept thread
     *
     * @param listen server socket
     */
    void SocketAccept_socket_accept(int listen);

private:
    /**
     * @brief accept client
     *
     * @param listen server socket
     */
    void SocketAccept_accept_client(int listen);

    /**
     * @brief receive client`s message
     *
     * @param clientSocket  client`s socket
     * @param clientIP  client's IP
     * @param port  client's port
     * @return true
     * @return false
     */
    bool SocketAccept_recvMessage(int clientSocket, char *clientIP, int clientPort);

private:
    sockaddr_in m_clientAddr;                         // client addr
    socklen_t m_clientAddrLen = sizeof(m_clientAddr); // client addr`s len
    int m_socket_server;                              // server socket

    ThreadPool *m_threadPool; // accept and recvMessage thread pool

    bool m_TerminateFlag = false; // stop flag (true - stop)

    // client list
    struct s_socket_client
    {
        int socket;              // client`s socket
        sockaddr_in socket_Addr; // client`s addr

        char ip[INET_ADDRSTRLEN];
        int port;
    };
    std::map<int, struct s_socket_client> m_client_list;

    // users
    struct s_user_Information
    {
        std::string UserID = "";
        int socket = -1;
        std::string IP = "";
        int port = -1;
        std::string Connect_UserID = "";
    };
    // <userID, information>
    std::map<std::string, struct s_user_Information> users;
};

#endif // __SOCKET_ACCPET_H__