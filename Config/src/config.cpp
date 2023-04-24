/**
 * @file configConst.cpp
 * @author TIAmo (tiamo1208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-23
 *
 * @copyright Copyright (c) 2022
 *
 */

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "config.h"

#include <cstring>
#include <fstream>
#include <map>
#include <unistd.h>

#include <thread>

#include <fcntl.h>
#include <sys/stat.h>

using namespace CONFIG;

/*______ D E F I N E _________________________________________________________*/

#define Config_Success 0
#define Config_Fail -1

#define Node_Type_Undefined -1
#define Node_Type_Int 0
#define Node_Type_Float 1
#define Node_Type_String 2
#define Node_Type_Bool 3

const char COMMOND_PATH[] = "./Commond";
#define COMMOND_MAX_SIZE 8

/*______ V A R I A B L E _____________________________________________________*/

bool m_fileOpenState = false;
bool m_stop          = false;

std::string m_configFilePath = "./config.cfg";
std::ifstream m_fs_input; // config file input file stream

ConfigList *m_config_list;

std::map<std::string, std::map<std::string, un_value>> m_Nodelist;

Function_CommonHandle m_Handle_Function = nullptr;

std::thread m_thread_GetCommond;

/*______ L O C A L - F U N C T I O N _________________________________________*/

void getCommond()
{
    {
        char *path = new char[1024];
        sprintf(path, "touch %s", COMMOND_PATH);
        system(path);
        delete[] path;
    }

    int fd = open(COMMOND_PATH, O_RDWR);

    // 获取文件状态
    struct stat fileState;
    fstat(fd, &fileState);
    off_t fileSize = fileState.st_size;
    char buffer[COMMOND_MAX_SIZE];

    while (!m_stop)
    {
        // 读取大小为空
        while (fileSize == 0)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            fstat(fd, &fileState);
            fileSize = fileState.st_size;

            if (m_stop)
            {
                break;
            }
        }

        if (m_stop)
        {
            break;
        }

        // 读取文件内容
        int len = read(fd, buffer, fileSize);
        if (len == -1)
        {
            printf("读取错误\n");
        }

        // 判断代码类型
        if (m_Handle_Function != nullptr)
        {
            (*m_Handle_Function)(buffer);
        }

        if (fcntl(fd, F_GETFD) < 0)
        {
            printf("获取文件描述符状态失败\n");
        }
        // 截断文件大小
        ftruncate(fd, 0);
        fileSize = 0;
    }

    close(fd);
}
/*______ F U N C T I O N _____________________________________________________*/

Config::Config(const int _argc, char *const _argv[])
{
    m_config_list = new ConfigList;
    try
    {
        if (this->Config_parseCommand(_argc, _argv) == Config_Success)
        {
            Config_getConfig();
        }
        else
        {
            throw "Error on command line options ";
        }
    }
    catch (const std::exception &e)
    {
        printf("%s \n", e.what());
    }

    m_thread_GetCommond = std::thread(getCommond);
}

Config::~Config()
{
    if (m_config_list != nullptr)
        delete m_config_list;

    m_stop = true;
    m_thread_GetCommond.join();
}

int Config::Config_parseCommand(const int _argc, char *const _argv[])
{
    int retval = -1;
    int opt;
    while ((opt = getopt(_argc, _argv, "c:m:d:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            m_configFilePath = optarg;
            printf("Command : c : config file path : %s \n", m_configFilePath.c_str());
            break;

        default:
            retval = -1;
            printf("Error : parse command Usage error \n");
            printf("Usage : program_name [-c config_file_path] \n");
            break;
        }
    }

    return retval = 0;
}

int Config::Config_getConfig()
{
    int retval = -1;

    try
    {
        this->Config_readFile(m_configFilePath);
    }
    catch (const std::exception &e)
    {
        printf("%s \n", e.what());
        return retval;
    }
    catch (const char *&e)
    {
        printf("%s \n", e);
        return retval;
    }

    return retval = 0;
}

int Config::Config_readFile(std::string _configFilePath)
{
    int retval = -1;

    // Confirm file status
    retval = access(_configFilePath.c_str(), R_OK);
    // return -1 when is not access
    if (retval < 0)
    {
        throw "The config file is Inaccessible or not exist";
        return retval;
    }

    // Open config file
    m_fs_input.open(_configFilePath);
    retval = m_fs_input.is_open();
    if (retval <= 0)
    {
        throw "The config file open error";
        return retval;
    }
    m_fileOpenState = true;

    // Get config file
    std::string str_buff, node_name;
    while (getline(m_fs_input, str_buff))
    {
        // if (str_buff[0] == '{')
        if (str_buff.find('{') != -1)
        {
            // Get the Child Node
            while (getline(m_fs_input, str_buff))
            {
                if (str_buff.find('}') != -1)
                    break;

                int value_type = Node_Type_Undefined;
                std::string value_name, value_str;
                // Gets the node name and value
                {
                    int i    = 0;
                    int size = str_buff.size();
                    char temp_str[1024];

                    // Skip space
                    for (; i < size; ++i)
                    {
                        if (str_buff[i] == ' ' || str_buff[i] == '\t')
                            continue;
                        else
                            break;
                    }
                    // break when this line is empty
                    if (i >= size)
                        continue;

                    // get value name
                    int count = 0;
                    memset(temp_str, 0, 1024);
                    for (; i < size; ++i)
                    {
                        if (str_buff[i] != ' ')
                            temp_str[count++] = str_buff[i];
                        else
                            break;
                    }
                    value_name = temp_str;

                    // skip " = "
                    for (; i < size; ++i)
                    {
                        if (str_buff[i] == ' ' || str_buff[i] == '=')
                            continue;
                        else
                            break;
                    }

                    // get value
                    count = 0;
                    memset(temp_str, 0, 1024);
                    for (; i < size; ++i)
                    {
                        if (str_buff[i] == ';')
                        {
                            break;
                        }

                        // Get the correct data type
                        if (str_buff[i] == ' ')
                        {
                            continue;
                        }
                        else
                        {
                            if (str_buff[i] == '"')
                            {
                                value_type = Node_Type_String;
                                continue;
                            }
                            else if (value_type != 1 && str_buff[i] >= '0' && str_buff[i] <= '9')
                            {
                                value_type = Node_Type_Int;
                            }
                            else if (value_type != 2 && str_buff[i] == '.')
                            {
                                value_type = Node_Type_Float;
                            }

                            temp_str[count] = str_buff[i];

                            ++count;
                        }
                    }
                    value_str = std::string(temp_str);

                    // set the node value
                    switch (value_type)
                    {
                    case Node_Type_Int:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), atoi(value_str.c_str()));
                        break;

                    case Node_Type_Float:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), (float) atof(value_str.c_str()));
                        break;

                    case Node_Type_String:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), value_str);
                        break;

                    default:
                        if (value_str == "true")
                        {
                            m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), true);
                            break;
                        }
                        else if (value_str == "false")
                        {
                            m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), false);
                            break;
                        }

                        printf("Config : The node :%s value type no detected \n", value_name.c_str());
                        break;
                    }
                }
            }
        }
        // Get the Node
        else
        {
            node_name = str_buff;
            m_config_list->ConfigList_CreatNode(node_name.c_str());
        }
    }

    return retval = 0;
}

