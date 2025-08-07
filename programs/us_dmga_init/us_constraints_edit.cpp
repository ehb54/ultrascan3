//! \file us_constraints_edit.cpp

#include "us_constraints_edit.h"
#include "us_associations_gui.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_images.h"
#include "us_investigator.h"
#include "us_passwd.h"
#include "us_properties.h"
#include "us_settings.h"
#include "us_util.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

const QString notapl = QObject::tr("");

// Constructor of dialog for editing discreteGA constraints
US_ConstraintsEdit::US_ConstraintsEdit(US_Model &current_model) : US_WidgetsDialog(0, 0), cmodel(current_model) {
   setWindowTitle("Discrete Model GA Constraints Editor");
   setPalette(US_GuiSettings::frameColor());
   setWindowModality(Qt::WindowModal);

   dbg_level = US_Settings::us_debug();
   oldRow = -2;
   inUpdate = false;
   chgStoi = false;
   crow = -1;
   arow = -1;
   normal = US_GuiSettings::editColor();

   // Initialize the check icon
   check = US_Images::getIcon(US_Images::CHECK);

   // Grid
   QGridLayout *main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   // Components
   QLabel *lb_comps = us_banner(tr("Components"));
   lw_comps = new US_ListWidget;

   // Components column headers
   QLabel *lb_attr = us_label(tr("Attribute"));
   QLabel *lb_avalue = us_label(tr("Value"));
   QLabel *lb_alow = us_label(tr("Low"));
   QLabel *lb_ahigh = us_label(tr("High"));
   QLabel *lb_float = us_label(tr("Float?"));
   QLabel *lb_locsc = us_label(tr("LogSc?"));
   //   QLabel*      lb_desc   = us_label( tr( "Analyte Description:" ) );

   // Attribute Fix? checkboxes and labels
   QGridLayout *lo_vbar = us_checkbox(tr("Vbar at 20 ") + DEGC + " (ml/g):", ck_sel_vbar, true);
   QGridLayout *lo_mw = us_checkbox(tr("Molecular Wt. (mw)"), ck_sel_mw, true);
   QGridLayout *lo_ff0 = us_checkbox(tr("Frictional Ratio (f/f0)"), ck_sel_ff0, true);
   QGridLayout *lo_s = us_checkbox(tr("Sedimentation Coeff. (s)"), ck_sel_s);
   QGridLayout *lo_D = us_checkbox(tr("Diffusion Coeff. (D)"), ck_sel_D);
   QGridLayout *lo_f = us_checkbox(tr("Frictional Coeff. (f)"), ck_sel_f);
   QGridLayout *lo_conc = us_checkbox(tr("Partial Concentration"), ck_sel_conc, true);
   ck_sel_vbar->setEnabled(false);
   ck_sel_mw->setEnabled(true);
   ck_sel_ff0->setEnabled(true);
   ck_sel_s->setEnabled(true);
   ck_sel_D->setEnabled(true);
   ck_sel_f->setEnabled(true);
   ck_sel_conc->setEnabled(false);
   DbgLv(1) << "cnG:main: hds,lbs defined";

   // Attribute value/low/high text boxes
   le_val_vbar = us_lineedit("0.7200");
   le_min_vbar = us_lineedit(notapl, true);
   le_max_vbar = us_lineedit(notapl, true);
   le_val_mw = us_lineedit("10000");
   le_min_mw = us_lineedit(notapl, true);
   le_max_mw = us_lineedit(notapl, true);
   le_val_ff0 = us_lineedit("2.0");
   le_min_ff0 = us_lineedit(notapl, true);
   le_max_ff0 = us_lineedit(notapl, true);
   le_val_s = us_lineedit("3e-13");
   le_min_s = us_lineedit(notapl, true);
   le_max_s = us_lineedit(notapl, true);
   le_val_D = us_lineedit("3e-7");
   le_min_D = us_lineedit(notapl, true);
   le_max_D = us_lineedit(notapl, true);
   le_val_f = us_lineedit("3e-6");
   le_min_f = us_lineedit(notapl, true);
   le_max_f = us_lineedit(notapl, true);
   le_val_conc = us_lineedit("0.5");
   le_min_conc = us_lineedit(notapl, true);
   le_max_conc = us_lineedit(notapl, true);
   DbgLv(1) << "cnG:main: le_vals defined";

   // Attribute Float? checkboxes
   ck_flt_vbar = new QCheckBox("", this);
   ck_flt_mw = new QCheckBox("", this);
   ck_log_mw = new QCheckBox("", this);
   ck_flt_ff0 = new QCheckBox("", this);
   ck_flt_s = new QCheckBox("", this);
   ck_flt_D = new QCheckBox("", this);
   ck_flt_f = new QCheckBox("", this);
   ck_flt_conc = new QCheckBox("", this);
   ck_flt_vbar->setChecked(false);
   ck_flt_mw->setChecked(false);
   ck_log_mw->setChecked(false);
   ck_flt_ff0->setChecked(false);
   ck_flt_s->setChecked(false);
   ck_flt_D->setChecked(false);
   ck_flt_f->setChecked(false);
   ck_flt_conc->setChecked(false);
   ck_flt_vbar->setEnabled(true);
   ck_flt_mw->setEnabled(true);
   ck_log_mw->setEnabled(true);
   ck_flt_ff0->setEnabled(true);
   ck_flt_s->setEnabled(true);
   ck_flt_D->setEnabled(true);
   ck_flt_f->setEnabled(true);
   ck_flt_conc->setEnabled(true);
   DbgLv(1) << "cnG:main: flt/log defined";

   QLabel *lb_extinction = us_label(tr("Extinction (OD/(cm*mol)):"));
   QLabel *lb_wavelength = us_label(tr("Wavelength (nm):"));
   le_extinction = us_lineedit("");
   le_wavelength = us_lineedit(QString::number(cmodel.wavelength, 'f', 1));
   le_wavelength->setMinimumWidth(80);
   le_wavelength->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
   us_setReadOnly(le_wavelength, true);

   QLabel *lb_oligomer = us_label(tr("Oligomer"));
   le_oligomer = us_lineedit("");
   QGridLayout *isreac_layout = us_checkbox(tr("Is Reactant"), ck_isreact);
   QGridLayout *isprod_layout = us_checkbox(tr("Is Product"), ck_isprod);
   us_setReadOnly(le_oligomer, true);
   ck_isreact->setEnabled(false);
   ck_isprod->setEnabled(false);

   // Advanced parameters
   QLabel *lb_sigma = us_label(tr("Concentration Dependency of s (<span>&sigma;</span>):"));
   le_sigma = us_lineedit("");
   QLabel *lb_delta = us_label(tr("Concentration Dependency of D (<span>&delta;</span>):"));
   le_delta = us_lineedit("");
   pb_load_c0 = us_pushbutton(tr("Load C0 from File"));
   QGridLayout *co_sed_layout = us_checkbox(tr("Co-sedimenting Solute"), ck_co_sed);
   pb_load_c0->setEnabled(false);
   QPushButton *pb_recompute = us_pushbutton(tr("Re-compute unselected component attribute values"));

   // Associations
   QLabel *lb_assocs = us_banner(tr("Associations (reactions)"));
   lw_assocs = new US_ListWidget;

   // Associations column headers
   QLabel *lb_attra = us_label(tr("Attribute"));
   QLabel *lb_avala = us_label(tr("Value"));
   QLabel *lb_alowa = us_label(tr("Low"));
   QLabel *lb_ahigha = us_label(tr("High"));
   QLabel *lb_floata = us_label(tr("Float?"));
   QLabel *lb_logsca = us_label(tr("LogSc?"));
   le_lbl_kd = us_lineedit(tr("K_dissociation"), true);
   le_val_kd = us_lineedit("0.0001");
   le_min_kd = us_lineedit(notapl, true);
   le_max_kd = us_lineedit(notapl, true);
   ck_flt_kd = new QCheckBox("", this);
   ck_log_kd = new QCheckBox("", this);
   le_lbl_koff = us_lineedit(tr("k_off rate"), true);
   le_val_koff = us_lineedit("0.0001");
   le_min_koff = us_lineedit(notapl, true);
   le_max_koff = us_lineedit(notapl, true);
   ck_flt_koff = new QCheckBox("", this);
   ck_log_koff = new QCheckBox("", this);
   us_setReadOnly(le_lbl_kd, true);
   us_setReadOnly(le_lbl_koff, true);
   le_lbl_kd->setEnabled(false);
   le_lbl_koff->setEnabled(false);
   ck_flt_kd->setEnabled(true);
   ck_log_kd->setEnabled(true);
   ck_flt_koff->setEnabled(true);
   ck_log_koff->setEnabled(true);
   DbgLv(1) << "cnG:main: assocs  defined";

   QPushButton *pb_help = us_pushbutton(tr("Help"));
   QPushButton *pb_close = us_pushbutton(tr("Cancel"));
   pb_accept = us_pushbutton(tr("Accept"));
   DbgLv(1) << "cnG:main: elements defined";

   int row = 0;
   main->addWidget(lb_comps, row++, 0, 1, 12);
   main->addWidget(lw_comps, row, 0, 3, 12);
   row += 3;
   main->addWidget(lb_attr, row, 0, 1, 4);
   main->addWidget(lb_avalue, row, 4, 1, 2);
   main->addWidget(lb_alow, row, 6, 1, 2);
   main->addWidget(lb_ahigh, row, 8, 1, 2);
   main->addWidget(lb_float, row, 10, 1, 1);
   main->addWidget(lb_locsc, row++, 11, 1, 1);
   main->addLayout(lo_vbar, row, 0, 1, 4);
   main->addWidget(le_val_vbar, row, 4, 1, 2);
   main->addWidget(le_min_vbar, row, 6, 1, 2);
   main->addWidget(le_max_vbar, row, 8, 1, 2);
   main->addWidget(ck_flt_vbar, row++, 10, 1, 1);
   main->addLayout(lo_mw, row, 0, 1, 4);
   main->addWidget(le_val_mw, row, 4, 1, 2);
   main->addWidget(le_min_mw, row, 6, 1, 2);
   main->addWidget(le_max_mw, row, 8, 1, 2);
   main->addWidget(ck_flt_mw, row, 10, 1, 1);
   main->addWidget(ck_log_mw, row++, 11, 1, 1);
   main->addLayout(lo_ff0, row, 0, 1, 4);
   main->addWidget(le_val_ff0, row, 4, 1, 2);
   main->addWidget(le_min_ff0, row, 6, 1, 2);
   main->addWidget(le_max_ff0, row, 8, 1, 2);
   main->addWidget(ck_flt_ff0, row++, 10, 1, 1);
   main->addLayout(lo_s, row, 0, 1, 4);
   main->addWidget(le_val_s, row, 4, 1, 2);
   main->addWidget(le_min_s, row, 6, 1, 2);
   main->addWidget(le_max_s, row, 8, 1, 2);
   main->addWidget(ck_flt_s, row++, 10, 1, 1);
   main->addLayout(lo_D, row, 0, 1, 4);
   main->addWidget(le_val_D, row, 4, 1, 2);
   main->addWidget(le_min_D, row, 6, 1, 2);
   main->addWidget(le_max_D, row, 8, 1, 2);
   main->addWidget(ck_flt_D, row++, 10, 1, 1);
   main->addLayout(lo_f, row, 0, 1, 4);
   main->addWidget(le_val_f, row, 4, 1, 2);
   main->addWidget(le_min_f, row, 6, 1, 2);
   main->addWidget(le_max_f, row, 8, 1, 2);
   main->addWidget(ck_flt_f, row++, 10, 1, 1);
   main->addLayout(lo_conc, row, 0, 1, 4);
   main->addWidget(le_val_conc, row, 4, 1, 2);
   main->addWidget(le_min_conc, row, 6, 1, 2);
   main->addWidget(le_max_conc, row, 8, 1, 2);
   main->addWidget(ck_flt_conc, row++, 10, 1, 1);
   main->addWidget(lb_extinction, row, 0, 1, 4);
   main->addWidget(le_extinction, row, 4, 1, 2);
   main->addWidget(lb_wavelength, row, 6, 1, 4);
   main->addWidget(le_wavelength, row++, 10, 1, 2);
   main->addWidget(lb_oligomer, row, 0, 1, 4);
   main->addWidget(le_oligomer, row, 4, 1, 2);
   main->addLayout(isreac_layout, row, 6, 1, 3);
   main->addLayout(isprod_layout, row++, 9, 1, 3);
   main->addWidget(lb_sigma, row, 0, 1, 6);
   main->addWidget(le_sigma, row, 6, 1, 2);
   main->addLayout(co_sed_layout, row++, 8, 1, 4);
   main->addWidget(lb_delta, row, 0, 1, 6);
   main->addWidget(le_delta, row, 6, 1, 2);
   main->addWidget(pb_load_c0, row++, 8, 1, 4);
   main->addWidget(pb_recompute, row++, 0, 1, 12);
   main->addWidget(lb_assocs, row++, 0, 1, 12);
   main->addWidget(lw_assocs, row, 0, 2, 12);
   row += 2;
   main->addWidget(lb_attra, row, 0, 1, 4);
   main->addWidget(lb_avala, row, 4, 1, 2);
   main->addWidget(lb_alowa, row, 6, 1, 2);
   main->addWidget(lb_ahigha, row, 8, 1, 2);
   main->addWidget(lb_floata, row, 10, 1, 1);
   main->addWidget(lb_logsca, row++, 11, 1, 1);
   main->addWidget(le_lbl_kd, row, 0, 1, 4);
   main->addWidget(le_val_kd, row, 4, 1, 2);
   main->addWidget(le_min_kd, row, 6, 1, 2);
   main->addWidget(le_max_kd, row, 8, 1, 2);
   main->addWidget(ck_flt_kd, row, 10, 1, 1);
   main->addWidget(ck_log_kd, row++, 11, 1, 1);
   main->addWidget(le_lbl_koff, row, 0, 1, 4);
   main->addWidget(le_val_koff, row, 4, 1, 2);
   main->addWidget(le_min_koff, row, 6, 1, 2);
   main->addWidget(le_max_koff, row, 8, 1, 2);
   main->addWidget(ck_flt_koff, row, 10, 1, 1);
   main->addWidget(ck_log_koff, row++, 11, 1, 1);
   main->addWidget(pb_help, row, 0, 1, 4);
   main->addWidget(pb_close, row, 4, 1, 4);
   main->addWidget(pb_accept, row++, 8, 1, 4);

   connect(le_extinction, SIGNAL(editingFinished()), SLOT(set_molar()));
   connect(pb_load_c0, SIGNAL(clicked()), SLOT(load_c0()));
   connect(ck_co_sed, SIGNAL(stateChanged(int)), SLOT(co_sed(int)));
   connect(pb_recompute, SIGNAL(clicked()), SLOT(check_selects()));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));
   connect(pb_accept, SIGNAL(clicked()), SLOT(acceptProp()));
   pb_accept->setEnabled(false);
   DbgLv(1) << "cnG:main: connections made";
   const QString clets("ABCDEFGHIJ");
   lcompx.clear();

   // Populate the lists from the model
   if (cmodel.analysis == US_Model::DMGA_CONSTR) { // Constraints model
      constraints.load_constraints(&cmodel);
      int kk = 0;
      DbgLv(1) << "cnG:main:  cmodel load rtn";
      for (int ii = 0; ii < cmodel.components.size(); ii++, kk++) {
         QString prenm = QString(clets).mid(kk, 1) + " ";
         QString flgnm = QString(cmodel.components[ ii ].name).left(4);
         QString name = QString(cmodel.components[ ii ].name).mid(4);
         lw_comps->addItem(prenm + name);
         lcompx << ii;

         if (!flgnm.contains("V"))
            ii++;
      }

      for (int ii = 0; ii < cmodel.associations.size(); ii += 2) {
         QVector<int> rcomps = cmodel.associations[ ii ].rcomps;
         QVector<int> stoichs = cmodel.associations[ ii ].stoichs;
         int nrc = rcomps.size();
         int rc1 = (nrc > 0) ? rcomps[ 0 ] : 0;
         int rc2 = (nrc > 1) ? rcomps[ 1 ] : 0;
         int rc3 = (nrc > 2) ? rcomps[ 2 ] : 0;
         int st1 = (nrc > 0) ? stoichs[ 0 ] : 1;
         int st2 = (nrc > 1) ? stoichs[ 1 ] : 1;
         int st3 = (nrc > 2) ? stoichs[ 2 ] : 1;
         int ol1 = cmodel.components[ lcompx[ rc1 ] ].oligomer;
         int ol2 = cmodel.components[ lcompx[ rc2 ] ].oligomer;
         int ol3 = cmodel.components[ lcompx[ rc3 ] ].oligomer;

         QString name;
         if (rcomps.size() == 2) {
            name = QString::number(st1) + QString(clets).mid(rc1, 1) + QString::number(ol1) + " => "
                   + QString::number(qAbs(st2)) + QString(clets).mid(rc2, 1) + QString::number(ol2);
         }
         else if (rcomps.size() == 3) {
            name = QString::number(st1) + QString(clets).mid(rc1, 1) + QString::number(ol1) + " + "
                   + QString::number(st2) + QString(clets).mid(rc2, 1) + QString::number(ol2) + " => "
                   + QString::number(qAbs(st3)) + QString(clets).mid(rc3, 1) + QString::number(ol3);
         }
         else {
            name = "(reaction " + QString::number(ii + 1) + ")";
         }

         lw_assocs->addItem(name);
      }

      if (lw_assocs->count() == 0)
         lw_assocs->addItem("(none)");

      pb_accept->setEnabled(constraints.float_constraints(NULL) > 0);
   }

   else { // Base model
      constraints.load_base_model(&cmodel);
      DbgLv(1) << "cnG:main:  bmodel load rtn";
      for (int ii = 0; ii < cmodel.components.size(); ii++) {
         QString prenm = QString(clets).mid(ii, 1) + " ";
         QString name = cmodel.components[ ii ].name;
         lw_comps->addItem(prenm + name);
         lcompx << ii;
      }

      for (int ii = 0; ii < cmodel.associations.size(); ii++) {
         QVector<int> rcomps = cmodel.associations[ ii ].rcomps;
         QVector<int> stoichs = cmodel.associations[ ii ].stoichs;
         int nrc = rcomps.size();
         int rc1 = (nrc > 0) ? rcomps[ 0 ] : 0;
         int rc2 = (nrc > 1) ? rcomps[ 1 ] : 0;
         int rc3 = (nrc > 2) ? rcomps[ 2 ] : 0;
         int st1 = (nrc > 0) ? stoichs[ 0 ] : 1;
         int st2 = (nrc > 1) ? stoichs[ 1 ] : 1;
         int st3 = (nrc > 2) ? stoichs[ 2 ] : 1;
         int ol1 = cmodel.components[ rc1 ].oligomer;
         int ol2 = cmodel.components[ rc2 ].oligomer;
         int ol3 = cmodel.components[ rc3 ].oligomer;
         QString name;

         if (rcomps.size() == 2) {
            name = QString::number(st1) + QString(clets).mid(rc1, 1) + QString::number(ol1) + "  =>  "
                   + QString::number(qAbs(st2)) + QString(clets).mid(rc2, 1) + QString::number(ol2);
         }
         else if (rcomps.size() == 3) {
            name = QString::number(st1) + QString(clets).mid(rc1, 1) + QString::number(ol1) + " + "
                   + QString::number(st2) + QString(clets).mid(rc2, 1) + QString::number(ol2) + "  =>  "
                   + QString::number(qAbs(st3)) + QString(clets).mid(rc3, 1) + QString::number(ol3);
         }
         else {
            name = "(reaction " + QString::number(ii + 1) + ")";
         }

         lw_assocs->addItem(name);
      }

      if (lw_assocs->count() == 0)
         lw_assocs->addItem("(none)");
   }

   connect(lw_comps, SIGNAL(currentRowChanged(int)), SLOT(component_select(int)));
   connect(lw_assocs, SIGNAL(currentRowChanged(int)), SLOT(association_select(int)));

   QFont font(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());
   QFontMetrics fm(font);
   int fhigh = fm.lineSpacing();
   int fwide = fm.width(QChar('6'));
   int chigh = fhigh * 3 + 12;
   int ahigh = fhigh * 2 + 12;
   int lwide = width() - 10;
   const int bmwide = 700;
   const int bmhigh = 540;
   const int bfwide = 8;
   const int bfhigh = 16;

   lw_comps->setMinimumHeight(chigh);
   lw_comps->resize(lwide, chigh);
   lw_assocs->setMinimumHeight(ahigh);
   lw_assocs->resize(lwide, ahigh);

   comps_connect(true);
   assocs_connect(true);

   DbgLv(1) << "cnG:main:  set comp row";
   lw_comps->setCurrentRow(0);
   lw_assocs->setCurrentRow(cmodel.associations.size() > 0 ? 0 : -1);

   DbgLv(1) << "cnG:main:  m size" << size() << "fwide fhigh" << fwide << fhigh;
   int mwide = (bmwide * fwide) / bfwide;
   int mhigh = (bmhigh * fhigh) / bfhigh;
   resize(mwide, mhigh);
   DbgLv(1) << "cnG:main:  m size" << size();
}

