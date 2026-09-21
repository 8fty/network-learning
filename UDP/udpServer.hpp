#pragma once
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <functional>
#include <stdlib.h>
#include <arpa/inet.h>
#include <strings.h>
#include <netinet/in.h>
namespace Server
{
    using namespace std;
    const static string defaultIp="0.0.0.0";
    const static int gnum=1024;
    enum{USAGE_ERR=1,SOCKET_ERR=2,BIND_ERR,OPEN_ERR};
    typedef function<void (int,string,uint16_t,string)> func_t;
    class udpServer
    {
        
    public:
        udpServer(const func_t&cb,const uint16_t &port, const string &ip=defaultIp)
        :_callback(cb)
        ,_port(port)
        ,_ip(ip)
        ,_sockfd(-1)
        {
        }
        void initServer()
        {
            //1.创建socket
            _sockfd=socket(AF_INET,SOCK_DGRAM,0);
            if(_sockfd==-1)
            {
                cerr<<"socket error:"<<errno<<":"<<strerror(errno)<<endl;
                exit(SOCKET_ERR);
            }
            //2.绑定port，ip
            struct sockaddr_in local;
            bzero(&local,sizeof(local));
            local.sin_family=AF_INET;
            local.sin_port=htons(_port);//转成大端
            //local.sin_addr.s_addr=inet_addr(_ip.c_str());//1.string->uint32_t 2.htonl() 
            local.sin_addr.s_addr=INADDR_ANY;//任意地址绑定
            int n=bind(_sockfd,(struct sockaddr*)&local,sizeof(local));
            if(n==-1)
            {
                cerr<<"bind error:"<<errno<<":"<<strerror(errno)<<endl;
                exit(BIND_ERR);
            }

        }
        void start()
        {
            char buffer[gnum];
            for(;;)
            {
                struct sockaddr_in peer;
                socklen_t len=sizeof(peer);
                ssize_t s=recvfrom(_sockfd,buffer,sizeof(buffer)-1,0,(struct sockaddr*)&peer,&len);
                //1.数据是什么 2.谁发的
                if(s>0)
                {
                   buffer[s]=0;
                   string clientip=inet_ntoa(peer.sin_addr); 
                   uint16_t clientport=ntohs(peer.sin_port);
                   string message=buffer;
                   cout<<clientip<<"["<<clientport<<"]#"<<message<<endl;
                   _callback(_sockfd,clientip,clientport,message);
                }

            }
        }
        ~udpServer()
        {
        }

    private:
        uint16_t _port;
        std::string _ip;
        int _sockfd;
        func_t _callback;
    };
}
