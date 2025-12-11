//! \file us_adv_dmgamc.cpp

#include "us_adv_dmgamc.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "qwt_legend.h"

// constructor:  advanced analysis control widget
US_AdvDmgaMc::US_AdvDmgaMc( US_Model* amodel,
   QVector< US_Model >& aimodels,
   QMap< QString, QString >& adv_vals, QWidget* p ) :
   US_WidgetsDialog( p, Qt::WindowFlags() ), imodels( aimodels ), parmap( adv_vals )
{
   model          = amodel;
   parentw        = p;
   umodel         = *model;
   lddesc         = umodel.description;

   setObjectName( "US_AdvDmgaMC" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "DMGA-MC Advanced Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   paramLayout     = new QGridLayout();
   plotLayout      = new US_Plot( data_plot,
                                  tr( "Attribute Distribution" ),
                                  tr( "Attribute Name" ),
                                  tr( "Frequency" ) );

   data_plot->setCanvasBackground( Qt::black );

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_modelsim  = us_banner( tr( "Simulate data using parameters from "                                          "Monte Carlo statistics or from "
                                         "current iteration model" ) );

   QButtonGroup* um_group = new QButtonGroup( this );
   lo_mean      = us_radiobutton( tr( "Mean"          ), rb_mean,   true  );
   lo_median    = us_radiobutton( tr( "Median"        ), rb_median, false );
   lo_mode      = us_radiobutton( tr( "Mode"          ), rb_mode,   false );
   lo_curmod    = us_radiobutton( tr( "Current Model" ), rb_curmod, false );
   um_group->addButton( rb_mean,   0 );
   um_group->addButton( rb_median, 1 );
   um_group->addButton( rb_mode,   2 );
   um_group->addButton( rb_curmod, 3 );
   QPushButton* pb_help     = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_simulate = us_pushbutton( tr( "Simulate" ) );
   QPushButton* pb_close    = us_pushbutton( tr( "Close" ) );

   le_modtype           = us_lineedit( "", -1, true );
   QPalette mtpal       = le_modtype->palette();
   mtpal.setColor( QPalette::Text, Qt::blue );
   le_modtype->setPalette( mtpal );

   pb_nextmodel             = us_pushbutton( tr( "Next Model"   ) );
   QPushButton* pb_nextparm = us_pushbutton( tr( "Next Parameter >" ) );
   QPushButton* pb_prevparm = us_pushbutton( tr( "< Previous Parameter" ) );
   cb_params            = us_comboBox();
   ls_params .clear();
   ls_params << tr( "Component 1 Sedimentation Coefficient" );
   ls_params << tr( "Component 1 Diffusion Coefficient" );
   ls_params << tr( "Component 1 Molecular Weight" );
   ls_params << tr( "Component 1 Frictional Ratio" );
   ls_params << tr( "Component 1 Vbar (20_W)" );
   ls_params << tr( "Component 1 Partial Concentration" );
   ls_params << tr( "Reaction 1 k_Dissociation:" );
   ls_params << tr( "Reaction 1 k_off Rate:" );
   cb_params->addItems( ls_params );

   ct_modelnbr  = us_counter( 2, 1,  imodels.size(),     1 );
//ct_modelnbr  = us_counter( 2, 1,  1000,     1 );

   QLabel* lb_ms_mean   = us_label(  tr( "Mean" ) );
   QLabel* lb_ms_95lo   = us_label(  tr( "95% Conf.Low" ) );
   QLabel* lb_ms_95hi   = us_label(  tr( "95% Conf.High" ) );
   QLabel* lb_ms_medi   = us_label(  tr( "Median" ) );
   QLabel* lb_ms_mode   = us_label(  tr( "Mode" ) );
           lb_ms_iter   = us_label(  tr( "Iteration 25" ) );
   le_ms_mean   = us_lineedit( "", -1, true );
   le_ms_95lo   = us_lineedit( "", -1, true );
   le_ms_95hi   = us_lineedit( "", -1, true );
   le_ms_medi   = us_lineedit( "", -1, true );
   le_ms_mode   = us_lineedit( "", -1, true );
   le_ms_iter   = us_lineedit( "", -1, true );

   int row      = 0;
   paramLayout->addWidget( lb_ms_mean,   row,    0, 1,  2 );
   paramLayout->addWidget( lb_ms_95lo,   row,    2, 1,  2 );
   paramLayout->addWidget( lb_ms_95hi,   row,    4, 1,  2 );
   paramLayout->addWidget( lb_ms_medi,   row,    6, 1,  2 );
   paramLayout->addWidget( lb_ms_mode,   row,    8, 1,  2 );
   paramLayout->addWidget( lb_ms_iter,   row++, 10, 1,  2 );
   paramLayout->addWidget( le_ms_mean,   row,    0, 1,  2 );
   paramLayout->addWidget( le_ms_95lo,   row,    2, 1,  2 );
   paramLayout->addWidget( le_ms_95hi,   row,    4, 1,  2 );
   paramLayout->addWidget( le_ms_medi,   row,    6, 1,  2 );
   paramLayout->addWidget( le_ms_mode,   row,    8, 1,  2 );
   paramLayout->addWidget( le_ms_iter,   row++, 10, 1,  2 );
   paramLayout->addWidget( lb_modelsim,  row++,  0, 1, 12 );
   paramLayout->addLayout( lo_mean,      row,    0, 1,  3 );
   paramLayout->addLayout( lo_median,    row,    3, 1,  3 );
   paramLayout->addLayout( lo_mode,      row,    6, 1,  3 );
   paramLayout->addLayout( lo_curmod,    row++,  9, 1,  3 );
   paramLayout->addWidget( le_modtype,   row,    0, 1,  6 );
   paramLayout->addWidget( pb_nextmodel, row,    6, 1,  3 );
   paramLayout->addWidget( ct_modelnbr,  row++,  9, 1,  3 );
   paramLayout->addWidget( pb_prevparm,  row,    0, 1,  3 );
   paramLayout->addWidget( pb_nextparm,  row,    3, 1,  3 );
   paramLayout->addWidget( cb_params,    row++,  6, 1,  6 );
   paramLayout->addWidget( pb_help,      row,    0, 1,  4 );
   paramLayout->addWidget( pb_simulate,  row,    4, 1,  4 );
   paramLayout->addWidget( pb_close,     row++,  8, 1,  4 );

   mainLayout  ->addLayout( plotLayout  );
   mainLayout  ->addLayout( paramLayout );

   ct_modelnbr ->setValue     ( 0 );
   ct_modelnbr ->setSingleStep( 1 );

   pb_nextmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );

   connect( pb_nextmodel, SIGNAL( clicked       ()         ),
            this,         SLOT  ( next_model    ()         ) );
   connect( ct_modelnbr,  SIGNAL( valueChanged  ( double ) ),
            this,         SLOT  ( change_model  ( double ) ) );
   connect( rb_mean,      SIGNAL( toggled       ( bool   ) ),
            this,         SLOT  ( set_model_type( bool   ) ) );
   connect( rb_median,    SIGNAL( toggled       ( bool   ) ),
            this,         SLOT  ( set_model_type( bool   ) ) );
   connect( rb_mode,      SIGNAL( toggled       ( bool   ) ),
            this,         SLOT  ( set_model_type( bool   ) ) );
   connect( rb_curmod,    SIGNAL( toggled       ( bool   ) ),
            this,         SLOT  ( set_model_type( bool   ) ) );
   connect( pb_prevparm,  SIGNAL( clicked       ()         ),
            this,         SLOT  ( prev_param    ()         ) );
   connect( pb_nextparm,  SIGNAL( clicked       ()         ),
            this,         SLOT  ( next_param    ()         ) );
   connect( cb_params,    SIGNAL( activated     ( int )    ),
            this,         SLOT  ( plot_distrib  ()         ) );

   connect( pb_help,      SIGNAL( clicked()  ),
            this,         SLOT  ( help()     ) );
   connect( pb_simulate,  SIGNAL( clicked()  ),
            this,         SLOT  ( simulate() ) );
   connect( pb_close,     SIGNAL( clicked()  ),
            this,         SLOT  ( done()     ) );

   adjustSize();
   QFontMetrics fmet( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() - 1 ) );
   int fwid        = fmet.maxWidth();
   int rhgt        = lb_modelsim->height();
   int csizw       = fwid * 2;
   ct_modelnbr ->resize( csizw, rhgt );
   ct_modelnbr ->setMaximumWidth( csizw * 3 );

   US_DmgaMcStats::build_used_model( "mean", 0, imodels, umodel );

   umodel.description = lddesc;
   int ncomp       = umodel.components.size();
   int nreac       = umodel.associations.size();

   le_modtype ->setText( tr( "Mean model, %1 components, %2 reaction(s)" )
                         .arg( ncomp ).arg( nreac ) );

   ls_params.clear();
  
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      QString comp = tr( "Component %1 " ).arg( ii + 1 );
      ls_params << comp + tr( "Sedimentation Coefficient" );
      ls_params << comp + tr( "Diffusion Coefficient" );
      ls_params << comp + tr( "Molecular Weight" );
      ls_params << comp + tr( "Frictional Ratio" );
      ls_params << comp + tr( "Vbar (20_W)" );
      ls_params << comp + tr( "Partial Concentration" );
   }

   for ( int ii = 0; ii < nreac; ii++ )
   {
      QString reac = tr( "Reaction %1 " ).arg( ii + 1 );
      ls_params << reac + tr( "k_Dissociation:" );
      ls_params << reac + tr( "k_off Rate:" );
   }

   cb_params->clear();
   cb_params->addItems( ls_params );
   cb_params->setCurrentIndex( 0 );
   lb_ms_iter->setText( tr( "Iteration 1" ) );

