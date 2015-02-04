//! \file us_adv_dmgamc.cpp

#include "us_adv_dmgamc.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"

// constructor:  advanced analysis control widget
US_AdvDmgaMc::US_AdvDmgaMc( US_Model* amodel,
   QVector< US_Model >& aimodels,
   QMap< QString, QString >& adv_vals, QWidget* p ) :
   US_WidgetsDialog( p, 0 ), imodels( aimodels ), parmap( adv_vals )
{
   model          = amodel;
   parentw        = p;
   umodel         = *model;

   setObjectName( "US_AdvDmgaMC" );
   setPalette( US_GuiSettings::frameColor() );
   setFont( QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() ) );

   // lay out the GUI
   setWindowTitle( tr( "DMGA-MC Advanced Controls" ) );

   mainLayout      = new QVBoxLayout( this );
   utypeLayout     = new QGridLayout();
   upperLayout     = new QHBoxLayout();
   lowerLayout     = new QVBoxLayout();
   analysisLayout  = new QGridLayout( );
   modelcomLayout  = new QGridLayout( );
   distrLayout     = new QGridLayout();
   plotLayout      = new US_Plot( data_plot,
                                  tr( "Attribute Distribution" ),
                                  tr( "Attribute Name" ),
                                  tr( "Frequency" ) );
   mstatLayout     = new QGridLayout();
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
   QPushButton* pb_help    = us_pushbutton( tr( "Help" ) );
   QPushButton* pb_cancel  = us_pushbutton( tr( "Cancel" ) );
   QPushButton* pb_accept  = us_pushbutton( tr( "Accept" ) );
   int row      = 0;
   utypeLayout->addWidget( lb_modelsim, row++, 0, 1, 12 );
   utypeLayout->addLayout( lo_mean,     row,   0, 1, 3 );
   utypeLayout->addLayout( lo_median,   row,   3, 1, 3 );
   utypeLayout->addLayout( lo_mode,     row,   6, 1, 3 );
   utypeLayout->addLayout( lo_curmod,   row++, 9, 1, 3 );
   utypeLayout->addWidget( pb_help,     row,   0, 1, 4 );
   utypeLayout->addWidget( pb_cancel,   row,   4, 1, 4 );
   utypeLayout->addWidget( pb_accept,   row++, 8, 1, 4 );

   upperLayout->addLayout( analysisLayout  );
   upperLayout->addLayout( modelcomLayout );
   upperLayout->setStretchFactor( analysisLayout, 1 );
   upperLayout->setStretchFactor( modelcomLayout, 1 );

   QLabel* lb_modselect = us_banner( tr( "MC Used Model Selection" ) );
   le_modtype           = us_lineedit( "", -1, true );
   QPalette mtpal       = le_modtype->palette();
   mtpal.setColor( QPalette::Text, Qt::blue );
   le_modtype->setPalette( mtpal );
   QLabel* lb_modreact  = us_banner( tr( "Model Reactions" ) );
   QLabel* lb_kdissoc   = us_label(  tr( "k_Dissociation:" ) );
   QLabel* lb_koffrate  = us_label(  tr( "k_off Rate:"     ) );
   le_kdissoc           = us_lineedit( "", -1, true );
   le_koffrate          = us_lineedit( "", -1, true );
   pb_reaction          = us_pushbutton( tr( "Next Reaction" ) );
   ct_reaction          = us_counter( 2, 1, 10,      1 );

   QLabel* lb_modelcom  = us_banner( tr( "Model Components"           ) );
   QLabel* lb_sedcoeff  = us_label(  tr( "Sedimentation Coefficient:" ) );
   QLabel* lb_difcoeff  = us_label(  tr( "Diffusion Coefficient:"     ) );
   QLabel* lb_moweight  = us_label(  tr( "Molecular Weight:"          ) );
   QLabel* lb_friratio  = us_label(  tr( "Frictional Ratio (f/f0):"   ) );
   QLabel* lb_vbar20    = us_label(  tr( "Vbar (20_W):"               ) );
   QLabel* lb_partconc  = us_label(  tr( "Partial Concentration:"     ) );

   QLabel* lb_distplot  = us_banner( tr( "Attribute Distribution Plot" ) );
   QPushButton*
      pb_component      = us_pushbutton( tr( "Next Component" ) );
   pb_nextmodel         = us_pushbutton( tr( "Next Model"   ) );
   QPushButton*
      pb_nextdist       = us_pushbutton( tr( "Next Distribution" ) );
   QPushButton*
      pb_distrib        = us_pushbutton( tr( "Plot Distribution:" ) );
   cb_distrib           = us_comboBox();
   ls_distrib .clear();
   ls_distrib << tr( "Component 1 Sedimentation Coefficient" );
   ls_distrib << tr( "Component 1 Diffusion Coefficient" );
   ls_distrib << tr( "Component 1 Molecular Weight" );
   ls_distrib << tr( "Component 1 Frictional Ratio" );
   ls_distrib << tr( "Component 1 Vbar (20_W)" );
   ls_distrib << tr( "Component 1 Partial Concentration" );
   ls_distrib << tr( "Reaction 1 k_Dissociation:" );
   ls_distrib << tr( "Reaction 1 k_off Rate:" );
   cb_distrib->addItems( ls_distrib );

   ct_modelnbr  = us_counter( 2, 1,  50,     1 );
   ct_component = us_counter( 2, 1, 200,     1 );

   le_sedcoeff  = us_lineedit( "", -1, true );
   le_difcoeff  = us_lineedit( "", -1, true );
   le_moweight  = us_lineedit( "", -1, true );
   le_friratio  = us_lineedit( "", -1, true );
   le_vbar20    = us_lineedit( "", -1, true );
   le_partconc  = us_lineedit( "", -1, true );

   QLabel* lb_ms_mean   = us_label(  tr( "Mean" ) );
   QLabel* lb_ms_99lo   = us_label(  tr( "99% Conf.Low" ) );
   QLabel* lb_ms_99hi   = us_label(  tr( "99% Conf.High" ) );
   QLabel* lb_ms_medi   = us_label(  tr( "Median" ) );
   QLabel* lb_ms_mode   = us_label(  tr( "Mode" ) );
   QLabel* lb_ms_iter   = us_label(  tr( "Iter. 25" ) );
   le_ms_mean   = us_lineedit( "", -1, true );
   le_ms_99lo   = us_lineedit( "", -1, true );
   le_ms_99hi   = us_lineedit( "", -1, true );
   le_ms_medi   = us_lineedit( "", -1, true );
   le_ms_mode   = us_lineedit( "", -1, true );
   le_ms_iter   = us_lineedit( "", -1, true );

   row          = 0;
   analysisLayout->addWidget( lb_modselect,  row++, 0, 1, 6 );
   analysisLayout->addWidget( le_modtype,    row++, 0, 1, 6 );
   analysisLayout->addWidget( pb_nextmodel,  row,   0, 1, 3 );
   analysisLayout->addWidget( ct_modelnbr,   row++, 3, 1, 3 );
   analysisLayout->addWidget( lb_modreact,   row++, 0, 1, 6 );
   analysisLayout->addWidget( lb_kdissoc,    row,   0, 1, 3 );
   analysisLayout->addWidget( le_kdissoc,    row++, 3, 1, 3 );
   analysisLayout->addWidget( lb_koffrate,   row,   0, 1, 3 );
   analysisLayout->addWidget( le_koffrate,   row++, 3, 1, 3 );
   analysisLayout->addWidget( pb_reaction,   row,   0, 1, 3 );
   analysisLayout->addWidget( ct_reaction,   row++, 3, 1, 3 );

   row          = 0;
   modelcomLayout->addWidget( lb_modelcom,   row++, 0, 1, 6 );
   modelcomLayout->addWidget( lb_sedcoeff,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_sedcoeff,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_difcoeff,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_difcoeff,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_moweight,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_moweight,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_friratio,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_friratio,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_vbar20,     row,   0, 1, 3 );
   modelcomLayout->addWidget( le_vbar20,     row++, 3, 1, 3 );
   modelcomLayout->addWidget( lb_partconc,   row,   0, 1, 3 );
   modelcomLayout->addWidget( le_partconc,   row++, 3, 1, 3 );
   modelcomLayout->addWidget( pb_component,  row,   0, 1, 3 );
   modelcomLayout->addWidget( ct_component,  row++, 3, 1, 3 );

   row          = 0;
   distrLayout   ->addWidget( lb_distplot,   row++, 0, 1, 4 );
   distrLayout   ->addWidget( pb_nextdist,   row,   0, 1, 1 );
   distrLayout   ->addWidget( pb_distrib,    row,   1, 1, 1 );
   distrLayout   ->addWidget( cb_distrib,    row++, 2, 1, 2 );

   row          = 0;
   mstatLayout   ->addWidget( lb_ms_mean,    row,   0, 1, 1 );
   mstatLayout   ->addWidget( lb_ms_99lo,    row,   1, 1, 1 );
   mstatLayout   ->addWidget( lb_ms_99hi,    row,   2, 1, 1 );
   mstatLayout   ->addWidget( lb_ms_medi,    row,   3, 1, 1 );
   mstatLayout   ->addWidget( lb_ms_mode,    row,   4, 1, 1 );
   mstatLayout   ->addWidget( lb_ms_iter,    row++, 5, 1, 1 );
   mstatLayout   ->addWidget( le_ms_mean,    row,   0, 1, 1 );
   mstatLayout   ->addWidget( le_ms_99lo,    row,   1, 1, 1 );
   mstatLayout   ->addWidget( le_ms_99hi,    row,   2, 1, 1 );
   mstatLayout   ->addWidget( le_ms_medi,    row,   3, 1, 1 );
   mstatLayout   ->addWidget( le_ms_mode,    row,   4, 1, 1 );
   mstatLayout   ->addWidget( le_ms_iter,    row,   5, 1, 1 );

   lowerLayout   ->addLayout( distrLayout );
   lowerLayout   ->addLayout( plotLayout  );
   mainLayout    ->addLayout( utypeLayout );
   mainLayout    ->addLayout( upperLayout );
   mainLayout    ->addLayout( lowerLayout );
   mainLayout    ->addLayout( mstatLayout );

   ct_modelnbr ->setValue( 0     );
   ct_component->setValue( 0     );
   ct_modelnbr ->setStep(     1 );
   ct_component->setStep(     1 );

   pb_nextmodel->setEnabled( false );
   ct_modelnbr ->setEnabled( false );

   connect( pb_component, SIGNAL( clicked       ()         ),
            this,         SLOT  ( next_component()         ) );
   connect( ct_component, SIGNAL( valueChanged  ( double ) ),
            this,         SLOT  ( set_component ( double ) ) );
   connect( pb_reaction,  SIGNAL( clicked       ()         ),
            this,         SLOT  ( next_reaction ()         ) );
   connect( ct_reaction,  SIGNAL( valueChanged  ( double ) ),
            this,         SLOT  ( set_reaction  ( double ) ) );
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
   connect( pb_distrib,   SIGNAL( clicked       ()         ),
            this,         SLOT  ( plot_distrib  ()         ) );
   connect( pb_nextdist,  SIGNAL( clicked       ()         ),
            this,         SLOT  ( next_distrib  ()         ) );

   set_component( 1.0 );

   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT  ( help()    ) );
   connect( pb_cancel,  SIGNAL( clicked() ),
            this,       SLOT  ( reject()  ) );
   connect( pb_accept,  SIGNAL( clicked() ),
            this,       SLOT  ( done()    ) );

   adjustSize();
   QFontMetrics fmet( QFont( US_GuiSettings::fontFamily(),
                             US_GuiSettings::fontSize() - 1 ) );
   int fwid        = fmet.maxWidth();
   int rhgt        = lb_modselect->height();
   int csizw       = fwid * 2;
   ct_modelnbr ->resize( csizw, rhgt );
   ct_modelnbr ->setMaximumWidth( csizw * 3 );

   US_DmgaMcStats::build_used_model( "mean", 0, imodels, umodel );
   int ncomp       = umodel.components.size();
   int nreac       = umodel.associations.size();

   le_modtype ->setText( tr( "Mean model, %1 components, %2 reaction(s)" )
                         .arg( ncomp ).arg( nreac ) );
   ct_component->setMaxValue( ncomp );
   ct_reaction ->setMaxValue( nreac );
   le_sedcoeff->setText( QString::number( umodel.components[ 0 ].s ) );
   le_difcoeff->setText( QString::number( umodel.components[ 0 ].D ) );
   le_moweight->setText( QString::number( umodel.components[ 0 ].mw ) );
   le_friratio->setText( QString::number( umodel.components[ 0 ].f_f0 ) );
   le_vbar20  ->setText( QString::number( umodel.components[ 0 ].vbar20 ) );
   le_partconc->setText(
      QString::number( umodel.components[ 0 ].signal_concentration ) );

   if ( nreac > 0 )
   {
      le_kdissoc ->setText( QString::number( umodel.associations[ 0 ].k_d   ) );
      le_koffrate->setText( QString::number( umodel.associations[ 0 ].k_off ) );
   }

   else
   {
      pb_reaction->setEnabled( false );
      ct_reaction->setEnabled( false );
      le_kdissoc ->setText( "0" );
      le_koffrate->setText( "0" );
   }
  
   ls_distrib.clear();
  
   for ( int ii = 0; ii < ncomp; ii++ )
   {
      QString comp = tr( "Component %1 " ).arg( ii + 1 );
      ls_distrib << comp + tr( "Sedimentation Coefficient" );
      ls_distrib << comp + tr( "Diffusion Coefficient" );
      ls_distrib << comp + tr( "Molecular Weight" );
      ls_distrib << comp + tr( "Frictional Ratio" );
      ls_distrib << comp + tr( "Vbar (20_W)" );
      ls_distrib << comp + tr( "Partial Concentration" );
   }

   for ( int ii = 0; ii < nreac; ii++ )
   {
      QString reac = tr( "Reaction %1 " ).arg( ii + 1 );
      ls_distrib << reac + tr( "k_Dissociation:" );
      ls_distrib << reac + tr( "k_off Rate:" );
   }

   cb_distrib->clear();
   cb_distrib->addItems( ls_distrib );
   cb_distrib->setCurrentIndex( 0 );
   lb_ms_iter->setText( tr( "Iter. %1" ).arg( imodels.size() / 2 ) );

