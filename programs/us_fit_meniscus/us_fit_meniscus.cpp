//! \file us_fit_meniscus.cpp
//#include <QApplication>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include "us_fit_meniscus.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_settings.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_investigator.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_model.h"
#include "us_noise.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setSymbol(a)       setSymbol(*a)
#endif

//US_FitMeniscus::US_FitMeniscus( QString auto_mode ) : US_Widgets()
US_FitMeniscus::US_FitMeniscus( QMap<QString, QString> triple_info_map ) : US_Widgets()
{
  this->triple_information = triple_info_map;
  qDebug() << "triple_information: filename, triple_name: " <<  triple_information["filename"] << triple_information["triple_name"];
  
  auto_mode = true;
  no_fm_data = false;
  bad_men_vals = false;
  
  setWindowTitle( tr( "Fit Meniscus from 2DSA Data: ") + triple_information[ "triple_name" ] );
  setPalette( US_GuiSettings::frameColor() );
   dbg_level    = US_Settings::us_debug();
   have3val     = true;

   // Main layout
   QBoxLayout*  mainLayout   = new QVBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Component layouts
   QHBoxLayout* topLayout    = new QHBoxLayout;
   QHBoxLayout* bottomLayout = new QHBoxLayout;
   QGridLayout* leftLayout   = new QGridLayout;
   QGridLayout* rightLayout  = new QGridLayout;
   QGridLayout* cntrlsLayout = new QGridLayout;
DbgLv(1) << "Main: AA";

   // Lay out the meniscus,rmsd text box
//   te_data = new US_Editor( US_Editor::LOAD, false,
//         "results/*-fm*.fit*dat;;*.dat;;*.*" );

 
   te_data = new US_Editor( US_Editor::LOAD, false,
         "results/*.dat;;*.fitmen.dat;;*.fitbot.dat;;*.*" );
   connect( te_data, SIGNAL( US_EditorLoadComplete( QString ) ), 
                     SLOT  ( file_loaded(           QString ) ) );

   te_data->edMenuBar->hide();
   
   QFontMetrics fm( te_data->e->font() ); 

   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setMinimumWidth ( fm.width( "123 :  6.34567, 7.34567, 0.00567890  0.012345" ) );
   te_data->e->setToolTip( tr( "Loaded, editable meniscus,rmsd table" ) );

   leftLayout->addWidget( te_data, 0, 0, 20, 1 );

   // Lay out the plot
//   QBoxLayout* plot = new US_Plot( meniscus_plot, 
//         tr( "Meniscus Fit" ),
//         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus,Bottom Fit" ),
         tr( "Distance from Base Radii" ),
         tr( "2DSA Meniscus,Bottom RMSD" ) );
   
DbgLv(1) << "Main: BB";
   QwtPlotPicker* pick = new US_PlotPicker( meniscus_plot );
   QwtPlotGrid*   grid = us_grid( meniscus_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   //connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
   //         this, SLOT(   new_value( const QwtDoublePoint& ) ) );

   //Mouse controls
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );
   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
   
   grid->attach( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );
   meniscus_plot->setToolTip( tr( "Fitted meniscus,rmsd plot" ) );

   rightLayout->addLayout( plot, 0, 1, 20, 1 );

   // Lay out the controls
   QLabel* lb_status    = us_label( tr( "Status:"    ) );
   
   le_status    = us_lineedit( tr( "No data loaded" ), -1, true );
   le_status->setToolTip(
         tr( "Results of the last action performed" ) );

   lb_zfloor     = us_label( tr( "Z Visibility Percent:" ) );
   ct_zfloor     = us_counter( 1, 50.0, 150.0, 1.0 );
   ct_zfloor->setSingleStep( 1 );
   ct_zfloor->setValue( 100.0 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( plot_data()            ) );


   lb_order     = us_label( tr( "Fit Order:" ) );
   lb_men_sel   = us_label( tr( "Meniscus selected:" ) );
   lb_rms_error = us_label( tr( "RMS Error:" ) );
   lb_men_lor   = us_label( tr( "Low-RMSD Meniscus:" ) );
   lb_bot_lor   = us_label( tr( "Low-RMSD Bottom:" ) );
   lb_men_fit   = us_label( tr( "Fit Meniscus:" ) );
   lb_bot_fit   = us_label( tr( "Fit Bottom:" ) );
   lb_mprads    = us_label( tr( "Mid-Point Radii:" ) );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   sb_order->setToolTip( tr( "Order of fitting curve" ) );

   le_men_lor   = us_lineedit( "", -1, true );
   le_men_lor->setToolTip(
         tr( "Selected-minimum meniscus radius value" ) );
   le_bot_lor   = us_lineedit( "", -1, true );
   le_bot_lor->setToolTip(
         tr( "Selected-minimum bottom radius value" ) );
   le_men_fit   = us_lineedit( "", -1, false );
   le_men_fit->setToolTip(
         tr( "Fit/Editable meniscus radius value" ) );
   le_bot_fit   = us_lineedit( "", -1, false );
   le_bot_fit->setToolTip(
         tr( "Fit/Editable bottom radius value" ) );
   le_mprads    = us_lineedit( "", -1, false );
   le_mprads ->setToolTip(
         tr( "Meniscus,Bottom current radii (midpoint of ranges)" ) );


   if ( auto_mode )
   {
      le_men_sel   = us_lineedit( "", -1, true );
      le_men_sel->setToolTip(
            tr( "Selected meniscus radius value" ) );
   }
   else
   {
      le_men_sel   = us_lineedit( "", -1, false );
      le_men_sel->setToolTip(
            tr( "Selected/Editable meniscus radius value" ) );
   }
   
   le_rms_error = us_lineedit( "", -1, true );
   le_rms_error->setToolTip(
         tr( "RMS error of curve to meniscus,rmsd points" ) );

   connect( sb_order, SIGNAL( valueChanged( int ) ),
            this,     SLOT( plot_data( int ) ) );

   dkdb_cntrls            = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool )        ),
            this,        SLOT(   update_disk_db( bool ) ) );

   pb_update = us_pushbutton( tr( "Update Edit" ) );
   connect( pb_update, SIGNAL( clicked() ),
            this,      SLOT( edit_update() ) );
   pb_update->setEnabled( false );
   pb_update->setToolTip(
         tr( "Update edit record with meniscus; remove non-chosen models" ) );

   pb_scandb = us_pushbutton( tr( "Scan Database" ) );
   connect( pb_scandb, SIGNAL( clicked() ),
            this,      SLOT( scan_dbase() ) );
   pb_scandb->setEnabled( dkdb_cntrls->db() );
   pb_scandb->setToolTip(
         tr( "Scan fit-meniscus models in DB; create local table files" ) );

   QPushButton*
   pb_invest    = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_invest, SIGNAL( clicked() ),
            this,      SLOT(   sel_investigator() ) );
   QString
      inv_name  = QString::number( US_Settings::us_inv_ID() )
                  + ": " + US_Settings::us_inv_name();
   le_invest    = us_lineedit( inv_name, -1, true );

   //us_checkbox( tr( "Confirm Each Update Step" ), ck_confirm,  true );
   us_checkbox( tr( "Confirm Each Update Step" ), ck_confirm,  false );
   
   us_checkbox( tr( "Apply to All Wavelengths" ), ck_applymwl, true );
//   ck_applymwl->setVisible( false );
   ck_confirm ->setToolTip(
         tr( "Pop up confirmation dialogs at each update step" ) );
   ck_applymwl->setToolTip(
         tr( "Apply the meniscus update to all wavelengths of"
             " the current cell/channel" ) );

   pb_plot   = us_pushbutton( tr( "Plot" ) );
   connect( pb_plot, SIGNAL( clicked() ),
            this,    SLOT( plot_data() ) );
   pb_plot->setToolTip(
         tr( "Plot,analyze meniscus,rmsd from current text" ) );

   pb_reset  = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ),
            this,     SLOT( reset() ) );
   pb_reset->setToolTip(
         tr( "Clear text,plot and various other controls" ) );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT( help() ) );
   pb_help->setToolTip(
         tr( "Open a dialog with detailed documentation" ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ),
            this,      SLOT( close() ) );
   pb_accept->setToolTip(
         tr( "Close this dialog and exit the program" ) );

   // Do detailed layout of the controls
   int row = 0;
   cntrlsLayout->addWidget( lb_status,    row,    0, 1,  1 );
   cntrlsLayout->addWidget( le_status,    row++,  1, 1, 15 );
   cntrlsLayout->addWidget( lb_men_sel,   row,    0, 1,  5 );
   cntrlsLayout->addWidget( le_men_sel,   row,    5, 1,  3 );
   cntrlsLayout->addWidget( lb_rms_error, row,    8, 1,  2 );
   cntrlsLayout->addWidget( le_rms_error, row,   10, 1,  2 );
   cntrlsLayout->addWidget( lb_order,     row,   12, 1,  2 );
   cntrlsLayout->addWidget( sb_order,     row++, 14, 1,  2 );

   /* to hide **/
   cntrlsLayout->addWidget( lb_men_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_men_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_men_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_men_fit,   row,    9, 1,  2 );
   cntrlsLayout->addWidget( lb_zfloor,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( ct_zfloor,    row++, 14, 1,  2 );
   cntrlsLayout->addWidget( lb_bot_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_bot_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_bot_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_bot_fit,   row,    9, 1,  2 );
   cntrlsLayout->addWidget( lb_mprads,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( le_mprads,    row++, 14, 1,  3 );
   //cntrlsLayout->addLayout( dkdb_cntrls,  row,    0, 1,  6 );

   cntrlsLayout->addWidget( pb_update,    row,    6, 1,  5 );

   /* to hide **/
   cntrlsLayout->addWidget( pb_scandb,    row++, 11, 1,  5 );
   cntrlsLayout->addWidget( pb_invest,    row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_invest,    row++,  3, 1,  5 );
   cntrlsLayout->addWidget( ck_confirm,   row,    0, 1,  8 );
   cntrlsLayout->addWidget( ck_applymwl,  row++,  8, 1,  8 );
   cntrlsLayout->addWidget( pb_plot,      row,    0, 1,  4 );
   cntrlsLayout->addWidget( pb_reset,     row,    4, 1,  4 );
   cntrlsLayout->addWidget( pb_help,      row,    8, 1,  4 );
   cntrlsLayout->addWidget( pb_accept,    row,   12, 1,  4 );


   for ( int ii = 0; ii < 16; ii++ )
      cntrlsLayout->setColumnStretch( ii, 1 );

   change_plot_type();

   // Define final layout
   topLayout   ->addLayout( leftLayout   );
   topLayout   ->addLayout( rightLayout  );
   topLayout   ->setStretchFactor( leftLayout,  1 );
   topLayout   ->setStretchFactor( rightLayout, 2 );
   bottomLayout->addLayout( cntrlsLayout );

   mainLayout  ->addLayout( topLayout    );
   mainLayout  ->addLayout( bottomLayout );
   mainLayout  ->setStretchFactor( topLayout,    2 );
   mainLayout  ->setStretchFactor( bottomLayout, 2 );

   te_data->e->setPlainText( "123 :  6.34567, 7.34567, 0.00567890" );
   te_data->adjustSize();
   adjustSize();

//   te_data->e->setPlainText( "" );
   reset();
   
   // Hide /////
   // lb_men_lor ->hide();
   // le_men_lor ->hide();
   // lb_men_fit ->hide();
   // le_men_fit ->hide();
   // lb_zfloor  ->hide();
   // ct_zfloor  ->hide();
   // lb_bot_lor ->hide();
   // le_bot_lor ->hide();
   // lb_bot_fit ->hide();
   // le_bot_fit ->hide();
   // lb_mprads  ->hide();
   // le_mprads  ->hide();
   
   pb_scandb  ->hide();   
   pb_invest  ->hide();  
   le_invest  ->hide();  
   ck_confirm ->hide();  
   ck_applymwl->hide();  
   pb_plot    ->hide();  
   pb_reset   ->hide();  
   pb_help    ->hide();  
   pb_accept  ->hide();

   // DEBUG: ScanDB, local EditProfile loaded, data_loaded
   scan_dbase_auto( triple_information );
   get_editProfile_copy( triple_information );
   file_loaded_auto( triple_information );
}

//Regular constructor
US_FitMeniscus::US_FitMeniscus() : US_Widgets()
{
   auto_mode = false;
   no_fm_data = false;
   bad_men_vals = false;
   
   setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
   setPalette( US_GuiSettings::frameColor() );
   dbg_level    = US_Settings::us_debug();
   have3val     = true;

   // Main layout
   QBoxLayout*  mainLayout   = new QVBoxLayout( this );
   mainLayout->setSpacing         ( 2 );
   mainLayout->setContentsMargins ( 2, 2, 2, 2 );

   // Component layouts
   QHBoxLayout* topLayout    = new QHBoxLayout;
   QHBoxLayout* bottomLayout = new QHBoxLayout;
   QGridLayout* leftLayout   = new QGridLayout;
   QGridLayout* rightLayout  = new QGridLayout;
   QGridLayout* cntrlsLayout = new QGridLayout;
DbgLv(1) << "Main: AA";

   // Lay out the meniscus,rmsd text box
//   te_data = new US_Editor( US_Editor::LOAD, false,
//         "results/*-fm*.fit*dat;;*.dat;;*.*" );
   te_data = new US_Editor( US_Editor::LOAD, false,
         "results/*.dat;;*.fitmen.dat;;*.fitbot.dat;;*.*" );
   connect( te_data, SIGNAL( US_EditorLoadComplete( QString ) ), 
                     SLOT  ( file_loaded(           QString ) ) );
   
   QFontMetrics fm( te_data->e->font() ); 

   te_data->setMinimumHeight( fm.height() * 20 );
   te_data->setMinimumWidth ( fm.width( "123 :  6.34567, 7.34567, 0.00567890  0.012345" ) );
   te_data->e->setToolTip( tr( "Loaded, editable meniscus,rmsd table" ) );

   leftLayout->addWidget( te_data, 0, 0, 20, 1 );

   // Lay out the plot
//   QBoxLayout* plot = new US_Plot( meniscus_plot, 
//         tr( "Meniscus Fit" ),
//         tr( "Radius" ), tr( "2DSA Meniscus RMSD Value" ) );
   QBoxLayout* plot = new US_Plot( meniscus_plot, 
         tr( "Meniscus,Bottom Fit" ),
         tr( "Distance from Base Radii" ),
         tr( "2DSA Meniscus,Bottom RMSD" ) );
   
DbgLv(1) << "Main: BB";
   QwtPlotPicker* pick = new US_PlotPicker( meniscus_plot );
   QwtPlotGrid*   grid = us_grid( meniscus_plot );
   pick->setRubberBand( QwtPicker::VLineRubberBand );
   //connect( pick, SIGNAL( moved    ( const QwtDoublePoint& ) ),
   //         this, SLOT(   new_value( const QwtDoublePoint& ) ) );

   //Mouse controls
   pick->setMousePattern( QwtEventPattern::MouseSelect1,
                          Qt::LeftButton, Qt::ControlModifier );
   connect( pick, SIGNAL( cMouseUp( const QwtDoublePoint& ) ),
                  SLOT  ( mouse   ( const QwtDoublePoint& ) ) );
   
   
   grid->attach( meniscus_plot );
   
   meniscus_plot->setMinimumSize( 400, 400 );
   meniscus_plot->setAxisScale( QwtPlot::xBottom, 5.7, 6.8 );
   meniscus_plot->setToolTip( tr( "Fitted meniscus,rmsd plot" ) );

   rightLayout->addLayout( plot, 0, 1, 20, 1 );

   // Lay out the controls
   QLabel* lb_status    = us_label( tr( "Status:"    ) );
   
   le_status    = us_lineedit( tr( "No data loaded" ), -1, true );
   le_status->setToolTip(
         tr( "Results of the last action performed" ) );

   lb_zfloor     = us_label( tr( "Z Visibility Percent:" ) );
   ct_zfloor     = us_counter( 1, 50.0, 150.0, 1.0 );
   ct_zfloor->setSingleStep( 1 );
   ct_zfloor->setValue( 100.0 );
   connect( ct_zfloor, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( plot_data()            ) );


   lb_order     = us_label( tr( "Fit Order:" ) );
   lb_men_sel   = us_label( tr( "Meniscus selected:" ) );
   lb_rms_error = us_label( tr( "RMS Error:" ) );
   lb_men_lor   = us_label( tr( "Low-RMSD Meniscus:" ) );
   lb_bot_lor   = us_label( tr( "Low-RMSD Bottom:" ) );
   lb_men_fit   = us_label( tr( "Fit Meniscus:" ) );
   lb_bot_fit   = us_label( tr( "Fit Bottom:" ) );
   lb_mprads    = us_label( tr( "Mid-Point Radii:" ) );

   sb_order = new QSpinBox();
   sb_order->setRange( 2, 9 );
   sb_order->setValue( 2 );
   sb_order->setPalette( US_GuiSettings::editColor() );
   sb_order->setToolTip( tr( "Order of fitting curve" ) );

   le_men_lor   = us_lineedit( "", -1, false );
   le_men_lor->setToolTip(
         tr( "Selected-minimum meniscus radius value" ) );
   le_bot_lor   = us_lineedit( "", -1, false );
   le_bot_lor->setToolTip(
         tr( "Selected-minimum bottom radius value" ) );
   le_men_fit   = us_lineedit( "", -1, false );
   le_men_fit->setToolTip(
         tr( "Fit/Editable meniscus radius value" ) );
   le_bot_fit   = us_lineedit( "", -1, false );
   le_bot_fit->setToolTip(
         tr( "Fit/Editable bottom radius value" ) );
   le_mprads    = us_lineedit( "", -1, false );
   le_mprads ->setToolTip(
         tr( "Meniscus,Bottom current radii (midpoint of ranges)" ) );

   if ( auto_mode )
   {
      le_men_sel   = us_lineedit( "", -1, true );
      le_men_sel->setToolTip(
            tr( "Selected meniscus radius value" ) );
   }
   else
   {
      le_men_sel   = us_lineedit( "", -1, false );
      le_men_sel->setToolTip(
            tr( "Selected/Editable meniscus radius value" ) );
   }
   
   le_rms_error = us_lineedit( "", -1, true );
   le_rms_error->setToolTip(
         tr( "RMS error of curve to meniscus,rmsd points" ) );

   connect( sb_order, SIGNAL( valueChanged( int ) ),
            this,     SLOT( plot_data( int ) ) );

   dkdb_cntrls            = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool )        ),
            this,        SLOT(   update_disk_db( bool ) ) );

   pb_update = us_pushbutton( tr( "Update Edit" ) );
   connect( pb_update, SIGNAL( clicked() ),
            this,      SLOT( edit_update() ) );
   pb_update->setEnabled( false );
   pb_update->setToolTip(
         tr( "Update edit record with meniscus; remove non-chosen models" ) );

   pb_scandb = us_pushbutton( tr( "Scan Database" ) );
   connect( pb_scandb, SIGNAL( clicked() ),
            this,      SLOT( scan_dbase() ) );
   pb_scandb->setEnabled( dkdb_cntrls->db() );
   pb_scandb->setToolTip(
         tr( "Scan fit-meniscus models in DB; create local table files" ) );

   QPushButton*
   pb_invest    = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_invest, SIGNAL( clicked() ),
            this,      SLOT(   sel_investigator() ) );
   QString
      inv_name  = QString::number( US_Settings::us_inv_ID() )
                  + ": " + US_Settings::us_inv_name();
   le_invest    = us_lineedit( inv_name, -1, true );

   us_checkbox( tr( "Confirm Each Update Step" ), ck_confirm,  true );
   us_checkbox( tr( "Apply to All Wavelengths" ), ck_applymwl, true );
