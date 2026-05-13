/*
 * WiFiWrite.c
 *
 *  Created on: Nov 9, 2023
 *      Author: root
 */

#include "CommonIncludes.h"
#include "GuiWiFiFrames.h"
//#include "WiFiWrite.h"

// comment if debug not required
//#define WIFI_DEBUG(...) printf(__VA_ARGS__)

#ifndef WIFI_DEBUG
#define WIFI_DEBUG(...)
#endif

#define CONSOLE_TX_BUF_SIZE	512
#define MAX_TX_FRAME_QUEUE_SIZE	15

#pragma pack (push,1)
typedef union
__attribute__((packed))
{
	unsigned char array[150];
	struct __attribute__((packed))
	{
		unsigned char sof;
		unsigned char packetId;
		GuiTime_t time;
		uint16_t length; // data length
		unsigned char data[128];
		unsigned char checksum;
		unsigned char eof;
	};
} TxFrame_t;

typedef struct
__attribute__((packed))
{
	unsigned char packetId;
	GuiTime_t time;
	uint16_t length; // data length
	unsigned char data[128];
	unsigned char isValid;
} FrameQueue_t;
#pragma pack(pop)

FrameQueue_t txFrameQueue[MAX_TX_FRAME_QUEUE_SIZE];
TxFrame_t txFrame;
static int8_t _taskHandler = -1;

int WiFiWriteFramePushToQueue(uint8_t pktid, uint8_t *ctime, uint8_t * data,
		uint16_t dataLen)
{
	int i;

	//	printf("[CNSL-W] pktid=%d, pkdLen=%d\n", pktid, dataLen);

	for (i = 0; i < MAX_TX_FRAME_QUEUE_SIZE; i++)
	{
		if (txFrameQueue[i].isValid == 0)
		{

			txFrameQueue[i].packetId = pktid;
			txFrameQueue[i].length = dataLen;
			//txFrameQueue[i].time.val = time;
			if (ctime != NULL)
			{
				bcopy(ctime, txFrameQueue[i].time.bytes, 8);
			}
			else
			{
				time_t t = time(NULL); // get time in seconds
				txFrameQueue[i].time.val = (t * 1000); // in milliseconds
			}

			if (dataLen < 128)
			{
				if (data != NULL)
					bcopy(data, txFrameQueue[i].data, dataLen);
				txFrameQueue[i].isValid = 1;
			}

			return i;
		}
	}

	printf("[CNSL-W] write queue is full\n");

	return -1;
}

static uint8_t CalculateChecksum(uint8_t *arr, uint16_t len)
{
	uint16_t i;
	uint8_t ck = arr[0];

	for (i = 1; i < len; i++)
	{
		ck = ck ^ arr[i];
	}

	return ck;

}

void ConsoleWrite(uint8_t *frame, uint16_t length)
{
	if (cansole_ctrl.write(serFdconsole, (uint8_t*) frame, length) == ERROR) //
	{
		printf("[WIFI-W] Write failed!\n");
	}
}

