#pragma once
#include <iostream>
#include <cstdarg>
#include <ctime>
#include <unistd.h>
#define DEBUG 0
#define NORMAL 1
#define WARNING 2
#define ERROR 3
#define FATAL 4
#define LOG_NORMAL "normal.txt"
#define LOG_ERR "err.txt"

const char *to_levelstr(int level)
{
    switch (level)
    {
    case DEBUG:
        return "DEBUG";
    case NORMAL:
        return "NORMAL";
    case WARNING:
        return "WARNING";
    case ERROR:
        return "ERROR";
    case FATAL:
        return "FATAL";
    default:
        return nullptr;
    }
}
void logMessage(int level, const char *format, ...)
{
//[日志等级] [时间] [pid] [messge]
#define NUM 1024

    char logprefix[NUM];
    snprintf(logprefix, sizeof(logprefix), "[%s][%ld][pid:%d]",to_levelstr(level),
    (long int)time(nullptr),getpid());

    char logcontent[NUM];
    va_list arg;
    va_start(arg, format);

    vsnprintf(logcontent, sizeof(logcontent), format, arg);

    //std::cout << logprefix << logcontent << std::endl;
    FILE *log=fopen(LOG_NORMAL,"a");
    FILE *err=fopen(LOG_ERR,"a");
    if(log!=nullptr&&err!=nullptr)
    {
        if(level==DEBUG||level==NORMAL||level==WARNING)
        {
            fprintf(log,"%s%s\n",logprefix,logcontent);
        }
         if(level==ERROR||level==FATAL)
        {
            fprintf(err,"%s%s\n",logprefix,logcontent);
        }
        fclose(log);
        fclose(err);

    }
    va_end(arg);
}
