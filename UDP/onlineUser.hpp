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
#include <unordered_map>
using namespace std;
class User
{
public:
    User(const string &ip, const uint16_t &port)
        : _ip(ip), _port(port)
    {
    }
    ~User()
    {
    }
    string getIp()
    {
        return _ip;
    }
    uint16_t getPort()
    {
        return _port;
    }

private:
    std::string _ip;
    uint16_t _port;
};
class OnlineUser
{
public:
    OnlineUser() {}
    ~OnlineUser() {}
    void addUser(const string &ip, const uint16_t &port)
    {
        string id = ip + '-' + to_string(port);
        users.insert(make_pair(id, User(ip, port)));
    }
    void delUser(const string &ip, const uint16_t &port)
    {
        string id = ip + '-' + to_string(port);
        users.erase(id);
    }
    bool isOnline(const string &ip, const uint16_t &port)
    {
        string id = ip + '-' + to_string(port);
        return users.find(id) == users.end() ? false : true;
    }
    void broadcast(int sockfd,const string &ip, const uint16_t &port,const string &message)
    {
        for (auto &user : users)
        {
            // 开始广播
            struct sockaddr_in client;
            bzero(&client, sizeof(client));
            client.sin_family = AF_INET;
            client.sin_port = htons(user.second.getPort());
            client.sin_addr.s_addr = inet_addr(user.second.getIp().c_str());
            string s=ip+'-'+to_string(port)+"#";
            s+=message;
            sendto(sockfd,s.c_str(),s.size(), 0, (struct sockaddr *)&client, sizeof(client));
        }
    }

private:
    unordered_map<string, User> users;
};