//! \file us_data_model.cpp

#include "us_data_model.h"
#include "us_data_process.h"
#include "us_data_tree.h"
#include "us_editor.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_settings.h"
#include "us_util.h"

#define timeFmt QString("hh:mm:ss")
#define nowTime() "T=" + QDateTime::currentDateTime().toString(timeFmt)

// Scan the database and local disk for R/E/M/N data sets
US_DataModel::US_DataModel(QWidget *parwidg /*=0*/) {
   parentw = parwidg; // parent (main manage_data) widget

   ddescs.clear(); // db descriptions
   ldescs.clear(); // local descriptions
   adescs.clear(); // all descriptions
   chgrows.clear(); // changed rows

   dbg_level = US_Settings::us_debug();
}

// Set database related pointers
void US_DataModel::setDatabase(US_DB2 *a_db) {
   db = a_db; // pointer to opened db connection
   invID = QString::number(US_Settings::us_inv_ID());
   DbgLv(1) << "DMod:setDB: invID" << invID;
}

// Set progress bar related pointers
void US_DataModel::setProgress(QProgressBar *a_progr, QLabel *a_lbstat) {
   progress = a_progr; // pointer to progress bar
   lb_status = a_lbstat; // pointer to status label
}

// set sibling classes pointers
void US_DataModel::setSiblings(QObject *a_proc, QObject *a_tree) {
   ob_process = a_proc; // pointer to sister DataProcess object
   ob_tree = a_tree; // pointer to sister DataTree object
}

// Get database pointer
US_DB2 *US_DataModel::dbase() {
   return db;
}

// Get progress bar pointer
QProgressBar *US_DataModel::progrBar() {
   return progress;
}

// Get status label pointer
QLabel *US_DataModel::statlab() {
   return lb_status;
}

// get us_data_process object pointer
QObject *US_DataModel::procobj() {
   return ob_process;
}

// get us_data_tree object pointer
QObject *US_DataModel::treeobj() {
   return ob_tree;
}

