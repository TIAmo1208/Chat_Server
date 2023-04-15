
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Server_impl.hpp"

/*______ D E F I N E _________________________________________________________*/

// 客户端登录
#define CODE_CONNECT_SERVER 0x0000
// 连接其他客户端
#define CODE_CONNECT_CLIENT 0x0001
// 客户端发送信息
#define CODE_SEND_MESSAGE 0x0002
// 客户端发送文件
#define CODE_SEND_FILE 0x0003

// 登录成功返回用户名
#define CODE_LOGIN_RETURN "1000"
// 返回好友列表
#define CODE_FRIEND_RETURN "1001"
// 信息转发
#define CODE_TRANSMIT_MESSAGE "1002"

// 接收错误
#define CODE_SERVER_RECV_ERROR "e001"
#define SOCKET_ERROR_RECV(target) send(target, CODE_SERVER_RECV_ERROR, sizeof(CODE_SERVER_RECV_ERROR), 0);
// 未登录错误
#define CODE_SERVER_NotLoggedIn_ERROR "e002"
#define SOCKET_ERROR_NotLoggedIn(target) send(target, CODE_SERVER_NotLoggedIn_ERROR, sizeof(CODE_SERVER_NotLoggedIn_ERROR), 0);
// 不存在错误
#define CODE_SERVER_NoExist_ERROR "e003"
#define SOCKET_ERROR_ConnectObject_NoExist(target) send(target, CODE_SERVER_NoExist_ERROR, sizeof(CODE_SERVER_NoExist_ERROR), 0);
// 未选择客户端错误
#define CODE_SERVER_NoConnect_ERROR "e004"
#define SOCKET_ERROR_NoConnect(target) send(target, CODE_SERVER_NoConnect_ERROR, sizeof(CODE_SERVER_NoConnect_ERROR), 0);
// 验证错误
#define CODE_SERVER_VERIFICATION_ERROR "e005"
#define SOCKET_ERROR_VERIFICATION(target) send(target, CODE_SERVER_VERIFICATION_ERROR, sizeof(CODE_SERVER_VERIFICATION_ERROR), 0);

/*______ L O C A L - F U N C T I O N _________________________________________*/

/// @brief convert char to int
/// @param str
/// @return code
int convert_hexChar_to_int(std::string &str);

/// @brief convert char to int
/// @param str
/// @return code
int convert_Char_to_int(std::string &_str);

/// @brief convert int to char
/// @param _num
/// @param _ret
void convert_int_to_char(int _num, std::string &_ret);

/*______ F U N C T I O N _____________________________________________________*/

Server_impl::Server_impl(/* args */)
{
    m_domain = SOCKET_CONFIG_DOMAIN;     // IPv4
    m_type = SOCKET_CONFIG_TYPE;         // Types of sockets
    m_protocol = SOCKET_CONFIG_PROTOCOL; // protocols
    m_port = SOCKET_CONFIG_PORT;         // port
}

Server_impl::~Server_impl() {}

int Server_impl::Server_Init(int port)
{
    Log_debug("Server_Init: Start");

    if (isInit)
    {
        Log_warn("Server_Init: The server has been initialized");
        return SOCKET_ERROR;
    }

    m_port = port;
    Log_info("---- Chat Server\tversion:%0.2f ----", VERSION);

    // socket
    Log_debug("Server_Init: init");
    int ret = socket(m_domain, m_type, m_protocol);
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: init fail");
        return SOCKET_ERROR;
    }
    m_socket_server = ret;

    // Set non-blocking
    int flags = fcntl(m_socket_server, F_GETFL, 0);
    fcntl(m_socket_server, F_SETFL, flags | O_NONBLOCK);

    // bind
    Log_debug("Server_Init: bind");
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret = bind(m_socket_server, (struct sockaddr *)&addr, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: bind fail");
        return ret;
    }

    // listen
    Log_info("Server_Init: listen (port:%d) ...", m_port);
    ret = listen(m_socket_server, 5);
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: listen fail");
        return ret;
    }

    Log_debug("Server_Init: socket init 127.0.0.1:%d", m_port);

    // Init fd array
    for (int i = 0; i < FD_ARRAY_MAXSIZE; ++i)
    {
        m_fd_array[i] = -1;
    }
    m_fd_array[0] = m_socket_server;
    m_fd_array_length = 1;

    // create task thread
    m_thread_task = std::thread(&Server_impl::Server_Thread_Task, this);

    // Init end
    isInit = true;
    Log_debug("Server_Init: End");

    return 1;
}

