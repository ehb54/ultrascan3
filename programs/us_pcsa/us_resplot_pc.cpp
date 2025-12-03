//! \file us_resplot_pc.cpp

#include "us_resplot_pc.h"
#include "us_pcsa.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include <qwt_legend.h>

// constructor:  residuals plot widget
US_ResidPlotPc::US_ResidPlotPc( QWidget* parent )
   : US_WidgetsDialog( parent, Qt::WindowFlags() )
{
   // lay out the GUI
   setObjectName( "US_ResidPlotPc" );
   setAttribute( Qt::WA_DeleteOnClose, true );
   setWindowTitle( tr( "Param. Constrained Spectrum Analysis "
                       "Data/Residuals Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 360 );
   QSize p2size( 560, 240 );

   dbg_level       = US_Settings::us_debug();
   resbmap         = 0;

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   datctrlsLayout  = new QGridLayout();
   resctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QVBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "PCSA Data Viewer" ) );
   QLabel* lb_resctrls    = us_banner( tr( "PCSA Residuals Viewer" ) );
   QLabel* lb_vari        = us_label(  tr( "Variance:" ) );
   QLabel* lb_rmsd        = us_label(  tr( "RMSD:" ) );

   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   QLayout* lo_plteda =
      us_checkbox( tr( "Plot Experimental Data" ),      ck_plteda, true );
   QLayout* lo_subtin =
      us_checkbox( tr( "Subtract Time Invariant Noise" ),     ck_subtin );
   QLayout* lo_subrin =
      us_checkbox( tr( "Subtract Radially Invariant Noise" ), ck_subrin );
   QLayout* lo_pltsda =
      us_checkbox( tr( "Plot Simulated/Modeled Data" ), ck_pltsda, true );
   QLayout* lo_addtin =
      us_checkbox( tr( "Add Time Invariant Noise" ),          ck_addtin );
   QLayout* lo_addrin =
      us_checkbox( tr( "Add Radially Invariant Noise" ),      ck_addrin );
   QLayout* lo_pltres =
      us_checkbox( tr( "Plot Residuals" ),              ck_pltres, true );
   QLayout* lo_plttin =
      us_checkbox( tr( "Plot Time Invariant Noise" ),         ck_plttin );
   QLayout* lo_pltrin =
      us_checkbox( tr( "Plot Radially Invariant Noise" ),     ck_pltrin );
   QLayout* lo_pltran =
      us_checkbox( tr( "Plot Random Noise"     ),             ck_pltran  );
   QLayout* lo_shorbm =
      us_checkbox( tr( "Show Residuals Bitmap" ),             ck_shorbm );

   le_vari   = us_lineedit( "", -1, true );
   le_rmsd   = us_lineedit( "", -1, true );

   datctrlsLayout->addWidget( lb_datctrls, 0, 0, 1, 8 );
   datctrlsLayout->addLayout( lo_plteda,   1, 0, 1, 8 );
   datctrlsLayout->addLayout( lo_subtin,   2, 1, 1, 7 );
   datctrlsLayout->addLayout( lo_subrin,   3, 1, 1, 7 );
   datctrlsLayout->addLayout( lo_pltsda,   4, 0, 1, 8 );
   datctrlsLayout->addLayout( lo_addtin,   5, 1, 1, 7 );
   datctrlsLayout->addLayout( lo_addrin,   6, 1, 1, 7 );

   resctrlsLayout->addWidget( lb_resctrls, 0, 0, 1, 8 );
   resctrlsLayout->addLayout( lo_pltres,   1, 0, 1, 8 );
   resctrlsLayout->addLayout( lo_plttin,   2, 0, 1, 8 );
   resctrlsLayout->addLayout( lo_pltrin,   3, 0, 1, 8 );
   resctrlsLayout->addLayout( lo_pltran,   4, 0, 1, 8 );
   resctrlsLayout->addLayout( lo_shorbm,   5, 0, 1, 8 );
   resctrlsLayout->addWidget( lb_vari,     6, 0, 1, 3 );
   resctrlsLayout->addWidget( le_vari,     6, 3, 1, 5 );
   resctrlsLayout->addWidget( lb_rmsd,     7, 0, 1, 3 );
   resctrlsLayout->addWidget( le_rmsd,     7, 3, 1, 5 );

   buttonsLayout ->addWidget( pb_close );

   plotLayout1 = new US_Plot( data_plot1,
         tr( "Experimental Data" ),
         tr( "Radius (cm)" ),
         tr( "Absorbance" ) );

   plotLayout2 = new US_Plot( data_plot2,
         tr( "Residuals" ),
         tr( "Radius (cm)" ),
         tr( "OD Difference" ) );

   data_plot1->setCanvasBackground( Qt::black );
   data_plot2->setCanvasBackground( Qt::black );
   data_plot1->setMinimumSize( p1size );
   data_plot2->setMinimumSize( p2size );

   rightLayout->addLayout( plotLayout1    );
   rightLayout->addLayout( plotLayout2    );

   leftLayout ->addLayout( datctrlsLayout );
   leftLayout ->addLayout( resctrlsLayout );
   leftLayout ->addStretch();
   leftLayout ->addLayout( buttonsLayout  );

   mainLayout->addLayout( leftLayout  );
   mainLayout->addLayout( rightLayout );

   connect( ck_plteda, SIGNAL( toggled( bool ) ),
            this,      SLOT( pedaCheck( bool ) ) );
   connect( ck_subtin, SIGNAL( toggled( bool ) ),
            this,      SLOT( stinCheck( bool ) ) );
   connect( ck_subrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( srinCheck( bool ) ) );
   connect( ck_pltsda, SIGNAL( toggled( bool ) ),
            this,      SLOT( psdaCheck( bool ) ) );
   connect( ck_addtin, SIGNAL( toggled( bool ) ),
            this,      SLOT( atinCheck( bool ) ) );
   connect( ck_addrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( arinCheck( bool ) ) );
   connect( ck_pltres, SIGNAL( toggled( bool ) ),
            this,      SLOT( presCheck( bool ) ) );
   connect( ck_plttin, SIGNAL( toggled( bool ) ),
            this,      SLOT( ptinCheck( bool ) ) );
   connect( ck_pltrin, SIGNAL( toggled( bool ) ),
            this,      SLOT( prinCheck( bool ) ) );
   connect( ck_pltran, SIGNAL( toggled( bool ) ),
            this,      SLOT( pranCheck( bool ) ) );
   connect( ck_shorbm, SIGNAL( toggled( bool ) ),
            this,      SLOT( srbmCheck( bool ) ) );

   connect( pb_close,  SIGNAL( clicked()   ),
            this,      SLOT( close_all()   ) );

   have_ed   = false;
   have_sd   = false;
   have_ti   = false;
   have_ri   = false;
   have_bm   = false;
   skip_plot = true;
DbgLv(1) << "RP: P" << ( parent != 0 );

   // Get data pointers from parent widget

   if ( parent )
   {
      US_pcsa*  mainw = (US_pcsa*)parent;
      edata           = mainw->mw_editdata();
      sdata           = mainw->mw_simdata();
      excllist        = mainw->mw_excllist();
      ti_noise        = mainw->mw_ti_noise();
      ri_noise        = mainw->mw_ri_noise();
      have_ed         = ( edata != 0 );
      have_sd         = ( sdata != 0 );
      have_ti         = ( ti_noise != 0  &&  ti_noise->count > 0 );
      have_ri         = ( ri_noise != 0  &&  ri_noise->count > 0 );
DbgLv(1) << "RP:edata  " << have_ed;
DbgLv(1) << "RP:sdata  " << have_sd;
DbgLv(1) << "RP:ti_noise count" << (have_ti ? ti_noise->count : 0);
DbgLv(1) << "RP:ri_noise count" << (have_ri ? ri_noise->count : 0);
   }

   else
   {
DbgLv(1) << "RP:edata  " << have_ed;
      qDebug() << "*ERROR* unable to get RP parent";
   }

   ck_subtin->setEnabled( have_ti );
   ck_subrin->setEnabled( have_ri );
   ck_addtin->setEnabled( have_ti );
   ck_addrin->setEnabled( have_ri );
   ck_plttin->setEnabled( have_ti );
   ck_pltrin->setEnabled( have_ri );

   ck_plteda->setChecked( true    );
   ck_subtin->setChecked( have_ti );
   ck_subrin->setChecked( have_ri );

   skip_plot = false;
   data_plot1->resize( p1size );
   data_plot2->resize( p2size );

   plot_data();

   setVisible( true );
   resize( p2size );
}

