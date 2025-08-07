#include "test_us_simparams.h"
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>
#include <QXmlStreamWriter>


// Test case for a missing file (e.g., file does not exist)
void TestUS_SimulationParameters::test_load_simparams_missing_file() {
   US_SimulationParameters simParams;
   QString filename = "sp_nonexistent_file.xml";
   QVERIFY2(!QFileInfo(filename).exists(), qPrintable("File " + filename + " already exists"));
   int status = simParams.load_simparms("sp_nonexistent_file.xml");
   QCOMPARE(status, -1);
}

// Test case for an empty file
void TestUS_SimulationParameters::test_load_simparams_empty_file() {
   QTemporaryFile tmpFile;
   QVERIFY(tmpFile.open());
   US_SimulationParameters simParams;
   int status = simParams.load_simparms(tmpFile.fileName());
   QCOMPARE(status, -1);
}

// Test case for US_SimulationParameters::speedstepToXml
void TestUS_SimulationParameters::test_speedstepToXml_without_set_speed() {
   // Create a SpeedProfile instance with set_speed == 0 (will not write extra attributes)
   US_SimulationParameters::SpeedProfile spi;
   spi.rotorspeed = 1000;
   spi.scans = 10;
   spi.time_first = 0.1;
   spi.time_last = 1.5;
   spi.w2t_first = 0.2;
   spi.w2t_last = 1.3;
   spi.duration_hours = 1;
   spi.duration_minutes = 30;
   spi.delay_hours = 0;
   spi.delay_minutes = 5;
   spi.acceleration = 2.5;
   spi.acceleration_flag = true;
   spi.set_speed = 0; // Extra attributes will not be written
   spi.avg_speed = 0;
   spi.speed_stddev = 0;

   // Write XML into a buffer
   QByteArray buffer;
   QBuffer device(&buffer);
   QVERIFY(device.open(QIODevice::WriteOnly));
   QXmlStreamWriter xmlo(&device);
   xmlo.setAutoFormatting(true);

   US_SimulationParameters::speedstepToXml(xmlo, &spi);

   device.seek(0);

   // Read the XML back to verify its content
   QXmlStreamReader xml(buffer);
   QVERIFY2(!xml.hasError(), "XML stream error");

   // Advance to the first start element
   while (!xml.atEnd() && !xml.isStartElement()) {
      xml.readNext();
   }
   QCOMPARE(xml.name().toString(), QString("speedstep"));
   auto attributes = xml.attributes();

   QCOMPARE(attributes.value("rotorspeed").toString(), QString::number(spi.rotorspeed));
   QCOMPARE(attributes.value("scans").toString(), QString::number(spi.scans));
   QCOMPARE(attributes.value("timefirst").toString(), QString::number(spi.time_first));
   QCOMPARE(attributes.value("timelast").toString(), QString::number(spi.time_last));
   QCOMPARE(attributes.value("w2tfirst").toString(), QString::number(spi.w2t_first));
   QCOMPARE(attributes.value("w2tlast").toString(), QString::number(spi.w2t_last));
   QCOMPARE(attributes.value("duration_hrs").toString(), QString::number(spi.duration_hours));
   QCOMPARE(attributes.value("duration_mins").toString(), QString::number(spi.duration_minutes));
   QCOMPARE(attributes.value("delay_hrs").toString(), QString::number(spi.delay_hours));
   QCOMPARE(attributes.value("delay_mins").toString(), QString::number(spi.delay_minutes));
   QCOMPARE(attributes.value("acceleration").toString(), QString::number(spi.acceleration));
   // acceleration_flag is written as "1" for true
   QCOMPARE(attributes.value("accelerflag").toString(), QString(spi.acceleration_flag ? "1" : "0"));

   // These attributes should not be present because set_speed is not > 0
   QVERIFY(!attributes.hasAttribute("set_speed"));
   QVERIFY(!attributes.hasAttribute("avg_speed"));
   QVERIFY(!attributes.hasAttribute("speed_stddev"));
}

