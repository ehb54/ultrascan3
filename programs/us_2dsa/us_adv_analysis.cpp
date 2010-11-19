//! \file us_adv_analysis.cpp

#include "us_2dsa.h"
#include "us_adv_analysis.h"
#include "us_settings.h"
#include "us_gui_settings.h"

#include <qwt_legend.h>

// constructor:  enhanced plot control widget
US_AdvAnalysis::US_AdvAnalysis( US_SimulationParameters* sim_par,
    QWidget* p ) : US_WidgetsDialog( p, 0 )
{
   sparms         = sim_par;
   parentw        = p;

   setObjectName( "US_AdvAnalysis" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "2DSA Advanced Controls" ) );

   mainLayout      = new QHBoxLayout( this );
   optimizeLayout  = new QGridLayout( );
   simparmsLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   mainLayout->addLayout( optimizeLayout  );
   mainLayout->addLayout( simparmsLayout );
   mainLayout->setStretchFactor( optimizeLayout, 1 );
   mainLayout->setStretchFactor( simparmsLayout, 1 );

   QLabel* lb_simpars      = us_banner( tr( "Simulation Parameters:" ) );
   QLabel* lb_bandload     = us_label(  tr( "Band loading volume (ml):" ) );
   QLabel* lb_spoints      = us_label(  tr( "Simulation Points:" ) );
   QLabel* lb_refopts      = us_banner( tr( "Refinement Options:" ) );

   QLabel* lb_optimiz      = us_banner( tr( "Optimization Methods:" ) );
   QLabel* lb_grrefine     = us_label(  tr( "Grid Refinements:" ) );
   QLabel* lb_menisrng     = us_label(  tr( "Meniscus Fit Range (cm):" ) );
   QLabel* lb_menispts     = us_label(  tr( "Meniscus Grid Points:" ) );
   QLabel* lb_repetloc     = us_label(  tr( "Repetitions:" ) );
   QLabel* lb_scfactor     = us_label(  tr( "Scaling Factor:" ) );
   QLabel* lb_scfact2      = us_label(  tr( "Scaling Factor 2:" ) );
   QLabel* lb_repetran     = us_label(  tr( "Repetitions:" ) );
   QLabel* lb_stddevia     = us_label(  tr( "Standard Deviation:" ) );
   QLabel* lb_coaldist     = us_label(  tr( "Coalescing Distance:" ) );
   QLabel* lb_nbrclips     = us_label(  tr( "# of Clipped Solutes:" ) );
   QLabel* lb_regufact     = us_label(  tr( "Regularization Factor:" ) );

   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel  = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept  = us_pushbutton( tr( "Accept" ) );

   QLayout* lo_stndcp      = us_radiobutton( tr( "Standard Centerpiece"     ),
         rb_stndcp, !sparms->band_forming );
   QLayout* lo_bandcp      = us_radiobutton( tr( "Band-forming Centerpiece" ),
         rb_bandcp,  sparms->band_forming );

   ct_bandload  = us_counter( 3,  0.1,    5,   1 );
   ct_spoints   = us_counter( 2,    1,  100,  10 );

   ct_bandload->setStep(  0.1 );
   ct_spoints ->setStep(    1 );
   ct_bandload->setEnabled( rb_bandcp->isChecked() );

   QLayout*  lo_unifgr  =
      us_checkbox( tr( "Uniform Grid"                      ), ck_unifgr, true );
   QLayout*  lo_menisc  =
      us_checkbox( tr( "Float Meniscus Position"           ), ck_menisc );
   QLayout*  lo_locugr  =
      us_checkbox( tr( "Local Uniform Grid"                ), ck_locugr );
   QLayout*  lo_ranlgr  =
      us_checkbox( tr( "Random Local Grid"                 ), ck_ranlgr );
   QLayout*  lo_soluco  =
      us_checkbox( tr( "Solute Coalescing"                 ), ck_soluco );
   QLayout*  lo_clipcs  =
      us_checkbox( tr( "Clip Lowest Concentration Solutes" ), ck_clipcs );
   QLayout*  lo_regulz  =
      us_checkbox( tr( "Regularization"                    ), ck_regulz );

   ct_grrefine  = us_counter( 2,    1,   20,    7 );
   ct_repetloc  = us_counter( 2,    1,   20,    1 );
   ct_scfactor  = us_counter( 3, 0.01, 10.0,  0.3 );
   ct_scfact2   = us_counter( 3, 0.01, 10.0,  0.9 );
   ct_repetran  = us_counter( 2,    1,   20,    1 );
   ct_stddevia  = us_counter( 3, 0.01, 10.0,  0.1 );
   ct_coaldist  = us_counter( 3, 0.01, 10.0,  0.1 );
   ct_nbrclips  = us_counter( 2,    1,   20,    1 );
   ct_menisrng  = us_counter( 3, 0.55, 0.65,  0.6 );
   ct_menispts  = us_counter( 2,    1,   20,   10 );
   ct_regufact  = us_counter( 3, 0.01, 10.0,  0.9 );

   ct_grrefine ->setStep(    1 );
   ct_repetloc ->setStep(    1 );
   ct_scfactor ->setStep( 0.01 );
   ct_scfact2  ->setStep( 0.01 );
   ct_repetran ->setStep(    1 );
   ct_stddevia ->setStep( 0.01 );
   ct_coaldist ->setStep( 0.01 );
   ct_nbrclips ->setStep(    1 );
   ct_menisrng ->setStep( 0.01 );
   ct_menispts ->setStep(    1 );
   ct_regufact ->setStep( 0.01 );

   cmb_mesh     = us_comboBox();
   cmb_mesh->setMaxVisibleItems( 5 );
   cmb_mesh->addItem( "Adaptive Space Time FE Mesh (ASTFEM)" );
   cmb_mesh->addItem( "Claverie Fixed Mesh" );
   cmb_mesh->addItem( "Moving Hat Mesh" );
   cmb_mesh->addItem( "Specified file (mesh.dat)" );
   cmb_mesh->addItem( "AST Finite Volume Method (ASTFVM)" );
   cmb_mesh->setCurrentIndex( (int)sparms->meshType );
   
   cmb_moving = us_comboBox();
   cmb_moving->setMaxVisibleItems( 2 );
   cmb_moving->addItem( "Constant Time Grid (Claverie/Acceleration)" );
   cmb_moving->addItem( "Moving Time Grid (ASTFEM/Moving Hat)" );
   cmb_moving->setCurrentIndex( (int)sparms->gridType );

   int row      = 0;
   optimizeLayout->addWidget( lb_optimiz,    row++, 0, 1, 6 );
   optimizeLayout->addLayout( lo_unifgr,     row++, 0, 1, 6 );
   optimizeLayout->addWidget( lb_grrefine,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_grrefine,   row++, 4, 1, 2 );
   optimizeLayout->addLayout( lo_locugr,     row++, 0, 1, 6 );
   optimizeLayout->addWidget( lb_repetloc,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_repetloc,   row++, 4, 1, 2 );
   optimizeLayout->addWidget( lb_scfactor,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_scfactor,   row++, 4, 1, 2 );
   optimizeLayout->addWidget( lb_scfact2,    row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_scfact2,    row++, 4, 1, 2 );
   optimizeLayout->addLayout( lo_ranlgr,     row++, 0, 1, 6 );
   optimizeLayout->addWidget( lb_repetran,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_repetran,   row++, 4, 1, 2 );
   optimizeLayout->addWidget( lb_stddevia,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_stddevia,   row++, 4, 1, 2 );
   optimizeLayout->addLayout( lo_soluco,     row++, 0, 1, 6 );
   optimizeLayout->addWidget( lb_coaldist,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_coaldist,   row++, 4, 1, 2 );
   optimizeLayout->addLayout( lo_clipcs,     row++, 0, 1, 6 );
   optimizeLayout->addWidget( lb_nbrclips,   row,   0, 1, 4 );
   optimizeLayout->addWidget( ct_nbrclips,   row++, 4, 1, 2 );

   row          = 0;
   simparmsLayout->addWidget( lb_simpars,    row++, 0, 1, 6 );
   simparmsLayout->addLayout( lo_stndcp,     row,   0, 1, 3 );
   simparmsLayout->addLayout( lo_bandcp,     row++, 3, 1, 3 );
   simparmsLayout->addWidget( lb_bandload,   row,   0, 1, 4 );
   simparmsLayout->addWidget( ct_bandload,   row++, 4, 1, 2 );
   simparmsLayout->addWidget( lb_spoints,    row,   0, 1, 4 );
   simparmsLayout->addWidget( ct_spoints,    row++, 4, 1, 2 );
   simparmsLayout->addWidget( cmb_mesh,      row++, 0, 1, 6 );
   simparmsLayout->addWidget( cmb_moving,    row++, 0, 1, 6 );
   simparmsLayout->addWidget( lb_refopts,    row++, 0, 1, 6 );
   simparmsLayout->addLayout( lo_menisc,     row++, 0, 1, 6 );
   simparmsLayout->addWidget( lb_menisrng,   row,   0, 1, 4 );
   simparmsLayout->addWidget( ct_menisrng,   row++, 4, 1, 2 );
   simparmsLayout->addWidget( lb_menispts,   row,   0, 1, 4 );
   simparmsLayout->addWidget( ct_menispts,   row++, 4, 1, 2 );
   simparmsLayout->addLayout( lo_regulz,     row++, 0, 1, 6 );
   simparmsLayout->addWidget( lb_regufact,   row,   0, 1, 4 );
   simparmsLayout->addWidget( ct_regufact,   row++, 4, 1, 2 );
   simparmsLayout->addWidget( pb_help,       row,   0, 1, 2 );
   simparmsLayout->addWidget( pb_cancel,     row,   2, 1, 2 );
   simparmsLayout->addWidget( pb_accept,     row++, 4, 1, 2 );

   ck_unifgr->setChecked( true  );
   ck_locugr->setChecked( true  );
   adjustSize();
   ck_locugr->setChecked( false );

   optimize_options();

   connect( ck_unifgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkUniGrid(  bool ) ) );
   connect( ck_locugr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkLocalUni( bool ) ) );
   connect( ck_ranlgr, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkRandLoc(  bool ) ) );
   connect( ck_soluco, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkSoluCoal( bool ) ) );
   connect( ck_clipcs, SIGNAL( toggled( bool ) ),
            this,  SLOT( checkClipLow(  bool ) ) );

   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );
   connect( pb_cancel,  SIGNAL( clicked() ),
            this,       SLOT(   reject()  ) );
   connect( pb_accept,  SIGNAL( clicked() ),
            this,       SLOT(   select()  ) );

