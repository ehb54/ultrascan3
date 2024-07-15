# Use an Alpine Linux base image with the necessary build tools
FROM alpine:3.18

# Install necessary packages including Qt5
RUN apk update && \
    apk add --no-cache cmake make g++ git \
    qt5-qtbase-dev qt5-qtdeclarative-dev qt5-qttools-dev qt5-qtsvg-dev qt5-qtx11extras-dev \
    && rm -rf /var/cache/apk/*

## get source
RUN git clone -b aaron-dev-test https://github.com/ehb54/ultrascan3.git

# Create a build directory
RUN mkdir -p /ultrascan3/build

# Set the working directory to the build directory
WORKDIR /ultrascan3/build

# Create log directory
RUN mkdir -p /app/logs

# Run cmake, make, and ctest, capturing output to log files
RUN cmake .. 2>&1 | tee /app/logs/cmake_output.log && \
    make 2>&1 | tee /app/logs/make_output.log && \
    ctest --output-on-failure 2>&1 | tee /app/logs/ctest_output.log
