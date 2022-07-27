#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "qregexp.h"
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

#define PREMERGE_LIST_LIMIT 20

// todo: the comparative entries should be map indexed for cleaner code

class sortable_vector_double {
   // sorts the vector in column order
public:
   vector < double > vd;
   unsigned int      index;
   bool operator < (const sortable_vector_double& objIn) const
   {
      unsigned int i;
      for ( i = 0; i < vd.size(); i++ )
      {
         if ( vd[i] < objIn.vd[i] )
         {
            return true;
            break;
         }
         if ( vd[i] > objIn.vd[i] )
         {
            return false;
            break;
         }
      }
      // if here, they are all equal
      return false;
   }
};

class sortable_unsigned_int {
public:
   unsigned int      ui;
   unsigned int      index;
   bool operator < (const sortable_unsigned_int& objIn) const
   {
      return ( ui < objIn.ui );
   }
};

void US_Hydrodyn_Comparative::csv_sort( csv &csv1, vector < unsigned int > cols )
{
   // build a sortable vector double containing the data of the relevant columns
   // cout << "csv_sort\ncols:";
   // for ( unsigned int i = 0; i < cols.size(); i++ )
   // {
   // cout << cols[i] << " ";
   // }
   cout << endl;

   list < sortable_vector_double > lsvd;

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      sortable_vector_double svd;
      svd.index = i;
      for ( unsigned int j = 0; j < cols.size(); j++ )
      {
         svd.vd.push_back( csv1.num_data[i][ cols[ j ] ] );
      }
      lsvd.push_back(svd);
   }
   lsvd.sort();
   csv csv_new = csv1;
   unsigned int i = 0;
   for ( list < sortable_vector_double >::iterator it = lsvd.begin();
         it != lsvd.end();
         it++, i++ )
   {
      csv_new.data[ i ] = csv1.data[ it->index ];
      csv_new.num_data[ i ] = csv1.num_data[ it->index ];
      csv_new.prepended_names[ i ] = csv1.prepended_names[ it->index ];
   }
   csv1 = csv_new;
}

US_Hydrodyn_Comparative::US_Hydrodyn_Comparative(
                                                 comparative_info *comparative,      
                                                 void *us_hydrodyn, 
                                                 bool *comparative_widget,  // no comparative widget implies non-gui
                                                 QWidget *p,
                                                 const char *
                                                 ) : QFrame( p )
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
   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("Model classifier"));
   updates_enabled = true;
   loaded_csv_names.clear( );
   loaded_csv_row_prepended_names.clear( );
   build_ce_names_map();
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

void US_Hydrodyn_Comparative::build_ce_names_map()
{
   ce_names.clear( );
   ce_map.clear( );
   ce_names.push_back( comparative->ce_s.name );
   ce_names.push_back( comparative->ce_D.name );
   ce_names.push_back( comparative->ce_sr.name );
   ce_names.push_back( comparative->ce_fr.name );
   ce_names.push_back( comparative->ce_rg.name );
   ce_names.push_back( comparative->ce_tau.name );
   ce_names.push_back( comparative->ce_eta.name );
   ce_map[ comparative->ce_s.name ] = &comparative->ce_s;
   ce_map[ comparative->ce_D.name ] = &comparative->ce_D;
   ce_map[ comparative->ce_sr.name ] = &comparative->ce_sr;
   ce_map[ comparative->ce_fr.name ] = &comparative->ce_fr;
   ce_map[ comparative->ce_rg.name ] = &comparative->ce_rg;
   ce_map[ comparative->ce_tau.name ] = &comparative->ce_tau;
   ce_map[ comparative->ce_eta.name ] = &comparative->ce_eta;
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
   ci.ce_D = empty_comparative_entry("Translational diffusion coefficient D [cm^2/sec]");
   ci.ce_sr = empty_comparative_entry("Stokes radius [nm]");
   ci.ce_fr = empty_comparative_entry("Frictional ratio");
   ci.ce_rg = empty_comparative_entry("Radius of gyration [nm] (from bead model)");
   ci.ce_tau = empty_comparative_entry("Relaxation Time, tau(h) [ns]");
   ci.ce_eta = empty_comparative_entry("Intrinsic viscosity [cm^3/g]");

   ci.by_pct = true;
   ci.rank = true;
   ci.weight_controls = false;
   ci.by_ec = false;

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
      QString(
#if QT_VERSION < 0x040000
              "%1|%1|%1|%1|%1|%1|%1|%1|%1|%1|%1|%1\n"
#else
              "%1|%2|%3|%4|%5|%6|%7|%8|%9|%10|%11|%12\n"
#endif

              )
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
   QStringList qsl = (qs).split( "|" , Qt::SkipEmptyParts );
   comparative_entry ce;
   if ( qsl.size() < 12 )
   {
      serial_error = "Error: invalid parameter file ";
   }
   int pos = 0;
   ce.name = qsl[pos++];
   ce.active = (bool)qsl[pos++].toInt();
   ce.target = qsl[pos++].toDouble();
   ce.rank = qsl[pos++].toInt();
   ce.include_in_weight = (bool)qsl[pos++].toInt();
   ce.weight = qsl[pos++].toDouble();
   ce.buckets = qsl[pos++].toInt();
   ce.min = qsl[pos++].toDouble();
   ce.max = qsl[pos++].toDouble();
   ce.store_reference = (bool)qsl[pos++].toInt();
   ce.store_diff = (bool)qsl[pos++].toInt();
   ce.store_abs_diff = (bool)qsl[pos++].toInt();
   return ce;
}   

QString US_Hydrodyn_Comparative::serialize_comparative_info( comparative_info ci )
{
   QString qs = QString("%1|%2|%3|%4\n")
      .arg(ci.by_pct)
      .arg(ci.rank)
      .arg(ci.weight_controls)
      .arg(ci.by_ec)
      ;
   qs += serialize_comparative_entry( ci.ce_s );
   qs += serialize_comparative_entry( ci.ce_D );
   qs += serialize_comparative_entry( ci.ce_sr );
   qs += serialize_comparative_entry( ci.ce_fr );
   qs += serialize_comparative_entry( ci.ce_rg );
   qs += serialize_comparative_entry( ci.ce_tau );
   qs += serialize_comparative_entry( ci.ce_eta );

   return qs;
}   

comparative_info US_Hydrodyn_Comparative::deserialize_comparative_info( QString qs )
{
   comparative_info ci = US_Hydrodyn_Comparative::empty_comparative_info();
   serial_error = "";
   QStringList qsl = (qs).split( "\n" , Qt::SkipEmptyParts );
   if ( qsl.size() < 8 )
   {
      cout << QString("qsl size %1 < 8 qs:<%2>\n").arg(qs,qsl.size());
      serial_error = us_tr("Error: invalid parameter file (too few lines)");
      return ci;
   }
   QStringList qsl0 = (qsl[0]).split( "|" , Qt::SkipEmptyParts );
   if ( qsl0.size() < 4 )
   {
      serial_error = us_tr("Error: invalid parameter file (line 1 too short)");
      return ci;
   }

   ci.by_pct = (bool)qsl0[0].toInt();
   ci.rank = (bool)qsl0[1].toInt();
   ci.weight_controls = (bool)qsl0[2].toInt();
   ci.by_ec = (bool)qsl0[3].toInt();

   int pos = 1;
   ci.ce_s = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_D = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_sr = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_fr = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_rg = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_tau = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   ci.ce_eta = deserialize_comparative_entry(qsl[pos++]);
   if ( !serial_error.isEmpty() )
   {
      serial_error += QString(us_tr(" line %1")).arg(pos);
      return ci;
   }
   return ci;
}   