//   ck_applymwl->setVisible( false );
   ck_confirm ->setToolTip(
         tr( "Pop up confirmation dialogs at each update step" ) );
   ck_applymwl->setToolTip(
         tr( "Apply the meniscus update to all wavelengths of"
             " the current cell/channel" ) );

   pb_plot   = us_pushbutton( tr( "Plot" ) );
   connect( pb_plot, SIGNAL( clicked() ),
            this,    SLOT( plot_data() ) );
   pb_plot->setToolTip(
         tr( "Plot,analyze meniscus,rmsd from current text" ) );

   pb_reset  = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ),
            this,     SLOT( reset() ) );
   pb_reset->setToolTip(
         tr( "Clear text,plot and various other controls" ) );

   QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ),
            this,    SLOT( help() ) );
   pb_help->setToolTip(
         tr( "Open a dialog with detailed documentation" ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ),
            this,      SLOT( close() ) );
   pb_accept->setToolTip(
         tr( "Close this dialog and exit the program" ) );

   // Do detailed layout of the controls
   int row = 0;
   cntrlsLayout->addWidget( lb_status,    row,    0, 1,  1 );
   cntrlsLayout->addWidget( le_status,    row++,  1, 1, 15 );
   cntrlsLayout->addWidget( lb_men_sel,   row,    0, 1,  5 );
   cntrlsLayout->addWidget( le_men_sel,   row,    5, 1,  3 );
   cntrlsLayout->addWidget( lb_rms_error, row,    8, 1,  2 );
   cntrlsLayout->addWidget( le_rms_error, row,   10, 1,  2 );
   cntrlsLayout->addWidget( lb_order,     row,   12, 1,  2 );
   cntrlsLayout->addWidget( sb_order,     row++, 14, 1,  2 );
   cntrlsLayout->addWidget( lb_men_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_men_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_men_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_men_fit,   row,    9, 1,  2 );
   cntrlsLayout->addWidget( lb_zfloor,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( ct_zfloor,    row++, 14, 1,  2 );
   cntrlsLayout->addWidget( lb_bot_lor,   row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_bot_lor,   row,    3, 1,  3 );
   cntrlsLayout->addWidget( lb_bot_fit,   row,    6, 1,  3 );
   cntrlsLayout->addWidget( le_bot_fit,   row,    9, 1,  2 );
   cntrlsLayout->addWidget( lb_mprads,    row,   11, 1,  3 );
   cntrlsLayout->addWidget( le_mprads,    row++, 14, 1,  3 );
   cntrlsLayout->addLayout( dkdb_cntrls,  row,    0, 1,  6 );
   cntrlsLayout->addWidget( pb_update,    row,    6, 1,  5 );
   cntrlsLayout->addWidget( pb_scandb,    row++, 11, 1,  5 );
   cntrlsLayout->addWidget( pb_invest,    row,    0, 1,  3 );
   cntrlsLayout->addWidget( le_invest,    row++,  3, 1,  5 );
   cntrlsLayout->addWidget( ck_confirm,   row,    0, 1,  8 );
   cntrlsLayout->addWidget( ck_applymwl,  row++,  8, 1,  8 );
   cntrlsLayout->addWidget( pb_plot,      row,    0, 1,  4 );
   cntrlsLayout->addWidget( pb_reset,     row,    4, 1,  4 );
   cntrlsLayout->addWidget( pb_help,      row,    8, 1,  4 );
   cntrlsLayout->addWidget( pb_accept,    row,   12, 1,  4 );

   for ( int ii = 0; ii < 16; ii++ )
      cntrlsLayout->setColumnStretch( ii, 1 );

   change_plot_type();

   // Define final layout
   topLayout   ->addLayout( leftLayout   );
   topLayout   ->addLayout( rightLayout  );
   topLayout   ->setStretchFactor( leftLayout,  1 );
   topLayout   ->setStretchFactor( rightLayout, 2 );
   bottomLayout->addLayout( cntrlsLayout );

   mainLayout  ->addLayout( topLayout    );
   mainLayout  ->addLayout( bottomLayout );
   mainLayout  ->setStretchFactor( topLayout,    2 );
   mainLayout  ->setStretchFactor( bottomLayout, 2 );

   te_data->e->setPlainText( "123 :  6.34567, 7.34567, 0.00567890" );
   te_data->adjustSize();
   adjustSize();

//   te_data->e->setPlainText( "" );
   reset();
}

// Clear the plot, m-r table text, and other elements
void US_FitMeniscus::reset( void )
{
//   dataPlotClear( meniscus_plot );
   meniscus_plot->replot();
   
   te_data->e   ->setPlainText( "" );
   le_men_lor   ->setText( "" );
   le_bot_lor   ->setText( "" );
   le_men_fit   ->setText( "" );
   le_bot_fit   ->setText( "" );
   le_mprads    ->setText( "" );
   sb_order     ->setValue( 2 );
   le_rms_error ->setText( "" );
   le_men_sel   ->setText( "" );
   v_meni       .clear();
   v_bott       .clear();
   v_rmsd       .clear();
}

// Plot the data
void US_FitMeniscus::plot_data( int )
{
   plot_data();
}

// Plot the data
void US_FitMeniscus::plot_data( void )
{
   if ( v_meni.count() < 1  &&
        te_data->e->toPlainText().length() > 20 ) 
      load_data();

   if ( have3val )
   {
      plot_3d();
   }
   else
   {
      plot_2d();
   }
}

// Load data from lines loaded to text box
void US_FitMeniscus::load_data()
{
   int count         = 0;
   QString contents  = te_data->e->toPlainText();
   contents.replace( QRegExp( "[^0-9eE\\.\\n\\+\\-]+" ), " " );

   QStringList lines = contents.split( "\n", QString::SkipEmptyParts );
   QStringList parsed;
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();
   bott_fit          = fname_load.contains( "fitbot" );
DbgLv(1) << "LD:  bott_fit" << bott_fit << "fname_load" << fname_load;

   for ( int ii = 0; ii < lines.size(); ii++ )
   {
      QStringList values = lines[ ii ].split( ' ', QString::SkipEmptyParts );

      int valsize        = values.size();
DbgLv(1) << "LD:  ii" << ii << "valsize" << valsize;

      if ( valsize < 2 )   continue;

      if ( valsize > 3 )
      {
         values.removeFirst();
         valsize--;
      }

      double rmeni  = values[ 0 ].toDouble();
      double rbott  = values[ 1 ].toDouble();
      double rmsdv  = rbott;

      if ( rmeni < 5.0  || rmeni > 8.0 )  continue;

      count++;

      if ( valsize == 3 ) 
      {
         rmsdv         = values[ 2 ].toDouble();
         v_meni << rmeni;
         v_bott << rbott;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rbott, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 

      }

      else if ( valsize == 2 )
      {
         v_meni << rmeni;
         v_rmsd << rmsdv;

         parsed << QString().sprintf( "%3d : ", count ) +
                   QString::number( rmeni, 'f', 5 ) + ", " +
                   QString::number( rmsdv, 'f', 8 ); 
      }
   }

   bool was3val  = have3val;
   have3val      = ( v_bott.count() > 0 );
   te_data->e->setPlainText( parsed.join( "\n" ) );
DbgLv(1) << "LD:  was3val have3val" << was3val << have3val
 << "v_rmsd size" << v_rmsd.size() << "parsed length"
 << parsed.length();

   if ( ( have3val && !was3val )  || 
        ( !have3val && was3val ) )
      change_plot_type();
}

// Handle a mouse click according to the current pick step
void US_FitMeniscus::mouse( const QwtDoublePoint& p )
{
  double radius_min[ 2 ];
  double rmsd_min  [ 2 ];
 
  // Plot the minimum selected by user
     
  radius_min[ 0 ] = p.x();
  radius_min[ 1 ] = p.x();
  
  rmsd_min  [ 0 ] = miny_global - 1.0 * dy_global;
  rmsd_min  [ 1 ] = miny_global + 2.0 * dy_global;
DbgLv(1) << "MOUSE: p.x" << p.x() << "rmsd_min" << rmsd_min;
  
  minimum_curve_sel->setSamples( radius_min, rmsd_min, 2 );
  
  le_men_sel->setText( QString::number( p.x(), 'f', 5 ) );

  meniscus_plot->replot();
}

// Plot the data
void US_FitMeniscus::plot_3d( void )
{

   bool auto_lim = false;
   double min_x  = 1.0e+99;
   double min_y  = 1.0e+99;
   double min_z  = 1.0e+99;
   double max_x  = -1.0e+99;
   double max_y  = -1.0e+99;
   double max_z  = -1.0e+99;
   double b_meni = 0.0;
   double b_bott = 0.0;
   int min_ix    = -1;

   // Get minima,maxima and meniscus,bottom at best rmsd
   for ( int ii = 0; ii < v_meni.size(); ii++ )
   {
      double rmeni  = v_meni[ ii ];
      double rbott  = v_bott[ ii ];
      double rrmsd  = v_rmsd[ ii ];

      // Find min and max
      min_x        = qMin( min_x, rmeni );
      max_x        = qMax( max_x, rmeni );
      min_y        = qMin( min_y, rbott );
      max_y        = qMax( max_y, rbott );
      max_z        = qMax( max_z, rrmsd );

      if ( rrmsd < min_z )
      {  // Save best-rmsd meniscus,bottom
         min_z        = rrmsd;
         b_meni       = rmeni;
         b_bott       = rbott;
         min_ix       = ii + 1;
      }
   }
   double min_r = 1.0 / max_z;
   double max_r = 1.0 / min_z;

   // Report low-rmsd meniscus and bottom values
   le_men_lor->setText( QString::number( b_meni, 'f', 5 ) );
   le_bot_lor->setText( QString::number( b_bott, 'f', 5 ) +
                        "  ( " +
                        QString::number( min_ix ) + " )" );
DbgLv(1) << "pl3d: v_meni size" << v_meni.size() << "min,max x,yz"
 << min_x << max_x << min_y << max_y << min_z << max_z;

   // Compute and report fitted meniscus and bottom

   int nmeni     = v_meni.count( v_meni[ 0 ] );
   int nbott     = v_bott.count( v_bott[ 0 ] );
   ix_best       = min_ix - 1;
   int ix_men    = ix_best / nbott;
   int ix_bot    = ix_best % nbott;
   QList< int >  ixs;
DbgLv(1) << "pl3d:  nmeni nbott" << nmeni << nbott << "ix_best" << ix_best
 << "ix_men ix_bot" << ix_men << ix_bot;

   // Compute indexes for up to 9 points centered at best index.
   //  Exclude some if center point is on edge(s).
   for ( int jmx = ix_men - 1; jmx < ix_men + 2; jmx++ )
   {
      if ( jmx < 0  ||  jmx >= nmeni )    // Skip out-of-bounds index
         continue;

      for ( int jbx = ix_bot - 1; jbx < ix_bot + 2; jbx++ )
      {
         if ( jbx < 0  ||  jbx >= nbott ) // Skip out-of-bounds index
            continue;

         ixs << ( jmx * nbott + jbx );    // Save full vector index
DbgLv(1) << "pl3d:   jmx jbx ixs" << jmx << jbx << (jmx*nbott+jbx);
      }
   }

   // Now calculate the weight averages of meniscus and bottom
   double f_meni = 0.0;
   double f_bott = 0.0;
   double w_rmsd = 0.0;
   for ( int ii = 0; ii < ixs.count(); ii++ )
   {
      int jx        = ixs[ ii ];
      double f_rmsd = 1.0 / v_rmsd[ jx ] - min_r;
      f_meni       += ( v_meni[ jx ] * f_rmsd );
      f_bott       += ( v_bott[ jx ] * f_rmsd );
      w_rmsd       += f_rmsd;
DbgLv(1) << "pl3d:  ixs" << jx << "f_meni f_bott w_rmsd"
 << f_meni << f_bott << w_rmsd;
   }
   f_meni       /= w_rmsd;
   f_bott       /= w_rmsd;
DbgLv(1) << "pl3d:  f_meni f_bott" << f_meni << f_bott;

   le_men_fit->setText( QString::number( f_meni, 'f', 5 ) );
   le_bot_fit->setText( QString::number( f_bott, 'f', 5 ) );

   // Start building 3-D plot
   double* a_meni = v_meni.data();
   double* a_bott = v_bott.data();
   double* a_rmsd = v_rmsd.data();

   QColor cblack( Qt::black );
   QColor cwhite( Qt::white );
DbgLv(1) << "pl3d: clear meniscus_plot";
   dataPlotClear( meniscus_plot );
DbgLv(1) << "pl3d: meniscus_plot replot";
   meniscus_plot->replot();
DbgLv(1) << "pl3d: set pick:";
   pick           = new US_PlotPicker( meniscus_plot );
DbgLv(1) << "pl3d: colormap bg";
   colormap       = new QwtLinearColorMap( Qt::white, Qt::black );
   colormap->addColorStop( 0.15, Qt::cyan );
   colormap->addColorStop( 0.33, Qt::green );
   colormap->addColorStop( 0.50, Qt::blue );
   colormap->addColorStop( 0.67, Qt::magenta );
   colormap->addColorStop( 0.85, Qt::red );

   QColor bg     = colormap->color1();
   meniscus_plot->setCanvasBackground( bg );

   int csum      = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( ( csum > 600 ) ? cblack : cwhite ) );
DbgLv(1) << "pl3d: bg" << bg << "csum" << csum;
DbgLv(1) << "pl3d:   pickpen" << pick->trackerPen().brush().color();
DbgLv(1) << "pl3d:    cblack" << cblack << "cwhite" << cwhite;

   QList< S_Solute > sdistro;
   for ( int ii = 0; ii < v_meni.count(); ii++ )
   {
      S_Solute sentry;
      sentry.s      = a_meni[ ii ];
      sentry.k      = a_bott[ ii ];
      sentry.c      = 1.0 / a_rmsd[ ii ] - min_r;
      sdistro << sentry;
   }

   // Set up spectrogram data
   min_z        = min_r;
   max_z        = max_r;
   d_spectrogram = new QwtPlotSpectrogram();
   spec_dat      = new US_SpectrogramData();
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   d_spectrogram->setData    ( spec_dat );
   d_spectrogram->attach( meniscus_plot );

#if 0
   QwtDoubleRect drect;

   if ( auto_lim )
   {
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
   }

   else
   {
      drect = QwtDoubleRect( min_x, min_y,
            ( max_x - min_x ), ( max_y - min_y ) );
   }
#endif
#if 1
   QwtDoubleRect drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );
#endif

   double xreso  = 300.0;
   double yreso  = 300.0;
   double resolu =  30.0;
   double zfloor = ct_zfloor->value();
   spec_dat->setRastRanges( xreso, yreso, resolu, zfloor, drect );
   spec_dat->setZRange( min_z, max_z );
   spec_dat->setRaster( &sdistro );

   // Set color map and axis settings
   QwtScaleWidget *rightAxis = meniscus_plot->axisWidget( QwtPlot::yRight );
   rightAxis    ->setColorBarEnabled( true );
   rightAxis    ->setColorMap( spec_dat->range(),
                               ColorMapCopy( colormap ) );
   d_spectrogram->setColorMap( ColorMapCopy( colormap ) );
   meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Meniscus Radius" ) );
   meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "Bottom Radius" ) );
   meniscus_plot->setAxisTitle( QwtPlot::yRight,  tr( "Offset Reciprocal RMSD" ) );
   meniscus_plot->axisTitle( QwtPlot::yRight ).setFont(
                        meniscus_plot->axisTitle( QwtPlot::yLeft ).font() );
   meniscus_plot->setAxisScale( QwtPlot::yRight,
      spec_dat->range().minValue(), spec_dat->range().maxValue() );
   meniscus_plot->enableAxis( QwtPlot::yRight );

   if ( auto_lim )
   {   // auto limits
      meniscus_plot->setAxisAutoScale( QwtPlot::yLeft   );
      meniscus_plot->setAxisAutoScale( QwtPlot::xBottom );
   }
   else
   {   // manual limits
      double dx       = qAbs( max_x - min_x ) * 0.05;
      double dy       = qAbs( max_y - min_y ) * 0.05;
      meniscus_plot->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx );
      meniscus_plot->setAxisScale( QwtPlot::yLeft,   min_y - dy, max_y + dy );
   }

   // Report mid-point meniscus,bottom
   double mid_meni = ( min_x + max_x ) * 0.5;
   double mid_bott = ( min_y + max_y ) * 0.5;
   le_mprads ->setText( QString::number( mid_meni ) + ", " +
                        QString::number( mid_bott ) );

   // Draw cross-hairs over best-rmsd point
   double mx_meni[ 2 ];
   double my_meni[ 2 ];
   double mx_bott[ 2 ];
   double my_bott[ 2 ];
   mx_meni[ 0 ] = b_meni;
   mx_meni[ 1 ] = b_meni;
   my_meni[ 0 ] = min_y;
   my_meni[ 1 ] = max_y;
   mx_bott[ 0 ] = min_x;
   mx_bott[ 1 ] = max_x;
   my_bott[ 0 ] = b_bott;
   my_bott[ 1 ] = b_bott;
   QwtPlotCurve* mmeni_curve = us_curve( meniscus_plot, tr( "Low-RMSD Meniscus" ) ); 
   mmeni_curve->setPen    ( QPen( Qt::black, 0, Qt::DashLine ) );
   mmeni_curve->setStyle  ( QwtPlotCurve::Lines );
   mmeni_curve->setSamples( mx_meni, my_meni, 2 );
   QwtPlotCurve* mbott_curve = us_curve( meniscus_plot, tr( "Low-RMSD Bottom" ) ); 
   mbott_curve->setPen    ( QPen( Qt::black, 0, Qt::DashLine ) );
   mbott_curve->setStyle  ( QwtPlotCurve::Lines );
   mbott_curve->setSamples( mx_bott, my_bott, 2 );

   // Plot it
   meniscus_plot->replot();
}

