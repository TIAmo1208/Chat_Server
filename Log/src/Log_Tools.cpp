
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Log_Tools.h"
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

using namespace Log;

/*______ F U N C T I O N _____________________________________________________*/

Log_Tools::Log_Tools() {}

Log_Tools::~Log_Tools()
{
    m_stop = true;
    m_condition_wait_time.notify_all();
    m_thread_SaveLog.join();

    if (m_WriteStream.is_open())
    {
        m_WriteStream.close();
    }

    delete[] m_logTime;
    delete[] m_logBuff;
}

int Log_Tools::Log_Tools_Init(bool _output, std::string _filePath)
{
    if (m_initState)
    {
        return -1;
    }

    m_output = _output;
    if (!m_output)
    {
        return 0;
    }

    m_logFilePath = _filePath;
    Log_Tools_UpdateLogFile(m_logFilePath);

    m_logBuff = new char[LOG_BUFF_MAX_SIZE];
    memset(m_logBuff, 0, LOG_BUFF_MAX_SIZE);
    m_thread_SaveLog = std::thread(&Log_Tools::Log_Tools_Save_Log, this);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    m_output_time = LOG_OUTPUT_TIME;

    m_initState = true;
    return 0;
}

void Log_Tools::Log_Tools_Write_File(const char *_LogFormat, int _len)
{
    if (!m_output)
    {
        return;
    }

    std::unique_lock<std::mutex> lock(m_mutex_LogBuff);
    memcpy(&m_logBuff[m_logLength], _LogFormat, _len);
    memcpy(&m_logBuff[m_logLength + _len], "\n", 1);
    m_logLength += (_len + 1);

    // 超出阈值，输出log
    if (m_logLength > LOG_TXT_OUTPUT_THRESHOLD)
    {
        m_buff_fill = true;
        m_condition_wait_time.notify_one();
    }
}

char *Log_Tools::Log_Tools_Construct_LogTxt(const char *_logLevel, const char *_LogFormat)
{
    // time for log
    time_t t = time(0);
    strftime(m_logTime, TIME_DATA_SIZE, "%H:%M:%H", localtime(&t));

    // log txt
    char *logTxt = new char[LOG_TXT_MAX_SIZE];
    memset(logTxt, 0, LOG_TXT_MAX_SIZE);
    sprintf(logTxt, "[%s]:[%s]\t%s", m_logTime, _logLevel, _LogFormat);

    return logTxt;
}

void Log_Tools::Log_Tools_set_LogOutput_Time(int _seconds) { m_output_time = _seconds; }

int Log_Tools::Log_Tools_set_FilePath(std::string &_filePath)
{
    if (Log_Tools_UpdateLogFile(_filePath) >= 0)
    {
        m_logFilePath = _filePath;
        return 0;
    }
    return -1;
}

/*______ L O C A L - F U N C T I O N _________________________________________*/

int Log_Tools::Log_Tools_UpdateLogFile(std::string &_filePath)
{
    //
    std::string tempPath = m_logFilePath;
    if (_filePath == m_logFilePath)
    {
        tempPath = _filePath;
    }

    // Created when no folder exists
    if (F_OK != access(tempPath.c_str(), 0))
    {
        std::string temp = "mkdir " + tempPath;
        int ret          = system(temp.c_str());
    }
    else
    {
        return -1;
    }

    // get log file's name
    time_t t = time(0);
    char fileName[32];
    strftime(fileName, sizeof(fileName), "%Y-%m-%d", localtime(&t));
    strcat(fileName, ".log");

    if (m_logFileName != fileName)
    {
        m_logFileName = fileName;

        // if not exist the log file, create and write new log
        if (0 != access((tempPath + "/" + m_logFileName).c_str(), 0))
        {
            // open file
            {
                std::unique_lock<std::mutex> lock_buff(m_mutex_LogBuff);
                if (m_WriteStream.is_open())
                {
                    m_WriteStream.close();
                }
                // open file
                m_WriteStream = std::ofstream((tempPath + "/" + m_logFileName), std::ios::app);
            }

            //
            char log[1024];
            memset(log, 0, 1024);
            sprintf(log, "Log: Opean a New file:%s", (m_logFilePath + "/" + m_logFileName).c_str());
            char *temp = Log_Tools_Construct_LogTxt("LOG_INFO", log);
            //
            printf("%s\n", temp);
            delete[] temp;
        }
        else
        {
            std::unique_lock<std::mutex> lock_buff(m_mutex_LogBuff);
            if (m_WriteStream.is_open())
            {
                m_WriteStream.close();
            }
            // open file
            m_WriteStream = std::ofstream((tempPath + "/" + m_logFileName), std::ios::app);
        }
    }

    m_logFilePath = tempPath;

    return 0;
}

void Log_Tools::Log_Tools_Save_Log()
{
    std::mutex mutex;
    while (!m_stop)
    {
        std::unique_lock<std::mutex> lock_wait(mutex);
        m_condition_wait_time.wait_until(lock_wait, std::chrono::system_clock::now() + std::chrono::seconds(m_output_time),
                                         [this] { return this->m_buff_fill || this->m_stop; });

        // Update file path
        Log_Tools_UpdateLogFile(m_logFilePath);

        if (m_WriteStream.is_open())
        {
            // write into file
            std::unique_lock<std::mutex> lock_buff(m_mutex_LogBuff);
            if (m_WriteStream.is_open())
            {
                m_WriteStream << std::move(m_logBuff) << std::endl;

                memset(m_logBuff, 0, m_logLength);
                m_logLength = 0;
                m_buff_fill = false;
            }
        }
    }
}
