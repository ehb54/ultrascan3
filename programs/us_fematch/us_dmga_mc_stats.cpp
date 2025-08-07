//! \file us_dmga_mc_stats.cpp

#include "us_dmga_mc_stats.h"

#include "us_math2.h"

// Class of static functions for DMGA-MC statistics

// Build a vector of iteration models from a DMGA-MC model
int US_DmgaMcStats::build_imodels(US_Model& model, QVector<US_Model>& imodels) {
  imodels.clear();
  QStringList xmls;
  int niters = 0;
  int ncomp = 0;
  int nasso = 0;
  int kcomp = 0;
  int kasso = 0;
  int nxmls = model.mc_iter_xmls(xmls);

  for (int ii = 0; ii < nxmls; ii++) {  // Build the individual iteration models
    QString mxml = xmls[ii];
    int jj = mxml.indexOf("description=");
    QString mdesc = QString(mxml).mid(jj, 100).section('"', 1, 1);

    US_Model imodel;
    imodel.load_string(mxml);
    kcomp = imodel.components.size();
    kasso = imodel.associations.size();
    ;
    qDebug() << "DmS:AdjMd:  ii" << ii << "xml.desc" << mdesc;
    qDebug() << "DmS:AdjMd:     ncomp" << kcomp << "nassoc" << kasso;

    if (ii == 0) {
      ncomp = kcomp;
      nasso = kasso;
    }

    if (kcomp == ncomp || kasso == nasso) {
      niters++;
      imodels << imodel;
    } else
      qDebug() << "DmS:AdjMd: ***ii=" << ii << "kcomp" << kcomp << "kassoc"
               << kasso;
  }

  return niters;
}

// Build RMSD statistics from iteration models
void US_DmgaMcStats::build_rmsd_stats(int niters, QVector<US_Model>& imodels,
                                      QVector<double>& rstats) {
  const int stsiz = 23;
  QVector<double> rmsds;
  QVector<double> rconcs;

  rstats.fill(0.0, stsiz);

  // Get statistics for RMSDs
  for (int jj = 0; jj < niters; jj++) {
    double rmsd = sqrt(imodels[jj].variance);
    rmsds << rmsd;
    rconcs << 1.0;
  }

  compute_statistics(niters, rmsds, rconcs, rstats);
  qDebug() << "Dst:BRs:   iters" << niters << "RMSD min max mean median"
           << rstats[0] << rstats[1] << rstats[2] << rstats[3];
}

