/*
 * udpTaskMgmt.h
 *
 *  UDP Task Management for receive and transmit tasks
 *  Created on: May 12, 2026
 */

#ifndef UDP_TASK_MGMT_H_
#define UDP_TASK_MGMT_H_

#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include "common/udp/udpserver.h"
#include "common/udp/udpclient.h"

#ifdef __cplusplus
extern "C" {
#endif

/* UDP Packet structure for RX/TX */
typedef struct {
    uint8_t buffer[1024];
    uint16_t length;
    uint32_t timestamp;
    uint8_t isValid;
} UdpPacket_t;

/* Shared data structure between RX and TX tasks */
typedef struct {
    /* UDP networking */
    struct UdpServerStruct *rxServer;
    struct UdpClientStruct *txClient;
    
    /* RX Task data */
    UdpPacket_t rxPacket;
    sem_t rxSem;
    pthread_t rxThreadId;
    uint8_t rxRunning;
    
    /* TX Task data */
    UdpPacket_t txPacket;
    sem_t txSem;
    pthread_t txThreadId;
    uint8_t txRunning;
    
    /* Counters and status */
    uint32_t rxCount;
    uint32_t txCount;
    uint32_t rxErrors;
    uint32_t txErrors;
    
    /* Control flags */
    volatile uint8_t taskRunning;
    volatile uint8_t taskExit;
    
} UdpTaskMgmt_t;

/* Task entry points - to be called from mission_manager */
void* UdpRxTaskEntry(void *arg);
void* UdpTxTaskEntry(void *arg);

/* Task initialization */
int UdpTaskMgmt_init(UdpTaskMgmt_t *mgmt, 
                     struct UdpServerStruct *rxServer,
                     struct UdpClientStruct *txClient);

/* Task cleanup */
int UdpTaskMgmt_deinit(UdpTaskMgmt_t *mgmt);

/* Utility functions */
uint8_t UdpPacket_calculateChecksum(uint8_t *data, uint16_t len);
int UdpPacket_validate(uint8_t *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* UDP_TASK_MGMT_H_ */
