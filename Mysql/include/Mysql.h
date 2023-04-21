/**
 * @file Mysql.h
 * @author  ()
 * @brief
 * @version 0.1
 * @date 2023-02-26
 *
 * @copyright Copyright (c) 2023
 *
 */
#ifndef __MYSQL_H__
#define __MYSQL_H__

/*______ I N C L U D E - F I L E S ___________________________________________*/

#include <map>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <vector>

enum UserState
{
    UserState_Unknown = 1,
    UserState_Online  = 2,
    UserState_Offline = 3
};

struct s_Friend_info
{
    std::string UserID   = "";
    std::string UserName = "";
};

class Mysql
{
public:
    /**
     * @brief initialization mysql server
     *
     * @param _hostname
     * @param _username
     * @param _password
     * @param _database
     * @param _port
     * @return int
     */
    int Mysql_init(std::string &_hostname, std::string &_username, std::string &_password, std::string &_database, int _port);

    /**
     * @brief insert new user
     *
     * @param _userID
     * @param _password
     * @param _userName
     * @return 0: success;
     * @return -1: server is error;
     * @return -2: User information exists;
     * @return -2: Account information exists;
     */
    int Mysql_insert_user(std::string &_userID, std::string &_password, std::string &_userName);

    /**
     * @brief check user
     *
     * @param _userID
     * @param _password
     * @param _user_name
     * @return 0: success;
     * @return -1: server is error;
     * @return -2: information error;
     */
    int Mysql_check_user(std::string &_userID, std::string &_password, std::string &_user_name);

    /**
     * @brief get friend list
     *
     * @param _userID
     * @param _friendList
     * @return int other: the size of friend list;
     * @return -1: server is error;
     * @return -2: information error;
     */
    int Mysql_Get_friendList(std::string &_userID, std::vector<s_Friend_info> &_friendList);

    /**
     * @brief set user state
     *
     * @param _userID
     * @param _state
     * @return 0: success;
     * @return -1: server is error;
     * @return -2: information error;
     */
    int Mysql_Set_userState(std::string &_userID, UserState _state);

private:
    Mysql(/* args */);

public:
    // return ptr of the mysql system
    // The Log_init function needs to be called first
    static Mysql *instance();

    void del_object();

    ~Mysql();
};

#endif // __MYSQL_H__
