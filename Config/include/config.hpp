/**
 * @file config.hpp
 * @author
 * @brief
 * @version 0.1
 * @date 2023-02-09
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __CONFIG_H__
#define __CONFIG_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <unistd.h>
#include <fstream>
#include <unistd.h>
#include <map>
#include <cstring>

/*______ F U N C T I O N _____________________________________________________*/

namespace CONFIG
{
    class Config
    {
    public:
        /// @brief Get Value by Node and Value name
        /// @param _NodeName
        /// @param _ValueName
        /// @param _value return value
        /// @return Processing result
        int Config_GetValue(const char *_NodeName, const char *_ValueName, int &_value);
        int Config_GetValue(const char *_NodeName, const char *_ValueName, float &_value);
        int Config_GetValue(const char *_NodeName, const char *_ValueName, std::string &_value);
        int Config_GetValue(const char *_NodeName, const char *_ValueName, bool &_value);

    public:
        Config(const int _argc, char *const _argv[]);
        ~Config();

    private:
        /// @brief Parse Command
        /// @param _argc Command
        /// @param _argv Parameter list
        /// @return Processing result
        int Config_parseCommand(const int _argc, char *const _argv[]);

        /// @brief get config
        /// @return Processing result
        int Config_getConfig();

        /// @brief
        /// @param _configFilePath
        /// @return Processing result
        int Config_readFile(std::string _configFilePath);
    };

    union un_value
    {
        bool value_bool;
        int value_int;
        float value_float;
        char value_str[1024];
    };

    class ConfigList
    {
    public:
        /// @brief Get the node value by Node name
        /// @param _NodeName
        /// @param _ValueName
        /// @param _value
        /// @return
        int ConfigList_getValue(const char *_NodeName, const char *_ValueName, int &_value);
        int ConfigList_getValue(const char *_NodeName, const char *_ValueName, float &_value);
        int ConfigList_getValue(const char *_NodeName, const char *_ValueName, std::string &_value);
        int ConfigList_getValue(const char *_NodeName, const char *_ValueName, bool &_value);

        /// @brief Set the node and value
        /// @param _NodeName    The
        /// @param _ValueName
        /// @param _value
        /// @return
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, int _value);
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, float _value);
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, std::string _value);
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, bool _value);

        /// @brief Creat New Node
        /// @param _NodeName
        /// @return
        int ConfigList_CreatNode(const char *_NodeName);

        ConfigList() {}
        ~ConfigList() {}

    private:
        /// @brief Set the value to the Node list
        /// @param _NodeName
        /// @param _ValueName
        /// @param _value
        /// @return
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, un_value &_value);

        /// @brief Get the value from the Node list
        /// @param _NodeName
        /// @param _ValueName
        /// @param _value
        /// @return
        int ConfigList_getValue(const char *_NodeName, const char *_ValueName, un_value &_value);
    };
}

#endif // __CONFIG_H__