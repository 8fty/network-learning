#pragma once
#include <iostream>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ctype.h>
#include "log.hpp"
#include "Protocol.hpp"
namespace client
{
    enum
    {
        USAGE_ERR = 1,
        SOCK_ERR,
        CONNECT_ERR

    };
    class CalClient
    {
    public:
        CalClient(const std::string &serverip, const uint16_t &serverport)
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
            // 开始链接
            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_addr.s_addr = inet_addr(_serverip.c_str());
            server.sin_family = AF_INET;
            server.sin_port = htons(_serverport);
            if (connect(_sock, (struct sockaddr *)&server, sizeof(server)) < 0)
            {
                logMessage(FATAL, "connect error");
                exit(CONNECT_ERR);
            }
            logMessage(NORMAL, "connect sucess");
            std::string line;
            std::string inbuffer;
            while (true)
            {
                std::cout << "mycal>>>";
                getline(std::cin, line);
                Request req = ParseLine(line); // 1+1
                std::string content;
                req.serialize(&content);
                std::string sendString = enLength(content);
                send(_sock, sendString.c_str(), sendString.size(), 0); // bug?

                std::string package, text;
                if (!recvPackage(_sock, inbuffer, &package))
                    continue;
                if (!deLength(package, &text))
                    continue;
                Response rsp;
                rsp.Deserialize(text);
                std::cout << "exitcode:" << rsp.exitcode << std::endl;
                std::cout << "result:" << rsp.result << std::endl;
            }
        }
        Request ParseLine(const std::string &line)
        {
            int status = 0;
            int i = 0;
            std::string left, right;
            int cnt = line.size();
            char op;
            while (i < cnt)
            {
                switch (status)
                {
                case 0:
                {
                    if (!isdigit(line[i]))
                    {
                        op = line[i];
                        status = 1;
                    }
                    else
                    {
                        left.push_back(line[i++]);
                    }
                }
                break;
                case 1:
                    i++;
                    status = 2;
                    break;
                case 2:
                {
                    right.push_back(line[i++]);
                }
                break;
                }
            }
            std::cout<<std::stoi(left)<<" "<<std::stoi(right)<<" "<<op<<std::endl;
            return Request(std::stoi(left),std::stoi(right),op);
        }
        ~CalClient()
        {
            if (_sock >= 0)
                close(_sock);
        }

    private:
        int _sock;
        std::string _serverip;
        uint16_t _serverport;
    };
}