void US_ConstraintsEdit::load_c0(void) {
   //   int row = lw_comps->currentRow();

   //   if ( row < 0 ) return;
   int row = 0;

   // See if the initialization vector is already loaded.
   if (!pb_load_c0->icon().isNull()) {
      int response = QMessageBox::question(
         this, tr("Remove C0 Data?"),
         tr("The C0 information is loaded.\n"
            "Remove it?"),
         QMessageBox::Yes, QMessageBox::No);

      if (response == QMessageBox::Yes) {
         US_Model::SimulationComponent *sc = &cmodel.components[ row ];

         sc->c0.radius.clear();
         sc->c0.concentration.clear();
         pb_load_c0->setIcon(QIcon());
      }

      return;
   }

   QMessageBox::information(
      this, tr("UltraScan Information"),
      tr("Please note:\n\n"
         "The initial concentration file should have\n"
         "the following format:\n\n"
         "radius_value1 concentration_value1\n"
         "radius_value2 concentration_value2\n"
         "radius_value3 concentration_value3\n"
         "etc...\n\n"
         "radius values smaller than the meniscus or\n"
         "larger than the bottom of the cell will be\n"
         "excluded from the concentration vector."));

   QString fn = QFileDialog::getOpenFileName(this, tr("Load initial concentration"), US_Settings::resultDir(), "*");

   if (!fn.isEmpty()) {
      QFile f(fn);
      ;

      if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
         QTextStream ts(&f);

         //         int row = lw_comps->currentRow();
         int row = 0;

         US_Model::SimulationComponent *sc = &cmodel.components[ row ];

         sc->c0.radius.clear();
         sc->c0.concentration.clear();

         // Sets concentration for this component to -1 to signal that we are
         // using a concentration vector
         double val1;
         double val2;

         while (!ts.atEnd()) {
            ts >> val1;
            ts >> val2;

            if (val1 > 0.0) // Ignore radius pairs that aren't positive
            {
               sc->c0.radius.push_back(val1);
               sc->c0.concentration.push_back(val2);
            }
         }

         f.close();
         pb_load_c0->setIcon(check);
      }
      else {
         QMessageBox::warning(this, tr("UltraScan Warning"), tr("UltraScan could not open the file specified\n") + fn);
      }
   }
}