// plot-experimental-data box [un]checked
void US_ResidPlotPc::pedaCheck( bool chkd )
{
   if ( chkd )
   {  // box is being checked:  sub boxes enabled if data present
      ck_subtin->setEnabled( have_ti );
      ck_subrin->setEnabled( have_ri );
   }

   else
   {  // box is being unchecked:  sub boxes disabled
      ck_subtin->setEnabled( false );
      ck_subrin->setEnabled( false );
   }

   plot_data();
}

// subtract-ti-noise box [un]checked
void US_ResidPlotPc::stinCheck( bool )
{
   plot_data();
}

// subtract-ri-noise box [un]checked
void US_ResidPlotPc::srinCheck( bool )
{
   plot_data();
}

// plot-simulation-data box [un]checked
void US_ResidPlotPc::psdaCheck( bool chkd )
{
   if ( chkd )
   {  // box is being checked:  sub boxes enabled if data present
      ck_addtin->setEnabled( have_ti );
      ck_addrin->setEnabled( have_ri );
   }

   else
   {  // box is being unchecked:  sub boxes disabled
      ck_addtin->setEnabled( false );
      ck_addrin->setEnabled( false );
   }

   plot_data();
}

// add-ti-noise box [un]checked
void US_ResidPlotPc::atinCheck( bool )
{
   plot_data();
}

