cmake_minimum_required(VERSION 3.10)
project(mission_manager CXX C)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# Find required packages
if(NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(MOSQUITTO libmosquitto)
    pkg_check_modules(JSONCPP REQUIRED jsoncpp)
else()
    # Windows: manually set library paths
    set(JSONCPP_FOUND TRUE)
    set(JSONCPP_INCLUDE_DIRS "C:/msys64/mingw64/include")
    set(JSONCPP_LIBRARIES "jsoncpp")
    set(JSONCPP_LIBRARY_DIRS "C:/msys64/mingw64/lib")
    link_directories("C:/msys64/mingw64/lib")
    set(MOSQUITTO_FOUND FALSE)  # Mosquitto is optional on Windows
endif()

# Add executable
if(WIN32)
    add_executable(mission_manager
        mission_manager.cpp
        json_stub.cpp
    )
else()
    add_executable(mission_manager
        mission_manager.cpp
        common/udp/udpserver.cpp
        common/udp/udpclient.cpp
        GuiWiFiTask.cpp
    )
endif()

# Include directories
target_include_directories(mission_manager PRIVATE
    ${JSONCPP_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(MOSQUITTO_FOUND)
    target_include_directories(mission_manager PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
endif()

# Link libraries
if(WIN32)
    target_link_libraries(mission_manager PRIVATE
        "C:/msys64/mingw64/lib/libjsoncpp.a"
        ws2_32
    )
else()
    target_link_libraries(mission_manager PRIVATE
        ${JSONCPP_LIBRARIES}
    )
    
    if(MOSQUITTO_FOUND)
        target_link_libraries(mission_manager PRIVATE ${MOSQUITTO_LIBRARIES})
    endif()
    
    target_link_libraries(mission_manager PRIVATE pthread m)
endif()

# Compiler flags for better debugging and optimization
target_compile_options(mission_manager PRIVATE
    -Wall
    -Wextra
    -O2
    -g
    
)

# Installation
install(TARGETS mission_manager
    RUNTIME DESTINATION /app/mission
)
