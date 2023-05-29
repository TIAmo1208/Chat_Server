#ifndef __SERVER_IMPL_H__
#define __SERVER_IMPL_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Server_Interface.hpp"
#include "Server_Tools.hpp"
#include "Server_config.h"

#ifdef __linux__
#include <arpa/inet.h>
#include <fcntl.h> // set no blocking
#include <unistd.h>
// #include <sys/select.h>
#endif

#include "threadPool.hpp" // ThreadPol

#include <condition_variable>
#include <map>
#include <mutex>
#include <queue>
#include <string>

/*______ C L A S S ___________________________________________________________*/

class Server_impl : public Server_Interface
{
    /*______ F U N C T I O N _________________________________________________*/

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
    virtual int Server_Init(int port = 8888, int threadNum = 5) override;

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
     * @brief Data processing thread
     *
     */
    void Server_Task_Processing();

    /**
     * @brief Accept client connection
     *
     */
    void Server_Accept_Client();

    /**
     * @brief Handle disconnected clients
     *
     * @param _index
     */
    void Server_Disconnect_Client(int _index);

    /**
     * @brief Receive client message
     *
     * @param _readfds
     */
    void Server_Receive_Event(fd_set _readfds);

    /**
     * @brief Handles the task of connect server
     *
     * @param _sendUserID
     * @param _task
     */
    void Server_Process_ConnectServer(std::string _sendUserID, EventTask _task);

    /**
     * @brief Handles the task of connect other client
     *
     * @param _sendUserID
     * @param _task
     */
    void Server_Process_ConnectClient(std::string _sendUserID, EventTask _task);

    /**
     * @brief Handles the task of client send message to other client
     *
     * @param _sendUserID
     * @param _task
     */
    void Server_Process_SendMessage(std::string _sendUserID, EventTask _task);

    /**
     * @brief Handles the task of client send file to other client
     *
     * @param _sendUserID
     * @param _task
     */
    void Server_Process_SendFile(std::string _sendUserID, EventTask _task);

    /**
     * @brief
     *
     * @param _sendUserID
     * @param _task
     * @param _userName
     */
    void Server_Process_ReturnFriendList(std::string _sendUserID, EventTask _task, std::string _userName);

private:
    /*______ V A R I A B L E _________________________________________________*/

    recvMsg_CallbackFunc m_recvMsgCallback = nullptr;

    int m_domain;
    int m_type;          // Types of sockets
    int m_protocol;      // protocols
    int m_port;          // port
    int m_socket_server; // server socket

    bool isInit          = false; // initialization flag
    bool m_TerminateFlag = false; // stop flag (true: stop)

    // fd list
    Thread_List m_fd_array;

    // <socket, information>
    Thread_Map<int, ClientInfo> m_client_list;

    // <userID, information>
    Thread_Map<std::string, UserInfo> m_users;

    // Task
    Thread_Queue<EventTask> m_queue_task;

    // thread pool
    std::shared_ptr<ThreadPool> m_threadPool;

    // Tools
    Server_Tools m_tools;
};

#endif // __SERVER_IMPL_H__
