#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "log.hpp"
namespace client
{
    enum
    {
        USAGE_ERR = 1,
        SOCK_ERR,
        CONNECT_ERR

    };
    class TcpClient
    {
    public:
        TcpClient(const std::string &serverip, const uint16_t &serverport)
            : _sock(-1), _serverip(serverip), _serverport(serverport)
        {
        }
        void initClient()
        {
            // 1.创建套接字
            _sock = socket(AF_INET, SOCK_STREAM, 0);
            if (_sock < 0)
            {
                logMessage(FATAL, "create socket error");
                exit(SOCK_ERR);
            }
            logMessage(NORMAL, "create socket sucess");
        }
        void start()
        {
            //开始链接
            struct sockaddr_in server;
            memset(&server,0,sizeof(server));
            server.sin_addr.s_addr=inet_addr(_serverip.c_str());
            server.sin_family=AF_INET;
            server.sin_port=htons(_serverport);
            if(connect(_sock,(struct sockaddr*)&server,sizeof(server))<0)
            {
                 logMessage(FATAL, "connect error");
                 exit(CONNECT_ERR);
            }
            logMessage(NORMAL, "connect sucess");
            std::string message;
            while(true)
            {
                std::cout<<"enter#";
                getline(std::cin,message);
                write(_sock,message.c_str(),message.size());
                char buffer[1024];
                buffer[0]=0;
                int n=read(_sock,buffer,sizeof(buffer)-1);
                if(n>0)
                {
                    buffer[n]=0;
                    std::cout<<"Server回显#"<<buffer<<std::endl;
                }
                else 
                {
                    break;;
                }
            }
        }
        ~TcpClient()
        {
            if(_sock>=0)
            close(_sock);
        }

    private:
        int _sock;
        std::string _serverip;
        uint16_t _serverport;
    };
}