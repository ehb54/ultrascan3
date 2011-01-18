//! file us_globalequil.cpp
#include <QApplication>
#include "us_globalequil.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_data_loader.h"
#include "us_gui_settings.h"
#include "us_run_details2.h"
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
                pb_details   = us_pushbutton( tr( "Run Details"         ) );
   QLayout*     lo_edlast    = us_checkbox(   tr( "Lastest Data Edit"   ),
                                              ck_edlast, true );
                dkdb_cntrls  = new US_Disk_DB_Controls(
                                  US_Settings::default_data_location() );
                pb_view      = us_pushbutton( tr( "View Report"         ) );
                pb_unload    = us_pushbutton( tr( "Unload all Data"     ) );
                pb_scdiags   = us_pushbutton( tr( "Scan Diagnostics"    ) );
                pb_ckscfit   = us_pushbutton( tr( "Check Scans for Fit" ) );
                pb_conchist  = us_pushbutton( tr( "Conc. Histogram"     ) );
                pb_resetsl   = us_pushbutton( tr( "Reset Scan Limits"   ) );
   QLabel*      lb_prjname   = us_label(      tr( "Project Name:"       ) );
                le_prjname   = us_lineedit();

   pb_details ->setEnabled( false );
   pb_view    ->setEnabled( false );
   pb_unload  ->setEnabled( false );
   pb_scdiags ->setEnabled( false );
   pb_ckscfit ->setEnabled( false );
   pb_conchist->setEnabled( false );
   pb_resetsl ->setEnabled( false );
   le_prjname ->setText( tr( "SampleFit" ) );

   connect( pb_loadExp,  SIGNAL( clicked()       ),
                         SLOT(   load()          ) );
   connect( pb_details,  SIGNAL( clicked()       ),
                         SLOT(   details()       ) );
   connect( pb_view,     SIGNAL( clicked()       ),
                         SLOT(   view_report()   ) );
   connect( pb_unload,   SIGNAL( clicked()       ),
                         SLOT(   unload()        ) );
   connect( pb_scdiags,  SIGNAL( clicked()       ),
                         SLOT(   scan_diags()    ) );
   connect( pb_ckscfit,  SIGNAL( clicked()       ),
                         SLOT(  check_scan_fit() ) );
   connect( pb_conchist, SIGNAL( clicked()       ),
                         SLOT(  conc_histogram() ) );
   connect( pb_resetsl,  SIGNAL( clicked()       ),
                         SLOT(   reset_scan_lims() ) );
   connect( le_prjname,  SIGNAL( textChanged(    const QString& ) ),
                         SLOT( new_project_name( const QString& ) ) );

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
   tw_equiscns = new QTableWidget( 15, 5, this );
   tw_equiscns->setPalette( US_GuiSettings::editColor() );
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize(),
         QFont::Bold );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   tw_equiscns->setMaximumHeight( fontHeight * 60 + 12 );
   tw_equiscns->setRowHeight( 0, fontHeight );

   QString icondirn = US_Settings::appBaseDir() + "/etc/";
   const QString bluarrfn( "bluearrow.png" );
   const QString grnarrfn( "greenarrow.png" );
   const QString redarrfn( "redarrow.png" );
   blue_arrow  = QIcon( QPixmap( icondirn + bluarrfn ) );
   green_arrow = QIcon( QPixmap( icondirn + grnarrfn ) );
   red_arrow   = QIcon( QPixmap( icondirn + redarrfn ) );
