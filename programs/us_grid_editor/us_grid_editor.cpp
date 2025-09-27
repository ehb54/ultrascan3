//! \file us_ga_init.cpp

#include "us_grid_editor.h"
#include "us_matrix.h"
#include "us_csv_loader.h"
#include <cmath>

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
   QDoubleValidator *dValid = new QDoubleValidator( this );
   dValid->setLocale( QLocale::C );
   QIntValidator    *iValid = new QIntValidator( this );
   iValid->setBottom( 1 );

   // primary layouts
   QHBoxLayout* main  = new QHBoxLayout( this );
   QVBoxLayout* right = new QVBoxLayout();
   QGridLayout* left  = new QGridLayout();
   main->setSpacing( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing( 2 );
   left->setContentsMargins( 0, 0, 0, 0 );
   right->setSpacing( 0 );
   right->setContentsMargins( 0, 0, 0, 0 );

   QLabel *lb_in_out = us_banner( tr( "Input / Output Ctrl" ) );

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

   // select bewteen cg & csv modes
   rb_cg_mode  = new QRadioButton();
   rb_csv_mode = new QRadioButton();
   QGridLayout* lyt_cg_mode  = us_radiobutton( "Create Grid", rb_cg_mode );
   QGridLayout* lyt_csv_mode = us_radiobutton( "Load Grid", rb_csv_mode );

   QHBoxLayout* lyt_grid_mode = new QHBoxLayout();
   lyt_grid_mode->setContentsMargins( 0, 0, 0, 0 );
   lyt_grid_mode->setSpacing( 0 );
   lyt_grid_mode->addLayout( lyt_cg_mode );
   lyt_grid_mode->addLayout( lyt_csv_mode );

   QButtonGroup* bg_mode = new QButtonGroup( this );
   bg_mode->addButton( rb_cg_mode );
   bg_mode->addButton( rb_csv_mode );
   rb_cg_mode->setChecked( true );
   connect( bg_mode, &QButtonGroup::idClicked, this, &US_Grid_Editor::set_cg_mode );

   pb_load_csv = us_pushbutton( "Load Grid File ");
   connect( pb_load_csv, &QPushButton::clicked, this, &US_Grid_Editor::load_grid_file );

   // Experimental Space
   QLabel* lb_experm = us_banner( tr( "Experimental Space" ) );

   QLabel *lb_buffer = us_label( tr( "Description" ) );
   lb_buffer->setAlignment( Qt::AlignCenter );

   le_buffer = us_lineedit( "", -1, true );

   QLabel *lb_dens_T = us_label( tr( "ρ( T ) [g/mL]" ) );
   lb_dens_T->setAlignment( Qt::AlignCenter );
   le_dens_T = us_lineedit( "", -1, true );
   le_dens_T->setValidator( dValid );

   QLabel *lb_visc_T = us_label( tr( "η( T ) [cP]" ) );
   lb_visc_T->setAlignment( Qt::AlignCenter );
   le_visc_T = us_lineedit( "", -1, true );
   le_visc_T->setValidator( dValid );

   QLabel *lb_dens_20 = us_label( tr( "ρ( 20° ) [g/mL]" ) );
   lb_dens_20->setAlignment( Qt::AlignCenter );
   le_dens_20 = us_lineedit();
   le_dens_20->setValidator( dValid );

   QLabel *lb_visc_20 = us_label( tr( "η( 20° ) [cP]" ) );
   lb_visc_20->setAlignment( Qt::AlignCenter );
   le_visc_20 = us_lineedit();
   le_visc_20->setValidator( dValid );

   QLabel *lb_temp = us_label( tr( "T [°C]" ) );
   lb_temp->setAlignment( Qt::AlignCenter );
   le_temp = us_lineedit();
   le_temp->setValidator( dValid );

   pb_lu_buffer = us_pushbutton( "Load Buffer" );

   connect( le_dens_20,   &QLineEdit::editingFinished, this, &US_Grid_Editor::new_dens_visc_temp );
   connect( le_visc_20,   &QLineEdit::editingFinished, this, &US_Grid_Editor::new_dens_visc_temp );
   connect( le_temp,      &QLineEdit::editingFinished, this, &US_Grid_Editor::new_dens_visc_temp );
   connect( pb_lu_buffer, &QPushButton::clicked,       this, &US_Grid_Editor::set_buffer );

   // Grid Setup
   lb_grid_setup = us_banner( tr( "Grid Setup Control" ) );
   x_param = Attribute::ATTR_S;
   y_param = Attribute::ATTR_K;
   z_param = Attribute::ATTR_V;

   lb_x_param = us_label( tr( "X-Axis" ) );
   lb_x_param->setAlignment( Qt::AlignCenter );

   lb_y_param = us_label( tr( "Y-Axis" ) );
   lb_y_param->setAlignment( Qt::AlignCenter );

   lb_z_param = us_label( tr( "Fixed Attribute" ) );
   lb_z_param->setAlignment( Qt::AlignCenter );

   le_x_param = us_lineedit( Attribute::long_desc( x_param ), -1, true );
   le_y_param = us_lineedit( Attribute::long_desc( y_param ), -1, true );
   le_z_param = us_lineedit( Attribute::long_desc( z_param ), -1, true );

   pb_grid_setup = us_pushbutton( "Grid Setup" );
   connect ( pb_grid_setup, &QPushButton::clicked, this, &US_Grid_Editor::setup_grid );

   // Grid Mode Control
   lb_grid_mode = us_banner( "" );

   chkb_log = new QCheckBox();
   lyt_log = us_checkbox( "X-Axis Logarithmic", chkb_log );
   connect( chkb_log, &QCheckBox::stateChanged, this, &US_Grid_Editor::refill_grid_points );

   QRadioButton* rb_exctpoints = new QRadioButton();
   QRadioButton* rb_midpoints    = new QRadioButton();
   QGridLayout* lyt_end_points = us_radiobutton( "Start / End Points", rb_exctpoints );
   QGridLayout* lyt_mid_points = us_radiobutton( "Bin Centers", rb_midpoints );

   lyt_points = new QHBoxLayout();
   lyt_points->setContentsMargins( 0, 0, 0, 0 );
   lyt_points->setSpacing( 0 );
   lyt_points->addLayout( lyt_end_points );
   lyt_points->addLayout( lyt_mid_points );

   bg_point_type = new QButtonGroup();
   bg_point_type->addButton( rb_midpoints, MIDPOINTS );
   bg_point_type->addButton( rb_exctpoints, EXACTPOINTS );
   rb_exctpoints->setChecked( true );
   connect( bg_point_type, &QButtonGroup::idClicked, this, &US_Grid_Editor::set_mid_exct_points );

   lb_grid_list = us_label( "Partial Grid List" );
   lb_grid_list->setAlignment( Qt::AlignCenter );

   lw_grids = us_listwidget();

   te_info = new QTextEdit();
   te_info->setAcceptRichText( true );
   te_info->setReadOnly( true );
   QString text( "<b>Acceptable Units:</b><br/><br/>");
   text += tr( "<b>Sedimentation Coefficient [ s ]</b><br/>" );
   text += tr( "<b>Diffusion coefficient [ cm<sup>2</sup> / s ]</b><br/>" );
   text += tr( "<b>Molecular weight [ Da ]</b><br/>" );
   text += tr( "<b>Partial specific volume [ mL / g]</b><br/>" );
   text += tr( "<b>Frictional coefficient [ g / s]</b><br/>" );
   te_info->setText( text );
   te_info->setStyleSheet("QTextEdit {color: blue; background-color: lightyellow;}" );

   pb_new = us_pushbutton( "New" );
   connect( pb_new, &QPushButton::clicked, this, &US_Grid_Editor::new_grid_clicked );

   pb_delete = us_pushbutton( "Delete" );
   connect( pb_delete, &QPushButton::clicked, this, &US_Grid_Editor::delete_grid_clicked );

   pb_update = us_pushbutton( "Update" );
   connect( pb_update, &QPushButton::clicked, this, &US_Grid_Editor::update_grid_clicked );

   QHBoxLayout* lyt_1 = new QHBoxLayout();
   lyt_1->addWidget( pb_new );
   lyt_1->addWidget( pb_update );
   lyt_1->addWidget( pb_delete );

   hline1 = new QFrame();
   hline1->setFrameShape( QFrame::HLine );
   hline1->setFrameShadow( QFrame::Sunken );

   lb_min = us_label( "Minimum" );
   lb_min->setAlignment( Qt::AlignCenter );
   lb_max = us_label( "Maximum" );
   lb_max->setAlignment( Qt::AlignCenter );
   lb_res = us_label( "Resolution" );
   lb_res->setAlignment( Qt::AlignCenter );

   lb_x_ax = us_label( Attribute::short_desc( x_param ) );
   lb_x_ax->setAlignment( Qt::AlignCenter );
   lb_y_ax = us_label( Attribute::short_desc( y_param ) );
   lb_y_ax->setAlignment( Qt::AlignCenter );

   le_x_min = us_lineedit();
   le_x_max = us_lineedit();
   le_x_res = us_lineedit();
   le_x_res->setValidator( iValid );

   le_y_min = us_lineedit();
   le_y_max = us_lineedit();
   le_y_res = us_lineedit();
   le_y_res->setValidator( iValid );

   lb_z_ax = us_label( Attribute::short_desc( z_param ) );
   lb_z_ax->setAlignment( Qt::AlignCenter );
   le_z_val = us_lineedit();

   QGridLayout* lyt_cons_z = us_radiobutton( "Constant", rb_cons_z );
   QGridLayout* lyt_vary_z = us_radiobutton( "Varying" , rb_vary_z );
   lyt_zvalues = new QHBoxLayout();
   lyt_zvalues->setContentsMargins( 0, 0, 0, 0 );
   lyt_zvalues->setSpacing( 0 );
   lyt_zvalues->addLayout( lyt_cons_z );
   lyt_zvalues->addLayout( lyt_vary_z );

   QButtonGroup* z_cons_vary = new QButtonGroup( this );
   z_cons_vary->addButton( rb_cons_z, 0 );
   z_cons_vary->addButton( rb_vary_z, 1 );
   rb_cons_z->setChecked( true );

   pb_z_set_func = us_pushbutton( "Set" );
   set_zval_type( 0 );

   connect( le_x_min,  &QLineEdit::editingFinished, this, &US_Grid_Editor::new_xMin );
   connect( le_x_max,  &QLineEdit::editingFinished, this, &US_Grid_Editor::new_xMax );
   connect( le_y_min,  &QLineEdit::editingFinished, this, &US_Grid_Editor::new_yMin );
   connect( le_y_max,  &QLineEdit::editingFinished, this, &US_Grid_Editor::new_yMax );
   connect( le_z_val,  &QLineEdit::editingFinished, this, &US_Grid_Editor::new_zVal );
   connect( z_cons_vary  , &QButtonGroup::idClicked, this, &US_Grid_Editor::set_zval_type );
   connect( pb_z_set_func, &QPushButton::clicked   , this, &US_Grid_Editor::set_z_function );

   pb_add_update = us_pushbutton( "Add / Update " );
   connect( pb_add_update, &QPushButton::clicked, this, &US_Grid_Editor::add_update );

   lb_subgrid_ctrl = us_banner( "Subgrid Control" );

   QLabel *lb_nsubgrids = us_label( "Number of Subgrids" );
   lb_nsubgrids->setAlignment( Qt::AlignCenter );

   ct_nsubgrids = us_counter( 2, 1, 1000, 30 );
   ct_nsubgrids->setSingleStep( 1 );
   QLineEdit* le_nsg = ct_nsubgrids->findChild<QLineEdit*>();
   if ( le_nsg ) {
      le_nsg->setAlignment( Qt::AlignCenter );
   }

   QLabel *lb_subgrid = us_label( "Highlight Subgrid" );
   lb_subgrid->setAlignment( Qt::AlignCenter );

   ct_subgrid = us_counter( 2, 1, ct_nsubgrids->value(), 1 );
   ct_subgrid->setSingleStep( 1 );
   QLineEdit* le_sg = ct_subgrid->findChild<QLineEdit*>();
   if ( le_sg ) {
      le_sg->setAlignment( Qt::AlignCenter );
   }

   connect( ct_subgrid,   &QwtCounter::valueChanged, this, &US_Grid_Editor::plot_subgrid );
   connect( ct_nsubgrids, &QwtCounter::valueChanged, this, &US_Grid_Editor::set_nsubgrids );

   QLabel *lb_npoints = us_label( "Total Number of Points" );
   lb_npoints->setAlignment( Qt::AlignCenter );
   le_npoints = us_lineedit( "", -1, true );
   le_npoints->setAlignment( Qt::AlignCenter );

   QLabel *lb_npoints_curr = us_label( "Current Subgrid's Number of Points" );
   lb_npoints_curr->setAlignment( Qt::AlignCenter );
   le_npoints_curr = us_lineedit( "", -1, true );
   le_npoints_curr->setAlignment( Qt::AlignCenter );

   QLabel *lb_npoints_last = us_label( "Last Subgrid's Number of Points" );
   lb_npoints_last->setAlignment( Qt::AlignCenter );
   le_npoints_last = us_lineedit( "", -1, true );
   le_npoints_last->setAlignment( Qt::AlignCenter );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset, &QPushButton::clicked, this, &US_Grid_Editor::reset );

   QPushButton* pb_load_model = us_pushbutton( "Load Model" );
   connect( pb_load_model, &QPushButton::clicked, this, &US_Grid_Editor::load );

   pb_save = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   connect( pb_save, &QPushButton::clicked, this, &US_Grid_Editor::save );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   // connect( pb_help, &QPushButton::clicked, this, &US_Grid_Editor::help );

   QPushButton* pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close, &QPushButton::clicked, this, &US_Grid_Editor::close );

   QHBoxLayout* lyt_rsl = new QHBoxLayout();
   lyt_rsl->setContentsMargins( 0, 0, 0, 0 );
   lyt_rsl->setSpacing( 2 );
   lyt_rsl->addWidget( pb_reset );
   lyt_rsl->addWidget( pb_load_model );
   lyt_rsl->addWidget( pb_save );

   int row = 0;
   left->addWidget( lb_in_out,            row++, 0, 1, 4 );
   left->addWidget( pb_investigator,      row,   0, 1, 2 );
   left->addWidget( le_investigator,      row++, 2, 1, 2 );

   left->addLayout( dkdb_cntrls,          row++, 0, 1, 4 );

   left->addLayout( lyt_rsl,              row++, 0, 1, 4 );

   left->addWidget( lb_experm,            row++, 0, 1, 4 );

   left->addWidget( lb_temp,              row,   0, 1, 1 );
   left->addWidget( le_temp,              row,   1, 1, 1 );
   left->addWidget( pb_lu_buffer,         row++, 2, 1, 2 );

   left->addWidget( lb_buffer,            row,   0, 1, 1 );
   left->addWidget( le_buffer,            row++, 1, 1, 3 );

   left->addWidget( lb_dens_20,           row,   0, 1, 1 );
   left->addWidget( le_dens_20,           row,   1, 1, 1 );
   left->addWidget( lb_visc_20,           row,   2, 1, 1 );
   left->addWidget( le_visc_20,           row++, 3, 1, 1 );

   left->addWidget( lb_dens_T,            row,   0, 1, 1 );
   left->addWidget( le_dens_T,            row,   1, 1, 1 );
   left->addWidget( lb_visc_T,            row,   2, 1, 1 );
   left->addWidget( le_visc_T,            row++, 3, 1, 1 );

   left->addWidget( lb_grid_setup,        row++, 0, 1, 4 );

   left->addWidget( lb_x_param,           row,   0, 1, 2 );
   left->addWidget( le_x_param,           row++, 2, 1, 2 );

   left->addWidget( lb_y_param,           row,   0, 1, 2 );
   left->addWidget( le_y_param,           row++, 2, 1, 2 );

   left->addWidget( lb_z_param,           row,   0, 1, 2 );
   left->addWidget( le_z_param,           row++, 2, 1, 2 );

   left->addWidget( pb_grid_setup,        row++, 1, 1, 2 );

   left->addWidget( lb_grid_mode,         row++, 0, 1, 4 );

   left->addLayout( lyt_grid_mode,        row++, 0, 1, 4 );

   left->addWidget( pb_load_csv,          row++, 1, 1, 2 );

   left->addLayout( lyt_points,           row++, 0, 1, 4 );

   left->addLayout( lyt_log,              row++, 0, 1, 4 );

   left->addWidget( lb_grid_list,         row++, 0, 1, 4 );

   left->addWidget( lw_grids,             row,   0, 1, 4 );
   left->addWidget( te_info,              row,   0, 1, 4 );
   left->setRowStretch( row++, 1 );

   left->addLayout( lyt_1,                row++, 0, 1, 4 );

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

   left->addWidget( lb_z_ax,              row,   0, 1, 1 );
   left->addWidget( le_z_val,             row,   1, 1, 1 );
   left->addWidget( pb_z_set_func,        row,   1, 1, 1 );
   left->addLayout( lyt_zvalues,          row++, 2, 1, 2 );

   left->addWidget( pb_add_update,        row++, 1, 1, 2 );

   left->addWidget( lb_subgrid_ctrl,    row++, 0, 1, 4 );

   left->addWidget( lb_subgrid,           row,   0, 1, 2 );
   left->addWidget( ct_subgrid,           row++, 2, 1, 2 );

   left->addWidget( lb_nsubgrids,         row,   0, 1, 2 );
   left->addWidget( ct_nsubgrids,         row++, 2, 1, 2 );

   left->addWidget( lb_npoints,           row,   0, 1, 3 );
   left->addWidget( le_npoints,           row++, 3, 1, 1 );

   left->addWidget( lb_npoints_curr,      row,   0, 1, 3 );
   left->addWidget( le_npoints_curr,      row++, 3, 1, 1 );

   left->addWidget( lb_npoints_last,      row,   0, 1, 3 );
   left->addWidget( le_npoints_last,      row++, 3, 1, 1 );

   QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
   left->addItem( spacer, row++, 0, 1, 4 );

   left->addWidget( pb_help,              row,   0, 1, 2 );
   left->addWidget( pb_close,             row++, 2, 1, 2 );

   for ( int ii = 0; ii < 4; ii++ ) {
      left->setColumnStretch( ii, 1 );
   }

   // set up plot component window on right side
   QBoxLayout* plot_lyt = new US_Plot( data_plot,
                                      tr( "Grid Layout" ),
                                      tr( "" ),
                                      tr( "" ) );
   data_plot->setAutoDelete( true );
   data_plot->setMinimumSize( 640, 480 );
   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   // data_plot->setCanvasBackground( QBrush( Qt::black ) );
   data_plot->setCanvasBackground( QBrush( QColor( 32, 32, 32 ) ) );

   color_base      = QColor( 0,    47, 167 );
   color_highlight = QColor( 255,  69,   0 );
   color_subgrid   = QColor( 173, 255,  47 );
   subgrid_curve = us_curve( data_plot, "SUBGRID" );

   QRadioButton *rb_x_s;
   QRadioButton *rb_x_mw;
   QRadioButton *rb_x_ff0;
   QRadioButton *rb_x_D;
   QRadioButton *rb_x_vbar;
   QRadioButton *rb_x_sr;
   QRadioButton *rb_x_Dr;

   QRadioButton *rb_y_s;
   QRadioButton *rb_y_mw;
   QRadioButton *rb_y_ff0;
   QRadioButton *rb_y_D;
   QRadioButton *rb_y_vbar;
   QRadioButton *rb_y_sr;
   QRadioButton *rb_y_Dr;

   QGridLayout* lyt_x_s     = us_radiobutton( Attribute::symbol( Attribute::ATTR_S ),  rb_x_s,    false );
   QGridLayout* lyt_x_ff0   = us_radiobutton( Attribute::symbol( Attribute::ATTR_K ),  rb_x_ff0,  false );
   QGridLayout* lyt_x_mw    = us_radiobutton( Attribute::symbol( Attribute::ATTR_M ),  rb_x_mw,   false );
   QGridLayout* lyt_x_vbar  = us_radiobutton( Attribute::symbol( Attribute::ATTR_V ),  rb_x_vbar, false );
   QGridLayout* lyt_x_D     = us_radiobutton( Attribute::symbol( Attribute::ATTR_D ),  rb_x_D,    false );
   QGridLayout* lyt_x_sr    = us_radiobutton( Attribute::symbol( Attribute::ATTR_SR ), rb_x_sr,   false );
   QGridLayout* lyt_x_Dr    = us_radiobutton( Attribute::symbol( Attribute::ATTR_DR ), rb_x_Dr,   false );

   QGridLayout* lyt_y_s     = us_radiobutton( Attribute::symbol( Attribute::ATTR_S ),  rb_y_s,    false );
   QGridLayout* lyt_y_ff0   = us_radiobutton( Attribute::symbol( Attribute::ATTR_K ),  rb_y_ff0,  false );
   QGridLayout* lyt_y_mw    = us_radiobutton( Attribute::symbol( Attribute::ATTR_M ),  rb_y_mw,   false );
   QGridLayout* lyt_y_vbar  = us_radiobutton( Attribute::symbol( Attribute::ATTR_V ),  rb_y_vbar, false );
   QGridLayout* lyt_y_D     = us_radiobutton( Attribute::symbol( Attribute::ATTR_D ),  rb_y_D,    false );
   QGridLayout* lyt_y_sr    = us_radiobutton( Attribute::symbol( Attribute::ATTR_SR ), rb_y_sr,   false );
   QGridLayout* lyt_y_Dr    = us_radiobutton( Attribute::symbol( Attribute::ATTR_DR ), rb_y_Dr,   false );

   x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    Attribute::ATTR_S );
   x_axis->addButton( rb_x_ff0,  Attribute::ATTR_K );
   x_axis->addButton( rb_x_mw,   Attribute::ATTR_M );
   x_axis->addButton( rb_x_vbar, Attribute::ATTR_V );
   x_axis->addButton( rb_x_D,    Attribute::ATTR_D );
   x_axis->addButton( rb_x_sr,   Attribute::ATTR_SR );
   x_axis->addButton( rb_x_Dr,   Attribute::ATTR_DR );

   y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    Attribute::ATTR_S );
   y_axis->addButton( rb_y_ff0,  Attribute::ATTR_K );
   y_axis->addButton( rb_y_mw,   Attribute::ATTR_M );
   y_axis->addButton( rb_y_vbar, Attribute::ATTR_V );
   y_axis->addButton( rb_y_D,    Attribute::ATTR_D );
   y_axis->addButton( rb_y_sr,   Attribute::ATTR_SR );
   y_axis->addButton( rb_y_Dr,   Attribute::ATTR_DR );

   connect( x_axis, &QButtonGroup::idReleased, this, &US_Grid_Editor::select_x_axis );
   connect( y_axis, &QButtonGroup::idReleased, this, &US_Grid_Editor::select_y_axis );

   QLabel* lb_plt_cntrl = us_banner( "Plot Control" );
   QLabel* lb_x_plot = us_label( "X-Axis" );
   lb_x_plot->setAlignment( Qt::AlignCenter );
   QLabel* lb_y_plot = us_label( "Y-Axis" );
   lb_y_plot->setAlignment( Qt::AlignCenter );

   QLabel* lb_p_size = us_label( "Point Size" );
   lb_p_size->setAlignment( Qt::AlignCenter );
   ct_size = us_counter( 1, 1, 50, 2 );
   ct_size->setSingleStep( 1 );
   QLineEdit* le_sz = ct_size->findChild<QLineEdit*>();
   if ( le_sz ) {
      le_sz->setAlignment( Qt::AlignCenter );
   }

   connect( ct_size, &QwtCounter::valueChanged, this, &US_Grid_Editor::set_symbol_size );

   pb_default_plot = us_pushbutton( "Plot" );
   connect( pb_default_plot, &QPushButton::clicked, this, &US_Grid_Editor::default_plot_ctrl );

   QGridLayout* lyt_r = new QGridLayout();
   lyt_r->setMargin( 0 );
   int rr = 0;
   lyt_r->addWidget( lb_x_plot,       rr, 0, 1, 1 );
   lyt_r->addLayout( lyt_x_s,         rr, 1, 1, 1 );
   lyt_r->addLayout( lyt_x_ff0,       rr, 2, 1, 1 );
   lyt_r->addLayout( lyt_x_mw,        rr, 3, 1, 1 );
   lyt_r->addLayout( lyt_x_vbar,      rr, 4, 1, 1 );
   lyt_r->addLayout( lyt_x_D,         rr, 5, 1, 1 );
   lyt_r->addLayout( lyt_x_sr,        rr, 6, 1, 1 );
   lyt_r->addLayout( lyt_x_Dr,        rr, 7, 1, 1 );
   rr++;
   lyt_r->addWidget( lb_y_plot,       rr, 0, 1, 1 );
   lyt_r->addLayout( lyt_y_s,         rr, 1, 1, 1 );
   lyt_r->addLayout( lyt_y_ff0,       rr, 2, 1, 1 );
   lyt_r->addLayout( lyt_y_mw,        rr, 3, 1, 1 );
   lyt_r->addLayout( lyt_y_vbar,      rr, 4, 1, 1 );
   lyt_r->addLayout( lyt_y_D,         rr, 5, 1, 1 );
   lyt_r->addLayout( lyt_y_sr,        rr, 6, 1, 1 );
   lyt_r->addLayout( lyt_y_Dr,        rr, 7, 1, 1 );
   rr++;
   lyt_r->addWidget( lb_p_size,       rr, 0, 1, 1 );
   lyt_r->addWidget( ct_size,         rr, 1, 1, 1 );
   lyt_r->addWidget( pb_default_plot, rr, 3, 1, 2 );

   for ( int ii = 0; ii < lyt_r->columnCount(); ii++ ) {
      lyt_r->setColumnStretch( ii, 1 );
   }

   right->addLayout( plot_lyt );
   right->addWidget( lb_plt_cntrl );
   right->addLayout( lyt_r );

   main->addLayout( left );
   main->addLayout( right );
   main->setStretchFactor( left, 2 );
   main->setStretchFactor( right, 6 );

   plot_flag = true;
   reset();
   select_x_axis( x_param );
   select_y_axis( y_param );
   set_cg_mode();
}

