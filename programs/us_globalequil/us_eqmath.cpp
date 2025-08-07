//! \file us_eqmath.cpp

#include "us_eqmath.h"

#include <cfloat>

#include "us_constants.h"
#include "us_math2.h"
#include "us_settings.h"

const double dflt_min = (double)FLT_MIN;
const double dflt_max = (double)FLT_MAX;

// Main constructor:  pass references to EditedData, ScanEdit, EqScanFit,
//  and EqRunFit objects needed by methods of this EqMath object
US_EqMath::US_EqMath(QVector<US_DataIO::EditedData>& dataList,
                     QVector<ScanEdit>& scedits, QVector<EqScanFit>& scanfits,
                     EqRunFit& runfit)
    : QObject(),
      dataList(dataList),
      scedits(scedits),
      scanfits(scanfits),
      runfit(runfit) {
  dbg_level = US_Settings::us_debug();
}

// Initialize parameters
void US_EqMath::init_params(int modx, bool update_mw, QList<double>& ds_vbar20s,
                            QList<double>& aud_pars) {
  modelx = modx;

  // Find the index to data corresponding to the first fitted scan
  int jdx = -1;

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (scanfits[ii].scanFit && jdx < 0) {
      jdx = scedits[ii].dsindex;
      break;
    }
  }

  DbgLv(1) << "EM:IP: jdx" << jdx << "modelx" << modelx << "update_mw"
           << update_mw;
  if (jdx < 0) return;

  double molecwt = runfit.mw_vals[0];
  double portion = molecwt;
  double mwll;
  double mwul;
  double uvbar;
  double dnumc;
  double mwinc;
  double tempa;
  double total;

  // Set runfit and scanfits parameters for the selected model
  switch (modelx) {
    case 0:  //  0: "1-Component, Ideal"
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      break;
    case 1:  //  1: "2-Component, Ideal, Noninteracting"
      if (update_mw) {
        runfit.mw_vals[0] = portion - (portion * 0.2);
        runfit.mw_vals[1] = portion + (portion * 0.2);
      }

      for (int ii = 0; ii < runfit.nbr_comps; ii++) {
        runfit.mw_rngs[ii] = runfit.mw_vals[ii] * 0.2;
        runfit.vbar_vals[ii] = ds_vbar20s[jdx];
        runfit.vbar_rngs[ii] = runfit.vbar_vals[ii] * 0.2;
      }

      if ((int)(100.0 * runfit.vbar_vals[1]) == 72) {
        runfit.vbar_vals[1] = runfit.vbar_vals[0];
        runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      }

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        portion = exp(scanfits[ii].amp_vals[0]) * 0.5;
        scanfits[ii].amp_vals[0] = log(portion * 0.7);
        scanfits[ii].amp_vals[1] = log(portion * 0.3);
        scanfits[ii].amp_rngs[0] = scanfits[ii].amp_vals[0] * 0.2;
        scanfits[ii].amp_rngs[1] = scanfits[ii].amp_rngs[0];
      }
      break;
    case 2:  //  2: "3-Component, Ideal, Noninteracting"
      if (update_mw) {
        runfit.mw_vals[0] = molecwt - (molecwt * 0.2);
        runfit.mw_vals[1] = molecwt;
        runfit.mw_vals[2] = molecwt + (molecwt * 0.2);
      }

      DbgLv(1) << "EM:IP: C2: nbrcomps" << runfit.nbr_comps;
      for (int ii = 0; ii < runfit.nbr_comps; ii++) {
        runfit.mw_rngs[ii] = runfit.mw_vals[ii] * 0.2;
        runfit.vbar_vals[ii] = ds_vbar20s[jdx];
        runfit.vbar_rngs[ii] = runfit.vbar_vals[ii] * 0.2;
      }

      if ((int)(100.0 * runfit.vbar_vals[1]) == 72) {
        runfit.vbar_vals[1] = runfit.vbar_vals[0];
        runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      }
      if ((int)(100.0 * runfit.vbar_vals[2]) == 72) {
        runfit.vbar_vals[2] = runfit.vbar_vals[0];
        runfit.vbar_rngs[2] = runfit.vbar_vals[2] * 0.2;
      }

      total = runfit.mw_vals[0] + runfit.mw_vals[1] + runfit.mw_vals[2];

      DbgLv(1) << "EM:IP: C2: total" << total;
      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        portion = exp(scanfits[ii].amp_vals[0]) / 3.0;
        scanfits[ii].amp_vals[0] = log(portion * 0.6);
        scanfits[ii].amp_vals[1] = log(portion * 0.3);
        scanfits[ii].amp_vals[2] = log(portion * 0.1);
        scanfits[ii].amp_rngs[0] = scanfits[ii].amp_vals[0] * 0.2;
        scanfits[ii].amp_rngs[1] = scanfits[ii].amp_rngs[0];
        scanfits[ii].amp_rngs[2] = scanfits[ii].amp_rngs[0];
      }
      break;
    case 3:  //  3: "Fixed Molecular Weight Distribution"
      runfit.nbr_comps = aud_pars[0];
      mwll = aud_pars[1];
      mwul = aud_pars[2];
      uvbar = aud_pars[3];
      uvbar = uvbar > 0.0 ? uvbar : ds_vbar20s[jdx];
      dnumc = (double)runfit.nbr_comps;
      mwinc = (mwul - mwll) / (dnumc - 1.0);
      tempa = log(1.0e-7 / dnumc);

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        for (int jj = 0; jj < runfit.nbr_comps; jj++) {
          scanfits[ii].amp_vals[jj] = tempa;
          scanfits[ii].amp_rngs[jj] = scanfits[ii].amp_vals[jj] * 0.2;
          scanfits[ii].baseline = 0.0;
          scanfits[ii].baseln_rng = 0.05;
          runfit.mw_vals[jj] = mwll + ((double)jj * mwinc);
          runfit.mw_rngs[jj] = runfit.mw_vals[jj] * 0.2;
          runfit.vbar_vals[jj] = uvbar;
          runfit.vbar_rngs[jj] = uvbar * 0.2;
        }
      }
      break;
    case 4:   //  4: "Monomer-Dimer Equilibrium"
    case 5:   //  5: "Monomer-Trimer Equilibrium"
    case 6:   //  6: "Monomer-Tetramer Equilibrium"
    case 7:   //  7: "Monomer-Pentamer Equilibrium"
    case 8:   //  8: "Monomer-Hexamer Equilibrium"
    case 9:   //  9: "Monomer-Heptamer Equilibrium"
    case 10:  // 10: "User-Defined Monomer-Nmer Equilibrium"
    case 11:  // 11: "Monomer-Dimer-Trimer Equilibrium"
    case 12:  // 12: "Monomer-Dimer-Tetramer Equilibrium"
    case 13:  // 13: "User-Defined Monomer - N-mer - M-mer Equilibrium"
      runfit.eq_vals[0] = -1.0;
      runfit.eq_vals[1] = -1.0e4;
      runfit.eq_rngs[0] = 5.0;
      runfit.eq_rngs[1] = 5.0;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      break;
    case 14:  // 14: "2-Component Hetero-Association: A + B <=> AB"
      if (update_mw) {
        runfit.mw_vals[0] = molecwt - (molecwt * 0.2);
        runfit.mw_vals[1] = molecwt + (molecwt * 0.2);
      }

      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;

      if ((int)(100.0 * runfit.vbar_vals[1]) == 72) {
        runfit.vbar_vals[1] = runfit.vbar_vals[0];
        runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      }

      runfit.eq_vals[0] = -1.0e4;
      runfit.eq_rngs[0] = 5.0e2;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        portion = exp(scanfits[ii].amp_vals[0]) / 2.0;
        scanfits[ii].amp_vals[0] = log(portion);
        scanfits[ii].amp_vals[1] = scanfits[ii].amp_vals[0];
        scanfits[ii].amp_rngs[0] = scanfits[ii].amp_vals[0] * 0.2;
        scanfits[ii].amp_rngs[1] = scanfits[ii].amp_rngs[0];
      }
      break;
    case 15:  // 15: "U-Defined self/Hetero-Assoc.: A + B <=> AB, nA <=> An"
      if (update_mw) {
        runfit.mw_vals[0] = molecwt - (molecwt * 0.2);
        runfit.mw_vals[1] = molecwt + (molecwt * 0.2);
      }

      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;

      if ((int)(100.0 * runfit.vbar_vals[1]) == 72) {
        runfit.vbar_vals[1] = runfit.vbar_vals[0];
        runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      }

      runfit.eq_vals[0] = -1.0e4;
      runfit.eq_rngs[0] = 5.0e2;
      runfit.eq_vals[1] = -1.0e4;
      runfit.eq_rngs[1] = 5.0e2;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        portion = exp(scanfits[ii].amp_vals[0]) / 2.0;
        scanfits[ii].amp_vals[0] = log(portion);
        scanfits[ii].amp_vals[1] = scanfits[ii].amp_vals[0];
        scanfits[ii].amp_rngs[0] = scanfits[ii].amp_vals[0] * 0.2;
        scanfits[ii].amp_rngs[1] = scanfits[ii].amp_rngs[0];
      }
      break;
    case 16:  // 16: "U-Defined Monomer-Nmer, some monomer is incompetent"
      runfit.mw_vals[1] = runfit.mw_vals[0];
      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      runfit.eq_vals[0] = -0.1;
      runfit.eq_rngs[0] = 5.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        scanfits[ii].amp_vals[1] = -1.0e4;
        scanfits[ii].amp_rngs[1] = 1.0e-3;
      }
      break;
    case 17:  // 17: "User-Defined Monomer-Nmer, some Nmer is incompetent"
      runfit.mw_vals[1] = runfit.stoichs[0] * runfit.mw_vals[0];
      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      runfit.eq_vals[0] = -0.1;
      runfit.eq_rngs[0] = 5.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        scanfits[ii].amp_vals[1] = -1.0e4;
        scanfits[ii].amp_rngs[1] = 1.0e-3;
      }
      break;
    case 18:  // 18: "User-Defined irreversible Monomer-Nmer"
      runfit.mw_vals[1] = runfit.stoichs[0] * runfit.mw_vals[0];
      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        portion = exp(scanfits[ii].amp_vals[0]) / 2.0;
        scanfits[ii].amp_vals[0] = log(portion * 0.7);
        scanfits[ii].amp_vals[1] = log(portion * 0.3);
        scanfits[ii].amp_rngs[0] = scanfits[ii].amp_vals[0] * 0.2;
        scanfits[ii].amp_rngs[1] = scanfits[ii].amp_rngs[0];
      }
      break;
    case 19:  // 19: "User-Defined Monomer-Nmer plus contaminant"
      runfit.mw_vals[1] = runfit.mw_vals[0];
      runfit.mw_rngs[0] = runfit.mw_vals[0] * 0.2;
      runfit.mw_rngs[1] = runfit.mw_vals[1] * 0.2;
      runfit.vbar_vals[0] = ds_vbar20s[jdx];
      runfit.vbar_rngs[0] = runfit.vbar_vals[0] * 0.2;
      runfit.vbar_vals[1] = ds_vbar20s[jdx];
      runfit.vbar_rngs[1] = runfit.vbar_vals[1] * 0.2;
      runfit.eq_vals[0] = -1.0e4;
      runfit.eq_rngs[0] = 5.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        if (!scanfits[ii].scanFit) continue;

        scanfits[ii].amp_vals[1] = -1.0e4;
        scanfits[ii].amp_rngs[1] = 1.0e-3;
      }
      break;
  }
