//! \file us_modelmetrics.cpp

#include "us_modelmetrics.h"

#include "us_gui_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#endif

#ifndef DbgLv
#define DbgLv(a) \
  if (dbg_level >= a) qDebug()
#endif

//! \brief Main program for US_ModelMetrics. Loads translators and starts
//         the class US_ModelMetrics

int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_ModelMetrics w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

// qSort LessThan method for S_Solute sort
bool distro_lessthan(
    const S_Solute& solu1,
    const S_Solute& solu2) {  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
  return (solu1.s < solu2.s) || ((solu1.s == solu2.s) && (solu1.k < solu2.k));
}

const double epsilon = 0.0005;  // equivalence magnitude ratio radius

// Constructor
US_ModelMetrics::US_ModelMetrics() : US_Widgets() {
  dbg_level = US_Settings::us_debug();

  mfilter = "";  // default model list filter used in loading the model

  setWindowTitle(tr("Model Metrics Calculator"));
  setPalette(US_GuiSettings::frameColor());

  QGridLayout* top = new QGridLayout(this);
  top->setSpacing(2);
  top->setContentsMargins(2, 2, 2, 2);

  int row = 0;

  QPushButton* pb_investigator = us_pushbutton(tr("Select Investigator"));
  connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

  if (US_Settings::us_inv_level() < 1) pb_investigator->setEnabled(false);

  int id = US_Settings::us_inv_ID();
  QString number =
      (id > 0) ? QString::number(US_Settings::us_inv_ID()) + ": " : "";
  le_investigator = us_lineedit(number + US_Settings::us_inv_name(), 1, true);

  disk_controls = new US_Disk_DB_Controls;

  QPushButton* pb_prefilter = us_pushbutton(tr("Select Prefilter"));
  connect(pb_prefilter, SIGNAL(clicked()), SLOT(select_prefilter()));

  le_prefilter = us_lineedit("", 1, true);

  QPushButton* pb_load_model = us_pushbutton(tr("Load Model"));
  connect(pb_load_model, SIGNAL(clicked()), SLOT(load_model()));

  le_model = us_lineedit("", 1, true);

  QLabel* lbl_experiment = us_label(tr("Edit + Model #: "), -1);

  le_experiment = us_lineedit("", 1, true);

  QLabel* lbl_plotxmin = us_label(tr("Plot min: "), -1);
  QLabel* lbl_plotxmax = us_label(tr("Plot max: "), -1);
  le_plotxmin = us_lineedit("", 1, false);
  le_plotxmax = us_lineedit("", 1, false);
  connect(le_plotxmin, SIGNAL(textChanged(const QString&)), this,
          SLOT(set_plotxmin(const QString&)));
  connect(le_plotxmax, SIGNAL(textChanged(const QString&)), this,
          SLOT(set_plotxmax(const QString&)));

  QLabel* lbl_param = us_label(tr("Select\nParameter: "), -1);
  QLabel* lbl_sigma = us_label(tr("Sigma: "), -1);

  xautoscale = true;

  ct_sigma = us_counter(2, 0.0, 1.0, 1.0);
  ct_sigma->setSingleStep(0.001);
  ct_sigma->setValue(0.0);
  ct_sigma->setEnabled(false);
  ct_sigma->setFixedSize(130, 25);
  connect(ct_sigma, SIGNAL(valueChanged(double)), this,
          SLOT(set_sigma(double)));

  pb_report = us_pushbutton(tr("Save to Report"));
  pb_report->setEnabled(false);
  connect(pb_report, SIGNAL(clicked()), SLOT(addReportItem()));

  bg_hp = new QButtonGroup(this);
  QGridLayout* gl_s = us_radiobutton(tr("s"), rb_s, true);
  QGridLayout* gl_k = us_radiobutton(tr("ff0"), rb_k, false);
  QGridLayout* gl_m = us_radiobutton(tr("mw"), rb_m, false);
  QGridLayout* gl_v = us_radiobutton(tr("vbar"), rb_v, false);
  QGridLayout* gl_d = us_radiobutton(tr("D"), rb_d, false);
  QGridLayout* gl_f = us_radiobutton(tr("f"), rb_f, false);
  QGridLayout* gl_r = us_radiobutton(tr("Rh"), rb_r, false);

  bg_hp->addButton(rb_s, HPs);
  bg_hp->addButton(rb_k, HPk);
  bg_hp->addButton(rb_m, HPm);
  bg_hp->addButton(rb_v, HPv);
  bg_hp->addButton(rb_d, HPd);
  bg_hp->addButton(rb_f, HPf);
  bg_hp->addButton(rb_r, HPr);

  rb_s->setChecked(true);
  rb_s->setToolTip(tr("Select Sedimentation Coefficient"));
  rb_k->setToolTip(tr("Select Frictional Ratio"));
  rb_m->setToolTip(tr("Select Molecular Weight"));
  rb_v->setToolTip(tr("Select Partial Specific Volume"));
  rb_d->setToolTip(tr("Select Diffusion Coefficient"));
  rb_f->setToolTip(tr("Select Frictional Coefficient"));
  rb_r->setToolTip(tr("Select Hydrodynamic Radius"));
  connect(bg_hp, SIGNAL(buttonReleased(int)), this, SLOT(select_hp(int)));

  lbl_dval1 = us_label(tr("D10 value: "), -1);
  lbl_dval2 = us_label(tr("D50 value: "), -1);
  lbl_dval3 = us_label(tr("D90 value: "), -1);
  lbl_span = us_label(tr("(D90-D10)/D50: "), -1);

  le_dval1 = us_lineedit("", 1, true);
  le_dval2 = us_lineedit("", 1, true);
  le_dval3 = us_lineedit("", 1, true);
  le_span = us_lineedit("", 1, true);

  ct_dval1 = us_counter(2, 0.0, 100.0, 10.0);
  ct_dval1->setSingleStep(0.1);
  ct_dval1->setEnabled(false);
  ct_dval1->setFixedSize(130, 25);
  connect(ct_dval1, SIGNAL(valueChanged(double)), this,
          SLOT(set_dval1(double)));

  ct_dval2 = us_counter(2, 0.0, 100.0, 50.0);
  ct_dval2->setSingleStep(0.1);
  ct_dval2->setEnabled(false);
  ct_dval2->setFixedSize(130, 25);
  connect(ct_dval2, SIGNAL(valueChanged(double)), this,
          SLOT(set_dval2(double)));

  ct_dval3 = us_counter(2, 0.0, 100.0, 90.0);
  ct_dval3->setSingleStep(0.1);
  ct_dval3->setEnabled(false);
  ct_dval3->setFixedSize(130, 25);
  connect(ct_dval3, SIGNAL(valueChanged(double)), this,
          SLOT(set_dval3(double)));

  lbl_integral = us_label(tr("D10, D90 Integral: "), -1);
  lbl_minimum = us_label(tr("Minimum: "), -1);
  lbl_maximum = us_label(tr("Maximum: "), -1);
  lbl_mean = us_label(tr("Mean: "), -1);
  lbl_mode = us_label(tr("Mode: "), -1);
  lbl_median = us_label(tr("Median: "), -1);
  lbl_skew = us_label(tr("Skew: "), -1);
  lbl_kurtosis = us_label(tr("Kurtosis: "), -1);
  lbl_name = us_label(tr("Model Description: "), -1);

  le_integral = us_lineedit("", 1, true);
  le_minimum = us_lineedit("", 1, true);
  le_maximum = us_lineedit("", 1, true);
  le_mean = us_lineedit("", 1, true);
  le_mode = us_lineedit("", 1, true);
  le_median = us_lineedit("", 1, true);
  le_skew = us_lineedit("", 1, true);
  le_kurtosis = us_lineedit("", 1, true);
  le_name = us_lineedit("", 1, false);
  connect(le_name, SIGNAL(textChanged(const QString&)), this,
          SLOT(update_name(const QString&)));

  pb_write = us_pushbutton(tr("Show Report"));
  pb_write->setEnabled(false);
  connect(pb_write, SIGNAL(clicked()), SLOT(write_report()));

  QPushButton* pb_reset = us_pushbutton(tr("Reset"));
  connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

  QPushButton* pb_help = us_pushbutton(tr("Help"));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  QPushButton* pb_accept = us_pushbutton(tr("Close"));
  connect(pb_accept, SIGNAL(clicked()), SLOT(close()));

  QBoxLayout* plot = new US_Plot(data_plot, tr(""), "Parameter Value",
                                 "Relative Concentration");

  data_plot->setAutoDelete(true);
  data_plot->setMinimumSize(500, 100);

  data_plot->enableAxis(QwtPlot::xBottom, true);
  data_plot->enableAxis(QwtPlot::yLeft, true);
  data_plot->enableAxis(QwtPlot::yRight, false);
  data_plot->setAxisScale(QwtPlot::xBottom, 1.0, 40.0);
  data_plot->setAxisScale(QwtPlot::yLeft, 1.0, 4.0);

  data_plot->setCanvasBackground(Qt::black);

  pick = new US_PlotPicker(data_plot);
  // Set rubber band to display for Control+Left Mouse Button
  pick->setRubberBand(QwtPicker::VLineRubberBand);
  pick->setMousePattern(QwtEventPattern::MouseSelect1, Qt::LeftButton,
                        Qt::ControlModifier);

  QGridLayout* gl0;
  gl0 = new QGridLayout();

  gl0->addLayout(gl_s, 0, 0, 1, 1);
  gl0->addLayout(gl_d, 0, 1, 1, 1);
  gl0->addLayout(gl_m, 0, 2, 1, 1);
  gl0->addLayout(gl_k, 0, 3, 1, 1);
  gl0->addLayout(gl_f, 1, 0, 1, 1);
  gl0->addLayout(gl_v, 1, 1, 1, 1);
  gl0->addLayout(gl_r, 1, 2, 1, 1);

  QGridLayout* gl_plot;
  gl_plot = new QGridLayout();
  gl_plot->addWidget(lbl_plotxmin, 0, 0);
  gl_plot->addWidget(le_plotxmin, 0, 1);
  gl_plot->addWidget(lbl_plotxmax, 0, 2);
  gl_plot->addWidget(le_plotxmax, 0, 3);

  QGridLayout* gl1;
  gl1 = new QGridLayout();
  gl1->setColumnStretch(0, 0);
  gl1->setColumnStretch(1, 0);
  gl1->setColumnStretch(2, 1);

  gl1->addWidget(lbl_param, 0, 0, 2, 1);
  gl1->addLayout(gl0, 0, 1, 2, 2);
  gl1->addLayout(gl_plot, 2, 0, 1, 3);
  gl1->addWidget(lbl_sigma, 3, 0, 1, 1);
  gl1->addWidget(ct_sigma, 3, 1, 1, 1);
  gl1->addWidget(pb_report, 3, 2, 1, 1);
  gl1->addWidget(lbl_dval1, 4, 0, 1, 1);
  gl1->addWidget(ct_dval1, 4, 1, 1, 1);
  gl1->addWidget(le_dval1, 4, 2, 1, 1);
  gl1->addWidget(lbl_dval2, 5, 0, 1, 1);
  gl1->addWidget(ct_dval2, 5, 1, 1, 1);
  gl1->addWidget(le_dval2, 5, 2, 1, 1);
  gl1->addWidget(lbl_dval3, 6, 0, 1, 1);
  gl1->addWidget(ct_dval3, 6, 1, 1, 1);
  gl1->addWidget(le_dval3, 6, 2, 1, 1);

  QGridLayout* gl2;
  gl2 = new QGridLayout();
  gl2->setColumnStretch(0, 0);
  gl2->setColumnStretch(1, 2);
  gl2->setColumnStretch(2, 8);

  gl2->addWidget(pb_investigator, row, 0, 1, 1);
  gl2->addWidget(le_investigator, row, 1, 1, 1);
  gl2->addLayout(plot, row, 2, 23, 1);
  row++;
  gl2->addLayout(disk_controls, row, 0, 1, 2);
  row++;
  gl2->addWidget(pb_prefilter, row, 0, 1, 1);
  gl2->addWidget(le_prefilter, row, 1, 1, 1);
  row++;
  gl2->addWidget(pb_load_model, row, 0, 1, 1);
  gl2->addWidget(le_model, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_experiment, row, 0, 1, 1);
  gl2->addWidget(le_experiment, row, 1, 1, 1);
  row++;
  gl2->addLayout(gl1, row, 0, 3, 2);
  row += 3;
  gl2->addWidget(lbl_span, row, 0, 1, 1);
  gl2->addWidget(le_span, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_integral, row, 0, 1, 1);
  gl2->addWidget(le_integral, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_minimum, row, 0, 1, 1);
  gl2->addWidget(le_minimum, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_maximum, row, 0, 1, 1);
  gl2->addWidget(le_maximum, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_mean, row, 0, 1, 1);
  gl2->addWidget(le_mean, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_mode, row, 0, 1, 1);
  gl2->addWidget(le_mode, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_median, row, 0, 1, 1);
  gl2->addWidget(le_median, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_skew, row, 0, 1, 1);
  gl2->addWidget(le_skew, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_kurtosis, row, 0, 1, 1);
  gl2->addWidget(le_kurtosis, row, 1, 1, 1);
  row++;
  gl2->addWidget(lbl_name, row, 0, 1, 1);
  gl2->addWidget(le_name, row, 1, 1, 1);
  row++;
  gl2->addWidget(pb_write, row, 0, 1, 1);
  gl2->addWidget(pb_reset, row, 1, 1, 1);
  row++;
  gl2->addWidget(pb_help, row, 0, 1, 1);
  gl2->addWidget(pb_accept, row, 1, 1, 1);

  top->addLayout(gl2, row, 0, 2, 2);

  reset();
}

