#include "GuiWiFiTask.hpp"
#include "DataManager.hpp"
#include "VehicleData.hpp"

#include <iostream>
#include <cstring>
#include <chrono>

GuiWiFiTask::GuiWiFiTask(ConfigData *pConfigData, SystemData_t *pSystemData)
{
    pConfig = pConfigData;
    pSysData = pSystemData;
}

void GuiWiFiTask::task()
{
    if (!server.init(pConfig->guiWiFiRxPort))
    {
        std::cout << "[GUIWIFI] failed to initialized UDP Server on port "
                  << pConfig->guiWiFiRxPort
                  << "(receive from host)\n";
        return;
    }

    std::cout << "[GUIWIFI] UDP Server initialized on port "
              << pConfig->guiWiFiRxPort
              << "(receive from host)\n";

    if (!client.init(pConfig->guiWiFiIP,
                     pConfig->guiWiFiTxPort))
    {
        std::cout << "[GUIWIFI] Failed to initialize UDP Client to \""
                  << pConfig->guiWiFiIP << "\" : " << pConfig->guiWiFiTxPort << "\n";
        server.deinit();
        return;
    }

    std::cout << "[GUIWIFI] UDP Client initialized to send to \""
              << pConfig->guiWiFiIP << "\" : " << pConfig->guiWiFiTxPort << "\n";

    // int counter = 0;
    std::chrono::steady_clock::time_point last_status_time = std::chrono::steady_clock::now();

    while (running)
    {
        // uint8_t msg[128];
        GuiPacket_t rxPacket;

        /* Try to receive data from server socket */
        int bytesReceived = server.recv(reinterpret_cast<uint8_t *>(rxPacket.msg), sizeof(rxPacket.msg));

        if (bytesReceived > 0)
        {
            // Process received data
            // printf("[GUIWIFI] Received %d bytes\n", bytesReceived);

            /* Validate packet */
            if (validatePacket(reinterpret_cast<uint8_t *>(rxPacket.msg), bytesReceived) == 0)
            {
                // Process valid packet

                // GuiPacket_t rxPacket;

                // copy payload data excluding SOF, CR and SOF (3 bytes)
                // bcopy((uint8_t *)&msg[1], (uint8_t *)&rxPacket, (bytesReceived - 3));
                // memcpy(reinterpret_cast<uint8_t *>(&rxPacket), reinterpret_cast<uint8_t *>(&msg[1]), (bytesReceived - 3));
                // printf("[GUIWIFI] rxpacket: %d, %d, %d, %d, %d\n", rxPacket.msg[10], rxPacket.msg[11],
                //     rxPacket.data[0], rxPacket.data[1], rxPacket.data[2]);

                uint64_t rxTime; // = rxPacket.timestamp.val;
                endianConverter((uint8_t *)&rxPacket.header.timestamp, (uint8_t *)&rxTime, 8);

                uint16_t rxLength; // = rxPacket.length;
                endianConverter((uint8_t *)&rxPacket.header.length, (uint8_t *)&rxLength, 2);

                printf("[GUIWIFI] Valid packet: ID=%02d, Length=%d, time: %ld\n",
                       rxPacket.header.packetId, rxLength, rxTime);

                int8_t ret = processPacket(rxPacket);
                if ((StdAckRetCode_t)ret != STD_ACK_MSG_NO_RESPONSE)
                {
                    // need to send feedback
                    // send ACK packet with ret as feedback value

                    sendAckPacket((StdAckRetCode_t)ret);

                    // WiFiWriteFramePushToQueue(packet_id_ack_std,
                    //                           rxPacket.time.bytes,
                    //                           (uint8_t *)&rxPacket.data, 1);

                    // DBG_MSG("[WiFi] sending ACK #%d \n", ret);
                }
            }
            else
            {
                // mgmt->rxErrors++;
                printf("[GUIWIFI] Invalid packet rejected\n");
            }
        }

        auto now = std::chrono::steady_clock::now();
        auto elapsed_status = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_status_time);

        // Publish navigation status at 0.2 Hz (send to host)
        if (elapsed_status.count() >= 5000)
        {
            last_status_time = now;

            // snprintf(msg,
            //          sizeof(msg),
            //          "Hello GuiWiFiTask %d",
            //          counter++);

            // client.send(
            //     reinterpret_cast<uint8_t *>(msg),
            //     strlen(msg));

            // std::cout
            //     << "[GuiWiFiTask] Sent: "
            //     << msg
            //     << "\n";

            sendStatusPacket();
        }

        usleep(50000); // Sleep for 50ms to reduce CPU usage
    }
}

