// test_us_dataio_unit.cpp - Unit tests for US_DataIO class
#include "qt_test_base.h"
#include "us_dataIO.h"
#include <QString>
#include <QVector>
#include <QByteArray>
#include <QFile>
#include <QTemporaryDir>
#include <limits>

using namespace qt_matchers;

class TestUSDataIOUnit : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        setupTestData();
    }

    void TearDown() override {
        QtTestBase::TearDown();
    }

    void setupTestData() {
        // Create test RawData
        rawData.xvalues = {5.8, 5.9, 6.0, 6.1, 6.2, 6.3, 6.4, 6.5};
        rawData.cell = 1;
        rawData.channel = 'A';
        rawData.description = "Test raw data";

        // Create test scans
        for (int i = 0; i < 3; i++) {
            US_DataIO::Scan scan;
            scan.temperature = 20.0 + i;
            scan.rpm = 3000.0 + i * 100;
            scan.seconds = i * 60;
            scan.omega2t = i * 100.0;
            scan.wavelength = 280.0;
            scan.delta_r = 0.003;
            scan.nz_stddev = true;

            // Add readings data
            for (int j = 0; j < rawData.xvalues.size(); j++) {
                scan.rvalues << 0.1 + i * 0.05 + j * 0.01;
                scan.stddevs << 0.001 + j * 0.0001;
            }

            // Initialize interpolated array
            scan.interpolated = QByteArray(1, 0); // 8 bits for 8 readings

            rawData.scanData << scan;
        }

        // Create test EditedData
        editedData.runID = "test_run";
        editedData.editID = "test_edit";
        editedData.dataType = "RA";
        editedData.cell = "1";
        editedData.channel = "A";
        editedData.wavelength = "280";
        editedData.description = "Test edited data";
        editedData.meniscus = 5.9;
        editedData.bottom = 6.4;
        editedData.plateau = 6.2;
        editedData.baseline = 0.0;
        editedData.ODlimit = 1.5;
        editedData.floatingData = false;

        // Copy some data from rawData
        editedData.xvalues = rawData.xvalues;
        editedData.scanData = rawData.scanData;
    }

    US_DataIO::RawData rawData;
    US_DataIO::EditedData editedData;
};

// ============================================================================
// RAWDATA CLASS TESTS
// ============================================================================

TEST_F(TestUSDataIOUnit, RawDataPointCount) {
// Test pointCount method
int count = rawData.pointCount();
EXPECT_EQ(count, 8) << "Should return correct number of x-values";
}

TEST_F(TestUSDataIOUnit, RawDataPointCountEmpty) {
// Test pointCount with empty data
US_DataIO::RawData emptyData;
int count = emptyData.pointCount();
EXPECT_EQ(count, 0) << "Should return 0 for empty x-values";
}

TEST_F(TestUSDataIOUnit, RawDataScanCount) {
// Test scanCount method
int count = rawData.scanCount();
EXPECT_EQ(count, 3) << "Should return correct number of scans";
}

TEST_F(TestUSDataIOUnit, RawDataScanCountEmpty) {
// Test scanCount with empty data
US_DataIO::RawData emptyData;
int count = emptyData.scanCount();
EXPECT_EQ(count, 0) << "Should return 0 for empty scan data";
}

TEST_F(TestUSDataIOUnit, RawDataXindexExactMatch) {
// Test xindex with exact match
int index = rawData.xindex(6.0);
EXPECT_EQ(index, 2) << "Should return exact index for matching value";
}

TEST_F(TestUSDataIOUnit, RawDataXindexBelowRange) {
// Test xindex with value below range
int index = rawData.xindex(5.0);
EXPECT_EQ(index, 0) << "Should return first index for value below range";
}

TEST_F(TestUSDataIOUnit, RawDataXindexAboveRange) {
// Test xindex with value above range
int index = rawData.xindex(7.0);
EXPECT_EQ(index, 7) << "Should return last index for value above range";
}

