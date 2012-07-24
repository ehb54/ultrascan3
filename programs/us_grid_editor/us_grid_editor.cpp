//! \file us_ga_init.cpp

#include <QApplication>
#include "us_grid_editor.h"

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
	density = DENS_20W;

   int s_row = 0;
   dbg_level = US_Settings::us_debug();

   // series of rows: most of them label on left, counter/box on right
   lbl_info1      = us_banner( tr( "Grid Editor Controls" ) );
   left->addWidget( lbl_info1, s_row++, 0, 1, 2 );

	QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   left->addWidget( pb_investigator, s_row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   left->addWidget( le_investigator, s_row++, 1 );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );
   left->addLayout( dkdb_cntrls, s_row++, 0, 1, 2 );

   lbl_xaxis      = us_label( tr( "Adjust X-Axis as:" ) );
   lbl_xaxis->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_xaxis, s_row, 0 );

	lbl_yaxis      = us_label( tr( "Adjust Y-Axis as:" ) );
   lbl_yaxis->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_yaxis, s_row++, 1 );

	QGridLayout* x_s = us_radiobutton( tr("Sedimentation Coeff."), rb_x_s, true );
	left->addLayout( x_s, s_row, 0 );

	QGridLayout* y_ff0 = us_radiobutton( tr("Frictional Ratio"), rb_y_ff0, true );
	left->addLayout( y_ff0, s_row++, 1 );

	QGridLayout* x_mw = us_radiobutton( tr("Molecular Weight"), rb_x_mw, true );
	left->addLayout( x_mw, s_row, 0 );

	QGridLayout* y_vbar = us_radiobutton( tr("Partial Specific Volume"), rb_y_vbar, true );
	left->addLayout( y_vbar, s_row++, 1 );

	QGridLayout* toggle1 = us_radiobutton( tr("Sedimentation View"), rb_plot1, true );
	left->addLayout( toggle1, s_row, 0 );

	QGridLayout* toggle2 = us_radiobutton( tr("Molecular Weight View"), rb_plot2, true );
	left->addLayout( toggle2, s_row++, 1 );

	QButtonGroup* x_axis = new QButtonGroup( this );
	x_axis->addButton( rb_x_s, 0);
	x_axis->addButton( rb_x_mw, 1);
	connect( x_axis, SIGNAL( buttonReleased( int )), SLOT( select_x_axis( int )));

	QButtonGroup* y_axis = new QButtonGroup( this );
	y_axis->addButton( rb_y_ff0, 0);
	y_axis->addButton( rb_y_vbar, 1);
	connect( y_axis, SIGNAL( buttonReleased( int )), SLOT( select_y_axis( int )));

	QButtonGroup* toggle_plot = new QButtonGroup( this );
	toggle_plot->addButton( rb_plot1, 0);
	toggle_plot->addButton( rb_plot2, 1);
	connect( toggle_plot, SIGNAL( buttonReleased( int )), SLOT( select_plot( int )));

   lbl_xRes      = us_label( tr( "X Resolution:" ) );
   lbl_xRes->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_xRes, s_row, 0 );

   ct_xRes      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_xRes->setStep( 1 );
   left->addWidget( ct_xRes, s_row++, 1 );
   connect( ct_xRes,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_xRes( double ) ) );

   lbl_yRes      = us_label( tr( "f/f0 Resolution:" ) );
   lbl_yRes->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_yRes, s_row, 0 );

   ct_yRes      = us_counter( 3, 10.0, 1000.0, 0.0 );
   ct_yRes->setStep( 1 );
   left->addWidget( ct_yRes, s_row++, 1 );
   connect( ct_yRes,  SIGNAL( valueChanged( double ) ),
            this,      SLOT( update_yRes( double ) ) );

   lbl_xMin     = us_label( tr( "s-value Mininmum:" ) );
   lbl_xMin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_xMin, s_row, 0 );

   ct_xMin     = us_counter( 3, -10000.0, 9.9, 0.0 );
   ct_xMin->setStep( 1 );
   left->addWidget( ct_xMin, s_row++, 1 );
   connect( ct_xMin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_xMin( double ) ) );

   lbl_xMax     = us_label( tr( "s-value Maximum:" ) );
   lbl_xMax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_xMax, s_row, 0 );

   ct_xMax     = us_counter( 3, 1.1, 10000.0, 0.0 );
   ct_xMax->setStep( 1 );
   left->addWidget( ct_xMax, s_row++, 1 );
   connect( ct_xMax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_xMax( double ) ) );

   lbl_yMin     = us_label( tr( "f/f0 Minimum:" ) );
   lbl_yMin->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_yMin, s_row, 0 );

   ct_yMin     = us_counter( 3, 1.0, 3.9, 0.1 );
   ct_yMin->setStep( 1 );
	left->addWidget( ct_yMin, s_row++, 1 );
   connect( ct_yMin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_yMin( double ) ) );

   lbl_yMax     = us_label( tr( "f/f0 Maximum:" ) );
   lbl_yMax->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_yMax, s_row, 0 );

   ct_yMax     = us_counter( 3, 1.1, 50.0, 0.1 );
   ct_yMax->setStep( 1 );
   left->addWidget( ct_yMax, s_row++, 1 );
   connect( ct_yMax, SIGNAL( valueChanged( double ) ),
         this,         SLOT( update_yMax( double ) ) );

   lbl_zVal     = us_label( tr( "Partial Spec. Volume:" ) );
   lbl_zVal->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_zVal, s_row, 0 );

   ct_zVal     = us_counter( 3, 0.05, 2, 0.001 );
   ct_zVal->setStep( 1 );
   left->addWidget( ct_zVal, s_row++, 1 );
   connect( ct_zVal, SIGNAL( valueChanged( double ) ),
         this,         SLOT( update_zVal( double ) ) );

   lbl_density     = us_label( tr( "Density:" ) );
   lbl_density->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_density, s_row, 0 );
	str.setNum( density );
	QLineEdit* le_density = us_lineedit( str.setNum( density ));
   left->addWidget( le_density, s_row++, 1 );
	connect( le_density, SIGNAL( textChanged( const QString& ) ),
			   SLOT( update_density( const QString& ) ) );

   lbl_viscosity     = us_label( tr( "Viscosity:" ) );
   lbl_viscosity->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_viscosity, s_row, 0 );
	str.setNum( viscosity );
	QLineEdit* le_viscosity = us_lineedit( str.setNum( viscosity ));
   left->addWidget( le_viscosity, s_row++, 1 );
	connect( le_viscosity, SIGNAL( textChanged( const QString& ) ),
			   SLOT( update_viscosity( const QString& ) ) );

   pb_add_partialGrid       = us_pushbutton( tr( "Add this Grid" ) );
   pb_add_partialGrid->setEnabled( true );
   left->addWidget( pb_add_partialGrid, s_row, 0 );
   connect( pb_add_partialGrid,    SIGNAL( clicked() ),
            this,       SLOT( add_partialGrid() ) );

	QGridLayout *showgrid = us_checkbox( tr("Show Final Grid"), cb_show_final_grid, false );
	left->addLayout( showgrid, s_row++, 1 );
	connect( cb_show_final_grid, SIGNAL( clicked(bool) ),
				this, 		SLOT( show_final_grid(bool) ) );
	cb_show_final_grid->setEnabled( false );

   pb_delete_partialGrid       = us_pushbutton( tr( "Delete Partial Grid" ) );
   pb_delete_partialGrid->setEnabled( false );
   left->addWidget( pb_delete_partialGrid, s_row, 0 );
   connect( pb_delete_partialGrid,    SIGNAL( clicked() ),
            this,       SLOT( delete_partialGrid() ) );

	QGridLayout *showsubgrid = us_checkbox( tr("Show Subgrids"), cb_show_sub_grid, false );
	left->addLayout( showsubgrid, s_row++, 1 );
	connect( cb_show_sub_grid, SIGNAL( clicked(bool) ),
				this, 		SLOT( show_sub_grid(bool) ) );
	cb_show_sub_grid->setEnabled( false );

   lbl_partialGrid     = us_label( tr( "Highlight Partial Grid #:" ) );
   lbl_partialGrid->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_partialGrid, s_row, 0 );

   ct_partialGrid     = us_counter( 3, 0, 1000, 0.0 );
   ct_partialGrid->setStep( 1 );
   ct_partialGrid->setEnabled( false );
   left->addWidget( ct_partialGrid, s_row++, 1 );
   connect( ct_partialGrid, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_partialGrid( double ) ) );

   lbl_subGrid     = us_label( tr( "Number of Subgrids:" ) );
   lbl_subGrid->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );
   left->addWidget( lbl_subGrid, s_row, 0 );

   ct_subGrids     = us_counter( 3, 1, 100, 13.0 );
   ct_subGrids->setStep( 1 );
   ct_subGrids->setEnabled( false );
   left->addWidget( ct_subGrids, s_row++, 1 );
   connect( ct_subGrids, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_subGrids( double ) ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   left->addWidget( pb_reset, s_row, 0 );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   left->addWidget( pb_save, s_row++, 1 );
   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT(   save() ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   left->addWidget( pb_help, s_row, 0 );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT(   help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   left->addWidget( pb_close, s_row++, 1 );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close() ) );

   // set up plot component window on right side

   QBoxLayout* plot1 = new US_Plot( data_plot1,
      tr( "Grid Layout" ),
      tr( "Sedimentation Coefficient (s20,W)"),
      tr( "Frictional Ratio f/f0" ) );
   right->addLayout( plot1 );
   data_plot1->setAutoDelete( true );
   data_plot1->setMinimumSize( 640, 480 );
   data_plot1->enableAxis( QwtPlot::xBottom, true );
   data_plot1->enableAxis( QwtPlot::yLeft,   true );
   data_plot1->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot1->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   main->addLayout( left );
   main->addLayout( right );
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( right, 6 );
   reset();
}

