/**
 * @file socket_accept.cpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief 
 * @version 0.1
 * @date 2023-02-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "../include/socket_accept.h"

/*______ D E F I N E _________________________________________________________*/

// heatbeat packet enable
// #define START_HEATBEAT_PACKET

// message process
#define CODE_CONNECT_SERVER 0x0000
#define CODE_CONNECT_CLIENT 0x0001
#define CODE_SEND_MESSAGE 0x0002
#define CODE_SEND_FILE 0x0003

#define CODE_SERVER_RECV_ERROR "0xe001"
#define SOCKET_ERROR_RECV(target) send(target, CODE_SERVER_RECV_ERROR, sizeof(CODE_SERVER_RECV_ERROR), 0);

#define CODE_SERVER_NotLoggedIn_ERROR "0xe002"
#define SOCKET_ERROR_NotLoggedIn(target) send(target, CODE_SERVER_NotLoggedIn_ERROR, sizeof(CODE_SERVER_NotLoggedIn_ERROR), 0);

#define CODE_SERVER_NoExist_ERROR "0xe003"
#define SOCKET_ERROR_ConnectObject_NoExist(target) send(target, CODE_SERVER_NoExist_ERROR, sizeof(CODE_SERVER_NoExist_ERROR), 0);

#define CODE_SERVER_NoConnect_ERROR "0xe004"
#define SOCKET_ERROR_NoConnect(target) send(target, CODE_SERVER_NoConnect_ERROR, sizeof(CODE_SERVER_NoConnect_ERROR), 0);

/*______ V A R I A B L E _____________________________________________________*/

sockaddr_in m_clientAddr;                         // client addr
socklen_t m_clientAddrLen = sizeof(m_clientAddr); // client addr`s len
int m_socket_server;                              // server socket

ThreadPool *m_threadPool; // accept and recvMessage thread pool

bool m_TerminateFlag = false; // stop flag (true: stop)

// client list
struct s_socket_client
{
    int socket;              // client`s socket
    sockaddr_in socket_Addr; // client`s addr
    std::string user_id;     // client's ID

    char ip[INET_ADDRSTRLEN];
    int port;
};
// <socket, information>
std::map<int, struct s_socket_client> m_client_list;

// users
struct s_user_Information
{
    int socket = -1;
    std::string IP = "";
    int port = -1;

    std::string UserID = "";
    std::string Connect_UserID = "";
};
// <userID, information>
std::map<std::string, struct s_user_Information> m_users;

// fd array
constexpr int m_fd_array_MaxSize = sizeof(fd_set) * 8; // fd_array's max size
int m_fd_array_length = 0;                             // fd_array's elements num
int m_fd_array[m_fd_array_MaxSize];

// recv file queue
std::queue<struct s_user_Information> m_queue_recvFile;

/*______ F U N C T I O N _____________________________________________________*/

/// @brief get the message to message
void selector();

/// @brief accept client
/// @param listen server socket
/// @return success return socekt, failure return -1
int accept_client(int listen);

/// @brief event processing
/// @param _readfds Readable events
void event_processing(fd_set &_readfds);

/// @brief recv the client message
/// @param _clientSocket
/// @return -1: client is discover; 0: recv success
int recvMessage(int _clientSocket);

/// @brief message processing
/// @param _message
/// @param _message_type
/// @param _user
void message_processing(std::string _message, int _message_type, s_user_Information &_user);

/// @brief convert char to int
/// @param str
/// @return
int convert_hexChar_to_int(char *_str);

/// @brief send file by thread pool
/// @param send
int sendFile(struct s_user_Information _send);

/// @brief clear data of close client
/// @param _client_socket
void clear_close_client(int _client_socket);

/*______ F U N C T I O N _____________________________________________________*/

Socket_accept::Socket_accept(ThreadPool *threadpool)
{
    m_threadPool = threadpool;
}

Socket_accept::~Socket_accept()
{
    m_TerminateFlag = true;

    if (m_threadPool != nullptr)
    {
        delete m_threadPool;
        m_threadPool = nullptr;
    }

    if (!m_client_list.empty())
    {
        for (std::pair<const int, s_socket_client> client : m_client_list)
        {
            close(client.first);
        }
        m_client_list.clear();
    }

    m_users.clear();
}

void Socket_accept::SocketAccept_socket_accept(int listen)
{
    m_socket_server = listen;

    // m_threadPool->add_Task(&Socket_accept::SokcetAccept_selector, this);
    Log_info("Server: start accept client");
    selector();

#ifdef START_HEATBEAT_PACKET
    bool state = false;
    char buff_true[] = "true\n";
    char buff_false[] = "false\n";

    // heartbeat packet
    while (!m_TerminateFlag)
    {
        if (m_client_list.empty())
            continue;

        for (auto client : m_client_list)
        {
            if (state)
                send(client.second.socket, (void *)buff_true, sizeof(buff_true), 0);
            else
                send(client.second.socket, (void *)buff_false, sizeof(buff_true), 0);
        }
        state = !state;

        sleep(3);
    }
#endif
}

