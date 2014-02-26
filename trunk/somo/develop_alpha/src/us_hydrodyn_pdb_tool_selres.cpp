#include "../include/us_hydrodyn_pdb_tool_selres.h"
//Added by qt3to4:
#include <Q3GridLayout>
#include <Q3HBoxLayout>
#include <Q3VBoxLayout>
#include <QLabel>
#include <QCloseEvent>

US_Hydrodyn_Pdb_Tool_Selres::US_Hydrodyn_Pdb_Tool_Selres(
                                                         void                     *              us_hydrodyn_pdb_tool,
                                                         map < QString, QString > *              parameters,
                                                         QWidget *                               p,
                                                         const char *                            name
                                                         ) : QDialog( p, name )
{
   this->us_hydrodyn_pdb_tool                 = us_hydrodyn_pdb_tool;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( ((US_Hydrodyn_Pdb_Tool *)us_hydrodyn_pdb_tool)->caption() + tr( " : Select residues" ) );

   setupGUI();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
   update_enables();
}

US_Hydrodyn_Pdb_Tool_Selres::~US_Hydrodyn_Pdb_Tool_Selres()
{
}

void US_Hydrodyn_Pdb_Tool_Selres::setupGUI()
{
   int minHeight1  = 26;

   lbl_title =  new QLabel      ( caption(), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_max_dist =  new QLabel      ( tr( "Maximum distance in Angstroms:" ), this );
   lbl_max_dist -> setAlignment    ( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_max_dist -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_max_dist -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold ) );

   le_max_dist = new QLineEdit(this, "le_max_dist Line Edit");
   le_max_dist->setText( "5" );
   le_max_dist->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_dist->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_dist->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1e0, 100e0, 2, le_max_dist );
      le_max_dist->setValidator( qdv );
   }
   le_max_dist->setMinimumWidth( 60 );

   cb_sel_only_new = new QCheckBox(this);
   cb_sel_only_new->setText( tr( "Unselect previous selection" ) );
   cb_sel_only_new->setEnabled( true );
   cb_sel_only_new->setChecked( false );
   cb_sel_only_new->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_sel_only_new->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_save_sel = new QCheckBox(this);
   cb_save_sel->setText( tr( "Save resulting residue list to file:" ) );
   cb_save_sel->setEnabled( true );
   cb_save_sel->setChecked( false );
   cb_save_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_save_sel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_save_sel, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_save_sel = new QLineEdit(this, "le_save_sel Line Edit");
   le_save_sel->setText( "selected_residues.pdb" );
   le_save_sel->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_save_sel->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_save_sel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   le_save_sel->setMinimumWidth( 200 );

   cb_asa = new QCheckBox(this);
   cb_asa->setText( tr( "Select only solvent exposed residues with an ASA of cutoff of %:" ) );
   cb_asa->setEnabled( true );
   cb_asa->setChecked( false );
   cb_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_asa->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect( cb_asa, SIGNAL( clicked() ), SLOT( update_enables() ) );

   le_asa = new QLineEdit(this, "le_asa Line Edit");
   le_asa->setText( "50" );
   le_asa->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_asa->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ));
   {
      QDoubleValidator *qdv = new QDoubleValidator( 1e0, 100e0, 2, le_asa );
      le_asa->setValidator( qdv );
   }
   le_asa->setMinimumWidth( 60 );

   cb_naccess = new QCheckBox(this);
   cb_naccess->setText( tr( "Use NACCESS for computation of solvent exposed residues" ) );
   cb_naccess->setEnabled( true );
   cb_naccess->setChecked( false );
   cb_naccess->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_naccess->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_naccess_sc_or_mc = new QCheckBox(this);
   cb_naccess_sc_or_mc->setText( tr( "Threshold for MC or SC" ) );
   cb_naccess_sc_or_mc->setEnabled( true );
   cb_naccess_sc_or_mc->setChecked( false );
   cb_naccess_sc_or_mc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   cb_naccess_sc_or_mc->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_help =  new QPushButton ( tr( "Help" ), this );
   pb_help -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_help -> setMinimumHeight( minHeight1 );
   pb_help -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   pb_quit =  new QPushButton ( tr( "Quit" ), this );
   pb_quit -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_quit -> setMinimumHeight( minHeight1 );
   pb_quit -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_quit, SIGNAL( clicked() ), SLOT( quit() ) );

   pb_go =  new QPushButton ( tr( "Select residues" ), this );
   pb_go -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_go -> setMinimumHeight( minHeight1 );
   pb_go -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_go, SIGNAL( clicked() ), SLOT( go() ) );

   Q3VBoxLayout *background = new Q3VBoxLayout( this );
   background->addWidget( lbl_title );

   Q3GridLayout *gl = new Q3GridLayout( 0 );

   gl->addWidget         ( lbl_max_dist , 0, 0 );
   gl->addWidget         ( le_max_dist  , 0, 1 );
   gl->addWidget         ( cb_save_sel  , 1, 0 );
   gl->addWidget         ( le_save_sel  , 1, 1 );
   gl->addWidget         ( cb_asa       , 2, 0 );
   gl->addWidget         ( le_asa       , 2, 1 );

   background->addLayout( gl );

   background->addWidget( cb_naccess );
   background->addWidget( cb_naccess_sc_or_mc );
   background->addWidget( cb_sel_only_new );

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_go );

   background->addLayout ( hbl_bottom );

   if ( 
       !QFile::exists( USglobal->config_list.system_dir + "/bin" +
#if defined(BIN64)
                        "64"
#endif
                        + "/naccess" ) 
       ||
       !QFile::exists( USglobal->config_list.system_dir + "/bin" +
#if defined(BIN64)
                        "64"
#endif
                       + "/vdw.radii" ) 
       )
   {
      cb_naccess->hide();
      cb_naccess_sc_or_mc->hide();
   }
}

void US_Hydrodyn_Pdb_Tool_Selres::quit()
{
   close();
}

void US_Hydrodyn_Pdb_Tool_Selres::go()
{
   (*parameters)[ "go" ] = "true";
   if ( cb_sel_only_new->isChecked() )
   {
      (*parameters)[ "only_new" ] = "true";
   }
   if ( cb_save_sel->isChecked() )
   {
      (*parameters)[ "save_sel" ] = le_save_sel->text();
   }
   if ( cb_asa->isChecked() )
   {
      (*parameters)[ "asa" ] = le_asa->text();
      if ( cb_naccess->isChecked() )
      {
         (*parameters)[ "naccess" ] = "true";
         if ( cb_naccess_sc_or_mc->isChecked() )
         {
            (*parameters)[ "naccess_sc_or_mc" ] = "true";
         }
      }
   }
   (*parameters)[ "max_dist" ] = le_max_dist->text();
   close();
}

void US_Hydrodyn_Pdb_Tool_Selres::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/pdb_tool_selres.html");
}

void US_Hydrodyn_Pdb_Tool_Selres::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Pdb_Tool_Selres::update_enables()
{
   le_save_sel ->setEnabled( cb_save_sel->isChecked() );
   le_asa      ->setEnabled( cb_asa     ->isChecked() );
   if ( !cb_asa->isChecked() &&
        ( cb_naccess         ->isChecked() ||
          cb_naccess_sc_or_mc->isChecked() ) )
   {
      cb_naccess         ->setChecked( false );
      cb_naccess_sc_or_mc->setChecked( false );
   }
   cb_naccess         ->setEnabled( cb_asa     ->isChecked() );
   cb_naccess_sc_or_mc->setEnabled( cb_asa     ->isChecked() );
}
