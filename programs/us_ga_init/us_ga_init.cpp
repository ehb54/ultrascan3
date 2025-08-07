//! \file us_ga_init.cpp

#include <QApplication>

#include "us_constants.h"
#include "us_ga_init.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_license.h"
#include "us_license_t.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model_loader.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_select_runs.h"
#include "us_settings.h"
#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#define setMinimum(a) setMinValue(a)
#define setMaximum(a) setMaxValue(a)
#define setSymbol(a) setSymbol(*a)
#define setStateMachine(a) setSelectionFlags(QwtPicker::RectSelection | QwtPicker::DragSelection)
#else
#include "qwt_picker_machine.h"
#define canvasBackground() canvasBackground().color();
#endif

// main program
int main(int argc, char *argv[]) {
   QApplication application(argc, argv);

#include "main1.inc"

   // License is OK.  Start up.

   US_GA_Initialize w;
   w.show(); //!< \memberof QWidget
   return application.exec(); //!< \memberof QApplication
}

// qSort LessThan method for S_Solute sort
bool distro_lessthan(const S_Solute &solu1, const S_Solute &solu2) { // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return (solu1.s < solu2.s) || ((solu1.s == solu2.s) && (solu1.k < solu2.k));
}

const double epsilon = 0.0005; // equivalence magnitude ratio radius

// US_GA_Initialize class constructor
US_GA_Initialize::US_GA_Initialize() : US_Widgets() {
   // set up the GUI

   setWindowTitle(tr("Genetic Algorithm Initialization Control Window"));
   setPalette(US_GuiSettings::frameColor());

   // primary layouts
   QHBoxLayout *main = new QHBoxLayout(this);
   QVBoxLayout *left = new QVBoxLayout();
   QVBoxLayout *rght = new QVBoxLayout();
   QGridLayout *spec = new QGridLayout();
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);
   left->setSpacing(0);
   left->setContentsMargins(0, 1, 0, 1);
   spec->setSpacing(1);
   spec->setContentsMargins(0, 0, 0, 0);
   rght->setSpacing(0);
   rght->setContentsMargins(0, 1, 0, 1);

   dbg_level = US_Settings::us_debug();

   // Insure working etc is populated with color maps
   clean_etc_dir(true);

   // series of rows: most of them label on left, counter/box on right
   lb_info1 = us_banner(tr("Genetic Algorithm Controls"));

   lb_nisols = us_label(tr("Number of Initial Solutes:"));
   lb_nisols->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_nisols = us_counter(3, 0.0, 1000.0, 0.0);
   ct_nisols->setSingleStep(1);
   ct_nisols->setEnabled(true);
   connect(ct_nisols, SIGNAL(valueChanged(double)), this, SLOT(update_nisols(double)));

   lb_wxbuck = us_label(tr("Width of s Bucket:"));
   lb_wxbuck->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_wxbuck = us_counter(3, 0.0, 10.0, 0.0);
   ct_wxbuck->setSingleStep(1);
   connect(ct_wxbuck, SIGNAL(valueChanged(double)), this, SLOT(update_wxbuck(double)));

   lb_hybuck = us_label(tr("Height of f/f0 Bucket:"));
   lb_hybuck->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_hybuck = us_counter(3, 0.0, 1.0, 0.0);
   ct_hybuck->setSingleStep(1);
   connect(ct_hybuck, SIGNAL(valueChanged(double)), this, SLOT(update_hybuck(double)));

   lb_info2 = us_banner(tr("Pseudo-3D Controls"));

   lb_resolu = us_label(tr("Pseudo-3D Resolution:"));
   lb_resolu->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_resolu = us_counter(3, 0.0, 100.0, 90.0);
   ct_resolu->setSingleStep(1);
   connect(ct_resolu, SIGNAL(valueChanged(double)), this, SLOT(update_resolu(double)));

   lb_xreso = us_label(tr("X Resolution:"));
   lb_xreso->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_xreso = us_counter(3, 10.0, 1000.0, 0.0);
   ct_xreso->setSingleStep(1);
   connect(ct_xreso, SIGNAL(valueChanged(double)), this, SLOT(update_xreso(double)));

   lb_yreso = us_label(tr("Y Resolution:"));
   lb_yreso->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_yreso = us_counter(3, 10.0, 1000.0, 0.0);
   ct_yreso->setSingleStep(1);
   connect(ct_yreso, SIGNAL(valueChanged(double)), this, SLOT(update_yreso(double)));

   lb_zfloor = us_label(tr("Z Visibility Percent:"));
   lb_zfloor->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_zfloor = us_counter(3, 50.0, 150.0, 1.0);
   ct_zfloor->setSingleStep(1);
   connect(ct_zfloor, SIGNAL(valueChanged(double)), this, SLOT(update_zfloor(double)));

   lb_autlim = us_label(tr("Automatic Plot Limits"));
   lb_autlim->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   us_checkbox(tr("(unselect to override)             "), ck_autlim, true);
   connect(ck_autlim, SIGNAL(clicked()), this, SLOT(select_autolim()));

   lb_plxmin = us_label(tr("Plot Limit s Min:"));
   lb_plxmin->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_plxmin = us_counter(3, -10000.0, 10000.0, 0.0);
   ct_plxmin->setSingleStep(1);
   connect(ct_plxmin, SIGNAL(valueChanged(double)), this, SLOT(update_plxmin(double)));

   lb_plxmax = us_label(tr("Plot Limit s Max:"));
   ct_plxmax = us_counter(3, -100.0, 10000.0, 0.0);
   ct_plxmax->setSingleStep(1);
   connect(ct_plxmax, SIGNAL(valueChanged(double)), this, SLOT(update_plxmax(double)));

   lb_plymin = us_label(tr("Plot Limit f/f0 Min:"));
   lb_plymin->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_plymin = us_counter(3, 0.5, 50.0, 0.0);
   ct_plymin->setSingleStep(1);
   connect(ct_plymin, SIGNAL(valueChanged(double)), this, SLOT(update_plymin(double)));

   lb_plymax = us_label(tr("Plot Limit f/f0 Max:"));
   lb_plymax->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

   ct_plymax = us_counter(3, 1.0, 50.0, 1.0);
   ct_plymax->setSingleStep(1);
   ct_plymax->setValue(1.34567e+01);
   connect(ct_plymax, SIGNAL(valueChanged(double)), this, SLOT(update_plymax(double)));

   lw_sbin_data = us_listwidget();
   connect(lw_sbin_data, SIGNAL(pressed(const QModelIndex &)), this, SLOT(sclick_sbdata(const QModelIndex &)));
   connect(lw_sbin_data, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(dclick_sbdata(const QModelIndex &)));
   connect(lw_sbin_data, SIGNAL(currentRowChanged(int)), this, SLOT(newrow_sbdata(int)));

   attr_x = 0;
   attr_y = 1;
   attr_z = 3;
   QLabel *lb_x_axis = us_label(tr("Plot X:"));
   QLabel *lb_y_axis = us_label(tr("Plot Y:"));
   bg_x_axis = new QButtonGroup(this);
   bg_y_axis = new QButtonGroup(this);
   QGridLayout *gl_x_s = us_radiobutton(tr("s"), rb_x_s, true);
   QGridLayout *gl_x_ff0 = us_radiobutton(tr("ff0"), rb_x_ff0, false);
   QGridLayout *gl_x_mw = us_radiobutton(tr("mw"), rb_x_mw, false);
   QGridLayout *gl_x_vbar = us_radiobutton(tr("vbar"), rb_x_vbar, false);
   QGridLayout *gl_x_D = us_radiobutton(tr("D"), rb_x_D, false);
   QGridLayout *gl_x_f = us_radiobutton(tr("f"), rb_x_f, false);
   QGridLayout *gl_y_s = us_radiobutton(tr("s"), rb_y_s, false);
   QGridLayout *gl_y_ff0 = us_radiobutton(tr("ff0"), rb_y_ff0, true);
   QGridLayout *gl_y_mw = us_radiobutton(tr("mw"), rb_y_mw, false);
   QGridLayout *gl_y_vbar = us_radiobutton(tr("vbar"), rb_y_vbar, false);
   QGridLayout *gl_y_D = us_radiobutton(tr("D"), rb_y_D, false);
   QGridLayout *gl_y_f = us_radiobutton(tr("f"), rb_y_f, false);
   bg_x_axis->addButton(rb_x_s, ATTR_S);
   bg_x_axis->addButton(rb_x_ff0, ATTR_K);
   bg_x_axis->addButton(rb_x_mw, ATTR_W);
   bg_x_axis->addButton(rb_x_vbar, ATTR_V);
   bg_x_axis->addButton(rb_x_D, ATTR_D);
   bg_x_axis->addButton(rb_x_f, ATTR_F);
   bg_y_axis->addButton(rb_y_s, ATTR_S);
   bg_y_axis->addButton(rb_y_ff0, ATTR_K);
   bg_y_axis->addButton(rb_y_mw, ATTR_W);
   bg_y_axis->addButton(rb_y_vbar, ATTR_V);
   bg_y_axis->addButton(rb_y_D, ATTR_D);
   bg_y_axis->addButton(rb_y_f, ATTR_F);
   rb_x_s->setChecked(true);
   rb_y_s->setEnabled(false);
   rb_y_ff0->setChecked(true);
   rb_x_ff0->setEnabled(false);
   rb_x_s->setToolTip(tr("Set X axis to Sedimentation Coefficient"));
   rb_x_ff0->setToolTip(tr("Set X axis to Frictional Ratio"));
   rb_x_mw->setToolTip(tr("Set X axis to Molecular Weight"));
   rb_x_vbar->setToolTip(tr("Set X axis to Partial Specific Volume"));
   rb_x_D->setToolTip(tr("Set X axis to Diffusion Coefficient"));
   rb_x_f->setToolTip(tr("Set X axis to Frictional Coefficient"));
   rb_y_s->setToolTip(tr("Set Y axis to Sedimentation Coefficient"));
   rb_y_ff0->setToolTip(tr("Set Y axis to Frictional Ratio"));
   rb_y_mw->setToolTip(tr("Set Y axis to Molecular Weight"));
   rb_y_vbar->setToolTip(tr("Set Y axis to Partial Specific Volume"));
   rb_y_D->setToolTip(tr("Set Y axis to Diffusion Coefficient"));
   rb_y_f->setToolTip(tr("Set Y axis to Frictional Coefficient"));
   connect(bg_x_axis, SIGNAL(buttonReleased(int)), this, SLOT(select_x_axis(int)));
   connect(bg_y_axis, SIGNAL(buttonReleased(int)), this, SLOT(select_y_axis(int)));

   dkdb_cntrls = new US_Disk_DB_Controls(US_Settings::default_data_location());
   connect(dkdb_cntrls, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));

   pb_prefilt = us_pushbutton(tr("Select PreFilter"));
   connect(pb_prefilt, SIGNAL(clicked()), this, SLOT(select_prefilt()));

   le_prefilt = us_lineedit("", -1, true);

   pb_lddistr = us_pushbutton(tr("Load Distribution"));
   connect(pb_lddistr, SIGNAL(clicked()), this, SLOT(load_distro()));

   us_checkbox(tr("1-Dimensional Plot"), ck_1dplot, false);
   connect(ck_1dplot, SIGNAL(clicked()), this, SLOT(select_plot1d()));

   pb_ldcolor = us_pushbutton(tr("Load Color File"));
   connect(pb_ldcolor, SIGNAL(clicked()), this, SLOT(load_color()));

   us_checkbox(tr("2-Dimensional Plot"), ck_2dplot, false);
   connect(ck_2dplot, SIGNAL(clicked()), this, SLOT(select_plot2d()));

   pb_refresh = us_pushbutton(tr("Refresh Plot"), false);
   connect(pb_refresh, SIGNAL(clicked()), this, SLOT(replot_data()));

   us_checkbox(tr("Pseudo 3-D Plot"), ck_3dplot, true);
   connect(ck_3dplot, SIGNAL(clicked()), this, SLOT(select_plot3d()));

   pb_mandrsb = us_pushbutton(tr("Manually Draw Bins"), false);
   connect(pb_mandrsb, SIGNAL(clicked()), this, SLOT(manDrawSb()));

   pb_ckovrlp = us_pushbutton(tr("Check for Bin Overlaps / Sort"), false);
   connect(pb_ckovrlp, SIGNAL(clicked()), this, SLOT(checkOverlaps()));

   pb_autassb = us_pushbutton(tr("Autoassign Solute Bins"), false);
   connect(pb_autassb, SIGNAL(clicked()), this, SLOT(autoAssignSb()));

   pb_resetsb = us_pushbutton(tr("Reset Solute Bins"), false);
   connect(pb_resetsb, SIGNAL(clicked()), this, SLOT(resetSb()));

   pb_reset = us_pushbutton(tr("Reset"), false);
   connect(pb_reset, SIGNAL(clicked()), this, SLOT(reset()));

   pb_view = us_pushbutton(tr("View Statistics"), false);
   connect(pb_view, SIGNAL(clicked()), this, SLOT(view()));

   pb_save = us_pushbutton(tr("Save"), false);
   connect(pb_save, SIGNAL(clicked()), this, SLOT(save()));

   pb_loadsb = us_pushbutton(tr("Load Solute Bins"), false);
   connect(pb_loadsb, SIGNAL(clicked()), this, SLOT(load_bins()));

   pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));

   pb_close = us_pushbutton(tr("Close"));
   connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));

   te_status = us_textedit();
   te_status->setTextBackgroundColor(Qt::white);
   te_status->setTextColor(Qt::blue);
   dfilname = "(NONE)";
   stcmline = tr("Color Map:  the default w-cyan-magenta-red-black");
   stdiline = tr("The distribution was loaded from the file:");
   stdfline = "  " + dfilname;
   stfxline = tr("Components fixed attribute is \"vbar\".");
   stnpline = tr("The number of distribution points is 0.");
   te_status->setText(stcmline + "\n" + stdiline + "\n" + stdfline + "\n" + stfxline + "\n" + stnpline);

   int s_row = 0;
   spec->addWidget(lb_info1, s_row++, 0, 1, 8);
   spec->addWidget(lb_nisols, s_row, 0, 1, 4);
   spec->addWidget(ct_nisols, s_row++, 4, 1, 4);
   spec->addWidget(lb_wxbuck, s_row, 0, 1, 4);
   spec->addWidget(ct_wxbuck, s_row++, 4, 1, 4);
   spec->addWidget(lb_hybuck, s_row, 0, 1, 4);
   spec->addWidget(ct_hybuck, s_row++, 4, 1, 4);
   spec->addWidget(lb_info2, s_row++, 0, 1, 8);
   spec->addWidget(lb_resolu, s_row, 0, 1, 4);
   spec->addWidget(ct_resolu, s_row++, 4, 1, 4);
   spec->addWidget(lb_xreso, s_row, 0, 1, 4);
   spec->addWidget(ct_xreso, s_row++, 4, 1, 4);
   spec->addWidget(lb_yreso, s_row, 0, 1, 4);
   spec->addWidget(ct_yreso, s_row++, 4, 1, 4);
   spec->addWidget(lb_zfloor, s_row, 0, 1, 4);
   spec->addWidget(ct_zfloor, s_row++, 4, 1, 4);
   spec->addWidget(lb_autlim, s_row, 0, 1, 4);
   spec->addWidget(ck_autlim, s_row++, 4, 1, 4);
   spec->addWidget(lb_plxmin, s_row, 0, 1, 4);
   spec->addWidget(ct_plxmin, s_row++, 4, 1, 4);
   spec->addWidget(lb_plxmax, s_row, 0, 1, 4);
   spec->addWidget(ct_plxmax, s_row++, 4, 1, 4);
   spec->addWidget(lb_plymin, s_row, 0, 1, 4);
   spec->addWidget(ct_plymin, s_row++, 4, 1, 4);
   spec->addWidget(lb_plymax, s_row, 0, 1, 4);
   spec->addWidget(ct_plymax, s_row++, 4, 1, 4);
   spec->addWidget(lw_sbin_data, s_row++, 0, 1, 8);
   spec->addWidget(lb_x_axis, s_row, 0, 1, 2);
   spec->addLayout(gl_x_s, s_row, 2, 1, 1);
   spec->addLayout(gl_x_ff0, s_row, 3, 1, 1);
   spec->addLayout(gl_x_mw, s_row, 4, 1, 1);
   spec->addLayout(gl_x_vbar, s_row, 5, 1, 1);
   spec->addLayout(gl_x_D, s_row, 6, 1, 1);
   spec->addLayout(gl_x_f, s_row++, 7, 1, 1);
   spec->addWidget(lb_y_axis, s_row, 0, 1, 2);
   spec->addLayout(gl_y_s, s_row, 2, 1, 1);
   spec->addLayout(gl_y_ff0, s_row, 3, 1, 1);
   spec->addLayout(gl_y_mw, s_row, 4, 1, 1);
   spec->addLayout(gl_y_vbar, s_row, 5, 1, 1);
   spec->addLayout(gl_y_D, s_row, 6, 1, 1);
   spec->addLayout(gl_y_f, s_row++, 7, 1, 1);
   spec->addLayout(dkdb_cntrls, s_row++, 0, 1, 8);
   spec->addWidget(pb_prefilt, s_row, 0, 1, 4);
   spec->addWidget(le_prefilt, s_row++, 4, 1, 4);
   spec->addWidget(pb_lddistr, s_row, 0, 1, 4);
   spec->addWidget(ck_1dplot, s_row++, 4, 1, 4);
   spec->addWidget(pb_ldcolor, s_row, 0, 1, 4);
   spec->addWidget(ck_2dplot, s_row++, 4, 1, 4);
   spec->addWidget(pb_refresh, s_row, 0, 1, 4);
   spec->addWidget(ck_3dplot, s_row++, 4, 1, 4);
   spec->addWidget(pb_mandrsb, s_row, 0, 1, 4);
   spec->addWidget(pb_ckovrlp, s_row++, 4, 1, 4);
   spec->addWidget(pb_autassb, s_row, 0, 1, 4);
   spec->addWidget(pb_resetsb, s_row++, 4, 1, 4);
   spec->addWidget(pb_loadsb, s_row, 0, 1, 4);
   spec->addWidget(pb_reset, s_row, 4, 1, 2);
   spec->addWidget(pb_save, s_row++, 6, 1, 2);
   spec->addWidget(pb_view, s_row, 0, 1, 4);
   spec->addWidget(pb_help, s_row, 4, 1, 2);
   spec->addWidget(pb_close, s_row++, 6, 1, 2);
   spec->addWidget(te_status, s_row++, 0, 6, 8);

   // set up plot component window on right side
   xa_title = tr("Sedimentation Coefficient corrected for water at 20") + DEGC;
   ya_title = tr("Frictional Ratio f/f0");

   QBoxLayout *plot = new US_Plot(data_plot, tr("Distribution Data"), xa_title, ya_title);
   rght->addLayout(plot);
   QBoxLayout *txed = new QHBoxLayout;
   te_pctl_help = us_textedit();
   te_pctl_help->setText(tr("Please load a sedimentation coefficient or molecular weight"
                            " distribution to initialize the genetic algorithm s-value or mw-value"
                            " range. The distribution should have a good resolution over the"
                            " sedimentation or weight coefficients. This distribution will be "
                            " used to initialize all experiments used in the run, so the"
                            " distribution taken from the experiment with the highest speed is"
                            " probably the most appropriate distribution. You can use a distribution"
                            " from the van Holde - Weischet method, the C(s) method, or 2-D Spectrum"
                            " Analysis. You may also load a Monte Carlo distribution."));
   us_setReadOnly(te_pctl_help, true);
   txed->addWidget(te_pctl_help);
   rght->addLayout(txed);
   rght->setStretchFactor(plot, 4);
   rght->setStretchFactor(txed, 1);

   data_plot->setAutoDelete(true);
   data_plot->setMinimumSize(600, 600);

   data_plot->enableAxis(QwtPlot::xBottom, true);
   data_plot->enableAxis(QwtPlot::yLeft, true);
   data_plot->enableAxis(QwtPlot::yRight, true);
   data_plot->setAxisScale(QwtPlot::xBottom, 1.0, 40.0);
   data_plot->setAxisScale(QwtPlot::yLeft, 1.0, 4.0);

   data_plot->setCanvasBackground(Qt::white);

   // put layouts together for overall layout
   left->addLayout(spec);

   main->addLayout(left);
   main->addLayout(rght);
   main->setStretchFactor(left, 3);
   main->setStretchFactor(spec, 3);
   main->setStretchFactor(rght, 5);

   // set up variables and initial state of GUI
   soludata = new US_SoluteData();
   sdistro = &xy_distro;
   plot_dim = 3; // default plot dimension
   attr_x = 0; // default X type: s
   attr_y = 1; // default Y type: f/f0
   attr_z = 3; // default Z (fixed) type:  vbar
   mfilter = ""; // default model list filter
   runsel = true; // default prefilter type
   latest = true; // default edit prefilter type
   is_saved = false; // files have been saved
   pfilts.clear(); // default prefilter edits list
   binfpath = US_Settings::resultDir();
   pick = NULL;
   pickpen = NULL;

   reset();
}

