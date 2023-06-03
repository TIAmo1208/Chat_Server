#ifndef __DEMO_H__
#define __DEMO_H__

#include <fstream>
#include <string>

#ifdef __linux__
#include <sys/socket.h>
#include <sys/stat.h> // file state
#endif

#include <condition_variable>
#include <thread>
#include <vector>

#define Task_Num 5
#define SendFile_Size_Max 1024 * 1024 * 128
#define SendFile_Size_Min 1024 * 1024 * 50

//
struct s_Task_File
{
    std::string fileName = "";
    int start            = -1;
    int end              = -1;

    int socket = -1;
};

//
std::condition_variable m_condition_task;
std::vector<s_Task_File> m_TaskList;

void send_file(s_Task_File _task)
{
    // 打开本地文件
    std::ifstream ifs(_task.fileName, std::ios::binary);
    if (!ifs.is_open())
    {
        printf("Cannot open file %s", _task.fileName.c_str());
        return;
    }

    // 定位到指定位置
    ifs.seekg(_task.start);

    // 发送指定范围内的文件内容
    char buffer[1024];
    int nread;
    while (_task.start < _task.end && (nread = ifs.read(buffer, std::min((int) sizeof(buffer), _task.end - _task.start)).gcount()) > 0)
    {
        // TODO 发送数据
        send(_task.socket, buffer, nread, 0);

        _task.start += nread;
    }

    // 关闭文件
    ifs.close();
}

int main()
{
    std::string filename = "./test.txt";
    int index            = 0;
    int connectSocket;

    // 获取本地文件大小
    struct stat fileStat;
    int size = 0;
    if (stat(filename.c_str(), &fileStat) == 0)
    {
        size = fileStat.st_size;
        printf("File size: %d Mb \n", size/1024/1024);
    }
    else
    {
        printf("Error opening file. \n");
    }

    // 计算每个线程需要发送的文件范围
    int num_task   = Task_Num;
    int chunk_size = size / num_task;
    // 小于最小值不分割
    if (SendFile_Size_Min >= size)
    {
        num_task   = 1;
        chunk_size = size;
    }
    else
    {
        // 根据分割文件大小再次分割或减少分割次数
        if (SendFile_Size_Max < chunk_size)
        {
            int num = chunk_size / SendFile_Size_Max;
            num_task += num;

            chunk_size = size / num_task; // 重新计算文件块大小
        }
        else if (SendFile_Size_Min >= chunk_size)
        {
            num_task -= 3;
            chunk_size = size / num_task; // 重新计算文件块大小
        }
    }
    printf("task num: %d, chunk size: %d Mb\n", num_task, chunk_size / 1024 / 1024);

    for (int i = 0; i < num_task; i++)
    {
        int start = i * chunk_size;
        int end   = (i == num_task - 1) ? size : (i + 1) * chunk_size;
        m_TaskList.push_back({filename, start, end, connectSocket});

        // 唤醒线程处理
        m_condition_task.notify_one();
    }
}

#endif // __DEMO_H__