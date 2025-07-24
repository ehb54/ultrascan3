# admin/test/docker/Dockerfile
FROM ehb1/us3comp:latest

# Install build dependencies for testing
RUN apt-get update -qq && \
    apt-get install -y \
        cmake \
        libmysqlclient-dev \
        libarchive-dev \
        pkg-config \
        build-essential \
        git \
        libqwt-qt5-dev && \
    rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /ultrascan3

# Add metadata
LABEL description="UltraScan3 Test Environment"
LABEL version="1.0"