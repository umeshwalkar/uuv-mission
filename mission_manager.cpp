#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <mosquitto.h>
#include <json/json.h>

#include "DataManager.hpp"
#include "VehicleData.hpp"

// #include "common/udp/udpclient.hpp"
// #include "common/udp/udpserver.hpp"
// #include "common/udp/udpSerialize.h"

#include "GuiWiFiTask.hpp"

#define UDP_GCS_RX_PORT 5500
#define UDP_GCS_TX_PORT 5510
#define UDP_GCS_IP "host.docker.internal"

// On Docker Desktop (Windows/Mac), host.docker.internal resolves to the host machine
// On Linux with --network host, this would need to be 127.0.0.1
#define MAX_MESSAGE_DIMENSION 1024

// Configuration
const char *MQTT_BROKER = "host.docker.internal"; //"127.0.0.1";  //"mqtt-broker";
// const char* MQTT_BROKER =  "127.0.0.1";  //"mqtt-broker";

int MQTT_PORT = 1883;
const char *CLIENT_ID = "mission-manager-dev";

// MQTT Topics
// const char* IMU_TOPIC = "uuv/sensors/imu";
const char *IMU_HNAV_TOPIC = "uuv/sensors/imu/hnav";
const char *MISSION_TOPIC = "uuv/mission";
const char *NAV_CONTROL_TOPIC = "uuv/navigation/control";
const char *NAV_STATUS_TOPIC = "uuv/navigation/status";
const char *BMS_STATUS_TOPIC = "uuv/sensors/bms";

// Global state
volatile sig_atomic_t running = 1;
struct mosquitto *mqtt_client = NULL;
int mqtt_connected = 0;
// ModeManager* mode_manager = NULL;
// UdpServerStruct udpServer;
// UdpClientStruct udpClient;
// SystemData_t systemData; // this will hold the latest telemetry and status data for use in the GUI and other tasks

// Counters
int nav_control_count = 0;
int nav_status_count = 0;
int imu_count = 0;

// Scaling factors
#define SIX_DECIMAL_SCALE(x) (round((x) * 1000000.0) / 1000000.0) // Scale values to 6 decimal places
#define THREE_DECIMAL_SCALE(x) (round((x) * 1000.0) / 1000.0)     // Scale values to 3 decimal places
#define TWO_DECIMAL_SCALE(x) (round((x) * 100.0) / 100.0)         // Scale values to 2 decimal places

/**
 * Signal handler for graceful shutdown
 */
void signal_handler(int sig)
{
    printf("[NavManager] Received signal %d, shutting down...\n", sig);
    running = 0;
}

/**
 * MQTT connection callback
 */
void mqtt_on_connect(struct mosquitto *mosq, void *userdata, int result)
{
    if (result == 0)
    {
        printf("[MQTT] Connected successfully (result: %d)\n", result);
        mqtt_connected = 1;

        // Subscribe to topics

        mosquitto_subscribe(mosq, NULL, NAV_CONTROL_TOPIC, 1);
        mosquitto_subscribe(mosq, NULL, NAV_STATUS_TOPIC, 1);
        mosquitto_subscribe(mosq, NULL, IMU_HNAV_TOPIC, 1);
        mosquitto_subscribe(mosq, NULL, BMS_STATUS_TOPIC, 1);

        printf("[MQTT] Subscribed to topics:\n");
        printf("  - %s\n", NAV_CONTROL_TOPIC);
        printf("  - %s\n", NAV_STATUS_TOPIC);
        printf("  - %s\n", IMU_HNAV_TOPIC);
        printf("  - %s\n", BMS_STATUS_TOPIC);
    }
    else
    {
        printf("[MQTT] Connection failed (result: %d)\n", result);
        mqtt_connected = 0;
    }
}

/**
 * MQTT disconnect callback
 */
void mqtt_on_disconnect(struct mosquitto *mosq, void *userdata, int result)
{
    printf("[MQTT] Disconnected (result: %d)\n", result);
    mqtt_connected = 0;
}

/**
 * MQTT message received callback
 */