// Build Model attribute statistics from iteration models
int US_DmgaMcStats::build_model_stats(int niters, QVector<US_Model>& imodels,
                                      QVector<QVector<double> >& astats) {
  QVector<double> concs;
  QVector<double> vbars;
  QVector<double> mwts;
  QVector<double> scos;
  QVector<double> dcos;
  QVector<double> ff0s;
  QVector<double> kds;
  QVector<double> koffs;
  const int ncatt = 6;
  const int naatt = 2;
  int ncomp = imodels[0].components.size();
  int nasso = imodels[0].associations.size();
  int ntatts = ncomp * ncatt + nasso * naatt;
  astats.resize(ntatts);

  int kt = 0;
  qDebug() << "DmS:BMs:  ncomp" << ncomp << "nasso" << nasso << "ntatts"
           << ntatts << "niters" << niters;

  // Get statistics for each component
  for (int ii = 0; ii < ncomp; ii++) {
    concs.fill(0.0, niters);
    vbars.fill(0.0, niters);
    mwts.fill(0.0, niters);
    scos.fill(0.0, niters);
    dcos.fill(0.0, niters);
    ff0s.fill(0.0, niters);
    int i0 = qMax(0, ii - 1);

    for (int jj = 0; jj < niters; jj++) {
      US_Model::SimulationComponent* sc = &imodels[jj].components[ii];
      US_Model::SimulationComponent* s0 = &imodels[jj].components[i0];
      concs[jj] = sc->signal_concentration > 0.0 ? sc->signal_concentration
                                                 : s0->signal_concentration;
      vbars[jj] = sc->vbar20;
      mwts[jj] = sc->mw;
      scos[jj] = sc->s;
      dcos[jj] = sc->D;
      ff0s[jj] = sc->f_f0;
    }

    compute_statistics(niters, concs, concs, astats[kt++]);
    compute_statistics(niters, vbars, concs, astats[kt++]);
    compute_statistics(niters, mwts, concs, astats[kt++]);
    compute_statistics(niters, scos, concs, astats[kt++]);
    compute_statistics(niters, dcos, concs, astats[kt++]);
    compute_statistics(niters, ff0s, concs, astats[kt++]);
    qDebug() << "DmS:BMs:   ii" << ii << "mean c v w s d k" << astats[kt - 6][2]
             << astats[kt - 5][2] << astats[kt - 4][2] << astats[kt - 3][2]
             << astats[kt - 2][2] << astats[kt - 1][2];
  }

  // Get statistics for reactions
  for (int ii = 0; ii < nasso; ii++) {
    US_Model::Association* as = &imodels[0].associations[ii];
    int nrcs = as->rcomps.size();
    int rc1 = as->rcomps[0];
    int rc2 = as->rcomps[1];
    // int stoi1       = as->stoichs[ 0 ];
    // int stoi2       = as->stoichs[ 1 ];
    // int stoi3       = ( nrcs == 2 ) ? stoi2 : as->stoichs[ 2 ];
    concs.fill(0.0, niters);
    kds.fill(0.0, niters);
    koffs.fill(0.0, niters);

    if (nrcs == 2) {  // Single reactant and a product
      for (int jj = 0; jj < niters; jj++) {
        US_Model::Association* as = &imodels[jj].associations[ii];
        US_Model::SimulationComponent* sc = &imodels[jj].components[rc1];
        concs[jj] = sc->signal_concentration;
        kds[jj] = as->k_d;
        koffs[jj] = as->k_off;
      }

      compute_statistics(niters, kds, concs, astats[kt++]);
      compute_statistics(niters, koffs, concs, astats[kt++]);
    }

    else {  // Two reactants and a product (sum concentrations)
      for (int jj = 0; jj < niters; jj++) {
        US_Model::Association* as = &imodels[jj].associations[ii];
        US_Model::SimulationComponent* sc = &imodels[jj].components[rc1];
        US_Model::SimulationComponent* c2 = &imodels[jj].components[rc2];
        concs[jj] = sc->signal_concentration + c2->signal_concentration;
        kds[jj] = as->k_d;
        koffs[jj] = as->k_off;
      }

      compute_statistics(niters, kds, concs, astats[kt++]);
      compute_statistics(niters, koffs, concs, astats[kt++]);
      qDebug() << "DmS:BMs:   ii" << ii << "mean kd ko" << astats[kt - 2][2]
               << astats[kt - 1][2];
    }
  }

  qDebug() << "DmS:BMs: RETURN w ntatts" << ntatts;
  return ntatts;
}