qDebug() << "AdvD:Pre-adjust size" << size();
   adjustSize();
qDebug() << "AdvD:Post-adjust size" << size();
   resize( 720, 640 );
qDebug() << "AdvD:Post-resize size" << size();

   plot_distrib();

   qApp->processEvents();
}

// Private slot to pass parameters then close with an accepted() signal
void US_AdvDmgaMc::done( void )
{
   parmap[ "modelnbr"  ] = QString::number( ct_modelnbr ->value() );
   parmap[ "modelsim"  ] = rb_mean  ->isChecked() ? "mean"   :
                         ( rb_median->isChecked() ? "median" :
                         ( rb_mode  ->isChecked() ? "mode"   :
                         ( rb_curmod->isChecked() ? "model"  : "" ) ) );

   *model                = umodel;

   accept();
}

// Private slot to advance to the next iteration model
void US_AdvDmgaMc::next_model( void )
{
   int imodl = (int)ct_modelnbr->value();
   int nmodl = imodels.size();

   imodl     = ( imodl < nmodl ) ? ( imodl + 1 ) : 1;
   ct_modelnbr->setValue( (double)imodl );
}

// Private slot to change the iteration number for the used model
void US_AdvDmgaMc::change_model( double mnbr )
{
   int mx        = (int)mnbr - 1;
   umodel        = imodels[ mx ];

   set_model_type( true );
}

