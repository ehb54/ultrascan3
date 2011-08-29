#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_screen.h"

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Screen::US_Hydrodyn_Saxs_Screen(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : QFrame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: SAXS Screen Control"));
   order_ascending = false;

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_screen_widget = true;

   best_fitness = 1e99;

   setupGUI();
   running = false;

   for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
   {
      qs.      push_back( saxs_window->plotted_q           [ i ] );
      Is.      push_back( saxs_window->plotted_I           [ i ] );
      I_errors.push_back( saxs_window->plotted_I_error     [ i ] );
      names.   push_back( saxs_window->qsl_plotted_iq_names[ i ] );
   }

   set_target();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   unsigned int csv_height = t_csv->rowHeight(0) + 30;
   unsigned int csv_width = t_csv->columnWidth(0) + 45;
   for ( int i = 0; i < t_csv->numRows(); i++ )
   {
      csv_height += t_csv->rowHeight(i);
   }
   for ( int i = 1; i < t_csv->numCols(); i++ )
   {
      csv_width += t_csv->columnWidth(i);
   }
   if ( csv_height > 700 )
   {
      csv_height = 700;
   }
   if ( csv_width > 1000 )
   {
      csv_width = 1000;
   }

   // cout << QString("csv size %1 %2\n").arg(csv_height).arg(csv_width);

   setGeometry(global_Xpos, global_Ypos, csv_width, 100 + csv_height );
}

US_Hydrodyn_Saxs_Screen::~US_Hydrodyn_Saxs_Screen()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_widget = false;
}

void US_Hydrodyn_Saxs_Screen::setupGUI()
{
   int minHeight1 = 30;
   int minHeight3 = 30;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new QTable(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // t_csv->setMinimumHeight(minHeight1 * 3);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.num_data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setItem( i, j, new QCheckTableItem( t_csv, "" ) );
            ((QCheckTableItem *)(t_csv->item( i, j )))->setChecked( csv1.data[i][j] == "Y" );
         } else {
            t_csv->setText( i, j, csv1.data[i][j] );
         }
      }
   }

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      t_csv->horizontalHeader()->setLabel(i, csv1.header[i]);
   }

   t_csv->setSorting(false);
   t_csv->setRowMovingEnabled(false);
   t_csv->setColumnMovingEnabled(false);
   t_csv->setReadOnly(false);

   t_csv->setColumnWidth(0, 200);
   t_csv->setColumnReadOnly(0, true);
   // t_csv->setColumnReadOnly(t_csv->numCols() - 1, true);

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   progress = new QProgressBar(this, "Progress");
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   progress->reset();

   cb_save_to_csv = new QCheckBox(this);
   cb_save_to_csv->setText(tr(" Results File:"));
   cb_save_to_csv->setChecked(false);
   cb_save_to_csv->setEnabled(false);
   cb_save_to_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_to_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_save_to_csv, SIGNAL(clicked()), SLOT(save_to_csv()));

   le_csv_filename = new QLineEdit(this, "csv_filename Line Edit");
   le_csv_filename->setText("search_results");
   le_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_filename->setMinimumWidth(150);
   le_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_replot_saxs = new QPushButton(tr("Replot stored I(q)"), this);
   pb_replot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_replot_saxs->setMinimumHeight(minHeight1);
   pb_replot_saxs->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_replot_saxs, SIGNAL(clicked()), SLOT(replot_saxs()));

   pb_save_saxs_plot = new QPushButton(tr("Store current I(q) plot"), this);
   pb_save_saxs_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_saxs_plot->setMinimumHeight(minHeight1);
   pb_save_saxs_plot->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_saxs_plot, SIGNAL(clicked()), SLOT(save_saxs_plot()));

   pb_set_target = new QPushButton(tr("Set Target"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   lbl_current_target = new QLabel("", this);
   lbl_current_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current_target->setMinimumHeight(minHeight1);
   lbl_current_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_current_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_start = new QPushButton(tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight(300);

   plot_dist = new QwtPlot(this);
   plot_dist->enableOutline(true);
   plot_dist->setOutlinePen(Qt::white);
   plot_dist->setOutlineStyle(Qwt::VLine);
   plot_dist->enableGridXMin();
   plot_dist->enableGridYMin();
   plot_dist->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
   plot_dist->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_dist->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
   plot_dist->setAxisTitle(QwtPlot::xBottom, tr("Radius (A)"));
   plot_dist->setAxisTitle(QwtPlot::yLeft, tr("Intensity"));
   plot_dist->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_dist->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_dist->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_dist->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   plot_dist->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_dist->setMargin(USglobal->config_list.margin);
   plot_dist->setTitle("");
   plot_dist->setCanvasBackground(USglobal->global_colors.plot);
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QHBoxLayout *hbl_csv = new QHBoxLayout(0);
   hbl_csv->addSpacing(4);
   hbl_csv->addWidget(cb_save_to_csv);
   hbl_csv->addSpacing(4);
   hbl_csv->addWidget(le_csv_filename);
   hbl_csv->addSpacing(4);

   QHBoxLayout *hbl_target = new QHBoxLayout(0);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_replot_saxs);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_save_saxs_plot);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_set_target);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(lbl_current_target);
   hbl_target->addSpacing(4);

   QHBoxLayout *hbl_controls = new QHBoxLayout(0);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);

   QHBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   QHBoxLayout *hbl_editor_plot = new QHBoxLayout(0);
   hbl_editor_plot->addLayout(vbl_editor_group);
   hbl_editor_plot->addWidget(plot_dist);

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addLayout(hbl_editor_plot);
   background->addSpacing(4);
   background->addWidget(progress);
   background->addSpacing(4);
   background->addLayout(hbl_csv);
   background->addSpacing(4);
   background->addLayout(hbl_target);
   background->addSpacing(4);
   background->addLayout(hbl_controls);
   background->addSpacing(4);
   background->addLayout(hbl_bottom);
   background->addSpacing(4);
}