// Scan the database and local for run IDs then return to caller
void US_DataModel::getRunIDs(QStringList &runIDs, int &source) {
   runIDs.clear();

   // Get a list of runIDs from the database (unless Source==Local)
   if (source != 2) {
      QStringList query;
      query << "get_experiment_desc" << invID;
      db->query(query);

      while (db->next()) {
         QString runID = db->value(1).toString();

         if (!runIDs.contains(runID))
            runIDs << runID;
      }
   }
   DbgLv(1) << "gRI: db runs" << runIDs.size();

   // Add any local runIDs not already represented (unless Source==DB)
   if (source != 1) {
      QString rdir = US_Settings::resultDir() + "/";
      QStringList aucdirs = QDir(rdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
      QStringList aucfilt;
      aucfilt << "*.auc";
      DbgLv(1) << "gRI: aucdirs" << aucdirs.size();

      for (int ii = 0; ii < aucdirs.size(); ii++) { // Loop thru potential data directories; add any new with AUC content
         QString aucdir = aucdirs.at(ii);
         QString subdir = rdir + aucdir;
         QStringList aucfiles = QDir(subdir).entryList(aucfilt, QDir::Files, QDir::Name);
         int naucf = aucfiles.size();
         QString runID = aucdir.section(".", 0, 0);

         if (naucf > 0) { // Possibly add if run has AUC content
            if (!runIDs.contains(runID) && source != 4) { // This run not in local and exclude-DB-only not specified
               runIDs << runID;
            }
         }
      }
      DbgLv(1) << "gRI: db+local runs" << runIDs.size();
   }

   runIDs.sort();
}

// Scan the database and local for triples in a runID then return to caller
void US_DataModel::getTriples(QStringList &triples, QString runID) {
   triples.clear();

   // Browse raw data with matching runID to accumulate triples
   QStringList query;
   query << "get_experiment_info_by_runID" << runID << invID;
   db->query(query);
   db->next();
   QString expID = db->value(1).toString();

   query.clear();
   query << "get_rawDataIDs" << expID;
   db->query(query);
   DbgLv(1) << "gTr: runID" << runID << "expID" << expID;

   while (db->next()) {
      QString fname = db->value(2).toString().section("/", -1, -1);
      QString rrID = fname.section(".", 0, 0);
      //DbgLv(1) << "gTr:  rrID" << rrID << "fname" << fname;

      if (rrID == runID) { // Matching runID, so add triple (if need be)
         QString triple = fname.section(".", -4, -2);

         if (!triples.contains(triple))
            triples << triple;
      }
   }
   DbgLv(1) << "gTr: db triples" << triples.size();

   // Add any local triples not already represented
   QString aucdir = US_Settings::resultDir() + "/" + runID + "/";
   DbgLv(1) << "gTr: aucdir" << aucdir;

   QStringList aucfilt;
   aucfilt << runID + "*.auc";
   QStringList aucfiles = QDir(aucdir).entryList(aucfilt, QDir::Files, QDir::Name);
   int naucf = aucfiles.size();
   DbgLv(1) << "gTr:  naucf" << naucf;

   for (int ii = 0; ii < naucf; ii++) { // Loop thru files to add any new triples found
      QString fname = aucfiles.at(ii);
      QString triple = fname.section(".", -4, -2);

      if (!triples.contains(triple))
         triples << triple;
   }
   DbgLv(1) << "gTr: db+local triples" << triples.size();

   triples.sort();
}

// Set run and triple filters
void US_DataModel::setFilters(QString a_runf, QString a_tripf, QString a_srcf) {
   if (filt_run != a_runf || filt_triple != a_tripf || filt_source != a_srcf)
      chgrows.clear(); // Reset changed rows if any filters changed

   filt_run = a_runf; // Filter string for runID
   filt_triple = a_tripf; // Filter string for triple
   filt_source = a_srcf; // Filter string for source (DB/local)
}

// Scan the database and local disk for R/E/M/N data sets
void US_DataModel::scan_data() {
   DbgLv(1) << "ScnD: start scan   " << nowTime();
   scan_dbase(); // Read db to build db descriptions
   DbgLv(1) << "ScnD: DB scan done " << nowTime();

   sort_descs(ddescs); // Sort db descriptions
   DbgLv(1) << "ScnD: DB sort done " << nowTime();

   scan_local(); // Read files to build local descriptions
   DbgLv(1) << "ScnD: Lcl scan done" << nowTime();

   sort_descs(ldescs); // Sort local descriptions
   DbgLv(1) << "ScnD: Lcl sort done" << nowTime();

   merge_dblocal(); // Merge database and local descriptions
   DbgLv(1) << "ScnD: Merge done   " << nowTime();

   if (filt_source.startsWith("Exclude")) {
      exclude_trees(); // Exclude DB-Only or Local-Only trees
   }
}

// Get data description object at specified row
US_DataModel::DataDesc US_DataModel::row_datadesc(int irow) {
   return adescs.at(irow);
}

// Get current data description object
US_DataModel::DataDesc US_DataModel::current_datadesc() {
   return cdesc;
}

// Change data description object at a specified row
void US_DataModel::change_datadesc(DataDesc ddesc, int row) {
   adescs[ row ] = ddesc;
   cdesc = ddesc;
   chgrows << row;
}

// Set current data description object
void US_DataModel::setCurrent(int irow) {
   cdesc = adescs.at(irow);
}

// get count of total data records
int US_DataModel::recCount() {
   return adescs.size();
}

// get count of DB data records
int US_DataModel::recCountDB() {
   return ddescs.size();
}

// get count of local data records
int US_DataModel::recCountLoc() {
   return ldescs.size();
}

// scan the database for R/E/M/N data sets
void US_DataModel::scan_dbase() {
   const int max_qrec = 200;
   QStringList rawIDs;
   QStringList edtIDs;
   QStringList modIDs;
   QStringList noiIDs;
   QStringList modDescs;
   QStringList query;
   QMap<QString, int> edtMap;
   QMap<QString, QString> rawGUIDs;
   QString dmyGUID = "00000000-0000-0000-0000-000000000000";
   QString recID;
   QString rawGUID;
   QString contents;
   int irecID;
   int istep = 0;
   int nstep = 1;
   progress->setMaximum(nstep);
   progress->setValue(istep);

   if (!filt_source.isEmpty() && filt_source == "Local Only") { // If source filter is "Local-only", skip DB scan
      ddescs.clear();
      return;
   }

   if (chgrows.size() > 0) { // If changes since last scan, just modify existing db descriptions
      review_dbase();
      return;
   }

   lb_status->setText(tr("Reading DataBase Data..."));
   qApp->processEvents();
   ddescs.clear();

   if (dbg_level > 1) {
      query.clear();
      query << "get_experiment_desc" << invID;
      db->query(query);
      QStringList expIDs;

      while (db->next()) {
         QString expID = db->value(0).toString();
         QString runID = db->value(1).toString();
         QString etype = db->value(2).toString();
         expIDs << expID;
         DbgLv(2) << " expID runID type" << expID << runID << etype;
      }

      lb_status->setText(tr("Reading Experiments"));
      DbgLv(2) << "  expID expGUID runID label comment date";
      qApp->processEvents();

      for (int ii = 0; ii < expIDs.size(); ii++) {
         QString expID = expIDs[ ii ];
         query.clear();
         query << "get_experiment_info" << expID;
         db->query(query);
         db->next();
         QString expGUID = db->value(0).toString();
         QString runID = db->value(2).toString();
         QString label = db->value(9).toString();
         QString comment = db->value(10).toString();
         QString date = US_Util::toUTCDatetimeText(db->value(12).toDateTime().toString(Qt::ISODate), true);
         DbgLv(2) << "  " << expID << expGUID << runID << label << comment << date;
      }
   }
   QDateTime basetime = QDateTime::currentDateTime();

   // Count raws, edits, models, noises
   rawIDs.clear();
   edtIDs.clear();
   modIDs.clear();
   noiIDs.clear();
   bool rfilt = (!filt_run.isEmpty() && filt_run != "ALL");
   bool tfilt = (!filt_triple.isEmpty() && filt_triple != "ALL");

   QString expID;
   QString expGUID;
   int nraws = 0;
   int nedts = 0;
   int nmods = 0;
   int nnois = 0;

   if (rfilt) { // Count records when run/triple filtering
      DbgLv(1) << "BrDb:  filt'd Count start" << nowTime();
      query.clear();
      query << "get_experiment_info_by_runID" << filt_run << invID;
      db->query(query);
      db->next();
      expID = db->value(1).toString();
      expGUID = db->value(2).toString();

      query.clear();
      query << "count_rawData_by_experiment" << expID;
      nraws = db->functionQuery(query);
      DbgLv(1) << "BrDb: nraws" << nraws;
      rawIDs.reserve(nraws);

      DbgLv(1) << "BrDb:  Count raws" << nowTime();
      query.clear();
      query << "get_rawDataIDs" << expID;
      db->query(query);
      nraws = 0;
      while (db->next()) {
         QString rawID = db->value(0).toString();
         QString filename = db->value(2).toString().replace("\\", "/");
         QString filebase = filename.section("/", -1, -1);
         QString triple = filebase.section(".", -4, -2);
         if (tfilt && triple != filt_triple)
            continue;
         rawIDs << rawID;
         nraws++;
      }
      DbgLv(1) << "BrDb: nraws" << nraws;

      DbgLv(1) << "BrDb:  Count edits" << nowTime();
      if (nraws < max_qrec) {
         for (int ii = 0; ii < nraws; ii++) {
            QString rawID = rawIDs[ ii ];
            query.clear();
            query << "get_editedDataIDs" << rawID;
            db->query(query);
            while (db->next()) {
               QString edtID = db->value(0).toString();
               QString filename = db->value(2).toString().replace("\\", "/");
               QString filebase = filename.section("/", -1, -1);
               QString triple = filebase.section(".", -4, -2);
               if (tfilt && triple != filt_triple)
                  continue;
               edtIDs << edtID;
               nedts++;
            }
         }
      }

      else {
         query.clear();
         query << "all_editedDataIDs" << invID;
         db->query(query);
         while (db->next()) {
            QString edtID = db->value(0).toString();
            QString expIDed = db->value(4).toString();
            if (expIDed != expID)
               continue;
            QString filename = db->value(2).toString().replace("\\", "/");
            QString filebase = filename.section("/", -1, -1);
            QString triple = filebase.section(".", -4, -2);
            if (tfilt && triple != filt_triple)
               continue;
            edtIDs << edtID;
            nedts++;
         }
      }
      DbgLv(1) << "BrDb: nedts" << nedts;

      DbgLv(1) << "BrDb:  Count models,noises" << nowTime();
      if (nedts < max_qrec) {
         for (int ii = 0; ii < nedts; ii++) {
            QString edtID = edtIDs[ ii ];
            query.clear();
            query << "get_model_desc_by_editID" << invID << edtID;
            db->query(query);
            while (db->next()) {
               QString modID = db->value(0).toString();
               modIDs << modID;
               nmods++;
            }
            query.clear();
            query << "get_noise_desc_by_editID" << invID << edtID;
            db->query(query);
            while (db->next()) {
               QString noiID = db->value(0).toString();
               noiIDs << noiID;
               nnois++;
            }
         }
      }

      else {
         query.clear();
         query << "get_model_desc" << invID;
         db->query(query);
         while (db->next()) {
            QString modID = db->value(0).toString();
            QString edtID = db->value(6).toString();
            if (edtIDs.contains(edtID)) {
               modIDs << modID;
               nmods++;
            }
         }
         query.clear();
         query << "get_noise_desc" << invID;
         db->query(query);
         while (db->next()) {
            QString noiID = db->value(0).toString();
            QString edtID = db->value(2).toString();
            if (edtIDs.contains(edtID)) {
               noiIDs << noiID;
               nnois++;
            }
         }
      }
      DbgLv(1) << "BrDb: nmods" << nmods << "nnois" << nnois;
   }

   else { // Count records when not run/triple filtering
      query.clear();
      query << "count_rawData" << invID;
      nraws = db->functionQuery(query);
      DbgLv(1) << "BrDb: nraws" << nraws;

      query.clear();
      query << "count_editedData" << invID;
      nedts = db->functionQuery(query);
      DbgLv(1) << "BrDb: nedts" << nedts;

      query.clear();
      query << "count_models" << invID;
      nmods = db->functionQuery(query);
      DbgLv(1) << "BrDb: nmods" << nmods;

      query.clear();
      query << "count_noise" << invID;
      nnois = db->functionQuery(query);
      DbgLv(1) << "BrDb: nnois" << nnois;
   }

   nstep = nraws + nedts + nmods + nnois;
   int incre = nraws + nedts;
   DbgLv(1) << "BrDb:  nstep" << nstep << "incre" << incre;
   incre = qMax(incre, 1);
   incre = (nmods + nnois + incre - 1) / (incre * 4);
   incre = qMax(incre, 1);
   DbgLv(1) << "BrDb:   incre" << incre;
   nstep += (nraws + nedts) * (incre - 1);
   nstep = qMax(nstep, 1);
   istep = 0;
   DbgLv(1) << "BrDb:   nstep" << nstep;
   //nstep=(nstep<1)?1000:nstep;
   progress->setMaximum(nstep);
   progress->setValue(istep);
   qApp->processEvents();
   DbgLv(1) << "BrDb: # steps raws edts mods nois" << nstep << nraws << nedts << nmods << nnois << "incre" << incre;
   DbgLv(1) << "BrDb:  count time:" << basetime.msecsTo(QDateTime::currentDateTime()) / 1000.0;

   if (!rfilt) {
      rawIDs.reserve(nraws);
      edtIDs.reserve(nedts);
      modIDs.reserve(nmods);
      noiIDs.reserve(nnois);
   }

   modDescs.reserve(nmods);

   // get raw data IDs
   lb_status->setText(tr("Reading Raws"));
   qApp->processEvents();
   int nqry = rfilt ? rawIDs.size() : qMin(nraws, 1);
   bool rfilt_q = rfilt && (nqry < max_qrec);
   nqry = rfilt_q ? nqry : qMin(nraws, 1);

   DbgLv(1) << "BrDb:  Query Raws" << nowTime() << "nqry" << nqry;
   for (int jq = 0; jq < nqry; jq++) {
      QString rawID;
      query.clear();
      if (rfilt_q) {
         rawID = rawIDs[ jq ];
         query << "get_rawData" << rawID;
      }
      else {
         query << "all_rawDataIDs" << invID;
      }
      db->query(query);

      while (db->next()) { // Read Raw records
         recID = db->value(0).toString();
         QString label = db->value(1).toString();
         QString filename = db->value(2).toString().replace("\\", "/");
         QString filebase = filename.section("/", -1, -1);
         QString runID = filebase.section(".", 0, 0);
         QString triple = filebase.section(".", -4, -2);

         if (rfilt && runID != filt_run)
            continue;
         if (tfilt && triple != filt_triple)
            continue;

         QString experID;
         QString date;
         QString cksum;
         QString recsize;
         QString comment;

         if (rfilt_q) {
            recID = rawID;
            experID = db->value(4).toString();
            date = US_Util::toUTCDatetimeText(db->value(7).toDateTime().toString(Qt::ISODate), true);
            cksum = db->value(8).toString();
            recsize = db->value(9).toString();
            rawGUID = db->value(0).toString();
            comment = db->value(3).toString();
         }

         else {
            experID = db->value(3).toString();
            date = US_Util::toUTCDatetimeText(db->value(5).toDateTime().toString(Qt::ISODate), true);
            cksum = db->value(6).toString();
            recsize = db->value(7).toString();
            rawGUID = db->value(9).toString();
            comment = db->value(10).toString();
            expGUID = db->value(11).toString();
         }

         rawGUIDs[ recID ] = rawGUID;
         irecID = recID.toInt();
         DbgLv(1) << "BrDb: RAW id" << recID << " expID" << experID;
         QString subType = "";
         contents = cksum + " " + recsize;

         if (comment.isEmpty())
            comment = filename.section(".", 0, -2);

         if (!label.contains("."))
            label = filename.section(".", 0, -2);

         if (!rfilt)
            rawIDs << recID;

         DbgLv(2) << "BrDb:     raw expGid" << expGUID;
         DbgLv(2) << "BrDb:      label filename comment" << label << filename << comment;

         //DbgLv(2) << "BrDb:       (R)contents" << contents;

         cdesc.recordID = irecID;
         cdesc.recType = 1;
         cdesc.subType = subType;
         cdesc.recState = REC_DB;
         cdesc.dataGUID = rawGUID.simplified();
         cdesc.parentGUID = expGUID.simplified();
         cdesc.parentID = experID.toInt();
         cdesc.filename = filename;
         cdesc.contents = contents;
         cdesc.label = label;
         cdesc.description = comment;
         cdesc.filemodDate = "";
         cdesc.lastmodDate = date;

         if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
            cdesc.dataGUID = US_Util::new_guid();

         cdesc.parentGUID = cdesc.parentGUID.length() == 36 ? cdesc.parentGUID : dmyGUID;

         ddescs << cdesc;
         istep += incre;
         progress->setValue(istep);
         qApp->processEvents();
      }
   }

   int kraw = rawIDs.size();
   if (rfilt && kraw == 1) {
      nstep /= qMax(nraws, 1);
      progress->setMaximum(nstep);
   }
   // get edited data IDs
   lb_status->setText(tr("Reading Edits"));
   qApp->processEvents();
   nqry = rfilt ? edtIDs.size() : qMin(nedts, 1);
   rfilt_q = rfilt && (nqry < max_qrec);
   nqry = rfilt_q ? nqry : qMin(nedts, 1);

   DbgLv(1) << "BrDb:  Query Edits" << nowTime() << "nqry" << nqry;
   for (int jq = 0; jq < nqry; jq++) {
      QString edtID;
      query.clear();
      if (rfilt_q) {
         edtID = edtIDs[ jq ];
         query << "get_editedData" << edtID;
      }
      else {
         query << "all_editedDataIDs" << invID;
      }
      db->query(query);

      while (db->next()) { // Read Edit records
         QString label;
         QString filename;
         QString filebase;
         QString runID;
         QString triple;
         QString experID;
         QString date;
         QString cksum;
         QString recsize;
         QString comment;
         QString rawID;
         QString editGUID;

         if (rfilt_q) {
            recID = edtID;
            rawID = db->value(0).toString();
            editGUID = db->value(1).toString();
            label = db->value(2).toString();
            filename = db->value(3).toString().replace("\\", "/");
            filebase = filename.section("/", -1, -1);
            date = US_Util::toUTCDatetimeText(db->value(5).toDateTime().toString(Qt::ISODate), true);
            comment = db->value(4).toString();
            cksum = db->value(6).toString();
            recsize = db->value(7).toString();
            runID = filebase.section(".", 0, 0);
            triple = filebase.section(".", -4, -2);
            if (runID != filt_run)
               continue;
            if (tfilt && triple != filt_triple)
               continue;
         }
         else {
            recID = db->value(0).toString();
            label = db->value(1).toString();
            filename = db->value(2).toString().replace("\\", "/");
            filebase = filename.section("/", -1, -1);
            rawID = db->value(3).toString();
            expID = db->value(4).toString();
            date = US_Util::toUTCDatetimeText(db->value(5).toDateTime().toString(Qt::ISODate), true);
            cksum = db->value(6).toString();
            recsize = db->value(7).toString();
            editGUID = db->value(9).toString();
            comment = "";
            runID = filebase.section(".", 0, 0);
            triple = filebase.section(".", -4, -2);
            if (rfilt && runID != filt_run)
               continue;
            if (tfilt && triple != filt_triple)
               continue;
         }

         irecID = recID.toInt();
         DbgLv(2) << "BrDb: EDT id" << recID << " raID" << db->value(3).toString() << " expID"
                  << db->value(4).toString();
         rawGUID = rawGUIDs[ rawID ];

         if (!rfilt)
            edtIDs << recID;

         QString subType = filebase.section(".", 2, 2);
         contents = cksum + " " + recsize;
         DbgLv(2) << "BrDb:     edt  id eGID rGID label date" << irecID << editGUID << rawGUID << label << date;
         //DbgLv(2) << "BrDb:       (E)contents" << contents;

         if (!filename.contains("/"))
            filename = US_Settings::resultDir() + "/" + filename.section(".", 0, 0) + "/" + filename;
         //DbgLv(2) << "BrDb:       fname" << filename;

         cdesc.recordID = irecID;
         cdesc.recType = 2;
         cdesc.subType = subType;
         cdesc.recState = REC_DB;
         cdesc.dataGUID = editGUID.simplified();
         cdesc.parentGUID = rawGUID.simplified();
         cdesc.parentID = rawID.toInt();
         cdesc.filename = filename;
         cdesc.contents = contents;
         cdesc.description = (comment.isEmpty()) ? filebase.section(".", 0, 2) : comment;
         cdesc.label = cdesc.description;
         cdesc.filemodDate = "";
         cdesc.lastmodDate = date;

         if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
            cdesc.dataGUID = US_Util::new_guid();

         cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;
         edtMap[ cdesc.dataGUID ] = cdesc.recordID; // save edit ID for GUID

         ddescs << cdesc;
         istep += incre;
         progress->setValue(istep);
         qApp->processEvents();
      }
   }
   DbgLv(1) << "BrDb: EDT loop done";

   // get model IDs
   const int _M_LARGE_ = 65000; // Model large size indicating CUSTOMGRID
   QStringList tmodels;
   QList<int> tmodnxs;
   lb_status->setText(tr("Reading Models"));
   progress->setValue(istep);
   qApp->processEvents();
   DbgLv(1) << "BrDb: Reading Models";
   nqry = rfilt ? edtIDs.size() : 1;
   rfilt_q = rfilt && (nqry < max_qrec);
   nqry = rfilt_q ? nqry : qMin(nedts, 1);

   DbgLv(1) << "BrDb:  Query Models" << nowTime();
   for (int jq = 0; jq < nqry; jq++) {
      query.clear();

      if (rfilt_q)
         query << "get_model_desc_by_editID" << invID << edtIDs[ jq ];
      else
         query << "get_model_desc" << invID;

      DbgLv(2) << "BrDb:  Query Models" << nowTime();
      int kmdl = 0;
      db->query(query);
      DbgLv(2) << "BrDb:  Query Return" << nowTime();

      while (db->next()) { // get model information from DB
         recID = db->value(0).toString();
         QString editID = db->value(6).toString();
         if ((++kmdl) == 1)
            DbgLv(2) << "BrDb:  First Model " << nowTime();
         if (rfilt) {
            if (!edtIDs.contains(editID))
               continue;
         }
         else
            modIDs << recID;

         irecID = recID.toInt();
         QString modelGUID = db->value(1).toString();
         QString descript = db->value(2).toString();
         modDescs << descript;

         if (descript.length() == 80) { // Truncated description?  save for later testing/replacement
            tmodels << recID;
            tmodnxs << ddescs.size();
         }

         QString editGUID = db->value(5).toString();
         DbgLv(2) << "BrDb: MOD id" << recID << " edID" << editID << " edGID" << editGUID;
         DbgLv(2) << "BrDb: MOD id" << recID << " desc" << descript;
         QString date = US_Util::toUTCDatetimeText(db->value(7).toDateTime().toString(Qt::ISODate), true);
         QString cksum = db->value(8).toString();
         QString recsize = db->value(9).toString();
         QString label = descript.section(".", 0, -2);

         if (label.length() > 40)
            label = label.left(13) + "..." + label.right(24);

         // Get the sub-analysis-type
         QString subType = descript.section(".", -2, -2).section("_", 2, 2);

         // Set as CUSTOMGRID if so marked or large non-MC
         if (descript.contains("CustomGrid") || (!descript.contains("_mc") && recsize.toInt() > _M_LARGE_))
            subType = "CUSTOMGRID";

         // If empty subtype, mark as MANUAL
         else if (subType.isEmpty())
            subType = "MANUAL";

         contents = cksum + " " + recsize;
         //DbgLv(2) << "BrDb:         det: cont" << contents;
         cdesc.recordID = irecID;
         cdesc.recType = 3;
         cdesc.subType = subType;
         cdesc.recState = REC_DB;
         cdesc.dataGUID = modelGUID.simplified();
         cdesc.parentGUID = editGUID;
         cdesc.parentID = edtMap[ editGUID ];
         cdesc.filename = "";
         cdesc.contents = contents;
         cdesc.label = label;
         cdesc.description = descript;
         cdesc.filemodDate = "";
         cdesc.lastmodDate = date;

         if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
            cdesc.dataGUID = US_Util::new_guid();

         cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;

         ddescs << cdesc;
         progress->setValue(++istep);
         qApp->processEvents();
      }
   }
   DbgLv(2) << "BrDb:  Last  Model " << nowTime();

   // Get noise IDs
   QStringList tnoises;
   QList<int> tnoinxs;
   lb_status->setText(tr("Reading Noises"));
   qApp->processEvents();

   for (int jq = 0; jq < nqry; jq++) {
      query.clear();

      if (rfilt_q)
         query << "get_noise_desc_by_editID" << invID << edtIDs[ jq ];
      else
         query << "get_noise_desc" << invID;

      DbgLv(2) << "BrDb:  Query Noises" << nowTime();
      db->query(query);

      while (db->next()) { // Get noise information from DB
         recID = db->value(0).toString();
         irecID = recID.toInt();
         QString editID = db->value(2).toString();
         if (rfilt) {
            if (!edtIDs.contains(editID))
               continue;
         }
         else
            noiIDs << recID;

         QString noiseGUID = db->value(1).toString();
         QString modelID = db->value(3).toString();
         QString noiseType = db->value(4).toString();
         QString modelGUID = db->value(5).toString();
         QString date = US_Util::toUTCDatetimeText(db->value(6).toDateTime().toString(Qt::ISODate), true);
         QString cksum = db->value(7).toString();
         QString recsize = db->value(8).toString();
         QString descript = db->value(9).toString();
         DbgLv(2) << "BrDb: NOI id" << recID << " edID" << editID << " moID" << modelID << " descript" << descript;

         if (descript.isEmpty() || descript.length() == 80) {
            int jmod = modIDs.indexOf(modelID);
            if (jmod >= 0) {
               descript = modDescs.at(jmod);

               if (descript.length() == 80) { // Truncated description?  Save for later review/replace
                  tnoises << recID;
                  tnoinxs << ddescs.size();
               }

               descript = descript.replace(".model", "." + noiseType);
               DbgLv(2) << "BrDb:     jmod" << jmod << " descript" << descript;
            }
         }
         //DbgLv(3) << "BrDb: contents================================================";
         //DbgLv(3) << contents.left( 200 );
         //DbgLv(3) << "BrDb: contents================================================";

         contents = cksum + " " + recsize;
         QString label = descript.section(".", 0, -2);

         if (label.length() > 40)
            label = label.left(13) + "..." + label.right(24);

         cdesc.recordID = irecID;
         cdesc.recType = 4;
         cdesc.subType = (noiseType == "ti_noise") ? "TI" : "RI";
         cdesc.recState = REC_DB;
         cdesc.dataGUID = noiseGUID.simplified();
         cdesc.parentGUID = modelGUID.simplified();
         cdesc.parentID = modelID.toInt();
         cdesc.filename = "";
         cdesc.contents = contents;
         cdesc.label = label;
         cdesc.description = descript;
         cdesc.filemodDate = "";
         cdesc.lastmodDate = date;
         DbgLv(2) << "BrDb:       noi id nGID dsc typ noityp" << irecID << noiseGUID << descript << cdesc.subType
                  << noiseType;

         if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
            cdesc.dataGUID = US_Util::new_guid();

         cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;

         ddescs << cdesc;
         progress->setValue(++istep);
         qApp->processEvents();
      }
      DbgLv(2) << "BrDb:  Noise IDs" << nowTime() << "size" << noiIDs.size();
   }

   for (int ii = 0; ii < tmodels.size(); ii++) { // Change truncated model descriptions
      recID = tmodels[ ii ];
      int jdsc = tmodnxs[ ii ];
      cdesc = ddescs.at(jdsc);
      US_Model model1;
      model1.load(recID, db);
      QString descript = model1.description;
      QString label = descript.section(".", 0, -2);

      if (label.length() > 40)
         label = label.left(13) + "..." + label.right(24);

      DbgLv(2) << "BrDb:   ii jdsc" << ii << jdsc << "dsc1" << cdesc.description << "dsc2" << descript;
      cdesc.description = descript;
      cdesc.label = label;
      ddescs.replace(jdsc, cdesc);
   }

   for (int ii = 0; ii < tnoises.size(); ii++) { // Change truncated noise descriptions
      recID = tnoises[ ii ];
      int jdsc = tnoinxs[ ii ];
      cdesc = ddescs.at(jdsc);
      US_Noise noise1;
      noise1.load(recID, db);
      QString descript = noise1.description;
      QString label = descript.section(".", 0, -2);

      if (label.length() > 40)
         label = label.left(13) + "..." + label.right(24);

      DbgLv(2) << "BrDb:   ii jdsc" << ii << jdsc << "dsc1" << cdesc.description << "dsc2" << descript;
      cdesc.description = descript;
      cdesc.label = label;
      ddescs.replace(jdsc, cdesc);
   }


   progress->setMaximum(nstep);
   qApp->processEvents();
   DbgLv(1) << "BrDb: kr ke km kn" << rawIDs.size() << edtIDs.size() << modIDs.size() << noiIDs.size();
   DbgLv(1) << "BrDb:  scan time:" << basetime.msecsTo(QDateTime::currentDateTime()) / 1000.0;

   progress->setValue(nstep);
   lb_status->setText(tr("Database Review Complete"));
   qApp->processEvents();
}

// scan the local disk for R/E/M/N data sets
void US_DataModel::scan_local() {
   ldescs.clear(); // local descriptions
   adescs.clear(); // all descriptions

   if (!filt_source.isEmpty() && filt_source == "DB Only") { // If source filter is "DB-only", skip Local scan
      return;
   }

   // start with AUC (raw) and edit files in directories of resultDir
   bool rfilt = (!filt_run.isEmpty() && filt_run != "ALL");
   bool tfilt = (!filt_triple.isEmpty() && filt_triple != "ALL");
   QString rdir = US_Settings::resultDir();
   //               rdir     = rfilt ? ( rdir + "/" + filt_run ) : rdir;
   QString ddir = US_Settings::dataDir();
   QString dirm = ddir + "/models";
   QString dirn = ddir + "/noises";
   QString contents = "";
   QString dmyGUID = "00000000-0000-0000-0000-000000000000";
   QStringList aucdirs = rfilt ? QStringList(filt_run)
                               : QDir(rdir).entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);

   QStringList aucfilt;
   QStringList edtfilt;
   QStringList edtIDs;
   QStringList mdlIDs;
   QStringList mdlfns;
   QStringList noifns;
   QStringList modfilt("M*xml");
   QStringList noifilt("N*xml");
   QStringList modfils = QDir(dirm).entryList(modfilt, QDir::Files, QDir::Name);
   QStringList noifils = QDir(dirn).entryList(noifilt, QDir::Files, QDir::Name);
   int ktask = 0;
   int naucd = aucdirs.size();
   int nedtf = naucd * 3;
   int nmodf = modfils.size();
   int nnoif = noifils.size();
   QString aucpatt = "*.auc";
   QString edtpatt = "*.xml";

   if (rfilt) { // If run filtering, count actual files that match
      nedtf = 0;
      nmodf = 0;
      nnoif = 0;

      for (int ii = 0; ii < naucd; ii++) {
         QString subdir = rdir + aucdirs.at(ii);
         edtpatt = tfilt ? filt_run + ".*" + filt_triple + ".xml" : filt_run + ".*.xml";
         edtfilt.clear();
         edtfilt << edtpatt;
         QStringList edtfiles = QDir(subdir).entryList(edtfilt, QDir::Files, QDir::Name);
         nedtf += edtfiles.size();
         aucpatt = tfilt ? filt_run + ".*" + filt_triple + ".auc" : filt_run + ".*.auc";
      }

      DbgLv(1) << "BrLoc:  modf size" << modfils.size() << "filt_run" << filt_run;
      for (int ii = 0; ii < modfils.size(); ii++) {
         US_Model model;
         QString modfil = dirm + "/" + modfils.at(ii);
         model.load(modfil);
         QString mdesc = model.description;
         DbgLv(2) << "BrLoc:     ii" << ii << "mdesc" << mdesc;
         if (!mdesc.startsWith(filt_run))
            continue;
         if (tfilt && !mdesc.contains(filt_triple))
            continue;
         nmodf++;
         mdlfns << modfil;
         DbgLv(2) << "BrLoc:       nmodf" << nmodf << "*MATCH*";
      }
      DbgLv(1) << "BrLoc:    nmodf" << nmodf;

      for (int ii = 0; ii < noifils.size(); ii++) {
         US_Noise noise;
         QString noifil = dirn + "/" + noifils.at(ii);
         noise.load(noifil);
         QString ndesc = noise.description;
         if (!ndesc.startsWith(filt_run))
            continue;
         if (tfilt && !ndesc.contains(filt_triple))
            continue;
         nnoif++;
         noifns << noifil;
      }
   }

   int nstep = naucd + nedtf + nmodf + nnoif;
   DbgLv(1) << "BrLoc:  naucd nedtf nmodf nnoif nstep" << naucd << nedtf << nmodf << nnoif << nstep;
   aucfilt.clear();
   edtfilt.clear();
   aucfilt << aucpatt;
   edtfilt << edtpatt;
   DbgLv(1) << "BrLoc:   aucfilt" << aucfilt << "edtfilt" << edtfilt;
   rdir = rdir + "/";
   lb_status->setText(tr("Reading Local-Disk Data..."));
   progress->setMaximum(nstep);
   qApp->processEvents();

   for (int ii = 0; ii < naucd; ii++) { // loop thru potential data directories
      QString subdir = rdir + aucdirs.at(ii);
      QStringList aucfiles = QDir(subdir).entryList(aucfilt, QDir::Files, QDir::Name);
      int naucf = aucfiles.size();
      DbgLv(1) << "BrLoc:     ii naucf" << ii << naucf << "subdir" << subdir;
      US_DataIO::RawData rdata;
      US_DataIO::EditValues edval;

      for (int jj = 0; jj < naucf; jj++) { // loop thru .auc files found in a directory
         QString fname = aucfiles.at(jj);
         QString runid = fname.section(".", 0, 0);
         QString tripl = fname.section(".", -5, -2);
         QString aucfile = subdir + "/" + fname;
         QString descr = "";
         QString expGUID = expGUIDauc(aucfile);
         DbgLv(2) << "BrLoc: ii jj file" << ii << jj << aucfile;

         // read in the raw data and build description record
         US_DataIO::readRawData(aucfile, rdata);

         contents = US_Util::md5sum_file(aucfile);
         DbgLv(2) << "BrLoc:      contents" << contents;

         QString uuid = US_Util::uuid_unparse(( uchar * ) rdata.rawGUID);
         QString rawGUID = uuid;

         cdesc.recordID = -1;
         cdesc.recType = 1;
         cdesc.subType = "";
         cdesc.recState = REC_LO;
         cdesc.dataGUID = rawGUID.simplified();
         cdesc.parentGUID = expGUID.simplified();
         cdesc.parentID = -1;
         cdesc.filename = aucfile;
         cdesc.contents = contents;
         cdesc.label = runid + "." + tripl;
         cdesc.description = rdata.description;
         cdesc.filemodDate
            = US_Util::toUTCDatetimeText(QFileInfo(aucfile).lastModified().toUTC().toString(Qt::ISODate), true);
         cdesc.lastmodDate = "";

         if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
            cdesc.dataGUID = US_Util::new_guid();

         cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;

         ldescs << cdesc;

         // now load edit files associated with this auc file
         edtfilt.clear();
         edtfilt << runid + ".*." + tripl + ".xml";
         DbgLv(2) << "BrLoc:  edtfilt" << edtfilt;

         QStringList edtfiles = QDir(subdir).entryList(edtfilt, QDir::Files, QDir::Name);

         for (int kk = 0; kk < edtfiles.size(); kk++) {
            QString efname = edtfiles.at(kk);
            QString editid = efname.section(".", 1, 3);
            QString edtfile = subdir + "/" + efname;
            contents = "";
            DbgLv(2) << "BrLoc:    kk file" << kk << edtfile;

            // read EditValues for the edit data and build description record
            US_DataIO::readEdits(edtfile, edval);

            contents = US_Util::md5sum_file(edtfile);
            //DbgLv(2) << "BrLoc:      (E)contents edtfile" << contents << edtfile;

            cdesc.recordID = -1;
            cdesc.recType = 2;
            cdesc.subType = efname.section(".", 2, 2);
            cdesc.recState = REC_LO;
            cdesc.dataGUID = edval.editGUID.simplified();
            cdesc.parentGUID = edval.dataGUID.simplified();
            cdesc.parentID = -1;
            cdesc.filename = edtfile;
            cdesc.contents = contents;
            cdesc.label = runid + "." + editid;
            cdesc.description = efname.section(".", 0, -2);
            cdesc.filemodDate
               = US_Util::toUTCDatetimeText(QFileInfo(edtfile).lastModified().toUTC().toString(Qt::ISODate), true);
            cdesc.lastmodDate = "";

            if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
               cdesc.dataGUID = US_Util::new_guid();

            cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;

            ldescs << cdesc;
            edtIDs << cdesc.dataGUID;
         }
         if (ii == (naucd / 2) && jj == (naucf / 2)) {
            progress->setValue(++ktask);
            qApp->processEvents();
         }
      }
      progress->setValue(++ktask);
      qApp->processEvents();
   }
   progress->setValue(++ktask);
   qApp->processEvents();

   DbgLv(2) << "BrLoc: edtIDs" << edtIDs;
   for (int ii = 0; ii < nmodf; ii++) { // loop thru potential model files
      US_Model model;
      QString modfil = rfilt ? mdlfns.at(ii) : (dirm + "/" + modfils.at(ii));
      contents = "";

      model.load(modfil);

      contents = US_Util::md5sum_file(modfil);

      cdesc.recordID = -1;
      cdesc.recType = 3;
      cdesc.subType = model_type(model);
      cdesc.recState = REC_LO;
      cdesc.dataGUID = model.modelGUID.simplified();
      cdesc.parentGUID = model.editGUID.simplified();
      DbgLv(2) << "BrLoc:   mdl ii" << ii << "modfil" << modfil << "pGUID" << cdesc.parentGUID;

      if (rfilt && !edtIDs.contains(cdesc.parentGUID))
         continue;
      else if (rfilt)
         DbgLv(2) << "BrLoc:       mdl GUID *MATCH*";

      cdesc.parentID = -1;
      cdesc.filename = modfil;
      cdesc.contents = contents;
      cdesc.description = model.description;
      cdesc.filemodDate
         = US_Util::toUTCDatetimeText(QFileInfo(modfil).lastModified().toUTC().toString(Qt::ISODate), true);
      cdesc.lastmodDate = "";
      if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
         cdesc.dataGUID = US_Util::new_guid();

      cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;
      QString label = model.description.section(".", 0, -2);
      cdesc.label = (label.length() < 41) ? label : (label.left(13) + "..." + label.right(24));

      ldescs << cdesc;
      mdlIDs << cdesc.dataGUID;

      progress->setValue(++ktask);
      qApp->processEvents();
   }
   DbgLv(2) << "BrLoc: ldesc size" << ldescs.count();

   for (int ii = 0; ii < nnoif; ii++) { // loop thru potential noise files
      US_Noise noise;
      QString noifil = rfilt ? noifns.at(ii) : (dirn + "/" + noifils.at(ii));

      noise.load(noifil);

      contents = US_Util::md5sum_file(noifil);

      cdesc.recordID = -1;
      cdesc.recType = 4;
      cdesc.subType = (noise.type == US_Noise::RI) ? "RI" : "TI";
      cdesc.recState = REC_LO;
      cdesc.dataGUID = noise.noiseGUID.simplified();
      cdesc.parentGUID = noise.modelGUID.simplified();

      if (rfilt && !mdlIDs.contains(cdesc.parentGUID))
         continue;

      cdesc.parentID = -1;
      cdesc.filename = noifil;
      cdesc.contents = contents;
      cdesc.description = noise.description;
      cdesc.filemodDate
         = US_Util::toUTCDatetimeText(QFileInfo(noifil).lastModified().toUTC().toString(Qt::ISODate), true);
      cdesc.lastmodDate = "";

      if (cdesc.dataGUID.length() != 36 || cdesc.dataGUID == dmyGUID)
         cdesc.dataGUID = US_Util::new_guid();

      cdesc.parentGUID = cdesc.parentGUID.simplified().length() == 36 ? cdesc.parentGUID.simplified() : dmyGUID;
      QString label = noise.description;
      cdesc.label = (label.length() < 41) ? label : (label.left(9) + "..." + label.right(28));

      ldescs << cdesc;

      progress->setValue(++ktask);
      qApp->processEvents();
   }

   progress->setValue(nstep);
   lb_status->setText(tr("Local Data Review Complete"));
   qApp->processEvents();
}

