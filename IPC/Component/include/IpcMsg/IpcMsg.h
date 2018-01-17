/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Define IPC msg structure and API
*/

#ifndef IPCMSG_H
#define IPCMSG_H

#include <iostream>
#include <string>

namespace Com {
namespace IpcMsg {
    
enum IpcMsgGlobalId
{
    IPC_MSG_ID_MAIN = 0, //ControlProcess
    IPC_MSG_ID_DC, //DcProcess

    IPC_MSG_ID_UNKNOW
};

enum IpcMsgType
{
    IPC_MSG_TYPE_BIN_START_SUCC = 0,
    IPC_MSG_TYPE_BIN_ALL_SUCC,
    IPC_MSG_TYPE_BIN_STOP,
    IPC_MSG_TYPE_BIN_STOP_SUCC,
    IPC_MSG_TYPE_DATA,

    IPC_MSG_TYPE_UNKNOW
};

struct IpcMsgObj
{
    void dump()
    {
        std::cout<<"dump, IpcMsgObj: sendId="<<sendId
            <<", recvId="<<recvId
            <<", msg type="<<type
            <<", requestId="<<requestId<<std::endl;
    }
    
    IpcMsgGlobalId sendId;
    IpcMsgGlobalId recvId;
    IpcMsgType type;
    int requestId;
    char data[256];
};

int createUdsIpc(const char* sockFile);
int sendUdsMsg(int sfd, const char* sockFile, void* buf, int size);
int recvUdsMsg(int sfd, void* buf, int size);
void closeSock(int sfd, int flag);
std::string ipcMsgGlobalIdToStr(IpcMsgGlobalId id);
std::string ipcMsgTypeToStr(IpcMsgType type);

}
}

#endif

