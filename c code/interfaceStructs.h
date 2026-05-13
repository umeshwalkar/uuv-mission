/*
 * interfaceStructs.h
 *
 *  Created on: Oct 31, 2023
 *      Author: root
 */

#ifndef INTERFACESTRUCTS_H_
#define INTERFACESTRUCTS_H_

#include "DigitalInput.h"
#include "DigitalOutput.h"
#include "echo_sounder.h"
#include "FinServo.h"
#include "BMS.h"
#include "ins_packets.h"

enum ErrorCodes
{
	NO_ERROR = 0,

	/*OperationMode*/
	MODE_ALREADY_RUNNING = 100,
	MODE_STANDBY_REQUIRED,
	MODE_EMERGENCY_CANNOT_OVERRIDE,
	MODE_PRELAUNCH_ONGOING,
	MODE_PRELAUNCH_FAIL,
	MODE_UNKNOWN,

	VEHICLE_NOT_ON_SURFACE
};

typedef enum
{
	HSDT_MISSION = 0, AUTONAV_MISSION
} MissionFileType_t;

typedef enum
{
	MISN_SUB_MOD_WPN = 0,
	MISN_SUB_MOD_GOTO,
	MISN_SUB_MOD_HOME,
	MISN_SUB_MOD_HOLD
} MissionSubMode_t;

#pragma pack (push,1)

typedef union
__attribute__((packed))
{
	short int Val;
	unsigned char byte[2];
} WORD_VAL;

typedef union
__attribute__((packed))
{
	int Val;
	short int sVal[2];
	unsigned char byte[4];
} DWORD_VAL;

typedef struct
__attribute__((packed))
{
	uint8_t *buffer;
	uint16_t buffer_length;
	uint16_t buffer_size;
	uint32_t crc_errors;
} decoder_t;

typedef struct
__attribute__((packed))
{
	float angleCommand; // in degree
	float angle; // in degree
	int16_t position; // related position, 0 to 16384, 4096=90 degree
	float angleDiff; // difference between expected and feedback angle/position
	float torque; // in percent, motor PWM duty, 0 to 4095, 4095=100%
	float voltage; // input voltage, 0 to 65535, 100=1.00 Volt
	float velocity; // velocity, 0 to 65535, pos/100mSec
	float current; // current, 0 to 65535, mA
	float temperature; // degree celsius, not reading yet
	union
	{
		uint8_t uVal;
		struct
		{
			char reserved15 :1;
			char overVoltage :1; //set when current voltage is higher than standard
			char underVoltage :1;//set when current voltage is lower than standard
			char reserved12 :1;
			char overMcuTemperature :1;//set when current MCU temp is higher than standard
			char underMcuTemperature :1;//set when current MCU temp is lower than standard
			char maxPosition :1;//set when current position is higher than max position
			char minPosition :1;//set when current position is lower than max position
			//			char reserved7_0 :8;  // considered only MSB of ERROR value, so commented
		}bit;
	}error;
	char linkStatus;
	int timeSinceLinkDownInMs;
	int nonOperationalError; // set when no-response, emergency_stop, angle/position error
} FinData_t;

typedef struct
__attribute__((packed))
{
	float thrustCommand; // in percent
	int32_t rpm; // rpm
	int32_t voltage;
	int32_t current;
	int32_t temperature;
	int32_t thrust;
	int32_t error;
	char linkStatus;
	int timeSinceLinkDownInMs;
	int nonOperationalError; // set when pressure not responding, values are out of range
} ThrusterData_t;

typedef struct
__attribute__((packed))
{
	float pressure; // BAR
	float temperature; // degree celsius
	float depth; // meters
	char linkStatus; // set when sensor not responding
	int timeSinceLinkDownInMs;
	int surfaceDetected;
	int outOfRangeDetected; // set when pressure sensor reading out of range
	int nonOperationalError; // set when pressure not responding, values are out of range
} PressureSensorData_t;

typedef struct
__attribute__((packed))
{
	system_state_packet_t system_state_packet;
	raw_sensors_packet_t raw_sensors_packet;
	char linkStatus;
	int timeSinceLinkDownInMs;
	int nonOperationalError; // set when no-response, emergency_stop, angle/position error
} InsData_t;

typedef struct
__attribute__((packed))
{
	//	float rssiLevel; // not receiving
	char linkStatus;
	int timeSinceLinkDownInMs;
	int nonOperationalError; // set when no wifi communication while on surface
} WiFiData_t;

