#ifndef __UDPSERVER_H_
#define __UDPSERVER_H_


#include <sys/socket.h>
#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif

/** UDP server object. */
struct UdpServerStruct {

    /** Server address struct. */
    struct sockaddr_in serverAddress;
    /** Client socket ID. */
    int clientSocket;
    /** Client socket length. */
    socklen_t clientLength;
    /** Local address struct. */
    struct sockaddr_in me;
    /** Client address struct. */
    struct sockaddr_in other;
    /** Server socket ID. */
    int serverSocket;
    /** Initialized flag. */
    uint8_t initialized;
};


/** Initialize server and client.
 *
 *	@param[in]	s
 *		object pointer
 *  @param[in]  localPort
 *      local port to bind
 */
void UdpServer_init(struct UdpServerStruct *s,
						  uint16_t localPort);


/** Deinitialize server and client.
 *
 *	@param[in]	s
 *		object pointer
 */
void UdpServer_deinit(struct UdpServerStruct *s);


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
int32_t UdpServer_recv(struct UdpServerStruct *s,
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
int32_t UdpServer_send(struct UdpServerStruct *s,
		  	  	  	  	  	 uint8_t *buffer,
		  	  	  	  	  	 uint16_t num);


#ifdef __cplusplus
}
#endif

#endif /* __UDPSERVER_H_ */