void mqtt_on_message(struct mosquitto *mosq, void *userdata,
                     const struct mosquitto_message *msg)
{
    if (msg->payloadlen == 0)
        return;

    char payload[4096];
    strncpy(payload, (char *)msg->payload, std::min(msg->payloadlen, 4095));
    payload[msg->payloadlen] = '\0';

    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(payload);

    if (!Json::parseFromStream(builder, s, &root, &errs))
    {
        printf("[MQTT] Failed to parse JSON from topic %s: %s\n", msg->topic, errs.c_str());
        return;
    }

    // Handle IMU data (HNAV)
    if (strcmp(msg->topic, NAV_CONTROL_TOPIC) == 0)
    {
        // use this data only when vehicle not in MANUAL mode

        auto &db = DataManager::instance();
        // auto nav = db.navigation.get();
        // auto thr = db.thrusters.get();
        auto sys = db.status.get();

        if (sys.mode != OperationMode::OP_MODE_MANUAL)
        {
            auto cmnd = db.commands.get();

            nav_control_count++;

            // mode_manager->update_navigation_state(root);

            // Parse autonomous parameters
            if (root.isMember("thrust"))
                cmnd.commandedThrust[THRUSTERS::SURGE_1] = root["thrust"].asInt();
            // if (root.isMember("thruster_rpm"))
            //     thr.rpm[THRUSTERS::SURGE_1] = root["thruster_rpm"].asInt();

            if (root.isMember("bottom_rudder"))
                cmnd.commandedFinAngle[FINS::BOTTOM] = root["bottom_rudder"].asDouble();
            if (root.isMember("top_rudder"))
                cmnd.commandedFinAngle[FINS::TOP] = root["top_rudder"].asDouble();
            if (root.isMember("starboard_rudder"))
                cmnd.commandedFinAngle[FINS::RIGHT] = root["starboard_rudder"].asDouble();
            if (root.isMember("port_rudder"))
                cmnd.commandedFinAngle[FINS::LEFT] = root["port_rudder"].asDouble();

            // commit to data dictionary
            db.commands.set(cmnd);

            if (nav_control_count % 300 == 0)
            {
                // const auto& nav = mode_manager->get_navigation_state();
                printf("[MisManager] NAV Update #%d: Thrust=%d %%, Top=%.1f, Bottom=%.1f, port=%.1f, starboard=%.1f\n",
                       nav_control_count, cmnd.commandedThrust[THRUSTERS::SURGE_1],
                       cmnd.commandedFinAngle[FINS::TOP], cmnd.commandedFinAngle[FINS::BOTTOM],
                       cmnd.commandedFinAngle[FINS::LEFT], cmnd.commandedFinAngle[FINS::RIGHT]);
            }
        }
    }

    // Handle IMU data (HNAV)
    else if (strcmp(msg->topic, IMU_HNAV_TOPIC) == 0)
    {
        auto &db = DataManager::instance();
        auto nav = db.navigation.get();
        auto att = db.attitude.get();

        imu_count++;

        // TelemetryData Telemetry;

        if (root.isMember("latitude"))
            nav.latitude = root["latitude"].asDouble();
        if (root.isMember("longitude"))
            nav.longitude = root["longitude"].asDouble();
        if (root.isMember("depth"))
            nav.depth = root["depth"].asDouble();

        if (root.isMember("speed"))
            nav.speed = root["speed"].asDouble();
        if (root.isMember("velocity_x"))
            nav.velocityNorth = root["velocity_x"].asDouble();
        if (root.isMember("velocity_y"))
            nav.velocityEast = root["velocity_y"].asDouble();
        if (root.isMember("velocity_z"))
            nav.velocityDown = root["velocity_z"].asDouble();

        if (root.isMember("heading"))
            att.yaw = root["heading"].asDouble();
        if (root.isMember("pitch"))
            att.pitch = root["pitch"].asDouble();
        if (root.isMember("roll"))
            att.roll = root["roll"].asDouble();

        db.attitude.set(att);
        db.navigation.set(nav);

        if (imu_count % 300 == 0)
        {
            printf("[MisManager] IMU Update #%d: Lat=%.6f, Lon=%.6f, Depth=%.2f, Heading=%.1f\n",
                   imu_count, nav.latitude, nav.longitude, nav.depth, att.yaw);
        }
    }

    // handles navigation manager status packet
    else if (strcmp(msg->topic, NAV_STATUS_TOPIC) == 0)
    {
        // handles navogation manager status message
        auto &db = DataManager::instance();
        auto nav = db.navigation.get();
        auto sys = db.status.get();

        auto navUpdate = false;
        // auto sysUpdate = false;

        // navigation
        if (root.isMember("mission_command"))
        {
            Json::Value &mission_cmd = root["mission_command"];
            if (mission_cmd.isMember("distance_target"))
            {
                nav.distanceToTarget = mission_cmd["distance_target"].asUInt();
                navUpdate = true;
                // db.navigation.set(nav);
            }
            if (mission_cmd.isMember("target_acheived"))
            {
                nav.target_acheived = mission_cmd["target_acheived"].asBool();
                navUpdate = true;
                // db.navigation.set(nav);
            }
        }

        // use this method when more than one parameters to be read
        if (navUpdate == true)
        {
            db.navigation.set(nav);
        }

        // status
        if (root.isMember("mode"))
        {
            sys.mode = db.string_to_mode(root["mode"].asString());
            db.status.set(sys);
            // sysUpdate = true;
        }

        // use this method when more than one parameters to be read
        // if (sysUpdate == true)
        // {
        //     db.status.set(sys);
        // }
    }
    else if (strcmp(msg->topic, BMS_STATUS_TOPIC) == 0)
    {
        // handles navogation manager status message
        auto &db = DataManager::instance();
        auto bms = db.bms.get();
        // auto bmsUpdate = false;

        if (root.isMember("voltage"))
            bms.voltage = root["voltage"].asUInt();
        if (root.isMember("current"))
            bms.current = root["current"].asUInt();
        if (root.isMember("remainCapacity"))
            bms.capacityRemain = root["remainCapacity"].asUInt();
        if (root.isMember("soc"))
            bms.chargePercent = root["soc"].asUInt();

        // commit to data dictionary
        // if (bmsUpdate == true)
        {
            db.bms.set(bms, 0.01);
        }
    }
}

