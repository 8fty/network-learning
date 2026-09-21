#include "tcpClient.hpp"
#include <memory>
//./tcpclient serverip serverport
using namespace std;
using namespace client;
static void Usage(string Proc)
{
    cout<<"\nUsage:\n\t"<<Proc<<"local _ip local_port"<<endl;
}
int main(int argc,char *argv[])
{
     if(argc!=3)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port=atoi(argv[2]);
    string serverip=argv[1];
    unique_ptr<TcpClient> tcl(new TcpClient(serverip,port));
    tcl->initClient();
    tcl->start();
    return 0;
}