// reset the GUI
void US_GA_Initialize::reset(void) {
   dataPlotClear(data_plot);
   data_plot->replot();
   if (pick != NULL)
      delete pick;
   pick = new US_PlotPicker(data_plot);

   lw_sbin_data->clear();
   soludata->clearBuckets();
   sxset = 0;

   minmax = false;
   zoom = false;
   ck_1dplot->setChecked(plot_dim == 1);
   ck_2dplot->setChecked(plot_dim == 2);
   ck_3dplot->setChecked(plot_dim == 3);

   nisols = 0;
   nibuks = 0;
   wxbuck = 0.0;
   hybuck = 0.0;
   ct_nisols->setValue(( double ) nisols);
   ct_wxbuck->setRange(0, 200);
   ct_hybuck->setRange(0, 50);
   ct_wxbuck->setSingleStep(0.1);
   ct_hybuck->setSingleStep(0.01);
   ct_wxbuck->setValue(wxbuck);
   ct_hybuck->setValue(hybuck);

   resolu = 90.0;
   ct_resolu->setRange(1, 100);
   ct_resolu->setSingleStep(1);
   ct_resolu->setValue(resolu);

   xreso = 300.0;
   yreso = 300.0;
   ct_xreso->setRange(10.0, 1000.0);
   ct_xreso->setSingleStep(1.0);
   ct_xreso->setValue(( double ) xreso);
   ct_yreso->setRange(10, 1000);
   ct_yreso->setSingleStep(1.0);
   ct_yreso->setValue(( double ) yreso);

   zfloor = 100.0;
   ct_zfloor->setRange(50, 150);
   ct_zfloor->setSingleStep(1.0);
   ct_zfloor->setValue(( double ) zfloor);

   auto_lim = true;
   ck_autlim->setChecked(auto_lim);

   plymin = 1.0;
   plymax = 4.0;
   ct_plymin->setRange(0.5, 50);
   ct_plymin->setSingleStep(0.01);
   ct_plymin->setValue(plymin);
   ct_plymin->setEnabled(false);
   ct_plymax->setRange(1.0, 50);
   ct_plymax->setSingleStep(0.01);
   ct_plymax->setValue(plymax);
   ct_plymax->setEnabled(false);

   plxmin = 1.0;
   plxmax = 10.0;
   ct_plxmin->setRange(-10000.0, 10000.0);
   ct_plxmin->setSingleStep(0.01);
   ct_plxmin->setValue(plxmin);
   ct_plxmin->setEnabled(false);
   ct_plxmax->setRange(0.0, 10000.0);
   ct_plxmax->setSingleStep(0.01);
   ct_plxmax->setValue(plxmax);
   ct_plxmax->setEnabled(false);

   // default to white-cyan-magenta-red-black color map
   colormap = new QwtLinearColorMap(Qt::white, Qt::black);
   colormap->addColorStop(0.10, Qt::cyan);
   colormap->addColorStop(0.50, Qt::magenta);
   colormap->addColorStop(0.80, Qt::red);
   cmapname = tr("Default Color Map: w-cyan-magenta-red-black");

   monte_carlo = false;
   is_saved = false;
   pb_reset->setEnabled(false);
}