#if 0
double aaa[ 12 ]; double ppp[ 16 ];
for ( int ii=0;ii<12;ii++ ) aaa[ii]=(double)( ii+1 );
for ( int ii=0;ii<16;ii++ ) ppp[ii]=0.0;
double* aa[3]; double* pp[4];
aa[0]=aaa;aa[1]=aaa+4;aa[2]=aaa+8;
pp[0]=ppp;pp[1]=ppp+4;pp[2]=ppp+8;pp[3]=ppp+12;
DbgLv(2) << "AA: " << aaa[0] << aaa[1] << aaa[2] << aaa[3];
DbgLv(2) << "AA: " << aaa[4] << aaa[5] << aaa[6] << aaa[7];
DbgLv(2) << "AA: " << aaa[8] << aaa[9] << aaa[10] << aaa[11];
US_Matrix::tmm( aa, pp, 3, 4, false );
DbgLv(2) << "==US_Matrix::tmm( aa, pp, 4, 3, false )==";
DbgLv(2) << " PP: " << ppp[0] << ppp[1] << ppp[2] << ppp[3];
DbgLv(2) << " PP: " << ppp[4] << ppp[5] << ppp[6] << ppp[7];
DbgLv(2) << " PP: " << ppp[8] << ppp[9] << ppp[10] << ppp[11];
DbgLv(2) << " PP: " << ppp[12] << ppp[13] << ppp[14] << ppp[15];
US_Matrix::tmm( aa, pp, 3, 4, true );
DbgLv(2) << "==US_Matrix::tmm( aa, pp, 4, 3, true )==";
DbgLv(2) << " PP: " << ppp[0] << ppp[1] << ppp[2] << ppp[3];
DbgLv(2) << " PP: " << ppp[4] << ppp[5] << ppp[6] << ppp[7];
DbgLv(2) << " PP: " << ppp[8] << ppp[9] << ppp[10] << ppp[11];
DbgLv(2) << " PP: " << ppp[12] << ppp[13] << ppp[14] << ppp[15];
#endif
}

// Initialize for fit
void US_EqMath::init_fit(int modx, int methx, FitCtrlPar& fitpars) {
  modelx = modx;    // Model type index
  nlsmeth = methx;  // NLS method index
  DbgLv(1) << "EM:IF: modelx nlsmeth nc" << modelx << nlsmeth
           << runfit.nbr_comps;

  // Find the index to the first fitted scan and count data sets, points
  ffitx = -1;
  ntpts = 0;
  ndsets = 0;
  nfpars = 0;
  nspts = 0;
  nslpts = 0;
  v_setpts.clear();
  v_setlpts.clear();

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (!scanfits[ii].scanFit) continue;

    if (ffitx < 0) ffitx = ii;

    nspts = scanfits[ii].stop_ndx - scanfits[ii].start_ndx + 1;
    v_setpts << nspts;
    v_setlpts << nspts;
    ndsets++;
    ntpts += nspts;
  }
  DbgLv(1) << "EM:IF: scan 1 ntpts ndsets nfpars" << ntpts << ndsets << nfpars;

  if (ffitx < 0 || ntpts == 0) return;

  // Count the number of fitting parameters
  for (int jj = 0; jj < runfit.nbr_comps; jj++) {
    if (runfit.mw_fits[jj] == true) nfpars++;

    if (runfit.vbar_fits[jj] == true) nfpars++;

    if (runfit.viri_fits[jj] == true) nfpars++;
  }
  DbgLv(1) << "EM:IF: scan 2 ntpts ndsets nfpars" << ntpts << ndsets << nfpars;

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (!scanfits[ii].scanFit) continue;

    EqScanFit* scnf = &scanfits[ii];

    for (int jj = 0; jj < runfit.nbr_comps; jj++)
      if (scnf->amp_fits[jj]) nfpars++;

    if (scnf->baseln_fit) nfpars++;
  }
  DbgLv(1) << "EM:IF: scan 3 ntpts ndsets nfpars" << ntpts << ndsets << nfpars;

  for (int jj = 0; jj < runfit.nbr_assocs; jj++)
    if (runfit.eq_fits[jj]) nfpars++;
  DbgLv(1) << "EM:IF: scan 4 ntpts ndsets nfpars" << ntpts << ndsets << nfpars;

  // Allocate vectors and matrices
  v_yraw.fill(0.0, ntpts);
  v_yguess.fill(0.0, ntpts);
  v_ydelta.fill(0.0, ntpts);
  v_BB.fill(0.0, nfpars);
  v_guess.fill(0.0, nfpars);
  v_tguess.fill(0.0, nfpars);

  m_jacobi.fill(0, ntpts);
  m_info.fill(0, nfpars);
  m_LLtrns.fill(0, nfpars);
  m_dcr2.fill(0, ndsets);
  m_dlncr2.fill(0, ndsets);
  m_lncr2.fill(0, ndsets);

  int njacs = ntpts * nfpars;
  int nfpsq = nfpars * nfpars;
  v_jacobi.fill(0.0, njacs);
  v_info.fill(0.0, nfpsq);
  v_LLtrns.fill(0.0, nfpsq);
  v_lncr2.fill(0.0, ntpts);
  v_dlncr2.fill(0.0, ntpts);
  v_dcr2.fill(0.0, ntpts);

  int dsx = 0;
  int ptx = 0;
  d_lncr2 = v_lncr2.data();
  d_dlncr2 = v_dlncr2.data();
  d_dcr2 = v_dcr2.data();

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (!scanfits[ii].scanFit) continue;

    EqScanFit* scnf = &scanfits[ii];

    m_dlncr2[dsx] = d_dlncr2;
    m_lncr2[dsx] = d_lncr2;
    m_dcr2[dsx] = d_dcr2;

    nspts = v_setpts[dsx++];
    d_dlncr2 += nspts;
    d_lncr2 += nspts;
    d_dcr2 += nspts;
    int jy = scnf->start_ndx;

    for (int jj = 0; jj < nspts; jj++) v_yraw[ptx++] = scnf->yvs[jy++];
  }
  DbgLv(1) << "EM:IF: matr fill 1 complete";

  // Initialize parameter guess
  guess_mapForward(v_guess.data());
  DbgLv(1) << "EM:IF: g map Forw complete";

  // Set up jacobian, info, and LLtranspose matrices
  d_jacobi = v_jacobi.data();
  d_info = v_info.data();
  d_LLtrns = v_LLtrns.data();

  for (int ii = 0; ii < ntpts; ii++) {
    m_jacobi[ii] = d_jacobi;
    d_jacobi += nfpars;
  }
  DbgLv(1) << "EM:IF: jacobi fill complete";

  for (int ii = 0; ii < nfpars; ii++) {
    m_info[ii] = d_info;
    m_LLtrns[ii] = d_LLtrns;
    d_info += nfpars;
    d_LLtrns += nfpars;
  }
  DbgLv(1) << "EM:IF: inf/trns fill complete";
  setpts = v_setpts.data();
  setlpts = v_setlpts.data();
  y_raw = v_yraw.data();
  y_guess = v_yguess.data();
  y_delta = v_ydelta.data();
  BB = v_BB.data();
  guess = v_guess.data();
  tguess = v_tguess.data();
  jacobian = m_jacobi.data();
  info = m_info.data();
  LLtr = m_LLtrns.data();
  dcr2 = m_dcr2.data();
  dlncr2 = m_dlncr2.data();
  lncr2 = m_lncr2.data();

  DbgLv(1) << "EM:FI: ffitx" << ffitx << "modelx" << modelx;
  // Return counts and other fit parameters to caller
  fitpars.nlsmeth = nlsmeth;
  fitpars.modelx = modelx;
  fitpars.ntpts = ntpts;
  fitpars.ndsets = ndsets;
  fitpars.nfpars = nfpars;
  fitpars.setpts = setpts;
  fitpars.setlpts = setlpts;
  fitpars.y_raw = y_raw;
  fitpars.y_guess = y_guess;
  fitpars.y_delta = y_delta;
  fitpars.BB = BB;
  fitpars.guess = guess;
  fitpars.tguess = tguess;
  fitpars.jacobian = jacobian;
  fitpars.info = info;
  fitpars.LLtr = LLtr;
  fitpars.dcr2 = dcr2;
  fitpars.dlncr2 = dlncr2;
  fitpars.lncr2 = lncr2;
  DbgLv(1) << "EM:FI: ktpts kdsets kfpars" << ntpts << ndsets << nfpars;
}

