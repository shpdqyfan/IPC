/*
    Date: 2017/12/18
    Author: shpdqyfan
    profile: Buffer supports to asynchronously and sequentially 
    process T-typed elements in a dedicated thread in FIFO order.
*/

#ifndef BUFFER_H
#define BUFFER_H

#include <iostream>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "Thread/Thread.h"

template<typename T>
class Buffer : public MyThread
{
public:
    typedef std::function<void(T)> Callback;
    
    explicit Buffer(const Callback& cb);
    virtual ~Buffer();
    
    void startBuffering();
    void stopBuffering();
    void pushToBuffer(const T& element);
    void clearBuffer();
    bool empty() const;
    size_t size() const;

protected:
    void run();
    
private:
    std::queue<T> myBufferQueue;
    std::recursive_mutex myMutex;
    std::condition_variable_any myCondVar;
    Callback myCb;
    bool running;
};

template<typename T>
Buffer<T>::Buffer(const Callback& cb)
    : myCb(cb)
    , running(false)        
{
    std::cout<<"Buffer, construct"<<std::endl;
}

template<typename T>
Buffer<T>::~Buffer()
{
    std::cout<<"Buffer, deconstruct"<<std::endl;
}

template<typename T>
void Buffer<T>::startBuffering()
{
    std::cout<<"Buffer, startBuffering"<<std::endl;
    
    start();
}

template<typename T>
void Buffer<T>::stopBuffering()
{
    std::cout<<"Buffer, stopBuffering"<<std::endl;
    
    running = false;
    myCondVar.notify_one();
    join();
}

template<typename T>
void Buffer<T>::pushToBuffer(const T& element)
{
    std::unique_lock<std::recursive_mutex> guard(myMutex);
    
    myBufferQueue.push(element);
    myCondVar.notify_one();
}

template<typename T>
void Buffer<T>::clearBuffer()
{
    std::queue<T> emptyQueue;

    std::unique_lock<std::recursive_mutex> guard(myMutex);
    std::swap(myBufferQueue, emptyQueue);
}

template<typename T>
bool Buffer<T>::empty() const
{
    std::unique_lock<std::recursive_mutex> guard(myMutex);
    
    return myBufferQueue.empty();
}

template<typename T>
size_t Buffer<T>::size() const
{
    std::unique_lock<std::recursive_mutex> guard(myMutex);
    
    return myBufferQueue.size();
}

template<typename T>
void Buffer<T>::run()
{
    std::cout<<"Buffer, running"<<std::endl;
    
    running = true;

    while(running)
    {
        std::unique_lock<std::recursive_mutex> guard(myMutex);
        
        if(myBufferQueue.empty())
        {
            myCondVar.wait(guard);
            if(!running)
            {
                return;
            }
        }

        T element = myBufferQueue.front();
        myBufferQueue.pop();
        guard.unlock();
        myCb(element);
    }
}

#endif