// save the GA data
void US_GA_Initialize::save(void) {
   // Sort the buckets in increasing-s order
   int hlx = lw_sbin_data->currentRow();
   soludata->sortBuckets();

   // Reset the buckets plot and list
   resetPlotAndList(hlx);

   // Test for overlaps
   int novlps = soludata->countOverlaps();
   DbgLv(1) << "SAVE novlps" << novlps;

   if (novlps > 0) {
      QString msg = (novlps == 1) ? tr("There is one case of overlap between bins.\n")
                                  : tr("%1 pairs of bins overlap one another.\n").arg(novlps);
      QMessageBox::warning(
         this, tr("Bin Overlaps"),
         msg
            + tr("You must correct this condition so that\n"
                 "no bins overlap, before you can save GA data."));
      return;
   }

   if (attr_x != ATTR_V && attr_y != ATTR_V && attr_z != ATTR_V) { // None of X,Y,fixed is vbar:  there can be no output file
      QMessageBox::information(
         this, tr("No Files Output"),
         tr("No files will be saved, since neither X,Y nor fixed attribute"
            " is vbar.\n\n"
            "To output a gadistro file, insure that one of X,Y,fixed"
            " is vbar."));
      return;
   }

   QString runid = run_name.section(".", 0, -2);
   if (runid.startsWith("Global-"))
      runid = runid.mid(7);
   QString trpid = run_name.section(".", -1, -1);
   QString fdir = US_Settings::resultDir() + "/" + runid;
   QString fndat = "gainit." + trpid + ".gadistro.dat";
   QString fnsta = "gainit." + trpid + ".sol_integ.stats";
   QString fname = fdir + "/" + fndat;
   QString fdir2 = US_Settings::reportDir() + "/" + runid;
   QString fnst2 = method + "." + trpid + ".statistics.rpt";
   QString fnam2 = fdir2 + "/" + fnst2;

   QDir dirp(US_Settings::resultDir());

   if (!dirp.exists(fdir))
      dirp.mkpath(fdir);

   if (!dirp.exists(fdir2))
      dirp.mkpath(fdir2);

   DbgLv(1) << "SAVE ax,y,z" << attr_x << attr_y << attr_z;
   double fixval = 0.0;
   fixval = (attr_z == ATTR_S) ? sk_distro[ 0 ].s : fixval;
   fixval = (attr_z == ATTR_K) ? sk_distro[ 0 ].k : fixval;
   fixval = (attr_z == ATTR_W) ? sk_distro[ 0 ].w : fixval;
   fixval = (attr_z == ATTR_D) ? sk_distro[ 0 ].d : fixval;
   fixval = (attr_z == ATTR_F) ? sk_distro[ 0 ].f : fixval;

   soludata->saveGAdata(fname, attr_x, attr_y, attr_z, fixval);

   // Report on files saved
   QString msg = tr("Saved:\n    ") + fndat + "\n";

   if (manbuks) { // if manual buckets, build up and analyze data, then report

      DbgLv(1) << "SAVE call buildMC";
      soludata->buildDataMC(); // build it

      fname = fdir + "/" + fnsta;

      DbgLv(1) << "SAVE call reportMC";
      soludata->reportDataMC(fname, mc_iters); // report it
      is_saved = true;

      // Copy the statistics file to the report directory
      QFile(fnam2).remove();
      QFile(fname).copy(fnam2);
      pb_view->setEnabled(true);

      msg += "    " + fnsta + "\n";
      msg += tr("in directory:\n    ") + fdir + tr("\n\nand\n");
      msg += "    " + fnst2 + "\n";
      msg += tr("in directory:\n    ") + fdir2;

      if (dkdb_cntrls->db()) { // Write statistics report to database
         US_Passwd pw;
         US_DB2 db(pw.getPasswd());
         US_DB2 *dbP = &db;
         QStringList query;

         query << "get_editID" << editGUID;
         db.query(query);
         db.next();
         int idEdit = db.value(0).toString().toInt();
         US_Report freport;
         freport.runID = runid;

         int status = freport.saveDocumentFromFile(fdir2, fnst2, dbP, idEdit);

         if (status == US_Report::REPORT_OK)
            msg += tr("\n\nThe report file was also saved to the database");
         else
            msg += tr("\n\n*ERROR* attempting save of report to database.");
      }
   }

   else {
      DbgLv(1) << "SAVE fnst2" << fnst2;
      msg += tr("in directory:\n    ") + fdir;
   }

   QMessageBox::information(this, tr("Distro/Stats File Save"), msg);
}

// Manually draw solute bins
void US_GA_Initialize::manDrawSb(void) {
   QColor cblack(Qt::black);
   QColor cwhite(Qt::white);

   // create a new plot picker to draw rectangles around solute points
   if (pick != NULL)
      delete pick;
   if (pickpen != NULL)
      delete pickpen;
   pick = new US_PlotPicker(data_plot);

   // make sure rubber band and tracker show up against background
   QColor bg = data_plot->canvasBackground();
   int csum = bg.red() + bg.green() + bg.blue();
   pickpen = new QPen((csum > 600) ? cblack : cwhite);
   DbgLv(1) << "manDr: bg" << bg << "csum" << csum;
   DbgLv(1) << "manDr:   pickpen" << pickpen->brush().color();
   DbgLv(1) << "manDr:    cblack" << cblack << "cwhite" << cwhite;

   pick->setStateMachine(new QwtPickerDragRectMachine());
   pick->setRubberBandPen(*pickpen);
   pick->setTrackerPen(*pickpen);
   pick->setRubberBand(QwtPicker::RectRubberBand);

   // set up to capture position and dimensions of solute bin
   connect(pick, SIGNAL(mouseDown(const QwtDoublePoint &)), this, SLOT(getMouseDown(const QwtDoublePoint &)));
   connect(pick, SIGNAL(mouseUp(const QwtDoublePoint &)), this, SLOT(getMouseUp(const QwtDoublePoint &)));

   pb_ckovrlp->setEnabled(false);

   //wxbuck       = ( plxmax - plxmin ) / 10.0;
   //hybuck       = ( plymax - plymin ) / 10.0;
   wxbuck = (plxmax - plxmin) * 0.08;
   hybuck = (plymax - plymin) * 0.2;
   ct_wxbuck->disconnect();
   ct_hybuck->disconnect();
   int rpwr = qRound(log10(wxbuck));
   double rmax = pow(10.0, rpwr + 3);
   double rinc = pow(10.0, rpwr - 3);
   wxbuck = qRound(wxbuck / rinc) * rinc;
   ct_wxbuck->setRange(0.0, rmax);
   ct_wxbuck->setSingleStep(rinc);
   ct_wxbuck->setValue(wxbuck);
   rpwr = qRound(log10(hybuck));
   rmax = pow(10.0, rpwr + 3);
   rinc = pow(10.0, rpwr - 3);
   hybuck = qRound(hybuck / rinc) * rinc;
   ct_hybuck->setValue(hybuck);
   connect(ct_wxbuck, SIGNAL(valueChanged(double)), this, SLOT(update_wxbuck(double)));
   connect(ct_hybuck, SIGNAL(valueChanged(double)), this, SLOT(update_hybuck(double)));

   manbuks = true;
}

// Check for bin overlaps
void US_GA_Initialize::checkOverlaps(void) {
   int novlps = soludata->countOverlaps();

   if (novlps == 0) {
      QMessageBox::information(
         this, tr("No Bins Overlap"),
         tr("No bin overlaps were found, so you"
            " may proceed to saving this GA data"
            " or viewing statistics."
            "\n\n*NOTE*: Bins will now be re-sorted."));
      pb_save->setEnabled(attr_x == ATTR_V || attr_y == ATTR_V || attr_z == ATTR_V);
      pb_view->setEnabled(true);
   }

   else {
      QString msg = (novlps == 1) ? tr("There is one case of overlap between bins.\n")
                                  : tr("%1 pairs of bins overlap one another.\n").arg(novlps);
      QMessageBox::warning(
         this, tr("Bin Overlaps"),
         msg
            + tr("You must correct this condition so that no\n"
                 "bins overlap, before you can save GA data."));
      pb_save->setEnabled(false);
      pb_view->setEnabled(false);
   }

   // Sort buckets, then reset the buckets plot and list
   soludata->sortBuckets();
   int hlx = lw_sbin_data->currentRow();
   resetPlotAndList(hlx);
}

// Auto assign solute bins
void US_GA_Initialize::autoAssignSb(void) {
   nisols = (nisols == 0) ? sdistro->size() : nisols;
   pc1 = NULL;
   lw_sbin_data->clear();
   soludata->clearBuckets();
   erase_buckets(true);

   nibuks = soludata->autoCalcBins(nisols, wxbuck, hybuck);

   for (int jj = 0; jj < nibuks; jj++) { // draw the auto-assigned buckets and add lines to list widget
      QRectF rect = soludata->bucketRect(jj);
      pc1 = drawBucketRect(jj, rect);

      lw_sbin_data->addItem(soludata->bucketLine(jj));
   }

   data_plot->replot();
   pb_resetsb->setEnabled(true);
   // X,Y or fixed must be vbar for Save to be allowed
   pb_save->setEnabled((attr_x == ATTR_V) || (attr_y == ATTR_V) || (attr_z == ATTR_V));
   pb_ckovrlp->setEnabled(true);
   pb_view->setEnabled(false);

   manbuks = false;
}

// Reset solute bins
void US_GA_Initialize::resetSb(void) {
   ct_nisols->setValue(0.0);
   lw_sbin_data->clear(); // clear solute bucket data
   soludata->clearBuckets();
   sxset = 0;
   is_saved = false;

   erase_buckets(true); // erase bucket rectangles from plot and delete

   nibuks = 0;
   pb_save->setEnabled(false);
   pb_view->setEnabled(false);
   pb_ckovrlp->setEnabled(false);

   data_plot->replot();
}

