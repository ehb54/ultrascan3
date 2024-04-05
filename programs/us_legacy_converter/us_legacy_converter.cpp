#include "us_legacy_converter.h"
#include "us_license_t.h"
#include "us_license.h"
//#include "us_extern.h"

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
   this->setMinimumSize(600, 500);
   this->setMaximumSize(600, 500);

   data_types.insert("RI", "Intensity");
   data_types.insert("RA", "Absorbance");
   data_types.insert("WI", "Intensity");
   data_types.insert("WA", "Absorbance");
   data_types.insert("IP", "Interference");
   data_types.insert("FI", "Fluorensce");

   pb_load = us_pushbutton("Load Optima File (tar.gz)");
   pb_load->setMinimumWidth(150);
   le_load = us_lineedit("", -1, true);

   lb_runid = us_label("Run ID:");
   lb_runid->setAlignment(Qt::AlignRight);
   le_runid = new US_LineEdit_RE("");

   QLabel *lb_runtype = us_label("Run Type:");
   lb_runtype->setAlignment(Qt::AlignRight);
   cb_runtype = us_comboBox();
   pb_save = us_pushbutton("Save", true, 0);

   QLabel* lb_tolerance = us_label("Separation Tolerance:");
   lb_tolerance->hide();
   lb_tolerance->setAlignment(Qt::AlignRight);
   ct_tolerance = us_counter ( 2, 0.0, 100.0, 5.0 );
   ct_tolerance->setSingleStep( 1 );
   ct_tolerance->hide();
   pb_reload = us_pushbutton("Reload");
   pb_reload->hide();

   te_info = us_textedit();
   te_info->setReadOnly(true);

   pb_save->setMinimumWidth(lb_tolerance->sizeHint().width());

   QGridLayout *layout = new QGridLayout();
   layout->addWidget(pb_load,  0, 0, 1, 1);
   layout->addWidget(le_load,  0, 1, 1, 2);
   layout->addWidget(lb_runid, 1, 0, 1, 1);
   layout->addWidget(le_runid, 1, 1, 1, 2);
   layout->addWidget(lb_runtype, 2, 0, 1, 1);
   layout->addWidget(cb_runtype, 2, 1, 1, 1);
   layout->addWidget(pb_save,    2, 2, 1, 1);
   layout->addWidget(lb_tolerance, 3, 0, 1, 1);
   layout->addWidget(ct_tolerance, 3, 1, 1, 1);
   layout->addWidget(pb_reload,    3, 2, 1, 1);
   layout->addWidget(te_info, 4, 0, 4, 3);
   layout->setMargin(3);
   layout->setSpacing(3);
   this->setLayout(layout);

   connect(pb_load, &QPushButton::clicked, this, &US_LegacyConverter::load);
   connect(pb_reload, &QPushButton::clicked, this, &US_LegacyConverter::reload);
   connect(le_runid, &US_LineEdit_RE::textUpdated, this, &US_LegacyConverter::runid_updated);
   connect(pb_save, &QPushButton::clicked, this, &US_LegacyConverter::save_auc);
   connect(ct_tolerance, &QwtCounter::valueChanged, this, &US_LegacyConverter::new_tolerance);
}

void US_LegacyConverter::new_tolerance(double){
   reset();
   if (! tar_fpath.isEmpty()){
      te_info->setText("Reload the current file or load another file!");
   }
}

