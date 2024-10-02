//! \file us_resplot_fem.cpp

#include "us_resplot_fem.h"
#include "us_fematch.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_constants.h"
#include "../us_autoflow_analysis/us_autoflow_analysis.h"
#include "../us_reporter_gmp/us_reporter_gmp.h"

#include <qwt_legend.h>
#if QT_VERSION < 0x050000
#define setSamples(a,b,c)  setData(a,b,c)
#endif

// constructor:  residuals plot widget
// US_ResidPlotFem::US_ResidPlotFem( QWidget* parent, const bool auto_mode )
//    : US_WidgetsDialog( 0, 0 )
US_ResidPlotFem::US_ResidPlotFem( QWidget* parent, const QString auto_mode )
   : US_WidgetsDialog( 0, 0 )
{
   // this->a_mode = auto_mode;
   this->a_mode = auto_mode;
  
   // lay out the GUI
   setWindowTitle( tr( "Finite Element Data/Residuals Viewer" ) );
   setPalette( US_GuiSettings::frameColor() );

   QSize p1size( 560, 360 );
   QSize p2size( 560, 240 );

   QSize p11size( 670, 360);
   QSize p21size( 670, 240);

   dbg_level       = US_Settings::us_debug();

   mainLayout      = new QHBoxLayout( this );
   leftLayout      = new QVBoxLayout();
   rightLayout     = new QVBoxLayout();
   datctrlsLayout  = new QGridLayout();
   resctrlsLayout  = new QGridLayout();
   buttonsLayout   = new QVBoxLayout();

   mainLayout->setSpacing        ( 2 );
   mainLayout->setContentsMargins( 2, 2, 2, 2 );

   QLabel* lb_datctrls    = us_banner( tr( "FE Analysis Data Viewer" ) );
   QLabel* lb_resctrls    = us_banner( tr( "FE Analysis Residuals Viewer" ) );
   QLabel* lb_vari        = us_label(  tr( "Variance:" ) );
   QLabel* lb_rmsd        = us_label(  tr( "RMSD:" ) );

   QPushButton* pb_close  = us_pushbutton( tr( "Close" ) );

   QLayout* lo_plteda =
      us_checkbox( tr( "Plot Experimental Data" ),            ck_plteda, true );
   QLayout* lo_subtin =
      us_checkbox( tr( "Subtract Time Invariant Noise" ),     ck_subtin );
   QLayout* lo_subrin =
      us_checkbox( tr( "Subtract Radially Invariant Noise" ), ck_subrin );
   QLayout* lo_pltsda =
      us_checkbox( tr( "Plot Simulated/Modeled Data" ),       ck_pltsda, true );
   QLayout* lo_addtin =
      us_checkbox( tr( "Add Time Invariant Noise" ),          ck_addtin );
   QLayout* lo_addrin =
      us_checkbox( tr( "Add Radially Invariant Noise" ),      ck_addrin );
   QLayout* lo_pltres =
      us_checkbox( tr( "Plot Residuals" ),                    ck_pltres, true );
   QLayout* lo_plttin =
      us_checkbox( tr( "Plot Time Invariant Noise" ),         ck_plttin );
   QLayout* lo_pltrin =
      us_checkbox( tr( "Plot Radially Invariant Noise" ),     ck_pltrin );
   QLayout* lo_pltran =
      us_checkbox( tr( "Plot Random Noise"     ),             ck_pltran  );
   QLayout* lo_shorbm =
      us_checkbox( tr( "Show Residuals Bitmap" ),             ck_shorbm );

   le_vari   = us_lineedit( "0.0", -1, true );
   le_rmsd   = us_lineedit( "0.0", -1, true );

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
         tr( "Absorbance" ),
         true, "^Curve" );

   plotLayout2 = new US_Plot( data_plot2,
         tr( "Residuals" ),
         tr( "Radius (cm)" ),
         tr( "OD Difference" ) );

   data_plot1->setMinimumSize( p1size );
   data_plot2->setMinimumSize( p2size );

   // if ( a_mode )
   //   {
   //     data_plot1->setMinimumSize( p11size );
   //     data_plot2->setMinimumSize( p21size );
   //   }
   if ( !a_mode.isEmpty() && a_mode == "ANALYSIS" )
     {
       data_plot1->setMinimumSize( p11size );
       data_plot2->setMinimumSize( p21size );
     }
   

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

   if ( parent )
   {
     //if ( !a_mode )
     if ( a_mode.isEmpty() )
       {
	 // Get data pointers from parent of parent
	 qDebug() << "THIS will be pointer to us_fematch---";

	 US_FeMatch* fem = (US_FeMatch*)parent;
	 edata           = fem->fem_editdata();
	 sdata           = fem->fem_simdata();
	 excllist        = fem->fem_excllist();
	 ti_noise        = fem->fem_ti_noise();
	 ri_noise        = fem->fem_ri_noise();
	 resbmap         = fem->fem_resbmap();
	 have_ed         = ( edata != 0 );
	 have_sd         = ( sdata != 0 );
	 have_ti         = ( ti_noise != 0  &&  ti_noise->count > 0 );
	 have_ri         = ( ri_noise != 0  &&  ri_noise->count > 0 );
	 have_bm         = ( resbmap != 0 );
	 DbgLv(1) << "RP:edata  " << have_ed;
	 DbgLv(1) << "RP:sdata  " << have_sd;
	 DbgLv(1) << "RP:ti_noise count" << (have_ti ? ti_noise->count : 0);
	 DbgLv(1) << "RP:ri_noise count" << (have_ri ? ri_noise->count : 0);
	 DbgLv(1) << "RP:resbmap" << have_bm;
       }
     else
       {
	 if ( a_mode == "ANALYSIS" ) 
	   {
	     qDebug() << "THIS will be pointer to autoflow_analsyis---";
	     //Autoflow_analysis
	     US_Analysis_auto* aa = (US_Analysis_auto*)parent;
	     edata           = aa->aa_editdata();
	     sdata           = aa->aa_simdata();
	     excllist        = aa->aa_excllist();
	     ti_noise        = aa->aa_ti_noise();
	     ri_noise        = aa->aa_ri_noise();
	     resbmap         = aa->aa_resbmap();
	     have_ed         = ( edata != 0 );
	     have_sd         = ( sdata != 0 );
	     have_ti         = ( ti_noise != 0  &&  ti_noise->count > 0 );
	     have_ri         = ( ri_noise != 0  &&  ri_noise->count > 0 );
	     have_bm         = ( resbmap != 0 );
	     qDebug() << "RP:edata  " << have_ed;
	     qDebug() << "RP:sdata  " << have_sd;
	     
	     qDebug() << "RP:have_ti " << have_ti;
	     qDebug() << "RP:have_ri " << have_ri;
	     
	     
	     qDebug() << "RP:ti_noise count1" << ti_noise->count;
	     qDebug() << "RP:ri_noise count1" << ri_noise->count;
	     
	     qDebug() << "RP:ti_noise count" << (have_ti ? ti_noise->count : 0);
	     qDebug() << "RP:ri_noise count" << (have_ri ? ri_noise->count : 0);
	     qDebug() << "RP:resbmap" << have_bm;
	     
	     tripleInfo = aa->aa_tripleInfo();
	     
	   }
	 
	 if ( a_mode == "REPORT" ) 
	   {
	     qDebug() << "THIS will be pointer to autoflow_report (US_ReporterGMP for now !!!) ---";
	     //Autoflow_report -- US_ReporterGMP for now!!!
	     US_ReporterGMP* rg = (US_ReporterGMP*)parent;
	     edata           = rg->rg_editdata();
	     sdata           = rg->rg_simdata();
	     excllist        = rg->rg_excllist();
	     ti_noise        = rg->rg_ti_noise();
	     ri_noise        = rg->rg_ri_noise();
	     resbmap         = rg->rg_resbmap();
	     have_ed         = ( edata != 0 );
	     have_sd         = ( sdata != 0 );
	     have_ti         = ( ti_noise != 0  &&  ti_noise->count > 0 );
	     have_ri         = ( ri_noise != 0  &&  ri_noise->count > 0 );
	     have_bm         = ( resbmap != 0 );
	     qDebug() << "RP:edata  " << have_ed;
	     qDebug() << "RP:sdata  " << have_sd;
	     
	     qDebug() << "RP:have_ti " << have_ti;
	     qDebug() << "RP:have_ri " << have_ri;
	     
	     
	     qDebug() << "RP:ti_noise count1" << ti_noise->count;
	     qDebug() << "RP:ri_noise count1" << ri_noise->count;
	     
	     qDebug() << "RP:ti_noise count" << (have_ti ? ti_noise->count : 0);
	     qDebug() << "RP:ri_noise count" << (have_ri ? ri_noise->count : 0);
	     qDebug() << "RP:resbmap" << have_bm;
	     
	     tripleInfo = rg->rg_tripleInfo();
	   }
       }
   }
   
   else
   {
      qDebug() << "*ERROR* unable to get RP parent";
   }

   ck_subtin->setEnabled( have_ti );
   ck_subrin->setEnabled( have_ri );
   ck_addtin->setEnabled( have_ti );
   ck_addrin->setEnabled( have_ri );
   ck_plttin->setEnabled( have_ti );
   ck_pltrin->setEnabled( have_ri );

   ck_plteda->setChecked( true );
   ck_subtin->setChecked( have_ti );
   ck_subrin->setChecked( have_ri );

   skip_plot = false;
   data_plot1->resize( p1size );
   data_plot2->resize( p2size );

   //qDebug() << "RESPLOT_FEM: before plot--";

   plot_data();

   //qDebug() << "RESPLOT_FEM: after plot--";

   if ( !a_mode.isEmpty() && a_mode == "REPORT" )
     setVisible( false );
   else
     setVisible( true );
   
   resize( p2size );

   
}

