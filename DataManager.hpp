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

public:
    DataNode<AttitudeData> attitude;

    DataNode<NavigationData> navigation;

    DataNode<ThrusterData> thrusters;

    DataNode<FinsData> fins;

    DataNode<CommandData> commands;

    DataNode<SystemStatus> status;

private:
    DataManager() = default;
};

#endif