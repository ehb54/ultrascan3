#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs_conc.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3GridLayout>
#include <Q3Frame>
#include <QLabel>

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsGuinier::US_Hydrodyn_SasOptionsGuinier(struct saxs_options *saxs_options, bool *sas_options_guinier_widget, void *us_hydrodyn, QWidget *p, const char *name) : Q3Frame(p, name)
{
   this->sas_options_guinier_widget = sas_options_guinier_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_guinier_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("US-SOMO SAS Guinier Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsGuinier::~US_Hydrodyn_SasOptionsGuinier()
{
   *sas_options_guinier_widget = false;
}

void US_Hydrodyn_SasOptionsGuinier::setupGUI()
{
   int minHeight1 = 30;

   lbl_guinier = new QLabel(tr("Guinier Options:"), this);
   lbl_guinier->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier->setPalette( PALET_FRAME );
   AUTFBACK( lbl_guinier );
   lbl_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_qRgmax = new QLabel(tr(" Guinier: Maximum q * Rg : "), this);
   lbl_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qRgmax->setPalette( PALET_LABEL );
   AUTFBACK( lbl_qRgmax );
   lbl_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qRgmax = new QLineEdit(this);
   le_qRgmax->setValidator( new QDoubleValidator( le_qRgmax) );
   ( (QDoubleValidator *)le_qRgmax->validator() )->setRange( 0.5, 3, 3 );
   le_qRgmax->setText( QString( "%1" ).arg( (*saxs_options).qRgmax ) );
   le_qRgmax->setEnabled(true);
   le_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qRgmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_qRgmax );
   connect(le_qRgmax, SIGNAL( textChanged( const QString & )), SLOT(update_qRgmax( const QString & )));

   //    lbl_cs_guinier = new QLabel(tr("CS and TV Guinier Options:"), this);
   //    lbl_cs_guinier->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   //    lbl_cs_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   //    lbl_cs_guinier->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   //    lbl_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_cs_qRgmax = new QLabel(tr(" CS Guinier: Maximum q * Rc : "), this);
   lbl_cs_qRgmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qRgmax->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cs_qRgmax );
   lbl_cs_qRgmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qRgmax = new QLineEdit(this);
   le_cs_qRgmax->setValidator( new QDoubleValidator( le_cs_qRgmax) );
   ( (QDoubleValidator *)le_qRgmax->validator() )->setRange( 0.5, 3, 3 );
   le_cs_qRgmax->setText( QString( "%1" ).arg( (*saxs_options).cs_qRgmax ) );
   le_cs_qRgmax->setEnabled(true);
   le_cs_qRgmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qRgmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_cs_qRgmax );
   connect(le_cs_qRgmax, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qRgmax( const QString & )));

   lbl_Rt_qRtmax = new QLabel(tr(" TV Guinier: Maximum q * Rt : "), this);
   lbl_Rt_qRtmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_Rt_qRtmax->setPalette( PALET_LABEL );
   AUTFBACK( lbl_Rt_qRtmax );
   lbl_Rt_qRtmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_Rt_qRtmax = new QLineEdit(this);
   le_Rt_qRtmax->setValidator( new QDoubleValidator( le_Rt_qRtmax) );
   ( (QDoubleValidator *)le_Rt_qRtmax->validator() )->setRange( 0.5, 3, 3 );
   le_Rt_qRtmax->setText( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "guinier_qRtmax" ) ? ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_qRtmax" ] : "" );
   le_Rt_qRtmax->setEnabled(true);
   le_Rt_qRtmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_Rt_qRtmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_Rt_qRtmax );
   connect(le_Rt_qRtmax, SIGNAL( textChanged( const QString & )), SLOT(update_Rt_qRtmax( const QString & )));

   lbl_cs_qstart = new QLabel(tr(" Minimum q^2 value : "), this);
   lbl_cs_qstart->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qstart->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cs_qstart );
   lbl_cs_qstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qstart = new QLineEdit(this);
   le_cs_qstart->setValidator( new QDoubleValidator( le_cs_qstart ) );
   ( (QDoubleValidator *)le_cs_qstart->validator() )->setRange( 0, 1, 3 );
   le_cs_qstart->setText( QString( "%1" ).arg( (*saxs_options).cs_qstart ) );
   le_cs_qstart->setEnabled(true);
   le_cs_qstart->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qstart->setPalette( PALET_NORMAL );
   AUTFBACK( le_cs_qstart );
   connect(le_cs_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qstart( const QString & )));

   lbl_cs_qend = new QLabel(tr(" Maximum q^2 value : "), this);
   lbl_cs_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cs_qend->setPalette( PALET_LABEL );
   AUTFBACK( lbl_cs_qend );
   lbl_cs_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_cs_qend = new QLineEdit(this);
   le_cs_qend->setValidator( new QDoubleValidator( le_cs_qend ) );
   ( (QDoubleValidator *)le_cs_qend->validator() )->setRange( 0, 1, 3 );
   le_cs_qend->setText( QString( "%1" ).arg( (*saxs_options).cs_qend ) );
   le_cs_qend->setEnabled(true);
   le_cs_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_qend->setPalette( PALET_NORMAL );
   AUTFBACK( le_cs_qend );
   connect(le_cs_qend, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qend( const QString & )));

   lbl_guinier_and_cs_guinier = new QLabel(tr("Guinier, CS Guinier and TV Guinier Options:"), this);
   lbl_guinier_and_cs_guinier->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_guinier_and_cs_guinier->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_guinier_and_cs_guinier->setPalette( PALET_FRAME );
   AUTFBACK( lbl_guinier_and_cs_guinier );
   lbl_guinier_and_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_qstart = new QLabel(tr(" Minimum q value : "), this);
   lbl_qstart->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qstart->setPalette( PALET_LABEL );
   AUTFBACK( lbl_qstart );
   lbl_qstart->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qstart = new QLineEdit(this);
   le_qstart->setValidator( new QDoubleValidator( le_qstart ) );
   ( (QDoubleValidator *)le_qstart->validator() )->setRange( 0, 1, 3 );
   le_qstart->setText( QString( "%1" ).arg( (*saxs_options).qstart ) );
   le_qstart->setEnabled(true);
   le_qstart->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qstart->setPalette( PALET_NORMAL );
   AUTFBACK( le_qstart );
   connect(le_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));

   lbl_qend = new QLabel(tr(" Maximum q value : "), this);
   lbl_qend->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_qend->setPalette( PALET_LABEL );
   AUTFBACK( lbl_qend );
   lbl_qend->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_qend = new QLineEdit(this);
   le_qend->setValidator( new QDoubleValidator( le_qend ) );
   ( (QDoubleValidator *)le_qend->validator() )->setRange( 0, 1, 3 );
   le_qend->setText( QString( "%1" ).arg( (*saxs_options).qend ) );
   le_qend->setEnabled(true);
   le_qend->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_qend->setPalette( PALET_NORMAL );
   AUTFBACK( le_qend );
   connect(le_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));

   cb_guinier_auto_fit = new QCheckBox(this);
   cb_guinier_auto_fit->setText(tr(" Search for best Guinier range "));
   cb_guinier_auto_fit->setEnabled(true);
   cb_guinier_auto_fit->setChecked( ( ( US_Hydrodyn * )us_hydrodyn)->gparams.count( "guinier_auto_fit" ) &&
                                    ( ( US_Hydrodyn * )us_hydrodyn)->gparams[ "guinier_auto_fit" ] == "1" );
   cb_guinier_auto_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_auto_fit->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_auto_fit );
   connect(cb_guinier_auto_fit, SIGNAL(clicked()), this, SLOT(set_guinier_auto_fit()));

   lbl_pointsmin = new QLabel(tr(" Minimum number of points : "), this);
   lbl_pointsmin->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmin->setPalette( PALET_LABEL );
   AUTFBACK( lbl_pointsmin );
   lbl_pointsmin->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmin = new QLineEdit(this);
   le_pointsmin->setValidator( new QIntValidator( le_pointsmin ) );
   ( (QIntValidator *)le_pointsmin->validator() )->setRange( 4, 50 );
   le_pointsmin->setText( QString( "%1" ).arg( (*saxs_options).pointsmin ) );
   le_pointsmin->setEnabled(true);
   le_pointsmin->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmin->setPalette( PALET_NORMAL );
   AUTFBACK( le_pointsmin );
   connect(le_pointsmin, SIGNAL( textChanged( const QString & )), SLOT(update_pointsmin( const QString & )));

   lbl_pointsmax = new QLabel(tr(" Maximum number of points : "), this);
   lbl_pointsmax->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_pointsmax->setPalette( PALET_LABEL );
   AUTFBACK( lbl_pointsmax );
   lbl_pointsmax->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_pointsmax = new QLineEdit(this);
   le_pointsmax->setValidator( new QIntValidator( le_pointsmax ) );
   ( (QIntValidator *)le_pointsmax->validator() )->setRange( 10, 100 );
   le_pointsmax->setText( QString( "%1" ).arg( (*saxs_options).pointsmax ) );
   le_pointsmax->setEnabled(true);
   le_pointsmax->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pointsmax->setPalette( PALET_NORMAL );
   AUTFBACK( le_pointsmax );
   connect(le_pointsmax, SIGNAL( textChanged( const QString & ) ), SLOT(update_pointsmax( const QString & )));

   cb_guinier_use_qRlimit = new QCheckBox(this);
   cb_guinier_use_qRlimit->setText(tr(" Limit maximum q to maximum q*Rg, q*Rc or q*Rg (not active in Search mode)"));
   cb_guinier_use_qRlimit->setEnabled(true);
   cb_guinier_use_qRlimit->setChecked( ( ( US_Hydrodyn * )us_hydrodyn)->gparams.count( "guinier_use_qRlimit" ) &&
                                       ( ( US_Hydrodyn * )us_hydrodyn)->gparams[ "guinier_use_qRlimit" ] == "1" );
   cb_guinier_use_qRlimit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_use_qRlimit->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_use_qRlimit );
   connect(cb_guinier_use_qRlimit, SIGNAL(clicked()), this, SLOT(set_guinier_use_qRlimit()));

   cb_guinier_use_sd = new QCheckBox(this);
   cb_guinier_use_sd->setText(tr(" Use SDs for fitting "));
   cb_guinier_use_sd->setEnabled(true);
   cb_guinier_use_sd->setChecked((*saxs_options).guinier_use_sd);
   cb_guinier_use_sd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_use_sd->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_use_sd );
   connect(cb_guinier_use_sd, SIGNAL(clicked()), this, SLOT(set_guinier_use_sd()));

   cb_guinier_outlier_reject = new QCheckBox(this);
   cb_guinier_outlier_reject->setText(tr(" Repeat the analysis after discarding points\n over the regression line by more than SD of "));
   cb_guinier_outlier_reject->setEnabled(true);
   cb_guinier_outlier_reject->setChecked((*saxs_options).guinier_outlier_reject);
   cb_guinier_outlier_reject->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_outlier_reject->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_outlier_reject );
   connect(cb_guinier_outlier_reject, SIGNAL(clicked()), this, SLOT(set_guinier_outlier_reject()));

   le_guinier_outlier_reject_dist = new QLineEdit(this);
   le_guinier_outlier_reject_dist->setValidator( new QDoubleValidator( le_guinier_outlier_reject_dist ) );
   ( (QDoubleValidator *)le_guinier_outlier_reject_dist->validator() )->setRange( 0, 1, 2 );
   le_guinier_outlier_reject_dist->setText( QString( "%1" ).arg( (*saxs_options).guinier_outlier_reject_dist ) );
   le_guinier_outlier_reject_dist->setEnabled(true);
   le_guinier_outlier_reject_dist->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_guinier_outlier_reject_dist->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_outlier_reject_dist );
   connect(le_guinier_outlier_reject_dist, SIGNAL( textChanged( const QString & )), SLOT(update_guinier_outlier_reject_dist( const QString & )));

   cb_guinier_csv = new QCheckBox(this);
   cb_guinier_csv->setText(tr(" Save Guinier results to csv file: "));
   cb_guinier_csv->setEnabled(true);
   cb_guinier_csv->setChecked((*saxs_options).guinier_csv);
   cb_guinier_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_csv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_csv );
   connect(cb_guinier_csv, SIGNAL(clicked()), this, SLOT(set_guinier_csv()));

   le_guinier_csv_filename = new QLineEdit(this, "guinier_csv_filename Line Edit");
   le_guinier_csv_filename->setText((*saxs_options).guinier_csv_filename);
   le_guinier_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_guinier_csv_filename->setPalette( PALET_NORMAL );
   AUTFBACK( le_guinier_csv_filename );
   le_guinier_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_guinier_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_guinier_csv_filename(const QString &)));

   cb_guinier_csv_save_data = new QCheckBox(this);
   cb_guinier_csv_save_data->setText(tr(" Save processed q, I(q) data to csv file"));
   cb_guinier_csv_save_data->setEnabled(true);
   cb_guinier_csv_save_data->setChecked( ( ( US_Hydrodyn * )us_hydrodyn)->gparams.count( "guinier_csv_save_data" ) &&
                                         ( ( US_Hydrodyn * )us_hydrodyn)->gparams[ "guinier_csv_save_data" ] == "1" );
   cb_guinier_csv_save_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_guinier_csv_save_data->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_csv_save_data );
   connect(cb_guinier_csv_save_data, SIGNAL(clicked()), this, SLOT(set_guinier_csv_save_data()));

   lbl_conc_header = new QLabel(tr("MW and M/L computation options:"), this);
   lbl_conc_header->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_conc_header->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_conc_header->setPalette( PALET_FRAME );
   AUTFBACK( lbl_conc_header );
   lbl_conc_header->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_curve_conc = new QPushButton(tr("Set Curve Concentration, PSV, I0 standard experimental"), this);
   pb_curve_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   // pb_curve_conc->setMinimumHeight(minHeight1);
   pb_curve_conc->setPalette( PALET_PUSHB );
   connect(pb_curve_conc, SIGNAL(clicked()), SLOT(curve_conc()));

   lbl_conc = new QLabel(tr(" Default concentration (mg/ml) : "), this);
   lbl_conc->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_conc->setPalette( PALET_LABEL );
   AUTFBACK( lbl_conc );
   lbl_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_conc = new QLineEdit(this);
   le_conc->setValidator( new QDoubleValidator( le_conc ) );
   le_conc->setText( QString( "%1" ).arg( (*saxs_options).conc ) );
   le_conc->setEnabled(true);
   le_conc->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_conc->setPalette( PALET_NORMAL );
   AUTFBACK( le_conc );
   connect(le_conc, SIGNAL( textChanged( const QString & )), SLOT(update_conc( const QString &)));

   lbl_psv = new QLabel( tr(" Default partial specific volume (ml/g): "), this);
   lbl_psv->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_psv->setPalette( PALET_LABEL );
   AUTFBACK( lbl_psv );
   lbl_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_psv = new QLineEdit(this);
   le_psv->setValidator( new QDoubleValidator( le_psv ) );
   le_psv->setText( QString( "%1" ).arg( (*saxs_options).psv ) );
   le_psv->setEnabled(true);
   le_psv->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_psv->setPalette( PALET_NORMAL );
   AUTFBACK( le_psv );
   connect(le_psv, SIGNAL( textChanged( const QString & )), SLOT(update_psv( const QString &)));

   cb_use_cs_psv = new QCheckBox(this);
   cb_use_cs_psv->setText( tr(" Partial specific volume override for CS (ml/g): ") );
   cb_use_cs_psv->setEnabled( true );
   connect( cb_use_cs_psv, SIGNAL( clicked() ), SLOT( set_use_cs_psv() ) );
   cb_use_cs_psv->setChecked((*saxs_options).use_cs_psv );
   cb_use_cs_psv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_use_cs_psv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_use_cs_psv );

   le_cs_psv = new QLineEdit(this);
   le_cs_psv->setValidator( new QDoubleValidator( le_cs_psv ) );
   le_cs_psv->setText( QString( "%1" ).arg( (*saxs_options).cs_psv ) );
   le_cs_psv->setEnabled(true);
   le_cs_psv->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_cs_psv->setPalette( PALET_NORMAL );
   AUTFBACK( le_cs_psv );
   connect(le_cs_psv, SIGNAL( textChanged( const QString & )), SLOT(update_cs_psv( const QString &)));

   lbl_diffusion_len = new QLabel(tr(" Diffusion length (cm) : "), this);
   lbl_diffusion_len->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_diffusion_len->setPalette( PALET_LABEL );
   AUTFBACK( lbl_diffusion_len );
   lbl_diffusion_len->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_diffusion_len = new QLineEdit(this);
   le_diffusion_len->setValidator( new QDoubleValidator( le_diffusion_len ) );
   le_diffusion_len->setText( QString( "%1" ).arg( (*saxs_options).diffusion_len ) );
   le_diffusion_len->setEnabled(true);
   le_diffusion_len->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_diffusion_len->setPalette( PALET_NORMAL );
   AUTFBACK( le_diffusion_len );
   connect(le_diffusion_len, SIGNAL( textChanged( const QString & )), SLOT(update_diffusion_len( const QString &)));

   lbl_electron_nucleon_ratio = new QLabel(tr(" Electron/nucleon ratio Z/A : "), this);
   lbl_electron_nucleon_ratio->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_electron_nucleon_ratio->setPalette( PALET_LABEL );
   AUTFBACK( lbl_electron_nucleon_ratio );
   lbl_electron_nucleon_ratio->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_electron_nucleon_ratio = new QLineEdit(this);
   le_electron_nucleon_ratio->setValidator( new QDoubleValidator( le_electron_nucleon_ratio ) );
   le_electron_nucleon_ratio->setText( QString( "%1" ).arg( 
                                                           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_electron_nucleon_ratio" ) ?
                                                           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_electron_nucleon_ratio" ].toDouble() : 1.87e0 ) );
   le_electron_nucleon_ratio->setEnabled(true);
   le_electron_nucleon_ratio->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_electron_nucleon_ratio->setPalette( PALET_NORMAL );
   AUTFBACK( le_electron_nucleon_ratio );
   connect(le_electron_nucleon_ratio, SIGNAL( textChanged( const QString & )), SLOT(update_electron_nucleon_ratio( const QString &)));

   lbl_nucleon_mass = new QLabel(tr(" Nucleon mass (g) : "), this);
   lbl_nucleon_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_nucleon_mass->setPalette( PALET_LABEL );
   AUTFBACK( lbl_nucleon_mass );
   lbl_nucleon_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_nucleon_mass = new QLineEdit(this);
   le_nucleon_mass->setValidator( new QDoubleValidator( le_nucleon_mass ) );
   le_nucleon_mass->setText( QString( "%1" ).arg( (*saxs_options).nucleon_mass ) );
   le_nucleon_mass->setEnabled(true);
   le_nucleon_mass->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_nucleon_mass->setPalette( PALET_NORMAL );
   AUTFBACK( le_nucleon_mass );
   connect(le_nucleon_mass, SIGNAL( textChanged( const QString & )), SLOT(update_nucleon_mass( const QString &)));

   cb_guinier_use_standards = new QCheckBox(this);
   cb_guinier_use_standards->setText( tr(" Use I0 standards for normalization") );
   cb_guinier_use_standards->setEnabled( true );
   connect( cb_guinier_use_standards, SIGNAL( clicked() ), SLOT( set_guinier_use_standards() ) );
   cb_guinier_use_standards->setChecked((*saxs_options).guinier_use_standards );
   cb_guinier_use_standards->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_guinier_use_standards->setPalette( PALET_NORMAL );
   AUTFBACK( cb_guinier_use_standards );

   lbl_I0_exp = new QLabel(tr(" Default I0 standard experimental (a.u.) : "), this);
   lbl_I0_exp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_I0_exp->setPalette( PALET_LABEL );
   AUTFBACK( lbl_I0_exp );
   lbl_I0_exp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_exp = new QLineEdit(this);
   le_I0_exp->setValidator( new QDoubleValidator( le_I0_exp ) );
   le_I0_exp->setText( QString( "%1" ).arg( (*saxs_options).I0_exp ) );
   le_I0_exp->setEnabled(true);
   le_I0_exp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_exp->setPalette( PALET_NORMAL );
   AUTFBACK( le_I0_exp );
   connect(le_I0_exp, SIGNAL( textChanged( const QString & )), SLOT(update_I0_exp( const QString &)));

   lbl_I0_theo = new QLabel(tr(" I0 standard theoretical (cm^-1) : "), this);
   lbl_I0_theo->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_I0_theo->setPalette( PALET_LABEL );
   AUTFBACK( lbl_I0_theo );
   lbl_I0_theo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_I0_theo = new QLineEdit(this);
   le_I0_theo->setValidator( new QDoubleValidator( le_I0_theo ) );
   le_I0_theo->setText( QString( "%1" ).arg( (*saxs_options).I0_theo ) );
   le_I0_theo->setEnabled(true);
   le_I0_theo->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_I0_theo->setPalette( PALET_NORMAL );
   AUTFBACK( le_I0_theo );
   connect(le_I0_theo, SIGNAL( textChanged( const QString & )), SLOT(update_I0_theo( const QString &)));

   pb_guinier = new QPushButton(tr("Process Guinier"), this);
   pb_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_guinier->setMinimumHeight(minHeight1);
   pb_guinier->setPalette( PALET_PUSHB );
   connect(pb_guinier, SIGNAL(clicked()), SLOT(guinier()));

   pb_cs_guinier = new QPushButton(tr("Process CS Guinier"), this);
   pb_cs_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cs_guinier->setMinimumHeight(minHeight1);
   pb_cs_guinier->setPalette( PALET_PUSHB );
   connect(pb_cs_guinier, SIGNAL(clicked()), SLOT(cs_guinier()));

   pb_Rt_guinier = new QPushButton(tr("Process TV Guinier"), this);
   pb_Rt_guinier->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_Rt_guinier->setMinimumHeight(minHeight1);
   pb_Rt_guinier->setPalette( PALET_PUSHB );
   connect(pb_Rt_guinier, SIGNAL(clicked()), SLOT(Rt_guinier()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   Q3GridLayout *background=new Q3GridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_guinier, j, j, 0, 1);
   j++;
   background->addWidget(lbl_qRgmax, j, 0);
   background->addWidget(le_qRgmax, j, 1);
   j++;

   // background->addMultiCellWidget(lbl_cs_guinier, j, j, 0, 1);
   // j++;
   background->addWidget(lbl_cs_qRgmax, j, 0);
   background->addWidget(le_cs_qRgmax, j, 1);
   j++;

   background->addWidget(lbl_Rt_qRtmax, j, 0);
   background->addWidget(le_Rt_qRtmax, j, 1);
   j++;

   background->addMultiCellWidget(lbl_guinier_and_cs_guinier, j, j, 0, 1);
   j++;
   background->addWidget(lbl_qstart, j, 0);
   background->addWidget(le_qstart, j, 1);
   j++;
   background->addWidget(lbl_qend, j, 0);
   background->addWidget(le_qend, j, 1);
   j++;
   background->addWidget(lbl_cs_qstart, j, 0);
   background->addWidget(le_cs_qstart, j, 1);
   j++;
   background->addWidget(lbl_cs_qend, j, 0);
   background->addWidget(le_cs_qend, j, 1);
   j++;
   background->addMultiCellWidget(cb_guinier_use_qRlimit, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(cb_guinier_use_sd, j, j, 0, 1);
   j++;
   background->addWidget(cb_guinier_outlier_reject, j, 0);
   background->addWidget(le_guinier_outlier_reject_dist, j, 1);
   j++;
   background->addWidget(cb_guinier_csv, j, 0);
   background->addWidget(le_guinier_csv_filename, j, 1);
   j++;

   background->addMultiCellWidget(cb_guinier_csv_save_data, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(cb_guinier_auto_fit, j, j, 0, 1);
   j++;
   background->addWidget(lbl_pointsmin, j, 0);
   background->addWidget(le_pointsmin, j, 1);
   j++;
   background->addWidget(lbl_pointsmax, j, 0);
   background->addWidget(le_pointsmax, j, 1);
   j++;

   background->addMultiCellWidget(lbl_conc_header, j, j, 0, 1);
   j++;

   background->addMultiCellWidget(pb_curve_conc, j, j, 0, 1);
   j++;

   background->addWidget(lbl_conc, j, 0);
   background->addWidget(le_conc, j, 1);
   j++;

   background->addWidget(lbl_psv, j, 0);
   background->addWidget(le_psv, j, 1);
   j++;

   background->addWidget(cb_use_cs_psv, j, 0);
   background->addWidget(le_cs_psv, j, 1);
   j++;

   background->addWidget(lbl_diffusion_len, j, 0);
   background->addWidget(le_diffusion_len, j, 1);
   j++;

   background->addWidget(lbl_electron_nucleon_ratio, j, 0);
   background->addWidget(le_electron_nucleon_ratio, j, 1);
   j++;

   background->addWidget(lbl_nucleon_mass, j, 0);
   background->addWidget(le_nucleon_mass, j, 1);
   j++;

   background->addMultiCellWidget( cb_guinier_use_standards, j, j, 0, 1);
   j++;

   background->addWidget(lbl_I0_exp, j, 0);
   background->addWidget(le_I0_exp, j, 1);
   j++;

   background->addWidget(lbl_I0_theo, j, 0);
   background->addWidget(le_I0_theo, j, 1);
   j++;



   {
      Q3GridLayout * gl2 = new Q3GridLayout( 0 );
      
      Q3HBoxLayout *hbl = new Q3HBoxLayout;
      hbl->addWidget( pb_guinier );
      hbl->addWidget( pb_cs_guinier );
      hbl->addWidget( pb_Rt_guinier );
      gl2->addMultiCellLayout( hbl, 0, 0, 0, 1 );

      gl2->addWidget( pb_help       , 1, 0 );
      gl2->addWidget( pb_cancel     , 1, 1 );

      background->addMultiCellLayout( gl2, j, j, 0, 1);
      j++;
   }

   if ( !U_EXPT )
   {
      cb_use_cs_psv->hide();
      le_cs_psv->hide();
   }
      
   setMinimumWidth( 400 );
}

void US_Hydrodyn_SasOptionsGuinier::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsGuinier::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_guinier.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsGuinier::closeEvent(QCloseEvent *e)
{
   *sas_options_guinier_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsGuinier::update_qRgmax( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qstart( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qstart = val;
   (*saxs_options).cs_qstart = val * val;
   disconnect(le_cs_qstart, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_cs_qstart->setText( QString( "%1" ).arg( (*saxs_options).cs_qstart ) );
   connect(le_cs_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qstart( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_qend( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).qend = val;
   (*saxs_options).cs_qend = val * val;
   disconnect(le_cs_qend, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_cs_qend->setText( QString( "%1" ).arg( (*saxs_options).cs_qend ) );
   connect(le_cs_qend, SIGNAL( textChanged( const QString & )), SLOT(update_cs_qend( const QString & )));
   
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qRgmax( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qRgmax = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_Rt_qRtmax( const QString & str )
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_qRtmax" ] = str;
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qstart( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qstart = val;
   (*saxs_options).qstart = sqrt( val );
   disconnect(le_qstart, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_qstart->setText( QString( "%1" ).arg( (*saxs_options).qstart ) );
   connect(le_qstart, SIGNAL( textChanged( const QString & )), SLOT(update_qstart( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_qend( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_qend = val;
   (*saxs_options).qend = sqrt( val );
   disconnect(le_qend, SIGNAL( textChanged( const QString & )), 0, 0 );
   le_qend->setText( QString( "%1" ).arg( (*saxs_options).qend ) );
   connect(le_qend, SIGNAL( textChanged( const QString & )), SLOT(update_qend( const QString & )));
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_pointsmin( const QString & str )
{
   unsigned int val = str.toUInt();
   (*saxs_options).pointsmin = (unsigned int) val;
   if ( (*saxs_options).pointsmax < (unsigned int) val )
   {
      (*saxs_options).pointsmax = (unsigned int) val;
      le_pointsmax->setText( QString( "%1" ).arg( val ) );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_pointsmax( const QString & str )
{
   unsigned int val = str.toUInt();
   (*saxs_options).pointsmax = val;
   if ( (*saxs_options).pointsmin > val )
   {
      (*saxs_options).pointsmin = val;
      le_pointsmin->setText( QString( "%1" ).arg( val ) );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_csv()
{
   (*saxs_options).guinier_csv = cb_guinier_csv->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_guinier_csv_filename(const QString &str)
{
   (*saxs_options).guinier_csv_filename = str;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_conc( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).conc = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_psv( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).psv = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_use_qRlimit()
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_use_qRlimit" ] = cb_guinier_use_qRlimit->isChecked() ? "1" : "0";
   if ( cb_guinier_use_qRlimit->isChecked() &&
        cb_guinier_auto_fit->isChecked() )
   {
      cb_guinier_auto_fit->setChecked( false );
      set_guinier_auto_fit();
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_use_sd()
{
   (*saxs_options).guinier_use_sd = cb_guinier_use_sd->isChecked();
   if ( !cb_guinier_use_sd->isChecked() &&
        cb_guinier_outlier_reject->isChecked() )
   {
      cb_guinier_outlier_reject->setChecked( false );
      set_guinier_outlier_reject();
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_outlier_reject()
{
   (*saxs_options).guinier_outlier_reject = cb_guinier_outlier_reject->isChecked();
   if ( cb_guinier_outlier_reject->isChecked() &&
        !cb_guinier_use_sd->isChecked() )
   {
      cb_guinier_use_sd->setChecked( true );
      set_guinier_use_sd();
   }
   if ( cb_guinier_outlier_reject->isChecked() &&
        cb_guinier_auto_fit->isChecked() )
   {
      cb_guinier_auto_fit->setChecked( false );
      set_guinier_auto_fit();
   }
      
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_guinier_outlier_reject_dist( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).guinier_outlier_reject_dist = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_use_cs_psv()
{
   (*saxs_options).use_cs_psv = cb_use_cs_psv->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_cs_psv( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).cs_psv = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_use_standards()
{
   (*saxs_options).guinier_use_standards = cb_guinier_use_standards->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_I0_exp( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).I0_exp = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_I0_theo( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).I0_theo = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_diffusion_len( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).diffusion_len = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_electron_nucleon_ratio( const QString & str )
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_electron_nucleon_ratio" ] = QString( "%1" ).arg( str.toDouble(), 0, 'g', 8 );
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::update_nucleon_mass( const QString & str )
{
   double val = str.toDouble();
   (*saxs_options).nucleon_mass = val;
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::guinier()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->show();
      }
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->run_guinier_analysis();
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}

void US_Hydrodyn_SasOptionsGuinier::cs_guinier()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->show();
      }
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->run_guinier_cs();
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}

void US_Hydrodyn_SasOptionsGuinier::Rt_guinier()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->show();
      }
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->run_guinier_Rt();
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}

void US_Hydrodyn_SasOptionsGuinier::curve_conc()
{
   if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->sync_conc_csv();
      US_Hydrodyn_Saxs_Conc *hsc = 
         new US_Hydrodyn_Saxs_Conc(
                                   ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window->conc_csv,
                                   ((US_Hydrodyn *)us_hydrodyn)->saxs_plot_window,
                                   this 
                                   );
      US_Hydrodyn::fixWinButtons( hsc );
      hsc->exec();
      delete hsc;
   } else {
      QMessageBox::message( caption() + ": Process Guinier",
                            tr( "The main SAS window is not active" ) );
   }
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_auto_fit()
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_auto_fit" ] = cb_guinier_auto_fit->isChecked() ? "1" : "0";
   if ( cb_guinier_auto_fit->isChecked() )
   {
      if ( cb_guinier_outlier_reject->isChecked() )
      {
         cb_guinier_outlier_reject->setChecked( false );
         set_guinier_outlier_reject();
      }
      if ( cb_guinier_use_qRlimit->isChecked() )
      {
         cb_guinier_use_qRlimit->setChecked( false );
         set_guinier_use_qRlimit();
      }
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsGuinier::set_guinier_csv_save_data()
{
   ((US_Hydrodyn *)us_hydrodyn)->gparams[ "guinier_csv_save_data" ] = cb_guinier_csv_save_data->isChecked() ? "1" : "0";
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