// (re)plot data
void US_GA_Initialize::replot_data() {
   if (sdistro->isEmpty() || sdistro->size() == 0)
      return;

   resetSb();

   sdistro = &xy_distro;

   if (plot_dim == 1) {
      plot_1dim();
   }
   else if (plot_dim == 2) {
      plot_2dim();
   }
   else {
      plot_3dim();
   }

   setBucketPens();
}

// plot data 1-D
void US_GA_Initialize::plot_1dim(void) {
   dataPlotClear(data_plot);
   data_plot->replot();
   if (pick != NULL)
      delete pick;
   pick = new US_PlotPicker(data_plot);

   data_plot->setCanvasBackground(Qt::black);
   pick->setTrackerPen(QColor(Qt::white));

   int dsize = sdistro->size();
   QVector<double> xvec(dsize);
   QVector<double> yvec(dsize);
   double *x = xvec.data();
   double *y = yvec.data();
   double sval = sdistro->at(0).s;
   double smin = sval;
   double smax = sval;
   double cval = sdistro->at(0).c;
   double cmin = cval;
   double cmax = cval;
   int nn = 1;
   x[ 0 ] = sval;
   y[ 0 ] = cval;

   for (int jj = 1; jj < dsize; jj++) {
      double svpr = sval;
      double cvpr = cval;
      sval = sdistro->at(jj).s;
      cval = sdistro->at(jj).c;

      if (equivalent(sval, svpr, epsilon)) { // effectively equal s values: sum c values
         cval += cvpr;
         x[ nn - 1 ] = (svpr + sval) * 0.5;
         y[ nn - 1 ] = cval;
      }

      else { // new s value:  save c value and bump count
         x[ nn ] = sval;
         y[ nn++ ] = cval;
      }

      smin = (smin < sval) ? smin : sval;
      smax = (smax > sval) ? smax : sval;
      cmin = (cmin < cval) ? cmin : cval;
      cmax = (cmax > cval) ? cmax : cval;
   }

   if (dsize == 1) {
      smin *= ((smin < 0) ? 1.05 : 0.95);
      smax *= ((smin < 0) ? 0.95 : 1.05);
      cmin *= 0.95;
      cmax *= 1.05;
   }

   double rdif = (smax - smin) / 20.0;
   smin -= rdif;
   smax += rdif;
   rdif = (cmax - cmin) / 20.0;
   cmin -= rdif;
   cmax += rdif;
   cmin = (cmin > 0.0) ? cmin : 0.0;

   QwtPlotGrid *data_grid = us_grid(data_plot);
   data_grid->enableYMin(true);
   data_grid->enableY(true);
   data_grid->setMajorPen(QPen(US_GuiSettings::plotMajGrid(), 0, Qt::DashLine));
   data_grid->attach(data_plot);

   QwtPlotCurve *data_curv = us_curve(data_plot, "distro");
   data_curv->setSamples(x, y, nn);
   data_curv->setPen(QPen(Qt::yellow, 3, Qt::SolidLine));
   data_curv->setStyle(QwtPlotCurve::Sticks);

   data_plot->setAxisAutoScale(QwtPlot::xBottom);
   data_plot->setAxisAutoScale(QwtPlot::yLeft);
   data_plot->enableAxis(QwtPlot::yRight, false);
   data_plot->setAxisTitle(QwtPlot::xBottom, xa_title);
   data_plot->setAxisTitle(QwtPlot::yLeft, tr("Partial Concentration"));
   data_plot->setAxisScale(QwtPlot::xBottom, smin, smax);
   data_plot->setAxisScale(QwtPlot::yLeft, cmin, cmax);

   data_plot->replot();

   pb_reset->setEnabled(true);
   pb_autassb->setEnabled(false);
   manbuks = true;
}

// plot data 2-D
void US_GA_Initialize::plot_2dim(void) {
   dataPlotClear(data_plot);
   data_plot->replot();
   if (pick != NULL)
      delete pick;
   pick = new US_PlotPicker(data_plot);

   data_plot->setCanvasBackground(Qt::black);
   pick->setTrackerPen(QColor(Qt::white));

   int dsize = sdistro->size();
   double *x = new double[ dsize ];
   double *y = new double[ dsize ];
   double smin = 1.0e30;
   double smax = -1.0e30;
   double kmin = 1.0e30;
   double kmax = -1.0e30;

   for (int jj = 0; jj < dsize; jj++) {
      double sval = sdistro->at(jj).s;
      double kval = sdistro->at(jj).k;
      x[ jj ] = sval;
      y[ jj ] = kval;
      smin = qMin(smin, sval);
      smax = qMax(smax, sval);
      kmin = qMin(kmin, kval);
      kmax = qMax(kmax, kval);
   }

   if (dsize == 1) {
      smin *= 0.95;
      smax *= 1.05;
      kmin *= 0.95;
      kmax *= 1.05;
   }

   double rdif = (smax - smin) / 20.0;
   smin -= rdif;
   smax += rdif;
   rdif = (kmax - kmin) / 20.0;
   kmin -= rdif;
   kmax += rdif;
   kmin = (kmin > 0.0) ? kmin : 0.0;

   QwtPlotGrid *data_grid = us_grid(data_plot);
   data_grid->enableYMin(true);
   data_grid->enableY(true);
   data_grid->setMajorPen(QPen(US_GuiSettings::plotMajGrid(), 0, Qt::DashLine));
   data_grid->attach(data_plot);

   QwtPlotCurve *data_curv = us_curve(data_plot, "distro");
   QwtSymbol *symbol = new QwtSymbol;

   symbol->setStyle(QwtSymbol::Ellipse);
   symbol->setPen(QPen(Qt::red));
   symbol->setBrush(QBrush(Qt::yellow));
   if (dsize < 100 && dsize > 50)
      symbol->setSize(8);
   else if (dsize < 50 && dsize > 20)
      symbol->setSize(10);
   else if (dsize < 21)
      symbol->setSize(12);
   else if (dsize > 100)
      symbol->setSize(6);

   data_curv->setStyle(QwtPlotCurve::NoCurve);
   data_curv->setSymbol(symbol);
   data_curv->setSamples(x, y, dsize);

   delete[] x;
   delete[] y;
   data_plot->setAxisAutoScale(QwtPlot::xBottom);
   data_plot->setAxisAutoScale(QwtPlot::yLeft);
   data_plot->enableAxis(QwtPlot::yRight, false);
   data_plot->setAxisTitle(QwtPlot::xBottom, xa_title);
   data_plot->setAxisTitle(QwtPlot::yLeft, ya_title);
   data_plot->setAxisTitle(QwtPlot::yRight, tr("Partial Concentration"));
   data_plot->axisTitle(QwtPlot::yRight).setFont(data_plot->axisTitle(QwtPlot::yLeft).font());
   data_plot->setAxisScale(QwtPlot::xBottom, smin, smax);
   data_plot->setAxisScale(QwtPlot::yLeft, kmin, kmax);

   data_plot->replot();

   pb_reset->setEnabled(true);
   pb_autassb->setEnabled(!monte_carlo);
}

// plot data 3-D
void US_GA_Initialize::plot_3dim(void) {
   QColor cblack(Qt::black);
   QColor cwhite(Qt::white);
   dataPlotClear(data_plot);
   data_plot->replot();
   if (pick != NULL)
      delete pick;
   pick = new US_PlotPicker(data_plot);
   QColor bg = colormap->color1();

   data_plot->setCanvasBackground(bg);

   int csum = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen(QPen((csum > 600) ? cblack : cwhite));
   DbgLv(1) << "pl3d: bg" << bg << "csum" << csum;
   DbgLv(1) << "pl3d:   pickpen" << pick->trackerPen().brush().color();
   DbgLv(1) << "pl3d:    cblack" << cblack << "cwhite" << cwhite;

   QString tstr = run_name + "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle(tstr);

   // set up spectrogram data
   d_spectrogram = new QwtPlotSpectrogram();
#if QT_VERSION < 0x050000
   d_spectrogram->setData(US_SpectrogramData());
   spec_dat = ( US_SpectrogramData * ) &d_spectrogram->data();
   d_spectrogram->setColorMap(*colormap);
#else
   spec_dat = new US_SpectrogramData();
   d_spectrogram->setColorMap(ColorMapCopy(colormap));
   d_spectrogram->setData(spec_dat);
#endif
   d_spectrogram->attach(data_plot);

   QwtDoubleRect drect;

   if (auto_lim) {
      drect = QwtDoubleRect(0.0, 0.0, 0.0, 0.0);
   }

   else {
      drect = QwtDoubleRect(plxmin, plymin, (plxmax - plxmin), (plymax - plymin));
   }

   double zmax = sdistro->at(0).c;
   double zmin = 0.0;

   for (int jj = 0; jj < sdistro->size(); jj++) {
      zmin = qMin(zmin, sdistro->at(jj).c);
      zmax = qMax(zmax, sdistro->at(jj).c);
   }

   spec_dat->setRastRanges(xreso, yreso, resolu, zfloor, drect);
   spec_dat->setZRange(zmin, zmax);
   spec_dat->setRaster(sdistro);

   // Set color map and axis settings
   QwtScaleWidget *rightAxis = data_plot->axisWidget(QwtPlot::yRight);
   rightAxis->setColorBarEnabled(true);
#if QT_VERSION < 0x050000
   rightAxis->setColorMap(spec_dat->range(), d_spectrogram->colorMap());
#else
   rightAxis->setColorMap(spec_dat->range(), ColorMapCopy(colormap));
   d_spectrogram->setColorMap(ColorMapCopy(colormap));
#endif
   data_plot->setAxisTitle(QwtPlot::xBottom, xa_title);
   data_plot->setAxisTitle(QwtPlot::yLeft, ya_title);
   data_plot->setAxisTitle(QwtPlot::yRight, tr("Partial Concentration"));
   data_plot->axisTitle(QwtPlot::yRight).setFont(data_plot->axisTitle(QwtPlot::yLeft).font());
   data_plot->setAxisScale(QwtPlot::yRight, spec_dat->range().minValue(), spec_dat->range().maxValue());
   data_plot->enableAxis(QwtPlot::yRight);

   if (auto_lim) { // auto limits
      data_plot->setAxisAutoScale(QwtPlot::yLeft);
      data_plot->setAxisAutoScale(QwtPlot::xBottom);
   }
   else { // manual limits
      data_plot->setAxisScale(QwtPlot::xBottom, plxmin, plxmax);
      data_plot->setAxisScale(QwtPlot::yLeft, plymin, plymax);
   }

   data_plot->replot();

   pb_reset->setEnabled(true);
   pb_autassb->setEnabled(!monte_carlo);
}

// update pseudo-3d resolution factor
void US_GA_Initialize::update_resolu(double dval) {
   resolu = dval;
}

// update raster x resolution
void US_GA_Initialize::update_xreso(double dval) {
   xreso = dval;
}

// update raster y resolution
void US_GA_Initialize::update_yreso(double dval) {
   yreso = dval;
}

// update Z (frequency) floor percent
void US_GA_Initialize::update_zfloor(double dval) {
   zfloor = dval;
}

// update number of initial solutes
void US_GA_Initialize::update_nisols(double dval) {
   nisols = qRound(dval);
}

// update width in s of buckets
void US_GA_Initialize::update_wxbuck(double dval) {
   wxbuck = dval;
}

// update height in f/f0 of buckets
void US_GA_Initialize::update_hybuck(double dval) {
   hybuck = dval;
}

// update plot limit s min
void US_GA_Initialize::update_plxmin(double dval) {
   plxmin = dval;
}

