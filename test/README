# UltraScan3 Test Framework

## Quick Start

```bash
# Run all tests
./test-docker.sh

# Run with verbose output (shows individual test results)
./test-docker.sh -v

# Run specific test categories
./test-docker.sh -f utils

# Show help and examples
./test-docker.sh -h
```

## How It Works

The test framework uses Docker to create a consistent environment for running tests. Tests are built with CMake and executed using CTest with automatic CPU detection for parallel execution.

## System Requirements

- Docker (must be installed and running)
- Run from the ultrascan3 root directory (where CMakeLists.txt exists)
- **Linux/macOS**: Run script directly
- **Windows**: Use WSL2, Git Bash, or create equivalent batch/PowerShell script

## Writing Tests

### Test Structure
Tests use Google Test with Qt5 integration through QtTestBase. Create test files in subdirectories and they'll be automatically discovered by CMake.

```cpp
// test/utils/string_utils_test.cpp
#include "qt_test_base.h"
#include "string_utils.h"

class StringUtilsTest : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        // Test-specific setup
    }
};

TEST_F(StringUtilsTest, ShouldTrimWhitespace) {
    QString input = "  hello world  ";
    QString result = StringUtils::trim(input);
    EXPECT_QSTRING_EQ(result, "hello world");
}

TEST_F(StringUtilsTest, ShouldHandleEmptyString) {
    QString input = "";
    QString result = StringUtils::trim(input);
    EXPECT_QSTRING_EQ(result, "");
}
```

### Signal Testing
Use the provided macros to test Qt signals and slots:

```cpp
TEST_F(MyWidgetTest, ShouldEmitSignalOnAction) {
    MyWidget widget;
    EXPECT_SIGNAL_EMITTED(widget, valueChanged, 1);
    widget.performAction();
}
```

### Test Organization
Place tests in subdirectories that mirror your source code structure. Each directory needs a CMakeLists.txt that adds the tests to the build system.

## Available Commands

| Command | Description |
|---------|-------------|
| `./test-docker.sh` | Run all tests with summary output |
| `./test-docker.sh -v` | Show detailed output for each test |
| `./test-docker.sh -f PATTERN` | Run only tests matching PATTERN |
| `./test-docker.sh -h` | Show help and examples |

## Test Environment

The Docker container provides:
- CMake build system with automatic parallel compilation
- CTest with parallel test execution
- Google Test and Google Mock frameworks
- Qt5 development environment
- Custom Qt testing utilities and matchers

## Build Process

1. **Docker Build**: Creates test environment (cached, only rebuilds when Dockerfile changes)
2. **CMake Configure**: Sets up build files (cached in persistent build-docker directory)
3. **Parallel Compilation**: Uses all available CPU cores automatically
4. **Parallel Test Execution**: CTest optimizes test scheduling across cores

## Performance Optimizations

- **Persistent build directory** (`build-docker`) enables incremental builds
- **Automatic CPU detection** for optimal parallel execution
- **Docker layer caching** speeds up subsequent runs
- **Incremental compilation** only rebuilds changed files

## Adding New Tests

1. Create your test file in the appropriate subdirectory
2. Include necessary headers (qt_test_base.h, your source files)
3. Use Google Test macros (TEST, TEST_F, EXPECT_*)
4. Run `./test-docker.sh` to automatically discover and run your tests

## Debugging Failed Tests

When tests fail, the framework automatically:
- Shows which tests failed in the summary
- Reruns failed tests with verbose output
- Displays detailed failure information

For targeted debugging:
```bash
./test-docker.sh -v -f specific_test_name
```

## Cross-Platform Notes

- Linux/macOS: Run script directly
- Windows: Use WSL2 for best compatibility, or Git Bash as alternative
- Docker requirement: Same across all platforms

## Example Script: Checking Which utils/ Files Are Missing Unit Tests
To identify which files in the utils directory are not covered by unit tests in test/utils, use the following one-liner:
```bash 
comm -23 \
<(find utils -type f -name '*.cpp' -exec basename {} \; | sort | uniq) \
<(find test/utils -type f -name '*.cpp' -exec basename {} \; | sed 's/^test_//' | sort | uniq)
```
This will output a list of utils files (by basename) that do not have a corresponding test file in test/utils/.

This assumes the naming convention:

Source file: utils/us_example.cpp \
Test file: test/utils/test_us_example.cpp \
You can run this from the root of the repository.