//! \file us_hardware.cpp
#include "us_hardware.h"

#include "us_settings.h"

// Read rotor information and generate a rotorCalID,coeffs map from Local
bool US_Hardware::readRotorMap(QMap<QString, QString>& rotor_map) {
  bool ok = false;
  QString path = US_Settings::appBaseDir() + "/etc/rotors";

  rotor_map.clear();
  rotor_map["0"] = "0 0";  // Create a simulation entry (no stretch)

  QDir dir(path);

  if (!dir.exists()) return ok;

  QStringList filter("C*.xml");
  QStringList fnames = dir.entryList(filter, QDir::Files, QDir::Name);

  for (int ii = 0; ii < fnames.size(); ii++) {  // Browse the calibration files
    QFile file(path + "/" + fnames[ii]);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "*ERROR: Could not read rotor file\n" << fnames[ii];
      continue;
    }

    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {  // Get ID and coefficients from a file
      xml.readNext();

      if (xml.isStartElement() && xml.name() == "Calibration") {
        QXmlStreamAttributes att = xml.attributes();

        QString calID = att.value("id").toString();
        QString coeffs = att.value("coeff1").toString() + " " +
                         att.value("coeff2").toString();

        rotor_map[calID] = coeffs;
        ok = true;
      }
    }
  }

  return ok;
}

// Read rotor information and generate a rotorCalID,coeffs map from DB
bool US_Hardware::readRotorMap(US_DB2* db, QMap<QString, QString>& rotor_map) {
  bool ok = false;

  if (db == 0 || !db->isConnected()) return readRotorMap(rotor_map);

  rotor_map.clear();
  rotor_map["0"] = "0 0";  // Create a simulation entry (no stretch)

  QStringList query;
  QStringList labIDs;
  QStringList rotorIDs;
  QStringList calibIDs;

  query << "get_lab_names";
  db->query(query);

  if (db->lastErrno() != US_DB2::OK) {
    qDebug() << "*ERROR* Unable to get lab IDs";
    return ok;
  }

  while (db->next())  // Get a list of lab IDs
  {
    labIDs << db->value(0).toString();  // Add a lab ID
  }

  for (int ii = 0; ii < labIDs.size();
       ii++) {  // Add to the list of rotor IDs, from each lab
    QString labID = labIDs[ii];

    query.clear();
    query << "get_rotor_names" << labID;
    db->query(query);

    if (db->lastErrno() != US_DB2::OK) {
      qDebug() << "*WARNING* Unable to get rotors for lab ID" << labID;
      continue;
    }

    while (db->next()) {
      rotorIDs << db->value(0).toString();  // Add rotor ID
    }
  }

  for (int ii = 0; ii < rotorIDs.size();
       ii++) {  // Add to the list of calibration IDs, from each rotor
    QString rotorID = rotorIDs[ii];

    query.clear();
    query << "get_rotor_calibration_profiles" << rotorID;
    db->query(query);

    if (db->lastErrno() != US_DB2::OK) {
      qDebug() << "*WARNING* Unable to get calIDs for rotor ID" << rotorID;
      continue;
    }

    while (db->next()) {
      calibIDs << db->value(0).toString();  // Add a calibration ID
    }
  }

  for (int ii = 0; ii < calibIDs.size();
       ii++) {  // Build coefficient pairs mapped to each calibration ID
    QString calibID = calibIDs[ii];

    query.clear();
    query << "get_rotor_calibration_info" << calibID;
    db->query(query);

    if (db->lastErrno() != US_DB2::OK) {
      qDebug() << "*WARNING* Unable to get info for calib ID" << calibID;
      continue;
    }

    while (db->next()) {  // Map this coefficient pair to a calibration ID
      QString coeffs = db->value(4).toString() + " " + db->value(5).toString();

      rotor_map[calibID] = coeffs;
      ok = true;  // Mark that at least one pair found
    }
  }

  return ok;
}

// Get rotor coefficients for a given rotor calibration identifier
bool US_Hardware::rotorValues(QString rCalID, QMap<QString, QString> rotor_map,
                              double* rotcoeffs) {
  bool ok = rotor_map.contains(rCalID);  // Flag:  calibration ID found?

  if (ok) {  // Return the coefficient pair for the given calibration ID
    QString coeffs = rotor_map[rCalID];
    rotcoeffs[0] = coeffs.section(" ", 0, 0).toDouble();
    rotcoeffs[1] = coeffs.section(" ", 1, 1).toDouble();
  }

  return ok;
}

