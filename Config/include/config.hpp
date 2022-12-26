/**
 * @file configConst.hpp
 * @author TIAmo (tiamo1208@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __CONFIGCONST_HPP__
#define __CONFIGCONST_HPP__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "configConst.h"
#include "../../Log/include/Log.h"
using namespace Log;

#include <string>
#include <unistd.h>
#include <fstream>
#include <unistd.h>
#include <map>

/*______ D E F I N E _________________________________________________________*/

#define Node_Type_Int "int"
#define Node_Type_Float "float"
#define Node_Type_String "string"

/*______ F U N C T I O N _____________________________________________________*/

namespace CONFIG
{
    class ConfigList;

    class Config
    {
    public:
        Config(const int _argc, char *const _argv[]);
        ~Config();

    private:
        /**
         * @brief Parse Command
         *
         * @param _argc Command
         * @param _argv Parameter list
         * @return Processing result
         */
        int config_parseCommand(const int _argc, char *const _argv[]);

        /**
         * @brief get config
         *
         * @return int
         */
        int config_getConfig();

        int config_readFile(std::string _configFilePath);

    private:
        std::string m_configFilePath = "";
        std::ifstream m_fs_input; // config file input file stream

        ConfigList *m_config_list;
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

        /// @brief Set the node and value
        /// @param _NodeName
        /// @param _ValueName
        /// @param _value
        /// @return
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, int _value);
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, float _value);
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, std::string _value);

        /// @brief Creat New Node
        /// @param _NodeName
        /// @return
        int ConfigList_CreatNode(const char *_NodeName);

        ConfigList() {}
        ~ConfigList() {}

    private:
        union un_value
        {
            int value_int;
            float value_float;
            char *value_str;
        };

    private:
        int ConfigList_setValue(const char *_NodeName, const char *_ValueName, un_value &_value);

    private:
        std::map<std::string, std::map<std::string, un_value>> m_Nodelist;
    };
}

#endif // __CONFIGCONST_HPP__
