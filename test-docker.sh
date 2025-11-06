#!/bin/bash
set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
MAGENTA='\033[0;35m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}[SUCCESS]${NC} $1"; }
print_warning() { echo -e "${YELLOW}[WARNING]${NC} $1"; }
print_error() { echo -e "${RED}[ERROR]${NC} $1"; }
print_debug() { echo -e "${CYAN}[DEBUG]${NC} $1"; }
print_highlight() { echo -e "${MAGENTA}[HIGHLIGHT]${NC} $1"; }

# Default values
TEST_FILTER=""
SPECIFIC_TEST=""
PARALLEL_JOBS=$(nproc 2>/dev/null || echo 4)
VERBOSE=false
DEBUG_MODE=false
INTERACTIVE=false
GDB=false
VALGRIND=false
REBUILD=false
LIST_TESTS=false
FAILED_ONLY=false
TIMEOUT=""
REPEAT=1
SEARCH_TESTS=""
SHOW_STATS=false
QUICK_MODE=false
SAVE_LOGS=false
STOP_ON_FIRST_FAILURE=false
PROFILE="TEST"


show_help() {
    cat << 'EOF'
Usage: ./test-docker.sh [options]

=== BASIC USAGE ===
  -h, --help               Show this comprehensive help
  -v, --verbose            Verbose output (shows individual tests)
  -q, --quick             Quick mode: minimal output, stop on first failure

=== TEST SELECTION ===
  -f, --filter PATTERN    Run tests matching CTest label pattern
  -t, --test TEST_NAME    Run specific GTest (supports wildcards)
  -s, --search PATTERN    Search for tests containing pattern
  -l, --list              List all available tests and exit
  --failed-only           Rerun only previously failed tests
  --repeat N              Repeat tests N times (useful for flaky tests)

=== DEBUGGING OPTIONS ===
  -d, --debug             Enable debug mode with extra output
  -i, --interactive       Start interactive shell in container
  -g, --gdb               Run specific test under GDB debugger
  --valgrind              Run specific test under Valgrind
  --timeout SECONDS       Set test timeout (default: CTest default)
  --stop-on-fail          Stop on first test failure
  --save-logs             Save test output to timestamped log files

=== BUILD OPTIONS ===
  -j, --jobs N            Parallel build jobs (default: auto-detect)
  --rebuild               Force complete rebuild
  --stats                 Show build and test statistics
  --profile               Profile of APP, HPC, or TEST (default)


=== EXAMPLES FOR DEBUGGING WORKFLOW ===

# 1. DISCOVER TESTS
./test-docker.sh -l | grep -i "xml"              # Find XML-related tests
./test-docker.sh -s "Profile"                    # Search for tests with "Profile"
./test-docker.sh --stats                         # Show test suite statistics

# 2. RUN SUBSETS
./test-docker.sh -f utils -v                     # Run utility tests with details
./test-docker.sh -t "*AnaProfile*" -v            # Run all AnaProfile tests
./test-docker.sh -t "US_AnaProfileXmlTest.*"     # Run entire test suite

# 3. DEBUG SPECIFIC ISSUES
./test-docker.sh -t "US_AnaProfileXmlTest.ToXmlSkipsInterferenceBChannels" -v
./test-docker.sh -g -t "US_AnaProfileXmlTest.ToXmlSkipsInterferenceBChannels"
./test-docker.sh --valgrind -t "SuspiciousTest.*"

# 4. HANDLE FLAKY TESTS
./test-docker.sh -t "FlakyTest" --repeat 10      # Run 10 times to catch intermittent issues
./test-docker.sh --failed-only -v                # Rerun only tests that failed last time
./test-docker.sh -t "TestSuite.*" --stop-on-fail # Stop at first failure for investigation

# 5. LARGE SCALE DEBUGGING
./test-docker.sh -q --save-logs                  # Run all tests, save logs, minimal output
./test-docker.sh -f utils --timeout 30           # Run with shorter timeout
./test-docker.sh -i                              # Interactive debugging session

# 6. PERFORMANCE AND STATISTICS
./test-docker.sh --stats                         # Show performance metrics
./test-docker.sh -j 8 -q                        # Use 8 cores, quick mode

=== DEBUGGING LARGE TEST SUITES ===

When you have thousands of tests, use this workflow:

1. GET OVERVIEW:
   ./test-docker.sh --stats                      # See test counts and categories

2. NARROW DOWN:
   ./test-docker.sh -s "keyword" -l              # Find tests related to your issue
   ./test-docker.sh -f category                  # Run tests by category

3. INVESTIGATE FAILURES:
   ./test-docker.sh --failed-only -v             # Focus on what's actually broken
   ./test-docker.sh -t "SpecificTest" -g         # Deep dive with debugger

4. BATCH DEBUGGING:
   ./test-docker.sh -q --save-logs               # Run all, save detailed logs
   ./test-docker.sh --repeat 5 -t "Test"    # Test for intermittent issues

=== ADVANCED CONTAINER OPTIONS ===

Interactive mode gives you full control:
  ./test-docker.sh -i

Inside container, you can:
  # Build and explore
  cd /ultrascan3/build-docker && cmake --build . -j $(nproc)

  # List and filter tests
  ctest -N                                       # All CTest tests
  ctest -N | grep -i xml                         # Filter CTest tests
  ./test/utils/test_us_utils --gtest_list_tests  # All GTest cases
  ./test/utils/test_us_utils --gtest_list_tests | grep Profile  # Filter GTest

  # Run tests with various options
  ctest -R "pattern" -V                          # CTest with pattern
  ./test/utils/test_us_utils --gtest_filter="Test.*" --gtest_print_time=1

  # Debug crashed tests
  gdb --args ./test/utils/test_us_utils --gtest_filter="CrashedTest"
  valgrind --tool=memcheck ./test/utils/test_us_utils --gtest_filter="MemoryTest"

  # Analyze test output
  ./test/utils/test_us_utils --gtest_filter="*" 2>&1 | tee test_output.log

EOF
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -f|--filter) TEST_FILTER="$2"; shift 2 ;;
        -t|--test) SPECIFIC_TEST="$2"; shift 2 ;;
        -s|--search) SEARCH_TESTS="$2"; shift 2 ;;
        -j|--jobs) PARALLEL_JOBS="$2"; shift 2 ;;
        --timeout) TIMEOUT="$2"; shift 2 ;;
        --repeat) REPEAT="$2"; shift 2 ;;
        -v|--verbose) VERBOSE=true; shift ;;
        -d|--debug) DEBUG_MODE=true; shift ;;
        -i|--interactive) INTERACTIVE=true; shift ;;
        -g|--gdb) GDB=true; shift ;;
        --valgrind) VALGRIND=true; shift ;;
        -l|--list) LIST_TESTS=true; shift ;;
        --failed-only) FAILED_ONLY=true; shift ;;
        --profile) PROFILE="$2"; shift 2 ;;
        --rebuild) REBUILD=true; shift ;;
        --stats) SHOW_STATS=true; shift ;;
        -q|--quick) QUICK_MODE=true; shift ;;
        --save-logs) SAVE_LOGS=true; shift ;;
        --stop-on-fail) STOP_ON_FIRST_FAILURE=true; shift ;;
        -h|--help) show_help; exit 0 ;;
        *) print_error "Unknown option: $1"; echo "Use -h for help"; exit 1 ;;
    esac
