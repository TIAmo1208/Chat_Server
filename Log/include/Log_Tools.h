/**
 * @file Log_Tools.h
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-04-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __LOG_TOOLS_H__
#define __LOG_TOOLS_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <fstream>
#include <mutex>
#include <string>
#include <thread>

/*______ D E F I N E _________________________________________________________*/

const constexpr int LOG_BUFF_MAX_SIZE        = (512 * 1024);
const constexpr int LOG_TXT_OUTPUT_THRESHOLD = LOG_BUFF_MAX_SIZE * 0.9;
#define LOG_OUTPUT_TIME 10

const constexpr int LOG_TXT_MAX_SIZE = 2048;
const constexpr int TIME_DATA_SIZE   = 10;

/*______ C L A S S ___________________________________________________________*/
namespace Log
{
class Log_Tools
{
public:
    /*______ F U N C T I O N _________________________________________________*/

    /// @brief write the log into the file
    /// @param _output
    /// @param _filePath
    /// @return 0:success; -1:file open fail; -2:is already init
    int Log_Tools_Init(bool _output = true, std::string _filePath = "Log");

    /// @brief write the log into the file
    /// @param _LogFormat
    /// @param _len
    void Log_Tools_Write_File(const char *_LogFormat, int _len);

    /// @brief write the log into the file
    /// @param _logLevel
    /// @param _LogFormat
    /// @return
    char *Log_Tools_Construct_LogTxt(const char *_logLevel, const char *_LogFormat);

    /// @brief set the log output time
    /// @param _seconds The default time is 10 seconds
    void Log_Tools_set_LogOutput_Time(int _seconds = 10);

    /// @brief set file path
    /// @brief If you want to set the file path, you have to set this path before use log function.
    /// @param filePath
    /// @return
    int Log_Tools_set_FilePath(std::string &_filePath);

private:
    /// @brief Using Time to Update Log File Name
    /// @return
    int Log_Tools_UpdateLogFile(std::string &_filePath);

    /// @brief Thread task: Save Log into buff
    void Log_Tools_Save_Log();

public:
    Log_Tools();
    ~Log_Tools();

private:
    /*______ V A R I A B L E _________________________________________________*/

    std::string m_logFilePath = "./Log";
    std::string m_logFileName = "";

    std::thread m_thread_SaveLog; // 使用线程清空缓冲区，写入文件
    std::mutex m_mutex_LogBuff;
    std::condition_variable m_condition_wait_time;

    char *m_logBuff = nullptr;
    int m_logLength = 0;

    bool m_initState = false; // true when init is done
    bool m_output    = true;
    bool m_stop      = false;
    bool m_buff_fill = false;

    char *m_logTime = new char[TIME_DATA_SIZE];

    int m_output_time = 10;

    std::ofstream m_WriteStream;
};
} // namespace Log

#endif // __LOG_TOOLS_H__