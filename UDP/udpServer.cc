#include "udpServer.hpp"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
#include "onlineUser.hpp"
using namespace std;
using namespace Server;

const std::string dictTxt = "./dict.txt";
unordered_map<string, string> dict;

static void Usage(string proc)
{
    cout << "Usage:\n\t" << proc << "local_port\n\n";
}
// static bool cutString(string line, string *key, string *value)
// {
//     auto pos = line.find(':');
//     if (pos == string::npos)
//     {
//         return false;
//     }
//     *key = line.substr(0, pos);
//     *value = line.substr(pos + 1);
//     return true;
// }
// static void debugPrint()
// {
//     for (auto iter : dict)
//     {
//         cout << iter.first << "#" << iter.second << endl;
//     }
// }
// static void initDict()
// {
//     ifstream in(dictTxt);
//     if (!in.is_open())
//     {
//         cout << "文件打开失败" << endl;
//         return;
//     }
//     string line;
//     string key, value;
//     while (getline(in, line))
//     {
//         if (cutString(line, &key, &value))
//         {
//             dict.insert(make_pair(key, value));
//         }
//     }
//     in.close();
//     cout << "load dict success" << endl;
// }
// void handlerMessage(int sockfd, string clientip, uint16_t clientport, string message)
// {
//     string response_message;
//     auto iter = dict.find(message);
//     if (iter == dict.end())
//     {
//         response_message = "unkonwn";
//     }
//     response_message = iter->second;
//     // 开始返回
//     struct sockaddr_in client;
//     bzero(&client, sizeof(client));
//     client.sin_family = AF_INET;
//     client.sin_port = htons(clientport);
//     client.sin_addr.s_addr = inet_addr(clientip.c_str());
//     sendto(sockfd, response_message.c_str(), response_message.size(), 0, (struct sockaddr *)&client, sizeof(client));
// }
// void execCommand(int sockfd, string clientip, uint16_t clientport, string cmd)
// {
//     if(cmd.find("rm")!=string::npos)
//     {
//         return;
//     }
//     string response;
//     FILE *fp = popen(cmd.c_str(), "r");
//     if (fp == nullptr)
//         response = cmd + "exec failed";
//     char line[1024];
//     while (fgets(line, sizeof(line), fp))
//     {
//         response += line;
//     }
//     pclose(fp);
//     // 开始返回
//     struct sockaddr_in client;
//     bzero(&client, sizeof(client));
//     client.sin_family = AF_INET;
//     client.sin_port = htons(clientport);
//     client.sin_addr.s_addr = inet_addr(clientip.c_str());
//     sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr *)&client, sizeof(client));
// }
OnlineUser onlineusr;
void routeMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    if (message == "online")
    {
        onlineusr.addUser(clientip,clientport);
    }
    if (message == "offline")
    {
        onlineusr.delUser(clientip,clientport);
    }
    if(onlineusr.isOnline(clientip,clientport))
    {
        onlineusr.broadcast(sockfd,clientip, clientport,message);
    }
    else
    {
        //开始返回
        struct sockaddr_in client;
        bzero(&client, sizeof(client));
        client.sin_family = AF_INET;
        client.sin_port = htons(clientport);
        client.sin_addr.s_addr = inet_addr(clientip.c_str());
        string response="你没有登录,请运行online";
        sendto(sockfd, response.c_str(), response.size(), 0, (struct sockaddr *)&client, sizeof(client));
    }
}
// void reload(int signo)
// {
//     (void)signo;
//     initDict();
// }
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    // signal(2, reload);
    // initDict();
    // debugPrint();
    string ip = argv[1];
    // std::unique_ptr<udpServer> usr(new udpServer(handlerMessage, port));
    // std::unique_ptr<udpServer> usr(new udpServer(execCommand, port));
    std::unique_ptr<udpServer> usr(new udpServer(routeMessage, port));
    usr->initServer();
    usr->start();
    return 0;
}