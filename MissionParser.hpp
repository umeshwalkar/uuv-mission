#ifndef _missionparser_h
#define _missionparser_h

// #pragma once

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <variant>
#include <vector>
#include <iostream>

namespace mission
{

    //=============================================================================
    // ENUMS
    //=============================================================================

    enum class MissionOperationType
    {
        CONFIG,
        CONFIGS,
        GOTOWP,
        HOVERING,
        FLOATING,
        SURFACING,
        ENDMISSION
    };

    enum class SensorState : uint8_t
    {
        OFF = 0,
        ON = 1,
        ACQUIRING = 2
    };

    //=============================================================================
    // PARSE ERROR
    //=============================================================================

    enum class MissionParseError
    {
        NONE,

        FILE_OPEN_FAILED,
        EMPTY_FILE,

        INVALID_CHECKSUM,
        INVALID_FIELD_COUNT,
        INVALID_OPERATION,
        INVALID_ID_SEQUENCE,

        CONFIG_NOT_FIRST,
        ENDMISSION_NOT_LAST,
        CONFIGS_REQUIRED_BEFORE_GOTOWP,

        INVALID_NUMERIC_VALUE
    };

    //=============================================================================
    // OPERATION STRUCTURES
    //=============================================================================

    struct ConfigOperation
    {
        uint32_t lsvAuvType{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};
        uint32_t spare4{};
        uint32_t spare5{};
    };

    struct ConfigSensorsOperation
    {
        uint32_t mbes_fs{};
        uint32_t mbes_tc{};
        uint32_t mbes_ta{};
        uint32_t mbes_tr{};
        uint32_t mbes_pt{};
        uint32_t mbes_bs{};
        uint32_t mbes_sr{};
        uint32_t mbes_ssc{};

        uint32_t mbes_spare1{};
        uint32_t mbes_spare2{};
        uint32_t mbes_spare3{};

        uint32_t sss_hon{};
        uint32_t sss_hr{};
        uint32_t sss_lon{};
        uint32_t sss_lr{};

        uint32_t sss_spare1{};
        uint32_t sss_spare2{};
        uint32_t sss_spare3{};

        uint32_t sbp_pr{};
        uint32_t sbp_ps{};

        uint32_t sbp_spare1{};
        uint32_t sbp_spare2{};
        uint32_t sbp_spare3{};

        uint32_t cam_iv{};

        uint32_t cam_spare1{};
        uint32_t cam_spare2{};
        uint32_t cam_spare3{};
    };

    struct GoToWpOperation
    {
        int32_t latitude{};
        int32_t longitude{};
        int32_t depthAltitude{};

        uint16_t speed{};
        uint16_t bubbleSize{};

        SensorState mbes{};
        SensorState sss{};
        SensorState sbp{};
        SensorState camera{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};

        uint32_t lawnMowerSize{};
    };

    struct HoveringOperation
    {
        uint32_t timeSec{};

        SensorState mbes{};
        SensorState sss{};
        SensorState sbp{};
        SensorState camera{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};
    };

    struct FloatingOperation
    {
        uint32_t timeSec{};

        SensorState mbes{};
        SensorState sss{};
        SensorState sbp{};
        SensorState camera{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};
    };

    struct SurfacingOperation
    {
        SensorState mbes{};
        SensorState sss{};
        SensorState sbp{};
        SensorState camera{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};
    };

    struct EndMissionOperation
    {
        uint32_t typeEnd{};

        SensorState mbes{};
        SensorState sss{};
        SensorState sbp{};
        SensorState camera{};

        uint32_t spare1{};
        uint32_t spare2{};
        uint32_t spare3{};
    };

    //=============================================================================
    // MISSION DATA POINT
    //=============================================================================

    using MissionOperationData = std::variant<
        ConfigOperation,
        ConfigSensorsOperation,
        GoToWpOperation,
        HoveringOperation,
        FloatingOperation,
        SurfacingOperation,
        EndMissionOperation>;

    struct MissionDataPoint
    {
        uint32_t id{};

        MissionOperationType type{};

        MissionOperationData operationData{};
    };

    struct MissionDataPoints
    {
        std::vector<MissionDataPoint> operations;
    };

    //=============================================================================
    // PARSE RESULT
    //=============================================================================

    struct MissionParseResult
    {
        bool success{false};

        MissionParseError error{MissionParseError::NONE};

        uint32_t lineNumber{0};

        std::string errorMessage;

        MissionDataPoints missionData;
    };

    //=============================================================================
    // PARSER
    //=============================================================================

