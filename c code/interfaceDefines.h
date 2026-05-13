/*
 * interfaceDefines.h
 *
 *  Created on: May 31, 2023
 *      Author: root
 */

#ifndef INTERFACEDEFINES_H_
#define INTERFACEDEFINES_H_

#include <stdlib.h>
#include <termios.h>
#include <stdint.h>

/*defines*/
#ifdef __linux__
#define OK	0
#define ERROR 	-1
#define IMPORT	extern
#define TRUE	1
#define FALSE	0

//typedef int STATUS;

typedef signed char INT8;
typedef signed short INT16;
typedef signed int INT32;
typedef signed long long INT64;

typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef unsigned long long UINT64;

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;

//typedef unsigned short UINT16;
//typedef	unsigned int	UINT32;
//typedef int BOOL;
#endif

#define SAMPLE_TIME									0.02
#define PI											3.1415926535897932384626433832795
#define DEG_TO_RAD									(PI/180.00) // degree to radians
#define RAD_TO_DEG									(180.00/PI) // radians to degree
#define MIL_TO_DEG									0.05625 	/*multiply to convert mils to degrees*/
#define DEG_TO_MIL									17.777778 	/*multiply to convert degs to mils*/

#define USE_MOTUS_READINGS							1 // 1: use motus reading in control loops, 0: use spatial reading
#define UDP_DEBUG_ENABLE 							0 // 1: use UDP console debug, 0: udp raw logparams packet send

/********************************* TASK PRIORITY **************************/
#define FIN_SERVO_TASK_PRIORITY					80
#define THRUSTER_TASK_PRIORITY					83

#define INS_MOTUS_READ_TASK_PRIORITY			93
//#define INS_MOTUS_WRITE_TASK_PRIORITY			94
#define INS_SPATIAL_READ_TASK_PRIORITY			95

#define BMS_TASK_PRIORITY						70

#define PRESSURE_SENSOR_TASK_PRIORITY			72
#define ALTIMETER_TASK_PRIORITY					74

#define STROBELIGHT_TASK_PRIORITY				30
#define BUZZER_TASK_PRIORITY					31
#define WRITELOG_TASK_PRIORITY					32 // after all task handled
#define SATELLITE_READ_TASK_PRIORITY			36
#define SATELLITE_WRITE_TASK_PRIORITY			37
#define WIFI_READ_TASK_PRIORITY					40
#define WIFI_WRITE_TASK_PRIORITY				41
#define DEBUG_TASK_PRIORITY						42
#define OPERATION_MODE_TASK_PRIORITY			43
#define MAT_OPERATION_TASK_PRIORITY				44 // mat operations shall have higher priority than operationMode
#define PID_LOOP_TEST_TASK_PRIORITY				45

#define DIGITAL_INPUT_TASK_PRIORITY				98 //20		//69 to 59
#define DIGITAL_OUTPUT_TASK_PRIORITY			99