// Compute the statistical values for a vector of values
bool US_DmgaMcStats::compute_statistics(const int nvals, QVector<double>& vals,
                                        QVector<double>& concs,
                                        QVector<double>& stats) {
  bool is_fixed = false;
  const int stsiz = 23;
  int nbins = 50;
  double vsiz = (double)nvals;
  double binsz = 50.0;
  double vlo = 9.9e30;
  double vhi = -9.9e30;

  QVector<double> xpvec(qMax(nvals, nbins));
  QVector<double> ypvec(qMax(nvals, nbins));

  double* xplot = xpvec.data();
  double* yplot = ypvec.data();
  double vsum = 0.0;
  double vm2 = 0.0;
  double vm3 = 0.0;
  double vm4 = 0.0;
  double vmean, vmedi;
  double mode_cen, mode_lo, mode_hi;
  double conf99lo, conf99hi, conf95lo, conf95hi;
  double clim99lo, clim99hi, clim95lo, clim95hi;
  double skew, kurto, slope, vicep, sigma;
  double corr, sdevi, sderr, vari, area;
  double bininc, val, conc;
  double vctot = 0.0;

  stats.resize(stsiz);

  // Get basic min,max,mean information

  for (int jj = 0; jj < nvals; jj++) {
    val = vals.at(jj);
    conc = concs.at(jj);
    vsum += (val * conc);
    vctot += conc;
    vlo = qMin(vlo, val);
    vhi = qMax(vhi, val);
    xplot[jj] = (double)jj;
    yplot[jj] = val;
  }

  vmean = vsum / vctot;
  is_fixed = (vlo == vhi);

  if (is_fixed) {  // Values are the same, special statistics for fixed
                   // attribute
    vmedi = vmean;
    qDebug() << "DmS:cSt:     FIXED  medi mean" << vmedi;
    mode_cen = vmean;
    skew = 0.0;
    kurto = 0.0;
    mode_lo = 0.0;
    mode_hi = 0.0;
    conf95lo = vmean;
    conf95hi = vmean;
    conf99lo = vmean;
    conf99hi = vmean;
    clim95lo = vmean;
    clim95hi = vmean;
    clim99lo = vmean;
    clim99hi = vmean;
    vari = 0.0;
    corr = 0.0;
    sdevi = 0.0;
    sderr = 0.0;
    area = 0.0;
  }

  else {  // Values are not the same, so statistics need to be computed

    for (int jj = 0; jj < nvals; jj++) {  // Get difference information
      val = vals.at(jj);
      double dif = val - vmean;
      double dsq = dif * dif;
      vm2 += dsq;          // diff squared
      vm3 += (dsq * dif);  // cubed
      vm4 += (dsq * dsq);  // to the 4th
    }

    vm2 /= vsiz;
    vm3 /= vsiz;
    vm4 /= vsiz;
    skew = vm3 / pow(vm2, 1.5);
    kurto = vm4 / pow(vm2, 2.0) - 3.0;

    // Do line fit (mainly for corr value)
    US_Math2::linefit(&xplot, &yplot, &slope, &vicep, &sigma, &corr, nvals);

    // Sort Y values and determine median
    QVector<double> wkvls = vals;
    qSort(wkvls);
    int hx = nvals / 2;
    vmedi = wkvls[hx];
    if ((hx * 2) == nvals) vmedi = (vmedi + wkvls[hx - 1]) * 0.5;
    qDebug() << "DmS:cSt:     FLT  hx vmedi vhx" << hx << vmedi << wkvls[hx];

    // Standard deviation and error
    sdevi = pow(vm2, 0.5);
    sderr = sdevi / pow(vsiz, 0.5);
    vari = vm2;
    area = 0.0;

    bininc = (vhi - vlo) / binsz;

    // Mode and confidence
    for (int ii = 0; ii < nbins; ii++) {
      xplot[ii] = vlo + bininc * (double)ii;
      yplot[ii] = 0.0;

      for (int jj = 0; jj < nvals; jj++) {
        val = vals.at(jj);

        if (val >= xplot[ii] && val < (xplot[ii] + bininc)) {
          yplot[ii] += (concs.at(jj));
        }
      }

      area += yplot[ii] * bininc;
    }

    // double fvdif     = qAbs( ( vhi - vlo ) / vlo );
    val = -1.0;
    int thisb = 0;

    for (int ii = 0; ii < nbins; ii++) {
      if (yplot[ii] > val) {
        val = yplot[ii];
        thisb = ii;
      }
    }

    mode_lo = xplot[thisb];
    mode_hi = mode_lo + bininc;
    mode_cen = (mode_lo + mode_hi) * 0.5;
    conf99lo = vmean - 2.576 * sdevi;
    conf99hi = vmean + 2.576 * sdevi;
    conf95lo = vmean - 1.960 * sdevi;
    conf95hi = vmean + 1.960 * sdevi;
    clim95lo = conf95hi - mode_cen;
    clim95hi = mode_cen - conf95lo;
    clim99lo = conf99hi - mode_cen;
    clim99hi = mode_cen - conf99lo;
  }

  stats[0] = vlo;        // Minimum
  stats[1] = vhi;        // Minimum
  stats[2] = vmean;      // Mean
  stats[3] = vmedi;      // Median
  stats[4] = skew;       // Skew
  stats[5] = kurto;      // Kurtosis
  stats[6] = mode_lo;    // Lower Mode
  stats[7] = mode_hi;    // Upper Mode
  stats[8] = mode_cen;   // Mode Center
  stats[9] = conf95lo;   // 95% Confidence Interval Low
  stats[10] = conf95hi;  // 95% Confidence Interval High
  stats[11] = conf99lo;  // 99% Confidence Interval Low
  stats[12] = conf99hi;  // 99% Confidence Interval High
  stats[13] = sdevi;     // Standard Deviation
  stats[14] = sderr;     // Standard Error
  stats[15] = vari;      // Variance
  stats[16] = corr;      // Correlation Coefficient
  stats[17] = binsz;     // Number of Bins
  stats[18] = area;      // Distribution Area
  stats[19] = clim95lo;  // 95% Confidence Limit Low
  stats[20] = clim95hi;  // 95% Confidence Limit High
  stats[21] = clim99lo;  // 99% Confidence Limit Low
  stats[22] = clim99hi;  // 99% Confidence Limit High

  return is_fixed;
}

