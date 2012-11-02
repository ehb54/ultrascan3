#include "../include/us_hydrodyn_asa.h"
#include "../include/us_hydrodyn.h"

#define SLASH "/"
#if defined(WIN32)
#  undef SLASH
#  define SLASH "\\"
#endif

US_Hydrodyn_SasOptionsBeadModel::US_Hydrodyn_SasOptionsBeadModel(struct saxs_options *saxs_options, bool *sas_options_bead_model_widget, void *us_hydrodyn, QWidget *p, const char *name) : QFrame(p, name)
{
   this->sas_options_bead_model_widget = sas_options_bead_model_widget;
   this->saxs_options = saxs_options;
   this->us_hydrodyn = us_hydrodyn;
   *sas_options_bead_model_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("US-SOMO SAS Bead Model Options"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_SasOptionsBeadModel::~US_Hydrodyn_SasOptionsBeadModel()
{
   *sas_options_bead_model_widget = false;
}

void US_Hydrodyn_SasOptionsBeadModel::setupGUI()
{
   int minHeight1 = 30;
   lbl_info = new QLabel(tr("US-SOMO SAS Bead Model Options:"), this);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   cb_compute_saxs_coeff_for_bead_models = new QCheckBox(this);
   cb_compute_saxs_coeff_for_bead_models->setText(tr(" Compute SAXS coefficients for bead models"));
   cb_compute_saxs_coeff_for_bead_models->setEnabled(true);
   cb_compute_saxs_coeff_for_bead_models->setChecked((*saxs_options).compute_saxs_coeff_for_bead_models);
   cb_compute_saxs_coeff_for_bead_models->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_saxs_coeff_for_bead_models->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_saxs_coeff_for_bead_models, SIGNAL(clicked()), this, SLOT(set_compute_saxs_coeff_for_bead_models()));

   cb_compute_sans_coeff_for_bead_models = new QCheckBox(this);
   cb_compute_sans_coeff_for_bead_models->setText(tr(" Compute SANS coefficients for bead models"));
   cb_compute_sans_coeff_for_bead_models->setEnabled(true);
   cb_compute_sans_coeff_for_bead_models->setChecked((*saxs_options).compute_sans_coeff_for_bead_models);
   cb_compute_sans_coeff_for_bead_models->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_compute_sans_coeff_for_bead_models->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_compute_sans_coeff_for_bead_models, SIGNAL(clicked()), this, SLOT(set_compute_sans_coeff_for_bead_models()));

   cb_bead_model_rayleigh = new QCheckBox(this);
   cb_bead_model_rayleigh->setText(tr(" Use Rayleigh (1911) for structure factors"));
   cb_bead_model_rayleigh->setEnabled(true);
   cb_bead_model_rayleigh->setChecked((*saxs_options).bead_model_rayleigh);
   cb_bead_model_rayleigh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_model_rayleigh->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_bead_model_rayleigh, SIGNAL(clicked()), this, SLOT(set_bead_model_rayleigh()));

   cb_iq_global_avg_for_bead_models = new QCheckBox(this);
   cb_iq_global_avg_for_bead_models->setText(tr(" Use global average scattering factors for loading bead models"));
   cb_iq_global_avg_for_bead_models->setEnabled(true);
   cb_iq_global_avg_for_bead_models->setChecked((*saxs_options).iq_global_avg_for_bead_models);
   cb_iq_global_avg_for_bead_models->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_iq_global_avg_for_bead_models->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_iq_global_avg_for_bead_models, SIGNAL(clicked()), this, SLOT(set_iq_global_avg_for_bead_models()));

   cb_apply_loaded_sf_repeatedly_to_pdb = new QCheckBox(this);
   cb_apply_loaded_sf_repeatedly_to_pdb->setText(tr(" Apply preloaded individual bead structure factors to bead model PDB"));
   cb_apply_loaded_sf_repeatedly_to_pdb->setEnabled(true);
   cb_apply_loaded_sf_repeatedly_to_pdb->setChecked((*saxs_options).apply_loaded_sf_repeatedly_to_pdb);
   cb_apply_loaded_sf_repeatedly_to_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_apply_loaded_sf_repeatedly_to_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_apply_loaded_sf_repeatedly_to_pdb, SIGNAL(clicked()), this, SLOT(set_apply_loaded_sf_repeatedly_to_pdb()));

   cb_bead_models_use_var_len_sf = new QCheckBox(this);
   cb_bead_models_use_var_len_sf->setText(tr(" Compute and use variable length scattering factors"));
   cb_bead_models_use_var_len_sf->setEnabled(true);
   cb_bead_models_use_var_len_sf->setChecked((*saxs_options).bead_models_use_var_len_sf);
   cb_bead_models_use_var_len_sf->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_bead_models_use_var_len_sf->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_bead_models_use_var_len_sf, SIGNAL(clicked()), this, SLOT(set_bead_models_use_var_len_sf()));

   lbl_bead_models_var_len_sf_max = new QLabel(tr(" Variable length scattering factors max length: "), this);
   lbl_bead_models_var_len_sf_max->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_models_var_len_sf_max->setMinimumHeight(minHeight1);
   lbl_bead_models_var_len_sf_max->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bead_models_var_len_sf_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_bead_models_var_len_sf_max = new QLineEdit( this, "" );
   le_bead_models_var_len_sf_max->setText( QString( "%1" ).arg( (*saxs_options).bead_models_var_len_sf_max ) );
   le_bead_models_var_len_sf_max->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_bead_models_var_len_sf_max->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_bead_models_var_len_sf_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_models_var_len_sf_max_qv = new QIntValidator( 5, 25, this );
   le_bead_models_var_len_sf_max->setValidator( le_bead_models_var_len_sf_max_qv );
   connect( le_bead_models_var_len_sf_max, SIGNAL( textChanged ( const QString & ) ), this, SLOT( update_bead_models_var_len_sf_max( const QString & ) ) );

   lbl_dummy_saxs_name = new QLabel(tr(" Saxs name for dummy atom models: "), this);
   lbl_dummy_saxs_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_dummy_saxs_name->setMinimumHeight(minHeight1);
   lbl_dummy_saxs_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_dummy_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_dummy_saxs_name = new QLineEdit( this, "" );
   le_dummy_saxs_name->setText( (*saxs_options).dummy_saxs_name );
   le_dummy_saxs_name->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   le_dummy_saxs_name->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   le_dummy_saxs_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect( le_dummy_saxs_name, SIGNAL( textChanged ( const QString & ) ), this, SLOT( update_dummy_saxs_name( const QString & ) ) );

   cb_dummy_atom_pdbs_in_nm = new QCheckBox(this);
   cb_dummy_atom_pdbs_in_nm->setText(tr(" Dummy atom PDB's in NM " ) );
   cb_dummy_atom_pdbs_in_nm->setEnabled(true);
   cb_dummy_atom_pdbs_in_nm->setChecked((*saxs_options).dummy_atom_pdbs_in_nm);
   cb_dummy_atom_pdbs_in_nm->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_dummy_atom_pdbs_in_nm->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_dummy_atom_pdbs_in_nm, SIGNAL(clicked()), this, SLOT(set_dummy_atom_pdbs_in_nm()));

   pb_cancel = new QPushButton(tr("Close"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows = 0, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget( lbl_info, j, j, 0, 1 );
   j++;

   background->addMultiCellWidget( cb_compute_saxs_coeff_for_bead_models, j, j, 0, 1 );
   j++;
   background->addMultiCellWidget( cb_compute_sans_coeff_for_bead_models, j, j, 0, 1 );
   j++;
   background->addMultiCellWidget( cb_bead_model_rayleigh               , j, j, 0, 1 );
   j++;
   background->addMultiCellWidget( cb_iq_global_avg_for_bead_models     , j, j, 0, 1 );
   j++;
   background->addMultiCellWidget( cb_apply_loaded_sf_repeatedly_to_pdb , j, j, 0, 1 );
   j++;
   background->addMultiCellWidget( cb_bead_models_use_var_len_sf        , j, j, 0, 1 );
   j++;

   background->addWidget         ( lbl_bead_models_var_len_sf_max       , j, 0 );
   background->addWidget         ( le_bead_models_var_len_sf_max        , j, 1 );
   j++;

   background->addWidget         ( lbl_dummy_saxs_name                  , j, 0 );
   background->addWidget         ( le_dummy_saxs_name                   , j, 1 );
   j++;

   background->addMultiCellWidget( cb_dummy_atom_pdbs_in_nm             , j, j, 0, 1 );
   j++;

   background->addWidget( pb_help  , j, 0 );
   background->addWidget( pb_cancel, j, 1 );
}

void US_Hydrodyn_SasOptionsBeadModel::cancel()
{
   close();
}

void US_Hydrodyn_SasOptionsBeadModel::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   // online_help->show_help("manual/somo_sas_options_bead_model.html");
   online_help->show_help("manual/somo_saxs_options.html");
}