void Server_impl::Server_Update()
{
    Log_debug("Server_Update: Start");

    //
    if (m_TerminateFlag)
    {
        Log_warn("Server_Update: Terminate Flag is true");
        return;
    }

    // Event processing
    fd_set readfds, writefds, errorfds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);
    int max_fd = m_fd_array[0];

    // fill fd_set
    int temp_count = 0;
    for (int i = 0; i < FD_ARRAY_MAXSIZE; ++i)
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
        temp_count++;

        if (m_fd_array_length <= temp_count)
        {
            break;
        }
    }

    // 同时监视多个文件描述符(socket)
    //  获取可读、可写、异常的文件描述符
    //  __timeout: 设置等待时间; 0:非阻塞; NULL:阻塞; 其他数值:等待指定时间;
    int ret = select(max_fd + 1, &readfds, &writefds, &errorfds, 0);

    Log_debug("Server_Update: select End \n");

    //
    if (SELECT_ERROR == ret)
    {
        Log_error("select error");
        return;
    }
    else if (SELECT_TIME_OUT == ret)
    {
        Log_warn("select timeout");
        return;
    }
    else
    {
        // server socket readable
        //  the server have new client
        if (FD_ISSET(m_socket_server, &readfds))
        {
            Server_Accept_Client();
        }

        //
        Server_Receive_Event(readfds);
    }

    Log_debug("Server_Update: End \n");
}

void Server_impl::Server_Receive_Event(fd_set &_readfds)
{
    Log_debug("Server_Receive_Event: Start");

    char recvBuff[BUFF_MAX_SIZE];
    std::string str_recvBuff, str_codeBuff, str_message;
    int temp_count = 1; // 已处理数量

    for (int i = 1; i < FD_ARRAY_MAXSIZE; ++i)
    {
        // check socket read state
        if (m_fd_array[i] == -1 || !FD_ISSET(m_fd_array[i], &_readfds))
        {
            continue;
        }
        Log_debug("Server_Receive_Event: %s:%d Client have new message", m_client_list[m_fd_array[i]].IP, m_client_list[m_fd_array[i]].Port);

        // receive data
        memset(recvBuff, 0, BUFF_MAX_SIZE);
        if (recv(m_fd_array[i], recvBuff, BUFF_MAX_SIZE, 0) <= 0)
        {
            Server_Disconnect_Client(i);
            Log_error("Server_Receive_Event: client connection down");
            continue;
        }
        Log_debug("Server_Receive_Event: recvBuff: %s", recvBuff);
        str_recvBuff = recvBuff;
        temp_count++;

        // Get the message type
        str_codeBuff = str_recvBuff.substr(0, 4);
        int code = convert_hexChar_to_int(str_codeBuff);
        if (SOCKET_ERROR == code)
        {
            SOCKET_ERROR_RECV(m_fd_array[i]);
            Log_error("Server_Receive_Event: %s:%d send an error code", m_client_list[m_fd_array[i]].IP, m_client_list[m_fd_array[i]].Port);
            continue;
        }

        // get 4bit high code
        str_codeBuff = str_recvBuff.substr(4, 4);
        int high = convert_Char_to_int(str_codeBuff);

        // get 4bit low code
        str_codeBuff = str_recvBuff.substr(8, 4);
        int low = convert_Char_to_int(str_codeBuff);

        // get message
        str_message = str_recvBuff.substr(12, str_recvBuff.length() - 12);

        // add task
        Log_debug("Server_Receive_Event: code:%d, 4bit high:%d, 4bit low:%d, str_message:%s", code, high, low, str_message.c_str());
        s_event_task task = s_event_task{m_fd_array[i], code, high, low, str_message};
        {
            std::unique_lock<std::mutex> lock(m_mutex_task);
            m_queue_task.push(task);
            this->m_new_task = true;
            m_condition_task.notify_one();
        }

        // process all client, return
        if (m_fd_array_length <= temp_count)
        {
            break;
        }
    }

    Log_debug("Server_Receive_Event: End \n");
}

