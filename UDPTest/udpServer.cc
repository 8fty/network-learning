#include "udpServer.hpp"
#include <memory>
#include <unordered_map>
#include <fstream>
#include <signal.h>
using namespace std;
using namespace Server;

const std::string dictTxt = "./dict.txt";
unordered_map<string, string> dict;

static void Usage(string proc)
{
    cout << "Usage:\n\t" << proc << "local_port\n\n";
}

void handlerMessage(int sockfd, string clientip, uint16_t clientport, string message)
{
    string response_message=message;
    response_message+="[server echo]";
    // 开始返回
    struct sockaddr_in client;
    bzero(&client, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(clientport);
    client.sin_addr.s_addr = inet_addr(clientip.c_str());
    sendto(sockfd, response_message.c_str(), response_message.size(), 0, (struct sockaddr *)&client, sizeof(client));
}

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
     std::unique_ptr<udpServer> usr(new udpServer(handlerMessage, port));
    // std::unique_ptr<udpServer> usr(new udpServer(execCommand, port));
    usr->initServer();
    usr->start();
    return 0;
}