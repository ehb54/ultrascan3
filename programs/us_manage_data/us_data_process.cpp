//! \file us_data_process.cpp

#include "us_data_process.h"

#include "us_data_model.h"
#include "us_datafiles.h"
#include "us_settings.h"
#include "us_sync_exper.h"
#include "us_util.h"

// class to process operatations on data:  upload/download/remove
US_DataProcess::US_DataProcess(US_DataModel* dmodel, QWidget* parent /*=0*/) {
  parentw = parent;
  da_model = dmodel;
  db = da_model->dbase();
  dbg_level = US_Settings::us_debug();

  syncExper = new US_SyncExperiment(db, parent);
}

// perform a record upload to the database from local disk
int US_DataProcess::record_upload(int row) {
  DbgLv(1) << "REC_ULD: row" << row + 1;
  int stat = 0;

  cdesc = da_model->row_datadesc(row);

  if (!raw_ancestor_ok(
          row)) {  // Cannot upload if ancestor Raw is not in both DB and Local
    errMsg =
        tr("Cannot upload records whose ancestor"
           " Raw record is not both DB and Local.\n"
           "Use us_convert (Utilities->Convert Legacy Data)"
           " to create or modify the ancestor Raw.");
    stat = 3061;
    return stat;
  }

  QDateTime dtime = QDateTime::fromString(cdesc.lastmodDate);
  QDateTime ftime = QDateTime::fromString(cdesc.filemodDate);

  if ((cdesc.recState & US_DataModel::REC_DB) != 0 &&
      ftime < dtime) {  // Cannot replace DB record with older Local record
    errMsg = tr("The Local file record is older than the DB record.");
    stat = 3061;
    return stat;
  }

  QStringList query;
  QString filepath = cdesc.filename;
  QString pathdir = filepath.section("/", 0, -2);
  QString filename = filepath.section("/", -1, -1);
  int idData = cdesc.recordID;

  // if      ( cdesc.recType == 1 )
  if (cdesc.recType == 1 && cdesc.recType != 1)  // never upload raw
  {                                              // upload a Raw record
    QString runID = filename.section(".", 0, 0);
    QString tripl = filename.section(".", -4, -2).replace(".", " / ");
    DbgLv(1) << "REC_ULD:RAW: runID" << runID << "  tripl" << tripl;

    // stat = syncExper->synchronize( cdesc );
    DbgLv(1) << "REC_ULD:RAW: parentGUID" << cdesc.parentGUID;
    QString expGUID = cdesc.parentGUID;

    if (stat == 0) {
      idData = cdesc.recordID;
      stat = db->writeBlobToDB(filepath, QString("upload_aucData"), idData);
      errMsg = tr("Raw upload writeBlobToDB() status %1").arg(stat);
      stat = (stat == 0) ? 0 : 3041;
    }
  }

  else if (cdesc.recType == 2) {  // upload an EditedData record
    QString runID = filename.section(".", 0, 0);
    QString label = runID;
    QString comment = filename.section(".", 0, 2);
    QString editGUID = cdesc.dataGUID;
    QString rawGUID = cdesc.parentGUID;
    QString editID = QString::number(idData);

    query.clear();
    query << "get_rawDataID_from_GUID" << rawGUID;
    db->query(query);

    if ((stat = db->lastErrno()) != US_DB2::OK) {
      errMsg = tr("Raw DB record for Edit could not be accessed: %1").arg(stat);
      stat = 3052;
    }

    else {
      db->next();
      DbgLv(1) << "editUpld: rawGUID" << rawGUID;

      QString rawDataID = db->value(0).toString();

      DbgLv(1) << "editUpld: rawDataID" << rawDataID;

      if (idData < 0) {
        query.clear();
        query << "new_editedData" << rawDataID << editGUID << runID << filename
              << comment;
        db->query(query);
        idData = db->lastInsertID();
        editID = QString::number(idData);
        DbgLv(1) << "editUpld: NEW idData" << idData << editID;
      }

      query.clear();
      query << "update_editedData" << editID << rawDataID << editGUID << label
            << filename << comment;
      db->query(query);
      DbgLv(1) << "editUpld: label" << label;

      stat = db->writeBlobToDB(filepath, QString("upload_editData"), idData);
      errMsg = tr("writeBlob Edited stat %1, idData %2").arg(stat).arg(idData);
      stat = (stat == 0) ? 0 : 3042;
      cdesc.recordID = idData;
    }
  }

  else if (cdesc.recType == 3) {  // upload a Model record
    US_Model model;
    filepath = get_model_filename(cdesc.dataGUID);

    model.load(filepath);         // load model from local disk
    model.update_coefficients();  // fill in any missing coefficients
    stat = model.write(db);       // store model to database
    errMsg = tr("model write to DB stat %1").arg(stat);
    stat = (stat == 0) ? 0 : 3043;
    cdesc.recordID = db->lastInsertID();
    DbgLv(1) << errMsg;
  }

  else if (cdesc.recType == 4) {  // upload a Noise record
    US_Noise noise;
    DbgLv(2) << "NOISE: (1) filepath" << filepath;
    filepath = get_noise_filename(cdesc.dataGUID);
    DbgLv(2) << "NOISE: (2) filepath" << filepath;

    noise.load(filepath);
    stat = noise.write(db);
    errMsg = tr("noise write to DB stat %1").arg(stat);
    stat = (stat == 0) ? 0 : 3044;
    cdesc.recordID = db->lastInsertID();
  }

  else {  // *ERROR*:  invalid type
    errMsg = tr("upload attempt with type=%1").arg(cdesc.recType);
    stat = 3045;
  }

  if (stat == 0) {
    da_model->change_datadesc(cdesc, row);
  }

  return stat;
}