// reset the GUI
void US_Grid_Editor::reset( void )
{
   data_plot1->detachItems( );
   pick1 = new US_PlotPicker( data_plot1 );

   xRes = 60.0;
   yRes = 60.0;
   yMin = 1.0;
   yMax = 4.0;
   xMin = 1.0;
   xMax = 10.0;
	zVal = 0.72;
	vbar = 0.72;
	ff0  = 1.0;
	plot_x = 0; // plot s
	plot_y = 0; // plot f/f0
	selected_plot = 0;
	viscosity = VISC_20W;
	density = DENS_20W;
	grid_index = 0;
	partialGrid = 0;
	subGrids=13;
	final_grid.clear();

   ct_xRes->setRange( 10.0, 1000.0, 1.0 );
   ct_xRes->setValue( (double) xRes );
   ct_yRes->setRange( 10.0, 1000.0, 1.0 );
   ct_yRes->setValue( (double) yRes );
	ct_partialGrid->setEnabled( false );
	ct_subGrids->setEnabled( false );
	ct_partialGrid->setRange( 0, 0, 0);
	ct_partialGrid->setValue( 0 );
	cb_show_final_grid->setEnabled( false );
	cb_show_final_grid->setChecked( false );
	cb_show_sub_grid->setEnabled( false );
   cb_show_sub_grid->setChecked( false );
	ct_xRes->setEnabled( true );
	ct_yRes->setEnabled( true );
	ct_xMin->setEnabled( true );
	ct_yMin->setEnabled( true );
	ct_xMax->setEnabled( true );
	ct_yMax->setEnabled( true );
	ct_zVal->setEnabled( true );
	rb_x_s->setEnabled( true );
	rb_x_s->setChecked( true );
	rb_x_mw->setEnabled( true );
   rb_y_ff0->setEnabled( true );
   rb_y_ff0->setChecked( true );
   rb_y_vbar->setEnabled( true );
	rb_plot1->setChecked(true);
	pb_add_partialGrid->setEnabled( true );
	select_x_axis(plot_x);
	select_y_axis(plot_y);
	select_y_axis(plot_y);
	select_plot(selected_plot);
   pb_save->setEnabled( false );
   pb_delete_partialGrid->setEnabled( false );
	update_plot();
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
   double          gridinc = 1.0 / (double)subGrids;
   int             indexsg = 1;
	bool flag;
	modelGuid         = US_Util::new_guid();
	model.analysis    = US_Model::INITIALGRID;
	model.description = "InitialGrid-" + now_time.toString( "MMddyyyy-hhmm") + ".model";
	model.subGrids    = subGrids;
   model.modelGUID   = modelGuid;
   model.global      = US_Model::NONE;
	sc.signal_concentration = 1.0;
	for (int i=0; i<final_grid.size(); i++)
	{
		flag      = true;
		sc.s      = final_grid[i].s * 1.0e-13;
		sc.D      = final_grid[i].D;
		sc.f      = final_grid[i].f;
		sc.f_f0   = final_grid[i].ff0;
		sc.vbar20 = final_grid[i].vbar;
		sc.mw     = final_grid[i].mw;

      double xval = ( plot_x == 0 ) ? sc.s : sc.mw;
      double yval = ( plot_y == 0 ) ? sc.f_f0 : sc.vbar20;
      int indexx = xvals.indexOf( xval );
      if ( indexx < 0 )  { indexx = xvals.size(); xvals << xval; }
      int indexy = yvals.indexOf( yval );
      if ( indexy < 0 )  { indexy = yvals.size(); yvals << yval; }
      sc.name   = QString().sprintf( "X%3.3dY%3.3d", indexx + 1, indexy + 1 );
      sc.signal_concentration = gridinc * (double)indexsg;
      if ( (++indexsg) > subGrids )  indexsg = 1;

		for (int j=0; j<model.components.size(); j++)
		{
			if (sc.s      == model.components[j].s    &&
		 	 	 sc.f_f0   == model.components[j].f_f0 &&
		 		 sc.vbar20 == model.components[j].vbar20 )
			{
				flag = false;
				break; // don't add a component that is already in the model
			}
		}

		if (flag) model.components.push_back(sc);
	}

   // Open a dialog that reports and allows modification of description
   QMessageBox mbox;
   QString msg1    = tr( "A grid model has been created. "
                         "It's description is:<br/><b>" )
      + model.description + "</b>.<br/><br/>"
      + tr( "Click:<br/><br/>" )
      + tr( "  <b>OK</b>     to output the model as is;<br/>"
            "  <b>Edit</b>   to modify the model description;<br/>"
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
      int     jj      = model.description.indexOf( ".model" );
      if ( jj > 0 ) model.description = model.description.left( jj );
      QString msg2    = tr( "The default run ID for the grid model<br/>"
                            "is <b>" ) + model.description + "</b>.<br/><br/>"
         + tr( "You may modify this part of the model description.<br/>"
               "Use alphanumeric characters, underscores, or hyphens<br/>"
               "(no spaces). Enter 3 to 32 characters." );
      model.description = QInputDialog::getText( this,
            tr( "Modify Model Name" ),
            msg2,
            QLineEdit::Normal,
            model.description,
            &ok );

      if ( !ok )  return;

      model.description.remove( QRegExp( "[^\\w\\d_-]" ) );
      int slen = model.description.length();
      if ( slen > 32 ) model.description = model.description.left( 32 );
      model.description = model.description + ".model";
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
      QString fnamo = US_Model::get_filename( modelPath, modelGuid );
      code = model.write( fnamo );
   }
   mbox.setWindowTitle( tr( "Grid Model Saving..." ) );
	mbox.removeButton(pb_edit);
	mbox.removeButton(pb_canc);
	QString str;
	if (code == US_DB2::OK)
	{
		mbox.setText ( tr("The file ") +  model.description + tr(" was successfully saved.") );
	}
	else
	{
		mbox.setText ( tr("Writing the model file  ") +  model.description + tr(" resulted in error code ") + str.arg(code) );
	}
	mbox.exec();
}

