#ifndef GuiWiFiTask_h
#define GuiWiFiTask_h

#include "common/ThreadTask.hpp"
#include "common/udp/udpclient.hpp"
#include "common/udp/udpserver.hpp"
#include "common/udp/udpSerialize.h"

#include "GuiWiFiFrames.hpp"
#include "VehicleData.hpp"

class GuiWiFiTask : public ThreadTask
{
public:
    explicit GuiWiFiTask(ConfigData *pConfigData, SystemData_t *pSystemData = nullptr);

protected:
    void task() override;

    uint8_t calculateChecksum(uint8_t *data, uint16_t len);
    int validatePacket(uint8_t *data, uint16_t len);
    void sendAckPacket(StdAckRetCode_t ackCode);
    void sendMotorPacket();
    void sendStatusPacket();
    int8_t processPacket(const GuiPacket_t &packet);
    void updateOperationMode(uint8_t mode);

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
    SystemData_t *pSysData;
    UdpClient client;
    UdpServer server;

    // GuiPacket_t rxPacket;
    // GuiFrameAckMotor_t motorAckFrame;
};

#endif // GuiWiFiTask_h