static void UpdateVuStatusData()
{
	// few values are not reflecting properly.
	// Note that on original GUI this packet length was considered 71 bytes.
	// but in Manual it is described 69 bytes.
	// so made changes in original GUI software and considered 69 bytes as packet length.

	// GUI required data in BIG ENDIAN format.
	// MSB need to be send first.
	int32_t dummy32;
	int16_t dummy16;
	uint16_t dummy16u;
	//	static InsData_t insData;

	WIFI_DEBUG("\n");

	GuiVuStaus.VuState = OperationModeGet();
	GuiVuStaus.MissionOperationId = 0; //65535;

	// flags bitwise orientation need to check for the big endianess.
	// I suggest to make change in union/struct for defining variable in order bit0-7 or bit7-0
	if (gSysVars.bmsData.linkStatus == 1)
		GuiVuStaus.VuStatusB0.BatteryComm = 1; // set if able to communicate with BMS

	//	if (gSysVars.insSpatial.linkStatus) // do logical ORing between comm. module on MCU board
	GuiVuStaus.VuStatusB0.LB_Comm = 1; // set if able to communicate with MCU board

	GuiVuStaus.VuStatusB0.Carris_Comm = 1; // do not know what this device is

	//	if (gSysVars.insMotus.linkStatus)
	GuiVuStaus.VuStatusB0.NCU_Comm = 1; // set if able to communicate with MCU board, receiving INS data

	GuiVuStaus.VuStatusB0.LogFileActive = gSysFlags.startLoggingFlag; // set if able Logger is started

	// these flags are set/reset in their respective read module task
	//	GuiVuStaus.VuStatusB1.WifiActive = 0;
	//	GuiVuStaus.VuStatusB1.WifiCommOk = 0;
	//	GuiVuStaus.VuStatusB1.RfActive = 0;
	//	GuiVuStaus.VuStatusB1.rfCommOk = 0;
	//	GuiVuStaus.VuStatusB1.UsblActive = 0;
	//	GuiVuStaus.VuStatusB1.UsblCommOk = 0;
	//	GuiVuStaus.VuStatusB1.SatActive = 0;
	//	GuiVuStaus.VuStatusB1.SatCommOk = 0;

	//=====================================================================================
	GuiVuStaus.LbStatusB0.MonSafetySwitch = 1; // not present, active low
	GuiVuStaus.LbStatusB0.AisEmSwitch = 0; // active high
	/*'Battery Charger Present' label Highlighted on GCS only when BatteryCharger set to 1*/
	GuiVuStaus.LbStatusB0.BatteryCharger = 0;
	/*'Emergency Active' label Highlighted on GCS only when MonKWD is set to 1*/
	GuiVuStaus.LbStatusB0.MonKWD = gSysFlags.dropweightEnable; // set when dropweight is triggered
	GuiVuStaus.LbStatusB0.Reserved_1 = 0;
	/*LbStatusB0 bit 5,6 and 7 are active low*/
	GuiVuStaus.LbStatusB0.VuSignal = 0; // reset if able to communicate with MCU board
	GuiVuStaus.LbStatusB0.NcuSignal = 0; // reset if able to communicate with MCU board
	GuiVuStaus.LbStatusB0.CarrisSignal = 0; // reset if able to communicate with MCU board

	GuiVuStaus.LbStatusB1.DnSwitch_1 = 0;
	GuiVuStaus.LbStatusB1.DnSwitch_2 = 0;
	GuiVuStaus.LbStatusB1.BuoyancyPosition_Ok = 0;
	GuiVuStaus.LbStatusB1.Buoyancy_Fault = 0;
	GuiVuStaus.LbStatusB1.DW_Switch2 = 0;
	GuiVuStaus.LbStatusB1.FloodSwitch = gSysFlags.leakDetected;
	GuiVuStaus.LbStatusB1.FailNavigationSwitch = 0;
	GuiVuStaus.LbStatusB1.FailScientificSwitch = 0;

	//=====================================================================================
	GuiVuStaus.CarrisStatusB0.MBES_Sensor = 0;
	GuiVuStaus.CarrisStatusB0.SSS_Sensor = 0;
	GuiVuStaus.CarrisStatusB1.SBP_Sensor = 0;
	GuiVuStaus.CarrisStatusB1.Camera_Sensor = 0;
	GuiVuStaus.CarrisStatusB1.CTD_Sensor = 0;

	//=====================================================================================
	// NCU B0 to B5 is not implemented
	//	GuiVuStaus.NcuStatusB0.Val = 0;
	//	GuiVuStaus.NcuStatusB1ThrusterWarn.Val = 0;
	//	GuiVuStaus.NcuStatusB2CommWarn.Val = 0;
	//	GuiVuStaus.NcuStatusB3GuidanceWarn.Val = 0;
	//	GuiVuStaus.NcuStatusB4.Val = 0;
	//	GuiVuStaus.NcuStatusB5GuidanceStatus.Val = 0;

	if ((gSysVars.thrusterData.linkStatus == 1) && (gSysVars.thrusterData.error
			== 0))
	{
		GuiVuStaus.NcuStatusB0.ThrusterWarning = 1;
	}
	else
	{
		GuiVuStaus.NcuStatusB0.ThrusterWarning = 0;
	}

	GuiVuStaus.NcuStatusB0.CommunicationWarning = 1;
	GuiVuStaus.NcuStatusB0.GuidanceWarning = 1;

	if (GuiVuStaus.NcuStatusB0.ThrusterWarning)
	{
		GuiVuStaus.NcuStatusB1ThrusterWarn.BackSX = 1;
		GuiVuStaus.NcuStatusB1ThrusterWarn.BackDX = 1;
		GuiVuStaus.NcuStatusB1ThrusterWarn.BackSide = 1;
		GuiVuStaus.NcuStatusB1ThrusterWarn.Backtop = 1;
		GuiVuStaus.NcuStatusB1ThrusterWarn.FrontSide = 1;
		GuiVuStaus.NcuStatusB1ThrusterWarn.FrontTop = 1;
	}
	else
	{
		GuiVuStaus.NcuStatusB1ThrusterWarn.Val = 0;
	}

#if USE_MOTUS_READINGS
	if (gSysVars.insMotus.nonOperationalError == TRUE) // gSysVars.insMotus.linkStatus==FALSE
#else
	if (gSysVars.insSpatial.nonOperationalError == TRUE)
#endif
	{
		GuiVuStaus.NcuStatusB2CommWarn.INS_Comm = 0;
	}
	else
	{
		GuiVuStaus.NcuStatusB2CommWarn.INS_Comm = 1;
	}

	if (1) // (gSysVars.insSpatial.linkStatus) // do logical ORing between comm. module on MCU board
	{
		GuiVuStaus.NcuStatusB2CommWarn.LogicBoardComm = 1;
		GuiVuStaus.NcuStatusB2CommWarn.VitalUnitComm = 1;
	}
	else
	{
		GuiVuStaus.NcuStatusB2CommWarn.LogicBoardComm = 0;
		GuiVuStaus.NcuStatusB2CommWarn.VitalUnitComm = 0;
	}

	if (gSysVars.altimeterData.linkStatus)
	{
		GuiVuStaus.NcuStatusB2CommWarn.EchoBottomComm = 1;
		GuiVuStaus.NcuStatusB2CommWarn.EchoFrontComm = 1;
		GuiVuStaus.NcuStatusB2CommWarn.EchoSlopedComm
				= gSysVars.altimeterData.confidanceLow;
	}
	else
	{
		GuiVuStaus.NcuStatusB2CommWarn.EchoBottomComm = 0;
		GuiVuStaus.NcuStatusB2CommWarn.EchoFrontComm = 0;
		GuiVuStaus.NcuStatusB2CommWarn.EchoSlopedComm = 0;
	}

	// not handled flags
	//	GuiVuStaus.NcuStatusB3GuidanceWarn.Val = 0;
	GuiVuStaus.NcuStatusB4.ObstacleAvoidStatusCode = 0;

	GuiVuStaus.NcuStatusB5GuidanceStatus.DepthMeasuresComparison = 0; // logical ORing flag

	if (GuiVuStaus.VuState == OP_MODE_MANUAL)
	{
		GuiVuStaus.NcuStatusB5GuidanceStatus.GuidanceModeCode = 1;
	}
	else
	{
		GuiVuStaus.NcuStatusB5GuidanceStatus.GuidanceModeCode = 0;
	}

	GuiVuStaus.NcuStatusB67.Val = 0xFFFF;
	//=====================================================================================

	if (!insData.system_state_packet.system_status.b.system_failure)
		GuiVuStaus.NcuStatusB67.SolutionValidity = 0;

	if (!insData.system_state_packet.system_status.b.accelerometer_sensor_failure)
		GuiVuStaus.NcuStatusB67.AccelerometerFail = 0;

	if (!insData.system_state_packet.system_status.b.gyroscope_sensor_failure)
		GuiVuStaus.NcuStatusB67.GyroFail = 0;

	if (!insData.system_state_packet.system_status.b.accelerometer_over_range)
		GuiVuStaus.NcuStatusB67.AccelerometerOutrancge = 0;

	if (!insData.system_state_packet.system_status.b.gyroscope_over_range)
		GuiVuStaus.NcuStatusB67.GyrosOutrange = 0;

	if (insData.system_state_packet.filter_status.b.gnss_fix_type > 0)
		GuiVuStaus.NcuStatusB67.GPS_Validity = 0;

	// testing NCU flag
	//	WIFI_DEBUG("NcuStatusB67=%X\n",GuiVuStaus.NcuStatusB67.Val);

	//=====================================================================================
	WIFI_DEBUG("[WIFI-W] GPS fix(Spatial)=%d, Latitude=%f Longitude=%f\n",(gSysVars.insSpatial.system_state_packet.filter_status.b.gnss_fix_type),
			(insData.system_state_packet.latitude * RAD_TO_DEG),
			(insData.system_state_packet.longitude * RAD_TO_DEG));
	dummy32 = (int32_t) ((insData.system_state_packet.latitude * RAD_TO_DEG)
			* 10000000L);
	endianConverter((uint8_t*) &dummy32, (uint8_t*) &GuiVuStaus.Lattitude, 4);

	dummy32 = (int32_t) ((insData.system_state_packet.longitude * RAD_TO_DEG)
			* 10000000L);
	endianConverter((uint8_t*) &dummy32, (uint8_t*) &GuiVuStaus.Longitude, 4);

	//=====================================================================================
	// ToDo: confirm this parameter INS-Height or presureSensor-depth
	// if pressureSensorDepth value writing into Motus then consider MotusHeight
	// else consider pressureSensorDepth value to send to GCS
#if 0
	WIFI_DEBUG("[WIFI-W] depth=%.2f\n", (insData.system_state_packet.height));
	dummy32 = (int32_t) (insData.system_state_packet.height * 100U);
#else
	WIFI_DEBUG("[WIFI-W] depth=%.2f\n", (gSysVars.pressureData.depth));
	dummy32 = (int32_t) (gSysVars.pressureData.depth * 100);
#endif
	endianConverter((uint8_t*) &dummy32, (uint8_t*) &GuiVuStaus.Depth, 4);

	//=====================================================================================
	WIFI_DEBUG("[WIFI-W] Acceleration X=%f, Y=%f, Z=%f\n",(insData.system_state_packet.velocity[0]
			),(insData.system_state_packet.velocity[1]),(insData.system_state_packet.velocity[2]));

	dummy16 = (int16_t) (insData.system_state_packet.velocity[0] * 100);
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.SpeedBody_X, 2);

	dummy16 = (int16_t) (insData.system_state_packet.velocity[1] * 100);
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.SpeedBody_Y, 2);

	dummy16 = (int16_t) (insData.system_state_packet.velocity[2] * 100);
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.SpeedBody_Z, 2);

	//ToDo: check INS Packet id 36 and 37 for velocity