// merge the database and local description vectors into a single combined
void US_DataModel::merge_dblocal() {
   int nddes = ddescs.size();
   int nldes = ldescs.size();
   int nstep = ((nddes + nldes) * 5) / 8;

   int jdr = 0;
   int jlr = 0;
   int kar = 1;

   DataDesc descd = (nddes > 0) ? ddescs.at(0) : DataDesc();
   DataDesc descl = (nldes > 0) ? ldescs.at(0) : DataDesc();
   DbgLv(1) << "MERGE: nd nl dlab llab" << nddes << nldes << descd.label << descl.label;

   lb_status->setText(tr("Merging Data ..."));
   progress->setMaximum(nstep);
   qApp->processEvents();

   while (jdr < nddes && jlr < nldes) { // main loop to merge records until one is exhausted

      progress->setValue(kar); // report progress

      if (kar > nstep) { // if count beyond max, bump max by one eighth
         nstep = (kar * 9) / 8;
         progress->setMaximum(nstep);
      }
      qApp->processEvents();

      while (descd.dataGUID == descl.dataGUID) { // records match in GUID:  merge them into one
         descd.recState |= descl.recState; // OR states
         descd.filename = descl.filename; // filename from local
         descd.filemodDate = descl.filemodDate; // file last mod from local
         descd.description = descl.description; // description from local
         //if ( descl.recType == 3 && descd.contents != descl.contents ) {
         // US_Model modell;
         // US_Model modeld;
         // modell.load( descl.filename );
         // modeld.load( QString::number(descd.recordID), db );
         // DbgLv(1) << " ++LOCAL Model:";
         // modell.debug();
         // DbgLv(1) << " ++DB Model:";
         // modeld.debug(); }
         descd.contents = descd.contents + " " + descl.contents;

         adescs << descd; // output combo record
         DbgLv(2) << "MERGE:  kar jdr jlr (1)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if (++jdr < nddes) // bump db count and test if done
            descd = ddescs.at(jdr); // get next db record

         else {
            if (++jlr < nldes)
               descl = ldescs.at(jlr); // get next local record
            break;
         }


         if (++jlr < nldes) // bump local count and test if done
            descl = ldescs.at(jlr); // get next local record
         else
            break;
      }

      if (jdr >= nddes || jlr >= nldes)
         break;

      while (descd.recType > descl.recType) { // output db records that are left-over children
         adescs << descd;
         DbgLv(2) << "MERGE:  kar jdr jlr (2)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if (++jdr < nddes)
            descd = ddescs.at(jdr);
         else
            break;
      }

      if (jdr >= nddes || jlr >= nldes)
         break;

      while (descl.recType > descd.recType) { // output local records that are left-over children
         adescs << descl;
         DbgLv(2) << "MERGE:  kar jdr jlr (3)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if (++jlr < nldes)
            descl = ldescs.at(jlr);
         else
            break;
      }

      if (jdr >= nddes || jlr >= nldes)
         break;

      // If we've reached another matching pair or if we are not at
      // the same level, go back up to the start of the main loop.
      if (descd.dataGUID == descl.dataGUID || descd.recType != descl.recType)
         continue;

      // If we are here, we have records at the same level,
      // but with different GUIDs. Output one of them, based on
      // an alphanumeric comparison of label values.

      QString dlabel = descd.label;
      QString llabel = descl.label;
      if (descd.recType > 2) {
         dlabel = descd.description;
         llabel = descl.description;
      }
      DbgLv(2) << "MERGE: rtype dlabel llabel" << descd.recType << dlabel << llabel;

      if (dlabel < llabel) { // output db record first based on alphabetic label sort
         adescs << descd;
         DbgLv(2) << "MERGE:  kar jdr jlr (4)GID" << kar << jdr << jlr << descd.dataGUID;
         kar++;

         if (++jdr < nddes)
            descd = ddescs.at(jdr);
         else
            break;
      }

      else { // output local record first based on alphabetic label sort
         adescs << descl;
         DbgLv(2) << "MERGE:  kar jdr jlr (5)GID" << kar << jdr << jlr << descl.dataGUID;
         kar++;

         if (++jlr < nldes)
            descl = ldescs.at(jlr);
         else
            break;
      }

   } // end of main merge loop;

   // after breaking from main loop, output any records left from one
   // source (db/local) or the other.
   nstep += (nddes - jdr + nldes - jlr);
   progress->setMaximum(nstep);
   qApp->processEvents();

   while (jdr < nddes) {
      adescs << ddescs.at(jdr++);
      descd = ddescs.at(jdr - 1);
      DbgLv(2) << "MERGE:  kar jdr jlr (8)GID" << kar << jdr << jlr << descd.dataGUID;
      progress->setValue(++kar);
      qApp->processEvents();
   }

   while (jlr < nldes) {
      adescs << ldescs.at(jlr++);
      descl = ldescs.at(jlr - 1);
      DbgLv(2) << "MERGE:  kar jdr jlr (9)GID" << kar << jdr << jlr << descl.dataGUID;
      progress->setValue(++kar);
      qApp->processEvents();
   }

   DbgLv(2) << "MERGE: nddes nldes kar" << nddes << nldes << --kar;
   DbgLv(2) << " a/d/l sizes" << adescs.size() << ddescs.size() << ldescs.size();

   progress->setValue(nstep);
   lb_status->setText(tr("Data Merge Complete"));
   qApp->processEvents();
}

