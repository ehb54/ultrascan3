#include "us_legacy_converter.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_archive.h"

int main(int argc, char *argv[])
{
   QApplication application(argc, argv);

   #include "main1.inc"

   // License is OK.  Start up.

   US_LegacyConverter* w = new US_LegacyConverter;
   w->show();                   //!< \memberof QWidget
   return application.exec();

}

US_LegacyConverter::US_LegacyConverter() : US_Widgets()
{
   setWindowTitle( tr( "Beckman to OpenAUC Data Converter" ) );
   setPalette( US_GuiSettings::frameColor() );
   this->setFixedSize(525, 525);

   data_types.insert("RI", "Intensity");
   data_types.insert("RA", "Absorbance");
   data_types.insert("WI", "WI");
   data_types.insert("WA", "WA");
   data_types.insert("IP", "Interference");
   data_types.insert("FI", "Fluorensce");

   pb_load = us_pushbutton("Load File (tar.gz)");
   pb_load->setMinimumWidth(100);
   le_load = us_lineedit("", 0, true);

   QLabel* lb_dir = us_label("Directory:");

   QString path = US_Settings::importDir();
   lb_dir->setAlignment(Qt::AlignRight);
   le_dir = us_lineedit(path, 0, true);
   QDir dir(path);
   if (! dir.exists()) {
      dir.mkpath(dir.absolutePath());
   }

   lb_runid = us_label("Run ID:");
   lb_runid->setAlignment(Qt::AlignRight);
   le_runid = new US_LineEdit_RE("", 0);

   pb_save = us_pushbutton("Save", true, 0);

   te_info = us_textedit();
   te_info->setReadOnly(true);

   QGridLayout *layout = new QGridLayout();
   layout->addWidget(pb_load,      0, 0, 1, 1);
   layout->addWidget(le_load,      0, 1, 1, 2);
   layout->addWidget(lb_dir,       1, 0, 1, 1);
   layout->addWidget(le_dir,       1, 1, 1, 2);
   layout->addWidget(lb_runid,     2, 0, 1, 1);
   layout->addWidget(le_runid,     2, 1, 1, 2);
   layout->addWidget(pb_save,      3, 1, 1, 1);
   layout->addWidget(te_info,      5, 0, 4, 3);
   layout->setContentsMargins( 2, 2, 2, 2 );
   layout->setSpacing(2);
   this->setLayout(layout);

   archive = new US_Archive();
   reset();

   connect(pb_load, &QPushButton::clicked, this, &US_LegacyConverter::load);
   connect(le_runid, &US_LineEdit_RE::textUpdated, this, &US_LegacyConverter::runid_updated);
   connect(le_dir, &QLineEdit::textChanged, this, &US_LegacyConverter::runid_updated);
   connect(pb_save, &QPushButton::clicked, this, &US_LegacyConverter::save_auc);
   connect(archive, &US_Archive::itemExtracted, this, &US_LegacyConverter::itemExtracted);
}

void US_LegacyConverter::runid_updated() {
   QDir dir = QDir(le_dir->text());
   QList<int> speed_list = data_map.keys();
   QString basename = le_runid->text();
   bool mspeed = speed_list.size() > 1;
   exists = false;
   bool br = false;
   foreach (int speed, speed_list) {
      foreach (QString rtype, data_map.value(speed).keys()) {
         QString path;
         if ( mspeed ) {
            path = tr("%1-%2-Speed%3").arg(basename, rtype).arg(speed);
         } else {
            path = tr("%1-%2").arg(basename, rtype);
         }
         if (dir.exists(path)) {
            exists = true;
            br = true;
            break;
         }
      }
      if (br) break;
   }
   if (exists) {
      lb_runid->setText("( Already Exists! ) Run ID:");
      le_runid->setStyleSheet("color: red;");
   } else {
      lb_runid->setText("Run ID:");
      le_runid->setStyleSheet("color: black;");
   }
}

