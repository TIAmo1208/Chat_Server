
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Server_impl.hpp"

#include "Log.h" // Log
#include <cstring>
#include <memory>
#include <mutex>
using namespace Log;

#include "Mysql.h" // Mysql

#include <math.h>
#include <thread>

/*______ D E F I N E _________________________________________________________*/

#define CODE_MIN_SIZE 4

#define CODE_CONNECT_SERVER 0000    // 客户端登录
#define CODE_CONNECT_CLIENT 0001    // 连接其他客户端
#define CODE_SEND_MESSAGE 0002      // 客户端发送信息
#define CODE_SEND_FILE_HEAD 0010    // 客户端发送文件信息
#define CODE_SEND_FILE_CONTENT 0011 // 客户端发送文件内容

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
// 文件错误
#define CODE_SERVER_FILE_ERROR "e010"
#define SOCKET_ERROR_FILE(target) send(target, CODE_SERVER_FILE_ERROR, sizeof(CODE_SERVER_FILE_ERROR), 0);
// CRC校验错误
#define CODE_SERVER_CRC_ERROR "e011"
#define SOCKET_ERROR_CRC(target, block) send(target, block, sizeof(block), 0);

/*______ F U N C T I O N _____________________________________________________*/

Server_impl::Server_impl(/* args */)
{
    this->m_domain   = SOCKET_CONFIG_DOMAIN;   // IPv4
    this->m_type     = SOCKET_CONFIG_TYPE;     // Types of sockets
    this->m_protocol = SOCKET_CONFIG_PROTOCOL; // protocols
    this->m_port     = SOCKET_CONFIG_PORT;     // port
}

Server_impl::~Server_impl()
{
    m_TerminateFlag = true;
    this->m_queue_task.quit();
    this->m_threadPool->ThreadPool_join();
}

int Server_impl::Server_Init(int port, int threadNum)
{
    Log_debug("Server_Init: Start");

    if (isInit)
    {
        Log_warn("Server_Init: The server has been initialized");
        return SOCKET_ERROR;
    }

    this->m_port = port;
    Log_info("---- Chat Server\tversion:%0.2f ----", VERSION);

    // socket
    Log_debug("Server_Init: init");
    int ret = socket(this->m_domain, this->m_type, this->m_protocol);
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: init fail");
        return SOCKET_ERROR;
    }
    this->m_socket_server = ret;

    // Set non-blocking
    int flags = fcntl(this->m_socket_server, F_GETFL, 0);
    fcntl(this->m_socket_server, F_SETFL, flags | O_NONBLOCK);

    // bind
    Log_debug("Server_Init: bind");
    sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(this->m_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    ret                  = bind(this->m_socket_server, (struct sockaddr *) &addr, sizeof(sockaddr_in));
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: bind fail");
        return ret;
    }

    // listen
    Log_info("Server_Init: listen (port:%d) ...", this->m_port);
    ret = listen(this->m_socket_server, 5);
    if (SOCKET_ERROR == ret)
    {
        Log_error("Server_Init: listen fail");
        return ret;
    }

    Log_debug("Server_Init: socket init 127.0.0.1:%d", this->m_port);

    // Init fd array
    this->m_fd_array.add(0, this->m_socket_server);

    // create task thread
    this->m_threadPool = std::make_shared<ThreadPool>(threadNum);
    // 等待一段时间，让线程池完成初始化，防止任务加入失败
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    //
    this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Task_Processing, this);

    // Init end
    isInit = true;
    Log_debug("Server_Init: End");

    return 1;
}

