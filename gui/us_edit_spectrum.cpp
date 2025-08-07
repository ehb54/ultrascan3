//! \file us_solution_gui.cpp

#include "us_edit_spectrum.h"

#include "us_analyte_gui.h"
#include "us_buffer_gui.h"
#include "us_db2.h"
#include "us_editor_gui.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_settings.h"
#include "us_solution.h"
#include "us_solution_gui.h"
#include "us_table.h"

#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#define setSymbol(a) setSymbol(*a)
#endif

US_EditSpectrum::US_EditSpectrum(QString tmp_type, QString tmp_exists,
                                 const QString& tmp_text,
                                 const QString& tmp_text_e280,
                                 US_Buffer* tmp_buffer)
    : US_Widgets() {
  buffer = tmp_buffer;
  text = tmp_text;
  type = tmp_type;
  text_e280 = tmp_text_e280;
  exists = tmp_exists;

  setPalette(US_GuiSettings::frameColor());
  QGridLayout* main = new QGridLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  this->setMinimumSize(480, 60);

  setWindowTitle(tr("Edit Spectrum"));

  QLabel* bn_info_new = us_banner(tr(" %1 has NO spectrum data: Choose among "
                                     "the options to enter new spectrum")
                                      .arg(type));
  bn_info_new->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QLabel* bn_info_exists = us_banner(
      tr(" %1 has spectrum data: Choose among the options to edit the spectrum")
          .arg(type));
  bn_info_exists->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  pb_view = us_pushbutton(tr("View Spectrum"));
  pb_delete = us_pushbutton(tr("Delete Spectrum"));
  pb_cancel = us_pushbutton(tr("Cancel"));

  QString enter_edit = "";
  if (exists == "EXISTS") enter_edit = "Edit";
  if (exists == "NEW") enter_edit = "Enter";
  pb_manual = us_pushbutton(tr("%1 Spectrum Manually").arg(enter_edit));

  pb_uploadDisk = us_pushbutton(tr("Upload New Spectrum from Disk"));
  pb_uploadFit = us_pushbutton(tr("Upload and Fit New Spectrum"));

  if (exists == "NEW") {
    pb_view->hide();
    pb_delete->hide();
    bn_info_exists->hide();
  }
  if (exists == "EXISTS") {
    bn_info_new->hide();
  }

  int row = 0;
  main->addWidget(bn_info_new, row++, 0, 1, 12);
  main->addWidget(bn_info_exists, row++, 0, 1, 12);
  main->addWidget(pb_view, row, 0, 1, 6);
  main->addWidget(pb_delete, row++, 6, 1, 6);

  main->addWidget(pb_uploadDisk, row, 0, 1, 6);
  main->addWidget(pb_uploadFit, row++, 6, 1, 6);

  main->addWidget(pb_manual, row, 0, 1, 6);
  main->addWidget(pb_cancel, row, 6, 1, 6);

  connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
  connect(pb_manual, SIGNAL(clicked()), this, SLOT(editmanually()));
  connect(pb_uploadDisk, SIGNAL(clicked()), this, SLOT(uploadDisk()));
  connect(pb_uploadFit, SIGNAL(clicked()), this, SLOT(uploadFit()));
  connect(pb_delete, SIGNAL(clicked()), this, SLOT(delete_spectrum()));
  connect(pb_view, SIGNAL(clicked()), this, SLOT(view_spectrum()));
}