// Exclude DB-only or Local-only trees
void US_DataModel::exclude_trees() {
   QVector<DataDesc> tdess = adescs; // temporary descr. vector
   DataDesc desct; // temporary descr. entry
   int krecs = tdess.size();
   int nrecs = 0;
   bool excdb = filt_source.contains("DB");
   bool exctr = false;
   adescs.clear();

   for (int ii = 0; ii < krecs; ii++) {
      desct = tdess[ ii ];

      if (desct.recType == 1) { // If head of tree, test whether DB/Local/Both
         bool isDba = ((desct.recState & REC_DB) != 0);
         bool isLoc = ((desct.recState & REC_LO) != 0);

         if (isDba && isLoc) { // If both, tree should not be excluded
            exctr = false;
         }

         else if (isDba) { // If DB-Only, exclude by Exclude-DB-Only filter
            exctr = excdb;
         }

         else { // If Local-Only, exclude by Exclude-Local-Only filter
            exctr = !excdb;
         }
      }

      if (exctr) { // Tree is excluded, so skip this record
         continue;
      }

      // Otherwise, copy to re-created descriptions vector
      adescs << desct;
      nrecs++;
   }
}

// sort a data-set description vector
void US_DataModel::sort_descs(QVector<DataDesc> &descs) {
   QVector<DataDesc> tdess; // temporary descr. vector
   DataDesc desct; // temporary descr. entry
   QStringList sortr; // sort string lists
   QStringList sorte;
   QStringList sortm;
   QStringList sortn;
   int nrecs = descs.size(); // number of descr. records

   lb_status->setText(tr("Sorting Descriptions..."));
   qApp->processEvents();
   DbgLv(1) << "sort_desc: nrecs" << nrecs;
   if (nrecs == 0)
      return;

   tdess.resize(nrecs);
   // Determine maximum description string length
   maxdlen = 0;
   for (int ii = 0; ii < nrecs; ii++)
      maxdlen = qMax(maxdlen, descs[ ii ].description.length());

   for (int ii = 0; ii < nrecs; ii++) { // build sort strings for Raw,Edit,Model,Noise; copy unsorted vector
      desct = descs[ ii ];

      if (desct.recType == 1)
         sortr << sort_string(desct, ii);

      else if (desct.recType == 2)
         sorte << sort_string(desct, ii);

      else if (desct.recType == 3)
         sortm << sort_string(desct, ii);

      else if (desct.recType == 4)
         sortn << sort_string(desct, ii);

      tdess[ ii ] = desct;
   }
   DbgLv(2) << "SrtD:  nrecs" << nrecs << nowTime();

   // sort the string lists for each type
   sortr.sort();
   sorte.sort();
   sortm.sort();
   sortn.sort();
   DbgLv(2) << "SrtD:  sort[remn]" << nowTime();

   lb_status->setText(tr("Finding Duplicates..."));
   qApp->processEvents();
   // review each type for duplicate GUIDs
   if (review_descs(sortr, tdess))
      return;
   DbgLv(2) << "SrtD:  RD(r)" << nowTime();
   if (review_descs(sorte, tdess))
      return;
   DbgLv(2) << "SrtD:  RD(e)" << nowTime();
   if (review_descs(sortm, tdess))
      return;
   DbgLv(2) << "SrtD:  RD(m)" << nowTime();
   if (review_descs(sortn, tdess))
      return;
   DbgLv(2) << "SrtD:  RD(n)" << nowTime();

   lb_status->setText(tr("Finding Orphans..."));
   qApp->processEvents();

   // create list of noise,model,edit orphans
   QStringList orphn = list_orphans(sortn, sortm);
   QStringList orphm = list_orphans(sortm, sorte);
   QStringList orphe = list_orphans(sorte, sortr);
   DbgLv(2) << "SrtD:  Orph(nme)" << nowTime();

   QString dmyGUID = "00000000-0000-0000-0000-000000000000";
   QString dsorts;
   QString dlabel;
   QString dindex;
   QString ddGUID;
   QString dpGUID;
   QString ppGUID;
   int kndx = tdess.size();
   int jndx;
   int kk;
   int ndmy = 0; // flag of duplicate dummies

   // Create lists of parent GUIDs
   QStringList guidsr;
   QStringList guidse;
   QStringList guidsm;

   for (int ii = 0; ii < sortr.size(); ii++)
      guidsr << sortr.at(ii).section(":", 2, 2).simplified();

   for (int ii = 0; ii < sorte.size(); ii++)
      guidse << sorte.at(ii).section(":", 2, 2).simplified();

   for (int ii = 0; ii < sortm.size(); ii++)
      guidsm << sortm.at(ii).section(":", 2, 2).simplified();

   // create dummy records to parent each orphan

   int nstep = orphn.size() + orphm.size() + orphe.size();
   int istep = 0;
   progress->setMaximum(nstep);
   progress->setValue(istep);
   qApp->processEvents();
   DbgLv(2) << "(1) orphan: N size M size" << orphn.size() << orphm.size();
   for (int ii = 0; ii < orphn.size(); ii++) { // for each orphan noise, create a dummy model
      dsorts = orphn.at(ii);
      dlabel = dsorts.section(":", 0, 0);
      dindex = dsorts.section(":", 1, 1);
      ddGUID = dsorts.section(":", 2, 2).simplified();
      dpGUID = dsorts.section(":", 3, 3).simplified();
      jndx = dindex.toInt();
      cdesc = tdess[ jndx ];

      if (dpGUID.length() < 2 || dpGUID == dmyGUID) { // handle case where there is no valid parentGUID
         if (ndmy == 0) // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID; // afterwards:  re-use same parent

         kk = sortn.indexOf(dsorts); // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if (kk >= 0) { // replace present record for new parentGUID
            sortn.replace(kk, dsorts);
            cdesc.parentGUID = dpGUID;
            tdess[ jndx ] = cdesc;
         }

         if (ndmy > 0) // after 1st time, skip creating new parent
            continue;

         ndmy++; // flag that we have a parent for invalid ones
         ppGUID = dpGUID; // save the GUID for new dummy parent
      }

      // If this record is no longer an orphan, skip creating new parent
      if (guidsm.indexOf(dpGUID) >= 0)
         continue;

      if (dpGUID == dmyGUID)
         cdesc.label = "Dummy-Model-for-Orphans";

      cdesc.parentID = cdesc.recordID;
      cdesc.recordID = -1;
      cdesc.recType = 3;
      cdesc.subType = "";
      cdesc.recState = NOSTAT;
      cdesc.dataGUID = dpGUID;
      cdesc.parentGUID = dmyGUID;
      cdesc.parentID = -1;
      cdesc.filename = "";
      cdesc.contents = "";
      cdesc.label = cdesc.label.section(".", 0, 0);
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.filemodDate = US_Util::toUTCDatetimeText(QDateTime::currentDateTime().toUTC().toString(Qt::ISODate), true);
      cdesc.lastmodDate = cdesc.filemodDate;

      dlabel = dlabel.section(".", 0, 0);
      dindex = QString().sprintf("%4.4d", kndx++);
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortm << dsorts;
      orphm << dsorts;
      guidsm << ddGUID;
      tdess << cdesc;
      DbgLv(2) << "N orphan:" << orphn.at(ii) << ii;
      DbgLv(2) << "  M dummy:" << dsorts;
      progress->setValue(++istep);
      qApp->processEvents();
   }
   DbgLv(2) << "(2) orphan: N size M size" << orphn.size() << orphm.size();
   DbgLv(2) << "SrtD:  Orph(N)" << nowTime();

   ndmy = 0;

   for (int ii = 0; ii < orphm.size(); ii++) { // for each orphan model, create a dummy edit
      dsorts = orphm.at(ii);
      dlabel = dsorts.section(":", 0, 0);
      dindex = dsorts.section(":", 1, 1);
      ddGUID = dsorts.section(":", 2, 2).simplified();
      dpGUID = dsorts.section(":", 3, 3).simplified();
      jndx = dindex.toInt();
      cdesc = tdess[ jndx ];

      if (dpGUID.length() < 16 || dpGUID == dmyGUID) { // handle case where there is no valid parentGUID
         if (ndmy == 0) // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID; // afterwards:  re-use same parent

         kk = sortm.indexOf(dsorts); // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if (kk >= 0) { // replace present record for new parentGUID
            sortm.replace(kk, dsorts);
            cdesc.parentGUID = dpGUID;
            tdess[ jndx ] = cdesc;
         }

         if (ndmy > 0) // after 1st time, skip creating new parent
            continue;

         ndmy++; // flag that we have a parent for invalid ones
         ppGUID = dpGUID; // save the GUID for new dummy parent
      }

      // If this record is no longer an orphan, skip creating new parent
      if (guidse.indexOf(dpGUID) >= 0)
         continue;

      if (dpGUID == dmyGUID)
         cdesc.label = "Dummy-Edit-for-Orphans";

      cdesc.parentID = cdesc.recordID;
      cdesc.recordID = -1;
      cdesc.recType = 2;
      cdesc.subType = "";
      cdesc.recState = NOSTAT;
      cdesc.dataGUID = dpGUID;
      cdesc.parentGUID = dmyGUID;
      cdesc.parentID = -1;
      cdesc.filename = "";
      cdesc.contents = "";
      cdesc.label = cdesc.label.section(".", 0, 0);
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = US_Util::toUTCDatetimeText(QDateTime::currentDateTime().toUTC().toString(Qt::ISODate), true);
      cdesc.filemodDate = cdesc.lastmodDate;

      dlabel = dlabel.section(".", 0, 0);
      dindex = QString().sprintf("%4.4d", kndx++);
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sorte << dsorts;
      orphe << dsorts;
      guidse << ddGUID;
      tdess << cdesc;
      DbgLv(2) << "M orphan:" << orphm.at(ii) << ii;
      DbgLv(2) << "  E dummy:" << dsorts;
      progress->setValue(++istep);
      qApp->processEvents();
   }

   DbgLv(2) << "(3) orphan: N size M size" << orphn.size() << orphm.size();
   DbgLv(2) << "SrtD:  Orph(M)" << nowTime();
   ndmy = 0;
   DbgLv(2) << "(4) orphan: M size E size" << orphm.size() << orphe.size();

   for (int ii = 0; ii < orphe.size(); ii++) { // for each orphan edit, create a dummy raw
      dsorts = orphe.at(ii);
      dlabel = dsorts.section(":", 0, 0);
      dindex = dsorts.section(":", 1, 1);
      ddGUID = dsorts.section(":", 2, 2).simplified();
      dpGUID = dsorts.section(":", 3, 3).simplified();
      jndx = dindex.toInt();
      cdesc = tdess[ jndx ];

      if (dpGUID.length() < 2) { // handle case where there is no valid parentGUID
         if (ndmy == 0) // first time:  create one
            dpGUID = dmyGUID;
         else
            dpGUID = ppGUID; // afterwards:  re-use same parent

         kk = sorte.indexOf(dsorts); // find index in full list
         dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

         if (kk >= 0) { // replace present record for new parentGUID
            sorte.replace(kk, dsorts);
            cdesc.parentGUID = dpGUID;
            tdess[ jndx ] = cdesc;
         }

         if (ndmy > 0) // after 1st time, skip creating new parent
            continue;

         ndmy++; // flag that we have a parent for invalid ones
         ppGUID = dpGUID; // save the GUID for new dummy parent
      }

      // If this record is no longer an orphan, skip creating new parent
      if (guidsr.indexOf(dpGUID) >= 0)
         continue;

      if (dpGUID == dmyGUID)
         cdesc.label = "Dummy-Raw-for-Orphans";

      cdesc.parentID = cdesc.recordID;
      cdesc.recordID = -1;
      cdesc.recType = 1;
      cdesc.subType = "";
      cdesc.recState = NOSTAT;
      cdesc.dataGUID = dpGUID;
      cdesc.parentGUID = dmyGUID;
      cdesc.parentID = -1;
      cdesc.filename = "";
      cdesc.contents = "";
      cdesc.label = cdesc.label.section(".", 0, 0);
      cdesc.description = cdesc.label + "--ARTIFICIAL-RECORD";
      cdesc.lastmodDate = US_Util::toUTCDatetimeText(QDateTime::currentDateTime().toUTC().toString(Qt::ISODate), true);
      cdesc.filemodDate = cdesc.lastmodDate;

      dlabel = dlabel.section(".", 0, 0);
      dindex = QString().sprintf("%4.4d", kndx++);
      ddGUID = cdesc.dataGUID;
      dpGUID = cdesc.parentGUID;
      dsorts = dlabel + ":" + dindex + ":" + ddGUID + ":" + dpGUID;

      sortr << dsorts;
      guidsr << ddGUID;
      tdess << cdesc;
      DbgLv(2) << "E orphan:" << orphe.at(ii);
      DbgLv(2) << "  R dummy:" << dsorts;
      progress->setValue(++istep);
      qApp->processEvents();
   }
   DbgLv(2) << "(5) orphan: M size E size" << orphm.size() << orphe.size();
   DbgLv(2) << "SrtD:  Orph(E)" << nowTime();

   //for ( int ii = 0; ii < sortr.size(); ii++ )
   // DbgLv(2) << "R entry:" << sortr.at( ii );
   int countR = sortr.size(); // count of each kind in sorted lists
   int countE = sorte.size();
   int countM = sortm.size();
   int countN = sortn.size();

   //sortr.sort();                 // re-sort for dummy additions
   //sorte.sort();
   //sortm.sort();
   //sortn.sort();
   DbgLv(1) << "sort/dumy: count REMN" << countR << countE << countM << countN;
   //for(int ii=0;ii<countM;ii++) DbgLv(2) << "sm" << ii << "++ " << sortm[ii];

   int noutR = 0; // count of each kind in hierarchical output
   int noutE = 0;
   int noutM = 0;
   int noutN = 0;
   int indx;
   int pstate = REC_LO | PAR_LO;

   descs.clear(); // reset input vector to become sorted output
   lb_status->setText(tr("Building Sorted Trees..."));
   istep = 0;
   progress->setMaximum(countR);
   progress->setValue(istep);
   qApp->processEvents();

   // rebuild the description vector with sorted trees
   for (int ii = 0; ii < countR; ii++) { // loop to output sorted Raw records
      QString recr = sortr[ ii ];
      QString didr = recr.section(":", 2, 2);
      QString pidr = recr.section(":", 3, 3);
      indx = recr.section(":", 1, 1).toInt();
      cdesc = tdess.at(indx);

      // set up a default parent state flag
      pstate = cdesc.recState;
      pstate = (pstate & REC_DB) != 0 ? (pstate | PAR_DB) : pstate;
      pstate = (pstate & REC_LO) != 0 ? (pstate | PAR_LO) : pstate;

      // new state is the default,  or NOSTAT if this is a dummy record
      cdesc.recState = record_state_flag(cdesc, pstate);

      descs << cdesc; // output Raw rec
      noutR++;

      // set up parent state for children to follow
      int rpstate = cdesc.recState;

      for (int jj = 0; jj < countE; jj++) { // loop to output sorted Edit records for the above Raw
         QString rece = sorte[ jj ];
         QString pide = rece.section(":", 3, 3);

         if (pide != didr) // skip if current Raw not parent
            continue;

         QString dide = rece.section(":", 2, 2);
         indx = rece.section(":", 1, 1).toInt();
         cdesc = tdess.at(indx);
         cdesc.recState = record_state_flag(cdesc, rpstate);

         descs << cdesc; // output Edit rec
         noutE++;

         // set up parent state for children to follow
         int epstate = cdesc.recState;

         for (int mm = 0; mm < countM; mm++) { // loop to output sorted Model records for above Edit
            QString recm = sortm[ mm ];
            QString pidm = recm.section(":", 3, 3);

            if (pidm != dide) // skip if current Edit not parent
               continue;

            QString didm = recm.section(":", 2, 2);
            indx = recm.section(":", 1, 1).toInt();
            cdesc = tdess.at(indx);
            cdesc.recState = record_state_flag(cdesc, epstate);

            descs << cdesc; // output Model rec

            noutM++;

            // set up parent state for children to follow
            int mpstate = cdesc.recState;

            for (int nn = 0; nn < countN; nn++) { // loop to output sorted Noise records for above Model
               QString recn = sortn[ nn ];
               QString pidn = recn.section(":", 3, 3);

               if (pidn != didm) // skip if current Model not parent
                  continue;

               indx = recn.section(":", 1, 1).toInt();
               cdesc = tdess.at(indx);
               cdesc.recState = record_state_flag(cdesc, mpstate);

               descs << cdesc; // output Noise rec

               noutN++;
            }
         }
      }
      progress->setValue(++istep);
      qApp->processEvents();
   }
   DbgLv(2) << "SrtD:  END" << nowTime();

   if (
      noutR != countR || noutE != countE || noutM != countM
      || noutN != countN) { // not all accounted for, so we will need some dummy parents
      DbgLv(1) << "sort_desc: count REMN" << countR << countE << countM << countN;
      DbgLv(1) << "sort_desc:   nout REMN" << noutR << noutE << noutM << noutN;
   }
}

