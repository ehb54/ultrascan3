//! \file us_ga_init.cpp

#include <QApplication>
#include "us_grid_editor.h"
#include "us_gui_util.h"
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#define setMinimum(a)      setMinValue(a)
#define setMaximum(a)      setMaxValue(a)
#define setSymbol(a)       setSymbol(*a)
#endif

const double MPISQ   = M_PI * M_PI;
const double THIRD   = 1.0 / 3.0;
const double VOL_FAC = 0.75 / M_PI;
const double SPH_FAC = 0.06 * M_PI * VISC_20W;
const int    MINSSZ  = 10;
const int    MAXSSZ  = 800;
const int    DEFSSZ  = 100;

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_Grid_Editor w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// US_Grid_Editor class constructor
US_Grid_Editor::US_Grid_Editor() : US_Widgets()
{
   // set up the GUI
   QString str;

   setWindowTitle( tr( "UltraScan 2DSA Grid Initialization Editor" ) );
   setPalette( US_GuiSettings::frameColor() );

   // primary layouts
   QHBoxLayout* main  = new QHBoxLayout( this );
   QVBoxLayout* right = new QVBoxLayout();
   QGridLayout* left  = new QGridLayout();
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing        ( 1 );
   left->setContentsMargins( 0, 0, 0, 0 );
   right->setSpacing        ( 0 );
   right->setContentsMargins( 0, 1, 0, 1 );
   viscosity = VISC_20W;
   density   = DENS_20W;
   vbar      = TYPICAL_VBAR;

   int s_row = 0;
   dbg_level = US_Settings::us_debug();

   // series of rows: most of them label on left, counter/box on right
   lb_info1      = us_banner( tr( "Grid Editor Controls" ) );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(),
                                  1, true );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   lb_xaxis      = us_label( tr( "Adjust X-Axis as:" ) );
   lb_xaxis->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   lb_yaxis      = us_label( tr( "Adjust Y-Axis as:" ) );
   lb_yaxis->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   QGridLayout* x_s     = us_radiobutton( tr( "Sedimentation Coeff." ),
                                          rb_x_s,    true );
   QGridLayout* x_ff0   = us_radiobutton( tr( "Frictional Ratio" ),
                                          rb_x_ff0,  false );
   QGridLayout* x_mw    = us_radiobutton( tr( "Molecular Weight" ),
                                          rb_x_mw,   true );
   QGridLayout* x_vbar  = us_radiobutton( tr( "Partial Specific Volume" ),
                                          rb_x_vbar, true );
   QGridLayout* x_D     = us_radiobutton( tr( "Diffusion Coefficient" ),
                                          rb_x_D,    true );
   QGridLayout* x_f     = us_radiobutton( tr( "Frictional Coefficient" ),
                                          rb_x_f,    true );
   QGridLayout* y_s     = us_radiobutton( tr( "Sedimentation Coeff." ),
                                          rb_y_s,    false );
   QGridLayout* y_ff0   = us_radiobutton( tr( "Frictional Ratio" ),
                                          rb_y_ff0,  true );
   QGridLayout* y_mw    = us_radiobutton( tr( "Molecular Weight" ),
                                          rb_y_mw,   true );
   QGridLayout* y_vbar  = us_radiobutton( tr( "Partial Specific Volume" ),
                                          rb_y_vbar, true );
   QGridLayout* y_D     = us_radiobutton( tr( "Diffusion Coefficient" ),
                                          rb_y_D,    true );
   QGridLayout* y_f     = us_radiobutton( tr( "Frictional Coefficient" ),
                                          rb_y_f,    true );

   QGridLayout* toggle1 = us_radiobutton( tr( "X-Axis View" ),
                                          rb_plot1, true );
   QGridLayout* toggle2 = us_radiobutton( tr( "Molecular Weight View" ),
                                          rb_plot2, true );

   QButtonGroup* x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    ATTR_S );
   x_axis->addButton( rb_x_ff0,  ATTR_K );
   x_axis->addButton( rb_x_mw,   ATTR_W );
   x_axis->addButton( rb_x_vbar, ATTR_V );
   x_axis->addButton( rb_x_D,    ATTR_D );
   x_axis->addButton( rb_x_f,    ATTR_F );
   connect( x_axis, SIGNAL( buttonReleased( int ) ),
                    SLOT  ( select_x_axis ( int ) ) );

   QButtonGroup* y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    ATTR_S );
   y_axis->addButton( rb_y_ff0,  ATTR_K );
   y_axis->addButton( rb_y_mw,   ATTR_W );
   y_axis->addButton( rb_y_vbar, ATTR_V );
   y_axis->addButton( rb_y_D,    ATTR_D );
   y_axis->addButton( rb_y_f,    ATTR_F );
   connect( y_axis, SIGNAL( buttonReleased( int ) ),
                    SLOT  ( select_y_axis ( int ) ) );

   QButtonGroup* toggle_plot = new QButtonGroup( this );
   toggle_plot->addButton( rb_plot1, 0 );
   toggle_plot->addButton( rb_plot2, 1 );
   rb_plot1   ->setChecked( true );
   connect( toggle_plot, SIGNAL( buttonReleased( int ) ),
                         SLOT  ( select_plot   ( int ) ) );

   lb_fixed      = us_label( tr( "Fixed Attribute:" ) );
   cb_fixed      = us_comboBox();
   cb_fixed->addItem( tr( "Partial Specific Volume" ) );
   cb_fixed->addItem( tr( "Frictional Ratio" ) );
   cb_fixed->addItem( tr( "Molecular Weight" ) );
   cb_fixed->addItem( tr( "Sedimentation Coefficient" ) );
   cb_fixed->addItem( tr( "Diffusion Coefficient" ) );
   cb_fixed->addItem( tr( "Frictional Coefficient" ) );
   cb_fixed->setEnabled( true );
   cb_fixed->setCurrentIndex( 0 );
   connect( cb_fixed, SIGNAL( activated   ( const QString& ) ),
            this,     SLOT  ( select_fixed( const QString& ) ) );

   lb_xRes      = us_label( tr( "X Resolution:" ) );
   lb_xRes->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xRes      = us_counter( 3, 1.0, 1000.0, 60.0 );
   ct_xRes->setSingleStep( 1 );
   connect( ct_xRes,  SIGNAL( valueChanged( double ) ),
            this,     SLOT  ( update_xRes ( double ) ) );

   lb_yRes      = us_label( tr( "f/f0 Resolution:" ) );
   lb_yRes->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yRes      = us_counter( 3, 1.0, 1000.0, 60.0 );
   ct_yRes->setSingleStep( 1 );
   connect( ct_yRes,  SIGNAL( valueChanged( double ) ),
            this,     SLOT  ( update_yRes ( double ) ) );

   lb_xMin     = us_label( tr( "s (x 1e13) Minimum:" ) );
   lb_xMin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xMin     = us_counter( 3, -500000.0, 500000.0, 0.1 );
   ct_xMin->setSingleStep( 1 );
   connect( ct_xMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMin ( double ) ) );

   lb_xMax     = us_label( tr( "s (x 1e13) Maximum:" ) );
   lb_xMax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_xMax     = us_counter( 3, -500000.0, 500000.0, 0.1 );
   ct_xMax->setSingleStep( 1 );
   connect( ct_xMax, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMax ( double ) ) );

   lb_yMin     = us_label( tr( "f/f0 Minimum:" ) );
   lb_yMin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yMin     = us_counter( 3, 1.0, 50.0, 0.1 );
   ct_yMin->setSingleStep( 1 );
   connect( ct_yMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT(   update_yMin ( double ) ) );

   lb_yMax     = us_label( tr( "f/f0 Maximum:" ) );
   lb_yMax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_yMax     = us_counter( 3, 1.0, 50.0, 0.1 );
   ct_yMax->setSingleStep( 1 );
   connect( ct_yMax, SIGNAL( valueChanged( double ) ),
         this,       SLOT(   update_yMax ( double ) ) );

   lb_zVal     = us_label( tr( "Partial Specific Volume:" ) );
   lb_zVal->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_zVal     = us_counter( 3, 0.01, 3.0, 0.001 );
   ct_zVal->setSingleStep ( 1 );
   ct_zVal->setValue( 0.72 );
   connect( ct_zVal, SIGNAL( valueChanged( double ) ),
         this,       SLOT(   update_zVal ( double ) ) );

   lb_density     = us_label( tr( "Density:" ) );
   lb_density->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   str.setNum( density );
   QLineEdit* le_density = us_lineedit( str.setNum( density ));
   connect( le_density, SIGNAL( textChanged   ( const QString& ) ),
            this,       SLOT  ( update_density( const QString& ) ) );

   lb_viscosity     = us_label( tr( "Viscosity:" ) );
   lb_viscosity->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   str.setNum( viscosity );
   QLineEdit* le_viscosity = us_lineedit( str.setNum( viscosity ));
   connect( le_viscosity, SIGNAL( textChanged     ( const QString& ) ),
            this,         SLOT  ( update_viscosity( const QString& ) ) );

   pb_add_partialGrid       = us_pushbutton( tr( "Add this Grid" ) );
   pb_add_partialGrid->setEnabled( true );
   connect( pb_add_partialGrid,    SIGNAL( clicked()         ),
            this,                  SLOT  ( add_partialGrid() ) );

   QGridLayout *showgrid = us_checkbox( tr("Show Final Grid"),
                                        ck_show_final_grid, false );
   connect( ck_show_final_grid, SIGNAL( clicked        ( bool ) ),
            this,               SLOT  ( show_final_grid( bool ) ) );
   ck_show_final_grid->setEnabled( false );

   pb_delete_partialGrid       = us_pushbutton( tr( "Delete Partial Grid" ) );
   pb_delete_partialGrid->setEnabled( false );
   connect( pb_delete_partialGrid, SIGNAL( clicked()            ),
            this,                  SLOT  ( delete_partialGrid() ) );

   QGridLayout *showsubgrid = us_checkbox( tr("Show Subgrids"), ck_show_sub_grid, false );
   connect( ck_show_sub_grid, SIGNAL( clicked(bool) ),
   this,    SLOT( show_sub_grid(bool) ) );
   ck_show_sub_grid->setEnabled( false );

   lb_partialGrid     = us_label( tr( "Highlight Partial Grid #:" ) );
   lb_partialGrid->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_partialGrid     = us_counter( 3, 0, 1000, 0.0 );
   ct_partialGrid->setSingleStep( 1 );
   ct_partialGrid->setEnabled( false );
   connect( ct_partialGrid, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_partialGrid( double ) ) );

   lb_subGrid     = us_label( tr( "Number of Subgrids:" ) );
   lb_subGrid->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_subGrids     = us_counter( 3, 1, 500, 13.0 );
   ct_subGrids->setSingleStep( 1 );
   ct_subGrids->setEnabled( false );
   connect( ct_subGrids, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_subGrids( double ) ) );

   QLabel* lb_counts = us_label( tr( "Number of Grid Points:" ) );
   le_counts     = us_lineedit( tr( "0 total, 0 per subgrid" ), 0, true );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT(   save() ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close() ) );

   // set up plot component window on right side

   QBoxLayout* plot1 = new US_Plot( data_plot1,
      tr( "Grid Layout" ),
      tr( "Sedimentation Coefficient (s20,W)"),
      tr( "Frictional Ratio f/f0" ) );
   data_plot1->setAutoDelete( true );
   data_plot1->setMinimumSize( 640, 480 );
   data_plot1->enableAxis( QwtPlot::xBottom, true );
   data_plot1->enableAxis( QwtPlot::yLeft,   true );
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   left->addWidget( lb_info1,              s_row++, 0, 1, 4 );
   left->addWidget( pb_investigator,       s_row,   0, 1, 2 );
   left->addWidget( le_investigator,       s_row++, 2, 1, 2 );
   left->addLayout( dkdb_cntrls,           s_row++, 0, 1, 4 );
   left->addWidget( lb_xaxis,              s_row,   0, 1, 2 );
   left->addWidget( lb_yaxis,              s_row++, 2, 1, 2 );
   left->addLayout( x_s,                   s_row,   0, 1, 2 );
   left->addLayout( y_s,                   s_row++, 2, 1, 2 );
   left->addLayout( x_ff0,                 s_row,   0, 1, 2 );
   left->addLayout( y_ff0,                 s_row++, 2, 1, 2 );
   left->addLayout( x_mw,                  s_row,   0, 1, 2 );
   left->addLayout( y_mw,                  s_row++, 2, 1, 2 );
   left->addLayout( x_vbar,                s_row,   0, 1, 2 );
   left->addLayout( y_vbar,                s_row++, 2, 1, 2 );
   left->addLayout( x_D,                   s_row,   0, 1, 2 );
   left->addLayout( y_D,                   s_row++, 2, 1, 2 );
   left->addLayout( x_f,                   s_row,   0, 1, 2 );
   left->addLayout( y_f,                   s_row++, 2, 1, 2 );
   left->addWidget( lb_fixed,              s_row,   0, 1, 2 );
   left->addWidget( cb_fixed,              s_row++, 2, 1, 2 );
   left->addLayout( toggle1,               s_row,   0, 1, 2 );
   left->addLayout( toggle2,               s_row++, 2, 1, 2 );
   left->addWidget( lb_xRes,               s_row,   0, 1, 2 );
   left->addWidget( ct_xRes,               s_row++, 2, 1, 2 );
   left->addWidget( lb_yRes,               s_row,   0, 1, 2 );
   left->addWidget( ct_yRes,               s_row++, 2, 1, 2 );
   left->addWidget( lb_xMin,               s_row,   0, 1, 2 );
   left->addWidget( ct_xMin,               s_row++, 2, 1, 2 );
   left->addWidget( lb_xMax,               s_row,   0, 1, 2 );
   left->addWidget( ct_xMax,               s_row++, 2, 1, 2 );
   left->addWidget( lb_yMin,               s_row,   0, 1, 2 );
   left->addWidget( ct_yMin,               s_row++, 2, 1, 2 );
   left->addWidget( lb_yMax,               s_row,   0, 1, 2 );
   left->addWidget( ct_yMax,               s_row++, 2, 1, 2 );
   left->addWidget( lb_zVal,               s_row,   0, 1, 2 );
   left->addWidget( ct_zVal,               s_row++, 2, 1, 2 );
   left->addWidget( lb_density,            s_row,   0, 1, 1 );
   left->addWidget( le_density,            s_row,   1, 1, 1 );
   left->addWidget( lb_viscosity,          s_row,   2, 1, 1 );
   left->addWidget( le_viscosity,          s_row++, 3, 1, 1 );
   left->addWidget( pb_add_partialGrid,    s_row,   0, 1, 2 );
   left->addLayout( showgrid,              s_row++, 2, 1, 2 );
   left->addWidget( pb_delete_partialGrid, s_row,   0, 1, 2 );
   left->addLayout( showsubgrid,           s_row++, 2, 1, 2 );
   left->addWidget( lb_partialGrid,        s_row,   0, 1, 2 );
   left->addWidget( ct_partialGrid,        s_row++, 2, 1, 2 );
   left->addWidget( lb_subGrid,            s_row,   0, 1, 2 );
   left->addWidget( ct_subGrids,           s_row++, 2, 1, 2 );
   left->addWidget( lb_counts,             s_row,   0, 1, 2 );
   left->addWidget( le_counts,             s_row++, 2, 1, 2 );
   left->addWidget( pb_reset,              s_row,   0, 1, 1 );
   left->addWidget( pb_save,               s_row,   1, 1, 1 );
   left->addWidget( pb_help,               s_row,   2, 1, 1 );
   left->addWidget( pb_close,              s_row++, 3, 1, 1 );

   right->addLayout( plot1 );

   main->addLayout( left );
   main->addLayout( right );
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( right, 6 );

   reset();
}