US_EditSpectrum::US_EditSpectrum(QString tmp_type, QString tmp_exists,
                                 const QString& tmp_text,
                                 const QString& tmp_text_e280,
                                 US_Analyte* tmp_analyte)
    : US_Widgets() {
  analyte = tmp_analyte;
  text = tmp_text;
  type = tmp_type;
  text_e280 = tmp_text_e280;
  exists = tmp_exists;

  setPalette(US_GuiSettings::frameColor());
  QGridLayout* main = new QGridLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  this->setMinimumSize(480, 60);

  setWindowTitle(tr("Edit Spectrum"));

  QLabel* bn_info_new = us_banner(tr(" %1 has NO spectrum data: Choose among "
                                     "the options to enter new spectrum")
                                      .arg(type));
  bn_info_new->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QLabel* bn_info_exists = us_banner(
      tr(" %1 has spectrum data: Choose among the options to edit the spectrum")
          .arg(type));
  bn_info_exists->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  pb_view = us_pushbutton(tr("View Spectrum"));
  pb_delete = us_pushbutton(tr("Delete Spectrum"));
  pb_cancel = us_pushbutton(tr("Cancel"));

  QString enter_edit = "";
  if (exists == "EXISTS") enter_edit = "Edit";
  if (exists == "NEW") enter_edit = "Enter";
  pb_manual = us_pushbutton(tr("%1 Spectrum Manually").arg(enter_edit));

  pb_uploadDisk = us_pushbutton(tr("Upload New Spectrum from Disk"));
  pb_uploadFit = us_pushbutton(tr("Upload and Fit New Spectrum"));

  if (exists == "NEW") {
    pb_view->hide();
    pb_delete->hide();
    bn_info_exists->hide();
  }
  if (exists == "EXISTS") {
    bn_info_new->hide();
  }

  int row = 0;
  main->addWidget(bn_info_new, row++, 0, 1, 12);
  main->addWidget(bn_info_exists, row++, 0, 1, 12);
  main->addWidget(pb_view, row, 0, 1, 6);
  main->addWidget(pb_delete, row++, 6, 1, 6);

  main->addWidget(pb_uploadDisk, row, 0, 1, 6);
  main->addWidget(pb_uploadFit, row++, 6, 1, 6);

  main->addWidget(pb_manual, row, 0, 1, 6);
  main->addWidget(pb_cancel, row, 6, 1, 6);

  connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
  connect(pb_manual, SIGNAL(clicked()), this, SLOT(editmanually()));
  connect(pb_uploadDisk, SIGNAL(clicked()), this, SLOT(uploadDisk()));
  connect(pb_uploadFit, SIGNAL(clicked()), this, SLOT(uploadFit()));
  connect(pb_delete, SIGNAL(clicked()), this, SLOT(delete_spectrum()));
  connect(pb_view, SIGNAL(clicked()), this, SLOT(view_spectrum()));
}

US_EditSpectrum::US_EditSpectrum(QString tmp_type, QString tmp_exists,
                                 const QString& tmp_text,
                                 const QString& tmp_text_e280,
                                 US_Solution* tmp_solution)
    : US_Widgets() {
  solution = tmp_solution;
  text = tmp_text;
  type = tmp_type;
  text_e280 = tmp_text_e280;
  exists = tmp_exists;

  setPalette(US_GuiSettings::frameColor());
  QGridLayout* main = new QGridLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);

  this->setMinimumSize(480, 60);

  setWindowTitle(tr("Edit Spectrum"));

  QLabel* bn_info_new = us_banner(tr(" %1 has NO spectrum data: Choose among "
                                     "the options to enter new spectrum")
                                      .arg(type));
  bn_info_new->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  QLabel* bn_info_exists = us_banner(
      tr(" %1 has spectrum data: Choose among the options to edit the spectrum")
          .arg(type));
  bn_info_exists->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  pb_view = us_pushbutton(tr("View Spectrum"));
  pb_delete = us_pushbutton(tr("Delete Spectrum"));
  pb_cancel = us_pushbutton(tr("Cancel"));

  QString enter_edit = "";
  if (exists == "EXISTS") enter_edit = "Edit";
  if (exists == "NEW") enter_edit = "Enter";
  pb_manual = us_pushbutton(tr("%1 Spectrum Manually").arg(enter_edit));

  pb_uploadDisk = us_pushbutton(tr("Upload New Spectrum from Disk"));
  pb_uploadFit = us_pushbutton(tr("Upload and Fit New Spectrum"));

  if (exists == "NEW") {
    pb_view->hide();
    pb_delete->hide();
    bn_info_exists->hide();
  }
  if (exists == "EXISTS") {
    bn_info_new->hide();
  }

  int row = 0;
  main->addWidget(bn_info_new, row++, 0, 1, 12);
  main->addWidget(bn_info_exists, row++, 0, 1, 12);
  main->addWidget(pb_view, row, 0, 1, 6);
  main->addWidget(pb_delete, row++, 6, 1, 6);

  main->addWidget(pb_uploadDisk, row, 0, 1, 6);
  main->addWidget(pb_uploadFit, row++, 6, 1, 6);

  main->addWidget(pb_manual, row, 0, 1, 6);
  main->addWidget(pb_cancel, row, 6, 1, 6);

  connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancel()));
  connect(pb_manual, SIGNAL(clicked()), this, SLOT(editmanually()));
  connect(pb_uploadDisk, SIGNAL(clicked()), this, SLOT(uploadDisk()));
  connect(pb_uploadFit, SIGNAL(clicked()), this, SLOT(uploadFit()));
  connect(pb_delete, SIGNAL(clicked()), this, SLOT(delete_spectrum()));
  connect(pb_view, SIGNAL(clicked()), this, SLOT(view_spectrum()));
}

void US_EditSpectrum::view_spectrum(void) {
  QMap<double, double> ext;
  if (type == "BUFFER") {
    ext = buffer->extinction;
  }
  if (type == "ANALYTE") {
    ext = analyte->extinction;
  }
  if (type == "SOLUTION") {
    ext = solution->extinction;
  }

  US_ViewSpectrum* s = new US_ViewSpectrum(ext);
  s->setParent(this, Qt::Window);
  s->show();
}

