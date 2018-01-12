/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Define IPC msg structure and API
*/

#ifndef IPCMSG_H
#define IPCMSG_H

#include <iostream>

namespace Com {
namespace IpcMsg {
    
enum IpcMsgGlobalId
{
    IPC_MSG_ID_MAIN = 0, //ControlProcess
    IPC_MSG_ID_DC //DcProcess
};

enum IpcMsgType
{
    IPC_MSG_TYPE_BIN = 0,
    IPC_MSG_TYPE_DATA
};

struct IpcMsgObj
{
public:
    IpcMsgObj(IpcMsgGlobalId sId, IpcMsgGlobalId rId, IpcMsgType t, int id)
        : sendId(sId)
        , recvId(rId)
        , type(t)
        , requestId(id)
    {}

    void setData(int d)
    {
        data = d;
    }

    void dump()
    {
        std::cout<<"dump, IpcMsgObj: sendId="<<sendId
            <<", recvId="<<recvId
            <<", msg type="<<type
            <<", requestId"<<requestId
            <<", data"<<data<<std::endl;
    }
    
    IpcMsgGlobalId sendId;
    IpcMsgGlobalId recvId;
    IpcMsgType type;
    int requestId;
    int data;
};

int createUdsIpc();
void sendUdsMsg();
void recvUdsMsg();

}
}

#endif