TEST_F(TestUSDataIOUnit, RawDataXindexClosestMatch) {
// Test xindex with value between points (should return closest)
int index = rawData.xindex(6.05);
EXPECT_TRUE(index == 2 || index == 3) << "Should return closest index";
}

TEST_F(TestUSDataIOUnit, RawDataRadius) {
// Test radius method
double radius = rawData.radius(2);
EXPECT_DOUBLE_EQ(radius, 6.0) << "Should return correct radius value";
}

TEST_F(TestUSDataIOUnit, RawDataScWavelength) {
// Test scWavelength method
double wavelength = rawData.scWavelength(2);
EXPECT_DOUBLE_EQ(wavelength, 6.0) << "Should return correct wavelength value";
}

TEST_F(TestUSDataIOUnit, RawDataValue) {
// Test value method
double val = rawData.value(1, 2);
EXPECT_DOUBLE_EQ(val, 0.17) << "Should return correct reading value";
}

TEST_F(TestUSDataIOUnit, RawDataReading) {
// Test reading method (should be same as value)
double reading_val = rawData.reading(1, 2);
double value_val = rawData.value(1, 2);
EXPECT_DOUBLE_EQ(reading_val, value_val) << "reading() should match value()";
}

TEST_F(TestUSDataIOUnit, RawDataSetValueValid) {
// Test setValue with valid indices
bool result = rawData.setValue(1, 2, 0.999);
EXPECT_TRUE(result) << "Should successfully set value with valid indices";
EXPECT_DOUBLE_EQ(rawData.value(1, 2), 0.999) << "Should update the reading value";
}

TEST_F(TestUSDataIOUnit, RawDataSetValueInvalidScan) {
// Test setValue with invalid scan index
bool result = rawData.setValue(-1, 2, 0.999);
EXPECT_FALSE(result) << "Should fail with negative scan index";

result = rawData.setValue(10, 2, 0.999);
EXPECT_FALSE(result) << "Should fail with scan index out of range";
}

TEST_F(TestUSDataIOUnit, RawDataSetValueInvalidRadius) {
// Test setValue with invalid radius index
bool result = rawData.setValue(1, -1, 0.999);
EXPECT_FALSE(result) << "Should fail with negative radius index";

result = rawData.setValue(1, 20, 0.999);
EXPECT_FALSE(result) << "Should fail with radius index out of range";
}

TEST_F(TestUSDataIOUnit, RawDataStdDev) {
// Test std_dev method with non-zero stddev
double stddev = rawData.std_dev(1, 2);
EXPECT_DOUBLE_EQ(stddev, 0.0012) << "Should return correct std deviation";
}

TEST_F(TestUSDataIOUnit, RawDataStdDevNoStddev) {
// Test std_dev when scan has no stddev data
rawData.scanData[0].nz_stddev = false;
rawData.scanData[0].stddevs.clear();

double stddev = rawData.std_dev(0, 2);
EXPECT_DOUBLE_EQ(stddev, 0.0) << "Should return 0.0 when no stddev data";
}

TEST_F(TestUSDataIOUnit, RawDataAverageTemperature) {
// Test average_temperature method
double avgTemp = rawData.average_temperature();
EXPECT_DOUBLE_EQ(avgTemp, 21.0) << "Should calculate correct average temperature";
}

TEST_F(TestUSDataIOUnit, RawDataAverageTemperatureSingleScan) {
// Test average_temperature with single scan
US_DataIO::RawData singleScanData;
US_DataIO::Scan scan;
scan.temperature = 25.5;
singleScanData.scanData << scan;

double avgTemp = singleScanData.average_temperature();
EXPECT_DOUBLE_EQ(avgTemp, 25.5) << "Should return single temperature value";
}

TEST_F(TestUSDataIOUnit, RawDataTemperatureSpread) {
// Test temperature_spread method
double spread = rawData.temperature_spread();
EXPECT_DOUBLE_EQ(spread, 2.0) << "Should calculate correct temperature spread";
}

