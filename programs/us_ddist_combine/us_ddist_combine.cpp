//! \file us_ddist_combine.cpp

#include <QApplication>

#include "us_ddist_combine.h"
#include "us_select_rundd.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_matrix.h"
#include "us_constants.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_util.h"
#include "us_model.h"
#include "us_math2.h"
#include "qwt_legend.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif


const double epsilon = 0.0005;    // Equivalence magnitude ratio radius

// US_DDistr_Combine class constructor
US_DDistr_Combine::US_DDistr_Combine( const QString auto_mode ) : US_Widgets()
{
   this->a_mode = auto_mode;
   
   // set up the GUI
   setWindowTitle( tr( "Combined Discrete Distributions:" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level     = US_Settings::us_debug();
   xtype         = 0;

   QBoxLayout*  mainLayout   = new QHBoxLayout( this );
                leftLayout   = new QGridLayout;
   QVBoxLayout* lfullLayout  = new QVBoxLayout;
   QVBoxLayout* rightLayout  = new QVBoxLayout;
   mainLayout ->setSpacing        ( 2 );
   mainLayout ->setContentsMargins( 2, 2, 2, 2 );
   leftLayout ->setSpacing        ( 0 );
   leftLayout ->setContentsMargins( 0, 1, 0, 1 );
   lfullLayout->setSpacing        ( 0 );
   lfullLayout->setContentsMargins( 0, 1, 0, 1 );
   rightLayout->setSpacing        ( 0 );
   rightLayout->setContentsMargins( 0, 1, 0, 1 );
   QFrame* frMidLeft         = new QFrame( this );
   QFrame* frBotLeft         = new QFrame( this );
   QVBoxLayout*  frmlLayout  = new QVBoxLayout( frMidLeft );
   QVBoxLayout*  frblLayout  = new QVBoxLayout( frBotLeft );

   dkdb_cntrls             = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   QPushButton* pb_loadda  = us_pushbutton( tr( "Select Run ID(s)"  ) );
                pb_saveda  = us_pushbutton( tr( "Save Data"  ) );
                pb_resetd  = us_pushbutton( tr( "Reset Data" ) );
                pb_resetp  = us_pushbutton( tr( "Reset Plot" ) );
   QPushButton* pb_help    = us_pushbutton( tr( "Help"       ) );
   QPushButton* pb_close   = us_pushbutton( tr( "Close"      ) );

   pb_saveda->setEnabled( false );
   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );

   QLabel* lb_distrtype  = us_banner( tr( "Select Distribution Type(s):" ) );
   QLabel* lb_plottype   = us_banner( tr( "Plot Type and Control:" ) );
   QLabel* lb_runinfo    = us_banner( tr( "Information for this Run:" ) );
   QLabel* lb_runid      = us_label ( tr( "Current Run ID:" ) );
   QLabel* lb_svproj     = us_label ( tr( "Save Plot under Project:" ) );
   QLabel* lb_runids     = us_banner( tr( "Run IDs:" ) );
   QLabel* lb_models     = us_banner( tr( "Distributions:" ) );

   QLayout* lo_2dsa     = us_checkbox( tr( "2DSA" ),       ck_2dsa,     true  );
   QLayout* lo_2dsait   = us_checkbox( tr( "2DSA-IT" ),    ck_2dsait,   false );
   QLayout* lo_2dsafm   = us_checkbox( tr( "2DSA-FM" ),    ck_2dsafm,   false );
   QLayout* lo_2dsamc   = us_checkbox( tr( "2DSA-MC" ),    ck_2dsamc,   false );
   QLayout* lo_2dsamw   = us_checkbox( tr( "2DSA-MW" ),    ck_2dsamw,   false );
   QLayout* lo_2dsamcmw = us_checkbox( tr( "2DSA-MC-MW" ), ck_2dsamcmw, false );
   QLayout* lo_2dsagl   = us_checkbox( tr( "2DSA-GL" ),    ck_2dsagl,   false );
   QLayout* lo_2dsaglmc = us_checkbox( tr( "2DSA-GL-MC" ), ck_2dsaglmc, false );
   QLayout* lo_2dsacg   = us_checkbox( tr( "2DSA-CG" ),    ck_2dsacg,   false );
   QLayout* lo_2dsacgit = us_checkbox( tr( "2DSA-CG-IT" ), ck_2dsacgit, false );
   QLayout* lo_2dsacgfm = us_checkbox( tr( "2DSA-CG-FM" ), ck_2dsacgfm, false );
   QLayout* lo_2dsacgmc = us_checkbox( tr( "2DSA-CG-MC" ), ck_2dsacgmc, false );
   QLayout* lo_ga       = us_checkbox( tr( "GA" ),         ck_ga,       false );
   QLayout* lo_gamc     = us_checkbox( tr( "GA-MC" ),      ck_gamc,     false );
   QLayout* lo_gamw     = us_checkbox( tr( "GA-MW" ),      ck_gamw,     false );
   QLayout* lo_gamcmw   = us_checkbox( tr( "GA-MC-MW" ),   ck_gamcmw,   false );
   QLayout* lo_gagl     = us_checkbox( tr( "GA-GL" ),      ck_gagl,     false );
   QLayout* lo_gaglmc   = us_checkbox( tr( "GA-GL-MC" ),   ck_gaglmc,   false );
   QLayout* lo_pcsais   = us_checkbox( tr( "PCSA-IS" ),    ck_pcsais,   false );
   QLayout* lo_pcsasl   = us_checkbox( tr( "PCSA-SL" ),    ck_pcsasl,   false );
   QLayout* lo_pcsads   = us_checkbox( tr( "PCSA-DS" ),    ck_pcsads,   false );
   QLayout* lo_pcsahl   = us_checkbox( tr( "PCSA-HL" ),    ck_pcsahl,   false );
   QLayout* lo_pcsaismc = us_checkbox( tr( "PCSA-IS-MC" ), ck_pcsaismc, false );
   QLayout* lo_pcsaslmc = us_checkbox( tr( "PCSA-SL-MC" ), ck_pcsaslmc, false );
   QLayout* lo_pcsadsmc = us_checkbox( tr( "PCSA-DS-MC" ), ck_pcsadsmc, false );
   QLayout* lo_pcsahlmc = us_checkbox( tr( "PCSA-HL-MC" ), ck_pcsahlmc, false );
   QLayout* lo_pcsaistr = us_checkbox( tr( "PCSA-IS-TR" ), ck_pcsaistr, false );
   QLayout* lo_pcsasltr = us_checkbox( tr( "PCSA-SL-TR" ), ck_pcsasltr, false );
   QLayout* lo_pcsadstr = us_checkbox( tr( "PCSA-DS-TR" ), ck_pcsadstr, false );
   QLayout* lo_pcsahltr = us_checkbox( tr( "PCSA-HL-TR" ), ck_pcsahltr, false );
   QLayout* lo_pcsa2o   = us_checkbox( tr( "PCSA-2O" ),    ck_pcsa2o,   false );
   QLayout* lo_pcsa2omc = us_checkbox( tr( "PCSA-2O-MC" ), ck_pcsa2omc, false );
   QLayout* lo_pcsa2otr = us_checkbox( tr( "PCSA-2O-TR" ), ck_pcsa2otr, false );
   QLayout* lo_dmga     = us_checkbox( tr( "DMGA" ),       ck_dmga,     false );
   QLayout* lo_dmgamc   = us_checkbox( tr( "DMGA-MC" ),    ck_dmgamc,   false );
   QLayout* lo_dmgara   = us_checkbox( tr( "DMGA-RA" ),    ck_dmgara,   false );
   QLayout* lo_dmgaramc = us_checkbox( tr( "DMGA-RA-MC" ), ck_dmgaramc, false );
   QLayout* lo_dmgagl   = us_checkbox( tr( "DMGA-GL" ),    ck_dmgagl,   false );
   QLayout* lo_dmgaglmc = us_checkbox( tr( "DMGA-GL-MC" ), ck_dmgaglmc, false );
   QLayout* lo_dtall    = us_checkbox( tr( "All" ),        ck_dtall,    false );
   QFont cfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 2,
                QFont::Bold );
   ck_2dsa    ->setFont( cfont );
   ck_2dsait  ->setFont( cfont );
   ck_2dsamc  ->setFont( cfont );
   ck_2dsamw  ->setFont( cfont );
   ck_2dsamcmw->setFont( cfont );
   ck_2dsagl  ->setFont( cfont );
   ck_2dsaglmc->setFont( cfont );
   ck_2dsacg  ->setFont( cfont );
   ck_2dsacgit->setFont( cfont );
   ck_2dsacgfm->setFont( cfont );
   ck_2dsacgmc->setFont( cfont );
   ck_2dsafm  ->setFont( cfont );
   ck_ga      ->setFont( cfont );
   ck_gamc    ->setFont( cfont );
   ck_gamw    ->setFont( cfont );
   ck_gamcmw  ->setFont( cfont );
   ck_gagl    ->setFont( cfont );
   ck_gaglmc  ->setFont( cfont );
   ck_pcsais  ->setFont( cfont );
   ck_pcsasl  ->setFont( cfont );
   ck_pcsads  ->setFont( cfont );
   ck_pcsahl  ->setFont( cfont );
   ck_pcsaismc->setFont( cfont );
   ck_pcsaslmc->setFont( cfont );
   ck_pcsadsmc->setFont( cfont );
   ck_pcsahlmc->setFont( cfont );
   ck_pcsaistr->setFont( cfont );
   ck_pcsasltr->setFont( cfont );
   ck_pcsadstr->setFont( cfont );
   ck_pcsahltr->setFont( cfont );
   ck_dmga    ->setFont( cfont );
   ck_dmgamc  ->setFont( cfont );
   ck_dmgara  ->setFont( cfont );
   ck_dmgaramc->setFont( cfont );
   ck_dmgagl  ->setFont( cfont );
   ck_dmgaglmc->setFont( cfont );
   ck_pcsa2o  ->setFont( cfont );
   ck_pcsa2omc->setFont( cfont );
   ck_pcsa2otr->setFont( cfont );
   ck_dtall   ->setFont( cfont );

   QButtonGroup* sel_plt  = new QButtonGroup( this );
   QGridLayout* lo_pltsw  = us_radiobutton( tr( "s20,W" ), rb_pltsw,    true  );
   QGridLayout* lo_pltMW  = us_radiobutton( tr( "MW"    ), rb_pltMW,    false );
   QGridLayout* lo_pltDw  = us_radiobutton( tr( "D20,W" ), rb_pltDw,    false );
   QGridLayout* lo_pltff0 = us_radiobutton( tr( "f/f0"  ), rb_pltff0,   false );
   QGridLayout* lo_pltvb  = us_radiobutton( tr( "vbar"  ), rb_pltvb,    false );
   QGridLayout* lo_pltMWl = us_radiobutton( tr( "MWlog" ), rb_pltMWl,   false );
   sel_plt->addButton( rb_pltsw,  0 );
   sel_plt->addButton( rb_pltMW,  1 );
   sel_plt->addButton( rb_pltDw,  2 );
   sel_plt->addButton( rb_pltff0, 3 );
   sel_plt->addButton( rb_pltvb,  4 );
   sel_plt->addButton( rb_pltMWl, 5 );
   QLayout* lo_mdltype  = us_checkbox(
         tr( "Use model descriptions for list and legend" ),
         ck_mdltype,  false );

   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid      = fmet.maxWidth();
   lb_sigma      = us_label( tr( "Envelope Gaussian Sigma:" ) );
   ct_sigma      = us_counter( 3,  0,  5, 1 );
   ct_sigma->setSingleStep( 0.001 );
   ct_sigma->setFont( sfont );
   ct_sigma->setValue( 0.01 );
   int rhgt      = ct_sigma ->height();
   int csizw     = fwid * 5;
   ct_sigma->setMinimumWidth( fwid );
   ct_sigma->resize( rhgt, csizw );
   lb_plxmin     = us_label( tr( "Plot X Minimum:" ) );
   lb_plxmax     = us_label( tr( "Plot X Maximum:" ) );
   le_plxmin     = us_lineedit( "0" );
   le_plxmax     = us_lineedit( "0" );

   le_runid      = us_lineedit( "(current run ID)", -1, true );
   cmb_svproj    = us_comboBox();
   lw_runids     = us_listwidget();
   lw_models     = us_listwidget();
   te_status     = us_textedit();
   us_setReadOnly( te_status, true );
   te_status->setTextColor( Qt::blue );

   int row = 0;
   leftLayout->addLayout( dkdb_cntrls,  row++, 0, 1, 8 );
   leftLayout->addWidget( pb_loadda,    row,   0, 1, 4 );
   leftLayout->addWidget( pb_saveda,    row++, 4, 1, 4 );
   leftLayout->addWidget( pb_resetd,    row,   0, 1, 2 );
   leftLayout->addWidget( pb_resetp,    row,   2, 1, 2 );
   leftLayout->addWidget( pb_help,      row,   4, 1, 2 );
   leftLayout->addWidget( pb_close,     row++, 6, 1, 2 );
   leftLayout->addWidget( lb_distrtype, row++, 0, 1, 8 );
   fckrow  = row;           // First checkbox row
   leftLayout->addLayout( lo_2dsa,      row,   0, 1, 2 );
   leftLayout->addLayout( lo_2dsait,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_2dsafm,    row,   4, 1, 2 );
   leftLayout->addLayout( lo_2dsamc,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_ga,        row,   0, 1, 2 );
   leftLayout->addLayout( lo_gamc,      row,   2, 1, 4 );
   leftLayout->addLayout( lo_dtall,     row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pcsais,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_pcsasl,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_pcsads,    row,   4, 1, 2 );
   leftLayout->addLayout( lo_pcsahl,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pcsaismc,  row,   0, 1, 2 );
   leftLayout->addLayout( lo_pcsaslmc,  row,   2, 1, 2 );
   leftLayout->addLayout( lo_pcsadsmc,  row,   4, 1, 2 );
   leftLayout->addLayout( lo_pcsahlmc,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_dmga,      row,   0, 1, 2 );
   leftLayout->addLayout( lo_dmgamc,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_dmgara,    row,   4, 1, 2 );
   leftLayout->addLayout( lo_dmgaramc,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_2dsacg,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_2dsacgit,  row,   2, 1, 2 );
   leftLayout->addLayout( lo_2dsacgfm,  row,   4, 1, 2 );
   leftLayout->addLayout( lo_2dsacgmc,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_2dsagl,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_2dsaglmc,  row,   2, 1, 2 );
   leftLayout->addLayout( lo_2dsamw,    row,   4, 1, 2 );
   leftLayout->addLayout( lo_2dsamcmw,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_gamw,      row,   0, 1, 2 );
   leftLayout->addLayout( lo_gamcmw,    row,   2, 1, 2 );
   leftLayout->addLayout( lo_gagl,      row,   4, 1, 2 );
   leftLayout->addLayout( lo_gaglmc,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pcsaistr,  row,   0, 1, 2 );
   leftLayout->addLayout( lo_pcsasltr,  row,   2, 1, 2 );
   leftLayout->addLayout( lo_pcsadstr,  row,   4, 1, 2 );
   leftLayout->addLayout( lo_pcsahltr,  row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pcsa2o,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_pcsa2omc,  row,   2, 1, 2 );
   leftLayout->addLayout( lo_pcsa2otr,  row++, 4, 1, 4 );
   leftLayout->addLayout( lo_dmgagl,    row,   0, 1, 2 );
   leftLayout->addLayout( lo_dmgaglmc,  row++, 2, 1, 6 );
   lckrow  = row;           // Last checkbox row

   leftLayout->addWidget( lb_plottype,  row++, 0, 1, 8 );
   leftLayout->addLayout( lo_pltsw,     row,   0, 1, 2 );
   leftLayout->addLayout( lo_pltMW,     row,   2, 1, 2 );
   leftLayout->addLayout( lo_pltDw,     row,   4, 1, 2 );
   leftLayout->addLayout( lo_pltff0,    row++, 6, 1, 2 );
   leftLayout->addLayout( lo_pltvb,     row,   0, 1, 8 );
   leftLayout->addLayout( lo_pltMWl,    row++, 2, 1, 8 );

   leftLayout->addWidget( lb_sigma,     row,   0, 1, 5 );
   leftLayout->addWidget( ct_sigma,     row++, 5, 1, 3 );
   leftLayout->addWidget( lb_plxmin,    row,   0, 1, 2 );
   leftLayout->addWidget( le_plxmin,    row,   2, 1, 2 );
   leftLayout->addWidget( lb_plxmax,    row,   4, 1, 2 );
   leftLayout->addWidget( le_plxmax,    row++, 6, 1, 2 );

   leftLayout->addWidget( lb_runinfo,   row++, 0, 1, 8 );
   leftLayout->addWidget( lb_runid,     row,   0, 1, 3 );
   leftLayout->addWidget( le_runid,     row++, 3, 1, 5 );
   leftLayout->addWidget( lb_svproj,    row,   0, 1, 3 );
   leftLayout->addWidget( cmb_svproj,   row++, 3, 1, 5 );

   frmlLayout ->addWidget( lb_runids  );
   frmlLayout ->addWidget( lw_runids  );
   frblLayout ->addLayout( lo_mdltype );
   frblLayout ->addWidget( lb_models  );
   frblLayout ->addWidget( lw_models  );

   QSplitter* spl1      = new QSplitter( Qt::Vertical, this );
   spl1->addWidget( frMidLeft );
   spl1->addWidget( frBotLeft );

   lfullLayout->addLayout( leftLayout );
   lfullLayout->addWidget( spl1       );
   lfullLayout->addWidget( te_status  );

   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );

   connect( pb_loadda, SIGNAL( clicked()    ),
            this,      SLOT(   load()       ) );
   connect( pb_saveda, SIGNAL( clicked()    ),
            this,      SLOT(   save()       ) );
   connect( pb_resetd, SIGNAL( clicked()    ),
            this,      SLOT(   reset_data() ) );
   connect( pb_resetp, SIGNAL( clicked()    ),
            this,      SLOT(   reset_plot() ) );
   connect( pb_help,   SIGNAL( clicked()    ),
            this,      SLOT(   help()       ) );
   connect( pb_close,  SIGNAL( clicked()    ),
            this,      SLOT(   close()      ) );

   connect( ck_2dsa,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsait,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamc,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamw,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsamcmw, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsagl,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsaglmc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsacg,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsacgit, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsacgfm, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsacgmc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_2dsafm,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_ga,       SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamc,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamw,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gamcmw,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gagl,     SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_gaglmc,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsais,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsasl,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsads,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsahl,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsaismc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsaslmc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsadsmc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsahlmc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsaistr, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsasltr, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsadstr, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsahltr, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsa2o,   SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsa2omc, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_pcsa2otr, SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   methodChanged   ( int ) ) );
   connect( ck_dtall,    SIGNAL( stateChanged    ( int ) ),
            this,        SLOT(   allMethodChanged( int ) ) );

   connect( rb_pltsw,    SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );
   connect( rb_pltMW,    SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );
   connect( rb_pltDw,    SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );
   connect( rb_pltff0,   SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );
   connect( rb_pltvb,    SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );
   connect( rb_pltMWl,   SIGNAL( toggled     ( bool ) ),
            this,        SLOT(   changedPlotX( bool ) ) );

   connect( ct_sigma,    SIGNAL( valueChanged( double ) ),
            this,        SLOT(   envvalChange(        ) ) );
   connect( le_plxmin,   SIGNAL( editingFinished( ) ),
            this,        SLOT(   envvalChange(    ) ) );
   connect( le_plxmax,   SIGNAL( editingFinished( ) ),
            this,        SLOT(   envvalChange(    ) ) );

   connect( ck_mdltype,  SIGNAL( stateChanged( int  ) ),
            this,        SLOT(   ltypeChanged(      )  ) );

   connect( lw_runids,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   runid_select(      int ) ) );
   connect( lw_models,   SIGNAL( currentRowChanged( int ) ),
            this,        SLOT(   model_select(      int ) ) );

   QBoxLayout* plot = new US_Plot( data_plot1,
         tr( "Discrete s20,W Distributions" ),
         tr( "Sedimentation Coefficient x 1e+13 (corr. for 20,W)" ),
         tr( "Signal Concentration" ) );

   if ( a_mode.isEmpty() )
     data_plot1->setMinimumSize( 560, 400 );
   else
     data_plot1->setMinimumSize( 400, 400 );
   
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0,  10.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   0.0, 100.0 );
	data_plot1->setCanvasBackground( QBrush(Qt::white) );
   data_grid = us_grid( data_plot1 );
   data_grid->enableXMin( true );
   data_grid->enableYMin( true );
   data_grid->setMajorPen( QPen( US_GuiSettings::plotMajGrid(),
                                 0, Qt::DashLine ) );
   data_grid->setMinorPen( QPen( US_GuiSettings::plotMinGrid(),
                                 0, Qt::DotLine  ) );

   QwtLegend *legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   legend->setFont( sfont );

   if ( a_mode.isEmpty() )
     data_plot1->insertLegend( legend, QwtPlot::BottomLegend  ); 

   rightLayout->addLayout( plot );

   mainLayout ->addLayout( lfullLayout );
   mainLayout ->addLayout( rightLayout );
   mainLayout ->setStretchFactor( lfullLayout, 2 );
   mainLayout ->setStretchFactor( rightLayout, 3 );

   le_runid   ->setText( "(current run ID)" );
   cmb_svproj ->addItem( "(project name for plot save)" );

   adjustSize();
   int hh  = lb_svproj->height();
   int ww  = lb_svproj->width() / 6;
   lw_runids  ->setMinimumHeight( hh * 2 );
   lw_runids  ->setMaximumHeight( hh * 4 );
   lw_models  ->setMinimumHeight( hh * 5 );
   cmb_svproj ->setMinimumWidth ( ww * 2 );
   for ( int ii = 0; ii < 8; ii++ )
   {
      leftLayout ->setColumnMinimumWidth( ii, ww );
      leftLayout ->setColumnStretch     ( ii, 1  );
   }
   te_status  ->setMaximumHeight( ( hh * 3 ) / 2 );

   adjustSize();
   resize( 1180, 580 );
   reset_data();

}

