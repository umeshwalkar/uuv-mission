#include "udpclient.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

UdpClient::UdpClient()
    : clientSocket(-1),
      clientLength(0),
      initialized(false),
      serverPort(0)
{
}

UdpClient::UdpClient(const std::string &serverIp,
                     uint16_t port)
    : UdpClient()
{
    init(serverIp, port);
}

UdpClient::~UdpClient()
{
    deinit();
}

bool UdpClient::init(const std::string &serverIp,
                     uint16_t port)
{
    serverIPString = serverIp;
    serverPort = port;

    clientLength = sizeof(clientAddress);

    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (clientSocket == -1)
    {
        std::cerr << "Error creating UDP client socket\n";
        return false;
    }

    memset(&clientAddress, 0, sizeof(clientAddress));

    clientAddress.sin_family = AF_INET;
    clientAddress.sin_port = htons(serverPort);

    if (inet_aton(serverIp.c_str(),
                  &clientAddress.sin_addr) == 0)
    {
        addrinfo hints{};
        addrinfo *res = nullptr;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        int status = getaddrinfo(serverIp.c_str(),
                                 nullptr,
                                 &hints,
                                 &res);

        if (status != 0)
        {
            std::cerr << "DNS resolution failed: "
                      << gai_strerror(status)
                      << "\n";

            close(clientSocket);
            return false;
        }

        clientAddress.sin_addr =
            ((sockaddr_in *)res->ai_addr)->sin_addr;

        freeaddrinfo(res);
    }

    initialized = true;

    return true;
}

void UdpClient::deinit()
{
    if (initialized)
    {
        close(clientSocket);
        initialized = false;
    }
}

int32_t UdpClient::send(const uint8_t *buffer,
                        uint16_t num)
{
    if (!initialized)
        return 0;

    int n = sendto(clientSocket,
                   buffer,
                   num,
                   0,
                   (sockaddr *)&clientAddress,
                   clientLength);

    return (n == -1) ? 0 : n;
}

int32_t UdpClient::recv(uint8_t *buffer,
                        uint16_t num)
{
    if (!initialized)
        return 0;

    int n = recvfrom(clientSocket,
                     buffer,
                     num,
                     MSG_DONTWAIT,
                     (sockaddr *)&clientAddress,
                     &clientLength);

    return (n == -1) ? 0 : n;
}

bool UdpClient::isInitialized() const
{
    return initialized;
}