qDebug() << "Pre-adjust size" << size();
   adjustSize();
qDebug() << "Post-adjust size" << size();
   resize( 740, 330 );
qDebug() << "Post-resize size" << size();
}

// public slot to get dialog parameters
void US_AdvAnalysis::get_parameters(
   int&  rtype, double& rtpar1, double& rtpar2, double& rtpar3,
   bool& men,   double& mepar1, double& mepar2,
   bool& reg,   double& repar1 )
{
   rtype   = US_2dsaProcess::UGRID;
   rtpar1  = ct_grrefine->value();
   rtpar2  = 0.0;
   rtpar3  = 0.0;

   if      ( ck_locugr->isChecked() )
   {
      rtype   = US_2dsaProcess::LUGRID;
      rtpar1  = ct_repetloc->value();
      rtpar2  = ct_scfactor->value();
      rtpar3  = ct_scfact2 ->value();
   }
   else if ( ck_ranlgr->isChecked() )
   {
      rtype   = US_2dsaProcess::RLGRID;
      rtpar1  = ct_repetran->value();
      rtpar2  = ct_stddevia->value();
   }
   else if ( ck_soluco->isChecked() )
   {
      rtype   = US_2dsaProcess::SOLCO;
      rtpar1  = ct_coaldist->value();
   }
   else if ( ck_clipcs->isChecked() )
   {
      rtype   = US_2dsaProcess::CLIPLO;
      rtpar1  = ct_nbrclips->value();
   }

   men     = ck_menisc->isChecked();
   mepar1  = ct_menisrng->value();
   mepar2  = ct_menispts->value();
   reg     = ck_regulz->isChecked();
   repar1  = ct_regufact->value();
}