// reset the GUI
void US_Grid_Editor::reset( void )
{
   dataPlotClear( data_plot1 );
   pick1 = new US_PlotPicker( data_plot1 );

   xRes          = 60.0;
   yRes          = 60.0;
qDebug() << "reset yRes" << yRes;
   yMin          = 1.0;
   yMax          = 4.0;
   xMin          = 1.0;
   xMax          = 10.0;
   zVal          = 0.72;
   vbar          = 0.72;
   ff0           = 1.0;
   plot_x        = ATTR_S; // plot s
   plot_y        = ATTR_K; // plot f/f0
   plot_z        = ATTR_V; // fixed vbar
   selected_plot = 0;
   viscosity     = VISC_20W;
   density       = DENS_20W;
   grid_index    = 0;
   partialGrid   = 0;
   subGrids      = 13;
   final_grid.clear();

qDebug() << "1)set yRes" << yRes;
   ct_xRes->setRange     ( 1.0, 1000.0 );
   ct_xRes->setSingleStep( 1.0 );
qDebug() << "2)set yRes" << yRes;
   ct_xRes->setValue( xRes );
qDebug() << "3)set yRes" << yRes;
   ct_yRes->setRange     ( 1.0, 1000.0 );
   ct_yRes->setSingleStep( 1.0 );
qDebug() << "set yRes" << yRes;
   ct_yRes->setValue( yRes );
   ct_partialGrid    ->setEnabled( false );
   ct_subGrids       ->setEnabled( false );
   ct_partialGrid    ->setRange( 0, 0);
   ct_partialGrid    ->setSingleStep( 0 );
   ct_partialGrid    ->setValue( 0 );
   ck_show_final_grid->setEnabled( false );
   ck_show_final_grid->setChecked( false );
   ck_show_sub_grid  ->setEnabled( false );
   ck_show_sub_grid  ->setChecked( false );
   ct_xRes  ->setEnabled( true );
   ct_yRes  ->setEnabled( true );
   ct_xMin  ->setEnabled( true );
   ct_yMin  ->setEnabled( true );
   ct_xMax  ->setEnabled( true );
   ct_yMax  ->setEnabled( true );
   ct_zVal  ->setEnabled( true );
   rb_x_s   ->setEnabled( true );
   rb_x_ff0 ->setEnabled( false );
   rb_x_mw  ->setEnabled( true );
   rb_x_vbar->setEnabled( true );
   rb_x_D   ->setEnabled( true );
   rb_x_f   ->setEnabled( true );
   rb_y_s   ->setEnabled( false );
   rb_y_ff0 ->setEnabled( true );
   rb_y_mw  ->setEnabled( true );
   rb_y_vbar->setEnabled( true );
   rb_y_D   ->setEnabled( true );
   rb_y_f   ->setEnabled( true );
   rb_x_s   ->setChecked( true );
   rb_y_ff0 ->setChecked( true );
   pb_add_partialGrid   ->setEnabled( true );
   pb_save              ->setEnabled( false );
   pb_delete_partialGrid->setEnabled( false );

   select_x_axis( plot_x );
   select_y_axis( plot_y );
   select_fixed ( cb_fixed->currentText() );

   select_plot  ( selected_plot );
   update_plot  ();
}