/*______ F U N C T I O N _____________________________________________________*/

void selector()
{
    // Init fd array
    for (int i = 0; i < m_fd_array_MaxSize; ++i)
    {
        m_fd_array[i] = -1;
    }
    m_fd_array[0] = m_socket_server;
    m_fd_array_length++;

    // Event processing
    fd_set readfds, writefds, errorfds;
    while (!m_TerminateFlag)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&errorfds);
        int max_fd = m_fd_array[0];

        // fill fd_set
        for (int i = 0; i < m_fd_array_length + 1; ++i)
        {
            if (m_fd_array[i] == -1)
            {
                continue;
            }

            FD_SET(m_fd_array[i], &readfds);
            if (m_fd_array[i] > max_fd)
            {
                max_fd = m_fd_array[i];
            }
        }

        // 同时监视多个文件描述符(socket)
        //  获取可读、可写、异常的文件描述符
        //  __timeout: 设置等待时间; 0:非阻塞; NULL:阻塞; 其他数值:等待指定时间;
        int ret = select(max_fd + 1, &readfds, &writefds, &errorfds, 0);

        //
        switch (ret)
        {
        case -1:
            Log_error("select error");
            return;

        case 0:
            Log_warn("select timeout");
            break;

        default:
            event_processing(readfds);
            break;
        }
    }
}

void event_processing(fd_set &_readfds)
{
    for (int i = 0; i < m_fd_array_MaxSize + 1; ++i)
    {
        // check socket read state
        if (m_fd_array[i] == -1 || !FD_ISSET(m_fd_array[i], &_readfds))
        {
            continue;
        }

        Log_debug("sockt:%d readable", m_fd_array[i]);

        // server socket readable
        //  the server have new client
        if (m_fd_array[i] == m_socket_server)
        {
            if (m_fd_array_length >= m_fd_array_MaxSize)
            {
                Log_warn("The maximum number of connections is reached");
                break;
            }

            int ret_accept = accept_client(m_socket_server);
            if (ret_accept != -1)
            {
                //
                if (m_fd_array[m_fd_array_length] == -1)
                {
                    m_fd_array[m_fd_array_length] = ret_accept;
                    m_fd_array_length++;
                }
                else
                {
                    for (int i = 1; i < m_fd_array_MaxSize; ++i)
                    {
                        if (m_fd_array[i] == -1)
                        {
                            m_fd_array[i] = ret_accept;
                            m_fd_array_length++;
                            break;
                        }
                    }
                }
            }
            else
            {
                close(ret_accept);
                continue;
            }
        }
        // client socket readable
        //  the client have new message
        else
        {
            Log_debug("%s:%d Client have new message", m_client_list[m_fd_array[i]].ip, m_client_list[m_fd_array[i]].port);
            int ret = recvMessage(m_fd_array[i]);

            // client discover
            if (ret == -1)
            {
                //
                clear_close_client(m_fd_array[i]);
                close(m_fd_array[i]);
                m_fd_array[i] = -1;
                m_fd_array_length--;
            }
        }
    }
}

int accept_client(int listen)
{
    int connect = accept(listen, (sockaddr *)&m_clientAddr, &m_clientAddrLen);
    if (SOCKET_ERROR == connect)
    {
        Log_error("The client connection failure");
        return -1;
    }

    // get the client
    struct s_socket_client client;
    client.socket = connect;
    client.socket_Addr = m_clientAddr;

    // get the IP and port
    inet_ntop(AF_INET, &client.socket_Addr.sin_addr, client.ip, INET_ADDRSTRLEN);
    client.port = ntohs(client.socket_Addr.sin_port);

    // save the information
    m_client_list[connect] = client;

    Log_info("START: recevice Message from %s:%d", client.ip, client.port);

    return connect;
}

