#include "HttpServer.hpp"
#include <memory>
using namespace server;
void Usage(std::string proc)
{
    std::cerr << "Usage:\n\t" << proc << "port\r\n\r\n";
}
std::string suffixToDesc(const std::string &suffix)
{
    std::string ct = "Content-Type:";
    if (suffix == ".html")
    {
        ct += "text/html";
    }
    else if (suffix == ".jpg")
    {
        ct += "application/x-jpg";
    }
    ct += "\r\n";
    return ct;
}
bool Get(const HttpRequest &req, HttpResponse &resp)
{
    std::cout << "----------------------http start----------------------" << std::endl;
    std::cout << req.inbuffer << std::endl;
    std::cout << "method:" << req.method << std::endl;
    std::cout << "url:" << req.url << std::endl;
    std::cout << "httpversion:" << req.httpversion << std::endl;
    std::cout << "path:" << req.path << std::endl;
    std::cout << "suffix:" << req.suffix << std::endl;
    std::cout << "size:" << req.size <<"字节"<< std::endl;
    std::cout << "----------------------http end----------------------" << std::endl;
    std::string respline = "HTTP/1.1 200 OK\r\n";
    std::string respheader = suffixToDesc(req.suffix);
    if (req.size > 0)
    {
        // respheader += "Content-Length:";
        // respheader += std::to_string(req.size);
        // respheader+="\r\n";
    }

    std::string respblank = "\r\n";
    // std::string body = "<html lang=\"en\"><head><meta charset=\"UTF-8\"><title>for test</title><h1>hello world</h1></head><body><p> 政府未公开公示文件、医疗系统、交通、金融后台数据，爬取会触及《数据安全法》，危害公共安全。</p></body></html>";
    std::string body;
    body.resize(req.size+1);
    if (!Util::readFile(req.path,(char*)body.c_str(),req.size))
    {
        Util::readFile(html_404,(char*)body.c_str(),req.size);
    }
    respheader += "Content-Length:";
    respheader += std::to_string(body.size());
    resp.outbuffer += respline;
    resp.outbuffer += respheader;
    resp.outbuffer += respblank;
    resp.outbuffer += body;
    return true;
}
int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(0);
    }
    uint16_t port = atoi(argv[1]);
    std::unique_ptr<HttpServer> httpsvr(new HttpServer(Get, port));
    //httpsvr->registerCb("/",Get);//功能路由
    httpsvr->initServer();
    httpsvr->start();
    return 0;
}