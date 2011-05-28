#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "qregexp.h"

US_Hydrodyn_Comparative::US_Hydrodyn_Comparative(
                                                 comparative_info *comparative,      
                                                 void *us_hydrodyn, 
                                                 bool *comparative_widget,  // no comparative widget implies non-gui
                                                 QWidget *p,
                                                 const char *name
                                                 ) : QFrame(p, name)
{
   this->comparative = comparative;
   this->us_hydrodyn = us_hydrodyn;

   this->comparative_widget = comparative_widget;
   if ( !comparative_widget )
   {
      return;
   }
   *comparative_widget = true;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Model classifier"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Comparative::~US_Hydrodyn_Comparative()
{
   if ( comparative_widget )
   {
      *comparative_widget = false;
   }
}

comparative_entry US_Hydrodyn_Comparative::empty_comparative_entry( QString name ) 
{
   comparative_entry ce;
   ce.name = name;
   ce.active = false;
   ce.target = 0e0;
   ce.rank = 1;
   ce.include_in_weight = true;
   ce.weight = 1e0;
   ce.buckets = 0;
   ce.min = 0e0;
   ce.max = 0e0;
   ce.store_reference = false;
   ce.store_diff = false;
   ce.store_abs_diff = true;
   return ce;
}

bool US_Hydrodyn_Comparative::comparative_entry_equals( comparative_entry ce1,
                                                        comparative_entry ce2 ) 
{
   return 
      ce1.name == ce2.name &&
      ce1.active == ce2.active &&
      ce1.target == ce2.target &&
      ce1.rank == ce2.rank &&
      ce1.include_in_weight == ce2.include_in_weight &&
      ce1.weight == ce2.weight &&
      ce1.buckets == ce2.buckets &&
      ce1.min == ce2.min &&
      ce1.max == ce2.max &&
      ce1.store_reference == ce2.store_reference &&
      ce1.store_diff == ce2.store_diff &&
      ce1.store_abs_diff == ce2.store_abs_diff;
}

comparative_info US_Hydrodyn_Comparative::empty_comparative_info() 
{
   comparative_info ci;
   ci.ce_s = empty_comparative_entry("Sedimentation coefficient s [S]");
   ci.ce_D = empty_comparative_entry("Translational diffusion coefficient D [cm/sec^2]");
   ci.ce_sr = empty_comparative_entry("Stokes radius [nm]");
   ci.ce_fr = empty_comparative_entry("Frictional ratio");
   ci.ce_rg = empty_comparative_entry("Radius of gyration [nm] (from bead model)");
   ci.ce_tau = empty_comparative_entry("Relaxation Time, tau(h) [ns]");
   ci.ce_eta = empty_comparative_entry("Intrinsic viscosity [cm^3/g]");

   ci.rank = true;
   ci.weight_controls = false;

   ci.path_param = "";
   ci.path_csv = "";

   return ci;
}

bool US_Hydrodyn_Comparative::comparative_info_equals( comparative_info ci1,
                                                       comparative_info ci2 ) 
{
   return 
      comparative_entry_equals(ci1.ce_s, ci2.ce_s) &&
      comparative_entry_equals(ci1.ce_D, ci2.ce_D) &&
      comparative_entry_equals(ci1.ce_sr, ci2.ce_sr) &&
      comparative_entry_equals(ci1.ce_fr, ci2.ce_fr) &&
      comparative_entry_equals(ci1.ce_rg, ci2.ce_rg) &&
      comparative_entry_equals(ci1.ce_tau, ci2.ce_tau) &&
      comparative_entry_equals(ci1.ce_eta, ci2.ce_eta);
}

QString US_Hydrodyn_Comparative::serialize_comparative_entry( comparative_entry ce )
{
   return 
      QString("%1|%1|%1|%1|%1|%1|%1|%1|%1|%1|%1|%1\n")
      .arg(ce.name)
      .arg(ce.active)
      .arg(ce.target)
      .arg(ce.rank)
      .arg(ce.include_in_weight)
      .arg(ce.weight)
      .arg(ce.buckets)
      .arg(ce.min)
      .arg(ce.max)
      .arg(ce.store_reference)
      .arg(ce.store_diff)
      .arg(ce.store_abs_diff);
}   

comparative_entry US_Hydrodyn_Comparative::deserialize_comparative_entry( QString qs )
{
   QStringList qsl = QStringList::split("|",qs);
   comparative_entry ce;
   if ( qsl.size() < 12 )
   {
      serial_error = "Error: invalid parameter file ";
   }
   int pos = 0;
   ce.name = *(qsl.at(pos++));
   ce.active = (bool)(*(qsl.at(pos++))).toInt();
   ce.target = (*(qsl.at(pos++))).toDouble();
   ce.rank = (*(qsl.at(pos++))).toInt();
   ce.include_in_weight = (bool)(*(qsl.at(pos++))).toInt();
   ce.weight = (*(qsl.at(pos++))).toDouble();
   ce.buckets = (*(qsl.at(pos++))).toInt();
   ce.min = (*(qsl.at(pos++))).toDouble();
   ce.max = (*(qsl.at(pos++))).toDouble();
   ce.store_reference = (bool)(*(qsl.at(pos++))).toInt();
   ce.store_diff = (bool)(*(qsl.at(pos++))).toInt();
   ce.store_abs_diff = (bool)(*(qsl.at(pos++))).toInt();
   return ce;
}   

QString US_Hydrodyn_Comparative::serialize_comparative_info( comparative_info ci )
{
   QString qs = QString("%1|%1\n")
      .arg(ci.rank)
      .arg(ci.weight_controls);
   qs += serialize_comparative_entry(ci.ce_s);
   qs += serialize_comparative_entry(ci.ce_D);
   qs += serialize_comparative_entry(ci.ce_sr);
   qs += serialize_comparative_entry(ci.ce_fr);
   qs += serialize_comparative_entry(ci.ce_rg);
   qs += serialize_comparative_entry(ci.ce_tau);
   qs += serialize_comparative_entry(ci.ce_eta);
   return qs;
}   

comparative_info US_Hydrodyn_Comparative::deserialize_comparative_info( QString qs )
{
   comparative_info ci = US_Hydrodyn_Comparative::empty_comparative_info();
   serial_error = "";
   QStringList qsl = QStringList::split("\n",qs);
   if ( qsl.size() < 8 )
   {
      cout << QString("qsl size %1 < 8 qs:<%1>\n").arg(qs,qsl.size());
      serial_error = tr("Error: invalid parameter file (too few lines)");
      return ci;
   }
   QStringList qsl0 = QStringList::split("|",*(qsl.at(0)));
   if ( qsl0.size() < 2 )
   {
      serial_error = tr("Error: invalid parameter file (line 1 too short)");
      return ci;
   }

   ci.rank = (bool)(*(qsl0.at(0))).toInt();
   ci.weight_controls = (bool)(*(qsl0.at(1))).toInt();

   int pos = 1;
   ci.ce_s = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_D = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_sr = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_fr = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_rg = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_tau = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_eta = deserialize_comparative_entry(*(qsl.at(pos++)));
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(tr(" line %1")).arg(pos);
      return ci;
   }
   return ci;
}   

