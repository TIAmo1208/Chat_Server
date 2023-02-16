#include "../include/threadPool.hpp"
#include "../include/threadPool_impl.h"

ThreadPool_impl *g_threadPool = nullptr;

ThreadPool::ThreadPool(int _threadNum)
{
    g_threadPool = new ThreadPool_impl(_threadNum);
}

ThreadPool::~ThreadPool()
{
    g_threadPool->join();
    delete g_threadPool;
}

void ThreadPool::ThreadPool_addTask(std::function<void()> &&_func)
{
    g_threadPool->add_Task(_func);
}

void ThreadPool::ThreadPool_join()
{
    g_threadPool->join();
}