void US_ModelMetrics::select_prefilter(void) {
  QString pfmsg;
  int nruns = 0;
  pfilts.clear();

  US_SelectRuns srdiag(disk_controls->db(), pfilts);
  connect(&srdiag, SIGNAL(dkdb_changed(bool)), this,
          SLOT(update_disk_db(bool)));

  if (srdiag.exec() == QDialog::Accepted)
    nruns = pfilts.size();
  else
    pfilts.clear();

  if (nruns == 0)
    pfmsg = tr("(none chosen)");

  else
    pfmsg = QString(pfilts[0]);

  DbgLv(2) << "PreFilt: pfilts[0]" << ((nruns > 0) ? pfilts[0] : "(none)");
  le_prefilter->setText(pfmsg);
  load_model();
}

void US_ModelMetrics::load_model(void) {
  US_Model model;
  QString mdesc;
  S_Solute sol_sk;
  bool loadDB = disk_controls->db();
  double tmpval1;

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  US_ModelLoader dialog(loadDB, mfilter, model, mdesc, pfilts);
  connect(&dialog, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));
  QApplication::restoreOverrideCursor();

  QString mfnam;
  QString mname;
  QString sep;
  QString aiters;
  cmin = 1e+39;
  cmax = 1e-39;
  smin = 1e+39;
  smax = 1e-39;
  kmin = 1e+39;
  kmax = 1e-39;
  wmin = 1e+39;
  wmax = 1e-39;
  vmin = 1e+39;
  vmax = 1e-39;
  dmin = 1e+39;
  dmax = 1e-39;
  fmin = 1e+39;
  fmax = 1e-39;
  rmin = 1e+39;
  rmax = 1e-39;

  if (dialog.exec() != QDialog::Accepted) return;

  sep = mdesc.left(1);
  mname = mdesc.section(sep, 1, 1);
  mfnam = mdesc.section(sep, 2, 2);
  aiters = mdesc.section(sep, 6, 6);

  if (mfnam.isEmpty()) {  // From db:  make ID the "filename"
    mfnam = "db ID " + mdesc.section(sep, 4, 4);
  }

  else {  // From disk:  use base file name
    mfnam = QFileInfo(mfnam).baseName();
  }

  if (model.components.size() < 1) {
    return;
  }

  // insure all model coefficient properties are set
  if (!model.update_coefficients()) {
    DbgLv(0) << "*** Unable to recalculate coefficient values ***";
  }

  // parse model information from its description
  mdesc = mdesc.section(sep, 1, 1);
  method = model.typeText();
  method = mdesc.contains("-CG") ? method.replace("2DSA", "2DSA-CG") : method;
  run_name = mdesc.section(".", 0, -4);
  QString triple = mdesc.section(".", -3, -3);
  QString asys = mdesc.section(".", -2, -2);
  analysis_name = asys.section("_", 0, 1) + "_" + asys.section("_", 3, 4);

  if (method == "Manual" || method == "CUSTOMGRID") {
    int jj = mdesc.indexOf(".model");
    mdesc = (jj < 1) ? mdesc : mdesc.left(jj);
    run_name = mdesc + ".0Z280";
    analysis_name = "e00_a00_" + method + "_local";
  }
  if (aiters == "") aiters = "1";
  monte_carlo = model.monteCarlo;
  mc_iters = monte_carlo ? aiters.toInt() : 1;
  editGUID = model.editGUID;
  QString cell = triple.at(0);
  QString ch = triple.at(1);
  QString wl = triple.remove(0, 2);
  triple = "Cell " + cell + ", Channel " + ch + ", " + wl + " nm";
  report_entry.runID = run_name;
  report_entry.triple = triple;
  report_entry.analysis = method;
  report_entry.iterations = aiters;
  report_entry.edit = analysis_name;

  DbgLv(2) << "monte_carlo:" << monte_carlo << "mc_iters:" << mc_iters
           << ", mdesc: " << mdesc;
  DbgLv(2) << "aiters:" << aiters << "method:" << method << "run_name"
           << run_name;
  DbgLv(2) << "asys:" << asys << "analysis_name:" << analysis_name
           << "Triple:" << triple;

  sk_distro.clear();
  total_conc = 0.0;

  le_model->setText(cell + "/" + ch + "/" + wl + " (" + method + ")");
  le_experiment->setText(analysis_name);

  // read in and set distribution s,c,k,d values
  if (model.analysis != US_Model::COFS) {
    for (int jj = 0; jj < model.components.size(); jj++) {
      US_Model::calc_coefficients(model.components[jj]);

      sol_sk.s = model.components[jj].s * 1.0e13;
      sol_sk.k = model.components[jj].f_f0;
      sol_sk.c = model.components[jj].signal_concentration;
      sol_sk.w = model.components[jj].mw;
      sol_sk.v = model.components[jj].vbar20;
      sol_sk.d = model.components[jj].D;
      sol_sk.f = model.components[jj].f;
      tmpval1 = model.components[jj].f / (6.0 * M_PI * 1.00194);
      total_conc += sol_sk.c;

      cmin = qMin(cmin, sol_sk.c);
      cmax = qMax(cmax, sol_sk.c);
      smin = qMin(smin, sol_sk.s);
      smax = qMax(smax, sol_sk.s);
      kmin = qMin(kmin, sol_sk.k);
      kmax = qMax(kmax, sol_sk.k);
      wmin = qMin(wmin, sol_sk.w);
      wmax = qMax(wmax, sol_sk.w);
      vmin = qMin(vmin, sol_sk.v);
      vmax = qMax(vmax, sol_sk.v);
      dmin = qMin(dmin, sol_sk.d);
      dmax = qMax(dmax, sol_sk.d);
      fmin = qMin(fmin, sol_sk.f);
      fmax = qMax(fmax, sol_sk.f);
      rmin = qMin(rmin, tmpval1);
      rmax = qMax(rmax, tmpval1);

      DbgLv(2) << "Solute jj s w k c d" << jj << sol_sk.s << sol_sk.w
               << sol_sk.k << sol_sk.c << sol_sk.d << " vb"
               << model.components[jj].vbar20;

      sk_distro << sol_sk;
    }

    DbgLv(2) << "sk_distro.size() before reduction: " << sk_distro.size();

    // sort and reduce distribution
    sort_distro(sk_distro, true);

    /*
          DbgLv(2) << "sk_distro.size() after reduction: " << sk_distro.size();
          for ( int jj=0;jj<sk_distro.size();jj++ )
          {
             DbgLv(2) << " jj" << jj << " s k" << sk_distro[jj].s <<
       sk_distro[jj].k
                      << " w v" << sk_distro[jj].w << sk_distro[jj].v;
          }
    */
  }

  // Determine which attribute is fixed
  if (equivalent(vmin, vmax, 0.001))
    fixed = HPv;
  else if (equivalent(kmin, kmax, 0.001))
    fixed = HPk;
  else if (equivalent(smin, smax, 0.001))
    fixed = HPs;
  else if (equivalent(wmin, wmax, 0.001))
    fixed = HPm;
  else if (equivalent(dmin, dmax, 0.001))
    fixed = HPd;
  else if (equivalent(fmin, fmax, 0.001))
    fixed = HPf;
  else if (equivalent(rmin, rmax, 0.001))
    fixed = HPr;

  /*
  DbgLv(2) << "dmin, dmax: " << dmin << dmax;
  DbgLv(2) << "kmin, kmax: " << kmin << kmax;
  DbgLv(2) << "wmin, wmax: " << wmin << wmax;
  DbgLv(2) << "vmin, vmax: " << vmin << vmax;
  DbgLv(2) << "fmin, fmax: " << fmin << fmax;
  */

  if (HPs != fixed) rb_s->setEnabled(true);
  if (HPd != fixed) rb_d->setEnabled(true);
  if (HPk != fixed) rb_k->setEnabled(true);
  if (HPf != fixed) rb_f->setEnabled(true);
  if (HPm != fixed) rb_m->setEnabled(true);
  if (HPv != fixed) rb_v->setEnabled(true);
  if (HPr != fixed) rb_r->setEnabled(true);
  DbgLv(2) << "Total concentration, array size original: " << total_conc
           << sk_distro.size();
  calc();

  ct_dval1->setEnabled(true);
  ct_dval2->setEnabled(true);
  ct_dval3->setEnabled(true);
  ct_sigma->setEnabled(true);
  pb_report->setEnabled(true);
}