// review sorted string lists for duplicate GUIDs
bool US_DataModel::review_descs(QStringList &sorts, QVector<DataDesc> &descv) {
   bool abort = false;
   int nrecs = sorts.size();
   int nmult = 0;
   int kmult = 0;
   int ityp;
   QString cGUID;
   QString pGUID;
   QString rtyp;
   QVector<int> multis;
   const char *rtyps[] = { "RawData", "EditedData", "Model", "Noise" };
   QStringList tGUIDs;

   if (nrecs < 1)
      return abort;

   int ii = sorts[ 0 ].section(":", 1, 1).toInt();
   ityp = descv[ ii ].recType;
   rtyp = QString(rtyps[ ityp - 1 ]);

   if (descv[ ii ].recordID >= 0)
      rtyp = "DB " + rtyp;
   else
      rtyp = "Local " + rtyp;
   DbgLv(2) << "RvwD: ii ityp rtyp nrecs" << ii << ityp << rtyp << nrecs;
   cGUID = sorts[ 0 ].section(":", 2, 2);
   tGUIDs << cGUID;

   for (int ii = 1; ii < nrecs; ii++) { // do a pass to determine if there are duplicate GUIDs
      cGUID = sorts[ ii ].section(":", 2, 2); // current rec GUID
      kmult = 0; // flag no multiples yet

      int jj = tGUIDs.indexOf(cGUID);

      if (jj >= 0) {
         kmult++;
         if (!multis.contains(jj)) { // not yet marked, so mark previous as multiple
            multis << jj; // save index
            nmult++; // bump count
         }
      }

      if (kmult > 0) { // this pass found a duplicate:  save the index and bump count
         multis << ii;
         nmult++;
         DbgLv(1) << "RvwD: nmult" << nmult << "ii,jj" << ii << jj << "cGUID,pGUID" << cGUID << tGUIDs[ jj ];
      }

      tGUIDs << cGUID;
      //DbgLv(2) << "RvwD:   ii kmult nmult" << ii << kmult << nmult;
   }

   DbgLv(2) << "RvwD:      GUID nmult" << nmult << nowTime();
   if (nmult > 0) { // there were multiple instances of the same GUID
      QMessageBox msgBox;
      QString msg;

      // format a message for the warning pop-up
      msg = tr("There are %1 %2 records that have\n").arg(nmult).arg(rtyp) + tr("the same GUID as another.\n")
            + tr("You should correct the situation before proceeding.\n")
            + tr("  Click \"Ok\" to see details, then abort.\n")
            + tr("  Click \"Ignore\" to proceed to further review.\n");
      msgBox.setWindowTitle(tr("Duplicate %1 Records").arg(rtyp));
      msgBox.setText(msg);
      msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
      msgBox.setDefaultButton(QMessageBox::Ok);

      if (msgBox.exec() == QMessageBox::Ok) { // user wants details, so display them
         QString fileexts = tr("Text,Log files (*.txt *.log);;") + tr("All files (*)");
         QString pGUID = "";
         QString cGUID;
         QString label;

         msg = tr("Review the details below on duplicate records.\n")
               + tr("Save or Print the contents of this message.\n")
               + tr("Decide which of the duplicates should be removed.\n")
               + tr("Close the main US_DataModel window after exiting here.\n") + tr("\nSummary of Duplicates:\n\n");

         for (int ii = 0; ii < nmult; ii++) { // add summary lines on duplicates
            int jj = multis.at(ii);
            cGUID = sorts.at(jj).section(":", 2, 2);
            label = sorts.at(jj).section(":", 0, 0);

            if (cGUID != pGUID) { // first instance of this GUID:  show GUID
               msg += tr("GUID:  ") + cGUID + "\n";
               pGUID = cGUID;
            }

            // one label line for each multiple
            msg += tr("  Label:  ") + label + "\n";
         }

         msg += tr("\nDetails of Duplicates:\n\n");

         for (int ii = 0; ii < nmult; ii++) { // add detail lines
            int jj = multis.at(ii);
            cGUID = sorts.at(jj).section(":", 2, 2);
            pGUID = sorts.at(jj).section(":", 3, 3);
            label = sorts.at(jj).section(":", 0, 0);
            int kk = sorts.at(jj).section(":", 1, 1).toInt();
            cdesc = descv[ kk ];

            msg += tr("GUID:  ") + cGUID + "\n" + tr("  ParentGUID:  ") + pGUID + "\n" + tr("  Label:  ") + label + "\n"
                   + tr("  Description:  ") + cdesc.description + "\n" + tr("  DB record ID:  %1").arg(cdesc.recordID)
                   + "\n" + tr("  File Directory:  ") + cdesc.filename.section("/", 0, -2) + "\n" + tr("  File Name:  ")
                   + cdesc.filename.section("/", -1, -1) + "\n" + tr("  File Last Mod:  ") + cdesc.filemodDate + "\n"
                   + tr("  Last Mod Date:  ") + cdesc.lastmodDate + "\n";
         }

         // pop up text dialog
         US_Editor *editd = new US_Editor(US_Editor::LOAD, true, fileexts);
         editd->setWindowTitle(tr("Data Set Duplicate GUID Details"));
         editd->move(QCursor::pos() + QPoint(200, 200));
         editd->resize(600, 500);
         editd->e->setFont(QFont(US_Widgets::fixedFont().family(), US_GuiSettings::fontSize()));
         editd->e->setText(msg);
         editd->show();

         abort = true; // tell caller to abort data tree build
      }

      else {
         abort = false; // signal to proceed with data tree build
      }
      DbgLv(2) << "review_descs   abort flag:" << abort;
   }

   return abort;
}