int Config::Config_GetValue(const char *_NodeName, const char *_ValueName, int &_value)
{
    int retval = -1;
    if (m_config_list->ConfigList_getValue(_NodeName, _ValueName, _value) == Config_Fail)
    {
        return retval;
    }
    return retval = 0;
}
int Config::Config_GetValue(const char *_NodeName, const char *_ValueName, float &_value)
{
    int retval = -1;
    if (m_config_list->ConfigList_getValue(_NodeName, _ValueName, _value) == Config_Fail)
    {
        return retval;
    }
    return retval = 0;
}
int Config::Config_GetValue(const char *_NodeName, const char *_ValueName, std::string &_value)
{
    int retval = -1;
    if (m_config_list->ConfigList_getValue(_NodeName, _ValueName, _value) == Config_Fail)
    {
        return retval;
    }
    return retval = 0;
}
int Config::Config_GetValue(const char *_NodeName, const char *_ValueName, bool &_value)
{
    int retval = -1;
    if (m_config_list->ConfigList_getValue(_NodeName, _ValueName, _value) == Config_Fail)
    {
        return retval;
    }
    return retval = 0;
}

void Config::Config_Regist_Commond_handle(Function_CommonHandle _func) { m_Handle_Function = _func; }

/*______ F U N C T I O N _____________________________________________________*/

int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, int &_value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // return node value
    union un_value value;
    if (this->ConfigList_getValue(_NodeName, _ValueName, value) == Config_Success)
    {
        _value = value.value_int;
    }
    else
    {
        return retval;
    }

    return retval = 0;
}
int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, float &_value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // return node value
    union un_value value;
    if (this->ConfigList_getValue(_NodeName, _ValueName, value) == Config_Success)
    {
        _value = value.value_float;
    }
    else
    {
        return retval;
    }

    return retval = 0;
}
int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, std::string &_value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // return node value
    union un_value value;
    if (this->ConfigList_getValue(_NodeName, _ValueName, value) == Config_Success)
    {
        _value = std::string(value.value_str);
    }
    else
    {
        return retval;
    }

    return retval = 0;
}
int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, bool &_value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // return node value
    union un_value value;
    if (this->ConfigList_getValue(_NodeName, _ValueName, value) == Config_Success)
    {
        _value = value.value_bool;
    }
    else
    {
        return retval;
    }

    return retval = 0;
}

int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, int _value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // Add new parameter
    union un_value value;
    value.value_int = _value;
    retval          = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}
int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, float _value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // Add new parameter
    union un_value value;
    value.value_float = _value;
    retval            = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}
int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, std::string _value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // Add new parameter
    union un_value value;
    strcpy(value.value_str, _value.c_str());
    retval = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}
int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, bool _value)
{
    int retval = -1;

    if (strlen(_NodeName) == 0 || strlen(_ValueName) == 0)
    {
        printf("Config : The NodeName/ValueName is NULL \n");
        return retval;
    }

    // Add new parameter
    union un_value value;
    value.value_bool = _value;
    retval           = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}

int ConfigList::ConfigList_CreatNode(const char *_NodeName)
{
    int retval = -1;

    std::map<std::string, un_value> valuelist;
    m_Nodelist[_NodeName] = valuelist;

    return retval = 0;
}

int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, un_value &_value)
{
    int retval = -1;

    // Add new parameter
    std::map<std::string, un_value> valuelist = m_Nodelist[_NodeName];

    valuelist[_ValueName] = _value;
    m_Nodelist[_NodeName] = valuelist;

    return retval = 0;
}

int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, un_value &_value)
{
    int retval = -1;

    if (!m_fileOpenState)
        return retval;

    // find node name
    if (m_Nodelist.count(_NodeName) == -1)
    {
        printf("Config : The Node name is warning \n");
        return retval;
    }

    std::map<std::string, un_value> valuelist = m_Nodelist[_NodeName];
    // find value name
    if (valuelist.count(_ValueName) == -1)
    {
        printf("Config : The Value name is warning \n");
        return retval;
    }

    // Get value by Node name and value name
    _value = valuelist[_ValueName];

    return retval = 0;
}