void US_Hydrodyn_Comparative::setupGUI()
{
   int minHeight0 = 0;
   int minHeight1 = 30;
   int minHeight2b = 52;
   int minHeight2c = 52;
   int minHeight3 = 22;
   int minHeightpb = 26;

   QPalette cg_modes = USglobal->global_colors.cg_label;
   cg_modes.setColor(QPalette::Shadow, Qt::gray);
   cg_modes.setColor(QPalette::Dark, Qt::gray);
   cg_modes.setColor(QPalette::Light, Qt::white);
   cg_modes.setColor(QPalette::Midlight, Qt::gray);

   QFont qf_modes = QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold );
   QPalette qp_modes = cg_modes;

   lbl_title_param = new QLabel(us_tr("Select parameters"), this);
   lbl_title_param->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title_param->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title_param->setMinimumHeight(minHeight1);
   lbl_title_param->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title_param );
   lbl_title_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_active = new QLabel(us_tr("Select to enable variable comparison"), this);
   lbl_active->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_active->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_active->setMinimumHeight(minHeight1);
   lbl_active->setPalette( PALET_LABEL );
   AUTFBACK( lbl_active );
   lbl_active->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_target = new QLabel(us_tr("Experimental\nvalue"), this);
   lbl_target->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_target->setMinimumHeight(minHeight1);
   lbl_target->setPalette( PALET_LABEL );
   AUTFBACK( lbl_target );
   lbl_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_sort = new QLabel(us_tr("Sort results"), this);
   lbl_sort->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_sort->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_sort->setMinimumHeight(minHeight3);
   lbl_sort->setPalette( PALET_LABEL );
   AUTFBACK( lbl_sort );
   lbl_sort->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_by_pct = new QCheckBox(this);
   cb_by_pct->setText(us_tr("Using percentage difference"));
   cb_by_pct->setEnabled(true);
   cb_by_pct->setMinimumHeight(minHeight3);
   cb_by_pct->setChecked(comparative->by_pct);
   cb_by_pct->setFont(qf_modes);
   cb_by_pct->setPalette( PALET_NORMAL );
   AUTFBACK( cb_by_pct );
   connect(cb_by_pct, SIGNAL(clicked()), SLOT(set_by_pct()));

   cb_rank = new QCheckBox(this);
   cb_rank->setText(QString(us_tr("By ranked\n%1absolute\ndifference")).arg(comparative->by_pct ? "% " : "" ));
   cb_rank->setEnabled(true);
   cb_rank->setMinimumHeight(minHeight2b);
   cb_rank->setChecked(comparative->rank);
   cb_rank->setFont(qf_modes);
   cb_rank->setPalette( PALET_NORMAL );
   AUTFBACK( cb_rank );
   connect(cb_rank, SIGNAL(clicked()), SLOT(set_rank()));

   lbl_rank = new QLabel(us_tr("Rank"), this);
   lbl_rank->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_rank->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_rank->setMinimumHeight(minHeight1);
   lbl_rank->setPalette( PALET_LABEL );
   AUTFBACK( lbl_rank );
   lbl_rank->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_weight_controls = new QCheckBox(this);
   cb_weight_controls->setMinimumHeight(minHeight2b);
   cb_weight_controls->setText(QString(us_tr("By weighted sum\nof %1absolute\ndifferences")).arg(comparative->by_pct ? "% " : "" ));
   cb_weight_controls->setEnabled(true);
   cb_weight_controls->setChecked(comparative->weight_controls);
   cb_weight_controls->setFont(qf_modes);
   cb_weight_controls->setPalette( PALET_NORMAL );
   AUTFBACK( cb_weight_controls );
   connect(cb_weight_controls, SIGNAL(clicked()), SLOT(set_weight_controls()));

   lbl_include_in_weight = new QLabel(us_tr("Include"), this);
   lbl_include_in_weight->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_include_in_weight->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_include_in_weight->setMinimumHeight(minHeight0);
   lbl_include_in_weight->setPalette( PALET_LABEL );
   AUTFBACK( lbl_include_in_weight );
   lbl_include_in_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_weight = new QLabel(us_tr("Weight"), this);
   lbl_weight->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_weight->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_weight->setMinimumHeight(minHeight0);
   lbl_weight->setPalette( PALET_LABEL );
   AUTFBACK( lbl_weight );
   lbl_weight->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_ec = new QLabel(us_tr("Equivalence class controls"), this);
   lbl_ec->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_ec->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_ec->setMinimumHeight(minHeight3);
   lbl_ec->setPalette( PALET_LABEL );
   AUTFBACK( lbl_ec );
   lbl_ec->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   cb_by_ec = new QCheckBox(this);
   cb_by_ec->setMinimumHeight(minHeight2b);
   cb_by_ec->setText(us_tr("By equivalence class rank"));
   cb_by_ec->setEnabled(true);
   cb_by_ec->setChecked(comparative->by_ec);
   cb_by_ec->setFont(qf_modes);
   cb_by_ec->setPalette( PALET_NORMAL );
   AUTFBACK( cb_by_ec );
   connect(cb_by_ec, SIGNAL(clicked()), SLOT(set_by_ec()));

   lbl_buckets = new QLabel(us_tr("Number of\npartitions"), this);
   lbl_buckets->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_buckets->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_buckets->setMinimumHeight(minHeight2c);
   lbl_buckets->setPalette( PALET_LABEL );
   AUTFBACK( lbl_buckets );
   lbl_buckets->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_min = new QLabel(us_tr("Minimum\nmodel\nvalue"), this);
   lbl_min->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_min->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_min->setMinimumHeight(minHeight2c);
   lbl_min->setPalette( PALET_LABEL );
   AUTFBACK( lbl_min );
   lbl_min->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_max = new QLabel(us_tr("Maximum\nmodel\nvalue"), this);
   lbl_max->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_max->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_max->setMinimumHeight(minHeight2c);
   lbl_max->setPalette( PALET_LABEL );
   AUTFBACK( lbl_max );
   lbl_max->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_csv_controls = new QLabel(us_tr("Add columns to results"), this);
   lbl_csv_controls->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_csv_controls->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_csv_controls->setMinimumHeight(minHeight1);
   lbl_csv_controls->setPalette( PALET_LABEL );
   AUTFBACK( lbl_csv_controls );
   lbl_csv_controls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_reference = new QLabel(us_tr("Experimental\nvalue"), this);
   lbl_store_reference->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_reference->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_store_reference->setMinimumHeight(minHeight1);
   lbl_store_reference->setPalette( PALET_LABEL );
   AUTFBACK( lbl_store_reference );
   lbl_store_reference->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_diff = new QLabel((comparative->by_pct ? "%\n" : "" ) + us_tr("Difference"), this);
   lbl_store_diff->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_diff->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_store_diff->setMinimumHeight(minHeight1);
   lbl_store_diff->setPalette( PALET_LABEL );
   AUTFBACK( lbl_store_diff );
   lbl_store_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_store_abs_diff = new QLabel((comparative->by_pct ? "%\n" : "") + us_tr("Absolute\ndifference"), this);
   lbl_store_abs_diff->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_store_abs_diff->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_store_abs_diff->setMinimumHeight(minHeight1);
   lbl_store_abs_diff->setPalette( PALET_LABEL );
   AUTFBACK( lbl_store_abs_diff );
   lbl_store_abs_diff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   // ------------------- s -------------------
   cb_active_s = new QCheckBox(this);
   cb_active_s->setMinimumHeight(minHeight3);
   cb_active_s->setText(comparative->ce_s.name);
   cb_active_s->setEnabled(true);
   cb_active_s->setChecked(comparative->ce_s.active);
   cb_active_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_s->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_s );
   connect(cb_active_s, SIGNAL(clicked()), SLOT(set_active_s()));

   le_target_s = new QLineEdit( this );    le_target_s->setObjectName( "target_s Line Edit" );
   le_target_s->setText(QString("%1").arg(comparative->ce_s.target));
   // le_target_s->setMinimumHeight(minHeight1);
   le_target_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_s );
   le_target_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_s, SIGNAL(textChanged(const QString &)), SLOT(update_target_s(const QString &)));

   le_rank_s = new QLineEdit( this );    le_rank_s->setObjectName( "rank_s Line Edit" );
   le_rank_s->setText(QString("%1").arg(comparative->ce_s.rank));
   // le_rank_s->setMinimumHeight(minHeight1);
   le_rank_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_s );
   le_rank_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_s, SIGNAL(textChanged(const QString &)), SLOT(update_rank_s(const QString &)));
   
   cb_include_in_weight_s = new QCheckBox(this);
   cb_include_in_weight_s->setMinimumHeight(minHeight3);
   cb_include_in_weight_s->setText("");
   cb_include_in_weight_s->setChecked(comparative->ce_s.include_in_weight);
   cb_include_in_weight_s->setEnabled(true);
   cb_include_in_weight_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_s->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_s );
   connect(cb_include_in_weight_s, SIGNAL(clicked()), SLOT(set_include_in_weight_s()));

   le_weight_s = new QLineEdit( this );    le_weight_s->setObjectName( "weight_s Line Edit" );
   le_weight_s->setText(QString("%1").arg(comparative->ce_s.weight));
   // le_weight_s->setMinimumHeight(minHeight1);
   le_weight_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_s );
   le_weight_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_s, SIGNAL(textChanged(const QString &)), SLOT(update_weight_s(const QString &)));

   le_buckets_s = new QLineEdit( this );    le_buckets_s->setObjectName( "buckets_s Line Edit" );
   le_buckets_s->setText(QString("%1").arg(comparative->ce_s.buckets));
   // le_buckets_s->setMinimumHeight(minHeight1);
   le_buckets_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_s );
   le_buckets_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_s, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_s(const QString &)));

   le_min_s = new QLineEdit( this );    le_min_s->setObjectName( "min_s Line Edit" );
   le_min_s->setText(QString("%1").arg(comparative->ce_s.min));
   // le_min_s->setMinimumHeight(minHeight1);
   le_min_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_s );
   le_min_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_s, SIGNAL(textChanged(const QString &)), SLOT(update_min_s(const QString &)));

   le_max_s = new QLineEdit( this );    le_max_s->setObjectName( "max_s Line Edit" );
   le_max_s->setText(QString("%1").arg(comparative->ce_s.max));
   // le_max_s->setMinimumHeight(minHeight1);
   le_max_s->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_s->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_s );
   le_max_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_s, SIGNAL(textChanged(const QString &)), SLOT(update_max_s(const QString &)));

   cb_store_reference_s = new QCheckBox(this);
   cb_store_reference_s->setMinimumHeight(minHeight3);
   cb_store_reference_s->setText("");
   cb_store_reference_s->setChecked(comparative->ce_s.store_reference);
   cb_store_reference_s->setEnabled(true);
   cb_store_reference_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_s->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_s );
   connect(cb_store_reference_s, SIGNAL(clicked()), SLOT(set_store_reference_s()));

   cb_store_diff_s = new QCheckBox(this);
   cb_store_diff_s->setMinimumHeight(minHeight3);
   cb_store_diff_s->setText("");
   cb_store_diff_s->setChecked(comparative->ce_s.store_diff);
   cb_store_diff_s->setEnabled(true);
   cb_store_diff_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_s->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_s );
   connect(cb_store_diff_s, SIGNAL(clicked()), SLOT(set_store_diff_s()));

   cb_store_abs_diff_s = new QCheckBox(this);
   cb_store_abs_diff_s->setMinimumHeight(minHeight3);
   cb_store_abs_diff_s->setText("");
   cb_store_abs_diff_s->setChecked(comparative->ce_s.store_abs_diff);
   cb_store_abs_diff_s->setEnabled(true);
   cb_store_abs_diff_s->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_s->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_s );
   connect(cb_store_abs_diff_s, SIGNAL(clicked()), SLOT(set_store_abs_diff_s()));

   // ------------------- D -------------------
   cb_active_D = new QCheckBox(this);
   cb_active_D->setMinimumHeight(minHeight3);
   cb_active_D->setText(comparative->ce_D.name);
   cb_active_D->setEnabled(true);
   cb_active_D->setChecked(comparative->ce_D.active);
   cb_active_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_D->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_D );
   connect(cb_active_D, SIGNAL(clicked()), SLOT(set_active_D()));

   le_target_D = new QLineEdit( this );    le_target_D->setObjectName( "target_D Line Edit" );
   le_target_D->setText(QString("%1").arg(comparative->ce_D.target));
   // le_target_D->setMinimumHeight(minHeight1);
   le_target_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_D );
   le_target_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_D, SIGNAL(textChanged(const QString &)), SLOT(update_target_D(const QString &)));

   le_rank_D = new QLineEdit( this );    le_rank_D->setObjectName( "rank_D Line Edit" );
   le_rank_D->setText(QString("%1").arg(comparative->ce_D.rank));
   // le_rank_D->setMinimumHeight(minHeight1);
   le_rank_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_D );
   le_rank_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_D, SIGNAL(textChanged(const QString &)), SLOT(update_rank_D(const QString &)));

   cb_include_in_weight_D = new QCheckBox(this);
   cb_include_in_weight_D->setMinimumHeight(minHeight3);
   cb_include_in_weight_D->setText("");
   cb_include_in_weight_D->setChecked(comparative->ce_D.include_in_weight);
   cb_include_in_weight_D->setEnabled(true);
   cb_include_in_weight_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_D->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_D );
   connect(cb_include_in_weight_D, SIGNAL(clicked()), SLOT(set_include_in_weight_D()));

   le_weight_D = new QLineEdit( this );    le_weight_D->setObjectName( "weight_D Line Edit" );
   le_weight_D->setText(QString("%1").arg(comparative->ce_D.weight));
   // le_weight_D->setMinimumHeight(minHeight1);
   le_weight_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_D );
   le_weight_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_D, SIGNAL(textChanged(const QString &)), SLOT(update_weight_D(const QString &)));

   le_buckets_D = new QLineEdit( this );    le_buckets_D->setObjectName( "buckets_D Line Edit" );
   le_buckets_D->setText(QString("%1").arg(comparative->ce_D.buckets));
   // le_buckets_D->setMinimumHeight(minHeight1);
   le_buckets_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_D );
   le_buckets_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_D, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_D(const QString &)));

   le_min_D = new QLineEdit( this );    le_min_D->setObjectName( "min_D Line Edit" );
   le_min_D->setText(QString("%1").arg(comparative->ce_D.min));
   // le_min_D->setMinimumHeight(minHeight1);
   le_min_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_D );
   le_min_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_D, SIGNAL(textChanged(const QString &)), SLOT(update_min_D(const QString &)));

   le_max_D = new QLineEdit( this );    le_max_D->setObjectName( "max_D Line Edit" );
   le_max_D->setText(QString("%1").arg(comparative->ce_D.max));
   // le_max_D->setMinimumHeight(minHeight1);
   le_max_D->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_D->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_D );
   le_max_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_D, SIGNAL(textChanged(const QString &)), SLOT(update_max_D(const QString &)));

   cb_store_reference_D = new QCheckBox(this);
   cb_store_reference_D->setMinimumHeight(minHeight3);
   cb_store_reference_D->setText("");
   cb_store_reference_D->setChecked(comparative->ce_D.store_reference);
   cb_store_reference_D->setEnabled(true);
   cb_store_reference_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_D->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_D );
   connect(cb_store_reference_D, SIGNAL(clicked()), SLOT(set_store_reference_D()));

   cb_store_diff_D = new QCheckBox(this);
   cb_store_diff_D->setMinimumHeight(minHeight3);
   cb_store_diff_D->setText("");
   cb_store_diff_D->setChecked(comparative->ce_D.store_diff);
   cb_store_diff_D->setEnabled(true);
   cb_store_diff_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_D->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_D );
   connect(cb_store_diff_D, SIGNAL(clicked()), SLOT(set_store_diff_D()));

   cb_store_abs_diff_D = new QCheckBox(this);
   cb_store_abs_diff_D->setMinimumHeight(minHeight3);
   cb_store_abs_diff_D->setText("");
   cb_store_abs_diff_D->setChecked(comparative->ce_D.store_abs_diff);
   cb_store_abs_diff_D->setEnabled(true);
   cb_store_abs_diff_D->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_D->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_D );
   connect(cb_store_abs_diff_D, SIGNAL(clicked()), SLOT(set_store_abs_diff_D()));

   // ------------------- sr -------------------
   cb_active_sr = new QCheckBox(this);
   cb_active_sr->setMinimumHeight(minHeight3);
   cb_active_sr->setText(comparative->ce_sr.name);
   cb_active_sr->setEnabled(true);
   cb_active_sr->setChecked(comparative->ce_sr.active);
   cb_active_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_sr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_sr );
   connect(cb_active_sr, SIGNAL(clicked()), SLOT(set_active_sr()));

   le_target_sr = new QLineEdit( this );    le_target_sr->setObjectName( "target_sr Line Edit" );
   le_target_sr->setText(QString("%1").arg(comparative->ce_sr.target));
   // le_target_sr->setMinimumHeight(minHeight1);
   le_target_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_sr );
   le_target_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_sr, SIGNAL(textChanged(const QString &)), SLOT(update_target_sr(const QString &)));

   le_rank_sr = new QLineEdit( this );    le_rank_sr->setObjectName( "rank_sr Line Edit" );
   le_rank_sr->setText(QString("%1").arg(comparative->ce_sr.rank));
   // le_rank_sr->setMinimumHeight(minHeight1);
   le_rank_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_sr );
   le_rank_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_sr, SIGNAL(textChanged(const QString &)), SLOT(update_rank_sr(const QString &)));

   cb_include_in_weight_sr = new QCheckBox(this);
   cb_include_in_weight_sr->setMinimumHeight(minHeight3);
   cb_include_in_weight_sr->setText("");
   cb_include_in_weight_sr->setChecked(comparative->ce_sr.include_in_weight);
   cb_include_in_weight_sr->setEnabled(true);
   cb_include_in_weight_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_sr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_sr );
   connect(cb_include_in_weight_sr, SIGNAL(clicked()), SLOT(set_include_in_weight_sr()));

   le_weight_sr = new QLineEdit( this );    le_weight_sr->setObjectName( "weight_sr Line Edit" );
   le_weight_sr->setText(QString("%1").arg(comparative->ce_sr.weight));
   // le_weight_sr->setMinimumHeight(minHeight1);
   le_weight_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_sr );
   le_weight_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_sr, SIGNAL(textChanged(const QString &)), SLOT(update_weight_sr(const QString &)));

   le_buckets_sr = new QLineEdit( this );    le_buckets_sr->setObjectName( "buckets_sr Line Edit" );
   le_buckets_sr->setText(QString("%1").arg(comparative->ce_sr.buckets));
   // le_buckets_sr->setMinimumHeight(minHeight1);
   le_buckets_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_sr );
   le_buckets_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_sr, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_sr(const QString &)));

   le_min_sr = new QLineEdit( this );    le_min_sr->setObjectName( "min_sr Line Edit" );
   le_min_sr->setText(QString("%1").arg(comparative->ce_sr.min));
   // le_min_sr->setMinimumHeight(minHeight1);
   le_min_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_sr );
   le_min_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_sr, SIGNAL(textChanged(const QString &)), SLOT(update_min_sr(const QString &)));

   le_max_sr = new QLineEdit( this );    le_max_sr->setObjectName( "max_sr Line Edit" );
   le_max_sr->setText(QString("%1").arg(comparative->ce_sr.max));
   // le_max_sr->setMinimumHeight(minHeight1);
   le_max_sr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_sr->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_sr );
   le_max_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_sr, SIGNAL(textChanged(const QString &)), SLOT(update_max_sr(const QString &)));

   cb_store_reference_sr = new QCheckBox(this);
   cb_store_reference_sr->setMinimumHeight(minHeight3);
   cb_store_reference_sr->setText("");
   cb_store_reference_sr->setChecked(comparative->ce_sr.store_reference);
   cb_store_reference_sr->setEnabled(true);
   cb_store_reference_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_sr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_sr );
   connect(cb_store_reference_sr, SIGNAL(clicked()), SLOT(set_store_reference_sr()));

   cb_store_diff_sr = new QCheckBox(this);
   cb_store_diff_sr->setMinimumHeight(minHeight3);
   cb_store_diff_sr->setText("");
   cb_store_diff_sr->setChecked(comparative->ce_sr.store_diff);
   cb_store_diff_sr->setEnabled(true);
   cb_store_diff_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_sr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_sr );
   connect(cb_store_diff_sr, SIGNAL(clicked()), SLOT(set_store_diff_sr()));

   cb_store_abs_diff_sr = new QCheckBox(this);
   cb_store_abs_diff_sr->setMinimumHeight(minHeight3);
   cb_store_abs_diff_sr->setText("");
   cb_store_abs_diff_sr->setChecked(comparative->ce_sr.store_abs_diff);
   cb_store_abs_diff_sr->setEnabled(true);
   cb_store_abs_diff_sr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_sr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_sr );
   connect(cb_store_abs_diff_sr, SIGNAL(clicked()), SLOT(set_store_abs_diff_sr()));

   // ------------------- fr -------------------
   cb_active_fr = new QCheckBox(this);
   cb_active_fr->setMinimumHeight(minHeight3);
   cb_active_fr->setText(comparative->ce_fr.name);
   cb_active_fr->setEnabled(true);
   cb_active_fr->setChecked(comparative->ce_fr.active);
   cb_active_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_fr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_fr );
   connect(cb_active_fr, SIGNAL(clicked()), SLOT(set_active_fr()));

   le_target_fr = new QLineEdit( this );    le_target_fr->setObjectName( "target_fr Line Edit" );
   le_target_fr->setText(QString("%1").arg(comparative->ce_fr.target));
   // le_target_fr->setMinimumHeight(minHeight1);
   le_target_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_fr );
   le_target_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_fr, SIGNAL(textChanged(const QString &)), SLOT(update_target_fr(const QString &)));

   le_rank_fr = new QLineEdit( this );    le_rank_fr->setObjectName( "rank_fr Line Edit" );
   le_rank_fr->setText(QString("%1").arg(comparative->ce_fr.rank));
   // le_rank_fr->setMinimumHeight(minHeight1);
   le_rank_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_fr );
   le_rank_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_fr, SIGNAL(textChanged(const QString &)), SLOT(update_rank_fr(const QString &)));

   cb_include_in_weight_fr = new QCheckBox(this);
   cb_include_in_weight_fr->setMinimumHeight(minHeight3);
   cb_include_in_weight_fr->setText("");
   cb_include_in_weight_fr->setChecked(comparative->ce_fr.include_in_weight);
   cb_include_in_weight_fr->setEnabled(true);
   cb_include_in_weight_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_fr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_fr );
   connect(cb_include_in_weight_fr, SIGNAL(clicked()), SLOT(set_include_in_weight_fr()));

   le_weight_fr = new QLineEdit( this );    le_weight_fr->setObjectName( "weight_fr Line Edit" );
   le_weight_fr->setText(QString("%1").arg(comparative->ce_fr.weight));
   // le_weight_fr->setMinimumHeight(minHeight1);
   le_weight_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_fr );
   le_weight_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_fr, SIGNAL(textChanged(const QString &)), SLOT(update_weight_fr(const QString &)));

   le_buckets_fr = new QLineEdit( this );    le_buckets_fr->setObjectName( "buckets_fr Line Edit" );
   le_buckets_fr->setText(QString("%1").arg(comparative->ce_fr.buckets));
   // le_buckets_fr->setMinimumHeight(minHeight1);
   le_buckets_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_fr );
   le_buckets_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_fr, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_fr(const QString &)));

   le_min_fr = new QLineEdit( this );    le_min_fr->setObjectName( "min_fr Line Edit" );
   le_min_fr->setText(QString("%1").arg(comparative->ce_fr.min));
   // le_min_fr->setMinimumHeight(minHeight1);
   le_min_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_fr );
   le_min_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_fr, SIGNAL(textChanged(const QString &)), SLOT(update_min_fr(const QString &)));

   le_max_fr = new QLineEdit( this );    le_max_fr->setObjectName( "max_fr Line Edit" );
   le_max_fr->setText(QString("%1").arg(comparative->ce_fr.max));
   // le_max_fr->setMinimumHeight(minHeight1);
   le_max_fr->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_fr->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_fr );
   le_max_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_fr, SIGNAL(textChanged(const QString &)), SLOT(update_max_fr(const QString &)));

   cb_store_reference_fr = new QCheckBox(this);
   cb_store_reference_fr->setMinimumHeight(minHeight3);
   cb_store_reference_fr->setText("");
   cb_store_reference_fr->setChecked(comparative->ce_fr.store_reference);
   cb_store_reference_fr->setEnabled(true);
   cb_store_reference_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_fr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_fr );
   connect(cb_store_reference_fr, SIGNAL(clicked()), SLOT(set_store_reference_fr()));

   cb_store_diff_fr = new QCheckBox(this);
   cb_store_diff_fr->setMinimumHeight(minHeight3);
   cb_store_diff_fr->setText("");
   cb_store_diff_fr->setChecked(comparative->ce_fr.store_diff);
   cb_store_diff_fr->setEnabled(true);
   cb_store_diff_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_fr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_fr );
   connect(cb_store_diff_fr, SIGNAL(clicked()), SLOT(set_store_diff_fr()));

   cb_store_abs_diff_fr = new QCheckBox(this);
   cb_store_abs_diff_fr->setMinimumHeight(minHeight3);
   cb_store_abs_diff_fr->setText("");
   cb_store_abs_diff_fr->setChecked(comparative->ce_fr.store_abs_diff);
   cb_store_abs_diff_fr->setEnabled(true);
   cb_store_abs_diff_fr->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_fr->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_fr );
   connect(cb_store_abs_diff_fr, SIGNAL(clicked()), SLOT(set_store_abs_diff_fr()));

   // ------------------- rg -------------------
   cb_active_rg = new QCheckBox(this);
   cb_active_rg->setMinimumHeight(minHeight3);
   cb_active_rg->setText(comparative->ce_rg.name);
   cb_active_rg->setEnabled(true);
   cb_active_rg->setChecked(comparative->ce_rg.active);
   cb_active_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_rg );
   connect(cb_active_rg, SIGNAL(clicked()), SLOT(set_active_rg()));

   le_target_rg = new QLineEdit( this );    le_target_rg->setObjectName( "target_rg Line Edit" );
   le_target_rg->setText(QString("%1").arg(comparative->ce_rg.target));
   // le_target_rg->setMinimumHeight(minHeight1);
   le_target_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_rg );
   le_target_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_rg, SIGNAL(textChanged(const QString &)), SLOT(update_target_rg(const QString &)));

   le_rank_rg = new QLineEdit( this );    le_rank_rg->setObjectName( "rank_rg Line Edit" );
   le_rank_rg->setText(QString("%1").arg(comparative->ce_rg.rank));
   // le_rank_rg->setMinimumHeight(minHeight1);
   le_rank_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_rg );
   le_rank_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_rg, SIGNAL(textChanged(const QString &)), SLOT(update_rank_rg(const QString &)));

   cb_include_in_weight_rg = new QCheckBox(this);
   cb_include_in_weight_rg->setMinimumHeight(minHeight3);
   cb_include_in_weight_rg->setText("");
   cb_include_in_weight_rg->setChecked(comparative->ce_rg.include_in_weight);
   cb_include_in_weight_rg->setEnabled(true);
   cb_include_in_weight_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_rg );
   connect(cb_include_in_weight_rg, SIGNAL(clicked()), SLOT(set_include_in_weight_rg()));

   le_weight_rg = new QLineEdit( this );    le_weight_rg->setObjectName( "weight_rg Line Edit" );
   le_weight_rg->setText(QString("%1").arg(comparative->ce_rg.weight));
   // le_weight_rg->setMinimumHeight(minHeight1);
   le_weight_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_rg );
   le_weight_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_rg, SIGNAL(textChanged(const QString &)), SLOT(update_weight_rg(const QString &)));

   le_buckets_rg = new QLineEdit( this );    le_buckets_rg->setObjectName( "buckets_rg Line Edit" );
   le_buckets_rg->setText(QString("%1").arg(comparative->ce_rg.buckets));
   // le_buckets_rg->setMinimumHeight(minHeight1);
   le_buckets_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_rg );
   le_buckets_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_rg, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_rg(const QString &)));

   le_min_rg = new QLineEdit( this );    le_min_rg->setObjectName( "min_rg Line Edit" );
   le_min_rg->setText(QString("%1").arg(comparative->ce_rg.min));
   // le_min_rg->setMinimumHeight(minHeight1);
   le_min_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_rg );
   le_min_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_rg, SIGNAL(textChanged(const QString &)), SLOT(update_min_rg(const QString &)));

   le_max_rg = new QLineEdit( this );    le_max_rg->setObjectName( "max_rg Line Edit" );
   le_max_rg->setText(QString("%1").arg(comparative->ce_rg.max));
   // le_max_rg->setMinimumHeight(minHeight1);
   le_max_rg->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_rg->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_rg );
   le_max_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_rg, SIGNAL(textChanged(const QString &)), SLOT(update_max_rg(const QString &)));

   cb_store_reference_rg = new QCheckBox(this);
   cb_store_reference_rg->setMinimumHeight(minHeight3);
   cb_store_reference_rg->setText("");
   cb_store_reference_rg->setChecked(comparative->ce_rg.store_reference);
   cb_store_reference_rg->setEnabled(true);
   cb_store_reference_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_rg );
   connect(cb_store_reference_rg, SIGNAL(clicked()), SLOT(set_store_reference_rg()));

   cb_store_diff_rg = new QCheckBox(this);
   cb_store_diff_rg->setMinimumHeight(minHeight3);
   cb_store_diff_rg->setText("");
   cb_store_diff_rg->setChecked(comparative->ce_rg.store_diff);
   cb_store_diff_rg->setEnabled(true);
   cb_store_diff_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_rg );
   connect(cb_store_diff_rg, SIGNAL(clicked()), SLOT(set_store_diff_rg()));

   cb_store_abs_diff_rg = new QCheckBox(this);
   cb_store_abs_diff_rg->setMinimumHeight(minHeight3);
   cb_store_abs_diff_rg->setText("");
   cb_store_abs_diff_rg->setChecked(comparative->ce_rg.store_abs_diff);
   cb_store_abs_diff_rg->setEnabled(true);
   cb_store_abs_diff_rg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_rg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_rg );
   connect(cb_store_abs_diff_rg, SIGNAL(clicked()), SLOT(set_store_abs_diff_rg()));

   // ------------------- tau -------------------
   cb_active_tau = new QCheckBox(this);
   cb_active_tau->setMinimumHeight(minHeight3);
   cb_active_tau->setText(comparative->ce_tau.name);
   cb_active_tau->setEnabled(true);
   cb_active_tau->setChecked(comparative->ce_tau.active);
   cb_active_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_tau->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_tau );
   connect(cb_active_tau, SIGNAL(clicked()), SLOT(set_active_tau()));

   le_target_tau = new QLineEdit( this );    le_target_tau->setObjectName( "target_tau Line Edit" );
   le_target_tau->setText(QString("%1").arg(comparative->ce_tau.target));
   // le_target_tau->setMinimumHeight(minHeight1);
   le_target_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_tau );
   le_target_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_tau, SIGNAL(textChanged(const QString &)), SLOT(update_target_tau(const QString &)));

   le_rank_tau = new QLineEdit( this );    le_rank_tau->setObjectName( "rank_tau Line Edit" );
   le_rank_tau->setText(QString("%1").arg(comparative->ce_tau.rank));
   // le_rank_tau->setMinimumHeight(minHeight1);
   le_rank_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_tau );
   le_rank_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_tau, SIGNAL(textChanged(const QString &)), SLOT(update_rank_tau(const QString &)));

   cb_include_in_weight_tau = new QCheckBox(this);
   cb_include_in_weight_tau->setMinimumHeight(minHeight3);
   cb_include_in_weight_tau->setText("");
   cb_include_in_weight_tau->setChecked(comparative->ce_tau.include_in_weight);
   cb_include_in_weight_tau->setEnabled(true);
   cb_include_in_weight_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_tau->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_tau );
   connect(cb_include_in_weight_tau, SIGNAL(clicked()), SLOT(set_include_in_weight_tau()));

   le_weight_tau = new QLineEdit( this );    le_weight_tau->setObjectName( "weight_tau Line Edit" );
   le_weight_tau->setText(QString("%1").arg(comparative->ce_tau.weight));
   // le_weight_tau->setMinimumHeight(minHeight1);
   le_weight_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_tau );
   le_weight_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_tau, SIGNAL(textChanged(const QString &)), SLOT(update_weight_tau(const QString &)));

   le_buckets_tau = new QLineEdit( this );    le_buckets_tau->setObjectName( "buckets_tau Line Edit" );
   le_buckets_tau->setText(QString("%1").arg(comparative->ce_tau.buckets));
   // le_buckets_tau->setMinimumHeight(minHeight1);
   le_buckets_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_tau );
   le_buckets_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_tau, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_tau(const QString &)));

   le_min_tau = new QLineEdit( this );    le_min_tau->setObjectName( "min_tau Line Edit" );
   le_min_tau->setText(QString("%1").arg(comparative->ce_tau.min));
   // le_min_tau->setMinimumHeight(minHeight1);
   le_min_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_tau );
   le_min_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_tau, SIGNAL(textChanged(const QString &)), SLOT(update_min_tau(const QString &)));

   le_max_tau = new QLineEdit( this );    le_max_tau->setObjectName( "max_tau Line Edit" );
   le_max_tau->setText(QString("%1").arg(comparative->ce_tau.max));
   // le_max_tau->setMinimumHeight(minHeight1);
   le_max_tau->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_tau->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_tau );
   le_max_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_tau, SIGNAL(textChanged(const QString &)), SLOT(update_max_tau(const QString &)));

   cb_store_reference_tau = new QCheckBox(this);
   cb_store_reference_tau->setMinimumHeight(minHeight3);
   cb_store_reference_tau->setText("");
   cb_store_reference_tau->setChecked(comparative->ce_tau.store_reference);
   cb_store_reference_tau->setEnabled(true);
   cb_store_reference_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_tau->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_tau );
   connect(cb_store_reference_tau, SIGNAL(clicked()), SLOT(set_store_reference_tau()));

   cb_store_diff_tau = new QCheckBox(this);
   cb_store_diff_tau->setMinimumHeight(minHeight3);
   cb_store_diff_tau->setText("");
   cb_store_diff_tau->setChecked(comparative->ce_tau.store_diff);
   cb_store_diff_tau->setEnabled(true);
   cb_store_diff_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_tau->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_tau );
   connect(cb_store_diff_tau, SIGNAL(clicked()), SLOT(set_store_diff_tau()));

   cb_store_abs_diff_tau = new QCheckBox(this);
   cb_store_abs_diff_tau->setMinimumHeight(minHeight3);
   cb_store_abs_diff_tau->setText("");
   cb_store_abs_diff_tau->setChecked(comparative->ce_tau.store_abs_diff);
   cb_store_abs_diff_tau->setEnabled(true);
   cb_store_abs_diff_tau->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_tau->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_tau );
   connect(cb_store_abs_diff_tau, SIGNAL(clicked()), SLOT(set_store_abs_diff_tau()));

   // ------------------- eta -------------------
   cb_active_eta = new QCheckBox(this);
   cb_active_eta->setMinimumHeight(minHeight3);
   cb_active_eta->setText(comparative->ce_eta.name);
   cb_active_eta->setEnabled(true);
   cb_active_eta->setChecked(comparative->ce_eta.active);
   cb_active_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_active_eta->setPalette( PALET_NORMAL );
   AUTFBACK( cb_active_eta );
   connect(cb_active_eta, SIGNAL(clicked()), SLOT(set_active_eta()));

   le_target_eta = new QLineEdit( this );    le_target_eta->setObjectName( "target_eta Line Edit" );
   le_target_eta->setText(QString("%1").arg(comparative->ce_eta.target));
   // le_target_eta->setMinimumHeight(minHeight1);
   le_target_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_target_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_target_eta );
   le_target_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_target_eta, SIGNAL(textChanged(const QString &)), SLOT(update_target_eta(const QString &)));

   le_rank_eta = new QLineEdit( this );    le_rank_eta->setObjectName( "rank_eta Line Edit" );
   le_rank_eta->setText(QString("%1").arg(comparative->ce_eta.rank));
   // le_rank_eta->setMinimumHeight(minHeight1);
   le_rank_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_rank_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_rank_eta );
   le_rank_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_rank_eta, SIGNAL(textChanged(const QString &)), SLOT(update_rank_eta(const QString &)));

   cb_include_in_weight_eta = new QCheckBox(this);
   cb_include_in_weight_eta->setMinimumHeight(minHeight3);
   cb_include_in_weight_eta->setText("");
   cb_include_in_weight_eta->setChecked(comparative->ce_eta.include_in_weight);
   cb_include_in_weight_eta->setEnabled(true);
   cb_include_in_weight_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include_in_weight_eta->setPalette( PALET_NORMAL );
   AUTFBACK( cb_include_in_weight_eta );
   connect(cb_include_in_weight_eta, SIGNAL(clicked()), SLOT(set_include_in_weight_eta()));

   le_weight_eta = new QLineEdit( this );    le_weight_eta->setObjectName( "weight_eta Line Edit" );
   le_weight_eta->setText(QString("%1").arg(comparative->ce_eta.weight));
   // le_weight_eta->setMinimumHeight(minHeight1);
   le_weight_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_weight_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_weight_eta );
   le_weight_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_weight_eta, SIGNAL(textChanged(const QString &)), SLOT(update_weight_eta(const QString &)));

   le_buckets_eta = new QLineEdit( this );    le_buckets_eta->setObjectName( "buckets_eta Line Edit" );
   le_buckets_eta->setText(QString("%1").arg(comparative->ce_eta.buckets));
   // le_buckets_eta->setMinimumHeight(minHeight1);
   le_buckets_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_buckets_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_buckets_eta );
   le_buckets_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_buckets_eta, SIGNAL(textChanged(const QString &)), SLOT(update_buckets_eta(const QString &)));

   le_min_eta = new QLineEdit( this );    le_min_eta->setObjectName( "min_eta Line Edit" );
   le_min_eta->setText(QString("%1").arg(comparative->ce_eta.min));
   // le_min_eta->setMinimumHeight(minHeight1);
   le_min_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_min_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_min_eta );
   le_min_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_min_eta, SIGNAL(textChanged(const QString &)), SLOT(update_min_eta(const QString &)));

   le_max_eta = new QLineEdit( this );    le_max_eta->setObjectName( "max_eta Line Edit" );
   le_max_eta->setText(QString("%1").arg(comparative->ce_eta.max));
   // le_max_eta->setMinimumHeight(minHeight1);
   le_max_eta->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_max_eta->setPalette( PALET_NORMAL );
   AUTFBACK( le_max_eta );
   le_max_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_max_eta, SIGNAL(textChanged(const QString &)), SLOT(update_max_eta(const QString &)));

   cb_store_reference_eta = new QCheckBox(this);
   cb_store_reference_eta->setMinimumHeight(minHeight3);
   cb_store_reference_eta->setText("");
   cb_store_reference_eta->setChecked(comparative->ce_eta.store_reference);
   cb_store_reference_eta->setEnabled(true);
   cb_store_reference_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_reference_eta->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_reference_eta );
   connect(cb_store_reference_eta, SIGNAL(clicked()), SLOT(set_store_reference_eta()));

   cb_store_diff_eta = new QCheckBox(this);
   cb_store_diff_eta->setMinimumHeight(minHeight3);
   cb_store_diff_eta->setText("");
   cb_store_diff_eta->setChecked(comparative->ce_eta.store_diff);
   cb_store_diff_eta->setEnabled(true);
   cb_store_diff_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_diff_eta->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_diff_eta );
   connect(cb_store_diff_eta, SIGNAL(clicked()), SLOT(set_store_diff_eta()));

   cb_store_abs_diff_eta = new QCheckBox(this);
   cb_store_abs_diff_eta->setMinimumHeight(minHeight3);
   cb_store_abs_diff_eta->setText("");
   cb_store_abs_diff_eta->setChecked(comparative->ce_eta.store_abs_diff);
   cb_store_abs_diff_eta->setEnabled(true);
   cb_store_abs_diff_eta->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_store_abs_diff_eta->setPalette( PALET_NORMAL );
   AUTFBACK( cb_store_abs_diff_eta );
   connect(cb_store_abs_diff_eta, SIGNAL(clicked()), SLOT(set_store_abs_diff_eta()));

   pb_load_param = new QPushButton(us_tr("Load Parameters"), this);
   pb_load_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_param->setMinimumHeight(minHeight1);
   pb_load_param->setEnabled(true);
   pb_load_param->setPalette( PALET_PUSHB );
   connect(pb_load_param, SIGNAL(clicked()), SLOT(load_param()));

   pb_reset_param = new QPushButton(us_tr("Reset Parameters"), this);
   pb_reset_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_reset_param->setMinimumHeight(minHeight1);
   pb_reset_param->setEnabled(true);
   pb_reset_param->setPalette( PALET_PUSHB );
   connect(pb_reset_param, SIGNAL(clicked()), SLOT(reset_param()));

   pb_save_param = new QPushButton(us_tr("Save Parameters"), this);
   pb_save_param->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_param->setMinimumHeight(minHeight1);
   pb_save_param->setEnabled(true);
   pb_save_param->setPalette( PALET_PUSHB );
   connect(pb_save_param, SIGNAL(clicked()), SLOT(save_param()));

   lbl_title_csv = new QLabel(us_tr("CSV Processing"), this);
   lbl_title_csv->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_title_csv->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_title_csv->setMinimumHeight(minHeight1);
   lbl_title_csv->setPalette( PALET_FRAME );
   AUTFBACK( lbl_title_csv );
   lbl_title_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   pb_load_csv = new QPushButton(us_tr("Load"), this);
   pb_load_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_load_csv->setMinimumHeight(minHeight1);
   pb_load_csv->setEnabled(true);
   pb_load_csv->setPalette( PALET_PUSHB );
   connect(pb_load_csv, SIGNAL(clicked()), SLOT(load_csv()));

   pb_process_csv = new QPushButton(us_tr("Process"), this);
   pb_process_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_process_csv->setMinimumHeight(minHeight1);
   pb_process_csv->setEnabled(false);
   pb_process_csv->setPalette( PALET_PUSHB );
   connect(pb_process_csv, SIGNAL(clicked()), SLOT(process_csv()));

   pb_save_csv = new QPushButton(us_tr("Save"), this);
   pb_save_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_csv->setMinimumHeight(minHeight1);
   pb_save_csv->setEnabled(false);
   pb_save_csv->setPalette( PALET_PUSHB );
   connect(pb_save_csv, SIGNAL(clicked()), SLOT(save_csv()));

   // lbl_loaded = new QLabel(us_tr("Loaded"), this);
   // lbl_loaded->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_loaded->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_loaded->setMinimumHeight(minHeight1);
   // lbl_loaded->setPalette( PALET_FRAME );
   // lbl_loaded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_loaded = new QListWidget(this);
   lb_loaded->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_loaded->setMinimumHeight(minHeight1 * 6);
   // lb_loaded->setMinimumWidth(minWidth1);
   // lb_loaded->addItems(*qsl_loaded);
   lb_loaded->setPalette( PALET_EDIT );
   AUTFBACK( lb_loaded );
   lb_loaded->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_loaded->setSelectionMode(QAbstractItemView::MultiSelection);
   lb_loaded->setEnabled(true);
   connect(lb_loaded, SIGNAL(itemSelectionChanged()), SLOT(update_loaded()));

   pb_loaded_select_all = new QPushButton(us_tr("Select all"), this);
   pb_loaded_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_select_all->setMinimumHeight(minHeightpb);
   pb_loaded_select_all->setEnabled(false);
   pb_loaded_select_all->setPalette( PALET_PUSHB );
   connect(pb_loaded_select_all, SIGNAL(clicked()), SLOT(loaded_select_all()));

   pb_loaded_view = new QPushButton(us_tr("View"), this);
   pb_loaded_view->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_view->setMinimumHeight(minHeightpb);
   pb_loaded_view->setEnabled(false);
   pb_loaded_view->setPalette( PALET_PUSHB );
   connect(pb_loaded_view, SIGNAL(clicked()), SLOT(loaded_view()));

   pb_loaded_merge = new QPushButton(us_tr("Merge"), this);
   pb_loaded_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_merge->setMinimumHeight(minHeightpb);
   pb_loaded_merge->setEnabled(false);
   pb_loaded_merge->setPalette( PALET_PUSHB );
   connect(pb_loaded_merge, SIGNAL(clicked()), SLOT(loaded_merge()));

   pb_loaded_set_ranges = new QPushButton(us_tr("Set min/max"), this);
   pb_loaded_set_ranges->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_loaded_set_ranges->setMinimumHeight(minHeightpb);
   pb_loaded_set_ranges->setEnabled(false);
   pb_loaded_set_ranges->setPalette( PALET_PUSHB );
   connect(pb_loaded_set_ranges, SIGNAL(clicked()), SLOT(loaded_set_ranges()));

   pb_loaded_remove = new QPushButton(us_tr("Remove"), this);
   pb_loaded_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_loaded_remove->setMinimumHeight(minHeightpb);
   pb_loaded_remove->setEnabled(false);
   pb_loaded_remove->setPalette( PALET_PUSHB );
   connect(pb_loaded_remove, SIGNAL(clicked()), SLOT(loaded_remove()));

   // lbl_selected = new QLabel(us_tr("Selected"), this);
   // lbl_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   // lbl_selected->setMinimumHeight(minHeight1);
   // lbl_selected->setPalette( PALET_FRAME );
   // lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_selected = new QListWidget(this);
   lb_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   // lb_selected->setMinimumHeight(minHeight1 * 15);
   // lb_selected->setMinimumWidth(minWidth1);
   // lb_selected->addItems(*qsl_selected);
   lb_selected->setPalette( PALET_EDIT );
   AUTFBACK( lb_selected );
   lb_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_selected->setSelectionMode(QAbstractItemView::MultiSelection);
   lb_selected->setEnabled(true);
   connect(lb_selected, SIGNAL(itemSelectionChanged()), SLOT(update_selected()));

   pb_selected_select_all = new QPushButton(us_tr("Select All"), this);
   pb_selected_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_select_all->setMinimumHeight(minHeightpb);
   pb_selected_select_all->setEnabled(false);
   pb_selected_select_all->setPalette( PALET_PUSHB );
   connect(pb_selected_select_all, SIGNAL(clicked()), SLOT(selected_select_all()));

   pb_selected_merge = new QPushButton(us_tr("Merge"), this);
   pb_selected_merge->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_merge->setMinimumHeight(minHeightpb);
   pb_selected_merge->setEnabled(false);
   pb_selected_merge->setPalette( PALET_PUSHB );
   connect(pb_selected_merge, SIGNAL(clicked()), SLOT(selected_merge()));

   pb_selected_set_ranges = new QPushButton(us_tr("Set min/max"),this);
   pb_selected_set_ranges->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_set_ranges->setMinimumHeight(minHeightpb);
   pb_selected_set_ranges->setEnabled(false);
   pb_selected_set_ranges->setPalette( PALET_PUSHB );
   connect(pb_selected_set_ranges, SIGNAL(clicked()), SLOT(selected_set_ranges()));

   pb_selected_remove = new QPushButton(us_tr("Remove"), this);
   pb_selected_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_selected_remove->setMinimumHeight(minHeightpb);
   pb_selected_remove->setEnabled(false);
   pb_selected_remove->setPalette( PALET_PUSHB );
   connect(pb_selected_remove, SIGNAL(clicked()), SLOT(selected_remove()));

   editor = new QTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   AUTFBACK( editor );
   editor->setReadOnly(true);
   // editor->setMinimumWidth(300);
   // editor->setMinimumHeight(minHeight1 * 7);

