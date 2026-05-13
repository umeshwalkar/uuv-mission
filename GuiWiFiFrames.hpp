/*
 * GuiWiFiFrames.h
 *
 *  Created on: Sep 29, 2022
 *      Author: root
 */

#ifndef GUIWIFIFRAMES_H_
#define GUIWIFIFRAMES_H_

/*This file contains variables and structures used in frames that is being sent over WiFi to GUI/GCS */

/* GUI */
#define GUI_START_MSG_VALUE 0x24
#define GUI_END_MSG_VALUE 0x0A
#define GUI_FRAME_OVERHEAD_BYTES 14

// #define GUI_PACKET_ID_VU_STATUS		151

typedef enum
{
	/* DO NOT ALTER */
	/* transmitting packets */
	packet_id_vu_status_pck = 151,
	packet_id_vu_plt_status_pck = 152,

	packet_id_ack_version = 156,
	packet_id_ack_std,
	packet_id_ack_tsync,
	packet_id_ack_motor,

	/* receiving packets */
	packet_id_select_mission = 171,
	packet_id_start_mission,
	packet_id_abort_mission,

	packet_id_select_mode = 176,
	packet_id_channel_selection,
	packet_id_time_sync,
	packet_id_autotest,
	packet_id_motor_state,
	packet_id_action_gotowp,
	packet_id_action_hovering,
	packet_id_action_floating,
	packet_id_action_surfacing,
	packet_id_battery_level,
	packet_id_bouyancy,
	packet_id_rov_ctrl_pck,
	packet_id_cmd_nose,
	packet_id_gui_keep_alive,
	packet_id_shutdown,
	packet_id_stop_log,
	packet_id_sw_version,
	packet_id_cmd_dw,

	/*newly added commands */
	packet_id_cmd_gcs_location = 197,
	packet_id_cmd_hsdt = 199
} GuiPacketId_t;

typedef enum
{
	STD_ACK_MSG_NOT_RECOGNIZED = -1,
	STD_ACK_MSG_REFUSED = 0,
	STD_ACK_MSG_EXECUTED = 1,
	STD_ACK_MSG_NO_RESPONSE
} StdAckRetCode_t;

/***** outgoing frames ******/
/**
 * GuiFrameStdAck, This packet is sent as ACK to all request received
 * from GUI, except for TIME_SYNC_REQ, SW_VERSION_REQ and MOTOR_STATE_REQ messages.
 * feedback value can be,
 * -1 if wrong ID,Length or checksum.
 * 0 if request is not executed for some reason.
 * 1 if request is executed successfully
 */

// struct
//{
//	uint8_t feedback;
// } GuiFrameStdAck;

// struct
//{
//	uint8_t feedback;
// } GuiFrameTSyncAck;

#pragma pack(push, 1)

typedef union __attribute__((packed))
{
	unsigned long long int val;
	unsigned char bytes[8];
} GuiTime_t;

/* packet without header and footer */
typedef struct __attribute__((packed))
{
	unsigned char packetId;
	GuiTime_t timestamp;
	unsigned int length; // data length
	unsigned char data[128];
} GuiPacket_t;

typedef struct
	__attribute__((packed))
{
	uint8_t startMsg;
	uint8_t packetId;
	uint64_t timestamp;
	uint16_t length;
} GUI_MSG_HEADER;

typedef struct
	__attribute__((packed))
{
	uint8_t checksum;
	uint8_t endMsg;
} GUI_MSG_FOOTER;

