#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

/**
 * @file threadPool.h
 * @author
 * @brief
 * @version 0.1
 * @date 2022-11-03
 *
 * @copyright Copyright (c) 2022
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <vector>
#include <queue>
#include <memory>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <future>
#include <functional>

/*______ F U N C T I O N _____________________________________________________*/

class ThreadPool
{
public:
    /// @brief add new task into task list
    /// @param _task
    void add_Task(std::function<void()> &_task);

    /// @brief wait for all the thread done
    void join();

    /// @brief Construct a new Thread Pool object
    /// @param threads the thread`s number
    ThreadPool(int threads);

    /// @brief Destroy the Thread Pool object
    ///         -the destructor joins all threads
    ~ThreadPool();
};

#endif // __THREADPOOL_H__