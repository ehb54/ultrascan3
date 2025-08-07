//! \file us_analyte.cpp
#include "us_analyte.h"

#include "us_constants.h"
#include "us_datafiles.h"
#include "us_eprofile.h"
#include "us_math2.h"
#include "us_settings.h"
#include "us_util.h"

#define DEBUG_QUERY                                \
  qDebug() << "Q" << q << "Err" << db->lastErrno() \
           << db->lastError();  //!< Debug print showing DB query string and
                                //!< error returns

US_Analyte::US_Analyte() {
  vbar20 = TYPICAL_VBAR;
  mw = 0.0;
  description = "New Analyte";
  analyteGUID.clear();
  sequence.clear();
  type = PROTEIN;
  grad_form = false;

  replace_spectrum = false;
  new_or_changed_spectrum = false;
  // Placeholders for DNA/RNA
  doubleStranded = true;
  complement = false;
  _3prime = false;
  _5prime = false;

  sodium = 0.0;
  potassium = 0.0;
  lithium = 0.0;
  magnesium = 0.0;
  calcium = 0.0;

  extinction.clear();
  refraction.clear();
  fluorescence.clear();
}

bool US_Analyte::operator==(const US_Analyte& a) const {
  if (vbar20 != a.vbar20) return false;
  if (mw != a.mw) return false;
  if (description != a.description) return false;
  if (analyteGUID != a.analyteGUID) return false;
  if (sequence != a.sequence) return false;
  if (type != a.type) return false;
  if (extinction != a.extinction) return false;
  if (refraction != a.refraction) return false;
  if (fluorescence != a.fluorescence) return false;
  if (grad_form != a.grad_form) return false;

  if (type == DNA || type == RNA) {
    if (doubleStranded != a.doubleStranded) return false;
    if (complement != a.complement) return false;
    if (_3prime != a._3prime) return false;
    if (_5prime != a._5prime) return false;
    if (sodium != a.sodium) return false;
    if (potassium != a.potassium) return false;
    if (lithium != a.lithium) return false;
    if (magnesium != a.magnesium) return false;
    if (calcium != a.calcium) return false;
  }

  // Not comparing message and analyteID

  return true;
}

int US_Analyte::load(bool db_access, const QString& guid, US_DB2* db) {
  if (db_access)
    return load_db(guid, db);
  else
    return load_disk(guid);
}

int US_Analyte::load_db(const QString& load_guid, US_DB2* db) {
  int error = US_DB2::OK;

  // Get analyteID
  QStringList q("get_analyteID");
  q << load_guid;

  db->query(q);
  DEBUG_QUERY;
  error = db->lastErrno();

  if (error != US_DB2::OK) {
    message = QObject::tr("Could not get analyteID");
    return error;
  }

  db->next();
  analyteID = db->value(0).toString();

  // Get analyte info;
  q.clear();
  q << "get_analyte_info" << analyteID;

  db->query(q);
  DEBUG_QUERY;
  error = db->lastErrno();

  if (error != US_DB2::OK) {
    message = QObject::tr("Could not get analyte info");
    return error;
  }

  db->next();

  analyteGUID = load_guid;
  ;
  QString a_type = db->value(1).toString();
  if (a_type == "Protein")
    type = PROTEIN;
  else if (a_type == "DNA")
    type = DNA;
  else if (a_type == "RNA")
    type = RNA;
  else if (a_type == "Other")
    type = CARBOHYDRATE;
  else
    type = PROTEIN;

  sequence = db->value(2).toString();
  vbar20 = db->value(3).toString().toDouble();
  description = db->value(4).toString();
  // We don't need spectrum  -- db->value( 5 ).toString();
  mw = db->value(6).toString().toDouble();
  grad_form = (type == CARBOHYDRATE)
                  ? US_Util::bool_flag(db->value(7).toString())
                  : false;

  q.clear();
  q << "get_nucleotide_info" << analyteID;
  db->query(q);
  DEBUG_QUERY;
  db->next();

  doubleStranded = US_Util::bool_flag(db->value(0).toString());
  complement = US_Util::bool_flag(db->value(1).toString());
  _3prime = US_Util::bool_flag(db->value(2).toString());
  _5prime = US_Util::bool_flag(db->value(3).toString());
  sodium = db->value(4).toString().toDouble();
  potassium = db->value(5).toString().toDouble();
  lithium = db->value(6).toString().toDouble();
  magnesium = db->value(7).toString().toDouble();
  calcium = db->value(8).toString().toDouble();

  if (type == DNA || type == RNA) nucleotide();

  q.clear();
  /*
  q << "get_spectrum" << analyteID << "Analyte" << "Extinction";

  db->query( q );
DEBUG_QUERY;

  while ( db->next() )
  {
     double lambda = db->value( 1 ).toDouble();
     double coeff  = db->value( 2 ).toDouble();
     extinction[ lambda ] = coeff;
  }

  q[ 3 ] = "Refraction";
  db->query( q );

  while ( db->next() )
  {
     double lambda = db->value( 1 ).toDouble();
     double coeff  = db->value( 2 ).toDouble();
     refraction[ lambda ] = coeff;
  }

  q[ 3 ] = "Fluorescence";
  db->query( q );

  while ( db->next() )
  {
     double lambda = db->value( 1 ).toDouble();
     double coeff  = db->value( 2 ).toDouble();
     fluorescence[ lambda ] = coeff;
  }
  */
  QString compType("Analyte");
  QString valType("molarExtinction");
  US_ExtProfile::fetch_eprofile(db, analyteID.toInt(), compType, valType,
                                extinction);

  return US_DB2::OK;
}