// Select DB investigator
void US_ModelMetrics::sel_investigator(void) {
  int investigator = US_Settings::us_inv_ID();

  US_Investigator* dialog = new US_Investigator(true, investigator);
  dialog->exec();

  investigator = US_Settings::us_inv_ID();

  QString inv_text =
      QString::number(investigator) + ": " + US_Settings::us_inv_name();

  le_investigator->setText(inv_text);
  report_entry.investigator = US_Settings::us_inv_name();
}

// Reset parameters to their defaults
void US_ModelMetrics::reset(void) {
  report_entry.investigator = US_Settings::us_inv_name();
  model_count = 0;
  QString str;
  tc = 0.0;
  sigma = 0.0;
  cmin = 1e+39;
  cmax = 1e-39;
  smin = 1e+39;
  smax = 1e-39;
  kmin = 1e+39;
  kmax = 1e-39;
  wmin = 1e+39;
  wmax = 1e-39;
  vmin = 1e+39;
  vmax = 1e-39;
  dmin = 1e+39;
  dmax = 1e-39;
  fmin = 1e+39;
  fmax = 1e-39;
  dval1 = 10.0;
  dval2 = 50.0;
  dval3 = 90.0;
  set_dval_labels(true);

  report_entry.sigma = "0.0";
  report_entry.sigma = str.setNum(sigma, 'g', 3);
  report_entry.d[0] = str.setNum(dval1, 'g', 4);
  report_entry.d[1] = str.setNum(dval2, 'g', 4);
  report_entry.d[2] = str.setNum(dval3, 'g', 4);

  DbgLv(1) << "Dsettings: " << report_entry.d[0] << report_entry.d[1]
           << report_entry.d[2] << endl;
  pb_report->setEnabled(false);
  pb_write->setEnabled(false);
  ct_dval1->setEnabled(false);
  ct_dval2->setEnabled(false);
  ct_dval3->setEnabled(false);
  ct_sigma->setEnabled(false);
  rb_s->setChecked(true);
  rb_s->setEnabled(false);
  rb_d->setEnabled(false);
  rb_k->setEnabled(false);
  rb_f->setEnabled(false);
  rb_m->setEnabled(false);
  rb_v->setEnabled(false);
  rb_r->setEnabled(false);
  calc_val = HPs;  // calculate sedimentation distributions by default

  le_model->setText("");
  le_experiment->setText("");
  le_dval1->setText("");
  le_dval2->setText("");
  le_dval3->setText("");
  le_span->setText("");
  le_integral->setText("");
  le_minimum->setText("");
  le_maximum->setText("");
  le_mean->setText("");
  le_mode->setText("");
  le_median->setText("");
  le_skew->setText("");
  le_kurtosis->setText("");
  le_name->setText("");
  if (report != "" && !saved)
    write_report();  // write any unsaved report items to disk
  report = "";
  dataPlotClear(data_plot);
  data_plot->replot();
  saved = false;
}