// externally force a specific plot for lower plot
void US_ResidPlotFem::set_plot( int plotf )
{
   if ( plotf == 1 )
   {  // ti_noise plot
      ck_plttin->setChecked( true );
   }

   else if ( plotf == 2 )
   {  // ri_noise plot
      ck_pltrin->setChecked( true );
   }

   else if ( plotf == 0 )
   {  // residuals plot
      ck_pltres->setChecked( true );
   }
}

// Return a pointer to the QwtPlot for the upper plot
QwtPlot* US_ResidPlotFem::rp_data_plot1()
{
   return data_plot1;
}

// Return a pointer to the QwtPlot for the lower plot
QwtPlot* US_ResidPlotFem::rp_data_plot2()
{
   return data_plot2;
}

// plot-experimental-data box [un]checked
void US_ResidPlotFem::pedaCheck( bool chkd )
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
void US_ResidPlotFem::stinCheck( bool )
{
   plot_data();
}

// subtract-ri-noise box [un]checked
void US_ResidPlotFem::srinCheck( bool )
{
   plot_data();
}

// plot-simulation-data box [un]checked
void US_ResidPlotFem::psdaCheck( bool chkd )
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
void US_ResidPlotFem::atinCheck( bool )
{
   plot_data();
}

// add-ri-noise box [un]checked
void US_ResidPlotFem::arinCheck( bool )
{
   plot_data();
}