void US_EditSpectrum::delete_spectrum(void) {
  US_Passwd pw;
  US_DB2 db(pw.getPasswd());

  if (db.lastErrno() != US_DB2::OK) {
    QMessageBox::warning(
        this, tr("Connection Problem"),
        tr("Could not connect to database \n") + db.lastError());
    return;
  }

  QStringList q;

  if (type == "BUFFER") q << "get_bufferID" << buffer->GUID;
  if (type == "ANALYTE") q << "get_analyteID" << analyte->analyteGUID;
  if (type == "SOLUTION")
    q << "get_solutionID_from_GUID" << solution->solutionGUID;

  db.query(q);

  int status = db.lastErrno();

  if (status == US_DB2::OK) {
    db.next();

    if (type == "BUFFER") {
      QString bufferID = db.value(0).toString();
      QString compType("Buffer");
      US_ExtProfile::delete_eprofile(&db, bufferID.toInt(), compType);
    }
    if (type == "ANALYTE") {
      QString analyteID = db.value(0).toString();
      QString compType("Analyte");
      US_ExtProfile::delete_eprofile(&db, analyteID.toInt(), compType);
    }
    if (type == "SOLUTION") {
      QString solutionID = db.value(0).toString();
      QString compType("Sample");
      US_ExtProfile::delete_eprofile(&db, solutionID.toInt(), compType);
    }

    QMessageBox::information(this, tr("Deletion: Success"),
                             tr("Spectrum was successfully deleted"));

    emit change_spectrum();
  }

  if (status == US_DB2::BUFFR_IN_USE) {
    QMessageBox::warning(
        this, tr("Spectrum Not Deleted"),
        tr("Spectrum for this BUFFER could not be deleted since\n"
           "it is in use in one or more solutions."));
    return;
  }

  if (status == US_DB2::ANALY_IN_USE) {
    QMessageBox::warning(
        this, tr("Spectrum Not Deleted"),
        tr("Spectrum for this ANALYTE could not be deleted since\n"
           "it is in use in one or more solutions."));
    return;
  }

  if (status == US_DB2::SOLUT_IN_USE) {
    QMessageBox::warning(
        this, tr("Spectrum Not Deleted"),
        tr("Spectrum for this SOLUTION could not be deleted since\n"
           "it is in use in one or more solutions."));
    return;
  }

  if (status != US_DB2::OK) {
    QMessageBox::warning(this, tr("Attention"),
                         tr("Delete failed.\n\n") + db.lastError());
  }
  this->close();
}

void US_EditSpectrum::uploadDisk(void) { add_spectrumDisk(); }

void US_EditSpectrum::uploadFit(void) {
  w_spec = new US_Extinction(type, text, "1.000", (QWidget*)this);

  connect(w_spec, SIGNAL(get_results(QMap<double, double>&)), this,
          SLOT(process_results(QMap<double, double>&)));

  w_spec->setParent(this, Qt::Window);
  w_spec->setAttribute(Qt::WA_DeleteOnClose);
  w_spec->show();
}

void US_EditSpectrum::process_results(QMap<double, double>& xyz) {
  if (type == "BUFFER") buffer->extinction = xyz;
  if (type == "ANALYTE") analyte->extinction = xyz;
  if (type == "SOLUTION") solution->extinction = xyz;

  QMap<double, double>::iterator it;
  QString output;

  for (it = xyz.begin(); it != xyz.end(); ++it) {
    // Format output here.
    output += QString(" %1 : %2 /n").arg(it.key()).arg(it.value());
  }

  QMessageBox::information(
      this, tr("Test: Data transmitted"),
      tr("Number of keys in extinction QMAP: %1 . You may click 'Accept'"
         " from the main window to write new %2 into DB")
          .arg(xyz.keys().count())
          .arg(type));

  // Update via STORED Procedures ....
  if (exists == "EXISTS") {
    if (type == "BUFFER") {
      buffer->replace_spectrum = true;
      buffer->new_or_changed_spectrum = true;
    }
    if (type == "ANALYTE") {
      analyte->replace_spectrum = true;
      analyte->new_or_changed_spectrum = true;
    }
    if (type == "SOLUTION") {
      solution->replace_spectrum = true;
      solution->new_or_changed_spectrum = true;
    }
  }
  if (exists == "NEW") {
    if (type == "BUFFER") buffer->new_or_changed_spectrum = true;
    if (type == "ANALYTE") analyte->new_or_changed_spectrum = true;
    if (type == "SOLUTION") solution->new_or_changed_spectrum = true;
  }

  w_spec->close();
  this->close();
  emit accept_enable();
}

