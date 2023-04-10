#ifndef __SERVER_IMPL_H__
#define __SERVER_IMPL_H__

#include "Server_Interface.hpp"

#include "Server_config.h"

// Log
#include "Log.h"
using namespace Log;
// Mysql
#include "Mysql.h"

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h> // set no blocking
#include <arpa/inet.h>

// #include <unistd.h>
// #include <sys/select.h>
#endif

#include <map>
#include <queue>
#include <math.h>

//
#include <thread>
#include <condition_variable>
#include <mutex>

/*______ S T R U C T _________________________________________________________*/

// client list
struct s_socket_client
{
    std::string user_id; // client's ID

    int socket; // client`s socket
    char ip[INET_ADDRSTRLEN];
    int port;
    sockaddr_in socket_Addr; // client`s addr
};

// users
struct s_user_Information
{
    int socket = -1;

    std::string UserID = "";
    std::string UserName = "";
    std::string Connect_UserID = "";
};

// task
struct s_event_task
{
    int socket = -1;
    int code;
    int bits_high;
    int bits_low;

    std::string recvBuff;
};

/*______ F U N C T I O N _____________________________________________________*/

class Server_impl : public Server_Interface
{
private:
    /* data */
public:
    Server_impl(/* args */);
    virtual ~Server_impl();

public:
    /**
     * @brief Server initialization
     *
     * @param port
     * @return int
     */
    virtual int Server_Init(int port = 8888) override;

    /**
     * @brief main Update
     *
     */
    virtual void Server_Update() override;

    /**
     * @brief Register Recive Message Callback Function
     *
     * @param callback The Point of Callback Function
     */
    virtual void Server_Reg_recvMsgCallbackFunc(recvMsg_CallbackFunc _callback) override;

private:
    /**
     * @brief
     *
     * @param _readfds
     */
    void Server_Receive_Event(fd_set &_readfds);

    void Server_Accept_Client();

    int Server_Receive_DataFrames(int _clientSocket, s_user_Information &_user, char *_recvBuff);

    void Server_Disconnect_Client(int _index);

    void Server_Thread_Task();

private:
    recvMsg_CallbackFunc m_recvMsgCallback = nullptr;

    int m_domain;
    int m_type;          // Types of sockets
    int m_protocol;      // protocols
    int m_port;          // port
    int m_socket_server; // server socket

    bool isInit = false;          // initialization flag
    bool m_TerminateFlag = false; // stop flag (true: stop)

    int m_fd_array[FD_ARRAY_MAXSIZE];
    int m_fd_array_length = 0; // fd_array's elements number

    // <socket, information>
    std::map<int, struct s_socket_client> m_client_list;

    // <userID, information>
    std::map<std::string, struct s_user_Information> m_users;

    // Task
    std::thread m_thread_task;
    std::condition_variable m_condition_task;
    std::queue<s_event_task> m_queue_task;
    std::mutex m_mutex_task;
    bool m_new_task = false;
};

#endif // __SERVER_IMPL_H__