#define decoder_initialise(decoder, size, array) ((decoder)->buffer_length = 0, (decoder)->buffer_size = size,(decoder)->buffer = array )
#define decoder_pointer(decoder) &(decoder)->buffer[(decoder)->buffer_length]
#define decoder_size(decoder) ((decoder)->buffer_size - (decoder)->buffer_length)
#define decoder_increment(decoder, bytes_received) (decoder)->buffer_length += bytes_received
/***/
#define SET_BIT(val, pos)							(val|=(0x1<<pos))
#define RESET_BIT(val, pos)							(val&= ~(0x1<<pos))
/***/
//constants from text files
#define CONTROL_LOOP_SAMPLING_TIME		ctrlLoopParams.SmapleTimeInSeconds
/***/
#define WPN_AUTO_U_ORBIT_ERR_TIME		ctrlLoopParams.autoWPN.waypointOrbitErrorForTimeInMetres
#define WPN_AUTO_U_EXIT_ANGLE_ERR_DEG	ctrlLoopParams.autoWPN.waypointExitAngleErrorDeg
#define WPN_AUTO_U_KP_PATH				ctrlLoopParams.autoWPN.waypointKpPath
#define WPN_AUTO_U_KI_PATH				ctrlLoopParams.autoWPN.waypointKiPath
#define WPN_AUTO_U_KP_ORBIT				ctrlLoopParams.autoWPN.waypointKpOrbit
#define WPN_AUTO_U_KI_ORBIT				ctrlLoopParams.autoWPN.waypointKiOrbit
#define WPN_AUTO_U_INT_ACT_PATH_LIM_LO	ctrlLoopParams.autoWPN.waypointIntegralActionPathLowerLimit
#define WPN_AUTO_U_INT_ACT_PATH_LIM_HI	ctrlLoopParams.autoWPN.waypointIntegralActionPathHighLimit
#define WPN_AUTO_U_INT_ACT_ORBIT_LIM_LO	ctrlLoopParams.autoWPN.waypointIntegralActionOrbitLowerLimit
#define WPN_AUTO_U_INT_ACT_ORBIT_LIM_HI ctrlLoopParams.autoWPN.waypointIntegralActionOrbitHighLimit
/***/
#define WPN_GOTO_U_ORBIT_ERR_TIME		ctrlLoopParams.gotoWPN.waypointOrbitErrorForTimeInMetres
#define WPN_GOTO_U_EXIT_ANGLE_ERR_DEG	ctrlLoopParams.gotoWPN.waypointExitAngleErrorDeg
#define WPN_GOTO_U_KP_PATH				ctrlLoopParams.gotoWPN.waypointKpPath
#define WPN_GOTO_U_KI_PATH				ctrlLoopParams.gotoWPN.waypointKiPath
#define WPN_GOTO_U_KP_ORBIT				ctrlLoopParams.gotoWPN.waypointKpOrbit
#define WPN_GOTO_U_KI_ORBIT				ctrlLoopParams.gotoWPN.waypointKiOrbit
#define WPN_GOTO_U_INT_ACT_PATH_LIM_LO	ctrlLoopParams.gotoWPN.waypointIntegralActionPathLowerLimit
#define WPN_GOTO_U_INT_ACT_PATH_LIM_HI	ctrlLoopParams.gotoWPN.waypointIntegralActionPathHighLimit
#define WPN_GOTO_U_INT_ACT_ORBIT_LIM_LO	ctrlLoopParams.gotoWPN.waypointIntegralActionOrbitLowerLimit
#define WPN_GOTO_U_INT_ACT_ORBIT_LIM_HI ctrlLoopParams.gotoWPN.waypointIntegralActionOrbitHighLimit
/***/
#define WPN_HOLD_U_ORBIT_ERR_TIME		ctrlLoopParams.holdWPN.waypointOrbitErrorForTimeInMetres
#define WPN_HOLD_U_EXIT_ANGLE_ERR_DEG	ctrlLoopParams.holdWPN.waypointExitAngleErrorDeg
#define WPN_HOLD_U_KP_PATH				ctrlLoopParams.holdWPN.waypointKpPath
#define WPN_HOLD_U_KI_PATH				ctrlLoopParams.holdWPN.waypointKiPath
#define WPN_HOLD_U_KP_ORBIT				ctrlLoopParams.holdWPN.waypointKpOrbit
#define WPN_HOLD_U_KI_ORBIT				ctrlLoopParams.holdWPN.waypointKiOrbit
#define WPN_HOLD_U_INT_ACT_PATH_LIM_LO	ctrlLoopParams.holdWPN.waypointIntegralActionPathLowerLimit
#define WPN_HOLD_U_INT_ACT_PATH_LIM_HI	ctrlLoopParams.holdWPN.waypointIntegralActionPathHighLimit
#define WPN_HOLD_U_INT_ACT_ORBIT_LIM_LO	ctrlLoopParams.holdWPN.waypointIntegralActionOrbitLowerLimit
#define WPN_HOLD_U_INT_ACT_ORBIT_LIM_HI ctrlLoopParams.holdWPN.waypointIntegralActionOrbitHighLimit
/***/
#define WPN_HOME_U_ORBIT_ERR_TIME		ctrlLoopParams.homeWPN.waypointOrbitErrorForTimeInMetres
#define WPN_HOME_U_EXIT_ANGLE_ERR_DEG	ctrlLoopParams.homeWPN.waypointExitAngleErrorDeg
#define WPN_HOME_U_KP_PATH				ctrlLoopParams.homeWPN.waypointKpPath
#define WPN_HOME_U_KI_PATH				ctrlLoopParams.homeWPN.waypointKiPath
#define WPN_HOME_U_KP_ORBIT				ctrlLoopParams.homeWPN.waypointKpOrbit
#define WPN_HOME_U_KI_ORBIT				ctrlLoopParams.homeWPN.waypointKiOrbit
#define WPN_HOME_U_INT_ACT_PATH_LIM_LO	ctrlLoopParams.homeWPN.waypointIntegralActionPathLowerLimit
#define WPN_HOME_U_INT_ACT_PATH_LIM_HI	ctrlLoopParams.homeWPN.waypointIntegralActionPathHighLimit
#define WPN_HOME_U_INT_ACT_ORBIT_LIM_LO	ctrlLoopParams.homeWPN.waypointIntegralActionOrbitLowerLimit
#define WPN_HOME_U_INT_ACT_ORBIT_LIM_HI ctrlLoopParams.homeWPN.waypointIntegralActionOrbitHighLimit
/***/
#define DEPTH_PID_U_KP					ctrlLoopParams.depthPID.kp
#define DEPTH_PID_U_KD					ctrlLoopParams.depthPID.kd
#define DEPTH_PID_U_KI					ctrlLoopParams.depthPID.ki
#define DEPTH_PID_U_KW					ctrlLoopParams.depthPID.kw
#define DEPTH_PID_U_DER_ERR_LIM_LO  	ctrlLoopParams.depthPID.derivativeErrorLimitLo
#define DEPTH_PID_U_DER_ERR_LIM_HI		ctrlLoopParams.depthPID.derivativeErrorLimitHi
#define DEPTH_PID_U_INT_ERR_LIM_LO		ctrlLoopParams.depthPID.integralErrorLimitLo
#define DEPTH_PID_U_INT_ERR_LIM_HI		ctrlLoopParams.depthPID.integralErrorLimitHi
#define DEPTH_PID_U_RATE_LIM_LO			ctrlLoopParams.depthPID.rateLimitDown
#define DEPTH_PID_U_RATE_LIM_HI			ctrlLoopParams.depthPID.rateLimitUp
#define DEPTH_PID_U_SAT_LIM_LO			ctrlLoopParams.depthPID.antiWindupDown
#define DEPTH_PID_U_SAT_LIM_HI			ctrlLoopParams.depthPID.antiWindupUp
/***/
#define PITCH_PID_U_KP					ctrlLoopParams.pitchPID.kp
#define PITCH_PID_U_KD					ctrlLoopParams.pitchPID.kd
#define PITCH_PID_U_KI					ctrlLoopParams.pitchPID.ki
#define PITCH_PID_U_KW					ctrlLoopParams.pitchPID.kw
#define PITCH_PID_U_DER_ERR_LIM_LO  	ctrlLoopParams.pitchPID.derivativeErrorLimitLo
#define PITCH_PID_U_DER_ERR_LIM_HI		ctrlLoopParams.pitchPID.derivativeErrorLimitHi
#define PITCH_PID_U_INT_ERR_LIM_LO		ctrlLoopParams.pitchPID.integralErrorLimitLo
#define PITCH_PID_U_INT_ERR_LIM_HI		ctrlLoopParams.pitchPID.integralErrorLimitHi
#define PITCH_PID_U_RATE_LIM_LO			ctrlLoopParams.pitchPID.rateLimitDown
#define PITCH_PID_U_RATE_LIM_HI			ctrlLoopParams.pitchPID.rateLimitUp
#define PITCH_PID_U_SAT_LIM_LO			ctrlLoopParams.pitchPID.antiWindupDown
#define PITCH_PID_U_SAT_LIM_HI			ctrlLoopParams.pitchPID.antiWindupUp
/***/
#define YAW_PID_U_KP					ctrlLoopParams.yawPID.kp
#define YAW_PID_U_KD					ctrlLoopParams.yawPID.kd
#define YAW_PID_U_KI					ctrlLoopParams.yawPID.ki
#define YAW_PID_U_KW					ctrlLoopParams.yawPID.kw
#define YAW_PID_U_DER_ERR_LIM_LO  		ctrlLoopParams.yawPID.derivativeErrorLimitLo
#define YAW_PID_U_DER_ERR_LIM_HI		ctrlLoopParams.yawPID.derivativeErrorLimitHi
#define YAW_PID_U_INT_ERR_LIM_LO		ctrlLoopParams.yawPID.integralErrorLimitLo
#define YAW_PID_U_INT_ERR_LIM_HI		ctrlLoopParams.yawPID.integralErrorLimitHi
#define YAW_PID_U_RATE_LIM_LO			ctrlLoopParams.yawPID.rateLimitDown
#define YAW_PID_U_RATE_LIM_HI			ctrlLoopParams.yawPID.rateLimitUp
#define YAW_PID_U_SAT_LIM_LO			ctrlLoopParams.yawPID.antiWindupDown
#define YAW_PID_U_SAT_LIM_HI			ctrlLoopParams.yawPID.antiWindupUp
/***/
#define VELOCITY_PID_U_KP				ctrlLoopParams.velocityPID.kp
#define VELOCITY_PID_U_KD				ctrlLoopParams.velocityPID.kd
#define VELOCITY_PID_U_KI				ctrlLoopParams.velocityPID.ki
#define VELOCITY_PID_U_KW				ctrlLoopParams.velocityPID.kw
#define VELOCITY_PID_U_DER_ERR_LIM_LO  	ctrlLoopParams.velocityPID.derivativeErrorLimitLo
#define VELOCITY_PID_U_DER_ERR_LIM_HI	ctrlLoopParams.velocityPID.derivativeErrorLimitHi
#define VELOCITY_PID_U_INT_ERR_LIM_LO	ctrlLoopParams.velocityPID.integralErrorLimitLo
#define VELOCITY_PID_U_INT_ERR_LIM_HI	ctrlLoopParams.velocityPID.integralErrorLimitHi
#define VELOCITY_PID_U_RATE_LIM_LO		ctrlLoopParams.velocityPID.rateLimitDown
#define VELOCITY_PID_U_RATE_LIM_HI		ctrlLoopParams.velocityPID.rateLimitUp
#define VELOCITY_PID_U_SAT_LIM_LO		ctrlLoopParams.velocityPID.antiWindupDown
#define VELOCITY_PID_U_SAT_LIM_HI		ctrlLoopParams.velocityPID.antiWindupUp
/***/
#define ROLL_PID_LOOP_U_KP					ctrlLoopParams.rollPID.kp
#define ROLL_PID_LOOP_U_KD					ctrlLoopParams.rollPID.kd
#define ROLL_PID_LOOP_U_KI					ctrlLoopParams.rollPID.ki
#define ROLL_PID_LOOP_U_KW					ctrlLoopParams.rollPID.kw
#define ROLL_PID_LOOP_U_DER_ERR_LIM_LO  	ctrlLoopParams.rollPID.derivativeErrorLimitLo
#define ROLL_PID_LOOP_U_DER_ERR_LIM_HI		ctrlLoopParams.rollPID.derivativeErrorLimitHi
#define ROLL_PID_LOOP_U_INT_ERR_LIM_LO		ctrlLoopParams.rollPID.integralErrorLimitLo
#define ROLL_PID_LOOP_U_INT_ERR_LIM_HI		ctrlLoopParams.rollPID.integralErrorLimitHi
#define ROLL_PID_LOOP_U_RATE_LIM_LO			ctrlLoopParams.rollPID.rateLimitDown
#define ROLL_PID_LOOP_U_RATE_LIM_HI			ctrlLoopParams.rollPID.rateLimitUp
#define ROLL_PID_LOOP_U_SAT_LIM_LO			ctrlLoopParams.rollPID.antiWindupDown
#define ROLL_PID_LOOP_U_SAT_LIM_HI			ctrlLoopParams.rollPID.antiWindupUp
/***/
#define YAW_DRL_U_RATE_LIM_LO 		 		ctrlLoopParams.yawDRL.rateLimitLo
#define YAW_DRL_U_RATE_LIM_HI 		 		ctrlLoopParams.yawDRL.rateLimitUp
/***/
#define DEPTH_DRL_U_RATE_LIM_LO		 		ctrlLoopParams.depthDRL.rateLimitLo
#define DEPTH_DRL_U_RATE_LIM_HI		 		ctrlLoopParams.depthDRL.rateLimitUp
/***/
#define VELOCITY_DRL_U_RATE_LIM_LO	 		ctrlLoopParams.velocityDRL.rateLimitLo
#define VELOCITY_DRL_U_RATE_LIM_HI	 		ctrlLoopParams.velocityDRL.rateLimitUp
/***/
#define IsVehicleOnSurface()				(gSysVars.pressureData.surfaceDetected == TRUE)
/**/
#define BMS_NO_RESPONSE_TIME				gSysParams.bms_LinkFailTimeout //5 seconds
#define BMS_MIN_SOC_LIMIT					gSysParams.bms_MinimumRequiredSOC //20 percent
/***/