int US_Analyte::load_disk(const QString& guid) {
  int error = US_DB2::NO_ANALYTE;  // Error by default

  QString path;

  if (!analyte_path(path)) {
    message = QObject::tr("Could not create analyte directory");
    return error;
  }

  QDir f(path);
  QStringList filter("A*.xml");
  QStringList names = f.entryList(filter, QDir::Files, QDir::Name);
  QString filename;
  bool found = false;

  for (int i = 0; i < names.size(); i++) {
    filename = path + "/" + names[i];
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

    QXmlStreamReader xml(&file);

    while (!xml.atEnd()) {
      xml.readNext();

      if (xml.isStartElement()) {
        if (xml.name() == "analyte") {
          QXmlStreamAttributes a = xml.attributes();

          if (a.value("analyteGUID").toString() == guid) found = true;
          break;
        }
      }
    }

    file.close();

    if (found) return read_analyte(filename);
  }

  message = QObject::tr("Could not find analyte guid");
  return error;
}

int US_Analyte::read_analyte(const QString& filename) {
  QFile file(filename);

  // Read in the filename and populate class
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot open file " << filename;
    message = QObject::tr("Could not open analyte file for reading");
    return US_DB2::DBERROR;
  }

  double freq;
  double value;
  QString type_string;
  QXmlStreamReader xml(&file);
  QXmlStreamAttributes a;

  while (!xml.atEnd()) {
    xml.readNext();

    if (xml.isStartElement()) {
      if (xml.name() == "analyte") {
        a = xml.attributes();

        type_string = a.value("type").toString();

        // Set description and guid
        description = a.value("description").toString();
        analyteGUID = a.value("analyteGUID").toString();
        vbar20 = a.value("vbar20").toString().toDouble();

        // Set type
        if (type_string == "PROTEIN") {
          type = PROTEIN;
        }

        else if (type_string == "DNA" || type_string == "RNA") {
          type = (type_string == "DNA") ? DNA : RNA;
        }

        else if (type_string == "CARBOHYDRATE") {
          type = CARBOHYDRATE;
        }

        mw = a.value("mw").toString().toDouble();
        grad_form = US_Util::bool_flag(a.value("gradient_forming").toString());
      }

      else if (xml.name() == "sequence") {
        sequence = xml.readElementText();

        // Set mw
        if (type == PROTEIN) {
          US_Math2::Peptide p;
          US_Math2::calc_vbar(p, sequence, NORMAL_TEMP);

          // The sequence tag comes before the extinction extinction tag
          // so a value set there will override this setting, if it
          // exists.  It's not the way xml is really supposed work, but it
          // will be ok in this case.

          extinction[280.0] = p.e280;
          mw = (mw == 0.0) ? p.mw : mw;
        } else if (type == DNA || type == RNA) {
          doubleStranded = US_Util::bool_flag(a.value("stranded").toString());
          complement =
              US_Util::bool_flag(a.value("complement_only").toString());
          _3prime =
              US_Util::bool_flag(a.value("ThreePrimeHydroxyl").toString());
          _5prime = US_Util::bool_flag(a.value("FivePrimeHydroxyl").toString());

          sodium = a.value("sodium").toString().toDouble();
          potassium = a.value("potassium").toString().toDouble();
          lithium = a.value("lithium").toString().toDouble();
          magnesium = a.value("magnesium").toString().toDouble();
          calcium = a.value("calcium").toString().toDouble();

          nucleotide();
        }

      } else if (xml.name() == "extinction") {
        QXmlStreamAttributes a = xml.attributes();
        freq = a.value("frequency").toString().toDouble();
        value = a.value("value").toString().toDouble();
        extinction[freq] = value;
      }

      else if (xml.name() == "refraction") {
        QXmlStreamAttributes a = xml.attributes();
        freq = a.value("frequency").toString().toDouble();
        value = a.value("value").toString().toDouble();
        refraction[freq] = value;
      }

      else if (xml.name() == "fluorescence") {
        QXmlStreamAttributes a = xml.attributes();
        freq = a.value("frequency").toString().toDouble();
        value = a.value("value").toString().toDouble();
        fluorescence[freq] = value;
      }
    }
  }

  return US_DB2::OK;
}

