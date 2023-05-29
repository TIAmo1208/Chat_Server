#ifndef __SERVER_INTERFACE_H__
#define __SERVER_INTERFACE_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#ifdef __linux__
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <memory>
#include <string>

/*______ S T R U C T _________________________________________________________*/

// client list
struct s_socket_client
{
    std::string UserID; // client's ID

    int Socket; // client`s socket
    char IP[INET_ADDRSTRLEN];
    int Port;
    sockaddr_in Socket_Addr; // client`s addr
};
using ClientInfo = std::shared_ptr<s_socket_client>;

// users
struct s_user_Information
{
    int Socket = -1;

    std::string UserID         = "";
    std::string UserName       = "";
    std::string Connect_UserID = "";
};
using UserInfo = std::shared_ptr<s_user_Information>;

// task
struct s_event_task
{
    int socket    = -1;
    int code      = -1;
    int bits_high = -1;
    int bits_low  = -1;

    std::string recvBuff = "";
};
using EventTask = std::shared_ptr<s_event_task>;

/*______ C L A S S ___________________________________________________________*/

/**
 * @brief The Insterface of Server
 */
class Server_Interface
{
public:
    virtual ~Server_Interface() {}

    /**
     * @brief Server initialization
     *
     * @param port
     * @return int
     */
    virtual int Server_Init(int port, int threadNum) = 0;

    /**
     * @brief main Update
     *
     */
    virtual void Server_Update() = 0;

    /**
     * @brief The Callback Function Of Recive Message
     *
     */
    using recvMsg_CallbackFunc = void (*)(std::string _recvMsg);

    /**
     * @brief Register Recive Message Callback Function
     *
     * @param callback The Point of Callback Function
     */
    virtual void Server_Reg_recvMsgCallbackFunc(recvMsg_CallbackFunc _callback) = 0;
};

#endif // __SERVER_INTERFACE_H__
