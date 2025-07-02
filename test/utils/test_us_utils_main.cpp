#include <QCoreApplication>
#include <QtTest>
#include <gtest/gtest.h>

// Include all test headers
#include "test_us_util.h"
#include "test_us_ana_profile.h"
#include "test_us_analyte.h"
#include "test_us_astfem_math.h"
#include "test_us_datafiles.h"
#include "test_us_crc.h"
#include "test_us_crypto.h"
#include "test_us_dataIO.h"
#include "test_us_db2.h"
#include "test_us_math2.h"
#include "test_us_matrix.h"
#include "test_us_memory.h"
#include "test_us_simparms.h"
#include "test_us_settings.h"
#include "test_us_time_state.h"

// Function to run Qt Test classes
void runQTest(QObject *test, int &status, int argc, char *argv[])
{
    if (test) {
        status |= QTest::qExec(test, argc, argv);
        delete test;
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    int status = 0;

    // Run Qt Test classes (only ones that exist)
    runQTest(new TestUSUtil, status, argc, argv);
    runQTest(new TestUSAnaProfile, status, argc, argv);
    runQTest(new TestUSAnalyte, status, argc, argv);
    runQTest(new TestUSAstfemMath, status, argc, argv);
    runQTest(new TestUSDataFiles, status, argc, argv);  // Only run once
    runQTest(new TestUSCrc, status, argc, argv);
    runQTest(new TestUSCrypto, status, argc, argv);
    runQTest(new TestUSDataIO, status, argc, argv);
    runQTest(new TestUSDB2, status, argc, argv);
    runQTest(new TestUSMath2, status, argc, argv);
    runQTest(new TestUSMatrix, status, argc, argv);
    runQTest(new TestUSMemory, status, argc, argv);
    runQTest(new TestUSSettings, status, argc, argv);
    runQTest(new TestUSTimeState, status, argc, argv);
    runQTest(new TestUSSimparms, status, argc, argv);

    // Run GoogleTest-based tests
    ::testing::InitGoogleTest(&argc, argv);
    status |= RUN_ALL_TESTS();

    return status;
}