// update raster x resolution
void US_Grid_Editor::update_xRes( double dval )
{
   xRes  = dval;
	update_plot();
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
   if (plot_x == 0) // x axis is for sed. coeff.
   {
      ct_xMax->setRange(xMin, 10000.0, 1);
   }
   else if (plot_x == 1) // x axis is for mw.
   {
      ct_xMax->setRange(xMin, 1e10, 100);
   }
	update_plot();
}

// update plot limit x max
void US_Grid_Editor::update_xMax( double dval )
{
   xMax    = dval;
   if (plot_x == 0) // x axis is for sed. coeff.
   {
      ct_xMin->setRange(-100000, xMax, 1);
   }
   else if (plot_x == 1) // x axis s for mw.
   {
      ct_xMin->setRange(0.0, xMax, 1);
   }
	update_plot();
}

// update plot limit y min
void US_Grid_Editor::update_yMin( double dval )
{
   yMin    = dval;
   if (plot_y == 0) // y axis is for f/f0.
   {
      ct_yMax->setRange(yMin, 50.0, 0.1);
   }
   else if (plot_y == 1) // x axis is for vbar.
   {
      ct_yMax->setRange(yMin, 10.0, 100);
   }
	update_plot();
}

// update plot limit y max
void US_Grid_Editor::update_yMax( double dval )
{
   yMax    = dval;
   if (plot_y == 0) // y axis is for f/f0.
   {
      ct_yMin->setRange(1.0, yMax, 0.1);
   }
   else if (plot_y == 1) // x axis is for vbar.
   {
      ct_yMin->setRange(0.01, yMax, 0.01);
   }
	update_plot();
}

