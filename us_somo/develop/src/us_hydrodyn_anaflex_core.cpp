// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian
// dynamic browflex computations (this) us_hydrodyn_anaflex_core.cpp contains
// the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for
// molecular dynamic (dmd) computations us_hydrodyn_other.cpp contains other
// routines such as file i/o

// includes and defines need cleanup

#include <qregexp.h>

#include "../include/us_hydrodyn.h"
// Added by qt3to4:
#include <QTextStream>

#ifndef WIN32
#include <unistd.h>
#define SLASH "/"
#else
#include <direct.h>
#include <dos.h>
#include <io.h>
#include <stdlib.h>
#define SLASH "\\"
#endif

// ---------- create anaflex files
// ---------- create one for each mode that is selected
int US_Hydrodyn::create_anaflex_files() {
  // right now we are just going to support one-mode per run
  // in future we will support multiple, which requires
  // the extra structures
  if (anaflex_options.run_mode_1) {
    return create_anaflex_files(1);
  }
  if (anaflex_options.run_mode_2) {
    return create_anaflex_files(2);
  }
  if (anaflex_options.run_mode_3) {
    if (anaflex_options.run_mode_3_1) {
      return create_anaflex_files(3, 1);
    }
    if (anaflex_options.run_mode_3_5) {
      return create_anaflex_files(3, 5);
    }
    if (anaflex_options.run_mode_3_9) {
      return create_anaflex_files(3, 9);
    }
    if (anaflex_options.run_mode_3_10) {
      return create_anaflex_files(3, 10);
    }
    if (anaflex_options.run_mode_3_14) {
      return create_anaflex_files(3, 14);
    }
    if (anaflex_options.run_mode_3_15) {
      return create_anaflex_files(3, 15);
    }
    if (anaflex_options.run_mode_3_16) {
      return create_anaflex_files(3, 16);
    }
  }
  if (anaflex_options.run_mode_4) {
    if (anaflex_options.run_mode_4_1) {
      return create_anaflex_files(4, 1);
    }
    if (anaflex_options.run_mode_4_6) {
      return create_anaflex_files(4, 6);
    }
    if (anaflex_options.run_mode_4_7) {
      return create_anaflex_files(4, 7);
    }
    if (anaflex_options.run_mode_4_8) {
      return create_anaflex_files(4, 8);
    }
  }
  if (anaflex_options.run_mode_9) {
    return create_anaflex_files(9);
  }
  return -1;
}

