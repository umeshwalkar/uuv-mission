# FROM alpine:3.16.3
# FROM uuv-base
FROM ubuntu:22.04

# Install C++ build dependencies
# RUN apt-get update && apt-get install -y \
#     build-essential \
#     cmake \
#     libmosquitto-dev \
#     libjsoncpp-dev \
#     pkg-config \
#     && rm -rf /var/lib/apt/lists/*

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y tzdata
RUN apt-get install -y vim build-essential git cmake net-tools gdb clang 
RUN apt-get install -y libmosquitto-dev libjsoncpp-dev pkg-config

# WORKDIR /app

# # Copy source files
# COPY mode_management.hpp .
# COPY mode_management.cpp .
# COPY navigation_manager.cpp .
# COPY CMakeLists.txt .

# Build application with verbose output
# RUN mkdir -p build && \
#     cd build && \
#     cmake -DCMAKE_BUILD_TYPE=Release .. && \
#     cmake --build . --verbose && \
#     cd .. && \
#     echo "✓ Navigation module built successfully"

# Verify executable exists
# RUN test -f ./build/navigation_manager || (echo "ERROR: navigation_manager executable not found" && exit 1)

# CMD ["./build/navigation_manager"]

# FROM mcr.microsoft.com/devcontainers/cpp:1-debian-12

# ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="none"

# # Optionally install the cmake for vcpkg
# COPY ./reinstall-cmake.sh /tmp/

# RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
#         chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
#     fi \
#     && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

EXPOSE 5500/udp
EXPOSE 5510/udp