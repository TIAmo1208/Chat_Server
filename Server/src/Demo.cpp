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

#include "../../out/include/config.h"
#include "../../out/include/Log.h"
#include "../../out/include/threadPool.hpp"
#include "../../out/include/Mysql.h"
#include <thread>

#include "../include/socket.h"
#include "../include/socket_config.h"

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
    ThreadPool *threadpool = new ThreadPool(threads);

    // Mysql
    std::string host = "localhost";
    std::string name = "TIAmo";
    std::string password = "sqm19991208";
    std::string database = "Chat_server";
    Mysql::instance()->Mysql_init(host, name, password, database, 3306);

    // socket
    Socket *socket = new Socket(server_port);
    std::thread thread_socket = std::thread(&Socket::socket_accept, socket, threadpool);
    Log_info("Thread : socket_accept id : %d", thread_socket.get_id());

    thread_socket.join();

    delete socket;
    delete threadpool;
    LogSystem::instance()->del_object();
    Mysql::instance()->del_object();

    return 0;
}

#endif // __DEMO_H__
