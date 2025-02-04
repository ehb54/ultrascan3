//! \file us_ga_init.cpp

#include <QApplication>
#include "us_grid_editor.h"
#include "us_gui_util.h"
#include <qwt_plot_shapeitem.h>

const double VISC_20WP = VISC_20W * 0.01;
const double RGK20   = R_GC * K20;
const double MPISQ   = M_PI * M_PI;

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
   dValid = new QDoubleValidator(this);
   iValid = new QIntValidator(this);
   iValid->setBottom(1);

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
   QLabel *lb_dens = us_label( tr( "ρ [g/mL]" ) );
   lb_dens->setAlignment( Qt::AlignCenter );
   le_dens = us_lineedit( QString::number( DENS_20W ) );
   le_dens->setValidator(dValid);

   QLabel *lb_visc = us_label( tr( "η [cP]" ) );
   lb_visc->setAlignment( Qt::AlignCenter );
   le_visc = us_lineedit( QString::number( VISC_20W ) );
   le_visc->setValidator(dValid);

   QLabel *lb_temp = us_label( tr( "T [°C]" ) );
   lb_temp->setAlignment( Qt::AlignCenter );
   le_temp = us_lineedit( QString::number( 20 ) );
   le_temp->setValidator(dValid);

   QPushButton* pb_set_exp_data = us_pushbutton("Update");

   // 20,w grid control
   QLabel *lb_20w_ctrl = us_banner( tr( "20,W Grid Control" ) );

   QLabel *lb_grid_list = us_label(" Grid List ");
   lb_grid_list->setAlignment( Qt::AlignCenter );

   lw_grids = us_listwidget();

   QPushButton* pb_new = us_pushbutton( "New Grid" );
   connect(pb_new, &QPushButton::clicked, this, &US_Grid_Editor::new_grid_clicked);

   QPushButton* pb_delete = us_pushbutton( "Delete Grid" );
   connect(pb_delete, &QPushButton::clicked, this, &US_Grid_Editor::delete_grid_clicked);

   QPushButton* pb_update = us_pushbutton( "Update Grid" );
   connect(pb_update, &QPushButton::clicked, this, &US_Grid_Editor::update_grid_clicked);

   QHBoxLayout* lyt_1 = new QHBoxLayout();
   lyt_1->addWidget(pb_new);
   lyt_1->addWidget(pb_update);
   lyt_1->addWidget(pb_delete);

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
   le_x_min->setValidator(dValid);
   le_x_max = us_lineedit();
   le_x_max->setValidator(dValid);
   le_x_res = us_lineedit();
   le_x_res->setValidator(iValid);

   le_y_min = us_lineedit();
   le_y_min->setValidator(dValid);
   le_y_max = us_lineedit();
   le_y_max->setValidator(dValid);
   le_y_res = us_lineedit();
   le_y_res->setValidator(iValid);

   QFrame *hline2 = new QFrame();
   hline2->setFrameShape(QFrame::HLine);
   hline2->setFrameShadow(QFrame::Sunken);

   lb_z_ax = us_label( Attr_to_short( z_param ));
   lb_z_ax->setAlignment( Qt::AlignCenter );
   le_z_val = us_lineedit();
   le_z_val->setValidator(dValid);

   connect(le_x_min, &QLineEdit::editingFinished, this, &US_Grid_Editor::update_xMin);
   connect(le_x_max, &QLineEdit::editingFinished, this, &US_Grid_Editor::update_xMax);
   connect(le_y_min, &QLineEdit::editingFinished, this, &US_Grid_Editor::update_yMin);
   connect(le_y_max, &QLineEdit::editingFinished, this, &US_Grid_Editor::update_yMax);

   QPushButton* pb_validate = us_pushbutton( "Add " );
   connect( pb_validate, &QPushButton::clicked, this, &US_Grid_Editor::add_update );

   wg_add_update = new QWidget();
   QGridLayout *lyt_2 = new QGridLayout();
   int row = 0;
   lyt_2->setSpacing        ( 2 );
   lyt_2->setContentsMargins( 0, 0, 0, 0 );
   lyt_2->addWidget( hline1,               row++, 0, 1, 4 );

   lyt_2->addWidget( lb_min,               row,   1, 1, 1 );
   lyt_2->addWidget( lb_max,               row,   2, 1, 1 );
   lyt_2->addWidget( lb_res,               row++, 3, 1, 1 );

   lyt_2->addWidget( lb_x_ax,              row,   0, 1, 1 );
   lyt_2->addWidget( le_x_min,             row,   1, 1, 1 );
   lyt_2->addWidget( le_x_max,             row,   2, 1, 1 );
   lyt_2->addWidget( le_x_res,             row++, 3, 1, 1 );

   lyt_2->addWidget( lb_y_ax,              row,   0, 1, 1 );
   lyt_2->addWidget( le_y_min,             row,   1, 1, 1 );
   lyt_2->addWidget( le_y_max,             row,   2, 1, 1 );
   lyt_2->addWidget( le_y_res,             row++, 3, 1, 1 );

   lyt_2->addWidget( hline2,               row++, 0, 1, 4 );

   lyt_2->addWidget( lb_z_ax,              row,   0, 1, 1 );
   lyt_2->addWidget( le_z_val,             row,   1, 1, 1 );
   lyt_2->addWidget( pb_validate,          row++, 2, 1, 2 );

   wg_add_update->setLayout(lyt_2);


   QLabel *lb_subgrids = us_label( "# Subgrids" );
   lb_subgrids->setAlignment( Qt::AlignCenter );
   le_subgrids = us_lineedit();
   le_subgrids->setValidator(iValid);

   QLabel *lb_allgrids = us_label( "# Grid Points" );
   lb_allgrids->setAlignment( Qt::AlignCenter );
   le_allgrids = us_lineedit("", -1, true);

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset, &QPushButton::clicked, this, &US_Grid_Editor::reset );

   QPushButton* pb_save = us_pushbutton( tr( "Save" ) );
   pb_save->setEnabled( false );
   // connect( pb_save, &QPushButton::clicked, this, &US_Grid_Editor::save );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   // connect( pb_help, &QPushButton::clicked, this, &US_Grid_Editor::help );

   QPushButton* pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close, &QPushButton::clicked, this, &US_Grid_Editor::close );

   row = 0;
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

   left->addWidget( lb_grid_list,         row++, 0, 1, 4 );

   left->addWidget( lw_grids,             row,   0, 5, 4 );
   row += 5;

   left->addLayout(lyt_1,                 row++, 0, 1, 4 );

   left->addWidget(wg_add_update,         row++, 0, 1, 4 );

   left->addWidget( lb_subgrids,          row,   0, 1, 1 );
   left->addWidget( le_subgrids,          row,   1, 1, 1 );
   left->addWidget( lb_allgrids,          row,   2, 1, 1 );
   left->addWidget( le_allgrids,          row++, 3, 1, 1 );

   QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
   left->addItem(spacer, row++, 0, 1, 4);

   left->addWidget( pb_reset,             row,   0, 1, 1 );
   left->addWidget( pb_save,              row,   1, 1, 1 );
   left->addWidget( pb_help,              row,   2, 1, 1 );
   left->addWidget( pb_close,             row++, 3, 1, 1 );

   for (int ii = 0; ii < 4; ii++) {
      left->setColumnStretch(ii, 1);
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
   data_plot->setCanvasBackground(QBrush(Qt::black));

   QRadioButton *rb_x_s;
   QRadioButton *rb_x_mw;
   QRadioButton *rb_x_ff0;
   QRadioButton *rb_x_D;
   QRadioButton *rb_x_f;
   QRadioButton *rb_x_vbar;
   QRadioButton *rb_x_sr;
   QRadioButton *rb_x_Dr;

   QRadioButton *rb_y_s;
   QRadioButton *rb_y_mw;
   QRadioButton *rb_y_ff0;
   QRadioButton *rb_y_D;
   QRadioButton *rb_y_f;
   QRadioButton *rb_y_vbar;
   QRadioButton *rb_y_sr;
   QRadioButton *rb_y_Dr;

   QGridLayout* lyt_x_s     = us_radiobutton( Attr_to_char( ATTR_S ),  rb_x_s,    false );
   QGridLayout* lyt_x_ff0   = us_radiobutton( Attr_to_char( ATTR_K ),  rb_x_ff0,  false );
   QGridLayout* lyt_x_mw    = us_radiobutton( Attr_to_char( ATTR_M ),  rb_x_mw,   false );
   QGridLayout* lyt_x_vbar  = us_radiobutton( Attr_to_char( ATTR_V ),  rb_x_vbar, false );
   QGridLayout* lyt_x_D     = us_radiobutton( Attr_to_char( ATTR_D ),  rb_x_D,    false );
   QGridLayout* lyt_x_f     = us_radiobutton( Attr_to_char( ATTR_F ),  rb_x_f,    false );
   QGridLayout* lyt_x_sr    = us_radiobutton( Attr_to_char( ATTR_SR ), rb_x_sr,   false );
   QGridLayout* lyt_x_Dr    = us_radiobutton( Attr_to_char( ATTR_DR ), rb_x_Dr,   false );

   QGridLayout* lyt_y_s     = us_radiobutton( Attr_to_char( ATTR_S ),  rb_y_s,    false );
   QGridLayout* lyt_y_ff0   = us_radiobutton( Attr_to_char( ATTR_K ),  rb_y_ff0,  false );
   QGridLayout* lyt_y_mw    = us_radiobutton( Attr_to_char( ATTR_M ),  rb_y_mw,   false );
   QGridLayout* lyt_y_vbar  = us_radiobutton( Attr_to_char( ATTR_V ),  rb_y_vbar, false );
   QGridLayout* lyt_y_D     = us_radiobutton( Attr_to_char( ATTR_D ),  rb_y_D,    false );
   QGridLayout* lyt_y_f     = us_radiobutton( Attr_to_char( ATTR_F ),  rb_y_f,    false );
   QGridLayout* lyt_y_sr    = us_radiobutton( Attr_to_char( ATTR_SR ), rb_y_sr,   false );
   QGridLayout* lyt_y_Dr    = us_radiobutton( Attr_to_char( ATTR_DR ), rb_y_Dr,   false );

   x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    ATTR_S );
   x_axis->addButton( rb_x_ff0,  ATTR_K );
   x_axis->addButton( rb_x_mw,   ATTR_M );
   x_axis->addButton( rb_x_vbar, ATTR_V );
   x_axis->addButton( rb_x_D,    ATTR_D );
   x_axis->addButton( rb_x_f,    ATTR_F );
   x_axis->addButton( rb_x_sr,   ATTR_SR );
   x_axis->addButton( rb_x_Dr,   ATTR_DR );

   y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    ATTR_S );
   y_axis->addButton( rb_y_ff0,  ATTR_K );
   y_axis->addButton( rb_y_mw,   ATTR_M );
   y_axis->addButton( rb_y_vbar, ATTR_V );
   y_axis->addButton( rb_y_D,    ATTR_D );
   y_axis->addButton( rb_y_f,    ATTR_F );
   y_axis->addButton( rb_y_sr,   ATTR_SR );
   y_axis->addButton( rb_y_Dr,   ATTR_DR );

   connect( x_axis, &QButtonGroup::idReleased, this, &US_Grid_Editor::select_x_axis );
   connect( y_axis, &QButtonGroup::idReleased, this, &US_Grid_Editor::select_y_axis );

   QLabel* lb_plt_cntrl = us_banner("Plot Control");
   QLabel* lb_x_plot = us_label("X-Axis");
   lb_x_plot->setAlignment(Qt::AlignCenter);
   QLabel* lb_y_plot = us_label("Y-Axis");
   lb_y_plot->setAlignment(Qt::AlignCenter);

   QLabel* lb_p_size = us_label("Point Size");
   lb_p_size->setAlignment(Qt::AlignCenter);
   ct_size = us_counter(2, 1, 50, 2);
   ct_size->setSingleStep(1);
   QLineEdit* le_sz = ct_size->findChild<QLineEdit*>();
   if (le_sz) {
      le_sz->setAlignment(Qt::AlignCenter);
   }

   connect(ct_size, &QwtCounter::valueChanged, this, &US_Grid_Editor::update_symsize);

   QGridLayout* lyt_r = new QGridLayout();
   lyt_r->setMargin(0);
   lyt_r->addWidget(lb_x_plot,  0, 0, 1, 1);
   lyt_r->addLayout(lyt_x_s,    0, 1, 1, 1);
   lyt_r->addLayout(lyt_x_ff0,  0, 2, 1, 1);
   lyt_r->addLayout(lyt_x_mw,   0, 3, 1, 1);
   lyt_r->addLayout(lyt_x_vbar, 0, 4, 1, 1);
   lyt_r->addLayout(lyt_x_D,    0, 5, 1, 1);
   lyt_r->addLayout(lyt_x_f,    0, 6, 1, 1);
   lyt_r->addLayout(lyt_x_sr,   0, 7, 1, 1);
   lyt_r->addLayout(lyt_x_Dr,   0, 8, 1, 1);

   lyt_r->addWidget(lb_y_plot,  1, 0, 1, 1);
   lyt_r->addLayout(lyt_y_s,    1, 1, 1, 1);
   lyt_r->addLayout(lyt_y_ff0,  1, 2, 1, 1);
   lyt_r->addLayout(lyt_y_mw,   1, 3, 1, 1);
   lyt_r->addLayout(lyt_y_vbar, 1, 4, 1, 1);
   lyt_r->addLayout(lyt_y_D,    1, 5, 1, 1);
   lyt_r->addLayout(lyt_y_f,    1, 6, 1, 1);
   lyt_r->addLayout(lyt_y_sr,   1, 7, 1, 1);
   lyt_r->addLayout(lyt_y_Dr,   1, 8, 1, 1);

   lyt_r->addWidget(lb_p_size,  2, 0, 1, 1);
   lyt_r->addWidget(ct_size,    2, 3, 1, 3);

   for ( int ii = 0; ii < lyt_r->columnCount(); ii++ ) {
      lyt_r->setColumnStretch(ii, 1);
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
   select_x_axis(x_param);
   select_y_axis(y_param);
}

