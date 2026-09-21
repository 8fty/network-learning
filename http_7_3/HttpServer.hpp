#pragma once
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include <memory>
#include <functional>
#include <unordered_map>
#include "Protocol.hpp"
namespace server
{
    
    enum
    {
        USAGE_ERR = 1,
        SOCK_ERR,
        BIND_ERR,
        LISTEN_ERR
    };

    static const uint16_t gport = 8080;
    static const int gbacklog = 5;
    using func_t = std::function<bool(const HttpRequest &, HttpResponse &)>;
    class HttpServer
    {
    public:
        HttpServer(func_t func, const uint16_t &port = gport)
            : _func(func), _sock(-1), _port(port)
        {
        }
        void initServer()
        {
            // 1.创建套接字
            _sock = socket(AF_INET, SOCK_STREAM, 0);
            if (_sock < 0)
            {
                exit(SOCK_ERR);
            }
            // 2.绑定网络信息
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = INADDR_ANY;
            if (bind(_sock, (struct sockaddr *)&local, sizeof(local)) < 0)
            {
                exit(BIND_ERR);
            }
            // 3.设置socket为监听状态
            if (listen(_sock, gbacklog) < 0)
            {

                exit(LISTEN_ERR);
            }
        }
        // void registerCb(std::string servicename,func_t cb)
        // {
        //     funcs.insert(std::make_pair(servicename,cb));
        // }
        void HandlerHttp(int sock)
        {
            // 1.读到完整的http请求
            // 2.反序列化
            // 3.httprequest,httpresponse,_func(req,rsp)
            // 4.resp序列化
            // 5.send
            char buffer[4096];
            HttpRequest req;
            HttpResponse resp;
            size_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (n > 0)
            {
                buffer[n] = 0;
                req.inbuffer=buffer;
                req.parse();
                //funcs[req.path](req,resp);
                _func(req, resp);
                send(sock,resp.outbuffer.c_str(),resp.outbuffer.size(),0);
            }
        }
        void start()
        {
            for (;;)
            {
                // 获取链接
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int sock = accept(_sock, (struct sockaddr *)&peer, &len);
                if (sock < 0)
                {
                    continue;
                }

                // v2 多进程
                pid_t id = fork();
                if (id == 0)
                {
                    close(_sock);
                    if (fork() > 0)
                        exit(0);
                    // serviceio(sock);
                    HandlerHttp(sock);
                    close(sock);
                    exit(0);
                }
                // father
                pid_t ret = waitpid(id, nullptr, 0);
                if (ret > 0)
                {
                }
            }
        }

        ~HttpServer()
        {
        }

    private:
        int _sock;
        uint16_t _port;
        func_t _func;
        std::unordered_map<std::string,func_t> funcs;
    };
}