// Fill in guess parameter vector
//   Parameter order (needs to be maintained so Jacobian columns match):
//     for each component k:
//       1. Molecular Weight ( k )
//       2. Vbar ( k )
//       3. Virial Coefficient ( k )
//
//     for each scan:
//       for each component:
//         4. Amplitude
//       5. Baseline
//
//     for each association constant:
//       6. Association constant
//
void US_EqMath::guess_mapForward(double* vguess) {
  int jpx = 0;
  DbgLv(1) << "EM:gmF: ncomps" << runfit.nbr_comps;

  for (int jj = 0; jj < runfit.nbr_comps; jj++) {
    // DbgLv(1) << "EM:gmF: jj jpx" << jj << jpx;
    if (runfit.mw_fits[jj]) {
      vguess[jpx] = runfit.mw_vals[jj];
      runfit.mw_ndxs[jj] = jpx++;
    }

    if (runfit.vbar_fits[jj]) {
      vguess[jpx] = runfit.vbar_vals[jj];
      runfit.vbar_ndxs[jj] = jpx++;
    }

    if (runfit.viri_fits[jj]) {
      vguess[jpx] = runfit.viri_vals[jj];
      runfit.viri_ndxs[jj] = jpx++;
    }
  }

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (!scanfits[ii].scanFit) continue;

    // DbgLv(1) << "EM:gmF: ii jpx" << ii << jpx;
    EqScanFit* scnf = &scanfits[ii];

    for (int jj = 0; jj < runfit.nbr_comps; jj++) {
      if (scnf->amp_fits[jj]) {
        vguess[jpx] = scnf->amp_vals[jj];
        scnf->amp_ndxs[jj] = jpx++;
      }
    }

    if (scnf->baseln_fit) {
      vguess[jpx] = scnf->baseline;
      scnf->baseln_ndx = jpx++;
    }
  }

  for (int jj = 0; jj < runfit.nbr_assocs; jj++) {
    // DbgLv(1) << "EM:gmF: as_jj jpx" << jj << jpx;
    if (runfit.eq_fits[jj]) {
      vguess[jpx] = runfit.eq_vals[jj];
      runfit.eq_ndxs[jj] = jpx++;
    }
  }
  DbgLv(1) << "EM:gmF:   jpx" << jpx;
}

// Map parameters back from guesses
void US_EqMath::parameter_mapBackward(double* vguess) {
  int jpx = 0;

  for (int jj = 0; jj < runfit.nbr_comps; jj++) {
    if (runfit.mw_fits[jj]) runfit.mw_vals[jj] = vguess[jpx++];

    if (runfit.vbar_fits[jj]) runfit.vbar_vals[jj] = vguess[jpx++];

    if (runfit.viri_fits[jj]) runfit.viri_vals[jj] = vguess[jpx++];
  }

  for (int ii = 0; ii < scanfits.size(); ii++) {
    if (!scanfits[ii].scanFit) continue;

    EqScanFit* scnf = &scanfits[ii];

    for (int jj = 0; jj < runfit.nbr_comps; jj++) {
      if (scnf->amp_fits[jj]) scnf->amp_vals[jj] = vguess[jpx++];

      if (scnf->baseln_fit) scnf->baseline = vguess[jpx++];
    }
  }

  for (int jj = 0; jj < runfit.nbr_assocs; jj++)
    if (runfit.eq_fits[jj]) runfit.eq_vals[jj] = vguess[jpx++];

  if (modelx == 3) {
    EqScanFit* sc1f = &scanfits[ffitx];

    for (int ii = ffitx + 1; ii < scanfits.size(); ii++) {
      EqScanFit* scnf = &scanfits[ii];

      if (!scnf->scanFit) continue;

      for (int jj = 0; jj < runfit.nbr_comps; jj++)
        scnf->amp_vals[jj] = sc1f->amp_vals[jj];
    }
  }
}