// Select DB investigator// Private slot to update disk/db control with dialog
// changes it
void US_ModelMetrics::update_disk_db(bool isDB) {
  if (isDB)
    disk_controls->set_db();
  else
    disk_controls->set_disk();
}

void US_ModelMetrics::write_report(void) {
  report += "</body>\n</html>";
  report_ts << report;
  report_file.close();
  QDesktopServices::openUrl("file://" + US_Settings::tmpDir() +
                            "/ModelStatistics.html");
  saved = true;
}

// Sort distribution solute list by s,k values and optionally reduce
void US_ModelMetrics::sort_distro(QList<S_Solute>& listsols, bool reduce) {
  int sizi = listsols.size();

  if (sizi < 2) return;  // nothing need be done for 1-element list

  // sort distro solute list by s,k values

  qSort(listsols.begin(), listsols.end(), distro_lessthan);

  // check reduce flag

  if (reduce) {  // skip any duplicates in sorted list
    S_Solute sol1;
    S_Solute sol2;
    QList<S_Solute> reduced;
    sol1 = listsols.at(0);
    reduced.append(sol1);  // output first entry
    int kdup = 0;
    int jdup = 0;

    for (int jj = 1; jj < sizi;
         jj++) {               // loop to compare each entry to previous
      sol2 = listsols.at(jj);  // solute entry

      if (sol1.s != sol2.s ||
          sol1.k != sol2.k) {  // not a duplicate, so output to temporary list
        reduced.append(sol2);
        jdup = 0;
      }

      else {  // duplicate:  sum c value
        DbgLv(2) << "DUP: sval svpr jj" << sol1.s << sol2.s << jj;
        kdup = max(kdup, ++jdup);
        qreal f = (qreal)(jdup + 1);
        sol2.c += sol1.c;                       // sum c value
        sol2.s = (sol1.s * jdup + sol2.s) / f;  // average s,k
        sol2.k = (sol1.k * jdup + sol2.k) / f;
        reduced.replace(reduced.size() - 1, sol2);
      }

      sol1 = sol2;  // save entry for next iteration
    }

    if (reduced.size() < sizi) {  // if some reduction happened, replace list
                                  // with reduced version
      // double sc = 1.0 / (double)( kdup + 1 );

      // for ( int ii = 0; ii < reduced.size(); ii++ )
      //    reduced[ ii ].c *= sc;

      listsols = reduced;
    }
  }
  return;
}

