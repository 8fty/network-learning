#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/in.h>
#include <pthread.h>
namespace Client
{
    using namespace std;
    class udpClient
    {
    public:
        udpClient(string ip, uint16_t port)
            : _sockfd(-1), _ip(ip), _port(port), _quit(false)
        {
        }
        void initClient()
        {
            // 创建socket
            _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (_sockfd == -1)
            {
                cerr << "socket error:" << errno << ":" << strerror(errno) << endl;
                exit(1);
            }
        }
        static void *readMessage(void *args)
        {
            pthread_detach(pthread_self());
            int sockfd = *(static_cast<int *>(args));
            while (true)
            {
                char buffer[1024];
                struct sockaddr_in temp;
                socklen_t temp_len = sizeof(temp);
                size_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (struct sockaddr *)&temp, &temp_len);
                if (n >= 0)
                {
                    buffer[n] = 0;
                }
                cout << buffer << endl;
            }
            return nullptr;
        }
        void run()
        {
            pthread_create(&_reader, nullptr, readMessage, (void *)&_sockfd);

            struct sockaddr_in server;
            memset(&server, 0, sizeof(server));
            server.sin_family = AF_INET;
            server.sin_port = htons(_port);
            server.sin_addr.s_addr = inet_addr(_ip.c_str()); // 将字符串转换成32位ip
            string message;
            char cmdline[1024];
            while (!_quit)
            {
                //cerr << "#";
                // cin>>message;
                fprintf(stderr,"Enter# ");
                fflush(stderr);
                fgets(cmdline, sizeof(cmdline), stdin);
                cmdline[strlen(cmdline)-1]=0;
                message = cmdline;
                // getline(cin,message);
                sendto(_sockfd, message.c_str(), message.size(), 0, (sockaddr *)&server, sizeof(server));
            }
        }
        ~udpClient()
        {
        }

    private:
        int _sockfd;
        uint16_t _port;
        std::string _ip;
        bool _quit;
        pthread_t _reader;
        pthread_t _writer;
    };
}