// Calculate the jacobian matrix
int US_EqMath::calc_jacobian() {
  int stat = 0;
  int ncomps = runfit.nbr_comps;
  int mcomp = max(ncomps, 4);
  int jpx = 0;
  int jdx = 0;
  QVector<double> v_ufunc(mcomp);
  QVector<double> v_vbar(mcomp);
  QVector<double> v_buoy(mcomp);

  v_jacobi.fill(0.0, ntpts * nfpars);

  switch (modelx) {
    case 0:  //  0: "1-Component, Ideal"
    case 1:  //  1: "2-Component, Ideal, Noninteracting"
    case 2:  //  2: "3-Component, Ideal, Noninteracting"
    case 3:  //  3: "Fixed Molecular Weight Distribution"
      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        for (int kk = 0; kk < ncomps; kk++) {
          v_vbar[kk] = US_Math2::adjust_vbar20(runfit.vbar_vals[kk], tempera);
          v_buoy[kk] = (1.0 - v_vbar[kk] * density);
        }

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;

          for (int kk = 0; kk < ncomps; kk++) {
            double buoy = v_buoy[kk];
            double mwv = runfit.mw_vals[kk];
            double ampv = scnf->amp_vals[kk];
            double ufunc = exp(ampv + dconst * mwv * buoy * xv);
            v_ufunc[kk] = ufunc;

            if (runfit.mw_fits[kk])
              m_jacobi[jpx][runfit.mw_ndxs[kk]] = dconst * xv * buoy * ufunc;

            if (runfit.vbar_fits[kk])
              m_jacobi[jpx][runfit.vbar_ndxs[kk]] =
                  (-1.0) * dconst * mwv * xv * ufunc * density;

            if (scnf->amp_fits[kk]) m_jacobi[jpx][scnf->amp_ndxs[kk]] = ufunc;
          }

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    case 4:   //  4: "Monomer-Dimer Equilibrium"
    case 5:   //  5: "Monomer-Trimer Equilibrium"
    case 6:   //  6: "Monomer-Tetramer Equilibrium"
    case 7:   //  7: "Monomer-Pentamer Equilibrium"
    case 8:   //  8: "Monomer-Hexamer Equilibrium"
    case 9:   //  9: "Monomer-Heptamer Equilibrium"
    case 10:  // 10: "User-Defined Monomer-Nmer Equilibrium"
    {
      double stoich1 = (double)(modelx - 2);
      double stoiexp = stoich1 - 1.0;
      runfit.stoichs[0] = stoich1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double ODcorrec =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        double buoy = v_buoy[0];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double mwv = runfit.mw_vals[0];
          double ampv = scnf->amp_vals[0];
          double ufunc0 = exp(ampv + dconst * mwv * buoy * xv);
          double ufunc1 = exp(stoich1 * ampv + runfit.eq_vals[0] + ODcorrec +
                              dconst * stoich1 * mwv * buoy * ufunc0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          double dcoeff = dconst * xv * buoy;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                dcoeff * ufunc0 + dcoeff * ufunc1 * stoich1;

          dcoeff = dconst * mwv * xv * density;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * dcoeff * ufunc0 - dcoeff * ufunc1 * stoich1;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0 + ufunc1 * stoich1;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc1;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 11:  // 11: "Monomer-Dimer-Trimer Equilibrium"
    case 12:  // 12: "Monomer-Dimer-Tetramer Equilibrium"
    case 13:  // 13: "User-Defined Monomer - N-mer - M-mer Equilibrium"
    {
      double stoich1 = 2.0;
      double stoich2 = (double)(modelx - 8);
      runfit.stoichs[0] = stoich1;
      runfit.stoichs[1] = stoich2;
      double stoiexp = stoich1 - 1.0;
      double stoiex2 = stoich2 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double ODcorec1 =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp));
        double ODcorec2 =
            log(stoich2 / pow(scnf->extincts[0] * scnf->pathlen, stoiex2));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        double buoy = v_buoy[0];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double mwv = runfit.mw_vals[0];
          double ampv = scnf->amp_vals[0];
          double ufunc0 = exp(ampv + dconst * mwv * buoy * xv);
          double ufunc1 = exp(stoich1 * ampv + runfit.eq_vals[0] + ODcorec1 +
                              dconst * stoich1 * mwv * buoy * xv);
          double ufunc2 = exp(stoich2 * ampv + runfit.eq_vals[1] + ODcorec2 +
                              dconst * stoich2 * mwv * buoy * xv);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;
          double dcoeff = dconst * xv * buoy;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                dcoeff * ufunc0 * +dcoeff * ufunc1 * stoich1 +
                dcoeff * ufunc2 * stoich2;

          dcoeff = dconst * mwv * xv * density;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] = (-1.0) * dcoeff * ufunc0 -
                                                 dcoeff * ufunc1 * stoich1 -
                                                 dcoeff * ufunc2 * stoich2;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] =
                ufunc0 + ufunc1 * stoich1 + ufunc2 * stoich2;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc1;

          if (runfit.eq_fits[1]) m_jacobi[jpx][runfit.eq_ndxs[1]] = ufunc2;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 14:  // 14: "2-Component Hetero-Association: A + B <=> AB"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double mw_ab = mwv0 + mwv1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double extinc1 = scnf->extincts[1];
        double ODcorrec =
            log((extinc0 + extinc1) / (scnf->pathlen * extinc0 * extinc1));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        v_vbar[2] = (v_vbar[0] * mwv0 + v_vbar[1] * mwv1) / mw_ab;
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        v_buoy[1] = (1.0 - v_vbar[1] * density);
        v_buoy[2] = (1.0 - v_vbar[2] * density);
        double buoy0 = v_buoy[0];
        double buoy1 = v_buoy[1];
        double buoy2 = v_buoy[2];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(ampv0 + ampv1 + runfit.eq_vals[0] + ODcorrec +
                              constx * mw_ab * buoy2);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy2 +
                constx * (v_vbar[2] * density - v_vbar[0] * density) + ufunc2;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * constx * mwv1 * density * ufunc1 -
                constx * mwv1 * density * ufunc2;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0 + ufunc2;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc2;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 15:  // 15: "U-Defined self/Hetero-Assoc.: A + B <=> AB, nA <=> An"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double mw_ab = mwv0 + mwv1;
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double extinc1 = scnf->extincts[1];
        double ODcorec1 =
            log((extinc0 + extinc0) / (scnf->pathlen * extinc0 * extinc1));
        double ODcorec2 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        v_vbar[2] = (v_vbar[0] * mwv0 + v_vbar[1] * mwv1) / mw_ab;
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        v_buoy[1] = (1.0 - v_vbar[1] * density);
        v_buoy[2] = (1.0 - v_vbar[2] * density);
        double buoy0 = v_buoy[0];
        double buoy1 = v_buoy[1];
        double buoy2 = v_buoy[2];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(ampv0 + ampv1 + runfit.eq_vals[0] + ODcorec1 +
                              constx * mw_ab * buoy2);
          double ufunc3 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec2 +
                              constx * stoich1 * mw_ab * buoy0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;
          v_ufunc[3] = ufunc3;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy2 +
                constx * (v_vbar[2] * density - v_vbar[0] * density) +
                constx * buoy0 * ufunc3 * stoich1 + ufunc2;

          if (runfit.mw_fits[1])
            m_jacobi[jpx][runfit.mw_ndxs[1]] =
                constx * buoy1 * ufunc1 + constx * buoy2 +
                constx * (v_vbar[2] * density - v_vbar[1] * density) + ufunc2;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * constx * mwv0 * density * ufunc0 -
                constx * mwv0 * density * ufunc3 * stoich1 -
                constx * mwv0 * density * ufunc2;

          if (runfit.vbar_fits[1])
            m_jacobi[jpx][runfit.vbar_ndxs[1]] =
                (-1.0) * constx * mwv1 * density * ufunc1 -
                constx * mwv1 * density * ufunc2;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] =
                ufunc0 + ufunc3 * stoich1 + ufunc2;

          if (scnf->amp_fits[1])
            m_jacobi[jpx][scnf->amp_ndxs[1]] = ufunc1 + ufunc2;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc2;

          if (runfit.eq_fits[1]) m_jacobi[jpx][runfit.eq_ndxs[1]] = ufunc3;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 16:  // 16: "U-Defined Monomer-Nmer, some monomer is incompetent"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(extinc0 * scnf->pathlen, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        double buoy0 = v_buoy[0];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy0 * ufunc1 +
                constx * buoy0 * ufunc2 * stoich1;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * constx * mwv0 * density * ufunc0 -
                constx * mwv0 * density * ufunc1 -
                constx * mwv0 * density * ufunc2 * stoich1;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0 + stoich1 * ufunc2;

          if (scnf->amp_fits[1]) m_jacobi[jpx][scnf->amp_ndxs[1]] = ufunc1;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc2;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 17:  // 17: "User-Defined Monomer-Nmer, some Nmer is incompetent"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        double buoy0 = v_buoy[0];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy0 * ufunc1 * stoich1 +
                constx * buoy0 * ufunc2 * stoich1;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[1]] =
                (-1.0) * constx * mwv0 * density * ufunc0 -
                constx * mwv0 * density * ufunc1 * stoich1 -
                constx * mwv0 * density * ufunc2 * stoich1;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0 + ufunc2 * stoich1;

          if (scnf->amp_fits[1]) m_jacobi[jpx][scnf->amp_ndxs[1]] = ufunc1;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc2;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 18:  // 18: "User-Defined irreversible Monomer-Nmer"
    {
      double mwv0 = runfit.mw_vals[0];
      double stoich1 = runfit.stoichs[0];
      double mwv1 = mwv0 * stoich1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        double buoy0 = v_buoy[0];
        double ampv0 = scnf->amp_vals[0];
        double ampv1 = scnf->amp_vals[1];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy0 * ufunc1 * stoich1;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * constx * mwv0 * density * ufunc0 -
                constx * mwv0 * density * ufunc1 * stoich1;

          if (scnf->amp_fits[0]) m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0;

          if (scnf->amp_fits[1]) m_jacobi[jpx][scnf->amp_ndxs[1]] = ufunc1;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
    case 19:  // 19: "User-Defined Monomer-Nmer plus contaminant"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        v_buoy[0] = (1.0 - v_vbar[0] * density);
        v_buoy[1] = (1.0 - v_vbar[1] * density);
        double buoy0 = v_buoy[0];
        double buoy1 = v_buoy[1];
        double ampv0 = scnf->amp_vals[0];
        double ampv1 = scnf->amp_vals[1];

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          v_ufunc[0] = ufunc0;
          v_ufunc[1] = ufunc1;
          v_ufunc[2] = ufunc2;

          if (runfit.mw_fits[0])
            m_jacobi[jpx][runfit.mw_ndxs[0]] =
                constx * buoy0 * ufunc0 + constx * buoy0 * ufunc2 * stoich1;

          if (runfit.mw_fits[1])
            m_jacobi[jpx][runfit.mw_ndxs[1]] = constx * buoy1 * ufunc1;

          if (runfit.vbar_fits[0])
            m_jacobi[jpx][runfit.vbar_ndxs[0]] =
                (-1.0) * constx * mwv0 * density * ufunc0 -
                constx * mwv0 * density * ufunc2;

          if (scnf->amp_fits[0])
            m_jacobi[jpx][scnf->amp_ndxs[0]] = ufunc0 + ufunc2 * stoich1;

          if (scnf->amp_fits[1]) m_jacobi[jpx][scnf->amp_ndxs[1]] = ufunc1;

          if (runfit.eq_fits[0]) m_jacobi[jpx][runfit.eq_ndxs[0]] = ufunc2;

          if (scnf->baseln_fit) m_jacobi[jpx][scnf->baseln_ndx] = 1.0;

          jpx++;
        }

        jdx++;
      }
      break;
    }
  }

  return stat;
}