// perform a record download from the database to local disk
int US_DataProcess::record_download(int row) {
  int stat = 0;
  QStringList query;

  cdesc = da_model->row_datadesc(row);

  if (!raw_ancestor_ok(row)) {  // Cannot download if ancestor Raw is not in
                                // both DB and Local
    errMsg =
        tr("Cannot download records whose ancestor"
           " Raw record is not both DB and Local.\n"
           "Use us_convert (Utilities->Convert Legacy Data)"
           " to create or modify the ancestor Raw.");
    stat = 3061;
    return stat;
  }

  int idData = cdesc.recordID;
  QString dataID = QString::number(idData);
  QString filepath = cdesc.filename;
  QString dataGUID = cdesc.dataGUID;
  QString filename = filepath.section("/", -1, -1);
  QString runID = filename.section(".", 0, 0);

  filepath = (filepath == filename)
                 ? US_Settings::resultDir() + "/" + runID + "/" + filename
                 : filepath;

  QDir dirp;
  QString filedir = filepath.section("/", 0, -2);

  if (!dirp.exists(filedir)) {
    if (!dirp.mkpath(filedir)) {
      stat = 3091;
      errMsg = tr("Unable to create download directory\n") + filedir;
      return stat;
    }
  }

  // if      ( cdesc.recType == 1 )
  if (cdesc.recType == 1 && cdesc.recType != 1)  // never download raw
  {                                              // download a Raw record
    stat = db->readBlobFromDB(filepath, "download_aucData", idData);
    if (stat != 0)
      errMsg = tr("Raw download status %1").arg(stat) + "\n " + filepath;
  }

  else if (cdesc.recType == 2) {  // download an EditedData record
    stat = db->readBlobFromDB(filepath, "download_editData", idData);
    if (stat != 0)
      errMsg = tr("Edited download status %1").arg(stat) + "\n " + filepath;
  }

  else if (cdesc.recType == 3) {  // download a Model record
                                  // QTime timer3;
    // timer3.start();
    filepath = get_model_filename(&cdesc);
    // qDebug() << "DwnLd:  model time1" << timer3.elapsed();
    QStringList query;
    query << "get_model_info" << dataID;
    db->query(query);
    stat = db->lastErrno();
    if (stat == US_DB2::OK) {
      db->next();
      // qDebug() << "DwnLd:   model time2" << timer3.elapsed();
      QString contents = db->value(2).toString();
      QFile fileo(filepath);
      if (fileo.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream tso(&fileo);
        tso << contents;
        fileo.close();
        // qDebug() << "DwnLd:   model time3" << timer3.elapsed();
      }
    }

    else {
      errMsg = tr("Model load for download, status %1").arg(stat);
      stat = 3033;  // download read error
    }
  }

  else if (cdesc.recType == 4) {  // download a Noise record
    US_Noise noise;

    filepath = get_noise_filename(&cdesc);
    stat = noise.load(dataID, db);

    if (stat == US_DB2::OK) {
      stat = noise.write(filepath);

      if (stat != US_DB2::OK) {
        errMsg = tr("Noise write for download, status %1").arg(stat);
        stat = 3024;  // download write error
      }

      else
        cdesc.filename = filepath;
    }

    else {
      errMsg = tr("Noise load for download, status %1").arg(stat);
      stat = 3034;  // download read error
    }
  }

  else {  // *ERROR*:  invalid type
    errMsg = tr("download attempt with type=%1").arg(cdesc.recType);
    stat = 3015;
  }

  if (stat == 0) {  // Change the record state to include "in-DB"
    cdesc.recState |= US_DataModel::REC_DB;
    da_model->change_datadesc(cdesc, row);
  }

  return stat;
}
// perform a record remove from the database
int US_DataProcess::record_remove_db(int row) {
  int stat = 0;
  QStringList query;

  cdesc = da_model->row_datadesc(row);
  int irecID = cdesc.recordID;
  QString dataID = QString::number(irecID);

  if (cdesc.recType == 1) {  // remove a Raw record
    query.clear();
    query << "get_rawDataID_from_GUID" << cdesc.dataGUID;
    db->query(query);
    db->next();
    DbgLv(1) << "REC_RMV: rawGUID rawID expID" << cdesc.dataGUID
             << db->value(0).toString() << db->value(1).toString();
    query.clear();
    query << "delete_rawData" << QString::number(cdesc.parentID);
    DbgLv(1) << "REC_RMV: exp ID" << cdesc.parentID;

    if ((stat = db->statusQuery(query)) != 0) {
      errMsg = tr("delete_rawData status=%1").arg(stat);
      stat = 2011;
    } else
      irecID = -1;
  }

  else if (cdesc.recType == 2) {  // remove an EditedData record
    query.clear();
    query << "delete_editedData" << dataID;
    DbgLv(1) << "EDT_RMV:" << query;

    if ((stat = db->statusQuery(query)) != 0) {
      errMsg = tr("delete_editedData status=%1").arg(stat);
      DbgLv(1) << "EDT_RMV:   stat" << stat << db->lastError()
               << db->lastErrno();
      DbgLv(1) << "EDT_RMV:   dataID" << dataID << "errMsg:" << errMsg;
      stat = 2012;
    } else
      irecID = -1;
    DbgLv(1) << "EDT_RMV:    stat" << stat;
  }

  else if (cdesc.recType == 3) {  // remove a Model record
    query.clear();
    query << "delete_model" << dataID;

    if ((stat = db->statusQuery(query)) != 0) {
      errMsg = tr("delete_model status=%1").arg(stat);
      DbgLv(1) << "MOD_RMV:   dataID" << dataID << "errMsg:" << errMsg;
      DbgLv(1) << "MOD_RMV:    lastErr" << db->lastError() << db->lastErrno();
      stat = 2013;
    } else
      irecID = -1;
  }

  else if (cdesc.recType == 4) {  // remove a Noise record
    query.clear();
    query << "delete_noise" << dataID;

    if ((stat = db->statusQuery(query)) != 0) {
      errMsg = tr("delete_noise status=%1").arg(stat);
      DbgLv(1) << "NOI_RMV:   dataID" << dataID << "errMsg:" << errMsg;
      DbgLv(1) << "NOI_RMV:    lastErr" << db->lastError() << db->lastErrno();
      stat = 2014;
    } else
      irecID = -1;
  }

  else {  // *ERROR*:  invalid type
    errMsg = tr("DB remove attempt for type=%1").arg(cdesc.recType);
    stat = 2015;
  }

  if (stat == 0) {
    cdesc.recordID = irecID;
    da_model->change_datadesc(cdesc, row);
  }

  return stat;
}

