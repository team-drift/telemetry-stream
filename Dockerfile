# Build Stage
FROM alpine:3.17.0 AS build

# Install build dependencies
RUN apk update && \
    apk add --no-cache \
        build-base \
        cmake \
        git \
        python3 \
        py3-pip \
        rust \
        cargo \
        boost-dev

# Set up work directory
WORKDIR /app

# Copy only necessary files for the build
COPY CMakeLists.txt .
COPY extern/ extern/

# Clone MAVSDK repository if not already present
RUN if [ ! -d "/app/extern/mavsdk" ]; then \
        git clone --recursive https://github.com/mavlink/MAVSDK.git /app/extern/mavsdk; \
    fi

# Update MAVSDK to a specific version
WORKDIR /app/extern/mavsdk
RUN git fetch --all --tags && \
    git checkout v2.10.2 && \
    git submodule update --init --recursive

# Configure and build MAVSDK
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release \
          -DBUILD_SHARED_LIBS=OFF \
          -DCMAKE_INSTALL_PREFIX=/usr/local \
          .. && \
    cmake --build . --parallel 8 && \
    cmake --install .

# Build the main project
WORKDIR /app
RUN mkdir -p build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . --parallel 8

# Final Stage
FROM alpine:3.17.0

# Install runtime dependencies
RUN apk update && \
    apk add --no-cache \
        libstdc++ \
        boost1.80-filesystem \
        boost1.80-system \
        boost1.80-thread

# Create user and group
RUN addgroup -S shs && adduser -S shs -G shs
USER shs

# Copy the built application and necessary libraries from the build stage
COPY --from=build /usr/local/lib /usr/local/lib
COPY --from=build /app/build/telemetry_stream /app/telemetry_stream

# Set the library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Set the entrypoint for the container
ENTRYPOINT ["/app/telemetry_stream"]