void US_Grid_Editor::delete_data()
{
   lw_grids->disconnect();
   lw_grids->clear();
   partial_grids.clear();
   partial_grid_info.clear();
   sorted_points.clear();
   final_subgrids.clear();
}

void US_Grid_Editor::show_layout( QLayout* layout, bool show )
{
   if (!layout)
      return;

   for (int i = 0; i < layout->count(); ++i) {
      QLayoutItem *item = layout->itemAt(i);

      if (QWidget *widget = item->widget()) {
         if ( show ) {
            widget->show();
         } else {
            widget->hide();
         }
      } else if (QLayout *childLayout = item->layout()) {
         show_layout( childLayout, show );
      }
   }
}

void US_Grid_Editor::reset()
{
   delete_data();

   x_param = Attribute::ATTR_S;
   y_param = Attribute::ATTR_K;
   z_param = Attribute::ATTR_V;

   le_x_param->setText( Attribute::long_desc( x_param ) );
   le_y_param->setText( Attribute::long_desc( y_param ) );
   le_z_param->setText( Attribute::long_desc( z_param ) );
   lb_x_ax->setText( Attribute::short_desc( x_param ) );
   lb_y_ax->setText( Attribute::short_desc( y_param ) );
   lb_z_ax->setText( Attribute::short_desc( z_param ) );
   QString s = tr( "plot ( %1 ) over ( %2 )" ).
               arg( Attribute::symbol( y_param), Attribute::symbol( x_param ) );
   pb_default_plot->setText( s );

   le_x_res->setText( QString::number( 64 ) );
   le_y_res->setText( QString::number( 64 ) );

   buff_dens = DENS_20W;
   buff_visc = VISC_20W;
   buff_temp = 20;
   buffer_type = ST_WATER;
   le_dens_20->setText( QString::number( buff_dens ) );
   le_visc_20->setText( QString::number( buff_visc ) );
   le_dens_T->setText( QString::number( buff_dens ) );
   le_visc_T->setText( QString::number( buff_visc ) );
   le_temp->setText( QString::number( buff_temp ) );
   le_buffer->setText( "Water" );

   default_plot_ctrl();
}

void US_Grid_Editor::rm_tmp_items()
{
   const auto items = data_plot->itemList();
   for ( QwtPlotItem *item : items ) {
      if ( item ) {
         QString txt = item->title().text();
         if ( txt.startsWith( "TMP_R" ) ) {
            item->detach();
            delete item;
         }
      }
   }
}

void US_Grid_Editor::rm_subgrid_curve()
{
   subgrid_curve->setTitle( "SUBGRID" );
   subgrid_curve->setSamples( QVector<QPointF>() );
}

void US_Grid_Editor::rm_point_curves()
{
   for ( int ii = 0; ii < point_curves.size(); ii++ ) {
      point_curves[ii]->detach();
      delete point_curves[ii];
   }
   point_curves.clear();
}

void US_Grid_Editor::plot_tmp()
{
   rm_tmp_items();

   double x1, x2, y1, y2;
   bool   xMin_set = validate_double( le_x_min->text(), x1 );
   bool   xMax_set = validate_double( le_x_max->text(), x2 );
   bool   yMin_set = validate_double( le_y_min->text(), y1 );
   bool   yMax_set = validate_double( le_y_max->text(), y2 );
   bool   grid_set = ! partial_grids.isEmpty();
   double px1 = data_plot->axisScaleDiv( QwtPlot::xBottom ).lowerBound();
   double px2 = data_plot->axisScaleDiv( QwtPlot::xBottom ).upperBound();
   double py1 = data_plot->axisScaleDiv( QwtPlot::yLeft ).lowerBound();
   double py2 = data_plot->axisScaleDiv( QwtPlot::yLeft ).upperBound();

   bool ok = xMin_set || xMax_set || yMin_set || yMax_set;
   if ( ! ok ) {
      data_plot->replot();
      return;
   }

   if ( grid_set )
   {
      GridInfo ginfo = partial_grid_info.first();
      px1 = correct_unit( ginfo.xMin, Attribute::from_symbol( ginfo.xType ), true );
      px2 = correct_unit( ginfo.xMax, Attribute::from_symbol( ginfo.xType ), true );
      py1 = correct_unit( ginfo.yMin, Attribute::from_symbol( ginfo.yType ), true );
      py2 = correct_unit( ginfo.yMax, Attribute::from_symbol( ginfo.yType ), true );
      for ( int ii = 0; ii < partial_grid_info.size(); ii++ ) {
         GridInfo ginfo = partial_grid_info.at( ii );
         px1 = qMin( px1, correct_unit( ginfo.xMin, Attribute::from_symbol( ginfo.xType ), true ) );
         px2 = qMax( px2, correct_unit( ginfo.xMax, Attribute::from_symbol( ginfo.xType ), true ) );
         py1 = qMin( py1, correct_unit( ginfo.yMin, Attribute::from_symbol( ginfo.yType ), true ) );
         py2 = qMax( py2, correct_unit( ginfo.yMax, Attribute::from_symbol( ginfo.yType ), true ) );
      }

      if ( xMin_set && xMax_set ) {
         px1 = qMin( qMin( x1, x2 ), px1 );
         px2 = qMax( qMax( x1, x2 ), px2 );
      } else if ( xMin_set && ! xMax_set ) {
         x2  = x1;
         px1 = qMin( x1, px1 );
         px2 = qMax( x1, px2 );
      } else if ( ! xMin_set && xMax_set ) {
         x1  = x2;
         px1 = qMin( x2, px1 );
         px2 = qMax( x2, px2 );
      } else {
         x1 = px1 - qAbs( px1 ) * 100;
         x2 = px2 + qAbs( px2 ) * 100;
      }

      if ( yMin_set && yMax_set ) {
         py1 = qMin( qMin( y1, y2 ), py1 );
         py2 = qMax( qMax( y1, y2 ), py2 );
      } else if ( yMin_set && ! yMax_set ) {
         y2  = y1;
         py1 = qMin( y1, py1 );
         py2 = qMax( y1, py2 );
      } else if ( ! yMin_set && yMax_set ) {
         y1  = y2;
         py1 = qMin( y2, py1 );
         py2 = qMax( y2, py2 );
      } else {
         y1 = py1 - qAbs( py1 ) * 100;
         y2 = py2 + qAbs( py2 ) * 100;
      }
   }
   else
   {
      if ( xMin_set && xMax_set ) {
         px1 = qMin( x1, x2 );
         px2 = qMax( x1, x2 );
      } else if ( xMin_set && ! xMax_set ) {
         x2  = x1;
         px1 = x1;
         px2 = x1;
      } else if ( ! xMin_set && xMax_set ) {
         x1  = x2;
         px1 = x2;
         px2 = x2;
      } else {
         x1 = px1 - qAbs( px1 ) * 100;
         x2 = px2 + qAbs( px2 ) * 100;
      }

      if ( yMin_set && yMax_set ) {
         py1 = qMin( y1, y2 );
         py2 = qMax( y1, y2 );
      } else if ( yMin_set && ! yMax_set ) {
         y2  = y1;
         py1 = y1;
         py2 = y1;
      } else if ( ! yMin_set && yMax_set ) {
         y1  = y2;
         py1 = y2;
         py2 = y2;
      } else {
         y1 = py1 - qAbs( py1 ) * 100;
         y2 = py2 + qAbs( py2 ) * 100;
      }
   }

   QPainterPath path;
   QRectF rect( x1, y1, x2 - x1, y2 - y1 );
   path.addRect( rect );
   QwtPlotShapeItem *shapeItem = new QwtPlotShapeItem();
   shapeItem->setTitle( "TMP_R" );
   shapeItem->setShape( path );
   shapeItem->setBrush( QBrush( QColor( 255,255,255, 128 ) ) );
   shapeItem->setPen( QPen( QColor( 255,255,255 ), 2 ) );
   shapeItem->attach( data_plot );

   double dx = px2 - px1;
   double dy = py2 - py1;
   dx = dx == 0 ? 0.05 : dx * 0.05;
   dy = dy == 0 ? 0.05 : dy * 0.05;
   px1 -= dx;
   px2 += dx;
   py1 -= dy;
   py2 += dy;

   data_plot->setAxisScale( QwtPlot::xBottom, px1, px2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , py1, py2 );
   data_plot->replot();
}

void US_Grid_Editor::plot_points()
{
   rm_tmp_items();
   rm_point_curves();

   Attribute::Type pxid = Attribute::from_int( x_axis->checkedId() );
   Attribute::Type pyid = Attribute::from_int( y_axis->checkedId() );

   data_plot->setAxisTitle( QwtPlot::xBottom, Attribute::title( pxid ) );
   data_plot->setAxisTitle( QwtPlot::yLeft,   Attribute::title( pyid ) );
   data_plot->setAxisScale( QwtPlot::xBottom, 1, 10 );
   data_plot->setAxisScale( QwtPlot::yLeft  , 1, 10 );
   data_plot->replot();

   if ( partial_grids.isEmpty() ) {
      pb_save->setDisabled( true );
      return;
   }
   pb_save->setEnabled( true );

   double px1 =  1e99;
   double px2 = -1e99;
   double py1 =  1e99;
   double py2 = -1e99;
   int ss = ct_size->value();
   for ( int nn = 0; nn < partial_grids.size(); nn++ ) {
      QVector<GridPoint> gps = partial_grids.at( nn );
      QVector<double> xarr;
      QVector<double> yarr;
      for ( int ii = 0; ii < gps.size(); ii++ ) {
         GridPoint gp = gps.at( ii );
         double x = value4plot( gp, pxid );
         double y = value4plot( gp, pyid );

         px1 = qMin( px1, x );
         px2 = qMax( px2, x );
         py1 = qMin( py1, y );
         py2 = qMax( py2, y );

         xarr << x;
         yarr << y;
      }
      QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
                                        QBrush( color_base ),
                                        QPen( color_base, 2 ), QSize( ss, ss ) );
      QString title = tr( "GRID_%1" ).arg( nn );
      QwtPlotCurve* curve = us_curve( data_plot, title );
      curve->setSymbol( symbol );
      curve->setStyle( QwtPlotCurve::NoCurve );
      curve->setSamples( xarr.data(), yarr.data(), xarr.size() );
      curve->setZ( 0 );
      point_curves << curve;
   }

   double fac = 0.05;
   double dx = ( px2 - px1 ) * fac;
   double dy = ( py2 - py1 ) * fac;
   px1 -= dx;
   px2 += dx;
   py1 -= dy;
   py2 += dy;
   if ( pxid == z_param ) {
      double mid = 0.5 * ( px1 + px2 );
      double dd = mid * fac;
      px1 = qMin( px1, mid - dd );
      px2 = qMax( px2, mid + dd );
   }
   if ( pyid == z_param ) {
      double mid = 0.5 * ( py1 + py2 );
      double dd = mid * fac;
      py1 = qMin( py1, mid - dd );
      py2 = qMax( py2, mid + dd );
   }

   data_plot->setAxisScale( QwtPlot::xBottom, px1, px2 );
   data_plot->setAxisScale( QwtPlot::yLeft  , py1, py2 );
   data_plot->replot();
}