#if MAT_OPERATION_SWITCH
	float insSpeed = dghSysData.velocityIMU;
#else
	float insSpeed = sqrtf((insData.system_state_packet.velocity[0]
			* insData.system_state_packet.velocity[0])
			+ (insData.system_state_packet.velocity[1]
					* insData.system_state_packet.velocity[1])
			+ (insData.system_state_packet.velocity[2]
					* insData.system_state_packet.velocity[2]));
#endif
	dummy16 = (int16_t) (insSpeed * 100);

	//	dummy16	= (int16_t) (insData.system_state_packet.velocity[0] * 100); // north m/s
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.Speed, 2);

	//=====================================================================================
	WIFI_DEBUG("[WIFI-W] Roll=%f, Pitch=%f, Yaw=%f\n",(insData.system_state_packet.orientation[0]
			),(insData.system_state_packet.orientation[1]),(insData.system_state_packet.orientation[2]));

	dummy16 = (int16_t) ((insData.system_state_packet.orientation[0]
			* RAD_TO_DEG * 100U));
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.RollAngle, 2);

	dummy16 = (int16_t) ((insData.system_state_packet.orientation[1]
			* RAD_TO_DEG * 100U));
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.PitchAngle, 2);

	dummy16 = (uint16_t) ((insData.system_state_packet.orientation[2]
			* RAD_TO_DEG * 100)); // * 10000U
	endianConverter((uint8_t*) &dummy16, (uint8_t*) &GuiVuStaus.YawAngle, 2);

	//	printf("[WIFI-W] Yaw=%d\n",dummy16);

	//=====================================================================================
	GuiVuStaus.AltimeterFront = (uint8_t) (gSysVars.pressureData.pressure);
	//			(gSysVars.altimeterData.distance / 1000); // convert to millimeter to meter.
	GuiVuStaus.AltimeterBottom = (uint8_t) (gSysVars.altimeterData.distance_gcs
			/ 10);
	GuiVuStaus.AltimeterSloped
			= (uint8_t) gSysVars.altimeterData.confidanceLevel; //(gSysVars.altimeterData.distance / 100);

	if (GuiVuStaus.VuStatusB0.LB_Comm == 1)
	{
		//		dummy16 = (uint16_t) (gSysVars.bmsData.voltage); // * 100
		//		endianConverter((uint8_t*) &dummy16,
		//				(uint8_t*) &GuiVuStaus.BatteryVoltage_LB, 2);

		//		GuiVuStaus.LbStatusB0.BatteryCharger = 1;
	}
	else
	{
		GuiVuStaus.BatteryVoltage_LB = 0;
		GuiVuStaus.LbStatusB0.Val = 0;
		GuiVuStaus.LbStatusB1.Val = 0;
	}

	if (GuiVuStaus.VuStatusB0.Carris_Comm == 1)
	{

	}
	else
	{
		GuiVuStaus.CarrisStatusB0.Val = 0;
		GuiVuStaus.CarrisStatusB1.Val = 0;
	}

	if (GuiVuStaus.VuStatusB0.NCU_Comm == 1)
	{

	}
	else
	{

	}

	//	if (GuiVuStaus.VuStatusB0.BatteryComm == 1)
	//	{

	//=====================================================================================
	WIFI_DEBUG("[WIFI-W] Batt Volt=%d, Current=%d, Capacity=%d, SOC=%d\n",
			(gSysVars.bmsData.voltage), gSysVars.bmsData.avgCurrent,
			gSysVars.bmsData.remainBatteryCapacity, gSysVars.bmsData.absoluteSOC);

	dummy16u = (uint16_t) (gSysVars.bmsData.voltage); // * 100
	endianConverter((uint8_t*) &dummy16u,
			(uint8_t*) &GuiVuStaus.BatteryVoltage, 2);

	dummy16u = (uint16_t) (gSysVars.bmsData.voltage / 10); // * 100
	endianConverter((uint8_t*) &dummy16u,
			(uint8_t*) &GuiVuStaus.BatteryVoltage_LB, 2);

	dummy16u = (uint16_t) (gSysVars.bmsData.avgCurrent) * 100;
	endianConverter((uint8_t*) &dummy16u,
			(uint8_t*) &GuiVuStaus.BatteryCurrent, 2);

	// this parameter is not reflecting properly

	dummy32 = (uint32_t) ((gSysVars.bmsData.remainBatteryCapacity)); // * 100
	endianConverter((uint8_t*) &dummy32,
			(uint8_t*) &GuiVuStaus.BatteryCapacityRemain, 4);

	// this parameter is not reflecting properly
	GuiVuStaus.BatteryCharge = (uint8_t) (gSysVars.bmsData.absoluteSOC); //99;

	//	}
	//	else
	//	{
	//
	//	}

	//	GuiVuStaus.VuStatusB0.val = 0; //0xFF; // all ok
	//	GuiVuStaus.VuStatusB1.val = 0; // 0x03;

	//	GuiVuStaus.array,
}

