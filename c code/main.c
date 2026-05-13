//OM GAN GANPATAYE NAMAHA
//OM NAMAHA SHIVAY
//JAI MATA DI
/*
 * main.c
 *
 *  Created on: May 31, 2023
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>

#include "CommonIncludes.h"
#include "interfaceDefines.h"
#include "interfaceGlobals.h" /* include here only */
#include "interfaceExterns.h" /* extern copy of global variables */
#include "externFuncs.h"

#include "initEthDrivers.h"
#include "taskMgmt.h"
#include "echo_sounder.h"
#include "pressure_sensor.h"
#include "serialInit.h"

/*EXTERNS*/
//extern pthread_t threadId[NUM_OF_THREADS];
//extern pthread_attr_t threadAttr[NUM_OF_THREADS];

static int ReadSystemParams()
{
	char tempBuff[250];
	FILE* fp;
	int textIter = 0;
	int textLim;
	int ret = 0;

	fp = fopen(SYSTEM_PARAM, "r");

	if (fp == NULL)
	{
		printf("System Parameters file doesn't exist!\n");

	}
	else
	{
		fscanf(fp, "%d", &textLim);

		if (textLim > MAX_SYSTEM_PARAMS || textLim < 0)
		{
			printf("System Parameters file corrupted!\n");
		}
		else if (textLim != MAX_SYSTEM_PARAMS)
		{
			printf("System Parameters file incompatible version!\n");
		}
		else
		{
			for (textIter = 0; textIter < textLim; textIter++)
			{
				fscanf(fp, "%[^:]", tempBuff);
				fscanf(fp, "%c", &tempBuff[0]);
				fscanf(fp, "%f", &gSysParams.val[textIter]);
				printf("%s= %.3f\n", &tempBuff[0], gSysParams.val[textIter]);
			}

			ret = 1;
			printf("Total #%d  System Parameters variables read from file\n",
					textLim);
		}
		fclose(fp);
	}

	return ret;
}

static void readIpSettings()
{
	char tempBuff[250];
	FILE* fp;
	int textIter = 0;
	int textLim;
	int ret = 0;

	fp = fopen(IP_PARAM_FILE, "r");

	if (fp == NULL)
	{
		printf("IP details file doesn't exist!\n");

	}
	else
	{
		char ip[16];
		int port = 0;

		fscanf(fp, "%[^:]", tempBuff);
		fscanf(fp, "%c", &tempBuff[0]);
		fscanf(fp, "%s", &GCS_IP[0]);
		printf("%s= %s\n", &tempBuff[0], GCS_IP);

		fscanf(fp, "%[^:]", tempBuff);
		fscanf(fp, "%c", &tempBuff[0]);
		fscanf(fp, "%d", &port);
		printf("%s= %d\n", &tempBuff[0], port);

		fscanf(fp, "%[^:]", tempBuff);
		fscanf(fp, "%c", &tempBuff[0]);
		fscanf(fp, "%s", &ip[0]);
		printf("%s= %s\n", &tempBuff[0], ip);

		fscanf(fp, "%[^:]", tempBuff);
		fscanf(fp, "%c", &tempBuff[0]);
		fscanf(fp, "%d", &port);
		printf("%s= %d\n", &tempBuff[0], port);
	}
}

