#include "test_us_analyte.h"
#include "us_settings.h"
#include "us_constants.h"

void TestUSAnalyte::initTestCase()
{
    // This is called before any test is executed. You can use it to set up
    // any global state or initialize variables that are needed across tests.
}

void TestUSAnalyte::cleanupTestCase()
{
    // This is called after all tests have been executed. Use it to clean up
    // any resources that were allocated in initTestCase().
}

void TestUSAnalyte::testConstructor()
{
    US_Analyte analyte;

    QCOMPARE(analyte.vbar20, TYPICAL_VBAR);  // Ensure TYPICAL_VBAR is defined
    QCOMPARE(analyte.mw, 0.0);
    QCOMPARE(analyte.description, QString("New Analyte"));
    QCOMPARE(analyte.analyteGUID, QString());
    QCOMPARE(analyte.sequence, QString());
    QCOMPARE(analyte.type, US_Analyte::PROTEIN);
    QCOMPARE(analyte.grad_form, false);
    QCOMPARE(analyte.doubleStranded, true);
    QCOMPARE(analyte.complement, false);
    QCOMPARE(analyte._3prime, false);
    QCOMPARE(analyte._5prime, false);
    QCOMPARE(analyte.sodium, 0.0);
    QCOMPARE(analyte.potassium, 0.0);
    QCOMPARE(analyte.lithium, 0.0);
    QCOMPARE(analyte.magnesium, 0.0);
    QCOMPARE(analyte.calcium, 0.0);
    QVERIFY(analyte.extinction.isEmpty());
    QVERIFY(analyte.refraction.isEmpty());
    QVERIFY(analyte.fluorescence.isEmpty());
}

void TestUSAnalyte::testEqualityOperator()
{
    US_Analyte analyte1;
    US_Analyte analyte2;

    // Initially, the two analytes should be equal
    QVERIFY(analyte1 == analyte2);

    // Modify analyte2 and check that they are no longer equal
    analyte2.mw = 65000.0;
    QVERIFY(!(analyte1 == analyte2));

    // Revert the modification and check that they are equal again
    analyte2.mw = analyte1.mw;
    QVERIFY(analyte1 == analyte2);
}

void TestUSAnalyte::testLoad()
{
    // Test the load method with disk access (assuming the guid does not exist on disk)
    US_Analyte analyte;
    QString guid = "non-existent-guid";

    int result = analyte.load(false, guid);  // `false` indicates disk loading

    // If the file does not exist, load should return NO_ANALYTE
    QCOMPARE(result, US_DB2::NO_ANALYTE);

    // If the file does exist, you would have a valid GUID and you could check its properties after loading
    // Here, assume we have a valid GUID "example-guid" (you should replace this with a valid GUID if available)
    // result = analyte.load(false, "example-guid");
    // QCOMPARE(result, US_DB2::OK);
    // QCOMPARE(analyte.description, QString("Expected Description"));
    // QCOMPARE(analyte.sequence, QString("Expected Sequence"));
}

void TestUSAnalyte::testWrite()
{
    // Test writing an analyte to disk using the public interface
    US_Analyte analyte;
    analyte.description = "Test Analyte";
    analyte.sequence = "ATCG";
    analyte.analyteGUID = QUuid::createUuid().toString();  // Generate a unique GUID for testing

    QString path;
    analyte.analyte_path(path);
    QString filename = analyte.get_filename(path, analyte.analyteGUID);

    int result = analyte.write(false, filename);  // `false` indicates disk writing
    QCOMPARE(result, US_DB2::OK);

    // Load the file back and verify its contents
    US_Analyte loadedAnalyte;
    result = loadedAnalyte.load(false, analyte.analyteGUID);
    QCOMPARE(result, US_DB2::OK);
    QCOMPARE(loadedAnalyte.description, QString("Test Analyte"));
    QCOMPARE(loadedAnalyte.sequence, QString("ATCG"));

    // Clean up the test file
    QFile::remove(filename);
}