// Build Model attribute statistics from iteration models
int US_DmgaMcStats::build_used_model(const QString smtype, const int iter,
                                     QVector<US_Model>& imodels,
                                     US_Model& umodel) {
  int stx = -iter;                       // Iteration index
  stx = (smtype == "mean") ? 2 : stx;    // Mean stats index
  stx = (smtype == "median") ? 3 : stx;  // Median stats index
  stx = (smtype == "mode") ? 8 : stx;    // Mode stats index
  qDebug() << "DmS:AdjMd:  stx" << stx;

  if (stx < 0) {  // Used model is one of the iterations
    umodel = imodels[iter - 1];
  }

  else {  // Used model uses mean|median|mode of each attribute
    umodel = imodels[0];
    int niters = imodels.size();
    QVector<QVector<double> > mstats;

    // Build statistics across iterations
    US_DmgaMcStats::build_model_stats(niters, imodels, mstats);

    int ncomp = umodel.components.size();
    int nasso = umodel.associations.size();
    int ks = 0;

    for (int ii = 0; ii < ncomp;
         ii++) {  // Pick up mean|median|mode of any floating attributes
      bool fixs = (mstats[ks + 3][0] == mstats[ks + 3][1]);
      bool fixd = (mstats[ks + 4][0] == mstats[ks + 4][1]);
      bool fixk = (mstats[ks + 5][0] == mstats[ks + 5][1]);
      double conc = mstats[ks++][stx];
      double vbar = mstats[ks++][stx];
      double mw = mstats[ks++][stx];
      double sedc = mstats[ks++][stx];
      double difc = mstats[ks++][stx];
      double ff0 = mstats[ks++][stx];
      conc = umodel.is_product(ii) ? 0.0 : conc;

      umodel.components[ii].signal_concentration = conc;
      umodel.components[ii].vbar20 = vbar;
      umodel.components[ii].mw = mw;
      umodel.components[ii].s = 0.0;
      umodel.components[ii].D = 0.0;
      umodel.components[ii].f_f0 = 0.0;
      umodel.components[ii].f = 0.0;

      if (fixk)
        umodel.components[ii].f_f0 = ff0;  // Set fixed f/f0
      else if (fixs)
        umodel.components[ii].s = sedc;  // Set fixed s
      else if (fixd)
        umodel.components[ii].D = difc;  // Set fixed D
      else
        umodel.components[ii].f_f0 = ff0;  // Set f/f0 if none fixed
      //*DEBUG*
      qDebug() << "DmS:UajMd: Cii=" << ii << "c v w s d k"
               << umodel.components[ii].signal_concentration
               << umodel.components[ii].vbar20 << umodel.components[ii].mw
               << umodel.components[ii].s << umodel.components[ii].D
               << umodel.components[ii].f_f0 << "fixs,d,k" << fixs << fixd
               << fixk;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:  Cii=" << ii << "c v w s d k"
               << umodel.components[ii].signal_concentration
               << umodel.components[ii].vbar20 << umodel.components[ii].mw
               << umodel.components[ii].s << umodel.components[ii].D
               << umodel.components[ii].f_f0;
      umodel.components[ii].D = 0.0;
      umodel.components[ii].f_f0 = 0.0;
      umodel.components[ii].f = 0.0;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:   fixS s d k" << umodel.components[ii].s
               << umodel.components[ii].D << umodel.components[ii].f_f0;
      umodel.components[ii].s = 0.0;
      umodel.components[ii].f_f0 = 0.0;
      umodel.components[ii].f = 0.0;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:   fixD s d k" << umodel.components[ii].s
               << umodel.components[ii].D << umodel.components[ii].f_f0;
      umodel.components[ii].s = 0.0;
      umodel.components[ii].D = 0.0;
      umodel.components[ii].f = 0.0;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:   fixK s d k" << umodel.components[ii].s
               << umodel.components[ii].D << umodel.components[ii].f_f0;
      umodel.components[ii].mw = 0.0;
      umodel.components[ii].D = 0.0;
      umodel.components[ii].f = 0.0;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:   fixD w d k" << umodel.components[ii].mw
               << umodel.components[ii].D << umodel.components[ii].f_f0;
      umodel.components[ii].s = 0.0;
      umodel.components[ii].mw = 0.0;
      umodel.components[ii].f = 0.0;
      umodel.calc_coefficients(umodel.components[ii]);
      qDebug() << "DmS:AdjMd:   fixK s d w" << umodel.components[ii].s
               << umodel.components[ii].D << umodel.components[ii].mw;
      //*DEBUG*
    }  // END: components loop

    for (int ii = 0; ii < nasso; ii++) {
      bool fltd = (mstats[ks][0] != mstats[ks][1]);
      bool flto = (mstats[ks + 1][0] != mstats[ks + 1][1]);
      double k_d = fltd ? mstats[ks++][stx] : mstats[ks++][0];
      double k_off = flto ? mstats[ks++][stx] : mstats[ks++][0];
      umodel.associations[ii].k_d = k_d;
      umodel.associations[ii].k_off = k_off;
      qDebug() << "DmS:AdjMd: Aii=" << ii << "d off"
               << umodel.associations[ii].k_d << umodel.associations[ii].k_off;
    }  // END: associations loop
  }  // END:  smtype==mean|median|mode

  return stx;
}