// perform a record remove from local disk
int US_DataProcess::record_remove_local(int row) {
  int stat = 0;
  cdesc = da_model->row_datadesc(row);

  QString filepath = cdesc.filename;
  QString filename = filepath.section("/", -1, -1);
  QFile file(filepath);

  if (file.exists()) {  // the file to remove does exist

    if (file.remove()) {  // the remove was successful

      if ((cdesc.recState & US_DataModel::REC_DB) ==
          0) {  // it was local-only, so now it's a dummy
        cdesc.recState = US_DataModel::NOSTAT;
      }

      else {  // it was on both, so now it's db-only
        cdesc.recState &= ~US_DataModel::REC_LO;
      }
    }

    else {  // an error occurred in removing
      errMsg = tr("File remove error,\n") + filepath;
      stat = 1000;
    }
  }

  else {  // file did not exist
    errMsg = tr("*ERROR* attempt to remove non-existent file\n") + filepath;
    stat = 2000;
  }

  if (stat == 0) {
    da_model->change_datadesc(cdesc, row);
  }

  return stat;
}

// Get next model file name
QString US_DataProcess::get_model_filename(US_DataModel::DataDesc* ddesc) {
  QString path = ddesc->filename;

  if (!path.isEmpty() && QFile(path).exists()) return path;

  if ((ddesc->recState & US_DataModel::REC_LO) == 0)
    return get_model_filename("");
  else
    return get_model_filename(ddesc->dataGUID);
}

