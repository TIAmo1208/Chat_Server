#include "threadPool.hpp"
#include "threadPool_impl.h"
#include <memory>

std::shared_ptr<ThreadPool_impl> g_threadPool = nullptr;

ThreadPool::ThreadPool(int _threadNum) { g_threadPool = std::make_shared<ThreadPool_impl>(_threadNum); }

ThreadPool::~ThreadPool() { g_threadPool->join(); }

void ThreadPool::ThreadPool_addTask(std::function<void()> &&_func) { g_threadPool->add_Task(_func); }

void ThreadPool::ThreadPool_join() { g_threadPool->join(); }
