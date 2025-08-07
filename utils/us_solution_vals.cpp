//! \file us_solution_vals.cpp
#include "us_solution_vals.h"

#include "us_settings.h"

// Get Solution/Buffer values for a data set
bool US_SolutionVals::values(US_DB2* dbP, US_DataIO::EditedData* edata,
                             QString& soluID, QString& cvbar20,
                             QString& density, QString& viscosity,
                             QString& compress, QString& manual,
                             QString& errmsg) {
  bool got_ok = false;
  QString bufID;
  QString bguid;
  QString bdesc;

  errmsg =
      QObject::tr(" Solution/Buffer value determination for data set\n   ") +
      edata->runID + "." + edata->editID + "." + edata->cell + "." +
      edata->channel + "." + edata->wavelength + "  --\n";

  if (dbP != 0) {
    got_ok =
        solinfo_db(dbP, edata, cvbar20, soluID, bufID, bguid, bdesc, errmsg);
    got_ok = got_ok ? got_ok
                    : solinfo_disk(edata, cvbar20, soluID, bufID, bguid, bdesc,
                                   errmsg);
    got_ok = bufvals_db(dbP, bufID, bguid, bdesc, density, viscosity, compress,
                        manual, errmsg);
    got_ok = got_ok ? got_ok
                    : bufvals_disk(bufID, bguid, bdesc, density, viscosity,
                                   compress, manual, errmsg);
  }

  else {
    got_ok = solinfo_disk(edata, cvbar20, soluID, bufID, bguid, bdesc, errmsg);
    got_ok = bufvals_disk(bufID, bguid, bdesc, density, viscosity, compress,
                          manual, errmsg);
  }

  return got_ok;
}

// Get solution/buffer info from DB: ID, GUID, description
bool US_SolutionVals::solinfo_db(US_DB2* dbP, US_DataIO::EditedData* edata,
                                 QString& cvbar20, QString& soluID,
                                 QString& bufId, QString& bufGuid,
                                 QString& bufDesc, QString& errmsg) {
  bool bufinfo = false;

  QStringList query;
  QString rawGUID = edata->dataGUID;

  query << "get_rawDataID_from_GUID" << rawGUID;
  dbP->query(query);

  if (dbP->lastErrno() != US_DB2::OK) {
    errmsg += QObject::tr("  Unable to get raw data ID from GUID ") + rawGUID +
              "\n" + dbP->lastError() + "\n";

    return bufinfo;
  }

  dbP->next();
  QString rawID = dbP->value(0).toString();
  QString expID = dbP->value(1).toString();
  soluID = dbP->value(2).toString();

  query.clear();
  query << "get_solutionBuffer" << soluID;
  dbP->query(query);

  if (dbP->lastErrno() != US_DB2::OK) {
    query.clear();
    query << "get_solutionIDs" << expID;
    dbP->query(query);
    dbP->next();
    soluID = dbP->value(0).toString();

    query.clear();
    query << "get_solutionBuffer" << soluID;
    dbP->query(query);

    if (dbP->lastErrno() != US_DB2::OK) {
      errmsg += QObject::tr("Unable to get solutionBuffer from solution ID") +
                soluID + "\n" + dbP->lastError() + "\n";
    }
  }

  dbP->next();
  QString id = dbP->value(0).toString();
  QString guid = dbP->value(1).toString();
  QString desc = dbP->value(2).toString();

  if (!id.isEmpty()) {
    bufId = id;
    bufGuid = guid.isEmpty() ? bufGuid : guid;
    bufDesc = desc.isEmpty() ? bufDesc : desc;
    bufinfo = true;
  }

  query.clear();
  query << "get_solution" << soluID;
  dbP->query(query);

  if (dbP->lastErrno() != US_DB2::OK) {
    errmsg += QObject::tr("Unable to get solution vbar from solution ID") +
              soluID + "\n" + dbP->lastError() + "\n";
  } else {
    dbP->next();
    cvbar20 = dbP->value(2).toString();
  }

  return bufinfo;
}