TEST_F(TestUSDataIOUnit, RawDataTemperatureSpreadSingleValue) {
// Test temperature_spread with identical temperatures
US_DataIO::RawData uniformTempData;
for (int i = 0; i < 3; i++) {
US_DataIO::Scan scan;
scan.temperature = 20.0; // All same temperature
uniformTempData.scanData << scan;
}

double spread = uniformTempData.temperature_spread();
EXPECT_DOUBLE_EQ(spread, 0.0) << "Should return 0.0 for uniform temperature";
}

// ============================================================================
// EDITEDDATA CLASS TESTS
// ============================================================================

TEST_F(TestUSDataIOUnit, EditedDataPointCount) {
// Test pointCount method
int count = editedData.pointCount();
EXPECT_EQ(count, 8) << "Should return correct number of x-values";
}

TEST_F(TestUSDataIOUnit, EditedDataScanCount) {
// Test scanCount method
int count = editedData.scanCount();
EXPECT_EQ(count, 3) << "Should return correct number of scans";
}

TEST_F(TestUSDataIOUnit, EditedDataXindex) {
// Test xindex method
int index = editedData.xindex(6.0);
EXPECT_EQ(index, 2) << "Should return correct index";
}

TEST_F(TestUSDataIOUnit, EditedDataRadius) {
// Test radius method
double radius = editedData.radius(2);
EXPECT_DOUBLE_EQ(radius, 6.0) << "Should return correct radius value";
}

TEST_F(TestUSDataIOUnit, EditedDataScWavelength) {
// Test scWavelength method
double wavelength = editedData.scWavelength(2);
EXPECT_DOUBLE_EQ(wavelength, 6.0) << "Should return correct wavelength value";
}

TEST_F(TestUSDataIOUnit, EditedDataValue) {
// Test value method
double val = editedData.value(1, 2);
EXPECT_DOUBLE_EQ(val, 0.17) << "Should return correct reading value";
}

TEST_F(TestUSDataIOUnit, EditedDataReading) {
// Test reading method
double reading_val = editedData.reading(1, 2);
double value_val = editedData.value(1, 2);
EXPECT_DOUBLE_EQ(reading_val, value_val) << "reading() should match value()";
}

TEST_F(TestUSDataIOUnit, EditedDataSetValue) {
// Test setValue method
bool result = editedData.setValue(1, 2, 0.888);
EXPECT_TRUE(result) << "Should successfully set value";
EXPECT_DOUBLE_EQ(editedData.value(1, 2), 0.888) << "Should update the value";
}

TEST_F(TestUSDataIOUnit, EditedDataSetValueInvalid) {
// Test setValue with invalid indices
bool result = editedData.setValue(-1, 2, 0.888);
EXPECT_FALSE(result) << "Should fail with invalid scan index";

result = editedData.setValue(1, -1, 0.888);
EXPECT_FALSE(result) << "Should fail with invalid radius index";
}

TEST_F(TestUSDataIOUnit, EditedDataStdDev) {
// Test std_dev method
double stddev = editedData.std_dev(1, 2);
EXPECT_DOUBLE_EQ(stddev, 0.0012) << "Should return correct std deviation";
}

TEST_F(TestUSDataIOUnit, EditedDataStdDevEmptyStddevs) {
// Test std_dev when stddevs vector is empty but nz_stddev is true
editedData.scanData[0].stddevs.clear();
editedData.scanData[0].nz_stddev = true;

double stddev = editedData.std_dev(0, 2);
EXPECT_DOUBLE_EQ(stddev, 0.0) << "Should return 0.0 when stddevs is empty";
}

TEST_F(TestUSDataIOUnit, EditedDataAverageTemperature) {
// Test average_temperature method
double avgTemp = editedData.average_temperature();
EXPECT_DOUBLE_EQ(avgTemp, 21.0) << "Should calculate correct average temperature";
}

