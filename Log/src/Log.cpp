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

#include "../include/Log.h"

using namespace Log;

/*______ D E F I N E _________________________________________________________*/

#define LOG_LEVEL_NONE (0)
#define LOG_LEVEL_ERROR (1)
#define LOG_LEVEL_WARN (2)
#define LOG_LEVEL_INFO (3)
#define LOG_LEVEL_DEBUG (4)

#define TYPE_LOG_DEBUG "LOG_DEBUG"
#define TYPE_LOG_INFO "LOG_INFO"
#define TYPE_LOG_ERROR "LOG_ERROR"
#define TYPE_LOG_WARN "LOG_WARN"
#define TYPE_LOG_FATAL "LOG_FATAL"

/*______ V A R I A B L E _____________________________________________________*/

static LogSystem *m_logSystem = nullptr; // log system ptr

bool m_outputFile = true; // write into file
std::string m_filePath = "Log";

int m_log_level = LOG_LEVEL_INFO;

std::string m_logFileName = "";
std::ofstream m_WriteStream;
bool m_initState = false; // true when init is done

/*______ F U N C T I O N _____________________________________________________*/

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
        m_logSystem = new LogSystem();
    }
    return m_logSystem;
}

void LogSystem::Log_init(int _log_level, bool _log_file_enable, std::string _filePath)
{
    if (m_initState)
    {
        Log_debug("The Log Log has completed initialization, the log path:%s", m_filePath);
        return;
    }

    m_log_level = _log_level;
    m_outputFile = _log_file_enable;
    m_filePath = _filePath;

    if (m_outputFile)
    {
        std::string tmpName(getLogFileName(m_filePath));

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
                log_fatal("the ofstream open fail, please check the file path");
                return;
            }
        }
    }

    m_initState = true;
}

// default constructor
LogSystem::LogSystem() {}

// destructor
LogSystem::~LogSystem()
{
    char path[255];
    if (NULL != getcwd(path, 255))
        printf("\nyou can see the Log file in the %s/%s\n", path, m_logFileName.c_str());

    if (m_logSystem != nullptr)
    {
        delete m_logSystem;
        m_logSystem = nullptr;
    }
    if (m_WriteStream.is_open())
    {
        m_WriteStream.close();
    }
}

// write the log into the file without time and log type
void Log_write(const char *pLogFormat)
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
            Log_error("the ofstream open fail, please check the file path");
            return;
        }
    }

    if (m_WriteStream.is_open())
        m_WriteStream << pLogFormat << std::endl;
}

// read and print the log file
void Log_print_logfile()
{
    std::ifstream readStream(m_logFileName, std::ios::in);
    if (!readStream.is_open())
    {
        Log_error("the ifstream open fail, please check the file path");
        return;
    }

    char buf[1024] = {0};
    while (readStream.getline(buf, sizeof(buf)))
    {
        Log_error("%s", buf);
    }
}

// print debug into the screan
bool LogSystem::log_debug(const char *file, const int line, const char *pLogFormat, ...)
{
    if (m_log_level < LOG_LEVEL_DEBUG)
        return false;

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
    if (m_log_level < LOG_LEVEL_INFO)
        return false;

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
    if (m_log_level < LOG_LEVEL_ERROR)
        return false;

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
    if (m_log_level < LOG_LEVEL_WARN)
        return false;

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
    if (m_log_level == LOG_LEVEL_NONE)
        return false;

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

//
void LogSystem::Log_set_OutputFile(bool state) { m_outputFile = state; }

//
void LogSystem::Log_set_FilePath(std::string filePath) { m_filePath = filePath; }

//
void LogSystem::Log_set_LogLevel(int _level) { m_log_level = _level; }
