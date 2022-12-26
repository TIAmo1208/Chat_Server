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

#include "../include/config.hpp"

using namespace CONFIG;

/*______ D E F I N E _________________________________________________________*/

/*______ F U N C T I O N _____________________________________________________*/

Config::Config(const int _argc, char *const _argv[])
    : m_configFilePath("./config.cfg")
{
    LogSystem::instance()->Log_init();
    m_config_list = new ConfigList;
    try
    {
        if (this->config_parseCommand(_argc, _argv) >= 0)
        {
            config_getConfig();
        }
        else
        {
            throw "Error on command line options ";
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
}

Config::~Config()
{
    if (LogSystem::instance() != nullptr)
        delete LogSystem::instance();
    if (m_config_list != nullptr)
        delete m_config_list;
}

int Config::config_parseCommand(const int _argc, char *const _argv[])
{
    int retval = -1;
    int opt;
    while ((opt = getopt(_argc, _argv, "c:m:d:")) != -1)
    {
        switch (opt)
        {
        case 'c':
            m_configFilePath = optarg;
            Log_info("Command : c : config file path : %s", m_configFilePath.c_str());
            break;

        default:
            retval = -1;
            Log_error("Usage : program_name [-c config_file_path]");
            break;
        }
    }

    return retval = 0;
}

int Config::config_getConfig()
{
    int retval = -1;

    try
    {
        this->config_readFile(m_configFilePath);
    }
    catch (const std::exception &e)
    {
        Log_error("%s", e.what());
    }
    catch (const char *&e)
    {
        Log_error("%s", e);
    }

    return retval = 0;
}

int Config::config_readFile(std::string _configFilePath)
{
    int retval = -1;

    // Confirm file status
    retval = access(_configFilePath.c_str(), R_OK);
    // return -1 when is not access
    if (retval < 0)
    {
        throw "The config file is Inaccessible or nonexistent";
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

    // Get config file
    std::string str_buff, node_name;
    while (getline(m_fs_input, str_buff))
    {
        if (str_buff == "{")
        {
            while (getline(m_fs_input, str_buff))
            {
                if (str_buff == "}")
                    break;

                // 0 : int
                // 1 : float
                // 2 : string
                int value_type = -1;
                std::string value_name, value_str;
                // Gets the node name and value
                {
                    int i = 0;
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
                        // Get the correct data type
                        if (str_buff[i] == ' ' || str_buff[i] == ';')
                        {
                            continue;
                        }
                        else
                        {
                            if (str_buff[i] == '"')
                            {
                                value_type = 2;
                                continue;
                            }
                            else if (value_type != 2 && str_buff[i] >= '0' && str_buff[i] <= '9')
                            {
                                value_type = 0;
                            }
                            else if (value_type != 2 && str_buff[i] == '.')
                            {
                                value_type = 1;
                            }

                            temp_str[count] = str_buff[i];

                            ++count;
                        }
                    }
                    value_str = temp_str;

                    // set the node value
                    switch (value_type)
                    {
                    case 0:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), atoi(value_str.c_str()));
                        Log_info("Value name:%s, type:int, value:%d", value_name.c_str(), atoi(value_str.c_str()));
                        break;

                    case 1:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), (float)atof(value_str.c_str()));
                        Log_info("Value name:%s, type:float, value:%d", value_name.c_str(), (float)atof(value_str.c_str()));
                        break;

                    case 2:
                        m_config_list->ConfigList_setValue(node_name.c_str(), value_name.c_str(), value_str.c_str());
                        Log_info("Value name:%s, type:string, value:%s", value_name.c_str(), value_str.c_str());
                        break;

                    default:
                        Log_error("No Node value type detected");
                        break;
                    }

                }

            }

        }
        else
        {
            m_config_list->ConfigList_CreatNode(str_buff.c_str());
            Log_info("Node name:%s", str_buff.c_str());
        }

    }

    return retval = 0;
}

/*______ F U N C T I O N _____________________________________________________*/

int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, int &_value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // return node value
    union un_value value;
    std::map<std::string, un_value> valuelist = m_Nodelist[_NodeName];
    _value = valuelist[_ValueName].value_int;

    return retval = 0;
}
int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, float &_value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // return node value
    union un_value value;
    std::map<std::string, un_value> valuelist = m_Nodelist[_NodeName];
    _value = valuelist[_ValueName].value_float;

    return retval = 0;
}
int ConfigList::ConfigList_getValue(const char *_NodeName, const char *_ValueName, std::string &_value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // return node value
    union un_value value;
    std::map<std::string, un_value> valuelist = m_Nodelist[_NodeName];
    _value = valuelist[_ValueName].value_str;

    return retval = 0;
}

int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, int _value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // Add new parameter
    union un_value value;
    value.value_int = _value;
    retval = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}
int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, float _value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // Add new parameter
    union un_value value;
    value.value_float = _value;
    retval = this->ConfigList_setValue(_NodeName, _ValueName, value);

    return retval;
}
int ConfigList::ConfigList_setValue(const char *_NodeName, const char *_ValueName, std::string _value)
{
    int retval = -1;

    if (_NodeName == "" || _ValueName == "")
    {
        Log_warn("The NodeName/ValueName is NULL");
        return retval;
    }

    // Add new parameter
    union un_value value;
    strcpy(value.value_str, _value.c_str());
    retval = this->ConfigList_setValue(_NodeName, _ValueName, value);

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