void US_ConstraintsEdit::set_molar(void) {
   int row = lw_comps->currentRow();
   if (row < 0)
      return;
   int mcx = lcompx[ row ];

   US_Model::SimulationComponent *sc = &cmodel.components[ mcx ];

   double extinction = le_extinction->text().toDouble();
   double signalConc = le_val_conc->text().toDouble();

   if (extinction > 0.0)
      sc->molar_concentration = signalConc / extinction;
   else
      sc->molar_concentration = 0.0;
}

void US_ConstraintsEdit::acceptProp(void) {
   DbgLv(1) << "cnG:aP:accept";
   QVector<C_CONSTRAINT> cnsv;
   save_comp_settings(crow, cnsv); // Save current page
   DbgLv(1) << "cnG:aP: svcs crow" << crow << "cnsv sz" << cnsv.count();
   constraints.update_constraints(cnsv); // Update comp constraints
   DbgLv(1) << "cnG:aP: upd_(c)cnst rtn";

   if (arow >= 0) {
      cnsv.clear();
      save_assoc_settings(arow, cnsv);
      DbgLv(1) << "cnG:aP: svas arow" << arow << "cnsv sz" << cnsv.count();
      constraints.update_constraints(cnsv); // Update assoc constraints
      DbgLv(1) << "cnG:aP: upd_(a)cnst rtn";
   }

   constraints.get_constr_model(&cmodel); // Get equivalent model
   DbgLv(1) << "cnG:aP: get_cmo RTN";

   accept();
   close();
}