void US_Analyte::nucleotide(void) {
  sequence.toLower();

  uint A = sequence.count("a");
  uint C = sequence.count("c");
  uint G = sequence.count("g");
  uint T = sequence.count("t");
  uint U = sequence.count("u");

  const double mw_A = 313.209;
  const double mw_C = 289.184;
  const double mw_G = 329.208;
  const double mw_T = 304.196;
  const double mw_U = 274.170;

  mw = 0.0;
  uint total = A + G + C + T + U;

  if (doubleStranded) total *= 2;

  if (type == DNA) {
    if (doubleStranded) {
      mw += A * mw_A;
      mw += G * mw_G;
      mw += C * mw_C;
      mw += T * mw_T;
      mw += A * mw_T;
      mw += G * mw_C;
      mw += C * mw_G;
      mw += T * mw_A;
    }

    if (complement) {
      mw += A * mw_T;
      mw += G * mw_C;
      mw += C * mw_G;
      mw += T * mw_A;
    }

    if (!complement && !doubleStranded) {
      mw += A * mw_A;
      mw += G * mw_G;
      mw += C * mw_C;
      mw += T * mw_T;
    }
  } else /* RNA */
  {
    if (doubleStranded) {
      mw += A * (mw_A + 15.999);
      mw += G * (mw_G + 15.999);
      mw += C * (mw_C + 15.999);
      mw += U * (mw_U + 15.999);
      mw += A * (mw_U + 15.999);
      mw += G * (mw_C + 15.999);
      mw += C * (mw_G + 15.999);
      mw += U * (mw_A + 15.999);
    }

    if (complement) {
      mw += A * (mw_U + 15.999);
      mw += G * (mw_C + 15.999);
      mw += C * (mw_G + 15.999);
      mw += U * (mw_A + 15.999);
    }

    if (!complement && !doubleStranded) {
      mw += A * (mw_A + 15.999);
      mw += G * (mw_G + 15.999);
      mw += C * (mw_C + 15.999);
      mw += U * (mw_U + 15.999);
    }
  }

  mw += sodium * total * 22.99;
  mw += potassium * total * 39.1;
  mw += lithium * total * 6.94;
  mw += magnesium * total * 24.305;
  mw += calcium * total * 40.08;

  if (_3prime) {
    mw += 17.01;
    if (doubleStranded) mw += 17.01;
  } else  // we have phosphate
  {
    mw += 94.87;
    if (doubleStranded) mw += 94.87;
  }

  if (_5prime) {
    mw -= 77.96;
    if (doubleStranded) mw -= 77.96;
  }
}

bool US_Analyte::analyte_path(QString& path) {
  QDir dir;
  path = US_Settings::dataDir() + "/analytes";

  if (!dir.exists(path)) {
    if (!dir.mkpath(path)) {
      return false;
    }
  }

  return true;
}

int US_Analyte::write(bool db_access, const QString& filename, US_DB2* db) {
  if (db_access)
    return write_db(db);
  else
    return write_disk(filename);
}