void Server_impl::Server_Update()
{
    Log_debug("Server_Update: Start");

    //
    if (!this->isInit)
    {
        Log_warn("Server_Update: uninitialized !!!");
        return;
    }
    if (this->m_TerminateFlag)
    {
        Log_warn("Server_Update: Terminate Flag is true");
        return;
    }

    // Event processing
    fd_set readfds, writefds, errorfds;

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&errorfds);
    int max_fd = this->m_fd_array[0];

    // fill fd_set
    int temp_count = 0;
    for (int i = 0; i < FD_ARRAY_MAXSIZE; ++i)
    {
        if (this->m_fd_array[i] == -1)
        {
            continue;
        }

        Log_debug("Server_Update: add fd[%d]:%d ", i, this->m_fd_array[i]);

        FD_SET(this->m_fd_array[i], &readfds);
        if (this->m_fd_array[i] > max_fd)
        {
            max_fd = this->m_fd_array[i];
        }
        temp_count++;

        if (m_fd_array.lenght() <= temp_count)
        {
            break;
        }
    }

    // 同时监视多个文件描述符(socket)
    //  获取可读、可写、异常的文件描述符
    //  __timeout: 设置等待时间; 0:非阻塞; NULL:阻塞; 其他数值:等待指定时间;
    int ret = select(max_fd + 1, &readfds, &writefds, &errorfds, 0);

    Log_debug("Server_Update: select End ");

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
        if (FD_ISSET(this->m_socket_server, &readfds))
        {
            // this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Accept_Client,
            // this);
            Server_Accept_Client();
        }

        // this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Receive_Event,
        // this, readfds);
        Server_Receive_Event(readfds);
    }

    Log_debug("Server_Update: End ");
}

void Server_impl::Server_Receive_Event(fd_set _readfds)
{
    Log_debug("Server_Receive_Event: Start");

    char recvBuff[BUFF_MAX_SIZE];
    std::string str_recvBuff, str_subTemp;
    int temp_count = 1; // 已处理数量

    for (int i = 1; i < FD_ARRAY_MAXSIZE; ++i)
    {
        // check socket read state
        if (this->m_fd_array[i] == -1 || !FD_ISSET(this->m_fd_array[i], &_readfds))
        {
            continue;
        }
        Log_debug("Server_Receive_Event: %s:%d Client have new message", this->m_client_list[this->m_fd_array[i]]->IP, this->m_client_list[this->m_fd_array[i]]->Port);

        // receive data
        memset(recvBuff, 0, BUFF_MAX_SIZE);
        if (recv(this->m_fd_array[i], recvBuff, BUFF_MAX_SIZE, 0) <= 0)
        {
            Server_Disconnect_Client(i);
            continue;
        }
        Log_debug("Server_Receive_Event: recvBuff:%s, length:%d", recvBuff, strlen(recvBuff));
        temp_count++;
        if (strlen(recvBuff) < CODE_MIN_SIZE)
        {
            continue;
        }
        str_recvBuff = recvBuff;

        //
        EventTask task = std::make_shared<s_event_task>();
        task->socket   = this->m_fd_array[i];

        // Get the message type
        str_subTemp = str_recvBuff.substr(0, CODE_LENGHT);
        task->code  = m_tools.Server_Tools_convert_Char_to_int(str_subTemp);

        // get 4bit high code
        str_subTemp     = str_recvBuff.substr(CODE_POSITION_HIGHT, CODE_LENGHT);
        task->bits_high = m_tools.Server_Tools_convert_Char_to_int(str_subTemp);

        // get 4bit low code
        str_subTemp    = str_recvBuff.substr(CODE_POSITION_LOW, CODE_LENGHT);
        task->bits_low = m_tools.Server_Tools_convert_Char_to_int(str_subTemp);

        // get message
        task->recvBuff = str_recvBuff.substr(12, str_recvBuff.length() - 12);

        // add task
        Log_debug("Server_Receive_Event: code:%d, 4bit high:%d, 4bit low:%d, "
                  "str_message:%s",
                  task->code, task->bits_high, task->bits_low, task->recvBuff.c_str());
        //
        this->m_queue_task.push(task);
        Log_debug("Server_Receive_Event: notify one task");

        // process all client, return
        if (m_fd_array.lenght() <= temp_count)
        {
            break;
        }
    }

    Log_debug("Server_Receive_Event: End ,count:%d", temp_count);
}