void US_Hydrodyn_SasOptionsBeadModel::closeEvent(QCloseEvent *e)
{
   *sas_options_bead_model_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_SasOptionsBeadModel::set_compute_saxs_coeff_for_bead_models()
{
   (*saxs_options).compute_saxs_coeff_for_bead_models = cb_compute_saxs_coeff_for_bead_models->isChecked();
   if ( (*saxs_options).compute_saxs_coeff_for_bead_models )
   {
      ((US_Hydrodyn *)us_hydrodyn)->saxs_util->setup_saxs_maps( 
                                                               saxs_options->default_atom_filename ,
                                                               saxs_options->default_hybrid_filename ,
                                                               saxs_options->default_saxs_filename 
                                                               );
   }
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_compute_sans_coeff_for_bead_models()
{
   (*saxs_options).compute_sans_coeff_for_bead_models = cb_compute_sans_coeff_for_bead_models->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_model_rayleigh()
{
   (*saxs_options).bead_model_rayleigh = cb_bead_model_rayleigh->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_iq_global_avg_for_bead_models()
{
   (*saxs_options).iq_global_avg_for_bead_models = cb_iq_global_avg_for_bead_models->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_apply_loaded_sf_repeatedly_to_pdb()
{
   (*saxs_options).apply_loaded_sf_repeatedly_to_pdb = cb_apply_loaded_sf_repeatedly_to_pdb->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::set_bead_models_use_var_len_sf()
{
   (*saxs_options).bead_models_use_var_len_sf = cb_bead_models_use_var_len_sf->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_SasOptionsBeadModel::update_bead_models_var_len_sf_max( const QString & qs )
{
   (*saxs_options).bead_models_var_len_sf_max = qs.toUInt();
}

void US_Hydrodyn_SasOptionsBeadModel::update_dummy_saxs_name( const QString & )
{
   bool ok = false;
   QString qs;
   if ( (*saxs_options).dummy_saxs_names.size() )
   {
      qs = QInputDialog::getItem(
                                 tr( "US-SOMO SAS Bead Model Options : Select dummy atom name" ) ,
                                 tr( "Select a defined dummy atom name" ),
                                 (*saxs_options).dummy_saxs_names, 
                                 0, 
                                 FALSE, 
                                 &ok,
                                 this );
   }
   if ( ok && !qs.isEmpty() )
   {
      (*saxs_options).dummy_saxs_name = qs;
   }
   disconnect( le_dummy_saxs_name, SIGNAL( textChanged ( const QString & ) ), 0, 0 );
   le_dummy_saxs_name->setText( (*saxs_options).dummy_saxs_name );
   connect( le_dummy_saxs_name, SIGNAL( textChanged ( const QString & ) ), this, SLOT( update_dummy_saxs_name( const QString & ) ) );
}

void US_Hydrodyn_SasOptionsBeadModel::set_dummy_atom_pdbs_in_nm()
{
   (*saxs_options).dummy_atom_pdbs_in_nm = cb_dummy_atom_pdbs_in_nm->isChecked();
   //   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}