// update plot limit z-value (f/f0 or vbar)
void US_Grid_Editor::update_zVal( double dval )
{
   zVal    = dval;
	if (plot_y == 0)
	{
		vbar = zVal;
	}
	if (plot_y == 1)
	{
		ff0 = zVal;
	}
	update_plot();
}

// Select a partialGrid from all grids combined in the final grid for highlighting:
void US_Grid_Editor::update_partialGrid( double dval )
{
	partialGrid = (int) dval;
	update_plot();
}

// Select a subgrid from the final grid for highlighting:
void US_Grid_Editor::update_subGrids( double dval )
{
	subGrids = (int) dval;
	ct_partialGrid->setRange( 1, subGrids, 1);
	update_plot();
}

// update density
void US_Grid_Editor::update_density( const QString & str )
{
   density = str.toDouble();
	update_plot();
}

// update viscosity
void US_Grid_Editor::update_viscosity( const QString & str )
{
   viscosity = str.toDouble();
	update_plot();
}

// update plot
void US_Grid_Editor::update_plot( void )
{
	calc_gridpoints();
	data_plot1->clear();
	int gridsize;
	QVector <double> xData1;
	QVector <double> yData1;
	QVector <double> xData2;
	QVector <double> yData2;

	xData1.clear();
	yData1.clear();
	xData2.clear();
	yData2.clear();
	if (cb_show_final_grid->isChecked() && !cb_show_sub_grid->isChecked())
	{
		gridsize = final_grid.size();
		if (plot_x == 0 && plot_y == 0) //grid over s and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].ff0);
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].ff0);
					}
				}
			}
		}
		if (plot_x == 0 && plot_y == 1) //grid over s and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].vbar);
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].vbar);
					}
				}
			}
		}
		if (plot_x == 1 && plot_y == 0) //grid over mw and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].ff0);
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].ff0);
					}
				}
			}
		}
		if (plot_x == 1 && plot_y == 1) //grid over mw and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].vbar);
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(final_grid[i].index == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].vbar);
					}
				}
			}
		}
		QwtPlotCurve *c1;
		QwtSymbol sym1;
		sym1.setStyle( QwtSymbol::Ellipse );
		sym1.setBrush( QColor( Qt::red ) );
		sym1.setPen  ( QColor( Qt::red ) );
		sym1.setSize( 3 );

		c1 = us_curve( data_plot1, "highlighted Grid points" );
		c1->setData  ( xData1.data(), yData1.data(), xData1.size());
		c1->setSymbol( sym1 );
		c1->setStyle ( QwtPlotCurve::NoCurve );

		QwtPlotCurve *c2;
		QwtSymbol sym2;
		sym2.setStyle( QwtSymbol::Ellipse );
		sym2.setBrush( QColor( Qt::yellow ) );
		sym2.setPen  ( QColor( Qt::yellow ) );
		sym2.setSize( 3 );

		c2 = us_curve( data_plot1, "Other Grid points" );
		c2->setData  ( xData2.data(), yData2.data(), xData2.size());
		c2->setSymbol( sym2 );
		c2->setStyle ( QwtPlotCurve::NoCurve );
	}
	else if (cb_show_final_grid->isChecked() && cb_show_sub_grid->isChecked())
	{
		gridsize = final_grid.size();
		int counter=1;
		if (plot_x == 0 && plot_y == 0) //grid over s and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].ff0);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].ff0);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
		}
		if (plot_x == 0 && plot_y == 1) //grid over s and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].vbar);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].vbar);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
		}
		if (plot_x == 1 && plot_y == 0) //grid over mw and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].ff0);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].ff0);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].ff0);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
		}
		if (plot_x == 1 && plot_y == 1) //grid over mw and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].s);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].s);
						yData2.push_back(final_grid[i].vbar);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					if(counter == partialGrid)
					{
						xData1.push_back(final_grid[i].mw);
						yData1.push_back(final_grid[i].vbar);
					}
					else
					{
						xData2.push_back(final_grid[i].mw);
						yData2.push_back(final_grid[i].vbar);
					}
					counter++;
					if (counter == subGrids + 1)
					{
						counter = 1;
					}
				}
			}
		}
		QwtPlotCurve *c1;
		QwtSymbol sym1;
		sym1.setStyle( QwtSymbol::Ellipse );
		sym1.setBrush( QColor( Qt::red ) );
		sym1.setPen  ( QColor( Qt::red ) );
		sym1.setSize( 3 );

		c1 = us_curve( data_plot1, "highlighted Grid points" );
		c1->setData  ( xData1.data(), yData1.data(), xData1.size());
		c1->setSymbol( sym1 );
		c1->setStyle ( QwtPlotCurve::NoCurve );

		QwtPlotCurve *c2;
		QwtSymbol sym2;
		sym2.setStyle( QwtSymbol::Ellipse );
		sym2.setBrush( QColor( Qt::yellow ) );
		sym2.setPen  ( QColor( Qt::yellow ) );
		sym2.setSize( 3 );

		c2 = us_curve( data_plot1, "Other Grid points" );
		c2->setData  ( xData2.data(), yData2.data(), xData2.size());
		c2->setSymbol( sym2 );
		c2->setStyle ( QwtPlotCurve::NoCurve );
	}
	else
	{
		gridsize = current_grid.size();
		xData1.resize(gridsize);
		yData1.resize(gridsize);
		if (plot_x == 0 && plot_y == 0) //grid over s and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].s;
					yData1 [i] = current_grid[i].ff0;
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].mw;
					yData1 [i] = current_grid[i].ff0;
				}
			}
		}
		if (plot_x == 0 && plot_y == 1) //grid over s and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].s;
					yData1 [i] = current_grid[i].vbar;
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].mw;
					yData1 [i] = current_grid[i].vbar;
				}
			}
		}
		if (plot_x == 1 && plot_y == 0) //grid over mw and f/f0
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].s;
					yData1 [i] = current_grid[i].ff0;
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].mw;
					yData1 [i] = current_grid[i].ff0;
				}
			}
		}
		if (plot_x == 1 && plot_y == 1) //grid over mw and vbar
		{
			if (selected_plot == 0)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].s;
					yData1 [i] = current_grid[i].vbar;
				}
			}
			if (selected_plot == 1)
			{
				for (int i=0; i<gridsize; i++)
				{
					xData1 [i] = current_grid[i].mw;
					yData1 [i] = current_grid[i].vbar;
				}
			}
		}
		QwtPlotCurve *c1;
		QwtSymbol sym;
		sym.setStyle( QwtSymbol::Ellipse );
		sym.setBrush( QColor( Qt::yellow ) );
		sym.setPen  ( QColor( Qt::yellow ) );
		sym.setSize( 3 );

		c1 = us_curve( data_plot1, "Grid points 1" );
		c1->setData  ( xData1.data(), yData1.data(), current_grid.size());
		c1->setSymbol( sym );
		c1->setStyle ( QwtPlotCurve::NoCurve );
	}
	data_plot1->setAxisAutoScale( QwtPlot::xBottom );
	data_plot1->setAxisAutoScale( QwtPlot::yLeft );
	data_plot1->replot();
}