// Flag whether two values are effectively equal within a given epsilon
bool US_ModelMetrics::equivalent(double a, double b, double eps) {
  return (qAbs((a - b) / a) <= eps);
}

void US_ModelMetrics::select_hp(int button) {
  xautoscale = true;
  calc_val = button;
  calc();
}

void US_ModelMetrics::calc() {
  int i;
  QString str1, str2, str3;
  HydroParm val1;
  HydroParm val2;
  QList<HydroParm> temp_list;
  orig_list.clear();  // contains saved copy of the original hp_distro list
  hp_distro
      .clear();  // contains the reduced array of desired parameter with
                 // concentrations of identical parameter values added together
  temp_list.clear();  // contains the unreduced array of desired parameter
  if (calc_val == HPs) {
    report_entry.parameter = "Sedimentation Coefficient Distribution";
    xtitle = "Sedimentation Coefficient";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].s;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = smin;
    xmax = smax;
  } else if (calc_val == HPd) {
    report_entry.parameter = "Diffusion Coefficient Distribution";
    xtitle = "Diffusion Coefficient";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].d;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = dmin;
    xmax = dmax;
  } else if (calc_val == HPm) {
    report_entry.parameter = "Moleclular Weight Distribution";
    xtitle = "Molecular Weight";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].w;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = wmin;
    xmax = wmax;
  } else if (calc_val == HPk) {
    report_entry.parameter = "Frictional Ratio Distribution";
    xtitle = "Frictional Ratio";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].k;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = kmin;
    xmax = kmax;
  } else if (calc_val == HPf) {
    report_entry.parameter = "Frictional Coefficient Distribution";
    xtitle = "Frictional Coefficient";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].f;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = fmin;
    xmax = fmax;
  } else if (calc_val == HPv) {
    report_entry.parameter = "Partial Specific Volume Distribution";
    xtitle = "Partial Specific Volume";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].v;
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = vmin;
    xmax = vmax;
  } else if (calc_val == HPr) {
    report_entry.parameter = "Hydrodynamic Radius Distribution (m)";
    xtitle = "Hydrodynamic Radius";
    for (i = 0; i < sk_distro.size(); i++) {
      val1.parm = sk_distro[i].f / (6.0 * M_PI * 1.00194);
      val1.conc = sk_distro[i].c;
      temp_list.append(val1);
    }
    xmin = rmin;
    xmax = rmax;
  }
  DbgLv(2) << "In calc: xmin: " << xmin << "xmax" << xmax;
  qSort(temp_list.begin(),
        temp_list.end());  // sort the list so reduction works.
  tc = 0.0;
  val1.parm = temp_list[0].parm;
  val1.conc = temp_list[0].conc;
  orig_list.append(val1);
  tc += val1.conc;
  for (i = 1; i < temp_list.size(); i++) {
    val2.parm = temp_list[i].parm;
    val2.conc = temp_list[i].conc;
    if (val1.parm != val2.parm)  // not a duplicate, so append
    {
      orig_list.append(val2);
      tc += val2.conc;
    } else  // a duplicate, so add concentrations
    {
      val2.conc = val1.conc + val2.conc;
      tc += val2.conc - val1.conc;
      orig_list.replace(orig_list.size() - 1, val2);
    }
    val1 = val2;
  }
  hp_distro.clear();
  for (i = 0; i < orig_list.size(); i++) {
    hp_distro.push_back(orig_list.at(i));
  }
  if (sigma > 0.0) {
    update_sigma();
  } else {
    plot_data();
  }
}