done

# Validation
if [ "$GDB" = true ] || [ "$VALGRIND" = true ]; then
    if [ -z "$SPECIFIC_TEST" ]; then
        print_error "GDB and Valgrind require a specific test (-t option)"
        exit 1
    fi
fi

if [ "$REPEAT" -gt 1 ] && [ -z "$SPECIFIC_TEST" ]; then
    print_error "Repeat option requires a specific test (-t option)"
    exit 1
fi

# Docker availability check
print_status "Checking Docker environment..."
if ! command -v docker &> /dev/null; then
    print_error "Docker not found. Please install Docker first."
    exit 1
fi

if ! docker info &> /dev/null; then
    print_error "Docker daemon is not running."
    exit 1
fi

# Build image
print_status "Building test image..."
# Force legacy Docker build to avoid buildx issues
export DOCKER_BUILDKIT=0

if [ "$QUICK_MODE" = true ]; then
    print_status "Quick mode: Building image quietly..."
    docker build -f admin/test/docker/Dockerfile -t us3comp-test:latest . --quiet
elif [ "$DEBUG_MODE" = true ]; then
    print_status "Debug mode: Building with full output..."
    docker build -f admin/test/docker/Dockerfile -t us3comp-test:latest .
else
    print_status "Building image (use -d for full build output)..."
    docker build -f admin/test/docker/Dockerfile -t us3comp-test:latest . --quiet
