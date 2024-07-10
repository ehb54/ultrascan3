// Include the necessary Qt headers for a Qt application and testing framework
#include <QCoreApplication>
#include <QtTest>

// Include the headers for the test classes
#include "test_us_util.h"
#include "test_us_datafiles.h"

// Main function: entry point for the test application
int main(int argc, char *argv[])
{
    // Create a QCoreApplication object to manage application-wide resources
    QCoreApplication app(argc, argv);

    // Variable to keep track of the overall status of all test executions
    int status = 0;

    // Run the tests for the TestUSUtil class
    {
        // Create an instance of the TestUSUtil class
        TestUSUtil testUSUtil;

        // Execute the tests in the TestUSUtil class
        // qExec returns the status of the test execution
        // Combine the status with the overall status using the bitwise OR operator
        status |= QTest::qExec(&testUSUtil, argc, argv);
    }

    // Run the tests for the TestUSDataFiles class
    {
        // Create an instance of the TestUSDataFiles class
        TestUSDataFiles testUSDatafiles;

        // Execute the tests in the TestUSDataFiles class
        // qExec returns the status of the test execution
        // Combine the status with the overall status using the bitwise OR operator
        status |= QTest::qExec(&testUSDatafiles, argc, argv);
    }

    // Return the overall status of all test executions
    // A non-zero value indicates that one or more tests failed
    return status;
}