int US_Hydrodyn::create_anaflex_files(int use_mode, int sub_mode) {
  editor->append(
      us_tr(QString("Creating anaflex files (mode %1)\n").arg(use_mode)));

  project = bd_project;
  current_model = bd_current_model;

#if defined(DEBUG_BD)
  cout << "project " << project << endl;
  cout << "ca bd last file " << bd_last_file << endl;
  cout << "ca bd last tra file " << bd_last_traj_file << endl;
  cout << "ca bd last molec file " << bd_last_molec_file << endl;
  fflush(stdout);
#endif

  int no_of_beads = browflex_get_no_of_beads(bd_last_file);

  QFileInfo fi(bd_last_traj_file);
  QString dir = fi.path();
  QString trajfile = fi.fileName();

  QString filename = project + QString("%1").arg(current_model + 1);
  // QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
  filename += QString("af%1%2").arg(use_mode).arg(
      sub_mode ? QString("-%1").arg(sub_mode) : "");
  QString bffilename =
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
      "-bf";

  QString basename = dir + SLASH + filename;

  // anaflex-main.txt
  QFile f;
  // main file
  {
    f.setFileName(basename + "m.txt");
    if (!f.open(QIODevice::WriteOnly)) {
      editor->append(
          QString("File write error: can't create %1\n").arg(f.fileName()));
      return -1;
    }
    anaflex_last_file = f.fileName();
    QTextStream ts(&f);
    ts << QString(
              "%1l.txt            !outputfile 1\n"
              "%2r.txt            !outputfile 2\n"
              "%3s.txt            !outputfile 3\n"
              "%4                    !sampling frequency\n"
              "%5                    !instprofiles\n"
              "%6                    !mode\n")
              .arg(filename)
              .arg(filename)
              .arg(filename)
              .arg(anaflex_options.nfrec)
              .arg(anaflex_options.instprofiles)
              .arg(use_mode);

    anaflex_last_out1_file = dir + SLASH + filename + "l.txt";
    switch (use_mode) {
      case 1:  // steady state
      {
        // count nprops
        int ncodes =
            anaflex_options.run_mode_1_1 + anaflex_options.run_mode_1_2 +
            anaflex_options.run_mode_1_3 + anaflex_options.run_mode_1_4 +
            anaflex_options.run_mode_1_5 + anaflex_options.run_mode_1_7 +
            anaflex_options.run_mode_1_8 + anaflex_options.run_mode_1_12 +
            anaflex_options.run_mode_1_13 + anaflex_options.run_mode_1_14 +
            anaflex_options.run_mode_1_18;

        int nprops =
            anaflex_options.run_mode_1_1 + anaflex_options.run_mode_1_2 +
            anaflex_options.run_mode_1_3 + anaflex_options.run_mode_1_4 * 2 +
            anaflex_options.run_mode_1_5 * 3 +
            anaflex_options.run_mode_1_7 * 2 +
            anaflex_options.run_mode_1_8 * 4 + anaflex_options.run_mode_1_12 +
            anaflex_options.run_mode_1_13 + anaflex_options.run_mode_1_14 * 6 +
            anaflex_options.run_mode_1_18 * 6;
        // editor->append(QString("mode 1 ncodes %1 nprops
        // %2\n").arg(ncodes).arg(nprops));
        ts << QString("%1,%2                 !number of codes & properties\n")
                  .arg(ncodes)
                  .arg(nprops);
        if (anaflex_options.run_mode_1_1) {
          ts << "1,0,0,0,0                      !linear end-to-end square "
                "distance\n";
        }
        if (anaflex_options.run_mode_1_2) {
          ts << "2,0,0,0,0                      !square radius of gyration\n";
        }
        if (anaflex_options.run_mode_1_3) {
          ts << "3,0,0,0,0                      !gzz components of gyration "
                "tensor\n";
        }
        if (anaflex_options.run_mode_1_4) {
          ts << "4,0,0,0,0                      !gxx,gyy components of "
                "gyration tensor\n";
        }
        if (anaflex_options.run_mode_1_5) {
          ts << "5,0,0,0,0                      !gxy,gxz,gyz components of "
                "gyration tensor\n";
        }
        if (anaflex_options.run_mode_1_7) {
          ts << "7,0,0,0,0                      !extension along z and it's "
                "square\n";
        }
        if (anaflex_options.run_mode_1_8) {
          ts << "8,0,0,0,0                      !extension along x and y and "
                "their squares\n";
        }
        if (anaflex_options.run_mode_1_12) {
          ts << "12,0,0,0,0                     !birefringence in FENE "
                "chains\n";
        }
        if (anaflex_options.run_mode_1_13) {
          ts << "13,0,0,0,0                     !p2 average over connectors \n";
        }
        if (anaflex_options.run_mode_1_14) {
          ts << "14,0,0,0,0                     !components of stress tensor\n";
        }
        if (anaflex_options.run_mode_1_18) {
          ts << "18,12,0,0,0                    !rigid body hydrodynamic "
                "props\n";
        }
      } break;

      case 2:  // time dependent
      {
        // count nprops
        int ncodes =
            anaflex_options.run_mode_2_1 + anaflex_options.run_mode_2_2 +
            anaflex_options.run_mode_2_3 + anaflex_options.run_mode_2_4 +
            anaflex_options.run_mode_2_5 + anaflex_options.run_mode_2_7 +
            anaflex_options.run_mode_2_8 + anaflex_options.run_mode_2_12 +
            anaflex_options.run_mode_2_13 + anaflex_options.run_mode_2_14 +
            anaflex_options.run_mode_2_18;

        int nprops =
            anaflex_options.run_mode_2_1 + anaflex_options.run_mode_2_2 +
            anaflex_options.run_mode_2_3 + anaflex_options.run_mode_2_4 * 2 +
            anaflex_options.run_mode_2_5 * 3 +
            anaflex_options.run_mode_2_7 * 2 +
            anaflex_options.run_mode_2_8 * 4 + anaflex_options.run_mode_2_12 +
            anaflex_options.run_mode_2_13 + anaflex_options.run_mode_2_14 * 6 +
            anaflex_options.run_mode_2_18 * 6;

        ts << QString("%1,%2                 !number of codes & properties\n")
                  .arg(ncodes)
                  .arg(nprops);
        if (anaflex_options.run_mode_2_1) {
          ts << "1,0,0,0,0                      !linear end-to-end square "
                "distance\n";
        }
        if (anaflex_options.run_mode_2_2) {
          ts << "2,0,0,0,0                      !square radius of gyration\n";
        }
        if (anaflex_options.run_mode_2_3) {
          ts << "3,0,0,0,0                      !gzz components of gyration "
                "tensor\n";
        }
        if (anaflex_options.run_mode_2_4) {
          ts << "4,0,0,0,0                      !gxx,gyy components of "
                "gyration tensor\n";
        }
        if (anaflex_options.run_mode_2_5) {
          ts << "5,0,0,0,0                      !gxy,gxz,gyz components of "
                "gyration tensor\n";
        }
        if (anaflex_options.run_mode_2_7) {
          ts << "7,0,0,0,0                      !extension along z and it's "
                "square\n";
        }
        if (anaflex_options.run_mode_2_8) {
          ts << "8,0,0,0,0                      !extension along x and y and "
                "their squares\n";
        }
        if (anaflex_options.run_mode_2_12) {
          ts << "12,0,0,0,0                     !birefringence in FENE "
                "chains\n";
        }
        if (anaflex_options.run_mode_2_13) {
          ts << "13,0,0,0,0                     !p2 average over connectors \n";
        }
        if (anaflex_options.run_mode_2_14) {
          ts << "14,0,0,0,0                     !components of stress tensor\n";
        }
        if (anaflex_options.run_mode_2_18) {
          ts << "18,12,0,0,0                    !rigid body hydrodynamic "
                "props\n";
        }
      } break;

      case 3:  // correlation functions
      {
        ts << sub_mode << ",";  // indcorr indicated type of function
        ts << (sub_mode == 1 ? 1 : 3)
           << ",";   // ian is type of fitting of correlation
                     // usually 3 except 1 for translation
        ts << "0,";  // inbase is the indicator of base line in some types of
                     // fitting
        ts << anaflex_options.ntimc << ",";
        ts << anaflex_options.tmax << ",";

        // iii,jjj,nvx, this will change if we support sub modes 5, 10 and 14
        switch (sub_mode) {
          case 5:
            ts << anaflex_options.run_mode_3_5_iii << ",";
            {
              int use_last_bead = anaflex_options.run_mode_3_5_jjj;
              if (use_last_bead > no_of_beads) {
                if (!QMessageBox::question(
                        this, QString("Number of beads"),
                        QString(
                            us_tr("You have selected an Anaflex"
                                  " option which requires a last bead number.\n"
                                  "There appear to be %1 beads in your "
                                  "Browflex file\n"
                                  "And your last bead number is set to %2.\n"))
                            .arg(no_of_beads)
                            .arg(use_last_bead),
                        us_tr(QString("&Use %1 as last bead").arg(no_of_beads)),
                        us_tr(QString("&Accept %1 as last bead")
                                  .arg(use_last_bead)),
                        QString(), 0, 1)) {
                  use_last_bead = no_of_beads;
                }
              }
              ts << use_last_bead << ",";
              ts << "0                           !Hydrodynamic properties\n";
            }
            break;
          case 10:
            ts << "0,0,1,"
               << (4.0 * M_PI * anaflex_options.run_mode_3_10_refractive_index /
                   (anaflex_options.run_mode_3_10_lambda * 1e-8)) *
                      sin(anaflex_options.run_mode_3_10_theta * (M_PI / 180.0) /
                          2.0)
               << "                            !Hydrodynamic properties\n";
            break;
          case 14:
            ts << anaflex_options.run_mode_3_14_iii << ",";
            {
              int use_last_bead = anaflex_options.run_mode_3_5_jjj;
              if (use_last_bead > no_of_beads) {
                if (!QMessageBox::question(
                        this, QString("Number of beads"),
                        QString(
                            us_tr("You have selected an Anaflex"
                                  " option which requires a last bead number.\n"
                                  "There appear to be %1 beads in your "
                                  "Browflex file\n"
                                  "And your last bead number is set to %2.\n"))
                            .arg(no_of_beads)
                            .arg(use_last_bead),
                        us_tr(QString("&Use %1 as last bead").arg(no_of_beads)),
                        us_tr(QString("&Accept %1 as last bead")
                                  .arg(use_last_bead)),
                        QString(), 0, 1)) {
                  use_last_bead = no_of_beads;
                }
              }
              ts << use_last_bead << ",";
              ts << "0                           !Hydrodynamic properties\n";
            }
            break;
          default:
            ts << "0,0,0                           !Hydrodynamic properties\n";
            break;
        }
      } break;

      case 9:  // textfile
        anaflex_last_log_file = dir + SLASH + filename + "-to.txt";
        ts << QString("%1-to.txt                !output textfile\n")
                  .arg(filename);
        break;

      default:
        editor->append(
            QString("anaflex mode %1 not yet supported\n").arg(use_mode));
        break;
    }

    ts << QString(
              "%1                         !trajectory file\n"
              "*\n\n\n")
              .arg(trajfile);

    f.close();
    anaflex_ready_to_run = true;
  }
  bd_anaflex_enables(true);
  return 0;
}

