/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Main process which will be used as a deamon of Ipc project. 
    Meanwhile, sending/receiving msg to/from another process which
    named as "DcProcess" in Ipc project.
*/

#include <iostream>
#include <thread>
#include <array>
#include <string>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com;
using namespace Com::IpcMsg;

void processIpcMsgObjCbInMainProc(IpcMsgObj obj);

///////////////////////////////////////////////////////////////////////
typedef enum
{
    START_WAITING = 0,
    START_SUCCESS,
    START_FAIL,
    START_UNKNOWN
}PROCESS_STATE;

class Process
{
public:
    Process()
    {}
    
    Process(const std::string& exec, const std::string& name)
        : execFile(exec)
        , procName(name)
        , execCmd("./" + execFile)
        , procState(START_WAITING)
    {}

    void dump()
    {
        std::cout<<"Process info: name="<<procName<<", state="<<procState<<std::endl;
    }
    
    std::string execFile;
    std::string procName;
    std::string execCmd;
    PROCESS_STATE procState;
};

unsigned nextProcIndex = 0;
static const unsigned TOTAL_PROC_NUM = 2;
static const char udsPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock";
std::array<Process, TOTAL_PROC_NUM> myProcArray;
int udsIpcSfd = -1;
Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObjCbInMainProc);

///////////////////////////////////////////////////////////////////////
void getProcList()
{
    Process controlProc("ControlProc.bin", "ControlProc");
    controlProc.procState = START_SUCCESS;
    
    Process dcProc("DcProc.bin", "DcProc");
    
    myProcArray[0] = controlProc;//main process at position [0]
    myProcArray[1] = dcProc;
}

int spawnProc(Process& proc)
{    
    std::cout<<"spawnProc"<<std::endl;
    proc.dump();
    
    int rlt = system(proc.execCmd.c_str());
    if(-1 == rlt)
    {
        proc.dump();
        std::cout<<"spawnProc, error happen, errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}

int startUp()
{
    std::cout<<"startUp, start checking all bin files..."<<std::endl;

    //skip main process
    for(unsigned i = 1;i < 2;i++)
    {
        struct stat execFileInfo;
        int rlt = stat(myProcArray[i].execFile.c_str(), &execFileInfo);
        if(-1 == rlt)
        {
            myProcArray[i].dump();
            std::cout<<"startUp, error happen, errno="<<errno<<", "<<strerror(errno)<<std::endl;
            return -1;
        }
    }

    std::cout<<"startUp, start all bin files..."<<std::endl;

    //skip main process, start other bin files sequentially
    nextProcIndex += 1;
    if(TOTAL_PROC_NUM > nextProcIndex)
    {
        spawnProc(myProcArray[nextProcIndex]);
    }

    return 0;
} 

void waitForProcExit()
{
    while(1){}
}

///////////////////////////////////////////////////////////////////////
void processIpcMsgObjCbInMainProc(IpcMsgObj obj)
{
    std::cout<<"processIpcMsgObjCbInMainProc"<<std::endl;
    obj.dump();

    if(IPC_MSG_TYPE_BIN_START == obj.type)
    {
        myProcArray[nextProcIndex].procState = START_SUCCESS;
        nextProcIndex += 1;
        if(TOTAL_PROC_NUM > nextProcIndex)
        {
            //start next bin
            spawnProc(myProcArray[nextProcIndex]);
        }
        else
        {
            std::cout<<"processIpcMsgObjCbInMainProc, IPC project start complete"<<std::endl;
        }
    }
    else if(IPC_MSG_TYPE_DATA == obj.type)
    {

    }
}

void recvUdsMsgCbInMainProc(int sfd)
{
    while(1)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));
    
        if(0 > IpcMsg::recvUdsMsg(sfd, (void*)(&ipcObj), sizeof(ipcObj)))
        {
            std::cout<<"recvUdsMsgCbInMainProc, receive error"<<std::endl;
        }
        else
        {
            std::cout<<"recvUdsMsgCbInMainProc, receive"
                <<", send id="<<ipcObj.sendId
                <<", recv id="<<ipcObj.recvId
                <<", msg type="<<ipcObj.type<<std::endl;
        }

        myIpcMsgBuffer.pushToBuffer(ipcObj);
    }
}

void workingCbInMainProc(int sfd)
{
    for(unsigned i = 1;i <= 10;i++)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));

        ipcObj.sendId = IPC_MSG_ID_MAIN;
        ipcObj.recvId = IPC_MSG_ID_DC;
        ipcObj.type = IPC_MSG_TYPE_DATA;
        ipcObj.requestId = i;

        std::string dataStr = "Data from MainProcess: " + std::to_string(i);
        memcpy(ipcObj.data, dataStr.c_str(), sizeof(dataStr));

        IpcMsg::sendUdsMsg(sfd, udsPath, (void*)(&ipcObj), sizeof(ipcObj));

        sleep(3);
    }    
}

///////////////////////////////////////////////////////////////////////
int main()
{
    std::cout<<"Main process start"<<std::endl;

    //delete socket file
    unlink(udsPath);

    //create UNIX domain IPC 
    udsIpcSfd = IpcMsg::createUdsIpc(udsPath);
    if(0 > udsIpcSfd)
    {
        return 1;
    }

    //start IPC msg receiving thread
    std::thread udsIpcRecvThread(recvUdsMsgCbInMainProc, udsIpcSfd);

    //start Buffering which is used to buffer the msg from other process
    myIpcMsgBuffer.startBuffering();

    //ensure that receiving thread and Buffering thread have finished starting
    sleep(2);

    //get process list of whole IPC project
    getProcList();

    //startup all process of IPC project
    startUp();

    //start a working thread which will send somthing to other process
    std::thread workingThread(workingCbInMainProc, udsIpcSfd);

    //wait for current process exit
    waitForProcExit();

    //join all threads
    udsIpcRecvThread.join();
    workingThread.join();
    
    return 0;
}

