#include <QApplication>
#include <QtSql>

#include "us_xpn_viewer_gui.h"
#include "us_tmst_plot.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_util.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_plot.h"
#include "us_math2.h"
#include "us_db2.h"
#include "us_passwd.h"
#include "us_investigator.h"
#include "us_constants.h"
#include "us_report.h"
#include "us_gui_util.h"
#include "us_util.h"
#include "us_crypto.h"
#include "us_editor.h"
#include "us_images.h"
#include "us_colorgradIO.h"
#include "qwt_legend.h"

#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#define AXISSCALEDIV(a)    data_plot->axisScaleDiv(a)
#define dPlotClearAll(a) a->clear()
#else
#include "qwt_picker_machine.h"
#define AXISSCALEDIV(a)    (QwtScaleDiv*)&data_plot->axisScaleDiv(a)
#define dPlotClearAll(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,true)
#endif

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//ALEXEY: reverted, based on v2802

// DialBox 
DialBox::DialBox( QWidget *parent ):
  QWidget( parent )
{
    d_dial = createDial();
    d_label = new QLabel( this );
    d_label->setAlignment( Qt::AlignCenter );
    d_label->setStyleSheet("font: bold;color: black;");
    
    QVBoxLayout *layout = new QVBoxLayout( this );;
    layout->setSpacing( 0 );
    layout->addWidget( d_dial, 15 );
    layout->addWidget( d_label );

    connect( d_dial, SIGNAL( valueChanged( double ) ), this, SLOT( setNum( double ) ) );
   
    setNum( d_dial->value() );
}

void DialBox::setSpeed( double v )
{
  d_dial->setValue( v );
}

void DialBox::setNum( double v )
{
    QString text;
    //text.setNum( v, 'f', 2 );
    text = "Rotor Speed: " + QString::number(v*1000) + " rpm";

    d_label->setText( text );
}

SpeedoMeter *DialBox::createDial( void ) const  
{
  SpeedoMeter *dial = new SpeedoMeter;

  dial->setScaleStepSize( 5 );
  dial->setScale( 0, 60 );
  dial->scaleDraw()->setPenWidth( 2 );
  dial->setValue(0);

  return dial;
}

// END of Dial box


// Speedometer
SpeedoMeter::SpeedoMeter( QWidget *parent ):
    QwtDial( parent ),
    d_label( "RPM" )
{
    QwtRoundScaleDraw *scaleDraw = new QwtRoundScaleDraw();
    scaleDraw->setSpacing( 8 );
    scaleDraw->enableComponent( QwtAbstractScaleDraw::Backbone, false );
    scaleDraw->setTickLength( QwtScaleDiv::MinorTick, 0 );
    scaleDraw->setTickLength( QwtScaleDiv::MediumTick, 4 );
    scaleDraw->setTickLength( QwtScaleDiv::MajorTick, 8 );
    setScaleDraw( scaleDraw );

    setWrapping( false );
    setReadOnly( true );

    setOrigin( 135.0 );
    setScaleArc( 0.0, 270.0 );

    
    QwtDialSimpleNeedle *needle = new QwtDialSimpleNeedle(
        QwtDialSimpleNeedle::Arrow, true, Qt::red,
        QColor( Qt::gray ).light( 130 ) );
    setNeedle( needle );
}

void SpeedoMeter::setLabel( const QString &label )
{
    d_label = label;
    update();
}

QString SpeedoMeter::label() const
{
    return d_label;
}

// Reloaded virtual function - needed to display units label (RPM)
void SpeedoMeter::drawScaleContents( QPainter *painter,
    const QPointF &center, double radius ) const
{
    QRectF rect( 0.0, 0.0, 2.0 * radius, 2.0 * radius - 10.0 );
    rect.moveCenter( center );

    const QColor color = palette().color( QPalette::Text );
    painter->setPen( color );

    const int flags = Qt::AlignBottom | Qt::AlignHCenter;
    painter->drawText( rect, flags, d_label );
}
// END of Speedometer calss

// WheelBox
WheelBox::WheelBox( Qt::Orientation orientation, QWidget *parent ): QWidget( parent )
{
    QWidget *box = createBox( orientation );
    d_label = new QLabel( this );
    d_label->setAlignment( Qt::AlignHCenter | Qt::AlignTop );
    d_label->setStyleSheet("font: bold;color: black;");

    QVBoxLayout *layout       = new QVBoxLayout( this );
    layout->addWidget( box, 15 );
    layout->addWidget( d_label );

    //connect( d_thermo, SIGNAL( valueChanged( double ) ), this, SLOT( setNum( double ) ) );
    
    setNum( d_thermo->value() );
}

QWidget *WheelBox::createBox( Qt::Orientation orientation ) 
{
    d_thermo = new QwtThermo();
    d_thermo->setOrientation( orientation );

    d_thermo->setScalePosition( QwtThermo::TrailingScale );

    QwtLinearColorMap *colorMap = new QwtLinearColorMap(); 
    colorMap->setColorInterval( Qt::blue, Qt::red );
    d_thermo->setColorMap( colorMap );

    double min = 0;
    double max = 40;

    d_thermo->setScale( min, max );
    d_thermo->setValue( min );

    QWidget *box = new QWidget();
    QBoxLayout *layout;
    layout = new QVBoxLayout( box );

    layout->addWidget( d_thermo, Qt::AlignCenter );
    
    return box;
}

void WheelBox::setTemp( double v )
{
  qDebug() << "Setting Temperature!!! Value: " << v ;
  d_thermo->setValue( v );

  setNum( v );
}

void WheelBox::setNum( double v )
{
  
    QString text;
    //text.setNum( v, 'f', 2 );

    text = "Temp.: " + QString::number( v, 'f', 1 ) + QChar(0x2103);
    d_label->setText( text );
}

//END of WheelBox


// Constructor for use in automated app
US_XpnDataViewer::US_XpnDataViewer(QString auto_mode) : US_Widgets()
{
   const QChar chlamb( 955 );

   setWindowTitle( tr( "Beckman Optima Data Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* settings    = new QGridLayout;
   QGridLayout* live_params = new QGridLayout;
   QGridLayout* time_params = new QGridLayout;

   auto_mode_bool     = true;
   experimentAborted  = false;
   
   navgrec      = 10;
   dbg_level    = US_Settings::us_debug();
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid     = fmet.maxWidth();
   int lwid     = fwid * 4;
   int swid     = lwid + fwid;
   isMWL        = false;
   isRaw        = true;
   haveData     = false;
   haveTmst     = false;
   xpn_data     = NULL;
   runID        = "";
   runType      = "RI";
   rlt_id       = 0;
   currentDir   = "";
   in_reload_auto    = false;
   in_reload_all_data  = false;
   in_reload_data_init = false;
   in_reload_check_sysdata  = false;

   ElapsedTimeOffset = 0;

   //ALEXEY: new way
   US_Passwd pw;
   US_DB2*   dbP = new US_DB2( pw.getPasswd() );
   if ( dbP != NULL )
     read_optima_machines( dbP );

   // for ( int ii = 0; ii < instruments.count(); ii++ )
   //   sl_optimas << QString::number( instruments[ ii ].ID )
   //     + ": " + instruments[ ii ].name;
   
   // cb_optima->clear();
   // cb_optima->addItems( sl_optimas );
   
   // connect( cb_optima,    SIGNAL( activated      ( int ) ),
   //          this,         SLOT  ( changeOptima   ( int ) ) );
   
   // changeOptima(0); 
   // /* End of Optima machines read */
   
   // Load controls     
   QLabel*      lb_run      = us_banner( tr( "Load the Run" ) );
   
                 pb_loadXpn  = us_pushbutton( tr( "Load Raw Optima Data" ) );
                 pb_loadAUC  = us_pushbutton( tr( "Load US3 AUC Data" ) );
                 pb_reset    = us_pushbutton( tr( "Reset Data" ) );
                 pb_details  = us_pushbutton( tr( "Data Details" ), true  );
                 pb_plot2d   = us_pushbutton( tr( "Refresh Plot" ) );
                 pb_saveauc  = us_pushbutton( tr( "Export openAUC" )  );
                 pb_showtmst = us_pushbutton( tr( "Show Time State" )  );
                 pb_reload   = us_pushbutton( tr( "Update Data" )      );
                 pb_colmap   = us_pushbutton( tr( "Color Map" )        );

   QLabel*      lb_dir      = us_label( tr( "Directory" ), -1 );
                 le_dir      = us_lineedit( "", -1, true );

   QLabel*      lb_dbhost   = us_label( tr( "DB Host" ), -1 );
                 le_dbhost   = us_lineedit( "", -1, true );

   QLabel*      lb_runID    = us_label( tr( "Run ID:" ), -1 );
                 le_runID    = us_lineedit( "", -1, false );

   QLabel*      lb_cellchn  = us_label( tr( "Cell/Channel:" ), -1 );
                cb_cellchn  = us_comboBox();

                le_colmap   = us_lineedit( "cm-rainbow", -1, true );

   int rhgt     = le_runID->height();

   // Plot controls     
   QLabel*      lb_prcntls  = us_banner( tr( "Plot Controls" ) );
   lb_prcntls->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

   QLabel*      lb_rstart   = us_label( tr( "Radius Start:"   ), -1 );
                 cb_rstart   = us_comboBox();
   QLabel*      lb_rend     = us_label( tr( "Radius End:"     ), -1 );
                 cb_rend     = us_comboBox();
   QLabel*      lb_lrange    = us_label( tr( "%1 Range:"   ).arg( chlamb ), -1 );
                le_lrange    = us_lineedit( "280 only", -1, true );
		ptype_mw     = tr( "Plot %1:"    ).arg( chlamb );
		ptype_tr     = tr( "Wavelength:" );
		prectype     = ptype_tr;

                lb_pltrec   = us_label( prectype, -1 );
                cb_pltrec   = us_comboBox();
   QLabel*      lb_optsys   = us_label( tr( "Optical System:" ), -1 );
                cb_optsys   = us_comboBox();

                pb_prev     = us_pushbutton( tr( "Previous" ) );
                pb_next     = us_pushbutton( tr( "Next" ) );
   us_checkbox( tr( "Always Auto-scale Y Axis" ), ck_autoscy, true );
   us_checkbox( tr( "Auto Update" ),              ck_autorld, false );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );


   QLabel*      lb_rinterv  = us_label( tr( "Update Interval Seconds:" ), -1 );
                ct_rinterv  = us_counter( 2, 10, 3600, 1 );
   ct_rinterv->setFont( sfont );
   ct_rinterv->setMinimumWidth( lwid );
   ct_rinterv->resize( rhgt, swid );
   ct_rinterv->setMinimum   (   10 );
   ct_rinterv->setMaximum   ( 3600 );
   ct_rinterv->setValue     (   60 );
   ct_rinterv->setSingleStep(    1 );

   // Scan controls     
   QLabel*      lb_scanctl  = us_banner( tr( "Scan Control" ) );
   QLabel*      lb_from     = us_label( tr( "From:" ) );
   QLabel*      lb_to       = us_label( tr( "To:" ) );
                ct_from     = us_counter( 3, 0, 500, 1 );
                ct_to       = us_counter( 3, 0, 500, 1 );
                pb_exclude  = us_pushbutton( tr( "Exclude Scan Range" ) );
                pb_include  = us_pushbutton( tr( "Include All Scans"  ) );
   ct_from  ->setFont( sfont );
   ct_from  ->setMinimumWidth( lwid );
   ct_from  ->resize( rhgt, swid );
   ct_to    ->setFont( sfont );
   ct_to    ->setMinimumWidth( lwid );
   ct_to    ->resize( rhgt, swid );
   ct_from  ->setValue( 0 );
   ct_to    ->setValue( 0 );
   ct_from  ->setSingleStep( 1 );
   ct_to    ->setSingleStep( 1 );


   // Status and standard pushbuttons
   QLabel*      lb_status   = us_banner( tr( "Status" ) );
   lb_status->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
                le_status   = us_lineedit( tr( "(no data loaded)" ), -1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );

   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Close" ) );

   pb_stop =  us_pushbutton( tr( "STOP" ) );
   pb_stop->setStyleSheet("font: bold;color: red;");

   pb_close  ->setEnabled(false);

      // Hide scan Control
   if ( auto_mode.toStdString() == "AUTO")
     {
       pb_close->hide();
       
       // Hide Load controls 
       lb_run     ->hide();
       pb_loadXpn ->hide();
       pb_loadAUC ->hide();
       pb_reset   ->hide();
       pb_details ->hide();
       pb_plot2d  ->hide();
       pb_saveauc ->hide();
       pb_showtmst->hide();
       pb_reload  ->hide();
       pb_colmap  ->hide();

       lb_dir     ->hide();
       le_dir     ->hide();
       lb_dbhost  ->hide();
       le_dbhost  ->hide();
       lb_runID   ->hide();
       le_runID   ->hide();
       
       le_colmap  ->hide();

       // Hide some plot controls
       lb_rstart  ->hide();
       cb_rstart  ->hide();
       lb_rend    ->hide();
       cb_rend    ->hide();
       ck_autoscy ->hide();
       ck_autorld ->hide();
       
       // Hide interval controls
       lb_rinterv->hide();
       ct_rinterv->hide();
       
       // Hide Scan controls
       lb_scanctl->hide();
       lb_from   ->hide();
       lb_to     ->hide();
       ct_from   ->hide();
       ct_to     ->hide();
       pb_exclude->hide();
       pb_include->hide();
      
     }
   
   // Default scan curve color list and count
   QString cmfpath          = US_Settings::etcDir() + "/cm-rainbow.xml";
 DbgLv(1) << "cmfpath" << cmfpath;
   US_ColorGradIO::read_color_gradient( cmfpath, mcolors );
   mcknt                    = mcolors.count();
DbgLv(1) << "mcolors count" << mcknt;
if(mcknt>0)
 DbgLv(1) << "mcolors c0,cn" << mcolors[0] << mcolors[mcknt-1];

   // Signals and Slots
   connect( pb_loadXpn,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_xpn_raw() ) );
   connect( pb_loadAUC,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_auc_xpn() ) );
   connect( pb_reset,     SIGNAL( clicked()      ),
            this,         SLOT  ( resetAll()     ) );
   connect( pb_details,   SIGNAL( clicked()      ),
            this,         SLOT  ( runDetails()   ) );
   connect( pb_saveauc,   SIGNAL( clicked()      ),
            this,         SLOT  ( export_auc()   ) );
   connect( pb_reload,    SIGNAL( clicked()      ),
            this,         SLOT  ( reloadData()   ) );
   connect( ck_autorld,   SIGNAL( clicked()      ),
            this,         SLOT  ( changeReload()             ) );
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh( )            ) );
   connect( cb_rstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_rend,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRecord( )            ) );
   connect( pb_prev,      SIGNAL( clicked()  ),
            this,         SLOT  ( prevPlot() ) );
   connect( pb_next,      SIGNAL( clicked()  ),
            this,         SLOT  ( nextPlot() ) );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude,   SIGNAL( clicked()       ),
            this,         SLOT  ( exclude_scans() ) );
   connect( pb_include,   SIGNAL( clicked()       ),
            this,         SLOT  ( include_scans() ) );
   connect( pb_plot2d,    SIGNAL( clicked()       ),
            this,         SLOT  ( changeCellCh()  ) );
   connect( pb_showtmst,  SIGNAL( clicked()       ),
            this,         SLOT  ( showTimeState() ) );
   connect( pb_colmap,    SIGNAL( clicked()        ),
            this,         SLOT  ( selectColorMap() ) );
   connect( ct_rinterv,   SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( changeInterval()       ) );
   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT  ( help()     ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT  ( close()    ) );

   // Do the left-side layout
   int row = 0;
   settings->addWidget( lb_run,        row++, 0, 1, 8 );
   settings->addWidget( lb_dir,        row++, 0, 1, 8 );
   settings->addWidget( le_dir,        row++, 0, 1, 8 );
   settings->addWidget( lb_dbhost,     row,   0, 1, 2 );
   settings->addWidget( le_dbhost,     row++, 2, 1, 6 );
   settings->addWidget( lb_runID,      row,   0, 1, 2 );
   settings->addWidget( le_runID,      row++, 2, 1, 6 );
   settings->addWidget( pb_loadXpn,    row,   0, 1, 4 );
   settings->addWidget( pb_loadAUC,    row++, 4, 1, 4 );
   settings->addWidget( pb_reset,      row,   0, 1, 4 );
   settings->addWidget( pb_details,    row++, 4, 1, 4 );
   settings->addWidget( pb_plot2d,     row,   0, 1, 4 );
   settings->addWidget( pb_saveauc,    row++, 4, 1, 4 );
   settings->addWidget( pb_reload,     row,   0, 1, 4 );
   settings->addWidget( ck_autorld,    row++, 4, 1, 4 );
   settings->addWidget( lb_rinterv,    row,   0, 1, 4 );
   settings->addWidget( ct_rinterv,    row++, 4, 1, 4 );
   settings->addWidget( lb_prcntls,    row++, 0, 1, 8 );
   settings->addWidget( lb_rstart,     row,   0, 1, 2 );
   settings->addWidget( cb_rstart,     row,   2, 1, 2 );
   settings->addWidget( lb_rend,       row,   4, 1, 2 );
   settings->addWidget( cb_rend,       row++, 6, 1, 2 );
   settings->addWidget( lb_optsys,     row,   0, 1, 4 );
   settings->addWidget( cb_optsys,     row++, 4, 1, 4 );
   settings->addWidget( lb_cellchn,    row,   0, 1, 4 );
   settings->addWidget( cb_cellchn,    row++, 4, 1, 4 );
   settings->addWidget( lb_lrange,     row,   0, 1, 4 );
   settings->addWidget( le_lrange,     row++, 4, 1, 4 );
   settings->addWidget( lb_pltrec,     row,   0, 1, 2 );
   settings->addWidget( cb_pltrec,     row,   2, 1, 2 );
   settings->addWidget( pb_prev,       row,   4, 1, 2 );
   settings->addWidget( pb_next,       row++, 6, 1, 2 );

   settings->addWidget( lb_status,     row,   0, 1, 4 );
   settings->addWidget( pb_stop,       row,   4, 1, 2 );
   settings->addWidget( pb_help,       row++, 6, 1, 2 );
   
   settings->addWidget( ck_autoscy,    row,   0, 1, 4 );
   settings->addWidget( pb_showtmst,   row++, 4, 1, 4 );
   settings->addWidget( pb_colmap,     row,   0, 1, 2 );
   settings->addWidget( le_colmap,     row++, 2, 1, 6 );
   settings->addWidget( lb_scanctl,    row++, 0, 1, 8 );
   settings->addWidget( lb_from,       row,   0, 1, 1 );
   settings->addWidget( ct_from,       row,   1, 1, 3 );
   settings->addWidget( lb_to,         row,   4, 1, 1 );
   settings->addWidget( ct_to,         row++, 5, 1, 3 );
   settings->addWidget( pb_exclude,    row,   0, 1, 4 );
   settings->addWidget( pb_include,    row++, 4, 1, 4 );

   //settings->addWidget( lb_status,     row++, 0, 1, 8 );

   settings->addWidget( le_status,     row++, 0, 1, 8 );
   //settings->addWidget( pb_help,       row,   0, 1, 8 );
   //settings->addWidget( pb_close,      row++, 4, 1, 4 );

   // Plot layout for the right side of window
