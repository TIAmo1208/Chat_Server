#ifndef __SERVER_INTERFACE_H__
#define __SERVER_INTERFACE_H__

#include <string>

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
    virtual int Server_Init(int port) = 0;

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
