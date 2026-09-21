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
#include "ThreadPool.hpp"
#include "Task.hpp"
#include "log.hpp"
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
    class TcpServer;
    class ThreadData
    {
    public:
        ThreadData(TcpServer *self,int sock)
        :_self(self)
        ,_sock(sock)
        {
        }

    public:
        TcpServer *_self;
        int _sock;
    };
    class TcpServer
    {
    public:
        TcpServer(const uint16_t &port = gport)
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
            logMessage(NORMAL, "create socket sucess:%d",_sock);
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
        //多进程多线程要用这个
        // void serviceio(int sock)
        // {
        //     char buffer[1024];
        //     while (true)
        //     {
        //         size_t n = read(sock, buffer, sizeof(buffer) - 1);
        //         if (n > 0)
        //         {
        //             buffer[n] = 0;
        //             std::cout << "recv message:" << buffer << std::endl;
        //             std::string outbuffer = buffer;
        //             outbuffer += "server[echo]";
        //             write(sock, outbuffer.c_str(), outbuffer.size());
        //         }
        //         else if (n == 0)
        //         {
        //             // 代表client退出
        //             logMessage(NORMAL, "client quit,me too!");
        //             break;
        //         }
        //     }
        // }
        //多线程版本
        // static void *threadRoutine(void *args)
        // {
        //     pthread_detach(pthread_self());
        //     ThreadData* td=static_cast<ThreadData*>(args);
        //     td->_self->serviceio(td->_sock);
        //     close(td->_sock);
        //     delete td;
            
        //     return nullptr;
        // }
        void start()
        {
             //线程池初始化
            ThreadPool<Task>::getInstance()->run();
            logMessage(NORMAL,"init thread success");
            //signal(SIGCHLD, SIG_IGN);
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
                logMessage(NORMAL, "accept success,get new sock:%d",sock);

                // 5.后面全是文件操作
                // v1
                // serviceio(sock);
                // close(sock);
                // v2 多进程
                // pid_t id = fork();
                // if (id == 0)
                // {
                //     close(_sock);
                //     if(fork()>0)exit(0);
                //     serviceio(sock);
                //     close(sock);
                //     exit(0);
                // }
                // //father
                // pid_t ret=waitpid(id,nullptr,0);
                // if(ret>0)
                // {
                //     std::cout <<"waitsuccess"<<ret << std::endl;
                // }
                // v2 多进程加入信号
                // pid_t id = fork();
                // if (id == 0)
                // {
                //     close(_sock);
                //     serviceio(sock);
                //     close(sock);
                //     exit(0);
                // }
                // else if(id>0)
                // {
                //     close(sock);
                // }
                // v3 多线程版
                // pthread_t tid;
                // ThreadData* td=new ThreadData(this,sock);
                // pthread_create(&tid, nullptr, threadRoutine, td);
                //v4 线程池版
                ThreadPool<Task>::getInstance()->push(Task(_sock,serviceio));
            }
        }

        ~TcpServer()
        {
        }

    private:
        int _sock;
        uint16_t _port;
    };
}