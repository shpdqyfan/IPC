/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: IPC msg based on Socket
*/

#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <string.h>

#include "Socket.h"

using namespace Com::IpcMsg;

int Socket::createSocket(sa_family_t family, int stype)
{
    int sfd = socket(family, stype, 0);
    if(0 > sfd)
    {
        std::cout<<"createSocket, error"<<std::endl;
        return -1;
    }

    std::cout<<"createSocket, succ, sfd="<<sfd<<std::endl;
    
    return sfd;
}

int Socket::setNonBlockSocket(int sfd)
{
    //non-block
    int flags = fcntl(sfd, F_GETFL, 0);
    if(0 > fcntl(sfd, F_SETFL, flags | O_NONBLOCK))
    {
        std::cout<<"createNonBlockSocket, set O_NONBLOCK error"<<std::endl;
        return -1;
    }

    return 0;
}

int Socket::bindSocketTo(int sfd, const struct sockaddr* saddr)
{
    if(0 > bind(sfd, saddr, sizeof saddr))
    {
        std::cout<<"bindSocketTo, error, sfd="<<sfd<<std::endl;
        return -1;
    }

    return 0;
}

int Socket::listenSocket(int sfd)
{
    if(listen(sfd, MAX_CONN_NUM))
    {
        std::cout<<"listenSocket, error, sfd="<<sfd<<std::endl;
        return -1;
    }

    return 0;
}

int Socket::acceptFromPeer(int sfd, struct sockaddr* saddr)
{
    socklen_t slen = sizeof(sockaddr);
    int newfd = accept(sfd, saddr, &slen);
    if(0 > newfd)
    {
        std::cout<<"acceptFromPeer, sfd="<<sfd<<", errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return newfd;
}

int Socket::connectToPeer(int sfd, const struct sockaddr* saddr)
{
    int rlt = connect(sfd, saddr, sizeof saddr);
    if(0 > rlt)
    {
        std::cout<<"connectToPeer, sfd="<<sfd<<", errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}

void Socket::closeFd(int sfd)
{
    std::cout<<"closeFd, sfd"<<sfd<<std::endl;
    
    close(sfd);
}

ssize_t Socket::recvMessage(int sfd, void* buf, int size)
{
    struct iovec ioBuf[1];
    struct msghdr mhdr;

    ioBuf[0].iov_base = buf;
    ioBuf[0].iov_len = size;
    
    mhdr.msg_name = NULL;
    mhdr.msg_namelen = 0;
    mhdr.msg_iov = ioBuf;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = NULL;
    mhdr.msg_controllen = 0;
    mhdr.msg_flags = 0;
    
    int rlt = recvmsg(sfd, &(mhdr), 0);
    if(0 > rlt)
    {
        std::cout<<"recvMessage, sfd="<<sfd<<", errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}

ssize_t Socket::sendMessage(int sfd, char* sockFile, void* buf, int size)
{
    struct sockaddr_un sUnAddr;
    struct iovec ioBuf[1];
    struct msghdr mhdr;

    sUnAddr.sun_family = AF_UNIX;
    strcpy(sUnAddr.sun_path, sockFile);

    ioBuf[0].iov_base = buf;
    ioBuf[0].iov_len = size;
    
    mhdr.msg_name = &sUnAddr;
    mhdr.msg_namelen = sizeof(sockaddr_un);
    mhdr.msg_iov = ioBuf;
    mhdr.msg_iovlen = 1;
    mhdr.msg_control = NULL;
    mhdr.msg_controllen = 0;
    mhdr.msg_flags = 0;
    
    int rlt = sendmsg(sfd, &(mhdr), 0);
    if(0 > rlt)
    {
        std::cout<<"sendMessage, sfd="<<sfd<<", errno="<<errno<<", "<<strerror(errno)<<std::endl;
        return -1;
    }

    return 0;
}