//Load auto | GMP report
QList< QStringList > US_DDistr_Combine::load_auto( QStringList runids_passed, QStringList aDescrs_passed  )
{
  QStringList runids;
  QStringList modelDescModified;
  QStringList modelDescModifiedGuid;
  
  runids. clear();
  aDescrs.clear();

  runids  = runids_passed;
  aDescrs = aDescrs_passed;

  qDebug() << "load_auto in ddist_comb: runids, aDescrs -- " << runids << "\n" << aDescrs;

  //create distros structure
  update_distros();

  runID = runids[ 0 ];
  //model names
  QString grunID = "Global-" + runID;
  for ( int ii = 0; ii < distros.size(); ii++ )  
    {
      DbgLv(1) << "RunIDSel:  ii runID" << ii << distros[ii].runID;

      if ( distros[ ii ].runID == runID  ||
           distros[ ii ].runID.startsWith( grunID ) )
	{  // Only (possibly) add item with matching run ID
	  QString mdesc = distros[ ii ].mdescr;
	  QString ddesc = distros[ ii ].ddescr;
	  QString mguid = distros[ ii ].mGUID;
	  
	  // if ( mfilter )
	  //   {  // If method-filtering, skip any item whose method is not checked
	  //     QString meth = mdesc.section( ".", -1, -1 ).section( "_", -3, -3 );
	  //     DbgLv(1) << "RunIDSel:    meth" << meth;
	  //     if ( ! methods.contains( meth ) )  continue;
	  //   }
	  
	  DbgLv(1) << "RunIDSel:     added: ddesc" << ddesc;
	  //lw_models->addItem( distribID( mdesc, ddesc ) );
	  modelDescModified     << distribID( mdesc, ddesc );
	  modelDescModifiedGuid << mguid;

	  //qDebug() << "load_auto: distribID() -- " << distribID( mdesc, ddesc );
	}
    }

  // //go over modelDescModified
  // for ( int ii = 0; ii < modelDescModified.size(); ii++ )  
  //   {
  //     //here maybe a fiter by type|model
  //     if ( modelDescModified[ ii ].contains("2DSA-IT") ) 
  // 	model_select_auto ( modelDescModified[ ii ] ); 
  //   }

  QList < QStringList > modelsList; 
  modelsList << modelDescModified << modelDescModifiedGuid;
  
  //return modelDescModified;
  return modelsList;
}

//return pointer to data_plot1
QwtPlot* US_DDistr_Combine::rp_data_plot1()
{
  return data_plot1;
}

//reset data_plot1
void US_DDistr_Combine::reset_data_plot1()
{
  xtype   = 0;
  pdistrs .clear();
  pdisIDs .clear(); 
  
  reset_plot();
}

