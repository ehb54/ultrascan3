#!/bin/bash
set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if Docker is available
print_status "Checking for Docker..."
if ! command -v docker &> /dev/null; then
    print_error "Docker not found. Please install Docker first."
    exit 1
fi

print_success "Docker found: $(docker --version)"

# Check if Docker daemon is running
print_status "Checking Docker daemon..."
if ! docker info &> /dev/null; then
    print_error "Docker daemon is not running."
    exit 1
fi

print_success "Docker daemon is running"

# Build our custom image (only rebuilds if Dockerfile changed)
print_status "Building test image..."
docker build -f admin/test/docker/Dockerfile -t us3comp-test:latest .

print_success "Image built successfully"

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_warning "CMakeLists.txt not found in current directory"
    print_warning "Make sure you're running this from the ultrascan3 root directory"
fi

# Create persistent build directory
print_status "Setting up persistent build directory..."
mkdir -p build-docker

# Run tests in container
print_status "Starting test container..."
echo ""

docker run --rm \
    -v "$(pwd)":/ultrascan3 \
    -v "$(pwd)/build-docker":/ultrascan3/build-docker \
    -w /ultrascan3 \
    us3comp-test:latest \
    bash -c "
        set -e

        echo '=== System Information ==='
        gcc --version
        g++ --version
        cmake --version
        cat /etc/os-release
        echo '=========================='

        # Go to build directory (now persistent)
        cd /ultrascan3/build-docker
        pwd  # Verify we're in build directory

        echo 'Configuring with CMake (if needed)...'
        if [ ! -f CMakeCache.txt ]; then
            echo 'Running initial CMake configuration...'
            cmake -DCMAKE_BUILD_TYPE=Debug ..
        else
            echo 'Using existing CMake configuration (incremental build)'
        fi

        echo 'Compiling test executable...'
        make test_us_utils -j\$(nproc)

        echo 'Running tests...'
        echo '=================================='
        if [ -f test/utils/test_us_utils ]; then
            echo 'Found test executable: test/utils/test_us_utils'
            ./test/utils/test_us_utils
        else
            echo 'Searching for test executable...'
            find . -name 'test_us_utils' -type f -executable
            TEST_EXECUTABLE=\$(find . -name 'test_us_utils' -type f -executable | head -1)
            if [ -n \"\$TEST_EXECUTABLE\" ]; then
                echo \"Found: \$TEST_EXECUTABLE\"
                \"\$TEST_EXECUTABLE\"
            else
                echo 'No test executable found'
                exit 1
            fi
        fi
        echo '=================================='
        echo 'All tests completed!'
    "

# Check exit code
if [ $? -eq 0 ]; then
    echo ""
    print_success "All tests passed!"
else
    echo ""
    print_error "Some tests failed!"
    exit 1
fi