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

int udsIpcSfd = -1;
static const char udsPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr";
Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObjCbInDcProc);

void waitForProcExit()
{

}

void processIpcMsgObjCbInDcProc(IpcMsgObj obj)
{
    std::cout<<"processIpcMsgObjCbInDcProc"<<std::endl;
    obj.dump();

    if(IPC_MSG_TYPE_DATA == obj.type)
    {

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
        else
        {
            std::cout<<"recvUdsMsgCbInDcProc, receive"
                <<", send id="<<ipcObj.sendId
                <<", recv id="<<ipcObj.recvId
                <<", msg type="<<ipcObj.type<<std::endl;
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

    //delete socket file
    unlink(udsPath);

    //create UNIX domain IPC 
    udsIpcSfd = IpcMsg::createUdsIpc(udsPath);
    if(0 > udsIpcSfd)
    {
        return 1;
    }

    //start IPC msg receiving thread
    std::thread udsIpcRecvThread(recvUdsMsgCbInDcProc, udsIpcSfd);

    //ensure that receiving thread has finished starting
    sleep(2);

    //start a working thread which will send somthing to other process
    std::thread workingThread(workingCbInDcProc, udsIpcSfd);

    //wait for process exit
    waitForProcExit();

    //join all threads
    udsIpcRecvThread.join();
    
    return 0;
}


