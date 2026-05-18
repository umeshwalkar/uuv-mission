#ifndef _VehicleData_H
#define _VehicleData_H

#include <cmath>
#include <array>

enum class OperationMode
{
    OP_MODE_STANDBY, // = 0,
    //	OP_MODE_ROV = 1,
    //	OP_MODE_AUV = 2,
    //	OP_MODE_DRIECT_ROV = 3,
    OP_MODE_SEMI_AUTONOMOUS, /*OP_MODE_DRIECT_ROV*/
    OP_MODE_AUTONOMOUS,      /*OP_MODE_AUV*/
    OP_MODE_MANUAL,          /*OP_MODE_ROV*/

    /*DO NOT CHANGE ABOVE SEQUENCE */

    OP_MODE_PRELAUNCH_TEST,
    OP_MODE_EMERGENCY,
    OP_MODE_LAUNCHING,
    OP_MODE_UNKNOWN
};

enum FINS
{
    LEFT = 0, /* Port Fin*/
    RIGHT,    /* Starboard Fin*/
    TOP,      /* Top Rudder*/
    BOTTOM,   /*Bottom Rudder*/
    MAX_FINS  /* Total FINs in this system */
};

enum THRUSTERS
{
    SURGE_1 = 0, /* Surge Thruster 1*/
    // SURGE_2,      /* Surge Thruster 2 */
    // FRONT_VERT,   /* Front Vertical Thruster */
    // FRONT_SIDE,   /* Front Side Thruster */
    // BACK_VERT,    /* Back Vertical Thruster */
    // BACK_SIDE,    /* Back Side Thruster */
    MAX_THRUSTERS /* Total Thrusters in this system */
};

#pragma pack(push, 1)

/**
 * Mission Command Structure
 */
// struct MissionCommand
// {
//     std::string command; // e.g., "GOTO", "HOLD", "SURFACE"
//     double latitude;     // Target latitude (for GOTO)
//     double longitude;    // Target longitude (for GOTO)
//     double depth;        // Target depth (for GOTO)
//     double heading;      // Target heading (for GOTO)
//     int duration;        // Duration in seconds (for HOLD)

//     MissionCommand() : command(""), latitude(0), longitude(0), depth(0), heading(0), duration(0) {}
// };

// struct TelemetryData
// {
//     std::string operation_mode;
//     double latitude;   // in degrees
//     double longitude;  // in degrees
//     double depth;      // in meters
//     double speed;      // in m/s
//     double velocity_x; // in m/s
//     double velocity_y; // in m/s
//     double velocity_z; // in m/s
//     double heading;    //  in degrees
//     double pitch;      // in degrees
//     double roll;       // in degrees

//     TelemetryData() : operation_mode(""), latitude(0), longitude(0), depth(0),
//                       speed(0), velocity_x(0), velocity_y(0), velocity_z(0), heading(0), pitch(0), roll(0) {}
// };

// struct finFeedbackData
// {
//     float angleCommanded; // in degree, range -30 to +30
//     float angle;          // range -30 to +30 degree
//     float voltage;        // input voltage, 0 to 65535, 100=1.00 Volt
//     float current;        // current, 0 to 65535, mA
//     int16_t position;
//     float temperature; // degree celsius, not reading yet
//     unsigned short status;
// };

// struct thrusterFedbackData
// {
//     double thrustCommanded; // Range -100 to +100
//     int32_t rpm;            // Range -1000 to +1000
//     int32_t voltage;
//     int32_t current;
//     int32_t temperature;
//     int32_t thrust;
//     int32_t errorCode;
// };

// struct NavControlData
// {

//     thrusterFedbackData thruster;
//     finFeedbackData port;
//     finFeedbackData starboard;
//     finFeedbackData topRudder;
//     finFeedbackData bottomRudder;
// };

// struct BatteryData
// {
//     uint16_t voltage;        // in millivolts
//     uint16_t current;        // in amps
//     uint16_t capacityRemain; // in Ah
//     uint8_t chargePercent;   // 0 to 100%
// };

