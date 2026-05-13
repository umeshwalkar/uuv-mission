#include "udpserver.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

UdpServer::UdpServer()
    : serverSocket(-1),
      clientLength(0),
      initialized(false)
{
}

UdpServer::UdpServer(uint16_t localPort)
    : UdpServer()
{
    init(localPort);
}

UdpServer::~UdpServer()
{
    deinit();
}

bool UdpServer::init(uint16_t localPort)
{
    clientLength = sizeof(other);

    serverSocket = socket(AF_INET,
                          SOCK_DGRAM,
                          IPPROTO_UDP);

    if (serverSocket == -1)
    {
        std::cerr << "Error creating UDP socket\n";
        return false;
    }

    int reuse = 1;

    setsockopt(serverSocket,
               SOL_SOCKET,
               SO_REUSEADDR,
               &reuse,
               sizeof(reuse));

    memset(&me, 0, sizeof(me));

    me.sin_family = AF_INET;
    me.sin_port = htons(localPort);
    me.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverSocket,
             (sockaddr *)&me,
             sizeof(me)) == -1)
    {
        std::cerr << "Bind failed\n";

        close(serverSocket);

        return false;
    }

    initialized = true;

    return true;
}

void UdpServer::deinit()
{
    if (initialized)
    {
        close(serverSocket);
        initialized = false;
    }
}

int32_t UdpServer::recv(uint8_t *buffer,
                        uint16_t num)
{
    if (!initialized)
        return 0;

    int n = recvfrom(serverSocket,
                     buffer,
                     num,
                     MSG_DONTWAIT,
                     (sockaddr *)&other,
                     &clientLength);

    return (n == -1) ? 0 : n;
}

int32_t UdpServer::send(const uint8_t *buffer,
                        uint16_t num)
{
    if (!initialized)
        return 0;

    int n = sendto(serverSocket,
                   buffer,
                   num,
                   0,
                   (sockaddr *)&other,
                   clientLength);

    return (n == -1) ? 0 : n;
}

bool UdpServer::isInitialized() const
{
    return initialized;
}
