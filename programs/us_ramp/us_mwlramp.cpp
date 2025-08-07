//! \file us_ramp.cpp

#include "us_mwlramp.h"

#include "us_editor.h"
#include "us_settings.h"
#include "us_util.h"

US_mwlRamp::US_mwlRamp() : QObject() {}

bool US_mwlRamp::import_data(QString& mwldir, QLineEdit* lestat,
                             QVector<US_mwlRamp::RampRawData>& allData,
                             QString& runID_xml_mwl) {
  bool status = true;
  cur_dir = mwldir;
  le_status = lestat;
  //    qDebug() << cur_dir;

  QDir ddir(cur_dir, "*", QDir::Name, QDir::Files | QDir::Readable);
  //    qDebug() << "before make absolute" << ddir;
  ddir.makeAbsolute();
  if (cur_dir.right(1) != "/") cur_dir += "/";

  // read xml
  read_runxml(ddir, cur_dir);
  runID_xml_mwl = runID;

  int kcelchn = cellchans.count();
  cellchans.clear();

  QString old_runID = runID;
  runID.replace(QRegExp("![A-Za-z0-9_-]"), "_");

  if (runID != old_runID) {
    QMessageBox::warning(
        0, tr("RunID Name Changed"),
        tr("The runID name has been changed. It may consist only\n"
           "of alphanumeric characters, underscore, or hyphen.\n"
           "  New runID: ") +
            runID);
  }

  // Read in the data
  QStringList mwrfs =
      ddir.entryList(QStringList("*.mwsr"), QDir::Files, QDir::Name);
  QStringList chans;
  cells.clear();
  mwrfs.sort();
  nfile = mwrfs.size();
  le_status->setText(QString("%1 files from %2 ...").arg(nfile).arg(runID));
  qApp->processEvents();
  qDebug() << "MwDa: nfile" << nfile;

  // read from measurement-filenames
  for (int ii = 0; ii < nfile; ii++) {
    QString fname = mwrfs.at(ii);
    QString fpath = cur_dir + fname;
    QString acell = fname.section(".", -3, -3);
    QString chann = fname.section(".", -2, -2);

    //         qDebug() << "\nacell" << acell <<  "\nchann" << chann;

    if (!cells.contains(acell)) cells << acell;
    if (!chans.contains(chann)) chans << chann;

    fnames << fname;
    fpaths << fpath;
  }

  int ncell = cells.size();
  int nchan = chans.size();
  //     qDebug() << "MwDa Ramp: ncell nchan" << ncell << nchan;
  ncelchn = ncell * nchan;
  if (ncelchn != kcelchn) {
    qDebug() << "kcelchn ncelchn" << kcelchn << ncelchn;
  }
  cells.sort();
  chans.sort();
  cellchans.clear();
  le_status->setText(
      QString("%1 cell(s) and %2 channel(s) ...").arg(ncell).arg(nchan));
  qApp->processEvents();

  for (int ii = 0; ii < ncell; ii++) {
    for (int jj = 0; jj < nchan; jj++) {
      QString celchn = cells[ii] + " / " + chans[jj];
      cellchans << celchn;
    }
  }

  for (int cc = 0; cc < cellchans.size(); cc++) {
    qDebug() << cellchans.at(cc) << fpaths.at(cc);

    QFile fi(fpaths.at(cc));
    if (!fi.open(QIODevice::ReadOnly)) {
      qDebug() << "*ERROR* Unable to open" << fpaths.at(cc);
      status = false;
    }

    QDataStream ds(&fi);
    RampRawData raw;

    char cbuf[2];
    ds.readRawData(cbuf, 2);
    raw.cell = QChar('0' | cbuf[0]);
    raw.chan = QChar(cbuf[1]);
    qint16 temp;
    ds >> temp;
    raw.temperature = ((double)temp) / 10;
    qDebug() << "raw.cell" << raw.cell << raw.chan << raw.temperature;
    qint16 n;
    ds >> n;
    n = n + 1;
    qDebug() << "#scans" << n;
    qint16 n_wl;  //! Number of Wavelengths
    ds >> n_wl;
    qDebug() << "#wl" << n_wl;

    qint16 wl_array_temp;
    raw.wl_array.resize(n_wl);
    for (int ii = 0; ii < n_wl; ii++) {
      ds >> wl_array_temp;
      raw.wl_array[ii] = (int)wl_array_temp;
    }

    raw.rpm.resize(n);
    for (int ii = 0; ii < n; ii++) {
      ds >> raw.rpm[ii];
    }

    raw.time.resize(n);
    for (int ii = 0; ii < n; ii++) {
      ds >> raw.time[ii];
    }

    raw.w2t.resize(n);
    for (int ii = 0; ii < n; ii++) {
      ds >> raw.w2t[ii];
    }
    qDebug() << "n_wl n" << n_wl << n;
    // (1) initialize 2Darray: [wl] [data]
    raw.intarray.resize(n_wl);
    for (int wl = 0; wl < n_wl; wl++) {
      raw.intarray[wl].resize(n);
    }
    // (2) read data

    qDebug() << "sizeintarray" << raw.intarray.size()
             << "raw.intarray[cc].size()" << raw.intarray[cc].size();
    for (int wl = 0; wl < n_wl; wl++) {
      for (int dat = 0; dat < n; dat++) {
        ds >> raw.intarray[wl][dat];
      }
    }

    slambda = raw.wl_array[0];
    elambda = raw.wl_array[n_wl - 1];
    qDebug() << slambda << elambda << "slambda << elambda";
    nlamb_i = n_wl;

    raw.bottom = bottoms[cc];
    raw.meniscus = menisci[cc];
    raw.radius = radius;
    raw.description = samplenames[cc];

    allData.resize(cellchans.size());
    allData[cc] = raw;
    qDebug() << "alldat" << allData[cc].chan << "temp"
             << allData[cc].temperature;
  }

  for (int ii = 0; ii < nlamb_i; ii++) {
    ri_wavelns.append(int(allData[0].wl_array[ii]));
  }
  qDebug() << "ri" << ri_wavelns.size() << "raw" << allData[0].wl_array.size();

  for (int cc = 0; cc < ncelchn; cc++) {
    ex_wavelns << ri_wavelns;
  }

  return status;
}