// plot-residuals  box [un]checked
void US_ResidPlotFem::presCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      connect_pboxes( false );
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
      connect_pboxes( true  );
   }

   skip_plot = false;

   plot_data();
}

// plot-ti-noise box [un]checked
void US_ResidPlotFem::ptinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      connect_pboxes( false );
      ck_pltres->setChecked( false );
      ck_pltrin->setChecked( false );
      ck_pltran->setChecked( false );
      connect_pboxes( true  );
   }

   skip_plot = false;

   plot_data();
}

// plot-ri-noise box [un]checked
void US_ResidPlotFem::prinCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      connect_pboxes( false );
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltran->setChecked( false );
      connect_pboxes( true  );
   }

   skip_plot = false;

   plot_data();
}

// plot-random-noise box [un]checked
void US_ResidPlotFem::pranCheck( bool chkd )
{
   skip_plot = true;

   if ( chkd )
   {  // alternate residual plots unchecked
      connect_pboxes( false );
      ck_pltres->setChecked( false );
      ck_plttin->setChecked( false );
      ck_pltrin->setChecked( false );
      connect_pboxes( true  );
   }

   skip_plot = false;

   plot_data();
}

// show-residual-bitmap box [un]checked
void US_ResidPlotFem::srbmCheck( bool chkd )
{

  if ( a_mode.isEmpty()  )  //Default case for stand-alone us_fematch
    {
      if ( chkd )
	{  // bitmap checked:  replot to possibly build new map
	  have_bm         = ( resbmap != 0 );
	  	  
	  if ( have_bm )
	    {  // if bitmap exists already, detect when closed
	      connect( resbmap, SIGNAL( destroyed()   ),
		       this,    SLOT( resids_closed() ) );
	    }
	  
	  plot_data();
	}
    }
  else
    {
      if ( a_mode == "ANALYSIS" )
	{
	  qDebug() << "ResBMap checkbox clicked in ANALYSIS mode!!! ";

	  if ( chkd )
	    {
	      have_ed = true;
	      have_sd = true;;

	      qDebug() << "ck_shorbm->isChecked(), have_ed, have_sd -- "
		       << ck_shorbm->isChecked() << have_ed << have_sd;

	      plot_data();
	      resbmap->activateWindow();
	      //resbmap->setParent(this, Qt::Widget);
	    }
	  else
	    {
	      resbmap->close();
	    }
	}
    }
   
}