void US_FitMeniscus::plot_2d( void )
{
   if ( ! bott_fit )
   {
     if ( auto_mode )
      meniscus_plot->setTitle    ( tr( "Meniscus Fit" )  + ": " + triple_information[ "triple_name" ] );
     else
       meniscus_plot->setTitle    ( tr( "Meniscus Fit" ) );

      meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "2DSA Meniscus RMSD" ) );
      meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Meniscus Radius" ) );
      lb_men_sel->setText( tr( "Meniscus selected:" ) );
   }
   else
   {
     if ( auto_mode )
       meniscus_plot->setTitle    ( tr( "Bottom Fit" ) + ": " + triple_information[ "triple_name" ] );
     else
       meniscus_plot->setTitle    ( tr( "Bottom Fit" ) );
      
      meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "2DSA Bottom RMSD" ) );
      meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Bottom Radius" ) );
      lb_men_sel->setText( tr( "Bottom selected:" ) );
   }
      
   meniscus_plot->enableAxis  ( QwtPlot::yRight,  false );
   dataPlotClear( meniscus_plot );
   lb_men_sel  ->setVisible( true  );
   le_men_sel  ->setVisible( true  );
   lb_rms_error->setVisible( true  );
   le_rms_error->setVisible( true  );
   lb_order    ->setVisible( true  );
   sb_order    ->setVisible( true  );
   lb_men_lor  ->setVisible( false );
   le_men_lor  ->setVisible( false );
   lb_men_fit  ->setVisible( false );
   le_men_fit  ->setVisible( false );
   lb_bot_lor  ->setVisible( false );
   le_bot_lor  ->setVisible( false );
   lb_bot_fit  ->setVisible( false );
   le_bot_fit  ->setVisible( false );
   lb_mprads   ->setVisible( false );
   le_mprads   ->setVisible( false );
   lb_zfloor   ->setVisible( false );
   ct_zfloor   ->setVisible( false );

   int     count = v_meni.count();
   double* radius_values = v_meni.data();
   double* rmsd_values   = v_rmsd.data();

   double  minx = 1e20;
   double  maxx = 0.0;

   double  miny = 1e20;
   double  maxy = 0.0;

   // Remove any non-data lines and put values in arrays
   for ( int ii = 0; ii < count; ii++ )
   {
      // Find min and max
      minx = qMin( minx, radius_values[ ii ] );
      maxx = qMax( maxx, radius_values[ ii ] );

      miny = qMin( miny, rmsd_values[ ii ] );
      maxy = qMax( maxy, rmsd_values[ ii ] );
   }

   if ( count < 3 ) return;

   double overscan = ( maxx - minx ) * 0.10;  // 10% overscan

   meniscus_plot->setAxisScale( QwtPlot::xBottom, 
         minx - overscan, maxx + overscan );
    
   // Adjust y axis to scale all the data
   double dy = fabs( maxy - miny ) / 10.0;
   dy_global = dy;
   
   meniscus_plot->setAxisScale( QwtPlot::yLeft, miny - dy, maxy + dy );

   raw_curve = us_curve( meniscus_plot, tr( "Raw Data" ) ); 
   raw_curve->setPen    ( QPen( Qt::yellow ) );
   raw_curve->setSamples( radius_values, rmsd_values, count );

   // Do the fit and get the minimum

   double c[ 10 ];

   int order = sb_order->value();

   if ( ! US_Matrix::lsfit( c, radius_values, rmsd_values, count, order + 1 ) )
   {
      QMessageBox::warning( this,
            tr( "Data Problem" ),
            tr( "The data is inadequate for this fit order" ) );
      
      le_men_fit   ->clear();
      le_bot_fit   ->clear();
      le_mprads    ->clear();
      le_rms_error ->clear();
      meniscus_plot->replot();

      return;  
   }

   int fit_count = (int) ( ( maxx - minx + 2 * overscan ) / 0.001 );

   QVector< double > vfitx( fit_count );
   QVector< double > vfity( fit_count );
   double* fit_x = vfitx.data();
   double* fit_y = vfity.data();
   double  x     = minx - overscan;
   double fit_xvl;

   for ( int i = 0; i < fit_count; i++, x += 0.001 )
   {
      fit_x[ i ] = x;
      fit_y[ i ] = c[ 0 ];

      for ( int j = 1; j <= order; j++ ) 
         fit_y[ i ] += c[ j ] * pow( x, j );
   }

   // Calculate Root Mean Square Error
   double rms_err = 0.0;

   for ( int i = 0; i < count; i++ )
   {
      double x = radius_values[ i ];
      double y = rmsd_values  [ i ];

      double y_calc = c[ 0 ];
      
      for ( int j = 1; j <= order; j++ )  
         y_calc += c[ j ] * pow( x, j );
      
      rms_err += sq ( fabs ( y_calc - y ) );
   }

   le_rms_error->setText( QString::number( sqrt( rms_err / count ), 'e', 5 ) );

   // Find the minimum
   if ( order == 2 )
   {
      // Take the derivitive and get the minimum
      // c1 + 2 * c2 * x = 0
      fit_xvl = - c[ 1 ] / ( 2.0 * c[ 2 ] );
   }
   else
   {
      // Find the zero of the derivative
      double dxdy  [ 9 ];
      double d2xdy2[ 8 ];

      // First take the derivitive
      for ( int i = 0; i < order; i++ ) 
         dxdy[ i ] = c[ i + 1 ] * ( i + 1 );

      // And we'll need the 2nd derivitive
      for ( int i = 0; i < order - 1; i++ ) 
         d2xdy2[ i ] = dxdy[ i + 1 ] * ( i + 1 );

      // We'll do a quadratic fit for the initial estimate
      double q[ 3 ];
      US_Matrix::lsfit( q, radius_values, rmsd_values, count, 3 );
      fit_xvl = - q[ 1 ] / ( 2.0 * q[ 2 ] );

      const double epsilon = 1.0e-4;

      int    k = 0;
      double f;
      double f_prime;
      do
      {
        // f is the 1st derivative
        f = dxdy[ 0 ];
        for ( int i = 1; i < order; i++ ) f += dxdy[ i ] * pow( fit_xvl, i );

        // f_prime is the 2nd derivative
        f_prime = d2xdy2[ 0 ];
        for ( int i = 1; i < order - 1; i++ ) 
           f_prime += d2xdy2[ i ] * pow( fit_xvl, i );

        if ( fabs( f ) < epsilon ) break;
        if ( k++ > 10 ) break;

        // Get the next estimate
        fit_xvl -= f / f_prime;

      } while ( true );
   }

   fit_curve = us_curve( meniscus_plot, tr( "Fitted Data" ) ); 
   fit_curve->setPen    ( QPen( Qt::red ) );
   fit_curve->setSamples( fit_x, fit_y, fit_count );


   // copy miny to global variables
   miny_global = miny;
   
   // Plot the minimum

   minimum_curve = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve->setPen( QPen( QBrush( Qt::cyan ), 3.0 ) );

   double radius_min[ 2 ];
   double rmsd_min  [ 2 ];

   radius_min[ 0 ] = fit_xvl;
   radius_min[ 1 ] = fit_xvl;

   rmsd_min  [ 0 ] = miny - 1.0 * dy;
   rmsd_min  [ 1 ] = miny + 2.0 * dy;

   minimum_curve->setSamples( radius_min, rmsd_min, 2 );

   // Display selected meniscus/bottom
   le_men_sel->setText( QString::number( fit_xvl, 'f', 5 ) );

   //Save Best (Fitted) Meniscus Value for later comparison:
   Meniscus_fitted_2d_val = le_men_sel->text().toDouble();

   // Find the "best-index", index where X closest to fit
   ix_best           = 0;
   double diff_min   = 1.0e+99;

   for ( int ii = 0; ii < v_meni.count(); ii++ )
   {
      double diff_x     = qAbs( v_meni[ ii ] - fit_xvl );

      if ( diff_x < diff_min )
      {  // Running least difference and index to it
         diff_min          = diff_x;
         ix_best           = ii;
      }
   }
   

   // Add the marker label -- bold, font size default + 1, lines 3 pixels wide
   QPen markerPen( QBrush( Qt::white ), 3.0 );
   markerPen.setWidth( 3 );
   
   QwtPlotMarker* pm  = new QwtPlotMarker();
   QwtText        label( QString::number( fit_xvl, 'f', 5 ) );
   QFont          font( pm->label().font() );
   QwtSymbol*     sym = new QwtSymbol( QwtSymbol::Cross, QBrush( Qt::white ),
                                       markerPen, QSize( 9, 9 ) );

   font.setBold( true );
   font.setPointSize( font.pointSize() + 1 );
   label.setFont( font );

   pm->setValue         ( fit_xvl, miny + 3.0 * dy );
   pm->setSymbol        ( sym ); 
   pm->setLabel         ( label );
   pm->setLabelAlignment( Qt::AlignTop );

   pm->attach( meniscus_plot );

   meniscus_plot->replot();
}


//get status of the unique start of the FITMEN update
int US_FitMeniscus::read_autoflowAnalysisStages( const QString& requestID )
{
  int status = 0;

  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "FitMeniscus: Could not connect to database \n" ) + db->lastError() );
      return status;
    }


  //qDebug() << "BEFORE query ";
  QStringList qry;
  qry << "fitmen_autoflow_analysis_status"
      << requestID;

  status = db->statusQuery( qry );
  //qDebug() << "AFTER query ";

  return status;
}

//Read autoflowAnalysis record for given requestID
QMap< QString, QString> US_FitMeniscus::read_autoflowAnalysis_record( const QString& requestID )
{
  QMap <QString, QString> analysis_details;

  US_Passwd pw;
  US_DB2* db = new US_DB2( pw.getPasswd() );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
  			    tr( "FitMeniscus: Could not connect to database \n" ) + db->lastError() );
      return analysis_details;
    }

  QStringList qry;
  qry << "read_autoflowAnalysis_record"
      << requestID;
  
  db->query( qry );

  while ( db->next() )
    {
      analysis_details[ "requestID" ]      = db->value( 0 ).toString();
      analysis_details[ "triple_name" ]    = db->value( 1 ).toString();
      analysis_details[ "cluster" ]        = db->value( 2 ).toString();
      analysis_details[ "filename" ]       = db->value( 3 ).toString();
      analysis_details[ "aprofileGUID" ]   = db->value( 4 ).toString();
      analysis_details[ "invID" ]          = db->value( 5 ).toString();
      analysis_details[ "CurrentGfacID" ]  = db->value( 6 ).toString();
      analysis_details[ "currentHPCARID" ] = db->value( 7 ).toString();
      analysis_details[ "status_json" ]    = db->value( 8 ).toString();
      analysis_details[ "status" ]         = db->value( 9 ).toString();
      analysis_details[ "status_msg" ]     = db->value( 10 ).toString();
      analysis_details[ "create_time" ]    = db->value( 11 ).toString();   
      analysis_details[ "update_time" ]    = db->value( 12 ).toString();
      analysis_details[ "create_userd" ]   = db->value( 13 ).toString();
      analysis_details[ "update_user" ]    = db->value( 14 ).toString();
      analysis_details[ "nextWaitStatus" ] = db->value( 15 ).toString();
      analysis_details[ "nextWaitStatusMsg" ] = db->value( 16 ).toString();
    }
  
  return analysis_details;
}

// Update an edit file with a new meniscus and/or bottom radius value
void US_FitMeniscus::edit_update( void )
{
  /***/
  //ALEXEY: if autoflow: check if edit profiles already updated from other FITMEN session
  if ( auto_mode )
    {
      QString requestID = triple_information[ "requestID" ];
      //--- LOCK && UPDATE the autoflowStages' ANALYSIS field for the record
      int status_fitmen_unique;
      status_fitmen_unique = read_autoflowAnalysisStages( requestID );

      qDebug() << "status_fitmen_unique -- " << status_fitmen_unique ;
      //status_fitmen_unique is 0 if already STARTED:
      
      if ( !status_fitmen_unique )
	{
	  QMessageBox::information( this,
				    tr( "FITMEN | Triple Analysis already processed" ),
				    tr( "It appears that FITMEN stage has already been processed by "
					"a different user from different session. \n\n"
					"The program will return to the autoflow runs dialog where "
					"you can re-attach to the actual current stage of the run. "));
	  
	  
	  emit triple_analysis_processed( );
	  close();
	  return;
	}
    }
  /****/
  
      /*
      //-- CHECK status --//
      // Regular procedure
      // if (started | done)
      //    return; 

      //--- CHECK for longer times after possible update ------- //
      QMap <QString, QString> analysis_details;
      
      analysis_details = read_autoflowAnalysis_record( requestID );

      if ( !analysis_details.size() )
	{
	  //no record, so analysis completed/cancelled and already in the autoflowAnalysisHistory

	  QMessageBox::information( this,
				    tr( "FITMEN | Triple Analysis already processed" ),
				    tr( "It appears that FITMEN stage has already been processed by "
					"a different user from different session and "
					"the entire analysis for the current triple is completed. \n\n"
					"The program will return to the autoflow runs dialog where "
					"you can re-attach to the actual current stage of the run. "));
					
	  
	  emit triple_analysis_processed( );
	  close();
	  
	  return;
	}
      else
	{
	  //there is an autoflowAnalysis record:
	  
	  QString status_gen     = analysis_details["status"];
	  QString nextWaitStatus = analysis_details[ "nextWaitStatus" ] ;
	  QString status_json    = analysis_details["status_json"];

	  QJsonDocument jsonDoc = QJsonDocument::fromJson( status_json.toUtf8() );
	  if (!jsonDoc.isObject())
	    {
	      qDebug() << "FITMEN: NOT a JSON Doc !!";
	      return;
	    }

	  const QJsonValue &submitted  = jsonDoc.object().value("submitted");          
	  
	  //look for FITMEN stage in "submitted" stages
	  QString stage_name = submitted.toString();
	  if ( submitted.toString() == "FITMEN" )
	    {
	      if ( status_gen != "WAIT" )
		{
		  QMessageBox::information( this,
					    tr( "FITMEN already processed" ),
					    tr( "It appears that FITMEN stage has already been processed by "
						"a different user from different session.\n\n"
						"The program will return to ANALYSIS tab where "
						"you can continue to monitor the overall analysis progress." ) );
		  
		  emit editProfiles_updated_earlier();
		  close();
		  
		  return;
		}
	    }
	  else
	    {
	      //FITMEN is no in "submitted" stages anymore, so it's prcessed:
	      QMessageBox::information( this,
					tr( "FITMEN already processed" ),
					tr( "It appears that FITMEN stage has already been processed by "
					    "a different user from different session.\n\n"
					    "The program will return to ANALYSIS tab where "
					    "you can continue to monitor the overall analysis progress." ) );
	      
	      emit editProfiles_updated_earlier();
	      close();
	      
	      return;
	    }
	}
    }
   */
  //-- End check for FITMEN stage status in autoflow ---- //
  //-- However, it's not complete: upon updating edit profile below, the row needs to be locked while updating --//
  
#define MENI_HIGHVAL 7.0
#define BOTT_LOWVAL 7.0
   QString fn = filedir + "/" + fname_edit;
DbgLv(1) << " eupd:  fname_edit" << fname_edit;
DbgLv(1) << " eupd:  fn" << fn;
   idEdit     = 0;
   QFile filei( fn );
   QString edtext;
   QStringList edtexts;

   if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      return;
   }

   bool confirm  = ck_confirm->isChecked();
   bool all_wvl  = ( nedtfs > 1  &&  ck_applymwl->isChecked() );
   bool rmv_mdls = true;
   bool db_upd   = dkdb_cntrls->db();

   qDebug() << "In edit_update(): confirm ? " << confirm;
//*DEBUG*
//db_upd=false;
//*DEBUG*
   int ixmlin    = 0;     // Meniscus line start index
   int ixmval    = 0;     // Meniscus value start index
   int ncmval    = 0;     // Meniscus value number characters
   int ncmlin    = 0;     // Meniscus line number characters
   int ixblin    = 0;     // Bottom line start index
   int ixbval    = 0;     // Bottom value start index
   int ncbval    = 0;     // Bottom value number characters
   int ncblin    = 0;     // Bottom line number characters
   int ixllin    = 0;     // Left-data line start index
   int ixlval    = 0;     // Left-data value start index
   int nclval    = 0;     // Left-data value number characters
   int demval    = 0;     // Delta old/new meniscus value
   int debval    = 0;     // Delta old/new bottom value

   // New meniscus and bottom values
   double mennew = 0.0;
   double botnew = 0.0;
   double lefval = 0.0;

   
   
   if ( have3val )
   {  // Fit is meniscus + bottom
      mennew         = le_men_fit->text().toDouble();
      botnew         = le_bot_fit->text().toDouble();
   }
   else if ( !bott_fit )
   {  // Fit is meniscus only
      mennew         = le_men_sel->text().toDouble();
   }
   else
   {  // Fit is bottom only
      botnew         = le_men_sel->text().toDouble();
   }

   QString s_meni = QString().sprintf( "%.5f", mennew );
   QString s_bott = QString().sprintf( "%.5f", botnew );