// Private slot to set model type and change used model
void US_AdvDmgaMc::set_model_type( bool chekd )
{
   if ( ! chekd )  return;

   bool ck_cmod    = rb_curmod->isChecked();
   bool ck_mean    = rb_mean  ->isChecked();
   bool ck_medi    = rb_median->isChecked();
   bool ck_mode    = rb_mode  ->isChecked();
   int iter        = (int)ct_modelnbr->value();

   QString smtype  = ck_mean ? tr( "mean"   ) : "";
   smtype          = ck_medi ? tr( "median" ) : smtype;
   smtype          = ck_mode ? tr( "mode"   ) : smtype;
   smtype          = ck_cmod ? tr( "model"  ) : smtype;

   US_DmgaMcStats::build_used_model( smtype, iter, imodels, umodel );

   if ( ! ck_cmod )
      umodel.description    = lddesc;
   int ncomp       = umodel.components  .size();
   int nreac       = umodel.associations.size();
   QString mtlabl  = ck_mean ? tr( "Mean"   ) : "";
   mtlabl          = ck_medi ? tr( "Median" ) : mtlabl;
   mtlabl          = ck_mode ? tr( "Mode"   ) : mtlabl;
   mtlabl          = ck_cmod ? tr( "Iteration %1" ).arg( iter ) : mtlabl;
   le_modtype ->setText( mtlabl + tr( " model,  %1 components, %2 reaction(s)" )
                                  .arg( ncomp ).arg( nreac ) );

   pb_nextmodel->setEnabled( ck_cmod );
   ct_modelnbr ->setEnabled( ck_cmod );

   plot_distrib();
}

