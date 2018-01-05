/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Main process which will be used as a deamon of Ipc project. 
    Meanwhile, sending/receiving msg to/from another process which
    named as "DcProcess" in Ipc project.
*/

#include <iostream>
#include <array>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#include "Buffer/Buffer.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com::IpcMsg;

///////////////////////////////////////////////////////////////////////
typedef enum
{
    START_WAITING = 0,
    START_SUCCESS,
    START_FAIL,
    START_UNKNOWN
}PROCESS_STATE;

///////////////////////////////////////////////////////////////////////
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
std::array<Process, TOTAL_PROC_NUM> myProcArray;

///////////////////////////////////////////////////////////////////////
void genProcList()
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

void processIpcMsgObj(IpcMsgObj obj)
{
    std::cout<<"processIpcMsgObj"<<std::endl;
    obj.dump();

    if(IPC_MSG_TYPE_BIN == obj.type)
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
            std::cout<<"processIpcMsgObj, IPC project start complete"<<std::endl;
        }
    }
    else if(IPC_MSG_TYPE_DATA == obj.type)
    {

    }
}

///////////////////////////////////////////////////////////////////////
Buffer<IpcMsgObj> myIpcMsgBuffer(processIpcMsgObj);
int main()
{
    std::cout<<"Main process start"<<std::endl;

    //create IPC msg sending thread

    //create IPC msg receiving thread

    //get process list of whole IPC project
    genProcList();

    //startup all process of IPC project
    startUp();
    
    return 0;
}