DbgLv(1) << " eupd:  s_meni s_bott" << s_meni << s_bott;
   QString mmsg   = "";
   QString mhdr   = "";
   bool bad_vals  = false;

   // Check meniscus and bottom values for reasonableness
   if ( mennew > MENI_HIGHVAL )
   {
      bad_vals       = true;

      if ( botnew != 0.0  &&  botnew < BOTT_LOWVAL )
      {
         mhdr           = tr( "Unreasonable Meniscus and Bottom" );
         mmsg           = tr( "Both the currently selected Mensicus and\n"
                              "Bottom values" ) + " ( " + s_meni + ", "
                        + s_bott + " )\n"
                        + tr( "are unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting reasonable values." );
      }
      else
      {
         mhdr           = tr( "Unreasonable Meniscus" );
         mmsg           = tr( "The currently selected Mensicus value" )
                        + " ( " + s_meni + " )\n"
                        + tr( "is unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting a reasonable value." );
      }
   }
   else if ( botnew != 0.0  &&  botnew < BOTT_LOWVAL )
   {
      bad_vals       = true;
      mhdr           = tr( "Unreasonable Bottom" );
      mmsg           = tr( "The currently selected Bottom value" )
                     + " ( " + s_bott + " )\n"
                     + tr( "is unreasonable !\n\nIt is recommended"
                              " that you \"Cancel\" the \"Update Edit\"\n"
                              "and retry after setting a reasonable value." );
   }

   if ( bad_vals )
   {
     if ( !auto_mode )
       {
	 int response   = QMessageBox::critical( (QWidget*)this,
						 mhdr,
						 mmsg,
						 QMessageBox::Save,
						 QMessageBox::Cancel );
	 
	 if ( response == QMessageBox::Cancel )
	   {
	     QMessageBox::information( (QWidget*)this,
				       tr( "Canceled" ),
				       tr( "\"Update Edit\" has been canceled!" ) );
	     
	     return;
	   }
	 else
	   {
	     QMessageBox::information( (QWidget*)this,
				       tr( "Saving" ),
				       tr( "\"Update Edit\" will proceed!" ) );
	   }
       }
     else //GMP framework: stop ANALYSIS for current triple, send signal
       {
	 bad_men_vals = true;
	 // QString reason_for_failure = mhdr + ", " + mmsg.split("!")[0];
	 // triple_information[ "failed" ] = reason_for_failure;
	 // emit bad_meniscus_values( triple_information );
	 // close();
	 

	 return;
       }
   }

   mmsg           = "";

   //ALEXEY: Set progressDialog
   progress_msg = NULL;
   
   // Proceed with updating the Edit data

   if ( ! all_wvl  ||  nedtfs == 1 )
   {  // Apply to a single triple
      QTextStream ts( &filei );    // Build up XML Edit text
      while ( !ts.atEnd() )
         edtext += ts.readLine() + "\n";
      filei.close();

      // Compute indecies,lengths of meniscus,bottom,data lines,values
      ixmlin   = edtext.indexOf( "<meniscus radius=" );
      if ( ixmlin < 0 )  return;
      ixmval   = edtext.indexOf( "=\"", ixmlin ) + 2;
      ncmval   = edtext.indexOf( "\"",  ixmval + 1 ) - ixmval;
      ncmlin   = edtext.indexOf( ">", ixmlin ) - ixmlin + 1;
      ixllin   = edtext.indexOf( "<data_range left=" );
      if ( ixllin < 0 )  return;
      ixlval   = edtext.indexOf( "=\"", ixllin ) + 2;
      nclval   = edtext.indexOf( "\"",  ixlval + 1 ) - ixlval;
      lefval   = edtext.mid( ixlval, nclval ).toDouble();
      ixblin   = edtext.indexOf( "<bottom radius=" );
      if ( ixblin > 0 )
      {
         ixbval   = edtext.indexOf( "=\"", ixblin ) + 2;
         ncbval   = edtext.indexOf( "\"",  ixbval + 1 ) - ixbval;
         ncblin   = edtext.indexOf( ">", ixblin ) - ixblin + 1;
      }
      
      DbgLv(1) << " eupd:  mennew" << mennew << "lefval" << lefval << "botnew" << botnew;
      DbgLv(1) << " eupd:   ixmlin ixblin" << ixmlin << ixblin << "ncmlin ncblin" << ncmlin << ncblin;

      
      if ( mennew >= lefval )
	{
	  if ( !auto_mode )
	    {
	      QMessageBox::warning( this, tr( "Meniscus within Data Range" ),
				    tr( "The selected Meniscus value, %1 , extends into the data"
					" range whose left-side value is %2 . This Edit update"
					" cannot be performed!" ).arg( mennew ).arg( lefval ) );
	      
	      return;
	    }
	  else // GMP
	    {
	      bad_men_vals = true;
	      // QString reason_for_failure = QString( "The selected Meniscus value, %1 , extends into the data range whose left-side value is %2")
	      // 	.arg( mennew )
	      // 	.arg( lefval );
	      // triple_information[ "failed" ] = reason_for_failure;
	      // emit bad_meniscus_values( triple_information );
	      // close();

	      return;
	    }
	}
      
      demval        = s_meni.length() - ncmval;  // Deltas old,new values
      debval        = s_bott.length() - ncbval;

      if ( have3val )
      {  // Replace meniscus and bottom values in edit
         edtext.replace( ixmval, ncmval, s_meni );
         ncmval       += demval;
         ncmlin       += demval;
         if ( ixbval > 0 )
         {  // Replace existing bottom value
            ixbval       += demval;
            edtext.replace( ixbval, ncbval, s_bott );
            ixblin       += demval;
            ncbval       += debval;
            ncblin       += debval;
         }
         else
         {  // Must insert an entirely new line for bottom
            QString bline = QString( "\n            <bottom radius=\"" )
                            + s_bott + QString( "\"/>" );
            ixblin        = ixmlin + ncmlin;
            edtext.insert( ixblin, bline );
            ncblin        = bline .length();
            ncbval        = s_bott.length();
         }
      }
      else if ( !bott_fit )
      {  // Replace meniscus value in edit
         edtext.replace( ixmval, ncmval, s_meni );
         ncmval       += demval;
         ncmlin       += demval;
      }
      else
      {  // Replace/insert bottom value in edit
         if ( ixbval > 0 )
         {  // Replace existing bottom value
            edtext.replace( ixbval, ncbval, s_bott );
            ncbval       += debval;
            ncblin       += debval;
         }
         else
         {  // Must insert an entirely new line for bottom
            QString bline = QString( "\n            <bottom radius=\"" )
                            + s_bott + QString( "\"/>" );
            ixblin        = ixmlin + ncmlin;
            edtext.insert( ixblin, bline );
            ncblin        = bline .length();
            ncbval        = s_bott.length();
         }
      }

      if ( auto_mode )
         le_status->setText( "Updating edit profile: " + fname_edit ); 
      
      // Write out the modified Edit XML text
      QFile fileo( fn );

      if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
	//-- Revert autoflowAnalysisSatges back to 'unknown'
	if ( auto_mode )
	  {
	    QString requestID = triple_information[ "requestID" ];
	    revert_autoflow_analysis_stages_record( requestID );
	  }
	//---------------------------------------------------//
	
         return;
      }

      QTextStream tso( &fileo );
      tso << edtext;
      fileo.close();

      mmsg     = tr( "In file directory\n    " ) + filedir + " ,\n" +
                 tr( "file\n    " ) + fname_edit + "\n";
      if ( have3val )
      {  // Report on Meniscus,Bottom mods
         mmsg     = mmsg + tr( "has been modified with the lines:" )
                    + "\n    " + edtext.mid( ixmlin, ncmlin )
                    + "\n    " + edtext.mid( ixblin, ncblin );
      }
      else if ( !bott_fit )
      {  // Report on Meniscus mods
         mmsg     = mmsg + tr( "has been modified with the line:" )
                    + "\n    " + edtext.mid( ixmlin, ncmlin );
      }
      else
      {  // Report on Bottom mods
         mmsg     = mmsg + tr( "has been modified with the line:" )
                    + "\n    " + edtext.mid( ixblin, ncblin );
      }
   
      // If using DB, update the edit record there

      if ( db_upd )
      {
         update_db_edit( edtext, fn, mmsg );
      }

      if ( confirm )
      {  // Confirm at each update step
         mmsg    += tr( "\n\nDo you want to remove all fit models"
                        " (and associated noises) except for the one"
                        " associated with the best RMSD value?" );

         int response = QMessageBox::question( this,
                                               tr( "Edit File Updated" ),
                                               mmsg,
                                               QMessageBox::Yes,
                                               QMessageBox::Cancel );

         rmv_mdls     = ( response == QMessageBox::Yes );
//*DEBUG*
//DbgLv(1) << "Test call to get index";
//index_model_setfit();
//DbgLv(1) << "RETURN: ix_setfit" << ix_setfit;
//rmv_mdls=false;
//*DEBUG*
      }
   }  // END: apply to single triple

   else
   {  // Apply to all wavelengths in a cell/channel

     //ALEXEY: Set progressDialog
     //progress_msg = NULL;
     if( auto_mode )
       {
	 progress_msg = new QProgressDialog ("Updating edit profiles...", QString(), 0, nedtfs, this);
	 progress_msg->setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
	 progress_msg->setWindowModality(Qt::WindowModal);
	 progress_msg->setWindowTitle(tr("Updating Edit Profiles"));
	 QFont font_d  = progress_msg->property("font").value<QFont>();
	 QFontMetrics fm(font_d);
	 int pixelsWide = fm.width( progress_msg->windowTitle() );
	 qDebug() << "Progress_msg: pixelsWide -- " << pixelsWide;
	 progress_msg ->setMinimumWidth( pixelsWide*2 );
	 progress_msg->adjustSize();
	 
	 progress_msg->setAutoClose( false );
	 progress_msg->setValue( 0 );
	 //progress_msg->setRange( 1, nedtfs );
	 progress_msg->show();
       }
     ////////////////////////////

      QString dmsg   = "";
      int idEdsv   = idEdit;
DbgLv(1) << " eupd: AppWvl: nedtfs" << nedtfs;
      QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

      for ( int jj = 0; jj < nedtfs; jj++ )
      {  // Modify each Edit file corresponding to a wavelength
         QString fn = filedir + "/" + edtfiles.at( jj );
DbgLv(1) << " eupd:     jj" << jj << "fn" << fn;

         if ( auto_mode )
            le_status->setText( "Updating edit profile: " + edtfiles.at( jj )  ); 
 
         QFile filei( fn );

         if ( ! filei.open( QIODevice::ReadOnly | QIODevice::Text ) )
         {  // Skip any file we cannot read
DbgLv(1) << " eupd:       *OPEN ERROR*";
            continue;
         }

DbgLv(1) << " eupd:       edtext read";
         // Read in the Edit XML text
         QTextStream ts( &filei );
         edtext  = "";
         while ( !ts.atEnd() )
            edtext += ts.readLine() + "\n";
         filei.close();
DbgLv(1) << " eupd:       edtext len" << edtext.length();

         // Compute locations and lengths of meniscus,bottom,data lines,values
         ixmlin   = edtext.indexOf( "<meniscus radius=" );
         if ( ixmlin < 0 )  continue;
         ixmval   = edtext.indexOf( "=\"", ixmlin ) + 2;
         ncmval   = edtext.indexOf( "\"",  ixmval + 1 ) - ixmval;
         ncmlin   = edtext.indexOf( ">", ixmlin ) - ixmlin + 1;
         ixllin   = edtext.indexOf( "<data_range left=" );
         if ( ixllin < 0 )  continue;
         ixlval   = edtext.indexOf( "=\"", ixllin ) + 2;
         nclval   = edtext.indexOf( "\"",  ixlval + 1 ) - ixlval;
         lefval   = edtext.mid( ixlval, nclval ).toDouble();
         ixblin   = edtext.indexOf( "<bottom radius=" );
         if ( ixblin > 0 )
         {
            ixbval   = edtext.indexOf( "=\"", ixblin ) + 2;
            ncbval   = edtext.indexOf( "\"",  ixbval + 1 ) - ixbval;
            ncblin   = edtext.indexOf( ">", ixblin ) - ixblin + 1;
         }
	 
	 DbgLv(1) << " eupd:       ixmlin ixblin ixllin" << ixmlin << ixblin << ixllin;

         if ( mennew >= lefval )
	   {
	     if ( !auto_mode )
	       {
		 QMessageBox::warning( this, tr( "Meniscus within Data Range" ),
				       tr( "The selected Meniscus value, %1 , extends into the data"
					   " range whose left-side value is %2 . This Edit update"
					   " cannot be performed!" ).arg( mennew ).arg( lefval ) );
		 
		 continue;
	       }
	     else //GMP
	       {
		 bad_men_vals = true;
		 // QString reason_for_failure = QString( "The selected Meniscus value, %1 , extends into the data range whose left-side value is %2")
		 //   .arg( mennew )
		 //   .arg( lefval );
		 // triple_information[ "failed" ] = reason_for_failure;
		 // emit bad_meniscus_values( triple_information );
		 // close();
		 		 
		 return;  //ALEXEY - if one wvl in a triple fails, ALL fail!!!
		 //continue;
	       }
	     
	   }

         demval        = s_meni.length() - ncmval;  // Deltas in old,new value strings
         debval        = s_bott.length() - ncbval;

         if ( have3val )
         {  // Replace meniscus and bottom values in edit
            edtext.replace( ixmval, ncmval, s_meni );
            ncmval       += demval;
            ncmlin       += demval;
            if ( ixbval > 0 )
            {  // Replace an existing bottom line
               ixbval       += demval;
               edtext.replace( ixbval, ncbval, s_bott );
               ixblin       += demval;
               ncbval       += debval;
               ncblin       += debval;
            }
            else
            {  // Must insert an entirely new line for bottom
               QString bline = QString( "\n            <bottom radius=\"" )
                               + s_bott + QString( "\"/>" );
               ixblin        = ixmlin + ncmlin;
               edtext.insert( ixblin, bline );
               ncblin        = bline .length();
               ncbval        = s_bott.length();
            }
DbgLv(1) << " eupd:       3DVL replace";
         }
         else if ( !bott_fit )
         {  // Replace meniscus value in edit
            edtext.replace( ixmval, ncmval, s_meni );
            ncmval       += demval;
            ncmlin       += demval;
DbgLv(1) << " eupd:       MENI replace";
         }
         else
         {  // Replace bottom value in edit
            if ( ixbval > 0 )
            {
               edtext.replace( ixbval, ncbval, s_bott );
               ncbval       += debval;
               ncblin       += debval;
            }
            else
            {  // Must insert an entirely new line for bottom
               QString bline = QString( "\n            <bottom radius=\"" )
                               + s_bott + QString( "\"/>" );
               ixblin        = ixmlin + ncmlin;
               edtext.insert( ixblin, bline );
               ncblin        = bline .length();
               ncbval        = s_bott.length();
            }
DbgLv(1) << " eupd:       BOTT replace";
         }

DbgLv(1) << " eupd:  write: fn" << fn;
         // Write out the modified Edit XML file
         QFile fileo( fn );

         if ( ! fileo.open( QIODevice::WriteOnly | QIODevice::Text ) )
            continue;

         QTextStream tso( &fileo );
         tso << edtext;
         fileo.close();

         // If using DB, update the edit record there

         if ( db_upd )
         {

	   if (progress_msg != NULL )
	     {
	       progress_msg->setValue( jj );
	     }
	     
DbgLv(1) << " eupd:       call upd_db_ed";
            update_db_edit( edtext, fn, dmsg );
DbgLv(1) << " eupd:       ret fr upd_db_ed  idEdit" << idEdit;

            if ( edtfiles[ jj ] == fname_edit )
               idEdsv       = idEdit;
         }

      }  // END: wavelengths loop
DbgLv(1) << " eupd:       idEdit idEdsv" << idEdit << idEdsv;
      idEdit       = idEdsv;
DbgLv(1) << " eupd:       idEdit" << idEdit;

      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

            
      // Construct the edit update message
      mmsg     = tr( "In file directory\n    " ) + filedir + " ,\n" +
                 tr( "file\n    " ) + fname_edit + "\n";

      if ( have3val )
      {  // Report on Meniscus+Bottom mods
         mmsg     = mmsg + tr( "has been modified with the lines:" )
                    + "\n    " + edtext.mid( ixmlin, ncmlin )
                    + "\n    " + edtext.mid( ixblin, ncblin );
      }
      else if ( !bott_fit )
      {  // Report on Meniscus mods
         mmsg     = mmsg + tr( "has been modified with the line:" )
                    + "\n    " + edtext.mid( ixmlin, ncmlin );
      }
      else
      {  // Report on Bottom mods
         mmsg     = mmsg + tr( "has been modified with the line:" )
                    + "\n    " + edtext.mid( ixblin, ncblin );
      }

      mmsg    += tr( "\n\n%1 other cell/channel files were"
                     " similarly modified." ).arg( nedtfs - 1 ) + dmsg;
   
      if ( confirm )
      {  // Confirm at each update step
         mmsg    += tr( "\n\nDo you want to remove all fit-meniscus models"
                        " (and associated noises) except for the one"
                        " associated with the nearest meniscus value?" );

         int response = QMessageBox::question( this, tr( "Edit File Updated" ),
                           mmsg, QMessageBox::Yes, QMessageBox::Cancel );

         rmv_mdls = ( response == QMessageBox::Yes );
      }

   }  // END: apply to all wavelengths

   if ( rmv_mdls )
   {
DbgLv(1) << " call Remove Models";
      remove_models();
   }

   if ( ! confirm && !auto_mode )
   {
      mmsg    += tr( "\n\nAll fit-meniscus models (and associated noises),"
                     " except for the one set associated with the best"
                     " RMSD value, were removed." );

      QMessageBox::information( this, tr( "Edit File Updated" ), mmsg );
   }

   //For autoflow Analysis
   if ( auto_mode )
     {
       if ( progress_msg != NULL )
	 {
	   progress_msg->setValue( progress_msg->maximum() );
	   progress_msg->close();
	 }

       
       //Identify if Meniscus || Bottom || (Meniscus && Bottom) have been changed
       if ( have3val )
	 {  // Fit is meniscus + bottom
	  
	 }
       else if ( !bott_fit )
	 {  // Fit is meniscus only
	   if ( mennew != Meniscus_fitted_2d_val )
	     triple_information[ "FMB_changed" ] = QString("YES");
	   else
	     triple_information[ "FMB_changed" ] = QString("NO");
	 }
       else
	 {  // Fit is bottom only
	   if ( botnew != Meniscus_fitted_2d_val )
	     triple_information[ "FMB_changed" ] = QString("YES");
	   else
	     triple_information[ "FMB_changed" ] = QString("NO");
	 }

       emit editProfiles_updated( triple_information );
       close();
     }
}