// enable/disable optimize counters based on chosen method
void US_AdvAnalysis::optimize_options()
{
   ct_grrefine->setEnabled( ck_unifgr->isChecked() );
   ct_repetloc->setEnabled( ck_locugr->isChecked() );
   ct_scfactor->setEnabled( ck_locugr->isChecked() );
   ct_scfact2 ->setEnabled( ck_locugr->isChecked() );
   ct_repetran->setEnabled( ck_ranlgr->isChecked() );
   ct_stddevia->setEnabled( ck_ranlgr->isChecked() );
   ct_coaldist->setEnabled( ck_soluco->isChecked() );
   ct_nbrclips->setEnabled( ck_clipcs->isChecked() );

}

// uncheck optimize options other than one just checked
void US_AdvAnalysis::uncheck_optimize( int ckflag )
{
   if ( ckflag != 1 ) ck_unifgr->setChecked( false );
   if ( ckflag != 2 ) ck_locugr->setChecked( false );
   if ( ckflag != 3 ) ck_ranlgr->setChecked( false );
   if ( ckflag != 4 ) ck_soluco->setChecked( false );
   if ( ckflag != 5 ) ck_clipcs->setChecked( false );
}

// handle uniform grid checked
void US_AdvAnalysis::checkUniGrid(  bool checked )
{
   if ( checked ) { uncheck_optimize( 1 ); optimize_options(); }
qDebug() << "checkuni size" << size();
}