// save the grid data
void US_Grid_Editor::save( void )
{
   US_Model model;
   US_Model::SimulationComponent sc;
   QString modelPath, modelGuid;
   US_Model::model_path( modelPath );
   QDateTime now_time = QDateTime::currentDateTime ();
   QList< double > xvals;
   QList< double > yvals;
   QList< double > zvals;
   double          gridinc = 1.0 / (double)subGrids;
   int             indexsg = 1;
   bool flag;
   modelGuid         = US_Util::new_guid();
   model.analysis    = US_Model::CUSTOMGRID;
   model.description = now_time.toString( "yyyyMMdd-hhmm")
      + "-CustomGrid" + ".model";
   model.subGrids    = subGrids;
   model.modelGUID   = modelGuid;
   model.global      = US_Model::NONE;
   double vbmin      = 1e99;
   double vbmax      = -1e99;
   double ffmin      = 1e99;
   double ffmax      = -1e99;
   sc.signal_concentration = 1.0;

   for ( int ii = 0; ii < final_grid.size(); ii++ )
   {
      flag        = true;
      sc.s        = final_grid[ ii ].s * 1.0e-13;
      sc.D        = final_grid[ ii ].D;
      sc.f        = final_grid[ ii ].f;
      sc.f_f0     = final_grid[ ii ].ff0;
      sc.vbar20   = final_grid[ ii ].vbar;
      sc.mw       = final_grid[ ii ].mw;

      double xval = grid_value( final_grid[ ii ], plot_x );
      double yval = grid_value( final_grid[ ii ], plot_y );
      double zval = grid_value( final_grid[ ii ], plot_z );
      int indexx  = xvals.indexOf( xval ) + 1;
      int indexy  = yvals.indexOf( yval ) + 1;
      int indexz  = zvals.indexOf( zval ) + 1;
      if ( indexx < 1 )  { indexx = xvals.size() + 1; xvals << xval; }
      if ( indexy < 1 )  { indexy = yvals.size() + 1; yvals << yval; }
      if ( indexz < 1 )  { indexz = zvals.size() + 1; zvals << zval; }
      sc.name     = QString().sprintf( "X%3.3dY%3.3dZ%2.2d",
                                       indexx, indexy, indexz );
      sc.signal_concentration = gridinc * (double)indexsg;
      if ( (++indexsg) > subGrids )  indexsg = 1;

      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         vbmin     = qMin( vbmin, sc.vbar20 );  // Accumulate vbar,f/f0 extents
         vbmax     = qMax( vbmax, sc.vbar20 );
         ffmin     = qMin( ffmin, sc.f_f0   );
         ffmax     = qMax( ffmax, sc.f_f0   );

         if ( sc.s      == model.components[ jj ].s     &&
              sc.f_f0   == model.components[ jj ].f_f0  &&
              sc.mw     == model.components[ jj ].mw    &&
              sc.vbar20 == model.components[ jj ].vbar20 )
         {
            flag = false;
            break; // don't add a component that is already in the model
         }
      }

      if ( flag ) model.components.push_back(sc);
   }

   // Open a dialog that reports and allows modification of description
   QMessageBox mbox;
   QString msg1    = tr( "A grid model has been created. "
                         "It's description is:<br/><b>" )
      + model.description + "</b>.<br/><br/>"
      + tr( "Click:<br/><br/>" )
      + tr( "  <b>OK</b>     to output the model as is;<br/>"
            "  <b>Edit</b>   to append custom text to the name;<br/>"
            "  <b>Cancel</b> to abort model creation.<br/>" );

   mbox.setWindowTitle( tr( "Save Grid Model" ) );
   mbox.setText       ( msg1 );
   QPushButton *pb_ok   = mbox.addButton( tr( "OK" ),
         QMessageBox::YesRole );
   QPushButton *pb_edit = mbox.addButton( tr( "Edit" ) ,
         QMessageBox::AcceptRole );
   QPushButton *pb_canc = mbox.addButton( tr( "Cancel" ),
         QMessageBox::RejectRole );
   mbox.setEscapeButton ( pb_canc );
   mbox.setDefaultButton( pb_ok   );

   mbox.exec();

   if ( mbox.clickedButton() == pb_canc )  return;

   if ( mbox.clickedButton() == pb_edit )
   {  // Open another dialog to get a modified runID
      bool    ok;
      QString newtext = "";
      int     jj      = model.description.indexOf( ".model" );
      if ( jj > 0 ) model.description = model.description.left( jj );
      QString msg2    = tr( "The default run ID for the grid model<br/>"
                            "is <b>" ) + model.description + "</b>.<br/><br/>"
         + tr( "You may append additional text to the model description.<br/>"
               "Use alphanumeric characters, underscores, or hyphens<br/>"
               "(no spaces). Enter 1 to 40 characters." );
      newtext = QInputDialog::getText( this,
            tr( "Modify Model Name" ),
            msg2,
            QLineEdit::Normal,
            newtext,
            &ok );

      if ( !ok )  return;

      newtext.remove( QRegExp( "[^\\w\\d_-]" ) );

      int     slen    = newtext.length();
      if ( slen > 40 ) newtext = newtext.left( 40 );
      // add string containing
      model.description = model.description + "-" + newtext + ".model";
   }

   // Output the combined grid model
   int code;
   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      code = model.write( &db );
   }
   else
   {
      bool newFile;
      QString fnamo = US_Model::get_filename( modelPath, modelGuid, newFile );
      code = model.write( fnamo );
   }

   QString mtitle = tr( "Grid Model Saving..." );

   if ( code == US_DB2::OK )
   {
      QString destination = dkdb_cntrls->db() ?
                            tr ( "local disk and database." ) :
                            tr ( "local disk." );
      QMessageBox::information( this, mtitle,
         tr( "The file \"" ) +  model.description
         + tr( "\"\n  was successfully saved to " ) + destination );
   }
   else
   {
      QMessageBox::warning( this, mtitle,
         tr( "Writing the model file  \"") +  model.description
         + tr( "\"\n  resulted in error code " )
         + QString::number( code ) + " ." );
   }
}

// update raster x resolution
void US_Grid_Editor::update_xRes( double dval )
{
qDebug() << "ux1)yRes" << yRes;
   xRes  = dval;
qDebug() << "ux2)yRes" << yRes;
   update_plot();
qDebug() << "ux3)yRes" << yRes;
}

// update raster y resolution
void US_Grid_Editor::update_yRes( double dval )
{
   yRes  = dval;
   update_plot();
}

// update plot limit x min
void US_Grid_Editor::update_xMin( double dval )
{
   xMin    = dval;
   ct_xMax->disconnect();
   ct_xMax->setMinimum( xMin );

   connect( ct_xMax, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMax ( double ) ) );

   validate_ff0();

   update_plot();
}

// update plot limit x max
void US_Grid_Editor::update_xMax( double dval )
{
   xMax    = dval;
   ct_xMin->disconnect();
   ct_xMin->setMaximum( xMax );

   connect( ct_xMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMin ( double ) ) );

   validate_ff0();

   update_plot();
}

// update plot limit y min
void US_Grid_Editor::update_yMin( double dval )
{
   yMin    = dval;
qDebug() << "update_yMin" << yMin;
   ct_yMax->disconnect();
   ct_yMax->setMinimum( yMin );

   connect( ct_yMax, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_yMax ( double ) ) );

   validate_ff0();

   update_plot();
}

// update plot limit y max
void US_Grid_Editor::update_yMax( double dval )
{
   yMax    = dval;
qDebug() << "update_yMax" << yMax;
   ct_yMin->disconnect();
   ct_yMin->setMaximum( yMax );

   connect( ct_yMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_yMin ( double ) ) );

   validate_ff0();

   update_plot();
}

// update plot limit z-value (f/f0 or vbar)
void US_Grid_Editor::update_zVal( double dval )
{
   zVal    = dval;
   vbar    = ( plot_y == ATTR_V ) ? zVal : vbar;
   ff0     = ( plot_y == ATTR_K ) ? zVal : ff0;

   validate_ff0();

   update_plot();
}

// Select a partialGrid from all subgrids in the final grid for highlighting
void US_Grid_Editor::update_partialGrid( double dval )
{
   partialGrid = (int) dval;
   update_plot();
}

// Select a subgrid from the final grid for highlighting:
void US_Grid_Editor::update_subGrids( double dval )
{
   int ntotg       = final_grid.size();
   subGrids        = (int)dval;
   ct_partialGrid->setRange     ( 1, subGrids );
   ct_partialGrid->setSingleStep( 1 );
   le_counts->setText( tr( "%1 total, %2 per subgrid" )
         .arg( ntotg ).arg( ntotg / subGrids ) );
   update_plot();
}

// update density
void US_Grid_Editor::update_density( const QString & str )
{
   // Skip updating if not likely done entering
   if ( str.toDouble() == 0.0 )   return;

   // Update density value and re-plot
   density = str.toDouble();
   update_plot();
}

// update viscosity
void US_Grid_Editor::update_viscosity( const QString & str )
{
   // Skip updating if not likely done entering
   if ( str.toDouble() == 0.0 )   return;

   // Update viscosity value and re-plot
   viscosity = str.toDouble();
   update_plot();
}

