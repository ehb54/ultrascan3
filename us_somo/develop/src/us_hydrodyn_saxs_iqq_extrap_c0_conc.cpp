#include "../include/us_hydrodyn_saxs_iqq_extrap_c0_conc.h"
#include "../include/us_hydrodyn.h"
#include <QRegularExpression>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QColor>
#include <QHeaderView>
#include <QIntValidator>
#include <QDoubleValidator>

US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc::US_Hydrodyn_Saxs_Iqq_Extrap_C0_Conc(
                                                                          QStringList names,
                                                                          map < QString, double > prepop_conc,
                                                                          map < QString, double > *out_name_to_conc,
                                                                          QStringList *out_selected_names,
                                                                          bool *out_ok,
                                                                          bool *out_ref_scale,
                                                                          bool *out_merge_ref,
                                                                          bool *out_show_regplots,
                                                                          int *out_fit_broaden,
                                                                          bool *out_gcv,
                                                                          bool *out_use_sd_weights,
                                                                          int *out_model,
                                                                          bool *out_recompute_inputs,
                                                                          int *out_recompute_inputs_mode,
                                                                          int *out_sd_mode,
                                                                          bool *out_discard_outlier,
                                                                          double *out_outlier_sigma,
                                                                          double *out_outlier_chi2_ratio,
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
   this->out_ref_scale        = out_ref_scale;
   this->out_merge_ref        = out_merge_ref;
   this->out_show_regplots  = out_show_regplots;
   this->out_fit_broaden    = out_fit_broaden;
   this->out_gcv            = out_gcv;
   this->out_use_sd_weights = out_use_sd_weights;
   this->out_model          = out_model;
   this->out_recompute_inputs      = out_recompute_inputs;
   this->out_recompute_inputs_mode = out_recompute_inputs_mode;
   this->out_sd_mode        = out_sd_mode;
   this->out_discard_outlier    = out_discard_outlier;
   this->out_outlier_sigma      = out_outlier_sigma;
   this->out_outlier_chi2_ratio = out_outlier_chi2_ratio;
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

   setGeometry( 200, 150, width, 100 + 30 * ( names.size() + 13 ) );
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

   cb_ref_scale = new QCheckBox( us_tr( "Output on reference (absolute) scale" ), this );
   cb_ref_scale->setChecked( false );
   cb_ref_scale->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_ref_scale->setPalette( PALET_NORMAL );
   AUTFBACK( cb_ref_scale );
   cb_ref_scale->setMinimumHeight( minHeight1 );
   cb_ref_scale->setToolTip(
                         us_tr( "Unchecked: output the concentration-normalized intensity I(q)/c (or its\n"
                                "reciprocal-model equivalent) extrapolated to c=0; tagged Conc:1.\n\n"
                                "Checked: multiply the extrapolated curve by the highest concentration so the\n"
                                "output is on that reference curve's absolute intensity scale, tagged with the\n"
                                "reference concentration. This is a pure output-scale choice; it does not change\n"
                                "the fit and works with any concentration model." ) );

   cb_merge = new QCheckBox( us_tr( "Splice reference curve at high q (almerge-style switchover)" ), this );
   cb_merge->setChecked( false );
   cb_merge->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_merge->setPalette( PALET_NORMAL );
   AUTFBACK( cb_merge );
   cb_merge->setMinimumHeight( minHeight1 );
   cb_merge->setToolTip(
                         us_tr( "Unchecked: the extrapolated curve is used at every q.\n\n"
                                "Checked: above an automatically-located merge point (the first q where the\n"
                                "extrapolation agrees with the highest-concentration reference within its errors),\n"
                                "the reference curve is copied verbatim, carrying its error bars -- replacing the\n"
                                "noisy high-q extrapolated tail with the cleaner reference data (the ATSAS almerge\n"
                                "idea). Independent of the scale option above." ) );

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

   cb_weight = new QCheckBox( us_tr( "Weight regression by curve errors (1/sigma\302\262)" ), this );
   cb_weight->setChecked( true );
   cb_weight->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_weight->setPalette( PALET_NORMAL );
   AUTFBACK( cb_weight );
   cb_weight->setMinimumHeight( minHeight1 );
   cb_weight->setToolTip(
                      us_tr( "Checked (default): fit each concentration regression with inverse-variance weights\n"
                             "(1/sigma^2), sigma propagated onto the fit axis from each curve's error column, and\n"
                             "propagate sigma into the extrapolated intercept's error bar. Statistically optimal\n"
                             "when the error bars are reliable.\n\n"
                             "Unchecked: ordinary (unweighted) least squares -- the curve errors are still shown\n"
                             "in the regression plots but are not used in the fit, and the intercept error bar is\n"
                             "the fit's residual scatter. More robust when the error bars are missing or mis-scaled\n"
                             "(common in SAXS). A large weighted-vs-unweighted difference is itself a sign the\n"
                             "errors are driving the result. This affects only the concentration regression, not\n"
                             "the high-q reference merge test or the post-fit SD reassessment." ) );

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
   cb_model->addItem( us_tr( "Additive  I(q)/c  \342\200\224 cleanest low-q profile" ) );
   cb_model->addItem( us_tr( "Reciprocal  c/I(q)  \342\200\224 unbiased MW / I(0)  (default)" ) );
   cb_model->addItem( us_tr( "2nd-order virial  c/I(q)  \342\200\224 MW at strong interaction (needs \342\211\2654-5 conc)" ) );
   cb_model->setCurrentIndex( 1 );
   cb_model->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_model->setPalette( PALET_NORMAL );
   AUTFBACK( cb_model );
   cb_model->setMinimumHeight( minHeight1 );
   cb_model->setToolTip(
                        us_tr( "Applies to all output conventions (normalized, reference-scale, and/or spliced).\n"
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

   // Optional preprocessing: reassess each INPUT curve's error bars (same BayesApp engine as
   // "Recompute output SDs" / the s.d.util "SD rescale" button) before the fit, so better errors
   // feed the outlier check, the 1/sigma^2 weights and the reference merge test. Transient: the
   // loaded curves are not modified (use the s.d.util "SD rescale" button to apply permanently).
   cb_recompute_inputs = new QCheckBox( us_tr( "Recompute input curve SDs before fitting (transient)" ), this );
   cb_recompute_inputs->setChecked( false );
   cb_recompute_inputs->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_recompute_inputs->setPalette( PALET_NORMAL );
   AUTFBACK( cb_recompute_inputs );
   cb_recompute_inputs->setMinimumHeight( minHeight1 );
   cb_recompute_inputs->setToolTip(
                        us_tr( "Off (default): use each input curve's error bars as loaded.\n\n"
                               "On: before extrapolating, reassess every selected curve's errors from its own\n"
                               "point-to-point scatter (the BayesApp method), and use those errors for this\n"
                               "extrapolation only -- the loaded curves are NOT changed (to change them\n"
                               "permanently, use the s.d.util \"SD rescale\" button instead). Most useful together\n"
                               "with error weighting on; with weighting off it only affects the plotted error bars\n"
                               "and the high-q reference merge test." ) );

   QHBoxLayout * hbl_ri = new QHBoxLayout; hbl_ri->setContentsMargins( 4, 0, 4, 0 ); hbl_ri->setSpacing( 6 );
   lbl_recompute_inputs_mode = new QLabel( us_tr( "Input SD recompute mode:" ), this );
   lbl_recompute_inputs_mode->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_recompute_inputs_mode->setMinimumHeight( minHeight1 );
   lbl_recompute_inputs_mode->setPalette( PALET_LABEL );
   AUTFBACK( lbl_recompute_inputs_mode );
   lbl_recompute_inputs_mode->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );

   cb_recompute_inputs_mode = new QComboBox( this );
   cb_recompute_inputs_mode->addItem( us_tr( "Constant  \342\200\224 one factor for all q  (default)" ) );
   cb_recompute_inputs_mode->addItem( us_tr( "Non-constant  \342\200\224 per q-bin factor" ) );
   cb_recompute_inputs_mode->addItem( us_tr( "Intensity-dependent  \342\200\224 sd \342\206\222 sd + a\302\267|I|" ) );
   cb_recompute_inputs_mode->setCurrentIndex( 0 );
   cb_recompute_inputs_mode->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_recompute_inputs_mode->setPalette( PALET_NORMAL );
   AUTFBACK( cb_recompute_inputs_mode );
   cb_recompute_inputs_mode->setMinimumHeight( minHeight1 );
   cb_recompute_inputs_mode->setToolTip(
                        us_tr( "How each input curve's errors are reassessed (same modes as the output SD\n"
                               "reassessment). Constant rescales every sigma by one factor and is the safest\n"
                               "default; Non-constant allows a per-q-bin correction; Intensity-dependent adds an\n"
                               "intensity-proportional term." ) );

   hbl_ri->addWidget( lbl_recompute_inputs_mode );
   hbl_ri->addWidget( cb_recompute_inputs_mode, 1 );

   // Optional post-fit reassessment of the extrapolated curve's error bars, reusing the
   // s.d.util "SD rescale" engine (US_Saxs_Util::recompute_errors, BayesApp-style). Off by
   // default: the propagated errors are kept. The other modes reassess I(q)'s own
   // point-to-point scatter and rescale sigma (Constant = one factor; Non-constant = per
   // q-bin factor; Intensity-dependent = sigma -> sigma + a*|I|).
   QHBoxLayout * hbl_sd_mode = new QHBoxLayout; hbl_sd_mode->setContentsMargins( 4, 0, 4, 0 ); hbl_sd_mode->setSpacing( 6 );
   lbl_sd_mode = new QLabel( us_tr( "Recompute output SDs:" ), this );
   lbl_sd_mode->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_sd_mode->setMinimumHeight( minHeight1 );
   lbl_sd_mode->setPalette( PALET_LABEL );
   AUTFBACK( lbl_sd_mode );
   lbl_sd_mode->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );

   cb_sd_mode = new QComboBox( this );
   cb_sd_mode->addItem( us_tr( "Off  \342\200\224 keep propagated errors  (default)" ) );
   cb_sd_mode->addItem( us_tr( "Constant  \342\200\224 one rescale factor for all q" ) );
   cb_sd_mode->addItem( us_tr( "Non-constant  \342\200\224 per q-bin rescale factor" ) );
   cb_sd_mode->addItem( us_tr( "Intensity-dependent  \342\200\224 sd \342\206\222 sd + a\302\267|I|" ) );
   cb_sd_mode->setCurrentIndex( 0 );
   cb_sd_mode->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_sd_mode->setPalette( PALET_NORMAL );
   AUTFBACK( cb_sd_mode );
   cb_sd_mode->setMinimumHeight( minHeight1 );
   cb_sd_mode->setToolTip(
                        us_tr( "After extrapolating, optionally reassess the output curve's error bars from the\n"
                               "curve's own point-to-point scatter (the s.d.util \"SD rescale\" / BayesApp method),\n"
                               "instead of keeping the analytically propagated errors.\n\n"
                               "Off: keep the propagated errors (default).\n"
                               "Constant: rescale every sigma by a single factor sqrt(reduced chi^2).\n"
                               "Non-constant: rescale per q-bin (captures a q-dependent misestimate).\n"
                               "Intensity-dependent: add an intensity-proportional term, sigma -> sigma + a*|I|.\n\n"
                               "Sigma is only rescaled when the data warrant it (poor fit and >10% off); the\n"
                               "verdict and factor are reported in the log. If the reference is spliced in at\n"
                               "high q, the reassessment covers the whole spliced curve." ) );

   hbl_sd_mode->addWidget( lbl_sd_mode );
   hbl_sd_mode->addWidget( cb_sd_mode, 1 );

   // Robust one-outlier-curve QC. A bad concentration / scale / aggregating curve is off the
   // concentration trend at essentially every q; detected at the curve level and, if this box is
   // checked, discarded from the whole fit (at most one). The two thresholds tune detection: a
   // curve is flagged when its median standardized residual across q exceeds "sigma" (and it is
   // consistently one-signed and well separated from the others), and discarded only if removing
   // it also improves the pooled reduced chi^2 by at least the "min chi^2 gain" factor.
   cb_outlier = new QCheckBox( us_tr( "Automatically discard one outlier concentration (robust QC)" ), this );
   cb_outlier->setChecked( false );
   cb_outlier->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   cb_outlier->setPalette( PALET_NORMAL );
   AUTFBACK( cb_outlier );
   cb_outlier->setMinimumHeight( minHeight1 );
   cb_outlier->setToolTip(
                        us_tr( "Off (default): use every selected curve.\n\n"
                               "On: if one concentration curve is consistently off the concentration trend across q\n"
                               "(a mis-entered concentration, a scaling error, or an aggregating curve), discard that\n"
                               "single curve before fitting -- so it cannot bias the extrapolation or be picked as the\n"
                               "reference. At most one curve is removed, only when enough curves remain (>=3, or >=4 for\n"
                               "the 2nd-order virial). The excluded curve is drawn as a red x in the regression plots and\n"
                               "reported in the log. Whether or not this is checked, a strong outlier is always flagged in\n"
                               "the log. Targets concentration/scale outliers; aggregation is caught by the Guinier check." ) );

   QHBoxLayout * hbl_outlier = new QHBoxLayout; hbl_outlier->setContentsMargins( 4, 0, 4, 0 ); hbl_outlier->setSpacing( 6 );
   lbl_outlier_sigma = new QLabel( us_tr( "Outlier threshold (sigma):" ), this );
   lbl_outlier_sigma->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_outlier_sigma->setMinimumHeight( minHeight1 );
   lbl_outlier_sigma->setPalette( PALET_LABEL );
   AUTFBACK( lbl_outlier_sigma );
   lbl_outlier_sigma->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   lbl_outlier_sigma->setToolTip( us_tr( "Median standardized residual (across q) above which a curve is flagged as an outlier." ) );

   le_outlier_sigma = new QLineEdit( this );
   le_outlier_sigma->setText( "3.0" );
   le_outlier_sigma->setValidator( new QDoubleValidator( 0.1, 100.0, 2, le_outlier_sigma ) );
   le_outlier_sigma->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   le_outlier_sigma->setMinimumHeight( minHeight1 );
   le_outlier_sigma->setMaximumWidth( 70 );
   le_outlier_sigma->setPalette( PALET_NORMAL );
   AUTFBACK( le_outlier_sigma );
   le_outlier_sigma->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );

   lbl_outlier_chi2 = new QLabel( us_tr( "Min chi\302\262 gain to discard:" ), this );
   lbl_outlier_chi2->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
   lbl_outlier_chi2->setMinimumHeight( minHeight1 );
   lbl_outlier_chi2->setPalette( PALET_LABEL );
   AUTFBACK( lbl_outlier_chi2 );
   lbl_outlier_chi2->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1 ) );
   lbl_outlier_chi2->setToolTip( us_tr( "Required factor by which removing the curve must improve the pooled reduced chi^2 before it is actually discarded." ) );

   le_outlier_chi2 = new QLineEdit( this );
   le_outlier_chi2->setText( "1.5" );
   le_outlier_chi2->setValidator( new QDoubleValidator( 1.0, 100.0, 2, le_outlier_chi2 ) );
   le_outlier_chi2->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
   le_outlier_chi2->setMinimumHeight( minHeight1 );
   le_outlier_chi2->setMaximumWidth( 70 );
   le_outlier_chi2->setPalette( PALET_NORMAL );
   AUTFBACK( le_outlier_chi2 );
   le_outlier_chi2->setFont( QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize ) );

   hbl_outlier->addWidget( lbl_outlier_sigma );
   hbl_outlier->addWidget( le_outlier_sigma );
   hbl_outlier->addSpacing( 16 );
   hbl_outlier->addWidget( lbl_outlier_chi2 );
   hbl_outlier->addWidget( le_outlier_chi2 );
   hbl_outlier->addStretch( 1 );

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
   connect( cb_ref_scale, SIGNAL( toggled( bool ) ), SLOT( refresh_broaden_enabled() ) );
   connect( cb_merge,     SIGNAL( toggled( bool ) ), SLOT( refresh_broaden_enabled() ) );
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
   background->addWidget( cb_ref_scale );
   background->addWidget( cb_merge );
   background->addWidget( cb_gcv );
   background->addWidget( cb_weight );
   background->addWidget( cb_regplots );
   background->addLayout( hbl_model );
   background->addWidget( cb_recompute_inputs );
   background->addLayout( hbl_ri );
   background->addLayout( hbl_sd_mode );
   background->addWidget( cb_outlier );
   background->addLayout( hbl_outlier );
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
   bool enable = !cb_ref_scale->isChecked() && !cb_merge->isChecked() && !cb_gcv->isChecked();
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
   *out_ref_scale       = cb_ref_scale->isChecked();
   *out_merge_ref       = cb_merge->isChecked();
   *out_show_regplots = cb_regplots->isChecked();
   *out_gcv           = cb_gcv->isChecked();
   *out_use_sd_weights = cb_weight->isChecked();
   *out_model         = cb_model->currentIndex();
   *out_recompute_inputs      = cb_recompute_inputs->isChecked();
   *out_recompute_inputs_mode = cb_recompute_inputs_mode->currentIndex();
   *out_sd_mode       = cb_sd_mode->currentIndex();
   *out_discard_outlier = cb_outlier->isChecked();
   {
      bool   ok_s = false, ok_c = false;
      double s = le_outlier_sigma->text().toDouble( &ok_s );
      double c = le_outlier_chi2 ->text().toDouble( &ok_c );
      // sigma is a standardized-residual threshold (can be well below 1 for mild outliers);
      // the chi^2 gain is a ratio, so values < 1 (removal makes the fit worse) are meaningless
      *out_outlier_sigma      = ( ok_s && s >  0e0 ) ? s : 3.0e0;
      *out_outlier_chi2_ratio = ( ok_c && c >= 1e0 ) ? c : 1.0e0;
   }
   // the manual q-window is used only for classic Zimm (no absolute-scale, no GCV)
   *out_fit_broaden   = ( cb_ref_scale->isChecked() || cb_merge->isChecked() || cb_gcv->isChecked() ) ? 0 : le_broaden->text().toInt();
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
