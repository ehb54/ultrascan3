#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_saxs_iqq_load_csv.h"
#include "../include/us_hydrodyn.h"
#include "qregexp.h"
//Added by qt3to4:
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_Hydrodyn_Saxs_Iqq_Load_Csv::US_Hydrodyn_Saxs_Iqq_Load_Csv(
                                                     QString msg,
                                                     QStringList *qsl_names,
                                                     QStringList *qsl_sel_names,
                                                     QStringList *qsl,
                                                     QString loaded_filename,
                                                     bool *create_avg,
                                                     bool *create_std_dev,
                                                     bool *only_plot_stats,
                                                     bool *save_to_csv,
                                                     QString *csv_filename,
                                                     bool *save_original_data,
                                                     bool *run_nnls,
                                                     bool *nnls_plot_contrib,
                                                     bool *nnls_csv,
                                                     bool *run_best_fit,
                                                     bool *run_ift,
                                                     bool *use_SDs_for_fitting,
                                                     QString *nnls_target,
                                                     bool *clear_plot_first,
                                                     bool expert_mode,
                                                     void *us_hydrodyn,
                                                     QWidget *p,
                                                     const char *
                                                     ) : QDialog( p )
{
   this->msg                  = msg;
   this->qsl_names            = qsl_names;
   this->qsl_sel_names        = qsl_sel_names;
   this->qsl                  = qsl;
   this->loaded_filename      = loaded_filename;
   this->create_avg           = create_avg;
   this->create_std_dev       = create_std_dev;
   this->only_plot_stats      = only_plot_stats;
   this->save_to_csv          = save_to_csv;
   this->csv_filename         = csv_filename;
   this->save_original_data   = save_original_data;
   this->run_nnls             = run_nnls;
   this->nnls_plot_contrib    = nnls_plot_contrib;
   this->nnls_csv             = nnls_csv;
   this->run_best_fit         = run_best_fit;
   this->run_ift              = run_ift;
   this->use_SDs_for_fitting  = use_SDs_for_fitting;
   this->nnls_target          = nnls_target;
   this->clear_plot_first     = clear_plot_first;
   this->expert_mode          = expert_mode;
   this->us_hydrodyn          = us_hydrodyn;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle("Load, manage and process SAS data");
   setupGUI();
   global_Xpos = 200;
   global_Ypos = 150;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
}