// update plot
void US_Grid_Editor::update_plot( void )
{
qDebug() << "update_plot:  call calc_gridpoints()";
   calc_gridpoints();

   QString xatitle = tr( "Sedimentation Coefficient" );
   QString yatitle = tr( "Frictional Ratio" );
qDebug() << "  up0)yRes" << yRes;

   switch ( plot_x )
   {
      default:
      case ATTR_S:
         xatitle         = tr( "Sedimentation Coefficient" );
         break;
      case ATTR_K:
         xatitle         = tr( "Frictional Ratio" );
         break;
      case ATTR_W:
         xatitle         = tr( "Molecular Weight" );
         break;
      case ATTR_V:
         xatitle         = tr( "Partial Specific Volume" );
         break;
      case ATTR_D:
         xatitle         = tr( "Diffusion Coefficient" );
         break;
      case ATTR_F:
         xatitle         = tr( "Frictional Coefficient" );
         break;
   }

   switch ( plot_y )
   {
      case ATTR_S:
         yatitle         = tr( "Sedimentation Coefficient" );
         break;
      default:
      case ATTR_K:
         yatitle         = tr( "Frictional Ratio" );
         break;
      case ATTR_W:
         yatitle         = tr( "Molecular Weight" );
         break;
      case ATTR_V:
         yatitle         = tr( "Partial Specific Volume" );
         break;
      case ATTR_D:
         yatitle         = tr( "Diffusion Coefficient" );
         break;
      case ATTR_F:
         yatitle         = tr( "Frictional Coefficient" );
         break;
   }

   if ( selected_plot == 1 )
      xatitle         = tr( "Molecular Weight" );

   dataPlotClear( data_plot1 );
   data_plot1->setAxisTitle( QwtPlot::xBottom, xatitle );
   data_plot1->setAxisTitle( QwtPlot::yLeft,   yatitle );

   //print_minmax();
   int gridsize;
   QVector <double> xData1;
   QVector <double> yData1;
   QVector <double> xData2;
   QVector <double> yData2;

   int iplt_x = ( selected_plot == 0 ) ? plot_x : ATTR_W;

   xData1.clear();
   yData1.clear();
   xData2.clear();
   yData2.clear();

   if ( ck_show_final_grid->isChecked()  &&
        !ck_show_sub_grid->isChecked())
   {
      gridsize = final_grid.size();

      for ( int ii = 0; ii < gridsize; ii++ )
      {
         if ( final_grid[ ii ].index == partialGrid )
         {
            xData1 << grid_value( final_grid[ ii ], iplt_x );
            yData1 << grid_value( final_grid[ ii ], plot_y );
         }

         else
         {
            xData2 << grid_value( final_grid[ ii ], iplt_x );
            yData2 << grid_value( final_grid[ ii ], plot_y );
         }
      }

      QwtPlotCurve *c1;
      QwtSymbol*   sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setBrush( QColor( Qt::red ) );
      sym1->setPen  ( QColor( Qt::red ) );
      sym1->setSize ( 3 );

      c1 = us_curve( data_plot1, "highlighted Grid points" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

      QwtPlotCurve *c2;
      QwtSymbol*   sym2 = new QwtSymbol;
      sym2->setStyle( QwtSymbol::Ellipse );
      sym2->setBrush( QColor( Qt::yellow ) );
      sym2->setPen  ( QColor( Qt::yellow ) );
      sym2->setSize ( 3 );

      c2 = us_curve( data_plot1, "Other Grid points" );
      c2->setSymbol ( sym2 );
      c2->setStyle  ( QwtPlotCurve::NoCurve );
      c2->setSamples( xData2.data(), yData2.data(), xData2.size() );
   }

   else if ( ck_show_final_grid->isChecked()  &&
             ck_show_sub_grid->isChecked())
   {
      gridsize    = final_grid.size();
      int counter = 1;

      for ( int ii = 0; ii < gridsize; ii++ )
      {
         if ( counter == partialGrid )
         {
            xData1 << grid_value( final_grid[ ii ], iplt_x );
            yData1 << grid_value( final_grid[ ii ], plot_y );
         }

         else
         {
            xData2 << grid_value( final_grid[ ii ], iplt_x );
            yData2 << grid_value( final_grid[ ii ], plot_y );
         }

         counter++;

         if ( counter > subGrids )
            counter = 1;
      }

      QwtPlotCurve *c1;
      QwtSymbol*   sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setBrush( QColor( Qt::red ) );
      sym1->setPen  ( QColor( Qt::red ) );
      sym1->setSize( 3 );

      c1 = us_curve( data_plot1, "highlighted Grid points" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

      QwtPlotCurve *c2;
      QwtSymbol*   sym2 = new QwtSymbol;
      sym2->setStyle( QwtSymbol::Ellipse );
      sym2->setBrush( QColor( Qt::yellow ) );
      sym2->setPen  ( QColor( Qt::yellow ) );
      sym2->setSize( 3 );

      c2 = us_curve( data_plot1, "Other Grid points" );
      c2->setSymbol ( sym2 );
      c2->setStyle  ( QwtPlotCurve::NoCurve );
      c2->setSamples( xData2.data(), yData2.data(), xData2.size() );
   }

   else
   {  // Set up current grid plot
      gridsize    = current_grid.size();
qDebug() << "  updplt: gridsize" << gridsize;
      xData1.resize( gridsize );
      yData1.resize( gridsize );

      for ( int ii = 0; ii < gridsize; ii++ )
      {
         xData1[ ii ] = grid_value( current_grid[ ii ], iplt_x );
         yData1[ ii ] = grid_value( current_grid[ ii ], plot_y );
      }

      QwtPlotCurve *c1;
      QwtSymbol*   sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setBrush( QColor( Qt::yellow ) );
      sym1->setPen  ( QColor( Qt::yellow ) );
      sym1->setSize( 3 );

      c1 = us_curve( data_plot1, "Grid points 1" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), gridsize );
   }

qDebug() << "  up9)yRes" << yRes;
   data_plot1->setAxisAutoScale( QwtPlot::xBottom );
   data_plot1->setAxisAutoScale( QwtPlot::yLeft );
   data_plot1->replot();
}

// Calculate all grid points for the selected setting
void US_Grid_Editor::calc_gridpoints( void )
{
   struct gridpoint tmp_point;
qDebug() << "calc_g: px py pz" << plot_x << plot_y << plot_z;

qDebug() << "  cg1)yRes" << yRes;
   current_grid.clear();
   //bool flag = true;
   maxgridpoint.s    = -9.9e99;
   maxgridpoint.D    =  0.0;
   maxgridpoint.vbar =  0.0;
   maxgridpoint.mw   =  0.0;
   maxgridpoint.ff0  =  0.0;
   maxgridpoint.f0   =  0.0;
   maxgridpoint.f    =  0.0;
   mingridpoint.s    =  9.9e99;
   mingridpoint.D    =  9.9e99;
   mingridpoint.vbar =  9.9e99;
   mingridpoint.mw   =  9.9e99;
   mingridpoint.ff0  =  9.9e99;
   mingridpoint.f0   =  9.9e99;
   mingridpoint.f    =  9.9e99;

qDebug() << "  cg2)yRes" << yRes;
   xRes              = ct_xRes->value();
   xMin              = ct_xMin->value();
   xMax              = ct_xMax->value();
   yRes              = ct_yRes->value();
   yMin              = ct_yMin->value();
   yMax              = ct_yMax->value();
   zVal              = ct_zVal->value();
   vbar              = zVal;
qDebug() << "  cg3)yRes" << yRes;
   double xinc       = ( xMax - xMin ) / ( xRes - 1.0 );
   double yinc       = ( yMax - yMin ) / ( yRes - 1.0 );
   double xval       = xMin;
   double yval       = yMin;
   int    nxvals     = (int)xRes;
   int    nyvals     = (int)yRes;
   int    nerr_w     = 0;
   int    nerr_k     = 0;
   bool   comp_ok    = true;
qDebug() << "calc_g: zVal nx ny" << zVal << nxvals << nyvals
 << "xMin xMax xinc" << xMin << xMax << xinc
 << "yMin yMax yinc" << yMin << yMax << yinc;

   switch( plot_z )
   {
      case ATTR_S:
         tmp_point.s       = zVal;
         break;
      case ATTR_K:
         tmp_point.ff0     = zVal;
         break;
      case ATTR_W:
         tmp_point.mw      = zVal;
         break;
      case ATTR_V:
         tmp_point.vbar    = zVal;
         break;
      case ATTR_D:
         tmp_point.D       = zVal;
         break;
      case ATTR_F:
         tmp_point.f       = zVal;
         break;
   }
qDebug() << "  cg4)yRes" << yRes;
   if ( plot_x != ATTR_V  &&  plot_y != ATTR_V  &&  plot_z != ATTR_V )
   {
qDebug() << "calc_g:  CG2";
      calc_gridpoints_2();
   }
                                                       ///////////////
   else if ( plot_x == ATTR_S  &&  plot_y == ATTR_K )  // s and f_f0
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.s       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.ff0     = yval;
            yval             += yinc;

            if ( set_comp_skv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
qDebug() << "  cg5)yRes" << yRes;
   }
                                                       ///////////////
   else if ( plot_x == ATTR_S  &&  plot_y == ATTR_W )  // s and mw   
   {                                                   ///////////////
      int lstiek = -1;
      int lstjek = -1;
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.s       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.mw      = yval;
            yval             += yinc;

            if ( set_comp_swv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
            {
               nerr_k++;
               lstiek          = ii;
               lstjek          = jj;
qDebug() << "    ii jj" << ii << jj << "s w k" << tmp_point.s << tmp_point.mw
 << tmp_point.ff0;
            }
         }
      }
qDebug() << "  (0)ff0" << current_grid[0].ff0;
qDebug() << "  (n)ff0" << current_grid[nxvals*nyvals-1].ff0;
qDebug() << "   lstiek lstjek" << lstiek << lstjek << "nxy" << nxvals << nyvals;
   }
                                                       ///////////////
   else if ( plot_x == ATTR_S  &&  plot_y == ATTR_V )  // s and vbar   
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.s       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.vbar    = yval;
            yval             += yinc;

            if ( plot_z == ATTR_K )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_svf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_S  &&  plot_y == ATTR_D )  // s and D
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.s       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.D       = yval;
            yval             += yinc;

            if ( set_comp_svd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_S  &&  plot_y == ATTR_W )  // s and f
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.s       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.f       = yval;
            yval             += yinc;

            if ( set_comp_swv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_K  &&  plot_y == ATTR_S )  // ff0 and s
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.ff0     = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.s       = yval;
            yval             += yinc;

            if ( set_comp_skv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_K  &&  plot_y == ATTR_W )  // ff0 and mw
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.ff0     = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.mw      = yval;
            yval             += yinc;

            if ( set_comp_kwv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_K  &&  plot_y == ATTR_V )  // ff0 and vbar
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.ff0     = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.vbar    = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_kvf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_K  &&  plot_y == ATTR_D )  // ff0 and D
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.ff0     = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.D       = yval;
            yval             += yinc;

            if ( set_comp_kvd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_K  &&  plot_y == ATTR_F )  // ff0 and f
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.ff0     = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.f       = yval;
            yval             += yinc;

            if ( set_comp_kvf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_W  &&  plot_y == ATTR_S )  // mw and s
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.mw      = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.s       = yval;
            yval             += yinc;

            if ( set_comp_swv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_W  &&  plot_y == ATTR_K )  // mw and f/f0   
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.mw      = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.ff0     = yval;
            yval             += yinc;

            if ( set_comp_kwv( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_W  &&  plot_y == ATTR_V )  // mw and vbar
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.mw      = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.vbar    = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_wvf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_W  &&  plot_y == ATTR_D )  // mw and D
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.mw      = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.D       = yval;
            yval             += yinc;

            if ( set_comp_wvd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_W  &&  plot_y == ATTR_F )  // mw and f
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.mw      = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.f       = yval;
            yval             += yinc;

            if ( set_comp_wvf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_V  &&  plot_y == ATTR_S )  // vbar and s
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.vbar    = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.s       = yval;
            yval             += yinc;

            if ( plot_z == ATTR_K )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_svf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_V  &&  plot_y == ATTR_K )  // vbar and ff0
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.vbar    = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.ff0     = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_kvf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_V  &&  plot_y == ATTR_W )  // vbar and mw
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.vbar    = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.mw      = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_wvf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_V  &&  plot_y == ATTR_D )  // vbar and D
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.vbar    = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.D       = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_vdf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_V  &&  plot_y == ATTR_F )  // vbar and f
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.vbar    = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.f       = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_svf( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kvf( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_wvf( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_vdf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_D  &&  plot_y == ATTR_S )  // D and s
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.D       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.s       = yval;
            yval             += yinc;

            if ( set_comp_svd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_D  &&  plot_y == ATTR_K )  // D and ff0
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.D       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.ff0     = yval;
            yval             += yinc;

            if ( set_comp_kvd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_D  &&  plot_y == ATTR_W )  // D and mw
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.D       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.mw      = yval;
            yval             += yinc;

            if ( set_comp_wvd( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_D  &&  plot_y == ATTR_V )  // D and vbar
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.D       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.vbar    = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( plot_z == ATTR_F )
               comp_ok           = set_comp_vdf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_D  &&  plot_y == ATTR_F )  // D and f
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.D       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.f       = yval;
            yval             += yinc;

            if ( set_comp_vdf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_F  &&  plot_y == ATTR_S )  // f and s
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.f       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.s       = yval;
            yval             += yinc;

            if ( set_comp_svf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_F  &&  plot_y == ATTR_K )  // f and ff0
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.f       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.ff0     = yval;
            yval             += yinc;

            if ( set_comp_kvf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_F  &&  plot_y == ATTR_W )  // f and mw
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.f       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.mw      = yval;
            yval             += yinc;

            if ( set_comp_wvf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_F  &&  plot_y == ATTR_V )  // f and vbar
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.f       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.vbar    = yval;
            yval             += yinc;

            if ( plot_z == ATTR_S )
               comp_ok           = set_comp_svf( tmp_point );
            else if ( plot_z == ATTR_K )
               comp_ok           = set_comp_kvf( tmp_point );
            else if ( plot_z == ATTR_W )
               comp_ok           = set_comp_wvf( tmp_point );
            else if ( plot_z == ATTR_D )
               comp_ok           = set_comp_vdf( tmp_point );

            if ( comp_ok )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
                                                       ///////////////
   else if ( plot_x == ATTR_F  &&  plot_y == ATTR_D )  // f and D
   {                                                   ///////////////
      for ( int ii = 0; ii < nxvals; ii++ )
      {
         tmp_point.f       = xval;
         xval             += xinc;
         yval              = yMin;

         for ( int jj = 0; jj < nyvals; jj++ )
         {
            tmp_point.D       = yval;
            yval             += yinc;

            if ( set_comp_vdf( tmp_point ) )
            {
               current_grid << tmp_point;
               set_minmax( tmp_point );
            }
            else
               nerr_w++;

            if ( tmp_point.ff0 < 1.0 )
               nerr_k++;
         }
      }
   }
qDebug() << "calc_g:  xval yval" << xval << yval
 << "gsize" << current_grid.size();

   if ( nerr_w > 0  ||  nerr_k > 0  ||  current_grid.size() == 0 )
   {
      QString wmsg;

      if ( plot_x != ATTR_V  &&  plot_y != ATTR_V  &&  plot_z != ATTR_V )
      {
         wmsg = tr( "Presently, one of the axes or the fixed attribute "
                    "must be Partial Specific Volume (vbar). Please "
                    "select the Axis types and Fixed attribute so that "
                    "one of them is Partial Specific Volume." );
      }

      else if ( nerr_w > 0 )
      {
         wmsg = tr( "You have selected a nonsensical parameter setting. "
                    "The product of the sedimentation coefficient, s, "
                    "and the buoyancy term, (1 - vbar * Density), "
                    "must be positive. Please examine your settings for "
                    "the sedimentation coefficient range, the density, "
                    "and the partial specific volume before proceeding." );
      }

      else if ( nerr_k > 0 )
      {
         wmsg = tr( "You have selected a set of parameter settings that "
                    "result in f/f0 value(s) less than 1. "
                    "Please adjust the fixed value or ranges so that "
                    "the frictional ratio is at least 1." );
      }

      else
      {
         wmsg = tr( "Grid size is zero" );
      }

      QMessageBox::warning( this,
         tr( "Invalid Attributes/Parameters" ),
         wmsg );
   }
}

// Calculate all grid points for the selected setting (vbar unspecified)
void US_Grid_Editor::calc_gridpoints_2( void )
{
   current_grid.clear();
#if 0    // Currently, we simply do not handle cases with no vbar given
   //bool flag = true;
   maxgridpoint.s    = -9.9e99;
   maxgridpoint.D    =  0.0;
   maxgridpoint.vbar =  0.0;
   maxgridpoint.mw   =  0.0;
   maxgridpoint.ff0  =  0.0;
   maxgridpoint.f0   =  0.0;
   maxgridpoint.f    =  0.0;
   mingridpoint.s    =  9.9e99;
   mingridpoint.D    =  9.9e99;
   mingridpoint.vbar =  9.9e99;
   mingridpoint.mw   =  9.9e99;
   mingridpoint.ff0  =  9.9e99;
   mingridpoint.f0   =  9.9e99;
   mingridpoint.f    =  9.9e99;
   struct gridpoint tmp_point;
   double zVal       = ct_zVal->value();
   double xinc       = ( xMax - xMin ) / ( xRes - 1.0 );
   double yinc       = ( yMax - yMin ) / ( yRes - 1.0 );
   double xval       = xMin;
   double yval       = yMin;
   int    nxvals     = (int)xRes;
   int    nyvals     = (int)yRes;

   switch( plot_z )
   {
      case ATTR_S:
         tmp_point.s       = zVal;
         break;
      case ATTR_K:
         tmp_point.ff0     = zVal;
         break;
      case ATTR_W:
         tmp_point.mw      = zVal;
         break;
      case ATTR_V:
         tmp_point.vbar    = zVal;
         break;
      case ATTR_D:
         tmp_point.D       = zVal;
         break;
      case ATTR_F:
         tmp_point.f       = zVal;
         break;
   }
#endif

   return;
}

// add current grid to the list of grids
void US_Grid_Editor::add_partialGrid( void )
{
   grid_index++;
   for (int i=0; i<current_grid.size(); i++)
   {
      //check for overlaps first...
      current_grid[i].index = grid_index;
      final_grid.push_back(current_grid[i]);
   }
   pb_save->setEnabled( true );
   ck_show_final_grid->setEnabled( true );
   pb_delete_partialGrid->setEnabled( true );
   ct_partialGrid->setEnabled( true );

   int ntotg       = final_grid.size();
   int minsubg     = ntotg / MAXSSZ;
   int maxsubg     = ntotg / MINSSZ + 1;
   subGrids        = ntotg / DEFSSZ + 1;
   ct_subGrids->disconnect   ();
   ct_subGrids->setRange     ( minsubg, maxsubg );
   ct_subGrids->setSingleStep( 1 );
   ct_subGrids->setValue     ( subGrids );
   connect( ct_subGrids, SIGNAL( valueChanged   ( double ) ),
            this,        SLOT  ( update_subGrids( double ) ) );

   le_counts->setText( tr( "%1 total, %2 per subgrid" )
         .arg( ntotg ).arg( ntotg / subGrids ) );

}

// delete current grid
void US_Grid_Editor::delete_partialGrid( void )
{
   for (int i=final_grid.size() - 1; i>=0; i--)
   {
      if (final_grid[i].index == partialGrid)
      {
         final_grid.removeAt(i);
      }
   }
   // renumber index positions
   for (int i=0; i<final_grid.size(); i++)
   {
      if (final_grid[i].index > partialGrid)
      {
         final_grid[i].index--;
      }
   }
   grid_index--;
   ct_partialGrid->setRange     ( 1, grid_index );
   ct_partialGrid->setSingleStep( 1 );
   if (grid_index == 0)
   {
      ck_show_final_grid->setChecked (false);
      ck_show_final_grid->setEnabled (false);
      ct_partialGrid->setRange      ( 0, 0 );
      ct_partialGrid->setSingleStep ( 1 );
      ct_partialGrid->setEnabled( false );
      show_final_grid( false );
   }
   update_plot();
}

// find the minimum and maximum in a grid
void US_Grid_Editor::set_minmax( const struct gridpoint & tmp_point)
{
   mingridpoint.s    = qMin( mingridpoint.s,    tmp_point.s );
   mingridpoint.D    = qMin( mingridpoint.D,    tmp_point.D );
   mingridpoint.vbar = qMin( mingridpoint.vbar, tmp_point.vbar );
   mingridpoint.mw   = qMin( mingridpoint.mw,   tmp_point.mw );
   mingridpoint.ff0  = qMin( mingridpoint.ff0,  tmp_point.ff0 );
   mingridpoint.f0   = qMin( mingridpoint.f0,   tmp_point.f0 );
   mingridpoint.f    = qMin( mingridpoint.f,    tmp_point.f );
   maxgridpoint.s    = qMax( maxgridpoint.s,    tmp_point.s );
   maxgridpoint.D    = qMax( maxgridpoint.D,    tmp_point.D );
   maxgridpoint.vbar = qMax( maxgridpoint.vbar, tmp_point.vbar );
   maxgridpoint.mw   = qMax( maxgridpoint.mw,   tmp_point.mw );
   maxgridpoint.ff0  = qMax( maxgridpoint.ff0,  tmp_point.ff0 );
   maxgridpoint.f0   = qMax( maxgridpoint.f0,   tmp_point.f0 );
   maxgridpoint.f    = qMax( maxgridpoint.f,    tmp_point.f );
}

// find the minimum and maximum in a grid
void US_Grid_Editor::print_minmax( void )
{
   DbgLv(1) << "min s:"       << mingridpoint.s
            << "\nmin D:"     << mingridpoint.D
            << "\nmin vbar:"  << mingridpoint.vbar
            << "\nmin MW:"    << mingridpoint.mw
            << "\nmin f/f0:"  << mingridpoint.ff0
            << "\nmin f0:"    << mingridpoint.f0
            << "\nmin f:"     << mingridpoint.f
            << "\nmax s:"     << maxgridpoint.s
            << "\nmax D:"     << maxgridpoint.D
            << "\nmax vbar:"  << maxgridpoint.vbar
            << "\nmax mw:"    << maxgridpoint.mw
            << "\nmax f/f0:"  << maxgridpoint.ff0
            << "\nmax f0:"    << maxgridpoint.f0
            << "\nmax f:"     << maxgridpoint.f << "\n";
}

// Select plot1 (X-axis view) or plot2 (Molecular Weight view)
void US_Grid_Editor::select_plot( int ival )
{
   selected_plot   = ival;

   update_plot();
}

// Select coordinate for horizontal axis
void US_Grid_Editor::select_x_axis( int ival )
{
   // Axis types                   s    f/f0      mw   vbar     D     f
   const double  xvmns[] = {       1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
   const double  xvmxs[] = {      10.0,   4.0,   1e+5,  0.80, 1e-7, 1e-7 };
   const double  xmins[] = { -500000.0,   1.0,    0.0,  0.01, 1e-9, 1e-9 };
   const double  xmaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 3e-5, 1e-5 };
   const double  xincs[] = {      0.01,  0.01, 5000.0,  0.01, 1e-9, 1e-9 };
   const QString xtitls[] = { tr( "s (x 1e13)" ),
                              tr( "f/f0-value" ),
                              tr( "mw-value" ),
                              tr( "vbar-value" ),
                              tr( "D-value" ),
                              tr( "f-value" ) };
   plot_x = ival;

   lb_xRes->setText( xtitls[ plot_x ] + tr( " Resolution:" ) );
   lb_xMin->setText( xtitls[ plot_x ] + tr( " Minimum:" ) );
   lb_xMax->setText( xtitls[ plot_x ] + tr( " Maximum:" ) );
   ct_xMin->disconnect();
   ct_xMax->disconnect();
   xMin   = xvmns[ plot_x ];
   xMax   = xvmxs[ plot_x ];
   ct_xMin->setRange     ( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_xMin->setSingleStep( xincs[ plot_x ] );
   ct_xMax->setRange     ( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_xMax->setSingleStep( xincs[ plot_x ] );
   ct_xMin->setValue( xMin );
   ct_xMax->setValue( xMax );

   connect( ct_xMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMin ( double ) ) );
   connect( ct_xMax, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_xMax ( double ) ) );

   rb_y_s   ->setEnabled( plot_x != ATTR_S );
   rb_y_ff0 ->setEnabled( plot_x != ATTR_K );
   rb_y_mw  ->setEnabled( plot_x != ATTR_W );
   rb_y_vbar->setEnabled( plot_x != ATTR_V );
   rb_y_D   ->setEnabled( plot_x != ATTR_D );
   rb_y_f   ->setEnabled( plot_x != ATTR_F );
   rb_x_s   ->setEnabled( plot_y != ATTR_S );
   rb_x_ff0 ->setEnabled( plot_y != ATTR_K );
   rb_x_mw  ->setEnabled( plot_y != ATTR_W );
   rb_x_vbar->setEnabled( plot_y != ATTR_V );
   rb_x_D   ->setEnabled( plot_y != ATTR_D );
   rb_x_f   ->setEnabled( plot_y != ATTR_F );

   cb_fixed->disconnect();
   cb_fixed->clear();
   if ( plot_x != ATTR_V  &&  plot_y != ATTR_V )
      cb_fixed->addItem( tr( "Partial Specific Volume" ) );
   if ( plot_x != ATTR_K  &&  plot_y != ATTR_K )
      cb_fixed->addItem( tr( "Frictional Ratio" ) );
   if ( plot_x != ATTR_W  &&  plot_y != ATTR_W )
      cb_fixed->addItem( tr( "Molecular Weight" ) );
   if ( plot_x != ATTR_S  &&  plot_y != ATTR_S )
      cb_fixed->addItem( tr( "Sedimentation Coefficient" ) );
   if ( plot_x != ATTR_D  &&  plot_y != ATTR_D )
      cb_fixed->addItem( tr( "Diffusion Coefficient" ) );
   if ( plot_x != ATTR_F  &&  plot_y != ATTR_F )
      cb_fixed->addItem( tr( "Frictional Coefficient" ) );
   cb_fixed->setCurrentIndex( 0 );
   select_fixed( cb_fixed->currentText() );
   connect( cb_fixed, SIGNAL( activated   ( const QString& ) ),
            this,     SLOT  ( select_fixed( const QString& ) ) );

   validate_ff0();

   update_plot();
}

// select coordinate for vertical axis
void US_Grid_Editor::select_y_axis( int ival )
{
   // Axis types                   s    f/f0      mw   vbar     D     f
   const double  yvmns[] = {       1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
   const double  yvmxs[] = {      10.0,   4.0,   1e+6,  0.80, 1e-7, 1e-7 };
   const double  ymins[] = { -500000.0,   1.0,    0.0,  0.01, 1e-9, 1e-9 };
   const double  ymaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 3e-5, 1e-5 };
   const double  yincs[] = {      0.01,  0.01, 5000.0,  0.01, 1e-9, 1e-9 };
   const QString ytitls[] = { tr( "s (x 1e13)" ),
                              tr( "f/f0-value" ),
                              tr( "mw-value" ),
                              tr( "vbar-value" ),
                              tr( "D-value" ),
                              tr( "f-value" ) };
   plot_y = ival;

   lb_yRes->setText( ytitls[ plot_y ] + tr( " Resolution:" ) );
   lb_yMin->setText( ytitls[ plot_y ] + tr( " Minimum:" ) );
   lb_yMax->setText( ytitls[ plot_y ] + tr( " Maximum:" ) );
   ct_yMin->disconnect();
   ct_yMax->disconnect();
   yMin   = yvmns[ plot_y ];
   yMax   = yvmxs[ plot_y ];
   ct_yMin->setRange     ( ymins[ plot_y ], ymaxs[ plot_y ] );
   ct_yMin->setSingleStep( yincs[ plot_y ] );
   ct_yMax->setRange     ( ymins[ plot_y ], ymaxs[ plot_y ] );
   ct_yMax->setSingleStep( yincs[ plot_y ] );
   ct_yMin->setValue( yMin );
   ct_yMax->setValue( yMax );

   connect( ct_yMin, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_yMin ( double ) ) );
   connect( ct_yMax, SIGNAL( valueChanged( double ) ),
            this,    SLOT  ( update_yMax ( double ) ) );

   rb_x_s   ->setEnabled( plot_y != ATTR_S );
   rb_x_ff0 ->setEnabled( plot_y != ATTR_K );
   rb_x_mw  ->setEnabled( plot_y != ATTR_W );
   rb_x_vbar->setEnabled( plot_y != ATTR_V );
   rb_x_D   ->setEnabled( plot_y != ATTR_D );
   rb_x_f   ->setEnabled( plot_y != ATTR_F );
   rb_y_s   ->setEnabled( plot_x != ATTR_S );
   rb_y_ff0 ->setEnabled( plot_x != ATTR_K );
   rb_y_mw  ->setEnabled( plot_x != ATTR_W );
   rb_y_vbar->setEnabled( plot_x != ATTR_V );
   rb_y_D   ->setEnabled( plot_x != ATTR_D );
   rb_y_f   ->setEnabled( plot_x != ATTR_F );

   cb_fixed->disconnect();
   cb_fixed->clear();
   if ( plot_x != ATTR_V  &&  plot_y != ATTR_V )
      cb_fixed->addItem( tr( "Partial Specific Volume" ) );
   if ( plot_x != ATTR_K  &&  plot_y != ATTR_K )
      cb_fixed->addItem( tr( "Frictional Ratio" ) );
   if ( plot_x != ATTR_W  &&  plot_y != ATTR_W )
      cb_fixed->addItem( tr( "Molecular Weight" ) );
   if ( plot_x != ATTR_S  &&  plot_y != ATTR_S )
      cb_fixed->addItem( tr( "Sedimentation Coefficient" ) );
   if ( plot_x != ATTR_D  &&  plot_y != ATTR_D )
      cb_fixed->addItem( tr( "Diffusion Coefficient" ) );
   if ( plot_x != ATTR_F  &&  plot_y != ATTR_F )
      cb_fixed->addItem( tr( "Frictional Coefficient" ) );
   cb_fixed->setCurrentIndex( 0 );
   select_fixed( cb_fixed->currentText() );
   connect( cb_fixed, SIGNAL( activated   ( const QString& ) ),
            this,     SLOT  ( select_fixed( const QString& ) ) );

   validate_ff0();

   update_plot();
}

