#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"

#define SLASH QDir::separator()

US_Hydrodyn_Cluster::US_Hydrodyn_Cluster(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *name
                                         ) : QDialog(p, name)
{
   this->us_hydrodyn = us_hydrodyn;
   this->batch_window = (US_Hydrodyn_Batch *)p;
   this->our_saxs_options = &((US_Hydrodyn *)us_hydrodyn)->saxs_options;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO: Cluster"));

   cluster_config[ "userid" ] = "";
   cluster_config[ "server" ] = "";

   selected_files.clear();
   for ( int i = 0; i < batch_window->lb_files->numRows(); i++ )
   {
      if ( batch_window->lb_files->isSelected(i) )
      {
         selected_files << batch_window->get_file_name( i );
      }
   }

   create_enabled =
      selected_files.size() &&
      // later we will add some of these other options,
      // right now, just iqq
      !batch_window->cb_somo->isChecked() && 
      !batch_window->cb_grid->isChecked() && 
      !batch_window->cb_hydro->isChecked() && 
      !batch_window->cb_prr->isChecked() && 
      ( batch_window->cb_iqq->isChecked() ||
        batch_window->cb_dmd->isChecked() ) 
      &&
      !batch_window->cb_compute_iq_avg->isChecked();

   setupGUI();

   pb_add_target  ->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
   pb_clear_target->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
   le_no_of_jobs  ->setEnabled( create_enabled );
   if ( batch_window->cluster_no_of_jobs.toUInt() &&
        batch_window->cluster_no_of_jobs.toUInt() <= selected_files.size() )
   {
      le_no_of_jobs->setText( batch_window->cluster_no_of_jobs );
   }
   lb_target_files   ->insertStringList  ( batch_window->cluster_target_datafiles );
   le_output_name    ->setText           ( batch_window->cluster_output_name.isEmpty() ?
                                           "job" : batch_window->cluster_output_name );
   cb_for_mpi        ->setChecked        ( batch_window->cluster_for_mpi );
   csv_advanced = batch_window->cluster_csv_advanced;
   csv_dmd      = batch_window->cluster_csv_dmd;
   if ( cb_for_mpi->isChecked() )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( selected_files.size() ) );
   }
   
   le_output_name->setEnabled( create_enabled );
   pb_create_pkg ->setEnabled( create_enabled );
   cb_for_mpi    ->setEnabled( create_enabled );
   pb_dmd        ->setEnabled( batch_window->cb_dmd->isChecked() );

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir dir1( pkg_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( pkg_dir ) );
      dir1.mkdir( pkg_dir );
   }

   {
      submitted_dir = pkg_dir  + SLASH + "submitted";
      QDir dir1( submitted_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( submitted_dir ) );
         dir1.mkdir( submitted_dir );
      }
      
      QDir::setCurrent( submitted_dir );

      QDir qd;

      QStringList tgz_files = qd.entryList( "*.tgz" );
      QStringList tar_files = qd.entryList( "*.tar" );
      QStringList submitted_files = QStringList::split( "\n", 
                                                        tgz_files.join("\n") + 
                                                        ( tgz_files.size() ? "\n" : "" ) +
                                                        tar_files.join("\n") );
      for ( unsigned int i = 0; i < submitted_files.count(); i++ )
      {
         submitted_jobs[ submitted_files[ i ].replace( QRegExp( "\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   {
      completed_dir = pkg_dir  + SLASH + "completed";
      QDir dir1( completed_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( completed_dir ) );
         dir1.mkdir( completed_dir );
      }

      QDir::setCurrent( completed_dir );

      QDir qd;

      QStringList tgz_files = qd.entryList( "*.tgz" );
      QStringList tar_files = qd.entryList( "*.tar" );
      QStringList completed_files = QStringList::split( "\n", 
                                            tgz_files.join("\n") + 
                                            ( tgz_files.size() ? "\n" : "" ) +
                                            tar_files.join("\n") );
      for ( unsigned int i = 0; i < completed_files.count(); i++ )
      {
         completed_jobs[ completed_files[ i ].replace( QRegExp( "_(out|OUT)\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   {
      results_dir = pkg_dir  + SLASH + "results";
      QDir dir1( results_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( results_dir ) );
         dir1.mkdir( results_dir );
      }

      QDir::setCurrent( results_dir );

      QDir qd;

      QStringList results_files = qd.entryList( "*" );
      for ( unsigned int i = 0; i < results_files.count(); i++ )
      {
         results_jobs[ results_files[ i ].replace( QRegExp( "_(out|OUT)\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   QDir::setCurrent( pkg_dir );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   editor_msg( "dark blue", options_summary() );
   unsigned int number_active;
   if ( batch_window->cb_dmd->isChecked() &&
        validate_csv_dmd( number_active ) )
   {
      dmd();
   } else {
      update_enables();
   }

   if ( !create_enabled )
   {
      editor_msg( "dark red", tr( "Notice: you must have files selected in batch model\n"
                                  "and only Compute I(q) or Run DMD selected to create a job.\n"
                                  "Future updates will provide additional functionality." ) );
   }
}

US_Hydrodyn_Cluster::~US_Hydrodyn_Cluster()
{
}

void US_Hydrodyn_Cluster::setupGUI()
{
   int minHeight1 = 30;

   lbl_title = new QLabel( tr( "Create file for cluster jobs" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_target = new QLabel("Grid from experimental data:", this);
   lbl_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_target->setMinimumHeight(minHeight1);
   lbl_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_create_pkg = new QPushButton(tr("Create cluster job package"), this);
   pb_create_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_pkg->setMinimumHeight(minHeight1);
   pb_create_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_pkg, SIGNAL(clicked()), SLOT(create_pkg()));

   pb_add_target = new QPushButton(tr("Add experimental data files"), this);
   pb_add_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_target->setMinimumHeight(minHeight1);
   pb_add_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add_target, SIGNAL(clicked()), SLOT(add_target()));

   pb_clear_target = new QPushButton(tr("Clear experimental data files"), this);
   pb_clear_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_target->setMinimumHeight(minHeight1);
   pb_clear_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_clear_target, SIGNAL(clicked()), SLOT(clear_target()));

   lb_target_files = new QListBox(this, "target files listbox" );
   lb_target_files->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_target_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   lb_target_files->setEnabled(true);
   lb_target_files->setSelectionMode( QListBox::NoSelection );
   
   lb_target_files->setMinimumHeight( minHeight1 * 2 );

   lbl_no_of_jobs = new QLabel( QString(tr( "Number of jobs (maximum %1):" )).arg( selected_files.size() ), this);
   lbl_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_no_of_jobs->setMinimumHeight(minHeight1);
   lbl_no_of_jobs->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_no_of_jobs_qv = new QIntValidator( 1, selected_files.size(), this );
   le_no_of_jobs = new QLineEdit(this, "csv_filename Line Edit");
   le_no_of_jobs->setText( "1" );
   le_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_no_of_jobs->setMinimumWidth(150);
   le_no_of_jobs->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_no_of_jobs->setValidator( le_no_of_jobs_qv );

   lbl_output_name = new QLabel(tr("Output base name (job identifier)"), this);
   lbl_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_output_name->setMinimumHeight(minHeight1);
   lbl_output_name->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_output_name = new QLineEdit(this, "csv_filename Line Edit");
   le_output_name->setText( "job" );
   le_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_output_name->setMinimumWidth(150);
   le_output_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_output_name, SIGNAL( textChanged( const QString &) ), SLOT( update_output_name( const QString & ) ) );

   cb_for_mpi = new QCheckBox(this);
   cb_for_mpi->setText(tr(" Package for parallel job submission"));
   cb_for_mpi->setEnabled(true);
   cb_for_mpi->setChecked(false);
   cb_for_mpi->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_for_mpi->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_for_mpi, SIGNAL( clicked() ), SLOT( for_mpi() ) );

   cb_split_grid = new QCheckBox(this);
   cb_split_grid->setText(tr(" Individual jobs for each grid"));
   cb_split_grid->setEnabled(true);
   cb_split_grid->setChecked(false);
   cb_split_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_split_grid->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_split_grid, SIGNAL( clicked() ), SLOT( split_grid() ) );

   pb_dmd = new QPushButton(tr("DMD settings"), this);
   pb_dmd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_dmd->setMinimumHeight(minHeight1);
   pb_dmd->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect( pb_dmd, SIGNAL( clicked() ), SLOT( dmd() ) );

   pb_advanced = new QPushButton(tr("Advanced options"), this);
   pb_advanced->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_advanced->setMinimumHeight(minHeight1);
   pb_advanced->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_advanced, SIGNAL(clicked()), SLOT(advanced()));

   pb_submit_pkg = new QPushButton(tr("Submit jobs for processing"), this);
   pb_submit_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit_pkg->setMinimumHeight(minHeight1);
   pb_submit_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_submit_pkg, SIGNAL(clicked()), SLOT(submit_pkg()));

   pb_check_status = new QPushButton(tr("Check job status / Retrieve results"), this);
   pb_check_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_check_status->setMinimumHeight(minHeight1);
   pb_check_status->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_check_status, SIGNAL(clicked()), SLOT(check_status()));

   pb_load_results = new QPushButton(tr("Extract results"), this);
   pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_results->setMinimumHeight(minHeight1);
   pb_load_results->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_results, SIGNAL(clicked()), SLOT(load_results()));
   
   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

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

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cluster_config = new QPushButton(tr("Cluster Configuration"), this);
   pb_cluster_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cluster_config->setMinimumHeight(minHeight1);
   pb_cluster_config->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cluster_config, SIGNAL(clicked()), SLOT(config()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QGridLayout *gl_target = new QGridLayout( 0 );
   gl_target->addMultiCellWidget( lbl_target      , 0, 1, 0, 0 );
   gl_target->addWidget         ( pb_add_target   , 0, 1 );
   gl_target->addWidget         ( pb_clear_target , 1, 1 );
   gl_target->addMultiCellWidget( lb_target_files , 0, 1, 2, 2 );

   QHBoxLayout *hbl_no_of_jobs = new QHBoxLayout( 0 );
   hbl_no_of_jobs->addSpacing( 4 );
   hbl_no_of_jobs->addWidget ( lbl_no_of_jobs );
   hbl_no_of_jobs->addSpacing( 4 );
   hbl_no_of_jobs->addWidget ( le_no_of_jobs );
   hbl_no_of_jobs->addSpacing( 4 );

   QHBoxLayout *hbl_output_name = new QHBoxLayout( 0 );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( lbl_output_name );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( le_output_name );
   hbl_output_name->addSpacing( 4 );

   QHBoxLayout *hbl_mpi_etc = new QHBoxLayout( 0 );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( cb_for_mpi );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( cb_split_grid );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( pb_dmd );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( pb_advanced );
   hbl_mpi_etc->addSpacing( 4 );
   

   QHBoxLayout *hbl_create = new QHBoxLayout( 0 );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_create_pkg );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_submit_pkg );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_check_status );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_load_results );
   hbl_create->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cluster_config );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QBoxLayout *vbl_editor_group = new QVBoxLayout(0);
   vbl_editor_group->addWidget(frame);
   vbl_editor_group->addWidget(editor);

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( gl_target );
   background->addSpacing( 4 );
   background->addLayout ( hbl_no_of_jobs );
   background->addSpacing( 4 );
   background->addLayout ( hbl_output_name );
   background->addSpacing( 4 );
   background->addLayout ( hbl_mpi_etc );
   background->addSpacing( 4 );
   background->addLayout ( hbl_create );
   background->addSpacing( 4 );
   background->addLayout ( vbl_editor_group );
   background->addSpacing( 4 );
   background->addLayout ( hbl_bottom );
   background->addSpacing( 4);
}

