#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os,
                                            const QString& str) {
  return os << qPrintable(str);
}

ostream& operator<<(ostream& out, const point& c) {
  out << "(" << c.axis[0] << "," << c.axis[1] << "," << c.axis[2] << ")";
  return out;
}

bool US_Saxs_Util::a2sb_validate() {
  if (!validate_control_parameters()) {
    return false;
  }

  if (!nsa_validate()) {
    return false;
  }

  setup_saxs_options();

  if (!sgp_init_sgp()) {
    return false;
  }

  QString missing_required;

  QStringList qsl_required;

  {
    qsl_required << "a2sbcubeside";

    for (unsigned int i = 0; i < (unsigned int)qsl_required.size(); i++) {
      if (!control_parameters.count(qsl_required[i])) {
        missing_required += " " + qsl_required[i];
      }
    }
  }

  if (!missing_required.isEmpty()) {
    errormsg = QString("Error: a2sb requires prior definition of:%1")
                   .arg(missing_required);
    return false;
  }

  a2sb_cube_side = control_parameters["a2sbcubeside"].toFloat();
  return true;
}

point US_Saxs_Util::a2sb_cubelet(point p1) {
  float half_cube_side = a2sb_cube_side / 2.0;

  for (unsigned int i = 0; i < 3; i++) {
    bool negative = (p1.axis[i] < 0);
    p1.axis[i] = ((int)(p1.axis[i] / a2sb_cube_side)) * a2sb_cube_side;
    if (negative) {
      p1.axis[i] -= half_cube_side;
    } else {
      p1.axis[i] += half_cube_side;
    }
  }
  return p1;
}

point US_Saxs_Util::a2sb_cubelet(PDB_atom& this_atom) {
  return a2sb_cubelet(this_atom.coordinate);
}

/*
bool US_Saxs_Util::a2sb_test()
{
   // test some points

   if ( !a2sb_validate() )
   {
      return false;
   }

   point p1;

   cout << "current cube side: " << a2sb_cube_side << endl;

   for ( float x = -2.0; x <= 2.0; x += 0.5 )
   {
      p1.axis[ 0 ] =
         p1.axis[ 1 ] =
         p1.axis[ 2 ] = x;
      cout << "Original point: " << p1 << " Cublet: " << a2sb_cubelet( p1 ) <<
endl;
   }

   return true;
}
*/

