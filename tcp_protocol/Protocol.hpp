#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <jsoncpp/json/json.h>
using namespace std;
#define SEP " "
#define SEP_LEN strlen(SEP)
#define LINE_SEP "\r\n"
#define LINE_SEP_LEN strlen(LINE_SEP)
enum
{
    OK = 0,
    DIV_ZERO,
    MOD_ZERO,
    OP_ERROR
};
// x op y->"text_len"\r\n"x op y"\r\n
const std::string enLength(const std::string &text)
{
    std::string send_string = std::to_string(text.size());
    send_string += LINE_SEP;
    send_string += text;
    send_string += LINE_SEP;
    return send_string;
}
bool deLength(const std::string &package, std::string *text)
{
    auto pos = package.find(LINE_SEP);
    if (pos == string::npos)
    {
        return false;
    }
    int text_len = std::stoi(package.substr(0, pos));
    *text = package.substr(pos + LINE_SEP_LEN, text_len);
    return true;
}
class Request
{
public:
    Request() : x(0), y(0), op(0)
    {
    }
    Request(int x_, int y_, char op_)
        : x(x_), y(y_), op(op_)
    {
    }
    bool serialize(std::string *out)
    {
#ifdef MYSELF
        // 结构化 -> "x op y"
        std::string x_string = std::to_string(x);
        std::string y_string = std::to_string(y);
        *out = x_string;
        *out += SEP;
        *out += op;
        *out += SEP;
        *out += y_string;
#else
        Json::Value root;
        root["first"]=x;
        root["second"]=y;
        root["oper"]=op;
        Json::FastWriter writer;
        *out=writer.write(root);

#endif
        return true;
    }
    bool Deserialize(const std::string &in)
    {
#ifdef MYSELF
        // "x op y" -> 结构化
        auto left = in.find(SEP);
        auto right = in.rfind(SEP);
        if (left == std::string::npos || right == std::string::npos)
            return false;
        if (left == right)
            return false;
        if (right - (left + SEP_LEN) != 1)
            return false;

        std::string x_string = in.substr(0, left); // [0, 2) [start, end), start, end - start
        std::string y_string = in.substr(right + SEP_LEN);
        x = std::stoi(x_string);
        y = std::stoi(y_string);
        op = in[left + SEP_LEN];
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in,root);
        x=root["first"].asInt();
        y=root["second"].asInt();
        op=root["oper"].asInt();
#endif
        return true;
    }

public:
    int x;
    int y;
    char op;
};

class Response
{
public:
    Response()
        : exitcode(0), result(0)
    {
    }
    Response(int exitcode_, int result_)
        : exitcode(exitcode_), result(result_)
    {
    }
    bool serialize(std::string *out)
    {
#ifdef MYSELF
        std::string ec_string = to_string(exitcode);
        std::string rs_string = to_string(result);
        *out = ec_string;
        *out += SEP;
        *out += rs_string;
#else
        Json::Value root;
        root["exitcode"]=exitcode;
        root["result"]=result;
        Json::FastWriter writer;
        *out=writer.write(root);
#endif
        return true;
    }
    bool Deserialize(const std::string &in)
    {
#ifdef MYSELF
        // exitcode result
        auto mid = in.find(SEP);
        if (mid == string::npos)
        {
            return false;
        }
        std::string ec_string = in.substr(0, mid);
        std::string rs_string = in.substr(mid + SEP_LEN);
        if (ec_string.empty() || rs_string.empty())
        {
            return false;
        }
        exitcode = std::stoi(ec_string);
        result = std::stoi(rs_string);
#else
        Json::Value root;
        Json::Reader reader;
        reader.parse(in,root);
        exitcode=root["exitcode"].asInt();
        result=root["result"].asInt();
#endif
        return true;
    }

public:
    int exitcode; // 0计算成功 非0计算失败
    int result;
};
//"text_len"\r\n"x op y"\r\n"text_len"\r\n"x op y"\r\n
bool recvPackage(int sock, std::string &inbuffer, std::string *text)
{
    char buffer[1024];
    while (true)
    {
        ssize_t n = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (n > 0)
        {
            buffer[n] = 0;
            inbuffer += buffer;
            auto pos = inbuffer.find(LINE_SEP);
            std::string text_len_string = inbuffer.substr(0, pos);
            int text_len = std::stoi(text_len_string);
            int total_len = text_len_string.size() + 2 * LINE_SEP_LEN + text_len;
            if (inbuffer.size() < total_len)
                continue;
            std::cout << "处理前#inbuffer:" << inbuffer << std::endl;
            *text = inbuffer.substr(0, total_len);
            inbuffer.erase(0, total_len);
            std::cout << "处理后#inbuffer:" << inbuffer << std::endl;
            break;
        }
        else
        {
            return false;
        }
    }
    return true;
}
