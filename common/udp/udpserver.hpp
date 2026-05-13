#ifndef UDPSERVER_HPP
#define UDPSERVER_HPP

#include <string>
#include <cstdint>
#include <netinet/in.h>

class UdpServer
{
public:
    UdpServer();
    UdpServer(uint16_t localPort);
    ~UdpServer();

    /** Initialize server and client.
     *
     *	@param[in]	s
     *		object pointer
     *  @param[in]  localPort
     *      local port to bind
     */
    bool init(uint16_t localPort);

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
    /** Server socket ID. */
    int serverSocket;

    /** Client socket length. */
    socklen_t clientLength;

    /** Local address struct. */
    sockaddr_in me;

    /** Client address struct. */
    sockaddr_in other;

    /** Initialized flag. */
    bool initialized;
};

#endif // UDPSERVER_HPP