// update plot limit s max
void US_GA_Initialize::update_plxmax(double dval) {
   plxmax = dval;

   if (attr_x == ATTR_W) { // For MW, use logarithmic steps
      double rinc = pow(10.0, qRound(log10(dval)) - 2.0);
      ct_plxmin->setSingleStep(rinc);
      ct_plxmax->setSingleStep(rinc);
   }
}

// update plot limit f/f0 min
void US_GA_Initialize::update_plymin(double dval) {
   plymin = dval;
}

// update plot limit f/f0 max
void US_GA_Initialize::update_plymax(double dval) {
   plymax = dval;

   if (attr_y == ATTR_W) { // For MW, use logarithmic steps
      double rinc = pow(10.0, qRound(log10(dval)) - 2.0);
      ct_plymin->setSingleStep(rinc);
      ct_plymax->setSingleStep(rinc);
   }
}

// select automatic plot limits
void US_GA_Initialize::select_autolim() {
   auto_lim = ck_autlim->isChecked();
   ct_plymin->setEnabled(!auto_lim);
   ct_plymax->setEnabled(!auto_lim);
   ct_plxmin->setEnabled(!auto_lim);
   ct_plxmax->setEnabled(!auto_lim);
   if (auto_lim) {
      set_limits();
   }

   //wxbuck       = ( plxmax - plxmin ) / 20;
   //hybuck       = ( plymax - plymin ) / 20;
   wxbuck = (plxmax - plxmin) * 0.08;
   hybuck = (plymax - plymin) * 0.2;
   double rmax = wxbuck * 10.0;
   double rinc = pow(10.0, ( double ) (( int ) (log10(rmax) - 3.0)));
   ct_wxbuck->disconnect();
   ct_wxbuck->setRange(0.0, rmax);
   ct_wxbuck->setSingleStep(rinc);
   ct_wxbuck->setValue(wxbuck);
   ct_hybuck->setValue(hybuck);
   connect(ct_wxbuck, SIGNAL(valueChanged(double)), this, SLOT(update_wxbuck(double)));
}

// select 1-dimensional plot
void US_GA_Initialize::select_plot1d() {
   plot_dim = 1;
   ck_2dplot->disconnect();
   ck_3dplot->disconnect();
   ck_2dplot->setChecked(false);
   ck_3dplot->setChecked(false);

   ck_1dplot->setEnabled(false);
   ck_2dplot->setEnabled(true);
   ck_3dplot->setEnabled(true);

   connect(ck_2dplot, SIGNAL(clicked()), this, SLOT(select_plot2d()));
   connect(ck_3dplot, SIGNAL(clicked()), this, SLOT(select_plot3d()));

   replot_data();

   pb_mandrsb->setEnabled(false);
   pb_autassb->setEnabled(false);
   manbuks = true;
}

// select 2-dimensional plot
void US_GA_Initialize::select_plot2d() {
   plot_dim = 2;
   ck_1dplot->disconnect();
   ck_3dplot->disconnect();
   ck_1dplot->setChecked(false);
   ck_3dplot->setChecked(false);

   ck_1dplot->setEnabled(true);
   ck_2dplot->setEnabled(false);
   ck_3dplot->setEnabled(true);

   connect(ck_1dplot, SIGNAL(clicked()), this, SLOT(select_plot1d()));
   connect(ck_3dplot, SIGNAL(clicked()), this, SLOT(select_plot3d()));

   replot_data();

   pb_mandrsb->setEnabled(true);
   pb_autassb->setEnabled(!monte_carlo);
}

// select 3-dimensional plot
void US_GA_Initialize::select_plot3d() {
   plot_dim = 3;
   ck_1dplot->disconnect();
   ck_2dplot->disconnect();
   ck_3dplot->disconnect();
   ck_1dplot->setChecked(false);
   ck_2dplot->setChecked(false);

   ck_1dplot->setEnabled(true);
   ck_2dplot->setEnabled(true);
   ck_3dplot->setEnabled(false);

   connect(ck_1dplot, SIGNAL(clicked()), this, SLOT(select_plot1d()));
   connect(ck_2dplot, SIGNAL(clicked()), this, SLOT(select_plot2d()));
   connect(ck_3dplot, SIGNAL(clicked()), this, SLOT(select_plot3d()));

   replot_data();

   pb_mandrsb->setEnabled(true);
   pb_autassb->setEnabled(!monte_carlo);
}