// Select coordinate flag for the fixed attribute
void US_Grid_Editor::select_fixed( const QString& fixstr )
{
   // Axis types                   s    f/f0      mw   vbar     D     f
   const double  zmins[] = { -500000.0,   1.0,    0.0,  0.01, 1e+6, 1e+6 };
   const double  zmaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 1e+8, 1e+6 };
   const double  zincs[] = {      0.01,  0.01, 5000.0, 0.001, 1e+5, 1e+5 };
   //const double  zvals[] = {     5.00,   2.0,   1e+5,  0.72, 1e+7, 1e+7 };

   plot_z   = fixstr.contains( tr( "Partial S" ) ) ? ATTR_V : 0;
   plot_z   = fixstr.contains( tr( "nal Ratio" ) ) ? ATTR_K : plot_z;
   plot_z   = fixstr.contains( tr( "ar Weight" ) ) ? ATTR_W : plot_z;
   plot_z   = fixstr.contains( tr( "Sedimenta" ) ) ? ATTR_S : plot_z;
   plot_z   = fixstr.contains( tr( "Diffusion" ) ) ? ATTR_D : plot_z;
   plot_z   = fixstr.contains( tr( "nal Coeff" ) ) ? ATTR_F : plot_z;
qDebug() << "SelFix: " << fixstr << "plot_z" << plot_z;

   ct_zVal->setRange     ( zmins[ plot_z ], zmaxs[ plot_z ] );
   ct_zVal->setSingleStep( zincs[ plot_z ] );
   //ct_zVal->setValue( zvals[ plot_z ] );
   lb_zVal->setText( fixstr );

   validate_ff0();
}