void Server_impl::Server_Accept_Client()
{
    Log_debug("Server_Accept_Client: Start");

    // return when array is full
    if (m_fd_array.lenght() >= FD_ARRAY_MAXSIZE)
    {
        Log_warn("The maximum number of connections is reached, length:%d", m_fd_array.lenght());
        return;
    }

    // accept new client
    sockaddr_in clientAddr;
    int len     = sizeof(clientAddr);
    int connect = accept(this->m_socket_server, (sockaddr *) &clientAddr, (socklen_t *) &len);
    if (SOCKET_ERROR == connect)
    {
        Log_error("The client connection failure");
        close(connect);
        return;
    }

    // get the client
    struct s_socket_client client;
    client.Socket      = connect;
    client.Socket_Addr = clientAddr;

    // get the IP and port
    inet_ntop(AF_INET, &client.Socket_Addr.sin_addr, client.IP, INET_ADDRSTRLEN);
    client.Port = ntohs(client.Socket_Addr.sin_port);

    // save the information
    this->m_client_list[client.Socket] = std::make_shared<s_socket_client>(client);

    Log_info("Server_Accept_Client: accept new client %s:%d", client.IP, client.Port);

    // save into fd list
    if (this->m_fd_array[m_fd_array.lenght()] == -1)
    {
        this->m_fd_array.add(m_fd_array.lenght(), connect);
    }
    else
    {
        for (int i = 1; i < FD_ARRAY_MAXSIZE; ++i)
        {
            if (this->m_fd_array[i] == -1)
            {
                this->m_fd_array.add(i, connect);
                break;
            }
        }
    }

    Log_debug("Server_Accept_Client: End ");
}

void Server_impl::Server_Disconnect_Client(int _index)
{
    //
    std::string client_userID;
    client_userID = this->m_client_list[this->m_fd_array[_index]]->UserID;
    if (!this->m_client_list.empty())
    {
        this->m_client_list.erase(this->m_fd_array[_index]);
    }

    if (!this->m_users.empty() && !this->m_users[client_userID]->UserID.empty())
    {
        this->m_users.erase(client_userID);
    }

    // 修改用户状态
    Mysql::instance()->Mysql_Set_userState(client_userID, UserState::UserState_Offline);

    //
    close(this->m_fd_array[_index]);
    Log_info("Server_Receive_Event: client:%s:%d connection down", this->m_client_list[this->m_fd_array[_index]]->IP, this->m_client_list[this->m_fd_array[_index]]->Port);
    this->m_fd_array.erase(_index);
}

void Server_impl::Server_Task_Processing()
{
    Log_debug("Server_Task_Processing: Start. thread id: %d", std::this_thread::get_id());

    int ret;

    while (!this->m_TerminateFlag)
    {
        // wait and get task from queue
        Log_debug("Server_Task_Processing: wait for task ");
        EventTask task = this->m_queue_task.pop();
        Log_debug("Server_Task_Processing: be notify");

        //
        if (this->m_TerminateFlag)
            break;

        if (task->socket == -1)
        {
            Log_warn("Server_Task_Processing: task is null");
            continue;
        }
        Log_debug("Server_Task_Processing: socket:%d, code:%d, bits_high:%d, "
                  "bits_low:%d, recvBuff:%s",
                  task->socket, task->code, task->bits_high, task->bits_low, task->recvBuff.c_str());
        Log_debug("Server_Task_Processing: task use_count :%d", task.use_count());

        // get user
        std::string user_id;
        user_id = this->m_client_list[task->socket]->UserID;

        // process task
        // connect server
        if (CODE_CONNECT_SERVER == task->code)
        {
            Log_debug("Server_Task_Processing: add task:CODE_CONNECT_SERVER");
            this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_ConnectServer, this, user_id, std::move(task));
        }
        // connect client
        else if (CODE_CONNECT_CLIENT == task->code)
        {
            Log_debug("Server_Task_Processing: add task:CODE_CONNECT_SERVER");
            this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_ConnectClient, this, user_id, std::move(task));
        }
        // client send message
        else if (CODE_SEND_MESSAGE == task->code)
        {
            Log_debug("Server_Task_Processing: add task:CODE_CONNECT_SERVER");
            this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_SendMessage, this, user_id, std::move(task));
        }
        // client send file information
        else if (CODE_SEND_FILE_HEAD == task->code)
        {
            Log_debug("Server_Task_Processing: add task:CODE_SEND_FILE");
            this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_SendFile, this, user_id, std::move(task));
        }
        // client send file content
        else if (CODE_SEND_FILE_CONTENT == task->code)
        {
            Log_debug("Server_Task_Processing: add task:CODE_SEND_FILE_CONTENT");
            this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_SendFile, this, user_id, std::move(task));
        }
        else
        {
            Log_error("Server_Task_Processing: code error");
            SOCKET_ERROR_RECV(task->socket);
            continue;
        }
    }

    Log_debug("Server_Task_Processing: End, thread id: %d", std::this_thread::get_id());
}