    class MissionParser
    {
    public:
        static MissionParseResult parseFile(const std::string &filePath)
        {
            MissionParseResult result;

            std::ifstream file(filePath);

            if (!file.is_open())
            {
                result.error = MissionParseError::FILE_OPEN_FAILED;
                result.errorMessage = "Unable to open file";
                return result;
            }

            std::string line;

            uint32_t expectedId = 0;
            uint32_t lineNumber = 0;

            bool configSeen = false;
            bool configsSeen = false;

            while (std::getline(file, line))
            {
                ++lineNumber;

                trimCR(line);

                if (line.empty())
                {
                    continue;
                }

                if (!validateChecksum(line))
                {
                    return makeError(
                        MissionParseError::INVALID_CHECKSUM,
                        lineNumber,
                        "Invalid checksum");
                }

                auto fields = split(line, ';');

                if (fields.size() < 4)
                {
                    return makeError(
                        MissionParseError::INVALID_FIELD_COUNT,
                        lineNumber,
                        "Malformed line");
                }

                uint32_t id = 0;

                if (!toUInt(fields[0], id))
                {
                    return makeError(
                        MissionParseError::INVALID_NUMERIC_VALUE,
                        lineNumber,
                        "Invalid operation ID");
                }

                if (id != expectedId)
                {
                    return makeError(
                        MissionParseError::INVALID_ID_SEQUENCE,
                        lineNumber,
                        "Operation IDs not incremental");
                }

                ++expectedId;

                const std::string &operation = fields[1];

                MissionDataPoint point;

                bool parseOk = false;

                if (operation == "CONFIG")
                {
                    configSeen = true;
                    parseOk = parseConfig(fields, point);
                }
                else if (operation == "CONFIGS")
                {
                    configsSeen = true;
                    parseOk = parseConfigs(fields, point);
                }
                else if (operation == "GOTOWP")
                {
                    if (!configsSeen)
                    {
                        return makeError(
                            MissionParseError::CONFIGS_REQUIRED_BEFORE_GOTOWP,
                            lineNumber,
                            "CONFIGS required before GOTOWP");
                    }

                    parseOk = parseGotoWp(fields, point);
                }
                else if (operation == "HOVERING")
                {
                    parseOk = parseHovering(fields, point);
                }
                else if (operation == "FLOATING")
                {
                    parseOk = parseFloating(fields, point);
                }
                else if (operation == "SURFACING")
                {
                    parseOk = parseSurfacing(fields, point);
                }
                else if (operation == "ENDMISSION")
                {
                    parseOk = parseEndMission(fields, point);
                }
                else
                {
                    return makeError(
                        MissionParseError::INVALID_OPERATION,
                        lineNumber,
                        "Unknown operation");
                }

                if (!parseOk)
                {
                    return makeError(
                        MissionParseError::INVALID_FIELD_COUNT,
                        lineNumber,
                        "Invalid field count");
                }

                result.missionData.operations.push_back(point);
            }

            if (result.missionData.operations.empty())
            {
                return makeError(
                    MissionParseError::EMPTY_FILE,
                    0,
                    "Mission file empty");
            }

            if (result.missionData.operations.front().type !=
                MissionOperationType::CONFIG)
            {
                return makeError(
                    MissionParseError::CONFIG_NOT_FIRST,
                    1,
                    "First operation must be CONFIG");
            }

            if (result.missionData.operations.back().type !=
                MissionOperationType::ENDMISSION)
            {
                return makeError(
                    MissionParseError::ENDMISSION_NOT_LAST,
                    lineNumber,
                    "Last operation must be ENDMISSION");
            }

            result.success = true;

            return result;
        }

    private:
        //=========================================================================
        // HELPERS
        //=========================================================================

        static MissionParseResult makeError(
            MissionParseError error,
            uint32_t line,
            const std::string &msg)
        {
            MissionParseResult r;

            r.success = false;
            r.error = error;
            r.lineNumber = line;
            r.errorMessage = msg;

            return r;
        }

        static void trimCR(std::string &s)
        {
            if (!s.empty() && s.back() == '\r')
            {
                s.pop_back();
            }
        }

        static std::vector<std::string> split(
            const std::string &line,
            char delimiter)
        {
            std::vector<std::string> tokens;

            std::stringstream ss(line);

            std::string item;

            while (std::getline(ss, item, delimiter))
            {
                tokens.push_back(item);
            }

            return tokens;
        }

