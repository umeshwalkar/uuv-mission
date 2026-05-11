#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <mosquitto.h>
#include <json/json.h>
#include "structures.hpp"

#include "common/udp/udpclient.h"
#include "common/udp/udpserver.h"
#include "common/udp/udpSerialize.h"

#define UDP_GCS_PORT        5500
#define UDP_GCS_IP          "127.0.0.1"  // Change to actual GCS IP if needed
#define MAX_MESSAGE_DIMENSION 1024

// Configuration
const char* MQTT_BROKER =  "host.docker.internal"; //"127.0.0.1";  //"mqtt-broker";
// const char* MQTT_BROKER =  "127.0.0.1";  //"mqtt-broker";

int MQTT_PORT = 1883;
const char* CLIENT_ID = "mission-manager-dev";

// MQTT Topics
// const char* IMU_TOPIC = "uuv/sensors/imu";
const char* IMU_HNAV_TOPIC = "uuv/sensors/imu/hnav";
const char* MISSION_TOPIC = "uuv/mission";
const char* NAV_CONTROL_TOPIC = "uuv/navigation/control";
const char* NAV_STATUS_TOPIC = "uuv/navigation/status";

// Global state
volatile sig_atomic_t running = 1;
struct mosquitto* mqtt_client = NULL;
int mqtt_connected = 0;
// ModeManager* mode_manager = NULL;
UdpServerStruct udpServer;

// Counters
int nav_control_count = 0;
int nav_status_count = 0;
int imu_count = 0;

// Scaling factors
#define SIX_DECIMAL_SCALE(x)  (round((x) * 1000000.0) / 1000000.0) // Scale values to 6 decimal places 
#define THREE_DECIMAL_SCALE(x)  (round((x) * 1000.0) / 1000.0) // Scale values to 3 decimal places
#define TWO_DECIMAL_SCALE(x)  (round((x) * 100.0) / 100.0) // Scale values to 2 decimal places


/**
 * Signal handler for graceful shutdown
 */
void signal_handler(int sig) {
    printf("[NavManager] Received signal %d, shutting down...\n", sig);
    running = 0;
}


/**
 * MQTT connection callback
 */
void mqtt_on_connect(struct mosquitto* mosq, void* userdata, int result) {
    if (result == 0) {
        printf("[MQTT] Connected successfully (result: %d)\n", result);
        mqtt_connected = 1;
        
        // Subscribe to topics
        
        mosquitto_subscribe(mosq, NULL, NAV_CONTROL_TOPIC, 1);
        // mosquitto_subscribe(mosq, NULL, NAV_STATUS_TOPIC, 1);
        mosquitto_subscribe(mosq, NULL, IMU_HNAV_TOPIC, 1);

        printf("[MQTT] Subscribed to topics:\n");
        printf("  - %s\n", NAV_CONTROL_TOPIC);
        // printf("  - %s\n", NAV_STATUS_TOPIC);
        printf("  - %s\n", IMU_HNAV_TOPIC);
        
    } else {
        printf("[MQTT] Connection failed (result: %d)\n", result);
        mqtt_connected = 0;
    }
}

/**
 * MQTT disconnect callback
 */
void mqtt_on_disconnect(struct mosquitto* mosq, void* userdata, int result) {
    printf("[MQTT] Disconnected (result: %d)\n", result);
    mqtt_connected = 0;
}

/**
 * MQTT message received callback
 */
void mqtt_on_message(struct mosquitto* mosq, void* userdata, 
                     const struct mosquitto_message* msg) {
    if (msg->payloadlen == 0) return;
    
    char payload[4096];
    strncpy(payload, (char*)msg->payload, std::min(msg->payloadlen, 4095));
    payload[msg->payloadlen] = '\0';
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(payload);
    
    if (!Json::parseFromStream(builder, s, &root, &errs)) {
        printf("[MQTT] Failed to parse JSON from topic %s: %s\n", msg->topic, errs.c_str());
        return;
    }

    // Handle IMU data (HNAV)
    if (strcmp(msg->topic, NAV_CONTROL_TOPIC) == 0) {
        nav_control_count++;
        NavControlData nav;

        // mode_manager->update_navigation_state(root);
        
         // Parse autonomous parameters
        if (root.isMember("thrust")) nav.thruster.thrust_percent = root["thrust"].asDouble();
        if (root.isMember("thruster_rpm")) nav.thruster.rpm = root["thruster_rpm"].asInt();
        
        if (root.isMember("bottom_rudder")) nav.bottomRudder.angle_deg = root["bottom_rudder"].asDouble();
        if (root.isMember("top_rudder")) nav.topRudder.angle_deg = root["top_rudder"].asDouble();
        if (root.isMember("starboard_rudder")) nav.starboard.angle_deg = root["starboard_rudder"].asDouble();
        if (root.isMember("port_rudder")) nav.port.angle_deg = root["port_rudder"].asDouble();
        

        if (nav_control_count % 30 == 0) {
            // const auto& nav = mode_manager->get_navigation_state();
            printf("[MisManager] NAV Update #%d: Thrust=%.1f %%, Top=%.1f, Bottom=%.1f, port=%.1f, starboard=%.1f\n",
                   nav_control_count, nav.topRudder.angle_deg, nav.bottomRudder.angle_deg, nav.port.angle_deg, nav.starboard.angle_deg);
            
        }
    }

    // Handle IMU data (HNAV)
    else if (strcmp(msg->topic, IMU_HNAV_TOPIC) == 0) {
        imu_count++;
        
        TelemetryData Telemetry;

        if (root.isMember("latitude")) Telemetry.latitude = root["latitude"].asDouble();
        if (root.isMember("longitude")) Telemetry.longitude = root["longitude"].asDouble();
        if (root.isMember("depth")) Telemetry.depth = root["depth"].asDouble();
        if (root.isMember("heading")) Telemetry.heading = root["heading"].asDouble();
        if (root.isMember("pitch")) Telemetry.pitch = root["pitch"].asDouble();
        if (root.isMember("roll")) Telemetry.roll = root["roll"].asDouble();
        if (root.isMember("speed")) Telemetry.speed = root["speed"].asDouble();
        // if (root.isMember("velocity_x")) Telemetry.velocity_x = imu_data["velocity_x"].asDouble();
        // if (root.isMember("velocity_y")) Telemetry.velocity_y = imu_data["velocity_y"].asDouble();
        // if (root.isMember("velocity_z")) nav_state.velocity_z = imu_data["velocity_z"].asDouble();
        
        if (imu_count % 30 == 0) {
              printf("[MisManager] IMU Update #%d: Lat=%.6f, Lon=%.6f, Depth=%.2f, Heading=%.1f\n",
                   imu_count, Telemetry.latitude, Telemetry.longitude, Telemetry.depth, Telemetry.heading);
        }
    }
}