// ------------- run anaflex  --------------

int US_Hydrodyn::run_anaflex() {
  int use_mode = 0;
  int sub_mode = 0;
  anaflex_return_to_bd_load_results = false;
  // possible setup a new text window for the anaflex runs?
  // later loop through for multiple runs?
  if (anaflex_options.run_mode_1) {
    use_mode = 1;
  }
  if (anaflex_options.run_mode_2) {
    use_mode = 2;
  }
  if (anaflex_options.run_mode_3) {
    use_mode = 3;
    if (anaflex_options.run_mode_3_1) {
      sub_mode = 1;
    }
    if (anaflex_options.run_mode_3_5) {
      sub_mode = 5;
    }
    if (anaflex_options.run_mode_3_9) {
      sub_mode = 9;
    }
    if (anaflex_options.run_mode_3_10) {
      sub_mode = 10;
    }
    if (anaflex_options.run_mode_3_14) {
      sub_mode = 14;
    }
    if (anaflex_options.run_mode_3_15) {
      sub_mode = 15;
    }
    if (anaflex_options.run_mode_3_16) {
      sub_mode = 16;
    }
  }
  if (anaflex_options.run_mode_4) {
    use_mode = 4;
    if (anaflex_options.run_mode_4_1) {
      sub_mode = 1;
    }
    if (anaflex_options.run_mode_4_6) {
      sub_mode = 6;
    }
    if (anaflex_options.run_mode_4_7) {
      sub_mode = 7;
    }
    if (anaflex_options.run_mode_4_8) {
      sub_mode = 8;
    }
  }
  if (anaflex_options.run_mode_9) {
    use_mode = 9;
  }
  return run_anaflex(use_mode, sub_mode);
}