// Load data
void US_DDistr_Combine::load( void )
{
   QStringList runids;
   QString     runid;
   te_status->setText( tr( "Building a list of selectable run IDs..." ) );
   qApp->processEvents();

   // Open a dialog and get the runID(s)
   US_SelectRunDD srdiag( dkdb_cntrls->db(), runids, aDescrs );
   connect( &srdiag,      SIGNAL( changed( bool ) ),
            this,    SLOT( update_disk_db( bool ) ) );
   srdiag.exec();


   qDebug() << "load in ddist_comb: runids, aDescrs -- " << runids << "\n" << aDescrs; 

     
   int nrunids = runids.count();
   int nsprojs = cmb_svproj->count();
   if ( nrunids < 1 )   return;

   te_status->setText( tr( "Updating the distributions list..." ) );
   update_distros();
//*DEBUG*
if(dbg_level>0)
{
 DbgLv(1) << "Selected runIDs[0]" << runids[0] << "count" << nrunids;
 DbgLv(1) << "Selected models  count" << aDescrs.count();
 for(int ii=0;ii<aDescrs.count();ii++)
 {
  QString mrun=aDescrs[ii].section("\t",0,0);
  QString mgid=aDescrs[ii].section("\t",1,1).left(9)+"...";
  QString mdes=aDescrs[ii].section("\t",2,2);
  QString ddes=aDescrs[ii].section("\t",3,3);
  bool iter=mdes.contains("-MC_");
  if(iter&&!mdes.contains("_mcN")) continue;
  mdes="..."+mdes.section(".",1,-1);
  DbgLv(1) << "  ii" << ii << "mrun" << mrun << "mgid" << mgid
   << "mdes" << mdes << "iter" << iter << "ddes" << ddes;
 }
}
//*DEBUG*
   te_status->setText( tr( "Added: %1 run(s), %2 implied total models." )
         .arg( nrunids ).arg( distros.count() ) );

   if ( nsprojs == 1 )
   {
      cmb_svproj->clear();
   }
   else
   {
      cmb_svproj->removeItem( nsprojs - 1 );
   }

   for ( int ii = 0; ii < nrunids; ii++ )
   {  // Add run IDs to list and to project combo box
      lw_runids->addItem( runids[ ii ] );
      cmb_svproj->addItem( runids[ ii ] );
   }

   cmb_svproj->addItem( "All" );
   le_runid->setText( runids[ 0 ] );
   pb_resetd->setEnabled( true );
   QStringList methods;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {  // Build a list of unique methods of distros
      QString method = distros[ ii ].mdescr.section( ".", -1, -1 )
                                           .section( "_", -3, -3 );
DbgLv(1) << "ii method" << ii << method << "mdes" << distros[ii].mdescr;
      if ( ! methods.contains( method ) )
         methods << method;
   }

   bool hv_2dsa     = methods.contains( "2DSA"       );
   bool hv_2dsait   = methods.contains( "2DSA-IT"    );
   bool hv_2dsafm   = methods.contains( "2DSA-FM"    );
   bool hv_2dsamc   = methods.contains( "2DSA-MC"    );
   bool hv_2dsamw   = methods.contains( "2DSA-MW"    );
   bool hv_2dsamcmw = methods.contains( "2DSA-MC-MW" );
   bool hv_2dsagl   = methods.contains( "2DSA-GL"    ) 
                  ||  methods.contains( "2DSA-GL-SG" )
                  ||  methods.contains( "2DSA-GL-SC" )
                  ||  methods.contains( "2DSA-GL-VR" )
                  ||  methods.contains( "2DSA-GL-IT" )
                  ||  methods.contains( "2DSA-GL-SG-IT" )
                  ||  methods.contains( "2DSA-GL-SC-IT" )
                  ||  methods.contains( "2DSA-GL-VR-IT" );
   bool hv_2dsaglmc = methods.contains( "2DSA-GL-MC" )
                  ||  methods.contains( "2DSA-GL-SG-MC" )
                  ||  methods.contains( "2DSA-GL-SC-MC" )
                  ||  methods.contains( "2DSA-GL-VR-MC" );

   bool hv_2dsacg   = methods.contains( "2DSA-CG"    );
   bool hv_2dsacgit = methods.contains( "2DSA-CG-IT" );
   bool hv_2dsacgfm = methods.contains( "2DSA-CG-FM" );
   bool hv_2dsacgmc = methods.contains( "2DSA-CG-MC" );
   bool hv_ga       = methods.contains( "GA"         );
   bool hv_gamc     = methods.contains( "GA-MC"      );
   bool hv_gamw     = methods.contains( "GA-MW"      );
   bool hv_gamcmw   = methods.contains( "GA-MC-MW"   );
   bool hv_gagl     = methods.contains( "GA-GL"      )
                  ||  methods.contains( "GA-GL-SG"   )
                  ||  methods.contains( "GA-GL-SC"   )
                  ||  methods.contains( "GA-GL-VR"   );
   bool hv_gaglmc   = methods.contains( "GA-GL-MC"   )
                  ||  methods.contains( "GA-GL-SG-MC" )
                  ||  methods.contains( "GA-GL-SC-MC" )
                  ||  methods.contains( "GA-GL-VR-MC" );
   bool hv_pcsais   = methods.contains( "PCSA-IS"    )
                  ||  methods.contains( "PCSA-GL-SG-IS" )
                  ||  methods.contains( "PCSA-GL-SC-IS" )
                  ||  methods.contains( "PCSA-GL-VR-IS" );
   bool hv_pcsasl   = methods.contains( "PCSA-SL"    )
                  ||  methods.contains( "PCSA-GL-SG-SL" )
                  ||  methods.contains( "PCSA-GL-SC-SL" )
                  ||  methods.contains( "PCSA-GL-VR-SL" );
   bool hv_pcsads   = methods.contains( "PCSA-DS"    )
                  ||  methods.contains( "PCSA-GL-SG-DS" )
                  ||  methods.contains( "PCSA-GL-SC-DS" )
                  ||  methods.contains( "PCSA-GL-VR-DS" );
   bool hv_pcsahl   = methods.contains( "PCSA-HL"    )
                  ||  methods.contains( "PCSA-GL-SG-HL" )
                  ||  methods.contains( "PCSA-GL-SC-HL" )
                  ||  methods.contains( "PCSA-GL-VR-HL" );
   bool hv_pcsaismc = methods.contains( "PCSA-IS-MC" )
                  ||  methods.contains( "PCSA-GL-SG-IS-MC" )
                  ||  methods.contains( "PCSA-GL-SC-IS-MC" )
                  ||  methods.contains( "PCSA-GL-VR-IS-MC" );
   bool hv_pcsaslmc = methods.contains( "PCSA-SL-MC" )
                  ||  methods.contains( "PCSA-GL-SG-SL-MC" )
                  ||  methods.contains( "PCSA-GL-SC-SL-MC" )
                  ||  methods.contains( "PCSA-GL-VR-SL-MC" );
   bool hv_pcsadsmc = methods.contains( "PCSA-DS-MC" )
                  ||  methods.contains( "PCSA-GL-SG-DS-MC" )
                  ||  methods.contains( "PCSA-GL-SC-DS-MC" )
                  ||  methods.contains( "PCSA-GL-VR-DS-MC" );
   bool hv_pcsahlmc = methods.contains( "PCSA-HL-MC" )
                  ||  methods.contains( "PCSA-GL-SG-HL-MC" )
                  ||  methods.contains( "PCSA-GL-SC-HL-MC" )
                  ||  methods.contains( "PCSA-GL-VR-HL-MC" );
   bool hv_pcsaistr = methods.contains( "PCSA-IS-TR" )
                  ||  methods.contains( "PCSA-GL-SG-IS-TR" )
                  ||  methods.contains( "PCSA-GL-SC-IS-TR" )
                  ||  methods.contains( "PCSA-GL-VR-IS-TR" );
   bool hv_pcsasltr = methods.contains( "PCSA-SL-TR" )
                  ||  methods.contains( "PCSA-GL-SG-SL-TR" )
                  ||  methods.contains( "PCSA-GL-SC-SL-TR" )
                  ||  methods.contains( "PCSA-GL-VR-SL-TR" );
   bool hv_pcsadstr = methods.contains( "PCSA-DS-TR" )
                  ||  methods.contains( "PCSA-GL-SG-DS-TR" )
                  ||  methods.contains( "PCSA-GL-SC-DS-TR" )
                  ||  methods.contains( "PCSA-GL-VR-DS-TR" );
   bool hv_pcsahltr = methods.contains( "PCSA-HL-TR" )
                  ||  methods.contains( "PCSA-GL-SG-HL-TR" )
                  ||  methods.contains( "PCSA-GL-SC-HL-TR" )
                  ||  methods.contains( "PCSA-GL-VR-HL-TR" );
   bool hv_pcsa2o   = methods.contains( "PCSA-2O"    )
                  ||  methods.contains( "PCSA-GL-SG-2O-TR" )
                  ||  methods.contains( "PCSA-GL-SC-2O-TR" )
                  ||  methods.contains( "PCSA-GL-VR-2O-TR" );
   bool hv_pcsa2omc = methods.contains( "PCSA-2O-MC" )
                  ||  methods.contains( "PCSA-GL-SG-2O-MC" )
                  ||  methods.contains( "PCSA-GL-SC-2O-MC" )
                  ||  methods.contains( "PCSA-GL-VR-2O-MC" );
   bool hv_pcsa2otr = methods.contains( "PCSA-2O-TR" )
                  ||  methods.contains( "PCSA-GL-SG-2O-TR" )
                  ||  methods.contains( "PCSA-GL-SC-2O-TR" )
                  ||  methods.contains( "PCSA-GL-VR-2O-TR" );
   bool hv_dmga     = methods.contains( "DMGA"       );
   bool hv_dmgamc   = methods.contains( "DMGA-MC"    );
   bool hv_dmgara   = methods.contains( "DMGA-RA"    );
   bool hv_dmgaramc = methods.contains( "DMGA-RA-MC" );
   bool hv_dmgagl   = methods.contains( "DMGA-GL"    )
                  ||  methods.contains( "DMGA-GL-SG" )
                  ||  methods.contains( "DMGA-GL-SC" )
                  ||  methods.contains( "DMGA-GL-VR" );
   bool hv_dmgaglmc = methods.contains( "DMGA-GL-MC" )
                  ||  methods.contains( "DMGA-GL-SG-MC" )
                  ||  methods.contains( "DMGA-GL-SC-MC" )
                  ||  methods.contains( "DMGA-GL-VR-MC" );
   bool hv_dtall    = methods.size() > 0;

   ck_2dsa    ->setEnabled( hv_2dsa     );
   ck_2dsait  ->setEnabled( hv_2dsait   );
   ck_2dsafm  ->setEnabled( hv_2dsafm   );
   ck_2dsamc  ->setEnabled( hv_2dsamc   );
   ck_2dsamw  ->setEnabled( hv_2dsamw   );
   ck_2dsamcmw->setEnabled( hv_2dsamcmw );
   ck_2dsagl  ->setEnabled( hv_2dsagl   );
   ck_2dsaglmc->setEnabled( hv_2dsaglmc );
   ck_2dsacg  ->setEnabled( hv_2dsacg   );
   ck_2dsacgit->setEnabled( hv_2dsacgit );
   ck_2dsacgfm->setEnabled( hv_2dsacgfm );
   ck_2dsacgmc->setEnabled( hv_2dsacgmc );
   ck_ga      ->setEnabled( hv_ga       );
   ck_gamc    ->setEnabled( hv_gamc     );
   ck_gamw    ->setEnabled( hv_gamw     );
   ck_gamcmw  ->setEnabled( hv_gamcmw   );
   ck_gagl    ->setEnabled( hv_gagl     );
   ck_gaglmc  ->setEnabled( hv_gaglmc   );
   ck_pcsais  ->setEnabled( hv_pcsais   );
   ck_pcsasl  ->setEnabled( hv_pcsasl   );
   ck_pcsads  ->setEnabled( hv_pcsads   );
   ck_pcsahl  ->setEnabled( hv_pcsahl   );
   ck_pcsaismc->setEnabled( hv_pcsaismc );
   ck_pcsaslmc->setEnabled( hv_pcsaslmc );
   ck_pcsadsmc->setEnabled( hv_pcsadsmc );
   ck_pcsahlmc->setEnabled( hv_pcsahlmc );
   ck_pcsaistr->setEnabled( hv_pcsaistr );
   ck_pcsasltr->setEnabled( hv_pcsasltr );
   ck_pcsadstr->setEnabled( hv_pcsadstr );
   ck_pcsahltr->setEnabled( hv_pcsahltr );
   ck_pcsa2o  ->setEnabled( hv_pcsa2o   );
   ck_pcsa2omc->setEnabled( hv_pcsa2omc );
   ck_pcsa2otr->setEnabled( hv_pcsa2otr );
   ck_dmga    ->setEnabled( hv_dmga     );
   ck_dmgamc  ->setEnabled( hv_dmgamc   );
   ck_dmgara  ->setEnabled( hv_dmgara   );
   ck_dmgaramc->setEnabled( hv_dmgaramc );
   ck_dmgagl  ->setEnabled( hv_dmgagl   );
   ck_dmgaglmc->setEnabled( hv_dmgaglmc );
   ck_dtall   ->setEnabled( hv_dtall    );

   ck_2dsa    ->setChecked( hv_2dsa     );
   ck_2dsait  ->setChecked( hv_2dsait   );
   ck_2dsamc  ->setChecked( hv_2dsamc   );
   ck_2dsamw  ->setChecked( hv_2dsamw   );
   ck_2dsamcmw->setChecked( hv_2dsamcmw );
   ck_2dsagl  ->setChecked( hv_2dsagl   );
   ck_2dsaglmc->setChecked( hv_2dsaglmc );
   ck_2dsacg  ->setChecked( hv_2dsacg   );
   ck_2dsacgit->setChecked( hv_2dsacgit );
   ck_2dsacgfm->setChecked( hv_2dsacgfm );
   ck_2dsacgmc->setChecked( hv_2dsacgmc );
   ck_2dsafm  ->setChecked( hv_2dsafm   );
   ck_ga      ->setChecked( hv_ga       );
   ck_gamc    ->setChecked( hv_gamc     );
   ck_gamw    ->setChecked( hv_gamw     );
   ck_gamcmw  ->setChecked( hv_gamcmw   );
   ck_gagl    ->setChecked( hv_gagl     );
   ck_gaglmc  ->setChecked( hv_gaglmc   );
   ck_pcsais  ->setChecked( hv_pcsais   );
   ck_pcsasl  ->setChecked( hv_pcsasl   );
   ck_pcsads  ->setChecked( hv_pcsads   );
   ck_pcsahl  ->setChecked( hv_pcsahl   );
   ck_pcsaismc->setChecked( hv_pcsaismc );
   ck_pcsaslmc->setChecked( hv_pcsaslmc );
   ck_pcsadsmc->setChecked( hv_pcsadsmc );
   ck_pcsahlmc->setChecked( hv_pcsahlmc );
   ck_pcsaistr->setChecked( hv_pcsaistr );
   ck_pcsasltr->setChecked( hv_pcsasltr );
   ck_pcsadstr->setChecked( hv_pcsadstr );
   ck_pcsahltr->setChecked( hv_pcsahltr );
   ck_pcsa2o  ->setChecked( hv_pcsa2o   );
   ck_pcsa2omc->setChecked( hv_pcsa2omc );
   ck_pcsa2otr->setChecked( hv_pcsa2otr );
   ck_dmga    ->setChecked( hv_dmga     );
   ck_dmgamc  ->setChecked( hv_dmgamc   );
   ck_dmgara  ->setChecked( hv_dmgara   );
   ck_dmgaramc->setChecked( hv_dmgaramc );
   ck_dmgagl  ->setChecked( hv_dmgagl   );
   ck_dmgaglmc->setChecked( hv_dmgaglmc );
   ck_dtall   ->setChecked( hv_dtall    );

   // Hide rows that have no enabled check boxes
   int nllcol = leftLayout->columnCount();

   for ( int ii = fckrow; ii < lckrow; ii++ )
   {  // Examine the leftLayout rows with checkboxes for row we can hide
      int nenab  = 0;             // Number of enabled checkboxes in row
      QList< QWidget* > rwidgs;   // List of row widgets (checkboxes)

      for ( int jj = 0; jj < nllcol; jj++ )
      {  // Examine columns in the row for widgets
         QLayoutItem* litem = leftLayout->itemAtPosition( ii, jj );
DbgLv(1) << "LL lay item ii,jj" << ii << jj << "itemP" << litem;

         if ( litem != NULL )
         {  // Check a layout for a widget child
            QGridLayout* layo   = (QGridLayout*)litem->layout();
            QWidget*     widg0  = litem->widget();
            QWidget*     widg1;
DbgLv(1) << " layo" << layo << "widg" << widg0;

            if ( layo != NULL  &&  widg0 == NULL  &&  layo->count()>1 )
            {  // Item is a layout:  check that its child is a widget
DbgLv(1) << "  layo count" << layo->count();
               widg0       = layo->itemAt( 0 )->widget();
               widg1       = layo->itemAt( 1 )->widget();

               if ( widg1 != NULL )
               {  // Save widget and test if it is enabled
                  rwidgs << widg0;                  // Save widgets in list
                  rwidgs << widg1;
                  bool enab   = widg1->isEnabled(); // Test if widget enabled
DbgLv(1) << "    widg" << widg1 << "enab" << enab;
                  if ( enab )  nenab++;             // Bump count of enabled
               }  // END: item child is a widget
            }  // END: item is a layout
         }  // END: non-null item in row
      }  // END: columns-in-row loop

      if ( nenab == 0 )
      {  // If this row had no enabled checkboxes, hide the row
DbgLv(1) << "  HIDE ALL,  row" << ii;
         for ( int jj = 0; jj < rwidgs.count(); jj++ )
         {  // Hide each widget in the row
            rwidgs[ jj ]->setVisible( false );
         }
      }

      else
      {  // If row had enabled checkboxes, show the entire row
         for ( int jj = 0; jj < rwidgs.count(); jj++ )
         {  // Show each widget in the row
            rwidgs[ jj ]->setVisible( true );
         }
      }
   }  // END: checkboxes row loop

   int wx   = x();           // Adjust height based on newly hidden rows
   int wy   = y();
   int wid  = width();
   adjustSize();
   resize( 10, wid );
   adjustSize();
   move( wx, wy );
   qApp->processEvents();
}

