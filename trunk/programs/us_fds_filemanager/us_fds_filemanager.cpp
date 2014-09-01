//! \file us_buoyancy.cpp

#include <QApplication>
#include <QDomDocument>

#include "us_fds_filemanager.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_FDS_FileManager. This program is intended to help the 
//user to filter useful files from FDS runs where multiple gains and speeds were collected.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_FDS_FileManager w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_FDS_FileManager::US_FDS_FileManager() : US_Widgets()
{
   setWindowTitle( tr( "Fluorescence Data File Manager" ) );
   setPalette( US_GuiSettings::frameColor() );

   QVBoxLayout* top = new QVBoxLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   // Put the Run Info across the entire window
   QHBoxLayout* runInfo = new QHBoxLayout();
   QLabel* lb_info = us_label( tr( "Dataset Info:" ), -1 );
   runInfo->addWidget( lb_info );

   le_info = us_lineedit( "", 1, true );
   runInfo->addWidget( le_info );

   top->addLayout( runInfo );

   QHBoxLayout* main = new QHBoxLayout();
   QVBoxLayout* left = new QVBoxLayout;

   // Start of Grid Layout
   QGridLayout* specs = new QGridLayout;
   int s_row = 0;

   // Row 1
   // Investigator

   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 1 );
   le_directory = us_lineedit( "" );
   specs->addWidget( le_directory, s_row++, 1, 1, 3 );

   QLabel* lbl_progress = us_label( tr( "Progress:" ), -1 );
   lbl_progress->setAlignment( Qt::AlignLeft );
   specs->addWidget( lbl_progress, s_row, 0, 1, 1 );

   progress = us_progressBar( 0, 100, 0 );
   specs->addWidget( progress, s_row++, 1, 1, 3 );

   QLabel* lb_triple = us_label( tr( "Cell / Channel / Wavelength:" ), -1 );
   specs->addWidget( lb_triple, s_row, 0, 1, 2 );

   cb_triple = us_comboBox();
   specs->addWidget( cb_triple, s_row++, 2, 1, 2 );

   lbl_rpms = us_label( tr( "Speed Step (RPM) of triple:" ), -1 );
   specs->addWidget( lbl_rpms,   s_row,   0, 1, 2 );

   cb_rpms = us_comboBox();
   specs->addWidget( cb_rpms,   s_row++, 2, 1, 2 );

   lbl_gains = us_label( tr( "Gain Setting::" ), -1 );
   specs->addWidget( lbl_gains,   s_row,   0, 1, 2 );

   cb_gains = us_comboBox();
   specs->addWidget( cb_gains,   s_row++, 2, 1, 2 );

   // Scans focus from:
   lbl_from = us_label( tr( "Scan Focus from:" ), 0 );
   lbl_from->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lbl_from,   s_row,   0, 1, 2 );

   ct_from = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_from->setStep( 1 );
   specs->addWidget( ct_from,   s_row++, 2, 1, 2 );
   connect( ct_from, SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_from   ( double ) ) );

   // Scan focus to
   lbl_to = us_label( tr( "Scan Focus to:" ), 0 );
   lbl_to->setAlignment( Qt::AlignVCenter | Qt::AlignRight );
   specs->addWidget( lbl_to,   s_row,   0, 1, 2 );

   ct_to = us_counter ( 3, 0.0, 0.0 ); // Update range upon load
   ct_to->setStep( 1 );
   specs->addWidget( ct_to,   s_row++, 2, 1, 2 );
   connect( ct_to  , SIGNAL( valueChanged ( double ) ),
                     SLOT  ( focus_to     ( double ) ) );

   // Exclude and Include pushbuttons
   pb_exclude = us_pushbutton( tr( "Exclude Scan(s)" ), false );
   specs->addWidget( pb_exclude,   s_row, 0, 1, 2 );
   pb_include = us_pushbutton( tr( "Include All"     ), false );
   specs->addWidget( pb_include,   s_row++, 2, 1, 2 );
   pb_include ->setEnabled( false );
   connect( pb_exclude,     SIGNAL( clicked()          ),
                            SLOT(   exclude_scans()    ) );
   connect( pb_include,     SIGNAL( clicked()  ),
                            SLOT(   include_scans()  ) );


   // Button rows
   QBoxLayout* buttons = new QHBoxLayout;

/*
   QPushButton* pb_load = us_pushbutton( tr( "Load Data" ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );
   specs->addWidget( pb_load, s_row, 0, 1, 2 );
*/
   pb_save = us_pushbutton( tr( "Save selected files" ), false );
   connect( pb_save, SIGNAL( clicked() ), SLOT( save() ) );
   specs->addWidget( pb_save, s_row++, 2, 1, 2 );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   buttons->addWidget( pb_reset );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   buttons->addWidget( pb_help );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   buttons->addWidget( pb_accept );

   // Plot layout on right side of window
   plot = new US_Plot( data_plot,
         tr( "Absorbance Data" ),
         tr( "Radius (in cm)" ), tr( "Absorbance" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );


   left->addLayout( specs );
   left->addStretch();
   left->addLayout( buttons );

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( plot, 3 );
   top ->addLayout( main );

   reset();
}