void US_ConstraintsEdit::co_sed(int new_state) {
   if (inUpdate)
      return;

   if (new_state == Qt::Checked) {
      int row = lw_comps->currentRow();

      if (cmodel.coSedSolute != -1) {
         int response = QMessageBox::question(
            this, tr("Change co-sedimenting solute?"),
            tr("Another component is marked as the co-sedimenting solute.\n"
               "Change it to the current analyte?"),
            QMessageBox::Yes, QMessageBox::No);

         if (response == QMessageBox::No) {
            ck_co_sed->disconnect();
            ck_co_sed->setChecked(false);
            connect(ck_co_sed, SIGNAL(stateChanged(int)), SLOT(co_sed(int)));
            return;
         }
      }
      cmodel.coSedSolute = row;
   }
   else
      cmodel.coSedSolute = -1;
}

void US_ConstraintsEdit::source_changed(bool db) {
   emit use_db(db); // Just pass on the signal
   qApp->processEvents();
}

void US_ConstraintsEdit::check_mw(bool checked) {
   DbgLv(1) << "cnG:check_mw   checked" << checked;
   ck_flt_mw->setEnabled(checked);
   ck_log_mw->setEnabled(checked);

   check_selects();
}

void US_ConstraintsEdit::check_ff0(bool checked) {
   DbgLv(1) << "cnG:check_ff0  checked" << checked;
   ck_flt_ff0->setEnabled(checked);

   check_selects();
}

void US_ConstraintsEdit::check_s(bool checked) {
   DbgLv(1) << "cnG:check_s    checked" << checked;
   ck_flt_s->setEnabled(checked);
   check_selects();
}

void US_ConstraintsEdit::check_D(bool checked) {
   DbgLv(1) << "cnG:check_D    checked" << checked;
   ck_flt_D->setEnabled(checked);
   check_selects();
}

void US_ConstraintsEdit::check_f(bool checked) {
   DbgLv(1) << "cnG:check_f    checked" << checked;
   ck_flt_f->setEnabled(checked);
   check_selects();
}

void US_ConstraintsEdit::float_vbar(bool floats) {
   DbgLv(1) << "cnG:float_vbar floats" << floats;
   float_par(floats, le_val_vbar, le_min_vbar, le_max_vbar);
}

void US_ConstraintsEdit::float_mw(bool floats) {
   DbgLv(1) << "cnG:float_mw   floats" << floats;
   float_par(floats, le_val_mw, le_min_mw, le_max_mw);
}

void US_ConstraintsEdit::float_ff0(bool floats) {
   DbgLv(1) << "cnG:float_ff0  floats" << floats;
   float_par(floats, le_val_ff0, le_min_ff0, le_max_ff0);
}

void US_ConstraintsEdit::float_s(bool floats) {
   DbgLv(1) << "cnG:float_s    floats" << floats;
   float_par(floats, le_val_s, le_min_s, le_max_s);
}

void US_ConstraintsEdit::float_D(bool floats) {
   DbgLv(1) << "cnG:float_D    floats" << floats;
   float_par(floats, le_val_D, le_min_D, le_max_D);
}

void US_ConstraintsEdit::float_f(bool floats) {
   DbgLv(1) << "cnG:float_f    floats" << floats;
   float_par(floats, le_val_f, le_min_f, le_max_f);
}

void US_ConstraintsEdit::float_conc(bool floats) {
   DbgLv(1) << "cnG:float_conc floats" << floats;
   float_par(floats, le_val_conc, le_min_conc, le_max_conc);
}

void US_ConstraintsEdit::float_kd(bool floats) {
   DbgLv(1) << "cnG:float_kd   floats" << floats;
   float_par(floats, le_val_kd, le_min_kd, le_max_kd);
}

void US_ConstraintsEdit::float_koff(bool floats) {
   DbgLv(1) << "cnG:float_koff floats" << floats;
   float_par(floats, le_val_koff, le_min_koff, le_max_koff);
}

void US_ConstraintsEdit::logsc_mw(bool logscl) {
   DbgLv(1) << "cnG:logsc_mw   logscl" << logscl;
}

void US_ConstraintsEdit::logsc_kd(bool logscl) {
   DbgLv(1) << "cnG:logsc_kd   logscl" << logscl;
}
void US_ConstraintsEdit::logsc_koff(bool logscl) {
   DbgLv(1) << "cnG:logsc_koff logscl" << logscl;
}

// Utility to connect/disconnect component slots
void US_ConstraintsEdit::comps_connect(bool c_on) {
   if (c_on) { // Turn connections on
      connect(ck_flt_vbar, SIGNAL(toggled(bool)), SLOT(float_vbar(bool)));
      connect(ck_sel_mw, SIGNAL(toggled(bool)), SLOT(check_mw(bool)));
      connect(ck_flt_mw, SIGNAL(toggled(bool)), SLOT(float_mw(bool)));
      connect(ck_log_mw, SIGNAL(toggled(bool)), SLOT(logsc_mw(bool)));
      connect(ck_sel_ff0, SIGNAL(toggled(bool)), SLOT(check_ff0(bool)));
      connect(ck_flt_ff0, SIGNAL(toggled(bool)), SLOT(float_ff0(bool)));
      connect(ck_sel_s, SIGNAL(toggled(bool)), SLOT(check_s(bool)));
      connect(ck_flt_s, SIGNAL(toggled(bool)), SLOT(float_s(bool)));
      connect(ck_sel_D, SIGNAL(toggled(bool)), SLOT(check_D(bool)));
      connect(ck_flt_D, SIGNAL(toggled(bool)), SLOT(float_D(bool)));
      connect(ck_sel_f, SIGNAL(toggled(bool)), SLOT(check_f(bool)));
      connect(ck_flt_f, SIGNAL(toggled(bool)), SLOT(float_f(bool)));
      connect(ck_flt_conc, SIGNAL(toggled(bool)), SLOT(float_conc(bool)));
   }
   else { // Turn connections off
      ck_flt_vbar->disconnect();
      ck_sel_mw->disconnect();
      ck_flt_mw->disconnect();
      ck_log_mw->disconnect();
      ck_sel_ff0->disconnect();
      ck_flt_ff0->disconnect();
      ck_sel_s->disconnect();
      ck_flt_s->disconnect();
      ck_sel_D->disconnect();
      ck_flt_D->disconnect();
      ck_sel_f->disconnect();
      ck_flt_f->disconnect();
      ck_flt_conc->disconnect();
   }
}

// Utility to connect/disconnect component slots
void US_ConstraintsEdit::assocs_connect(bool c_on) {
   if (c_on) { // Turn connections on
      connect(ck_flt_kd, SIGNAL(toggled(bool)), SLOT(float_kd(bool)));
      connect(ck_log_kd, SIGNAL(toggled(bool)), SLOT(logsc_kd(bool)));
      connect(ck_flt_koff, SIGNAL(toggled(bool)), SLOT(float_koff(bool)));
      connect(ck_log_koff, SIGNAL(toggled(bool)), SLOT(logsc_koff(bool)));
   }
   else { // Turn connections off
      ck_flt_kd->disconnect();
      ck_log_kd->disconnect();
      ck_flt_koff->disconnect();
      ck_log_koff->disconnect();
   }
}