void US_Grid_Editor::set_nsubgrids( double num )
{
   ct_subgrid->disconnect();
   ct_nsubgrids->disconnect();

   final_subgrids.clear();
   int nsubgrids = num;
   int npoints = sorted_points.size();
   if ( npoints > 0 && npoints < nsubgrids ) {
      nsubgrids = npoints;
   }
   ct_nsubgrids->setValue( nsubgrids );

   int sbg;
   if ( static_cast<int> ( ct_subgrid->value() ) > nsubgrids ) {
      sbg = nsubgrids;
   } else {
      sbg = ct_subgrid->value();
   }
   ct_subgrid->setMaximum( nsubgrids );
   ct_subgrid->setValue( sbg );

   connect( ct_subgrid,   &QwtCounter::valueChanged, this, &US_Grid_Editor::plot_subgrid );
   connect( ct_nsubgrids, &QwtCounter::valueChanged, this, &US_Grid_Editor::set_nsubgrids );

   if ( npoints > 0 ) {
      for ( int ii = 0; ii < nsubgrids; ii++ ) {
         QVector<int> arr;
         for ( int jj = ii; jj < npoints; jj += nsubgrids )
         {
            arr << jj;
         }
         final_subgrids << arr;
      }
   }
   plot_subgrid( ct_subgrid->value() );
}

void US_Grid_Editor::plot_subgrid( double subgrid_id )
{
   rm_subgrid_curve();
   le_npoints->setText( "0" );
   le_npoints_curr->clear();
   le_npoints_last->clear();
   if ( final_subgrids.isEmpty() || point_curves.isEmpty() ) {
      data_plot->replot();
      return;
   }

   subgrid_id--;
   int np_all = sorted_points.size();
   int np_last = final_subgrids.last().size();
   QVector<int> grid_id = final_subgrids.at( subgrid_id );
   int np = grid_id.size();
   Attribute::Type pxid = Attribute::from_int( x_axis->checkedId() );
   Attribute::Type pyid = Attribute::from_int( y_axis->checkedId() );
   QVector<double> xarr;
   QVector<double> yarr;
   for ( int ii = 0; ii < np; ii++ ) {
      GridPoint gp = sorted_points.at( grid_id.at( ii ) );
      double x = value4plot( gp, pxid );
      double y = value4plot( gp, pyid );
      xarr << x;
      yarr << y;
   }
   int ss = ct_size->value() + 2;
   QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
                                      QBrush( color_subgrid ),
                                      QPen( color_subgrid, 2 ), QSize( ss, ss ) );
   QString title = tr( "SUBGRID_%1" ).arg( subgrid_id );
   subgrid_curve->setTitle( title );
   subgrid_curve->setSymbol( symbol );
   subgrid_curve->setSamples( xarr.data(), yarr.data(), np );
   subgrid_curve->setZ( 1 );
   subgrid_curve->setStyle( QwtPlotCurve::NoCurve );
   data_plot->replot();

   le_npoints->setText( tr( "%1" ).arg( np_all ) );
   le_npoints_curr->setText( tr( "%1" ).arg( np ) );
   le_npoints_last->setText( tr( "%1" ).arg( np_last ) );
}

void US_Grid_Editor::select_partial_grid( int id )
{
   clear_xyz();
   for ( int ii = 0; ii < point_curves.size(); ii++ ) {
      QSize size = point_curves[ii]->symbol()->size();
      QPen pen = point_curves[ii]->symbol()->pen();
      pen.setBrush( color_base );
      QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( color_base ), pen, size );
      point_curves[ii]->setSymbol( symbol );
   }

   if ( id != -1 ) {
      QSize size = point_curves[id]->symbol()->size();
      QPen pen = point_curves[id]->symbol()->pen();
      pen.setBrush( color_highlight );
      QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse, QBrush( color_highlight ), pen, size );
      point_curves[id]->setSymbol( symbol );

      GridInfo ginfo = partial_grid_info.at( id );

      double x_min = correct_unit( ginfo.xMin, Attribute::from_symbol( ginfo.xType ), true );
      double x_max = correct_unit( ginfo.xMax, Attribute::from_symbol( ginfo.xType ), true );
      double y_min = correct_unit( ginfo.yMin, Attribute::from_symbol( ginfo.yType ), true );
      double y_max = correct_unit( ginfo.yMax, Attribute::from_symbol( ginfo.yType ), true );
      bool constant;
      QString type, dpnt;
      QVector<double> prm;
      parse_z_expression( ginfo.zVal, type, dpnt, prm, constant );
      if ( constant ) {
         set_zval_type( 0 );
         rb_cons_z->setChecked( true );
         le_z_val->setText( QString::number( prm.first() ) );
      }
      else {
         set_zval_type( 1 );
         rb_vary_z->setChecked( true );
         le_z_val->setText( ginfo.zVal );
      }

      int    x_res = ginfo.xRes;
      int    y_res = ginfo.yRes;

      le_x_min->setText( QString::number( x_min ) );
      le_x_max->setText( QString::number( x_max ) );
      le_x_res->setText( QString::number( x_res ) );

      le_y_min->setText( QString::number( y_min ) );
      le_y_max->setText( QString::number( y_max ) );
      le_y_res->setText( QString::number( y_res ) );
   }
   plot_subgrid( ct_subgrid->value() );
}

void US_Grid_Editor::enable_ctrl( bool state )
{
   le_x_min->setEnabled( state );
   le_x_max->setEnabled( state );
   le_x_res->setEnabled( state );
   le_y_min->setEnabled( state );
   le_y_max->setEnabled( state );
   le_y_res->setEnabled( state );
   le_z_val->setEnabled( state );
   pb_add_update->setEnabled( state );
   pb_z_set_func->setEnabled( state );
   rb_cons_z->setEnabled( state );
   rb_vary_z->setEnabled( state );
}

void US_Grid_Editor::clear_xyz()
{
   le_x_min->clear();
   le_x_max->clear();
   le_y_min->clear();
   le_y_max->clear();
   le_z_val->clear();
   pb_add_update->setText( "Add / Update" );
   enable_ctrl( false );
   rm_tmp_items();
}

double US_Grid_Editor::value4plot( GridPoint& grid, Attribute::Type type )
{
   double val = grid.value( type );
   val = correct_unit( val, type, true );
   return val;
}

void US_Grid_Editor::default_plot_ctrl()
{
   plot_flag = false;
   x_axis->button( x_param )->setChecked( true );
   y_axis->button( y_param )->setChecked( true );
   select_x_axis( x_param );
   select_y_axis( y_param );
   plot_flag = true;
   plot_points();
   select_partial_grid( lw_grids->currentRow() );
}

void US_Grid_Editor::select_x_axis( int index )
{
   QVector<Attribute::Type> tlist;
   tlist << Attribute::ATTR_S << Attribute::ATTR_K
         << Attribute::ATTR_M << Attribute::ATTR_V
         << Attribute::ATTR_D << Attribute::ATTR_SR
         << Attribute::ATTR_DR;

   foreach ( Attribute::Type type, tlist ) {
      y_axis->button( type )->setEnabled( true );
   }
   y_axis->button( index )->setDisabled( true );

   if ( plot_flag ) {
      plot_points();
      int row = lw_grids->currentRow();
      select_partial_grid( row );
   }
   enable_ctrl( false );
}

void US_Grid_Editor::select_y_axis( int index )
{
   QVector<Attribute::Type> tlist;
   tlist << Attribute::ATTR_S << Attribute::ATTR_K
         << Attribute::ATTR_M << Attribute::ATTR_V
         << Attribute::ATTR_D << Attribute::ATTR_SR
         << Attribute::ATTR_DR;

   foreach ( Attribute::Type type, tlist ) {
      x_axis->button( type )->setEnabled( true );
   }
   x_axis->button( index )->setDisabled( true );

   if ( plot_flag ) {
      plot_points();
      int row = lw_grids->currentRow();
      select_partial_grid( row );
   }
   enable_ctrl( false );
}

void US_Grid_Editor::new_grid_clicked()
{
   default_plot_ctrl();
   select_partial_grid( -1 );
   enable_ctrl( true );
   pb_add_update->setText( "Add" );
}

void US_Grid_Editor::update_grid_clicked()
{
   default_plot_ctrl();
   if ( lw_grids->currentRow() >= 0 ) {
      enable_ctrl( true );
      pb_add_update->setText( "Update" );
      plot_tmp();
   }
}

void US_Grid_Editor::delete_grid_clicked()
{
   int row = lw_grids->currentRow();
   if ( row == -1 ) return;
   int yes = QMessageBox::question( this, "Warning!",
                                   tr( "The grid containing the following "
                                      "data will be deleted.<br/><br/><b>%1</b><br/><br/>"
                                      "Press <b>\"Yes\"</b> to confirm." ).
                                   arg( lw_grids->currentItem()->text() ) );
   if ( yes == QMessageBox::No ) return;

   rm_point_curves();

   partial_grids.removeAt( row );
   partial_grid_info.removeAt( row );
   check_grid_id();
   fill_list();
   sort_points();
   set_nsubgrids( ct_nsubgrids->value() );

   plot_points();
   select_partial_grid( lw_grids->currentRow() );
}

void US_Grid_Editor::add_update()
{
   int excl;
   if ( pb_add_update->text().compare( "Add" ) == 0 ) {
      excl = -1;
   } else if ( pb_add_update->text().compare( "Update" ) == 0 ) {
      excl = lw_grids->currentRow();
      if ( excl == -1 ) return;
   } else {
      return;
   }

   GridInfo ginfo;
   if ( ! validate_partial_grid( ginfo ) ) return;

   if ( check_overlap( ginfo.xMin, ginfo.xMax,
                     ginfo.yMin, ginfo.yMax, excl ) ) {
      QMessageBox::critical( this, "Error!", "Grid Points Overlap!" );
      return;
   }

   QVector<GridPoint> gpoints;
   if ( ! gen_grid_points( ginfo, gpoints ) ) {
      return;
   }

   if ( excl == -1 ) {
      partial_grids << gpoints;
      partial_grid_info << ginfo;
   } else {
      partial_grids.replace( excl, gpoints );
      partial_grid_info.replace( excl, ginfo );
   }
   check_grid_id();
   fill_list();
   sort_points();
   plot_points();
   set_nsubgrids( ct_nsubgrids->value() );
   select_partial_grid( lw_grids->currentRow() );
}

void US_Grid_Editor::set_mid_exct_points( int id )
{
   if ( partial_grid_info.isEmpty() ) return;
   if ( id == EXACTPOINTS ) 
   {
      GridInfo ginfo = partial_grid_info.first();
      double xMin = ginfo.xMin;
      double xMax = ginfo.xMax;
      double yMin = ginfo.yMin;
      double yMax = ginfo.yMax;
      bool overlap = false;
      for (int ii = 1; ii < partial_grid_info.size(); ii++)
      {
         GridInfo ginfo = partial_grid_info.at(ii);
         double x1 = ginfo.xMin;
         double x2 = ginfo.xMax;
         double y1 = ginfo.yMin;
         double y2 = ginfo.yMax;
         bool x_ovrlp = !(xMin > x2 || xMax < x1);
         bool y_ovrlp = !(yMin > y2 || yMax < y1);
         if (x_ovrlp && y_ovrlp)
         {
            overlap = true;
            break;
         }
      }
      if ( overlap )
      {
         QMessageBox::warning( this, "Warning!", 
         "Partial grids are overlapping.\n"
         "Please correct the ranges, then try again.");
         bg_point_type->disconnect();
         bg_point_type->button( MIDPOINTS )->setChecked( true );
         connect( bg_point_type, &QButtonGroup::idClicked, this, &US_Grid_Editor::set_mid_exct_points );
         return;
      }
   }

   refill_grid_points();
}

void US_Grid_Editor::set_cg_mode()
{
   delete_data();
   default_plot_ctrl();

   if( rb_cg_mode->isChecked() ) {
      lb_grid_mode->setText( "Create Grid Control" );
      pb_load_csv->hide();
      te_info->hide();

      lb_grid_list->show();
      lw_grids->show();
      lb_min->show();
      lb_max->show();
      lb_res->show();
      lb_x_ax->show();
      lb_y_ax->show();
      lb_z_ax->show();
      le_x_min->show();
      le_x_max->show();
      le_x_res->show();
      le_y_min->show();
      le_y_max->show();
      le_y_res->show();
      le_z_val->show();
      pb_add_update->show();
      pb_new->show();
      pb_update->show();
      pb_delete->show();
      hline1->show();

      show_layout( lyt_log, true );
      show_layout( lyt_points, true );
      show_layout( lyt_zvalues, true );
   } else {
      lb_grid_mode->setText( "Load Grid Control" );
      pb_load_csv->show();
      te_info->show();

      lb_grid_list->hide();
      lw_grids->hide();
      lb_min->hide();
      lb_max->hide();
      lb_res->hide();
      lb_x_ax->hide();
      lb_y_ax->hide();
      lb_z_ax->hide();
      le_x_min->hide();
      le_x_max->hide();
      le_x_res->hide();
      le_y_min->hide();
      le_y_max->hide();
      le_y_res->hide();
      le_z_val->hide();
      pb_add_update->hide();
      pb_new->hide();
      pb_update->hide();
      pb_delete->hide();
      hline1->hide();

      show_layout( lyt_log, false );
      show_layout( lyt_points, false );
      show_layout( lyt_zvalues, false );
   }
}

void US_Grid_Editor::load_grid_file()
{
   QString filter = "Text Files (*.csv *.dat *.txt);; All Files (*)";
   QString fpath = QFileDialog::getOpenFileName(this, "Load GRID FILE",
                                                US_Settings::dataDir(), filter);
   if (fpath.isEmpty()) {
      return;
   }

   US_CSV_Loader *csv_loader = new US_CSV_Loader(fpath, "", true, this);
   int state = csv_loader->exec();
   if (state != QDialog::Accepted) return;
   US_CSV_Data csv_data = csv_loader->data();
   if (csv_data.columnCount() < 3) {
      QMessageBox::warning( this, "Warning!", "Three columns of data needed for this program!" );
      return;
   }

   US_WidgetsDialog* dialog = new US_WidgetsDialog( this );
   dialog->setWindowTitle( "Set Grid Parameters" );
   dialog->setPalette( US_GuiSettings::frameColor() );

   QGridLayout *layout = new QGridLayout();
   layout->setContentsMargins( 3, 3, 3, 3 );
   layout->setSpacing( 2 );
   dialog->setLayout( layout );

   QLabel* lb_param  = us_banner( "Parameters" );
   QLabel* lb_header = us_banner( "Headers" );
   QLabel* lb_p1 = us_label( Attribute::long_desc( x_param ) );
   QLabel* lb_p2 = us_label( Attribute::long_desc( y_param ) );
   QLabel* lb_p3 = us_label( Attribute::long_desc( z_param ) );
   QComboBox* cb_p1  = us_comboBox();
   QComboBox* cb_p2  = us_comboBox();
   QComboBox* cb_p3 = us_comboBox();

   cb_p1->addItems( csv_data.header() );
   cb_p2->addItems( csv_data.header() );
   cb_p3->addItems( csv_data.header() );

   QPushButton* pb_apply = us_pushbutton( "Apply" );
   QPushButton* pb_cancel = us_pushbutton( "Cancel" );

   layout->addWidget( lb_param,   0, 0, 1, 2 );
   layout->addWidget( lb_header,  0, 2, 1, 2 );
   layout->addWidget( lb_p1,      1, 0, 1, 2 );
   layout->addWidget( cb_p1,      1, 2, 1, 2 );
   layout->addWidget( lb_p2,      2, 0, 1, 2 );
   layout->addWidget( cb_p2,      2, 2, 1, 2 );
   layout->addWidget( lb_p3,      3, 0, 1, 2 );
   layout->addWidget( cb_p3,      3, 2, 1, 2 );
   layout->addItem( new QSpacerItem( 0, 30 ), 4, 0 );
   layout->addWidget( pb_cancel,  5, 2, 1, 1 );
   layout->addWidget( pb_apply,   5, 3, 1, 1 );

   connect( pb_cancel, &QPushButton::clicked, dialog, &QDialog::reject );
   connect( pb_apply, &QPushButton::clicked, this, [=]() {
      bool ok = cb_p1->currentIndex() != cb_p2->currentIndex();
      ok &= cb_p1->currentIndex() != cb_p3->currentIndex();
      ok &= cb_p2->currentIndex() != cb_p3->currentIndex();
      if ( ok ) {
         dialog->accept();
      } else {
         QMessageBox::warning( dialog, "Warning!", "A column is selected twice or more!" );
      }
   });

   if ( dialog->exec() == QDialog::Accepted ) {
      QVector<double> p1_vals = csv_data.columnAt( cb_p1->currentIndex() );
      QVector<double> p2_vals = csv_data.columnAt( cb_p2->currentIndex() );
      QVector<double> p3_vals = csv_data.columnAt( cb_p3->currentIndex() );
      if ( ! ( p1_vals.size() == p2_vals.size() && p2_vals.size() == p3_vals.size() ) ) {
         QMessageBox::warning( this, "Error!", "The number of grid points between selected parameter is different!" );
         return;
      }
      QVector<Attribute::Type> types;
      types << x_param << y_param << z_param;
      QVector<double> vals;
      QVector<GridPoint> gpoints;
      for ( int ii = 0; ii < p1_vals.size(); ii++ ) {
         vals.clear();
         vals << p1_vals.at( ii ) << p2_vals.at( ii ) << p3_vals.at( ii );
         GridPoint gp;
         gp.set_dens_visc_temp( buff_dens, buff_visc, buff_temp );

         if ( gp.set_param( vals, types ) ) {
            gp.set_row_col( 0, 0 );
            gpoints << gp;
         } else {
            QString msg = tr( "Row :%1  <br/>" ).arg( ii + 1);
            msg += tr( "Parameter 1: %1 <br/>" ).arg( lb_p1->text() );
            msg += tr( "Header 1: %1 <br/>" ).arg( csv_data.header().at( cb_p1->currentIndex() ) );
            msg += tr( "Parameter 2: %1 <br/>" ).arg( lb_p2->text() );
            msg += tr( "Header 2: %1 <br/>" ).arg( csv_data.header().at( cb_p2->currentIndex() ) );
            msg += tr( "Parameter 3: %1 <br/>" ).arg( lb_p3->text() );
            msg += tr( "Header 3: %1 <br/>" ).arg( csv_data.header().at( cb_p3->currentIndex() ) );
            msg += tr( "<br/>Error: <br/>%1" ).arg( gp.error_string() );

            QMessageBox msgBox( this );
            msgBox.setIcon( QMessageBox::Warning );
            msgBox.setWindowTitle( "Error!" );
            msgBox.setTextFormat( Qt::RichText );
            msgBox.setText( msg );
            msgBox.setInformativeText("Click OK to continue.");
            msgBox.exec();
            return;
         }
      }
      partial_grids << gpoints;
      sorted_points << gpoints;
      plot_points();
      set_nsubgrids( ct_nsubgrids->value() );
      plot_subgrid( ct_subgrid->value() );
   }

}

