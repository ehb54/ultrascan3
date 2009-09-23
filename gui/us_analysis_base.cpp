//! \file us_analysis_base.cpp

#include "us_analysis_base.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_run_details.h"
#include "us_vbar.h"

US_AnalysisBase::US_AnalysisBase() : US_Widgets()
{
   setPalette( US_GuiSettings::frameColor() );


   mainLayout      = new QHBoxLayout( this );
   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   
   analysisLayout  = new QGridLayout();
   runInfoLayout   = new QGridLayout();
   parameterLayout = new QGridLayout();
   controlsLayout  = new QGridLayout();
   buttonLayout    = new QHBoxLayout();

   leftLayout->addLayout( analysisLayout  );
   leftLayout->addLayout( runInfoLayout   );
   leftLayout->addLayout( parameterLayout );
   leftLayout->addLayout( controlsLayout  );
   leftLayout->addStretch();
   leftLayout->addLayout( buttonLayout    );

   // Plots
   plotLayout1 = new US_Plot( data_plot1,
            tr( "Plot 1 Title" ),
            tr( "X-Axis Title" ),
            tr( "Y-Axis Title" ) );

   data_plot1->setMinimumSize( 600, 300 );

   plotLayout2 = new US_Plot( data_plot2,
            tr( "Plot 2 Title" ),
            tr( "X-Axis Title" ),
            tr( "Y-Axis Title" ) );

   data_plot2->setMinimumSize( 600, 300 );

   rightLayout->addLayout( plotLayout1 );
   rightLayout->addLayout( plotLayout2 );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );

   // Analysis buttons
   pb_load    = us_pushbutton( tr( "Load Data" ) );
   pb_details = us_pushbutton( tr( "Run Details" ) );
   connect( pb_details, SIGNAL( clicked() ), SLOT( details() ) );
   pb_view    = us_pushbutton( tr( "View Data Report" ) );
   pb_save    = us_pushbutton( tr( "Save Data" ) );

   pb_details->setEnabled( false );
   pb_view   ->setEnabled( false );
   pb_save   ->setEnabled( false );

   analysisLayout->addWidget( pb_load,    0, 0 );
   analysisLayout->addWidget( pb_details, 0, 1 );
   analysisLayout->addWidget( pb_view,    1, 0 );
   analysisLayout->addWidget( pb_save,    1, 1 );

   // Standard buttons
   pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_help  = us_pushbutton( tr( "Help"  ) );
   pb_close = us_pushbutton( tr( "Close" ) );

   buttonLayout->addWidget( pb_reset );
   buttonLayout->addWidget( pb_help  );
   buttonLayout->addWidget( pb_close );
   connect( pb_close, SIGNAL( clicked() ), SLOT( close() ) );

   // Run info
   QLabel* lb_info    = us_banner( tr( "Information for this Run" ) );
   QLabel* lb_triples = us_banner( tr( "Cell / Channel / Wavelength" ) );
   QLabel* lb_id      = us_label ( tr( "Run ID / Edit ID:" ) );
   QLabel* lb_temp    = us_label ( tr( "Avg Temperature:" ) );

   le_id      = us_lineedit();
   le_temp    = us_lineedit();

   te_desc    = us_textedit();
   lw_triples = us_listwidget();

   QFont        font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm  ( font );

   int fontHeight = fm.lineSpacing();

   te_desc   ->setMaximumHeight( fontHeight * 2 + 12 );  // Add for border
   lw_triples->setMaximumHeight( fontHeight * 4 + 12 );

   le_id     ->setReadOnly( true );
   le_temp   ->setReadOnly( true );
   te_desc   ->setReadOnly( true );

   runInfoLayout->addWidget( lb_info   , 0, 0, 1, 2 );
   runInfoLayout->addWidget( lb_id     , 1, 0 );
   runInfoLayout->addWidget( le_id     , 1, 1 );
   runInfoLayout->addWidget( lb_temp   , 2, 0 );
   runInfoLayout->addWidget( le_temp   , 2, 1 );
   runInfoLayout->addWidget( te_desc   , 3, 0, 2, 2 );
   runInfoLayout->addWidget( lb_triples, 5, 0, 1, 2 );
   runInfoLayout->addWidget( lw_triples, 6, 0, 4, 2 );

   // Parameters

   QPushButton* pb_density   = us_pushbutton( tr( "Density"   ) );
   connect( pb_density, SIGNAL( clicked() ), SLOT( tbd() ) );
   
   QPushButton* pb_viscosity = us_pushbutton( tr( "Viscosity" ) );
   connect( pb_viscosity, SIGNAL( clicked() ), SLOT( tbd() ) );
   
   QPushButton* pb_vbar      = us_pushbutton( tr( "vbar"   ) );
   connect( pb_vbar, SIGNAL( clicked() ), SLOT( get_vbar() ) );
   
   QLabel*      lb_skipped   = us_label     ( tr( "Skipped:"  ) );

   le_density   = us_lineedit( "0.998234" );
   le_viscosity = us_lineedit( "1.001940" );
   le_vbar      = us_lineedit( "0.7200" );
   le_skipped   = us_lineedit( "0" );
   le_skipped->setReadOnly( true );

   parameterLayout->addWidget( pb_density  , 0, 0 );
   parameterLayout->addWidget( le_density  , 0, 1 );
   parameterLayout->addWidget( pb_viscosity, 0, 2 );
   parameterLayout->addWidget( le_viscosity, 0, 3 );
   parameterLayout->addWidget( pb_vbar     , 1, 0 );
   parameterLayout->addWidget( le_vbar     , 1, 1 );
   parameterLayout->addWidget( lb_skipped  , 1, 2 );
   parameterLayout->addWidget( le_skipped  , 1, 3 );

   // Analysis Controls

   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"  ) ); 
   QLabel* lb_scan         = us_banner( tr( "Scan Control"       ) ); 
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"    ) ); 
   QLabel* lb_boundPercent = us_label ( tr( "% of Boundary:"     ) ); 
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Pos. (%):" ) ); 

   QLabel* lb_from         = us_label ( tr( "From:" ) );
   QLabel* lb_to           = us_label ( tr( "to:"   ) );

   pb_exclude = us_pushbutton( tr( "Exclude Scan Range" ) );
   pb_exclude->setEnabled( false );

   ct_smoothing       = us_counter( 2, 1 , 1 );
   ct_boundaryPercent = us_counter( 3, 10, 100, 100 );
   ct_boundaryPos     = us_counter( 3, 0 , 100, 0   );
   
   ct_from            = us_counter( 2, 0, 0 );
   ct_to              = us_counter( 2, 0, 0 );

   controlsLayout->addWidget( lb_analysis       , 0, 0, 1, 4 );
   controlsLayout->addWidget( lb_smoothing      , 1, 0, 1, 2 );
   controlsLayout->addWidget( ct_smoothing      , 1, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPercent   , 2, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPercent, 2, 2, 1, 2 );
   controlsLayout->addWidget( lb_boundPos       , 3, 0, 1, 2 );
   controlsLayout->addWidget( ct_boundaryPos    , 3, 2, 1, 2 );
   controlsLayout->addWidget( lb_scan           , 4, 0, 1, 4 );
   controlsLayout->addWidget( lb_from           , 5, 0 );
   controlsLayout->addWidget( ct_from           , 5, 1 );
   controlsLayout->addWidget( lb_to             , 5, 2 );
   controlsLayout->addWidget( ct_to             , 5, 3 );
   controlsLayout->addWidget( pb_exclude        , 6, 0, 1, 4 );

}