void US_Grid_Editor::rm_tmp_items()
{
   const auto items = data_plot->itemList();
   for (QwtPlotItem *item : items) {
      if (item ) {
         QString txt = item->title().text();
         if ( txt.startsWith("TMP_H") || txt.startsWith("TMP_V") || txt.startsWith("TMP_R") ) {
            item->detach();
            delete item;
         }
      }
   }
}

void US_Grid_Editor::rm_all_items()
{
   const auto items = data_plot->itemList();
   for (QwtPlotItem *item : items) {
      if (item ) {
         item->detach();
         delete item;
      }
   }
}

void US_Grid_Editor::plot_tmp()
{
   rm_tmp_items();
   bool is_xMin = validate_num("x_min");
   bool is_xMax = validate_num("x_max");
   bool is_yMin = validate_num("y_min");
   bool is_yMax = validate_num("y_max");

   double x1 = le_x_min->text().toDouble();
   double x2 = le_x_max->text().toDouble();
   double y1 = le_y_min->text().toDouble();
   double y2 = le_y_max->text().toDouble();

   if (! is_xMin ) x1 = -1e6;
   if (! is_xMax ) x2 = +1e6;
   if (! is_yMin ) y1 = -1e6;
   if (! is_yMax ) y2 = +1e6;

   double px1 = data_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
   double px2 = data_plot->axisScaleDiv(QwtPlot::xBottom).upperBound();
   double py1 = data_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
   double py2 = data_plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
   if ( x1 != -1e6 && x2 != 1e6 ) {
      double dx = qAbs(x2 - x1) * 0.1;
      px1 = x1 - dx;
      px2 = x2 + dx;
   } else if ( x1 != -1e6 && x2 == 1e6 ) {
      double dx = qAbs(x1) * 0.1;
      px1 = x1 - dx;
      px2 = x1 + dx;
   } else if ( x1 == -1e6 && x2 != 1e6 ) {
      double dx = qAbs(x2) * 0.1;
      px1 = x2 - dx;
      px2 = x2 + dx;
   }

   if ( y1 != -1e6 && y2 != 1e6 ) {
      double dy = qAbs(y2 - y1) * 0.1;
      py1 = y1 - dy;
      py2 = y2 + dy;
   } else if ( y1 != -1e6 && y2 == 1e6 ) {
      double dy = qAbs(y1) * 0.1;
      py1 = y1 - dy;
      py2 = y1 + dy;
   } else if ( y1 == -1e6 && y2 != 1e6 ) {
      double dy = qAbs(y2) * 0.1;
      py1 = y2 - dy;
      py2 = y2 + dy;
   }

   QPainterPath path;
   QString title;
   if (is_xMin && !is_xMax && !is_yMin && !is_yMax) {
      path.moveTo(x1, y1);
      path.lineTo(x1, y2);
      title = "TMP_V1";
   } else if (!is_xMin && is_xMax && !is_yMin && !is_yMax) {
      path.moveTo(x2, y1);
      path.lineTo(x2, y2);
      title = "TMP_V2";
   } else if (!is_xMin && !is_xMax && is_yMin && !is_yMax) {
      path.moveTo(x1, y1);
      path.lineTo(x2, y1);
      title = "TMP_H1";
   } else if (!is_xMin && !is_xMax && !is_yMin && is_yMax) {
      path.moveTo(x1, y2);
      path.lineTo(x2, y2);
      title = "TMP_H2";
   } else {
      QRectF rect(x1, y1, x2 - x1, y2 - y1);
      path.addRect(rect);
      title = "TMP_R";
   }

   QwtPlotShapeItem *shapeItem = new QwtPlotShapeItem();
   shapeItem->setTitle(title);
   shapeItem->setShape(path);
   // shapeItem->setBrush(QBrush(QColor(255,255,51, 128)));
   // shapeItem->setPen(QPen(QColor(255,255,51), 2));
   shapeItem->setBrush(QBrush(QColor(255,255,255, 128)));
   shapeItem->setPen(QPen(QColor(255,255,255), 2));
   shapeItem->attach(data_plot);

   if ( ! grid_points.isEmpty() ) {
      px1 = qMin(px1, data_plot->axisScaleDiv(QwtPlot::xBottom).lowerBound());
      px2 = qMax(px2, data_plot->axisScaleDiv(QwtPlot::xBottom).upperBound());
      py1 = qMin(py1, data_plot->axisScaleDiv(QwtPlot::yLeft).lowerBound());
      py2 = qMax(py2, data_plot->axisScaleDiv(QwtPlot::yLeft).upperBound());
   }

   data_plot->setAxisScale( QwtPlot::xBottom, px1, px2);
   data_plot->setAxisScale( QwtPlot::yLeft  , py1, py2);
   data_plot->replot();
}