void US_Grid_Editor::refill_grid_points()
{
   if ( partial_grid_info.isEmpty() ) {
      return;
   }

   QList<QVector<GridPoint>> new_grid_points;
   for ( int ii = 0; ii < partial_grid_info.size(); ii++ ) {
      GridInfo ginfo = partial_grid_info.at( ii );
      QVector<GridPoint> gpoints;
      if ( ! gen_grid_points( ginfo, gpoints ) ) {
         return;
      }
      new_grid_points << gpoints;
   }

   partial_grids.clear();
   partial_grids << new_grid_points;
   check_grid_id();
   fill_list();
   sort_points();
   plot_points();
   set_nsubgrids( ct_nsubgrids->value() );
   select_partial_grid( lw_grids->currentRow() );
}

bool US_Grid_Editor::spaced_numbers( double x1, double x2, int np,
                           bool isLog, bool isMid, QVector<double>& result )
{
   if ( x2 <= x1 ) return false;
   if ( isLog && ( x1 * x2 < 0 ) ) return false;
   result.resize( np );
   double sign = 1;
   if ( isLog ) {
      if ( x1 < 0 || x2 < 0 ) {
         sign = -1;
      }
      if ( x1 == 0 ) {
         x1 = 1e-20;
      }
      if ( x2 == 0 ) {
         x2 = 1e-20;
      }
      x1 = std::log10( x1 * sign );
      x2 = std::log10( x2 * sign );
   }
   double dx, val;
   if ( isMid ) {
       dx = ( x2 - x1 ) / static_cast<double>( np );
       val = x1 + 0.5 * dx;
   } else {
       dx = ( x2 - x1 ) / static_cast<double>( np - 1 );
       val = x1;
   }
   for ( int ii = 0; ii < np; ii++ ) {
      if ( ii != 0 ) {
         val += dx;
      }
      if ( isLog ) {
         result[ii] = sign * std::pow( 10, val );
      } else {
         result[ii] = val;
      }
   }
   return true;
}

bool US_Grid_Editor::parse_z_expression( const QString &z_expr, QString &type, QString &dependent,
                                         QVector<double> &parameters, bool& constant )
{
   type = "polynomial";
   dependent = "x";
   parameters.clear();
   constant = false;
   QStringList plist = z_expr.trimmed().split( ";" );
   if ( z_expr.trimmed().isEmpty() ) {
      return false;
   }
   else if ( plist.size() == 1 ) {
      double cc = correct_unit( plist.first().toDouble(), z_param, true );
      parameters << cc << 0.0;
      constant = true;
      return true;
   }
   else if ( plist.size() >= 4 ){
      type = plist.at( 0 ).trimmed();
      dependent = plist.at( 1 ).trimmed();
      for ( int ii = 2; ii < plist.size(); ii++ ) {
         parameters << plist.at( ii ).toDouble();
      }
      return true;
   }
   else {
      return false;
   }
}

bool US_Grid_Editor::calc_xyz( double xx, double yy , const QString &z_expr,
                               QVector<double> &xyz )
{
   xyz.clear();
   QString type, dependent;
   QVector< double > parameters;
   bool constant;
   if ( ! parse_z_expression( z_expr, type, dependent, parameters, constant ) ) {
      return false;
   }
   xx = correct_unit( xx, x_param, true );
   yy = correct_unit( yy, y_param, true );
   double dd;
   if ( dependent == "x" ) {
      dd = xx;
   }
   else {
      dd = yy;
   }
   double zz = 0;
   if ( type == "polynomial" ) {
      for( int ii = 0; ii < parameters.size(); ii++ ) {
         zz += parameters.at( ii ) * qPow( dd, ii );
      }
   }
   else if ( type == "exponential" ) {
      zz = parameters.at( 0 ) * qExp( parameters.at( 1 ) * dd );
   }
   xx = correct_unit( xx, x_param, false );
   yy = correct_unit( yy, y_param, false );
   zz = correct_unit( zz, z_param, false );
   xyz << xx << yy << zz;
   return true;
}

bool US_Grid_Editor::gen_grid_points( const GridInfo& ginfo, QVector<GridPoint>& gpoints )
{
   bool isLog = chkb_log->isChecked();
   bool isMid = bg_point_type->checkedId() == MIDPOINTS;
   QVector<double> xpoints;
   QVector<double> ypoints;
   bool ok =  spaced_numbers( ginfo.xMin, ginfo.xMax, ginfo.xRes, isLog, isMid, xpoints );
   ok = ok && spaced_numbers( ginfo.yMin, ginfo.yMax, ginfo.yRes, false, isMid, ypoints );
   if ( ! ok ) {
      return false;
   }

   QVector<Attribute::Type> types;
   types << x_param << y_param << z_param;
   gpoints.clear();
   QStringList wrong_list;
   QStringList error_list;
   QString wrong_item( "%1 : %2, %3 : %4, %5 : %6" );
   QString xs = Attribute::symbol( x_param );
   QString ys = Attribute::symbol( y_param );
   QString zs = Attribute::symbol( z_param );
   for ( int ii = 0; ii < ypoints.size(); ii++ ) {
      int row = ii % ginfo.yRes;
      for ( int jj = 0; jj < xpoints.size(); jj++ ) {
         int col = jj % ginfo.xRes;
         QVector<double> vals;
         double xx = xpoints.at( jj );
         double yy = ypoints.at( ii );
         if ( ! calc_xyz( xx, yy, ginfo.zVal, vals ) ) {
            return false;
         }
         GridPoint gp;
         gp.set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
         if ( gp.set_param( vals, types ) ) {
            gp.set_row_col( row, col );
            gpoints << gp;
         } else {
            double x = vals.at( 0 );
            double y = vals.at( 1 );
            double z = vals.at( 2 );
            wrong_list << wrong_item.arg( xs ).arg( x ).arg( ys ).arg( y ).arg( zs ).arg( z );
            error_list << gp.error_string();
         }
      }
   }

   if ( ! wrong_list.isEmpty() ) {
      QString line( "%1 - %2 :\n %3 \n\n" );
      QMessageBox msg_box;
      msg_box.setIcon( QMessageBox::Warning );
      msg_box.setWindowTitle( "Warning" );
      msg_box.setText( "The following grid points are excluded." );
      QTextEdit *text = new QTextEdit( &msg_box );
      for ( int ii = 0; ii < wrong_list.size(); ii++ ) {
         text->append( line.arg( ii + 1 )
                         .arg( wrong_list.at( ii ), error_list.at( ii ) ) );
      }
      text->setReadOnly( true );
      text->setMinimumSize( 500, 500 );
      QGridLayout *layout = qobject_cast<QGridLayout *>( msg_box.layout() );
      if ( layout ) {
         layout->addWidget( text, layout->rowCount(), 0, 1, layout->columnCount() );
      }
      msg_box.exec();
   }

   if ( gpoints.isEmpty() ) {
      return false;
   } else {
      return true;
   }
}

double US_Grid_Editor::correct_unit( double val, Attribute::Type type, bool Sv_kDa )
{
   double output = val;
   if ( type == Attribute::ATTR_S ) {
      output = Sv_kDa ? val * 1e13 : val * 1e-13;
   }
   else if ( type == Attribute::ATTR_SR ) {
      output = Sv_kDa ? val * 1e13 : val * 1e-13;
   }
   else if ( type == Attribute::ATTR_M ) {
      output = Sv_kDa ? val / 1000 : val * 1000;
   }
   return output;
}

bool US_Grid_Editor::check_overlap( double xMin, double xMax,
                                    double yMin, double yMax, int excl )
{
   bool isMid = bg_point_type->checkedId() == MIDPOINTS;
   for ( int ii= 0; ii < partial_grid_info.size(); ii++ ) {
      if ( ii == excl ) continue;
      GridInfo ginfo = partial_grid_info.at( ii );
      double x1 = ginfo.xMin;
      double x2 = ginfo.xMax;
      double y1 = ginfo.yMin;
      double y2 = ginfo.yMax;
      bool x_ovrlp, y_ovrlp;
      if ( isMid ) {
         x_ovrlp = ! ( xMin >= x2 || xMax <= x1 );
         y_ovrlp = ! ( yMin >= y2 || yMax <= y1 );
      } else {
         x_ovrlp = ! ( xMin > x2 || xMax < x1 );
         y_ovrlp = ! ( yMin > y2 || yMax < y1 );
      }
      if ( x_ovrlp && y_ovrlp ) {
         return true;
      }
   }
   return false;
}

bool US_Grid_Editor::validate_xyz( GridInfo& ginfo, QString& error )
{
   error.clear();

   double x_min = 0;
   double x_max = 0;
   double y_min = 0;
   double y_max = 0;
   double z_val = 0;
   int    x_res = 0;
   int    y_res = 0;
   QString x_min_t = le_x_min->text();
   QString x_max_t = le_x_max->text();
   QString x_res_t = le_x_res->text();
   QString y_min_t = le_y_min->text();
   QString y_max_t = le_y_max->text();
   QString y_res_t = le_y_res->text();
   QString z_val_t = le_z_val->text();

   error += validate_double( x_min_t, x_min ) ? "" : tr( "%1: Minimum value is not set.\n" ).arg( lb_x_ax->text() );
   error += validate_double( x_max_t, x_max ) ? "" : tr( "%1: Maximum value is not set.\n" ).arg( lb_x_ax->text() );
   error += validate_int   ( x_res_t, x_res ) ? "" : tr( "%1: Resolusion value is not set.\n" ).arg( lb_x_ax->text() );
   error += validate_double( y_min_t, y_min ) ? "" : tr( "%1: Minimum value is not set.\n" ).arg( lb_y_ax->text() );
   error += validate_double( y_max_t, y_max ) ? "" : tr( "%1: Maximum value is not set.\n" ).arg( lb_y_ax->text() );
   error += validate_int   ( y_res_t, y_res ) ? "" : tr( "%1: Resolusion value is not set.\n" ).arg( lb_y_ax->text() );
   if ( rb_cons_z->isChecked() ) {
      error += validate_double( z_val_t, z_val ) ? "" : tr( "%1: Value is not set!" ).arg( lb_z_ax->text() );
   }
   else {
      if ( z_val_t.isEmpty() ) {
         error += tr( "%1: Function is not set!" ).arg( lb_z_ax->text() );
      }
   }

   if ( ! error.isEmpty() ) return false;

   if ( x_res == 0 || y_res == 0 ) {
      error = "Resolution value cannot be zero.";
      return false;
   }

   x_min = correct_unit( x_min, x_param, false );
   x_max = correct_unit( x_max, x_param, false );
   y_min = correct_unit( y_min, y_param, false );
   y_max = correct_unit( y_max, y_param, false );

   ginfo.xType = Attribute::symbol( x_param );
   ginfo.yType = Attribute::symbol( y_param );
   ginfo.zType = Attribute::symbol( z_param );

   ginfo.xMin = x_min;
   ginfo.xMax = x_max;
   ginfo.yMin = y_min;
   ginfo.yMax = y_max;
   ginfo.xRes = x_res;
   ginfo.yRes = y_res;

   if ( rb_cons_z->isChecked() ) {
      z_val = correct_unit( z_val, z_param, false );
      ginfo.zVal = QString::number( z_val );
   }
   else {
      ginfo.zVal = z_val_t;
   }

   return true;
}

bool US_Grid_Editor::validate_double( const QString str, double& val )
{
   bool ok;
   double d = str.toDouble( &ok );
   if ( ok ) val = d;
   return ok;
}

bool US_Grid_Editor::validate_int( const QString str, int& val )
{
   bool ok;
   int i = str.toInt( &ok );
   if ( ok ) val = i;
   return ok;
}

bool US_Grid_Editor::validate_partial_grid( GridInfo& ginfo )
{
   QString error;
   check_dens_visc_temp();
   if ( ! validate_xyz( ginfo, error ) ) {
      QMessageBox msg_box;
      msg_box.setIcon( QMessageBox::Critical );
      msg_box.setWindowTitle( "Error" );
      msg_box.setText( "Invalid parameter settings detected." );
      QTextEdit *text = new QTextEdit( &msg_box );
      text->setPlainText( error );
      text->setReadOnly( true );
      text->setMinimumSize( 300, 150 );
      QGridLayout *layout = qobject_cast<QGridLayout *>( msg_box.layout() );
      if ( layout ) {
         layout->addWidget( text, layout->rowCount(), 0, 1, layout->columnCount() );
      }
      msg_box.exec();
      return false;
   }

   GridPoint gp;
   gp.set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
   QVector<Attribute::Type> types;
   types << x_param << y_param << z_param;
   QVector<double> vals;
   if ( ! calc_xyz( ginfo.xMin, ginfo.yMin, ginfo.zVal, vals ) ) {
      return false;
   }
   if ( ! gp.set_param( vals, types ) ) {
      error = gp.error_string();
   } else {
      if ( ! calc_xyz( ginfo.xMax, ginfo.yMax, ginfo.zVal, vals ) ) {
         return false;
      }
      if ( ! gp.set_param( vals, types ) ) {
         error = gp.error_string();
      }
   }

   if ( ! error.isEmpty() ) {
      QMessageBox::critical( this, "Error", error );
      return false;
   }

   return true;

}

void US_Grid_Editor::fill_list()
{
   lw_grids->disconnect();

   int row = lw_grids->currentRow();
   lw_grids->clear();
   int n_grids = partial_grids.size();
   QString title = "%1 ) %2 = %3 to %4 ; %5 = %6 to %7 ; %8 = %9";

   for ( int ii = 0; ii < n_grids; ii++ ) {
      GridInfo ginfo = partial_grid_info.at( ii );
      double x1 = correct_unit( ginfo.xMin, Attribute::from_symbol( ginfo.xType ), true );
      double x2 = correct_unit( ginfo.xMax, Attribute::from_symbol( ginfo.xType ), true );
      double y1 = correct_unit( ginfo.yMin, Attribute::from_symbol( ginfo.yType ), true );
      double y2 = correct_unit( ginfo.yMax, Attribute::from_symbol( ginfo.yType ), true );
      bool constant;
      QString type, dpnt;
      QVector<double> prm;
      parse_z_expression( ginfo.zVal, type, dpnt, prm, constant );
      QString z_str;
      if ( constant ) {
         double z  = correct_unit( prm.first(), Attribute::from_symbol( ginfo.zType ), true );
         z_str = QString::number( z );
      }
      else {
         z_str = type;
      }

      QString str = title.arg( ii + 1 ).
                    arg( ginfo.xType ).arg( x1 ).arg( x2 ).
                    arg( ginfo.yType ).arg( y1 ).arg( y2 ).
                    arg( ginfo.zType ).arg( z_str );
      lw_grids->addItem( str );
   }

   if ( row == -1 || row >= lw_grids->count() ) row = 0;
   lw_grids->setCurrentRow( row );
   connect( lw_grids, &QListWidget::currentRowChanged, this, &US_Grid_Editor::select_partial_grid );
}

void US_Grid_Editor::sort_points()
{
   sorted_points.clear();
   QVector< QMap< QString, QVariant > > data;
   int max_rows = -1;
   for ( int ii = 0; ii < partial_grid_info.size(); ii++ ) {
      GridInfo ginfo = partial_grid_info.at( ii );
      max_rows = qMax( max_rows, ginfo.yRes );
      QMap< QString, QVariant > info;
      info.insert( "x_min" , ginfo.xMin );
      info.insert( "y_min" , ginfo.yMin );
      info.insert( "n_cols", ginfo.xRes );
      info.insert( "p_grid_id", ii );
      info.insert( "grid_id"  , 0 );
      data << info;
   }
   std::stable_sort( data.begin(), data.end(),
                    []( const QMap< QString, QVariant >& a, const QMap< QString, QVariant >& b){
                       return a.value( "x_min" ).toDouble() < b.value( "x_min" ).toDouble();
                    });
   std::stable_sort( data.begin(), data.end(),
                    []( const QMap< QString, QVariant >& a, const QMap< QString, QVariant >& b){
                       return a.value( "y_min" ).toDouble() < b.value( "y_min" ).toDouble();
                    });
   for ( int row = 0; row < max_rows; row++ ) {
      for ( int dd = 0; dd < data.size(); dd++ ) {
         int ii = data.at( dd ).value( "p_grid_id" ).toInt();
         int aa = data.at( dd ).value( "grid_id" ).toInt();
         int bb = aa + data.at( dd ).value( "n_cols" ).toInt();
         int sz = partial_grids.at( ii ).size();
         data[ dd ][ "grid_id" ] = bb;
         for ( int jj = aa; jj < bb; jj++ ) {
            if ( jj >= sz ) {
               break;
            }
            sorted_points << partial_grids.at( ii ).at( jj );
         }
      }
   }
}

