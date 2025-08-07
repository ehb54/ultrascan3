//! \file us_density_match.cpp

#include "us_density_match.h"

#include <QApplication>

#include "qwt_legend.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_model_params.h"
#include "us_passwd.h"
#include "us_remove_models.h"
#include "us_report.h"
#include "us_select_runs.h"
#include "us_settings.h"
#include "us_sleep.h"
#include "us_solution_vals.h"

#define DbgLv(a) \
  if (dbg_level >= a) qDebug()

// main program
int main(int argc, char* argv[]) {
  QApplication application(argc, argv);

#include "main1.inc"

  // License is OK.  Start up.

  US_Density_Match w;
  w.show();                   //!< \memberof QWidget
  return application.exec();  //!< \memberof QApplication
}

// qSort LessThan method for S_Solute sort
bool distro_lessthan(
    const S_Solute& solu1,
    const S_Solute& solu2) {  // TRUE iff  (s1<s2) || (s1==s2 && d1<d2)
  return (solu1.s < solu2.s) || ((solu1.s == solu2.s) && (solu1.d < solu2.d));
}

// US_Density_Match class constructor
US_Density_Match::US_Density_Match() : US_Widgets() {
  dbg_level = US_Settings::us_debug();

  // Set up the GUI
  setWindowTitle(tr("Density Matching"));
  setPalette(US_GuiSettings::frameColor());

  // Primary layouts
  QHBoxLayout* main = new QHBoxLayout(this);
  QVBoxLayout* left = new QVBoxLayout();
  QGridLayout* spec = new QGridLayout();
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);
  left->setSpacing(0);
  left->setContentsMargins(0, 1, 0, 1);
  spec->setSpacing(1);
  spec->setContentsMargins(0, 0, 0, 0);

  // Top banner
  QLabel* lb_info1 = us_banner(tr("Model Selection Controls"));

  // Distribution information text box
  te_distr_info = us_textedit();
  te_distr_info->setText(tr("Run:  runID.triple (method)\n") +
                         tr("    analysisID"));
  us_setReadOnly(te_distr_info, true);

  // X axis radio buttons and button group
  plot_x = 0;
  QLabel* lb_x_axis = us_label(tr("Plot X:"));
  bg_x_axis = new QButtonGroup(this);
  QGridLayout* gl_x_s = us_radiobutton(tr("s"), rb_x_s, false);
  QGridLayout* gl_x_d = us_radiobutton(tr("D"), rb_x_d, false);
  QGridLayout* gl_x_vbar = us_radiobutton(tr("vbar"), rb_x_vbar, false);
  QGridLayout* gl_x_mass = us_radiobutton(tr("m.mass"), rb_x_mass, true);
  QGridLayout* gl_x_ff0 = us_radiobutton(tr("ff0"), rb_x_ff0, false);
  QGridLayout* gl_x_rh = us_radiobutton(tr("Rh"), rb_x_rh, false);
  bg_x_axis->addButton(rb_x_s, ATTR_S);
  bg_x_axis->addButton(rb_x_d, ATTR_D);
  bg_x_axis->addButton(rb_x_vbar, ATTR_V);
  bg_x_axis->addButton(rb_x_mass, ATTR_W);
  bg_x_axis->addButton(rb_x_ff0, ATTR_K);
  bg_x_axis->addButton(rb_x_rh, ATTR_R);
  rb_x_s->setToolTip(tr("Set X axis to Sedimentation Coefficient"));
  rb_x_d->setToolTip(tr("Set X axis to Diffusion Coefficient"));
  rb_x_vbar->setToolTip(tr("Set X axis to Partial Specific Volume"));
  rb_x_mass->setToolTip(tr("Set X axis to Molar Mass"));
  rb_x_ff0->setToolTip(tr("Set X axis to Frictional Ratio"));
  rb_x_rh->setToolTip(tr("Set X axis to Hydrodynamic Radius"));
  rb_x_s->setChecked(true);
  connect(bg_x_axis, SIGNAL(buttonReleased(int)), this,
          SLOT(select_x_axis(int)));

  // Diffusion-averaging radio buttons and button group
  diff_avg = 1;
  QLabel* lb_di_avg = us_label(tr("Diffusion Coefficient Averaging:"));
  bg_di_avg = new QButtonGroup(this);
  QGridLayout* gl_da_n = us_radiobutton(tr("none"), rb_da_n, false);
  QGridLayout* gl_da_s = us_radiobutton(tr("simple"), rb_da_s, true);
  QGridLayout* gl_da_w = us_radiobutton(tr("weighted"), rb_da_w, false);
  bg_di_avg->addButton(rb_da_n, 0);
  bg_di_avg->addButton(rb_da_s, 1);
  bg_di_avg->addButton(rb_da_w, 2);
  rb_da_n->setToolTip(tr("Perform NO Diffusion Coefficient Averaging"));
  rb_da_s->setToolTip(
      tr("Do a SIMPLE average of Diffusion Coefficients across distributions"));
  rb_da_w->setToolTip(tr(
      "Do a WEIGHTED average of Diffusion Coefficients across distributions"));
  rb_da_s->setChecked(true);
  connect(bg_di_avg, SIGNAL(buttonReleased(int)), this,
          SLOT(select_di_avg(int)));

  // Various other GUI elements
  pb_refresh = us_pushbutton(tr("Refresh Plot"));
  pb_refresh->setEnabled(false);
  connect(pb_refresh, SIGNAL(clicked()), this, SLOT(plot_data()));

  pb_reset = us_pushbutton(tr("Reset"));
  pb_reset->setEnabled(true);
  connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));

  dkdb_cntrls = new US_Disk_DB_Controls(US_Settings::default_data_location());
  connect(dkdb_cntrls, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));

  pb_prefilt = us_pushbutton(tr("Select PreFilter"));

  le_prefilt = us_lineedit(tr(""), -1, true);
  connect(pb_prefilt, SIGNAL(clicked()), this, SLOT(select_prefilt()));

  pb_lddistr = us_pushbutton(tr("Load Distribution(s)"));
  pb_lddistr->setEnabled(true);
  connect(pb_lddistr, SIGNAL(clicked()), this, SLOT(load_distro()));

  pb_rmvdist = us_pushbutton(tr("Remove Distribution(s)"));
  pb_rmvdist->setEnabled(false);
  connect(pb_rmvdist, SIGNAL(clicked()), this, SLOT(remove_distro()));

  pb_mdlpars = us_pushbutton(tr("Set Model Parameters"));
  pb_mdlpars->setEnabled(false);
  connect(pb_mdlpars, SIGNAL(clicked()), this, SLOT(set_mparms()));

  pb_save = us_pushbutton(tr("Save"));
  pb_save->setEnabled(false);
  connect(pb_save, SIGNAL(clicked()), this, SLOT(save()));

  pb_help = us_pushbutton(tr("Help"));
  pb_help->setEnabled(true);
  connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));

  pb_close = us_pushbutton(tr("Close"));
  pb_close->setEnabled(true);
  connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));

  // Order plot components on the left side
  int s_row = 0;
  spec->addWidget(lb_info1, s_row++, 0, 1, 8);
  spec->addLayout(dkdb_cntrls, s_row++, 0, 1, 8);
  spec->addWidget(le_prefilt, s_row++, 0, 1, 8);
  spec->addWidget(pb_prefilt, s_row++, 0, 1, 4);
  spec->addWidget(pb_lddistr, s_row, 0, 1, 4);
  spec->addWidget(pb_rmvdist, s_row++, 4, 1, 4);
  spec->addWidget(pb_refresh, s_row, 0, 1, 4);
  spec->addWidget(pb_reset, s_row++, 4, 1, 4);
  spec->addWidget(pb_mdlpars, s_row, 0, 1, 4);
  spec->addWidget(pb_save, s_row++, 4, 1, 4);
  spec->addWidget(lb_x_axis, s_row, 0, 1, 2);
  spec->addLayout(gl_x_s, s_row, 2, 1, 2);
  spec->addLayout(gl_x_d, s_row, 4, 1, 2);
  spec->addLayout(gl_x_vbar, s_row++, 6, 1, 2);
  spec->addLayout(gl_x_mass, s_row, 2, 1, 2);
  spec->addLayout(gl_x_ff0, s_row, 4, 1, 2);
  spec->addLayout(gl_x_rh, s_row++, 6, 1, 2);
  spec->addWidget(lb_di_avg, s_row++, 0, 1, 8);
  spec->addLayout(gl_da_n, s_row, 2, 1, 2);
  spec->addLayout(gl_da_s, s_row, 4, 1, 2);
  spec->addLayout(gl_da_w, s_row++, 6, 1, 2);
  spec->addWidget(te_distr_info, s_row, 0, 2, 8);
  s_row += 2;

  // Set up analysis controls
  QLabel* lb_analysis = us_banner(tr("Analysis Controls"));
  QLabel* lb_boundPct = us_label(tr("% of Boundary:"));
  QLabel* lb_boundPos = us_label(tr("Boundary Position (%):"));
  QLabel* lb_smoothing = us_label(tr("Data Smoothing:"));

  lb_division = us_label(tr("Divisions:"));
  lb_division->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

  ct_division = us_counter(2, 0, 100, 50);
  ct_boundaryPct = us_counter(2, 10, 100, 1);
  ct_boundaryPos = us_counter(2, 0, 100, 1);
  ct_smoothing = us_counter(2, 1, 100, 1);

  ct_division->setSingleStep(1);
  ct_boundaryPct->setSingleStep(1);
  ct_boundaryPos->setSingleStep(1);
  ct_smoothing->setSingleStep(1);
  ct_division->setValue(50);
  ct_boundaryPct->setValue(90);
  ct_boundaryPos->setValue(5);
  ct_smoothing->setValue(1);

  connect(ct_division, SIGNAL(valueChanged(double)), this,
          SLOT(update_divis(double)));
  connect(ct_boundaryPct, SIGNAL(valueChanged(double)), this,
          SLOT(update_divis(double)));
  connect(ct_boundaryPos, SIGNAL(valueChanged(double)), this,
          SLOT(update_divis(double)));
  connect(ct_smoothing, SIGNAL(valueChanged(double)), this,
          SLOT(update_divis(double)));

  spec->addWidget(lb_analysis, s_row++, 0, 1, 8);
  spec->addWidget(lb_division, s_row, 0, 1, 4);
  spec->addWidget(ct_division, s_row++, 4, 1, 4);
  spec->addWidget(lb_boundPct, s_row, 0, 1, 4);
  spec->addWidget(ct_boundaryPct, s_row++, 4, 1, 4);
  spec->addWidget(lb_boundPos, s_row, 0, 1, 4);
  spec->addWidget(ct_boundaryPos, s_row++, 4, 1, 4);
  spec->addWidget(lb_smoothing, s_row, 0, 1, 4);
  spec->addWidget(ct_smoothing, s_row++, 4, 1, 4);

  spec->addWidget(pb_help, ++s_row, 0, 1, 4);
  spec->addWidget(pb_close, s_row++, 4, 1, 4);

  // Set up plot component window on right side
  xa_title = anno_title(ATTR_S);
  ya_title = anno_title(ATTR_F);
  QBoxLayout* plot =
      new US_Plot(data_plot, tr("Density Matching Data"), xa_title, ya_title);

  data_plot->setMinimumSize(600, 600);

  data_plot->enableAxis(QwtPlot::xBottom, true);
  data_plot->enableAxis(QwtPlot::yLeft, true);
  data_plot->setAxisScale(QwtPlot::xBottom, -200.0, 200.0);
  data_plot->setAxisScale(QwtPlot::yLeft, 0.0, 1.0);
  data_plot->setCanvasBackground(Qt::white);

  pick = new US_PlotPicker(data_plot);
  pick->setRubberBand(QwtPicker::RectRubberBand);

  // Put layouts together for overall layout
  left->addLayout(spec);
  left->addStretch();

  main->addLayout(left);
  main->addLayout(plot);
  main->setStretchFactor(left, 3);
  main->setStretchFactor(plot, 5);

  mfilter = "";

  // Set up variables and initial state of GUI
  reset();
}

