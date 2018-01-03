/*
    Date: 2016/11/08
    Author: shpdqyfan
    profile: a simple thread
*/

#include <chrono>
#include <functional>
#include <stdexcept>

#include "Thread/Thread.h"

MyThread::MyThread()
    : myThreadPtr(NULL)
    , myRunningState(WAITING)
{

}

MyThread::~MyThread()
{

}

void MyThread::start()
{
    if(NULL == myThreadPtr)
    {
        myThreadPtr.reset(new std::thread(std::bind(&MyThread::run, this)));
    }
    else
    {
        throw std::runtime_error("MyThread already start");
    }
}

void MyThread::join()
{
    if(NULL != myThreadPtr)
    {
        myThreadPtr->join();
    }
}

bool MyThread::getRunningState(int waiting)
{
    std::unique_lock<std::mutex> guard(myMutex);

    if(WAITING == myRunningState)
    {
        myCondVar.wait_for(guard, std::chrono::milliseconds(waiting));
    }

    if(WAITING == myRunningState)
    {
        return false;
    }
    else if(RUNNING == myRunningState)
    {
        return true;
    }
    else
    {
        throw std::runtime_error("MyThread unknown state");
        return false;
    }
}

void MyThread::setRunningState(const THREAD_STATE& state)
{
    std::unique_lock<std::mutex> guard(myMutex);

    myRunningState = state;
    myCondVar.notify_all();
}

