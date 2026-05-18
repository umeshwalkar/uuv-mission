#ifndef DATAMANAGER_HPP
#define DATAMANAGER_HPP

#include "common/DataNode.hpp"
#include "VehicleData.hpp"

class DataManager
{
public:
    static DataManager &instance()
    {
        static DataManager inst;

        return inst;
    }

    /**
     * Convert string to operation mode
     */
    OperationMode string_to_mode(const std::string &mode_str)
    {
        if (mode_str == "standby")
            return OperationMode::OP_MODE_STANDBY;
        if (mode_str == "autonomous")
            return OperationMode::OP_MODE_AUTONOMOUS;
        if (mode_str == "semi_autonomous")
            return OperationMode::OP_MODE_SEMI_AUTONOMOUS;
        if (mode_str == "semi-autonomous")
            return OperationMode::OP_MODE_SEMI_AUTONOMOUS;
        if (mode_str == "manual")
            return OperationMode::OP_MODE_MANUAL;
        if (mode_str == "emergency")
            return OperationMode::OP_MODE_EMERGENCY;
        return OperationMode::OP_MODE_UNKNOWN;
    }

    /**
     * Convert operation mode to string
     */
    std::string mode_to_string(OperationMode mode)
    {
        switch (mode)
        {
        case OperationMode::OP_MODE_STANDBY:
            return "standby";
        case OperationMode::OP_MODE_AUTONOMOUS:
            return "autonomous";
        case OperationMode::OP_MODE_SEMI_AUTONOMOUS:
            return "semi-autonomous";
        case OperationMode::OP_MODE_MANUAL:
            return "manual";
        case OperationMode::OP_MODE_EMERGENCY:
            return "emergency";
        default:
            return "unknown";
        }
    }

public:
    DataNode<AttitudeData> attitude;

    DataNode<NavigationData> navigation;

    DataNode<ThrusterData> thrusters;

    DataNode<FinsData> fins;

    DataNode<BatteryData>bms;

    DataNode<CommandData> commands;

    DataNode<SystemStatus> status;

    DataNode<AutoMissionData> autoMissionData;
 
private:
    DataManager() = default;
};

#endif