// calculate all grid points for the selected setting
void US_Grid_Editor::calc_gridpoints( void )
{
	current_grid.clear();
	bool flag = true;
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
	if (plot_x == 0 && plot_y == 0) //grid over s and f/f0
	{
		double s_inc = (xMax - xMin)/(xRes-1);
		double ff0_inc = (yMax - yMin)/(yRes-1);
		for (int i=0; i< (int) xRes; i++)
		{
			tmp_point.s    = xMin + i * s_inc;
			for (int j=0; j< (int) yRes; j++)
			{
				tmp_point.ff0  = yMin + j * ff0_inc;
				tmp_point.D    = R * K20 /( AVOGADRO * 18 * M_PI *
				                 pow((viscosity * 0.01 * tmp_point.ff0), (3.0/2.0)) *
			  	                 pow((tmp_point.s * 1.0e-13 * vbar
									/ (2.0 * (1.0 - vbar * density))), 0.5));
//
// check to make sure there aren't any nonsensical settings selected by
// the user. If so, mark the molecular weight negative (-1) and exclude
// the point from the grid point list:
//
				if ((1 - vbar * density) == 0 ||
			  		(tmp_point.s < 0 && (1 - vbar * density) > 0 ) ||
			  		(tmp_point.s > 0 && (1 - vbar * density) < 0 ))
				{
					tmp_point.mw = -1.0;
					flag = false;
				}
				else
				{
					tmp_point.mw   = tmp_point.s * 1.0e-13 * R * K20
							         / (tmp_point.D * (1 - vbar * density ));
				}
				tmp_point.f    = R * K20 / (AVOGADRO * tmp_point.D);
				tmp_point.f0   = tmp_point.f/tmp_point.ff0;
				tmp_point.vbar = vbar;
				if ((tmp_point.s < -0.1 || tmp_point.s > 0.1) && tmp_point.mw > 0)
				{
					current_grid.push_back(tmp_point);
				}
				set_minmax(tmp_point);
			}
		}
	}
	if (plot_x == 1 && plot_y == 0) //grid over MW and f/f0
	{
		double mw_inc = (xMax - xMin)/(xRes - 1);
		double ff0_inc = (yMax - yMin)/(yRes - 1);
		for (int i=0; i< (int) xRes; i++)
		{
			tmp_point.mw   = xMin + i * mw_inc;
			for (int j=0; j< (int) yRes; j++)
			{
				tmp_point.ff0  = yMin + j * ff0_inc;
				tmp_point.f0   = viscosity * 0.01 * pow((162 * tmp_point.mw * M_PI * M_PI
								   * vbar/AVOGADRO), (1.0/3.0));
				tmp_point.f    = tmp_point.ff0 * tmp_point.f0;
				tmp_point.s    = 1.0e13 * tmp_point.mw * (1.0 - vbar * density )
						  			/ (AVOGADRO * tmp_point.f);
				tmp_point.D    = R * K20/(AVOGADRO * tmp_point.f);
				tmp_point.vbar = vbar;
				if (tmp_point.s < -0.1 || tmp_point.s > 0.1)
				{
					current_grid.push_back(tmp_point);
				}
				set_minmax(tmp_point);
			}
		}
	}
	if (plot_x == 0 && plot_y == 1) //grid over s and vbar
	{
		double s_inc = (xMax - xMin)/(xRes - 1);
		double vbar_inc = (yMax - yMin)/(yRes - 1);
		for (int i=0; i< (int) xRes; i++)
		{
			tmp_point.s    = xMin + i * s_inc;
			for (int j=0; j< (int) yRes; j++)
			{
				tmp_point.vbar = yMin + j * vbar_inc;
				tmp_point.D    = R * K20 /( AVOGADRO * 18 * M_PI *
				                 pow((viscosity * 0.01 * ff0), (3.0/2.0)) *
			  	                 pow((tmp_point.s * 1.0e-13 * tmp_point.vbar
									/ (2 * (1.0 - vbar * density))), 0.5));
//
// check to make sure there aren't any nonsensical settings selected by
// the user. If so, mark the molecular weight negative (-1) and exclude
// the point from the grid point list:
//
				if ((1.0 - tmp_point.vbar * density) == 0 ||
			  		(tmp_point.s < 0 && (1.0 - tmp_point.vbar * density) > 0 ) ||
			  		(tmp_point.s > 0 && (1.0 - tmp_point.vbar * density) < 0 ))
				{
					tmp_point.mw = -1.0;
					flag = false;
				}
				else
				{
					tmp_point.mw   = tmp_point.s * 1.0e-13 * R * K20
							  			/ (tmp_point.D * (1.0 - tmp_point.vbar * density ));
				}
				tmp_point.f0   = viscosity * 0.01 * pow((162 * tmp_point.mw * M_PI * M_PI
								   * vbar/AVOGADRO), (1.0/3.0));
				tmp_point.f    = R * K20 / (AVOGADRO * tmp_point.D);
				tmp_point.ff0  = tmp_point.f / tmp_point.f0;
				if ((tmp_point.s < -0.1 || tmp_point.s > 0.1) && tmp_point.mw > 0)
				{
					current_grid.push_back(tmp_point);
				}
				set_minmax(tmp_point);
			}
		}
	}
	if (plot_x == 1 && plot_y == 1) //grid over MW and vbar
	{
		double mw_inc = (xMax - xMin)/(xRes - 1);
		double vbar_inc = (yMax - yMin)/(yRes - 1);
		for (int i=0; i< (int) xRes; i++)
		{
			tmp_point.mw   = xMin + i * mw_inc;
			for (int j=0; j< (int) yRes; j++)
			{
				tmp_point.vbar = yMin + j * vbar_inc;
				tmp_point.ff0  = ff0;
				tmp_point.f0   = viscosity * 0.01 * pow((162 * tmp_point.mw * M_PI * M_PI
								   * tmp_point.vbar / AVOGADRO), (1.0/3.0));
				tmp_point.f    = tmp_point.ff0 * tmp_point.f0;
				tmp_point.s    = 1.0e13 * tmp_point.mw * (1.0 - tmp_point.vbar * density )
						  			/ (AVOGADRO * tmp_point.f);
				tmp_point.D    = R * K20/(AVOGADRO * tmp_point.f);
				if (tmp_point.s < -0.1 || tmp_point.s > 0.1)
				{
					current_grid.push_back(tmp_point);
				}
				set_minmax(tmp_point);
			}
		}
	}
	if (flag == false)
	{
		int status = QMessageBox::information( this,
		tr( "Warning" ),
		tr( "You have selected a nonsensical parameter setting. The product " ) +
		tr( "of the sedimentation coefficient, s, and the buoyancy term, "    ) +
	   tr( "(1 - vbar * Density), must be positive. Please examine your "    ) +
		tr( "settings for the sedimentation coefficient range, the density, " ) +
		tr( "and the partial specific volume before proceeding." ),
		tr( "&OK" ), tr( "&Cancel" ),
		0, 0, 1 );
		if ( status != 0 ) return;
	}
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
	cb_show_final_grid->setEnabled( true );
   pb_delete_partialGrid->setEnabled( true );
   ct_partialGrid->setEnabled( true );
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
   ct_partialGrid->setRange( 1, grid_index, 1 );
   if (grid_index == 0)
   {
      cb_show_final_grid->setChecked (false);
      cb_show_final_grid->setEnabled (false);
      ct_partialGrid->setRange (0, 0, 1);
      ct_partialGrid->setEnabled( false );
      show_final_grid( false );
   }
   update_plot();
}

