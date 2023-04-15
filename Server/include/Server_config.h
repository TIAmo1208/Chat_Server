/**
 * @file socket_config.h
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-02-11
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __SOCKET_I_H__
#define __SOCKET_I_H__

/*______ D E F I N E _________________________________________________________*/

//
#define VERSION (0.4)

// socket config
#define SOCKET_CONFIG_DOMAIN AF_INET;                // IPv4
#define SOCKET_CONFIG_TYPE SOCK_STREAM;              // Types of sockets
#define SOCKET_CONFIG_PROTOCOL IPPROTO_TCP;          // protocols
#define SOCKET_CONFIG_PORT 8888;                     // port
constexpr int FD_ARRAY_MAXSIZE = sizeof(fd_set) * 8; // fd_array's max size

// socket returned value
#define SOCKET_ERROR -1
#define SOCKET_SUCCESS 0

// select returned value
#define SELECT_ERROR -1
#define SELECT_TIME_OUT 0

// get or set Buffer size
#define BUFF_MAX_SIZE 1024

// Config
//  Log
#define CONFIG_MODULE_LOG "Log"
#define CONFIG_LOG_LogLevel "Log_Level"
#define CONFIG_LOG_LogFileEnable "Log_File_Enable"
#define CONFIG_LOG_FilePath "Log_File_Path"
//  Thread pool
#define CONFIG_MODULE_THREAD_POOL "Threadpool"
#define CONFIG_THREAD_POOL_THREAD_NUM "Threadpool_thread_num"
//  server
#define CONFIG_MODULE_SERVER "Server"
#define CONFIG_SERVER_Server_Port "Server_Port"

#endif // __SOCKET_I_H__