//   QBoxLayout* plot = new US_Plot( data_plot,
   plot             = new US_Plot( data_plot,
                                   tr( "Intensity Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Intensity" ) );

   data_plot->setMinimumSize( 400, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );

   picker = new US_PlotPicker( data_plot );
   picker->setRubberBand     ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
            this, SLOT  ( currentRectf ( QRectF ) ) );

   //Live params (rpm speed, temp.)
   int row_params = 0;
   rpm_box = new DialBox( this );
   live_params->addWidget( rpm_box, row_params, 0, 1, 6 );

   temperature_box = new WheelBox( Qt::Vertical  );  // Blue/Red box
   live_params->addWidget( temperature_box, row_params++, 6, 1, 2 );

   QLabel*      lb_elapsed     = us_label( tr( "Elapsed Time:" ), -1 );
                le_elapsed     = us_lineedit( "00:00:00", -1, true );
   QLabel*      lb_running     = us_label( tr( "Running Time:" ), -1 );
                le_running     = us_lineedit( "00:00:00", -1, true );		
   QLabel*      lb_remaining   = us_label( tr( "Remaining Time:" ), -1 );
                le_remaining   = us_lineedit( "00:00:00", -1, true );

		
   time_params-> addWidget( lb_elapsed,   row_params,   0, 1, 2 );
   time_params-> addWidget( le_elapsed,   row_params,   2, 1, 2 );
   time_params-> addWidget( lb_running,   row_params,   4, 1, 2 );
   time_params-> addWidget( le_running,   row_params,   6, 1, 2 );
   time_params-> addWidget( lb_remaining, row_params,   8, 1, 2 );
   time_params-> addWidget( le_remaining, row_params++, 10, 1, 2 );
   
   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   left->addLayout( live_params );
   //left->addLayout( time_params );
   
   QVBoxLayout* right    = new QVBoxLayout;
   
   right->addLayout( plot );
   right->addLayout( time_params );

   QHBoxLayout* main = new QHBoxLayout;
   // main->setSpacing         ( 2 );
   // main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( right );

   main->setStretch( 0, 3 );
   main->setStretch( 1, 7 );
     
   // //Plots for Temp.
   // plot_temp             = new US_Plot( data_plot_temp, "", "", QString("Degrees, ")+QChar(0x2103));
  
   // //data_plot_temp->setMinimumSize( 50, 400 );

   // data_plot_temp->enableAxis( QwtPlot::xBottom, false );
   // data_plot_temp->enableAxis( QwtPlot::yLeft  , true );

   // data_plot_temp->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   // data_plot_temp->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );

   // picker_temp = new US_PlotPicker( data_plot_temp );
   // picker_temp->setRubberBand     ( QwtPicker::VLineRubberBand );
   // picker_temp->setMousePattern   ( QwtEventPattern::MouseSelect1,
   // 				    Qt::LeftButton, Qt::ControlModifier );

   // connect( plot_temp, SIGNAL( zoomedCorners( QRectF ) ),
   //          this, SLOT  ( currentRectf ( QRectF ) ) );

   
   //plot RPM
   plot_rpm             = new US_Plot( data_plot_rpm, "", tr( "Elapsed Time (minutes)" ),
                                                          tr( "RPM" ) );
   QFont tfont( US_GuiSettings::fontFamily(),
                US_GuiSettings::fontSize(),
                QFont::Bold );

   QwtText qwtTitleR( tr( "Temp." ) + DEGC );
   qwtTitleR.setFont( tfont );
   data_plot_rpm->setAxisTitle( QwtPlot::yRight , qwtTitleR );

   QwtLegend *legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   legend->setFont( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() - 1 ) );
   data_plot_rpm->insertLegend( legend, QwtPlot::BottomLegend  );

  
   //data_plot_rpm->setMinimumSize( 50, 400 );

   data_plot_rpm->enableAxis( QwtPlot::xBottom, true );
   data_plot_rpm->enableAxis( QwtPlot::yLeft  , true );
   data_plot_rpm->enableAxis( QwtPlot::yRight , true );

   data_plot_rpm->setAxisScale( QwtPlot::xBottom, 1.0, 14400.0 );
   data_plot_rpm->setAxisScale( QwtPlot::yLeft  , 0.0, 6e+4 );
   data_plot_rpm->setAxisScale( QwtPlot::yRight , 0.0, 40 );

   picker_rpm = new US_PlotPicker( data_plot_rpm );
   picker_rpm->setRubberBand     ( QwtPicker::VLineRubberBand );
   picker_rpm->setMousePattern   ( QwtEventPattern::MouseSelect1,
				   Qt::LeftButton, Qt::ControlModifier );

   grid_rpm          = us_grid( data_plot_rpm );

   //ALEXEY: curves for sysdata must be re-defined in the reset_auto() !!!
   curv_rpm  = us_curve( data_plot_rpm, "RPM" );
   curv_temp = us_curve( data_plot_rpm, "Temperature" );
   curv_temp->setYAxis     ( QwtPlot::yRight );

   QPen    pen_red ( Qt::red );
   QPen    pen_green ( Qt::green );

   curv_temp->setPen( pen_red);
   curv_rpm ->setPen( pen_green);
   /*************************************************************************/

   
   connect( plot_rpm, SIGNAL( zoomedCorners( QRectF ) ),
            this, SLOT  ( currentRectf ( QRectF ) ) );

   

   // QHBoxLayout* running_temp  = new QHBoxLayout;
   // running_temp->addLayout( plot_temp );
   QHBoxLayout* running_rpm  = new QHBoxLayout;
   running_rpm->addLayout( plot_rpm );

   // MAIN panel
   QVBoxLayout* supermain = new QVBoxLayout( this );
   supermain->setSpacing         ( 2 );
   supermain->setContentsMargins ( 2, 2, 2, 2 );     
   supermain->addLayout( main );
   //supermain->addLayout( running_temp );
   supermain->addLayout( running_rpm );

   // supermain->setStretch( 0, 7 );
   // supermain->setStretch( 1, 3 );

   reset();
   setMinimumSize( 950, 450 );
   adjustSize();

   
   // //1. Temporary test - C. Horne's experiment: ExpID = 408 (ExperimentDefinition, Postgres, Optima 1); ProtocolID = 43 (Us-lims, Mysql);
   // // description = CHorne-NanR_Trunc_2r-DNA-MWL_60K_111918;
   // // 2 cells; 41 wvls; 82 triples; 
   // QMap < QString, QString > protocol_details;
   // protocol_details["experimentId"] = QString("408");   
   // protocol_details["protocolName"] = QString("some_prot");
   // protocol_details[ "experimentName" ] = QString("some_name");
   // protocol_details[ "CellChNumber" ] = QString("2");
   // protocol_details[ "TripleNumber" ] = QString("82");
   // protocol_details[ "OptimaName" ] = QString("Optima 1");
   // protocol_details[ "duration" ]   = QString("32400");

   //2. Temporary test - S. Ling's experiment: ExpID = 465 (ExperimentDefinition, Postgres, Optima 1); ProtocolID = 150 (Us-lims, Mysql);
   // description = CCLing-PZ5077-27k-021519;
   // 2 cells; 1 wvls; 2 triples - for absorbance ONLY!! 
   // QMap < QString, QString > protocol_details;
   // protocol_details["experimentId"] = QString("465");   
   // protocol_details["protocolName"] = QString("some_prot");
   // protocol_details[ "experimentName" ] = QString("some_name");
   // protocol_details[ "CellChNumber" ] = QString("2");
   // protocol_details[ "TripleNumber" ] = QString("2");
   // protocol_details[ "OptimaName" ] = QString("Optima 1"); 
   // protocol_details[ "duration" ]   = QString("27000");
   
   //3. Temporary test - Harmen's experiment:  ExpID = 286 (ExperimentDefinition, Postgres, Optima 2); ProtocolID = 286 (Us-lims, Mysql);
   // description = HarmenS_20180417_Purity_Run_6;
   // 8 cells (16 channels); 29 wvls; 29 triples - for absorbance ONLY!! 
   // QMap < QString, QString > protocol_details;
   // protocol_details["experimentId"] = QString("286");   
   // protocol_details["protocolName"] = QString("HarmenS_20180417_Purity_Run_6");
   // protocol_details[ "experimentName" ] = QString("some_name");
   // protocol_details[ "CellChNumber" ] = QString("16");
   // protocol_details[ "TripleNumber" ] = QString("29");
   // protocol_details[ "OptimaName" ] = QString("Optima 2"); 
   // protocol_details[ "duration" ]   = QString("43200");
      

   // check_for_data( protocol_details );
   // End of test
   
}


// Regular constructor
US_XpnDataViewer::US_XpnDataViewer() : US_Widgets()
{
   const QChar chlamb( 955 );

   setWindowTitle( tr( "Beckman Optima Data Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* settings = new QGridLayout;
   
   auto_mode_bool = false;
   experimentAborted  = false;
   
   navgrec      = 10;
   dbg_level    = US_Settings::us_debug();
   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   int fwid     = fmet.maxWidth();
   int lwid     = fwid * 4;
   int swid     = lwid + fwid;
   isMWL        = false;
   isRaw        = true;
   haveData     = false;
   haveTmst     = false;
   xpn_data     = NULL;
   runID        = "";
   runType      = "RI";
   rlt_id       = 0;
   currentDir   = "";
   in_reload    = false;

   //ALEXEY: old way, from .conf file
   // QStringList xpnentr = US_Settings::defaultXpnHost();
   // DbgLv(1) << "xpnentr count" << xpnentr.count();
   
   // if ( xpnentr.count() == 0 )
   //   {
   //     xpnentr << "test-host" << "bcf.uthscsa.edu" << "5432";
    
   //     QMessageBox::warning( this,
   // 			     tr( "No Optima Host Entry" ),
   // 			     tr( "A default Optima Host entry is being used.\n"
   // 				 "You should add entries via Preferences:Optima Host Preferences\n"
   // 				 "as soon as possible" ) );
   //   }
   // else
   //   DbgLv(1) << "xpnentr ..." << xpnentr;
   // QString encpw;
   // QString decpw;
   // QString encpw0;
   // QString encpw1;
   // QString masterpw;
   // US_Passwd pw;
   // xpndesc      = xpnentr.at( 0 );
   // xpnhost      = xpnentr.at( 1 );
   // xpnport      = xpnentr.at( 2 );
   // xpnname      = xpnentr.at( 3 );
   // xpnuser      = xpnentr.at( 4 );
   // encpw        = xpnentr.at( 5 );
   // encpw0       = encpw.section( "^", 0, 0 );
   // encpw1       = encpw.section( "^", 1, 1 );
   // masterpw     = pw.getPasswd();
   // xpnpasw      = US_Crypto::decrypt( encpw0, masterpw, encpw1 );


   // Load controls     
   QLabel*      lb_run      = us_banner( tr( "Load the Run" ) );

                pb_loadXpn  = us_pushbutton( tr( "Load Raw Optima Data" ) );
                pb_loadAUC  = us_pushbutton( tr( "Load US3 AUC Data" ) );
                pb_reset    = us_pushbutton( tr( "Reset Data" ) );
                pb_details  = us_pushbutton( tr( "Data Details" ), true  );
                pb_plot2d   = us_pushbutton( tr( "Refresh Plot" ) );
                pb_saveauc  = us_pushbutton( tr( "Export openAUC" )  );
                pb_showtmst = us_pushbutton( tr( "Show Time State" )  );
                pb_reload   = us_pushbutton( tr( "Update Data" )      );
                pb_colmap   = us_pushbutton( tr( "Color Map" )        );

   QLabel*      lb_dir      = us_label( tr( "Directory" ), -1 );
                le_dir      = us_lineedit( "", -1, true );

   QLabel*      lb_dbhost   = us_label( tr( "DB Host" ), -1 );
                //le_dbhost   = us_lineedit( "", -1, true );
   cb_optima           = new QComboBox( this );                                // New
   QLabel*      lb_optima_connected = us_label( tr( "Connection Status: " ) ); //New
   le_optima_connected = us_lineedit( "", 0, true );                           //New
   
   //ALEXEY: new way <--------------------------------------------- //New
   US_Passwd pw;
   US_DB2*   dbP = new US_DB2( pw.getPasswd() );
   if ( dbP != NULL )
     read_optima_machines( dbP );

   for ( int ii = 0; ii < instruments.count(); ii++ )
     sl_optimas << instruments[ ii ][ "ID" ] 
       + ": " + instruments[ ii ][ "name" ];
   
   cb_optima->clear();
   cb_optima->addItems( sl_optimas );
   
   connect( cb_optima,    SIGNAL( activated      ( int ) ),
            this,         SLOT  ( changeOptima   ( int ) ) );
   
   changeOptima(0); 
   /* End of Optima machines read                                      //New    */ 

		
   QLabel*      lb_runID    = us_label( tr( "Run ID:" ), -1 );
                le_runID    = us_lineedit( "", -1, false );

   QLabel*      lb_cellchn  = us_label( tr( "Cell/Channel:" ), -1 );
                cb_cellchn  = us_comboBox();

                le_colmap   = us_lineedit( "cm-rainbow", -1, true );

   int rhgt     = le_runID->height();
   ptype_mw     = tr( "Plot %1:"    ).arg( chlamb );
   ptype_tr     = tr( "Plot Triple:" );
   prectype     = ptype_tr;

   // Plot controls     
   QLabel*      lb_prcntls  = us_banner( tr( "Plot Controls" ) );
   QLabel*      lb_rstart   = us_label( tr( "Radius Start:"   ), -1 );
                cb_rstart   = us_comboBox();
   QLabel*      lb_rend     = us_label( tr( "Radius End:"     ), -1 );
                cb_rend     = us_comboBox();
   QLabel*      lb_lrange   = us_label( tr( "%1 Range:"   ).arg( chlamb ), -1 );
                le_lrange   = us_lineedit( "280 only", -1, true );
                lb_pltrec   = us_label( prectype, -1 );
                cb_pltrec   = us_comboBox();
   QLabel*      lb_optsys   = us_label( tr( "Optical System:" ), -1 );
                cb_optsys   = us_comboBox();

                pb_prev     = us_pushbutton( tr( "Previous" ) );
                pb_next     = us_pushbutton( tr( "Next" ) );
   us_checkbox( tr( "Always Auto-scale Y Axis" ), ck_autoscy, true );
   us_checkbox( tr( "Auto Update" ),              ck_autorld, false );
   pb_prev->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
   pb_next->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
   QLabel*      lb_rinterv  = us_label( tr( "Update Interval Seconds:" ), -1 );
                ct_rinterv  = us_counter( 2, 10, 3600, 1 );
   ct_rinterv->setFont( sfont );
   ct_rinterv->setMinimumWidth( lwid );
   ct_rinterv->resize( rhgt, swid );
   ct_rinterv->setMinimum   (   10 );
   ct_rinterv->setMaximum   ( 3600 );
   ct_rinterv->setValue     (   60 );
   ct_rinterv->setSingleStep(    1 );

   // Scan controls     
   QLabel*      lb_scanctl  = us_banner( tr( "Scan Control" ) );
   QLabel*      lb_from     = us_label( tr( "From:" ) );
   QLabel*      lb_to       = us_label( tr( "To:" ) );
                ct_from     = us_counter( 3, 0, 500, 1 );
                ct_to       = us_counter( 3, 0, 500, 1 );
                pb_exclude  = us_pushbutton( tr( "Exclude Scan Range" ) );
                pb_include  = us_pushbutton( tr( "Include All Scans"  ) );
   ct_from  ->setFont( sfont );
   ct_from  ->setMinimumWidth( lwid );
   ct_from  ->resize( rhgt, swid );
   ct_to    ->setFont( sfont );
   ct_to    ->setMinimumWidth( lwid );
   ct_to    ->resize( rhgt, swid );
   ct_from  ->setValue( 0 );
   ct_to    ->setValue( 0 );
   ct_from  ->setSingleStep( 1 );
   ct_to    ->setSingleStep( 1 );

   // Status and standard pushbuttons
   QLabel*      lb_status   = us_banner( tr( "Status" ) );
                le_status   = us_lineedit( tr( "(no data loaded)" ), -1, true );
   QPalette stpal;
   stpal.setColor( QPalette::Text, Qt::white );
   stpal.setColor( QPalette::Base, Qt::blue  );
   le_status->setPalette( stpal );

   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Close" ) );

   // Default scan curve color list and count
   QString cmfpath          = US_Settings::etcDir() + "/cm-rainbow.xml";
 DbgLv(1) << "cmfpath" << cmfpath;
   US_ColorGradIO::read_color_gradient( cmfpath, mcolors );
   mcknt                    = mcolors.count();
DbgLv(1) << "mcolors count" << mcknt;
if(mcknt>0)
 DbgLv(1) << "mcolors c0,cn" << mcolors[0] << mcolors[mcknt-1];

   // Signals and Slots
   connect( pb_loadXpn,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_xpn_raw() ) );
   connect( pb_loadAUC,   SIGNAL( clicked()      ),
            this,         SLOT  ( load_auc_xpn() ) );
   connect( pb_reset,     SIGNAL( clicked()      ),
            this,         SLOT  ( resetAll()     ) );
   connect( pb_details,   SIGNAL( clicked()      ),
            this,         SLOT  ( runDetails()   ) );
   connect( pb_saveauc,   SIGNAL( clicked()      ),
            this,         SLOT  ( export_auc()   ) );
   connect( pb_reload,    SIGNAL( clicked()      ),
            this,         SLOT  ( reloadData()   ) );
   connect( ck_autorld,   SIGNAL( clicked()      ),
            this,         SLOT  ( changeReload()             ) );
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh( )            ) );
   connect( cb_rstart,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_rend,      SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRadius( )            ) );
   connect( cb_pltrec,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeRecord( )            ) );
   connect( pb_prev,      SIGNAL( clicked()  ),
            this,         SLOT  ( prevPlot() ) );
   connect( pb_next,      SIGNAL( clicked()  ),
            this,         SLOT  ( nextPlot() ) );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   connect( pb_exclude,   SIGNAL( clicked()       ),
            this,         SLOT  ( exclude_scans() ) );
   connect( pb_include,   SIGNAL( clicked()       ),
            this,         SLOT  ( include_scans() ) );
   connect( pb_plot2d,    SIGNAL( clicked()       ),
            this,         SLOT  ( changeCellCh()  ) );
   connect( pb_showtmst,  SIGNAL( clicked()       ),
            this,         SLOT  ( showTimeState() ) );
   connect( pb_colmap,    SIGNAL( clicked()        ),
            this,         SLOT  ( selectColorMap() ) );
   connect( ct_rinterv,   SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( changeInterval()       ) );
   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT  ( help()     ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT  ( close()    ) );

   // Do the left-side layout
   int row = 0;
   settings->addWidget( lb_run,        row++, 0, 1, 8 );
   settings->addWidget( lb_dir,        row++, 0, 1, 8 );
   settings->addWidget( le_dir,        row++, 0, 1, 8 );

   settings->addWidget( lb_dbhost,     row,   0, 1, 2 );
   //settings->addWidget( le_dbhost,     row++, 2, 1, 6 );
   settings->addWidget( cb_optima,     row++, 2, 1, 6 );      //New

   settings->addWidget( lb_optima_connected,     row,   0, 1, 2 );      //New
   settings->addWidget( le_optima_connected,     row++, 2, 1, 6 );      //New
   
   settings->addWidget( lb_runID,      row,   0, 1, 2 );
   settings->addWidget( le_runID,      row++, 2, 1, 6 );
   settings->addWidget( pb_loadXpn,    row,   0, 1, 4 );
   settings->addWidget( pb_loadAUC,    row++, 4, 1, 4 );
   settings->addWidget( pb_reset,      row,   0, 1, 4 );
   settings->addWidget( pb_details,    row++, 4, 1, 4 );
   settings->addWidget( pb_plot2d,     row,   0, 1, 4 );
   settings->addWidget( pb_saveauc,    row++, 4, 1, 4 );
   settings->addWidget( pb_reload,     row,   0, 1, 4 );
   settings->addWidget( ck_autorld,    row++, 4, 1, 4 );
   settings->addWidget( lb_rinterv,    row,   0, 1, 4 );
   settings->addWidget( ct_rinterv,    row++, 4, 1, 4 );
   settings->addWidget( lb_prcntls,    row++, 0, 1, 8 );
   settings->addWidget( lb_rstart,     row,   0, 1, 2 );
   settings->addWidget( cb_rstart,     row,   2, 1, 2 );
   settings->addWidget( lb_rend,       row,   4, 1, 2 );
   settings->addWidget( cb_rend,       row++, 6, 1, 2 );
   settings->addWidget( lb_optsys,     row,   0, 1, 4 );
   settings->addWidget( cb_optsys,     row++, 4, 1, 4 );
   settings->addWidget( lb_cellchn,    row,   0, 1, 2 );
   settings->addWidget( cb_cellchn,    row,   2, 1, 2 );
   settings->addWidget( lb_lrange,     row,   4, 1, 2 );
   settings->addWidget( le_lrange,     row++, 6, 1, 2 );
   settings->addWidget( lb_pltrec,     row,   0, 1, 2 );
   settings->addWidget( cb_pltrec,     row,   2, 1, 2 );
   settings->addWidget( pb_prev,       row,   4, 1, 2 );
   settings->addWidget( pb_next,       row++, 6, 1, 2 );
   settings->addWidget( ck_autoscy,    row,   0, 1, 4 );
   settings->addWidget( pb_showtmst,   row++, 4, 1, 4 );
   settings->addWidget( pb_colmap,     row,   0, 1, 2 );
   settings->addWidget( le_colmap,     row++, 2, 1, 6 );
   settings->addWidget( lb_scanctl,    row++, 0, 1, 8 );
   settings->addWidget( lb_from,       row,   0, 1, 1 );
   settings->addWidget( ct_from,       row,   1, 1, 3 );
   settings->addWidget( lb_to,         row,   4, 1, 1 );
   settings->addWidget( ct_to,         row++, 5, 1, 3 );
   settings->addWidget( pb_exclude,    row,   0, 1, 4 );
   settings->addWidget( pb_include,    row++, 4, 1, 4 );
   settings->addWidget( lb_status,     row++, 0, 1, 8 );
   settings->addWidget( le_status,     row++, 0, 1, 8 );
   settings->addWidget( pb_help,       row,   0, 1, 4 );
   settings->addWidget( pb_close,      row++, 4, 1, 4 );

   // Plot layout for the right side of window
//   QBoxLayout* plot = new US_Plot( data_plot,
   plot             = new US_Plot( data_plot,
                                   tr( "Intensity Data" ),
                                   tr( "Radius (in cm)" ), 
                                   tr( "Intensity" ) );

   data_plot->setMinimumSize( 600, 400 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft  , true );

   data_plot->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );

   picker = new US_PlotPicker( data_plot );
   picker->setRubberBand     ( QwtPicker::VLineRubberBand );
   picker->setMousePattern   ( QwtEventPattern::MouseSelect1,
                               Qt::LeftButton, Qt::ControlModifier );

   connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
            this, SLOT  ( currentRectf ( QRectF ) ) );

   // Now let's assemble the page
   
   QVBoxLayout* left     = new QVBoxLayout;

   left->addLayout( settings );
   
   QVBoxLayout* right    = new QVBoxLayout;
   
   right->addLayout( plot );

   QHBoxLayout* main = new QHBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   main->addLayout( left );
   main->addLayout( right );

   main->setStretch( 0, 2 );
   main->setStretch( 1, 4 );

   reset();
   adjustSize();

}



