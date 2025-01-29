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
   setWindowTitle( tr( "UltraScan 2DSA Grid Initialization Editor" ) );
   setPalette( US_GuiSettings::frameColor() );

   // validators
   QDoubleValidator *d_valid = new QDoubleValidator(this);
   QIntValidator    *i_valid = new QIntValidator(this);
   i_valid->setBottom(1);

   // primary layouts
   QHBoxLayout* main  = new QHBoxLayout( this );
   QVBoxLayout* right = new QVBoxLayout();
   QGridLayout* left  = new QGridLayout();
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing        ( 2 );
   left->setContentsMargins( 0, 0, 0, 0 );
   right->setSpacing        ( 0 );
   right->setContentsMargins( 0, 1, 0, 1 );

   dbg_level = US_Settings::us_debug();

   QLabel *lb_preset = us_banner( tr( "Grid Preset" ) );

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, &QPushButton::clicked, this, &US_Grid_Editor::sel_investigator );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );

   dkdb_cntrls   = new US_Disk_DB_Controls( US_Settings::default_data_location() );
   connect( dkdb_cntrls, &US_Disk_DB_Controls::changed, this, &US_Grid_Editor::update_disk_db );

   grid_preset = new US_Grid_Preset(this);
   grid_preset->parameters(&x_param, &y_param, &z_param);

   QLabel *lb_x_param = us_label( tr( "X-Axis" ) );
   lb_x_param->setAlignment( Qt::AlignCenter );

   QLabel *lb_y_param = us_label( tr( "Y-Axis" ) );
   lb_y_param->setAlignment( Qt::AlignCenter );

   QLabel *lb_z_param = us_label( tr( "Fixed Attribute" ) );
   lb_z_param->setAlignment( Qt::AlignCenter );

   le_x_param = us_lineedit( Attr_to_long(x_param), -1, true);
   le_y_param = us_lineedit( Attr_to_long(y_param), -1, true);
   le_z_param = us_lineedit( Attr_to_long(z_param), -1, true);

   QPushButton *pb_preset = us_pushbutton( "Setup Griding" );
   connect ( pb_preset, &QPushButton::clicked, this, &US_Grid_Editor::set_grid_axis);

   // Experimental Space

   QLabel *lb_experm = us_banner( tr( "Experimental Space" ) );
   QLabel *lb_dens = us_label( tr( "ρ at 20°C [g/mL]" ) );
   lb_dens->setAlignment( Qt::AlignCenter );
   le_dens = us_lineedit( QString::number( DENS_20W ) );
   le_dens->setValidator(d_valid);

   QLabel *lb_visc = us_label( tr( "η at 20°C [cP]" ) );
   lb_visc->setAlignment( Qt::AlignCenter );
   le_visc = us_lineedit( QString::number( VISC_20W ) );
   le_visc->setValidator(d_valid);

   QLabel *lb_temp = us_label( tr( "T [°C]" ) );
   lb_temp->setAlignment( Qt::AlignCenter );
   le_temp = us_lineedit( QString::number( 20 ) );
   le_temp->setValidator(d_valid);

   QPushButton* pb_set_exp_data = us_pushbutton("Update");

   // 20,w grid control
   QLabel *lb_20w_ctrl = us_banner( tr( "20,w Grid Control" ) );

   QGridLayout* toggle1 = us_radiobutton( tr( "X-Axis View" ), rb_plot1, true );
   QGridLayout* toggle2 = us_radiobutton( tr( "Molecular Weight View" ), rb_plot2, true );

   QButtonGroup* toggle_plot = new QButtonGroup( this );
   toggle_plot->addButton( rb_plot1, 0 );
   toggle_plot->addButton( rb_plot2, 1 );
   rb_plot1   ->setChecked( true );
   connect( toggle_plot, &QButtonGroup::idReleased, this, &US_Grid_Editor::select_plot );

   QLabel *lb_grid_list = us_label(" Grid List ");
   lb_grid_list->setAlignment( Qt::AlignCenter );

   lw_grids = us_listwidget();

   QPushButton* pb_make_new = us_pushbutton( "New Grid" );
   pb_delete = us_pushbutton( "Delete Grid" );

   QFrame *hline1 = new QFrame();
   hline1->setFrameShape(QFrame::HLine);
   hline1->setFrameShadow(QFrame::Sunken);

   QLabel *lb_min = us_label( "Minimum" );
   lb_min->setAlignment( Qt::AlignCenter );
   QLabel *lb_max = us_label( "Maximum" );
   lb_max->setAlignment( Qt::AlignCenter );
   QLabel *lb_res = us_label( "Resolution" );
   lb_res->setAlignment( Qt::AlignCenter );

   lb_x_ax = us_label( Attr_to_short( x_param ));
   lb_x_ax->setAlignment( Qt::AlignCenter );
   lb_y_ax = us_label( Attr_to_short( y_param ));
   lb_y_ax->setAlignment( Qt::AlignCenter );

   le_x_min = us_lineedit();
   le_x_min->setValidator(d_valid);
   le_x_max = us_lineedit();
   le_x_max->setValidator(d_valid);
   le_x_res = us_lineedit();
   le_x_res->setValidator(i_valid);

   le_y_min = us_lineedit();
   le_y_min->setValidator(d_valid);
   le_y_max = us_lineedit();
   le_y_max->setValidator(d_valid);
   le_y_res = us_lineedit();
   le_y_res->setValidator(i_valid);

   QFrame *hline2 = new QFrame();
   hline2->setFrameShape(QFrame::HLine);
   hline2->setFrameShadow(QFrame::Sunken);

   lb_z_ax = us_label( Attr_to_short( z_param ));
   lb_z_ax->setAlignment( Qt::AlignCenter );
   le_z_val = us_lineedit();
   le_z_val->setValidator(d_valid);

   QPushButton* pb_validate = us_pushbutton( "Validate" );
   connect( pb_validate, &QPushButton::clicked, this, &US_Grid_Editor::call_validate );

   QLabel *lb_subgrids = us_label( "# Subgrids" );
   lb_subgrids->setAlignment( Qt::AlignCenter );
   le_subgrids = us_lineedit();
   le_subgrids->setValidator(i_valid);

   QLabel *lb_allgrids = us_label( "# Grid Points" );
   lb_allgrids->setAlignment( Qt::AlignCenter );
   le_allgrids = us_lineedit("", -1, true);

   pb_add_update = us_pushbutton( "Add/Update" );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset, &QPushButton::clicked, this, &US_Grid_Editor::reset );

   QPushButton* pb_save = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   connect( pb_save, &QPushButton::clicked, this, &US_Grid_Editor::save );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   connect( pb_help, &QPushButton::clicked, this, &US_Grid_Editor::help );

   QPushButton* pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close, &QPushButton::clicked, this, &US_Grid_Editor::close );

   // set up plot component window on right side

   QBoxLayout* plot1 = new US_Plot( data_plot,
                                   tr( "Grid Layout" ),
                                   tr( "Sedimentation Coefficient (s20,W)"),
                                   tr( "Frictional Ratio f/f0" ) );
   data_plot->setAutoDelete( true );
   data_plot->setMinimumSize( 640, 480 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   int row = 0;
   left->addWidget( lb_preset,            row++, 0, 1, 4 );

   left->addWidget( pb_investigator,      row,   0, 1, 2 );
   left->addWidget( le_investigator,      row++, 2, 1, 2 );

   left->addLayout( dkdb_cntrls,          row++, 0, 1, 4 );

   left->addWidget( lb_x_param,           row,   0, 1, 2 );
   left->addWidget( le_x_param,           row++, 2, 1, 2 );

   left->addWidget( lb_y_param,           row,   0, 1, 2 );
   left->addWidget( le_y_param,           row++, 2, 1, 2 );

   left->addWidget( lb_z_param,           row,   0, 1, 2 );
   left->addWidget( le_z_param,           row++, 2, 1, 2 );

   left->addWidget( pb_preset,            row++, 1, 1, 2 );

   left->addWidget( lb_experm,            row++, 0, 1, 4 );

   left->addWidget( lb_dens,              row,   0, 1, 1 );
   left->addWidget( le_dens,              row,   1, 1, 1 );
   left->addWidget( lb_visc,              row,   2, 1, 1 );
   left->addWidget( le_visc,              row++, 3, 1, 1 );

   left->addWidget( lb_temp,              row,   0, 1, 1 );
   left->addWidget( le_temp,              row,   1, 1, 1 );
   left->addWidget( pb_set_exp_data,      row++, 2, 1, 2 );

   left->addWidget( lb_20w_ctrl,          row++, 0, 1, 4 );

   left->addLayout( toggle1,              row,   0, 1, 2 );
   left->addLayout( toggle2,              row++, 2, 1, 2 );

   left->addWidget( lb_grid_list,         row++, 0, 1, 4 );

   left->addWidget( lw_grids,             row,   0, 5, 4 );
   row += 5;

   left->addWidget( pb_delete,            row,   0, 1, 2 );
   left->addWidget( pb_make_new,          row++, 2, 1, 2 );

   left->addWidget( hline1,               row++, 0, 1, 4 );

   left->addWidget( lb_min,               row,   1, 1, 1 );
   left->addWidget( lb_max,               row,   2, 1, 1 );
   left->addWidget( lb_res,               row++, 3, 1, 1 );

   left->addWidget( lb_x_ax,              row,   0, 1, 1 );
   left->addWidget( le_x_min,             row,   1, 1, 1 );
   left->addWidget( le_x_max,             row,   2, 1, 1 );
   left->addWidget( le_x_res,             row++, 3, 1, 1 );

   left->addWidget( lb_y_ax,              row,   0, 1, 1 );
   left->addWidget( le_y_min,             row,   1, 1, 1 );
   left->addWidget( le_y_max,             row,   2, 1, 1 );
   left->addWidget( le_y_res,             row++, 3, 1, 1 );

   left->addWidget( hline2,               row++, 0, 1, 4 );

   left->addWidget( lb_z_ax,              row,   0, 1, 1 );
   left->addWidget( le_z_val,             row,   1, 1, 1 );
   left->addWidget( pb_validate,          row++, 2, 1, 2 );

   left->addWidget( lb_subgrids,          row,   0, 1, 1 );
   left->addWidget( le_subgrids,          row,   1, 1, 1 );
   left->addWidget( pb_add_update,        row++, 2, 1, 2 );

   left->addWidget( lb_allgrids,          row,   0, 1, 1 );
   left->addWidget( le_allgrids,          row++, 1, 1, 3 );

   QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   left->addItem(spacer, row++, 0, 1, 4);

   left->addWidget( pb_reset,             row,   0, 1, 1 );
   left->addWidget( pb_save,              row,   1, 1, 1 );
   left->addWidget( pb_help,              row,   2, 1, 1 );
   left->addWidget( pb_close,             row++, 3, 1, 1 );

   for (int ii = 0; ii < 4; ii++) {
      left->setColumnStretch(ii, 1);
   }

   right->addLayout( plot1 );

   main->addLayout( left );
   main->addLayout( right );
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( right, 6 );

   reset();
}