struct ConfigData
{
    char guiWiFiIP[32];     // IP address of remote server
    uint16_t guiWiFiTxPort; // udp port number of remoter server to send data to
    uint16_t guiWiFiRxPort; // udp port number to listen for incoming data from remote server
};

// struct SystemData_t
// {
//     TelemetryData telemetry;
//     thrusterFedbackData thruster;
//     finFeedbackData port;
//     finFeedbackData starboard;
//     finFeedbackData topRudder;
//     finFeedbackData bottomRudder;
//     BatteryData bms;
// };

struct SystemStatePacket
{
    OperationMode operationMode;
    uint8_t missionType;    // 1: consider waypoint navigation, 0:HSDT navigation
    uint8_t missionSubMode; // 0:WPN Nav, 1: goto wp, 2: goto home, 3: hold mode
};

struct AttitudeData
{
    float roll = 0;
    float pitch = 0;
    float yaw = 0;

    float rollRate = 0;
    float pitchRate = 0;
    float yawRate = 0;

    bool operator!=(
        const AttitudeData &other) const
    {
        constexpr float THRESH = 0.01f;

        return fabs(roll - other.roll) > THRESH ||
               fabs(pitch - other.pitch) > THRESH ||
               fabs(yaw - other.yaw) > THRESH ||

               fabs(rollRate - other.rollRate) > THRESH ||
               fabs(pitchRate - other.pitchRate) > THRESH ||
               fabs(yawRate - other.yawRate) > THRESH;
    }
};

struct NavigationData
{
    double latitude = 0;
    double longitude = 0;
    uint32_t distanceToTarget = 0;

    float altitude = 0;
    float depth = 0;

    float speed = 0;
    float velocityNorth = 0;
    float velocityEast = 0;
    float velocityDown = 0;
    bool target_acheived = false;
    
    bool operator!=(
        const NavigationData &other) const
    {
        constexpr float THRESH = 0.01f;

        return (fabs(latitude - other.latitude) > THRESH) ||
               (fabs(longitude - other.longitude) > THRESH) ||
               (fabs(altitude - other.altitude) > THRESH) ||
               (fabs(depth - other.depth) > THRESH) ||
               (fabs(speed - other.speed) > THRESH) ||
               (fabs(velocityNorth - other.velocityNorth) > THRESH) ||
               (fabs(velocityEast - other.velocityEast) > THRESH) ||
               (fabs(velocityDown - other.velocityDown) > THRESH) ||
               (distanceToTarget != other.distanceToTarget);
    }
};

struct ThrusterData
{
    static constexpr int COUNT = THRUSTERS::MAX_THRUSTERS; // 4;

    std::array<int32_t, COUNT> rpm{};
    std::array<int32_t, COUNT> thrust{};
    std::array<int32_t, COUNT> voltage{};
    std::array<int32_t, COUNT> current{};
    std::array<int32_t, COUNT> temperature{};

    bool operator!=(
        const ThrusterData &other) const
    {
        for (size_t i = 0; i < COUNT; i++)
        {
            if ((rpm[i] != other.rpm[i]) ||
                (thrust[i] != other.thrust[i]) ||
                (voltage[i] != other.voltage[i]) ||
                (current[i] != other.current[i]) ||
                (temperature[i] != other.temperature[i]))
            {
                return true;
            }
        }

        return false;
    }
};

struct FinsData
{
    static constexpr int COUNT = FINS::MAX_FINS;

    // FLOATS
    std::array<float, COUNT> angle{};
    std::array<float, COUNT> temperature{};
    std::array<float, COUNT> voltage{};
    std::array<float, COUNT> current{};

    // INTEGERS
    std::array<int16_t, COUNT> position{};
    // std::array<int16_t, COUNT> temperature{};

    // STATUS FLAGS
    std::array<unsigned short, COUNT> status{};

