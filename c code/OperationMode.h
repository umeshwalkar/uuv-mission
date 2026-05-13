/*
 * OperationMode.h
 *
 *  Created on: Jan 5, 2023
 *      Author: root
 */

#ifndef OPERATIONMODE_H_
#define OPERATIONMODE_H_

/*# Includes*/
//#include <stdio.h>                     /* This ert_main.c example uses printf/fflush */
//
//#include "ahsGlobals.h"
//#include "ahsExterns.h"
//#include "ahsDefines.h"
//#include "ahsError.h"
//#include "strobelight.h"
//#include "buzzer.h"

typedef enum
{
	OP_MODE_STANDBY = 0,
	//	OP_MODE_ROV = 1,
	//	OP_MODE_AUV = 2,
	//	OP_MODE_DRIECT_ROV = 3,

	OP_MODE_MANUAL, /*OP_MODE_ROV*/
	OP_MODE_AUTONOMOUS, /*OP_MODE_AUV*/
	OP_MODE_SEMI_AUTONOMOUS, /*OP_MODE_DRIECT_ROV*/

	/*DO NOT CHANGE ABOVE SEQUENCE */

	OP_MODE_PRELAUNCH_TEST,
	OP_MODE_EMERGENCY,
	OP_MODE_LAUNCHING,
	OP_MODE_UNKNOWN
} OperationModes_t;

int OperationModeSet(OperationModes_t newMode);
OperationModes_t OperationModeGet();
const char* OperationModeGetName();
void OperationModeInit(void);

int ReadAutoNav_MissionFile(const char *filename);
int ReadHSDT_MissionFile(const char *filename);

#endif /* OPERATIONMODE_H_ */
