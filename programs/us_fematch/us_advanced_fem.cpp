//! \file us_advanced_fem.cpp

#include "us_advanced_fem.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// constructor:  advanced analysis control widget
US_AdvancedFem::US_AdvancedFem( US_Model* amodel,
   QMap< QString, QString >& adv_vals, QWidget* p ) :
   US_WidgetsDialog( p, 0 ), parmap( adv_vals )
{
   model          = amodel;
   parentw        = p;

   setObjectName( "US_AdvancedFem" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "FeMatch Advanced Controls" ) );

   mainLayout      = new QGridLayout( this );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_analysis   = us_banner( tr( "Advanced Analysis"          ) );
   QLabel* lb_simpoints  = us_label(  tr( "Simulation Points:"         ) );
   QLabel* lb_bandvolume = us_label(  tr( "Band-loading Volume:"       ) );
   QLabel* lb_parameter  = us_label(  tr( "Parameter:"                 ) );

   ct_simpoints  = us_counter( 3, 0, 10000,   1 );
   ct_bandvolume = us_counter( 3, 0,   1, 0.001 );
   ct_parameter  = us_counter( 2, 1,  50,     1 );

   cb_mesh      = us_comboBox();
   cb_mesh->addItem( "Adaptive Space Time Mesh (ASTFEM)" );
   cb_mesh->addItem( "Claverie Mesh"                     );
   cb_mesh->addItem( "Moving Hat Mesh"                   );
   cb_mesh->addItem( "File: \"$ULTRASCAN/mesh.dat\""     );
   cb_mesh->addItem( "AST Finite Volume Method (ASTFVM)" );

   cb_grid      = us_comboBox();
   cb_grid->addItem( "Moving Time Grid"                  );
   cb_grid->addItem( "Constant Time Grid"                );

   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel  = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept  = us_pushbutton( tr( "Accept" ) );

   int row      = 0;
   mainLayout->addWidget( lb_analysis,     row++, 0, 1, 6 );
   mainLayout->addWidget( lb_simpoints,    row,   0, 1, 3 );
   mainLayout->addWidget( ct_simpoints,    row++, 3, 1, 3 );
   mainLayout->addWidget( lb_bandvolume,   row,   0, 1, 3 );
   mainLayout->addWidget( ct_bandvolume,   row++, 3, 1, 3 );
   mainLayout->addWidget( lb_parameter,    row,   0, 1, 3 );
   mainLayout->addWidget( ct_parameter,    row++, 3, 1, 3 );
   mainLayout->addWidget( cb_mesh,         row++, 0, 1, 6 );
   mainLayout->addWidget( cb_grid,         row++, 0, 1, 6 );
   mainLayout->addWidget( pb_help,         row,   0, 1, 2 );
   mainLayout->addWidget( pb_cancel,       row,   2, 1, 2 );
   mainLayout->addWidget( pb_accept,       row++, 4, 1, 2 );

   ct_simpoints ->setValue( parmap[ "simpoints" ].toDouble() );
   ct_bandvolume->setValue( parmap[ "bandvolume" ].toDouble() );
   ct_parameter ->setValue( parmap[ "parameter" ].toDouble() );
   ct_simpoints ->setSingleStep(     5 );
   ct_bandvolume->setSingleStep( 0.001 );
   ct_parameter ->setSingleStep(     1 );

   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help()    ) );
   connect( pb_cancel,  SIGNAL( clicked() ),
            this,       SLOT(   reject()  ) );
   connect( pb_accept,  SIGNAL( clicked() ),
            this,       SLOT(   done()    ) );

qDebug() << "AdvF:Pre-adjust size" << size();
   adjustSize();
qDebug() << "AdvF:Post-adjust size" << size();
   resize( 400, 150 );
qDebug() << "AdvF:Post-resize size" << size();
   qApp->processEvents();
}

// private slot to pass parameters then close with an accepted() signal
void US_AdvancedFem::done( void )
{
   parmap[ "simpoints"  ] = QString::number( ct_simpoints ->value() );
   parmap[ "bandvolume" ] = QString::number( ct_bandvolume->value() );
   parmap[ "parameter"  ] = QString::number( ct_parameter ->value() );
   parmap[ "meshtype"   ] = cb_mesh->currentText();
   parmap[ "gridtype"   ] = cb_grid->currentText();

   accept();
}