// Calculate the chi-squared for the fixed molecular weight estimate
// for a single component model
double US_EqMath::calc_testParameter(double mwval) {
  int points = 0;
  double chi_sq = 0.0;
  double x_temp = 0.0;
  double dconst;
  double buoyancy_tb;
  double omega_s;
  double x_val;
  double y_val;

  QVector<double*> mmat;   // M matrix (vector of pointers to arrays)
  QVector<double> mvec;    // Concatenated matrix doubles
  QVector<double> yrvec;   // Raw Y-values vector
  QVector<double> covec;   // Coefficients vector
  QVector<QPointF> xyvec;  // X,Y vector (of points)

  // Determine maximum number of points for vectors
  for (int jes = 0; jes < scanfits.size(); jes++)
    points = max(points, scanfits[jes].xvs.size());

  DbgLv(1) << "ctPar: max points" << points;
  mmat.fill(NULL, points);     // Initialize matrix (array pointers)
  mvec.fill(0.0, points * 2);  // Initialize vector of matrix elements
  yrvec.fill(0.0, points);     // Initialize vector of raw Y's
  covec.fill(0.0, 2);
  xyvec.fill(QPointF(0.0, 0.0), points);

  for (int jes = 0; jes < scanfits.size();
       jes++) {  // Build up chi-squared for fitted scans
    EqScanFit* scnf = &scanfits[jes];

    if (!scnf->scanFit) continue;  // Ignore any scan that is not fitted

    int strtx = scnf->start_ndx;  // Get scan parameters
    int stopx = scnf->stop_ndx;
    int stopn = stopx + 1;
    double fstemp = scnf->tempera;
    double fsdens = scnf->density;
    double fsvisc = scnf->viscosity;
    double fsrpm = scnf->rpm;
    double fsvbar = runfit.vbar_vals[0];
    // DbgLv(1) << "ctPar: jes strtx stopx" << jes << strtx << stopx;

    US_Math2::SolutionData solution;  // Calculate constant based on
    solution.vbar = fsvbar;           //  buoyancy, based on vbar,density...
    solution.vbar20 = fsvbar;
    solution.density = fsdens;
    solution.viscosity = fsvisc;
    US_Math2::data_correction(fstemp, solution);
    buoyancy_tb = solution.buoyancyb;

    omega_s = sq(fsrpm * M_PI / 30.0);
    dconst = (buoyancy_tb * omega_s) / (2.0 * R_GC * (K0 + fstemp));
    x_temp = scnf->xvs[strtx];
    x_temp = sq(x_temp);
    // DbgLv(1) << "ctPar: xt x0 xN" << x_temp << scnf->xvs[strtx] <<
    // scnf->xvs[stopx];
    points = 0;

    for (int jxy = strtx; jxy < stopn;
         jxy++) {  // Accumulate the X,Y values within the scan's start,stop
                   // range
      x_val = scnf->xvs[jxy];
      y_val = scnf->yvs[jxy];
      x_val = dconst * (sq(x_val) - x_temp);
      xyvec[points++] = QPointF(x_val, y_val);
    }

    double** MM = mmat.data();      // Pointer to matrix data
    double* yraw = yrvec.data();    // Pointer to raw-Y data
    double* mvls = mvec.data();     // Pointer to matrix values
    double* coeffs = covec.data();  // Pointer to output coefficients
    int kk = 0;

    for (int ii = 0; ii < points;
         ii++) {              // Build matrix and vector needed for LS
      x_val = xyvec[ii].x();  // Raw X value
      y_val = xyvec[ii].y();  // Raw Y value

      // The following builds and stores MM[ii][0] and MM[ii][1]
      mmat[ii] = mvls + kk;  // Store pointer to 2-point array
      mvec[kk++] = 1.0;      // Update matrix column
      mvec[kk++] = exp(mwval * x_val);

      yrvec[ii] = y_val;  // Update Y-Raw vector
    }
    // DbgLv(1) << "ctPar:   yrvec[0]" << yrvec[0] << "points" << points;
    // DbgLv(1) << "ctPar:    mm00 mm01" << MM[0][0] << MM[0][1];
    // int N=points-1;
    // DbgLv(1) << "ctPar:    mmN0 mN01" << MM[N][0] << MM[N][1];
    // DbgLv(1) << "ctPar:    mwval x_val" << mwval << x_val;

    // Get coefficients thru general Least Squares, order=2
    genLeastSquaresOrd2(MM, points, yraw, &coeffs);

    for (int ii = 0; ii < points; ii++) {  // Update the chi-squared value
      double chi =
          yrvec[ii] - (coeffs[0] + coeffs[1] * exp(mwval * xyvec[ii].x()));
      chi_sq += sq(chi);
    }
    // DbgLv(1) << "ctPar:    chi_sq" << chi_sq;

    scnf->baseline = coeffs[0];
    scnf->baseln_rng = 0.05;

    if (coeffs[1] < dflt_min || isNan(coeffs[1])) {
      DbgLv(1) << "ctPar:     ** jes" << jes << "coeffs1 " << covec[1];
      scnf->amp_vals[0] = dflt_min;
    }

    else {
      scnf->amp_vals[0] = log(coeffs[1]);
    }

    scnf->amp_rngs[0] = scnf->amp_vals[0] * 0.2;
  }
  DbgLv(1) << "ctPar: coeffs 0 1 " << covec[0] << covec[1];
  DbgLv(1) << "ctPar:chi_sq" << chi_sq;

  return chi_sq;
}

// Find the minimum residual.
// Residual values are f0, f1, f2; evaluated at x0, x1, x2.
// x0, x1, x2 are multipliers for incremental change of the parameter.
// Calculate bracket: assume the minimum is between x0=0 and some stepsize
// x2 away from x0. Then find an x1 in the middle between x0 and x2; and
// calculate f1(x1).
double US_EqMath::linesearch() {
  double residm = 0.0;
  double old_f0 = 0.0;
  double old_f1 = 0.0;
  double old_f2 = 0.0;
  double x0 = 100.0;
  double x1 = 5000.0;
  double x2 = 10000.0;
  double hh = 0.01;
  double toler = 100.0;
  double errmx;
  double xmin;
  double fmin;
  int iter = 1;

  double f0 = calc_testParameter(x0);
  double f1 = calc_testParameter(x1);
  double f2 = calc_testParameter(x2);
  DbgLv(1) << "MinRes:iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
  DbgLv(1) << "MinRes: dflt_min dflt_max" << dflt_min << dflt_max;

  if (dataList[0].dataType == "RA")
    errmx = 1.0e4;  // absorbance
  else
    errmx = 1.0e12;  // larger value for interference

  while (f0 >= errmx || f0 < 0.0 || f1 >= errmx || f1 < 0.0 || f2 >= errmx ||
         f2 < 0.0) {  // Assure f1,f2 are between 0.0 and errmx
    x1 *= 0.5;
    x2 *= 0.5;

    f1 = calc_testParameter(x1);
    f2 = calc_testParameter(x2);

    if (x1 < dflt_min) return (-1.0);
  }
  DbgLv(1) << "  x1 x2" << x1 << x2 << " f1 f2" << f1 << f2 << "errmx" << errmx;

  bool check_flag = true;

  while (check_flag) {
    if ((isNan(f0) && isNan(f1)) || (isNan(f1) && isNan(f2)) ||
        (isNan(f0) && isNan(f1)))
      return (-1.0);  // At least two values screwed up

    if ((qAbs(f2 - old_f2) < dflt_min) && (qAbs(f1 - old_f1) < dflt_min) &&
        (qAbs(f0 - old_f0) < dflt_min))
      return (0.0);  // Solution converged horizontally

    old_f0 = f0;
    old_f1 = f1;
    old_f2 = f2;
    DbgLv(1) << "   old f0 f1 f2" << f0 << f1 << f2;

    DbgLv(1) << "     test-0a";
    bool t1 = (qAbs(f2 - f0) < dflt_min);
    bool t2 = (qAbs(f1 - f0) < dflt_min);
    bool t3 = (f0 > f1);
    bool t4 = (qAbs(f2 - f1) < dflt_min);
    bool t12 = t1 && t2;
    bool t34 = t3 && t4;
    DbgLv(1) << "     t-0a t1-4" << t1 << t2 << t3 << t4 << "t12,34" << t12
             << t34;
    if (((qAbs(f2 - f0) < dflt_min) && (qAbs(f1 - f0) < dflt_min)) ||
        ((f0 > f1) && (qAbs(f2 - f1) < dflt_min)))
      return (0.0);

    DbgLv(1) << "     test-0b";
    if ((qAbs(x0) < dflt_min) && (qAbs(x1) < dflt_min) && (qAbs(x2) < dflt_min))
      return (0.0);

    DbgLv(1) << "     test-0c";
    if (((qAbs(f0 - f1) < dflt_min) && (qAbs(f1 - f2) < dflt_min)) ||
        ((f2 > f1) && (qAbs(f0 - f1) < dflt_min)))
      return (0.0);

    DbgLv(1) << "     test-0 x0 x1 x2" << x0 << x1 << x2;
    if ((f0 > f1) && (f2 > f1))  // We have a bracket
    {
      DbgLv(1) << "     update-0 f0 f1 f2" << f0 << f1 << f2;
      check_flag = false;
      break;
    }

    if ((f2 > f1 && f1 > f0) || (f1 > f0 && f1 > f2) || (f1 == f2 && f1 > f0)) {
      x2 = x1;
      f2 = f1;
      x1 = (x2 + x1) * 0.5;
      f1 = calc_testParameter(x1);
      DbgLv(1) << "     update-1 f0 f1 f2" << f0 << f1 << f2;
    }

    else if ((f0 > f1) && (f1 > f2)) {
      x0 = x1;
      f0 = f1;
      x1 = x2;
      f1 = f2;
      x2 = x2 + ((pow(2.0, (double)(iter + 2))) + hh);
      f2 = calc_testParameter(x2);
      DbgLv(1) << "     update-2 f0 f1 f2" << f0 << f1 << f2;
    }

    iter++;
    DbgLv(1) << "MinRes:         iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
  }

  DbgLv(1) << "MinRes:  iter" << iter << " f0 f1 f2" << f0 << f1 << f2;
  x1 = (x0 + x2) * 0.5;
  hh = x1 - x0;
  f1 = calc_testParameter(x1);

  while (true) {
    if (f0 < f1)  // Shift left
    {
      x2 = x1;
      f2 = f1;
      x1 = x0;
      f1 = f0;
      x0 = x1 - hh;
      f0 = calc_testParameter(x0);
    }

    if (f2 < f1)  // Shift right
    {
      x0 = x1;
      f0 = f1;
      x1 = x2;
      f1 = f2;
      x2 = x1 + hh;
      f2 = calc_testParameter(x2);
    }

    if (qAbs(f0 - (f1 * 2.0) + f2) < dflt_min) {
      residm = 0.0;
      break;
    }

    xmin = x1 + (hh * (f0 - f2)) / (2.0 * (f0 - f1 * 2.0 + f2));
    fmin = calc_testParameter(xmin);

    if (fmin < f1) {
      x1 = xmin;
      f1 = fmin;
    }

    hh = hh * 0.5;

    if (hh < toler) {
      residm = x1;  // Shouldn't it be "f" we return?
      // residm = f1;
      break;
    }

    x0 = x1 - hh;
    x2 = x1 + hh;
    f0 = calc_testParameter(x0);
    f2 = calc_testParameter(x2);
    DbgLv(1) << "     update-9 x0 x2 f0 f2" << x0 << x2 << f0 << f2;
  }

  return residm;
}

