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

   setupGUI();

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
   le_target_file->setText("");
   le_target_file->setReadOnly( true );
   le_target_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_file->setMinimumWidth(150);
   le_target_file->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   lbl_jobs_per = new QLabel( QString("Jobs per job file maximum %1:").arg( selected_files.size() ), this);
   lbl_jobs_per->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_jobs_per->setMinimumHeight(minHeight1);
   lbl_jobs_per->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_jobs_per->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   QValidator *qv = new QIntValidator( 1, selected_files.size(), this );
   le_jobs_per = new QLineEdit(this, "csv_filename Line Edit");
   le_jobs_per->setText("1");
   le_jobs_per->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_jobs_per->setMinimumWidth(150);
   le_jobs_per->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_jobs_per->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_jobs_per->setValidator( qv );

   lbl_output_name = new QLabel( QString("Jobs per job file maximum %1:").arg( selected_files.size() ), this);
   lbl_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_output_name->setMinimumHeight(minHeight1);
   lbl_output_name->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   le_output_name = new QLineEdit(this, "csv_filename Line Edit");
   le_output_name->setText("cluster_output");
   le_output_name->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_output_name->setMinimumWidth(150);
   le_output_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_output_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

   pb_create = new QPushButton(tr("Create cluster job files"), this);
   pb_create->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create->setMinimumHeight(minHeight1);
   pb_create->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create, SIGNAL(clicked()), SLOT(create()));

   pb_create_pkg = new QPushButton(tr("Create cluster job package"), this);
   pb_create_pkg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_create_pkg->setMinimumHeight(minHeight1);
   pb_create_pkg->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_create_pkg, SIGNAL(clicked()), SLOT(create_pkg()));
   
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
   QHBoxLayout *hbl_target = new QHBoxLayout( 0 );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( lbl_target );
   hbl_target->addSpacing( 4);
   hbl_target->addWidget ( pb_set_target );
   hbl_target->addSpacing( 4 );
   hbl_target->addWidget ( le_target_file );
   hbl_target->addSpacing( 4 );

   QHBoxLayout *hbl_jobs_per = new QHBoxLayout( 0 );
   hbl_jobs_per->addSpacing( 4 );
   hbl_jobs_per->addWidget ( lbl_jobs_per );
   hbl_jobs_per->addSpacing( 4 );
   hbl_jobs_per->addWidget ( le_jobs_per );
   hbl_jobs_per->addSpacing( 4 );

   QHBoxLayout *hbl_output_name = new QHBoxLayout( 0 );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( lbl_output_name );
   hbl_output_name->addSpacing( 4 );
   hbl_output_name->addWidget ( le_output_name );
   hbl_output_name->addSpacing( 4 );

   QHBoxLayout *hbl_create = new QHBoxLayout( 0 );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_create );
   hbl_create->addSpacing( 4 );
   hbl_create->addWidget ( pb_create_pkg );
   hbl_create->addSpacing( 4 );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addSpacing( 4 );
   hbl_bottom->addWidget ( pb_cancel );
   hbl_bottom->addSpacing( 4 );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addSpacing( 4);
   background->addWidget ( lbl_title );
   background->addSpacing( 4 );
   background->addLayout ( hbl_target );
   background->addSpacing( 4 );
   background->addLayout ( hbl_jobs_per );
   background->addSpacing( 4 );
   background->addLayout ( hbl_output_name );
   background->addSpacing( 4 );
   background->addLayout ( hbl_create );
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

void US_Hydrodyn_Cluster::create()
{
   QString unimplemented;

   // create the output file
   QString filename = QFileDialog::getSaveFileName(QString::null, QString::null,this );

   QString base = 
      "# us_saxs_cmds_t iq controlfile\n"
      "# blank lines ok, format token <params>\n"
      "\n";

   base += 
      QString( "ResidueFile     %1\n" ).arg( QFileInfo( ((US_Hydrodyn *)us_hydrodyn)->lbl_table->text() ).fileName() );
   base += 
      QString( "AtomFile        %1\n" ).arg( QFileInfo( our_saxs_options->default_atom_filename ).fileName() );
   base += 
      QString( "HybridFile      %1\n" ).arg( QFileInfo( our_saxs_options->default_hybrid_filename ).fileName() );
   base += 
      QString( "SaxsFile        %1\n" ).arg( QFileInfo( our_saxs_options->default_saxs_filename ).fileName() );

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
         unimplemented += "foxs method currently unimplemented\n";
         iqmethod = "foxs";
      }
      if ( our_saxs_options->saxs_iq_crysol )
      {
         unimplemented += "crysol method currently unimplemented\n";
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
   } else {
      base += 
         QString( "StartQ          %1\n" ).arg( our_saxs_options->start_q );
      base += 
         QString( "EndQ            %1\n" ).arg( our_saxs_options->end_q );
      base += 
         QString( "DeltaQ          %1\n" ).arg( our_saxs_options->delta_q );
   }

   if ( batch_window->cb_mm_all->isChecked() )
   {
      base += "PDBAllModels\n";
   }
      
   base += QString( "Output          %1\n" )
      .arg( batch_window->cb_csv_saxs->isChecked() ? "csv" : "ssaxs" );

   base += QString( "OutputFile      %1\n" ).arg( le_output_name->text() );

   cout << base;
   

   // now loop through selected, jumping every le_jobs_per->text()->toUInt()
   // for a new extension
   if ( !le_jobs_per->text().toUInt() )
   {
      le_jobs_per->setText( "1" );
   }
   
   QString      out = base;
   unsigned int write_count = 0;
   bool         use_extension =  !( le_jobs_per->text().toUInt() == selected_files.size() );
   unsigned int extension_count = selected_files.size() / le_jobs_per->text().toUInt();
   unsigned int extension_count_length = QString("%1").arg( extension_count ).length();

   for ( unsigned int i = 0; i < selected_files.size(); i++ )
   {
      out += QString( "InputFile       %1\n" ).arg( QFileInfo( selected_files[ i ] ).fileName() );
      out += "Process\n";
      if ( !( ( i + 1 ) % le_jobs_per->text().toUInt() ) )
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
         QString use_file = QString( "%1%2" ).arg( filename ).arg( use_extension ? QString("-%1").arg( ext ) : "" );
         if ( !((US_Hydrodyn *)us_hydrodyn)->overwrite && QFile::exists( use_file ) )
         {
            use_file = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( use_file );
         }
         cout << use_file << endl;
         QFile f( use_file );
         if ( f.open( IO_WriteOnly ) )
         {
            QTextStream ts( &f );
            ts << out;
            ts << QString( "TarOutput       %1.tar\n" ).arg( le_output_name->text() + 
                                                             ( use_extension ? QString("-%1").arg( ext ) : "" ) );
            f.close();
         }
         out = base;
      }
   }
}

void US_Hydrodyn_Cluster::create_pkg()
{
   // create the output file
   QString filename = QFileDialog::getSaveFileName(QString::null, QString::null,this );
}