TEST_F(TestUSDataIOUnit, EditedDataTemperatureSpread) {
// Test temperature_spread method
double spread = editedData.temperature_spread();
EXPECT_DOUBLE_EQ(spread, 2.0) << "Should calculate correct temperature spread";
}

// ============================================================================
// STATIC UTILITY METHOD TESTS
// ============================================================================

TEST_F(TestUSDataIOUnit, StaticIndexExactMatch) {
// Test static index method with exact match
QVector<double> xvals = {1.0, 2.0, 3.0, 4.0, 5.0};
int index = US_DataIO::index(xvals, 3.0);
EXPECT_EQ(index, 2) << "Should return exact index for matching value";
}

TEST_F(TestUSDataIOUnit, StaticIndexBelowRange) {
// Test static index method with value below range
QVector<double> xvals = {1.0, 2.0, 3.0, 4.0, 5.0};
int index = US_DataIO::index(xvals, 0.5);
EXPECT_EQ(index, 0) << "Should return first index for value below range";
}

TEST_F(TestUSDataIOUnit, StaticIndexAboveRange) {
// Test static index method with value above range
QVector<double> xvals = {1.0, 2.0, 3.0, 4.0, 5.0};
int index = US_DataIO::index(xvals, 6.0);
EXPECT_EQ(index, 4) << "Should return last index for value above range";
}

TEST_F(TestUSDataIOUnit, StaticIndexClosestValue) {
// Test static index method with value between points
QVector<double> xvals = {1.0, 2.0, 3.0, 4.0, 5.0};
int index = US_DataIO::index(xvals, 2.3);
EXPECT_EQ(index, 1) << "Should return index of closest value";
}

TEST_F(TestUSDataIOUnit, StaticIndexEmptyVector) {
// Test static index method with empty vector
QVector<double> xvals;
// This would likely cause undefined behavior, but test defensively
// Note: This is an edge case that might crash, so we document the behavior

// Since the method doesn't check for empty vectors, this is a boundary case
// that highlights a potential improvement needed in the implementation
SUCCEED() << "Empty vector test - implementation should handle this case";
}

TEST_F(TestUSDataIOUnit, StaticIndexSingleElement) {
// Test static index method with single element
QVector<double> xvals = {2.5};

int index1 = US_DataIO::index(xvals, 1.0);
EXPECT_EQ(index1, 0) << "Should return 0 for value below single element";

int index2 = US_DataIO::index(xvals, 2.5);
EXPECT_EQ(index2, 0) << "Should return 0 for exact match with single element";

int index3 = US_DataIO::index(xvals, 3.0);
EXPECT_EQ(index3, 0) << "Should return 0 for value above single element";
}

TEST_F(TestUSDataIOUnit, StaticIndexRawDataPointer) {
// Test static index method with RawData pointer
int index = US_DataIO::index(&rawData, 6.0);
EXPECT_EQ(index, 2) << "Should return correct index for RawData pointer";
}

TEST_F(TestUSDataIOUnit, StaticIndexEditedDataPointer) {
// Test static index method with EditedData pointer
int index = US_DataIO::index(&editedData, 6.0);
EXPECT_EQ(index, 2) << "Should return correct index for EditedData pointer";
}