void US_AnalysisBase::load( void )
{
   // Determine the edit ID
   load_status    = 1;
   QString filter = "*.*.*.*.*.*.xml";

   QString filename = QFileDialog::getOpenFileName( this, 
         tr( "Select a file with the desired Edit ID" ),
         US_Settings::resultDir(),
         filter );

   if ( filename.isEmpty() ) return;

   QStringList sl = filename.split( "." );
   runID  = sl[ 0 ];
   editID = sl[ 1 ];

   filename.replace( "\\", "/" );  // For WIN32
   directory = filename.left( filename.lastIndexOf( "/" ) );
   
   // Get the raw data and edit parameters file names
   filter = "*." + editID + ".*.*.*.*.xml";
   
   QDir d( directory );
   sl = d.entryList( QStringList() << filter, 
                     QDir::Files | QDir::Readable, QDir::Name );

   // Read the data into the structure
   dataList.clear();
   rawList .clear();
   lw_triples->clear();
   triples.clear();

   try
   {
      for ( int i = 0; i < sl.size(); i++ )
      {
         filename = sl[ i ];
         US_DataIO::loadData( directory, filename, dataList, rawList );

         US_DataIO::editedData* ed = &dataList[ i ];

         QString ccw = ed->cell + " / " + ed->channel + " / " + ed->wavelength;

         lw_triples->addItem( ccw );
         triples << ccw;
      }
   }
   catch ( US_DataIO::ioError error )
   {
      // There was an error reading filname or it's associated raw data
      qDebug() << US_DataIO::errorString( error );
   }

   lw_triples->setCurrentRow( 0 );
   update( 0 );

   // Enable other buttons
   pb_details->setEnabled( true );
   pb_view   ->setEnabled( true );
   pb_save   ->setEnabled( true );
   pb_exclude->setEnabled( true );

   load_status = 0;
}

void US_AnalysisBase::update( int selection )
{
   US_DataIO::editedData* d = &dataList[ selection ];
   int scanCount = d->scanData.size();
   le_id->setText( d->runID + " / " + d->editID );

   double sum = 0.0;
   
   for ( int i = 0; i < scanCount; i++ ) 
      sum += d->scanData[ i ].temperature;

   QString t = QString::number( sum / scanCount, 'f', 1 ) 
             + tr( "deg C" );
   le_temp->setText( t );

   te_desc->setText( d->description );

   ct_from->setMaxValue( scanCount );
   ct_from->setStep( 1.0 );
   ct_to  ->setMaxValue( scanCount );
   ct_to  ->setStep( 1.0 );
}

void US_AnalysisBase::details( void )
{
   US_RunDetails* dialog
      = new US_RunDetails( rawList, runID, directory, triples );
   dialog->exec();
   qApp->processEvents();
   delete dialog;
}

void US_AnalysisBase::get_vbar( void )
{
   US_Vbar* vbar_dialog = new US_Vbar( -1, true );
   connect( vbar_dialog, SIGNAL( valueChanged( double ) ),
                         SLOT  ( update_vbar ( double ) ) );
   vbar_dialog->exec();
   qApp->processEvents();
   //delete vbar_dialog;
}

void US_AnalysisBase::update_vbar( double new_vbar )
{
   vbar = new_vbar;
   le_vbar->setText( QString::number( new_vbar, 'f', 4 ) );
}

void US_AnalysisBase::tbd( void )
{
   QMessageBox::information( this, "TBD", "Under Construction" );
}
void US_AnalysisBase::tbd( double )
{
   QMessageBox::information( this, "TBD", "Under Construction" );
}
