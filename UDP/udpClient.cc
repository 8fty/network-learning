#include "udpClient.hpp"
#include <memory>

// ./udpClient server_ip server_port
using namespace std;
using namespace Client;
static void Usage(string proc)
{
    cout<<"Usage:\n\t"<<proc<<"server_ip server_port\n\n";
}
int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        Usage(argv[0]);
        exit(0);
    }
    string serverip=argv[1];
    uint16_t serverport=atoi(argv[2]);
    unique_ptr<udpClient> ucli(new udpClient(serverip,serverport));
    ucli->initClient();
    ucli->run();
    return 0;
}