// Get next model file name
QString US_DataProcess::get_model_filename(QString guid) {
  QString path;

  if (!US_Model::model_path(path)) return "";

  return US_DataFiles::get_filename(path, guid, "M", "model", "modelGUID");
}

// Get next noise file name
QString US_DataProcess::get_noise_filename(US_DataModel::DataDesc* ddesc) {
  QString path = ddesc->filename;

  if (!path.isEmpty() && QFile(path).exists()) return path;

  if ((ddesc->recState & US_DataModel::REC_LO) == 0)
    return get_noise_filename("");
  else
    return get_noise_filename(ddesc->dataGUID);
}

// Get next noise file name
QString US_DataProcess::get_noise_filename(QString guid) {
  QString path = US_Settings::dataDir() + "/noises";
  QDir dir;

  if (!dir.exists(path)) {
    if (!dir.mkpath(path)) return "";
  }

  return US_DataFiles::get_filename(path, guid, "N", "noise", "noiseGUID");
}

// Return the last error message
QString US_DataProcess::lastError() { return errMsg; }

// Append to last error message a notice of errors on some actions
void US_DataProcess::partialError(int nacterrs, int nactions) {
  errMsg += tr("\n\n%1 error(s) on %2 attempted actions.\n")
                .arg(nacterrs)
                .arg(nactions);
}

// Append a string to the last error message
void US_DataProcess::appendError(QString aperrMsg) { errMsg += aperrMsg; }

// Test for ancestor Raw record in both DB and local
bool US_DataProcess::raw_ancestor_ok(int row) {
  bool isOK = false;
  int jrow = row;
  int bSrc = US_DataModel::REC_DB | US_DataModel::REC_LO;

  while (--jrow >= 0) {  // walk up tree until a Raw record is encountered.
    US_DataModel::DataDesc ddesc = da_model->row_datadesc(jrow);

    if (ddesc.recType == 1) {  // This is the Raw ancestor of the given record
      if ((ddesc.recState & bSrc) ==
          bSrc) {  // If both DB and Local, mark as OK for upload/download
        isOK = true;
      }
      break;
    }
  }

  return isOK;
}