void TestUS_SimulationParameters::test_speedstepToXml_with_set_speed() {
   // Create a SpeedProfile instance with set_speed > 0 (will write the extra attributes)
   US_SimulationParameters::SpeedProfile spi;
   spi.rotorspeed = 1500;
   spi.scans = 20;
   spi.time_first = 0.2;
   spi.time_last = 2.5;
   spi.w2t_first = 0.4;
   spi.w2t_last = 2.0;
   spi.duration_hours = 2;
   spi.duration_minutes = 45;
   spi.delay_hours = 1;
   spi.delay_minutes = 10;
   spi.acceleration = 3.5;
   spi.acceleration_flag = false;
   spi.set_speed = 500; // Extra attributes will be written
   spi.avg_speed = 550;
   spi.speed_stddev = 25;

   QByteArray buffer;
   QBuffer device(&buffer);
   QVERIFY(device.open(QIODevice::WriteOnly));
   QXmlStreamWriter xmlo(&device);
   xmlo.setAutoFormatting(true);

   US_SimulationParameters::speedstepToXml(xmlo, &spi);

   device.seek(0);

   // Parse the XML written to verify attributes
   QXmlStreamReader xml(buffer);
   QVERIFY2(!xml.hasError(), "XML stream error");

   while (!xml.atEnd() && !xml.isStartElement()) {
      xml.readNext();
   }
   QCOMPARE(xml.name().toString(), QString("speedstep"));
   auto attributes = xml.attributes();
   QCOMPARE(attributes.value("rotorspeed").toString(), QString::number(spi.rotorspeed));
   QCOMPARE(attributes.value("scans").toString(), QString::number(spi.scans));
   QCOMPARE(attributes.value("timefirst").toString(), QString::number(spi.time_first));
   QCOMPARE(attributes.value("timelast").toString(), QString::number(spi.time_last));
   QCOMPARE(attributes.value("w2tfirst").toString(), QString::number(spi.w2t_first));
   QCOMPARE(attributes.value("w2tlast").toString(), QString::number(spi.w2t_last));
   QCOMPARE(attributes.value("duration_hrs").toString(), QString::number(spi.duration_hours));
   QCOMPARE(attributes.value("duration_mins").toString(), QString::number(spi.duration_minutes));
   QCOMPARE(attributes.value("delay_hrs").toString(), QString::number(spi.delay_hours));
   QCOMPARE(attributes.value("delay_mins").toString(), QString::number(spi.delay_minutes));
   QCOMPARE(attributes.value("acceleration").toString(), QString::number(spi.acceleration));
   // acceleration_flag is "0" because it is false
   QCOMPARE(attributes.value("accelerflag").toString(), QString(spi.acceleration_flag ? "1" : "0"));

   // Verify the extra attributes are present and correctly set.
   QCOMPARE(attributes.value("set_speed").toString(), QString::number(spi.set_speed));
   QCOMPARE(attributes.value("avg_speed").toString(), QString::number(spi.avg_speed));
   QCOMPARE(attributes.value("speed_stddev").toString(), QString::number(spi.speed_stddev));
}