int8_t GuiWiFiTask::processPacket(const GuiPacket_t &packet)
{
    // Note: TIME_SYNC_REQ, SW_VERSION_REQ and MOTOR_STATE_REQ does not required ACK_STD upon request
    int8_t ret = STD_ACK_MSG_NOT_RECOGNIZED;

    // Process the packet based on its ID and payload
    switch (packet.header.packetId)
    {
    case packet_id_select_mission:
        // Process VU command packet
        std::cout << "[GUIWIFI] Processing packet_id_select_mission\n";
        break;
    case packet_id_start_mission:
        // Process Mission command packet
        std::cout << "[GUIWIFI] Processing packet_id_start_mission\n";
        break;
    case packet_id_abort_mission:
        std::cout << "[GUIWIFI] Processing packet_id_abort_mission\n";
        break;
    case packet_id_select_mode:
        std::cout << "[GUIWIFI] Processing packet_id_select_mode\n";
        updateOperationMode(packet.data[0]);
        // printf("[GUIWIFI] Updated operation mode to %d, %d, %d, %d, %d\n", packet.data[0], packet.data[1], packet.data[2], packet.data[3], packet.data[4]);
        ret = STD_ACK_MSG_EXECUTED; // acknowledge mode change command
        break;
    case packet_id_channel_selection:
        std::cout << "[GUIWIFI] Processing packet_id_channel_selection\n";
        break;
    case packet_id_time_sync:
        std::cout << "[GUIWIFI] Processing packet_id_time_sync\n";
        break;
    case packet_id_autotest:
        std::cout << "[GUIWIFI] Processing packet_id_autotest\n";
        break;
    case packet_id_motor_state:
        // std::cout << "[GUIWIFI] Processing packet_id_motor_state\n";
        sendMotorPacket();
        ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required
        break;
    case packet_id_action_gotowp:
        std::cout << "[GUIWIFI] Processing packet_id_action_gotowp\n";
        break;
    case packet_id_action_hovering:
        std::cout << "[GUIWIFI] Processing packet_id_action_hovering\n";
        break;
    case packet_id_action_floating:
        std::cout << "[GUIWIFI] Processing packet_id_action_floating\n";
        break;
    case packet_id_action_surfacing:
        std::cout << "[GUIWIFI] Processing packet_id_action_surfacing\n";
        break;
    case packet_id_battery_level:
        std::cout << "[GUIWIFI] Processing packet_id_battery_level\n";
        break;
    case packet_id_bouyancy:
        std::cout << "[GUIWIFI] Processing packet_id_bouyancy\n";
        break;
    case packet_id_rov_ctrl_pck:
        std::cout << "[GUIWIFI] Processing packet_id_rov_ctrl_pck\n";
        break;
    case packet_id_cmd_nose:
        std::cout << "[GUIWIFI] Processing packet_id_cmd_nose\n";
        break;
    case packet_id_gui_keep_alive:
        // std::cout << "[GUIWIFI] Processing packet_id_gui_keep_alive\n";
        ret = STD_ACK_MSG_NO_RESPONSE; // no ACK_STD feedback required
        break;
    case packet_id_shutdown:
        std::cout << "[GUIWIFI] Processing packet_id_shutdown\n";
        break;
    case packet_id_stop_log:
        std::cout << "[GUIWIFI] Processing packet_id_stop_log\n";
        break;
    case packet_id_sw_version:
        std::cout << "[GUIWIFI] Processing packet_id_sw_version\n";
        break;
    case packet_id_cmd_dw:
        std::cout << "[GUIWIFI] Processing packet_id_cmd_dw\n";
        break;
    case packet_id_cmd_hsdt: // 199
        std::cout << "[GUIWIFI] Processing packet_id_cmd_hsdt\n";
        break;
    case packet_id_cmd_gcs_location:
        std::cout << "[GUIWIFI] Processing packet_id_cmd_gcs_location\n";
        break;

    default:
        std::cout << "[GUIWIFI] Unknown Packet ID: " << static_cast<int>(packet.header.packetId) << "\n";
        ret = STD_ACK_MSG_NOT_RECOGNIZED; // feedback for unknown packet ID
        // return -1; // Unknown packet ID
        break;
    }

    return ret; // Success
}