// find the minimum and maximum in a grid
void US_Grid_Editor::set_minmax( const struct gridpoint & tmp_point)
{
	mingridpoint.s    = min(mingridpoint.s, tmp_point.s);
	mingridpoint.D    = min(mingridpoint.D, tmp_point.D);
	mingridpoint.vbar = min(mingridpoint.vbar, tmp_point.vbar);
	mingridpoint.mw   = min(mingridpoint.mw, tmp_point.mw);
	mingridpoint.ff0  = min(mingridpoint.ff0, tmp_point.ff0);
	mingridpoint.f0   = min(mingridpoint.f0, tmp_point.f0);
	mingridpoint.f    = min(mingridpoint.f, tmp_point.f);
   maxgridpoint.s    = max(maxgridpoint.s, tmp_point.s);
	maxgridpoint.D    = max(maxgridpoint.D, tmp_point.D);
	maxgridpoint.vbar = max(maxgridpoint.vbar, tmp_point.vbar);
	maxgridpoint.mw   = max(maxgridpoint.mw, tmp_point.mw);
	maxgridpoint.ff0  = max(maxgridpoint.ff0, tmp_point.ff0);
	maxgridpoint.f0   = max(maxgridpoint.f0, tmp_point.f0);
	maxgridpoint.f    = max(maxgridpoint.f, tmp_point.f);
}