typedef struct
	__attribute__((packed))
{
	GUI_MSG_HEADER header;

	uint8_t VuState;			 // 0:standby(default), 1:ROV, 2:AUV, 3:AutoTest
	uint16_t MissionOperationId; // 0:No mission Active, 65535:End Mission,65534:Abort, 65535:Error
	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t BatteryComm : 1;   // set to 1 if communication with battery is OK, 0 otherwise.
			uint8_t LB_Comm : 1;	   // set to 1 if communication with LB(Logic Board) is OK, 0 otherwise.
			uint8_t NCU_Comm : 1;	   // set to 1 if communication with NCU is OK, 0 otherwise.
			uint8_t Carris_Comm : 1;   // set to 1 if communication with CARRIS is OK, 0 otherwise.
			uint8_t Reserved_1 : 1;	   // not used.
			uint8_t Reserved_2 : 1;	   // not used.
			uint8_t LogFileActive : 1; // set to 1 if the Log File is active is OK, 0 otherwise.
			uint8_t Reserved_3 : 1;	   // not used.
		};
	} VuStatusB0; // Internal communication

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t WifiActive : 1; // set to 1 if WiFi supply is active, 0 otherwise.
			uint8_t WifiCommOk : 1; // set to 1 if WiFi communication is OK, 0 otherwise.
			uint8_t RfActive : 1;	// set to 1 if RF supply is active, 0 otherwise.
			uint8_t rfCommOk : 1;	// set to 1 if RF communication is OK, 0 otherwise.
			uint8_t UsblActive : 1; // set to 1 if USBL supply is active, 0 otherwise.
			uint8_t UsblCommOk : 1; // set to 1 if USBL communication is OK, 0 otherwise.
			uint8_t SatActive : 1;	// set to 1 if SATCOM supply is active, 0 otherwise.
			uint8_t SatCommOk : 1;	// set to 1 if SATCOM communication is OK, 0 otherwise.
		};
	} VuStatusB1; // External communication

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t MonSafetySwitch : 1; // set to 1 if "Safety Hw Airbag and Dropweight" is NOT PRESENT, 0 is PRESENT
			uint8_t AisEmSwitch : 1;	 // set to 1 if "Presence Emergency Battery" is OK, 0 is NOT PRESENT
			uint8_t BatteryCharger : 1;	 // set to 1 if "Battery Charger" is PRESENT, 0 is NOT PRESENT
			uint8_t MonKWD : 1;			 // set to 1 if "Airbag and Dropweight" is ACTIVE, 0 otherwise
			uint8_t Reserved_1 : 1;		 // not used.
			uint8_t VuSignal : 1;		 // set to 1 if Vital Unit(VU) Signal is PRESENT, 0 otherwise
			uint8_t NcuSignal : 1;		 // set to 1 if NCU Board Signal is PRESENT, 0 otherwise
			uint8_t CarrisSignal : 1;	 // set to 1 if CARRIS Board Signal is PRESENT, 0 otherwise
		};
	} LbStatusB0; // Logic Board (LB) byte0

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t DnSwitch_1 : 1;			  // set to 1 if "Micro Sgancio Naso" is ACTIVE, 0 otherwise
			uint8_t DnSwitch_2 : 1;			  // set to 1 if "Micro Aggancio Naso" is ACTIVE, 0 otherwise
			uint8_t BuoyancyPosition_Ok : 1;  // set to 1 if "BUOYANCY Position" is OK, 0 otherwise
			uint8_t Buoyancy_Fault : 1;		  // set to 1 if "BUOYANCY" is OK, 0 if fail
			uint8_t DW_Switch2 : 1;			  // set to 1 if "Micro Aggancio DW" is ACTIVE, 0 otherwise
			uint8_t FloodSwitch : 1;		  // set to 1 if "SW Flood" is ACTIVE, 0 otherwise
			uint8_t FailNavigationSwitch : 1; // set to 1 if "Navigation Switch" is Fail, 0 otherwise
			uint8_t FailScientificSwitch : 1; // set to 1 if "Scientific Switch" is Fail, 0 otherwise
		};
	} LbStatusB1; // Logic Board (LB) byte1

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t MBES_Sensor : 2; // 0:Off, 1:Startup, 2:Fault, 3:On
			uint8_t Reserved_1 : 1;	 // not used
			uint8_t Reserved_2 : 1;	 // not used
			uint8_t SSS_Sensor : 2;	 // 0:Off, 1:Startup, 2:Fault, 3:On
			uint8_t Reserved_3 : 1;	 // not used
			uint8_t Reserved_4 : 1;	 // not used
		};
	} CarrisStatusB0;

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t SBP_Sensor : 2;	   // 0:Off, 1:Startup, 2:Fault, 3:On
			uint8_t Reserved_1 : 1;	   // not used
			uint8_t Reserved_2 : 1;	   // not used
			uint8_t Camera_Sensor : 2; // 0:Off, 1:Startup, 2:Fault, 3:On
			uint8_t CTD_Sensor : 2;	   // 0:Off, 1:Startup, 2:Fault, 3:On
		};
	} CarrisStatusB1;

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t ThrusterWarning : 1;	  // Set to 1 if motor driver works correctly, 0 otherwise, see Byte1 for details.
			uint8_t CommunicationWarning : 1; // set to 1 if communication works correctly, 0 otherwise, see Byte2 for details.
			uint8_t GuidanceWarning : 1;	  // set to 1 if navigation/guidance procedure works correctly, 0 otherwise. see Byte3 for details.
			uint8_t Reserved : 5;			  // not used
		};
	} NcuStatusB0;

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t BackSX : 1;	   // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t BackDX : 1;	   // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t BackSide : 1;  // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t FrontSide : 1; // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t Backtop : 1;   // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t FrontTop : 1;  // set to 1 if motor driver works correctly, 0 otherwise
			uint8_t Reserved : 2;  // not used
		};
	} NcuStatusB1ThrusterWarn; // Thruster warning details

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t INS_Comm : 1;		// set to 1 if communication works correctly, 0 otherwise
			uint8_t EchoFrontComm : 1;	// set to 1 if communication works correctly, 0 otherwise
			uint8_t EchoSlopedComm : 1; // set to 1 if communication works correctly, 0 otherwise
			uint8_t EchoBottomComm : 1; // set to 1 if communication works correctly, 0 otherwise
			uint8_t LogicBoardComm : 1; // set to 1 if communication works correctly, 0 otherwise
			uint8_t VitalUnitComm : 1;	// set to 1 if communication works correctly, 0 otherwise
			uint8_t reserved : 2;		// set to 1 if communication works correctly, 0 otherwise
		};
	} NcuStatusB2CommWarn; // Communication warning details

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t ManualCmndTimeout : 1;	   // if a ROV_CTRL_PCK timeout occurs is reset to 0, set to 1 otherwise
			uint8_t ManualSurfaceRecovery : 1; // Reset to 0 during the automatic surface recovery when the vehicle
			// mode is MANUAL and the vehicle altitude is <-2m, set to 1 otherwise
			uint8_t SeaBedWarning : 1; // Reset to 0 if the depth/altitude current target involves a violation of safety
			// altitude limit above Sea Bed. The NCU takes the altitude safety limit as reference. set to 1 otherwise
			uint8_t SurfaceWarning : 1; // reset to 0 it the depth/altitude current target involves a violation of safety
			// altitude limit below Sea Level or if NCU estimates vehicle to be on the surface. The NUC takes the
			// altitude safety limit as reference. set to 1 otherwise.
			uint8_t ObstacleAvoidLock : 1; // reset to 0 if the Obstacle Avoidance Procedure is in lock state.
			// for example, The vehicle reaches the sea surface during obstacle avoidance procedure and there is
			// still an obstacle in front of the vehicle), set to 1 otherwise
			uint8_t CriticalRollLock : 1; // Reset to 0 during the  momentary navigation suspension due to exceeding
			// the critical roll threshold, set to 1 otherwise
			uint8_t Reserved : 2;
		};
	} NcuStatusB3GuidanceWarn; // navigation/Guidance warning details

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t ObstacleAvoidStatusCode : 3; // 0: Not Running, 1:Heave Phase, 2: wait On Bottom Echo Phase
			uint8_t Reserved : 5;				 // not used
		};
	} NcuStatusB4; // Obstacle Avoidance warning

	union __attribute__((packed))
	{
		uint8_t Val;
		struct
		{
			uint8_t GuidanceModeCode : 4; // 0:Disabled, 1:Manual,2:Goto depth reference,3:Goto altitude reference,
			// 4:Hovering,5:Surfacing,6:Floating,7:Direct,8:Heading
			uint8_t DepthMeasuresComparison : 3; // 0:All measures are coherent, 1: Altimeter measure is not coherent,
			// 2:CARIS measure is not coherent, 4:INS measure is not coherent, 7:All measures are not coherent
			//  Note: these are bitwise ORing flags for one or more condition occurred.
			uint8_t reserved : 1; // not used
		};
	} NcuStatusB5GuidanceStatus; // Navigation/Guidance status

	union __attribute__((packed))
	{
		uint16_t Val;
		struct
		{
			uint8_t Reserved : 1;				// not used
			uint8_t SolutionValidity : 1;		// 1:Invalid, 0:Valid
			uint8_t AccelerometerFail : 1;		// 1:Invalid, 0:Valid
			uint8_t GyroFail : 1;				// 1:Invalid, 0:Valid
			uint8_t AccelerometerOutrancge : 1; // 1:Invalid, 0:Valid
			uint8_t GyrosOutrange : 1;			// 1:Invalid, 0:Valid
			uint8_t GPS_Validity : 1;			// 1:Invalid, 0:Valid
			uint8_t DVL_Validity : 1;			// 1:Invalid, 0:Valid
			uint8_t USBL_Validity : 1;			// 1:Invalid, 0:Valid
			uint8_t GPS_Timeout : 1;			// 1:Timeout, 0:Valid
			uint8_t DVL_Timeout : 1;			// 1:Timeout, 0:Valid
			uint8_t USBL_Timeout : 1;			// 1:Timeout, 0:Valid
			uint8_t Reserved_2 : 4;				// not used
		};
	} NcuStatusB67;
	//	struct
	//	{
	//
	//	} NcuStatusB7;
	int32_t Lattitude;				// Vehicle latitude [deg 10*10e6]
	int32_t Longitude;				// Vehicle longitude [deg 10*10e6]
	int32_t Depth;					// Vehicle altitude below sea level [m*100]
	int16_t SpeedBody_X;			// Vehicle X body speed [m/s*100]
	int16_t SpeedBody_Y;			// Vehicle Y body speed [m/s*100]
	int16_t SpeedBody_Z;			// Vehicle Z body speed [m/s*100]
	int16_t Speed;					// Target speed [m/s*100]
	int16_t RollAngle;				// Vehicle Roll angle [rad*10000]
	int16_t PitchAngle;				// Vehicle Pitch angle [rad*10000]
	int16_t YawAngle;				// Vehicle Yaw angle [rad*10000]
	uint8_t AltimeterFront;			// distance[meter], set to 0 if measure is not good
	uint8_t AltimeterSloped;		// distance[meter], set to 0 if measure is not good
	uint8_t AltimeterBottom;		// distance[meter], set to 0 if measure is not good
	uint16_t BatteryVoltage_LB;		// Battery Voltage read Logic Board [V*100]
	uint16_t BatteryVoltage;		// Battery Voltage read Vital Unit [V*100]
	uint16_t BatteryCurrent;		// Battery Current read Vital Unit [A*100]
	uint32_t BatteryCapacityRemain; // Battery remaining capacity read Vital Unit [Ah*100]
	uint8_t BatteryCharge;			// Battery State Of Charge % read Vital Unit % [0 to 100]

	GUI_MSG_FOOTER footer;
} GuiFrameVuStatus_t;