// Utility to count the selected main component attributes
int US_ConstraintsEdit::count_checks() {
   int nchecks = (ck_sel_mw->isChecked() ? 1 : 0) + (ck_sel_ff0->isChecked() ? 1 : 0) + (ck_sel_s->isChecked() ? 1 : 0)
                 + (ck_sel_D->isChecked() ? 1 : 0) + (ck_sel_f->isChecked() ? 1 : 0);
   return nchecks;
}

// Slot to populate component attribute values after component change
void US_ConstraintsEdit::component_select(int srow) {
   DbgLv(1) << "cnG: component_select  row" << srow << crow;
   QVector<C_CONSTRAINT> cnsv;

   if (srow < 0)
      return;

   if (crow >= 0) { // Save settings from previous component screen
      save_comp_settings(crow, cnsv);

      DbgLv(1) << "cnG:   update_constraints call";
      constraints.update_constraints(cnsv);
      DbgLv(1) << "cnG:   update_constraints  rtn";
   }

   if (srow == crow)
      return;

   // Get constraints for new component
   crow = srow;
   DbgLv(1) << "cnG:   comp_constraints call  row" << crow;
   constraints.comp_constraints(crow, &cnsv, NULL);
   DbgLv(1) << "cnG:   comp_constraints rtn   cnsv size" << cnsv.size();
   bool is_prod = cmodel.is_product(crow);
   bool not_prod = !is_prod;

   if (is_prod) { // Impose various restrictions for product component
      QVector<C_CONSTRAINT> old_cnsv = cnsv;
      cnsv.clear();
      DbgLv(1) << "cnG:   IS_PROD";

      // First scan associations to determine the reactant values
      double vsum = 0.0; // vbar sum
      double cval = 0.0; // concentration value
      double wsum = 0.0; // weight sum
      double esum = 0.0; // extinction sum
      int nreact = 0;

      for (int ii = 0; ii < cmodel.associations.size(); ii++) {
         US_Model::Association *as1 = &cmodel.associations[ ii ];
         QVector<int> *rcomps = &as1->rcomps;
         DbgLv(1) << "cnG:    ii" << ii << "rc0" << rcomps->at(0) << "crow" << crow;
         if (rcomps->at(0) < 0)
            continue;

         if (rcomps->contains(crow)) {
            QVector<int> *stoichs = &as1->stoichs;
            DbgLv(1) << "cnG:    ii" << ii << "rc-cont-crow: rcsize stsize" << rcomps->size() << stoichs->size();

            for (int jj = 0; jj < rcomps->size(); jj++) {
               int rc1 = rcomps->at(jj);
               int st1 = stoichs->at(jj);
               DbgLv(1) << "cnG:      jj" << jj << "rc1" << rc1 << "st1" << st1;
               if (rc1 >= 0 && st1 > 0) {
                  // Get reactant's constraints
                  QVector<C_CONSTRAINT> rcnsv;
                  constraints.comp_constraints(rc1, &rcnsv, NULL);

                  nreact++;
                  int mcx = lcompx[ rc1 ];
                  double rst1 = ( double ) st1;
                  double mw = constr_value(C_ATYPE_MW, rcnsv) * rst1;
                  double vbar = constr_value(C_ATYPE_VBAR, rcnsv) * mw;
                  double conc = constr_value(C_ATYPE_CONC, rcnsv);
                  double extinc = constr_value(C_ATYPE_EXT, rcnsv) * rst1;
                  cval = (cval == 0.0) ? conc : cval;
                  vsum += vbar;
                  wsum += mw;
                  esum += extinc;
                  DbgLv(1) << "cnG:      mcx" << mcx << "cval,vsum,wsum,esum" << cval << vsum << wsum << esum;
               }
            }

            break;
         }
      }

      vsum /= wsum;
      bool miss_vb = true;
      bool miss_co = true;
      bool miss_mw = true;
      bool miss_ff = true;
      C_CONSTRAINT cnse;

      // Replace product constraints with appropriate ones

      for (int ii = 0; ii < old_cnsv.size(); ii++) {
         cnse = old_cnsv[ ii ];
         C_ATYPE atype = cnse.atype;

         if (atype == C_ATYPE_VBAR) { // Replace vbar
            cnse.low = vsum;
            cnse.high = cnse.low;
            cnse.floats = false;
            cnsv << cnse;
            miss_vb = false;
         }

         else if (atype == C_ATYPE_MW) { // Replace mw
            cnse.low = wsum;
            cnse.high = cnse.low;
            cnse.floats = false;
            cnsv << cnse;
            miss_mw = false;
         }

         else if (atype == C_ATYPE_FF0) { // Just copy f/f0 as is
            cnsv << cnse;
            miss_ff = false;
         }

         else if (atype == C_ATYPE_CONC) { // Replace concentration
            cnse.low = cval;
            cnse.high = cnse.low;
            cnse.floats = false;
            cnsv << cnse;
            miss_co = false;
         }

         else if (atype == C_ATYPE_EXT) { // Replace extinction
            cnse.low = esum;
            cnse.high = cnse.low;
            cnse.floats = false;
            cnsv << cnse;
         }
      }
      DbgLv(1) << "cnG: miss_vb,mw,ff,co" << miss_vb << miss_mw << miss_ff << miss_co;

      // Insure product constraints includes needed values
      if (miss_vb) { // Supply missing vbar constraint
         cnse.atype = C_ATYPE_VBAR;
         cnse.low = vsum;
         cnse.high = cnse.low;
         cnse.floats = false;
         cnse.logscl = false;
         cnsv << cnse;
      }

      if (miss_mw) { // Supply missing mw constraint
         cnse.atype = C_ATYPE_MW;
         cnse.low = wsum;
         cnse.high = cnse.low;
         cnse.floats = false;
         cnse.logscl = false;
         cnsv << cnse;
      }

      if (miss_ff) { // Supply missing f/f0 constraint
         cnse.atype = C_ATYPE_FF0;
         cnse.low = 1.8;
         cnse.high = cnse.low;
         cnse.floats = false;
         cnse.logscl = false;
         cnsv << cnse;
      }

      if (miss_co) { // Supply missing concentration constraint
         cnse.atype = C_ATYPE_CONC;
         cnse.low = cval;
         cnse.high = cval;
         cnse.floats = false;
         cnse.logscl = false;
         cnsv << cnse;
      }
   }

   comps_connect(false);
   DbgLv(1) << "cnG:   comp_constraints  rtn";
   //QListWidgetItem* item = lw_comps->item( crow );

   // Initialize component attribute GUI elements
   ck_sel_vbar->setChecked(true);
   ck_sel_mw->setChecked(false);
   ck_sel_ff0->setChecked(false);
   ck_sel_s->setChecked(false);
   ck_sel_D->setChecked(false);
   ck_sel_f->setChecked(false);
   ck_sel_conc->setChecked(true);
   ck_sel_mw->setEnabled(false);
   ck_sel_ff0->setEnabled(false);
   ck_sel_s->setEnabled(false);
   ck_sel_D->setEnabled(false);
   ck_sel_f->setEnabled(false);
   us_setReadOnly(le_val_vbar, true);
   us_setReadOnly(le_min_vbar, true);
   us_setReadOnly(le_max_vbar, true);
   us_setReadOnly(le_val_mw, true);
   us_setReadOnly(le_min_mw, true);
   us_setReadOnly(le_max_mw, true);
   us_setReadOnly(le_val_ff0, true);
   us_setReadOnly(le_min_ff0, true);
   us_setReadOnly(le_max_ff0, true);
   us_setReadOnly(le_val_s, true);
   us_setReadOnly(le_min_s, true);
   us_setReadOnly(le_max_s, true);
   us_setReadOnly(le_val_D, true);
   us_setReadOnly(le_min_D, true);
   us_setReadOnly(le_max_D, true);
   us_setReadOnly(le_val_f, true);
   us_setReadOnly(le_min_f, true);
   us_setReadOnly(le_max_f, true);
   us_setReadOnly(le_val_conc, true);
   us_setReadOnly(le_min_conc, true);
   us_setReadOnly(le_max_conc, true);
   ck_flt_vbar->setEnabled(not_prod);
   ck_flt_mw->setEnabled(false);
   ck_flt_ff0->setEnabled(false);
   ck_flt_s->setEnabled(false);
   ck_flt_D->setEnabled(false);
   ck_flt_f->setEnabled(false);
   ck_flt_conc->setEnabled(not_prod);

   // Populate component attribute values
   for (int ii = 0; ii < cnsv.size(); ii++) {
      bool floats = cnsv[ ii ].floats;

      DbgLv(1) << "cnG:cmp_sel: ii" << ii << "atype" << cnsv[ ii ].atype << "fl" << floats;
      if (cnsv[ ii ].atype == C_ATYPE_VBAR) {
         ck_sel_vbar->setChecked(true);
         ck_sel_vbar->setEnabled(false);
         check_value(cnsv[ ii ], le_val_vbar, le_min_vbar, le_max_vbar);
         ck_flt_vbar->setChecked(floats);
         ck_flt_vbar->setEnabled(not_prod);
      }
      if (cnsv[ ii ].atype == C_ATYPE_MW) {
         ck_sel_mw->setChecked(true);
         ck_sel_mw->setEnabled(not_prod);
         check_value(cnsv[ ii ], le_val_mw, le_min_mw, le_max_mw);
         ck_flt_mw->setChecked(floats);
         ck_log_mw->setChecked(cnsv[ ii ].logscl);
         ck_flt_mw->setEnabled(not_prod);
         ck_log_mw->setEnabled(not_prod);
      }
      if (cnsv[ ii ].atype == C_ATYPE_FF0) {
         ck_sel_ff0->setChecked(true);
         ck_sel_ff0->setEnabled(true);
         check_value(cnsv[ ii ], le_val_ff0, le_min_ff0, le_max_ff0);
         ck_flt_ff0->setChecked(floats);
         ck_flt_ff0->setEnabled(true);
      }
      if (cnsv[ ii ].atype == C_ATYPE_S) {
         ck_sel_s->setChecked(true);
         ck_sel_s->setEnabled(not_prod);
         check_value(cnsv[ ii ], le_val_s, le_min_s, le_max_s);
         ck_flt_s->setChecked(floats);
         ck_flt_s->setEnabled(not_prod);
      }
      if (cnsv[ ii ].atype == C_ATYPE_D) {
         ck_sel_D->setChecked(true);
         ck_sel_D->setEnabled(not_prod);
         check_value(cnsv[ ii ], le_val_D, le_min_D, le_max_D);
         ck_flt_D->setChecked(floats);
         ck_flt_D->setEnabled(not_prod);
      }
      if (cnsv[ ii ].atype == C_ATYPE_F) {
         ck_sel_f->setChecked(true);
         ck_sel_f->setEnabled(not_prod);
         check_value(cnsv[ ii ], le_val_f, le_min_f, le_max_f);
         ck_flt_f->setChecked(floats);
         ck_flt_f->setEnabled(true);
      }
      if (cnsv[ ii ].atype == C_ATYPE_CONC) {
         check_value(cnsv[ ii ], le_val_conc, le_min_conc, le_max_conc);
         ck_flt_conc->setChecked(floats);
         ck_flt_conc->setEnabled(not_prod);
      }
      if (cnsv[ ii ].atype == C_ATYPE_EXT) {
         le_extinction->setText(QString::number(cnsv[ ii ].low));
      }
      if (is_prod) {
         us_setReadOnly(le_val_vbar, true);
         us_setReadOnly(le_min_vbar, true);
         us_setReadOnly(le_max_vbar, true);
         us_setReadOnly(le_val_mw, true);
         us_setReadOnly(le_min_mw, true);
         us_setReadOnly(le_max_mw, true);
         us_setReadOnly(le_val_conc, true);
         us_setReadOnly(le_min_conc, true);
         us_setReadOnly(le_max_conc, true);
      }
   }

   // Set oligomer and reactant,product flags
   le_oligomer->setText(QString::number(cmodel.components[ lcompx[ crow ] ].oligomer));
   ck_isreact->setChecked(cmodel.is_reactant(crow));
   ck_isprod->setChecked(cmodel.is_product(crow));

   check_selects();
   comps_connect(true);
}

