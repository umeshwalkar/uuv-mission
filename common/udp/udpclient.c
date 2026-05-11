#include "udpclient.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>


void UdpClient_init(struct UdpClientStruct *s,
                           int8_t *serverIP,
                           uint16_t serverPort) {
    strcpy((char*)s->serverIPString, (char*)serverIP);
    s->serverPort = serverPort;
    s->clientLength = sizeof(s->clientAddress);
    if ((s->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
        printf("UdpServerClient_init(): err creating client socket\n");
    }

    bzero(&s->clientAddress, sizeof(s->clientAddress));
    s->clientAddress.sin_family = AF_INET;
    s->clientAddress.sin_port = htons(serverPort);
    if (inet_aton((char*)serverIP, &s->clientAddress.sin_addr) == 0) {
    	printf("UdpServerClient_init(): error creating connection to server\n");;
    }

    if ((s->clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
      printf("UdpServerClient_init(): error socket\n");
    }

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