bool US_Grid_Editor::validate_num(const QString label)
{
   QString value;
   int pos = 0;
   if ( label == "x_min" ) {
      value = le_x_min->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "x_max" ) {
      value = le_x_max->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "x_res" ) {
      value = le_x_res->text();
      return iValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "y_min" ) {
      value = le_y_min->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "y_max" ) {
      value = le_y_max->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "y_res" ) {
      value = le_y_res->text();
      return iValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "z_val" ) {
      value = le_z_val->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "dens" ) {
      value = le_dens->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "visc" ) {
      value = le_visc->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else if ( label == "temp" ) {
      value = le_temp->text();
      return dValid->validate(value, pos) == QValidator::Acceptable;
   } else {
      return false;
   }
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
   }
}

void US_Grid_Editor::add_update()
{
   if ( ! validate() ) return;

   double dens = le_dens->text().toDouble();
   double visc = le_visc->text().toDouble();
   double temp = le_temp->text().toDouble();
   bool dvt_set = validate_num("dens") && validate_num("visc") && validate_num("temp");

   QHash<QString, double> xyz;
   get_xyz(xyz);

   int x_res    = xyz.value("xRes");
   int y_res    = xyz.value("yRes");
   double z_val = xyz.value("zVal");
   QVector<double> xpoints;
   linspace(xyz.value("xMin"), xyz.value("xMax"), x_res, xpoints);

   QVector<double> ypoints;
   linspace(xyz.value("yMin"), xyz.value("yMax"), y_res, ypoints);

   if ( overlap(xpoints.first(), xpoints.last(),
               ypoints.first(), ypoints.last()) ) {
      QMessageBox::critical(this, "Error!", "Grid points overlap!");
      return;
   }

   QVector<int> types;
   types << x_param << y_param << z_param;
   QVector<double> vals;
   QVector<GridPoint> gps;
   for ( int jj = 0; jj < y_res; jj++ ) {
      for ( int ii = 0; ii < x_res; ii++ ) {
         vals.clear();
         vals << xpoints.at(ii) << ypoints.at(jj) << z_val;
         GridPoint gp;
         if ( dvt_set ) {
            gp.set_dens_visc_t(dens, visc, temp);
         }
         if ( gp.set_param(vals, types) ) {
            gps << gp;
         }
      }
   }

   grid_points << gps;
   QVector<double> ginfo;
   ginfo << xyz.value("xMinU") << xyz.value("xMaxU") << xyz.value("xRes")
         << xyz.value("yMinU") << xyz.value("yMaxU") << xyz.value("yRes")
         << xyz.value("zValU");
   grid_info << ginfo;
   clear_xyz();
   plot_all();
   fill_list();
}

bool US_Grid_Editor::validate( )
{
   QString error_msg;
   if ( ! validate_num("x_min") )
   {
      error_msg = tr("%1 \n\nMinimum value is not set!").arg(lb_x_ax->text());

   } else if ( ! validate_num("x_max") )
   {
      error_msg = tr("%1 \n\nMaximum value is not set!").arg(lb_x_ax->text());

   } else if ( ! validate_num("x_res") )
   {
      error_msg = tr("%1 \n\nResolusion value is not set!").arg(lb_x_ax->text());

   } else if ( ! validate_num("y_min") )
   {
      error_msg = tr("%1 \n\nMinimum value is not set!").arg(lb_y_ax->text());

   } else if ( ! validate_num("y_max") )
   {
      error_msg = tr("%1 \n\nMaximum value is not set!").arg(lb_y_ax->text());

   } else if ( ! validate_num("y_res") )
   {
      error_msg = tr("%1 \n\nResolusion value is not set!").arg(lb_y_ax->text());

   } else if ( ! validate_num("z_val") )
   {
      error_msg = tr("%1 \n\nValue is not set!").arg(lb_z_ax->text());

   } else if ( le_x_max->text().toDouble() <= le_x_min->text().toDouble() )
   {
      error_msg = tr("%1 \n\nMaximum value is less than or equal to the Minimum value!").
                  arg(lb_x_ax->text());

   } else if ( le_y_max->text().toDouble() <= le_y_min->text().toDouble() )
   {
      error_msg = tr("%1 \n\nMaximum value is less than or equal to the Minimum value!").
                  arg(lb_y_ax->text());
   } else if ( le_x_res->text().toInt() == 0 )
   {
      error_msg = tr("%1 \n\nResolution value is zero!").arg(lb_x_ax->text());

   } else if ( le_y_res->text().toInt() == 0 )
   {
      error_msg = tr("%1 \n\nResolution value is zero!").arg(lb_y_ax->text());
   }

   QHash<QString, double> xyz;
   get_xyz(xyz);

   GridPoint gp;
   QVector<int> types;
   types << x_param << y_param << z_param;
   QVector<double> vals;
   vals << xyz.value("xMin") << xyz.value("yMin") << xyz.value("zVal");
   if ( ! gp.set_param(vals, types) ) {
      error_msg = gp.error_string();
   } else {
      vals.clear();
      vals << xyz.value("xMax") << xyz.value("yMax") << xyz.value("zVal");
      if ( ! gp.set_param(vals, types) ) {
         error_msg = gp.error_string();
      }
   }

   if ( ! error_msg.isEmpty() ) {
      QMessageBox::warning(this, "Error!", error_msg);
      return false;
   }

   return true;

}

bool US_Grid_Editor::overlap(double xMin, double xMax, double yMin, double yMax)
{
   for (int ii= 0; ii < grid_points.size(); ii++) {
      QVector<GridPoint> item = grid_points.at(ii);
      double x1, x2, y1, y2;
      int sz = item.size();
      item[0].value_by_name(Attr_to_char( x_param ), x1 );
      item[0].value_by_name(Attr_to_char( y_param), y1 );
      item[sz - 1].value_by_name(Attr_to_char( x_param ), x2 );
      item[sz - 1].value_by_name(Attr_to_char( y_param ), y2 );
      bool x_ovrlp = ! ( xMin > x2 || xMax < x1 );
      bool y_ovrlp = ! ( yMin > y2 || yMax < y1 );
      if ( x_ovrlp && y_ovrlp ) {
         return true;
      }
   }
   return false;
}

void US_Grid_Editor::linspace(double x1, double x2, int np, QVector<double> &vec)
{
   double dx = (x2 - x1) / static_cast<double>(np);
   vec.clear();
   double x0 = x1 + 0.5 * dx;
   for (int ii = 0; ii < np; ii++) {
      if (ii != 0) {
         x0 += dx;
      }
      vec << x0;
   }
}

void US_Grid_Editor::unit_corr(double& val, int type)
{
   if ( type == ATTR_S ) {
      val = val * 1e-13;
   }
   if ( type == ATTR_M ) {
      val = val * 1000;
   }
}

void US_Grid_Editor::fill_list()
{
   lw_grids->disconnect();

   int row = lw_grids->currentRow();
   lw_grids->clear();
   int n_grids = grid_points.size();
   QString title = "%1) %2 = %3 - %4 ; %5 = %6 - %7 ; %8 = %9";

   for (int ii = 0; ii < n_grids; ii++) {
      QVector<double> ginfo = grid_info.at(ii);
      double x1 = ginfo.at(0);
      double x2 = ginfo.at(1);
      double y1 = ginfo.at(3);
      double y2 = ginfo.at(4);
      double z  = ginfo.at(6);

      QString str = title.arg(ii + 1).
                    arg(Attr_to_char(x_param)).arg(x1, 0, 'f', 2).arg(x2, 0, 'f', 2).
                    arg(Attr_to_char(y_param)).arg(y1, 0, 'f', 2).arg(y2, 0, 'f', 2).
                    arg(Attr_to_char(z_param)).arg(z,  0, 'f', 2);
      lw_grids->addItem(str);
   }

   if ( row == -1 ) row = 0;
   lw_grids->setCurrentRow( row );

   connect(lw_grids, &QListWidget::currentRowChanged, this, &US_Grid_Editor::highlight);
   lw_grids->setMouseTracking(true);
   highlight(row);
}

double US_Grid_Editor::value4plot(int ii, int jj, int pid)
{
   double val = 0;
   grid_points[ii][jj].value_by_name( Attr_to_char( pid ), val);
   if ( pid == ATTR_S || pid == ATTR_SR ) {
      val *= 1e13;
   } else if ( pid == ATTR_M ) {
      val /= 1000.0;
   }
   return val;
}

void US_Grid_Editor::clear_xyz()
{
   le_x_min->clear();
   le_x_max->clear();
   le_y_min->clear();
   le_y_max->clear();
   le_z_val->clear();
   wg_add_update->setDisabled(true);
}

void US_Grid_Editor::get_xyz(QHash<QString, double> & vals)
{
   vals.clear();
   double x_min = le_x_min->text().toDouble();
   double x_max = le_x_max->text().toDouble();
   int    x_res = le_x_res->text().toDouble();
   double y_min = le_y_min->text().toDouble();
   double y_max = le_y_max->text().toDouble();
   int    y_res = le_y_res->text().toDouble();
   double z_val = le_z_val->text().toDouble();

   vals.insert("xRes", x_res);
   vals.insert("yRes", y_res);

   vals.insert("xMinU", x_min);
   vals.insert("xMaxU", x_max);
   vals.insert("yMinU", y_min);
   vals.insert("yMaxU", y_max);
   vals.insert("zValU", z_val);

   unit_corr(x_min, x_param);
   unit_corr(x_max, x_param);
   unit_corr(y_min, y_param);
   unit_corr(y_max, y_param);
   unit_corr(z_val, z_param);

   vals.insert("xMin", x_min);
   vals.insert("xMax", x_max);
   vals.insert("yMin", y_min);
   vals.insert("yMax", y_max);
   vals.insert("zVal", z_val);
}