void US_XpnDataViewer::reset( void )
{
   runID         = "";
   currentDir    = US_Settings::importDir() + "/" + runID;
   cb_cellchn ->disconnect();
   cb_cellchn ->clear();
   le_dir     ->setText( currentDir );
   le_runID   ->setText( runID );
   //le_dbhost  ->setText( xpnhost + ":" + xpnport + "   (" + xpndesc + ")" );       //New

   pb_loadXpn ->setEnabled( true );
   pb_loadAUC ->setEnabled( true );
   pb_details ->setEnabled( false );
   pb_reload  ->setEnabled( false );
   ck_autorld ->setEnabled( true  );
   cb_cellchn ->setEnabled( false );
   cb_rstart  ->setEnabled( false );
   cb_rend    ->setEnabled( false );
   cb_pltrec  ->setEnabled( false );
   pb_prev    ->setEnabled( false );
   pb_next    ->setEnabled( false );
   ct_from    ->setEnabled( false );
   ct_to      ->setEnabled( false );
   pb_exclude ->setEnabled( false );
   pb_include ->setEnabled( false );
   pb_reset   ->setEnabled( false );
   ct_from    ->setEnabled( false );
   ct_to      ->setEnabled( false );
   pb_exclude ->setEnabled( false );
   pb_include ->setEnabled( false );
   pb_plot2d  ->setEnabled( false );
   pb_saveauc ->setEnabled( false );
   pb_showtmst->setEnabled( false );
//   pb_movie2d->setEnabled( false );

   // Clear any data structures
   allData   .clear();
   lambdas   .clear();
   r_radii   .clear();
   excludes  .clear();
   runInfo   .clear();
   cellchans .clear();
   triples   .clear();
   haveData      = false;

   dPlotClearAll( data_plot );
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );
   grid          = us_grid( data_plot );
   data_plot->replot();

 
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );
//   connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
//            this, SLOT  ( currentRectf ( QRectF ) ) );

   last_xmin     = -1.0;
   last_xmax     = -1.0;
   last_ymin     = -1.0;
   last_ymax     = -1.0;
   xpn_data      = ( xpn_data == NULL ) ? new US_XpnData() : xpn_data;

   connect( xpn_data, SIGNAL( status_text  ( QString ) ),
            this,     SLOT  ( status_report( QString ) ) );

   xpn_data->clear();
   le_status->setText( tr( "(no data loaded)" ) );

}

void US_XpnDataViewer::reset_auto( void )
{
   runID         = "";
   currentDir    = US_Settings::importDir() + "/" + runID;
   cb_cellchn ->disconnect();
   cb_cellchn ->clear();
   le_dir     ->setText( currentDir );
   le_runID   ->setText( runID );
   //le_dbhost  ->setText( xpnhost + ":" + xpnport + "   (" + xpndesc + ")" );       //New

   pb_loadXpn ->setEnabled( true );
   pb_loadAUC ->setEnabled( true );
   pb_details ->setEnabled( false );
   pb_reload  ->setEnabled( false );
   ck_autorld ->setEnabled( true  );
   cb_cellchn ->setEnabled( false );
   cb_rstart  ->setEnabled( false );
   cb_rend    ->setEnabled( false );
   cb_pltrec  ->setEnabled( false );
   pb_prev    ->setEnabled( false );
   pb_next    ->setEnabled( false );
   ct_from    ->setEnabled( false );
   ct_to      ->setEnabled( false );
   pb_exclude ->setEnabled( false );
   pb_include ->setEnabled( false );
   pb_reset   ->setEnabled( false );
   ct_from    ->setEnabled( false );
   ct_to      ->setEnabled( false );
   pb_exclude ->setEnabled( false );
   pb_include ->setEnabled( false );
   pb_plot2d  ->setEnabled( false );
   pb_saveauc ->setEnabled( false );
   pb_showtmst->setEnabled( false );
//   pb_movie2d->setEnabled( false );

   // Clear any data structures
   allData   .clear();
   lambdas   .clear();
   r_radii   .clear();
   excludes  .clear();
   runInfo   .clear();
   cellchans .clear();
   triples   .clear();
   haveData      = false;

   dPlotClearAll( data_plot );
   picker   ->disconnect();
   data_plot->setAxisScale( QwtPlot::xBottom, 5.8,  7.2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 0.0, 5e+4 );
   grid          = us_grid( data_plot );
   data_plot->replot();

   //ALEXEY: also reset sys data plot/temperature bar/RPM speedometer
   dPlotClearAll( data_plot_rpm );
   picker_rpm   ->disconnect();
   data_plot_rpm->setAxisScale( QwtPlot::xBottom, 1.0, 14400.0 );
   data_plot_rpm->setAxisScale( QwtPlot::yLeft  , 0.0, 6e+4 );
   data_plot_rpm->setAxisScale( QwtPlot::yRight , 0.0, 40 );
   grid_rpm      = us_grid( data_plot_rpm );
   data_plot_rpm->replot();
   //Curves for RPM/Temp.
   curv_rpm  = us_curve( data_plot_rpm, "RPM" );
   curv_temp = us_curve( data_plot_rpm, "Temperature" );
   curv_temp->setYAxis     ( QwtPlot::yRight );
   QPen    pen_red ( Qt::red );
   QPen    pen_green ( Qt::green );
   curv_temp->setPen( pen_red);
   curv_rpm ->setPen( pen_green);
   
   //RPM/Temp.
   rpm_box->setSpeed(0);
   temperature_box->setTemp(0);
   //times
   le_elapsed  ->setText("00:00:00");
   le_remaining->setText("00:00:00");
   le_running  ->setText("00:00:00");
   
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );
//   connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
//            this, SLOT  ( currentRectf ( QRectF ) ) );

   last_xmin     = -1.0;
   last_xmax     = -1.0;
   last_ymin     = -1.0;
   last_ymax     = -1.0;
   xpn_data      = ( xpn_data == NULL ) ? new US_XpnData() : xpn_data;

   connect( xpn_data, SIGNAL( status_text  ( QString ) ),
            this,     SLOT  ( status_report( QString ) ) );

   xpn_data->clear();
   le_status->setText( tr( "(no data loaded)" ) );

}

// Slot to read all Optima machines <------------------------------- // New
void US_XpnDataViewer::read_optima_machines( US_DB2* db )
{
  QStringList q( "" );
  q.clear();
  q  << QString( "get_instrument_names" )
     << QString::number( 1 );
  db->query( q );
  
  if ( db->lastErrno() == US_DB2::OK )      // If not, no instruments defined
    {
      QList< int > instrumentIDs;
      
      // Grab all the IDs so we can reuse the db connection
      while ( db->next() )
	{
	  int ID = db->value( 0 ).toString().toInt();
	  instrumentIDs << ID;
	  
	  qDebug() << "InstID: " << ID;
	}
      
      // Instrument information
      foreach ( int ID, instrumentIDs )
	{
	  QMap<QString,QString> instrument;
	  
	  q.clear();
	  q  << QString( "get_instrument_info_new" )
	     << QString::number( ID );
	  db->query( q );
	  db->next();

	  instrument[ "ID" ]              =   QString::number( ID );
	  instrument[ "name" ]            =   db->value( 0 ).toString();
	  instrument[ "serial" ]          =   db->value( 1 ).toString();
	  instrument[ "optimaHost" ]      =   db->value( 5 ).toString();	   
	  instrument[ "optimaPort" ]      =   db->value( 6 ).toString(); 
	  instrument[ "optimaDBname" ]    =   db->value( 7 ).toString();	   
	  instrument[ "optimaDBusername" ] =  db->value( 8 ).toString();	   
	  instrument[ "optimaDBpassw" ]    =  db->value( 9 ).toString();	   
	  instrument[ "selected" ]        =   db->value( 10 ).toString();
	    
	  instrument[ "opsys1" ]  = db->value( 11 ).toString();
	  instrument[ "opsys2" ]  = db->value( 12 ).toString();
	  instrument[ "opsys3" ]  = db->value( 13 ).toString();

	  instrument[ "radcalwvl" ]  =  db->value( 14 ).toString();
	  instrument[ "chromoab" ]   =  db->value( 15 ).toString();

	  
	  if ( instrument[ "name" ].contains("Optima") || instrument[ "optimaHost" ].contains("AUC_DATA_DB") )
	    this->instruments << instrument;
	}
    }
  qDebug() << "Reading Instrument: FINISH";
}

// Slot to select in Optima in use        <----------------------- // New   
void US_XpnDataViewer::changeOptima( int ndx )
{
   cb_optima->setCurrentIndex( ndx );
   QString coptima     = cb_optima->currentText();
   QString descr       = coptima.section( ":", 1, 1 ).simplified();

   for ( int ii = 0; ii < instruments.size(); ii++ )
     {
       QString name = instruments[ii][ "name" ].trimmed();
       if ( name == descr )
	 currentInstrument = instruments[ii];
     }

   xpndesc     = currentInstrument[ "name" ];
   xpnhost     = currentInstrument[ "optimaHost" ];
   xpnport     = currentInstrument[ "optimaPort" ];
   xpnname     = currentInstrument[ "optimaDBname" ];
   xpnuser     = currentInstrument[ "optimaDBusername" ];
   xpnpasw     = currentInstrument[ "optimaDBpassw" ];

   test_optima_connection();
}

//Slot to test Optima connection when Optima selection changed <------------- //New
void US_XpnDataViewer::test_optima_connection()
{

   qDebug() << "Optima in use: name, host, port, dbname, dbuser, dbpasw: " << xpndesc << " " << xpnhost << " "
   	    << xpnport << " "  << xpnname << " " << xpnuser << " " << xpnpasw ;


   QPalette orig_pal = le_optima_connected->palette();
   
   if ( xpnhost.isEmpty() || xpnport.isEmpty()
	|| xpnname.isEmpty() || xpnuser.isEmpty() || xpnpasw.isEmpty()  )
     {
       le_optima_connected->setText( "disconnected" );
       QPalette *new_palette = new QPalette();
       new_palette->setColor(QPalette::Text,Qt::red);
       new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
       le_optima_connected->setPalette(*new_palette);

       return;
     }
   
   US_XpnData* xpn_data1 = new US_XpnData();
   bool o_connected           = xpn_data1->connect_data( xpnhost, xpnport.toInt(), xpnname, xpnuser,  xpnpasw );
   xpn_data1->close();
   delete xpn_data1;
      
   if ( o_connected )
     {
       le_optima_connected->setText( "connected" );
       QPalette *new_palette = new QPalette();
       new_palette->setColor(QPalette::Text, Qt::darkGreen);
       new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
       le_optima_connected->setPalette(*new_palette);
     }
   else
     {
       le_optima_connected->setText( "disconnected" );
       QPalette *new_palette = new QPalette();
       new_palette->setColor(QPalette::Text,Qt::red);
       new_palette->setColor(QPalette::Base, orig_pal.color(QPalette::Base));
       le_optima_connected->setPalette(*new_palette);
     }
}


// Slot to select Optima in use by name for autoflow    <----------------------- // New   
void US_XpnDataViewer::selectOptimaByName_auto( QString OptimaName )
{
  for ( int ii = 0; ii < instruments.size(); ii++ )
    {
      QString name = instruments[ii][ "name" ].trimmed();
      if ( name == OptimaName )
	currentInstrument = instruments[ii];
    }
  
  xpndesc     = currentInstrument[ "name" ];
  xpnhost     = currentInstrument[ "optimaHost" ];
  xpnport     = currentInstrument[ "optimaPort" ];
  xpnname     = currentInstrument[ "optimaDBname" ];
  xpnuser     = currentInstrument[ "optimaDBusername" ];
  xpnpasw     = currentInstrument[ "optimaDBpassw" ];
}


void US_XpnDataViewer::resetAll( void )
{
   if ( allData.size() > 0 )
   {
      int status = QMessageBox::information( this,
               tr( "New Data Warning" ),
               tr( "This will erase all data currently on the screen, and " 
                   "reset the program to its starting condition. No hard-drive "
                   "data or database information will be affected. Proceed? " ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return;
   }

   reset();

   runID           = "";
   data_plot->setTitle( tr( "Intensity Data" ) );
}


// Enable the common dialog controls based on the presence of data
void US_XpnDataViewer::enableControls( void )
{
   const QChar chlamb( 955 );

   if ( allData.size() == 0 )
   {  // If no data yet, just reset
      reset();
      return;
   }

   // Enable and disable controls now
   pb_loadXpn ->setEnabled( false );
   pb_loadAUC ->setEnabled( false );
   pb_reset   ->setEnabled( true );
   pb_details ->setEnabled( true );
   pb_reload  ->setEnabled( true );
   cb_cellchn ->setEnabled( true );
   cb_rstart  ->setEnabled( true );
   cb_rend    ->setEnabled( true );
   cb_pltrec  ->setEnabled( true );
   pb_prev    ->setEnabled( true );
   pb_next    ->setEnabled( true );
   pb_plot2d  ->setEnabled( true );
   pb_saveauc ->setEnabled( true );
   pb_showtmst->setEnabled( haveTmst );
//   pb_movie2d->setEnabled( true );
   ct_from    ->setEnabled( true );
   ct_to      ->setEnabled( true );
   pb_exclude ->setEnabled( true );

   ncellch     = cellchans.count();
   nlambda     = lambdas  .count();
   ntriple     = triples  .count();
   nscan       = allData[ 0 ].scanCount();
   npoint      = allData[ 0 ].pointCount();
   ntpoint     = nscan * npoint;
   int ktrip   = ncellch * nlambda;
   isMWL       = ( nlambda > 2  &&  ntriple == ktrip  &&  ntriple > 48 );
   cb_cellchn ->setEnabled( isMWL );

DbgLv(1) << "ec: ncc nwl nsc npt ntpt" << ncellch << nlambda << nscan
 << npoint << ntpoint << "Mwl" << isMWL;
DbgLv(1) << "ec: npoint" << npoint << "radsize" << r_radii.count();
DbgLv(1) << "ec: nlambda" << nlambda << "lmbsize" << lambdas.count();
DbgLv(1) << "ec: ntriple" << ntriple << "trpsize" << triples.count() << "ktrip" << ktrip;
   QStringList slrads;
   QStringList sllmbs;
   QStringList plrecs;

   for ( int jj = 0; jj < npoint; jj++ )
      slrads << QString().sprintf( "%.3f", r_radii[ jj ] );

   for ( int jj = 0; jj < nlambda; jj++ )
      sllmbs << QString::number( lambdas[ jj ] );

   if ( isMWL )
   {
      prectype    = ptype_mw;
      plrecs      = sllmbs;
   }
   else
   {
      prectype    = ptype_tr;
      for ( int jj = 0; jj < ntriple; jj++ )
         plrecs << QString( triples[ jj ] ).replace( " ", "" );
   }

   lb_pltrec->setText( prectype );
   connect_ranges( false );
   cb_cellchn->clear();
   cb_rstart ->clear();
   cb_rend   ->clear();
   cb_pltrec ->clear();

   cb_cellchn->addItems( cellchans );
   cb_rstart ->addItems( slrads );
   cb_rend   ->addItems( slrads );
   cb_pltrec ->addItems( plrecs );

   if ( nlambda == 1 )
      le_lrange ->setText( sllmbs[ 0 ] + tr( " only" ) );
   else if ( nlambda > 1 )
      le_lrange ->setText( sllmbs[ 0 ] + tr( " to " ) 
                         + sllmbs[ nlambda - 1 ] );

   cb_cellchn->setCurrentIndex( 0 );
   cb_rstart ->setCurrentIndex( 0 );
   cb_rend   ->setCurrentIndex( npoint - 1 );
   connect_ranges( true );

   have_rngs  = false;
   compute_ranges( );

   ct_from   ->setMaximum( nscan );
   ct_to     ->setMaximum( nscan );
   cb_pltrec ->setCurrentIndex( nlambda / 2 );
   qApp->processEvents();

DbgLv(1) << "ec: call changeCellCh";
   changeCellCh();                          // Force a plot initialize
}

// Load Optima raw (.postgres) data
bool US_XpnDataViewer::load_xpn_raw_auto( )
{
  bool status_ok = false;

  if ( in_reload_data_init )             // If already doing a reload,
    return status_ok;                   //  skip starting a new one
  
  in_reload_data_init   = true;

  // Ask for data directory
  QString dbhost    = xpnhost;
  int     dbport    = xpnport.toInt();
  if ( xpn_data->connect_data( dbhost, dbport, xpnname, xpnuser, xpnpasw ) )
    {
      if ( dbg_level > 0 )
	xpn_data->dump_tables();

      // Implement: query ExperiementRun and based on ExpID build array of RunIDs, find the bigger (the latest) and call it RunID_to_retrieve
      RunID_to_retrieve = QString::number(xpn_data->get_runid( ExpID_to_use ));

      qDebug() << "RunID_to_retrieve 1: " << RunID_to_retrieve;

      // runInfo.clear();
      
      //xpn_data->scan_runs( runInfo );                          // ALEXEY initial query (for us_comproject needs to be based on ExpId ) 
      // xpn_data->filter_runs( runInfo );                        // ALEXEY Optima data filtering by type [Absorbance, Interference etc.]

      // for ( int ii = 0; ii < runInfo.count(); ii++ )
      // 	{
      // 	  QString delim_t       = QString( runInfo[ 0 ] ).left( 1 );
      // 	  QString rDesc_t       = runInfo[ ii ];
      // 	  QString lRunID_t      = QString( rDesc_t ).mid( 1 ).section( delim_t, 0, 0 );

      // 	  qDebug() << "FIRST: runInfo: delim, rDesc, lRunID: " << delim_t << ", " << rDesc_t << ", " << lRunID_t;
      // 	}
    }
  else
    {
      runInfo.clear();
    }
  
  // Check if there are non-zero data 
  //if ( runInfo.size() < 1 )
  if ( RunID_to_retrieve.toInt() == 0) 
    {
      // QMessageBox::information( this,
      // 				tr( "Status" ),
      // 				tr( "Run was submitted to the Optima, but not launched yet. \n"
      // 				    "Awaiting for data to emerge... \n" ) );
      // OR Message on connection to Optima: BUT it should be connected here as experiment has just been submitted...
      status_ok = false;
    }
  else 
    {
      status_ok = true;
      timer_data_init->stop();
      disconnect(timer_data_init, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
      msg_data_avail->accept();
      //msg_data_avail->close();
      //ok_msg_data->click();
      
      //ALEXEY: need to update 'autoflow' table with the unique RunID_to_retrieve && Start Run Time fields !!!
      //Conditional:  Do it ONLY once !!! 
      update_autoflow_runId_timeStarted();

      //ALEXEY: retrieve startTime from autoflow table:
      ElapsedTimeOffset = read_timeElapsed_offset();

      qDebug() << "Elapsed Time Offset as read form autoflow table DB:" << ElapsedTimeOffset;
       
      //ALEXEY: elapsed timer start
      elapsed_timer = new QElapsedTimer;
      elapsed_timer->start();
      
      qDebug() << "RunID_to_retrieve 2: " << RunID_to_retrieve;

      runInfo.clear();
      xpn_data->scan_runs_auto( runInfo, RunID_to_retrieve );                          // ALEXEY initial query (for us_comproject needs to be based on ExpId ) 
      
      for ( int ii = 0; ii < runInfo.count(); ii++ )
	{
	  QString delim_t       = QString( runInfo[ 0 ] ).left( 1 );
	  QString rDesc_t       = runInfo[ ii ];
	  QString lRunID_t      = QString( rDesc_t ).mid( 1 ).section( delim_t, 0, 0 );
	  
	  qDebug() << "SECOND: runInfo: delim, rDesc, lRunID: " << delim_t << ", " << rDesc_t << ", " << lRunID_t;
	}

      
      //ALEXEY: Start another timer - SysData (RPM, Temp.) - should be run in a separate thread!!!
      //Plus to check_for_sys_Data: elapsed time counter!!!
      temp_data.clear();
      rpm_data.clear();
      time_data.clear();
      
    // Check if all triple info is available
      timer_all_data_avail = new QTimer;
      connect(timer_all_data_avail, SIGNAL(timeout()), this, SLOT( retrieve_xpn_raw_auto ( ) ));
      timer_all_data_avail->start(5000);     // 5 sec



      // timer_check_sysdata = new QTimer(this);
      // connect(timer_check_sysdata, SIGNAL(timeout()), this, SLOT(  check_for_sysdata( )  ));
      // timer_check_sysdata->start(2000);     //

      // OR
      //Alternativly: put it in separate thread:
      //QThread* sys_thread = new QThread(this);
      sys_thread = new QThread(this);
      timer_check_sysdata = new QTimer(0); // parent to 0 !
      timer_check_sysdata->setInterval(2000);
      timer_check_sysdata->moveToThread(sys_thread);
      //connect( timer_check_sysdata, SIGNAL(timeout()), this, SLOT( check_for_sysdata( )  ), Qt::QueuedConnection ) ; //Qt::DirectConnection );
      connect( timer_check_sysdata, SIGNAL(timeout()), this, SLOT( check_for_sysdata( )  ) );//, Qt::QueuedConnection );
      //QThread's started() SIGNAL: before the run()/exec() function is called!!! Is this a potential issue, timer is started from a thread???
      connect( sys_thread, SIGNAL( started() ), timer_check_sysdata, SLOT( start() ));
      connect( sys_thread, SIGNAL( finished() ), timer_check_sysdata, SLOT( stop() ));
      sys_thread->start();
      // How to stop sys_thread?
      
            
      // // Check if all triple info is available
      // timer_all_data_avail = new QTimer;
      // connect(timer_all_data_avail, SIGNAL(timeout()), this, SLOT( retrieve_xpn_raw_auto ( ) ));
      // timer_all_data_avail->start(5000);     // 5 sec

    }
  
  in_reload_data_init   = false;
  return status_ok;
}


// When Optima run started & runID aqcuried, update 'autoflow' table
void US_XpnDataViewer::update_autoflow_runId_timeStarted( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }

   QStringList qry;
   qry << "update_autoflow_runid_starttime"
       << ExpID_to_use
       << RunID_to_retrieve;

   db->query( qry );
   //ALEXEY: Updates with runID && timeStarted only once, when runID && runStarttime IS NULL
}

// Read diffference btw started tiem and NOW() as an offset for Elapsed time
int US_XpnDataViewer::read_timeElapsed_offset( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   int time_offset = 0;
   
   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return time_offset;
     }
   
   QStringList qry;
   qry << "read_autoflow_times_mod"
       << RunID_to_retrieve;

   // //Test only
   // qry << "read_autoflow_times_mod_test";
     
   time_offset = db->functionQuery( qry );

   return time_offset;
}

//Delete autoflow record upon Run abortion
void US_XpnDataViewer::delete_autoflow_record( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }

   QStringList qry;
   qry << "delete_autoflow_record"
       << RunID_to_retrieve;

   //db->query( qry );

   // OR
   
   int status = db->statusQuery( qry );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
     {
       QMessageBox::warning( this,
			     tr( "Autoflow Record Not Deleted" ),
			     tr( "No autoflow record\n"
				 "associated with this experiment." ) );
       return;
     }
}
   

