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

/*______ I N C L U D E - F I L E S ___________________________________________*/

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
