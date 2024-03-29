#ifndef __THREADPOOL_IMPL_H__
#define __THREADPOOL_IMPL_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <memory>
#include <queue>
#include <vector>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <functional>
#include <future>

//
#include "Log.h"
using namespace Log;

/*______ F U N C T I O N _____________________________________________________*/

class ThreadPool_impl
{
public:
    /// @brief add new task into task list
    /// @param _task
    void add_Task(std::function<void()> &_task);

    /// @brief wait for all the thread done
    void join();

    /// @brief Construct a new Thread Pool object
    /// @param threads the thread`s number
    ThreadPool_impl(int threads = 5);

    /// @brief Destroy the Thread Pool object
    ///         -the destructor joins all threads
    ~ThreadPool_impl();
};

#endif // __THREADPOOL_IMPL_H__