void US_Grid_Editor::sort_partial_grid_points( QVector<GridPoint> &vec )
{
   std::stable_sort( vec.begin(), vec.end(),
                    []( const GridPoint &g1, const GridPoint &g2 ) {
                       if ( g1.get_row() != g2.get_row() ) {
                          return g1.get_row() < g2.get_row();
                       }
                       else {
                          return g1.get_col() < g2.get_col();
                       }
                    });
}

void US_Grid_Editor::check_dens_visc_temp()
{
   buff_dens = DENS_20W;
   buff_visc = VISC_20W;
   buff_temp = 20;
   validate_double( le_dens_20->text(), buff_dens );
   validate_double( le_visc_20->text(), buff_visc );
   validate_double( le_temp->text(), buff_temp );

   if ( buff_dens < 0.001 ) {
      QMessageBox::warning( this, "Warning!", "The entered buffer density is below 0.001, "
                                             "so it has been reset to the density of water at 20°C." );
      buff_dens = DENS_20W;
   }
   if ( buff_visc < 0.001 ) {
      QMessageBox::warning( this, "Warning!", "The entered buffer viscosity is below 0.001, "
                                             "so it has been reset to the viscosity of water at 20°C." );
      buff_visc = VISC_20W;
   }
   if ( buff_temp < 0 ) {
      QMessageBox::warning( this, "Warning!", "The entered buffer temperature is negative, "
                                             "so it has been reset to 20°C." );
      buff_temp = 20;
   }

   if ( buffer_type == ARBITRARY ) {
      if ( buff_dens == DENS_20W && buff_visc == VISC_20W ) {
         buffer_type = ST_WATER;
         le_buffer->setText( "Water" );
      } else {
         le_buffer->setText( "Arbitrary Values!!!" );
      }
   }

   le_dens_20->setText( QString::number( buff_dens ) );
   le_visc_20->setText( QString::number( buff_visc ) );
   le_temp->setText( QString::number( buff_temp ) );
}

void US_Grid_Editor::new_dens_visc_temp()
{
   buffer_type = ARBITRARY;
   pb_lu_buffer->setText( "Update Buffer" );
   pb_lu_buffer->setStyleSheet( "QPushButton { background-color: yellow }" );
}

void US_Grid_Editor::check_grid_id()
{
   for ( int ii = 0; ii < partial_grids.size(); ii++ ) {
      double xMin = partial_grid_info.at( ii ).xMin;
      double xMax = partial_grid_info.at( ii ).xMax;
      double yMin = partial_grid_info.at( ii ).yMin;
      double yMax = partial_grid_info.at( ii ).yMax;
      QPair< double, double > xrng( xMin, xMax );
      QPair< double, double > yrng( yMin, yMax );
      for ( int jj = 0; jj < partial_grids.at( ii ).size(); jj++ ) {
         partial_grids[ ii ][ jj ].set_id_range( ii, xrng, yrng );
      }
   }
}

bool US_Grid_Editor::check_minmax( const QString &type )
{
   double min = 0;
   double max = 0;
   bool   min_set = false;
   bool   max_set = false;
   if ( type.startsWith( "X_" ) ) {
      min_set = validate_double( le_x_min->text(), min );
      max_set = validate_double( le_x_max->text(), max );
   } else if ( type.startsWith( "Y_" ) ) {
      min_set = validate_double( le_y_min->text(), min );
      max_set = validate_double( le_y_max->text(), max );
   }
   else {
      return false;
   }

   if ( min_set && max_set ) {
      if ( max > min ) return true;
      else             return false;
   } else if ( type.endsWith( "_MIN" ) && min_set ) {
      return true;
   } else if ( type.endsWith( "_MAX" ) && max_set ) {
      return true;
   } else {
      return false;
   }
}

void US_Grid_Editor::new_xMin()
{
   if ( ! check_minmax( "X_MIN" ) ) {
      le_x_min->clear();
   }
   plot_tmp();
}

void US_Grid_Editor::new_xMax()
{
   if ( ! check_minmax( "X_MAX" ) ) {
      le_x_max->clear();
   }
   plot_tmp();
}

void US_Grid_Editor::new_yMin()
{
   if ( ! check_minmax( "Y_MIN" ) ) {
      le_y_min->clear();
   }
   plot_tmp();
}

void US_Grid_Editor::new_yMax()
{
   if ( ! check_minmax( "Y_MAX" ) ) {
      le_y_max->clear();
   }
   plot_tmp();
}

void US_Grid_Editor::new_zVal()
{
   double val;
   if ( ! validate_double( le_z_val->text(), val ) ) {
      le_z_val->clear();
   }
}

void US_Grid_Editor::set_symbol_size( double )
{
   int ss = ct_size->value();
   for ( int ii = 0; ii < point_curves.size(); ii++ ) {
      const QwtSymbol* symbol = point_curves[ii]->symbol();
      QwtSymbol* new_symbol = new QwtSymbol();
      new_symbol->setStyle( symbol->style() );
      new_symbol->setBrush( symbol->brush() );
      new_symbol->setPen( symbol->pen() );
      new_symbol->setSize( ss, ss );
      point_curves[ii]->setSymbol( new_symbol );
   }
   ss++;
   const QwtSymbol* symbol = subgrid_curve->symbol();
   QwtSymbol* new_symbol = new QwtSymbol();
   new_symbol->setStyle( symbol->style() );
   new_symbol->setBrush( symbol->brush() );
   new_symbol->setPen( symbol->pen() );
   new_symbol->setSize( ss, ss );
   subgrid_curve->setSymbol( new_symbol );

   data_plot->replot();
}

void US_Grid_Editor::buffer_selected( US_Buffer buffer )
{
   le_dens_20->setText( QString::number( buffer.density ) );
   le_visc_20->setText( QString::number( buffer.viscosity ) );
   le_buffer->setText( buffer.description );
   buffer_type = USER_BUFFER;
}

void US_Grid_Editor::set_buffer()
{
   if ( pb_lu_buffer->text().startsWith( "Load Buffer" ) ) {
      int state;
      if ( dkdb_cntrls->db() ) {
         state = US_Disk_DB_Controls::DB;
      } else {
         state = US_Disk_DB_Controls::Disk;
      }
      US_BufferGui* buffer_gui = new US_BufferGui( true, US_Buffer(), state );
      connect( buffer_gui, QOverload<US_Buffer>::of( &US_BufferGui::valueChanged ),
               this      , &US_Grid_Editor::buffer_selected );
      buffer_gui->exec();
   }

   QString qs = "QPushButton { background-color: %1 }";
   QColor color = US_GuiSettings::pushbColor().color( QPalette::Active, QPalette::Button );
   pb_lu_buffer->setStyleSheet( qs.arg( color.name() ) );
   pb_lu_buffer->setText( "Load Buffer" );

   check_dens_visc_temp();

   for ( int ii = 0; ii < partial_grids.size(); ii++ ) {
      for ( int jj = 0; jj < partial_grids.at( ii ).size(); jj++ ) {
         partial_grids[ii][jj].set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
      }
   }

   for ( int ii = 0; ii < sorted_points.size(); ii++ ) {
      sorted_points[ii].set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
   }

   US_Math2::SolutionData sd;
   sd.manual = false;
   sd.vbar = 0.72;
   sd.vbar20 = 0.72;
   sd.density = buff_dens;
   sd.viscosity = buff_visc;
   US_Math2::data_correction( buff_temp, sd );
   if ( buff_temp == 20 ) {
      le_dens_T->setText( QString::number( buff_dens ) );
      le_visc_T->setText( QString::number( buff_visc ) );
   } else {
      le_dens_T->setText( QString::number( sd.density_tb ) );
      le_visc_T->setText( QString::number( sd.viscosity_tb ) );
   }

   plot_points();
   select_partial_grid( lw_grids->currentRow() );
}

void US_Grid_Editor::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

void US_Grid_Editor::setup_grid()
{
   if ( ! partial_grids.isEmpty() ) {
      int res = QMessageBox::question( this, "Warning!",
                                      tr( "The grid points will be deleted if you proceed with the grid setup?" ) );
      if ( res == QMessageBox::No ) return;
   }

   delete_data();
   default_plot_ctrl();

   US_Grid_Preset *grid_preset = new US_Grid_Preset( this, x_param, y_param, z_param );
   if ( grid_preset->exec() != QDialog::Accepted ) return;

   Attribute::Type xp, yp, zp;
   grid_preset->parameters( xp, yp, zp );
   if ( xp == x_param && yp == y_param && zp == z_param ) return;
   x_param = xp;
   y_param = yp;
   z_param = zp;
   le_x_param->setText( Attribute::long_desc( x_param ) );
   le_y_param->setText( Attribute::long_desc( y_param ) );
   le_z_param->setText( Attribute::long_desc( z_param ) );
   lb_x_ax->setText( Attribute::short_desc( x_param ) );
   lb_y_ax->setText( Attribute::short_desc( y_param ) );
   lb_z_ax->setText( Attribute::short_desc( z_param ) );
   QString s = tr( "plot ( %1 ) over ( %2 )" ).
               arg( Attribute::symbol( y_param), Attribute::symbol( x_param ) );
   pb_default_plot->setText( s );

   default_plot_ctrl();
}

void US_Grid_Editor::load()
{
   bool load_db = dkdb_cntrls->db();
   QString mfilter ( "CustomGrid" );
   QString mdesc;
   US_Model model;

   US_ModelLoader* mloader = new US_ModelLoader( load_db, mfilter, model, mdesc, "" );
   if ( mloader->exec() != QDialog::Accepted ) return;

   if ( ! partial_grids.isEmpty() ) {
      int yes = QMessageBox::question( this, "Warning!",
                                      tr( "Grid list is not empty and it will "
                                         "be deleted by loading a model.<br/>To proceed with "
                                         "loading a custom grid model, press <b>\"Yes\"</b> button." ) );
      if ( yes == QMessageBox::No ) return;
   }

   delete_data();
   default_plot_ctrl();
   check_dens_visc_temp();

   int nsubgrids = model.subGrids;
   QVector<Attribute::Type> types;
   bool cgmdata = ! model.customGridData.grids.isEmpty() &&
                  ! model.customGridData.components.isEmpty() &&
                  model.customGridData.components.size() == model.components.size();
   if ( cgmdata ) {
      rb_cg_mode->setChecked( true );
      set_cg_mode();
      partial_grid_info << model.customGridData.grids;
      Attribute::Type xt = Attribute::from_symbol( partial_grid_info.first().xType );
      Attribute::Type yt = Attribute::from_symbol( partial_grid_info.first().yType );
      Attribute::Type zt = Attribute::from_symbol( partial_grid_info.first().zType );
      if ( xt == Attribute::ATTR_NONE || yt == Attribute::ATTR_NONE || zt ==  Attribute::ATTR_NONE ) {
         QMessageBox::warning( this, "Warning!", "Incorrect grid type!"
                                                "<br/><br/><b>" + mdesc + "</b>" );
         return;
      }
      x_param = xt;
      y_param = yt;
      z_param = zt;
      types << x_param << y_param << z_param;

      le_x_param->setText( Attribute::long_desc( x_param ) );
      le_y_param->setText( Attribute::long_desc( y_param ) );
      le_z_param->setText( Attribute::long_desc( z_param ) );
      lb_x_ax->setText( Attribute::short_desc( x_param ) );
      lb_y_ax->setText( Attribute::short_desc( y_param ) );
      lb_z_ax->setText( Attribute::short_desc( z_param ) );

      chkb_log->disconnect();
      bg_point_type->disconnect();
      chkb_log->setChecked( model.customGridData.xLogarithmic );
      bool isMid = model.customGridData.midpointBins;
      bg_point_type->button( MIDPOINTS )->setChecked( isMid );
      connect( chkb_log, &QCheckBox::stateChanged, this, &US_Grid_Editor::refill_grid_points );
      connect( bg_point_type, &QButtonGroup::idClicked, this, &US_Grid_Editor::set_mid_exct_points );

      for ( int ii = 0; ii < partial_grid_info.size(); ii++ ) {
         QVector<GridPoint> gpvec;
         partial_grids << gpvec;
      }

      for ( int ii = 0; ii < model.components.size(); ii++ ) {
         US_Model::SimulationComponent sc = model.components.at( ii );
         int id  = model.customGridData.components.at( ii ).grid_id;
         int row = model.customGridData.components.at( ii ).row;
         int col = model.customGridData.components.at( ii ).column;

         GridPoint gp;
         gp.set_component( sc, &types );
         gp.set_row_col( row, col );
         gp.set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
         partial_grids[ id ] << gp;
      }

      for ( int ii = 0; ii < partial_grids.size(); ii++ ) {
         sort_partial_grid_points( partial_grids[ii] );
      }
      check_grid_id();
      fill_list();
      sort_points();

   } else {
      rb_csv_mode->setChecked( true );
      set_cg_mode();
      types << x_param << y_param << z_param;

      QVector<GridPoint> gpoints;
      for ( int ii = 0; ii < model.components.size(); ii++ ) {
         US_Model::SimulationComponent sc = model.components.at( ii );

         GridPoint gp;
         gp.set_component( sc, &types );
         gp.set_row_col( 0, 0 );
         gp.set_dens_visc_temp( buff_dens, buff_visc, buff_temp );
         gpoints << gp;
      }
      partial_grids << gpoints;
      sorted_points << gpoints;
   }

   plot_points();
   set_nsubgrids( nsubgrids );
   default_plot_ctrl();
}

void US_Grid_Editor::save()
{
   if ( final_subgrids.first().size() > 150 ) {
      QMessageBox::warning( this, "Warning!", "There are too many components in each subgrid. "
                                             "Threshold is <b>150<b/> components.<br/>"
                                             "Please increase the number of subgrids, then try again." );
      return;
   }

   US_Model model;
   QString modelPath;
   US_Model::model_path( modelPath );
   QDateTime now_time = QDateTime::currentDateTime ();
   QString modelGuid = US_Util::new_guid();
   model.analysis    = US_Model::CUSTOMGRID;
   model.description = now_time.toString( "yyyyMMdd-hhmm" )
                       + "-CustomGrid" + ".model";
   model.subGrids    = final_subgrids.size();
   model.modelGUID   = modelGuid;
   model.global      = US_Model::NONE;

   model.customGridData.grids.clear();
   model.customGridData.components.clear();

   if ( rb_cg_mode->isChecked() ) {
      model.customGridData.grids << partial_grid_info;
      model.customGridData.xLogarithmic = chkb_log->isChecked();
      model.customGridData.midpointBins = bg_point_type->checkedId() == MIDPOINTS;
   }

   for ( int ii = 0; ii < sorted_points.size(); ii++ )
   {
      GridPoint gp = sorted_points.at( ii );
      US_Model::SimulationComponent sc;
      sc.vbar20 = gp.value( Attribute::ATTR_V );
      sc.s      = gp.value( Attribute::ATTR_S );
      sc.D      = gp.value( Attribute::ATTR_D );
      sc.f_f0   = gp.value( Attribute::ATTR_K );
      sc.f      = gp.value( Attribute::ATTR_F );
      sc.mw     = gp.value( Attribute::ATTR_M );
      QString name;
      if ( rb_cg_mode->isChecked() ) {
         int ss = ( ii % model.subGrids ) + 1;
         int pp = ( ii / model.subGrids ) + 1;
         name = QString( "sg%1_p%2" )
                   .arg( ss, 3, 10, QChar( '0' ) )
                   .arg( pp, 3, 10, QChar( '0' ) );

      } else {
         name = tr( "P_%1" ).arg( ii + 1, 4, 10, QChar( '0' ) );
      }

      sc.name     = name;
      model.components << sc;

      CompInfo cinfo;
      cinfo.grid_id = gp.get_id();
      cinfo.row     = gp.get_row();
      cinfo.column  = gp.get_col();
      model.customGridData.components << cinfo;
   }

   // Open a dialog that reports and allows modification of description
   QMessageBox mbox;
   QString msg1 = tr( "A grid model has been created. "
                     "It's description is:<br/>"
                     "<b>%1</b>.<br/><br/>"
                     "Click:<br/><br/>"
                     "<b>OK</b> to output the model as is;<br/>"
                     "<b>Edit</b> to append custom text to the name;<br/>"
                     "<b>Cancel</b> to abort model creation.<br/>" ).arg( model.description );

   mbox.setWindowTitle( tr( "Save Grid Model" ) );
   mbox.setText       ( msg1 );
   QPushButton *pb_ok   = mbox.addButton( tr( "OK" ),
                                       QMessageBox::YesRole );
   QPushButton *pb_edit = mbox.addButton( tr( "Edit" ) ,
                                         QMessageBox::AcceptRole );
   QPushButton *pb_canc = mbox.addButton( tr( "Cancel" ),
                                         QMessageBox::RejectRole );
   mbox.setEscapeButton ( pb_canc );
   mbox.setDefaultButton( pb_ok );

   mbox.exec();

   if ( mbox.clickedButton() == pb_canc )  return;

   if ( mbox.clickedButton() == pb_edit )
   {  // Open another dialog to get a modified runID
      bool    ok;
      QString newtext = "";
      int     jj      = model.description.indexOf( ".model" );
      if ( jj > 0 ) model.description = model.description.left( jj );
      QString msg2    = tr( "The default run ID for the grid model"
                        "<br/> is <b>%1</b>.<br/><br/>"
                        "You may append additional text to the model description.<br/>"
                        "Use alphanumeric characters, underscores, or hyphens<br/>"
                        "( no spaces ). Enter 1 to 40 characters." ).arg( model.description );

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
   int code = US_DB2::NOT_CONNECTED;
   QString db_error;
   if ( dkdb_cntrls->db() )
   {
      US_Passwd pw;
      US_DB2 db( pw.getPasswd() );
      if ( db.isConnected() ) {
         code = model.write( &db );
         db_error = db.lastError();
      } else {
         db_error = db.lastError();
      }
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
                               tr( "The file <br/><b>%1</b><br/>"
                                  "was successfully saved to %2" ).
                               arg( model.description, destination ) );
   }
   else
   {
      QMessageBox::warning( this, mtitle,
                           tr( "Writing the model file <br/><b>%1</b><br/>"
                              "resulted in the following error:<br/><br/>%2" ).arg( model.description, db_error ) );
   }
}