void US_Hydrodyn_Saxs_Screen::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Screen::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_csv_viewer.html");
}

void US_Hydrodyn_Saxs_Screen::closeEvent(QCloseEvent *e)
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_screen_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_saxs_screen_csv = current_csv();

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Screen::table_value( int /* row */, int col )
{
   if ( col == 4 || col == 2 || col == 3 )
   {
      recompute_interval_from_points();
   }
   if ( col == 5 )
   {
      recompute_points_from_interval();
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Screen::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn_Saxs_Screen::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null, this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( IO_WriteOnly | IO_Translate) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Saxs_Screen::set_target()
{
   QString scaling_target = "";
   if ( *saxs_widget ) 
   { 
      saxs_window->set_scaling_target( scaling_target );
      if ( !scaling_target.isEmpty() )
      {
         saxs_window->ask_iq_target_grid();
      }
   }
   lbl_current_target->setText( scaling_target );
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::start()
{
   if ( !validate() ||
        !any_to_run() )
   {
      return;
   }
   running = true;
   update_enables();

   map < unsigned int, double >       starts;
   map < unsigned int, double >       ends;
   map < unsigned int, unsigned int > points;
   map < unsigned int, double >       increments;
   map < unsigned int, unsigned int > offsets;
   map < unsigned int, unsigned int > next_offsets;

   unsigned int current_offset = 0;

   csv_source_name_iqq.clear();
   saxs_q.clear();
   saxs_iqq.clear();

   double min_radius      = 0e0;
   double max_radius      = 0e0;
   double delta_radius    = 1e0;
   double min_delta_rho   = 0e0;
   double max_delta_rho   = 0e0;
   double delta_delta_rho = 1e0;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         starts    [i]   =  t_csv->text(i, 2).toDouble();
         ends      [i]   =  t_csv->text(i, 3).toDouble();
         points    [i]   =  t_csv->text(i, 4).toUInt();
         offsets   [i]   =  current_offset;
         current_offset +=  points[i];
         next_offsets[i] =  current_offset;

         if ( points[i] > 1 )
         {
            increments[i] = (ends[i] - starts[i]) / ( points[i] - 1 );
         } else {
            increments[i] = 0;
         }

         if ( t_csv->text( i, 0 ).contains("Radius (A)") )
         {
            min_radius   = starts[ i ];
            max_radius   = ends[ i ];
            delta_radius = increments[ i ];
            editor_msg("dark blue", QString("Radius range %1 to %2 step %3 (A)\n")
                       .arg(min_radius)
                       .arg(max_radius)
                       .arg(delta_radius)
                       );
         }
         if ( t_csv->text( i, 0 ).contains("Delta rho (A^-3)") )
         {
            min_delta_rho   = starts[ i ];
            max_delta_rho   = ends[ i ];
            delta_delta_rho = increments[ i ];
            editor_msg("dark blue", QString("Delta rho range %1 to %2 step %3 (A^-3)\n")
                       .arg(min_delta_rho)
                       .arg(max_delta_rho)
                       .arg(delta_delta_rho)
                       );
         }
      } else {
         if ( t_csv->text( i, 0 ).contains("Radius (A)") )
         {
            min_radius   = t_csv->text(i, 2).toDouble();
            max_radius   = min_radius;
            delta_radius = 1e0;
            editor_msg("dark red", QString("Using a fixed radius of %1 (A)\n").arg(min_radius));
         }
         if ( t_csv->text( i, 0 ).contains("Delta rho (A^-3)") )
         {
            min_delta_rho   = t_csv->text(i, 2).toDouble();
            max_delta_rho   = min_delta_rho;
            delta_delta_rho = 1e0;
            editor_msg("dark red", QString("Using a fixed delta rho of %1 (A^-3)\n").arg(min_delta_rho));
         }
      }         
   }
   
   vector < double > target_q;
   vector < double > target_I;
   vector < double > target_I_errors;

   // recover from target
   bool target_found = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      if ( names[i] == lbl_current_target->text() )
      {
         target_q        = qs[ i ];
         target_I        = Is[ i ];
         target_I_errors = I_errors[ i ];
         target_found = true;
         break;
      }         
   }

   if ( !target_found )
   {
      editor_msg("red", "could not find data for target!");
      running = false;
      update_enables();
      return;
   }

   unsigned int total_points = 1;
   for ( map < unsigned int, unsigned int >::iterator it = points.begin();
         it != points.end();
         it++ )
   {
      total_points *= it->second;
   }

   editor_msg("black", QString("Total spheres tested %1\n").arg(total_points));

   // linearization of an arbitrary number of loops
   US_Saxs_Util usu;

   vector < double >             by_radius;
   vector < double >             by_delta_rho;
   vector < double >             val_radius;
   vector < double >             val_delta_rho;
   map < double, unsigned int >  index_radius;
   map < double, unsigned int >  index_delta_rho;

   if ( !usu.iqq_sphere_fit(
                            "saxs_screen",
                            target_q,
                            target_I,
                            target_I_errors,
                            min_radius,
                            max_radius,
                            delta_radius,
                            min_delta_rho,
                            max_delta_rho,
                            delta_delta_rho,
                            (double) saxs_window->our_saxs_options->start_q,
                            (double) saxs_window->our_saxs_options->end_q,
                            by_radius,
                            by_delta_rho,
                            val_radius,
                            val_delta_rho,
                            index_radius,
                            index_delta_rho,
                            true // do_normalize = true
                            ) )
   {
      editor_msg("red", usu.errormsg );
      running = false;
      update_enables();
      return;
   }

   editor_msg("blue", usu.noticemsg );
   
   // then plot, summarize results
   
   // if ( running && cb_save_to_csv->isChecked() )
   // {
   // save_csv_saxs_iqq();
   // }


   vector < double > radii;
   vector < double > intensity;

   for ( unsigned int i = 0; i < by_radius.size(); i++ )
   {
      radii.push_back( val_radius[ i ] );
      intensity.push_back( by_radius[ i ] );
   }

   plot_dist->clear();
   long curvekey = plot_dist->insertCurve("Radii histogram");
   plot_dist->setCurveStyle( curvekey, QwtCurve::Sticks);
   plot_dist->setCurveData ( curvekey, 
                             (double *)&( radii    [ 0 ] ),
                             (double *)&( intensity[ 0 ] ), 
                             radii.size() );
   plot_dist->setCurvePen  ( curvekey, QPen( "yellow", 2, SolidLine ) );
   plot_dist->replot();

   progress->setProgress(1, 1);
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::stop()
{
   running = false;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::update_enables()
{
   // cout << "US_Hydrodyn_Saxs_Screen::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());
   bool any_selected  = false;
   if ( !running )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
      {
         if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
         {
            any_selected = true;
         }
      }
   }
   pb_start            ->setEnabled( !running && any_selected && !lbl_current_target->text().isEmpty() );
   pb_stop             ->setEnabled( running );
   pb_replot_saxs      ->setEnabled( !running && *saxs_widget && names.size() );
   pb_save_saxs_plot   ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   pb_set_target       ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   cb_save_to_csv      ->setEnabled( !running );
   le_csv_filename     ->setEnabled( !running && cb_save_to_csv->isChecked() );
}

bool US_Hydrodyn_Saxs_Screen::any_to_run()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((QCheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         return true;
      }
   }
   editor_msg("dark red", "Nothing to do: At least one of the rows must have Active set to Y");
   return false;
}