// reset the GUI
void US_Grid_Editor::reset( void )
{
   rm_all_items();
   lw_grids->disconnect();
   lw_grids->clear();
   grid_points.clear();
   grid_info.clear();

   x_param = ATTR_S; // plot s
   y_param = ATTR_K; // plot f/f0
   z_param = ATTR_V; // fixed vbar
   // gstate = G_ADD;

   le_x_param->setText( Attr_to_long(x_param) );
   le_y_param->setText( Attr_to_long(y_param) );
   le_z_param->setText( Attr_to_long(z_param) );
   lb_x_ax->setText( Attr_to_short( x_param ));
   lb_y_ax->setText( Attr_to_short( y_param ));
   lb_z_ax->setText( Attr_to_short( z_param ));

   x_axis->button(x_param)->setChecked(true);
   y_axis->button(y_param)->setChecked(true);
   plot_all();
   le_x_res->setText(QString::number(64));
   le_y_res->setText(QString::number(64));
   clear_xyz();
}

// save the grid data
// void US_Grid_Editor::save( void )
// {
   // US_Model model;
   // US_Model::SimulationComponent sc;
   // QString modelPath, modelGuid;
   // US_Model::model_path( modelPath );
   // QDateTime now_time = QDateTime::currentDateTime ();
   // QList< double > xvals;
   // QList< double > yvals;
   // QList< double > zvals;
   // double          gridinc = 1.0 / (double)subGrids;
   // int             indexsg = 1;
   // bool flag;
   // modelGuid         = US_Util::new_guid();
   // model.analysis    = US_Model::CUSTOMGRID;
   // model.description = now_time.toString( "yyyyMMdd-hhmm")
   //    + "-CustomGrid" + ".model";
   // model.subGrids    = subGrids;
   // model.modelGUID   = modelGuid;
   // model.global      = US_Model::NONE;
   // double vbmin      = 1e99;
   // double vbmax      = -1e99;
   // double ffmin      = 1e99;
   // double ffmax      = -1e99;
   // sc.signal_concentration = 1.0;

   // for ( int ii = 0; ii < final_grid.size(); ii++ ) D_20W_to_R
   // {
   //    flag        = true;
   //    sc.s        = final_grid[ ii ].s * 1.0e-13;
   //    sc.D        = final_grid[ ii ].D;
   //    sc.f        = final_grid[ ii ].f;
   //    sc.f_f0     = final_grid[ ii ].ff0;
   //    sc.vbar20   = final_grid[ ii ].vbar;
   //    sc.mw       = final_grid[ ii ].mw;

   //    double xval = grid_value( final_grid[ ii ], x_param );
   //    double yval = grid_value( final_grid[ ii ], y_param );
   //    double zval = grid_value( final_grid[ ii ], z_param );
   //    int indexx  = xvals.indexOf( xval ) + 1;
   //    int indexy  = yvals.indexOf( yval ) + 1;
   //    int indexz  = zvals.indexOf( zval ) + 1;
   //    if ( indexx < 1 )  { indexx = xvals.size() + 1; xvals << xval; }
   //    if ( indexy < 1 )  { indexy = yvals.size() + 1; yvals << yval; }
   //    if ( indexz < 1 )  { indexz = zvals.size() + 1; zvals << zval; }
   //    sc.name     = QString::asprintf( "X%3.3dY%3.3dZ%2.2d",
   //                                     indexx, indexy, indexz );
   //    sc.signal_concentration = gridinc * (double)indexsg;
   //    if ( (++indexsg) > subGrids )  indexsg = 1;

   //    for ( int jj = 0; jj < model.components.size(); jj++ )
   //    {
   //       vbmin     = qMin( vbmin, sc.vbar20 );  // Accumulate vbar,f/f0 extents
   //       vbmax     = qMax( vbmax, sc.vbar20 );
   //       ffmin     = qMin( ffmin, sc.f_f0   );
   //       ffmax     = qMax( ffmax, sc.f_f0   );

   //       if ( sc.s      == model.components[ jj ].s     &&
   //            sc.f_f0   == model.components[ jj ].f_f0  &&
   //            sc.mw     == model.components[ jj ].mw    &&
   //            sc.vbar20 == model.components[ jj ].vbar20 )
   //       {
   //          flag = false;
   //          break; // don't add a component that is already in the model
   //       }
   //    }

   //    if ( flag ) model.components.push_back(sc);
   // }

   // // Open a dialog that reports and allows modification of description
   // QMessageBox mbox;
   // QString msg1    = tr( "A grid model has been created. "
   //                       "It's description is:<br/><b>" )
   //    + model.description + "</b>.<br/><br/>"
   //    + tr( "Click:<br/><br/>" )
   //    + tr( "  <b>OK</b>     to output the model as is;<br/>"
   //          "  <b>Edit</b>   to append custom text to the name;<br/>"
   //          "  <b>Cancel</b> to abort model creation.<br/>" );

   // mbox.setWindowTitle( tr( "Save Grid Model" ) );
   // mbox.setText       ( msg1 );
   // QPushButton *pb_ok   = mbox.addButton( tr( "OK" ),
   //       QMessageBox::YesRole );
   // QPushButton *pb_edit = mbox.addButton( tr( "Edit" ) ,
   //       QMessageBox::AcceptRole );
   // QPushButton *pb_canc = mbox.addButton( tr( "Cancel" ),
   //       QMessageBox::RejectRole );
   // mbox.setEscapeButton ( pb_canc );
   // mbox.setDefaultButton( pb_ok   );

   // mbox.exec();

   // if ( mbox.clickedButton() == pb_canc )  return;

   // if ( mbox.clickedButton() == pb_edit )
   // {  // Open another dialog to get a modified runID
   //    bool    ok;
   //    QString newtext = "";
   //    int     jj      = model.description.indexOf( ".model" );
   //    if ( jj > 0 ) model.description = model.description.left( jj );
   //    QString msg2    = tr( "The default run ID for the grid model<br/>"
   //                          "is <b>" ) + model.description + "</b>.<br/><br/>"
   //       + tr( "You may append additional text to the model description.<br/>"
   //             "Use alphanumeric characters, underscores, or hyphens<br/>"
   //             "(no spaces). Enter 1 to 40 characters." );
   //    newtext = QInputDialog::getText( this,
   //          tr( "Modify Model Name" ),
   //          msg2,
   //          QLineEdit::Normal,
   //          newtext,
   //          &ok );

   //    if ( !ok )  return;

   //    newtext.remove( QRegExp( "[^\\w\\d_-]" ) );

   //    int     slen    = newtext.length();
   //    if ( slen > 40 ) newtext = newtext.left( 40 );
   //    // add string containing
   //    model.description = model.description + "-" + newtext + ".model";
   // }

   // // Output the combined grid model
   // int code;
   // if ( dkdb_cntrls->db() )
   // {
   //    US_Passwd pw;
   //    US_DB2 db( pw.getPasswd() );
   //    code = model.write( &db );
   // }
   // else
   // {
   //    bool newFile;
   //    QString fnamo = US_Model::get_filename( modelPath, modelGuid, newFile );
   //    code = model.write( fnamo );
   // }

   // QString mtitle = tr( "Grid Model Saving..." );

   // if ( code == US_DB2::OK )
   // {
   //    QString destination = dkdb_cntrls->db() ?
   //                          tr ( "local disk and database." ) :
   //                          tr ( "local disk." );
   //    QMessageBox::information( this, mtitle,
   //       tr( "The file \"" ) +  model.description
   //       + tr( "\"\n  was successfully saved to " ) + destination );
   // }
   // else
   // {
   //    QMessageBox::warning( this, mtitle,
   //       tr( "Writing the model file  \"") +  model.description
   //       + tr( "\"\n  resulted in error code " )
   //       + QString::number( code ) + " ." );
   // }
// }

// update plot limit x min
void US_Grid_Editor::update_xMin( )
{
   plot_tmp();
}

// update plot limit x max
void US_Grid_Editor::update_xMax( )
{
   plot_tmp();
}

// update plot limit y min
void US_Grid_Editor::update_yMin( )
{
   plot_tmp();
}

// update plot limit y max
void US_Grid_Editor::update_yMax( )
{
   plot_tmp();
}

void US_Grid_Editor::highlight(int id)
{
   rm_tmp_items();
   QString title = tr("GRID_%1").arg(id);
   auto items = data_plot->itemList(QwtPlotItem::Rtti_PlotCurve);
   for (int ii = 0; ii < items.size(); ii++) {
      QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>(items.at(ii));
      if (curve) {
         const QwtSymbol* symbol = curve->symbol();
         if (symbol) {
            QPen pen = symbol->pen();
            QwtSymbol* new_symbol = new QwtSymbol();
            new_symbol->setStyle(symbol->style());
            new_symbol->setSize(symbol->size());
            if ( curve->title() == title ) {
               pen.setColor(Qt::red);
               new_symbol->setBrush(Qt::red);
            } else {
               pen.setColor(Qt::yellow);
               new_symbol->setBrush(Qt::yellow);
            }
            new_symbol->setPen(pen);
            curve->setSymbol(new_symbol);
         }
      }
   }
   data_plot->replot();
   clear_xyz();
   if ( id != -1 ) {
      QVector<double> ginfo = grid_info.at(id);
      le_x_min->setText(QString::number(ginfo.at(0)));
      le_x_max->setText(QString::number(ginfo.at(1)));
      le_x_res->setText(QString::number(ginfo.at(2)));
      le_y_min->setText(QString::number(ginfo.at(3)));
      le_y_max->setText(QString::number(ginfo.at(4)));
      le_y_res->setText(QString::number(ginfo.at(5)));
      le_z_val->setText(QString::number(ginfo.at(6)));
   }
}