// Slot to populate association attribute values after association change
void US_ConstraintsEdit::association_select(int srow) {
   DbgLv(1) << "cnG: association_select  row" << srow << arow;
   QVector<C_CONSTRAINT> cnsv;

   if (srow < 0)
      return;

   if (srow != arow && arow >= 0) { // Save settings from previous component screen
      save_assoc_settings(arow, cnsv);

      constraints.update_constraints(cnsv);
   }

   // Get constraints for new association
   arow = srow;
   constraints.assoc_constraints(arow, &cnsv, NULL);
   assocs_connect(false);

   // Populate association attribute values
   ck_flt_kd->setChecked(false);
   ck_flt_koff->setChecked(false);
   ck_log_kd->setChecked(false);
   ck_log_koff->setChecked(false);

   for (int ii = 0; ii < cnsv.size(); ii++) {
      bool floats = cnsv[ ii ].floats;
      bool logscl = cnsv[ ii ].logscl;

      if (cnsv[ ii ].atype == C_ATYPE_KD) {
         check_value(cnsv[ ii ], le_val_kd, le_min_kd, le_max_kd);
         ck_flt_kd->setChecked(floats);
         ck_log_kd->setChecked(logscl);
      }

      if (cnsv[ ii ].atype == C_ATYPE_KOFF) {
         check_value(cnsv[ ii ], le_val_koff, le_min_koff, le_max_koff);
         ck_flt_koff->setChecked(floats);
         ck_log_koff->setChecked(logscl);
      }
   }

   assocs_connect(true);
}

