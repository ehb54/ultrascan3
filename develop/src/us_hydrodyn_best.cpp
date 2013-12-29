#include "../include/us_hydrodyn_best.h"

#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  define isnan _isnan
#endif

US_Hydrodyn_Best::US_Hydrodyn_Best(
                                   void                     *              us_hydrodyn,
                                   bool                     *              best_widget,
                                   QWidget *                               p,
                                   const char *                            name
                                   ) : QFrame( p, name )
{
   this->us_hydrodyn                          = us_hydrodyn;
   this->best_widget                          = best_widget;
   if ( !best_widget )
   {
      return;
   }
   *best_widget = true;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( tr( "US-SOMO: BEST results analysis tool" ) );

   cg_red = USglobal->global_colors.cg_label;
   cg_red.setBrush( QColorGroup::Foreground, QBrush( QColor( "red" ),  QBrush::SolidPattern ) );

   plot_data_zoomer      = (ScrollZoomer *) 0;

   tau_inputs
      << "EIGENVALUES OF Drr TENSOR (1/s) [1]"
      << "EIGENVALUES OF Drr TENSOR (1/s) [2]"
      << "EIGENVALUES OF Drr TENSOR (1/s) [3]"
      ;

   for ( int i = 0; i < (int) tau_inputs.size(); ++i )
   {
      tau_input_set.insert( tau_inputs[ i ] );
   }

   tau_msg
      << "Tau (1) (ns)"
      << "Tau (2) (ns)"
      << "Tau (3) (ns)"
      << "Tau (4) (ns)"
      << "Tau (5) (ns)"
      << "Tau (h) (ns)"
      << "Tau (m) (ns)"
      ;

   setupGUI();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Best::~US_Hydrodyn_Best()
{
}

void US_Hydrodyn_Best::setupGUI()
{
   int minHeight1 = 24;
   int minHeight3 = 25;

   // ------ input section 
   lbl_input = new mQLabel("Data fields", this);
   lbl_input->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_input->setMinimumHeight(minHeight1);
   lbl_input->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_input->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_input, SIGNAL( pressed() ), SLOT( hide_input() ) );

   lb_data = new QListBox( this );
   lb_data->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_data->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lb_data->setEnabled(true);
   connect( lb_data, SIGNAL( selectionChanged() ), SLOT( data_selected() ) );

   input_widgets.push_back( lb_data );

   pb_load =  new QPushButton ( tr( "Load CSV" ), this );
   pb_load -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_load -> setMinimumHeight( minHeight1 );
   pb_load -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_load, SIGNAL( clicked() ), SLOT( load() ) );

   pb_save_results =  new QPushButton ( tr( "Save Results" ), this );
   pb_save_results -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_save_results -> setMinimumHeight( minHeight1 );
   pb_save_results -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   pb_save_results -> setEnabled      ( false );
   connect( pb_save_results, SIGNAL( clicked() ), SLOT( save_results() ) );

   input_widgets.push_back( pb_save_results );

   // ------ editor section

   lbl_editor = new mQLabel("Messages", this);
   lbl_editor->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_editor->setMinimumHeight(minHeight1);
   lbl_editor->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   connect( lbl_editor, SIGNAL( pressed() ), SLOT( hide_editor() ) );

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   editor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2 ));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   editor_widgets.push_back( frame );

   mb_editor = new QMenuBar(frame, "menu" );
   mb_editor->setMinimumHeight(minHeight1 - 5);
   mb_editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   QPopupMenu * file = new QPopupMenu(editor);
   mb_editor->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);
   editor->setMinimumHeight( minHeight1 * 3 );

   editor_widgets.push_back( editor );

   // ------ plot section
   plot_data = new QwtPlot(this);
#ifndef QT4
   // plot_data->enableOutline(true);
   // plot_data->setOutlinePen(Qt::white);
   // plot_data->setOutlineStyle(Qwt::VLine);
   plot_data->enableGridXMin();
   plot_data->enableGridYMin();
#else
   grid_saxs = new QwtPlotGrid;
   grid_saxs->enableXMin( true );
   grid_saxs->enableYMin( true );
#endif
   plot_data->setPalette( QPalette(USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot, USglobal->global_colors.cg_plot));
#ifndef QT4
   plot_data->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
   plot_data->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
   grid_saxs->setMajPen( QPen( USglobal->global_colors.major_ticks, 0, Qt::DotLine ) );
   grid_saxs->setMinPen( QPen( USglobal->global_colors.minor_ticks, 0, Qt::DotLine ) );
   grid_saxs->attach( plot_data );
#endif
   plot_data->setAxisTitle(QwtPlot::xBottom, tr( "1/Triangles"      ) ); 
   plot_data->setAxisTitle(QwtPlot::yLeft,   tr( "Parameter [a.u.]" ) );
#ifndef QT4
   plot_data->setTitleFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3, QFont::Bold));
   plot_data->setAxisTitleFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yLeft, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_data->setAxisTitleFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::xBottom, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
#ifndef QT4
   plot_data->setAxisTitleFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