bool US_Saxs_Util::a2sb_run() {
  // test some points

  if (!a2sb_validate()) {
    return false;
  }

  cout << "current cube side: " << a2sb_cube_side << endl;

  if (!read_pdb(control_parameters["inputfile"])) {
    return false;
  }

  map<QString, double> save_sgp_params = sgp_params;
  map<QString, QString> save_control_parameters = control_parameters;
  vector<double> save_sgp_exp_q = sgp_exp_q;
  map<double, unsigned int> save_sgp_exp_q_index = sgp_exp_q_index;
  vector<double> save_sgp_exp_I = sgp_exp_I;
  vector<double> save_sgp_exp_e = sgp_exp_e;
  bool save_sgp_use_e = sgp_use_e;
  // these should probably be externally accessible...
  sgp_params["distancequantum"] = 0.001;
  sgp_params["radiusmin"] = 1.0;
  sgp_params["radiusmax"] = 1000000.0;

  bool failed = false;

  for (unsigned int current_model = 0;
       current_model < (unsigned int)model_vector_as_loaded.size();
       current_model++) {
    if (failed) {
      break;
    }

    // select one model at a time

    model_vector.clear();
    model_vector.push_back(model_vector_as_loaded[0]);

    // build map

    a2sb_map.clear();

    unsigned int tot_atoms = 0;

    for (unsigned int j = 0; j < model_vector[0].molecule.size(); j++) {
      for (unsigned int k = 0; k < model_vector[0].molecule[j].atom.size();
           k++) {
        tot_atoms++;
        PDB_atom* this_atom = &(model_vector[0].molecule[j].atom[k]);
        a2sb_map[a2sb_cubelet(*this_atom)].push_back(*this_atom);
      }
    }

    vector<PDB_atom> a2sb_model;

    for (map<point, vector<PDB_atom> >::iterator it = a2sb_map.begin();
         it != a2sb_map.end(); it++) {
      model_vector.clear();

      // make it->second into a PDB_model:

      PDB_chain tmp_chain;
      tmp_chain.atom = it->second;
      PDB_model tmp_model;
      tmp_model.molecule.push_back(tmp_chain);
      model_vector.push_back(tmp_model);

      // compute saxs curve using full debye
      control_parameters["iqmethod"] = "db";

      control_parameters["a2sb_running"] = "running";

      if (!run_iqq()) {
        failed = true;
        break;
      }

      sgp_exp_q = sgp_last_q;
      sgp_exp_I = sgp_last_I;
      sgp_exp_e.clear();
      sgp_exp_q_index.clear();
      sgp_use_e = false;
      for (unsigned int i = 0; i < sgp_exp_q.size(); i++) {
        sgp_exp_q_index[sgp_exp_q[i]] = i;
      }

      if (!nsa_fitness_setup(1)) {
        failed = true;
        break;
      }
      control_parameters["sgp_running"] = "yes";

      double nrmsd;

      our_vector* v = new_our_vector(1);
      // starting radius is half cube side
      v->d[0] = a2sb_cube_side * 0.5;

      // compute 1sa using original iq method
      control_parameters["iqmethod"] = save_control_parameters["iqmethod"];
      if (!nsa_gsm(nrmsd, v)) {
        failed = true;
        break;
      }
      control_parameters.erase("sgp_running");
      control_parameters.erase("a2sb_running");
      free_our_vector(v);
      // add 1sa to a2sb_model
      bead_models[0][0].bead_coordinate = it->first;
      a2sb_model.push_back(bead_models[0][0]);
    }
    cout << "end of create bead model loop\n" << flush;

    if (failed) {
      break;
    }

    // write out 1sa model ( ? with difference to saxs curve of original model )

    a2sb_map.clear();

    bead_models[0] = a2sb_model;

    if (control_parameters.count("a2sbequalize")) {
      // equalize bead sizes:
      double avg_size = 0e0;

      for (unsigned int i = 0; i < bead_models[0].size(); i++) {
        avg_size += bead_models[0][i].bead_computed_radius;
      }

      avg_size /= bead_models[0].size();

      for (unsigned int i = 0; i < bead_models[0].size(); i++) {
        bead_models[0][i].bead_computed_radius = avg_size;
      }
    }

    if (control_parameters.count("a2sbcubesize")) {
      // make each bead the size of a cube
      for (unsigned int i = 0; i < bead_models[0].size(); i++) {
        bead_models[0][i].bead_computed_radius = a2sb_cube_side * 0.5;
      }
    }

    QString outname =
        QString("%1%2-%3-%4%5%6-a2sb")
            .arg(QFileInfo(control_parameters["inputfile"]).completeBaseName())
            .arg(model_vector_as_loaded.size()
                     ? QString("-%1").arg(current_model + 1)
                     : "")
            .arg(control_parameters["nsagsm"])
            .arg(QString("cs%1").arg(a2sb_cube_side).replace(".", "_"))
            .arg(control_parameters.count("a2sbequalize") ? "-eq" : "")
            .arg(control_parameters.count("a2sbcubesize") ? "-es" : "");

    // compute saxs curve:

    control_parameters["outputfile"] = outname;
    nsa_fitness();

    QFile f(QString("%1.bead_model").arg(outname));

    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << nsa_qs_bead_model();
      ts << nsa_physical_stats();
      ts << QString(
                "original atoms in structure %1\n"
                "number of beads             %2\n")
                .arg(tot_atoms)
                .arg(bead_models[0].size());
      f.close();
      cout << QString("written: %1\n").arg(f.fileName());
      output_files << f.fileName();
    } else {
      errormsg = QString("Error: could not create file %1").arg(f.fileName());
      failed = true;
      break;
    }
  }

  cout << "done calcs\n" << flush;

  // clean up, restore state:

  a2sb_map.clear();

  model_vector = model_vector_as_loaded;

  sgp_params = save_sgp_params;
  control_parameters = save_control_parameters;
  sgp_exp_q = save_sgp_exp_q;
  sgp_exp_q_index = save_sgp_exp_q_index;
  sgp_exp_I = save_sgp_exp_I;
  sgp_exp_e = save_sgp_exp_e;
  sgp_use_e = save_sgp_use_e;

  cout << "returning\n" << flush;

  return !failed;
}
