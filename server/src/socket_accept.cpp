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

// #define START_HEATBEAT_PACKET

#define CODE_CONNECT_SERVER 0x0000
#define CODE_CONNECT_CLIENT 0x0001
#define CODE_SEND_MESSAGE 0x0002
#define CODE_SERVER_RECV_ERROR 0xe001

#define SOCKET_ERROR_RECV(target) send(target, (char *)CODE_SERVER_RECV_ERROR, sizeof((char *)CODE_SERVER_RECV_ERROR), 0);

int convert_hexChar_to_int(char *str);

Socket_accept::Socket_accept()
{
    m_threadPool = new ThreadPool;
}

Socket_accept::~Socket_accept()
{
    m_TerminateFlag = true;
    delete m_threadPool;

    m_client_list.clear();
}

void Socket_accept::SocketAccept_socket_accept(int listen)
{
    // new thread - accept client
    m_socket_server = listen;
    m_threadPool->add_Task(&Socket_accept::SocketAccept_accept_client, this, m_socket_server);

    Log_info("Thread: start accept client");

    bool state = false;
    char buff_true[] = "true\n";
    char buff_false[] = "false\n";

    // heartbeat packet
    while (!m_TerminateFlag)
    {
        if (m_client_list.empty())
            continue;

#ifdef START_HEATBEAT_PACKET
        for (std::map<int, Socket_accept::s_socket_client>::iterator ite = m_client_list.begin(); ite != m_client_list.end(); ++ite)
        {
            if (state)
                send(ite->first, (void *)buff_true, sizeof(buff_true), 0);
            else
                send(ite->first, (void *)buff_false, sizeof(buff_true), 0);
        }
#endif
        state = !state;

        sleep(3);
    }
}

void Socket_accept::SocketAccept_accept_client(int listen)
{
    Log_info("START: accept...");
    while (!m_TerminateFlag)
    {
        int connect = accept(listen, (sockaddr *)&m_clientAddr, &m_clientAddrLen);
        if (SOCKET_ERROR == connect)
        {
            Log_error("The client connection failure");
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
        // this->m_client_list.push_back(client);
        this->m_client_list[connect] = client;

        // Log_info("Get new client: %s:%d", client.ip, client.port);
        Log_info("START: recevice Message from %s:%d", client.ip, client.port);

        // add task of receive the message
        std::future<bool> res = this->m_threadPool->add_Task(&Socket_accept::SocketAccept_recvMessage, this, client.socket, client.ip, client.port);
    }
}

bool Socket_accept::SocketAccept_recvMessage(int clientSocket, char *clientIP, int clientPort)
{
    char recvBuff[BUFF_MAX_SIZE];
    std::string str_messageProcess;
    int messageType = 0;
    s_user_Information user;
    std::stringstream sstream;

    while (true)
    {
        memset(recvBuff, 0, BUFF_MAX_SIZE);

        // It will be close when recv return the message's length is 0
        if (recv(clientSocket, recvBuff, BUFF_MAX_SIZE, 0) == 0)
        {
            m_client_list.erase(clientSocket);
            Log_info("%s:%d : The client is disconnect ", clientIP, clientPort);

            if (!user.UserID.empty())
            {
                users.erase(user.UserID);
            }

            return false;
        }

        char tmp_code[6];
        memset(tmp_code, 0, 6);
        strncpy(tmp_code, recvBuff, 6);
        // strcpy(tmp_code, str_messageProcess.c_str());

        messageType = convert_hexChar_to_int(tmp_code);
        str_messageProcess = recvBuff;
        // message processing
        {
            str_messageProcess = str_messageProcess.substr(6, str_messageProcess.length());
            if (messageType == CODE_CONNECT_SERVER)
            {
                // char tmp_userID[BUFF_MAX_SIZE];
                // memset(tmp_userID, 0, BUFF_MAX_SIZE);

                // recv error
                // if (recv(clientSocket, tmp_userID, BUFF_MAX_SIZE, 0) == 0)
                // {
                //     Log_warn("Message receiving error");
                //     SOCKET_ERROR_RECV(user.socket);
                //     continue;
                // }

                //// TODO : 数据处理，检验 userID 是否正确
                //// TODO : 获取数据库，验证 用户密码 是否正确

                user.UserID = str_messageProcess;
                user.socket = clientSocket;
                user.IP = clientIP;
                user.port = clientPort;

                users[user.UserID] = user;
                Log_info("code : %s connect server", user.UserID.c_str());
            }
            else if (messageType == CODE_CONNECT_CLIENT)
            {
                // char tmp_connectID[BUFF_MAX_SIZE];
                // memset(tmp_connectID, 0, BUFF_MAX_SIZE);

                // // recv error
                // if (recv(clientSocket, tmp_connectID, BUFF_MAX_SIZE, 0) == 0)
                // {
                //     Log_warn("Message receiving error");
                //     SOCKET_ERROR_RECV(user.socket);
                //     continue;
                // }

                user.Connect_UserID = str_messageProcess;
                Log_info("code : %s:%d connect client %s:%d", user.IP.c_str(), user.port, users[user.Connect_UserID].IP.c_str(), users[user.Connect_UserID].port);
            }
            else if (messageType == CODE_SEND_MESSAGE)
            {
                // char tmp_sendMsg[BUFF_MAX_SIZE];
                // memset(tmp_sendMsg, 0, BUFF_MAX_SIZE);

                // // recv error
                // if (recv(clientSocket, tmp_sendMsg, BUFF_MAX_SIZE, 0) == 0)
                // {
                //     Log_warn("Message receiving error");
                //     SOCKET_ERROR_RECV(user.socket);
                //     continue;
                // }

                if (user.Connect_UserID == "" || users[user.Connect_UserID].socket == -1)
                {
                    Log_warn("send message : It is not set the connect client ");
                    continue;
                }
                send(users[user.Connect_UserID].socket, str_messageProcess.c_str(), str_messageProcess.length(), 0);

                Log_info("code : %s:%d send message to %s:%d", user.IP.c_str(), user.port, users[user.Connect_UserID].IP.c_str(), users[user.Connect_UserID].port);
            }
            else if (messageType == 0x1000)
            {
                Log_info("0x1000");
            }
            else
            {
                Log_info("Unknow code !!!");
            }
        }
    }
}

/**
 * @brief convert char to int
 *
 * @param str
 * @return int
 */
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