// find index of substring at given position in strings of string list
int US_DataModel::index_substring(QString ss, int ixs, QStringList &sl) {
   QString sexp = "XXX";
   QRegExp rexp;

   if (ixs == 0)
      sexp = ss + ":*"; // label at beginning of strings in list

   else if (ixs == 1 || ixs == 2)
      sexp = "*:" + ss + ":*"; // RecIndex/recGUID in middle of list strings

   else if (ixs == 3)
      sexp = "*:" + ss; // parentGUID at end of strings in list

   rexp = QRegExp(sexp, Qt::CaseSensitive, QRegExp::Wildcard);

   return sl.indexOf(rexp);
}

// Get sublist from string list of substring matches at given string position
QStringList US_DataModel::filter_substring(QString ss, int ixs, QStringList &sl) {
   QStringList subl;

   if (ixs == 0)
      // match label at beginning of strings in list
      subl = sl.filter(QRegExp("^" + ss + ":"));

   else if (ixs == 1 || ixs == 2)
      // match RecIndex or recGUID in middle of strings in list
      subl = sl.filter(":" + ss + ":");

   else if (ixs == 3)
      // match parentGUID at end of strings in list
      subl = sl.filter(QRegExp(":" + ss + "$"));

   return subl;
}

// List orphans of a record type (in rec list, no tie to parent list)
QStringList US_DataModel::list_orphans(QStringList &rlist, QStringList &plist) {
   QStringList olist;
   QStringList tlist;

   for (int ii = 0; ii < plist.size(); ii++) { // Build test-parent-GUID list
      tlist << plist.at(ii).section(":", 2, 2);
   }

   for (int ii = 0; ii < rlist.size(); ii++) { // examine parentGUID for each record in the list
      QString pReco = rlist.at(ii);
      QString pGUID = pReco.section(":", 3, 3);

      // see if it is the recordGUID of any in the potential parent list
      if (!tlist.contains(pGUID))
         olist << pReco; // no parent found, so add to the orphan list
   }

   return olist;
}