void US_ModelMetrics::set_dval_labels(bool update) {
  QString str1, str2, str3;
  lbl_dval1->setText("D" + str1.setNum((float)dval1, 'g', 4) + " value: ");
  lbl_dval2->setText("D" + str1.setNum((float)dval2, 'g', 4) + " value: ");
  lbl_dval3->setText("D" + str1.setNum((float)dval3, 'g', 4) + " value: ");
  lbl_span->setText("(D" + str1.setNum((float)dval3, 'g', 4) + "-D" +
                    str2.setNum((float)dval1, 'g', 4) + ")/D" +
                    str3.setNum((float)dval2, 'g', 4) + ": ");
  lbl_integral->setText("D" + str1.setNum((float)dval1, 'g', 4) + ", D" +
                        str2.setNum((float)dval3, 'g', 4) + " Integral: ");
  if (update) {
    disconnect(ct_dval1);
    disconnect(ct_dval2);
    disconnect(ct_dval3);
    disconnect(ct_sigma);
    ct_dval1->setValue(dval1);
    ct_dval2->setValue(dval2);
    ct_dval3->setValue(dval3);
    ct_sigma->setValue(sigma);
    connect(ct_dval1, SIGNAL(valueChanged(double)), this,
            SLOT(set_dval1(double)));
    connect(ct_dval2, SIGNAL(valueChanged(double)), this,
            SLOT(set_dval2(double)));
    connect(ct_dval3, SIGNAL(valueChanged(double)), this,
            SLOT(set_dval3(double)));
    connect(ct_sigma, SIGNAL(valueChanged(double)), this,
            SLOT(set_sigma(double)));
  }
}

void US_ModelMetrics::plot_data() {
  QString str1, str2, str3;
  int points = hp_distro.size(), i;
  double sum1 = 0.0, sum2, sum3, mode, median, skew, kurtosis, percentage,
         integral;
  set_dval_labels();
  i = 0;
  // qDebug() << "Points: " << points;

  while (sum1 <= tc * dval1 / 100.0 && i < points) {
    sum1 += hp_distro[i].conc;
    i++;
    // qDebug() << sum1 << tc * dval1/100.0 << points;
  }
  xval1 = hp_distro[i - 1].parm;
  sum2 = sum1;
  while (sum2 <= tc * dval2 / 100.0 && i < points) {
    sum2 += hp_distro[i].conc;
    i++;
  }
  xval2 = hp_distro[i - 1].parm;
  sum3 = sum2;
  while (sum3 <= tc * dval3 / 100.0 && i < points) {
    sum3 += hp_distro[i].conc;
    i++;
  }
  integral = sum3 - sum1;
  percentage = 100.0 * integral / total_conc;
  xval3 = hp_distro[i - 1].parm;

  int midx = (points + 1) / 2 - 1;
  median = (midx == (points / 2))
               ? hp_distro[midx].parm
               : (hp_distro[midx].parm + hp_distro[midx + 1].parm) * 0.5;

  sum1 = 0.0;
  double mode_conc = 0.0, m2 = 0.0, m3 = 0.0, m4 = 0.0, tmp_val;
  mode = 0.0;
  skew = 0.0;
  kurtosis = 0.0;
  for (i = 0; i < points; i++) {
    sum1 += hp_distro[i].parm * hp_distro[i].conc;
    if (hp_distro[i].conc > mode_conc) {
      mode = hp_distro[i].parm;
      mode_conc = hp_distro[i].conc;
    }
  }
  sum1 /= tc;  // mean
  for (i = 0; i < points; i++) {
    sum2 = hp_distro[i].parm - sum1;
    tmp_val = sum2 * sum2;
    m2 += tmp_val;
    m3 += tmp_val * sum2;
    m4 += tmp_val * tmp_val;
  }
  m2 /= (double)points;
  m3 /= (double)points;
  m4 /= (double)points;
  skew = m3 / pow(m2, 1.5);
  kurtosis = m4 / pow(m2, 2.0) - 3.0;

  if (calc_val == HPs || calc_val == HPk) {
    le_dval1->setText(str1.setNum(xval1, 'f', 3));
    le_dval2->setText(str1.setNum(xval2, 'f', 3));
    le_dval3->setText(str1.setNum(xval3, 'f', 3));
    le_integral->setText(str1.setNum(integral, 'f', 3) + " (" +
                         str2.setNum(percentage, 'f', 3) + "\%)");
    le_minimum->setText(str1.setNum(hp_distro[0].parm, 'f', 3));
    le_maximum->setText(str1.setNum(hp_distro[points - 1].parm, 'f', 3));
    le_mean->setText(str1.setNum(sum1, 'f', 3));
    le_mode->setText(str1.setNum(mode, 'f', 3));
    le_median->setText(str1.setNum(median, 'f', 3));
  } else {
    le_dval1->setText(str1.setNum(xval1, 'e', 3));
    le_dval2->setText(str1.setNum(xval2, 'e', 3));
    le_dval3->setText(str1.setNum(xval3, 'e', 3));
    le_integral->setText(str1.setNum(integral, 'e', 3) + " (" +
                         str2.setNum(percentage, 'f', 3) + "\%)");
    le_minimum->setText(str1.setNum(hp_distro[0].parm, 'e', 3));
    le_maximum->setText(str1.setNum(hp_distro[points - 1].parm, 'e', 3));
    le_mean->setText(str1.setNum(sum1, 'e', 3));
    le_mode->setText(str1.setNum(mode, 'e', 3));
    le_median->setText(str1.setNum(median, 'e', 3));
  }
  le_span->setText(str1.setNum(((xval3 - xval1) / xval2), 'f', 3));
  le_skew->setText(str1.setNum(skew, 'f', 3));
  le_kurtosis->setText(str1.setNum(kurtosis, 'f', 3));

  report_entry.x[0] = le_dval1->text();
  report_entry.x[1] = le_dval2->text();
  report_entry.x[2] = le_dval3->text();
  report_entry.span_label = lbl_span->text();
  report_entry.span = le_span->text();
  report_entry.integral = le_integral->text();
  report_entry.totalc = str1.setNum(total_conc, 'f', 3);
  report_entry.minimum = le_minimum->text();
  report_entry.maximum = le_maximum->text();
  report_entry.mean = le_mean->text();
  report_entry.mode = le_mode->text();
  report_entry.median = le_median->text();
  report_entry.skew = le_skew->text();
  report_entry.kurtosis = le_kurtosis->text();
  report_entry.sigma = str1.setNum(sigma, 'f', 3);

  QVector<double> xv;
  QVector<double> yv;
  xv.clear();
  yv.clear();
  double mxc = 0.0;
  for (i = 0; i < points; i++) {
    xv.push_back(hp_distro[i].parm);
    yv.push_back(hp_distro[i].conc);
    mxc = qMax(mxc, hp_distro[i].conc);
  }
  xx = xv.data();
  yy = yv.data();
  double x1[2], x2[2], x3[2], y1[2], y2[2], y3[2];
  x1[0] = xval1;
  x2[0] = xval2;
  x3[0] = xval3;
  x1[1] = xval1;
  x2[1] = xval2;
  x3[1] = xval3;
  y1[0] = 0;
  y2[0] = 0;
  y3[0] = 0;
  y1[1] = mxc;
  y2[1] = mxc;
  y3[1] = mxc;
  dataPlotClear(data_plot);

  QFont sfont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1);
  QFontMetrics fmet(sfont);
  QwtLegend* legend = new QwtLegend;
  legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
  legend->setFont(sfont);
  data_plot->insertLegend(legend, QwtPlot::BottomLegend);

  QwtPlotCurve* curve1;
  QwtPlotCurve* curve2;
  QwtPlotCurve* curve3;
  QwtPlotCurve* curve4;
  curve1 = us_curve(data_plot, tr("Distribution"));
  curve2 = us_curve(data_plot, "D" + str1.setNum((float)dval1, 'g', 4));
  curve3 = us_curve(data_plot, "D" + str1.setNum((float)dval2, 'g', 4));
  curve4 = us_curve(data_plot, "D" + str1.setNum((float)dval3, 'g', 4));
  if (ct_sigma->value() > 0.0) {
    curve1->setPen(QPen(QBrush(Qt::yellow), 2.0));
    curve1->setStyle(QwtPlotCurve::Lines);
  } else {
    curve1->setStyle(QwtPlotCurve::Sticks);
    curve1->setPen(QPen(QBrush(Qt::yellow), 4.0));
  }
  curve2->setStyle(QwtPlotCurve::Sticks);
  curve3->setStyle(QwtPlotCurve::Sticks);
  curve4->setStyle(QwtPlotCurve::Sticks);
  curve2->setPen(QPen(QBrush(Qt::red), 2.0));
  curve3->setPen(QPen(QBrush(Qt::cyan), 2.0));
  curve4->setPen(QPen(QBrush(Qt::green), 2.0));
  curve1->setSamples(xx, yy, points);
  curve2->setSamples(x1, y1, 2);
  curve3->setSamples(x2, y2, 2);
  curve4->setSamples(x3, y3, 2);
  data_plot->setAxisAutoScale(QwtPlot::yLeft);
  if (xautoscale) {
    data_plot->setAxisAutoScale(QwtPlot::xBottom);
    plotxmin = curve1->minXValue();
    plotxmax = curve1->maxXValue();
    le_plotxmin->setText(str1.setNum(plotxmin));
    le_plotxmax->setText(str1.setNum(plotxmax));
  } else {
    data_plot->setAxisScale(QwtPlot::xBottom, plotxmin, plotxmax);
  }
  data_plot->setAxisTitle(QwtPlot::xBottom, xtitle);
  data_plot->replot();
}