void US_Grid_Editor::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();
   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();
   investigator = US_Settings::us_inv_ID();
   QString inv_text = tr( "%1: %2" ).arg( investigator ).arg( US_Settings::us_inv_name() );
   le_investigator->setText( inv_text );
}

void US_Grid_Editor::set_zval_type( int id )
{
   le_z_val->clear();
   if ( id == 0 )
   {
      le_z_val->show();
      pb_z_set_func->hide();
   }
   else
   {
      le_z_val->hide();
      pb_z_set_func->show();
   }
}

void US_Grid_Editor::set_z_function()
{
   QMap<QString, QString> setting;
   setting.insert( "x_short_title", Attribute::symbol( x_param ) );
   setting.insert( "y_short_title", Attribute::symbol( y_param ) );
   setting.insert( "z_short_title", Attribute::symbol( z_param ) );
   setting.insert( "x_long_title" , Attribute::short_desc( x_param ) );
   setting.insert( "y_long_title" , Attribute::short_desc( y_param ) );
   setting.insert( "z_long_title" , Attribute::short_desc( z_param ) );
   setting.insert( "parameters"   , le_z_val->text() );

   QString x_min_t = le_x_min->text();
   QString x_max_t = le_x_max->text();
   QString y_min_t = le_y_min->text();
   QString y_max_t = le_y_max->text();

   double x_min = 0;
   double x_max = 0;
   double y_min = 0;
   double y_max = 0;
   QString error;

   error += validate_double( x_min_t, x_min ) ? "" : tr( "%1: Minimum value is not set.\n" ).arg( lb_x_ax->text() );
   error += validate_double( x_max_t, x_max ) ? "" : tr( "%1: Maximum value is not set.\n" ).arg( lb_x_ax->text() );
   error += validate_double( y_min_t, y_min ) ? "" : tr( "%1: Minimum value is not set.\n" ).arg( lb_y_ax->text() );
   error += validate_double( y_max_t, y_max ) ? "" : tr( "%1: Maximum value is not set.\n" ).arg( lb_y_ax->text() );

   if ( ! error.isEmpty() ) {
      QMessageBox::warning( this, "Warning!", error );
      return;
   }

   setting.insert( "x_min" , x_min_t );
   setting.insert( "x_max" , x_max_t );
   setting.insert( "y_min" , y_min_t );
   setting.insert( "y_max" , y_max_t );

   US_Grid_ZFunction *diag = new US_Grid_ZFunction( this, setting );
   if ( diag->exec() == QDialog::Accepted ) {
      le_z_val->clear();
      QString params = diag->get_parameters();
      qDebug() << params;
      le_z_val->setText( params );
   }
}

US_Grid_Preset::US_Grid_Preset( QWidget * parent, Attribute::Type x,
                               Attribute::Type y, Attribute::Type z ) : US_WidgetsDialog( parent )
{
   setWindowTitle( tr( "Grid Setup" ) );
   setPalette( US_GuiSettings::frameColor() );
   setFixedSize( 500, 250 );

   QLabel *lb_x_axis = us_label( "X Axis" );
   lb_x_axis->setAlignment( Qt::AlignCenter );
   QLabel *lb_y_axis = us_label( "Y Axis" );
   lb_y_axis->setAlignment( Qt::AlignCenter );

   QGridLayout* x_s     = us_radiobutton( Attribute::long_desc( Attribute::ATTR_S ), rb_x_s, true );
   QGridLayout* x_ff0   = us_radiobutton( Attribute::long_desc( Attribute::ATTR_K ), rb_x_ff0, false );
   QGridLayout* x_mw    = us_radiobutton( Attribute::long_desc( Attribute::ATTR_M ), rb_x_mw, true );
   QGridLayout* x_vbar  = us_radiobutton( Attribute::long_desc( Attribute::ATTR_V ), rb_x_vbar, true );
   QGridLayout* x_D     = us_radiobutton( Attribute::long_desc( Attribute::ATTR_D ), rb_x_D, true );

   QGridLayout* y_s     = us_radiobutton( Attribute::long_desc( Attribute::ATTR_S ), rb_y_s, false );
   QGridLayout* y_ff0   = us_radiobutton( Attribute::long_desc( Attribute::ATTR_K ), rb_y_ff0, true );
   QGridLayout* y_mw    = us_radiobutton( Attribute::long_desc( Attribute::ATTR_M ), rb_y_mw, true );
   QGridLayout* y_vbar  = us_radiobutton( Attribute::long_desc( Attribute::ATTR_V ), rb_y_vbar, true );
   QGridLayout* y_D     = us_radiobutton( Attribute::long_desc( Attribute::ATTR_D ), rb_y_D, true );

   x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    Attribute::ATTR_S );
   x_axis->addButton( rb_x_ff0,  Attribute::ATTR_K );
   x_axis->addButton( rb_x_mw,   Attribute::ATTR_M );
   x_axis->addButton( rb_x_vbar, Attribute::ATTR_V );
   x_axis->addButton( rb_x_D,    Attribute::ATTR_D );

   y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    Attribute::ATTR_S );
   y_axis->addButton( rb_y_ff0,  Attribute::ATTR_K );
   y_axis->addButton( rb_y_mw,   Attribute::ATTR_M );
   y_axis->addButton( rb_y_vbar, Attribute::ATTR_V );
   y_axis->addButton( rb_y_D,    Attribute::ATTR_D );

   QLabel *lb_fixed      = us_label( tr( "Fixed Attribute" ) );
   lb_fixed->setAlignment( Qt::AlignCenter );

   z_axis = us_comboBox();

   QPushButton *pb_apply = us_pushbutton( "Apply" );
   QFont font = pb_apply->font();
   font.setBold( true );
   pb_apply->setFont( font );
   pb_apply->setIcon( this->style()->standardIcon( QStyle::SP_DialogApplyButton ) );
   QPushButton *pb_cancel = us_pushbutton( "Cancel" );
   pb_cancel->setIcon( this->style()->standardIcon( QStyle::SP_DialogCancelButton ) );
   pb_cancel->setFont( font );

   QFrame *hline1 = new QFrame();
   hline1->setFrameShape( QFrame::HLine );
   hline1->setFrameShadow( QFrame::Sunken );

   QGridLayout* layout = new QGridLayout();
   layout->setMargin( 2 );
   layout->setSpacing( 3 );
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
   layout->addWidget( hline1,     row++, 0, 1, 4 );
   layout->addWidget( lb_fixed,   row,   0, 1, 2 );
   layout->addWidget( z_axis,     row++, 2, 1, 2 );

   QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
   layout->addItem( spacer,                 row++,  0, 1, 4 );

   layout->addWidget( pb_apply,              row,   2, 1, 1 );
   layout->addWidget( pb_cancel,             row++, 3, 1, 1 );

   setLayout( layout );

   x_param = x;
   y_param = y;
   z_param = z;
   x_axis->button( x_param )->setChecked( true );
   y_axis->button( y_param )->setChecked( true );
   select_x_axis( x_param );
   select_y_axis( y_param );
   z_axis->setCurrentIndex( z_axis->findData( z_param, Qt::UserRole ) );

   connect( x_axis,    &QButtonGroup::idReleased, this, &US_Grid_Preset::select_x_axis );
   connect( y_axis,    &QButtonGroup::idReleased, this, &US_Grid_Preset::select_y_axis );
   connect( pb_apply,  &QPushButton::clicked,     this, &US_Grid_Preset::apply );
   connect( pb_cancel, &QPushButton::clicked,     this, &US_Grid_Preset::cancel );
}

void US_Grid_Preset::parameters( Attribute::Type& x, Attribute::Type& y, Attribute::Type& z )
{
   x = x_param;
   y = y_param;
   z = z_param;
}

void US_Grid_Preset::select_x_axis( int index )
{
   QVector<Attribute::Type> tlist;
   tlist << Attribute::ATTR_S << Attribute::ATTR_K
         << Attribute::ATTR_M << Attribute::ATTR_V
         << Attribute::ATTR_D;
   foreach ( Attribute::Type type, tlist ) {
      y_axis->button( type )->setEnabled( true );
   }

   x_param = Attribute::from_int( index );
   y_axis->button( x_param )->setDisabled( true );
   set_z_axis();
}

void US_Grid_Preset::select_y_axis( int index )
{
   QVector<Attribute::Type> tlist;
   tlist << Attribute::ATTR_S << Attribute::ATTR_K
         << Attribute::ATTR_M << Attribute::ATTR_V
         << Attribute::ATTR_D;
   foreach ( Attribute::Type type, tlist ) {
      x_axis->button( type )->setEnabled( true );
   }

   y_param = Attribute::from_int( index );
   x_axis->button( y_param )->setDisabled( true );
   set_z_axis();
}

void US_Grid_Preset::select_z_axis( int index )
{
   int id = z_axis->itemData( index, Qt::UserRole ).toInt();
   z_param = Attribute::from_int( id );
}

void US_Grid_Preset::apply()
{
   accept();
}

void US_Grid_Preset::cancel()
{
   reject();
}

void US_Grid_Preset::set_z_axis()
{
   z_axis->disconnect();
   z_axis->clear();

   QVector<Attribute::Type> tlist;
   tlist << Attribute::ATTR_S << Attribute::ATTR_K
         << Attribute::ATTR_M << Attribute::ATTR_V
         << Attribute::ATTR_D;

   QSet<Attribute::Type> skm;
   skm.insert( Attribute::ATTR_S );
   skm.insert( Attribute::ATTR_K );
   skm.insert( Attribute::ATTR_M );

   foreach ( Attribute::Type type, tlist ) {
      if ( type == x_param || type == y_param ) continue;
      QSet<Attribute::Type> xyz;
      xyz.insert( x_param );
      xyz.insert( y_param );
      xyz.insert( type );
      if ( xyz.contains( skm ) ) continue;
      z_axis->addItem( Attribute::long_desc( type ), type );
   }

   int index = z_axis->findData( z_param, Qt::UserRole );
   if ( index == -1 ) {
      z_axis->setCurrentIndex( 0 );
      int id = z_axis->itemData( 0, Qt::UserRole ).toInt();
      z_param = Attribute::from_int( id );
   } else {
      z_axis->setCurrentIndex( index );
   }

   connect( z_axis, QOverload<int>::of( &QComboBox::currentIndexChanged ),
           this,   &US_Grid_Preset::select_z_axis );
}

GridPoint::GridPoint()
{
   dvt_set = false;
   S_real = 0;
   D_real = 0;
}

bool GridPoint::set_param( const QVector<double>& values,
                           const QVector<Attribute::Type>& types )
{
   if ( values.size() != 3 || types.size() != 3 ) {
      error = "Three parameters are needed to set the grid point.";
      return false;
   }
   ptypes.clear();

   for ( int ii = 0; ii < 3; ii++ ) {
      ptypes.insert( types.at( ii ) );

      if      ( ii == 0 ) x_param = types.at( ii );
      else if ( ii == 1 ) y_param = types.at( ii );
      else if ( ii == 2 ) z_param = types.at( ii );

      if ( types.at( ii ) == Attribute::ATTR_S ) {
         S = values.at( ii );
         if ( S == 0 ) {
            error = "Sedimentation value is zero. Please correct it!";
            return false;
         }
      } else if ( types.at( ii ) == Attribute::ATTR_K ) {
         FF0 = values.at( ii );
         if ( FF0 < 1 ) {
            error = "Frictional ratio is less than one. Please correct it!";
            return false;
         }
      } else if ( types.at( ii ) == Attribute::ATTR_M ) {
         MW = values.at( ii );
         if ( MW < 1 ) {
            error = "Molecular weight is less than one. Please correct it!";
            return false;
         }
      } else if ( types.at( ii ) == Attribute::ATTR_V ) {
         VBAR = values.at( ii );
         if ( VBAR <= 0 ) {
            error = "Partial specific volume is less than or equal to zero. Please correct it!";
            return false;
         }
      } else if ( types.at( ii ) == Attribute::ATTR_D ) {
         D = values.at( ii );
         if ( D <= 0 ) {
            error = "Diffusion coefficient is less than or equal to zero. Please correct it!";
            return false;
         }
      } else if ( types.at( ii ) == Attribute::ATTR_F ) {
         F = values.at( ii );
         if ( F <= 0 ) {
            error = "Frictional coefficient is less than or equal to zero. Please correct it!";
            return false;
         }
      } else {
         error = "Parameter type is not found.";
         return false;
      }
   }
   if ( ! calculate_20w() ) {
      return false;
   }
   if ( dvt_set ) {
      calculate_real();
   }
   return true;
}

void GridPoint::set_dens_visc_temp( double dens, double visc, double T )
{
   density = dens;
   viscosity = visc;
   temperature = T;
   dvt_set = true;
   if ( ptypes.size() == 3 ) {
      calculate_real();
   }
}

void GridPoint::set_component( const US_Model::SimulationComponent &sc,
                               const QVector<Attribute::Type>* types )
{
   if ( types != nullptr ) {
      ptypes.clear();
      x_param = types->at( 0 );
      y_param = types->at( 1 );
      z_param = types->at( 2 );
      ptypes.insert( x_param );
      ptypes.insert( y_param );
      ptypes.insert( z_param );
   }
   S     = sc.s;
   D     = sc.D;
   VBAR  = sc.vbar20;
   MW    = sc.mw;
   F     = sc.f;
   FF0   = sc.f_f0;
   F0    = F / FF0;
}

QString GridPoint::error_string()
{
   return error;
}

double GridPoint::value( Attribute::Type type ) const
{
   double val = 0;
   if      ( type == Attribute::ATTR_S )  val = S;
   else if ( type == Attribute::ATTR_D )  val = D;
   else if ( type == Attribute::ATTR_V )  val = VBAR;
   else if ( type == Attribute::ATTR_M )  val = MW;
   else if ( type == Attribute::ATTR_F )  val = F;
   else if ( type == Attribute::ATTR_K )  val = FF0;
   else if ( type == Attribute::ATTR_F0 ) val = F0;
   else if ( type == Attribute::ATTR_SR ) val = S_real;
   else if ( type == Attribute::ATTR_DR ) val = D_real;
   return val;
}

void GridPoint::set_id_range( int i,
                              const QPair< double, double >& xrng,
                              const QPair< double, double >& yrng )
{
   id  = i;
   x_range = xrng;
   y_range = yrng;
}

void GridPoint::set_row_col( int r, int c )
{
   row = r;
   col = c;
}

int GridPoint::get_row() const
{
   return row;
}

int GridPoint::get_col() const
{
   return col;
}

QPair<double, double> GridPoint::get_x_range() const
{
   return x_range;
}

QPair<double, double> GridPoint::get_y_range() const
{
   return y_range;
}

int GridPoint::get_id() const
{
   return id;
}

double GridPoint::x_value () const
{
   return value( x_param );
}

double GridPoint::y_value () const
{
   return value( y_param );
}

double GridPoint::z_value () const
{
   return value( z_param );
}

void GridPoint::calc_coefficients()
{
   US_Model::SimulationComponent sc;
   sc.vbar20 = VBAR;
   sc.s      = S;
   sc.D      = D;
   sc.f_f0   = 0.0;
   sc.f      = 0.0;
   sc.mw     = 0.0;
   US_Model::calc_coefficients( sc );
   set_component( sc );
}

bool GridPoint::calculate_20w()
{
   error.clear();
   // S, K, M, V, D, F, F0
   const double RT   = R_GC * K20;
   const double PI   = M_PI;
   const double DENS = DENS_20W;
   const double VISC = VISC_20W * 0.01;
   const double NA   = AVOGADRO;

   if ( contains( Attribute::ATTR_V, Attribute::ATTR_S, Attribute::ATTR_K ) )         // 1: M, D, F, F0
   {
      if ( ! check_s_vbar() ) return false;
      double BUOY = 1 - VBAR * DENS;
      F0 = 9 * VISC * PI * std::sqrt( 2 * VBAR * VISC * S * FF0 / BUOY );
      F = FF0 * F0;
      D = RT / ( NA * F );
   } else if ( contains( Attribute::ATTR_V, Attribute::ATTR_S, Attribute::ATTR_M ) )  // 2: K, D, F, F0
   {
      if ( ! check_s_vbar() ) return false;
      double BUOY = 1 - VBAR * DENS;
      F = MW * BUOY / ( S * NA );
      D = RT / ( NA * F );
   } else if ( contains( Attribute::ATTR_V, Attribute::ATTR_S, Attribute::ATTR_D ) )  // 3: K, M, F, F0
   {
      if ( ! check_s_vbar() ) return false;
   } else if ( contains( Attribute::ATTR_V, Attribute::ATTR_K, Attribute::ATTR_M ) )  // 4: S, D, F, F0
   {
      double BUOY = 1 - VBAR * DENS;
      D = RT / ( 3 * VISC * FF0 ) /
          std::cbrt( 6 * MW * VBAR ) /
          std::pow( std::cbrt ( NA * PI ), 2 );
      S = MW * D * BUOY / RT;
   } else if ( contains( Attribute::ATTR_V, Attribute::ATTR_K, Attribute::ATTR_D ) )  // 5: S, M, F, F0
   {
      double BUOY = 1 - VBAR * DENS;
      F = RT / ( NA * D );
      F0 = F / FF0;
      S = std::pow( F0 / ( 9 * VISC * PI ), 2 ) * BUOY / ( 2 * VISC * VBAR * FF0 );
   } else if ( contains( Attribute::ATTR_V, Attribute::ATTR_M, Attribute::ATTR_D ) )  // 6: S, K, F, F0
   {
      double BUOY = 1 - VBAR * DENS;
      S = MW * D * BUOY / RT;
   } else if ( contains( Attribute::ATTR_S, Attribute::ATTR_K, Attribute::ATTR_M ) )  // 7: V, D, F, F0 ( qube root of VBAR )
   {
      double A = 6 * std::pow( 3 * VISC * S * FF0, 3 ) * std::pow( NA * PI / MW, 2 );
      double Q = A / DENS;
      double Z = std::sqrt( ( 27 * std::pow( Q, 2 ) * 4 * std::pow( Q, 3 ) ) / 108 );
      double X = std::cbrt( 0.5 * Q + Z ) + std::cbrt( 0.5 * Q - Z );
      VBAR = std::pow( X, 3 ) / A;

      if ( ! check_s_vbar() ) return false;
      double BUOY = 1 - VBAR * DENS;
      F0 = 9 * VISC * PI * std::sqrt( 2 * VBAR * VISC * S * FF0 / BUOY );
      F = FF0 * F0;
      D = RT / ( NA * F );
   } else if ( contains( Attribute::ATTR_S, Attribute::ATTR_K, Attribute::ATTR_D ) )  // 8: M, V, F, F0
   {
      F = RT / ( NA * D );
      F0 = F / FF0;
      double f02 = std::pow( F0, 2 );
      double vp2 = std::pow( 9 * PI * VISC, 2 );
      VBAR = f02 / ( 2 * VISC * S * FF0 * vp2 + f02 * DENS );
   } else if ( contains( Attribute::ATTR_S, Attribute::ATTR_M, Attribute::ATTR_D ) )  // 9: K, V, F, F0
   {
      VBAR = ( 1 - S * RT / ( MW * D ) ) / DENS;
   } else if ( contains( Attribute::ATTR_K, Attribute::ATTR_M, Attribute::ATTR_D ) )  // 10: S, V, F, F0
   {
      VBAR = std::pow( RT / ( 3.0 * VISC * D * FF0 ), 3 ) /
             ( 6 * MW * std::pow( NA * PI, 2 ) );
      double BUOY = 1 - VBAR * DENS;
      S = D * MW * BUOY / RT;
   }
   else return false;
   calc_coefficients();
   return check_s_vbar();
}

