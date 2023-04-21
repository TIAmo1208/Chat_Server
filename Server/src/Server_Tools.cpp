
/*______ I N C L U D E - F I L E S ___________________________________________*/

#include "Server_Tools.hpp"

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