void US_ModelMetrics::update_name(const QString& text) {
  report_entry.name = text;
}

void US_ModelMetrics::update_sigma(void) {
  QList<HydroParm> tmp_hplist;
  HydroParm hp;
  int points = 10000, i, j;
  double t_xmin, t_xmax, range, inc, tmp_tc = 0.0, tmp_sigma;
  t_xmin = xmin;
  t_xmax = xmax;
  range = t_xmax - t_xmin;
  t_xmin -= range / 5.0;  // add 20% to the range
  DbgLv(2) << "In update_sigma: xmin: " << xmin << "t_xmin" << t_xmin
           << "xmax:" << xmax << "t_xmax:" << t_xmax << "range:" << range;
  if (t_xmin < 0.0) t_xmin = 0.0;
  t_xmax += range / 5.0;
  range = t_xmax - t_xmin;
  tmp_sigma = sigma * range;
  inc = range / (double)(points - 1);
  tmp_hplist.clear();
  for (i = 0; i < points; i++) {
    hp.parm = t_xmin + i * inc;
    hp.conc = 0.0;
    tmp_hplist.push_back(hp);
  }
  double amp = 1.0 / (tmp_sigma * pow(2.0 * M_PI, 0.5));
  double sigsqr = tmp_sigma * tmp_sigma;
  hp_distro.clear();
  for (i = 0; i < orig_list.size(); i++) {
    hp_distro.push_back(orig_list.at(i));
  }

  for (j = 0; j < hp_distro.size(); j++) {
    for (i = 0; i < points; i++) {
      tmp_hplist[i].conc +=
          hp_distro[j].conc * amp *
          exp(-0.5 * pow(tmp_hplist[i].parm - hp_distro[j].parm, 2.0) / sigsqr);
    }
  }
  hp_distro.clear();
  for (i = 0; i < points; i++) {
    hp_distro << tmp_hplist[i];
    tmp_tc += tmp_hplist[i].conc;
  }
  for (i = 0; i < points; i++) {
    hp_distro[i].conc *=
        tc / tmp_tc;  // rescale everything to original total concentration
  }
  QString str;
  report_entry.sigma = str.setNum(sigma, 'g', 3);
  plot_data();
}

void US_ModelMetrics::set_dval1(double val) {
  dval1 = val;
  plot_data();
}

void US_ModelMetrics::set_dval2(double val) {
  dval2 = val;
  plot_data();
}

void US_ModelMetrics::set_dval3(double val) {
  dval3 = val;
  plot_data();
}

void US_ModelMetrics::set_sigma(double val) {
  sigma = val;
  if (sigma == 0.0) {
    calc();  // need to update hp_distro dimensions
  } else {
    update_sigma();
  }
}