void US_LegacyConverter::save_auc() {
   te_info->moveCursor(QTextCursor::End);
   if (all_data.isEmpty()) {
      QMessageBox::warning(this, "Warning!", "No Data Loaded!");
      return;
   }
   QString runid = le_runid->text();
   if (runid.isEmpty()) {
      QMessageBox::warning(this, "Error!", "No RunID Set!");
      return;
   }

   if (exists) {
      int state = QMessageBox::question(this, "Warning!", "RunID already exists!\n"
                                                          "Do you want to overwrite it?");
      if (state == QMessageBox::No) return;
   }
   qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
   pb_save->setDisabled(true);
   QDir dir = QDir(le_dir->text());
   QDir subdir = QDir();
   QString basename = le_runid->text();

   QString msg;
   QList<int> speed_list = data_map.keys();
   bool mspeed = speed_list.size() > 1;
   foreach (int speed, speed_list) {
      foreach (QString rtype, data_map.value(speed).keys()) {
         QString runid;
         if ( mspeed ) {
            runid = tr("%1-%2-Speed%3").arg(basename, rtype).arg(speed);
         } else {
            runid = tr("%1-%2").arg(basename, rtype);
         }
         QString path = dir.absoluteFilePath(runid);
         subdir.setPath(path);
         if (subdir.exists()) {
            subdir.removeRecursively();
         }
         subdir.mkpath(subdir.absolutePath());

         QVector< US_DataIO::RawData* > data;
         QList< US_Convert::TripleInfo > triples;
         QVector< US_Convert::Excludes > excludes;
         foreach (QString ccw, data_map.value(speed).value(rtype).keys()) {
            int idx = data_map.value(speed).value(rtype).value(ccw);
            triples << all_data.at(idx).triple;
            data << &all_data[idx].rdata;
            US_Convert::Excludes excl;
            excludes << excl;
         }

         int state = US_Convert::saveToDisk(data, triples, excludes, rtype, runid, path, false);
         if (state == US_Convert::OK) {
            msg += runid + "\n";
         } else {
            QMessageBox::warning(this, "Error!", "Failed to Save this RunID!\n\n" + runid);
            subdir.removeRecursively();
         }
      }
   }
   runid_updated();
   qApp->restoreOverrideCursor();
   if (! msg.isEmpty()) {
      QMessageBox::information(this, "Data Stored!", "Run(s) Successfully Saved!\n\n" + msg);
   }
   pb_save->setEnabled(true);
}

void US_LegacyConverter::reset(void) {
   le_runid->clear();
   lb_runid->setText("Run ID:");
   le_runid->setStyleSheet("color: black;");
   te_info->clear();
   all_data.clear();
   data_map.clear();
   counter = 0;
   exists = false;
   pb_save->setDisabled(true);
}