// Test that a basic set of parameters are written correctly.
void TestUS_SimulationParameters::test_save_simparms_default() {
   // Use a temporary file to ensure clean-up.
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString fileName = tempFile.fileName();
   tempFile.close(); // save_simparms will reopen this file

   US_SimulationParameters simParams;
   // Set required simulation parameter fields.
   simParams.meshType = US_SimulationParameters::ASTFEM; // Assuming enum starts at 0
   simParams.gridType = US_SimulationParameters::FIXED; // Assuming enum
   simParams.simpoints = 100;
   simParams.radial_resolution = 0.5;
   simParams.meniscus = 1.0;
   simParams.bottom = 2.0;
   simParams.rnoise = 0.01;
   simParams.lrnoise = 0.02;
   simParams.tinoise = 0.03;
   simParams.rinoise = 0.04;
   simParams.temperature = 20.0;
   simParams.band_forming = false;
   simParams.cp_sector = 1;
   simParams.cp_pathlen = 5.0;
   simParams.cp_angle = 30.0;
   simParams.cp_width = 2.0;

   // Save simulation parameters to file.
   int rv = simParams.save_simparms(fileName);
   QCOMPARE(rv, 0);

   // Read back the file and verify key XML attributes.
   QFile inFile(fileName);
   QVERIFY(inFile.open(QIODevice::ReadOnly | QIODevice::Text));

   QXmlStreamReader xml(&inFile);
   bool foundParamsElement = false;
   while (!xml.atEnd()) {
      xml.readNext();
      if (xml.isStartElement() && xml.name() == "params") {
         QXmlStreamAttributes attr = xml.attributes();

         // Verify mesh and grid types (first entries in the respective arrays).
         QCOMPARE(attr.value("meshType").toString(), QString("ASTFEM"));
         QCOMPARE(attr.value("gridType").toString(), QString("Fixed"));
         QCOMPARE(attr.value("simpoints").toString(), QString::number(simParams.simpoints));
         QCOMPARE(attr.value("radialres").toString(), QString::number(simParams.radial_resolution));
         QCOMPARE(attr.value("meniscus").toString(), QString::number(simParams.meniscus));
         QCOMPARE(attr.value("bottom").toString(), QString::number(simParams.bottom));
         QCOMPARE(attr.value("rnoise").toString(), QString::number(simParams.rnoise));
         QCOMPARE(attr.value("lrnoise").toString(), QString::number(simParams.lrnoise));
         QCOMPARE(attr.value("tinoise").toString(), QString::number(simParams.tinoise));
         QCOMPARE(attr.value("rinoise").toString(), QString::number(simParams.rinoise));
         QCOMPARE(attr.value("temperature").toString(), QString::number(simParams.temperature));
         QCOMPARE(attr.value("bandform").toString(), QString(simParams.band_forming ? "1" : "0"));
         QCOMPARE(attr.value("sector").toString(), QString::number(simParams.cp_sector));
         QCOMPARE(attr.value("pathlength").toString(), QString::number(simParams.cp_pathlen));
         QCOMPARE(attr.value("angle").toString(), QString::number(simParams.cp_angle));
         QCOMPARE(attr.value("width").toString(), QString::number(simParams.cp_width));


         foundParamsElement = true;
         break;
      }
   }
   QVERIFY(foundParamsElement);
   inFile.close();
}

// Test that when meshType is USER, usermesh elements are written.
void TestUS_SimulationParameters::test_save_simparms_user_mesh() {
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString fileName = tempFile.fileName();

   US_SimulationParameters simParams;
   // Set parameters with mesh type USER.
   simParams.meshType = US_SimulationParameters::USER;
   simParams.gridType = US_SimulationParameters::FIXED;
   simParams.simpoints = 50;
   simParams.radial_resolution = 1.0;
   simParams.meniscus = 0.0;
   simParams.bottom = 0.0;
   simParams.rnoise = 0.0;
   simParams.lrnoise = 0.0;
   simParams.tinoise = 0.0;
   simParams.rinoise = 0.0;
   simParams.temperature = 25.0;
   simParams.band_forming = false;
   simParams.cp_sector = 2;
   simParams.cp_pathlen = 10.0;
   simParams.cp_angle = 45.0;
   simParams.cp_width = 3.0;

   // Fill the user mesh vector.
   simParams.mesh_radius.push_back(0.12345);
   simParams.mesh_radius.push_back(0.67890);

   int rv = simParams.save_simparms(fileName);
   QCOMPARE(rv, 0);

   QFile inFile(fileName);
   QVERIFY(inFile.open(QIODevice::ReadOnly | QIODevice::Text));

   QXmlStreamReader xml(&inFile);
   int userMeshCount = 0;
   while (!xml.atEnd()) {
      xml.readNext();
      if (xml.isStartElement() && xml.name() == "usermesh") {
         QXmlStreamAttributes attr = xml.attributes();
         QVERIFY(attr.hasAttribute("radius"));
         QCOMPARE(
            attr.value("radius").toString(),
            QString().asprintf("%11.5e", simParams.mesh_radius[ userMeshCount ]).simplified());
         ++userMeshCount;
      }
   }
   QCOMPARE(userMeshCount, 2);
   inFile.close();
}