void US_Grid_Editor::new_grid_clicked()
{

   clear_xyz();
   plot_flag = false;
   x_axis->button(x_param)->setChecked(true);
   y_axis->button(y_param)->setChecked(true);
   select_x_axis(x_param);
   select_y_axis(y_param);
   plot_flag = true;
   plot_all();
   highlight(-1);
   wg_add_update->setEnabled(true);
}

void US_Grid_Editor::update_grid_clicked()
{

}

void US_Grid_Editor::delete_grid_clicked()
{

}

void US_Grid_Editor::select_x_axis(int index)
{
   for (int ii = ATTR_S; ii <= ATTR_DR; ii++) {
      y_axis->button( ii )->setEnabled(true);
   }
   y_axis->button(index)->setDisabled(true);

   if ( index == ATTR_S ) {
      y_axis->button(ATTR_SR)->setDisabled(true);
   } else if ( index == ATTR_D ) {
      y_axis->button(ATTR_DR)->setDisabled(true);
   } else if ( index == ATTR_SR ) {
      y_axis->button(ATTR_S)->setDisabled(true);
   } else if ( index == ATTR_DR ) {
      y_axis->button(ATTR_D)->setDisabled(true);
   }
   if ( plot_flag ) {
      plot_all();
      int row = lw_grids->currentRow();
      highlight(row);
   }
   wg_add_update->setDisabled(true);
}

void US_Grid_Editor::select_y_axis(int index)
{
   for (int ii = ATTR_S; ii <= ATTR_DR; ii++) {
      x_axis->button( ii )->setEnabled(true);
   }
   x_axis->button(index)->setDisabled(true);

   if ( index == ATTR_S ) {
      x_axis->button(ATTR_SR)->setDisabled(true);
   } else if ( index == ATTR_D ) {
      x_axis->button(ATTR_DR)->setDisabled(true);
   } else if ( index == ATTR_SR ) {
      x_axis->button(ATTR_S)->setDisabled(true);
   } else if ( index == ATTR_DR ) {
      x_axis->button(ATTR_D)->setDisabled(true);
   }
   if ( plot_flag ) {
      plot_all();
      int row = lw_grids->currentRow();
      highlight(row);
   }
   wg_add_update->setDisabled(true);
}

void US_Grid_Editor::update_symsize(double)
{
   int ssize = ct_size->value();
   auto items = data_plot->itemList(QwtPlotItem::Rtti_PlotCurve);
   for (int ii = 0; ii < items.size(); ii++) {
      QwtPlotCurve* curve = dynamic_cast<QwtPlotCurve*>(items.at(ii));
      if (curve) {
         const QwtSymbol* symbol = curve->symbol();
         if (symbol) {
            QwtSymbol* new_symbol = new QwtSymbol();
            new_symbol->setStyle(symbol->style());
            new_symbol->setBrush(symbol->brush());
            new_symbol->setPen(symbol->pen());
            new_symbol->setSize(ssize, ssize);
            curve->setSymbol(new_symbol);
         }
      }
   }
   data_plot->replot();
}

void US_Grid_Editor::plot_all()
{
   rm_all_items();
   int xid = x_axis->checkedId();
   int yid = y_axis->checkedId();

   data_plot->setAxisTitle( QwtPlot::xBottom, Attr_to_title(xid) );
   data_plot->setAxisTitle( QwtPlot::yLeft,   Attr_to_title(yid) );
   data_plot->setAxisScale( QwtPlot::xBottom, 1, 10);
   data_plot->setAxisScale( QwtPlot::yLeft  , 1, 10);
   data_plot->replot();

   if ( grid_points.isEmpty() ) return;

   double px1 =  1e99;
   double px2 = -1e99;
   double py1 =  1e99;
   double py2 = -1e99;
   int pxid = x_axis->checkedId();
   int pyid = y_axis->checkedId();
   int ssize = ct_size->value();
   for ( int nn = 0; nn < grid_points.size(); nn++ ) {
      QVector<GridPoint> gps = grid_points.at(nn);
      QVector<double> xarr;
      QVector<double> yarr;
      for ( int ii = 0; ii < gps.size(); ii++ ) {
         double x = value4plot(nn, ii, pxid);
         double y = value4plot(nn, ii, pyid);

         px1 = qMin(px1, x);
         px2 = qMax(px2, x);
         py1 = qMin(py1, y);
         py2 = qMax(py2, y);

         xarr << x;
         yarr << y;
      }
      QString title = tr("GRID_%1").arg(nn);
      QwtPlotCurve* curve = us_curve(data_plot, title);

      QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse, QBrush(Qt::yellow),
                                        QPen(Qt::yellow), QSize(ssize, ssize));
      curve->setSymbol(symbol);
      curve->setStyle(QwtPlotCurve::NoCurve);
      curve->setSamples(xarr.data(), yarr.data(), xarr.size());
   }

   double dx = (px2 - px1) * 0.1;
   double dy = (py2 - py1) * 0.1;
   px1 -= dx;
   px2 += dx;
   py1 -= dy;
   py2 += dy;
   data_plot->setAxisScale( QwtPlot::xBottom, px1, px2);
   data_plot->setAxisScale( QwtPlot::yLeft  , py1, py2);
   data_plot->replot();
}



// // Select a partialGrid from all subgrids in the final grid for highlighting
// void US_Grid_Editor::update_partialGrid( double dval )
// {
//    partialGrid = (int) dval;
//    update_plot();
// }

// // Select a subgrid from the final grid for highlighting:
// void US_Grid_Editor::update_subGrids( double dval )
// {
//    int ntotg       = final_grid.size();
//    subGrids        = (int)dval;
//    // ct_partialGrid->setRange     ( 1, subGrids );
//    // ct_partialGrid->setSingleStep( 1 );
//    // le_counts->setText( tr( "%1 total, %2 per subgrid" )
//    //       .arg( ntotg ).arg( ntotg / subGrids ) );
//    update_plot();
// }

// // update density
// void US_Grid_Editor::update_exp_data( )
// {

// }

// // update plot
// void US_Grid_Editor::update_plot( void )
// {
// qDebug() << "update_plot:  call calc_gridpoints()";

//    QString xatitle = Attr_to_long( x_param );
//    QString yatitle = Attr_to_long( y_param );

//    if ( selected_plot == 1 )
//       xatitle         = tr( "Molecular Weight" );

//    dataPlotClear( data_plot );
//    data_plot->setAxisTitle( QwtPlot::xBottom, xatitle );
//    data_plot->setAxisTitle( QwtPlot::yLeft,   yatitle );

//    //print_minmax();
//    int gridsize;
//    QVector <double> xData1;
//    QVector <double> yData1;
//    QVector <double> xData2;
//    QVector <double> yData2;

//    int iplt_x = ( selected_plot == 0 ) ? x_param : ATTR_W;

//    xData1.clear();
//    yData1.clear();
//    xData2.clear();
//    yData2.clear();

//    // if ( ck_show_final_grid->isChecked()  &&
//    //      !ck_show_sub_grid->isChecked())
//       if (true)
//    {
//       gridsize = final_grid.size();

//       // for ( int ii = 0; ii < gridsize; ii++ )
//       // {
//       //    if ( final_grid[ ii ].index == partialGrid )
//       //    {
//       //       xData1 << grid_value( final_grid[ ii ], iplt_x );
//       //       yData1 << grid_value( final_grid[ ii ], y_param );
//       //    }

//       //    else
//       //    {
//       //       xData2 << grid_value( final_grid[ ii ], iplt_x );
//       //       yData2 << grid_value( final_grid[ ii ], y_param );
//       //    }
//       // }

//       QwtPlotCurve *c1;
//       QwtSymbol*   sym1 = new QwtSymbol;
//       sym1->setStyle( QwtSymbol::Ellipse );
//       sym1->setBrush( QColor( Qt::red ) );
//       sym1->setPen  ( QColor( Qt::red ) );
//       sym1->setSize ( 3 );

//       c1 = us_curve( data_plot, "highlighted Grid points" );
//       c1->setSymbol ( sym1 );
//       c1->setStyle  ( QwtPlotCurve::NoCurve );
//       c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

//       QwtPlotCurve *c2;
//       QwtSymbol*   sym2 = new QwtSymbol;
//       sym2->setStyle( QwtSymbol::Ellipse );
//       sym2->setBrush( QColor( Qt::yellow ) );
//       sym2->setPen  ( QColor( Qt::yellow ) );
//       sym2->setSize ( 3 );

//       c2 = us_curve( data_plot, "Other Grid points" );
//       c2->setSymbol ( sym2 );
//       c2->setStyle  ( QwtPlotCurve::NoCurve );
//       c2->setSamples( xData2.data(), yData2.data(), xData2.size() );
//    }

//    // else if ( ck_show_final_grid->isChecked()  &&
//    //           ck_show_sub_grid->isChecked())
//       else if (0)
//    {
//       gridsize    = final_grid.size();
//       int counter = 1;

//       // for ( int ii = 0; ii < gridsize; ii++ )
//       // {
//       //    if ( counter == partialGrid )
//       //    {
//       //       xData1 << grid_value( final_grid[ ii ], iplt_x );
//       //       yData1 << grid_value( final_grid[ ii ], y_param );
//       //    }

//       //    else
//       //    {
//       //       xData2 << grid_value( final_grid[ ii ], iplt_x );
//       //       yData2 << grid_value( final_grid[ ii ], y_param );
//       //    }

//       //    counter++;

//       //    if ( counter > subGrids )
//       //       counter = 1;
//       // }

//       QwtPlotCurve *c1;
//       QwtSymbol*   sym1 = new QwtSymbol;
//       sym1->setStyle( QwtSymbol::Ellipse );
//       sym1->setBrush( QColor( Qt::red ) );
//       sym1->setPen  ( QColor( Qt::red ) );
//       sym1->setSize( 3 );