// Slot to revert autoflowAnalysisStages record
void US_FitMeniscus::revert_autoflow_analysis_stages_record( const QString& requestID )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "FitMen revert: Could not connect to database \n" ) + db->lastError() );
       return;
     }
   
   //qDebug() << "BEFORE query ";
   QStringList qry;
   qry << "fitmen_autoflow_analysis_status_revert"
       << requestID;
   
   db->query( qry );
   //qDebug() << "AFTER query ";
}


// Slot for handling a loaded file:  set the name of loaded,edit files
void US_FitMeniscus::file_loaded( QString fn )
{
DbgLv(1) << "FL: IN:  fn" << fn;
   filedir    = fn.section( "/",  0, -2 );
   fname_load = fn.section( "/", -1, -1 );
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();

   QString runID    = filedir.section( "/", -1, -1 );
   QString anType   = fname_load.section( ".", -2, -2 );
   QString tripnode = fname_load.section( ".", -3, -3 );
   QString edtLabel = fname_load.section( ".", -4, -4 ).mid( 1 );
   QString tripl    = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 ) + "." +
                      tripnode.mid( 2 );
   QStringList edtfilt;
DbgLv(1) << "edtLabel" << edtLabel;
   if ( !edtLabel.contains( "DSA-F" ) )
   {  // fitmen file name has edit label
      edtfilt << runID + "." + edtLabel + ".*." + tripl + ".xml";
   }
   else
   {  // fitmen file name has NO edit label (older form)
      edtfilt << runID + ".*.*." + tripl + ".xml";
   }

   fname_edit = "";
   edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
   nedtfs     = edtfiles.size();
DbgLv(1) << "EDITFILT" << edtfilt << "nedtfs" << nedtfs;

   if ( nedtfs == 1 )
   {  // Choose the single corresponding edit
      fname_edit       = edtfiles.at( 0 );
      pb_update->setEnabled( true );
   }

   else if ( nedtfs > 0 )
   {  // Choose the latest edit
      int min_ms       = 999999;
      QString fname    = edtfiles.at( 0 );
      QString fpath    = filedir + "/" + fname;
      QDateTime cdate  = QFileInfo( QFile( fpath ) )
                         .lastModified().toUTC();

      for ( int jj = 0; jj < nedtfs; jj++ )
      {
         fname            = edtfiles.at( jj );
         fpath            = filedir + "/" + fname;
         QDateTime fdate  = QFileInfo( QFile( fpath ) )
                            .lastModified().toUTC();
         int file_ms      = fdate.msecsTo( cdate );
DbgLv(1) << "     jj" << jj << "fdate" << fdate << "file_ms" << file_ms;
         if ( file_ms < min_ms )
         {
            min_ms           = file_ms;
            fname_edit       = fname;
         }
      }
DbgLv(1) << " nedtfs" << nedtfs << "fname_edit" << fname_edit;
DbgLv(1) << "   f0" << edtfiles.at(0);
if(nedtfs>1) DbgLv(1) << "   f1" << edtfiles.at(1);

      pb_update->setEnabled( true );
   }

   else
   {  // Could not find edit file, so inform the user
      pb_update->setEnabled( false );

      QMessageBox::warning( this,
            tr( "Missing Local Edit" ),
            tr( "Update Edit is not possible\n"
                "without a local copy of the Edit file\n"
                "corresponding to the FM models.\n"
                "Use\n     Convert Legacy Data\nand\n     Manage Data\n"
                "to create a local copy of an Edit file for\n     " )
            + fname_load + tr( "\nof run\n     " ) + runID );
   }

   // If apply-to-all-wavelengths, get list with all wavelengths
   if ( ck_applymwl->isChecked() )
   {
      edtfilt.clear();
      edtfilt << fname_edit.section( ".", 0, -3 ) + ".*.xml";
      edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
      nedtfs     = edtfiles.size();
DbgLv(1) << "FL: aplmwl: nedtfs" << nedtfs << "edtfilt" << edtfilt;
   }

   plot_data();
   
   le_status->setText( tr( "Data loaded:  " ) + runID + "/" + fname_load );

   //Meniscus position, user specified curve
   minimum_curve_sel = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve_sel->setPen( QPen( QBrush( Qt::red ), 3.0 ) );
}

void US_FitMeniscus::get_editProfile_copy( QMap < QString, QString > & triple_information  )
{
  qDebug() << "In get_edit COPY: ";
  /* ALEXEY: we can use only triple_information[ "filename" ] (the editedData table's 'label' filed) 
     to extract all editProfiles for this runID (which is triple_information[ "filename" ] )
   */
  
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  // To be extracted from editedData table ('filename' field)
  // QMap below - establish correspondence btw EditProfile filename && EditDataID
  // select filename, editedDataID from editedData where label='ISSF-KulkarniJ_NP1-pDNA-D2O-0_091020-run822-2A';
  QMap <QString, int> EProfs_to_IDs;
  
  QStringList query;
  query << "get_editedDataFilenamesIDs" << triple_information["filename"];
  db->query( query );

  qDebug() << "Query: " << query;
  
  while ( db->next() )
    {
      QString  filename  = db->value( 0 ).toString();
      int      ID        = db->value( 1 ).toInt();

      EProfs_to_IDs[ filename ] = ID;
    }
  
   QMap<QString, int>::iterator fn;
   for ( fn = EProfs_to_IDs.begin(); fn != EProfs_to_IDs.end(); ++fn )
   {
qDebug() << "EditProfile filename / EditDataID: " << fn.key() << "/" << fn.value();
      QString filename = fn.key();
      int editedDataID = fn.value();

      QString dirpath  = US_Settings::resultDir() + "/" + triple_information[ "filename" ];
      QString filepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + filename;

      // Can check here if such filename exists
      QFileInfo check_file( filepath );
      if ( check_file.exists() && check_file.isFile() )
qDebug() << "EditProfile file: " << filepath << " exists";
      else
	{
	  if ( !QDir( dirpath ).exists() )
	    QDir().mkdir( dirpath );
	    
	  db->readBlobFromDB( filepath, "download_editData", editedDataID );
	}
   }
}

// Slot for handling a loaded file:  set the name of loaded,edit files
bool US_FitMeniscus::file_loaded_auto( QMap < QString, QString > & triple_information  )
{
  qDebug() << "In file_loaded_auto: ";
  QString file_directory = US_Settings::resultDir() + QString("/") + triple_information[ "filename" ];
  QString triple_name_cut = triple_information[ "triple_name" ];
  triple_name_cut.simplified();
  triple_name_cut.replace("/","");
  triple_name_cut.replace(" ","");

  if ( triple_name_cut.contains("Interference") )
    triple_name_cut.replace("Interference","660");
  

  qDebug() << "In file_loaded_auto: 11: " << triple_name_cut;
  qDebug() << "Triple filename: " << triple_information[ "filename" ];
  
  QDir directory (file_directory);
  QStringList fm_files = directory.entryList( QStringList() << "2DSA-FM*" + triple_name_cut + "*.fitmen.dat", QDir::Files | QDir::NoSymLinks);

  qDebug() << "In file_loaded_auto: 22";

  //ALEXEY: if there is no files (since no "-FM" models produced for what ever reason), issue a warning:
  if ( !fm_files.size()  )
    {
      QMessageBox::warning( this,
			    tr( "FM models problem" ),
			    tr( "No \"FM | FMB\" models have been found for the present run. \n\n"
				"Program will proceed with the analysis of other triples...") );

      no_fm_data = true;
      return false;
    }
  
  QString fn = directory.absoluteFilePath( fm_files[0] );  //ALEXEY: *should be* the only one fitmen.dat file 

  qDebug() << "In file_loaded_auto: 33";
  
  qDebug() << "File to open: " << fn;
  
  QString text;
  QFile f( fn );

  if ( f.open( QIODevice::ReadOnly | QIODevice::Text ) )
  {
     QTextStream t( &f );

     text = t.readAll();
     f.close(  );

     te_data->e->setPlainText( text );
  }
  else
     QMessageBox::information( this,
           tr( "Error" ),
           tr( "Could not open\n\n" ) + fn + tr( "\n\n for reading." ) );
  
  
DbgLv(1) << "FL: IN:  fn" << fn;
   filedir    = fn.section( "/",  0, -2 );
   fname_load = fn.section( "/", -1, -1 );
   v_meni.clear();
   v_bott.clear();
   v_rmsd.clear();

   QString runID    = filedir.section( "/", -1, -1 );
   QString anType   = fname_load.section( ".", -2, -2 );
   QString tripnode = fname_load.section( ".", -3, -3 );
   QString edtLabel = fname_load.section( ".", -4, -4 ).mid( 1 );
   QString tripl    = tripnode.left( 1 ) + "." +
                      tripnode.mid( 1, 1 ) + "." +
                      tripnode.mid( 2 );
   QStringList edtfilt;
DbgLv(1) << "edtLabel" << edtLabel;
   if ( !edtLabel.contains( "DSA-F" ) )
   {  // fitmen file name has edit label
      edtfilt << runID + "." + edtLabel + ".*." + tripl + ".xml";
   }
   else
   {  // fitmen file name has NO edit label (older form)
      edtfilt << runID + ".*.*." + tripl + ".xml";
   }

   fname_edit = "";
   edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
   nedtfs     = edtfiles.size();
DbgLv(1) << "EDITFILT" << edtfilt << "nedtfs" << nedtfs;

   if ( nedtfs == 1 )
   {  // Choose the single corresponding edit
      fname_edit       = edtfiles.at( 0 );
      pb_update->setEnabled( true );
   }

   else if ( nedtfs > 0 )
   {  // Choose the latest edit
      int min_ms       = 999999;
      QString fname    = edtfiles.at( 0 );
      QString fpath    = filedir + "/" + fname;
      QDateTime cdate  = QFileInfo( QFile( fpath ) )
                         .lastModified().toUTC();

      for ( int jj = 0; jj < nedtfs; jj++ )
      {
         fname            = edtfiles.at( jj );
         fpath            = filedir + "/" + fname;
         QDateTime fdate  = QFileInfo( QFile( fpath ) )
                            .lastModified().toUTC();
         int file_ms      = fdate.msecsTo( cdate );
DbgLv(1) << "     jj" << jj << "fdate" << fdate << "file_ms" << file_ms;
         if ( file_ms < min_ms )
         {
            min_ms           = file_ms;
            fname_edit       = fname;
         }
      }
DbgLv(1) << " nedtfs" << nedtfs << "fname_edit" << fname_edit;
DbgLv(1) << "   f0" << edtfiles.at(0);
if(nedtfs>1) DbgLv(1) << "   f1" << edtfiles.at(1);

      pb_update->setEnabled( true );
   }

   else
   {  // Could not find edit file, so inform the user
      pb_update->setEnabled( false );

      QMessageBox::warning( this,
            tr( "Missing Local Edit" ),
            tr( "Update Edit is not possible\n"
                "without a local copy of the Edit file\n"
                "corresponding to the FM models.\n"
                "Use\n     Convert Legacy Data\nand\n     Manage Data\n"
                "to create a local copy of an Edit file for\n     " )
            + fname_load + tr( "\nof run\n     " ) + runID );
   }

   // If apply-to-all-wavelengths, get list with all wavelengths
   if ( ck_applymwl->isChecked() )
   {
      edtfilt.clear();
      edtfilt << fname_edit.section( ".", 0, -3 ) + ".*.xml";
      edtfiles   = QDir( filedir ).entryList( edtfilt, QDir::Files, QDir::Name );
      nedtfs     = edtfiles.size();
DbgLv(1) << "FL: aplmwl: nedtfs" << nedtfs << "edtfilt" << edtfilt;
   }

   plot_data();
   
   le_status->setText( tr( "Data loaded:  " ) + runID + "/" + fname_load );

   //Meniscus position, user specified curve
   minimum_curve_sel = us_curve( meniscus_plot, tr( "Minimum Pointer" ) ); 
   minimum_curve_sel->setPen( QPen( QBrush( Qt::red ), 3.0 ) );

   return true;
}



