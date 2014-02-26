#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_saxs_search.h"
//Added by qt3to4:
#include <Q3TextStream>
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include <Q3PopupMenu>
#include <Q3VBoxLayout>
#include <Q3BoxLayout>
#include <QCloseEvent>

#define SLASH QDir::separator()

US_Hydrodyn_Saxs_Search::US_Hydrodyn_Saxs_Search(
                                               csv csv1,
                                               void *us_hydrodyn, 
                                               QWidget *p, 
                                               const char *name
                                               ) : Q3Frame(p, name)
{
   this->csv1 = csv1;
   this->us_hydrodyn = us_hydrodyn;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("US-SOMO: SAXS Search Control"));
   order_ascending = false;

   saxs_widget = &(((US_Hydrodyn *) us_hydrodyn)->saxs_plot_widget);
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   ((US_Hydrodyn *) us_hydrodyn)->saxs_search_widget = true;

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

US_Hydrodyn_Saxs_Search::~US_Hydrodyn_Saxs_Search()
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_search_widget = false;
}

void US_Hydrodyn_Saxs_Search::setupGUI()
{
   int minHeight1 = 30;
   int minHeight3 = 30;

   lbl_title = new QLabel(csv1.name.left(80), this);
   lbl_title->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   t_csv = new Q3Table(csv1.data.size(), csv1.header.size(), this);
   t_csv->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   // t_csv->setMinimumWidth(minWidth1);
   t_csv->setPalette( PALET_EDIT );
   AUTFBACK( t_csv );
   t_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   t_csv->setEnabled(true);

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.num_data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            t_csv->setItem( i, j, new Q3CheckTableItem( t_csv, "" ) );
            ((Q3CheckTableItem *)(t_csv->item( i, j )))->setChecked( csv1.data[i][j] == "Y" );
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
   t_csv->setColumnReadOnly(t_csv->numCols() - 1, true);

   // probably I'm not understanding something, but these next two lines don't seem to do anything
   t_csv->horizontalHeader()->adjustHeaderSize();
   t_csv->adjustSize();
   t_csv->setMinimumHeight( ( t_csv->rowHeight( 0 ) + 6 ) * csv1.num_data.size() );
   t_csv->setMaximumHeight( ( t_csv->rowHeight( 0 ) + 10 ) * csv1.num_data.size() );

   recompute_interval_from_points();

   connect(t_csv, SIGNAL(valueChanged(int, int)), SLOT(table_value(int, int )));

   progress = new Q3ProgressBar(this, "Progress");
   progress->setMinimumHeight(minHeight1);
   progress->setPalette( PALET_NORMAL );
   AUTFBACK( progress );
   progress->reset();

   cb_save_to_csv = new QCheckBox(this);
   cb_save_to_csv->setText(tr(" Combined I(q) Results File:"));
   cb_save_to_csv->setChecked(false);
   cb_save_to_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_to_csv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_to_csv );
   connect(cb_save_to_csv, SIGNAL(clicked()), SLOT(save_to_csv()));

   le_csv_filename = new QLineEdit(this, "csv_filename Line Edit");
   le_csv_filename->setText("search_results");
   le_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_filename->setMinimumWidth(150);
   le_csv_filename->setPalette( PALET_NORMAL );
   AUTFBACK( le_csv_filename );
   le_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   cb_individual_files = new QCheckBox(this);
   cb_individual_files->setText(tr(" Create individual SAXS Results Files"));
   cb_individual_files->setChecked(false);
   cb_individual_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_individual_files->setPalette( PALET_NORMAL );
   AUTFBACK( cb_individual_files );

   pb_replot_saxs = new QPushButton(tr("Replot stored I(q)"), this);
   pb_replot_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_replot_saxs->setMinimumHeight(minHeight1);
   pb_replot_saxs->setPalette( PALET_PUSHB );
   connect(pb_replot_saxs, SIGNAL(clicked()), SLOT(replot_saxs()));

   pb_save_saxs_plot = new QPushButton(tr("Store current I(q) plot"), this);
   pb_save_saxs_plot->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_saxs_plot->setMinimumHeight(minHeight1);
   pb_save_saxs_plot->setPalette( PALET_PUSHB );
   connect(pb_save_saxs_plot, SIGNAL(clicked()), SLOT(save_saxs_plot()));

   pb_set_target = new QPushButton(tr("Set Target"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( PALET_PUSHB );
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   lbl_current_target = new QLabel("", this);
   lbl_current_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_current_target->setMinimumHeight(minHeight1);
   lbl_current_target->setPalette( PALET_LABEL );
   AUTFBACK( lbl_current_target );
   lbl_current_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_start = new QPushButton(tr("Start"), this);
   pb_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_start->setMinimumHeight(minHeight1);
   pb_start->setPalette( PALET_PUSHB );
   connect(pb_start, SIGNAL(clicked()), SLOT(start()));

   pb_run_current = new QPushButton(tr("Run Current"), this);
   pb_run_current->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_run_current->setMinimumHeight(minHeight1);
   pb_run_current->setPalette( PALET_PUSHB );
   connect(pb_run_current, SIGNAL(clicked()), SLOT(run_current()));

   pb_run_best = new QPushButton(tr("Run Best"), this);
   pb_run_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_run_best->setMinimumHeight(minHeight1);
   pb_run_best->setPalette( PALET_PUSHB );
   connect(pb_run_best, SIGNAL(clicked()), SLOT(run_best()));

   pb_stop = new QPushButton(tr("Stop"), this);
   pb_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_stop->setMinimumHeight(minHeight1);
   pb_stop->setPalette( PALET_PUSHB );
   connect(pb_stop, SIGNAL(clicked()), SLOT(stop()));

   editor = new Q3TextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

   Q3Frame *frame;
   frame = new Q3Frame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
   editor->setWordWrap (Q3TextEdit::WidgetWidth);
   editor->setMinimumHeight(300);
   
   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   Q3HBoxLayout *hbl_csv = new Q3HBoxLayout(0);
   hbl_csv->addSpacing(4);
   hbl_csv->addWidget(cb_save_to_csv);
   hbl_csv->addSpacing(4);
   hbl_csv->addWidget(le_csv_filename);
   hbl_csv->addSpacing(4);
   hbl_csv->addWidget(cb_individual_files);
   hbl_csv->addSpacing(4);

   Q3HBoxLayout *hbl_target = new Q3HBoxLayout(0);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_replot_saxs);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_save_saxs_plot);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(pb_set_target);
   hbl_target->addSpacing(4);
   hbl_target->addWidget(lbl_current_target);
   hbl_target->addSpacing(4);

   Q3HBoxLayout *hbl_controls = new Q3HBoxLayout(0);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_start);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_run_current);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_run_best);
   hbl_controls->addSpacing(4);
   hbl_controls->addWidget(pb_stop);
   hbl_controls->addSpacing(4);

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout(0);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(4);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(4);

   Q3BoxLayout *vbl_editor_group = new Q3VBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   Q3VBoxLayout *background = new Q3VBoxLayout(this);
   background->addSpacing(4);
   background->addWidget(lbl_title);
   background->addSpacing(4);
   background->addWidget(t_csv);
   background->addLayout(vbl_editor_group);
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