int US_Hydrodyn::run_anaflex(int /* use_mode */, int /* sub_mode */) {
  //   QString dir = somo_dir + SLASH + "bd";
  QString prog = USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
                 "bin64"
#else
                 "/bin/"
#endif
                 + SLASH + "anaflex2a-"
#if defined(WIN32)
                 + "msd"
#else
                 + "lnx"
#endif
                 + ".exe";

  {
    QFileInfo qfi(prog);
    if (!qfi.exists()) {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(
          QString("Anaflex program '%1' does not exist\n").arg(prog));
      editor->setTextColor(save_color);
      return -1;
    }
    if (!qfi.isExecutable()) {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(
          QString("Anaflex program '%1' is not executable\n").arg(prog));
      editor->setTextColor(save_color);
      return -1;
    }
  }

  QFileInfo fi(anaflex_last_file);
  QString anafile = fi.fileName();
  //      project + QString("_%1").arg(current_model + 1) +
  //      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
  //      + "-bf-main.txt\n" ;
  QString dir = fi.path();

  //   QString anafile =
  //      project + QString("%1").arg(current_model + 1) +
  //      //      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix)
  //      : "")
  //      + QString("af%1%2m.txt\n").arg(use_mode).arg(sub_mode ?
  //      QString("-%1").arg(sub_mode) : "");

#if defined(DEBUG_BD)
  cout << QString("run anaflex dir <%1> prog <%2> stdin <%3>\n")
              .arg(dir)
              .arg(prog)
              .arg(anafile);
#endif
  anaflex = new QProcess(this);
  anaflex->setWorkingDirectory(dir);

#if QT_VERSION < 0x040000
  anaflex->addArgument(prog);
#else
  QStringList args;
  args << "<" << anafile;
#endif

  connect(anaflex, SIGNAL(readyReadStandardOutput()), this,
          SLOT(anaflex_readFromStdout()));
  connect(anaflex, SIGNAL(readyReadStandardError()), this,
          SLOT(anaflex_readFromStderr()));
  connect(anaflex, SIGNAL(finished(int, QProcess::ExitStatus)), this,
          SLOT(anaflex_finished(int, QProcess::ExitStatus)));
  connect(anaflex, SIGNAL(started()), this, SLOT(anaflex_started()));

#if QT_VERSION < 0x040000
  anaflex->launch(anafile);
#else
  anaflex->start(prog, args, QIODevice::ReadOnly);
#endif

  return 0;
}