//Delete autoflow record upon Run abortion
void US_XpnDataViewer::updateautoflow_record_atLiveUpdate( void )
{

   details_at_live_update[ "runID" ] = RunID_to_retrieve;
  
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       return;
     }

   QStringList qry;
   qry << "update_autoflow_at_live_update"
       << RunID_to_retrieve
       << currentDir;

   db->query( qry );

   details_at_live_update[ "dataPath" ] = currentDir;

   //ALEXEY: if there is NO chromatic Abber. data, set corrRadii in autoflow record to 'NO'
   if ( correctRadii == "NO" )
     {
       qry.clear();
       qry << "update_autoflow_at_live_update_radiicorr"
	   << RunID_to_retrieve;

       db->query( qry );

       details_at_live_update[ "correctRadii" ] = QString("NO"); //currentDir;
     }

   //ALEXEY: if run was aborted manually from the Optima panel, set expAborted to 'YES'
   if ( experimentAborted )
     {
       qry.clear();
       qry << "update_autoflow_at_live_update_expaborted"
	   << RunID_to_retrieve;

       db->query( qry );

       details_at_live_update[ "expAborted" ] = QString("YES");
     }
   
   // // OR

   // qDebug() << "Trying to Update Autoflow table AT LIVE_UPDATE!!";
   // int status = db->statusQuery( qry );
   
   // if ( status == US_DB2::NO_AUTOFLOW_RECORD )
   //   {
   //     QMessageBox::warning( this,
   // 			     tr( "Autoflow Record Not Updated" ),
   // 			     tr( "No autoflow record\n"
   // 				 "associated with RunID %1 exists!" ).arg( RunID_to_retrieve ) );
   //     return;
   //   }
}
   

