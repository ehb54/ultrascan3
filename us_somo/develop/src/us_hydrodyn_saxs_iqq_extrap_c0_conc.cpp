#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QColor>
#include <QHeaderView>
#include <QIntValidator>

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc(
                                                                          QStringList names,
                                                                          map < QString, double > prepop_conc,
                                                                          map < QString, double > *out_name_to_conc,
                                                                          QStringList *out_selected_names,
                                                                          bool *out_ok,
                                                                          bool *out_primus_mode,
                                                                          bool *out_show_regplots,
                                                                          int *out_fit_broaden,
                                                                          void *us_hydrodyn,
                                                                          QWidget *p,
                                                                          const char *
                                                                          ) : QDialog( p )
{
   this->names              = names;
   this->prepop_conc        = prepop_conc;
   this->out_name_to_conc   = out_name_to_conc;
   this->out_selected_names = out_selected_names;
   this->out_ok             = out_ok;
   this->out_primus_mode    = out_primus_mode;
   this->out_show_regplots  = out_show_regplots;
   this->out_fit_broaden    = out_fit_broaden;
   this->us_hydrodyn        = us_hydrodyn;

   *out_ok = false;

   disable_updates = true;

   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setWindowTitle( us_tr( "US-SOMO: SAXS: Assign concentrations for extrapolation to zero concentration" ) );
   setModal( true );

   setupGUI();

   disable_updates = false;

   // default to all curves selected (preserves the prior "extrapolate all" behavior);
   // selectAll() emits itemSelectionChanged which drives validate_all_rows()
   t_conc->selectAll();
   validate_all_rows();

   // curve names can get quite long (e.g. derived HPLC/KIN frame names), so size
   // the window width to the longest name rather than using a fixed width
   int max_name_len = 0;
   for ( int i = 0; i < names.size(); i++ )
   {
      QString display_name = names[ i ];
      display_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );
      if ( display_name.length() > max_name_len )
      {
         max_name_len = display_name.length();
      }
   }
   int width = qBound( 600, 250 + max_name_len * 8, 1400 );

   setGeometry( 200, 150, width, 100 + 30 * ( names.size() + 7 ) );
}

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::~US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc()
{
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::setupGUI()
{
   int minHeight1 = 30;

   lbl_info = new QLabel(
                         us_tr( "Enter a non-negative concentration for each curve, then select (highlight)\n"
                                "the rows to use. Extrapolation needs at least 3 selected curves." ),
                         this
                         );
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1 * 2);
   lbl_info->setPalette( PALET_LABEL );
   AUTFBACK( lbl_info );
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   t_conc = new QTableWidget( names.size(), 2, this );
   t_conc->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   t_conc->setPalette( PALET_EDIT );
   AUTFBACK( t_conc );
   t_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   t_conc->setHorizontalHeaderLabels( QStringList() << us_tr( "Curve Name" ) << us_tr( "Concentration" ) );
   t_conc->setSortingEnabled( false );
   t_conc->verticalHeader()->hide();
   t_conc->horizontalHeader()->setSectionResizeMode( 0, QHeaderView::Stretch );
   t_conc->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::ResizeToContents );

   // the selected (highlighted) rows are the curves that get extrapolated; whole-row
   // selection, multiple rows, concentrations still editable by double-clicking a cell
   t_conc->setSelectionBehavior( QAbstractItemView::SelectRows );
   t_conc->setSelectionMode( QAbstractItemView::ExtendedSelection );

   populate_table();

   connect( t_conc, SIGNAL( itemChanged( QTableWidgetItem * ) ), SLOT( cell_changed( QTableWidgetItem * ) ) );
   connect( t_conc, SIGNAL( itemSelectionChanged() ), SLOT( selection_changed() ) );

   cb_primus = new QCheckBox( us_tr( "Primus mode (extrapolate absolute intensity, ATSAS almerge-style)" ), this );
   cb_primus->setChecked( false );
   cb_primus->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_primus->setPalette( PALET_NORMAL );
   AUTFBACK( cb_primus );
   cb_primus->setMinimumHeight( minHeight1 );
   cb_primus->setToolTip(
                         us_tr( "Unchecked (Zimm mode): output the concentration-normalized intensity I(q)/c\n"
                                "extrapolated to c=0 (SAXS analogue of a Zimm plot); tagged Conc:1.\n\n"
                                "Checked (Primus mode): scale each curve onto the highest-concentration curve,\n"
                                "then combine them with an inverse-variance weighted merge -- matches ATSAS\n"
                                "almerge; the output carries the reference curve's error bars and absolute scale." ) );

   cb_regplots = new QCheckBox( us_tr( "Show per-q regression plots (scrollable pop-up)" ), this );
   cb_regplots->setChecked( false );
   cb_regplots->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_regplots->setPalette( PALET_NORMAL );
   AUTFBACK( cb_regplots );
   cb_regplots->setMinimumHeight( minHeight1 );
   cb_regplots->setToolTip(
                           us_tr( "After extrapolating, open a pop-up showing the per-q linear regression\n"
                                  "(the concentration data points with error bars, the fit, and the c=0\n"
                                  "intercept), scrollable q-by-q with a wheel." ) );

   // Zimm fit-broadening: couple neighbouring q by smoothing the concentration slope
   QHBoxLayout * hbl_broaden = new QHBoxLayout; hbl_broaden->setContentsMargins( 4, 0, 4, 0 ); hbl_broaden->setSpacing( 6 );
   lbl_broaden = new QLabel( us_tr( "Zimm fit broadening (q-window, 0 = off):" ), this );
   lbl_broaden->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_broaden->setMinimumHeight( minHeight1 );
   lbl_broaden->setPalette( PALET_LABEL );
   AUTFBACK( lbl_broaden );
   lbl_broaden->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   lbl_broaden->setToolTip(
                           us_tr( "Zimm mode only. 0 = independent per-q fits. A value N > 1 averages the\n"
                                  "concentration slope over an N-point q-window before taking the c=0\n"
                                  "intercept -- the interaction term varies smoothly with q, so this reduces\n"
                                  "extrapolation noise without smearing the form-factor detail in the intercept." ) );

   le_broaden = new QLineEdit( this );
   le_broaden->setText( "0" );
   le_broaden->setValidator( new QIntValidator( 0, 999, le_broaden ) );
   le_broaden->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   le_broaden->setMinimumHeight( minHeight1 );
   le_broaden->setMaximumWidth( 70 );
   le_broaden->setPalette( PALET_NORMAL );
   AUTFBACK( le_broaden );
   le_broaden->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );
   // broadening applies to Zimm mode only; disable it while Primus mode is checked
   connect( cb_primus, SIGNAL( toggled( bool ) ), le_broaden, SLOT( setDisabled( bool ) ) );
   connect( cb_primus, SIGNAL( toggled( bool ) ), lbl_broaden, SLOT( setDisabled( bool ) ) );

   hbl_broaden->addWidget( lbl_broaden );
   hbl_broaden->addWidget( le_broaden );
   hbl_broaden->addStretch( 1 );

   lbl_status = new QLabel( "", this );
   lbl_status->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_status->setMinimumHeight(minHeight1);
   lbl_status->setPalette( PALET_LABEL );
   AUTFBACK( lbl_status );
   lbl_status->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   pb_help = new QPushButton(us_tr("Help"), this);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(us_tr("Cancel"), this);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_ok = new QPushButton(us_tr("Extrapolate"), this);
   pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_ok->setMinimumHeight(minHeight1);
   pb_ok->setPalette( PALET_PUSHB );
   connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

   QHBoxLayout * hbl_bottom = new QHBoxLayout; hbl_bottom->setContentsMargins( 0, 0, 0, 0 ); hbl_bottom->setSpacing( 0 );
   hbl_bottom->addWidget( pb_help );
   hbl_bottom->addWidget( pb_cancel );
   hbl_bottom->addWidget( pb_ok );

   QVBoxLayout * background = new QVBoxLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 );
   background->addWidget( lbl_info );
   background->addWidget( t_conc );
   background->addWidget( cb_primus );
   background->addWidget( cb_regplots );
   background->addLayout( hbl_broaden );
   background->addWidget( lbl_status );
   background->addLayout( hbl_bottom );
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::populate_table()
{
   disable_updates = true;
   for ( int i = 0; i < names.size(); i++ )
   {
      QString display_name = names[ i ];
      display_name.remove( QRegularExpression( "^\"" ) ).remove( QRegularExpression( "\"$" ) );

      QTableWidgetItem *name_item = new QTableWidgetItem( display_name );
      name_item->setFlags( name_item->flags() & ~Qt::ItemIsEditable );
      t_conc->setItem( i, 0, name_item );

      QString conc_text = "";
      if ( prepop_conc.count( names[ i ] ) )
      {
         conc_text = QString( "%1" ).arg( prepop_conc[ names[ i ] ] );
      }
      t_conc->setItem( i, 1, new QTableWidgetItem( conc_text ) );
   }
   t_conc->resizeColumnsToContents();
   disable_updates = false;
}

