/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Process which will be used for sending/receiving msg to/from 
    main process "ControlProcess" in Ipc project.
*/

#include <iostream>
#include <thread>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <signal.h>

#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com;
using namespace Com::IpcMsg;

static void processIpcMsgObjCbInDcProc(IpcMsgObj obj);

static bool recvThreadRunning = false;
static int udsIpcSfd = -1;
static const char udsDcPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock_dc.socket";
static const char udsMainPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock_main.socket";
static sem_t semAllBinsStarted;
static sem_t semProcExit;
static Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObjCbInDcProc);

static void waitForAllbinsStart()
{
    std::cout<<"PID="<<getpid()<<", "<<"dc proc waitForAllbinsStart..."<<std::endl;
    
    if(0 != sem_wait(&semAllBinsStarted))
    {
        std::cout<<"PID="<<getpid()<<", "<<"waitForAllbinsStart, sem_wait error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }

    std::cout<<"PID="<<getpid()<<", "<<"dc proc waitForAllbinsStart, notified"<<std::endl;
}

static void waitForProcExit()
{
    std::cout<<"PID="<<getpid()<<", "<<"dc proc waitForProcExit..."<<std::endl;
    
    if(0 != sem_wait(&semProcExit))
    {
        std::cout<<"PID="<<getpid()<<", "<<"waitForProcExit, sem_wait error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }

    std::cout<<"PID="<<getpid()<<", "<<"dc proc waitForProcExit, notified"<<std::endl;
}

static void handleTermSig(int signal)
{
    std::cout<<"PID="<<getpid()<<", "<<"handleTermSig, signal="<<signal<<std::endl;
    
    if(SIGINT != signal)
    {
        return;
    }

    //step.23 receive SIGINT (ctrl+c), stop dc process
    recvThreadRunning = false;
    IpcMsg::closeSock(udsIpcSfd, 2);//SHUT_RDWR
    if(0 != sem_post(&semProcExit))
    {
        std::cout<<"PID="<<getpid()<<", "<<"handleTermSig, semProcExit posted error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }
    else
    {
        std::cout<<"PID="<<getpid()<<", "<<"handleTermSig, semProcExit posted"<<std::endl;
    }
}

void processIpcMsgObjCbInDcProc(IpcMsgObj obj)
{
    //std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc"<<std::endl;
    //obj.dump();

    if(IPC_MSG_ID_DC != obj.recvId)
    {
        std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, recvId error"<<std::endl;
        return;
    }

    std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, received:"<<std::endl;
    std::cout<<"PID="<<getpid()<<", "<<"                      send id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.sendId)<<std::endl;
    std::cout<<"PID="<<getpid()<<", "<<"                      recv id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.recvId)<<std::endl;
    std::cout<<"PID="<<getpid()<<", "<<"                      msgtype: "<<IpcMsg::ipcMsgTypeToStr(obj.type)<<std::endl;
    std::cout<<"PID="<<getpid()<<", "<<"                      content: "<<obj.data<<std::endl;

    //step.18 
    if(IPC_MSG_TYPE_BIN_ALL_SUCC == obj.type)
    {
        if(0 != sem_post(&semAllBinsStarted))
        {
            std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, semAllBinsStarted posted error, errno="
                <<errno<<", "<<strerror(errno)<<std::endl;
        }
        else
        {
            std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, semAllBinsStarted posted"<<std::endl;
        }
    }
    else if(IPC_MSG_TYPE_DATA == obj.type)
    { 
    }
   /* (not needed)
    //step.24 receive proc stop msg, post semProcExit, release all resource
    else if(IPC_MSG_TYPE_BIN_STOP == obj.type)
    {
        if(0 != sem_post(&semProcExit))
        {
            std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, semProcExit posted error, errno="
                <<errno<<", "<<strerror(errno)<<std::endl;
        }
        else
        {
            std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInDcProc, semProcExit posted"<<std::endl;
        }

        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));

        ipcObj.sendId = IPC_MSG_ID_DC;
        ipcObj.recvId = IPC_MSG_ID_MAIN;
        ipcObj.type = IPC_MSG_TYPE_BIN_STOP_SUCC;
        ipcObj.requestId = 0;

        std::string dataStr = "DataCenterProcess already stop";
        memcpy(ipcObj.data, dataStr.c_str(), dataStr.size());
        IpcMsg::sendUdsMsg(udsIpcSfd, udsMainPath, (void*)(&ipcObj), sizeof(ipcObj));
    }
    */
}

static void recvUdsMsgCbInDcProc(int sfd)
{
    recvThreadRunning = true;
    while(recvThreadRunning)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));
    
        if(0 > IpcMsg::recvUdsMsg(sfd, (void*)(&ipcObj), sizeof(ipcObj)))
        {
            std::cout<<"PID="<<getpid()<<", "<<"recvUdsMsgCbInDcProc, receive error"<<std::endl;
        }
        else
        {
            std::cout<<"PID="<<getpid()<<", "<<"recvUdsMsgCbInDcProc, receiving, sfd="<<sfd<<std::endl;

            myIpcMsgBuffer.pushToBuffer(ipcObj);
        }
    }
}

