#include "Timestamp.h"
#include "Logging.h"
#include "LogStream.h"
#include "noncopyable.h"
#include "Thread.h"
#include <stdio.h>
#include "CurrentThread.h"
using namespace irono;
using namespace CurrentThread;

int k = 0;
MutexLock mutex;

// void ThreadFunc() {
//     printf("线程名字:%s, 线程pid:%s \n", t_threadName, t_tidString);
//     int h = 500000;
//     while (h--) {
//         MutexLockGuard lock(mutex);
//         k++;
//     }
// }

int main() {
    LOG_DEBUG<<"progame has been started";
    int k = 50;
    // Thread mythread1(ThreadFunc, "testThread1");
    // Thread mythread2(ThreadFunc, "testThread2");
    // mythread1.start();
    // mythread2.start();
    // int hh = 500000;
    // while (hh--) {
    //     MutexLockGuard lock(mutex);
    //     k++;
    // }

    // mythread1.join();
    // mythread2.join();
    // printf("k:   %d \n",k);
    while (k--) {
        LOG_DEBUG<<"WAIT FOR 3 SECONDS";
        sleep(3);
    };
    LOG_DEBUG<<"sub thread has dead";
}