// Plot distribution for a specified attribute
void US_AdvDmgaMc::plot_distrib()
{
   const QString ytitle = tr( "Frequency" );
   const double pad_lo  = 0.95;
   const double pad_hi  = 1.05;
   QVector< double > xvec_in;
   QVector< double > xvec_pl;
   QVector< double > yvec_pl;

   int iter        = (int)ct_modelnbr->value();
   int nxi         = imodels.size();
   QString attrib  = cb_params->currentText();
   int jc          = attrib.section( " ", 1, 1 ).toInt() - 1;
   int jr          = jc;
   jc              = attrib.contains( tr( "Component" ) ) ? jc : -1;
   jr              = attrib.contains( tr( "Reaction" ) )  ? jr : -1;
   QString xtitle  = attrib.section( " ", 2, -1 ).simplified();
   xvec_in.clear();
   double xvalcm   = 0.0;
   int hx          = iter - 1;

   for ( int ii = 0; ii < nxi; ii++ )
   {  // Accumulate the input model X distribution values
      US_Model::SimulationComponent* sc;
      US_Model::Association* as;
      double xval     = 0.0;

      if ( jr < 0 )
      {  // Get a component attribute value
         sc           = &imodels[ ii ].components[ jc ];
         xval         = sc->s;
         xval         = xtitle.contains( tr( "Diffusion" ) )  ? sc->D : xval;
         xval         = xtitle.contains( tr( "Molecular" ) )  ? sc->mw : xval;
         xval         = xtitle.contains( tr( "Frictional" ) ) ? sc->f_f0 : xval;
         xval         = xtitle.contains( tr( "Vbar" ) )    ? sc->vbar20 : xval;
         xval         = xtitle.contains( tr( "Concentration" ) )
                        ? sc->signal_concentration : xval;
      }

      else
      {  // Get a reaction attribute value
         as           = &imodels[ ii ].associations[ jr ];
         xval         = as->k_d;
         xval         = xtitle.contains( tr( "Rate" ) ) ? as->k_off : xval;
      }

      xvec_in << xval;

      if ( ii == hx )
         xvalcm       = xval;
   }

   // Sort the X values
   std::sort( xvec_in.begin(), xvec_in.end() );
   double* xvi     = xvec_in.data();

   // Build unique X's and their frequencies (Y's)
   xvec_pl.clear();
   yvec_pl.clear();
   double xprev    = xvi[ 0 ];
   double xkmax    = 1.0;
   double xknt     = 1.0;

   for ( int ii = 1; ii < nxi; ii++ )
   {
      double xval     = xvi[ ii ];

      if ( xval == xprev )
      {
         xknt           += 1.0;
         xkmax           = qMax( xkmax, xknt );
      }

      else
      {
         xvec_pl << xprev;
         yvec_pl << xknt;
         xknt             = 1.0;
         xprev            = xval;
      }
   }

   // Store the last pair
   xvec_pl << xprev;
   yvec_pl << xknt;

   double* xvp     = xvec_pl.data();
   double* yvp     = yvec_pl.data();
   int nxyp        = xvec_pl.size();
   double ymax     = xkmax * pad_hi;   // Add a slight pad to Y maximum

   if ( nxyp < 3 )
   {  // If only 1 or 2 points, expand to center points in X
      nxyp           += 2;
      xvec_pl.resize( nxyp );
      yvec_pl.resize( nxyp );

      if ( nxyp == 3 )
      {  // If there was 1, expand to 3
         xvec_pl[ 1 ]    = xvec_pl[ 0 ];
         yvec_pl[ 1 ]    = yvec_pl[ 0 ];
         xvec_pl[ 0 ]    = xvec_pl[ 1 ] * pad_lo;
         yvec_pl[ 0 ]    = 0.0;
         xvec_pl[ 2 ]    = xvec_pl[ 1 ] * pad_hi;
         yvec_pl[ 2 ]    = 0.0;
      }

      else
      {  // If there were 2, expand to 4
         xvec_pl[ 2 ]    = xvec_pl[ 1 ];
         yvec_pl[ 2 ]    = yvec_pl[ 1 ];
         xvec_pl[ 1 ]    = xvec_pl[ 0 ];
         yvec_pl[ 1 ]    = yvec_pl[ 0 ];
         xvec_pl[ 0 ]    = xvec_pl[ 1 ] * pad_lo;
         yvec_pl[ 0 ]    = 0.0;
         xvec_pl[ 3 ]    = xvec_pl[ 2 ] * pad_hi;
         yvec_pl[ 3 ]    = 0.0;
      }

      xvp             = xvec_pl.data();
      yvp             = yvec_pl.data();
   }

   // Do the plot
   dataPlotClear( data_plot );
   data_plot->setTitle        ( tr( "Distribution" ) );
   data_plot->setAxisTitle    ( QwtPlot::yLeft,   ytitle );
   data_plot->setAxisTitle    ( QwtPlot::xBottom, xtitle );
   data_plot->setAxisScale    ( QwtPlot::yLeft, 0.0, ymax );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   us_grid( data_plot );
   QwtPlotCurve* curve  = us_curve( data_plot, tr( "Curve " ) + xtitle );
   curve->setPen    ( QPen( US_GuiSettings::plotCurve(), 2 ) );
   curve->setStyle  ( QwtPlotCurve::Sticks );
   curve->setSamples( xvp, yvp, nxyp );

   // Fill in the model statistics summary for the attribute
   QVector< QVector< double > >  mstats;
   int niters      = imodels.size();
   int dx          = cb_params->currentIndex();
   int ncomp       = imodels[ 0 ].components.size();

   if ( dx < ( ncomp * 6 ) )
   {  // Remap from distribution index to model stats index
      int cx          = ( dx / 6 ) * 6;
      int ax          = dx - cx;
      dx              = ( ax == 0 ) ? ( cx + 3 ) : dx;
      dx              = ( ax == 1 ) ? ( cx + 4 ) : dx;
      dx              = ( ax == 2 ) ? ( cx + 2 ) : dx;
      dx              = ( ax == 3 ) ? ( cx + 5 ) : dx;
      dx              = ( ax == 4 ) ? ( cx + 1 ) : dx;
      dx              = ( ax == 5 ) ? ( cx     ) : dx;
   }

   US_DmgaMcStats::build_model_stats( niters, imodels, mstats );

   le_ms_mean->setText( QString::number( mstats[ dx ][  2 ] ) );
   le_ms_95lo->setText( QString::number( mstats[ dx ][  9 ] ) );
   le_ms_95hi->setText( QString::number( mstats[ dx ][ 10 ] ) );
   le_ms_medi->setText( QString::number( mstats[ dx ][  3 ] ) );
   le_ms_mode->setText( QString::number( mstats[ dx ][  8 ] ) );
   le_ms_iter->setText( QString::number( xvalcm ) );

   // Add single points and legend entries for Mean, Median, ...
   QwtLegend*    legend  = new QwtLegend;
   double xm[ 4 ];
   double ym[ 4 ];
   xm[ 0 ]         = mstats[ dx ][ 2 ];       
   xm[ 1 ]         = mstats[ dx ][ 3 ];       
   xm[ 2 ]         = mstats[ dx ][ 8 ];       
   xm[ 3 ]         = xvalcm;
   ym[ 0 ]         = xkmax * 1.015;
   ym[ 1 ]         = xkmax * 1.025;
   ym[ 2 ]         = xkmax * 1.035;
   ym[ 3 ]         = xkmax * 1.045;
   const QString cl_mean = tr( "Mean"     );
   const QString cl_medi = tr( "Median"   );
   const QString cl_mode = tr( "Mode"     );
         QString cl_iter = tr( "Iteration %1" ).arg( iter );
   lb_ms_iter->setText( cl_iter );
   const QColor  co_bord ( Qt::blue  );
   const QColor  co_mean ( Qt::cyan  );
   const QColor  co_medi ( "orange"  );
   const QColor  co_mode ( Qt::red   );
   const QColor  co_iter ( "lime"    );
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   legend->setFont( QFont( US_GuiSettings::fontFamily(),
                           US_GuiSettings::fontSize() - 2 ) );
   data_plot->insertLegend( legend, QwtPlot::BottomLegend );
   curve->setItemAttribute( QwtPlotItem::Legend, false );
   QwtPlotCurve* curv_mean  = us_curve( data_plot, cl_mean );
   QwtPlotCurve* curv_medi  = us_curve( data_plot, cl_medi );
   QwtPlotCurve* curv_mode  = us_curve( data_plot, cl_mode );
   QwtPlotCurve* curv_iter  = us_curve( data_plot, cl_iter );
   QwtSymbol*    sym_mean   = new QwtSymbol;
   QwtSymbol*    sym_medi   = new QwtSymbol;
   QwtSymbol*    sym_mode   = new QwtSymbol;
   QwtSymbol*    sym_iter   = new QwtSymbol;
   sym_mean->setStyle( QwtSymbol::Ellipse   );
   sym_mean->setPen  ( QPen  (  co_bord   ) );
   sym_mean->setBrush( QBrush(  co_mean   ) );
   sym_mean->setSize ( 6 );
   curv_mean->setStyle( QwtPlotCurve::NoCurve );
   curv_mean->setSymbol( sym_mean );
   curv_mean->setItemAttribute( QwtPlotItem::Legend, true );
   sym_medi->setStyle( QwtSymbol::Rect      );
   sym_medi->setPen  ( QPen  (  co_bord   ) );
   sym_medi->setBrush( QBrush(  co_medi   ) );
   sym_medi->setSize ( 6 );
   curv_medi->setStyle( QwtPlotCurve::NoCurve );
   curv_medi->setSymbol( sym_medi );
   curv_medi->setItemAttribute( QwtPlotItem::Legend, true );
   sym_mode->setStyle( QwtSymbol::DTriangle );
   sym_mode->setPen  ( QPen  (  co_bord   ) );
   sym_mode->setBrush( QBrush(  co_mode   ) );
   sym_mode->setSize ( 8 );
   curv_mode->setStyle( QwtPlotCurve::NoCurve );
   curv_mode->setSymbol( sym_mode );
   curv_mode->setItemAttribute( QwtPlotItem::Legend, true );
   sym_iter->setStyle( QwtSymbol::Cross     );
   sym_iter->setPen  ( QPen  (  co_iter   ) );
   sym_iter->setBrush( QBrush(  co_iter   ) );
   sym_iter->setSize ( 8 );
   curv_iter->setStyle( QwtPlotCurve::NoCurve );
   curv_iter->setSymbol( sym_iter );
   curv_iter->setItemAttribute( QwtPlotItem::Legend, true );
   curv_mean->setSamples( xm    , ym    , 1 );
   curv_medi->setSamples( xm + 1, ym + 1, 1 );
   curv_mode->setSamples( xm + 2, ym + 2, 1 );
   curv_iter->setSamples( xm + 3, ym + 3, 1 );

   // Show the plot
   data_plot->replot();
}

