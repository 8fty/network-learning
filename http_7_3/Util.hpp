#pragma once
#include <iostream>
#include <string>
#include <fstream>
class Util
{
public:
    static std::string getOneLine(std::string &buffer,const std::string &sep)
    {
        auto pos=buffer.find(sep);
        if(pos==std::string::npos)return "";
        std::string sub=buffer.substr(0,pos);
        buffer.erase(0,sub.size()+sep.size());
        return sub;
    }
   
     static bool readFile(const std::string &resource,char *buffer,int size)
    {
        std::ifstream in(resource,std::ios::binary);
        if(!in.is_open())return false;
        in.read(buffer,size);
        in.close();
        return true;
    }
};