// Load an AUC data set
void US_FDS_FileManager::load( void )
{
   reset();
   QFileDialog fd;
   fd.setFileMode(QFileDialog::Directory);
   source_dir = fd.getExistingDirectory( this,
         tr( "Raw Data Directory" ),
         US_Settings::importDir(),
         QFileDialog::DontResolveSymlinks);

   source_dir.replace( "\\", "/" );

   if ( source_dir.isEmpty() )      // If no directory chosen, return now
   {
      return;
   }
   else
   {
      le_directory->setText(source_dir);
   }

   QDir readDir( source_dir, "*.[F,f][I,i][1-8]", QDir::Name, QDir::Files | QDir::Readable );
   readDir.makeAbsolute();
   if ( source_dir.right( 1 ) != "/" ) source_dir += "/";  // Ensure trailing "/"
   files = QDir( readDir ).entryList(
         QDir::Files | QDir::NoDotAndDotDot, QDir::Name );
   qDebug() << "calling  parse_files()";
   parse_files();
}

void US_FDS_FileManager::parse_files( void )
{
   cb_triple->disconnect();
   cb_triple->clear();
   scaninfo.clear();
   QString line, str, str1, str2;
   QStringList tokens, triplelist;
   int i;
   triplelist.clear();
   qDebug() << "starting to load " << files.size() << " files...";
   for ( i=0; i<files.size(); i++)
   {
      tmp_scaninfo.filename = files.at(i);
      if ( files.at(i).contains( "A", Qt::CaseInsensitive ) )
      {
         tmp_scaninfo.channel  = "A";
      }
      else
      {
         tmp_scaninfo.channel  = "B";
      }
      QFile f(source_dir + tmp_scaninfo.filename);
      if ( f.open( QFile::ReadOnly ) )
      {
         QTextStream ts(&f);
         line = ts.readLine();
         tokens = line.split(" ", QString::SkipEmptyParts);
         tmp_scaninfo.date    = tokens.at(0);
         tmp_scaninfo.time    = tokens.at(1) + " " + tokens.at(2);
         tmp_scaninfo.voltage = tokens.at(4).toInt();
         tmp_scaninfo.gain    = tokens.at(6).toInt();
         tmp_scaninfo.range   = tokens.at(8).toInt();
         tmp_scaninfo.include = true;
         tokens.clear();
         line = ts.readLine();
         tokens = line.split(" ", QString::SkipEmptyParts);
         tmp_scaninfo.cell    = tokens.at(1).toInt();
         tmp_scaninfo.rpm     = tokens.at(3).toInt();
         tmp_scaninfo.seconds = tokens.at(4).toDouble();
         tmp_scaninfo.omega_s = tokens.at(4).toDouble();
         tmp_scaninfo.lambda  = tokens.at(6).toInt();
         tmp_scaninfo.triple  = str1.setNum(tmp_scaninfo.cell) + " / " +
                                tmp_scaninfo.channel + " / " +
                                str2.setNum(tmp_scaninfo.lambda);
         tmp_scaninfo.gainset  = "V: " + str1.setNum(tmp_scaninfo.voltage) + ", G: " +
                                str.setNum(tmp_scaninfo.gain) + ", R: " +
                                str2.setNum(tmp_scaninfo.range);
         tmp_scaninfo.include = true;
         tmp_scaninfo.x.clear();
         tmp_scaninfo.y.clear();
         while (!ts.atEnd())
         {
            line = ts.readLine();
            tokens.clear();
            tokens = line.split(" ", QString::SkipEmptyParts);
            tmp_scaninfo.x.append(tokens.at(0).toDouble() );
            tmp_scaninfo.y.append(tokens.at(1).toDouble() );
         }
      }
      triplelist.append(tmp_scaninfo.triple);
      progress->setValue( 100 * ( i+1.0 )/files.size() );
      scaninfo.append(tmp_scaninfo);
   }
   triplelist.removeDuplicates();
   triplelist.sort();
   cb_triple->addItems( triplelist );
   connect( cb_triple, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( select_triple ( int ) ) );
   qDebug() << "calling  select_triple(" << cb_triple->currentIndex() << ")";
   select_triple(cb_triple->currentIndex());
}

// Select a new triple
void US_FDS_FileManager::select_triple( int index )
{
   cb_rpms->disconnect();
   cb_rpms->clear();
   current_triple = index;
   QStringList rpmlist;
   rpmlist.clear();
   QString str;
   for (int i=0; i<scaninfo.size(); i++)
   {
      if(scaninfo[i].triple == cb_triple->currentText() && scaninfo[i].include )
      {
         rpmlist.append(str.setNum(scaninfo[i].rpm));
      }
   }
   rpmlist.removeDuplicates();
   rpmlist.sort();
   cb_rpms->addItems(rpmlist);
   connect( cb_rpms, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( select_rpm ( int ) ) );
   qDebug() << "calling  select_rpm(" << cb_rpms->currentIndex() << ")";
   select_rpm(cb_rpms->currentIndex());
}

