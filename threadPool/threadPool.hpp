/**
 * @file threadPoool.hpp
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-11-16
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
    ThreadPool(int threads = THREAD_NUM)
    {
        init(threads);
    }

    /**
     * @brief add new work item to the pool
     *
     * @param f add the function as task
     * @param args  the function's args
     * @return use future save the function's return valuer, get the value when you need
     */
    template <typename F, typename... Args>
    std::future<typename std::result_of<F(Args...)>::type> add_Task(F &&f, Args &&...args)
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        // bind the task function into a shared_ptr
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        // get the return value of the task function
        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(m_queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (m_stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            // add the task
            m_tasks.emplace([task]()
                            { (*task)(); });

            lock.unlock();
        }

        // Notice one thread of thread pool
        m_condition.notify_one();

        // return the future, get the value when you need
        return res;
    }

    /**
     * @brief Destroy the Thread Pool object
     *      - the destructor joins all threads
     *
     */
    ~ThreadPool()
    {
        // get the lock and set the stop flag true
        std::unique_lock<std::mutex> lock(m_queue_mutex);
        m_stop = true;

        // notify all thread, and end it
        m_condition.notify_all();
        for (std::thread &worker : m_workers)
            worker.join();
    }

private:
    void init(int threads)
    {
        m_stop = false;

        for (size_t i = 0; i < threads; ++i)
        {
            m_workers.emplace_back(
                [this]
                {
                    while (true)
                    {
                        std::function<void()> task;

                        // wait for task
                        std::unique_lock<std::mutex> lock(this->m_queue_mutex);

                        // block when the thread pool not stop, or tasks is empty
                        // unlock when the thread pool stop, or has new task
                        this->m_condition.wait(lock, [this]
                                               { return this->m_stop || !this->m_tasks.empty(); });

                        lock.unlock();

                        // exit when the stop is true and no task
                        if (this->m_stop && this->m_tasks.empty())
                            return;

                        // start task
                        task = std::move(this->m_tasks.front());
                        this->m_tasks.pop();
                        task();
                    }
                });
        }
    }

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