// return a record state flag with parent state ORed in
int US_DataModel::record_state_flag(DataDesc descr, int pstate) {
   int state = descr.recState;

   if (descr.recState == NOSTAT || descr.description.contains("-ARTIFICIAL"))
      state = NOSTAT; // mark a dummy record

   else { // detect and mark parentage of non-dummy
      if ((pstate & REC_DB) != 0)
         state = state | PAR_DB; // mark a record with db parent

      if ((pstate & REC_LO) != 0)
         state = state | PAR_LO; // mark a record with local parent
   }

   return state;
}

// compose concatenation on which to sort (label:index:dataGUID:parentGUID)
QString US_DataModel::sort_string(DataDesc ddesc, int indx) { // create string for ascending sort on label
   QString label = (ddesc.recType < 3) ? ddesc.label : ddesc.description;
   int lablen = label.length();
   if (lablen < maxdlen)
      label = label.leftJustified(maxdlen, ' ');

   QString ostr = label // label to sort on
                  + ":" + QString().sprintf("%4.4d", indx) // index in desc. vector
                  + ":" + ddesc.dataGUID // data GUID
                  + ":" + ddesc.parentGUID; // parent GUID
   return ostr;
}

// compose string describing model type
QString US_DataModel::model_type(int imtype, int nassoc, int gtype, bool isMC) {
   QString mtype;

   // format the base model type string
   switch (imtype) {
      default:
      case ( int ) US_Model::MANUAL:
         mtype = "MANUAL";
         break;
      case ( int ) US_Model::TWODSA:
         mtype = "2DSA";
         break;
      case ( int ) US_Model::TWODSA_MW:
         mtype = "2DSA-MW";
         break;
      case ( int ) US_Model::GA:
         mtype = "GA";
         break;
      case ( int ) US_Model::GA_MW:
         mtype = "GA-MW";
         break;
      case ( int ) US_Model::COFS:
         mtype = "COFS";
         break;
      case ( int ) US_Model::FE:
         mtype = "FE";
         break;
      case ( int ) US_Model::PCSA:
         mtype = "PCSA";
         break;
      case ( int ) US_Model::CUSTOMGRID:
         mtype = "CUSTOMGRID";
         break;
   }

   // add RA for Reversible Associations (if associations count > 1)
   if (nassoc > 1)
      mtype = mtype + "-RA";

   // add FM | GL | SG for Fit-Meniscus|GLobal|SuperGlobal
   if (gtype == ( int ) US_Model::MENISCUS)
      mtype = mtype + "-FM";

   else if (gtype == ( int ) US_Model::GLOBAL)
      mtype = mtype + "-GL";

   else if (gtype == ( int ) US_Model::SUPERGLOBAL)
      mtype = mtype + "-SG";

   // add MC for Monte Carlo
   if (isMC)
      mtype = mtype + "-MC";

   return mtype;
}

