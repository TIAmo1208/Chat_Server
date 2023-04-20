#include "Log.h"

using namespace Log;

int main()
{
    LogSystem::instance()->Log_init();

    Log_info("Test");

    LogSystem::instance()->del_object();

    return 0;
}