#include "tcpServer.hpp"
#include "daemon.hpp"
#include <memory>
using namespace std;
using namespace server;
static void Usage(string Proc)
{
    cout<<"\nUsage:\n\t"<<Proc<<"local_port"<<endl;
}
int main(int argc,char *argv[])
{
    if(argc!=2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port=atoi(argv[1]);
    unique_ptr<TcpServer> tsvr(new TcpServer(port));
    tsvr->initServer();
    daemonSelf();
    tsvr->start();
    return 0;
}