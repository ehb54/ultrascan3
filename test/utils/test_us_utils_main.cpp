#include <QCoreApplication>
#include <QtTest>
#include "test_us_util.h"
#include "test_us_ana_profile.h"
#include "test_us_datafiles.h"
#include "test_us_crypto.h"
#include "test_us_dataio.h"

// Function to dynamically load and run all test classes
void runTest(QObject *test, int &status, int argc, char *argv[])
{
    status |= QTest::qExec(test, argc, argv);
    delete test;
}

// Main function: entry point for the test application
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    // Run tests in the correct dependency order
    runTest(new TestUSUtil, status, argc, argv);
    runTest(new TestUSAnaProfile, status, argc, argv);
    runTest(new TestUSDataFiles, status, argc, argv);
    runTest(new TestUSCrypto, status, argc, argv);
    runTest(new TestUSDataIO, status, argc, argv);

    return status;
}