/**
 * Connect to MQTT broker with retries
 */
int mqtt_connect_retry(int max_retries = 10)
{
    int retry_count = 0;

    while (retry_count < max_retries && running)
    {
        printf("[MQTT] Connecting to %s:%d (attempt %d/%d)...\n",
               MQTT_BROKER, MQTT_PORT, retry_count + 1, max_retries);

        int ret = mosquitto_connect(mqtt_client, MQTT_BROKER, MQTT_PORT, 60);

        if (ret == MOSQ_ERR_SUCCESS)
        {
            printf("[MQTT] Connection successful\n");
            return 0;
        }

        printf("[MQTT] Connection failed: %s\n", mosquitto_strerror(ret));
        retry_count++;
        sleep(2);
    }

    return -1;
}

/**
 * Publish mission data
 */
void publish_mission_data()
{
    Json::Value root;
    auto &db = DataManager::instance();
    auto cmnd = db.commands.get();

    // auto stat = db.status.get();

    root["mode"] = db.mode_to_string(cmnd.mode);

    root["timestamp"] = (Json::UInt64)std::chrono::system_clock::now().time_since_epoch().count();

    if (cmnd.mode == OperationMode::OP_MODE_AUTONOMOUS)
    {
        // autonomous
        root["target_latitude"] = SIX_DECIMAL_SCALE(cmnd.targetLatitude);
        root["target_longitude"] = SIX_DECIMAL_SCALE(cmnd.targetLongitude);
        root["target_radius"] = cmnd.targetRadius;
        root["target_depth"] = TWO_DECIMAL_SCALE(cmnd.targetDepth);
        root["target_speed"] = TWO_DECIMAL_SCALE(cmnd.targetSpeed);
    }
    else if (cmnd.mode == OperationMode::OP_MODE_SEMI_AUTONOMOUS)
    {
        // semi-autonomus
        root["target_heading_semi"] = TWO_DECIMAL_SCALE(cmnd.commandedHeading_semi);
        root["target_depth_semi"] = TWO_DECIMAL_SCALE(cmnd.commandedDepth_semi);
        root["target_speed_semi"] = TWO_DECIMAL_SCALE(cmnd.commandedSpeed_semi);
        root["target_duration_semi"] = cmnd.commandedDuration_semi;
    }
    else if (cmnd.mode == OperationMode::OP_MODE_MANUAL)
    {
        // manual
        root["fin_percent"] = TWO_DECIMAL_SCALE(cmnd.commandedFinAngle[FINS::LEFT]);
        root["rudder_percent"] = TWO_DECIMAL_SCALE(cmnd.commandedFinAngle[FINS::TOP]);
        root["thruster_percent"] = cmnd.commandedThrust[THRUSTERS::SURGE_1];
    }
    else // standby
    {
        // home location
        root["home_latitude"] = cmnd.homeLatitude;
        root["home_longitude"] = cmnd.homeLongitude;
    }

    Json::StreamWriterBuilder writer;
    std::string payload = Json::writeString(writer, root);

    int ret = mosquitto_publish(mqtt_client, NULL, MISSION_TOPIC,
                                payload.length(), payload.c_str(), 1, false);

    if (ret == MOSQ_ERR_SUCCESS)
    {
        // control_count++;
    }
    else
    {
        printf("[MQTT] Publish failed: %s\n", mosquitto_strerror(ret));
    }
}

