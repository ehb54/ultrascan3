//! \file us_advanced.cpp

#include "us_advanced.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// constructor:  advanced analysis control widget
US_Advanced::US_Advanced( US_Model* amodel, QMap< QString, QString >& adv_vals,
    QWidget* p ) : US_WidgetsDialog( p, 0 ), parmap( adv_vals )
{
   model          = amodel;
   parentw        = p;

   setObjectName( "US_Advanced" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "FeMatch Advanced Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   upperLayout     = new QHBoxLayout();
   lowerLayout     = new QHBoxLayout();
   analysisLayout  = new QGridLayout( );
   modelcomLayout  = new QGridLayout( );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   upperLayout->addLayout( analysisLayout  );
   upperLayout->addLayout( modelcomLayout );
   upperLayout->setStretchFactor( analysisLayout, 1 );
   upperLayout->setStretchFactor( modelcomLayout, 1 );

   QLabel* lb_analysis  = us_banner( tr( "Advanced Analysis"          ) );
   QLabel* lb_simpoints = us_label(  tr( "Simulation Points:"         ) );
   QLabel* lb_bldvolume = us_label(  tr( "Band-loading Volume:"       ) );
   QLabel* lb_parameter = us_label(  tr( "Parameter:"                 ) );

   QLabel* lb_modelcom  = us_banner( tr( "Model Components"           ) );
   QLabel* lb_sedcoeff  = us_label(  tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_difcoeff  = us_label(  tr( "Diffusion Coefficient:"     ) );
   QLabel* lb_moweight  = us_label(  tr( "Molecular Weight (kD):"     ) );
   QLabel* lb_friratio  = us_label(  tr( "Frictional Ratio (f/f0):"   ) );
   QLabel* lb_partconc  = us_label(  tr( "Partial Concentration:"     ) );

   QPushButton* pb_component = us_pushbutton( tr( "Next Component" ) );
   QPushButton* pb_showmodel = us_pushbutton( tr( "Show Model #"   ) );

   ct_simpoints = us_counter( 3, 0, 500,     1 );
   ct_bldvolume = us_counter( 3, 0,   1, 0.001 );
   ct_parameter = us_counter( 2, 1,  50,     1 );
   ct_modelnbr  = us_counter( 2, 1,  50,     1 );
   ct_component = us_counter( 2, 1, 200,     1 );

   le_sedcoeff  = us_lineedit( "", -1, true );
   le_difcoeff  = us_lineedit( "", -1, true );
   le_moweight  = us_lineedit( "", -1, true );
   le_friratio  = us_lineedit( "", -1, true );
   le_partconc  = us_lineedit( "", -1, true );

   cb_mesh      = us_comboBox();
   cb_mesh->addItem( "Adaptive Space Time Mesh (ASTFEM)" );
   cb_mesh->addItem( "Claverie Mesh"                     );
   cb_mesh->addItem( "Moving Hat Mesh"                   );
   cb_mesh->addItem( "File: \"$ULTRASCAN/mesh.dat\""     );
   cb_mesh->addItem( "AST Finite Volume Method (ASTFVM)" );

   cb_grid      = us_comboBox();
   cb_grid->addItem( "Moving Time Grid"                  );
   cb_grid->addItem( "Constant Time Grid"                );

   gb_modelsim  = new QGroupBox(
      tr( "Simulate data using parameters from model"
          " or from Monte Carlo statistics" ) );
   gb_modelsim->setFlat( true );
   rb_curmod    = new QRadioButton( tr( "Current Model" ) );
   rb_mode      = new QRadioButton( tr( "Mode"          ) );
   rb_mean      = new QRadioButton( tr( "Mean"          ) );
   rb_median    = new QRadioButton( tr( "Median"        ) );
   gb_modelsim->setFont( pb_showmodel->font() );
   gb_modelsim->setPalette( US_GuiSettings::normalColor() );
   QHBoxLayout* mosbox = new QHBoxLayout();
   mosbox->addWidget( rb_curmod );
   mosbox->addWidget( rb_mode   );
   mosbox->addWidget( rb_mean   );
   mosbox->addWidget( rb_median );
   mosbox->setSpacing( 0 );
   gb_modelsim->setLayout( mosbox );
   rb_curmod  ->setChecked( true );
   lowerLayout->addWidget( gb_modelsim );

   mainLayout ->addLayout( upperLayout );
   mainLayout ->addLayout( lowerLayout );

   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel  = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept  = us_pushbutton( tr( "Accept" ) );

   int row      = 0;
   analysisLayout->addWidget( lb_analysis,   row++, 0, 1, 6 );
   analysisLayout->addWidget( lb_simpoints,  row,   0, 1, 3 );
   analysisLayout->addWidget( ct_simpoints,  row++, 3, 1, 3 );
   analysisLayout->addWidget( lb_bldvolume,  row,   0, 1, 3 );
   analysisLayout->addWidget( ct_bldvolume,  row++, 3, 1, 3 );
   analysisLayout->addWidget( lb_parameter,  row,   0, 1, 3 );
   analysisLayout->addWidget( ct_parameter,  row++, 3, 1, 3 );
   analysisLayout->addWidget( pb_showmodel,  row,   0, 1, 3 );
   analysisLayout->addWidget( ct_modelnbr,   row++, 3, 1, 3 );
   analysisLayout->addWidget( cb_mesh,       row++, 0, 1, 6 );
   analysisLayout->addWidget( cb_grid,       row++, 0, 1, 6 );
   analysisLayout->addWidget( pb_help,       row,   0, 1, 2 );
   analysisLayout->addWidget( pb_cancel,     row,   2, 1, 2 );
   analysisLayout->addWidget( pb_accept,     row++, 4, 1, 2 );

   row          = 0;
   modelcomLayout->addWidget( lb_modelcom,   row++, 0, 1, 6 );
   modelcomLayout->addWidget( lb_sedcoeff,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_sedcoeff,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_difcoeff,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_difcoeff,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_moweight,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_moweight,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_friratio,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_friratio,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_partconc,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_partconc,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( pb_component,  row,   0, 1, 3 );
   modelcomLayout->addWidget( ct_component,  row++, 3, 1, 3 );

   ct_simpoints->setValue( 200   );
   ct_bldvolume->setValue( 0.015 );
   ct_parameter->setValue( 0     );
   ct_modelnbr ->setValue( 0     );
   ct_component->setValue( 0     );
   ct_simpoints->setStep(     5 );
   ct_bldvolume->setStep( 0.001 );
   ct_parameter->setStep(     1 );
   ct_modelnbr ->setStep(     1 );
   ct_component->setStep(     1 );
   ct_component->setMaxValue( model->components.size() );

   pb_showmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );

   connect( pb_component, SIGNAL( clicked()        ),
            this,         SLOT(   next_component() ) );
   connect( ct_component, SIGNAL( valueChanged(  double ) ),
            this,         SLOT(   set_component( double ) ) );

   set_component( 1.0 );

   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );
   connect( pb_cancel,  SIGNAL( clicked() ),
            this,       SLOT(   reject()  ) );
   connect( pb_accept,  SIGNAL( clicked() ),
            this,       SLOT(   done()    ) );

qDebug() << "Pre-adjust size" << size();
   adjustSize();
qDebug() << "Post-adjust size" << size();
   resize( 700, 250 );
qDebug() << "Post-resize size" << size();
   qApp->processEvents();
}

// private slot to pass parameters then close with an accepted() signal
void US_Advanced::done( void )
{
   parmap[ "simpoints" ] = QString::number( ct_simpoints->value() );
   parmap[ "bldvolume" ] = QString::number( ct_bldvolume->value() );
   parmap[ "parameter" ] = QString::number( ct_parameter->value() );
   parmap[ "modelnbr"  ] = QString::number( ct_modelnbr ->value() );
   parmap[ "meshtype"  ] = cb_mesh->currentText();
   parmap[ "gridtype"  ] = cb_grid->currentText();
   parmap[ "modelsim"  ] = rb_curmod->isChecked() ? "model"  :
                         ( rb_mode  ->isChecked() ? "mode"   :
                         ( rb_mean  ->isChecked() ? "mean"   :
                         ( rb_median->isChecked() ? "median" : "" ) ) );

   accept();
}

// private slot to advance to the next model component
void US_Advanced::next_component( void )
{
   int icomp = (int)ct_component->value();
   int ncomp = model->components.size();

   icomp     = ( icomp < ncomp ) ? ( icomp + 1 ) : 1;
   ct_component->setValue( (double)icomp );
}

// private slot to set the model component index and fill in the implied text
void US_Advanced::set_component( double compx )
{
   int icomp  = (int)compx - 1;

   if ( icomp < 0 )
      return;

   le_sedcoeff->setText( QString::number( model->components[ icomp ].s ) );
   le_difcoeff->setText( QString::number( model->components[ icomp ].D ) );
   le_moweight->setText(
      QString::number( model->components[ icomp ].mw / 1000.0 ) );
   le_friratio->setText( QString::number( model->components[ icomp ].f_f0 ) );
   le_partconc->setText(
      QString::number( model->components[ icomp ].signal_concentration ) );
}