// Reset data: remove all loaded data and clear plots
void US_DDistr_Combine::reset_data( void )
{
   distros.clear();
   distIDs.clear();
   aDescrs.clear();

   lw_runids  ->clear();
   lw_models  ->clear();
   le_runid   ->clear();
   cmb_svproj ->clear();

   reset_plot();

   pb_resetd->setEnabled( false );
   pb_resetp->setEnabled( false );
DbgLv(1) << "main size" << size();
}

// Reset plot:  Clear plots and lists of plotted data
void US_DDistr_Combine::reset_plot( void )
{
	data_plot1->setCanvasBackground( QBrush(Qt::white) );
   dataPlotClear( data_plot1 );
   data_plot1->replot();
   pdistrs.clear();
   pdisIDs.clear();
   pb_saveda->setEnabled( false );

   lw_models  ->setCurrentRow( -1 );
   le_plxmin->disconnect();
   le_plxmax->disconnect();
   QString smin = "0";
   QString smax = "0";
   le_plxmin->setText( smin );
   le_plxmax->setText( smax );
   connect( le_plxmin,   SIGNAL( editingFinished( ) ),
      this, SLOT( envvalChange() ) );
   connect( le_plxmax,   SIGNAL( editingFinished( ) ),
      this, SLOT( envvalChange() ) );

}

// Plot all data: for GMP Report
void US_DDistr_Combine::plot_data_auto( QMap < QString, QString> c_parms )
{
  
DbgLv(1) << "pDa:  xtype" << xtype;
   dataPlotClear( data_plot1 );
	data_plot1->setCanvasBackground( QBrush(Qt::white) );
   QString titleY = tr( "Signal Concentration" );
DbgLv(1) << "pDa:  titleY" << titleY;
   QString titleP;
   QString titleX;

   if      ( xtype == 0 )
   {
      titleP = tr( "Discrete s20,W Distributions" );
      titleX = tr( "Sedimentation Coefficient x 1.e+13 (20,W)" );
   }
   else if ( xtype == 1 )
   {
      titleP = tr( "Discrete Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Dalton)" );
   }
   else if ( xtype == 2 )
   {
      titleP = tr( "Discrete D20,W Distributions" );
      titleX = tr( "Diffusion Coefficient (20,W)" );
   }
   else if ( xtype == 3 )
   {
      titleP = tr( "Discrete Frictional Ratio Distributions" );
      titleX = tr( "Frictional Ratio (f/f0)" );
   }
   else if ( xtype == 4 )
   {
      titleP = tr( "Discrete Vbar Distributions" );
      titleX = tr( "Partial Specific Volume (vbar)" );
   }
   else if ( xtype == 5 )
   {
      titleP = tr( "Discrete Log of Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Log, Dalton)" );
   }
DbgLv(1) << "pDa:  titleP" << titleP;
DbgLv(1) << "pDa:  titleX" << titleX;
   data_plot1->setTitle    ( titleP );
   data_plot1->setAxisTitle( QwtPlot::xBottom, titleX );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   titleY );
   double plxmin = 1e+30;
   double plxmax = -1e+30;

   QStringList legend_texts;
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
      setColor( pdistrs[ ii ], ii );
      plot_distr_auto ( pdistrs[ ii ], pdisIDs[ ii ], c_parms );

      legend_texts << pdisIDs[ ii ];
   }

   //qDebug() << "LEGEND TEXT -- " <<  legend_texts;
   //data_plot1->legend()->setVisible( false ); // <-- TEMP!!!!!
   
   //    for ( int jj = 0; jj < pdistrs[ ii ].xvals.size(); jj++ )
   //    {
   //       plxmin        = qMin( plxmin, pdistrs[ ii ].xvals[ jj ] );
   //       plxmax        = qMax( plxmax, pdistrs[ ii ].xvals[ jj ] );
   //    }
   // }
   // le_plxmin->disconnect();
   // le_plxmax->disconnect();
   // le_plxmin->setText( QString::number( plxmin ) );
   // le_plxmax->setText( QString::number( plxmax ) );
   // connect( le_plxmin,   SIGNAL( editingFinished( ) ),
   //    this, SLOT(envvalChange( ) ) );
   // connect( le_plxmax,   SIGNAL( editingFinished( ) ),
   //    this, SLOT(envvalChange( ) ) );
}

// Plot all data
void US_DDistr_Combine::plot_data( void )
{
DbgLv(1) << "pDa:  xtype" << xtype;
   dataPlotClear( data_plot1 );
	data_plot1->setCanvasBackground( QBrush(Qt::white) );
   QString titleY = tr( "Signal Concentration" );
DbgLv(1) << "pDa:  titleY" << titleY;
   QString titleP;
   QString titleX;

   if      ( rb_pltsw->isChecked() )
   {
      titleP = tr( "Discrete s20,W Distributions" );
      titleX = tr( "Sedimentation Coefficient x 1.e+13 (20,W)" );
   }
   else if ( rb_pltMW->isChecked() )
   {
      titleP = tr( "Discrete Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Dalton)" );
   }
   else if ( rb_pltDw->isChecked() )
   {
      titleP = tr( "Discrete D20,W Distributions" );
      titleX = tr( "Diffusion Coefficient (20,W)" );
   }
   else if ( rb_pltff0->isChecked() )
   {
      titleP = tr( "Discrete Frictional Ratio Distributions" );
      titleX = tr( "Frictional Ratio (f/f0)" );
   }
   else if ( rb_pltvb->isChecked() )
   {
      titleP = tr( "Discrete Vbar Distributions" );
      titleX = tr( "Partial Specific Volume (vbar)" );
   }
   else if ( rb_pltMWl->isChecked() )
   {
      titleP = tr( "Discrete Log of Molecular Weight Distributions" );
      titleX = tr( "Molecular Weight (Log, Dalton)" );
   }
DbgLv(1) << "pDa:  titleP" << titleP;
DbgLv(1) << "pDa:  titleX" << titleX;
   data_plot1->setTitle    ( titleP );
   data_plot1->setAxisTitle( QwtPlot::xBottom, titleX );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   titleY );
   double plxmin = 1e+30;
   double plxmax = -1e+30;

   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
      setColor( pdistrs[ ii ], ii );
      plot_distr( pdistrs[ ii ], pdisIDs[ ii ] );

      for ( int jj = 0; jj < pdistrs[ ii ].xvals.size(); jj++ )
      {
         plxmin        = qMin( plxmin, pdistrs[ ii ].xvals[ jj ] );
         plxmax        = qMax( plxmax, pdistrs[ ii ].xvals[ jj ] );
      }
   }
   le_plxmin->disconnect();
   le_plxmax->disconnect();
   le_plxmin->setText( QString::number( plxmin ) );
   le_plxmax->setText( QString::number( plxmax ) );
   connect( le_plxmin,   SIGNAL( editingFinished( ) ),
      this, SLOT(envvalChange( ) ) );
   connect( le_plxmax,   SIGNAL( editingFinished( ) ),
      this, SLOT(envvalChange( ) ) );
}

// Add a single distribution to the plot
void US_DDistr_Combine::plot_distr( DistrDesc ddesc, QString distrID )
{
   int  ndispt = ddesc.xvals.size();
   double* xx  = ddesc.xvals.data();
   double* yy  = ddesc.yvals.data();
   QVector< double > xenv;
   QVector< double > yenv;
   QString str;
   double minx=1e99, maxx=-1e99;
DbgLv(1) << "pDi:  ndispt" << ndispt << "ID" << distrID.left(20);

   QwtPlotCurve* data_curv = us_curve( data_plot1, distrID );

   if ( ct_sigma->value() > 0.0 )
   {
      data_curv->setPen  ( QPen( QBrush( ddesc.color ), 3.0, Qt::SolidLine ) );
      data_curv->setStyle( QwtPlotCurve::Lines );
      QwtSymbol* symbol = new QwtSymbol();
      data_curv->setSymbol(symbol);

      ndispt    = envel_data( ddesc.xvals, ddesc.yvals, xenv, yenv );

      xx        = xenv.data();
      yy        = yenv.data();
      for (int i=0; i<xenv.size(); i++)
      {
         minx = qMin(minx, xenv[i]);
         maxx = qMax(maxx, xenv[i]);
      }
   }
   else
   {
      data_curv->setPen  ( QPen( QBrush( ddesc.color ), 3.0, Qt::SolidLine ) );
      data_curv->setStyle( QwtPlotCurve::Sticks );
      for (int i=0; i<ddesc.xvals.size(); i++)
      {
         minx = qMin(minx, ddesc.xvals[i]);
         maxx = qMax(maxx, ddesc.xvals[i]);
      }
   }

   data_curv->setSamples( xx, yy, ndispt );
   data_curv->setItemAttribute( QwtPlotItem::Legend, true );
   if (le_plxmin->text().toDouble() < minx) 
   {
      minx = le_plxmin->text().toDouble();
   }
   if (le_plxmax->text().toDouble() > maxx) 
   {
      maxx = le_plxmax->text().toDouble();
   }
   le_plxmin->disconnect();
   le_plxmax->disconnect();
   le_plxmin->setText(str.setNum(minx));
   le_plxmax->setText(str.setNum(maxx));
   connect( le_plxmin,   SIGNAL( editingFinished( ) ),
      this, SLOT(envvalChange() ) );
   connect( le_plxmax,   SIGNAL( editingFinished( ) ),
      this, SLOT(envvalChange() ) );
   data_plot1->setAxisScale( QwtPlot::xBottom, minx, maxx );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->enableAxis      ( QwtPlot::xBottom, true );
   data_plot1->enableAxis      ( QwtPlot::yLeft,   true );
	data_plot1->setCanvasBackground( QBrush(Qt::white) );
   data_plot1->replot();

}

// Add a single distribution to the plot: copy for GMP Report
void US_DDistr_Combine::plot_distr_auto( DistrDesc ddesc, QString distrID, QMap< QString, QString > c_parms )
{
  //Retrive comboPlot parameters (for s20):
  double sigma_p = 0.01;
  double xmin_p  = 1;
  double xmax_p  = 10;
  QString ranges_p;
  
  qDebug() << "c_ranges, begin -- " <<  c_parms[ "Ranges" ];
  qDebug() << "c_parms.keys() -- " << c_parms.keys();
  
  QMap<QString, QString >::iterator jj;
  for ( jj = c_parms.begin(); jj != c_parms.end(); ++jj )
    {
      if ( jj.key().contains( "Gaussian" ) )
	sigma_p = jj.value().toDouble();
      else if ( jj.key().contains( "Minimum" )  )
	xmin_p = jj.value().toDouble();
      else if ( jj.key().contains( "Maximum" ) )
	xmax_p = jj.value().toDouble();
      else if ( jj.key().contains( "Ranges" ) )
	ranges_p = jj.value();
      else if ( jj.key().contains( "s_ranges" ) )
	{
	  xmin_p = jj.value().split(",")[0].toDouble();
	  xmax_p = jj.value().split(",")[1].toDouble();
	}
      else if ( jj.key().contains( "k_ranges" ) )
	{
	  xmin_p = jj.value().split(",")[0].toDouble();
	  xmax_p = jj.value().split(",")[1].toDouble();
	}
    }

  qDebug() << "In plot_distr_auto(): sigma_p, xmin_p, xmax_p, ranges_p -- "
	   << sigma_p << xmin_p << xmax_p << ranges_p;
    
  //////////////////////////////////////////
  
   int  ndispt = ddesc.xvals.size();
   double* xx  = ddesc.xvals.data();
   double* yy  = ddesc.yvals.data();
   QVector< double > xenv;
   QVector< double > yenv;
   QString str;
   double minx=1e99, maxx=-1e99;
DbgLv(1) << "pDi:  ndispt" << ndispt << "ID" << distrID.left(20);

   QwtPlotCurve* data_curv = us_curve( data_plot1, distrID );

   if ( sigma_p > 0.0 )
   {
      data_curv->setPen  ( QPen( QBrush( ddesc.color ), 3.0, Qt::SolidLine ) );
      data_curv->setStyle( QwtPlotCurve::Lines );

      ndispt    = envel_data_auto ( ddesc.xvals, ddesc.yvals, xenv, yenv, sigma_p, xmin_p, xmax_p );

      xx        = xenv.data();
      yy        = yenv.data();
      for (int i=0; i<xenv.size(); i++)
      {
         minx = qMin(minx, xenv[i]);
         maxx = qMax(maxx, xenv[i]);
      }
   }
   else
   {
      data_curv->setPen  ( QPen( QBrush( ddesc.color ), 3.0, Qt::SolidLine ) );
      data_curv->setStyle( QwtPlotCurve::Sticks );
      for (int i=0; i<ddesc.xvals.size(); i++)
      {
         minx = qMin(minx, ddesc.xvals[i]);
         maxx = qMax(maxx, ddesc.xvals[i]);
      }
   }

   data_curv->setSamples( xx, yy, ndispt );
   data_curv->setItemAttribute( QwtPlotItem::Legend, true ); 

   
   
   // if (le_plxmin->text().toDouble() < minx) 
   // {
   //    minx = le_plxmin->text().toDouble();
   // }
   // if (le_plxmax->text().toDouble() > maxx) 
   // {
   //    maxx = le_plxmax->text().toDouble();
   // }
   // le_plxmin->disconnect();
   // le_plxmax->disconnect();
   // le_plxmin->setText(str.setNum(minx));
   // le_plxmax->setText(str.setNum(maxx));
   // connect( le_plxmin,   SIGNAL( editingFinished( ) ),
   //    this, SLOT(envvalChange() ) );
   // connect( le_plxmax,   SIGNAL( editingFinished( ) ),
   //    this, SLOT(envvalChange() ) );


   data_plot1->setAxisScale( QwtPlot::xBottom, minx, maxx );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->enableAxis      ( QwtPlot::xBottom, true );
   data_plot1->enableAxis      ( QwtPlot::yLeft,   true );
   data_plot1->setCanvasBackground( QBrush(Qt::white) );

   //add ranges ///////////////////////////////////////////////////////////
   //Also, include info on vertical lines (integration limits) -- passed also from c_parms[ "integration_limits" ]

   if ( !ranges_p.isEmpty() )
     {
       QStringList c_ranges = ranges_p.split(",");
       qDebug() << "c_ranges, c_ranges.size() --" << c_ranges << c_ranges.size();
       for (int i=0; i<c_ranges.size(); ++i )
	 {
	   double point1 = c_ranges[ i ].split(":")[0].toDouble();
	   double point2 = c_ranges[ i ].split(":")[1].toDouble();
	   
	   qDebug() << "Ranges: " << point1 << point2;
	   
	   QwtPlotCurve* v_line_peak1;
	   double r1[ 2 ];
	   r1[ 0 ] = point1;
	   r1[ 1 ] = point1;
	   
	   QwtPlotCurve* v_line_peak2;
	   double r2[ 2 ];
	   r2[ 0 ] = point2;
	   r2[ 1 ] = point2;
	   
#if QT_VERSION < 0x050000
	   QwtScaleDiv* y_axis = data_plot1->axisScaleDiv( QwtPlot::yLeft );
#else
	   QwtScaleDiv* y_axis = (QwtScaleDiv*)&(data_plot1->axisScaleDiv( QwtPlot::yLeft ));
#endif
	   
	   double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;
	   
	   double v[ 2 ];
	   v [ 0 ] = y_axis->upperBound() - padding;
	   v [ 1 ] = y_axis->lowerBound();// + padding;
	   
	   QString vline_name1 = "Low"  + QString::number( i );
	   QString vline_name2 = "High" + QString::number( i );
	   
	   v_line_peak1 = us_curve( data_plot1, vline_name1 );
	   v_line_peak1 ->setSamples( r1, v, 2 );
	   
	   v_line_peak2 = us_curve( data_plot1, vline_name2 );
	   v_line_peak2 ->setSamples( r2, v, 2 );

	   //set unique color for current range pair
	   possibleColors();              // Make sure possible colors exist

	   int ncolors     = colors.size();
	   int color_index = i;
	   
	   while ( color_index >= ncolors )
	     color_index -= ncolors;
	   
	   QPen pen = QPen( QBrush( colors[ color_index ] ), 2.0, Qt::DotLine );
	   v_line_peak1->setPen( pen );
	   v_line_peak2->setPen( pen );
	 }
     }
   
   data_plot1->replot();

}