fi

print_success "Docker image built successfully"

# Check directory
if [ ! -f "CMakeLists.txt" ]; then
    print_warning "CMakeLists.txt not found. Ensure you're in the ultrascan3 root directory."
fi

# Detect repo root and set build dir consistently
ROOT_DIR="$(git rev-parse --show-toplevel 2>/dev/null || pwd)"
BUILD_DIR="${ROOT_DIR}/build-docker"

print_status "Setting up build environment..."
if [ "$REBUILD" = true ]; then
    print_status "Forcing complete rebuild..."
    rm -rf "${BUILD_DIR}"
fi

mkdir -p "${BUILD_DIR}"  # ensure parent exists for logs and CMakeFiles


# Interactive mode
if [ "$INTERACTIVE" = true ]; then
    print_highlight "Starting interactive debugging session..."
    echo ""
    echo "=== INTERACTIVE DEBUGGING COMMANDS ==="
    echo ""
    echo "BUILD:"
    echo "  cd /ultrascan3/build-docker && cmake --build . -j $PARALLEL_JOBS"
    echo ""
    echo "DISCOVER TESTS:"
    echo "  ctest -N                                    # List CTest tests"
    echo "  ctest -N | grep -i 'pattern'                # Filter CTest tests"
    echo "  ./test/utils/test_us_utils --gtest_list_tests              # List GTest cases"
    echo "  ./test/utils/test_us_utils --gtest_list_tests | grep Pattern # Filter GTest"
    echo ""
    echo "RUN TESTS:"
    echo "  ctest -R 'pattern' -V                       # Run CTest with pattern, verbose"
    echo "  ctest -L utils --output-on-failure          # Run by label"
    echo "  ./test/utils/test_us_utils --gtest_filter='Test.*' -v    # Run GTest with filter"
    echo "  ./test/utils/test_us_utils --gtest_filter='Test.*' --gtest_print_time=1"
    echo ""
    echo "DEBUG:"
    echo "  gdb --args ./test/utils/test_us_utils --gtest_filter='CrashedTest'"
    echo "  valgrind --tool=memcheck ./test/utils/test_us_utils --gtest_filter='MemoryTest'"
    echo ""
    echo "ANALYZE:"
    echo "  ./test/utils/test_us_utils 2>&1 | tee full_test_log.txt"
    echo "  grep -A5 -B5 'FAILED' full_test_log.txt    # Find failure context"
    echo ""

    docker run --rm -it \
        -v "$(pwd)":/ultrascan3 \
        -v "$(pwd)/build-docker":/ultrascan3/build-docker \
        -w /ultrascan3 \
        us3comp-test:latest bash
    exit 0
fi

# Prepare status messages
if [ "$QUICK_MODE" = false ]; then
    [ -n "$TEST_FILTER" ] && print_status "CTest filter: $TEST_FILTER"
    [ -n "$SPECIFIC_TEST" ] && print_status "GTest filter: $SPECIFIC_TEST"
    [ -n "$SEARCH_TESTS" ] && print_status "Searching tests for: $SEARCH_TESTS"
    [ "$VERBOSE" = true ] && print_status "Verbose output enabled"
    [ "$GDB" = true ] && print_status "GDB debugging enabled"
    [ "$VALGRIND" = true ] && print_status "Valgrind memory checking enabled"
    [ "$FAILED_ONLY" = true ] && print_status "Running only previously failed tests"
    [ "$REPEAT" -gt 1 ] && print_status "Repeating tests $REPEAT times"
    [ "$SAVE_LOGS" = true ] && print_status "Saving logs to timestamped files"
fi

# Create log directory if needed
if [ "$SAVE_LOGS" = true ]; then
    LOG_DIR="test-logs-$(date +%Y%m%d-%H%M%S)"
    mkdir -p "$LOG_DIR"
    print_status "Logs will be saved to: $LOG_DIR"
fi

