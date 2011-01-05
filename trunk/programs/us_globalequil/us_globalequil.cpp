//! file us_globalequil.cpp
#include <QApplication>
#include "us_globalequil.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_constants.h"
#include "us_math2.h"

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_GlobalEquil w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_GlobalEquil class constructor
US_GlobalEquil::US_GlobalEquil() : US_Widgets( true )
{
   setWindowTitle( tr( "Global Equilibrium Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );

   QBoxLayout* mainLayout = new QHBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Left Column
   QVBoxLayout* leftLayout     = new QVBoxLayout;
   QVBoxLayout* rightLayout    = new QVBoxLayout;

   QGridLayout* dataSelLayout  = new QGridLayout;
   QGridLayout* scnListLayout  = new QGridLayout;
   QGridLayout* modlFitLayout  = new QGridLayout;
   QGridLayout* paramLayout    = new QGridLayout;
   QGridLayout* statusLayout   = new QGridLayout;

   QLabel*      lb_datasel   = us_banner(     tr( "Data Selection"      ) ); 
   QPushButton* pb_loadExp   = us_pushbutton( tr( "Load Experiment"     ) );
   QPushButton* pb_details   = us_pushbutton( tr( "Run Details"         ) );
   QLayout*     lo_edlast    = us_checkbox(   tr( "Lastest Data Edit"   ),
                                              ck_edlast, true );
                dkdb_cntrls  = new US_Disk_DB_Controls;
   QPushButton* pb_view      = us_pushbutton( tr( "View Report"         ) );
   QPushButton* pb_unload    = us_pushbutton( tr( "Unload all Data"     ) );
   QPushButton* pb_scdiags   = us_pushbutton( tr( "Scan Diagnostics"    ) );
   QPushButton* pb_ckscfit   = us_pushbutton( tr( "Check Scans for Fit" ) );
   QPushButton* pb_conchist  = us_pushbutton( tr( "Conc. Histogram"     ) );
   QPushButton* pb_resetsl   = us_pushbutton( tr( "Reset Scan Limits"   ) );
   QLabel*      lb_prjname   = us_label(      tr( "Project Name:"       ) );
   QLineEdit*   le_prjname   = us_lineedit();

   connect( pb_loadExp,  SIGNAL( clicked() ),
                         SLOT(   load()    ) );
   connect( pb_details,  SIGNAL( clicked() ),
                         SLOT(   details() ) );
   //connect( pb_view,     SIGNAL( clicked() ),
   //                      SLOT( change_model() ) );

   int row = 0;
   dataSelLayout->addWidget( lb_datasel,  row++, 0, 1, 2 );
   dataSelLayout->addWidget( pb_loadExp,  row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_details,  row++, 1, 1, 1 );
   dataSelLayout->addLayout( lo_edlast,   row,   0, 1, 1 );
   dataSelLayout->addLayout( dkdb_cntrls, row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_unload,   row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_view,     row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_scdiags,  row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_ckscfit,  row++, 1, 1, 1 );
   dataSelLayout->addWidget( pb_conchist, row,   0, 1, 1 );
   dataSelLayout->addWidget( pb_resetsl,  row++, 1, 1, 1 );
   dataSelLayout->addWidget( lb_prjname,  row,   0, 1, 1 );
   dataSelLayout->addWidget( le_prjname,  row++, 1, 1, 1 );

   QLabel*      lb_equiscns  = us_banner( tr( "List of available Equilibrium"
                                              " Scans:" ) );
   QTextEdit*   te_equiscns  = us_textedit();
   QFont font( US_GuiSettings::fontFamily(),
               US_GuiSettings::fontSize(),
               QFont::Bold );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   te_equiscns->setMaximumHeight( fontHeight * 15 + 12 );

   row     = 0;
   scnListLayout->addWidget( lb_equiscns, row++, 0, 1, 2 );
   scnListLayout->addWidget( te_equiscns, row,   0, 5, 2 );
   row    += 5;

   QLabel*      lb_mfitinfo  = us_banner(     tr( "Model and Fitting"
                                                  " Information:" ) );
   QPushButton* pb_selModel  = us_pushbutton( tr( "Select Model"        ) );
   QPushButton* pb_modlCtrl  = us_pushbutton( tr( "Model Control"         ) );
   QPushButton* pb_fitcntrl  = us_pushbutton( tr( "Fitting Control"     ) );
   QPushButton* pb_loadFit   = us_pushbutton( tr( "Load Fit"            ) );
   QPushButton* pb_monCarlo  = us_pushbutton( tr( "Monte Carlo"         ) );
   
   row     = 0;
   modlFitLayout->addWidget( lb_mfitinfo, row++, 0, 1, 2 );
   modlFitLayout->addWidget( pb_selModel, row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_modlCtrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_loadFit,  row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_fitcntrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_monCarlo, row,   0, 1, 1 );

   QLabel*      lb_parminfo  = us_banner( tr( "Parameter Information:"    ) );
   QPushButton* pb_floatPar  = us_pushbutton( tr( "Float Parameters"      ) );
   QPushButton* pb_initPars  = us_pushbutton( tr( "Initialize Parameters" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help"                  ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close"                 ) );
   QLabel*      lb_scselect  = us_label(      tr( "Scan Selector:"      ) );
   QwtCounter*  ct_scselect  = us_counter( 2, 0, 50, 1 );
   ct_scselect->setStep(  1.0 );
   ct_scselect->setValue( 0.0 );

   connect( pb_help,     SIGNAL( clicked() ),
                         SLOT(   help()    ) );
   connect( pb_close,    SIGNAL( clicked() ),
                         SLOT(   close()   ) );
   //connect( ct_scselect, SIGNAL( valueChanged( double ) ),
   //                      SLOT(   scan_select(  double ) ) );

   row     = 0;
   paramLayout  ->addWidget( lb_parminfo, row++, 0, 1, 4 );
   paramLayout  ->addWidget( pb_floatPar, row,   0, 1, 2 );
   paramLayout  ->addWidget( pb_initPars, row++, 2, 1, 2 );
   paramLayout  ->addWidget( pb_help,     row,   0, 1, 2 );
   paramLayout  ->addWidget( pb_close,    row++, 2, 1, 2 );
   paramLayout  ->addWidget( lb_scselect, row,   0, 1, 2 );
   paramLayout  ->addWidget( ct_scselect, row++, 2, 1, 2 );
   
   leftLayout->addLayout( dataSelLayout );
   leftLayout->addLayout( scnListLayout );
   leftLayout->addLayout( modlFitLayout );
   leftLayout->addLayout( paramLayout   );


   // Right Column

   // Simulation plot

   QBoxLayout* plot = new US_Plot( equilibrium_plot, 
         tr( "Experimental Equilibrium Data" ),
         tr( "Radius" ), tr( "Concentration" ) );
   us_grid( equilibrium_plot );
   
   equilibrium_plot->setMinimumSize( 600, 400 );
   equilibrium_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   equilibrium_plot->setAxisScale( QwtPlot::xBottom, 5.9, 6.2 );

   QLabel*      lb_status    = us_label(    tr( "Status/Information:" ) );
   QLineEdit*   le_status    = us_lineedit( tr( "Please select an edited"
                                                " Equilibrium Dataset with"
                                                " \"Load Experiment\"" ) );
   QLabel*      lb_currmodl  = us_label(    tr( "Current Model:"      ) );
   QLineEdit*   le_currmodl  = us_lineedit( tr( "-- none selected --" ) );
   QLabel*      lb_mxfringe  = us_label(    tr( "Max. OD/Fringe:"     ) );
   QLineEdit*   le_mxfringe  = us_lineedit( tr( "0.90" ) );
   QLabel*      lb_mxfnotes  = us_label(    tr( "(set to zero to inactivate"
                                                " high conc. limits)"  ) );
   row     = 0;
   statusLayout ->addWidget( lb_status,   row,   0, 1, 2 );
   statusLayout ->addWidget( le_status,   row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_currmodl, row,   0, 1, 2 );
   statusLayout ->addWidget( le_currmodl, row++, 2, 1, 4 );
   statusLayout ->addWidget( lb_mxfringe, row,   0, 1, 2 );
   statusLayout ->addWidget( le_mxfringe, row,   2, 1, 1 );
   statusLayout ->addWidget( lb_mxfnotes, row++, 3, 1, 3 );

   rightLayout->addLayout( plot );
   rightLayout->addLayout( statusLayout );
   rightLayout->setStretchFactor( plot,        10 );
   rightLayout->setStretchFactor( statusLayout, 2 );

   mainLayout->addLayout( leftLayout  ); 
   mainLayout->addLayout( rightLayout ); 
   mainLayout->setStretchFactor( leftLayout,   3 );
   mainLayout->setStretchFactor( rightLayout,  5 );
}

void US_GlobalEquil::load( void )
{
   /*
   QString fn = QFileDialog::getOpenFileName( this,
                  tr( "Select Experiment File" ),
                  US_Settings::resultDir(), "*.us_system" );

   if ( ! fn.isEmpty() )
   {
      //FIXME
      int error_code = 0;//US_FemGlobal::read_experiment( model, simparams, fn );

      if ( error_code < 0 )
      {
         QMessageBox::information( this,
            tr( "Simulation Module" ),
            tr( "Unable to load System: ") + fn + tr( "\n\nError code: " ) +
                QString::number( error_code ) );
         return;
      }
      else
      {
         pb_changeModel->setEnabled( true );
         pb_saveExp    ->setEnabled( true );
         
         QMessageBox::information( this,
            tr( "Simulation Module" ),
            tr( "Successfully loaded Model:\n\n" ) + model.description );
      }
   }
    */

}

void US_GlobalEquil::new_model( void )
{
   //FIXME
}

void US_GlobalEquil::change_model( void )
{
   //US_ModelEditor* component_dialog = new US_ModelEditor( model );
   //component_dialog->exec();
}

void US_GlobalEquil::load_model( void )
{
}

void US_GlobalEquil::init_simparams( void )
{

}

void US_GlobalEquil::init_astfem_data( void )
{
}

void US_GlobalEquil::simulate( void )
{

}

void US_GlobalEquil::details( void )
{
   //US_RunDetails2 dialog( rawList, runID, workDir, triples );
   //dialog.exec();
   qApp->processEvents();
}