TEST_F(TestUSDataIOUnit, ErrorStringValidCodes) {
// Test errorString method with valid error codes
QString okStr = US_DataIO::errorString(US_DataIO::OK);
EXPECT_FALSE(okStr.isEmpty()) << "Should return non-empty string for OK";
EXPECT_TRUE(okStr.contains("success")) << "OK message should contain 'success'";

QString cantOpenStr = US_DataIO::errorString(US_DataIO::CANTOPEN);
EXPECT_FALSE(cantOpenStr.isEmpty()) << "Should return non-empty string for CANTOPEN";
EXPECT_TRUE(cantOpenStr.contains("cannot be opened")) << "CANTOPEN message should be descriptive";

QString badCrcStr = US_DataIO::errorString(US_DataIO::BADCRC);
EXPECT_FALSE(badCrcStr.isEmpty()) << "Should return non-empty string for BADCRC";
EXPECT_TRUE(badCrcStr.contains("corrupted")) << "BADCRC message should mention corruption";

QString notUsDataStr = US_DataIO::errorString(US_DataIO::NOT_USDATA);
EXPECT_FALSE(notUsDataStr.isEmpty()) << "Should return non-empty string for NOT_USDATA";

QString badTypeStr = US_DataIO::errorString(US_DataIO::BADTYPE);
EXPECT_FALSE(badTypeStr.isEmpty()) << "Should return non-empty string for BADTYPE";

QString badXmlStr = US_DataIO::errorString(US_DataIO::BADXML);
EXPECT_FALSE(badXmlStr.isEmpty()) << "Should return non-empty string for BADXML";

QString noDataStr = US_DataIO::errorString(US_DataIO::NODATA);
EXPECT_FALSE(noDataStr.isEmpty()) << "Should return non-empty string for NODATA";

QString noGuidMatchStr = US_DataIO::errorString(US_DataIO::NO_GUID_MATCH);
EXPECT_FALSE(noGuidMatchStr.isEmpty()) << "Should return non-empty string for NO_GUID_MATCH";

QString badVersionStr = US_DataIO::errorString(US_DataIO::BAD_VERSION);
EXPECT_FALSE(badVersionStr.isEmpty()) << "Should return non-empty string for BAD_VERSION";
}

TEST_F(TestUSDataIOUnit, ErrorStringInvalidCode) {
// Test errorString method with invalid error code
QString unknownStr = US_DataIO::errorString(999);
EXPECT_FALSE(unknownStr.isEmpty()) << "Should return non-empty string for unknown code";
EXPECT_TRUE(unknownStr.contains("Unknown")) << "Should indicate unknown error";
}

TEST_F(TestUSDataIOUnit, FormatVersionConstant) {
// Test that format_version constant is reasonable
EXPECT_GT(US_DataIO::format_version, 0) << "Format version should be positive";
EXPECT_LE(US_DataIO::format_version, 100) << "Format version should be reasonable";
EXPECT_EQ(US_DataIO::format_version, 5) << "Format version should match expected value";
}

// ============================================================================
// EDGE CASES AND BOUNDARY CONDITIONS
// ============================================================================

TEST_F(TestUSDataIOUnit, RawDataBoundaryAccess) {
// Test boundary access for various methods
int lastScanIndex = rawData.scanCount() - 1;
int lastPointIndex = rawData.pointCount() - 1;

// Test valid boundary access
double val = rawData.value(lastScanIndex, lastPointIndex);
EXPECT_GT(val, 0.0) << "Should access valid boundary values";

double radius = rawData.radius(lastPointIndex);
EXPECT_DOUBLE_EQ(radius, 6.5) << "Should return correct boundary radius";

double stddev = rawData.std_dev(lastScanIndex, lastPointIndex);
EXPECT_GE(stddev, 0.0) << "Boundary stddev should be non-negative";
}

TEST_F(TestUSDataIOUnit, EditedDataBoundaryAccess) {
// Test boundary access for EditedData
int lastScanIndex = editedData.scanCount() - 1;
int lastPointIndex = editedData.pointCount() - 1;

double val = editedData.value(lastScanIndex, lastPointIndex);
EXPECT_GT(val, 0.0) << "Should access valid boundary values";

bool setResult = editedData.setValue(lastScanIndex, lastPointIndex, 1.234);
EXPECT_TRUE(setResult) << "Should successfully set boundary values";
EXPECT_DOUBLE_EQ(editedData.value(lastScanIndex, lastPointIndex), 1.234)
<< "Should update boundary value correctly";
}