void GridPoint::calculate_real()
{
   if ( ! dvt_set ) return;
   if ( ptypes.isEmpty() ) return;

   US_Math2::SolutionData sd;
   sd.manual = false;
   sd.vbar = VBAR;
   sd.vbar20 = VBAR;
   sd.density = density;
   sd.viscosity = viscosity;
   US_Math2::data_correction( temperature, sd );
   S_real = S / sd.s20w_correction;
   D_real = D / sd.D20w_correction;

   DbgLv( 2 ) << "Solution Data:"
            << QObject::tr( "T=%1"        ).arg( temperature        )
            << QObject::tr( "dens_20W=%1" ).arg( DENS_20W           )
            << QObject::tr( "dens_TW=%1"  ).arg( sd.density_wt      )
            << QObject::tr( "visc_20W=%1" ).arg( VISC_20W           )
            << QObject::tr( "visc_TW=%1"  ).arg( sd.viscosity_wt    )
            << QObject::tr( "vbar20=%1"   ).arg( sd.vbar20          )
            << QObject::tr( "vbarT=%1"    ).arg( sd.vbar            )
            << QObject::tr( "dens_20B=%1" ).arg( sd.density         )
            << QObject::tr( "dens_TB =%1" ).arg( sd.density_tb      )
            << QObject::tr( "visc_20B=%1" ).arg( sd.viscosity       )
            << QObject::tr( "visc_TB=%1"  ).arg( sd.viscosity_tb    )
            << QObject::tr( "s=%1"        ).arg( S                  )
            << QObject::tr( "s_corr=%1"   ).arg( sd.s20w_correction )
            << QObject::tr( "s*=%1"       ).arg( S_real             )
            << QObject::tr( "D=%1"        ).arg( D                  )
            << QObject::tr( "D_corr=%1"   ).arg( sd.D20w_correction )
            << QObject::tr( "D*=%1"       ).arg( D_real             );
}

bool GridPoint::check_s_vbar()
{
   double buoy = 1 - VBAR * DENS_20W;
   if ( buoy == 0 ) {
      error = "Buoyancy is zero. Correct the vbar value.";
      return false;
   }
   if ( ( buoy > 0 && S < 0 ) || ( buoy < 0 && S > 0 ) ) {
      error  = "Sedimentation and buoyancy should have the same sign!<br/><br/>";
      error += QObject::tr( "<b>Sedimentation = %1<b/> [Sv] <br/>" ).arg( S * 1e+13 );
      error += QObject::tr( "<b>Buoyancy = 1 - Density( 20°C water ) x vbar<b/><br/>" );
      error += QObject::tr( "<b>Buoyancy = 1 - %1 x %2 = %3<b/><br/>" ).arg( DENS_20W ).arg( VBAR ).arg( buoy );
      return false;
   }
   if ( std::abs( S ) < S_TRSHL ) {
      error = QObject::tr( "s = %1 [Sv]. s values less than %2 [Sv] are not allowed!" )
                 .arg( S * 1e13 ).arg( S_TRSHL * 1e13 );
      return false;
   }
   return true;
}

bool GridPoint::contains( Attribute::Type p1, Attribute::Type p2, Attribute::Type p3 )
{
   return ptypes.contains( p1 ) && ptypes.contains( p2 ) && ptypes.contains( p3 );
}

QString Attribute::long_desc( Type t )
{
   QString str;
   if      ( t == Attribute::ATTR_S  ) str = "Sedimentation Coefficient";
   else if ( t == Attribute::ATTR_K  ) str = "Frictional Ratio";
   else if ( t == Attribute::ATTR_M  ) str = "Molecular Weight";
   else if ( t == Attribute::ATTR_V  ) str = "Partial Specific Volume";
   else if ( t == Attribute::ATTR_D  ) str = "Diffusion Coefficient";
   else if ( t == Attribute::ATTR_F  ) str = "Frictional Coefficient";
   else if ( t == Attribute::ATTR_F0 ) str = "Frictional Coefficient ( f0 )";
   else if ( t == Attribute::ATTR_SR ) str = "Sedimentation Coefficient ( Real )";
   else if ( t == Attribute::ATTR_DR ) str = "Diffusion Coefficient ( Real )";
   return str;
}

QString Attribute::short_desc( Type t )
{
   QString str;
   if      ( t == Attribute::ATTR_S  ) str = "s [ Sv ]";
   else if ( t == Attribute::ATTR_K  ) str = "f / f0";
   else if ( t == Attribute::ATTR_M  ) str = "MW [ kDa ]";
   else if ( t == Attribute::ATTR_V  ) str = "vbar [ mL / g ]";
   else if ( t == Attribute::ATTR_D  ) str = "<p>D [ cm<sup>2</sup> s<sup>-1</sup> ]</p>";
   else if ( t == Attribute::ATTR_F  ) str = "f [ g / s ]";
   else if ( t == Attribute::ATTR_F0 ) str = "f0 [ g / s ]";
   else if ( t == Attribute::ATTR_SR ) str = "s* [ Sv ]";
   else if ( t == Attribute::ATTR_DR ) str = "<p>D* [ cm<sup>2</sup> s<sup>-1</sup> ]</p>";
   return str;
}

QString Attribute::title( Type t )
{
   QString str;
   if      ( t == Attribute::ATTR_S  ) str = "Sedimentation Coefficient ( 20,W ) [ Sv ]";
   else if ( t == Attribute::ATTR_K  ) str = "Frictional Ratio";
   else if ( t == Attribute::ATTR_M  ) str = "Molecular Weight [ kDa ]";
   else if ( t == Attribute::ATTR_V  ) str = "Partial Specific Volume [ mL / g ]";
   else if ( t == Attribute::ATTR_D  ) str = "<p>Diffusion Coefficient ( 20,W ) [ cm<sup>2</sup> s<sup>-1</sup> ]</p>";
   else if ( t == Attribute::ATTR_F  ) str = "Frictional Coefficient [ g / s ]";
   else if ( t == Attribute::ATTR_F0 ) str = "Frictional Coefficient ( f0 ) [ g / s ]";
   else if ( t == Attribute::ATTR_SR ) str = "Sedimentation Coefficient ( T,Buffer ) [ Sv ]";
   else if ( t == Attribute::ATTR_DR ) str = "<p>Diffusion Coefficient ( T,Buffer ) [ cm<sup>2</sup> s<sup>-1</sup> ]</p>";
   return str;
}

QString Attribute::symbol( Type t )
{
   QString str;
   if      ( t == Attribute::ATTR_S  ) str = "s";
   else if ( t == Attribute::ATTR_K  ) str = "f/f0";
   else if ( t == Attribute::ATTR_M  ) str = "MW";
   else if ( t == Attribute::ATTR_V  ) str = "vbar";
   else if ( t == Attribute::ATTR_D  ) str = "D";
   else if ( t == Attribute::ATTR_F  ) str = "f";
   else if ( t == Attribute::ATTR_F0 ) str = "f0";
   else if ( t == Attribute::ATTR_SR ) str = "s*";
   else if ( t == Attribute::ATTR_DR ) str = "D*";
   return str;
}

Attribute::Type Attribute::from_symbol( const QString & symbol )
{
   Attribute::Type t = ATTR_NONE;
   if      ( symbol == "s"    ) t = Attribute::ATTR_S;
   else if ( symbol == "f/f0" ) t = Attribute::ATTR_K;
   else if ( symbol == "MW"   ) t = Attribute::ATTR_M;
   else if ( symbol == "vbar" ) t = Attribute::ATTR_V;
   else if ( symbol == "D"    ) t = Attribute::ATTR_D;
   else if ( symbol == "f"    ) t = Attribute::ATTR_F;
   else if ( symbol == "f0"   ) t = Attribute::ATTR_F0;
   else if ( symbol == "s*"   ) t = Attribute::ATTR_SR;
   else if ( symbol == "D*"   ) t = Attribute::ATTR_DR;
   return t;
}

Attribute::Type Attribute::from_int( int id )
{
   Attribute::Type t = ATTR_NONE;
   if      ( id == Attribute::ATTR_S  ) t = Attribute::ATTR_S;
   else if ( id == Attribute::ATTR_K  ) t = Attribute::ATTR_K;
   else if ( id == Attribute::ATTR_M  ) t = Attribute::ATTR_M;
   else if ( id == Attribute::ATTR_V  ) t = Attribute::ATTR_V;
   else if ( id == Attribute::ATTR_D  ) t = Attribute::ATTR_D;
   else if ( id == Attribute::ATTR_F  ) t = Attribute::ATTR_F;
   else if ( id == Attribute::ATTR_F0 ) t = Attribute::ATTR_F0;
   else if ( id == Attribute::ATTR_SR ) t = Attribute::ATTR_SR;
   else if ( id == Attribute::ATTR_DR ) t = Attribute::ATTR_DR;
   return t;
}

US_Grid_ZFunction::US_Grid_ZFunction( QWidget *parent, const QMap< QString, QString>& settings )
                  : US_WidgetsDialog( parent )
{
   bool has_data = settings.contains( "x_short_title" ) &&
                   settings.contains( "y_short_title" ) &&
                   settings.contains( "z_short_title" ) &&
                   settings.contains( "x_long_title"  ) &&
                   settings.contains( "y_long_title"  ) &&
                   settings.contains( "z_long_title"  ) &&
                   settings.contains( "parameters"    ) &&
                   settings.contains( "x_min"         ) &&
                   settings.contains( "x_max"         ) &&
                   settings.contains( "y_min"         ) &&
                   settings.contains( "y_max"         );

   if ( ! has_data ) {
      QMessageBox::warning( this, "Warning!", "Grid parameters are not set yet!");
      QTimer::singleShot( 10, this, [ this ]() { this->close(); } );
   }
   else {
      set_gui( settings );
   }
}

void US_Grid_ZFunction::set_gui( const QMap< QString, QString>& settings )
{
   setWindowTitle( tr( "Set Z-Value Function" ) );
   setPalette( US_GuiSettings::frameColor() );
   setWindowFlags( Qt::Window | Qt::WindowSystemMenuHint );

   lb_dependent = us_label( "Dependent" );
   lb_function  = us_label( "Function" );
   lb_min       = us_label( "Minimum" );
   lb_max       = us_label( "Maximum" );
   lb_order     = us_label( "Order" );
   lb_formula   = new QLabel();
   lb_p0        = us_label( "1"  );
   lb_p1        = us_label( "2"  );
   lb_p2        = us_label( "3"  );
   lb_p3        = us_label( "4"  );
   lb_p4        = us_label( "5"  );
   lb_p5        = us_label( "6"  );
   lb_c0        = us_label( "c0" );
   lb_c1        = us_label( "c1" );
   lb_c2        = us_label( "c2" );
   lb_c3        = us_label( "c3" );
   lb_c4        = us_label( "c4" );
   lb_c5        = us_label( "c5" );
   lb_p0_x      = us_label( "" );
   lb_p0_y      = us_label( "" );
   lb_p1_x      = us_label( "" );
   lb_p1_y      = us_label( "" );
   lb_p2_x      = us_label( "" );
   lb_p2_y      = us_label( "" );
   lb_p3_x      = us_label( "" );
   lb_p3_y      = us_label( "" );
   lb_p4_x      = us_label( "" );
   lb_p4_y      = us_label( "" );
   lb_p5_x      = us_label( "" );
   lb_p5_y      = us_label( "" );

   lb_formula->setFixedHeight( 30 );
   lb_formula->setStyleSheet( "QLabel { background-color: lightblue; color: black; }" );

   lb_formula  ->setAlignment( Qt::AlignCenter );
   lb_dependent->setAlignment( Qt::AlignCenter );
   lb_function ->setAlignment( Qt::AlignCenter );
   lb_order    ->setAlignment( Qt::AlignCenter );
   lb_min      ->setAlignment( Qt::AlignCenter );
   lb_max      ->setAlignment( Qt::AlignCenter );

   list_lb << lb_p0 << lb_p0_x << lb_p0_y << lb_c0
           << lb_p1 << lb_p1_x << lb_p1_y << lb_c1
           << lb_p2 << lb_p2_x << lb_p2_y << lb_c2
           << lb_p3 << lb_p3_x << lb_p3_y << lb_c3
           << lb_p4 << lb_p4_x << lb_p4_y << lb_c4
           << lb_p5 << lb_p5_x << lb_p5_y << lb_c5;

   for ( int ii = 0; ii < list_lb.size(); ii++ ) {
      list_lb[ ii ]->setAlignment( Qt::AlignCenter );
      list_lb[ ii ]->setFixedWidth( 80 );
      int mod = ii % 4;
      if ( mod == 0 ) {
         list_lb[ ii ]->setFixedWidth( 25 );
      }
   }

   le_min  = us_lineedit( "", 0, true );
   le_max  = us_lineedit( "", 0, true );
   le_c0   = us_lineedit( "" );
   le_c1   = us_lineedit( "" );
   le_c2   = us_lineedit( "" );
   le_c3   = us_lineedit( "" );
   le_c4   = us_lineedit( "" );
   le_c5   = us_lineedit( "" );
   le_p0_x = us_lineedit( "" );
   le_p1_x = us_lineedit( "" );
   le_p2_x = us_lineedit( "" );
   le_p3_x = us_lineedit( "" );
   le_p4_x = us_lineedit( "" );
   le_p5_x = us_lineedit( "" );
   le_p0_y = us_lineedit( "" );
   le_p1_y = us_lineedit( "" );
   le_p2_y = us_lineedit( "" );
   le_p3_y = us_lineedit( "" );
   le_p4_y = us_lineedit( "" );
   le_p5_y = us_lineedit( "" );

   list_le << le_p0_x << le_p0_y << le_c0
           << le_p1_x << le_p1_y << le_c1
           << le_p2_x << le_p2_y << le_c2
           << le_p3_x << le_p3_y << le_c3
           << le_p4_x << le_p4_y << le_c4
           << le_p5_x << le_p5_y << le_c5;

   for ( int ii = 0; ii < list_le.size(); ii++ ) {
      QDoubleValidator *validator = new QDoubleValidator( this );
      list_le[ ii ]->setValidator( validator );
      list_le[ ii ]->setFixedWidth( 80 );
   }

   cb_dependent = us_comboBox();
   cb_dependent->addItem( settings.value( "x_long_title" ) );
   cb_dependent->addItem( settings.value( "y_long_title" ) );
   cb_dependent->setCurrentIndex( 0 );

   short_title << settings.value( "x_short_title" )
               << settings.value( "y_short_title" )
               << settings.value( "z_short_title" );
   long_title  << settings.value( "x_long_title" )
               << settings.value( "y_long_title" )
               << settings.value( "z_long_title" );

   min_xy << settings.value( "x_min" ).toDouble()
                 << settings.value( "y_min" ).toDouble();
   max_xy << settings.value( "x_max" ).toDouble()
                 << settings.value( "y_max" ).toDouble();

   cb_function = us_comboBox();
   cb_function->addItem( "Polynomial" );
   cb_function->addItem( "Exponential" );
   cb_function->setCurrentIndex( 0 );
   if ( settings.value( "function" ) == "y_param" ) {
      cb_function->setCurrentIndex( 1 );
   }

   cb_order = us_comboBox();
   for ( int ii = 1; ii < 6; ii++ ) {
      cb_order->addItem( QString::number( ii ) );
   }
   cb_order->setCurrentText( QString::number( 3 ) );

   QPushButton *pb_fit    = us_pushbutton( "Fit" );
   QPushButton *pb_cancel = us_pushbutton( "Cancel" );
   QPushButton *pb_apply  = us_pushbutton( "Apply" );

   plot = new QwtPlot();
   QBoxLayout* usplot = new US_Plot( plot, "Grid Layout", "", "" );
   plot->setCanvasBackground( QBrush( QColor( 32, 32, 32 ) ) );

   // layouts
   int row = 0;
   QLabel *lb_points = us_banner( "Points" );
   QGridLayout *lyt_points = new QGridLayout();
   lyt_points->setContentsMargins( 0, 0, 0, 0 );
   lyt_points->setSpacing( 2 );
   lyt_points->addWidget( lb_points, row++, 0, 1, 5 );
   lyt_points->addWidget( lb_p0,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p0_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p0_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p0_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p0_y,   row++, 4, 1, 1 );
   lyt_points->addWidget( lb_p1,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p1_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p1_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p1_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p1_y,   row++, 4, 1, 1 );
   lyt_points->addWidget( lb_p2,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p2_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p2_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p2_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p2_y,   row++, 4, 1, 1 );
   lyt_points->addWidget( lb_p3,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p3_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p3_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p3_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p3_y,   row++, 4, 1, 1 );
   lyt_points->addWidget( lb_p4,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p4_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p4_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p4_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p4_y,   row++, 4, 1, 1 );
   lyt_points->addWidget( lb_p5,     row,   0, 1, 1 );
   lyt_points->addWidget( lb_p5_x,   row,   1, 1, 1 );
   lyt_points->addWidget( le_p5_x,   row,   2, 1, 1 );
   lyt_points->addWidget( lb_p5_y,   row,   3, 1, 1 );
   lyt_points->addWidget( le_p5_y,   row++, 4, 1, 1 );

   QFrame *frm_points = new QFrame();
   frm_points->setFrameStyle( QFrame::Panel | QFrame::Plain );
   frm_points->setLineWidth( 1 );
   frm_points->setMidLineWidth( 1 );
   frm_points->setLayout( lyt_points );

   row = 0;
   QLabel *lb_params = us_banner( "Parameters" );
   QGridLayout *lyt_params = new QGridLayout();
   lyt_params->setContentsMargins( 0, 0, 0, 0 );
   lyt_params->setSpacing( 2 );
   lyt_params->addWidget( lb_params, row++ , 0, 1, 2 );
   lyt_params->addWidget( lb_c0,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c0,     row++ , 1, 1, 1 );
   lyt_params->addWidget( lb_c1,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c1,     row++ , 1, 1, 1 );
   lyt_params->addWidget( lb_c2,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c2,     row++ , 1, 1, 1 );
   lyt_params->addWidget( lb_c3,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c3,     row++ , 1, 1, 1 );
   lyt_params->addWidget( lb_c4,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c4,     row++ , 1, 1, 1 );
   lyt_params->addWidget( lb_c5,     row   , 0, 1, 1 );
   lyt_params->addWidget( le_c5,     row++ , 1, 1, 1 );

   QFrame *frm_params = new QFrame();
   frm_params->setFrameStyle( QFrame::Panel | QFrame::Plain );
   frm_params->setLineWidth( 1 );
   frm_params->setMidLineWidth( 1 );
   frm_params->setLayout( lyt_params );

   QHBoxLayout *lyt_pnt_prm = new QHBoxLayout();
   lyt_pnt_prm->setContentsMargins( 0, 0, 0, 0 );
   lyt_pnt_prm->setSpacing( 2 );
   lyt_pnt_prm->addStretch( 1 );
   lyt_pnt_prm->addWidget( frm_points );
   lyt_pnt_prm->addWidget( frm_params );
   lyt_pnt_prm->addStretch( 1 );

   row = 0;
   QGridLayout *lyt_top = new QGridLayout();
   lyt_top->setContentsMargins( 0, 0, 0, 0 );
   lyt_top->setSpacing( 2 );
   lyt_top->addWidget( lb_formula    , row++ , 0, 1, 4 );
   lyt_top->addWidget( lb_dependent  , row   , 0, 1, 1 );
   lyt_top->addWidget( cb_dependent  , row   , 1, 1, 1 );
   lyt_top->addWidget( lb_function   , row   , 2, 1, 1 );
   lyt_top->addWidget( cb_function   , row++ , 3, 1, 1 );

   lyt_top->addWidget( lb_min        , row   , 0, 1, 1 );
   lyt_top->addWidget( le_min        , row   , 1, 1, 1 );
   lyt_top->addWidget( lb_order      , row   , 2, 1, 1 );
   lyt_top->addWidget( cb_order      , row++ , 3, 1, 1 );

   lyt_top->addWidget( lb_max        , row   , 0, 1, 1 );
   lyt_top->addWidget( le_max        , row   , 1, 1, 1 );
   lyt_top->addWidget( pb_fit        , row++ , 2, 1, 2 );

   lyt_top->addWidget( pb_cancel     , row   , 2, 1, 1 );
   lyt_top->addWidget( pb_apply      , row++ , 3, 1, 1 );
   lyt_top->addLayout( lyt_pnt_prm   , row++ , 0, 1, 4 );

   int w = 125;
   lb_dependent->setFixedWidth( w );
   lb_function-> setFixedWidth( w );
   cb_function-> setFixedWidth( w );
   lb_order->    setFixedWidth( w );
   cb_order->    setFixedWidth( w );
   lb_min->      setFixedWidth( w );
   lb_max->      setFixedWidth( w );

   QGridLayout *layout = new QGridLayout();
   layout->setContentsMargins( 2, 2, 2, 2 );
   layout->setSpacing( 2 );
   layout->addLayout( lyt_top, 0, 1, 1, 1 );
   layout->addLayout( usplot,  1, 0, 1, 3 );
   layout->setColumnStretch( 0, 1 );
   layout->setColumnStretch( 1, 0 );
   layout->setColumnStretch( 2, 1 );
   layout->setRowStretch( 0, 0 );
   layout->setRowStretch( 1, 1 );

   setLayout( layout );

   // parse settings
   parse_params( settings.value( "parameters" ) );
   set_dependent( cb_dependent->currentIndex() );

   connect( cb_function,  QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &US_Grid_ZFunction::set_function  );
   connect( cb_dependent, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &US_Grid_ZFunction::set_dependent );
   connect( cb_order    , QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &US_Grid_ZFunction::set_order );
   connect( pb_fit      , &QPushButton::clicked, this, &US_Grid_ZFunction::fit );
   connect( pb_apply    , &QPushButton::clicked, this, &US_Grid_ZFunction::apply );
   connect( pb_cancel   , &QPushButton::clicked, this, &US_Grid_ZFunction::reject );
   connect( le_c0       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c0 );
   connect( le_c1       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c1 );
   connect( le_c2       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c2 );
   connect( le_c3       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c3 );
   connect( le_c4       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c4 );
   connect( le_c5       , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_c5 );
   connect( le_p0_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p0_x );
   connect( le_p1_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p1_x );
   connect( le_p2_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p2_x );
   connect( le_p3_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p3_x );
   connect( le_p4_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p4_x );
   connect( le_p5_x     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p5_x );
   connect( le_p0_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p0_y );
   connect( le_p1_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p1_y );
   connect( le_p2_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p2_y );
   connect( le_p3_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p3_y );
   connect( le_p4_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p4_y );
   connect( le_p5_y     , &QLineEdit::editingFinished, this, &US_Grid_ZFunction::set_p5_y );
}