void US_Hydrodyn_Cluster::cancel()
{
   close();
}

void US_Hydrodyn_Cluster::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_cluster.html");
}

void US_Hydrodyn_Cluster::closeEvent(QCloseEvent *e)
{
   batch_window->cluster_no_of_jobs       = le_no_of_jobs->text();
   QStringList target_files;
   for ( int i = 0; i < lb_target_files->numRows(); i++ )
   {
      target_files << lb_target_files->text( i );
   }
   batch_window->cluster_target_datafiles = target_files;
   batch_window->cluster_output_name      = le_output_name->text();
   batch_window->cluster_for_mpi          = cb_for_mpi->isChecked();
   batch_window->cluster_split_grid       = cb_split_grid->isChecked();
   batch_window->cluster_csv_advanced     = csv_advanced;
   batch_window->cluster_csv_dmd          = csv_dmd;

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster::clear_target()
{
   lb_target_files->clear();
   update_validator();
}

void US_Hydrodyn_Cluster::add_target()
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_target_files->numRows(); i++ )
   {
      existing_items[ lb_target_files->text( i ) ] = true;
   }

   QStringList filenames = QFileDialog::getOpenFileNames(
                                                   "All files (*);;"
                                                   "ssaxs files (*.ssaxs);;"
                                                   "csv files (*.csv);;"
                                                   "int files [crysol] (*.int);;"
                                                   "dat files [foxs / other] (*.dat);;"
                                                   "fit files [crysol] (*.fit);;"
                                                   , ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs"
                                                   , this
                                                   , "open file dialog"
                                                   , "Set files for grid target"
                                                   );

   QStringList add_filenames;

   for ( unsigned int i = 0; i < filenames.size(); i++ )
   {
      if ( !existing_items.count( filenames[ i ] ) )
      {
         add_filenames << filenames[ i ];
      }
   }

   lb_target_files->insertStringList( add_filenames );
   update_validator();
}