void US_ModelMetrics::addReportItem(void) {
  QString str;
  QString timestamp =
      QDateTime::currentDateTime().toUTC().toString("MMddyyhhmmss");
  saved = false;
  report_entry.d[0] = str.setNum(dval1, 'g', 4);
  report_entry.d[1] = str.setNum(dval2, 'g', 4);
  report_entry.d[2] = str.setNum(dval3, 'g', 4);
  // report_entry.filename = US_Settings::tmpDir() + "/" + timestamp + ".png";
  report_entry.filename = timestamp + ".png";
  report_entry.csv = timestamp + ".csv";
  if (!pb_write->isEnabled())  // open a new report file in ultrascan's tmp dir
  {
    report_file.setFileName(US_Settings::tmpDir() + "/ModelStatistics.html");
    if (!report_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    report_ts.setDevice(&report_file);
    QString title = "UltraScan Model Statistics Report";
    pb_write->setEnabled(true);
    report = "<?xml version=\"1.0\"?>\n";
    report += "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"\n";
    report +=
        "                      \"http://www.w3.org/TR/xhtml1/DTD"
        "/xhtml1-strict.dtd\">\n";
    report +=
        "<html xmlns=\"http://www.w3.org/1999/xhtml\""
        " xml:lang=\"en\" lang=\"en\">\n";
    report += "<head>\n";
    report += "  <title> " + title + " </title>\n";
    report +=
        "  <meta http-equiv=\"Content-Type\" content="
        "\"text/html; charset=iso-8859-1\"/>\n";
    report += "  <style type=\"text/css\" >\n";
    report += "    td { padding-right: 1em; }\n";
    report += "    body { background-color: white; }\n";
    report += "    .break { page-break-before: always; }\n";
    report += "  </style>\n";
    report += "</head>\n<body>\n  <center>\n";
    report += "    <h1>" + title + "</h1>\n  </center>\n";
  }
  model_count++;
  report +=
      "  <center>\n    <table border='1' bgcolor=#CCCCCC cellpadding='5'>\n";
  report +=
      "      <tr><td colspan='4' bgcolor='#FFFFFF' "
      "align='center'><b>Information for Model " +
      str.setNum(model_count) + " (" + report_entry.name + "):</b></td></tr>\n";
  report +=
      "      <tr><td colspan='4' align='center' bgcolor='#FFFFFF'><img src='" +
      report_entry.filename + "'></td></tr>\n";
  report += table_row(6, "Investigator:", report_entry.investigator,
                      "Distribution Type:", report_entry.parameter);
  if (report_entry.iterations == "1") {
    report += table_row(6, "Experiment:", report_entry.runID,
                        "Analysis Method:", report_entry.analysis);
  } else {
    report +=
        table_row(6, "Experiment:", report_entry.runID, "Analysis Method:",
                  report_entry.analysis + " (" + report_entry.iterations +
                      " MC Iterations)");
  }
  report += table_row(6, "Dataset:", report_entry.triple,
                      "Edit and Model name:", report_entry.edit);
  report += indent(6) +
            "<tr><td colspan='4' align='center' bgcolor='#FFFFFF'>" +
            "<b>Statistics:</b></td></tr>\n";
  report +=
      indent(6) + "<tr><td colspan='4' align='center' bgcolor='#FFFFFF'>\n";
  report += indent(8) + "<table border='1' bgcolor=#CCCCCC cellpadding='5'>\n";

  report += table_row(10, "D" + report_entry.d[0], report_entry.x[0],
                      "D" + report_entry.d[1], report_entry.x[1],
                      "D" + report_entry.d[2], report_entry.x[2]);
  report += table_row(10, report_entry.span_label, report_entry.span,
                      "Minimum:", report_entry.minimum,
                      "Maximum:", report_entry.maximum);
  report += table_row(
      10,
      "D" + report_entry.d[0] + ", " + "D" + report_entry.d[2] + " Integral:",
      report_entry.integral, "Total Concentration:", report_entry.totalc,
      "<a href=\"" + report_entry.csv + "\">CSV File</a>", "");
  report +=
      table_row(10, "Mean:", report_entry.mean, "Mode:", report_entry.mode,
                "Median:", report_entry.median);
  report +=
      table_row(10, "Sigma:", report_entry.sigma, "Skew:", report_entry.skew,
                "Kurtosis:", report_entry.kurtosis);
  report += indent(8) + "</table>\n" + indent(6) + "</td></tr>\n";
  report += "    </table>\n  </center>\n  <p class=\"break\">\n";
  write_plot(US_Settings::tmpDir() + "/" + report_entry.filename,
             data_plot);  // write current image to tmp dir
  QFile csv(US_Settings::tmpDir() + "/" + report_entry.csv);
  if (csv.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream ts(&csv);
    for (int i = 0; i < hp_distro.size(); i++) {
      ts << hp_distro[i].parm << ", " << hp_distro[i].conc << endl;
    }
  }
  csv.close();
}

QString US_ModelMetrics::table_row(const int spaces, const QString& s1,
                                   const QString& s2, const QString& s3,
                                   const QString& s4) const {
  return (indent(spaces) + "<tr><td bgcolor='#CCCCFC'><b>" + s1 +
          "</b></td><td>" + s2 + "</td>\n" + indent(spaces) +
          "<td bgcolor='#CCCCFC'><b>" + s3 + "</b></td><td>" + s4 +
          "</td></tr>\n");
}
QString US_ModelMetrics::table_row(const int spaces, const QString& s1,
                                   const QString& s2, const QString& s3,
                                   const QString& s4, const QString& s5,
                                   const QString& s6) const {
  return (indent(spaces) + "<tr><td bgcolor='#CCCCFC'><b>" + s1 +
          "</b></td><td>" + s2 + "</td>\n" + indent(spaces) +
          "<td bgcolor='#CCCCFC'><b>" + s3 + "</b></td><td>" + s4 + "</td>\n" +
          indent(spaces) + "<td bgcolor='#CCCCFC'><b>" + s5 + "</b></td><td>" +
          s6 + "</td></tr>\n");
}

QString US_ModelMetrics::indent(const int spaces) const {
  return QString(" ").leftJustified(spaces, ' ');
}

void US_ModelMetrics::set_plotxmin(const QString& val) {
  plotxmin = val.toDouble();
  data_plot->setAxisScale(QwtPlot::xBottom, plotxmin, plotxmax);
  xautoscale = false;
  data_plot->replot();
}

void US_ModelMetrics::set_plotxmax(const QString& val) {
  plotxmax = val.toDouble();
  data_plot->setAxisScale(QwtPlot::xBottom, plotxmin, plotxmax);
  xautoscale = false;
  data_plot->replot();
}
