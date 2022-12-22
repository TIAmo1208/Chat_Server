/**
 * @file Log.cpp
 * @author Sun Qiuming (qiuming.sun@external.marelli.com)
 * @brief
 * @version 0.1
 * @date 2022-10-28
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Log.h"

using namespace Log;

LogSystem *LogSystem::m_logSystem = nullptr;

// generate log txt
char *setLogTxt(const char *logLevel, const char *pLogFormat)
{
    // time for log
    char date[32];
    time_t t = time(0);
    strftime(date, sizeof(date), "%H:%M:%S", localtime(&t));

    // log txt
    char *logTxt = new char[2048];
    memset(logTxt, 0, sizeof(&logTxt));
    sprintf(logTxt, "[%s]:[%s]\t%s", date, logLevel, pLogFormat);

    return logTxt;
}

// using the time as file name
std::string getLogFileName(std::string filePath)
{
    // get log file's name
    time_t t = time(0);
    char fileName[32];
    strftime(fileName, sizeof(fileName), "%Y-%m-%d", localtime(&t));
    strcat(fileName, ".log");

    // Created when no folder exists
    if (F_OK != access(filePath.c_str(), 0))
    {
        std::string temp = "mkdir " + filePath;
        int ret = system(temp.c_str());
    }

    filePath = filePath + "/" + fileName;

    return filePath;
}

// init and return ptr of the log system
LogSystem *LogSystem::instance()
{
    if (m_logSystem == nullptr)
    {
        m_logSystem = new LogSystem(true);
    }
    return m_logSystem;
}

void LogSystem::Log_init(std::string filePath)
{
    if (m_initState)
    {
        log_error("The Log Log has completed initialization, the log path:%s", m_filePath);
        return;
    }

    std::string tmpName(getLogFileName(m_filePath = filePath));

    // if not exist the log file,create and write new log
    if (0 != access(tmpName.c_str(), 0))
    {
        log_info("start create new log file");
    }
    else
    {
        m_WriteStream = std::ofstream(m_logFileName = tmpName, std::ios::app);
        if (!m_WriteStream.is_open())
        {
            m_outputFile = false;
            log_fatal("the ofstream open fail %s:%d", __FILE__, __LINE__);
            return;
        }
    }

    m_initState = true;
}

// default constructor
LogSystem::LogSystem(bool outputFile) : m_outputFile(outputFile) {}

// write the log into the file without time and log type
void LogSystem::Log_write(const char *pLogFormat)
{
    // write into file
    std::string tmpName(getLogFileName(m_filePath));
    if (tmpName != m_logFileName)
    {
        if (m_WriteStream.is_open())
            m_WriteStream.close();

        //
        m_WriteStream = std::ofstream(m_logFileName = tmpName, std::ios::app);
        if (!m_WriteStream.is_open())
        {
            m_outputFile = false;
            log_fatal("the ofstream open fail %s:%d", __FILE__, __LINE__);
            return;
        }
    }

    if (m_WriteStream.is_open())
        m_WriteStream << pLogFormat << std::endl;
}

// read and print the log file
void LogSystem::Log_print_logfile()
{
    std::ifstream readStream(m_logFileName, std::ios::in);
    if (!readStream.is_open())
    {
        log_error("the ifstream open fail %s:%d", __FILE__, __LINE__);
        return;
    }

    char buf[1024] = {0};
    while (readStream.getline(buf, sizeof(buf)))
    {
        printf("%s\n", buf);
    }
}

// print debug into the screan
bool LogSystem::log_debug(const char *file, const int line, const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    //
    char *logTxt = setLogTxt(TYPE_LOG_DEBUG, temp);
    printf("%s\n %s:%d\n", logTxt, file, line);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
    return true;
}

// print info into the screan
bool LogSystem::log_info(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    //
    char *logTxt = setLogTxt(TYPE_LOG_INFO, temp);
    printf("%s\n", logTxt);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
    return true;
}

// print error into the screan
bool LogSystem::log_error(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    //
    char *logTxt = setLogTxt(TYPE_LOG_ERROR, temp);
    printf("%s\n", logTxt);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
    return true;
}

// print warn into the screan
bool LogSystem::log_warn(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    //
    char *logTxt = setLogTxt(TYPE_LOG_WARN, temp);
    printf("%s\n", logTxt);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
    return true;
}

// print fatal into the screan
bool LogSystem::log_fatal(const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList);

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1;
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    //
    char *logTxt = setLogTxt(TYPE_LOG_FATAL, temp);
    printf("%s\n", logTxt);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
    return true;
}

//
void LogSystem::operator()(const char *file, const int line, const char *logType, const char *pLogFormat, ...)
{
    // dereference
    va_list paramList, parm_copy;
    va_start(paramList, pLogFormat);
    va_copy(parm_copy, paramList); // need copy

    int size = vsnprintf(0, 0, pLogFormat, paramList) + 1; // get size and clear paramList
    char *temp = (char *)malloc(size);
    vsnprintf(temp, size, pLogFormat, parm_copy);

    va_end(paramList);

    // create log txt
    char *logTxt = setLogTxt(logType, temp);
    sprintf(logTxt, "%s %s:%d", logTxt, file, line);
    printf("%s\n", logTxt);

    if (m_outputFile)
        Log_write(logTxt);

    free(temp);
    free(logTxt);
}
