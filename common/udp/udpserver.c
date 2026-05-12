#include "udpserver.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


void UdpServer_init(struct UdpServerStruct *s,
                          uint16_t localPort) {
    if (!s) {
        printf("UdpServer_init(): Invalid parameters\\n");
        return;
    }
    
    s->clientLength = sizeof(s->other);

    // Create the socket
    if ((s->serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("UdpServer_init(): Error creating socket\\n");
        s->initialized = 0;
        return;
    }

    // Enable socket reuse to avoid "Address already in use" errors
    int reuse = 1;
    if (setsockopt(s->serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        printf("UdpServer_init(): Error setting SO_REUSEADDR\\n");
        close(s->serverSocket);
        s->initialized = 0;
        return;
    }

    // Initialize server address
    bzero(&s->me, sizeof(s->me));
    s->me.sin_family = AF_INET;
    s->me.sin_port = htons(localPort);
    s->me.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port
    if (bind(s->serverSocket, (struct sockaddr*) &s->me,
             sizeof(s->me)) == -1) {
        printf("UdpServer_init(): Error binding to port %d\\n", localPort);
        close(s->serverSocket);
        s->initialized = 0;
        return;
    }
    
    printf("UdpServer_init(): Listening on port %d\\n", localPort);
    s->initialized = 1;
}
/*--------------------------------------------------------------------------*/
void UdpServer_deinit(struct UdpServerStruct *s) {
	close(s->serverSocket);
    s->initialized = 0;
}
/*--------------------------------------------------------------------------*/
int32_t UdpServer_recv(struct UdpServerStruct *s,
		  	  	  	  	  	 uint8_t *buffer,
		  	  	  	  	  	 uint16_t num) {
    if (s->initialized) {
        int n = recvfrom(s->serverSocket, buffer, num, MSG_DONTWAIT,
                         (struct sockaddr*)&s->other, &s->clientLength);
        if (n ==-1) {
            return 0;
        }
        return n;
    }
    else {
        return 0;
    }
}
/*--------------------------------------------------------------------------*/
int32_t UdpServer_send(struct UdpServerStruct *s,
		  	  	  	  	     uint8_t *buffer,
		  	  	  	  	     uint16_t num) {
    if (s->initialized) {

        int n = sendto(s->serverSocket, buffer, num, 0,
                       (struct sockaddr*)&s->other, s->clientLength);
        if (n == -1) {
            return 0;
        }
        return n;
    }
    else {
        return 0;
    }
}
