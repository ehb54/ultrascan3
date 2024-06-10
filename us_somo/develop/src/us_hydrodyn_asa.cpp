#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_unicode.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>

US_Hydrodyn_ASA::US_Hydrodyn_ASA(struct asa_options *asa, bool *asa_widget, void *us_hydrodyn, QWidget *p, const char *) : QFrame( p )
{
   this->asa = asa;
   this->asa_widget = asa_widget;
   this->us_hydrodyn = us_hydrodyn;
   *asa_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SOMO Accessible Surface Area Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_ASA::~US_Hydrodyn_ASA()
{
   *asa_widget = false;
}

void US_Hydrodyn_ASA::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(us_tr("Accessible Surface Area Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

#if QT_VERSION < 0x040000
   bg_asa_method = new QGroupBox(2, Qt::Vertical, "ASA Method:", this);
   bg_asa_method->setExclusive(true);
   connect(bg_asa_method, SIGNAL(clicked(int)), this, SLOT(select_asa_method(int)));

   cb_surfracer = new QCheckBox(bg_asa_method);
   cb_surfracer->setText(us_tr(" Voronoi Tesselation (Surfrace, Tsodikov et al.)"));
   cb_surfracer->setEnabled(true);
   cb_surfracer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_surfracer->setPalette( PALET_NORMAL );
   AUTFBACK( cb_surfracer );

   cb_asab1 = new QCheckBox(bg_asa_method);
   cb_asab1->setText(us_tr(" Rolling Sphere (ASAB1, Lee && Richards' Method)"));
   cb_asab1->setEnabled(true);
   cb_asab1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_asab1->setPalette( PALET_NORMAL );
   AUTFBACK( cb_asab1 );

   bg_asa_method->setButton((*asa).method);
#else
   bg_asa_method = new QGroupBox("ASA Method:");

   rb_surfracer = new QRadioButton();
   rb_surfracer->setText(us_tr(" Voronoi Tesselation (Surfrace, Tsodikov et al.)"));
   rb_surfracer->setEnabled(true);
   rb_surfracer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_surfracer->setPalette( PALET_NORMAL );
   AUTFBACK( rb_surfracer );
   connect( rb_surfracer, SIGNAL( clicked() ), this, SLOT( select_asa_method() ) );
   

   rb_asab1 = new QRadioButton();
   rb_asab1->setText(us_tr(" Rolling Sphere (ASAB1, Lee && Richards' Method)"));
   rb_asab1->setEnabled(true);
   rb_asab1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   rb_asab1->setPalette( PALET_NORMAL );
   AUTFBACK( rb_asab1 );
   connect( rb_asab1, SIGNAL( clicked() ), this, SLOT( select_asa_method() ) );

   {
      QVBoxLayout * bl = new QVBoxLayout; bl->setContentsMargins( 0, 0, 0, 0 ); bl->setSpacing( 0 );
      bl->addWidget( rb_surfracer );
      bl->addWidget( rb_asab1 );
      bg_asa_method->setLayout( bl );
   }

   switch( (*asa).method ) {
   case 0 : rb_surfracer->setChecked( true ); break;
   case 1 : rb_asab1->setChecked( true ); break;
   default : qDebug() << "asa missing asa method selection error"; break;
   }
   
#endif
   
   lbl_probe_radius = new QLabel(us_tr(" ASA Probe Radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   Q_CHECK_PTR(lbl_probe_radius);
   lbl_probe_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_probe_radius->setMinimumHeight(minHeight1);
   lbl_probe_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_probe_radius );
   lbl_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_probe_radius= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_probe_radius );
   Q_CHECK_PTR(cnt_probe_radius);
   cnt_probe_radius->setRange(0, 10); cnt_probe_radius->setSingleStep( 0.01);
   cnt_probe_radius->setValue((*asa).probe_radius);
   cnt_probe_radius->setMinimumHeight(minHeight1);
   cnt_probe_radius->setEnabled(true);
   cnt_probe_radius->setNumButtons(3);
   cnt_probe_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_probe_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_probe_radius );
   connect(cnt_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_probe_radius(double)));

   lbl_probe_recheck_radius = new QLabel(us_tr(" ASA Probe Recheck Radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   Q_CHECK_PTR(lbl_probe_recheck_radius);
   lbl_probe_recheck_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_probe_recheck_radius->setMinimumHeight(minHeight1);
   lbl_probe_recheck_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_probe_recheck_radius );
   lbl_probe_recheck_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_probe_recheck_radius= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_probe_recheck_radius );
   Q_CHECK_PTR(cnt_probe_recheck_radius);
   cnt_probe_recheck_radius->setRange(0, 10); cnt_probe_recheck_radius->setSingleStep( 0.01);
   cnt_probe_recheck_radius->setValue((*asa).probe_recheck_radius);
   cnt_probe_recheck_radius->setMinimumHeight(minHeight1);
   cnt_probe_recheck_radius->setEnabled(true);
   cnt_probe_recheck_radius->setNumButtons(3);
   cnt_probe_recheck_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_probe_recheck_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_probe_recheck_radius );
   connect(cnt_probe_recheck_radius, SIGNAL(valueChanged(double)), SLOT(update_probe_recheck_radius(double)));

   lbl_asa_threshold = new QLabel(us_tr(" SOMO ASA Threshold [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_2 + "]: "), this);
   Q_CHECK_PTR(lbl_asa_threshold);
   lbl_asa_threshold->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asa_threshold->setMinimumHeight(minHeight1);
   lbl_asa_threshold->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asa_threshold );
   lbl_asa_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_asa_threshold= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_asa_threshold );
   Q_CHECK_PTR(cnt_asa_threshold);
   cnt_asa_threshold->setRange(0, 100); cnt_asa_threshold->setSingleStep( 0.1);
   cnt_asa_threshold->setValue((*asa).threshold);
   cnt_asa_threshold->setMinimumHeight(minHeight1);
   cnt_asa_threshold->setEnabled(true);
   cnt_asa_threshold->setNumButtons(3);
   cnt_asa_threshold->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_asa_threshold->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_asa_threshold );
   connect(cnt_asa_threshold, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold(double)));

   lbl_asa_threshold_percent = new QLabel(us_tr(" SOMO Bead ASA Threshold %: "), this);
   Q_CHECK_PTR(lbl_asa_threshold_percent);
   lbl_asa_threshold_percent->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asa_threshold_percent->setMinimumHeight(minHeight1);
   lbl_asa_threshold_percent->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asa_threshold_percent );
   lbl_asa_threshold_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_asa_threshold_percent= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_asa_threshold_percent );
   Q_CHECK_PTR(cnt_asa_threshold_percent);
   cnt_asa_threshold_percent->setRange(0, 100); cnt_asa_threshold_percent->setSingleStep( 0.1);
   cnt_asa_threshold_percent->setValue((*asa).threshold_percent);
   cnt_asa_threshold_percent->setMinimumHeight(minHeight1);
   cnt_asa_threshold_percent->setEnabled(true);
   cnt_asa_threshold_percent->setNumButtons(3);
   cnt_asa_threshold_percent->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_asa_threshold_percent->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_asa_threshold_percent );
   connect(cnt_asa_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_asa_threshold_percent(double)));

   lbl_asa_grid_threshold = new QLabel(us_tr(" Grid ASA Threshold [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_2 + "]: "), this);
   Q_CHECK_PTR(lbl_asa_grid_threshold);
   lbl_asa_grid_threshold->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asa_grid_threshold->setMinimumHeight(minHeight1);
   lbl_asa_grid_threshold->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asa_grid_threshold );
   lbl_asa_grid_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_asa_grid_threshold= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_asa_grid_threshold );
   Q_CHECK_PTR(cnt_asa_grid_threshold);
   cnt_asa_grid_threshold->setRange(0, 100); cnt_asa_grid_threshold->setSingleStep( 0.1);
   cnt_asa_grid_threshold->setValue((*asa).grid_threshold);
   cnt_asa_grid_threshold->setMinimumHeight(minHeight1);
   cnt_asa_grid_threshold->setEnabled(true);
   cnt_asa_grid_threshold->setNumButtons(3);
   cnt_asa_grid_threshold->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_asa_grid_threshold->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_asa_grid_threshold );
   connect(cnt_asa_grid_threshold, SIGNAL(valueChanged(double)), SLOT(update_asa_grid_threshold(double)));

   lbl_asa_grid_threshold_percent = new QLabel(us_tr(" Grid Bead ASA Threshold %: "), this);
   Q_CHECK_PTR(lbl_asa_grid_threshold_percent);
   lbl_asa_grid_threshold_percent->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asa_grid_threshold_percent->setMinimumHeight(minHeight1);
   lbl_asa_grid_threshold_percent->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asa_grid_threshold_percent );
   lbl_asa_grid_threshold_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_asa_grid_threshold_percent= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_asa_grid_threshold_percent );
   Q_CHECK_PTR(cnt_asa_grid_threshold_percent);
   cnt_asa_grid_threshold_percent->setRange(0, 100); cnt_asa_grid_threshold_percent->setSingleStep( 0.1);
   cnt_asa_grid_threshold_percent->setValue((*asa).grid_threshold_percent);
   cnt_asa_grid_threshold_percent->setMinimumHeight(minHeight1);
   cnt_asa_grid_threshold_percent->setEnabled(true);
   cnt_asa_grid_threshold_percent->setNumButtons(3);
   cnt_asa_grid_threshold_percent->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_asa_grid_threshold_percent->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_asa_grid_threshold_percent );
   connect(cnt_asa_grid_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_asa_grid_threshold_percent(double)));

   lbl_vdw_grpy_probe_radius = new QLabel(us_tr(" vdW+GRPY ASA Probe Radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   lbl_vdw_grpy_probe_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vdw_grpy_probe_radius->setMinimumHeight(minHeight1);
   lbl_vdw_grpy_probe_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vdw_grpy_probe_radius );
   lbl_vdw_grpy_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_vdw_grpy_probe_radius = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_vdw_grpy_probe_radius );
   cnt_vdw_grpy_probe_radius->setRange(0, 10); cnt_vdw_grpy_probe_radius->setSingleStep( 0.01);
   cnt_vdw_grpy_probe_radius->setValue((*asa).vdw_grpy_probe_radius);
   cnt_vdw_grpy_probe_radius->setMinimumHeight(minHeight1);
   cnt_vdw_grpy_probe_radius->setEnabled(true);
   cnt_vdw_grpy_probe_radius->setNumButtons(3);
   cnt_vdw_grpy_probe_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_vdw_grpy_probe_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_vdw_grpy_probe_radius );
   connect(cnt_vdw_grpy_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_vdw_grpy_probe_radius(double)));

   lbl_vdw_grpy_threshold_percent = new QLabel(us_tr(" vdW+GRPY ASA Threshold %: "), this);
   lbl_vdw_grpy_threshold_percent->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vdw_grpy_threshold_percent->setMinimumHeight(minHeight1);
   lbl_vdw_grpy_threshold_percent->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vdw_grpy_threshold_percent );
   lbl_vdw_grpy_threshold_percent->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_vdw_grpy_threshold_percent= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_vdw_grpy_threshold_percent );
   cnt_vdw_grpy_threshold_percent->setRange(0, 100); cnt_vdw_grpy_threshold_percent->setSingleStep( 0.1);
   cnt_vdw_grpy_threshold_percent->setValue((*asa).vdw_grpy_threshold_percent);
   cnt_vdw_grpy_threshold_percent->setMinimumHeight(minHeight1);
   cnt_vdw_grpy_threshold_percent->setEnabled(true);
   cnt_vdw_grpy_threshold_percent->setNumButtons(3);
   cnt_vdw_grpy_threshold_percent->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_vdw_grpy_threshold_percent->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_vdw_grpy_threshold_percent );
   connect(cnt_vdw_grpy_threshold_percent, SIGNAL(valueChanged(double)), SLOT(update_vdw_grpy_threshold_percent(double)));

   lbl_hydrate_probe_radius = new QLabel(us_tr(" vdW Hydrate ASA Probe Radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   lbl_hydrate_probe_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hydrate_probe_radius->setMinimumHeight(minHeight1);
   lbl_hydrate_probe_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_hydrate_probe_radius );
   lbl_hydrate_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hydrate_probe_radius = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hydrate_probe_radius );
   cnt_hydrate_probe_radius->setRange(0, 10); cnt_hydrate_probe_radius->setSingleStep( 0.01);
   cnt_hydrate_probe_radius->setValue((*asa).hydrate_probe_radius);
   cnt_hydrate_probe_radius->setMinimumHeight(minHeight1);
   cnt_hydrate_probe_radius->setEnabled(true);
   cnt_hydrate_probe_radius->setNumButtons(3);
   cnt_hydrate_probe_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_hydrate_probe_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_hydrate_probe_radius );
   connect(cnt_hydrate_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_hydrate_probe_radius(double)));

   lbl_hydrate_threshold = new QLabel(us_tr(" vdW Hydrate ASA Threshold [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_2 + "]: "), this);
   lbl_hydrate_threshold->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hydrate_threshold->setMinimumHeight(minHeight1);
   lbl_hydrate_threshold->setPalette( PALET_LABEL );
   AUTFBACK( lbl_hydrate_threshold );
   lbl_hydrate_threshold->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_hydrate_threshold= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hydrate_threshold );
   cnt_hydrate_threshold->setRange(0, 100); cnt_hydrate_threshold->setSingleStep( 0.1);
   cnt_hydrate_threshold->setValue((*asa).hydrate_threshold);
   cnt_hydrate_threshold->setMinimumHeight(minHeight1);
   cnt_hydrate_threshold->setEnabled(true);
   cnt_hydrate_threshold->setNumButtons(3);
   cnt_hydrate_threshold->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_hydrate_threshold->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_hydrate_threshold );
   connect(cnt_hydrate_threshold, SIGNAL(valueChanged(double)), SLOT(update_hydrate_threshold(double)));

   lbl_asab1_step = new QLabel(us_tr(" ASAB1 Step Size [" + UNICODE_ANGSTROM_QS + "]: "), this);
   Q_CHECK_PTR(lbl_asab1_step);
   lbl_asab1_step->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_asab1_step->setMinimumHeight(minHeight1);
   lbl_asab1_step->setPalette( PALET_LABEL );
   AUTFBACK( lbl_asab1_step );
   lbl_asab1_step->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_asab1_step= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_asab1_step );
   Q_CHECK_PTR(cnt_asab1_step);
   cnt_asab1_step->setRange(0.1, 100); cnt_asab1_step->setSingleStep( 0.1);
   cnt_asab1_step->setValue((*asa).asab1_step);
   cnt_asab1_step->setMinimumHeight(minHeight1);
   cnt_asab1_step->setEnabled(true);
   cnt_asab1_step->setNumButtons(3);
   cnt_asab1_step->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_asab1_step->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_asab1_step );
   connect(cnt_asab1_step, SIGNAL(valueChanged(double)), SLOT(update_asab1_step(double)));

   cb_vvv = new QCheckBox(this);
   cb_vvv->setText(us_tr(" Compute VVV volume, surface area on load PDB"));
   cb_vvv->setChecked((*asa).vvv);
   cb_vvv->setEnabled(true);
   cb_vvv->setMinimumHeight(minHeight1);
   cb_vvv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_vvv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_vvv );
   connect(cb_vvv, SIGNAL(clicked()), SLOT(set_vvv()));

   lbl_vvv_probe_radius = new QLabel(us_tr(" VVV probe radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   lbl_vvv_probe_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vvv_probe_radius->setMinimumHeight(minHeight1);
   lbl_vvv_probe_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vvv_probe_radius );
   lbl_vvv_probe_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_vvv_probe_radius= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_vvv_probe_radius );
   cnt_vvv_probe_radius->setRange(0.0, 20.0); cnt_vvv_probe_radius->setSingleStep( 0.1);
   cnt_vvv_probe_radius->setValue((*asa).vvv_probe_radius);
   cnt_vvv_probe_radius->setMinimumHeight(minHeight1);
   cnt_vvv_probe_radius->setEnabled(true);
   cnt_vvv_probe_radius->setNumButtons(3);
   cnt_vvv_probe_radius->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_vvv_probe_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_vvv_probe_radius );
   connect(cnt_vvv_probe_radius, SIGNAL(valueChanged(double)), SLOT(update_vvv_probe_radius(double)));

   lbl_vvv_grid_dR = new QLabel(us_tr(" VVV grid edge size [" + UNICODE_ANGSTROM_QS + "]: "), this);
   lbl_vvv_grid_dR->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vvv_grid_dR->setMinimumHeight(minHeight1);
   lbl_vvv_grid_dR->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vvv_grid_dR );
   lbl_vvv_grid_dR->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_vvv_grid_dR= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_vvv_grid_dR );
   cnt_vvv_grid_dR->setRange(0.1, 20.0); cnt_vvv_grid_dR->setSingleStep( 0.01);
   cnt_vvv_grid_dR->setValue((*asa).vvv_grid_dR);
   cnt_vvv_grid_dR->setMinimumHeight(minHeight1);
   cnt_vvv_grid_dR->setEnabled(true);
   cnt_vvv_grid_dR->setNumButtons(3);
   cnt_vvv_grid_dR->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_vvv_grid_dR->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_vvv_grid_dR );
   connect(cnt_vvv_grid_dR, SIGNAL(valueChanged(double)), SLOT(update_vvv_grid_dR(double)));

   cb_asa_calculation = new QCheckBox(this);
   cb_asa_calculation->setText(us_tr(" Perform ASA Calculation "));
   cb_asa_calculation->setChecked((*asa).calculation);
   cb_asa_calculation->setEnabled(true);
   cb_asa_calculation->setMinimumHeight(minHeight1);
   cb_asa_calculation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_asa_calculation->setPalette( PALET_NORMAL );
   AUTFBACK( cb_asa_calculation );
   connect(cb_asa_calculation, SIGNAL(clicked()), SLOT(set_asa_calculation()));

   cb_bead_check = new QCheckBox(this);
   cb_bead_check->setText(us_tr(" Re-check bead ASA "));
   cb_bead_check->setChecked((*asa).recheck_beads);
   cb_bead_check->setEnabled(true);
   cb_bead_check->setMinimumHeight(minHeight1);
   cb_bead_check->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_check->setPalette( PALET_NORMAL );
   AUTFBACK( cb_bead_check );
   connect(cb_bead_check, SIGNAL(clicked()), SLOT(set_bead_check()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int /* rows=8, columns = 2, */ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(cb_asa_calculation, j, 0);
   background->addWidget(cb_bead_check, j, 1);
   j++;
   background->addWidget( bg_asa_method , j , 0 , 1 + ( j+3 ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j+=4;
   background->addWidget(lbl_probe_radius, j, 0);
   background->addWidget(cnt_probe_radius, j, 1);
   j++;
   background->addWidget(lbl_probe_recheck_radius, j, 0);
   background->addWidget(cnt_probe_recheck_radius, j, 1);
   j++;
   background->addWidget(lbl_asa_threshold, j, 0);
   background->addWidget(cnt_asa_threshold, j, 1);
   j++;
   background->addWidget(lbl_asa_threshold_percent, j, 0);
   background->addWidget(cnt_asa_threshold_percent, j, 1);
   j++;
   background->addWidget(lbl_asa_grid_threshold, j, 0);
   background->addWidget(cnt_asa_grid_threshold, j, 1);
   j++;
   background->addWidget(lbl_asa_grid_threshold_percent, j, 0);
   background->addWidget(cnt_asa_grid_threshold_percent, j, 1);
   j++;
   background->addWidget(lbl_vdw_grpy_probe_radius, j, 0);
   background->addWidget(cnt_vdw_grpy_probe_radius, j, 1);
   j++;
   background->addWidget(lbl_vdw_grpy_threshold_percent, j, 0);
   background->addWidget(cnt_vdw_grpy_threshold_percent, j, 1);
   j++;
   background->addWidget(lbl_hydrate_probe_radius, j, 0);
   background->addWidget(cnt_hydrate_probe_radius, j, 1);
   j++;
   background->addWidget(lbl_hydrate_threshold, j, 0);
   background->addWidget(cnt_hydrate_threshold, j, 1);
   j++;
   background->addWidget(lbl_asab1_step, j, 0);
   background->addWidget(cnt_asab1_step, j, 1);
   j++;
   background->addWidget( cb_vvv , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(lbl_vvv_probe_radius, j, 0);
   background->addWidget(cnt_vvv_probe_radius, j, 1);
   j++;
   background->addWidget(lbl_vvv_grid_dR, j, 0);
   background->addWidget(cnt_vvv_grid_dR, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_ASA::cancel()
{
   close();
}

void US_Hydrodyn_ASA::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_asa.html");
}

void US_Hydrodyn_ASA::closeEvent(QCloseEvent *e)
{
   *asa_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_ASA::update_probe_radius(double val)
{
   (*asa).probe_radius = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_probe_recheck_radius(double val)
{
   (*asa).probe_recheck_radius = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_asa_threshold(double val)
{
   (*asa).threshold = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_asa_threshold_percent(double val)
{
   (*asa).threshold_percent = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_asa_grid_threshold(double val)
{
   (*asa).grid_threshold = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_asa_grid_threshold_percent(double val)
{
   (*asa).grid_threshold_percent = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_hydrate_probe_radius(double val)
{
   (*asa).hydrate_probe_radius = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_hydrate_threshold(double val)
{
   (*asa).hydrate_threshold = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_vdw_grpy_probe_radius(double val)
{
   (*asa).vdw_grpy_probe_radius = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_vdw_grpy_threshold_percent(double val)
{
   (*asa).vdw_grpy_threshold_percent = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_asab1_step(double val)
{
   (*asa).asab1_step = (float) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::set_asa_calculation()
{
   (*asa).calculation = cb_asa_calculation->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::set_bead_check()
{
   (*asa).recheck_beads = cb_bead_check->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::select_asa_method()
{
   if ( rb_surfracer->isChecked() ) {
      return select_asa_method( 0 );
   }
   if ( rb_asab1->isChecked() ) {
      return select_asa_method( 1 );
   }
}

void US_Hydrodyn_ASA::select_asa_method(int val)
{
   (*asa).method = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::set_vvv()
{
   (*asa).vvv = cb_vvv->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_vvv_probe_radius(double val)
{
   (*asa).vvv_probe_radius = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_ASA::update_vvv_grid_dR(double val)
{
   (*asa).vvv_grid_dR = (float) val;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
