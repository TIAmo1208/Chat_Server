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

#include "config.h"
#include "Log.h"
#include "threadPool.hpp"
#include "Mysql.h"
#include <thread>

#include "Server_Interface.hpp"
#include "Server_impl.hpp"

int main(const int _argc, char *const _argv[])
{
    // config
    CONFIG::Config cfg(_argc, _argv);

    int log_level = 3;
    bool log_file_enable = true;
    std::string log_file_path = "./log_server";
    int server_port = 8000;
    int threads = 5;

    cfg.Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_LogLevel, log_level);
    cfg.Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_LogFileEnable, log_file_enable);
    cfg.Config_GetValue(CONFIG_MODULE_LOG, CONFIG_LOG_FilePath, log_file_path);
    cfg.Config_GetValue(CONFIG_MODULE_SERVER, CONFIG_SERVER_Server_Port, server_port);
    cfg.Config_GetValue(CONFIG_MODULE_THREAD_POOL, CONFIG_THREAD_POOL_THREAD_NUM, threads);

    // log
    LogSystem::instance()->Log_init(log_level, log_file_enable, log_file_path);

    // thread pool
    // ThreadPool *threadpool = new ThreadPool(threads);

    // Mysql
    std::string host = "localhost";
    std::string name = "TIAmo";
    std::string password = "sqm19991208";
    std::string database = "Chat_server";
    Mysql::instance()->Mysql_init(host, name, password, database, 3306);

    // socket
    std::shared_ptr<Server_Interface> server = std::make_shared<Server_impl>();

    server->Server_Init(8888);

    while (1)
    {
        server->Server_Update();
    }

    //
    LogSystem::instance()->del_object();
    Mysql::instance()->del_object();

    return 0;
}

#endif // __DEMO_H__
