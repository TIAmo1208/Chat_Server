/**
 * @file threadPool.cpp
 * @author TIAmo (tiamo1208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-04-24
 *
 * @copyright Copyright (c) 2023
 *
 */
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "threadPool.hpp"
#include "threadPool_impl.h"
#include <memory>

#include <csignal> // 信号

/*______ V A R I A B L E _____________________________________________________*/

std::shared_ptr<ThreadPool_impl> g_threadPool = nullptr;

/*______ L O C A L - F U N C T I O N _________________________________________*/

void signalHandler(int signum)
{
    Log_info("ThreadPool: Get signal: SIGINT, signum:%d\n", signum);

    if (g_threadPool != nullptr)
    {
        g_threadPool.reset();
    }

    exit(signum);
}

/*______ F U N C T I O N _____________________________________________________*/

ThreadPool::ThreadPool(int _threadNum)
{
    signal(SIGINT, signalHandler);
    g_threadPool = std::make_shared<ThreadPool_impl>(_threadNum);
}

ThreadPool::~ThreadPool() { g_threadPool->join(); }

void ThreadPool::ThreadPool_addTask(std::function<void()> &&_func) { g_threadPool->add_Task(_func); }

void ThreadPool::ThreadPool_join() { g_threadPool->join(); }