qDebug() << "ArrowIcon isNull" << blue_arrow.isNull();

   row     = 0;
   scnListLayout->addWidget( lb_equiscns, row++, 0, 1, 2 );
   scnListLayout->addWidget( tw_equiscns, row,   0, 5, 2 );
   row    += 5;
   lb_equiscns->setMaximumHeight( lb_prjname->height() );

   QLabel*      lb_mfitinfo  = us_banner(     tr( "Model and Fitting"
                                                  " Information:" ) );
                pb_selModel  = us_pushbutton( tr( "Select Model"        ) );
                pb_modlCtrl  = us_pushbutton( tr( "Model Control"         ) );
                pb_fitcntrl  = us_pushbutton( tr( "Fitting Control"     ) );
                pb_loadFit   = us_pushbutton( tr( "Load Fit"            ) );
                pb_monCarlo  = us_pushbutton( tr( "Monte Carlo"         ) );

   pb_selModel->setEnabled( false );
   pb_modlCtrl->setEnabled( false );
   pb_fitcntrl->setEnabled( false );
   pb_monCarlo->setEnabled( false );

   connect( pb_selModel, SIGNAL( clicked()       ),
                         SLOT(   select_model()    ) );
   connect( pb_modlCtrl, SIGNAL( clicked()       ),
                         SLOT(   model_control()   ) );
   connect( pb_fitcntrl, SIGNAL( clicked()       ),
                         SLOT(   fitting_control() ) );
   connect( pb_loadFit,  SIGNAL( clicked()       ),
                         SLOT(   load_fit()        ) );
   connect( pb_monCarlo, SIGNAL( clicked()       ),
                         SLOT(   monte_carlo()     ) );

   row     = 0;
   modlFitLayout->addWidget( lb_mfitinfo, row++, 0, 1, 2 );
   modlFitLayout->addWidget( pb_selModel, row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_modlCtrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_loadFit,  row,   0, 1, 1 );
   modlFitLayout->addWidget( pb_fitcntrl, row++, 1, 1, 1 );
   modlFitLayout->addWidget( pb_monCarlo, row,   0, 1, 1 );

   QLabel*      lb_parminfo  = us_banner( tr( "Parameter Information:"    ) );
                pb_floatPar  = us_pushbutton( tr( "Float Parameters"      ) );
                pb_initPars  = us_pushbutton( tr( "Initialize Parameters" ) );
   QPushButton* pb_help      = us_pushbutton( tr( "Help"                  ) );
   QPushButton* pb_close     = us_pushbutton( tr( "Close"                 ) );
   QLabel*      lb_scselect  = us_label(      tr( "Scan Selector:"      ) );
                ct_scselect  = us_counter( 2, 0, 50, 1 );

   pb_floatPar->setEnabled( false );
   pb_initPars->setEnabled( false );
   ct_scselect->setStep(  1.0 );
   ct_scselect->setValue( 0.0 );

   connect( pb_floatPar, SIGNAL( clicked()              ),
                         SLOT(   float_params()         ) );
   connect( pb_initPars, SIGNAL( clicked()              ),
                         SLOT(   init_params()          ) );
   connect( pb_help,     SIGNAL( clicked()              ),
                         SLOT(   help()                 ) );
   connect( pb_close,    SIGNAL( clicked()              ),
                         SLOT(   close_all()            ) );
   connect( ct_scselect, SIGNAL( valueChanged( double ) ),
                         SLOT(   scan_select(  double ) ) );

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
   leftLayout->setStretchFactor( scnListLayout, 10 );
   leftLayout->setStretchFactor( modlFitLayout,  1 );


   // Right Column

   // Simulation plot

   QBoxLayout* plot = new US_Plot( equil_plot, 
         tr( "Experiment Equilibrium Data" ),
         tr( "Radius (cm)" ),
         tr( "Absorbance (280 nm)" ) );
   us_grid( equil_plot );
   
   equil_plot->setMinimumSize( 600, 400 );
   equil_plot->setAxisScale( QwtPlot::yLeft  , 0.1, 0.601 );
   equil_plot->setAxisScale( QwtPlot::xBottom, 5.9, 6.2 );

   QLabel*      lb_status    = us_label(    tr( "Status/Information:" ) );
                le_status    = us_lineedit( tr( "Please select an edited"
                                                " Equilibrium Dataset with"
                                                " \"Load Experiment\"" ) );
   QLabel*      lb_currmodl  = us_label(    tr( "Current Model:"      ) );
                le_currmodl  = us_lineedit( tr( "-- none selected --" ) );
   QLabel*      lb_mxfringe  = us_label(    tr( "Max. OD/Fringe:"     ) );
                le_mxfringe  = us_lineedit( tr( "0.90" ) );
   QLabel*      lb_mxfnotes  = us_label(    tr( "(set to zero to inactivate"
                                                " high conc. limits)"  ) );
   le_status  ->setAlignment( Qt::AlignCenter );
   le_currmodl->setAlignment( Qt::AlignCenter );

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
qDebug() << "LOAD()";
pb_details ->setEnabled( true );
pb_view    ->setEnabled( true );
pb_unload  ->setEnabled( true );
pb_scdiags ->setEnabled( true );
pb_ckscfit ->setEnabled( true );
pb_conchist->setEnabled( true );
pb_resetsl ->setEnabled( true );
pb_selModel->setEnabled( true );
pb_modlCtrl->setEnabled( true );
pb_fitcntrl->setEnabled( true );
pb_loadFit ->setEnabled( true );
pb_monCarlo->setEnabled( true );
pb_floatPar->setEnabled( true );
pb_initPars->setEnabled( true );

   excludedScans.clear();
   dataList     .clear();
   rawList      .clear();
   triples      .clear();

   dataLoaded  = false;
   buffLoaded  = false;
   dataLatest  = ck_edlast->isChecked();
   dbdisk      = dkdb_cntrls->db() ? US_Disk_DB_Controls::DB
                                   : US_Disk_DB_Controls::Disk;

   US_DataLoader* dialog = new US_DataLoader( dataLatest, dbdisk, rawList,
         dataList, triples, workingDir );

   connect( dialog, SIGNAL( changed(      bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   if ( dialog->exec() != QDialog::Accepted )  return;

   if ( dkdb_cntrls->db() )
      workingDir = tr( "(database)" );

   else
   {
      workingDir = workingDir.section( workingDir.left( 1 ), 4, 4 );
      workingDir = workingDir.left( workingDir.lastIndexOf( "/" ) );
   }
qDebug() << " workingDir" << workingDir;
qDebug() << " dataList size" << dataList.size();
//for ( int jj=0;jj<triples.size();jj++ ) qDebug() << "  " << triples[jj];

   qApp->processEvents();

   int nscans = 0;

   for ( int jd = 0; jd < dataList.size(); jd++ )
      nscans += dataList[ jd ].speedData.size();

   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize(),
         QFont::Bold );
   QFontMetrics fm( font );
   int fontHeight = fm.lineSpacing();
   int rowHght    = fontHeight;
   tw_equiscns->setMaximumHeight( fontHeight * 60 + 12 );
   tw_equiscns->setRowCount( nscans );
   QString hdr1 = tr( "Scan" );
   QString hdr2 = tr( "Cell/Chan/WaveL" );
   QString hdr3 = tr( "Speed (RPM)" );
   QString hdr4 = tr( "CCW Scan" );
   int     whd1 = fm.width( hdr1 + " " );
   int     whd2 = fm.width( hdr2 + " " );
   int     whd3 = fm.width( hdr3 + " " );
   int     whd4 = fm.width( hdr4 + " " );
   QStringList headers;

   headers << "" << hdr1 << hdr2 << hdr3 << hdr4;
   tw_equiscns->setHorizontalHeaderLabels( headers );
   tw_equiscns->verticalHeader()->hide();
   tw_equiscns->setShowGrid( false );
   tw_equiscns->setSelectionBehavior( QAbstractItemView::SelectRows );
   tw_equiscns->setColumnWidth( 0, whd1 );
   tw_equiscns->setColumnWidth( 1, whd1 );
   tw_equiscns->setColumnWidth( 2, whd2 );
   tw_equiscns->setColumnWidth( 3, whd3 );
   tw_equiscns->setColumnWidth( 4, whd4 );
   tw_equiscns->setMinimumWidth(  400 );
   tw_equiscns->setMinimumHeight( 200 );

   int jsscn = 0;

   for ( int jd = 0; jd < dataList.size(); jd++ )
   {
      edata          = &dataList[ jd ];
      QString triple = triples[ jd ];

      for ( int jr = 0; jr < edata->speedData.size(); jr++, jsscn++ )
      {
         QTableWidgetItem* item;
         double  drpm  = edata->speedData[ jr ].speed;
         int     iscn  = edata->speedData[ jr ].first_scan;
         int     lscn  = iscn + edata->speedData[ jr ].scan_count - 1;
         QString ascn  = QString::number( iscn );

         if ( lscn != iscn )
            ascn       = ascn + " - " + QString::number( lscn );

         item = new QTableWidgetItem( blue_arrow, "" );
//if(iscn==3) item=new QTableWidgetItem( green_arrow, "" );
//if(iscn==5) item=new QTableWidgetItem( red_arrow, "" );
         tw_equiscns->setItem( jsscn, 0, item );

         item = new QTableWidgetItem( QString::number( jsscn + 1 ) );
         item->setFlags( item->flags() ^ Qt::ItemIsEditable );
         tw_equiscns->setItem( jsscn, 1, item );

         item = new QTableWidgetItem( triple );
         item->setFlags( item->flags() ^ Qt::ItemIsEditable );
         tw_equiscns->setItem( jsscn, 2, item );

         item = new QTableWidgetItem( QString::number( drpm ) );
         item->setFlags( item->flags() ^ Qt::ItemIsEditable );
         tw_equiscns->setItem( jsscn, 3, item );

         item = new QTableWidgetItem( ascn );
         item->setFlags( item->flags() ^ Qt::ItemIsEditable );
         tw_equiscns->setItem( jsscn, 4, item );

         tw_equiscns->setRowHeight( jsscn, rowHght );
      }
   }

   connect( tw_equiscns, SIGNAL( itemClicked( QTableWidgetItem* ) ),
            this,        SLOT(   clickedItem( QTableWidgetItem* ) ) );

   le_status->setText( tr( "To edit scan limits:  click, move, and "
                           "release mouse in the plot area." ) );

   edata       = &dataList[ 0 ];
qDebug() << "eData readings size" << edata->scanData[0].readings.size();
   dataLoaded  = true;
}

void US_GlobalEquil::details( void )
{
qDebug() << "DETAILS()";
   QString runID  = edata->runID;

   US_RunDetails2 dialog( rawList, runID, workingDir, triples );
   dialog.exec();
   qApp->processEvents();
}

void US_GlobalEquil::view_report( void )
{ qDebug() << "VIEW_REPORT()"; }
void US_GlobalEquil::unload( void )
{ qDebug() << "UNLOAD()"; }
void US_GlobalEquil::scan_diags( void )
{ qDebug() << "SCAN_DIAGS()"; }
void US_GlobalEquil::check_scan_fit( void )
{ qDebug() << "CHECK_SCAN_FIT()"; }
void US_GlobalEquil::conc_histogram( void )
{ qDebug() << "CONC_HISTOGRAM()"; }
void US_GlobalEquil::reset_scan_lims( void )
{ qDebug() << "RESET_SCAN_LIMS()"; }
void US_GlobalEquil::load_model( void )
{ qDebug() << "LOAD_MODEL()"; }
void US_GlobalEquil::new_project_name( const QString& newpname )
{ qDebug() << "NEW_PROJECT_NAME()" << newpname; }
void US_GlobalEquil::select_model( void )
{ qDebug() << "SELECT_MODEL()"; }
void US_GlobalEquil::model_control( void )
{ qDebug() << "MODEL_CONTROL()"; }
void US_GlobalEquil::fitting_control( void )
{ qDebug() << "FITTING_CONTROL()"; }
void US_GlobalEquil::load_fit( void )
{ qDebug() << "LOAD_FIT()"; }
void US_GlobalEquil::monte_carlo( void )
{ qDebug() << "MONTE_CARLO()"; }
void US_GlobalEquil::float_params( void )
{ qDebug() << "FLOAT_PARAMS()"; }
void US_GlobalEquil::init_params( void )
{ qDebug() << "INIT_PARAMS()"; }
void US_GlobalEquil::scan_select( double newscan )
{ qDebug() << "SCAN_SELECT()" << newscan; }

void US_GlobalEquil::close_all( void )
{
qDebug() << "CLOSE_ALL()";
   close();
}

void US_GlobalEquil::update_disk_db( bool dbaccess )
{
   if ( dbaccess )
   {
      dkdb_cntrls->set_db();
      dbdisk    = US_Disk_DB_Controls::DB;
   }

   else
   {
      dkdb_cntrls->set_disk();
      dbdisk    = US_Disk_DB_Controls::Disk;
   }
}

void US_GlobalEquil::clickedItem( QTableWidgetItem* item )
{
   int row = item->row();
//qDebug() << "TableItemClicked row col" << row << item->column();

   QTableWidget* tabw = item->tableWidget();

   QString triple = tabw->item( row, 2 )->text();
   double  drpm   = tabw->item( row, 3 )->text().toDouble();
//qDebug() << " Clicked: triple" << triple << "rpm" << drpm;
   int     jdx    = -1;
   int     jsx    = -1;

   bool    found  = findData( triple, drpm, jdx, jsx );
//qDebug() << " Clicked:   found" << found << " jdx jsx" << jdx << jsx;

   if ( found )  edata_plot();
}

bool US_GlobalEquil::findData( QString trip, double drpm, int& jdx, int& jsx )
{
   bool found = false;
   jdx    = -1;
   jsx    = -1;

   while ( ++jdx < dataList.size() )
   {
      jsx  = -1;

      if ( trip == triples[ jdx ] )
      {
         while ( ++jsx < dataList[ jdx ].speedData.size() )
         {
            if ( dataList[ jdx ].speedData[ jsx ].speed == drpm )
            {
               edata  = &dataList[ jdx ];
               spdata = &edata->speedData[ jsx ];
               found  = true;
               break;
            }
         }
      }

      if ( found )  break;
   }

   return found;
}

void US_GlobalEquil::edata_plot()
{
   int     npoints = edata->x.size();
   int     iscan   = spdata->first_scan;
   int     kscan   = spdata->scan_count;
   double  drpm    = spdata->speed;
   double  radl    = spdata->dataLeft;
   double  radr    = spdata->dataRight;
   QString runID   = edata->runID;
   QString editID  = edata->editID;
   QString cell    = edata->cell;
   QString chan    = edata->channel;
   QString waveln  = edata->wavelength;

   equil_plot->detachItems();
   equil_plot->setTitle(
      tr( "Run: " ) + runID + tr( "  Edit: " ) + editID + "\n" +
      tr( "Cell: " ) + cell + tr( "  Channel: " ) + chan +
      tr( "  Speed: " ) + QString::number( drpm ) + tr( " rpm" ) );
   equil_plot->setAxisTitle( QwtPlot::yLeft,
      tr( "Absorbance (" ) + waveln + tr( " nm)" ) );

   QwtPlotGrid* grid = us_grid( equil_plot );
   grid->enableYMin( true );
   grid->enableY   ( true );
   grid->setMajPen( QPen( US_GuiSettings::plotMajGrid(), 0, Qt::DashLine ) );
   grid->setMinPen( QPen( US_GuiSettings::plotMinGrid(), 0, Qt::DotLine  ) );

   QVector< double > rvec( npoints, 0.0 );
   QVector< double > vvec( npoints, 0.0 );
   double* ra  = rvec.data();
   double* va  = vvec.data();

   int jsc     = iscan - 1;
   int count   = 0;
   double rlo  = 9e+10;
   double rhi  = -9e+10;
   double vlo  = 9e+10;
   double vhi  = -9e+10;

   for ( int ii = jsc; ii < jsc + kscan; ii++ )
   {
      for ( int jj = 0; jj < npoints; jj++ )
      {
         double rv = edata->radius( jj );

         if ( rv >= radl  &&  rv <= radr )
         {
            double vv     = edata->value( ii, jj );
            ra[ count   ] = rv;
            va[ count++ ] = vv;
            rlo           = min( rlo, rv );
            rhi           = max( rhi, rv );
            vlo           = min( vlo, vv );
            vhi           = max( vhi, vv );
         }
      }
   }

   double rpad = ( rhi - rlo ) * 0.05;
   double vpad = ( vhi - vlo ) * 0.05;
   rlo -= rpad;
   rhi += rpad;
   vlo -= vpad;
   vhi += vpad;

   equil_plot->setAxisScale( QwtPlot::xBottom, rlo, rhi );
   equil_plot->setAxisScale( QwtPlot::yLeft,   vlo, vhi );
   QwtPlotCurve* curve = us_curve( equil_plot, "Equil Data" );
   QwtSymbol sym;
   sym.setStyle( QwtSymbol::Ellipse );
   sym.setPen  ( QPen( Qt::blue ) );
   sym.setBrush( QBrush( Qt::yellow ) );
   sym.setSize ( 10 );
   curve->setStyle( QwtPlotCurve::NoCurve );
   curve->setSymbol( sym );
   curve->setData( ra, va, count );

   equil_plot->replot();

}