// Test that optional attributes (rotorCalID, rotorcoeffs, bandvolume) are written.
void TestUS_SimulationParameters::test_save_simparms_optionalAttributes() {
   QTemporaryFile tempFile;
   QVERIFY(tempFile.open());
   QString fileName = tempFile.fileName();

   US_SimulationParameters simParams;
   simParams.meshType = US_SimulationParameters::ASTFEM;
   simParams.gridType = US_SimulationParameters::FIXED;
   simParams.simpoints = 200;
   simParams.radial_resolution = 0.8;
   simParams.meniscus = 1.5;
   simParams.bottom = 2.5;
   simParams.rnoise = 0.05;
   simParams.lrnoise = 0.06;
   simParams.tinoise = 0.07;
   simParams.rinoise = 0.08;
   simParams.temperature = 37.0;
   // Set optional fields.
   simParams.rotorCalID = "CAL123";
   simParams.rotorcoeffs[ 0 ] = 1.234;
   simParams.rotorcoeffs[ 1 ] = 5.678;
   simParams.band_forming = true;
   simParams.band_volume = 42.0;
   simParams.cp_sector = 3;
   simParams.cp_pathlen = 12.0;
   simParams.cp_angle = 60.0;
   simParams.cp_width = 4.0;

   int rv = simParams.save_simparms(fileName);
   QCOMPARE(rv, 0);

   QFile inFile(fileName);
   QVERIFY(inFile.open(QIODevice::ReadOnly | QIODevice::Text));

   QXmlStreamReader xml(&inFile);
   bool foundRotorCalID = false;
   bool foundRotorcoeffs = false;
   bool foundBandVolume = false;
   while (!xml.atEnd()) {
      xml.readNext();
      if (xml.isStartElement() && xml.name() == "params") {
         QXmlStreamAttributes attr = xml.attributes();
         if (attr.hasAttribute("rotorCalID")) {
            QCOMPARE(attr.value("rotorCalID").toString(), QString("CAL123"));
            foundRotorCalID = true;
         }
         if (attr.hasAttribute("rotorcoeffs")) {
            QString coeffStr = attr.value("rotorcoeffs").toString();
            // Verify scientific format is present.
            QVERIFY(coeffStr.contains("e"));
            foundRotorcoeffs = true;
         }
         if (attr.hasAttribute("bandvolume")) {
            QCOMPARE(attr.value("bandvolume").toString(), QString::number(42.0));
            foundBandVolume = true;
         }
      }
   }
   QVERIFY(foundRotorCalID);
   QVERIFY(foundRotorcoeffs);
   QVERIFY(foundBandVolume);
   inFile.close();
}

// Test that the function returns an error when file cannot be opened.
void TestUS_SimulationParameters::test_save_simparms_file_open_fail() {
   // Provide an invalid file location to simulate failure.
   QString fileName = "/ABSFA/simparams.xml";
   QVERIFY(!QFileInfo(fileName).exists());

   US_SimulationParameters simParams;

   int rv = simParams.save_simparms(fileName);
   QCOMPARE(rv, -1);
}

void TestUS_SimulationParameters::test_speedstepFromXml_AllAttributes() {
   // Create an XML string with all attributes set.
   QString xml = "<speedstep "
                 "duration_hrs='2' duration_mins='30.5' "
                 "delay_hrs='1' delay_mins='15.25' "
                 "rotorspeed='1000' acceleration='150' "
                 "accelerflag='yes' scans='12' "
                 "w2tfirst='0.12' w2tlast='0.98' "
                 "timefirst='10' timelast='50' "
                 "set_speed='500' avg_speed='750.5' speed_stddev='25.75'/>";
   QXmlStreamReader reader(xml);
   // Advance to the start element.
   QVERIFY(reader.readNextStartElement());

   US_SimulationParameters::SpeedProfile spo;
   US_SimulationParameters::speedstepFromXml(reader, spo);

   QCOMPARE(spo.duration_hours, 2);
   QCOMPARE(spo.duration_minutes, 30.5);
   QCOMPARE(spo.delay_hours, 1);
   QCOMPARE(spo.delay_minutes, 15.25);
   QCOMPARE(spo.rotorspeed, 1000);
   QCOMPARE(spo.acceleration, 150);
   // According to the implementation, accelerflag uses indexOf – "yes" is present,
   // so it should be true.
   QVERIFY(spo.acceleration_flag);
   QCOMPARE(spo.scans, 12);
   QCOMPARE(spo.w2t_first, 0.12);
   QCOMPARE(spo.w2t_last, 0.98);
   QCOMPARE(spo.time_first, 10);
   QCOMPARE(spo.time_last, 50);
   QCOMPARE(spo.set_speed, 500);
   QCOMPARE(spo.avg_speed, 750.5);
   QCOMPARE(spo.speed_stddev, 25.75);
}

