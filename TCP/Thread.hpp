#pragma once
#include <iostream>
#include <pthread.h>
#include <string>
#include <cassert>
#include <functional>

namespace ThreadNs
{
    typedef std::function<void *(void *)> func_t;
    static const int num = 1024;
    class Thread
    {
    private:
        // 在类内创建线程，需要将方法设置为static，这样没有this指针
        // 所以我们将其设置为静态成员函数
        static void *start_routinue(void *args)
        {
            Thread *self = static_cast<Thread *>(args);
            return self->callback();
            // return func_();
        }

    public:
        Thread()
        {
            
            char namebuffer[num];
            snprintf(namebuffer, sizeof namebuffer, "thread-%d", threadNum++);
            name_ = namebuffer;
        }
        void start(func_t func, void *args=nullptr)
        {
            func_=func;
            args_=args;
            int n = pthread_create(&tid_, nullptr, start_routinue, this);
            assert(n == 0);
            (void)n;
        }
        void join()
        {
            int n = pthread_join(tid_, nullptr);
            assert(n == 0); // debug的方式发布的时候存在
            (void)n;
        }
        std::string threadname()
        {
            return name_;
        }
        void *callback()
        {
            return func_(args_);
        }

    private:
        std::string name_;
        func_t func_;
        void *args_; // 线程执行所需要的参数
        pthread_t tid_;
        static int threadNum;
    };
    int Thread::threadNum = 1;
}