// Internal function to save current page's component settings
void US_ConstraintsEdit::save_comp_settings(int crow, QVector<C_CONSTRAINT> &cnsv) {
   DbgLv(1) << "cnG: svcs: save_comp_settings  crow" << crow;
   C_CONSTRAINT cnse;
   cnse.mcompx = crow;
   bool floats = false;
   int kselect = 0;

   // Save any s selection
   if (ck_sel_s->isChecked()) {
      floats = ck_flt_s->isChecked();
      cnse.atype = C_ATYPE_S;
      cnse.low = le_val_s->text().toDouble();
      cnse.low = floats ? le_min_s->text().toDouble() : cnse.low;
      cnse.high = floats ? le_max_s->text().toDouble() : cnse.low;
      cnse.floats = floats;
      cnse.logscl = false;
      cnsv << cnse;
      kselect++;
      DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "S";
   }
   // Save any f/f0 selection
   if (ck_sel_ff0->isChecked()) {
      floats = ck_flt_ff0->isChecked();
      cnse.atype = C_ATYPE_FF0;
      cnse.low = le_val_ff0->text().toDouble();
      cnse.low = floats ? le_min_ff0->text().toDouble() : cnse.low;
      cnse.high = floats ? le_max_ff0->text().toDouble() : cnse.low;
      cnse.floats = floats;
      cnse.logscl = false;
      cnsv << cnse;
      kselect++;
      DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "FF0";
   }
   // Save any mw selection
   if (ck_sel_mw->isChecked()) {
      floats = ck_flt_mw->isChecked();
      cnse.atype = C_ATYPE_MW;
      cnse.low = le_val_mw->text().toDouble();
      cnse.low = floats ? le_min_mw->text().toDouble() : cnse.low;
      cnse.high = floats ? le_max_mw->text().toDouble() : cnse.low;
      cnse.floats = floats;
      cnse.logscl = ck_log_mw->isChecked();
      cnsv << cnse;
      kselect++;
      DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "MW";
   }
   // Save any D selection
   if (ck_sel_D->isChecked()) {
      floats = ck_flt_D->isChecked();
      cnse.atype = C_ATYPE_D;
      cnse.low = le_val_D->text().toDouble();
      cnse.low = floats ? le_min_D->text().toDouble() : cnse.low;
      cnse.high = floats ? le_max_D->text().toDouble() : cnse.low;
      cnse.floats = floats;
      cnse.logscl = false;
      cnsv << cnse;
      kselect++;
      DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "D";
   }
   // Save any f selection
   if (ck_sel_f->isChecked()) {
      floats = ck_flt_f->isChecked();
      cnse.atype = C_ATYPE_F;
      cnse.low = le_val_f->text().toDouble();
      cnse.low = floats ? le_min_f->text().toDouble() : cnse.low;
      cnse.high = floats ? le_max_f->text().toDouble() : cnse.low;
      cnse.floats = floats;
      cnse.logscl = false;
      cnsv << cnse;
      kselect++;
      DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "F";
   }

   // Save the vbar constraints
   floats = ck_flt_vbar->isChecked();
   cnse.atype = C_ATYPE_VBAR;
   cnse.low = le_val_vbar->text().toDouble();
   cnse.low = floats ? le_min_vbar->text().toDouble() : cnse.low;
   cnse.high = floats ? le_max_vbar->text().toDouble() : cnse.low;
   cnse.floats = floats;
   cnse.logscl = false;
   cnsv << cnse;
   kselect++;
   DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "VBAR";

   // Save the concentration constraints
   floats = ck_flt_conc->isChecked();
   cnse.atype = C_ATYPE_CONC;
   cnse.low = le_val_conc->text().toDouble();
   cnse.low = floats ? le_min_conc->text().toDouble() : cnse.low;
   cnse.high = floats ? le_max_conc->text().toDouble() : cnse.low;
   cnse.floats = floats;
   cnse.logscl = false;
   cnsv << cnse;
   kselect++;
   DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << floats << "CONC";

   // There should be a total of 4 checked
   if (kselect != 4) {
      qDebug() << "*ERROR* dmGA_Init: component" << cnse.mcompx + 1 << "has" << kselect << "attributes selected (SB 4)";
   }

   // Add one more entry for extinction
   cnse.atype = C_ATYPE_EXT;
   cnse.low = le_extinction->text().toDouble();
   cnse.high = cnse.low;
   cnse.floats = false;
   cnse.logscl = false;
   cnsv << cnse;
   kselect++;
   DbgLv(1) << "cnG: svcs:  ks" << kselect << "flt" << cnse.floats << "EXT"
            << "cnsv size" << cnsv.size();
}

// Internal function to save current page's association settings
void US_ConstraintsEdit::save_assoc_settings(int arow, QVector<C_CONSTRAINT> &cnsv) {
   C_CONSTRAINT cnse;
   bool floats = ck_flt_kd->isChecked();
   cnse.mcompx = arow;
   cnse.atype = C_ATYPE_KD;
   cnse.low = le_val_kd->text().toDouble();
   cnse.low = floats ? le_min_kd->text().toDouble() : cnse.low;
   cnse.high = floats ? le_max_kd->text().toDouble() : cnse.low;
   cnse.floats = floats;
   cnse.logscl = ck_log_kd->isChecked();
   cnsv << cnse;
   DbgLv(1) << "cnG: svas:  ks 1  flt" << floats << "KD";

   floats = ck_flt_koff->isChecked();
   cnse.atype = C_ATYPE_KOFF;
   cnse.low = le_val_koff->text().toDouble();
   cnse.low = floats ? le_min_koff->text().toDouble() : cnse.low;
   cnse.high = floats ? le_max_koff->text().toDouble() : cnse.low;
   cnse.floats = floats;
   cnse.logscl = ck_log_koff->isChecked();
   cnsv << cnse;
   DbgLv(1) << "cnG: svas:  ks 2  flt" << floats << "KOFF";
}

// Switch between value and low,high based on float check
void US_ConstraintsEdit::float_par(bool floats, QLineEdit *le_val, QLineEdit *le_min, QLineEdit *le_max) {
   bool fixed = !floats;

   DbgLv(1) << "cnG:float_par floats" << floats;
   if (floats) { // Changed to float:  at least 1 float, so capable of saving constraints
      pb_accept->setEnabled(true);
   }

   else { // Changed to fixed:  value only
      // Disable Accept if no floats remaining
      bool have_fl = false;
      have_fl = (have_fl || ck_flt_vbar->isChecked());
      have_fl = (have_fl || ck_flt_mw->isChecked());
      have_fl = (have_fl || ck_flt_ff0->isChecked());
      have_fl = (have_fl || ck_flt_s->isChecked());
      have_fl = (have_fl || ck_flt_D->isChecked());
      have_fl = (have_fl || ck_flt_f->isChecked());
      have_fl = (have_fl || ck_flt_conc->isChecked());
      have_fl = (have_fl || ck_flt_kd->isChecked());
      have_fl = (have_fl || ck_flt_koff->isChecked());
      pb_accept->setEnabled(have_fl);
   }

   le_val->setEnabled(fixed);
   le_min->setEnabled(floats);
   le_max->setEnabled(floats);
   us_setReadOnly(le_val, floats);
   us_setReadOnly(le_min, fixed);
   us_setReadOnly(le_max, fixed);
}

// Set value and low,high states based on constraints entry
void US_ConstraintsEdit::check_value(const C_CONSTRAINT cnse, QLineEdit *le_val, QLineEdit *le_min, QLineEdit *le_max) {
   bool floats = cnse.floats;
   bool fixed = !floats;
   DbgLv(1) << "cnG:check_value: floats" << floats << "atype" << cnse.atype << "low high" << cnse.low << cnse.high;

   if (fixed) {
      le_val->setText(QString::number(cnse.low));
      double vmin = cnse.low * 0.9;
      double vmax = cnse.low * 1.1;
      le_min->setText(QString::number(vmin));
      le_max->setText(QString::number(vmax));
   }

   else {
      le_min->setText(QString::number(cnse.low));
      le_max->setText(QString::number(cnse.high));
      double vval = (cnse.low + cnse.high) * 0.5;
      le_val->setText(QString::number(vval));
   }

   us_setReadOnly(le_val, floats);
   us_setReadOnly(le_min, fixed);
   us_setReadOnly(le_max, fixed);
}

