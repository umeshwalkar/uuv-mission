#include "udpclient.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>


void UdpClient_init(struct UdpClientStruct *s,
                           int8_t *serverIP,
                           uint16_t serverPort) {
    if (!s || !serverIP) {
        printf("UdpClient_init(): Invalid parameters\n");
        return;
    }
    
    strcpy((char*)s->serverIPString, (char*)serverIP);
    s->serverPort = serverPort;
    s->clientLength = sizeof(s->clientAddress);
    
    // Create the socket
    if ((s->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("UdpClient_init(): Error creating client socket\n");
        s->initialized = 0;
        return;
    }

    // Initialize client address structure
    bzero(&s->clientAddress, sizeof(s->clientAddress));
    s->clientAddress.sin_family = AF_INET;
    s->clientAddress.sin_port = htons(serverPort);
    
    // Try to parse as IP address first
    if (inet_aton((char*)serverIP, &s->clientAddress.sin_addr) == 0) {
        // Not a valid IP address, try hostname resolution via DNS
        printf("UdpClient_init(): Resolving hostname '%s' via DNS...\n", (char*)serverIP);
        struct addrinfo hints, *res;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;
        
        int status = getaddrinfo((char*)serverIP, NULL, &hints, &res);
        if (status != 0) {
            printf("UdpClient_init(): DNS resolution failed for '%s': %s\n", 
                   (char*)serverIP, gai_strerror(status));
            close(s->clientSocket);
            s->initialized = 0;
            return;
        }
        
        // Copy the resolved address
        s->clientAddress.sin_addr = ((struct sockaddr_in*)res->ai_addr)->sin_addr;
        freeaddrinfo(res);
        printf("UdpClient_init(): Hostname resolved successfully\n");
    }

    printf("UdpClient_init(): Initialized for %s:%d\n", (char*)serverIP, serverPort);
    s->initialized = 1;
}
void UdpClient_deinit(struct UdpClientStruct *s) {
	close(s->clientSocket);
    s->initialized = 0;
}
/*--------------------------------------------------------------------------*/
int32_t UdpClient_recv(struct UdpClientStruct *s,
		  	  	  	  	  	 uint8_t *buffer,
		  	  	  	  	  	 uint16_t num) {
    if (s->initialized) {
        int n = recvfrom(s->clientSocket, buffer, num, MSG_DONTWAIT,
                         (struct sockaddr*)&s->clientAddress, &s->clientLength);
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
int32_t UdpClient_send(struct UdpClientStruct *s,
		  	  	  	  	     uint8_t *buffer,
		  	  	  	  	     uint16_t num) {
    if (s->initialized) {

        int n = sendto(s->clientSocket, buffer, num, 0,
                       (struct sockaddr*)&s->clientAddress, s->clientLength);
        if (n == -1) {
            return 0;
        }
        return n;
    }
    else {
        return 0;
    }
}
