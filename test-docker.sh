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

# Parse command line arguments
TEST_FILTER=""
PARALLEL_JOBS=$(nproc)
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--filter)
            TEST_FILTER="$2"
            shift 2
            ;;
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [options]"
            echo "Options:"
            echo "  -f, --filter PATTERN    Run only tests matching pattern"
            echo "  -j, --jobs N           Use N parallel jobs (default: $(nproc))"
            echo "  -v, --verbose          Verbose output (shows individual tests)"
            echo "  -h, --help             Show this help"
            echo ""
            echo "Examples:"
            echo "  $0                     # Run all tests"
            echo "  $0 -v                  # Show individual test output"
            echo "  $0 -f utils            # Run only utility tests"
            echo "  $0 -j 4 -v            # Use 4 jobs with verbose output"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

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
if [ -n "$TEST_FILTER" ]; then
    print_status "Running tests with filter: $TEST_FILTER"
fi
if [ "$VERBOSE" = true ]; then
    print_status "Verbose output enabled - showing individual test details"
fi
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

        echo 'Building project...'
        make -j$PARALLEL_JOBS

        echo 'Listing available tests...'
        echo '=================================='
        ctest -N
        echo '=================================='

        echo 'Running tests with CTest...'
        echo '=================================='

        # Build CTest command based on options
        if [ '$VERBOSE' = true ]; then
            # Verbose mode - shows all individual test output
            if [ -n '$TEST_FILTER' ]; then
                ctest --output-on-failure --verbose -L '$TEST_FILTER' -j$PARALLEL_JOBS
            else
                ctest --output-on-failure --verbose -j$PARALLEL_JOBS
            fi
        else
            # Normal mode - just summary
            if [ -n '$TEST_FILTER' ]; then
                ctest --output-on-failure -L '$TEST_FILTER' -j$PARALLEL_JOBS
            else
                ctest --output-on-failure -j$PARALLEL_JOBS
            fi
        fi

        # Check result
        if [ \$? -eq 0 ]; then
            echo ''
            echo '=== Test Summary ==='
            echo 'All tests passed successfully!'
        else
            echo ''
            echo '=== Failed Tests Details ==='
            echo 'Some tests failed. Rerunning failed tests with verbose output:'
            ctest --rerun-failed -V || true
            exit 1
        fi

        echo '=================================='
        echo 'CTest execution completed!'
    "

# Check exit code
if [ $? -eq 0 ]; then
    echo ""
    print_success "All tests passed!"
    echo ""
    print_status "Available commands:"
    echo "  ./test-docker.sh                    # Run all tests (summary)"
    echo "  ./test-docker.sh -v                # Show individual test output"
    echo "  ./test-docker.sh -f utils          # Run only utility tests"
    echo "  ./test-docker.sh -j 8 -v          # 8 parallel jobs with verbose output"
else
    echo ""
    print_error "Some tests failed!"
    exit 1
fi