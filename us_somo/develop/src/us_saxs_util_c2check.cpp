#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QTextStream>

bool US_Saxs_Util::c2check(QString a, QString b) {
  errormsg = "";
  noticemsg = "";
  if (!model_vector.size() || !control_parameters.count("inputfile")) {
    errormsg = "c2check requires an input pdb";
    return false;
  }

  vector<saxs_atom> atomsa;
  vector<saxs_atom> atomsb;

  // us_qdebug( QString( "caonly %1" ).arg( control_parameters.count(
  // "c2checkcaonly" ) ) );

  QString inputbase = QFileInfo(control_parameters["inputfile"]).baseName();

  for (unsigned int i = 0; i < model_vector.size(); i++) {
    saxs_atom new_atom;
    for (unsigned int j = 0; j < model_vector[current_model].molecule.size();
         j++) {
      for (unsigned int k = 0;
           k < model_vector[current_model].molecule[j].atom.size(); k++) {
        PDB_atom *this_atom =
            &(model_vector[current_model].molecule[j].atom[k]);
        if (this_atom->chainID == a || this_atom->chainID == b) {
          new_atom.pos[0] = this_atom->coordinate.axis[0];
          new_atom.pos[1] = this_atom->coordinate.axis[1];
          new_atom.pos[2] = this_atom->coordinate.axis[2];
          new_atom.atom_name = this_atom->name;
          new_atom.residue_name = this_atom->resName;
          if (new_atom.atom_name == "CA" ||
              !control_parameters.count("c2checkcaonly")) {
            this_atom->chainID == a ? atomsa.push_back(new_atom)
                                    : atomsb.push_back(new_atom);
          }
        }
      }
    }
    if (atomsa.size() != atomsb.size()) {
      errormsg = QString(
                     "Error: chain %1 has %2 atoms and chain %3 has %4 atoms "
                     "inputfile %5")
                     .arg(a)
                     .arg(atomsa.size())
                     .arg(b)
                     .arg(atomsb.size())
                     .arg(control_parameters["inputfile"]);
      return false;
    }
    if (!atomsa.size()) {
      errormsg = QString("Error: chain %1 and %2 have no atoms inputfile %3")
                     .arg(a)
                     .arg(atomsa.size())
                     .arg(b)
                     .arg(atomsb.size())
                     .arg(control_parameters["inputfile"]);
      return false;
    }

    vector<point> centers;
    vector<point> pa;
    vector<point> pb;
    point new_point;
    point org_a;
    point org_b;

    for (int k = 0; k < (int)atomsa.size(); ++k) {
      if (atomsa[k].atom_name != atomsb[k].atom_name ||
          atomsa[k].residue_name != atomsb[k].residue_name) {
        errormsg =
            QString("Error: name mismatch %1:%2:%3 vs %4:%5:%6 inputfile %7")
                .arg(a)
                .arg(atomsa[k].atom_name)
                .arg(atomsa[k].residue_name)
                .arg(b)
                .arg(atomsb[k].atom_name)
                .arg(atomsb[k].residue_name)
                .arg(control_parameters["inputfile"]);
        return false;
      }

      for (int j = 0; j < 3; ++j) {
        new_point.axis[j] = 0.5 * (atomsa[k].pos[j] + atomsb[k].pos[j]);
        org_a.axis[j] = atomsa[k].pos[j];
        org_b.axis[j] = atomsb[k].pos[j];
      }
      pa.push_back(org_a);
      pb.push_back(org_b);
      centers.push_back(new_point);
    }

    // us_qdebug( QString( "c2check ran providing %1 centers" ).arg(
    // centers.size() ) ); create output file

    QString plotlog;

    {
      QFile f(
          QString("%1_%2_%3_%4.txt").arg(inputbase).arg(i + 1).arg(a).arg(b));

      if (!f.open(QIODevice::WriteOnly)) {
        errormsg = QString("Error: can not create file %1").arg(f.fileName());
        return false;
      }

      plotlog += QString("\"%1\"").arg(f.fileName());

      QTextStream ts(&f);
      for (int j = 0; j < (int)centers.size(); ++j) {
        ts << QString("%1 %2 %3\n")
                  .arg(centers[j].axis[0])
                  .arg(centers[j].axis[1])
                  .arg(centers[j].axis[2]);
      }
      f.close();
      output_files << f.fileName();
    }
    {
      QFile f(QString("%1_%2_%3.txt").arg(inputbase).arg(i + 1).arg(a));

      if (!f.open(QIODevice::WriteOnly)) {
        errormsg = QString("Error: can not create file %1").arg(f.fileName());
        return false;
      }

      plotlog += QString(", \"%1\"").arg(f.fileName());

      QTextStream ts(&f);
      for (int j = 0; j < (int)pa.size(); ++j) {
        ts << QString("%1 %2 %3\n")
                  .arg(pa[j].axis[0])
                  .arg(pa[j].axis[1])
                  .arg(pa[j].axis[2]);
      }
      f.close();
      output_files << f.fileName();
    }
    {
      QFile f(QString("%1_%2_%3.txt").arg(inputbase).arg(i + 1).arg(b));

      if (!f.open(QIODevice::WriteOnly)) {
        errormsg = QString("Error: can not create file %1").arg(f.fileName());
        return false;
      }

      plotlog += QString(", \"%1\"").arg(f.fileName());

      QTextStream ts(&f);
      for (int j = 0; j < (int)pb.size(); ++j) {
        ts << QString("%1 %2 %3\n")
                  .arg(pb[j].axis[0])
                  .arg(pb[j].axis[1])
                  .arg(pb[j].axis[2]);
      }
      f.close();
      output_files << f.fileName();
    }

    // us_qdebug(  QString( "splot %1\n" ).arg( plotlog ) );

    {
      QFile f("plotlog");
      if (!f.open(QIODevice::WriteOnly | QIODevice::Append)) {
        errormsg = QString("Error: can not create file %1").arg(f.fileName());
        return false;
      }
      QTextStream ts(&f);
      ts << QString("splot %1\n").arg(plotlog);
      f.close();
    }
  }

  return true;
}