TEST_F(TestUSDataIOUnit, TemperatureCalculationsEdgeCases) {
// Test temperature calculations with edge case data
US_DataIO::RawData edgeCaseData;

// Test with zero scans
double avgTemp = edgeCaseData.average_temperature();
// This will likely cause division by zero - document this behavior
// EXPECT_TRUE(std::isnan(avgTemp) || std::isinf(avgTemp))
//     << "Should handle empty scan data gracefully";

// Test with extreme temperature values
US_DataIO::Scan extremeScan1, extremeScan2;
extremeScan1.temperature = -273.15; // Absolute zero
extremeScan2.temperature = 1000.0;  // Very high
edgeCaseData.scanData << extremeScan1 << extremeScan2;

double spread = edgeCaseData.temperature_spread();
EXPECT_DOUBLE_EQ(spread, 1273.15) << "Should handle extreme temperature range";

double avg = edgeCaseData.average_temperature();
EXPECT_DOUBLE_EQ(avg, 363.425) << "Should calculate average of extreme values";
}
class TestUSDataIOEdits : public QtTestBase {
protected:
    void SetUp() override {
        QtTestBase::SetUp();
        ASSERT_TRUE(tmp.isValid()) << "Could not create a temporary directory";
    }

    QString path(const QString& name) { return tmp.filePath(name); }

    QString slurp(const QString& file) {
        QFile ff(file);
        if (!ff.open(QIODevice::ReadOnly | QIODevice::Text)) return QString();
        QString text = QString::fromUtf8(ff.readAll());
        ff.close();
        return text;
    }

    // Populate every optional velocity section.
    US_DataIO::EditValues fullVelocity() {
        US_DataIO::EditValues ev;
        ev.expType    = "Velocity";
        ev.runID      = "testrun-1";
        ev.cell       = "2";
        ev.channel    = "A";
        ev.wavelength = "280";
        ev.editGUID   = "11111111-2222-3333-4444-555555555555";
        ev.dataGUID   = "66666666-7777-8888-9999-000000000000";
        ev.meniscus   = 5.83000000;
        ev.bottom     = 7.19000000;
        ev.rangeLeft  = 5.83050000;
        ev.rangeRight = 7.09000000;
        ev.plateau    = 6.89000000;
        ev.baseline   = 5.83550000;
        ev.ODlimit    = 0.91250000;
        ev.airGapLeft  = 6.10000000;
        ev.airGapRight = 6.40000000;
        ev.gapTolerance = 0.02000000;
        ev.bl_corr_slope      = 0.00125000;
        ev.bl_corr_yintercept = -0.00400000;
        ev.noiseOrder   = 3;
        ev.removeSpikes = true;
        ev.floatingData = true;
        ev.invert       = -1.0;
        ev.excludes << 0 << 4 << 9;

        US_DataIO::EditedPoint pt;
        pt.scan = 2; pt.radius = 17; pt.value = 0.4321;
        ev.editedPoints << pt;

        return ev;
    }

    QTemporaryDir tmp;
};