//       c1 = us_curve( data_plot, "highlighted Grid points" );
//       c1->setSymbol ( sym1 );
//       c1->setStyle  ( QwtPlotCurve::NoCurve );
//       c1->setSamples( xData1.data(), yData1.data(), xData1.size() );

//       QwtPlotCurve *c2;
//       QwtSymbol*   sym2 = new QwtSymbol;
//       sym2->setStyle( QwtSymbol::Ellipse );
//       sym2->setBrush( QColor( Qt::yellow ) );
//       sym2->setPen  ( QColor( Qt::yellow ) );
//       sym2->setSize( 3 );

//       c2 = us_curve( data_plot, "Other Grid points" );
//       c2->setSymbol ( sym2 );
//       c2->setStyle  ( QwtPlotCurve::NoCurve );
//       c2->setSamples( xData2.data(), yData2.data(), xData2.size() );
//    }

//    else
//    {  // Set up current grid plot
//       // gridsize    = current_grid.size();
// qDebug() << "  updplt: gridsize" << gridsize;
//       xData1.resize( gridsize );
//       yData1.resize( gridsize );

//       // for ( int ii = 0; ii < gridsize; ii++ )
//       // {
//       //    xData1[ ii ] = grid_value( current_grid[ ii ], iplt_x );
//       //    yData1[ ii ] = grid_value( current_grid[ ii ], y_param );
//       // }

//       QwtPlotCurve *c1;
//       QwtSymbol*   sym1 = new QwtSymbol;
//       sym1->setStyle( QwtSymbol::Ellipse );
//       sym1->setBrush( QColor( Qt::yellow ) );
//       sym1->setPen  ( QColor( Qt::yellow ) );
//       sym1->setSize( 3 );

//       c1 = us_curve( data_plot, "Grid points 1" );
//       c1->setSymbol ( sym1 );
//       c1->setStyle  ( QwtPlotCurve::NoCurve );
//       c1->setSamples( xData1.data(), yData1.data(), gridsize );
//    }

//    data_plot->setAxisAutoScale( QwtPlot::xBottom );
//    data_plot->setAxisAutoScale( QwtPlot::yLeft );
//    data_plot->replot();
// }

// // delete current grid
// void US_Grid_Editor::delete_partialGrid( void )
// {
//    // for (int i=final_grid.size() - 1; i>=0; i--)
//    // {
//    //    if (final_grid[i].index == partialGrid)
//    //    {
//    //       final_grid.removeAt(i);
//    //    }
//    // }
//    // // renumber index positions
//    // for (int i=0; i<final_grid.size(); i++)
//    // {
//    //    if (final_grid[i].index > partialGrid)
//    //    {
//    //       final_grid[i].index--;
//    //    }
//    // }
//    grid_index--;
//    // ct_partialGrid->setRange     ( 1, grid_index );
//    // ct_partialGrid->setSingleStep( 1 );
//    // if (grid_index == 0)
//    // {
//    //    ck_show_final_grid->setChecked (false);
//    //    ck_show_final_grid->setEnabled (false);
//    //    ct_partialGrid->setRange      ( 0, 0 );
//    //    ct_partialGrid->setSingleStep ( 1 );
//    //    ct_partialGrid->setEnabled( false );
//    //    show_final_grid( false );
//    // }
//    update_plot();
// }

// // Select plot1 (X-axis view) or plot2 (Molecular Weight view)
// void US_Grid_Editor::select_plot( int ival )
// {
//    selected_plot   = ival;

//    update_plot();
// }

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

// // Adjust value/ranges so that f/f0 is not less than 1
// bool US_Grid_Editor::validate_ff0()
// {
// //    bool is_ok     = true;

// //    if ( x_param == ATTR_K  ||  y_param == ATTR_K  ||  z_param == ATTR_K )
// //    {  // If one of the attributes is f/f0, no need for checking
// //       return is_ok;
// //    }

// //    double xMin = le_x_min->text().toDouble();
// //    double xMax = le_x_max->text().toDouble();
// //    double yMin = le_y_min->text().toDouble();
// //    double yMax = le_y_max->text().toDouble();
// //    double zVal = le_z_val->text().toDouble();
// // qDebug() << "valFF0: xMin xMax yMin yMax" << xMin << xMax << yMin << yMax;
// //    clear_grid( zpoint );
// //    set_grid_value( zpoint,    z_param, zVal );
// //    // Get f/f0 for xMin,yMin
// //    tmp_point      = zpoint;
// //    set_grid_value( tmp_point, x_param, xMin );
// //    set_grid_value( tmp_point, y_param, yMin );
// //    complete_comp ( tmp_point );
// //    double ffx1y1  = tmp_point.ff0;
// //    // Get f/f0 for xMin,yMax
// //    tmp_point      = zpoint;
// //    set_grid_value( tmp_point, x_param, xMin );
// //    set_grid_value( tmp_point, y_param, yMax );
// //    complete_comp ( tmp_point );
// //    double ffx1y2  = tmp_point.ff0;
// //    // Get f/f0 for xMax,yMin
// //    tmp_point      = zpoint;
// //    set_grid_value( tmp_point, x_param, xMax );
// //    set_grid_value( tmp_point, y_param, yMin );
// //    complete_comp ( tmp_point );
// //    double ffx2y1  = tmp_point.ff0;
// //    // Get f/f0 for xMax,yMax
// //    tmp_point      = zpoint;
// //    set_grid_value( tmp_point, x_param, xMax );
// //    set_grid_value( tmp_point, y_param, yMax );
// //    complete_comp ( tmp_point );
// //    double ffx2y2  = tmp_point.ff0;
// //    // Get overall minimum f/f0
// //    double ff0min  = qMin( ffx1y1, ffx1y2 );
// //    ff0min         = qMin( ff0min, ffx2y1 );
// //    ff0min         = qMin( ff0min, ffx2y2 );
// // qDebug() << "valFF0: zVal xMin yMin ff0" << zVal << xMin << yMin << ffx1y1;
// // qDebug() << "valFF0:      xMin yMax ff0   " << xMin << yMax << ffx1y2;
// // qDebug() << "valFF0:      xMax yMin ff0   " << xMax << yMin << ffx2y1;
// // qDebug() << "valFF0:      xMax yMax ff0   " << xMax << yMax << ffx2y2;

// //    if ( ff0min < 1.0 )
// //    {  // Ranges include values that set f/f0 less than 1:  must adjust ranges

// //       if ( x_param == ATTR_W  ||  x_param == ATTR_F )
// //       {  // Adjust the X range (if MW or f)
// //          // ct_xMin->disconnect();
// //          // ct_xMax->disconnect();
// //          tmp_point      = zpoint;
// //          set_grid_value( tmp_point, ATTR_K, 1.0  );

// //          if ( ffx1y1 < ffx1y2 )
// //          {  // Increasing Y means increasing f/f0, so get X for k=1,ymin
// //             set_grid_value( tmp_point, y_param, yMin );
// //             complete_comp ( tmp_point );
// //             double xVal    = grid_value( tmp_point, x_param );

// //             if ( ffx1y1 < ffx2y1 )
// //             {  // Increasing X means increasing f/f0, so set lower limit
// // qDebug() << "valFF0:  (1xMin)xVal" << xVal;
// //                // ct_xMin->setMinimum( xVal );
// //                // ct_xMax->setMinimum( xVal );
// //             }

// //             else
// //             {  // Increasing X means decreasing f/f0, so set upper limit
// // qDebug() << "valFF0:  (2xMin)xVal" << xVal;
// //                // ct_xMin->setMaximum( xVal );
// //                // ct_xMax->setMaximum( xVal );
// //             }
// //          }

// //          else
// //          {  // Increasing Y means decreasing f/f0, so get X for k=1,ymax
// //             set_grid_value( tmp_point, y_param, yMax );
// //             complete_comp ( tmp_point );
// //             double xVal    = grid_value( tmp_point, x_param );

// //             if ( ffx1y1 < ffx2y1 )
// //             {  // Increasing X means increasing f/f0, so set lower limit
// // qDebug() << "valFF0:  (3xMin)xVal" << xVal;
// //                // ct_xMin->setMinimum( xVal );
// //                // ct_xMax->setMinimum( xVal );
// //             }

// //             else
// //             {  // Increasing X means decreasing f/f0, so set upper limit
// // qDebug() << "valFF0:  (4xMin)xVal" << xVal;
// //                // ct_xMin->setMaximum( xVal );
// //                // ct_xMax->setMaximum( xVal );
// //             }
// //          }

// //          // connect( ct_xMin, SIGNAL( valueChanged( double ) ),
// //          //          this,    SLOT  ( update_xMin ( double ) ) );
// //          // connect( ct_xMax, SIGNAL( valueChanged( double ) ),
// //          //          this,    SLOT  ( update_xMax ( double ) ) );
// //       }

// //       if ( y_param == ATTR_W  ||  y_param == ATTR_F )
// //       {  // Adjust the Y range (if MW or f)
// //          // ct_yMin->disconnect();
// //          // ct_yMax->disconnect();
// //          tmp_point      = zpoint;
// //          set_grid_value( tmp_point, ATTR_K, 1.0  );

// //          if ( ffx1y1 < ffx2y1 )
// //          {  // Increasing X means increasing f/f0, so get Y for k=1,xmin
// //             set_grid_value( tmp_point, x_param, xMin );
// //             complete_comp ( tmp_point );
// //             double yVal    = grid_value( tmp_point, y_param );