void WiFiWriteTask()
{
	static uint8_t writeBuffer[CONSOLE_TX_BUF_SIZE];
	//	int i;

	if (!semCreate(_taskHandler) == -1)
	{
		printf("Failed! semCreate WiFiWriteTask\n");
		return; //ERROR;
	}

	writeBuffer[0] = 0x24;
	writeBuffer[1] = 0x9D;
	writeBuffer[2] = 0x00;
	writeBuffer[3] = 0x00;
	writeBuffer[4] = 0x00;
	writeBuffer[5] = 0x0;
	writeBuffer[6] = 0x0;
	writeBuffer[7] = 0x0;
	writeBuffer[8] = 0x0;
	writeBuffer[9] = 0x0;
	writeBuffer[10] = 0x0;
	writeBuffer[11] = 0x0F;
	writeBuffer[12] = 0x01;
	writeBuffer[13] = 0x93;
	writeBuffer[14] = 0x0A;

	ConsoleWrite(writeBuffer, 15);

	DBG_MSG("\n\n");
	DBG_MSG("===WiFiWriteTask started===\n");

	while (1)
	{
		if (semTake(_taskHandler) == -1)
		{
			printf("[%d] WiFiWriteTask exited\n", GetTaskTick());
			return;
		}

		int i;
		for (i = 0; i < MAX_TX_FRAME_QUEUE_SIZE; i++)
		{
			if (txFrameQueue[i].isValid == 1)
			{
				// no matter whether packet sending success or fail, release this slot
				txFrameQueue[i].isValid = 0;

#if 0
				// consider this when we do not know probable write packet data size
				uint8_t txBuff[txFrameQueue[i].length + 14];
				int j = 0;

				txBuff[j++] = GUI_START_MSG_VALUE;
				txBuff[j++] = txFrameQueue[i].packetId;
				txBuff[j++] = txFrameQueue[i].time.bytes[7];
				txBuff[j++] = txFrameQueue[i].time.bytes[6];
				txBuff[j++] = txFrameQueue[i].time.bytes[5];
				txBuff[j++] = txFrameQueue[i].time.bytes[4];
				txBuff[j++] = txFrameQueue[i].time.bytes[3];
				txBuff[j++] = txFrameQueue[i].time.bytes[2];
				txBuff[j++] = txFrameQueue[i].time.bytes[1];
				txBuff[j++] = txFrameQueue[i].time.bytes[0];
				txBuff[j++] = txFrameQueue[i].time.bytes[0];

				// copy the datalen, data, checksum, eof

#else
				bcopy(txFrameQueue[i].data, &txFrame.data,
						txFrameQueue[i].length);

				endianConverter((unsigned char*) &txFrameQueue[i].time.bytes,
						(unsigned char*) &txFrame.time.bytes, 8);

				txFrameQueue[i].length += 14; // +14 considering header and footer
				endianConverter((unsigned char*) &txFrameQueue[i].length,
						(unsigned char*) &txFrame.length, 2);

				txFrame.packetId = txFrameQueue[i].packetId;

				txFrame.checksum = CalculateChecksum(&txFrame.array[1],
						(txFrameQueue[i].length - 3));

				txFrame.sof = GUI_START_MSG_VALUE;
				txFrame.eof = GUI_END_MSG_VALUE;

				txFrame.array[txFrameQueue[i].length - 2] = txFrame.checksum;
				txFrame.array[txFrameQueue[i].length - 1] = txFrame.eof;

				//				printf("\n[WIFI-W] len=%d,%d \n", txFrame.length,
				//						txFrameQueue[i].length);
				//				printf("[WIFI-W] ck=%02X,%Ld \n", txFrame.checksum,
				//						txFrame.time.val);

#if 0 // print raw packet
				printf("[WIFI-W] [%d] Tx=> ", GetTaskTick());
				int j;
				for (j = 0; j < txFrameQueue[i].length; j++)
				{
					printf("%02X ", txFrame.array[j]);
				}
				printf("\n");
#endif

				ConsoleWrite(txFrame.array, txFrameQueue[i].length);
#endif

			}
		}

		/* send periodically system status and parmeter values, (SyReq_21)*/
		if (GetTaskTick() % WAIT_5_SEC == 0)
		{
			// send status packet every 5seconds
			//			printf("GuiVuStaus size= %d\n", sizeof(GuiVuStaus));

			//			GuiCurrentTime.val += 5000; // increment with 5000 msec

			// update data
			UpdateVuStatusData();

			uint16_t pakcetLen = sizeof(GuiFrameVuStatus_t)
					+ GUI_FRAME_OVERHEAD_BYTES;
			//+ 2; // +2 added because GUI expecting 71bytes whereas document says 69bytes

			txFrame.packetId = packet_id_vu_status_pck;
			txFrame.sof = GUI_START_MSG_VALUE;
			txFrame.eof = GUI_END_MSG_VALUE;

			endianConverter((unsigned char*) &GuiCurrentTime.bytes,
					(unsigned char*) &txFrame.time.bytes, 8);
			//			txFrame.time.val = 0;

			endianConverter((unsigned char*) &pakcetLen,
					(unsigned char*) &txFrame.length, 2);

			endianConverter((unsigned char*) &pakcetLen,
					(unsigned char*) &txFrame.length, 2);

			bcopy((uint8_t*) &GuiVuStaus, (uint8_t*) &txFrame.data,
					sizeof(GuiVuStaus));

			txFrame.checksum = CalculateChecksum(&txFrame.array[1], (pakcetLen
					- 3));

			txFrame.array[pakcetLen - 2] = txFrame.checksum;
			txFrame.array[pakcetLen - 1] = txFrame.eof;

			// values are not reflecting properly
			/*if ((GuiVuStaus.VuState == OP_MODE_MANUAL)
			 && (GuiVuStaus.NcuStatusB3GuidanceWarn.ManualCmndTimeout
			 == 0))
			 {
			 ConsoleWrite(txFrame.array, pakcetLen);
			 }
			 else*/

			//			if (GuiVuStaus.VuState != OP_MODE_MANUAL)
			{
				ConsoleWrite(txFrame.array, pakcetLen);

#if 0
				printf("\n[WIFI-W] len=0x%04X, %d \n", txFrame.length,
						pakcetLen); //	sizeof(GuiVuStaus)
				printf("[WIFI-W] ck=%02X,%Ld \n", txFrame.checksum,
						GuiCurrentTime.val);

				printf("[WIFI-W] [%d] Tx=> ", GetTaskTick());
				int j;
				for (j = 0; j < pakcetLen; j++)
				{
					printf("%02X ", txFrame.array[j]);
				}
				printf("\n");
#endif
			}

			// reset some flags and those shall be set by respective subtask before we came here again
			GuiVuStaus.NcuStatusB3GuidanceWarn.ManualCmndTimeout = 0;

		} // periodic packet



	} // while (1)
}

void WiFiWriteInit()
{
	/*Initialize variables*/

	/* create task */
	if (!TaskCreate(&WiFiWriteTask, WIFI_WRITE_TASK_PRIORITY, 20, &_taskHandler))
	{
		printf("WiFiWriteTask creation failed!!\n");
	}
	else
	{
		printf("WiFiWriteTask handler #%d\n", _taskHandler);
	}
}