// add-ri-noise box [un]checked
void US_ResidPlotPc::arinCheck( bool )
{
   plot_data();
}

// plot-residuals  box [un]checked
void US_ResidPlotPc::presCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}

// plot-ti-noise box [un]checked
void US_ResidPlotPc::ptinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}

// plot-ri-noise box [un]checked
void US_ResidPlotPc::prinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltran->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}

// plot-random-noise box [un]checked
void US_ResidPlotPc::pranCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
   }

   skip_plot = false;

   plot_data();
}

// show-residual-bitmap box [un]checked
void US_ResidPlotPc::srbmCheck( bool chkd )
{
   if ( chkd )
   {  // bitmap checked:  replot to possibly build new map

      if ( have_bm )
      {  // if bitmap exists already, detect when closed
         connect( resbmap, SIGNAL( destroyed()   ),
                  this,    SLOT( resids_closed() ) );
      }

      plot_data();
   }
}

// close button clicked
void US_ResidPlotPc::close_all()
{
   if ( resbmap != 0 )
   {
      resbmap->close();
   }

   close();
}

// plot the data
void US_ResidPlotPc::plot_data()
{
   if ( skip_plot )  // avoid redundant successive calls
      return;

   plot_edata();     // plot experimental

   plot_rdata();     // plot residuals
}

