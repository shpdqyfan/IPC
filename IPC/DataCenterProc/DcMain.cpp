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

#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com;
using namespace Com::IpcMsg;

void processIpcMsgObjCbInDcProc(IpcMsgObj obj);

bool startFinished = false;
int udsIpcSfd = -1;
static const char udsPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock";
Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObjCbInDcProc);

//use semaphore to replace
void waitForAllbinsStart()
{
    while(1 && !startFinished)
    {
        sleep(1);
    }
}

//use semaphore to replace
void waitForProcExit()
{
    while(1)
    {
        sleep(1);
    }
}

void processIpcMsgObjCbInDcProc(IpcMsgObj obj)
{
    std::cout<<"processIpcMsgObjCbInDcProc"<<std::endl;
    obj.dump();

    if(IPC_MSG_ID_DC != obj.recvId)
    {
        std::cout<<"processIpcMsgObjCbInDcProc, recvId error"<<std::endl;
        return;
    }

    //step.18 
    if(IPC_MSG_TYPE_DATA == obj.type)
    {
        std::cout<<"processIpcMsgObjCbInDcProc, all bins start finished"<<std::endl;
        startFinished = true;
    }
    else if(IPC_MSG_TYPE_DATA == obj.type)
    {
        std::cout<<"recvUdsMsgCbInDcProc, received:"<<std::endl;
        std::cout<<"                      send id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.sendId)<<std::endl;
        std::cout<<"                      recv id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.recvId)<<std::endl;
        std::cout<<"                      msgtype: "<<IpcMsg::ipcMsgTypeToStr(obj.type)<<std::endl;
        std::cout<<"                      content: "<<obj.data<<std::endl;
    }
}

void recvUdsMsgCbInDcProc(int sfd)
{
    while(1)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));
    
        if(0 > IpcMsg::recvUdsMsg(sfd, (void*)(&ipcObj), sizeof(ipcObj)))
        {
            std::cout<<"recvUdsMsgCbInDcProc, receive error"<<std::endl;
        }

        myIpcMsgBuffer.pushToBuffer(ipcObj);
    }
}

void workingCbInDcProc(int sfd)
{
    for(unsigned i = 1;i <= 10;i++)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));

        ipcObj.sendId = IPC_MSG_ID_MAIN;
        ipcObj.recvId = IPC_MSG_ID_DC;
        ipcObj.type = IPC_MSG_TYPE_DATA;
        ipcObj.requestId = i;

        std::string dataStr = "Data from DataCenterProcess: " + std::to_string(i);
        memcpy(ipcObj.data, dataStr.c_str(), sizeof(dataStr));

        IpcMsg::sendUdsMsg(sfd, udsPath, (void*)(&ipcObj), sizeof(ipcObj));

        sleep(3);
    }    
}

int main()
{
    std::cout<<"Dc process start, triggered by main process"<<std::endl;

    //step.9 delete socket file
    unlink(udsPath);

    //step.10 create UNIX domain IPC 
    udsIpcSfd = IpcMsg::createUdsIpc(udsPath);
    if(0 > udsIpcSfd)
    {
        return 1;
    }

    //step.11 start IPC msg receiving thread
    std::thread udsIpcRecvThread(recvUdsMsgCbInDcProc, udsIpcSfd);

    //step.12 start Buffering which is used to buffer the msg from other process
    myIpcMsgBuffer.startBuffering();

    //step.13 ensure that receiving thread and Buffering thread have finished starting
    sleep(2);

    //step.14 send complete msg to main process 
    struct IpcMsgObj ipcObj;
    memset(&ipcObj, 0, sizeof(ipcObj));
    ipcObj.sendId = IPC_MSG_ID_DC;
    ipcObj.recvId = IPC_MSG_ID_MAIN;
    ipcObj.type = IPC_MSG_TYPE_BIN_START_SUCC;
    ipcObj.requestId = 0;
    std::string dataStr = "DataCenterProcess already startup";
    memcpy(ipcObj.data, dataStr.c_str(), sizeof(dataStr));
    IpcMsg::sendUdsMsg(udsIpcSfd, udsPath, (void*)(&ipcObj), sizeof(ipcObj));

    //step.15 wait for all bins have finished starting
    waitForAllbinsStart();

    //step.20 start a working thread which will send somthing to other process
    std::thread workingThread(workingCbInDcProc, udsIpcSfd);

    //step.22 wait for process exit
    waitForProcExit();

    //join all threads
    udsIpcRecvThread.join();
    workingThread.join();
    
    return 0;
}


