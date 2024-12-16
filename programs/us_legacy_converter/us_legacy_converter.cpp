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
   data_types.insert("WI", "Intensity");
   data_types.insert("WA", "Absorbance");
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
   layout->setMargin(2);
   layout->setSpacing(2);
   this->setLayout(layout);

   archive = new US_Archive();
   counter = 0;

   connect(pb_load, &QPushButton::clicked, this, &US_LegacyConverter::load);
   connect(le_runid, &US_LineEdit_RE::textUpdated, this, &US_LegacyConverter::runid_updated);
   connect(le_dir, &QLineEdit::textChanged, this, &US_LegacyConverter::runid_updated);
   connect(pb_save, &QPushButton::clicked, this, &US_LegacyConverter::save_auc);
   connect(archive, &US_Archive::itemExtracted, this, &US_LegacyConverter::itemExtracted);
}

void US_LegacyConverter::runid_updated() {
   QDir dir = QDir(le_dir->text());
   dir.setPath(dir.absoluteFilePath(le_runid->text()));
   if (dir.exists()) {
      lb_runid->setText("( existing )    Run ID:");
      le_runid->setStyleSheet("color: red;");
   } else {
      lb_runid->setText("Run ID:");
      le_runid->setStyleSheet("color: black;");
   }
}

void US_LegacyConverter::save_auc() {
   // RI -> Intensity
   // RA -> Absorbance
   // WI -> Intensity
   // WA -> Absorbance
   // IP -> Interference
   // FI -> Fluorensce
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
   QDir dir = QDir(le_dir->text());
   dir.setPath(dir.absoluteFilePath(runid));
   if (dir.exists()) {
      QMessageBox::StandardButton state;
      state = QMessageBox::question(this, "Warning!", "The output directory exists!\n\n"
                            + dir.absolutePath() + "\n\nBy clicking on 'YES', all data will be overwritten! "
                                                           + "Do you want to proceed?");
      if (state == QMessageBox::No) {
         return;
      } else {
         dir.removeRecursively();
      }
   }
   dir.mkdir(dir.absolutePath());
   QMapIterator<QString, QString> it(data_types);
   QString rtype;
   // while (it.hasNext()) {
   //    it.next();
   //    if (QString::compare(it.value(), cb_runtype->currentText(), Qt::CaseInsensitive) == 0) {
   //       rtype = it.key();
   //       break;
   //    }
   // }
   QVector< US_DataIO::RawData* > data;
   QList< US_Convert::TripleInfo > triples;
   QVector< US_Convert::Excludes > excludes;
   QMapIterator< QString, US_Convert::TripleInfo > it_triple(all_triples);
   // QString msg = tr("Saving the %1 OpenAuc files:\n").arg(cb_runtype->currentText());
   QString msg;
   msg += dir.absolutePath() + "\n";
   while (it_triple.hasNext()) {
      it_triple.next();
      if (QString::compare(it_triple.key().split(':').at(0), rtype) == 0){
         triples << it_triple.value();
         US_DataIO::RawData *rdp;
         US_DataIO::RawData rd = all_data[it_triple.key()];
         rdp = &all_data[it_triple.key()];
         data << rdp;
         US_Convert::Excludes excl;
         excludes << excl;
         msg += it_triple.key().split(':').at(1).trimmed() + "\n";
      }
   }
   msg += "------------------------------\n";
   int state = US_Convert::saveToDisk(data, triples, excludes, rtype, runid, dir.absolutePath(), false);
   if (state == US_Convert::OK) {
      te_info->insertPlainText(msg);
      te_info->moveCursor(QTextCursor::End);
      // QMessageBox::information(this, "Data Saved!", cb_runtype->currentText() +
      //                          " data saved in \n\n" + dir.absolutePath());
   } else {
      QMessageBox::warning(this, "Error!", "Data cannot be saved! Check the output directory!");
   }
   runid_updated();
}

void US_LegacyConverter::reset(void) {
   le_runid->clear();
   lb_runid->setText("Run ID:");
   le_runid->setStyleSheet("color: black;");
   te_info->clear();
   all_data.clear();
   all_triples.clear();
}

