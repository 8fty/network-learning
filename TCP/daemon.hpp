#pragma once
#include <unistd.h>
#include <signal.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define DEV "/dev/null"
void daemonSelf()
{
    //1.让调用进程忽略掉异常的信号
    signal(SIGPIPE,SIG_IGN);

    //2.让自己不是组长在调用setsid
    if(fork()>0)exit(0);
    setsid();
    //3. 守护进程是脱离终端的，关闭或者重定向以前进程默认打开的文件
    int fd=open(DEV,O_RDWR);
    if(fd>=0)
    {
        dup2(fd,0);
        dup2(fd,1);
        dup2(fd,2);
        close(fd);
    }
    //4.可选：进程执行路径发生更改
}