typedef struct
__attribute__((packed))
{
	MissionFileType_t missionType; // 1: consider waypoint navigation, 0:HSDT navigation
	MissionSubMode_t missionSubMode; // 0:WPN Nav, 1: goto wp, 2: goto home, 3: hold mode

	/*GCS location lat lon*/
	float gcsLat;
	float gcsLon;

	/*vehicle recovery lat,lon,depth*/
	float homeLat;
	float homeLon;
	float homeDepth;

	/*vehicle goto lat,lon,depth*/
	float gotoLat;
	float gotoLon;
	float gotoDepth;

	/* data received from GCS for Manual mode */
	float throttleManualValue;
	float topRudderManualValue;
	float bottomRudderManualValue;
	float portFinManualValue;
	float starboardFinManualValue;
	uint32_t timeoutTickManual; // tracking time elapsed since last Manual command received

	/* data received from GCS for SemiAuto mode HSDT*/
	float depthSemiAutoValue;
	float yawSemiAutoValue;
	float velocitySemiAutoValue;
	float durationSemiAutoValueInSec;
	//	unsigned int timeoutTickSemiAuto;
	//	unsigned int newSemiAutoValueReceived;

	/* data from connected sensor converted before feeding to PIDs */
	float yawrateFeedback;
	float rollrateFeedback;
	float pitchrateFeedback;

	float yawFeedback;
	float rollFeedback;
	float pitchFeedback;
	float velocityFeedback;
	float depthFeedback;

	/***** CB-3: Dynamic Rate Limiter *****/
	float yawExpected; //InDRL, in radians
	float depthExpected; //InDRL, in meters
	float velocityExpected; // in meter/seconds
	float pitchExpected; //InDRL, in radians

	float yawOutDRL; // in radians
	float depthOutDRL; // in meters
	float velocityOutDRL; // in meter/seconds

	float yawRateVsSpeed;

	unsigned int totalHSDT_waypoints;
	unsigned int totalAutoNav_waypoints;

	//	int altimeterDistance;
	//	uint8_t altimeterConfidence;

	//	float pressureValue;
	//	uint8_t waterTemperature;

	//	uint8_t gpsFixStatus;

	//	float lattitude;
	//	float longitude;
	//	float depthHeight;
	//	float rollRate;
	//	float pitchRate;
	//	float yawRate; // in radians/seconds
	//	float roll;
	//	float pitch;
	//	float yaw; // in radians
	//	float speed;

	uint8_t phyDI[40];
	uint8_t phyDO[16];

	PressureSensorData_t pressureData;
	AltimeterData_t altimeterData;
	BMSData_t bmsData;
	FinData_t finData[MAX_SERVO];
	ThrusterData_t thrusterData;
	InsData_t insMotus;
	InsData_t insSpatial;
	WiFiData_t wifi;
} SystemVars_t;

typedef struct
__attribute__((packed))
{
	uint8_t consoleDebugEnable :1;
	uint8_t udpDebugEnable :1;
	uint8_t modeHSDTorWPN :1; // 1: HSDT , 0: WPN generated values
	uint8_t disablePID :1; // 1: do not use control loop, 0: use control loop

	uint8_t startLoggingFlag :1;

	//	uint8_t altimeterComm;
	//	uint8_t lowConfidenceEcho;
	uint8_t pressureSensorComm :1;
	//	uint8_t pitchUncontrolFlag;
	//	uint8_t depthUncontrolFlag;
	//	uint8_t depthOutOfRangeFlag;
	//	uint8_t headingUncontrolFlag;
	//	uint8_t seabedDetectionFlag;
	//	uint8_t insSpatialCommFlag;
	//	uint8_t insMotusCommFlag;
	//	uint8_t insSystemFailureFlag;
	//	uint8_t insGyroFailureFlag;
	//	uint8_t insAccelFailureFlag;
	//	uint8_t insMagnetoFailureFlag;

	uint8_t heartbeatEnable :1; // instruction to activate heartbeat
	uint8_t strobelightEnable :1; // instruction to activate strobelight
	uint8_t dropweightEnable :1; // instruction to enable dropweight

	uint8_t leakDetected :1;
	uint8_t emergencySwitchPressed :1;

	uint8_t IsHomeLocationAvailable :1;
	uint8_t IsEmergncyActivated :1;
	uint8_t IsHSDT_MissionPointsAvailable :1;
	uint8_t IsAutoNav_MissionPointsAvailable :1;
	uint8_t IsPfcCheckSuccess :1;
	uint8_t autoModeMissionStarted :1;
	uint8_t autoModeMissionAbort :1;
	uint8_t newSemiAutoValueReceived :1;
	uint8_t manualModeEmergency:1;

	uint8_t yawPIDEnable :1;
	uint8_t depthPIDEnable :1;
	uint8_t pitchPIDEnable :1;
	uint8_t velocityPIDEnable :1;
	uint8_t rollPIDEnable : 1;

	uint8_t yawPIDError :1;
	uint8_t pitchPIDError :1;
	uint8_t depthPIDError :1;
	uint8_t rollPIDError :1;

	uint8_t resetRollPID_IntegralFlag :1;
	uint8_t resetPitchPID_IntegralFlag :1;
	uint8_t resetDepthPID_IntegralFlag :1;
	uint8_t resetYawPID_IntegralFlag :1;
	uint8_t resetVelocityPID_IntegralFlag :1;

	uint8_t resetWPN_AutoIntegralFlag :1;
	uint8_t resetWPN_GoToIntegralFlag :1;
	uint8_t resetWPN_HomeIntegralFlag :1;
	uint8_t resetWPN_HoldIntegralFlag :1;

	uint8_t reentryWPN_AutoFlag :1;
	uint8_t reentryWPN_GoToFlag :1;
	uint8_t reentryWPN_HoldFlag :1;
	uint8_t reentryWPN_HomeFlag :1;

} SystemFlags_t;