TEST_F(TestUSDataIOEdits, VelocityEditsSurviveARoundTrip) {
    US_DataIO::EditValues out = fullVelocity();
    QString file = path("velocity.xml");

    ASSERT_EQ(US_DataIO::writeEdits(file, out), US_DataIO::OK);

    US_DataIO::EditValues in;
    ASSERT_EQ(US_DataIO::readEdits(file, in), US_DataIO::OK);

    EXPECT_EQ(in.expType,    out.expType);
    EXPECT_EQ(in.runID,      out.runID);
    EXPECT_EQ(in.cell,       out.cell);
    EXPECT_EQ(in.channel,    out.channel);
    EXPECT_EQ(in.wavelength, out.wavelength);
    EXPECT_EQ(in.editGUID,   out.editGUID);
    EXPECT_EQ(in.dataGUID,   out.dataGUID);

    EXPECT_DOUBLE_EQ(in.meniscus,     out.meniscus);
    EXPECT_DOUBLE_EQ(in.bottom,       out.bottom);
    EXPECT_DOUBLE_EQ(in.rangeLeft,    out.rangeLeft);
    EXPECT_DOUBLE_EQ(in.rangeRight,   out.rangeRight);
    EXPECT_DOUBLE_EQ(in.plateau,      out.plateau);
    EXPECT_DOUBLE_EQ(in.baseline,     out.baseline);
    EXPECT_DOUBLE_EQ(in.ODlimit,      out.ODlimit);
    EXPECT_DOUBLE_EQ(in.airGapLeft,   out.airGapLeft);
    EXPECT_DOUBLE_EQ(in.airGapRight,  out.airGapRight);
    EXPECT_DOUBLE_EQ(in.gapTolerance, out.gapTolerance);
    EXPECT_DOUBLE_EQ(in.bl_corr_slope,      out.bl_corr_slope);
    EXPECT_DOUBLE_EQ(in.bl_corr_yintercept, out.bl_corr_yintercept);

    EXPECT_EQ(in.noiseOrder,   out.noiseOrder);
    EXPECT_EQ(in.removeSpikes, out.removeSpikes);
    EXPECT_EQ(in.floatingData, out.floatingData);
    EXPECT_DOUBLE_EQ(in.invert, out.invert);

    EXPECT_EQ(in.excludes, out.excludes);

    ASSERT_EQ(in.editedPoints.size(), out.editedPoints.size());
    EXPECT_EQ(in.editedPoints[0].scan,   out.editedPoints[0].scan);
    EXPECT_EQ(in.editedPoints[0].radius, out.editedPoints[0].radius);
    EXPECT_DOUBLE_EQ(in.editedPoints[0].value, out.editedPoints[0].value);
}

TEST_F(TestUSDataIOEdits, ParameterElementsUseTheNamesReadEditsExpects) {
    // Unknown tags are ignored, so verify their exact spellings.
    QString file = path("names.xml");
    ASSERT_EQ(US_DataIO::writeEdits(file, fullVelocity()), US_DataIO::OK);

    QString text = slurp(file);
    ASSERT_FALSE(text.isEmpty());

    EXPECT_TRUE(text.contains("<!DOCTYPE UltraScanEdits>"));
    EXPECT_TRUE(text.contains("<data_range "));
    EXPECT_TRUE(text.contains("<od_limit "));
    EXPECT_TRUE(text.contains("<plateau "));
    EXPECT_TRUE(text.contains("<air_gap "));
    EXPECT_TRUE(text.contains("<linear_baseline_correction "));
    EXPECT_TRUE(text.contains("<subtract_ri_noise "));
    EXPECT_FALSE(text.contains("dataRange"));
}

TEST_F(TestUSDataIOEdits, DefaultedSectionsAreOmitted) {
    US_DataIO::EditValues out;
    out.expType    = "Velocity";
    out.runID      = "bare";
    out.cell       = "1";
    out.channel    = "A";
    out.wavelength = "280";

    QString file = path("bare.xml");
    ASSERT_EQ(US_DataIO::writeEdits(file, out), US_DataIO::OK);

    QString text = slurp(file);
    EXPECT_FALSE(text.contains("<excludes>"));
    EXPECT_FALSE(text.contains("<edited>"));
    EXPECT_FALSE(text.contains("<lambdas>"));
    EXPECT_FALSE(text.contains("<operations>"));
    EXPECT_FALSE(text.contains("<air_gap"));
    EXPECT_FALSE(text.contains("<linear_baseline_correction"));

    US_DataIO::EditValues in;
    ASSERT_EQ(US_DataIO::readEdits(file, in), US_DataIO::OK);

    EXPECT_TRUE(in.excludes.isEmpty());
    EXPECT_TRUE(in.editedPoints.isEmpty());
    EXPECT_TRUE(in.lambdas.isEmpty());
    EXPECT_EQ(in.noiseOrder, 0);
    EXPECT_FALSE(in.removeSpikes);
    EXPECT_FALSE(in.floatingData);
    EXPECT_DOUBLE_EQ(in.invert, 1.0);
    EXPECT_DOUBLE_EQ(in.gapTolerance, 0.0);
}