// Check periodically for SysData
void US_XpnDataViewer::check_for_sysdata( void )
{
  qDebug() << "sys_timer IS RUNNING here: ";
  qDebug() << "sys_timer IS RUNNING here: in_reload_check_sysdata " << in_reload_check_sysdata;
  
  if ( in_reload_check_sysdata )            // If already doing a reload,
    return;                                //  skip starting a new one
  
  in_reload_check_sysdata   = true;          // Flag in the midst of a reload

  int idrun = RunID_to_retrieve.toInt();
    
  //xpn_data->update_isysrec( idrun );

  int exp_time = 0;
  double temperature=0;
  int rpm;
  //int etimoff;
  //int stage_number;
  while ( temperature == 0 ) // what if the temperature is actually set to zero degrees?
  {
    xpn_data->update_isysrec( idrun );
    exp_time       = xpn_data->countOf_sysdata( "exp_time"  ).toInt();     //time form the start
  //stage_number   = xpn_data->countOf_sysdata( "stage_number" ).toInt();  //stage number
    temperature    = xpn_data->countOf_sysdata( "tempera" ).toDouble();    //temperature 
    rpm            = xpn_data->countOf_sysdata( "last_rpm"  ).toInt();     //revolutions per minute !
    //etimoff        = xpn_data->countOf_sysdata( "etim_off"  ).toInt();     //experimental time offset 
  }

  // Update rmp, temperature GUI icons...
  //RPM speed
  double rpm_for_meter = double(rpm/1000.0);
  rpm_box->setSpeed(rpm_for_meter);
  qApp->processEvents();
  
  //Temperature
  temperature_box->setTemp(temperature);
  qApp->processEvents();
  
  //Running Time
  QList< int > dhms_r;
  int running_time = exp_time;// + etimoff; //ALEXEY: OR elapsed_time + etimoff? OR just exp_time ???? //ALEXEY: etimoff < 0 ?
  if ( running_time < 0 )
    running_time = 0;
  timeToList( running_time, dhms_r );
  QString running_time_text;
  //ALEXEY: hh:mm:ss - OR do we need dd:hh:mm instead ?
  //ALEXEY: if times >~1 day, update #hours
  if ( dhms_r[0] > 0 )
    dhms_r[1] += 24;
  
  running_time_text = QString::number(dhms_r[1]) + ":" + QString::number(dhms_r[2]) + ":" + QString::number(dhms_r[3]);
  le_running->setText( running_time_text );
  qApp->processEvents();

  //Elapsed Time
  qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Elapsed Time Offset as read form autoflow table DB:" << ElapsedTimeOffset;

  QList< int > dhms_e;
  int elapsed_time = int( elapsed_timer->elapsed() / 1000 ) + ElapsedTimeOffset;
  int elapsed_time_1 = elapsed_time;
  timeToList( elapsed_time, dhms_e );
  QString elapsed_time_text;
  //ALEXEY: hh:mm:ss - OR do we need dd:hh:mm instead ?
  //ALEXEY: if times >~1 day, update #hours
  if ( dhms_e[0] > 0 )
    dhms_e[1] += 24;
  
  elapsed_time_text = QString::number(dhms_e[1]) + ":" + QString::number(dhms_e[2]) + ":" + QString::number(dhms_e[3]);
  le_elapsed->setText( elapsed_time_text );
  qApp->processEvents();
  
  //Remaining Time
  QList< int > dhms_remain;
  int remaining_time = TotalDuration.toInt() - ( exp_time ); //+ etimoff );
  timeToList( remaining_time, dhms_remain );
  QString remaining_time_text;
  //ALEXEY: hh:mm:ss - OR do we need dd:hh:mm instead ?
  //ALEXEY: if times >~1 day, update #hours
  if ( dhms_remain[0] > 0 )
    dhms_remain[1] += 24;
    
  remaining_time_text = QString::number(dhms_remain[1]) + ":" + QString::number(dhms_remain[2]) + ":" + QString::number(dhms_remain[3]);
  le_remaining->setText( remaining_time_text );
  qApp->processEvents();

  //RPM/Temp. Plots:

  //counter_mins += 500; //temporary for testing only

  rpm_data.push_back(rpm);
  time_data.push_back( double(elapsed_time_1/60.0) );  // Running time in minutes
  temp_data.push_back(temperature);
  
  //time_data.push_back( double(elapsed_time_1/60.0) );  // Running time in minutes

  // //Debugs for RPM, Temp., Time
  //qDebug() << "Sizes of arrays: RMP.size(), temperature.size() " <<  rpm_data.size() << ", " << temp_data.size(); 
     
  double* d_rpm          = rpm_data.data();
  double* d_temp         = temp_data.data();
  double* d_time         = time_data.data();
  
  //axis ranges, temporary
  double rpm_min = 0;
  double rpm_max = rpm + 5000;
  double temp_min = temperature - 1;
  if(temp_min < 0)
    temp_min = 0;
  double temp_max = temperature + 1;

  //if (temp_data.size() != 0 )
  curv_temp->setSamples( d_time, d_temp, time_data.size() );
  curv_rpm->setSamples(  d_time, d_rpm,  time_data.size() );   
  
  //data_plot_rpm->setAxisScale( QwtPlot::xBottom, 0.0, double(elapsed_time_1/60.0) );  // <-- HERE
  data_plot_rpm->setAxisScale( QwtPlot::xBottom, double(ElapsedTimeOffset/60.0), double(elapsed_time_1/60.0) );  // 
  //data_plot_rpm->setAxisScale( QwtPlot::xBottom, 0.0, double(exp_time/60.0 + counter_mins ) ); // for testing only
  data_plot_rpm->setAxisScale( QwtPlot::yLeft, rpm_min, rpm_max );     //Y-RPM 
  data_plot_rpm->setAxisScale( QwtPlot::yRight, temp_min, temp_max );  //Y-Temp.
  
  //ALEXEY: no plot rescaling to bounds...

  qDebug() << "SYS_STAT: BEFORE replot(), BEFORE CheExpStat!! ";
   
  data_plot_rpm->replot();
  qApp->processEvents();

  qDebug() << "SYS_STAT: After replot(), BEFORE CheExpStat!! ";
  
  int exp_status = CheckExpComplete_auto( RunID_to_retrieve  );
   
  if ( exp_status == 5 || exp_status == 0 )
    {
      if ( exp_status == 0)
	experimentAborted  = true;
      
      //timer_check_sysdata->stop();
      //ALEXEY: This timer cannot be stopped from another thread, but can be dealt with signal/slot upon Qthread termination..
      //        disconnection maybe enough...
      disconnect(timer_check_sysdata, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
      //Maybe add this?
      sys_thread->quit();     // ALEXEY: I think this emits Qthread's finished() signal... (connected to stopping timer_sysdata)
      qApp->processEvents();  // <-- IMPORTANT to process event loop while stopping thread!!!
      
      if ( sys_thread->isFinished() )
	qDebug() << "QThread STOPPED !!! ";
      
      if ( !timer_check_sysdata->isActive() )
	qDebug() << "QTimer timer_check_sysdata STOPPED by quitting the QThread !!! ";

      qDebug() << "ExpStat: 5/0  - sys_timer STOPPED here: ";
      
      rpm_box->setSpeed( 0 );
      le_remaining->setText( "00:00:00" );
      qApp->processEvents();
      
      if ( !timer_all_data_avail->isActive() ) // Check if reload_data Timer is stopped
	{
	  if ( !timer_data_reload->isActive() )
	    {
	      qDebug() << "TRY PROCEED INTO == 5/0 from check_for_sys_data()....";

	      export_auc_auto();
	      
	      // QString mtitle_complete  = tr( "Complete!" );
	      // QString message_done     = tr( "Experiment was completed. Optima data saved..." );
	      // QMessageBox::information( this, mtitle_complete, message_done );
	      
	      updateautoflow_record_atLiveUpdate();
	      //emit experiment_complete_auto( currentDir, ProtocolName, invID_passed, correctRadii  );  // Updtade later: what should be passed with signal ??

	      reset_auto();
	      emit experiment_complete_auto( details_at_live_update );
	      
	      return;
	    }
	}
    }
  
  
  // if ( exp_status == 0 ) //ALEXEY should be == 3 as per documentation
  //   {
  //     experimentAborted  = true;
  //     //timer_check_sysdata->stop();
  //     disconnect(timer_check_sysdata, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
  //     sys_thread->quit(); // ALEXEY: does this emit Qthread's finished() signal??
  //     qDebug() << "ExpStat: 3  - sys_timer STOPPED here: ";
      
  //     rpm_box->setSpeed( 0 );
  //     le_remaining->setText( "00:00:00" );
      
  //     if ( !timer_all_data_avail->isActive() ) // Check if reload_data Timer is stopped
  // 	{
  // 	  if ( !timer_data_reload->isActive() )
  // 	    {
  // 	      // Ask if data retrived so far should be saved:
	      
  // 	      QMessageBox msgBox;
  // 	      msgBox.setText(tr("Experiment was aborted!"));
  // 	      msgBox.setInformativeText("The data retrieved so far can be saved or disregarded. If saved, the program will proceed to the next stage (Editing). Otherwise, it will return to the initial stage (Experiment), all data will be lost.");
  // 	      msgBox.setWindowTitle(tr("Experiment Abortion"));
  // 	      QPushButton *Save      = msgBox.addButton(tr("Save Data"), QMessageBox::YesRole);
  // 	      QPushButton *Ignore    = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
	      
  // 	      msgBox.setIcon(QMessageBox::Question);
  // 	      msgBox.exec();
	      
  // 	      if (msgBox.clickedButton() == Save)
  // 		{
  // 		  export_auc_auto();
		  
  // 		  QString mtitle_complete  = tr( "Complete!" );
  // 		  QString message_done     = tr( "Experiment was completed. Optima data saved..." );
  // 		  QMessageBox::information( this, mtitle_complete, message_done );
		  
  // 		  updateautoflow_record_atLiveUpdate();
  // 		  emit experiment_complete_auto( currentDir, ProtocolName, invID_passed, correctRadii  );  // Updtade later: what should be passed with signal ??
  // 		  return;
  // 		}
	      
  // 	      else if (msgBox.clickedButton() == Ignore)
  // 		{
  // 		  reset();
  // 		  delete_autoflow_record();
  // 		  emit return_to_experiment( ProtocolName  ); 
  // 		  return;
  // 		}
  // 	    }  
  // 	}
  //   }
  

   qDebug() << "sys_timer RAN here: ";
   in_reload_check_sysdata   = false;
  
  //qDebug() << "sys_timer RAN here: ";
}


// Function to convert from a time in sec. to days, hours, minutes, seconds 
void US_XpnDataViewer::timeToList( int& sectime, QList< int >& dhms )
{
   int t_day = (int)( sectime / (24*3600) );
   sectime -= t_day * 24 * 3600;

   int t_hour = (int)( sectime / 3600 );
   sectime -= t_hour * 3600;

   int t_minute = (int)( sectime / 60 );
   sectime -= t_minute * 60;

   int t_second = sectime;

   qDebug() << "TimeToList(): DD: " << t_day;
   qDebug() << "TimeToList(): HH: " << t_hour;
   qDebug() << "TimeToList(): MM: " << t_minute;
   qDebug() << "TimeToList(): SS: " << t_second;
   
   dhms.clear();
   dhms << t_day << t_hour << t_minute << t_second;
}

//Query for Optima DB periodically, see if data available
void US_XpnDataViewer::check_for_data( QMap < QString, QString > & protocol_details)
{
  xpn_data->setEtimOffZero(); //ALEXEY: intialize etimoff to zero for the first time

  experimentAborted  = false;
  counter_mins = 0;
  ElapsedTimeOffset = 0;
  
  ExpID_to_use = protocol_details["experimentId"];   
  ProtocolName = protocol_details["protocolName"];
  RunName      = protocol_details[ "experimentName" ];
  CellChNumber = protocol_details[ "CellChNumber" ];
  TripleNumber = protocol_details[ "TripleNumber" ];
  OptimaName   = protocol_details[ "OptimaName" ];               //New
  TotalDuration = protocol_details[ "duration" ];
  invID_passed = protocol_details[ "invID_passed" ];
  correctRadii = protocol_details[ "correctRadii" ];
  expAborted   = protocol_details[ "expAborted" ];
  runID_passed = protocol_details[ "runID" ];
  
  qDebug() << "RUNID_PASSED !!! " << runID_passed;

  details_at_live_update = protocol_details;

  selectOptimaByName_auto( OptimaName );                         //New  
  
  timer_data_init = new QTimer;
  connect(timer_data_init, SIGNAL(timeout()), this, SLOT( load_xpn_raw_auto( ) ));
  timer_data_init->start(5000);     // 5 sec

  msg_data_avail = new QMessageBox;
  msg_data_avail->setIcon(QMessageBox::Information);
  
  //msg_data_avail->setStandardButtons(0);
  msg_data_avail->setWindowFlags ( Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  // QPushButton *okButton = msg_data_avail->addButton(tr("Ok"), QMessageBox::AcceptRole);
  // okButton->hide();

  QPushButton *Close    = msg_data_avail->addButton(tr("Return to Managing Optima Runs?"), QMessageBox::RejectRole);
  msg_data_avail->setText(tr( "Run named <b>%1</b> was submitted to: <br>"
   		              "<br><b>%2</b> <br>"
   			      "<br>Please start this method scan from the instrument panel. <br>"
   			      "<br>You may return to managing Optima runs now and reattach later by reopening and "
   			      "selecting <b>%1</b> among the list of Optima runs to follow. "
   			      "Alternatively, you can wait untill the method is started from "
   			      "the Optima panel and monitor the progress.")
   			  .arg(RunName).arg(OptimaName) );

  msg_data_avail->setWindowTitle(tr("Live Update"));
  
  if ( runID_passed.isEmpty() || runID_passed == "NULL" )
    {
      msg_data_avail->exec();
      
      if (msg_data_avail->clickedButton() == Close)
	{
	  timer_data_init->stop();
	  disconnect(timer_data_init, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
	  
	  reset_auto();

	  emit close_program(); 
	}
    }
}

// Reset LIVE UPDATE panel && stop timers && quit threads
void US_XpnDataViewer::reset_liveupdate_panel ( void )
{
  //Quit sys_thread, emit finished() to stop timer_check_sysdata from withing thread
  if ( !sys_thread->isFinished() )
    {
      sys_thread->quit();    
      qApp->processEvents();
    }

  //Disconnect timer_check_sysdata
  if ( timer_check_sysdata->isActive() )
    {
      timer_check_sysdata->stop();
      disconnect(timer_check_sysdata, SIGNAL(timeout()), 0, 0);
    }
  
  //Stop other timers if active
  if ( timer_all_data_avail->isActive() ) 
    {
      timer_all_data_avail->stop();
      disconnect(timer_all_data_avail, SIGNAL(timeout()), 0, 0);
    }
  
  if ( timer_data_reload->isActive() )
    {
      timer_data_reload->stop();
      disconnect(timer_data_reload, SIGNAL(timeout()), 0, 0);
    }

  if ( timer_data_init->isActive() )
    {
      timer_data_init->stop();
      disconnect(timer_data_init, SIGNAL(timeout()), 0, 0);
    }

  qApp->processEvents();
  
  /***** DEBUG ***********************/
  if ( sys_thread->isFinished() )
    qDebug() << "QThread STOPPED upon clickig Manage Optima runs !!! ";
  
  if ( !timer_check_sysdata->isActive() )
    qDebug() << "QTimer timer_check_sysdata STOPPED by clickig Manage Optima runs !!! ";

  if ( !timer_data_init->isActive() )
    qDebug() << "QTimer timer_data_init STOPPED by clickig Manage Optima runs !!! ";
  
  if ( !timer_all_data_avail->isActive() )
    qDebug() << "QTimer timer_all_data_avail STOPPED by clickig Manage Optima runs !!! ";

  if ( !timer_data_reload->isActive() )
    qDebug() << "QTimer timer_data_reload STOPPED by clickig Manage Optima runs !!! ";
  /*************************************/
  
  reset_auto();

  qDebug() << in_reload_auto << ", " << in_reload_all_data << ", " << in_reload_data_init << ", " << in_reload_check_sysdata;
  
  in_reload_auto    = false;
  in_reload_all_data  = false;
  in_reload_data_init = false;
  in_reload_check_sysdata  = false;
  
  qApp->processEvents();
}


//void US_XpnDataViewer::retrieve_xpn_raw_auto( QString & RunID )
void US_XpnDataViewer::retrieve_xpn_raw_auto( void )
{
   if ( in_reload_all_data )            // If already doing a reload,
     return;                            //  skip starting a new one
  
   in_reload_all_data   = true;          // Flag in the midst of a reload
  
   QString drDesc    = "";
   QString delim       = ( runInfo.count() > 0 ) ?
                         QString( runInfo[ 0 ] ).left( 1 ) :
                         "";
   /************* For Automated *****/
   // Search description list and choose the item with matching runID
   for ( int ii = 0; ii < runInfo.count(); ii++ )                   // In principle there should be just 1 record in runInfo
   {
      QString rDesc       = runInfo[ ii ];
      QString lRunID      = QString( rDesc ).mid( 1 ).section( delim, 0, 0 );

      qDebug() << "IN retrieve_xpn_raw_auto: rDesc " << rDesc << ", lRunID: "  << lRunID << ", RunID: " <<  RunID_to_retrieve;
      
      if ( lRunID == RunID_to_retrieve )                                       // ExpII is passed from US_Experiment
	{
	  qDebug() << "RunID found !!!";
	  drDesc = rDesc;
	  break;
	}
   }
   
//   US_XpnRunRaw* lddiag = new US_XpnRunRaw( drDesc, runInfo );
//    if ( lddiag->exec() == QDialog::Rejected )                    //ALEXEY need drDesc but do NOT need dialog
//    {
// DbgLv(1) << "RDr:  rtn fr XpnRunRaw dialog: CANCEL";
//       return;
//    }

//    // Restore area beneath dialog
//    qApp->processEvents();
// DbgLv(1) << "RDr:  rtn fr XpnRunRaw dialog";
// DbgLv(1) << "RDr:   drDesc" << drDesc;

   // See if we need to fix the runID


   QString fRunId    = QString( drDesc ).section( delim, 1, 1 );  

   qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!DRDESC: " << drDesc << ", fRunId:  " << fRunId; 

   
   QString fExpNm    = QString( drDesc ).section( delim, 5, 5 );
   QString new_runID = fExpNm + "-run" + fRunId;
   runType           = "RI";
   QRegExp rx( "[^A-Za-z0-9_-]" );

   int pos            = 0;
   bool runID_changed = false;

   if ( new_runID.length() > 60 )
   {
      int kchar         = 60 - 4 - fRunId.length();
      new_runID         = fExpNm.left( kchar ) + "-run" + fRunId;
      runID_changed     = true;
   }

   while ( ( pos = rx.indexIn( new_runID ) ) != -1 )
   {
      new_runID.replace( pos, 1, "_" );         // Replace 1 char at pos
      runID_changed     = true;
   }

   // Let the user know if the runID name has changed
   if ( runID_changed )
   {
      QMessageBox::warning( this,
            tr( "RunId Name Changed" ),
            tr( "The runId name has been changed.\nIt may consist only "
                "of alphanumeric characters,\nthe underscore, and the "
                "hyphen;\nand may be at most 60 characters in length."
                "\nNew runId:\n  " ) + new_runID );
   }

   // Set the runID and directory
   runID       = new_runID;
   le_runID->setText( runID );
   currentDir  = US_Settings::importDir() + "/" + runID;
   le_dir  ->setText( currentDir );
   qApp->processEvents();

   // Read the data
   //QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Reading Raw Optima data ..." ) );
   qApp->processEvents();
QDateTime sttime=QDateTime::currentDateTime();

   int iRunId         = fRunId.toInt();
   QString sMasks     = QString( drDesc ).section( delim, 7, 10 );
   int scanmask       = QString( sMasks ).mid( 0, 1 ) == "1" ? 1 : 0;
   scanmask          += QString( sMasks ).mid( 2, 1 ) == "1" ? 2 : 0;
   scanmask          += QString( sMasks ).mid( 4, 1 ) == "1" ? 4 : 0;
   scanmask          += QString( sMasks ).mid( 6, 1 ) == "1" ? 8 : 0;
DbgLv(1) << "RDr:     iRId" << iRunId << "sMsks scnmask" << sMasks << scanmask;

 qDebug() << "RDr:     iRId" << iRunId << "sMsks scnmask" << sMasks << scanmask;

 //ALEXEY: maybe put in_reload_check_sysdata = true; and then false (after xpn_data->import_data( iRunId, scanmask ); )
 //in_reload_check_sysdata = true; //ALEXEY
 xpn_data->import_data( iRunId, scanmask );                               // ALEXEY <-- actual data retreiving
   int ntsrows        = xpn_data->countOf( "scan_rows" );
DbgLv(1) << "RDr:     ntsrows" << ntsrows;
DbgLv(1) << "RDr:      knt(triple)   " << xpn_data->countOf( "triple"    );
 qApp->processEvents();

   if ( ntsrows < 1 )
   {
      le_status->setText( tr( "Run %1 has no associated data..." )
                          .arg( fRunId ) );

      in_reload_all_data   = false;  
      return;
   }

   le_status->setText( tr( "Initial Raw Optima data import complete." ) );
   qApp->processEvents();
   //in_reload_check_sysdata = false; //ALEXEY
   
double tm1=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
   QStringList opsys;

   // Infer and report on type of data to eventually export
   runType            = "RI";
   int optndx         = 0;

   if ( scanmask == 1  ||  scanmask == 2  ||
        scanmask == 4  ||  scanmask == 8 )
   {
      runType            = ( scanmask == 2 ) ? "FI" : runType;
      runType            = ( scanmask == 4 ) ? "IP" : runType;
      runType            = ( scanmask == 8 ) ? "WI" : runType;
      if ( scanmask == 1 )
         opsys << "Absorbance";
      else if ( scanmask == 2 )
         opsys << "Fluorescence";
      else if ( scanmask == 4 )
         opsys << "Interference";
      else if ( scanmask == 8 )
         opsys << "Wavelength";
   }

   else if ( ( scanmask & 1 ) != 0 )
   {
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
      QString runType2( "IP" );
      runType2           = ( ( scanmask & 2 ) != 0 ) ? "FI" : runType2;
      runType2           = ( ( scanmask & 8 ) != 0 ) ? "WI" : runType2;
      QString drtype1    = "Absorbance";
      QString drtype2    = "Interference";
      drtype1            = ( runType  == "FI" ) ? "Fluorescence" : drtype1;
      drtype1            = ( runType  == "IP" ) ? "Interference" : drtype1;
      drtype1            = ( runType  == "WI" ) ? "Wavelength"   : drtype1;
      drtype2            = ( runType2 == "RI" ) ? "Absorbance"   : drtype2;
      drtype2            = ( runType2 == "FI" ) ? "Fluorescence" : drtype2;
      drtype2            = ( runType2 == "WI" ) ? "Wavelength"   : drtype2;
      opsys << drtype1 << drtype2;

      QString msg        = tr( "Multiple scan data types are present:\n" )
                           +   "'" + drtype1 + "'\n or \n"
                           +   "'" + drtype2 + "' .\n\n"
                           + tr( "Choose one for initial display." );
      QMessageBox mbox;
      mbox.setWindowTitle( tr( "Scan Data Type to Process" ) );
      mbox.setText( msg );
      QPushButton* pb_opt1 = mbox.addButton( drtype1, QMessageBox::AcceptRole );
      QPushButton* pb_opt2 = mbox.addButton( drtype2, QMessageBox::RejectRole );
      mbox.setEscapeButton ( pb_opt2 );
      mbox.setDefaultButton( pb_opt1 );

      mbox.exec();
      if ( mbox.clickedButton() == pb_opt2 )
      {
         runType            = runType2;
         optndx             = 1;
      }
   }

   qApp->processEvents();  //ALEXEY: maybe this will help
   
   qDebug() << "1. Crashes HERE!!!!";
     
   cb_optsys->disconnect();
   cb_optsys->clear();

   qDebug() << "1a. Crashes HERE!!!!";
   
   cb_optsys->addItems( opsys );                                  // ALEXEY fill out Optics listbox

   qDebug() << "1ab. Crashes HERE!!!! - BEFORE Setting index to cb_optsys";
   cb_optsys->setCurrentIndex( optndx );
   qDebug() << "1ac. Crashes HERE!!!! - AFTER Setting index to cb_optsys";
   
   connect( cb_optsys,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeOptics( )            ) );

   qDebug() << "1b. Crashes HERE!!!!";
   
   runID         = new_runID;
DbgLv(1) << "RDr:  runID" << runID << "runType" << runType;
   xpn_data->set_run_values( runID, runType );                    // ALEXEY

   qDebug() << "2. Crashes HERE!!!! (after xpn_data->set_run_values( runID, runType ) )";

   // Build the AUC equivalent
   //QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Building AUC data ..." ) );
   qApp->processEvents();

   xpn_data->build_rawData( allData );                            // ALEXEY Builds Raw Data

   qDebug() << "3. Crashes HERE!!!! (after  xpn_data->build_rawData( allData ))";
   
double tm2=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
DbgLv(1) << "RDr:      build-raw done: tm1 tm2" << tm1 << tm2;

   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
   isRaw         = true;
   haveData      = true;
   ncellch       = xpn_data->cellchannels( cellchans );
   r_radii.clear();
   r_radii << allData[ 0 ].xvalues;
   nscan         = allData[ 0 ].scanCount();
   npoint        = allData[ 0 ].pointCount();

DbgLv(1) << "RDr: mwr ntriple" << ntriple;
DbgLv(1) << "RDr: ncellch" << ncellch << cellchans.count();
DbgLv(1) << "RDr: nscan" << nscan << "npoint" << npoint;
DbgLv(1) << "RDr:   rvS rvE" << r_radii[0] << r_radii[npoint-1];
   cb_cellchn->disconnect();                                      
   cb_cellchn->clear();
   cb_cellchn->addItems( cellchans );                             // ALEXEY fill out Cells/Channels listbox
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );

   nlambda      = xpn_data->lambdas_raw( lambdas );               // ALEXEY  lambdas
   int wvlo     = lambdas[ 0 ];
   int wvhi     = lambdas[ nlambda - 1 ];
#if 0
   ntriple      = nlambda * ncellch;  // Number triples
   ntpoint      = npoint  * nscan;    // Number radius points per triple
DbgLv(1) << "RDr: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple;
   triples.clear();

   for ( int jj = 0; jj < ncellch; jj++ )
   {
      QString celchn  = cellchans[ jj ];

      for ( int kk = 0; kk < nlambda; kk++ )
         triples << celchn + " / " + QString::number( lambdas[ kk] );
   }
#endif
#if 1
   ntriple      = xpn_data->data_triples( triples );              // ALEXEY triples
DbgLv(1) << "RDr: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple
   << triples.count();

 qDebug() << "RDr: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple
   << triples.count();
#endif

DbgLv(1) << "RDr: allData size" << allData.size();

 qDebug() << "RDr: allData size" << allData.size();

 //QApplication::restoreOverrideCursor();
   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   haveTmst       = QFile( tspath ).exists();
   in_reload_auto      = false;

   // Ok to enable some buttons now
   enableControls();                                    //ALEXEY ...and actual plotting data

   qDebug() << "CellChNumber, cellchans.count() " << CellChNumber.toInt() << ", " << cellchans.count();
   qDebug() << "TripleNumber, ntriple " << TripleNumber.toInt() << ", " << ntriple;

   //ALEXEY: Add Exp. Abortion Exception HERE... 
   if ( CheckExpComplete_auto( RunID_to_retrieve ) == 0 ) //ALEXEY should be == 3 as per documentation
     {
       qDebug() << "ABORTION IN EARLY STAGE...";
       
       experimentAborted  = true;
       
       timer_all_data_avail->stop();
       disconnect(timer_all_data_avail, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
       
       if ( !timer_check_sysdata->isActive()  ) // Check if sys_data Timer is stopped
	 {
	   export_auc_auto();
	   updateautoflow_record_atLiveUpdate();

	   reset_auto();
	   emit experiment_complete_auto( details_at_live_update  ); 
	   return;
	 }
     }
   
   // //ALEXEY: Add Exp. Abortion Exception HERE... 
   // if ( CheckExpComplete_auto( RunID_to_retrieve ) == 0 ) //ALEXEY should be == 3 as per documentation
   //   {
   //     experimentAborted  = true;
       
   //     timer_all_data_avail->stop();
   //     disconnect(timer_all_data_avail, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
       
   //     if ( !timer_check_sysdata->isActive()  ) // Check if sys_data Timer is stopped
   // 	 {
   // 	   // Ask if data retrived so far should be saved:
	   
   // 	   QMessageBox msgBox;
   // 	   msgBox.setText(tr("Experiment was aborted!"));
   // 	   msgBox.setInformativeText("The data retrieved so far can be saved or disregarded. If saved, the program will proceed to the next stage (Editing). Otherwise, it will return to the initial stage (Experiment), all data will be lost.");
   // 	   msgBox.setWindowTitle(tr("Experiment Abortion"));
   // 	   QPushButton *Save      = msgBox.addButton(tr("Save Data"), QMessageBox::YesRole);
   // 	   QPushButton *Ignore    = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
	   
   // 	   msgBox.setIcon(QMessageBox::Question);
   // 	   msgBox.exec();
	   
   // 	   if (msgBox.clickedButton() == Save)
   // 	     {
   // 	       export_auc_auto();
	       
   // 	       QString mtitle_complete  = tr( "Complete!" );
   // 	       QString message_done     = tr( "Experiment was completed. Optima data saved..." );
   // 	       QMessageBox::information( this, mtitle_complete, message_done );
	       
   // 	       updateautoflow_record_atLiveUpdate();
   // 	       emit experiment_complete_auto( currentDir, ProtocolName, invID_passed, correctRadii  );  // Updtade later: what should be passed with signal ??
   // 	       return;
   // 	     }
	   
   // 	   else if (msgBox.clickedButton() == Ignore)
   // 	     {
   // 	       reset();
   // 	       delete_autoflow_record();
   // 	       emit return_to_experiment( ProtocolName  ); 
   // 	       return;
   // 	     }
   // 	 }
   //   }      
   
   
   
   if ( cellchans.count() == CellChNumber.toInt() && ntriple == TripleNumber.toInt() )                // <--- Change to the values from the protocol
     {
       //stop timer
       timer_all_data_avail->stop();
       disconnect(timer_all_data_avail, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything
       
       // Auto-update hereafter
       timer_data_reload = new QTimer;
       connect(timer_data_reload, SIGNAL(timeout()), this, SLOT( reloadData_auto( ) ));
       timer_data_reload->start(10000);     // 5 sec

       in_reload_all_data   = false;  
     }

   in_reload_all_data   = false;  
}


// Load Optima raw (.postgres) data
void US_XpnDataViewer::load_xpn_raw( )
{

   // Ask for data directory
   QString dbhost    = xpnhost;
   int     dbport    = xpnport.toInt();
DbgLv(1) << "RDr: call connect_data  dbname h p u w"
 << xpnname << dbhost << dbport << xpnuser << xpnpasw;
   if ( xpn_data->connect_data( dbhost, dbport, xpnname, xpnuser, xpnpasw ) )
   {
if ( dbg_level > 0 ) xpn_data->dump_tables();
       xpn_data->scan_runs( runInfo );                         
#if 0
DbgLv(1) << "RDr:  pre-filter runs" << runInfo.count();
 xpn_data->filter_runs( runInfo );                             
DbgLv(1) << "RDr:  post-filter runs" << runInfo.count();
#endif
DbgLv(1) << "RDr:  rtn fr scan_runs,filter_runs";
   }
   else
   {
DbgLv(1) << "RDr:  connection failed";
      runInfo.clear();
   }
   
   QString drDesc    = "";
   US_XpnRunRaw* lddiag = new US_XpnRunRaw( drDesc, runInfo );
   if ( lddiag->exec() == QDialog::Rejected )                  
   {
DbgLv(1) << "RDr:  rtn fr XpnRunRaw dialog: CANCEL";
      return;
   }

   // Restore area beneath dialog
   qApp->processEvents();
DbgLv(1) << "RDr:  rtn fr XpnRunRaw dialog";
DbgLv(1) << "RDr:   drDesc" << drDesc;

   // See if we need to fix the runID
   QString delim     = QString( drDesc ).left( 1 );
   QString fRunId    = QString( drDesc ).section( delim, 1, 1 );
   QString fExpNm    = QString( drDesc ).section( delim, 5, 5 );
   QString new_runID = fExpNm + "-run" + fRunId;
   runType           = "RI";
   QRegExp rx( "[^A-Za-z0-9_-]" );

   int pos            = 0;
   bool runID_changed = false;

   if ( new_runID.length() > 60 )
   {
      int kchar         = 60 - 4 - fRunId.length();
      new_runID         = fExpNm.left( kchar ) + "-run" + fRunId;
      runID_changed     = true;
   }

   while ( ( pos = rx.indexIn( new_runID ) ) != -1 )
   {
      new_runID.replace( pos, 1, "_" );         // Replace 1 char at pos
      runID_changed     = true;
   }

   // Let the user know if the runID name has changed
   if ( runID_changed )
   {
      QMessageBox::warning( this,
            tr( "RunId Name Changed" ),
            tr( "The runId name has been changed.\nIt may consist only "
                "of alphanumeric characters,\nthe underscore, and the "
                "hyphen;\nand may be at most 60 characters in length."
                "\nNew runId:\n  " ) + new_runID );
   }

   // Set the runID and directory
   runID       = new_runID;
   le_runID->setText( runID );
   currentDir  = US_Settings::importDir() + "/" + runID;
   le_dir  ->setText( currentDir );
   qApp->processEvents();

   // Read the data
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Reading Raw Optima data ..." ) );
   qApp->processEvents();
QDateTime sttime=QDateTime::currentDateTime();

   int iRunId         = fRunId.toInt();
   QString sMasks     = QString( drDesc ).section( delim, 7, 10 );
   int scanmask       = QString( sMasks ).mid( 0, 1 ) == "1" ? 1 : 0;
   scanmask          += QString( sMasks ).mid( 2, 1 ) == "1" ? 2 : 0;
   scanmask          += QString( sMasks ).mid( 4, 1 ) == "1" ? 4 : 0;
   scanmask          += QString( sMasks ).mid( 6, 1 ) == "1" ? 8 : 0;
DbgLv(1) << "RDr:     iRId" << iRunId << "sMsks scnmask" << sMasks << scanmask;

 xpn_data->import_data( iRunId, scanmask );                             
   int ntsrows        = xpn_data->countOf( "scan_rows" );
DbgLv(1) << "RDr:     ntsrows" << ntsrows;
DbgLv(1) << "RDr:      knt(triple)   " << xpn_data->countOf( "triple"    );


   if ( ntsrows < 1 )
   {
      le_status->setText( tr( "Run %1 has no associated data..." )
                          .arg( fRunId ) );
      return;
   }

   le_status->setText( tr( "Initial Raw Optima data import complete." ) );
   qApp->processEvents();
double tm1=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
   QStringList opsys;

   // Infer and report on type of data to eventually export
   runType            = "RI";
   int optndx         = 0;

   if ( scanmask == 1  ||  scanmask == 2  ||
        scanmask == 4  ||  scanmask == 8 )
   {
      runType            = ( scanmask == 2 ) ? "FI" : runType;
      runType            = ( scanmask == 4 ) ? "IP" : runType;
      runType            = ( scanmask == 8 ) ? "WI" : runType;
      if ( scanmask == 1 )
         opsys << "Absorbance";
      else if ( scanmask == 2 )
         opsys << "Fluorescence";
      else if ( scanmask == 4 )
         opsys << "Interference";
      else if ( scanmask == 8 )
         opsys << "Wavelength";
   }

   else if ( ( scanmask & 1 ) != 0 )
   {
      QApplication::restoreOverrideCursor();
      QApplication::restoreOverrideCursor();
      QString runType2( "IP" );
      runType2           = ( ( scanmask & 2 ) != 0 ) ? "FI" : runType2;
      runType2           = ( ( scanmask & 8 ) != 0 ) ? "WI" : runType2;
      QString drtype1    = "Absorbance";
      QString drtype2    = "Interference";
      drtype1            = ( runType  == "FI" ) ? "Fluorescence" : drtype1;
      drtype1            = ( runType  == "IP" ) ? "Interference" : drtype1;
      drtype1            = ( runType  == "WI" ) ? "Wavelength"   : drtype1;
      drtype2            = ( runType2 == "RI" ) ? "Absorbance"   : drtype2;
      drtype2            = ( runType2 == "FI" ) ? "Fluorescence" : drtype2;
      drtype2            = ( runType2 == "WI" ) ? "Wavelength"   : drtype2;
      opsys << drtype1 << drtype2;

      QString msg        = tr( "Multiple scan data types are present:\n" )
                           +   "'" + drtype1 + "'\n or \n"
                           +   "'" + drtype2 + "' .\n\n"
                           + tr( "Choose one for initial display." );
      QMessageBox mbox;
      mbox.setWindowTitle( tr( "Scan Data Type to Process" ) );
      mbox.setText( msg );
      QPushButton* pb_opt1 = mbox.addButton( drtype1, QMessageBox::AcceptRole );
      QPushButton* pb_opt2 = mbox.addButton( drtype2, QMessageBox::RejectRole );
      mbox.setEscapeButton ( pb_opt2 );
      mbox.setDefaultButton( pb_opt1 );

      mbox.exec();
      if ( mbox.clickedButton() == pb_opt2 )
      {
         runType            = runType2;
         optndx             = 1;
      }
   }

   cb_optsys->disconnect();
   cb_optsys->clear();
   cb_optsys->addItems( opsys );                               
   cb_optsys->setCurrentIndex( optndx );
   connect( cb_optsys,    SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeOptics( )            ) );

   runID         = new_runID;
DbgLv(1) << "RDr:  runID" << runID << "runType" << runType;
   xpn_data->set_run_values( runID, runType );                  

   // Build the AUC equivalent
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Building AUC data ..." ) );
   qApp->processEvents();

   xpn_data->build_rawData( allData );                           
double tm2=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
DbgLv(1) << "RDr:      build-raw done: tm1 tm2" << tm1 << tm2;

   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
   isRaw         = true;
   haveData      = true;
   ncellch       = xpn_data->cellchannels( cellchans );
   r_radii.clear();
   r_radii << allData[ 0 ].xvalues;
   nscan         = allData[ 0 ].scanCount();
   npoint        = allData[ 0 ].pointCount();

DbgLv(1) << "RDr: mwr ntriple" << ntriple;
DbgLv(1) << "RDr: ncellch" << ncellch << cellchans.count();
DbgLv(1) << "RDr: nscan" << nscan << "npoint" << npoint;
DbgLv(1) << "RDr:   rvS rvE" << r_radii[0] << r_radii[npoint-1];
   cb_cellchn->disconnect();                                      
   cb_cellchn->clear();
   cb_cellchn->addItems( cellchans );                           
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );

   nlambda      = xpn_data->lambdas_raw( lambdas );             
   int wvlo     = lambdas[ 0 ];
   int wvhi     = lambdas[ nlambda - 1 ];
#if 0
   ntriple      = nlambda * ncellch;  // Number triples
   ntpoint      = npoint  * nscan;    // Number radius points per triple
DbgLv(1) << "RDr: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple;
   triples.clear();

   for ( int jj = 0; jj < ncellch; jj++ )
   {
      QString celchn  = cellchans[ jj ];

      for ( int kk = 0; kk < nlambda; kk++ )
         triples << celchn + " / " + QString::number( lambdas[ kk] );
   }
#endif
#if 1
   ntriple      = xpn_data->data_triples( triples );           
DbgLv(1) << "RDr: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple
   << triples.count();
#endif

DbgLv(1) << "RDr: allData size" << allData.size();
   QApplication::restoreOverrideCursor();
   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   haveTmst       = QFile( tspath ).exists();
   in_reload      = false;

   // Ok to enable some buttons now
   enableControls();                                  
}


// Load US3 AUC Optima-derived data
void US_XpnDataViewer::load_auc_xpn( )
{
   int status        = 0;
   QStringList ifpaths;

   resetAll();

   QString dir       = "";
   US_XpnRunAuc lddiag( dir );
   if ( lddiag.exec() == QDialog::Rejected )
      return;
   
   // Restore area beneath dialog
   qApp->processEvents();

   if ( dir.isEmpty() ) return; 
   
   dir.replace( "\\", "/" );                 // WIN32 issue
   if ( dir.right( 1 ) != "/" ) dir += "/";  // Ensure trailing '/'
   
   // Check the runID
   QString new_runID = dir.section( "/", -2, -2 ).simplified();
DbgLv(1) << "RDa: runID" << new_runID;
DbgLv(1) << "RDa: dir" << dir;
      
   QRegExp rx( "^[A-Za-z0-9_-]{1,80}$" );
   if ( rx.indexIn( new_runID ) < 0 )
   {
      QMessageBox::warning( this,
            tr( "Bad runID Name" ),
            tr( "The runID name may consist only of alphanumeric\n"  
                "characters, the underscore, and the hyphen." ) );
      return;
   }

   // Set the runID and directory
   runID             = new_runID;
   currentDir        = dir;
   le_runID ->setText( runID );
   le_dir   ->setText( currentDir );
DbgLv(1) << "RDa: runID" << runID;
DbgLv(1) << "RDa: dir" << currentDir;

   // Error reporting 
   if ( status == US_DB2::NO_PROJECT ) 
   { 
      QMessageBox::information( this, 
            tr( "Attention" ), 
            tr( "The project was not found.\n" 
                "Please select an existing project and try again.\n" ) ); 
   } 
   
   else if ( status != US_DB2::OK ) 
   { 
      QMessageBox::information( this, 
            tr( "Disk Read Problem" ), 
            tr( "Could not read data from the disk.\n" 
                "Disk status: " ) + QString::number( status ) ); 
   }

   // Load the AUC data
   le_status->setText( tr( "Reading AUC Xpn data ..." ) );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   QDir dirdir( dir, "*", QDir::Name, QDir::Files | QDir::Readable );
   dirdir.makeAbsolute();
   if ( dir.right( 1 ) != "/" ) dir += "/"; // Ensure trailing /
   xpn_fnames       = dirdir.entryList( QStringList( "*.auc" ),
                                        QDir::Files, QDir::Name );
   xpn_fnames.sort();
   ntriple           = xpn_fnames.size();
   cellchans.clear();
   lambdas  .clear();
   triples  .clear();
   int missm         = 0;
DbgLv(1) << "RDa: mwr ntriple" << ntriple;

   for ( int ii = 0; ii < ntriple; ii++ )
   {
      QString mwrfname  = xpn_fnames.at( ii );
      QString mwrfpath  = currentDir + mwrfname;
      ifpaths << mwrfpath;
      US_DataIO::RawData  rdata;

      US_DataIO::readRawData( mwrfpath, rdata );

      allData << rdata;

      QString celchn    = QString::number( rdata.cell ) + " / " +
                          QString( rdata.channel );
      QString ccnode    = mwrfname.section( ".", -4, -3 );
      QString celchnf   = ccnode.replace( ".", " / " );

      if ( celchn != celchnf )
      {
DbgLv(1) << "RDa:   **F/D MISMATCH** " << celchn << celchnf << ii;
         celchn            = celchnf;
         missm++;
      }

      int lambda        = qRound( rdata.scanData[ 0 ].wavelength );
      QString triple    = celchn + " / " + QString::number( lambda );

      if ( ! cellchans.contains( celchn ) )
         cellchans << celchn;

      if ( ! lambdas.contains( lambda ) )
         lambdas   << lambda;

      if ( ! triples.contains( triple ) )
         triples   << triple;

      le_status->setText( tr( "Data in for triple %1 of %2" )
                          .arg( ii + 1 ).arg( ntriple ) );
      qApp->processEvents();
   }

   if ( missm > 0 )
   {
      QMessageBox::warning( this,
            tr( "Data/File Cell/Channel Mismatches" ),
            tr( "%1 mismatch(es) occurred in the Cell/Channel setting\n"
               " of AUC data versus File name nodes.\n\n"
               "The Cell/Channel settings  have been corrected,\n"
               " but input data should be reviewed!!" )
               .arg( missm ) );
   }

   r_radii.clear();
   r_radii << allData[ 0 ].xvalues;

   isRaw        = false;
   haveData     = true;
   ncellch      = cellchans .size();
   nlambda      = lambdas   .size();
   nscan        = allData[ 0 ].scanCount();
   npoint       = allData[ 0 ].pointCount();
   ntpoint      = nscan * npoint;
DbgLv(1) << "RDa: mwr ncellch nlambda nscan npoint"
 << ncellch << nlambda << nscan << npoint;
DbgLv(1) << "RDa:   rvS rvE" << r_radii[0] << r_radii[npoint-1];
   le_status->setText( tr( "All %1 raw AUCs have been loaded." )
                       .arg( ntriple ) );
   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();
   qApp->processEvents();

   ct_from->setMaximum( nscan );
   ct_to  ->setMaximum( nscan );

   xpn_data->load_auc( allData, ifpaths );

   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   haveTmst       = QFile( tspath ).exists();
DbgLv(1) << "RDa: load_auc complete";
   // Ok to enable some buttons now
   enableControls();
DbgLv(1) << "RDa: enableControls complete";
}

// Display detailed information about the data
void US_XpnDataViewer::runDetails( void )
{
   // Use the data object to compose details text
   QString msg = xpn_data->runDetails();

   // Open the dialog and display the report text
   US_Editor* editd = new US_Editor( US_Editor::DEFAULT, true );

   editd->setWindowTitle( tr( "Optima Raw Data Statistics" ) );
   editd->move( pos() + QPoint( 200, 200 ) );
   editd->resize( 600, 500 );
   editd->e->setFont( QFont( US_Widgets::fixedFont().family(),
                             US_GuiSettings::fontSize() ) );
   editd->e->setText( msg );
   editd->show();
}

// Plot the current data record
void US_XpnDataViewer::plot_current( void )
{
   if ( allData.size() == 0 )
      return;

   plot_titles();     // Set the titles

   plot_all();        // Plot the data
}

// Compose plot titles for the current record
void US_XpnDataViewer::plot_titles( void )
{
DbgLv(1) << "pTit: prectype" << prectype;
   QString prec     = cb_pltrec->currentText();
   QString cell     = prec.section( "/", 0, 0 ).simplified();
   QString chan     = prec.section( "/", 1, 1 ).simplified();
   QString wvln     = prec.section( "/", 2, 2 ).simplified();

   if ( isMWL )
   {
      QString cellch   = cb_cellchn ->currentText();
      cell             = cellch.section( "/", 0, 0 ).simplified();
      chan             = cellch.section( "/", 1, 1 ).simplified();
      wvln             = prec;
   }
DbgLv(1) << "pTit: prec" << prec << "isMWL" << isMWL << "wvln" << wvln;

   // Plot Title and legends
   QString title    = "Radial Intensity Data\nRun ID: " + runID +
                      "\n    Cell: " + cell + "  Channel: " + chan +
                      "  Wavelength: " + wvln;
   QString xLegend  = QString( "Radius (in cm)" );
   QString yLegend  = "Radial Intensity at " + wvln + " nm";

   if ( runType == "IP" )
   {
      title            = "Interference Data\nRun ID: " + runID +
                         "\n    Cell: " + cell + "  Channel: " + chan +
                         "  Wavelength: " + wvln;
      yLegend          = "Interference at " + wvln + " nm";
   }
   else if ( runType == "FI" )
   {
      title            = "Fluorescence Intensity Data\nRun ID: " + runID +
                         "\n    Cell: " + cell + "  Channel: " + chan +
                         "  Wavelength: " + wvln;
      yLegend          = "Fluorescence at " + wvln + " nm";
   }
   else if ( runType == "WI" )
   {
      title            = "Wavelength Intensity Data\nRun ID: " + runID +
                         "\n    Cell: " + cell + "  Channel: " + chan +
                         "  Radius: " + wvln;
      xLegend          = QString( "Wavelength (in nm)" );
      yLegend          = "Radial Intensity at " + wvln;
   }

   data_plot->setTitle( title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   yLegend );
   data_plot->setAxisTitle( QwtPlot::xBottom, xLegend );
}

// Draw scan curves for the current plot record
void US_XpnDataViewer::plot_all( void )
{
   dPlotClearAll( data_plot );
   grid           = us_grid( data_plot );

   // Make sure ranges are set up, then build an averaged data vector
   compute_ranges();

//DbgLv(1) << "PltA: kpoint" << kpoint << "datsize" << curr_adata.size();
DbgLv(1) << "PltA: kpoint" << kpoint << "trpxs" << trpxs
 << "nscan" << nscan << allData[trpxs].scanCount();
   // Build the X,Y vectors
   QVector< double > rvec( kpoint );
   QVector< double > vvec( kpoint );
   double* rr     = rvec.data();
   double* vv     = vvec.data();

   int     scan_from = (int)ct_from->value();
   int     scan_to   = (int)ct_to  ->value();
   int     scan_nbr  = 0;
   QPen    pen_red ( Qt::red );
   QPen    pen_plot( US_GuiSettings::plotCurve() );
   int     rdx       = radxs;
   int     colx      = -1;
   US_DataIO::RawData* rdata = &allData[ trpxs ];
   int     scan_knt  = rdata->scanCount();

   for ( int ptx = 0; ptx < kpoint; ptx++, rdx++ )
   {  // One-time build of X vector
      rr[ ptx ]       = rdata->xvalues[ rdx ];
   }
DbgLv(1) << "PltA:   rr[n]" << rr[kpoint-1];

//   for ( int scnx = 0; scnx < nscan; scnx++ )
   for ( int scnx = 0; scnx < scan_knt; scnx++ )
   {  // Build Y vector for each scan
      if ( excludes.contains( scnx ) )  continue;

      for ( int ptx = 0, rdx = radxs; ptx < kpoint; ptx++, rdx++ )
      {
         vv[ ptx ]       = rdata->value( scnx, rdx );
      }

      if ( mcknt > 0 )
      {
         colx                = scan_nbr % mcknt;
         pen_plot            = QPen( mcolors[ colx ] );
      }

      scan_nbr++;
if(scnx<3 || (scnx+4)>nscan)
DbgLv(1) << "PltA:   vv[n]" << vv[kpoint-1] << "scnx" << scnx
 << "scan_nbr" << scan_nbr << "colx" << colx;
      QString       title = tr( "Raw Data at scan " )
                            + QString::number( scan_nbr );
      QwtPlotCurve* curv  = us_curve( data_plot, title );

      if ( scan_nbr > scan_to  ||  scan_nbr < scan_from )
         curv->setPen( pen_plot );            // Normal pen
      else
         curv->setPen( pen_red  );            // Scan-focus pen

      curv->setSamples( rr, vv, kpoint );     // Build a scan curve
//DbgLv(1) << "PltA:   scx" << scx << "rr0 vv0 rrn vvn"
// << rr[0] << rr[kpoint-1] << vv[0] << vv[kpoint-1];
   }

DbgLv(1) << "PltA: last_xmin" << last_xmin;
   if ( last_xmin < 0.0 )
   {  // If first time, use auto scale to set plot ranges
      data_plot->setAxisAutoScale( QwtPlot::yLeft   );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }

   else
   {  // After first time, use the same plot ranges as set before
      data_plot->setAxisScale( QwtPlot::xBottom, last_xmin, last_xmax );
      if ( ck_autoscy->isChecked() )
         data_plot->setAxisAutoScale( QwtPlot::yLeft   );
      else
         data_plot->setAxisScale( QwtPlot::yLeft  , last_ymin, last_ymax );
   }

   // Draw the plot
//   connect( plot, SIGNAL( zoomedCorners( QRectF ) ),
//            this, SLOT  ( currentRectf ( QRectF ) ) );
   data_plot->replot();

   // Pick up the actual bounds plotted (including any Config changes)
   QwtScaleDiv* sdx = AXISSCALEDIV( QwtPlot::xBottom );
   QwtScaleDiv* sdy = AXISSCALEDIV( QwtPlot::yLeft   );
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();
DbgLv(1) << "PltA: xlo xhi" << last_xmin << last_xmax
 << "ylo yhi" << last_ymin << last_ymax;
}

// Slot to handle a change in start or end radius
void US_XpnDataViewer::changeRadius()
{
DbgLv(1) << "chgRadius";
   have_rngs  = false;
   compute_ranges();                        // Recompute ranges
   last_xmin      = -1;
   last_xmax      = -1;

   plot_current();                      // Re-plot with adjusted range
}

void US_XpnDataViewer::changeCellCh( void )
{
   // Match the description to find the correct triple in memory
   QString cellch = cb_cellchn->currentText();
DbgLv(1) << "chgCC:  cellch" << cellch << "last_xmin" << last_xmin;
   if ( allData.size() < 1 )
      return;

   if ( last_xmin < 0.0 )
   {  // If first time plotting any data, detect actual data bounds
      have_rngs     = false;
      compute_ranges();

      int trxs      = trpxs + lmbxs;
      int trxe      = trpxs + lmbxe;

      last_xmin     = allData[ trxs ].xvalues[ radxs ];
      last_xmax     = last_xmin;
      US_DataIO::RawData* edata = &allData[ trxs ];

      for ( int rdx = radxs; rdx < radxe; rdx++ )
      {
         last_xmin  = qMin( last_xmin, edata->xvalues[ rdx ] );
         last_xmax  = qMax( last_xmax, edata->xvalues[ rdx ] );
      }

      last_ymin     = allData[ trxs ].reading( 0, 0 );
      last_ymax     = last_ymin;

      for ( int trx = trxs; trx < trxe; trx++ )
      {  // Accumulate Y bounds, the amplitude bounds
         US_DataIO::RawData* edata = &allData[ trx ];

         for ( int scx = 0; scx < nscan; scx++ )
         {
            for ( int rdx = radxs; rdx < radxe; rdx++ )
            {
               last_ymin  = qMin( last_ymin, edata->reading( scx, rdx ) );
               last_ymax  = qMax( last_ymax, edata->reading( scx, rdx ) );
            }
         }
      }
DbgLv(1) << "chgCC: trxs trxe" << trxs << trxe;
   }

   else
   {  // After first time, detect what has been already set
      QwtScaleDiv* sdx = AXISSCALEDIV( QwtPlot::xBottom );
      QwtScaleDiv* sdy = AXISSCALEDIV( QwtPlot::yLeft   );
      last_xmin      = sdx->lowerBound();
      last_xmax      = sdx->upperBound();
      last_ymin      = sdy->lowerBound();
      last_ymax      = sdy->upperBound();
   }
DbgLv(1) << "chgCC:  xmin xmax ymin ymax"
 << last_xmin << last_xmax << last_ymin << last_ymax;

   have_rngs      = false;
   int lplrec     = ntriple - 1;

   // Make sure cell/channel and plot record are in sync
   if ( ! isMWL )
   {  // If plot record ranges are triples, point to first of cell/channel
      QString ccval  = cb_cellchn->currentText().replace( " / ", "/" );
      QString ccplr  = cb_pltrec->currentText().section( "/", 0, 1 );
DbgLv(1) << "chgCC: ccval" << ccval << "ccplr" << ccplr;

      if ( ccval != ccplr )
      {
         for ( int jj = 0; jj < ntriple; jj++ )
         {
            ccplr          = triples[ jj ].section( "/", 0, 1 ).simplified()
                                          .replace( " / ", "/" );
DbgLv(1) << "chgCC:   jj" << jj << "ccplr" << ccplr;
            if ( ccplr == ccval )
            {  // Set plot record to first triple from cell/channel
               connect_ranges( false );
DbgLv(1) << "chgCC:     set pltrec index" << jj;
               cb_pltrec->setCurrentIndex( jj );
               connect_ranges( true );
               break;
            }
         }
      }
   }
   else
   {  // If plot record ranges are wavelengths, point to first wavelength
DbgLv(1) << "chgCC: isMWL" << isMWL;
      lplrec         = nlambda - 1;
//      connect_ranges( false );
//      cb_pltrec->setCurrentIndex( 0 );
//      connect_ranges( true );
   }

   compute_ranges();
   pb_prev  ->setEnabled( ( recx > 0 )      );
   pb_next  ->setEnabled( ( recx < lplrec ) );

//   changeRecord();
   plot_current();
}

// Slot to handle a change in the plot record
void US_XpnDataViewer::changeRecord( void )
{
   recx           = cb_pltrec->currentIndex();
   int lplrec     = ntriple - 1;

   if ( isMWL )
   {  // For MWL, limit record range to lambda range
      lplrec         = nlambda - 1;
   }

   else
   {  // For non-MWL, change cell/channel if appropriate
      QString new_cc = QString( cb_pltrec->currentText() )
                       .section( "/", 0, 1 ).replace( "/", " / " );
      QString celchn = cb_cellchn->currentText();

      if ( new_cc != celchn )
      {
         int iccx       = cellchans.indexOf( new_cc );
         connect_ranges( false );
         cb_cellchn->setCurrentIndex( iccx );
         QString new_wl = QString( cb_pltrec->currentText() )
                          .section( "/", 2, 2 );
         le_lrange ->setText( new_wl + tr( " only" ) );
         connect_ranges( true );
      }
   }

DbgLv(1) << "chgRec: recx" << recx;

   // Plot what we have
   plot_current();

   // Update status text (if not part of movie save) and set prev/next arrows
   le_status->setText( lb_pltrec->text() + "  " + cb_pltrec->currentText() );
   pb_prev  ->setEnabled( ( recx > 0 )      );
   pb_next  ->setEnabled( ( recx < lplrec ) );
}

// Slot to handle a change in the optical system
void US_XpnDataViewer::changeOptics( void )
{
   // Determine the new run type
   int optrx      = cb_optsys->currentIndex();
   QString ostyp  = cb_optsys ->currentText();
   QString rtype( "RI" );
   if ( ostyp == "Interference" )
      rtype          = "IP";
   else if ( ostyp == "Fluorescence" )
      rtype          = "FI";
   else if ( ostyp == "Wavelength" )
      rtype          = "WI";
DbgLv(1) << "chgOpt: optrx" << optrx << "ostyp" << ostyp
 << "rtype" << rtype << "nopts" << cb_optsys->children().count();

  // If simply re-choosing the same optics, bale out now
   if ( rtype == runType )
      return;

QDateTime sttime=QDateTime::currentDateTime();
   runType       = rtype;    // Set the new run type (RI, IP, ...)

   // Turn off auto-reload if on
   if ( rlt_id != 0 )
   {
      ck_autorld->setChecked( false );
      QMessageBox::warning( this,
            tr( "Auto-Reload Stopped" ),
            tr( "Auto-Reload has been stopped and its box unchecked,"
                "\n since the optical system has been changed." ) );
   }

   // Set up for a new run type
   xpn_data->set_run_values( runID, runType );

   // For new optics, rebuild internal arrays and all AUC
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   le_status->setText( tr( "Rebuilding AUC data ..." ) );
   qApp->processEvents();

   xpn_data->build_rawData( allData );
double tm2=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
DbgLv(1) << "chgOpt:   build-raw done: tm2" << tm2;

   QApplication::restoreOverrideCursor();
   QApplication::restoreOverrideCursor();

   // Reset various flags, counts, and lists
   isRaw         = true;
   haveData      = true;
   ncellch       = xpn_data->cellchannels( cellchans );
   r_radii.clear();
   r_radii << allData[ 0 ].xvalues;
   nscan         = allData[ 0 ].scanCount();
   npoint        = allData[ 0 ].pointCount();

DbgLv(1) << "chgOpt: mwr ntriple" << ntriple;
DbgLv(1) << "chgOpt: ncellch" << ncellch << cellchans.count();
DbgLv(1) << "chgOpt: nscan" << nscan << "npoint" << npoint;
DbgLv(1) << "chgOpt:   rvS rvE" << r_radii[0] << r_radii[npoint-1];
   cb_cellchn->disconnect();
   cb_cellchn->clear();
   cb_cellchn->addItems( cellchans );
   connect( cb_cellchn,   SIGNAL( currentIndexChanged( int ) ),
            this,         SLOT  ( changeCellCh(            ) ) );

   nlambda      = xpn_data->lambdas_raw( lambdas );
   int wvlo     = lambdas[ 0 ];
   int wvhi     = lambdas[ nlambda - 1 ];
   ntriple      = xpn_data->data_triples( triples );
DbgLv(1) << "chgOpt: nwl wvlo wvhi" << nlambda << wvlo << wvhi
   << "ncellch" << ncellch << "nlambda" << nlambda << "ntriple" << ntriple
   << triples.count();

DbgLv(1) << "chgOpt: allData size" << allData.size();
   QApplication::restoreOverrideCursor();
   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   haveTmst       = QFile( tspath ).exists();

   // Ok to reenable some buttons now
   enableControls();
}

// Slot to handle a click to go to the previous record
void US_XpnDataViewer::prevPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() - 1;

   if ( pltrx < 1 )
   {
      pltrx          = 0;
      pb_prev->setEnabled( false );
   }

   QwtScaleDiv* sdx = AXISSCALEDIV( QwtPlot::xBottom );
   QwtScaleDiv* sdy = AXISSCALEDIV( QwtPlot::yLeft   );
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Slot to handle a click to go to the next record
void US_XpnDataViewer::nextPlot( void )
{
   int pltrx      = cb_pltrec->currentIndex() + 1;
   int nitems     = cb_pltrec->count();

   if ( ( pltrx + 2 ) > nitems )
   {
      pltrx          = nitems - 1;
      pb_next->setEnabled( false );
   }

   QwtScaleDiv* sdx = AXISSCALEDIV( QwtPlot::xBottom );
   QwtScaleDiv* sdy = AXISSCALEDIV( QwtPlot::yLeft   );
   last_xmin      = sdx->lowerBound();
   last_xmax      = sdx->upperBound();
   last_ymin      = sdy->lowerBound();
   last_ymax      = sdy->upperBound();

   cb_pltrec->setCurrentIndex( pltrx );
}

// Compute the plot range indexes implied by current settings
void US_XpnDataViewer::compute_ranges()
{
   if ( have_rngs )   // If we just did this computation, return now
      return;

   ccx        = cb_cellchn->currentIndex();         // Cell/Channel index
   rad_start  = cb_rstart ->currentText().toDouble();  // Radius start
   rad_end    = cb_rend   ->currentText().toDouble();  // Radius end
   lmb_start  = lambdas[ 0 ];                       // Lambda start
   lmb_end    = lambdas[ nlambda - 1 ];             // Lambda end
   recx       = cb_pltrec ->currentIndex();         // Plot record index
   lmbxs      = lambdas.indexOf( lmb_start );       // Lambda start index
   lmbxe      = lambdas.indexOf( lmb_end   ) + 1;   // Lambda end index
   radxs      = dvec_index( r_radii, rad_start );     // Radius start index
   radxe      = dvec_index( r_radii, rad_end   ) + 1; // Radius end index
DbgLv(1) << "cmpR:  rS rE rxS rxE" << rad_start << rad_end << radxs << radxe;
DbgLv(1) << "cmpR:   rvS rvE" << r_radii[radxs] << r_radii[radxe-1];
   klambda    = lmbxe - lmbxs;                      // Count of plot lambdas
   kradii     = radxe - radxs;                      // Count of plot radii
   kscan      = nscan - excludes.size();            // Count included scans
   trpxs      = ccx * nlambda;                      // Start triple index
   kpoint     = kradii;                             // Count of plot points
   ktpoint    = kscan * kpoint;                     // Total plot data points

   if ( isMWL )
   {
      trpxs     += recx;                            // Triple-to-plot index
   }
   else
   {
      trpxs      = recx;                            // Triple-to-plot index
   }
//   have_rngs  = true;                               // Mark ranges computed
DbgLv(1) << "cmpR:  isMWL" << isMWL << "kpoint" << kpoint << "trpxs" << trpxs;
}

// Connect or Disconnect plot-range related controls
void US_XpnDataViewer::connect_ranges( bool conn )
{
   if ( conn )
   {  // Connect the range-related controls
      connect( cb_cellchn, SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeCellCh(            ) ) );
      connect( cb_rstart,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRadius(            ) ) );
      connect( cb_rend,    SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRadius(            ) ) );
      connect( cb_pltrec,  SIGNAL( currentIndexChanged( int ) ),
               this,       SLOT  ( changeRecord(            ) ) );
   }

   else
   {  // Disconnect the range-related controls
      cb_cellchn->disconnect();
      cb_rstart ->disconnect();
      cb_rend   ->disconnect();
      cb_pltrec ->disconnect();
   }
}

