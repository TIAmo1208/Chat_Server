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
#include <cstdio>
#include <cstring>
#include <memory>
#include <thread>

using namespace Log;

/*______ D E F I N E _________________________________________________________*/

#define COMMOND_TEST "test"
#define COMMOND_LOG_LEVEL_NONE "log_none"
#define COMMOND_LOG_LEVEL_ERROR "log_error"
#define COMMOND_LOG_LEVEL_WARN "log_warn"
#define COMMOND_LOG_LEVEL_INFO "log_info"
#define COMMOND_LOG_LEVEL_DEBUG "log_debug"

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
    Mysql::instance()->del_object();
    LogSystem::instance()->del_object();

    exit(signum);
}

void Commond_handle(char *_commond)
{
    if (memcmp(_commond, COMMOND_TEST, strlen(COMMOND_TEST)) == 0)
    {
        Log_debug("COMMOND: test");
    }
    else if (memcmp(_commond, COMMOND_LOG_LEVEL_NONE, strlen(COMMOND_LOG_LEVEL_NONE)) == 0)
    {
        Log_debug("COMMOND: log_none");
        LogSystem::instance()->Log_set_LogLevel(0);
    }
    else if (memcmp(_commond, COMMOND_LOG_LEVEL_ERROR, strlen(COMMOND_LOG_LEVEL_ERROR)) == 0)
    {
        Log_debug("COMMOND: log_error");
        LogSystem::instance()->Log_set_LogLevel(1);
    }
    else if (memcmp(_commond, COMMOND_LOG_LEVEL_WARN, strlen(COMMOND_LOG_LEVEL_WARN)) == 0)
    {
        Log_debug("COMMOND: log_warn");
        LogSystem::instance()->Log_set_LogLevel(2);
    }
    else if (memcmp(_commond, COMMOND_LOG_LEVEL_INFO, strlen(COMMOND_LOG_LEVEL_INFO)) == 0)
    {
        Log_debug("COMMOND: log_info");
        LogSystem::instance()->Log_set_LogLevel(3);
    }
    else if (memcmp(_commond, COMMOND_LOG_LEVEL_DEBUG, strlen(COMMOND_LOG_LEVEL_DEBUG)) == 0)
    {
        Log_debug("COMMOND: log_debug");
        LogSystem::instance()->Log_set_LogLevel(4);
    }
    else
    {
        Log_debug("NO COMMOND");
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
