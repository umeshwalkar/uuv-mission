#ifndef UDPCLIENT_HPP
#define UDPCLIENT_HPP

#include <string>
#include <cstdint>
#include <netinet/in.h>

class UdpClient
{
public:
    UdpClient();
    UdpClient(const std::string &serverIp, uint16_t serverPort);
    ~UdpClient();

    /** Initialize server and client.
     *
     *	@param[in]	s
     *		object pointer
     *  @param[in]  serverIP
     *      remote server IP string value
     *  @param[in]  serverPort
     *      port of the remote server
     */
    bool init(const std::string &serverIp, uint16_t serverPort);
    /** Deinitialize server and client.
     *
     *	@param[in]	s
     *		object pointer
     */
    void deinit();

    /** Send bytes.
     *
     *	@param[in]	s
     *		object pointer
     *  @param[in]  buffer
     *      pointer to the buffer to send
     *  @param[in]  num
     *      number of bytes to send
     *  @return
     *      number of bytes sent
     */
    int32_t send(const uint8_t *buffer, uint16_t num);

    /** Receive bytes.
     *
     *	@param[in]	s
     *		object pointer
     *  @param[out] buffer
     *      pointer to buffer to fill with received data
     *  @param[in]  num
     *      maximum number of bytes to read
     *  @return
     *      number of bytes read
     */
    int32_t recv(uint8_t *buffer, uint16_t num);

    bool isInitialized() const;

private:
    int clientSocket;
    socklen_t clientLength;
    sockaddr_in clientAddress;
    bool initialized;

    std::string serverIPString;
    uint16_t serverPort;
};

#endif /* UDPCLIENT_HPP */