static void workingCbInDcProc(int sfd)
{
    for(unsigned i = 1;i <= 2;i++)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));

        ipcObj.sendId = IPC_MSG_ID_DC;
        ipcObj.recvId = IPC_MSG_ID_MAIN;
        ipcObj.type = IPC_MSG_TYPE_DATA;
        ipcObj.requestId = i;

        std::string dataStr = "Data from DataCenterProcess: " + std::to_string(i);
        memcpy(ipcObj.data, dataStr.c_str(), dataStr.size());
        IpcMsg::sendUdsMsg(sfd, udsMainPath, (void*)(&ipcObj), sizeof(ipcObj));
    }    
}

int main()
{
    std::cout<<"PID="<<getpid()<<", "<<"Dc process start, triggered by main process, parent pid="<<getppid()<<std::endl;

    signal(SIGINT, handleTermSig);

    //step.9 delete socket file
    unlink(udsDcPath);

    //step.10 create UNIX domain IPC 
    udsIpcSfd = IpcMsg::createUdsIpc(udsDcPath);
    if(0 > udsIpcSfd)
    {
        return 1;
    }

    //init semaphore
    if(0 != sem_init(&semAllBinsStarted, 0, 0))
    {
        std::cout<<"PID="<<getpid()<<", "<<"semAllBinsStarted init error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }

    if(0 != sem_init(&semProcExit, 0, 0))
    {
        std::cout<<"PID="<<getpid()<<", "<<"semProcExit init error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }

    //step.11 start IPC msg receiving thread
    std::thread udsIpcRecvThread(recvUdsMsgCbInDcProc, udsIpcSfd);

    //step.12 start Buffering which is used to buffer the msg from other process
    myIpcMsgBuffer.startBuffering();

    //step.13 ensure that receiving thread and Buffering thread have finished starting
    sleep(1);

    //step.14 send complete msg to main process 
    struct IpcMsgObj ipcObj;
    memset(&ipcObj, 0, sizeof(ipcObj));
    ipcObj.sendId = IPC_MSG_ID_DC;
    ipcObj.recvId = IPC_MSG_ID_MAIN;
    ipcObj.type = IPC_MSG_TYPE_BIN_START_SUCC;
    ipcObj.requestId = 0;
    std::string dataStr = "DataCenterProcess already startup";
    memcpy(ipcObj.data, dataStr.c_str(), dataStr.size());
    IpcMsg::sendUdsMsg(udsIpcSfd, udsMainPath, (void*)(&ipcObj), sizeof(ipcObj));

    //step.15 wait for all bins have finished starting
    waitForAllbinsStart();

    //step.20 start a working thread which will send somthing to other process
    std::thread workingThread(workingCbInDcProc, udsIpcSfd);

    //step.22 wait for current process exit
    waitForProcExit();

    //join all threads
    myIpcMsgBuffer.stopBuffering();
    udsIpcRecvThread.join();
    workingThread.join();

    std::cout<<"PID="<<getpid()<<", "<<"Dc process stopped, parent pid="<<getppid()<<std::endl;
    
    return 0;
}