/**
 * Connect to MQTT broker with retries
 */
int mqtt_connect_retry(int max_retries = 10) {
    int retry_count = 0;
    
    while (retry_count < max_retries && running) {
        printf("[MQTT] Connecting to %s:%d (attempt %d/%d)...\n", 
               MQTT_BROKER, MQTT_PORT, retry_count + 1, max_retries);
        
        int ret = mosquitto_connect(mqtt_client, MQTT_BROKER, MQTT_PORT, 60);
        
        if (ret == MOSQ_ERR_SUCCESS) {
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
 * Main control loop
 */
int main(int argc, char* argv[]) {
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║   Mission Manager - C++ Application                 ║\n");
    printf("║   Receiving on MQTT: %s, %s                ║\n", NAV_CONTROL_TOPIC, IMU_HNAV_TOPIC);
    printf("║   Publishing to: %s        ║\n", MISSION_TOPIC);
    printf("╚════════════════════════════════════════════════════════╝\n\n");
    
    if (argc > 1) {
        // MQTT_BROKER = argv[1];
        // printf("[Config] MQTT Broker set to: %s\n", MQTT_BROKER);
        for(uint8_t i=0; i<argc; i++) {
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
    if (!mqtt_client) {
        printf("[MQTT] Failed to create client\n");
        return 1;
    }
    
      // Set callbacks
    mosquitto_connect_callback_set(mqtt_client, mqtt_on_connect);
    mosquitto_disconnect_callback_set(mqtt_client, mqtt_on_disconnect);
    mosquitto_message_callback_set(mqtt_client, mqtt_on_message);
    mosquitto_subscribe_callback_set(mqtt_client, [](struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos) {
        printf("[MQTT] Subscription acknowledged (mid: %d) for QoS %d\n", mid, granted_qos[0]);
    });
    
    printf("[Config] MQTT Broker: %s:%d\n", MQTT_BROKER, MQTT_PORT);
    printf("[Config] MQTT Client ID: %s\n", CLIENT_ID);
    
    // Connect to MQTT
    if (mqtt_connect_retry() != 0) {
        printf("[MQTT] Failed to connect after retries\n");
        mosquitto_destroy(mqtt_client);
        mosquitto_lib_cleanup();
        // delete mode_manager;
        return 1;
    }
    
    // Start MQTT loop in background thread
    int loop_ret = mosquitto_loop_start(mqtt_client);
    if (loop_ret != MOSQ_ERR_SUCCESS) {
        printf("[MQTT] Failed to start loop: %s\n", mosquitto_strerror(loop_ret));
        mosquitto_destroy(mqtt_client);
        mosquitto_lib_cleanup();
        // delete mode_manager;
        return 1;
    }

    //bind the server on the receive port
    UdpServer_init(&udpServer, UDP_GCS_PORT);
    printf("[Config] UDP Server Port: %d\n", UDP_GCS_PORT);
    
    printf("[Main] Application started. Press Ctrl+C to exit.\n\n");
    
    // Main control loop
    std::chrono::steady_clock::time_point last_control_time = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point last_status_time = std::chrono::steady_clock::now();
    while (running) {
        auto now = std::chrono::steady_clock::now();
        
        // Control loop at 20 Hz
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_control_time).count() >= 50) {
        }

        int32_t numberBytesReceived;
        unsigned char buffer[MAX_MESSAGE_DIMENSION] = {0};
        numberBytesReceived = UdpServer_recv(&udpServer, buffer, (uint16_t)MAX_MESSAGE_DIMENSION);
        if(numberBytesReceived > 0) {
            printf("[UDP] Received %d bytes from GCS\n", numberBytesReceived);
            // Process the received data as needed
        }
        
        usleep(10000); // Sleep for 10ms to reduce CPU usage
    }
}