// Set state of selectable component attributes based on overall picture
void US_ConstraintsEdit::check_selects() {
   int kcheck = count_checks();
   bool ckd_mw = ck_sel_mw->isChecked();
   bool ckd_ff0 = ck_sel_ff0->isChecked();
   bool ckd_s = ck_sel_s->isChecked();
   bool ckd_D = ck_sel_D->isChecked();
   bool ckd_f = ck_sel_f->isChecked();
   bool flt_vbar = ck_flt_vbar->isChecked();
   bool flt_mw = ck_flt_mw->isChecked();
   bool flt_ff0 = ck_flt_ff0->isChecked();
   bool flt_s = ck_flt_s->isChecked();
   bool flt_D = ck_flt_D->isChecked();
   bool flt_f = ck_flt_f->isChecked();
   bool flt_any = flt_vbar || flt_mw || flt_ff0 || flt_s || flt_D || flt_f;
   DbgLv(1) << "cnG:check_selects kcheck" << kcheck << "flt_any" << flt_any;

   // Read-only based on selected state
   us_setReadOnly(le_val_mw, (!ckd_mw || flt_mw));
   us_setReadOnly(le_min_mw, (!ckd_mw || !flt_mw));
   us_setReadOnly(le_max_mw, (!ckd_mw || !flt_mw));
   us_setReadOnly(le_val_ff0, (!ckd_ff0 || flt_ff0));
   us_setReadOnly(le_min_ff0, (!ckd_ff0 || !flt_ff0));
   us_setReadOnly(le_max_ff0, (!ckd_ff0 || !flt_ff0));
   us_setReadOnly(le_val_s, (!ckd_s || flt_s));
   us_setReadOnly(le_min_s, (!ckd_s || !flt_s));
   us_setReadOnly(le_max_s, (!ckd_s || !flt_s));
   us_setReadOnly(le_val_D, (!ckd_D || flt_D));
   us_setReadOnly(le_min_D, (!ckd_D || !flt_D));
   us_setReadOnly(le_max_D, (!ckd_D || !flt_D));
   us_setReadOnly(le_val_f, (!ckd_f || flt_f));
   us_setReadOnly(le_min_f, (!ckd_f || !flt_f));
   us_setReadOnly(le_max_f, (!ckd_f || !flt_f));

   if (kcheck == 2) { // Right number of attributes are selected
      int ksel = 0;
      US_Model::SimulationComponent scva;
      scva.vbar20 = 0.0;
      scva.mw = 0.0;
      scva.s = 0.0;
      scva.D = 0.0;
      scva.f = 0.0;
      scva.f_f0 = 0.0;
      US_Model::SimulationComponent scll = scva;
      US_Model::SimulationComponent schh = scva;
      US_Model::SimulationComponent sclh = scva;
      US_Model::SimulationComponent schl = scva;
      DbgLv(1) << "cnG:ck_sels: ff0 B0" << scva.f_f0;

      // Set the selected component values
      scva.vbar20 = le_val_vbar->text().toDouble();
      scll.vbar20 = le_min_vbar->text().toDouble();
      schh.vbar20 = le_max_vbar->text().toDouble();

      sclh.vbar20 = scll.vbar20;
      schl.vbar20 = schh.vbar20;
      DbgLv(1) << "cnG:ck_sels: ff0 B1" << scva.f_f0;

      if (ckd_mw) {
         scva.mw = le_val_mw->text().toDouble();
         scll.mw = le_min_mw->text().toDouble();
         schh.mw = le_max_mw->text().toDouble();
         sclh.mw = scll.mw;
         schl.mw = schh.mw;
         ksel++;
      }

      if (ckd_ff0) {
         scva.f_f0 = le_val_ff0->text().toDouble();
         scll.f_f0 = le_min_ff0->text().toDouble();
         schh.f_f0 = le_max_ff0->text().toDouble();
         sclh.f_f0 = (ksel == 0) ? scll.f_f0 : schh.f_f0;
         schl.f_f0 = (ksel == 0) ? schh.f_f0 : scll.f_f0;
         ksel++;
      }
      DbgLv(1) << "cnG:ck_sels: ff0 B3" << scva.f_f0;

      if (ckd_s) {
         scva.s = le_val_s->text().toDouble();
         scll.s = le_min_s->text().toDouble();
         schh.s = le_max_s->text().toDouble();
         sclh.s = (ksel == 0) ? scll.s : schh.s;
         schl.s = (ksel == 0) ? schh.s : scll.s;
         ksel++;
      }

      if (ckd_D) {
         scva.D = le_val_D->text().toDouble();
         scll.D = le_min_D->text().toDouble();
         schh.D = le_max_D->text().toDouble();
         sclh.D = (ksel == 0) ? scll.D : schh.D;
         schl.D = (ksel == 0) ? schh.D : scll.D;
         ksel++;
      }
      DbgLv(1) << "cnG:ck_sels: ff0 B6" << scva.f_f0;

      if (ckd_f) {
         scva.f = le_val_f->text().toDouble();
         scll.f = le_min_f->text().toDouble();
         schh.f = le_max_f->text().toDouble();
         sclh.f = (ksel == 0) ? scll.f : schh.f;
         schl.f = (ksel == 0) ? schh.f : scll.f;
         ksel++;
      }

      // Compute unselected values
      DbgLv(1) << "cnG:ck_sels: ff0 BEF" << scva.f_f0 << scll.f_f0 << schh.f_f0 << sclh.f_f0 << schl.f_f0 << "ksel"
               << ksel;
      US_Model::calc_coefficients(scva);
      US_Model::calc_coefficients(scll);
      US_Model::calc_coefficients(schh);
      US_Model::calc_coefficients(sclh);
      US_Model::calc_coefficients(schl);
      DbgLv(1) << "cnG:ck_sels: ff0 AFT" << scva.f_f0 << scll.f_f0 << schh.f_f0 << sclh.f_f0 << schl.f_f0;

      // Fill unselected attribute value fields
      if (!ckd_mw) {
         double vmin = qMin(scll.mw, sclh.mw);
         double vmax = qMax(scll.mw, sclh.mw);
         vmin = qMin(vmin, qMin(schh.mw, schl.mw));
         vmax = qMax(vmax, qMax(schh.mw, schl.mw));
         le_val_mw->setText(QString::number(scva.mw));
         le_min_mw->setText(QString::number(vmin));
         le_max_mw->setText(QString::number(vmax));
      }

      if (!ckd_ff0) {
         double vmin = qMin(scll.f_f0, sclh.f_f0);
         double vmax = qMax(scll.f_f0, sclh.f_f0);
         vmin = qMin(vmin, qMin(schh.f_f0, schl.f_f0));
         vmax = qMax(vmax, qMax(schh.f_f0, schl.f_f0));
         le_val_ff0->setText(QString::number(scva.f_f0));
         le_min_ff0->setText(QString::number(vmin));
         le_max_ff0->setText(QString::number(vmax));
      }

      if (!ckd_s) {
         double vmin = qMin(scll.s, sclh.s);
         double vmax = qMax(scll.s, sclh.s);
         vmin = qMin(vmin, qMin(schh.s, schl.s));
         vmax = qMax(vmax, qMax(schh.s, schl.s));
         le_val_s->setText(QString::number(scva.s));
         le_min_s->setText(QString::number(vmin));
         le_max_s->setText(QString::number(vmax));
         DbgLv(1) << "cnG:check_selects !CKD_S  flt_any" << flt_any;
      }

      if (!ckd_D) {
         double vmin = qMin(scll.D, sclh.D);
         double vmax = qMax(scll.D, sclh.D);
         vmin = qMin(vmin, qMin(schh.D, schl.D));
         vmax = qMax(vmax, qMax(schh.D, schl.D));
         le_val_D->setText(QString::number(scva.D));
         le_min_D->setText(QString::number(vmin));
         le_max_D->setText(QString::number(vmax));
      }

      if (!ckd_f) {
         double vmin = qMin(scll.f, sclh.f);
         double vmax = qMax(scll.f, sclh.f);
         vmin = qMin(vmin, qMin(schh.f, schl.f));
         vmax = qMax(vmax, qMax(schh.f, schl.f));
         le_val_f->setText(QString::number(scva.f));
         le_min_f->setText(QString::number(vmin));
         le_max_f->setText(QString::number(vmax));
      }

      // Only enable checks to be unchecked
      ck_sel_mw->setEnabled(ckd_mw);
      ck_sel_ff0->setEnabled(ckd_ff0);
      ck_sel_s->setEnabled(ckd_s);
      ck_sel_D->setEnabled(ckd_D);
      ck_sel_f->setEnabled(ckd_f);
   }

   else { // Attribute newly unselected, enable ability to check a different one
      ck_sel_mw->setEnabled(true);
      ck_sel_ff0->setEnabled(true);
      ck_sel_s->setEnabled(true);
      ck_sel_D->setEnabled(true);
      ck_sel_f->setEnabled(true);
   }
   DbgLv(1) << "cnG:check_selects   END";
}

// Get the value of a specified type in a constraints record
double US_ConstraintsEdit::constr_value(const C_ATYPE atype, QVector<C_CONSTRAINT> &cnsv) {
   double cval = 0.0;

   for (int ii = 0; ii < cnsv.size(); ii++) {
      if (cnsv[ ii ].atype == atype) { // Found the type:  return its value or range average
         double cmin = cnsv[ ii ].low;
         double cmax = cnsv[ ii ].high;

         if (cnsv[ ii ].floats) {
            cval = cnsv[ ii ].logscl ? exp((log(cmin) + log(cmax)) * 0.5) : ((cmin + cmax) * 0.5);
         }

         else {
            cval = cmin;
         }
         break;
      }
   }

   return cval;
}