// plot the experimental data
void US_ResidPlotPc::plot_edata()
{
   dataPlotClear( data_plot1 );

   bool   do_plteda = have_ed  &&  ck_plteda->isChecked();
   bool   do_pltsda = have_sd  &&  ck_pltsda->isChecked();
   bool   do_addtin = have_ti  &&  ck_addtin->isChecked();
   bool   do_subtin = have_ti  &&  ck_subtin->isChecked();
   bool   do_addrin = have_ri  &&  ck_addrin->isChecked();
   bool   do_subrin = have_ri  &&  ck_subrin->isChecked();

   int    points    = 0;
   int    count     = 0;
   double tinoi     = 0.0;
   double rinoi     = 0.0;
   double rl        = 0.0;
   //double vh        = 9999.0;
   double sval;

   if ( have_ed )
   {
      points   = edata->pointCount();
      rl       = edata->radius( 0 );
      //vh       = edata->value( 0, points - 1 );
      //vh      *= 1.05;
   }

   if ( !do_plteda  &&  !do_pltsda )
   {  // no real experimental plot specified:  re-do plot and return
      data_plot1->replot();
      return;
   }

   if ( do_plteda  &&  ! do_pltsda )
      data_plot1->setTitle( tr( "Experimental Data" ) );

   else if ( do_plteda  &&  do_pltsda )
      data_plot1->setTitle( tr( "Experimental and Simulated Data" ) );

   else
      data_plot1->setTitle( tr( "Simulated Data" ) );

   us_grid( data_plot1 );

   if ( do_plteda )
   {  // set title and values count for experimental data
      data_plot1->setAxisTitle( QwtPlot::yLeft,
         tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
      points   = edata->pointCount();
   }

   if ( do_pltsda )
   {  // set title and values count for simulation data
      if ( have_ed )
         data_plot1->setAxisTitle( QwtPlot::yLeft,
            tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
      else
         data_plot1->setAxisTitle( QwtPlot::yLeft, tr( "Absorbance" ) );

      count    = sdata->pointCount();
   }

   count    = ( points > count ) ? points : count;  // maximum array count

   QVector< double > rvec( count, 0.0 );
   QVector< double > vvec( count, 0.0 );

   double* rr  = rvec.data();
   double* vv  = vvec.data();

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   if ( do_plteda )
   {  // plot experimental curves
      points   = edata->pointCount();
      count    = edata->scanCount();
      rinoi    = 0.0;
      tinoi    = 0.0;

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = edata->radius( jj );
      }

      for ( int ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan
         if ( excllist->contains( ii ) )  continue;

         if ( do_subrin )
            rinoi    = ri_noise->values[ ii ];

         for ( int jj = 0; jj < points; jj++ )
         {  // each y is reading, optionally minus some noise
            if ( do_subtin )
               tinoi    = ti_noise->values[ jj ];

            vv[ jj ] = edata->value( ii, jj ) - rinoi - tinoi;
         }

         title   = tr( "Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen    ( pen_plot );
         curv->setSamples( rr, vv, points );
      }
   }

   if ( do_pltsda )
   {  // plot simulation curves
      points   = sdata->pointCount();
      count    = sdata->scanCount();
      rinoi    = 0.0;
      tinoi    = 0.0;

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      for ( int ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan
         if ( excllist->contains( ii ) )  continue;

         if ( do_addrin )
            rinoi    = ri_noise->values[ ii ];

         for ( int jj = 0; jj < points; jj++ )
         {  // each y is reading, optionally plus some noise
            if ( do_addtin )
               tinoi    = ti_noise->values[ jj ];

            if ( rr[ jj ] >= rl )
               sval     = sdata->value( ii, jj ) + rinoi + tinoi;

            else
               sval     = edata->value( ii, jj ) - rinoi - tinoi;

            //if ( sval > vh )
            //   sval     = vv[ jj - 1 ];

            vv[ jj ] = sval;
         }

         title   = tr( "S-Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen    ( pen_red );
         curv->setSamples( rr, vv, points );
      }
   }

   data_plot1->replot();
}

// plot the residual data
void US_ResidPlotPc::plot_rdata()
{
   dataPlotClear( data_plot2 );

   bool   do_pltres = have_ed  &&  ck_pltres->isChecked()  &&  have_sd;
   bool   do_plttin = have_ti  &&  ck_plttin->isChecked();
   bool   do_pltrin = have_ri  &&  ck_pltrin->isChecked();
   bool   do_pltran = have_ed  &&  ck_pltran->isChecked()  &&  have_sd;
   bool   do_shorbm = have_ed  &&  ck_shorbm->isChecked()  &&  have_sd;
   bool   do_addtin = have_ti  &&  ck_addtin->isChecked();
   bool   do_subtin = have_ti  &&  ck_subtin->isChecked();
   bool   do_addrin = have_ri  &&  ck_addrin->isChecked();
   bool   do_subrin = have_ri  &&  ck_subrin->isChecked();

   int    points    = edata->pointCount();
   int    count     = edata->scanCount();
   double tinoi     = 0.0;
   double rinoi     = 0.0;
   double rvalu     = 0.0;
   double rmsd      = 0.0;

   if ( !do_pltres  &&  !do_plttin  && !do_pltrin  &&
        !do_pltran  &&  !do_shorbm )
   {  // if no plots specified, just clean up plot and return
      data_plot2->replot();
      return;
   }

   us_grid( data_plot2 );
   data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Radius (cm)" ) );

   int vsize = qMax( sdata->pointCount(), qMax( points, count ) );

   QVector< double > rvec( vsize, 0.0 );
   QVector< double > vvec( vsize, 0.0 );

   double* rr  = rvec.data();
   double* vv  = vvec.data();

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_plot( Qt::green );

   points     = edata->pointCount();
   count      = edata->scanCount();

   // plot a zero line in red
   double xlo = edata->radius(          0 ) - 0.05;
   double xhi = edata->radius( points - 1 ) + 0.05;
   rr[ 0 ]    = !do_pltrin ? xlo : 0.0;
   rr[ 1 ]    = !do_pltrin ? xhi : double( count );
   vv[ 0 ]    = 0.0;
   vv[ 1 ]    = 0.0;
   curv       = us_curve( data_plot2, "zero-line" );
   curv->setPen    ( QPen( QBrush( Qt::red ), 2 ) );
   curv->setSamples( rr, vv, 2 );

   if ( do_pltres )
   {  // plot residuals
      data_plot2->setTitle( tr( "Residuals" ) );

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      int    kntva = 0;

      for ( int ii = 0; ii < count; ii++ )
      {  // get readings (y) for each scan
         if ( excllist->contains( ii ) )  continue;

         rinoi    = 0.0;
         if ( do_subrin )
            rinoi    = ri_noise->values[ ii ];
         if ( do_addrin )
            rinoi   += ri_noise->values[ ii ];

         for ( int jj = 0; jj < points; jj++ )
         {  // each residual is e-value minus s-value
            tinoi    = 0.0;
            if ( do_subtin )
               tinoi    = ti_noise->values[ jj ];
            if ( do_addtin )
               tinoi   += ti_noise->values[ jj ];

            rvalu    = edata->value( ii, jj ) - sdata->value( ii, jj )
                       - rinoi - tinoi;

            rmsd    += sq( rvalu );
            kntva++;

            vv[ jj ] = rvalu;
         }

         title   = tr( "resids " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen    ( pen_plot );
         curv->setStyle  ( QwtPlotCurve::Dots );
         curv->setSamples( rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( kntva );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
//DbgLv(1) << "RP:(1res)rmsd" << rmsd;
//*Debug
//DbgLv(1) << "BEFORE data00" << edata->value(0,0);
//ti_noise->apply_to_data(*edata);
//DbgLv(1) << " TIrmv  data00" << edata->value(0,0);
//ti_noise->apply_to_data(*edata,false);
//DbgLv(1) << "  TIadd  data00" << edata->value(0,0);
//ri_noise->apply_to_data(*edata);
//DbgLv(1) << " RIrmv  data00" << edata->value(0,0);
//ri_noise->apply_to_data(*edata,false);
//DbgLv(1) << "  RIadd  data00" << edata->value(0,0);
//US_Noise::apply_noise(*edata,ti_noise);
//DbgLv(1) << " TIrmv  data00" << edata->value(0,0);
//US_Noise::apply_noise(*edata,ti_noise,false);
//DbgLv(1) << "  TIadd  data00" << edata->value(0,0);
//US_Noise::apply_noise(*edata,ri_noise);
//DbgLv(1) << " RIrmv  data00" << edata->value(0,0);
//US_Noise::apply_noise(*edata,ri_noise,false);
//DbgLv(1) << "  RIadd  data00" << edata->value(0,0);
   }

   else if ( do_plttin )
   {  // plot time-invariant noise
      data_plot2->setTitle( tr( "Time-Invariant Noise" ) );

      for ( int jj = 0; jj < points; jj++ )
      {  // accumulate radii and noise values
         rr[ jj ] = edata->radius( jj );
         vv[ jj ] = ti_noise->values[ jj ];
      }

      title   = tr( "ti_noise" );
      curv    = us_curve( data_plot2, title );

      curv->setPen    ( pen_plot );
      curv->setSamples( rr, vv, points );
   }

   else if ( do_pltrin )
   {  // plot radially-invariant noise
      data_plot2->setTitle( tr( "Radially-Invariant Noise" ) );

      for ( int ii = 0; ii < count; ii++ )
      {  // accumulate scan numbers and noise values
         rr[ ii ] = (double)( ii + 1 );
         vv[ ii ] = ri_noise->values[ ii ];
      }

      title   = tr( "ri_noise" );
      curv    = us_curve( data_plot2, title );

      data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Scan Number" ) );
      curv->setPen    ( pen_plot );
      curv->setSamples( rr, vv, count );
   }

   else if ( do_pltran )
   {  // plot random noise
      data_plot2->setTitle( tr( "Random Noise" ) );

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      int    kntva    = 0;

      for ( int ii = 0; ii < count; ii++ )
      {  // get random noise (y) for each scan
         if ( excllist->contains( ii ) )  continue;

         rinoi    = have_ri ? ri_noise->values[ ii ] : 0.0;

         for ( int jj = 0; jj < points; jj++ )
         {  // each random value is e-value minus s-value with optional noise
            tinoi    = have_ti ? ti_noise->values[ jj ] : 0.0;
            rvalu    = edata->value( ii, jj ) - sdata->value( ii, jj )
                       - rinoi - tinoi;

            rmsd    += sq( rvalu );
            kntva++;

            vv[ jj ] = rvalu;
         }

         title   = tr( "random noise " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen    ( pen_plot );
         curv->setStyle  ( QwtPlotCurve::Dots );
         curv->setSamples( rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( kntva );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
//DbgLv(1) << "RP:(2ran)rmsd" << rmsd;
   }


   if ( do_shorbm )
   {  // show residuals bitmap (if not already shown)

      QVector< QVector< double > > resids;
      QVector< double >            resscan;

      resids .resize( count );
      resscan.resize( points );
      rmsd      = 0.0;
      int kntva = 0;

      for ( int ii = 0; ii < count; ii++ )
      {  // build a vector for each scan
         if ( excllist->contains( ii ) )
         {
            resscan.fill( 0.0 );
            resids[ ii ] = resscan;
            continue;
         }

         rinoi    = 0.0;
         if ( do_subrin )
            rinoi    = ri_noise->values[ ii ];
         if ( do_addrin )
            rinoi   += ri_noise->values[ ii ];

         for ( int jj = 0; jj < points; jj++ )
         {  // build residual values within a scan
            tinoi    = 0.0;
            if ( do_subtin )
               tinoi    = ti_noise->values[ jj ];
            if ( do_addtin )
               tinoi   += ti_noise->values[ jj ];

            rvalu         = edata->value( ii, jj ) - sdata->value( ii, jj )
                            - rinoi - tinoi;

            rmsd         += sq( rvalu );
            kntva++;

            resscan[ jj ] = rvalu;
         }

         resids[ ii ] = resscan;
      }

      rmsd   /= (double)( kntva );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );

      if ( resbmap )
      {  // already have resbmap:  just replot residuals bitmap
         resbmap->replot( resids );
         resbmap->raise();
      }

      else
      {  // pop up a little dialog with residuals bitmap
         resbmap = new US_ResidsBitmap( resids );
         connect( resbmap, SIGNAL( destroyed() ),
                  this,    SLOT(   resids_closed() ) );
         resbmap->move( this->pos() + QPoint( 100, 100 ) );
         resbmap->show();
         resbmap->raise();
      }

      qApp->processEvents();
   }

   // display curves we have created; then clean up

   data_plot2->setAxisScale( QwtPlot::xBottom, xlo, xhi );
   data_plot2->replot();
}

// react to residual bitmap having been closed
void US_ResidPlotPc::resids_closed()
{
DbgLv(1) << "Resids BitMap Closed!!!";
   resbmap = 0;
   have_bm = false;
   ck_shorbm->setChecked( false );
}

// Return a pointer to the QwtPlot for the upper plot
QwtPlot* US_ResidPlotPc::rp_data_plot1()
{
   return data_plot1;
}

// Return a pointer to the QwtPlot for the lower plot
QwtPlot* US_ResidPlotPc::rp_data_plot2()
{
   return data_plot2;
}