void US_Hydrodyn_Saxs_Search::cancel()
{
   close();
}

void US_Hydrodyn_Saxs_Search::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_SAXS_search.html");
}

void US_Hydrodyn_Saxs_Search::closeEvent(QCloseEvent *e)
{
   ((US_Hydrodyn *)us_hydrodyn)->saxs_search_widget = false;
   ((US_Hydrodyn *)us_hydrodyn)->last_saxs_search_csv = current_csv();

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Search::table_value( int /* row */, int col )
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

void US_Hydrodyn_Saxs_Search::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Saxs_Search::update_font()
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

void US_Hydrodyn_Saxs_Search::save()
{
   QString fn;
   fn = Q3FileDialog::getSaveFileName(QString::null, QString::null,this );
   if(!fn.isEmpty() )
   {
      QString text = editor->text();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      Q3TextStream t( &f );
      t << text;
      f.close();
      editor->setModified( false );
      setCaption( fn );
   }
}

void US_Hydrodyn_Saxs_Search::set_target()
{
   QString scaling_target = "";
   if ( *saxs_widget ) 
   { 
      saxs_window->set_scaling_target( scaling_target );
      if ( !scaling_target.isEmpty() )
      {
         saxs_window->ask_iq_target_grid();
         if (
             saxs_window->our_saxs_options->saxs_iq_crysol &&
             ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
             !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
         {
            editor_msg( "red", QString( tr( "WARNING: Crysol has an active experimental target of %1 which may or may not be the same as the selected plot target %2" ) )
                        .arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] )
                        .arg( scaling_target ) );
         }
      } else {
         if (
             saxs_window->our_saxs_options->saxs_iq_crysol &&
             ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
             !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
         {
            editor_msg( "dark red", QString( tr( "NOTICE: Crysol has an active experimental target of %1" ) )
                        .arg( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ) );
         }
      }         
   }
        
   lbl_current_target->setText( scaling_target );
}

void US_Hydrodyn_Saxs_Search::start()
{
   if ( !validate() ||
        !any_to_run() ||
        !not_active_warning() )
   {
      return;
   }
   best_fitness = 1e99;
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

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
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
      }
   }
   
   unsigned int total_points = 1;
   for ( map < unsigned int, unsigned int >::iterator it = points.begin();
         it != points.end();
         it++ )
   {
      total_points *= it->second;
   }

   editor_msg("black", QString("total points %1\n").arg(total_points));

   // linearization of an arbitrary number of loops

   for ( unsigned int i = 0; i < total_points; i++ )
   {
      if ( !running )
      {
         break;
      }
      progress->setProgress(i, total_points);
      unsigned int pos = i;
      QString msg = "";
      for ( map < unsigned int, unsigned int >::iterator it = points.begin();
            it != points.end();
            it++ )
      {
         t_csv->setText(it->first, 6, QString("%1").arg(starts[it->first] + ( pos % it->second ) * increments[it->first]));
         pos /= it->second;
         msg += QString("%1 %2; ").arg(t_csv->text(it->first, 0)).arg(t_csv->text(it->first, 6));
      }
      run_one();
      
      csv_source_name_iqq.push_back( saxs_window->te_filename2->text() + " " +
                                     ( lbl_current_target->text().isEmpty() ?
                                       msg : QString("Target: %1 %2 fitness %3")
                                       .arg(lbl_current_target->text()) 
                                       .arg(msg) 
                                       .arg(saxs_window->last_rescaling_chi2)
                                       ) );
      saxs_header_iqq = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header;
      if ( saxs_q.size() < ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.size() )
      {
         saxs_q = ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q;
      }
      saxs_iqq.push_back(((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq);
   }

   if ( running && cb_save_to_csv->isChecked() )
   {
      save_csv_saxs_iqq();
   }

   progress->setProgress(1, 1);
   if ( saxs_window->plotted )
   {
      saxs_window->editor->append("\n");
      saxs_window->plotted = false;
   }
   running = false;
   update_enables();

}

void US_Hydrodyn_Saxs_Search::run_current()
{
   if ( !validate() ||
        !any_to_run() ||
        !not_active_warning() )
   {
      return;
   }
   running = true;
   update_enables();
   run_one();
   if ( saxs_window->plotted )
   {
      saxs_window->editor->append("\n");
      saxs_window->plotted = false;
   }
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Search::run_best()
{
   if ( !validate() ||
        !any_to_run() ||
        !not_active_warning() )
   {
      return;
   }

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         if ( t_csv->text(i, 7).isEmpty() )
         {
            editor_msg("red", "Some of the selected \"Best value\" fields are empty\n");
            return;
         }
      }
   }

   running = true;
   update_enables();
   
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         t_csv->setText(i, 6, t_csv->text(i, 7) );
      }
   }

   run_one();
   if ( saxs_window->plotted )
   {
      saxs_window->editor->append("\n");
      saxs_window->plotted = false;
   }
   running = false;
   update_enables();
}

