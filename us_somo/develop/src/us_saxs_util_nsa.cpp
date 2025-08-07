#if defined(USE_MPI)
#include <mpi.h>
// Added by qt3to4:
#include <QTextStream>
extern int myrank;
#endif

#include "../include/us_hydrodyn_pat.h"
#include "../include/us_saxs_gp.h"
#include "../include/us_saxs_util.h"

#if defined(USE_MPI)
#include <mpi.h>
extern int myrank;
#endif

// #define USUN_DEBUG

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char> &operator<<(std::basic_ostream<char> &os,
                                            const QString &str) {
  return os << qPrintable(str);
}

bool US_Saxs_Util::nsa_validate() {
  errormsg = "";
  noticemsg = "";

  // need to be able to process iq:
  if (!validate_control_parameters(true)) {
    return false;
  }

  // first check required parameters

  QString missing_required;

  QStringList qsl_required;

  {
    qsl_required << "targetedensity";
    if (control_parameters.count("nsaga")) {
      qsl_required << "nsagenerations";
      qsl_required << "nsapopulation";
    }
    qsl_required << "nsaiterations";
    qsl_required << "nsaepsilon";
    qsl_required << "nsagsm";

    for (unsigned int i = 0; i < (unsigned int)qsl_required.size(); i++) {
      if (!control_parameters.count(qsl_required[i])) {
        missing_required += " " + qsl_required[i];
      }
    }
  }

  if (!missing_required.isEmpty()) {
    errormsg = QString("Error: nsa requires prior definition of:%1")
                   .arg(missing_required);
    return false;
  }

  // if certain parameters are not set, set them to defaults
  QStringList checks;
  QStringList vals;

  {
    checks << "sgpdistancequantum";
    vals << "2.5";
    checks << "sgpdistancemin";
    vals << "2.0";
    checks << "sgpdistancemax";
    vals << "10.0";
    checks << "sgpradiusmin";
    vals << "1.0";
    checks << "sgpradiusmax";
    vals << "5.0";
    checks << "sgpbranchmax";
    vals << "4.0";

    checks << "nsamutate";
    vals << "0.1";
    checks << "nsacrossover";
    vals << "0.3";

    validate_control_parameters_set_one(checks, vals);
  }

  if (!sgp_exp_q.size() || sgp_exp_q.size() != sgp_exp_I.size()) {
    errormsg =
        "Error: empty or inconsistant grid found experimental data, note:csv "
        "experiment grids not currently supported for sgp\n";
    return false;
  }

#if defined(USE_MPI)
  if (!nsa_mpi || !myrank) {
#endif
    cout << QString("setup experiment grid sizes %1 %2 %3\n")
                .arg(sgp_exp_q.size())
                .arg(sgp_exp_I.size())
                .arg(sgp_exp_e.size());
#if defined(USE_MPI)
  }
#endif

  if (sgp_exp_e.size() && sgp_exp_e.size() == sgp_exp_q.size() &&
      is_nonzero_vector(sgp_exp_q)) {
    sgp_use_e = true;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa using sd's for fitting\n";
#if defined(USE_MPI)
    }
#endif
  } else {
    sgp_use_e = false;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa NOT using sd's for fitting\n";
#if defined(USE_MPI)
    }
#endif
  }

  if (control_parameters.count("nsascale") &&
      control_parameters["nsascale"].toLower().left(1) == "y") {
    nsa_use_scaling_fit = true;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa using scaling fits\n";
#if defined(USE_MPI)
    }
#endif
  } else {
    nsa_use_scaling_fit = false;
#if defined(USE_MPI)
    if (!nsa_mpi || !myrank) {
#endif
      cout << "Notice: nsa NOT using scaling fits\n";
#if defined(USE_MPI)
    }
#endif
  }

  return true;
}

