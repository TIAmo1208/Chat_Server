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

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <new>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "Server_config.h"

#include "Log.h" // Log
using namespace Log;

/*______ D E F I N E _________________________________________________________*/

#define FILE_PATH "./file/"

enum Return_Code
{
    Success        = 0,
    File_Open_Fail = -1,
    CRC_Check_Fail = -2
};

/*______ C L A S S ___________________________________________________________*/

class Server_Tools
{
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

    struct s_FileTask
    {
        std::string fileName = "";
        std::string filePath = "./file/";
        size_t fileSize      = -1;

        int sendTarget = -1;
    };

    /// @brief open file
    /// @param _task
    /// @return
    Return_Code Server_Tools_open_file(s_FileTask _task);

    struct s_FileBlock
    {
        int sendTarget = -1;

        int block_num;    // 接收到的文件块编号
        int block_size;   // 接收到的文件块大小
        char *block_data; // 接收到的文件块数据
    };

    /// @brief save file
    /// @param _block
    /// @return
    Return_Code Server_Tools_save_file(s_FileBlock *_block);

private:
    int m_series_dec[CODE_LENGHT] = {1000, 100, 10, 1};
    int m_series_hex[CODE_LENGHT] = {65536, 256, 16, 1};

    std::unordered_map<int, s_FileTask> m_list_task;
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
    Thread_Queue()
    {
        this->m_node_front = m_node_end = nullptr;
        this->m_length                  = 0;
        this->m_TerminateFlag           = false;
    }
    ~Thread_Queue() { quit(); }

    /// @brief Wait and get the value from queue
    /// @return
    ValueType pop()
    {
        if (empty())
        {
            std::mutex mutex;
            std::unique_lock<std::mutex> lock(mutex);
            this->m_condition_task.wait(lock, [this] { return this->m_length != 0 || this->m_TerminateFlag; });
        }

        if (this->m_TerminateFlag)
        {
            std::unique_lock<std::mutex> lock(m_mutex_value);
            while (m_node_front != nullptr)
            {
                Node *temp = m_node_front->m_nextNode;
                delete m_node_front;
                m_node_front = temp;
            }
            return ValueType();
        }

        ValueType value = {0};
        {
            std::unique_lock<std::mutex> lock(m_mutex_value);
            value = std::move(m_node_front->m_value);

            this->m_length.store(m_length - 1, std::memory_order_acquire);
            Node *temp = m_node_front->m_nextNode;
            delete m_node_front;
            if (temp == nullptr)
            {
                m_node_front = m_node_end = nullptr;
            }
            else
            {
                m_node_front = temp;
            }
        }

        return std::move(value);
    }

    /// @brief Push new value into queue and notify one to pop
    /// @param _value
    void push(const ValueType &_value)
    {
        Node *temp = new Node(std::move(_value), nullptr);
        {
            std::unique_lock<std::mutex> lock(m_mutex_value);
            if (m_node_front == nullptr)
            {
                m_node_front = m_node_end = temp;
            }
            else
            {
                m_node_end->m_nextNode = temp;
                m_node_end             = temp;
            }
        }

        this->m_length.store(m_length + 1, std::memory_order_release);
        this->m_condition_task.notify_one();
    }

    bool empty()
    {
        std::unique_lock<std::mutex> lock(m_mutex_value);
        return this->m_length == 0;
    }

    void quit()
    {
        this->m_TerminateFlag = true;
        this->m_condition_task.notify_all();
    }

private:
    class Node
    {
    public:
        Node(ValueType _value = {0}, Node *_nextNode = nullptr) : m_value(_value), m_nextNode(_nextNode) {}

        ValueType m_value = {0};
        Node *m_nextNode  = nullptr;
    };
    Node *m_node_front = nullptr;
    Node *m_node_end   = nullptr;
    std::atomic<int> m_length;

    std::mutex m_mutex_value;
    std::condition_variable m_condition_task;

    bool m_TerminateFlag;
};

/*______ C L A S S ___________________________________________________________*/

/// @brief No Lock Queue. Only used in single producer and single consumer scenarios
///
/// @tparam ValueType
template <typename ValueType> class NoLock_Queue
{
private:
    enum ValueState
    {
        Readable,
        Read,
        Writeable,
        Write
    };
    enum ReturnCode
    {
        TaskListSpacial,
        TaskListFull
    };
    struct s_Task
    {
        ValueState state = ValueState::Writeable;
        ValueType value;
    };

public:
    NoLock_Queue(int _num)
    {
        if (_num <= 0)
        {
            return;
        }

        m_task = new s_Task[_num];
        size   = _num;
    }
    ~NoLock_Queue()
    {
        if (m_task != nullptr)
        {
            delete[] m_task;
        }
        delete m_task;

        while (!m_cashQueue.empty())
        {
            m_cashQueue.pop();
        }
    }

    /// @brief Push Value into list. If the cache queue no empty, It will export cache first.
    ///     After clearing the cache, if the list is still full, the cache is populated with data.
    //      If the list is fully loaded for a long time, expand the list size.
    /// @param _value
    void push(ValueType &_value)
    {
        if (ReturnCode::TaskListSpacial == dump())
        {
            SaveValue(_value);
        }
        else
        {
            Log_warn("push data into cache. If the list size is full for a long time, please modify the list size, or check the task "
                     "processing");
            m_cashQueue.push(std::move(_value));
            m_queue_len += 1;
        }
    }

    /// @brief Pop Value from list.
    ///     check the empty first, when use the pop
    /// @return
    ValueType pop()
    {
        if (ValueState::Readable == m_task[m_front].state)
        {
            m_task[m_front].state = ValueState::Read;
            ValueType task        = std::move(m_task[m_front].value);
            m_task[m_front].state = ValueState::Writeable;

            if (++m_front >= size)
            {
                m_front = 0;
            }

            return std::move(task);
        }
        else
        {
            return ValueType();
        }
    }

    /// @brief return empty state
    /// @return 0:list no empty; 1:list is empty; 2:list and cache is empty;
    int empty()
    {
        int ret = 0;
        if ((m_front == m_end) && (m_task[m_front].state == ValueState::Writeable))
        {
            if (m_cashQueue.empty())
            {
                ret = 2;
            }
            else
            {
                ret = 1;
            }
        }

        return ret;
    }

    /// @brief Clear cache queue, can only be used on producer threads
    /// @return
    ReturnCode dump()
    {
        while (!m_cashQueue.empty())
        {
            if (m_task[m_end].state != ValueState::Writeable)
            {
                return ReturnCode::TaskListFull;
            }

            Log_debug("dump Data from cache queue");
            SaveValue(m_cashQueue.front());
            m_cashQueue.pop();
            m_queue_len -= 1;
        }

        if (m_task[m_end].state != ValueState::Writeable)
        {
            return ReturnCode::TaskListFull;
        }
        else
        {
            return ReturnCode::TaskListSpacial;
        }
    }

private:
    void SaveValue(ValueType &_task)
    {
        if (ValueState::Writeable == m_task[m_end].state)
        {
            m_task[m_end].state = ValueState::Write;
            m_task[m_end].value = std::move(_task);
            m_task[m_end].state = ValueState::Readable;

            if (++m_end >= size)
            {
                m_end = 0;
            }
        }
    }

private:
    s_Task *m_task;
    std::queue<ValueType> m_cashQueue;

    int size        = 0;
    int m_queue_len = 0;
    int m_front     = 0;
    int m_end       = 0;
};

#endif // __SERVER_TOOLS_H__