// Save the plot data
void US_DDistr_Combine::save( void )
{
   QString oproj    = cmb_svproj->currentText();
   QString runID    = ( oproj == "All" ) ? pdistrs[ 0 ].runID : oproj;
   QString fdir     = US_Settings::reportDir() + "/" + runID;
   QString mdescr   = pdistrs[ 0 ].mdescr;
   QString annode   = mdescr.section( ".", -1, -1 ).section( "_", -3, -3 );
   QString trnode   = "0Z9999";
   QString sanode1  = "combo-distrib-s";
   QString sanode2  = "combo-vcdat-s";
   QString sanode3  = "combo-listincl-s";

   if ( xtype == 1 )
   {
      sanode1       = "combo-distrib-mw";
      sanode2       = "combo-vcdat-mw";
      sanode3       = "combo-listincl-mw";
   }
   else if ( xtype == 2 )
   {
      sanode1       = "combo-distrib-d";
      sanode2       = "combo-vcdat-d";
      sanode3       = "combo-listincl-d";
   }
   else if ( xtype == 3 )
   {
      sanode1       = "combo-distrib-ff0";
      sanode2       = "combo-vcdat-ff0";
      sanode3       = "combo-listincl-ff0";
   }
   else if ( xtype == 4 )
   {
      sanode1       = "combo-distrib-vbar";
      sanode2       = "combo-vcdat-vbar";
      sanode3       = "combo-listincl-vbar";
   }
   else if ( xtype == 5 )
   {
      sanode1       = "combo-distrib-mwl";
      sanode2       = "combo-vcdat-mwl";
      sanode3       = "combo-listincl-mwl";
   }
   QString fnamsvg  = annode + "." + trnode + "." + sanode1 + ".svgz"; 
   QString fnampng  = annode + "." + trnode + "." + sanode1 + ".png"; 
   //QString fnamdat  = annode + "." + trnode + "." + sanode2 + ".dat"; 
   QString fnamdat  = annode + "." + trnode + "." + sanode2 + ".csv"; 
   QString fnamlst  = annode + "." + trnode + "." + sanode3 + ".rpt"; 
   QString plotFile = fdir + "/" + fnamsvg;
   QString dataFile = fdir + "/" + fnamdat;
   QString listFile = fdir + "/" + fnamlst;
   QStringList prunids;
   QList< int > prndxs;
   QString svmsg   = tr( "Saved:\n    " ) + fnampng + "\n    "
                                          + fnamsvg + "\n    "
                                          + fnamdat + "\n    "
                                          + fnamlst + "\n";
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Look for multiple run IDs
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
      QString prun     = pdistrs[ ii ].runID;
      if ( oproj == "All" )
      {  // If save-plot project is "All", save lists of runids and indexes
         if ( ! prunids.contains( prun ) )
         {
            prunids << prun;
            prndxs  << ii;
         }
      }

      else if ( prun == runID )
      {  // If save-plot project matches current run, save it and its index
         prunids << prun;
         prndxs  << ii;
         break;
      }
   }

   int     iruns   = 0;
   int     nruns   = prunids.size();

   while( iruns < nruns )
   {
      if ( ! QFile( fdir ).exists() )
      {  // If need be, create runID directory
         QDir().mkpath( fdir );
      }

      // Save plot file as SVG and as PNG; write data and list files
      write_plot( plotFile, data_plot1 );
      write_data( dataFile, listFile, iruns );
      svmsg += tr( "in directory:" ) + "\n    " + fdir + "\n";

      if ( dkdb_cntrls->db() )
      {
         US_Passwd    pw;
         US_DB2       db( pw.getPasswd() );
         int          idEdit = 0;
         int          kl     = pdistrs.size() - 1;

         QString trfirst  = pdistrs[ 0  ].mdescr.section( ".", -2, -2 );
         QString trlast   = pdistrs[ kl ].mdescr.section( ".", -2, -2 );
         QString trdesc   = "Combined Analyses (" + trfirst
            + "..." + trlast + ")";

         QString editID;         // Edit ID for matching experiment,triple
         QString eeditID;        // First edit ID from experiment match
         // Get test triple to match file part and investigator
         QString trip1 = "";
         QString invID = QString::number( US_Settings::us_inv_ID() );
         // Query for the experiment ID matching the run ID
         QStringList  query;
         query << "get_experiment_info_by_runID" << runID << invID;
         db.query( query );
         db.next();
         QString expID = db.value( 1 ).toString();
DbgLv(1) << "SV:  runID expID" << runID << expID;
         // Query for the raw ID in experiment matching a triple
         QString rawID;
         query.clear();
         query << "get_rawDataIDs" << expID;
         db.query( query );
         while ( db.next() )
         {
                    rawID  = db.value( 0 ).toString();
            QString efname = db.value( 2 ).toString();
DbgLv(1) << "SV:   rawID" << rawID << "efname" << efname << "trip1" << trip1;
            // Save rawID when we have found a triple match
            if ( efname.contains( trip1 ) )
               break;
         }
         // Query edit IDs for raw ID and look for triple match
         query.clear();
         query << "get_editedDataIDs" << rawID;
         db.query( query );
         while ( db.next() )
         {
            QString aeditID = db.value( 0 ).toString();
            QString efname  = db.value( 2 ).toString();
            if ( eeditID.isEmpty() )
               eeditID      = aeditID;  // Save 1st valid from experiment
DbgLv(1) << "SV:     editID" << eeditID << "raw exp trip1 fname"
 << rawID << expID << trip1 << efname;
            if ( efname.contains( trip1 ) )
            {  // Keep saving editID from last triple match
               editID       = aeditID;
DbgLv(1) << "SV:        Name-Trip MATCH:  editID" << editID;
            }
         }

         if ( ! editID.isEmpty() )
         {  // Use edit ID from last matching triple
            idEdit              = editID.toInt();
         }
         else
         {  // Or fall back to one from first valid edit in experiment
            idEdit              = eeditID.toInt();
         }
DbgLv(1) << "SV: editID idEdit" << editID << idEdit << "  eeditID" << eeditID;

         // Add or update report documents in the database
         QStringList rfiles;
         rfiles << fnamsvg << fnampng << fnamdat << fnamlst;
         int st = reportDocsFromFiles( runID, fdir, rfiles, &db,
                                       idEdit, trdesc );

DbgLv(1) << "SV:runID" << runID << "idEdit" << idEdit
 << "fnamlst" << fnamlst << "trdesc" << trdesc;
         if ( iruns == ( nruns - 1 ) )
         {  // Append message line after last run save
            if ( st == 0 )
               svmsg += tr( "\nThe files were also saved to the database" );
            else
               svmsg += tr( "\n*ERROR* in saving files to the database" );
         }
      }  // END:  database

      if ( ++iruns >= nruns )  break;

      runID         = prunids[ iruns ];
      fdir          = US_Settings::reportDir() + "/" + runID;
      plotFile      = fdir + "/" + fnamsvg;
      dataFile      = fdir + "/" + fnamdat;
      listFile      = fdir + "/" + fnamlst;
   }  // END:  runs loop

   QApplication::restoreOverrideCursor();

   // Report saved files
   QMessageBox::information( this, tr( "Combo Distro Plot File Save" ), svmsg );
}

// RunID selected
void US_DDistr_Combine::runid_select( int row )
{
DbgLv(1) << "RunIDSel:row" << row;
   if ( row < 0 )  return;

   // Get selected run item and its ID
   QListWidgetItem* item = lw_runids->item( row );
   runID    = item->text();
DbgLv(1) << "RunIDSel:runID" << runID << "distrsize" << distros.size();
   le_runid ->setText( runID );

   // Check for filtering by method
   bool mfilter = ! ck_dtall->isChecked();
   QStringList methods;

   if ( mfilter )
   {
      if ( ck_2dsa    ->isChecked() )  methods << "2DSA";
      if ( ck_2dsait  ->isChecked() )  methods << "2DSA-IT";
      if ( ck_2dsamc  ->isChecked() )  methods << "2DSA-MC";
      if ( ck_2dsamw  ->isChecked() )  methods << "2DSA-MW";
      if ( ck_2dsamcmw->isChecked() )  methods << "2DSA-MC-MW";
      if ( ck_2dsagl  ->isChecked() )  methods << "2DSA-GL"
                                               << "2DSA-GL-SG"
                                               << "2DSA-GL-SC"
                                               << "2DSA-GL-VR"
                                               << "2DSA-GL-IT"
                                               << "2DSA-GL-SG-IT"
                                               << "2DSA-GL-SC-IT"
                                               << "2DSA-GL-VR-IT";
      if ( ck_2dsaglmc->isChecked() )  methods << "2DSA-GL-MC"
                                               << "2DSA-GL-SG-MC"
                                               << "2DSA-GL-SC-MC"
                                               << "2DSA-GL-VR-MC";
      if ( ck_2dsacg  ->isChecked() )  methods << "2DSA-CG";
      if ( ck_2dsacgit->isChecked() )  methods << "2DSA-CG-IT";
      if ( ck_2dsacgfm->isChecked() )  methods << "2DSA-CG-FM";
      if ( ck_2dsacgmc->isChecked() )  methods << "2DSA-CG-MC";
      if ( ck_2dsafm  ->isChecked() )  methods << "2DSA-FM";
      if ( ck_ga      ->isChecked() )  methods << "GA";
      if ( ck_gamc    ->isChecked() )  methods << "GA-MC";
      if ( ck_gamw    ->isChecked() )  methods << "GA-MW";
      if ( ck_gamcmw  ->isChecked() )  methods << "GA-MC-MW";
      if ( ck_gagl    ->isChecked() )  methods << "GA-GL"
                                               << "GA-GL-SG"
                                               << "GA-GL-SC"
                                               << "GA-GL-VR";
      if ( ck_gaglmc  ->isChecked() )  methods << "GA-GL-MC"
                                               << "GA-GL-SG-MC"
                                               << "GA-GL-SC-MC"
                                               << "GA-GL-VR-MC";
      if ( ck_pcsais  ->isChecked() )  methods << "PCSA-IS"
                                               << "PCSA-GL-SG-IS"
                                               << "PCSA-GL-SC-IS"
                                               << "PCSA-GL-VR-IS";
      if ( ck_pcsasl  ->isChecked() )  methods << "PCSA-SL"
                                               << "PCSA-GL-SG-SL"
                                               << "PCSA-GL-SC-SL"
                                               << "PCSA-GL-VR-SL";
      if ( ck_pcsads  ->isChecked() )  methods << "PCSA-DS"
                                               << "PCSA-GL-SG-DS"
                                               << "PCSA-GL-SC-DS"
                                               << "PCSA-GL-VR-DS";
      if ( ck_pcsahl  ->isChecked() )  methods << "PCSA-HL"
                                               << "PCSA-GL-SG-HL"
                                               << "PCSA-GL-SC-HL"
                                               << "PCSA-GL-VR-HL";
      if ( ck_pcsaismc->isChecked() )  methods << "PCSA-IS-MC"
                                               << "PCSA-GL-SG-IS-MC"
                                               << "PCSA-GL-SC-IS-MC"
                                               << "PCSA-GL-VR-IS-MC";
      if ( ck_pcsaslmc->isChecked() )  methods << "PCSA-SL-MC"
                                               << "PCSA-GL-SG-SL-MC"
                                               << "PCSA-GL-SC-SL-MC"
                                               << "PCSA-GL-VR-SL-MC";
      if ( ck_pcsadsmc->isChecked() )  methods << "PCSA-DS-MC"
                                               << "PCSA-GL-SG-DS-MC"
                                               << "PCSA-GL-SC-DS-MC"
                                               << "PCSA-GL-VR-DS-MC";
      if ( ck_pcsahlmc->isChecked() )  methods << "PCSA-HL-MC"
                                               << "PCSA-GL-SG-HL-MC"
                                               << "PCSA-GL-SC-HL-MC"
                                               << "PCSA-GL-VR-HL-MC";
      if ( ck_pcsaistr->isChecked() )  methods << "PCSA-IS-TR"
                                               << "PCSA-GL-SG-IS-TR"
                                               << "PCSA-GL-SC-IS-TR"
                                               << "PCSA-GL-VR-IS-TR";
      if ( ck_pcsasltr->isChecked() )  methods << "PCSA-SL-TR"
                                               << "PCSA-GL-SG-IS-TR"
                                               << "PCSA-GL-SC-IS-TR"
                                               << "PCSA-GL-VR-IS-TR";
      if ( ck_pcsadstr->isChecked() )  methods << "PCSA-DS-TR"
                                               << "PCSA-GL-SG-IS-TR"
                                               << "PCSA-GL-SC-IS-TR"
                                               << "PCSA-GL-VR-IS-TR";
      if ( ck_pcsahltr->isChecked() )  methods << "PCSA-HL-TR"
                                               << "PCSA-GL-SG-IS-TR"
                                               << "PCSA-GL-SC-IS-TR"
                                               << "PCSA-GL-VR-IS-TR";
      if ( ck_pcsa2o  ->isChecked() )  methods << "PCSA-2O"
                                               << "PCSA-GL-SG-2O"
                                               << "PCSA-GL-SC-2O"
                                               << "PCSA-GL-VR-2O";
      if ( ck_pcsa2omc->isChecked() )  methods << "PCSA-2O-MC"
                                               << "PCSA-GL-SG-2O-MC"
                                               << "PCSA-GL-SC-2O-MC"
                                               << "PCSA-GL-VR-2O-MC";
      if ( ck_pcsa2otr->isChecked() )  methods << "PCSA-2O-TR"
                                               << "PCSA-GL-SG-2O-TR"
                                               << "PCSA-GL-SC-2O-TR"
                                               << "PCSA-GL-VR-2O-TR";
      if ( ck_dmga    ->isChecked() )  methods << "DMGA";
      if ( ck_dmgamc  ->isChecked() )  methods << "DMGA-MC";
      if ( ck_dmgara  ->isChecked() )  methods << "DMGA-RA";
      if ( ck_dmgaramc->isChecked() )  methods << "DMGA-RA-MC";
      if ( ck_dmgagl  ->isChecked() )  methods << "DMGA-GL";
      if ( ck_dmgaglmc->isChecked() )  methods << "DMGA-GL-MC";
   }

   lw_models ->clear();
   QString grunID = "Global-" + runID;

   for ( int ii = 0; ii < distros.size(); ii++ )
   {
DbgLv(1) << "RunIDSel:  ii runID" << ii << distros[ii].runID;
      if ( distros[ ii ].runID == runID  ||
           distros[ ii ].runID.startsWith( grunID ) )
      {  // Only (possibly) add item with matching run ID
         QString mdesc = distros[ ii ].mdescr;
         QString ddesc = distros[ ii ].ddescr;

         if ( mfilter )
         {  // If method-filtering, skip any item whose method is not checked
            QString meth = mdesc.section( ".", -1, -1 ).section( "_", -3, -3 );
DbgLv(1) << "RunIDSel:    meth" << meth;
            if ( ! methods.contains( meth ) )  continue;
         }

DbgLv(1) << "RunIDSel:     added: ddesc" << ddesc;
         lw_models->addItem( distribID( mdesc, ddesc ) );
      }
   }

   if ( pdistrs.size() == 0 )
   {
      cmb_svproj->setCurrentIndex( cmb_svproj->findText( runID ) );
   }
}