int US_Analyte::write_disk(const QString& filename) {
  QFile file(filename);

  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    qDebug() << "Cannot open file for writing: " << filename;
    message = QObject::tr("Cannot open file for writing");
    return US_DB2::DBERROR;
  }

  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);

  xml.writeStartDocument();
  xml.writeDTD("<!DOCTYPE US_Analyte>");
  xml.writeStartElement("AnalyteData");
  xml.writeAttribute("version", "1.0");

  xml.writeStartElement("analyte");

  // Set attributes depending on type
  QString b;  // bool

  switch (type) {
    case US_Analyte::PROTEIN: {
      if (vbar20 < 1e-2) {
        US_Math2::Peptide p;
        US_Math2::calc_vbar(p, sequence, NORMAL_TEMP);
        vbar20 = p.vbar20;
      }

      xml.writeAttribute("type", "PROTEIN");
      xml.writeAttribute("vbar20", QString::number(vbar20));
    } break;

    case US_Analyte::DNA:
    case US_Analyte::RNA:
      if (type == US_Analyte::DNA)
        xml.writeAttribute("type", "DNA");
      else
        xml.writeAttribute("type", "RNA");
      xml.writeAttribute("stranded", US_Util::bool_string(doubleStranded));
      xml.writeAttribute("complement_only", US_Util::bool_string(complement));
      xml.writeAttribute("ThreePrimeHydroxyl", US_Util::bool_string(_3prime));
      xml.writeAttribute("FivePrimeHydroxyl", US_Util::bool_string(_5prime));

      xml.writeAttribute("sodium", QString::number(sodium));
      xml.writeAttribute("potassium", QString::number(potassium));
      xml.writeAttribute("lithium", QString::number(lithium));
      xml.writeAttribute("magnesium", QString::number(magnesium));
      xml.writeAttribute("calcium", QString::number(calcium));
      xml.writeAttribute("vbar20", QString::number(vbar20));
      break;

    case US_Analyte::CARBOHYDRATE:
      xml.writeAttribute("type", "CARBOHYDRATE");
      xml.writeAttribute("vbar20", QString::number(vbar20));
      xml.writeAttribute("gradient_forming", US_Util::bool_string(grad_form));
      break;
  }

  if (mw > 0.0) xml.writeAttribute("mw", QString::number(mw));

  xml.writeAttribute("description", description);
  xml.writeAttribute("analyteGUID", analyteGUID);

  xml.writeStartElement("sequence");
  xml.writeCharacters("\n");

  for (int i = 0; i < sequence.length() / 80; i++)
    xml.writeCharacters(sequence.mid(i * 80, 80) + "\n");

  if (sequence.length() % 80 > 0)
    xml.writeCharacters(sequence.mid((sequence.length() / 80) * 80));

  xml.writeCharacters("\n");
  xml.writeEndElement();  // sequence

  // Add extinction values
  QList<double> keys = extinction.keys();
  double freq;
  double value;

  for (int i = 0; i < keys.size(); i++) {
    freq = keys[i];
    value = extinction[keys[i]];

    xml.writeStartElement("extinction");
    xml.writeAttribute("frequency", QString::number(freq, 'f', 1));
    xml.writeAttribute("value", QString::number(value));
    xml.writeEndElement();  // extinction
  }

  // Add refraction values
  keys = refraction.keys();

  for (int i = 0; i < keys.size(); i++) {
    freq = keys[i];
    value = refraction[keys[i]];

    xml.writeStartElement("refraction");
    xml.writeAttribute("frequency", QString::number(freq, 'f', 1));
    xml.writeAttribute("value", QString::number(value));
    xml.writeEndElement();  // refraction
  }

  // Add fluorescence values
  keys = fluorescence.keys();

  for (int i = 0; i < keys.size(); i++) {
    freq = keys[i];
    value = fluorescence[keys[i]];

    xml.writeStartElement("fluorescence");
    xml.writeAttribute("frequency", QString::number(freq, 'f', 1));
    xml.writeAttribute("value", QString::number(value));
    xml.writeEndElement();  // fluorescence
  }

  xml.writeEndElement();  // analyte
  xml.writeEndDocument();
  file.close();

  file.close();

  return US_DB2::OK;
}