int main()
{
	printf("\n\nMaya Acoustic Targert(MAT)\n\n");
	printf("Firmware Version: 1.0\n");
	printf("Build: %s %s\n", __DATE__, __TIME__);
	//	taskCreate(&mainThread2);

	time_t currentTime = time(NULL);
	//	currentTime += 19800; // add local timezone seconds, India +5:30 zone=19,800 seconds

	struct tm ltm = *gmtime(&currentTime); // UTC time
	printf("\nSystem Date is: %02d/%02d/%02d\n", ltm.tm_mday, ltm.tm_mon + 1,
			ltm.tm_year + 1900);
	printf("System Time is: %02d:%02d:%02d\n", ltm.tm_hour, ltm.tm_min,
			ltm.tm_sec);
	printf("\nUTC: %ld seconds\n", currentTime);

	printf("initializing system...\n");

	/* clear all global variables and flags*/
	memset((uint8_t*) &gSysVars, 0, sizeof(SystemVars_t));
	memset((uint8_t*) &gSysFlags, 0, sizeof(SystemFlags_t));
	memset((uint8_t*) &gSysParams, 0, sizeof(SystemParams_t));
	memset((uint8_t*) &ctrlLoopParams, 0, sizeof(ControlLoopsParams_t));
	memset((uint8_t*) &hsdtMission, 0, sizeof(HSDTpacket_t));
	memset((uint8_t*) &gotowp, 0, sizeof(GoToWP_t));
	memset((uint8_t*) &homeLoc, 0, sizeof(HomeLocation_t));
	memset((uint8_t*) &GuiVuStaus, 0, sizeof(GuiFrameVuStatus_t));
	memset((uint8_t*) &GuiCurrentTime, 0, sizeof(GuiTime_t));

	/*init system configuration*/
	if (ReadSystemParams() == 1)
	{
		printf("Configuring system as per text files...\n\n");
	}
	else
	{
		printf("System could NOT be configured!\nUsing defaults...\n");
		printf("Enabling HILs Mode...\n\n");

		//		SetHILsMode(TRUE);
		gSysParams.simPlant = 0;

		gSysParams.thruster_LinkFailTimeout = 10; //seconds
		gSysParams.thruster_ZeroRPMtimeout = 10;// seconds
		gSysParams.thruster_MaxPositiveThrottle = 30; //for HILs otherwise 100
		gSysParams.thruster_MaxNegativeThrottle = -10;
		gSysParams.thruster_ThrottleOffset = 0;
		gSysParams.thruster_slewRate = 1;

		gSysParams.pressureSensor_LinkFailTimeout = 5; //seconds
		gSysParams.pressureSensor_WaterSurfaceDetectLvl = 0.5; //meters
		//#define WATER_DENSITY			=			1023 //kg/m3
		gSysParams.waterDensity = 1023.000;
		gSysParams.pressureSensorOffset = 0.95;

		gSysParams.echosounder_LinkFailTimeout = 5;//seconds
		gSysParams.echosounder_ConfidenceLvlMin = 80; // ercent
		gSysParams.echosounder_SeaBedDetectLvl = 30;//  meter
		gSysParams.echosounder_OutOfRangeLimit = 310;//  meter

		gSysParams.spatial_GPSFixTimeout = 10; // seconds
		gSysParams.printParameterTimeoutInSeconds = 30; // seconds
		SetDebugMode(TRUE);

		gSysParams.vehicle_DefaultSpeed = 1; //meter/sec
		gSysParams.vehicle_DefaultRadius = 5; //meter
		//		VEHICLE_DEFAULT_DEPTH = PRESSURE_WATER_SURFACE_DEPTH;

		/***/
		//		gSysParams.pressureSensor_LinkFailTimeout  = 5; //seconds
		//		gSysParams.pressureSensor_WaterSurfaceDetectLvl = 2; //12 meters
		//		 WATER_DENSITY						dghSysParams.sysParams.waterDensity //kg/m3

		gSysParams.bms_LinkFailTimeout = 5;
		gSysParams.bms_MinimumRequiredSOC = 20;

		gSysParams.manualModeIdle_Timeout = 10;
		gSysParams.manualCommand_Timeout = 5;
		gSysParams.manualModeDepthLimit = 5;
		gSysParams.manualModePitchLimit = -5.0;
		gSysParams.manualModeRollMaxLimit = 30.0;
		gSysParams.manualModeRollMinLimit = 30.0;
		gSysParams.manualModeEmrgncyThrustLimit = -10;
		gSysParams.manualModeEmrgncyThrustTimeout = 10;

		gSysParams.divein_thrust = 50;
		gSysParams.divein_pitchUpFinAngle = 10;
		gSysParams.divein_pitchDownFinAngle = -10;
		gSysParams.divein_pitchFeedbackLmt = 1;
		gSysParams.divein_depthFeedbackLmt = 1;

	}

	readIpSettings();
	initEthDrivers();
	serialInit();

	if (pthread_mutex_init(&logParams_mutex, NULL) != 0)
	{
		printf("\nlogparams mutex failed to init\n");
		return 1;
	}

	DebugInit();

	DigitalInputInit();
	DigitalOutputInit();

	PressureSensorInit();

	if (IS_ALTIMETER_ENABLE())
	{
		EcoSounderInit();
	}
	else
	{
		gSysVars.altimeterData.linkStatus = 1;
	}

	//#if USE_MOTUS_READINGS
	MotusInit();
//	MotusWriteInit();
	//#endif
	SpatialReadInit();

	// Note: RIO card required to receive data on CAN-associated UDP port to enable tx/rx on all port
	ThrusterInit();
	FinServoInit();
	BMSInit();

	MatlabControlInit();
	OperationModeInit();

	WiFiReadInit();
	WiFiWriteInit();

	WriteLogInit();

	BuzzerInit();
	StrobelightInit();
//	SatComInit();

	// call it after all tasks has been created
	TaskSchdlInit();

	printf("\n\nHope you have enjoyed the ride...!!\nthank you!!!\n");

	closeEthDrivers();
	pthread_mutex_destroy(&logParams_mutex);

	return 0;
}
