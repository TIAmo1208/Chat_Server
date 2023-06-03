
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Server_Tools.hpp"

#include <fstream>

/*______ F U N C T I O N _________________________________________________*/

Server_Tools::Server_Tools() {}
Server_Tools ::~Server_Tools() {}

/// @brief convert char to int
/// @param str
/// @return code
int Server_Tools::Server_Tools_convert_hexChar_to_int(std::string &str)
{
    int temp[CODE_LENGHT];
    int ret = 0, size = CODE_LENGHT;

    for (int i = 0; i < size; ++i)
    {
        if (str[i] >= 48 && str[i] <= 57)
        {
            temp[i] = str[i] - 48;
        }
        else if (str[i] >= 65 && str[i] <= 70)
        {
            temp[i] = str[i] - 55;
        }
        ret += temp[i] * m_series_hex[i];
    }

    if (ret < 0)
        return -1;
    else
        return ret;
}

/// @brief convert char to int
/// @param str
/// @return code
int Server_Tools::Server_Tools_convert_Char_to_int(std::string &_str)
{
    int temp[CODE_LENGHT];
    int ret = 0, size = CODE_LENGHT;

    for (int i = 0; i < size; ++i)
    {
        if (_str[i] >= 48 && _str[i] <= 57)
        {
            temp[i] = _str[i] - 48;
        }
        ret += temp[i] * m_series_dec[i];
    }

    if (ret < 0)
        return -1;
    else
        return ret;
}

/// @brief convert int to char
/// @param _num
/// @param _ret
void Server_Tools::Server_Tools_convert_int_to_char(int _num, std::string &_ret)
{
    int size = CODE_LENGHT;
    _ret.resize(size);

    for (int i = 0; i < size; ++i)
    {
        _ret[i] = (char) ((_num / m_series_dec[i]) + 48);
        _num    = _num % m_series_dec[i];
    }
}

/// @brief open file
/// @param _task
/// @return
Return_Code Server_Tools::Server_Tools_open_file(s_FileTask _task)
{
    if (_task.sendTarget == -1)
    {
        return Return_Code::File_Open_Fail;
    }

    std::ofstream outfile(_task.filePath + _task.fileName);
    if (!outfile.is_open())
    {
        return Return_Code::File_Open_Fail;
    }

    Log_debug("Server_Tools_open_file: Save path:%s", (_task.filePath + _task.fileName).c_str());

    // 关闭文件
    outfile.clear();
    outfile.close();

    m_list_task[_task.sendTarget] = _task;
    return Return_Code::Success;
}

/// @brief save file
/// @param _block
/// @return
Return_Code Server_Tools::Server_Tools_save_file(s_FileBlock *_block)
{
    if (m_list_task[_block->sendTarget].sendTarget == -1)
    {
        Log_debug("Server_Tools_save_file: Send Target is null");
        return Return_Code::File_Open_Fail;
    }

    std::string filePath = m_list_task[_block->sendTarget].filePath + m_list_task[_block->sendTarget].fileName;
    std::ofstream outfile(filePath);
    if (!outfile.is_open())
    {
        Log_warn("Server_Tools_save_file: File %s open fail, path :%s", m_list_task[_block->sendTarget].fileName.c_str(), filePath.c_str());
        return Return_Code::File_Open_Fail;
    }

    long offset = (_block->block_num - 1) * _block->block_size; // 计算文件写入位置
    outfile.seekp(offset, std::ios::beg);                       // 移动文件指针到写入位置
    outfile.write(_block->block_data, _block->block_size);      // 写入
    outfile.close();                                            // 关闭文件

    return Return_Code::Success;
}
