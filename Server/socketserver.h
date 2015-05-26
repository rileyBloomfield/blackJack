#ifndef SOCKETSERVER_H
#define SOCKETSERVER_H
#include "socket.h"
typedef int TerminationException;

class SocketServer
{
private:
    int socketFD;
    sockaddr_in socketDescriptor;
public:
    SocketServer(int port);
    Socket Accept(void);
    void Shutdown(void);
};

#endif // SOCKETSERVER_H
