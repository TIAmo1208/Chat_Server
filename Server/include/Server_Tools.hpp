/**
 * @file Server_Tools.hpp
 * @author TIAmo (s13144281208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2023-04-21
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __SERVER_TOOLS_H__
#define __SERVER_TOOLS_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <queue>
#include <string>
#include <vector>

#include "Server_config.h"

/*______ D E F I N E _________________________________________________________*/

/*______ C L A S S ___________________________________________________________*/

class Server_Tools
{
    /*______ F U N C T I O N _________________________________________________*/
public:
    Server_Tools();
    ~Server_Tools();

    /// @brief convert char to int
    /// @param str
    /// @return code
    int Server_Tools_convert_hexChar_to_int(std::string &str);

    /// @brief convert char to int
    /// @param str
    /// @return code
    int Server_Tools_convert_Char_to_int(std::string &_str);

    /// @brief convert int to char
    /// @param _num
    /// @param _ret
    void Server_Tools_convert_int_to_char(int _num, std::string &_ret);

private:
    /*______ V A R I A B L E _________________________________________________*/

    int m_series_dec[CODE_LENGHT] = {1000, 100, 10, 1};
    int m_series_hex[CODE_LENGHT] = {65536, 256, 16, 1};
};

/*______ C L A S S ___________________________________________________________*/

template <typename Key, typename ValueType> class Thread_Map
{
public:
    /*______ F U N C T I O N _________________________________________________*/
    Thread_Map() { m_map = std::map<Key, ValueType>(); }
    ~Thread_Map() {}

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_map.empty();
    }

    void erase(const Key &_key)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        m_map.erase(_key);
    }

    ValueType &operator[](const Key &_key)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_map[_key];
    }

private:
    /*______ V A R I A B L E _________________________________________________*/

    std::map<Key, ValueType> m_map;
    std::mutex m_mutex_value;
};

/*______ C L A S S ___________________________________________________________*/

class Thread_List
{
public:
    /*______ F U N C T I O N _________________________________________________*/
    Thread_List()
    {
        for (int i = 0; i < FD_ARRAY_MAXSIZE; ++i)
        {
            m_list[i] = -1;
        }
    }
    ~Thread_List() {}

    const int &operator[](const int &_key)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_list[_key];
    }

    void add(const int &_key, const int &value)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        if (m_list[_key] == -1)
        {
            m_lenght++;
        }
        m_list[_key] = value;
    }

    void erase(const int &_key)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        if (m_list[_key] != -1)
        {
            m_lenght--;
            m_list[_key] = -1;
        }
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_lenght == 0;
    }

    size_t lenght()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_lenght;
    }

private:
    /*______ V A R I A B L E _________________________________________________*/

    int m_list[FD_ARRAY_MAXSIZE];
    int m_lenght = 0;
    std::mutex m_mutex_value;
};

/*______ C L A S S ___________________________________________________________*/

template <typename ValueType> class Thread_Queue
{
public:
    /*______ F U N C T I O N _________________________________________________*/
    Thread_Queue() { m_task = std::queue<ValueType>(); }
    ~Thread_Queue() {}

    ValueType pop()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        ValueType value = m_task.front();
        m_task.pop();
        return value;
    }

    void push(const ValueType &_value)
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        m_task.push(_value);
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return m_task.empty();
    }

private:
    /*______ V A R I A B L E _________________________________________________*/

    std::queue<ValueType> m_task;
    std::mutex m_mutex_value;
};

#endif // __SERVER_TOOLS_H__