void GuiWiFiTask::updateOperationMode(uint8_t mode)
{
    // This function can be used to send the current operation mode of the vehicle to the host GUI
    // It can be called whenever there is a change in the operation mode or periodically as needed

    // Example implementation:
    // Create a packet with the current operation mode and send it to the host

    // uint8_t buffer[128];
    // GuiPacket_t packet;
    // packet.packetId = packet_id_operation_mode; // Define this packet ID in GuiWiFiFrames.hpp
    // packet.timestamp.val = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // packet.length = sizeof(OperationModes_t);
    // OperationModes_t currentMode = pSysData->operationMode; // Assuming operationMode is stored in SystemData_t
    // memcpy(packet.data, &currentMode, sizeof(OperationModes_t));

    // client.send(reinterpret_cast<uint8_t*>(&packet), sizeof(GuiPacket_t));

    auto &db = DataManager::instance();
    auto cmnd = db.commands.get();

    cmnd.mode = (OperationModes_t)mode;
    db.commands.set(cmnd);

    printf("[GUIWIFI] changed mode from %d to %d\n", mode, (uint8_t)cmnd.mode);
}

void GuiWiFiTask::sendMotorPacket()
{
    GuiFrameAckMotor_t motorAckFrame;
    uint16_t dummy16u = 0;
    // uint32_t dummy32u = 0;
    int16_t dummy16 = 0;
    // int32_t dummy32 = 0;

    memset(&motorAckFrame, 0, sizeof(GuiFrameAckMotor_t));

    /***Thruster***/
    dummy16 = (int16_t)pSysData->thruster.rpm;
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&motorAckFrame.thrusterBSx.rpm, 2);

    motorAckFrame.thrusterBSx.current = (int8_t)(pSysData->thruster.current * 5);
    motorAckFrame.thrusterBSx.voltage = (int8_t)pSysData->thruster.voltage * 2;
    motorAckFrame.thrusterBSx.temperature = (int8_t)pSysData->thruster.temperature;
    motorAckFrame.thrusterBSx.command = (int8_t)pSysData->thruster.thrustCommanded;

    /***Left/Port FIN***/
    dummy16 = (int16_t)pSysData->port.position;
    endianConverter((uint8_t *)&dummy16,
                    (uint8_t *)&motorAckFrame.thrusterBDx.rpm, 2);

    motorAckFrame.thrusterBDx.current = (int8_t)(pSysData->port.current * 5);
    motorAckFrame.thrusterBDx.voltage = (int8_t)(pSysData->port.voltage);
    motorAckFrame.thrusterBDx.temperature = (int8_t)pSysData->port.temperature;
    motorAckFrame.thrusterBDx.command = (int8_t)pSysData->port.angleCommanded;

    /***Right/Starboard FIN***/
    dummy16 = (int16_t)pSysData->starboard.position;
    endianConverter((uint8_t *)&dummy16,
                    (uint8_t *)&motorAckFrame.thrusterBSide.rpm, 2);

    motorAckFrame.thrusterBSide.current = (int8_t)(pSysData->starboard.current * 5);
    motorAckFrame.thrusterBSide.voltage = (int8_t)(pSysData->starboard.voltage);
    motorAckFrame.thrusterBSide.temperature = (int8_t)pSysData->starboard.temperature;
    motorAckFrame.thrusterBSide.command = (int8_t)pSysData->starboard.angleCommanded;

    /***Top Rudder FIN***/
    dummy16 = (int16_t)pSysData->topRudder.position;
    endianConverter((uint8_t *)&dummy16,
                    (uint8_t *)&motorAckFrame.thrusterFSide.rpm, 2);

    motorAckFrame.thrusterFSide.current = (int8_t)(pSysData->topRudder.current * 5);
    motorAckFrame.thrusterFSide.voltage = (int8_t)(pSysData->topRudder.voltage);
    motorAckFrame.thrusterFSide.temperature = (int8_t)pSysData->topRudder.temperature;
    motorAckFrame.thrusterFSide.command = (int8_t)pSysData->topRudder.angleCommanded;

    /***Bottom Rudder FIN***/
    dummy16 = (int16_t)pSysData->bottomRudder.position;
    endianConverter((uint8_t *)&dummy16,
                    (uint8_t *)&motorAckFrame.thrusterBTop.rpm, 2);

    motorAckFrame.thrusterBTop.current = (int8_t)(pSysData->bottomRudder.current * 5);
    motorAckFrame.thrusterBTop.voltage = (int8_t)(pSysData->bottomRudder.voltage);
    motorAckFrame.thrusterBTop.temperature = (int8_t)pSysData->bottomRudder.temperature;
    motorAckFrame.thrusterBTop.command = (int8_t)pSysData->bottomRudder.angleCommanded;

    /***current thrust and angles ***/
    dummy16 = (int16_t)pSysData->thruster.thrust;
    endianConverter((uint8_t *)&dummy16,
                    (uint8_t *)&motorAckFrame.thrusterFTop.rpm, 2);

    motorAckFrame.thrusterFTop.current = (int8_t)pSysData->port.angle;
    motorAckFrame.thrusterFTop.voltage = (int8_t)pSysData->starboard.angle;
    motorAckFrame.thrusterFTop.temperature = (int8_t)pSysData->topRudder.angle;
    motorAckFrame.thrusterFTop.command = (int8_t)pSysData->bottomRudder.angle;

    //--------

    motorAckFrame.header.startMsg = GUI_START_MSG_VALUE; // '$'
    motorAckFrame.header.packetId = packet_id_ack_motor; // Example packet ID for motor state ACK

    uint64_t dTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    endianConverter((uint8_t *)&dTime, (uint8_t *)&motorAckFrame.header.timestamp, 8);

    dummy16u = sizeof(GuiFrameAckMotor_t) /*sizeof(GUI_MSG_HEADER) - sizeof(GUI_MSG_FOOTER)*/;
    endianConverter((uint8_t *)&dummy16u, (uint8_t *)&motorAckFrame.header.length, 2);

    // calculate checksum for header and payload (excluding start and end bytes)
    motorAckFrame.footer.checksum = calculateChecksum((uint8_t *)&motorAckFrame.header.packetId, sizeof(motorAckFrame) - sizeof(GUI_MSG_FOOTER));

    motorAckFrame.footer.endMsg = GUI_END_MSG_VALUE; // '\n'

    int32_t bytesSent = client.send((uint8_t *)&motorAckFrame.header.startMsg, sizeof(GuiFrameAckMotor_t));

    // endianConverter((uint8_t *)&motorAckFrame.header.timestamp, (uint8_t *)&dTime, 8);
    printf("[GUIWIFI] Sent motor packet: ID=%02d, Length=%ld, time: %ld, bytesSent: %d\n",
           motorAckFrame.header.packetId, sizeof(GuiFrameAckMotor_t), dTime, bytesSent);
}