void US_Hydrodyn_Saxs_Search::run_one()
{
   // assume validated, updates set, this runs one with "current" values

   // save current values:
   float     save_water_e_density            = saxs_window->our_saxs_options->water_e_density;
   float     save_swh_excl_vol               = saxs_window->our_saxs_options->swh_excl_vol;
   float     save_scale_excl_vol             = saxs_window->our_saxs_options->scale_excl_vol;
   bool      save_iqq_ask_target_grid        = saxs_window->our_saxs_options->iqq_ask_target_grid;
   QString   save_iqq_default_scaling_target = saxs_window->our_saxs_options->iqq_default_scaling_target;
   bool      save_create_native_saxs         = saxs_window->create_native_saxs;

   // set current values:
   saxs_window->our_saxs_options->iqq_ask_target_grid = false;
   saxs_window->our_saxs_options->iqq_default_scaling_target = lbl_current_target->text();
   if ( lbl_current_target->text().isEmpty() )
   {
      saxs_window->clear_plot_saxs( true );
   }
   saxs_window->cb_create_native_saxs        ->setChecked( cb_individual_files->isChecked() );
   saxs_window->create_native_saxs           = cb_individual_files->isChecked();
   
   QString msg =  QString("%1 model %2: ")
      .arg(saxs_window->te_filename2->text())
      .arg(saxs_window->selected_models[0] + 1);

   bool any = false;

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         if ( any )
         {
            msg += "; ";
         }
         msg += QString("%1 %2").arg(t_csv->text( i, 0 )).arg(t_csv->text(i, 6));
         any = true;
         if ( t_csv->text( i, 0 ).contains("Buffer electron density") )
         {
            saxs_window->our_saxs_options->water_e_density = t_csv->text(i, 6).toFloat();
         }
         if ( t_csv->text( i, 0 ).contains("Scaling excluded volume") )
         {
            saxs_window->our_saxs_options->scale_excl_vol = t_csv->text(i, 6).toFloat();
         }
         if ( t_csv->text( i, 0 ).contains("WAT excluded volume") )
         {
            saxs_window->our_saxs_options->swh_excl_vol = t_csv->text(i, 6).toFloat();
         }
         if ( t_csv->text( i, 0 ).contains("Crysol: average atomic radius") )
         {
            (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] = t_csv->text(i, 6);
         }
         if ( t_csv->text( i, 0 ).contains("Crysol: Excluded volume") )
         {
            (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] = t_csv->text(i, 6);
         }
         if ( t_csv->text( i, 0 ).contains("Crysol: contrast of hydration shell") )
         {
            saxs_window->our_saxs_options->crysol_hydration_shell_contrast = t_csv->text(i, 6).toFloat();
         }
      }
   }

   saxs_window->update_iqq_suffix();

   // run the analysis, get fitness
   
   editor_msg("black", "Running anaysis " + msg + "\n");
   qApp->processEvents();
   
   // change to saxs/sans?

   saxs_window->show_plot_saxs();

   raise();

   if (
       ( !saxs_window->our_saxs_options->saxs_sans &&
         ( saxs_window->our_saxs_options->saxs_iq_foxs ||
           saxs_window->our_saxs_options->saxs_iq_crysol ) ) ||
       ( saxs_window->our_saxs_options->saxs_sans &&
         saxs_window->our_saxs_options->saxs_iq_crysol )
       )
   {
      // block until finished, there must be a better way of doing this
      // #if !defined(WIN32)
      //       timespec ns;
      //       timespec ns_ret;
      //       ns.tv_sec = 0;
      //       ns.tv_nsec = 25000000l;
      // #endif
      while ( saxs_window->external_running )
      {
         qApp->processEvents();
         mQThread::msleep( 333 );
         // #if defined(WIN32)
         //          _sleep( 1 );
         // #else
         //          nanosleep(&ns, &ns_ret);
         // #endif
      }
   }

   if ( running )
   {
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("dark gray") );
      if ( lbl_current_target->text().isEmpty() )
      {
         editor_msg_qc(saxs_window->plot_colors
                       [(saxs_window->plotted_I.size() - 1) % saxs_window->plot_colors.size()], "Run complete\n\n");
      } else {
         editor_msg_qc(saxs_window->plot_colors
                       [(saxs_window->plotted_I.size() - 1) % saxs_window->plot_colors.size()], "Run complete");
         QString msg = QString("Fitness %1").arg(saxs_window->last_rescaling_chi2);
         if ( saxs_window->last_rescaling_chi2 < best_fitness )
         {
            msg += " *** Best fitness so far *** ";
            best_fitness = saxs_window->last_rescaling_chi2;
            for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
            {
               if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
               {
                  t_csv->setText(i, 7, t_csv->text(i, 6));
               }
            }
         }
         editor_msg("black", msg + "\n");
      }
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
   }
      
   // restore values
   saxs_window->our_saxs_options->water_e_density            = save_water_e_density;
   saxs_window->our_saxs_options->swh_excl_vol               = save_swh_excl_vol;
   saxs_window->our_saxs_options->scale_excl_vol             = save_scale_excl_vol;
   saxs_window->our_saxs_options->iqq_ask_target_grid        = save_iqq_ask_target_grid;
   saxs_window->our_saxs_options->iqq_default_scaling_target = save_iqq_default_scaling_target;
   saxs_window->create_native_saxs                           = save_create_native_saxs;

   saxs_window->cb_create_native_saxs                        ->setChecked( save_create_native_saxs );
}