bool US_Saxs_Util::check_overlap(vector<PDB_atom> &bm, bool quiet) {
  bool overlaps_found = false;
  for (unsigned int i = 0; i < bm.size() - 1; i++) {
    for (unsigned int j = i + 1; j < bm.size() - 1; j++) {
      if (bm[i].bead_computed_radius + bm[j].bead_computed_radius >
          dist(bm[i].bead_coordinate, bm[j].bead_coordinate)) {
        if (quiet) {
          return true;
        }
        overlaps_found = true;
        cout << QString(
                    "overlap found between beads %1 %2 with distance of %3 and "
                    "overlap of %4\n")
                    .arg(i + 1)
                    .arg(j + 1)
                    .arg(dist(bm[i].bead_coordinate, bm[j].bead_coordinate))
                    .arg(bm[i].bead_computed_radius +
                         bm[j].bead_computed_radius -
                         dist(bm[i].bead_coordinate, bm[j].bead_coordinate));
        cout << QString("        %1: (%2,%3,%4) r %5\n")
                    .arg(i)
                    .arg(bm[i].bead_coordinate.axis[0])
                    .arg(bm[i].bead_coordinate.axis[1])
                    .arg(bm[i].bead_coordinate.axis[2])
                    .arg(bm[i].bead_computed_radius);
        cout << QString("        %1: (%2,%3,%4) r %5\n")
                    .arg(j)
                    .arg(bm[j].bead_coordinate.axis[0])
                    .arg(bm[j].bead_coordinate.axis[1])
                    .arg(bm[j].bead_coordinate.axis[2])
                    .arg(bm[j].bead_computed_radius);
      }
    }
  }
  return overlaps_found;
}

double US_Saxs_Util::nsa_fitness() {
  // take node & run current bead model iq on its bead model and compute chi2
  // (if errors present) or rmsd & return

  // compute iq:
  if (nsa_ess) {
    for (unsigned int i = 1; i < bead_models[0].size(); i++) {
      bead_models[0][i].bead_computed_radius =
          bead_models[0][0].bead_computed_radius;
    }
  }

  if (nsa_excl) {
    // cout << "nsa_excl checking for overlaps\n";
    // move out each bead until non-overlap with all previous beads
    unsigned int overlap_count = 0;
    bool overlaps_found;
    do {
      overlaps_found = false;
      for (unsigned int i = 1; i < bead_models[0].size(); i++) {
        float r1 = bead_models[0][i].bead_computed_radius;
        point p1 = bead_models[0][i].bead_coordinate;

        float r2;
        point p2;
        for (unsigned int j = 0; j < i; j++) {
          r2 = bead_models[0][j].bead_computed_radius;
          p2 = bead_models[0][j].bead_coordinate;

          float d = dist(p1, p2);
          if (d < r1 + r2) {
            overlaps_found = true;
            overlap_count++;
            if ((overlap_count > 1000) && !(overlap_count % 1000)) {
              cout << QString("overlap check count %1\n").arg(overlap_count);
            }

            // overlap exists
            // expand along the axis from p2 to p1
            point pn = minus(p1, p2);
            if (pn.axis[0] == 0.0 && pn.axis[1] == 0.0 && pn.axis[2] == 0.0) {
              // expand out in p1's coordinate
              pn = p1;
              if (pn.axis[0] == 0.0 && pn.axis[1] == 0.0 && pn.axis[2] == 0.0) {
                // ouch, still zero?
                pn.axis[0] = 1.0;
              }
            }
            float overlap = r1 + r2 - d + 1e-7;
            pn = scale(normal(pn), overlap);
            p1 = plus(p1, pn);
            bead_models[0][i].bead_coordinate = p1;
          }
        }
      }
    } while (overlaps_found && overlap_count < 2000);

    if (check_overlap(bead_models[0], false)) {
      cout << QString("argh, overlaps found after fix attempt!\n") << flush;
      QFile f("overlap_error__model.bead_model");
      if (f.open(QIODevice::WriteOnly)) {
        QTextStream ts(&f);
        ts << nsa_qs_bead_model();
        f.close();
      }
#if defined(USE_MPI)
      MPI_Abort(MPI_COMM_WORLD, -55001);
#endif
      exit(-55001);
    }

    // cout << "overlaps fixed\n";
  }

  if (bead_models[0].size() > 1) {
    run_iqq_bead_model();
  } else {
    // cout << "single sphere fit\n";
    double delta_rho = control_parameters["targetedensity"].toDouble() -
                       our_saxs_options.water_e_density;
    if (fabs(delta_rho) < 1e-5) {
      delta_rho = 0e0;
    }

    if (!iqq_sphere("tmp", bead_models[0][0].bead_computed_radius, delta_rho,
                    our_saxs_options.start_q, our_saxs_options.end_q,
                    our_saxs_options.delta_q)) {
      cout << errormsg;
      return false;
    }
    sgp_last_I = wave["tmp"].r;
    write_output("0", wave["tmp"].q, wave["tmp"].r);
  }

  // values stored in last_q, last_I, experiment in sgp_exp_q,I,e
  nsa_last_scaling = 1e0;
  double chi2;
  bool fit_ok;
  if (nsa_use_scaling_fit) {
    if (sgp_use_e) {
      fit_ok =
          scaling_fit(sgp_last_I, sgp_exp_I, sgp_exp_e, nsa_last_scaling, chi2);
    } else {
      fit_ok = scaling_fit(sgp_last_I, sgp_exp_I, nsa_last_scaling, chi2);
    }
  } else {
    if (sgp_use_e) {
      fit_ok = calc_mychi2(sgp_last_I, sgp_exp_I, sgp_exp_e, chi2);
    } else {
      fit_ok = calc_myrmsd(sgp_last_I, sgp_exp_I, chi2);
    }
  }

#if defined(USE_MPI)
  if (!fit_ok) {
    MPI_Abort(MPI_COMM_WORLD, -55545);
    exit(-55545);
  }
#endif

  if (!fit_ok || us_isnan(chi2)) {
    QFile f("nan_fitness_model.bead_model");
    if (f.open(QIODevice::WriteOnly)) {
      QTextStream ts(&f);
      ts << nsa_qs_bead_model();
      f.close();
    }
  }

  return chi2;
}