void GuiWiFiTask::sendStatusPacket()
{
    auto &db = DataManager::instance();
    auto nav = db.navigation.get();
    auto att = db.attitude.get();
    auto sys = db.status.get();

    // printf("[GUIWIFI] status : Lat=%.5f, Lon=%.5f, Depth=%.2f, Heading=%.2f, Pitch=%.2f, Roll=%.2f\n",
    //    nav.latitude, nav.longitude, nav.depth, att.yaw, att.pitch, att.roll);

    // printf("[GUIWIFI] status mode: %d\n", (uint8_t)sys.mode);

    GuiFrameVuStatus_t GuiVuStaus;
    uint16_t dummy16u = 0;
    uint32_t dummy32u = 0;
    int16_t dummy16 = 0;
    int32_t dummy32 = 0;

    memset(&GuiVuStaus, 0, sizeof(GuiFrameVuStatus_t));

    //-- set header
    GuiVuStaus.header.startMsg = GUI_START_MSG_VALUE;     // '$'
    GuiVuStaus.header.packetId = packet_id_vu_status_pck; // Example packet ID for

    uint64_t dTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << dTime << std::endl;
    endianConverter((uint8_t *)&dTime, (uint8_t *)&GuiVuStaus.header.timestamp, 8);

    dummy16u = sizeof(GuiFrameVuStatus_t) /*sizeof(GUI_MSG_HEADER) - sizeof(GUI_MSG_FOOTER)*/;
    endianConverter((uint8_t *)&dummy16u, (uint8_t *)&GuiVuStaus.header.length, 2);

    //---------------------------------------------------
    GuiVuStaus.MissionOperationId = 0;
    GuiVuStaus.VuState = (uint8_t)sys.mode;

    GuiVuStaus.VuStatusB0.LB_Comm = 1;
    GuiVuStaus.VuStatusB0.Carris_Comm = 1;
    GuiVuStaus.VuStatusB0.NCU_Comm = 1;
    GuiVuStaus.VuStatusB0.LogFileActive = 1;

    GuiVuStaus.NcuStatusB4.ObstacleAvoidStatusCode = 0;
    GuiVuStaus.NcuStatusB5GuidanceStatus.DepthMeasuresComparison = 0;

    if (GuiVuStaus.VuState == (uint8_t)OperationModes_t::OP_MODE_MANUAL) // Example: set guidance mode code based on current operation mode of the vehicle
    {
        GuiVuStaus.NcuStatusB5GuidanceStatus.GuidanceModeCode = 1;
    }
    else
    {
        GuiVuStaus.NcuStatusB5GuidanceStatus.GuidanceModeCode = 0;
    }
    GuiVuStaus.NcuStatusB67.Val = 0xFFFF;

    //=====================================================================================
    GuiVuStaus.LbStatusB0.MonSafetySwitch = 1; // not present, active low
    GuiVuStaus.LbStatusB0.AisEmSwitch = 0;     // active high
    /*'Battery Charger Present' label Highlighted on GCS only when BatteryCharger set to 1*/
    GuiVuStaus.LbStatusB0.BatteryCharger = 0;
    /*'Emergency Active' label Highlighted on GCS only when MonKWD is set to 1*/
    GuiVuStaus.LbStatusB0.MonKWD = 0; // set when dropweight is triggered
    GuiVuStaus.LbStatusB0.Reserved_1 = 0;
    /*LbStatusB0 bit 5,6 and 7 are active low*/
    GuiVuStaus.LbStatusB0.VuSignal = 0;     // reset if able to communicate with MCU board
    GuiVuStaus.LbStatusB0.NcuSignal = 0;    // reset if able to communicate with MCU board
    GuiVuStaus.LbStatusB0.CarrisSignal = 0; // reset if able to communicate with MCU board

    GuiVuStaus.LbStatusB1.DnSwitch_1 = 0;
    GuiVuStaus.LbStatusB1.DnSwitch_2 = 0;
    GuiVuStaus.LbStatusB1.BuoyancyPosition_Ok = 0;
    GuiVuStaus.LbStatusB1.Buoyancy_Fault = 0;
    GuiVuStaus.LbStatusB1.DW_Switch2 = 0;
    GuiVuStaus.LbStatusB1.FloodSwitch = 0;
    GuiVuStaus.LbStatusB1.FailNavigationSwitch = 0;
    GuiVuStaus.LbStatusB1.FailScientificSwitch = 0;
    //=====================================================================================
    GuiVuStaus.CarrisStatusB0.MBES_Sensor = 0;
    GuiVuStaus.CarrisStatusB0.SSS_Sensor = 0;
    GuiVuStaus.CarrisStatusB1.SBP_Sensor = 0;
    GuiVuStaus.CarrisStatusB1.Camera_Sensor = 0;
    GuiVuStaus.CarrisStatusB1.CTD_Sensor = 0;

    GuiVuStaus.NcuStatusB0.CommunicationWarning = 1;
    GuiVuStaus.NcuStatusB0.GuidanceWarning = 1;

    // update thruster status
    GuiVuStaus.NcuStatusB0.ThrusterWarning = 0;
    GuiVuStaus.NcuStatusB1ThrusterWarn.Val = 0;

    // Update navigation status
    dummy32 = (int32_t)((nav.latitude) * 10000000L);
    endianConverter((uint8_t *)&dummy32, (uint8_t *)&GuiVuStaus.Lattitude, 4);

    dummy32 = (int32_t)((nav.longitude) * 10000000L);
    endianConverter((uint8_t *)&dummy32, (uint8_t *)&GuiVuStaus.Longitude, 4);

    dummy16 = (int16_t)(nav.speed * 100);
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.Speed, 2);

    dummy16 = (int16_t)(nav.velocityNorth * 100);
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.SpeedBody_X, 2);

    dummy16 = (int16_t)(nav.velocityEast * 100);
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.SpeedBody_Y, 2);

    dummy16 = (int16_t)(nav.velocityDown * 100);
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.SpeedBody_Z, 2);

    // update INS status
    GuiVuStaus.NcuStatusB2CommWarn.INS_Comm = 0;
    GuiVuStaus.NcuStatusB2CommWarn.LogicBoardComm = 1;
    GuiVuStaus.NcuStatusB2CommWarn.VitalUnitComm = 1;

    GuiVuStaus.NcuStatusB67.SolutionValidity = 0;
    GuiVuStaus.NcuStatusB67.AccelerometerFail = 0;
    GuiVuStaus.NcuStatusB67.GyroFail = 0;
    GuiVuStaus.NcuStatusB67.AccelerometerOutrancge = 0;
    GuiVuStaus.NcuStatusB67.GyrosOutrange = 0;
    GuiVuStaus.NcuStatusB67.GPS_Validity = 0;

    dummy16 = (int16_t)((att.roll * 100U));
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.RollAngle, 2);

    dummy16 = (int16_t)((att.pitch * 100U));
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.PitchAngle, 2);

    dummy16 = (uint16_t)((att.yaw * 100)); // * 10000U
    endianConverter((uint8_t *)&dummy16, (uint8_t *)&GuiVuStaus.YawAngle, 2);

    dummy32 = (int32_t)(nav.depth * 100);
    endianConverter((uint8_t *)&dummy32, (uint8_t *)&GuiVuStaus.Depth, 4);

    // alitmeter status
    GuiVuStaus.NcuStatusB2CommWarn.EchoBottomComm = 1;
    GuiVuStaus.NcuStatusB2CommWarn.EchoFrontComm = 1;
    GuiVuStaus.NcuStatusB2CommWarn.EchoSlopedComm = 1;

    // update battery status
    GuiVuStaus.VuStatusB0.BatteryComm = 1; // battery communication OK

    dummy16u = (uint16_t)(pSysData->bms.voltage * 100);
    endianConverter((uint8_t *)&dummy16u, (uint8_t *)&GuiVuStaus.BatteryVoltage, 2);

    dummy16u = (uint16_t)(pSysData->bms.voltage);
    endianConverter((uint8_t *)&dummy16u, (uint8_t *)&GuiVuStaus.BatteryVoltage_LB, 2);

    dummy16u = (uint16_t)(pSysData->bms.current * 100);
    endianConverter((uint8_t *)&dummy16u, (uint8_t *)&GuiVuStaus.BatteryCurrent, 2);

    dummy32u = (uint32_t)(pSysData->bms.capacityRemain * 100);
    endianConverter((uint8_t *)&dummy32u, (uint8_t *)&GuiVuStaus.BatteryCapacityRemain, 4);

    GuiVuStaus.BatteryCharge = pSysData->bms.chargePercent;

    //-- set footer

    // calculate checksum for header and payload (excluding start and end bytes)
    GuiVuStaus.footer.checksum = calculateChecksum((uint8_t *)&GuiVuStaus.header.packetId, sizeof(GuiVuStaus) - 3 /*sizeof(GUI_MSG_FOOTER)*/);

    GuiVuStaus.footer.endMsg = GUI_END_MSG_VALUE; // '\n'

    //-- send the status packet to host
    int32_t bytesSent = client.send((uint8_t *)&GuiVuStaus.header.startMsg, sizeof(GuiFrameVuStatus_t));

    // endianConverter((uint8_t *)&GuiVuStaus.header.timestamp, (uint8_t *)&dTime, 8);
    printf("[GUIWIFI] Sent status packet: ID=%02d, Length=%ld, time: %ld, bytesSent: %d\n",
           GuiVuStaus.header.packetId, sizeof(GuiFrameVuStatus_t), dTime, bytesSent);
}

