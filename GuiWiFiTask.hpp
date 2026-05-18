#ifndef GuiWiFiTask_h
#define GuiWiFiTask_h

#include <queue>
#include <vector>
#include <cstdint>

#include "common/ThreadTask.hpp"
#include "common/udp/udpclient.hpp"
#include "common/udp/udpserver.hpp"
#include "common/udp/udpSerialize.h"

#include "GuiWiFiFrames.hpp"
#include "VehicleData.hpp"

class GuiWiFiTask : public ThreadTask
{
public:
    explicit GuiWiFiTask(ConfigData *pConfigData/*, SystemData_t *pSystemData = nullptr*/);

protected:
    void task() override;

    uint8_t calculateChecksum(uint8_t *data, uint16_t len);
    int validatePacket(uint8_t *data, uint16_t len);
    int8_t processPacket(const GuiPacket_t &packet);
    int8_t sendPacketPushToQueue(uint8_t *data, uint16_t length);
    void sendAckPacket(StdAckRetCode_t ackCode, uint64_t time);
    void sendMotorPacket();
    void sendStatusPacket();

    int8_t validateMissionFile(uint8_t missionType, uint8_t filenameLength, const uint8_t* pFilename, const uint8_t *pMd5hash);
    int8_t updateOperationMode(uint8_t mode);
    int8_t updateManualControlData(int8_t finsAngleDeg, int8_t rudderAngleDeg, int8_t thrustPercent);
    int8_t updateSemiAutonomousData(float headingDeg, float speedMPS, float depthMtr, uint32_t durationSec);
    int8_t updateHomeLocationData(int32_t lattitude, int32_t longitude);

    void endianConverter(unsigned char *fromPointer, unsigned char *toPointer,
                         int noOfBytes)
    {
        int count = 0;
        int reverse = 0;

        reverse = noOfBytes - 1;

        for (count = 0; count < noOfBytes; count++)
        {
            *(toPointer + count) = *(fromPointer + reverse);

            reverse = reverse - 1;
        }
    }

private:
    ConfigData *pConfig;
    // SystemData_t *pSysData;
    UdpClient client;
    UdpServer server;

    // GuiPacket_t rxPacket;
    // GuiFrameAckMotor_t motorAckFrame;

    // Define the queue
    std::queue<std::vector<uint8_t>> txPktQueue;
};

#endif // GuiWiFiTask_h