// close button clicked
void US_ResidPlotFem::close_all()
{
   if ( resbmap != 0 )
      resbmap->close();

   close();
}

// plot the data
void US_ResidPlotFem::plot_data()
{
  if ( skip_plot )  // avoid redundant successive calls
      return;

   plot_edata();     // plot experimental

   plot_rdata();     // plot residuals
}

// plot the experimental data
void US_ResidPlotFem::plot_edata()
{
   // Get any previous map colors set up
   QList< QColor > mcolors;
   int nmcols        = plotLayout1->map_colors( mcolors );
   // Clear the upper plot for re-do
   dataPlotClear( data_plot1 );

   //qDebug() << "PLOT edata 1 ";

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
   double vh        = 9999.0;
   double sval;

   // qDebug() << "PLOT edata 2 ";
   
   if ( have_ed )
   {
      points   = edata->pointCount();
      rl       = edata->radius( 0 );
   }

   if ( !do_plteda  &&  !do_pltsda )
   {  // no real experimental plot specified:  re-do plot and return
      data_plot1->replot();
      return;
   }

   //   qDebug() << "PLOT edata 3 ";

   //if ( !a_mode )
   if ( a_mode.isEmpty() )  
     {
       if      ( do_plteda  &&  !do_pltsda )
	 data_plot1->setTitle( tr( "Experimental Data" ) );
       
       else if ( do_plteda  &&  do_pltsda  )
	 data_plot1->setTitle( tr( "Experimental and Simulated Data" ) );
       
       else if ( do_pltsda )
	 data_plot1->setTitle( tr( "Simulated Data" ) );
     }
   else
     {
       if ( a_mode == "ANALYSIS" )
	 {
	   // For autoflow Analysis
	   if      ( do_plteda  &&  !do_pltsda )
	     {
	       QString exp_data = QString( tr( "Experimental Data " ) ) + tripleInfo;
	       data_plot1->setTitle( exp_data  );
	     }
	   else if ( do_plteda  &&  do_pltsda  )
	     {
	       QString expsim_data = QString( tr( "Experimental and Simulated Data " ) ) + tripleInfo;
	       data_plot1->setTitle( expsim_data ); 
	     }
	   else if ( do_pltsda )
	     {
	       QString sim_data = QString( tr( "Simulated Data " ) ) + tripleInfo;
	       data_plot1->setTitle( sim_data );
	     }
	 }
       if ( a_mode == "REPORT" )
	 {
	   // For autoflow REPORT
	   if      ( do_plteda  &&  !do_pltsda )
	     {
	       QString exp_data = QString( tr( "Experimental Data " ) );
	       data_plot1->setTitle( exp_data  );
	     }
	   else if ( do_plteda  &&  do_pltsda  )
	     {
	       QString expsim_data = QString( tr( "Experimental and Simulated Data " ) );
	       data_plot1->setTitle( expsim_data ); 
	     }
	   else if ( do_pltsda )
	     {
	       QString sim_data = QString( tr( "Simulated Data " ) );
	       data_plot1->setTitle( sim_data );
	     }
	 }
     }
   

   us_grid( data_plot1 );

   if ( do_plteda )
   {  // set title and values count for experimental data
      data_plot1->setAxisTitle( QwtPlot::yLeft,
         tr( "Absorbance at " ) + edata->wavelength + tr( " nm" ) );
      points   = edata->pointCount();
   }

   //qDebug() << "PLOT edata 4 ";
   
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

   //qDebug() << "PLOT edata 5 ";
   
   QVector< double > rvec( count, 0.0 );
   QVector< double > vvec( count, 0.0 );
   double* rr  = rvec.data();
   double* vv  = vvec.data();

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_red(  Qt::red );
   QPen          pen_plot( US_GuiSettings::plotCurve() );

   //qDebug() << "PLOT edata 6 ";

   if ( do_plteda )
   {  // plot experimental curves
      points   = edata->pointCount();
      count    = edata->scanCount();
      rinoi    = 0.0;
      tinoi    = 0.0;
      if ( nmcols == 1 )
         pen_plot       = QPen( mcolors[ 0 ] );


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

         if ( nmcols > 1 )
         {  // Get pen plot color from gradient
            int colx       = ii % nmcols;
            pen_plot       = QPen( mcolors[ colx ] );
         }

         curv->setPen    ( pen_plot );
         curv->setSamples( rr, vv, points );
      }
   }

   //qDebug() << "PLOT edata 7 ";


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

            if ( sval > vh )
               sval     = vv[ jj - 1 ];

            vv[ jj ] = sval;
         }

         title   = tr( "S-Curve " ) + QString::number( ii );
         curv    = us_curve( data_plot1, title );

         curv->setPen    ( pen_red );
         curv->setSamples( rr, vv, points );
      }
   }

   //qDebug() << "PLOT edata 8 ";
   
   data_plot1->replot();

   //qDebug() << "PLOT edata 9";
}