/**/
//#define BMS_NO_RESPONSE_TIME				gSysParams.bms_LinkFailTimeout //5 seconds
//#define BMS_MIN_SOC_LIMIT					gSysParams.bms_MinimumRequiredSOC //20 percent
/***/
/***/
#define IS_ALTIMETER_ENABLE()				(gSysParams.echosounder_enable == 1.0)
#define ALTIMETER_NO_RESPONSE_TIME_LIM		gSysParams.echosounder_LinkFailTimeout //5 seconds
#define ALTIMETER_MIN_CONFIDENCE_LVL		gSysParams.echosounder_ConfidenceLvlMin //80 percent
#define ALTIMETER_SEA_BED_DETECT_LVL		gSysParams.echosounder_SeaBedDetectLvl //100  meter
#define ALTIMETER_OUTOFRANGE_DETECT_LVL		gSysParams.echosounder_OutOfRangeLimit //310  meter
/***/
/***/
#define FIN_NO_RESPONSE_TIME_LIM			gSysParams.fins_LinkFailTimeout //5 seconds
#define FIN_POSITION_LIM_LO					0 //
#define FIN_POSITION_LIM_HI					16385 //
#define FIN_TEMPERATURE_LIM_LO				-57 // degree celsius
#define FIN_TEMPERATURE_LIM_HI				196 //degree celsius
#define FIN_TORQUE_LIM_LO					-90 // percent
#define FIN_TORQUE_LIM_HI					90 // percent
#define FIN_VOLTAGE_LIM_LO					1.0 // degree celsius
#define FIN_VOLTAGE_LIM_HI					6.0 //degree celsius
#define FIN_ANGLE_ERR_LIM					2.0 // 1 degree = 45.11 count
#define FIN_ANGLE_ERR_TIME_LIM				10 // seconds
/***/
//#define IsHILsEnable()						(gSysParams.simPlant == 1)
//#define SetHILsMode(mode)					gSysParams.simPlant = mode
//#define HILS_THRUST_MAX_LIM					10.0
//#define HILS_THRUST_MIN_LIM					0.0
/***/
#define MANUAL_MODE_IDLE_TIME_LIM			gSysParams.manualModeIdle_Timeout // second
#define MANUAL_CMND_NO_REPONSE_TIME_LIM		gSysParams.manualCommand_Timeout // second
#define MANUAL_MODE_SAFETY_DEPTH_LIM		gSysParams.manualModeDepthLimit // meters
#define MANUAL_MODE_SAFETY_PITCH_LIM		gSysParams.manualModePitchLimit // degree
#define MANUAL_MODE_SAFETY_ROLL_POS_LIM		gSysParams.manualModeRollMaxLimit // degree
#define MANUAL_MODE_SAFETY_ROLL_NEG_LIM		gSysParams.manualModeRollMinLimit // degree
#define MANUAL_MODE_EMERGNCY_THRUST_LIM		gSysParams.manualModeEmrgncyThrustLimit // -ve percent
#define MANUAL_MODE_EMERGNCY_THRUST_TIME_LIM		gSysParams.manualModeEmrgncyThrustTimeout // seconds
/***/
#define THRUSTER_NO_RESPONSE_TIME_LIM		gSysParams.thruster_LinkFailTimeout //seconds
#define THRUSTER_NON_ZERO_RPM_TIME_LIM		gSysParams.thruster_ZeroRPMtimeout//10 seconds
#define THRUSTER_MAX_POSITIVE_THRUST_LIM	gSysParams.thruster_MaxPositiveThrottle
#define THRUSTER_MAX_NEGATIVE_THRUST_LIM	gSysParams.thruster_MaxNegativeThrottle
#define THRUSTER_THROTTLE_OFFSET			gSysParams.thruster_ThrottleOffset
#define THRUSTER_SPEED_PER_SYSTICK			gSysParams.thruster_slewRate
/***/
#define VEHICLE_DEFAULT_SPEED				gSysParams.vehicle_DefaultSpeed
#define VEHICLE_DEFAULT_RADIUS				gSysParams.vehicle_DefaultRadius
#define VEHICLE_DEFAULT_DEPTH				PRESSURE_WATER_SURFACE_DEPTH

