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

#include <iostream>
#include <fstream>
#include <time.h> // time txt
#include <cstring>
#include <stdarg.h> // parameter list
#include <unistd.h> // check path 、 pwd

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
        // return ptr of the log system
        // The Log_init function needs to be called first
        static LogSystem *instance();

        // init
        void Log_init(int _log_level = 3, bool _log_file_enable = true, std::string _filePath = "Log");

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

        // set enable output file
        void Log_set_OutputFile(bool state);
        // set file path
        // If you want to set the file path, you have to set this path before use log function.
        void Log_set_FilePath(std::string filePath);
        // set log level
        void Log_set_LogLevel(int _level);

        //
        void operator()(const char *file, const int line, const char *logType, const char *pLogFormat, ...);

    private:
        LogSystem();

    public:
        void del_object();
        ~LogSystem();
    };
}

#endif // __LOG_H__
