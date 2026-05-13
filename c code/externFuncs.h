/*
 * externFuncs.h
 *
 *  Created on: May 31, 2023
 *      Author: root
 */

#ifndef EXTERNFUNCS_H_
#define EXTERNFUNCS_H_

#include <stdint.h>

extern void TaskSchdlInit();
extern void MatlabControlInit();
extern void MotusInit();
extern void SpatialReadInit();
extern void ThrusterInit();
extern void FinServoInit();
//extern void DigitalInputInit();
//extern void DigitalOutputInit();
extern void OperationModeInit();
extern void WriteLogInit();
extern void WiFiReadInit();
extern void WiFiWriteInit();

extern int WiFiWriteFramePushToQueue(uint8_t pktid, uint8_t *ctime, uint8_t * data, uint16_t dataLen);
extern int WriteLogEnable(uint8_t status);
extern void ThrusterPowerOn();
extern void ThrusterPowerOff();

extern void SwitchToGoToMode();
extern void ModifyGoToMode();
extern void SwitchToHoldMode();
extern void SwitchToHome();

extern void SatComInit();

#endif /* EXTERNFUNCS_H_ */