/***/
#define INS_NO_RESPONE_TIME_LIM				gSysParams.ins_LinkFailTimeout //10 // seconds
#define SPATIAL_NO_GPS_TIME_LIM				gSysParams.spatial_GPSFixTimeout //10 // seconds
/***/
/***/
#define IsHILsEnable()						(gSysParams.simPlant == 1.0)
//#define SetHILsMode(mode)					gSysParams.simPlant = mode
//#define HILS_THRUST_MAX_LIM					100.0
//#define HILS_THRUST_MIN_LIM					-100.0
/**/
#define IsPLS_Mandatory()					(gSysParams.IsPlcMandatory==1.0)
/***/
#define PRESSURE_NO_RESPONSE_TIME_LIM		gSysParams.pressureSensor_LinkFailTimeout //5 seconds
#define PRESSURE_WATER_SURFACE_DEPTH		gSysParams.pressureSensor_WaterSurfaceDetectLvl //12 meters
#define WATER_DENSITY						(gSysParams.waterDensity) //kg/m3
#define PRESSURE_OFFSET_IN_BAR				gSysParams.pressureSensorOffset
/***/
#define DEBUG_PRINT_TIME_IN_SECONDS			gSysParams.printParameterTimeoutInSeconds
#define IsDebugEnabled()					(gSysParams.printEnabled==1.0)
#define SetDebugMode(mode)					gSysParams.printEnabled = mode
/***/
#define DIVE_IN_THRUST_IN_PERCENT			gSysParams.divein_thrust // angle
#define DIVE_IN_PITCH_UP_FIN_ANGLE			gSysParams.divein_pitchUpFinAngle // angle
#define DIVE_IN_PITCH_DN_FIN_ANGLE			gSysParams.divein_pitchDownFinAngle // angle
#define DIVE_IN_PITCH_UP_IMU_FDBK 			gSysParams.divein_pitchFeedbackLmt // angle
#define DIVE_IN_DEPTH_IMU_FDBK				gSysParams.divein_depthFeedbackLmt // meter
/***/