        static bool toUInt(
            const std::string &s,
            uint32_t &value)
        {
            try
            {
                value = static_cast<uint32_t>(std::stoul(s));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static bool toInt(
            const std::string &s,
            int32_t &value)
        {
            try
            {
                value = static_cast<int32_t>(std::stol(s));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        
        static bool toUInt16(
            const std::string &s,
            uint16_t &value)
        {
            try
            {
                value = static_cast<uint16_t>(std::stoul(s));
                return true;
            }
            catch (...)
            {
                return false;
            }
        }

        static bool validateChecksum(const std::string &line)
        {
            return true;
            auto fields = split(line, ';');

            if (fields.size() < 2)
            {
                return false;
            }

            std::string checksumField = fields.back();

            std::string payload = line;

            payload.erase(
                payload.size() - checksumField.size());

            payload.erase(payload.size() - 1);

            uint8_t checksum = 0;

            for (char c : payload)
            {
                checksum ^= static_cast<uint8_t>(c);
            }

            std::stringstream ss;

            ss << std::uppercase
               << std::hex
               << std::setw(2)
               << std::setfill('0')
               << static_cast<int>(checksum);

            return ss.str() == checksumField;
        }

        //=========================================================================
        // OPERATION PARSERS
        //=========================================================================

        static bool parseConfig(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 10)
            {
                return false;
            }

            ConfigOperation op;

            toUInt(f[2], op.lsvAuvType);

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::CONFIG;
            out.operationData = op;

            return true;
        }

        static bool parseConfigs(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 31)
            {
                return false;
            }

            ConfigSensorsOperation op;

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::CONFIGS;
            out.operationData = op;

            return true;
        }

        static bool parseGotoWp(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 17)
            {
                return false;
            }

            GoToWpOperation op;

            toInt(f[2], op.latitude);
            toInt(f[3], op.longitude);
            toInt(f[4], op.depthAltitude);
            toUInt16(f[5], op.speed);
            toUInt16(f[6], op.bubbleSize);

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::GOTOWP;
            out.operationData = op;

            return true;
        }

        static bool parseHovering(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 12)
            {
                return false;
            }

            HoveringOperation op;

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::HOVERING;
            out.operationData = op;

            return true;
        }

        static bool parseFloating(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 12)
            {
                return false;
            }

            FloatingOperation op;

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::FLOATING;
            out.operationData = op;

            return true;
        }

        static bool parseSurfacing(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 11)
            {
                return false;
            }

            SurfacingOperation op;

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::SURFACING;
            out.operationData = op;