int recvMessage(int _clientSocket)
{
    int ret = -1;

    // get the use if it is exist
    s_user_Information user;
    if (m_users.count(m_client_list[_clientSocket].user_id))
    {
        user = m_users[m_client_list[_clientSocket].user_id];
    }
    // create a new use if it is not exist
    else
    {
        user.socket = _clientSocket;
        user.IP = m_client_list[_clientSocket].ip;
        user.port = m_client_list[_clientSocket].port;
    }

    // recv file
    if (!m_queue_recvFile.empty() && m_queue_recvFile.front().socket == _clientSocket)
    {
        //// TODO : 接收文件，另起线程
        m_threadPool->add_Task(sendFile, user);
        // sleep for add task
        std::this_thread::sleep_for(std::chrono::microseconds(3));
        return ret = 0;
    }

    // Init recv buff
    char recvBuff[BUFF_MAX_SIZE];
    memset(recvBuff, 0, BUFF_MAX_SIZE);

    // It will be close when recv the feel back message's length is 0
    if (recv(_clientSocket, recvBuff, BUFF_MAX_SIZE, 0) == 0)
    {
        Log_info("%s:%d : The client is disconnect ", user.IP.c_str(), user.port);
        return ret = -1;
    }

    // get the code
    char tmp_code[6];
    int messageType;
    memset(tmp_code, 0, 6);
    strncpy(tmp_code, recvBuff, 6);
    messageType = convert_hexChar_to_int(tmp_code);

    // message processing
    std::string str_message;
    str_message = std::string(recvBuff);
    if (str_message.length() < 6)
    {
        Log_warn("recv Message : Data format error");
        return ret = 0;
    }
    str_message = str_message.substr(6, str_message.length());
    message_processing(str_message, messageType, user);

    // save the user change
    m_users[user.UserID] = user;
    m_client_list[_clientSocket].user_id = user.UserID;

    return ret = 0;
}

void message_processing(std::string _message, int _message_type, s_user_Information &_user)
{
    if (_message_type == CODE_CONNECT_SERVER)
    {
        //// TODO : 数据处理，检验 userID 是否正确
        //// TODO : 获取数据库，验证 用户密码 是否正确

        _user.UserID = _message;
        Log_info("code : %s connect server", _user.UserID.c_str());
    }
    else if (_message_type == CODE_CONNECT_CLIENT)
    {
        if (_user.UserID == "")
        {
            Log_warn("connect client : User ID is null");
            SOCKET_ERROR_NotLoggedIn(_user.socket);
            return;
        }

        //// TODO : 数据处理，检验 userID 是否正确
        if (m_users[_message].socket == -1)
        {
            Log_warn("connect client : the client %s is no exist", _message.c_str());
            SOCKET_ERROR_ConnectObject_NoExist(_user.socket);
            return;
        }

        _user.Connect_UserID = _message;
        Log_info("code : %s:%d connect client %s:%d", _user.IP.c_str(), _user.port, m_users[_user.Connect_UserID].IP.c_str(), m_users[_user.Connect_UserID].port);
    }
    else if (_message_type == CODE_SEND_MESSAGE)
    {
        if (_user.UserID == "")
        {
            Log_warn("send message : User ID is null");
            SOCKET_ERROR_NotLoggedIn(_user.socket);
            return;
        }

        if (_user.Connect_UserID == "" || m_users[_user.Connect_UserID].socket == -1)
        {
            Log_warn("send message : The connect client is no exist");
            SOCKET_ERROR_NoConnect(_user.socket);
            return;
        }

        send(m_users[_user.Connect_UserID].socket, _message.c_str(), _message.length(), 0);
        Log_info("code : %s:%d send message to %s:%d", _user.IP.c_str(), _user.port, m_users[_user.Connect_UserID].IP.c_str(), m_users[_user.Connect_UserID].port);
    }
    else if (_message_type == CODE_SEND_FILE)
    {
        //// TODO : 新增文件传输功能
        m_queue_recvFile.push(_user);
    }
    else
    {
        SOCKET_ERROR_RECV(_user.socket);
        Log_info("Unknow code !!! cod : %d", _message_type);
    }
}

/*______ F U N C T I O N _____________________________________________________*/

int convert_hexChar_to_int(char *str)
{
    int temp[4];
    int ret = 0, size = 4;

    if ((str[0] != '0' || str[1] != 'x') || (strlen(str) < 6))
        return -1;

    for (int i = 0; i < size; ++i)
    {
        if (str[i + 2] >= 48 && str[i + 2] <= 57)
        {
            temp[i] = str[i + 2] - 48;
        }
        else if (str[i + 2] >= 65 && str[i + 2] <= 70)
        {
            temp[i] = str[i + 2] - 55;
        }
        ret += temp[i] * pow(16, size - 1 - i);
    }

    return ret;
}

int sendFile(struct s_user_Information send)
{
    int ret = -1;

    std::string str_target = send.Connect_UserID;
    if (str_target.empty())
        return ret = -1;

    return ret = 0;
}

void clear_close_client(int _client_socket)
{
    std::string client_userID = m_client_list[_client_socket].user_id;

    if (!m_client_list.empty())
    {
        m_client_list.erase(_client_socket);
    }
    if (!m_users.empty() && !m_users[client_userID].UserID.empty())
    {
        m_users.erase(client_userID);
    }
    if (!m_queue_recvFile.empty())
    {
    }
}