// plot the residual data
void US_ResidPlotFem::plot_rdata()
{
   dataPlotClear( data_plot2 );

   bool   do_pltres = have_ed  &&  ck_pltres->isChecked()  &&  have_sd;
   bool   do_plttin = have_ti  &&  ck_plttin->isChecked();
   bool   do_pltrin = have_ri  &&  ck_pltrin->isChecked();
   bool   do_pltran = have_ed  &&  ck_pltran->isChecked()  &&  have_sd;
   bool   do_shorbm = have_ed  &&  ck_shorbm->isChecked()  &&  have_sd;

   qDebug() << "do_shorbm -- " << do_shorbm;
   
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

   int    vsize = qMax( sdata->pointCount(), qMax( points, count ) );

   QVector< double > rvec( vsize, 0.0 );
   QVector< double > vvec( vsize, 0.0 );
   double* rr   = rvec.data();
   double* vv   = vvec.data();

   QString       title;
   QwtPlotCurve* curv;
   QPen          pen_plot( Qt::green );

   // plot a zero line in red
   double rmin  = sdata->radius( 0 ) - 0.02;
   double rmax  = sdata->radius( points - 1 ) + 0.02;
   rr[ 0 ] = !do_pltrin ? rmin : 0.0;
   rr[ 1 ] = !do_pltrin ? rmax : double( count );
   vv[ 0 ] = 0.0;
   vv[ 1 ] = 0.0;
   curv    = us_curve( data_plot2, "zero-line" );
   curv->setPen    ( QPen( QBrush( Qt::red ), 2 ) );
   curv->setSamples( rr, vv, 2 );

   if ( do_pltres )
   {  // plot residuals
      data_plot2->setTitle( tr( "Residuals" ) );

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      int    kpts  = 0;

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
            kpts++;

            vv[ jj ] = rvalu;
         }

         title   = tr( "resids " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen    ( pen_plot );
         curv->setStyle  ( QwtPlotCurve::Dots );
         curv->setSamples( rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( kpts );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
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
      rmin    = 1.0;
      rmax    = (double)count;

      data_plot2->setAxisTitle( QwtPlot::xBottom, tr( "Scan Number" ) );
      curv->setPen    ( pen_plot );
      curv->setSamples( rr, vv, count );
   }

   else if ( do_pltran )
   {  // plot random noise
      data_plot2->setTitle( tr( "Random Noise" ) );
      points   = sdata->pointCount();
      count    = sdata->scanCount();

      for ( int jj = 0; jj < points; jj++ )
      {  // get radii (x) just once
         rr[ jj ] = sdata->radius( jj );
      }

      int    kpts  = 0;

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
            kpts++;

            vv[ jj ] = rvalu;
         }

         title   = tr( "random noise " ) + QString::number( ii );
         curv    = us_curve( data_plot2, title );

         curv->setPen    ( pen_plot );
         curv->setStyle  ( QwtPlotCurve::Dots );
         curv->setSamples( rr, vv, points );
      }

      // display variance and RMSD
      rmsd   /= (double)( kpts );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );
   }


   if ( do_shorbm )
   {  // show residuals bitmap (if not already shown)

      QVector< QVector< double > > resids;
      QVector< double >            resscan;

      resids .resize( count );
      resscan.resize( points );
      rmsd         = 0.0;
      int    kpts  = 0;

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
            kpts++;

            resscan[ jj ] = rvalu;
         }

         resids[ ii ] = resscan;
      }

      rmsd   /= (double)( kpts );
      le_vari->setText( QString::number( rmsd ) );
      rmsd    = sqrt( rmsd );
      le_rmsd->setText( QString::number( rmsd ) );

      if ( resbmap )
      {  // already have resbmap:  just replot residuals bitmap
         resbmap->replot( resids );
         resbmap->raise();
         resbmap->activateWindow();

	 qDebug() << "ResBitMap: have window already";
      }

      else
      {  // pop up a little dialog with residuals bitmap

	qDebug() << "ResBitMap: NO window yet";
	
         resbmap = new US_ResidsBitmap( resids );
         connect( resbmap, SIGNAL( destroyed() ),
                  this,    SLOT(   resids_closed() ) );
         resbmap->move( this->pos() + QPoint( 100, 100 ) );

	 if ( !a_mode.isEmpty() && a_mode == "ANALYSIS" )
	   {
	     //resbmap->setWindowFlags( Qt::Dialog );
	     //resbmap->setWindowModality(Qt::ApplicationModal);
	     //resbmap->setParent(this, Qt::Window);
	     //resbmap->disconnect();
	     resbmap->setParent(this, Qt::Dialog);
	     //resbmap->setWindowFlags ( Qt::WindowTitleHint );
	   }
	 
         resbmap->show();
         resbmap->raise();
      }

      qApp->processEvents();
   }

   // display curves we have created; then clean up

   data_plot2->setAxisScale( QwtPlot::xBottom, rmin, rmax );
   data_plot2->replot();
}

