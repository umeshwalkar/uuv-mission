#ifndef _STRUCTURES_H
#define _STRUCTURES_H

/**
 * Mission Command Structure
 */
struct MissionCommand
{
    std::string command; // e.g., "GOTO", "HOLD", "SURFACE"
    double latitude;     // Target latitude (for GOTO)
    double longitude;    // Target longitude (for GOTO)
    double depth;        // Target depth (for GOTO)
    double heading;      // Target heading (for GOTO)
    int duration;        // Duration in seconds (for HOLD)

    MissionCommand() : command(""), latitude(0), longitude(0), depth(0), heading(0), duration(0) {}
};

struct TelemetryData
{
    std::string operation_mode;
    double latitude;   // in degrees
    double longitude;  // in degrees
    double depth;      // in meters
    double speed;      // in m/s
    double velocity_x; // in m/s
    double velocity_y; // in m/s
    double velocity_z; // in m/s
    double heading;    //  in degrees
    double pitch;      // in degrees
    double roll;       // in degrees

    TelemetryData() : operation_mode(""), latitude(0), longitude(0), depth(0),
                      heading(0), pitch(0), roll(0), speed(0), velocity_x(0), velocity_y(0), velocity_z(0) {}
};

struct finFeedbackData
{
    float angleCommanded; // in degree, range -30 to +30
    float angle; // range -30 to +30 degree
    float voltage;// input voltage, 0 to 65535, 100=1.00 Volt
    float current; // current, 0 to 65535, mA
    int16_t position;
    float temperature; // degree celsius, not reading yet
    unsigned short status;
};

struct thrusterFedbackData
{
    double thrustCommanded; // Range -100 to +100
    int32_t rpm;               // Range -1000 to +1000
    int32_t voltage;
    int32_t current;
    int32_t temperature;
    int32_t thrust;
    int32_t errorCode;
};

struct NavControlData
{

    thrusterFedbackData thruster;
    finFeedbackData port;
    finFeedbackData starboard;
    finFeedbackData topRudder;
    finFeedbackData bottomRudder;
};

struct BatteryData
{
    uint16_t voltage;        // in millivolts
    uint16_t current;        // in amps
    uint16_t capacityRemain; // in Ah
    uint8_t chargePercent;   // 0 to 100%
};

struct ConfigData
{
    char guiWiFiIP[32];     // IP address of remote server
    uint16_t guiWiFiTxPort; // udp port number of remoter server to send data to
    uint16_t guiWiFiRxPort; // udp port number to listen for incoming data from remote server
};

struct SystemData_t
{
    TelemetryData telemetry;
    thrusterFedbackData thruster;
    finFeedbackData port;
    finFeedbackData starboard;
    finFeedbackData topRudder;
    finFeedbackData bottomRudder;
    BatteryData bms;
};

#endif // _STRUCTURES_H