/*
    Date: 2016/11/08
    Author: shpdqyfan
    profile: a simple thread
*/

#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

typedef enum
{
    WAITING = 0,
    RUNNING = 1
} THREAD_STATE;

const int DEFAULT_WAITING_TIME_MS = 3000;

class MyThread
{
public:
    MyThread();
    virtual ~MyThread();

    void start();
    void join();
    void setRunningState(const THREAD_STATE& state);
    bool getRunningState(int waiting = DEFAULT_WAITING_TIME_MS);

protected:
    virtual void run() = 0;

private:
    std::shared_ptr<std::thread> myThreadPtr;
    THREAD_STATE myRunningState;
    std::mutex myMutex;
    std::condition_variable myCondVar;
};

#endif

