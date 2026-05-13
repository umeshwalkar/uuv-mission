/*
 * interfaceGlobals.h
 *
 *  Created on: May 31, 2023
 *      Author: root
 */

#ifndef INTERFACEGLOBALS_H_
#define INTERFACEGLOBALS_H_

#include <semaphore.h>
#include <stdint.h>

#include "interfaceDefines.h"


//unsigned int sysSchedCount = 0,sysSchedCountPrev=0;
//timer_t mainSchedTimerId;
//int baseSchedTimeInMillisec = 10;
//sem_t mainThreadSchedSem,mainThreadSchedSem2;
pthread_mutex_t logParams_mutex;

volatile InsData_t insData;
volatile SystemVars_t gSysVars;
SystemFlags_t gSysFlags;
SystemParams_t gSysParams;
ControlLoopsParams_t ctrlLoopParams;
GoToWP_t gotowp;
HomeLocation_t homeLoc; // gcsLocation
volatile GuiFrameVuStatus_t GuiVuStaus;
volatile GuiTime_t GuiCurrentTime;
//sFinData finServoRx[MAX_SERVO];
//sThrusterData thrusterRx;
HSDTpacket_t hsdtMission[MAX_HSDT_MISSION_POINTS];

#endif /* INTERFACEGLOBALS_H_ */