// Reset information: clear prefilter and distributions
void US_Density_Match::reset(void) {
  DbgLv(1) << "MD:   reset: AA";
  dataPlotClear(data_plot);
  data_plot->replot();

  plot_x = ATTR_V;

  alldis.clear();
  pfilts.clear();
  v_bfracs.clear();
  v_vbars.clear();
  v_mmass.clear();
  v_hrads.clear();
  v_frats.clear();
  v_sedcs.clear();
  v_difcs.clear();

  pb_refresh->setEnabled(false);
  pb_mdlpars->setEnabled(false);
  pb_save->setEnabled(false);
  pb_rmvdist->setEnabled(false);
  le_prefilt->setText(tr("(no prefilter)"));
  te_distr_info->setText(tr("(all data cleared)"));
}

// Save plots and CSV files
void US_Density_Match::save(void) {
  int ndists = v_sedcs.size();
  QString runid = QString(alldis[0].run_name).section(".", 0, -2);
  QString reppath = US_Settings::reportDir() + "/" + runid + "/";
  QStringList fnames;

  // Save all distributions as CSV files in */ultrascan/reports/X

  QString fpfix = "dens_match_distrib_";
  QString fname;
  QString fpath;
  QDir diro(reppath);
  if (!diro.exists())  // Make sure report directory exists
    QDir(US_Settings::reportDir()).mkdir(runid);

  // Write sedimentation coeff, bfrac CSV files
  for (int ii = 0; ii < ndists; ii++) {
    fname = fpfix + "sedc_" + QString::number((ii + 1)) + ".csv";
    fpath = reppath + fname;
    write_csv(fpath, "sedi_coeff", v_sedcs[ii], "boundary_fraction", v_bfracs);
    fnames << fname;
  }

  // Write diffusion coeff, bfrac CSV files
  for (int ii = 0; ii < ndists; ii++) {
    fname = fpfix + "difc_" + QString::number((ii + 1)) + ".csv";
    fpath = reppath + fname;
    write_csv(fpath, "diff_coeff", v_difcs[ii], "boundary_fraction", v_bfracs);
    fnames << fname;
  }

  // Write X,bfrac CSV files for vbars,mmass,hrads,frats
  fname = fpfix + "vbar.csv";
  fpath = reppath + fname;
  write_csv(fpath, "vbar", v_vbars, "boundary_fraction", v_bfracs);
  fnames << fname;
  fname = fpfix + "mass.csv";
  fpath = reppath + fname;
  write_csv(fpath, "molar_mass", v_mmass, "boundary_fraction", v_bfracs);
  fnames << fname;
  fname = fpfix + "frat.csv";
  fpath = reppath + fname;
  write_csv(fpath, "fric_ratio", v_frats, "boundary_fraction", v_bfracs);
  fnames << fname;
  fname = fpfix + "hrad.csv";
  fpath = reppath + fname;
  write_csv(fpath, "hydro_radius", v_hrads, "boundary_fraction", v_bfracs);
  fnames << fname;

  // Cycle through possible plots and save PNG files
  const int plxs[] = {ATTR_S, ATTR_D, ATTR_V, ATTR_W, ATTR_K, ATTR_R};
  const char* pltp[] = {"sedc", "difc", "vbar", "mass", "frat", "hrad"};
  const int nplots = sizeof(plxs) / sizeof(plxs[0]);
  DbgLv(1) << "SV: nplots" << nplots;
  for (int ii = 0; ii < nplots; ii++) {
    plot_x = plxs[ii];
    DbgLv(1) << "SV:   plot_x" << plot_x << "ii" << ii;
    plot_data();

    QPixmap plotmap = ((QWidget*)data_plot)->grab();
    fname = fpfix + QString(pltp[ii]) + ".png";
    fpath = reppath + fname;
    plotmap.save(fpath);
    DbgLv(1) << "SV:     fpath" << fpath;
    fnames << fname;
  }

  // Restore the plot indicated by x-axis radio buttons
  plot_x = -1;
  plot_data();

  // Report files created and saved
  QString dtext = tr("In directory,\n") + reppath + ",\n" +
                  tr("   Files created and saved:\n");
  for (int ii = 0; ii < fnames.size(); ii++) {
    dtext += fnames[ii] + "\n";
  }
  te_distr_info->setText(dtext);

#if 0
   QVector< double >             v_bfracs;
   QVector< double >             v_vbars;
   QVector< double >             v_mmass;
   QVector< double >             v_hrads;
   QVector< double >             v_frats;
   QVector< QVector< double > >  v_sedcs;
   QVector< QVector< double > >  v_difcs;
#endif
}