void US_Analyte::set_spectrum(US_DB2* db) {
  QStringList q;

  q << "delete_spectrum" << analyteID << "Analyte" << "Extinction";
  db->statusQuery(q);
  DEBUG_QUERY;
  q[3] = "Refraction";
  db->statusQuery(q);
  DEBUG_QUERY;
  q[3] = "Fluorescence";
  db->statusQuery(q);
  DEBUG_QUERY;

  QList<double> keys = extinction.keys();
  int kntext = keys.size();
  kntext = (kntext > 0 && extinction.values()[0] == 0.0) ? 0 : kntext;

  q.clear();
  q << "new_spectrum" << analyteID << "Analyte" << "Extinction" << "" << "";

  for (int i = 0; i < kntext; i++) {
    double key = keys[i];
    QString lambda = QString::number(key, 'f', 1);
    q[4] = lambda;

    QString coeff = QString::number(extinction[key]);
    q[5] = coeff;

    db->statusQuery(q);
    DEBUG_QUERY;
  }

  keys = refraction.keys();
  int kntref = keys.size();
  kntref = (kntref > 0 && refraction.values()[0] == 0.0) ? 0 : kntref;

  q[3] = "Refraction";

  for (int i = 0; i < kntref; i++) {
    double key = keys[i];
    QString lambda = QString::number(key, 'f', 1);
    q[4] = lambda;

    QString coeff = QString::number(refraction[key]);
    q[5] = coeff;

    db->statusQuery(q);
    DEBUG_QUERY;
  }

  keys = fluorescence.keys();
  int kntflu = keys.size();
  kntflu = (kntflu > 0 && fluorescence.values()[0] == 0.0) ? 0 : kntflu;

  q[3] = "Fluorescence";

  for (int i = 0; i < kntflu; i++) {
    double key = keys[i];
    QString lambda = QString::number(key, 'f', 1);
    q[4] = lambda;

    QString coeff = QString::number(fluorescence[key]);
    q[5] = coeff;

    db->statusQuery(q);
    DEBUG_QUERY;
  }

  q.clear();
  q << "count_spectrum" << analyteID << "Analyte" << "Extinction";
  int cntext = db->functionQuery(q);
  DEBUG_QUERY;
  q[3] = "Refraction";
  int cntref = db->functionQuery(q);
  DEBUG_QUERY;
  q[3] = "Fluorescence";
  int cntflu = db->functionQuery(q);
  DEBUG_QUERY;

  if ((kntext != cntext) || (kntref != cntref) || (kntflu != cntflu)) {
    qDebug() << "set_spectrum *ERROR* ext k c" << kntext << cntext << "ref k c"
             << kntref << cntref << "flu k c" << kntflu << cntflu;
  }
}

int US_Analyte::write_db(US_DB2* db) {
  qDebug() << "Extinciton 1 : " << extinction.count();

  QStringList q;
  bool insert = true;

  message = QObject::tr("inserted into");

  if (analyteGUID.size() != 36) {
    message = QObject::tr("The analyte GUID is invalid");
    return US_DB2::BADGUID;
  }

  q << "new_analyte" << analyteGUID;

  // Check that the guid exists in the db
  QStringList q2;

  q2 << "get_analyteID" << analyteGUID;

  db->query(q2);
  DEBUG_QUERY;

  if (db->lastErrno() == US_DB2::OK) {
    db->next();
    analyteID = db->value(0).toString();
    q[0] = "update_analyte";
    q[1] = analyteID;
    message = QObject::tr("updated in");
    insert = false;
  }

  // Finish populating the query
  if (type == US_Analyte::PROTEIN)
    q << "Protein";
  else if (type == US_Analyte::DNA)
    q << "DNA";
  else if (type == US_Analyte::RNA)
    q << "RNA";
  else
    q << "Other";

  QString spectrum = "";  // Unused element
  double lambda1 = 0.0;
  double coeff1 = 0.0;

  if (extinction.keys().count() > 0) {
    lambda1 = extinction.keys()[0];
    coeff1 = extinction[lambda1];
  }

  else if (refraction.keys().count() > 0) {
    lambda1 = refraction.keys()[0];
    coeff1 = refraction[lambda1];
  }

  else if (fluorescence.keys().count() > 0) {
    lambda1 = fluorescence.keys()[0];
    coeff1 = fluorescence[lambda1];
  }

  if (coeff1 != 0.0) {  // Compose spectrum string from first lambda/coeff pair
    spectrum = QString::number(lambda1) + "/" + QString::number(coeff1);
  }

  q << sequence;
  q << QString::number(vbar20);
  q << description;
  q << spectrum;
  q << QString::number(mw);
  q << US_Util::bool_string(grad_form);

  if (insert) q << QString::number(US_Settings::us_inv_ID());

  db->statusQuery(q);
  DEBUG_QUERY;

  int error = db->lastErrno();
  if (error != US_DB2::OK) {
    message = QObject::tr("Could not update the DB");
    return error;
  }

  if (insert) analyteID = QString::number(db->lastInsertID());

  if (type == US_Analyte::DNA || type == US_Analyte::RNA) write_nucleotide(db);

  // set_spectrum( db );

  QString compType("Analyte");
  QString valType("molarExtinction");
  qDebug() << "AnalyteID for extProfile: " << analyteID.toInt();

  // if ( !extinction.isEmpty() )

  qDebug() << "new_or_changed_spectrum: " << new_or_changed_spectrum;
  qDebug() << "Extinciton 2 : " << extinction.count();

  if (!extinction.isEmpty() and new_or_changed_spectrum) {
    if (!replace_spectrum) {
      qDebug() << "Creating Spectrum!!!";
      US_ExtProfile::create_eprofile(db, analyteID.toInt(), compType, valType,
                                     extinction);
    } else {
      qDebug() << "Updating Spectrum!!!";

      QMap<double, double> new_extinction = extinction;
      int profileID = US_ExtProfile::fetch_eprofile(
          db, analyteID.toInt(), compType, valType, extinction);

      // qDebug() << "Old Extinction keys: " << extinction.keys().count() << ",
      // ProfileID: " << profileID; US_ExtProfile::update_eprofile( db,
      // profileID, analyteID.toInt(), compType, valType, new_extinction);
      // qDebug() << "New Extinction keys: " << new_extinction.keys().count() <<
      // ", ProfileID: " << profileID;

      if (profileID == -1) {
        qDebug() << "Creating profile when profileID = -1!!!! ";
        US_ExtProfile::create_eprofile(db, analyteID.toInt(), compType, valType,
                                       extinction);
      } else {
        qDebug() << "Old Extinction keys: " << extinction.keys().count()
                 << ", ProfileID: " << profileID;
        US_ExtProfile::update_eprofile(db, profileID, analyteID.toInt(),
                                       compType, valType, new_extinction);
        qDebug() << "New Extinction keys: " << new_extinction.keys().count()
                 << ", ProfileID: " << profileID;
      }
      replace_spectrum = false;
    }
    new_or_changed_spectrum = false;
  }

  // Write to disk too
  QString path;
  analyte_path(path);
  QString filename = get_filename(path, analyteGUID);
  write_disk(filename);

  return US_DB2::OK;
}