void US_LegacyConverter::runid_updated() {
   QDir dir = QDir(US_Settings::importDir());
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
   te_info->moveCursor(QTextCursor::End);
   if (all_data.isEmpty()) {
      QMessageBox::warning(this, "Warning!", "No Data Loaded!");
      return;
   }
   QString runid = le_runid->text();
   if (runid.isEmpty()) {
      QMessageBox::warning(this, "Error!", "No Run ID Set!");
      return;
   }
   QDir dir = QDir(US_Settings::importDir());
   dir.setPath(dir.absoluteFilePath(runid));
   if (dir.exists()) {
      QMessageBox::StandardButton state;
      state = QMessageBox::question(this, "Directory Exists!", "The output directory exists!\n"
                            + dir.absolutePath() + "\nBy clicking on 'YES', all data will be overwritten!"
                                                           + "\nDo you want to proceed?");
      if (state == QMessageBox::No) {
         return;
      } else {
         dir.removeRecursively();
      }
   }
   dir.mkdir(dir.absolutePath());
   QMapIterator<QString, QString> it(data_types);
   QString rtype;
   while (it.hasNext()) {
      it.next();
      if (QString::compare(it.value(), cb_runtype->currentText(), Qt::CaseInsensitive) == 0) {
         rtype = it.key();
         break;
      }
   }
   QVector< US_DataIO::RawData* > data;
   QList< US_Convert::TripleInfo > triples;
   QVector< US_Convert::Excludes > excludes;
   QMapIterator< QString, US_Convert::TripleInfo > it_triple(all_triples);
   QString msg = tr("Saving the %1 OpenAuc files:\n").arg(cb_runtype->currentText());
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
      QMessageBox::information(this, "Data Saved!", cb_runtype->currentText() +
                               " data saved in \n\n" + dir.absolutePath());
   } else {
      QMessageBox::warning(this, "Error!", "Data wasn't saved! Try again!");
   }
   runid_updated();
}

void US_LegacyConverter::reset(void) {
   le_runid->clear();
   lb_runid->setText("Run ID:");
   le_runid->setStyleSheet("color: black;");
   te_info->clear();
   cb_runtype->clear();
   all_data.clear();
   all_triples.clear();
}

void US_LegacyConverter::load() {
   QString ext_str = "TAR Files ( *.tar.gz *.tar )";
   QString fpath = QFileDialog::getOpenFileName(this, tr("Beckman Optima Tar File"), QDir::homePath(), ext_str);
   if (fpath.size() == 0){
      return;
   }
   tar_fpath = fpath;
   reload();
}

