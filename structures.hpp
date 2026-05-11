#ifndef _STRUCTURES_H
#define _STRUCTURES_H

/**
 * Mission Command Structure
 */
struct MissionCommand {
    std::string command; // e.g., "GOTO", "HOLD", "SURFACE"
    double latitude;    // Target latitude (for GOTO)
    double longitude;   // Target longitude (for GOTO)
    double depth;       // Target depth (for GOTO)
    double heading;     // Target heading (for GOTO)
    int duration;       // Duration in seconds (for HOLD)
    
    MissionCommand() : command(""), latitude(0), longitude(0), depth(0), heading(0), duration(0) {}
};

struct TelemetryData {
    std::string operation_mode;
    double latitude;
    double longitude;
    double depth;
    double speed;
    double heading;
    double pitch;
    double roll;
    
    TelemetryData() : operation_mode(""), latitude(0), longitude(0), depth(0), heading(0), speed(0), pitch(0), roll(0) {}
};

struct finFeedbackData
{
    double angle_deg;          // range -30 to +30 degree
    // double voltage;
    // double current;
    // double prosition;
    // unsigned short status;
};

struct thrusterFedbackData
{
    double thrust_percent;      // Range -100 to +100
    int rpm;                 // Range -1000 to +1000
};

struct NavControlData{
    
    thrusterFedbackData thruster; 
    finFeedbackData port;
    finFeedbackData starboard;
    finFeedbackData topRudder;
    finFeedbackData bottomRudder;
};

#endif // _STRUCTURES_H