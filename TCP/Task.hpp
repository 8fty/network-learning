#pragma once
#include <iostream>
#include <functional>
#include <cstdio>
#include <string>
#include "log.hpp"
void serviceio(int sock)
{
    char buffer[1024];
    while (true)
    {
        size_t n = read(sock, buffer, sizeof(buffer) - 1);
        if (n > 0)
        {
            buffer[n] = 0;
            std::cout << "recv message:" << buffer << std::endl;
            std::string outbuffer = buffer;
            outbuffer += "server[echo]";
            write(sock, outbuffer.c_str(), outbuffer.size());
        }
        else if (n == 0)
        {
            // 代表client退出
            logMessage(NORMAL, "client quit,me too!");
            break;
        }
    }
    close(sock);
}
class Task
{

    // using func_t=std::function<int(int,int)>;
    typedef std::function<void(int)> func_t;

public:
    Task()
    {
    }
    Task(int sock, func_t func)
        : _sock(sock), _callback(func)
    {
    }
    void operator()()
    {
        _callback(_sock);
    }

public:
    int _sock;
    func_t _callback;
};