void US_Grid_Editor::set_grid_axis()
{
   int state = grid_preset->exec();
   if ( state == QDialog::Accepted ) {
      grid_preset->parameters(&x_param, &y_param, &z_param);
      le_x_param->setText( Attr_to_long(x_param) );
      le_y_param->setText( Attr_to_long(y_param) );
      le_z_param->setText( Attr_to_long(z_param) );
      lb_x_ax->setText( Attr_to_short( x_param ));
      lb_y_ax->setText( Attr_to_short( y_param ));
      lb_z_ax->setText( Attr_to_short( z_param ));
      le_x_min->clear();
      le_x_max->clear();
      le_y_min->clear();
      le_y_max->clear();
      le_z_val->clear();
      if ( z_param == -1 ) {
         lb_z_ax->hide();
         le_z_val->hide();
      } else {
         lb_z_ax->show();
         le_z_val->show();
      }
   }
}

void US_Grid_Editor::call_validate()
{
   gridpoint grid_point;
   if ( validate(grid_point)) {
      qDebug() << "";
   }

}

bool US_Grid_Editor::validate( gridpoint& grid_point)
{
   double x_min = le_x_min->text().toDouble();
   double x_max = le_x_min->text().toDouble();
   double y_min = le_y_min->text().toDouble();
   double y_max = le_y_min->text().toDouble();
   double z_val = -1;
   if ( z_param != -1 ) {
      z_val = le_z_val->text().toDouble();
   }


}