#endif
   plot_data->setAxisFont(QwtPlot::yRight, QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   plot_data->setMargin(USglobal->config_list.margin);
   plot_data->setTitle("");
#ifndef QT4
   plot_data->setAxisOptions(QwtPlot::yLeft, QwtAutoScale::None);
#else
   plot_data->setAxisScaleEngine(QwtPlot::yLeft, new QwtScaleEngine );
#endif
   plot_data->setCanvasBackground(USglobal->global_colors.plot);

   lbl_points = new mQLabel( "Linear:", this );
   lbl_points->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_points->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_points->show();
   connect( lbl_points, SIGNAL( pressed() ), SLOT( toggle_points() ) );

   lbl_points_ln = new mQLabel( "Log:   ", this );
   lbl_points_ln->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_points_ln->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_points_ln->show();
   connect( lbl_points_ln, SIGNAL( pressed() ), SLOT( toggle_points_ln() ) );

   // connect( plot_data_zoomer, SIGNAL( zoomed( const QwtDoubleRect & ) ), SLOT( plot_data_zoomed( const QwtDoubleRect & ) ) );

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_close =  new QPushButton ( tr( "Close" ), this );
   pb_close -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_close -> setMinimumHeight( minHeight1 );
   pb_close -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_close, SIGNAL( clicked() ), SLOT( cancel() ) );

   // -------- build layout

   QVBoxLayout *background = new QVBoxLayout( this );
   QHBoxLayout *top        = new QHBoxLayout( 0 );

   // ----- left side
   {
      QBoxLayout *bl = new QVBoxLayout( 0 );
      bl->addWidget( lbl_input );
      bl->addWidget( lb_data );
      {
         QHBoxLayout *hbl = new QHBoxLayout( 0 );
         hbl->addWidget( pb_load );
         hbl->addWidget( pb_save_results );
         bl->addLayout( hbl );
      }

      bl->addWidget( lbl_editor );
      bl->addWidget( frame );
      bl->addWidget( editor );
      
      top->addLayout( bl );
   }

   // ----- right side
   {
      QBoxLayout *bl = new QVBoxLayout( 0 );
      bl->addWidget( plot_data );
      // needs layout for dynamic cb_'s 
      hbl_points = new QHBoxLayout( 0 );
      hbl_points->addWidget( lbl_points );
      bl->addLayout( hbl_points );
      hbl_points_ln = new QHBoxLayout( 0 );
      hbl_points_ln->addWidget( lbl_points_ln );
      bl->addLayout( hbl_points_ln );
      top->addLayout( bl );
   }

   background->addLayout( top );
   background->addSpacing( 4 );

   {
      QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
      hbl_bottom->addSpacing( 4 );
      hbl_bottom->addWidget ( pb_help );
      hbl_bottom->addSpacing( 4 );
      hbl_bottom->addWidget ( pb_close );
      hbl_bottom->addSpacing( 4 );
      background->addLayout ( hbl_bottom );
   }

}

void US_Hydrodyn_Best::cancel()
{
   close();
}

void US_Hydrodyn_Best::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/best.html");
}

void US_Hydrodyn_Best::closeEvent( QCloseEvent *e )
{
   *best_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Best::hide_input()
{
   hide_widgets( input_widgets, input_widgets[ 0 ]->isVisible() );
}

void US_Hydrodyn_Best::hide_editor()
{
   hide_widgets( editor_widgets, editor_widgets[ 0 ]->isVisible() );
   if ( editor_widgets[ 0 ]->isVisible() )
   {
      cout << "resetting editor palette\n";
      lbl_editor->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }
}

void US_Hydrodyn_Best::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);

   if ( !editor_widgets[ 0 ]->isVisible() && color == "red" && !msg.stripWhiteSpace().isEmpty() )
   {
      lbl_editor->setPalette(QPalette(cg_red, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   }
}

void US_Hydrodyn_Best::hide_widgets( vector < QWidget *> widgets, bool hide )
{
   for ( unsigned int i = 0; i < ( unsigned int ) widgets.size(); i++ )
   {
      hide ? widgets[ i ]->hide() : widgets[ i ]->show();
   }
}

void US_Hydrodyn_Best::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Best::update_font()
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

void US_Hydrodyn_Best::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
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

void US_Hydrodyn_Best::clear()
{
   lb_data  ->clear();
   plot_data->clear();
   if ( plot_data_zoomer )
   {
      plot_data_zoomer->zoom ( 0 );
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *) 0;
   }

   points             = 0;
   one_over_triangles .clear();
   parameter_data     .clear();
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      hbl_points->remove( cb_points[ i ] );
      delete cb_points[ i ];
   }
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      hbl_points_ln->remove( cb_points_ln[ i ] );
      delete cb_points_ln[ i ];
   }
   cb_points             .clear();
   cb_points_ln          .clear();
   cb_checked            .clear();
   cb_checked_ln         .clear();
   loaded_csv_trimmed    .clear();
   loaded_csv_filename    = "";
   last_pts_removed       = "";
   last_lin_extrapolation.clear();
   last_log_extrapolation.clear();
   tau_csv_addendum_tag  .clear();
   tau_csv_addendum_val  .clear();
}      

