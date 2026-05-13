/*
 * interfaceExterns.h
 *
 *  Created on: May 31, 2023
 *      Author: root
 */

#ifndef INTERFACEEXTERNS_H_
#define INTERFACEEXTERNS_H_

#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string.h>
#include <ctype.h>
#include <float.h>
#include <time.h>
//#include <mathdef.h>
//#include <mathcalls.h>

//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/sem.h>
//#include <sys/shm.h>
//#include <sys/msg.h>
#include <sys/stat.h>

#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/ipc.h>
//#include <sys/sem.h>
//#include <sys/shm.h>
//#include <sys/msg.h>
//#include "defines.h"
//#include "posixExterns.h"
//#include "osSupport.h"
//#include "dotAIncludes.h"
//#include <sys/types.h>
//#include <stdlib.h>
//#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
//#include <netdb.h>
//#include <sys/select.h>
//#include <errno.h>
//#include <sys/time.h>
//#include <termios.h>
#include <sys/stat.h>
//#include <signal.h>
//#include <semaphore.h>
#include <pthread.h>

//#include <errno.h>

#include "interfaceDefines.h"

extern unsigned int sysSchedCount,sysSchedCountPrev;
extern timer_t mainSchedTimerId;
extern int baseSchedTimeInMillisec;
extern sem_t mainThreadSchedSem,mainThreadSchedSem2;

extern pthread_mutex_t logParams_mutex;

extern volatile InsData_t insData;
extern volatile SystemVars_t gSysVars;
extern SystemFlags_t gSysFlags;
extern SystemParams_t gSysParams;
extern ControlLoopsParams_t ctrlLoopParams;
extern volatile GuiFrameVuStatus_t GuiVuStaus;
extern volatile GuiTime_t GuiCurrentTime;
extern GoToWP_t gotowp;
extern HomeLocation_t homeLoc; // gcsLocation

extern HSDTpacket_t hsdtMission[MAX_HSDT_MISSION_POINTS];

#endif /* INTERFACEEXTERNS_H_ */