void US_LegacyConverter::load() {

   QString ext_str = "tar.gz Files ( *.tar.gz )";
   QString tar_fpath = QFileDialog::getOpenFileName(this, tr("Beckman Optima tar.gz File"), QDir::homePath(), ext_str);
   if (tar_fpath.isEmpty()){
      return;
   }

   reset();
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
         return;
      }
      te_info->clear();
      te_info->append("Parsing Data. Please Wait!");
      counter = 0;
      qApp->processEvents();
      runid = tar_finfo.fileName().chopped(7);
   } else {
      QMessageBox::warning(this, "Error!", tr("FAILED to Create a Temporary Directory!"));
      tar_fpath.clear();
      qApp->restoreOverrideCursor();
      return;
   }
   QStringList filelist;
   list_files(tmp_dir.path(), filelist);
   if (filelist.isEmpty()) {
      QMessageBox::warning(this, "Warning!", tr("Empty TGZ File!\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      te_info->clear();
      qApp->restoreOverrideCursor();
      return;
   }
   if (! sort_files( filelist, tmp_dir_sorted.path() ) ) {
      QMessageBox::warning(this, "Warning!", tr("Incorrect Filename Pattern!\n\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      te_info->clear();
      qApp->restoreOverrideCursor();
      return;
   }
   QString status;
   if(! read_beckman_files(tmp_dir_sorted.path(), status)) {
      qApp->restoreOverrideCursor();
      return;
   }
   le_load->setText(tar_finfo.absoluteFilePath());
   le_runid->setText(runid);
   te_info->setText(status);
   te_info->moveCursor(QTextCursor::End);
   runid_updated();
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

bool US_LegacyConverter::sort_files(const QStringList& flist, const QString& tmpDir) {
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
   //group6  (ra|ri|ip|fi|wa|wi)\\d  =  run type
   QString pattern = "^(.+)-s(\\d{4,6})-c(\\d)-s(\\d{4,6})(?:-w(\\d{3})-r_)?-n\\d[.](ra|ri|ip|fi|wa|wi)\\d$";
   re.setPattern(pattern);

   QString runid;
   QMap<QString, QVector<int>> stcw_scans; // speed-type-cell-wavelength -> scans
   QMap<QString, QString> file_map;  // speed-type-cell-wavelength-scan -> filepath
   QVector<int> speed_list;

   foreach (QString fpath, flist) {
      QFileInfo finfo = QFileInfo(fpath);
      QFile file(fpath);
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
         }
         QString cell = match.captured(3);
         int scan = match.captured(4).toInt();
         QString wavl = match.captured(5);
         if (wavl.isEmpty()) {
             wavl = "000";
         }
         QString runtype = match.captured(6);
         QString key1 = tr("%1-%2-%3-%4").arg(speed_list.indexOf(speed) + 1).arg(runtype, cell, wavl);
         if (stcw_scans.contains(key1)) {
            stcw_scans[key1] << scan;
         } else {
            QVector<int> ss(1, scan);
            stcw_scans.insert(key1, ss);
         }
         QString key2 = tr("%1-%2").arg(key1).arg(scan);
         if (file_map.contains(key2)) {
            QMessageBox::warning(this, "Error!", tr("Redundancy In Scans!\%1").arg(fname));
            return false;
         } else {
            file_map.insert(key2, fpath);
         }
      }
   }
   QDir dir = QDir(tmpDir);
   QDir subdir = QDir();
   QMapIterator<QString, QVector<int>> it(stcw_scans);
   bool state = true;
   while (it.hasNext()) {
      it.next();
      QString key1 = it.key();
      dir.mkdir(key1);
      subdir.setPath(dir.absoluteFilePath(key1));
      QVector<int> scans = it.value();
      std::sort(scans.begin(), scans.end());
      // QFileInfo finfo = QFileInfo();
      for (int ii = 0; ii < scans.size(); ii++) {
         QString key2 = tr("%1-%2").arg(key1).arg(scans.at(ii));
         QString fpath1 = file_map.value(key2);
         QString fname2 = tr("%1.%2").arg(ii + 1).arg(fpath1.right(3));
         // finfo.setFile(subdir, fname2.rightJustified(9, '0'));
         if (! QFile::copy(fpath1, subdir.absoluteFilePath(fname2.rightJustified(9, '0'))) ) {
            state = false;
         }
      }
   }
   return state;
}

bool US_LegacyConverter::read_beckman_files(const QString& path, QString& status){
   QDir tmpdir(path);
   QStringList subdirs = tmpdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
   foreach (QString path, subdirs) {
      QList<US_DataIO::BeckmanRawScan> rawscan;
      QString runtype;
      US_Convert::readLegacyData(tmpdir.absoluteFilePath(path), rawscan, runtype);
      if (rawscan.size() == 0) {
         continue;
      }
      QString speed = path.split('-').at(0);
      QVector< US_DataIO::RawData  > rawdata;
      QList< US_Convert::TripleInfo > triples;
      US_Convert::convertLegacyData(rawscan, rawdata, triples, runtype, 0.5);

      QDir subd = QDir(tmpdir.absoluteFilePath(path), "*", QDir::Name, QDir::Files);
      status += tr("RPM : %1\n").arg(speed);
      status += tr("Run Type : %1 (%2)\n").arg(data_types.value(runtype), runtype);
      status += tr("Number of Parsed Files : %1\n").arg(subd.count());
      status += tr("Number of Beckman Data Objects : %1\n").arg(rawscan.count());

      QString msg("RPM:%1, Run type: %2 (%3), Number of the processed files: %4");
      te_info->append(msg.arg(speed, data_types.value(runtype), runtype).arg(subd.count()));
      te_info->moveCursor(QTextCursor::End);
      qApp->processEvents();

      for (int ii = 0; ii < triples.size(); ii ++) {
         QStringList ccw = triples.at(ii).tripleDesc.split('/');
         // speed-runtype-cell-channel-lambda
         QString key = tr("%1-%2-%3-%4-%5").arg(speed, runtype, ccw.at(0).trimmed(),
                                                ccw.at(1).trimmed(), ccw.at(2).trimmed());
         if (all_triples.contains(key)){
            QMessageBox::warning(this, "Error!", "Triple Redundancy!");
            qDebug().noquote() << status;
            return false;
         }
         all_triples.insert(key, triples.at(ii));
         all_data.insert(key, rawdata.at(ii));
         status += tr( "Triple: %1   # Scans: %2\n").arg(triples.at(ii).tripleDesc)
                      .arg(rawdata.at(ii).scanData.count() );
      }
      status += "------------------------------\n";
      // qApp->processEvents();
   }
   qDebug().noquote() << status;
   return true;
}


void US_LegacyConverter::itemExtracted(const QString& relative, const QString&) {
   te_info->append(relative);
   if ( counter % 10 == 0) qApp->processEvents();
   counter++;
}
