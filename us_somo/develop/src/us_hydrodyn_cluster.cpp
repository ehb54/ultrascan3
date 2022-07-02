#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_hydrodyn_cluster.h"
#include "../include/us_pm_global.h"
#include "../include/us_pm.h"
//Added by qt3to4:
#include <QBoxLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QGridLayout>
#include <QTextStream>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
 //#include <Q3PopupMenu>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

US_Hydrodyn_Cluster::US_Hydrodyn_Cluster(
                                         void *us_hydrodyn, 
                                         QWidget *p, 
                                         const char *
                                         ) : QDialog( p )
{
   this->us_hydrodyn = us_hydrodyn;
   this->batch_window = (US_Hydrodyn_Batch *)p;
   this->our_saxs_options = &((US_Hydrodyn *)us_hydrodyn)->saxs_options;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("US-SOMO: Cluster"));

   cluster_additional_methods_options_active   = &( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_active;
   cluster_additional_methods_options_selected = &( ( US_Hydrodyn * ) us_hydrodyn )->cluster_additional_methods_options_selected;

   cluster_additional_methods_use_experimental_data    [ "bfnb"     ] = true;
   cluster_additional_methods_require_experimental_data[ "bfnb"     ] = true;
   cluster_additional_methods_use_experimental_data    [ "bfnb_nsa" ] = true;
   cluster_additional_methods_require_experimental_data[ "bfnb_nsa" ] = true;
   cluster_additional_methods_require_sleep            [ "dammin"   ] = true;
   cluster_additional_methods_require_sleep            [ "dammif"   ] = true;
   cluster_additional_methods_require_sleep            [ "gasbor"   ] = true;
   cluster_additional_methods_parallel_mpi             [ "bfnb"     ] = true;
   cluster_additional_methods_parallel_mpi             [ "bfnb_nsa" ] = true;
   cluster_additional_methods_parallel_mpi             [ "oned"     ] = true;
   cluster_additional_methods_prepend                  [ "bfnb"     ] = "bfnbpm_";
   cluster_additional_methods_prepend                  [ "bfnb_nsa" ] = "bfnb_";
   cluster_additional_methods_prepend                  [ "oned"     ] = "oned_";
   cluster_additional_methods_prepend                  [ "best"     ] = "best_";
   cluster_additional_methods_one_pdb_exactly          [ "oned"     ] = true;
   cluster_additional_methods_no_tgz_output            [ "oned"     ] = true;
   cluster_additional_methods_must_run_alone           [ "bfnb"     ] = true;
   cluster_additional_methods_must_run_alone           [ "oned"     ] = true;
   cluster_additional_methods_must_run_alone           [ "best"     ] = true;
   cluster_additional_methods_add_selected_files       [ "oned"     ] = true;

   cluster_additional_methods_modes[ "one_run_per_file"               ][ "best" ] = true;
   cluster_additional_methods_modes[ "additional_processing_per_file" ][ "best" ] = true;
   cluster_additional_methods_modes[ "inputfile_pat_addendum"         ][ "best" ] = true;
   cluster_additional_methods_modes[ "additional_processing_global"   ][ "best" ] = true;
   cluster_additional_methods_modes[ "no_multi_model_pdb"             ][ "best" ] = true;
   cluster_additional_methods_modes[ "inputfilenoread"                ][ "best" ] = true;
   cluster_additional_methods_modes[ "acceptcommon"                   ][ "best" ] = true;

   cluster_additional_methods_job_multiplier[ "best" ] = 3;

   cluster_config[ "userid" ] = "";
   cluster_config[ "server" ] = "";

   selected_files.clear( );
   for ( int i = 0; i < batch_window->lb_files->count(); i++ )
   {
      if ( batch_window->lb_files->item(i)->isSelected() )
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
   
   QString unsupported;

   if ( batch_window->cb_somo->isChecked() )
   {
      unsupported += us_tr( "Build SOMO bead model\n" );
   }
   if ( batch_window->cb_grid->isChecked() )
   {
      unsupported += us_tr( "Build A to B (Grid) bead model\n" );
   }
   if ( batch_window->cb_hydro->isChecked() )
   {
      unsupported += us_tr( "Calculate hydrodynamics\n" );
   }
   if ( batch_window->cb_prr->isChecked() )
   {
      unsupported += us_tr( "Compute SAXS P(r)\n" );
   }
   if ( batch_window->cb_prr->isChecked() )
   {
      unsupported += us_tr( "Compute SAXS P(r)\n" );
   }
   if ( batch_window->cb_compute_iq_avg->isChecked() )
   {
      unsupported += us_tr( "Compute I(q) average curves\n" );
   }

   setupGUI();

   pb_add_target  ->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
   pb_clear_target->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
   le_no_of_jobs  ->setEnabled( create_enabled );
   if ( batch_window->cluster_no_of_jobs.toUInt() &&
        batch_window->cluster_no_of_jobs.toUInt() <= (unsigned int)selected_files.size() )
   {
      le_no_of_jobs->setText( batch_window->cluster_no_of_jobs );
   }
   lb_target_files->addItems( batch_window->cluster_target_datafiles );
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
      editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( pkg_dir ) );
      dir1.mkdir( pkg_dir );
   }

   {
      submitted_dir = pkg_dir  + SLASH + "submitted";
      QDir dir1( submitted_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( submitted_dir ) );
         dir1.mkdir( submitted_dir );
      }
      
      QDir::setCurrent( submitted_dir );

      QDir qd;

      QStringList tgz_files = qd.entryList( QStringList() << "*.tgz" );
      QStringList tar_files = qd.entryList( QStringList() << "*.tar" );

      // QStringList submitted_files = QStringList::split( "\n", 
      //                                                   tgz_files.join("\n") + 
      //                                                   ( tgz_files.size() ? "\n" : "" ) +
      //                                                   tar_files.join("\n") );

      QStringList submitted_files;
      {
         QString qs =
            tgz_files.join("\n") + 
            ( tgz_files.size() ? "\n" : "" ) +
            tar_files.join("\n")
            ;
         submitted_files = (qs ).split( "\n" , Qt::SkipEmptyParts );
      }

      for ( unsigned int i = 0; i < (unsigned int) submitted_files.count(); i++ )
      {
         submitted_jobs[ submitted_files[ i ].replace( QRegExp( "\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   {
      completed_dir = pkg_dir  + SLASH + "completed";
      QDir dir1( completed_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( completed_dir ) );
         dir1.mkdir( completed_dir );
      }

      QDir::setCurrent( completed_dir );

      QDir qd;

      QStringList tgz_files = qd.entryList( QStringList() << "*.tgz" );
      QStringList tar_files = qd.entryList( QStringList() << "*.tar" );

      // QStringList completed_files = QStringList::split( "\n", 
      //                                       tgz_files.join("\n") + 
      //                                       ( tgz_files.size() ? "\n" : "" ) +
      //                                       tar_files.join("\n") );

      QStringList completed_files;
      {
         QString qs =
            tgz_files.join("\n") + 
            ( tgz_files.size() ? "\n" : "" ) +
            tar_files.join("\n")
            ;
         completed_files = (qs ).split( "\n" , Qt::SkipEmptyParts );
      }
      
      for ( unsigned int i = 0; i < (unsigned int) completed_files.count(); i++ )
      {
         completed_jobs[ completed_files[ i ].replace( QRegExp( "_(out|OUT)\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   {
      results_dir = pkg_dir  + SLASH + "results";
      QDir dir1( results_dir );
      if ( !dir1.exists() )
      {
         editor_msg( "black", QString( us_tr( "Created directory %1" ) ).arg( results_dir ) );
         dir1.mkdir( results_dir );
      }

      QDir::setCurrent( results_dir );

      QDir qd;

      QStringList results_files = qd.entryList( QStringList() << "*" );
      for ( unsigned int i = 0; i < (unsigned int)results_files.count(); i++ )
      {
         results_jobs[ results_files[ i ].replace( QRegExp( "_(out|OUT)\\.(tar|tgz|TAR|TGZ)$" ), "" ) ] = true;
      }
   }

   QDir::setCurrent( pkg_dir );

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );

   editor_msg( "dark blue", options_summary() );
   if ( unsupported.length() )
   {
      editor_msg( "red", us_tr( "Unsupported options selected:\n" + unsupported ) );
   }

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
      editor_msg( "dark red", us_tr( "Notice: you must have files selected in batch model\n"
                                  "and only Compute I(q) or Run DMD selected to create a job.\n"
                                  "Future updates will provide additional functionality." ) );
   }
}

US_Hydrodyn_Cluster::~US_Hydrodyn_Cluster()
{
}

void US_Hydrodyn_Cluster::setupGUI()
{
   // bool expert_mode = 
   //    U_EXPT ||
   //    active_additional_methods().size();

   int minHeight1 = 30;

   lbl_title = new QLabel( us_tr( "Create file for cluster jobs" ), this);
   lbl_title->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title->setMinimumHeight(minHeight1);
   lbl_title->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title );
   lbl_title->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_target = new QLabel("Grid from experimental data:", this);
   lbl_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_target->setMinimumHeight(minHeight1);
   lbl_target->setPalette( PALET_LABEL );
   AUTFBACK( lbl_target );
   lbl_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_create_pkg = new QPushButton(us_tr("Create cluster job package"), this);
   pb_create_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_pkg->setMinimumHeight(minHeight1);
   pb_create_pkg->setPalette( PALET_PUSHB );
   connect(pb_create_pkg, SIGNAL(clicked()), SLOT(create_pkg()));

   pb_add_target = new QPushButton(us_tr("Add experimental data files"), this);
   pb_add_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_add_target->setMinimumHeight(minHeight1);
   pb_add_target->setPalette( PALET_PUSHB );
   connect(pb_add_target, SIGNAL(clicked()), SLOT(add_target()));

   pb_clear_target = new QPushButton(us_tr("Clear experimental data files"), this);
   pb_clear_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_clear_target->setMinimumHeight(minHeight1);
   pb_clear_target->setPalette( PALET_PUSHB );
   connect(pb_clear_target, SIGNAL(clicked()), SLOT(clear_target()));

   lb_target_files = new QListWidget( this );
   lb_target_files->setPalette( PALET_NORMAL );
   AUTFBACK( lb_target_files );
   lb_target_files->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   lb_target_files->setEnabled(true);
   lb_target_files->setSelectionMode( QListWidget::NoSelection );
   
   lb_target_files->setMinimumHeight( minHeight1 * 2 );

   lbl_no_of_jobs = new QLabel( QString(us_tr( "Number of jobs (cores) (maximum %1):" )).arg( selected_files.size() ), this);
   lbl_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_no_of_jobs->setMinimumHeight(minHeight1);
   lbl_no_of_jobs->setPalette( PALET_LABEL );
   AUTFBACK( lbl_no_of_jobs );
   lbl_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_no_of_jobs_qv = new QIntValidator( 1, selected_files.size(), this );
   le_no_of_jobs = new QLineEdit( this );    le_no_of_jobs->setObjectName( "csv_filename Line Edit" );
   le_no_of_jobs->setText( "1" );
   le_no_of_jobs->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_no_of_jobs->setMinimumWidth(150);
   le_no_of_jobs->setPalette( PALET_NORMAL );
   AUTFBACK( le_no_of_jobs );
   le_no_of_jobs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_no_of_jobs->setValidator( le_no_of_jobs_qv );

   lbl_output_name = new QLabel(us_tr("Output base name (job identifier)"), this);
   lbl_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_output_name->setMinimumHeight(minHeight1);
   lbl_output_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_output_name );
   lbl_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_output_name = new QLineEdit( this );    le_output_name->setObjectName( "csv_filename Line Edit" );
   le_output_name->setText( "job" );
   le_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_output_name->setMinimumWidth(150);
   le_output_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_output_name );
   le_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect( le_output_name, SIGNAL( textChanged( const QString &) ), SLOT( update_output_name( const QString & ) ) );

   cb_for_mpi = new QCheckBox(this);
   cb_for_mpi->setText(us_tr(" Package for parallel job submission"));
   cb_for_mpi->setEnabled(true);
   cb_for_mpi->setChecked(true);
   cb_for_mpi->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_for_mpi->setPalette( PALET_NORMAL );
   AUTFBACK( cb_for_mpi );
   connect( cb_for_mpi, SIGNAL( clicked() ), SLOT( for_mpi() ) );

   cb_split_grid = new QCheckBox(this);
   cb_split_grid->setText(us_tr(" Individual jobs for each grid"));
   cb_split_grid->setEnabled(true);
   cb_split_grid->setChecked(false);
   cb_split_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_split_grid->setPalette( PALET_NORMAL );
   AUTFBACK( cb_split_grid );
   connect( cb_split_grid, SIGNAL( clicked() ), SLOT( split_grid() ) );

   pb_dmd = new QPushButton(us_tr("DMD settings"), this);
   pb_dmd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_dmd->setMinimumHeight(minHeight1);
   pb_dmd->setPalette( PALET_PUSHB );
   connect( pb_dmd, SIGNAL( clicked() ), SLOT( dmd() ) );

   // if ( expert_mode )
   // {
   pb_additional = new QPushButton(us_tr("Other methods"), this);
   pb_additional->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_additional->setMinimumHeight(minHeight1);
   pb_additional->setPalette( PALET_PUSHB );
   connect(pb_additional, SIGNAL(clicked()), SLOT(additional()));
   // }

   pb_advanced = new QPushButton(us_tr("Advanced options"), this);
   pb_advanced->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_advanced->setMinimumHeight(minHeight1);
   pb_advanced->setPalette( PALET_PUSHB );
   connect(pb_advanced, SIGNAL(clicked()), SLOT(advanced()));

   pb_submit_pkg = new QPushButton(us_tr("Submit jobs for processing"), this);
   pb_submit_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_submit_pkg->setMinimumHeight(minHeight1);
   pb_submit_pkg->setPalette( PALET_PUSHB );
   connect(pb_submit_pkg, SIGNAL(clicked()), SLOT(submit_pkg()));

   pb_check_status = new QPushButton(us_tr("Check job status / Retrieve results"), this);
   pb_check_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_check_status->setMinimumHeight(minHeight1);
   pb_check_status->setPalette( PALET_PUSHB );
   connect(pb_check_status, SIGNAL(clicked()), SLOT(check_status()));

   pb_load_results = new QPushButton(us_tr("Extract results"), this);
   pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_results->setMinimumHeight(minHeight1);
   pb_load_results->setPalette( PALET_PUSHB );
   connect(pb_load_results, SIGNAL(clicked()), SLOT(load_results()));
   
   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );

   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