void US_LegacyConverter::load() {

   QString ext_str = "tar.gz Files ( *.tar.gz )";
   QString tar_fpath = QFileDialog::getOpenFileName(this, tr("Beckman Optima tar.gz File"), QDir::homePath(), ext_str);
   if (tar_fpath.isEmpty()){
      return;
   }

   reset();
   pb_load->setDisabled(true);
   le_load->clear();
   QRegularExpression re;
   re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
   QRegularExpressionMatch match;

   te_info->clear();
   te_info->append("Extracting the Archive File. Please Wait!");
   te_info->moveCursor(QTextCursor::End);
   qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
   qApp->processEvents();

   QFileInfo tar_finfo = QFileInfo(tar_fpath);
   qDebug() << "file path: " << tar_fpath;

   QTemporaryDir tmp_dir;
   QTemporaryDir tmp_dir_sorted;
   QString runid;
   if (tmp_dir.isValid()) {
      QString fpath = tar_finfo.absoluteFilePath();
      QString opath = tmp_dir.path();
      bool ok = archive->extract(fpath, opath);
      te_info->append("Process: Extracting File ...");
      qApp->processEvents();
      if (! ok) {
         te_info->clear();
         te_info->append( tr("Failed to Exctract the File: %1 \n").arg(tar_finfo.absoluteFilePath()) );
         te_info->append(archive->getError());
         tar_fpath.clear();
         qApp->restoreOverrideCursor();
         pb_load->setEnabled(true);
         return;
      }
      te_info->clear();
      te_info->append("Parsing Data. Please Wait!");
      qApp->processEvents();
      runid = tar_finfo.fileName().chopped(7);
   } else {
      QMessageBox::warning(this, "Error!", tr("FAILED to Create a Temporary Directory!"));
      tar_fpath.clear();
      qApp->restoreOverrideCursor();
      pb_load->setEnabled(true);
      return;
   }
   QStringList filelist;
   list_files(tmp_dir.path(), filelist);
   if (filelist.isEmpty()) {
      QMessageBox::warning(this, "Warning!", tr("Empty TGZ File!\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      te_info->clear();
      qApp->restoreOverrideCursor();
      pb_load->setEnabled(true);
      return;
   }
   if (! sort_files( filelist, tmp_dir_sorted.path() ) ) {
      QMessageBox::warning(this, "Warning!", tr("Incorrect Filename Pattern!\n\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      te_info->clear();
      qApp->restoreOverrideCursor();
      pb_load->setEnabled(true);
      return;
   }
   QString status;
   if(! read_beckman_files(tmp_dir_sorted.path(), status)) {
      qApp->restoreOverrideCursor();
      pb_load->setEnabled(true);
      return;
   }

   le_load->setText(tar_finfo.absoluteFilePath());
   le_runid->setText(runid);
   te_info->setText(status);
   te_info->moveCursor(QTextCursor::End);
   runid_updated();
   pb_save->setEnabled(true);
   pb_load->setEnabled(true);
   qApp->restoreOverrideCursor();
}

void US_LegacyConverter::list_files(const QString& path, QStringList& flist) {
   QDir dir(path);
   QDir::Filters filter = QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot | QDir::NoDotAndDotDot | QDir::NoSymLinks;
   QFileInfoList info_list = dir.entryInfoList(filter);
   foreach (const QFileInfo item, info_list) {
      if (item.isDir()) {
         list_files(item.absoluteFilePath(), flist);
      } else {
         flist << item.absoluteFilePath();
      }
   }
}

bool US_LegacyConverter::sort_files(const QStringList& flist, const QString& path) {
   QRegularExpression re;
   re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
   QRegularExpressionMatch match;
   // RunId1991-s0001-c2-s0009-w260-r_-n1.ri2
   // RunId1991-s0002-c1-s0001-n1.ip1
   //group1                    (.+)  =  runID
   //group2            -s(\\d{4,6})  =  speed
   //group3                 -c(\\d)  =  cell number
   //group4            -s(\\d{4,6})  =  scan number
   //group5      (?:-w(\\d{3})-r_)?  =  wavelength (optional: some files include it)
   //group6                 -n(\\d)  =  scan repetition
   //group7  (ra|ri|ip|fi|wa|wi)\\d  =  run type
   QString pattern = "^(.+)-s(\\d{4,6})-c(\\d)-s(\\d{4,6})(?:-w(\\d{3})-r_)?-n(\\d)[.](ra|ri|ip|fi|wa|wi)\\d$";
   re.setPattern(pattern);

   QString runid;
   QMap<QString, QVector<int>> scan_list; // speed-type-cell-wavelength-replica      -> scans
   QMap<QString, QString> fpath_list;     // speed-type-cell-wavelength-replica-scan -> filepath
   QVector<int> speed_list;

   foreach (QString fpath, flist) {
      QFileInfo finfo(fpath);
      QString fname = finfo.fileName();
      match = re.match(fname.toLower());
      if (match.hasMatch()) {
         if (runid.isEmpty()) {
            runid = match.captured(1);
         } else {
            if (QString::compare(runid, match.captured(1)) != 0) {
               QMessageBox::warning(this, "Error!", "Multiple Run IDs Found!");
               return false;
            }
         }
         int speed = match.captured(2).toInt();
         if (! speed_list.contains(speed)) {
            speed_list << speed;
            std::sort(speed_list.begin(), speed_list.end());
         }
         int speed_id = speed_list.indexOf(speed) + 1;
         QString cell = match.captured(3);
         int scan = match.captured(4).toInt();
         QString wavl = match.captured(5);
         if (wavl.isEmpty()) {
             wavl = "000";
         }
         QString replica = match.captured(6);
         QString runtype = match.captured(7);
         // speed-type-cell-wavelength-replica
         QString key1 = tr("%1-%2-%3-%4-%5").arg(speed_id).arg(runtype, cell, wavl, replica);
         if (scan_list.contains(key1)) {
            scan_list[key1] << scan;
         } else {
            QVector<int> ss(1, scan);
            scan_list.insert(key1, ss);
         }
         // speed-type-cell-wavelength-replica-scan
         QString key2 = tr("%1-%2").arg(key1).arg(scan);
         if (fpath_list.contains(key2)) {
            QMessageBox::warning(this, "Error!", tr("Some of scans are redundant!\n\n%1").arg(fname));
            return false;
         } else {
            fpath_list.insert(key2, fpath);
         }
      }
   }
   QDir dir = QDir(path);
   QDir subdir = QDir();
   QMapIterator<QString, QVector<int>> it(scan_list);
   bool state = true;
   while (it.hasNext()) {
      it.next();
      QString key1 = it.key();
      dir.mkdir(key1);
      subdir.setPath(dir.absoluteFilePath(key1));
      QVector<int> scans = it.value();
      std::sort(scans.begin(), scans.end());
      for (int ii = 0; ii < scans.size(); ii++) {
         QString key2 = tr("%1-%2").arg(key1).arg(scans.at(ii));
         QString fpath1 = fpath_list.value(key2);

         QString fname2 = tr("%1").arg(ii + 1).rightJustified(6, '0') + fpath1.right(4);
         if (! QFile::copy(fpath1, subdir.absoluteFilePath(fname2)) ) {
            state = false;
         }
      }
   }
   return state;
}

bool US_LegacyConverter::read_beckman_files(const QString& path, QString& status){
   QDir dir(path);
   QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
   QStringList replicates;
   foreach (QString path, subdirs) {
      QList<US_DataIO::BeckmanRawScan> rawscan;
      QString runtype;
      US_Convert::readLegacyData(dir.absoluteFilePath(path), rawscan, runtype);
      if (rawscan.size() == 0) {
         continue;
      }
      QVector< US_DataIO::RawData > rdata_list;
      QList< US_Convert::TripleInfo > triple_list;
      US_Convert::convertLegacyData(rawscan, rdata_list, triple_list, runtype, 0.5);

      // speed-type-cell-wavelength-replica
      int speed = path.split('-').at(0).toInt();

      QDir subd = QDir(dir.absoluteFilePath(path), "*", QDir::Name, QDir::Files);
      QString log_1;
      log_1 += tr("Speed : %1\n").arg(speed);
      log_1 += tr("Run Type : %1 (%2)\n").arg(data_types.value(runtype), runtype);
      log_1 += tr("Number of Parsed Files : %1\n").arg(subd.count());
      log_1 += tr("Number of Beckman Data Objects : %1\n").arg(rawscan.count());
      qDebug().noquote() << log_1;

      QString msg("Speed : %1, Run type : %2 (%3), Number of the processed files: %4");
      te_info->append(msg.arg(speed).arg(data_types.value(runtype), runtype).arg(subd.count()));
      te_info->moveCursor(QTextCursor::End);
      qApp->processEvents();

      for (int ii = 0; ii < triple_list.size(); ii ++) {
         US_Convert::TripleInfo triple = triple_list.at(ii);
         US_DataIO::RawData rdata = rdata_list.at(ii);
         int nscans  = rdata.scanCount();
         int npoints = rdata.pointCount();
         QString ccw = triple.tripleDesc;
         DataCrate dc;
         dc.rdata = rdata;
         dc.triple = triple;
         dc.n_replicates = 1;

         bool replicated = false;
         if (data_map.contains(speed))
         {
            if (data_map.value(speed).contains(runtype))
            {
               if (data_map.value(speed).value(runtype).contains(ccw))
               {
                  replicated = true;
                  int idx = data_map.value(speed).value(runtype).value(ccw);
                  int ns = all_data[idx].rdata.scanCount();
                  int np = all_data[idx].rdata.pointCount();
                  if ( np == npoints && ns == nscans )
                  {
                     for (int ss = 0; ss < nscans; ss++) {
                        for (int pp = 0; pp < npoints; pp++)
                        {
                           double val1 = all_data[idx].rdata.value(ss, pp);
                           double val2 = rdata.value(ss, pp);
                           all_data[idx].rdata.setValue(ss, pp, val1 + val2);
                        }
                     }
                     all_data[idx].n_replicates++;
                     QString rr = tr("%1-%2-%3").arg(speed).arg(runtype, ccw);
                     if (! replicates.contains(rr)) {
                        replicates << rr;
                     }
                  }
               }
               else
               {
                  all_data << dc;
                  int idx = all_data.size() - 1;
                  data_map[speed][runtype].insert(ccw, idx);
               }
            }
            else
            {
               all_data << dc;
               int idx = all_data.size() - 1;
               QHash< QString, int > m;
               m.insert(ccw, idx);
               data_map[speed].insert(runtype, m);
            }
         }
         else
         {
            all_data << dc;
            int idx = all_data.size() - 1;

            QHash< QString, int > m1;
            m1.insert(ccw, idx);

            QHash< QString, QHash< QString, int > > m2;
            m2.insert(runtype, m1);

            data_map.insert(speed, m2);
         }
         QString log_2 = tr( "Triple : %1 ; Number of Scans : %2\n").arg(ccw).arg(nscans);
         qDebug().noquote() << log_2;
         if ( ii == 0 && !replicated ) {
            status += log_1 + log_2;
         } else if ( !replicated ) {
            status += log_2;
         }
      }
      qDebug().noquote() << "------------------------------\n";
      status += "------------------------------\n";
   }
   replicates.sort();
   if ( ! replicates.isEmpty() ) {
      QString details;
      foreach (QString key, replicates) {
         QStringList ksp = key.split("-");
         int speed = ksp.at(0).toInt();
         QString runtype = ksp.at(1);
         QString ccw = ksp.at(2);
         int idx = data_map.value(speed).value(runtype).value(ccw);
         int nscans = all_data[idx].rdata.scanCount();
         int npoints = all_data[idx].rdata.pointCount();
         double n_replicas = all_data[idx].n_replicates;
         details += tr("Speed: %1, Type: %2, Triple: %3, Number of Replicates: %4\n").arg(speed).arg(runtype, ccw).arg(n_replicas);
         for (int ss = 0; ss < nscans; ss++) {
            for (int pp = 0; pp < npoints; pp++)
            {
               double val = all_data[idx].rdata.value(ss, pp);
               all_data[idx].rdata.setValue(ss, pp, val / n_replicas);
            }
         }
      }

      QMessageBox msgBox(this);
      msgBox.setIcon(QMessageBox::Warning);
      msgBox.setWindowTitle("Warning");
      msgBox.setText("Your experiment contains replicates. "
                     "Replicates will be averaged and saved "
                     "as a single scan. \nAveraging multiple scans "
                     "produces suboptimal velocity experiments and "
                     "replicates should not be used for analysis.");
      msgBox.setInformativeText("Please check the details below.");
      msgBox.setDetailedText(details);
      msgBox.setStandardButtons(QMessageBox::Ok);
      msgBox.exec();
   }
   return true;
}


void US_LegacyConverter::itemExtracted(const QString& relative, const QString&) {
   te_info->append(relative);
   if ( counter % 10 == 0) qApp->processEvents();
   counter++;
}
