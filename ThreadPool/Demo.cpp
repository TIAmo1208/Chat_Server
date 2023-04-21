#include <iostream>

#include "../Log/include/Log.h"
using namespace Log;

#include "threadPool.hpp"

class Test
{
    int test2 = 0;

public:
    int testfunc(int num)
    {
        int test = 0;
        Log_info("worker thread ID: %d, num: %d, test:%d, test2:%d", std::this_thread::get_id(), num, test, test2);
        return num;
    }
};

int main()
{
    LogSystem::instance()->Log_init();

    ThreadPool threadPool(5);

    int num = 0;

    using namespace std ::chrono;
    auto start = system_clock::now();

    Test *test = new Test;
    Log_info("start add task");
    while (num < 100)
    {
        std::future<int> ret = threadPool.ThreadPool_add_Task(&Test::testfunc, test, num++);
        std::this_thread::sleep_for(std::chrono::microseconds(3));
    }

    threadPool.ThreadPool_join();

    duration<double> dur = system_clock::now() - start;
    Log_info("thoolpool 耗时: %f", dur.count());

    return 0;
}
