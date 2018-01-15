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
#include <semaphore.h>

#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com;
using namespace Com::IpcMsg;

static void processIpcMsgObjCbInMainProc(IpcMsgObj obj);

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
        , execFilePath("")
        , execCmd("")
        , procState(START_WAITING)
    {}

    void dump()
    {
        std::cout<<"PID="<<getpid()<<", "<<"Process: name="<<procName<<", state="<<procState<<std::endl;
    }
    
    std::string execFile;
    std::string procName;
    std::string execFilePath;
    std::string execCmd;
    PROCESS_STATE procState;
};

static unsigned nextProcIndex = 0;
static int udsIpcSfd = -1;
static const unsigned TOTAL_PROC_NUM = 2;
static const char udsDcPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock_dc.socket";
static const char udsMainPath[] = "/home/yqian1/testing/Unix_Domain_Socket_Addr/ipc_sock_main.socket";
static sem_t semAllBinsStarted;
static sem_t semProcExit;
static std::array<Process, TOTAL_PROC_NUM> myProcArray;
static Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObjCbInMainProc);

///////////////////////////////////////////////////////////////////////
static void getProcList()
{
    Process controlProc("ControlProc.bin", "ControlProc");
    controlProc.procState = START_SUCCESS;
    
    Process dcProc("DcProc.bin", "DcProc");
    dcProc.execFilePath = "../dcp/DcProc.bin";
    dcProc.execCmd = "./" + dcProc.execFilePath;
    
    myProcArray[0] = controlProc;//main process at position [0]
    myProcArray[1] = dcProc;
}