// Plot the data
void US_Density_Match::plot_data(void) {
  int syssiz = alldis.size();

  if (syssiz < 1) return;

  DisSys* tsys = (DisSys*)&alldis.at(0);
  plot_x = (plot_x < 0) ? plot_x_select() : plot_x;
  DbgLv(1) << "DaPl: plot_x" << plot_x;

#if 0
   QVector< double >             v_bfracs;
   QVector< double >             v_vbars;
   QVector< double >             v_mmass;
   QVector< double >             v_hrads;
   QVector< double >             v_frats;
   QVector< QVector< double > >  v_sedcs;
   QVector< QVector< double > >  v_difcs;
#endif
  int npoint = v_bfracs.size();
  int ncurvs = 1;
  double* yy = v_bfracs.data();
  double* xx = v_sedcs[0].data();
  QString curvtitl("curve");
  // Plot title
  QString tstr = QString(tsys->run_name).section(".", 0, -2);
  DbgLv(1) << "DaPl: (1)tstr" << tstr;
  DbgLv(1) << "DaPl:    tstr len" << tstr.length();
  if (tstr.length() > 30) {
    tstr = QString(tstr).left(15) + "..." + QString(tstr).right(15) + "\n";
  } else
    tstr += "\n";
  DbgLv(1) << "DaPl: (2)tstr" << tstr;

  // X pointer, curve title, rest of plot title; based on plot type
  if (plot_x == ATTR_S) {
    xx = v_sedcs[0].data();
    ncurvs = v_sedcs.size();
    curvtitl = alldis[0].label;
    tstr += tr("Sedimentation Coeffs. per Distribution");
  } else if (plot_x == ATTR_W) {
    xx = v_mmass.data();
    curvtitl = tr("mmass_curve");
    tstr += tr("Molar Mass");
  } else if (plot_x == ATTR_V) {
    xx = v_vbars.data();
    DbgLv(1) << "DaPl: v_vbars" << v_vbars;
    curvtitl = tr("vbar_curve");
    tstr += tr("Partial Specific Density");
  } else if (plot_x == ATTR_D) {
    xx = v_difcs[0].data();
    ncurvs = v_difcs.size();
    curvtitl = alldis[0].label;
    tstr += tr("Diffusion Coeffs. per Distribution");
  } else if (plot_x == ATTR_R) {
    xx = v_hrads.data();
    curvtitl = tr("Rh_curve");
    tstr += tr("Hydrodynamic Radius");
  } else if (plot_x == ATTR_K) {
    xx = v_frats.data();
    curvtitl = tr("ff0_curve");
    tstr += tr("Frictional Ratio");
  }
  DbgLv(1) << "DaPl: (3)tstr" << tstr;

  // Initial plot settings
  dataPlotClear(data_plot);
  data_plot->replot();
  data_plot->setTitle(tstr);
  QColor bg = QColor(Qt::white);
  data_plot->setCanvasBackground(bg);
  int csum = bg.red() + bg.green() + bg.blue();
  pick->setTrackerPen(QPen(csum > 600 ? QColor(Qt::black) : QColor(Qt::white)));

  // Set axis settings
  xa_title = anno_title(plot_x);
  ya_title = anno_title(ATTR_F);
  data_plot->setAxisTitle(QwtPlot::xBottom, xa_title);
  data_plot->setAxisTitle(QwtPlot::yLeft, ya_title);
  QwtPlotCurve* data_curv;
  data_plot->setAxisAutoScale(QwtPlot::xBottom);
  data_plot->setAxisScale(QwtPlot::yLeft, 0.0, 1.0);

  if (ncurvs == 1) {
    // Plot a single line
    data_plot->detachItems(QwtPlotItem::Rtti_PlotLegend, true);
    QwtLegend* legend = new QwtLegend;
    data_plot->insertLegend(legend, QwtPlot::BottomLegend);
    QColor colr1(Qt::blue);
    data_curv = us_curve(data_plot, curvtitl);
    data_curv->setItemAttribute(QwtPlotItem::Legend, true);
    data_curv->setPen(QPen(QBrush(colr1), 3.0, Qt::SolidLine));
    data_curv->setStyle(QwtPlotCurve::Lines);

    DbgLv(1) << "DaPl:   npoint" << npoint << "xx" << xx[0] << xx[npoint - 1]
             << "yy" << yy[0] << yy[npoint - 1];
    data_curv->setSamples(xx, yy, npoint);

    data_plot->replot();
    return;
  }

  // Plot multiple lines, one for each model
  QFont sfont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1);
  QwtLegend* legend = new QwtLegend;
  legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
  legend->setFont(sfont);
  data_plot->insertLegend(legend, QwtPlot::BottomLegend);

  QColor lncolr1[] = {QColor(Qt::blue),    QColor(Qt::red),  QColor(Qt::green),
                      QColor(Qt::magenta), QColor(Qt::cyan), QColor(Qt::yellow),
                      QColor(Qt::gray),    QColor(Qt::black)};
  QList<QColor> lncolrs;
  if (ncurvs < 9) {  // for 8 or less curves, use set color array
    for (int ii = 0; ii < ncurvs; ii++) lncolrs << lncolr1[ii];
  } else {  // for 9 or more curves, build color array from rainbow colors
    QwtInterval colorinterv(0.0, 1.0);
    QwtLinearColorMap* colormap;
    colormap = new QwtLinearColorMap(Qt::magenta, Qt::red);
    colormap->addColorStop(0.2000, Qt::blue);
    colormap->addColorStop(0.4000, Qt::cyan);
    colormap->addColorStop(0.6000, Qt::green);
    colormap->addColorStop(0.8000, Qt::yellow);
    double posi = 1.0 / (double)(ncurvs - 1);
    double pos = 0.0;
    DbgLv(1) << "pC: ncurvs" << ncurvs << "posi" << posi;
    for (int ii = 0; ii < ncurvs; ii++) {
      DbgLv(1) << "pC:  pos" << pos << "color"
               << QColor(colormap->rgb(colorinterv, pos)) << "ii" << ii;
      lncolrs << QColor(colormap->rgb(colorinterv, pos));
      pos += posi;
      pos = qMin(pos, 1.0);
    }
  }

  for (int ii = 0; ii < ncurvs; ii++) {  // Draw each model line
    curvtitl = alldis[ii].label;
    data_curv = us_curve(data_plot, curvtitl);
    data_curv->setPen(QPen(QBrush(lncolrs[ii]), 3.0, Qt::SolidLine));
    xx = (plot_x == ATTR_S) ? v_sedcs[ii].data() : v_difcs[ii].data();
    data_curv->setStyle(QwtPlotCurve::Lines);
    data_curv->setItemAttribute(QwtPlotItem::Legend, true);
    data_curv->setSamples(xx, yy, npoint);
  }
  data_plot->replot();
}