// //             if ( ffx1y1 < ffx1y2 )
// //             {  // Increasing Y means increasing f/f0, so set lower limit
// // qDebug() << "valFF0:  (5yMin)yVal" << yVal;
// //                // ct_yMin->setMinimum( yVal );
// //                // ct_yMax->setMinimum( yVal );
// //             }

// //             else
// //             {  // Increasing Y means decreasing f/f0, so set upper limit
// // qDebug() << "valFF0:  (6yMax)yVal" << yVal;
// //                // ct_yMin->setMaximum( yVal );
// //                // ct_yMax->setMaximum( yVal );
// //             }
// //          }

// //          else
// //          {  // Increasing X means decreasing f/f0, so get y for k=1,xmax
// //             set_grid_value( tmp_point, x_param, xMax );
// //             complete_comp ( tmp_point );
// //             double yVal    = grid_value( tmp_point, y_param );

// //             if ( ffx1y1 < ffx1y2 )
// //             {  // Increasing Y means increasing f/f0, so set lower limit
// // qDebug() << "valFF0:  (7yMin)yVal" << yVal;
// //                // ct_yMin->setMinimum( yVal );
// //                // ct_yMax->setMinimum( yVal );
// //             }

// //             else
// //             {  // Increasing Y means decreasing f/f0, so set upper limit
// // qDebug() << "valFF0:  (8yMax)yVal" << yVal;
// //                // ct_yMin->setMaximum( yVal );
// //                // ct_yMax->setMaximum( yVal );
// //             }
// //          }

// //          // connect( ct_yMin, SIGNAL( valueChanged( double ) ),
// //          //          this,    SLOT  ( update_yMin ( double ) ) );
// //          // connect( ct_yMax, SIGNAL( valueChanged( double ) ),
// //          //          this,    SLOT  ( update_yMax ( double ) ) );
// //       }
// //    }

// //    // xMin           = ct_xMin->value();
// //    // xMax           = ct_xMax->value();
// //    // yMin           = ct_yMin->value();
// //    // yMax           = ct_yMax->value();
// // qDebug() << "valFF0: (out)xMin xMax yMin yMax" << xMin << xMax << yMin << yMax;

// //    return is_ok;
// return false;
// }

// // Complete component where 3 attributes are given
// bool US_Grid_Editor::complete_comp( struct gridpoint& gpoint )
// {
//    // US_Model::SimulationComponent sc;
//    // sc.s          = gpoint.s * 1.0e-13;
//    // sc.f_f0       = gpoint.ff0;
//    // sc.mw         = gpoint.mw;
//    // sc.vbar20     = gpoint.vbar;
//    // sc.D          = gpoint.D;
//    // sc.f          = gpoint.f;

//    // bool is_ok    = US_Model::calc_coefficients( sc );

//    // if ( is_ok )
//    // {
//    //    gpoint.s      = sc.s * 1.0e+13;
//    //    gpoint.ff0    = sc.f_f0;
//    //    gpoint.mw     = sc.mw;
//    //    gpoint.vbar   = sc.vbar20;
//    //    gpoint.D      = sc.D;
//    //    gpoint.f      = sc.f;
//    // }

//    return false;
// }


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
   QGridLayout* x_mw    = us_radiobutton( Attr_to_long( ATTR_M ), rb_x_mw, true );
   QGridLayout* x_vbar  = us_radiobutton( Attr_to_long( ATTR_V ), rb_x_vbar, true );
   QGridLayout* x_D     = us_radiobutton( Attr_to_long( ATTR_D ), rb_x_D, true );
   QGridLayout* x_f     = us_radiobutton( Attr_to_long( ATTR_F ), rb_x_f, true );

   QGridLayout* y_s     = us_radiobutton( Attr_to_long( ATTR_S ), rb_y_s, false );
   QGridLayout* y_ff0   = us_radiobutton( Attr_to_long( ATTR_K ), rb_y_ff0, true );
   QGridLayout* y_mw    = us_radiobutton( Attr_to_long( ATTR_M ), rb_y_mw, true );
   QGridLayout* y_vbar  = us_radiobutton( Attr_to_long( ATTR_V ), rb_y_vbar, true );
   QGridLayout* y_D     = us_radiobutton( Attr_to_long( ATTR_D ), rb_y_D, true );
   QGridLayout* y_f     = us_radiobutton( Attr_to_long( ATTR_F ), rb_y_f, true );

   x_axis = new QButtonGroup( this );
   x_axis->addButton( rb_x_s,    ATTR_S );
   x_axis->addButton( rb_x_ff0,  ATTR_K );
   x_axis->addButton( rb_x_mw,   ATTR_M );
   x_axis->addButton( rb_x_vbar, ATTR_V );
   x_axis->addButton( rb_x_D,    ATTR_D );
   x_axis->addButton( rb_x_f,    ATTR_F );

   y_axis = new QButtonGroup( this );
   y_axis->addButton( rb_y_s,    ATTR_S );
   y_axis->addButton( rb_y_ff0,  ATTR_K );
   y_axis->addButton( rb_y_mw,   ATTR_M );
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
   bool is_DF = x_param == ATTR_F || x_param == ATTR_D ||
                y_param == ATTR_F || y_param == ATTR_D;
   if ( has_vbar ) {
      for (int ii = ATTR_S; ii <= ATTR_F; ii++) {
         if ( ii == x_param || ii == y_param ) continue;
         if ( is_DF && ( ii == ATTR_F || ii == ATTR_D ) ) continue;
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

GridPoint::GridPoint()
{
   dvt_set = false;
   S_real = 0;
   D_real = 0;
}

bool GridPoint::set_param(const QVector<double>& values,
                          const QVector<int>& types)
{
   if ( values.size() != 3 || types.size() != 3 ) {
      error = "Three parameters are needed to set the grid point.";
      return false;
   }
   ptypes.clear();

   for ( int ii =0; ii < 3; ii++ ) {
      ptypes.insert( types.at(ii) );
      if ( types.at(ii) == ATTR_S ) {
         S = values.at(ii);
         if ( S == 0 ) {
            error = "Sedimentation value is zero. Please correct it!";
            return false;
         }
      } else if ( types.at(ii) == ATTR_K ) {
         FF0 = values.at(ii);
         if ( FF0 < 1 ) {
            error = "Frictional ratio is less than one. Please correct it!";
            return false;
         }
      } else if ( types.at(ii) == ATTR_M ) {
         MW = values.at(ii);
         if ( MW < 1 ) {
            error = "Molecular weight is less than one. Please correct it!";
            return false;
         }
      } else if ( types.at(ii) == ATTR_V ) {
         VBAR = values.at(ii);
         if ( VBAR <= 0 ) {
            error = "Partial specific volume is less than or equal to zero. Please correct it!";
            return false;
         }
      } else if ( types.at(ii) == ATTR_D ) {
         D = values.at(ii);
         if ( D <= 0 ) {
            error = "Diffusion coefficient is less than or equal to zero. Please correct it!";
            return false;
         }
      } else if ( types.at(ii) == ATTR_F ) {
         F = values.at(ii);
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

bool GridPoint::set_dens_visc_t(double dens, double visc, double T)
{
   error.clear();
   if ( dens < 0.001 ) {
      error = "Buffer density is less than 0.001. Please correct it!";
      return false;
   }
   if ( visc < 0.001 ) {
      error = "Buffer viscosity is less than 0.001. Please correct it!";
      return false;
   }
   if ( T < 1 ) {
      error = "Buffer temperature is less than one. Please correct it!";
      return false;
   }
   density = dens;
   viscosity = visc;
   temperature = T;
   dvt_set = true;

   if ( ptypes.size() == 3 ) {
      calculate_real();
   }
   return true;
}

QString GridPoint::error_string()
{
   return error;
}

bool GridPoint::value_by_name(const QString &vname, double &val)
{
   bool ok = true;
   if (vname.compare("S", Qt::CaseInsensitive) == 0) {
      val = S;
   } else if (vname.compare("D", Qt::CaseInsensitive) == 0) {
      val = D;
   } else if (vname.compare("VBAR", Qt::CaseInsensitive) == 0) {
      val = VBAR;
   } else if (vname.compare("MW", Qt::CaseInsensitive) == 0) {
      val = MW;
   } else if (vname.compare("F", Qt::CaseInsensitive) == 0) {
      val = F;
   } else if (vname.compare("F/F0", Qt::CaseInsensitive) == 0) {
      val = FF0;
   } else if (vname.compare("F0", Qt::CaseInsensitive) == 0) {
      val = F0;
   } else if (vname.compare("S*", Qt::CaseInsensitive) == 0) {
      val = S_real;
   } else if (vname.compare("D*", Qt::CaseInsensitive) == 0) {
      val = D_real;
   } else {
      ok = false;
   }
   return ok;
}

bool GridPoint::calculate_20w()
{
   error.clear();
   // S, K, M, V, D, F, F0

   if ( contains( ATTR_V, ATTR_S, ATTR_K ) )         // 1: M, D, F, F0
   {
      if ( ! check_s_vbar()) return false;

      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      F = FF0 * F0;
      D = RGK20 / ( AVOGADRO * F );
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_V, ATTR_S, ATTR_M ) )  // 2: K, D, F, F0
   {
      if ( ! check_s_vbar()) return false;

      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      F = MW * buoy / ( AVOGADRO * S );
      D = RGK20 / ( AVOGADRO * F );
      FF0 = F / F0;
   } else if ( contains( ATTR_V, ATTR_S, ATTR_D ) )  // 3: K, M, F, F0
   {
      if ( ! check_s_vbar()) return false;

      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      F = RGK20 / ( AVOGADRO * D );
      FF0 = F / F0;
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_V, ATTR_S, ATTR_F ) )  // 4: K, M, D, F0
   {
      if ( ! check_s_vbar()) return false;

      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      FF0 = F / F0;
      D = RGK20 / ( AVOGADRO * F );
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_V, ATTR_K, ATTR_M ) )  // 5: S, D, F, F0
   {
      double buoy = 1 - VBAR * DENS_20W;
      D = RGK20 / ( 3 * VISC_20WP ) *
              qPow( 6 * MW * VBAR, -1.0 / 3.0 ) *
              qPow( AVOGADRO * M_PI * FF0, -2.0 / 3.0 );
      S = MW * D * buoy / RGK20;
      F = RGK20 / ( AVOGADRO * D );
      F0 = F / FF0;
   } else if ( contains( ATTR_V, ATTR_K, ATTR_D ) )  // 6: S, M, F, F0
   {
      double buoy = 1 - VBAR * DENS_20W;
      F = RGK20 / ( AVOGADRO * D );
      F0 = F / FF0;
      S = qPow( F0 / ( 9 * VISC_20WP * M_PI ), 2 ) * buoy / ( 2 * VISC_20WP * VBAR );
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_V, ATTR_K, ATTR_F ) )  // 7: S, M, D, F0
   {
      double buoy = 1 - VBAR * DENS_20W;
      D = RGK20 / ( AVOGADRO * F );
      F0 = F / FF0;
      S = qPow( F0 / ( 9 * VISC_20WP * M_PI ), 2 ) * buoy / ( 2 * VISC_20WP * VBAR );
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_V, ATTR_M, ATTR_D ) )  // 8: S, K, F, F0
   {
      double buoy = 1 - VBAR * DENS_20W;
      F = RGK20 / ( AVOGADRO * D );
      S = MW * D * buoy / RGK20;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      FF0 = F / F0;
   } else if ( contains( ATTR_V, ATTR_M, ATTR_F ) )  // 9: S, K, D, F0
   {
      double buoy = 1 - VBAR * DENS_20W;
      D = RGK20 / ( AVOGADRO * F );
      S = MW * D * buoy / RGK20;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      FF0 = F / F0;
   } else if ( contains( ATTR_S, ATTR_K, ATTR_M ) )  // 10: V, D, F, F0 ?????
   {
   } else if ( contains( ATTR_S, ATTR_K, ATTR_D ) )  // 11: M, V, F, F0
   {
      F0 = RGK20 / ( AVOGADRO * FF0 * D );
      F = F0 * FF0;
      double f02 = qPow( F0, 2 );
      double vis3 = qPow( VISC_20WP, 3 );
      VBAR = f02 / ( 162 * S * MPISQ *  vis3 + f02 * DENS_20W );
      double buoy = 1 - VBAR * DENS_20W;
      MW = S * AVOGADRO * F / buoy;
   } else if ( contains( ATTR_S, ATTR_K, ATTR_F ) )  // 12: M, V, D, F0
   {
      D = RGK20 / ( AVOGADRO * F );
      F0 = F / FF0;
      double f02 = qPow( F0, 2 );
      double vis3 = qPow( VISC_20WP, 3 );
      VBAR = f02 / ( 162 * S * MPISQ *  vis3 + f02 * DENS_20W );
      double buoy = 1 - VBAR * DENS_20W;
      MW = S * AVOGADRO * F / buoy;
   }
   else if ( contains( ATTR_S, ATTR_M, ATTR_D ) )  // 13: K, V, F, F0
   {
      VBAR = ( 1 - ( S * RGK20 ) / ( MW * D ) ) / DENS_20W;
      F = RGK20 / ( AVOGADRO * D );
      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      FF0 = F / F0;
   } else if ( contains( ATTR_S, ATTR_M, ATTR_F ) )  // 14: K, V, D, F0
   {
      VBAR = ( 1 - S * AVOGADRO * F / MW ) / DENS_20W;
      double buoy = 1 - VBAR * DENS_20W;
      F0 = 9 * VISC_20WP * M_PI * qSqrt( 2 * VBAR * S * VISC_20WP / buoy );
      FF0 = F / F0;
      D = RGK20 / ( AVOGADRO * F );
   } else if ( contains( ATTR_K, ATTR_M, ATTR_D ) )  // 15: S, V, F, F0
   {
      F = RGK20 / ( AVOGADRO * D );
      F0 = F / FF0;
      VBAR = qPow( RGK20 / ( 3 * VISC_20WP * D), 3 ) /
                 ( 6 * MW * qPow(AVOGADRO * K20 * M_PI, 2 ) );
      double buoy = 1 - VBAR * DENS_20W;
      S = MW * buoy / ( AVOGADRO * F );
   } else if ( contains( ATTR_K, ATTR_M, ATTR_F ) )  // 16: S, V, D, F0
   {
      D = RGK20 / ( AVOGADRO * F );
      F0 = F / FF0;
      VBAR = qPow( RGK20 / ( 3 * VISC_20WP * D ), 3 ) /
                 ( 6 * MW * qPow(AVOGADRO * K20 * M_PI, 2 ) );
      double buoy = 1 - VBAR * DENS_20W;
      S = MW * buoy / ( AVOGADRO * F );
   }

   return true;
}