static int spawnProc(Process& proc)
{    
    std::cout<<"PID="<<getpid()<<", "<<"spawnProc"<<std::endl;
    proc.dump();
    
    int rlt = system(proc.execCmd.c_str());
    if(-1 == rlt)
    {
        proc.dump();
        std::cout<<"PID="<<getpid()<<", "<<"spawnProc, error happen, errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}

static int startUp()
{
    std::cout<<"PID="<<getpid()<<", "<<"startUp, start checking all bin files..."<<std::endl;

    //skip main process
    for(unsigned i = 1;i < TOTAL_PROC_NUM;i++)
    {
        struct stat execFileInfo;
        int rlt = stat(myProcArray[i].execFilePath.c_str(), &execFileInfo);
        if(-1 == rlt)
        {
            myProcArray[i].dump();
            std::cout<<"PID="<<getpid()<<", "<<"startUp, error happen, errno="<<errno<<", "<<strerror(errno)<<std::endl;
            return -1;
        }
    }

    std::cout<<"PID="<<getpid()<<", "<<"startUp, start all bin files..."<<std::endl;

    //skip main process, start other bin files sequentially
    nextProcIndex += 1;
    if(TOTAL_PROC_NUM > nextProcIndex)
    {
        spawnProc(myProcArray[nextProcIndex]);
    }

    return 0;
} 

static void waitForAllbinsStart()
{
    if(0 != sem_wait(&semAllBinsStarted))
    {
        std::cout<<"PID="<<getpid()<<", "<<"waitForAllbinsStart, sem_wait error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }
}

static void waitForProcExit()
{
    if(0 != sem_wait(&semProcExit))
    {
        std::cout<<"PID="<<getpid()<<", "<<"waitForProcExit, sem_wait error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }
}

static void handleTermSig()
{
    if(0 != sem_post(&semProcExit))
    {
        std::cout<<"PID="<<getpid()<<", "<<"handleTermSig, semProcExit posted error, errno="
            <<errno<<", "<<strerror(errno)<<std::endl;
    }
}

///////////////////////////////////////////////////////////////////////
void processIpcMsgObjCbInMainProc(IpcMsgObj obj)
{
    //std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInMainProc"<<std::endl;
    //obj.dump();

    if(IPC_MSG_ID_MAIN != obj.recvId)
    {
        std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInMainProc, recvId error"<<std::endl;
        return;
    }

    //step.16 
    if(IPC_MSG_TYPE_BIN_START_SUCC == obj.type)
    {
        myProcArray[nextProcIndex].procState = START_SUCCESS;
        nextProcIndex += 1;

        //step.17 start next bin
        if(TOTAL_PROC_NUM > nextProcIndex)
        {
            spawnProc(myProcArray[nextProcIndex]);
        }
        //step.17 notify all bins that IPC start success
        else
        {
            std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInMainProc, received:"<<std::endl;
            std::cout<<"PID="<<getpid()<<", "<<"                      send id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.sendId)<<std::endl;
            std::cout<<"PID="<<getpid()<<", "<<"                      recv id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.recvId)<<std::endl;
            std::cout<<"PID="<<getpid()<<", "<<"                      msgtype: "<<IpcMsg::ipcMsgTypeToStr(obj.type)<<std::endl;
            std::cout<<"PID="<<getpid()<<", "<<"                      content: "<<obj.data<<std::endl;

            struct IpcMsgObj ipcObj;
            memset(&ipcObj, 0, sizeof(ipcObj));
            ipcObj.sendId = IPC_MSG_ID_MAIN;
            ipcObj.recvId = IPC_MSG_ID_DC;
            ipcObj.type = IPC_MSG_TYPE_BIN_ALL_SUCC;
            ipcObj.requestId = 0;
            std::string dataStr = "ALL processes startup";
            memcpy(ipcObj.data, dataStr.c_str(), sizeof(dataStr));
            IpcMsg::sendUdsMsg(udsIpcSfd, udsDcPath, (void*)(&ipcObj), sizeof(ipcObj));

            if(0 != sem_post(&semAllBinsStarted))
            {
                std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInMainProc, semAllBinsStarted posted error, errno="
                    <<errno<<", "<<strerror(errno)<<std::endl;
            }
        }
    }
    else if(IPC_MSG_TYPE_DATA == obj.type)
    {
        std::cout<<"PID="<<getpid()<<", "<<"processIpcMsgObjCbInMainProc, received:"<<std::endl;
        std::cout<<"PID="<<getpid()<<", "<<"                      send id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.sendId)<<std::endl;
        std::cout<<"PID="<<getpid()<<", "<<"                      recv id: "<<IpcMsg::ipcMsgGlobalIdToStr(obj.recvId)<<std::endl;
        std::cout<<"PID="<<getpid()<<", "<<"                      msgtype: "<<IpcMsg::ipcMsgTypeToStr(obj.type)<<std::endl;
        std::cout<<"PID="<<getpid()<<", "<<"                      content: "<<obj.data<<std::endl;
    }
}

static void recvUdsMsgCbInMainProc(int sfd)
{
    while(1)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));
    
        if(0 > IpcMsg::recvUdsMsg(sfd, (void*)(&ipcObj), sizeof(ipcObj)))
        {
            std::cout<<"PID="<<getpid()<<", "<<"recvUdsMsgCbInMainProc, receive error"<<std::endl;
        }

        std::cout<<"PID="<<getpid()<<", "<<"recvUdsMsgCbInMainProc, receiving, sfd="<<sfd<<std::endl;

        myIpcMsgBuffer.pushToBuffer(ipcObj);
    }
}

static void workingCbInMainProc(int sfd)
{
    for(unsigned i = 1;i <= 3;i++)
    {
        struct IpcMsgObj ipcObj;
        memset(&ipcObj, 0, sizeof(ipcObj));

        ipcObj.sendId = IPC_MSG_ID_MAIN;
        ipcObj.recvId = IPC_MSG_ID_DC;
        ipcObj.type = IPC_MSG_TYPE_DATA;
        ipcObj.requestId = i;

        std::string dataStr = "Data from MainProcess: " + std::to_string(i);
        memcpy(ipcObj.data, dataStr.c_str(), sizeof(dataStr));

        IpcMsg::sendUdsMsg(sfd, udsDcPath, (void*)(&ipcObj), sizeof(ipcObj));

        sleep(2);
    }    
}

///////////////////////////////////////////////////////////////////////
int main()
{
    std::cout<<"PID="<<getpid()<<", "<<"Main process start"<<std::endl;

    //step.1 delete socket file
    unlink(udsMainPath);

    //step.2 create UNIX domain IPC 
    udsIpcSfd = IpcMsg::createUdsIpc(udsMainPath);
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

    //step.3 start IPC msg receiving thread
    std::thread udsIpcRecvThread(recvUdsMsgCbInMainProc, udsIpcSfd);

    //step.4 start Buffering which is used to buffer the msg from other process
    myIpcMsgBuffer.startBuffering();

    //step.5 ensure that receiving thread and Buffering thread have finished starting
    sleep(1);

    //step.6 get process list of whole IPC project
    getProcList();

    //step.7 startup all process of IPC project
    startUp();

    //step.8 wait for all bins have finished starting
    waitForAllbinsStart();

    //step.19 start a working thread which will send somthing to other process
    std::thread workingThread(workingCbInMainProc, udsIpcSfd);

    //step.21 wait for current process exit
    waitForProcExit();

    //join all threads
    udsIpcRecvThread.join();
    workingThread.join();

    //close socket fd
    close(udsIpcSfd);
    
    return 0;
}