void US_Grid_ZFunction::apply()
{
   if ( ! check_data() ) {
      QMessageBox::warning( this, "Warning!", "Paremeters are not set yet!" );
      return;
   }
   accept();
}

bool US_Grid_ZFunction::check_data()
{
   bool ready = false;
   for ( int ii = 0; ii < parameters.size(); ii++ ) {
      if ( parameters.at(ii) != 0 ) {
         ready = true;
         break;
      }
   }
   return ready;
}

void US_Grid_ZFunction::parse_params( const QString &pstr )
{
   parameters.clear();
   QVector< double > pvec;
   QStringList plist = pstr.trimmed().split( ";" );
   int d_id = 0;
   int f_id = 0;
   int o_id = 0;

   if ( plist.size() >= 4 ) {
      if ( plist.at( 0 ).trimmed() == "polynomial" ) {
         f_id = 0;
      }
      else if ( plist.at( 0 ).trimmed() == "exponential" ){
         f_id = 1;
      }

      if ( plist.at( 1 ).trimmed() == "x" ) {
         d_id = 0;
      }
      else if ( plist.at( 1 ).trimmed() == "y" ){
         d_id = 1;
      }

      for ( int ii = 2; ii < plist.size(); ii++ ) {
         if ( ii >= 8 ) {
            break;
         }
         bool ok;
         double val = plist.at( ii ).toDouble( &ok );
         if ( ok ) {
            pvec << val;
         }
         else {
            pvec.clear();
            break;
         }
      }
   }
   if ( pvec.isEmpty() ) {
      o_id = 0;
   }
   else {
      parameters << pvec;
      o_id = pvec.size() - 2;
   }
   cb_dependent->setCurrentIndex( d_id );
   cb_function->setCurrentIndex( f_id );
   cb_order->setCurrentIndex( o_id );
}

QString US_Grid_ZFunction::get_parameters()
{
   QString params;
   if ( ! check_data() ) {
      return params;
   }
   QString type = cb_function->currentText().toLower();
   params += type;
   if ( cb_dependent->currentIndex() == 0 ) {
      params += ";x";
   }
   else {
      params += ";y";
   }
   for ( int ii = 0; ii < parameters.size(); ii++ ) {
      double pp = parameters.at( ii );
      params += QString( ";%1" ).arg( pp );
   }
   return params;
}

void US_Grid_ZFunction::show_formula()
{
   lb_formula->clear();
   QString type = cb_function->currentText().toLower();
   QString formula = "<html><b><var>y</var> = ";
   bool flag = true;
   if ( type == "polynomial" ) {
      formula += "<var>c</var><sub>0</sub>";
      formula += " + <var>c</var><sub>1</sub><var>x</var>";
      QString fmt = " + <var>c</var><sub>%1</sub><var>x</var><sup>%1</sup>";
      int order = cb_order->currentText().toInt();
      for (int ii = 1; ii < order; ii++) {
         formula += fmt.arg( ii + 1 );
      }
   }
   else if ( type == "exponential" ) {
      formula += "<var>a</var><var>e</var><sup><var>b</var><var>x</var></sup>";
   }
   else {
      flag = false;
   }
   formula += "</b></html>";
   if ( flag ) {
      lb_formula->setText( formula );
   }
}

void US_Grid_ZFunction::set_dependent( int index )
{
   double min = min_xy.at( index );
   double max = max_xy.at( index );
   int num = 200;
   double dx = ( max - min ) / static_cast<double>( num );
   xvalues.clear();
   yvalues.clear();
   double val = min;
   while ( val <= max ) {
      xvalues << val;
      val += dx;
   }
   yvalues.resize( xvalues.size() );
   QString xtitle = short_title.at( index );
   QString ytitle = short_title.last();
   for ( int ii = 0; ii < 6; ii++ ) {
      list_lb[ ii * 4 + 1 ]->setText( xtitle );
      list_lb[ ii * 4 + 2 ]->setText( ytitle );
   }
   lb_min->setText( QString( "Min of %1" ).arg( ytitle ) );
   lb_max->setText( QString( "Max of %1" ).arg( ytitle ) );
   set_function( cb_function->currentIndex() );
}

void US_Grid_ZFunction::set_function( int index )
{
   QString type = cb_function->itemText( index ).toLower();
   lb_order->hide();
   cb_order->hide();
   int size = 0;
   if ( type == "polynomial" ) {
      size = cb_order->currentText().toInt() + 1;
      lb_c0->setText( QString( "c%1" ).arg( 0 ) );
      lb_c1->setText( QString( "c%1" ).arg( 1 ) );
      lb_order->show();
      cb_order->show();
   }
   else if ( type == "exponential" ) {
      size = 2;
      lb_c0->setText( "a" );
      lb_c1->setText( "b" );
   }
   set_points( size );
}

void US_Grid_ZFunction::set_points( int size )
{
   double x_min = min_xy.at( cb_dependent->currentIndex() );
   double x_max = max_xy.at( cb_dependent->currentIndex() );
   parameters.resize( size );
   x_points.resize( size );
   y_points.resize( size );

   x_points[ 0 ] = x_min;
   x_points[ size - 1 ] = x_max;
   double dx = ( x_max - x_min ) / static_cast<double>( size - 1 );
   double val = x_min;
   for( int ii = 0; ii < size - 2; ii++ ) {
      val += dx;
      x_points[ ii + 1 ] = val;
   }

   for ( int ii = 0; ii < list_lb.size(); ii++ ) {
      if ( ii < 4 * size ) {
         list_lb[ ii ]->show();
      }
      else {
         list_lb[ ii ]->hide();
      }
   }
   for ( int ii = 0; ii < list_le.size(); ii++ ) {
      if ( ii < 3 * size ) {
         list_le[ ii ]->show();
      }
      else {
         list_le[ ii ]->hide();
      }
   }

   compute( x_points, y_points );
   for ( int ii = 0; ii < size; ii++ ) {
      list_le[ ii * 3     ]->setText( QString::number(   x_points.at( ii ) ) );
      list_le[ ii * 3 + 1 ]->setText( QString::number(   y_points.at( ii ) ) );
      list_le[ ii * 3 + 2 ]->setText( QString::number( parameters.at( ii ) ) );
   }
   plot_data();
   show_formula();
}

void US_Grid_ZFunction::compute( QVector< double >& x_vec, QVector< double >& y_vec )
{
   QString type = cb_function->currentText().toLower();
   y_vec.clear();
   double xx;
   double yy;
   for ( int ii = 0; ii < x_vec.size(); ii++ ) {
      xx = x_vec.at( ii );
      yy = 0;
      if ( type == "polynomial" ) {
         for( int jj = 0; jj < parameters.size(); jj++ ) {
            yy += parameters.at( jj ) * qPow( xx, jj );
         }
      }
      else if ( type == "exponential" ) {
         yy = parameters.at( 0 ) * qExp( parameters.at( 1 ) * xx );
      }
      y_vec << yy;
   }
}

void US_Grid_ZFunction::plot_data()
{
   plot->detachItems(QwtPlotItem::Rtti_PlotItem, false);

   compute( xvalues, yvalues );

   QwtPlotCurve* curve = us_curve( plot, "Curve" );
   curve->setSamples( xvalues.data(), yvalues.data(), xvalues.size() );
   curve->setPen( QPen( Qt::yellow, 2 ) );

   QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
                                     QBrush( Qt::red ),
                                     QPen( Qt::red, 5 ), QSize( 5, 5 ) );
   QwtPlotCurve* points = us_curve( plot, "Points" );
   points->setSymbol( symbol );
   points->setStyle( QwtPlotCurve::NoCurve );
   points->setSamples( x_points.data(), y_points.data(), x_points.size() );

   std::pair<double*, double*> min_max;
   double x_min, x_max, y_min, y_max, dx, dy;
   min_max = std::minmax_element( xvalues.begin(), xvalues.end() );
   x_min   = *min_max.first;
   x_max   = *min_max.second;

   min_max = std::minmax_element( yvalues.begin(), yvalues.end() );
   y_min   = *min_max.first;
   y_max   = *min_max.second;

   le_min->setText( QString::number( y_min ) );
   le_max->setText( QString::number( y_max ) );

   min_max = std::minmax_element( x_points.begin(), x_points.end() );
   x_min   = qMin( *min_max.first, x_min );
   x_max   = qMax( *min_max.second, x_max );

   min_max = std::minmax_element( y_points.begin(), y_points.end() );
   y_min   = qMin( *min_max.first, y_min );
   y_max   = qMax( *min_max.second, y_max );

   dx = ( x_max - x_min ) * 0.1;
   dy = ( y_max - y_min ) * 0.1;
   dx = qMax( dx, 0.1 );
   dy = qMax( dy, 0.1 );
   x_min -= dx;
   x_max += dx;
   y_min -= dy;
   y_max += dy;
   plot->setAxisTitle( QwtPlot::xBottom, long_title.at( cb_dependent->currentIndex() ) );
   plot->setAxisTitle( QwtPlot::yLeft,   long_title.last() );
   plot->setAxisScale( QwtPlot::xBottom, x_min, x_max );
   plot->setAxisScale( QwtPlot::yLeft  , y_min, y_max );
   // plot->setAxisAutoScale(QwtPlot::xBottom, true);
   // plot->setAxisAutoScale(QwtPlot::yLeft, true);
   // us_grid( plot );
   plot->replot();
}

void US_Grid_ZFunction::set_order( int index )
{
   int size = cb_order->itemText( index ).toInt() + 1;
   set_points( size );
}

void US_Grid_ZFunction::fit()
{
   QString type = cb_function->currentText().toLower();
   QVector<double> xvec = x_points;
   QVector<double> yvec = y_points;
   QVector<double> pars = parameters;
   if ( type == "exponential" ) {
      for ( int ii = 0; ii < yvec.size(); ii++ ) {
         yvec[ ii ] = std::log( yvec.at( ii ) );
      }
   }
   double *c = pars.data();
   double *x = xvec.data();
   double *y = yvec.data();
   int np    = xvec.size();

   bool ok = US_Matrix::lsfit( c, x, y, np, np );
   if ( ok ) {
      if ( type == "exponential" ) {
         pars [ 0 ] = std::exp( pars.first() );
      }
      parameters.clear();
      parameters << pars;
      for ( int ii = 0; ii < parameters.size(); ii++ ) {
         QString str = QString::number( parameters.at( ii ) );
         switch ( ii ) {
         case 0:
            le_c0->setText( str );
            break;
         case 1:
            le_c1->setText( str );
            break;
         case 2:
            le_c2->setText( str );
            break;
         case 3:
            le_c3->setText( str );
            break;
         case 4:
            le_c4->setText( str );
            break;
         case 5:
            le_c5->setText( str );
            break;
         }
      }
      plot_data();
   }
}

void US_Grid_ZFunction::set_c0()
{
   parameters[ 0 ] = le_c0->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_c1()
{
   parameters[ 1 ] = le_c1->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_c2()
{
   parameters[ 2 ] = le_c2->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_c3()
{
   parameters[ 3 ] = le_c3->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_c4()
{
   parameters[ 4 ] = le_c4->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_c5()
{
   parameters[ 5 ] = le_c5->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p0_x()
{
   x_points[ 0 ] = le_p0_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p1_x()
{
   x_points[ 1 ] = le_p1_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p2_x()
{
   x_points[ 2 ] = le_p2_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p3_x()
{
   x_points[ 3 ] = le_p3_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p4_x()
{
   x_points[ 4 ] = le_p4_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p5_x()
{
   x_points[ 5 ] = le_p5_x->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p0_y()
{
   y_points[ 0 ] = le_p0_y->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p1_y()
{
   y_points[ 1 ] = le_p1_y->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p2_y()
{
   y_points[ 2 ] = le_p2_y->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p3_y()
{
   y_points[ 3 ] = le_p3_y->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p4_y()
{
   y_points[ 4 ] = le_p4_y->text().toDouble();
   plot_data();
}

void US_Grid_ZFunction::set_p5_y()
{
   y_points[ 5 ] = le_p5_y->text().toDouble();
   plot_data();
}

