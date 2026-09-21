#include <memory>
#include "calServer.hpp"
using namespace std;
using namespace server;
// req：里面一定是我们的处理好的一个完整的请求对象
// resp：根据req，进行业务处理，填充resp，不用管理任何读取和写入，序列化和反序列化等任何细节
bool cal(const Request &req, Response &resp)
{
    switch (req.op)
    {
    case '+':
        resp.result = req.x + req.y;
        break;
    case '-':
        resp.result = req.x - req.y;
        break;
    case '*':
        resp.result = req.x * req.y;
        break;
    case '/':
    {
        if (req.y == 0)
            resp.exitcode = DIV_ZERO;
        else
            resp.result = req.x / req.y;
    }
    break;
    case '%':
    {
        if (req.y == 0)
            resp.exitcode = MOD_ZERO;
        else
            resp.result = req.x % req.y;
    }
    break;
    default:
    resp.exitcode=OP_ERROR;
    break;
    }
    return true;
}
static void Usage(string Proc)
{
    cout << "\nUsage:\n\t" << Proc << "local_port" << endl;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(USAGE_ERR);
    }
    uint16_t port = atoi(argv[1]);
    unique_ptr<CalServer> tsvr(new CalServer(port));
    tsvr->initServer();
    tsvr->start(cal);
    return 0;
}