// Scan the database for models to use to write local fit table files
void US_FitMeniscus::scan_dbase()
{
   QVector< ModelDesc > mDescrs;   // Vector of model description objects
   US_Passwd pw;                   // DB password
   US_DB2 db( pw.getPasswd() );    // DB control
   QStringList query;              // DB query string list
   QStringList mfnams;             // List of FM model fit file names
   QStringList ufnams;             // List of unique model fit file names
   QStringList uantms;             // List of unique model fit analysis times
   QStringList tmodels;            // List: IDs of models with truncated descrs
   QStringList tedGIDs;            // List: edit GUIDs of models w/ trunc descrs
   QStringList tedIDs;             // List: edit IDs of models w/ trunc descrs
   QList< int > botredo;           // List: bottom-redo Mdescr vector

   int         nfmods = 0;         // Number of fit-meniscus models
   int         nfsets = 0;         // Number of fit-meniscus analysis sets
   int         nfrpls = 0;         // Number of fit file replacements
   int         nfadds = 0;         // Number of fit file additions
   int         nfexss = 0;         // Number of fit files left as they existed

   QString invID = QString::number( US_Settings::us_inv_ID() );

   QRegExp fmIter  = QRegExp( "i\?\?-[mb]*",
         Qt::CaseSensitive, QRegExp::Wildcard );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Scan the database and find fit-meniscus models

   le_status->setText(
         tr( "Scanning DB fit-meniscus models ..." ) );
   query << "get_model_desc" << invID;
   db.query( query );

   while( db.next() )
   {
      ModelDesc mdescr;
      QString modelID    = db.value( 0 ).toString();
      QString modelGUID  = db.value( 1 ).toString();
      QString descript   = db.value( 2 ).toString();
      QString editGUID   = db.value( 5 ).toString();
      QString editID     = db.value( 6 ).toString();

      if ( descript.length() == 80 )
      {  // Truncated description:  save ID and skip update for now
DbgLv(1) << "DbSc:     TRUNC: modelID" << modelID << "descr" << descript;
         tmodels << modelID;
         tedGIDs << editGUID;
         tedIDs  << editID;
         continue;
      }

      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      double  bottom     = 0.0;
      QDateTime lmtime   = db.value( 7 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
DbgLv(1) << "DbSc:   modelID vari meni" << modelID << variance << meniscus
 << "ansysID" << ansysID << "iterID" << iterID;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information

         // Format and save the potential fit table file name
         QString fitVals    = iterID  .section( '-',  1,  1 );
DbgLv(1) << "DbSc:    *FIT* " << descript << "fitVals" << fitVals;
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";
DbgLv(1) << "DbSc:     fittype" << fittype << "fextn" << fextn;
         if ( fittype == 2  ||  fittype == 3 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString anType     = ansysID .section( '_',  2, -3 );
DbgLv(1) << "DbSc:       anType" << anType << "editLabel" << editLabel << "ansysID" << ansysID;
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      } // END: model is fit type
   } // END: database model record reads
DbgLv(1) << "DbSc: tmodels size" << tmodels.size() << "ted sizes"
 << tedGIDs.size() << tedIDs.size();

   // Review models with truncated descriptions
   for ( int ii = 0; ii < tmodels.size(); ii++ )
   {
      QString modelID    = tmodels[ ii ];
      query.clear();
      query << "get_model_info" << modelID;
      db.query( query );

      if ( db.lastErrno() != US_DB2::OK )  continue;

      db.next();

      QString modelGUID  = db.value( 0 ).toString();
      QString descript1  = db.value( 1 ).toString();
      QString contents   = db.value( 2 ).toString();
      int     jdx        = contents.indexOf( "description=" );
//DbgLv(1) << "DbSc:    ii jdtx" << ii << jdtx << "modelID" << modelID
//   << "  dsc1" << descript1 << " cont" << contents.left( 20 );

      if ( jdx < 1 )  continue;

//DbgLv(1) << "DbSc:      jdx lend" << jdx << lend;
      QString descript   = contents.mid( jdx ).section( '"', 1, 1 );
      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QString editGUID   = tedGIDs[ ii ];
      QString editID     = tedIDs [ ii ];

      QDateTime lmtime   = db.value( 6 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
//DbgLv(1) << "DbSc:   dscr1" << descript1 << "dcs" << descript;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;
         ModelDesc mdescr;
         double  bottom     = 0.0;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;
         if ( fitVals.length() > 6 )
            fittype            = 3;
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;
         else if ( fitVals.startsWith( "f" ) )
            fittype            = 1;
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";

         if ( fittype != 1 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }

         QString anType     = ansysID .section( '_',  2, -3 );
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      }
   }

   // Redo any model descriptions that need a bottom value
   for ( int ii = 0; ii < botredo.count(); ii++ )
   {
      int jj             = botredo[ ii ];
      ModelDesc mdescr   = mDescrs[ jj ];
      QString modelID    = mdescr.modelID;
      US_Model wmodel;
      wmodel.load( modelID, &db );
      double bottom      = wmodel.bottom;

      if ( bottom < 1.0 )
      {  // Bottom not reliable in model, get from model description
         QString descript   = mdescr.description;
         QString ansysID    = descript.section( '.', -2, -2 );
         QString iterID     = ansysID .section( '_', -1, -1 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         bottom             = ( fittype == 2 ) ?
                              ( fitVals.mid( 1, 5 ).toDouble() / 10000.0 ) :
                              ( fitVals.mid( 6, 5 ).toDouble() / 10000.0 );
      }

      mdescr.bottom      = bottom;
      mDescrs[ jj ]      = mdescr;
   }

   nfmods     = mDescrs.count();
DbgLv(1) << "Number of FM models found: " << nfmods;
if(nfmods>0) {
DbgLv(1) << " pre:D0" <<  mDescrs[0].description;
DbgLv(1) << " pre:Dn" <<  mDescrs[nfmods-1].description; }
   qSort( mDescrs );
if(nfmods>0) {
DbgLv(1) << " sorted:D0" <<  mDescrs[0].description;
DbgLv(1) << " sorted:Dn" <<  mDescrs[nfmods-1].description; }

   // Scan local files to see what fit table files already exist

   le_status->setText(
         tr( "Comparing to existing local meniscus,rmsd table files ..." ) );
   mfnams.clear();
   ufnams.clear();
   uantms.clear();

   for ( int ii = 0; ii < nfmods; ii++ )
   {  // Find unique file names in order to create sets
      QString ftfname    = mDescrs[ ii ].fitfname;
      QString antime     = mDescrs[ ii ].antime;

      if ( ! ufnams.contains( ftfname ) )
      {  // This is a new fit-file name, so new analysis set
         ufnams << ftfname;
         uantms << antime;
      }

      else if ( ! uantms.contains( antime ) )
      {  // Already seen fit-file, but new analysis time, so duplicate
         uantms << antime;
      }

      mfnams << mDescrs[ ii ].fitfname;
   }

   nfsets     = ufnams.size();
   int nantm  = uantms.size();
   int ndupl  = nantm - nfsets;
DbgLv(1) << "Number of FM analysis sets: " << nfsets;
DbgLv(1) << "Number of FM analysis set duplicates: " << ndupl;
   int kfsets = nfsets;
   QString rdir = US_Settings::resultDir().replace( "\\", "/" ) + "/";
   QString fnamesv;

   for ( int ii = 0; ii < kfsets; ii++ )
   {  // Find out for each set whether a corresponding fit file exists
      QString ftfname    = ufnams.at( ii );

      if ( mfnams.count( ftfname ) == 1 )
      {  // Not really a set; single fit model after previous fm run
         nfsets--;
DbgLv(1) << "ScDB: SINGLE:" << ftfname;
         continue;
      }

      QString ftfpath    = rdir + ftfname;
      QFile   ftfile( ftfpath );

      if ( ftfile.exists() )
      {  // File exists, so we must check the need to replace it
         QString ftfpath    = rdir + ftfname;
         QDateTime fdate    = QFileInfo( ftfile ).lastModified().toUTC();
         int       jj       = mfnams.lastIndexOf( ftfname );
         QDateTime rdate    = mDescrs[ jj ].lmtime;
DbgLv(1) << " ii rdate fdate" << ii << rdate << fdate << "   ftfname"
   << ftfname << "  fdate.msecsTo(rdate)" << fdate.msecsTo(rdate);
DbgLv(1) << "   jj desc" << jj << mDescrs[jj].description
 << "antime meniscus" << mDescrs[jj].antime << mDescrs[jj].meniscus;

         if ( fdate.msecsTo( rdate ) > 0 )
         {  // DB record date is later than file date, so must replace file
            nfrpls++;
            ftfile.remove();
         }

         else
         {  // DB record date is not later than file date, so leave file as is
            nfexss++;
            continue;
         }
      }

      else
      {  // File does not exist, so we definitely need to create it
         nfadds++;
         QString ftfpath    = QString( rdir + ftfname ).section( "/", 0, -2 );
DbgLv(1) << "ScDB: NOT-EXIST local:  nfadds" << nfadds;
         QDir().mkpath( ftfpath );
      }

      if ( ! ftfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {  // Problem!!!
         qDebug() << "*ERROR* Unable to open file" << ftfname;
         continue;
      }

      // Creating a new or replacement file:  build list of meniscus,rmsd pairs
      int       jfirst   = mfnams.indexOf( ftfname );
      int       jlast    = mfnams.lastIndexOf( ftfname ) + 1;
      QString   antiml   = mDescrs[ jlast - 1 ].antime;
      QStringList mrpairs;

DbgLv(1) << " Creating" << ftfname << "jf,jl" << jfirst << jlast;
      for ( int jj = jfirst; jj < jlast; jj++ )
      {  // First build the pairs (or triples) list
         double bottom   = mDescrs[ jj ].bottom;
         double meniscus = mDescrs[ jj ].meniscus;
         double variance = mDescrs[ jj ].variance;
         double rmsd     = sqrt( variance );
         QString antime  = mDescrs[ jj ].antime;
         QString mrpair  = QString::number( meniscus, 'f', 6 ) + " "
                         + QString::number( rmsd,     'e', 6 ); 

         if ( bottom > 1.0 )
         {  // Either Bottom or Meniscus+Bottom
            if ( ftfname.contains( "FB" ) )
            {  // Bottom only
               mrpair          = QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
            else
            {  // Meniscus and Bottom
               mrpair          = QString::number( meniscus, 'f', 6 ) + " "
                               + QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
         }
DbgLv(1) << "  jj desc" << jj << mDescrs[jj].description;

         if ( antime == antiml )
            mrpairs << mrpair;
      }

      mrpairs.sort();
      QTextStream ts( &ftfile );

      // Output the pairs to the file
      for ( int jj = 0; jj < mrpairs.size(); jj++ )
         ts << mrpairs.at( jj ) + "\n";

      ftfile.close();

      fnamesv = fnamesv.isEmpty() ? ftfname : fnamesv;
   }

DbgLv(1) << "Number of FM REPLACE  sets: " << nfrpls;
DbgLv(1) << "Number of FM ADD      sets: " << nfadds;
DbgLv(1) << "Number of FM EXISTING sets: " << nfexss;

   // Report
   QString msg = tr( "File" );
   int nftota  = nfadds + nfrpls;

   if      ( nfadds == 1  &&  nfrpls == 0 )
      msg += tr( " added: " );

   else if ( nfadds == 0  &&  nfrpls == 1 )
      msg += tr( " updated: " );

   else if ( nfadds == 0  &&  nfrpls == 0 )
      msg  = tr( "No new fit files were created." );

   else
      msg  = tr( "Last of %1 added/updated: " ).arg( nftota );

   if ( nftota > 0 )
      msg += fnamesv;

   le_status->setText( msg );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
}


// Scan the database for models to use to write local fit table files
void US_FitMeniscus::scan_dbase_auto( QMap <QString, QString> & triple_information )
{
   QVector< ModelDesc > mDescrs;   // Vector of model description objects
   US_Passwd pw;                   // DB password
   US_DB2 db( pw.getPasswd() );    // DB control
   QStringList query;              // DB query string list
   QStringList mfnams;             // List of FM model fit file names
   QStringList ufnams;             // List of unique model fit file names
   QStringList uantms;             // List of unique model fit analysis times
   QStringList tmodels;            // List: IDs of models with truncated descrs
   QStringList tedGIDs;            // List: edit GUIDs of models w/ trunc descrs
   QStringList tedIDs;             // List: edit IDs of models w/ trunc descrs
   QList< int > botredo;           // List: bottom-redo Mdescr vector

   int         nfmods = 0;         // Number of fit-meniscus models
   int         nfsets = 0;         // Number of fit-meniscus analysis sets
   int         nfrpls = 0;         // Number of fit file replacements
   int         nfadds = 0;         // Number of fit file additions
   int         nfexss = 0;         // Number of fit files left as they existed

   QString invID = triple_information[ "invID" ];

   QRegExp fmIter  = QRegExp( "i\?\?-[mb]*",
         Qt::CaseSensitive, QRegExp::Wildcard );

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Scan the database and find fit-meniscus models

   le_status->setText(
         tr( "Scanning DB fit-meniscus models ..." ) );
   
   query << "get_model_desc_auto" << invID << triple_information[ "filename" ];
   // Make use of the description && '2DSA-FM'
   //query << "get_model_desc" << invID;
   db.query( query );

   while( db.next() )
   {
      ModelDesc mdescr;
      QString modelID    = db.value( 0 ).toString();
      QString modelGUID  = db.value( 1 ).toString();
      QString descript   = db.value( 2 ).toString();
      QString editGUID   = db.value( 5 ).toString();
      QString editID     = db.value( 6 ).toString();

      if ( descript.length() == 80 )
      {  // Truncated description:  save ID and skip update for now
DbgLv(1) << "DbSc:     TRUNC: modelID" << modelID << "descr" << descript;
         tmodels << modelID;
         tedGIDs << editGUID;
         tedIDs  << editID;
         continue;
      }

      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      double  bottom     = 0.0;
      QDateTime lmtime   = db.value( 7 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
DbgLv(1) << "DbSc:   modelID vari meni" << modelID << variance << meniscus
 << "ansysID" << ansysID << "iterID" << iterID;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information

         // Format and save the potential fit table file name
         QString fitVals    = iterID  .section( '-',  1,  1 );
DbgLv(1) << "DbSc:    *FIT* " << descript << "fitVals" << fitVals;
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";
DbgLv(1) << "DbSc:     fittype" << fittype << "fextn" << fextn;
         if ( fittype == 2  ||  fittype == 3 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString anType     = ansysID .section( '_',  2, -3 );

DbgLv(1) << "DbSc:       anType" << anType << "editLabel" << editLabel << "ansysID" << ansysID;
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      } // END: model is fit type
   } // END: database model record reads
DbgLv(1) << "DbSc: tmodels size" << tmodels.size() << "ted sizes"
 << tedGIDs.size() << tedIDs.size();

   // Review models with truncated descriptions
   for ( int ii = 0; ii < tmodels.size(); ii++ )
   {
      QString modelID    = tmodels[ ii ];
      query.clear();
      query << "get_model_info" << modelID;
      db.query( query );

      if ( db.lastErrno() != US_DB2::OK )  continue;

      db.next();

      QString modelGUID  = db.value( 0 ).toString();
      QString descript1  = db.value( 1 ).toString();
      QString contents   = db.value( 2 ).toString();
      int     jdx        = contents.indexOf( "description=" );
//DbgLv(1) << "DbSc:    ii jdtx" << ii << jdtx << "modelID" << modelID
//   << "  dsc1" << descript1 << " cont" << contents.left( 20 );

      if ( jdx < 1 )  continue;

//DbgLv(1) << "DbSc:      jdx lend" << jdx << lend;
      QString descript   = contents.mid( jdx ).section( '"', 1, 1 );
      double  variance   = db.value( 3 ).toString().toDouble();
      double  meniscus   = db.value( 4 ).toString().toDouble();
      QString editGUID   = tedGIDs[ ii ];
      QString editID     = tedIDs [ ii ];

      QDateTime lmtime   = db.value( 6 ).toDateTime();
      lmtime.setTimeSpec( Qt::UTC );
      QString ansysID    = descript.section( '.', -2, -2 );
      QString iterID     = ansysID .section( '_', -1, -1 );
//DbgLv(1) << "DbSc:   dscr1" << descript1 << "dcs" << descript;

      if ( ansysID.contains( "2DSA-F" )  ||  iterID.contains( fmIter ) )
      {  // Model from meniscus fit, so save information
DbgLv(1) << "DbSc:    *FIT* " << descript;
         ModelDesc mdescr;
         double  bottom     = 0.0;

         // Format and save the potential fit table file name
         QString runID      = descript.section( '.',  0, -4 );
         QString tripleID   = descript.section( '.', -3, -3 );
         QString editLabel  = ansysID .section( '_',  0, -5 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;
         if ( fitVals.length() > 6 )
            fittype            = 3;
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;
         else if ( fitVals.startsWith( "f" ) )
            fittype            = 1;
         QString fextn      = ( fittype != 2 ) ?
                              ".fitmen.dat" : ".fitbot.dat";

         if ( fittype != 1 )
         {  // Bottom or Meniscus+Bottom:  add to list of bottom redo's
            botredo << mDescrs.count();
         }

         QString anType     = ansysID .section( '_',  2, -3 );
	 
         QString ftfname    = runID + "/" + anType + "." + editLabel + "."
                              + tripleID + fextn;
         mdescr.description = descript;
         mdescr.baseDescr   = runID + "." + tripleID + "."
                              + ansysID.section( "-", 0, 3 );
         mdescr.fitfname    = ftfname;
         mdescr.modelID     = modelID;
         mdescr.modelGUID   = modelGUID;
         mdescr.editID      = editID;
         mdescr.editGUID    = editGUID;
         mdescr.variance    = variance;
         mdescr.meniscus    = meniscus;
         mdescr.bottom      = bottom;
         mdescr.antime      = descript.section( '.', -2, -2 )
                              .section( '_',  1,  1 ).mid( 1 );
         mdescr.lmtime      = lmtime;

         mDescrs << mdescr;
      }
   }

   // Redo any model descriptions that need a bottom value
   for ( int ii = 0; ii < botredo.count(); ii++ )
   {
      int jj             = botredo[ ii ];
      ModelDesc mdescr   = mDescrs[ jj ];
      QString modelID    = mdescr.modelID;
      US_Model wmodel;
      wmodel.load( modelID, &db );
      double bottom      = wmodel.bottom;

      if ( bottom < 1.0 )
      {  // Bottom not reliable in model, get from model description
         QString descript   = mdescr.description;
         QString ansysID    = descript.section( '.', -2, -2 );
         QString iterID     = ansysID .section( '_', -1, -1 );
         QString fitVals    = iterID  .section( '-',  1,  1 );
         int fittype        = 0;         // no fit
         if ( fitVals.length() > 6 )
            fittype            = 3;      // meniscus+bottom fit
         else if ( fitVals.startsWith( "b" ) )
            fittype            = 2;      // bottom fit
         else if ( fitVals.startsWith( "m" ) )
            fittype            = 1;      // meniscus fit
         bottom             = ( fittype == 2 ) ?
                              ( fitVals.mid( 1, 5 ).toDouble() / 10000.0 ) :
                              ( fitVals.mid( 6, 5 ).toDouble() / 10000.0 );
      }

      mdescr.bottom      = bottom;
      mDescrs[ jj ]      = mdescr;
   }

   nfmods     = mDescrs.count();
DbgLv(1) << "Number of FM models found: " << nfmods;
if(nfmods>0) {
DbgLv(1) << " pre:D0" <<  mDescrs[0].description;
DbgLv(1) << " pre:Dn" <<  mDescrs[nfmods-1].description; }
   qSort( mDescrs );
if(nfmods>0) {
DbgLv(1) << " sorted:D0" <<  mDescrs[0].description;
DbgLv(1) << " sorted:Dn" <<  mDescrs[nfmods-1].description; }

   // Scan local files to see what fit table files already exist

   le_status->setText(
         tr( "Comparing to existing local meniscus,rmsd table files ..." ) );
   mfnams.clear();
   ufnams.clear();
   uantms.clear();

   for ( int ii = 0; ii < nfmods; ii++ )
   {  // Find unique file names in order to create sets
      QString ftfname    = mDescrs[ ii ].fitfname;
      QString antime     = mDescrs[ ii ].antime;

      if ( ! ufnams.contains( ftfname ) )
      {  // This is a new fit-file name, so new analysis set
         ufnams << ftfname;
         uantms << antime;
      }

      else if ( ! uantms.contains( antime ) )
      {  // Already seen fit-file, but new analysis time, so duplicate
         uantms << antime;
      }

      mfnams << mDescrs[ ii ].fitfname;
   }

   nfsets     = ufnams.size();
   int nantm  = uantms.size();
   int ndupl  = nantm - nfsets;
DbgLv(1) << "Number of FM analysis sets: " << nfsets;
DbgLv(1) << "Number of FM analysis set duplicates: " << ndupl;
   int kfsets = nfsets;
   QString rdir = US_Settings::resultDir().replace( "\\", "/" ) + "/";
   QString fnamesv;

   for ( int ii = 0; ii < kfsets; ii++ )
   {  // Find out for each set whether a corresponding fit file exists
      QString ftfname    = ufnams.at( ii );

      if ( mfnams.count( ftfname ) == 1 )
      {  // Not really a set; single fit model after previous fm run
         nfsets--;
DbgLv(1) << "ScDB: SINGLE:" << ftfname;
         continue;
      }

      QString ftfpath    = rdir + ftfname;
      QFile   ftfile( ftfpath );

      if ( ftfile.exists() )
      {  // File exists, so we must check the need to replace it
         QString ftfpath    = rdir + ftfname;
         QDateTime fdate    = QFileInfo( ftfile ).lastModified().toUTC();
         int       jj       = mfnams.lastIndexOf( ftfname );
         QDateTime rdate    = mDescrs[ jj ].lmtime;
DbgLv(1) << " ii rdate fdate" << ii << rdate << fdate << "   ftfname"
   << ftfname << "  fdate.msecsTo(rdate)" << fdate.msecsTo(rdate);
DbgLv(1) << "   jj desc" << jj << mDescrs[jj].description
 << "antime meniscus" << mDescrs[jj].antime << mDescrs[jj].meniscus;

         if ( fdate.msecsTo( rdate ) > 0 )
         {  // DB record date is later than file date, so must replace file
            nfrpls++;
            ftfile.remove();
         }

         else
         {  // DB record date is not later than file date, so leave file as is
            nfexss++;
            continue;
         }
      }

      else
      {  // File does not exist, so we definitely need to create it
         nfadds++;
         QString ftfpath    = QString( rdir + ftfname ).section( "/", 0, -2 );
	 DbgLv(1) << "ScDB: NOT-EXIST local:  nfadds" << nfadds << "ftfpath: " << ftfpath;
         QDir().mkpath( ftfpath );
      }

      if ( ! ftfile.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {  // Problem!!!
         qDebug() << "*ERROR* Unable to open file" << ftfname;
         continue;
      }

      // Creating a new or replacement file:  build list of meniscus,rmsd pairs
      int       jfirst   = mfnams.indexOf( ftfname );
      int       jlast    = mfnams.lastIndexOf( ftfname ) + 1;
      QString   antiml   = mDescrs[ jlast - 1 ].antime;
      QStringList mrpairs;

DbgLv(1) << " Creating" << ftfname << "jf,jl" << jfirst << jlast;
      for ( int jj = jfirst; jj < jlast; jj++ )
      {  // First build the pairs (or triples) list
         double bottom   = mDescrs[ jj ].bottom;
         double meniscus = mDescrs[ jj ].meniscus;
         double variance = mDescrs[ jj ].variance;
         double rmsd     = sqrt( variance );
         QString antime  = mDescrs[ jj ].antime;
         QString mrpair  = QString::number( meniscus, 'f', 6 ) + " "
                         + QString::number( rmsd,     'e', 6 ); 

         if ( bottom > 1.0 )
         {  // Either Bottom or Meniscus+Bottom
            if ( ftfname.contains( "FB" ) )
            {  // Bottom only
               mrpair          = QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
            else
            {  // Meniscus and Bottom
               mrpair          = QString::number( meniscus, 'f', 6 ) + " "
                               + QString::number( bottom,   'f', 6 ) + " "
                               + QString::number( rmsd,     'e', 6 ); 
            }
         }
DbgLv(1) << "  jj desc" << jj << mDescrs[jj].description;

         if ( antime == antiml )
            mrpairs << mrpair;
      }

      mrpairs.sort();
      QTextStream ts( &ftfile );

      // Output the pairs to the file
      for ( int jj = 0; jj < mrpairs.size(); jj++ )
         ts << mrpairs.at( jj ) + "\n";

      ftfile.close();

      fnamesv = fnamesv.isEmpty() ? ftfname : fnamesv;
   }

DbgLv(1) << "Number of FM REPLACE  sets: " << nfrpls;
DbgLv(1) << "Number of FM ADD      sets: " << nfadds;
DbgLv(1) << "Number of FM EXISTING sets: " << nfexss;

   // Report
   QString msg = tr( "File" );
   int nftota  = nfadds + nfrpls;

   if      ( nfadds == 1  &&  nfrpls == 0 )
      msg += tr( " added: " );

   else if ( nfadds == 0  &&  nfrpls == 1 )
      msg += tr( " updated: " );

   else if ( nfadds == 0  &&  nfrpls == 0 )
      msg  = tr( "No new fit files were created." );

   else
      msg  = tr( "Last of %1 added/updated: " ).arg( nftota );

   if ( nftota > 0 )
      msg += fnamesv;

   le_status->setText( msg );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

   qDebug() << "End of scan auto:";
}


// Reset state of database scan button based on DB/Disk choice
void US_FitMeniscus::update_disk_db( bool isDB )
{
   pb_scandb->setEnabled( isDB );
}

// Update the DB edit record with a new meniscus and/or bottom value
void US_FitMeniscus::update_db_edit( QString edtext, QString efilepath,
      QString& msg )
{
   int     elnx     = edtext.indexOf( "<editGUID " );
   int     esvx     = edtext.indexOf( "\"", elnx ) + 1;
   int     nvch     = edtext.indexOf( "\"", esvx ) - esvx;
   QString edGUID   = edtext.mid( esvx, nvch );
DbgLv(1) << "updDbEd: edGUID" << edGUID;

   US_Passwd pw;
   US_DB2 db( pw.getPasswd() );
   QStringList query;
   query << "get_editID" << edGUID;
   db.query( query );
   db.next();
   idEdit           = db.value( 0 ).toString().toInt();
DbgLv(1) << "updDbEd: idEdit" << idEdit;
   if ( db.writeBlobToDB( efilepath, "upload_editData", idEdit )
        != US_DB2::OK )
   {
      qDebug() << tr( "*ERROR* update_db_edit: " ) << db.lastError();
      msg += tr( "*ERROR* update_db_edit: " );
      msg += db.lastError();
      return;
   }

DbgLv(1) << "updDbEd: nedtfs" << nedtfs << "applymwl" << ck_applymwl->isChecked();
   if ( nedtfs == 1  ||  ! ck_applymwl->isChecked() )
   {
      if ( have3val )
         msg += tr( "\n\nThe meniscus and bottom values were"
                    " also updated for the corresponding edit"
                    " record in the database." );
      else if ( !bott_fit )
         msg += tr( "\n\nThe meniscus value was also updated for the"
                    " corresponding edit record in the database." );
      else
         msg += tr( "\n\nThe bottom value was also updated for the"
                    " corresponding edit record in the database." );
   }

   else
   {
      QString fn       = efilepath.section( "/", -1, -1 );
      int lstfx        = nedtfs - 1;
DbgLv(1) << "updDbEd: fn" << fn << "lstfx" << lstfx;
      if ( fn == edtfiles[ lstfx ] )
      {
         if ( have3val )
            msg += tr( "\n\nThe meniscus and bottom values were"
                       " also updated for the corresponding edit"
                       " records in the database." );
         else if ( !bott_fit )
            msg += tr( "\n\nThe meniscus value was also updated for the"
                       " corresponding edit records in the database." );
         else
            msg += tr( "\n\nThe bottom value was also updated for the"
                       " corresponding edit records in the database." );
DbgLv(1) << "updDbEd:  fn==fns[l]: msg" << msg;
      }
else
 DbgLv(1) << "updDbEd:  fn!=fns[l]";
   }

   return;
}

// Remove f-m models (and associated noise) except for the single chosen one
void US_FitMeniscus::remove_models()
{
   QString srchRun  = filedir.section   ( "/", -1, -1 );
   QString srchEdit = "e" +
                      fname_edit.section( ".", -6, -6 );
   QString srchTrip = fname_load.section( ".", -3, -3 );
   QString msetBase;
DbgLv(1) << "RmvMod: fname_load" << fname_load;
DbgLv(1) << "RmvMod: fname_edit" << fname_edit;
DbgLv(1) << "RmvMod: scn1  srchRun"
 << srchRun << "srchEdit" << srchEdit << "srchTrip" << srchTrip;

   // Get the model,list index to selected meniscus/bottom value(s)
   index_model_setfit();

   // Scan models files; get list of fit-meniscus type matching run/edit/triple
   QStringList modfilt;
   modfilt << "M*.xml";
   QString     moddir   = US_Settings::dataDir() + "/models";
   QStringList modfiles = QDir( moddir ).entryList(
         modfilt, QDir::Files, QDir::Name );
DbgLv(1) << "RmvMod: raw modfiles count" << modfiles.count();
   moddir               = moddir + "/";

   QList< ModelDesc >  lMDescrs;
   QList< ModelDesc >  dMDescrs;

   QStringList     lmodFnams;             // local model full path file names
   QStringList     lmodGUIDs;             // Local model GUIDs
   QList< double > lmodVaris;             // Local variance values
   QList< double > lmodMenis;             // Local meniscus values
   QStringList     lmodDescs;             // Local descriptions

   QStringList     dmodIDs;               // Database model IDs
   QStringList     dmodGUIDs;             // Database model GUIDs
   QList< double > dmodVaris;             // Database variance values
   QList< double > dmodMenis;             // Database meniscus values
   QStringList     dmodDescs;             // Database descriptions
   int nlmods           = 0;
   int ndmods           = 0;
   int nlnois           = 0;
   int ndnois           = 0;
   int lArTime          = 0;
   int dArTime          = 0;
   int lkModx           = -1;
   int dkModx           = -1;
   bool db_upd          = dkdb_cntrls->db();
//*DEBUG*
//db_upd=false;
//*DEBUG*

DbgLv(1) << "RmvMod: dk: modfiles size" << modfiles.size();
   for ( int ii = 0; ii < modfiles.size(); ii++ )
   {
      ModelDesc lmodd;
      QString modfname   = modfiles.at( ii );
      QString modpath    = moddir + modfname;
      US_Model model;
      
      if ( model.load( modpath ) != US_DB2::OK )
      {
DbgLv(1) << "RmvMod:  *LOAD ERR*" << modfname;
         continue;    // Can't use if can't load
      }

      QString descript   = model.description;
      QString runID      = descript.section( '.',  0, -4 );
      QString tripID     = descript.section( '.', -3, -3 );
      QString anRunID    = descript.section( '.', -2, -2 );
      QString editLabl   = anRunID .section( '_',  0, -5 );
      QString iterID     = anRunID .section( '_', -1, -1 );
      QString itNum      = iterID  .section( '-',  0,  0 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;
if(ii<3 || (ii+4)>modfiles.size() || ii==(modfiles.size()/2))
 DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
  << ii << runID << editLabl << tripID;

      if ( runID != srchRun  ||  editLabl != srchEdit  ||  tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

      if ( !iterID.contains( "-m" )  &&  !iterID.contains( "-b" ) )
      {
DbgLv(1) << "RmvMod:   ii" << ii << "iterID" << iterID
 << "has -m" << iterID.contains("-m") << "has -b" << iterID.contains("-b");
         continue;    // Can't use if not a fit-meniscus or fit-bottom type
      }

      if ( itNum == "01"  &&  msetBase.isEmpty() )
      {  // When fit iteration is "01" and not yet saved, save model set base
         msetBase       = descript.section( ".", 0, -3 ) + "." + 
                          anRunID .section( "_", 0,  3 );
DbgLv(1) << "RmvMod:lfiles: msetBase" << msetBase << "ii" << ii;
      }

      // Probably a file from the right set, but let's check for other sets
      int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();
DbgLv(1) << "RmvMod:    arTime lArTime" << arTime << lArTime;

      if ( arTime > lArTime )
      {  // If first set or new one younger than previous, start lists
         lmodFnams.clear();
         lmodGUIDs.clear();
         lmodVaris.clear();
         lmodMenis.clear();
         lmodDescs.clear();
         lMDescrs .clear();
         lArTime            = arTime;
      }

      else if ( arTime < lArTime )
      {  // If new one older than previous, skip it
         continue;
      }

      lmodFnams << modpath;             // Save full path file name
      lmodGUIDs << model.modelGUID;     // Save model GUID
      lmodVaris << model.variance;      // Save variance
      lmodMenis << model.meniscus;      // Save meniscus
      lmodDescs << model.description;   // Save description

      lmodd.description = model.description;
      lmodd.modelGUID   = model.modelGUID;
      lmodd.modelID     = "-1";
      lmodd.variance    = model.variance;
      lmodd.meniscus    = model.meniscus;
      lMDescrs << lmodd;
   }

   nlmods         = lMDescrs.size();
   qSort( lMDescrs );
DbgLv(1) << "RmvMod: nlmods" << nlmods << "msetBase" << msetBase;

   for ( int ii = 0; ii < nlmods; ii++ )
   {  // Scan to identify model in set with iteration index
      //  corresponding to best fit
      ModelDesc lmodd = lMDescrs[ ii ];
      QString descrip = QString( lmodd.description );
      QString ansysID = descrip.section( '.', -2, -2 );
      QString iterID  = ansysID.section( '_', -1, -1 );
      int iterx       = iterID .section( '-',  0,  0 )
                               .mid( 1 ).toInt() - 1;
DbgLv(1) << "RmvMod: best ndx scan: ii vari" << ii << lmodd.variance
 << "iterID iters" << iterID << iterx;

      if ( iterx == ix_setfit )
      {
         lkModx         = ii;
DbgLv(1) << "RmvMod:   best ndx scan:   lkModx" << lkModx;
      }
   }
DbgLv(1) << "RmvMod:  ix_setfit lkModx" << ix_setfit << lkModx;
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );

   // Make a list of fit models that match for DB, if possible
   if ( db_upd )
   {
      ModelDesc dmodd;
      QString   invID = QString::number( US_Settings::us_inv_ID() );

      if ( auto_mode )
	{
	  invID =  triple_information[ "invID" ];
	  qDebug() << "In remove_models(): invID -- " << invID;
	}
      
      QString   edtID = QString::number( idEdit );
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList query;
      QStringList modIDs;

      if ( idEdit > 0 )
         query << "get_model_desc_by_editID" << invID << edtID;

      else
         query << "get_model_desc" << invID;

DbgLv(1) << "RmvMod:  idEdit" << idEdit << "query" << query;
      db.query( query );

      while( db.next() )
      {
         QString modelID    = db.value( 0 ).toString();
         QString descript   = db.value( 2 ).toString();
         QString runID      = descript.section( '.',  0, -4 );
         if ( runID == srchRun )
         {
            modIDs << modelID;
         }
      }

DbgLv(1) << "RmvMod:  modIDs size" << modIDs.size();
      for ( int ii = 0; ii < modIDs.size(); ii++ )
      {
         QString modelID    = modIDs.at( ii );
         query.clear();
         query << "get_model_info" << modelID;
         db.query( query );

         if ( db.lastErrno() != US_DB2::OK )  continue;

         db.next();

         QString modelGUID  = db.value( 0 ).toString();
         QString descript1  = db.value( 1 ).toString();
         QString contents   = db.value( 2 ).toString();
         int jdtx           = contents.indexOf( "description=" );

         if ( jdtx < 1 )  continue;

         int jdx            = contents.indexOf( "\"", jdtx ) + 1;
         int lend           = contents.indexOf( "\"", jdx  ) - jdx;
         QString descript   = contents.mid( jdx, lend );

         // Skip model that is not from the right set
         if ( !msetBase.isEmpty()  &&
              !descript.startsWith( msetBase ) )
         {
DbgLv(1) << "RmvMod:  descript" << descript << "msetBase" << msetBase << "ii" << ii;
            continue;
         }

         double  variance   = db.value( 3 ).toString().toDouble();
         double  meniscus   = db.value( 4 ).toString().toDouble();
         QString runID      = descript.section( '.',  0, -4 );
         QString tripID     = descript.section( '.', -3, -3 );
         QString anRunID    = descript.section( '.', -2, -2 );
         QString editLabl   = anRunID .section( '_',  0, -5 );
//DbgLv(1) << "RmvMod:  scn1 ii runID editLabl tripID"
// << ii << runID << editLabl << tripID;

         if ( runID != srchRun  ||  editLabl != srchEdit  ||
              tripID != srchTrip )
         continue;    // Can't use if from a different runID or edit or triple

         QString iterID     = anRunID .section( '_', -1, -1 );
//DbgLv(1) << "RmvMod:    iterID" << iterID;

         if ( !iterID.contains( "-m" )  &&  !iterID.contains( "-b" ) )
         {
            continue;    // Can't use if not a fit-meniscus type
         }

         // Probably a file from the right set, but let's check for other sets
         int     arTime     = anRunID .section( '_', -4, -4 ).mid( 1 ).toInt();

         if ( arTime > dArTime )
         {  // If first set or new one younger than previous, start lists
            dmodIDs  .clear();
            dmodGUIDs.clear();
            dmodVaris.clear();
            dmodMenis.clear();
            dmodDescs.clear();
            dMDescrs .clear();
            dArTime            = arTime;
         }

         else if ( arTime < dArTime )
         {  // If new one older than previous, skip it
            continue;
         }

         dmodIDs   << modelID;             // Save model DB ID
         dmodGUIDs << modelGUID;           // Save model GUID
         dmodVaris << variance;            // Save variance
         dmodMenis << meniscus;            // Save meniscus
         dmodDescs << descript;            // Save description

         dmodd.description = descript;
         dmodd.modelGUID   = modelGUID;
         dmodd.modelID     = modelID;
         dmodd.variance    = variance;
         dmodd.meniscus    = meniscus;
         dMDescrs << dmodd;
DbgLv(1) << "RmvMod:  scn2 ii dmodDesc" << descript; 
      }

      ndmods         = dMDescrs.size();
      qSort( dMDescrs );

      if ( dArTime > lArTime )      // Don't count any older group
         nlmods         = 0;
      else if ( lArTime > dArTime )
         ndmods         = 0;
DbgLv(1) << "RmvMod: ndmods" << ndmods;

      for ( int ii = 0; ii < ndmods; ii++ )
      {  // Scan to identify model in set with lowest variance
         ModelDesc dmodd = dMDescrs[ ii ];
         QString descrip = QString( dmodd.description );
         QString ansysID = descrip.section( '.', -2, -2 );
         QString iterID  = ansysID.section( '_', -1, -1 );
         int iterx       = iterID .section( '-',  0,  0 )
                               .mid( 1 ).toInt() - 1;
DbgLv(1) << "RmvMod: best ndx scan: ii vari" << ii << dmodd.variance
 << "iterID iters" << iterID << iterx;

         if ( iterx == ix_setfit )
         {
            dkModx         = ii;
DbgLv(1) << "RmvMod:   best ndx scan:   dkModx" << dkModx;
         }
      }

      // Now, compare the findings for local versus database
      if ( nlmods == ndmods  ||  ( ndmods > 0 && nlmods == 0 ) )
      {
         int    nmatch  = 0;

         for ( int jj = 0; jj < nlmods; jj++ )
         {
            ModelDesc lmodd = lMDescrs[ jj ];
            ModelDesc dmodd = dMDescrs[ jj ];

            if ( lmodd.modelGUID   == dmodd.modelGUID &&
                 lmodd.description == dmodd.description )
               nmatch++;

            lmodGUIDs[ jj ]    = lmodd.modelGUID;
            lmodVaris[ jj ]    = lmodd.variance;
            lmodMenis[ jj ]    = lmodd.meniscus; 
            lmodDescs[ jj ]    = lmodd.description;
         }

         for ( int jj = 0; jj < ndmods; jj++ )
         {
            ModelDesc dmodd = dMDescrs[ jj ];
            dmodIDs  [ jj ]    = dmodd.modelID;
            dmodGUIDs[ jj ]    = dmodd.modelGUID;
            dmodVaris[ jj ]    = dmodd.variance;
            dmodMenis[ jj ]    = dmodd.meniscus; 
            dmodDescs[ jj ]    = dmodd.description;
         }

         if ( nmatch == nlmods )
         {  // OK if they match or local only
DbgLv(1) << "++local/dbase match, or local only";
         }

         else
         {  // Not good if they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH";
DbgLv(1) << "  nmatch ndmods nlmods" << nmatch << ndmods << nlmods;
            return;
         }
      }

      else if ( nlmods == 0 )
      {  // It is OK if there are no local records, when DB ones were found
DbgLv(1) << "++only dbase records exist";
      }

      else if ( ndmods == 0 )
      {  // It is OK if there are only local records, when local ones found
DbgLv(1) << "++only local records exist";
      }

      else
      {  // Non-zero local & DB, but they do not match
DbgLv(1) << "**local/dbase DO NOT MATCH in count";
DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
         return;
      }
   }

DbgLv(1) << "  nlmods ndmods" << nlmods << ndmods;
   if ( ndmods > 0  ||  nlmods > 0 )
   {  // There are models to scan, so build a list of models,noises to remove
      ModelDesc       rmodDescrs;
      NoiseDesc       rnoiDescrs;
      QStringList     rmodIDs;
      QStringList     rmodDescs;
      QStringList     rmodFnams;
      QStringList     rnoiIDs;
      QStringList     rnoiFnams;
      QStringList     rnoiDescs;
      QStringList     nieDescs;
      QStringList     nieIDs;
      QStringList     nieFnams;
      int             nlrmod = 0;
      int             ndrmod = 0;
      int             nlrnoi = 0;
      int             ndrnoi = 0;
      int             ntmods = ( ndmods > 0 ) ? ndmods : nlmods;
      int             ikModx = ( ndmods > 0 ) ? dkModx : lkModx;
DbgLv(1) << "  ntmods ikModx" << ntmods << ikModx;
//*DEBUG*
//if ( ndmods > 0  ||  nlmods > 0 ) return;
//*DEBUG*

      QString modDesc    = "";

      for ( int jj = 0; jj < ntmods; jj++ )
      {  // Build the list of model files and DB ids for removal
         if ( jj != ikModx )
         {
            int itix;
            int irix;
            QString fname;
            QString mDesc;
            QString mID;
            QString tiDesc;
            QString riDesc;
            QString noiID; 
            QString noiFname;

            if ( nlmods > 0 )
            {
               fname  = lmodFnams[ jj ];
               mDesc  = lmodDescs[ jj ];
               nlrmod++;
               if ( ndmods == 0 )
                  mID    = "-1";
            }

            if ( ndmods > 0 )
            {
               mID    = dmodIDs  [ jj ];
               mDesc  = dmodDescs[ jj ];
               ndrmod++;
               if ( nlmods == 0 )
                  fname  = "";
            }
            rmodIDs   << mID;
            rmodFnams << fname;
            rmodDescs << mDesc;
//DbgLv(1) << "RmvMod: jj" << jj << "mID" << mID << "mDesc" << mDesc;

            if ( modDesc.isEmpty() )
            {
               modDesc = mDesc;   // Save 1st model's description
DbgLv(1) << "RmvMod: 1st rmv-mod: jj modDesc" << jj << modDesc;
               // Build noises-in-edit lists for database and local
               noises_in_edit( modDesc, nieDescs, nieIDs, nieFnams );
            }

            tiDesc = QString( mDesc ).replace( ".model", ".ti_noise" )
                                     .replace( "2DSA-FM-IT", "2DSA-FM" )
                                     .replace( "2DSA-FB-IT", "2DSA-FB" );
            riDesc = QString( mDesc ).replace( ".model", ".ri_noise" )
                                     .replace( "2DSA-FM-IT", "2DSA-FM" )
                                     .replace( "2DSA-FB-IT", "2DSA-FB" );
            itix   = nieDescs.indexOf( tiDesc );
            irix   = nieDescs.indexOf( riDesc );
DbgLv(1) << "RmvMod:    itix irix" << itix << irix
 << "tiDesc" << tiDesc << "riDesc" << riDesc;

            if ( itix >= 0 )
            {  // There is a TI noise to remove
               noiID    = nieIDs  [ itix ];
               noiFname = nieFnams[ itix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << tiDesc;
            }

            if ( irix >= 0 )
            {  // There is an RI noise to remove
               noiID    = nieIDs  [ irix ];
               noiFname = nieFnams[ irix ];

               if ( noiID != "-1" )
                  ndrnoi++;

               if ( ! noiFname.isEmpty() )
                  nlrnoi++;
               else
                  noiFname = "";

               rnoiIDs   << noiID;
               rnoiFnams << noiFname;
               rnoiDescs << riDesc;
            }
         }
      }
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();

      nlnois             = nlrnoi + ( nlrnoi > nlrmod ? 2 : 1 );
      ndnois             = ndrnoi + ( ndrnoi > ndrmod ? 2 : 1 );
      bool rmv_mdls      = true;
DbgLv(1) << "RmvMod: nlrmod ndrmod nlrnoi ndrnoi nlnois ndnois"
 << nlrmod << ndrmod << nlrnoi << ndrnoi << nlnois << ndnois;
      if ( ck_confirm->isChecked() )
      {
         QString msg = tr( "%1 local model files;\n"
                           "%2 database model files;\n"
                           "%3 local noise files;\n"
                           "%4 database noise files;\n"
                           "have been identified for removal.\n\n"
                           "Do you really want to delete them?" )
            .arg( nlrmod ).arg( ndrmod ).arg( nlrnoi ).arg( ndrnoi );

         int response = QMessageBox::question( this,
               tr( "Remove Models and Noises?" ),
               msg, QMessageBox::Yes, QMessageBox::Cancel );
         rmv_mdls           = ( response == QMessageBox::Yes );
      }

      if ( rmv_mdls )
      {
         US_Passwd pw;
         US_DB2* dbP = db_upd ? new US_DB2( pw.getPasswd() ) : NULL;

         QStringList query;
         QString recID;
         QString recFname;
         QString recDesc;
DbgLv(1) << " Remove Models and Noises";
         for ( int ii = 0; ii < rmodIDs.size(); ii++ )
         {  // Remove models and db noises
            recID    = rmodIDs  [ ii ];
            recDesc  = rmodDescs[ ii ];
            recFname = rmodFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file model
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            if ( recID != "-1"  &&  dbP != NULL )
            {  // Delete model (and any child noise) from DB
               query.clear();
               query << "delete_model" << recID;
               int stat = dbP->statusQuery( query );
               if ( stat != 0 )
                  qDebug() << "delete_model error" << stat;
else DbgLv(1) << "     DB record deleted";
            }
         }

         if ( dbP != NULL )
         {
            delete dbP;
            dbP   = NULL;
         }

         for ( int ii = 0; ii < rnoiIDs.size(); ii++ )
         {  // Remove local noises
            recID    = rnoiIDs  [ ii ];
            recDesc  = rnoiDescs[ ii ];
            recFname = rnoiFnams[ ii ];
DbgLv(1) << "  Delete: " << recID << recFname.section("/",-1,-1) << recDesc;

            if ( ! recFname.isEmpty() )
            {  // Delete local file noise
               QFile recf( recFname );
               if ( recf.exists() )
               {
                  if ( recf.remove() )
{ DbgLv(1) << "     local file removed"; }
                  else { qDebug() << "*ERROR* removing" << recFname; }
               }
               else { qDebug() << "*ERROR* does not exist:" << recFname; }
            }

            // No need to remove noises from DB; model remove did that
         }
      }
   }

   else
   {  // No models were found!!! (huh!!!)
DbgLv(1) << "**NO local/dbase models-to-remove were found!!!!";
   }

   QApplication::restoreOverrideCursor();
   return;
}

// Create lists of information for noises that match a sample model from a set
void US_FitMeniscus::noises_in_edit( QString modDesc, QStringList& nieDescs,
      QStringList& nieIDs, QStringList& nieFnams )
{
   QString msetBase = modDesc.section( ".", 0, -3 ) + "." + 
                      modDesc.section( ".", -2, -2 ).section( "_", 0, 3 );
   QString srchTlab = msetBase.section( ".", -2, -2 );
   QString srchTrip = srchTlab.left( 1 ) + "." + srchTlab.mid( 1, 1 ) + "." +
                      srchTlab.mid( 2 ) + ".xml";
   QString srchRun  = msetBase.section( ".", 0, -3 ) + "." +
                      msetBase.section( ".", -1, -1 )
                      .section( "_", 0, 0 ).mid( 1 );
DbgLv(1) << "NIE: msetBase" << msetBase;
   if ( msetBase.contains( "2DSA-FM-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FM-IT", "2DSA-FM" );
   }
   if ( msetBase.contains( "2DSA-FB-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FB-IT", "2DSA-FB" );
   }
   if ( msetBase.contains( "2DSA-FMB-IT" ) )
   {
      msetBase     = msetBase.replace( "2DSA-FMB-IT", "2DSA-FMB" );
   }
DbgLv(1) << "NIE: msetBase" << msetBase;
   QStringList query;
   QString fname;
   QString noiID;
   int nlnois   = 0;
   bool db_upd  = dkdb_cntrls->db();
//*DEBUG*
//db_upd=false;
//*DEBUG*

   QStringList noifilt;
   noifilt << "N*.xml";
   QString     noidir   = US_Settings::dataDir() + "/noises";
   QStringList noifiles = QDir( noidir ).entryList(
         noifilt, QDir::Files, QDir::Name );
   noidir               = noidir + "/";
DbgLv(1) << "NIE: noise-files-size" << noifiles.size();

   for ( int ii = 0; ii < noifiles.size(); ii++ )
   {
      QString noiFname   = noifiles.at( ii );
      QString noiPath    = noidir + noiFname;
      US_Noise noise;
      
      if ( noise.load( noiPath ) != US_DB2::OK )
         continue;    // Can't use if can't load

      QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiDesc" << ii << noiDesc;

      if ( ! noiDesc.startsWith( msetBase ) )
         continue;    // Can't use if not from the model set

      nlnois++;

      nieDescs << noiDesc;
      nieFnams << noiPath;
DbgLv(1) << "NIE:     noiFname" << noiFname;

      if ( ! db_upd )
         nieIDs   << "-1";
   }

   if ( db_upd )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      QStringList nIDs;
      QString invID = QString::number( US_Settings::us_inv_ID() );

      if ( auto_mode )
	{
	  invID =  triple_information[ "invID" ];
	  qDebug() << "In noises_in_edits(): invID -- " << invID;
	}
      
      QStringList edtIDs;
      QStringList edtNams;

      query.clear();
      query << "all_editedDataIDs" << invID;
      db.query( query );

      while( db.next() )
      {
         QString edtID    = db.value( 0 ).toString();
         QString edtName  = db.value( 2 ).toString();

         if ( edtName.startsWith( srchRun )  &&
              edtName.endsWith(   srchTrip ) )
         {
            edtIDs  << edtID;
            edtNams << edtName;
DbgLv(1) << "NIE:  edtID edtName" << edtID << edtName;
         }
      }
DbgLv(1) << "NIE: edtIDs-size" << edtIDs.size();
if ( edtIDs.size() > 0 ) DbgLv(1) << "NIE: edtName0" << edtNams[0];

      query.clear();

      if ( edtIDs.size() == 1 )
         query << "get_noise_desc_by_editID" << invID << edtIDs[ 0 ];

      else
         query << "get_noise_desc" << invID;

      db.query( query );

      while( db.next() )
      {
         QString noiID    = db.value( 0 ).toString();
         QString edtID    = db.value( 2 ).toString();
DbgLv(1) << "NIE:  noiID edtID" << noiID << edtID;

         if ( edtIDs.contains( edtID ) )
            nIDs << noiID;
      }
DbgLv(1) << "NIE: nIDs-size" << nIDs.size() << "msetBase" << msetBase;

      for ( int ii = 0; ii < nIDs.size(); ii++ )
      {
         QString noiID      = nIDs[ ii ];
         US_Noise noise;
      
         if ( noise.load( noiID, &db ) != US_DB2::OK )
            continue;    // Can't use if can't load

         QString noiDesc    = noise.description;
DbgLv(1) << "NIE:  ii noiID noiDesc" << ii << noiID << noiDesc;

         if ( ! noiDesc.startsWith( msetBase ) )
            continue;    // Can't use if not from the model set

         nieIDs   << noiID;

         if ( nlnois == 0 )
         {
            nieFnams << "";
            nieDescs << noiDesc;
         }
      }
   }
DbgLv(1) << "NIE: db_upd" << db_upd << "nlnois" << nlnois
 << "nieDescs-size" << nieDescs.size() << "nieIDs-size" << nieIDs.size();

   return;
}

// Make a ColorMap copy and return a pointer to the new ColorMap
QwtLinearColorMap* US_FitMeniscus::ColorMapCopy( QwtLinearColorMap* colormap )
{
   QVector< double >  cstops   = colormap->colorStops();
   int                lstop    = cstops.count() - 1;
   QwtInterval        csvals( 0.0, 1.0 );
   QwtLinearColorMap* cmapcopy = new QwtLinearColorMap( colormap->color1(),
                                                        colormap->color2() );

   for ( int jj = 1; jj < lstop; jj++ )
   {
      QColor scolor = colormap->color( csvals, cstops[ jj ] );
      cmapcopy->addColorStop( cstops[ jj ], scolor );
   }

   return cmapcopy;
}

// Change plot type and related GUI elements
void US_FitMeniscus::change_plot_type( )
{
   dataPlotClear( meniscus_plot );

   if ( have3val )
   {  // Set up 3-D plot

     if ( auto_mode )
       {
         setWindowTitle( tr( "Fit Meniscus,Bottom from 2DSA Data" ) + ": " + triple_information[ "triple_name" ]  );
         meniscus_plot->setTitle    ( tr( "Meniscus,Bottom Fit" ) + ": " + triple_information[ "triple_name" ]  );
       }
     else
       {
         setWindowTitle( tr( "Fit Meniscus,Bottom from 2DSA Data" ) );
         meniscus_plot->setTitle    ( tr( "Meniscus,Bottom Fit" ) );
       }
     
      meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "2DSA Meniscus,Bottom RMSD" ) );
      meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Meniscus Radius" ) );
      meniscus_plot->enableAxis  ( QwtPlot::yRight,  true );
   }

   else
   {  // Set up 2-D plot

     if ( auto_mode )
       {
         setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) + ": " + triple_information[ "triple_name" ] );
         meniscus_plot->setTitle    ( tr( "Meniscus Fit" ) + ": " + triple_information[ "triple_name" ] );
       }
     else
       {
         setWindowTitle( tr( "Fit Meniscus from 2DSA Data" ) );
         meniscus_plot->setTitle    ( tr( "Meniscus Fit" ) );
       }
     
     
      meniscus_plot->setAxisTitle( QwtPlot::yLeft,   tr( "2DSA Meniscus,Bottom RMSD" ) );
      meniscus_plot->setAxisTitle( QwtPlot::xBottom, tr( "Meniscus Radius" ) );
      meniscus_plot->enableAxis  ( QwtPlot::yRight, false );
      meniscus_plot->setCanvasBackground( QBrush( US_GuiSettings::plotCanvasBG() ) );
   }

   // Turn on/off 2D elements
   lb_men_sel  ->setVisible( !have3val  );
   le_men_sel  ->setVisible( !have3val  );
   lb_rms_error->setVisible( !have3val  );
   le_rms_error->setVisible( !have3val  );
   lb_order    ->setVisible( !have3val  );
   sb_order    ->setVisible( !have3val  );

   // Turn on/off 3D elements
   lb_men_lor  ->setVisible( have3val );
   le_men_lor  ->setVisible( have3val );
   lb_men_fit  ->setVisible( have3val );
   le_men_fit  ->setVisible( have3val );
   lb_bot_lor  ->setVisible( have3val );
   le_bot_lor  ->setVisible( have3val );
   lb_bot_fit  ->setVisible( have3val );
   le_bot_fit  ->setVisible( have3val );
   lb_mprads   ->setVisible( have3val );
   le_mprads   ->setVisible( have3val );
   lb_zfloor   ->setVisible( have3val );
   ct_zfloor   ->setVisible( have3val );
}

// Set the fit models index for set fit meniscus/bottom
void US_FitMeniscus::index_model_setfit()
{
   if ( have3val )
   {  // Fit Mensicus + Bottom:  find closest x,y in list
      double xmeni  = le_men_fit->text().toDouble();
      double ybott  = le_bot_fit->text().toDouble();
      double lowxyd = 1.0e+99;
DbgLv(1) << "IMS:  xmeni ybott" << xmeni << ybott;

      for ( int ii = 0; ii < v_meni.count(); ii++ )
      {  // Find lowest difference in X,Y list
         double xydiff = sq( ( xmeni - v_meni[ ii ] ) ) +
                         sq( ( ybott - v_bott[ ii ] ) );
         xydiff        = ( xydiff == 0.0 ) ? 0.0 : sqrt( xydiff );

         if ( xydiff < lowxyd )
         {  // Save lowest difference so far and its index
            ix_setfit     = ii;
            lowxyd        = xydiff;
DbgLv(1) << "IMS:    ii" << ii << "lowxyd" << lowxyd;
         }
      }
   }

   else
   {  // Fit Meniscus OR Fit Bottom:  find close X in list
      double xselec = le_men_sel->text().toDouble();
      double lowxd  = 1.0e+99;
DbgLv(1) << "IMS:  xselec" << xselec << "v_meni count" << v_meni.count();

      for ( int ii = 0; ii < v_meni.count(); ii++ )
      {  // Find lowest difference in X list
         double xdiff  = qAbs( ( xselec - v_meni[ ii ] ) );

         if ( xdiff < lowxd )
         {  // Save lowest difference so far and its index
            ix_setfit     = ii;
            lowxd         = xdiff;
DbgLv(1) << "IMS:    ii" << ii << "lowxd" << lowxd << "v_meni-ii" << v_meni[ii];
         }
      }
   }
DbgLv(1) << "IMS: ix_setfit" << ix_setfit;

   return;
}

// Open the dialog to select a new investigator
void US_FitMeniscus::sel_investigator( void )
{
   if ( !dkdb_cntrls->db() )  return;   // Ignore if Disk selected

   int personID  = US_Settings::us_inv_ID();
   US_Investigator* inv_dialog = new US_Investigator( true, personID );

   connect( inv_dialog, SIGNAL( investigator_accepted( int ) ),
            this,       SLOT  ( assign_investigator  ( int ) ) );

   inv_dialog->exec();
}

// Assign this dialog's investigator text
void US_FitMeniscus::assign_investigator( int invID )
{
   QString number = ( invID > 0 ) ? QString::number( invID ) + ": " : "";

   le_invest->setText( number + US_Settings::us_inv_name() );
}

