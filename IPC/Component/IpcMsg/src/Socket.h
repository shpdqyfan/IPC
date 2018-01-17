/*
    Date: 2017/12/26
    Author: shpdqyfan
    profile: IPC msg based on Socket
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>

namespace Com {
namespace IpcMsg {
namespace Socket {

    const unsigned MAX_CONN_NUM = 5;

    int createSocket(sa_family_t family, int stype);
    int setNonBlockSocket(int sfd);
    int bindSocketTo(int sfd, const struct sockaddr* saddr, int len);
    int listenSocket(int sfd);
    int acceptFromPeer(int sfd, struct sockaddr* saddr);
    int connectToPeer(int sfd, const struct sockaddr* saddr);
    void closeFd(int sfd);
    void shutdownSock(int sfd, int flag);
    ssize_t recvMessage(int sfd, void* buf, int size);
    //used in unix domain socket
    ssize_t sendMessage(int sfd, const char* sockFile, void* buf, int size);
    
}
}
}

#endif