void US_Hydrodyn::anaflex_readFromStdout() {
#if QT_VERSION < 0x040000
  while (anaflex->canReadLineStdout()) {
    editor_msg("brown", anaflex->readLineStdout() + "\n");
  }
#else
  editor_msg("brown", QString(anaflex->readAllStandardOutput()));
#endif
}

void US_Hydrodyn::anaflex_readFromStderr() {
#if QT_VERSION < 0x040000
  while (anaflex->canReadLineStderr()) {
    editor_msg("red", anaflex->readLineStderr() + "\n");
  }
#else
  editor_msg("red", QString(anaflex->readAllStandardError()));
#endif
}

void US_Hydrodyn::anaflex_finished(int, QProcess::ExitStatus) {
  anaflex_readFromStderr();
  anaflex_readFromStdout();
  disconnect(anaflex, SIGNAL(readyReadStandardOutput()), 0, 0);
  disconnect(anaflex, SIGNAL(readyReadStandardError()), 0, 0);
  disconnect(anaflex, SIGNAL(finished(int, QProcess::ExitStatus)), 0, 0);
  editor_msg("brown", "Anaflex process exited\n");
  if (!anaflex_return_to_bd_load_results) {
    for (current_model = 0; current_model < (unsigned int)lb_model->count();
         current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
        bd_anaflex_enables(true);
        break;
      }
    }
  }
  if (anaflex_return_to_bd_load_results) {
    anaflex_return_to_bd_load_results = false;
    bd_load_results_after_anaflex();
  }
}

void US_Hydrodyn::anaflex_started() {
  editor_msg("brown", "Anaflex launch exited\n");
  disconnect(anaflex, SIGNAL(started()), 0, 0);
}

void US_Hydrodyn::anaflex_prepare() { create_anaflex_files(); }

void US_Hydrodyn::anaflex_load_error(QString filename) {
  editor_msg("red",
             QString(us_tr("\nFile %1 does not look like an Anaflex file.\n"))
                 .arg(filename));
}

void US_Hydrodyn::anaflex_load() {
  QString filename;
  if (anaflex_last_file != "") {
    // ask if use existing or new
    QFileInfo fi(anaflex_last_file);
    switch (QMessageBox::question(
        this, us_tr("Load Anaflex Files"),
        QString(us_tr("Replace current Anaflex file ") + fi.fileName() + "?"),
        QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton)) {
      case QMessageBox::Yes:
        filename = QFileDialog::getOpenFileName(
            this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
        break;
      case QMessageBox::No:
        return;
        break;
      case QMessageBox::Cancel:
      default:
        return;
        break;
    }
  } else {
    filename = QFileDialog::getOpenFileName(
        this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
  }
  // check to make sure it is a good anaflex file
  if (!filename.isEmpty()) {
    QFileInfo fi(filename);
    QString dir = fi.path();
    QString name = fi.fileName();
    // check for file format
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
      editor_msg(
          "red",
          QString(
              us_tr(
                  "\nCould not open file %1 for reading. Check permissions.\n"))
              .arg(filename));
      return;
    }
    if (!anaflex_valid_anaflex_main(filename)) {
      bd_load_error(filename);
      f.close();
      return;
    }
    f.close();
    editor->append(QString("Anaflex file %1 loaded\n").arg(filename));

    anaflex_last_file = filename;
    anaflex_ready_to_run = true;
    bd_anaflex_enables(true);
  }
}

bool US_Hydrodyn::anaflex_valid_anaflex_main(QString filename) {
  // make sure this is a valid anaflex main file
  /*
--- example anaflex file ----:
anaDNAdoubhel20bp-log1.txt                 !outputfile 1
anaDNAdoubhel20bp-res1.txt                 !outputfile 2
anaDNAdoubhel20bp-sum1.txt                 !outputfile 3
1                                         !sampling frequency in traj.
0                                         !instprofiles 1-yes,write
3                                         !mode
1,1,0,11,1.6e-8,0,0,0                     !Hydrodynamic properties
DNAdoubhel20bp-tra.txt                    !trajectory file
*
--- end example --- */
  if (!filename.isEmpty()) {
    QFileInfo fi(filename);
    QString dir = fi.path();
    // check for file format
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
      return false;
    }
    QTextStream ts(&f);
    QString tmp_filename;
    // outputfile 1
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return false;
      }
    } else {
      f.close();
      return false;
    }
    // outputfile 2
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return false;
      }
    } else {
      f.close();
      return false;
    }
    // outputfile 3
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return false;
      }
    } else {
      f.close();
      return false;
    }
    // other lines
    f.close();
  } else {
    return false;
  }
  return true;
}

