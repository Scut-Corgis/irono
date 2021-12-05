#include "ThreadPool.h"
#include <assert.h>
#include <stdio.h>

using namespace irono;

ThreadPool::ThreadPool(const std::string& nameArg)
    : mutex_(),
      notEmpty_(mutex_),
      notFull_(mutex_),
      name_(nameArg),
      maxQueueSize_(0),
      running_(false)
{
}

ThreadPool::~ThreadPool() {
    if (running_) {
        stop();
    }
}

//start()要在用户预热服务器时执行，或者说在用户调用run()插入计算事件之前执行
void ThreadPool::start(int numThreads) {
    assert(threads_.empty());
    running_ = true;
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);
        threads_.emplace_back(new Thread( std::bind(&ThreadPool::runInThread, this), name_+id));
        threads_[i]->start();
    }
    if (numThreads == 0 && threadInitCallback_) {
        threadInitCallback_();
    }
}

void ThreadPool::stop() {
    {
    MutexLockGuard lock(mutex_);
    running_ = false;
    notEmpty_.notifyAll();
    notFull_.notifyAll();
    }
    for (auto& thr : threads_) {
        thr->join();
    }
}

size_t ThreadPool::queueSize() const {
    MutexLockGuard lock(mutex_);
    return queue_.size();
}

//用户插入计算事件的入口
void ThreadPool::run(Task task) {
    if (threads_.empty()) {
        task();
    }
    else {
        MutexLockGuard lock(mutex_);
        //如果处理不过来，主线程会直接睡眠等待队列中事件处理完再插入，所以I/O线程可能会阻塞在此处
        while (isFull() && running_) {
            notFull_.wait();
        }
        if (!running_) return;
        assert(!isFull());

        queue_.push_back(std::move(task));
        notEmpty_.notify();
    }
}

//线程池取任务的函数，一般没事件时会沉睡在此处
ThreadPool::Task ThreadPool::take() {
    MutexLockGuard lock(mutex_);
    // 常规手法,while
    while (queue_.empty() && running_) {
        notEmpty_.wait();
    }
    Task task;
    if (!queue_.empty()) {
        task = queue_.front();
        queue_.pop_front();
        if (maxQueueSize_ > 0) {
        notFull_.notify();
        }
    }
    return task;
}

//注意没有上锁，所以调用它的函数一定要确保上锁
bool ThreadPool::isFull() const {
    return maxQueueSize_ > 0 && queue_.size() >= maxQueueSize_;
}

void ThreadPool::runInThread() {
    if (threadInitCallback_) {
        threadInitCallback_();
    }
    while (running_) {
        Task task(take());
        if (task) {
            task();
        }
    }
}