/**
 * Calculate XOR checksum for packet validation
 */
uint8_t GuiWiFiTask::calculateChecksum(uint8_t *data, uint16_t len)
{
    uint16_t i;
    uint8_t ck = 0;

    if (!data || len == 0)
    {
        return 0;
    }

    for (i = 0; i < len; i++)
    {
        ck ^= data[i];
    }

    return ck;
}

/**
 * Validate GuiWiFiTask packet structure
 * Expected format: [START_BYTE | PACKET_ID | LENGTH (2 bytes) | DATA | CHECKSUM | END_BYTE]
 */
int GuiWiFiTask::validatePacket(uint8_t *data, uint16_t len)
{
    if (!data || len < 6)
    {
        return -1; // Invalid packet size
    }

    uint8_t startByte = data[0];
    uint8_t endByte = data[len - 1];

    // Check frame delimiters
    if (startByte != 0x24)
    { // '$'
        printf("[GuiWiFiTask] Invalid start byte: 0x%02X\n", startByte);
        return -1;
    }

    if (endByte != 0x0A)
    { // '\n'
        printf("[GuiWiFiTask] Invalid end byte: 0x%02X\n", endByte);
        return -1;
    }

    // Checksum is at len-2 position (before end byte)
    uint8_t receivedChecksum = data[len - 2];
    uint8_t calculatedChecksum = calculateChecksum(data + 1, len - 3);

    if (receivedChecksum != calculatedChecksum)
    {
        printf("[GuiWiFiTask] Checksum mismatch: received=0x%02X, calculated=0x%02X\n",
               receivedChecksum, calculatedChecksum);
        return -1;
    }

    return 0; // Valid packet
}

