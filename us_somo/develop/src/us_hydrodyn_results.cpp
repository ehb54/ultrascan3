#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_results.h"
#include "../include/us_hydrodyn.h"

//Added by qt3to4:
#include <QTextStream>
#include <QCloseEvent>
#include <QGridLayout>
#include <QFrame>
#include <QLabel>
#define DOTSOMO      ""
#define DOTSOMOCAP   ""

US_Hydrodyn_Results::US_Hydrodyn_Results(struct hydro_results *results,
                                         bool *result_widget,
                                         void *us_hydrodyn,
                                         QWidget *p, const char *) : QFrame( p )
{
   this->results = results;
   this->result_widget = result_widget;
   this->us_hydrodyn = us_hydrodyn;
   *result_widget = true;
   USglobal=new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SOMO Hydrodynamic Results"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   somo_dir = USglobal->config_list.root_dir + "/somo";
}

US_Hydrodyn_Results::~US_Hydrodyn_Results()
{
   *result_widget = false;
}

void US_Hydrodyn_Results::setupGUI()
{
   int minHeight1 = 70;
   int minHeight2 = 30;
   QString str;

   lbl_info = new QLabel(us_tr( QString("SOMO Hydrodynamic Results\n(%1 at %2%3C, pH %4)\n(Density %5 g/ml, Viscosity %6 cP)" )
                                .arg(results->solvent_name)
                                .arg(results->temperature)
                                .arg( QChar(0260) )
                                .arg(results->pH)
                                .arg(results->solvent_density)
                                .arg(results->solvent_viscosity)
                                ), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_name = new QLabel(us_tr(" Model: "), this);
   Q_CHECK_PTR(lbl_name);
   lbl_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_name->setMinimumWidth(200);
   lbl_name->setPalette( PALET_LABEL );
   AUTFBACK( lbl_name );
   lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_name = new QLineEdit( this );    le_name->setObjectName( "name Line Edit" );
   le_name->setText(results->name);
   le_name->setReadOnly(true);
   le_name->setMinimumWidth(300);
   //   le_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_name->setPalette( PALET_NORMAL );
   AUTFBACK( le_name );
   le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_method = new QLabel(us_tr(" Method: "), this);
   Q_CHECK_PTR(lbl_method);
   lbl_method->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_method->setMinimumWidth(200);
   lbl_method->setPalette( PALET_LABEL );
   AUTFBACK( lbl_method );
   lbl_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_method = new QLineEdit( this );    le_method->setObjectName( "method Line Edit" );
   le_method->setText(results->method);
   le_method->setReadOnly(true);
   le_method->setMinimumWidth(200);
   //   le_method->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_method->setPalette( PALET_NORMAL );
   AUTFBACK( le_method );
   le_method->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_total_beads = new QLabel(us_tr(" Total Beads in Model: "), this);
   Q_CHECK_PTR(lbl_total_beads);
   lbl_total_beads->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_total_beads->setMinimumWidth(200);
   lbl_total_beads->setPalette( PALET_LABEL );
   AUTFBACK( lbl_total_beads );
   lbl_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_total_beads = new QLineEdit( this );    le_total_beads->setObjectName( "total_beads Line Edit" );
   if ( fabs(results->total_beads_sd) <= 1e-100 )
   {
      le_total_beads->setText(QString("%1").arg(results->total_beads));
   }
   else
   {
      le_total_beads->setText(QString("").sprintf("%u (%4.2e)",
                                                  (int)(results->total_beads + .5),
                                                  results->total_beads_sd));
   }

      
   le_total_beads->setReadOnly(true);
   le_total_beads->setMinimumWidth(200);
   le_total_beads->setAlignment(Qt::AlignVCenter);
   le_total_beads->setPalette( PALET_NORMAL );
   AUTFBACK( le_total_beads );
   le_total_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_used_beads = new QLabel(us_tr(" Used Beads in Model: "), this);
   Q_CHECK_PTR(lbl_used_beads);
   lbl_used_beads->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_used_beads->setPalette( PALET_LABEL );
   AUTFBACK( lbl_used_beads );
   lbl_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_used_beads = new QLineEdit( this );    le_used_beads->setObjectName( "used_beads Line Edit" );
   if ( fabs(results->used_beads_sd) <= 1e-100 )
   {
      le_used_beads->setText(QString("%1").arg(results->used_beads));
   }
   else
   {
      le_used_beads->setText(QString("").sprintf("%u (%4.2e)",
                                                  (int)(results->used_beads + .5),
                                                  results->used_beads_sd));
   }
   le_used_beads->setReadOnly(true);
   le_used_beads->setAlignment(Qt::AlignVCenter);
   le_used_beads->setPalette( PALET_NORMAL );
   AUTFBACK( le_used_beads );
   le_used_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_mass = new QLabel(us_tr(" Molecular Mass: "), this);
   Q_CHECK_PTR(lbl_mass);
   lbl_mass->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_mass->setPalette( PALET_LABEL );
   AUTFBACK( lbl_mass );
   lbl_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_mass = new QLineEdit( this );    le_mass->setObjectName( "mass Line Edit" );
   le_mass->setText(str.sprintf("%6.4e Da", (*results).mass));
   le_mass->setReadOnly(true);
   le_mass->setAlignment(Qt::AlignVCenter);
   le_mass->setPalette( PALET_NORMAL );
   AUTFBACK( le_mass );
   le_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_vbar = new QLabel(us_tr(" Part. Specif. Volume: "), this);
   Q_CHECK_PTR(lbl_vbar);
   lbl_vbar->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_vbar->setPalette( PALET_LABEL );
   AUTFBACK( lbl_vbar );
   lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_vbar = new QLineEdit( this );    le_vbar->setObjectName( "vbar Line Edit" );
   le_vbar->setText(str.sprintf("%5.3f cm^3/g", (*results).vbar));
   le_vbar->setReadOnly(true);
   le_vbar->setAlignment(Qt::AlignVCenter);
   le_vbar->setPalette( PALET_NORMAL );
   AUTFBACK( le_vbar );
   le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_s20w = new QLabel(us_tr(" Sedimentation Coefficient s: "), this);
   Q_CHECK_PTR(lbl_s20w);
   lbl_s20w->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_s20w->setPalette( PALET_LABEL );
   AUTFBACK( lbl_s20w );
   lbl_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_s20w = new QLineEdit( this );    le_s20w->setObjectName( "s20w Line Edit" );
   if (fabs((*results).s20w_sd) <= 1e-100)
   {
      le_s20w->setText(str.sprintf("%4.2e S", (*results).s20w));
   }
   else
   {
      le_s20w->setText(str.sprintf("%4.2e S (%4.2e)", (*results).s20w, (*results).s20w_sd));
   }
   le_s20w->setReadOnly(true);
   le_s20w->setAlignment(Qt::AlignVCenter);
   le_s20w->setPalette( PALET_NORMAL );
   AUTFBACK( le_s20w );
   le_s20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_D20w = new QLabel(us_tr(" Tr. Diffusion Coefficient D: "), this);
   Q_CHECK_PTR(lbl_D20w);
   lbl_D20w->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_D20w->setPalette( PALET_LABEL );
   AUTFBACK( lbl_D20w );
   lbl_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_D20w = new QLineEdit( this );    le_D20w->setObjectName( "D20w Line Edit" );
   if (fabs((*results).D20w_sd) <= 1e-100)
   {
      le_D20w->setText(str.sprintf("%4.2e cm^2/sec", (*results).D20w));
   }
   else
   {
      le_D20w->setText(str.sprintf("%4.2e cm^2/sec (%4.2e)", (*results).D20w, (*results).D20w_sd));
   }
   le_D20w->setReadOnly(true);
   le_D20w->setAlignment(Qt::AlignVCenter);
   le_D20w->setPalette( PALET_NORMAL );
   AUTFBACK( le_D20w );
   le_D20w->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_tau = new QLabel(us_tr(" Relaxation Time, tau(h): "), this);
   Q_CHECK_PTR(lbl_tau);
   lbl_tau->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_tau->setPalette( PALET_LABEL );
   AUTFBACK( lbl_tau );
   lbl_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_tau = new QLineEdit( this );    le_tau->setObjectName( "tau Line Edit" );
   if ( le_method->text() == "Zeno" ||
        (le_method->text() == "SMI" && !((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode )
        ) {
      le_tau->setText( "n/a" ); 
   } else {
      if (fabs((*results).tau_sd) <= 1e-100)
      {
         le_tau->setText(str.sprintf("%4.2e ns", (*results).tau));
      } else {
         le_tau->setText(str.sprintf("%4.2e ns (%4.2e)", (*results).tau, (*results).tau_sd));
      }
   }
   le_tau->setReadOnly(true);
   le_tau->setAlignment(Qt::AlignVCenter);
   le_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_tau );
   le_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_viscosity = new QLabel(us_tr(" Intrinsic Viscosity: "), this);
   Q_CHECK_PTR(lbl_viscosity);
   lbl_viscosity->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_viscosity->setPalette( PALET_LABEL );
   AUTFBACK( lbl_viscosity );
   lbl_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_viscosity = new QLineEdit( this );    le_viscosity->setObjectName( "viscosity Line Edit" );
   if (fabs((*results).viscosity_sd) <= 1e-100)
   {
      le_viscosity->setText(str.sprintf("%4.2e cm^3/g", (*results).viscosity));
   }
   else
   {
      le_viscosity->setText(str.sprintf("%4.2e cm^3/g (%4.2e)", (*results).viscosity, (*results).viscosity_sd));
   }
   le_viscosity->setReadOnly(true);
   le_viscosity->setAlignment(Qt::AlignVCenter);
   le_viscosity->setPalette( PALET_NORMAL );
   AUTFBACK( le_viscosity );
   le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   if ( le_method->text() == "SMI" && !((US_Hydrodyn *)us_hydrodyn)->advanced_config.expert_mode ) {
      le_viscosity->setText( "n/a" );
   }

   lbl_rs = new QLabel(us_tr(" Stokes Radius: "), this);
   Q_CHECK_PTR(lbl_rs);
   lbl_rs->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_rs->setPalette( PALET_LABEL );
   AUTFBACK( lbl_rs );
   lbl_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_rs = new QLineEdit( this );    le_rs->setObjectName( "rs Line Edit" );
   if (fabs((*results).rs_sd) <= 1e-100)
   {
      le_rs->setText(str.sprintf("%4.2e nm", (*results).rs));
   }
   else
   {
      le_rs->setText(str.sprintf("%4.2e nm (%4.2e)", (*results).rs, (*results).rs_sd));
   }
   le_rs->setReadOnly(true);
   le_rs->setAlignment(Qt::AlignVCenter);
   le_rs->setPalette( PALET_NORMAL );
   AUTFBACK( le_rs );
   le_rs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_ff0 = new QLabel(us_tr(" Frictional Ratio: "), this);
   Q_CHECK_PTR(lbl_ff0);
   lbl_ff0->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_ff0->setPalette( PALET_LABEL );
   AUTFBACK( lbl_ff0 );
   lbl_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_ff0 = new QLineEdit( this );    le_ff0->setObjectName( "ff0 Line Edit" );
   if (fabs((*results).ff0_sd) <= 1e-100)
   {
      le_ff0->setText(str.sprintf("%3.2f", (*results).ff0));
   }
   else
   {
      le_ff0->setText(str.sprintf("%3.2f nm (%3.2e)", (*results).ff0, (*results).ff0_sd));
   }
   le_ff0->setReadOnly(true);
   le_ff0->setAlignment(Qt::AlignVCenter);
   le_ff0->setPalette( PALET_NORMAL );
   AUTFBACK( le_ff0 );
   le_ff0->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   lbl_rg = new QLabel(us_tr(" Radius of Gyration: "), this);
   Q_CHECK_PTR(lbl_rg);
   lbl_rg->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_rg->setPalette( PALET_LABEL );
   AUTFBACK( lbl_rg );
   lbl_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_rg = new QLineEdit( this );    le_rg->setObjectName( "rg Line Edit" );
   if (fabs((*results).rg_sd) <= 1e-100)
   {
      le_rg->setText(str.sprintf("%4.2e nm", (*results).rg));
   }
   else
   {
      le_rg->setText(str.sprintf("%4.2e nm (%4.2e)", (*results).rg, (*results).rg_sd));
   }
   le_rg->setReadOnly(true);
   le_rg->setAlignment(Qt::AlignVCenter);
   le_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_rg );
   le_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_load_asa = new QPushButton(us_tr("View ASA Results File"), this);
   Q_CHECK_PTR(pb_load_asa);
   pb_load_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_asa->setMinimumHeight(minHeight2);
   pb_load_asa->setPalette( PALET_PUSHB );
   connect(pb_load_asa, SIGNAL(clicked()), SLOT(load_asa()));

   pb_load_results = new QPushButton(us_tr("View Full Hydrodynamics Results File"), this);
   Q_CHECK_PTR(pb_load_results);
   pb_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_results->setMinimumHeight(minHeight2);
   pb_load_results->setPalette( PALET_PUSHB );
   connect(pb_load_results, SIGNAL(clicked()), SLOT(load_results()));

   pb_load_beadmodel = new QPushButton(us_tr("View Bead Model File"), this);
   Q_CHECK_PTR(pb_load_beadmodel);
   pb_load_beadmodel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_beadmodel->setMinimumHeight(minHeight2);
   pb_load_beadmodel->setPalette( PALET_PUSHB );
   connect(pb_load_beadmodel, SIGNAL(clicked()), SLOT(load_beadmodel()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight2);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int /* rows=13, columns = 2, */ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( lbl_info , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(lbl_name, j, 0);
   background->addWidget(le_name, j, 1);
   j++;
   background->addWidget(lbl_method, j, 0);
   background->addWidget(le_method, j, 1);
   j++;
   background->addWidget(lbl_total_beads, j, 0);
   background->addWidget(le_total_beads, j, 1);
   j++;
   background->addWidget(lbl_used_beads, j, 0);
   background->addWidget(le_used_beads, j, 1);
   j++;
   background->addWidget(lbl_mass, j, 0);
   background->addWidget(le_mass, j, 1);
   j++;
   background->addWidget(lbl_vbar, j, 0);
   background->addWidget(le_vbar, j, 1);
   j++;
   background->addWidget(lbl_s20w, j, 0);
   background->addWidget(le_s20w, j, 1);
   j++;
   background->addWidget(lbl_D20w, j, 0);
   background->addWidget(le_D20w, j, 1);
   j++;
   background->addWidget(lbl_rs, j, 0);
   background->addWidget(le_rs, j, 1);
   j++;
   background->addWidget(lbl_ff0, j, 0);
   background->addWidget(le_ff0, j, 1);
   j++;
   background->addWidget(lbl_rg, j, 0);
   background->addWidget(le_rg, j, 1);
   j++;
   background->addWidget(lbl_tau, j, 0);
   background->addWidget(le_tau, j, 1);
   j++;
   background->addWidget(lbl_viscosity, j, 0);
   background->addWidget(le_viscosity, j, 1);
   j++;
   background->addWidget(pb_load_asa, j, 0);
   background->addWidget(pb_load_beadmodel, j, 1);
   j++;
   background->addWidget( pb_load_results , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Results::cancel()
{
   close();
}

void US_Hydrodyn_Results::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_results.html");
}

void US_Hydrodyn_Results::load_results()
{
   emit ((US_Hydrodyn *)us_hydrodyn)->open_hydro_results();
   
   // QString filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir , le_method->text() == "Zeno" ? "*.zno *.ZNO" : "*.hydro_res *.HYDRO_RES" );
   // if (!filename.isEmpty())
   // {
   //    view_file(filename);
   // }
}

void US_Hydrodyn_Results::load_beadmodel()
{
   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir , "*.bead_model* *.BEAD_MODEL*" );
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn_Results::load_asa()
{
   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , somo_dir , "*.asa_res *.ASA_RES" );
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn_Results::view_file(const QString &filename)
{
#if QT_VERSION < 0x040000
   e = new TextEdit();
   e->setFont(QFont("Courier"));
   e->setPalette( PALET_NORMAL );
   AUTFBACK( e );
   e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   e->load(filename);
   e->show();
#else
   {
      QFile f( filename );
      if ( f.open( QIODevice::ReadOnly ) ) {
         QString text;
         QTextStream ts( &f );
         text = ts.readAll();
         f.close();
         US3i_Editor * edit = new US3i_Editor( US3i_Editor::DEFAULT, true, QString(), 0 );
         edit->setWindowTitle( "US-SOMO Results" );
         edit->resize( 685, 700 );
         edit->move( this->pos().x() + 30, this->pos().y() + 30 );
         edit->e->setFont( QFont( "monospace",
                                  US3i_GuiSettings::fontSize() ) );
         edit->e->setText( text );
         edit->show();
      }
   }
#endif
}

void US_Hydrodyn_Results::closeEvent(QCloseEvent *e)
{
   *result_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

hydro_results US_Hydrodyn_Results::hydro_results_initialized() {
   hydro_results data;

   data.name               = "unknown";
   data.method             = "unknown";

   data.num_models         = 0;
   data.total_beads        = 0e0;
   data.total_beads_sd     = 0e0;
   data.used_beads         = 0e0;
   data.used_beads_sd      = 0e0;
   data.mass               = 0e0;
   data.s20w               = 0e0;
   data.s20w_sd            = 0e0;
   data.D20w               = 0e0;
   data.D20w_sd            = 0e0;
   data.viscosity          = 0e0;
   data.viscosity_sd       = 0e0;
   data.rs                 = 0e0;
   data.rs_sd              = 0e0;
   data.rg                 = 0e0;
   data.rg_sd              = 0e0;
   data.tau                = 0e0;
   data.tau_sd             = 0e0;
   data.vbar               = 0e0;
   data.asa_rg_pos         = 0e0;
   data.asa_rg_neg         = 0e0;
   data.ff0                = 0e0;
   data.ff0_sd             = 0e0;
   data.solvent_name       = "unknown";
   data.solvent_acronym    = "unknown";
   data.temperature        = 0e0;
   data.solvent_viscosity  = 0e0;
   data.solvent_density    = 0e0;
   data.pH                 = 0e0;

   return data;
}