#if QT_VERSION < 0x040000
# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
      file->insertItem( us_tr("&Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );

      QMenuBar *menu = new QMenuBar( this );
      AUTFBACK( menu );

      menu->insertItem(us_tr("&Messages"), file );
   }
# else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);

   m = new QMenuBar( frame );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1 - 5);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
   file->insertItem( us_tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight3);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );

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

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   int spacing = 2;
   int margin = 4;
   int j = 0;

   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( lbl_title_param , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
   j++;

   background->addWidget( lbl_active , j , 0 , 1 + ( j+3 ) - ( j ) , 1 + ( 0 ) - ( 0 ) );
   background->addWidget( lbl_target , j , 1 , 1 + ( j+3 ) - ( j ) , 1 + ( 1 ) - ( 1 ) );
   background->addWidget( lbl_sort , j , 2 , 1 + ( j ) - ( j ) , 1 + ( 7 ) - ( 2 ) );
   background->addWidget( cb_by_pct , j+1 , 2 , 1 + ( j+1 ) - ( j+1 ) , 1 + ( 4 ) - ( 2 ) );

   background->addWidget( cb_rank , j+2 , 2 , 1 + ( j+2 ) - ( j+2 ) , 1 + ( 2 ) - ( 2 ) );
   background->addWidget(lbl_rank, j+3, 2);

   background->addWidget( cb_weight_controls , j+2 , 3 , 1 + ( j+2 ) - ( j+2 ) , 1 + ( 4 ) - ( 3 ) );
   background->addWidget(lbl_include_in_weight, j+3, 3);
   background->addWidget(lbl_weight, j+3, 4);

   background->addWidget( lbl_ec , j+1 , 5 , 1 + ( j+1 ) - ( j+1 ) , 1 + ( 7 ) - ( 5 ) );
   background->addWidget( cb_by_ec , j+2 , 5 , 1 + ( j+2 ) - ( j+2 ) , 1 + ( 7 ) - ( 5 ) );
   background->addWidget( lbl_buckets , j+3 , 5 , 1 + ( j+3 ) - ( j+3 ) , 1 + ( 5 ) - ( 5 ) );
   background->addWidget( lbl_min , j+3 , 6 , 1 + ( j+3 ) - ( j+3 ) , 1 + ( 6 ) - ( 6 ) );
   background->addWidget( lbl_max , j+3 , 7 , 1 + ( j+3 ) - ( j+3 ) , 1 + ( 7 ) - ( 7 ) );

   background->addWidget( lbl_csv_controls , j , 8 , 1 + ( j+1 ) - ( j ) , 1 + ( 10 ) - ( 8 ) );
   background->addWidget( lbl_store_reference , j+2 , 8 , 1 + ( j+3 ) - ( j+2 ) , 1 + ( 8 ) - ( 8 ) );
   background->addWidget( lbl_store_diff , j+2 , 9 , 1 + ( j+3 ) - ( j+2 ) , 1 + ( 9 ) - ( 9 ) );
   background->addWidget( lbl_store_abs_diff , j+2 , 10 , 1 + ( j+3 ) - ( j+2 ) , 1 + ( 10 ) - ( 10 ) );

   j += 4;

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

   QBoxLayout * hbl_param = new QHBoxLayout(); hbl_param->setContentsMargins( 0, 0, 0, 0 ); hbl_param->setSpacing( 0 );
   hbl_param->addWidget(pb_load_param);
   hbl_param->addWidget(pb_reset_param);
   hbl_param->addWidget(pb_save_param);
   background->addLayout( hbl_param , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
   j++;

   background->addWidget( lbl_title_csv , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
   j++;

   QBoxLayout * hbl_csv = new QHBoxLayout(); hbl_csv->setContentsMargins( 0, 0, 0, 0 ); hbl_csv->setSpacing( 0 );
   hbl_csv->addWidget(pb_load_csv);
   hbl_csv->addWidget(pb_process_csv);
   hbl_csv->addWidget(pb_save_csv);
   background->addLayout( hbl_csv , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
   j++;

   QGridLayout * gl_loaded_selected_editor = new QGridLayout; gl_loaded_selected_editor->setContentsMargins( 0, 0, 0, 0 ); gl_loaded_selected_editor->setSpacing( 0 ); gl_loaded_selected_editor->setSpacing( 0 ); gl_loaded_selected_editor->setContentsMargins( 0, 0, 0, 0 );

   // gl_loaded_selected_editor->addWidget(lbl_loaded, 0, 0);
   gl_loaded_selected_editor->addWidget( lb_loaded , 0 , 0 , 1 + ( 1 ) - ( 0 ) , 1 + ( 0 ) - ( 0 ) );
   QBoxLayout * hbl_loaded_buttons1 = new QHBoxLayout(); hbl_loaded_buttons1->setContentsMargins( 0, 0, 0, 0 ); hbl_loaded_buttons1->setSpacing( 0 );
   hbl_loaded_buttons1->addWidget(pb_loaded_select_all);
   hbl_loaded_buttons1->addWidget(pb_loaded_remove);
   gl_loaded_selected_editor->addLayout(hbl_loaded_buttons1, 2, 0);

   QBoxLayout * hbl_loaded_buttons2 = new QHBoxLayout(); hbl_loaded_buttons2->setContentsMargins( 0, 0, 0, 0 ); hbl_loaded_buttons2->setSpacing( 0 );
   hbl_loaded_buttons2->addWidget(pb_loaded_view);
   hbl_loaded_buttons2->addWidget(pb_loaded_merge);
   hbl_loaded_buttons2->addWidget(pb_loaded_set_ranges);
   gl_loaded_selected_editor->addLayout(hbl_loaded_buttons2, 3, 0);

   // gl_loaded_selected_editor->addWidget(lbl_selected, 0, 1);
   gl_loaded_selected_editor->addWidget( lb_selected , 0 , 1 , 1 + ( 1 ) - ( 0 ) , 1 + ( 1 ) - ( 1 ) );

   QBoxLayout * hbl_selected_buttons1 = new QHBoxLayout(); hbl_selected_buttons1->setContentsMargins( 0, 0, 0, 0 ); hbl_selected_buttons1->setSpacing( 0 );
   hbl_selected_buttons1->addWidget(pb_selected_select_all);
   hbl_selected_buttons1->addWidget(pb_selected_remove);
   gl_loaded_selected_editor->addLayout(hbl_selected_buttons1, 2, 1);

   QBoxLayout * hbl_selected_buttons2 = new QHBoxLayout(); hbl_selected_buttons2->setContentsMargins( 0, 0, 0, 0 ); hbl_selected_buttons2->setSpacing( 0 );
   hbl_selected_buttons2->addWidget(pb_selected_merge);
   hbl_selected_buttons2->addWidget(pb_selected_set_ranges);
   gl_loaded_selected_editor->addLayout(hbl_selected_buttons2, 3, 1);

   QBoxLayout * vbl_editor_group = new QVBoxLayout(0); vbl_editor_group->setContentsMargins( 0, 0, 0, 0 ); vbl_editor_group->setSpacing( 0 );
#if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   vbl_editor_group->addWidget(frame);
#endif
   vbl_editor_group->addWidget(editor);

   // gl_loaded_selected_editor->addWidget(frame, 0, 2);
   //   gl_loaded_selected_editor->addWidget( editor , 1 , 2 , 1 + ( 1 ) - ( 1 ) , 1 + ( 2 ) - ( 2 ) );
   gl_loaded_selected_editor->addLayout( vbl_editor_group , 0 , 2 , 1 + ( 3 ) - ( 0 ) , 1 + ( 2 ) - ( 2 ) );
   
   background->addLayout( gl_loaded_selected_editor , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
   j++;

   QBoxLayout * hbl_bottom = new QHBoxLayout(); hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addWidget(pb_cancel);
   background->addLayout( hbl_bottom , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 10 ) - ( 0 ) );
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
   online_help->show_help("manual/somo/somo_comparative.html");
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
   if ( updates_enabled )
   {
      cout << "update_enables\n";

      lbl_store_diff->setText((comparative->by_pct ? "%\n" : "" ) + us_tr("Difference"));
      lbl_store_abs_diff->setText((comparative->by_pct ? "%\n" : "") + us_tr("Absolute\ndifference"));
      cb_rank->setText(QString(us_tr("By ranked\n%1absolute\ndifference")).arg(comparative->by_pct ? "% " : "" ));
      cb_weight_controls->setText(QString(us_tr("By weighted sum\nof %1absolute\ndifferences")).arg(comparative->by_pct ? "% " : "" ));

      bool any_selected = any_loaded_selected();
      
      bool enable_s = 
         !any_selected || all_selected_csv_contain( comparative->ce_s );
      
      //   cout << QString(
      //                   "update enables:\n"
      //                   " enable_s = %1\n"
      //                   " any_selected = %2\n"
      //                   " all_selected_csv_contain = %3\n"
      //                   )
      //      .arg(enable_s ? "true" : "false")
      //      .arg(any_selected ? "true" : "false")
      //      .arg(all_selected_csv_contain( comparative->ce_s ) ? "true" : "false");
      
      cb_active_s->setEnabled(enable_s);
      le_target_s->setEnabled(enable_s && cb_active_s->isChecked());
      le_rank_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_weight_controls->isChecked());
      le_weight_s->setEnabled(enable_s && cb_active_s->isChecked() && cb_weight_controls->isChecked());
      le_buckets_s->setEnabled(cb_by_ec->isChecked() && enable_s && cb_active_s->isChecked());
      le_min_s->setEnabled(cb_by_ec->isChecked() && enable_s && cb_active_s->isChecked() && comparative->ce_s.buckets);
      le_max_s->setEnabled(cb_by_ec->isChecked() && enable_s && cb_active_s->isChecked() && comparative->ce_s.buckets);
      cb_store_reference_s->setEnabled(enable_s && cb_active_s->isChecked());
      cb_store_diff_s->setEnabled(enable_s && cb_active_s->isChecked());
      cb_store_abs_diff_s->setEnabled(enable_s && cb_active_s->isChecked() && !cb_rank->isChecked());
      
      bool enable_D = 
         !any_selected || all_selected_csv_contain( comparative->ce_D );
      
      cb_active_D->setEnabled(enable_D);
      le_target_D->setEnabled(enable_D && cb_active_D->isChecked());
      le_rank_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_weight_controls->isChecked());
      le_weight_D->setEnabled(enable_D && cb_active_D->isChecked() && cb_weight_controls->isChecked());
      le_buckets_D->setEnabled(cb_by_ec->isChecked() && enable_D && cb_active_D->isChecked());
      le_min_D->setEnabled(cb_by_ec->isChecked() && enable_D && cb_active_D->isChecked() && comparative->ce_D.buckets);
      le_max_D->setEnabled(cb_by_ec->isChecked() && enable_D && cb_active_D->isChecked() && comparative->ce_D.buckets);
      cb_store_reference_D->setEnabled(enable_D && cb_active_D->isChecked());
      cb_store_diff_D->setEnabled(enable_D && cb_active_D->isChecked());
      cb_store_abs_diff_D->setEnabled(enable_D && cb_active_D->isChecked() && !cb_rank->isChecked());
      
      bool enable_sr = 
         !any_selected || all_selected_csv_contain( comparative->ce_sr );
      
      cb_active_sr->setEnabled(enable_sr);
      le_target_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
      le_rank_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_weight_controls->isChecked());
      le_weight_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && cb_weight_controls->isChecked());
      le_buckets_sr->setEnabled(cb_by_ec->isChecked() && enable_sr && cb_active_sr->isChecked());
      le_min_sr->setEnabled(cb_by_ec->isChecked() && enable_sr && cb_active_sr->isChecked() && comparative->ce_sr.buckets);
      le_max_sr->setEnabled(cb_by_ec->isChecked() && enable_sr && cb_active_sr->isChecked() && comparative->ce_sr.buckets);
      cb_store_reference_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
      cb_store_diff_sr->setEnabled(enable_sr && cb_active_sr->isChecked());
      cb_store_abs_diff_sr->setEnabled(enable_sr && cb_active_sr->isChecked() && !cb_rank->isChecked());
      
      bool enable_fr = 
         !any_selected || all_selected_csv_contain( comparative->ce_fr );
      
      cb_active_fr->setEnabled(enable_fr);
      le_target_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
      le_rank_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_weight_controls->isChecked());
      le_weight_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && cb_weight_controls->isChecked());
      le_buckets_fr->setEnabled(cb_by_ec->isChecked() && enable_fr && cb_active_fr->isChecked());
      le_min_fr->setEnabled(cb_by_ec->isChecked() && enable_fr && cb_active_fr->isChecked() && comparative->ce_fr.buckets);
      le_max_fr->setEnabled(cb_by_ec->isChecked() && enable_fr && cb_active_fr->isChecked() && comparative->ce_fr.buckets);
      cb_store_reference_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
      cb_store_diff_fr->setEnabled(enable_fr && cb_active_fr->isChecked());
      cb_store_abs_diff_fr->setEnabled(enable_fr && cb_active_fr->isChecked() && !cb_rank->isChecked());
      
      bool enable_rg = 
         !any_selected || all_selected_csv_contain( comparative->ce_rg );
      
      cb_active_rg->setEnabled(enable_rg);
      le_target_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
      le_rank_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_weight_controls->isChecked());
      le_weight_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && cb_weight_controls->isChecked());
      le_buckets_rg->setEnabled(cb_by_ec->isChecked() && enable_rg && cb_active_rg->isChecked());
      le_min_rg->setEnabled(cb_by_ec->isChecked() && enable_rg && cb_active_rg->isChecked() && comparative->ce_rg.buckets);
      le_max_rg->setEnabled(cb_by_ec->isChecked() && enable_rg && cb_active_rg->isChecked() && comparative->ce_rg.buckets);
      cb_store_reference_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
      cb_store_diff_rg->setEnabled(enable_rg && cb_active_rg->isChecked());
      cb_store_abs_diff_rg->setEnabled(enable_rg && cb_active_rg->isChecked() && !cb_rank->isChecked());
      
      bool enable_tau = 
         !any_selected || all_selected_csv_contain( comparative->ce_tau );
      
      cb_active_tau->setEnabled(enable_tau);
      le_target_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
      le_rank_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_weight_controls->isChecked());
      le_weight_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && cb_weight_controls->isChecked());
      le_buckets_tau->setEnabled(cb_by_ec->isChecked() && enable_tau && cb_active_tau->isChecked());
      le_min_tau->setEnabled(cb_by_ec->isChecked() && enable_tau && cb_active_tau->isChecked() && comparative->ce_tau.buckets);
      le_max_tau->setEnabled(cb_by_ec->isChecked() && enable_tau && cb_active_tau->isChecked() && comparative->ce_tau.buckets);
      cb_store_reference_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
      cb_store_diff_tau->setEnabled(enable_tau && cb_active_tau->isChecked());
      cb_store_abs_diff_tau->setEnabled(enable_tau && cb_active_tau->isChecked() && !cb_rank->isChecked());
      
      bool enable_eta = 
         !any_selected || all_selected_csv_contain( comparative->ce_eta );
      
      cb_active_eta->setEnabled(enable_eta);
      le_target_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
      le_rank_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_rank->isChecked());
      cb_include_in_weight_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_weight_controls->isChecked());
      le_weight_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && cb_weight_controls->isChecked());
      le_buckets_eta->setEnabled(cb_by_ec->isChecked() && enable_eta && cb_active_eta->isChecked());
      le_min_eta->setEnabled(cb_by_ec->isChecked() && enable_eta && cb_active_eta->isChecked() && comparative->ce_eta.buckets);
      le_max_eta->setEnabled(cb_by_ec->isChecked() && enable_eta && cb_active_eta->isChecked() && comparative->ce_eta.buckets);
      cb_store_reference_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
      cb_store_diff_eta->setEnabled(enable_eta && cb_active_eta->isChecked());
      cb_store_abs_diff_eta->setEnabled(enable_eta && cb_active_eta->isChecked() && !cb_rank->isChecked());
      
      pb_process_csv->setEnabled(
                                 any_selected_selected() &&
                                 ( !comparative->by_ec || ec_ready() ) &&
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
}