void US_Hydrodyn_Saxs_Search::stop()
{
   running = false;
   saxs_window->stopFlag = true;
   editor_msg("red", "Stopped by user request\n");
   update_enables();
}

void US_Hydrodyn_Saxs_Search::update_enables()
{
   // cout << "US_Hydrodyn_Saxs_Search::update_enables()\n";
   // cout << QString("saxs_window->qsl_plotted_iq_names.size() %1\n").arg(saxs_window->qsl_plotted_iq_names.size());
   bool any_best_empty = false;
   bool any_selected   = false;
   if ( !running )
   {
      for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
      {
         if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
         {
            any_selected = true;
            if ( t_csv->text(i, 7).isEmpty() )
            {
               any_best_empty = true;
            }
         }
      }
   }
   pb_start            ->setEnabled( !running && any_selected );
   pb_run_current      ->setEnabled( !running && any_selected );
   pb_run_best         ->setEnabled( !running && !any_best_empty && any_selected);
   pb_stop             ->setEnabled( running );
   pb_replot_saxs      ->setEnabled( !running && *saxs_widget && names.size() );
   pb_save_saxs_plot   ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   pb_set_target       ->setEnabled( !running && *saxs_widget && saxs_window->qsl_plotted_iq_names.size() );
   cb_save_to_csv      ->setEnabled( !running );
   le_csv_filename     ->setEnabled( !running && cb_save_to_csv->isChecked() );
   cb_individual_files ->setEnabled( !running );
   t_csv               ->setEnabled( !running );

   if ( *saxs_widget )
   {
      saxs_window->update_iqq_suffix();
   }
}