// reset the GUI
void US_Grid_Editor::reset( void )
{
   dataPlotClear( data_plot );
   picker = new US_PlotPicker( data_plot );

   x_param = ATTR_S; // plot s
   y_param = ATTR_K; // plot f/f0
   z_param = ATTR_V; // fixed vbar
   // selected_plot = 0;
   grid_index    = 0;
   partialGrid   = 0;
   subGrids      = 13;
   final_grid.clear();

   le_x_param->setText( Attr_to_long(x_param) );
   le_y_param->setText( Attr_to_long(y_param) );
   le_z_param->setText( Attr_to_long(z_param) );
   lb_x_ax->setText( Attr_to_short( x_param ));
   lb_y_ax->setText( Attr_to_short( y_param ));
   lb_z_ax->setText( Attr_to_short( z_param ));
   le_x_min->clear();
   le_x_max->clear();
   le_x_res->setText(QString::number(64));
   le_y_min->clear();
   le_y_max->clear();
   le_y_res->setText(QString::number(64));
   le_z_val->clear();
   lb_z_ax->show();
   le_z_val->show();
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

      double xval = grid_value( final_grid[ ii ], x_param );
      double yval = grid_value( final_grid[ ii ], y_param );
      double zval = grid_value( final_grid[ ii ], z_param );
      int indexx  = xvals.indexOf( xval ) + 1;
      int indexy  = yvals.indexOf( yval ) + 1;
      int indexz  = zvals.indexOf( zval ) + 1;
      if ( indexx < 1 )  { indexx = xvals.size() + 1; xvals << xval; }
      if ( indexy < 1 )  { indexy = yvals.size() + 1; yvals << yval; }
      if ( indexz < 1 )  { indexz = zvals.size() + 1; zvals << zval; }
      sc.name     = QString::asprintf( "X%3.3dY%3.3dZ%2.2d",
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
// qDebug() << "ux1)yRes" << yRes;
//    xRes  = dval;
// qDebug() << "ux2)yRes" << yRes;
//    update_plot();
// qDebug() << "ux3)yRes" << yRes;
}

// update raster y resolution
void US_Grid_Editor::update_yRes( double dval )
{
   // yRes  = dval;
   // update_plot();
}

// update plot limit x min
void US_Grid_Editor::update_xMin( double dval )
{
   // xMin    = dval;
   // // ct_xMax->disconnect();
   // // ct_xMax->setMinimum( xMin );

   // // connect( ct_xMax, SIGNAL( valueChanged( double ) ),
   // //          this,    SLOT  ( update_xMax ( double ) ) );

   // validate_ff0();

   // update_plot();
}

// update plot limit x max
void US_Grid_Editor::update_xMax( double dval )
{
   // xMax    = dval;
   // // ct_xMin->disconnect();
   // // ct_xMin->setMaximum( xMax );

   // // connect( ct_xMin, SIGNAL( valueChanged( double ) ),
   // //          this,    SLOT  ( update_xMin ( double ) ) );

   // validate_ff0();

   // update_plot();
}

// update plot limit y min
void US_Grid_Editor::update_yMin( double dval )
{
//    yMin    = dval;
// qDebug() << "update_yMin" << yMin;
//    // ct_yMax->disconnect();
//    // ct_yMax->setMinimum( yMin );

//    // connect( ct_yMax, SIGNAL( valueChanged( double ) ),
//    //          this,    SLOT  ( update_yMax ( double ) ) );

//    validate_ff0();

//    update_plot();
}

// update plot limit y max
void US_Grid_Editor::update_yMax( double dval )
{
//    yMax    = dval;
// qDebug() << "update_yMax" << yMax;
//    // ct_yMin->disconnect();
//    // ct_yMin->setMaximum( yMax );

//    // connect( ct_yMin, SIGNAL( valueChanged( double ) ),
//    //          this,    SLOT  ( update_yMin ( double ) ) );

//    validate_ff0();

//    update_plot();
}

// update plot limit z-value (f/f0 or vbar)
void US_Grid_Editor::update_zVal( double dval )
{
   // zVal    = dval;
   // vbar    = ( y_param == ATTR_V ) ? zVal : vbar;
   // ff0     = ( y_param == ATTR_K ) ? zVal : ff0;

   // validate_ff0();

   // update_plot();
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
   // ct_partialGrid->setRange     ( 1, subGrids );
   // ct_partialGrid->setSingleStep( 1 );
   // le_counts->setText( tr( "%1 total, %2 per subgrid" )
   //       .arg( ntotg ).arg( ntotg / subGrids ) );
   update_plot();
}

// update density
void US_Grid_Editor::update_exp_data( )
{

}

// update plot
void US_Grid_Editor::update_plot( void )
{
qDebug() << "update_plot:  call calc_gridpoints()";
   calc_gridpoints();

   QString xatitle = Attr_to_long( x_param );
   QString yatitle = Attr_to_long( y_param );

   if ( selected_plot == 1 )
      xatitle         = tr( "Molecular Weight" );

   dataPlotClear( data_plot );
   data_plot->setAxisTitle( QwtPlot::xBottom, xatitle );
   data_plot->setAxisTitle( QwtPlot::yLeft,   yatitle );

   //print_minmax();
   int gridsize;
   QVector <double> xData1;
   QVector <double> yData1;
   QVector <double> xData2;
   QVector <double> yData2;

   int iplt_x = ( selected_plot == 0 ) ? x_param : ATTR_W;

   xData1.clear();
   yData1.clear();
   xData2.clear();
   yData2.clear();

   // if ( ck_show_final_grid->isChecked()  &&
   //      !ck_show_sub_grid->isChecked())
      if (true)
   {
      gridsize = final_grid.size();

      for ( int ii = 0; ii < gridsize; ii++ )
      {
         if ( final_grid[ ii ].index == partialGrid )
         {
            xData1 << grid_value( final_grid[ ii ], iplt_x );
            yData1 << grid_value( final_grid[ ii ], y_param );
         }

         else
         {
            xData2 << grid_value( final_grid[ ii ], iplt_x );
            yData2 << grid_value( final_grid[ ii ], y_param );
         }
      }

      QwtPlotCurve *c1;
      QwtSymbol*   sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setBrush( QColor( Qt::red ) );
      sym1->setPen  ( QColor( Qt::red ) );
      sym1->setSize ( 3 );

      c1 = us_curve( data_plot, "highlighted Grid points" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

      QwtPlotCurve *c2;
      QwtSymbol*   sym2 = new QwtSymbol;
      sym2->setStyle( QwtSymbol::Ellipse );
      sym2->setBrush( QColor( Qt::yellow ) );
      sym2->setPen  ( QColor( Qt::yellow ) );
      sym2->setSize ( 3 );

      c2 = us_curve( data_plot, "Other Grid points" );
      c2->setSymbol ( sym2 );
      c2->setStyle  ( QwtPlotCurve::NoCurve );
      c2->setSamples( xData2.data(), yData2.data(), xData2.size() );
   }

   // else if ( ck_show_final_grid->isChecked()  &&
   //           ck_show_sub_grid->isChecked())
      else if (0)
   {
      gridsize    = final_grid.size();
      int counter = 1;

      for ( int ii = 0; ii < gridsize; ii++ )
      {
         if ( counter == partialGrid )
         {
            xData1 << grid_value( final_grid[ ii ], iplt_x );
            yData1 << grid_value( final_grid[ ii ], y_param );
         }

         else
         {
            xData2 << grid_value( final_grid[ ii ], iplt_x );
            yData2 << grid_value( final_grid[ ii ], y_param );
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

      c1 = us_curve( data_plot, "highlighted Grid points" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

      QwtPlotCurve *c2;
      QwtSymbol*   sym2 = new QwtSymbol;
      sym2->setStyle( QwtSymbol::Ellipse );
      sym2->setBrush( QColor( Qt::yellow ) );
      sym2->setPen  ( QColor( Qt::yellow ) );
      sym2->setSize( 3 );

      c2 = us_curve( data_plot, "Other Grid points" );
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
         yData1[ ii ] = grid_value( current_grid[ ii ], y_param );
      }

      QwtPlotCurve *c1;
      QwtSymbol*   sym1 = new QwtSymbol;
      sym1->setStyle( QwtSymbol::Ellipse );
      sym1->setBrush( QColor( Qt::yellow ) );
      sym1->setPen  ( QColor( Qt::yellow ) );
      sym1->setSize( 3 );

      c1 = us_curve( data_plot, "Grid points 1" );
      c1->setSymbol ( sym1 );
      c1->setStyle  ( QwtPlotCurve::NoCurve );
      c1->setSamples( xData1.data(), yData1.data(), gridsize );
   }

   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->replot();
}

// Calculate all grid points for the selected setting
void US_Grid_Editor::calc_gridpoints( void )
{
   struct gridpoint tmp_point;
qDebug() << "calc_g: px py pz" << x_param << y_param << z_param;

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

   int    xRes = le_x_res->text().toInt();
   double xMin = le_x_min->text().toDouble();
   double xMax = le_x_max->text().toDouble();
   int    yRes = le_y_res->text().toInt();
   double yMin = le_y_min->text().toDouble();
   double yMax = le_y_max->text().toDouble();
   double zVal = le_z_val->text().toDouble();

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

   switch( z_param )
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
   if ( x_param != ATTR_V  &&  y_param != ATTR_V  &&  z_param != ATTR_V )
   {
qDebug() << "calc_g:  CG2";
      calc_gridpoints_2();
   }
                                                       ///////////////
   else if ( x_param == ATTR_S  &&  y_param == ATTR_K )  // s and f_f0
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
   else if ( x_param == ATTR_S  &&  y_param == ATTR_W )  // s and mw
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
   else if ( x_param == ATTR_S  &&  y_param == ATTR_V )  // s and vbar
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

            if ( z_param == ATTR_K )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_S  &&  y_param == ATTR_D )  // s and D
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
   else if ( x_param == ATTR_S  &&  y_param == ATTR_W )  // s and f
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
   else if ( x_param == ATTR_K  &&  y_param == ATTR_S )  // ff0 and s
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
   else if ( x_param == ATTR_K  &&  y_param == ATTR_W )  // ff0 and mw
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
   else if ( x_param == ATTR_K  &&  y_param == ATTR_V )  // ff0 and vbar
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_K  &&  y_param == ATTR_D )  // ff0 and D
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
   else if ( x_param == ATTR_K  &&  y_param == ATTR_F )  // ff0 and f
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
   else if ( x_param == ATTR_W  &&  y_param == ATTR_S )  // mw and s
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
   else if ( x_param == ATTR_W  &&  y_param == ATTR_K )  // mw and f/f0
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
   else if ( x_param == ATTR_W  &&  y_param == ATTR_V )  // mw and vbar
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_W  &&  y_param == ATTR_D )  // mw and D
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
   else if ( x_param == ATTR_W  &&  y_param == ATTR_F )  // mw and f
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
   else if ( x_param == ATTR_V  &&  y_param == ATTR_S )  // vbar and s
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

            if ( z_param == ATTR_K )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_V  &&  y_param == ATTR_K )  // vbar and ff0
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_skv( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_V  &&  y_param == ATTR_W )  // vbar and mw
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_swv( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kwv( tmp_point );
            else if ( z_param == ATTR_D )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_V  &&  y_param == ATTR_D )  // vbar and D
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_V  &&  y_param == ATTR_F )  // vbar and f
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_svf( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kvf( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_wvf( tmp_point );
            else if ( z_param == ATTR_D )
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
   else if ( x_param == ATTR_D  &&  y_param == ATTR_S )  // D and s
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
   else if ( x_param == ATTR_D  &&  y_param == ATTR_K )  // D and ff0
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
   else if ( x_param == ATTR_D  &&  y_param == ATTR_W )  // D and mw
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
   else if ( x_param == ATTR_D  &&  y_param == ATTR_V )  // D and vbar
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_svd( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kvd( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_wvd( tmp_point );
            else if ( z_param == ATTR_F )
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
   else if ( x_param == ATTR_D  &&  y_param == ATTR_F )  // D and f
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
   else if ( x_param == ATTR_F  &&  y_param == ATTR_S )  // f and s
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
   else if ( x_param == ATTR_F  &&  y_param == ATTR_K )  // f and ff0
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
   else if ( x_param == ATTR_F  &&  y_param == ATTR_W )  // f and mw
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
   else if ( x_param == ATTR_F  &&  y_param == ATTR_V )  // f and vbar
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

            if ( z_param == ATTR_S )
               comp_ok           = set_comp_svf( tmp_point );
            else if ( z_param == ATTR_K )
               comp_ok           = set_comp_kvf( tmp_point );
            else if ( z_param == ATTR_W )
               comp_ok           = set_comp_wvf( tmp_point );
            else if ( z_param == ATTR_D )
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
   else if ( x_param == ATTR_F  &&  y_param == ATTR_D )  // f and D
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

      if ( x_param != ATTR_V  &&  y_param != ATTR_V  &&  z_param != ATTR_V )
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

   switch( z_param )
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
   // pb_save->setEnabled( true );
   // ck_show_final_grid->setEnabled( true );
   // pb_delete_partialGrid->setEnabled( true );
   // ct_partialGrid->setEnabled( true );

   int ntotg       = final_grid.size();
   int minsubg     = ntotg / MAXSSZ;
   int maxsubg     = ntotg / MINSSZ + 1;
   subGrids        = ntotg / DEFSSZ + 1;
   // ct_subGrids->disconnect   ();
   // ct_subGrids->setRange     ( minsubg, maxsubg );
   // ct_subGrids->setSingleStep( 1 );
   // ct_subGrids->setValue     ( subGrids );
   // connect( ct_subGrids, SIGNAL( valueChanged   ( double ) ),
   //          this,        SLOT  ( update_subGrids( double ) ) );

   // le_counts->setText( tr( "%1 total, %2 per subgrid" )
   //       .arg( ntotg ).arg( ntotg / subGrids ) );

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
   // ct_partialGrid->setRange     ( 1, grid_index );
   // ct_partialGrid->setSingleStep( 1 );
   // if (grid_index == 0)
   // {
   //    ck_show_final_grid->setChecked (false);
   //    ck_show_final_grid->setEnabled (false);
   //    ct_partialGrid->setRange      ( 0, 0 );
   //    ct_partialGrid->setSingleStep ( 1 );
   //    ct_partialGrid->setEnabled( false );
   //    show_final_grid( false );
   // }
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

// // Select coordinate for horizontal axis
// void US_Grid_Editor::select_x_axis( int ival )
// {
//    // Axis types                   s    f/f0      mw   vbar     D     f
//    const double  xvmns[] = {       1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
//    const double  xvmxs[] = {      10.0,   4.0,   1e+5,  0.80, 1e-7, 1e-7 };
//    const double  xmins[] = { -500000.0,   1.0,    0.0,  0.01, 1e-9, 1e-9 };
//    const double  xmaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 3e-5, 1e-5 };
//    const double  xincs[] = {      0.01,  0.01, 5000.0,  0.01, 1e-9, 1e-9 };
//    const QString xtitls[] = { tr( "s (x 1e13)" ),
//                               tr( "f/f0-value" ),
//                               tr( "mw-value" ),
//                               tr( "vbar-value" ),
//                               tr( "D-value" ),
//                               tr( "f-value" ) };
//    x_param = ival;

//    lb_xRes->setText( xtitls[ x_param ] + tr( " Resolution:" ) );
//    lb_xMin->setText( xtitls[ x_param ] + tr( " Minimum:" ) );
//    lb_xMax->setText( xtitls[ x_param ] + tr( " Maximum:" ) );
//    ct_xMin->disconnect();
//    ct_xMax->disconnect();
//    xMin   = xvmns[ x_param ];
//    xMax   = xvmxs[ x_param ];
//    ct_xMin->setRange     ( xmins[ x_param ], xmaxs[ x_param ] );
//    ct_xMin->setSingleStep( xincs[ x_param ] );
//    ct_xMax->setRange     ( xmins[ x_param ], xmaxs[ x_param ] );
//    ct_xMax->setSingleStep( xincs[ x_param ] );
//    ct_xMin->setValue( xMin );
//    ct_xMax->setValue( xMax );

//    connect( ct_xMin, SIGNAL( valueChanged( double ) ),
//             this,    SLOT  ( update_xMin ( double ) ) );
//    connect( ct_xMax, SIGNAL( valueChanged( double ) ),
//             this,    SLOT  ( update_xMax ( double ) ) );

//    rb_y_s   ->setEnabled( x_param != ATTR_S );
//    rb_y_ff0 ->setEnabled( x_param != ATTR_K );
//    rb_y_mw  ->setEnabled( x_param != ATTR_W );
//    rb_y_vbar->setEnabled( x_param != ATTR_V );
//    rb_y_D   ->setEnabled( x_param != ATTR_D );
//    rb_y_f   ->setEnabled( x_param != ATTR_F );
//    rb_x_s   ->setEnabled( y_param != ATTR_S );
//    rb_x_ff0 ->setEnabled( y_param != ATTR_K );
//    rb_x_mw  ->setEnabled( y_param != ATTR_W );
//    rb_x_vbar->setEnabled( y_param != ATTR_V );
//    rb_x_D   ->setEnabled( y_param != ATTR_D );
//    rb_x_f   ->setEnabled( y_param != ATTR_F );

//    cb_fixed->disconnect();
//    cb_fixed->clear();
//    if ( x_param != ATTR_V  &&  y_param != ATTR_V )
//       cb_fixed->addItem( tr( "Partial Specific Volume" ) );
//    if ( x_param != ATTR_K  &&  y_param != ATTR_K )
//       cb_fixed->addItem( tr( "Frictional Ratio" ) );
//    if ( x_param != ATTR_W  &&  y_param != ATTR_W )
//       cb_fixed->addItem( tr( "Molecular Weight" ) );
//    if ( x_param != ATTR_S  &&  y_param != ATTR_S )
//       cb_fixed->addItem( tr( "Sedimentation Coefficient" ) );
//    if ( x_param != ATTR_D  &&  y_param != ATTR_D )
//       cb_fixed->addItem( tr( "Diffusion Coefficient" ) );
//    if ( x_param != ATTR_F  &&  y_param != ATTR_F )
//       cb_fixed->addItem( tr( "Frictional Coefficient" ) );
//    cb_fixed->setCurrentIndex( 0 );
//    select_fixed( cb_fixed->currentText() );
//    connect( cb_fixed, SIGNAL( activated   ( const QString& ) ),
//             this,     SLOT  ( select_fixed( const QString& ) ) );

//    validate_ff0();

//    update_plot();
// }

// // select coordinate for vertical axis
// void US_Grid_Editor::select_y_axis( int ival )
// {
//    // Axis types                   s    f/f0      mw   vbar     D     f
//    const double  yvmns[] = {       1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
//    const double  yvmxs[] = {      10.0,   4.0,   1e+6,  0.80, 1e-7, 1e-7 };
//    const double  ymins[] = { -500000.0,   1.0,    0.0,  0.01, 1e-9, 1e-9 };
//    const double  ymaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 3e-5, 1e-5 };
//    const double  yincs[] = {      0.01,  0.01, 5000.0,  0.01, 1e-9, 1e-9 };
//    const QString ytitls[] = { tr( "s (x 1e13)" ),
//                               tr( "f/f0-value" ),
//                               tr( "mw-value" ),
//                               tr( "vbar-value" ),
//                               tr( "D-value" ),
//                               tr( "f-value" ) };
//    y_param = ival;

//    lb_yRes->setText( ytitls[ y_param ] + tr( " Resolution:" ) );
//    lb_yMin->setText( ytitls[ y_param ] + tr( " Minimum:" ) );
//    lb_yMax->setText( ytitls[ y_param ] + tr( " Maximum:" ) );
//    ct_yMin->disconnect();
//    ct_yMax->disconnect();
//    yMin   = yvmns[ y_param ];
//    yMax   = yvmxs[ y_param ];
//    ct_yMin->setRange     ( ymins[ y_param ], ymaxs[ y_param ] );
//    ct_yMin->setSingleStep( yincs[ y_param ] );
//    ct_yMax->setRange     ( ymins[ y_param ], ymaxs[ y_param ] );
//    ct_yMax->setSingleStep( yincs[ y_param ] );
//    ct_yMin->setValue( yMin );
//    ct_yMax->setValue( yMax );

//    connect( ct_yMin, SIGNAL( valueChanged( double ) ),
//             this,    SLOT  ( update_yMin ( double ) ) );
//    connect( ct_yMax, SIGNAL( valueChanged( double ) ),
//             this,    SLOT  ( update_yMax ( double ) ) );

//    rb_x_s   ->setEnabled( y_param != ATTR_S );
//    rb_x_ff0 ->setEnabled( y_param != ATTR_K );
//    rb_x_mw  ->setEnabled( y_param != ATTR_W );
//    rb_x_vbar->setEnabled( y_param != ATTR_V );
//    rb_x_D   ->setEnabled( y_param != ATTR_D );
//    rb_x_f   ->setEnabled( y_param != ATTR_F );
//    rb_y_s   ->setEnabled( x_param != ATTR_S );
//    rb_y_ff0 ->setEnabled( x_param != ATTR_K );
//    rb_y_mw  ->setEnabled( x_param != ATTR_W );
//    rb_y_vbar->setEnabled( x_param != ATTR_V );
//    rb_y_D   ->setEnabled( x_param != ATTR_D );
//    rb_y_f   ->setEnabled( x_param != ATTR_F );

//    cb_fixed->disconnect();
//    cb_fixed->clear();
//    if ( x_param != ATTR_V  &&  y_param != ATTR_V )
//       cb_fixed->addItem( tr( "Partial Specific Volume" ) );
//    if ( x_param != ATTR_K  &&  y_param != ATTR_K )
//       cb_fixed->addItem( tr( "Frictional Ratio" ) );
//    if ( x_param != ATTR_W  &&  y_param != ATTR_W )
//       cb_fixed->addItem( tr( "Molecular Weight" ) );
//    if ( x_param != ATTR_S  &&  y_param != ATTR_S )
//       cb_fixed->addItem( tr( "Sedimentation Coefficient" ) );
//    if ( x_param != ATTR_D  &&  y_param != ATTR_D )
//       cb_fixed->addItem( tr( "Diffusion Coefficient" ) );
//    if ( x_param != ATTR_F  &&  y_param != ATTR_F )
//       cb_fixed->addItem( tr( "Frictional Coefficient" ) );
//    cb_fixed->setCurrentIndex( 0 );
//    select_fixed( cb_fixed->currentText() );
//    connect( cb_fixed, SIGNAL( activated   ( const QString& ) ),
//             this,     SLOT  ( select_fixed( const QString& ) ) );

//    validate_ff0();

//    update_plot();
// }

// // Select coordinate flag for the fixed attribute
// void US_Grid_Editor::select_fixed( const QString& fixstr )
// {
//    // Axis types                   s    f/f0      mw   vbar     D     f
//    const double  zmins[] = { -500000.0,   1.0,    0.0,  0.01, 1e+6, 1e+6 };
//    const double  zmaxs[] = {  500000.0,  50.0,  1e+10,  3.00, 1e+8, 1e+6 };
//    const double  zincs[] = {      0.01,  0.01, 5000.0, 0.001, 1e+5, 1e+5 };
//    //const double  zvals[] = {     5.00,   2.0,   1e+5,  0.72, 1e+7, 1e+7 };

//    z_param   = fixstr.contains( tr( "Partial S" ) ) ? ATTR_V : 0;
//    z_param   = fixstr.contains( tr( "nal Ratio" ) ) ? ATTR_K : z_param;
//    z_param   = fixstr.contains( tr( "ar Weight" ) ) ? ATTR_W : z_param;
//    z_param   = fixstr.contains( tr( "Sedimenta" ) ) ? ATTR_S : z_param;
//    z_param   = fixstr.contains( tr( "Diffusion" ) ) ? ATTR_D : z_param;
//    z_param   = fixstr.contains( tr( "nal Coeff" ) ) ? ATTR_F : z_param;
// qDebug() << "SelFix: " << fixstr << "z_param" << z_param;

//    ct_zVal->setRange     ( zmins[ z_param ], zmaxs[ z_param ] );
//    ct_zVal->setSingleStep( zincs[ z_param ] );
//    //ct_zVal->setValue( zvals[ z_param ] );
//    lb_zVal->setText( fixstr );

//    validate_ff0();
// }

// activated when the "Show Final Grid" Checkbox is set
void US_Grid_Editor::show_final_grid( bool flag )
{
   // if (flag)
   // {
   //    ct_partialGrid       ->setRange     ( 1, grid_index );
   //    ct_partialGrid       ->setSingleStep( 1 );
   //    ct_partialGrid       ->setEnabled( true );
   //    pb_delete_partialGrid->setEnabled( true );
   //    pb_add_partialGrid   ->setEnabled( false );
   //    ck_show_sub_grid     ->setEnabled( true );
   //    ct_subGrids          ->setEnabled( false );
   // }
   // else
   // {
   //    ct_partialGrid       ->setEnabled( false );
   //    pb_delete_partialGrid->setEnabled( false );
   //    pb_add_partialGrid   ->setEnabled( true );
   //    ck_show_sub_grid     ->setEnabled( false );
   //    ct_subGrids          ->setEnabled( false );
   // }

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
      // lb_partialGrid       ->setText   ( tr( "Highlight Subgrid #:" ) );
      // ct_subGrids          ->setEnabled   ( true );
      // ct_subGrids          ->setRange     ( minsubg, maxsubg );
      // ct_subGrids          ->setSingleStep( 1 );
      // ct_subGrids          ->setValue     ( subGrids );
      // pb_delete_partialGrid->setEnabled   ( false );
      // le_counts->setText( tr( "%1 total, %2 per subgrid" )
      //       .arg( ntotg ).arg( ntotg / subGrids ) );
   }
   else
   {
      // lb_partialGrid       ->setText   ( tr( "Highlight Partial Grid #:" ) );
      // ct_subGrids          ->setEnabled   ( false );
      // ct_partialGrid       ->setRange     ( 1, grid_index );
      // ct_partialGrid       ->setSingleStep( 1 );
      // ct_partialGrid       ->setValue     ( 1 );
      // pb_delete_partialGrid->setEnabled   ( true );
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
   double buoy   = 1.0 - vbar * DENS_20W;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}

// Complete component given s+k+v (s, ff0, vbar)
bool US_Grid_Editor::set_comp_skv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
   double sval   = gpoint.s * 1.0e-13;

   gpoint.D      = R_GC * K20 / ( AVOGADRO * 18 * M_PI
                   * pow( ( VISC_20W * 0.01 * gpoint.ff0 ), (3.0/2.0) )
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
   double buoy   = 1.0 - vbar * DENS_20W;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+k+f (s, ff0, f)
bool US_Grid_Editor::set_comp_skf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+w+v (s, mw, vbar)
bool US_Grid_Editor::set_comp_swv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
   bool   is_ok  = check_grid_point( buoy, gpoint );
qDebug() << "comp_swd buoy" << buoy;

   return is_ok;
}
// Complete component given s+w+f (s, mw, f)
bool US_Grid_Editor::set_comp_swf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_swf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given s+v+d (s, vbar, D)
bool US_Grid_Editor::set_comp_svd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_sdf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;

   return is_ok;
}
// Complete component given k+w+v (ff0, mw, vbar)
bool US_Grid_Editor::set_comp_kwv( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
   double vbrat  = vbar / AVOGADRO;
   double viscf  = VISC_20W * 0.01;

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
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_kwd buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+w+f (ff0, mw, f)
bool US_Grid_Editor::set_comp_kwf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_kwf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given k+v+d (ff0, vbar, D)
bool US_Grid_Editor::set_comp_kvd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_kdf buoy" << buoy;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given w+v+d (mw, vbar, D)
bool US_Grid_Editor::set_comp_wvd( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
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
   double buoy   = 1.0 - vbar * DENS_20W;
qDebug() << "comp_wdf buoy" << buoy;
   gpoint.ff0    = qRound( gpoint.ff0 * 1.0e+5 ) * 1.0e-5;
   bool   is_ok  = check_grid_point( buoy, gpoint );

   return is_ok;
}
// Complete component given v+d+f (vbar, D, f)
bool US_Grid_Editor::set_comp_vdf( struct gridpoint& gpoint )
{
   double vbar   = gpoint.vbar;
   double buoy   = 1.0 - vbar * DENS_20W;
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

   if ( x_param == ATTR_K  ||  y_param == ATTR_K  ||  z_param == ATTR_K )
   {  // If one of the attributes is f/f0, no need for checking
      return is_ok;
   }

   struct gridpoint zpoint;
   struct gridpoint tmp_point;
   double xMin = le_x_min->text().toDouble();
   double xMax = le_x_max->text().toDouble();
   double yMin = le_y_min->text().toDouble();
   double yMax = le_y_max->text().toDouble();
   double zVal = le_z_val->text().toDouble();
qDebug() << "valFF0: xMin xMax yMin yMax" << xMin << xMax << yMin << yMax;
   clear_grid( zpoint );
   set_grid_value( zpoint,    z_param, zVal );
   // Get f/f0 for xMin,yMin
   tmp_point      = zpoint;
   set_grid_value( tmp_point, x_param, xMin );
   set_grid_value( tmp_point, y_param, yMin );
   complete_comp ( tmp_point );
   double ffx1y1  = tmp_point.ff0;
   // Get f/f0 for xMin,yMax
   tmp_point      = zpoint;
   set_grid_value( tmp_point, x_param, xMin );
   set_grid_value( tmp_point, y_param, yMax );
   complete_comp ( tmp_point );
   double ffx1y2  = tmp_point.ff0;
   // Get f/f0 for xMax,yMin
   tmp_point      = zpoint;
   set_grid_value( tmp_point, x_param, xMax );
   set_grid_value( tmp_point, y_param, yMin );
   complete_comp ( tmp_point );
   double ffx2y1  = tmp_point.ff0;
   // Get f/f0 for xMax,yMax
   tmp_point      = zpoint;
   set_grid_value( tmp_point, x_param, xMax );
   set_grid_value( tmp_point, y_param, yMax );
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

      if ( x_param == ATTR_W  ||  x_param == ATTR_F )
      {  // Adjust the X range (if MW or f)
         // ct_xMin->disconnect();
         // ct_xMax->disconnect();
         tmp_point      = zpoint;
         set_grid_value( tmp_point, ATTR_K, 1.0  );

         if ( ffx1y1 < ffx1y2 )
         {  // Increasing Y means increasing f/f0, so get X for k=1,ymin
            set_grid_value( tmp_point, y_param, yMin );
            complete_comp ( tmp_point );
            double xVal    = grid_value( tmp_point, x_param );

            if ( ffx1y1 < ffx2y1 )
            {  // Increasing X means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (1xMin)xVal" << xVal;
               // ct_xMin->setMinimum( xVal );
               // ct_xMax->setMinimum( xVal );
            }

            else
            {  // Increasing X means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (2xMin)xVal" << xVal;
               // ct_xMin->setMaximum( xVal );
               // ct_xMax->setMaximum( xVal );
            }
         }

         else
         {  // Increasing Y means decreasing f/f0, so get X for k=1,ymax
            set_grid_value( tmp_point, y_param, yMax );
            complete_comp ( tmp_point );
            double xVal    = grid_value( tmp_point, x_param );

            if ( ffx1y1 < ffx2y1 )
            {  // Increasing X means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (3xMin)xVal" << xVal;
               // ct_xMin->setMinimum( xVal );
               // ct_xMax->setMinimum( xVal );
            }

            else
            {  // Increasing X means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (4xMin)xVal" << xVal;
               // ct_xMin->setMaximum( xVal );
               // ct_xMax->setMaximum( xVal );
            }
         }

         // connect( ct_xMin, SIGNAL( valueChanged( double ) ),
         //          this,    SLOT  ( update_xMin ( double ) ) );
         // connect( ct_xMax, SIGNAL( valueChanged( double ) ),
         //          this,    SLOT  ( update_xMax ( double ) ) );
      }

      if ( y_param == ATTR_W  ||  y_param == ATTR_F )
      {  // Adjust the Y range (if MW or f)
         // ct_yMin->disconnect();
         // ct_yMax->disconnect();
         tmp_point      = zpoint;
         set_grid_value( tmp_point, ATTR_K, 1.0  );

         if ( ffx1y1 < ffx2y1 )
         {  // Increasing X means increasing f/f0, so get Y for k=1,xmin
            set_grid_value( tmp_point, x_param, xMin );
            complete_comp ( tmp_point );
            double yVal    = grid_value( tmp_point, y_param );

            if ( ffx1y1 < ffx1y2 )
            {  // Increasing Y means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (5yMin)yVal" << yVal;
               // ct_yMin->setMinimum( yVal );
               // ct_yMax->setMinimum( yVal );
            }

            else
            {  // Increasing Y means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (6yMax)yVal" << yVal;
               // ct_yMin->setMaximum( yVal );
               // ct_yMax->setMaximum( yVal );
            }
         }

         else
         {  // Increasing X means decreasing f/f0, so get y for k=1,xmax
            set_grid_value( tmp_point, x_param, xMax );
            complete_comp ( tmp_point );
            double yVal    = grid_value( tmp_point, y_param );

            if ( ffx1y1 < ffx1y2 )
            {  // Increasing Y means increasing f/f0, so set lower limit
qDebug() << "valFF0:  (7yMin)yVal" << yVal;
               // ct_yMin->setMinimum( yVal );
               // ct_yMax->setMinimum( yVal );
            }

            else
            {  // Increasing Y means decreasing f/f0, so set upper limit
qDebug() << "valFF0:  (8yMax)yVal" << yVal;
               // ct_yMin->setMaximum( yVal );
               // ct_yMax->setMaximum( yVal );
            }
         }

         // connect( ct_yMin, SIGNAL( valueChanged( double ) ),
         //          this,    SLOT  ( update_yMin ( double ) ) );
         // connect( ct_yMax, SIGNAL( valueChanged( double ) ),
         //          this,    SLOT  ( update_yMax ( double ) ) );
      }
   }

   // xMin           = ct_xMin->value();
   // xMax           = ct_xMax->value();
   // yMin           = ct_yMin->value();
   // yMax           = ct_yMax->value();
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


US_Grid_Preset::US_Grid_Preset(QWidget * parent) : US_WidgetsDialog(parent)
{
   setWindowTitle( tr( "Grid Setup" ) );
   setPalette( US_GuiSettings::frameColor() );
   setFixedSize(500, 250);

   QLabel *lb_x_axis = us_label("X Axis");
   lb_x_axis->setAlignment(Qt::AlignCenter);
   QLabel *lb_y_axis = us_label("Y Axis");
   lb_y_axis->setAlignment(Qt::AlignCenter);

   QGridLayout* x_s     = us_radiobutton( Attr_to_long( ATTR_S ), rb_x_s, true );
   QGridLayout* x_ff0   = us_radiobutton( Attr_to_long( ATTR_K ), rb_x_ff0, false );
   QGridLayout* x_mw    = us_radiobutton( Attr_to_long( ATTR_W ), rb_x_mw, true );
   QGridLayout* x_vbar  = us_radiobutton( Attr_to_long( ATTR_V ), rb_x_vbar, true );
   QGridLayout* x_D     = us_radiobutton( Attr_to_long( ATTR_D ), rb_x_D, true );
   QGridLayout* x_f     = us_radiobutton( Attr_to_long( ATTR_F ), rb_x_f, true );

   QGridLayout* y_s     = us_radiobutton( Attr_to_long( ATTR_S ), rb_y_s, false );
   QGridLayout* y_ff0   = us_radiobutton( Attr_to_long( ATTR_K ), rb_y_ff0, true );
   QGridLayout* y_mw    = us_radiobutton( Attr_to_long( ATTR_W ), rb_y_mw, true );
   QGridLayout* y_vbar  = us_radiobutton( Attr_to_long( ATTR_V ), rb_y_vbar, true );
   QGridLayout* y_D     = us_radiobutton( Attr_to_long( ATTR_D ), rb_y_D, true );
   QGridLayout* y_f     = us_radiobutton( Attr_to_long( ATTR_F ), rb_y_f, true );

   x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    ATTR_S );
   x_axis->addButton( rb_x_ff0,  ATTR_K );
   x_axis->addButton( rb_x_mw,   ATTR_W );
   x_axis->addButton( rb_x_vbar, ATTR_V );
   x_axis->addButton( rb_x_D,    ATTR_D );
   x_axis->addButton( rb_x_f,    ATTR_F );

   y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    ATTR_S );
   y_axis->addButton( rb_y_ff0,  ATTR_K );
   y_axis->addButton( rb_y_mw,   ATTR_W );
   y_axis->addButton( rb_y_vbar, ATTR_V );
   y_axis->addButton( rb_y_D,    ATTR_D );
   y_axis->addButton( rb_y_f,    ATTR_F );

   QLabel *lb_fixed      = us_label( tr( "Fixed Attribute" ) );
   lb_fixed->setAlignment(Qt::AlignCenter);

   z_axis = us_comboBox();

   QPushButton *pb_apply = us_pushbutton( "Apply" );
   QFont font = pb_apply->font();
   font.setBold(true);
   pb_apply->setFont(font);
   pb_apply->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
   QPushButton *pb_cancel = us_pushbutton( "Cancel" );
   pb_cancel->setIcon(this->style()->standardIcon(QStyle::SP_DialogCancelButton));
   pb_cancel->setFont(font);

   QFrame *hline1 = new QFrame();
   hline1->setFrameShape(QFrame::HLine);
   hline1->setFrameShadow(QFrame::Sunken);

   QGridLayout* layout = new QGridLayout();
   layout->setMargin(2);
   layout->setSpacing(3);
   int row = 0;
   layout->addWidget( lb_x_axis,  row,   0, 1, 2 );
   layout->addWidget( lb_y_axis,  row++, 2, 1, 2 );
   layout->addLayout( x_s,        row,   0, 1, 2 );
   layout->addLayout( y_s,        row++, 2, 1, 2 );
   layout->addLayout( x_ff0,      row,   0, 1, 2 );
   layout->addLayout( y_ff0,      row++, 2, 1, 2 );
   layout->addLayout( x_mw,       row,   0, 1, 2 );
   layout->addLayout( y_mw,       row++, 2, 1, 2 );
   layout->addLayout( x_vbar,     row,   0, 1, 2 );
   layout->addLayout( y_vbar,     row++, 2, 1, 2 );
   layout->addLayout( x_D,        row,   0, 1, 2 );
   layout->addLayout( y_D,        row++, 2, 1, 2 );
   layout->addLayout( x_f,        row,   0, 1, 2 );
   layout->addLayout( y_f,        row++, 2, 1, 2 );
   layout->addWidget( hline1,     row++, 0, 1, 4 );
   layout->addWidget( lb_fixed,   row,   0, 1, 2 );
   layout->addWidget( z_axis,     row++, 2, 1, 2 );

   QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   layout->addItem( spacer,                 row++,  0, 1, 4 );

   layout->addWidget( pb_apply,              row,   2, 1, 1 );
   layout->addWidget( pb_cancel,             row++, 3, 1, 1 );

   setLayout(layout);

   x_param        = ATTR_S; // plot s
   y_param        = ATTR_K; // plot f/f0
   z_param        = ATTR_V; // fixed vbar
   x_axis->button(x_param)->setChecked(true);
   y_axis->button(x_param)->setDisabled(true);

   y_axis->button(y_param)->setChecked(true);
   x_axis->button(y_param)->setDisabled(true);
   z_axis->setCurrentIndex(z_axis->findData(z_param, Qt::UserRole));

   set_z_axis();

   connect( x_axis,    &QButtonGroup::idReleased, this, &US_Grid_Preset::select_x_axis );
   connect( y_axis,    &QButtonGroup::idReleased, this, &US_Grid_Preset::select_y_axis );
   connect( pb_apply,  &QPushButton::clicked,     this, &US_Grid_Preset::apply );
   connect( pb_cancel, &QPushButton::clicked,     this, &US_Grid_Preset::cancel );
}

void US_Grid_Preset::parameters(int *x, int *y, int *z)
{
   (*x) = x_param;
   (*y) = y_param;
   (*z) = z_param;
}

// Select coordinate for horizontal axis
void US_Grid_Preset::select_x_axis( int index )
{
   for (int ii = ATTR_S; ii <= ATTR_F; ii++) {
      y_axis->button( ii )->setEnabled(true);
   }
   x_param = index;
   y_axis->button(x_param)->setDisabled(true);

   if ( x_param == ATTR_D ) {
      y_axis->button(ATTR_F)->setDisabled(true);
   } else if ( x_param == ATTR_F ) {
      y_axis->button(ATTR_D)->setDisabled(true);
   }

   set_z_axis();
}

// select coordinate for vertical axis
void US_Grid_Preset::select_y_axis( int index )
{
   for (int ii = ATTR_S; ii <= ATTR_F; ii++) {
      x_axis->button( ii )->setEnabled(true);
   }
   y_param = index;
   x_axis->button(y_param)->setDisabled(true);

   if ( y_param == ATTR_D ) {
      x_axis->button(ATTR_F)->setDisabled(true);
   } else if ( y_param == ATTR_F ) {
      x_axis->button(ATTR_D)->setDisabled(true);
   }

   set_z_axis();
}

void US_Grid_Preset::select_z_axis(int index)
{
   z_param = z_axis->itemData(index, Qt::UserRole ).toInt();
}

void US_Grid_Preset::apply()
{
   accept();
}

void US_Grid_Preset::cancel()
{
   reject();
}

// Select coordinate flag for the fixed attribute
void US_Grid_Preset::set_z_axis( )
{
   z_axis->disconnect();
   z_axis->clear();
   bool has_vbar = (x_param == ATTR_V || y_param == ATTR_V);
   if ( has_vbar ) {
      for (int ii = ATTR_S; ii <= ATTR_F; ii++) {
         if ( ii == x_param || ii == y_param ) continue;
         z_axis->addItem( Attr_to_long( ii ), ii );
      }
      int index = z_axis->findData(z_param, Qt::UserRole);
      if ( index == -1 ) {
         z_axis->setCurrentIndex( 0 );
         z_param = z_axis->itemData(0, Qt::UserRole ).toInt();
      } else {
         z_axis->setCurrentIndex( index );
      }
   } else {
      z_param = ATTR_V;
      z_axis->addItem( Attr_to_long( ATTR_V ), ATTR_V );
   }

   connect( z_axis, QOverload<int>::of( &QComboBox::currentIndexChanged ),
            this,   &US_Grid_Preset::select_z_axis );
}

QString Attr_to_long(int attr)
{
   if ( attr == ATTR_S )
      return QString("Sedimentation Coefficient");
   else if ( attr == ATTR_K )
      return QString("Frictional Ratio");
   else if ( attr == ATTR_W )
      return QString("Molecular Weight");
   else if ( attr == ATTR_V )
      return QString("Partial Specific Volume");
   else if ( attr == ATTR_D )
      return QString("Diffusion Coefficient");
    else if ( attr == ATTR_F )
      return QString("Frictional Coefficient");
   else
      return QString("");
}

QString Attr_to_short(int attr)
{
   if ( attr == ATTR_S )
      return QString("s [ Svedbergs (S) ]");
   else if ( attr == ATTR_K )
      return QString("f / f0");
   else if ( attr == ATTR_W )
      return QString("M [ g / mol ]");
   else if ( attr == ATTR_V )
      return QString("vbar [ mL / g ]");
   else if ( attr == ATTR_D )
      // return QString("D [cm2 s−1]  (\(m^{2}/s\)");
      return QString("<p>D [ cm<sup>2</sup> s<sup>-1</sup> ]</p>");
   else if ( attr == ATTR_F )
      return QString("f [ g / s ]");
   else
      return QString("");
}


GridPoint::GridPoint(int id)
{
   index = id;
   dvt_set = false;
   s.fill(0, 3);
   D.fill(0, 3);
   vbar.fill(0, 3);
   mw.fill(0, 3);
   f.fill(0, 3);
   f0.fill(0, 3);
   ff0.fill(0, 3);

   _s.fill(0, 3);
   _D.fill(0, 3);
   _vbar.fill(0, 3);
   _mw.fill(0, 3);
   _f.fill(0, 3);
   _f0.fill(0, 3);
   _ff0.fill(0, 3);
}

bool GridPoint::set_param(const QVector<double> & param, attr_type ptype)
{
   if ( param.size() != 3 ) {
      return false;
   }
   if ( ! ptypes.contains( ptype ) && ptypes.size() == 3 ) {
      return false;
   }

   switch( ptype )
   {
   case ATTR_S:
      for ( int ii = 0; ii < 3; ii++ ) {
         s[ii] = param.at(ii) * 1.0e-13;
      }
      break;
   case ATTR_K:
      ff0     = param;
      break;
   case ATTR_W:
      mw      = param;
      break;
   case ATTR_V:
      vbar    = param;
      break;
   case ATTR_D:
      D       = param;
      break;
   case ATTR_F:
      f       = param;
      break;
   }

   if ( ! ptypes.contains( ptype ) ) {
      ptypes.insert( ptype );
   }

   if ( ptypes.size() == 3 ) {
      calculate_20w();
      if ( dvt_set ) {
         calculate_real();
      }
   }
   return true;
}

void GridPoint::set_dens_visc_t(double dens, double visc, double T)
{
   density = dens;
   viscosity = visc;
   temperature = T;
   dvt_set = true;

   if ( ptypes.size() == 3 ) {
      calculate_real();
   }
}

void GridPoint::calculate_20w()
{
   if ( contains( ATTR_V, ATTR_S, ATTR_K ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         f[ii] = ff0.at(ii) * f0.at(ii);
         D[ii] = ( R_GC * K20 ) / ( AVOGADRO * f.at(ii));
         mw[ii] = s.at(ii) * AVOGADRO * f.at(ii) / buoyancy;
      }
   } else if ( contains( ATTR_V, ATTR_S, ATTR_W ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         f[ii] = ( mw.at(ii) * buoyancy ) / ( s.at(ii) * AVOGADRO );
         ff0[ii] = f.at(ii) / f0.at(ii);
         D[ii] = ( R_GC * K20 ) / ( AVOGADRO * f.at(ii));
      }

   } else if ( contains( ATTR_V, ATTR_S, ATTR_D ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         f[ii] = ( R_GC * K20 ) / ( AVOGADRO * D.at(ii) );
         ff0[ii] = f.at(ii) / f0.at(ii);
         mw[ii] = s.at(ii) * AVOGADRO * f.at(ii) / buoyancy;
      }
   } else if ( contains( ATTR_V, ATTR_S, ATTR_F ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         ff0[ii] = f.at(ii) / f0.at(ii);
         D[ii] = ( R_GC * K20 ) / ( AVOGADRO * f.at(ii));
         mw[ii] = s.at(ii) * AVOGADRO * f.at(ii) / buoyancy;
      }
   } else if ( contains( ATTR_V, ATTR_K, ATTR_W ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         D[ii] = ( R_GC * K20 ) / ( 3 * VISC_20W ) *
                 qPow( 6 * mw.at(ii) * vbar.at(ii), -1.0 / 3.0 ) *
                 qPow( AVOGADRO * M_PI * ff0.at(ii), -2.0 / 3.0 );
         s[ii] = mw.at(ii) * D.at(ii) * buoyancy / ( R_GC * K20 );
         f[ii] = ( R_GC * K20 ) / ( AVOGADRO * D.at(ii) );
         f0[ii] = f.at(ii) / ff0[ii];
      }
   } else if ( contains( ATTR_V, ATTR_K, ATTR_D ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f[ii] = ( R_GC * K20 ) / ( AVOGADRO * D.at(ii) );
         f0[ii] = f.at(ii) / ff0[ii];
         s[ii] = qPow( f0.at(ii) / ( 9 * VISC_20W * 0.01 * M_PI ), 2 ) * buoyancy / ( 2 * VISC_20W * 0.01 * vbar.at(ii));
         mw[ii] = s.at(ii) * AVOGADRO * f.at(ii) / buoyancy;
      }
   } else if ( contains( ATTR_V, ATTR_K, ATTR_F ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         D[ii] = ( R_GC * K20 ) / ( AVOGADRO * f.at(ii));
         f0[ii] = f.at(ii) / ff0[ii];
         s[ii] = qPow( f0.at(ii) / ( 9 * VISC_20W * 0.01 * M_PI ), 2 ) * buoyancy / ( 2 * VISC_20W * 0.01 * vbar.at(ii));
         mw[ii] = s.at(ii) * AVOGADRO * f.at(ii) / buoyancy;
      }
   } else if ( contains( ATTR_V, ATTR_W, ATTR_D ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         f[ii] = ( R_GC * K20 ) / ( AVOGADRO * D.at(ii) );
         s[ii] = mw.at(ii) * D.at(ii) * buoyancy / ( R_GC * K20 );
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         ff0[ii] = f.at(ii) / f0.at(ii);
      }
   } else if ( contains( ATTR_V, ATTR_W, ATTR_F ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {
         double buoyancy = 1 - vbar.at(ii) * DENS_20W;
         D[ii] = ( R_GC * K20 ) / ( AVOGADRO * f.at(ii));
         s[ii] = mw.at(ii) * D.at(ii) * buoyancy / ( R_GC * K20 );
         f0[ii] = 9 * VISC_20W * 0.01 * M_PI * qSqrt( 2 * vbar.at(ii) * s.at(ii) * VISC_20W * 0.01 / buoyancy );
         ff0[ii] = f.at(ii) / f0.at(ii);
      }
   } else if ( contains( ATTR_V, ATTR_D, ATTR_F ) )
   {
      for ( int ii = 0; ii < 3; ii++ ) {

      }
   }





}

void GridPoint::calculate_real()
{

}

bool GridPoint::contains(attr_type p1, attr_type p2, attr_type p3)
{
   return ptypes.contains(p1) && ptypes.contains(p2) && ptypes.contains(p3);
}