# Create the container script as a separate function to avoid quoting issues
create_container_script() {
    cat > /tmp/container_script.sh << 'SCRIPT_EOF'
#!/bin/bash
set -e
export QT_QPA_PLATFORM=offscreen

if [ "$DEBUG_MODE" = "true" ] && [ "$QUICK_MODE" = "false" ]; then
    echo '=== System Information ==='
    gcc --version | head -1
    cmake --version | head -1
    cat /etc/os-release | grep PRETTY_NAME
    echo "Available CPU cores: $(nproc)"
    echo "Available memory: $(free -h | grep Mem | awk '{print $2}')"
    echo '==========================='
fi

cd /ultrascan3/build-docker

if [ "$QUICK_MODE" = "false" ]; then
    echo 'Configuring with CMake...'
else
    echo 'Configuring project...'
fi

if [ ! -f CMakeCache.txt ] || [ "$REBUILD" = "true" ]; then
    echo 'Running CMake configuration...'
    # Always capture the full configure output so we can see the *first* real error
    # Select a generator based on what is available in the container
    GEN_ARGS=()
    if command -v ninja >/dev/null 2>&1; then
        GEN_ARGS=(-G Ninja)
    elif command -v make >/dev/null 2>&1; then
        GEN_ARGS=(-G "Unix Makefiles")
    elif command -v gmake >/dev/null 2>&1; then
        # Some distros have gmake only
        GEN_ARGS=(-G "Unix Makefiles" -DCMAKE_MAKE_PROGRAM="$(command -v gmake)")
    else
        echo "No build tool found (ninja, make, or gmake). Install one in the container and retry." >&2
        exit 1
    fi

#    Enable testing to build static library
        cmake -S .. -B . \
            "${GEN_ARGS[@]}" \
            -DCMAKE_BUILD_TYPE=Debug \
            -DUS3_PROFILE=TEST \
            -DBUILD_TESTING=ON \
            -DCMAKE_MODULE_PATH=/ultrascan3/admin/cmake \
            | tee configure.log

    # Point directly to the first configure error if there was one
    if grep -q 'CMake Error' configure.log; then
        echo '---- First CMake Error ----'
        grep -n 'CMake Error' configure.log | head -1
        exit 1
    fi
    echo 'CMake configuration complete'
else
    echo 'Using existing CMake configuration (incremental build)'
fi

if [ "$QUICK_MODE" = "false" ]; then
    echo "Building project with $PARALLEL_JOBS parallel jobs..."
    cmake --build . -j $PARALLEL_JOBS
else
    echo 'Building project (this may take a few minutes)...'
    cmake --build . -j $PARALLEL_JOBS > build.log 2>&1
    echo 'Build complete'
fi

# Show statistics
if [ "$SHOW_STATS" = "true" ]; then
    echo ''
    echo '=== TEST SUITE STATISTICS ==='
    echo "Total CTest cases: $(ctest -N 2>/dev/null | grep -c 'Test #' || echo 'Unknown')"
    if [ -f ./test/utils/test_us_utils ]; then
        GTEST_COUNT=$(./test/utils/test_us_utils --gtest_list_tests 2>/dev/null | grep -c '\.' || echo 'Unknown')
        echo "Total GTest cases: $GTEST_COUNT"
    fi
    echo "Build directory size: $(du -sh . | cut -f1)"
    echo "Parallel jobs: $PARALLEL_JOBS"
    echo '============================='
    echo ''
fi

# List/search tests
if [ "$LIST_TESTS" = "true" ]; then
    echo ''
    echo '=== AVAILABLE TESTS ==='
    echo ''
    echo 'CTest Tests:'
    ctest -N | grep 'Test #' | head -20
    if [ $(ctest -N 2>/dev/null | grep -c 'Test #') -gt 20 ]; then
        echo "... and $(($(ctest -N 2>/dev/null | grep -c 'Test #') - 20)) more CTest cases"
    fi
    echo ''
    echo 'GTest Cases (first 20):'
    if [ -f ./test/utils/test_us_utils ]; then
        ./test/utils/test_us_utils --gtest_list_tests 2>/dev/null | head -20
        TOTAL_GTESTS=$(./test/utils/test_us_utils --gtest_list_tests 2>/dev/null | wc -l)
        if [ $TOTAL_GTESTS -gt 20 ]; then
            echo "... and $(($TOTAL_GTESTS - 20)) more GTest cases"
        fi
    fi
    echo '======================='
    exit 0
fi

if [ -n "$SEARCH_TESTS" ]; then
    echo ''
    echo '=== SEARCH RESULTS ==='
    echo "Searching for tests containing: $SEARCH_TESTS"
    echo ''
    echo 'Matching CTest cases:'
    ctest -N | grep -i "$SEARCH_TESTS" || echo 'No matching CTest cases'
    echo ''
    echo 'Matching GTest cases:'
    if [ -f ./test/utils/test_us_utils ]; then
        ./test/utils/test_us_utils --gtest_list_tests 2>/dev/null | grep -i "$SEARCH_TESTS" || echo 'No matching GTest cases'
    fi
    echo '===================='
    exit 0
fi

# GDB debugging
if [ "$GDB" = "true" ]; then
    if ! command -v gdb &> /dev/null; then
        echo 'ERROR: GDB not available in container'
        echo 'To add GDB, modify the Dockerfile to include: gdb'
        exit 1
    fi
    echo 'Starting GDB debugging session...'
    echo 'Useful GDB commands:'
    echo '  run                    # Start the test'
    echo '  bt                     # Show backtrace when crashed'
    echo '  info locals            # Show local variables'
    echo '  break function_name    # Set breakpoint'
    echo '  continue               # Continue execution'
    echo '  print variable_name    # Print variable value'
    echo '  quit                   # Exit GDB'
    echo ''
    gdb --args ./test/utils/test_us_utils --gtest_filter="$SPECIFIC_TEST"
    exit 0
fi

# Valgrind debugging
if [ "$VALGRIND" = "true" ]; then
    if ! command -v valgrind &> /dev/null; then
        echo 'ERROR: Valgrind not available in container'
        echo 'To add Valgrind, modify the Dockerfile:'
        echo 'Add: RUN apt-get update && apt-get install -y valgrind'
        exit 1
    fi
    echo 'Running memory analysis with Valgrind...'
    valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes ./test/utils/test_us_utils --gtest_filter="$SPECIFIC_TEST"
    exit 0
fi

# Run specific GTest
if [ -n "$SPECIFIC_TEST" ]; then
    echo "Running specific test: $SPECIFIC_TEST"
    if [ "$REPEAT" -gt 1 ]; then
        echo "Repeating $REPEAT times to check for flaky behavior..."
        for i in $(seq 1 $REPEAT); do
            echo "=== Run $i/$REPEAT ==="
            ./test/utils/test_us_utils --gtest_filter="$SPECIFIC_TEST" ${VERBOSE:+--gtest_print_time=1} || {
                echo "Test failed on run $i"
                exit 1
            }
        done
        echo "All $REPEAT runs passed successfully!"
    else
        ./test/utils/test_us_utils --gtest_filter="$SPECIFIC_TEST" ${VERBOSE:+--gtest_print_time=1}
    fi
    exit $?
fi

# Run tests with CTest
CTEST_ARGS="--output-on-failure"

[ "$PARALLEL_JOBS" -gt 1 ] && CTEST_ARGS="$CTEST_ARGS --parallel $PARALLEL_JOBS"
[ "$VERBOSE" = "true" ] && CTEST_ARGS="$CTEST_ARGS --verbose"
[ "$STOP_ON_FIRST_FAILURE" = "true" ] && CTEST_ARGS="$CTEST_ARGS --stop-on-failure"
[ -n "$TEST_FILTER" ] && CTEST_ARGS="$CTEST_ARGS -L $TEST_FILTER"
[ -n "$TIMEOUT" ] && CTEST_ARGS="$CTEST_ARGS --timeout $TIMEOUT"
[ "$FAILED_ONLY" = "true" ] && CTEST_ARGS="$CTEST_ARGS --rerun-failed"

if [ "$DEBUG_MODE" = "true" ]; then
    echo "Executing: ctest $CTEST_ARGS"
fi

START_TIME=$(date +%s)

if [ "$SAVE_LOGS" = "true" ]; then
    ctest $CTEST_ARGS 2>&1 | tee test_output.log
    RESULT=${PIPESTATUS[0]}
else
    ctest $CTEST_ARGS
    RESULT=$?
fi

END_TIME=$(date +%s)
DURATION=$((END_TIME - START_TIME))

if [ $RESULT -eq 0 ]; then
    echo ''
    echo '=== TEST SUMMARY ==='
    echo "All tests passed! (Duration: ${DURATION}s)"
    [ "$SHOW_STATS" = "true" ] && echo "Used $PARALLEL_JOBS parallel jobs"
else
    echo ''
    echo '=== FAILURE ANALYSIS ==='
    echo "Tests failed! (Duration: ${DURATION}s)"
    echo ''
    echo 'Next steps for debugging:'
    echo '  ./test-docker.sh --failed-only -v        # Rerun failed tests with details'
    echo '  ./test-docker.sh -s "keyword"             # Search for specific tests'
    echo '  ./test-docker.sh -t "SpecificTest" -g     # Debug with GDB'
    echo '  ./test-docker.sh -i                      # Interactive debugging'
    echo ''

    # Show failed test details
    echo 'Failed tests summary:'
    ctest --rerun-failed -N 2>/dev/null | grep 'Test #' || echo 'Unable to get failed test list'
fi

exit $RESULT
SCRIPT_EOF
    chmod +x /tmp/container_script.sh
}