void US_Hydrodyn_Best::load()
{
   // open file, clear lb_data, clear plot, reload lb_data
   pb_load->setEnabled( false );
   QString use_dir = 
      USglobal->config_list.root_dir + 
      QDir::separator() + "somo" + 
      QDir::separator() + "cluster" +
      QDir::separator() + "results"
      ;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName(use_dir, "*.csv *.CSV", this,
                                                   "open file",
                                                   caption() + tr( " Load CSV from BEST results" )
                                                   );

   if ( filename.isEmpty() )
   {
      pb_load->setEnabled( true );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   clear();

   QFile f( filename );
   if ( !f.open( IO_ReadOnly ) )
   {
      editor_msg( "red", QString( tr( "Error opening file %1 (check permissions)" ) ).arg( f.name() ) );
      pb_load->setEnabled( true );
      return;
   }

   QTextStream ts( &f );
   if ( ts.atEnd() )
   {
      f.close();
      editor_msg( "red", QString( tr( "Error on file %1 no data" ) ).arg( f.name() ) );
      pb_load->setEnabled( true );
      return;
   }
      

   {
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).gres( "\"", "" );
      
      if ( qsl.size() < 8 )
      {
         f.close();
         editor_msg( "red", QString( tr( "Error on file %1 insufficient data or no triangles?" ) ).arg( f.name() ) );
         pb_load->setEnabled( true );
         return;
      }
      
      points = (int) qsl.size() - 8;

      // qDebug( QString( "points read %1 qsl size %2" ).arg( points ).arg( qsl.size() ) );
      // qDebug( QString( "qsl points + 0 %1" ).arg( qsl[ points + 0 ] ) );
      // qDebug( QString( "qsl points + 1 %1" ).arg( qsl[ points + 1 ] ) );
      // qDebug( QString( "qsl points + 2 %1" ).arg( qsl[ points + 2 ] ) );
      // qDebug( QString( "qsl points + 3 %1" ).arg( qsl[ points + 3 ] ) );

      if ( qsl[ points + 1 ] != "Extrapolation to zero triangles (a)" )
      {
         f.close();
         editor_msg( "red", QString( tr( "Error on file %1 improper format on line 1" ) ).arg( f.name() ) );
         pb_load->setEnabled( true );
         return;
      }
   }         


   loaded_csv_trimmed << ts.readLine(); // triangles used
   {
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).gres( "\"", "" );

      // qDebug( "1/triangle line:" + qsl.join( ":" ) + QString( "size %1" ).arg( qsl.size() ) );
      if ( (int) qsl.size() < 1 + points || qsl[ 0 ] != "1/Triangles used" )
      {
         f.close();
         editor_msg( "red", QString( tr( "Error on file %1 improper format on line 3" ) ).arg( f.name() ) );
         pb_load->setEnabled( true );
         return;
      }

      for ( int i = 1; i <= points; ++i )
      {
         if ( qsl[ i ] == "=-1" )
         {
            f.close();
            editor_msg( "red", QString( tr( "Error on file %1 line 3: 1/triangles incorrect" ) ).arg( f.name() ) );
            pb_load->setEnabled( true );
            return;
         }
         one_over_triangles.push_back( QString( qsl[ i ] ).replace( QRegExp( "^=" ), "" ).toDouble() );
      }
   }

   int line = 3;
   disconnect( lb_data, SIGNAL( selectionChanged() ), 0, 0 );
   while ( !ts.atEnd() )
   {
      ++line;
      loaded_csv_trimmed << ts.readLine();
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed.back() ).gres( "\"", "" );
      // qDebug( "next" + qsl.join( ":" ) );
      if ( (int) qsl.size() < 1 + points )
      {
         f.close();
         editor_msg( "red", QString( tr( "Error on file %1 improper format on line %2" ) ).arg( f.name() ).arg( line ) );
         pb_load->setEnabled( true );
         connect( lb_data, SIGNAL( selectionChanged() ), SLOT( data_selected() ) );
         return;
      }
      // qDebug( QString( "qsl 0: %1 : size %2" ).arg( qsl[ 0 ] ).arg( qsl.size() ) );
      if ( (int) qsl.size() >= points + 7 )
      {
         // have a linear fittable line
         loaded_csv_trimmed.pop_back();
         QString qs;
         qs += "\"" + qsl[ 0 ] + "\",";
         
         for ( int i = 1; i <= points; ++i )
         {
            qs += QString( "%1," ).arg( qsl[ i ] );
            parameter_data[ qsl[ 0 ] ].push_back( QString( qsl[ i ] ).replace( QRegExp( "^=" ), "" ).toDouble() );
            if ( qsl[ i ] == "?" )
            {
               f.close();
               editor_msg( "red", QString( tr( "Error on file %1 line %2 - missing data for column %3" ) )
                           .arg( f.name() )
                           .arg( line ) 
                           .arg( i + 1 ) 
                           );
               pb_load->setEnabled( true );
               connect( lb_data, SIGNAL( selectionChanged() ), SLOT( data_selected() ) );
               return;
            }
         }
         loaded_csv_trimmed << qs;
         lb_data->insertItem( qsl[ 0 ] );
      }
   }
   editor_msg( "blue", QString( "%1 data columns found" ).arg( points ) );
   for ( int i = 0; i < points; ++i )
   {
      QCheckBox * cb = new QCheckBox( this );
      cb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      cb->setText( QString( "%1" ).arg( i + 1 ) );
      cb->setChecked( true );
      cb->setEnabled( true );
      cb->show();
      connect( cb, SIGNAL( clicked() ), SLOT( cb_changed() ) );
      cb_points.push_back( cb );
      hbl_points->addWidget( cb );
   }
   for ( int i = 0; i < points; ++i )
   {
      QCheckBox * cb = new QCheckBox( this );
      cb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      cb->setText( QString( "%1" ).arg( i + 1 ) );
      cb->setChecked( false );
      cb->setEnabled( true );
      cb->show();
      connect( cb, SIGNAL( clicked() ), SLOT( cb_changed_ln() ) );
      cb_points_ln.push_back( cb );
      hbl_points_ln->addWidget( cb );
   }
   
   f.close();
   loaded_csv_filename = f.name();
   for ( int i = 0; i < (int) lb_data->count(); ++i )
   {
      lb_data->setSelected( i, true );
      data_selected( false );
   }
   lb_data->setSelected( 0, true );
   recompute_tau();
   connect( lb_data, SIGNAL( selectionChanged() ), SLOT( data_selected() ) );
   cb_changed   ( false );
   cb_changed_ln( true  );
   pb_load->setEnabled( true );
   pb_save_results->setEnabled( true );
}