void US_Hydrodyn_Comparative::update_lb_loaded_enables()
{
   if ( updates_enabled )
   {
      cout << "update_lb_loaded_enables\n";
      pb_loaded_select_all->setEnabled(lb_loaded->count());
      bool any_selected = any_loaded_selected();
      pb_loaded_set_ranges->setEnabled(any_selected && any_params_enabled());
      pb_loaded_merge->setEnabled(any_selected && !one_loaded_selected());
      pb_loaded_view->setEnabled(any_selected);
      pb_loaded_remove->setEnabled(any_selected);
   }
}
      
void US_Hydrodyn_Comparative::update_lb_selected_enables()
{
   if ( updates_enabled )
   {
      cout << "update_lb_selected_enables\n";
      pb_selected_select_all->setEnabled(lb_selected->count());
      bool any_selected = any_selected_selected();
      pb_selected_merge->setEnabled(any_selected && !one_selected_selected());
      pb_selected_set_ranges->setEnabled(any_selected && any_params_enabled());
      pb_selected_remove->setEnabled(any_selected);
      pb_process_csv->setEnabled(
                                 any_selected &&
                                 ( !comparative->by_ec || ec_ready() ) &&
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
}

void US_Hydrodyn_Comparative::disable_updates()
{
   updates_enabled = false;

   // extra disables (be sure to enable these in enable_updates())
   lb_loaded->setEnabled(false);
   lb_selected->setEnabled(false);
   pb_load_param->setEnabled(false);
   pb_reset_param->setEnabled(false);
   pb_save_param->setEnabled(false);
   pb_load_csv->setEnabled(false);
   pb_process_csv->setEnabled(false);
   pb_save_csv->setEnabled(false);

   // standard list for update_lb_loaded_enables()
   pb_loaded_select_all->setEnabled(false);
   pb_loaded_view->setEnabled(false);
   pb_loaded_merge->setEnabled(false);
   pb_loaded_set_ranges->setEnabled(false);
   pb_loaded_remove->setEnabled(false);

   // standard list for update_lb_selected_enables()
   pb_selected_select_all->setEnabled(false);
   pb_selected_merge->setEnabled(false);
   pb_selected_set_ranges->setEnabled(false);
   pb_selected_remove->setEnabled(false);

   // standard list from update_enables():

   cb_active_s->setEnabled(false);
   le_target_s->setEnabled(false);
   le_rank_s->setEnabled(false);
   cb_include_in_weight_s->setEnabled(false);
   le_weight_s->setEnabled(false);
   le_buckets_s->setEnabled(false);
   le_min_s->setEnabled(false);
   le_max_s->setEnabled(false);
   cb_store_reference_s->setEnabled(false);
   cb_store_diff_s->setEnabled(false);
   cb_store_abs_diff_s->setEnabled(false);
      
   cb_active_D->setEnabled(false);
   le_target_D->setEnabled(false);
   le_rank_D->setEnabled(false);
   cb_include_in_weight_D->setEnabled(false);
   le_weight_D->setEnabled(false);
   le_buckets_D->setEnabled(false);
   le_min_D->setEnabled(false);
   le_max_D->setEnabled(false);
   cb_store_reference_D->setEnabled(false);
   cb_store_diff_D->setEnabled(false);
   cb_store_abs_diff_D->setEnabled(false);
      
   cb_active_sr->setEnabled(false);
   le_target_sr->setEnabled(false);
   le_rank_sr->setEnabled(false);
   cb_include_in_weight_sr->setEnabled(false);
   le_weight_sr->setEnabled(false);
   le_buckets_sr->setEnabled(false);
   le_min_sr->setEnabled(false);
   le_max_sr->setEnabled(false);
   cb_store_reference_sr->setEnabled(false);
   cb_store_diff_sr->setEnabled(false);
   cb_store_abs_diff_sr->setEnabled(false);
      
   cb_active_fr->setEnabled(false);
   le_target_fr->setEnabled(false);
   le_rank_fr->setEnabled(false);
   cb_include_in_weight_fr->setEnabled(false);
   le_weight_fr->setEnabled(false);
   le_buckets_fr->setEnabled(false);
   le_min_fr->setEnabled(false);
   le_max_fr->setEnabled(false);
   cb_store_reference_fr->setEnabled(false);
   cb_store_diff_fr->setEnabled(false);
   cb_store_abs_diff_fr->setEnabled(false);
      
   cb_active_rg->setEnabled(false);
   le_target_rg->setEnabled(false);
   le_rank_rg->setEnabled(false);
   cb_include_in_weight_rg->setEnabled(false);
   le_weight_rg->setEnabled(false);
   le_buckets_rg->setEnabled(false);
   le_min_rg->setEnabled(false);
   le_max_rg->setEnabled(false);
   cb_store_reference_rg->setEnabled(false);
   cb_store_diff_rg->setEnabled(false);
   cb_store_abs_diff_rg->setEnabled(false);
      
   cb_active_tau->setEnabled(false);
   le_target_tau->setEnabled(false);
   le_rank_tau->setEnabled(false);
   cb_include_in_weight_tau->setEnabled(false);
   le_weight_tau->setEnabled(false);
   le_buckets_tau->setEnabled(false);
   le_min_tau->setEnabled(false);
   le_max_tau->setEnabled(false);
   cb_store_reference_tau->setEnabled(false);
   cb_store_diff_tau->setEnabled(false);
   cb_store_abs_diff_tau->setEnabled(false);
      
   cb_active_eta->setEnabled(false);
   le_target_eta->setEnabled(false);
   le_rank_eta->setEnabled(false);
   cb_include_in_weight_eta->setEnabled(false);
   le_weight_eta->setEnabled(false);
   le_buckets_eta->setEnabled(false);
   le_min_eta->setEnabled(false);
   le_max_eta->setEnabled(false);
   cb_store_reference_eta->setEnabled(false);
   cb_store_diff_eta->setEnabled(false);
   cb_store_abs_diff_eta->setEnabled(false);
      
   pb_process_csv->setEnabled(false);
   pb_save_csv->setEnabled(false);
}

void US_Hydrodyn_Comparative::enable_updates()
{
   updates_enabled = true;

   lb_loaded->setEnabled(true);
   lb_selected->setEnabled(true);
   pb_load_param->setEnabled(true);
   pb_reset_param->setEnabled(true);
   pb_save_param->setEnabled(true);
   pb_load_csv->setEnabled(true);
   // pb_process_csv->setEnabled(true);
   // pb_save_csv->setEnabled(true);

   update_lb_loaded_enables();
   update_lb_selected_enables();
   update_enables();
}

void US_Hydrodyn_Comparative::set_by_pct()
{
   comparative->by_pct = cb_by_pct->isChecked();
   update_enables();
}

void US_Hydrodyn_Comparative::set_by_ec()
{
   comparative->by_ec = cb_by_ec->isChecked();

   comparative->rank = false;
   cb_rank->setChecked(false);

   comparative->weight_controls = !cb_by_ec->isChecked();
   cb_weight_controls->setChecked(!cb_by_ec->isChecked());

   update_enables();
}

void US_Hydrodyn_Comparative::set_rank()
{
   comparative->rank = cb_rank->isChecked();
   comparative->weight_controls = !cb_rank->isChecked();
   cb_weight_controls->setChecked(comparative->weight_controls);

   if ( comparative->rank && comparative->by_ec )
   {
      comparative->by_ec = false;
      cb_by_ec->setChecked(false);
   }

   if ( cb_rank->isChecked() )
   {
      cb_store_abs_diff_s->setChecked(true);
      comparative->ce_s.store_abs_diff = true;
      cb_store_abs_diff_D->setChecked(true);
      comparative->ce_D.store_abs_diff = true;
      cb_store_abs_diff_sr->setChecked(true);
      comparative->ce_sr.store_abs_diff = true;
      cb_store_abs_diff_fr->setChecked(true);
      comparative->ce_fr.store_abs_diff = true;
      cb_store_abs_diff_rg->setChecked(true);
      comparative->ce_rg.store_abs_diff = true;
      cb_store_abs_diff_tau->setChecked(true);
      comparative->ce_tau.store_abs_diff = true;
      cb_store_abs_diff_eta->setChecked(true);
      comparative->ce_eta.store_abs_diff = true;
   }
   update_enables();
}

void US_Hydrodyn_Comparative::set_weight_controls()
{
   comparative->weight_controls = cb_weight_controls->isChecked();
   comparative->rank = !cb_weight_controls->isChecked();
   cb_rank->setChecked(comparative->rank);
   if ( comparative->weight_controls && comparative->by_ec )
   {
      comparative->by_ec = false;
      cb_by_ec->setChecked(false);
   }
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
   if ( comparative->ce_s.weight != 0.0 &&
        !comparative->ce_s.include_in_weight )
   {
      cb_include_in_weight_s->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_s(const QString &val)
{
   comparative->ce_s.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_s(const QString &val)
{
   comparative->ce_s.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_s(const QString &val)
{
   comparative->ce_s.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_D.weight != 0.0 &&
        !comparative->ce_D.include_in_weight )
   {
      cb_include_in_weight_D->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_D(const QString &val)
{
   comparative->ce_D.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_D(const QString &val)
{
   comparative->ce_D.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_D(const QString &val)
{
   comparative->ce_D.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_sr.weight != 0.0 &&
        !comparative->ce_sr.include_in_weight )
   {
      cb_include_in_weight_sr->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_sr(const QString &val)
{
   comparative->ce_sr.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_sr(const QString &val)
{
   comparative->ce_sr.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_sr(const QString &val)
{
   comparative->ce_sr.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_fr.weight != 0.0 &&
        !comparative->ce_fr.include_in_weight )
   {
      cb_include_in_weight_fr->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_fr(const QString &val)
{
   comparative->ce_fr.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_fr(const QString &val)
{
   comparative->ce_fr.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_fr(const QString &val)
{
   comparative->ce_fr.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_rg.weight != 0.0 &&
        !comparative->ce_rg.include_in_weight )
   {
      cb_include_in_weight_rg->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_rg(const QString &val)
{
   comparative->ce_rg.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_rg(const QString &val)
{
   comparative->ce_rg.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_rg(const QString &val)
{
   comparative->ce_rg.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_tau.weight != 0.0 &&
        !comparative->ce_tau.include_in_weight )
   {
      cb_include_in_weight_tau->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_tau(const QString &val)
{
   comparative->ce_tau.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_tau(const QString &val)
{
   comparative->ce_tau.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_tau(const QString &val)
{
   comparative->ce_tau.max = val.toDouble();
   update_enables();
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
   if ( comparative->ce_eta.weight != 0.0 &&
        !comparative->ce_eta.include_in_weight )
   {
      cb_include_in_weight_eta->setChecked(true);
      update_enables();
   }
}

void US_Hydrodyn_Comparative::update_buckets_eta(const QString &val)
{
   comparative->ce_eta.buckets = val.toInt();
   update_enables();
}

void US_Hydrodyn_Comparative::update_min_eta(const QString &val)
{
   comparative->ce_eta.min = val.toDouble();
   update_enables();
}

void US_Hydrodyn_Comparative::update_max_eta(const QString &val)
{
   comparative->ce_eta.max = val.toDouble();
   update_enables();
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
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_param;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString fname = QFileDialog::getOpenFileName( this , us_tr("Choose a filename to load the parameters") , use_dir , "*.smp" );

   if ( fname.isEmpty() )
   {
      return;
   }

   comparative->path_param = QFileInfo(fname).absolutePath();

   QFile f(fname);

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for reading! (permissions?)")).arg(fname) );
      return;
   }

   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( f.fileName() );

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
   editor->append(QString(us_tr("Loaded parameter file: %1\n")).arg(fname));
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
                                us_tr("Reset Parameters"),
                                us_tr("Are you sure you want to reset the parameters?"),
                                us_tr("&Yes"), us_tr("&No"),
                                QString(), 0, 1 ) 
          ) 
      {
         return;
      }
      *comparative = ci;
      refresh();
      editor->append(us_tr("Parameters reset\n"));
   }
}

void US_Hydrodyn_Comparative::save_param()
{
   //   cout << serialize_comparative_info(*comparative);
   QString use_dir = 
      comparative->path_param.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_param;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the parameters") , use_dir , "*.smp" );

   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".smp$", Qt::CaseInsensitive )) )
   {
      fname += ".smp";
   }

   comparative->path_param = QFileInfo(fname).absolutePath();
   
   if ( QFile::exists(fname) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }

   QFile f(fname);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(fname) );
      return;
   }
   ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( f.fileName() );

   QTextStream t( &f );
   t << serialize_comparative_info( *comparative );
   f.close();
   editor->append(QString(us_tr("Saved parameter file: %1\n")).arg(fname));
}

void US_Hydrodyn_Comparative::load_csv()
{
   QString use_dir = 
      comparative->path_csv.isEmpty() ?
      USglobal->config_list.root_dir + "/" + "somo" :
      comparative->path_csv;

   ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history( use_dir, this );

   QStringList filenames = QFileDialog::getOpenFileNames( this , "Open" , use_dir , "csv files (*.csv)" );


   if ( filenames.empty() )
   {
      return;
   }

   comparative->path_csv = QFileInfo(filenames[0]).absolutePath();


   for ( QStringList::iterator it = filenames.begin();
         it != filenames.end();
         it++ )
   {
      ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history( *it );
      csv tmp_csv = csv_read(*it);
      if ( !csv_error.isEmpty() )
      {
         if ( csv_error != us_tr("alread loaded") )
         {
            editor_msg("red", QString("%1: %2").arg(*it).arg(csv_error));
         }
      } else {
         csvs[*it] = tmp_csv;
         loaded_csv_names[tmp_csv.name] = true;
         lb_loaded->addItem(tmp_csv.name);
         if ( !csv_warn.isEmpty() )
         {
            QMessageBox::warning( this, "UltraScan",
                                  QString(us_tr("Loading the csv %1 produced the following warnings:\n%2"))
                                  .arg(*it)
                                  .arg(csv_warn) 
                                  );
            editor_msg("dark red", QString("loaded with warnings: %1\n").arg(*it));
         } else {
            editor->append(QString("loaded: %1\n").arg(*it));
         }
         // cout << csv_info(tmp_csv);
      }
   }
   update_lb_loaded_enables();
}

void US_Hydrodyn_Comparative::update_loaded()
{
   // clear lb_selected entries
   // maybe we want to be a bit more sophisticated with this
   //  i.e. doing a "delta" to only update the changes
   lb_selected->clear( );
   
   // add selected loaded entries to selected
   for ( int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         if ( csvs.count(lb_loaded->item(i)->text()) )
         {
            lb_selected->addItems(csv_model_names(csvs[ lb_loaded->item(i)->text() ]));
         } else {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
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
   for ( int i = 0; i < lb_loaded->count(); i++ )
   {
      if ( !lb_loaded->item(i)->isSelected() )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_loaded->count(); i++ )
   {
      lb_loaded->item(i)->setSelected( select_all);
   }
   if ( select_all )
   {
      lb_loaded->scrollToItem( lb_loaded->item(lb_loaded->count() - 1) );
   }
}

void US_Hydrodyn_Comparative::loaded_set_ranges()
{
   if ( !any_loaded_selected() )
   {
      editor_msg("red", us_tr("internal error: loaded set exp min/max called but no item selected!\n"));
      return;
   }
      
   double min;
   double max;

   if ( all_selected_csv_contain( comparative->ce_s ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_s ) )
      {
         comparative->ce_s.min = min;
         comparative->ce_s.max = max;
         le_min_s->setText(QString("%1").arg(comparative->ce_s.min));
         le_max_s->setText(QString("%1").arg(comparative->ce_s.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_D ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_D ) )
      {
         comparative->ce_D.min = min;
         comparative->ce_D.max = max;
         le_min_D->setText(QString("%1").arg(comparative->ce_D.min));
         le_max_D->setText(QString("%1").arg(comparative->ce_D.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_sr ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_sr ) )
      {
         comparative->ce_sr.min = min;
         comparative->ce_sr.max = max;
         le_min_sr->setText(QString("%1").arg(comparative->ce_sr.min));
         le_max_sr->setText(QString("%1").arg(comparative->ce_sr.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_fr ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_fr ) )
      {
         comparative->ce_fr.min = min;
         comparative->ce_fr.max = max;
         le_min_fr->setText(QString("%1").arg(comparative->ce_fr.min));
         le_max_fr->setText(QString("%1").arg(comparative->ce_fr.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_rg ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_rg ) )
      {
         comparative->ce_rg.min = min;
         comparative->ce_rg.max = max;
         le_min_rg->setText(QString("%1").arg(comparative->ce_rg.min));
         le_max_rg->setText(QString("%1").arg(comparative->ce_rg.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_tau ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_tau ) )
      {
         comparative->ce_tau.min = min;
         comparative->ce_tau.max = max;
         le_min_tau->setText(QString("%1").arg(comparative->ce_tau.min));
         le_max_tau->setText(QString("%1").arg(comparative->ce_tau.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_eta ) )
   {
      if ( csv_get_loaded_min_max(min, max, comparative->ce_eta ) )
      {
         comparative->ce_eta.min = min;
         comparative->ce_eta.max = max;
         le_min_eta->setText(QString("%1").arg(comparative->ce_eta.min));
         le_max_eta->setText(QString("%1").arg(comparative->ce_eta.max));
      }
   }
}

void US_Hydrodyn_Comparative::loaded_view()
{
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() ) 
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
         } else {
            US_Hydrodyn_Csv_Viewer 
               *csv_viewer_window =
               new US_Hydrodyn_Csv_Viewer(csvs[ lb_loaded->item(i)->text() ], this);
            csv_viewer_window->show();
         }
      }
   }
}

void US_Hydrodyn_Comparative::loaded_merge()
{
   if ( !csv_premerge_column_warning_all_loaded_selected() )
   {
      if (
          QMessageBox::warning(
                               this,
                               us_tr("Merge CSV's"),
                               QString(
                                       us_tr("The CSVs do not all have the same column names.\n"
                                          "This will create additional columns with blank entries\n"
                                          "for rows from CSVs without these extra columns:\n"
                                          "\n%1\n\n"
                                          "This will effect statistics computed on these extra columns.\n"
                                          "Do you still want to merge them?")
                                       ).arg(csv_premerge_missing_header_qsl.join("\n")),
                               us_tr("&Yes"), us_tr("&No"),
                               QString(), 0, 1 )
          )
      {
         return;
      }
   }
         
   csv_merge_loaded_selected();
}

void US_Hydrodyn_Comparative::csv_remove( QString name, int i )
{
   // cout << QString("csv_remove %1 %2\n").arg(name).arg(i);
   // cout << loaded_info();
   if ( csvs.count(name) )
   {
      for ( unsigned int j = 0; j < csvs[ name ].prepended_names.size(); j++ )
      {
         // cout << QString(" name: %1|%2\n").arg(csvs[ name ].name).arg(csvs[ name ].prepended_names[j]);
         map < QString, bool >::iterator it = 
            loaded_csv_row_prepended_names.find(csvs[ name ].name + 
                                                "|" + 
                                                csvs[ name ].prepended_names[j]);
         loaded_csv_row_prepended_names.erase(it);
      }
      map < QString, bool >::iterator it = loaded_csv_names.find(name);
      loaded_csv_names.erase(it);
      map < QString, csv >::iterator it2 = csvs.find(name);
      csvs.erase(it2);
   } else {
      editor_msg("red", QString(us_tr("internal error: csv_remove %1 could not find csv to remove!")).arg(name));
   }
   delete lb_loaded->takeItem(i);
}

void US_Hydrodyn_Comparative::loaded_remove()
{
   disable_updates();
   for ( int i = lb_loaded->count() - 1; i >= 0; i-- )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         csv_remove(lb_loaded->item(i)->text(), i);
      }
   }
   enable_updates();
}


void US_Hydrodyn_Comparative::set_loaded_csv_row_prepended_names( csv &csv1 )
{
   for ( unsigned int i = 0; i < csv1.prepended_names.size(); i++ )
   {
      if ( loaded_csv_row_prepended_names.count(csv1.name + 
                                                "|" + 
                                                csv1.prepended_names[i]) )
      {
         editor_msg("red", us_tr("internal error: duplicate loaded prepended names found!"));
      } else {
         loaded_csv_row_prepended_names[csv1.name + 
                                        "|" + 
                                        csv1.prepended_names[i]] = true;
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
   for ( int i = 0; i < lb_selected->count(); i++ )
   {
      if ( !lb_selected->item(i)->isSelected() )
      {
         select_all = true;
         break;
      }
   }

   for ( int i = 0; i < lb_selected->count(); i++ )
   {
      lb_selected->item(i)->setSelected( select_all);
   }
   if ( select_all )
   {
      lb_selected->scrollToItem( lb_selected->item(lb_selected->count() - 1) );
   }
}

void US_Hydrodyn_Comparative::selected_merge()
{
   csv csv_merged;
   if ( csv_merge_selected_selected(csv_merged) )
   {
      for ( int i = 0; i < lb_loaded->count(); i++ )
      {
         lb_loaded->item(i)->setSelected( false);
      }
      
      csv_merged.name = get_unique_csv_name(csv_merged.name);
      csvs[ csv_merged.name ] = csv_merged;
      loaded_csv_names[csv_merged.name] = true;
      set_loaded_csv_row_prepended_names( csv_merged );
      
      lb_loaded->addItem(csv_merged.name);
      lb_loaded->item(lb_loaded->count() - 1)->setSelected( true);
      lb_loaded->scrollToItem( lb_loaded->item(lb_loaded->count() - 1) );
      // I don't think these next 2 lines should be needed, but sometimes
      // if a scrollbar is created, this keeps it from overwriting the bottom item:
      lb_loaded->setCurrentItem( lb_loaded->item(lb_loaded->count() - 1) );
      lb_loaded->scrollToItem( lb_loaded->currentItem() );
      
      editor->append(QString(us_tr("CSVs merged: %1\n")).arg(csv_merged.name));
      
      for ( int i = 0; i < lb_selected->count(); i++ )
      {
         lb_selected->item(i)->setSelected( true);
      }
   }
   enable_updates();
}

void US_Hydrodyn_Comparative::selected_set_ranges()
{
   if ( !any_selected_selected() )
   {
      editor_msg("red", us_tr("internal error: loaded set exp min/max called but no item selected!\n"));
      return;
   }
      
   double min;
   double max;

   if ( all_selected_csv_contain( comparative->ce_s ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_s ) )
      {
         comparative->ce_s.min = min;
         comparative->ce_s.max = max;
         le_min_s->setText(QString("%1").arg(comparative->ce_s.min));
         le_max_s->setText(QString("%1").arg(comparative->ce_s.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_D ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_D ) )
      {
         comparative->ce_D.min = min;
         comparative->ce_D.max = max;
         le_min_D->setText(QString("%1").arg(comparative->ce_D.min));
         le_max_D->setText(QString("%1").arg(comparative->ce_D.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_sr ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_sr ) )
      {
         comparative->ce_sr.min = min;
         comparative->ce_sr.max = max;
         le_min_sr->setText(QString("%1").arg(comparative->ce_sr.min));
         le_max_sr->setText(QString("%1").arg(comparative->ce_sr.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_fr ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_fr ) )
      {
         comparative->ce_fr.min = min;
         comparative->ce_fr.max = max;
         le_min_fr->setText(QString("%1").arg(comparative->ce_fr.min));
         le_max_fr->setText(QString("%1").arg(comparative->ce_fr.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_rg ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_rg ) )
      {
         comparative->ce_rg.min = min;
         comparative->ce_rg.max = max;
         le_min_rg->setText(QString("%1").arg(comparative->ce_rg.min));
         le_max_rg->setText(QString("%1").arg(comparative->ce_rg.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_tau ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_tau ) )
      {
         comparative->ce_tau.min = min;
         comparative->ce_tau.max = max;
         le_min_tau->setText(QString("%1").arg(comparative->ce_tau.min));
         le_max_tau->setText(QString("%1").arg(comparative->ce_tau.max));
      }
   }
   if ( all_selected_csv_contain( comparative->ce_eta ) )
   {
      if ( csv_get_selected_min_max(min, max, comparative->ce_eta ) )
      {
         comparative->ce_eta.min = min;
         comparative->ce_eta.max = max;
         le_min_eta->setText(QString("%1").arg(comparative->ce_eta.min));
         le_max_eta->setText(QString("%1").arg(comparative->ce_eta.max));
      }
   }
}

void US_Hydrodyn_Comparative::selected_remove()
{
   for ( int i = lb_selected->count() - 1; i >= 0; i-- )
   {
      if ( lb_selected->item(i)->isSelected() )
      {
         delete lb_selected->takeItem(i);
      }
   }
}

void US_Hydrodyn_Comparative::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor( color );
   editor->append( msg );
   editor->setTextColor( save_color );
}

void US_Hydrodyn_Comparative::process_csv()
{
   csv csv_processed;
   if ( !csv_merge_selected_selected(csv_processed) )
   {
      enable_updates();
      // editor_msg("red", us_tr("internal error: should not get here without anything selected!"));
      return;
   }

   csv_processed.name.replace(QRegExp("^selected_rows_from_"),"");
   csv_processed.name.replace(QRegExp("^Processed_"),"");
   csv_processed.name = "Processed_" + csv_processed.name;
   if ( !csv_process(csv_processed) )
   {
      editor_msg("red", csv_error);
   }

   for ( int i = 0; i < lb_loaded->count(); i++ )
   {
      lb_loaded->item(i)->setSelected( false);
   }

   csv_processed.name = get_unique_csv_name(csv_processed.name);
   csvs[ csv_processed.name ] = csv_processed;
   loaded_csv_names[csv_processed.name] = true;
   set_loaded_csv_row_prepended_names( csv_processed );

   lb_loaded->addItem(csv_processed.name);
   lb_loaded->item(lb_loaded->count() - 1)->setSelected( true);
   lb_loaded->scrollToItem( lb_loaded->item(lb_loaded->count() - 1) );
   // I don't think these next 2 lines should be needed, but sometimes
   // if a scrollbar is created, this keeps it from overwriting the bottom item:
   lb_loaded->setCurrentItem( lb_loaded->item(lb_loaded->count() - 1) );
   lb_loaded->scrollToItem( lb_loaded->currentItem() );

   editor->append(QString(us_tr("Processed into: %1\n")).arg(csv_processed.name));
   enable_updates();
}

void US_Hydrodyn_Comparative::save_csv()
{
   if ( any_loaded_selected() && !one_loaded_selected() )
   {
      if ( 
          QMessageBox::question(
                                this,
                                us_tr("Merge CSV's"),
                                us_tr("Multiple CSV are selected and must be merged before saving\n"
                                   "Do you want to merge them?"),
                                us_tr("&Yes"), us_tr("&No"),
                                QString(), 0, 1 ) 
          ) 
      {
         return;
      }
      if ( !csv_premerge_column_warning_all_loaded_selected() )
      {
         if (
             QMessageBox::warning(
                                  this,
                                  us_tr("Merge CSV's"),
                                  QString(
                                          us_tr("The CSVs do not all have the same column names.\n"
                                             "This will create additional columns with blank entries\n"
                                             "for rows from CSVs without these extra columns:\n"
                                             "\n%1\n\n"
                                             "This will effect statistics computed on these extra columns.\n"
                                             "Do you still want to merge them?")
                                          ).arg(csv_premerge_missing_header_qsl.join("\n")),
                                  us_tr("&Yes"), us_tr("&No"),
                                  QString(), 0, 1 )
             )
         {
            return;
         }
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
      editor_msg("red", us_tr("internal error: could not find csv for saving!"));
      return;
   }
   if ( !csvs.count(sel_name) )
   {
      editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(sel_name));
      return;
   }

   QString use_name = sel_name.isEmpty() ? "*.csv" : sel_name;
   if ( !use_name.contains(QRegExp(".csv$", Qt::CaseInsensitive )) )
   {
      use_name += ".csv";
   }

   if ( use_name == QFileInfo(use_name).fileName() )
   {
      // cout << QString(" use_name <%1> fi <%2>\n").arg(use_name).arg(QFileInfo(use_name).fileName());
      use_name = use_dir + QDir::separator() + use_name;
   }
   // cout << "use_name: " << use_name << "\n";

   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the parameters") , use_name , "*.csv" );

   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".csv$", Qt::CaseInsensitive )) )
   {
      fname += ".csv";
   }

   comparative->path_csv = QFileInfo(fname).absolutePath();

   csv_write( fname, csvs[ sel_name ] );
}

void US_Hydrodyn_Comparative::save()
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

void US_Hydrodyn_Comparative::print()
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      //  QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > printer.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     printer.width(), fm.lineSpacing(),
                                   ExpandTabs | DontClip,
                                   editor->toPlainText( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn_Comparative::clear_display()
{
   editor->clear( );
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
   cb_by_pct->setChecked(comparative->by_pct);
   cb_rank->setChecked(comparative->rank);
   cb_weight_controls->setChecked(comparative->weight_controls);
   cb_by_ec->setChecked(comparative->by_ec);
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
   csv_warn = "";

   if ( filename.isEmpty() )
   {
      csv_error = us_tr("csv read called with empty filename");
      return csv1;
   }

   QFile f(filename);

   if ( !f.exists() )
   {
      csv_error = QString(us_tr("File %1 does not exist")).arg(f.fileName());
      return csv1;
   }

   if ( !f.open(QIODevice::ReadOnly) )
   {
      csv_error = QString(us_tr("Can not open file %1.  Check permissions")).arg(f.fileName());
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
   if ( loaded_csv_names.count(filename) )
   {
      if (
          QMessageBox::warning(
                               this,
                               us_tr("Read CSV"),
                               QString(
                                       us_tr("The CSV named %1 is already loaded\nDo you want to reload it?")
                                       ).arg(filename),
                               us_tr("&Yes"), us_tr("&No"),
                               QString(), 0, 1 )
          )
      {
         csv_error = us_tr("already loaded");
         return csv1;
      }
      // remove old load
      bool removed_one = false;
      for ( int i = 0; i < lb_loaded->count(); i++ )
      {
         if ( lb_loaded->item(i)->text() == filename )
         {
            removed_one = true;
            csv_remove(filename, i);
            break;
         }
      }
      if ( !removed_one )
      {
         editor_msg("red", QString(us_tr("internal error: could not find %1 in loaded list.")).arg(filename));
      }
   }

   int i = 0;
   QStringList qsl_h = csv_parse_line(qsl[0]);

   for ( QStringList::iterator it = qsl_h.begin();
         it != qsl_h.end();
         it++ )
   {
      QString qs = *it;
      qs.replace("\"","");
      if ( csv1.header_map.count(qs) )
      {
         csv_error = QString(us_tr("Duplicate header name \"%1\" found in file %2")).arg(qs).arg(f.fileName());
         return csv1;
      }
      csv1.header_map[qs] = i++;
      csv1.header.push_back(qs);
   }

   unsigned int row = 1;
   {
      QStringList::iterator it = qsl.begin();
      it++;
      for ( ;
            it != qsl.end();
            it++ )
      {
         row++;
         QStringList qsl_d = csv_parse_line(*it);
         vector < QString > vqs;
         vector < double > vd;
         if ( qsl_d.size() )
         {
            for ( QStringList::iterator it2 = qsl_d.begin();
                  it2 != qsl_d.end();
                  it2++ )
            {
               if ( vd.size() >= csv1.header.size() )
               {
                  csv_warn += QString(us_tr("%1Row %2 has more columns than the header, these columns are skipped"))
                     .arg(csv_warn.isEmpty() ? "" : "\n").arg(row);
                  break;
               }
               vqs.push_back(*it2);
               vd.push_back((*it2).toDouble());
            }
            if ( vd.size() < csv1.header.size() )
            {
               csv_warn += QString(us_tr("%1Row %2 has insufficient columns, skipped"))
                  .arg(csv_warn.isEmpty() ? "" : "\n").arg(row);
            } else {
               csv1.data.push_back(vqs);
               csv1.num_data.push_back(vd);
            }
         }
      }
   }

   if ( !csv1.header_map.count("Model name") )
   {
      csv_error = us_tr("no \"Model name\" header found in csv");
      return csv1;
   }

   if ( !csv1.data.size() )
   {
      csv_error = us_tr("no data lines found in csv");
      return csv1;
   }

   QString qs_prepend = QFileInfo(csv1.name).completeBaseName() + ": ";

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      unsigned inc = 0;
      while ( loaded_csv_row_prepended_names.count(
                                                   csv1.name + 
                                                   "|" + 
                                                   qs_prepend + 
                                                   csv1.data[i][csv1.header_map["Model name"]] +
                                                   QString("%1").arg(inc ? QString("-%1").arg(inc) : "" )
                                                   )
              )
      {
         inc++;
      }
      
      loaded_csv_row_prepended_names[
                                     csv1.name + 
                                     "|" + 
                                     qs_prepend + 
                                     csv1.data[i][csv1.header_map["Model name"]] +
                                     QString("%1").arg(inc ? QString("-%1").arg(inc) : "" ) ] = true;
              
      csv1.prepended_names.push_back(
                                     qs_prepend + 
                                     csv1.data[i][csv1.header_map["Model name"]] +
                                     QString("%1").arg(inc ? QString("-%1").arg(inc) : "" )
                                     );
   }
      
   return csv1;
}

QString US_Hydrodyn_Comparative::csv_info( csv &csv1 )
{
   QString qs = 
      QString(
              "csv_info for %1:\n"
              " headers_map: %2\n"
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
      qs += "  " + QString("%1 %2\n").arg(i).arg(csv1.header[i]);
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
      qs += QString("  row %1 data columns %2  num_data columns %3\n")
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
      csv_error = us_tr("internal error: csv_merge called with zero data csv");
      return csv_merge;
   }

   if ( !csv_merge.name.contains("+") )
   {
      csv_merge.name = QFileInfo(csv_merge.name).completeBaseName();
   }
   csv_merge.name += "+" + QFileInfo(csv2.name).completeBaseName();


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

   for ( unsigned int i = 0; i < (unsigned int)lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         if ( csvs.count(lb_loaded->item(i)->text()) )
         {
            if ( !csvs[ lb_loaded->item(i)->text() ].header_map.count(ce.name) )
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
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
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
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
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
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         qs = lb_loaded->item(i)->text();
         break;
      }
   }
   return qs;
}

bool US_Hydrodyn_Comparative::any_selected_selected()
{
   bool any_selected = false;
   for ( unsigned int i = 0; i < (unsigned int) lb_selected->count(); i++ )
   {
      if ( lb_selected->item(i)->isSelected() )
      {
         any_selected = true;
         break;
      }
   }
   return any_selected;
}

bool US_Hydrodyn_Comparative::one_selected_selected()
{
   int no_selected = 0;
   for ( unsigned int i = 0; i < (unsigned int) lb_selected->count(); i++ )
   {
      if ( lb_selected->item(i)->isSelected() )
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

void US_Hydrodyn_Comparative::csv_write( QString filename, csv &csv1 )
{
   if ( QFile::exists(filename) )
   {
      filename = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( filename, 0, this );
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }
   QTextStream t( &f );
   QString qs;
   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(csv1.header[i]);
   }
   t << qs << Qt::endl;
   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(csv1.data[i][j]);
      }
      t << qs << Qt::endl;
   }
   f.close();
   editor->append(QString(us_tr("Saved csv file: %1\n")).arg(filename));
}

void US_Hydrodyn_Comparative::csv_merge_loaded_selected() 
{
   if ( !any_loaded_selected() || one_loaded_selected() )
   {
      // nothing to do here
      return;
   }

   disable_updates();

   if ( !csvs.count(first_loaded_selected()) )
   {
      editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(first_loaded_selected()));
      return;
   }
   
   csv csv_merged = csvs[ first_loaded_selected() ];
   bool skip_first = true;
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( !skip_first && lb_loaded->item(i)->isSelected() ) 
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
            enable_updates();
            return;
         }
         csv_merged = csv_merge(csv_merged, csvs[ lb_loaded->item(i)->text() ]);
      }

      if ( skip_first && lb_loaded->item(i)->isSelected() ) 
      {
         skip_first = false;
      }
   }
   for ( int i = 0; i < lb_loaded->count(); i++ )
   {
      lb_loaded->item(i)->setSelected( false);
   }

   csv_merged.name = get_unique_csv_name(csv_merged.name);
   csvs[ csv_merged.name ] = csv_merged;
   loaded_csv_names[csv_merged.name] = true;
   set_loaded_csv_row_prepended_names( csv_merged );

   lb_loaded->addItem(csv_merged.name);
   lb_loaded->item(lb_loaded->count() - 1)->setSelected( true);
   lb_loaded->scrollToItem( lb_loaded->item(lb_loaded->count() - 1) );
   // I don't think these next 2 lines should be needed, but sometimes
   // if a scrollbar is created, this keeps it from overwriting the bottom item:
   lb_loaded->setCurrentItem( lb_loaded->item(lb_loaded->count() - 1) );
   lb_loaded->scrollToItem( lb_loaded->currentItem() );

   editor->append(QString(us_tr("CSVs merged: %1\n")).arg(csv_merged.name));

   enable_updates();
}

QString US_Hydrodyn_Comparative::get_unique_csv_name( QString name )
{
   unsigned int inc = 0;
   while ( loaded_csv_names.count(name + ( inc ? QString("-%1").arg(inc) : "" )) )
   {
      inc++;
   }
   return name + (inc ? QString("-%1").arg(inc) : "");
}
   
bool US_Hydrodyn_Comparative::csv_merge_selected_selected( csv &csv_merged ) 
{
   disable_updates();

   if ( !any_selected_selected() )
   {
      // nothing to do here
      return false;
   }

   map < QString, bool > selected_names;

   // create a map of the selected selected names
   for ( unsigned int i = 0; i < (unsigned int) lb_selected->count(); i++ )
   {
      if ( lb_selected->item(i)->isSelected() ) 
      {
         selected_names[ lb_selected->item(i)->text() ] = true;
      }
   }
   
   map < QString, bool > selected_models;
   map < QString, bool > csv_used;
   vector < QString > csv_used_vector;
   map < QString, int > csv_used_models;
   QString last_csv;

   // now find the csv's that go with them to make our csv list
   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() ) 
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(first_loaded_selected()));
            enable_updates();
            return false;
         }
         for ( unsigned int j = 0; j < csvs[ lb_loaded->item(i)->text() ].prepended_names.size(); j++ )
         {
            if ( selected_names.count(csvs[ lb_loaded->item(i)->text() ].prepended_names[j]) )
            {
               if ( !csv_used.count(lb_loaded->item(i)->text()) )
               {
                  last_csv = lb_loaded->item(i)->text();
                  csv_used[ lb_loaded->item(i)->text() ] = true;
                  csv_used_vector.push_back(lb_loaded->item(i)->text());
               }
               csv_used_models[ lb_loaded->item(i)->text() ]++;
               selected_models[csvs[ lb_loaded->item(i)->text() ].prepended_names[j]] = true;
            }
         }
      }
   }

   if ( csv_used_vector.size() == 0 )
   {
      return false;
   }

   // csv csv_merged;

   csv *ref_csv = &csvs[ csv_used_vector[0] ];
   csv_merged.name.replace(QRegExp("^selected_rows_from_"),"");
   csv_merged.name = "selected_rows_from_" + QFileInfo(ref_csv->name).completeBaseName();
   csv_merged.header_map = ref_csv->header_map;
   csv_merged.header = ref_csv->header;

   for ( unsigned int i = 0; i < ref_csv->prepended_names.size(); i++ )
   {
      if ( selected_models.count(ref_csv->prepended_names[i] ) )
      {
         csv_merged.data.push_back(ref_csv->data[i]);
         csv_merged.num_data.push_back(ref_csv->num_data[i]);
         csv_merged.prepended_names.push_back(ref_csv->prepended_names[i]);
      }
   }

   // ok, we have one selected models worth in csv_merged

   // merge any other csvs
   csv_premerge_missing_header_map.clear( );
   csv_premerge_missing_header_qsl.clear( );
   bool is_ok = true;

   for ( unsigned int i = 1; i < csv_used_vector.size(); i++ )
   {
      csv csv_to_merge;
      csv *ref_csv = &csvs[ csv_used_vector[i ]];
      csv_to_merge.name = ref_csv->name;
      csv_to_merge.header_map = ref_csv->header_map;
      csv_to_merge.header = ref_csv->header;
      for ( unsigned int j = 0; j < ref_csv->prepended_names.size(); j++ )
      {
         if ( selected_models.count(ref_csv->prepended_names[j] ) )
         {
            csv_to_merge.data.push_back(ref_csv->data[j]);
            csv_to_merge.num_data.push_back(ref_csv->num_data[j]);
            csv_to_merge.prepended_names.push_back(ref_csv->prepended_names[j]);
         }
      }
      // now merge to csv_merged
      if ( !csv_premerge_column_warning(csv_merged, csv_to_merge) )
      {
         is_ok = false;
      }
      csv_merged = csv_merge(csv_merged, csv_to_merge);
   }

   if ( !is_ok )
   {
      if (
          QMessageBox::warning(
                               this,
                               us_tr("Merge notice"),
                               QString(
                                       us_tr("The selected models do not all have the same column names.\n"
                                          "This will create additional columns with blank entries\n"
                                          "for rows from CSVs without these extra columns:\n"
                                          "\n%1\n\n"
                                          "This will effect statistics computed on these extra columns.\n"
                                          "Do you want to continue?")
                                       ).arg(csv_premerge_missing_header_qsl.join("\n")),
                               us_tr("&Yes"), us_tr("&No"),
                               QString(), 0, 1 )
          )
      {
         return false;
      }
   }

   return true;
}

bool US_Hydrodyn_Comparative::csv_process( csv &csv1 )
{
   cout << "csv_process\n";
   // cout << csv_info(csv1);
   csv_error = "";

   // for each selected ce, add the requested columns
   // remember to check for dups etc

   vector < bool > do_( ce_names.size() );
   vector < bool > ref_( ce_names.size() );
   vector < bool > diff_( ce_names.size() );
   vector < bool > abs_diff_( ce_names.size() );
   vector < bool > ec_( ce_names.size() );

   QString header_exp      = "Exp:";
   QString header_diff     = QString( comparative->by_pct ? "% " : "" ) + "Diff:";
   QString header_abs_diff = QString( comparative->by_pct ? "% " : "" ) + "AbsDiff:";
   QString header_weight   = "Weighted sum of " + QString( comparative->by_pct ? "% " : "" ) + " absolute differences";
   QString header_ec       = "Equivalence class partial rank";

   vector < double > exp_             ( ce_names.size() );
   vector < double > ec_min_          ( ce_names.size() );
   vector < double > ec_max_          ( ce_names.size() );
   vector < double > buckets_         ( ce_names.size() );
   vector < double > ec_delta_        ( ce_names.size() );
   vector < double > ec_delta_over_2_ ( ce_names.size() );

   bool do_weight = comparative->weight_controls;
   bool do_by_ec  = comparative->by_ec;
   bool do_ec     = false;

   bool processed_fields_exist = ( do_weight && csv1.header_map.count(header_weight) );

   for ( unsigned int i = 0; i < ce_names.size(); i++ )
   {
      do_[i]       = csv1.header_map.count( ce_names[i] ) && ce_map[ce_names[i]]->active;
      ref_[i]      = ce_map[ce_names[i]]->store_reference;
      diff_[i]     = ce_map[ce_names[i]]->store_diff;
      abs_diff_[i] = ce_map[ce_names[i]]->store_abs_diff;
      exp_[i]      = ce_map[ce_names[i]]->target;
      ec_min_[i]   = ce_map[ce_names[i]]->min;
      ec_max_[i]   = ce_map[ce_names[i]]->max;
      buckets_[i]  = ce_map[ce_names[i]]->buckets;
      ec_[i]       = buckets_[i] > 0 && ec_max_[i] > ec_min_[i];

      if ( ec_[i] )
      {
         do_ec = true;
         ec_delta_[i]       = ( ec_max_[i] - ec_min_[i] ) / buckets_[i];
         ec_delta_over_2_[i] =  0.5 * ec_delta_[i];
      }

      if ( !processed_fields_exist && do_[i] &&
           ( ( ref_[i] && csv1.header_map.count(header_exp + ce_names[i] ) ) ||
             ( diff_[i] && csv1.header_map.count(header_diff + ce_names[i] ) ) ||
             ( abs_diff_[i] && csv1.header_map.count(header_abs_diff + ce_names[i] ) ) ) )
      {
         processed_fields_exist = true;
      }
   }

   if ( !processed_fields_exist && do_ec && csv1.header_map.count( header_ec ) )
   {
      processed_fields_exist = true;
   }
      
   if ( processed_fields_exist )
   {
      if ( 
          !QMessageBox::question(
                                 this,
                                 us_tr("Process"),
                                 us_tr("Some of the added columns that will be computed already exist.\n"
                                    "What do you want remove them?"),
                                 us_tr("&Yes, remove them"), us_tr("&No, rename them"),
                                 QString(), 0, 1 ) 
          )
      {
         for ( unsigned int i = 0; i < ce_names.size(); i++ )
         {
            if ( do_[i] )
            {
               if ( ref_[i] && csv1.header_map.count(header_exp + ce_names[i]) )
               {
                  csv_remove_column(csv1, header_exp + ce_names[i]);
               }
               if ( diff_[i] && csv1.header_map.count(header_diff + ce_names[i]) )
               {
                  csv_remove_column(csv1, header_diff + ce_names[i]);
               }
               if ( abs_diff_[i] && csv1.header_map.count(header_abs_diff + ce_names[i]) )
               {
                  csv_remove_column(csv1, header_abs_diff + ce_names[i]);
               }
            }
         }
         if ( do_weight && csv1.header_map.count(header_weight) )
         {
            csv_remove_column(csv1, header_weight);
         }
         if ( do_ec && csv1.header_map.count(header_ec) )
         {
            csv_remove_column(csv1, header_ec);
         }
      } else {
         for ( unsigned int i = 0; i < ce_names.size(); i++ )
         {
            if ( do_[i] )
            {
               if ( ref_[i] && csv1.header_map.count(header_exp + ce_names[i]) )
               {
                  csv_make_unique_header_name(csv1, header_exp + ce_names[i]);
               }
               if ( diff_[i] && csv1.header_map.count(header_diff + ce_names[i]) )
               {
                  csv_make_unique_header_name(csv1, header_diff + ce_names[i]);
               }
               if ( abs_diff_[i] && csv1.header_map.count(header_abs_diff + ce_names[i]) )
               {
                  csv_make_unique_header_name(csv1, header_abs_diff + ce_names[i]);
               }
            }
         }
         if ( do_weight && csv1.header_map.count(header_weight) )
         {
            csv_make_unique_header_name(csv1, header_weight);
         }
         if ( do_ec && csv1.header_map.count(header_ec) )
         {
            csv_make_unique_header_name(csv1, header_ec);
         }
      }
   }
   unsigned int next_col = csv1.header.size();

   vector < unsigned int > col_model_( ce_names.size() );
   vector < unsigned int > col_ref_( ce_names.size() );
   vector < unsigned int > col_diff_( ce_names.size() );
   vector < unsigned int > col_abs_diff_( ce_names.size() );

   for ( unsigned int i = 0; i < ce_names.size(); i++ )
   {
      col_model_[i]    = 0;
      col_ref_[i]      = 0;
      col_diff_[i]     = 0;
      col_abs_diff_[i] = 0;

      if ( do_[i] )
      {
         col_model_[i] = csv1.header_map[ce_names[i]];
         if ( ref_[i] )
         {
            col_ref_[i] = next_col++;
            csv1.header.push_back(header_exp + ce_names[i]);
            csv1.header_map[header_exp + ce_names[i]] = col_ref_[i];
         }
         if ( diff_[i] )
         {
            col_diff_[i] = next_col++;
            csv1.header.push_back(header_diff + ce_names[i]);
            csv1.header_map[header_diff + ce_names[i]] = col_diff_[i];
         }
         if ( abs_diff_[i] )
         {
            col_abs_diff_[i] = next_col++;
            csv1.header.push_back(header_abs_diff + ce_names[i]);
            csv1.header_map[header_abs_diff + ce_names[i]] = col_abs_diff_[i];
         }
      }
   }

   unsigned int col_weight = 0;
   if ( do_weight )
   {
      col_weight = next_col++;
      csv1.header.push_back(header_weight);
      csv1.header_map[header_weight] = col_weight;
   }

   unsigned int col_ec = 0;
   if ( do_ec )
   {
      col_ec = next_col++;
      csv1.header.push_back(header_ec);
      csv1.header_map[header_weight] = col_ec;
   }

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      csv1.data[i].resize(csv1.header.size());
      csv1.num_data[i].resize(csv1.header.size());

      double weight = 0e0;
      unsigned int ec_pos = 0;

      for ( unsigned int j = 0; j < ce_names.size(); j++ )
      {
         if ( do_[j] )
         {
            if ( ref_[j] )
            {
               csv1.num_data[i][col_ref_[j]] = exp_[j];
               csv1.data[i][col_ref_[j]] = QString("%1").arg(csv1.num_data[i][col_ref_[j]]);
            }
            if ( diff_[j] )
            {
               csv1.num_data[i][col_diff_[j]] = csv1.num_data[i][col_model_[j]] - exp_[j];
               if ( comparative->by_pct && exp_[j] != 0.0 )
               {
                  csv1.num_data[i][col_diff_[j]] = 100.0 * csv1.num_data[i][col_diff_[j]] / exp_[j];
               }
               csv1.data[i][col_diff_[j]] = QString("%1").arg(csv1.num_data[i][col_diff_[j]]);
            }
            if ( abs_diff_[j] )
            {
               csv1.num_data[i][col_abs_diff_[j]] = fabs( csv1.num_data[i][col_model_[j]] - exp_[j] );
               if ( comparative->by_pct && exp_[j] != 0.0 )
               {
                  csv1.num_data[i][col_abs_diff_[j]] = 100.0 * csv1.num_data[i][col_abs_diff_[j]] / exp_[j];
               }
               csv1.data[i][col_abs_diff_[j]] = QString("%1").arg(csv1.num_data[i][col_abs_diff_[j]]);
            }
            if ( do_weight && ce_map[ce_names[j]]->include_in_weight )
            {
               if ( comparative->by_pct && exp_[j] != 0.0 )
               {
                  weight += 
                     100.0 *
                     ( fabs( csv1.num_data[i][col_model_[j]] - exp_[j] ) / exp_[j] ) * 
                     ce_map[ce_names[j]]->weight;
               } else {
                  weight += fabs( csv1.num_data[i][col_model_[j]] - exp_[j] ) * ce_map[ce_names[j]]->weight;
               }
            }
            if ( do_ec && ec_[j] )
            {
               double abs_diff = fabs( csv1.num_data[i][col_model_[j]] - exp_[j] );
               if ( abs_diff > ec_delta_over_2_[j] )
               {
                  ec_pos += (unsigned int) ( ( abs_diff - ec_delta_over_2_[j] ) / ec_delta_over_2_[j] );
               }
            }
         }
      }

      if ( do_weight )
      {
         csv1.num_data[i][col_weight] = weight;
         csv1.data[i][col_weight] = QString("%1").arg(weight);
      }
      if ( do_ec )
      {
         csv1.num_data[i][col_ec] = (double) ec_pos;
         csv1.data[i][col_ec] = QString("%1").arg(ec_pos);
      }
   }

   // sort appropriately

   // figure out the relevant columns

   vector < unsigned int > sort_cols;

   if ( do_weight && col_weight )
   {
      sort_cols.push_back( col_weight );
   } else {
      if ( do_ec && do_by_ec && col_ec )
      {
         sort_cols.push_back( col_ec );
      } else {
         // by ranked abs diff's
         // first sort the ranking order of the cols
         
         list < sortable_unsigned_int > lsui;
         
         sortable_unsigned_int sui;
         
         for ( unsigned int i = 0; i < ce_names.size(); i++ )
         {
            if ( do_[i] )
            {
               sui.ui = ce_map[ce_names[i]]->rank;
               sui.index = col_abs_diff_[i];
               lsui.push_back( sui );
            }
         }
         
         lsui.sort();
         
         for ( list < sortable_unsigned_int >::iterator it = lsui.begin();
               it != lsui.end();
               it++ )
         {
            sort_cols.push_back( it->index );
         }
      }
   }
      
   if ( !sort_cols.size() )
   {
      editor_msg("red", us_tr("Internal error: could not find any columns to sort!"));
   } else {
      csv_sort( csv1, sort_cols );
   }

   // cout << csv_info(csv1);
   return true;
}

bool US_Hydrodyn_Comparative::any_params_enabled()
{
   bool any_enabled = false;
   for ( unsigned int i = 0; i < ce_names.size(); i++ )
   {
      if ( all_selected_csv_contain( *ce_map[ce_names[i]] ) )
      {
         any_enabled = true;
         break;
      }
   }
   return any_enabled;
}

void US_Hydrodyn_Comparative::update_selected_map() 
{
   selected_map.clear( );
   for ( unsigned int i = 0; i < (unsigned int) lb_selected->count(); i++ )
   {
      if ( lb_selected->item(i)->isSelected() )
      {
         selected_map[ lb_selected->item(i)->text() ] = true;
      }
   }
}

bool US_Hydrodyn_Comparative::csv_get_min_max( 
                                              double &min, 
                                              double &max, 
                                              unsigned int &rows_used_count,
                                              comparative_entry ce, 
                                              csv &csv1,
                                              bool ignore_selected_selected
                                              )
{
   // cout << QString("start csv_get_min_max ce.name %1 %2\n").arg(ce.name).arg(ignore_selected_selected ?
   // "all loaded selected" :
   // "only selected selected" );
   rows_used_count = 0;

   if ( !csv1.header_map.count(ce.name) || !csv1.data.size() )
   {
      return false;
   }

   unsigned int col = csv1.header_map[ce.name];

   bool done_first = false;

   for ( unsigned int i = 0; i < csv1.num_data.size(); i++ )
   {
      if ( ignore_selected_selected || selected_map.count(csv1.prepended_names[i]) )
      {   
         rows_used_count++;
         if ( done_first )
         {
            if ( min > csv1.num_data[i][col] )
            {
               min = csv1.num_data[i][col];
            }
            if ( max < csv1.num_data[i][col] )
            {
               max = csv1.num_data[i][col];
            }
         } else {
            done_first = true;
            min = csv1.num_data[i][col];
            max = csv1.num_data[i][col];
         }
      }
   }
   // cout << QString("finish csv_get_min_max ce.name %1 min %2 max %3 rows used %4\n").arg(ce.name).arg(min).arg(max).arg(rows_used_count);

   return true;
}

bool US_Hydrodyn_Comparative::csv_get_loaded_min_max( 
                                                     double &min, 
                                                     double &max, 
                                                     comparative_entry ce
                                                     )
{
   //  cout << QString("get loaded min_max %1\n").arg(ce.name);

   double tmp_min;
   double tmp_max;
   unsigned int total_rows_used_count = 0;
   unsigned int rows_used_count;

   bool done_first = false;

   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
            return false;
         }
         if ( !csv_get_min_max(tmp_min, tmp_max, rows_used_count, ce, csvs[ lb_loaded->item(i)->text() ]) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 in csv %2")).arg(ce.name).arg(lb_loaded->item(i)->text()));
            return false;
         }
         if ( !rows_used_count )
         {
            editor_msg("red", QString(us_tr("internal error: could not find any rows for %1 in csv %2")).arg(ce.name).arg(lb_loaded->item(i)->text()));
            return false;
         }
         total_rows_used_count += rows_used_count;
         if ( done_first )
         {
            if ( min > tmp_min )
            {
               min = tmp_min;
            }
            if ( max < tmp_max )
            {
               max = tmp_max;
            }
         } else {
            done_first = true;
            min = tmp_min;
            max = tmp_max;
         }
      }
   }

   if ( !total_rows_used_count )
   {
      editor_msg("red", QString(us_tr("internal error: could not find any rows for %1 in any of the loaded selected csvs")).arg(ce.name));
      return false;
   }
   return true;
}

bool US_Hydrodyn_Comparative::csv_get_selected_min_max( 
                                                       double &min, 
                                                       double &max, 
                                                       comparative_entry ce
                                                       )
{
   //   cout << QString("get selected min_max %1\n").arg(ce.name);
   double tmp_min;
   double tmp_max;
   unsigned int rows_used_count;
   bool done_first = false;

   update_selected_map();

   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( lb_loaded->item(i)->isSelected() )
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
            return false;
         }
         if ( !csv_get_min_max(tmp_min, tmp_max, rows_used_count, ce, csvs[ lb_loaded->item(i)->text() ], false) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 in csv %2")).arg(ce.name).arg(lb_loaded->item(i)->text()));
            return false;
         }
         if ( rows_used_count )
         {
            if ( done_first )
            {
               if ( min > tmp_min )
               {
                  min = tmp_min;
               }
               if ( max < tmp_max )
               {
                  max = tmp_max;
               }
            } else {
               done_first = true;
               min = tmp_min;
               max = tmp_max;
            }
         }
      }
   }

   return true;
}

