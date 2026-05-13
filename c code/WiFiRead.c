/*
 * WiFiRead.c
 *
 *  Created on: Nov 9, 2023
 *      Author: root
 */

#include "CommonIncludes.h"
#include "GuiWiFiFrames.h"

#define CONSOLE_RX_BUF_SIZE	512

typedef enum
{
	STD_ACK_MSG_NOT_RECOGNIZED = -1,
	STD_ACK_MSG_REFUSED = 0,
	STD_ACK_MSG_EXECUTED = 1,
	STD_ACK_MSG_NO_RESPONSE
} StdAckRetCode_t;

GuiPacket_t rxPacket;
GuiFrameAckMotor_t motorAckFrame;

static int8_t _taskHandler = -1;

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

static int DecodePackets(void)
{
	//Note: TIME_SYNC_REQ, SW_VERSION_REQ and MOTOR_STATE_REQ does not required ACK_STD upon request
	int ret = STD_ACK_MSG_NOT_RECOGNIZED;

	switch (rxPacket.packetId)
	{
	case packet_id_select_mission:
		printf("packet_id_select_mission\n");
		if (gSysFlags.IsHomeLocationAvailable == TRUE)
		{
			int8_t i, nameLen, missionType;

			//			printf("\npacket: ");
			//			for (i = 0; ((i < rxPacket.length)); i++)
			//				printf("%02X ", rxPacket.data[i]);

			missionType = rxPacket.data[0];
			printf("\nMision Type Requested: %s", (missionType == 1) ? "WPN"
					: "HSDT");

			nameLen = rxPacket.data[1];
			printf("\nMision File: ");
			for (i = 2; ((i < rxPacket.length) && (i <= (nameLen + 1))); i++)
				printf("%c", rxPacket.data[i]);

			printf("\nMD5 Hash: ");
			for (; i < rxPacket.length; i++)
				printf("%02X ", rxPacket.data[i]);
			printf("\n");

			//ToDo: check this file is available in memory
			// also check it's integrity by comparing MD5 hash code
			// upon success send ACK_STD +1 or 0

			rxPacket.data[nameLen + 2] = 0; //terminate char array, +2 because file name start from 2nd position
			//			rxPacket.data[(nameLen + 1) - 3] = 0; // removing file extension

			if (missionType == 0)
			{
				ret = ReadHSDT_MissionFile((char *) &rxPacket.data[2]);
			}
			else
			{
				ret = ReadAutoNav_MissionFile((char *) &rxPacket.data[2]);
			}
			if (1 == ret)
			{
				gSysVars.missionType = missionType;
			}
		}
		else
		{
			ret = STD_ACK_MSG_REFUSED;
			DBG_MSG("[WIFI] Send GCS Location first!!");
		}

		break;

	case packet_id_start_mission:
		printf("packet_id_start_mission\n");
		//ToDo:
		// check if Pre-Launch test is carried out with success
		if (gSysVars.missionType == AUTONAV_MISSION)
		{
			if (gSysFlags.IsAutoNav_MissionPointsAvailable == 0)
			{
				printf("AutoNAv mission file is not loaded yet\n");
				ret = STD_ACK_MSG_REFUSED;
				break;
			}
		}
		else
		{
			if (gSysFlags.IsHSDT_MissionPointsAvailable == 0)
			{
				printf("HSDT mission file is not loaded yet\n");
				ret = STD_ACK_MSG_REFUSED;
				break;
			}
		}

		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			printf("Auto mode not selected\n");
			break;
		}
		if (gSysFlags.autoModeMissionStarted == FALSE)
		{
			gSysFlags.autoModeMissionStarted = TRUE;
			ret = STD_ACK_MSG_EXECUTED;
		}
		else
		{
			printf("mission already running\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_abort_mission:
		printf("packet_id_abort_mission\n");
		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			ret = STD_ACK_MSG_NOT_RECOGNIZED;
			printf("Auto mode not selected\n");
			break;
		}

		if (gSysFlags.autoModeMissionStarted == TRUE)
		{
			gSysFlags.autoModeMissionAbort = TRUE;
			ret = STD_ACK_MSG_EXECUTED;

			//get the action to perform while aborting
			printf("Action Asked %c", rxPacket.data[0]);
		}
		else
		{
			printf("mission is not running\n");
			ret = STD_ACK_MSG_NOT_RECOGNIZED;
		}

		break;

	case packet_id_select_mode:
		printf("packet_id_select_mode\n");

#if 0
		ret = STD_ACK_MSG_EXECUTED;
		printf("invalid op mode(#%d) requested\n", rxPacket.data[0]);

		printf("pkt Id=%d, time=%Ld, pktLen=%d\n", rxPacket.packetId,
				(uint64_t) rxPacket.time.val, rxPacket.length);

#else
		ret = OperationModeSet((OperationModes_t) rxPacket.data[0]);
		printf("op mode(#%d) requested ", rxPacket.data[0]);
		if ((ret == NO_ERROR) || (ret == MODE_ALREADY_RUNNING))
		{
			ret = STD_ACK_MSG_EXECUTED;
			printf("\n");
		}
		else
		{
			//			printf("invalid op mode(#%d) requested\n", rxPacket.data[0]);
			printf(" is invalid\n");

			//			printf("pkt Id=%d, time=%Ld, pktLen=%d\n", rxPacket.packetId,
			//					(uint64_t) rxPacket.time.val, rxPacket.length);
			ret = STD_ACK_MSG_REFUSED;
		}
#endif
		break;

	case packet_id_channel_selection:
		printf("packet_id_channel_selection\n");
		break;
	case packet_id_time_sync:
		printf("packet_id_time_sync\n");
		ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required

		//ToDo: this packet shall accept in STANDBY_MODE only
		if (OperationModeGet() != OP_MODE_STANDBY)
		{
			DBG_MSG("[WiFi] command is accepted in STANDBY mode\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}

		// NCU current INS/GPS synchronized time.
		// if the NCU time is not synchronized with INS/GPS time this field is set 0.
		if (gSysVars.insSpatial.system_state_packet.filter_status.b.utc_time_initialised)
		{
			// time is synchronized
			//ToDo: check GPS fix, check time_seconds validity
			rxPacket.time.val
					= gSysVars.insSpatial.system_state_packet.unix_time_seconds
							* 1000; // convert in milliseconds
		}
		else
		{
			// time is not synchronized
			rxPacket.time.val = time(NULL);
			DBG_MSG("[WiFi] GPS time not sync'd\n");
		}

		WiFiWriteFramePushToQueue(packet_id_ack_tsync, rxPacket.time.bytes,
				NULL, 0);
		break;

	case packet_id_autotest:
		printf("packet_id_autotest\n");
		ret = OperationModeGet();
		if (ret == OP_MODE_STANDBY)
		{
			OperationModeSet(OP_MODE_PRELAUNCH_TEST);
			//			ret = STD_ACK_MSG_NO_RESPONSE; // feedback after PFC test done
			ret = STD_ACK_MSG_EXECUTED;
		}
		else if (ret == OP_MODE_PRELAUNCH_TEST) //if(_prelaunchTestOngoing)
		{
			// already running prelaunch test
			printf("Pre-Launch Test is already running\n");
			//			ret = STD_ACK_MSG_NO_RESPONSE;
			ret = STD_ACK_MSG_EXECUTED;
		}
		else
		{
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_motor_state:
		//		printf("packet_id_motor_state\n"); // frequently receiving

		// acknowledge with ACK_MOTOR packet
		//		memset(rxPacket.data, 0, sizeof(rxPacket.data));
		//		if (GuiVuStaus.VuStatusB0.NCU_Comm == 1)
		//		{

		// all are dummy data. rpm and command values are not reflecting on GUI
		// note the datatypes of variables
#if 0
		ret = (int16_t) gSysVars.thrusterData.thrust; //gSysVars.thrusterData.rpm;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBSx.rpm, 2);

		motorAckFrame.thrusterBSx.voltage
		= (uint8_t) gSysVars.thrusterData.voltage * 2;
		motorAckFrame.thrusterBSx.current
		= (uint8_t) gSysVars.thrusterData.current * 5;
		motorAckFrame.thrusterBSx.temperature
		= (uint8_t) gSysVars.thrusterData.temperature;
		motorAckFrame.thrusterBSx.command
		= (int8_t) gSysVars.thrusterData.thrustCommand;

		ret = (int16_t) gSysVars.finData[PORT_FIN].angle;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBDx.rpm, 2);
		motorAckFrame.thrusterBDx.voltage
		= (uint8_t) (gSysVars.finData[PORT_FIN].voltage * 2);
		motorAckFrame.thrusterBDx.current
		= (uint8_t) gSysVars.finData[PORT_FIN].current * 5;
		motorAckFrame.thrusterBDx.temperature
		= (uint8_t) gSysVars.finData[PORT_FIN].temperature;
		motorAckFrame.thrusterBDx.command
		= (int8_t) gSysVars.finData[PORT_FIN].angleCommand;

		ret = (int16_t) gSysVars.finData[STARBOARD_FIN].angle;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBSide.rpm, 2);
		motorAckFrame.thrusterBSide.voltage
		= (uint8_t) (gSysVars.finData[STARBOARD_FIN].voltage * 2);
		motorAckFrame.thrusterBSide.current
		= (uint8_t) gSysVars.finData[STARBOARD_FIN].current * 5;
		motorAckFrame.thrusterBSide.temperature
		= (uint8_t) gSysVars.finData[STARBOARD_FIN].temperature;
		motorAckFrame.thrusterBSide.command
		= (int8_t) gSysVars.finData[STARBOARD_FIN].angleCommand;

		ret = (int16_t) gSysVars.finData[TOP_RUDDER].angle;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterFSide.rpm, 2);
		motorAckFrame.thrusterFSide.voltage
		= (uint8_t) (gSysVars.finData[TOP_RUDDER].voltage * 2);
		motorAckFrame.thrusterFSide.current
		= (uint8_t) gSysVars.finData[TOP_RUDDER].current * 5;
		motorAckFrame.thrusterFSide.temperature
		= (uint8_t) gSysVars.finData[TOP_RUDDER].temperature;
		motorAckFrame.thrusterFSide.command
		= (int8_t) gSysVars.finData[TOP_RUDDER].angleCommand;

		ret = (int) gSysVars.finData[BOTTOM_RUDDER].angle;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBTop.rpm, 2);
		motorAckFrame.thrusterBTop.voltage
		= (uint8_t) (gSysVars.finData[BOTTOM_RUDDER].voltage * 2);
		motorAckFrame.thrusterBTop.current
		= (uint8_t) gSysVars.finData[BOTTOM_RUDDER].current * 5;
		motorAckFrame.thrusterBTop.temperature
		= (uint8_t) gSysVars.finData[BOTTOM_RUDDER].temperature;
		motorAckFrame.thrusterBTop.command
		= (int8_t) gSysVars.finData[BOTTOM_RUDDER].angleCommand;

		ret = (int16_t) gSysVars.thrusterData.rpm;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterFTop.rpm, 2);
		motorAckFrame.thrusterFTop.voltage = 60;
		motorAckFrame.thrusterFTop.current = 60;
		motorAckFrame.thrusterFTop.temperature = 35;
		motorAckFrame.thrusterFTop.command = 6;
