/**
 * @file socket.h
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
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
#include "../../out/include/Log.h"
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
    Socket(int port = 8000, int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_TCP);
    ~Socket();

    /**
     * @brief start accept new client
     *
     * @param accept the accept class
     */
    void socket_accept(ThreadPool *threadpool);
};

#endif // __SOCKET_H__