/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Define IPC msg structure and API
*/

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

class IpcMsgObj
{
public:
    IpcMsgObj(IpcMsgGlobalId sId, IpcMsgGlobalId rId, IpcMsgType t)
        : sendId(sId)
        , recvId(rId)
        , type(t)
    {}

    void dump()
    {
        std::cout<<"dump, IpcMsgObj: sendId="<<sendId<<", recvId="
            <<recvId<<", msg type="<<type<<std::endl;
    }
    
    IpcMsgGlobalId sendId;
    IpcMsgGlobalId recvId;
    IpcMsgType type;
    int requestId;
};

}
}