US_Hydrodyn_Saxs_Iqq_Load_Csv::~US_Hydrodyn_Saxs_Iqq_Load_Csv()
{
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::setupGUI()
{
   int minWidth1 = 600;
   int minHeight1 = 30;
   int minHeight1dl = 40;
   int minHeight2 = 30;

   lbl_info = new QLabel(msg, this);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight2 * 2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   lb_names = new QListWidget(this);
   lb_names->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_names->setMinimumHeight(minHeight1 * 15);
   lb_names->setMinimumWidth(minWidth1);
   lb_names->addItems(*qsl_names);
   lb_names->setPalette( PALET_EDIT );
   AUTFBACK( lb_names );
   lb_names->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_names->setSelectionMode(QAbstractItemView::MultiSelection);
   lb_names->setEnabled(true);
   connect(lb_names, SIGNAL(itemSelectionChanged()), SLOT(update_selected()));

   cb_create_avg = new QCheckBox(this);
   cb_create_avg->setText(us_tr("Create average curve   "));
   cb_create_avg->setEnabled(true);
   cb_create_avg->setChecked(*create_avg);
   cb_create_avg->setMinimumHeight(minHeight2);
   cb_create_avg->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_avg->setPalette( PALET_NORMAL );
   AUTFBACK( cb_create_avg );
   connect(cb_create_avg, SIGNAL(clicked()), this, SLOT(set_create_avg()));

   cb_create_std_dev = new QCheckBox(this);
   cb_create_std_dev->setText(us_tr("Create std deviation curves   "));
   cb_create_std_dev->setEnabled(true);
   cb_create_std_dev->setChecked(*create_std_dev);
   cb_create_std_dev->setMinimumHeight(minHeight2);
   cb_create_std_dev->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_std_dev->setPalette( PALET_NORMAL );
   AUTFBACK( cb_create_std_dev );
   connect(cb_create_std_dev, SIGNAL(clicked()), this, SLOT(set_create_std_dev()));

   cb_only_plot_stats = new QCheckBox(this);
   cb_only_plot_stats->setText(us_tr("Only plot stats   "));
   cb_only_plot_stats->setEnabled(true);
   cb_only_plot_stats->setChecked(*only_plot_stats);
   cb_only_plot_stats->setMinimumHeight(minHeight2);
   cb_only_plot_stats->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_only_plot_stats->setPalette( PALET_NORMAL );
   AUTFBACK( cb_only_plot_stats );
   connect(cb_only_plot_stats, SIGNAL(clicked()), this, SLOT(set_only_plot_stats()));

   cb_save_to_csv = new QCheckBox(this);
   cb_save_to_csv->setText(us_tr("Save to csv "));
   cb_save_to_csv->setEnabled(true);
   cb_save_to_csv->setChecked(*save_to_csv);
   cb_save_to_csv->setMinimumHeight(minHeight2);
   cb_save_to_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_to_csv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_to_csv );
   connect(cb_save_to_csv, SIGNAL(clicked()), this, SLOT(set_save_to_csv()));

   le_csv_filename = new QLineEdit( this );    le_csv_filename->setObjectName( "csv_filename Line Edit" );
   le_csv_filename->setText(*csv_filename);
   le_csv_filename->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_csv_filename->setMinimumWidth(100);
   le_csv_filename->setMinimumHeight(minHeight2);
   le_csv_filename->setPalette( PALET_NORMAL );
   AUTFBACK( le_csv_filename );
   le_csv_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   connect(le_csv_filename, SIGNAL(textChanged(const QString &)), SLOT(update_csv_filename(const QString &)));

   cb_save_original_data = new QCheckBox(this);
   cb_save_original_data->setText(us_tr("Include source data "));
   cb_save_original_data->setEnabled(true);
   cb_save_original_data->setChecked(*save_original_data);
   cb_save_original_data->setMinimumHeight(minHeight2);
   cb_save_original_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_save_original_data->setPalette( PALET_NORMAL );
   AUTFBACK( cb_save_original_data );
   connect(cb_save_original_data, SIGNAL(clicked()), this, SLOT(set_save_original_data()));
   
   if ( expert_mode )
   {
      cb_run_nnls = new QCheckBox(this);
      cb_run_nnls->setText(us_tr("NNLS fit"));
      cb_run_nnls->setEnabled(true);
      cb_run_nnls->setChecked(*run_nnls);
      cb_run_nnls->setMinimumHeight(minHeight1dl);
      cb_run_nnls->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_run_nnls->setPalette( PALET_NORMAL );
      AUTFBACK( cb_run_nnls );
      connect(cb_run_nnls, SIGNAL(clicked()), this, SLOT(set_run_nnls()));

      cb_nnls_plot_contrib = new QCheckBox(this);
      cb_nnls_plot_contrib->setText(us_tr("NNLS plot\ncontributing data"));
      cb_nnls_plot_contrib->setEnabled(true);
      cb_nnls_plot_contrib->setChecked(*nnls_plot_contrib);
      cb_nnls_plot_contrib->setMinimumHeight(minHeight1dl);
      cb_nnls_plot_contrib->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_nnls_plot_contrib->setPalette( PALET_NORMAL );
      AUTFBACK( cb_nnls_plot_contrib );
      connect(cb_nnls_plot_contrib, SIGNAL(clicked()), this, SLOT(set_nnls_plot_contrib()));

      cb_nnls_csv = new QCheckBox(this);
      cb_nnls_csv->setText(us_tr("Save NNLS CSV"));
      cb_nnls_csv->setEnabled(true);
      cb_nnls_csv->setChecked(*nnls_csv);
      cb_nnls_csv->setMinimumHeight(minHeight1dl);
      cb_nnls_csv->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_nnls_csv->setPalette( PALET_NORMAL );
      AUTFBACK( cb_nnls_csv );
      connect(cb_nnls_csv, SIGNAL(clicked()), this, SLOT(set_nnls_csv()));
      
      cb_run_best_fit = new QCheckBox(this);
      cb_run_best_fit->setText(us_tr("Best fit"));
      cb_run_best_fit->setEnabled(true);
      cb_run_best_fit->setChecked(*run_best_fit);
      cb_run_best_fit->setMinimumHeight(minHeight1dl);
      cb_run_best_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_run_best_fit->setPalette( PALET_NORMAL );
      AUTFBACK( cb_run_best_fit );
      connect(cb_run_best_fit, SIGNAL(clicked()), this, SLOT(set_run_best_fit()));

      cb_use_SDs_for_fitting = new QCheckBox(this);
      cb_use_SDs_for_fitting->setText(us_tr("Use SD in fit\n(if present)"));
      cb_use_SDs_for_fitting->setEnabled(true);
      cb_use_SDs_for_fitting->setChecked(*use_SDs_for_fitting);
      cb_use_SDs_for_fitting->setMinimumHeight(minHeight1dl);
      cb_use_SDs_for_fitting->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_use_SDs_for_fitting->setPalette( PALET_NORMAL );
      AUTFBACK( cb_use_SDs_for_fitting );
      connect(cb_use_SDs_for_fitting, SIGNAL(clicked()), this, SLOT(set_use_SDs_for_fitting()));

      cb_run_ift = new QCheckBox(this);
      cb_run_ift->setText(us_tr("Compute IFT"));
      cb_run_ift->setEnabled(true);
      cb_run_ift->setChecked(*run_ift);
      cb_run_ift->setMinimumHeight(minHeight1dl);
      cb_run_ift->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
      cb_run_ift->setPalette( PALET_NORMAL );
      AUTFBACK( cb_run_ift );
      connect(cb_run_ift, SIGNAL(clicked()), this, SLOT(set_run_ift()));
      // ift only when plotted curves
      if ( !msg.contains( "Plotted I(q) curves" ) ) {
         cb_run_ift->hide();
      }
      
      pb_select_target = new QPushButton(us_tr("Select Target"), this);
      pb_select_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
      pb_select_target->setMinimumHeight(minHeight1dl);
      pb_select_target->setPalette( PALET_PUSHB );
      connect(pb_select_target, SIGNAL(clicked()), SLOT(select_target()));

      lbl_nnls_target = new QLabel(*nnls_target, this);
      lbl_nnls_target->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
      lbl_nnls_target->setMinimumHeight(minHeight2);
      lbl_nnls_target->setPalette( PALET_LABEL );
      AUTFBACK( lbl_nnls_target );
      lbl_nnls_target->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));
      if ( !nnls_target->isEmpty() )
      {
         lbl_nnls_target->setText(us_tr("Target model: ") + *nnls_target);
      }
   }

   cb_clear_plot_first = new QCheckBox(this);
   cb_clear_plot_first->setText(us_tr("Clear plot"));
   cb_clear_plot_first->setEnabled(true);
   cb_clear_plot_first->setMinimumHeight(minHeight1);
   cb_clear_plot_first->setChecked(*clear_plot_first);
   cb_clear_plot_first->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_clear_plot_first->setPalette( PALET_NORMAL );
   AUTFBACK( cb_clear_plot_first );
   connect(cb_clear_plot_first, SIGNAL(clicked()), this, SLOT(set_clear_plot_first()));

   pb_select_all = new QPushButton(us_tr("Select All"), this);
   pb_select_all->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_select_all->setMinimumHeight(minHeight1);
   pb_select_all->setPalette( PALET_PUSHB );
   connect(pb_select_all, SIGNAL(clicked()), SLOT(select_all()));

   pb_transpose = new QPushButton(us_tr("Transpose"), this);
   pb_transpose->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_transpose->setMinimumHeight(minHeight1);
   pb_transpose->setPalette( PALET_PUSHB );
   connect(pb_transpose, SIGNAL(clicked()), SLOT(transpose()));

   pb_save_as_dat = new QPushButton(us_tr("Save .DAT"), this);
   pb_save_as_dat->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_as_dat->setMinimumHeight(minHeight1);
   pb_save_as_dat->setPalette( PALET_PUSHB );
   connect(pb_save_as_dat, SIGNAL(clicked()), SLOT(save_as_dat()));

   pb_save_selected = new QPushButton(us_tr("Save"), this);
   pb_save_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_save_selected->setMinimumHeight(minHeight1);
   pb_save_selected->setPalette( PALET_PUSHB );
   connect(pb_save_selected, SIGNAL(clicked()), SLOT(save_selected()));

   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton("Help", this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight2);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_ok = new QPushButton(us_tr("Plot"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   // build layout

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget(pb_select_all);
   hbl_bottom->addWidget(pb_transpose);
   hbl_bottom->addWidget(pb_save_as_dat);
   hbl_bottom->addWidget(pb_save_selected);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addWidget(pb_ok);
   hbl_bottom->addWidget(cb_clear_plot_first);

   QHBoxLayout * hbl_avg_std = new QHBoxLayout; hbl_avg_std->setContentsMargins( 0, 0, 0, 0 ); hbl_avg_std->setSpacing( 0 );
   hbl_avg_std->addWidget(cb_create_avg);
   hbl_avg_std->addWidget(cb_create_std_dev);
   hbl_avg_std->addWidget(cb_only_plot_stats);
   hbl_avg_std->addWidget(cb_save_to_csv);
   hbl_avg_std->addWidget(le_csv_filename);
   hbl_avg_std->addWidget(cb_save_original_data);

   QVBoxLayout * background = new QVBoxLayout(this); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget(lbl_info);
   background->addWidget(lb_names);
   background->addLayout(hbl_avg_std);
   if ( expert_mode )
   {
      QHBoxLayout * hbl_nnls_best_fit = new QHBoxLayout; hbl_nnls_best_fit->setContentsMargins( 0, 0, 0, 0 ); hbl_nnls_best_fit->setSpacing( 0 );
      hbl_nnls_best_fit->addWidget(cb_run_nnls);
      hbl_nnls_best_fit->addWidget(cb_nnls_plot_contrib);
      hbl_nnls_best_fit->addWidget(cb_nnls_csv);
      hbl_nnls_best_fit->addWidget(cb_run_best_fit);
      hbl_nnls_best_fit->addWidget(cb_use_SDs_for_fitting);
      hbl_nnls_best_fit->addWidget(cb_run_ift);

      QHBoxLayout * hbl_nnls = new QHBoxLayout; hbl_nnls->setContentsMargins( 0, 0, 0, 0 ); hbl_nnls->setSpacing( 0 );
      hbl_nnls->addLayout(hbl_nnls_best_fit);
      hbl_nnls->addWidget(pb_select_target);
      hbl_nnls->addWidget(lbl_nnls_target);
      background->addLayout(hbl_nnls);
   }
   background->addLayout(hbl_bottom);

   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::select_all()
{
   // if there are any, not selected, select all
   // if all are selected, unselect all
   bool select_all = false;
   qDebug() << "select all, lb_names->count(): " << lb_names->count();
   
   disconnect(lb_names, SIGNAL(itemSelectionChanged()), 0, 0 );

   // are any unselected ?
   for ( int i = 0; i < lb_names->count(); i++ )
   {
      if ( !lb_names->item(i)->isSelected() )
      {
         select_all = true;
         break;
      }
   }

   select_all ? lb_names->selectAll() : lb_names->clearSelection();
   // for ( int i = 0; i < lb_names->count(); i++ )
   // {
   //    lb_names->item(i)->setSelected( select_all);
   // }
   if ( select_all )
   {
      lb_names->scrollToItem( lb_names->item(lb_names->count() - 1) );
   }
   connect(lb_names, SIGNAL(itemSelectionChanged()), SLOT(update_selected()));
   emit update_selected();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::ok()
{
   close();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::transpose()
{
   // pick 
   QString save_file = loaded_filename;
   save_file.replace(".csv","_t.csv");
   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the transposed csv file") , save_file , "*.csv" );

   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".csv$", Qt::CaseInsensitive )) )
   {
      fname += "_t.csv";
   }
   // open
   if ( QFile::exists(fname) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }
   FILE *of = us_fopen(fname, "wb");
   if ( of )
   {
      // go through the qsl, find selected...
      map < QString, bool > map_sel_names;
      for ( QStringList::iterator it = qsl_sel_names->begin();
            it != qsl_sel_names->end();
            it++ )
      {
         map_sel_names[*it] = true;
      }
      
      // find max length & build a 2d array of values
      unsigned int max_len = 0;
      vector < vector < QString > > array2d_to_save;
      for ( QStringList::iterator it = qsl->begin();
               it != qsl->end();
               it++ )
      {
         QStringList qsl_tmp = (*it).split( "," );
         if ( qsl_tmp.size() )
         {
            if ( ( map_sel_names.count(qsl_tmp[0]) || it == qsl->begin() ) )
            {
               vector < QString > array_to_save;
               // cout << "ok: " << qsl_tmp[0] << endl;
               if ( max_len < (unsigned int)qsl_tmp.count() )
               {
                  max_len = qsl_tmp.count();
               }
               for ( QStringList::iterator it2 = qsl_tmp.begin();
                     it2 != qsl_tmp.end();
                     it2++ )
               {
                  array_to_save.push_back(*it2);
               }
               // cout << QString("line %1 qsl_tmp.count() %2 size %3\n").arg(qsl_tmp[0]).arg(qsl_tmp.count()).arg(array_to_save.size());
               array2d_to_save.push_back(array_to_save);
            }
         }
      }

      // write them out
      //      cout << QString("max len %1\n").arg(max_len);

      for ( unsigned int i = 0; i < max_len; i++ ) 
      {
         QString comma = "";
         for ( unsigned int j = 0; j < array2d_to_save.size(); j++ )
         {
            if ( array2d_to_save[j].size() <= i ||
                 array2d_to_save[j][i].isNull() )
            {
               // cout <<  QString("array2d_to_save[%1].size() == %2\n")
               // .arg(j)
               // .arg(array2d_to_save[j].size());
               // fprintf(of, comma.toLatin1().data());
               fputs( comma.toLatin1().data(), of );
            } else {
               fprintf(of, "%s%s", comma.toLatin1().data(), array2d_to_save[j][i].toLatin1().data());
            }
            comma = ",";
         }
         fprintf(of, "\n");
      }
      fclose(of);
      QMessageBox::information( this, "UltraScan",
                                QString(us_tr("Created the output file:\n") + "%1").arg(fname));

   } else {
      QMessageBox::warning( this, "UltraScan",
                                QString(us_tr("Could not create the output file:\n") + "%1").arg(fname));
   }
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::save_selected()
{
   // pick 
   QString save_file = loaded_filename;
   save_file.replace(".csv","_s.csv");
   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the new csv file") , save_file , "*.csv" );

   if ( fname.isEmpty() )
   {
      return;
   }
   if ( !fname.contains(QRegExp(".csv$", Qt::CaseInsensitive )) )
   {
      fname += ".csv";
   }
   // open
   if ( QFile::exists(fname) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }
   FILE *of = us_fopen(fname, "wb");
   if ( of )
   {
      // go through the qsl, find selected...
      map < QString, bool > map_sel_names;
      for ( QStringList::iterator it = qsl_sel_names->begin();
            it != qsl_sel_names->end();
            it++ )
      {
         map_sel_names[*it] = true;
      }
      
      // find max length & build a 2d array of values
      for ( QStringList::iterator it = qsl->begin();
               it != qsl->end();
               it++ )
      {
         QStringList qsl_tmp = (*it).split( "," );
         if ( qsl_tmp.size() ) 
         {
            if ( map_sel_names.count(qsl_tmp[0]) || it == qsl->begin() )
            {
               fprintf(of, "%s\n", (*it).toLatin1().data());
            }
         }
      }
      fclose(of);
      QMessageBox::information( this, "UltraScan",
                                QString(us_tr("Created the output file:\n") + "%1").arg(fname));

   } else {
      QMessageBox::warning( this, "UltraScan",
                                QString(us_tr("Could not create the output file:\n") + "%1").arg(fname));
   }
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::cancel()
{
   qsl_sel_names->clear( );
   close();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo_saxs_load_csv.html");
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::update_selected()
{
   qsl_sel_names->clear( );
   for ( int i = 0; i < lb_names->count(); i++ )
   {
      if ( lb_names->item(i)->isSelected() )
      {
         *qsl_sel_names << lb_names->item(i)->text();
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::update_csv_filename(const QString &str)
{
   *csv_filename = str;
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_create_avg()
{
   *create_avg = cb_create_avg->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_create_std_dev()
{
   *create_std_dev = cb_create_std_dev->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_only_plot_stats()
{
   *only_plot_stats = cb_only_plot_stats->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_save_to_csv()
{
   *save_to_csv = cb_save_to_csv->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_save_original_data()
{
   *save_original_data = cb_save_original_data->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_run_nnls()
{
   *run_nnls = cb_run_nnls->isChecked();
   if ( *run_nnls ) {
      cb_run_ift        ->setChecked( false );
      *run_ift          = false;
   }
   if ( nnls_target->isEmpty() && *run_nnls )
   {
      select_target();
   } else {
      if ( !*run_nnls && !*run_best_fit && !*run_ift )
      {
         *nnls_target = "";
         lbl_nnls_target->setText("");
      }
   }         
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_nnls_csv()
{
   *nnls_csv = cb_nnls_csv->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_nnls_plot_contrib()
{
   *nnls_plot_contrib = cb_nnls_plot_contrib->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_run_ift()
{
   *run_ift = cb_run_ift->isChecked();
   if ( *run_ift ) {
      cb_clear_plot_first->setChecked( false );
      cb_run_nnls        ->setChecked( false );
      cb_run_best_fit    ->setChecked( false );
      *clear_plot_first = false;
      *run_nnls         = false;
      *run_best_fit     = false;
   }
   if ( nnls_target->isEmpty() && *run_ift )
   {
      // select_target();
   } else {
      if ( !*run_nnls && !*run_best_fit && !*run_ift )
      {
         // *nnls_target = "";
         // lbl_nnls_target->setText("");
      }
   }         
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_run_best_fit()
{
   *run_best_fit = cb_run_best_fit->isChecked();
   if ( *run_best_fit ) {
      cb_run_ift        ->setChecked( false );
      *run_ift          = false;
   }
   if ( nnls_target->isEmpty() && *run_best_fit )
   {
      select_target();
   } else {
      if ( !*run_nnls && !*run_best_fit && !*run_ift )
      {
         *nnls_target = "";
         lbl_nnls_target->setText("");
      }
   }
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_use_SDs_for_fitting()
{
   *use_SDs_for_fitting = cb_use_SDs_for_fitting->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::set_clear_plot_first()
{
   *clear_plot_first = cb_clear_plot_first->isChecked();
   update_enables();
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::select_target()
{
   if ( *run_nnls || *run_best_fit )
   {
      if ( qsl_sel_names->size() )
      {
         *nnls_target = (*qsl_sel_names)[0];
      } else {
         *nnls_target = (*qsl_names)[0];
      }
      lbl_nnls_target->setText(us_tr("Target model: ") + *nnls_target);
   }
}   

void US_Hydrodyn_Saxs_Iqq_Load_Csv::update_enables()
{
   pb_ok->setText( us_tr( cb_run_ift->isChecked() ? "Run" : "Plot" ) );
   
   cb_create_avg->setEnabled(qsl_sel_names->size() > 1);
   cb_create_std_dev->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 2);
   cb_only_plot_stats->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 1);
   cb_save_to_csv->setEnabled(
                              ( cb_create_avg->isChecked() || ( expert_mode && 
                                                                ( 
                                                                 cb_run_nnls->isChecked() ||
                                                                 cb_run_best_fit->isChecked() 
                                                                 )
                                                                ) )
                              && qsl_sel_names->size() > 1 );
   le_csv_filename->setEnabled(
                               ( cb_create_avg->isChecked() || ( expert_mode &&
                                                                 (
                                                                  cb_run_nnls->isChecked() ||
                                                                  cb_run_best_fit->isChecked() 
                                                                  ) 
                                                                 ) )
                               && cb_save_to_csv->isChecked()
                               && qsl_sel_names->size() > 1 );
   cb_save_original_data->setEnabled(cb_create_avg->isChecked() && qsl_sel_names->size() > 1);
   if ( expert_mode )
   {
      pb_select_target->setEnabled( (
                                     cb_run_nnls->isChecked() ||
                                     cb_run_best_fit->isChecked() 
                                     )
                                    && qsl_sel_names->size() == 1 );
   }
   pb_transpose->setEnabled(qsl_sel_names->size());
   pb_transpose->setText(QString(us_tr("Transpose") + "%1")
                         .arg(qsl_sel_names->size() > 0 ? QString(" (%1)").arg(qsl_sel_names->size()) : ""));
   pb_save_as_dat->setEnabled(qsl_sel_names->size() == 1);
   pb_save_selected->setEnabled(qsl_sel_names->size());
   pb_ok->setEnabled( qsl_sel_names->size() );
   cb_clear_plot_first->setEnabled( qsl_sel_names->size() && !cb_run_ift->isChecked() );
   cb_run_nnls->setEnabled( !cb_run_ift->isChecked() );
   cb_run_best_fit->setEnabled( !cb_run_ift->isChecked() );
   cb_nnls_csv->setEnabled( cb_run_nnls->isChecked() );
   if ( !cb_run_nnls->isChecked() ) {
      cb_nnls_csv->setChecked( false );
      *nnls_csv = false;
   }
   cb_use_SDs_for_fitting->setEnabled( cb_run_nnls->isChecked() || cb_run_best_fit->isChecked() );
   cb_nnls_plot_contrib->setEnabled( cb_run_nnls->isChecked() );
}

void US_Hydrodyn_Saxs_Iqq_Load_Csv::save_as_dat()
{
   // find selected one & name it that

   bool save_qIq = 0;
   /*
      QMessageBox::question(this, 
                            us_tr( "US-SOMO: Save .DAT" ),
                            us_tr( "How do you want to save as a q*I(q) file?" ),
                            us_tr( "&No, just normally" ),
                            us_tr( "&Yes, compute and save I as q*I(q)" ),
                            QString(),
                            0,
                            1
                            ) 
      == 1;
   */

   QString sel_name;

   for ( QStringList::iterator it = qsl_sel_names->begin();
         it != qsl_sel_names->end();
         it++ )
   {
      sel_name = *it;
   }

   QString save_file = sel_name;
   cout << sel_name << endl;
   save_file.replace("\"","");
   save_file.replace(QRegExp("\\..*$"),"");
   save_file += ".dat";
   save_file = QFileInfo( save_file ).fileName();
   cout << save_file << endl;

   QString fname = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the DAT file") , save_file , "*.dat" );

   if ( fname.isEmpty() )
   {
      return;
   }

   if ( !fname.contains(QRegExp(".dat$", Qt::CaseInsensitive )) )
   {
      fname += ".dat";
   }

   // open
   if ( QFile::exists(fname) )
   {
      fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
   }

   FILE *of = us_fopen(fname, "wb");

   if ( of )
   {
      // go through the qsl, find selected...
      
      // find data and possible errors and create a .dat file of I(q) & I(q) sd

      // find max length & build a 2d array of values
      unsigned int max_len = 0;
      vector < vector < QString > > array2d_to_save;
      for ( QStringList::iterator it = qsl->begin();
               it != qsl->end();
               it++ )
      {
         QStringList qsl_tmp = (*it).split( "," );
         if ( qsl_tmp.size() )
         {
            if ( qsl_tmp[0] == sel_name || it == qsl->begin() )
            {
               vector < QString > array_to_save;
               // cout << "ok: " << qsl_tmp[0] << endl;
               if ( max_len < (unsigned int)qsl_tmp.count() )
               {
                  max_len = qsl_tmp.count();
               }
               for ( QStringList::iterator it2 = qsl_tmp.begin();
                     it2 != qsl_tmp.end();
                     it2++ )
               {
                  array_to_save.push_back(*it2);
               }
               // cout << QString("line %1 qsl_tmp.count() %2 size %3\n").arg(qsl_tmp[0]).arg(qsl_tmp.count()).arg(array_to_save.size());
               array2d_to_save.push_back(array_to_save);
               if ( save_qIq && qsl_tmp[0] == sel_name )
               {
                  unsigned int pos = array2d_to_save.size() - 1;
                  for ( unsigned int i = 2; i < ( unsigned int )array2d_to_save[ pos ].size() - 1; i++ )
                  {
                     array2d_to_save[ pos ][ i ] = 
                        QString( "%1" ).arg( array2d_to_save[ pos ][ i ].toDouble() *
                                             array2d_to_save[ 0 ][ i ].toDouble() );
                  }
               }
            }
         }
      }

      // write them out
      //      cout << QString("max len %1\n").arg(max_len);

      for ( unsigned int i = 0; i < max_len; i++ ) 
      {
         QString tab = "";
         for ( unsigned int j = 0; j < array2d_to_save.size(); j++ )
         {
            if ( array2d_to_save[j].size() <= i ||
                 array2d_to_save[j][i].isNull() )
            {
               // cout <<  QString("array2d_to_save[%1].size() == %2\n")
               // .arg(j)
               // .arg(array2d_to_save[j].size());
               // fprintf(of, tab.toLatin1().data());
               fputs( tab.toLatin1().data(), of );
            } else {
               fprintf(of, "%s%s", tab.toLatin1().data(), QString( "%1" ).arg( array2d_to_save[j][i] ).replace( "\"", "'" ).toLatin1().data() );
            }
            tab = "\t";
         }
         fprintf(of, "\n");
      }
      fclose(of);
      QMessageBox::information( this, "UltraScan",
                                QString(us_tr("Created the output file:\n") + "%1").arg(fname));

   } else {
      QMessageBox::warning( this, "UltraScan",
                                QString(us_tr("Could not create the output file:\n") + "%1").arg(fname));
   }
}