#define MAX_SYSTEM_PARAMS	45
typedef union
__attribute__((packed))
{
	float val[MAX_SYSTEM_PARAMS];
	struct __attribute__((packed))
	{
		/* DO NOT CHANGE SEQUENCE OF ELEMENTS */
		/* ADD NEW ELEMENTS AT LAST IN THE LIST */

		//	int labBoot;
		float simPlant; // 1: hils mode, 0: regular run mode
		float missionType; // 1:waypoint navigation, 0:HSDT navigation

		float manualModeIdle_Timeout; // seconds
		float manualCommand_Timeout; // seconds
		float manualModeDepthLimit; // meters
		float manualModePitchLimit; // degree
		float manualModeRollMaxLimit; // degree
		float manualModeRollMinLimit; // degree

		float manualModeEmrgncyThrustLimit; // percent in -ve value
		float manualModeEmrgncyThrustTimeout; // percent in -ve value
		float thruster_MaxPositiveThrottle; // in percent
		float thruster_MaxNegativeThrottle; // in percent

		float echosounder_enable; // 1: enable, 0:disable
		float echosounder_LinkFailTimeout;
		float echosounder_ConfidenceLvlMin;
		float echosounder_SeaBedDetectLvl;
		float echosounder_OutOfRangeLimit;

		float pressureSensor_LinkFailTimeout;
		float pressureSensor_WaterSurfaceDetectLvl;
		float pressureSensorOffset;
		float waterDensity; //kg/m3

		float bms_LinkFailTimeout;
		float bms_MinimumRequiredSOC;

		float ins_LinkFailTimeout;
		float spatial_GPSFixTimeout;

		float thruster_LinkFailTimeout;
		float thruster_ZeroRPMtimeout;
		float thruster_ThrottleOffset;
		float thruster_slewRate;

		float fins_LinkFailTimeout;
		float printParameterTimeoutInSeconds;
		float printEnabled; // 1: enable, 0:disable

		float vehicle_DefaultSpeed; // meter/seconds
		float vehicle_DefaultRadius; // meters
		float IsPlcMandatory;

		float divein_thrust; // percent
		float divein_pitchUpFinAngle; // degree
		float divein_pitchDownFinAngle; // degree
		float divein_pitchFeedbackLmt; // degree
		float divein_depthFeedbackLmt; // meter

		float yawPIDErrorLimit; // angle in degree
		float rollPIDErrorLimit; // angle in degree
		float pitchPIDErrorLimit; // angle in degree
		float depthPIDErrorLimit; // in meter
		float pidErrorCheckTimeout; // in seconds

	};
} SystemParams_t;

typedef struct
__attribute__((packed))
{
	/* DO NOT CHANGE SEQUENCE OF ELEMENTS */
	float waypointOrbitErrorForTimeInMetres;
	float waypointExitAngleErrorDeg; //rad
	float waypointKpPath;
	float waypointKiPath;
	float waypointKpOrbit;
	float waypointKiOrbit;
	float waypointIntegralActionPathLowerLimit; // m
	float waypointIntegralActionPathHighLimit; // m
	float waypointIntegralActionOrbitLowerLimit;
	float waypointIntegralActionOrbitHighLimit;
	//	float waypointReEntryFlag;
} wpnConstants_t;