void Server_impl::Server_Process_ConnectServer(std::string _sendUserID, EventTask _task)
{
    Log_debug("Server_Process_ConnectServer: CODE_CONNECT_SERVER");

    std::string recv_userID, userName, recv_password;
    int ret = -1;

    // 验证数据库
    recv_userID   = _task->recvBuff.substr(0, _task->bits_high);
    recv_password = _task->recvBuff.substr(_task->bits_high, _task->bits_low);
    ret           = Mysql::instance()->Mysql_check_user(recv_userID, recv_password, userName);
    // 获取结果
    switch (ret)
    {
    case -1:
        Log_fatal("Server_Process_ConnectServer: Mysql_check_user mysql server "
                  "error");
        return;
    case -2:
        Log_debug("Server_Process_ConnectServer: Mysql_check_user fail");
        SOCKET_ERROR_VERIFICATION(_task->socket);
        return;
    }

    // 填充数据
    Log_debug("Server_Process_ConnectServer: Mysql_check_user success");
    Log_debug("Server_Process_ConnectServer: userName:%s", userName.c_str());
    _sendUserID = this->m_client_list[_task->socket]->UserID = recv_userID;

    this->m_users[_sendUserID] = std::make_shared<s_user_Information>(s_user_Information{_task->socket, _sendUserID, userName});

    // 组建报文 发送
    {
        std::stringstream strstream;
        std::string len;

        m_tools.Server_Tools_convert_int_to_char(userName.length(), len);
        strstream << CODE_LOGIN_RETURN << len << "0000" << userName;
        std::string buff = strstream.str();
        Log_debug("Server_Process_ConnectServer: send commond:%s", buff.c_str());

        // 返回用户名
        send(_task->socket, buff.c_str(), buff.length(), 0);
        Log_info("Server_Process_ConnectServer: %s : %s connect server", _sendUserID.c_str(), userName.c_str());
    }

    // 修改用户状态
    Mysql::instance()->Mysql_Set_userState(_sendUserID, UserState::UserState_Online);

    // 添加任务：获取返回好友列表
    this->m_threadPool->ThreadPool_add_Task(&Server_impl::Server_Process_ReturnFriendList, this, _sendUserID, _task, userName);
}

void Server_impl::Server_Process_ConnectClient(std::string _sendUserID, EventTask _task)
{
    Log_debug("Server_Process_ConnectClient: CODE_CONNECT_CLIENT");

    if (this->m_users[_sendUserID]->UserID == "")
    {
        Log_warn("Server_Process_ConnectClient: connect object's User ID is null");
        SOCKET_ERROR_NotLoggedIn(_task->socket);
        return;
    }

    if (_task->bits_low == _task->recvBuff.length())
    {
        this->m_users[_sendUserID]->Connect_UserID = _task->recvBuff;
        Log_debug("Server_Process_ConnectClient: %s connect %s", this->m_users[_sendUserID]->UserID.c_str(), this->m_users[_sendUserID]->Connect_UserID.c_str());
    }
    else
    {
        Log_warn("Server_Process_ConnectClient: recvBuff length:%d is "
                 "error,low bit:%d",
                 _task->recvBuff.length(), _task->bits_low);
        return;
    }
}