// compose string describing model type
QString US_DataModel::model_type(US_Model model) {
   // return model type string based on flags in the model object
   return model_type(( int ) model.analysis, model.associations.size(), ( int ) model.global, model.monteCarlo);
}

// compose string describing model type
QString US_DataModel::model_type(QString modxml) {
   QChar quo('"');
   int jj;
   int imtype;
   int nassoc;
   int gtype;
   bool isMC;

   // model type number from type attribute
   jj = modxml.indexOf(" analysisType=");
   imtype = (jj < 1) ? 0 : modxml.mid(jj).section(quo, 1, 1).toInt();

   // count of associations is count of K_d attributes present
   nassoc = modxml.count("K_d=");
   nassoc = (nassoc == 0) ? modxml.count("k_assoc=") : nassoc;
   nassoc = (nassoc == 0) ? modxml.count("k_eq=") : nassoc;

   // global type number from type attribute
   jj = modxml.indexOf(" globalType=");
   gtype = (jj < 1) ? 0 : modxml.mid(jj).section(quo, 1, 1).toInt();

   // flag if MonteCarlo
   jj = modxml.indexOf(" MonteCarlo=\"1");
   isMC = (jj > 0);

   // return model type string based on integer flags
   return model_type(imtype, nassoc, gtype, isMC);
}

void US_DataModel::dummy_data() {
   adescs.clear();
   ddescs.clear();
   ldescs.clear();

   cdesc.recType = 1;
   cdesc.recState = REC_DB | PAR_DB;
   cdesc.subType = "";
   cdesc.label = "item_1_2";
   cdesc.description = "demo1_veloc";
   cdesc.dataGUID = "demo1_veloc";
   cdesc.parentID = 1;
   cdesc.recordID = 1;
   cdesc.filename = "";
   adescs << cdesc;
   ddescs << cdesc;

   cdesc.recType = 2;
   cdesc.recState = REC_DB | REC_LO | PAR_DB | PAR_LO;
   cdesc.subType = "RA";
   cdesc.label = "item_2_2";
   cdesc.description = "demo1_veloc";
   cdesc.contents = "AA 12 AA 12";
   cdesc.parentID = 1;
   cdesc.recordID = 2;
   cdesc.filename = "demo1_veloc_edit.xml";
   adescs << cdesc;
   ddescs << cdesc;
   ldescs << cdesc;

   cdesc.recType = 3;
   cdesc.recState = REC_LO | PAR_LO;
   cdesc.subType = "2DSA";
   cdesc.label = "item_3_2";
   cdesc.description = "demo1_veloc.sa2d.model.11";
   cdesc.parentID = 2;
   cdesc.recordID = -1;
   cdesc.filename = "demo1_veloc_model.xml";
   adescs << cdesc;
   ldescs << cdesc;

   cdesc.recType = 4;
   cdesc.recState = REC_DB | REC_LO | PAR_DB | PAR_LO;
   cdesc.subType = "TI";
   cdesc.label = "item_4_2";
   cdesc.description = "demo1_veloc.ti_noise";
   cdesc.contents = "BB 12 AA 13";
   cdesc.parentID = 2;
   cdesc.recordID = 3;
   cdesc.filename = "demo1_veloc_noise.xml";
   adescs << cdesc;
   ddescs << cdesc;
   ldescs << cdesc;

   cdesc.recType = 2;
   cdesc.recState = NOSTAT;
   cdesc.subType = "RA";
   cdesc.label = "item_5_2";
   cdesc.description = "demo1_veloc";
   cdesc.contents = "CC 15";
   cdesc.recordID = -1;
   cdesc.filename = "";
   adescs << cdesc;
}

QString US_DataModel::expGUIDauc(QString aucfile) {
   QString expGUID = "00000000-0000-0000-0000-000000000000";
   QString expfnam = aucfile.section("/", -1, -1).section(".", 0, 1) + ".xml";
   QString expfile = aucfile.section("/", 0, -2) + "/" + expfnam;

   QFile file(expfile);

   if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QXmlStreamReader xml(&file);

      while (!xml.atEnd()) {
         xml.readNext();

         if (xml.isStartElement() && xml.name() == "experiment") {
            QXmlStreamAttributes a = xml.attributes();
            expGUID = a.value("guid").toString();
            break;
         }
      }

      file.close();
   }

   return expGUID;
}

// Review and reset database description list after changes
void US_DataModel::review_dbase() {
   ddescs.clear(); // Wipe out the old DB descriptions list
   DbgLv(1) << "RvDB: #chgr #adesc" << chgrows.size() << adescs.size();

   for (int row = 0; row < adescs.size(); row++) { // Review all records looking for changed DB records
      cdesc = adescs[ row ];

      if (chgrows.contains(row)) { // Test whether to add to list if a changed row
         DbgLv(1) << "RvDB:   row state ID" << row << cdesc.recState << cdesc.recordID;

         if (cdesc.recordID < 0) // skip if DB rec removed
            continue; //  or local-only

         DbgLv(1) << "RvDB:     ++ upd cdesc, row state" << row << cdesc.recState;
      }

      else if ((cdesc.recState & PAR_DB) == 0)
         continue; // skip if local-only

      // Modify contents and state to be DB-only
      QString contdb = cdesc.contents;
      contdb = contdb.section(" ", 0, 0) + " " + contdb.section(" ", 1, 1);
      cdesc.contents = contdb;
      cdesc.recState = (cdesc.recState & PAR_DB) | REC_DB;

      // Save this DB record to the DB list
      ddescs << cdesc;
   }
   DbgLv(1) << "RvDB:  #ddescs" << ddescs.size();

   chgrows.clear(); // Clear changed-row list
   lb_status->setText(tr("Database Review Complete"));
}
