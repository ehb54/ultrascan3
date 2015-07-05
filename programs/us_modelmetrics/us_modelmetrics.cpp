//! \file us_modelmetrics.cpp

#include "us_modelmetrics.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \brief Main program for US_ModelMetrics. Loads translators and starts
//         the class US_ModelMetrics

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ModelMetrics w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// Constructor
US_ModelMetrics::US_ModelMetrics() : US_Widgets()
{
   dbg_level    = US_Settings::us_debug();

   mfilter = ""; //default model list filter used in loading the model
   
   setWindowTitle( tr( "Model Metrics Calculator" ) );
   setPalette( US_GuiSettings::frameColor() );

   QGridLayout* top = new QGridLayout( this );
   top->setSpacing         ( 2 );
   top->setContentsMargins ( 2, 2, 2, 2 );

   int row=0;

   QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
   connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
   top->addWidget( pb_investigator, row, 0 );

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );
   le_investigator->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Preferred );
   top->addWidget( le_investigator, row++, 1);

   disk_controls = new US_Disk_DB_Controls;
   top->addLayout( disk_controls, row++, 0, 1, 2 );

   QPushButton* pb_prefilter= us_pushbutton( tr( "Select Prefilter" ) );
   connect( pb_prefilter, SIGNAL( clicked() ), SLOT( select_prefilter() ) );
   top->addWidget( pb_prefilter, row, 0 );

   le_prefilter= us_lineedit( "", 1, true );
   le_prefilter->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Preferred );
   top->addWidget( le_prefilter, row++, 1 );

   QPushButton* pb_load_model = us_pushbutton( tr( "Load Model" ) );
   connect( pb_load_model, SIGNAL( clicked() ), SLOT( load_model() ) );
   top->addWidget( pb_load_model, row, 0 );

   le_model = us_lineedit( "", 1, true );
   top->addWidget( le_model, row++, 1 );

   QPushButton* pb_write = us_pushbutton( tr( "Write Report" ) );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );
   top->addWidget( pb_write, row, 0 );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );
   top->addWidget( pb_reset, row++, 1 );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );
   top->addWidget( pb_help, row, 0 );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );
   top->addWidget( pb_accept, row, 1 );

   this->setMinimumWidth(640);

   reset();
}

void US_ModelMetrics::select_prefilter( void )
{
   QString pfmsg;
   int nruns = 0;
   pfilts.clear();

   US_SelectRuns srdiag( disk_controls->db(), pfilts );
   connect( &srdiag, SIGNAL( dkdb_changed  ( bool ) ),
            this,    SLOT(   update_disk_db( bool ) ) );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns      = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 0 )
      pfmsg = tr( "(none chosen)" );

   else if ( nruns > 1 )
      pfmsg = tr( "RunID prefilter - %1 run(s)" ).arg( nruns );

   else
      pfmsg = tr( "RunID prefilter - 1 run: " ) +
              QString( pfilts[ 0 ] ).left( 8 ) + "...";

DbgLv(1) << "PreFilt: pfilts[0]" << ((nruns>0)?pfilts[0]:"(none)");
   le_prefilter->setText( pfmsg );
}

void US_ModelMetrics::load_model( void )
{
   US_Model        model;
   QString         mdesc;
   bool            loadDB = disk_controls->db();
   double          smin   = 1e+39;
   double          smax   = 1e-39;
   double          kmin   = 1e+39;
   double          kmax   = 1e-39;
   double          wmin   = 1e+39;
   double          wmax   = 1e-39;
   double          vmin   = 1e+39;
   double          vmax   = 1e-39;
   double          dmin   = 1e+39;
   double          dmax   = 1e-39;
   double          fmin   = 1e+39;
   double          fmax   = 1e-39;

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, model, mdesc, pfilts );
   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );
   QApplication::restoreOverrideCursor();

   QString         mfnam;
   QString         mname;
   QString         sep;
   QString         aiters;

   if ( dialog.exec() != QDialog::Accepted )
      return;

   sep       = mdesc.left( 1 );
   mname     = mdesc.section( sep, 1, 1);
   mfnam     = mdesc.section( sep, 2, 2 );
   aiters    = mdesc.section( sep, 6, 6 );

   if ( mfnam.isEmpty() )
   {  // From db:  make ID the "filename"
      mfnam     = "db ID " + mdesc.section( sep, 4, 4 );
   }

   else
   {  // From disk:  use base file name
      mfnam     = QFileInfo( mfnam ).baseName();
   }
   le_model->setText(mname);

   if ( model.components.size() < 1 )
   {
      return;
   }

   // insure all model coefficient properties are set
   if ( ! model.update_coefficients() )
   {
      DbgLv(0) << "*** Unable to recalculate coefficient values ***";
   }

   // parse model information from its description
   mdesc          = mdesc.section( sep, 1, 1 );
   method         = model.typeText();
   method         = mdesc.contains( "-CG" )
                    ? method.replace( "2DSA", "2DSA-CG" ) : method;
   run_name       = mdesc.section( ".",  0, -3 );
   QString asys   = mdesc.section( ".", -2, -2 );
   analysis_name  = asys.section( "_", 0, 1 ) + "_"
                + asys.section( "_", 3, 4 );

   if ( method == "Manual"  ||  method == "CUSTOMGRID" )
   {
      int jj         = mdesc.indexOf( ".model" );
      mdesc          = ( jj < 1 ) ? mdesc : mdesc.left( jj );
      run_name       = mdesc + ".0Z280";
      analysis_name  = "e00_a00_" + method + "_local";
   }

   monte_carlo  = model.monteCarlo;
   mc_iters     = monte_carlo ? aiters.toInt() : 1;
   editGUID     = model.editGUID;