// Select a new speed within a triple
void US_FDS_FileManager::select_rpm( int index )
{
   cb_gains->disconnect();
   cb_gains->clear();
   current_rpm = index;
   QStringList gainlist;
   gainlist.clear();
   QString str1;
   gainlist.clear();
   for (int i=0; i<scaninfo.size(); i++)
   {
      if(scaninfo[i].triple == cb_triple->currentText()
         && str1.setNum( scaninfo[i].rpm ) == cb_rpms->currentText() 
         && scaninfo[i].include )
      {
         gainlist.append(scaninfo[i].gainset);
      }
   }
   gainlist.removeDuplicates();
   gainlist.sort();
   cb_gains->addItems(gainlist);
   connect( cb_gains, SIGNAL( currentIndexChanged( int ) ),
                       SLOT  ( select_gain ( int ) ) );
   select_gain(cb_gains->currentIndex());
}

void US_FDS_FileManager::select_gain( int index )
{
   current_gain = index;
   plot_scans();
}

// Plot a single scan curve
void US_FDS_FileManager::plot_scans( void )
{
   qDebug() << "entering plot_scans";
   QString str1;
   plotindex.clear();
   data_plot->clear();
   for (int i=0; i<scaninfo.size(); i++)
   {
      if (scaninfo[i].triple    == cb_triple->currentText()
         && scaninfo[i].rpm     == cb_rpms->currentText().toInt()
         && scaninfo[i].gainset == cb_gains->currentText()
         && scaninfo[i].include )
      {
         plotindex.append(i);
      }
   }
   int npts;
   QList < QwtPlotCurve * > c;
   QwtPlotCurve *curve;
   c.clear();
   for (int i=0; i<plotindex.size(); i++)
   {
      npts = scaninfo[plotindex[i]].x.size();
      double x[npts], y[npts];
      for (int j=0; j<npts; j++)
      {
         x[j] = scaninfo[plotindex[i]].x.at(j);
         y[j] = scaninfo[plotindex[i]].y.at(j);
      }
      str1.setNum( i+1 );
      qDebug() << "str1, i: " << str1 << i;
      curve = us_curve( data_plot, "Scan " + str1 );
      curve->setData( x, y, npts );
      c.append( curve );
/*      if (i == current_scan - 1 )
      { // set the temperature to the currently highlighted scan:
         tmp_dpoint.temperature = s->temperature;
         c->setPen( QPen( Qt::red ) );
      }
*/
      data_plot->setAxisAutoScale( QwtPlot::yLeft );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }
//   ct_selectScan->setMinValue( 1 );
//   ct_selectScan->setMaxValue( maxscan );
   data_plot->replot();
}

// Reset parameters to their defaults
void US_FDS_FileManager::reset( void )
{
   le_info     ->setText( "" );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.7, 7.3 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 1.5 );
   data_plot->replot();

   pb_save->setEnabled( false );
   ct_from->disconnect();
   ct_from->setMinValue( 0 );
   ct_from->setMaxValue( 0 );
   ct_from->setValue   ( 0 );

   ct_to   ->disconnect();
   ct_to   ->setMinValue( 0 );
   ct_to   ->setMaxValue( 0 );
   ct_to   ->setValue   ( 0 );


   scaninfo  .clear();
   cb_triple->clear();
   cb_rpms  ->clear();
   cb_gains ->clear();
}

void US_FDS_FileManager::include_scans( void )
{
}

void US_FDS_FileManager::exclude_scans( void )
{
}

void US_FDS_FileManager::focus_from( double scan )
{
   int from = (int)scan;
   int to   = (int)ct_to->value();

   if ( from > to )
   {
      ct_to->disconnect();
      ct_to->setValue( scan );
      to = from;

      connect( ct_to, SIGNAL( valueChanged ( double ) ),
                      SLOT  ( focus_to     ( double ) ) );
   }

   focus( from, to );
}

void US_FDS_FileManager::focus_to( double scan )
{
   int to   = (int)scan;
   int from = (int)ct_from->value();

   if ( from > to )
   {
      ct_from->disconnect();
      ct_from->setValue( scan );
      from = to;

      connect( ct_from, SIGNAL( valueChanged ( double ) ),
                        SLOT  ( focus_from   ( double ) ) );
   }

   focus( from, to );
}

void US_FDS_FileManager::focus( int from, int to )
{
   if ( from == 0 && to == 0 )
   {
      pb_exclude->setEnabled( false );
      pb_include->setEnabled( false );
   }
   else
   {
      pb_exclude->setEnabled( true );
      pb_include->setEnabled( true );
   }

   QList< int > focus;  // We don't care if -1 is in the list
   for ( int i = from - 1; i <= to - 1; i++ ) focus << i;

   //set_colors( focus );

}

void US_FDS_FileManager::save( void )
{
}