void US_Hydrodyn_Comparative::setupGUI()
{
   int minHeight0 = 0;
   int minHeight1 = 30;
   int minHeight2a = 120;
   int minHeight2b = 60;
   int minHeight3 = 22;

   QColorGroup cg_modes = USglobal->global_colors.cg_label;
   cg_modes.setColor(QColorGroup::Shadow, Qt::gray);
   cg_modes.setColor(QColorGroup::Dark, Qt::gray);
   cg_modes.setColor(QColorGroup::Light, Qt::white);
   cg_modes.setColor(QColorGroup::Midlight, Qt::gray);

   QFont qf_modes = QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold );
   QPalette qp_modes = QPalette( cg_modes, cg_modes, cg_modes );

   lbl_title_param = new QLabel(tr("Select parameters"), this);
   lbl_title_param->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title_param->setAlignment(AlignCenter|AlignVCenter);
   lbl_title_param->setMinimumHeight(minHeight1);
   lbl_title_param->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_active = new QLabel(tr("Select to enable variable comparison"), this);
   lbl_active->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_active->setAlignment(AlignCenter|AlignVCenter);
   lbl_active->setMinimumHeight(minHeight1);
   lbl_active->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_active->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_target = new QLabel(tr("Experimental\nvalue"), this);
   lbl_target->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_target->setAlignment(AlignCenter|AlignVCenter);
   lbl_target->setMinimumHeight(minHeight1);
   lbl_target->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_sort = new QLabel(tr("Sort results"), this);
   lbl_sort->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_sort->setAlignment(AlignCenter|AlignVCenter);
   lbl_sort->setMinimumHeight(minHeight1);
   lbl_sort->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_rank = new QCheckBox(this);
   cb_rank->setText(tr("By ranked\nabsolute\ndifference"));
   cb_rank->setEnabled(true);
   cb_rank->setMinimumHeight(minHeight2a);
   cb_rank->setChecked(comparative->rank);
   cb_rank->setFont(qf_modes);
   cb_rank->setPalette(qp_modes);
   connect(cb_rank, SIGNAL(clicked()), SLOT(set_rank()));

   cb_weight_controls = new QCheckBox(this);
   cb_weight_controls->setMinimumHeight(minHeight2b);
   cb_weight_controls->setText(tr("By weighted\nsum of absolute\ndifferences"));
   cb_weight_controls->setEnabled(true);
   cb_weight_controls->setChecked(comparative->weight_controls);
   cb_weight_controls->setFont(qf_modes);
   cb_weight_controls->setPalette(qp_modes);
   connect(cb_weight_controls, SIGNAL(clicked()), SLOT(set_weight_controls()));

   lbl_include_in_weight = new QLabel(tr("Include"), this);
   lbl_include_in_weight->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_include_in_weight->setAlignment(AlignCenter|AlignVCenter);
   lbl_include_in_weight->setMinimumHeight(minHeight0);
   lbl_include_in_weight->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_include_in_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_weight = new QLabel(tr("Weight"), this);
   lbl_weight->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_weight->setAlignment(AlignCenter|AlignVCenter);
   lbl_weight->setMinimumHeight(minHeight0);
   lbl_weight->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_ec = new QLabel(tr("Equivalance class controls"), this);
   lbl_ec->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_ec->setAlignment(AlignCenter|AlignVCenter);
   lbl_ec->setMinimumHeight(minHeight1);
   lbl_ec->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_ec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_buckets = new QLabel(tr("Number of\npartitions"), this);
   lbl_buckets->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_buckets->setAlignment(AlignCenter|AlignVCenter);
   lbl_buckets->setMinimumHeight(minHeight1);
   lbl_buckets->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_buckets->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_min = new QLabel(tr("Minimum\nExperimental\nvalue"), this);
   lbl_min->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_min->setAlignment(AlignCenter|AlignVCenter);
   lbl_min->setMinimumHeight(minHeight1);
   lbl_min->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_max = new QLabel(tr("Maximum\nExperimental\nvalue"), this);
   lbl_max->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_max->setAlignment(AlignCenter|AlignVCenter);
   lbl_max->setMinimumHeight(minHeight1);
   lbl_max->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_csv_controls = new QLabel(tr("Add columns to results"), this);
   lbl_csv_controls->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_csv_controls->setAlignment(AlignCenter|AlignVCenter);
   lbl_csv_controls->setMinimumHeight(minHeight1);
   lbl_csv_controls->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_csv_controls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_reference = new QLabel(tr("Experimental\nvalue"), this);
   lbl_store_reference->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_reference->setAlignment(AlignCenter|AlignVCenter);
   lbl_store_reference->setMinimumHeight(minHeight1);
   lbl_store_reference->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_store_reference->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_diff = new QLabel(tr("Difference"), this);
   lbl_store_diff->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_diff->setAlignment(AlignCenter|AlignVCenter);
   lbl_store_diff->setMinimumHeight(minHeight1);
   lbl_store_diff->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_store_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_abs_diff = new QLabel(tr("Absolute\ndifference"), this);
   lbl_store_abs_diff->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_abs_diff->setAlignment(AlignCenter|AlignVCenter);
   lbl_store_abs_diff->setMinimumHeight(minHeight1);
   lbl_store_abs_diff->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_store_abs_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   // ------------------- s -------------------
   cb_active_s = new QCheckBox(this);
   cb_active_s->setMinimumHeight(minHeight3);
   cb_active_s->setText(comparative->ce_s.name);
   cb_active_s->setEnabled(true);
   cb_active_s->setChecked(comparative->ce_s.active);
   cb_active_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_s, SIGNAL(clicked()), SLOT(set_active_s()));

   le_target_s = new QLineEdit(this, "target_s Line Edit");
   le_target_s->setText(QString("%1").arg(comparative->ce_s.target));
   // le_target_s->setMinimumHeight(minHeight1);
   le_target_s->setAlignment(AlignCenter|AlignVCenter);
   le_target_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_s, SIGNAL(textChanged(const QString &)), SLOT(update_target_s(const QString &)));

   le_rank_s = new QLineEdit(this, "rank_s Line Edit");
   le_rank_s->setText(QString("%1").arg(comparative->ce_s.rank));
   // le_rank_s->setMinimumHeight(minHeight1);
   le_rank_s->setAlignment(AlignCenter|AlignVCenter);
   le_rank_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_s, SIGNAL(textChanged(const QString &)), SLOT(update_rank_s(const QString &)));
   
   cb_include_in_weight_s = new QCheckBox(this);
   cb_include_in_weight_s->setMinimumHeight(minHeight3);
   cb_include_in_weight_s->setText("");
   cb_include_in_weight_s->setChecked(comparative->ce_s.include_in_weight);
   cb_include_in_weight_s->setEnabled(true);
   cb_include_in_weight_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_s, SIGNAL(clicked()), SLOT(set_include_in_weight_s()));

   le_weight_s = new QLineEdit(this, "weight_s Line Edit");
   le_weight_s->setText(QString("%1").arg(comparative->ce_s.weight));
   // le_weight_s->setMinimumHeight(minHeight1);
   le_weight_s->setAlignment(AlignCenter|AlignVCenter);
   le_weight_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_s, SIGNAL(textChanged(const QString &)), SLOT(update_weight_s(const QString &)));

   le_buckets_s = new QLineEdit(this, "buckets_s Line Edit");
   le_buckets_s->setText(QString("%1").arg(comparative->ce_s.buckets));
   // le_buckets_s->setMinimumHeight(minHeight1);
   le_buckets_s->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_s, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_s(const QString &)));

   le_min_s = new QLineEdit(this, "min_s Line Edit");
   le_min_s->setText(QString("%1").arg(comparative->ce_s.min));
   // le_min_s->setMinimumHeight(minHeight1);
   le_min_s->setAlignment(AlignCenter|AlignVCenter);
   le_min_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_s, SIGNAL(textChanged(const QString &)), SLOT(update_min_s(const QString &)));

   le_max_s = new QLineEdit(this, "max_s Line Edit");
   le_max_s->setText(QString("%1").arg(comparative->ce_s.max));
   // le_max_s->setMinimumHeight(minHeight1);
   le_max_s->setAlignment(AlignCenter|AlignVCenter);
   le_max_s->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_s, SIGNAL(textChanged(const QString &)), SLOT(update_max_s(const QString &)));

   cb_store_reference_s = new QCheckBox(this);
   cb_store_reference_s->setMinimumHeight(minHeight3);
   cb_store_reference_s->setText("");
   cb_store_reference_s->setChecked(comparative->ce_s.store_reference);
   cb_store_reference_s->setEnabled(true);
   cb_store_reference_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_s, SIGNAL(clicked()), SLOT(set_store_reference_s()));

   cb_store_diff_s = new QCheckBox(this);
   cb_store_diff_s->setMinimumHeight(minHeight3);
   cb_store_diff_s->setText("");
   cb_store_diff_s->setChecked(comparative->ce_s.store_diff);
   cb_store_diff_s->setEnabled(true);
   cb_store_diff_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_s, SIGNAL(clicked()), SLOT(set_store_diff_s()));

   cb_store_abs_diff_s = new QCheckBox(this);
   cb_store_abs_diff_s->setMinimumHeight(minHeight3);
   cb_store_abs_diff_s->setText("");
   cb_store_abs_diff_s->setChecked(comparative->ce_s.store_abs_diff);
   cb_store_abs_diff_s->setEnabled(true);
   cb_store_abs_diff_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_s->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_s, SIGNAL(clicked()), SLOT(set_store_abs_diff_s()));

   // ------------------- D -------------------
   cb_active_D = new QCheckBox(this);
   cb_active_D->setMinimumHeight(minHeight3);
   cb_active_D->setText(comparative->ce_D.name);
   cb_active_D->setEnabled(true);
   cb_active_D->setChecked(comparative->ce_D.active);
   cb_active_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_D->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_D, SIGNAL(clicked()), SLOT(set_active_D()));

   le_target_D = new QLineEdit(this, "target_D Line Edit");
   le_target_D->setText(QString("%1").arg(comparative->ce_D.target));
   // le_target_D->setMinimumHeight(minHeight1);
   le_target_D->setAlignment(AlignCenter|AlignVCenter);
   le_target_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_D, SIGNAL(textChanged(const QString &)), SLOT(update_target_D(const QString &)));

   le_rank_D = new QLineEdit(this, "rank_D Line Edit");
   le_rank_D->setText(QString("%1").arg(comparative->ce_D.rank));
   // le_rank_D->setMinimumHeight(minHeight1);
   le_rank_D->setAlignment(AlignCenter|AlignVCenter);
   le_rank_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_D, SIGNAL(textChanged(const QString &)), SLOT(update_rank_D(const QString &)));

   cb_include_in_weight_D = new QCheckBox(this);
   cb_include_in_weight_D->setMinimumHeight(minHeight3);
   cb_include_in_weight_D->setText("");
   cb_include_in_weight_D->setChecked(comparative->ce_D.include_in_weight);
   cb_include_in_weight_D->setEnabled(true);
   cb_include_in_weight_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_D->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_D, SIGNAL(clicked()), SLOT(set_include_in_weight_D()));

   le_weight_D = new QLineEdit(this, "weight_D Line Edit");
   le_weight_D->setText(QString("%1").arg(comparative->ce_D.weight));
   // le_weight_D->setMinimumHeight(minHeight1);
   le_weight_D->setAlignment(AlignCenter|AlignVCenter);
   le_weight_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_D, SIGNAL(textChanged(const QString &)), SLOT(update_weight_D(const QString &)));

   le_buckets_D = new QLineEdit(this, "buckets_D Line Edit");
   le_buckets_D->setText(QString("%1").arg(comparative->ce_D.buckets));
   // le_buckets_D->setMinimumHeight(minHeight1);
   le_buckets_D->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_D, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_D(const QString &)));

   le_min_D = new QLineEdit(this, "min_D Line Edit");
   le_min_D->setText(QString("%1").arg(comparative->ce_D.min));
   // le_min_D->setMinimumHeight(minHeight1);
   le_min_D->setAlignment(AlignCenter|AlignVCenter);
   le_min_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_D, SIGNAL(textChanged(const QString &)), SLOT(update_min_D(const QString &)));

   le_max_D = new QLineEdit(this, "max_D Line Edit");
   le_max_D->setText(QString("%1").arg(comparative->ce_D.max));
   // le_max_D->setMinimumHeight(minHeight1);
   le_max_D->setAlignment(AlignCenter|AlignVCenter);
   le_max_D->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_D, SIGNAL(textChanged(const QString &)), SLOT(update_max_D(const QString &)));

   cb_store_reference_D = new QCheckBox(this);
   cb_store_reference_D->setMinimumHeight(minHeight3);
   cb_store_reference_D->setText("");
   cb_store_reference_D->setChecked(comparative->ce_D.store_reference);
   cb_store_reference_D->setEnabled(true);
   cb_store_reference_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_D->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_D, SIGNAL(clicked()), SLOT(set_store_reference_D()));

   cb_store_diff_D = new QCheckBox(this);
   cb_store_diff_D->setMinimumHeight(minHeight3);
   cb_store_diff_D->setText("");
   cb_store_diff_D->setChecked(comparative->ce_D.store_diff);
   cb_store_diff_D->setEnabled(true);
   cb_store_diff_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_D->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_D, SIGNAL(clicked()), SLOT(set_store_diff_D()));

   cb_store_abs_diff_D = new QCheckBox(this);
   cb_store_abs_diff_D->setMinimumHeight(minHeight3);
   cb_store_abs_diff_D->setText("");
   cb_store_abs_diff_D->setChecked(comparative->ce_D.store_abs_diff);
   cb_store_abs_diff_D->setEnabled(true);
   cb_store_abs_diff_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_D->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_D, SIGNAL(clicked()), SLOT(set_store_abs_diff_D()));

   // ------------------- sr -------------------
   cb_active_sr = new QCheckBox(this);
   cb_active_sr->setMinimumHeight(minHeight3);
   cb_active_sr->setText(comparative->ce_sr.name);
   cb_active_sr->setEnabled(true);
   cb_active_sr->setChecked(comparative->ce_sr.active);
   cb_active_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_sr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_sr, SIGNAL(clicked()), SLOT(set_active_sr()));

   le_target_sr = new QLineEdit(this, "target_sr Line Edit");
   le_target_sr->setText(QString("%1").arg(comparative->ce_sr.target));
   // le_target_sr->setMinimumHeight(minHeight1);
   le_target_sr->setAlignment(AlignCenter|AlignVCenter);
   le_target_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_sr, SIGNAL(textChanged(const QString &)), SLOT(update_target_sr(const QString &)));

   le_rank_sr = new QLineEdit(this, "rank_sr Line Edit");
   le_rank_sr->setText(QString("%1").arg(comparative->ce_sr.rank));
   // le_rank_sr->setMinimumHeight(minHeight1);
   le_rank_sr->setAlignment(AlignCenter|AlignVCenter);
   le_rank_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_sr, SIGNAL(textChanged(const QString &)), SLOT(update_rank_sr(const QString &)));

   cb_include_in_weight_sr = new QCheckBox(this);
   cb_include_in_weight_sr->setMinimumHeight(minHeight3);
   cb_include_in_weight_sr->setText("");
   cb_include_in_weight_sr->setChecked(comparative->ce_sr.include_in_weight);
   cb_include_in_weight_sr->setEnabled(true);
   cb_include_in_weight_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_sr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_sr, SIGNAL(clicked()), SLOT(set_include_in_weight_sr()));

   le_weight_sr = new QLineEdit(this, "weight_sr Line Edit");
   le_weight_sr->setText(QString("%1").arg(comparative->ce_sr.weight));
   // le_weight_sr->setMinimumHeight(minHeight1);
   le_weight_sr->setAlignment(AlignCenter|AlignVCenter);
   le_weight_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_sr, SIGNAL(textChanged(const QString &)), SLOT(update_weight_sr(const QString &)));

   le_buckets_sr = new QLineEdit(this, "buckets_sr Line Edit");
   le_buckets_sr->setText(QString("%1").arg(comparative->ce_sr.buckets));
   // le_buckets_sr->setMinimumHeight(minHeight1);
   le_buckets_sr->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_sr, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_sr(const QString &)));

   le_min_sr = new QLineEdit(this, "min_sr Line Edit");
   le_min_sr->setText(QString("%1").arg(comparative->ce_sr.min));
   // le_min_sr->setMinimumHeight(minHeight1);
   le_min_sr->setAlignment(AlignCenter|AlignVCenter);
   le_min_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_sr, SIGNAL(textChanged(const QString &)), SLOT(update_min_sr(const QString &)));

   le_max_sr = new QLineEdit(this, "max_sr Line Edit");
   le_max_sr->setText(QString("%1").arg(comparative->ce_sr.max));
   // le_max_sr->setMinimumHeight(minHeight1);
   le_max_sr->setAlignment(AlignCenter|AlignVCenter);
   le_max_sr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_sr, SIGNAL(textChanged(const QString &)), SLOT(update_max_sr(const QString &)));

   cb_store_reference_sr = new QCheckBox(this);
   cb_store_reference_sr->setMinimumHeight(minHeight3);
   cb_store_reference_sr->setText("");
   cb_store_reference_sr->setChecked(comparative->ce_sr.store_reference);
   cb_store_reference_sr->setEnabled(true);
   cb_store_reference_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_sr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_sr, SIGNAL(clicked()), SLOT(set_store_reference_sr()));

   cb_store_diff_sr = new QCheckBox(this);
   cb_store_diff_sr->setMinimumHeight(minHeight3);
   cb_store_diff_sr->setText("");
   cb_store_diff_sr->setChecked(comparative->ce_sr.store_diff);
   cb_store_diff_sr->setEnabled(true);
   cb_store_diff_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_sr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_sr, SIGNAL(clicked()), SLOT(set_store_diff_sr()));

   cb_store_abs_diff_sr = new QCheckBox(this);
   cb_store_abs_diff_sr->setMinimumHeight(minHeight3);
   cb_store_abs_diff_sr->setText("");
   cb_store_abs_diff_sr->setChecked(comparative->ce_sr.store_abs_diff);
   cb_store_abs_diff_sr->setEnabled(true);
   cb_store_abs_diff_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_sr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_sr, SIGNAL(clicked()), SLOT(set_store_abs_diff_sr()));

   // ------------------- fr -------------------
   cb_active_fr = new QCheckBox(this);
   cb_active_fr->setMinimumHeight(minHeight3);
   cb_active_fr->setText(comparative->ce_fr.name);
   cb_active_fr->setEnabled(true);
   cb_active_fr->setChecked(comparative->ce_fr.active);
   cb_active_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_fr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_fr, SIGNAL(clicked()), SLOT(set_active_fr()));

   le_target_fr = new QLineEdit(this, "target_fr Line Edit");
   le_target_fr->setText(QString("%1").arg(comparative->ce_fr.target));
   // le_target_fr->setMinimumHeight(minHeight1);
   le_target_fr->setAlignment(AlignCenter|AlignVCenter);
   le_target_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_fr, SIGNAL(textChanged(const QString &)), SLOT(update_target_fr(const QString &)));

   le_rank_fr = new QLineEdit(this, "rank_fr Line Edit");
   le_rank_fr->setText(QString("%1").arg(comparative->ce_fr.rank));
   // le_rank_fr->setMinimumHeight(minHeight1);
   le_rank_fr->setAlignment(AlignCenter|AlignVCenter);
   le_rank_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_fr, SIGNAL(textChanged(const QString &)), SLOT(update_rank_fr(const QString &)));

   cb_include_in_weight_fr = new QCheckBox(this);
   cb_include_in_weight_fr->setMinimumHeight(minHeight3);
   cb_include_in_weight_fr->setText("");
   cb_include_in_weight_fr->setChecked(comparative->ce_fr.include_in_weight);
   cb_include_in_weight_fr->setEnabled(true);
   cb_include_in_weight_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_fr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_fr, SIGNAL(clicked()), SLOT(set_include_in_weight_fr()));

   le_weight_fr = new QLineEdit(this, "weight_fr Line Edit");
   le_weight_fr->setText(QString("%1").arg(comparative->ce_fr.weight));
   // le_weight_fr->setMinimumHeight(minHeight1);
   le_weight_fr->setAlignment(AlignCenter|AlignVCenter);
   le_weight_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_fr, SIGNAL(textChanged(const QString &)), SLOT(update_weight_fr(const QString &)));

   le_buckets_fr = new QLineEdit(this, "buckets_fr Line Edit");
   le_buckets_fr->setText(QString("%1").arg(comparative->ce_fr.buckets));
   // le_buckets_fr->setMinimumHeight(minHeight1);
   le_buckets_fr->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_fr, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_fr(const QString &)));

   le_min_fr = new QLineEdit(this, "min_fr Line Edit");
   le_min_fr->setText(QString("%1").arg(comparative->ce_fr.min));
   // le_min_fr->setMinimumHeight(minHeight1);
   le_min_fr->setAlignment(AlignCenter|AlignVCenter);
   le_min_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_fr, SIGNAL(textChanged(const QString &)), SLOT(update_min_fr(const QString &)));

   le_max_fr = new QLineEdit(this, "max_fr Line Edit");
   le_max_fr->setText(QString("%1").arg(comparative->ce_fr.max));
   // le_max_fr->setMinimumHeight(minHeight1);
   le_max_fr->setAlignment(AlignCenter|AlignVCenter);
   le_max_fr->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_fr, SIGNAL(textChanged(const QString &)), SLOT(update_max_fr(const QString &)));

   cb_store_reference_fr = new QCheckBox(this);
   cb_store_reference_fr->setMinimumHeight(minHeight3);
   cb_store_reference_fr->setText("");
   cb_store_reference_fr->setChecked(comparative->ce_fr.store_reference);
   cb_store_reference_fr->setEnabled(true);
   cb_store_reference_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_fr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_fr, SIGNAL(clicked()), SLOT(set_store_reference_fr()));

   cb_store_diff_fr = new QCheckBox(this);
   cb_store_diff_fr->setMinimumHeight(minHeight3);
   cb_store_diff_fr->setText("");
   cb_store_diff_fr->setChecked(comparative->ce_fr.store_diff);
   cb_store_diff_fr->setEnabled(true);
   cb_store_diff_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_fr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_fr, SIGNAL(clicked()), SLOT(set_store_diff_fr()));

   cb_store_abs_diff_fr = new QCheckBox(this);
   cb_store_abs_diff_fr->setMinimumHeight(minHeight3);
   cb_store_abs_diff_fr->setText("");
   cb_store_abs_diff_fr->setChecked(comparative->ce_fr.store_abs_diff);
   cb_store_abs_diff_fr->setEnabled(true);
   cb_store_abs_diff_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_fr->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_fr, SIGNAL(clicked()), SLOT(set_store_abs_diff_fr()));

   // ------------------- rg -------------------
   cb_active_rg = new QCheckBox(this);
   cb_active_rg->setMinimumHeight(minHeight3);
   cb_active_rg->setText(comparative->ce_rg.name);
   cb_active_rg->setEnabled(true);
   cb_active_rg->setChecked(comparative->ce_rg.active);
   cb_active_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_rg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_rg, SIGNAL(clicked()), SLOT(set_active_rg()));

   le_target_rg = new QLineEdit(this, "target_rg Line Edit");
   le_target_rg->setText(QString("%1").arg(comparative->ce_rg.target));
   // le_target_rg->setMinimumHeight(minHeight1);
   le_target_rg->setAlignment(AlignCenter|AlignVCenter);
   le_target_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_rg, SIGNAL(textChanged(const QString &)), SLOT(update_target_rg(const QString &)));

   le_rank_rg = new QLineEdit(this, "rank_rg Line Edit");
   le_rank_rg->setText(QString("%1").arg(comparative->ce_rg.rank));
   // le_rank_rg->setMinimumHeight(minHeight1);
   le_rank_rg->setAlignment(AlignCenter|AlignVCenter);
   le_rank_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_rg, SIGNAL(textChanged(const QString &)), SLOT(update_rank_rg(const QString &)));

   cb_include_in_weight_rg = new QCheckBox(this);
   cb_include_in_weight_rg->setMinimumHeight(minHeight3);
   cb_include_in_weight_rg->setText("");
   cb_include_in_weight_rg->setChecked(comparative->ce_rg.include_in_weight);
   cb_include_in_weight_rg->setEnabled(true);
   cb_include_in_weight_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_rg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_rg, SIGNAL(clicked()), SLOT(set_include_in_weight_rg()));

   le_weight_rg = new QLineEdit(this, "weight_rg Line Edit");
   le_weight_rg->setText(QString("%1").arg(comparative->ce_rg.weight));
   // le_weight_rg->setMinimumHeight(minHeight1);
   le_weight_rg->setAlignment(AlignCenter|AlignVCenter);
   le_weight_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_rg, SIGNAL(textChanged(const QString &)), SLOT(update_weight_rg(const QString &)));

   le_buckets_rg = new QLineEdit(this, "buckets_rg Line Edit");
   le_buckets_rg->setText(QString("%1").arg(comparative->ce_rg.buckets));
   // le_buckets_rg->setMinimumHeight(minHeight1);
   le_buckets_rg->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_rg, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_rg(const QString &)));

   le_min_rg = new QLineEdit(this, "min_rg Line Edit");
   le_min_rg->setText(QString("%1").arg(comparative->ce_rg.min));
   // le_min_rg->setMinimumHeight(minHeight1);
   le_min_rg->setAlignment(AlignCenter|AlignVCenter);
   le_min_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_rg, SIGNAL(textChanged(const QString &)), SLOT(update_min_rg(const QString &)));

   le_max_rg = new QLineEdit(this, "max_rg Line Edit");
   le_max_rg->setText(QString("%1").arg(comparative->ce_rg.max));
   // le_max_rg->setMinimumHeight(minHeight1);
   le_max_rg->setAlignment(AlignCenter|AlignVCenter);
   le_max_rg->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_rg, SIGNAL(textChanged(const QString &)), SLOT(update_max_rg(const QString &)));

   cb_store_reference_rg = new QCheckBox(this);
   cb_store_reference_rg->setMinimumHeight(minHeight3);
   cb_store_reference_rg->setText("");
   cb_store_reference_rg->setChecked(comparative->ce_rg.store_reference);
   cb_store_reference_rg->setEnabled(true);
   cb_store_reference_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_rg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_rg, SIGNAL(clicked()), SLOT(set_store_reference_rg()));

   cb_store_diff_rg = new QCheckBox(this);
   cb_store_diff_rg->setMinimumHeight(minHeight3);
   cb_store_diff_rg->setText("");
   cb_store_diff_rg->setChecked(comparative->ce_rg.store_diff);
   cb_store_diff_rg->setEnabled(true);
   cb_store_diff_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_rg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_rg, SIGNAL(clicked()), SLOT(set_store_diff_rg()));

   cb_store_abs_diff_rg = new QCheckBox(this);
   cb_store_abs_diff_rg->setMinimumHeight(minHeight3);
   cb_store_abs_diff_rg->setText("");
   cb_store_abs_diff_rg->setChecked(comparative->ce_rg.store_abs_diff);
   cb_store_abs_diff_rg->setEnabled(true);
   cb_store_abs_diff_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_rg->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_rg, SIGNAL(clicked()), SLOT(set_store_abs_diff_rg()));

   // ------------------- tau -------------------
   cb_active_tau = new QCheckBox(this);
   cb_active_tau->setMinimumHeight(minHeight3);
   cb_active_tau->setText(comparative->ce_tau.name);
   cb_active_tau->setEnabled(true);
   cb_active_tau->setChecked(comparative->ce_tau.active);
   cb_active_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_tau->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_tau, SIGNAL(clicked()), SLOT(set_active_tau()));

   le_target_tau = new QLineEdit(this, "target_tau Line Edit");
   le_target_tau->setText(QString("%1").arg(comparative->ce_tau.target));
   // le_target_tau->setMinimumHeight(minHeight1);
   le_target_tau->setAlignment(AlignCenter|AlignVCenter);
   le_target_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_tau, SIGNAL(textChanged(const QString &)), SLOT(update_target_tau(const QString &)));

   le_rank_tau = new QLineEdit(this, "rank_tau Line Edit");
   le_rank_tau->setText(QString("%1").arg(comparative->ce_tau.rank));
   // le_rank_tau->setMinimumHeight(minHeight1);
   le_rank_tau->setAlignment(AlignCenter|AlignVCenter);
   le_rank_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_tau, SIGNAL(textChanged(const QString &)), SLOT(update_rank_tau(const QString &)));

   cb_include_in_weight_tau = new QCheckBox(this);
   cb_include_in_weight_tau->setMinimumHeight(minHeight3);
   cb_include_in_weight_tau->setText("");
   cb_include_in_weight_tau->setChecked(comparative->ce_tau.include_in_weight);
   cb_include_in_weight_tau->setEnabled(true);
   cb_include_in_weight_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_tau->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_tau, SIGNAL(clicked()), SLOT(set_include_in_weight_tau()));

   le_weight_tau = new QLineEdit(this, "weight_tau Line Edit");
   le_weight_tau->setText(QString("%1").arg(comparative->ce_tau.weight));
   // le_weight_tau->setMinimumHeight(minHeight1);
   le_weight_tau->setAlignment(AlignCenter|AlignVCenter);
   le_weight_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_tau, SIGNAL(textChanged(const QString &)), SLOT(update_weight_tau(const QString &)));

   le_buckets_tau = new QLineEdit(this, "buckets_tau Line Edit");
   le_buckets_tau->setText(QString("%1").arg(comparative->ce_tau.buckets));
   // le_buckets_tau->setMinimumHeight(minHeight1);
   le_buckets_tau->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_tau, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_tau(const QString &)));

   le_min_tau = new QLineEdit(this, "min_tau Line Edit");
   le_min_tau->setText(QString("%1").arg(comparative->ce_tau.min));
   // le_min_tau->setMinimumHeight(minHeight1);
   le_min_tau->setAlignment(AlignCenter|AlignVCenter);
   le_min_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_tau, SIGNAL(textChanged(const QString &)), SLOT(update_min_tau(const QString &)));

   le_max_tau = new QLineEdit(this, "max_tau Line Edit");
   le_max_tau->setText(QString("%1").arg(comparative->ce_tau.max));
   // le_max_tau->setMinimumHeight(minHeight1);
   le_max_tau->setAlignment(AlignCenter|AlignVCenter);
   le_max_tau->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_tau, SIGNAL(textChanged(const QString &)), SLOT(update_max_tau(const QString &)));

   cb_store_reference_tau = new QCheckBox(this);
   cb_store_reference_tau->setMinimumHeight(minHeight3);
   cb_store_reference_tau->setText("");
   cb_store_reference_tau->setChecked(comparative->ce_tau.store_reference);
   cb_store_reference_tau->setEnabled(true);
   cb_store_reference_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_tau->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_tau, SIGNAL(clicked()), SLOT(set_store_reference_tau()));

   cb_store_diff_tau = new QCheckBox(this);
   cb_store_diff_tau->setMinimumHeight(minHeight3);
   cb_store_diff_tau->setText("");
   cb_store_diff_tau->setChecked(comparative->ce_tau.store_diff);
   cb_store_diff_tau->setEnabled(true);
   cb_store_diff_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_tau->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_tau, SIGNAL(clicked()), SLOT(set_store_diff_tau()));

   cb_store_abs_diff_tau = new QCheckBox(this);
   cb_store_abs_diff_tau->setMinimumHeight(minHeight3);
   cb_store_abs_diff_tau->setText("");
   cb_store_abs_diff_tau->setChecked(comparative->ce_tau.store_abs_diff);
   cb_store_abs_diff_tau->setEnabled(true);
   cb_store_abs_diff_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_tau->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_tau, SIGNAL(clicked()), SLOT(set_store_abs_diff_tau()));

   // ------------------- eta -------------------
   cb_active_eta = new QCheckBox(this);
   cb_active_eta->setMinimumHeight(minHeight3);
   cb_active_eta->setText(comparative->ce_eta.name);
   cb_active_eta->setEnabled(true);
   cb_active_eta->setChecked(comparative->ce_eta.active);
   cb_active_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_eta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_active_eta, SIGNAL(clicked()), SLOT(set_active_eta()));

   le_target_eta = new QLineEdit(this, "target_eta Line Edit");
   le_target_eta->setText(QString("%1").arg(comparative->ce_eta.target));
   // le_target_eta->setMinimumHeight(minHeight1);
   le_target_eta->setAlignment(AlignCenter|AlignVCenter);
   le_target_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_target_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_eta, SIGNAL(textChanged(const QString &)), SLOT(update_target_eta(const QString &)));

   le_rank_eta = new QLineEdit(this, "rank_eta Line Edit");
   le_rank_eta->setText(QString("%1").arg(comparative->ce_eta.rank));
   // le_rank_eta->setMinimumHeight(minHeight1);
   le_rank_eta->setAlignment(AlignCenter|AlignVCenter);
   le_rank_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_rank_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_eta, SIGNAL(textChanged(const QString &)), SLOT(update_rank_eta(const QString &)));

   cb_include_in_weight_eta = new QCheckBox(this);
   cb_include_in_weight_eta->setMinimumHeight(minHeight3);
   cb_include_in_weight_eta->setText("");
   cb_include_in_weight_eta->setChecked(comparative->ce_eta.include_in_weight);
   cb_include_in_weight_eta->setEnabled(true);
   cb_include_in_weight_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_eta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include_in_weight_eta, SIGNAL(clicked()), SLOT(set_include_in_weight_eta()));

   le_weight_eta = new QLineEdit(this, "weight_eta Line Edit");
   le_weight_eta->setText(QString("%1").arg(comparative->ce_eta.weight));
   // le_weight_eta->setMinimumHeight(minHeight1);
   le_weight_eta->setAlignment(AlignCenter|AlignVCenter);
   le_weight_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_weight_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_eta, SIGNAL(textChanged(const QString &)), SLOT(update_weight_eta(const QString &)));

   le_buckets_eta = new QLineEdit(this, "buckets_eta Line Edit");
   le_buckets_eta->setText(QString("%1").arg(comparative->ce_eta.buckets));
   // le_buckets_eta->setMinimumHeight(minHeight1);
   le_buckets_eta->setAlignment(AlignCenter|AlignVCenter);
   le_buckets_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_buckets_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_eta, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_eta(const QString &)));

   le_min_eta = new QLineEdit(this, "min_eta Line Edit");
   le_min_eta->setText(QString("%1").arg(comparative->ce_eta.min));
   // le_min_eta->setMinimumHeight(minHeight1);
   le_min_eta->setAlignment(AlignCenter|AlignVCenter);
   le_min_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_min_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_eta, SIGNAL(textChanged(const QString &)), SLOT(update_min_eta(const QString &)));

   le_max_eta = new QLineEdit(this, "max_eta Line Edit");
   le_max_eta->setText(QString("%1").arg(comparative->ce_eta.max));
   // le_max_eta->setMinimumHeight(minHeight1);
   le_max_eta->setAlignment(AlignCenter|AlignVCenter);
   le_max_eta->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   le_max_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_eta, SIGNAL(textChanged(const QString &)), SLOT(update_max_eta(const QString &)));

   cb_store_reference_eta = new QCheckBox(this);
   cb_store_reference_eta->setMinimumHeight(minHeight3);
   cb_store_reference_eta->setText("");
   cb_store_reference_eta->setChecked(comparative->ce_eta.store_reference);
   cb_store_reference_eta->setEnabled(true);
   cb_store_reference_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_eta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_reference_eta, SIGNAL(clicked()), SLOT(set_store_reference_eta()));

   cb_store_diff_eta = new QCheckBox(this);
   cb_store_diff_eta->setMinimumHeight(minHeight3);
   cb_store_diff_eta->setText("");
   cb_store_diff_eta->setChecked(comparative->ce_eta.store_diff);
   cb_store_diff_eta->setEnabled(true);
   cb_store_diff_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_eta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_diff_eta, SIGNAL(clicked()), SLOT(set_store_diff_eta()));

   cb_store_abs_diff_eta = new QCheckBox(this);
   cb_store_abs_diff_eta->setMinimumHeight(minHeight3);
   cb_store_abs_diff_eta->setText("");
   cb_store_abs_diff_eta->setChecked(comparative->ce_eta.store_abs_diff);
   cb_store_abs_diff_eta->setEnabled(true);
   cb_store_abs_diff_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_eta->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_store_abs_diff_eta, SIGNAL(clicked()), SLOT(set_store_abs_diff_eta()));

   pb_load_param = new QPushButton(tr("Load Parameters"), this);
   pb_load_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_param->setMinimumHeight(minHeight1);
   pb_load_param->setEnabled(true);
   pb_load_param->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_param, SIGNAL(clicked()), SLOT(load_param()));

   pb_reset_param = new QPushButton(tr("Reset Parameters"), this);
   pb_reset_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset_param->setMinimumHeight(minHeight1);
   pb_reset_param->setEnabled(true);
   pb_reset_param->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_reset_param, SIGNAL(clicked()), SLOT(reset_param()));

   pb_save_param = new QPushButton(tr("Save Parameters"), this);
   pb_save_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_param->setMinimumHeight(minHeight1);
   pb_save_param->setEnabled(true);
   pb_save_param->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_param, SIGNAL(clicked()), SLOT(save_param()));

   lbl_title_csv = new QLabel(tr("CSV Processing"), this);
   lbl_title_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title_csv->setAlignment(AlignCenter|AlignVCenter);
   lbl_title_csv->setMinimumHeight(minHeight1);
   lbl_title_csv->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_title_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_load_csv = new QPushButton(tr("Load"), this);
   pb_load_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_csv->setMinimumHeight(minHeight1);
   pb_load_csv->setEnabled(true);
   pb_load_csv->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_csv, SIGNAL(clicked()), SLOT(load_csv()));

   pb_process_csv = new QPushButton(tr("Process"), this);
   pb_process_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_process_csv->setMinimumHeight(minHeight1);
   pb_process_csv->setEnabled(false);
   pb_process_csv->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_process_csv, SIGNAL(clicked()), SLOT(process_csv()));

   pb_save_csv = new QPushButton(tr("Save"), this);
   pb_save_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_csv->setMinimumHeight(minHeight1);
   pb_save_csv->setEnabled(false);
   pb_save_csv->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_csv, SIGNAL(clicked()), SLOT(save_csv()));

   // lbl_loaded = new QLabel(tr("Loaded"), this);
   // lbl_loaded->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_loaded->setAlignment(AlignCenter|AlignVCenter);
   // lbl_loaded->setMinimumHeight(minHeight1);
   // lbl_loaded->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   // lbl_loaded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_loaded = new QListBox(this);
   lb_loaded->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_loaded->setMinimumHeight(minHeight1 * 6);
   // lb_loaded->setMinimumWidth(minWidth1);
   // lb_loaded->insertStringList(*qsl_loaded);
   lb_loaded->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_loaded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_loaded->setSelectionMode(QListBox::Multi);
   lb_loaded->setEnabled(true);
   connect(lb_loaded, SIGNAL(selectionChanged()), SLOT(update_loaded()));

   pb_loaded_select_all = new QPushButton(tr("Select all"), this);
   pb_loaded_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_select_all->setMinimumHeight(minHeight1);
   pb_loaded_select_all->setEnabled(false);
   pb_loaded_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_loaded_select_all, SIGNAL(clicked()), SLOT(loaded_select_all()));

   pb_loaded_remove = new QPushButton(tr("Remove"), this);
   pb_loaded_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_remove->setMinimumHeight(minHeight1);
   pb_loaded_remove->setEnabled(false);
   pb_loaded_remove->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_loaded_remove, SIGNAL(clicked()), SLOT(loaded_remove()));

   // lbl_selected = new QLabel(tr("Selected"), this);
   // lbl_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_selected->setAlignment(AlignCenter|AlignVCenter);
   // lbl_selected->setMinimumHeight(minHeight1);
   // lbl_selected->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   // lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_selected = new QListBox(this);
   lb_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lb_selected->setMinimumHeight(minHeight1 * 15);
   // lb_selected->setMinimumWidth(minWidth1);
   // lb_selected->insertStringList(*qsl_selected);
   lb_selected->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_selected->setSelectionMode(QListBox::Multi);
   lb_selected->setEnabled(true);
   connect(lb_selected, SIGNAL(selectionChanged()), SLOT(update_selected()));

   pb_selected_select_all = new QPushButton(tr("Select All"), this);
   pb_selected_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_select_all->setMinimumHeight(minHeight1);
   pb_selected_select_all->setEnabled(false);
   pb_selected_select_all->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_selected_select_all, SIGNAL(clicked()), SLOT(selected_select_all()));

   pb_selected_remove = new QPushButton(tr("Remove"), this);
   pb_selected_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_remove->setMinimumHeight(minHeight1);
   pb_selected_remove->setEnabled(false);
   pb_selected_remove->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_selected_remove, SIGNAL(clicked()), SLOT(selected_remove()));

   editor = new QTextEdit(this);
   editor->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   editor->setReadOnly(true);
   // editor->setMinimumWidth(300);
   // editor->setMinimumHeight(minHeight1 * 7);

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar(frame, "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   QPopupMenu * file = new QPopupMenu(editor);
   m->insertItem( tr("&File"), file );
   file->insertItem( tr("Font"),  this, SLOT(update_font()),    ALT+Key_F );
   file->insertItem( tr("Save"),  this, SLOT(save()),    ALT+Key_S );
   file->insertItem( tr("Print"), this, SLOT(print()),   ALT+Key_P );
   file->insertItem( tr("Clear Display"), this, SLOT(clear_display()),   ALT+Key_X );
   editor->setWordWrap (QTextEdit::WidgetWidth);

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
   int spacing = 2;
   int margin = 4;
   int j = 0;

   QGridLayout *background = new QGridLayout(this, 0, 0, margin, spacing);

   background->addMultiCellWidget(lbl_title_param, j, j, 0, 10);
   j++;

   background->addMultiCellWidget(lbl_active, j, j+2, 0, 0);
   background->addMultiCellWidget(lbl_target, j, j+2, 1, 1);
   background->addMultiCellWidget(lbl_sort, j, j, 2, 4);

   background->addMultiCellWidget(cb_rank, j+1, j+2, 2, 2);

   background->addMultiCellWidget(cb_weight_controls, j+1, j+1, 3, 4);
   background->addWidget(lbl_include_in_weight, j+2, 3);
   background->addWidget(lbl_weight, j+2, 4);

   background->addMultiCellWidget(lbl_ec, j, j, 5, 7);
   background->addMultiCellWidget(lbl_buckets, j+1, j+2, 5, 5);
   background->addMultiCellWidget(lbl_min, j+1, j+2, 6, 6);
   background->addMultiCellWidget(lbl_max, j+1, j+2, 7, 7);

   background->addMultiCellWidget(lbl_csv_controls, j, j, 8, 10);
   background->addMultiCellWidget(lbl_store_reference, j+1, j+2, 8, 8);
   background->addMultiCellWidget(lbl_store_diff, j+1, j+2, 9, 9);
   background->addMultiCellWidget(lbl_store_abs_diff, j+1, j+2, 10, 10);

   j += 3;

   background->addWidget(cb_active_s, j, 0);
   background->addWidget(le_target_s, j, 1);
   background->addWidget(le_rank_s, j, 2);
   background->addWidget(cb_include_in_weight_s, j, 3);
   background->addWidget(le_weight_s, j, 4);
   background->addWidget(le_buckets_s, j, 5);
   background->addWidget(le_min_s, j, 6);
   background->addWidget(le_max_s, j, 7);
   background->addWidget(cb_store_reference_s, j, 8);
   background->addWidget(cb_store_diff_s, j, 9);
   background->addWidget(cb_store_abs_diff_s, j, 10);
   j++;

   background->addWidget(cb_active_D, j, 0);
   background->addWidget(le_target_D, j, 1);
   background->addWidget(le_rank_D, j, 2);
   background->addWidget(cb_include_in_weight_D, j, 3);
   background->addWidget(le_weight_D, j, 4);
   background->addWidget(le_buckets_D, j, 5);
   background->addWidget(le_min_D, j, 6);
   background->addWidget(le_max_D, j, 7);
   background->addWidget(cb_store_reference_D, j, 8);
   background->addWidget(cb_store_diff_D, j, 9);
   background->addWidget(cb_store_abs_diff_D, j, 10);
   j++;

   background->addWidget(cb_active_sr, j, 0);
   background->addWidget(le_target_sr, j, 1);
   background->addWidget(le_rank_sr, j, 2);
   background->addWidget(cb_include_in_weight_sr, j, 3);
   background->addWidget(le_weight_sr, j, 4);
   background->addWidget(le_buckets_sr, j, 5);
   background->addWidget(le_min_sr, j, 6);
   background->addWidget(le_max_sr, j, 7);
   background->addWidget(cb_store_reference_sr, j, 8);
   background->addWidget(cb_store_diff_sr, j, 9);
   background->addWidget(cb_store_abs_diff_sr, j, 10);
   j++;

   background->addWidget(cb_active_fr, j, 0);
   background->addWidget(le_target_fr, j, 1);
   background->addWidget(le_rank_fr, j, 2);
   background->addWidget(cb_include_in_weight_fr, j, 3);
   background->addWidget(le_weight_fr, j, 4);
   background->addWidget(le_buckets_fr, j, 5);
   background->addWidget(le_min_fr, j, 6);
   background->addWidget(le_max_fr, j, 7);
   background->addWidget(cb_store_reference_fr, j, 8);
   background->addWidget(cb_store_diff_fr, j, 9);
   background->addWidget(cb_store_abs_diff_fr, j, 10);
   j++;

   background->addWidget(cb_active_rg, j, 0);
   background->addWidget(le_target_rg, j, 1);
   background->addWidget(le_rank_rg, j, 2);
   background->addWidget(cb_include_in_weight_rg, j, 3);
   background->addWidget(le_weight_rg, j, 4);
   background->addWidget(le_buckets_rg, j, 5);
   background->addWidget(le_min_rg, j, 6);
   background->addWidget(le_max_rg, j, 7);
   background->addWidget(cb_store_reference_rg, j, 8);
   background->addWidget(cb_store_diff_rg, j, 9);
   background->addWidget(cb_store_abs_diff_rg, j, 10);
   j++;

   background->addWidget(cb_active_tau, j, 0);
   background->addWidget(le_target_tau, j, 1);
   background->addWidget(le_rank_tau, j, 2);
   background->addWidget(cb_include_in_weight_tau, j, 3);
   background->addWidget(le_weight_tau, j, 4);
   background->addWidget(le_buckets_tau, j, 5);
   background->addWidget(le_min_tau, j, 6);
   background->addWidget(le_max_tau, j, 7);
   background->addWidget(cb_store_reference_tau, j, 8);
   background->addWidget(cb_store_diff_tau, j, 9);
   background->addWidget(cb_store_abs_diff_tau, j, 10);
   j++;

   background->addWidget(cb_active_eta, j, 0);
   background->addWidget(le_target_eta, j, 1);
   background->addWidget(le_rank_eta, j, 2);
   background->addWidget(cb_include_in_weight_eta, j, 3);
   background->addWidget(le_weight_eta, j, 4);
   background->addWidget(le_buckets_eta, j, 5);
   background->addWidget(le_min_eta, j, 6);
   background->addWidget(le_max_eta, j, 7);
   background->addWidget(cb_store_reference_eta, j, 8);
   background->addWidget(cb_store_diff_eta, j, 9);
   background->addWidget(cb_store_abs_diff_eta, j, 10);
   j++;

   QBoxLayout *hbl_param = new QHBoxLayout(0);
   hbl_param->addWidget(pb_load_param);
   hbl_param->addWidget(pb_reset_param);
   hbl_param->addWidget(pb_save_param);
   background->addMultiCellLayout(hbl_param, j, j, 0, 10);
   j++;

   background->addMultiCellWidget(lbl_title_csv, j, j, 0, 10);
   j++;

   QBoxLayout *hbl_csv = new QHBoxLayout(0);
   hbl_csv->addWidget(pb_load_csv);
   hbl_csv->addWidget(pb_process_csv);
   hbl_csv->addWidget(pb_save_csv);
   background->addMultiCellLayout(hbl_csv, j, j, 0, 10);
   j++;

   QGridLayout *gl_loaded_selected_editor = new QGridLayout(0, 0, 0, 0, 0);

   // gl_loaded_selected_editor->addWidget(lbl_loaded, 0, 0);
   gl_loaded_selected_editor->addMultiCellWidget(lb_loaded, 0, 1, 0, 0);
   QBoxLayout *hbl_loaded_buttons = new QHBoxLayout(0);
   hbl_loaded_buttons->addWidget(pb_loaded_select_all);
   hbl_loaded_buttons->addWidget(pb_loaded_remove);
   gl_loaded_selected_editor->addLayout(hbl_loaded_buttons, 2, 0);

   // gl_loaded_selected_editor->addWidget(lbl_selected, 0, 1);
   gl_loaded_selected_editor->addMultiCellWidget(lb_selected, 0, 1, 1, 1);
   QBoxLayout *hbl_selected_buttons = new QHBoxLayout(0);
   hbl_selected_buttons->addWidget(pb_selected_select_all);
   hbl_selected_buttons->addWidget(pb_selected_remove);
   gl_loaded_selected_editor->addLayout(hbl_selected_buttons, 2, 1);

   gl_loaded_selected_editor->addWidget(frame, 0, 2);
   gl_loaded_selected_editor->addMultiCellWidget(editor, 1, 1, 2, 2);
   
   background->addMultiCellLayout(gl_loaded_selected_editor, j, j, 0, 10);
   j++;

   QBoxLayout *hbl_bottom = new QHBoxLayout(0);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addWidget(pb_cancel);
   background->addMultiCellLayout(hbl_bottom, j, j, 0, 10);
   j++;

   update_enables();
}