// handle local uniform grid checked
void US_AdvAnalysis::checkLocalUni( bool checked )
{
   if ( checked ) { uncheck_optimize( 2 ); optimize_options(); }
}

// handle random local grid checked
void US_AdvAnalysis::checkRandLoc(  bool checked )
{
   if ( checked ) { uncheck_optimize( 3 ); optimize_options(); }
}

// handle solute coalescing checked
void US_AdvAnalysis::checkSoluCoal( bool checked )
{
   if ( checked ) { uncheck_optimize( 4 ); optimize_options(); }
}

// handle clip lowest conc. solute checked
void US_AdvAnalysis::checkClipLow(  bool checked )
{
   if ( checked ) { uncheck_optimize( 5 ); optimize_options(); }
}

// handle float meniscus position checkec
void US_AdvAnalysis::checkMeniscus( bool checked )
{
   ct_menisrng->setEnabled( checked );
   ct_menispts->setEnabled( checked );
}

// handle regularization checked
void US_AdvAnalysis::checkRegular(  bool checked )
{
   ct_regufact->setEnabled( checked );
}

// accept button clicked
void US_AdvAnalysis::select()
{
   sparms->band_forming = rb_bandcp  ->isChecked();
   sparms->band_volume  = ct_bandload->value();
   sparms->simpoints    = ct_spoints ->value();
   sparms->meshType     = (US_SimulationParameters::MeshType)
                          cmb_mesh   ->currentIndex();
   sparms->gridType     = (US_SimulationParameters::GridType)
                          cmb_moving ->currentIndex();

   accept();
}