void US_Hydrodyn_Best::cb_changed( bool do_data )
{
   // qDebug( "cb_changed" );
   QString text = lb_data->selectedItem()->text();
   cb_checked[ text ].clear();
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      if ( cb_points[ i ]->isChecked() )
      {
         cb_checked[ text ].insert( i );
      }
   }
   if ( do_data )
   {
      data_selected();
   }
}

void US_Hydrodyn_Best::cb_changed_ln( bool do_data )
{
   // qDebug( "cb_changed_ln" );
   QString text = lb_data->selectedItem()->text();
   cb_checked_ln[ text ].clear();
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isChecked() )
      {
         cb_checked_ln[ text ].insert( i );
      }
   }
   if ( do_data )
   {
      data_selected();
   }
}
   
void US_Hydrodyn_Best::data_selected( bool do_recompute_tau )
{
   // qDebug( "data_selected" );
   QString text = lb_data->selectedItem()->text();
   // qDebug( QString( "selected %1 map %2" ).arg( text ).arg( parameter_data[ text ].size() ) );
   plot_data->clear();
   if ( !cb_checked.count( text ) )
   {
      // set all on
      for ( int i = 0; i < (int) cb_points.size(); ++i )
      {
         cb_checked[ text ].insert( i );
      }
   }
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      cb_points[ i ]->setChecked( cb_checked[ text ].count( i ) );
   }

   // if ( !cb_checked_ln.count( text ) )
   // {
   //    // set all off
   //    for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   //    {
   //       if ( parameter_data[ text ][ i ] > 0e0 )
   //       {
   //          cb_checked_ln[ text ].insert( i );
   //       }
   //    }
   // }

   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      cb_points_ln[ i ]->setChecked( cb_checked_ln[ text ].count( i ) );
      cb_points_ln[ i ]->setEnabled( parameter_data[ text ][ i ] > 0e0 );
   }

   vector < double > use_one_over_triangles;
   vector < double > use_parameter_data;
   vector < double > skip_one_over_triangles;
   vector < double > skip_parameter_data;
   vector < double > use_one_over_triangles_ln;
   vector < double > use_parameter_data_ln;
   vector < double > skip_one_over_triangles_ln;
   vector < double > skip_parameter_data_ln;

   last_pts_removed    = "";
   last_pts_removed_ln = "";

   set < int > selected_points;
   set < int > selected_points_ln;

   //for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
   for ( int i = (int) one_over_triangles.size() - 1; i >= 0; --i )
   {
      if ( cb_points[ points - i - 1 ]->isChecked() )
      {
         use_one_over_triangles .push_back( one_over_triangles[ i ] );
         use_parameter_data     .push_back( parameter_data[ text ][ i ] );
         selected_points        .insert( i );
      } else {
         skip_one_over_triangles.push_back( one_over_triangles[ i ] );
         skip_parameter_data    .push_back( parameter_data[ text ][ i ] );
         last_pts_removed += QString( "%1 " ).arg( points - i );
      }  
      if ( cb_points_ln[ points - i - 1 ]->isChecked() &&
           parameter_data[ text ][ i ] > 0e0 )
      {
         use_one_over_triangles_ln .push_back( one_over_triangles[ i ] );
         use_parameter_data_ln     .push_back( parameter_data[ text ][ i ] );
         selected_points_ln        .insert( i );
      } else {
         skip_one_over_triangles_ln.push_back( one_over_triangles[ i ] );
         skip_parameter_data_ln    .push_back( parameter_data[ text ][ i ] );
         last_pts_removed_ln += QString( "%1 " ).arg( points - i );
      }  
   }

   for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
   {
#ifndef QT4
      long curve = plot_data->insertCurve( "plot" );
      plot_data->setCurveStyle( curve, QwtCurve::Dots );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "plot" );
      curve->setStyle( QwtPlotCurve::Dots );