void US_Hydrodyn_Comparative::cancel()
{
   close();
}

void US_Hydrodyn_Comparative::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_comparative.html");
}

void US_Hydrodyn_Comparative::closeEvent(QCloseEvent *e)
{
   *comparative_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Comparative::update_enables()
{
   cout << "update_enables\n";
   bool any_selected = any_loaded_selected();

   bool enable_s = 
      !any_selected || all_selected_csv_contain( comparative->ce_s );

   //   cout << QString(
   //                   "update enables:\n"
   //                   " enable_s = %1\n"
   //                   " any_selected = %1\n"
   //                   " all_selected_csv_contain = %1\n"
   //                   )
   //      .arg(enable_s ? "true" : "false")
   //      .arg(any_selected ? "true" : "false")
   //      .arg(all_selected_csv_contain( comparative->ce_s ) ? "true" : "false");

   cb_active_s->setEnabled(enable_s);
   le_target_s->setEnabled(enable_s && cb_active_s->isChecked());
   le_rank_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_weight_controls->isChecked());
   le_weight_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_weight_controls->isChecked());
   le_buckets_s->setEnabled(enable_s && cb_active_s->isChecked());
   le_min_s->setEnabled(enable_s && cb_active_s->isChecked() && comparative->ce_s.buckets);
   le_max_s->setEnabled(enable_s && cb_active_s->isChecked() && comparative->ce_s.buckets);
   cb_store_reference_s->setEnabled(enable_s && cb_active_s->isChecked());
   cb_store_diff_s->setEnabled(enable_s && cb_active_s->isChecked());
   cb_store_abs_diff_s->setEnabled(enable_s && cb_active_s->isChecked());

   bool enable_D = 
      !any_selected || all_selected_csv_contain( comparative->ce_D );

   cb_active_D->setEnabled(enable_D);
   le_target_D->setEnabled(enable_D && cb_active_D->isChecked());
   le_rank_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_weight_controls->isChecked());
   le_weight_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_weight_controls->isChecked());
   le_buckets_D->setEnabled(enable_D && cb_active_D->isChecked());
   le_min_D->setEnabled(enable_D && cb_active_D->isChecked() && comparative->ce_D.buckets);
   le_max_D->setEnabled(enable_D && cb_active_D->isChecked() && comparative->ce_D.buckets);
   cb_store_reference_D->setEnabled(enable_D && cb_active_D->isChecked());
   cb_store_diff_D->setEnabled(enable_D && cb_active_D->isChecked());
   cb_store_abs_diff_D->setEnabled(enable_D && cb_active_D->isChecked());

   bool enable_sr = 
      !any_selected || all_selected_csv_contain( comparative->ce_sr );

   cb_active_sr->setEnabled(enable_sr);
   le_target_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
   le_rank_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_weight_controls->isChecked());
   le_weight_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_weight_controls->isChecked());
   le_buckets_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
   le_min_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && comparative->ce_sr.buckets);
   le_max_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && comparative->ce_sr.buckets);
   cb_store_reference_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
   cb_store_diff_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
   cb_store_abs_diff_sr->setEnabled(enable_sr && cb_active_sr->isChecked());

   bool enable_fr = 
      !any_selected || all_selected_csv_contain( comparative->ce_fr );

   cb_active_fr->setEnabled(enable_fr);
   le_target_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
   le_rank_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_weight_controls->isChecked());
   le_weight_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_weight_controls->isChecked());
   le_buckets_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
   le_min_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && comparative->ce_fr.buckets);
   le_max_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && comparative->ce_fr.buckets);
   cb_store_reference_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
   cb_store_diff_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
   cb_store_abs_diff_fr->setEnabled(enable_fr && cb_active_fr->isChecked());

   bool enable_rg = 
      !any_selected || all_selected_csv_contain( comparative->ce_rg );

   cb_active_rg->setEnabled(enable_rg);
   le_target_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
   le_rank_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_weight_controls->isChecked());
   le_weight_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_weight_controls->isChecked());
   le_buckets_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
   le_min_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && comparative->ce_rg.buckets);
   le_max_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && comparative->ce_rg.buckets);
   cb_store_reference_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
   cb_store_diff_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
   cb_store_abs_diff_rg->setEnabled(enable_rg && cb_active_rg->isChecked());

   bool enable_tau = 
      !any_selected || all_selected_csv_contain( comparative->ce_tau );

   cb_active_tau->setEnabled(enable_tau);
   le_target_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
   le_rank_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_weight_controls->isChecked());
   le_weight_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_weight_controls->isChecked());
   le_buckets_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
   le_min_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && comparative->ce_tau.buckets);
   le_max_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && comparative->ce_tau.buckets);
   cb_store_reference_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
   cb_store_diff_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
   cb_store_abs_diff_tau->setEnabled(enable_tau && cb_active_tau->isChecked());

   bool enable_eta = 
      !any_selected || all_selected_csv_contain( comparative->ce_eta );

   cb_active_eta->setEnabled(enable_eta);
   le_target_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
   le_rank_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_rank->isChecked());
   cb_include_in_weight_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_weight_controls->isChecked());
   le_weight_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_weight_controls->isChecked());
   le_buckets_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
   le_min_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && comparative->ce_eta.buckets);
   le_max_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && comparative->ce_eta.buckets);
   cb_store_reference_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
   cb_store_diff_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
   cb_store_abs_diff_eta->setEnabled(enable_eta && cb_active_eta->isChecked());

   pb_process_csv->setEnabled(
                              any_selected_selected() &&
                              ( 
                               ( cb_active_s->isEnabled() && cb_active_s->isChecked() ) ||
                               ( cb_active_D->isEnabled() && cb_active_D->isChecked() ) ||
                               ( cb_active_sr->isEnabled() && cb_active_sr->isChecked() ) ||
                               ( cb_active_fr->isEnabled() && cb_active_fr->isChecked() ) ||
                               ( cb_active_rg->isEnabled() && cb_active_rg->isChecked() ) ||
                               ( cb_active_tau->isEnabled() && cb_active_tau->isChecked() ) ||
                               ( cb_active_eta->isEnabled() && cb_active_eta->isChecked() ) 
                               ) 
                              );

   pb_save_csv->setEnabled(any_selected);
}