// Model distribution selected -- FOR GMP reporter | 6. REPORT
QMap< QStringList, QList< QColor> > US_DDistr_Combine::model_select_auto( QString modelNameMod, QMap< QString, QString > c_parms )
{
   QString          distrID = modelNameMod;
   int              mdx     = distro_by_descr( distrID );
   DbgLv(1) << "ModelSel_auto: model" << distrID << "mdx" << mdx;
   DistrDesc*       ddesc   = &distros[ mdx ];

   QList< QColor >  pdistrs_colors;
   QMap < QStringList, QList< QColor > > s_type_map; 
      
   if ( ! pdisIDs.contains( distrID ) )
   {  // If this distro not yet filled out, do so now

      fill_in_desc( distros[ mdx ], pdistrs.size() );

      pdistrs << *ddesc;     // Add to list of plotted distros
      pdisIDs << distrID;    // Add to list of IDs of plotted distros

      //pdistrs_colors <<  (*ddesc).  color; 
   }

   if ( ddesc->model.components.size() == 0 )
   {
      QMessageBox::critical( this, tr( "Zero-Components Model" ),
            tr( "*ERROR* The selected model has zero components.\n"
                "This selection is ignored" ) );
      //return pdisIDs;
      return  s_type_map;
   }

 
   plot_data_auto( c_parms );

   pb_saveda->setEnabled( true );
   pb_resetd->setEnabled( true );
   pb_resetp->setEnabled( true );

   te_status->setText( tr( "Count of plotted distributions: %1." )
         .arg( pdistrs.count() ) );

   //
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
     pdistrs_colors << pdistrs[ ii ]. color; 
   }

   s_type_map [ pdisIDs ] = pdistrs_colors;
   
   return s_type_map;
   //return pdisIDs;
   //ALEXEY: also need to return color corresponsing to each plotted curve: (QColor) pdistrs[ ii ].color
   //Possibly, return a structure {QstringList pdisIDS, QList<QColor>  pdistrs[ ii ].color }
}

// Model distribution selected
void US_DDistr_Combine::model_select( int row )
{
DbgLv(1) << "ModelSel:row" << row;
   if ( row < 0 )  return;
   QListWidgetItem* item    = lw_models ->item( row );
   QString          distrID = item->text();
   int              mdx     = distro_by_descr( distrID );
DbgLv(1) << "ModelSel: model" << distrID << "mdx" << mdx;
   DistrDesc*       ddesc   = &distros[ mdx ];

   if ( ! pdisIDs.contains( distrID ) )
   {  // If this distro not yet filled out, do so now

      fill_in_desc( distros[ mdx ], pdistrs.size() );

      pdistrs << *ddesc;     // Add to list of plotted distros
      pdisIDs << distrID;    // Add to list of IDs of plotted distros
   }

   if ( ddesc->model.components.size() == 0 )
   {
      QMessageBox::critical( this, tr( "Zero-Components Model" ),
            tr( "*ERROR* The selected model has zero components.\n"
                "This selection is ignored" ) );
      return;
   }

   plot_data();

   pb_saveda->setEnabled( true );
   pb_resetd->setEnabled( true );
   pb_resetp->setEnabled( true );

   te_status->setText( tr( "Count of plotted distributions: %1." )
         .arg( pdistrs.count() ) );
}

// Assign color for a distribution
void US_DDistr_Combine::setColor( DistrDesc& ddesc, int distx )
{
   possibleColors();              // Make sure possible colors exist

   int ncolors     = colors.size();
   int color_index = distx;

   while ( color_index >= ncolors )
      color_index -= ncolors;

DbgLv(1) << "sC:  color_index" << color_index;
   ddesc.color = colors[ color_index ];
   return;
}

