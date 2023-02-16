/**
 * @file socket_accept.h
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __SOCKET_ACCPET_H__
#define __SOCKET_ACCPET_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <vector>
#include <map>
#include <queue>

#include <stdio.h>

#include <thread>
#include <condition_variable>
#include <mutex>

#include <math.h>
#include <sstream>
#include <cstring>

// socket
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "socket_config.h"

// thread pool
#include "threadPool.hpp"

// log
#include "Log.h"
using namespace Log;

/*______ F U N C T I O N _____________________________________________________*/

/// @brief Socket_accept - be use to accept new client, and Manage the client
class Socket_accept
{
public:
    Socket_accept(ThreadPool *threadpool);
    ~Socket_accept();

    /// @brief start accept thread
    /// @param listen server socket
    void SocketAccept_socket_accept(int listen);
};

#endif // __SOCKET_ACCPET_H__