bool US_Hydrodyn_Saxs_Search::any_to_run()
{
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         return true;
      }
   }
   editor_msg("dark red", "Nothing to do: At least one of the rows must have Active set to Y");
   return false;
}

void US_Hydrodyn_Saxs_Search::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

void US_Hydrodyn_Saxs_Search::editor_msg_qc( QColor qcolor, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(qcolor);
   editor->append(msg);
   editor->setColor(save_color);
}

bool US_Hydrodyn_Saxs_Search::validate()
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

bool US_Hydrodyn_Saxs_Search::activate_saxs_window()
{
   if ( !*saxs_widget )
   {
      ((US_Hydrodyn *)us_hydrodyn)->pdb_saxs();
      raise();
      setFocus();
      if ( !*saxs_widget )
      {
         editor_msg("red", tr("Could not activate SAXS window!\n"));
         return false;
      }
   }
   saxs_window = ((US_Hydrodyn *) us_hydrodyn)->saxs_plot_window;
   return true;
}

bool US_Hydrodyn_Saxs_Search::validate_saxs_window()
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

   if ( !saxs_window->selected_models.size() )
   {
      editor_msg("red", "No model is currently available in the SAXS window\n");
      return false;
   }

   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( ((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() )
      {
         if ( 
             ( t_csv->text( i, 0 ).contains("Scaling excluded volume") ||
               t_csv->text( i, 0 ).contains("WAT excluded volume") ) 
             )
         {
            if ( saxs_window->our_saxs_options->saxs_iq_crysol ||
                 saxs_window->our_saxs_options->saxs_iq_foxs )
            {
               QMessageBox::warning( this, 
                                     "US-SOMO Search",
                                     "Scaling excluded volume and WAT excluded volume\n"
                                     "are not supported by the FoXS and CRYSOL I(q) functions\n"
                                     );
               return false;
            }
         }
         if ( 
             ( t_csv->text( i, 0 ).contains("Crysol: Excluded volume") ||
               t_csv->text( i, 0 ).contains("Crysol: average atomic radius") ||
               t_csv->text( i, 0 ).contains("Crysol: contrast of hydration shell") ) 
             )
         {
            if ( !saxs_window->our_saxs_options->saxs_iq_crysol )
            {
               QMessageBox::warning( this, 
                                     "US-SOMO Search",
                                     "Crysol excluded volume, average atomic radius and contrast of hydration shell\n"
                                     "are only supported by Crysol I(q)\n"
                                     );
               return false;
            }
         }
      }
   }

   if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite &&
        ( cb_individual_files->isChecked() ||
          ( !saxs_window->our_saxs_options->saxs_sans &&
            ( saxs_window->our_saxs_options->saxs_iq_foxs ||
              saxs_window->our_saxs_options->saxs_iq_crysol ) ) ||
          ( saxs_window->our_saxs_options->saxs_sans &&
            saxs_window->our_saxs_options->saxs_iq_crysol )
          )
        )
   {
      switch ( QMessageBox::warning(this, 
                                    tr("UltraScan Warning"),
                                    QString(tr("Please note:\n\n"
                                               "Overwriting of existing files currently off.\n"
                                               "This could cause Searching to block during processing.\n"
                                               "What would you like to do?\n")),
                                    tr("&Stop"), 
                                    tr("&Turn on overwrite now"),
                                    tr("C&ontinue anyway"),
                                    0, // Stop == button 0
                                    0 // Escape == button 0
                                    ) )
      {
      case 0 : // stop
         return false;
         break;
      case 1 :
         ((US_Hydrodyn *)us_hydrodyn)->overwrite = true;
         ((US_Hydrodyn *)us_hydrodyn)->cb_overwrite->setChecked(true);
         break;
      case 2 : // continue
         break;
      }
   }

   return true;
}