// Calculate the B matrix  ( = Jacobian-tranpose * ydelta )
void US_EqMath::calc_B() {
  // Calculate B = J' * d
  US_Matrix::tvv(jacobian, y_delta, BB, ntpts, nfpars);
}

// Calculate the delta array and the variance value
double US_EqMath::calc_residuals() {
  double residual = 0.0;

  for (int ii = 0; ii < ntpts; ii++) {
    double delta = y_raw[ii] - y_guess[ii];

    y_delta[ii] = delta;
    residual += sq(delta);
  }

  if (residual > dflt_max)
    residual = -1.0;

  else
    residual /= (double)ntpts;

  if (residual < 0.0)
    qDebug() << "CalcResid: resid" << residual << "delta 0,1,m,n" << y_delta[0]
             << y_delta[1] << y_delta[ntpts - 2] << y_delta[ntpts - 1];

  return residual;
}

// Calculate the model from a set of guesses
int US_EqMath::calc_model(double* guess) {
  int stat = 0;

  parameter_mapBackward(guess);

  int ncomps = runfit.nbr_comps;
  int jpx = 0;  // Total points count
  int jlx = 0;  // Log count
  int jdx = 0;  // Datasets count
  int mcomp = max(ncomps, 4);
  QVector<double> v_vbar(mcomp);
  QVector<double> v_buoy(mcomp);

  v_setlpts.clear();

  switch (modelx) {
    case 0:  //  0: "1-Component, Ideal"
    case 1:  //  1: "2-Component, Ideal, Noninteracting"
    case 2:  //  2: "3-Component, Ideal, Noninteracting"
    case 3:  //  3: "Fixed Molecular Weight Distribution"
      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        for (int kk = 0; kk < ncomps; kk++) {
          v_vbar[kk] = US_Math2::adjust_vbar20(runfit.vbar_vals[kk], tempera);
          v_buoy[kk] = (1.0 - v_vbar[kk] * density);
        }

        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double cguess = 0.0;

          for (int kk = 0; kk < ncomps; kk++) {
            double buoy = v_buoy[kk];
            double mwv = runfit.mw_vals[kk];
            double ampv = scnf->amp_vals[kk];
            double ufunc = exp(ampv + dconst * mwv * buoy * xv);
            cguess += ufunc;
          }

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) {
            lncr2[jdx][jlx++] = log(cguess);
          }

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    case 4:   //  4: "Monomer-Dimer Equilibrium"
    case 5:   //  5: "Monomer-Trimer Equilibrium"
    case 6:   //  6: "Monomer-Tetramer Equilibrium"
    case 7:   //  7: "Monomer-Pentamer Equilibrium"
    case 8:   //  8: "Monomer-Hexamer Equilibrium"
    case 9:   //  9: "Monomer-Heptamer Equilibrium"
    case 10:  // 10: "User-Defined Monomer-Nmer Equilibrium"
    {
      double stoich1 = (double)(modelx - 2);
      double stoiexp = stoich1 - 1.0;
      runfit.stoichs[0] = stoich1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double ODcorrec =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy = (1.0 - v_vbar[0] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double mwv = runfit.mw_vals[0];
          double ampv = scnf->amp_vals[0];
          double ufunc0 = exp(ampv + dconst * mwv * buoy * xv);
          double ufunc1 = exp(stoich1 * ampv + runfit.eq_vals[0] + ODcorrec +
                              dconst * stoich1 * mwv * buoy * ufunc0);
          double cguess = ufunc0 + ufunc1;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) {
            lncr2[jdx][jlx++] = log(cguess);
          }

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 11:  // 11: "Monomer-Dimer-Trimer Equilibrium"
    case 12:  // 12: "Monomer-Dimer-Tetramer Equilibrium"
    case 13:  // 13: "User-Defined Monomer - N-mer - M-mer Equilibrium"
    {
      double stoich1 = 2.0;
      double stoich2 = (double)(modelx - 8);
      runfit.stoichs[0] = stoich1;
      runfit.stoichs[1] = stoich2;
      double stoiexp = stoich1 - 1.0;
      double stoiex2 = stoich2 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double ODcorec1 =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp));
        double ODcorec2 =
            log(stoich2 / pow(scnf->extincts[0] * scnf->pathlen, stoiex2));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy = (1.0 - v_vbar[0] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double mwv = runfit.mw_vals[0];
          double ampv = scnf->amp_vals[0];
          double ufunc0 = exp(ampv + dconst * mwv * buoy * xv);
          double ufunc1 = exp(stoich1 * ampv + runfit.eq_vals[0] + ODcorec1 +
                              dconst * stoich1 * mwv * buoy * xv);
          double ufunc2 = exp(stoich2 * ampv + runfit.eq_vals[1] + ODcorec2 +
                              dconst * stoich2 * mwv * buoy * xv);
          double cguess = ufunc0 + ufunc1 + ufunc2;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) {
            lncr2[jdx][jlx++] = log(cguess);
          }

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 14:  // 14: "2-Component Hetero-Association: A + B <=> AB"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double mw_ab = mwv0 + mwv1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double extinc1 = scnf->extincts[1];
        double ODcorrec =
            log((extinc0 + extinc1) / (scnf->pathlen * extinc0 * extinc1));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        v_vbar[2] = (v_vbar[0] * mwv0 + v_vbar[1] * mwv1) / mw_ab;
        double buoy0 = (1.0 - v_vbar[0] * density);
        double buoy1 = (1.0 - v_vbar[1] * density);
        double buoy2 = (1.0 - v_vbar[2] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(ampv0 + ampv1 + runfit.eq_vals[0] + ODcorrec +
                              constx * mw_ab * buoy2);
          double cguess = ufunc0 + ufunc1 + ufunc2;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) {
            lncr2[jdx][jlx++] = log(cguess);
          }

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 15:  // 15: "U-Defined self/Hetero-Assoc.: A + B <=> AB, nA <=> An"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double mw_ab = mwv0 + mwv1;
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double extinc1 = scnf->extincts[1];
        double ODcorec1 =
            log((extinc0 + extinc0) / (scnf->pathlen * extinc0 * extinc1));
        double ODcorec2 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        v_vbar[2] = (v_vbar[0] * mwv0 + v_vbar[1] * mwv1) / mw_ab;
        double buoy0 = (1.0 - v_vbar[0] * density);
        double buoy1 = (1.0 - v_vbar[1] * density);
        double buoy2 = (1.0 - v_vbar[2] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(ampv0 + ampv1 + runfit.eq_vals[0] + ODcorec1 +
                              constx * mw_ab * buoy2);
          double ufunc3 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec2 +
                              constx * stoich1 * mw_ab * buoy0);
          double cguess = ufunc0 + ufunc1 + ufunc2 + ufunc3;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) lncr2[jdx][jlx++] = log(cguess);

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 16:  // 16: "U-Defined Monomer-Nmer, some monomer is incompetent"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(extinc0 * scnf->pathlen, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy0 = (1.0 - v_vbar[0] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          double cguess = ufunc0 + ufunc1 + ufunc2;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) lncr2[jdx][jlx++] = log(cguess);

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 17:  // 17: "User-Defined Monomer-Nmer, some Nmer is incompetent"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy0 = (1.0 - v_vbar[0] * density);
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double ampv0 = scnf->amp_vals[0];
          double ampv1 = scnf->amp_vals[1];
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          double cguess = ufunc0 + ufunc1 + ufunc2;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) lncr2[jdx][jlx++] = log(cguess);

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 18:  // 18: "User-Defined irreversible Monomer-Nmer"
    {
      double mwv0 = runfit.mw_vals[0];
      double stoich1 = runfit.stoichs[0];
      double mwv1 = mwv0 * stoich1;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy0 = (1.0 - v_vbar[0] * density);
        double ampv0 = scnf->amp_vals[0];
        double ampv1 = scnf->amp_vals[1];
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy0);
          double cguess = ufunc0 + ufunc1;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) lncr2[jdx][jlx++] = log(cguess);

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
    case 19:  // 19: "User-Defined Monomer-Nmer plus contaminant"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double stoich1 = runfit.stoichs[0];
      double stoiexp = stoich1 - 1.0;

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        int jstx = scnf->start_ndx;
        double xm_sq = sq(scnf->xvs[jstx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));
        double extinc0 = scnf->extincts[0];
        double ODcorec1 = log(stoich1 / pow(scnf->pathlen * extinc0, stoiexp));
        v_vbar[0] = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        v_vbar[1] = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        double buoy0 = (1.0 - v_vbar[0] * density);
        double buoy1 = (1.0 - v_vbar[1] * density);
        double ampv0 = scnf->amp_vals[0];
        double ampv1 = scnf->amp_vals[1];
        jlx = 0;

        for (int jj = jstx; jj < jstx + v_setpts[jdx]; jj++) {
          double xv = sq(scnf->xvs[jj]) - xm_sq;
          double constx = dconst * xv;
          double ufunc0 = exp(ampv0 + constx * mwv0 * buoy0);
          double ufunc1 = exp(ampv1 + constx * mwv1 * buoy1);
          double ufunc2 = exp(stoich1 * ampv0 + runfit.eq_vals[0] + ODcorec1 +
                              constx * stoich1 * mwv0 * buoy0);
          double cguess = ufunc0 + ufunc1 + ufunc2;

          y_guess[jpx] = cguess + scnf->baseline;

          if (cguess > 0.0) lncr2[jdx][jlx++] = log(cguess);

          jpx++;
        }

        v_setlpts << jlx;
        jdx++;
      }
      break;
    }
  }
  return stat;
}

