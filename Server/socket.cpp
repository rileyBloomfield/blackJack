#include <strings.h>
#include <unistd.h>
#include <iostream>
#include <errno.h>

#include "socket.h"

Socket::Socket(std::string const & ipAddress, unsigned int port)
    : open(false)
{
    // First, call socket() to get a socket file descriptor
    socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0)
        throw std::string("Unable to initialize socket server");

    // Start by zeroing out the socket descriptor
     bzero((char*)&socketDescriptor,sizeof(sockaddr_in));

    // Now try to map the IP address, as provided, into the socket Descriptor
    if (!inet_aton(ipAddress.c_str(),&socketDescriptor.sin_addr))
        throw std::string("IP Address provided is invalid");
    socketDescriptor.sin_family = AF_INET;
    socketDescriptor.sin_port = htons(port);
}

Socket::Socket(int sFD)
{
    socketFD = sFD;
    open = true;
}

Socket::Socket(Socket const & s)
{
    *this = s;
}

Socket & Socket::operator=(Socket const & rhs)
{
    socketDescriptor = rhs.socketDescriptor;
    socketFD = dup(rhs.socketFD);
    open = rhs.open;
}


Socket::~Socket(void)
{
    //if (open)
        //Close();
}


int Socket::Open(void)
{
    int connectReturn = connect(socketFD,(sockaddr*)&socketDescriptor,sizeof(socketDescriptor));
    if (connectReturn != 0)
    {
        throw std::string("Unable to open connection");
    }
    open = true;
}

int Socket::Write(ByteArray const & buffer)
{
    if (!open)
        throw std::string("Attempt to write to an unopened socket");
    char * raw = new char[buffer.v.size()];
    for (int i=0;i<buffer.v.size();i++)
        raw[i] = buffer.v[i];
    return write(socketFD,raw,buffer.v.size());
}

static const int MAX_BUFFER_SIZE = 256;
int Socket::Read(ByteArray & buffer)
{
    char raw[MAX_BUFFER_SIZE];
    if (!open)
        throw std::string("Attempt to receive from an unopened socket");

    buffer.v.clear();
    // Messages greater than MAX_BUFFER_SIZE are not handled gracefully.
    ssize_t received = recv(socketFD, raw, MAX_BUFFER_SIZE, 0);
    for (int i=0;i<received;i++)
        buffer.v.push_back(raw[i]);
    return received;
}

void Socket::Close(void)
{
   if (open) {
        shutdown(socketFD, SHUT_RDWR); //Break active reads
        close(socketFD);
   }
}