#else
		/***Thruster***/
		ret = (int16_t) gSysVars.thrusterData.rpm;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBSx.rpm, 2);

		motorAckFrame.thrusterBSx.current
				= (int8_t) (gSysVars.thrusterData.current * 5);
		motorAckFrame.thrusterBSx.voltage
				= (int8_t) gSysVars.thrusterData.voltage;
		motorAckFrame.thrusterBSx.temperature
				= (int8_t) gSysVars.thrusterData.temperature;
		motorAckFrame.thrusterBSx.command
				= (int8_t) gSysVars.thrusterData.thrustCommand;

		/***Left/Port FIN***/
		ret = (int16_t) gSysVars.finData[PORT_FIN].position;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBDx.rpm, 2);

		motorAckFrame.thrusterBDx.current
				= (int8_t) (gSysVars.finData[PORT_FIN].current * 5);
		motorAckFrame.thrusterBDx.voltage
				= (int8_t) (gSysVars.finData[PORT_FIN].voltage);
		motorAckFrame.thrusterBDx.temperature
				= (int8_t) gSysVars.finData[PORT_FIN].temperature;
		motorAckFrame.thrusterBDx.command
				= (int8_t) gSysVars.finData[PORT_FIN].angleCommand;

		/***Right/Starboard FIN***/
		ret = (int16_t) gSysVars.finData[STARBOARD_FIN].position;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBSide.rpm, 2);

		motorAckFrame.thrusterBSide.current
				= (int8_t) (gSysVars.finData[STARBOARD_FIN].current * 5);
		motorAckFrame.thrusterBSide.voltage
				= (int8_t) (gSysVars.finData[STARBOARD_FIN].voltage);
		motorAckFrame.thrusterBSide.temperature
				= (int8_t) gSysVars.finData[STARBOARD_FIN].temperature;
		motorAckFrame.thrusterBSide.command
				= (int8_t) gSysVars.finData[STARBOARD_FIN].angleCommand;

		/***Top Rudder FIN***/
		ret = (int16_t) gSysVars.finData[TOP_RUDDER].position;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterFSide.rpm, 2);

		motorAckFrame.thrusterFSide.current
				= (int8_t) (gSysVars.finData[TOP_RUDDER].current * 5);
		motorAckFrame.thrusterFSide.voltage
				= (int8_t) (gSysVars.finData[TOP_RUDDER].voltage);
		motorAckFrame.thrusterFSide.temperature
				= (int8_t) gSysVars.finData[TOP_RUDDER].temperature;
		motorAckFrame.thrusterFSide.command
				= (int8_t) gSysVars.finData[TOP_RUDDER].angleCommand;

		/***Bottom Rudder FIN***/
		ret = (int) gSysVars.finData[BOTTOM_RUDDER].position;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterBTop.rpm, 2);

		motorAckFrame.thrusterBTop.current
				= (int8_t) (gSysVars.finData[BOTTOM_RUDDER].current * 5);
		motorAckFrame.thrusterBTop.voltage
				= (int8_t) (gSysVars.finData[BOTTOM_RUDDER].voltage);
		motorAckFrame.thrusterBTop.temperature
				= (int8_t) gSysVars.finData[BOTTOM_RUDDER].temperature;
		motorAckFrame.thrusterBTop.command
				= (int8_t) gSysVars.finData[BOTTOM_RUDDER].angleCommand;

		/***current thrust and angles ***/
		ret = (int16_t) gSysVars.thrusterData.thrust;
		endianConverter((uint8_t*) &ret,
				(uint8_t*) &motorAckFrame.thrusterFTop.rpm, 2);

		motorAckFrame.thrusterFTop.current
				= (int8_t) gSysVars.finData[PORT_FIN].angle;
		motorAckFrame.thrusterFTop.voltage
				= (int8_t) gSysVars.finData[STARBOARD_FIN].angle;
		motorAckFrame.thrusterFTop.temperature
				= (int8_t) gSysVars.finData[TOP_RUDDER].angle;
		motorAckFrame.thrusterFTop.command
				= (int8_t) gSysVars.finData[BOTTOM_RUDDER].angle;