// Generate list of colors if need be
void US_DDistr_Combine::possibleColors()
{
   if ( colors.size() > 0 )
      return;

   colors  << QColor( 255,   0,   0 );
   colors  << QColor(   0, 255,   0 );
   colors  << QColor(   0,   0, 255 );
   colors  << QColor(   0,   0,   0 );
   colors  << QColor( 255,   0, 255 );
   colors  << QColor(   0, 255, 255 );
   colors  << QColor( 122,   0, 255 );
   colors  << QColor(   0, 255, 122 );
   colors  << QColor(   0, 122, 255 );
   colors  << QColor( 255, 122,   0 );
   colors  << QColor( 122, 255,   0 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(  80,   0, 255 );
   colors  << QColor( 255,   0,  80 );
   colors  << QColor(   0, 255,  80 );
   colors  << QColor(   0,  80, 255 );
   colors  << QColor(  80, 255,   0 );
   colors  << QColor( 255,  80,  40 );
   colors  << QColor(  40, 255,  40 );
   colors  << QColor(  40,  40, 255 );
   return;
}

// Return a distribution ID string: shortened model or cell description
QString US_DDistr_Combine::distribID( QString mdescr, QString ddescr )
{
   const int mxrch = 30;
   int mdx         = 9999;
   QString distrID;
   QString runID   = mdescr.section( ".",  0, -3 );
           runID   = runID.length() <= mxrch ?
                     runID : runID.left( mxrch ) + "(++)";
   QString triple  = mdescr.section( ".", -2, -2 );
   QString iterID  = mdescr.section( ".", -1, -1 );
   QString andate  = iterID.section( "_",  1,  1 );
   QString method  = iterID.section( "_",  2,  2 );
           iterID  = ( method != "2DSA-FM" )
                   ? iterID.section( "_", -2, -2 )
                   : iterID.section( "_", -1, -1 );

   if ( ck_mdltype->isChecked() )
   {  // Model-description list/legend type
      distrID      = runID + "." + triple + "." + andate + "_" + method
                     + "_" + iterID;
   }
   else
   {  // Data-description list/legend type  (the default)
      distrID      = runID + " (" + triple + ", " + method + ") " + ddescr;
   }

   for ( int ii = 0; ii < distros.size(); ii++ )
   {  // Find index of full model description match as distinguishing suffix
      if ( mdescr == distros[ ii ].mdescr )
      {
         mdx          = ii + 1;
         break;
      }
   }

   distrID     += "[" + QString::number( mdx ) + "]";

   return distrID;
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_DDistr_Combine::update_disk_db( bool isDB )
{ isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
DbgLv(1) << "Upd_Dk_Db isDB" << isDB;

   reset_data();
}

// Fill in a distribution description object with model and values
void US_DDistr_Combine::fill_in_desc( DistrDesc& ddesc, int distx )
{
   if ( ddesc.xvals.size() > 0  &&
        ddesc.model.components.size() > 0  &&
        ddesc.xtype == xtype )
      return;

   // The distribution record is at least partially uninitialized
   QVector< double > mxvals;
   QVector< double > myvals;
   QString mdescr = ddesc.mdescr;
   ddesc.iters    = mdescr.contains( "-MC_" ) ? 1 : 0;
DbgLv(1) << "FID: mdescr" << mdescr << "iters" << ddesc.iters;
   bool    isDB   = dkdb_cntrls->db();
   US_Passwd pw;
   US_DB2* db     = isDB ? new US_DB2( pw.getPasswd() ) : 0;
   int     ncomps = ddesc.model.components.size();
DbgLv(1) << "FID:  ncomps" << ncomps;

   // Read in the (first) model
   if ( ncomps == 0 )
   {  // Model not yet loaded, so load it now
      ddesc.model.load( isDB, ddesc.mGUID, db );

      ncomps         = ddesc.model.components.size();   // Composite components
DbgLv(1) << "FID:  (2)ncomps" << ncomps;
   }
DbgLv(1) << "FID:  (3)ncomps" << ncomps;

   // Build the X,Y vectors with values at every component point
   for ( int jj = 0; jj < ncomps; jj++ )
   {
      myvals << ddesc.model.components[ jj ].signal_concentration;
      double                  xval = ddesc.model.components[ jj ].s * 1.e+13;
      if ( xtype == 1 )       xval = ddesc.model.components[ jj ].mw;
      else if ( xtype == 2 )  xval = ddesc.model.components[ jj ].D;
      else if ( xtype == 3 )  xval = ddesc.model.components[ jj ].f_f0;
      else if ( xtype == 4 )  xval = ddesc.model.components[ jj ].vbar20;
      else if ( xtype == 5 )  xval = log( ddesc.model.components[ jj ].mw );

      mxvals << xval;
   }

   ddesc.xtype       = xtype;
   if ( ncomps == 0 )
      return;
DbgLv(1) << "FID:   xtype" << xtype << "mxval.size" << mxvals.size();
   ddesc.xvals.fill( 0.0, ncomps );
   ddesc.yvals.fill( 0.0, ncomps );
   int    kk         = 0;
   int    nn         = 1;
   double xval       = mxvals[ 0 ];
   double yval       = myvals[ 0 ];
   ddesc.xvals[ 0 ]  = xval;
   ddesc.yvals[ 0 ]  = yval;

   // Sum y values where x values are effectively equivalent
   for ( int jj = 1; jj < ncomps; jj++ )
   {
      double xvpr       = xval;
      double yvpr       = yval;
      xval              = mxvals[ jj ];
      yval              = myvals[ jj ];

      if ( equivalent( xval, xvpr, epsilon ) )
      {  // Effectively equal x values:  sum y values
         yval               += yvpr;
         ddesc.xvals[ kk ]   = ( xvpr + xval ) * 0.5;
         ddesc.yvals[ kk ]   = yval;
      }

      else
      {  // New x value:  save y value and bump count
         kk                  = nn;
         ddesc.xvals[ nn   ] = xval;
         ddesc.yvals[ nn++ ] = yval;
      }
   }

   kk                = 0;

   // Compress the vectors down to only non-zero-Y points
   for ( int jj = 0; jj < nn; jj++ )
   {
      if ( ddesc.yvals[ jj ] != 0.0 )
      {  // Move point to next non-zero output location
         ddesc.xvals[ kk   ] = ddesc.xvals[ jj ];
         ddesc.yvals[ kk++ ] = ddesc.yvals[ jj ];
      }
   }

   ddesc.xvals.resize( kk );     // Resize to just the non-zero points
   ddesc.yvals.resize( kk );

   setColor( ddesc, distx );
}

// Write data and list report files
void US_DDistr_Combine::write_data( QString& dataFile, QString& listFile,
      int& irun )
{
   int arrsize      = 300;

   if ( irun > 0 )
   {  // After first/only time:  just make a copy of the files
      QFile( dat1File ).copy( dataFile );
      QFile( lis1File ).copy( listFile );
      return;
   }

   // First/only time through:  compute the data and create files
   QStringList pdlong;
   QString line;
   dat1File = dataFile;
   lis1File = listFile;

   QFile dfile( dataFile );

   if ( ! dfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << dataFile;
      return;
   }

   QTextStream tsd( &dfile );

   QVector< QVector< double > > peyvals;
   QVector< double >  xenvs;
   QVector< double >  yenvs;
   QVector< double >* xvals;
   QVector< double >* yvals;
   int nplots       = pdistrs.size();
   int lplot        = nplots - 1;
   int maxnvl       = 0;
   int nenvvl       = arrsize;
   line             = "";

   // Build header lines and accumulate envelope Y vectors for each plot

   for ( int ii = 0; ii < nplots; ii++ )
   {
      // Accumulate long descriptions and build header line
      maxnvl           = qMax( maxnvl, pdistrs[ ii ].xvals.size() );
      QString mdescr   = pdistrs[ ii ].mdescr;
      QString mdtrip   = mdescr.section( ".", -2, -2 );
      QString mditer   = mdescr.section( ".", -1, -1 );
      QString mdmeth   = mditer.section( "_",  2,  2 );
      QString pd       = "\"" + mdtrip + "." + mdmeth + ".";

      pdlong << mdescr;

DbgLv(1) << "WrDa:  plot" << ii << "pd" << pd;
      // X,Y header entries for contributor
      line            += pd + "raw-x\","    + pd + "raw-y\",";

      // Compute envelope vectors and save the Y vector for each plot
      envel_data( pdistrs[ ii ].xvals, pdistrs[ ii ].yvals, xenvs, yenvs );

      peyvals << yenvs;
   }

   // Add the single smooth X header string
   line            += "\"all.smooth-x\",";

   for ( int ii = 0; ii < nplots; ii++ )
   {  // Add smooth Y header strings
      QString mdescr   = pdistrs[ ii ].mdescr;
      QString mdtrip   = mdescr.section( ".", -2, -2 );
      QString mditer   = mdescr.section( ".", -1, -1 );
      QString mdmeth   = mditer.section( "_",  2,  2 );
      QString pd       = "\"" + mdtrip + "." + mdmeth + ".";
      line            += pd + "smooth-y\"";
      line            += ( ( ii < lplot ) ? "," : "\n" );
   }

   tsd << line;                             // Write header line

DbgLv(1) << "WrDa: maxnvl" << maxnvl << "nplots" << nplots;
   char  valfm1[] = "\"%.5f\",\"%.5f\"";
   char  valfm2[] = "\"%.4e\",\"%.5f\"";
   char  valfm3[] = "\"%.4e\",\"%.4e\"";
   char  valfx1[] = "\"%.5f\"";
   char  valfx2[] = "\"%.4e\"";
   char  valfx3[] = "\"%.4e\"";
   char  valfy1[] = "\"%.5f\"";
   char  valfy2[] = "\"%.5f\"";
   char  valfy3[] = "\"%.4e\"";
   QString dummy_pair( "\"\",\"\"," );
   QString dummy_valu( "\"\"" );
   char* valfmt   = valfm1;
   char* valfmx   = valfx1;
   char* valfmy   = valfy1;
   maxnvl         = qMax( maxnvl, nenvvl );
   if ( xtype == 1  ||  xtype == 2  || xtype == 5 )
   {
      valfmt         = valfm2;        // Formatting for "MW"/"D"/"MWlog"
      valfmx         = valfx2;
      valfmy         = valfy2;
   }

   // Test whether X or Y are below 1.0
   double xvmin      = 1e99;
   double yvmin      = 1e99;
   for ( int jj = 0; jj < maxnvl; jj++ )
   {
      for ( int ii = 0; ii < nplots; ii++ )
      {
         xvals       = &pdistrs[ ii ].xvals;
         yvals       = &pdistrs[ ii ].yvals;

         if ( jj < xvals->size() )
         {
            xvmin       = qMin( xvmin, xvals->at( jj ) );
            yvmin       = qMin( yvmin, yvals->at( jj ) );
         }
      }
   }

   if ( xvmin < 0.1  ||  yvmin < 0.1 )
   {
      valfmt         = valfm3;        // Formatting for values below 1.0
      valfmx         = valfx3;
      valfmy         = valfy3;
   }

   for ( int jj = 0; jj < maxnvl; jj++ )
   {  // Build and write xvalue+concentration data line
      line       = "";

      // First add X,Y for raw plots
      for ( int ii = 0; ii < nplots; ii++ )
      {  // Add each pair of X,Y data pairs
         xvals       = &pdistrs[ ii ].xvals;
         yvals       = &pdistrs[ ii ].yvals;

         // Get and add raw data to line
         if ( jj < xvals->size() )
            line       += QString().sprintf(
                             valfmt, xvals->at( jj ), yvals->at( jj ) ) + ",";
         else
            line       += dummy_pair;

         // Get and add envelope (smoothed) data to line
      }

      // Now add X for envelopes and the Y's for each plot
      if ( jj < nenvvl )
         line       += QString().sprintf( valfmx, xenvs[ jj ] ) + ",";
      else
         line       += dummy_valu + ",";

      for ( int ii = 0; ii < nplots; ii++ )
      {
         if ( jj < nenvvl )
            line       += QString().sprintf( valfmy, peyvals[ ii ][ jj ] );
         else
            line       += dummy_valu;

         line       += ( ii < lplot ) ? "," : "\n";
      }

      tsd << line;                           // Write data line
//DbgLv(1) << "WrDa:   jj" << jj << " line written";
   }

   dfile.close();

   // Write list-of-included file
   QFile lfile( listFile );
   if ( ! lfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      qDebug() << "***Error opening output file" << listFile;
      return;
   }
   QTextStream tsl( &lfile );

   for ( int ii = 0; ii < nplots; ii++ )
   {  // Build and write each long-description line
      line       = pdlong[ ii ] + "\n";
      tsl << line;
   }

   lfile.close();

   return;
}

// Save report documents from files
int US_DDistr_Combine::reportDocsFromFiles( QString& runID, QString& fdir,
   QStringList& files, US_DB2* db, int& idEdit, QString& trdesc )
{
 DbgLv(1) << "rDFF: runID fdir files0" << runID << fdir << files[0];
 DbgLv(1) << "rDFF:  idEdit trdesc" << idEdit << trdesc;
   int ostat      = 0;
   US_Report    freport;
   freport.runID  = runID;

   for ( int ii = 0; ii < files.size(); ii++ )
   {
      QString fname = files[ ii ];
      int st = freport.saveDocumentFromFile( fdir, fname, db, idEdit, trdesc );

      ostat = ( st == US_Report::REPORT_OK ) ? ostat : st;
   }

//*DEBUG*
   if ( dbg_level > 0 )
   {
      int status = freport.readDB( runID, db );
      DbgLv(1) << "DFF:report readDB status" << status << "ID" << freport.ID;
      DbgLv(1) << "DFF:  report triples size" << freport.triples.size();
      for ( int ii = 0; ii < freport.triples.size(); ii++ )
      {
         int ndoc = freport.triples[ii].docs.size();
         DbgLv(1) << "DFF:  triple" << ii << "docssize" << ndoc
            << "ID" << freport.triples[ii].tripleID
            << "triple" << freport.triples[ii].triple;
         int jj   = ndoc - 1;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ii].docs[0].documentID
               << "label" << freport.triples[ii].docs[0].label;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ii].docs[jj].documentID
               << "label" << freport.triples[ii].docs[jj].label;
         }
      }
      QString fname = files[0];
      QString tripl( "0/Z/9999" );
      int ndx = freport.findTriple( tripl );
      DbgLv(1) << "DFF:triple" << tripl << "ndx" << ndx;
      if ( ndx >= 0 )
      {
         int ndoc = freport.triples[ndx].docs.size();
         DbgLv(1) << "DFF:  triple" << ndx << "docs size" << ndoc
            << "ID" << freport.triples[ndx].tripleID
            << "triple" << freport.triples[ndx].triple;
         if ( ndoc > 0 )
         {
            DbgLv(1) << "DFF:    doc" << 0
               << "ID" << freport.triples[ndx].docs[0].documentID
               << "label" << freport.triples[ndx].docs[0].label;
            int jj   = ndoc - 1;
            DbgLv(1) << "DFF:    doc" << jj
               << "ID" << freport.triples[ndx].docs[jj].documentID
               << "label" << freport.triples[ndx].docs[jj].label;
         }
      }
   }
//*DEBUG*

   return ostat;
}

