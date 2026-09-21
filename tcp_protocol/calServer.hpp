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
#include "log.hpp"
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
    // const Request &req 输入型
    // Response &rsp 输出型
    typedef std::function<bool(const Request &req, Response &rsp)> func_t;
    
    // 保证解耦
    void handlerEnter(int sock, func_t func)
    {
        string inbuffer;
        while (true)
        {
            // 1. 读取
            // 1.1 你怎么保证读到的消息是【一个】完整请求
            std::string req_text;
            // 1.2->得到一个完整的请求"text_len"\r\n"x op y"\r\n
            if (!recvPackage(sock, inbuffer, &req_text))
                return;
            std::cout<<"带报头的请求：\n"<<req_text<<std::endl;
            std::string req_str;
            if (!deLength(req_text, &req_str))
                return;
            std::cout<<"去掉报头的正文：\n"<<req_text<<std::endl;
            // 2. 对请求Request，反序列化
            // 2.1 得到一个结构化的请求对象
            Request req;
            if (!req.Deserialize(req_str))
                return;
            // 3.1 得到一个结构化的响应
            Response resp;
            func(req, resp);
            // 4. 对响应Response，进行序列化
            // 4.1 得到了一个"字符串"
            std::string resp_str;
            resp.serialize(&resp_str);
            std::cout<<"计算完成，序列化响应：\n"<<resp_str<<std::endl;
            // 5. 然后我们在发送响应
            // 5.1构建一个完整的报文
            std::string send_string = enLength(resp_str);
            std::cout<<"构建完成完整的相应：\n"<<send_string<<std::endl;
            send(sock, send_string.c_str(), send_string.size(), 0); // 有bug
        }
    }
    class CalServer
    {
    public:
        CalServer(const uint16_t &port = gport)
            : _sock(-1), _port(port)
        {
        }
        void initServer()
        {
            // 1.创建套接字
            _sock = socket(AF_INET, SOCK_STREAM, 0);
            if (_sock < 0)
            {
                logMessage(FATAL, "create socket error");
                exit(SOCK_ERR);
            }
            logMessage(NORMAL, "create socket sucess:%d", _sock);
            // 2.绑定网络信息
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(_port);
            local.sin_addr.s_addr = INADDR_ANY;
            if (bind(_sock, (struct sockaddr *)&local, sizeof(local)) < 0)
            {
                logMessage(FATAL, "bind socket error");
                exit(BIND_ERR);
            }
            logMessage(NORMAL, "bind socket sucess");
            // 3.设置socket为监听状态
            if (listen(_sock, gbacklog) < 0)
            {
                logMessage(FATAL, "listen socket error");
                exit(LISTEN_ERR);
            }
            logMessage(NORMAL, "listen socket sucess");
        }
        void start(func_t func)
        {
            for (;;)
            {
                // 获取链接
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int sock = accept(_sock, (struct sockaddr *)&peer, &len);
                if (sock < 0)
                {
                    logMessage(ERROR, "accept error next");
                    continue;
                }
                logMessage(NORMAL, "accept success,get new sock:%d", sock);
                // v2 多进程
                pid_t id = fork();
                if (id == 0)
                {
                    close(_sock);
                    if (fork() > 0)
                        exit(0);
                    // serviceio(sock);
                    handlerEnter(sock, func);
                    close(sock);
                    exit(0);
                }
                // father
                pid_t ret = waitpid(id, nullptr, 0);
                if (ret > 0)
                {
                    logMessage(NORMAL, "wait child sucess");
                }
            }
        }

        ~CalServer()
        {
        }

    private:
        int _sock;
        uint16_t _port;
    };
}