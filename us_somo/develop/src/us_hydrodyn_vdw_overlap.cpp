#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_vdw_overlap.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Vdw_Overlap::US_Hydrodyn_Vdw_Overlap(struct misc_options *misc,
                                                 bool *misc_widget, void *us_hydrodyn, QWidget *p, const char *) : QFrame( p )
{
   this->misc = misc;
   this->misc_widget = misc_widget;
   *misc_widget = true;
   this->us_hydrodyn = us_hydrodyn;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US_SOMO : vdW Overlap Parameters"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Vdw_Overlap::~US_Hydrodyn_Vdw_Overlap()
{
   *misc_widget = false;
}

void US_Hydrodyn_Vdw_Overlap::setupGUI()
{
   int minHeight1 = 22;
   auto hdrFontSize = USglobal->config_list.fontSize + 2;
   auto useFontSize = USglobal->config_list.fontSize + 1;
   
   QString str;
   lbl_info = new QLabel(us_tr("vdW Overlap Parameters:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   lbl_info->setMinimumWidth( 300 );

   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, hdrFontSize, QFont::Bold));

   lbl_vdw_ot_mult = new QLabel(us_tr(" vdW OT multiplier: "), this);
   lbl_vdw_ot_mult->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
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
      cb_vdw_ot_alt              ->hide();
      cb_vdw_saxs_water_beads    ->hide();
      cb_vdw_saxs_skip_pr0pair   ->hide();
   }

   // ot_dpct current not being used
   lbl_vdw_ot_dpct ->hide();
   le_vdw_ot_dpct  ->hide();

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

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   ++j;
   
   background->addWidget( lbl_vdw_ot_mult, j, 0 );
   background->addWidget( le_vdw_ot_mult, j, 1 );
   ++j;

   background->addWidget( lbl_vdw_ot_dpct, j, 0 );
   background->addWidget( le_vdw_ot_dpct, j, 1 );
   ++j;

   background->addWidget( cb_vdw_ot_alt, j, 0, 1, 2 );
   ++j;

   background->addWidget( cb_vdw_saxs_water_beads, j, 0, 1, 2 );
   ++j;

   background->addWidget( cb_vdw_saxs_skip_pr0pair, j, 0, 1, 2 );
   ++j;

   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
   ++j;
}

void US_Hydrodyn_Vdw_Overlap::update_vdw_ot_mult(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_mult" ] = str;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Vdw_Overlap::update_vdw_ot_dpct(const QString &str)
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_dpct" ] = str;
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Vdw_Overlap::set_vdw_ot_alt()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_ot_alt" ] = cb_vdw_ot_alt->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Vdw_Overlap::set_vdw_saxs_water_beads()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_water_beads" ] = cb_vdw_saxs_water_beads->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Vdw_Overlap::set_vdw_saxs_skip_pr0pair()
{
   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "vdw_saxs_skip_pr0pair" ] = cb_vdw_saxs_skip_pr0pair->isChecked() ? "true" : "false";
   // ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}


void US_Hydrodyn_Vdw_Overlap::cancel()
{
   close();
}

void US_Hydrodyn_Vdw_Overlap::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_vdw_overlap.html");
}

void US_Hydrodyn_Vdw_Overlap::closeEvent(QCloseEvent *e)
{
   *misc_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