// select coordinate for horizontal axis
void US_Grid_Editor::select_plot( int ival )
{
	selected_plot = ival;
	if (selected_plot == 0)
	{
		data_plot1->setAxisTitle( QwtPlot::xBottom, tr("Sedimentation Coefficient (s20,W)"));
	}
	if (selected_plot == 1)
	{
		data_plot1->setAxisTitle( QwtPlot::xBottom, tr("Molecular Weight (Dalton)"));
	}
	update_plot();
}

// select coordinate for horizontal axis
void US_Grid_Editor::select_x_axis( int ival )
{
	plot_x = ival;
	switch (plot_x)
	{
		case 0:
		{
			lbl_xRes->setText(tr("s-value Resolution:"));
			lbl_xMin->setText(tr("s-value Minimum:"));
			lbl_xMax->setText(tr("s-value Maximum:"));
			ct_xMin->disconnect();
			ct_xMax->disconnect();
   		ct_xMin->setRange( -10000.0, 10000.0, 0.01 );
			xMin = 1;
   		ct_xMin->setValue( xMin );
   		ct_xMax->setRange( -10000.0, 10000.0, 0.01 );
			xMax = 10;
  	  		ct_xMax->setValue( xMax );
   		connect( ct_xMin, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_xMin( double ) ) );
   		connect( ct_xMax, SIGNAL( valueChanged( double ) ),
            this,        SLOT( update_xMax( double ) ) );
			break;
		}
		case 1:
		{
			lbl_xRes->setText(tr("Mol. Weight Resolution:"));
			lbl_xMin->setText(tr("Mol. Weight Minimum:"));
			lbl_xMax->setText(tr("Mol. Weight Maximum:"));
			ct_xMin->disconnect();
			ct_xMax->disconnect();
      	ct_xMin->setRange( 0.0, 1e10, 10 );
			xMin = 1.0e4;
      	ct_xMin->setValue( xMin );
      	ct_xMax->setRange( 0.0, 1e10, 10 );
			xMax = 1.0e5;
  	  	   ct_xMax->setValue( xMax );
      	connect( ct_xMin, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_xMin( double ) ) );
      	connect( ct_xMax, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_xMax( double ) ) );
			break;
		}
	}
	update_plot();
}