qDebug() << "AdvD:Pre-adjust size" << size();
   adjustSize();
qDebug() << "AdvD:Post-adjust size" << size();
   resize( 720, 720 );
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

// Private slot to advance to the next model component
void US_AdvDmgaMc::next_component( void )
{
   int icomp = (int)ct_component->value();
   int ncomp = umodel.components.size();

   icomp     = ( icomp < ncomp ) ? ( icomp + 1 ) : 1;
   ct_component->setValue( (double)icomp );
}

// Private slot to set the model component index and fill in the implied text
void US_AdvDmgaMc::set_component( double compx )
{
   int icomp  = (int)compx - 1;

   if ( icomp < 0 )
      return;

   le_sedcoeff->setText( QString::number( umodel.components[ icomp ].s ) );
   le_difcoeff->setText( QString::number( umodel.components[ icomp ].D ) );
   le_moweight->setText( QString::number( umodel.components[ icomp ].mw ) );
   le_friratio->setText( QString::number( umodel.components[ icomp ].f_f0 ) );
   le_vbar20  ->setText( QString::number( umodel.components[ icomp ].vbar20 ) );
   le_partconc->setText(
      QString::number( umodel.components[ icomp ].signal_concentration ) );
}

// Private slot to advance to the next model component
void US_AdvDmgaMc::next_reaction( void )
{
   int ireac = (int)ct_reaction->value();
   int nreac = umodel.associations.size();

   ireac     = ( ireac < nreac ) ? ( ireac + 1 ) : 1;
   ct_reaction->setValue( (double)ireac );
}

