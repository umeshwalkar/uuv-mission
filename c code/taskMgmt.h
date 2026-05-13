/*
 * taskMgmt.h
 *
 *  Created on: Jun 21, 2023
 *      Author: root
 */

#ifndef TASKMGMT_H_
#define TASKMGMT_H_

#include <stdint.h>

typedef volatile unsigned  int  TICKS;

extern TICKS schedulerTick;

extern void errExit(const char* msg);
extern int TaskCreate(void(*entryPoint)(void), uint16_t priority, uint16_t interval_msec, int8_t *handler);

extern int semCreate(int8_t taskHandler);
extern int semTake(int8_t taskHandler);
extern int semGive(int8_t taskHandler);

#define NUM_OF_THREADS  		30
#define MAIN_SCHED_TMR_SIG  	SIGRTMIN
#define BASE_SCHEDULE_INTERVAL	10 // milli sec
#define GetTaskTick()			schedulerTick
#define GetTaskTickDiff(a)	(schedulerTick - a)

#define WAIT_SEC(sec)			(sec*100) //(sec*1000/10)
#define WAIT_HALF_SEC			50
#define WAIT_1_SEC				100
#define WAIT_2_SEC				200
#define WAIT_3_SEC				300
#define WAIT_4_SEC				400
#define WAIT_5_SEC				500
#define WAIT_6_SEC				600
#define WAIT_7_SEC				700
#define WAIT_8_SEC				800
#define WAIT_9_SEC				900
#define WAIT_10_SEC				1000
#define WAIT_15_SEC				1500

#endif /* TASKMGMT_H_ */