void US_Hydrodyn_Comparative::update_lb_loaded_enables()
{
   cout << "update_lb_loaded_enables\n";
   pb_loaded_select_all->setEnabled(lb_loaded->count());
   bool any_selected = any_loaded_selected();
   pb_loaded_remove->setEnabled(any_selected);
}
      
void US_Hydrodyn_Comparative::update_lb_selected_enables()
{
   cout << "update_lb_selected_enables\n";
   pb_selected_select_all->setEnabled(lb_selected->count());
   bool any_selected = any_selected_selected();
   pb_selected_remove->setEnabled(any_selected);
   pb_process_csv->setEnabled(
                              any_selected &&
                              ( 
                               ( cb_active_s->isEnabled() && cb_active_s->isChecked() ) ||
                               ( cb_active_D->isEnabled() && cb_active_D->isChecked() ) ||
                               ( cb_active_sr->isEnabled() && cb_active_sr->isChecked() ) ||
                               ( cb_active_fr->isEnabled() && cb_active_fr->isChecked() ) ||
                               ( cb_active_rg->isEnabled() && cb_active_rg->isChecked() ) ||
                               ( cb_active_tau->isEnabled() && cb_active_tau->isChecked() ) ||
                               ( cb_active_eta->isEnabled() && cb_active_eta->isChecked() ) 
                               )
                              );
}