void TestUS_SimulationParameters::test_speedstepFromXml_MissingAttributes() {
   US_SimulationParameters simParams;
   // Create an XML string that omits some attributes.
   QString xml = "<speedstep "
                 "duration_hrs='3' delay_mins='20.0' "
                 "rotorspeed='800' scans='8'/>";
   QXmlStreamReader reader(xml);
   QVERIFY(reader.readNextStartElement());

   US_SimulationParameters::SpeedProfile spo;
   // Initialize some members with non-default values to check they persist if not overwritten.
   spo.duration_minutes = -1;
   spo.delay_hours = -1;
   spo.acceleration = -1;
   spo.acceleration_flag = true; // if not provided, remains unchanged
   spo.w2t_first = -1;
   spo.w2t_last = -1;
   spo.time_first = -1;
   spo.time_last = -1;
   spo.set_speed = -1;
   spo.avg_speed = -1;
   spo.speed_stddev = -1;

   US_SimulationParameters::speedstepFromXml(reader, spo);

   QCOMPARE(spo.duration_hours, 3);
   // duration_minutes was not provided
   QCOMPARE(spo.duration_minutes, -1.0);
   // delay_hrs was not provided
   QCOMPARE(spo.delay_hours, -1);
   QCOMPARE(spo.delay_minutes, 20.0);
   QCOMPARE(spo.rotorspeed, 800);
   // acceleration and acceleration_flag remain unchanged
   QCOMPARE(spo.acceleration, -1);
   QVERIFY(spo.acceleration_flag);
   QCOMPARE(spo.scans, 8);
   // The rest were not provided
   QCOMPARE(spo.w2t_first, -1.0);
   QCOMPARE(spo.w2t_last, -1.0);
   QCOMPARE(spo.time_first, -1);
   QCOMPARE(spo.time_last, -1);
   QCOMPARE(spo.set_speed, -1);
   QCOMPARE(spo.avg_speed, -1.0);
   QCOMPARE(spo.speed_stddev, -1.0);
}

void TestUS_SimulationParameters::test_speedstepFromXml_AccelerationFlagTrue() {
   // Test different variations of a "true" string and verify the flag is set.
   // The internal test searches in " 1YesyesTruetrue", so any substring that is found at an index > 0 should yield true.
   QStringList trueValues = { "yes", "Yes", "true", "True", "1" };
   for (const QString &val: trueValues) {
      QString xml = "<speedstep accelerflag='" + val + "'/>";
      QXmlStreamReader reader(xml);
      QVERIFY(reader.readNextStartElement());

      US_SimulationParameters::SpeedProfile spo;
      spo.acceleration_flag = false; // default false
      US_SimulationParameters::speedstepFromXml(reader, spo);
      // Check that the flag is set to true.
      QVERIFY2(spo.acceleration_flag, QString("accelerflag value '%1' should yield true").arg(val).toUtf8().constData());
   }
}

void TestUS_SimulationParameters::test_speedstepFromXml_AccelerationFlagFalse() {
   // For values not in the string " 1YesyesTruetrue" at appropriate index,
   // the flag should remain false. For example, "false" is not a substring that triggers condition.
   QStringList falseValues = { "false", "NO", "0" };
   for (const QString &val: falseValues) {
      QString xml;
      if (val.isEmpty())
         xml = "<speedstep/>";
      else
         xml = "<speedstep accelerflag='" + val + "'/>";
      QXmlStreamReader reader(xml);
      QVERIFY(reader.readNextStartElement());

      US_SimulationParameters::SpeedProfile spo;
      spo.acceleration_flag = true; // preset to true; expect function to set it to false if not found
      US_SimulationParameters::speedstepFromXml(reader, spo);
      // The flag should be false.
      QVERIFY2(!spo.acceleration_flag, QString("accelerflag value '%1' should yield false").arg(val).toUtf8().constData());
   }
}


