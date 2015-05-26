#include "socketserver.h"
#include <strings.h>
#include <iostream>
#include <errno.h>

SocketServer::SocketServer(int port)
{
    // The first call has to be to socket(). This creates a UNIX socket.
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
        throw std::string("Unable to open the socket server");
   int optval = 1;
   setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(sockaddr_in));
    // The second call is to bind().  This identifies the socket file
    // descriptor with the description of the kind of socket we want to have.
    bzero((char*)&socketDescriptor,sizeof(sockaddr_in));
    socketDescriptor.sin_family = AF_INET;
    socketDescriptor.sin_port = htons(port);
    socketDescriptor.sin_addr.s_addr = INADDR_ANY;
    if (bind(socketFD,(sockaddr*)&socketDescriptor,sizeof(socketDescriptor)) < 0)
        throw std::string("Unable to bind socket to requested port");

    // Set up a maximum number of pending connections to accept
    listen(socketFD,5);

    // At this point, the object is initialized.  So return.
}


// This function will block forever.  It is probably best to use shutdown from a master thread to
// kill the call to accept.  But let's do it later.
Socket SocketServer::Accept(void)
{
    int connectionFD = accept(socketFD,NULL,0);
    if (connectionFD < 0)
    {
        if (errno == EINVAL)
            throw TerminationException(errno);
        else
            throw std::string("Unexpected error in the server");
    }
    std::cout << "cfd:" << connectionFD << " socketFD:" << socketFD << std::endl;
    return Socket(connectionFD);
}

void SocketServer::Shutdown(void)
{
    shutdown(socketFD, SHUT_RDWR);
}