// react to residual bitmap having been closed
void US_ResidPlotFem::resids_closed()
{
DbgLv(1) << "Resids BitMap Closed!!!";
   resbmap = 0;
   have_bm = false;

   if ( a_mode.isEmpty() )
     ck_shorbm->setChecked( false );
   else
     {
       if ( a_mode == "ANALYSIS" )
	 {
	   ck_shorbm->disconnect();
	   ck_shorbm->setChecked( false );
	   connect( ck_shorbm, SIGNAL( toggled( bool ) ),
		    this,      SLOT( srbmCheck( bool ) ) );
	 }
     }
}

// Connect/Disconnect plot type boxes
void US_ResidPlotFem::connect_pboxes( bool conn )
{
   if ( conn )
   {  // Connect slots for plot boxes
      connect( ck_pltres, SIGNAL( toggled( bool ) ),
               this,      SLOT( presCheck( bool ) ) );
      connect( ck_plttin, SIGNAL( toggled( bool ) ),
               this,      SLOT( ptinCheck( bool ) ) );
      connect( ck_pltrin, SIGNAL( toggled( bool ) ),
               this,      SLOT( prinCheck( bool ) ) );
      connect( ck_pltran, SIGNAL( toggled( bool ) ),
               this,      SLOT( pranCheck( bool ) ) );
   }

   else
   {  // Disconnect slots for plot boxes
      ck_pltres->disconnect();
      ck_plttin->disconnect();
      ck_pltrin->disconnect();
      ck_pltran->disconnect();
   }
}

void US_ResidPlotFem::closeEvent( QCloseEvent* event )
{
  qDebug() << "US_ResidPlotFem CLOSED -- ";
  
  emit on_close();
  event->accept();
}
