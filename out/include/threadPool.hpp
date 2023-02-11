/**
 * @file threadPool.hpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
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

/*______ F U N C T I O N _____________________________________________________*/

class Mutex_Queue
{
public:
    Mutex_Queue() {}
    void Mutex_Queue_pop();
    bool Mutex_Queue_empty();
    void Mutex_Queue_emplace(std::function<void()> &&_func);
    std::function<void()> Mutex_Queue_front();
};

class ThreadPool
{
public:
    /// @brief Construct a new Thread Pool object
    /// @param threads the thread`s number
    ThreadPool(int _num = 5);

    /// @brief Destroy the Thread Pool object
    ///         -the destructor joins all threads
    ~ThreadPool();

    /// @brief add new task into task list
    /// @tparam Func
    /// @tparam ...Args
    /// @param _func
    /// @param ...args
    /// @return
    template <typename Func, typename... Args>
    std::future<typename std::result_of<Func(Args...)>::type> add_Task(Func &&_func, Args &&...args)
    {
        using return_type = typename std::result_of<Func(Args...)>::type;

        // bind the task function into a shared_ptr
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(_func), std::forward<Args>(args)...));

        // get the return value of the task function
        std::future<return_type> res = task->get_future();
        {
            // don't allow enqueueing after stopping the pool
            if (m_stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            // add the task
            m_tasks.Mutex_Queue_emplace([task]()
                                        { (*task)(); });
        }

        // Notice one thread of thread pool
        m_condition.notify_one();

        // return the future, get the value when you need
        return res;
    }

    /// @brief wait for all the thread done
    void join();

    /// @brief get task queue isnot empty
    /// @return
    bool is_tasks_empty();

private:
    /*______ V A R I A B L E _____________________________________________________*/

    // task queue
    Mutex_Queue m_tasks;

    // condition variale
    std::condition_variable m_condition;

    // the stop flag
    bool m_stop;
};

#endif // __THREADPOOL_H__