bool US_Saxs_Util::nsa_fitness_setup(unsigned int size) {
  if (!size) {
    errormsg = "nsa: size must be at least 1";
    return false;
  }

  compute_gsm_exponentials_is_on = false;

  nsa_delta_rho = control_parameters["targetedensity"].toDouble() -
                  our_saxs_options.water_e_density;
  nsa_ess = control_parameters.count("nsaess") ? true : false;
  nsa_excl = control_parameters.count("nsaexcl") ? true : false;

  // probably want to setup nsa_gsm_delta as a parameter or somehow optimize:
  // also may be variable dependent ( i.e. coordinate vs radius etc )
  nsa_gsm_delta = 1e-3;
  nsa_gsm_delta2_r = 1e0 / (2e0 * nsa_gsm_delta);

  bead_models.resize(1);
  bead_models[0].clear();

  // essential for iqq:
  // bead_coordinate
  // bead_computed_radius
  // forcing rayleigh for this, so no need for excluded volume, saxs atoms etc

  PDB_atom tmp_atom;
  tmp_atom.bead_coordinate.axis[0] = 0.0;
  tmp_atom.bead_coordinate.axis[1] = 0.0;
  tmp_atom.bead_coordinate.axis[2] = 0.0;
  tmp_atom.bead_computed_radius = 0.0;
  tmp_atom.bead_actual_radius = 0.0;
  tmp_atom.radius = 0.0;
  tmp_atom.bead_mw = 20;
  tmp_atom.mw = 20;
  tmp_atom.bead_ref_volume = 0;
  tmp_atom.bead_color = 1;
  tmp_atom.exposed_code = 1;
  tmp_atom.all_beads.clear();
  tmp_atom.name = "SGP";
  tmp_atom.resName = "SGP";
  tmp_atom.iCode = "";
  tmp_atom.chainID = "";
  tmp_atom.chain = 1;
  tmp_atom.active = 1;
  tmp_atom.normalized_ot_is_valid = false;

  for (unsigned int i = 0; i < size; i++) {
    tmp_atom.serial = i + 1;
    bead_models[0].push_back(tmp_atom);
  }

  // size dependent variable setup:

  nsa_var_ref.clear();

  nsa_var_ref.push_back(&(bead_models[0][0].bead_computed_radius));
  nsa_var_min.push_back(sgp_params["radiusmin"] *
                        sgp_params["distancequantum"]);
  nsa_var_max.push_back(sgp_params["radiusmax"] *
                        sgp_params["distancequantum"]);

  if (size > 1) {
    if (!nsa_ess) {
      nsa_var_ref.push_back(&(bead_models[0][1].bead_computed_radius));
      nsa_var_min.push_back(sgp_params["radiusmin"] *
                            sgp_params["distancequantum"]);
      nsa_var_max.push_back(sgp_params["radiusmax"] *
                            sgp_params["distancequantum"]);
    }

    nsa_var_ref.push_back(&(bead_models[0][1].bead_coordinate.axis[0]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);
  }
  if (size > 2) {
    if (!nsa_ess) {
      nsa_var_ref.push_back(&(bead_models[0][2].bead_computed_radius));
      nsa_var_min.push_back(sgp_params["radiusmin"] *
                            sgp_params["distancequantum"]);
      nsa_var_max.push_back(sgp_params["radiusmax"] *
                            sgp_params["distancequantum"]);
    }

    nsa_var_ref.push_back(&(bead_models[0][2].bead_coordinate.axis[0]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);

    nsa_var_ref.push_back(&(bead_models[0][2].bead_coordinate.axis[1]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);
  }
  for (unsigned int j = 3; j < size; j++) {
    if (!nsa_ess) {
      nsa_var_ref.push_back(&(bead_models[0][j].bead_computed_radius));
      nsa_var_min.push_back(sgp_params["radiusmin"] *
                            sgp_params["distancequantum"]);
      nsa_var_max.push_back(sgp_params["radiusmax"] *
                            sgp_params["distancequantum"]);
    }

    nsa_var_ref.push_back(&(bead_models[0][j].bead_coordinate.axis[0]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);

    nsa_var_ref.push_back(&(bead_models[0][j].bead_coordinate.axis[1]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);

    nsa_var_ref.push_back(&(bead_models[0][j].bead_coordinate.axis[2]));
    nsa_var_min.push_back(sgp_params["distancemin"] *
                          sgp_params["distancequantum"]);
    nsa_var_max.push_back(sgp_params["distancemax"] *
                          sgp_params["distancequantum"]);
  }

  nsa_gsm_setup = true;
  // cout << "Variable limits setup:\n";
  // for ( unsigned int i = 0; i < nsa_var_min.size(); i++ )
  // {
  // cout << QString( "%1 [%2:%3]\n" )
  // .arg( i )
  // .arg( nsa_var_min[ i ] )
  // .arg( nsa_var_max[ i ] );
  // }

  return true;
}

double US_Saxs_Util::nsa_gsm_f(our_vector *v) {
  if (compute_gsm_exponentials_is_on) {
    return compute_gsm_exponentials_f(v);
  }

  for (int i = 0; i < v->len; i++) {
    if (v->d[i] < nsa_var_min[i]) {
      v->d[i] = nsa_var_min[i];
    }
    if (v->d[i] > nsa_var_max[i]) {
      v->d[i] = nsa_var_max[i];
    }
    *(nsa_var_ref[i]) = v->d[i];
  }
  return nsa_fitness();
}

void US_Saxs_Util::nsa_gsm_df(our_vector *vd, our_vector *v) {
  double y0;
  double y2;
  double sav_ve;

  // compute partials for each our_vector element

  for (int i = 0; i < v->len; i++) {
    sav_ve = v->d[i];
    v->d[i] -= nsa_gsm_delta;
    y0 = nsa_gsm_f(v);
    v->d[i] = sav_ve + nsa_gsm_delta;
    y2 = nsa_gsm_f(v);
    vd->d[i] = (y2 - y0) * nsa_gsm_delta2_r;
    v->d[i] = sav_ve;
  }
}

bool US_Saxs_Util::nsa_gsm(double &nrmsd, our_vector *vi, QString method) {
#if defined(USUN_DEBUG)
#if defined(USE_MPI)
  cout << QString("%1: entering nsa_gsm\n").arg(myrank) << fflush;
#else
  cout << QString("entering nsa_gsm vector\n") << fflush;
#endif
#endif

  errormsg = "";
  global_iter = 0;
  if (!nsa_gsm_setup) {
    errormsg = "nsa_gsm not setup";
    return false;
  }

  our_vector *v = new_our_vector(nsa_var_ref.size());
  if (vi && vi->len == v->len) {
    copy_our_vector(v, vi);
  } else {
    for (int i = 0; i < v->len; i++) {
      v->d[i] = nsa_var_min[i] + (nsa_var_max[i] - nsa_var_min[i]) / 2.0;
    }
  }

#if defined(PRINT_GSM_INFO)
  for (int i = 0; i < v->len; i++) {
    cout << QString("gsm initial v[ %1 ] = %2\n").arg(i).arg(v->d[i]);
  }
#endif

  int gsm_method = -1;

  if (method.isEmpty()) {
    method = control_parameters["nsagsm"];
  }

#if defined(USUN_DEBUG)
  method = "sd";
#endif

  if (method == "cg") {
    gsm_method = CONJUGATE_GRADIENT;
  }
  if (method == "sd") {
    gsm_method = STEEPEST_DESCENT;
  }
  if (method == "ih") {
    gsm_method = INVERSE_HESSIAN;
  }
#if defined(USUN_DEBUG)
#if defined(USE_MPI)
  cout << QString("%1: starting gsm\n").arg(myrank) << fflush;
#else
  cout << QString("starting gsm\n") << fflush;
#endif
#endif

  switch (gsm_method) {
    case CONJUGATE_GRADIENT:
      nsa_min_fr_pr_cgd(v, control_parameters["nsaepsilon"].toDouble(),
                        control_parameters["nsaiterations"].toLong());
      break;
    case STEEPEST_DESCENT:
      nsa_min_gsm_5_1(v, control_parameters["nsaepsilon"].toDouble(),
                      control_parameters["nsaiterations"].toLong());
      break;
    case INVERSE_HESSIAN:
      nsa_min_hessian_bfgs(v, control_parameters["nsaepsilon"].toDouble(),
                           control_parameters["nsaiterations"].toLong());
      break;
    default: {
      errormsg =
          QString("unknown gsm method %1 ").arg(control_parameters["nsagsm"]);
      free_our_vector(v);
      return false;
    } break;
  }
  // one extra to make sure best was last and to get nrmsd

  nrmsd = nsa_gsm_f(v);
  // bead_models[ 0 ] should have it now
  if (vi && vi->len == v->len) {
    copy_our_vector(vi, v);
  }
  free_our_vector(v);
#if defined(USUN_DEBUG)
#if defined(USE_MPI)
  cout << QString("%1: leaving nsa_gsm\n").arg(myrank) << fflush;
#else
  cout << QString("leaving nsa_gsm vector\n") << fflush;
#endif
#endif
  return true;
}

QString US_Saxs_Util::nsa_qs_bead_model() {
  QString qs;

  qs += QString("%1 %2\n").arg(bead_models[0].size()).arg(.7);

  for (unsigned int i = 0; i < bead_models[0].size(); i++) {
    qs += QString("%1 %2 %3 %4  20 8 Unk 10\n")
              .arg(bead_models[0][i].bead_coordinate.axis[0])
              .arg(bead_models[0][i].bead_coordinate.axis[1])
              .arg(bead_models[0][i].bead_coordinate.axis[2])
              .arg(bead_models[0][i].bead_computed_radius);
  }
  qs +=
      "\nModel scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : "
      "10\n";

  return qs;
}

QString US_Saxs_Util::nsa_physical_stats() {
  vector<PDB_atom> bm = bead_models[0];

  if (!bm.size()) {
    return "empty model\n";
  }

  // run pat? i.e. do a principal axis transformation

  {
    vector<dati1_supc> in_dt;

    for (unsigned int i = 0; i < bm.size(); i++) {
      dati1_supc dt;

      dt.x = bm[i].bead_coordinate.axis[0];
      dt.y = bm[i].bead_coordinate.axis[1];
      dt.z = bm[i].bead_coordinate.axis[2];
      dt.r = bm[i].bead_computed_radius;
      dt.ru = bm[i].bead_computed_radius;
      dt.m = (4.0 / 3.0) * M_PI * bm[i].bead_computed_radius *
             bm[i].bead_computed_radius * bm[i].bead_computed_radius;

      in_dt.push_back(dt);
    }

    int out_nat;
    vector<dati1_pat> out_dt(in_dt.size() + 1);

    if (!us_hydrodyn_pat_main((int)in_dt.size(), (int)in_dt.size(), &(in_dt[0]),
                              &out_nat, &(out_dt[0]))) {
      // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
      for (unsigned int i = 0; i < bm.size(); i++) {
        bm[i].bead_coordinate.axis[0] = out_dt[i].x;
        bm[i].bead_coordinate.axis[1] = out_dt[i].y;
        bm[i].bead_coordinate.axis[2] = out_dt[i].z;
        bm[i].bead_coordinate = bm[i].bead_coordinate;
        // bm[ i ].radius               = out_dt[ i ].r;
        // bm[ i ].bead_actual_radius   = out_dt[ i ].r;
        // bm[ i ].bead_computed_radius = out_dt[ i ].r;
      }
    }
  }

  // also compute intersection volumes & subtract?
  // every pair, remove intersection volume ?

  // compute average, center of electron density?

  double volume = 0e0;
  double volume_intersection = 0e0;
  point pmin;
  // initialization to shut up compiler warning
  pmin.axis[0] = 0.0f;
  pmin.axis[1] = 0.0f;
  pmin.axis[2] = 0.0f;
  point pmax = pmin;
  point prmin = pmin;
  point prmax = pmin;

  // subtract each radius from min add to max?
  // possible alternate bounding box ...

  for (unsigned int i = 0; i < bm.size(); i++) {
    if (i) {
      for (unsigned int j = 0; j < 3; j++) {
        if (pmin.axis[j] > bm[i].bead_coordinate.axis[j]) {
          pmin.axis[j] = bm[i].bead_coordinate.axis[j];
        }
        if (pmax.axis[j] < bm[i].bead_coordinate.axis[j]) {
          pmax.axis[j] = bm[i].bead_coordinate.axis[j];
        }

        if (prmin.axis[j] >
            bm[i].bead_coordinate.axis[j] - bm[i].bead_computed_radius) {
          prmin.axis[j] =
              bm[i].bead_coordinate.axis[j] - bm[i].bead_computed_radius;
        }
        if (prmax.axis[j] <
            bm[i].bead_coordinate.axis[j] + bm[i].bead_computed_radius) {
          prmax.axis[j] =
              bm[i].bead_coordinate.axis[j] + bm[i].bead_computed_radius;
        }
      }
    } else {
      pmin = bm[i].bead_coordinate;
      pmax = bm[i].bead_coordinate;
      prmin = bm[i].bead_coordinate;
      prmax = bm[i].bead_coordinate;
      for (unsigned int j = 0; j < 3; j++) {
        prmin.axis[j] -= bm[i].bead_computed_radius;
        prmax.axis[j] += bm[i].bead_computed_radius;
      }
    }

    volume += (4e0 / 3e0) * M_PI * bm[i].bead_computed_radius *
              bm[i].bead_computed_radius * bm[i].bead_computed_radius;
  }

  for (unsigned int i = 0; i < bm.size(); i++) {
    for (unsigned int j = i + 1; j < bm.size(); j++) {
      float d = dist(bm[i].bead_coordinate, bm[j].bead_coordinate);
      if (d < bm[i].bead_computed_radius + bm[j].bead_computed_radius) {
        float r1 = bm[i].bead_computed_radius;
        float r = bm[j].bead_computed_radius;
        if (d > 0.0) {
          volume_intersection += M_PI * (r1 + r - d) * (r1 + r - d) *
                                 (d * d + 2.0 * d * r + 2.0 * d * r1 -
                                  3.0 * r * r - 3.0 * r1 * r1 + 6.0 * r * r1) /
                                 (12.0 * d);
        } else {
          // one is within the other:
          if (r > r1) {
            r = r1;
          }
          volume_intersection += (4.0 / 3.0) * M_PI * r * r * r;
        }
      }
    }
  }

  nsa_physical_stats_map.clear();

  nsa_physical_stats_map["result total volume"] = QString("%1").arg(volume);
  nsa_physical_stats_map["result intersection volume"] =
      QString("%1").arg(volume_intersection);
  nsa_physical_stats_map["result excluded volume"] =
      QString("%1").arg(volume - volume_intersection);
  nsa_physical_stats_map["result centers bounding box size x"] =
      QString("%1").arg(pmax.axis[0] - pmin.axis[0]);
  nsa_physical_stats_map["result centers bounding box size y"] =
      QString("%1").arg(pmax.axis[1] - pmin.axis[1]);
  nsa_physical_stats_map["result centers bounding box size z"] =
      QString("%1").arg(pmax.axis[2] - pmin.axis[2]);

  nsa_physical_stats_map["result centers axial ratios x:z"] = QString("%1").arg(
      (pmax.axis[0] - pmin.axis[0]) / (pmax.axis[2] - pmin.axis[2]));
  nsa_physical_stats_map["result centers axial ratios x:y"] = QString("%1").arg(
      (pmax.axis[0] - pmin.axis[0]) / (pmax.axis[1] - pmin.axis[1]));
  nsa_physical_stats_map["result centers axial ratios y:z"] = QString("%1").arg(
      (pmax.axis[1] - pmin.axis[1]) / (pmax.axis[2] - pmin.axis[2]));

  nsa_physical_stats_map["result radial extent bounding box size x"] =
      QString("%1").arg(prmax.axis[0] - prmin.axis[0]);
  nsa_physical_stats_map["result radial extent bounding box size y"] =
      QString("%1").arg(prmax.axis[1] - prmin.axis[1]);
  nsa_physical_stats_map["result radial extent bounding box size z"] =
      QString("%1").arg(prmax.axis[2] - prmin.axis[2]);

  nsa_physical_stats_map["result radial extent axial ratios x:z"] =
      QString("%1").arg((prmax.axis[0] - prmin.axis[0]) /
                        (prmax.axis[2] - prmin.axis[2]));
  nsa_physical_stats_map["result radial extent axial ratios x:y"] =
      QString("%1").arg((prmax.axis[0] - prmin.axis[0]) /
                        (prmax.axis[1] - prmin.axis[1]));
  nsa_physical_stats_map["result radial extent axial ratios y:z"] =
      QString("%1").arg((prmax.axis[1] - prmin.axis[1]) /
                        (prmax.axis[2] - prmin.axis[2]));

  QString qs;

  qs += QString(
            "total        volume (A^3) %1\n"
            "intersection volume (A^3) %2\n"
            "excluded     volume (A^3) %3\n")
            .arg(volume)
            .arg(volume_intersection)
            .arg(volume - volume_intersection);

  qs += QString("centers bounding box size (A) %1 %2 %3\n")
            .arg(pmax.axis[0] - pmin.axis[0])
            .arg(pmax.axis[1] - pmin.axis[1])
            .arg(pmax.axis[2] - pmin.axis[2]);

  qs += QString("centers axial ratios: [x:z] = %1  [x:y] = %2  [y:z] = %3\n")
            .arg((pmax.axis[0] - pmin.axis[0]) / (pmax.axis[2] - pmin.axis[2]))
            .arg((pmax.axis[0] - pmin.axis[0]) / (pmax.axis[1] - pmin.axis[1]))
            .arg((pmax.axis[1] - pmin.axis[1]) / (pmax.axis[2] - pmin.axis[2]));

  qs += QString("radial extent bounding box size (A) %1 %2 %3\n")
            .arg(prmax.axis[0] - prmin.axis[0])
            .arg(prmax.axis[1] - prmin.axis[1])
            .arg(prmax.axis[2] - prmin.axis[2]);

  qs += QString(
            "radial extent axial ratios: [x:z] = %1  [x:y] = %2  [y:z] = %3\n")
            .arg((prmax.axis[0] - prmin.axis[0]) /
                 (prmax.axis[2] - prmin.axis[2]))
            .arg((prmax.axis[0] - prmin.axis[0]) /
                 (prmax.axis[1] - prmin.axis[1]))
            .arg((prmax.axis[1] - prmin.axis[1]) /
                 (prmax.axis[2] - prmin.axis[2]));

  return qs;
}