#endif

		WiFiWriteFramePushToQueue(packet_id_ack_motor, rxPacket.time.bytes,
				(uint8_t*) &motorAckFrame, sizeof(motorAckFrame));
		//		}
		// echo the received frame for testing
		//		WiFiWriteFramePushToQueue(rxPacket.packetId, rxPacket.time.bytes,
		//				rxPacket.data, rxPacket.length);

		ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required
		break;

	case packet_id_action_gotowp:
		printf("packet_id_action_gotowp\n");
		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			printf("Auto mode not selected\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}
		if (gSysVars.missionType == HSDT_MISSION)
		{
			printf("HSDT missionType is selected in sysParam.txt\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}
		if (gSysFlags.autoModeMissionStarted == FALSE)
		{
			DWORD_VAL tmp;

			//			int i = 0;
			//			printf("\n");
			//			for (i = 0; i < rxPacket.length; i++)
			//				printf("%02X ", rxPacket.data[i]);
			//			printf("\n");

			//			memcpy(&tmp.byte[0], &rxPacket.data[0], 4);
			endianConverter(&rxPacket.data[0], &tmp.byte[0], 4);
			gotowp.lat = (float) tmp.Val / 10000000UL;
			printf("Lat=%.5f, ", gotowp.lat);

			//			memcpy(&tmp.byte[0], &rxPacket.data[4], 4);
			endianConverter(&rxPacket.data[4], &tmp.byte[0], 4);
			gotowp.lon = (float) tmp.Val / 10000000UL;
			printf("Lon=%.5f, ", gotowp.lon);

			//			memcpy(&tmp.byte[0], &rxPacket.data[8], 4);
			endianConverter(&rxPacket.data[8], &tmp.byte[0], 4);
			gotowp.depthAltitude = (float) tmp.Val / 100;
			printf("Depth/Altitude=%.1f, ", gotowp.depthAltitude);

			endianConverter(&rxPacket.data[12], &tmp.byte[0], 2);
			gotowp.speed = (float) tmp.sVal[0] / 100;
			printf("Speed=%.2f, ", gotowp.speed);

			gotowp.radius = rxPacket.data[14];
			printf("Bubble/Radius=%d\n", gotowp.radius);

			ret = STD_ACK_MSG_EXECUTED;

			//ToDo:get the WP
			if (gSysVars.missionSubMode == MISN_SUB_MOD_GOTO)
				ModifyGoToMode();
			else
				SwitchToGoToMode();
		}
		else
		{
			printf("mission is running, can not consider goto points\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_action_hovering:
		printf("packet_id_action_hovering\n");
		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			printf("Auto mode not selected\n");
			break;
		}
		if (gSysVars.missionType == HSDT_MISSION)
		{
			printf("HSDT missionType is selected in sysParam.txt\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}
		if (gSysFlags.autoModeMissionStarted == FALSE)
		{
			ret = STD_ACK_MSG_EXECUTED;
			//ToDo:get the WP
		}
		else
		{
			printf("mission is running, can not follow hovering\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_action_floating:
		printf("packet_id_action_floating\n");
		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			printf("Auto mode not selected\n");
			break;
		}
		if (gSysVars.missionType == HSDT_MISSION)
		{
			printf("HSDT missionType is selected in sysParam.txt\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}
		if (gSysFlags.autoModeMissionStarted == FALSE)
		{
			ret = STD_ACK_MSG_EXECUTED;
			//ToDo:get the WP
		}
		else
		{
			printf("mission is running, can not follow floating\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_action_surfacing:
		printf("packet_id_action_surfacing\n");
		if (OperationModeGet() != OP_MODE_AUTONOMOUS)
		{
			printf("Auto mode not selected\n");
			break;
		}
		if (gSysVars.missionType == HSDT_MISSION)
		{
			printf("HSDT missionType is selected in sysParam.txt\n");
			ret = STD_ACK_MSG_REFUSED;
			break;
		}
		if (gSysFlags.autoModeMissionStarted == FALSE)
		{
			ret = STD_ACK_MSG_EXECUTED;
			//ToDo:get the WP
		}
		else
		{
			printf("mission is running, can not follow surfacing\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_battery_level:
		printf("packet_id_battery_level\n");
		DBG_MSG("[WIFI-R] New Battery Level #%d% Requested",rxPacket.data[0]);
		if ((rxPacket.data[0] >= 10) && (rxPacket.data[0] < 100))
		{
			BMS_MIN_SOC_LIMIT = (uint8_t) rxPacket.data[0];
			DBG_MSG("[WIFI-R] Battery Level Updated to #%d%",BMS_MIN_SOC_LIMIT);
			ret = STD_ACK_MSG_EXECUTED;
		}
		else
		{
			DBG_MSG("[WIFI-R] Battery Level Refused. Range 10-100%");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;
	case packet_id_bouyancy:
		printf("packet_id_bouyancy\n");
		//ToDo: drop weight and turn off all actuators
		gSysVars.phyDO[DO_CH_DROPWEIGHT] = 1;
		OperationModeSet(OP_MODE_STANDBY);
		break;

	case packet_id_rov_ctrl_pck:
		//		printf("packet_id_rov_ctrl_pck\n"); // frequently receiving
		ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required


		//		printf("[WiFi-R] surge=%d, sway=%d, heave=%d, pitch=%d, yaw=%d\n",
		//				(int8_t) rxPacket.data[0], (int8_t) rxPacket.data[1],
		//				(int8_t) rxPacket.data[2], (int8_t) rxPacket.data[3],
		//				(int8_t) rxPacket.data[4]);

		GuiVuStaus.NcuStatusB3GuidanceWarn.ManualCmndTimeout = 1;

		gSysVars.timeoutTickManual = GetTaskTick();

		gSysFlags.disablePID = 1.0;

		if (gSysFlags.manualModeEmergency == 0)
		{
			gSysVars.portFinManualValue = (int8_t) rxPacket.data[1];
			gSysVars.starboardFinManualValue = (int8_t) rxPacket.data[1];
			gSysVars.topRudderManualValue = (int8_t) rxPacket.data[4];
			gSysVars.bottomRudderManualValue = (int8_t) rxPacket.data[4];

			gSysVars.throttleManualValue = (int8_t) rxPacket.data[0] * -1.0;

#if 0
			if (IsHILsEnable())
			{
				// for testing we have limited joystick value in range -10 to +10 for thruster
				// in open to air scenario

				gSysVars.throttleManualValue = map_rationalize(
						(gSysVars.throttleManualValue / 10), 0, 10,
						(float) HILS_THRUST_MIN_LIM, (float) HILS_THRUST_MAX_LIM);
			}
			else
			{

				if (THRUSTER_MAX_NEGATIVE_THRUST_LIM >= 0)
				{
					// considering only +ve thrust
					gSysVars.throttleManualValue = map_rationalize(
							gSysVars.throttleManualValue, 0, 100,
							THRUSTER_MAX_NEGATIVE_THRUST_LIM,
							THRUSTER_MAX_POSITIVE_THRUST_LIM); // take constants from file
				}
				else
				{
					// considering -ve thrust
					gSysVars.throttleManualValue = map_rationalize(
							gSysVars.throttleManualValue, 100, 100,
							THRUSTER_MAX_NEGATIVE_THRUST_LIM,
							THRUSTER_MAX_POSITIVE_THRUST_LIM); // take constants from file
				}
			}
#endif

			if (gSysVars.throttleManualValue < 0.0)
			{
				//			gSysVars.throttleManualValue = 0; // not considering -ve values
				gSysVars.throttleManualValue = map_rationalize(
						gSysVars.throttleManualValue, -100, 0,
						(THRUSTER_MAX_NEGATIVE_THRUST_LIM), 0);

			}
			else
			{
				gSysVars.throttleManualValue = map_rationalize(
						gSysVars.throttleManualValue, 0, 100, (0),
						THRUSTER_MAX_POSITIVE_THRUST_LIM); // take constants from file
			}

			// -100 to +100 percentage value is the range of joystick up/down and left/right movement
			gSysVars.topRudderManualValue = map_rationalize(
					gSysVars.topRudderManualValue, -100, 100, FIN_ANGLE_MIN,
					FIN_ANGLE_MAX);

			gSysVars.bottomRudderManualValue = map_rationalize(
					gSysVars.bottomRudderManualValue, -100, 100, FIN_ANGLE_MIN,
					FIN_ANGLE_MAX);

			gSysVars.portFinManualValue = map_rationalize(
					gSysVars.portFinManualValue, -100, 100, FIN_ANGLE_MIN,
					FIN_ANGLE_MAX);

			gSysVars.starboardFinManualValue = map_rationalize(
					gSysVars.starboardFinManualValue, -100, 100, FIN_ANGLE_MIN,
					FIN_ANGLE_MAX);

			//		printf(
			//				"[WiFi-R] top=%.1f, bottom=%.1f, port=%.1f, starboard=%.1f, Throttle=%.2f\n",
			//				gSysVars.topRudderManualValue,
			//				gSysVars.bottomRudderManualValue,
			//				gSysVars.portFinManualValue,
			//				gSysVars.starboardFinManualValue,
			//				gSysVars.throttleManualValue);

			// data packets are received in 100 msec
			// rate limiter either keep here or in SetServoPosition() in FinServo
			//		gSysVars.topRudderManualValue = RateLimit_TopRudder(
			//				gSysVars.topRudderManualValue, FIN_ROTATION_PER_SYSTICK, 1);
			//
			//		gSysVars.bottomRudderManualValue = RateLimit_BottomRudder(
			//				gSysVars.bottomRudderManualValue, FIN_ROTATION_PER_SYSTICK,
			//				1);
			//
			//		gSysVars.portFinManualValue = RateLimit_PortFin(
			//				gSysVars.portFinManualValue, FIN_ROTATION_PER_SYSTICK, 1);
			//
			//		gSysVars.starboardFinManualValue = RateLimit_StarboardFin(
			//				gSysVars.starboardFinManualValue, FIN_ROTATION_PER_SYSTICK,
			//				1);

			//		gSysVars.throttleManualValue
			//				= RateLimit_Throttle(gSysVars.throttleManualValue,
			//						THRUSTER_SPEED_PER_SYSTICK, 0.1);

		} // if (gSysFlags.manualModeEmergency == 0)

		break;

	case packet_id_cmd_nose:
		printf("packet_id_cmd_nose\n");
		break;
	case packet_id_gui_keep_alive:
		//		printf("packet_id_gui_keep_alive\n"); // frequently receiving
		ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required

		// extract time and sync our time
		GuiCurrentTime.val = rxPacket.time.val;
		// set communication flag
		GuiVuStaus.VuStatusB1.WifiActive = 1;
		GuiVuStaus.VuStatusB1.WifiCommOk = 1;
		break;

	case packet_id_shutdown:
		printf("packet_id_shutdown\n");
		//ToDo: set in standby operation mode and then shutdown system after few ticks
		break;
	case packet_id_stop_log:
		printf("packet_id_stop_log\n");
		//ToDo: we can toggle flag on every time this command received.
		ret = OperationModeGet();
		if (ret == OP_MODE_STANDBY)
		{
			// command will considered in 'standby' mode only
			WriteLogEnable(FALSE);
			ret = STD_ACK_MSG_EXECUTED;
			printf(
					"Logging will be auto enabled in other than 'STANDBY' mode\n");
		}
		else
		{
			printf("Logging can stop in 'STANDBY' mode only\n");
			ret = STD_ACK_MSG_REFUSED;
		}
		break;

	case packet_id_sw_version:
	{
		printf("packet_id_sw_version\n");
		ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required

		GuiFrameAckVersion_t ackPacket;

		memset(&ackPacket, 0, sizeof(ackPacket));

		ackPacket.VuSwVersionLSB = 0x01;
		ackPacket.VuSwVersionMSB = 0x02;

		if (GuiVuStaus.VuStatusB0.LB_Comm == 1)
		{
			ackPacket.LbSwVersionLSB = 0x03;
			ackPacket.LbSwVersionMSB = 0x04;
		}
		if (GuiVuStaus.VuStatusB0.NCU_Comm == 1)
		{
			ackPacket.NcuSwVersionLSB = 0x05;
			ackPacket.NcuSwVersionMSB = 0x06;
		}
		if (GuiVuStaus.VuStatusB0.Carris_Comm == 1)
		{
			ackPacket.CarrisSwVersionLSB = 0x07;
			ackPacket.CarrisSwVersionMSB = 0x08;
		}

		WiFiWriteFramePushToQueue(packet_id_ack_version, rxPacket.time.bytes,
				(uint8_t*) &ackPacket, sizeof(ackPacket));
	}
		break;

	case packet_id_cmd_dw:
		printf("packet_id_cmd_dw\n");
		break;

	case packet_id_cmd_hsdt: // 199
		printf("packet_id_cmd_HSDT\n");
		if (rxPacket.length == 18)
		{
			DWORD_VAL heading, duration;
			WORD_VAL depth, speed;

			//			printf("heading= %02X %02X %02X %02X ", rxPacket.data[0],
			//					rxPacket.data[1], rxPacket.data[2], rxPacket.data[3]);
			heading.byte[3] = rxPacket.data[0];
			heading.byte[2] = rxPacket.data[1];
			heading.byte[1] = rxPacket.data[2];
			heading.byte[0] = rxPacket.data[3];

			depth.byte[1] = rxPacket.data[10];
			depth.byte[0] = rxPacket.data[11];

			speed.byte[1] = rxPacket.data[12];
			speed.byte[0] = rxPacket.data[13];

			duration.byte[3] = rxPacket.data[14];
			duration.byte[2] = rxPacket.data[15];
			duration.byte[1] = rxPacket.data[16];
			duration.byte[0] = rxPacket.data[17];

			if (OperationModeGet() == OP_MODE_SEMI_AUTONOMOUS)
			{
				gSysVars.yawSemiAutoValue = ((float) heading.Val / 10000000);
				if (gSysVars.yawSemiAutoValue < 0)
				{
					gSysVars.yawSemiAutoValue += 360.0; // move in 0 to 360
				}

				gSysVars.depthSemiAutoValue = (float) depth.Val / 100;
				if(gSysVars.depthSemiAutoValue < 0)
				{
					gSysVars.depthSemiAutoValue *= 1.0; // take positive value only
				}

				gSysVars.velocitySemiAutoValue = (float) speed.Val / 100;
				gSysVars.durationSemiAutoValueInSec = (float) duration.Val;

//				gSysVars.timeoutTickSemiAuto = GetTaskTick();

				gSysFlags.newSemiAutoValueReceived = TRUE;

				printf(
						"New Heading=%.1f, depth=%.1f m, speed=%.2f m/s, duration=%.1f Sec\n",
						gSysVars.yawSemiAutoValue, gSysVars.depthSemiAutoValue,
						gSysVars.velocitySemiAutoValue,
						gSysVars.durationSemiAutoValueInSec);

				ret = STD_ACK_MSG_EXECUTED;
			}
			else
			{
				printf(
						"Heading=%d, depth=%d m, speed=%.2f m/s, duration=%d Sec\n",
						(heading.Val / 10000000), (depth.Val / 100),
						((float) speed.Val / 100), duration.Val);
				printf("not in 'ROV Direct' mode!\n");
				ret = STD_ACK_MSG_REFUSED;
			}
		}
		break;

	case packet_id_cmd_gcs_location:
		printf("packet_id_cmd_gcs_location\n");
		ret = STD_ACK_MSG_EXECUTED;
		if (1)
		{
			DWORD_VAL tmp;

			//			int i = 0;
			//			printf("\n");
			//			for (i = 0; i < rxPacket.length; i++)
			//				printf("%02X ", rxPacket.data[i]);
			//			printf("\n");

			//			memcpy(&tmp.byte[0], &rxPacket.data[0], 4);
			endianConverter(&rxPacket.data[0], &tmp.byte[0], 4);
			homeLoc.lat = (float) tmp.Val / 1000000UL;
			gSysVars.gcsLat = homeLoc.lat;

			//			memcpy(&tmp.byte[0], &rxPacket.data[4], 4);
			endianConverter(&rxPacket.data[4], &tmp.byte[0], 4);
			homeLoc.lon = (float) tmp.Val / 1000000UL;
			gSysVars.gcsLon = homeLoc.lon;

			DBG_MSG("[WIFI] GCS Lat=%.5f, Lon=%.5f received",gSysVars.gcsLat, gSysVars.gcsLon);

			gSysFlags.IsHomeLocationAvailable = TRUE;
		}
		break;

	default: // unknown packet
		printf("unknown packet id #%d, len=%d\n", rxPacket.packetId,
				rxPacket.length);
		{
			int8_t i;
			for (i = 0; i < rxPacket.length; i++)
				printf("%02X ", rxPacket.data[i]);
			printf("\n");
		}
		// generate error packet
		break;
	} // switch()
	return ret;
}

static int8_t ReadReceivedData()
{
	//	int bytesReceived = 0;
	int bytesAvailable = 0;
	int status = -1;
	static uint8_t readBuffer[CONSOLE_RX_BUF_SIZE];

	/*check No of bytes available*/
	if ((cansole_ctrl.ioctl(serFdconsole, FIONREAD, (int) &bytesAvailable))
			== ERROR)
	{
		DBG_MSG("[WiFi] Ioctl Read : Error\n");
	}

	if (bytesAvailable <= 0)
		return -1;

	int bytesRead = 0;
	if ((bytesRead = cansole_ctrl.read(serFdconsole, (char*) &readBuffer[0],
			CONSOLE_RX_BUF_SIZE)) == ERROR)
	{
		DBG_MSG("[WiFi] Error in reading from serial!\n");
	}
	else
	{
		unsigned int i;
#if 0
		printf("\n[CNSL-R] [%d] Rx=> ", systemCycleCount);
		for (i = 0; i < bytesRead; i++)
		{
			printf("%02X ", readBuffer[i]);
		}
		printf("\n");
#endif
		// minimum packet size is 14 including header and footer
		if (bytesRead < 14)
			return -1;

		i = 0;
		if (readBuffer[i++] == GUI_START_MSG_VALUE)
		{
			rxPacket.packetId = readBuffer[i++];
			//next 8 bytes are time in milliseconds
			rxPacket.time.bytes[7] = readBuffer[i++];
			rxPacket.time.bytes[6] = readBuffer[i++];
			rxPacket.time.bytes[5] = readBuffer[i++];
			rxPacket.time.bytes[4] = readBuffer[i++];
			rxPacket.time.bytes[3] = readBuffer[i++];
			rxPacket.time.bytes[2] = readBuffer[i++];
			rxPacket.time.bytes[1] = readBuffer[i++];
			rxPacket.time.bytes[0] = readBuffer[i++];

			rxPacket.length = (readBuffer[i++] << 8);
			rxPacket.length = rxPacket.length | readBuffer[i++];

			//					time_t t = rxPacket.time.val / 1000;// convert to seconds
			//					t += 19800; // add local timezone seconds, India +5:30 zone=19,800 seconds
			//					struct tm ltm = *gmtime(&t); // UTC time
			//					printf("System Date is: %02d/%02d/%04d\n", ltm.tm_mday,
			//							ltm.tm_mon + 1, ltm.tm_year + 1900);
			//					printf("System Time is: %02d:%02d:%02d\n", ltm.tm_hour,
			//							ltm.tm_min, ltm.tm_sec);

			// required super privileged to set time
			//					struct timeval tVal;
			//					struct timezone tZ;
			//					tVal.tv_sec = 167602767 + 19800; // local time seconds added
			//					tZ.tz_minuteswest = 330; //+5:30
			//					printf("set time %d\n", settimeofday(&tVal, 0));

			//					printf("pkt Id=%d, time=%Ld, pktLen=%d\n",
			//							rxPacket.packetId, (uint64_t) rxPacket.time.val,
			//							rxPacket.length);


			// check if expected bytes received
			if (rxPacket.length <= bytesRead)
			{
				if (readBuffer[rxPacket.length - 1] == GUI_END_MSG_VALUE)
				{
					uint8_t ck = CalculateChecksum(&readBuffer[1],
							rxPacket.length - 3);

					if (ck == readBuffer[rxPacket.length - 2])
					{
						// a valid packet found
						//						if (!gSysVars.wifiData.linkStatus)
						//						{
						//							DBG_MSG("[WiFi] now responding\n");
						//						}
						//
						//						gSysVars.wifi.timeSinceLinkDownInMs = GetTaskTick();
						//						gSysVars.wifi.linkStatus = 1;
						//						gSysVars.wifi.nonOperationalError = FALSE;

						status = 1;

						// set system time with GUI time
						if (!gSysVars.wifi.linkStatus)
						{
							//							_once = 0;
							time_t t = rxPacket.time.val / 1000;// convert to seconds
							DBG_MSG("[WiFi] Set System Time : %s\n",
									stime(&t) == 0 ? "Success" : "Fail");
						}

						// printf("valid packet found\n");
						rxPacket.length = rxPacket.length - 14; // removing overhead

						// copy payload data
						bcopy(&readBuffer[12], &rxPacket.data[0],
								rxPacket.length);

						int ret = DecodePackets();
						if (ret != STD_ACK_MSG_NO_RESPONSE)
						{
							// need to send feedback
							// send ACK packet
							rxPacket.data[0] = ret;
							WiFiWriteFramePushToQueue(packet_id_ack_std,
									rxPacket.time.bytes,
									(uint8_t*) &rxPacket.data, 1);

							DBG_MSG("[WiFi] sending ACK #%d \n", ret);
						}

					}
					else
					{
						// checksum mismatched, generate error
						rxPacket.data[0] = STD_ACK_MSG_NOT_RECOGNIZED; // message not recognized (wrong ID,Length,chkesum)
						WiFiWriteFramePushToQueue(packet_id_ack_std,
								rxPacket.time.bytes, (uint8_t*) &rxPacket.data,
								1);

					}
				}
				else
				{
					// generate error

				}
			}

		}
		else
		{
			// generate error status packet
		}
	}

	return status;

}

void WiFiReadTask()
{

	if (!semCreate(_taskHandler) == -1)
	{
		printf("Failed! semCreate WiFiReadTask\n");
		return; //ERROR;
	}

	DBG_MSG("\n\n");
	DBG_MSG("===WiFiReadTask started===\n");

	while (1)
	{
		if (semTake(_taskHandler) == -1)
		{
			printf("[%d] WiFiReadTask exited\n", GetTaskTick());
			return;
		}

		if (ReadReceivedData() == 1)
		{
			if (!gSysVars.wifi.linkStatus)
			{
				DBG_MSG("[WiFi] now responding\n");
			}

			gSysVars.wifi.timeSinceLinkDownInMs = GetTaskTick();
			gSysVars.wifi.linkStatus = 1;
			gSysVars.wifi.nonOperationalError = FALSE;

		}

		if (GetTaskTickDiff(gSysVars.wifi.timeSinceLinkDownInMs) > WAIT_15_SEC)
		{
			if ((gSysVars.wifi.linkStatus == 1)
					|| (gSysVars.wifi.nonOperationalError == FALSE))
			{
				DBG_MSG("[WiFi] not responding\n");
				//ToDo: check all source of incoming wireless communication links
				// before deciding to call standby operation
				//				OperationModeSet(OP_MODE_STANDBY);
			}
			gSysVars.wifi.linkStatus = 0;
			gSysVars.wifi.nonOperationalError = TRUE;

//			GuiVuStaus.VuStatusB1.WifiCommOk = 0;
			GuiVuStaus.VuStatusB1.WifiActive = 0;
		}
	} // while (1)
}

void WiFiReadInit()
{
	/*Initialize variables*/

	/* create task */
	if (!TaskCreate(&WiFiReadTask, WIFI_READ_TASK_PRIORITY, 20, &_taskHandler))
	{
		printf("WiFiReadTask creation failed!!\n");
	}
	else
	{
		printf("WiFiReadTask handler #%d\n", _taskHandler);
	}
}
