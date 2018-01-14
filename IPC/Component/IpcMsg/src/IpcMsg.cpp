/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Define IPC msg structure and API
*/

#include <sys/un.h>
#include <string.h>

#include "Socket.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com;
using namespace Com::IpcMsg;

int IpcMsg::createUdsIpc(const char* sockFile)
{
    //create un socket
    int sfd = Socket::createSocket(AF_UNIX, SOCK_DGRAM);
    if(0 > sfd)
    {
        std::cout<<"createUdsIpc, create socket error"<<std::endl;
        return -1;
    }

    //set non-block flag
    if(0 > Socket::setNonBlockSocket(sfd))
    {
        std::cout<<"createUdsIpc, set non block flag error"<<std::endl;
        Socket::closeFd(sfd);
        return -1;
    }

    //bind sfd to addr
    struct sockaddr_un saddr;
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, sockFile);
    std::cout<<"createUdsIpc, uds file="<<saddr.sun_path<<std::endl;
    
    if(0 > Socket::bindSocketTo(sfd, (struct sockaddr*)&saddr, sizeof(saddr)))
    {
        std::cout<<"createUdsIpc, bind error"<<std::endl;
        Socket::closeFd(sfd);
        return -1;
    }
    
    return sfd;
}

int IpcMsg::sendUdsMsg(int sfd, const char* sockFile, void* buf, int size)
{
    if(-1 == Socket::sendMessage(sfd, sockFile, buf, size))
    {
        std::cout<<"sendUdsMsg, error"<<std::endl;
        return -1;
    }

    return 0;
}

int IpcMsg::recvUdsMsg(int sfd, void* buf, int size)
{
    if(-1 == Socket::recvMessage(sfd, buf, size))
    {
        std::cout<<"recvUdsMsg, error"<<std::endl;
        return -1;
    }
    
    return 0;
}