QStringList US_Hydrodyn_Comparative::csv_parse_line( QString qs )
{
   // cout << QString("csv_parse_line:\ninital string <%1>\n").arg(qs);
   QStringList qsl;
   if ( qs.isEmpty() )
   {
      // cout << QString("csv_parse_line: empty\n");
      return qsl;
   }
   if ( !qs.contains(",") )
   {
      // cout << QString("csv_parse_line: one token\n");
      qsl << qs;
      return qsl;
   }

   QStringList qsl_chars = (qs).split( "" , Qt::SkipEmptyParts );
   QString token = "";

   bool in_quote = false;

   for ( QStringList::iterator it = qsl_chars.begin();
         it != qsl_chars.end();
         it++ )
   {
      if ( !in_quote && *it == "," )
      {
         qsl << token;
         token = "";
         continue;
      }
      if ( in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = true;
         continue;
      }
      if ( !in_quote && *it == "\"" )
      {
         in_quote = false;
         continue;
      }
      token += *it;
   }
   if ( !token.isEmpty() )
   {
      qsl << token;
   }
   // cout << QString("csv_parse_line results:\n<%1>\n").arg(qsl.join(">\n<"));
   return qsl;
}

bool US_Hydrodyn_Comparative::csv_premerge_column_warning( csv &csv1, csv &csv2 )
{
   bool is_ok = true;
   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      if ( !csv2.header_map.count(csv1.header[i]) )
      {
         is_ok = false;
         if ( !csv_premerge_missing_header_map.count(csv1.header[i]) )
         {
            csv_premerge_missing_header_map[csv1.header[i]] = true;
            if ( csv_premerge_missing_header_qsl.size() < PREMERGE_LIST_LIMIT + 1 )
            {
               if ( csv_premerge_missing_header_qsl.size() == PREMERGE_LIST_LIMIT )
               {
                  csv_premerge_missing_header_qsl << us_tr(" (Additional extra columns not shown.)");
                  return is_ok;
               }
               csv_premerge_missing_header_qsl << QString(" From %1: %2").arg(QFileInfo(csv1.name).completeBaseName()).arg(csv1.header[i]);
            }
         }
      }
   }

   for ( unsigned int i = 0; i < csv2.header.size(); i++ )
   {
      if ( !csv1.header_map.count(csv2.header[i] ) )
      {
         is_ok = false;
         if ( !csv_premerge_missing_header_map.count(csv2.header[i]) )
         {
            csv_premerge_missing_header_map[csv2.header[i]] = true;
            if ( csv_premerge_missing_header_qsl.size() < PREMERGE_LIST_LIMIT + 1 )
            {
               if ( csv_premerge_missing_header_qsl.size() == PREMERGE_LIST_LIMIT )
               {
                  csv_premerge_missing_header_qsl << us_tr(" (Additional extra columns not shown.)");
                  return is_ok;
               }
               csv_premerge_missing_header_qsl << QString(" From %1: %2").arg(QFileInfo(csv2.name).completeBaseName()).arg(csv2.header[i]);
            }
         }
      }
   }
   return is_ok;
}

