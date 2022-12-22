/**
 * @file threadPool.cpp
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-11-03
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "threadPool.h"

ThreadPool::ThreadPool(int threads)
{
	init(threads);
}

// template <typename F, typename... Args>
// std::future<typename std::result_of<F(Args...)>::type> ThreadPool::add_Task(F &&f, Args &&...args)
// {
// 	using return_type = typename std::result_of<F(Args...)>::type;
//
// 	// bind the task function into a shared_ptr
// 	auto task = std::make_shared<std::packaged_task<return_type()>>(
// 		std::bind(std::forward<F>(f), std::forward<Args>(args)...));
//
// 	// get the return value of the task function
// 	std::future<return_type> res = task->get_future();
// 	{
// 		std::unique_lock<std::mutex> lock(m_queue_mutex);
//
// 		// don't allow enqueueing after stopping the pool
// 		if (m_stop)
// 		{
// 			throw std::runtime_error("enqueue on stopped ThreadPool");
// 		}
//
// 		// add the task
// 		m_tasks.emplace([task]()
// 						{ (*task)(); });
// 	}
//
// 	// Notice one thread of thread pool
// 	m_condition.notify_one();
//
// 	// return the future, get the value when you need
// 	return res;
// }

ThreadPool::~ThreadPool()
{
	// get the lock and set the stop flag true
	std::unique_lock<std::mutex> lock(m_queue_mutex);
	m_stop = true;

	// notify all thread, and end it
	m_condition.notify_all();
	for (std::thread &worker : m_workers)
		worker.join();
}

void ThreadPool::init(int threads)
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