void US_Hydrodyn_Cluster::create_pkg()
{
   // check for prior existing jobs in submitted/completed
   if ( dup_in_submitted_or_completed() )
   {
      return;
   }

   QString unimplemented;
   QStringList base_source_files;

   if ( !le_no_of_jobs->text().toUInt() )
   {
      le_no_of_jobs->setText( "1" );
   }
   bool use_extension =  le_no_of_jobs->text().toUInt() != 1;

   // create the output file
   QString filename = 
      le_output_name->text() == QFileInfo( le_output_name->text() ).fileName() ?
      pkg_dir + SLASH + le_output_name->text() :
      le_output_name->text();

   if ( cb_for_mpi->isChecked() )
   {
      QString tar_filename = filename + ".tar";
      if ( QFile::exists( tar_filename ) )
      {
         tar_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( tar_filename, 0, this );
         filename = tar_filename;
         filename.replace( QRegExp( "\\.tar$" ), "" );
         le_output_name->setText( QFileInfo( filename ).dirPath() == pkg_dir ?
                                  QFileInfo( filename ).fileName() : 
                                  filename );
      }
   } else {
      if ( use_extension )
      {
         QString ext  =  "_p1.tgz";
         QString targz_filename = filename + ext;
         if ( QFile::exists( targz_filename ) )
         {
            QString path =  QFileInfo( targz_filename ).dirPath() + SLASH;
            QString name =  QFileInfo( targz_filename ).fileName().replace( QRegExp( "\\_p1.tgz$" ), "" );
            targz_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( 
                                                                         &path, &name, &ext, 0, this );
            filename = targz_filename;
            filename.replace( QRegExp( "\\_p1.tgz$" ), "" );
            le_output_name->setText( QFileInfo( filename ).dirPath() == pkg_dir ?
                                     QFileInfo( filename ).fileName() : 
                                     filename );
         }
      } else {
         QString targz_filename = filename + ".tgz";
         if ( QFile::exists( targz_filename ) )
         {
            targz_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( targz_filename, 0, this );
            filename = targz_filename;
            filename.replace( QRegExp( "\\.tgz$" ), "" );
            le_output_name->setText( QFileInfo( filename ).dirPath() == pkg_dir ?
                                     QFileInfo( filename ).fileName() : 
                                     filename );
         }
      }
   } 

   if ( dup_in_submitted_or_completed() )
   {
      return;
   }


   QString base_dir = QFileInfo( filename ).dirPath();
   if ( !QDir::setCurrent( base_dir ) )
   {
      editor_msg( "red", QString( tr( "Can not find output directory %1\n" ) ).arg( base_dir ) );
      return;
   }

   {
      QFileInfo qfi( le_output_name->text() );
      if ( qfi.exists() && !qfi.isFile() )
      {
         editor_msg( "red", QString( tr( "Output file %1 already exists and is not a regular file\n" ) ).arg( le_output_name->text() ) );
         return;
      }
   }

   editor_msg( "black", QString( tr( "Output directory is %1\n" ) ).arg( base_dir ) );

   QString base = 
      "# us_saxs_cmds_t iq controlfile\n"
      "# blank lines ok, format token <params>\n"
      "\n";

   base += 
      QString( "ResidueFile     %1\n" ).arg( QFileInfo( ((US_Hydrodyn *)us_hydrodyn)->lbl_table->text() ).fileName() );
   base_source_files << ((US_Hydrodyn *)us_hydrodyn)->lbl_table->text();

   base += 
      QString( "AtomFile        %1\n" ).arg( QFileInfo( our_saxs_options->default_atom_filename ).fileName() );
   base_source_files << our_saxs_options->default_atom_filename;
   base += 
      QString( "HybridFile      %1\n" ).arg( QFileInfo( our_saxs_options->default_hybrid_filename ).fileName() );
   base_source_files << our_saxs_options->default_hybrid_filename;
   base += 
      QString( "SaxsFile        %1\n" ).arg( QFileInfo( our_saxs_options->default_saxs_filename ).fileName() );
   base_source_files << our_saxs_options->default_saxs_filename;
   if ( batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
   {
      base += 
         QString( "HydrationFile   %1\n" ).arg( QFileInfo( our_saxs_options->default_rotamer_filename ).fileName() );
      base_source_files << our_saxs_options->default_rotamer_filename;
   }

   base += 
      QString( "\n%1\n" ).arg( our_saxs_options->saxs_sans ? "Sans" : "Saxs" );
   if ( our_saxs_options->saxs_sans )
   {
      unimplemented += "SANS methods currently unimplemented\n";
   }

   QString iqmethod = "";
   if ( our_saxs_options->saxs_iq_native_debye || our_saxs_options->sans_iq_native_debye )
   {
      iqmethod = "db";
   }
   if ( our_saxs_options->saxs_iq_native_hybrid || our_saxs_options->sans_iq_native_hybrid )
   {
      iqmethod = "hy";
   }
   if ( our_saxs_options->saxs_iq_native_hybrid2 || our_saxs_options->sans_iq_native_hybrid2 )
   {
      iqmethod = "h2";
   }
   if ( our_saxs_options->saxs_iq_native_hybrid3 || our_saxs_options->sans_iq_native_hybrid3 )
   {
      iqmethod = "h3";
   }
   if ( our_saxs_options->saxs_iq_hybrid_adaptive &&
        ( our_saxs_options->saxs_iq_native_hybrid || our_saxs_options->sans_iq_native_hybrid ||
          our_saxs_options->saxs_iq_native_hybrid2 || our_saxs_options->sans_iq_native_hybrid2 ||
          our_saxs_options->saxs_iq_native_hybrid3 || our_saxs_options->sans_iq_native_hybrid3 ) )
   {
      iqmethod += "a";
   }

   if ( our_saxs_options->saxs_iq_native_fast || our_saxs_options->sans_iq_native_fast )
   {
      iqmethod = "fd";
   }

   if ( our_saxs_options->saxs_sans )
   {
      if ( our_saxs_options->sans_iq_cryson )
      {
         unimplemented += "cryson method currently unimplemented\n";
         iqmethod = "cryson";
      }
   } else {
      if ( our_saxs_options->saxs_iq_foxs )
      {
         if ( batch_window->cb_mm_all->isChecked() )
         {
            QMessageBox::message( tr( "Please note:" ), 
                                  tr( "You have selected to process all models in multi-model PDBs with the FoXS external I(q) method.\n"
                                      "If you are including multi-model PDBs, a single composite curve will be produced.\n"
                                      "If you wish curves for individual models, either split the multi model PDB\n"
                                      "or use the \"fast\" Iq method.\n"
                                      "You can split the multi-model PDB into individual files using the PDB Editor / Split button." ) );
         }
         iqmethod = "foxs";
      }
      if ( our_saxs_options->saxs_iq_crysol )
      {
         if ( batch_window->cb_mm_all->isChecked() )
         {
            QMessageBox::message( tr( "Please note:" ), 
                                  tr( "You have selected to process all models in multi-model PDBs with the CRYSOL external I(q) method.\n"
                                      "If you are including multi-model PDBs, only the first model will actually be computed by CRYSOL.\n"
                                      "If you wish curves for individual models, split the multi model PDB into individual PDBs.\n"
                                      "You can split the multi-model PDB into individual files using the PDB Editor / Split button." ) );
         }
         iqmethod = "crysol";
      }
   }
   
   base += 
      QString( "IqMethod        %1\n" ).arg( iqmethod );

   base += 
      QString( "FdBinSize       %1\n" ).arg( our_saxs_options->fast_bin_size );
   base += 
      QString( "FdModulation    %1\n" ).arg( our_saxs_options->fast_modulation );
   base += 
      QString( "HyPoints        %1\n" ).arg( our_saxs_options->hybrid2_q_points );
   base += 
      QString( "CrysolHarm      %1\n" ).arg( our_saxs_options->crysol_max_harmonics );
   base += 
      QString( "CrysolGrid      %1\n" ).arg( our_saxs_options->crysol_fibonacci_grid_order );
   base += 
      QString( "CrysolChs       %1\n" ).arg( our_saxs_options->crysol_hydration_shell_contrast );
   base += 
      QString( "WaterEDensity   %1\n" ).arg( our_saxs_options->water_e_density );
   base += 
      QString( "AsaHydrateThresh      %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.hydrate_threshold );
   base += 
      QString( "AsaThreshPct          %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.threshold_percent );
   base += 
      QString( "AsaHydrateProbeRadius %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.hydrate_probe_radius );
   base += 
      QString( "AsaStep               %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.asab1_step );
   base += 
      QString( "AsaCalculation        %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->asa.calculation ? 1 : 0 );

   if ( lb_target_files->numRows() )
   {
      if ( lb_target_files->numRows() == 1 || !cb_split_grid->isChecked() )
      {
         base += 
            QString( "ExperimentGrid  %1\n" ).arg( QFileInfo( lb_target_files->text( 0 ) ).fileName() );
         base_source_files << lb_target_files->text( 0 );
         for ( int i = 1; i < lb_target_files->numRows(); i++ )
         {
            base += 
               QString( "AdditionalExperimentGrid  %1\n" ).arg( QFileInfo( lb_target_files->text( i ) ).fileName() );
            base_source_files << lb_target_files->text( i );
         }
      }
   } else {
      base += 
         QString( "StartQ          %1\n" ).arg( our_saxs_options->start_q );
      base += 
         QString( "EndQ            %1\n" ).arg( our_saxs_options->end_q );
      base += 
         QString( "DeltaQ          %1\n" ).arg( our_saxs_options->delta_q );

      cout << QString( "startq %1 endq %2 deltaq %3\n")
         .arg( our_saxs_options->start_q )
         .arg( our_saxs_options->end_q )
         .arg( our_saxs_options->delta_q );
      if ( our_saxs_options->delta_q == 0e0 ||
           our_saxs_options->start_q == our_saxs_options->end_q )
      {
         unimplemented += QString("Error: saxs q range is empty");
      }
   }

   if ( batch_window->cb_mm_all->isChecked() )
   {
      base += "PDBAllModels\n";
   }

   if ( batch_window->cb_dmd->isChecked() )
   {
      base += dmd_base_addition( base_source_files );
   }
      
   if ( !unimplemented.isEmpty() )
   {
      editor_msg( "red", QString( "Can not create job files:\n%1" ).arg( unimplemented ) );
      return;
   }

   base += QString( "Output          %1\n" )
      .arg( batch_window->cb_csv_saxs->isChecked() ? "csv" : "ssaxs" );

   if ( batch_window->cb_csv_saxs->isChecked() )
   {
      base += QString( "OutputFile      %1\n" ).arg( le_output_name->text() );
   }

   // cout << base;

   // now loop through selected, jumping every le_no_of_jobs->text()->toUInt()
   // for a new extension
   
   QString      out = base;
   unsigned int write_count = 0;
   unsigned int extension_count = selected_files.size();
   unsigned int extension_count_length = QString("%1").arg( extension_count ).length();

   if ( !copy_files_to_pkg_dir( base_source_files ) )
   {
      editor_msg( "red", errormsg );
      return;
   }

   QStringList source_files;
   QStringList dest_files;
   QStringList qsl_advanced;

   if ( any_advanced() )
   {
      editor_msg( "blue", tr( "Note: using Advanced Options" ) );
      qsl_advanced = advanced_addition();
   }

   unsigned int multi_grid_multiplier = 
      cb_split_grid->isChecked() && lb_target_files->numRows() > 1 ?
      lb_target_files->numRows() : 1;

   unsigned int advanced_multiplier =
      qsl_advanced.size() ? qsl_advanced.size() : 1;

   unsigned int max_no_of_jobs = 
      (unsigned int) ceil( ( selected_files.size() * multi_grid_multiplier * advanced_multiplier )
                           / (double) le_no_of_jobs->text().toUInt() );
   cout << "max jobs per " << max_no_of_jobs << endl;

   map < QString, bool > already_added;

   for ( unsigned int this_i = 0; this_i < selected_files.size() * multi_grid_multiplier * advanced_multiplier; this_i++ )
   {
      unsigned int i    = this_i % selected_files.size();
      if ( multi_grid_multiplier > 1 )
      {
         unsigned int grid = ( this_i / selected_files.size() ) % multi_grid_multiplier;
         out += QString( "ExperimentGrid  %1\n" ).arg( QFileInfo( lb_target_files->text( grid ) ).fileName() );
         if ( !already_added.count( lb_target_files->text( grid ) ) )
         {
            already_added[ lb_target_files->text( grid ) ] = true;
            source_files << lb_target_files->text( grid );
         }
      }

      unsigned int advanced_pos = 0;
      if ( advanced_multiplier > 1 )
      {
         advanced_pos = this_i / ( selected_files.size() * multi_grid_multiplier );
      }

      cout << QString( "this_i %1 grid %2 advanced_pos %3\n" )
         .arg( this_i )
         .arg( ( this_i / selected_files.size() ) % multi_grid_multiplier )
         .arg( advanced_pos );

      out += QString( "InputFile       %1\n" ).arg( QFileInfo( selected_files[ i ] ).fileName() );
      QString use_file_name = QFileInfo( selected_files[ i ] ).fileName();
      QString use_output_name = QFileInfo( selected_files[ i ] ).baseName();
      if ( !batch_window->cb_dmd->isChecked()
           && batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
      {
         out += "Hydrate\n";
         use_output_name += "-h";
      }
      if ( !batch_window->cb_csv_saxs->isChecked() )
      {
         out += QString( "OutputFile      %1\n" ).arg( use_output_name );
      }
      if ( !already_added.count( selected_files[ i ] ) )
      {
         already_added[ selected_files[ i ] ] = true;
         source_files << selected_files[ i ];
      }
      if ( any_advanced() )
      {
         // out += advanced_addition( use_output_name );
         out += qsl_advanced[ advanced_pos ];
      } else {
         if ( batch_window->cb_dmd->isChecked() )
         {
            out += dmd_file_addition( use_file_name, use_output_name );
         } else {
            out += "Process\n";
         }
      }
      if ( !( ( this_i + 1 ) % max_no_of_jobs ) )
      {
         write_count++;
         QString ext = "";
         if ( use_extension )
         {
            ext = QString("%1").arg( write_count );
            while ( ext.length() < extension_count_length )
            {
               ext = "0" + ext;
            }
         }
         QString use_file = QString( "%1%2" ).arg( filename ).arg( use_extension ? QString("_p%1").arg( ext ) : "" );
         // if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite && QFile::exists( use_file ) )
         // {
         // use_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_file, 0, this );
         // }
         cout << use_file << endl;
         QFile f( use_file );
         if ( f.open( IO_WriteOnly ) )
         {
            QTextStream ts( &f );
            ts << out;
            ts << QString( "TgzOutput       %1_out.tgz\n" ).arg( le_output_name->text() + 
                                                                 ( use_extension ? QString("_p%1").arg( ext ) : "" ) );
            f.close();
            editor_msg( "dark gray", QString("Created: %1").arg( use_file ) );
            qApp->processEvents();
         }

         // copy ne files to base_dir
         if ( !copy_files_to_pkg_dir( source_files ) )
         {
            editor_msg( "red", errormsg );
            return;
         }

         // build tar archive
         QStringList list;
         QStringList to_tar_list;
         QStringList remove_file_list;
         to_tar_list << QFileInfo( use_file ).fileName();
         for ( unsigned int i = 0; i <  base_source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
         }
         for ( unsigned int i = 0; i < source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( source_files[ i ] ).fileName();
            remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
         }
         remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();
         US_Tar ust;
         QString tar_name = use_file + ".tar";
         int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
         cout << "tar_name:" << tar_name << endl;
         cout << "to tar:\n" << to_tar_list.join("\n") << endl;

         if ( result != TAR_OK )
         {
            editor_msg( "red" , QString( tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
            return;
         }
         editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );
         US_Gzip usg;
         result = usg.gzip( tar_name );
         if ( result != GZIP_OK )
         {
            editor_msg( "red" , QString( tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
            return;
         }
         QDir qd;
         QString use_targz_filename = tar_name;
         use_targz_filename.replace(QRegExp("\\.tar$"), ".tgz" );
         qd.remove( use_targz_filename );
         if ( !qd.rename( QFileInfo( tar_name + ".gz" ).fileName(), QFileInfo( use_targz_filename ).fileName() ) )
         {
            editor_msg( "red", QString("Error renaming %1 to %2")
                        .arg( QFileInfo( tar_name + ".gz" ).fileName() )
                        .arg( QFileInfo( use_targz_filename ).fileName() ) );
         }

         dest_files << use_targz_filename;

         editor_msg( "dark gray", QString("Gzipped: %1").arg( use_targz_filename ) );
         // clean up droppings
         if ( !remove_files( remove_file_list ) )
         {
            return;
         }

         source_files .clear();
         already_added.clear();

         out = base;
      }
   }
   if ( source_files.size() )
   {
      write_count++;
      QString ext = "";
      if ( use_extension )
      {
         ext = QString("%1").arg( write_count );
         while ( ext.length() < extension_count_length )
         {
            ext = "0" + ext;
         }
      }
      QString use_file = QString( "%1%2" ).arg( filename ).arg( use_extension ? QString("_p%1").arg( ext ) : "" );
      // if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite && QFile::exists( use_file ) )
      // {
      // use_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_file, 0, this );
      // }
      cout << use_file << endl;
      QFile f( use_file );
      if ( f.open( IO_WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << out;
         ts << QString( "TgzOutput       %1_out.tgz\n" ).arg( le_output_name->text() + 
                                                              ( use_extension ? QString("_p%1").arg( ext ) : "" ) );
         f.close();
         editor_msg( "dark gray", QString("Created: %1").arg( use_file ) );
      }
      
      // copy ne files to base_dir
      if ( !copy_files_to_pkg_dir( source_files ) )
      {
         editor_msg( "red", errormsg );
         return;
      }

      // build tar archive
      QStringList qsl;
      QStringList to_tar_list;
      QStringList remove_file_list;
      to_tar_list << QFileInfo( use_file ).fileName();
      for ( unsigned int i = 0; i <  base_source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
      }
      for ( unsigned int i = 0; i < source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( source_files[ i ] ).fileName();
         remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
      }
      remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();
      US_Tar ust;
      QString tar_name = use_file + ".tar";
      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &qsl );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;
      
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
         return;
      }
      editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );
      US_Gzip usg;
      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
         return;
      }
      QDir qd;
      QString use_targz_filename = tar_name;
      use_targz_filename.replace(QRegExp("\\.tar$"), ".tgz" );
      qd.remove( use_targz_filename );
      if ( !qd.rename( QFileInfo( tar_name + ".gz" ).fileName(), QFileInfo( use_targz_filename ).fileName() ) )
      {
         editor_msg( "red", QString("Error renaming %1 to %2")
                     .arg( QFileInfo( tar_name + ".gz" ).fileName() )
                     .arg( QFileInfo( use_targz_filename ).fileName() ) );
      }
      
      dest_files << use_targz_filename;
      
      editor_msg( "dark gray", QString("Gzipped: %1").arg( use_targz_filename ) );
      // clean up droppings
      if ( !remove_files( remove_file_list ) )
      {
         return;
      }
      
      
      source_files.clear();
      
      out = base;
   }

   QStringList base_remove_file_list;
   for ( unsigned int i = 0; i < base_source_files.size(); i++ )
   {
      base_remove_file_list << pkg_dir + SLASH + QFileInfo( base_source_files[ i ] ).fileName();
   }
   remove_files( base_remove_file_list );
   cout << "written:" << write_count << endl;
   if ( write_count != le_no_of_jobs->text().toUInt() )
   {
      editor_msg( "dark red", QString( tr( "Notice: the actually number of jobs created (%1) is less than requested (%2)\n"
                                           "This is due to the fact that the selected files were evenly distributed among the jobs" ) ).arg( write_count ).arg( le_no_of_jobs->text().toUInt() ) );
   }
   if ( cb_for_mpi->isChecked() )
   {
      QString tarout = le_output_name->text() + ".tar";
      US_Tar ust;
      QStringList qsl;
      QStringList local_dest_files;
      for ( unsigned int i = 0; i < dest_files.size(); i++ )
      {
         local_dest_files << QFileInfo( dest_files[ i ] ).fileName();
      }

      int result = ust.create( QFileInfo( tarout ).filePath(), local_dest_files, &qsl );
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( tr( "Error: Problem creating tar archive %1: %2") ).arg( tarout ).arg( ust.explain( result ) ) );
         return;
      }
      if ( !remove_files( dest_files ) )
      {
         return;
      }
      editor_msg( "blue", QString( tr( "Package: %1 created" ) ).arg( tarout ) );
   } else {
      editor_msg( "blue", 
                  dest_files
                  .gres( QRegExp( "^" ), tr( "Package: " ) )
                  .gres( QRegExp( "$" ), tr( " created" ) )
                  .join( "\n" ) );
   }
   editor_msg( "black", tr( "Package complete" ) );
}

void US_Hydrodyn_Cluster::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Cluster::update_font()
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

void US_Hydrodyn_Cluster::save()
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

void US_Hydrodyn_Cluster::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
   editor->scrollToBottom();
}

bool US_Hydrodyn_Cluster::remove_files( QStringList &filenames )
{
   errormsg = "";
   for ( unsigned int i = 0; i < filenames.size(); i++ )
   {
      if ( !QFile::remove( filenames[ i ] ) )
      {
         editor_msg("red", QString(tr("Notice: remove of temporary file %1 failed")).arg( filenames[ i ] ));
         return false;
      } else {
         // editor_msg("dark gray", QString(tr("Removed temporary file %1")).arg( filenames[ i ] ));
      } 
   }
   return true;
}

bool US_Hydrodyn_Cluster::copy_files_to_pkg_dir( QStringList &filenames )
{
   errormsg = "";
   US_File_Util ufu;
   if ( !ufu.copy( filenames, pkg_dir, true ) )
   {
      errormsg = ufu.errormsg;
      return false;
   }
   return true;
}

void US_Hydrodyn_Cluster::submit_pkg()
{
   // open cluster directory and find *_out.tgz
   // process and make unified csvs etc and load into standard somo/saxs directory
   if ( !read_config() ||
        !cluster_systems.size() )
   {
      config();
      return;
   }

   US_Hydrodyn_Cluster_Submit *hcs = 
      new US_Hydrodyn_Cluster_Submit(
                                     us_hydrodyn,
                                     this );
   if ( hcs->files.size() )
   {
      hcs->exec();
   }
   delete hcs;
}

void US_Hydrodyn_Cluster::load_results()
{
   // open cluster directory and find *_out.tgz
   // process and make unified csvs etc and load into standard somo/saxs directory
   US_Hydrodyn_Cluster_Results *hcr = 
      new US_Hydrodyn_Cluster_Results(
                                      us_hydrodyn,
                                      this );
   if ( hcr->files.size() )
   {
      hcr->exec();
   }
   delete hcr;
}

void US_Hydrodyn_Cluster::update_output_name( const QString &cqs )
{
   if ( cqs.contains( QRegExp( "^nsa_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^nsa_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( "_out", false ) )
   {
      QString qs = cqs;
      qs.replace( "_out", "", false );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "(\\s+|\\/|\\\\)" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "(\\s+|\\/|\\\\)" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^(tmp|TMP)_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^(tmp|TMP)_" ), "" );
      le_output_name->setText( qs );
   }
}

void US_Hydrodyn_Cluster::config()
{
   read_config();

   US_Hydrodyn_Cluster_Config *hcc = 
      new US_Hydrodyn_Cluster_Config(
                                     us_hydrodyn,
                                     this );
   hcc->exec();
}

void US_Hydrodyn_Cluster::check_status()
{
   read_config();

   US_Hydrodyn_Cluster_Status *hcs = 
      new US_Hydrodyn_Cluster_Status(
                                     us_hydrodyn,
                                     this );
   if ( hcs->files.size() )
   {
      hcs->exec();
   }
   delete hcs;
}

bool US_Hydrodyn_Cluster::dup_in_submitted_or_completed()
{
   if ( submitted_jobs.count( le_output_name->text() ) )
   {
      QMessageBox::message( tr( "Please note:" ), 
                            tr( "There is a previously submitted job with the same name\n"
                                "Please change the output base name (job identifier) to a unique value\n"
                                "or cancel the submitted job\n") );
      return true;
   }
   if ( completed_jobs.count( le_output_name->text() ) ||
        results_jobs.count( le_output_name->text() ) )
   {
      QMessageBox::message( tr( "Please note:" ), 
                            tr( "There is a previously completed job with the same name\n"
                                "Please change the output base name (job identifier) to a unique value\n"
                                "or remove the completed job\n" ) );
      return true;
   }
   return false;
}

void US_Hydrodyn_Cluster::for_mpi()
{
   update_validator();
   if ( cb_for_mpi->isChecked() )
   {
      le_no_of_jobs->setText( QString( "%1" )
                              .arg( cb_split_grid->isChecked() &&
                                    lb_target_files->numRows() ?
                                    selected_files.size() * lb_target_files->numRows() :
                                    selected_files.size() ) );
   } 
}

void US_Hydrodyn_Cluster::update_validator()
{
   le_no_of_jobs->setValidator( 0 );
   if ( le_no_of_jobs_qv )
   {
      delete le_no_of_jobs_qv;
   }

   unsigned int max_jobs = 
      ( cb_split_grid->isChecked() && lb_target_files->numRows() ) ?
      selected_files.size() * lb_target_files->numRows() : selected_files.size();
                           
   max_jobs *= advanced_addition().size();

   le_no_of_jobs_qv = new QIntValidator( 1, 
                                         max_jobs,
                                         this
                                         );
   le_no_of_jobs ->setValidator( le_no_of_jobs_qv );
   lbl_no_of_jobs->setText( QString(tr( "Number of jobs (maximum %1):" )).arg( max_jobs ) );
   if ( le_no_of_jobs->text().toUInt() > max_jobs )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( max_jobs ) );
   }
}

void US_Hydrodyn_Cluster::split_grid()
{
   update_validator();
   if ( cb_for_mpi->isChecked() && cb_split_grid->isChecked() && lb_target_files->numRows() )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( selected_files.size() * lb_target_files->numRows() ) );
   }
}

void US_Hydrodyn_Cluster::advanced()
{
   US_Hydrodyn_Cluster_Advanced *hca = 
      new US_Hydrodyn_Cluster_Advanced(
                                       csv_advanced,
                                       us_hydrodyn,
                                       this );
   hca->exec();
   delete hca;
   update_validator();
}

void US_Hydrodyn_Cluster::dmd()
{
   US_Hydrodyn_Cluster_Dmd *hcd = 
      new US_Hydrodyn_Cluster_Dmd(
                                  csv_dmd,
                                  us_hydrodyn,
                                  this );
   hcd->exec();
   delete hcd;
   update_enables();
}

bool US_Hydrodyn_Cluster::any_advanced()
{
   // cout << "any_advanced()\n";
   if ( !csv_advanced.data.size() )
   {
      // cout << "any_advanced() false: no data\n";
      return false;
   }

   for ( unsigned int i = 0; i < csv_advanced.data.size(); i++ )
   {
      if ( csv_advanced.data[ i ].size() > 1 &&
           csv_advanced.data[ i ][ 1 ] == "Y" )
      {
         // cout << "any_advanced() true: found active\n";
         return true;
      }
   }
   // cout << "any_advanced() false: no active\n";
   return false;
}

QString US_Hydrodyn_Cluster::advanced_addition_methods()
{
   QString out;
   QString adaptive = "";
   if ( our_saxs_options->saxs_iq_hybrid_adaptive &&
        ( our_saxs_options->saxs_iq_native_hybrid || our_saxs_options->sans_iq_native_hybrid ||
          our_saxs_options->saxs_iq_native_hybrid2 || our_saxs_options->sans_iq_native_hybrid2 ||
          our_saxs_options->saxs_iq_native_hybrid3 || our_saxs_options->sans_iq_native_hybrid3 ) )
   {
      adaptive = "a";
   }

   bool any_methods = false;

   for ( unsigned int i = 0; i < csv_advanced.data.size(); i++ )
   {
      if ( csv_advanced.data[ i ].size() > 1 &&
           csv_advanced.data[ i ][ 1 ] == "Y" )
      {
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Full Debye" )
         {
            out += 
               "IqMethod        db\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Hybrid" )
         {
            out += 
               "IqMethod        hy" + adaptive + "\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Hybrid2" )
         {
            out += 
               "IqMethod        h2" + adaptive + "\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Hybrid3" )
         {
            out += 
               "IqMethod        h3" + adaptive + "\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Fast" )
         {
            out += 
               "IqMethod        fd\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) FoXS" )
         {
            out += 
               "IqMethod        foxs\n"
               "Process\n";
            any_methods = true;
         }
         if ( csv_advanced.data[ i ][ 0 ] == "I(q) Crysol" )
         {
            out += 
               "IqMethod        crysol\n"
               "Process\n";
            any_methods = true;
         }
      }
   }
   if ( !any_methods )
   {
      out += "Process\n";
   }

   return out;
}

QStringList US_Hydrodyn_Cluster::advanced_addition()
{
   QStringList qsl_out;

   map < unsigned int, double >       starts;
   map < unsigned int, double >       ends;
   map < unsigned int, unsigned int > points;
   map < unsigned int, double >       increments;
   map < unsigned int, unsigned int > offsets;
   map < unsigned int, unsigned int > next_offsets;
   map < unsigned int, double >       run_value;

   unsigned int current_offset = 0;

   for ( unsigned int i = 0; i < csv_advanced.data.size(); i++ )
   {
      for ( unsigned int i = 0; i < csv_advanced.data.size(); i++ )
      {
         if ( csv_advanced.data[ i ].size() > 4 &&
              csv_advanced.data[ i ][ 1 ] == "Y" &&
              !csv_advanced.data[ i ][ 2 ].isEmpty() )

         {
            starts    [i]   =  csv_advanced.data[ i ][ 2 ].toDouble();
            ends      [i]   =  csv_advanced.data[ i ][ 3 ].toDouble();
            points    [i]   =  csv_advanced.data[ i ][ 4 ].toUInt();
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
   }
   
   unsigned int total_points = 1;
   for ( map < unsigned int, unsigned int >::iterator it = points.begin();
         it != points.end();
         it++ )
   {
      total_points *= it->second;
   }

   // linearization of an arbitrary number of loops

   for ( unsigned int i = 0; i < total_points; i++ )
   {
      QString out;
      unsigned int pos = i;
      QString msg = "";
      QString tag;

      for ( map < unsigned int, unsigned int >::iterator it = points.begin();
            it != points.end();
            it++ )
      {
         run_value[ it->first ] = starts[ it->first ] + ( pos % it->second ) * increments[ it->first ];
         pos /= it->second;
         bool ok = false;
         tag += "_";
         if ( csv_advanced.data[ it->first ][ 0 ] == "Buffer electron density" )
         {
            tag += "bed";
            out += "WaterEDensity   ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "Scaling excluded volume" )
         {
            tag += "evs";
            out += "ScaleExclVol    ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "SWH excluded volume" )
         {
            tag += "swh";
            out += "SwhExclVol      ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "I(q) Crysol: contrast of hydration shell" )
         {
            tag += "hs";
            out += "CrysolChs       ";
            ok = true;
         }
         if ( ok )
         {
            tag += QString( "%1" ).arg( run_value[ it->first ] ).replace( ".", "_" );
            out += QString( "%1\n" ).arg( run_value[ it->first ] );
         }
      }

      if ( !tag.isEmpty() )
      {
         out += "Tag             " + tag + "\n";
      }
      out += advanced_addition_methods();
      qsl_out << out;
   }
   return qsl_out;
}

QString US_Hydrodyn_Cluster::dmd_base_addition( QStringList &base_source_files )
{
   QStringList files;
   QString dmd_base_dir = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "dmd" + SLASH;

   files 
      << "DMD_NA_allatom.par"
      << "PRO_RNA_MOL.DMDParam"
      << "allatom.par"
      << "medutop.pro";

   QString out;
   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      out += QString( "DMDSupportFile  %1\n" )
         .arg( files[ i ] );
   }

   files.gres( QRegExp( "^" ), dmd_base_dir );
   for ( unsigned int i = 0; i < files.size(); i++ )
   {
      base_source_files << files[ i ];
   }

   return out;
}

QString US_Hydrodyn_Cluster::dmd_file_addition( QString inputfile, QString /* outputfile */ )
{
   QString out;

   // find matching files in csv_dmd and compare with selected files
   // for current file, create DMDTime/Temp/Relax/Run commands
   
   vector < unsigned int > active_csv_rows;
   bool ok = false;
   for ( unsigned int i = 0; i < csv_dmd.prepended_names.size(); i++ )
   {
      if ( inputfile == csv_dmd.prepended_names[ i ] &&
           csv_dmd.data[ i ].size() > 5 &&
           csv_dmd.data[ i ][ 1 ] == "Y" )
      {
         active_csv_rows.push_back( i );
         ok = true;
      }
   }
   if ( !ok )
   {
      return "";
   }

   // we should probably parameterize this, maybe system dependent
   out += "DMDBoxSpacing   +10\n";

   out += 
      "DMDStripPdb\n"
      "DMDFindSS\n"
      "DMDPrepare\n";

   for ( unsigned int j = 0; j < active_csv_rows.size(); j++ )
   {
      unsigned int i = active_csv_rows[ j ];

      bool run_ok = true;
      bool run_is_on = false;
      
      bool relax_ok = true;
      bool relax_is_on =
         ( csv_dmd.data[ i ][ 3 ].toFloat() > 0 ||
           csv_dmd.data[ i ][ 5 ].toFloat() > 0 );
               
      if ( relax_is_on && csv_dmd.data[ i ][ 2 ].toFloat() <= 0 )
      {
         relax_ok = false;
      }
      if ( relax_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
      {
         relax_ok = false;
      }
      if ( csv_dmd.data[ i ][ 5 ].toFloat() <= 0 &&
           csv_dmd.data[ i ].size() > 9 &&
           csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
      {
         relax_ok = false;
      }
      if ( csv_dmd.data[ i ].size() > 9 )
      {
         run_ok = true;
         run_is_on =
            ( csv_dmd.data[ i ][ 7 ].toFloat() > 0 ||
              csv_dmd.data[ i ][ 9 ].toFloat() > 0 );
                  
         if ( run_is_on && csv_dmd.data[ i ][ 6 ].toFloat() <= 0 )
         {
            run_ok = false;
         }
         if ( run_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
         {
            run_ok = false;
         }
         if ( run_is_on && csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
         {
            run_ok = false;
         }
      }
      if ( relax_is_on && relax_ok )
      {
         out += QString( "DMDTime         %1\n"
                         "DMDTemp         %2\n" )
            .arg( csv_dmd.data[ i ][ 3 ] )
            .arg( csv_dmd.data[ i ][ 2 ] );
         if ( csv_dmd.data[ i ][ 4 ].toUInt() )
         {
            out += QString( "DMDTimeStep     %1\n" ).arg( csv_dmd.data[ i ][ 4 ] );
         }
         out += "DMDRun          relax\n";
      }
      if ( run_is_on && run_ok )
      {
         out += QString( "DMDTime         %1\n"
                         "DMDTemp         %2\n" )
            .arg( csv_dmd.data[ i ][ 7 ] )
            .arg( csv_dmd.data[ i ][ 6 ] );
         if ( csv_dmd.data[ i ][ 8 ].toUInt() )
         {
            out += QString( "DMDTimeStep     %1\n" ).arg( csv_dmd.data[ i ][ 8 ] );
         }
         out += "DMDRun          equi\n";
      }
      if ( batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
      {
         out += "Hydrate\n";
      }
      if ( batch_window->cb_iqq->isChecked() )
      {
         out += "Process\n";
      }
   }
   return out;
}

bool US_Hydrodyn_Cluster::validate_csv_dmd( unsigned int &number_active )
{
   // make sure syncs up with selected files & 
   errormsg = "";
   noticemsg = "";
   number_active = 0;
   map < QString, bool > selected_map;
   map < QString, bool > present_map;
   
   // build map of selected files:

   for ( unsigned int i = 0; i < selected_files.size(); i++ )
   {
      selected_map[ QFileInfo( selected_files[ i ] ).fileName() ] = true;
   }

   // see what's in csv_dmd
   for ( unsigned int i = 0; i < csv_dmd.prepended_names.size(); i++ )
   {
      present_map[ csv_dmd.prepended_names[ i ] ] = true;
      if ( !selected_map.count( csv_dmd.prepended_names[ i ] ) )
      {
         // somethings present that isn't selected
         if ( csv_dmd.data[ i ].size() > 1 &&
              csv_dmd.data[ i ][ 1 ] == "Y" )
         {
            errormsg += QString( tr( "Error: %1 is marked as Active in the DMD settings, "
                                     "but the file is not present in the batch selected files\n" ) )
               .arg( csv_dmd.prepended_names[ i ] );
         } else {
            noticemsg += QString( tr( "Notice: %1 is in the DMD settings and is not Active, "
                                      "but the file is not present in the batch selected files\n" ) )
               .arg( csv_dmd.prepended_names[ i ] );
         }
      } else {
         if ( csv_dmd.data[ i ].size() > 1 &&
              csv_dmd.data[ i ][ 1 ] == "Y" )
         {
            number_active++;
            if ( csv_dmd.data[ i ].size() < 6 )
            {
               errormsg += QString( tr( "Error: %1 is marked as Active but has insufficient DMD settings, "
                                        "at least a Relax temp, time & output count must be specified\n" ) )
                  .arg( csv_dmd.prepended_names[ i ] );
            } else {
               bool run_ok = true;
               bool run_is_on = false;

               bool relax_ok = true;
               bool relax_is_on =
                  ( csv_dmd.data[ i ][ 3 ].toFloat() > 0 ||
                    csv_dmd.data[ i ][ 5 ].toFloat() > 0 );
               
               if ( relax_is_on && csv_dmd.data[ i ][ 2 ].toFloat() <= 0 )
               {
                  errormsg += QString( tr( "Error: %1 is Active and does not have a positive Relax temp\n" ) )
                     .arg( csv_dmd.prepended_names[ i ] );
                  relax_ok = false;
               }
               if ( relax_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
               {
                  errormsg += QString( tr( "Error: %1 is Active and does not have a positive Relax time\n" ) )
                     .arg( csv_dmd.prepended_names[ i ] );
                  relax_ok = false;
               }
               if ( csv_dmd.data[ i ][ 5 ].toFloat() <= 0 &&
                    csv_dmd.data[ i ].size() > 9 &&
                    csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
               {
                  errormsg += QString( tr( "Error: %1 is Active and the Relax output count and the Run output count are not positive\n" ) )
                     .arg( csv_dmd.prepended_names[ i ] );
                  relax_ok = false;
               }
               if ( csv_dmd.data[ i ].size() > 9 )
               {
                  run_ok = true;
                  run_is_on =
                     ( csv_dmd.data[ i ][ 7 ].toFloat() > 0 ||
                       csv_dmd.data[ i ][ 9 ].toFloat() > 0 );
                  
                  if ( run_is_on && csv_dmd.data[ i ][ 6 ].toFloat() <= 0 )
                  {
                     errormsg += QString( tr( "Error: %1 is Active and does not have a positive Run temp\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
                  if ( run_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
                  {
                     errormsg += QString( tr( "Error: %1 is Active and does not have a positive Run time\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
                  if ( run_is_on && csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
                  {
                     errormsg += QString( tr( "Error: %1 is Active and the Run output count is not positive\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
               }
               if ( !run_ok &&
                    !relax_ok )
               {
                  errormsg += QString( tr( "Error: %1 is Active neither the Relax or Run is ok\n" ) )
                                       .arg( csv_dmd.prepended_names[ i ] );
               }
            }
         }
      }
   }

   // now go thru selected and find what is not not present 
   for ( map < QString, bool >::iterator it = selected_map.begin();
         it != selected_map.end();
         it++ )
   {
      if ( !present_map.count( it->first ) )
      {
         noticemsg += QString( tr( "Notice: %1 is a batch selected file but is not "
                                   "present in the DMD settings.\n" ) )
            .arg( it->first );
      }
   }

   return errormsg.isEmpty();
}

void US_Hydrodyn_Cluster::update_enables()
{
   unsigned int number_active;
   if ( batch_window->cb_dmd->isChecked() )
   {
      editor_msg( "black", tr( "\nChecking DMD settings:" ) );
      bool dmd_ok = validate_csv_dmd( number_active );
      if ( !noticemsg.isEmpty() )
      {
         editor_msg( "dark red", noticemsg );
      }
      if ( !dmd_ok )
      {
         editor_msg( "red", errormsg );
         pb_create_pkg->setEnabled( false );
         return;
      }
      if ( number_active < selected_files.size() &&
           !batch_window->cb_somo->isChecked() && 
           !batch_window->cb_grid->isChecked() && 
           !batch_window->cb_hydro->isChecked() && 
           !batch_window->cb_prr->isChecked() && 
           !batch_window->cb_iqq->isChecked() )
      {
         editor_msg( "red", tr( "For DMD only runs, all batch selected files must be accounted for and active in the DMD settings" ) );
         pb_create_pkg->setEnabled( false );
         return;
      }         
      editor_msg( "black", tr( "DMD settings ok." ) );
      pb_create_pkg->setEnabled( false );
   } 
   pb_create_pkg->setEnabled( create_enabled );
}

QString US_Hydrodyn_Cluster::options_summary()
{
   QString prefix =
      QString( tr( "Number of selected files: %1\n"
                   "Options summary:" ) )
      .arg( selected_files.size() );

   bool any_options = false;

   QString qs;
   if ( batch_window->cb_dmd->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "DMD";
      any_options = true;
   }

   if ( batch_window->cb_somo->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "SOMO bead models";
      any_options = true;
   }

   if ( batch_window->cb_grid->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "A2B Grid bead models";
      any_options = true;
   }

   if ( batch_window->cb_iqq->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "I(q) curves";
      any_options = true;
   }

   if ( batch_window->cb_prr->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "P(r) curves";
      any_options = true;
   }

   if ( batch_window->cb_hydro->isChecked() )
   {
      if ( !qs.isEmpty() )
      {
         qs += ", ";
      }
      qs += "Hydrodynamic parameter calculations";
      any_options = true;
   }

   if ( !any_options )
   {
      qs += "None selected";
   }

   return prefix + qs;
}

bool US_Hydrodyn_Cluster::read_config()
{
   // read "config.new" in package dir

   errormsg = "";

   QDir::setCurrent( pkg_dir );

   QString configfile = "config";
   QFile f( configfile );
   if ( !f.exists() )
   {
      errormsg = "config file does not exist";
      return false;
   }

   if ( !f.open( IO_ReadOnly ) )
   {
      errormsg = "config file is not readable";
      return false;
   }

   QTextStream ts( &f );
   
   if ( ts.atEnd() )
   {
      errormsg = "config file: premature end of file";
      f.close();
      return false;
   }

   cluster_config.clear();
   cluster_systems.clear();
   cluster_stage_to_system.clear();
   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );
   QRegExp rx_valid  ( 
                      "^("
                      "userid|"
                      "server|"
                      "system|"
                      "type|"
                      "corespernode|"
                      "maxcores|"
                      "runtime|"
                      "maxruntime|"
                      "queue|"
                      "stage"
                      ")$"
                      );
   QRegExp rx_config ( 
                      "^("
                      "userid|"
                      "server"
                      ")$"
                      );

   QRegExp rx_systems( 
                      "^("
                      "type|"
                      "corespernode|"
                      "maxcores|"
                      "runtime|"
                      "maxruntime|"
                      "queue|"
                      "stage"
                      ")$"
                      );

   QString last_system;
   unsigned int line = 0;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine().replace( rx_comment, "" ).replace( "^\\s+", "" ).replace( "\\s+$", "" );

      line++;

      if ( qs.contains( rx_blank ) )
      {
         continue;
      }

      QStringList qsl = QStringList::split( QRegExp("\\s+"), qs );

      if ( !qsl.size() )
      {
         continue;
      }

      QString option = qsl[ 0 ].lower();

      if ( rx_valid.search( option ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( configfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return false;
      }

      if ( qsl.size() < 1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( configfile )
            .arg( line );
         return false;
      }

      qsl.pop_front();

      if ( rx_config.search( option ) != -1 )
      {
         cluster_config[ option ] = qsl[ 0 ];
         continue;
      }

      if ( option == "system" )
      {
         last_system = qsl[ 0 ];
         if ( cluster_systems.count( last_system ) )
         {
            errormsg = QString( "Error reading %1 line %2 : system %3 multiply defined." )
               .arg( configfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }

         map < QString, QString > tmp_system;
         cluster_systems[ last_system ] = tmp_system;
         continue;
      }

      if ( rx_systems.search( option ) != -1 )
      {
         if ( last_system.isEmpty() )
         {
            errormsg = QString( "Error reading %1 line %2 : system must be specified before %3 " )
               .arg( configfile )
               .arg( line )
               .arg( option );
            return false;
         }

         cluster_systems[ last_system ][ option ] = qsl[ 0 ];
         if ( option == "stage" )
         {
            QString system = qsl[ 0 ];
            system.replace( QRegExp( ":.*$" ), "" );
            cluster_stage_to_system[ system ] = last_system;
         }
         continue;
      }
      
      errormsg = QString( "Error reading %1 line %2: unknown option %3" )
         .arg( configfile )
         .arg( line )
         .arg( option );
      return false;
   }
   f.close();

   cout << list_config();

   return true;
}

QString US_Hydrodyn_Cluster::list_config()
{
   QString qs;

   qs += "cluster config info:\n";

   for ( map < QString, QString >::iterator it = cluster_config.begin();
         it != cluster_config.end();
         it++ )
   {
      qs +=
         QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );
   }

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      for ( map < QString, QString >::iterator it2 = it->second.begin();
            it2 != it->second.end();
            it2++ )
      {
         qs +=
            QString( "%1\t%2\t%3\n" ).arg( it->first ).arg( it2->first ).arg( it2->second );
      }
   }

   qs += "\nstage to systems:\n";

   for ( map < QString, QString >::iterator it = cluster_stage_to_system.begin();
         it != cluster_stage_to_system.end();
         it++ )
   {
      qs +=
         QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );
   }

   return qs;
}

bool US_Hydrodyn_Cluster::write_config()
{
   QString out;

   for ( map < QString, QString >::iterator it = cluster_config.begin();
         it != cluster_config.end();
         it++ )
   {
      out +=
         QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );
   }

   for ( map < QString, map < QString, QString > >::iterator it = cluster_systems.begin();
         it != cluster_systems.end();
         it++ )
   {
      out += QString( "system\t%1\n" ).arg( it->first );
      for ( map < QString, QString >::iterator it2 = it->second.begin();
            it2 != it->second.end();
            it2++ )
      {
         out +=
            QString( "%1\t%2\n" ).arg( it2->first ).arg( it2->second );
      }
   }

   errormsg = "";

   QString configfile = "config";
   QFile f( configfile );

   if ( !f.open( IO_WriteOnly ) )
   {
      errormsg = "can not create config file";
      return false;
   }

   QTextStream ts( &f );
   
   ts << out;

   f.close();

   return true;
}