bool US_Hydrodyn_Comparative::csv_premerge_column_warning_all_loaded_selected()
{
   csv_premerge_missing_header_map.clear( );
   csv_premerge_missing_header_qsl.clear( );
   
   if ( !any_loaded_selected() || one_loaded_selected() )
   {
      // nothing to do here
      return true;
   }

   if ( !csvs.count(first_loaded_selected()) )
   {
      editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(first_loaded_selected()));
      return true;
   }
      
   csv *csv_ref = &csvs[ first_loaded_selected() ];
   bool skip_first = true;

   for ( unsigned int i = 0; i < (unsigned int) lb_loaded->count(); i++ )
   {
      if ( !skip_first && lb_loaded->item(i)->isSelected() ) 
      {
         if ( !csvs.count(lb_loaded->item(i)->text()) )
         {
            editor_msg("red", QString(us_tr("internal error: could not find %1 csv data")).arg(lb_loaded->item(i)->text()));
            return true;
         }
         if (
             !csv_premerge_column_warning(*csv_ref, csvs[ lb_loaded->item(i)->text() ]) && 
             csv_premerge_missing_header_qsl.size() >= PREMERGE_LIST_LIMIT )
         {
            return false;
         }
      }

      if ( skip_first && lb_loaded->item(i)->isSelected() ) 
      {
         skip_first = false;
      }
   }
   return csv_premerge_missing_header_qsl.size() == 0;
}