void GuiWiFiTask::sendAckPacket(StdAckRetCode_t ackCode)
{
    GuiFrameAckStandard_t stdAckFrame;

    memset(&stdAckFrame, 0, sizeof(GuiFrameAckStandard_t));

    stdAckFrame.header.startMsg = GUI_START_MSG_VALUE; // '$'

    stdAckFrame.header.packetId = packet_id_ack_std;

    uint64_t dTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    // std::cout << dTime << std::endl;
    endianConverter((uint8_t *)&dTime, (uint8_t *)&stdAckFrame.header.timestamp, 8);

    uint16_t length = sizeof(GuiFrameAckStandard_t) /*sizeof(GUI_MSG_HEADER) - sizeof(GUI_MSG_FOOTER)*/;
    endianConverter((uint8_t *)&length, (uint8_t *)&stdAckFrame.header.length, 2);

    stdAckFrame.testResult = (int8_t)ackCode;

    // calculate checksum for header and payload (excluding start and end bytes)
    stdAckFrame.footer.checksum = calculateChecksum((uint8_t *)&stdAckFrame.header.packetId, sizeof(GuiFrameAckStandard_t) - sizeof(GUI_MSG_FOOTER));

    stdAckFrame.footer.endMsg = GUI_END_MSG_VALUE; // '\n'

    int32_t bytesSent = client.send((uint8_t *)&stdAckFrame.header.startMsg, sizeof(GuiFrameAckStandard_t));

    printf("[GUIWIFI] Sent ACK: %d, packet: ID=%02d, Length=%ld, time: %ld, bytesSent: %d\n",
           stdAckFrame.testResult, stdAckFrame.header.packetId, length, dTime, bytesSent);
}