// Plot data based on current plot type index
void US_Density_Match::plot_data(int cplx) {
  plot_x = cplx;
  plot_data();
  plot_x = plot_x_select();
}

// Load all the distributions (models)
void US_Density_Match::load_distro() {
  // Get a model description or set of descriptions for distribution data
  QList<US_Model> models;
  bool loadDB = dkdb_cntrls->db();

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  US_ModelLoader dialog(loadDB, mfilter, models, mdescs, pfilts);
  dialog.move(this->pos() + QPoint(200, 200));

  connect(&dialog, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));
  QApplication::restoreOverrideCursor();

  if (dialog.exec() != QDialog::Accepted) return;  // no selection made

  te_distr_info->setText(QString(models[0].description).section(".", 0, -4));

  for (int jj = 0; jj < models.count();
       jj++) {  // Load each selected distribution model
    load_distro(models[jj], mdescs[jj]);
  }

  bool have_m = (models.count() > 0);
  pb_rmvdist->setEnabled(have_m);
  pb_mdlpars->setEnabled(have_m);
  pb_save->setEnabled(have_m);

  // Notify user of need to set D2O-percent, label, density model values
  QString qmsg = tr("%1 models are loaded.\n\nIn the dialog to follow,\n"
                    "you must set D2O Percent values for each,\n"
                    "then review and set Label and Density values for them.\n"
                    "Insure there is a model with 0% D2O.")
                     .arg(alldis.size());
  QMessageBox::warning(this, tr("Model Parameters"), qmsg);

  // Set model distributions parameters
  set_mparms();

  // Plot S lines for all models
  rb_x_s->setChecked(true);
  select_x_axis(ATTR_S);
}