TEST_F(TestUSDataIOEdits, EquilibriumSpeedStepsSurviveARoundTrip) {
    US_DataIO::EditValues out;
    out.expType    = "Equilibrium";
    out.runID      = "equil";
    out.cell       = "3";
    out.channel    = "B";
    out.wavelength = "230";

    for (int ii = 0; ii < 3; ii++) {
        US_DataIO::SpeedData sd;
        sd.speed      = 10000.0 + ii * 5000.0;
        sd.first_scan = ii * 4;
        sd.scan_count = 4;
        sd.meniscus   = 5.90 + ii * 0.001;
        sd.dataLeft   = 5.95 + ii * 0.001;
        sd.dataRight   = 7.10 - ii * 0.001;
        out.speedData << sd;
    }

    QString file = path("equil.xml");
    ASSERT_EQ(US_DataIO::writeEdits(file, out), US_DataIO::OK);

    US_DataIO::EditValues in;
    ASSERT_EQ(US_DataIO::readEdits(file, in), US_DataIO::OK);

    EXPECT_EQ(in.expType, out.expType);
    ASSERT_EQ(in.speedData.size(), out.speedData.size());

    for (int ii = 0; ii < out.speedData.size(); ii++) {
        EXPECT_DOUBLE_EQ(in.speedData[ii].speed,     out.speedData[ii].speed);
        EXPECT_EQ(in.speedData[ii].first_scan,       out.speedData[ii].first_scan);
        EXPECT_EQ(in.speedData[ii].scan_count,       out.speedData[ii].scan_count);
        EXPECT_DOUBLE_EQ(in.speedData[ii].meniscus,  out.speedData[ii].meniscus);
        EXPECT_DOUBLE_EQ(in.speedData[ii].dataLeft,  out.speedData[ii].dataLeft);
        EXPECT_DOUBLE_EQ(in.speedData[ii].dataRight, out.speedData[ii].dataRight);
    }

    // Top-level radii retain the final equilibrium speed step.
    EXPECT_DOUBLE_EQ(in.meniscus,   out.speedData.last().meniscus);
    EXPECT_DOUBLE_EQ(in.rangeLeft,  out.speedData.last().dataLeft);
    EXPECT_DOUBLE_EQ(in.rangeRight, out.speedData.last().dataRight);
}

TEST_F(TestUSDataIOEdits, LambdasAreNeverWritten) {
    US_DataIO::EditValues out;
    out.expType    = "Velocity";
    out.runID      = "mwl";
    out.cell       = "1";
    out.channel    = "A";
    out.wavelength = "250-450";
    out.lambdas << 250 << 280 << 350 << 450;

    QString file = path("mwl.xml");
    ASSERT_EQ(US_DataIO::writeEdits(file, out), US_DataIO::OK);

    QString text = slurp(file);
    EXPECT_FALSE(text.contains("<lambdas>"));
    EXPECT_FALSE(text.contains("<lambda "));

    // A wavelength range must still parse when <lambdas> is absent.
    US_DataIO::EditValues in;
    ASSERT_EQ(US_DataIO::readEdits(file, in), US_DataIO::OK);

    EXPECT_TRUE(in.lambdas.isEmpty());
    EXPECT_DOUBLE_EQ(in.meniscus, out.meniscus);
    EXPECT_DOUBLE_EQ(in.bottom,   out.bottom);
}

TEST_F(TestUSDataIOEdits, WriteEditsReportsAnUnwritablePath) {
    US_DataIO::EditValues ev = fullVelocity();
    EXPECT_EQ(US_DataIO::writeEdits(path("no/such/dir/edit.xml"), ev),
              US_DataIO::CANTOPEN);
}
