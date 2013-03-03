#include "../include/us_hydrodyn_saxs_hplc_p3d.h"

US_Hydrodyn_Saxs_Hplc_P3d::US_Hydrodyn_Saxs_Hplc_P3d(
                                                     void                     *              us_hydrodyn_saxs_hplc,
                                                     map < QString, QString > *              parameters,
                                                     QWidget *                               p,
                                                     const char *                            name
                                                     ) : QDialog( p, name )
{
   this->us_hydrodyn_saxs_hplc                = us_hydrodyn_saxs_hplc;
   this->parameters                           = parameters;

   USglobal = new US_Config();
   setPalette( QPalette( USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame ) );
   setCaption( tr( "US-SOMO: SAXS HPLC : Plot 3D" ) );

   setupGUI();
   update_enables();

   global_Xpos += 30;
   global_Ypos += 30;

   setGeometry( global_Xpos, global_Ypos, 0, 0 );
}

US_Hydrodyn_Saxs_Hplc_P3d::~US_Hydrodyn_Saxs_Hplc_P3d()
{
}

void US_Hydrodyn_Saxs_Hplc_P3d::setupGUI()
{
   int minHeight1  = 30;

   lbl_title =  new QLabel      ( tr( "US-SOMO: SAXS HPLC : Plot 3D" ), this );
   lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
   lbl_title -> setMinimumHeight( minHeight1 );
   lbl_title -> setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold ) );

   lbl_plot_curves = new QLabel(tr(" Include Gaussians: "), this);
   lbl_plot_curves->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_plot_curves->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_plot_curves->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   for ( unsigned int i = 0; i < (* parameters)[ "gaussians" ].toUInt(); i++ )
   {
      QCheckBox *cb_tmp;
      cb_tmp = new QCheckBox(this);
      cb_tmp->setText( QString( " %1 " ).arg( i + 1 ) );
      cb_tmp->setEnabled( true );
      cb_tmp->setChecked( false );
      cb_tmp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_tmp->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_tmp, SIGNAL( clicked() ), SLOT( update_enables() ) );
      cb_plot_curves.push_back( cb_tmp );
   }

   pb_plot_all =  new QPushButton ( tr( "Plot all" ), this );
   pb_plot_all -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_plot_all -> setMinimumHeight( minHeight1 );
   pb_plot_all -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_plot_all, SIGNAL( clicked() ), SLOT( plot_all() ) );

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

   pb_plot =  new QPushButton ( tr( "Plot selected" ), this );
   pb_plot -> setFont         ( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1) );
   pb_plot -> setMinimumHeight( minHeight1 );
   pb_plot -> setPalette      ( QPalette( USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active ) );
   connect( pb_plot, SIGNAL( clicked() ), SLOT( plot() ) );

   QVBoxLayout *background = new QVBoxLayout( this );
   background->addWidget( lbl_title );

   QHBoxLayout *hbl_plot_curves = new QHBoxLayout( 0 );
   hbl_plot_curves->addWidget( lbl_plot_curves );
   for ( unsigned int i = 0; i < ( unsigned int ) cb_plot_curves.size(); i++ )
   {
      hbl_plot_curves->addWidget( cb_plot_curves[ i ] );
   }

   background->addLayout( hbl_plot_curves );

   QHBoxLayout *hbl_bottom = new QHBoxLayout( 0 );
   hbl_bottom->addWidget ( pb_help );
   hbl_bottom->addWidget ( pb_quit );
   hbl_bottom->addWidget ( pb_plot_all );
   hbl_bottom->addWidget ( pb_plot );

   background->addLayout ( hbl_bottom );

}

void US_Hydrodyn_Saxs_Hplc_P3d::plot_all()
{
   for ( unsigned int i = 0; i < ( unsigned int ) cb_plot_curves.size(); i++ )
   {
      if ( !cb_plot_curves[ i ]->isChecked() )
      {
         cb_plot_curves[ i ]->setChecked( true );
      }
   }
   update_enables();
   plot();
}

void US_Hydrodyn_Saxs_Hplc_P3d::quit()
{
   close();
}

void US_Hydrodyn_Saxs_Hplc_P3d::plot()
{
   (*parameters)[ "plot" ] = "true";
   for ( unsigned int i = 0; i < ( unsigned int ) cb_plot_curves.size(); i++ )
   {
      if ( cb_plot_curves[ i ]->isChecked() )
      {
         (*parameters)[ QString( "%1" ).arg( i ) ] = "true";
      }
   }
   close();
}

void US_Hydrodyn_Saxs_Hplc_P3d::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help("manual/saxs_hplc_p3d.html");
}

void US_Hydrodyn_Saxs_Hplc_P3d::closeEvent( QCloseEvent *e )
{

   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Saxs_Hplc_P3d::update_enables()
{
   bool any_enabled = false;
   for ( unsigned int i = 0; i < ( unsigned int ) cb_plot_curves.size(); i++ )
   {
      if ( cb_plot_curves[ i ]->isChecked() )
      {
         any_enabled = true;
      }
   }
   pb_plot->setEnabled( any_enabled );
}