// export_auc data in us_com_project
void US_XpnDataViewer::export_auc_auto()
{
   correct_radii();      // Perform chromatic aberration radius corrections

   int nfiles     = xpn_data->export_auc( allData );
//   int noptsy     = cb_optsys->children().count();
   int noptsy     = cb_optsys->count();
DbgLv(1) << "ExpAucA: noptsy koptsy" << noptsy << cb_optsys->children().count();

   if ( noptsy > 1 )
   {  // Export data from Optical Systems other than currently selected one
      int currsx     = cb_optsys->currentIndex();

      for ( int osx = 0; osx < noptsy; osx++ )
      {
         if ( osx == currsx )  continue;   // Skip already-handled opt sys

         cb_optsys->setCurrentIndex( osx );
         correct_radii();                  // Chromatic aberration correction if needed
         int kfiles     = xpn_data->export_auc( allData ) - 2;  // Export data
         nfiles        += kfiles;          // Total files written
      }

      // Restore Optical System selection to what it was before
      cb_optsys->setCurrentIndex( currsx );
   }

   le_status  ->setText( tr( "%1 AUC/TMST files written ..." ).arg( nfiles ) );
   qApp->processEvents();
}

// Slot to export to openAUC
void US_XpnDataViewer::export_auc()
{
   if ( !isRaw )
   {
      int status = QMessageBox::information( this, tr( "Data Overwrite Warning..." ),
               tr( "This operation will overwrite all data currently in"
                   "the same directory where these data were loaded from. Proceed? " ),
               tr( "&OK" ), tr( "&Cancel" ),
               0, 0, 1 );
      if ( status != 0 ) return;
   }
   QString runIDt = le_runID->text();              // User given run ID text
DbgLv(1) << "ExpAuc: runIDt" << runIDt;
DbgLv(1) << "ExpAuc: runID" << runID;

   if ( runIDt != runID )
   {  // Set runID to new entry given by user
      QRegExp rx( "[^A-Za-z0-9_-]" );              // Possibly modify entry
      QString new_runID  = runIDt;
      int pos            = 0;

      while ( ( pos = rx.indexIn( new_runID ) ) != -1 )
      {  // Loop thru any invalid characters given (not alphanum.,'_','-')
         new_runID.replace( pos, 1, "_" );         // Replace 1 char at pos
      }

DbgLv(1) << "ExpAuc: new_runID len" << new_runID.length();
      if ( new_runID.length() > 60 )
      {
         new_runID          = QString( new_runID ).left( 52 )
                            + QString( new_runID ).right( 8 );
DbgLv(1) << "ExpAuc: corr new_runID len" << new_runID.length();
DbgLv(1) << "ExpAuc: new_runID" << new_runID;
      }

      // Let the user know that the runID name has changed
      QMessageBox::warning( this,
         tr( "RunId Name Changed" ),
         tr( "The runId name has been changed."
             "\nNew runId:\n  " ) + new_runID );

      runID          = new_runID;                  // Show new run ID, dir.
      le_runID->setText( runID );
      currentDir     = US_Settings::importDir() + "/" + runID;
      le_dir  ->setText( currentDir );
      qApp->processEvents();

      xpn_data->set_run_values( runID, runType );  // Set run ID for export
   }

   // Export the AUC data to a local directory and build TMST
DbgLv(1) << "ExpAuc: BEFORE correct_radii() !!!!";
   correct_radii();      // Perform chromatic aberration radius corrections
DbgLv(1) << "ExpAuc: AFTER correct_radii() !!!!";
   int nfiles     = xpn_data->export_auc( allData );  // Export AUC/TMST

   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   haveTmst       = QFile( tspath ).exists();

   pb_showtmst->setEnabled( haveTmst );
   qApp->processEvents();
DbgLv(1) << "ExpAuc: haveTmst" << haveTmst << "tmst file" << tspath;
//   int noptsy     = cb_optsys->children().count();
   int noptsy     = cb_optsys->count();
DbgLv(1) << "ExpAucA: noptsy koptsy" << noptsy << cb_optsys->children().count();

   qDebug() << "Optical Systems Count: (noptsy) = " << cb_optsys->children().count();
   
   if ( noptsy > 1 )
   {  // Export data from Optical Systems other than currently selected one
      int currsx     = cb_optsys->currentIndex();

      for ( int osx = 0; osx < noptsy; osx++ )
      {
         if ( osx == currsx )  continue;   // Skip already-handled opt sys

         cb_optsys->setCurrentIndex( osx );
         correct_radii();                  // Chromatic aberration correction if needed
         int kfiles     = xpn_data->export_auc( allData ) - 2;  // Export data
         nfiles        += kfiles;          // Total files written
      }

      // Restore Optical System selection to what it was before
      cb_optsys->setCurrentIndex( currsx );
   }

   le_status  ->setText( tr( "%1 AUC/TMST files written ..." ).arg( nfiles ) );
}