// Read the run XML file and return its values
void US_mwlRamp::read_runxml(QDir ddir, QString curdir) {
  QStringList mwrfs =
      ddir.entryList(QStringList("*.mwsr.xml"), QDir::Files, QDir::Name);
  int nxfile = mwrfs.count();

  if (nxfile > 1) {
    qDebug() << "*ERROR* '*.mwsr.xml' count > 1" << nxfile << curdir;
    return;
  }
  qDebug() << mwrfs.size();
  QString fname = mwrfs.at(0);
  QString fpath = curdir + fname;

  QFile xfi(fpath);

  if (!xfi.open(QIODevice::ReadOnly)) {
    qDebug() << "*ERROR* Unable to open" << fname;
    qDebug() << fpath;
    return;
  }

  QXmlStreamReader xml(&xfi);
  QString celi;
  QString chni;
  QString cech;
  double menisc;
  double bott;
  cellchans.clear();

  while (!xml.atEnd()) {
    xml.readNext();
    if (xml.isStartElement()) {
      QXmlStreamAttributes att = xml.attributes();
      if (xml.name() == "runID") {
        runID = att.value("name").toString();
        radius = att.value("radius_measurement").toString().toDouble();
      } else if (xml.name() == "cell") {
        celi = att.value("id").toString();
        celi = QString::number(celi.toInt());
      } else if (xml.name() == "channel") {
        chni = att.value("id").toString();
        cech = celi + " / " + chni;
        QString desc = att.value("sample").toString();
        menisc = att.value("radius_meniscus").toString().toDouble();
        bott = att.value("radius_bottom").toString().toDouble();

        if (!cellchans.contains(cech)) {
          cellchans << cech;
          samplenames << desc;
          menisci << menisc;
          bottoms << bott;
        }
      }
    }
  }
  xfi.close();
}

// Clear all data from US_mwlRamp
void US_mwlRamp::clear(void) {
  runID.clear();
  cellchans.clear();
  samplenames.clear();
  menisci.clear();
  bottoms.clear();
  ri_wavelns.clear();
  ex_wavelns.clear();
  fpaths.clear();
  fnames.clear();
}