// Solve a general least squares (order=2)
void US_EqMath::genLeastSquaresOrd2(double** MM, int points, double* y_raw,
                                    double** coeff) {
  double* aa[2];        // 2 x 2 matrix
  double a0[2];         // matrix column 1
  double a1[2];         // matrix column 2
  double bb[2];         // 2-point vector
  aa[0] = (double*)a0;  // finish matrix creation
  aa[1] = (double*)a1;
  a0[0] = 0.0;  // initialize work matrix
  a0[1] = 0.0;
  a1[0] = 0.0;
  a1[1] = 0.0;

  for (int ii = 0; ii < 2;
       ii++) {  // Fill in the lower triangle of the 2 x 2 "A" matrix
    for (int jj = 0; jj <= ii; jj++) {
      double dotp = 0.0;

      for (int kk = 0; kk < points; kk++) dotp += (MM[kk][ii] * MM[kk][jj]);

      aa[ii][jj] = dotp;
    }
  }

  for (int jj = 0; jj < 2; jj++) {  // Fill in the 2 values of the "B" vector
    double dotp = 0.0;

    for (int kk = 0; kk < points; kk++) dotp += (y_raw[kk] * MM[kk][jj]);

    bb[jj] = dotp;
  }

  // Do 2nd-order Cholesky decomposition and solve system
  Cholesky_DecompOrd2((double**)aa);
  Cholesky_SolveSysOrd2((double**)aa, bb);

  // Result is the coefficients vector
  (*coeff)[0] = bb[0];
  (*coeff)[1] = bb[1];
  // DbgLv(1) << "LS2: CSS b0 b1" << bb[0] << bb[1];
}

// Cholesky Decomposition for order=2
bool US_EqMath::Cholesky_DecompOrd2(double** aa) {
  DbgLv(1) << "LS2:CDec a00 a10 a11" << aa[0][0] << aa[1][0] << aa[1][1];
  aa[0][0] = sqrt(aa[0][0]);
  aa[1][0] = aa[1][0] / aa[0][0];

  double sum = sq(aa[1][0]);
  double diff = aa[1][1] - sum;
  DbgLv(1) << "LS2:CDec  sum diff" << sum << diff;

  if (diff <= 0.0) return false;

  aa[1][1] = sqrt(diff);
  aa[0][1] = 0.0;
  DbgLv(1) << "LS2:CDec   a00 a10 a11" << aa[0][0] << aa[1][0] << aa[1][1];

  return true;
}

// Cholesky Solve System for order=2
bool US_EqMath::Cholesky_SolveSysOrd2(double** LL, double* bb) {
  // Forward substitution

  bb[0] = bb[0] / LL[0][0];
  bb[1] = bb[1] - LL[1][0] * bb[0];
  bb[1] = bb[1] / LL[1][1];

  // Backward substitution

  bb[1] = bb[1] / LL[1][1];  // AGAIN????
  bb[0] = bb[0] - LL[1][0] * bb[1];
  bb[0] = bb[0] / LL[0][0];

  return true;
}

// Convenience function:  is a double value NAN (Not A valid Number)?
bool US_EqMath::isNan(double value) {
  if (value != value)  // NAN, the one case where value!=value
    return true;

  double avalue = qAbs(value);  // Also mark NAN if beyond float min,max

  if (avalue < dflt_min || avalue > dflt_max) return true;

  return false;
}

// Calculate runs counts
void US_EqMath::calc_runs() {
  // Accumulate runs and +/- counts for scans
  runfit.nbr_runs = 0;
  int jptx = 0;

  for (int ii = 0; ii < scanfits.size(); ii++) {
    EqScanFit* scnf = &scanfits[ii];

    if (!scnf->scanFit) continue;

    scnf->nbr_posr = 0;
    scnf->nbr_negr = 0;
    scnf->runs = 0;
    bool lastpos = (y_guess[jptx] > scnf->xvs[scnf->start_ndx]);

    for (int jj = scnf->start_ndx; jj < scnf->stop_ndx + 1; jj++) {
      bool thispos = (y_guess[jptx++] > scnf->yvs[jj]);

      if (thispos) {
        scnf->nbr_posr++;
        if (!lastpos) scnf->runs++;
      }

      else {
        scnf->nbr_negr++;
        if (lastpos) scnf->runs++;
      }

      lastpos = thispos;
    }

    runfit.nbr_runs += scnf->runs;
  }
}

