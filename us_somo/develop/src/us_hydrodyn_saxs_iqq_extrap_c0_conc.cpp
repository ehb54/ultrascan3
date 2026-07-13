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
                                                                          bool *out_absolute_mode,
                                                                          bool *out_show_regplots,
                                                                          int *out_fit_broaden,
                                                                          bool *out_gcv,
                                                                          int *out_model,
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
   this->out_absolute_mode    = out_absolute_mode;
   this->out_show_regplots  = out_show_regplots;
   this->out_fit_broaden    = out_fit_broaden;
   this->out_gcv            = out_gcv;
   this->out_model          = out_model;
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

   cb_absolute = new QCheckBox( us_tr( "Absolute-scale mode (extrapolate absolute intensity, reference-scaled)" ), this );
   cb_absolute->setChecked( false );
   cb_absolute->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_absolute->setPalette( PALET_NORMAL );
   AUTFBACK( cb_absolute );
   cb_absolute->setMinimumHeight( minHeight1 );
   cb_absolute->setToolTip(
                         us_tr( "Unchecked (Zimm mode): output the concentration-normalized intensity I(q)/c\n"
                                "extrapolated to c=0 (SAXS analogue of a Zimm plot); tagged Conc:1.\n\n"
                                "Checked (Absolute-scale mode): scale each curve onto the highest-concentration\n"
                                "(reference) curve, extrapolate to c=0 at low q, and take the reference curve\n"
                                "verbatim above the merging point; the output is on the reference's absolute\n"
                                "intensity scale and carries the reference curve's error bars above the merge\n"
                                "(and the propagated fit error below it)." ) );

   cb_gcv = new QCheckBox( us_tr( "Automatic slope regularization (GCV) -- recommended" ), this );
   cb_gcv->setChecked( true );
   cb_gcv->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_gcv->setPalette( PALET_NORMAL );
   AUTFBACK( cb_gcv );
   cb_gcv->setMinimumHeight( minHeight1 );
   cb_gcv->setToolTip(
                      us_tr( "Applies to both modes. Fits all q jointly with a smoothness penalty on only the\n"
                             "concentration slope (the interparticle term, smooth in q) while leaving the\n"
                             "extrapolated intensity's form-factor detail intact; the smoothing strength is\n"
                             "chosen automatically by Generalized Cross-Validation (no tuning). This denoises\n"
                             "the low-q extrapolation and supersedes the manual q-window below.\n"
                             "Uncheck for the classic independent per-q fits (plus any manual window)." ) );

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

   // Concentration-dependence model (Zimm mode only): how I(q,c) is fit vs c at each q.
   // Additive I/c is unbiased for the profile but reads MW low for a saturating structure
   // factor; the reciprocal c/I (second-virial form) is linear in c and gives an unbiased
   // MW/I(0); the 2nd-order virial absorbs the residual curvature at strong interaction.
   QHBoxLayout * hbl_model = new QHBoxLayout; hbl_model->setContentsMargins( 4, 0, 4, 0 ); hbl_model->setSpacing( 6 );
   lbl_model = new QLabel( us_tr( "Concentration model:" ), this );
   lbl_model->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_model->setMinimumHeight( minHeight1 );
   lbl_model->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model );
   lbl_model->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );

   cb_model = new QComboBox( this );
   cb_model->addItem( us_tr( "Additive  I(q)/c  \342\200\224 cleanest low-q profile (default)" ) );
   cb_model->addItem( us_tr( "Reciprocal  c/I(q)  \342\200\224 unbiased MW / I(0)" ) );
   cb_model->addItem( us_tr( "2nd-order virial  c/I(q)  \342\200\224 MW at strong interaction (needs \342\211\2654-5 conc)" ) );
   cb_model->setCurrentIndex( 0 );
   cb_model->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_model->setPalette( PALET_NORMAL );
   AUTFBACK( cb_model );
   cb_model->setMinimumHeight( minHeight1 );
   cb_model->setToolTip(
                        us_tr( "Zimm mode only (absolute-scale mode always uses the additive fit).\n"
                               "Additive: fit I(q)/c vs c, intercept = dilute I(q)/c. Best profile; MW biased\n"
                               "low when interparticle repulsion/attraction is strong.\n"
                               "Reciprocal: fit c/I(q) vs c (the second-virial form, linear in c); I0 = 1/intercept.\n"
                               "Unbiased forward scatter, so a more accurate molecular weight.\n"
                               "2nd-order virial: adds a c^2 term for strongly interacting series; needs 4-5\n"
                               "well-spread concentrations. Validated against a Percus-Yevick hard-sphere model." ) );
   // a power-user default may preselect the model via gparam saxs_extrap_c0_model
   {
      US_Hydrodyn *uh = (US_Hydrodyn *) us_hydrodyn;
      if ( uh && uh->gparams.count( "saxs_extrap_c0_model" ) )
      {
         int m = uh->gparams[ "saxs_extrap_c0_model" ].toInt();
         if ( m >= 0 && m <= 2 ) { cb_model->setCurrentIndex( m ); }
      }
   }

   hbl_model->addWidget( lbl_model );
   hbl_model->addWidget( cb_model, 1 );

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
   // the manual broadening window applies to Zimm mode only and is superseded by GCV;
   // disable it whenever absolute-scale or automatic GCV regularization is selected
   connect( cb_absolute, SIGNAL( toggled( bool ) ), SLOT( refresh_broaden_enabled() ) );
   connect( cb_gcv,    SIGNAL( toggled( bool ) ), SLOT( refresh_broaden_enabled() ) );

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
   background->addWidget( cb_absolute );
   background->addWidget( cb_gcv );
   background->addWidget( cb_regplots );
   background->addLayout( hbl_model );
   background->addLayout( hbl_broaden );
   background->addWidget( lbl_status );
   background->addLayout( hbl_bottom );

   // reflect the default state (GCV on => manual window disabled)
   refresh_broaden_enabled();
}

void US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::refresh_broaden_enabled()
{
   // the manual fit-broadening window is meaningful only for Zimm mode with automatic
   // GCV off; absolute-scale and GCV each supersede it
   bool enable = !cb_absolute->isChecked() && !cb_gcv->isChecked();
   le_broaden->setEnabled( enable );
   lbl_broaden->setEnabled( enable );
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
   *out_absolute_mode   = cb_absolute->isChecked();
   *out_show_regplots = cb_regplots->isChecked();
   *out_gcv           = cb_gcv->isChecked();
   *out_model         = cb_model->currentIndex();
   // the manual q-window is used only for classic Zimm (no absolute-scale, no GCV)
   *out_fit_broaden   = ( cb_absolute->isChecked() || cb_gcv->isChecked() ) ? 0 : le_broaden->text().toInt();
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