void US_LegacyConverter::reload() {
   reset();
   le_load->clear();
   QRegularExpression re;
   re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
   QRegularExpressionMatch match;

   te_info->clear();
   te_info->append("Parsing Data. Please Wait!");
   te_info->moveCursor(QTextCursor::End);
   qApp->setOverrideCursor(QCursor(Qt::WaitCursor));
   qApp->processEvents();
   if (tar_fpath.size() == 0) {
      QMessageBox::warning(this, "Warning!", tr("No File Loaded!"));
      qApp->restoreOverrideCursor();
      return;
   }
   QFileInfo tar_finfo = QFileInfo(tar_fpath);
   if (! tar_finfo.exists()) {
      QMessageBox::warning(this, "Error!", tr("File Not Found!\n\n(%1)!").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      qApp->restoreOverrideCursor();
      return;
   }
   qDebug() << "file path: " << tar_fpath;

   QTemporaryDir tmp_dir;
   QTemporaryDir tmp_dir_sorted;
   QString tmpfile = tmp_dir.filePath("data.tar");
   QString runid;
   bool unzip = false;
   if (tmp_dir.isValid()) {
      if (tar_finfo.absoluteFilePath().endsWith(".tar.gz", Qt::CaseInsensitive)) {
         tmpfile += ".gz";
         unzip = true;
         runid = tar_finfo.fileName().chopped(7);
      } else {
         runid = tar_finfo.fileName().chopped(4);
      }
      qDebug() << tmpfile;
      if (QFile::copy(tar_finfo.absoluteFilePath(), tmpfile)) {
         if (unzip) {
            te_info->append("Process: Extracting ZIP File.");
            qApp->processEvents();
            US_Gzip gzip;
            int state = gzip.gunzip(tmpfile);
            if (state != 0) {
               QMessageBox::warning(this, "Error!", "Failed to extract the ZIP file!\n" +
                                                        gzip.explain(state));
               qApp->restoreOverrideCursor();
               tar_fpath.clear();
               return;
            }
            tmpfile.chop(3);
            qDebug() << tmpfile;
         }
         te_info->append("Process: Extracting TAR data.");
         qApp->processEvents();
         US_Tar ustar;
         QStringList extlist;
         int state = ustar.extract(tmpfile, tmp_dir.path(), &extlist);
         if (state != 0) {
            QMessageBox::warning(this, "Error!", "Failed to extract the TAR file!\n" +
                                                     ustar.explain(state));
            qApp->restoreOverrideCursor();
            tar_fpath.clear();
            return;
         }
      } else {
         QMessageBox::warning(this, "Error!", tr("FAILED to copy the file to the /tmp directory!"));
         tar_fpath.clear();
         qApp->restoreOverrideCursor();
         return;
      }
   } else {
      QMessageBox::warning(this, "Error!", tr("FAILED to create a /tmp directory!"));
      tar_fpath.clear();
      qApp->restoreOverrideCursor();
      return;
   }
   QStringList filelist;
   list_files(tmp_dir.path(), filelist);
   if (filelist.size() == 0) {
      QMessageBox::warning(this, "Warning!", tr("Empty File!\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
      qApp->restoreOverrideCursor();
      return;
   }
   if (! sort_files( filelist, tmp_dir_sorted.path() ) ) {
      QMessageBox::warning(this, "Warning!", tr("No right files found in the TAR file!\n\n(%1)").arg(tar_finfo.absoluteFilePath()));
      tar_fpath.clear();
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
   QStringList loaded_types;
   QMapIterator< QString, US_Convert::TripleInfo > it(all_triples);
   while (it.hasNext()) {
      it.next();
      QString dtype = it.key().split(':').at(0).trimmed();
      if (! loaded_types.contains(dtype)){
         loaded_types << dtype;
      }
   }
   foreach (QString key, loaded_types) {
      cb_runtype->addItem(data_types.value(key));
   }
   te_info->setText(status);
   te_info->moveCursor(QTextCursor::End);
   runid_updated();
   qApp->restoreOverrideCursor();
}

void US_LegacyConverter::list_files(const QString& path, QStringList& flist) {
   QDir dir(path);
   QStringList filter;
   QStringList tmp_list = dir.entryList(QStringList({"*"}), QDir::Files | QDir::NoSymLinks);
   foreach (const QString& fname, tmp_list) {
      QFileInfo fileInfo(dir.absoluteFilePath(fname));
      flist.append(fileInfo.absoluteFilePath());
   }

   // Recursively process subdirectories
   QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoDotAndDotDot | QDir::NoSymLinks);
   foreach (const QString& subdir, subdirs) {
      QString subdirPath = dir.absoluteFilePath(subdir);
      list_files(subdirPath, flist);
   }
}

bool US_LegacyConverter::sort_files(const QStringList& flist, const QString& tmpDir) {
   QRegularExpression re;
   re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
   QRegularExpressionMatch match;

   QString runid;
   QString runtype;
   QMap<QString, QString> file_map;
   QMap<QString, QVector<int>> tcws_map;
   foreach (QString fpath, flist) {
      QFileInfo finfo = QFileInfo(fpath);
      QFile file(fpath);
      QString fname = finfo.fileName();

      // RunId1991-s0001-c2-s0009-w260-r_-n1.ri2
      re.setPattern("^(.+)-s(\\d{4,6})-c(\\d)-s(\\d{4,6})-w(\\d{3})-(.+?)[.](?:RA|RI|IP|FI|WA|WI)\\d$");
      match = re.match(fname);
      if (match.hasMatch()) {
         if (runid.size() == 0) {
            runid = match.captured(1);
         } else {
            if (QString::compare(runid, match.captured(1)) != 0) {
               QMessageBox::warning(this, "Error!", "Multiple run IDs found!");
               return false;
            }
         }
         runtype = fname.right(3).left(2).toUpper();
         QString cell = match.captured(3);
         int scan = match.captured(4).toInt();
         QString wavl = match.captured(5);
         QString tcw = runtype + "-" + cell + "-" + wavl;
         if (tcws_map.contains(tcw)) {
            tcws_map[tcw] << scan;
         } else {
            QVector<int> ss(1, scan);
            tcws_map.insert(tcw, ss);
         }
         QString tcws = tcw + "-" + QString::number(scan);
         if (file_map.contains(tcws)) {
            QMessageBox::warning(this, "Error!", "Scan number redundancy!");
            return false;
         } else {
            file_map.insert(tcws, fpath);
         }
      }
   }
   QDir dir = QDir(tmpDir);
   QMapIterator<QString, QVector<int>> it(tcws_map);
   bool state = false;
   while (it.hasNext()) {
      it.next();
      QString tcw = it.key();
      dir.mkdir(tcw);
      QDir subdir = QDir(dir.absoluteFilePath(tcw));
      QVector<int> scans = it.value();
      std::sort(scans.begin(), scans.end());
      QFileInfo finfo = QFileInfo();
      for (int ii = 0; ii < scans.size(); ii++) {
         QString ss = QString::number(scans.at(ii));
         QString fpath1 = file_map.value(tcw + "-" + ss);
         QString fname2 = ss.rightJustified(5, '0') + fpath1.right(4);
         finfo.setFile(subdir, fname2);
         if (QFile::copy(fpath1, finfo.absoluteFilePath())) state = true;
      }
   }
   return state;
}

bool US_LegacyConverter::read_beckman_files(const QString& path, QString& status){
   QDir tmpdir(path);
   QStringList subdirs = tmpdir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
   int counter = 1;
   double tolerance = static_cast<double>(ct_tolerance->value());
   foreach (QString path, subdirs) {
      QList<US_DataIO::BeckmanRawScan> rawscan;
      QString runtype;
      US_Convert::readLegacyData(tmpdir.absoluteFilePath(path), rawscan, runtype);
      if (rawscan.size() == 0) {
         continue;
      }

      QVector< US_DataIO::RawData  > rawdata;
      QList< US_Convert::TripleInfo > triples;
      US_Convert::convertLegacyData(rawscan, rawdata, triples, runtype, tolerance);

      QDir subd = QDir(tmpdir.absoluteFilePath(path), "*", QDir::Name, QDir::Files);
      status += QString::number(counter) + ":\n";
      counter ++;
      status += tr("Run type: %1 (%2)\n").arg(data_types.value(runtype), runtype);
      status += tr("Number of the parsed files: %1\n").arg(subd.count());
      status += tr("Number of the beckman data objects: %1\n").arg(rawscan.count());

      QString msg("Run type: %1 (%2), Number of the processed files: %3");
      te_info->append(msg.arg(data_types.value(runtype), runtype).arg(subd.count()));
      te_info->moveCursor(QTextCursor::End);
      qApp->processEvents();

      for (int ii = 0; ii < triples.size(); ii ++) {
         QString tdesc = triples.at(ii).tripleDesc.trimmed();
         QString key = tr("%1:%2").arg(runtype, tdesc);
         if (all_triples.contains(key)){
            QMessageBox::warning(this, "Error!", "Triple redundancy!");
            qDebug().noquote() << status;
            return false;
         }
         all_triples.insert(key, triples.at(ii));
         all_data.insert(key, rawdata.at(ii));
         status += tr("Triple: %1   # Scans: %2\n").arg(tdesc).arg(rawdata.at(ii).scanData.count());
      }
      status += "------------------------------\n";
      // qApp->processEvents();
   }
   qDebug().noquote() << status;
   return true;
}