// Test case for minimum required
void TestUS_SimulationParameters::test_load_simparams_minimum_file() {
   US_SimulationParameters simParams;
   US_SimulationParameters::SpeedProfile speedProfile;
   speedProfile.duration_hours = 5;
   speedProfile.duration_minutes = 6.7;
   speedProfile.delay_hours = 1;
   speedProfile.delay_minutes = 2.0;
   speedProfile.scans = 3;
   speedProfile.rotorspeed = 6000;
   speedProfile.acceleration = 800;
   speedProfile.acceleration_flag = false;
   speedProfile.w2t_first = 1.0;
   speedProfile.w2t_last = 4.0;
   speedProfile.time_first = 1;
   speedProfile.time_last = 200;
   speedProfile.avg_speed = 6000.0;
   speedProfile.speed_stddev = 0.1;
   speedProfile.set_speed = 6000;


   QTemporaryFile tmpFile;
   QVERIFY(tmpFile.open());
   QXmlStreamWriter xml(&tmpFile);
   xml.setAutoFormatting(true);

   xml.writeStartDocument();

   xml.writeStartElement("params");
   simParams.speedstepToXml(xml, &speedProfile);
   xml.writeEndElement(); // params

   xml.writeEndDocument();
   tmpFile.flush();
   simParams.speed_step.clear();
   QVERIFY(simParams.speed_step.isEmpty());
   int status = simParams.load_simparms(tmpFile.fileName());
   QCOMPARE(status, 0);
   QVERIFY(!simParams.speed_step.isEmpty());
}

