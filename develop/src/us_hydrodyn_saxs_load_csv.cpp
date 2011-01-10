#include "../include/us_hydrodyn_saxs_load_csv.h"

US_Hydrodyn_Saxs_Load_Csv::US_Hydrodyn_Saxs_Load_Csv(
                                                     QString msg,
                                                     QStringList *qsl_names,
                                                     QStringList *qsl_sel_names,
                                                     bool *create_avg,
                                                     bool *create_std_dev,
                                                     bool *only_plot_stats,
                                                     bool *save_to_csv,
                                                     QString *csv_filename,
                                                     bool *save_original_data,
                                                     bool *run_nnls,
                                                     QString *nnls_target,
                                                     bool expert_mode,
                                                     QWidget *p,
                                                     const char *name
                                                     ) : QDialog(p, name)
{
   this->msg = msg;
   this->qsl_names = qsl_names;
   this->qsl_sel_names = qsl_sel_names;
   this->create_avg = create_avg;
   this->create_std_dev = create_std_dev;
   this->only_plot_stats = only_plot_stats;
   this->save_to_csv = save_to_csv;
   this->csv_filename = csv_filename;
   this->save_original_data = save_original_data;
   this->run_nnls = run_nnls;
   this->nnls_target = nnls_target;
   this->expert_mode = expert_mode;

   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption("Load CSV style SAXS results");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Saxs_Load_Csv::~US_Hydrodyn_Saxs_Load_Csv()
{
}

void US_Hydrodyn_Saxs_Load_Csv::setupGUI()
{
   int minWidth1 = 600;
   int minHeight1 = 30;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_names = new QListBox(this);
   lb_names->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_names->setMinimumHeight(minHeight1 * 15);
   lb_names->setMinimumWidth(minWidth1);
   lb_names->insertStringList(*qsl_names);
   lb_names->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_names->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_names->setSelectionMode(QListBox::Multi);
   lb_names->setEnabled(true);
   connect(lb_names, SIGNAL(selectionChanged()), SLOT(update_selected()));

   cb_create_avg = new QCheckBox(this);
   cb_create_avg->setText(tr(" Create average curve"));
   cb_create_avg->setEnabled(true);
   cb_create_avg->setChecked(*create_avg);
   cb_create_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_avg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_create_avg, SIGNAL(clicked()), this, SLOT(set_create_avg()));

   cb_create_std_dev = new QCheckBox(this);
   cb_create_std_dev->setText(tr(" Create std deviation curves"));
   cb_create_std_dev->setEnabled(true);
   cb_create_std_dev->setChecked(*create_std_dev);
   cb_create_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_std_dev->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_create_std_dev, SIGNAL(clicked()), this, SLOT(set_create_std_dev()));

   cb_only_plot_stats = new QCheckBox(this);
   cb_only_plot_stats->setText(tr(" Only plot stats"));
   cb_only_plot_stats->setEnabled(true);
   cb_only_plot_stats->setChecked(*only_plot_stats);
   cb_only_plot_stats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_only_plot_stats->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_only_plot_stats, SIGNAL(clicked()), this, SLOT(set_only_plot_stats()));

   cb_save_to_csv = new QCheckBox(this);
   cb_save_to_csv->setText(tr(" Save to csv"));
   cb_save_to_csv->setEnabled(true);
   cb_save_to_csv->setChecked(*save_to_csv);
   cb_save_to_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_to_csv->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_save_to_csv, SIGNAL(clicked()), this, SLOT(set_save_to_csv()));

   le_csv_filename = new QLineEdit(this, "csv_filename Line Edit");
   le_csv_filename->setText(*csv_filename);
   le_csv_filename->setAlignment(AlignCenter|AlignVCenter);
   le_csv_filename->setMinimumWidth(100);
   le_csv_filename->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_csv_filename(const QString &)));

   cb_save_original_data = new QCheckBox(this);
   cb_save_original_data->setText(tr(" Include source data"));
   cb_save_original_data->setEnabled(true);
   cb_save_original_data->setChecked(*save_original_data);
   cb_save_original_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_original_data->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_save_original_data, SIGNAL(clicked()), this, SLOT(set_save_original_data()));
   
   if ( expert_mode )
   {
      cb_run_nnls = new QCheckBox(this);
      cb_run_nnls->setText(tr(" NNLS fit"));
      cb_run_nnls->setEnabled(true);
      cb_run_nnls->setChecked(*run_nnls);
      cb_run_nnls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_run_nnls->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      connect(cb_run_nnls, SIGNAL(clicked()), this, SLOT(set_run_nnls()));
      
      lbl_nnls_target = new QLabel(*nnls_target, this);
      lbl_nnls_target->setAlignment(AlignCenter|AlignVCenter);
      lbl_nnls_target->setMinimumHeight(minHeight2);
      lbl_nnls_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
      lbl_nnls_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));
   }

   pb_select_all = new QPushButton(tr("Select All"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_cancel = new QPushButton(tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton(tr("Plot"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_select_all);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_ok);
   hbl_bottom->addSpacing(5);

   QHBoxLayout *hbl_avg_std = new QHBoxLayout;
   hbl_avg_std->addSpacing(5);
   hbl_avg_std->addWidget(cb_create_avg);
   hbl_avg_std->addWidget(cb_create_std_dev);
   hbl_avg_std->addWidget(cb_only_plot_stats);
   hbl_avg_std->addWidget(cb_save_to_csv);
   hbl_avg_std->addWidget(le_csv_filename);
   hbl_avg_std->addWidget(cb_save_original_data);
      hbl_avg_std->addSpacing(5);

   QVBoxLayout *background = new QVBoxLayout(this);
   background->addSpacing(5);
   background->addWidget(lbl_info);
   background->addSpacing(5);
   background->addWidget(lb_names);
   background->addSpacing(5);
   background->addLayout(hbl_avg_std);
   if ( expert_mode )
   {
      QHBoxLayout *hbl_nnls = new QHBoxLayout;
      hbl_nnls->addWidget(cb_run_nnls);
      hbl_nnls->addWidget(lbl_nnls_target);
      background->addLayout(hbl_nnls);
   }
   background->addSpacing(5);
   background->addLayout(hbl_bottom);
   background->addSpacing(5);

   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::select_all()
{
   // if there are any, not selected, select all
   // if all are selected, unselect all
   bool select_all = false;

   // are any unselected ?
   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      if ( !lb_names->isSelected(i) )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      lb_names->setSelected(i, select_all);
   }
}

void US_Hydrodyn_Saxs_Load_Csv::ok()
{
   close();
}

void US_Hydrodyn_Saxs_Load_Csv::cancel()
{
   qsl_sel_names->clear();
   close();
}

void US_Hydrodyn_Saxs_Load_Csv::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_saxs_load_csv.html");
}

void US_Hydrodyn_Saxs_Load_Csv::update_selected()
{
   qsl_sel_names->clear();
   for ( int i = 0; i < lb_names->numRows(); i++ )
   {
      if ( lb_names->isSelected(i) )
      {
         *qsl_sel_names << lb_names->text(i);
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::update_csv_filename(const QString &str)
{
   *csv_filename = str;
}

void US_Hydrodyn_Saxs_Load_Csv::set_create_avg()
{
   *create_avg = cb_create_avg->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::set_create_std_dev()
{
   *create_std_dev = cb_create_std_dev->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::set_only_plot_stats()
{
   *only_plot_stats = cb_only_plot_stats->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::set_save_to_csv()
{
   *save_to_csv = cb_save_to_csv->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::set_save_original_data()
{
   *save_original_data = cb_save_original_data->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::set_run_nnls()
{
   *run_nnls = cb_run_nnls->isChecked();
   if ( *run_nnls )
   {
      if ( qsl_sel_names->size() )
      {
         *nnls_target = *(qsl_sel_names->at(0));
      } else {
         *nnls_target = *(qsl_names->at(0));
      }
      lbl_nnls_target->setText(tr("Target model: ") + *nnls_target);
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Load_Csv::update_enables()
{
   cb_create_avg->setEnabled(qsl_sel_names->size() > 1);
   cb_create_std_dev->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 2);
   cb_only_plot_stats->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 1);
   cb_save_to_csv->setEnabled(
                              ( cb_create_avg->isChecked() || ( expert_mode && cb_run_nnls->isChecked() ) )
                              && qsl_sel_names->size() > 1);
   le_csv_filename->setEnabled(
                               ( cb_create_avg->isChecked() || ( expert_mode && cb_run_nnls->isChecked() ) )
                               && qsl_sel_names->size() > 1);
   cb_save_original_data->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 1);
}