// Slot to handle a change in scan exclude "from" value
void US_XpnDataViewer::exclude_from( double sfr )
{
   int scan_from  = (int)sfr;
   int scan_to    = (int)ct_to  ->value();

   if ( scan_to < scan_from )
   {
      ct_to  ->disconnect();
      ct_to  ->setValue( scan_from );

      connect( ct_to,        SIGNAL( valueChanged( double ) ),
               this,         SLOT  ( exclude_to  ( double ) ) );
   }

   plot_current();
}

// Slot to handle a change in scan exclude "to" value
void US_XpnDataViewer::exclude_to( double sto )
{
   int scan_to    = (int)sto;
   int scan_from  = (int)ct_from->value();

   if ( scan_from > scan_to )
   {
      ct_from->disconnect();
      ct_from->setValue( scan_to );

      connect( ct_from,      SIGNAL( valueChanged( double ) ),
               this,         SLOT  ( exclude_from( double ) ) );
   }

   plot_current();
}

// Slot to handle click of Exclude Scan Range
void US_XpnDataViewer::exclude_scans()
{
   int scan_from  = (int)ct_from->value();
   int scan_to    = (int)ct_to  ->value();
   int scan_knt   = 1;

   for ( int scnx = 0; scnx < nscan; scnx++ )
   {
      if ( excludes.contains( scnx ) )  continue;

      if ( scan_knt >= scan_from  &&  scan_knt <= scan_to )
         excludes << scnx;

      scan_knt++;
   }

   qSort( excludes );
   kscan      = nscan - excludes.count();
DbgLv(1) << "Excl: kscan" << kscan;
   ct_from   ->disconnect();
   ct_to     ->disconnect();
   ct_from   ->setMaximum( kscan );
   ct_to     ->setMaximum( kscan );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   ct_to     ->setValue( 0 );
   pb_include->setEnabled( true );
}

// Slot to handle click of Include All (restore of all scans)
void US_XpnDataViewer::include_scans()
{
   excludes.clear();

   kscan      = nscan;
   ktpoint    = kscan * kpoint;
DbgLv(1) << "Incl: nscan" << nscan << "kscn ecnt" << kscan << excludes.count();
   ct_to     ->setValue( 0 );
//   changeRecord();                     // Force replot
   plot_current();                     // Force replot

   ct_from   ->disconnect();
   ct_to     ->disconnect();
   ct_from   ->setMaximum( kscan );
   ct_to     ->setMaximum( kscan );
   connect( ct_from,      SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_from( double ) ) );
   connect( ct_to,        SIGNAL( valueChanged( double ) ),
            this,         SLOT  ( exclude_to  ( double ) ) );
   ct_to     ->setValue( 0 );
   pb_include->setEnabled( false );
}

// Utility to find an index in a QVector<double> to a value epsilon match
int US_XpnDataViewer::dvec_index( QVector< double >& dvec, const double dval )
{
   const double eps = 1.e-4;
   double dmag     = qAbs( dval);
//DbgLv(1) << "dvix: dval dvknt" << dval << dvec.count();

   int indx        = dvec.indexOf( dval );   // Try to find an exact match
//DbgLv(1) << "dvix: dvec0 dval indx" << dvec[0] << dval << indx;

   if ( indx < 0 )
   {  // If no exact match was found, look for a match within epsilon

      for ( int jj = 0; jj < dvec.size(); jj++ )
      {  // Search doubles vector
         double ddif     = qAbs( dvec[ jj ] - dval ) / dmag;
//if(jj<3||(jj+4)>dvec.size())
//DbgLv(1) << "dvix:   jj" << jj << "dvj" << dvec[jj] << "ddif" << ddif;

         if ( ddif < eps )
         {  // If vector value matches within epsilon, break and return
            indx            = jj;
            break;
         }
      }
   }

//DbgLv(1) << "dvix:   dval" << dval << "indx" << indx;
   return indx;
}

// Slot to pop up dialog to plot TMST values
void US_XpnDataViewer::showTimeState()
{
   QString tspath = currentDir + "/" + runID + ".time_state.tmst";
   US_TmstPlot* tsdiag = new US_TmstPlot( this, tspath );

DbgLv(1) << "sTS: tsdiag exec()";
   tsdiag->exec();
DbgLv(1) << "sTS: tsdiag DONE";
}

// Slot to report updated status text
void US_XpnDataViewer::status_report( QString stat_text )
{
   qApp->processEvents();

   le_status->setText( stat_text );

   qApp->processEvents();
}

// Slot to reload data
void US_XpnDataViewer::reloadData()
{
DbgLv(1) << "RLd:  in_reload" << in_reload;
   if ( in_reload )             // If already doing a reload,
      return;                   //  skip starting a new one

   in_reload   = true;          // Flag in the midst of a reload
   int runix          = runID.lastIndexOf( "-run" ) + 4;
   QString fRunId     = runID.mid( runix );
   int iRunId         = fRunId.toInt();
DbgLv(1) << "RLd:  runID" << runID << "runix" << runix << "iRunId" << iRunId;
   int scanmask       = 1;
   scanmask           = ( runType == "FI" ) ? 2 : scanmask;
   scanmask           = ( runType == "IP" ) ? 4 : scanmask;
   scanmask           = ( runType == "WI" ) ? 8 : scanmask;
DbgLv(1) << "RLd:     iRunId" << iRunId << "runType scanmask" << runType << scanmask;

QDateTime sttime=QDateTime::currentDateTime();
   QString smsg       = le_status->text();
   le_status->setText( tr( "Scanning Optima DB for any data updates..." ) );
   qApp->processEvents();

   // Import any newly added Scan Data records
   bool upd_ok        =  xpn_data->reimport_data( iRunId, scanmask );

   if ( ! upd_ok )
   {  // No change in data scans:  report inability to update
      nscan       = allData[ trpxs ].scanCount();
DbgLv(1) << "RLd:      upd_ok" << upd_ok << "rlt_id" << rlt_id << "nscan" << nscan;
      if ( rlt_id == 0 )    // Output message only if not auto-reload
      {
         QMessageBox::warning( this,
               tr( "Reload Data Not Possible" ),
               tr( "The \"Reload Data\" action had no effect.\n"
                   "No additional data has been recorded." ) );
      }

      if ( ! smsg.endsWith( tr( " scans)" ) ) )
      {  // If need be, add scan count to the status message
         smsg        = smsg + tr( "  (%1 scans)" ).arg( nscan );
         le_status->setText( smsg );
        qApp->processEvents();
      }
DbgLv(1) << "RLd:       NO CHANGE";
      in_reload   = false;         // Flag no longer in the midst of reload
      return;     // Return with no change in AUC data
   }
double tm1=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;

   // Otherwise, report updated raw data import
   le_status->setText( tr( "Update of Raw Optima data import complete." ) );
   qApp->processEvents();

   // Now, update the AUC data with new scans
DbgLv(1) << "RLd:      build-raw started: tm1" << tm1;
   xpn_data->rebuild_rawData( allData );

double tm2=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
DbgLv(1) << "RLd:      build-raw done: tm1 tm2" << tm1 << tm2
 << "tm2i" << (tm2-tm1);
   // Reset scan counter maximum and report update complete
   nscan       = allData[ trpxs ].scanCount();
   npoint      = allData[ trpxs ].pointCount();
   ntpoint     = nscan * npoint;
   ct_from->setMaximum( nscan );
   ct_to  ->setMaximum( nscan );
   le_status->setText( tr( "Update of AUC data complete -- now %1 scans." )
                       .arg( nscan ) );
   qApp->processEvents();

   // Do resets and re-plot the current triple
   changeCellCh();
   in_reload   = false;         // Flag no longer in the midst of reload
}


int US_XpnDataViewer::CheckExpComplete_auto( QString & runid )
{
  // Implement Optima's ExperimentRun query for RunStatus field [enum: 0 - NoRunInfo; 2- InProgress; 5- CompleteOK], look in db_defines.h of Dennis's util
  // in utils/us_xpn_data.cpp
  int exp_status =  xpn_data->checkExpStatus( runid );

  return exp_status;
}

