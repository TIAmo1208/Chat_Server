#include "./include/Mysql.h"
#include <iostream>
// #include <mysql.h>

int main()
{
    std::string host     = "localhost";
    std::string name     = "TIAmo";
    std::string password = "mysqlPassword";
    std::string database = "DataBaseName";
    Mysql::instance()->Mysql_init(host, name, password, database, 3306);

    std::string userID       = "TestUser";
    std::string userPassword = "TestPassword";
    std::string userName     = "陆舟";
    Mysql::instance()->Mysql_insert_user(userID, userPassword, userName);

    std::string user_Name;
    Mysql::instance()->Mysql_check_user(userID, userPassword, user_Name);

    std::cout << user_Name << std::endl;

    Mysql::instance()->del_object();

    std::cout << "end" << std::endl;
    return 0;
}