QString US_Analyte::get_filename(const QString& path, const QString& guid) {
  return US_DataFiles::get_filename(path, guid, "A", "analyte", "analyteGUID");
}

void US_Analyte::write_nucleotide(US_DB2* db) {
  QStringList q;
  q << "set_nucleotide_info" << analyteID;
  q << QString::number(doubleStranded);
  q << QString::number(complement);
  q << QString::number(_3prime);
  q << QString::number(_5prime);
  q << QString::number(sodium);
  q << QString::number(potassium);
  q << QString::number(lithium);
  q << QString::number(magnesium);
  q << QString::number(calcium);

  db->statusQuery(q);

  if (db->lastErrno() != US_DB2::OK)
    message = QObject::tr("Could not update nucleotide info");
}

void US_Analyte::dump(void) {
  switch (type) {
    case PROTEIN:
      qDebug() << "Type: PROTEIN";
      break;
    case DNA:
      qDebug() << "Type: DNA";
      break;
    case RNA:
      qDebug() << "Type: RNA";
      break;
    case CARBOHYDRATE:
      qDebug() << "Type: OTHER";
      break;
    default:
      qDebug() << "Type: **UNKNOWN**";
      break;
  }

  qDebug() << "msg   :" << message;
  qDebug() << "anaID :" << analyteID;

  qDebug() << "vbar20:" << vbar20;
  qDebug() << "mw    :" << mw;
  qDebug() << "descr :" << description;
  qDebug() << "guid  :" << analyteGUID;
  qDebug() << "seq   :" << sequence;

  qDebug() << "dblStr:" << doubleStranded;
  qDebug() << "comple:" << complement;
  qDebug() << "3prime:" << _3prime;
  qDebug() << "5prime:" << _5prime;
  qDebug() << "na    :" << sodium;
  qDebug() << "k     :" << potassium;
  qDebug() << "li    :" << lithium;
  qDebug() << "mg    :" << magnesium;
  qDebug() << "ca    :" << calcium;
  qDebug() << "grad_f:" << grad_form;

  qDebug() << "extinction";
  foreach (double wl, extinction.keys()) qDebug() << wl << extinction[wl];

  qDebug() << "refraction";
  foreach (double wl, refraction.keys()) qDebug() << wl << refraction[wl];

  qDebug() << "fluorescence";
  foreach (double wl, fluorescence.keys()) qDebug() << wl << fluorescence[wl];
}