// Create distributions from a loaded model
void US_Density_Match::load_distro(US_Model model, QString mdescr) {
  DisSys tsys;
  S_Solute sol_in;
  S_Solute sol_nm;
  S_Solute sol_bf;
  QList<S_Solute> wk_distro;

  model.update_coefficients();  // fill in any missing coefficients

  QString mdesc = mdescr.section(mdescr.left(1), 1, 1);

  tsys.run_name = mdesc.section(".", 0, -3);
  QString asys = mdesc.section(".", -2, -2);
  tsys.analys_name = asys.section("_", 0, -4) + "_" + asys.section("_", -2, -1);
  tsys.method = model.typeText();
  tsys.method = (mdesc.contains("-CG")) ? tsys.method.replace("2DSA", "2DSA-CG")
                                        : tsys.method;
  if (model.global == US_Model::GLOBAL) {
    tsys.method = QString(mdesc).section('.', 2, 2).section('_', 2, 2);
  }
  tsys.editGUID = model.editGUID;
  DbgLv(1) << "LD: method" << tsys.method << "mdesc" << mdesc;

  if (tsys.method == "Manual" || tsys.method == "CUSTOMGRID") {
    int jj = mdesc.indexOf(".model");
    mdesc = (jj < 1) ? mdesc : mdesc.left(jj);
    tsys.run_name = mdesc + ".0Z280";
    tsys.analys_name = "e00_a00_" + tsys.method + "_local";
    DbgLv(1) << "LD:  run_name" << tsys.run_name;
    DbgLv(1) << "LD:  analys_name" << tsys.analys_name;
  }

  DbgLv(1) << "LD:  model:" << model.description;
  tsys.distro_type = (int)model.analysis;
  QString edir = US_Settings::tmpDir();
  QString efname = tsys.run_name + ".xml";

  // Read in edit for this model
  US_DataIO::EditedData edata;
  US_DB2* dbP = NULL;

  if (dkdb_cntrls->db()) {  // Set up to read from database
    US_Passwd pw;
    dbP = new US_DB2(pw.getPasswd());
    QStringList qry;
    qry << "get_editID" << model.editGUID;
    dbP->query(qry);
    dbP->next();
    QString editID = dbP->value(0).toString();
    int idEdit = editID.toInt();
    DbgLv(1) << "LD:   idEdit" << idEdit;
    qry.clear();
    qry << "get_editedData" << editID;
    dbP->query(qry);
    dbP->next();
    int idRaw = dbP->value(0).toString().toInt();
    efname = dbP->value(3).toString();
    DbgLv(1) << "LD:   edir" << edir;
    DbgLv(1) << "LD:   efname" << efname;
    QString epath = edir + "/" + efname;
    QString rpath = edir + "/" + efname.section(".", 0, -7) + "." +
                    efname.section(".", -5, -2) + ".auc";

    dbP->readBlobFromDB(epath, "download_editData", idEdit);
    dbP->readBlobFromDB(rpath, "download_aucData", idRaw);
  }

  // Read in edit
  US_DataIO::loadData(edir, efname, edata);
  DbgLv(1) << "LD:  edata: desc run cell chan" << edata.description
           << edata.runID << edata.cell << edata.channel;
  tsys.label = edata.description;
  tsys.d2opct = -1.0;

  // Now, get associated solution,buffer values
  QString soluID;
  QString cvbar20;
  QString bdens;
  QString bvisc;
  QString cmprss;
  QString bmanu;
  QString errmsg;
  DbgLv(1) << "LD:  solvals CALL";
  US_SolutionVals::values(dbP, &edata, soluID, cvbar20, bdens, bvisc, cmprss,
                          bmanu, errmsg);
  tsys.bdensity = bdens.toDouble();
  DbgLv(1) << "LD:  solval: bdens soluID bdensity" << bdens << soluID
           << tsys.bdensity;

  int nsolmc = model.components.size();

  // Read in and set distribution s,k,c,... values
  double tot_conc = 0.0;

  for (int jj = 0; jj < nsolmc; jj++) {
    sol_in.s = model.components[jj].s * 1.0e13;
    sol_in.k = model.components[jj].f_f0;
    sol_in.c = model.components[jj].signal_concentration;
    sol_in.w = model.components[jj].mw;
    sol_in.v = model.components[jj].vbar20;
    sol_in.d = model.components[jj].D * 1.0e7;
    sol_in.f = model.components[jj].f;

    tsys.in_distro << sol_in;
    wk_distro << sol_in;

    tot_conc += sol_in.c;
    if (jj < 3 || (jj + 4) > nsolmc)
      DbgLv(1) << "LD:    jj" << jj << "soli s,d,c,t" << sol_in.s << sol_in.d
               << sol_in.c << tot_conc;
  }

  // Sort and reduce distributions, then normalize
  sort_distro(wk_distro, true);

  tsys.nm_distro.clear();
  int nsolin = tsys.in_distro.size();
  int nsolnm = wk_distro.size();
  DbgLv(1) << "LD: totconc" << tot_conc << "nsolin nsolnm" << nsolin << nsolnm;
  for (int jj = 0; jj < nsolnm; jj++) {
    sol_nm = wk_distro[jj];    // Solute point
    sol_nm.c /= tot_conc;      // Normalized concentration
    tsys.nm_distro << sol_nm;  // Saved to "normalized" distro
    if (jj < 3 || (jj + 4) > nsolnm)
      DbgLv(1) << "LD:    jj" << jj << "soln s,d,c" << sol_nm.s << sol_nm.d
               << sol_nm.c;
  }
  DbgLv(1) << "LD: nsolin nsolnm" << nsolin << nsolnm << tsys.nm_distro.size();

  // Create version of distribution with boundary fraction

  tsys.bo_distro.clear();  // Boundary distro
  double sum_co = 0.0;

  for (int jj = 0; jj < nsolnm; jj++) {
    sol_nm = tsys.nm_distro[jj];  // Norm'd solute point
    sum_co += sol_nm.c;           // Concentration integration
    sol_bf = sol_nm;              // Boundary fraction solute point
    sol_bf.f = sum_co;            // With boundary fraction for "f"
    tsys.bo_distro << sol_bf;     // Save to boundary distro
    if (jj < 3 || (jj + 4) > nsolnm)
      DbgLv(1) << "LD:    jj" << jj << "solb s,d,c,f" << sol_bf.s << sol_bf.d
               << sol_bf.c << sol_bf.f;
  }

  // Update current distribution record
  DbgLv(1) << "LD:  call alldis.append";
  alldis.append(tsys);
  DbgLv(1) << "LD:   retn fr alldis.append";

  pb_refresh->setEnabled(true);
  pb_reset->setEnabled(true);

  // Update status text box
  QString distripl = QString(model.description).section(".", -3, -3);
  DbgLv(1) << "LD:  model:" << model.description;
  QString disdesc = tsys.label;
  QString disinfo = te_distr_info->toPlainText() + "\n  " + distripl + "  " +
                    disdesc + tr("  loaded");
  te_distr_info->setText(disinfo);
  DbgLv(1) << "LD: RETURN";
}

// Determine type of plot currently selected
int US_Density_Match::plot_x_select() {
  int plotx = ATTR_S;
  plotx = rb_x_mass->isChecked() ? ATTR_W : plotx;
  plotx = rb_x_ff0->isChecked() ? ATTR_K : plotx;
  plotx = rb_x_rh->isChecked() ? ATTR_R : plotx;
  plotx = rb_x_vbar->isChecked() ? ATTR_V : plotx;
  plotx = rb_x_s->isChecked() ? ATTR_S : plotx;
  plotx = rb_x_d->isChecked() ? ATTR_D : plotx;
  return plotx;
}