void GridPoint::calculate_real()
{
   if ( ! dvt_set ) return;
   if ( ptypes.isEmpty() ) return;

   for (int ii = 0; ii < 3; ii++) {
      US_Math2::SolutionData sol;
      sol.manual = false;
      sol.vbar = VBAR;
      sol.vbar20 = VBAR;
      sol.density = density;
      sol.viscosity = viscosity;
      US_Math2::data_correction(temperature, sol);
      S_real = S / sol.s20w_correction;
      D_real = D / sol.D20w_correction;
   }
}

bool GridPoint::check_s_vbar()
{
   double buoy = 1 - VBAR * DENS_20W;
   if ( buoy == 0 ) {
      error = "Buoyancy is zero. Correct the vbar value.";
      return false;
   }
   if ( ( buoy > 0 && S < 0 ) || ( buoy < 0 && S > 0 ) ) {
      error  = "Sedimentation and buoyancy should have the same sign!\n";
      error += QObject::tr("Sedimentation = %1\n").arg(buoy);
      error += QObject::tr("Buoyancy = 1 - %1 x vbar = %2\n").arg(DENS_20W, buoy);
      return false;
   }
   return true;
}

bool GridPoint::contains(int p1, int p2, int p3)
{
   return ptypes.contains(p1) && ptypes.contains(p2) && ptypes.contains(p3);
}

QString Attr_to_long(int attr)
{
   if ( attr == ATTR_S )
      return QString("Sedimentation Coefficient");
   else if ( attr == ATTR_K )
      return QString("Frictional Ratio");
   else if ( attr == ATTR_M )
      return QString("Molecular Weight");
   else if ( attr == ATTR_V )
      return QString("Partial Specific Volume");
   else if ( attr == ATTR_D )
      return QString("Diffusion Coefficient");
   else if ( attr == ATTR_F )
      return QString("Frictional Coefficient");
   else if ( attr == ATTR_SR )
      return QString("Sedimentation Coefficient (Real)");
   else if ( attr == ATTR_DR )
      return QString("Diffusion Coefficient (Real)");
   else
      return QString("");
}

QString Attr_to_title(int attr)
{
   if ( attr == ATTR_S )
      return QString("Sedimentation Coefficient (20,W) [ Sv ]");
   else if ( attr == ATTR_K )
      return QString("Frictional Ratio");
   else if ( attr == ATTR_M )
      return QString("Molecular Weight [ kDa ]");
   else if ( attr == ATTR_V )
      return QString("Partial Specific Volume [ mL / g ]");
   else if ( attr == ATTR_D )
      return QString("<p>Diffusion Coefficient (20,W) [ cm<sup>2</sup> s<sup>-1</sup> ]</p>");
   else if ( attr == ATTR_F )
      return QString("Frictional Coefficient [ g / s ]");
   else if ( attr == ATTR_SR )
      return QString("Sedimentation Coefficient (T,Buffer) [ Sv ]");
   else if ( attr == ATTR_DR )
      return QString("<p>Diffusion Coefficient (T,Buffer) [ cm<sup>2</sup> s<sup>-1</sup> ]</p>");
   else
      return QString("");
}

QString Attr_to_short(int attr)
{
   if ( attr == ATTR_S )
      return QString("s [ Sv ]");
   else if ( attr == ATTR_K )
      return QString("f / f0");
   else if ( attr == ATTR_M )
      return QString("MW [ kDa ]");
   else if ( attr == ATTR_V )
      return QString("vbar [ mL / g ]");
   else if ( attr == ATTR_D )
      // return QString("D [cm2 s−1]  (\(m^{2}/s\)");
      return QString("<p>D [ cm<sup>2</sup> s<sup>-1</sup> ]</p>");
   else if ( attr == ATTR_F )
      return QString("f [ g / s ]");
   else if ( attr == ATTR_SR )
      return QString("s* [ Sv ]");
   else if ( attr == ATTR_DR )
      return QString("<p>D* [ cm<sup>2</sup> s<sup>-1</sup> ]</p>");
   else
      return QString("");
}

QString Attr_to_char(int attr)
{
   if ( attr == ATTR_S )
      return QString("s");
   else if ( attr == ATTR_K )
      return QString("f/f0");
   else if ( attr == ATTR_M )
      return QString("MW");
   else if ( attr == ATTR_V )
      return QString("vbar");
   else if ( attr == ATTR_D )
      return QString("D");
   else if ( attr == ATTR_F )
      return QString("f");
   else if ( attr == ATTR_SR )
      return QString("s*");
   else if ( attr == ATTR_DR )
      return QString("D*");
   else
      return QString("");
}