/*
   sk_distro.clear();
   xy_distro.clear();

   resetSb();

   QString tstr = run_name+ "\n" + analysis_name + "\n (" + method + ")";
   data_plot->setTitle( tstr );

   // read in and set distribution s,c,k,d values
   if ( model.analysis != US_Model::COFS )
   {
      for ( int jj = 0; jj < model.components.size(); jj++ )
      {
         US_Model::calc_coefficients( model.components[ jj ] );

         sol_sk.s  = model.components[ jj ].s * 1.0e13;
         sol_sk.k  = model.components[ jj ].f_f0;
         sol_sk.c  = model.components[ jj ].signal_concentration;
         sol_sk.w  = model.components[ jj ].mw;
         sol_sk.v  = model.components[ jj ].vbar20;
         sol_sk.d  = model.components[ jj ].D;
         sol_sk.f  = model.components[ jj ].f;

         sol_xy    = sol_sk;
         sol_xy.s  = ( attr_x == ATTR_S ) ? sol_sk.s : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_K ) ? sol_sk.k : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_W ) ? sol_sk.w : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_V ) ? sol_sk.v : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_D ) ? sol_sk.d : sol_xy.s;
         sol_xy.s  = ( attr_x == ATTR_F ) ? sol_sk.f : sol_xy.s;
         sol_xy.k  = ( attr_y == ATTR_S ) ? sol_sk.s : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_K ) ? sol_sk.k : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_W ) ? sol_sk.w : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_V ) ? sol_sk.v : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_D ) ? sol_sk.d : sol_xy.k;
         sol_xy.k  = ( attr_y == ATTR_F ) ? sol_sk.f : sol_xy.k;
         sol_xy.si = sol_sk.s;
         sol_xy.ki = sol_sk.k;

         smin      = qMin( smin, sol_sk.s );
         smax      = qMax( smax, sol_sk.s );
         kmin      = qMin( kmin, sol_sk.k );
         kmax      = qMax( kmax, sol_sk.k );
         wmin      = qMin( wmin, sol_sk.w );
         wmax      = qMax( wmax, sol_sk.w );
         vmin      = qMin( vmin, sol_sk.v );
         vmax      = qMax( vmax, sol_sk.v );
         dmin      = qMin( dmin, sol_sk.d );
         dmax      = qMax( dmax, sol_sk.d );
         fmin      = qMin( fmin, sol_sk.f );
         fmax      = qMax( fmax, sol_sk.f );
//DbgLv(2) << "Solute jj s w k c d" << jj << sol_s.s << sol_w.s << sol_s.k
//   << sol_s.c << sol_s.d << " vb" << model.components[jj].vbar20;

         sk_distro << sol_sk;
         xy_distro << sol_xy;
      }

      // sort and reduce distributions
      sdistro   = &xy_distro;
      psdsiz    = sdistro->size();
      sort_distro( sk_distro, false );
      sort_distro( xy_distro, true  );
DbgLv(1) << "Solute psdsiz sdsiz xdsiz" << psdsiz << sk_distro.size()
 << xy_distro.size();
for ( int jj=0;jj<sk_distro.size();jj++ ) {
 DbgLv(2) << " jj" << jj << " s k" << sk_distro[jj].s << sk_distro[jj].k
    << " w v" << sk_distro[jj].w << sk_distro[jj].v; }
   }

   // Determine which attribute is fixed
   if (      equivalent( vmin, vmax, 0.001 ) )
      attr_z    = ATTR_V;
   else if ( equivalent( kmin, kmax, 0.001 ) )
      attr_z    = ATTR_K;
   else if ( equivalent( smin, smax, 0.001 ) )
      attr_z    = ATTR_S;
   else if ( equivalent( wmin, wmax, 0.001 ) )
      attr_z    = ATTR_W;
   else if ( equivalent( dmin, dmax, 0.001 ) )
      attr_z    = ATTR_D;
   else if ( equivalent( fmin, fmax, 0.001 ) )
      attr_z    = ATTR_F;

   if ( attr_x != ATTR_V  &&  attr_y != ATTR_V  &&  attr_z != ATTR_V )
   {  // No attribute is vbar, so use vbar as the default X or Y
      int attrv = ATTR_V;

      if ( attr_y == attr_z )
      {
         rb_y_vbar->setChecked( true  );
         select_y_axis( attrv );
      }
      else
      {
         rb_x_vbar->setChecked( true  );
         select_x_axis( attrv );
      }
   }

   QStringList attrs;
   attrs << "s" << "f/f0" << "MW" << "vbar" << "D" << "f";
   QString s_attr = attrs[ attr_z ];

   if ( auto_lim )
   {
      set_limits();

      ct_plymin->setEnabled( false );
      ct_plymax->setEnabled( false );
      ct_plxmin->setEnabled( false );
      ct_plxmax->setEnabled( false );
   }
   else
   {
      plxmin    = ct_plxmin->value();
      plxmax    = ct_plxmax->value();
      plymin    = ct_plymin->value();
      plymax    = ct_plymax->value();
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plxmin, plxmax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plymin, plymax );

   pb_resetsb->setEnabled( true );

   nisols       = sdistro->size();
   s_attr       = "\"" + s_attr + "\".";
   dfilname     = "(" + mfnam + ") " + mdesc;
   stdfline     = "  " + dfilname;
   stfxline     = tr( "The components fixed attribute is " ) + s_attr;
   stnpline     = tr( "The number of distribution points is %1" )
                  .arg( nisols );
   if ( nisols != psdsiz )
      stnpline    += tr( "\n  (reduced from %1)" ).arg( psdsiz );

   te_status->setText( stcmline + "\n" + stdiline + "\n"
         + stdfline + "\n" + stfxline + "\n" + stnpline + "." );

   replot_data();

   soludata->setDistro( sdistro, attr_x, attr_y, attr_z );

   nibuks       = 0;
   wxbuck       = ( plxmax - plxmin ) / 20.0;
   hybuck       = ( plymax - plymin ) / 20.0;
   ct_wxbuck->setValue( wxbuck );
   ct_hybuck->setValue( hybuck );
   ct_nisols->setValue( double( nisols ) );
   ct_wxbuck->setEnabled(  true );
   ct_hybuck->setEnabled(  true );
   pb_refresh->setEnabled( true );
   pb_mandrsb->setEnabled( plot_dim != 1 );
   */
}