void US_EditSpectrum::add_spectrumDisk(void) {
  QStringList files;
  QFile f;

  QFileDialog dialog(this);
  // dialog.setNameFilter(tr("Text (*.txt *.csv *.dat *.wa *.dsp)"));

  dialog.setNameFilter(
      tr("Text files (*.[Tt][Xx][Tt] *.[Cc][Ss][Vv] *.[Dd][Aa][Tt] *.[Ww][Aa]* "
         "*.[Dd][Ss][Pp]);;All files (*)"));

  dialog.setFileMode(QFileDialog::ExistingFile);
  dialog.setViewMode(QFileDialog::Detail);
  // dialog.setDirectory("/home/alexsav/ultrascan/data/spectra");

  QString work_dir_data = US_Settings::dataDir();
  qDebug() << work_dir_data;
  dialog.setDirectory(work_dir_data);

  if (dialog.exec()) {
    files = dialog.selectedFiles();
    readingspectra(files[0]);
  }
  // qDebug() << "Files: " << files[0];
}

void US_EditSpectrum::readingspectra(const QString& fileName) {
  QString str1;
  QStringList strl;
  float temp_x, temp_y;
  QMap<double, double> temp_extinct;

  if (!fileName.isEmpty()) {
    QFile f(fileName);

    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
      QTextStream ts(&f);
      while (!ts.atEnd()) {
        if (!ts.atEnd()) {
          str1 = ts.readLine();
        }
        str1 = str1.simplified();
        str1 = str1.replace("\"", " ");
        str1 = str1.replace(",", " ");
        strl = str1.split(" ");
        temp_x = strl.at(0).toFloat();
        temp_y = strl.at(1).toFloat();

        // qDebug() << temp_x << ", " << temp_y;

        if (temp_x != 0) {
          temp_extinct[double(temp_x)] = double(temp_y);
        }
      }
    }
    if (type == "BUFFER") buffer->extinction = temp_extinct;
    if (type == "ANALYTE") analyte->extinction = temp_extinct;
    if (type == "SOLUTION") solution->extinction = temp_extinct;

    // Update via STORED Procedures ....
    if (exists == "EXISTS") {
      if (type == "BUFFER") {
        buffer->replace_spectrum = true;
        buffer->new_or_changed_spectrum = true;
      }
      if (type == "ANALYTE") {
        analyte->replace_spectrum = true;
        analyte->new_or_changed_spectrum = true;
      }
      if (type == "SOLUTION") {
        solution->replace_spectrum = true;
        solution->new_or_changed_spectrum = true;
      }
    }
    if (exists == "NEW") {
      if (type == "BUFFER") buffer->new_or_changed_spectrum = true;
      if (type == "ANALYTE") analyte->new_or_changed_spectrum = true;
      if (type == "SOLUTION") solution->new_or_changed_spectrum = true;
    }
  }

  emit accept_enable();
  this->close();
}

void US_EditSpectrum::editmanually(void) {
  US_Table* sdiag;

  loc_extinct.clear();

  if (type == "BUFFER") loc_extinct = buffer->extinction;
  if (type == "ANALYTE") loc_extinct = analyte->extinction;
  if (type == "SOLUTION") loc_extinct = solution->extinction;

  QString stype("Extinction");
  bool changed = false;
  sdiag = new US_Table(loc_extinct, stype, changed, this);
  sdiag->setWindowTitle("Manage Extinction Spectrum");
  sdiag->exec();

  // qDebug() << "Before edit maually, e280: " << analyte->extinction[280.0];

  if (changed) {
    qDebug() << "Existing: Inside Changed: ";

    if (type == "BUFFER") {
      buffer->extinction.clear();
      buffer->extinction = loc_extinct;
    }
    if (type == "ANALYTE") {
      analyte->extinction.clear();
      analyte->extinction = loc_extinct;
      // qDebug() << "Inside edit manually, e280: " <<
      // analyte->extinction[280.0];
    }
    if (type == "SOLUTION") {
      solution->extinction.clear();
      solution->extinction = loc_extinct;
    }

    // Update via STORED Procedures ....
    if (exists == "EXISTS") {
      if (type == "BUFFER") {
        buffer->replace_spectrum = true;
        buffer->new_or_changed_spectrum = true;
      }
      if (type == "ANALYTE") {
        analyte->replace_spectrum = true;
        analyte->new_or_changed_spectrum = true;
      }
      if (type == "SOLUTION") {
        solution->replace_spectrum = true;
        solution->new_or_changed_spectrum = true;
      }
    }
    if (exists == "NEW") {
      if (type == "BUFFER") buffer->new_or_changed_spectrum = true;
      if (type == "ANALYTE") analyte->new_or_changed_spectrum = true;
      if (type == "SOLUTION") solution->new_or_changed_spectrum = true;
    }
  }

  emit accept_enable();
  this->close();
}

void US_EditSpectrum::cancel(void) { this->close(); }