# Create the container script
create_container_script

# Run the container
if [ "$SAVE_LOGS" = true ]; then
    TIMESTAMP=$(date +%Y%m%d-%H%M%S)
    docker run --rm \
        -v "$(pwd)":/ultrascan3 \
        -v "$(pwd)/build-docker":/ultrascan3/build-docker \
        -v /tmp/container_script.sh:/tmp/container_script.sh \
        -w /ultrascan3 \
        -e DEBUG_MODE="$DEBUG_MODE" \
        -e QUICK_MODE="$QUICK_MODE" \
        -e REBUILD="$REBUILD" \
        -e PARALLEL_JOBS="$PARALLEL_JOBS" \
        -e SHOW_STATS="$SHOW_STATS" \
        -e LIST_TESTS="$LIST_TESTS" \
        -e SEARCH_TESTS="$SEARCH_TESTS" \
        -e GDB="$GDB" \
        -e VALGRIND="$VALGRIND" \
        -e SPECIFIC_TEST="$SPECIFIC_TEST" \
        -e REPEAT="$REPEAT" \
        -e VERBOSE="$VERBOSE" \
        -e TEST_FILTER="$TEST_FILTER" \
        -e STOP_ON_FIRST_FAILURE="$STOP_ON_FIRST_FAILURE" \
        -e TIMEOUT="$TIMEOUT" \
        -e FAILED_ONLY="$FAILED_ONLY" \
        -e SAVE_LOGS="$SAVE_LOGS" \
        us3comp-test:latest \
        bash /tmp/container_script.sh 2>&1 | tee "test-log-$TIMESTAMP.txt"
    RESULT=${PIPESTATUS[0]}
