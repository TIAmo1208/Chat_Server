#include "./include/threadPool.hpp"
#include <thread>

#include <iostream>

#include "../Log/include/Log.h"
using namespace Log;

int testfunc(int num)
{
    Log_info("worker thread ID: %d, num: %d", std::this_thread::get_id(), num);
    return num;
}

int main()
{
    LogSystem::instance()->Log_init();

    ThreadPool threadPool(5);

    int num = 0;

    using namespace std ::chrono;
    auto start = system_clock::now();

    Log_info("start add task");
    while (num < 100)
    {
        std::future<int> ret = threadPool.add_Task(testfunc, num++);
        std::this_thread::sleep_for(std::chrono::microseconds(3));
    }

    threadPool.join();

    duration<double> dur = system_clock::now() - start;
    Log_info("thoolpool 耗时: %f", dur.count());

    return 0;
}