#endif

      QwtSymbol sym;
      if ( selected_points   .count( i ) &&
           selected_points_ln.count( i ) )
      {
         sym.setStyle(QwtSymbol::Diamond);
         sym.setSize( 12 );
         sym.setPen  ( Qt::cyan );
         sym.setBrush( Qt::blue );
      }
      if (  selected_points   .count( i ) &&
           !selected_points_ln.count( i ) )
      {
         sym.setStyle(QwtSymbol::UTriangle);
         sym.setSize( 10 );
         sym.setPen  ( Qt::cyan );
         sym.setBrush( Qt::blue );
      }
      if ( !selected_points   .count( i ) &&
            selected_points_ln.count( i ) )
      {
         sym.setStyle(QwtSymbol::DTriangle);
         sym.setSize( 10 );
         sym.setPen  ( Qt::cyan );
         sym.setBrush( Qt::blue );
      }
      if ( !selected_points   .count( i ) &&
           !selected_points_ln.count( i ) )
      {
         sym.setStyle(QwtSymbol::XCross);
         sym.setSize( 10 );
         sym.setPen  ( Qt::red );
         sym.setBrush( Qt::red );
      }

#ifndef QT4
      plot_data->setCurveData( curve, 
                               (double *)&( one_over_triangles[ i ] ),
                               (double *)&( parameter_data[ text ][ i ] ),
                               1
                               );
      plot_data->setCurveStyle( curve, QwtCurve::Lines);
      plot_data->setCurveSymbol( curve, sym );
#else
      curve->setData(
                     (double *)&( one_over_triangles[ i ] ),
                     (double *)&( parameter_data[ text ][ i ] ),
                     1
                     );

      // curve->setPen( QPen( Qt::red, 2, Qt::SolidLine ) );
      curve->attach( plot_data );
      curve->setStyle( QwtPlotCurve::Lines );
      curve->setSymbol( sym );
