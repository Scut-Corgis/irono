#pragma once
#include <pthread.h>
#include <functional>
#include <string>
#include "noncopyable.h"
#include "CountDownLatch.h"
namespace irono {
    class Thread : noncopyable {
    public:
        typedef std::function<void()> ThreadFunc;
        explicit Thread(const ThreadFunc&, const std::string& name = std::string());
        ~Thread();
        void start();
        int join();
        bool started() const {return started_;}

    private:
        void setDefaultName();
        bool started_;
        bool joined_;
        pthread_t pthreadId_;
        pid_t tid_;
        ThreadFunc func_;
        std::string name_;
        CountDownLatch latch_;
    };
}
