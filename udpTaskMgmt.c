/*
 * udpTaskMgmt.c
 *
 *  UDP Task Management Implementation
 *  Created on: May 12, 2026
 */

#include "udpTaskMgmt.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "GuiWiFiFrames.hpp"

volatile GuiFrameVuStatus_t GuiVuStaus;
volatile GuiTime_t GuiCurrentTime;




/**
 * Calculate XOR checksum for packet validation
 */
uint8_t UdpPacket_calculateChecksum(uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint8_t ck = 0;
    
    if (!data || len == 0) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        ck ^= data[i];
    }
    
    return ck;
}

/**
 * Validate UDP packet structure
 * Expected format: [START_BYTE | PACKET_ID | LENGTH (2 bytes) | DATA | CHECKSUM | END_BYTE]
 */
int UdpPacket_validate(uint8_t *data, uint16_t len)
{
    if (!data || len < 6) {
        return -1;  // Invalid packet size
    }
    
    uint8_t startByte = data[0];
    uint8_t endByte = data[len - 1];
    
    // Check frame delimiters
    if (startByte != 0x24) {  // '$'
        printf("[UDP] Invalid start byte: 0x%02X\n", startByte);
        return -1;
    }
    
    if (endByte != 0x0A) {    // '\n'
        printf("[UDP] Invalid end byte: 0x%02X\n", endByte);
        return -1;
    }
    
    // Checksum is at len-2 position (before end byte)
    uint8_t receivedChecksum = data[len - 2];
    uint8_t calculatedChecksum = UdpPacket_calculateChecksum(data + 1, len - 3);
    
    if (receivedChecksum != calculatedChecksum) {
        printf("[UDP] Checksum mismatch: received=0x%02X, calculated=0x%02X\n", 
               receivedChecksum, calculatedChecksum);
        return -1;
    }
    
    return 0;  // Valid packet
}

/**
 * Initialize UDP Task Management structure
 */
int UdpTaskMgmt_init(UdpTaskMgmt_t *mgmt, 
                     struct UdpServerStruct *rxServer,
                     struct UdpClientStruct *txClient)
{
    if (!mgmt || !rxServer || !txClient) {
        printf("[UDP-TaskMgmt] Invalid parameters\n");
        return -1;
    }
    
    mgmt->rxServer = rxServer;
    mgmt->txClient = txClient;
    
    /* Initialize semaphores */
    if (sem_init(&mgmt->rxSem, 0, 0) == -1) {
        printf("[UDP-TaskMgmt] Failed to initialize RX semaphore\n");
        return -1;
    }
    
    if (sem_init(&mgmt->txSem, 0, 0) == -1) {
        printf("[UDP-TaskMgmt] Failed to initialize TX semaphore\n");
        sem_destroy(&mgmt->rxSem);
        return -1;
    }
    
    /* Initialize counters */
    mgmt->rxCount = 0;
    mgmt->txCount = 0;
    mgmt->rxErrors = 0;
    mgmt->txErrors = 0;
    
    /* Initialize status flags */
    mgmt->rxRunning = 1;
    mgmt->txRunning = 1;
    mgmt->taskRunning = 1;
    mgmt->taskExit = 0;
    
    /* Clear packet buffers */
    memset(&mgmt->rxPacket, 0, sizeof(UdpPacket_t));
    memset(&mgmt->txPacket, 0, sizeof(UdpPacket_t));
    
    printf("[UDP-TaskMgmt] Initialized successfully\n");
    return 0;
}

/**
 * Cleanup UDP Task Management
 */
int UdpTaskMgmt_deinit(UdpTaskMgmt_t *mgmt)
{
    if (!mgmt) {
        return -1;
    }
    
    /* Signal tasks to exit */
    mgmt->taskExit = 1;
    
    /* Post semaphores to wake up tasks */
    sem_post(&mgmt->rxSem);
    sem_post(&mgmt->txSem);
    
    /* Wait for threads to complete */
    if (mgmt->rxRunning) {
        pthread_join(mgmt->rxThreadId, NULL);
    }
    
    if (mgmt->txRunning) {
        pthread_join(mgmt->txThreadId, NULL);
    }
    
    /* Destroy semaphores */
    sem_destroy(&mgmt->rxSem);
    sem_destroy(&mgmt->txSem);
    
    printf("[UDP-TaskMgmt] Cleanup complete. RX: %u packets, TX: %u packets\n", 
           mgmt->rxCount, mgmt->txCount);
    printf("[UDP-TaskMgmt] Errors - RX: %u, TX: %u\n", mgmt->rxErrors, mgmt->txErrors);
    
    return 0;
}

/**
 * UDP RX Task - Receives and parses UDP packets
 * Runs in separate thread
 */