// Calculate the integral from meniscus to bottom for each exponential term
void US_EqMath::calc_integral() {
  int ncomps = runfit.nbr_comps;

  switch (modelx) {
    case 0:  //  0: "1-Component, Ideal"
    case 1:  //  1: "2-Component, Ideal, Noninteracting"
    case 2:  //  2: "3-Component, Ideal, Noninteracting"
      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        double bottom = calc_bottom(scnf->rpm);
        scnf->bottom = bottom;
        double deltr = (bottom - scnf->meniscus) / 5000.0;
        double xm_sq = sq(scnf->xvs[scnf->start_ndx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        for (int kk = 0; kk < ncomps; kk++) {
          double vbar = US_Math2::adjust_vbar20(runfit.vbar_vals[kk], tempera);
          double buoy = (1.0 - vbar * density);
          double xval = scnf->meniscus;
          double amplv = scnf->amp_vals[kk];
          double ampl2 = exp(amplv);
          double mwfac = runfit.mw_vals[kk] * dconst * buoy;
          double sumi = 0.0;

          for (int mm = 0; mm < 4999; mm++) {
            xval += deltr;
            double xvsq = sq(xval) - xm_sq;
            double ampl1 = ampl2;
            ampl2 = exp(amplv + mwfac * xvsq);
            sumi += (deltr * (ampl1 + ampl2) / 2.0);
          }

          scnf->integral[kk] = sumi;
        }
      }
      break;
    case 3:  //  3: "Fixed Molecular Weight Distribution"
      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        double bottom = calc_bottom(scnf->rpm);
        scnf->bottom = bottom;
        double deltr = (bottom - scnf->meniscus) / 5000.0;
        double xm_sq = sq(scnf->xvs[scnf->start_ndx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        for (int kk = 0; kk < ncomps; kk++) {
          double vbar = US_Math2::adjust_vbar20(runfit.vbar_vals[kk], tempera);
          double buoy = (1.0 - vbar * density);
          double xval = scnf->meniscus;
          double amplv = scnf->amp_vals[kk];
          double ampl2 = exp(amplv);
          double mwfac = runfit.mw_vals[kk] * dconst * buoy;
          double sumi = 0.0;

          for (int mm = 0; mm < 4999; mm++) {
            xval += deltr;
            double xvsq = sq(xval) - xm_sq;
            double ampl1 = ampl2;
            ampl2 = exp(amplv + mwfac * xvsq);
            sumi += (deltr * (ampl1 + ampl2) / 2.0);
          }

          scnf->integral[kk] = sumi;
        }
      }
      break;
    case 4:   //  4: "Monomer-Dimer Equilibrium"
    case 5:   //  5: "Monomer-Trimer Equilibrium"
    case 6:   //  6: "Monomer-Tetramer Equilibrium"
    case 7:   //  7: "Monomer-Pentamer Equilibrium"
    case 8:   //  8: "Monomer-Hexamer Equilibrium"
    case 9:   //  9: "Monomer-Heptamer Equilibrium"
    case 10:  // 10: "User-Defined Monomer-Nmer Equilibrium"
    {
      double stoich1 = (double)(modelx - 2);
      double stoiexp = stoich1 - 1.0;
      runfit.stoichs[0] = stoich1;
      double eqval0 = runfit.eq_vals[0];

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        double bottom = calc_bottom(scnf->rpm);
        scnf->bottom = bottom;
        double deltr = (bottom - scnf->meniscus) / 5000.0;
        double xm_sq = sq(scnf->xvs[scnf->start_ndx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double ODcorrec =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp)) +
            eqval0;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        double vbar = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy = (1.0 - vbar * density);
        double xval = scnf->meniscus;
        double amplv = scnf->amp_vals[0];
        double ampl2 = exp(amplv);
        double mwfac = runfit.mw_vals[0] * dconst * buoy;
        double sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[0] = sumi;

        xval = scnf->meniscus;
        sumi = 0.0;
        amplv = scnf->amp_vals[0] * stoich1;
        ampl2 = exp(amplv + ODcorrec);
        mwfac *= stoich1;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + ODcorrec + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[1] = sumi;
      }
      break;
    }
    case 11:  // 11: "Monomer-Dimer-Trimer Equilibrium"
    case 12:  // 12: "Monomer-Dimer-Tetramer Equilibrium"
    case 13:  // 13: "User-Defined Monomer - N-mer - M-mer Equilibrium"
    {
      double stoich1 = 2.0;
      double stoich2 = (double)(modelx - 8);
      runfit.stoichs[0] = stoich1;
      runfit.stoichs[1] = stoich2;
      // double stoiexp      = stoich1 - 1.0;
      // double stoiex2      = stoich2 - 1.0;
      double stoiexp = stoich1;
      double stoiex2 = stoich2;
      double eqval0 = runfit.eq_vals[0];
      double eqval1 = runfit.eq_vals[1];

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        double bottom = calc_bottom(scnf->rpm);
        scnf->bottom = bottom;
        double deltr = (bottom - scnf->meniscus) / 5000.0;
        double xm_sq = sq(scnf->xvs[scnf->start_ndx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double ODcorre1 =
            log(stoich1 / pow(scnf->extincts[0] * scnf->pathlen, stoiexp)) +
            eqval0;
        double ODcorre2 =
            log(stoich2 / pow(scnf->extincts[0] * scnf->pathlen, stoiex2)) +
            eqval1;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        double vbar = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double buoy = (1.0 - vbar * density);
        double xval = scnf->meniscus;
        double amplv = scnf->amp_vals[0];
        double ampl2 = exp(amplv);
        double mwfac = runfit.mw_vals[0] * dconst * buoy;
        double sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[0] = sumi;

        xval = scnf->meniscus;
        amplv = scnf->amp_vals[0] * stoich1;
        ampl2 = exp(amplv + ODcorre1);
        mwfac = runfit.mw_vals[0] * dconst * buoy * stoich1;
        sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + ODcorre1 + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[1] = sumi;

        xval = scnf->meniscus;
        amplv = scnf->amp_vals[0] * stoich2;
        ampl2 = exp(amplv + ODcorre2);
        mwfac = runfit.mw_vals[0] * dconst * buoy * stoich2;
        sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + ODcorre1 + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[2] = sumi;
      }
      break;
    }
    case 14:  // 14: "2-Component Hetero-Association: A + B <=> AB"
    {
      double mwv0 = runfit.mw_vals[0];
      double mwv1 = runfit.mw_vals[1];
      double mwv2 = runfit.mw_vals[2];
      double mw_ab = mwv0 + mwv2;
      double eqval0 = runfit.eq_vals[0];

      for (int ii = 0; ii < scanfits.size(); ii++) {
        EqScanFit* scnf = &scanfits[ii];

        if (!scnf->scanFit) continue;

        double bottom = calc_bottom(scnf->rpm);
        scnf->bottom = bottom;
        double deltr = (bottom - scnf->meniscus) / 5000.0;
        double xm_sq = sq(scnf->xvs[scnf->start_ndx]);
        double omega_sq = sq(M_PI * scnf->rpm / 30.0);
        double tempera = scnf->tempera;
        double density = scnf->density;
        double dconst = omega_sq / (2.0 * R_GC * (K0 + tempera));

        double vbar0 = US_Math2::adjust_vbar20(runfit.vbar_vals[0], tempera);
        double vbar1 = US_Math2::adjust_vbar20(runfit.vbar_vals[1], tempera);
        double vbar2 = (vbar0 * mwv0 + vbar1 * mwv1) / mw_ab;
        double buoy0 = (1.0 - vbar0 * density);
        double buoy1 = (1.0 - vbar1 * density);
        double buoy2 = (1.0 - vbar2 * density);

        double xval = scnf->meniscus;
        double amplv = scnf->amp_vals[0];
        double ampl2 = exp(amplv);
        double mwfac = mwv0 * dconst * buoy0;
        double sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[0] = sumi;

        xval = scnf->meniscus;
        amplv = scnf->amp_vals[1];
        ampl2 = exp(amplv);
        mwfac = mwv1 * dconst * buoy1;
        sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[1] = sumi;

        double extinc0 = scnf->extincts[0];
        double extinc1 = scnf->extincts[1];
        double extin_ab = extinc0 + extinc1;
        double ODcorrec =
            log(extin_ab / (extinc0 * extinc1 * scnf->pathlen)) + eqval0;
        xval = scnf->meniscus;
        amplv = scnf->amp_vals[0] + scnf->amp_vals[1];
        ampl2 = exp(amplv + ODcorrec);
        mwfac = mw_ab * dconst * buoy2;
        sumi = 0.0;

        for (int mm = 0; mm < 4999; mm++) {
          xval += deltr;
          double xvsq = sq(xval) - xm_sq;
          double ampl1 = ampl2;
          ampl2 = exp(amplv + ODcorrec + mwfac * xvsq);
          sumi += (deltr * (ampl1 + ampl2) / 2.0);
        }

        scnf->integral[2] = sumi;
      }
      break;
    }
    case 15:  // 15: "U-Defined self/Hetero-Assoc.: A + B <=> AB, nA <=> An"
    case 16:  // 16: "U-Defined Monomer-Nmer, some monomer is incompetent"
    case 17:  // 17: "User-Defined Monomer-Nmer, some Nmer is incompetent"
    case 18:  // 18: "User-Defined irreversible Monomer-Nmer"
    case 19:  // 19: "User-Defined Monomer-Nmer plus contaminant"
      break;
  }
}

// Calculate the bottom for a scan, using rotor information and speed
double US_EqMath::calc_bottom(double rpm) {
  return (runfit.bottom_pos + runfit.rcoeffs[0] * rpm +
          runfit.rcoeffs[1] * sq(rpm));
}
