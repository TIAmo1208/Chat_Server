#ifndef __THREADPOOL_H__
#define __THREADPOOL_H__

#include <functional>
#include <future>

class ThreadPool
{
public:
    ThreadPool(int _threadNum = 5);
    ~ThreadPool();

    template <typename Func, typename... Args>
    std::future<typename std::result_of<Func(Args...)>::type> ThreadPool_add_Task(Func &&_func, Args &&... args)
    {
        using return_type = typename std::result_of<Func(Args...)>::type;

        // bind the task function into a shared_ptr
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(_func), std::forward<Args>(args)...));

        // get the return value of the task function
        std::future<return_type> res = task->get_future();
        ThreadPool_addTask([task]() { (*task)(); });

        // return the future, get the value when you need
        return res;
    }

    /// @brief wait for all the thread done
    void ThreadPool_join();

private:
    void ThreadPool_addTask(std::function<void()> &&_func);
};

#endif // __THREADPOOL_H__