// load the solute distribution from a file or from DB
void US_GA_Initialize::load_distro() {
   S_Solute sol_sk;
   S_Solute sol_xy;
   US_Model model;
   QString mdesc;
   bool loadDB = dkdb_cntrls->db();
   double smin = 1e+39;
   double smax = 1e-39;
   double kmin = 1e+39;
   double kmax = 1e-39;
   double wmin = 1e+39;
   double wmax = 1e-39;
   double vmin = 1e+39;
   double vmax = 1e-39;
   double dmin = 1e+39;
   double dmax = 1e-39;
   double fmin = 1e+39;
   double fmax = 1e-39;

   QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
   US_ModelLoader dialog(loadDB, mfilter, model, mdesc, pfilts);
   dialog.move(this->pos() + QPoint(200, 200));

   connect(&dialog, SIGNAL(changed(bool)), this, SLOT(update_disk_db(bool)));
   QApplication::restoreOverrideCursor();

   QString mfnam;
   QString sep;
   QString aiters;

   if (dialog.exec() != QDialog::Accepted)
      return;


   DbgLv(1) << "LOAD ACCEPT  Description:\n " << mdesc;
   sep = mdesc.left(1);
   mfnam = mdesc.section(sep, 2, 2);
   aiters = mdesc.section(sep, 6, 6);

   if (mfnam.isEmpty()) { // From db:  make ID the "filename"
      mfnam = "db ID " + mdesc.section(sep, 4, 4);
   }

   else { // From disk:  use base file name
      mfnam = QFileInfo(mfnam).baseName();
   }

   if (model.components.size() < 1) {
      DbgLv(0) << "  NO Model components";
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
   run_name = mdesc.section(".", 0, -3);
   QString asys = mdesc.section(".", -2, -2);
   analys_name = asys.section("_", 0, 1) + "_" + asys.section("_", 3, 4);

   if (method == "Manual" || method == "CUSTOMGRID") {
      int jj = mdesc.indexOf(".model");
      mdesc = (jj < 1) ? mdesc : mdesc.left(jj);
      run_name = mdesc + ".0Z280";
      analys_name = "e00_a00_" + method + "_local";
   }

   monte_carlo = model.monteCarlo;
   mc_iters = monte_carlo ? aiters.toInt() : 1;
   editGUID = model.editGUID;
   DbgLv(1) << "MC" << monte_carlo << " iters" << mc_iters;

   sk_distro.clear();
   xy_distro.clear();

   resetSb();

   QString tstr = run_name + "\n" + analys_name + "\n (" + method + ")";
   data_plot->setTitle(tstr);

   // read in and set distribution s,c,k,d values
   if (model.analysis != US_Model::COFS) {
      for (int jj = 0; jj < model.components.size(); jj++) {
         US_Model::calc_coefficients(model.components[ jj ]);

         sol_sk.s = model.components[ jj ].s * 1.0e13;
         sol_sk.k = model.components[ jj ].f_f0;
         sol_sk.c = model.components[ jj ].signal_concentration;
         sol_sk.w = model.components[ jj ].mw;
         sol_sk.v = model.components[ jj ].vbar20;
         sol_sk.d = model.components[ jj ].D;
         sol_sk.f = model.components[ jj ].f;

         sol_xy = sol_sk;
         sol_xy.s = (attr_x == ATTR_S) ? sol_sk.s : sol_xy.s;
         sol_xy.s = (attr_x == ATTR_K) ? sol_sk.k : sol_xy.s;
         sol_xy.s = (attr_x == ATTR_W) ? sol_sk.w : sol_xy.s;
         sol_xy.s = (attr_x == ATTR_V) ? sol_sk.v : sol_xy.s;
         sol_xy.s = (attr_x == ATTR_D) ? sol_sk.d : sol_xy.s;
         sol_xy.s = (attr_x == ATTR_F) ? sol_sk.f : sol_xy.s;
         sol_xy.k = (attr_y == ATTR_S) ? sol_sk.s : sol_xy.k;
         sol_xy.k = (attr_y == ATTR_K) ? sol_sk.k : sol_xy.k;
         sol_xy.k = (attr_y == ATTR_W) ? sol_sk.w : sol_xy.k;
         sol_xy.k = (attr_y == ATTR_V) ? sol_sk.v : sol_xy.k;
         sol_xy.k = (attr_y == ATTR_D) ? sol_sk.d : sol_xy.k;
         sol_xy.k = (attr_y == ATTR_F) ? sol_sk.f : sol_xy.k;
         sol_xy.si = sol_sk.s;
         sol_xy.ki = sol_sk.k;

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
         //DbgLv(2) << "Solute jj s w k c d" << jj << sol_s.s << sol_w.s << sol_s.k
         //   << sol_s.c << sol_s.d << " vb" << model.components[jj].vbar20;

         sk_distro << sol_sk;
         xy_distro << sol_xy;
      }

      // sort and reduce distributions
      sdistro = &xy_distro;
      psdsiz = sdistro->size();
      sort_distro(sk_distro, false);
      sort_distro(xy_distro, true);
      DbgLv(1) << "Solute psdsiz sdsiz xdsiz" << psdsiz << sk_distro.size() << xy_distro.size();
      for (int jj = 0; jj < sk_distro.size(); jj++) {
         DbgLv(2) << " jj" << jj << " s k" << sk_distro[ jj ].s << sk_distro[ jj ].k << " w v" << sk_distro[ jj ].w
                  << sk_distro[ jj ].v;
      }
   }

   // Determine which attribute is fixed
   if (equivalent(vmin, vmax, 0.001))
      attr_z = ATTR_V;
   else if (equivalent(kmin, kmax, 0.001))
      attr_z = ATTR_K;
   else if (equivalent(smin, smax, 0.001))
      attr_z = ATTR_S;
   else if (equivalent(wmin, wmax, 0.001))
      attr_z = ATTR_W;
   else if (equivalent(dmin, dmax, 0.001))
      attr_z = ATTR_D;
   else if (equivalent(fmin, fmax, 0.001))
      attr_z = ATTR_F;

   if (attr_x != ATTR_V && attr_y != ATTR_V && attr_z != ATTR_V) { // No attribute is vbar, so use vbar as the default X or Y
      int attrv = ATTR_V;

      if (attr_y == attr_z) {
         rb_y_vbar->setChecked(true);
         select_y_axis(attrv);
      }
      else {
         rb_x_vbar->setChecked(true);
         select_x_axis(attrv);
      }
   }

   QStringList attrs;
   attrs << "s" << "f/f0" << "MW" << "vbar" << "D" << "f";
   QString s_attr = attrs[ attr_z ];

   if (auto_lim) {
      set_limits();

      ct_plymin->setEnabled(false);
      ct_plymax->setEnabled(false);
      ct_plxmin->setEnabled(false);
      ct_plxmax->setEnabled(false);
   }
   else {
      plxmin = ct_plxmin->value();
      plxmax = ct_plxmax->value();
      plymin = ct_plymin->value();
      plymax = ct_plymax->value();
   }
   data_plot->setAxisScale(QwtPlot::xBottom, plxmin, plxmax);
   data_plot->setAxisScale(QwtPlot::yLeft, plymin, plymax);

   pb_resetsb->setEnabled(true);

   nisols = sdistro->size();
   s_attr = "\"" + s_attr + "\".";
   dfilname = "(" + mfnam + ") " + mdesc;
   stdfline = "  " + dfilname;
   stfxline = tr("The components fixed attribute is ") + s_attr;
   stnpline = tr("The number of distribution points is %1").arg(nisols);
   if (nisols != psdsiz)
      stnpline += tr("\n  (reduced from %1)").arg(psdsiz);

   te_status->setText(stcmline + "\n" + stdiline + "\n" + stdfline + "\n" + stfxline + "\n" + stnpline + ".");

   replot_data();

   soludata->setDistro(sdistro, attr_x, attr_y, attr_z);

   nibuks = 0;
   //wxbuck       = ( plxmax - plxmin ) / 20.0;
   //hybuck       = ( plymax - plymin ) / 20.0;
   wxbuck = (plxmax - plxmin) * 0.08;
   hybuck = (plymax - plymin) * 0.2;
   ct_wxbuck->setValue(wxbuck);
   ct_hybuck->setValue(hybuck);
   ct_nisols->setValue(double(nisols));
   ct_wxbuck->setEnabled(true);
   ct_hybuck->setEnabled(true);
   pb_refresh->setEnabled(true);
   pb_mandrsb->setEnabled(plot_dim != 1);
   pb_loadsb->setEnabled(true);

   if (monte_carlo) {
      manDrawSb();
   }
}

// load the color map from a file
void US_GA_Initialize::load_color() {
   QString filter = tr("Color Map files (*cm-*.xml);;") + tr("Any XML files (*.xml);;") + tr("Any files (*)");

   // get an xml file name for the color map
   QString fname = QFileDialog::getOpenFileName(this, tr("Load Color Map File"), US_Settings::etcDir(), filter, 0, 0);

   if (fname.isEmpty())
      return;

   // get the map from the file
   QList<QColor> cmcolor;
   QList<double> cmvalue;

   US_ColorGradIO::read_color_steps(fname, cmcolor, cmvalue);
   colormap = new QwtLinearColorMap(cmcolor.first(), cmcolor.last());

   for (int jj = 1; jj < cmvalue.size() - 1; jj++) {
      colormap->addColorStop(cmvalue.at(jj), cmcolor.at(jj));
   }
   QFileInfo fi(fname);
   cmapname = tr("Color Map: ") + fi.baseName();

   stcmline = cmapname;
   te_status->setText(stcmline + "\n" + stdiline + "\n" + stdfline + "\n" + stfxline + "\n" + stnpline);

   replot_data();
}

// set plot x,y limits
void US_GA_Initialize::set_limits() {
   double smin = 1.0e30;
   double smax = -1.0e30;
   double kmin = 1.0e30;
   double kmax = -1.0e30;
   double sinc;
   double kinc;

   resetSb();
   sdistro = &xy_distro;

   soludata->setDistro(sdistro, attr_x, attr_y, attr_z);

   data_plot->setAxisTitle(QwtPlot::xBottom, xa_title);
   data_plot->setAxisTitle(QwtPlot::yLeft, ya_title);

   // find min,max for S distributions
   for (int jj = 0; jj < sdistro->size(); jj++) {
      double sval = sdistro->at(jj).s;
      double kval = sdistro->at(jj).k;
      smin = qMin(smin, sval);
      smax = qMax(smax, sval);
      kmin = qMin(kmin, kval);
      kmax = qMax(kmax, kval);
   }
   DbgLv(1) << "SL: distr skmin,max" << smin << smax << kmin << kmax;

   // adjust minima, maxima
   sinc = (smax - smin) / 10.0;
   kinc = (kmax - kmin) / 10.0;
   DbgLv(1) << "SL: adj sinc kinc" << sinc << kinc;
   sinc = (sinc == 0.0 || sdistro->size() == 1) ? (smin * 0.05) : sinc;
   kinc = (kinc == 0.0 || sdistro->size() == 1) ? (kmin * 0.05) : kinc;
   DbgLv(1) << "SL:  adj sinc kinc" << sinc << kinc;

   smin -= sinc;
   smax += sinc;
   kmin -= kinc;
   kmax += kinc;
   DbgLv(1) << "SL: adj smin,max" << smin << smax << "kmin,max" << kmin << kmax;

   if (auto_lim) { // Set auto limits
      sinc = pow(10.0, qFloor(log10(smax)) - 3.0);
      kinc = pow(10.0, qFloor(log10(kmax)) - 3.0);
      DbgLv(1) << "SL: aut min,inc" << smax << sinc << kmax << kinc;

      // Make x,y limits multiples of reasonable values
      if (equivalent(smin, smax, 0.001)) {
         smin -= sinc;
         smax += sinc;
      }
      if (equivalent(kmin, kmax, 0.001)) {
         kmin -= kinc;
         kmax += kinc;
      }
      smin = qFloor(smin / sinc) * sinc;
      smin = (smin < 0.0) ? (smin - sinc) : smin;
      smax = qFloor(smax / sinc) * sinc + sinc;
      smin = (attr_x != ATTR_S) ? qMax(smin, 0.0) : smin;
      smin = (attr_x == ATTR_K) ? qMax(smin, 0.8) : smin;
      kmin = qFloor(kmin / kinc) * kinc;
      kmax = qFloor(kmax / kinc) * kinc + kinc;
      kmin = (attr_y != ATTR_S) ? qMax(kmin, 0.0) : kmin;
      kmin = (attr_y == ATTR_K) ? qMax(kmin, 0.8) : kmin;
      DbgLv(1) << "SL: auto smin,max,inc" << smin << smax << sinc << "kmin,max,inc" << kmin << kmax << kinc;

      ct_plxmin->setValue(smin);
      ct_plxmax->setValue(smax);
      ct_plymin->setValue(kmin);
      ct_plymax->setValue(kmax);

      plxmin = smin;
      plxmax = smax;
      plymin = kmin;
      plymax = kmax;
   }
   else {
      plxmin = ct_plxmin->value();
      plxmax = ct_plxmax->value();
      plymin = ct_plymin->value();
      plymax = ct_plymax->value();
   }

   // Set bucket width,height values
   //wxbuck       = ( plxmax - plxmin ) / 20.0;
   //hybuck       = ( plymax - plymin ) / 20.0;
   wxbuck = (plxmax - plxmin) * 0.1;
   hybuck = (plymax - plymin) * 0.2;
   ct_wxbuck->disconnect();
   ct_hybuck->disconnect();
   int spwr = qRound(log10(wxbuck));
   int kpwr = qRound(log10(hybuck));
   smax = pow(10.0, spwr + 3);
   kmax = pow(10.0, kpwr + 3);
   sinc = pow(10.0, spwr - 3);
   kinc = pow(10.0, kpwr - 3);
   //wxbuck       = qRound( wxbuck / sinc ) * sinc;
   ct_wxbuck->setRange(0.0, smax);
   ct_wxbuck->setSingleStep(sinc);
   ct_wxbuck->setValue(wxbuck);
   //hybuck       = qRound( hybuck / kinc ) * kinc;
   ct_hybuck->setRange(0.0, kmax);
   ct_hybuck->setSingleStep(kinc);
   ct_hybuck->setValue(hybuck);
   connect(ct_wxbuck, SIGNAL(valueChanged(double)), this, SLOT(update_wxbuck(double)));
   connect(ct_hybuck, SIGNAL(valueChanged(double)), this, SLOT(update_hybuck(double)));

   // Update help and status text
   QString hmsg = tr("Now either auto-assign the solute bins, or manually select bins"
                     " by clicking on a bin vertex, then moving and releasing on the"
                     " other vertex. If you auto-assign the bins you should first"
                     " select the number of solute bins you want to use. UltraScan will"
                     " space the bins proportional to the integral value of each peak,"
                     " such that each bin contains the same integral value."
                     " You can select each solute bin from the listbox on the left and"
                     " modify its size by first changing the bucket dimensions with the"
                     " respective counters, then double-click on the listbox item."
                     " You may remove a bin by right-mouse-button clicking on the listbox"
                     " item and responding/defaulting Yes in the resulting dialog.");

   if (attr_x != ATTR_V && attr_y != ATTR_V && attr_z != ATTR_V) {
      hmsg = hmsg
             + tr("\n\nNO SAVE ENABLED:  None of X "
                  "or Y or the fixed-attribute is VBAR.");
   }

   if (monte_carlo) {
      hmsg = hmsg
             + tr("\n\nNO AUTOASSIGN ENABLED:  "
                  "Distribution is from a Monte-Carlo.");
   }

   te_pctl_help->setText(hmsg);

   DbgLv(1) << "SL: autoassn xmin,xmax,ymin,ymax" << plxmin << plxmax << plymin << plymax;
   pb_autassb->setEnabled(!monte_carlo);

   int kisols = sdistro->size();
   stnpline = tr("The number of distribution points is %1.").arg(kisols);
   te_status->setText(stcmline + "\n" + stdiline + "\n" + stdfline + "\n" + stfxline + "\n" + stnpline);
}

// Sort distribution solute list by s,k values and optionally reduce
void US_GA_Initialize::sort_distro(QList<S_Solute> &listsols, bool reduce) {
   int sizi = listsols.size();

   if (sizi < 2)
      return; // nothing need be done for 1-element list

   // sort distro solute list by s,k values

   qSort(listsols.begin(), listsols.end(), distro_lessthan);

   // check reduce flag

   if (reduce) { // skip any duplicates in sorted list
      S_Solute sol1;
      S_Solute sol2;
      QList<S_Solute> reduced;
      sol1 = listsols.at(0);
      reduced.append(sol1); // output first entry
      int kdup = 0;
      int jdup = 0;

      for (int jj = 1; jj < sizi; jj++) { // loop to compare each entry to previous
         sol2 = listsols.at(jj); // solute entry

         if (sol1.s != sol2.s || sol1.k != sol2.k) { // not a duplicate, so output to temporary list
            reduced.append(sol2);
            jdup = 0;
         }

         else { // duplicate:  sum c value
            DbgLv(1) << "DUP: sval svpr jj" << sol1.s << sol2.s << jj;
            kdup = max(kdup, ++jdup);
            qreal f = ( qreal ) (jdup + 1);
            sol2.c += sol1.c; // sum c value
            sol2.s = (sol1.s * jdup + sol2.s) / f; // average s,k
            sol2.k = (sol1.k * jdup + sol2.k) / f;
            reduced.replace(reduced.size() - 1, sol2);
         }

         sol1 = sol2; // save entry for next iteration
      }

      if (reduced.size() < sizi) { // if some reduction happened, replace list with reduced version
         //double sc = 1.0 / (double)( kdup + 1 );

         //for ( int ii = 0; ii < reduced.size(); ii++ )
         //   reduced[ ii ].c *= sc;

         listsols = reduced;
      }
   }
   return;
}

// set bucket pens for previous and current bin
void US_GA_Initialize::setBucketPens() {
   QPen penCR(QColor(Qt::red), 1, Qt::SolidLine);
   QPen penCY(QColor(Qt::yellow), 1, Qt::SolidLine);
   QPen penCB(QColor(Qt::blue), 1, Qt::SolidLine);

   // current is always red
   cbukpen = penCR;

   // previous is blue for light background, yellow for dark
   QColor bg = data_plot->canvasBackground();
   int csum = bg.red() + bg.green() + bg.blue();
   pbukpen = (csum > 600) ? penCB : penCY;

   return;
}

// highlight solute bin rectangle in red; previous in yellow or blue
void US_GA_Initialize::highlight_solute(QwtPlotCurve *bc1) {
   if (bc1 == NULL)
      return;

   if (nibuks > 0 && pc1 != NULL) { // re-color previous bucket yellow or blue
      pc1->setPen(pbukpen);
   }

   // current bucket borders drawn in red
   bc1->setPen(cbukpen);

   pc1 = bc1; // save previous bucket curve
   return;
}

// find bucket curve by solute index, then highlight
void US_GA_Initialize::highlight_solute(int sx) {
   highlight_solute(bucketCurveAt(sx));
   return;
}

// mouse down:  save of first point
void US_GA_Initialize::getMouseDown(const QwtDoublePoint &p) {
   p1 = p; // save the first rubberband point
}

// mouse up:  draw bucket rectangle
void US_GA_Initialize::getMouseUp(const QwtDoublePoint &p) {
   double tx[ 2 ];
   double ty[ 2 ];
   QwtPlotCurve *bc1;

   p2 = p; // save the second rubberband point

   // draw the bucket rectangle
   bc1 = drawBucketRect(nibuks, p1, p2);

   // highlight it (and turn off highlight for previous)
   highlight_solute(bc1);

   data_plot->replot();

   // construct and save a bucket entry
   tx[ 0 ] = p1.x(); // upper,left and lower,right points
   ty[ 0 ] = p1.y();
   tx[ 1 ] = p2.x();
   ty[ 1 ] = p2.y();

   if (tx[ 0 ] > tx[ 1 ]) // insure properly ordered
   {
      tx[ 0 ] = p2.x();
      tx[ 1 ] = p1.x();
   }

   if (ty[ 0 ] > ty[ 1 ]) {
      ty[ 0 ] = p2.y();
      ty[ 1 ] = p1.y();
   }

   // create bucket rectangle, solute point, and concentration value
   QRectF bucr(QPointF(tx[ 0 ], ty[ 1 ]), QPointF(tx[ 1 ], ty[ 0 ]));
   QPointF tpt((tx[ 0 ] + tx[ 1 ]) / 2.0, (ty[ 0 ] + ty[ 1 ]) / 2.0);
   QPointF &bucp = tpt;
   int sx = soludata->findNearestPoint(bucp);

   if (sx >= 0) { // for solute point nearest to rectangle midpoint
      S_Solute sol = sdistro->at(sx);
      qreal bucc = sol.c; // get concentrate value

      // add the bucket entry and add a text box entry
      soludata->appendBucket(bucr, bucp, bucc, 2);

      QString txt = soludata->bucketLine(-3);
      lw_sbin_data->addItem(txt);
   }

   // bump solute bins count
   nibuks++;
   ct_nisols->setValue(( double ) nibuks);

   pb_save->setEnabled(attr_x == ATTR_V || attr_y == ATTR_V || attr_z == ATTR_V);
   pb_view->setEnabled(true);
   pb_ckovrlp->setEnabled(nibuks > 1);
   is_saved = false;
}

// draw a bucket rectangle by index and top-left,bottom-right points
QwtPlotCurve *US_GA_Initialize::drawBucketRect(int sx, QPointF pt1, QPointF pt2) {
   double tx[ 5 ];
   double ty[ 5 ];
   QwtPlotCurve *bc1;

   tx[ 0 ] = pt1.x(); // set 5 points needed to draw rectangle
   ty[ 0 ] = pt1.y();
   tx[ 1 ] = pt2.x();
   ty[ 1 ] = pt1.y();
   tx[ 2 ] = pt2.x();
   ty[ 2 ] = pt2.y();
   tx[ 3 ] = pt1.x();
   ty[ 3 ] = pt2.y();
   tx[ 4 ] = pt1.x();
   ty[ 4 ] = pt1.y();

   // create the bucket rectangle curve
   bc1 = us_curve(data_plot, QString("bucket border %1").arg(sx));
   bc1->setPen(pbukpen);
   bc1->setStyle(QwtPlotCurve::Lines);
   bc1->setSamples(tx, ty, 5);

   return bc1;
}

// draw a bucket rectangle by index and rectangle
QwtPlotCurve *US_GA_Initialize::drawBucketRect(int sx, QRectF rect) {
   return drawBucketRect(sx, rect.topLeft(), rect.bottomRight());
}

// solute bin list row selected by arrow key
void US_GA_Initialize::newrow_sbdata(int /*row*/) {
   sclick_sbdata(lw_sbin_data->currentIndex());
}

// solute bin list row clicked: highlight bucket
void US_GA_Initialize::sclick_sbdata(const QModelIndex &mx) {
   sclick_sbdata(mx.row());
}

// solute bin list row clicked: highlight bucket
void US_GA_Initialize::sclick_sbdata(const int mrow) {
   int sx = mrow;
   bool global = monte_carlo || manbuks;
   bool rtbtn = (QApplication::mouseButtons() == Qt::RightButton);
   DbgLv(1) << "gain: sclick_sbd: sx" << sx << "rtbtn" << rtbtn;

   highlight_solute(sx);
   data_plot->replot();

   if (rtbtn) { // Handle right-mouse-button click:  delete bin
      lw_sbin_data->disconnect();
      int binx = sx + 1;
      QMessageBox msgBox;
      QString msg = tr("Are you sure you want to delete solute bin %1 ?").arg(binx);
      msgBox.setWindowTitle(tr("GA_Init Solute Bin Delete"));
      msgBox.setText(msg);
      msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
      msgBox.setDefaultButton(QMessageBox::Yes);
      if (msgBox.exec() == QMessageBox::Yes) {
         removeSoluteBin(sx);
      }
      connect(lw_sbin_data, SIGNAL(pressed(const QModelIndex &)), this, SLOT(sclick_sbdata(const QModelIndex &)));
      connect(lw_sbin_data, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(dclick_sbdata(const QModelIndex &)));
      connect(lw_sbin_data, SIGNAL(currentRowChanged(int)), this, SLOT(newrow_sbdata(int)));

      int rx = qMin(sx, (nibuks - 1));
      if (rx >= 0) {
         lw_sbin_data->setCurrentRow(rx);
         highlight_solute(rx);
         data_plot->replot();
         sx = rx;
      }
   }

   else if (global && sx != sxset) {
      QRectF rect = soludata->bucketRect(sx);
      ct_wxbuck->setValue(rect.width());
      ct_hybuck->setValue(rect.height());
   }
   sxset = sx;
}

// solute bin list row double-clicked:  change bucket values
void US_GA_Initialize::dclick_sbdata(const QModelIndex &mx) {
   int sx = mx.row();
   bool global = monte_carlo || manbuks;
   QwtPlotCurve *bc1;
   QPointF pt0;

   if (!global) {
      pt0 = soludata->bucketPoint(sx, false);
   }
   else {
      QRectF rect = soludata->bucketRect(sx);
      QPointF ptl = rect.topLeft();
      QPointF pbr = rect.bottomRight();
      pt0 = QPointF((ptl.x() + pbr.x()) / 2.0, (ptl.y() + pbr.y()) / 2.0);
   }
   sxset = sx;
   qreal x1 = pt0.x() - wxbuck / 2.0;
   qreal y1 = pt0.y() + hybuck / 2.0;
   qreal x2 = pt0.x() + wxbuck / 2.0;
   qreal y2 = pt0.y() - hybuck / 2.0;
   QPointF pt1(x1, y1);
   QPointF pt2(x2, y2);
   QRectF brect(pt1, pt2);

   pc1->detach(); // erase old rectangle for this bucket

   changeBucketRect(sx, brect); // change bucket rectangle

   bc1 = drawBucketRect(sx, pt1, pt2); // draw a new rectangle

   pc1 = bc1; // save previous bucket curve

   data_plot->replot();

   return;
}

// change the rectangle (vertices) for a bucket
void US_GA_Initialize::changeBucketRect(int sx, QRectF &rect) {
   bucket abuck = soludata->bucketAt(sx);

   QPointF bpnt = soludata->bucketPoint(sx, true);
   qreal bconc = soludata->bucketAt(sx).conc;
   int bstat = 0;

   soludata->setBucket(sx, rect, bpnt, bconc, bstat);

   QString line = soludata->bucketLine(sx);
   lw_sbin_data->item(sx)->setText(line);

   return;
}

// find the plot curve in the list of curves
QwtPlotCurve *US_GA_Initialize::bucketCurveAt(int sx) {
   // get title of desired bucket curve and list of all items
   QString ctext = QString("bucket border %1").arg(sx);
   QwtPlotItemList ilist = data_plot->itemList();

   for (int jj = 0; jj < ilist.size(); jj++) { // test each item for Curve type and matching title
      QwtPlotCurve *bc1 = ( QwtPlotCurve * ) ilist.at(jj);

      if (bc1->rtti() == QwtPlotItem::Rtti_PlotCurve) { // right type,  so check title
         QString itext = bc1->title().text();

         if (itext.compare(ctext) == 0) { // this is the one we want, return a pointer to the curve
            return bc1;
         }
      }
   }

   return ( QwtPlotCurve * ) NULL;
}

// erase all bucket curves (option to completely delete )
void US_GA_Initialize::erase_buckets(bool delflag) {
   for (int jj = 0; jj < nibuks; jj++) {
      QwtPlotCurve *bc1 = bucketCurveAt(jj);
      if (bc1 != NULL) {
         bc1->detach();

         if (delflag)
            delete bc1;
      }
   }
   nibuks = 0;

   data_plot->replot();

   return;
}
// erase all bucket curves (from plot only)
void US_GA_Initialize::erase_buckets() {
   erase_buckets(false);
   return;
}

// Remove a solute bin from data, plot, and list
void US_GA_Initialize::removeSoluteBin(int sx) {
   // Remove the solute bin
   int bsize = soludata->bucketsCount();
   soludata->removeBucketAt(sx);
   nibuks = bsize - 1;

   // Reset the buckets plot and list
   resetPlotAndList(sx);
}

// Reset the buckets plot and list after bucket remove or sort
void US_GA_Initialize::resetPlotAndList(int hlx) {
   // Replace the List Widget contents and redraw bin rectangles
   int kibuks = soludata->bucketsCount();
   lw_sbin_data->clear();
   erase_buckets();
   DbgLv(1) << "rPAL:kibuks nibuks" << kibuks << nibuks;
   nibuks = kibuks;
   ct_nisols->setValue(( double ) nibuks);

   for (int jj = 0; jj < nibuks; jj++) { // Draw the auto-assigned buckets and add lines to list widget
      QRectF rect = soludata->bucketRect(jj);
      QwtPlotCurve *bc1 = drawBucketRect(jj, rect);
      bc1->attach(data_plot);

      lw_sbin_data->addItem(soludata->bucketLine(jj));
      DbgLv(1) << "rPAL:  " << soludata->bucketLine(jj);
   }

   // Highlight a specified next bucket
   hlx = (hlx >= 0 && hlx < nibuks) ? hlx : (nibuks - 1);
   DbgLv(1) << "rPAL: hlx" << hlx;
   if (hlx >= 0) {
      DbgLv(1) << "rPAL:  set-row" << hlx << nibuks << lw_sbin_data->count();
      lw_sbin_data->setCurrentRow(hlx);
      DbgLv(1) << "rPAL:  highlight sol";
      highlight_solute(hlx);
   }

   DbgLv(1) << "rPAL:  replot sol";
   data_plot->replot();
   qApp->processEvents();

   return;
}

// Flag whether two values are effectively equal within a given epsilon
bool US_GA_Initialize::equivalent(double a, double b, double eps) {
   return (qAbs((a - b) / a) <= eps);
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_GA_Initialize::update_disk_db(bool isDB) {
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}
// Select a prefilter for model distribution list
void US_GA_Initialize::select_prefilt(void) {
   QString pfmsg;
   int nruns = 0;
   pfilts.clear();

   US_SelectRuns srdiag(dkdb_cntrls->db(), pfilts);
   srdiag.move(this->pos() + QPoint(200, 200));
   connect(&srdiag, SIGNAL(dkdb_changed(bool)), this, SLOT(update_disk_db(bool)));

   if (srdiag.exec() == QDialog::Accepted)
      nruns = pfilts.size();
   else
      pfilts.clear();

   if (nruns == 0)
      pfmsg = tr("(none chosen)");

   else if (nruns > 1)
      pfmsg = tr("RunID prefilter - %1 run(s)").arg(nruns);

   else
      pfmsg = tr("RunID prefilter - 1 run: ") + QString(pfilts[ 0 ]).left(8) + "...";

   DbgLv(1) << "PreFilt: pfilts[0]" << ((nruns > 0) ? pfilts[ 0 ] : "(none)");
   le_prefilt->setText(pfmsg);
}

// View the statistics file produced in a file editor
void US_GA_Initialize::view() {
   DbgLv(1) << "VIEW";
   QString runid = run_name.section(".", 0, -2);
   if (runid.startsWith("Global-"))
      runid = runid.mid(7);
   QString trpid = run_name.section(".", -1, -1);
   QString fdir = US_Settings::resultDir() + "/" + runid;
   QString fnsta = "gainit." + trpid + ".sol_integ.stats";
   QString fname = fdir + "/" + fnsta;
   DbgLv(1) << "VIEW fname" << fname;

   if (!is_saved) { // No save was done, so generate report file
      soludata->buildDataMC(); // build it
      soludata->reportDataMC(fname, mc_iters); // report it
      is_saved = true;
   }

   QFile filei(fname);
   if (filei.open(QIODevice::ReadOnly | QIODevice::Text)) {
      DbgLv(1) << "VIEW OPENED";
      QTextStream ts(&filei);
      QString rtext = ts.readAll();
      filei.close();

      US_Editor *eddiag = new US_Editor(US_Editor::DEFAULT, true);
      eddiag->setWindowTitle(tr("Statistics File Contents"));
      eddiag->move(this->pos() + QPoint(30, 30));
      eddiag->resize(760, 700);
      eddiag->e->setFont(US_Widgets::fixedFont());
      eddiag->e->setPlainText(rtext);
      DbgLv(1) << "VIEW eddiag SHOW";
      eddiag->show();
   }

   else {
      DbgLv(1) << "VIEW OPEN ERROR" << fname;
      QMessageBox::critical(this, tr("File Read Error"), tr("Unable to open file\n\"%1\"\nfor read").arg(fname));
   }
}

// Select the coordinate for the horizontal axis
void US_GA_Initialize::select_x_axis(int ival) {
   const QString xlabs[] = { "s", "f/f0", "MW", "vbar", "D", "f" };
   const double xvlos[] = { 1.0, 1.0, 2e+4, 0.60, 1e-8, 1e-8 };
   const double xvhis[] = { 10.0, 4.0, 1e+5, 0.80, 1e-7, 1e-7 };
   const double xmins[] = { -10000.0, 1.0, 0.0, 0.01, 1e-9, 1e-9 };
   const double xmaxs[] = { 10000.0, 50.0, 1e+10, 3.00, 1e-5, 1e-5 };
   const double xincs[] = { 0.01, 0.01, 1000.0, 0.01, 1e-9, 1e-9 };

   attr_x = ival;
   xa_title = anno_title(attr_x);
   QString xlab = xlabs[ attr_x ];
   double xmin = xmins[ attr_x ];
   double xmax = xmaxs[ attr_x ];
   double xinc = xincs[ attr_x ];
   lb_plxmin->setText(tr("Plot Limit ") + xlab + tr(" Minimum:"));
   lb_plxmax->setText(tr("Plot Limit ") + xlab + tr(" Maximum:"));
   lb_wxbuck->setText(tr("Width of ") + xlab + tr(" Bucket:"));
   ct_plxmin->setRange(xmin, xmax);
   ct_plxmax->setRange(xmin, xmax);
   ct_plxmin->setSingleStep(xinc);
   ct_plxmax->setSingleStep(xinc);
   ct_plxmin->setValue(xvlos[ attr_x ]);
   ct_plxmax->setValue(xvhis[ attr_x ]);

   rb_y_s->setEnabled(attr_x != ATTR_S);
   rb_y_ff0->setEnabled(attr_x != ATTR_K);
   rb_y_mw->setEnabled(attr_x != ATTR_W);
   rb_y_vbar->setEnabled(attr_x != ATTR_V);
   rb_y_D->setEnabled(attr_x != ATTR_D);
   rb_y_f->setEnabled(attr_x != ATTR_F);

   build_xy_distro();

   set_limits();

   replot_data();
}

// Select the coordinate for the vertical axis
void US_GA_Initialize::select_y_axis(int ival) {
   const QString ylabs[] = { "s", "f/f0", "MW", "vbar", "D", "f" };
   const double yvlos[] = { 1.0, 1.0, 2e+4, 0.60, 1e-8, 1e-8 };
   const double yvhis[] = { 10.0, 4.0, 1e+5, 0.80, 1e-7, 1e-7 };
   const double ymins[] = { -10000.0, 1.0, 0.0, 0.01, 1e-9, 1e-9 };
   const double ymaxs[] = { 10000.0, 50.0, 1e+10, 3.00, 1e-5, 1e-5 };
   const double yincs[] = { 0.01, 0.01, 1000.0, 0.01, 1e-9, 1e-9 };

   attr_y = ival;
   ya_title = anno_title(attr_y);
   QString ylab = ylabs[ attr_y ];
   double ymin = ymins[ attr_y ];
   double ymax = ymaxs[ attr_y ];
   double yinc = yincs[ attr_y ];
   lb_plymin->setText(tr("Plot Limit ") + ylab + tr(" Minimum:"));
   lb_plymax->setText(tr("Plot Limit ") + ylab + tr(" Maximum:"));
   lb_hybuck->setText(tr("Height of ") + ylab + tr(" Bucket:"));
   ct_plymin->setRange(ymin, ymax);
   ct_plymax->setRange(ymin, ymax);
   ct_plymin->setSingleStep(yinc);
   ct_plymax->setSingleStep(yinc);
   ct_plymin->setValue(yvlos[ attr_y ]);
   ct_plymax->setValue(yvhis[ attr_y ]);

   rb_x_s->setEnabled(attr_y != ATTR_S);
   rb_x_ff0->setEnabled(attr_y != ATTR_K);
   rb_x_mw->setEnabled(attr_y != ATTR_W);
   rb_x_vbar->setEnabled(attr_y != ATTR_V);
   rb_x_D->setEnabled(attr_y != ATTR_D);
   rb_x_f->setEnabled(attr_y != ATTR_F);

   build_xy_distro();

   set_limits();

   replot_data();
}

// Re-generate the XY version of the current distribution
void US_GA_Initialize::build_xy_distro() {
   S_Solute sol_sk;
   S_Solute sol_xy;
   xy_distro.clear();

   for (int jj = 0; jj < sk_distro.size(); jj++) {
      sol_sk = sk_distro[ jj ];
      sol_xy = sol_sk;
      sol_xy.si = sol_sk.s;
      sol_xy.ki = sol_sk.k;

      sol_xy.s = (attr_x == ATTR_S) ? sol_sk.s : sol_xy.s;
      sol_xy.s = (attr_x == ATTR_K) ? sol_sk.k : sol_xy.s;
      sol_xy.s = (attr_x == ATTR_W) ? sol_sk.w : sol_xy.s;
      sol_xy.s = (attr_x == ATTR_V) ? sol_sk.v : sol_xy.s;
      sol_xy.s = (attr_x == ATTR_D) ? sol_sk.d : sol_xy.s;
      sol_xy.s = (attr_x == ATTR_F) ? sol_sk.f : sol_xy.s;
      sol_xy.k = (attr_y == ATTR_S) ? sol_sk.s : sol_xy.k;
      sol_xy.k = (attr_y == ATTR_K) ? sol_sk.k : sol_xy.k;
      sol_xy.k = (attr_y == ATTR_W) ? sol_sk.w : sol_xy.k;
      sol_xy.k = (attr_y == ATTR_V) ? sol_sk.v : sol_xy.k;
      sol_xy.k = (attr_y == ATTR_D) ? sol_sk.d : sol_xy.k;
      sol_xy.k = (attr_y == ATTR_F) ? sol_sk.f : sol_xy.k;

      xy_distro << sol_xy;
   }

   sort_distro(xy_distro, true);
}

// Set annotation title for a plot index
QString US_GA_Initialize::anno_title(int pltndx) {
   QString a_title;

   if (pltndx == ATTR_S)
      a_title = tr("Sedimentation Coefficient (1e-13)"
                   " for water at 20")
                + DEGC;
   else if (pltndx == ATTR_K)
      a_title = tr("Frictional Ratio f/f0");
   else if (pltndx == ATTR_W)
      a_title = tr("Molecular Weight (Dalton)");
   else if (pltndx == ATTR_V)
      a_title = tr("Vbar at 20") + DEGC;
   else if (pltndx == ATTR_D)
      a_title = tr("Diffusion Coefficient");
   else if (pltndx == ATTR_F)
      a_title = tr("Frictional Coefficient");

   return a_title;
}

// Load bins from a saved gadistro file
void US_GA_Initialize::load_bins(void) {
   int hlx = 0;
   pc1 = NULL;
   DbgLv(1) << "gain: load_bins()";

   // Get a distro file
   QString filter = tr("Bin GaDistro files (*gadistro.dat);;") + tr("Any DAT files (*.dat);;") + tr("Any files (*)");
   QString fname = QFileDialog::getOpenFileName(this, tr("Load Solute Bin (GaDistro) File"), binfpath, filter, 0, 0);

   if (fname.isEmpty())
      return;

   // Load the solute bins and plot them
   binfpath = QString(fname).section("/", 0, -2);
   soludata->loadGAdata(fname, &attr_x, &attr_y, &attr_z);
   DbgLv(1) << "gain:  ld_b : fname attr_x attr_y attr_z" << fname << attr_x << attr_y << attr_z;

   soludata->sortBuckets();
   resetPlotAndList(hlx);

   is_saved = false;
   nibuks = soludata->bucketsCount();
   pb_save->setEnabled(attr_x == ATTR_V || attr_y == ATTR_V || attr_z == ATTR_V);
   pb_view->setEnabled(true);
   pb_ckovrlp->setEnabled(nibuks > 1);
}

// Make a ColorMap copy and return a pointer to the new ColorMap
QwtLinearColorMap *US_GA_Initialize::ColorMapCopy(QwtLinearColorMap *colormap) {
   QVector<double> cstops = colormap->colorStops();
   int lstop = cstops.count() - 1;
   QwtInterval csvals(0.0, 1.0);
   QwtLinearColorMap *cmapcopy = new QwtLinearColorMap(colormap->color1(), colormap->color2());

   for (int jj = 1; jj < lstop; jj++) {
      QColor scolor = colormap->color(csvals, cstops[ jj ]);
      cmapcopy->addColorStop(cstops[ jj ], scolor);
   }

   return cmapcopy;
}