void US_Hydrodyn_Saxs_Search::save_to_csv()
{
   update_enables();
}

void US_Hydrodyn_Saxs_Search::do_replot_saxs()
{
   saxs_window->clear_plot_saxs(true);
   saxs_window->plotted = false;
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      saxs_window->plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   if ( saxs_window->plotted )
   {
      saxs_window->editor_msg( "black", "I(q) plot done\n");
      saxs_window->plotted = false;
   }
   saxs_window->rescale_plot();
}

void US_Hydrodyn_Saxs_Search::replot_saxs()
{
   if ( activate_saxs_window() )
   {
      do_replot_saxs();
   }
}

void US_Hydrodyn_Saxs_Search::save_saxs_plot()
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
                                           "US_SOMO: I(q) search",
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
                              "US_SOMO: I(q) search",
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

void US_Hydrodyn_Saxs_Search::save_csv_saxs_iqq()
{
   QString fname = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      le_csv_filename->text() + "_iqq_search.csv";

   if ( QFile::exists(fname) )
      // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
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


csv US_Hydrodyn_Saxs_Search::current_csv()
{
   csv tmp_csv = csv1;
   
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         if ( csv1.data[i][j] == "Y" || csv1.data[i][j] == "N" )
         {
            tmp_csv.data[i][j] = ((Q3CheckTableItem *)(t_csv->item( i, j )))->isChecked() ? "Y" : "N";
         } else {
            tmp_csv.data[i][j] = t_csv->text( i, j );
         }
         tmp_csv.num_data[i][j] = tmp_csv.data[i][j].toDouble();
      }
   }
   return tmp_csv;
}
  
