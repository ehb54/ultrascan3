#include "test_us_simparms.h"
#include "us_simparms.h"
#include "us_db2.h"
#include "us_hardware.h"

// Mock classes
class MockUS_DB2 : public US_DB2 {
    // Implement necessary mock methods if required
};

class MockUS_AbstractCenterpiece {
public:
    static bool read_centerpieces(US_DB2* db, QList<US_AbstractCenterpiece>& cp_list) {
        // Add mock data
        US_AbstractCenterpiece cp;
        cp.serial_number = 1;
        cp.shape = "standard";
        cp.bottom_position = {7.2, 2.0};
        cp.path_length = {1.5, 2.5};
        cp.angle = 30.0;
        cp.width = 5.0;
        cp_list.append(cp);
        return true;
    }
};

class MockUS_Hardware {
public:
    static bool readRotorMap(US_DB2* db, QMap<QString, QString>& rotor_map) {
        // Add mock data
        rotor_map["calID"] = "mock_calibration";
        return true;
    }

    static void rotorValues(QString rCalID, QMap<QString, QString>& rotor_map, double* rotorcoeffs) {
        // Add mock data
        rotorcoeffs[0] = 1.1;
        rotorcoeffs[1] = 2.2;
    }
};

// Extending US_SimulationParameters to override the setHardware method for testing
class TestUS_SimulationParameters : public US_SimulationParameters {
public:
    void setHardwareMock(US_DB2* db, QString rCalID, int cp, int ch) {
        QList<US_AbstractCenterpiece> cp_list;
        QMap<QString, QString> rotor_map;
        MockUS_AbstractCenterpiece::read_centerpieces(db, cp_list);
        MockUS_Hardware::readRotorMap(db, rotor_map);
        MockUS_Hardware::rotorValues(rCalID, rotor_map, rotorcoeffs);

        // Copy data from the mock data to the member variables
        this->rotorCalID = rCalID;
        this->bottom_position = cp_list[cp].bottom_position[ch];
        this->cp_pathlen = cp_list[cp].path_length[ch];
        this->cp_angle = cp_list[cp].angle;
        this->cp_width = cp_list[cp].width;
        this->cp_sector = 1;  // Assuming "standard" is the second item in the shapes list
        this->band_forming = false;
        this->rotorcoeffs[0] = rotorcoeffs[0];
        this->rotorcoeffs[1] = rotorcoeffs[1];
    }

    void setHardwareMock(QString rCalID, int cp, int ch) {
        setHardwareMock(nullptr, rCalID, cp, ch);
    }
};

void TestUSSimparms::testConstructor() {
    TestUS_SimulationParameters simParams;

    QCOMPARE(simParams.mesh_radius.size(), 0);
    QCOMPARE(simParams.speed_step.size(), 1);
    QCOMPARE(simParams.simpoints, 200);
    QCOMPARE(simParams.meshType, US_SimulationParameters::ASTFEM);
    QCOMPARE(simParams.gridType, US_SimulationParameters::MOVING);
    QCOMPARE(simParams.radial_resolution, 0.001);
    QCOMPARE(simParams.meniscus, 5.8);
    QCOMPARE(simParams.bottom, 7.2);
    QCOMPARE(simParams.rnoise, 0.0);
    QCOMPARE(simParams.lrnoise, 0.0);
    QCOMPARE(simParams.tinoise, 0.0);
    QCOMPARE(simParams.rinoise, 0.0);
    QCOMPARE(simParams.temperature, 20.0);  // Assuming NORMAL_TEMP is 20.0
    QCOMPARE(simParams.rotorCalID, QString("0"));
    QCOMPARE(simParams.band_forming, false);
    QCOMPARE(simParams.band_volume, 0.015);
    QCOMPARE(simParams.bottom_position, 7.2);
    QCOMPARE(simParams.rotorcoeffs[0], 0.0);  // Default value in constructor
    QCOMPARE(simParams.rotorcoeffs[1], 0.0);  // Default value in constructor
    QCOMPARE(simParams.cp_sector, 0);
    QCOMPARE(simParams.cp_pathlen, 1.2);
    QCOMPARE(simParams.cp_angle, 2.5);
    QCOMPARE(simParams.cp_width, 0.0);
    QCOMPARE(simParams.sim, false);
    QCOMPARE(simParams.tsobj, nullptr);
    QCOMPARE(simParams.firstScanIsConcentration, false);
}

void TestUSSimparms::testSetHardware_DB() {
    TestUS_SimulationParameters simParams;
    MockUS_DB2 db;
    QString rCalID = "calID";
    int cp = 0; // Index 0 because cp_list will have only one item
    int ch = 0;

    // Use mock classes
    simParams.setHardwareMock(&db, rCalID, cp, ch);

    QCOMPARE(simParams.rotorCalID, rCalID);
    QCOMPARE(simParams.bottom_position, 7.2);
    QCOMPARE(simParams.cp_pathlen, 1.5);
    QCOMPARE(simParams.cp_angle, 30.0);
    QCOMPARE(simParams.cp_width, 5.0);
    QCOMPARE(simParams.cp_sector, 1);  // Assuming "standard" is the second item in the shapes list
    QCOMPARE(simParams.band_forming, false);
    QCOMPARE(simParams.rotorcoeffs[0], 1.1);
    QCOMPARE(simParams.rotorcoeffs[1], 2.2);
}

void TestUSSimparms::testSetHardware_Local() {
    TestUS_SimulationParameters simParams;
    QString rCalID = "calID";
    int cp = 0; // Index 0 because cp_list will have only one item
    int ch = 0;

    // Use mock classes
    simParams.setHardwareMock(rCalID, cp, ch);

    QCOMPARE(simParams.rotorCalID, rCalID);
    QCOMPARE(simParams.bottom_position, 7.2);
    QCOMPARE(simParams.cp_pathlen, 1.5);
    QCOMPARE(simParams.cp_angle, 30.0);
    QCOMPARE(simParams.cp_width, 5.0);
    QCOMPARE(simParams.cp_sector, 1);  // Assuming "standard" is the second item in the shapes list
    QCOMPARE(simParams.band_forming, false);
    QCOMPARE(simParams.rotorcoeffs[0], 1.1);
    QCOMPARE(simParams.rotorcoeffs[1], 2.2);
}
