#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_misc.h"
#include "../include/us_hydrodyn.h"
//Added by qt3to4:
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#include "../include/us_unicode.h"

US_Hydrodyn_Misc::US_Hydrodyn_Misc(struct misc_options *misc,
                                   bool *misc_widget, void *us_hydrodyn, QWidget *p, const char *) : QFrame( p )
{
   this->misc = misc;
   this->misc_widget = misc_widget;
   *misc_widget = true;
   this->us_hydrodyn = us_hydrodyn;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("Misceallaneous SOMO Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Misc::~US_Hydrodyn_Misc()
{
   *misc_widget = false;
}

void US_Hydrodyn_Misc::setupGUI()
{
   int minHeight1 = 22;
   auto hdrFontSize = USglobal->config_list.fontSize + 2;
   auto useFontSize = USglobal->config_list.fontSize + 1;
   
   QString str;
   lbl_info = new QLabel(us_tr("Miscellaneous SOMO Options:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   lbl_hydrovol = new QLabel(us_tr(" Hydration Water Vol. [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_3_QS + "]: "), this);
   Q_CHECK_PTR(lbl_hydrovol);
   lbl_hydrovol->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_hydrovol->setMinimumWidth(220);
   lbl_hydrovol->setMinimumHeight(minHeight1);
   lbl_hydrovol->setPalette( PALET_LABEL );
   AUTFBACK( lbl_hydrovol );
   lbl_hydrovol->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_hydrovol= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_hydrovol );
   Q_CHECK_PTR(cnt_hydrovol);
   cnt_hydrovol->setRange(0, 100); cnt_hydrovol->setSingleStep( 0.001);
   cnt_hydrovol->setValue((*misc).hydrovol);
   cnt_hydrovol->setMinimumHeight(minHeight1);
   cnt_hydrovol->setMinimumWidth(220);
   cnt_hydrovol->setEnabled(true);
   cnt_hydrovol->setNumButtons(3);
   cnt_hydrovol->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_hydrovol->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_hydrovol );
   connect(cnt_hydrovol, SIGNAL(valueChanged(double)), SLOT(update_hydrovol(double)));

   cb_vbar = new QCheckBox(this);
   cb_vbar->setText(us_tr(" Calculate vbar "));
   cb_vbar->setChecked((*misc).compute_vbar);
   cb_vbar->setMinimumHeight(minHeight1);
   cb_vbar->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_vbar->setPalette( PALET_NORMAL );
   AUTFBACK( cb_vbar );
   connect(cb_vbar, SIGNAL(clicked()), SLOT(set_vbar()));

   pb_vbar = new QPushButton(us_tr("Select vbar"), this);
   Q_CHECK_PTR(pb_vbar);
   pb_vbar->setEnabled(!(*misc).compute_vbar);
   pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize));
   pb_vbar->setMinimumHeight(minHeight1);
   pb_vbar->setPalette( PALET_PUSHB );
   connect(pb_vbar, SIGNAL(clicked()), SLOT(select_vbar()));


   lbl_vbar = new QLabel(us_tr(" Enter a vbar value [cm" + UNICODE_SUPER_3_QS + "/g]: "), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar->setMinimumHeight(minHeight1);
   lbl_vbar->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vbar );
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_vbar = new QLineEdit( this );    le_vbar->setObjectName( "vbar Line Edit" );
   le_vbar->setMinimumHeight(minHeight1);
   le_vbar->setEnabled(!(*misc).compute_vbar);
   le_vbar->setText(str.sprintf("%5.3f", (*misc).vbar));
   le_vbar->setPalette( PALET_NORMAL );
   AUTFBACK( le_vbar );
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

   lbl_vbar_temperature = new QLabel(us_tr( QString( " Vbar measured/computed at T=[%1C]: " ).arg( DEGREE_SYMBOL ) ), this);
   Q_CHECK_PTR(lbl_vbar_temperature);
   lbl_vbar_temperature->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar_temperature->setMinimumHeight(minHeight1);
   lbl_vbar_temperature->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vbar_temperature );
   lbl_vbar_temperature->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_vbar_temperature = new QLineEdit( this );    le_vbar_temperature->setObjectName( "vbar_temperature Line Edit" );
   le_vbar_temperature->setMinimumHeight(minHeight1);
   le_vbar_temperature->setEnabled(!(*misc).compute_vbar);
   le_vbar_temperature->setText(str.sprintf("%5.2f", (*misc).vbar_temperature));
   le_vbar_temperature->setPalette( PALET_NORMAL );
   AUTFBACK( le_vbar_temperature );
   le_vbar_temperature->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_vbar_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_vbar_temperature(const QString &)));

   cb_pb_rule_on = new QCheckBox(this);
   cb_pb_rule_on->setText(us_tr(" Enable Peptide Bond Rule "));
   cb_pb_rule_on->setChecked((*misc).pb_rule_on);
   cb_pb_rule_on->setMinimumHeight(minHeight1);
   cb_pb_rule_on->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_pb_rule_on->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pb_rule_on );
   connect(cb_pb_rule_on, SIGNAL(clicked()), SLOT(set_pb_rule_on()));

   lbl_avg_banner = new QLabel(us_tr("Average Parameters for Automatic Bead Builder:"), this);
   Q_CHECK_PTR(lbl_avg_banner);
   lbl_avg_banner->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_avg_banner->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_avg_banner->setMinimumHeight(minHeight1);
   lbl_avg_banner->setPalette( PALET_FRAME );
   AUTFBACK( lbl_avg_banner );
   lbl_avg_banner->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   lbl_avg_radius = new QLabel(us_tr(" Average atomic radius [" + UNICODE_ANGSTROM_QS + "]: "), this);
   Q_CHECK_PTR(lbl_avg_radius);
   lbl_avg_radius->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_radius->setMinimumWidth(220);
   lbl_avg_radius->setMinimumHeight(minHeight1);
   lbl_avg_radius->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_radius );
   lbl_avg_radius->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_radius = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_radius );
   Q_CHECK_PTR(cnt_avg_radius);
   cnt_avg_radius->setRange(0.5, 5.0); cnt_avg_radius->setSingleStep( 0.001);
   cnt_avg_radius->setValue((*misc).avg_radius);
   cnt_avg_radius->setMinimumHeight(minHeight1);
   cnt_avg_radius->setMinimumWidth(220);
   cnt_avg_radius->setEnabled(true);
   cnt_avg_radius->setNumButtons(3);
   cnt_avg_radius->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_radius->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_radius );
   connect(cnt_avg_radius, SIGNAL(valueChanged(double)), SLOT(update_avg_radius(double)));

   lbl_avg_mass = new QLabel(us_tr(" Average atomic mass [Da]: "), this);
   Q_CHECK_PTR(lbl_avg_mass);
   lbl_avg_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_mass->setMinimumWidth(220);
   lbl_avg_mass->setMinimumHeight(minHeight1);
   lbl_avg_mass->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_mass );
   lbl_avg_mass->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_mass = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_mass );
   Q_CHECK_PTR(cnt_avg_mass);
   cnt_avg_mass->setRange(0.0, 200.0); cnt_avg_mass->setSingleStep( 0.01);
   cnt_avg_mass->setValue((*misc).avg_mass);
   cnt_avg_mass->setMinimumHeight(minHeight1);
   cnt_avg_mass->setMinimumWidth(220);
   cnt_avg_mass->setEnabled(true);
   cnt_avg_mass->setNumButtons(3);
   cnt_avg_mass->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_mass->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_mass );
   connect(cnt_avg_mass, SIGNAL(valueChanged(double)), SLOT(update_avg_mass(double)));

   lbl_avg_num_elect = new QLabel(us_tr(" Average atomic number of electrons: "), this);
   Q_CHECK_PTR(lbl_avg_num_elect);
   lbl_avg_num_elect->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_num_elect->setMinimumWidth(220);
   lbl_avg_num_elect->setMinimumHeight(minHeight1);
   lbl_avg_num_elect->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_num_elect );
   lbl_avg_num_elect->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_num_elect = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_num_elect );
   Q_CHECK_PTR(cnt_avg_num_elect);
   cnt_avg_num_elect->setRange(1.0, 25.0); cnt_avg_num_elect->setSingleStep( 0.001);
   cnt_avg_num_elect->setValue((*misc).avg_num_elect);
   cnt_avg_num_elect->setMinimumHeight(minHeight1);
   cnt_avg_num_elect->setMinimumWidth(220);
   cnt_avg_num_elect->setEnabled(true);
   cnt_avg_num_elect->setNumButtons(3);
   cnt_avg_num_elect->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_num_elect->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_num_elect );
   connect(cnt_avg_num_elect, SIGNAL(valueChanged(double)), SLOT(update_avg_num_elect(double)));

   lbl_avg_protons = new QLabel(us_tr(" Average atomic number of protons: "), this);
   Q_CHECK_PTR(lbl_avg_protons);
   lbl_avg_protons->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_protons->setMinimumWidth(220);
   lbl_avg_protons->setMinimumHeight(minHeight1);
   lbl_avg_protons->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_protons );
   lbl_avg_protons->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_protons = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_protons );
   Q_CHECK_PTR(cnt_avg_protons);
   cnt_avg_protons->setRange(1.0, 25.0); cnt_avg_protons->setSingleStep( .001 );
   cnt_avg_protons->setValue((*misc).avg_protons);
   cnt_avg_protons->setMinimumHeight(minHeight1);
   cnt_avg_protons->setMinimumWidth(220);
   cnt_avg_protons->setEnabled(true);
   cnt_avg_protons->setNumButtons(3);
   cnt_avg_protons->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_protons->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_protons );
   connect(cnt_avg_protons, SIGNAL(valueChanged(double)), SLOT(update_avg_protons(double)));

   lbl_avg_hydration = new QLabel(us_tr(" Average atomic hydration: "), this);
   Q_CHECK_PTR(lbl_avg_hydration);
   lbl_avg_hydration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_hydration->setMinimumWidth(220);
   lbl_avg_hydration->setMinimumHeight(minHeight1);
   lbl_avg_hydration->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_hydration );
   lbl_avg_hydration->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_hydration = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_hydration );
   Q_CHECK_PTR(cnt_avg_hydration);
   cnt_avg_hydration->setRange(0.0, 10.0); cnt_avg_hydration->setSingleStep( 0.001);
   cnt_avg_hydration->setValue((*misc).avg_hydration);
   cnt_avg_hydration->setMinimumHeight(minHeight1);
   cnt_avg_hydration->setMinimumWidth(220);
   cnt_avg_hydration->setEnabled(true);
   cnt_avg_hydration->setNumButtons(3);
   cnt_avg_hydration->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_hydration->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_hydration );
   connect(cnt_avg_hydration, SIGNAL(valueChanged(double)), SLOT(update_avg_hydration(double)));

   lbl_avg_volume = new QLabel(us_tr(" Average bead/atom volume [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_3_QS + "]: "), this);
   Q_CHECK_PTR(lbl_avg_volume);
   lbl_avg_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_volume->setMinimumWidth(220);
   lbl_avg_volume->setMinimumHeight(minHeight1);
   lbl_avg_volume->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_volume );
   lbl_avg_volume->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_volume = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_volume );
   Q_CHECK_PTR(cnt_avg_volume);
   cnt_avg_volume->setRange(1.0, 50.0); cnt_avg_volume->setSingleStep( 0.01);
   cnt_avg_volume->setValue((*misc).avg_volume);
   cnt_avg_volume->setMinimumHeight(minHeight1);
   cnt_avg_volume->setMinimumWidth(220);
   cnt_avg_volume->setEnabled(true);
   cnt_avg_volume->setNumButtons(3);
   cnt_avg_volume->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_volume->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_volume );
   connect(cnt_avg_volume, SIGNAL(valueChanged(double)), SLOT(update_avg_volume(double)));

   lbl_avg_vbar = new QLabel(us_tr(" Average Residue vbar [cm" + UNICODE_SUPER_3_QS + "/g]: "), this);
   Q_CHECK_PTR(lbl_avg_vbar);
   lbl_avg_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_avg_vbar->setMinimumWidth(220);
   lbl_avg_vbar->setMinimumHeight(minHeight1);
   lbl_avg_vbar->setPalette( PALET_LABEL );
   AUTFBACK( lbl_avg_vbar );
   lbl_avg_vbar->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cnt_avg_vbar = new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_avg_vbar );
   Q_CHECK_PTR(cnt_avg_vbar);
   cnt_avg_vbar->setRange(0.01, 2.0); cnt_avg_vbar->setSingleStep( 0.001);
   cnt_avg_vbar->setValue((*misc).avg_vbar);
   cnt_avg_vbar->setMinimumHeight(minHeight1);
   cnt_avg_vbar->setMinimumWidth(220);
   cnt_avg_vbar->setEnabled(true);
   cnt_avg_vbar->setNumButtons(3);
   cnt_avg_vbar->setFont(QFont(USglobal->config_list.fontFamily, useFontSize));
   cnt_avg_vbar->setPalette( PALET_NORMAL );
   AUTFBACK( cnt_avg_vbar );
   connect(cnt_avg_vbar, SIGNAL(valueChanged(double)), SLOT(update_avg_vbar(double)));

   lbl_bead_model_controls = new QLabel(us_tr("Bead model controls:"), this);
   lbl_bead_model_controls->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_bead_model_controls->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_bead_model_controls->setMinimumHeight(minHeight1);
   lbl_bead_model_controls->setPalette( PALET_FRAME );
   AUTFBACK( lbl_bead_model_controls );
   lbl_bead_model_controls->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   lbl_target_e_density = new QLabel(us_tr(" Target electron density [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_MINUS_QS + UNICODE_SUPER_3_QS + "]: "), this);
   lbl_target_e_density->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_target_e_density->setMinimumWidth(220);
   lbl_target_e_density->setMinimumHeight(minHeight1);
   lbl_target_e_density->setPalette( PALET_LABEL );
   AUTFBACK( lbl_target_e_density );
   lbl_target_e_density->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_target_e_density = new QLineEdit( this );    le_target_e_density->setObjectName( "target_e_density Line Edit" );
   le_target_e_density->setMinimumHeight(minHeight1);
   le_target_e_density->setEnabled(true);
   le_target_e_density->setText(QString("%1").arg((*misc).target_e_density));
   le_target_e_density->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_e_density );
   le_target_e_density->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_target_e_density, SIGNAL(textChanged(const QString &)), SLOT(update_target_e_density(const QString &)));

   lbl_target_volume = new QLabel(us_tr(" Target volume [" + UNICODE_ANGSTROM_QS + UNICODE_SUPER_3_QS + "]: "), this);
   lbl_target_volume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_target_volume->setMinimumWidth(220);
   lbl_target_volume->setMinimumHeight(minHeight1);
   lbl_target_volume->setPalette( PALET_LABEL );
   AUTFBACK( lbl_target_volume );
   lbl_target_volume->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_target_volume = new QLineEdit( this );    le_target_volume->setObjectName( "target_volume Line Edit" );
   le_target_volume->setMinimumHeight(minHeight1);
   le_target_volume->setEnabled(true);
   le_target_volume->setText(QString("%1").arg((*misc).target_volume));
   le_target_volume->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_volume );
   le_target_volume->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_target_volume, SIGNAL(textChanged(const QString &)), SLOT(update_target_volume(const QString &)));

   cb_set_target_on_load_pdb = new QCheckBox(this);
   cb_set_target_on_load_pdb->setText(us_tr(" Set targets on load PDB "));
   cb_set_target_on_load_pdb->setChecked((*misc).set_target_on_load_pdb);
   cb_set_target_on_load_pdb->setMinimumHeight(minHeight1);
   cb_set_target_on_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_set_target_on_load_pdb->setPalette( PALET_NORMAL );
   AUTFBACK( cb_set_target_on_load_pdb );
   connect(cb_set_target_on_load_pdb, SIGNAL(clicked()), SLOT(set_set_target_on_load_pdb()));

   cb_equalize_radii = new QCheckBox(this);
   cb_equalize_radii->setText(us_tr(" Equalize radii (constant volume)"));
   cb_equalize_radii->setChecked((*misc).equalize_radii);
   cb_equalize_radii->setMinimumHeight(minHeight1);
   cb_equalize_radii->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_equalize_radii->setPalette( PALET_NORMAL );
   AUTFBACK( cb_equalize_radii );
   connect(cb_equalize_radii, SIGNAL(clicked()), SLOT(set_equalize_radii()));

   // lbl_hydro_method = new QLabel(us_tr("Hydrodynamic Method:"), this);
   // lbl_hydro_method->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_hydro_method->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_hydro_method->setMinimumHeight(minHeight1);
   // lbl_hydro_method->setPalette( PALET_FRAME );
   // lbl_hydro_method->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   // cb_hydro_supc = new QCheckBox(this);
   // cb_hydro_supc->setText(us_tr(" Standard matrix inversion"));
   // cb_hydro_supc->setChecked((*misc).hydro_supc);
   // cb_hydro_supc->setMinimumHeight(minHeight1);
   // cb_hydro_supc->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   // cb_hydro_supc->setPalette( PALET_NORMAL );
   // connect(cb_hydro_supc, SIGNAL(clicked()), SLOT(set_hydro_supc()));

   // cb_hydro_zeno = new QCheckBox(this);
   // cb_hydro_zeno->setText(us_tr(" Zeno method"));
   // cb_hydro_zeno->setChecked((*misc).hydro_zeno);
   // cb_hydro_zeno->setMinimumHeight(minHeight1);
   // cb_hydro_zeno->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   // cb_hydro_zeno->setPalette( PALET_NORMAL );
   // connect(cb_hydro_zeno, SIGNAL(clicked()), SLOT(set_hydro_zeno()));

   lbl_threshold = new QLabel(us_tr("Additonal computed vbar controls:"), this);
   lbl_threshold->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_threshold->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_threshold->setMinimumHeight(minHeight1);
   lbl_threshold->setPalette( PALET_FRAME );
   AUTFBACK( lbl_threshold );
   lbl_threshold->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));
   lbl_threshold->hide();

   lbl_covolume = new QLabel(us_tr(" Covolume [cm" + UNICODE_SUPER_3_QS + "/mol]: "), this);
   lbl_covolume->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_covolume->setMinimumWidth(220);
   lbl_covolume->setMinimumHeight(minHeight1);
   lbl_covolume->setPalette( PALET_LABEL );
   AUTFBACK( lbl_covolume );
   lbl_covolume->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_covolume = new QLineEdit( this );    le_covolume->setObjectName( "covolume Line Edit" );
   le_covolume->setMinimumHeight(minHeight1);
   le_covolume->setEnabled(true);
   le_covolume->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "covolume" ) ?
                                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "covolume" ] : "12.4" ) );
   le_covolume->setPalette( PALET_NORMAL );
   AUTFBACK( le_covolume );
   le_covolume->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_covolume, SIGNAL(textChanged(const QString &)), SLOT(update_covolume(const QString &)));

   // lbl_thresh_SS = new QLabel(us_tr(" Disulfide distance threshold [" + UNICODE_ANGSTROM_QS + "]: "), this);
   // lbl_thresh_SS->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   // lbl_thresh_SS->setMinimumWidth(220);
   // lbl_thresh_SS->setMinimumHeight(minHeight1);
   // lbl_thresh_SS->setPalette( PALET_LABEL );
   // AUTFBACK( lbl_thresh_SS );
   // lbl_thresh_SS->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   // le_thresh_SS = new QLineEdit( this );    le_thresh_SS->setObjectName( "thresh_SS Line Edit" );
   // le_thresh_SS->setMinimumHeight(minHeight1);
   // le_thresh_SS->setEnabled(true);
   // le_thresh_SS->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "thresh_SS" ) ?
   //                                            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_SS" ] : "2.3" ) );
   // le_thresh_SS->setPalette( PALET_NORMAL );
   // AUTFBACK( le_thresh_SS );
   // le_thresh_SS->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   // connect(le_thresh_SS, SIGNAL(textChanged(const QString &)), SLOT(update_thresh_SS(const QString &)));

   lbl_thresh_carb_O = new QLabel(us_tr(" Carbohydrate O distance threshold [" + UNICODE_ANGSTROM_QS + "]:"), this);
   lbl_thresh_carb_O->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_thresh_carb_O->setMinimumWidth(220);
   lbl_thresh_carb_O->setMinimumHeight(minHeight1);
   lbl_thresh_carb_O->setPalette( PALET_LABEL );
   AUTFBACK( lbl_thresh_carb_O );
   lbl_thresh_carb_O->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));
   lbl_thresh_carb_O->hide();

   le_thresh_carb_O = new QLineEdit( this );    le_thresh_carb_O->setObjectName( "thresh_carb_O Line Edit" );
   le_thresh_carb_O->setMinimumHeight(minHeight1);
   le_thresh_carb_O->setEnabled(true);
   le_thresh_carb_O->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "thresh_carb_O" ) ?
                                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_carb_O" ] : "2" ) );
   le_thresh_carb_O->setPalette( PALET_NORMAL );
   AUTFBACK( le_thresh_carb_O );
   le_thresh_carb_O->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_thresh_carb_O, SIGNAL(textChanged(const QString &)), SLOT(update_thresh_carb_O(const QString &)));
   le_thresh_carb_O->hide();

   lbl_thresh_carb_N = new QLabel(us_tr(" Carbohydrate N distance threshold [" + UNICODE_ANGSTROM_QS + "]:"), this);
   lbl_thresh_carb_N->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_thresh_carb_N->setMinimumWidth(220);
   lbl_thresh_carb_N->setMinimumHeight(minHeight1);
   lbl_thresh_carb_N->setPalette( PALET_LABEL );
   AUTFBACK( lbl_thresh_carb_N );
   lbl_thresh_carb_N->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));
   lbl_thresh_carb_N->hide();

   le_thresh_carb_N = new QLineEdit( this );    le_thresh_carb_N->setObjectName( "thresh_carb_N Line Edit" );
   le_thresh_carb_N->setMinimumHeight(minHeight1);
   le_thresh_carb_N->setEnabled(true);
   le_thresh_carb_N->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "thresh_carb_N" ) ?
                                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_carb_N" ] : "2" ) );
   le_thresh_carb_N->setPalette( PALET_NORMAL );
   AUTFBACK( le_thresh_carb_N );
   le_thresh_carb_N->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_thresh_carb_N, SIGNAL(textChanged(const QString &)), SLOT(update_thresh_carb_N(const QString &)));
   le_thresh_carb_N->hide();

   lbl_other = new QLabel(us_tr("Other options:"), this);
   lbl_other->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_other->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_other->setMinimumHeight(minHeight1);
   lbl_other->setPalette( PALET_FRAME );
   AUTFBACK( lbl_other );
   lbl_other->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   cb_export_msroll = new QCheckBox(this);
   cb_export_msroll->setText(us_tr(" Create MSROLL atomic radii and name files on load residue file"));
   cb_export_msroll->setChecked((*misc).export_msroll);
   cb_export_msroll->setMinimumHeight(minHeight1);
   cb_export_msroll->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_export_msroll->setPalette( PALET_NORMAL );
   AUTFBACK( cb_export_msroll );
   connect(cb_export_msroll, SIGNAL(clicked()), SLOT(set_export_msroll()));

   cb_export_ssbond = new QCheckBox(this);
   cb_export_ssbond->setText(us_tr(" Create SSBOND txt file when loading PDB"));
   cb_export_ssbond->setChecked((*misc).export_ssbond);
   cb_export_ssbond->setMinimumHeight(minHeight1);
   cb_export_ssbond->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_export_ssbond->setPalette( PALET_NORMAL );
   AUTFBACK( cb_export_ssbond );
   connect(cb_export_ssbond, SIGNAL(clicked()), SLOT(set_export_ssbond()));

   cb_parallel_grpy = new QCheckBox(this);
   cb_parallel_grpy->setText(us_tr(" Enable Parallel GRPY"));
   cb_parallel_grpy->setChecked((*misc).parallel_grpy);
   cb_parallel_grpy->setMinimumHeight(minHeight1);
   cb_parallel_grpy->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_parallel_grpy->setPalette( PALET_NORMAL );
   AUTFBACK( cb_parallel_grpy );
   connect(cb_parallel_grpy, SIGNAL(clicked()), SLOT(set_parallel_grpy()));

   lbl_vdw_ot_mult = new QLabel(us_tr(" vdW OT multiplier: "), this);
   lbl_vdw_ot_mult->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vdw_ot_mult->setMinimumWidth(220);
   lbl_vdw_ot_mult->setMinimumHeight(minHeight1);
   lbl_vdw_ot_mult->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vdw_ot_mult );
   lbl_vdw_ot_mult->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_vdw_ot_mult = new QLineEdit( this );    le_vdw_ot_mult->setObjectName( "vdw_ot_mult Line Edit" );
   le_vdw_ot_mult->setMinimumHeight(minHeight1);
   le_vdw_ot_mult->setEnabled(true);
   le_vdw_ot_mult->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_ot_mult" ) ?
                                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_mult" ] : "0" ) );
   le_vdw_ot_mult->setPalette( PALET_NORMAL );
   AUTFBACK( le_vdw_ot_mult );
   le_vdw_ot_mult->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_vdw_ot_mult, SIGNAL(textChanged(const QString &)), SLOT(update_vdw_ot_mult(const QString &)));

   lbl_vdw_ot_dpct = new QLabel(us_tr(" vdW OT additonal water decrease %: "), this);
   lbl_vdw_ot_dpct->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vdw_ot_dpct->setMinimumWidth(220);
   lbl_vdw_ot_dpct->setMinimumHeight(minHeight1);
   lbl_vdw_ot_dpct->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vdw_ot_dpct );
   lbl_vdw_ot_dpct->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   le_vdw_ot_dpct = new QLineEdit( this );    le_vdw_ot_dpct->setObjectName( "vdw_ot_dpct Line Edit" );
   le_vdw_ot_dpct->setMinimumHeight(minHeight1);
   le_vdw_ot_dpct->setEnabled(true);
   le_vdw_ot_dpct->setText(QString("%1").arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_ot_dpct" ) ?
                                              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_dpct" ] : "0" ) );
   le_vdw_ot_dpct->setPalette( PALET_NORMAL );
   AUTFBACK( le_vdw_ot_dpct );
   le_vdw_ot_dpct->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   connect(le_vdw_ot_dpct, SIGNAL(textChanged(const QString &)), SLOT(update_vdw_ot_dpct(const QString &)));

   cb_vdw_ot_alt = new QCheckBox(this);
   cb_vdw_ot_alt->setText(us_tr(" vdW use alternate method for OT"));
   cb_vdw_ot_alt->setChecked(( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_ot_alt" ) &&
                             ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_alt" ] == "true" );
   cb_vdw_ot_alt->setMinimumHeight(minHeight1);
   cb_vdw_ot_alt->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_vdw_ot_alt->setPalette( PALET_NORMAL );
   AUTFBACK( cb_vdw_ot_alt );
   connect(cb_vdw_ot_alt, SIGNAL(clicked()), SLOT(set_vdw_ot_alt()));

   cb_vdw_saxs_water_beads = new QCheckBox(this);
   cb_vdw_saxs_water_beads->setText(us_tr(" vdW saxs pr create water beads"));
   cb_vdw_saxs_water_beads->setChecked(( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_saxs_water_beads" ) &&
                                       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_water_beads" ] == "true" );
   cb_vdw_saxs_water_beads->setMinimumHeight(minHeight1);
   cb_vdw_saxs_water_beads->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_vdw_saxs_water_beads->setPalette( PALET_NORMAL );
   AUTFBACK( cb_vdw_saxs_water_beads );
   connect(cb_vdw_saxs_water_beads, SIGNAL(clicked()), SLOT(set_vdw_saxs_water_beads()));

   cb_vdw_saxs_skip_pr0pair = new QCheckBox(this);
   cb_vdw_saxs_skip_pr0pair->setText(us_tr(" vdW saxs pr skip 0 distance pairs"));
   cb_vdw_saxs_skip_pr0pair->setChecked(( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "vdw_saxs_skip_pr0pair" ) &&
                                       ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_skip_pr0pair" ] == "true" );
   cb_vdw_saxs_skip_pr0pair->setMinimumHeight(minHeight1);
   cb_vdw_saxs_skip_pr0pair->setFont(QFont( USglobal->config_list.fontFamily, useFontSize));
   cb_vdw_saxs_skip_pr0pair->setPalette( PALET_NORMAL );
   AUTFBACK( cb_vdw_saxs_skip_pr0pair );
   connect(cb_vdw_saxs_skip_pr0pair, SIGNAL(clicked()), SLOT(set_vdw_saxs_skip_pr0pair()));
   
   if ( !((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode ) {
      cb_export_ssbond->hide();
   }

   // vdw controls move to us_hydrodyn_vdw_overlap
   lbl_vdw_ot_mult             ->hide();
   le_vdw_ot_mult              ->hide();
   lbl_vdw_ot_dpct             ->hide();
   le_vdw_ot_dpct              ->hide();
   cb_vdw_ot_alt               ->hide();
   cb_vdw_saxs_water_beads     ->hide();
   cb_vdw_saxs_skip_pr0pair    ->hide();
   lbl_vdw_ot_dpct             ->hide();
   le_vdw_ot_dpct              ->hide();
   

   // auto calc method
   lbl_auto_calc_hydro_method = new QLabel( us_tr( " Method used when automatically\n calculate hydrodynamics is checked:" ), this );
   lbl_auto_calc_hydro_method->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_auto_calc_hydro_method->setMinimumHeight(minHeight1 * 2);
   lbl_auto_calc_hydro_method->setPalette( PALET_LABEL );
   AUTFBACK( lbl_auto_calc_hydro_method );
   lbl_auto_calc_hydro_method->setFont(QFont( USglobal->config_list.fontFamily, useFontSize, QFont::Bold));

   cmb_auto_calc_hydro_method = new QComboBox();
   cmb_auto_calc_hydro_method->setPalette( PALET_NORMAL );
   AUTFBACK( cmb_auto_calc_hydro_method );
   cmb_auto_calc_hydro_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   cmb_auto_calc_hydro_method->setEnabled(true);
   cmb_auto_calc_hydro_method->setMaxVisibleItems( 1 );

   cmb_auto_calc_hydro_method->addItem( us_tr( "SMI or ZENO if overlaps present (legacy)" ), AUTO_CALC_HYDRO_SMI );
   cmb_auto_calc_hydro_method->addItem( us_tr( "ZENO" ),                                     AUTO_CALC_HYDRO_ZENO );
   cmb_auto_calc_hydro_method->addItem( us_tr( "GRPY" ),                                     AUTO_CALC_HYDRO_GRPY );

   {
      int index = cmb_auto_calc_hydro_method->findData( (int) (*misc).auto_calc_hydro_method );
      if ( index != -1 ) {
         cmb_auto_calc_hydro_method->setCurrentIndex( index );
      }
   }

   connect( cmb_auto_calc_hydro_method, SIGNAL( currentIndexChanged( QString ) ), SLOT( update_auto_calc_hydro_method() ) );

   pb_cancel = new QPushButton(us_tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int /* rows=8, columns = 2, */ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   QGridLayout * leftside = new QGridLayout(); leftside->setContentsMargins( 0, 0, 0, 0 ); leftside->setSpacing( 0 ); leftside->setSpacing( spacing ); leftside->setContentsMargins( margin, margin, margin, margin );
   {
      int j = 0;
      leftside->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      j++;
      leftside->addWidget(cb_vbar, j, 0);
      leftside->addWidget(pb_vbar, j, 1);
      j++;
      leftside->addWidget(lbl_covolume, j, 0);
      leftside->addWidget(le_covolume, j, 1);
      j++;
      leftside->addWidget(lbl_vbar, j, 0);
      leftside->addWidget(le_vbar, j, 1);
      j++;
      leftside->addWidget(lbl_vbar_temperature, j, 0);
      leftside->addWidget(le_vbar_temperature, j, 1);
      j++;
      leftside->addWidget(lbl_hydrovol, j, 0);
      leftside->addWidget(cnt_hydrovol, j, 1);
      j++;
      leftside->addWidget( cb_pb_rule_on , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      j++;
      leftside->addWidget( lbl_avg_banner , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      j++;
      leftside->addWidget(lbl_avg_radius, j, 0);
      leftside->addWidget(cnt_avg_radius, j, 1);
      j++;
      leftside->addWidget(lbl_avg_mass, j, 0);
      leftside->addWidget(cnt_avg_mass, j, 1);
      j++;
      leftside->addWidget(lbl_avg_num_elect, j, 0);
      leftside->addWidget(cnt_avg_num_elect, j, 1);
      j++;
      leftside->addWidget(lbl_avg_protons, j, 0);
      leftside->addWidget(cnt_avg_protons, j, 1);
      j++;
      leftside->addWidget(lbl_avg_hydration, j, 0);
      leftside->addWidget(cnt_avg_hydration, j, 1);
      j++;
      leftside->addWidget(lbl_avg_volume, j, 0);
      leftside->addWidget(cnt_avg_volume, j, 1);
      j++;
      leftside->addWidget(lbl_avg_vbar, j, 0);
      leftside->addWidget(cnt_avg_vbar, j, 1);
      j++;

      leftside->addWidget( lbl_bead_model_controls , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      j++;
      leftside->addWidget(lbl_target_e_density, j, 0);
      leftside->addWidget(le_target_e_density, j, 1);
      j++;
      leftside->addWidget(lbl_target_volume, j, 0);
      leftside->addWidget(le_target_volume, j, 1);
      j++;
      leftside->addWidget(cb_set_target_on_load_pdb, j, 0);
      leftside->addWidget(cb_equalize_radii, j, 1);
      j++;
      leftside->addWidget( lbl_other , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      j++;


      // leftside->addWidget( cb_export_msroll , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1  ) - ( 0 ) );
      // j++;
      // leftside->addWidget( cb_export_ssbond , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1  ) - ( 0 ) );
      // j++;

      {
         QBoxLayout * hbl = new QHBoxLayout( 0 ); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
         hbl->addWidget( cb_export_msroll );
         hbl->addWidget( cb_export_ssbond );
         hbl->addWidget( cb_parallel_grpy );
         leftside->addLayout( hbl, j, 0, 1, 2 );
      }
      j++;

      // leftside->addWidget(lbl_vdw_ot_mult, j, 0);
      // leftside->addWidget(le_vdw_ot_mult, j, 1);
      // j++;

      leftside->addWidget(lbl_vdw_ot_dpct, j, 0);
      leftside->addWidget(le_vdw_ot_dpct, j, 1);
      j++;

      {
         QBoxLayout * hbl = new QHBoxLayout( 0 ); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
         hbl->addWidget(lbl_vdw_ot_mult );
         hbl->addWidget(le_vdw_ot_mult );
         hbl->addWidget( cb_vdw_ot_alt );
         hbl->addWidget( cb_vdw_saxs_water_beads );
         hbl->addWidget( cb_vdw_saxs_skip_pr0pair );
         leftside->addLayout( hbl, j, 0, 1, 2 );
      }
      j++;

      leftside->addWidget( lbl_auto_calc_hydro_method, j, 0 );
      leftside->addWidget( cmb_auto_calc_hydro_method, j, 1 );
      j++;
      

#if !defined( USE_RIGHTSIDE )
      leftside->addWidget(pb_help, j, 0);
      leftside->addWidget(pb_cancel, j, 1);
#endif
   }

#if defined( USE_RIGHTSIDE )
   QGridLayout * rightside = new QGridLayout(); rightside->setContentsMargins( 0, 0, 0, 0 ); rightside->setSpacing( 0 ); rightside->setSpacing( spacing ); rightside->setContentsMargins( margin, margin, margin, margin );
   {
      int j = 0;

      // rightside->addWidget( lbl_hydro_method , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      // j++;
      // rightside->addWidget(cb_hydro_supc, j, 0);
      // rightside->addWidget(cb_hydro_zeno, j, 1);
      // j++;
      // rightside->addWidget( lbl_threshold , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
      // j++;
      // rightside->addWidget(lbl_covolume, j, 0);
      // rightside->addWidget(le_covolume, j, 1);
      // j++;
      // rightside->addWidget(lbl_thresh_SS, j, 0);
      // rightside->addWidget(le_thresh_SS, j, 1);
      // j++;
      // rightside->addWidget(lbl_thresh_carb_O, j, 0);
      // rightside->addWidget(le_thresh_carb_O, j, 1);
      // j++;
      // rightside->addWidget(lbl_thresh_carb_N, j, 0);
      // rightside->addWidget(le_thresh_carb_N, j, 1);
      // j++;
      // rightside->setRowStretch(j, 1);
   }
   background->addLayout(rightside, j, 1);
#endif
   background->addLayout(leftside , j, 0);
   j++;


#if defined( USE_RIGHTSIDE )
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
#endif
}

void US_Hydrodyn_Misc::update_vbar(const QString &str)
{
   (*misc).vbar = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_vbar_temperature(const QString &str)
{
   (*misc).vbar_temperature = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_vbar_signal(float /* val1 */, float val2)
{
   QString str;
   // float vbar20;
   // vbar20 = val1;
   (*misc).vbar = val2;
   le_vbar->setText(str.sprintf("%5.3f", (*misc).vbar));
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::select_vbar()
{
#ifndef NO_DB
   float val;
   vbar_dlg = new US_Vbar_DB(20.0, &val, &val, true, false, 0);
   vbar_dlg->setPalette( PALET_FRAME );
   AUTFBACK( vbar_dlg );
   vbar_dlg->setWindowTitle(us_tr("V-bar Calculation"));
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_signal(float, float)));
   vbar_dlg->exec();
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
#endif
}

void US_Hydrodyn_Misc::set_vbar()
{
   (*misc).compute_vbar = cb_vbar->isChecked();
   le_vbar->setEnabled(!(*misc).compute_vbar);
   pb_vbar->setEnabled(!(*misc).compute_vbar);
   le_vbar_temperature->setEnabled(!(*misc).compute_vbar);
   emit vbar_changed();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_pb_rule_on()
{
   // if( cb_pb_rule_on->isChecked() )
   // {
   //    ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_atoms = 0;
   //    ((US_Hydrodyn *)us_hydrodyn)->pdb_parse.missing_residues = 0;
   // }

   (*misc).pb_rule_on = cb_pb_rule_on->isChecked();
   (*misc).restore_pb_rule = false;

   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   ((US_Hydrodyn *)us_hydrodyn)->set_disabled();
}

void US_Hydrodyn_Misc::update_target_e_density(const QString &str)
{
   (*misc).target_e_density = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_target_volume(const QString &str)
{
   (*misc).target_volume = str.toDouble();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
   if ( ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->isEnabled() &&
        (*misc).target_volume != 0e0 )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->setEnabled(true);
   } else {
      ((US_Hydrodyn *)us_hydrodyn)->pb_bead_saxs->setEnabled(false);
   }
}

void US_Hydrodyn_Misc::update_covolume(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "covolume" ] = str;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Misc::update_thresh_SS(const QString &str)
// {
//    ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_SS" ] = str;
//    // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Misc::update_thresh_carb_O(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_carb_O" ] = str;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_thresh_carb_N(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "thresh_carb_N" ] = str;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_vdw_ot_mult(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_mult" ] = str;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_vdw_ot_dpct(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_dpct" ] = str;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_vdw_ot_alt()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_alt" ] = cb_vdw_ot_alt->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_vdw_saxs_water_beads()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_water_beads" ] = cb_vdw_saxs_water_beads->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_vdw_saxs_skip_pr0pair()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_skip_pr0pair" ] = cb_vdw_saxs_skip_pr0pair->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_set_target_on_load_pdb()
{
   (*misc).set_target_on_load_pdb = cb_set_target_on_load_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_equalize_radii()
{
   (*misc).equalize_radii = cb_equalize_radii->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

// void US_Hydrodyn_Misc::set_hydro_supc()
// {
//    (*misc).hydro_supc = cb_hydro_supc->isChecked();
//    (*misc).hydro_zeno = !cb_hydro_supc->isChecked();
//    cb_hydro_zeno->setChecked( (*misc).hydro_zeno );

//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

// void US_Hydrodyn_Misc::set_hydro_zeno()
// {
//    (*misc).hydro_zeno = cb_hydro_zeno->isChecked();
//    (*misc).hydro_supc = !cb_hydro_zeno->isChecked();
//    cb_hydro_supc->setChecked( (*misc).hydro_supc );
      
//    ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
// }

void US_Hydrodyn_Misc::set_export_msroll()
{
   (*misc).export_msroll = cb_export_msroll->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_export_ssbond()
{
   (*misc).export_ssbond = cb_export_ssbond->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::set_parallel_grpy()
{
   (*misc).parallel_grpy = cb_parallel_grpy->isChecked();
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::cancel()
{
   close();
}

void US_Hydrodyn_Misc::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_misc.html");
}

void US_Hydrodyn_Misc::update_hydrovol(double val)
{
   (*misc).hydrovol = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_radius(double val)
{
   (*misc).avg_radius = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_mass(double val)
{
   (*misc).avg_mass = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_num_elect(double val)
{
   (*misc).avg_num_elect = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_protons(double val)
{
   (*misc).avg_protons = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_hydration(double val)
{
   (*misc).avg_hydration = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_volume(double val)
{
   (*misc).avg_volume = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_avg_vbar(double val)
{
   (*misc).avg_vbar = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::update_auto_calc_hydro_method() {
   int val = cmb_auto_calc_hydro_method->currentData().toInt();
   qDebug() << "US_Hydrodyn_Misc::update_auto_calc_hydro_method() " << val;
   (*misc).auto_calc_hydro_method = (CALC_HYDRO_METHOD) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Misc::closeEvent(QCloseEvent *e)
{
   *misc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