#endif


   editor->setWordWrapMode (QTextOption::WordWrap);
   editor->setMinimumHeight(300);

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cluster_config = new QPushButton(us_tr("Cluster Configuration"), this);
   pb_cluster_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cluster_config->setMinimumHeight(minHeight1);
   pb_cluster_config->setPalette( PALET_PUSHB );
   connect(pb_cluster_config, SIGNAL(clicked()), SLOT(config()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   QGridLayout * gl_target = new QGridLayout( 0 ); gl_target->setContentsMargins( 0, 0, 0, 0 ); gl_target->setSpacing( 0 );
   gl_target->addWidget( lbl_target       , 0 , 0 , 1 + ( 1 ) - ( 0 ) , 1 + ( 0  ) - ( 0 ) );
   gl_target->addWidget         ( pb_add_target   , 0, 1 );
   gl_target->addWidget         ( pb_clear_target , 1, 1 );
   gl_target->addWidget( lb_target_files  , 0 , 2 , 1 + ( 1 ) - ( 0 ) , 1 + ( 2  ) - ( 2 ) );

   QHBoxLayout * hbl_no_of_jobs = new QHBoxLayout(); hbl_no_of_jobs->setContentsMargins( 0, 0, 0, 0 ); hbl_no_of_jobs->setSpacing( 0 );
   hbl_no_of_jobs->addSpacing( 4 );
   hbl_no_of_jobs->addWidget ( lbl_no_of_jobs );
   hbl_no_of_jobs->addSpacing( 4 );
   hbl_no_of_jobs->addWidget ( le_no_of_jobs );
   hbl_no_of_jobs->addSpacing( 4 );

   QHBoxLayout * hbl_output_name = new QHBoxLayout(); hbl_output_name->setContentsMargins( 0, 0, 0, 0 ); hbl_output_name->setSpacing( 0 );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( lbl_output_name );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( le_output_name );
   hbl_output_name->addSpacing( 4 );

   QHBoxLayout * hbl_mpi_etc = new QHBoxLayout(); hbl_mpi_etc->setContentsMargins( 0, 0, 0, 0 ); hbl_mpi_etc->setSpacing( 0 );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( cb_for_mpi );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( cb_split_grid );
   hbl_mpi_etc->addSpacing( 4 );
   hbl_mpi_etc->addWidget ( pb_dmd );
   hbl_mpi_etc->addSpacing( 4 );
   // if ( expert_mode )
   // {
   hbl_mpi_etc->addWidget ( pb_additional );
   hbl_mpi_etc->addSpacing( 4 );
   // }
   hbl_mpi_etc->addWidget ( pb_advanced );
   hbl_mpi_etc->addSpacing( 4 );
   

   QHBoxLayout * hbl_create = new QHBoxLayout(); hbl_create->setContentsMargins( 0, 0, 0, 0 ); hbl_create->setSpacing( 0 );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_create_pkg );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_submit_pkg );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_check_status );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_load_results );
   hbl_create->addSpacing( 4 );

   QHBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cluster_config );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget(frame);
#endif
   vbl_editor_group->addWidget(editor);

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
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
   online_help->show_help("manual/somo/somo_cluster.html");
}