typedef struct
__attribute__((packed))
{
	/* DO NOT CHANGE SEQUENCE OF ELEMENTS */
	float kp;
	float kd;
	float ki;
	float kw;
	float derivativeErrorLimitLo;
	float derivativeErrorLimitHi;
	float integralErrorLimitLo;
	float integralErrorLimitHi;
	float rateLimitUp;
	float rateLimitDown;
	float antiWindupUp;
	float antiWindupDown;
} pidConstants_t;

typedef struct
__attribute__((packed))
{
	/* DO NOT CHANGE SEQUENCE OF ELEMENTS */
	float rateLimitUp;
	float rateLimitLo;

} DyanmicRateLimitter_t;

#define PID_NO_OF_CONSTANTS			107
typedef union
__attribute__((packed))
{
	float val[PID_NO_OF_CONSTANTS];

	struct __attribute__((packed))
	{
		/* DO NOT CHANGE SEQUENCE OF ELEMENTS */
		float SmapleTimeInSeconds; // common for all blocks

		/* CB-1: Waypoint, 10 * 4 = 40 params */
		wpnConstants_t autoWPN;
		wpnConstants_t gotoWPN;
		wpnConstants_t holdWPN;
		wpnConstants_t homeWPN;

		/* CB-2: Mode Selection*/

		/***** manual mode values *****/

		/* CB-3: Dynamic Rate Limiter, 2*3 = 6 params */
		DyanmicRateLimitter_t yawDRL;
		DyanmicRateLimitter_t depthDRL;
		DyanmicRateLimitter_t velocityDRL;

		/***** CB-6 to CB10: PID YAW,Roll,Pitch,Depth and Velocity, 12*5 = 60 params *****/
		pidConstants_t depthPID;
		pidConstants_t pitchPID;
		pidConstants_t yawPID;
		pidConstants_t velocityPID;
		pidConstants_t rollPID;
	};
} ControlLoopsParams_t;

typedef struct
__attribute__((packed))
{
	float heading; // degree angle
	float speed; // meter/seconds
	float depth; // meter
	uint32_t duration; // seconds
} HSDTpacket_t;

typedef struct
__attribute__((packed))
{
	float lat;
	float lon;
	float depthAltitude;
	float speed;
	uint8_t radius;
} GoToWP_t;

typedef struct
__attribute__((packed))
{
	float lat; // latitude in degree
	float lon; // longitude in degree
	float northXaxis; // north(x axis) from GCS in meter
	float eastYaxis; // east(y axis) from GCS in meter
	float wpType; // waypoint type, 1:line, 0:circle
	float radius; // the imaginary spherical area surrounding waypoint used as tolerance to consider reach.
	float lambda; // 1:clockwise for circular motion, -1:anti clockwise circular motion
	float exitOrbit; // 0:exit at defined angle, 1: exit after defined time
	float exitTimeAngle; // angle or time value as per 'exitOrbit'
	float depth; //meter
	float speed; //meter/sec
} AuoNavWP_t;

typedef struct
__attribute__((packed))
{
	float lat;
	float lon;
} HomeLocation_t;

typedef struct
__attribute__((packed))
{
	uint8_t pidEnable; // use of PID, 1: Enable, 0: Disable
	uint8_t buzzerON; // Buzzer 1: ON, 0:OFF
	uint8_t flaherON; // Strobelight 1: ON, 0:OFF
	uint8_t pingerON; // 1: ON, 0:OFF
	//	uint8_t buzzerDuraion; // buzzer ON duration in seconds or num of beeps
	uint8_t dropweight; // 1: trigger emergency break relay
	float speedOrThrust; // thruster speed in meter/sec OR thrust in percent.
	int8_t depthOrPitchOrAngleDeg; // depth in meters OR pitch in degree angle OR fin angle in degree
	int8_t rudderFinAngleDeg; // top rudder fin angle in degrees
	uint8_t durationSec; // duration in seconds before turning off thruster and fins
	//ToDo:
	// uint8_t leftFinEnable; // enabled FIN are considered for action
} EmergencyActions_t;
#pragma pack(pop)

#endif /* INTERFACESTRUCTS_H_ */