void TestUS_SimulationParameters::test_load_simparams_all() {
   // Create a temporary file with all attributes and elements
   QTemporaryFile file;
   QVERIFY(file.open());
   QString tempFilename = file.fileName();

   QXmlStreamWriter xml(&file);
   xml.setAutoFormatting(true);

   xml.writeStartDocument();

   xml.writeStartElement("params");
   xml.writeAttribute("meshType", "MovingHat");
   xml.writeAttribute("gridType", "Moving");
   xml.writeAttribute("simpoints", "100");
   xml.writeAttribute("radialres", "12.34");
   xml.writeAttribute("meniscus", "1.23");
   xml.writeAttribute("bottom", "2.34");
   xml.writeAttribute("rnoise", "0.1");
   xml.writeAttribute("lrnoise", "0.2");
   xml.writeAttribute("tinoise", "0.3");
   xml.writeAttribute("rinoise", "0.4");
   xml.writeAttribute("temperature", "25.0");
   xml.writeAttribute("bandform", "yes");
   xml.writeAttribute("bandvolume", "0.55");
   xml.writeAttribute("rotorCalID", "cal123");
   xml.writeAttribute("rotorcoeffs", "1.0 2.0");
   xml.writeAttribute("sector", "3");
   xml.writeAttribute("pathlength", "1000.0");
   xml.writeAttribute("angle", "45.0");
   xml.writeAttribute("width", "5.5");
   xml.writeStartElement("speedstep");
   xml.writeAttribute("duration_hrs", "2");
   xml.writeAttribute("duration_mins", "30.5");
   xml.writeAttribute("delay_hrs", "1");
   xml.writeAttribute("delay_mins", "15.25");
   xml.writeAttribute("rotorspeed", "1000");
   xml.writeAttribute("acceleration", "150");
   xml.writeAttribute("accelerflag", "yes");
   xml.writeAttribute("scans", "12");
   xml.writeAttribute("w2tfirst", "0.12");
   xml.writeAttribute("w2tlast", "0.98");
   xml.writeAttribute("timefirst", "10");
   xml.writeAttribute("timelast", "50");
   xml.writeAttribute("set_speed", "500");
   xml.writeAttribute("avg_speed", "750.5");
   xml.writeAttribute("speed_stddev", "25.75");
   xml.writeEndElement(); // speedstep
   xml.writeStartElement("speedstep");
   xml.writeAttribute("duration_hrs", "4");
   xml.writeAttribute("duration_mins", "50.5");
   xml.writeAttribute("delay_hrs", "1");
   xml.writeAttribute("delay_mins", "15.25");
   xml.writeAttribute("rotorspeed", "1000");
   xml.writeAttribute("acceleration", "150");
   xml.writeAttribute("accelerflag", "yes");
   xml.writeAttribute("scans", "12");
   xml.writeAttribute("w2tfirst", "0.12");
   xml.writeAttribute("w2tlast", "0.98");
   xml.writeAttribute("timefirst", "10");
   xml.writeAttribute("timelast", "50");
   xml.writeAttribute("set_speed", "500");
   xml.writeAttribute("avg_speed", "750.5");
   xml.writeAttribute("speed_stddev", "25.75");
   xml.writeEndElement(); // speedstep
   xml.writeStartElement("usermesh");
   xml.writeAttribute("radius", "3.3");
   xml.writeEndElement();
   xml.writeStartElement("usermesh");
   xml.writeAttribute("radius", "4.4");
   xml.writeEndElement(); // usermesh
   xml.writeEndElement(); // params
   xml.writeEndDocument();
   file.flush();

   // Load the parameters
   US_SimulationParameters simparms;
   int stat = simparms.load_simparms(file.fileName());
   QCOMPARE(stat, 0);

   // Check attribute loading from <params>
   // (The returned indices for the string lists are assumed; adjust if needed.)
   QCOMPARE(static_cast<int>(simparms.meshType), 2); // "MovingHat" index (0: ASTFEM,1: Claverie,2: MovingHat,...)
   QCOMPARE(static_cast<int>(simparms.gridType), 1); // "Moving" index (0: Fixed, 1: Moving)
   QCOMPARE(simparms.simpoints, 100);
   QCOMPARE(simparms.radial_resolution, 12.34);
   QCOMPARE(simparms.meniscus, 1.23);
   QCOMPARE(simparms.bottom, 2.34);
   QCOMPARE(simparms.rnoise, 0.1);
   QCOMPARE(simparms.lrnoise, 0.2);
   QCOMPARE(simparms.tinoise, 0.3);
   QCOMPARE(simparms.rinoise, 0.4);
   QCOMPARE(simparms.temperature, 25.0);
   QVERIFY(simparms.band_forming); // "yes" should enable band forming
   QCOMPARE(simparms.band_volume, 0.55);
   QCOMPARE(simparms.rotorCalID, QString("cal123"));
   // rotorcoeffs array: first two values specified
   QCOMPARE(simparms.rotorcoeffs[ 0 ], 1.0);
   QCOMPARE(simparms.rotorcoeffs[ 1 ], 2.0);
   QCOMPARE(simparms.cp_sector, 3);
   QCOMPARE(simparms.cp_pathlen, 1000.0);
   QCOMPARE(simparms.cp_angle, 45.0);
   QCOMPARE(simparms.cp_width, 5.5);

   // Check speed step count
   QCOMPARE(simparms.speed_step.count(), 2);

   // Check usermesh: two entries expected (mesh_radius)
   QCOMPARE(simparms.mesh_radius.count(), 2);
   QCOMPARE(simparms.mesh_radius.at(0), 3.3);
   QCOMPARE(simparms.mesh_radius.at(1), 4.4);
}

// Test case 2: Minimum file (only <params> with required attributes) but missing <speedstep>
void TestUS_SimulationParameters::test_load_simparams_missing_speedstep() {
   QTemporaryFile file;
   QVERIFY(file.open());
   QString tempFilename = file.fileName();

   QTextStream out(&file);
   out << "<?xml version=\"1.0\"?>\n"
       << "<root>\n"
       << "  <params meshType=\"ASTFEM\" gridType=\"Fixed\" "
       << "simpoints=\"50\" radialres=\"10.0\" meniscus=\"1.0\" bottom=\"2.0\" "
       << "rnoise=\"0.05\" lrnoise=\"0.1\" tinoise=\"0.15\" rinoise=\"0.2\" temperature=\"20.0\" "
       << "bandform=\"no\" rotorCalID=\"cal001\" rotorcoeffs=\"0.5 1.5\" "
       << "sector=\"2\" pathlength=\"800.0\" angle=\"30.0\" width=\"4.0\"/>\n"
       << "</root>\n";
   file.flush();

   US_SimulationParameters simparms;
   int stat = simparms.load_simparms(tempFilename);
   // As speed_step is missing, the function should revert to the old speed_step and return an error.
   QVERIFY(stat != 0);
   QCOMPARE(simparms.speed_step.count(), 1); // unchanged (or reverted)
}