// Slot to reload data
void US_XpnDataViewer::reloadData_auto()
{
   if ( in_reload_auto )            // If already doing a reload,
     return;                   //  skip starting a new one

   in_reload_auto   = true;          // Flag in the midst of a reload

   int runix          = runID.lastIndexOf( "-run" ) + 4;
   QString fRunId     = runID.mid( runix );
   int iRunId         = fRunId.toInt();
DbgLv(1) << "RLd:  runID" << runID << "runix" << runix << "iRunId" << iRunId;
   int scanmask       = 1;
   scanmask           = ( runType == "FI" ) ? 2 : scanmask;
   scanmask           = ( runType == "IP" ) ? 4 : scanmask;
   scanmask           = ( runType == "WI" ) ? 8 : scanmask;
DbgLv(1) << "RLd:     iRunId" << iRunId << "runType scanmask" << runType << scanmask;

QDateTime sttime=QDateTime::currentDateTime();
   QString smsg       = le_status->text();
   le_status->setText( tr( "Scanning Optima DB for any data updates..." ) );
   qApp->processEvents();

   // Import any newly added Scan Data records
   bool upd_ok        =  xpn_data->reimport_data( iRunId, scanmask );

   if ( ! upd_ok )
   {  // No change in data scans:  report inability to update
      nscan       = allData[ trpxs ].scanCount();

      if ( ! smsg.endsWith( tr( " scans)" ) ) )
      {  // If need be, add scan count to the status message
         smsg        = smsg + tr( "  (%1 scans)" ).arg( nscan );
         le_status->setText( smsg );
        qApp->processEvents();
      }
DbgLv(1) << "RLd:       NO CHANGE";

      /*** Check Experiement Status: if completed, kill the timer, export the data into AUC format, return, signal to switch panels in US_comproject ***/
      int statusExp = CheckExpComplete_auto( RunID_to_retrieve  );

      if ( statusExp == 5 || statusExp == 0 )
	{
	  if ( statusExp == 0 )
	    experimentAborted  = true;
	  
	  timer_data_reload->stop();
	  disconnect(timer_data_reload, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything

	  qDebug() << "STOPPING timer_data_reload...";

	  if ( !timer_check_sysdata->isActive()  ) // Check if sys_data Timer is stopped
	    {

	      qDebug() << "Exporing/Writing to disk...";
	      // ALEXEY Export AUC data: devise export_auc_auto() function which would return directory name with saved data - to pass to emit signal below... 
	      export_auc_auto();
	  
	      // QString mtitle_complete  = tr( "Complete!" );
	      // QString message_done     = tr( "Experiment was completed. Optima data saved..." );
	      // QMessageBox::information( this, mtitle_complete, message_done );

	      updateautoflow_record_atLiveUpdate();
	      reset_auto();
	      
	      //emit experiment_complete_auto( currentDir, ProtocolName, invID_passed, correctRadii  );  // Updtade later: what should be passed with signal ??
	      emit experiment_complete_auto( details_at_live_update );
	      
	      return;
	    }
	}

      
      // /** Experiment Aborted ***/
      
      // if ( statusExp == 0 ) //ALEXEY should be == 3 as per documentation
      // 	{
      // 	  experimentAborted  = true;

      // 	  timer_data_reload->stop();
      // 	  disconnect(timer_data_reload, SIGNAL(timeout()), 0, 0);   //Disconnect timer from anything

      // 	  if ( !timer_check_sysdata->isActive()  ) // Check if sys_data Timer is stopped
      // 	    {
      // 	      // Ask if data retrived so far should be saved:
	      
      // 	      QMessageBox msgBox;
      // 	      msgBox.setText(tr("Experiment was aborted!"));
      // 	      msgBox.setInformativeText("The data retrieved so far can be saved or disregarded. If saved, the program will proceed to the next stage (Editing). Otherwise, it will return to the initial stage (Experiment), all data will be lost.");
      // 	      msgBox.setWindowTitle(tr("Experiment Abortion"));
      // 	      QPushButton *Save      = msgBox.addButton(tr("Save Data"), QMessageBox::YesRole);
      // 	      QPushButton *Ignore    = msgBox.addButton(tr("Ignore Data"), QMessageBox::RejectRole);
	      
      // 	      msgBox.setIcon(QMessageBox::Question);
      // 	      msgBox.exec();
	      
      // 	      if (msgBox.clickedButton() == Save)
      // 		{
      // 		  export_auc_auto();

      // 		  QString mtitle_complete  = tr( "Complete!" );
      // 		  QString message_done     = tr( "Experiment was completed. Optima data saved..." );
      // 		  QMessageBox::information( this, mtitle_complete, message_done );

      // 		  updateautoflow_record_atLiveUpdate();
      // 		  emit experiment_complete_auto( currentDir, ProtocolName, invID_passed, correctRadii );  // Updtade later: what should be passed with signal ??
      // 		  return;
      // 		}
	      
      // 	      else if (msgBox.clickedButton() == Ignore)
      // 		{
      // 		  reset();
      // 		  delete_autoflow_record();
      // 		  emit return_to_experiment( ProtocolName  ); 
      // 		  return;
      // 		}
      // 	    }
      // 	}      
      
      
       in_reload_auto   = false;         // Flag no longer in the midst of reload
       return;     // Return with no change in AUC data
   }
double tm1=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;

   // Otherwise, report updated raw data import
   le_status->setText( tr( "Update of Raw Optima data import complete." ) );
   qApp->processEvents();

   // Now, update the AUC data with new scans
DbgLv(1) << "RLd:      build-raw started: tm1" << tm1;
   xpn_data->rebuild_rawData( allData );

double tm2=(double)sttime.msecsTo(QDateTime::currentDateTime())/1000.0;
DbgLv(1) << "RLd:      build-raw done: tm1 tm2" << tm1 << tm2
 << "tm2i" << (tm2-tm1);
   // Reset scan counter maximum and report update complete
   nscan       = allData[ trpxs ].scanCount();
   npoint      = allData[ trpxs ].pointCount();
   ntpoint     = nscan * npoint;
   ct_from->setMaximum( nscan );
   ct_to  ->setMaximum( nscan );
   le_status->setText( tr( "Update of AUC data complete -- now %1 scans." )
                       .arg( nscan ) );
   qApp->processEvents();

   // Do resets and re-plot the current triple
   changeCellCh();
   in_reload_auto   = false;         // Flag no longer in the midst of reload
}


// Slot to respond to a timer event (auto-reload)
void US_XpnDataViewer::timerEvent( QTimerEvent *event )
{
   int tim_id  = event->timerId();
DbgLv(1) << "            timerEvent:   tim_id" << tim_id << "    "
 << QDateTime::currentDateTime().toString( "hh:mm:ss" )
 << "  rlt_id" << rlt_id;

   if ( tim_id != rlt_id )
   {  // if other than auto-reload event, pass on to normal handler
      QWidget::timerEvent( event );
      return;
   }

   // Do a reload of data (if none currently underway)
   reloadData();
}

// Slot to handle change in auto-reload check
void US_XpnDataViewer::changeReload()
{
DbgLv(1) << "chgRld: checked" << ck_autorld->isChecked();
   if ( ck_autorld->isChecked() )
   {  // Changing to checked:  start the timer
      data_plot->replot();

      rlt_dlay    = qRound( ct_rinterv->value() * 1000.0 );
      rlt_id      = startTimer( rlt_dlay );
DbgLv(1) << "chgRld:  rlt_dlay" << rlt_dlay << "rlt_id" << rlt_id;
   }

   else
   {  // Just been unchecked:  stop the timer if need be
DbgLv(1) << "chgRld:  rlt_id" << rlt_id;
      if ( rlt_id != 0 )
      {
         killTimer( rlt_id );
DbgLv(1) << "chgRld:    STOPPED";
      }
      rlt_id      = 0;
      in_reload   = false;
   }
}

// Slot to handle change in auto-reload interval
void US_XpnDataViewer::changeInterval()
{
DbgLv(1) << "chgInt: checked" << ck_autorld->isChecked();
   if ( ! ck_autorld->isChecked() )
   {  // If no auto reload checked, ignore this interval change for now
      rlt_id      = 0;
      return;
   }

DbgLv(1) << "chgInt:  rlt_id" << rlt_id << "newInt" << ct_rinterv->value()
 << "    " << QDateTime::currentDateTime().toString( "hh:mm:ss" );
   // Otherwise, stop the timer and then restart it with a new delay
   if ( rlt_id != 0 )
   {
      killTimer( rlt_id );
   }

   changeReload();
}

// Slot to use a file dialog to select a new scan curve color map
void US_XpnDataViewer::selectColorMap()
{
   QString cmapname( "cm-rainbow" );
   QString filter  = tr( "Color Map files (*cm-*.xml);;"
                         "Any XML files (*.xml);;"
                         "Any files (*)" );

   // get an xml file name for the color map
   QString cmfpath = QFileDialog::getOpenFileName( this,
       tr( "Load Color Map File" ),
       US_Settings::etcDir(), filter, 0, 0 );

   if ( cmfpath.isEmpty() )
        return;

DbgLv(1) << "sCM: cmfpath" << cmfpath;
   US_ColorGradIO::read_color_gradient( cmfpath, mcolors );
   mcknt           = mcolors.count();
DbgLv(1) << "sCM: mcolors count" << mcknt;
   cmapname        = QFileInfo( cmfpath ).baseName().replace( ".xml$", "" );
   le_colmap->setText( cmapname );

   if ( allData.size() > 0 )
      plot_all();
}


// Apply a chromatic aberration correction to auc data radius values <--- New function: uses lambdas/corr. from DB
void US_XpnDataViewer::correct_radii()
{
   const double rad_inc = 1e-5;     // Radius precision

   char wktyp[ 3 ];
   strncpy( wktyp, allData[ 0 ].type, 2 );
   wktyp[ 2 ] = '\0';
   QString dtype = QString( wktyp );
DbgLv(1) << "CR: dtype" << dtype << wktyp;
   if ( dtype == "IP" )
   {
DbgLv(0) << "NO Chromatic Aberration correction for Interference data";
      return;			// No correction for Interference data
   }

   int ntripl = allData.count();
   int npoint = allData[ 0 ].pointCount();
   double radval;
   QString fline;
   QVector <double> lambda;
   QVector <double> correction;
   lambda.clear();
   correction.clear();

   QString chromoArrayString;
   QStringList strl;

   chromoArrayString = currentInstrument["chromoab"].trimmed();  //<--- From DB

   if ( !chromoArrayString.isEmpty() )
     {
       strl = chromoArrayString.split(QRegExp("[\r\n\t ]+"));
       
       foreach (QString str, strl)
	 {
	   str.remove("(");
	   str.remove(")");
	   
	   lambda.push_back( double( str.split(",")[0].trimmed().toFloat() ) );
	   correction.push_back( double( str.split(",")[1].trimmed().toFloat() ) );
	   
	   qDebug() << str.split(",")[0].trimmed() << " " << str.split(",")[1].trimmed();
	 }
     }
   // a correction was found
   if (correction.size() > 0)
   {
     if ( !auto_mode_bool  )
       {
	 		int response=QMessageBox::question( this,
			       tr( "Chromatic Aberration Correction:" ),
			       tr( "Wavelength correction data for currently used Optima machine\n"
				   "are found in DB and will be used to correct your data for\n"
				   "chromatic aberration between 190 nm and 800 nm.\n\n"
				   "Exported data will be modified!\n") );
			if (response == QMessageBox::No) return;
       }
      DbgLv(1) << "go ahead and correct..."; 
      // For each triple, get the wavelength; then compute and apply a correction
      for ( int jd = 0; jd < ntripl; jd++ )
      {
         int i=0;
         while ((int) lambda.at(i) != (int) allData[ jd ].scanData[ 0 ].wavelength)
         {
            i++; // Wavelength index
            if (i > 610) break;
            if (correction.at(i)  != 0.0 )
            {
               for ( int jr = 0; jr < npoint; jr++ )
               {  // Correct each radial point
                  radval = r_radii[ jr ] - correction.at(i);     // Corrected radius
                  radval = qRound( radval / rad_inc ) * rad_inc; // Rounded
                  allData[ jd ].xvalues[ jr ] = radval;          // Replace radius
               }
            }
         }
	 if (jd<3 || (jd+4)>ntripl )
	   DbgLv(1) << "c_r:  ri0 ro0" << r_radii[0] << allData[jd].xvalues[0]
		    << "rin ron" << r_radii[npoint-1] << allData[jd].xvalues[npoint-1];
      }
   }
   else // No chromatic abberation in instrument's table
     {
       if ( auto_mode_bool  )
	 {
	   correctRadii = QString("NO");
	 }
     }
}


/*
// Apply a chromatic aberration correction to auc data radius values       <--- OLD function
void US_XpnDataViewer::correct_radii()
{
   const double rad_inc = 1e-5;     // Radius precision
   int ntripl = allData.count();
   int npoint = allData[ 0 ].pointCount();
   double radval;
   QString fline;
   QVector <double> lambda;
   QVector <double> correction;
   lambda.clear();
   correction.clear();

   // If coefficient values are in an etc file, use them
   QString cofname = US_Settings::etcDir() + "/chromo-aberration-array.dat";
   QFile cofile( cofname );

   if ( cofile.open( QIODevice::ReadOnly | QIODevice::Text ) )
   {
      int i=190;
      QTextStream cotxti( &cofile );
      while ( ! cotxti.atEnd() )
      {
         fline = cotxti.readLine().simplified();
         if ( ! fline.isEmpty()  &&  ! fline.startsWith( "#" ) )
         {  // Get values from first non-empty, non-comment line
            // make sure there is one entry for each wavelength
            if (i == fline.section( " ", 0, 0 ).simplified().toInt())
            {
               lambda.push_back((double) i);
               correction.push_back(fline.section( " ", 1, 1 ).simplified().toDouble());
            }
            i++;
         }
      }
      cofile.close();
      if (i != 801)
      {
         correction.clear(); // delete any entries, invalid interpolation.
         lambda.clear();
         QMessageBox::warning( this,
            tr( "Incorrect File Format..." ),
            tr( "The wavelength correction file:\n") +
                US_Settings::etcDir() + "/chromo-aberration-array.dat\n" +
            tr( "is incorrectly formatted or contains invalid data.\n"
                "Exported data will not be corrected for any chromatic\n"
                "aberration." ) );
      }
      else
      {
         QMessageBox::warning( this,
            tr( "Chromatic Aberration Correction:" ),
            tr( "The wavelength correction file:\n") +
                US_Settings::etcDir() + "/chromo-aberration-array.dat\n" +
            tr( "is found and will be used to correct your data for chromatic\n"
                "aberration between 190 nm and 800 nm. Exported data will be modified.\n") );
      }
   }
   else
   {
      // only use chromo-aberration-coeffs.dat if chromo-aberration-array.dat isn't available
      cofname = US_Settings::etcDir() + "/chromo-aberration-coeffs.dat";
      cofile.setFileName( cofname );

      if ( cofile.open( QIODevice::ReadOnly | QIODevice::Text ) )
      {
         double a_coef  = 0.0;         // Default 5th order polynomial coefficents
         double b1_coef = 0.0;     
         double b2_coef = 0.0;
         double b3_coef = 0.0;
         double b4_coef = 0.0;
         double b5_coef = 0.0;
         QTextStream cotxti( &cofile );
         while ( ! cotxti.atEnd() )
         {
            fline = cotxti.readLine().simplified();
            if ( ! fline.isEmpty()  &&  ! fline.startsWith( "#" ) )
            {  // Get values from first non-empty, non-comment line
               a_coef  = QString( fline ).section( " ", 0, 0 )
                         .simplified().toDouble();
               b1_coef = QString( fline ).section( " ", 1, 1 )
                         .simplified().toDouble();
               b2_coef = QString( fline ).section( " ", 2, 2 )
                         .simplified().toDouble();
               b3_coef = QString( fline ).section( " ", 3, 3 )
                         .simplified().toDouble();
               b4_coef = QString( fline ).section( " ", 4, 4 )
                         .simplified().toDouble();
               b5_coef = QString( fline ).section( " ", 5, 5 )
                         .simplified().toDouble();
               break;
            }
         }
         cofile.close();
         double wl_p2, wl_p3, wl_p4, wl_p5, corr;
         for (int i=190; i<801; i++)
         {
            lambda.push_back((double) i);
            wl_p2  =      sq((double) i);      // Squared
            wl_p3  = wl_p2 * (double) i;       // Cubed
            wl_p4  = wl_p2 * wl_p2;            // To 4th power
            wl_p5  = wl_p3 * wl_p2;            // To 5th power

            // Wavelength-dependent correction
            corr = a_coef + (double) i * b1_coef
                          + wl_p2      * b2_coef
                          + wl_p3      * b3_coef
                          + wl_p4      * b4_coef
                          + wl_p5      * b5_coef;
            correction.push_back(corr);
         }
            QMessageBox::warning( this,
            tr( "Chromatic Aberration Correction:" ),
            tr( "The wavelength correction file:\n") +
                US_Settings::etcDir() + "/chromo-aberration-coeffs.dat\n" +
            tr( "is found and will be used to correct your data for chromatic aberration\n"
                "between 190 nm and 800 nm. Exported data will be modified.\n") );
      }
   }
   // a correction was found
   if (correction.size() > 0)
   {
      // For each triple, get the wavelength; then compute and apply a correction
      for ( int jd = 0; jd < ntripl; jd++ )
      {
         int i=0;
         while ((int) lambda.at(i) != (int) allData[ jd ].scanData[ 0 ].wavelength)
         {
            i++; // Wavelength index
            if (i > 610) break;
            if (correction.at(i)  != 0.0 )
            {
               for ( int jr = 0; jr < npoint; jr++ )
               {  // Correct each radial point
                  radval = r_radii[ jr ] - correction.at(i);     // Corrected radius
                  radval = qRound( radval / rad_inc ) * rad_inc; // Rounded
                  allData[ jd ].xvalues[ jr ] = radval;          // Replace radius
               }
            }
         }
if (jd<3 || (jd+4)>ntripl )
DbgLv(1) << "c_r:  ri0 ro0" << r_radii[0] << allData[jd].xvalues[0]
 << "rin ron" << r_radii[npoint-1] << allData[jd].xvalues[npoint-1];
      }
   }
}
*/



// Capture X range of latest Zoom
void US_XpnDataViewer::currentRectf( QRectF rectf )
{
   QVector< double >  ascdat;
   QVector< int > srpms;
   QVector< int > ssfscs;
   QVector< int > sslscs;
   double radv1     = qRound( rectf.left()  * 10000.0 ) * 0.0001;
   double radv2     = qRound( rectf.right() * 10000.0 ) * 0.0001;
   if ( radv2 > 7.0 )
      return;           // Skip further processing if not reasonable zoom
   int irx1         = 0;
   int irx2         = 0;
   double kpoint    = 0;
   QString cellch   = cb_cellchn ->currentText();
   QString cech     = QString( cellch ).replace( " / ", "" );

   QString impath   = US_Settings::importDir() + "/" + runID;
   QDir dir;
   if ( ! dir.exists( impath ) )
      dir.mkpath( impath );
   QString dapath   = impath + "/" + cech + ".wavelen.radpos.dat";
   QFile dafile( dapath );
   if ( !dafile.open( QIODevice::WriteOnly | QIODevice::Text ) )
   {
      return;
   }
   QTextStream datxto( &dafile );

   QString msg      = tr( "%1, %2-to-%3 Radial adjustment scan..." )
                      .arg( cellch ).arg( radv1 ).arg( radv2 );
   le_status->setText( msg );
   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
   qApp->processEvents();
DbgLv(1) << "cRect" << msg;
   datxto << "Wavelength,Meniscus\n";

   // Determine meniscus position for each wavelength of this channel
   for ( int jd = 0; jd < allData.count(); jd++ )
   {
      US_DataIO::RawData *rdata = &allData[ jd ];

      QString dchann    = QString::number( rdata->cell )
                          + QString( rdata->channel );
      if ( dchann != cech )
         continue;

      if ( irx2 == 0 )
      {  // Get radial range indexes
         irx1             = US_DataIO::index( rdata, radv1 );
         irx2             = US_DataIO::index( rdata, radv2 );
         kpoint           = irx2 - irx1 + 1;
      }

      double wavelen    = rdata->scanData[ 0 ].wavelength; // Wavelength

      // Create an average scan value vector for the search range
      int nscan         = rdata->scanCount();
      double afact      = 1.0 / (double)nscan;
      ascdat.fill( 0.0, kpoint );

      for ( int js = 0; js < nscan; js++ )
      {
         US_DataIO::Scan* dscan = &rdata->scanData[ js ];

         // Save unique speeds and each one's scan start,end indecies
         int ssrpm         = (int)qRound( dscan->rpm * 0.01 ) * 100;
         int ssx           = srpms.indexOf( ssrpm );
         if ( ssx >= 0 )
         {
            sslscs[ ssx ]     = js;
         }
         else
         {
            srpms  << ssrpm;
            ssfscs << js;
            sslscs << js;
         }

         // Accumulate average scan for the current channel
         for ( int jr = 0; jr < kpoint; jr++ )
         {
            ascdat[ jr ] += dscan->rvalues[ jr + irx1 ] * afact;
         }
      }

      // Find the position of the minimum average-scan value in the range
      double rvmin      = 1.0e+99;
      int irpos         = -1;
      for ( int jr = 0; jr < kpoint; jr++ )
      {
         double rval       = ascdat[ jr ];
         if ( rval < rvmin )
         {
            rvmin             = rval;
            irpos             = jr;
         }
      }

      // Meniscus radius value for the wavelength
      double radiusw    = rdata->xvalues[ irpos + irx1 ];
DbgLv(1) << "  wavelen/radpos:  " << wavelen << " / " << radiusw;

      // Output a wavelength,radial-position line
      QString outline   = QString::number( wavelen ) + ","
                        + QString::number( radiusw ) + "\n";
      datxto << outline;
   }  // END: datasets loop
   dafile.close();

   int nspeed        = srpms.count();

   if ( nspeed > 1 )
   {  // If multi-speed, add search/output as "wavelen/speed/radpos"
      QString dapath    = impath + "/" + cech + ".wavelen.speeds-meniscus.dat";
      QFile dafile( dapath );
      dafile.open( QIODevice::WriteOnly | QIODevice::Text );
      QTextStream datxto( &dafile );
      QString outline   = tr( "Wavelength" );
      for ( int jq = 0; jq < nspeed; jq++ )
      {  // Add speeds to header line
         int irpm          = srpms[ jq ];
         outline          += "," + QString::number( irpm );
      }
      outline          += "\n";
      datxto << outline;
      for ( int jd = 0; jd < allData.count(); jd++ )
      {  // Get information for each dataset
         US_DataIO::RawData *rdata = &allData[ jd ];

         QString dchann    = QString::number( rdata->cell )
                             + QString( rdata->channel );
         if ( dchann != cech )
            continue;

         if ( irx2 == 0 )
         {  // Get radial range indexes
            irx1             = US_DataIO::index( rdata, radv1 );
            irx2             = US_DataIO::index( rdata, radv2 );
            kpoint           = irx2 - irx1 + 1;
         }

         double wavelen    = rdata->scanData[ 0 ].wavelength; // Wavelength
         // Start data line with wavelength value
         outline           = QString::number( wavelen );

         for ( int jq = 0; jq < nspeed; jq++ )
         {
            // Create an average scan value vector for the search range
            int irpm          = srpms [ jq ];
            int jsf           = ssfscs[ jq ];
            int jsl           = sslscs[ jq ] + 1;
            int nscan         = jsl - jsf;
            double afact      = 1.0 / (double)nscan;
            ascdat.fill( 0.0, kpoint );
            for ( int js = jsf; js < jsl; js++ )
            {  // Average scans in the current speed step
               US_DataIO::Scan* dscan = &rdata->scanData[ js ];

               for ( int jr = 0; jr < kpoint; jr++ )
               {
                  ascdat[ jr ] += dscan->rvalues[ jr + irx1 ] * afact;
               }
            }

            // Find the position of the minimum value in the range
            double rvmin      = 1.0e+99;
            int irpos         = -1;
            for ( int jr = 0; jr < kpoint; jr++ )
            {
               double rval       = ascdat[ jr ];
               if ( rval < rvmin )
               {
                  rvmin             = rval;
                  irpos             = jr;
               }
            }

            double radiusw    = rdata->xvalues[ irpos + irx1 ];
DbgLv(1) << "  wavelen/speed/radpos:  " << wavelen
   << " / " << irpm << " / " << radiusw;

            outline          += "," + QString::number( radiusw );
         }  // END: speed loop
         outline          += "\n";
         datxto << outline;
      }  // END: dataset loop

      dafile.close();
   }  // END: multi-speed

   le_status->setText( tr( "%1  Radial adjustment scan complete." )
                      .arg( cech ) );
   QApplication::restoreOverrideCursor();
   qApp->processEvents();
}

