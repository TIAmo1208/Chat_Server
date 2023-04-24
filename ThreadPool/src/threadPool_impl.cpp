/**
 * @file threadPool.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2022-11-03
 *
 * @copyright Copyright (c) 2022
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "threadPool_impl.h"
#include "Log.h"
#include <thread>

/*______ V A R I A B L E _____________________________________________________*/

class Mutex_Queue
{
public:
    Mutex_Queue() {}
    void Mutex_Queue_pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex_queue);
        m_queue_tasks.pop();
    }
    bool Mutex_Queue_empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex_queue);
        return m_queue_tasks.empty();
    }
    void Mutex_Queue_emplace(std::function<void()> &&_func)
    {
        std::unique_lock<std::mutex> lock(m_mutex_queue);
        m_queue_tasks.emplace(_func);
    }
    std::function<void()> Mutex_Queue_front()
    {
        std::unique_lock<std::mutex> lock(m_mutex_queue);
        std::function<void()> func = m_queue_tasks.front();
        m_queue_tasks.pop();
        return func;
    }

private:
    std::mutex m_mutex_queue;                        // the mutex for task queue
    std::queue<std::function<void()>> m_queue_tasks; // the task queue
};
Mutex_Queue m_queue;

// need to keep track of threads so we can join them
std::vector<std::thread> m_list_threads;

// the num of thread
int m_num_thread = 0;

// the mutex for work thread
std::mutex m_mutex_work;

// condition variale
std::condition_variable m_condition;

// the stop flag
bool m_stop = false;

/*______ L O C A L - F U N C T I O N _________________________________________*/

void workthread()
{
    while (!m_stop)
    {
        // wait for task
        {
            std::unique_lock<std::mutex> lock_work(m_mutex_work);

            // block when the thread pool not stop, or tasks is empty
            // unlock when the thread pool stop, or has new task
            // Log_debug("ThreadPool_impl: threadID:%d, wait for task...", std::this_thread::get_id());
            m_condition.wait(lock_work);
        }

        // exit when the stop is true and no task
        if (m_stop)
            return;

        Log_debug("ThreadPool_impl: do work");

        // start task
        std::function<void()> task = std::move(m_queue.Mutex_Queue_front());
        task();
    }
}

/*______ F U N C T I O N _____________________________________________________*/

ThreadPool_impl::ThreadPool_impl(int threads)
{
    m_stop       = false;
    m_num_thread = threads;

    for (size_t i = 0; i < m_num_thread; ++i)
    {
        // Instance thread
        m_list_threads.emplace_back(workthread);
    }
    Log_info("ThreadPool_impl: Init done");
}

ThreadPool_impl::~ThreadPool_impl()
{
    while (!m_queue.Mutex_Queue_empty())
    {
        m_queue.Mutex_Queue_pop();
    }

    this->join();
}

void ThreadPool_impl::add_Task(std::function<void()> &_task)
{
    // save task function ptr
    std::shared_ptr<std::packaged_task<void()>> task = std::make_shared<std::packaged_task<void()>>(_task);

    if (m_stop)
    {
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }

    // add task to task list
    m_queue.Mutex_Queue_emplace([task]() { (*task)(); });
    Log_debug("ThreadPool_impl: add_Task: add new task into task list");

    // Notice one thread of thread pool
    m_condition.notify_one();
}

void ThreadPool_impl::join()
{
    if (m_list_threads.empty())
        return;

    m_stop = true;

    // wait for all the task done
    while (!m_queue.Mutex_Queue_empty())
    {
        m_queue.Mutex_Queue_pop();
    }

    // notify all thread, and end it
    m_condition.notify_all();
    for (std::thread &worker : m_list_threads)
    {
        worker.join();
    }
    m_list_threads.clear();
}
