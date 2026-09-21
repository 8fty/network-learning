#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Util.hpp"
const std::string sep = "\r\n";
const std::string default_root = "./wwwroot";
const std::string home_page = "index.html";
const std::string html_404 = "wwwroot/404.html";
class HttpRequest
{
public:
    HttpRequest() {}
    ~HttpRequest() {}
    void parse()
    {
        // 1.从inbuffer中拿到一行
        std::string line = Util::getOneLine(inbuffer, sep);
        if (line.empty())
            return;
        std::cout << "line:" << line << std::endl;
        // 2.进行分离
        std::stringstream ss(line);
        ss >> method >> url >> httpversion;
        //2.1 a/b/s?name=zhangsan&pwd=12345<-Get方法得到的
        //Post方法，本来就是分离的
        // 3.添加web默认路径
        path = default_root; // ./wwwroot
        path += url;         //./wwwroot/
        if (path[path.size() - 1] == '/')
            path += home_page;
        // 4.获取path对应的资源后缀
        //./wwwroot/image/1.image
        auto pos = path.rfind(".");
        if (pos == std::string::npos)
            suffix = ".html";
        else
            suffix = path.substr(pos);
        // 5.得到资源的大小
        struct stat st;
        int n = stat(path.c_str(), &st);
        if (n < 0)
        {
            perror("stat error");
            size = -1;
        }
        else
        {
            size = st.st_size;
        }
    }

public:
    std::string inbuffer;
    std::string method;
    std::string url;
    std::string httpversion;
    std::string path;
    std::string suffix;
    std::string parm;
    int size;
};

class HttpResponse
{
public:
    std::string outbuffer;
};