void* UdpRxTaskEntry(void *arg)
{
    UdpTaskMgmt_t *mgmt = (UdpTaskMgmt_t *)arg;
    int32_t bytesReceived;
    uint8_t buffer[1024];
    
    printf("[UDP-RxTask] Started on thread %ld\n", pthread_self());
    
    while (!mgmt->taskExit) {
        /* Try to receive data from server socket */
        bytesReceived = UdpServer_recv(mgmt->rxServer, buffer, sizeof(buffer));
        
        if (bytesReceived > 0) {
            printf("[UDP-RxTask] Received %d bytes\n", bytesReceived);
            
            /* Validate packet */
            if (UdpPacket_validate(buffer, bytesReceived) == 0) {
                /* Valid packet - store in shared structure */
                memcpy(mgmt->rxPacket.buffer, buffer, bytesReceived);
                mgmt->rxPacket.length = bytesReceived;
                mgmt->rxPacket.timestamp = time(NULL);
                mgmt->rxPacket.isValid = 1;
                
                /* Parse packet ID and log */
                uint8_t packetId = buffer[1];
                printf("[UDP-RxTask] Valid packet: ID=0x%02X, Length=%d\n", 
                       packetId, bytesReceived);
                
                mgmt->rxCount++;
                
                /* TODO: Parse packet based on ID and handle accordingly */
                
            } else {
                mgmt->rxErrors++;
                printf("[UDP-RxTask] Invalid packet rejected\n");
            }
        }
        
        /* Sleep to prevent busy waiting */
        usleep(10000);  // 10ms
    }
    
    printf("[UDP-RxTask] Exiting...\n");
    mgmt->rxRunning = 0;
    return NULL;
}

/**
 * UDP TX Task - Periodically sends status and acknowledgment packets
 * Runs in separate thread
 */
void* UdpTxTaskEntry(void *arg)
{
    UdpTaskMgmt_t *mgmt = (UdpTaskMgmt_t *)arg;
    static uint32_t txCounter = 0;
    uint32_t lastTxTime = 0;
    uint32_t currentTime;
    uint8_t txBuffer[256];
    uint16_t dataLen;
    int32_t bytesSent;
    
    printf("[UDP-TxTask] Started on thread %ld\n", pthread_self());
    
    while (!mgmt->taskExit) {
        currentTime = time(NULL);
        
        /* Send status packet every 1 second */
        if ((currentTime - lastTxTime) >= 5) {
            lastTxTime = currentTime;
            
            /* Build status packet: $[ID][Length][Data][Checksum]\n */
            txBuffer[0] = 0x24;         // START_BYTE '$'
            txBuffer[1] = 0xA1;         // PACKET_ID (Status Packet)
            
            /* Build data payload */
            uint8_t *dataPtr = &txBuffer[2];
            dataLen = 0;
            
            /* Length placeholder (will update after data) */
            uint16_t *lenPtr = (uint16_t *)dataPtr;
            dataPtr += 2;
            dataLen += 2;
            
            /* Payload: TX counter and RX counter */
            uint32_t *txCountPtr = (uint32_t *)dataPtr;
            *txCountPtr = txCounter++;
            dataPtr += 4;
            dataLen += 4;
            
            uint32_t *rxCountPtr = (uint32_t *)dataPtr;
            *rxCountPtr = mgmt->rxCount;
            dataPtr += 4;
            dataLen += 4;
            
            /* Update length field */
            *lenPtr = dataLen;
            
            /* Calculate and add checksum */
            uint8_t checksum = UdpPacket_calculateChecksum(txBuffer + 1, 1 + dataLen);
            txBuffer[1 + 1 + dataLen] = checksum;
            
            /* Add END_BYTE */
            txBuffer[1 + 1 + dataLen + 1] = 0x0A;  // END_BYTE '\n'
            
            uint16_t totalLen = 1 + 1 + dataLen + 1 + 1;  // START + ID + DATA + CHECKSUM + END
            
            /* Send packet */
            bytesSent = UdpClient_send(mgmt->txClient, txBuffer, totalLen);
            
            if (bytesSent > 0) {
                printf("[UDP-TxTask] Sent status packet: %d bytes (TX Count: %u, RX Count: %u)\n", 
                       bytesSent, txCounter - 1, mgmt->rxCount);
                mgmt->txCount++;
            } else {
                mgmt->txErrors++;
                printf("[UDP-TxTask] Failed to send packet\n");
            }
        }
        
        /* TODO: Send acknowledgment packet if rxPacket.isValid is set */
        if (mgmt->rxPacket.isValid) {
            /* Build and send ACK packet */
            printf("[UDP-TxTask] Sending ACK for received packet\n");
            
            txBuffer[0] = 0x24;         // START_BYTE
            txBuffer[1] = 0xA2;         // PACKET_ID (ACK)
            txBuffer[2] = 0x02;         // Length (2 bytes)
            txBuffer[3] = 0x00;
            txBuffer[4] = 0x01;         // ACK status (1 = success)
            txBuffer[5] = UdpPacket_calculateChecksum(txBuffer + 1, 4);  // Checksum
            txBuffer[6] = 0x0A;         // END_BYTE
            
            bytesSent = UdpClient_send(mgmt->txClient, txBuffer, 7);
            if (bytesSent > 0) {
                mgmt->txCount++;
            } else {
                mgmt->txErrors++;
            }
            
            mgmt->rxPacket.isValid = 0;  // Clear the flag
        }
        
        /* Sleep to prevent busy waiting */
        usleep(100000);  // 100ms
    }
    
    printf("[UDP-TxTask] Exiting...\n");
    mgmt->txRunning = 0;
    return NULL;
}