// Select DB investigator
void US_ModelMetrics::sel_investigator( void )
{
   int investigator = US_Settings::us_inv_ID();

   US_Investigator* dialog = new US_Investigator( true, investigator );
   dialog->exec();

   investigator = US_Settings::us_inv_ID();

   QString inv_text = QString::number( investigator ) + ": "
                      +  US_Settings::us_inv_name();

   le_investigator->setText( inv_text );
}

// Reset parameters to their defaults
void US_ModelMetrics::reset( void )
{
   le_model     ->setText( "" );
}

// Select DB investigator// Private slot to update disk/db control with dialog changes it
void US_ModelMetrics::update_disk_db( bool isDB )
{
   if ( isDB )
      disk_controls->set_db();
   else
      disk_controls->set_disk();
}

void US_ModelMetrics::write( void )
{
   QString str, str2;
   te = new US_Editor( US_Editor::LOAD, false, "results/*.rpt*", 0, 0 );
   te->e->setFontFamily("Arial");
   te->e->setFontPointSize( 13 );
   te->e->append("UltraScan Model Metrics Report:\n");
   te->e->setFontPointSize( 11 );
//      te->e->append("Peak " + str.setNum( i+1 ) + " (" + dpoint[i].name +
//      " from experiment \"" + dpoint[i].dataset + "\"):" );
   te->setMinimumHeight( 400 );
   te->setMinimumWidth( 600 );
   te->show();
}