void Server_impl::Server_Accept_Client()
{
    Log_debug("Server_Accept_Client: Start");

    // return when array is full
    if (m_fd_array_length >= FD_ARRAY_MAXSIZE)
    {
        Log_warn("The maximum number of connections is reached");
        return;
    }

    // accept new client
    sockaddr_in clientAddr;
    int len = sizeof(clientAddr);
    int connect = accept(m_socket_server, (sockaddr *)&clientAddr, (socklen_t *)&len);
    if (SOCKET_ERROR == connect)
    {
        Log_error("The client connection failure");
        close(connect);
        return;
    }

    // get the client
    struct s_socket_client client;
    client.Socket = connect;
    client.Socket_Addr = clientAddr;

    // get the IP and port
    inet_ntop(AF_INET, &client.Socket_Addr.sin_addr, client.IP, INET_ADDRSTRLEN);
    client.Port = ntohs(client.Socket_Addr.sin_port);

    // save the information
    m_client_list[client.Socket] = client;

    Log_info("Server_Accept_Client: accept new client %s:%d", client.IP, client.Port);

    // save into fd list
    if (m_fd_array[m_fd_array_length] == -1)
    {
        m_fd_array[m_fd_array_length] = connect;
        m_fd_array_length++;
    }
    else
    {
        for (int i = 1; i < FD_ARRAY_MAXSIZE; ++i)
        {
            if (m_fd_array[i] == -1)
            {
                m_fd_array[i] = connect;
                m_fd_array_length++;
                break;
            }
        }
    }

    Log_debug("Server_Accept_Client: End \n");
}

void Server_impl::Server_Disconnect_Client(int _index)
{
    //
    std::string client_userID = m_client_list[m_fd_array[_index]].UserID;
    if (!m_client_list.empty())
    {
        m_client_list.erase(m_fd_array[_index]);
    }
    if (!m_users.empty() && !m_users[client_userID].UserID.empty())
    {
        m_users.erase(client_userID);
    }

    // 修改用户状态
    Mysql::instance()->Mysql_Set_userState(client_userID, UserState::UserState_Offline);

    //
    close(m_fd_array[_index]);
    m_fd_array[_index] = -1;
    m_fd_array_length--;
}