// Get solution/buffer info from local disk: ID, GUID, description
bool US_SolutionVals::solinfo_disk(US_DataIO::EditedData* edata,
                                   QString& cvbar20, QString& soluID,
                                   QString& bufId, QString& bufGuid,
                                   QString& bufDesc, QString& errmsg) {
  bool bufinfo = false;
  QString soluGUID = "";

  QString exppath = US_Settings::resultDir() + "/" + edata->runID + "/" +
                    edata->runID + "." + edata->dataType.left(2) + ".xml";

  QFile filei(exppath);
  if (!filei.open(QIODevice::ReadOnly | QIODevice::Text)) {
    errmsg += QObject::tr("  Unable to open file ") + exppath + "\n";
    return bufinfo;
  }

  QXmlStreamReader xml(&filei);

  while (!xml.atEnd()) {
    xml.readNext();

    if (xml.isStartElement()) {
      QXmlStreamAttributes ats = xml.attributes();

      if (xml.name() == "buffer") {
        QString id = ats.value("id").toString();
        QString guid = ats.value("guid").toString();
        QString desc = ats.value("desc").toString();

        if (!id.isEmpty() || !guid.isEmpty()) {
          bufId = id.isEmpty() ? bufId : id;
          bufGuid = guid.isEmpty() ? bufGuid : guid;
          bufDesc = desc.isEmpty() ? bufDesc : desc;
          bufinfo = true;
          bufId = bufId.isEmpty() ? "N/A" : bufId;
        }
        break;
      }

      else if (xml.name() == "solution") {
        soluGUID = ats.value("guid").toString();
        soluID = soluGUID;
      }
    }
  }

  filei.close();

  if (!bufinfo &&
      !soluGUID.isEmpty()) {  // No buffer info yet, but solution GUID found:
                              // get buffer from solution
    QString spath = US_Settings::dataDir() + "/solutions/";
    QDir f(spath);

    QStringList filter("S*.xml");
    QStringList names = f.entryList(filter, QDir::Files, QDir::Name);
    QString fname;
    QString bdens;
    QString bvisc;

    for (int ii = 0; ii < names.size(); ii++) {
      fname = spath + names[ii];
      QFile filei(fname);

      if (!filei.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

      QXmlStreamReader xml(&filei);

      while (!xml.atEnd()) {
        xml.readNext();

        if (xml.isStartElement()) {
          QXmlStreamAttributes ats = xml.attributes();

          if (xml.name() == "solution") {
            QString sguid = ats.value("guid").toString();

            if (sguid != soluGUID) break;

            cvbar20 = ats.value("commonVbar20").toString();
          }

          else if (xml.name() == "buffer") {
            QString bid = ats.value("id").toString();
            QString bguid = ats.value("guid").toString();
            QString bdesc = ats.value("desc").toString();

            if (!bid.isEmpty() || !bguid.isEmpty()) {
              bufId = bid.isEmpty() ? bufId : bid;
              bufId = bufId.isEmpty() ? "N/A" : bufId;
              bufGuid = bguid.isEmpty() ? bufGuid : bguid;
              bufDesc = bdesc.isEmpty() ? bufDesc : bdesc;
              bufinfo = true;
            }
            break;
          }
        }
        if (bufinfo) break;
      }
      if (bufinfo) break;
    }
  }

  return bufinfo;
}

// Get buffer values from DB:  density, viscosity
bool US_SolutionVals::bufvals_db(US_DB2* dbP, QString& bufId, QString& bufGuid,
                                 QString& bufDesc, QString& density,
                                 QString& viscosity, QString& compress,
                                 QString& manual, QString& errmsg) {
  bool bufvals = false;

  QStringList query;

  if (bufId.isEmpty() && !bufGuid.isEmpty()) {
    query.clear();
    query << "get_bufferID" << bufGuid;
    dbP->query(query);

    if (dbP->lastErrno() != US_DB2::OK) {
      errmsg += QObject::tr("Unable to get buffer ID from buffer GUID ") +
                bufGuid + "\n" + dbP->lastError() + "\n";
    } else {
      dbP->next();
      bufId = dbP->value(0).toString();
      bufId = bufId.isEmpty() ? "N/A" : bufId;
    }
  }

  if (!bufId.isEmpty()) {
    query.clear();
    query << "get_buffer_info" << bufId;
    dbP->query(query);
    if (dbP->lastErrno() != US_DB2::OK) {
      errmsg +=
          QObject::tr("Unable to get buffer information from buffer ID ") +
          bufId + "\n" + dbP->lastError() + "\n";
      return bufvals;
    }

    dbP->next();
    QString ddesc = dbP->value(1).toString();
    QString dcomp = dbP->value(2).toString();
    QString dvisc = dbP->value(4).toString();
    QString ddens = dbP->value(5).toString();
    QString dmanu = dbP->value(6).toString();
    density = ddens.isEmpty() ? density : ddens;
    viscosity = dvisc.isEmpty() ? viscosity : dvisc;
    compress = dcomp.isEmpty() ? compress : dcomp;
    manual = dmanu.isEmpty() ? manual : ddesc.contains("  [M]") ? "1" : dmanu;
    bufvals = true;
  } else {
    QString invID = QString::number(US_Settings::us_inv_ID());
    query.clear();
    query << "get_buffer_desc" << invID;
    dbP->query(query);

    if (dbP->lastErrno() != US_DB2::OK) {
      errmsg +=
          QObject::tr("Unable to get buffer description for investigator ") +
          invID + "\n" + dbP->lastError() + "\n";
      return bufvals;
    }

    while (dbP->next()) {
      QString desc = dbP->value(1).toString();

      if (desc == bufDesc) {
        bufId = dbP->value(0).toString();
        break;
      }
    }

    if (!bufId.isEmpty()) {
      query.clear();
      query << "get_buffer_info" << bufId;
      dbP->query(query);

      if (dbP->lastErrno() != US_DB2::OK) {
        errmsg +=
            QObject::tr("Unable to get buffer information for buffer ID ") +
            bufId + "\n" + dbP->lastError() + "\n";
        return bufvals;
      }

      dbP->next();
      QString ddesc = dbP->value(1).toString();
      QString dcomp = dbP->value(2).toString();
      QString dvisc = dbP->value(4).toString();
      QString ddens = dbP->value(5).toString();
      QString dmanu = dbP->value(6).toString();
      density = ddens.isEmpty() ? density : ddens;
      viscosity = dvisc.isEmpty() ? viscosity : dvisc;
      compress = dcomp.isEmpty() ? compress : dcomp;
      manual = dmanu.isEmpty() ? manual : ddesc.contains("  [M]") ? "1" : dmanu;
      bufvals = true;
    }
  }

  return bufvals;
}

// Get buffer values from local disk:  density, viscosity
bool US_SolutionVals::bufvals_disk(QString& bufId, QString& bufGuid,
                                   QString& bufDesc, QString& density,
                                   QString& viscosity, QString& compress,
                                   QString& manual, QString& errmsg) {
  bool bufvals = false;
  bool dfound = false;
  QString bpath = US_Settings::dataDir() + "/buffers/";
  QDir f(bpath);

  QStringList filter("B*.xml");
  QStringList names = f.entryList(filter, QDir::Files, QDir::Name);
  QString fname;
  QString bdens;
  QString bvisc;
  QString bcomp;
  QString bmanu;
  QString bdesc;

  for (int ii = 0; ii < names.size(); ii++) {
    fname = bpath + names[ii];
    QFile filei(fname);

    if (!filei.open(QIODevice::ReadOnly | QIODevice::Text)) continue;

    QXmlStreamReader xml(&filei);

    while (!xml.atEnd()) {
      xml.readNext();

      if (xml.isStartElement() && xml.name() == "buffer") {
        QXmlStreamAttributes ats = xml.attributes();
        QString bid = ats.value("id").toString();
        QString bguid = ats.value("guid").toString();
        bdesc = ats.value("description").toString();
        bid = bid.isEmpty() ? "EMPTY" : bid;
        bguid = bguid.isEmpty() ? "EMPTY" : bguid;
        bdesc = bdesc.isEmpty() ? "EMPTY" : bdesc;

        if ((bufGuid != "EMPTY" && bguid == bufGuid) ||
            (bufGuid == "EMPTY" && bid == bufId)) {
          bdens = ats.value("density").toString();
          bvisc = ats.value("viscosity").toString();
          bcomp = ats.value("compressibility").toString();
          bmanu = ats.value("manual").toString();
          density = bdens.isEmpty() ? density : bdens;
          viscosity = bvisc.isEmpty() ? viscosity : bvisc;
          compress = bcomp.isEmpty() ? compress : bcomp;
          manual = bmanu.isEmpty()           ? manual
                   : bdesc.contains("  [M]") ? "1"
                                             : bmanu;
          bufvals = true;
        }

        else if (bdesc == bufDesc) {
          bdens = ats.value("density").toString();
          bvisc = ats.value("viscosity").toString();
          bcomp = ats.value("compressibility").toString();
          bmanu = ats.value("manual").toString();
          dfound = true;
        }

        break;
      }
    }

    if (bufvals) break;
  }

  if (!bufvals && dfound) {
    density = bdens.isEmpty() ? density : bdens;
    viscosity = bvisc.isEmpty() ? viscosity : bvisc;
    compress = bcomp.isEmpty() ? compress : bcomp;
    manual = bmanu.isEmpty() ? manual : bdesc.contains("  [M]") ? "1" : bmanu;
    bufvals = true;
  }

  if (!bufvals) {
    errmsg += QObject::tr(
                  "  Unable to find buffer values locally\n"
                  "   from buffer ID's (Id,Guid,Desc)\n     ") +
              (bufId.isEmpty() ? QObject::tr("(EMPTY)") : bufId) + "," +
              (bufGuid.isEmpty() ? QObject::tr("(EMPTY)") : bufGuid) + "," +
              (bufDesc.isEmpty() ? QObject::tr("(EMPTY)") : bufDesc) + "\n";
  }

  return bufvals;
}
