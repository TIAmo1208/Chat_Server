/**
 * @file Log.cpp
 * @author
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <cstring>
#include <mutex>
#include <stdarg.h> // parameter list
#include <unistd.h>

#include <csignal> // 信号

#include "Log.h"
#include "Log_Tools.h"

using namespace Log;

/*______ D E F I N E _________________________________________________________*/

#define LOG_LEVEL_NONE (0)
#define LOG_LEVEL_ERROR (1)
#define LOG_LEVEL_WARN (2)
#define LOG_LEVEL_INFO (3)
#define LOG_LEVEL_DEBUG (4)

#define TYPE_LOG_DEBUG "DEBUG"
#define TYPE_LOG_INFO "INFO"
#define TYPE_LOG_ERROR "ERROR"
#define TYPE_LOG_WARN "WARN"
#define TYPE_LOG_FATAL "FATAL"

/*______ V A R I A B L E _____________________________________________________*/

static LogSystem *m_logSystem = nullptr; // log system ptr

Log_Tools *m_logTools = nullptr;

bool m_initState  = false; // true when init is done
bool m_outputFile = true;  // write into file

std::string m_filePath    = "./Log";
std::string m_logFileName = "";

std::mutex m_mutex;

int m_log_level = LOG_LEVEL_INFO;

int m_outputTime = 10;

/*______ L O C A L - F U N C T I O N _________________________________________*/

void signalHandler(int signum)
{
    Log_info("LogSystem: Get signal: SIGINT, signum:%d\n", signum);

    if (m_logSystem != nullptr)
    {
        delete m_logSystem;
    }

    exit(signum);
}

/*______ F U N C T I O N _____________________________________________________*/

/// @brief default constructor
LogSystem::LogSystem() { signal(SIGINT, signalHandler); }

/// @brief destructor
LogSystem::~LogSystem()
{
    delete m_logTools;
    printf("\nyou can see the Log file in the %s\n", m_filePath.c_str());
}

/// @brief init and return ptr of the log system
/// @return
LogSystem *LogSystem::instance()
{
    if (m_logSystem == nullptr)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_logSystem == nullptr)
        {
            m_logSystem = new LogSystem();
        }
    }
    return m_logSystem;
}

/// @brief release instance
void LogSystem::del_object()
{
    if (m_logSystem != nullptr)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_logSystem != nullptr)
        {
            delete m_logSystem;
            m_logSystem = nullptr;
        }
    }
}

/// @brief initialization
/// @param _log_level
/// @param _log_file_enable
/// @param _filePath
void LogSystem::Log_init(int _log_level, bool _log_file_enable, std::string _filePath)
{
    if (m_initState)
    {
        Log_debug("Log: Log has completed initialization, the log path:%s", m_filePath.c_str());
        return;
    }

    m_log_level  = _log_level;
    m_outputFile = _log_file_enable;
    m_filePath   = _filePath;

    m_logTools = new Log_Tools();
    m_logTools->Log_Tools_Init(m_outputFile, _filePath);
    log_info("Log: Init done, log level:%d, file path:%s, output:%s, ", m_log_level, m_filePath.c_str(), m_outputFile ? "true" : "false");

    m_initState = true;
}

/// @brief print debug into the screan
/// @param file
/// @param line
/// @param pLogFormat
/// @param
/// @return
bool LogSystem::log_debug(const char *file, const int line, const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log
    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(TYPE_LOG_DEBUG, temp);
    if (m_log_level >= LOG_LEVEL_DEBUG)
    {
        printf("%s\t %s:%d\n", logTxt, file, line);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
    return true;
}

/// @brief print info into the screan
/// @param pLogFormat
/// @param
/// @return
bool LogSystem::log_info(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log
    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(TYPE_LOG_INFO, temp);
    if (m_log_level >= LOG_LEVEL_INFO)
    {
        printf("%s\n", logTxt);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
    return true;
}

/// @brief print error into the screan
/// @param pLogFormat
/// @param
/// @return
bool LogSystem::log_error(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log
    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(TYPE_LOG_ERROR, temp);
    if (m_log_level >= LOG_LEVEL_ERROR)
    {
        printf("%s\n", logTxt);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
    return true;
}

/// @brief print warn into the screan
/// @param pLogFormat
/// @param
/// @return
bool LogSystem::log_warn(const char *pLogFormat, ...)
{
    if (m_log_level < LOG_LEVEL_WARN)
        return false;

    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log
    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(TYPE_LOG_WARN, temp);
    if (m_log_level >= LOG_LEVEL_WARN)
    {
        printf("%s\n", logTxt);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
    return true;
}

/// @brief print fatal into the screan
/// @param pLogFormat
/// @param
/// @return
bool LogSystem::log_fatal(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log
    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(TYPE_LOG_INFO, temp);
    if (m_log_level != LOG_LEVEL_NONE)
    {
        printf("%s\n", logTxt);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
    return true;
}

/// @brief
/// @param file
/// @param line
/// @param logType
/// @param pLogFormat
/// @param
void LogSystem::operator()(const char *file, const int line, const char *logType, const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList); // need copy

    int size   = vsnprintf(0, 0, pLogFormat, paramList) + 1; // get size and clear paramList
    char *temp = new char[size];
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // 填充log

    char *logTxt = m_logTools->Log_Tools_Construct_LogTxt(logType, temp);
    if (m_log_level != LOG_LEVEL_NONE)
    {
        printf("%s\n", logTxt);
    }

    // 输出日志文件
    if (m_outputFile)
    {
        m_logTools->Log_Tools_Write_File(logTxt, strlen(logTxt));
    }

    delete[] temp;
    delete[] logTxt;
}

/// @brief Set the state of the output file
/// @param state true: ouput; false: No output
void LogSystem::Log_set_OutputFile(bool state) { m_outputFile = state; }

/// @brief set file path
/// @param filePath
void LogSystem::Log_set_FilePath(std::string &filePath)
{
    if (m_logTools->Log_Tools_set_FilePath(m_filePath) >= 0)
    {
        m_filePath = filePath;
    }
}

/// @brief set log level
/// @param _level
void LogSystem::Log_set_LogLevel(int _level) { m_log_level = _level; }

/// @brief set the log output time
/// @param _seconds The default time is 10 seconds
void Log_set_LogOutput_Time(int _seconds = 10)
{
    m_outputTime = _seconds;
    m_logTools->Log_Tools_set_LogOutput_Time(m_outputTime);
}
