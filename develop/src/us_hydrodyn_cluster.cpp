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

   selected_files.clear();
   for ( int i = 0; i < batch_window->lb_files->numRows(); i++ )
   {
      if ( batch_window->lb_files->isSelected(i) )
      {
         selected_files << batch_window->get_file_name( i );
      }
   }

   bool create_enabled =
      selected_files.size() &&
      // later we will add some of these other options,
      // right now, just iqq
      !batch_window->cb_somo->isChecked() && 
      !batch_window->cb_grid->isChecked() && 
      !batch_window->cb_hydro->isChecked() && 
      !batch_window->cb_prr->isChecked() && 
      batch_window->cb_iqq->isChecked() &&
      !batch_window->cb_compute_iq_avg->isChecked();

   setupGUI();

   pb_set_target->setEnabled( create_enabled );
   le_no_of_jobs->setEnabled( create_enabled );
   if ( batch_window->cluster_no_of_jobs.toUInt() &&
        batch_window->cluster_no_of_jobs.toUInt() <= selected_files.size() )
   {
      le_no_of_jobs->setText( batch_window->cluster_no_of_jobs );
   }
   le_target_file    ->setText   ( batch_window->cluster_target_datafile );
   le_output_name    ->setText   ( batch_window->cluster_output_name.isEmpty() ?
                                   "job" : batch_window->cluster_output_name );
   cb_for_mpi        ->setChecked( batch_window->cluster_for_mpi );
   
   le_output_name->setEnabled( create_enabled );
   pb_create_pkg ->setEnabled( create_enabled );
   cb_for_mpi    ->setEnabled( create_enabled );

   if ( !create_enabled )
   {
      editor_msg( "dark red", tr( "Notice: you must have files selected in batch model\n"
                                  "and only Compute I(q) selected to create a job.\n"
                                  "Future updates will provide additional functionality." ) );
   }

   pkg_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "cluster";
   QDir dir1( pkg_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( pkg_dir ) );
      dir1.mkdir( pkg_dir );
   }

   QDir::setCurrent( pkg_dir );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
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

   pb_set_target = new QPushButton(tr("Set experimental data file"), this);
   pb_set_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_set_target->setMinimumHeight(minHeight1);
   pb_set_target->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_set_target, SIGNAL(clicked()), SLOT(set_target()));

   le_target_file = new QLineEdit(this, "csv_filename Line Edit");
   le_target_file->setText( "" );
   le_target_file->setReadOnly( true );
   le_target_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_file->setMinimumWidth(150);
   le_target_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_no_of_jobs = new QLabel( QString(tr( "Number of jobs (maximum %1):" )).arg( selected_files.size() ), this);
   lbl_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_no_of_jobs->setMinimumHeight(minHeight1);
   lbl_no_of_jobs->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   QValidator *qv = new QIntValidator( 1, selected_files.size(), this );
   le_no_of_jobs = new QLineEdit(this, "csv_filename Line Edit");
   le_no_of_jobs->setText( "1" );
   le_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_no_of_jobs->setMinimumWidth(150);
   le_no_of_jobs->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_no_of_jobs->setValidator( qv );

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

   pb_create_pkg = new QPushButton(tr("Create cluster job package"), this);
   pb_create_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_pkg->setMinimumHeight(minHeight1);
   pb_create_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_pkg, SIGNAL(clicked()), SLOT(create_pkg()));

   pb_submit_pkg = new QPushButton(tr("Submit jobs for processing"), this);
   pb_submit_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit_pkg->setMinimumHeight(minHeight1);
   pb_submit_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_submit_pkg, SIGNAL(clicked()), SLOT(submit_pkg()));

   pb_check_status = new QPushButton(tr("Check job status"), this);
   pb_check_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_check_status->setMinimumHeight(minHeight1);
   pb_check_status->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_check_status, SIGNAL(clicked()), SLOT(check_status()));

   pb_load_results = new QPushButton(tr("Load results"), this);
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
   connect(pb_cluster_config, SIGNAL(clicked()), SLOT(cluster_config()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QHBoxLayout *hbl_target = new QHBoxLayout( 0 );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( lbl_target );
   hbl_target->addSpacing( 4);
   hbl_target->addWidget ( pb_set_target );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( le_target_file );
   hbl_target->addSpacing( 4 );

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
   background->addLayout ( hbl_target );
   background->addSpacing( 4 );
   background->addLayout ( hbl_no_of_jobs );
   background->addSpacing( 4 );
   background->addLayout ( hbl_output_name );
   background->addSpacing( 4 );
   background->addWidget ( cb_for_mpi );
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
   batch_window->cluster_no_of_jobs      = le_no_of_jobs->text();
   batch_window->cluster_target_datafile = le_target_file->text();
   batch_window->cluster_output_name     = le_output_name->text();
   batch_window->cluster_for_mpi         = cb_for_mpi->isChecked();

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Cluster::set_target()
{
   QString filename = QFileDialog::getOpenFileName(
                                                   ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs",
                                                   "All files (*);;"
                                                   "ssaxs files (*.ssaxs);;"
                                                   "csv files (*.csv);;"
                                                   "int files [crysol] (*.int);;"
                                                   "dat files [foxs / other] (*.dat);;"
                                                   "fit files [crysol] (*.fit);;"
                                                   , this
                                                   , "open file dialog"
                                                   , "Set file for grid target"
                                                   );

   le_target_file->setText( filename );
}

void US_Hydrodyn_Cluster::create_pkg()
{
   // check for prior existing jobs in submitted/completed

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
         tar_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( tar_filename );
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
                                                                         &path, &name, &ext, 0 );
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
            targz_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( targz_filename );
            filename = targz_filename;
            filename.replace( QRegExp( "\\.tgz$" ), "" );
            le_output_name->setText( QFileInfo( filename ).dirPath() == pkg_dir ?
                                     QFileInfo( filename ).fileName() : 
                                     filename );
         }
      }
   } 

   QString base_dir = QFileInfo( filename ).dirPath();
   if ( !QDir::setCurrent( base_dir ) )
   {
      editor_msg( "red", QString( tr( "Can not find output directory %1\n" ) ).arg( base_dir ) );
      return;
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


   if ( !le_target_file->text().isEmpty() )
   {
      base += 
         QString( "ExperimentGrid  %1\n" ).arg( QFileInfo( le_target_file->text() ).fileName() );
      base_source_files << le_target_file->text();
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
           our_saxs_options->start_q == our_saxs_options->delta_q )
      {
         unimplemented += QString("Error: saxs q range is empty");
      }
   }

   if ( batch_window->cb_mm_all->isChecked() )
   {
      base += "PDBAllModels\n";
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
   unsigned int max_no_of_jobs = (unsigned int) ceil( selected_files.size() / (double) le_no_of_jobs->text().toUInt() );
   cout << "max jobs per " << max_no_of_jobs << endl;
   unsigned int extension_count = selected_files.size();
   unsigned int extension_count_length = QString("%1").arg( extension_count ).length();

   if ( !copy_files_to_pkg_dir( base_source_files ) )
   {
      editor_msg( "red", errormsg );
      return;
   }

   QStringList source_files;
   QStringList dest_files;

   for ( unsigned int i = 0; i < selected_files.size(); i++ )
   {
      if ( !batch_window->cb_csv_saxs->isChecked() )
      {
         out += QString( "OutputFile      %1\n" ).arg( QFileInfo( selected_files[ i ] ).baseName() );
      }
      out += QString( "InputFile       %1\n" ).arg( QFileInfo( selected_files[ i ] ).fileName() );
      source_files << selected_files[ i ];
      out += "Process\n";
      if ( !( ( i + 1 ) % max_no_of_jobs ) )
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
         // use_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_file );
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
         

         source_files.clear();

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
      // use_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_file );
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
   for ( unsigned int i = 0; i < filenames.size(); i++ )
   {
      QString src  = filenames[ i ];
      QString dest = pkg_dir + SLASH + QFileInfo( src ).fileName();
      QFile fi( src  );
      QFile fo( dest );

      if ( !fi.exists() )
      {
         errormsg = QString( tr( "Error: requested source file %1 does not exist" ) ).arg( src );
         return false;
      }

      if ( !fi.open( IO_ReadOnly ) )
      {
         errormsg = QString( tr( "Error: requested source file %1 can not be opened. Check permissions" ) ).arg( src );
         return false;
      }

      if ( !fo.open( IO_WriteOnly ) )
      {
         errormsg = QString( tr( "Error: output file %1 can not be created." ) ).arg( dest );
         return false;
      }

      editor_msg( "dark gray", QString( tr( "Copying %1 to %2\n" ) ).arg( src ).arg( dest ) );
      QDataStream dsi( &fi );
      QDataStream dso( &fo );
      while ( !dsi.atEnd() )
      {
         dso << dsi;
      }
      fi.close();
      fo.close();
   }
   return true;
}

void US_Hydrodyn_Cluster::submit_pkg()
{
   // open cluster directory and find *_out.tgz
   // process and make unified csvs etc and load into standard somo/saxs directory
   if ( !read_config() )
   {
      cluster_config();
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
}


bool US_Hydrodyn_Cluster::read_config()
{
   // read "config" in package dir

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
   ts >> cluster_id;

   if ( ts.atEnd() )
   {
      errormsg = "config file: premature end of file";
      f.close();
      return false;
   }
   ts >> submit_url;

   if ( ts.atEnd() )
   {
      errormsg = "config file: premature end of file";
      f.close();
      return false;
   }
   ts >> stage_url;
   f.close();

   return true;
}

bool US_Hydrodyn_Cluster::write_config()
{
   // write "config" file

   errormsg = "";

   QString configfile = "config";
   QFile f( configfile );

   if ( !f.open( IO_WriteOnly ) )
   {
      errormsg = "can not create config file";
      return false;
   }

   QTextStream ts( &f );
   
   ts << cluster_id << endl;

   ts << submit_url << endl;

   ts << stage_url << endl;

   f.close();

   return true;
}

void US_Hydrodyn_Cluster::cluster_config()
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
   hcs->exec();
}
