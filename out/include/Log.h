/**
 * @file Log.h
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __LOG_H__
#define __LOG_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <iostream>
#include <fstream>
#include <time.h> // time txt
#include <cstring>
#include <stdarg.h> // parameter list
#include <unistd.h> // check path 、 pwd

/*______ D E F I N E _________________________________________________________*/

#define LOG_LEVEL (LOG_LEVEL_INFO)

#define LOG_LEVEL_NONE (0)
#define LOG_LEVEL_ERROR (1)
#define LOG_LEVEL_WARN (2)
#define LOG_LEVEL_INFO (3)
#define LOG_LEVEL_DEBUG (4)

namespace Log
{
/*______ D E F I N E _________________________________________________________*/

#if (LOG_LEVEL == LOG_LEVEL_DEBUG)
#define Log_debug(format, ...) LogSystem::instance()->log_debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define Log_info(format, ...) LogSystem::instance()->log_info(format, ##__VA_ARGS__)
#define Log_error(format, ...) LogSystem::instance()->log_error(format, ##__VA_ARGS__)
#define Log_warn(format, ...) LogSystem::instance()->log_warn(format, ##__VA_ARGS__)
#define Log_fatal(format, ...) LogSystem::instance()->log_fatal(format, ##__VA_ARGS__)
#elif (LOG_LEVEL == LOG_LEVEL_INFO)
#define Log_debug(format, ...)
#define Log_info(format, ...) LogSystem::instance()->log_info(format, ##__VA_ARGS__)
#define Log_error(format, ...) LogSystem::instance()->log_error(format, ##__VA_ARGS__)
#define Log_warn(format, ...) LogSystem::instance()->log_warn(format, ##__VA_ARGS__)
#define Log_fatal(format, ...) LogSystem::instance()->log_fatal(format, ##__VA_ARGS__)
#elif (LOG_LEVEL == LOG_LEVEL_NONE)
#define Log_debug(format, ...)
#define Log_info(format, ...)
#define Log_error(format, ...)
#define Log_warn(format, ...)
#define Log_fatal(format, ...)
#endif

#define LOG_FILE_PATH "Log"
#define TYPE_LOG_DEBUG "LOG_DEBUG"
#define TYPE_LOG_INFO "LOG_INFO"
#define TYPE_LOG_ERROR "LOG_ERROR"
#define TYPE_LOG_WARN "LOG_WARN"
#define TYPE_LOG_FATAL "LOG_FATAL"

class LogSystem
{
public:
    // return ptr of the log system
    // The Log_init function needs to be called first
    static LogSystem *instance();

    // init
    void Log_init(std::string filePath = LOG_FILE_PATH);

public:
    // print debug into the screan
    bool log_debug(const char *file, const int line, const char *pLogFormat, ...);
    // print info into the screan
    bool log_info(const char *pLogFormat, ...);
    // print error into the screan
    bool log_error(const char *pLogFormat, ...);
    // print warn into the screan
    bool log_warn(const char *pLogFormat, ...);
    // print fatal into the screan
    bool log_fatal(const char *pLogFormat, ...);

    // write into the log file without time and log type
    void Log_write(const char *pLogFormat);
    // read and print the log file
    void Log_print_logfile();

    // set enable output file
    void Log_setOutputFile(bool state) { m_outputFile = state; }
    // set file path
    void Log_setFilePath(std::string filePath) { m_filePath = filePath; }

    //
    void operator()(const char *file, const int line, const char *logType, const char *pLogFormat, ...);

private:
    LogSystem(bool outputFile = true);

public:
    ~LogSystem()
    {
        char path[255];
        if (NULL != getcwd(path, 255))
            printf("\nyou can see the Log file in the %s/%s\n", path, m_logFileName.c_str());

        if (m_WriteStream.is_open())
        {
            m_WriteStream.close();
        }
    }

private:
    static LogSystem *m_logSystem; // log system ptr
    bool m_outputFile;             // write into file
    std::string m_filePath = LOG_FILE_PATH;
    std::string m_logFileName;
    std::ofstream m_WriteStream;
    bool m_initState = false; // true when init is done
};
}

#endif // __LOG_H__