void US_Hydrodyn::anaflex_edit() {
  QString filename;
  if (anaflex_last_file != "") {
    // ask if use existing or new
    QFileInfo fi(anaflex_last_file);
    switch (QMessageBox::question(
        this, us_tr("View/Edit Anaflex Files"),
        QString(us_tr("View/Edit current file ") + fi.fileName() + "?"),
        QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel)) {
      case QMessageBox::Yes:
        filename = anaflex_last_file;
        break;
      case QMessageBox::No:
        filename = QFileDialog::getOpenFileName(
            this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
        break;
      case QMessageBox::Cancel:
      default:
        return;
        break;
    }
  } else {
    filename = QFileDialog::getOpenFileName(
        this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
  }
  if (!filename.isEmpty()) {
    QFileInfo fi(filename);
    QString dir = fi.path();
    // open file and view this and all associated files if first line ends with
    // .txt or .TXT
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
      editor_msg(
          "red",
          QString(
              us_tr(
                  "\nCould not open file %1 for reading. Check permissions.\n"))
              .arg(filename));
      return;
    }
    view_file(filename);
    f.close();
  }
}

void US_Hydrodyn::anaflex_run() { run_anaflex(); }

void US_Hydrodyn::anaflex_load_results() {
  QString filename;
  if (anaflex_last_file != "") {
    // ask if use existing or new
    QFileInfo fi(anaflex_last_file);
    switch (QMessageBox::question(
        this, us_tr("Load Anaflex Results"),
        QString(us_tr("View/Edit current file ") + fi.fileName() + "?"),
        QMessageBox::Yes, QMessageBox::No, QMessageBox::Cancel)) {
      case QMessageBox::Yes:
        filename = anaflex_last_file;
        break;
      case QMessageBox::No:
        filename = QFileDialog::getOpenFileName(
            this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
        break;
      case QMessageBox::Cancel:
      default:
        return;
        break;
    }
  } else {
    filename = QFileDialog::getOpenFileName(
        this, windowTitle(), somo_dir + SLASH + "bd", "*.txt *.TXT");
  }
  if (!filename.isEmpty()) {
    QFileInfo fi(filename);
    QString dir = fi.path();
    // open file and view this and all associated files if first line ends with
    // .txt or .TXT
    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly)) {
      editor_msg(
          "red",
          QString(
              us_tr(
                  "\nCould not open file %1 for reading. Check permissions.\n"))
              .arg(filename));
      return;
    }
    view_file(filename);
    if (!anaflex_valid_anaflex_main(filename)) {
      f.close();
      return;
    }
    switch (QMessageBox::question(
        this, us_tr("Load Anaflex Files"),
        QString(us_tr(fi.fileName() + " appears to be a 'main' Anaflex file, "
                                      "open all ancillary files? ")),
        QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton)) {
      case QMessageBox::No:
        f.close();
        return;
        break;
      case QMessageBox::Yes:
      default:
        break;
    }

    QTextStream ts(&f);
    QString tmp_filename;
    // outputfile 1
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return;
      }
      bd_edit_util(dir, tmp_filename);
    } else {
      f.close();
      return;
    }
    // outputfile 2
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return;
      }
      bd_edit_util(dir, tmp_filename);
    } else {
      f.close();
      return;
    }
    // outputfile 3
    if (!ts.atEnd()) {
      ts >> tmp_filename;
      ts.readLine();
      if (!tmp_filename.contains(QRegExp("(txt|TXT)$"))) {
        f.close();
        return;
      }
      bd_edit_util(dir, tmp_filename);
    } else {
      f.close();
      return;
    }
    f.close();
  }
}
