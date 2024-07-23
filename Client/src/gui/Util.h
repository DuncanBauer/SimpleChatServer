#pragma once

#include <string>

std::string stripWhitespace(char* data, int size)
{
    std::string ret;
    int j = 0;
    while (data[j] != '\0')
        ret += data[j++];
    return ret;
}