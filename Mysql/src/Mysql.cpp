/**
 * @file Mysql.cpp
 * @author  ()
 * @brief
 * @version 0.1
 * @date 2023-02-26
 *
 * @copyright Copyright (c) 2023
 *
 */
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "../include/Mysql.h"

/*______ D E F I N E _________________________________________________________*/

/*______ V A R I A B L E _____________________________________________________*/

static Mysql *s_mysqlSystem = nullptr; // mysql system ptr
bool initState = false;

MYSQL g_mysql;
MYSQL_RES *g_mysql_res = nullptr;
std::string g_dataBase;

/*______ F U N C T I O N _____________________________________________________*/

// Return execution result
//  1: Have a return value
//  0: No return value
//  -1: mysql error
int checkRepeat(std::string commond)
{
    int ret = -1;

    // execute the command
    if (mysql_real_query(&g_mysql, commond.c_str(), strlen(commond.c_str())))
    {
        printf("mysql select fail : %d \n", mysql_errno(&g_mysql));
        printf("commond : %s\n", commond.c_str());
        return ret = -1;
    }

    // get the result
    g_mysql_res = mysql_store_result(&g_mysql);
    if (nullptr == g_mysql_res)
    {
        printf("mysql store result fail : %d \n", mysql_errno(&g_mysql));
        return ret = -1;
    }

    //
    if (nullptr != mysql_fetch_row(g_mysql_res))
    {
        return ret = 1;
    }

    mysql_free_result(g_mysql_res);
    return ret = 0;
}

// Detect if the userID is duplicated
//  1: the user userID is exists
//  0: the user userID is no exists
//  -1: mysql error
int checkRepeat_UserID(std::string _userID)
{
    // assembly command
    std::stringstream strstream;
    strstream << "SELECT * FROM " << g_dataBase << ".User WHERE user_id = '" << _userID << "';";

    return checkRepeat(strstream.str());
}

// Detect if the name is duplicated
//  1: the user name is exists
//  0: the user name is no exists
//  -1: mysql error
int checkRepeat_UserName(std::string _userName)
{
    // assembly command
    std::stringstream strstream;
    strstream << "SELECT * FROM " << g_dataBase << ".User WHERE user_name = '" << _userName << "';";

    return checkRepeat(strstream.str());
}

/*______ F U N C T I O N _____________________________________________________*/

Mysql *Mysql::instance()
{
    if (s_mysqlSystem == nullptr)
    {
        s_mysqlSystem = new Mysql();
    }
    return s_mysqlSystem;
}

int Mysql::Mysql_init(std::string _hostname, std::string _username, std::string _password, std::string _database, int _port)
{
    int ret = -1;

    if (initState)
    {
        printf("mysql is already init");
        return ret = -1;
    }

    g_dataBase = _database;

    // init
    mysql_init(&g_mysql);

    // connect
    mysql_real_connect(&g_mysql, _hostname.c_str(), _username.c_str(), _password.c_str(), _database.c_str(), _port, nullptr, CLIENT_FOUND_ROWS);
    if (nullptr == &g_mysql)
    {
        printf("mysql connect fail\n");
        return ret = -1;
    }

    initState = true;
    return ret = 0;
}

int Mysql::Mysql_insert_user(std::string _userID, std::string _password, std::string _userName)
{
    int ret = -1;

    if (checkRepeat_UserName(_userName) > 0)
    {
        printf("mysql : User name already exists\n");
        return ret = -2;
    }
    if (checkRepeat_UserID(_userID) > 0)
    {
        printf("mysql : Account already exists\n");
        return ret = -3;
    }

    // TODO : 随机盐
    char salt[] = "12345678";
    std::stringstream strstream;

    // TODO : 密码加盐
    strstream << "INSERT INTO " << g_dataBase << ".User (user_id, user_password, user_name, salt) VALUES ('" << _userID << "', '" << _password << "', '" << _userName << "', '" << salt << "');";

    std::string commond = strstream.str();
    if (mysql_real_query(&g_mysql, commond.c_str(), strlen(commond.c_str())))
    {
        printf("mysql execute fail : %d \n", mysql_errno(&g_mysql));
        printf("commond : %s\n", commond.c_str());
        return ret = -1;
    }

    g_mysql_res = mysql_store_result(&g_mysql);
    if (nullptr == g_mysql_res)
    {
        printf("mysql store result fail : %d \n", mysql_errno(&g_mysql));
        return ret = -1;
    }

    mysql_free_result(g_mysql_res);
    return ret = 0;
}

int Mysql::Mysql_check_user(std::string _userID, std::string _password, std::string &_user_name)
{
    int ret = -1;

    // assembly command
    std::stringstream strstream;
    strstream << "SELECT * FROM " << g_dataBase << ".User WHERE user_id = '" << _userID << "';";
    std::string commond = strstream.str();

    // execute the command
    if (mysql_real_query(&g_mysql, commond.c_str(), strlen(commond.c_str())))
    {
        printf("mysql select fail : %d \n", mysql_errno(&g_mysql));
        printf("commond : %s\n", commond.c_str());
        return ret = -1;
    }

    // get the result
    g_mysql_res = mysql_store_result(&g_mysql);
    if (nullptr == g_mysql_res)
    {
        printf("mysql store result fail : %d \n", mysql_errno(&g_mysql));
        return ret = -1;
    }

    // get the result data
    MYSQL_ROW row;
    if (row = mysql_fetch_row(g_mysql_res))
    {
        // TODO : 密码加盐计算
        // row[3]
        std::string password = _password;

        if (_userID != row[0] || password != row[1])
        {
            return ret = -2;
        }
        _user_name = row[2];
    }
    else
    {
        return ret = -2;
    }

    mysql_free_result(g_mysql_res);
    return ret = 0;
}

Mysql::Mysql(/* args */)
{
}

Mysql::~Mysql()
{
    if (g_mysql_res != nullptr)
        mysql_free_result(g_mysql_res);

    mysql_close(&g_mysql);
}

void Mysql::del_object()
{
    if (s_mysqlSystem == nullptr)
    {
        delete s_mysqlSystem;
        s_mysqlSystem = nullptr;
    }
}
