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
#include <stdio.h>
#include <string.h>
#include <sstream>

class Mysql
{
public:
    // return ptr of the mysql system
    // The Log_init function needs to be called first
    static Mysql *instance();

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
    int Mysql_init(std::string _hostname, std::string _username, std::string _password, std::string _database, int _port);

    /**
     * @brief insert new user
     *
     * @param _userID
     * @param _password
     * @param _userName
     * @return int
     */
    int Mysql_insert_user(std::string _userID, std::string _password, std::string _userName);

    /**
     * @brief check user
     *
     * @param _userID
     * @param _password
     * @param _user_name
     * @return int 0: success;
     *  -1: server is error;
     *  -2: information error;
     */
    int Mysql_check_user(std::string _userID, std::string _password, std::string &_user_name);

private:
    Mysql(/* args */);

public:
    void del_object();
    ~Mysql();
};

#endif // __MYSQL_H__