// Get the index to a distro using the model description
int US_DDistr_Combine::distro_by_descr( QString& mdesc )
{
   int index = -1;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {
      if ( mdesc
            == distribID( distros[ ii ].mdescr, distros[ ii ].ddescr ) )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Get the index to a distro using the model GUID
int US_DDistr_Combine::distro_by_mguid( QString& mguid )
{
   int index = -1;

   for ( int ii = 0; ii < distros.count(); ii++ )
   {
      if ( mguid == distros[ ii ].mGUID )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Get the next index to a distro that has a matching run ID
int US_DDistr_Combine::distro_by_runid( QString& runid, int first )
{
   int index = -1;

   for ( int ii = first; ii < distros.count(); ii++ )
   {
      if ( runid == distros[ ii ].runID )
      {
         index  = ii;
         break;
      }
   }

   return index;
}

// Update distributions list objects from new run models
void US_DDistr_Combine::update_distros()
{
   distros.clear();

   for ( int ii = 0; ii < aDescrs.count(); ii++ )
   {
      QString mrun = aDescrs[ii].section( "\t", 0, 0 );
      QString mgid = aDescrs[ii].section( "\t", 1, 1 );
      QString mdes = aDescrs[ii].section( "\t", 2, 2 );
      QString ddes = aDescrs[ii].section( "\t", 3, 3 );

      DistrDesc dd;
      dd.runID     = mrun;
      dd.mGUID     = mgid;
      dd.mdescr    = mdes;
      dd.ddescr    = ddes;
      dd.iters     = mdes.contains( "-MC_" ) ? 1 : 0;
      if ( dd.iters != 0  &&  ! mdes.contains( "_mcN" ) )  continue;
      if ( distro_by_mguid( mgid ) >= 0 )                  continue;

      dd.xvals.clear();
      dd.yvals.clear();

      distros << dd;
   }

   qSort( distros );

   return;
}

// Update Distributions list when a method check box is changed
void US_DDistr_Combine::methodChanged( int state )
{
   if ( state == Qt::Unchecked )
      ck_dtall->setChecked( false );

   list_distributions();
}

// Update Distributions list when the All method check box is changed
void US_DDistr_Combine::allMethodChanged( int state )
{
   if ( state == Qt::Checked )
   {
      ck_2dsa    ->setChecked( ck_2dsa    ->isEnabled() );
      ck_2dsait  ->setChecked( ck_2dsait  ->isEnabled() );
      ck_2dsafm  ->setChecked( ck_2dsafm  ->isEnabled() );
      ck_2dsamc  ->setChecked( ck_2dsamc  ->isEnabled() );
      ck_2dsamw  ->setChecked( ck_2dsamw  ->isEnabled() );
      ck_2dsamcmw->setChecked( ck_2dsamcmw->isEnabled() );
      ck_2dsagl  ->setChecked( ck_2dsagl  ->isEnabled() );
      ck_2dsaglmc->setChecked( ck_2dsaglmc->isEnabled() );
      ck_2dsacg  ->setChecked( ck_2dsacg  ->isEnabled() );
      ck_2dsacgit->setChecked( ck_2dsacgit->isEnabled() );
      ck_2dsacgfm->setChecked( ck_2dsacgfm->isEnabled() );
      ck_2dsacgmc->setChecked( ck_2dsacgmc->isEnabled() );
      ck_ga      ->setChecked( ck_ga      ->isEnabled() );
      ck_gamc    ->setChecked( ck_gamc    ->isEnabled() );
      ck_gamw    ->setChecked( ck_gamw    ->isEnabled() );
      ck_gamcmw  ->setChecked( ck_gamcmw  ->isEnabled() );
      ck_gagl    ->setChecked( ck_gagl    ->isEnabled() );
      ck_gaglmc  ->setChecked( ck_gaglmc  ->isEnabled() );
      ck_pcsais  ->setChecked( ck_pcsais  ->isEnabled() );
      ck_pcsasl  ->setChecked( ck_pcsasl  ->isEnabled() );
      ck_pcsads  ->setChecked( ck_pcsads  ->isEnabled() );
      ck_pcsahl  ->setChecked( ck_pcsahl  ->isEnabled() );
      ck_pcsaismc->setChecked( ck_pcsaismc->isEnabled() );
      ck_pcsaslmc->setChecked( ck_pcsaslmc->isEnabled() );
      ck_pcsadsmc->setChecked( ck_pcsadsmc->isEnabled() );
      ck_pcsahlmc->setChecked( ck_pcsahlmc->isEnabled() );
      ck_pcsaistr->setChecked( ck_pcsaistr->isEnabled() );
      ck_pcsasltr->setChecked( ck_pcsasltr->isEnabled() );
      ck_pcsadstr->setChecked( ck_pcsadstr->isEnabled() );
      ck_pcsahltr->setChecked( ck_pcsahltr->isEnabled() );
      ck_pcsa2o  ->setChecked( ck_pcsa2o  ->isEnabled() );
      ck_pcsa2omc->setChecked( ck_pcsa2omc->isEnabled() );
      ck_pcsa2otr->setChecked( ck_pcsa2otr->isEnabled() );
      ck_dmga    ->setChecked( ck_dmga    ->isEnabled() );
      ck_dmgamc  ->setChecked( ck_dmgamc  ->isEnabled() );
      ck_dmgara  ->setChecked( ck_dmgara  ->isEnabled() );
      ck_dmgaramc->setChecked( ck_dmgaramc->isEnabled() );
      ck_dmgagl  ->setChecked( ck_dmgagl  ->isEnabled() );
      ck_dmgaglmc->setChecked( ck_dmgaglmc->isEnabled() );
   }

   list_distributions();
}

// Change the contents of the distributions list based on method filtering
void US_DDistr_Combine::list_distributions()
{
   runid_select( lw_runids->currentRow() );
}

// React to a change in the X type of plots
void US_DDistr_Combine::changedPlotX( bool on_state )
{
   if ( ! on_state )  return;

DbgLv(1) << "changedPlotX" << on_state;
   bool x_is_sw    = rb_pltsw ->isChecked();
   bool x_is_MW    = rb_pltMW ->isChecked();
   bool x_is_Dw    = rb_pltDw ->isChecked();
   bool x_is_ff0   = rb_pltff0->isChecked();
   bool x_is_vb    = rb_pltvb ->isChecked();
   bool x_is_MWl   = rb_pltMWl->isChecked();
        xtype      = 0;

   if ( x_is_sw )
   {
DbgLv(1) << "  PX=Sed.Coeff";
      xtype           = 0;
   }

   else if ( x_is_MW )
   {
DbgLv(1) << "  PX=Molec.Wt.";
      xtype           = 1;
   }

   else if ( x_is_Dw )
   {
DbgLv(1) << "  PX=Diff.Coeff";
      xtype           = 2;
   }

   else if ( x_is_ff0 )
   {
DbgLv(1) << "  PX=f/f0";
      xtype           = 3;
   }

   else if ( x_is_vb )
   {
DbgLv(1) << "  PX=Vbar";
      xtype           = 4;
   }

   else if ( x_is_MWl )
   {
DbgLv(1) << "  PX=Molec.Wt.log";
      xtype           = 5;
   }

   int npdis    = pdistrs.size();
   if ( npdis > 0 )
   {  // Re-do plot distros to account for X-type change
      QList< DistrDesc >  wdistros;
      DistrDesc           ddist;
      DistrDesc*          pddist;

      for ( int ii = 0; ii < npdis; ii++ )
      {  // Build rudimentary plot distros without value arrays
         pddist       = &pdistrs[ ii ];
         ddist.runID  = pddist->runID;
         ddist.mGUID  = pddist->mGUID;
         ddist.mdescr = pddist->mdescr;
         ddist.iters  = pddist->iters;
         ddist.xtype  = xtype;
         ddist.model  = pddist->model;
         ddist.ddescr = pddist->ddescr;

         wdistros << ddist;
      }

      pdistrs.clear();
      pdisIDs.clear();

      for ( int ii = 0; ii < npdis; ii++ )
      {
         pddist       = &wdistros[ ii ];
         fill_in_desc( *pddist, ii );

         pdistrs << *pddist;
         pdisIDs << distribID( pddist->mdescr, pddist->ddescr );
      }

      le_plxmin->disconnect();
      le_plxmax->disconnect();
      le_plxmin->setText( "0" );
      le_plxmax->setText( "0" );
      connect( le_plxmin,   SIGNAL( editingFinished( ) ),
               this,        SLOT(   envvalChange(    ) ) );
      connect( le_plxmax,   SIGNAL( editingFinished( ) ),
               this,        SLOT(   envvalChange(    ) ) );

      plot_data();
   }
}

// React to a change in the X type of plots
QMap< QStringList, QList< QColor> > US_DDistr_Combine::changedPlotX_auto( int type, QMap< QString, QString > c_parms )
{
   xtype      = type;

   QList< QColor >  pdistrs_colors;
   QMap < QStringList, QList< QColor > > other_type_map; 
   
   int npdis    = pdistrs.size();
   if ( npdis > 0 )
   {  // Re-do plot distros to account for X-type change
      QList< DistrDesc >  wdistros;
      DistrDesc           ddist;
      DistrDesc*          pddist;

      for ( int ii = 0; ii < npdis; ii++ )
      {  // Build rudimentary plot distros without value arrays
         pddist       = &pdistrs[ ii ];
         ddist.runID  = pddist->runID;
         ddist.mGUID  = pddist->mGUID;
         ddist.mdescr = pddist->mdescr;
         ddist.iters  = pddist->iters;
         ddist.xtype  = xtype;
         ddist.model  = pddist->model;
         ddist.ddescr = pddist->ddescr;

         wdistros << ddist;
      }

      pdistrs.clear();
      pdisIDs.clear();

      for ( int ii = 0; ii < npdis; ii++ )
      {
         pddist       = &wdistros[ ii ];
         fill_in_desc( *pddist, ii );

         pdistrs << *pddist;
         pdisIDs << distribID( pddist->mdescr, pddist->ddescr );
      }

      le_plxmin->disconnect();
      le_plxmax->disconnect();
      le_plxmin->setText( "0" );
      le_plxmax->setText( "0" );
      connect( le_plxmin,   SIGNAL( editingFinished( ) ),
               this,        SLOT(   envvalChange(    ) ) );
      connect( le_plxmax,   SIGNAL( editingFinished( ) ),
               this,        SLOT(   envvalChange(    ) ) );

      plot_data_auto( c_parms );
   }

   //
   for ( int ii = 0; ii < pdistrs.size(); ii++ )
   {
     pdistrs_colors << pdistrs[ ii ]. color; 
   }

   other_type_map [ pdisIDs ] = pdistrs_colors;
   
   return other_type_map;
   
   //return pdisIDs;
   //ALEXEY: also need to return color corresponsing to each plotted curve: (QColor) pdistrs[ ii ].color
   //Possibly, return a structure {QstringList pdisIDS, QList<QColor>  pdistrs[ ii ].color }
}

// Change the contents of the distributions list based on list type change
void US_DDistr_Combine::ltypeChanged()
{
   if ( lw_runids->count() > 0  &&  lw_models->count() > 0 )
   {
      list_distributions();

      //reset_plot();
      plot_data();
   }
}

// Determine if two values are functionally equivalent within a given epsilon
bool US_DDistr_Combine::equivalent( double aa, double bb, double eps )
{
   double dd       = ( aa != 0.0 ) ? qAbs( aa )
                     : ( bb != 0.0 ? qAbs( bb ) : 1.0 );
   return ( ( qAbs( aa - bb ) / dd ) <= eps );
}

// Generate envelope data
int US_DDistr_Combine::envel_data( 
      QVector< double >& xvals, QVector< double >& yvals,
      QVector< double >& xenvs, QVector< double >& yenvs )
{
   int     arrsize  = 300;
   int     vCount   = xvals.size();
   double  min_xval = 1.0e+50;
   double  max_xval = -min_xval;
   double  con_sum  = 0.0;
   double  env_sum  = 0.0;
   double* xv       = xvals.data();
   double* yv       = yvals.data();

   for ( int jj = 0; jj < vCount; jj++ )
   {  // Get min,max of x values (e.g., sedimentation coefficients)
      max_xval         = qMax( max_xval, xv[ jj ] );
      min_xval         = qMin( min_xval, xv[ jj ] );
      con_sum         += yv[ jj ];        // Accumulate total concentration
   }

   // Calculate values based on range
   bool min_neg     = ( min_xval < 0.0 );
   double rng_xval  = max_xval - min_xval;
   double xval_pad  = rng_xval * 0.1;
   min_xval         = min_xval - xval_pad;
   min_xval         = min_neg ? min_xval : qMax( 0.0, min_xval );
   //max_xval         = min_xval + rng_xval;
   max_xval         = max_xval + xval_pad;
   double minx      = le_plxmin->text().toDouble();
   double maxx      = le_plxmax->text().toDouble();
   min_xval         = ( minx != 0.0 ) ? minx : min_xval;
   max_xval         = ( maxx != 0.0 ) ? maxx : max_xval;
   rng_xval         = max_xval - min_xval;

   // Initialize envelope arrays
   xenvs.fill( 0.0, arrsize );
   yenvs.fill( 0.0, arrsize );
   double* xe       = xenvs.data();
   double* ye       = yenvs.data();
   double  xinc     = rng_xval / (double)( arrsize - 1 );
DbgLv(1) << "ED:  rng_xval arrsize xinc" << rng_xval << arrsize << xinc;

   for ( int jj = 0; jj < arrsize; jj++ )
   {  // Initialize envelope values
      xe[ jj ]         = min_xval + xinc * (double)( jj );
      ye[ jj ]         = 0.0;
   }

   // Populate envelope Ys with gaussian sums
   double pisqr     = sqrt( M_PI * 2.0 );
   double sigma     = ct_sigma->value();
   sigma            = qMax( 0.0001, sigma );
   double xterm     = 1.0 / ( sigma * rng_xval );
   double zterm     = 1.0 / ( sigma * pisqr );

   for ( int kk = 0; kk < arrsize; kk++ )
   {  // Loop to compute envelope grid Y values
      double xval_env  = xe[ kk ];                 // Envelope X value
      double yval_env  = 0.0;                      // Initial envelope Y value

      for ( int jj = 0; jj < vCount; jj++ )
      {  // Accumulate Gaussian Y's from each solute
         double xval_sol  = xv[ jj ];              // Solute X value
         double yval_sol  = yv[ jj ];              // Solute Y value

         double xdiff     = sq( ( xval_sol - xval_env ) * xterm );
         double yfac      = exp( -0.5 * xdiff ) * zterm;
         yval_env        += ( yfac * yval_sol );   // Sum envelope Y value
      }

      ye[ kk ]         = yval_env;                 // Store envelope Y value
      env_sum         += ye[ kk ];                 // Build envelope sum
   }

   double scale     = con_sum / env_sum;    // Normalizing scale factor
DbgLv(1) << "ED: csum esum scale " << con_sum << env_sum << scale;

env_sum=0.0;
double yemx=-1e+30;
double xemx=-1e+30;
   for ( int kk = 0; kk < arrsize; kk++ )
   {  // Normalize Y values so integral is equal to total concentration
      ye[ kk ]        *= scale;
if(ye[kk]>yemx) { yemx=ye[kk]; xemx=xe[kk]; }
env_sum+=ye[kk];
   }
DbgLv(1) << "ED: Final esum" << env_sum << "csum" << con_sum
 << "xemx yemx" << xemx << yemx
 << "sigma  vcount" << sigma << vCount;

   return arrsize;                          // Return size of arrays
}

// Generate envelope data: copy for GMP Report
int US_DDistr_Combine::envel_data_auto( 
				       QVector< double >& xvals, QVector< double >& yvals,
				       QVector< double >& xenvs, QVector< double >& yenvs,
				       double sigma_passed, double xmin_passed, double xmax_passed )
{
   int     arrsize  = 300;
   int     vCount   = xvals.size();
   double  min_xval = 1.0e+50;
   double  max_xval = -min_xval;
   double  con_sum  = 0.0;
   double  env_sum  = 0.0;
   double* xv       = xvals.data();
   double* yv       = yvals.data();

   for ( int jj = 0; jj < vCount; jj++ )
   {  // Get min,max of x values (e.g., sedimentation coefficients)
      max_xval         = qMax( max_xval, xv[ jj ] );
      min_xval         = qMin( min_xval, xv[ jj ] );
      con_sum         += yv[ jj ];        // Accumulate total concentration
   }

   // Calculate values based on range
   bool min_neg     = ( min_xval < 0.0 );
   double rng_xval  = max_xval - min_xval;
   double xval_pad  = rng_xval * 0.1;
   min_xval         = min_xval - xval_pad;
   min_xval         = min_neg ? min_xval : qMax( 0.0, min_xval );
   //max_xval         = min_xval + rng_xval;
   max_xval         = max_xval + xval_pad;
   double minx      = le_plxmin->text().toDouble();
   double maxx      = le_plxmax->text().toDouble();
   min_xval         = ( minx != 0.0 ) ? minx : min_xval;
   max_xval         = ( maxx != 0.0 ) ? maxx : max_xval;
   rng_xval         = max_xval - min_xval;

   //Use passed xmin/xmax values:           //ALEXEY <----------- here, use pre-defined parms!!!
   min_xval = xmin_passed;
   max_xval = xmax_passed;
   rng_xval = max_xval - min_xval;

   // Initialize envelope arrays
   xenvs.fill( 0.0, arrsize );
   yenvs.fill( 0.0, arrsize );
   double* xe       = xenvs.data();
   double* ye       = yenvs.data();
   double  xinc     = rng_xval / (double)( arrsize - 1 );
DbgLv(1) << "ED:  rng_xval arrsize xinc" << rng_xval << arrsize << xinc;

   for ( int jj = 0; jj < arrsize; jj++ )
   {  // Initialize envelope values
      xe[ jj ]         = min_xval + xinc * (double)( jj );
      ye[ jj ]         = 0.0;
   }

   // Populate envelope Ys with gaussian sums
   double pisqr     = sqrt( M_PI * 2.0 );

   //double sigma     = ct_sigma->value();
   double sigma     = sigma_passed;           //ALEXEY <----------- here, use pre-defined parms!!!
   
   sigma            = qMax( 0.0001, sigma );
   double xterm     = 1.0 / ( sigma * rng_xval );
   double zterm     = 1.0 / ( sigma * pisqr );

   for ( int kk = 0; kk < arrsize; kk++ )
   {  // Loop to compute envelope grid Y values
      double xval_env  = xe[ kk ];                 // Envelope X value
      double yval_env  = 0.0;                      // Initial envelope Y value

      for ( int jj = 0; jj < vCount; jj++ )
      {  // Accumulate Gaussian Y's from each solute
         double xval_sol  = xv[ jj ];              // Solute X value
         double yval_sol  = yv[ jj ];              // Solute Y value

         double xdiff     = sq( ( xval_sol - xval_env ) * xterm );
         double yfac      = exp( -0.5 * xdiff ) * zterm;
         yval_env        += ( yfac * yval_sol );   // Sum envelope Y value
      }

      ye[ kk ]         = yval_env;                 // Store envelope Y value
      env_sum         += ye[ kk ];                 // Build envelope sum
   }

   double scale     = con_sum / env_sum;    // Normalizing scale factor
DbgLv(1) << "ED: csum esum scale " << con_sum << env_sum << scale;

env_sum=0.0;
double yemx=-1e+30;
double xemx=-1e+30;
   for ( int kk = 0; kk < arrsize; kk++ )
   {  // Normalize Y values so integral is equal to total concentration
      ye[ kk ]        *= scale;
if(ye[kk]>yemx) { yemx=ye[kk]; xemx=xe[kk]; }
env_sum+=ye[kk];
   }
DbgLv(1) << "ED: Final esum" << env_sum << "csum" << con_sum
 << "xemx yemx" << xemx << yemx
 << "sigma  vcount" << sigma << vCount;

   return arrsize;                          // Return size of arrays
}

// Slot for change in sigma value or plot min,max
void US_DDistr_Combine::envvalChange( )
{
   QString pmintxt  = le_plxmin->text();
   QString pmaxtxt  = le_plxmax->text();

   // Plot data with changed sigma
   plot_data();

   le_plxmin->disconnect();
   le_plxmax->disconnect();
   le_plxmin->setText( pmintxt );
   le_plxmax->setText( pmaxtxt );
   connect( le_plxmin,   SIGNAL( editingFinished( ) ),
            this,        SLOT(   envvalChange(    ) ) );
   connect( le_plxmax,   SIGNAL( editingFinished( ) ),
            this,        SLOT(   envvalChange(    ) ) );
}

