/**
 * @file Log.h
 * @author
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

#include <chrono>
#include <string>

/*______ D E F I N E _________________________________________________________*/
namespace Log
{
#define Log_debug(format, ...) LogSystem::instance()->log_debug(__FILE__, __LINE__, format, ##__VA_ARGS__)
#define Log_info(format, ...) LogSystem::instance()->log_info(format, ##__VA_ARGS__)
#define Log_error(format, ...) LogSystem::instance()->log_error(format, ##__VA_ARGS__)
#define Log_warn(format, ...) LogSystem::instance()->log_warn(format, ##__VA_ARGS__)
#define Log_fatal(format, ...) LogSystem::instance()->log_fatal(format, ##__VA_ARGS__)

class LogSystem
{
public:
    /// @brief init and return ptr of the log system.
    /// @brief The Log_init function needs to be called first
    /// @return
    static LogSystem *instance();

    /// @brief initialization
    /// @param _log_level
    /// @param _log_file_enable
    /// @param _filePath
    void Log_init(int _log_level = 3, bool _log_file_enable = true, std::string _filePath = "./Log");

public:
    /// @brief print debug into the screan
    /// @param file
    /// @param line
    /// @param pLogFormat
    /// @param
    /// @return
    bool log_debug(const char *file, const int line, const char *pLogFormat, ...);

    /// @brief print info into the screan
    /// @param pLogFormat
    /// @param
    /// @return
    bool log_info(const char *pLogFormat, ...);

    /// @brief print error into the screan
    /// @param pLogFormat
    /// @param
    /// @return
    bool log_error(const char *pLogFormat, ...);

    /// @brief print warn into the screan
    /// @param pLogFormat
    /// @param
    /// @return
    bool log_warn(const char *pLogFormat, ...);

    /// @brief print fatal into the screan
    /// @param pLogFormat
    /// @param
    /// @return
    bool log_fatal(const char *pLogFormat, ...);

    /// @brief Set the state of the output file
    /// @param state true: ouput; false: No output
    void Log_set_OutputFile(bool state);

    /// @brief set file path
    /// @brief If you want to set the file path, you have to set this path before use log function.
    /// @param filePath
    void Log_set_FilePath(std::string &_filePath);

    /// @brief set log level
    /// @param _level
    void Log_set_LogLevel(int _level);

    /// @brief set the log output time
    /// @param _seconds The default time is 10 seconds
    void Log_set_LogOutput_Time(int _seconds = 10);

    /// @brief output log with instance
    /// @param file
    /// @param line
    /// @param logType
    /// @param pLogFormat
    /// @param
    void operator()(const char *file, const int line, const char *logType, const char *pLogFormat, ...);

private:
    LogSystem();

public:
    void del_object();
    ~LogSystem();
};
} // namespace Log

#endif // __LOG_H__