// activated when the "Show Final Grid" Checkbox is set
void US_Grid_Editor::show_final_grid( bool flag )
{
   if (flag)
   {
      ct_partialGrid       ->setRange     ( 1, grid_index );
      ct_partialGrid       ->setSingleStep( 1 );
      ct_partialGrid       ->setEnabled( true );
      pb_delete_partialGrid->setEnabled( true );
      pb_add_partialGrid   ->setEnabled( false );
      ck_show_sub_grid     ->setEnabled( true );
      ct_subGrids          ->setEnabled( false );
   }
   else
   {
      ct_partialGrid       ->setEnabled( false );
      pb_delete_partialGrid->setEnabled( false );
      pb_add_partialGrid   ->setEnabled( true );
      ck_show_sub_grid     ->setEnabled( false );
      ct_subGrids          ->setEnabled( false );
   }

   update_plot();
}

// activated when the "Show Subgrids" Checkbox is set
void US_Grid_Editor::show_sub_grid( bool flag )
{
   if ( flag )
   {
      int ntotg       = final_grid.size();
      int minsubg     = ntotg / MAXSSZ;
      int maxsubg     = ntotg / MINSSZ + 1;
      subGrids        = ntotg / DEFSSZ + 1;
DbgLv(1) << "finalsize" << ntotg << "maxsubg" << maxsubg;
      lb_partialGrid       ->setText   ( tr( "Highlight Subgrid #:" ) );
      ct_subGrids          ->setEnabled   ( true );
      ct_subGrids          ->setRange     ( minsubg, maxsubg );
      ct_subGrids          ->setSingleStep( 1 );
      ct_subGrids          ->setValue     ( subGrids );
      pb_delete_partialGrid->setEnabled   ( false );
      le_counts->setText( tr( "%1 total, %2 per subgrid" )
            .arg( ntotg ).arg( ntotg / subGrids ) );
   }
   else
   {
      lb_partialGrid       ->setText   ( tr( "Highlight Partial Grid #:" ) );
      ct_subGrids          ->setEnabled   ( false );
      ct_partialGrid       ->setRange     ( 1, grid_index );
      ct_partialGrid       ->setSingleStep( 1 );
      ct_partialGrid       ->setValue     ( 1 );
      pb_delete_partialGrid->setEnabled   ( true );
   }

   update_plot();
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_Grid_Editor::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Select DB investigator
void US_Grid_Editor::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();

   investigator = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investigator ) + ": "
                      +  US_Settings::us_inv_name();

   le_investigator->setText( inv_text );
}

