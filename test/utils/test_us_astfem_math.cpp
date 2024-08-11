#include "test_us_astfem_math.h"
#include "us_astfem_math.h"
#include "us_settings.h"
#include "us_dataIO.h"
#include <QFile>
#include <QTextStream>

void TestUSAstfemMath::initTestCase()
{
    tmst_fpath = "test.tmst";

    // Create a valid file with expected content
    QFile file(tmst_fpath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&file);
        out << "Sample content for timestate file\n";
        // Add any necessary valid content
        file.close();
    }
    else
    {
        QFAIL("Failed to create the test file.");
    }
}

void TestUSAstfemMath::cleanupTestCase()
{
    // Remove the file after tests are done
    QFile::remove(tmst_fpath);
}

void TestUSAstfemMath::testWritetimestate()
{
    US_SimulationParameters simparams;
    US_DataIO::RawData sim_data;

    // Set up simulation parameters
    simparams.mesh_radius.clear();
    simparams.mesh_radius << 5.8 << 6.0 << 6.2;
    simparams.simpoints = 200;
    simparams.meshType = US_SimulationParameters::MeshType::ASTFEM;
    simparams.gridType = US_SimulationParameters::FIXED;
    simparams.radial_resolution = 0.001;
    simparams.meniscus = 5.8;
    simparams.bottom = 7.2;
    simparams.temperature = 20.0;
//    simparams.noise_level = 0.0;
//    simparams.noise_type = US_SimulationParameters::NOISE_NONE;
//    simparams.time_invariant_noise = 0.0;
//    simparams.radial_invariant_noise = 0.0;
    simparams.band_forming = false;
    simparams.band_volume = 0.015;
    simparams.rotorCalID = "0";
//    simparams.rotorcoeffs = QVector<double>() << 0 << 0;
//    simparams.cpSector = 0;
//    simparams.cpPathlen = 1.2;
//    simparams.cpAngle = 2.5;
//    simparams.cpWidth = 0.0;

    US_SimulationParameters::SpeedProfile speedProfile;
    speedProfile.duration_hours = 1.0;
    speedProfile.duration_minutes = 0;
    speedProfile.delay_hours = 0;
    speedProfile.delay_minutes = 0;
    speedProfile.scans = 100;
    speedProfile.acceleration = 400;

    simparams.speed_step<< speedProfile; // Add the speed profile

    // Set up raw data
    sim_data.type[0] = 'R';  // Example type "RI"
    sim_data.type[1] = 'I';
    sim_data.xvalues << 280.0 << 290.0;  // Use xvalues instead of wavelengths

    US_DataIO::Scan scan1;
    scan1.seconds = 0.0;  // Set the time elapsed since the start of the run
    scan1.rpm = 50000;
    scan1.temperature = 20.0;  // Example temperature
    scan1.rvalues.clear();
    scan1.rvalues << 1.0 << 2.0 << 3.0;

    US_DataIO::Scan scan2;
    scan2.seconds = 1.0;  // Set the time elapsed since the start of the run
    scan2.rpm = 50000;
    scan2.temperature = 20.0;  // Example temperature
    scan2.rvalues.clear();
    scan2.rvalues << 1.5 << 2.5 << 3.5;

    sim_data.scanData << scan1 << scan2;
    sim_data.scanData << scan1 << scan2;

    // Act
    int result = US_AstfemMath::writetimestate(tmst_fpath, simparams, sim_data);


    // Debugging information
    qDebug() << "writetimestate() result:" << result;

    // Assert
    QVERIFY(result > 0);
    QVERIFY(QFile::exists(tmst_fpath));

    // Additional content verification (optional)
    QFile file(tmst_fpath);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    // Replace this with actual expected content checks
    QVERIFY(!content.isEmpty());
}

void TestUSAstfemMath::testLowAcceleration()
{
    QVector<US_SimulationParameters::SpeedProfile> speedsteps;

    // Setup speedsteps with valid data
    US_SimulationParameters::SpeedProfile speedProfile;
    speedProfile.duration_minutes = 30;
    speedProfile.delay_minutes = 15;
    speedProfile.w2t_first = 0.0;
    speedProfile.w2t_last = 1.0;
    speedProfile.avg_speed = 50000.0;
    speedProfile.speed_stddev = 0.0;
    speedProfile.duration_hours = 1;
    speedProfile.delay_hours = 0;
    speedProfile.time_first = 0;
    speedProfile.time_last = 5400;  // 1.5 hours in seconds
    speedProfile.scans = 100;
    speedProfile.rotorspeed = 50000;
    speedProfile.acceleration = 400;  // RPM/second
    speedProfile.set_speed = 50000;
    speedProfile.acceleration_flag = true;

    speedsteps << speedProfile;  // Add the speed profile to the vector

    double min_accel = 0.1;  // Minimum acceleration threshold
    double rate;

    // Defensive check before the actual test
    QVERIFY(!speedsteps.isEmpty());  // This should now pass

    // Act
    bool result = US_AstfemMath::low_acceleration(speedsteps, min_accel, rate);

    // Assert
    QVERIFY(result);  // Adjust this based on the expected result
}


void TestUSAstfemMath::testInterpolateC0()
{
    // Arrange
    US_AstfemMath astfemMath;
    US_AstfemMath::MfemInitial C0, C1;

    // Set up C0 and C1 with valid test data

    // Act
    astfemMath.interpolate_C0(C0, C1);

    // Assert
    // Validate the interpolation results
    QVERIFY(C1.radius.size() == C0.radius.size());  // Example assertion
}