// Sort distribution solute list by s,d values and optionally reduce
void US_Density_Match::sort_distro(QList<S_Solute>& listsols, bool reduce) {
  int sizi = listsols.size();

  if (sizi < 2) return;  // nothing need be done for 1-element list

  // sort distro solute list by s,k values

  qSort(listsols.begin(), listsols.end(), distro_lessthan);

  // check reduce flag

  if (reduce) {  // skip any duplicates in sorted list
    S_Solute sol1;
    S_Solute sol2;
    QList<S_Solute> reduced;
    QList<S_Solute>::iterator jj = listsols.begin();
    sol1 = *jj;
    reduced.append(*jj);  // output first entry
    int kdup = 0;
    int jdup = 0;

    while ((++jj) !=
           listsols.end()) {  // loop to compare each entry to previous
      sol2 = *jj;             // solute entry

      if (sol1.s != sol2.s ||
          sol1.d != sol2.d) {  // not a duplicate, so output to temporary list
        reduced.append(sol2);
        jdup = 0;
      }

      else {                               // duplicate, so sum c value;
        sol2.c += sol1.c;                  // sum c value
        sol2.s = (sol1.s + sol2.s) * 0.5;  // average s,d
        sol2.d = (sol1.d + sol2.d) * 0.5;
        reduced.replace(reduced.size() - 1, sol2);
        kdup = max(kdup, ++jdup);
      }

      sol1 = sol2;  // save entry for next iteration
    }

    if (kdup >
        0) {  // if some reduction happened, replace list with reduced version
      double sc = 1.0 / (double)(kdup + 1);
      DbgLv(1) << "KDUP" << kdup;
      // sc = 1.0;

      for (int ii = 0; ii < reduced.size();
           ii++) {  // first scale c values by reciprocal of maximum replicate
                    // count
        reduced[ii].c *= sc;
      }

      listsols = reduced;
      DbgLv(1) << " reduced-size" << reduced.size();
    }
  }
  DbgLv(1) << " sol-size" << listsols.size();
  return;
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_Density_Match::update_disk_db(bool isDB) {
  isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Select a prefilter for model distributions list
void US_Density_Match::select_prefilt(void) {
  QString pfmsg;
  QString dinfo;
  int nruns = 0;
  pfilts.clear();

  US_SelectRuns srdiag(dkdb_cntrls->db(), pfilts);
  srdiag.move(this->pos() + QPoint(200, 200));
  connect(&srdiag, SIGNAL(dkdb_changed(bool)), this,
          SLOT(update_disk_db(bool)));

  if (srdiag.exec() == QDialog::Accepted)
    nruns = pfilts.size();
  else
    pfilts.clear();

  if (nruns == 0) {
    pfmsg = tr("(no prefilter)");
    dinfo = pfmsg + "\n";
  }

  else if (nruns > 1) {
    pfmsg = tr("RunID prefilter - %1 runs: ").arg(nruns) +
            QString(pfilts[0]).left(20) + "*, ...";
    dinfo = tr("RunID prefilter - %1 runs:\n").arg(nruns);
    for (int ii = 0; ii < nruns; ii++) {
      dinfo += pfilts[ii] + "\n";
    }
  }

  else {
    pfmsg =
        tr("RunID prefilter - 1 run: ") + QString(pfilts[0]).left(20) + " ...";
    dinfo = tr("RunID prefilter - 1 run:\n") + pfilts[0];
  }

  le_prefilt->setText(pfmsg);
  te_distr_info->setText(dinfo);
}

// Remove distribution(s) from the models list
void US_Density_Match::remove_distro(void) {
  DbgLv(1) << "rmvdis:Remove Distros";
  US_RemoveModels rmvd(alldis);

  if (rmvd.exec() == QDialog::Accepted) {
    int jd = alldis.size();

    if (jd < 1) {  // Handle case where ALL distributions removed
      QString dinfo = te_distr_info->toPlainText().section("\n", 0, 0) +
                      tr("\n(no models)\n");
      te_distr_info->setText(dinfo);
      dataPlotClear(data_plot);
      data_plot->replot();
    }
    DbgLv(1) << "rmvdis:Accepted";
  }

  if (alldis.size() > 0) {
    // Summarize new set of models in info box
    models_summary();
  }

  // Build and plot updated data
  build_bf_dists();  // (Re-)build boundary fraction distributions
  build_bf_vects();  // (Re-)build boundary fraction vectors
  DbgLv(1) << "rmvdis:plot_data";
  plot_data();
  DbgLv(1) << "rmvdis:DONE";
}

// Set/modify model distribution parameters
void US_Density_Match::set_mparms(void) {
  DbgLv(1) << "mdlpar:Set Model Parameters";
  // Open Model Parameters dialog
  US_ModelParams mpdiag(alldis, this);

  // Use parameters returned
  if (mpdiag.exec() == QDialog::Accepted) {  // Redo text box summarizing
                                             // models; calculate vectors
    DbgLv(1) << "mdlpar: Accepted";
    // Summarize new set of models in info box
    models_summary();

    // Rebuild vectors and replot data
    DbgLv(1) << "mdlpar:   plot_data";
    build_bf_dists();
    build_bf_vects();
    plot_data();
  }
}

// Select coordinate for horizontal axis
void US_Density_Match::select_x_axis(int ival) {
  DbgLv(1) << "sel_x:  ival" << ival;
  plot_x = ival;

  build_bf_dists();  // Build the boundary fraction distributions
  build_bf_vects();  // Build the boundary fraction vectors

  plot_data();  // Plot data
}

// Select diffusion coefficient averaging flag
void US_Density_Match::select_di_avg(int ival) {
  DbgLv(1) << "sel_da:  ival" << ival;
  diff_avg = ival;
  build_bf_dists();  // Build the boundary fraction distributions
  build_bf_vects();  // Build the boundary fraction vectors
  plot_data();       // Plot data
}

// Generate the BF version of the current distribution
void US_Density_Match::build_bf_distro(int modx) {
  if (alldis.size() <= modx) return;

  DisSys* tsys = (DisSys*)&alldis.at(modx);
  DbgLv(1) << "BldBf: modx" << modx;

  tsys->bf_distro.clear();
  int nsolbo = tsys->bo_distro.size();
  double bfrac = ct_boundaryPos->value() * 0.01;
  double bfextn = ct_boundaryPct->value() * 0.01;
  int ksolbf = (int)(ct_division->value());
  int nsolbf = ksolbf + 1;
  double bfincr = bfextn / (double)ksolbf;
  DbgLv(1) << "BldBf: bfrac bfextn bfincr" << bfrac << bfextn << bfincr
           << "nsolbo nsolbf" << nsolbo << nsolbf << (bfextn / bfincr)
           << ksolbf;
  int j2 = 0;

  // Create solute points with specified boundary fraction extent
  for (int kk = 0; kk < nsolbf; kk++) {
    while (j2 <
           nsolbo) {  // Position boundary fraction within input distribution
      if (bfrac <= tsys->bo_distro[j2].f || j2 == (nsolbo - 1)) break;
      j2++;
    }

    // Set values for current output fraction
    S_Solute sol_bf = tsys->bo_distro[j2];
    sol_bf.f = bfrac;  // Boundary fraction
    int kk2 = kk * 2;
    if (kk < 3 || (kk + 4) > nsolbf ||
        ((kk2 > (nsolbf - 4)) && (kk2 < nsolbf + 4)))
      DbgLv(1) << "BldBf:  kk bfrac" << kk << bfrac << "j2" << j2 << "f s d"
               << sol_bf.f << sol_bf.s << sol_bf.d;

    tsys->bf_distro << sol_bf;  // Solute point at boundary fraction
    bfrac += bfincr;            // Bump to next boundary fraction
  }

  DbgLv(1) << "BldBf: nsolbo nsolbf" << nsolbo << nsolbf;
}

// Generate the BoundaryFraction version of all distributions
void US_Density_Match::build_bf_dists() {
  int ndists = alldis.size();
  if (ndists < 1) return;
  for (int jj = 0; jj < ndists; jj++) {  // Build bfrac distro for each model
    build_bf_distro(jj);
  }

  if (diff_avg >
      0) {  // Replace diffusion coefficients with simple or weighted averages
    int npoints = alldis[0].bf_distro.size();
    for (int jj = 0; jj < npoints; jj++) {
      double dsum = 0.0;
      double wsum = 0.0;
      for (int ii = 0; ii < ndists;
           ii++) {  // Accumulate (weighted) sum and sum of weights
        double dwt = (diff_avg == 1) ? 1.0 : alldis[ii].bf_distro[jj].c;
        dsum += (alldis[ii].bf_distro[jj].d * dwt);
        wsum += dwt;
      }
      double dval = dsum / wsum;             // average diffusion coefficient
      for (int ii = 0; ii < ndists; ii++) {  // Propagate to all models
        alldis[ii].bf_distro[jj].d = dval;
      }
    }
  }
}

// Generate the BoundaryFraction-associated vectors
void US_Density_Match::build_bf_vects() {
  int nsmoo = (int)(ct_smoothing->value());
  DbgLv(1) << "BldVc: nsmoo" << nsmoo;
  QVector<double> v_dens;
  QVector<double> v_seds;

  // Build the boundary fractions (x) vector
  int ndists = alldis.size();
  if (ndists < 1) return;
  int npoints = alldis[0].bf_distro.size();
  v_bfracs.clear();
  v_bfracs.reserve(npoints);
  for (int jj = 0; jj < npoints;
       jj++) {  // Grab each fraction from 1st model's distro
    v_bfracs << alldis[0].bf_distro[jj].f;
  }
  DbgLv(1) << "BldVc: bf 0 1 k n" << v_bfracs[0] << v_bfracs[1]
           << v_bfracs[npoints - 2] << v_bfracs[npoints - 1];

  // Build sedimentation and diffusion vectors for each model
  v_sedcs.clear();
  v_sedcs.resize(ndists);
  v_difcs.clear();
  v_difcs.resize(ndists);
  for (int ii = 0; ii < ndists; ii++) {
    // Save density for each model distribution
    double density = alldis[ii].bdensity;
    v_dens << density;  // Vector of densities
    v_sedcs[ii].clear();
    v_difcs[ii].clear();
    v_sedcs[ii].reserve(npoints);
    v_difcs[ii].reserve(npoints);

    // Build vectors of s and D for this model
    for (int jj = 0; jj < npoints;
         jj++) {  // Append s,D values for each bf-point for this model
      v_sedcs[ii] << alldis[ii].bf_distro[jj].s;
      v_difcs[ii] << alldis[ii].bf_distro[jj].d;
    }
    DbgLv(1) << "BldVc: ii" << ii << "se 0 1 k n" << v_sedcs[ii][0]
             << v_sedcs[ii][1] << v_sedcs[ii][npoints - 2]
             << v_sedcs[ii][npoints - 1];
    DbgLv(1) << "BldVc:     di 0 1 k n" << v_difcs[ii][0] << v_difcs[ii][1]
             << v_difcs[ii][npoints - 2] << v_difcs[ii][npoints - 1];

    if (nsmoo > 1) {  // Apply gaussian smoothing to s and D vectors
      US_Math2::gaussian_smoothing(v_sedcs[ii], nsmoo);
      US_Math2::gaussian_smoothing(v_difcs[ii], nsmoo);
    }
  }
  DbgLv(1) << "BldVc: vdens" << v_dens;

  // Build vbars vector by fitting sedc,density across models at each bfrac
  v_vbars.clear();
  v_vbars.reserve(npoints);
  for (int jj = 0; jj < npoints; jj++) {
    v_seds.clear();
    for (int ii = 0; ii < ndists;
         ii++) {  // Build sed coeffs vector across models
      v_seds << v_sedcs[ii][jj];
    }
    double* xx = v_seds.data();  // X is sed coeffs
    double* yy = v_dens.data();  // Y is densities
    double slope, intcept, sigma, corre;
    US_Math2::linefit(&xx, &yy, &slope, &intcept, &sigma, &corre, ndists);
    // Intercept is density value and vbar is its reciprocal
    double vbari = (intcept > 0.0) ? (1.0 / intcept) : 1.0;
    if (jj == 0) {
      DbgLv(1) << "BldVc:  jj0: seds:" << v_seds;
      DbgLv(1) << "BldVc:  jj0: dens:" << v_dens;
      DbgLv(1) << "BldVc:  jj0:  intcept vbari" << intcept << vbari;
    }

    v_vbars << vbari;
  }
  DbgLv(1) << "BldVc: vb 0 1 k n" << v_vbars[0] << v_vbars[1]
           << v_vbars[npoints - 2] << v_vbars[npoints - 1];
  // Determine the distribution index of zero-percent-D2O
  int zx = 0;
  for (int ii = 0; ii < ndists; ii++) {
    if (alldis[ii].d2opct == 0.0) {
      zx = ii;
      break;
    }
  }
  DbgLv(1) << "BldVc:   zx" << zx;

  // Compute molar mass values and build the vector
  v_mmass.clear();
  v_mmass.reserve(npoints);
  for (int jj = 0; jj < npoints; jj++) {
    // *** Mi = si*R*T/(Di_avg*(1-vbari*rho))
    double sedco = v_sedcs[zx][jj] * 1.0e-13;
    double difco = v_difcs[zx][jj] * 1.0e-7;
    double vbari = v_vbars[jj];
    double mmass = sedco * R_GC * K20 / (difco * (1.0 - vbari * DENS_20W));
    mmass = qAbs(mmass);
    v_mmass << mmass;
  }
  DbgLv(1) << "BldVc: mm 0 1 k n" << v_mmass[0] << v_mmass[1]
           << v_mmass[npoints - 2] << v_mmass[npoints - 1];

  // Compute hydrodynamic radius values and build the vector
  v_hrads.clear();
  v_hrads.reserve(npoints);
  for (int jj = 0; jj < npoints; jj++) {
    // fi = RT/(N*Di)
    // *** ri = fi/(6 * pi * eta)   <-- hydrodynamic radius
    double difco = v_difcs[zx][jj] * 1.0e-7;
    double frico = R_GC * K20 / (difco * AVOGADRO);
    double hyrad = frico / (6.0 * M_PI * VISC_20W);
    v_hrads << (hyrad * 1.e+9);
  }
  DbgLv(1) << "BldVc: hr 0 1 k n" << v_hrads[0] << v_hrads[1]
           << v_hrads[npoints - 2] << v_hrads[npoints - 1];

  // Compute frictional ratio values and build the vector
  const double a_third = (1.0 / 3.0);
  double vol_fac = (0.75 / M_PI);
  double spfac = 0.06 * M_PI * VISC_20W;
  v_frats.clear();
  v_frats.reserve(npoints);
  for (int jj = 0; jj < npoints; jj++) {
    // Mi*vbari/N = Volume of moleculei
    // V=4/3 * pi*r_0^3    (3/(4*pi) *v)^1/3 = r_0
    // f_0i = 6 * pi * eta * r_0i
    // fi = RT/(N*Di)
    // *** fi/f_0i
    // *** ri = fi/(6 * pi * eta)   <-- hydrodynamic radius
    //      double difco     = alldis[ 0 ].bf_distro[ jj ].d * 1.0e-7;
    double vbari = v_vbars[jj];
    double mmass = v_mmass[jj];
#if 0
      double difco     = v_difcs[ zx ][ jj ] * 1.0e-7;
//      double rzero     = pow( ( ( 0.75 / M_PI ) * vbari ), a_third );
      double volum     = mmass * vbari / AVOGADRO;
      double rzero     = pow( ( vol_fac * volum ), a_third );
      double fcoef     = R_GC * K20 / ( difco * AVOGADRO );
//      double fzero     = 6.0 * M_PI * VISC_20W * rzero;
      double fzero     = 0.06 * M_PI * VISC_20W * rzero;
#endif
#if 1
    double sedco = v_sedcs[zx][jj] * 1.0e-13;
    double volum = mmass * vbari / AVOGADRO;
    double rzero = pow((vol_fac * volum), a_third);
    //   rsph_fac       = 0.06 * M_PI * VISC_20W;
    //   buoyancyb      = 1.0 - vbar * DENS_20W;
    double fcoef = mmass * (1.0 - vbari * DENS_20W) / (sedco * AVOGADRO);
    double fzero = spfac * rzero;
#endif
    double frati = qAbs(fcoef / fzero);
    v_frats << frati;
  }
  DbgLv(1) << "BldVc: fr 0 1 k n" << v_frats[0] << v_frats[1]
           << v_frats[npoints - 2] << v_frats[npoints - 1];

#if 0

*** Mi = si*R*T/(Di_avg*(1-vbari*rho))

i = boundary fraction from 0 to 1

Mi*vbari/N = Volume of moleculei

V=4/3 * pi*r_0^3    (3/(4*pi) *v)^1/3 = r_0

f_0i = 6 * pi * eta * r_0i

fi = RT/(N*Di)

*** fi/f_0i

*** ri = fi/(6 * pi * eta)   <-- hydrodynamic radius

*** --> distributions to plot, let user choose which
#endif
#if 0
   QVector< double >             v_bfracs;
   QVector< double >             v_vbars;
   QVector< double >             v_mmass;
   QVector< double >             v_hrads;
   QVector< double >             v_frats;
   QVector< QVector< double > >  v_sedcs;
   QVector< QVector< double > >  v_difcs;
#endif
}

// Set annotation title for a plot index
QString US_Density_Match::anno_title(int pltndx) {
  QString a_title;

  if (pltndx == ATTR_S)
    a_title = tr("Sedimentation Coefficient (1e-13)"
                 " for water at 20") +
              DEGC;
  else if (pltndx == ATTR_K)
    a_title = tr("Frictional Ratio f/f0");
  else if (pltndx == ATTR_W)
    a_title = tr("Molar Mass (Dalton)");
  else if (pltndx == ATTR_V)
    a_title = tr("Vbar at 20") + DEGC;
  else if (pltndx == ATTR_D)
    a_title = tr("Diffusion Coefficient (1e-7)");
  else if (pltndx == ATTR_R)
    a_title = tr("Hydrodynamic Radius (nm)");
  else if (pltndx == ATTR_F)
    a_title = tr("Boundary Fraction");

  return a_title;
}

// Set plot-type text suffix for a plot index
QString US_Density_Match::ptype_text(int pltndx) {
  QString t_text;

  if (pltndx == ATTR_S)
    t_text = "s";
  else if (pltndx == ATTR_K)
    t_text = "ff0";
  else if (pltndx == ATTR_W)
    t_text = "mmass";
  else if (pltndx == ATTR_V)
    t_text = "vbar";
  else if (pltndx == ATTR_D)
    t_text = "d";
  else if (pltndx == ATTR_R)
    t_text = "rh";
  else if (pltndx == ATTR_F)
    t_text = "bf";

  return t_text;
}

// Update structures and plot after division/percent/position change
void US_Density_Match::update_divis(double dval) {
  DbgLv(1) << "UpdDiv:" << dval;
  build_bf_dists();  // (Re-)build boundary fraction distributions
  build_bf_vects();  // (Re-)build boundary fraction vectors
  plot_data();       // Plot data

  // Also limit percent so upper limit is 100.0
  double mx_bpct = 100.0 - ct_boundaryPos->value();
  ct_boundaryPct->setMaximum(mx_bpct);
  DbgLv(1) << "UpdDiv:  mx_bpct" << mx_bpct;

  // Limit smoothing so upper limit half of divisions and odd
  double mx_smoo = ct_division->value() * 0.5;
  int nsmoo = (int)mx_smoo;
  nsmoo += ((nsmoo & 1) == 0) ? 1 : 0;
  mx_smoo = nsmoo;
  DbgLv(1) << "UpdDiv:  mx_smoo" << mx_smoo;
  ct_smoothing->setMaximum(mx_smoo);
}

// Write a CSV file from given vectors
void US_Density_Match::write_csv(const QString fpath, const QString hdr1,
                                 QVector<double>& vals1, const QString hdr2,
                                 QVector<double>& vals2) {
  DbgLv(1) << "WrCsv: fpath" << fpath;
  // Open file and textstream
  QFile datf(fpath);
  if (!datf.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
  QTextStream ts(&datf);

  // Write header line
  ts << "\"" + hdr1 + "\",\"" + hdr2 + "\"\n";

  // Write data lines
  for (int jj = 0; jj < vals1.size(); jj++) {
    QString line =
        QString().sprintf("\"%9.6e\",\"%6.4f\"\n", vals1[jj], vals2[jj]);
    line.replace(" ", "");
    ts << line;
  }

  // Close file
  datf.close();
  return;
}

// Write summary of models to info box
void US_Density_Match::models_summary(void) {
  QString dinfo = alldis[0].run_name.section(".", 0, -2) + "\n\n" +
                  tr("  D2O_Percent  Density  Label  MDescr.\n");
  DbgLv(1) << "mosmry: hdr" << dinfo;

  for (int jj = 0; jj < alldis.size();
       jj++) {  // Compose and display a distribution line for distro info box
    double d2opct = alldis[jj].d2opct;
    double bdens = alldis[jj].bdensity;
    QString mlab = alldis[jj].label;
    DbgLv(1) << "mosmry:  jj" << jj << "d2opct bdens mlab" << d2opct << bdens
             << mlab;
    QString mdesc = alldis[jj].run_name;
    int kk = mdesc.indexOf("-run");
    mdesc = (kk > 0) ? "..." + QString(mdesc).mid((kk + 1), 20) + "..." : mdesc;
    DbgLv(1) << "mosmry:    mdesc" << mdesc;
    dinfo += QString().sprintf("%.1f  %f  ", d2opct, bdens) + mlab + "  " +
             mdesc + "\n";
  }

  if (alldis.size() < 1) {
    dinfo += tr("(no models)\n");
  }

  te_distr_info->setText(dinfo);
}