void Server_impl::Server_Thread_Task()
{
    Log_debug("Server_Thread_Task: Start. thread id: %d", std::this_thread::get_id());

    int ret;

    while (!m_TerminateFlag)
    {
        // wait for task
        bool empty = false;
        {
            std::unique_lock<std::mutex> lock(m_mutex_task);
            empty = m_queue_task.empty();
        }
        if (empty)
        {
            this->m_new_task = false;
            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            Log_debug("Server_Thread_Task: wait for task \n");
            m_condition_task.wait(lock, [this]
                                  { return this->m_new_task | this->m_TerminateFlag; });
            Log_debug("Server_Thread_Task: be notify");
        }

        if (this->m_TerminateFlag)
            return;

        // get task from queue
        s_event_task task;
        {
            std::unique_lock<std::mutex> lock(m_mutex_task);
            task = m_queue_task.front();
            m_queue_task.pop();
        }
        if (task.socket == -1)
        {
            Log_warn("Server_Thread_Task: task is null");
            continue;
        }
        Log_debug("Server_Thread_Task: socket:%d, code:%d, bits_high:%d, bits_low:%d, recvBuff:%s", task.socket, task.code, task.bits_high, task.bits_low, task.recvBuff.c_str());

        // get user
        std::string user_id = m_client_list[task.socket].UserID;

        // process task
        // connect server
        if (CODE_CONNECT_SERVER == task.code)
        {
            Log_debug("Server_Thread_Task: CODE_CONNECT_SERVER");

            std::string recv_userID = task.recvBuff.substr(0, task.bits_high);
            std::string userName, recv_password;

            // 验证数据库
            recv_password = task.recvBuff.substr(task.bits_high, task.bits_low);
            ret = Mysql::instance()->Mysql_check_user(recv_userID, recv_password, userName);
            // 获取结果
            switch (ret)
            {
            case -1:
                Log_fatal("Server_Thread_Task: Mysql_check_user mysql server error");
                continue;
            case -2:
                Log_debug("Server_Thread_Task: Mysql_check_user fail");
                SOCKET_ERROR_VERIFICATION(task.socket);
                continue;
            }

            // 填充数据
            Log_debug("Server_Thread_Task: Mysql_check_user success");
            Log_debug("Server_Thread_Task: userName:%s", userName.c_str());
            user_id = m_client_list[task.socket].UserID = recv_userID;
            m_users[user_id] = s_user_Information{task.socket, user_id, userName};

            // 组建报文 发送
            {
                std::stringstream strstream;
                std::string len;

                convert_int_to_char(userName.length(), len);
                strstream << CODE_LOGIN_RETURN << len << "0000" << userName;
                std::string buff = strstream.str();
                Log_debug("Server_Thread_Task: send commond:%s", buff.c_str());

                // 返回用户名
                send(task.socket, buff.c_str(), buff.length(), 0);
                Log_info("Server_Thread_Task: %s : %s connect server", user_id.c_str(), userName.c_str());
            }

            // 修改用户状态
            Mysql::instance()->Mysql_Set_userState(user_id, UserState::UserState_Online);

            // 获取好友列表
            std::vector<s_Friend_info> userList;
            ret = Mysql::instance()->Mysql_Get_friendList(user_id, userList);
            // 获取结果
            switch (ret)
            {
            case -1:
                Log_fatal("Server_Thread_Task: Mysql_check_user mysql server error");
                continue;
            case -2:
                Log_debug("Server_Thread_Task: Mysql_check_user fail");
                SOCKET_ERROR_VERIFICATION(task.socket);
                continue;
            }

            //
            for (int i = 0, size = userList.size(); i < size; ++i)
            {
                std::stringstream strstream;
                std::string len_id, len_name;

                convert_int_to_char(userList[i].UserID.length(), len_id);
                convert_int_to_char(userList[i].UserName.length(), len_name);
                strstream << CODE_FRIEND_RETURN << len_id << len_name << userList[i].UserID << userList[i].UserName;
                std::string buff = strstream.str();

                // 返回列表
                send(task.socket, buff.c_str(), buff.length(), 0);
                Log_debug("Server_Thread_Task: return %s's friend, buff:%s ", userName.c_str(), buff.c_str());
            }
        }
        // connect client
        else if (CODE_CONNECT_CLIENT == task.code)
        {
            Log_debug("Server_Thread_Task: CODE_CONNECT_CLIENT");

            if (m_users[user_id].UserID == "")
            {
                Log_warn("Server_Thread_Task: connect object's User ID is null");
                SOCKET_ERROR_NotLoggedIn(task.socket);
                continue;
            }

            if (task.bits_low == task.recvBuff.length())
            {
                m_users[user_id].Connect_UserID = task.recvBuff;
                Log_debug("Server_Thread_Task: %s connect %s", m_users[user_id].UserID, m_users[user_id].Connect_UserID);
            }
            else
            {
                Log_warn("Server_Thread_Task: recvBuff length:%d is error,low bit:%d", task.recvBuff.length(), task.bits_low);
                continue;
            }
        }
        // client send message
        else if (CODE_SEND_MESSAGE == task.code)
        {
            Log_debug("Server_Thread_Task: CODE_SEND_MESSAGE");

            if (m_users[user_id].UserID == "")
            {
                Log_warn("Server_Thread_Task: send object's User ID is null");
                SOCKET_ERROR_NotLoggedIn(task.socket);
                continue;
            }

            //
            std::string connectID = task.recvBuff.substr(0, task.bits_high);
            m_users[user_id].Connect_UserID = connectID;
            Log_debug("Server_Thread_Task: send object: connectID:%s, socket:%d", m_users[user_id].Connect_UserID.c_str(), m_users[connectID].Socket);
            if (connectID == "")
            {
                Log_warn("Server_Thread_Task: no connect other client");
                SOCKET_ERROR_NoConnect(task.socket);
                continue;
            }
            else if (m_users[connectID].Socket == -1)
            {
                Log_warn("Server_Thread_Task: send object is no exist");
                SOCKET_ERROR_ConnectObject_NoExist(task.socket);
                continue;
            }

            //
            std::stringstream strstream;
            std::string userID_len, msg_len;

            convert_int_to_char(user_id.length(), userID_len);
            convert_int_to_char(task.bits_low, msg_len);
            strstream << CODE_LOGIN_RETURN << userID_len << msg_len << user_id << task.recvBuff.substr(task.bits_high, task.bits_low);
            std::string buff = strstream.str();

            //
            send(m_users[connectID].Socket, buff.c_str(), buff.size(), 0);
            Log_debug("Server_Thread_Task: %s:%d send message to %s:%d, send message:%s",
                      m_client_list[task.socket].IP, m_client_list[task.socket].Port,
                      m_client_list[m_users[connectID].Socket].IP, m_client_list[m_users[connectID].Socket].Port,
                      buff.c_str());
        }
        else
        {
            Log_error("Server_Thread_Task: code error");
            SOCKET_ERROR_RECV(task.socket);
            continue;
        }
    }

    Log_debug("Server_Thread_Task: End, thread id: %d", std::this_thread::get_id());
}