void US_Hydrodyn_Saxs_Screen::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Screen::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(qcolor);
   editor->append(msg);
   editor->setColor(save_color);
}

bool US_Hydrodyn_Saxs_Screen::validate()
{
   bool errors = false;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( t_csv->text( i, 2 ).toDouble() > t_csv->text( i, 3 ).toDouble() )
      {
         editor_msg("red", QString("Row %1 column \"Low value\" can not be greater than \"High value\"\n").arg(i));
         errors = true;
      }
      if ( t_csv->text( i, 4 ).toInt() < 1 )
      {
         editor_msg("red", QString("Row %1 column \"Points\" must be greater or equal to one\n").arg(i));
         errors = true;
      }
      if ( t_csv->text( i, 4 ).toDouble() == 1  &&
           t_csv->text( i, 2 ).toDouble() > t_csv->text( i, 3 ).toDouble() )
      {
         editor_msg("red", QString("Row %1 one \"Points\" requires \"Low value\" equals \"High value\"\n").arg(i));
         errors = true;
      }
   }
   errors |= !validate_saxs_window();
   return !errors;
}

bool US_Hydrodyn_Saxs_Screen::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      if ( !*saxs_widget )
      {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   return true;
}

bool US_Hydrodyn_Saxs_Screen::validate_saxs_window()
{
   if ( !activate_saxs_window() )
   {
      return false;
   }

   if ( !lbl_current_target->text().isEmpty() &&
        !saxs_window->qsl_plotted_iq_names.grep(QRegExp(QString("^%1$").arg(lbl_current_target->text()))).size() )
   {
      // need to replot
      editor_msg("dark red", "Current target missing from SAXS window plot, replotting");
      do_replot_saxs();
   }

   return true;
}