// Private slot to set the model component index and fill in the implied text
void US_AdvDmgaMc::set_reaction( double reacx )
{
   int ireac  = (int)reacx - 1;

   if ( ireac < 0 )
      return;

   le_sedcoeff->setText( QString::number( umodel.associations[ ireac ].k_d ) );
   le_difcoeff->setText( QString::number( umodel.associations[ ireac ].k_off) );
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

   bool cur_mod    = rb_curmod->isChecked();
   QString smtype  = tr( "model" );
   int iter        = cur_mod ? ct_modelnbr->value() : 0;
   smtype          = rb_mean  ->isChecked() ? tr( "mean"   ) : smtype;
   smtype          = rb_median->isChecked() ? tr( "median" ) : smtype;
   smtype          = rb_mode  ->isChecked() ? tr( "mode"   ) : smtype;
   smtype          = cur_mod                ? tr( "model"  ) : smtype;

   US_DmgaMcStats::build_used_model( smtype, iter, imodels, umodel );

   int ncomp       = umodel.components.size();
   int nreac       = umodel.associations.size();
   int cx          = (int)ct_component->value() - 1;
   int rx          = (int)ct_reaction ->value() - 1;

   QString mtlabl  = tr( "Mean" );
   mtlabl          = rb_mean  ->isChecked() ? tr( "Mean"   ) : mtlabl;
   mtlabl          = rb_median->isChecked() ? tr( "Median" ) : mtlabl;
   mtlabl          = rb_mode  ->isChecked() ? tr( "Mode"   ) : mtlabl;
   mtlabl          = cur_mod ? tr( "Iteration %1" ).arg( iter ) : mtlabl;
   le_modtype ->setText( mtlabl + tr( " model, %1 components, %2 reaction(s)" )
                         .arg( ncomp ).arg( nreac ) );
   le_sedcoeff->setText( QString::number( umodel.components[ cx ].s ) );
   le_difcoeff->setText( QString::number( umodel.components[ cx ].D ) );
   le_moweight->setText( QString::number( umodel.components[ cx ].mw ) );
   le_friratio->setText( QString::number( umodel.components[ cx ].f_f0 ) );
   le_vbar20  ->setText( QString::number( umodel.components[ cx ].vbar20 ) );
   le_partconc->setText(
      QString::number( umodel.components[ 0 ].signal_concentration ) );

   if ( nreac > 0 )
   {
      le_kdissoc ->setText( QString::number( umodel.associations[rx].k_d   ) );
      le_koffrate->setText( QString::number( umodel.associations[rx].k_off ) );
   }

   else
   {
      pb_reaction->setEnabled( false );
      ct_reaction->setEnabled( false );
      le_kdissoc ->setText( "0" );
      le_koffrate->setText( "0" );
   }

   pb_nextmodel->setEnabled( cur_mod );
   ct_modelnbr ->setEnabled( cur_mod );

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

   int nxi         = imodels.size();
   QString attrib  = cb_distrib->currentText();
   int jc          = attrib.section( " ", 1, 1 ).toInt() - 1;
   int jr          = jc;
   jc              = attrib.contains( tr( "Component" ) ) ? jc : -1;
   jr              = attrib.contains( tr( "Reaction" ) )  ? jr : -1;
   QString xtitle  = attrib.section( " ", 2, -1 ).simplified();
   xvec_in.clear();
   double xvalcm   = 0.0;
   int hx          = nxi / 2;

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
   qSort( xvec_in );
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
   xkmax          *= pad_hi;           // Add a slight pad to Y maximum

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
   data_plot->detachItems();
   data_plot->setTitle        ( tr( "Distribution" ) );
   data_plot->setAxisTitle    ( QwtPlot::yLeft,   ytitle );
   data_plot->setAxisTitle    ( QwtPlot::xBottom, xtitle );
   data_plot->setAxisScale    ( QwtPlot::yLeft, 0.0, xkmax );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );
   us_grid( data_plot );
   QwtPlotCurve* curve  = us_curve( data_plot, tr( "Curve " ) + xtitle );
   curve->setPen  ( QPen( US_GuiSettings::plotCurve(), 2 ) );
   curve->setStyle( QwtPlotCurve::Sticks );
   curve->setData ( xvp, yvp, nxyp );
   data_plot->replot();

   // Fill in the model statistics summary for the attribute
   QVector< QVector< double > >  mstats;
   int niters      = imodels.size();
   int dx          = cb_distrib->currentIndex();
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
   le_ms_99lo->setText( QString::number( mstats[ dx ][ 11 ] ) );
   le_ms_99hi->setText( QString::number( mstats[ dx ][ 12 ] ) );
   le_ms_medi->setText( QString::number( mstats[ dx ][  3 ] ) );
   le_ms_mode->setText( QString::number( mstats[ dx ][  8 ] ) );
   le_ms_iter->setText( QString::number( xvalcm ) );
}

// Bump selected distribution to the next one
void US_AdvDmgaMc::next_distrib()
{
   // Bump distribute choice to the next one
   int disx        = cb_distrib->currentIndex();
   disx++;
   disx            = ( disx < ls_distrib.size() ) ? disx : 0;
   cb_distrib->setCurrentIndex( disx );

   plot_distrib();
}

