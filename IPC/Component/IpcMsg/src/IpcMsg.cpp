/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: Define IPC msg structure and API
*/

#include <sys/un.h>
#include <string.h>

#include "Socket.h"
#include "IpcMsg/IpcMsg.h"

using namespace Com::IpcMsg;

int createUdsIpc(char* sockFile)
{
    //create un socket
    int sfd = Socket::createSocket(AF_UNIX, SOCK_DGRAM);
    if(0 > sfd)
    {
        std::cout<<"createUdsIpc, create socket error"<<std::endl;
    }

    //set non-block flag
    if(0 > Socket::setNonBlockSocket(sfd))
    {
        std::cout<<"createUdsIpc, set non block error"<<std::endl;
        Socket::closeFd(sfd);
        return -1;
    }

    //bind sfd to addr
    struct sockaddr_un saddr;
    saddr.sun_family = AF_UNIX;
    strcpy(saddr.sun_path, sockFile);
    if(0 > Socket::bindSocketTo(sfd, (struct sockaddr*)&saddr))
    {
        std::cout<<"createUdsIpc, bind error"<<std::endl;
        Socket::closeFd(sfd);
        return -1;
    }
    
    return 0;
}

void sendUdsMsg()
{

}

void recvUdsMsg()
{

}