// Get a grid value of the selected type
double US_Grid_Editor::grid_value( struct gridpoint& gpoint, int atype )
{
   double gvalue = ( atype == ATTR_S ) ? gpoint.s    : 0.0;
   gvalue        = ( atype == ATTR_K ) ? gpoint.ff0  : gvalue;
   gvalue        = ( atype == ATTR_W ) ? gpoint.mw   : gvalue;
   gvalue        = ( atype == ATTR_V ) ? gpoint.vbar : gvalue;
   gvalue        = ( atype == ATTR_D ) ? gpoint.D    : gvalue;
   gvalue        = ( atype == ATTR_F ) ? gpoint.f    : gvalue;

   return gvalue;
}

// Complete component given s+k+w (s, ff0, mw)
bool US_Grid_Editor::set_comp_skw( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}

// Complete component given s+k+v (s, ff0, vbar)
bool US_Grid_Editor::set_comp_skv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   double sval   = gpoint.s * 1.0e-13;

   gpoint.D      = R_GC * K20 / ( AVOGADRO * 18 * M_PI
                   * pow( ( viscosity * 0.01 * gpoint.ff0 ), (3.0/2.0) )
                   * pow( ( sval * vbar / ( 2.0 * buoy ) ), 0.5 ) );

   bool   is_ok  = check_grid_point( buoy, gpoint );

   if ( is_ok )
   {
      gpoint.mw     = sval * R_GC * K20 / ( gpoint.D * buoy );
      gpoint.f      = R_GC * K20 / ( AVOGADRO * gpoint.D );
      gpoint.f0     = gpoint.f / gpoint.ff0;
   }

   return is_ok;
}

// Complete component given s+k+d (s, ff0, D)
bool US_Grid_Editor::set_comp_skd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+k+f (s, ff0, f)
bool US_Grid_Editor::set_comp_skf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+w+v (s, mw, vbar)
bool US_Grid_Editor::set_comp_swv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   double sval   = gpoint.s * 1.0e-13;
   double ssgn   = ( gpoint.s < 0.0 ) ? -1.0 : 1.0;
   gpoint.D      = ssgn * sval * R_GC * K20 / ( buoy * gpoint.mw );
   gpoint.f      = ssgn * gpoint.mw * buoy / ( sval * AVOGADRO );
   double volume = vbar * gpoint.mw / AVOGADRO;
   double sphere = pow( volume * VOL_FAC, THIRD );
   gpoint.f0     = sphere * SPH_FAC;
   gpoint.f0     = ( gpoint.f0 == 0.0 ) ? 1.0 : gpoint.f0;
   gpoint.ff0    = gpoint.f / gpoint.f0;
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;
   bool   is_ok  = check_grid_point( buoy, gpoint );
//qDebug() << "comp_swv s w v k" << gpoint.s << gpoint.mw << gpoint.vbar
// << gpoint.ff0;

   return is_ok;
}
// Complete component given s+w+d (s, mw, D)
bool US_Grid_Editor::set_comp_swd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   bool   is_ok  = check_grid_point( buoy, gpoint );
qDebug() << "comp_swd buoy" << buoy;

   return is_ok;
}
// Complete component given s+w+f (s, mw, f)
bool US_Grid_Editor::set_comp_swf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_swf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+v+d (s, vbar, D)
bool US_Grid_Editor::set_comp_svd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   double ssgn   = ( gpoint.s < 0.0 ) ? -1.0 : 1.0;
   double sval   = gpoint.s * 1.0e-13;
   gpoint.f      = R_GC * K20 / ( AVOGADRO * gpoint.D );
   gpoint.mw     = ssgn * sval * R_GC * K20 / ( gpoint.D * buoy );
   double volume = vbar * gpoint.mw / AVOGADRO;
   double sphere = pow( volume * VOL_FAC, THIRD );
   gpoint.ff0    = gpoint.f / ( sphere * SPH_FAC );
   gpoint.f0     = gpoint.f / qMax( 1.0, gpoint.ff0 );
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+v+f (s, vbar, f)
bool US_Grid_Editor::set_comp_svf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   double ssgn   = ( gpoint.s < 0.0 ) ? -1.0 : 1.0;
   double sval   = gpoint.s * 1.0e-13;
   gpoint.D      = R_GC * K20 / ( AVOGADRO * gpoint.f );
   gpoint.mw     = ssgn * sval * R_GC * K20 / ( gpoint.D * buoy );
   double volume = vbar * gpoint.mw / AVOGADRO;
   double sphere = pow( volume * VOL_FAC, THIRD );
   gpoint.ff0    = gpoint.f / ( sphere * SPH_FAC );
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;
   gpoint.f0     = gpoint.f / qMax( 1.0, gpoint.ff0 );
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+d+f (s, D, f)
bool US_Grid_Editor::set_comp_sdf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_sdf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;

   return is_ok;
}
// Complete component given k+w+v (ff0, mw, vbar)
bool US_Grid_Editor::set_comp_kwv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
   double vbrat  = vbar / AVOGADRO;
   double viscf  = viscosity * 0.01;

   gpoint.f0     = viscf * pow( ( 162.0 * gpoint.mw * MPISQ * vbrat ), THIRD );
   gpoint.f      = gpoint.ff0 * gpoint.f0;
   gpoint.s      = 1.0e13 * gpoint.mw * buoy / ( AVOGADRO * gpoint.f );
   gpoint.D      = R_GC * K20 / ( AVOGADRO * gpoint.f );
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+w+d (ff0, mw, D)
bool US_Grid_Editor::set_comp_kwd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_kwd buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+w+f (ff0, mw, f)
bool US_Grid_Editor::set_comp_kwf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_kwf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+v+d (ff0, vbar, D)
bool US_Grid_Editor::set_comp_kvd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_kvd buoy" << buoy;
   gpoint.f      = R_GC * K20 / ( AVOGADRO * gpoint.D );
   gpoint.f0     = gpoint.f / qMax( 1.0, gpoint.ff0 );
   double sphere = gpoint.f0 / ( 0.06 * M_PI * VISC_20W );
   double volume = ( 4.0 / 3.0 ) * M_PI * pow( sphere, 3.0 );
   gpoint.mw     = volume * AVOGADRO / vbar;
   double sval   = gpoint.mw * buoy / ( AVOGADRO * gpoint.f );
   gpoint.s      = sval * 1.0e+13;
   gpoint.ff0    = ( gpoint.f0 != 0.0 ) ? gpoint.f / gpoint.f0 : 1.0;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+v+f (ff0, vbar, f)
bool US_Grid_Editor::set_comp_kvf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_kvf buoy" << buoy;
   gpoint.f0     = gpoint.f / qMax( 1.0, gpoint.ff0 );
   gpoint.D      = R_GC * K20 / ( AVOGADRO * gpoint.f );
   double sphere = gpoint.f0 / ( 0.06 * M_PI * VISC_20W );
   double volume = ( 4.0 / 3.0 ) * M_PI * pow( sphere, 3.0 );
   gpoint.mw     = volume * AVOGADRO / vbar;
   double sval   = gpoint.mw * buoy / ( AVOGADRO * gpoint.f );
   gpoint.s      = sval * 1.0e+13;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+d+f (ff0, D, f)
bool US_Grid_Editor::set_comp_kdf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_kdf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given w+v+d (mw, vbar, D)
bool US_Grid_Editor::set_comp_wvd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_wvd buoy" << buoy;
   double volume = vbar * gpoint.mw / AVOGADRO;
   double sphere = pow( volume * VOL_FAC, THIRD );
   gpoint.f0     = sphere * SPH_FAC;
   double sval   = gpoint.D * buoy * gpoint.mw / ( R_GC * K20 );
   gpoint.f      = gpoint.mw * buoy / ( sval * AVOGADRO );
   gpoint.ff0    = ( gpoint.f0 != 0.0 ) ? gpoint.f / gpoint.f0 : 1.0;
   gpoint.s      = sval * 1.0e+13;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given w+v+f (mw, vbar, f)
