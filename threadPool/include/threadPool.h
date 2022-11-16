/**
 * @file threadPool.h
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-11-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <vector>
#include <queue>
#include <memory>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>

#include <functional>
// #include <stdexcept>

#define THREAD_NUM 5

class ThreadPool
{
public:
    /**
     * @brief Construct a new Thread Pool object
     *
     * @param threads the thread`s number
     */
    ThreadPool(int threads = THREAD_NUM);

    /**
     * @brief add new work item to the pool
     *
     * @param f add the function as task
     * @param args  the function's args
     * @return use future save the function's return valuer, get the value when you need
     */
    template <typename F, typename... Args>
    std::future<typename std::result_of<F(Args...)>::type> add_Task(F &&f, Args &&...args);

    /**
     * @brief Destroy the Thread Pool object
     *      - the destructor joins all threads
     *
     */
    ~ThreadPool();

private:
    void init(int threads);

private:
    // need to keep track of threads so we can join them
    std::vector<std::thread> m_workers;

    // the task queue
    std::queue<std::function<void()>> m_tasks;

    // the mutex of synchronization
    std::mutex m_queue_mutex;

    // condition variale
    std::condition_variable m_condition;

    bool m_stop;
};

#endif // __THREADPOOL_H__