// select coordinate for vertical axis
void US_Grid_Editor::select_y_axis( int ival )
{
	plot_y = ival;
	switch (plot_y)
	{
		case 0:
		{
			data_plot1->setAxisTitle( QwtPlot::yLeft, tr("Frictional Ratio f/f0"));
			lbl_yRes->setText(tr("f/f0 Resolution:"));
			lbl_yMin->setText(tr("f/f0 Minimum:"));
			lbl_yMax->setText(tr("f/f0 Maximum:"));
			lbl_zVal->setText(tr("Partial Spec. Volume:"));
			ct_yMin->disconnect();
			ct_yMax->disconnect();
			ct_zVal->disconnect();
   		ct_yMin->setRange( 1.0, 50, 0.01 );
			yMin = 1;
   		ct_yMin->setValue( yMin );
   		ct_yMax->setRange( 1.0, 50, 0.01 );
			yMax = 4;
   		ct_yMax->setValue( yMax );
   		ct_zVal->setRange( 0.01, 3, 0.001 );
			zVal = vbar;
   		ct_zVal->setValue( zVal );
      	connect( ct_yMin, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_yMin( double ) ) );
      	connect( ct_yMax, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_yMax( double ) ) );
      	connect( ct_zVal, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_zVal( double ) ) );
			break;
		}
		case 1:
		{
			vbar = 0.72;
			data_plot1->setAxisTitle( QwtPlot::yLeft, tr("Partial Specific Volume (ml/mg)"));
			lbl_yRes->setText(tr("vbar Resolution:"));
			lbl_yMin->setText(tr("vbar Minimum:"));
			lbl_yMax->setText(tr("vbar Maximum:"));
			lbl_zVal->setText(tr("Frictional Ratio:"));
			ct_yMin->disconnect();
			ct_yMax->disconnect();
			ct_zVal->disconnect();
   		ct_yMin->setRange( 0.01, 3, 0.001 );
			yMin = 0.1;
   		ct_yMin->setValue( yMin );
   		ct_yMax->setRange( 0.02, 3, 0.001 );
			yMax = 1.0;
   		ct_yMax->setValue( yMax );
   		ct_zVal->setRange( 1.0, 50, 0.01 );
			zVal = ff0;
   		ct_zVal->setValue( zVal );
      	connect( ct_yMin, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_yMin( double ) ) );
      	connect( ct_yMax, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_yMax( double ) ) );
      	connect( ct_zVal, SIGNAL( valueChanged( double ) ),
               this,        SLOT( update_zVal( double ) ) );
			break;
		}
	}
	update_plot();
}

// activated when the "Show Final Grid" Checkbox is set
void US_Grid_Editor::show_final_grid( bool flag )
{
	if (flag)
	{
		ct_xRes->setEnabled( false );
		ct_yRes->setEnabled( false );
		ct_xMin->setEnabled( false );
		ct_yMin->setEnabled( false );
		ct_xMax->setEnabled( false );
		ct_yMax->setEnabled( false );
		ct_zVal->setEnabled( false );
		rb_x_s->setEnabled( false );
		rb_x_mw->setEnabled( false );
      rb_y_ff0->setEnabled( false );
      rb_y_vbar->setEnabled( false );
		ct_partialGrid->setRange(1, grid_index, 1);
		ct_partialGrid->setEnabled( true );
      pb_delete_partialGrid->setEnabled( true );
      pb_add_partialGrid->setEnabled( false );
      cb_show_sub_grid->setEnabled( true );
		ct_subGrids->setEnabled( false );
	}
	else
	{
		ct_xRes->setEnabled( true );
		ct_yRes->setEnabled( true );
		ct_xMin->setEnabled( true );
		ct_yMin->setEnabled( true );
		ct_xMax->setEnabled( true );
		ct_yMax->setEnabled( true );
		ct_zVal->setEnabled( true );
		rb_x_s->setEnabled( true );
		rb_x_mw->setEnabled( true );
      rb_y_ff0->setEnabled( true );
      rb_y_vbar->setEnabled( true );
		ct_partialGrid->setEnabled( false );
      pb_delete_partialGrid->setEnabled( false );
      pb_add_partialGrid->setEnabled( true );
      cb_show_sub_grid->setEnabled( false );
		ct_subGrids->setEnabled( false );
	}
	update_plot();
}

// activated when the "Show Subgrids" Checkbox is set
void US_Grid_Editor::show_sub_grid( bool flag )
{
	if (flag)
	{
		int maxsubgrids = (int) final_grid.size()/50;
		lbl_partialGrid->setText("Highlight Subgrid #:");
		ct_subGrids->setEnabled( true );
		ct_subGrids->setRange(1, maxsubgrids, 1);
		ct_partialGrid->setRange( 1, subGrids, 1);
		pb_delete_partialGrid->setEnabled( false );
	}
	else
	{
		lbl_partialGrid->setText("Highlight Partial Grid #:");
		ct_subGrids->setEnabled( false );
		ct_partialGrid->setRange( 1, grid_index, 1);
		ct_partialGrid->setValue( 1 );
		pb_delete_partialGrid->setEnabled( true );
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
