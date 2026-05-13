#ifndef __UDPCLIENT_H_
#define __UDPCLIENT_H_


#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/** UDP client object. */
struct UdpClientStruct {
    /** Client socket ID. */
    int clientSocket;
    /** Client socket length. */
    socklen_t clientLength;
    /** Local address struct. */
    struct sockaddr_in localAddress;
    /** Client address struct. */
    struct sockaddr_in clientAddress;
    /** Initialized flag. */
    uint8_t initialized;
    int8_t *serverIPString[32];
    uint16_t serverPort;
};


/** Initialize server and client.
 *
 *	@param[in]	s
 *		object pointer
 *  @param[in]  serverIP
 *      remote server IP string value
 *  @param[in]  serverPort
 *      port of the remote server
 */
void UdpClient_init(struct UdpClientStruct *s,
						  int8_t *serverIP,
						  uint16_t serverPort);


/** Deinitialize server and client.
 *
 *	@param[in]	s
 *		object pointer
 */
void UdpClient_deinit(struct UdpClientStruct *s);


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
int32_t UdpClient_recv(struct UdpClientStruct *s,
		  	  	  	  	  	 uint8_t *buffer,
		  	  	  	  	  	 uint16_t num);


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
int32_t UdpClient_send(struct UdpClientStruct *s,
		  	  	  	  	  	 uint8_t *buffer,
		  	  	  	  	  	 uint16_t num);


#ifdef __cplusplus
}
#endif

#endif /* __UDPCLIENT_H_ */