QString US_Hydrodyn_Comparative::loaded_info()
{
   QString qs;

   qs = "loaded_csv_names:\n";
   for ( map < QString, bool >::iterator it = loaded_csv_names.begin();
         it != loaded_csv_names.end();
         it++ )
   {
      qs += " " + it->first + "\n";
   }

   qs += "loaded_csv_row_prepended_names:\n";
   for ( map < QString, bool >::iterator it = loaded_csv_row_prepended_names.begin();
         it != loaded_csv_row_prepended_names.end();
         it++ )
   {
      qs += " " + it->first + "\n";
   }

   return qs;
}

bool US_Hydrodyn_Comparative::csv_has_column_name( csv &csv1, QString name )
{
   return csv1.header_map.count(name) > 0;
}

void US_Hydrodyn_Comparative::csv_remove_column( csv &csv1, QString name )
{
   cout << "csv_remove_column " << name << endl;
   // cout << csv_info(csv1);

   if ( !csv1.header_map.count(name) )
   {
      return;
   }
   unsigned int col = csv1.header_map[name];
   cout << "csv_remove_column column number " << col << endl;
   
   // map < QString, int >::iterator it = csv1.header_map.find(name);
   csv1.header_map.erase(name);
   
   unsigned int last_col = csv1.header.size() - 1;
   cout << "csv_remove_column 1\n";

   if ( col < last_col )
   {
      for ( unsigned int i = col; i < last_col; i++ )
      {
         csv1.header[i] = csv1.header[i+1];
      }
   }
   csv1.header.resize(last_col);
   cout << "csv_remove_column 2\n";

   if ( col < last_col )
   {
      for ( unsigned int i = 0; i < csv1.data.size(); i++ )
      {
         for ( unsigned int j = col; j < last_col; j++ )
         {
            csv1.data[i][j] = csv1.data[i][j+1];
            csv1.num_data[i][j] = csv1.num_data[i][j+1];
         }
         csv1.data[i].resize(last_col);
         csv1.num_data[i].resize(last_col);
      }
   }
   cout << "csv_remove_column 3\n";
}

void US_Hydrodyn_Comparative::csv_make_unique_header_name( csv &csv1, QString name )
{
   if ( !csv1.header_map.count(name) )
   {
      return;
   }
   int col = csv1.header_map[name];

   unsigned int inc = 0;
   while ( csv1.header_map.count(name + ( inc ? QString("-%1").arg(inc) : "" )) 
          )
   {
      inc++;
   }
   map < QString, int >::iterator it = csv1.header_map.find(name);
   csv1.header_map.erase(it);
   csv1.header_map[name + ( inc ? QString("-%1").arg(inc) : "" )] = col;
   csv1.header[col] = name + ( inc ? QString("-%1").arg(inc) : "" );
}

bool US_Hydrodyn_Comparative::ec_ready()
{
   bool is_ec_ready = false;
   if ( comparative->by_ec )
   {
      for ( unsigned int i = 0; i < ce_names.size(); i++ )
      {
         if ( ce_map[ce_names[i]]->active &&
              ce_map[ce_names[i]]->buckets > 0 &&
              ce_map[ce_names[i]]->max > ce_map[ce_names[i]]->min )
         {
            is_ec_ready = true;
            break;
         }
      }
   }
   return is_ec_ready;
}
