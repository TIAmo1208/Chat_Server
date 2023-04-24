/**
 * @file Demo.cpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __DEMO_H__
#define __DEMO_H__

#include "Log.h"
#include "Mysql.h"
#include "Server_config.h"
#include "config.h"

#include <csignal> // 信号

#include "Server_Interface.hpp"
#include "Server_impl.hpp"
#include <chrono>
#include <memory>
#include <thread>

using namespace Log;

/*______ D E F I N E _________________________________________________________*/

#define COMMOND_1 "1"

/*______ V A R I A B L E _____________________________________________________*/

// socket
std::shared_ptr<Server_Interface> m_server;

// config
std::shared_ptr<CONFIG::Config> m_config;

/*______ L O C A L - F U N C T I O N _________________________________________*/

void signalHandler(int signum)
{
    Log_info("Server: Get signal: SIGINT, signum:%d\n", signum);

    // server
    if (m_server != nullptr)
    {
        m_server.reset();
    }
    // config
    if (m_config != nullptr)
    {
        m_config.reset();
    }

    //
    LogSystem::instance()->del_object();
    Mysql::instance()->del_object();

    exit(signum);
}

void Commond_handle(char *_commond)
{
    if (memcmp(_commond, COMMOND_1, 1) == 0)
    {
        printf("COMMOND_TEST : 1\n");
    }
    else
    {
        printf("NO COMMOND\n");
    }
}

/*______ F U N C T I O N _____________________________________________________*/

int main(const int _argc, char *const _argv[])
{
    signal(SIGINT, signalHandler);

    int log_level             = 3;
    bool log_file_enable      = true;
    std::string log_file_path = "./log_server";
    int server_port           = 8000;
    int threads               = 5;

    m_config = std::make_shared<CONFIG::Config>(_argc, _argv);

    m_config->Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_LogLevel, log_level);
    m_config->Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_LogFileEnable, log_file_enable);
    m_config->Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_FilePath, log_file_path);
    m_config->Config_GetValue(CONFIG_MODULE_SERVER, CONFIG_SERVER_Server_Port, server_port);
    m_config->Config_GetValue(CONFIG_MODULE_THREAD_POOL, CONFIG_THREAD_POOL_THREAD_NUM, threads);

    m_config->Config_Regist_Commond_handle(Commond_handle);

    // log
    LogSystem::instance()->Log_init(log_level, log_file_enable, log_file_path);

    // Mysql
    std::string host     = "localhost";
    std::string name     = "TIAmo";
    std::string password = "sqm19991208";
    std::string database = "Chat_server";
    Mysql::instance()->Mysql_init(host, name, password, database, 3306);

    // socket
    m_server = std::make_shared<Server_impl>();

    int ret = m_server->Server_Init(8888, threads);
    if (SOCKET_ERROR == ret)
    {
        do
        {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            ret = m_server->Server_Init(8888, threads);
        } while (SOCKET_SUCCESS == ret);
    }

    while (1)
    {
        m_server->Server_Update();
    }

    //
    LogSystem::instance()->del_object();
    Mysql::instance()->del_object();

    return 0;
}

#endif // __DEMO_H__