    // COMPARISON OPERATOR
    bool operator!=(
        const FinsData &other) const
    {
        // THRESHOLDS
        constexpr float ANGLE_THRESH = 0.1f;
        constexpr float TEMP_THRESH = 0.5f;
        constexpr float VOLTAGE_THRESH = 0.1f;
        constexpr float CURRENT_THRESH = 0.05f;

        for (size_t i = 0; i < COUNT; i++)
        {
            // FLOAT ARRAYS
            // ANGLE
            if (std::fabs(
                    angle[i] - other.angle[i]) > ANGLE_THRESH)
            {
                return true;
            }

            // TEMPERATURE
            if (std::fabs(
                    temperature[i] - other.temperature[i]) > TEMP_THRESH)
            {
                return true;
            }

            // VOLTAGE
            if (std::fabs(
                    voltage[i] - other.voltage[i]) > VOLTAGE_THRESH)
            {
                return true;
            }

            // CURRENT
            if (std::fabs(
                    current[i] - other.current[i]) > CURRENT_THRESH)
            {
                return true;
            }

            // INTEGER ARRAYS
            // SERVO POSITION
            if (position[i] != other.position[i])
            {
                return true;
            }

            // if (temperature[i]
            //     != other.temperature[i])
            // {
            //     return true;
            // }

            // STATUS FLAGS
            if (status[i] != other.status[i])
            {
                return true;
            }
        }

        return false;
    }
};

struct BatteryData
{
    uint16_t voltage;        // in millivolts
    uint16_t current;        // in amps
    uint16_t capacityRemain; // in Ah
    uint8_t chargePercent;   // 0 to 100%

    bool operator!=(
        const BatteryData &other) const
    {

        if ((voltage != other.voltage) ||
            (current != other.current) ||
            (capacityRemain != other.capacityRemain) ||
            (chargePercent != other.chargePercent))
        {
            return true;
        }

        return false;
    }
};

struct SystemStatus
{
    OperationMode mode =
        OperationMode::OP_MODE_STANDBY;

    bool armed = false;

    bool leakDetected = false;

    bool lowBattery = false;

    uint8_t currentMisnLegPerforming = 0;
    uint8_t totalMisnlegs = 0;

    // ToDo: Add comparison operator if needed for command data change detection
    //  COMPARISON OPERATOR
    bool operator!=(
        const SystemStatus &other) const
    {
        // check mode
        return true; // for now, always return true to indicate command has changed. Implement proper comparison logic as needed.

        // return false;
    }
};

struct CommandData
{
    OperationMode mode =
        OperationMode::OP_MODE_STANDBY;

    // home latitude and longitude
    double homeLatitude = 0;
    double homeLongitude = 0;

    // ROV or Semi-autonomous mode commands
    float commandedDepth_semi = 0;
    float commandedHeading_semi = 0;
    float commandedSpeed_semi = 0;
    uint32_t commandedDuration_semi = 0;

    // Direct ROV or manual mode commands
    std::array<int32_t, 1> commandedThrust{};
    std::array<float, 4> commandedFinAngle{};

    // Autonomous mode commands
    double targetLatitude = 0;
    double targetLongitude = 0;
    float targetDepth = 0;
    float targetSpeed = 0;
    float targetRadius = 0; // for waypoint navigation, radius to consider waypoint reached
    
    // ToDo: Add more command fields as needed for different mission types and modes

    // ToDo: Add comparison operator if needed for command data change detection
    //  COMPARISON OPERATOR
    bool operator!=(const CommandData &other) const
    {
        // check mode
        return true; // for now, always return true to indicate command has changed. Implement proper comparison logic as needed.

        // return false;
    }
};

struct AutoMissionData
{
    static constexpr int COUNT = 50;

    std::array<double, COUNT> misnlatitude{};
    std::array<double, COUNT> misnLongitude{};
    std::array<float, COUNT> misnDepthAltitude{};
    std::array<float, COUNT> misnSpeed{};

    std::array<uint8_t, COUNT> misnRadius{};
    // std::array<int16_t, COUNT> temperature{};

    // std::array<unsigned short, COUNT> status{};

    bool operator!=(const AutoMissionData &other) const
    {
        // check mode
        return true; // for now, always return true to indicate command has changed. Implement proper comparison logic as needed.

        // return false;
    }
};

#pragma pack(pop)

#endif // _VehicleData_H