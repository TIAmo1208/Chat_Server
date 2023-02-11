/**
 * @file threadPool.cpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "threadPool.hpp"

/*______ V A R I A B L E _____________________________________________________*/

std::condition_variable m_condition_join;

// need to keep track of threads so we can join them
std::vector<std::thread> m_list_threads;

// the num of thread
int m_num_thread = 0;

// the mutex for work thread
std::mutex m_mutex_work;

/*______ F U N C T I O N _____________________________________________________*/

ThreadPool::ThreadPool(int _num)
{
    m_stop = false;
    m_num_thread = _num;

    for (size_t i = 0; i < _num; ++i)
    {
        m_list_threads.emplace_back(
            [this]()
            {
                while (!m_stop)
                {
                    // wait for task
                    {
                        std::unique_lock<std::mutex> lock_work(m_mutex_work);

                        // block when the thread pool not stop, or tasks is empty
                        // unlock when the thread pool stop, or has new task
                        m_condition.wait(lock_work);
                    }

                    // exit when the stop is true and no task
                    if (m_stop && m_tasks.Mutex_Queue_empty())
                        return;

                    // start task
                    std::function<void()> task = std::move(m_tasks.Mutex_Queue_front());
                    task();
                }
            });
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    this->join();

    while (!m_tasks.Mutex_Queue_empty())
    {
        m_tasks.Mutex_Queue_pop();
    }
}

void ThreadPool::join()
{
    if (m_list_threads.empty())
        return;

    // wait for all the task done
    if (!m_tasks.Mutex_Queue_empty())
    {
        std::mutex mutex_join;
        std::unique_lock<std::mutex> lock_join(mutex_join);
        m_condition_join.wait(lock_join);
    }

    m_stop = true;

    // notify all thread, and end it
    m_condition.notify_all();
    for (std::thread &worker : m_list_threads)
    {
        worker.join();
    }
    m_list_threads.clear();
}

bool ThreadPool::is_tasks_empty()
{
    return m_tasks.Mutex_Queue_empty();
}

/*______ V A R I A B L E _____________________________________________________*/

std::mutex m_mutex_tasks;                        // the mutex for task queue
std::queue<std::function<void()>> m_queue_tasks; // the task queue
int m_queue_length = 0;                          // the task num

/*______ F U N C T I O N _____________________________________________________*/

void Mutex_Queue::Mutex_Queue_pop()
{
    std::unique_lock<std::mutex> lock(m_mutex_tasks);
    m_queue_tasks.pop();
    m_queue_length--;
    if (m_queue_length <= 0)
    {
        m_condition_join.notify_all();
    }
}
bool Mutex_Queue::Mutex_Queue_empty()
{
    std::unique_lock<std::mutex> lock(m_mutex_tasks);
    return m_queue_tasks.empty();
}
void Mutex_Queue::Mutex_Queue_emplace(std::function<void()> &&_func)
{
    std::unique_lock<std::mutex> lock(m_mutex_tasks);
    m_queue_tasks.emplace(_func);
    m_queue_length++;
}
std::function<void()> Mutex_Queue::Mutex_Queue_front()
{
    std::unique_lock<std::mutex> lock(m_mutex_tasks);
    std::function<void()> func = m_queue_tasks.front();
    m_queue_tasks.pop();
    m_queue_length--;
    if (m_queue_length <= 0)
    {
        m_condition_join.notify_all();
    }
    return func;
}