void Server_impl::Server_Reg_recvMsgCallbackFunc(recvMsg_CallbackFunc _callback)
{
    m_recvMsgCallback = _callback;
}

/*______ L O C A L - F U N C T I O N _________________________________________*/

int convert_hexChar_to_int(std::string &str)
{
    int temp[4];
    int ret = 0, size = 4;

    for (int i = 0; i < size; ++i)
    {
        if (str[i] >= 48 && str[i] <= 57)
        {
            temp[i] = str[i] - 48;
        }
        else if (str[i] >= 65 && str[i] <= 70)
        {
            temp[i] = str[i] - 55;
        }
        ret += temp[i] * pow(16, size - 1 - i);
    }

    if (ret < 0)
        return -1;
    else
        return ret;
}

int convert_Char_to_int(std::string &_str)
{
    int temp[4];
    int ret = 0, size = 4;

    for (int i = 0; i < size; ++i)
    {
        if (_str[i] >= 48 && _str[i] <= 57)
        {
            temp[i] = _str[i] - 48;
        }
        ret += temp[i] * pow(10, size - 1 - i);
    }

    if (ret < 0)
        return -1;
    else
        return ret;
}

void convert_int_to_char(int _num, std::string &_ret)
{
    int size = 4;
    char temp[4];

    for (int i = 0; i < size; ++i)
    {
        int series = pow(10, size - i - 1);
        temp[i] = (char)((_num / series) + 48);
        _num = _num % series;
    }

    _ret = temp;
}