typedef struct
	__attribute__((packed))
{
	int16_t rpm;		 // motor RPM
	uint8_t current;	 // driver input current [A*5]
	uint8_t voltage;	 // driver input voltage [v*2]
	uint8_t temperature; // driver temperature degree celsius
	int8_t command;		 // driver command from NCU [+-%]
} GuiThrusterData_t;

typedef struct
	__attribute__((packed))
{
	GUI_MSG_HEADER header;

	GuiThrusterData_t thrusterBSx;
	GuiThrusterData_t thrusterBDx;
	GuiThrusterData_t thrusterBSide;
	GuiThrusterData_t thrusterFSide;
	GuiThrusterData_t thrusterBTop;
	GuiThrusterData_t thrusterFTop;

	GUI_MSG_FOOTER footer;
} GuiFrameAckMotor_t;

typedef struct
	__attribute__((packed))
{
	uint8_t VuSwVersionLSB;
	uint8_t VuSwVersionMSB;
	uint8_t LbSwVersionLSB;
	uint8_t LbSwVersionMSB;
	uint8_t NcuSwVersionLSB;
	uint8_t NcuSwVersionMSB;
	uint8_t CarrisSwVersionLSB;
	uint8_t CarrisSwVersionMSB;
} GuiFrameAckVersion_t;

typedef union
	__attribute__((packed))
{
	uint16_t val;
	struct
	{
		uint8_t testOngoing : 1;
		uint8_t strobelight : 1;
		uint8_t leftPortFin : 1;
		uint8_t rightStarboardFin : 1;
		uint8_t topRudderFin : 1;
		uint8_t bottomRudderFin : 1;
		uint8_t thruster : 1;
		uint8_t reserved_2 : 1;

		uint8_t insMotusTest : 1;
		uint8_t inSpatialTest : 1;
		uint8_t echosounderTest : 1;
		uint8_t pressuresensorTest : 1;
		uint8_t bmsTest : 1;
		uint8_t gspFixed : 1;
		uint8_t pltSucceess : 1;
		uint8_t reserved_3 : 1;
	} b;
} GuiFramePreLaunchTestResult_t;

typedef struct
	__attribute__((packed))
{
	GUI_MSG_HEADER header;
	StdAckRetCode_t testResult; 
	GUI_MSG_FOOTER footer;
} GuiFrameAckStandard_t;

#pragma pack(pop)

#endif /* GUIWIFIFRAMES_H_ */