US_AbstractCenterpiece::US_AbstractCenterpiece() {
  serial_number = 0;
  guid = "";
  name = "";
  material = "";
  channels = 1;
  shape = "standard";
  angle = 2.5;
  width = 0.0;
  path_length.clear();
  bottom_position.clear();
}

// Read centerpiece information from database or local
bool US_AbstractCenterpiece::read_centerpieces(
    US_DB2* db, QList<US_AbstractCenterpiece>& centerpieces) {
  centerpieces.clear();
  US_AbstractCenterpiece cp;
  bool got_cp = false;

  while (db != NULL &&
         db->isConnected()) {  // Get centerpiece information from the database
    QList<int> cpids;
    QStringList query;

    query << "get_abstractCenterpiece_names";
    db->query(query);

    if (db->lastErrno() != US_DB2::OK) {
      qDebug() << "*ERROR* Unable to get centerpiece information from DB";
      break;
    }

    while (db->next()) {
      cpids << db->value(0).toString().toInt();
    }

    qSort(cpids);

    for (int ii = 0; ii < cpids.size(); ii++) {
      cp.serial_number = cpids[ii];
      QString cpid = QString::number(cp.serial_number);
      query.clear();
      query << "get_abstractCenterpiece_info" << cpid;
      db->query(query);
      db->next();

      cp.guid = db->value(0).toString();
      cp.name = db->value(1).toString();
      cp.channels = db->value(2).toString().toInt();
      cp.channels = qMax(cp.channels, 1);
      QString bottoms = db->value(3).toString();
      cp.shape = db->value(4).toString();
      cp.shape = cp.shape.isEmpty() ? "sector" : cp.shape;
      cp.maxRPM = db->value(5).toString().toDouble();
      double pathlen = db->value(6).toString().toDouble();
      cp.angle = db->value(7).toString().toDouble();
      cp.width = db->value(8).toString().toDouble();
      cp.material = cp.name.section(" ", 0, 0);

      cp.path_length.clear();
      cp.bottom_position.clear();
      for (int jj = 0; jj < cp.channels; jj++) {
        cp.bottom_position << bottoms.section(":", jj, jj).toDouble();
        cp.path_length << pathlen;
      }

      centerpieces << cp;  // Add centerpiece entry
    }

    got_cp = true;
    break;
  }

  if (!got_cp) {  // Get centerpiece information from local (if db==NULL or db
                  // failed)
    QFile cp_file(US_Settings::appBaseDir() + "/etc/abstractCenterpieces.xml");

    if (!cp_file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

    QXmlStreamReader xml(&cp_file);
    double xversion = 1.0;

    while (!xml.atEnd()) {
      xml.readNext();

      if (xml.isStartElement()) {
        if (xml.name() == "abstractCenterpieces") {
          QXmlStreamAttributes a = xml.attributes();
          xversion = a.value("version").toString().toDouble();
        }

        if (xml.name() == "abstractCenterpiece") {
          if (cp.serial_number > 0) centerpieces << cp;
          cp = US_AbstractCenterpiece();

          QXmlStreamAttributes a = xml.attributes();
          cp.serial_number = a.value("id").toString().toInt();
          cp.guid = a.value("guid").toString();
          cp.name = a.value("name").toString();
          cp.material = a.value("materialName").toString();
          cp.channels = a.value("channels").toString().toInt();
          cp.shape = a.value("shape").toString();
          cp.angle = a.value("angle").toString().toDouble();
          cp.width = a.value("width").toString().toDouble();

          if (xversion == 1.0) {
            if ((cp.channels % 2) == 0) cp.channels--;
            if (cp.serial_number == 2 || cp.serial_number == 7)
              cp.maxRPM = 42000;
            else if (cp.serial_number == 4)
              cp.maxRPM = 48000;
            else
              cp.maxRPM = 60000;
          } else {
            cp.maxRPM = a.value("maxRPM").toString().toDouble();
          }
        }

        if (xml.name() == "row") {
          QXmlStreamAttributes a = xml.attributes();
          cp.path_length << a.value("pathlen").toString().toDouble();
          cp.bottom_position << a.value("bottom").toString().toDouble();
        }
      }
    }

    centerpieces << cp;  // Add last centerpiece

    cp_file.close();
    got_cp = true;
  }

  return got_cp;
}

// Read centerpiece information from local
bool US_AbstractCenterpiece::read_centerpieces(
    QList<US_AbstractCenterpiece>& centerpieces) {
  return read_centerpieces(NULL, centerpieces);
}