void US_Hydrodyn_Comparative::set_rank()
{
   comparative->rank = cb_rank->isChecked();
   comparative->weight_controls = !cb_rank->isChecked();
   cb_weight_controls->setChecked(comparative->weight_controls);
   update_enables();
}

void US_Hydrodyn_Comparative::set_weight_controls()
{
   comparative->weight_controls = cb_weight_controls->isChecked();
   comparative->rank = !cb_weight_controls->isChecked();
   cb_rank->setChecked(comparative->rank);
   update_enables();
}

void US_Hydrodyn_Comparative::set_active_s()
{
   comparative->ce_s.active = cb_active_s->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_s(const QString &val)
{
   comparative->ce_s.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_s(const QString &val)
{
   comparative->ce_s.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_s()
{
   comparative->ce_s.include_in_weight = cb_include_in_weight_s->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_s(const QString &val)
{
   comparative->ce_s.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_s(const QString &val)
{
   comparative->ce_s.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_s(const QString &val)
{
   comparative->ce_s.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_s(const QString &val)
{
   comparative->ce_s.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_s()
{
   comparative->ce_s.store_reference = cb_store_reference_s->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_s()
{
   comparative->ce_s.store_diff = cb_store_diff_s->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_s()
{
   comparative->ce_s.store_abs_diff = cb_store_abs_diff_s->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_active_D()
{
   comparative->ce_D.active = cb_active_D->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_D(const QString &val)
{
   comparative->ce_D.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_D(const QString &val)
{
   comparative->ce_D.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_D()
{
   comparative->ce_D.include_in_weight = cb_include_in_weight_D->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_D(const QString &val)
{
   comparative->ce_D.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_D(const QString &val)
{
   comparative->ce_D.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_D(const QString &val)
{
   comparative->ce_D.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_D(const QString &val)
{
   comparative->ce_D.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_D()
{
   comparative->ce_D.store_reference = cb_store_reference_D->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_D()
{
   comparative->ce_D.store_diff = cb_store_diff_D->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_D()
{
   comparative->ce_D.store_abs_diff = cb_store_abs_diff_D->isChecked();
   update_enables();
}


void US_Hydrodyn_Comparative::set_active_sr()
{
   comparative->ce_sr.active = cb_active_sr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_sr(const QString &val)
{
   comparative->ce_sr.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_sr(const QString &val)
{
   comparative->ce_sr.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_sr()
{
   comparative->ce_sr.include_in_weight = cb_include_in_weight_sr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_sr(const QString &val)
{
   comparative->ce_sr.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_sr(const QString &val)
{
   comparative->ce_sr.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_sr(const QString &val)
{
   comparative->ce_sr.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_sr(const QString &val)
{
   comparative->ce_sr.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_sr()
{
   comparative->ce_sr.store_reference = cb_store_reference_sr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_sr()
{
   comparative->ce_sr.store_diff = cb_store_diff_sr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_sr()
{
   comparative->ce_sr.store_abs_diff = cb_store_abs_diff_sr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_active_fr()
{
   comparative->ce_fr.active = cb_active_fr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_fr(const QString &val)
{
   comparative->ce_fr.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_fr(const QString &val)
{
   comparative->ce_fr.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_fr()
{
   comparative->ce_fr.include_in_weight = cb_include_in_weight_fr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_fr(const QString &val)
{
   comparative->ce_fr.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_fr(const QString &val)
{
   comparative->ce_fr.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_fr(const QString &val)
{
   comparative->ce_fr.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_fr(const QString &val)
{
   comparative->ce_fr.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_fr()
{
   comparative->ce_fr.store_reference = cb_store_reference_fr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_fr()
{
   comparative->ce_fr.store_diff = cb_store_diff_fr->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_fr()
{
   comparative->ce_fr.store_abs_diff = cb_store_abs_diff_fr->isChecked();
   update_enables();
}


void US_Hydrodyn_Comparative::set_active_rg()
{
   comparative->ce_rg.active = cb_active_rg->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_rg(const QString &val)
{
   comparative->ce_rg.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_rg(const QString &val)
{
   comparative->ce_rg.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_rg()
{
   comparative->ce_rg.include_in_weight = cb_include_in_weight_rg->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_rg(const QString &val)
{
   comparative->ce_rg.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_rg(const QString &val)
{
   comparative->ce_rg.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_rg(const QString &val)
{
   comparative->ce_rg.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_rg(const QString &val)
{
   comparative->ce_rg.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_rg()
{
   comparative->ce_rg.store_reference = cb_store_reference_rg->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_rg()
{
   comparative->ce_rg.store_diff = cb_store_diff_rg->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_rg()
{
   comparative->ce_rg.store_abs_diff = cb_store_abs_diff_rg->isChecked();
   update_enables();
}


void US_Hydrodyn_Comparative::set_active_tau()
{
   comparative->ce_tau.active = cb_active_tau->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_tau(const QString &val)
{
   comparative->ce_tau.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_tau(const QString &val)
{
   comparative->ce_tau.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_tau()
{
   comparative->ce_tau.include_in_weight = cb_include_in_weight_tau->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_tau(const QString &val)
{
   comparative->ce_tau.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_tau(const QString &val)
{
   comparative->ce_tau.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_tau(const QString &val)
{
   comparative->ce_tau.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_tau(const QString &val)
{
   comparative->ce_tau.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_tau()
{
   comparative->ce_tau.store_reference = cb_store_reference_tau->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_tau()
{
   comparative->ce_tau.store_diff = cb_store_diff_tau->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_tau()
{
   comparative->ce_tau.store_abs_diff = cb_store_abs_diff_tau->isChecked();
   update_enables();
}


void US_Hydrodyn_Comparative::set_active_eta()
{
   comparative->ce_eta.active = cb_active_eta->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_target_eta(const QString &val)
{
   comparative->ce_eta.target = val.toDouble();
}

void US_Hydrodyn_Comparative::update_rank_eta(const QString &val)
{
   comparative->ce_eta.rank = val.toInt();
}

void US_Hydrodyn_Comparative::set_include_in_weight_eta()
{
   comparative->ce_eta.include_in_weight = cb_include_in_weight_eta->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::update_weight_eta(const QString &val)
{
   comparative->ce_eta.weight = val.toDouble();
}

void US_Hydrodyn_Comparative::update_buckets_eta(const QString &val)
{
   comparative->ce_eta.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_eta(const QString &val)
{
   comparative->ce_eta.min = val.toDouble();
}

void US_Hydrodyn_Comparative::update_max_eta(const QString &val)
{
   comparative->ce_eta.max = val.toDouble();
}

void US_Hydrodyn_Comparative::set_store_reference_eta()
{
   comparative->ce_eta.store_reference = cb_store_reference_eta->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_diff_eta()
{
   comparative->ce_eta.store_diff = cb_store_diff_eta->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_store_abs_diff_eta()
{
   comparative->ce_eta.store_abs_diff = cb_store_abs_diff_eta->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::load_param()
{
   QString use_dir = 
      comparative->path_param.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" + "/" + "saxs" :
      comparative->path_param;

   QString fname = QFileDialog::getSaveFileName(
                                                use_dir,
                                                "*.smp",
                                                this,
                                                "save file dialog",
                                                tr("Choose a filename to save the parameters") );
   if ( fname.isEmpty() )
   {
      return;
   }

   comparative->path_param = QFileInfo(fname).dirPath(true);

   QFile f(fname);

   if ( !f.open( IO_ReadOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %! for reading! (permissions?)")).arg(fname) );
      return;
   }

   QString qs = "";
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine() + "\n";
   }
   f.close();

   comparative_info ci = deserialize_comparative_info( qs );
   if ( !serial_error.isEmpty() ) 
   {
      QMessageBox::warning( this, "UltraScan", serial_error );
      return;
   }
   ci.path_param = comparative->path_param;
   ci.path_csv = comparative->path_csv;
   *comparative = ci;
   refresh();
   editor->append(QString(tr("Loaded parameter file: %1\n")).arg(fname));
}

void US_Hydrodyn_Comparative::reset_param()
{
   comparative_info ci = empty_comparative_info();
   ci.path_param = comparative->path_param;
   ci.path_csv = comparative->path_csv;
   if ( !comparative_info_equals( *comparative, ci ) )
   {
      if ( 
          QMessageBox::question(
                                this,
                                tr("Reset Parameters"),
                                tr("Are you sure you want to reset the parameters?"),
                                tr("&Yes"), tr("&No"),
                                QString::null, 0, 1 ) 
          ) 
      {
         return;
      }
      *comparative = ci;
      refresh();
      editor->append(tr("Parameters reset\n"));
   }
}

void US_Hydrodyn_Comparative::save_param()
{
   //   cout << serialize_comparative_info(*comparative);
   QString use_dir = 
      comparative->path_param.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_param;

   QString fname = QFileDialog::getSaveFileName(
                                                use_dir,
                                                "*.smp",
                                                this,
                                                "save file dialog",
                                                tr("Choose a filename to save the parameters") );
   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".smp$",false)) )
   {
      fname += ".smp";
   }

   comparative->path_param = QFileInfo(fname).dirPath(true);
   
   if ( QFile::exists(fname) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
   }

   QFile f(fname);

   if ( !f.open( IO_WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %! for writing!")).arg(fname) );
      return;
   }
   QTextStream t( &f );
   t << serialize_comparative_info( *comparative );
   f.close();
   editor->append(QString(tr("Saved parameter file: %1\n")).arg(fname));
}

void US_Hydrodyn_Comparative::load_csv()
{
   QString use_dir = 
      comparative->path_csv.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_csv;

   QStringList filenames = QFileDialog::getOpenFileNames(
                                                         "csv files (*.csv)"
                                                         , use_dir
                                                         , this
                                                         , "open file dialog"
                                                         , "Open"
                                                         );

   if ( filenames.empty() )
   {
      return;
   }

   comparative->path_csv = QFileInfo(*filenames.at(0)).dirPath(true);

   for ( QStringList::iterator it = filenames.begin();
         it != filenames.end();
         it++ )
   {
      csv tmp_csv = csv_read(*it);
      if ( !csv_error.isEmpty() )
      {
         editor_msg("red", QString("%1: %1").arg(*it).arg(csv_error));
      } else {
         csvs[*it] = tmp_csv;
         editor->append(QString("loaded: %1\n").arg(*it));
         // cout << csv_info(tmp_csv);
         lb_loaded->insertItem(tmp_csv.name);
      }
   }
   update_lb_loaded_enables();
}

void US_Hydrodyn_Comparative::update_loaded()
{
   // clear lb_selected entries
   // maybe we want to be a bit more sophisticated with this
   //  i.e. doing a "delta" to only update the changes
   lb_selected->clear();
   
   // add selected loaded entries to selected
   for ( int i = 0; i < lb_loaded->numRows(); i++ )
   {
      if ( lb_loaded->isSelected(i) )
      {
         if ( csvs.count(lb_loaded->text(i)) )
         {
            lb_selected->insertStringList(csv_model_names(csvs[lb_loaded->text(i)]));
         } else {
            editor_msg("red", QString(tr("internal error: could not find %1 csv data")).arg(lb_loaded->text(i)));
         }
      }
   }
   update_lb_loaded_enables();
   update_lb_selected_enables();
   update_enables();
}

void US_Hydrodyn_Comparative::loaded_select_all()
{
   bool select_all = false;

   // are any unselected ?
   for ( int i = 0; i < lb_loaded->numRows(); i++ )
   {
      if ( !lb_loaded->isSelected(i) )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_loaded->numRows(); i++ )
   {
      lb_loaded->setSelected(i, select_all);
   }
   if ( select_all )
   {
      lb_loaded->setBottomItem(lb_loaded->numRows() - 1);
   }
}

void US_Hydrodyn_Comparative::loaded_remove()
{
   for ( int i = lb_loaded->numRows() - 1; i >= 0; i-- )
   {
      if ( lb_loaded->isSelected(i) )
      {
         map < QString, csv >::iterator it = csvs.find(lb_loaded->text(i));
         csvs.erase(it);
         lb_loaded->removeItem(i);
      }
   }
}

void US_Hydrodyn_Comparative::update_selected()
{
   update_lb_selected_enables();
   update_enables();
}

void US_Hydrodyn_Comparative::selected_select_all()
{
   bool select_all = false;

   // are any unselected ?
   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      if ( !lb_selected->isSelected(i) )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      lb_selected->setSelected(i, select_all);
   }
   if ( select_all )
   {
      lb_selected->setBottomItem(lb_selected->numRows() - 1);
   }
}

void US_Hydrodyn_Comparative::selected_remove()
{
   for ( int i = lb_selected->numRows() - 1; i >= 0; i-- )
   {
      if ( lb_selected->isSelected(i) )
      {
         lb_selected->removeItem(i);
      }
   }
}

void US_Hydrodyn_Comparative::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor( color );
   editor->append( msg );
   editor->setColor( save_color );
}

void US_Hydrodyn_Comparative::process_csv()
{
}

void US_Hydrodyn_Comparative::save_csv()
{
   if ( any_loaded_selected() && !one_loaded_selected() )
   {
      if ( 
          QMessageBox::question(
                                this,
                                tr("Merge CSV's"),
                                tr("Multiple CSV are selected and must be merged before saving\n"
                                   "Did you want to merge them?"),
                                tr("&Yes"), tr("&No"),
                                QString::null, 0, 1 ) 
          ) 
      {
         return;
      }
      csv_merge_loaded_selected();
      if ( any_loaded_selected() && !one_loaded_selected() )
      {
         editor_msg("red", "oops, csv_merge_loaded_selected didn't work as planned");
         return;
      }
   }

   QString use_dir = 
      comparative->path_csv.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_csv;

   QString sel_name = first_loaded_selected();
   if ( sel_name.isEmpty() )
   {
      editor_msg("red", tr("internal error: could not find csv for saving!"));
      return;
   }
   if ( !csvs.count(sel_name) )
   {
      editor_msg("red", QString(tr("internal error: could not find %1 csv data")).arg(sel_name));
      return;
   }

   QString use_name = sel_name.isEmpty() ? "*.csv" : sel_name;
   if ( !use_name.contains(QRegExp(".csv$",false)) )
   {
      use_name += ".csv";
   }

   if ( use_name == QFileInfo(use_name).fileName() )
   {
      // cout << QString(" use_name <%1> fi <%1>\n").arg(use_name).arg(QFileInfo(use_name).fileName());
      use_name = use_dir + QDir::separator() + use_name;
   }
   // cout << "use_name: " << use_name << "\n";

   QString fname = QFileDialog::getSaveFileName(
                                                use_name,
                                                "*.csv",
                                                this,
                                                "save file dialog",
                                                tr("Choose a filename to save the parameters") );
   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".csv$",false)) )
   {
      fname += ".csv";
   }

   comparative->path_csv = QFileInfo(fname).dirPath(true);

   csv_write( fname, csvs[sel_name] );
}

void US_Hydrodyn_Comparative::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName(QString::null, QString::null,this );
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

void US_Hydrodyn_Comparative::print()
{
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > metrics.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     metrics.width(), fm.lineSpacing(),
                                   ExpandTabs | DontClip,
                                   editor->text( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
}

void US_Hydrodyn_Comparative::clear_display()
{
   editor->clear();
   editor->append("\n\n");
}

void US_Hydrodyn_Comparative::update_font()
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

void US_Hydrodyn_Comparative::refresh()
{
   cb_rank->setChecked(comparative->rank);
   cb_weight_controls->setChecked(comparative->weight_controls);
   cb_active_s->setText(comparative->ce_s.name);
   cb_active_s->setChecked(comparative->ce_s.active);
   le_target_s->setText(QString("%1").arg(comparative->ce_s.target));
   le_rank_s->setText(QString("%1").arg(comparative->ce_s.rank));
   cb_include_in_weight_s->setChecked(comparative->ce_s.include_in_weight);
   le_weight_s->setText(QString("%1").arg(comparative->ce_s.weight));
   le_buckets_s->setText(QString("%1").arg(comparative->ce_s.buckets));
   le_min_s->setText(QString("%1").arg(comparative->ce_s.min));
   le_max_s->setText(QString("%1").arg(comparative->ce_s.max));
   cb_store_reference_s->setChecked(comparative->ce_s.store_reference);
   cb_store_diff_s->setChecked(comparative->ce_s.store_diff);
   cb_store_abs_diff_s->setChecked(comparative->ce_s.store_abs_diff);
   cb_active_D->setText(comparative->ce_D.name);
   cb_active_D->setChecked(comparative->ce_D.active);
   le_target_D->setText(QString("%1").arg(comparative->ce_D.target));
   le_rank_D->setText(QString("%1").arg(comparative->ce_D.rank));
   cb_include_in_weight_D->setChecked(comparative->ce_D.include_in_weight);
   le_weight_D->setText(QString("%1").arg(comparative->ce_D.weight));
   le_buckets_D->setText(QString("%1").arg(comparative->ce_D.buckets));
   le_min_D->setText(QString("%1").arg(comparative->ce_D.min));
   le_max_D->setText(QString("%1").arg(comparative->ce_D.max));
   cb_store_reference_D->setChecked(comparative->ce_D.store_reference);
   cb_store_diff_D->setChecked(comparative->ce_D.store_diff);
   cb_store_abs_diff_D->setChecked(comparative->ce_D.store_abs_diff);
   cb_active_sr->setText(comparative->ce_sr.name);
   cb_active_sr->setChecked(comparative->ce_sr.active);
   le_target_sr->setText(QString("%1").arg(comparative->ce_sr.target));
   le_rank_sr->setText(QString("%1").arg(comparative->ce_sr.rank));
   cb_include_in_weight_sr->setChecked(comparative->ce_sr.include_in_weight);
   le_weight_sr->setText(QString("%1").arg(comparative->ce_sr.weight));
   le_buckets_sr->setText(QString("%1").arg(comparative->ce_sr.buckets));
   le_min_sr->setText(QString("%1").arg(comparative->ce_sr.min));
   le_max_sr->setText(QString("%1").arg(comparative->ce_sr.max));
   cb_store_reference_sr->setChecked(comparative->ce_sr.store_reference);
   cb_store_diff_sr->setChecked(comparative->ce_sr.store_diff);
   cb_store_abs_diff_sr->setChecked(comparative->ce_sr.store_abs_diff);
   cb_active_fr->setText(comparative->ce_fr.name);
   cb_active_fr->setChecked(comparative->ce_fr.active);
   le_target_fr->setText(QString("%1").arg(comparative->ce_fr.target));
   le_rank_fr->setText(QString("%1").arg(comparative->ce_fr.rank));
   cb_include_in_weight_fr->setChecked(comparative->ce_fr.include_in_weight);
   le_weight_fr->setText(QString("%1").arg(comparative->ce_fr.weight));
   le_buckets_fr->setText(QString("%1").arg(comparative->ce_fr.buckets));
   le_min_fr->setText(QString("%1").arg(comparative->ce_fr.min));
   le_max_fr->setText(QString("%1").arg(comparative->ce_fr.max));
   cb_store_reference_fr->setChecked(comparative->ce_fr.store_reference);
   cb_store_diff_fr->setChecked(comparative->ce_fr.store_diff);
   cb_store_abs_diff_fr->setChecked(comparative->ce_fr.store_abs_diff);
   cb_active_rg->setText(comparative->ce_rg.name);
   cb_active_rg->setChecked(comparative->ce_rg.active);
   le_target_rg->setText(QString("%1").arg(comparative->ce_rg.target));
   le_rank_rg->setText(QString("%1").arg(comparative->ce_rg.rank));
   cb_include_in_weight_rg->setChecked(comparative->ce_rg.include_in_weight);
   le_weight_rg->setText(QString("%1").arg(comparative->ce_rg.weight));
   le_buckets_rg->setText(QString("%1").arg(comparative->ce_rg.buckets));
   le_min_rg->setText(QString("%1").arg(comparative->ce_rg.min));
   le_max_rg->setText(QString("%1").arg(comparative->ce_rg.max));
   cb_store_reference_rg->setChecked(comparative->ce_rg.store_reference);
   cb_store_diff_rg->setChecked(comparative->ce_rg.store_diff);
   cb_store_abs_diff_rg->setChecked(comparative->ce_rg.store_abs_diff);
   cb_active_tau->setText(comparative->ce_tau.name);
   cb_active_tau->setChecked(comparative->ce_tau.active);
   le_target_tau->setText(QString("%1").arg(comparative->ce_tau.target));
   le_rank_tau->setText(QString("%1").arg(comparative->ce_tau.rank));
   cb_include_in_weight_tau->setChecked(comparative->ce_tau.include_in_weight);
   le_weight_tau->setText(QString("%1").arg(comparative->ce_tau.weight));
   le_buckets_tau->setText(QString("%1").arg(comparative->ce_tau.buckets));
   le_min_tau->setText(QString("%1").arg(comparative->ce_tau.min));
   le_max_tau->setText(QString("%1").arg(comparative->ce_tau.max));
   cb_store_reference_tau->setChecked(comparative->ce_tau.store_reference);
   cb_store_diff_tau->setChecked(comparative->ce_tau.store_diff);
   cb_store_abs_diff_tau->setChecked(comparative->ce_tau.store_abs_diff);
   cb_active_eta->setText(comparative->ce_eta.name);
   cb_active_eta->setChecked(comparative->ce_eta.active);
   le_target_eta->setText(QString("%1").arg(comparative->ce_eta.target));
   le_rank_eta->setText(QString("%1").arg(comparative->ce_eta.rank));
   cb_include_in_weight_eta->setChecked(comparative->ce_eta.include_in_weight);
   le_weight_eta->setText(QString("%1").arg(comparative->ce_eta.weight));
   le_buckets_eta->setText(QString("%1").arg(comparative->ce_eta.buckets));
   le_min_eta->setText(QString("%1").arg(comparative->ce_eta.min));
   le_max_eta->setText(QString("%1").arg(comparative->ce_eta.max));
   cb_store_reference_eta->setChecked(comparative->ce_eta.store_reference);
   cb_store_diff_eta->setChecked(comparative->ce_eta.store_diff);
   cb_store_abs_diff_eta->setChecked(comparative->ce_eta.store_abs_diff);
   update_enables();
}

csv US_Hydrodyn_Comparative::csv_read( QString filename )
{
   csv csv1;

   csv_error = "";
   if ( filename.isEmpty() )
   {
      csv_error = tr("csv read called with empty filename");
      return csv1;
   }

   QFile f(filename);

   if ( !f.exists() )
   {
      csv_error = QString(tr("File %1 does not exist")).arg(f.name());
      return csv1;
   }

   if ( !f.open(IO_ReadOnly) )
   {
      csv_error = QString(tr("Can not open file %1.  Check permissions")).arg(f.name());
      return csv1;
   }

   QTextStream ts( &f );

   QStringList qsl;

   while( !ts.atEnd() )
   {
      qsl << ts.readLine();
   }

   f.close();

   csv1.name = filename;

   int i = 0;
   QStringList qsl_h = QStringList::split("\",\"",*qsl.at(0), true);
   for ( QStringList::iterator it = qsl_h.begin();
         it != qsl_h.end();
         it++ )
   {
      QString qs = *it;
      qs.replace("\"","");
      csv1.header_map[qs] = i++;
      csv1.header.push_back(qs);
   }

   for ( QStringList::iterator it = qsl.at(1);
         it != qsl.end();
         it++ )
   {
      QStringList qsl_d = QStringList::split(",",*it, true);
      vector < QString > vqs;
      vector < double > vd;
      for ( QStringList::iterator it2 = qsl_d.begin();
            it2 != qsl_d.end();
            it2++ )
      {
         vqs.push_back(*it2);
         vd.push_back((*it2).toDouble());
      }
      csv1.data.push_back(vqs);
      csv1.num_data.push_back(vd);
   }

   if ( !csv1.header_map.count("Model name") )
   {
      csv_error = tr("no \"Model name\" header found in csv");
      return csv1;
   }

   if ( !csv1.data.size() )
   {
      csv_error = tr("no data lines found in csv");
      return csv1;
   }

   QString qs_prepend = QFileInfo(csv1.name).baseName(true) + ": ";
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      csv1.prepended_names.push_back(qs_prepend + csv1.data[i][csv1.header_map["Model name"]]);
   }
      
   return csv1;
}

QString US_Hydrodyn_Comparative::csv_info( csv &csv1 )
{
   QString qs = 
      QString(
              "csv_info for %1:\n"
              " headers_map: %1\n"
              )
      .arg(csv1.name)
      .arg(csv1.header_map.size())
      ;

   for ( map < QString, int >::iterator it = csv1.header_map.begin();
         it != csv1.header_map.end();
         it++ )
   {
      qs += "  " + QString("%1 %2\n").arg(it->first).arg(it->second);
   }

   qs +=  
      QString(" header: %1\n")
      .arg(csv1.header.size());
   
   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      qs += "  " + QString("%1 %1\n").arg(i).arg(csv1.header[i]);
   }
   qs += " prepended names:\n";

   for ( unsigned int i = 0; i < csv1.prepended_names.size(); i++ )
   {
      qs += "  " + csv1.prepended_names[i] + "\n";
   }

   qs +=  
      QString(" data rows: %1\n")
      .arg(csv1.data.size());

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      qs += QString("  row %1 data columns %1  num_data columns %1\n")
         .arg(i).arg(csv1.data[i].size()).arg(csv1.num_data[i].size());
   }

   return qs;
}
      
csv US_Hydrodyn_Comparative::csv_merge( csv &csv1, csv &csv2 )
{
   csv_error = "";

   csv csv_merge = csv1;
   // cout << csv_info(csv1);   
   // cout << csv_info(csv2);   

   if ( !csv1.data.size() || !csv2.data.size() )
   {
      csv_error = tr("internal error: csv_merge called with zero data csv");
      return csv_merge;
   }

   if ( !csv_merge.name.contains("+") )
   {
      csv_merge.name = QFileInfo(csv_merge.name).baseName(true);
   }
   csv_merge.name += "+" + QFileInfo(csv2.name).baseName(true);


   // save number of empties that will be needed for csv2's columns that are not present in csv1

   unsigned int empties = csv1.data.size();

   // 1st append all csv2's columns that are present in csv1 as new rows

   // for each row of csv2:
   for ( unsigned int i = 0; i < csv2.data.size(); i++ )
   {
      vector < QString > new_data(csv_merge.header.size());
      vector < double >  new_num_data(csv_merge.header.size());
      // not sure if a vector initializes to zeros/blanks
      for ( unsigned int j = 0; j < new_data.size(); j++ )
      {
         new_data[j] = "";
         new_num_data[j] = 0e0;
      }

      // now set the values for existing columns
      for ( unsigned int k = 0; k < csv_merge.header.size(); k++ )
      {
         if ( csv2.header_map.count(csv_merge.header[k]) )
         {
            unsigned int csv2_col = csv2.header_map[csv_merge.header[k]];
            new_data[k] = csv2.data[i][csv2_col];
            new_num_data[k] = csv2.num_data[i][csv2_col];
         }
      }
      csv_merge.data.push_back(new_data);
      csv_merge.num_data.push_back(new_num_data);
   }

   // now append all csv2's columns that are not present in csv1

   for ( unsigned int i = 0; i < csv2.header.size(); i++ )
   {
      if ( !csv_merge.header_map.count(csv2.header[i]) )
      {
         csv_merge.header_map[csv2.header[i]] = csv_merge.header.size();
         csv_merge.header.push_back(csv2.header[i]);
         
         for ( unsigned int j = 0; j < empties; j++ )
         {
            csv_merge.data[j].push_back("");
            csv_merge.num_data[j].push_back(0e0);
         }
         
         for ( unsigned int j = 0; j < csv2.data.size(); j++ )
         {
            csv_merge.data[j + empties].push_back(csv2.data[j][i]);
            csv_merge.num_data[j + empties].push_back(csv2.num_data[j][i]);
         }
      }
   }

   // now add the prepended names

   for ( unsigned int i = 0; i < csv2.prepended_names.size(); i++ )
   {
      csv_merge.prepended_names.push_back(csv2.prepended_names[i]);
   }
   
   return csv_merge;
}

QStringList US_Hydrodyn_Comparative::csv_model_names ( csv &csv1 )
{
   QStringList qsl;
   for ( unsigned int i = 0; i < csv1.prepended_names.size(); i++ )
   {
      qsl << csv1.prepended_names[i];
   }
   return qsl;
}

bool US_Hydrodyn_Comparative::csv_contains( comparative_entry ce, csv &csv1 )
{
   return csv1.header_map.count(ce.name) > 0;
}

bool US_Hydrodyn_Comparative::all_selected_csv_contain( comparative_entry ce )
{
   bool all_contain = true;

   for ( unsigned int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( lb_loaded->isSelected(i) )
      {
         if ( csvs.count(lb_loaded->text(i)) )
         {
            if ( !csvs[lb_loaded->text(i)].header_map.count(ce.name) )
            {
               all_contain = false;
               break;
            }
         }
      }
   }
   return all_contain;
}

bool US_Hydrodyn_Comparative::any_loaded_selected()
{
   bool any_selected = false;
   for ( unsigned int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( lb_loaded->isSelected(i) )
      {
         any_selected = true;
         break;
      }
   }
   return any_selected;
}

bool US_Hydrodyn_Comparative::one_loaded_selected()
{
   int no_selected = 0;
   for ( unsigned int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( lb_loaded->isSelected(i) )
      {
         no_selected++;
         if ( no_selected > 1 )
         {
            break;
         }
      }
   }
   return no_selected == 1;
}

QString US_Hydrodyn_Comparative::first_loaded_selected()
{
   QString qs;
   for ( unsigned int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( lb_loaded->isSelected(i) )
      {
         qs = lb_loaded->text(i);
         break;
      }
   }
   return qs;
}

bool US_Hydrodyn_Comparative::any_selected_selected()
{
   bool any_selected = false;
   for ( unsigned int i = 0; i < lb_selected->count(); i++ )
   {
      if ( lb_selected->isSelected(i) )
      {
         any_selected = true;
         break;
      }
   }
   return any_selected;
}


csv US_Hydrodyn_Comparative::csv_process( csv &csv1 )
{
   cout << "csv_process\n";
   return csv1;
}

void US_Hydrodyn_Comparative::csv_write( QString filename, csv &csv1 )
{
   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(filename);
   }

   QFile f(filename);

   if ( !f.open( IO_WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(tr("Could not open %! for writing!")).arg(filename) );
      return;
   }
   QTextStream t( &f );
   QString qs;
   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      qs += QString("%1\"%1\"").arg(i ? "," : "").arg(csv1.header[i]);
   }
   t << qs << endl;
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         qs += QString("%1%1").arg(j ? "," : "").arg(csv1.data[i][j]);
      }
      t << qs << endl;
   }
   f.close();
   editor->append(QString(tr("Saved csv file: %1\n")).arg(filename));
}

void US_Hydrodyn_Comparative::csv_merge_loaded_selected() 
{
   if ( !any_loaded_selected() || one_loaded_selected() )
   {
      // nothing to do here
      return;
   }

   csv csv_merged = csvs[first_loaded_selected()];
   bool skip_first = true;
   for ( unsigned int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( !skip_first && lb_loaded->isSelected(i) ) 
      {
         if ( !csvs.count(lb_loaded->text(i)) )
         {
            editor_msg("red", QString(tr("internal error: could not find %1 csv data")).arg(lb_loaded->text(i)));
            return;
         }
         csv_merged = csv_merge(csv_merged, csvs[lb_loaded->text(i)]);
      }

      if ( skip_first && lb_loaded->isSelected(i) ) 
      {
         skip_first = false;
      }
   }
   for ( int i = 0; i < lb_loaded->numRows(); i++ )
   {
      lb_loaded->setSelected(i, false);
   }

   csvs[csv_merged.name] = csv_merged;
   lb_loaded->insertItem(csv_merged.name);
   lb_loaded->setSelected(lb_loaded->numRows() - 1, true);
   lb_loaded->setBottomItem(lb_loaded->numRows() - 1);
}