bool US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::validate_all_rows()
{
   // only the selected (highlighted) rows drive extrapolation; a row's concentration
   // matters only when that row is selected. Invalid concentrations are flagged red
   // regardless, but they only block the action when the row is selected.
   int selected_count   = 0;
   int selected_invalid = 0;

   for ( int i = 0; i < t_conc->rowCount(); i++ )
   {
      QTableWidgetItem *item = t_conc->item( i, 1 );
      bool ok = false;
      double val = item->text().toDouble( &ok );
      bool row_valid = ok && val >= 0e0;

      item->setBackground( row_valid ? QColor( 255, 255, 255 ) : QColor( 255, 200, 200 ) );

      if ( t_conc->item( i, 0 )->isSelected() )
      {
         selected_count++;
         if ( !row_valid )
         {
            selected_invalid++;
         }
      }
   }

   bool can_extrap = selected_count >= 3 && selected_invalid == 0;

   QString status;
   if ( selected_count < 3 )
   {
      status = QString( us_tr( "Select at least 3 curves (%1 selected)" ) ).arg( selected_count );
   }
   else if ( selected_invalid )
   {
      status = QString( us_tr( "%1 selected, but %2 have an invalid concentration" ) )
         .arg( selected_count ).arg( selected_invalid );
   }
   else
   {
      status = QString( us_tr( "%1 curves selected for extrapolation" ) ).arg( selected_count );
   }
   lbl_status->setText( status );

   pb_ok->setEnabled( can_extrap );

   return can_extrap;
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::cell_changed( QTableWidgetItem * )
{
   if ( disable_updates )
   {
      return;
   }
   validate_all_rows();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::selection_changed()
{
   if ( disable_updates )
   {
      return;
   }
   validate_all_rows();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::ok()
{
   if ( !validate_all_rows() )
   {
      return;
   }

   out_name_to_conc->clear();
   out_selected_names->clear();
   for ( int i = 0; i < names.size(); i++ )
   {
      if ( !t_conc->item( i, 0 )->isSelected() )
      {
         continue;
      }
      ( *out_name_to_conc )[ names[ i ] ] = t_conc->item( i, 1 )->text().toDouble();
      *out_selected_names << names[ i ];
   }
   *out_primus_mode   = cb_primus->isChecked();
   *out_show_regplots = cb_regplots->isChecked();
   *out_fit_broaden   = cb_primus->isChecked() ? 0 : le_broaden->text().toInt();
   *out_ok = true;
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::cancel()
{
   *out_ok = false;
   close();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::help()
{
   US_Help *online_help;
   online_help = new US_Help( this );
   online_help->show_help( "manual/somo/somo_saxs_extrap_c0.html" );
}