void US_Hydrodyn_Saxs_Screen::save_to_csv()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::do_replot_saxs()
{
   saxs_window->clear_plot_saxs(true);
   saxs_window->plotted = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      saxs_window->plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   if ( saxs_window->plotted )
   {
      saxs_window->editor->setParagraphBackgroundColor ( saxs_window->editor->paragraphs() - 1, QColor("white") );
      saxs_window->editor->append("I(q) plot done\n");
      saxs_window->plotted = false;
   }
   saxs_window->rescale_plot();
}

void US_Hydrodyn_Saxs_Screen::replot_saxs()
{
   if ( activate_saxs_window() )
   {
      do_replot_saxs();
   }
}

void US_Hydrodyn_Saxs_Screen::save_saxs_plot()
{
   if ( !activate_saxs_window() )
   {
      return;
   }

   if ( !saxs_window->plotted_q.size() )
   {
      QMessageBox::information(this, 
                               tr("US-SOMO: I(q) search"),
                               QString(tr("Nothing plotted")));
      return;
   }

   bool add_target = false;
   if ( !lbl_current_target->text().isEmpty() )
   {
      bool current_target_in_plot = false;
      for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
      {
         if ( saxs_window->qsl_plotted_iq_names[ i ] == lbl_current_target->text() )
         {
            current_target_in_plot = true;
            break;
         }
      }
      if ( !current_target_in_plot )
      {
         switch( QMessageBox::information( this, 
                                           "US_SOMO: I(q) screen",
                                           tr("The target is not currently plotted"),
                                           tr("Clear target and continue"), 
                                           tr("Add target to stored plots"), 
                                           tr("Cancel"),
                                           0,      // Enter == button 0
                                           2 ) ) { // Escape == button 2
         case 0:
            lbl_current_target->setText("");
            break;
         case 1: 
            add_target = true;
            break;
         case 2: // Cancel clicked or Escape pressed
            return;
            break;
         }
      }
   }    
         
   vector < double > add_q;
   vector < double > add_I;
   vector < double > add_I_error;
   if ( add_target )
   {
      add_target = false;
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( names[i] == lbl_current_target->text() )
         {
            add_q       = qs[ i ];
            add_I       = Is[ i ];
            add_I_error = I_errors[ i ];
            add_target = true;
            break;
         }         
      }
      if ( !add_target )
      {
         QMessageBox::warning(this, 
                              "US_SOMO: I(q) screen",
                              tr("Target not found, clearing"));
         lbl_current_target->setText("");
      }
   }

   qs      .clear();
   Is      .clear();
   I_errors.clear();
   names   .clear();

   if ( add_target )
   {
      qs.      push_back( add_q );
      Is.      push_back( add_I );
      I_errors.push_back( add_I_error );
      names.   push_back( lbl_current_target->text() );
   }

   for ( unsigned int i = 0; i < saxs_window->plotted_I_error.size(); i++ )
   {
      qs.      push_back( saxs_window->plotted_q           [ i ] );
      Is.      push_back( saxs_window->plotted_I           [ i ] );
      I_errors.push_back( saxs_window->plotted_I_error     [ i ] );
      names.   push_back( saxs_window->qsl_plotted_iq_names[ i ] );
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Screen::save_csv_saxs_iqq()
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      le_csv_filename->text() + "_iqq_search.csv";

   if ( QFile::exists(fname) )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
   }         

   FILE *of = fopen(fname, "wb");
   if ( of )
   {
      //  header: "name","type",q1,q2,...,qn, header info
      fprintf(of, "\"Name\",\"Type; q:\",%s,\"%s\"\n", 
              saxs_window->vector_double_to_csv(saxs_q).ascii(),
              saxs_header_iqq.remove("\n").ascii());
      for ( unsigned int i = 0; i < csv_source_name_iqq.size(); i++ )
      {
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 csv_source_name_iqq[i].ascii(),
                 "I(q)",
                 saxs_window->vector_double_to_csv(saxs_iqq[i]).ascii());
      }
      fprintf(of, "\n");

      fclose(of);
      editor->append(tr("Created file: " + fname + "\n"));
   } else {
      editor_msg("red",tr("ERROR creating file: " + fname + "\n"));
   }
}

csv US_Hydrodyn_Saxs_Screen::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((QCheckTableItem *)(t_csv->item( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->text( i, j );
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Saxs_Screen::recompute_interval_from_points()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 5, 
                     ( 
                      t_csv->text(i, 4).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( ( t_csv->text(i, 3).toDouble() -
                              t_csv->text(i, 2).toDouble() )
                             / ( t_csv->text(i, 4).toDouble() - 1e0 ) ) 
                      )
                     );
   }
}

void US_Hydrodyn_Saxs_Screen::recompute_points_from_interval()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      t_csv->setText(
                     i, 4, 
                     ( 
                      t_csv->text(i, 5).toDouble() == 0e0 ?
                      ""
                      :
                      QString("%1")
                      .arg( 1 + (unsigned int)(( t_csv->text(i, 3).toDouble() -
                                                 t_csv->text(i, 2).toDouble() )
                                               / t_csv->text(i, 5).toDouble() + 0.5) ) 
                      )
                     );
   }
}