// Bump selected distribution parameter to the next one
void US_AdvDmgaMc::next_param()
{
   // Bump distribute choice to the next one
   int disx        = cb_params->currentIndex();
   disx++;
   disx            = ( disx < ls_params.size() ) ? disx : 0;
   cb_params->setCurrentIndex( disx );

   plot_distrib();
}

// Bump selected distribution parameter to the previous one
void US_AdvDmgaMc::prev_param()
{
   // Bump distribute choice to the next one
   int disx        = cb_params->currentIndex();
   disx--;
   disx            = ( disx < 0 ) ? ( ls_params.size() - 1 ) : disx;
   cb_params->setCurrentIndex( disx );

   plot_distrib();
}

// Signal main-window Simulate, after insuring parameter settings are saved
void US_AdvDmgaMc::simulate()
{
   parmap[ "modelnbr"  ] = QString::number( ct_modelnbr ->value() );
   parmap[ "modelsim"  ] = rb_mean  ->isChecked() ? "mean"   :
                         ( rb_median->isChecked() ? "median" :
                         ( rb_mode  ->isChecked() ? "mode"   :
                         ( rb_curmod->isChecked() ? "model"  : "" ) ) );

   if ( ! rb_curmod->isChecked() )
      umodel.description    = lddesc;
   *model                = umodel;
   US_FeMatch* fem_wind  = (US_FeMatch*)parentw;

   fem_wind->simulate();
}