/**
 * Main control loop
 */
int main(int argc, char *argv[])
{
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║   Mission Manager - C++ Application                 ║\n");
    printf("║   Receiving on MQTT: %s, %s                ║\n", NAV_CONTROL_TOPIC, IMU_HNAV_TOPIC);
    printf("║   Publishing to: %s        ║\n", MISSION_TOPIC);
    printf("╚════════════════════════════════════════════════════════╝\n\n");

    if (argc > 1)
    {
        // MQTT_BROKER = argv[1];
        // printf("[Config] MQTT Broker set to: %s\n", MQTT_BROKER);
        for (uint8_t i = 0; i < argc; i++)
        {
            printf("[Config] Arg %d: %s\n", i, argv[i]);
        }
    }

    // Initialize signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // initialize mode_manager
    // mode_manager = new ModeManager();

    // Initialize MQTT
    mosquitto_lib_init();

    mqtt_client = mosquitto_new(CLIENT_ID, true, NULL);
    if (!mqtt_client)
    {
        printf("[MQTT] Failed to create client\n");
        return 1;
    }

    // Set callbacks
    mosquitto_connect_callback_set(mqtt_client, mqtt_on_connect);
    mosquitto_disconnect_callback_set(mqtt_client, mqtt_on_disconnect);
    mosquitto_message_callback_set(mqtt_client, mqtt_on_message);
    mosquitto_subscribe_callback_set(mqtt_client, [](struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
                                     { printf("[MQTT] Subscription acknowledged (mid: %d) for QoS %d\n", mid, granted_qos[0]); });

    printf("[Config] MQTT Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    printf("[Config] MQTT Client ID: %s\n", CLIENT_ID);

    // Connect to MQTT
    if (mqtt_connect_retry() != 0)
    {
        printf("[MQTT] Failed to connect after retries\n");
        mosquitto_destroy(mqtt_client);
        mosquitto_lib_cleanup();
        // delete mode_manager;
        return 1;
    }

    // Start MQTT loop in background thread
    int loop_ret = mosquitto_loop_start(mqtt_client);
    if (loop_ret != MOSQ_ERR_SUCCESS)
    {
        printf("[MQTT] Failed to start loop: %s\n", mosquitto_strerror(loop_ret));
        mosquitto_destroy(mqtt_client);
        mosquitto_lib_cleanup();
        // delete mode_manager;
        return 1;
    }

    printf("[Main] Application started. Press Ctrl+C to exit.\n\n");

    // Main control loop
    // std::chrono::steady_clock::time_point last_control_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point last_status_time = std::chrono::steady_clock::now();
    // int32_t numberBytesReceived;
    // unsigned char buffer[MAX_MESSAGE_DIMENSION] = {0};
    ConfigData config;
    strcpy(config.guiWiFiIP, UDP_GCS_IP);
    config.guiWiFiTxPort = UDP_GCS_TX_PORT;
    config.guiWiFiRxPort = UDP_GCS_RX_PORT;

#if 0
    // dummy variables for testing
    systemData.telemetry.latitude = 37.7749;
    systemData.telemetry.longitude = -122.4194;
    systemData.telemetry.depth = 10.5;
    systemData.telemetry.speed = 1.5;
    systemData.telemetry.velocity_x = 1.0;
    systemData.telemetry.velocity_y = 0.5;
    systemData.telemetry.velocity_z = 0.0;
    systemData.telemetry.heading = 90.0;
    systemData.telemetry.pitch = 5.0;
    systemData.telemetry.roll = 2.0;

    systemData.thruster.thrust = 50; // in percent
    systemData.thruster.rpm = 500;
    systemData.thruster.voltage = 48;     // in volts
    systemData.thruster.current = 10;     // in amps
    systemData.thruster.temperature = 25; // in degrees Celsius

    systemData.port.angle = 10.0;
    systemData.port.angleCommanded = 10.0;
    systemData.port.voltage = 12.0;
    systemData.port.current = 1.0;
    systemData.port.position = 100;
    systemData.port.temperature = 30.0;

    systemData.starboard.angle = -10.0;
    systemData.starboard.angleCommanded = -10.0;
    systemData.starboard.voltage = 12.0;
    systemData.starboard.current = 1.0;
    systemData.starboard.position = -100;
    systemData.starboard.temperature = 30.0;

    systemData.topRudder.angle = 5.0;
    systemData.topRudder.angleCommanded = 5.0;
    systemData.topRudder.voltage = 12.0;
    systemData.topRudder.current = 1.0;
    systemData.topRudder.position = 50;
    systemData.topRudder.temperature = 30.0;

    systemData.bottomRudder.angle = -5.0;
    systemData.bottomRudder.angleCommanded = -5.0;
    systemData.bottomRudder.voltage = 12.0;
    systemData.bottomRudder.current = 1.0;
    systemData.bottomRudder.position = -50;
    systemData.bottomRudder.temperature = 30.0;

    systemData.bms.voltage = 48;         // in millivolts
    systemData.bms.current = 120;        // in milliamps
    systemData.bms.capacityRemain = 100; // in Ah
    systemData.bms.chargePercent = 90;

    //------------------------------------------------------
    auto &db = DataManager::instance();

    NavigationData nav;
    nav.latitude = 13.70;
    nav.longitude = 84.35;
    nav.depth = 10.5;
    db.navigation.set(nav, 0.0001);

    //===
    AttitudeData att;

    att.roll = 10.0;
    att.pitch = 11.0;
    att.yaw = 12.0;

    att.rollRate = 1.0;
    att.pitchRate = 2.0;
    att.yawRate = 3.0;

    db.attitude.set(att, 0.01);
#endif
    //------------------------------------------------------

    // intitialize and start GUI WiFi Task
    GuiWiFiTask guiwifi(&config /*, &systemData*/);
    guiwifi.start();

    while (running)
    {
        auto now = std::chrono::steady_clock::now();
        auto elapsed_status = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_status_time);

        // Control loop at 20 Hz
        // if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_control_time).count() >= 1000)
        if (elapsed_status.count() >= 1000)
        {
            //===
            auto &db = DataManager::instance();
            auto cmnd = db.commands.get();
            auto sys = db.status.get();
            auto nav = db.navigation.get();

            if ((sys.mode == OperationMode::OP_MODE_AUTONOMOUS) &&
                (sys.armed == true) && (sys.totalMisnlegs > 0))
            {
                // autnomous mission to be started
                auto misn = db.autoMissionData.get();
                auto nav = db.navigation.get();

                if (/*nav.distanceToTarget <= misn.misnRadius[sys.currentMisnLegPerforming]*/
                    nav.target_acheived == true)
                {
                    // consider reached to destination waypoint
                    sys.currentMisnLegPerforming++; // advance to next waypoint
                    if (sys.currentMisnLegPerforming >= sys.totalMisnlegs)
                    {
                        // mission finished. go to standby or stay there only
                        // std::cout  << "[MAIN] Mission Completed: " << static_cast<int>(sys.currentMisnLegPerforming) << "\n";
                        printf("[MAIN] Mission completed, going standby\n");

                        // sys.currentMisnLegPerforming--; // stay at last waypoint
                        cmnd.mode = OperationMode::OP_MODE_STANDBY;
                    }
                }
                cmnd.targetLatitude = misn.misnlatitude[sys.currentMisnLegPerforming];
                cmnd.targetLongitude = misn.misnLongitude[sys.currentMisnLegPerforming];
                cmnd.targetDepth = misn.misnDepthAltitude[sys.currentMisnLegPerforming];
                cmnd.targetSpeed = (float)misn.misnSpeed[sys.currentMisnLegPerforming];
                cmnd.targetRadius = (float)misn.misnRadius[sys.currentMisnLegPerforming];
                db.commands.set(cmnd, 0);
            }
            // sys.mode = cmnd.mode;
            // db.status.set(sys, 0);

            publish_mission_data();
            last_status_time = now;
        }

        // // Receive data from host on server port
        // numberBytesReceived = UdpServer_recv(&udpServer, buffer, (uint16_t)MAX_MESSAGE_DIMENSION);
        // if (numberBytesReceived > 0)
        // {
        //     printf("[UDP] Received %d bytes from host\n", numberBytesReceived);
        //     // Process the received data as needed
        // }

        usleep(1000000); // Sleep for 1000ms to reduce CPU usage
    }
}
