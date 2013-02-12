//! \file us_mlplot.cpp

#include "us_mlplot.h"
#include "us_1dsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_math2.h"

#include <qwt_legend.h>

// constructor:  model lines plot widget
US_MLinesPlot::US_MLinesPlot( double& flo, double& fhi, double& fin,
      double& slo, double& shi, int& nlp, int& bmx, int& nkp, int &typ )
   : US_WidgetsDialog( 0, 0 ), fmin( flo ), fmax( fhi ), finc( fin ),
   smin( slo ), smax( shi ), nlpts( nlp ), bmndx( bmx ), nkpts( nkp ),
   ctype( typ )
{
   // lay out the GUI
   setWindowTitle( tr( "1-D Spectrum Analysis Model Lines Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 480 );

   dbg_level       = US_Settings::us_debug();
   model           = 0;
   le_fact         = 1.5;
   me_fact         = 1.3;
   he_fact         = 1.1;
   mp_fact         = 0.30;

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   pltctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QVBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "1DSA Model Parameters" ) );
   QLabel* lb_mtype       = us_label(  tr( "Model Type:" ) );
   QLabel* lb_nlines      = us_label(  tr( "Lines (Models):" ) );
   QLabel* lb_npoints     = us_label(  tr( "Points per Line:" ) );
   QLabel* lb_kincr       = us_label(  tr( "f/f0 Increment:" ) );
   QLabel* lb_legend      = us_banner( tr( "Model Line Color Legend" ) );
           lb_lefact      = us_label(  tr( "Low-Elite Factor" ) );
           lb_mefact      = us_label(  tr( "Mid-Elite Factor" ) );
           lb_hefact      = us_label(  tr( "High-Elite Factor" ) );
           lb_mpfact      = us_label(  tr( "Mid-Poor Factor" ) );
   QTextEdit* te_legend   = us_textedit();
   te_legend->setTextColor( Qt::blue );
   te_legend->setText( 
         tr( " Red -> Best;\n"
             " Reddish -> High Elite;\n"
             " Bluish -> Mid Elite;\n"
             " Greenish -> Low Elite;\n"
             " Yellow -> Non Elite / Undetermined;\n"
             " Gray -> Poorest (Highest) RMSD;\n"
             " Cyan -> Best Computed Solutes." ) );
   us_setReadOnly( te_legend, true );
   QFont font( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
   QFontMetrics fm( font );
   te_legend->setMaximumHeight( fm.lineSpacing() * 8 );

   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   int    nline = nkpts * nkpts;
   le_mtype     = us_lineedit( tr( "Straight Line" ), -1, true );
   le_nlines    = us_lineedit( QString::number( nline ), -1, true );
   le_npoints   = us_lineedit( QString::number( nlpts ), -1, true );
   le_kincr     = us_lineedit( QString::number( finc  ), -1, true );
   ct_lefact    = us_counter( 3, 1.01, 10.0, le_fact );
   ct_mefact    = us_counter( 3, 1.01, 10.0, me_fact );
   ct_hefact    = us_counter( 3, 1.01, 10.0, he_fact );
   ct_mpfact    = us_counter( 3, 0.00, 1.00, mp_fact );
   ct_lefact->setStep( 0.001 );
   ct_mefact->setStep( 0.001 );
   ct_hefact->setStep( 0.001 );
   ct_mpfact->setStep( 0.001 );

   if ( ctype == 1  ||  ctype == 2 )
   {
      lb_kincr->setText( tr( "Variation Count" ) );
      le_kincr->setText( QString::number( nkpts ) );
   }

   int row      = 0;
   pltctrlsLayout->addWidget( lb_datctrls, row++, 0, 1, 2 );
   pltctrlsLayout->addWidget( lb_mtype,    row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_mtype,    row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_nlines,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_nlines,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_npoints,  row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_npoints,  row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_kincr,    row,   0, 1, 1 );
   pltctrlsLayout->addWidget( le_kincr,    row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_lefact,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( ct_lefact,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_mefact,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( ct_mefact,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_hefact,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( ct_hefact,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_mpfact,   row,   0, 1, 1 );
   pltctrlsLayout->addWidget( ct_mpfact,   row++, 1, 1, 1 );
   pltctrlsLayout->addWidget( lb_legend,   row++, 0, 1, 2 );
   pltctrlsLayout->addWidget( te_legend,   row++, 0, 1, 2 );
//   row         += 7;
   if      ( ctype == 0 ) le_mtype->setText( tr( "Straight Line" ) );
   else if ( ctype == 1 ) le_mtype->setText( tr( "Increasing Sigmoid" ) );
   else if ( ctype == 2 ) le_mtype->setText( tr( "Decreasing Sigmoid" ) );

   bool fact_vis = ( model != 0 );
   lb_lefact->setVisible( fact_vis );
   ct_lefact->setVisible( fact_vis );
   lb_mefact->setVisible( fact_vis );
   ct_mefact->setVisible( fact_vis );
   lb_hefact->setVisible( fact_vis );
   ct_hefact->setVisible( fact_vis );
   lb_mpfact->setVisible( fact_vis );
   ct_mpfact->setVisible( fact_vis );

   buttonsLayout ->addWidget( pb_close );

   plotLayout1 = new US_Plot( data_plot1,
         tr( "Model Lines" ),
         tr( "Sedimentation Coefficient (x 1e13)" ),
         tr( "Frictional Ratio (f/f0)" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );

   rightLayout->addLayout( plotLayout1    );

   leftLayout ->addLayout( pltctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );
   mainLayout->setStretchFactor( leftLayout,  2 );
   mainLayout->setStretchFactor( rightLayout, 5 );

   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT  ( close_all() ) );
   connect( ct_lefact, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( updateLeFact( double ) ) );
   connect( ct_mefact, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( updateMeFact( double ) ) );
   connect( ct_hefact, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( updateHeFact( double ) ) );
   connect( ct_mpfact, SIGNAL( valueChanged( double ) ),
            this,      SLOT  ( updateMpFact( double ) ) );

DbgLv(1) << "RP:  p1size" << p1size;
   data_plot1->resize( p1size );
   setAttribute( Qt::WA_DeleteOnClose, true );
}

// close button clicked
void US_MLinesPlot::close_all()
{
   close();
}

// plot the data
void US_MLinesPlot::plot_data()
{
   data_plot1->detachItems();
   data_plot1->clear();

   bool   got_best  = ( model != 0  &&  bmndx >= 0 );
DbgLv(1) << "RP:PD got_best" << got_best << "bmndx" << bmndx;

   us_grid( data_plot1 );
DbgLv(1) << "RP:PD us_grid RTN";

   QString       title;
   QwtPlotCurve* curv;
DbgLv(1) << "RP:PD smin smax" << smin << smax;
   data_plot1->setAxisScale( QwtPlot::xBottom, smin, smax, 1.0 );
DbgLv(1) << "RP:PD AxisScale RTN";

   QVector< double > xvec( nlpts, 0.0 );
   QVector< double > yvec( nlpts, 0.0 );

   double* xx    = xvec.data();
   double* yy    = yvec.data();
   int     nmodl = mrecs.size();
   QPen    pen_plot( US_GuiSettings::plotCurve(), 1 );

DbgLv(1) << "RP:PD   got_best" << got_best;
DbgLv(1) << "RP:PD (2)smin smax" << smin << smax;

   if ( got_best )
   { // Plot lines after best and sorted model records have been produced
DbgLv(1) << "RP:PD mrecs size" << mrecs.size() << nmodl;
      le_fact       = ct_lefact->value();
      me_fact       = ct_mefact->value();
      he_fact       = ct_hefact->value();
      mp_fact       = ct_mpfact->value();
      best_rmsd     = mrecs[ 0         ].rmsd;
      worst_rmsd    = mrecs[ nmodl - 1 ].rmsd;
DbgLv(1) << "RP:PD  best worst" << best_rmsd << worst_rmsd;
      low_elite     = best_rmsd * le_fact;
      mid_elite     = best_rmsd * me_fact;
      high_elite    = best_rmsd * he_fact;
      mid_poor      = low_elite + ( worst_rmsd - low_elite ) * mp_fact;
DbgLv(1) << "RP:PD  l,m,h elite" << low_elite << mid_elite << high_elite;
DbgLv(1) << "RP:PD   mid_poor" << mid_poor;
DbgLv(1) << "RP:PD (3)smin smax" << smin << smax;

      QColor colr_he( Qt::red );
      QColor colr_me( Qt::blue );
      QColor colr_le( Qt::green );
      QColor colr_hp( US_GuiSettings::plotCurve() );
      QColor colr_lp( 160, 160, 160 );
      QPen   pen_red(  Qt::red, 3.0 );
      QPen   pen_heli( colr_he, 2 );
      QPen   peh_meli( colr_me, 2 );
      QPen   pen_leli( colr_le, 2 );
      QPen   pen_hipo( colr_hp, 1 );
      QPen   pen_lopo( colr_lp, 1 );
      QPen   pen_gray( QColor( 160, 160, 160 ),     1 );
      int    ired_he = colr_he.red();
      int    igrn_he = colr_he.green();
      int    iblu_he = colr_he.blue();
      int    ired_me = colr_me.red();
      int    igrn_me = colr_me.green();
      int    iblu_me = colr_me.blue();
      int    ired_le = colr_le.red();
      int    igrn_le = colr_le.green();
      int    iblu_le = colr_le.blue();
      int    ired_hp = colr_hp.red();
      int    igrn_hp = colr_hp.green();
      int    iblu_hp = colr_hp.blue();
      int    ired, igrn, iblu;

      // Determine maximum concentration in 10 best models
      double max_conc = 0.0;
      for ( int ii = 0; ii < qMin( nmodl, 10 ); ii++ )
      {
         for ( int kk = 0; kk < mrecs[ ii ].csolutes.size(); kk++ )
         {
            max_conc     = qMax( max_conc, mrecs[ ii ].csolutes[ kk ].c );
         }
      }
DbgLv(1) << "RP:PD (4)smin smax" << smin << smax;

      for ( int ii = ( nmodl - 1 ); ii >= 0; ii-- )
      { // Loop over model records from worst (highest) rmsd to best
         double rmsd_rec  = mrecs[ ii ].rmsd;
         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );
//DbgLv(1) << "RP:PD    ii" << ii << "rmsd_rec" << rmsd_rec;

         if ( rmsd_rec >= mid_poor )
         { // Poorest (highest) RMSD:  gray line
            curv->setPen( pen_gray );
         }

         else if ( rmsd_rec > low_elite )
         { // Non-elite, low-poor RMSD:  default (yellow?) line
            curv->setPen( pen_plot );
         }

         else if ( rmsd_rec > mid_elite )
         { // Between low- and mid-elite:  greenish line
            double crange = low_elite - mid_elite;
            double cfrac  = low_elite - rmsd_rec;
            ired  = ired_hp + ( ired_le - ired_hp ) * cfrac / crange;
            igrn  = igrn_hp + ( igrn_le - igrn_hp ) * cfrac / crange;
            iblu  = iblu_hp + ( iblu_le - iblu_hp ) * cfrac / crange;
            curv->setPen( QPen( QColor( ired, igrn, iblu ), 2 ) );
         }

         else if ( rmsd_rec > high_elite )
         { // Between mid- and high-elite:  bluish line
            double crange = mid_elite - high_elite;
            double cfrac  = mid_elite - rmsd_rec;
            ired  = ired_le + ( ired_me - ired_le ) * cfrac / crange;
            igrn  = igrn_le + ( igrn_me - igrn_le ) * cfrac / crange;
            iblu  = iblu_le + ( iblu_me - iblu_le ) * cfrac / crange;
            curv->setPen( QPen( QColor( ired, igrn, iblu ), 2 ) );
         }

         else if ( ii != 0 )
         { // Between high_elite and best:  reddish line
            double crange = high_elite - best_rmsd;
            double cfrac  = high_elite - rmsd_rec;
            ired  = ired_me + ( ired_he - ired_me ) * cfrac / crange;
            igrn  = igrn_me + ( igrn_he - igrn_me ) * cfrac / crange;
            iblu  = iblu_me + ( iblu_he - iblu_me ) * cfrac / crange;
            curv->setPen( QPen( QColor( ired, igrn, iblu ), 2 ) );
         }

         else
         { // The best RMSD:  red
            curv->setPen  ( pen_heli );    // Red
         }

         int klpts   = nlpts;

         if ( ctype == 0 )
         { // For straight line, just draw from start to end
            klpts       = 2;
            xx[ 0 ]     = smin;
            xx[ 1 ]     = smax;
            yy[ 0 ]     = mrecs[ ii ].str_k;
            yy[ 1 ]     = mrecs[ ii ].end_k;
         }

         else
         { // Otherwise, set each point on the curve
//DbgLv(1) << "RP:PD nlpts" << nlpts;
//DbgLv(1) << "RP:PD   isol size" << mrecs[ii].isolutes.size();
            for ( int kk = 0; kk < nlpts; kk++ )
            {
               xx[ kk ]     = mrecs[ ii ].isolutes[ kk ].s * 1.e+13;
               yy[ kk ]     = mrecs[ ii ].isolutes[ kk ].k;
            }
         }

//DbgLv(1) << "RP:PD   klpts" << klpts;
         curv->setData ( xx, yy, klpts );

         if ( rmsd_rec < mid_elite )
         { // For mid and high elite, plot the solute points
            int ncomp   = mrecs[ ii ].csolutes.size();
            for ( int kk = 0; kk < ncomp; kk++ )
            {
               double xv  = mrecs[ ii ].csolutes[ kk ].s * 1.0e+13;
               double yv  = mrecs[ ii ].csolutes[ kk ].k;
               double cv  = mrecs[ ii ].csolutes[ kk ].c;
               int    szd = qMax( 2, qRound( 9.0 * cv / max_conc ) );
               title      = tr( "ElitePoint " ) + QString::number( ii )
                            + " " + QString::number( kk );
               curv       = us_curve( data_plot1, title );
               curv->setPen  ( QPen( Qt::cyan, szd ) );
               curv->setStyle( QwtPlotCurve::Dots );
               curv->setData ( &xv, &yv, 1 );
            }
//DbgLv(1) << "RP:PD       ncomp" << ncomp << "x0 y0 xn yn"
// << xx[0] << yy[0] << xx[ncomp-1] << yy[ncomp-1];
         }
      } // END: models loop
DbgLv(1) << "RP:PD (5)smin smax" << smin << smax;
   }
   else
   { // Plot lines before any best-fit computations
      for ( int ii = 0; ii < nmodl; ii++ )
      {
         for ( int kk = 0; kk < nlpts; kk++ )
         {
            xx[ kk ]     = mrecs[ ii ].isolutes[ kk ].s * 1.e+13;
            yy[ kk ]     = mrecs[ ii ].isolutes[ kk ].k;
         }

         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );
         curv->setPen  ( pen_plot );    // Default color (yellow)
         curv->setData ( xx, yy, nlpts );
      } // END: model lines loop
   } // END: pre-fit lines

DbgLv(1) << "RP:PD (6)smin smax" << smin << smax;
   data_plot1->replot();
DbgLv(1) << "RP:PD  return";
DbgLv(1) << "RP:PD   R: smin smax" << smin << smax;
}

// Public slot to set a pointer to a model to use in the plot for highlights
void US_MLinesPlot::setModel( US_Model* a_model, QVector< ModelRecord >& mrs )
{
   model   = a_model;
   mrecs   = mrs;
DbgLv(1) << "RP:SM  bmndx" << bmndx;
DbgLv(1) << "RP:PD (7)smin smax" << smin << smax;

   bool fact_vis = ( model != 0 );
   lb_lefact->setVisible( fact_vis );
   ct_lefact->setVisible( fact_vis );
   lb_mefact->setVisible( fact_vis );
   ct_mefact->setVisible( fact_vis );
   lb_hefact->setVisible( fact_vis );
   ct_hefact->setVisible( fact_vis );
   lb_mpfact->setVisible( fact_vis );
   ct_mpfact->setVisible( fact_vis );
DbgLv(1) << "RP:PD (8)smin smax" << smin << smax;
}

// Private slot to handle change in Low-Elite Factor value
void US_MLinesPlot::updateLeFact( double value )
{
   le_fact  = value;
   ct_mefact->setRange( he_fact, le_fact, 0.001 );
   me_fact  = ct_mefact->value();
   ct_hefact->setRange( 1.01,    me_fact, 0.001 );
   he_fact  = ct_hefact->value();

   if ( model != 0 )
      plot_data();
}

// Private slot to handle change in Mid-Elite Factor value
void US_MLinesPlot::updateMeFact( double value )
{
   me_fact = value;
   ct_hefact->setRange( 1.01,    me_fact, 0.001 );
   he_fact = ct_hefact->value();
   ct_lefact->setRange( me_fact, 10.00,   0.001 );
   le_fact = ct_lefact->value();

   if ( model != 0 )
      plot_data();
}

// Private slot to handle change in High-Elite Factor value
void US_MLinesPlot::updateHeFact( double value )
{
   he_fact = value;
   ct_mefact->setRange( he_fact, le_fact, 0.001 );
   me_fact = ct_mefact->value();
   ct_lefact->setRange( me_fact, 10.00,   0.001 );
   le_fact = ct_lefact->value();

   if ( model != 0 )
      plot_data();
}

// Private slot to handle change in Mid-Poor Factor value
void US_MLinesPlot::updateMpFact( double value )
{
   mp_fact = value;
//DbgLv(1) << "updMF: smin smax" << smin << smax;

   if ( model != 0 )
      plot_data();
}