void US_Hydrodyn_Cluster::closeEvent(QCloseEvent *e)
{
   batch_window->cluster_no_of_jobs       = le_no_of_jobs->text();
   QStringList target_files;
   for ( int i = 0; i < lb_target_files->count(); i++ )
   {
      target_files << lb_target_files->item( i )->text();
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
   lb_target_files->clear( );
   update_validator();
}

void US_Hydrodyn_Cluster::add_target()
{
   map < QString, bool > existing_items;
   for ( int i = 0; i < lb_target_files->count(); i++ )
   {
      existing_items[ lb_target_files->item( i )->text() ] = true;
   }

   QString use_dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + QDir::separator() + "saxs";
   ((US_Hydrodyn  *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QStringList filenames = QFileDialog::getOpenFileNames( this , "Set files for grid target" , use_dir , "All files (*);;"
                                                   "ssaxs files (*.ssaxs);;"
                                                   "csv files (*.csv);;"
                                                   "int files [crysol] (*.int);;"
                                                   "dat files [foxs / other] (*.dat);;"
                                                   "fit files [crysol] (*.fit)" );


   QStringList add_filenames;

   for ( unsigned int i = 0; i < (unsigned int)filenames.size(); i++ )
   {
      if ( !existing_items.count( filenames[ i ] ) )
      {
         add_filenames << filenames[ i ];
         ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( filenames[ i ] );
      }
   }

   lb_target_files->addItems( add_filenames );
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

   QString common_prefix = cb_for_mpi->isChecked() ? "../common/" : "";
   QString prepend = job_prepend_name();
   if ( !prepend.isEmpty() )
   {
      editor_msg( "blue", QString( us_tr( "Notice: job output name will be prepended with %1" ) ).arg( prepend ) );
   }

   if ( !le_no_of_jobs->text().toUInt() )
   {
      le_no_of_jobs->setText( "1" );
   }
   bool use_extension =  le_no_of_jobs->text().toUInt() != 1;

   // create the output file
   QString filename = 
      le_output_name->text() == QFileInfo( le_output_name->text() ).fileName() ?
      pkg_dir + SLASH + prepend + le_output_name->text() :
      prepend + le_output_name->text();

   if ( cb_for_mpi->isChecked() )
   {
      QString tar_filename = filename + ".tar";
      if ( QFile::exists( tar_filename ) )
      {
         tar_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( tar_filename, 0, this );
         filename = tar_filename;
         filename.replace( QRegExp( "\\.tar$" ), "" );
         le_output_name->setText( QDir::toNativeSeparators( QFileInfo( filename ).path() ) == 
                                  QDir::toNativeSeparators( pkg_dir ) ?
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
            QString path =  QFileInfo( targz_filename ).path() + SLASH;
            QString name =  QFileInfo( targz_filename ).fileName().replace( QRegExp( "\\_p1.tgz$" ), "" );
            targz_filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( 
                                                                         &path, &name, &ext, 0, this );
            filename = targz_filename;
            filename.replace( QRegExp( "\\_p1.tgz$" ), "" );
            le_output_name->setText( QDir::toNativeSeparators( QFileInfo( filename ).path() ) == 
                                     QDir::toNativeSeparators( pkg_dir ) ?
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
            le_output_name->setText( QDir::toNativeSeparators( QFileInfo( filename ).path() ) == 
                                     QDir::toNativeSeparators( pkg_dir ) ?
                                     QFileInfo( filename ).fileName() : 
                                     filename );
         }
      }
   } 

   if ( dup_in_submitted_or_completed() )
   {
      return;
   }

   QString base_dir = QFileInfo( filename ).path();
   if ( !QDir::setCurrent( base_dir ) )
   {
      editor_msg( "red", QString( us_tr( "Can not find output directory %1\n" ) ).arg( base_dir ) );
      return;
   }

   {
      QFileInfo qfi( le_output_name->text() );
      if ( qfi.exists() && !qfi.isFile() )
      {
         editor_msg( "red", QString( us_tr( "Output file \"%1\" already exists and is not a regular file\n" ) ).arg( le_output_name->text() ) );
         return;
      }
   }

   editor_msg( "black", QString( us_tr( "Output directory is %1\n" ) ).arg( base_dir ) );
   if ( active_additional_methods().size() )
   {
      create_additional_methods_pkg( base_dir, filename, common_prefix, use_extension );
      return;
   }      

   QString base = 
      "# us_saxs_cmds_t iq controlfile\n"
      "# blank lines ok, format token <params>\n"
      "\n";

   base += 
      QString( "ResidueFile     %1\n" ).arg( common_prefix + QFileInfo( ((US_Hydrodyn *)us_hydrodyn)->lbl_table->text() ).fileName() );
   base_source_files << ((US_Hydrodyn *)us_hydrodyn)->lbl_table->text();

   base += 
      QString( "AtomFile        %1\n" ).arg( common_prefix + QFileInfo( our_saxs_options->default_atom_filename ).fileName() );
   base_source_files << our_saxs_options->default_atom_filename;
   base += 
      QString( "HybridFile      %1\n" ).arg( common_prefix + QFileInfo( our_saxs_options->default_hybrid_filename ).fileName() );
   base_source_files << our_saxs_options->default_hybrid_filename;
   base += 
      QString( "SaxsFile        %1\n" ).arg( common_prefix + QFileInfo( our_saxs_options->default_saxs_filename ).fileName() );
   base_source_files << our_saxs_options->default_saxs_filename;

   if ( our_saxs_options->swh_excl_vol != 0e0 )
   {
      base += 
         QString( "SwhExclVol      %1\n" ).arg( our_saxs_options->swh_excl_vol );
   }
   if ( our_saxs_options->iq_target_ev != 0e0 )
   {
      base += 
         QString( "IqTargetEv      %1\n" ).arg( our_saxs_options->iq_target_ev );
   }
   if ( our_saxs_options->hybrid_radius_excl_vol )
   {
      base += 
         QString( "HybridRadiusExclVol\n" );
   }
   if ( our_saxs_options->set_iq_target_ev_from_vbar )
   {
      base += 
         QString( "SetIqTargetEvFromVbar\n" );
   }
   if ( our_saxs_options->use_iq_target_ev )
   {
      base += 
         QString( "UseIqTargetEv\n" );
   }

   if ( batch_window->cb_prr->isChecked() )
   {
      base += 
         QString( "prbinsize       %1\n" ).arg( our_saxs_options->bin_size );
      base += 
         QString( "prcurve         %1\n" ).arg( our_saxs_options->curve );
   }

   if ( batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
   {
      base += 
         QString( "HydrationFile   %1\n" ).arg( common_prefix + QFileInfo( our_saxs_options->default_rotamer_filename ).fileName() );
      base += 
         QString( "HydrationSCD    %1\n" ).arg( our_saxs_options->steric_clash_distance );
      base += 
         QString( "HydrationRSCD   %1\n" ).arg( our_saxs_options->steric_clash_recheck_distance );
      base_source_files << our_saxs_options->default_rotamer_filename;
   }



   if ( our_saxs_options->use_somo_ff )
   {
      base += 
         QString( "FFFile          %1\n" ).arg( common_prefix + QFileInfo( our_saxs_options->default_ff_filename ).fileName() );
      base_source_files << our_saxs_options->default_ff_filename;
   }

   if ( our_saxs_options->iq_exact_q )
   {
      base += 
         QString( "ExactQ\n" );
   }

//    if ( our_saxs_options->alt_ff )
//    {
//       base += 
//          QString( "TestingFF\n" );
//    }

   if ( our_saxs_options->five_term_gaussians )
   {
      base += 
         QString( "FiveTermGaussian\n" );
   }

   if ( our_saxs_options->iqq_use_atomic_ff )
   {
      base += 
         QString( "ExplicitH\n" );
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
            US_Static::us_message( us_tr( "Please note:" ), 
                                  us_tr( "You have selected to process all models in multi-model PDBs with the FoXS external I(q) method.\n"
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
            US_Static::us_message( us_tr( "Please note:" ), 
                                  us_tr( "You have selected to process all models in multi-model PDBs with the CRYSOL external I(q) method.\n"
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
      QString( "CrysolHarm      %1\n" ).arg( our_saxs_options->sh_max_harmonics );
   base += 
      QString( "CrysolGrid      %1\n" ).arg( our_saxs_options->sh_fibonacci_grid_order );
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

   if ( ((US_Hydrodyn *)us_hydrodyn)->misc.pb_rule_on )
   {
      base += 
         QString( "PbRuleOn\n" );
   }

   if ( lb_target_files->count() )
   {
      if ( lb_target_files->count() == 1 || !cb_split_grid->isChecked() )
      {
         base += 
            QString( "ExperimentGrid  %1\n" ).arg( common_prefix + QFileInfo( lb_target_files->item( 0 )->text() ).fileName() );
         base_source_files << lb_target_files->item( 0 )->text();
         for ( int i = 1; i < lb_target_files->count(); i++ )
         {
            base += 
               QString( "AdditionalExperimentGrid  %1\n" ).arg( common_prefix + QFileInfo( lb_target_files->item( i )->text() ).fileName() );
            base_source_files << lb_target_files->item( i )->text();
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
      base += dmd_base_addition( base_source_files, common_prefix );
   }
      
   if ( !cb_for_mpi->isChecked() &&
        !copy_files_to_pkg_dir( base_source_files ) )
   {
      editor_msg( "red", errormsg );
      return;
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

   QStringList source_files;
   QStringList dest_files;
   QStringList qsl_advanced;

   if ( any_advanced() )
   {
      editor_msg( "blue", us_tr( "Note: using Advanced Options" ) );
      qsl_advanced = advanced_addition();
   }

   // create use_selected_files for dmd expansion
   
   QStringList use_selected_files;

   QRegExp rx_dmd( "^(.*) _dmd(\\d+)$" );

   for ( unsigned int i = 0; i < (unsigned int)selected_files.size(); i++ )
   {
      if ( batch_window->cb_dmd->isChecked() &&
           dmd_entry_count( QFileInfo( selected_files[ i ] ).fileName() ) > 1 )
      {
         for ( unsigned int j = 0; j < dmd_entry_count( QFileInfo( selected_files[ i ] ).fileName() ); j++ )
         {
            use_selected_files << QString( "%1 _dmd%2" ).arg( selected_files[ i ] ).arg( j );
         }
      } else {
         use_selected_files << selected_files[ i ];
      }
   }

   for ( unsigned int i = 0; i < (unsigned int)use_selected_files.size(); i++ )
   {
      cout << QString( "pos %1: %2\n" ).arg( i ).arg( use_selected_files[ i ] );
   }

   unsigned int multi_grid_multiplier = 
      cb_split_grid->isChecked() && lb_target_files->count() > 1 ?
      lb_target_files->count() : 1;

   unsigned int advanced_multiplier =
      qsl_advanced.size() ? (unsigned int)qsl_advanced.size() : 1;

   unsigned int max_no_of_jobs = 
      (unsigned int) ceil( ( use_selected_files.size() * multi_grid_multiplier * advanced_multiplier )
                           / (double) le_no_of_jobs->text().toUInt() );
   cout << "max jobs per " << max_no_of_jobs << endl;

   unsigned int extension_count = max_no_of_jobs;
   unsigned int extension_count_length = QString("%1").arg( extension_count ).length();
   map < QString, bool > already_added;

   bool last_unwritten = false;
   US_Tar ust;

   for ( unsigned int this_i = 0; this_i < (unsigned int)use_selected_files.size() * multi_grid_multiplier * advanced_multiplier; this_i++ )
   {
      unsigned int i    = this_i % (unsigned int)use_selected_files.size();

      QString name    = use_selected_files[ i ];
      int     dmd_pos = -1;
      if ( rx_dmd.indexIn( name ) != -1 )
      {
         name    = rx_dmd.cap( 1 );
         dmd_pos = rx_dmd.cap( 2 ).toInt();
      }

      QString use_file_name   = QFileInfo( name ).fileName();
      QString use_output_name = QFileInfo( name ).baseName();

      last_unwritten = true;
      if ( multi_grid_multiplier > 1 )
      {
         unsigned int grid = ( this_i / (unsigned int)use_selected_files.size() ) % multi_grid_multiplier;
         out += QString( "ExperimentGrid  %1\n" ).arg( common_prefix + QFileInfo( lb_target_files->item( grid )->text() ).fileName() );
         if ( !already_added.count( lb_target_files->item( grid )->text() ) )
         {
            already_added[ lb_target_files->item( grid )->text() ] = true;
            if ( !cb_for_mpi->isChecked() )
            {
               source_files << lb_target_files->item( grid )->text();
            } else {
               base_source_files << lb_target_files->item( grid )->text();
            }
         }
      }
      
      unsigned int advanced_pos = 0;
      if ( advanced_multiplier > 1 )
      {
         advanced_pos = this_i / ( (unsigned int)use_selected_files.size() * multi_grid_multiplier );
      }
      
      cout << QString( "this_i %1 grid %2 advanced_pos %3\n" )
         .arg( this_i )
         .arg( ( this_i / (unsigned int)use_selected_files.size() ) % multi_grid_multiplier )
         .arg( advanced_pos );
      
      out += QString( "InputFile       %1\n" ).arg( common_prefix + QFileInfo( name ).fileName() );
      
      if ( !batch_window->cb_dmd->isChecked()
           && batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
      {
         if ( our_saxs_options->alt_hydration )
         {
            out += "HydrateAlt\n";
         }
         if ( our_saxs_options->hydration_rev_asa )
         {
            out += "HydrateRevASA\n";
         }
         out += "Hydrate\n";
         use_output_name += "-h";
      }
      if ( !batch_window->cb_csv_saxs->isChecked() )
      {
         out += QString( "OutputFile      %1\n" ).arg( use_output_name );
      }
      if ( !already_added.count( name ) )
      {
         already_added[ name ] = true;
         if ( !cb_for_mpi->isChecked() )
         {
            source_files << name;
         } else {
            base_source_files << name;
         }
      }
      if ( any_advanced() )
      {
         // out += advanced_addition( use_output_name );
         out += qsl_advanced[ advanced_pos ];
      } else {
         if ( batch_window->cb_dmd->isChecked() )
         {
            out += dmd_file_addition( use_file_name, use_output_name, dmd_pos );
         } else {
            out += "Process\n";
         }
      }
      if ( !( ( this_i + 1 ) % max_no_of_jobs ) )
      {
         last_unwritten = false;
         write_count++;
         QString ext = "";
         if ( use_extension )
         {
            ext = QString("%1").arg( write_count );
            while ( (unsigned int) ext.length() < extension_count_length )
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
         if ( f.open( QIODevice::WriteOnly ) )
         {
            QTextStream ts( &f );
            ts << out;
            ts << QString( "TgzOutput       %1_out.tgz\n" ).arg(
                                                                le_output_name->text() + 
                                                                ( use_extension ? QString("_p%1").arg( ext ) : "" )
                                                                );
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
         if ( !cb_for_mpi->isChecked() )
         {
            for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
            {
               to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
            }
         }
         for ( unsigned int i = 0; i < (unsigned int)source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( source_files[ i ] ).fileName();
            remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
         }
         remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();

         QString tar_name = use_file + ".tar";
         int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
         cout << "tar_name:" << tar_name << endl;
         cout << "to tar:\n" << to_tar_list.join("\n") << endl;
         
         if ( result != TAR_OK )
         {
            editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
            return;
         }
         editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );
         US_Gzip usg;
         result = usg.gzip( tar_name );
         if ( result != GZIP_OK )
         {
            editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
         
         if ( !cb_for_mpi->isChecked() )
         {
            source_files .clear( );
            already_added.clear( );
         }
         
         out = base;
      }
   }
   if ( last_unwritten )
   {
      write_count++;
      QString ext = "";
      if ( use_extension )
      {
         ext = QString("%1").arg( write_count );
         while ( (unsigned int) ext.length() < extension_count_length )
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
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << out;
         ts << QString( "TgzOutput       %1_out.tgz\n" ).arg(
                                                             le_output_name->text() + 
                                                             ( use_extension ? QString("_p%1").arg( ext ) : "" )
                                                             );
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
      if ( !cb_for_mpi->isChecked() )
      {
         for ( unsigned int i = 0; i <  (unsigned int)base_source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
         }
      }
      for ( unsigned int i = 0; i < (unsigned int)source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( source_files[ i ] ).fileName();
         remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
      }
      remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();

      QString tar_name = use_file + ".tar";
      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &qsl );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;
      
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
         return;
      }
      editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );
      US_Gzip usg;
      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
      
      source_files.clear( );
      
      out = base;
   }

   if ( cb_for_mpi->isChecked() )
   {
      if ( !copy_files_to_pkg_dir( base_source_files ) )
      {
         editor_msg( "red", errormsg );
         return;
      }

      QStringList list;
      QStringList to_tar_list;
      for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
      }

      QString tar_name = QString( "%1%2common_%3.tar" )
         .arg( QFileInfo( filename ).path() )
         .arg( QDir::separator() )
         .arg( QFileInfo( filename ).fileName() )
         ;

      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;

      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( tar_name ).arg( ust.explain( result ) ) );
         return;
      }

      US_Gzip usg;
      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
   }

   QStringList base_remove_file_list;
   for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
   {
      base_remove_file_list << pkg_dir + SLASH + QFileInfo( base_source_files[ i ] ).fileName();
   }
   remove_files( base_remove_file_list );
   cout << "written:" << write_count << endl;
   if ( write_count != le_no_of_jobs->text().toUInt() )
   {
      editor_msg( "dark red", QString( us_tr( "Notice: the actually number of jobs created (%1) is less than requested (%2)\n"
                                           "This is due to the fact that the selected files were evenly distributed among the jobs" ) ).arg( write_count ).arg( le_no_of_jobs->text().toUInt() ) );
   }
   if ( cb_for_mpi->isChecked() )
   {
      QString tarout = le_output_name->text() + ".tar";
      QStringList local_dest_files;
      for ( unsigned int i = 0; i < (unsigned int)dest_files.size(); i++ )
      {
         local_dest_files << QFileInfo( dest_files[ i ] ).fileName();
      }
      int result = ust.create( QFileInfo( tarout ).filePath(), local_dest_files );
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( tarout ).arg( ust.explain( result ) ) );
         return;
      }
      if ( !remove_files( dest_files ) )
      {
         return;
      }
      editor_msg( "blue", QString( us_tr( "Package: %1 created" ) ).arg( tarout ) );
   } else {
      editor_msg( "blue", 
                  dest_files
                  .replaceInStrings( QRegExp( "^" ), us_tr( "Package: " ) )
                  .replaceInStrings( QRegExp( "$" ), us_tr( " created" ) )
                  .join( "\n" ) );
   }
   editor_msg( "black", us_tr( "Package complete" ) );
}

void US_Hydrodyn_Cluster::clear_display()
{
   editor->clear( );
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
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

void US_Hydrodyn_Cluster::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
   editor->verticalScrollBar()->setValue(editor->verticalScrollBar()->maximum());
}

bool US_Hydrodyn_Cluster::remove_files( QStringList &filenames )
{
   errormsg = "";
   for ( unsigned int i = 0; i < (unsigned int)filenames.size(); i++ )
   {
      if ( !QFile::remove( filenames[ i ] ) )
      {
         editor_msg("red", QString(us_tr("Notice: remove of temporary file %1 failed")).arg( filenames[ i ] ));
         return false;
      } else {
         // editor_msg("dark gray", QString(us_tr("Removed temporary file %1")).arg( filenames[ i ] ));
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
      US_Hydrodyn::fixWinButtons( hcs );
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
      US_Hydrodyn::fixWinButtons( hcr );
      hcr->exec();
   }
   delete hcr;
}

void US_Hydrodyn_Cluster::update_output_name( const QString &cqs )
{
   if ( cqs.contains( QRegExp( "^common_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^common_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^nsa_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^nsa_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^bfnb_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^bfnb_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^bfnbpm_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^bfnbpm_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^oned_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^oned_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( QRegExp( "^best_" ) ) )
   {
      QString qs = cqs;
      qs.replace( QRegExp( "^best_" ), "" );
      le_output_name->setText( qs );
   }
   if ( cqs.contains( "_out", Qt::CaseInsensitive ) )
   {
      QString qs = cqs;
      qs.replace( "_out", "", Qt::CaseInsensitive );
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
   US_Hydrodyn::fixWinButtons( hcc );
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
      US_Hydrodyn::fixWinButtons( hcs );
      hcs->exec();
   }
   delete hcs;
}

bool US_Hydrodyn_Cluster::dup_in_submitted_or_completed()
{
   if ( submitted_jobs.count( le_output_name->text() ) )
   {
      US_Static::us_message( us_tr( "Please note:" ), 
                            us_tr( "There is a previously submitted job with the same name\n"
                                "Please change the output base name (job identifier) to a unique value\n"
                                "or cancel the submitted job\n") );
      return true;
   }
   if (
       completed_jobs.count( le_output_name->text() ) ||
       results_jobs.count( le_output_name->text() )
       )
   {
      US_Static::us_message( us_tr( "Please note:" ), 
                            us_tr( "There is a previously completed job with the same name\n"
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
      if ( !active_additional_methods().size() )
      {
         le_no_of_jobs->setText( QString( "%1" )
                                 .arg( cb_split_grid->isChecked() &&
                                       lb_target_files->count() ?
                                       selected_files.size() * lb_target_files->count() :
                                       selected_files.size() ) );
      } 
   }
}

void US_Hydrodyn_Cluster::update_validator()
{
   le_no_of_jobs->setValidator( 0 );
   if ( le_no_of_jobs_qv )
   {
      delete le_no_of_jobs_qv;
   }

   unsigned int max_jobs;
   if ( active_additional_methods().size() )
   {
      max_jobs = 16384;
   } else {
      unsigned int tot_dmd_file_count = 0;
      for ( unsigned int i = 0; i < (unsigned int)selected_files.size(); i++ )
      {
         if ( batch_window->cb_dmd->isChecked() &&
              dmd_entry_count( QFileInfo( selected_files[ i ] ).fileName() ) > 1 )
         {
            tot_dmd_file_count += dmd_entry_count( QFileInfo( selected_files[ i ] ).fileName() );
         } else {
            tot_dmd_file_count++;
         }
      }
      // cout << QString( "tot dmd file count %1\n" ).arg( tot_dmd_file_count );

      max_jobs = 
         ( cb_split_grid->isChecked() && lb_target_files->count() ) ?
         tot_dmd_file_count * lb_target_files->count() : tot_dmd_file_count;
                           
      max_jobs *= advanced_addition().size();
   }

   le_no_of_jobs_qv = new QIntValidator( 1, 
                                         max_jobs,
                                         this
                                         );
   le_no_of_jobs ->setValidator( le_no_of_jobs_qv );
   lbl_no_of_jobs->setText( QString(us_tr( "Number of jobs (maximum %1):" )).arg( max_jobs ) );
   if ( le_no_of_jobs->text().toUInt() > max_jobs )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( max_jobs ) );
   }
   if ( le_no_of_jobs->text().toUInt() < 1 )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( 1 ) );
   }
}

void US_Hydrodyn_Cluster::split_grid()
{
   update_validator();
   if ( cb_for_mpi->isChecked() && cb_split_grid->isChecked() && lb_target_files->count() )
   {
      le_no_of_jobs->setText( QString( "%1" ).arg( selected_files.size() * lb_target_files->count() ) );
   }
}

void US_Hydrodyn_Cluster::advanced()
{
   US_Hydrodyn_Cluster_Advanced *hca = 
      new US_Hydrodyn_Cluster_Advanced(
                                       csv_advanced,
                                       us_hydrodyn,
                                       this );
   US_Hydrodyn::fixWinButtons( hca );
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
   US_Hydrodyn::fixWinButtons( hcd );
   hcd->exec();
   delete hcd;
   if ( ((US_Hydrodyn *)us_hydrodyn)->dmd_failed_validation ) {
      QStringList errors;
      for ( auto it = ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports.begin();
            it != ((US_Hydrodyn *)us_hydrodyn)->dmd_all_pdb_prepare_reports.end();
            ++it ) {
         if ( it->second.count( "errors" ) ) {
            errors << it->first + " : " + it->second[ "errors" ].join( "\n" );
         }
      }
               
      QMessageBox::warning(
                           this
                           ,us_tr( windowTitle() )
                           ,us_tr("There were errors found preventing DMD from running:\n" + errors.join("\n") )
                           ,QMessageBox::Ok
                           ,QMessageBox::NoButton
                           ,QMessageBox::NoButton
                           );

      QMetaObject::invokeMethod(this, "close", Qt::QueuedConnection);
      return;
   }
   update_enables();
   update_validator();
}

bool US_Hydrodyn_Cluster::any_advanced()
{
   // cout << "any_advanced()\n";
   if ( !csv_advanced.data.size() )
   {
      // cout << "any_advanced() false: no data\n";
      return false;
   }

   for ( unsigned int i = 0; i < (unsigned int)csv_advanced.data.size(); i++ )
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

   for ( unsigned int i = 0; i < (unsigned int)csv_advanced.data.size(); i++ )
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

   for ( unsigned int i = 0; i < (unsigned int)csv_advanced.data.size(); i++ )
   {
      for ( unsigned int i = 0; i < (unsigned int)csv_advanced.data.size(); i++ )
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
         if ( csv_advanced.data[ it->first ][ 0 ] == "WAT excluded volume" )
         {
            tag += "swh";
            out += "SwhExclVol      ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "Crysol: contrast of hydration shell" )
         {
            tag += "hs";
            out += "CrysolChs       ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "Crysol: average atomic radius" )
         {
            tag += "ra";
            out += "CrysolRa       ";
            ok = true;
         }
         if ( csv_advanced.data[ it->first ][ 0 ] == "Crysol: Excluded volume" )
         {
            tag += "ev";
            out += "CrysolEv       ";
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

QString US_Hydrodyn_Cluster::dmd_base_addition( QStringList &base_source_files, QString common_prefix )
{
   QStringList files;
   QString dmd_base_dir = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "dmd" + SLASH;

   files 
      << "DMD_NA_allatom.par"
      << "PRO_RNA_MOL.DMDParam"
      << "allatom.par"
      << "medutop.pro"
      ;

   QString out;
   for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
   {
      out += QString( "DMDSupportFile  %1\n" )
         .arg( common_prefix + files[ i ] );
   }

   files.replaceInStrings( QRegExp( "^" ), dmd_base_dir );
   for ( unsigned int i = 0; i < (unsigned int)files.size(); i++ )
   {
      base_source_files << files[ i ];
   }

   return out;
}

unsigned int US_Hydrodyn_Cluster::dmd_entry_count( QString inputfile )
{
   // find matching files in csv_dmd and compare with selected files
   // for current file, create DMDTime/Temp/Relax/Run commands
   
   vector < unsigned int > active_csv_rows;
   // bool ok = false;
   for ( unsigned int i = 0; i < (unsigned int)csv_dmd.prepended_names.size(); i++ )
   {
      if ( inputfile == csv_dmd.prepended_names[ i ] &&
           csv_dmd.data[ i ].size() > 5 &&
           csv_dmd.data[ i ][ 1 ] == "Y" )
      {
         active_csv_rows.push_back( i );
         // ok = true;
      }
   }
   return active_csv_rows.size();
}

QString US_Hydrodyn_Cluster::dmd_file_addition( QString inputfile, 
                                                QString /* outputfile */,
                                                int use_entry )
{
   QString out;

   // find matching files in csv_dmd and compare with selected files
   // for current file, create DMDTime/Temp/Relax/Run commands
   
   vector < unsigned int > active_csv_rows;
   bool ok = false;
   for ( unsigned int i = 0; i < (unsigned int)csv_dmd.prepended_names.size(); i++ )
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

   if ( use_entry != -1 )
   {
      if ( use_entry >= (int) active_csv_rows.size() )
      {
         editor_msg( "red", QString( us_tr( "Internal Error: issue creating dmd entry for %1 position %2. Insufficient dmd entries (%3)\n" ) )
                     .arg( inputfile )
                     .arg( use_entry )
                     .arg( active_csv_rows.size() )
                     );
         return "";
      }
      active_csv_rows[ 0 ] = active_csv_rows[ use_entry ];
      active_csv_rows.resize( 1 );
   }

   if ( use_entry == -1 &&
        active_csv_rows.size() > 1 )
   {
      // must all have matching static ranges
      QString static_range = "";
      for ( unsigned int j = 0; j < (unsigned int) active_csv_rows.size(); j++ )
      {
         unsigned int i = active_csv_rows[ j ];
         QString this_static_range = "";
         if ( csv_dmd.data[ i ].size() > 11 )
         {
            this_static_range = csv_dmd.data[ i ][ 11 ];
            if ( j == 0 )
            {
               static_range = this_static_range;
            }
         }
         if ( this_static_range != static_range )
         {
            editor_msg( "red", QString( us_tr( "Error: multiple (%1) same named DMD (%2) entries with varying static ranges not supported for simultaneous execution on one core in one job\n" ) )
                        .arg( active_csv_rows.size() )
                        .arg( inputfile )
                        );
            return "";
         }
      }
   }

   // we should probably parameterize this, maybe system dependent
   out += "DMDBoxSpacing   +10\n";
   
   if ( active_csv_rows.size() < 1 )
   {
      editor_msg( "red", QString( us_tr( "Internal Error: no active DMD rows %1 position %2.\n" ) )
                  .arg( inputfile )
                  .arg( use_entry )
                  );
      return "";
   }

   {
      // the static range should all be identical if we got this far
      unsigned int i = active_csv_rows[ 0 ];
      if ( csv_dmd.data[ i ].size() > 11 &&
           !csv_dmd.data[ i ][ 11 ].isEmpty() )
      {
         out += QString( "DMDStatic       %1\n" ).arg( csv_dmd.data[ i ][ 11 ] );
      } else {
         out += QString( "DMDStatic       none\n" );
      }
   }

   out += 
      "DMDStripPdb\n"
      "DMDFindSS\n"
      "DMDPrepare\n";

   for ( unsigned int j = 0; j < (unsigned int)active_csv_rows.size(); j++ )
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
         if ( our_saxs_options->alt_hydration )
         {
            out += "HydrateAlt\n";
         }
         if ( our_saxs_options->hydration_rev_asa )
         {
            out += "HydrateRevASA\n";
         }
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

   for ( unsigned int i = 0; i < (unsigned int)selected_files.size(); i++ )
   {
      selected_map[ QFileInfo( selected_files[ i ] ).fileName() ] = true;
   }

   // see what's in csv_dmd
   for ( unsigned int i = 0; i < (unsigned int)csv_dmd.prepended_names.size(); i++ )
   {
      present_map[ csv_dmd.prepended_names[ i ] ] = true;
      if ( !selected_map.count( csv_dmd.prepended_names[ i ] ) )
      {
         // somethings present that isn't selected
         if ( csv_dmd.data[ i ].size() > 1 &&
              csv_dmd.data[ i ][ 1 ] == "Y" )
         {
            errormsg += QString( us_tr( "Error: %1 is marked as Active in the DMD settings, "
                                     "but the file is not present in the batch selected files\n" ) )
               .arg( csv_dmd.prepended_names[ i ] );
         } else {
            noticemsg += QString( us_tr( "Notice: %1 is in the DMD settings and is not Active, "
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
               errormsg += QString( us_tr( "Error: %1 is marked as Active but has insufficient DMD settings, "
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
                  errormsg += QString( us_tr( "Error: %1 is Active and does not have a positive Relax temp\n" ) )
                     .arg( csv_dmd.prepended_names[ i ] );
                  relax_ok = false;
               }
               if ( relax_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
               {
                  errormsg += QString( us_tr( "Error: %1 is Active and does not have a positive Relax time\n" ) )
                     .arg( csv_dmd.prepended_names[ i ] );
                  relax_ok = false;
               }
               if ( csv_dmd.data[ i ][ 5 ].toFloat() <= 0 &&
                    csv_dmd.data[ i ].size() > 9 &&
                    csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
               {
                  errormsg += QString( us_tr( "Error: %1 is Active and the Relax output count and the Run output count are not positive\n" ) )
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
                     errormsg += QString( us_tr( "Error: %1 is Active and does not have a positive Run temp\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
                  if ( run_is_on && csv_dmd.data[ i ][ 3 ].toFloat() <= 0 )
                  {
                     errormsg += QString( us_tr( "Error: %1 is Active and does not have a positive Run time\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
                  if ( run_is_on && csv_dmd.data[ i ][ 9 ].toFloat() <= 0 )
                  {
                     errormsg += QString( us_tr( "Error: %1 is Active and the Run output count is not positive\n" ) )
                        .arg( csv_dmd.prepended_names[ i ] );
                     run_ok = false;
                  }
               }
               if ( !run_ok &&
                    !relax_ok )
               {
                  errormsg += QString( us_tr( "Error: %1 is Active neither the Relax or Run is ok\n" ) )
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
         noticemsg += QString( us_tr( "Notice: %1 is a batch selected file but is not "
                                   "present in the DMD settings.\n" ) )
            .arg( it->first );
      }
   }

   return errormsg.isEmpty();
}

bool US_Hydrodyn_Cluster::active_additional_experimental_data()
{
   QStringList qsl = active_additional_methods();
   for ( int i = 0; i < ( int ) qsl.size(); i++ )
   {
      if ( cluster_additional_methods_use_experimental_data.count( qsl[ i ] ) )
      {
         return true;
      }
   }
   return false;
}

void US_Hydrodyn_Cluster::update_enables()
{
   unsigned int number_active;
   if ( active_additional_methods().size() )
   {

      le_no_of_jobs  ->setEnabled( true );
      pb_create_pkg  ->setEnabled( true );
      pb_add_target  ->setEnabled( active_additional_experimental_data() );
      pb_clear_target->setEnabled( pb_add_target->isEnabled() );
      le_output_name ->setEnabled( true );
      pb_create_pkg  ->setEnabled( true );
      cb_for_mpi     ->setEnabled( true );
      pb_dmd         ->setEnabled( false );
      return;
   } else {
      pb_add_target  ->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
      pb_clear_target->setEnabled( create_enabled && batch_window->cb_iqq->isChecked() );
      le_no_of_jobs  ->setEnabled( create_enabled );
      if ( batch_window->cluster_no_of_jobs.toUInt() &&
           batch_window->cluster_no_of_jobs.toUInt() <= (unsigned int)selected_files.size() )
      {
         le_no_of_jobs->setText( batch_window->cluster_no_of_jobs );
      }
      if ( cb_for_mpi->isChecked() )
      {
         le_no_of_jobs->setText( QString( "%1" ).arg( selected_files.size() ) );
      }
      le_output_name->setEnabled( create_enabled );
      pb_create_pkg ->setEnabled( create_enabled );
      cb_for_mpi    ->setEnabled( create_enabled );
      pb_dmd        ->setEnabled( batch_window->cb_dmd->isChecked() );
   }
      
   if ( batch_window->cb_dmd->isChecked() )
   {
      editor_msg( "black", us_tr( "\nChecking DMD settings:" ) );
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
      if ( number_active < (unsigned int)selected_files.size() &&
           !batch_window->cb_somo->isChecked() && 
           !batch_window->cb_grid->isChecked() && 
           !batch_window->cb_hydro->isChecked() && 
           !batch_window->cb_prr->isChecked() && 
           !batch_window->cb_iqq->isChecked() )
      {
         editor_msg( "red", us_tr( "For DMD only runs, all batch selected files must be accounted for and active in the DMD settings" ) );
         pb_create_pkg->setEnabled( false );
         return;
      }         
      editor_msg( "black", us_tr( "DMD settings ok." ) );
      pb_create_pkg->setEnabled( false );
   } 
   pb_create_pkg->setEnabled( create_enabled );
}

QString US_Hydrodyn_Cluster::options_summary()
{
   QString prefix =
      QString( us_tr( "Number of selected files: %1\n"
                   "Options summary:" ) )
      .arg( selected_files.size() );

   bool any_options = false;

   QString qs;

   if ( active_additional_methods().size() )
   {
      prefix = us_tr( "Options summary: " );
      qs += active_additional_methods().join( " " ).toUpper() + "\n";
      qs += us_tr( "Note: current active methods take precedence.\n"
                "Disable under the \"Other methods\" button.\n" );
      return prefix + qs;
   }

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

bool US_Hydrodyn_Cluster::corrupt_config()
{
   switch (
           QMessageBox::question(
                                 this,
                                 windowTitle() + us_tr( " : Configuration file corrupt" ),
                                 QString( us_tr( "The cluster configuration file was found to be corrupt.\n"
                                              "Would you like to recreate it from the template file?\n"
                                              "Answering \"Yes\" is recommended." ) ),
                                 QMessageBox::Yes, 
                                 QMessageBox::No,
                                 QMessageBox::NoButton
                                 ) )
   {
   case QMessageBox::Yes : 
      {
         QDir::setCurrent( pkg_dir );
         
         QString configfile        = "config";
         QString configcorruptfile = "config.corrupt";
      
         US_File_Util ufu;

         if ( !ufu.move( configfile, configcorruptfile, true ) )
         {
            editor_msg( "red", QString( us_tr( "Error: Could not rename corrupt cluster configuration file:" + ufu.errormsg + "\nCheck permissions or manually remove: \"%1\"") )
                        .arg( pkg_dir + QDir::separator() + configfile ) );
            return false;
         } else {
            editor_msg( "blue", 
                        QString( us_tr( "Notice: Renamed corrupt cluster configuration file to \"%1\"." ) )
                        .arg( configcorruptfile ) );
         }
         return read_config();
      }
      break;
   case QMessageBox::No : 
   default :
      return false;
      break;
   }
   return false;
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
      // try and copy over cluster.config
      US_File_Util ufu;
      QString ref_config = USglobal->config_list.system_dir + "/etc/cluster.config";
      if ( !ufu.copy( ref_config, configfile ) )
      {
         errormsg = QString( us_tr( "Error: Can not create blank cluster configuration file:" + ufu.errormsg ) );
         return false;
      } else {
         editor_msg( "blue", us_tr( "Notice: Created default cluster configuration file." ) );
      }
   } 

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = "Error: Cluster configuration file is not readable";
      return false;
   }

   QTextStream ts( &f );
   
   if ( ts.atEnd() )
   {
      errormsg = "Error: Cluster configuration file: premature end of file";
      f.close();
      return false;
   }

   cluster_config.clear( );
   cluster_systems.clear( );
   cluster_stage_to_system.clear( );
   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );
   QRegExp rx_valid  ( 
                      "^("
                      "userid|"
                      "userpw|"
                      "useremail|"
                      "server|"
                      "manage|"
                      "system|"
                      "type|"
                      "corespernode|"
                      "maxcores|"
                      "runtime|"
                      "maxruntime|"
                      "queue|"
                      "executable|"
                      "stage|"
                      "ftp"
                      ")$"
                      );

   QRegExp rx_req_arg  ( 
                        "^("
                        "server|"
                        "manage|"
                        "system|"
                        "type|"
                        "corespernode|"
                        "maxcores|"
                        "runtime|"
                        "maxruntime|"
                        "queue|"
                        "executable|"
                        "stage|"
                        "ftp"
                        ")$"
                         );
   QRegExp rx_config ( 

                      "^("
                      "userid|"
                      "userpw|"
                      "useremail|"
                      "server|"
                      "manage"
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
                      "executable|"
                      "stage|"
                      "ftp"
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

      QStringList qsl = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );

      if ( !qsl.size() )
      {
         continue;
      }

      QString option = qsl[ 0 ].toLower();

      if ( rx_valid.indexIn( option ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( configfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return corrupt_config();
      }

      if ( rx_req_arg.indexIn( option ) != -1 && qsl.size() < 2 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( configfile )
            .arg( line );
         return corrupt_config();
      }

      qsl.pop_front();
      if ( !qsl.size() )
      {
         continue;
      }

      if ( rx_config.indexIn( option ) != -1 )
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

            return corrupt_config();
         }

         map < QString, QString > tmp_system;
         cluster_systems[ last_system ] = tmp_system;
         continue;
      }

      if ( rx_systems.indexIn( option ) != -1 )
      {
         if ( last_system.isEmpty() )
         {
            errormsg = QString( "Error reading %1 line %2 : system must be specified before %3 " )
               .arg( configfile )
               .arg( line )
               .arg( option );
            return corrupt_config();
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
      return corrupt_config();
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

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errormsg = "can not create config file";
      return false;
   }

   QTextStream ts( &f );
   
   ts << out;

   f.close();

   return true;
}

void US_Hydrodyn_Cluster::additional()
{
   US_Hydrodyn_Cluster_Additional *hca = 
      new US_Hydrodyn_Cluster_Additional(
                                         us_hydrodyn,
                                         this 
                                         );
   US_Hydrodyn::fixWinButtons( hca );
   hca->exec();
   delete hca;
   update_enables();
   update_validator();
   editor_msg( "dark blue", options_summary() );
}

QStringList US_Hydrodyn_Cluster::active_additional_methods()
{
   QStringList methods;
   for ( map < QString, bool >::iterator it = cluster_additional_methods_options_active->begin();
         it != cluster_additional_methods_options_active->end();
         it++ )
   {
      if ( it->second )
      {
         methods << it->first;
      }
   }
   return methods;
}

QStringList US_Hydrodyn_Cluster::additional_method_files( QString method )
{
   QStringList files;
   if ( cluster_additional_methods_options_active->count( method ) &&
        (*cluster_additional_methods_options_active)[ method ] &&
        cluster_additional_methods_options_selected->count( method ) )
   {
      for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ method ].begin();
            it != (*cluster_additional_methods_options_selected)[ method ].end();
            it++ )
      {
         if ( it->first.contains( "file" ) )
         {
            files << it->second;
         }
      }
   }
   return files;
}

QString US_Hydrodyn_Cluster::additional_method_package_text( QString method )
{
   QString text;
   if ( cluster_additional_methods_options_active->count( method ) &&
        (*cluster_additional_methods_options_active)[ method ] &&
        cluster_additional_methods_options_selected->count( method ) )
   {
      for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ method ].begin();
            it != (*cluster_additional_methods_options_selected)[ method ].end();
            it++ )
      {
         if ( it->first.contains( "file" ) )
         {
            text += QString( "%1\t%2\n" ).arg( it->first ).arg( QFileInfo( it->second ).fileName() );
         } else {
            text += QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );
         }
      }
   }
   return text;
}


bool US_Hydrodyn_Cluster::active_additional_mpi_mix_issue()
{
   // we can not mix real parallel jobs with trivially parallel jobs
   bool has_trivial_jobs  = false;
   bool has_parallel_jobs = false;
   QStringList methods = active_additional_methods();
   for ( int i = 0; i < ( int ) methods.size(); i++ )
   {
      if ( cluster_additional_methods_parallel_mpi.count( methods[ i ] ) )
      {
         has_parallel_jobs = true;
      } else {
         has_trivial_jobs = true;
      }
   }
   return has_parallel_jobs && has_trivial_jobs;
}

bool US_Hydrodyn_Cluster::active_additional_prepend_issue()
{
   // we can not mix real parallel jobs with trivially parallel jobs
   QString first_prepend;
   bool    first_one_pdb_exactly = false;
   QStringList methods = active_additional_methods();
   for ( int i = 0; i < ( int ) methods.size(); i++ )
   {
      if ( !i )
      {
         first_prepend = 
            cluster_additional_methods_prepend.count( methods[ i ] ) ? 
            cluster_additional_methods_prepend[ methods[ i ] ] : "";
         first_one_pdb_exactly = 
            cluster_additional_methods_one_pdb_exactly.count( methods[ i ] ) ? 
            true : false;
      } else {
         QString this_prepend =
            cluster_additional_methods_prepend.count( methods[ i ] ) ? 
            cluster_additional_methods_prepend[ methods[ i ] ] : "";
         bool this_one_pdb_exactly = 
            cluster_additional_methods_one_pdb_exactly.count( methods[ i ] ) ? 
            true : false;
         if ( this_prepend != first_prepend ||
              this_one_pdb_exactly != first_one_pdb_exactly )
         {
            return true;
         }
      }
   }
   return false;
}

QString US_Hydrodyn_Cluster::job_prepend_name()
{
   QStringList methods = active_additional_methods();
   QString prepend =
      methods.size() && cluster_additional_methods_prepend.count( methods[ 0 ] ) ? 
      cluster_additional_methods_prepend[ methods[ 0 ] ] : "";
   if ( !prepend.isEmpty() &&
        cb_for_mpi->isChecked() )
   {
      QStringList methods = active_additional_methods();
      if ( methods.size() &&
           cluster_additional_methods_job_multiplier.count( methods[ 0 ] ) )
      {
         prepend += QString( "p%1_" )
            .arg( le_no_of_jobs->text().toUInt() * cluster_additional_methods_job_multiplier[ methods[ 0 ] ] );
      } else {
         prepend += QString( "p%1_" ).arg( le_no_of_jobs->text() );
      }
   }
   return prepend;
}

void US_Hydrodyn_Cluster::create_additional_methods_pkg( QString base_dir,
                                                         QString filename, 
                                                         QString common_prefix,
                                                         bool    use_extension )
{
   editor_msg( "dark blue" , us_tr( "Files:\n" + additional_method_files        ( active_additional_methods()[ 0 ] ).join( "\n" ) + "\n" ) );
   editor_msg( "dark blue", us_tr( "Text:\n" + additional_method_package_text ( active_additional_methods()[ 0 ] ) + "\n" ) );

   if ( active_additional_mpi_mix_issue() )
   {
      editor_msg( "red"     , 
                   us_tr( 
                      "Multiple other methods selected that are currently incompatible for a single job package\n"
                      "(Some of the jobs are trivially parallel and others are parallel jobs with interprocess communications)"
                      ) );
      return;
   }

   if ( active_additional_prepend_issue() )
   {
      editor_msg( "red"     , 
                   us_tr( 
                      "Multiple other methods selected that are currently incompatible for a single job package\n"
                      "(Methods require differing namimg conventions)"
                      ) );
      return;
   }


   QStringList methods = active_additional_methods();

   if ( !methods.size() )
   {
      editor_msg( "red"     , 
                   us_tr( 
                      "Internal error: no other methods selected"
                      ) );
      return;
   }

   if ( selected_files.size() != 1 ) 
   {
      for ( int i = 0; i < ( int ) methods.size(); i++ )
      {
         if ( cluster_additional_methods_one_pdb_exactly.count( methods[ i ] ) )
         {
            editor_msg( "red"     , 
                        us_tr( 
                           QString( "The addtional method %1 requires exactly one pdb to be selected" )
                           .arg( methods[ i ] )
                           ) );
            return;
         }
      }
   }

   if ( methods.size() > 1 )
   {
      for ( int i = 0; i < ( int ) methods.size(); i++ )
      {
         if ( cluster_additional_methods_must_run_alone.count( methods[ i ] ) )
         {
            editor_msg( "red"     , 
                        us_tr( 
                           "An active additional method is required to be run without other active active additional methods"
                           ) );
            return;
         }
      }
   }
            
   if ( cluster_additional_methods_parallel_mpi.count( methods[ 0 ] ) )
   {
      return create_additional_methods_parallel_pkg ( base_dir,
                                                      filename, 
                                                      common_prefix,
                                                      use_extension );
   }

   if ( batch_window->cb_mm_all->isChecked() )
   {
      QString errors;
      for ( int i = 0; i < ( int ) methods.size(); i++ )
      {
         if (
             cluster_additional_methods_modes.count( "no_multi_model_pdb" ) &&
             cluster_additional_methods_modes[ "no_multi_model_pdb" ].count( methods[ i ] ) )
         {
            errors += methods[ i ] + " ";
         }
      }
      if ( !errors.isEmpty() )
      {
            US_Static::us_message( us_tr( "Please note:" ), 
                                  QString( 
                                          us_tr( "You have selected to process all models in multi-model PDBs with the %1additional method(s).\n"
                                              "If you are including multi-model PDBs, only the first model's results will be produced.\n"
                                              "If you wish to process all the individual models, split the multi model PDB into individual files\n"
                                              "You can split the multi-model PDB into individual files using the PDB Editor / Split button." ) )
                                  .arg( errors.toUpper() )
                                  );
         
      }
   }

   QString base = 
      "# us_saxs_cmds_t iq controlfile\n"
      "# blank lines ok, format token <params>\n"
      "\n";

   if ( !lb_target_files->count() )
   {
      bool not_ok = false;
      for ( int i = 0; i < ( int ) methods.size(); i++ )
      {
         if ( cluster_additional_methods_require_experimental_data.count( methods[ i ] ) )
         {
            editor_msg( "red"     , 
                        QString( us_tr( "Selected other method %1 requires experimental data and none are listed" ) )
                        .arg( methods[ i ].toUpper() ) );
            not_ok = true;
         }
      }
      if ( not_ok )
      {
         return;
      }

      // target files implies extra control info,
      // possibly some redundancy, but add them anyway:
      // (could check for method-specific additions
      base += 
         QString( "TargeteDensity      %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->misc.target_e_density );
      base += 
         QString( "OutputFile      %1\n" ).arg( le_output_name->text() );
      // base += QString( "Output          %1\n" )
      // .arg( batch_window->cb_csv_saxs->isChecked() ? "csv" : "ssaxs" );
   }

   QStringList base_source_files;

   // take care of all input files first
   // this is a problem for dammin etc, as they can't reference a ../common directory
   // later we could force copy over
   // -------------------------------------------------
   // for ( int m = 0; m < ( int )methods.size(); m++ )
   // {
   // if ( cluster_additional_methods_options_selected->count( methods[ m ] ) )
   // {
   // for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ methods[ m ] ].begin();
   // it != (*cluster_additional_methods_options_selected)[ methods[ m ] ].end();
   // it++ )
   // {
   // if ( it->first.contains( "file" ) )
   // {
   // base += 
   // QString( "%1\t%2\n" )
   // .arg( it->first )
   // .arg( common_prefix + QFileInfo( it->second ).fileName() );
   // base_source_files << it->second;
   // }
   // }
   // }
   // }
   // ------------------------------------------------- 

   for ( int m = 0; m < ( int )methods.size(); m++ )
   {
      if ( 
          cluster_additional_methods_modes.count( "additional_processing_global" ) &&
          cluster_additional_methods_modes[ "additional_processing_global" ].count( methods[ m ] ) )
      {
         if ( !additional_processing(
                                     base,
                                     base_source_files,
                                     "additional_processing_global",
                                     methods[ m ]
                                     ) )
         {
            editor_msg( "red", us_tr( "Internal error:additional processing per file error" ) );
         }
      }
   }

   if ( !cb_for_mpi->isChecked() &&
        !copy_files_to_pkg_dir( base_source_files ) )
   {
      editor_msg( "red", errormsg );
      return;
   }

   QString      out = base;
   QStringList  source_files;
   QStringList  source_files_to_clear;
   QStringList  dest_files;
   QStringList  use_selected_files;

   unsigned int write_count = 0;
   bool         last_unwritten;

   int jobs = le_no_of_jobs->text().toInt();

   unsigned int extension_count = jobs;
   unsigned int extension_count_length = QString("%1").arg( extension_count ).length();
   map < QString, bool > already_added;

   US_Tar       ust;
   US_Gzip      usg;

   for ( int i = 0; i < jobs; i++ )
   {
      for ( int m = 0; m < ( int )methods.size(); m++ )
      {
         if ( cb_for_mpi->isChecked() &&
              cluster_additional_methods_require_sleep.count( methods[ i ] ) )
         {
            out += QString( "sleep\t%1\n" ).arg( i );
         }

         if ( cluster_additional_methods_add_selected_files.count( methods[ m ] ) )
         {
            for ( unsigned int i = 0; i < (unsigned int)selected_files.size(); i++ )
            {
               out += QString( "InputFile       %1\n" ).arg( QFileInfo( selected_files[ i ] ).fileName() );
               if ( !already_added.count( selected_files[ i ] ) )
               {
                  source_files << selected_files[ i ];
                  already_added[ selected_files[ i ] ] = true;
               }
            }            
         }
            
         if ( cluster_additional_methods_options_selected->count( methods[ m ] ) )
         {
            for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ methods[ m ] ].begin();
                  it != (*cluster_additional_methods_options_selected)[ methods[ m ] ].end();
                  it++ )
            {
               if ( !it->first.contains( "file" ) )
               {
                  out += QString( "%1\t%2\n" )
                     .arg( it->first )
                     .arg( it->second );
               } else {
                  if ( cluster_additional_methods_modes[ "acceptcommon" ].count( methods[ m ] ) )
                  {
                     out += 
                        QString( "%1\t%2\n" )
                        .arg( it->first )
                        .arg( common_prefix + QFileInfo( it->second ).fileName() );
                     if ( !already_added.count( it->second ) )
                     {
                        base_source_files << it->second;
                        already_added[ it->second ] = true;
                     }
                  } else {
                     out += 
                        QString( "%1\t%2\n" )
                        .arg( it->first )
                        .arg( QFileInfo( it->second ).fileName() );
                     if ( !already_added.count( it->second ) )
                     {
                        source_files << it->second;
                        already_added[ it->second ] = true;
                     }
                  }
               }
            }
            if ( cluster_additional_methods_modes.count( "one_run_per_file"  ) &&
                 cluster_additional_methods_modes[ "one_run_per_file" ].count( methods[ m ] ) )
            {
               for ( unsigned int j = 0; j < (unsigned int)selected_files.size(); ++j )
               {
                  if ( (int)j % jobs == i )
                  {
                     QString use_filename = selected_files[ j ];
                     if ( cluster_additional_methods_modes[ "inputfile_pat_addendum" ].count( methods[ m ] ) )
                     {
                        use_filename = QFileInfo( selected_files[ j ] ).baseName() + "_pat" + "." + QFileInfo( selected_files[ j ] ).completeSuffix();
                     }

                     out += QString( "%1 %2\n" )
                        .arg( cluster_additional_methods_modes.count( "inputfilenoread"  ) &&
                              cluster_additional_methods_modes[ "inputfilenoread" ].count( methods[ m ] ) ?
                              "InputFileNoRead" : "InputFile     " )
                        .arg( QFileInfo( use_filename ).fileName() );
                     if ( !already_added.count( selected_files[ j ] ) )
                     {
                        source_files_to_clear << selected_files[ j ];
                        already_added[ selected_files[ j ] ] = true;
                        if ( 
                            cluster_additional_methods_modes.count( "additional_processing_per_file" ) &&
                            cluster_additional_methods_modes[ "additional_processing_per_file" ].count( methods[ m ] ) )
                        {
                           if ( !additional_processing(
                                                       out,
                                                       source_files_to_clear,
                                                       "additional_processing_per_file",
                                                       methods[ m ], 
                                                       selected_files[ j ] 
                                                       ) )
                           {
                              editor_msg( "red", us_tr( "Internal error:additional processing per file error" ) );
                           }
                        }
                        out += methods[ m ] + "run\n";
                        out += "\n";
                     }
                  }
               }            
            } else {
               out += methods[ m ] + "run\n";
               out += "\n";
            }               
         }
      }
   
      // if ( !( ( i + 1 ) % jobs ) )
      {
         last_unwritten = false;
         write_count++;
         QString ext = "";
         if ( use_extension )
         {
            ext = QString("%1").arg( write_count );
            while ( (unsigned int) ext.length() < extension_count_length )
            {
               ext = "0" + ext;
            }
         }
         QString use_file = QString( "%1%2" ).arg( filename ).arg( use_extension ? QString("_p%1").arg( ext ) : "" );
         cout << use_file << endl;
         QFile f( use_file );
         if ( f.open( QIODevice::WriteOnly ) )
         {
            QTextStream ts( &f );
            ts << out;
            if ( !cluster_additional_methods_no_tgz_output.count( methods[ 0 ] ) )
            {
               ts << QString( "TgzOutput       %1_out.tgz\n" ).arg(
                                                                   le_output_name->text() + 
                                                                   ( use_extension ? QString("_p%1").arg( ext ) : "" )
                                                                   );
            }
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
         if ( source_files_to_clear.size() )
         {
            if ( !copy_files_to_pkg_dir( source_files_to_clear ) )
            {
               editor_msg( "red", errormsg );
               return;
            }
         }
         
         // build tar archive
         QStringList list;
         QStringList to_tar_list;
         QStringList remove_file_list;
         to_tar_list << QFileInfo( use_file ).fileName();
         if ( !cb_for_mpi->isChecked() )
         {
            for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
            {
               to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
            }
         }
         for ( unsigned int i = 0; i < (unsigned int)source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( source_files[ i ] ).fileName();
            remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
         }
         for ( unsigned int i = 0; i < (unsigned int)source_files_to_clear.size(); i++ )
         {
            to_tar_list << QFileInfo( source_files_to_clear[ i ] ).fileName();
            remove_file_list << pkg_dir + SLASH + QFileInfo( source_files_to_clear[ i ] ).fileName();
         }
         remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();

         QString tar_name = use_file + ".tar";
         int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
         cout << "tar_name:" << tar_name << endl;
         cout << "to tar:\n" << to_tar_list.join("\n") << endl;
         
         if ( result != TAR_OK )
         {
            editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
            return;
         }
         editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );

         result = usg.gzip( tar_name );
         if ( result != GZIP_OK )
         {
            editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
         
         if ( !cb_for_mpi->isChecked() )
         {
            source_files .clear( );
            already_added.clear( );
         }
         source_files_to_clear.clear( );

         out = base;
         // out += QString( "sleep\t%!\n" ).arg( i );
      }
   }
   if ( last_unwritten )
   {
      write_count++;
      QString ext = "";
      if ( use_extension )
      {
         ext = QString("%1").arg( write_count );
         while ( (unsigned int) ext.length() < extension_count_length )
         {
            ext = "0" + ext;
         }
      }
      QString use_file = QString( "%1%2" ).arg( filename ).arg( use_extension ? QString("_p%1").arg( ext ) : "" );
      cout << use_file << endl;
      QFile f( use_file );
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << out;
         ts << QString( "TgzOutput       %1_out.tgz\n" ).arg(
                                                             le_output_name->text() + 
                                                             ( use_extension ? QString("_p%1").arg( ext ) : "" )
                                                             );
         f.close();
         editor_msg( "dark gray", QString("Created: %1").arg( use_file ) );
      }
      
      // copy ne files to base_dir
      if ( !copy_files_to_pkg_dir( source_files ) )
      {
         editor_msg( "red", errormsg );
         return;
      }
      if ( source_files_to_clear.size() )
      {
         if ( !copy_files_to_pkg_dir( source_files_to_clear ) )
         {
            editor_msg( "red", errormsg );
            return;
         }
      }

      // build tar archive
      QStringList qsl;
      QStringList to_tar_list;
      QStringList remove_file_list;
      to_tar_list << QFileInfo( use_file ).fileName();
      if ( !cb_for_mpi->isChecked() )
      {
         for ( unsigned int i = 0; i < (unsigned int) base_source_files.size(); i++ )
         {
            to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
         }
      }
      for ( unsigned int i = 0; i < (unsigned int)source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( source_files[ i ] ).fileName();
         remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
      }
      for ( unsigned int i = 0; i < (unsigned int)source_files_to_clear.size(); i++ )
      {
         to_tar_list << QFileInfo( source_files_to_clear[ i ] ).fileName();
         remove_file_list << pkg_dir + SLASH + QFileInfo( source_files_to_clear[ i ] ).fileName();
      }
      remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();

      QString tar_name = use_file + ".tar";
      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &qsl );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;
      
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
         return;
      }
      editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );

      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
      
      source_files         .clear( );
      source_files_to_clear.clear( );
      
      out = base;
   }

   if ( cb_for_mpi->isChecked() )
   {
      if ( !copy_files_to_pkg_dir( base_source_files ) )
      {
         editor_msg( "red", errormsg );
         return;
      }

      QStringList list;
      QStringList to_tar_list;
      for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
      }

      QString tar_name = QString( "%1%2common_%3.tar" )
         .arg( QFileInfo( filename ).path() )
         .arg( QDir::separator() )
         .arg( QFileInfo( filename ).fileName() )
         ;

      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;

      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( tar_name ).arg( ust.explain( result ) ) );
         return;
      }

      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
   }

   QStringList base_remove_file_list;
   for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
   {
      base_remove_file_list << pkg_dir + SLASH + QFileInfo( base_source_files[ i ] ).fileName();
   }
   remove_files( base_remove_file_list );
   cout << "written:" << write_count << endl;
   if ( write_count != le_no_of_jobs->text().toUInt() )
   {
      editor_msg( "dark red", QString( us_tr( "Notice: the actually number of jobs created (%1) is less than requested (%2)\n"
                                           "This is due to the fact that the selected files were evenly distributed among the jobs" ) ).arg( write_count ).arg( le_no_of_jobs->text().toUInt() ) );
   }
   if ( cb_for_mpi->isChecked() )
   {
      QString tarout = filename + ".tar";
      QStringList local_dest_files;
      for ( unsigned int i = 0; i < (unsigned int)dest_files.size(); i++ )
      {
         local_dest_files << QFileInfo( dest_files[ i ] ).fileName();
      }
      int result = ust.create( QFileInfo( tarout ).filePath(), local_dest_files );
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( tarout ).arg( ust.explain( result ) ) );
         return;
      }
      if ( !remove_files( dest_files ) )
      {
         return;
      }
      editor_msg( "blue", QString( us_tr( "Package: %1 created" ) ).arg( tarout ) );
   } else {
      editor_msg( "blue", 
                  dest_files
                  .replaceInStrings( QRegExp( "^" ), us_tr( "Package: " ) )
                  .replaceInStrings( QRegExp( "$" ), us_tr( " created" ) )
                  .join( "\n" ) );
   }
   editor_msg( "black", us_tr( "Package complete" ) );
}

void US_Hydrodyn_Cluster::create_additional_methods_parallel_pkg( QString /* base_dir */,
                                                                  QString filename, 
                                                                  QString /* common_prefix */,
                                                                  bool    /* use_extension */ )
{
   QStringList methods = active_additional_methods();

   if ( methods.size() && methods[ 0 ] == "bfnb" )
   {
      return create_additional_methods_parallel_pkg_bfnb( filename );
   }

   QString     unimplemented;
   QStringList base_source_files;

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

   if ( our_saxs_options->swh_excl_vol != 0e0 )
   {
      base += 
         QString( "SwhExclVol      %1\n" ).arg( our_saxs_options->swh_excl_vol );
   }
   if ( our_saxs_options->iq_target_ev != 0e0 )
   {
      base += 
         QString( "IqTargetEv      %1\n" ).arg( our_saxs_options->iq_target_ev );
   }
   if ( our_saxs_options->hybrid_radius_excl_vol )
   {
      base += 
         QString( "HybridRadiusExclVol\n" );
   }
   if ( our_saxs_options->set_iq_target_ev_from_vbar )
   {
      base += 
         QString( "SetIqTargetEvFromVbar\n" );
   }
   if ( our_saxs_options->use_iq_target_ev )
   {
      base += 
         QString( "UseIqTargetEv\n" );
   }

   if ( batch_window->cb_prr->isChecked() )
   {
      base += 
         QString( "prbinsize       %1\n" ).arg( our_saxs_options->bin_size );
      base += 
         QString( "prcurve         %1\n" ).arg( our_saxs_options->curve );
   }

   if ( batch_window->cb_hydrate && batch_window->cb_hydrate->isChecked() )
   {
      base += 
         QString( "HydrationFile   %1\n" ).arg( QFileInfo( our_saxs_options->default_rotamer_filename ).fileName() );
      base += 
         QString( "HydrationSCD    %1\n" ).arg( our_saxs_options->steric_clash_distance );
      base += 
         QString( "HydrationRSCD   %1\n" ).arg( our_saxs_options->steric_clash_recheck_distance );
      base_source_files << our_saxs_options->default_rotamer_filename;
   }

   if ( our_saxs_options->use_somo_ff )
   {
      base += 
         QString( "FFFile          %1\n" ).arg( QFileInfo( our_saxs_options->default_ff_filename ).fileName() );
      base_source_files << our_saxs_options->default_ff_filename;
   }

   if ( our_saxs_options->iq_exact_q )
   {
      base += 
         QString( "ExactQ\n" );
   }

//    if ( our_saxs_options->alt_ff )
//    {
//       base += 
//          QString( "TestingFF\n" );
//    }

   if ( our_saxs_options->five_term_gaussians )
   {
      base += 
         QString( "FiveTermGaussian\n" );
   }

   if ( our_saxs_options->iqq_use_atomic_ff )
   {
      base += 
         QString( "ExplicitH\n" );
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
         iqmethod = "foxs";
      }
      if ( our_saxs_options->saxs_iq_crysol )
      {
         iqmethod = "crysol";
      }
   }
   
   base += 
      QString( "IqMethod        %1\n" ).arg( iqmethod );

   base += QString( "Output          %1\n" )
      .arg( batch_window->cb_csv_saxs->isChecked() ? "csv" : "ssaxs" );

   if ( !unimplemented.isEmpty() )
   {
      editor_msg( "red", QString( "Can not create job files:\n%1" ).arg( unimplemented ) );
      return;
   }

   if ( !lb_target_files->count() )
   {
      bool not_ok = false;
      for ( int i = 0; i < ( int ) methods.size(); i++ )
      {
         if ( cluster_additional_methods_require_experimental_data.count( methods[ i ] ) )
         {
            editor_msg( "red"     , 
                        QString( us_tr( "Selected other method %1 requires experimental data and none are listed" ) )
                        .arg( methods[ i ].toUpper() ) );
            not_ok = true;
         }
      }
      if ( not_ok )
      {
         return;
      }
   } 

   // possibly some redundancy, but add them anyway:
   // (could check for method-specific additions
   base += 
      QString( "TargeteDensity      %1\n" ).arg( ((US_Hydrodyn *)us_hydrodyn)->misc.target_e_density );
   base += 
      QString( "OutputFile      %1\n" ).arg( le_output_name->text() );
      // base += QString( "Output          %1\n" )
      // .arg( batch_window->cb_csv_saxs->isChecked() ? "csv" : "ssaxs" );

   if ( !copy_files_to_pkg_dir( base_source_files ) )
   {
      editor_msg( "red", errormsg );
      return;
   }

   QString      out = base;
   QStringList  source_files;
   QStringList  dest_files;
   QStringList  use_selected_files;

   unsigned int write_count = 0;

   map < QString, bool > already_added;

   US_Tar       ust;
   US_Gzip      usg;

   // parallel methods are really only one job
   // we have to communicate this somehow to submission (?), i guess package naming, otherwise it will require submission to inspect the package
   // package here is one tgz

   int loop_count = lb_target_files->count()
      ? lb_target_files->count()
      : 1;

   for ( int i = 0; i < loop_count; i++ )
   {
      if ( lb_target_files->count() )
      {
         out += QString( "ExperimentGrid     %1\n" ).arg( QFileInfo( lb_target_files->item( i )->text() ).fileName() );
         if ( !already_added.count( QFileInfo( lb_target_files->item( i )->text() ).fileName() ) )
         {
            source_files << lb_target_files->item( i )->text();
         }
      }

      for ( int m = 0; m < ( int )methods.size(); m++ )
      {
         if ( cluster_additional_methods_add_selected_files.count( methods[ m ] ) )
         {
            for ( unsigned int i = 0; i < (unsigned int)selected_files.size(); i++ )
            {
               out += QString( "InputFile       %1\n" ).arg( QFileInfo( selected_files[ i ] ).fileName() );
               if ( !already_added.count( selected_files[ i ] ) )
               {
                  source_files << selected_files[ i ];
                  already_added[ selected_files[ i ] ] = true;
               }
            }            
         }

         if ( cluster_additional_methods_options_selected->count( methods[ m ] ) )
         {
            for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ methods[ m ] ].begin();
                  it != (*cluster_additional_methods_options_selected)[ methods[ m ] ].end();
                  it++ )
            {
               if ( !it->first.contains( "file" ) )
               {
                  out += QString( "%1\t%2\n" )
                     .arg( it->first )
                     .arg( it->second );
               } else {
                  out += 
                     QString( "%1\t%2\n" )
                     .arg( it->first )
                     .arg( QFileInfo( it->second ).fileName() );
                  if ( !already_added.count( it->second ) )
                  {
                     source_files << it->second;
                     already_added[ it->second ] = true;
                  }
               }
            }
            out += methods[ m ] + "run\n";
            out += "\n";
         }
      }
   }
   
   {
      write_count++;
      QString use_file = filename;
      cout << use_file << endl;
      QFile f( use_file );
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << out;
         if ( !cluster_additional_methods_no_tgz_output.count( methods[ 0 ] ) )
         {
            ts << QString( "TgzOutput       %1_out.tgz\n" ).arg( QFileInfo( use_file ).baseName() );
         }
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
      for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( base_source_files[ i ] ).fileName();
      }
      for ( unsigned int i = 0; i < (unsigned int)source_files.size(); i++ )
      {
         to_tar_list << QFileInfo( source_files[ i ] ).fileName();
         remove_file_list << pkg_dir + SLASH + QFileInfo( source_files[ i ] ).fileName();
      }
      remove_file_list << pkg_dir + SLASH + QFileInfo( use_file ).fileName();

      QString tar_name = use_file + ".tar";
      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;
         
      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
         return;
      }
      editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );

      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
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
      out = base;
      // out += QString( "sleep\t%!\n" ).arg( i );
   }

   QStringList base_remove_file_list;
   for ( unsigned int i = 0; i < (unsigned int)base_source_files.size(); i++ )
   {
      base_remove_file_list << pkg_dir + SLASH + QFileInfo( base_source_files[ i ] ).fileName();
   }
   remove_files( base_remove_file_list );
   cout << "written:" << write_count << endl;
   editor_msg( "blue", 
               dest_files
               .replaceInStrings( QRegExp( "^" ), us_tr( "Package: " ) )
               .replaceInStrings( QRegExp( "$" ), us_tr( " created" ) )
               .join( "\n" ) );
   // }
   editor_msg( "black", us_tr( "Package complete" ) );
}

static void combo_with_replacement(
                                   vector < int >            & elems,
                                   unsigned int                req_len,
                                   vector < int >            & pos,
                                   vector < vector < int > > & results,
                                   unsigned int                depth = 0,
                                   unsigned int                margin = 0
                                   )
{
   if ( depth >= req_len ) 
   {
      vector < int > result( pos.size() );
      for ( unsigned int ii = 0; ii < (unsigned int)pos.size(); ++ii )
      {
         result[ ii ] = elems[ pos[ ii ] ];
      }
      results.push_back( result );
      return;
   }

   for ( unsigned int ii = margin; ii < (unsigned int) elems.size(); ++ii ) 
   {
      pos[ depth ] = ii;
      combo_with_replacement( elems, req_len, pos, results, depth + 1, ii );
   }
   return;
}

void US_Hydrodyn_Cluster::create_additional_methods_parallel_pkg_bfnb( QString filename )
{
   QStringList methods = active_additional_methods();

   QString errors;

   if ( le_no_of_jobs->text().toUInt() < 2 )
   {
      errors += QString( us_tr( "Error: method %1 requires a minimum of 2 cores\n" ) ).arg( methods[ 0 ] );
   }

   if ( !lb_target_files->count() )
   {
      if ( cluster_additional_methods_require_experimental_data.count( methods[ 0 ] ) )
      {
         errors += 
            QString( us_tr( "Selected other method %1 requires experimental data and none are listed\n" ) )
            .arg( methods[ 0 ].toUpper() );
      }
   } 

   // (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmrayleighdrho" ] = ".1";
   if ( !(*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmrayleighdrho" ) )
   {
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmrayleighdrho" ] = ".425";
      editor_msg( "dark red", us_tr( "Notice: setting sample e density to protein average of .425" ) );
   }

   if ( !(*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmbufferedensity" ) )
   {
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmbufferedensity" ] = QString( "%1" ).arg( our_saxs_options->water_e_density );
      editor_msg( "dark red", QString( us_tr( "Notice: setting buffer e density to SAS Options value of %1" ) ).arg( our_saxs_options->water_e_density ) );
   }

   QString base = 
      "# us_saxs_cmds_t pm controlfile\n"
      "# blank lines ok, format token <params>\n"
      "\n";
   
   // we are going to have to take each experimental file, produce pmi, pmq, pme lines and pmbest
   // when "pmincrementally", run for subsets of pmtypes
   // when "pmallcombinations", run for each possible combo
   // suffix pmoutname appropriately (source data, model type)
   
   QStringList qsl_skip;
   qsl_skip 
      << "pmtypes"
      << "pmincrementally"
      << "pmallcombinations"
      << "pmapproxmaxdimension"
      ;

   map < QString, bool > skip;
   for ( int i = 0; i < (int) qsl_skip.size(); ++i )
   {
      skip[ qsl_skip[ i ] ] = true;
   }

   QString out_per_file;
   QStringList qsl_per_file;
   qsl_per_file 
      << "pmrayleighdrho"
      << "pmbufferedensity"
      ;

   map < QString, bool > per_file;
   for ( int i = 0; i < (int) qsl_per_file.size(); ++i )
   {
      per_file[ qsl_per_file[ i ] ] = true;
   }

   QStringList qsl_noargs;
   qsl_noargs 
      << "pmcsv"
      ;

   map < QString, bool > noargs;
   for ( int i = 0; i < (int) qsl_noargs.size(); ++i )
   {
      noargs[ qsl_noargs[ i ] ] = true;
   }

   QStringList qsl_req;
   qsl_req 
      << "pmtypes"
      << "pmrayleighdrho"
      << "pmbufferedensity"
      ;

   map < QString, bool > req;
   for ( int i = 0; i < (int) qsl_req.size(); ++i )
   {
      req[ qsl_req[ i ] ] = true;
   }

   for ( map < QString, QString >::iterator it = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].begin();
         it != (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].end();
         it++ )
   {
      if ( req.count( it->first ) )
      {
         req.erase( it->first );
      }
      if ( per_file.count( it->first ) )
      {
         if ( noargs.count( it->first ) )
         {
            out_per_file += QString( "%1\n" ).arg( it->first );
         } else {
            out_per_file += QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );            
         }
      } else {
         if ( !skip.count( it->first ) && !it->second.trimmed().isEmpty() )
         {
            if ( noargs.count( it->first ) )
            {
               base += QString( "%1\n" ).arg( it->first );
            } else {
               base += QString( "%1\t%2\n" ).arg( it->first ).arg( it->second );            
            }
         }
      }
   }
      
   if ( (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmrayleighdrho" ].toDouble() == 0e0 )
   {
      errors += QString( us_tr( "Method %1 pmrayleighdrho must not be zero" ) ).arg( methods[ 0 ] );
   }

   if ( req.size() )
   {
      for ( map < QString, bool >::iterator it = req.begin();
            it != req.end();
            it++ )
      {
         errors += QString( us_tr( "Method %1 requires parameter %2" ) ).arg( methods[ 0 ] ).arg( it->first );
      }
   }

   cout << "------------------------------------------------------------\n";
   cout << base;
   cout << "------------------------------------------------------------\n";

   QString pmapproxmaxdimension = 
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmapproxmaxdimension" ) ?
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmapproxmaxdimension" ] : "";

   if ( !pmapproxmaxdimension.isEmpty() )
   {
      if ( (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmbestcoarseconversion" ) &&
           !(*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmbestcoarseconversion" ].isEmpty() )
      {
         pmapproxmaxdimension = "pmapproxmaxdimension\t" + (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmbestcoarseconversion" ] + "\n";
      } else {
         pmapproxmaxdimension = "pmapproxmaxdimension\t10\n";
      }
   }

   QString pmtypes = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmtypes" ];
   QStringList qsl_pmtypes;
   {
      QRegExp rx = QRegExp( "(\\s+|(\\s*(,|:)\\s*))" );
      qsl_pmtypes = (pmtypes ).split( rx , Qt::SkipEmptyParts );
   }
   for ( int i = 0; i < (int) qsl_pmtypes.size(); ++i )
   {
      if ( qsl_pmtypes[ i ].toInt() < US_PM::OBJECTS_FIRST || 
           qsl_pmtypes[ i ].toInt() > US_PM::OBJECTS_LAST )
      {
         errors += QString( us_tr( "Method %1 pmtype out of range %2" ) ).arg( methods[ 0 ] ).arg( qsl_pmtypes[ i ] );
      }
   }

   if ( !errors.isEmpty() )
   {
      editor_msg( "red", errors );
      return;
   }

   QString pmoutprefix =
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmoutname" ) &&
      !(*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmoutname" ].isEmpty() 
      ?
      (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmoutname" ] : QString( "" ) ;

   pmoutprefix += pmoutprefix.isEmpty() ? "" : "-";

   QString out_per_experimental_dataset;

   unsigned int tot_runs = 0;

   if ( (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmincrementally" ) )
   {
      if ( (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmallcombinations" ) )
      {
         set < int > types;

         for ( int i = 0; i < (int) qsl_pmtypes.size(); ++i )
         {
            types.insert( qsl_pmtypes[ i ].toInt() );
         }

         vector < int > typesv;
         for ( set < int >::iterator it = types.begin();
               it != types.end();
               it++ )
         {
            typesv.push_back( *it );
         }

         for ( int i = 1; i <= (int) qsl_pmtypes.size(); ++i )
         {
            vector < vector < int > > results;
            vector < int >            pos( i );
            combo_with_replacement( typesv,
                                    (int)pos.size(),
                                    pos,
                                    results );
            for ( int j = 0; j < (int) results.size(); ++j )
            {
               out_per_experimental_dataset += QString( "pmoutname %1___EXPERIMENT_NAME___-" ).arg( pmoutprefix );
               for ( int k = 0; k < (int) results[ j ].size(); ++k )
               {
                  out_per_experimental_dataset += QString( "%1" ).arg( results[ j ][ k ] );
               }
               out_per_experimental_dataset += "\n";
               out_per_experimental_dataset += "pmtypes ";
               for ( int k = 0; k < (int) results[ j ].size(); ++k )
               {
                  out_per_experimental_dataset += QString( "%1%2" ).arg( k ? "," : "" ).arg( results[ j ][ k ] );
               }
               out_per_experimental_dataset += "\n";
               out_per_experimental_dataset += "pmbestga\n";
               tot_runs++;
            }
         }
      } else {
         for ( int i = 0; i < (int) qsl_pmtypes.size(); ++i )
         {
            out_per_experimental_dataset += QString( "pmoutname %1___EXPERIMENT_NAME___-" ).arg( pmoutprefix );
            for ( int j = 0; j <= i; ++j )
            {
               out_per_experimental_dataset += QString( "%1" ).arg( qsl_pmtypes[ j ] );
            }
            out_per_experimental_dataset += "\n";
            out_per_experimental_dataset += "pmtypes ";
            for ( int j = 0; j <= i; ++j )
            {
               out_per_experimental_dataset += QString( "%1%2" ).arg( j ? "," : "" ).arg( qsl_pmtypes[ j ] );
            }
            out_per_experimental_dataset += "\n";
            out_per_experimental_dataset += "pmbestga\n";
            tot_runs++;
         }
      }
   } else {
      if ( (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmallcombinations" ) )
      {
         set < int > types;

         for ( int i = 0; i < (int) qsl_pmtypes.size(); ++i )
         {
            types.insert( qsl_pmtypes[ i ].toInt() );
         }
         vector < int > typesv;
         for ( set < int >::iterator it = types.begin();
               it != types.end();
               it++ )
         {
            typesv.push_back( *it );
         }
         vector < vector < int > > results;
         vector < int >            pos( qsl_pmtypes.size() );
         combo_with_replacement( typesv,
                                 (int)pos.size(),
                                 pos,
                                 results );
         for ( int j = 0; j < (int) results.size(); ++j )
         {
            out_per_experimental_dataset += QString( "pmoutname %1___EXPERIMENT_NAME___-" ).arg( pmoutprefix );
            for ( int k = 0; k < (int) results[ j ].size(); ++k )
            {
               out_per_experimental_dataset += QString( "%1" ).arg( results[ j ][ k ] );
            }
            out_per_experimental_dataset += "\n";
            out_per_experimental_dataset += "pmtypes ";
            for ( int k = 0; k < (int) results[ j ].size(); ++k )
            {
               out_per_experimental_dataset += QString( "%1%2" ).arg( k ? "," : "" ).arg( results[ j ][ k ] );
            }
            out_per_experimental_dataset += "\n";
            out_per_experimental_dataset += "pmbestga\n";
            tot_runs++;
         }
      } else {
         out_per_experimental_dataset += QString( "pmoutname %1___EXPERIMENT_NAME___-" ).arg( pmoutprefix );
         for ( int i = 0; i < (int) qsl_pmtypes.size(); ++i )
         {
            out_per_experimental_dataset += QString( "%1" ).arg( qsl_pmtypes[ i ] );
         }
         out_per_experimental_dataset += "\n";
         out_per_experimental_dataset += QString( "pmtypes\t%1\n" ).arg( pmtypes );
         out_per_experimental_dataset += "pmbestga\n";
         tot_runs++;
      }
   }

   cout << out_per_experimental_dataset;
   cout << "------------------------------------------------------------\n";

   QString out = base;

   for ( int i = 0; i < lb_target_files->count(); i++ )
   {
      QString target_file      = lb_target_files->item( i )->text();
      QString target_file_name = QFileInfo( target_file ).completeBaseName().replace( ".", "_" );

      // read file and extract q,I,e... add  pmq, pmi, and possibly pme with 0, 'g', 8
      // later add pmf via "pmusedummyatomff" option
      // and possible q editing (log binning), qmax, max pts ?
      vector < double > q;
      vector < double > I;
      vector < double > e;
      QString error_msg;

      if ( !US_Saxs_Util::read_sas_data( target_file, q, I, e, error_msg ) )
      {
         errors += error_msg + "\n";
      } else {
         if ( q.size() < 3 )
         {
            errors +=  QString( "Error: Too few q points read from data (%1)" ).arg( q.size() );
            continue;
         }            

         double pmminq = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmminq" ) ?
            (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmminq" ].toDouble() : 0e0;
         double pmmaxq = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmmaxq" ) ?
            (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmmaxq" ].toDouble() : 7e0;
         int pmqpoints = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmqpoints" ) ?
            (*cluster_additional_methods_options_selected)[ methods[ 0 ] ][ "pmqpoints" ].toInt() : 10000;
         bool pmlogbin = (*cluster_additional_methods_options_selected)[ methods[ 0 ] ].count( "pmlogbin" );
         US_Saxs_Util::clip_data( pmminq, pmmaxq, q, I, e );

         if ( pmlogbin && !pmqpoints )
         {
            errors += "Error: log binning was selected with zero q points";
         }
         QString error_msg;
         QString notice_msg;
         
         US_Saxs_Util::bin_data( pmqpoints, pmlogbin, q, I, e, error_msg, notice_msg );
         if ( !notice_msg.isEmpty() )
         {
            editor_msg( "dark red", us_tr( notice_msg ) );
         }
         if ( !error_msg.isEmpty() )
         {
            errors += us_tr( error_msg ) + "\n";
            continue;
         }

         if ( q.size() < 3 )
         {
            errors +=  QString( "Error: After cropping and binning, there are too few q points left (%1)" ).arg( q.size() );
         }            

         out += "pmq ";
         for ( int j = 0; j < (int) q.size(); ++j )
         {
            out += QString( "%1%2" ).arg( j ? "," : "" ).arg( q[ j ], 0, 'g', 8 );
         }
         out += "\n";
         out += "pmi ";
         for ( int j = 0; j < (int) I.size(); ++j )
         {
            out += QString( "%1%2" ).arg( j ? "," : "" ).arg( I[ j ], 0, 'g', 8 );
         }
         out += "\n";
         if ( e.size() )
         {
            out += "pme ";
            for ( int j = 0; j < (int) e.size(); ++j )
            {
               out += QString( "%1%2" ).arg( j ? "," : "" ).arg( e[ j ], 0, 'g', 8 );
            }
            out += "\n";
         }
         out += out_per_file;
         out += pmapproxmaxdimension;
      }

      QString out_this_experimental_dataset = out_per_experimental_dataset;
      out_this_experimental_dataset.replace( "___EXPERIMENT_NAME___", target_file_name );

      out += out_this_experimental_dataset;
   }

   cout << out;
   cout << "------------------------------------------------------------\n";

   if ( !errors.isEmpty() )
   {
      editor_msg( "red", errors );
      return;
   }

   editor_msg( "dark blue", QString( "Notice: the total number of models to be computed is %1\n" ).arg( tot_runs ) );

   QString use_file = QString( "%1" ).arg( filename );

   cout << use_file << endl;

   {
      QFile f( use_file );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: could not create file %1" ) ).arg( use_file ) );
         return;
      }

      {
         QTextStream ts( &f );
         ts << out;
         f.close();
         editor_msg( "dark gray", QString( us_tr( "Created: %1" ) ).arg( use_file ) );
      }
   }

   // write it, tar it, package it
   QStringList to_tar_list;

   to_tar_list << QFileInfo( use_file ).fileName();

   QStringList  dest_files;

   {
      US_Tar       ust;
      US_Gzip      usg;
      QDir qd;

      QStringList list;
      QString tar_name = use_file + ".tar";
      int result = ust.create( QFileInfo( tar_name ).filePath(), to_tar_list, &list );
      cout << "tar_name:" << tar_name << endl;
      cout << "to tar:\n" << to_tar_list.join("\n") << endl;

      qd.remove( use_file );

      if ( result != TAR_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem creating tar archive %1: %2") ).arg( filename ).arg( ust.explain( result ) ) );
         return;
      }
      editor_msg( "dark gray", QString("Created: %1").arg( tar_name ) );

      result = usg.gzip( tar_name );
      if ( result != GZIP_OK )
      {
         editor_msg( "red" , QString( us_tr( "Error: Problem gzipping tar archive %1: %2") ).arg( tar_name ).arg( usg.explain( result ) ) );
         return;
      }

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
   }

   editor_msg( "blue", 
               dest_files
               .replaceInStrings( QRegExp( "^" ), us_tr( "Package: " ) )
               .replaceInStrings( QRegExp( "$" ), us_tr( " created" ) )
               .join( "\n" ) );

   editor_msg( "black", us_tr( "Package complete" ) );

}

bool US_Hydrodyn_Cluster::additional_processing(
                                                QString       & out,
                                                QStringList   & source_files,
                                                const QString & type, 
                                                const QString & method, 
                                                const QString & file
                                                )
{
   if ( method == "best" )
   {
      if ( type == "additional_processing_per_file" )
      {
         double mw = 0e0;
         if (  (*cluster_additional_methods_options_selected).count( method ) &&
               (*cluster_additional_methods_options_selected)[ method ].count( "bestbestmw" ) )
         {
            mw = (*cluster_additional_methods_options_selected)[ method ][ "bestbestmw" ].toDouble();
         }
         if (  (*cluster_additional_methods_options_selected).count( method ) &&
               !(*cluster_additional_methods_options_selected)[ method ].count( "bestbestmw" ) )
         {
            // compute mw for file
            if ( file.contains(QRegExp(".(pdb|PDB)$")) &&
                 !((US_Hydrodyn *)us_hydrodyn)->is_dammin_dammif(file) )
            {
               bool ok = true;
               if ( batch_window->screen_pdb( file, false ) )
               {
                  mw = ((US_Hydrodyn *)us_hydrodyn)->model_vector[ 0 ].mw;
               } else {
                  // try alternale
                  editor_msg( "dark red", QString( us_tr( "Warning: error loading %1 for computation of molecular weight" ) ).arg( file ) );
                  if ( ((US_Hydrodyn *)us_hydrodyn)->saxs_util->pdb_mw( file, mw ) )
                  {
                     editor_msg( "dark red", QString( us_tr( "Notice: direct atom method for computation of molecular weight of %1 used" ) ).arg( file ) );
                  } else {
                     editor_msg( "red", QString( us_tr( "Error: direct atom method for computation of molecular weight of %1 failed" ) ).arg( file ) );
                     ok = false;
                  }
               }

               if ( ok )
               {
                  editor_msg( "blue", QString( us_tr( "Molecular weight of %1: %2 Daltons\n" ) ).arg( QFileInfo( file ).fileName() ).arg( mw ) );
                  out += QString( "bestbestmw      %1\n" ).arg( mw );
                  // PAT
                  if ( !US_Saxs_Util::pat_model( ((US_Hydrodyn *)us_hydrodyn)->model_vector[ 0 ] ) )
                  {
                     editor_msg( "red", QString( us_tr( "Error: perform PAT on  %1" ) ).arg( file ) );
                  } else {
                     editor_msg( "blue", QString( us_tr( "PAT on %1 ok" ) ).arg( file ) );
                     QString dir = ( ( US_Hydrodyn * ) us_hydrodyn)->somo_dir + QDir::separator() + "tmp" + QDir::separator();
                     QString patfile =  dir + QFileInfo( file ).baseName() + "_pat" + "." + QFileInfo( file ).completeSuffix();
                     if ( !US_Saxs_Util::write_model( ((US_Hydrodyn *)us_hydrodyn)->model_vector[ 0 ] , patfile ) )
                     {
                        editor_msg( "red", QString( us_tr( "Error: writing PAT'd pdb %1" ) ).arg( patfile ) );
                     } else {
                        source_files << patfile;
                     }
                  }
               }
            } else {
               editor_msg( "red", "Error: bead models are not supported for msroll" );
               // result = batch_window->screen_bead_model( file );
            }
            if (  (*cluster_additional_methods_options_selected).count( method ) &&
                  (*cluster_additional_methods_options_selected)[ method ].count( "bestrcoalautominmax" ) &&
                  (*cluster_additional_methods_options_selected)[ method ][ "bestrcoalautominmax" ] == "true" 
                  )
            {
               int maxtriangles = (int) ( 30 * sqrt( mw ) );
               int mintriangles = (int) ( 18 * sqrt( mw ) );
               out += QString( "bestrcoalnmin   %1\n" ).arg( mintriangles );
               out += QString( "bestrcoalnmax   %1\n" ).arg( maxtriangles );
               editor_msg( "blue", QString( "COALESCE: triangles range %1 to %2 for file %3" )
                           .arg( mintriangles )
                           .arg( maxtriangles )
                           .arg( file ) );
            }               
         }
         return true;
      }
      if ( type == "additional_processing_global" )
      {
         QString common_prefix = cb_for_mpi->isChecked() ? "../common/" : "";
         QString my_msroll_radii = ( ( US_Hydrodyn * ) us_hydrodyn)->msroll_radii.join( "" );
         if (  (*cluster_additional_methods_options_selected).count( method ) &&
               (*cluster_additional_methods_options_selected)[ method ].count( "bestexpand" ) &&
               (*cluster_additional_methods_options_selected)[ method ][ "bestexpand" ].toDouble() > 0e0
               )
         {
            double mult = (*cluster_additional_methods_options_selected)[ method ][ "bestexpand" ].toDouble();
            editor_msg( "dark blue", QString( us_tr( "Radii will be multiplied by %1" ) ).arg( mult ) );
            QStringList my_qsl_radii = ( ( US_Hydrodyn * ) us_hydrodyn)->msroll_radii;
            QRegExp rx( "(^\\d+) (\\S+) (\\S+) (\\S+)" );
            for ( int i = 0; i < (int) my_qsl_radii.size(); ++i )
            {
               if ( rx.indexIn( my_qsl_radii[ i ] ) == -1 )
               {
                  editor_msg( "red", us_tr( "radii multiplication failed" ) );
                  return false;
               } else {
                  my_qsl_radii[ i ] = QString( "%1 %2 %3 %4\n" )
                     .arg( rx.cap( 1 ) )
                     .arg( rx.cap( 2 ).toDouble() * mult )
                     .arg( rx.cap( 3 ) )
                     .arg( rx.cap( 4 ) );
               }
            }
            my_msroll_radii = my_qsl_radii.join( "" );
         }

         if (  (*cluster_additional_methods_options_selected).count( method ) &&
               (*cluster_additional_methods_options_selected)[ method ].count( "bestbestwatr" ) )
         {
            {
               QRegExp rx( " (\\S+) (\\S)+ WATOW" );
               if ( rx.indexIn( my_msroll_radii ) != -1 )
               {
                  my_msroll_radii.replace( rx, QString( " %1 %2 WATOW" )
                                           .arg( (*cluster_additional_methods_options_selected)[ method ][ "bestbestwatr" ] )
                                           .arg( (*cluster_additional_methods_options_selected)[ method ][ "bestbestwatr" ].toDouble() / 2.68 )
                                           );
               }
            }
            {
               QRegExp rx( " (\\S+) (\\S)+ SWHOW" );
               if ( rx.indexIn( my_msroll_radii ) != -1 )
               {
                  my_msroll_radii.replace( rx, QString( " %1 %2 SWHOW" )
                                           .arg( (*cluster_additional_methods_options_selected)[ method ][ "bestbestwatr" ] )
                                           .arg( (*cluster_additional_methods_options_selected)[ method ][ "bestbestwatr" ].toDouble() / 2.68 )
                                           );
               }
            }
         }
         QString dir = ( ( US_Hydrodyn * ) us_hydrodyn)->somo_dir + QDir::separator() + "tmp" + QDir::separator();
         
         if ( !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrusesomoradii" ) &&
              !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) )
         {
            (*cluster_additional_methods_options_selected)[ method ][ "bestmsrradiifile" ] =
               QString( USglobal->config_list.system_dir + QDir::separator() + "etc" + QDir::separator() + "best.radii" );
            editor_msg( "blue", QString( us_tr( "Notice: using default BEST radii  file: %1" ) )
                        .arg( (*cluster_additional_methods_options_selected)[ method ][ "bestmsrradiifile" ] ) );
         }

         if ( (*cluster_additional_methods_options_selected)[ method ].count( "bestmsrusesomoradii" ) )
         {
            if ( (*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) )
            {
               (*cluster_additional_methods_options_selected)[ method ].erase( "bestmsrradiifile" );
            }
            if ( (*cluster_additional_methods_options_selected)[ method ].count( "bestmsrpatternfile" ) )
            {
               (*cluster_additional_methods_options_selected)[ method ].erase( "bestmsrpatternfile" );
            }
         }

         if ( !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) )
         {
            QFile f_radii( dir + "msroll_radii.txt" );
            if ( !f_radii.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( us_tr( "Error: can not create MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
            } else {
               QTextStream ts( &f_radii );
               ts << my_msroll_radii;
               f_radii.close();
               source_files << f_radii.fileName();
               out += QString( "bestmsrradiifile %1%2\n" ).arg( common_prefix ).arg( QFileInfo( f_radii ).fileName() );
               editor_msg( "blue", QString( us_tr( "Notice: created MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
            }
         }

         if ( !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrpatternfile" ) &&
              !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) )
         {
            QFile f_names( dir + "msroll_names.txt" );
            if ( !f_names.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( us_tr( "Error: can not create MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
            } else {
               QTextStream ts( &f_names );
               for ( unsigned int i = 0; i < (unsigned int) ( ( US_Hydrodyn * ) us_hydrodyn)->msroll_names.size(); i++ )
               {
                  ts << ( ( US_Hydrodyn * ) us_hydrodyn)->msroll_names[ i ];
               }
               f_names.close();
               source_files << f_names.fileName();
               out += QString( "bestmsrpatternfile %1%2\n" ).arg( common_prefix ).arg( QFileInfo( f_names ).fileName() );
               editor_msg( "blue", QString( us_tr( "Notice: created MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
            }
         }

         if ( (*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) )
         {
            if ( (*cluster_additional_methods_options_selected).count( method ) )
            {
               if ( (*cluster_additional_methods_options_selected)[ method ].count( "bestbestwatr" ) ||
                    (*cluster_additional_methods_options_selected)[ method ].count( "bestexpand" ) )
               {
                  editor_msg( "red", us_tr( "Manual radii file does not support water radius or expansion" ) );
                  return false;
               }
            }
            editor_msg( "blue", us_tr( "Manual radii file specified" ) );
         }

         if ( 
             ( !(*cluster_additional_methods_options_selected)[ method ].count( "bestmsrradiifile" ) &&
               (*cluster_additional_methods_options_selected)[ method ].count( "bestmsrpatternfile" ) ) 
              )
         {
            editor_msg( "red", us_tr( "Manual pattern file requires radii file" ) );
            return false;
         }

         QFile f_directives( dir + "__directives" );
         if ( !f_directives.open( QIODevice::WriteOnly ) )
         {
            editor_msg( "red", QString( us_tr( "Error: can not create directives file: %1" ) ).arg( f_directives.fileName() ) );
         } else {
            QTextStream ts( &f_directives );
            ts << QString( "%1" ).arg( cluster_additional_methods_job_multiplier[ "best" ] - 1 ) << Qt::endl;
            f_directives.close();
            source_files << f_directives.fileName();
         }

         return true;
      }
   }
   return false;
}