#endif
   }      
   

   double miny = 0e0;
   double maxy = 0e0;

   if ( parameter_data[ text ].size() )
   {
      miny = parameter_data[ text ][ 0 ];
      maxy = parameter_data[ text ][ 0 ];
   }

   for ( int i = 0; i < (int) parameter_data[ text ].size(); ++i )
   {
      if ( miny > parameter_data[ text ][ i ] )
      {
         miny = parameter_data[ text ][ i ];
      }
      if ( maxy < parameter_data[ text ][ i ] )
      {
         maxy = parameter_data[ text ][ i ];
      }
   }

   // run LR
   last_lin_extrapolation.erase( text );


   {
      last_a    = 0e0;
      last_siga = 0e0;
      last_b    = 0e0;
      last_sigb = 0e0;
      last_chi2 = 0e0;

      if ( use_one_over_triangles.size() > 1 )   
      {
         double a;
         double b;
         double siga;
         double sigb;
         double chi2;


         US_Saxs_Util::linear_fit( use_one_over_triangles,
                                   use_parameter_data,
                                   a,
                                   b,
                                   siga,
                                   sigb,
                                   chi2 );


         last_a    = a;
         last_siga = siga;
         last_b    = b;
         last_sigb = sigb;
         last_chi2 = chi2;

         last_lin_extrapolation[ text ].push_back( last_a );
         last_lin_extrapolation[ text ].push_back( last_siga );
         last_lin_extrapolation[ text ].push_back( last_b );
         last_lin_extrapolation[ text ].push_back( last_sigb );
         last_lin_extrapolation[ text ].push_back( last_chi2 );

         double x[ 2 ];
         double y[ 2 ];

         editor_msg( "blue", 
                     QString( tr( "%1: 0 triangle extrapolation=%2 sigma=%3 sigma %=%4 slope=%5 sigma=%6 sigma %=%7 chi^2=%8" ) )
                     .arg( text )
                     .arg( a,    0, 'g', 8 )
                     .arg( siga, 0, 'g', 8 )
                     .arg( a != 0 ? fabs( 100.0 * siga / a ) : (double) 0, 0, 'g', 8 )
                     .arg( b,    0, 'g', 8 )
                     .arg( sigb, 0, 'g', 8 )
                     .arg( b != 0 ? fabs( 100.0 * sigb / b ) : (double) 0, 0, 'g', 8 )
                     .arg( chi2, 0, 'g', 8 )
                     );

         x[ 0 ] = 0e0;
         x[ 1 ] = one_over_triangles[ 0 ] * 1.1;
         y[ 0 ] = a;
         y[ 1 ] = a + x[ 1 ] * b;

#ifndef QT4
         long curve = plot_data->insertCurve( "plot lr" );
         plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "plot lr" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_data->setCurveData( curve, 
                                  (double *)&( x[ 0 ] ),
                                  (double *)&( y[ 0 ] ),
                                  2
                                  );
         plot_data->setCurvePen( curve, QPen( Qt::green, 2, SolidLine));

#else
         curve->setData(
                        (double *)&( x[ 0 ] ),
                        (double *)&( y[ 0 ] ),
                        2
                        );

         curve->setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
         curve->attach( plot_data );
#endif

         double min = y[ 0 ] < y[ 1 ] ? y[ 0 ] : y[ 1 ];
         double max = y[ 0 ] < y[ 1 ] ? y[ 1 ] : y[ 0 ];
         if ( miny > min )
         {
            miny = min;
         }
         if ( maxy < max )
         {
            maxy = max;
         }
      }
   }

   // run LR ln

   last_log_extrapolation.erase( text );

   {
      last_a_ln    = 0e0;
      last_siga_ln = 0e0;
      last_b_ln    = 0e0;
      last_sigb_ln = 0e0;
      last_chi2_ln = 0e0;

      ln_plot_ok = false;
      if ( use_one_over_triangles_ln.size() > 1 )   
      {
         double a;
         double b;
         double siga;
         double sigb;
         double chi2;

         vector < double > fit_x;
         vector < double > fit_y;
         for ( int i = 0; i < (int) use_one_over_triangles_ln.size(); ++i )
         {
            fit_x.push_back( use_one_over_triangles_ln[ i ] * log( use_one_over_triangles_ln[ i ] ) );
            fit_y.push_back( log( use_parameter_data_ln    [ i ] ) );
         }
            
         US_Saxs_Util::linear_fit( fit_x,
                                   fit_y,
                                   a,
                                   b,
                                   siga,
                                   sigb,
                                   chi2 );

         ln_plot_ok = true;

         last_a_ln    = a;
         last_siga_ln = siga;
         last_b_ln    = b;
         last_sigb_ln = sigb;
         last_chi2_ln = chi2;

         last_log_extrapolation[ text ].push_back( last_a_ln );
         last_log_extrapolation[ text ].push_back( last_siga_ln );
         last_log_extrapolation[ text ].push_back( last_b_ln );
         last_log_extrapolation[ text ].push_back( last_sigb_ln );
         last_log_extrapolation[ text ].push_back( last_chi2_ln );

#define UHB_PTS  200
#define UHB_MINX 1e-20

         double x[ UHB_PTS ];
         double y[ UHB_PTS ];

         editor_msg( "dark blue", 
                     QString( tr( "%1: 0 triangle LN extrapolation=%2 sigma=%3 sigma %=%4 slope=%5 sigma=%6 sigma %=%7 chi^2=%8" ) )
                     .arg( text )
                     .arg( exp( a ),    0, 'g', 8 )
                     .arg( exp( a ) * siga, 0, 'g', 8 )
                     .arg( exp( a ) != 0 ? fabs( 100.0 * exp( a ) * siga / exp( a ) ) : (double) 0, 0, 'g', 8 )
                     .arg( b,    0, 'g', 8 )
                     .arg( sigb, 0, 'g', 8 )
                     .arg( b != 0 ? fabs( 100.0 * sigb / b ) : (double) 0, 0, 'g', 8 )
                     .arg( chi2, 0, 'g', 8 )
                     );

         double deltax = ( one_over_triangles[ 0 ] * 1.1 - UHB_MINX ) / ( UHB_PTS - 1 );
         for ( int i = 0; i < UHB_PTS; ++i )
         {
            x[ i ] = UHB_MINX + deltax * i;
            y[ i ] = exp( a + b * x[ i ] * log( x[ i ] ) );
         }
      
#ifndef QT4
         long curve = plot_data->insertCurve( "plot lr ln" );
         plot_data->setCurveStyle( curve, QwtCurve::Lines );
#else
         QwtPlotCurve *curve = new QwtPlotCurve( "plot lr ln" );
         curve->setStyle( QwtPlotCurve::Lines );
#endif

#ifndef QT4
         plot_data->setCurveData( curve, 
                                  (double *)&( x[ 0 ] ),
                                  (double *)&( y[ 0 ] ),
                                  UHB_PTS
                                  );
         plot_data->setCurvePen( curve, QPen( Qt::darkMagenta, 2, SolidLine));

#else
         curve->setData(
                        (double *)&( x[ 0 ] ),
                        (double *)&( y[ 0 ] ),
                        UHB_PTS
                        );

         curve->setPen( QPen( Qt::green, 2, Qt::SolidLine ) );
         curve->attach( plot_data );
#endif

         double min = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ 0 ] : y[ UHB_PTS - 1 ];
         double max = y[ 0 ] < y[ UHB_PTS - 1 ] ? y[ UHB_PTS - 1 ] : y[ 0 ];
         if ( miny > min )
         {
            miny = min;
         }
         if ( maxy < max )
         {
            maxy = max;
         }
      }
   }

   // set up axis scale

   {
      plot_data->setAxisScale( QwtPlot::xBottom, 0, one_over_triangles[ 0 ] * 1.1e0  );

                            
      miny < 0 ?
             plot_data->setAxisScale( QwtPlot::yLeft  , miny * 1.03e0 , maxy * .97e0 ) :
         plot_data->setAxisScale( QwtPlot::yLeft  , miny * 0.97e0 , maxy * 1.03e0 ) ;
   }

   if ( plot_data_zoomer )
   {
      delete plot_data_zoomer;
      plot_data_zoomer = (ScrollZoomer *)0;
   }

   plot_data_zoomer = new ScrollZoomer(plot_data->canvas());
   plot_data_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#ifndef QT4
   plot_data_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
   if ( do_recompute_tau &&
        tau_input_set.count( text ) )
   {
      qDebug( "data selected & do recompute_tau" );
      recompute_tau();
   }


   plot_data->replot();
}