bool US_Grid_Editor::set_comp_wvf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_wvf buoy" << buoy;
   double volume = vbar * gpoint.mw / AVOGADRO;
   double sphere = pow( volume * VOL_FAC, THIRD );
   gpoint.f0     = sphere * SPH_FAC;
   gpoint.ff0    = ( gpoint.f0 != 0.0 ) ? gpoint.f / gpoint.f0 : 1.0;
   double sval   = gpoint.mw * buoy / ( AVOGADRO * gpoint.f );
   gpoint.D      = sval * R_GC * K20 / ( buoy * gpoint.mw );
   gpoint.s      = sval * 1.0e+13;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given w+d+f (mw, D, f)
bool US_Grid_Editor::set_comp_wdf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_wdf buoy" << buoy;
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given v+d+f (vbar, D, f)
bool US_Grid_Editor::set_comp_vdf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * density;
qDebug() << "comp_vdf buoy" << buoy;
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}

// Check the validity of a grid point
bool US_Grid_Editor::check_grid_point( double buoy, struct gridpoint& gpoint )
{
   bool is_ok = true;

   if ( buoy == 0.0  ||  gpoint.vbar == 0.0  ||
        ( gpoint.s < 0  &&  buoy > 0.0 )  ||
        ( gpoint.s > 0  &&  buoy < 0.0 ) )
   {  // If point is nonsensical, flag it and mark molecular weight negative
      gpoint.mw  = -1.0;
      is_ok      = false;
   }

   return is_ok;
}

// Adjust value/ranges so that f/f0 is not less than 1
bool US_Grid_Editor::validate_ff0()
{
   bool is_ok     = true;

   if ( plot_x == ATTR_K  ||  plot_y == ATTR_K  ||  plot_z == ATTR_K )
   {  // If one of the attributes is f/f0, no need for checking
      return is_ok;
   }

   struct gridpoint zpoint;
   struct gridpoint tmp_point;
   xMin           = ct_xMin->value();
   xMax           = ct_xMax->value();
   yMin           = ct_yMin->value();
   yMax           = ct_yMax->value();
qDebug() << "valFF0: xMin xMax yMin yMax" << xMin << xMax << yMin << yMax;
   zVal           = ct_zVal->value();
   clear_grid( zpoint );
   set_grid_value( zpoint,    plot_z, zVal );
   // Get f/f0 for xMin,yMin
   tmp_point      = zpoint;
   set_grid_value( tmp_point, plot_x, xMin );
   set_grid_value( tmp_point, plot_y, yMin );
   complete_comp ( tmp_point );
   double ffx1y1  = tmp_point.ff0;
   // Get f/f0 for xMin,yMax
   tmp_point      = zpoint;
   set_grid_value( tmp_point, plot_x, xMin );
   set_grid_value( tmp_point, plot_y, yMax );
   complete_comp ( tmp_point );
   double ffx1y2  = tmp_point.ff0;
   // Get f/f0 for xMax,yMin
   tmp_point      = zpoint;
   set_grid_value( tmp_point, plot_x, xMax );
   set_grid_value( tmp_point, plot_y, yMin );
   complete_comp ( tmp_point );
   double ffx2y1  = tmp_point.ff0;
   // Get f/f0 for xMax,yMax
   tmp_point      = zpoint;
   set_grid_value( tmp_point, plot_x, xMax );
   set_grid_value( tmp_point, plot_y, yMax );
   complete_comp ( tmp_point );
   double ffx2y2  = tmp_point.ff0;
   // Get overall minimum f/f0
   double ff0min  = qMin( ffx1y1, ffx1y2 );
   ff0min         = qMin( ff0min, ffx2y1 );
   ff0min         = qMin( ff0min, ffx2y2 );
qDebug() << "valFF0: zVal xMin yMin ff0" << zVal << xMin << yMin << ffx1y1;
qDebug() << "valFF0:      xMin yMax ff0   " << xMin << yMax << ffx1y2;
qDebug() << "valFF0:      xMax yMin ff0   " << xMax << yMin << ffx2y1;
qDebug() << "valFF0:      xMax yMax ff0   " << xMax << yMax << ffx2y2;

   if ( ff0min < 1.0 )
   {  // Ranges include values that set f/f0 less than 1:  must adjust ranges

      if ( plot_x == ATTR_W  ||  plot_x == ATTR_F )
      {  // Adjust the X range (if MW or f)
         ct_xMin->disconnect();
         ct_xMax->disconnect();
         tmp_point      = zpoint;
         set_grid_value( tmp_point, ATTR_K, 1.0  );

         if ( ffx1y1 < ffx1y2 )
         {  // Increasing Y means increasing f/f0, so get X for k=1,ymin
            set_grid_value( tmp_point, plot_y, yMin );
            complete_comp ( tmp_point );
            double xVal    = grid_value( tmp_point, plot_x );

            if ( ffx1y1 < ffx2y1 )
            {  // Increasing X means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (1xMin)xVal" << xVal;
               ct_xMin->setMinimum( xVal );
               ct_xMax->setMinimum( xVal );
            }

            else
            {  // Increasing X means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (2xMin)xVal" << xVal;
               ct_xMin->setMaximum( xVal );
               ct_xMax->setMaximum( xVal );
            }
         }

         else
         {  // Increasing Y means decreasing f/f0, so get X for k=1,ymax
            set_grid_value( tmp_point, plot_y, yMax );
            complete_comp ( tmp_point );
            double xVal    = grid_value( tmp_point, plot_x );

            if ( ffx1y1 < ffx2y1 )
            {  // Increasing X means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (3xMin)xVal" << xVal;
               ct_xMin->setMinimum( xVal );
               ct_xMax->setMinimum( xVal );
            }

            else
            {  // Increasing X means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (4xMin)xVal" << xVal;
               ct_xMin->setMaximum( xVal );
               ct_xMax->setMaximum( xVal );
            }
         }

         connect( ct_xMin, SIGNAL( valueChanged( double ) ),
                  this,    SLOT  ( update_xMin ( double ) ) );
         connect( ct_xMax, SIGNAL( valueChanged( double ) ),
                  this,    SLOT  ( update_xMax ( double ) ) );
      }

      if ( plot_y == ATTR_W  ||  plot_y == ATTR_F )
      {  // Adjust the Y range (if MW or f)
         ct_yMin->disconnect();
         ct_yMax->disconnect();
         tmp_point      = zpoint;
         set_grid_value( tmp_point, ATTR_K, 1.0  );

         if ( ffx1y1 < ffx2y1 )
         {  // Increasing X means increasing f/f0, so get Y for k=1,xmin
            set_grid_value( tmp_point, plot_x, xMin );
            complete_comp ( tmp_point );
            double yVal    = grid_value( tmp_point, plot_y );

            if ( ffx1y1 < ffx1y2 )
            {  // Increasing Y means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (5yMin)yVal" << yVal;
               ct_yMin->setMinimum( yVal );
               ct_yMax->setMinimum( yVal );
            }

            else
            {  // Increasing Y means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (6yMax)yVal" << yVal;
               ct_yMin->setMaximum( yVal );
               ct_yMax->setMaximum( yVal );
            }
         }

         else
         {  // Increasing X means decreasing f/f0, so get y for k=1,xmax
            set_grid_value( tmp_point, plot_x, xMax );
            complete_comp ( tmp_point );
            double yVal    = grid_value( tmp_point, plot_y );

            if ( ffx1y1 < ffx1y2 )
            {  // Increasing Y means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (7yMin)yVal" << yVal;
               ct_yMin->setMinimum( yVal );
               ct_yMax->setMinimum( yVal );
            }

            else
            {  // Increasing Y means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (8yMax)yVal" << yVal;
               ct_yMin->setMaximum( yVal );
               ct_yMax->setMaximum( yVal );
            }
         }

         connect( ct_yMin, SIGNAL( valueChanged( double ) ),
                  this,    SLOT  ( update_yMin ( double ) ) );
         connect( ct_yMax, SIGNAL( valueChanged( double ) ),
                  this,    SLOT  ( update_yMax ( double ) ) );
      }
   }

   xMin           = ct_xMin->value();
   xMax           = ct_xMax->value();
   yMin           = ct_yMin->value();
   yMax           = ct_yMax->value();
qDebug() << "valFF0: (out)xMin xMax yMin yMax" << xMin << xMax << yMin << yMax;

   return is_ok;
}

// Clear a gridpoint structure
void US_Grid_Editor::clear_grid( struct gridpoint& gpoint )
{
   gpoint.s      = 0.0;
   gpoint.ff0    = 0.0;
   gpoint.mw     = 0.0;
   gpoint.vbar   = 0.0;
   gpoint.D      = 0.0;
   gpoint.f      = 0.0;
}

// Set a gridpoint value of the selected type
void US_Grid_Editor::set_grid_value( struct gridpoint& gpoint, int atype,
                                     double gvalue )
{
   if      ( atype == ATTR_S )
      gpoint.s      = gvalue;
   else if ( atype == ATTR_K )
      gpoint.ff0    = gvalue;
   else if ( atype == ATTR_W )
      gpoint.mw     = gvalue;
   else if ( atype == ATTR_V )
      gpoint.vbar   = gvalue;
   else if ( atype == ATTR_D )
      gpoint.D      = gvalue;
   else if ( atype == ATTR_F )
      gpoint.f      = gvalue;
}

// Complete component where 3 attributes are given
bool US_Grid_Editor::complete_comp( struct gridpoint& gpoint )
{
   US_Model::SimulationComponent sc;
   sc.s          = gpoint.s * 1.0e-13;
   sc.f_f0       = gpoint.ff0;
   sc.mw         = gpoint.mw;
   sc.vbar20     = gpoint.vbar;
   sc.D          = gpoint.D;
   sc.f          = gpoint.f;

   bool is_ok    = US_Model::calc_coefficients( sc );

   if ( is_ok )
   {
      gpoint.s      = sc.s * 1.0e+13;
      gpoint.ff0    = sc.f_f0;
      gpoint.mw     = sc.mw;
      gpoint.vbar   = sc.vbar20;
      gpoint.D      = sc.D;
      gpoint.f      = sc.f;
   }

   return is_ok;
}