#define SYSTEM_PARAM						"./vcuTextFiles/sysParam.txt"
#define PID_CONSTANTS						"./vcuTextFiles/PIDConstants.txt"
#define EMERGENCY_ACTION_FILE				"./vcuTextFiles/emergency_actions.csv"
#define IP_PARAM_FILE						"./vcuTextFiles/ip_details.txt"

#define PID_NO_OF_CONSTANTS			107
#define MAX_HSDT_MISSION_POINTS		50
#define MAX_AUTONAV_MISSION_POINTS	10 // depends on 'WPN_Auto_U_Path' in WPN_Auto_mat.h
#define AUTONAV_PARAMS_PER_MISSION	9 // this is fixed. DO NOT ALTER
//#define MAX_ROWS_PATH			10 // WPN_xxx_U_Path array rows, do not change
//#define MAX_COLS_PATH			9 // WPN_xxx_U_Path array columns, do not change

#define WPN_MAX_ROWS				MAX_AUTONAV_MISSION_POINTS
#define WPN_MAX_COLS				AUTONAV_PARAMS_PER_MISSION

#define PID_ERR_TIME_LIM					gSysParams.pidErrorCheckTimeout
#define YAW_ERR_LIM							gSysParams.yawPIDErrorLimit // degrees
#define YAW_ERR_TIME_LIM					PID_ERR_TIME_LIM // seconds
#define PITCH_ERR_LIM						gSysParams.pitchPIDErrorLimit // degrees
#define PITCH_ERR_TIME_LIM					PID_ERR_TIME_LIM // seconds
#define DEPTH_ERR_LIM						gSysParams.depthPIDErrorLimit // meters
#define DEPTH_ERR_TIME_LIM					PID_ERR_TIME_LIM // seconds
#define ROLL_ERR_LIM						gSysParams.rollPIDErrorLimit // degrees
#define ROLL_ERR_TIME_LIM					PID_ERR_TIME_LIM // seconds
/**/
#define SEMIAUTO_HSD_TIMEOUT_LIM_IN_SECONDS 60 //seconds, if new set of HSDT is not received within this time, goto Home
/**/


#include "interfaceStructs.h"
#include "GuiWiFiFrames.h"

#endif /* INTERFACEDEFINES_H_ */