void US_Hydrodyn_Best::save_results()
{
   QString use_dir = 
      USglobal->config_list.root_dir + 
      QDir::separator() + "somo" + 
      QDir::separator() + "cluster" +
      QDir::separator() + "results"
      ;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );
   use_dir += QDir::separator() + QFileInfo( loaded_csv_filename ).baseName() + "_results.csv";
   qDebug( use_dir );

   QString filename = QFileDialog::getSaveFileName(use_dir, "*.csv *.CSV", this,
                                                   caption() + tr( " Save Results" ),
                                                   tr( "Select a name to save the state" )
                                                   );

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filename );

   if ( filename.isEmpty() )
   {
      return;
   }

   if ( QFile::exists( filename ) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
      raise();
   }

   QFile f( filename );
   if ( !f.open( IO_WriteOnly ) )
   {
      editor_msg( "red", QString( tr( "Could not open file %1 for writing" ) ).arg( f.name() ) );
      return;
   }

   pb_save_results->setEnabled( false );
   pb_load        ->setEnabled( false );
   lb_data        ->setEnabled( false );
   disconnect( lb_data, SIGNAL( selectionChanged() ), 0, 0 );

   int cur_selected = lb_data->index( lb_data->selectedItem() );

   map < QString, QString > additions;

   bool any_ln_plot = false;
   for ( int i = 0; i < (int) lb_data->count(); ++i )
   {
      lb_data->setSelected( i, true );
      data_selected( false );
      additions[ lb_data->text( i ) ] = 
         QString( "=%1,%2%3,%4%5" )
         .arg( last_a,    0, 'g', 8 )
         .arg( isnan( last_siga ) ? "=" : "" )
         .arg( last_siga, 0, 'g', 8 )
         .arg( isnan( last_siga ) ? "=" : "" )
         .arg( last_a != 0 ? fabs( 100.0 * last_siga / last_a ) : (double) 0, 0, 'g', 8 )
         +
         QString( ",=%1,%2%3,%4%5" )
         .arg( last_b,    0, 'g', 8 )
         .arg( isnan( last_sigb ) ? "=" : "" )
         .arg( last_sigb, 0, 'g', 8 )
         .arg( isnan( last_sigb ) ? "=" : "" )
         .arg( last_b != 0 ? fabs( 100.0 * last_sigb / last_b ) : (double) 0, 0, 'g', 8 )
         +
         QString( ",=%1,%2" )
         .arg( last_chi2, 0, 'g', 8 )
         .arg( last_pts_removed )
         ;

      if ( ln_plot_ok )
      {
         any_ln_plot = true;
         additions[ lb_data->text( i ) ] += 
            QString( ",=%1,%2%3,%4%5" )
            .arg( exp( last_a_ln ),    0, 'g', 8 )
            .arg( ( isnan( last_siga_ln ) || isnan( last_a_ln ) || isnan( exp( last_a_ln ) ) )  ? "=" : "" )
            .arg( exp( last_a_ln ) * last_siga_ln, 0, 'g', 8 )
            .arg( ( isnan( last_siga_ln ) || isnan( last_a_ln ) )? "=" : "" )
            .arg( exp( last_a_ln ) != 0 ? fabs( 100.0 * exp( last_a_ln ) * last_siga_ln / exp( last_a_ln ) ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2%3,%4%5" )
            .arg( last_b_ln,    0, 'g', 8 )
            .arg( isnan( last_sigb_ln ) ? "=" : "" )
            .arg( last_sigb_ln, 0, 'g', 8 )
            .arg( isnan( last_sigb_ln ) ? "=" : "" )
            .arg( last_b_ln != 0 ? fabs( 100.0 * last_sigb_ln / last_b_ln ) : (double) 0, 0, 'g', 8 )
            +
            QString( ",=%1,%2" )
            .arg( last_chi2_ln, 0, 'g', 8 )
            .arg( last_pts_removed_ln )
            ;
      }
   }      
   recompute_tau();

   QStringList out;
   for ( int i = 0; i < (int)loaded_csv_trimmed.size(); ++i )
   {
      QStringList qsl = US_Csv::parse_line( loaded_csv_trimmed[ i ] ).gres( "\"", "" );
      out << loaded_csv_trimmed[ i ];
      if ( qsl.size() && additions.count( qsl[ 0 ] ) )
      {
         out.back() += additions[ qsl[ 0 ] ];
      }
   }
   for ( int i = 0; i < (int) tau_csv_addendum_tag.size(); ++i )
   {
      out << QString( "\"%1\"," ).arg( tau_csv_addendum_tag[ i ] );
      for ( int j = 0; j < points; ++j )
      {
         out.back() += ",";
      }
      out.back() += tau_csv_addendum_val[ i ];
   }

   out[ 0 ] += ",\"Points removed (largest number of triangles is point 1)\"";
   if ( any_ln_plot )
   {
      out[ 0 ] +=
         ",\"LN: Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\""
         ;
      out[ 0 ] += ",\"Points removed (largest number of triangles is point 1)\"";
   }
   // qDebug( loaded_csv_filename );
   // qDebug( loaded_csv_trimmed.join( "\n" ) );
   // qDebug( out.join( "\n" ) );
   QTextStream ts( &f );
   ts << out.join( "\n" ) << endl;
   f.close();

   connect( lb_data, SIGNAL( selectionChanged() ), SLOT( data_selected() ) );
   lb_data->setSelected( cur_selected, TRUE );
   pb_save_results->setEnabled( true );
   pb_load        ->setEnabled( true );
   lb_data        ->setEnabled( true );

}

void US_Hydrodyn_Best::recompute_tau()
{

   bool lin_ok = true;
   bool log_ok = true;

   tau_csv_addendum_tag.clear();
   tau_csv_addendum_val.clear();

   for ( int i = 0; i < (int) tau_inputs.size(); ++i )
   {
      if ( !last_lin_extrapolation.count( tau_inputs[ i ] ) )
      {
         lin_ok = false;
      }
      if ( !last_log_extrapolation.count( tau_inputs[ i ] ) )
      {
         log_ok = false;
      }
   }

   if ( lin_ok )
   {
      vector < double > this_tau_results;
      QString msg;
      
      US_Saxs_Util::compute_tau( 
                                last_lin_extrapolation[ tau_inputs[ 0 ] ][ 0 ] * 1e-3,
                                last_lin_extrapolation[ tau_inputs[ 1 ] ][ 0 ] * 1e-3,
                                last_lin_extrapolation[ tau_inputs[ 2 ] ][ 0 ] * 1e-3,
                                .1,
                                this_tau_results );

      for ( int i = 0; i < (int) this_tau_results.size(); ++i )
      {
         tau_csv_addendum_tag << QString( tr( "Linear extrapolation of Drr EV (1/s) %1" ) ).arg( tau_msg[ i ] );
         tau_csv_addendum_val << QString( "%1" ).arg( this_tau_results[ i ], 0, 'g', 8 );
         msg += tau_csv_addendum_tag.back() + " " + tau_csv_addendum_val.back() + "\n";
      }
      editor_msg( "dark blue", msg );
   }

   if ( log_ok )
   {
      vector < double > this_tau_results;
      
      US_Saxs_Util::compute_tau( 
                                exp( last_log_extrapolation[ tau_inputs[ 0 ] ][ 0 ] ) * 1e-3,
                                exp( last_log_extrapolation[ tau_inputs[ 1 ] ][ 0 ] ) * 1e-3,
                                exp( last_log_extrapolation[ tau_inputs[ 2 ] ][ 0 ] ) * 1e-3,
                                .1,
                                this_tau_results );

      QString msg;
      for ( int i = 0; i < (int) this_tau_results.size(); ++i )
      {
         tau_csv_addendum_tag << QString( tr( "Log extrapolation of Drr EV (1/s) %1" ) ).arg( tau_msg[ i ] );
         tau_csv_addendum_val << QString( "%1" ).arg( this_tau_results[ i ], 0, 'g', 8 );
         msg += tau_csv_addendum_tag.back() + " " + tau_csv_addendum_val.back() + "\n";
      }
      editor_msg( "dark blue", msg );
   }
}

void US_Hydrodyn_Best::toggle_points()
{
   bool any_checked = false;
   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      if ( cb_points[ i ]->isChecked() )
      {
         any_checked = true;
         break;
      }
   }

   for ( int i = 0; i < (int) cb_points.size(); ++i )
   {
      disconnect( cb_points[ i ], SIGNAL( clicked() ), 0, 0 );
      cb_points[ i ]->setChecked( !any_checked );
      connect( cb_points[ i ], SIGNAL( clicked() ), SLOT( cb_changed() ) );
   }
   cb_changed();
   // data_selected();
}

void US_Hydrodyn_Best::toggle_points_ln()
{
   bool any_checked = false;
   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isChecked() )
      {
         any_checked = true;
         break;
      }
   }

   for ( int i = 0; i < (int) cb_points_ln.size(); ++i )
   {
      if ( cb_points_ln[ i ]->isEnabled() )
      {
         disconnect( cb_points_ln[ i ], SIGNAL( clicked() ), 0, 0 );
         cb_points_ln[ i ]->setChecked( !any_checked );
         connect( cb_points_ln[ i ], SIGNAL( clicked() ), SLOT( cb_changed_ln() ) );
      }
   }
   cb_changed_ln();
   // data_selected();
}