void US_Hydrodyn_Saxs_Search::recompute_interval_from_points()
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


void US_Hydrodyn_Saxs_Search::recompute_points_from_interval()
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

bool US_Hydrodyn_Saxs_Search::not_active_warning()
{
   QString msg;
   for ( unsigned int i = 0; i < (unsigned int)t_csv->numRows(); i++ )
   {
      if ( !((Q3CheckTableItem *)(t_csv->item( i, 1 )))->isChecked() &&
           !t_csv->text( i, 6 ).isEmpty() )
      {
         if ( t_csv->text( i, 0 ).contains("Buffer e density") &&
              saxs_window->our_saxs_options->water_e_density != t_csv->text(i, 6).toFloat() )
         {
            switch ( QMessageBox::warning(this, 
                                          tr("US-SOMO: I(q) search"),
                                          QString(tr("Please note:\n\n"
                                                     "\"Buffer e density\" is inactive, "
                                                     "yet is has a current value of %1, "
                                                     "whereas the SAXS options value is set to %2\n"
                                                     "What would you like to do?\n"))
                                          .arg(t_csv->text(i, 6).toFloat())
                                          .arg(saxs_window->our_saxs_options->water_e_density)
                                          ,
                                          tr("&Stop"), 
                                          tr("&Set the SAXS options value to the current value"),
                                          tr("C&ontinue anyway"),
                                          0, // Stop == button 0
                                          0 // Escape == button 0
                                    ) )
            {
            case 0 : // stop
               return false;
               break;
            case 1 :
               saxs_window->our_saxs_options->water_e_density = t_csv->text(i, 6).toFloat();
               break;
            case 2 : // continue
               break;
            }
         }
         if ( t_csv->text( i, 0 ).contains("Scaling excluded volume") &&
              saxs_window->our_saxs_options->scale_excl_vol != t_csv->text(i, 6).toFloat() )
         {
            switch ( QMessageBox::warning(this, 
                                          tr("US-SOMO: I(q) search"),
                                          QString(tr("Please note:\n\n"
                                                     "\"Scaling excluded volume\" is inactive, "
                                                     "yet is has a current value of %1, "
                                                     "whereas the SAXS options value is set to %2\n"
                                                     "What would you like to do?\n"))
                                          .arg(t_csv->text(i, 6).toFloat())
                                          .arg(saxs_window->our_saxs_options->scale_excl_vol)
                                          ,
                                          tr("&Stop"), 
                                          tr("&Set the SAXS options value to the current value"),
                                          tr("C&ontinue anyway"),
                                          0, // Stop == button 0
                                          0 // Escape == button 0
                                    ) )
            {
            case 0 : // stop
               return false;
               break;
            case 1 :
               saxs_window->our_saxs_options->scale_excl_vol = t_csv->text(i, 6).toFloat();
               break;
            case 2 : // continue
               break;
            }
         }
         if ( t_csv->text( i, 0 ).contains("WAT excluded volume") &&
              saxs_window->our_saxs_options->swh_excl_vol != t_csv->text(i, 6).toFloat() )
         {
            switch ( QMessageBox::warning(this, 
                                          tr("US-SOMO: I(q) search"),
                                          QString(tr("Please note:\n\n"
                                                     "\"WAT excluded volume\" is inactive, "
                                                     "yet is has a current value of %1, "
                                                     "whereas the SAXS options value is set to %2\n"
                                                     "What would you like to do?\n"))
                                          .arg(t_csv->text(i, 6).toFloat())
                                          .arg(saxs_window->our_saxs_options->swh_excl_vol)
                                          ,
                                          tr("&Stop"), 
                                          tr("&Set the SAXS options value to the current value"),
                                          tr("C&ontinue anyway"),
                                          0, // Stop == button 0
                                          0 // Escape == button 0
                                    ) )
            {
            case 0 : // stop
               return false;
               break;
            case 1 :
               saxs_window->our_saxs_options->swh_excl_vol = t_csv->text(i, 6).toFloat();
               break;
            case 2 : // continue
               break;
            }
         }
      }
   }
   return true;
}