            return true;
        }

        static bool parseEndMission(
            const std::vector<std::string> &f,
            MissionDataPoint &out)
        {
            if (f.size() != 12)
            {
                return false;
            }

            EndMissionOperation op;

            out.id = std::stoul(f[0]);
            out.type = MissionOperationType::ENDMISSION;
            out.operationData = op;

            return true;
        }

    }; // class MissionParser

    //=============================================================================
    // SENSOR STATE STRING
    //=============================================================================

    static const char *sensorStateToString(SensorState s)
    {
        switch (s)
        {
        case SensorState::OFF:
            return "OFF";

        case SensorState::ON:
            return "ON";

        case SensorState::ACQUIRING:
            return "ACQUIRING";

        default:
            return "UNKNOWN";
        }
    }

    //=============================================================================
    // OPERATION TYPE STRING
    //=============================================================================

    static const char *operationTypeToString(
        MissionOperationType type)
    {
        switch (type)
        {
        case MissionOperationType::CONFIG:
            return "CONFIG";

        case MissionOperationType::CONFIGS:
            return "CONFIGS";

        case MissionOperationType::GOTOWP:
            return "GOTOWP";

        case MissionOperationType::HOVERING:
            return "HOVERING";

        case MissionOperationType::FLOATING:
            return "FLOATING";

        case MissionOperationType::SURFACING:
            return "SURFACING";

        case MissionOperationType::ENDMISSION:
            return "ENDMISSION";

        default:
            return "UNKNOWN";
        }
    }

    //=============================================================================
    // PRINT HELPERS
    //=============================================================================

    static void printConfig(
        const ConfigOperation &op)
    {
        std::cout
            << "  LSV AUV Type : "
            << op.lsvAuvType
            << std::endl;
    }

    static void printConfigs(
        const ConfigSensorsOperation &op)
    {
        std::cout
            << "  MBES FS      : " << op.mbes_fs << std::endl
            << "  MBES TC      : " << op.mbes_tc << std::endl
            << "  MBES TA      : " << op.mbes_ta << std::endl
            << "  MBES TR      : " << op.mbes_tr << std::endl
            << "  SSS HON      : " << op.sss_hon << std::endl
            << "  SSS HR       : " << op.sss_hr << std::endl
            << "  CAM IV       : " << op.cam_iv << std::endl;
    }

    static void printGotoWp(
        const GoToWpOperation &op)
    {
        std::cout
            << "  Latitude     : " << op.latitude << std::endl
            << "  Longitude    : " << op.longitude << std::endl
            << "  Depth/Alt    : " << op.depthAltitude << std::endl
            << "  Speed        : " << op.speed << std::endl
            << "  Bubble Size  : " << op.bubbleSize << std::endl
            << "  MBES         : "
            << sensorStateToString(op.mbes) << std::endl
            << "  SSS          : "
            << sensorStateToString(op.sss) << std::endl
            << "  SBP          : "
            << sensorStateToString(op.sbp) << std::endl
            << "  CAMERA       : "
            << sensorStateToString(op.camera) << std::endl
            << "  Lawn Size    : "
            << op.lawnMowerSize << std::endl;
    }

    static void printHovering(
        const HoveringOperation &op)
    {
        std::cout
            << "  Time Sec     : " << op.timeSec << std::endl
            << "  MBES         : "
            << sensorStateToString(op.mbes) << std::endl
            << "  SSS          : "
            << sensorStateToString(op.sss) << std::endl
            << "  SBP          : "
            << sensorStateToString(op.sbp) << std::endl
            << "  CAMERA       : "
            << sensorStateToString(op.camera) << std::endl;
    }

    static void printFloating(
        const FloatingOperation &op)
    {
        std::cout
            << "  Time Sec     : " << op.timeSec << std::endl
            << "  MBES         : "
            << sensorStateToString(op.mbes) << std::endl
            << "  SSS          : "
            << sensorStateToString(op.sss) << std::endl
            << "  SBP          : "
            << sensorStateToString(op.sbp) << std::endl
            << "  CAMERA       : "
            << sensorStateToString(op.camera) << std::endl;
    }
    static void printSurfacing(
        const SurfacingOperation &op)
    {
        std::cout
            << "  MBES         : "
            << sensorStateToString(op.mbes) << std::endl
            << "  SSS          : "
            << sensorStateToString(op.sss) << std::endl
            << "  SBP          : "
            << sensorStateToString(op.sbp) << std::endl
            << "  CAMERA       : "
            << sensorStateToString(op.camera) << std::endl;
    }

    static void printEndMission(
        const EndMissionOperation &op)
    {
        std::cout
            << "  Type End     : " << op.typeEnd << std::endl
            << "  MBES         : "
            << sensorStateToString(op.mbes) << std::endl
            << "  SSS          : "
            << sensorStateToString(op.sss) << std::endl
            << "  SBP          : "
            << sensorStateToString(op.sbp) << std::endl
            << "  CAMERA       : "
            << sensorStateToString(op.camera) << std::endl;
    }
    //=============================================================================
    // MAIN PRINT FUNCTION
    //=============================================================================

    static void printMissionData(
        const MissionDataPoints &missionData)
    {
        std::cout
            << "=================================================="
            << std::endl;

        std::cout
            << "MISSION OPERATION COUNT : "
            << missionData.operations.size()
            << std::endl;

        std::cout
            << "=================================================="
            << std::endl;

        for (const auto &operation : missionData.operations)
        {
            std::cout << std::endl;

            std::cout
                << "Operation ID   : "
                << operation.id
                << std::endl;

            std::cout
                << "Operation Type : "
                << operationTypeToString(operation.type)
                << std::endl;

            std::visit(
                [](const auto &op)
                {
                    using T = std::decay_t<decltype(op)>;

                    if constexpr (
                        std::is_same_v<T, ConfigOperation>)
                    {
                        printConfig(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, ConfigSensorsOperation>)
                    {
                        printConfigs(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, GoToWpOperation>)
                    {
                        printGotoWp(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, HoveringOperation>)
                    {
                        printHovering(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, FloatingOperation>)
                    {
                        printFloating(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, SurfacingOperation>)
                    {
                        printSurfacing(op);
                    }
                    else if constexpr (
                        std::is_same_v<T, EndMissionOperation>)
                    {
                        printEndMission(op);
                    }
                },
                operation.operationData);

            std::cout
                << "--------------------------------------------------"
                << std::endl;
        }
    }

} // namespace mission

#endif // _missionparser_h

// ==================================================
// MISSION OPERATION COUNT : 5
// ==================================================

// Operation ID   : 0
// Operation Type : CONFIG
//   LSV AUV Type : 1
// --------------------------------------------------

// Operation ID   : 1
// Operation Type : CONFIGS
//   MBES FS      : 0
//   MBES TC      : 63
//   MBES TA      : 35
// --------------------------------------------------

// Operation ID   : 2
// Operation Type : GOTOWP
//   Latitude     : 441471829
//   Longitude    : 90963862
//   Depth/Alt    : -40000
//   Speed        : 150
//   Bubble Size  : 10
//   MBES         : OFF
//   SSS          : OFF
//   SBP          : OFF
//   CAMERA       : OFF
// --------------------------------------------------