else
    docker run --rm \
        -v "$(pwd)":/ultrascan3 \
        -v "$(pwd)/build-docker":/ultrascan3/build-docker \
        -v /tmp/container_script.sh:/tmp/container_script.sh \
        -w /ultrascan3 \
        -e DEBUG_MODE="$DEBUG_MODE" \
        -e QUICK_MODE="$QUICK_MODE" \
        -e REBUILD="$REBUILD" \
        -e PARALLEL_JOBS="$PARALLEL_JOBS" \
        -e SHOW_STATS="$SHOW_STATS" \
        -e LIST_TESTS="$LIST_TESTS" \
        -e SEARCH_TESTS="$SEARCH_TESTS" \
        -e GDB="$GDB" \
        -e VALGRIND="$VALGRIND" \
        -e SPECIFIC_TEST="$SPECIFIC_TEST" \
        -e REPEAT="$REPEAT" \
        -e VERBOSE="$VERBOSE" \
        -e TEST_FILTER="$TEST_FILTER" \
        -e STOP_ON_FIRST_FAILURE="$STOP_ON_FIRST_FAILURE" \
        -e TIMEOUT="$TIMEOUT" \
        -e FAILED_ONLY="$FAILED_ONLY" \
        -e SAVE_LOGS="$SAVE_LOGS" \
        us3comp-test:latest \
        bash /tmp/container_script.sh
    RESULT=$?
fi

# Final summary
if [ $RESULT -eq 0 ]; then
    if [ "$QUICK_MODE" = false ]; then
        echo ""
        print_success "Test execution completed successfully!"
        if [ "$SAVE_LOGS" = true ]; then
            print_status "Detailed logs saved to: test-log-$TIMESTAMP.txt"
        fi
    fi
else
    if [ "$QUICK_MODE" = false ]; then
        echo ""
        print_error "Test execution failed!"
        echo ""
        print_highlight "Quick debugging commands:"
        echo "  ./test-docker.sh --failed-only -v          # Focus on failures"
        echo "  ./test-docker.sh -s \"keyword\" -l           # Find related tests"
        echo "  ./test-docker.sh -i                        # Interactive debugging"
        if [ "$SAVE_LOGS" = true ]; then
            print_status "Detailed logs saved to: test-log-$TIMESTAMP.txt"
        fi
    fi
    exit 1
fi