void Server_impl::Server_Process_SendMessage(std::string _sendUserID, EventTask _task)
{
    Log_debug("Server_Process_SendMessage: CODE_SEND_MESSAGE");

    std::string connectID;
    int connectSocket;
    {
        if (this->m_users[_sendUserID]->UserID == "")
        {
            Log_warn("Server_Process_SendMessage: send object's User ID is null");
            SOCKET_ERROR_NotLoggedIn(_task->socket);
            return;
        }

        //
        connectID     = _task->recvBuff.substr(0, _task->bits_high);
        connectSocket = this->m_users[connectID]->Socket;

        this->m_users[_sendUserID]->Connect_UserID = connectID;
        Log_debug("Server_Process_SendMessage: send object: connectID:%s, socket:%d", this->m_users[_sendUserID]->Connect_UserID.c_str(), connectSocket);
        if (connectID == "")
        {
            Log_warn("Server_Process_SendMessage: no connect other client");
            SOCKET_ERROR_NoConnect(_task->socket);
            return;
        }
        else if (connectSocket == -1)
        {
            Log_warn("Server_Process_SendMessage: send object is no exist");
            SOCKET_ERROR_ConnectObject_NoExist(_task->socket);
            return;
        }
    }

    //
    std::stringstream strstream;
    std::string userID_len, msg_len;

    m_tools.Server_Tools_convert_int_to_char(_sendUserID.length(), userID_len);
    m_tools.Server_Tools_convert_int_to_char(_task->bits_low, msg_len);
    strstream << CODE_TRANSMIT_MESSAGE << userID_len << msg_len << _sendUserID << _task->recvBuff.substr(_task->bits_high, _task->bits_low).c_str();
    std::string buff = strstream.str();

    //
    send(connectSocket, buff.c_str(), buff.size(), 0);
    Log_debug("Server_Process_SendMessage: %s:%d send message to %s:%d, send message:%s", this->m_client_list[_task->socket]->IP, this->m_client_list[_task->socket]->Port,
              this->m_client_list[connectSocket]->IP, this->m_client_list[connectSocket]->Port, buff.c_str());
}

void Server_impl::Server_Process_SendFile(std::string _sendUserID, EventTask _task)
{
    Log_debug("Server_Process_SendFile: CODE_SEND_FILE");

    if (this->m_users[_sendUserID]->UserID == "")
    {
        Log_warn("Server_Process_SendFile: send object's User ID is null");
        SOCKET_ERROR_NotLoggedIn(_task->socket);
        return;
    }

    if (_task->code == CODE_SEND_FILE_HEAD)
    {
        // create task
        Server_Tools::s_FileTask task;
        {
            task.fileName       = _task->recvBuff.substr(0, _task->bits_high);
            std::string str_tmp = _task->recvBuff.substr(_task->bits_high, _task->bits_low);
            task.fileSize       = m_tools.Server_Tools_convert_Char_to_int(str_tmp);

            task.sendTarget = _task->socket;
        }

        // test open file
        int ret = m_tools.Server_Tools_open_file(task);
        if (ret == Return_Code::File_Open_Fail)
        {
            SOCKET_ERROR_FILE(_task->socket);
        }
    }
    else if (_task->code == CODE_SEND_FILE_CONTENT)
    {
        // create task
        Server_Tools::s_FileBlock *task = new Server_Tools::s_FileBlock;

        // test open file
        int ret = m_tools.Server_Tools_save_file(task);
        if (ret == Return_Code::CRC_Check_Fail)
        {
            char temp[8];
            sprintf(temp, "%s%d", CODE_SERVER_CRC_ERROR, task->block_num);
            SOCKET_ERROR_CRC(_task->socket, temp);
        }
    }
}

void Server_impl::Server_Process_ReturnFriendList(std::string _sendUserID, EventTask _task, std::string _userName)
{
    // 获取好友列表
    std::vector<s_Friend_info> userList;
    int ret = Mysql::instance()->Mysql_Get_friendList(_sendUserID, userList);
    // 获取结果
    switch (ret)
    {
    case -1:
        Log_fatal("Server_Process_ReturnFriendList: Mysql_check_user mysql "
                  "server error");
        return;
    case -2:
        Log_debug("Server_Process_ReturnFriendList: Mysql_check_user fail");
        SOCKET_ERROR_VERIFICATION(_task->socket);
        return;
    }

    // 返回好友列表到 Client
    for (int i = 0, size = userList.size(); i < size; ++i)
    {
        std::stringstream strstream;
        std::string len_id, len_name;

        m_tools.Server_Tools_convert_int_to_char(userList[i].UserID.length(), len_id);
        m_tools.Server_Tools_convert_int_to_char(userList[i].UserName.length(), len_name);
        strstream << CODE_FRIEND_RETURN << len_id << len_name << userList[i].UserID << userList[i].UserName;
        std::string buff = strstream.str();

        // 返回列表
        send(_task->socket, buff.c_str(), buff.length(), 0);
        Log_debug("Server_Process_ReturnFriendList: return %s's friend, buff:%s ", _userName.c_str(), buff.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Server_impl::Server_Reg_recvMsgCallbackFunc(recvMsg_CallbackFunc _callback) { this->m_recvMsgCallback = _callback; }
