//! \file us_modelmetrics.cpp

#include "us_modelmetrics.h"
#include <iostream>
using namespace std;

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

// qSort LessThan method for S_Solute sort
bool distro_lessthan( const S_Solute &solu1, const S_Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && k1<k2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.k < solu2.k ) );
}

const double epsilon = 0.0005;   // equivalence magnitude ratio radius

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

   if ( US_Settings::us_inv_level() < 1 )
      pb_investigator->setEnabled( false );

   int id = US_Settings::us_inv_ID();
   QString number  = ( id > 0 ) ?
      QString::number( US_Settings::us_inv_ID() ) + ": "
      : "";
   le_investigator = us_lineedit( number + US_Settings::us_inv_name(), 1, true );

   disk_controls = new US_Disk_DB_Controls;

   QPushButton* pb_prefilter= us_pushbutton( tr( "Select Prefilter" ) );
   connect( pb_prefilter, SIGNAL( clicked() ), SLOT( select_prefilter() ) );

   le_prefilter= us_lineedit( "", 1, true );

   QPushButton* pb_load_model = us_pushbutton( tr( "Load Model" ) );
   connect( pb_load_model, SIGNAL( clicked() ), SLOT( load_model() ) );

   le_model = us_lineedit( "", 1, true );

   QLabel* lbl_experiment = us_label( tr( "Edit + Model #: " ), -1 );

   le_experiment= us_lineedit( "", 1, true );

   QLabel* lbl_param = us_label( tr( "Select Parameter: " ), -1 );
   QLabel* lbl_sigma = us_label( tr( "Sigma: " ), -1 );

   bg_hp = new QButtonGroup( this );
   QGridLayout* gl_s = us_radiobutton( tr( "s"    ), rb_s, true  );
   QGridLayout* gl_k = us_radiobutton( tr( "ff0"  ), rb_k, false );
   QGridLayout* gl_m = us_radiobutton( tr( "mw"   ), rb_m, false );
   QGridLayout* gl_v = us_radiobutton( tr( "vbar" ), rb_v, false );
   QGridLayout* gl_d = us_radiobutton( tr( "D"    ), rb_d, false );
   QGridLayout* gl_f = us_radiobutton( tr( "f"    ), rb_f, false );

   bg_hp->addButton( rb_s, HPs );
   bg_hp->addButton( rb_k, HPk );
   bg_hp->addButton( rb_m, HPm );
   bg_hp->addButton( rb_v, HPv );
   bg_hp->addButton( rb_d, HPd );
   bg_hp->addButton( rb_f, HPf );

   rb_s->setChecked( true  );
   rb_s->setToolTip( tr( "Select Sedimentation Coefficient" ));
   rb_k->setToolTip( tr( "Select Frictional Ratio"          ));
   rb_m->setToolTip( tr( "Select Molecular Weight"          ));
   rb_v->setToolTip( tr( "Select Partial Specific Volume"   ));
   rb_d->setToolTip( tr( "Select Diffusion Coefficient"     ));
   rb_f->setToolTip( tr( "Select Frictional Coefficient"    ));
   connect( bg_hp, SIGNAL( buttonReleased( int )),
            this,  SLOT  ( select_hp     ( int )));

   lbl_dval1 = us_label( tr( "D10 value: " ), -1 );
   lbl_dval2 = us_label( tr( "D50 value: " ), -1 );
   lbl_dval3 = us_label( tr( "D90 value: " ), -1 );
   lbl_span  = us_label( tr( "(D90-D10)/D50: " ), -1 );

   le_dval1= us_lineedit( "", 1, true );
   le_dval2= us_lineedit( "", 1, true );
   le_dval3= us_lineedit( "", 1, true );
   le_span = us_lineedit( "", 1, true );

   ct_sigma= us_counter( 3, 0.0, 1.0, 1.0 );
   ct_sigma->setStep( 0.001 );
   ct_sigma->setValue( 0.0 );
   ct_sigma->setEnabled( false );
   connect (ct_sigma, SIGNAL(valueChanged(double)), this, SLOT(set_sigma(double)));

   ct_dval1 = us_counter( 2, 0.0, 100.0, 10.0 );
   ct_dval1->setStep( 1 );
   ct_dval1->setEnabled( false );
   ct_dval1->setFixedSize(130, 25);
   connect (ct_dval1, SIGNAL(valueChanged(double)), this, SLOT(set_dval1(double)));

   ct_dval2 = us_counter( 2, 0.0, 100.0, 50.0 );
   ct_dval2->setStep( 1 );
   ct_dval2->setEnabled( false );
   ct_dval2->setFixedSize(130, 25);
   connect (ct_dval2, SIGNAL(valueChanged(double)), this, SLOT(set_dval2(double)));

   ct_dval3 = us_counter( 2, 0.0, 100.0, 90.0 );
   ct_dval3->setStep( 1 );
   ct_dval3->setEnabled( false );
   ct_dval3->setFixedSize(130, 25);
   connect (ct_dval3, SIGNAL(valueChanged(double)), this, SLOT(set_dval3(double)));

   lbl_minimum  = us_label( tr( "Minimum: " ), -1 );
   lbl_maximum  = us_label( tr( "Maximum: " ), -1 );
   lbl_mean     = us_label( tr( "Mean: "    ), -1 );
   lbl_mode     = us_label( tr( "Mode: "    ), -1 );
   lbl_median   = us_label( tr( "Median: "  ), -1 );
   lbl_kurtosis = us_label( tr( "Kurtosis: "  ), -1 );
   lbl_skew     = us_label( tr( "Skew: "  ), -1 );

   le_minimum  = us_lineedit( "", 1, true );
   le_maximum  = us_lineedit( "", 1, true );
   le_mean     = us_lineedit( "", 1, true );
   le_mode     = us_lineedit( "", 1, true );
   le_median   = us_lineedit( "", 1, true );
   le_kurtosis = us_lineedit( "", 1, true );
   le_skew     = us_lineedit( "", 1, true );

   QPushButton* pb_write = us_pushbutton( tr( "Show Report" ) );
   connect( pb_write, SIGNAL( clicked() ), SLOT( write() ) );

   QPushButton* pb_reset = us_pushbutton( tr( "Reset" ) );
   connect( pb_reset, SIGNAL( clicked() ), SLOT( reset() ) );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   QPushButton* pb_accept = us_pushbutton( tr( "Close" ) );
   connect( pb_accept, SIGNAL( clicked() ), SLOT( close() ) );

   QBoxLayout* plot = new US_Plot( data_plot,
   tr( "Distribution Data" ), "Parameter Value", "Relative Concentration" );

   data_plot->setAutoDelete( true );
   data_plot->setMinimumSize( 500, 100 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->enableAxis( QwtPlot::yRight,  false );
   data_plot->setAxisScale( QwtPlot::xBottom, 1.0, 40.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,   1.0,  4.0 );

   data_plot->setCanvasBackground( Qt::black );

   QGridLayout* gl0;
   gl0 = new QGridLayout();

   gl0->addLayout(gl_s, 0, 1, 1, 1);
   gl0->addLayout(gl_d, 0, 2, 1, 1);
   gl0->addLayout(gl_m, 0, 3, 1, 1);
   gl0->addLayout(gl_k, 1, 1, 1, 1);
   gl0->addLayout(gl_f, 1, 2, 1, 1);
   gl0->addLayout(gl_v, 1, 3, 1, 1);

   QGridLayout* gl1;
   gl1 = new QGridLayout();
   gl1->setColumnStretch(0, 0);
   gl1->setColumnStretch(1, 0);
   gl1->setColumnStretch(2, 1);

   gl1->addWidget(lbl_dval1, 0, 0, 1, 1);
   gl1->addWidget(ct_dval1,  0, 1, 1, 1);
   gl1->addWidget(le_dval1,  0, 2, 1, 1);
   gl1->addWidget(lbl_dval2, 1, 0, 1, 1);
   gl1->addWidget(ct_dval2,  1, 1, 1, 1);
   gl1->addWidget(le_dval2,  1, 2, 1, 1);
   gl1->addWidget(lbl_dval3, 2, 0, 1, 1);
   gl1->addWidget(ct_dval3,  2, 1, 1, 1);
   gl1->addWidget(le_dval3,  2, 2, 1, 1);

   QGridLayout* gl2;
   gl2 = new QGridLayout();
   gl2->setColumnStretch(0, 0);
   gl2->setColumnStretch(1, 2);
   gl2->setColumnStretch(2, 8);

   gl2->addWidget(pb_investigator, row, 0, 1, 1);
   gl2->addWidget(le_investigator, row, 1, 1, 1);
   gl2->addLayout(plot,            row, 2, 22, 1);
   row++;
   gl2->addLayout(disk_controls,   row, 0, 1, 2);
   row++;
   gl2->addWidget(pb_prefilter,    row, 0, 1, 1);
   gl2->addWidget(le_prefilter,    row, 1, 1, 1);
   row++;
   gl2->addWidget(pb_load_model,   row, 0, 1, 1);
   gl2->addWidget(le_model,        row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_experiment,  row, 0, 1, 1);
   gl2->addWidget(le_experiment,   row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_param,       row, 0, 2, 1);
   gl2->addLayout(gl0,             row, 1, 2, 1);
   row += 2;
   gl2->addWidget(lbl_sigma,       row, 0, 1, 1);
   gl2->addWidget(ct_sigma,        row, 1, 1, 1);
   row++;
   gl2->addLayout(gl1,             row, 0, 3, 2);
   row += 3;
   gl2->addWidget(lbl_span,        row, 0, 1, 1);
   gl2->addWidget(le_span,         row, 1, 1, 1);
   row++;                          
   gl2->addWidget(lbl_minimum,     row, 0, 1, 1);
   gl2->addWidget(le_minimum,      row, 1, 1, 1);
   row++;                          
   gl2->addWidget(lbl_maximum,     row, 0, 1, 1);
   gl2->addWidget(le_maximum,      row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_mean,        row, 0, 1, 1);
   gl2->addWidget(le_mean,         row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_mode,        row, 0, 1, 1);
   gl2->addWidget(le_mode,         row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_median,      row, 0, 1, 1);
   gl2->addWidget(le_median,       row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_skew,        row, 0, 1, 1);
   gl2->addWidget(le_skew,         row, 1, 1, 1);
   row++;
   gl2->addWidget(lbl_kurtosis,    row, 0, 1, 1);
   gl2->addWidget(le_kurtosis,     row, 1, 1, 1);
   row++;                          
   gl2->addWidget(pb_write,        row, 0, 1, 1);
   gl2->addWidget(pb_reset,        row, 1, 1, 1);
   row++;
   gl2->addWidget(pb_help,         row, 0, 1, 1);
   gl2->addWidget(pb_accept,       row, 1, 1, 1);

   top->addLayout(gl2, row, 0, 2, 2);
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

   else
      pfmsg = QString( pfilts[ 0 ] );

// DbgLv(1) << "PreFilt: pfilts[0]" << ((nruns>0)?pfilts[0]:"(none)");
   le_prefilter->setText( pfmsg );
}

void US_ModelMetrics::load_model( void )
{
   US_Model        model;
   QString         mdesc;
   S_Solute        sol_sk;
   bool            loadDB = disk_controls->db();

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

   sk_distro.clear();
   total_conc = 0.0;

   le_model->setText(run_name + " (" + method + ")");
   le_experiment->setText( analysis_name );

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
         total_conc += sol_sk.c;

         cmin      = qMin( cmin, sol_sk.c );
         cmax      = qMax( cmax, sol_sk.c );
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

//         DbgLv(2) << "Solute jj s w k c d" << jj << sol_sk.s << sol_sk.w << sol_sk.k
//                  << sol_sk.c << sol_sk.d << " vb" << model.components[jj].vbar20;

         sk_distro << sol_sk;
      }

//      DbgLv(2) << "sk_distro.size() before reduction: " << sk_distro.size();
      
      // sort and reduce distribution
      sort_distro( sk_distro, true );

//      DbgLv(2) << "sk_distro.size() after reduction: " << sk_distro.size();
//      for ( int jj=0;jj<sk_distro.size();jj++ ) 
//      {
//         DbgLv(2) << " jj" << jj << " s k" << sk_distro[jj].s << sk_distro[jj].k
//                  << " w v" << sk_distro[jj].w << sk_distro[jj].v; 
//      }
   }

   // Determine which attribute is fixed
   if      ( equivalent( vmin, vmax, 0.001 )) fixed = HPv;
   else if ( equivalent( kmin, kmax, 0.001 )) fixed = HPk;
   else if ( equivalent( smin, smax, 0.001 )) fixed = HPs;
   else if ( equivalent( wmin, wmax, 0.001 )) fixed = HPm;
   else if ( equivalent( dmin, dmax, 0.001 )) fixed = HPd;
   else if ( equivalent( fmin, fmax, 0.001 )) fixed = HPf;

   /*
   DbgLv(2) << "dmin, dmax: " << dmin << dmax;
   DbgLv(2) << "kmin, kmax: " << kmin << kmax;
   DbgLv(2) << "wmin, wmax: " << wmin << wmax;
   DbgLv(2) << "vmin, vmax: " << vmin << vmax;
   DbgLv(2) << "fmin, fmax: " << fmin << fmax;
   */

   if (HPs != fixed) rb_s->setEnabled( true );
   if (HPd != fixed) rb_d->setEnabled( true );
   if (HPk != fixed) rb_k->setEnabled( true );
   if (HPf != fixed) rb_f->setEnabled( true );
   if (HPm != fixed) rb_m->setEnabled( true );
   if (HPv != fixed) rb_v->setEnabled( true );
//   DbgLv(0) << "Total concentration, array size original: " << total_conc << sk_distro.size();
   calc();

   ct_dval1->setEnabled( true );
   ct_dval2->setEnabled( true );
   ct_dval3->setEnabled( true );
   ct_sigma->setEnabled( true );
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
   tc     = 0.0;
   sigma  = 0.0;
   cmin   = 1e+39;
   cmax   = 1e-39;
   smin   = 1e+39;
   smax   = 1e-39;
   kmin   = 1e+39;
   kmax   = 1e-39;
   wmin   = 1e+39;
   wmax   = 1e-39;
   vmin   = 1e+39;
   vmax   = 1e-39;
   dmin   = 1e+39;
   dmax   = 1e-39;
   fmin   = 1e+39;
   fmax   = 1e-39;
   dval1  = 10.0;
   dval2  = 50.0;
   dval3  = 90.0;
   ct_dval1->setEnabled( false );
   ct_dval2->setEnabled( false );
   ct_dval3->setEnabled( false );
   rb_s->setChecked( true  );
   rb_s->setEnabled( false );
   rb_d->setEnabled( false );
   rb_k->setEnabled( false );
   rb_f->setEnabled( false );
   rb_m->setEnabled( false );
   rb_v->setEnabled( false );
   calc_val = HPs; //calculate sedimentation distributions by default

   le_model->     setText("");
   le_experiment->setText("");
   le_dval1->     setText("");
   le_dval2->     setText("");
   le_dval3->     setText("");
   le_span->      setText("");
   le_minimum->   setText("");
   le_maximum->   setText("");
   le_mean->      setText("");
   le_mode->      setText("");
   le_median->    setText("");
   le_kurtosis->  setText("");
   le_skew->      setText("");
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
   te = new US_Editor( US_Editor::LOAD, true );
   te->setWindowTitle("UltraScan Model Statistics Report:");
   te->resize(700,600);
   te->e->setFont( QFont( US_GuiSettings::fontFamily(),
                          US_GuiSettings::fontSize() ) );
   te->e->append("Details:");
   te->show();
}

// Sort distribution solute list by s,k values and optionally reduce
void US_ModelMetrics::sort_distro( QList< S_Solute >& listsols,
      bool reduce )
{
   int sizi = listsols.size();

   if ( sizi < 2 )
      return;        // nothing need be done for 1-element list

   // sort distro solute list by s,k values

   qSort( listsols.begin(), listsols.end(), distro_lessthan );

   // check reduce flag

   if ( reduce )
   {     // skip any duplicates in sorted list
      S_Solute sol1;
      S_Solute sol2;
      QList< S_Solute > reduced;
      sol1     = listsols.at( 0 );
      reduced.append( sol1 );    // output first entry
      int kdup = 0;
      int jdup = 0;

      for ( int jj = 1; jj < sizi; jj++ )
      {     // loop to compare each entry to previous
          sol2    = listsols.at( jj );  // solute entry

          if ( sol1.s != sol2.s  ||  sol1.k != sol2.k )
          {   // not a duplicate, so output to temporary list
             reduced.append( sol2 );
             jdup    = 0;
          }

          else
          {   // duplicate:  sum c value
// DbgLv(1) << "DUP: sval svpr jj" << sol1.s << sol2.s << jj;
             kdup    = max( kdup, ++jdup );
             qreal f = (qreal)( jdup + 1 );
             sol2.c += sol1.c;   // sum c value
             sol2.s  = ( sol1.s * jdup + sol2.s ) / f;  // average s,k
             sol2.k  = ( sol1.k * jdup + sol2.k ) / f;
             reduced.replace( reduced.size() - 1, sol2 );
          }

          sol1    = sol2;        // save entry for next iteration
      }

      if ( reduced.size() < sizi )
      {   // if some reduction happened, replace list with reduced version
         //double sc = 1.0 / (double)( kdup + 1 );

         //for ( int ii = 0; ii < reduced.size(); ii++ )
         //   reduced[ ii ].c *= sc;

         listsols = reduced;
      }
   }
   return;
}

// Flag whether two values are effectively equal within a given epsilon
bool US_ModelMetrics::equivalent( double a, double b, double eps )
{
	   return ( qAbs( ( a - b ) / a ) <= eps );
}

void US_ModelMetrics::select_hp( int button )
{
   calc_val = button;
   calc();
}

void US_ModelMetrics::calc()
{
   int i;
   QString str1, str2, str3;
   lbl_dval1->setText( "D" + str1.setNum((int) dval1) + " value: ");
   lbl_dval2->setText( "D" + str2.setNum((int) dval2) + " value: ");
   lbl_dval3->setText( "D" + str3.setNum((int) dval3) + " value: ");
   lbl_span->setText( "(D" + str3.setNum((int) dval3) + "-D" 
		                 + str1.setNum((int) dval1) + ")/D" 
				 + str2.setNum((int) dval2) + ": ");
   HydroParm val1;
   HydroParm val2;
   QList <HydroParm> temp_list;
   orig_list.clear(); // contains saved copy of the original hp_distro list
   hp_distro.clear(); // contains the reduced array of desired parameter with 
                      // concentrations of identical parameter values added together
   temp_list.clear(); // contains the unreduced array of desired parameter
   if (calc_val == HPs)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].s;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = smin;
      xmax = smax;
   }
   else if (calc_val == HPd)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].d;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = dmin;
      xmax = dmax;
   }
   else if (calc_val == HPm)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].w;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = wmin;
      xmax = wmax;
   }
   else if (calc_val == HPk)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].k;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = kmin;
      xmax = kmax;
   }
   else if (calc_val == HPf)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].f;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = fmin;
      xmax = fmax;
   }
   else if (calc_val == HPv)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].v;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
      xmin = vmin;
      xmax = vmax;
   }
   qSort(temp_list.begin(), temp_list.end()); // sort the list so reduction works.
   tc = 0.0;
   val1.parm = temp_list[0].parm;
   val1.conc = temp_list[0].conc;
   orig_list.append(val1);
   tc += val1.conc;
   for (i=1; i<temp_list.size(); i++)
   {
      val2.parm = temp_list[i].parm;
      val2.conc = temp_list[i].conc;
      if (val1.parm != val2.parm) //not a duplicate, so append
      {
         orig_list.append(val2);
         tc += val2.conc;
      }
      else //a duplicate, so add concentrations
      {
         val2.conc = val1.conc + val2.conc;
         tc += val2.conc - val1.conc;
         orig_list.replace(orig_list.size() - 1,  val2);
      }
      val1 = val2;
   }
   hp_distro.clear();
   for (i=0; i<orig_list.size(); i++)
   {
      hp_distro.push_back(orig_list.at(i));
   }
   if (sigma > 0.0) 
   {
      update_sigma();
   }
   else
   {
      plot_data();
   }
}

void US_ModelMetrics::plot_data()
{
   int points = hp_distro.size(), i;
   double sum1=0.0, sum2, sum3, mode, median, skew, kurtosis;
   QString str;
   i=0;
   while (sum1 <= tc * dval1/100.0 && i < points)
   {
      sum1 += hp_distro[i].conc;
      i++; 
   }
   xval1 = hp_distro[i-1].parm;
   sum2 = sum1;
   while (sum2 <= tc * dval2/100.0 && i < points)
   {
      sum2 += hp_distro[i].conc;
      i++; 
   }
   xval2 = hp_distro[i-1].parm;
   sum3 = sum2;
   while (sum3 <= tc * dval3/100.0 && i < points)
   {
      sum3 += hp_distro[i].conc;
      i++; 
   }
   xval3  = hp_distro[i-1].parm;
   median = hp_distro[points-1].parm -
           (hp_distro[points-1].parm - hp_distro[0].parm)/2.0;

   sum1 = 0.0;
   double mode_conc=0.0, m2=0.0, m3=0.0, m4=0.0, tmp_val;
   mode = 0.0;
   skew=0.0;
   kurtosis=0.0;
   for (i=0; i<points; i++)
   {
      sum1 += hp_distro[i].parm * hp_distro[i].conc;
      if(hp_distro[i].conc > mode_conc)
      {
         mode      = hp_distro[i].parm;
         mode_conc = hp_distro[i].conc;
      }
   }
   sum1 /= tc;     // mean
   for (i=0; i<points; i++)
   {
      sum2 = hp_distro[i].parm - sum1;
      tmp_val = sum2 * sum2;
      m2 += tmp_val;
      m3 += tmp_val * sum2;
      m4 += tmp_val * tmp_val;
   }
   m2 /= (double) points;
   m3 /= (double) points;
   m4 /= (double) points;
   skew     = m3/pow(m2, 1.5);
   kurtosis = m4/pow(m2, 2.0) - 3.0;

   if (calc_val == HPs || calc_val == HPk)
   {
      le_dval1->setText(str.setNum(xval1, 'f', 5));
      le_dval2->setText(str.setNum(xval2, 'f', 5));
      le_dval3->setText(str.setNum(xval3, 'f', 5));
      le_minimum->setText(str.setNum(hp_distro[0].parm, 'f', 5));
      le_maximum->setText(str.setNum(hp_distro[points-1].parm, 'f', 5));
      le_mean->setText(str.setNum(sum1, 'f', 5));
      le_mode->setText(str.setNum(mode, 'f', 5));
      le_median->setText(str.setNum(median, 'f', 5));
   }
   else
   {
      le_dval1->setText(str.setNum(xval1, 'e', 6));
      le_dval2->setText(str.setNum(xval2, 'e', 6));
      le_dval3->setText(str.setNum(xval3, 'e', 6));
      le_minimum->setText(str.setNum(hp_distro[0].parm, 'e', 6));
      le_maximum->setText(str.setNum(hp_distro[points-1].parm, 'e', 6));
      le_mean->setText(str.setNum(sum1, 'e', 6));
      le_mode->setText(str.setNum(mode, 'e', 3));
      le_median->setText(str.setNum(median, 'e', 3));
   }
   le_span->setText(str.setNum(((xval3-xval1)/xval2), 'f', 6));
   le_skew->setText(str.setNum(skew, 'f', 6));
   le_kurtosis->setText(str.setNum(kurtosis, 'f', 6));
   QVector <double> xv;
   QVector <double> yv;
   xv.clear();
   yv.clear();
   double mxc=0.0;
   for (i=0; i<points; i++)
   {
      xv.push_back(hp_distro[i].parm);
      yv.push_back(hp_distro[i].conc);
      mxc = qMax(mxc, hp_distro[i].conc);
   }
   xx = xv.data();
   yy = yv.data();
   double x1[2], x2[2], x3[2], y1[2], y2[2], y3[2];
   x1[0] = xval1;
   x2[0] = xval2;
   x3[0] = xval3;
   x1[1] = xval1;
   x2[1] = xval2;
   x3[1] = xval3;
   y1[0] = 0;
   y2[0] = 0;
   y3[0] = 0;
   y1[1] = mxc;
   y2[1] = mxc;
   y3[1] = mxc;
   data_plot->clear();
   data_plot->setAxisAutoScale( QwtPlot::yLeft );
   data_plot->setAxisAutoScale( QwtPlot::xBottom );

   QFont sfont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 1 );
   QFontMetrics fmet( sfont );
   QwtLegend *legend = new QwtLegend;
   legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
   legend->setFont( sfont );
   data_plot->insertLegend( legend, QwtPlot::BottomLegend  );
   
   QwtPlotCurve* curve1;
   QwtPlotCurve* curve2;
   QwtPlotCurve* curve3;
   QwtPlotCurve* curve4;
   curve1  = us_curve( data_plot, tr( "Distribution" ) );
   curve2  = us_curve( data_plot, "D" + str.setNum((int) dval1));
   curve3  = us_curve( data_plot, "D" + str.setNum((int) dval2));
   curve4  = us_curve( data_plot, "D" + str.setNum((int) dval3));
   if (ct_sigma->value() > 0.0)
   {
      curve1->setPen( QPen( QBrush( Qt::yellow ), 2.0 ) );
      curve1->setStyle( QwtPlotCurve::Lines );
   }
   else
   {
      curve1->setStyle( QwtPlotCurve::Sticks );
      curve1->setPen( QPen( QBrush( Qt::yellow ), 4.0 ) );
   }
   curve2->setStyle( QwtPlotCurve::Sticks );
   curve3->setStyle( QwtPlotCurve::Sticks );
   curve4->setStyle( QwtPlotCurve::Sticks );
   curve2->setPen( QPen( QBrush( Qt::red), 2.0 ) );
   curve3->setPen( QPen( QBrush( Qt::cyan ), 2.0 ) );
   curve4->setPen( QPen( QBrush( Qt::green ), 2.0 ) );
   curve1->setData( xx, yy, points );
   curve2->setData( x1, y1, 2);
   curve3->setData( x2, y2, 2);
   curve4->setData( x3, y3, 2);
   data_plot->replot();
}

void US_ModelMetrics::update_sigma( void )
{
   QList <HydroParm> tmp_hplist;
   HydroParm hp;
   int points=500, i, j;
   double t_xmin, t_xmax, range, inc, tmp_tc=0.0, tmp_sigma;
   t_xmin = xmin;
   t_xmax = xmax;
   range = t_xmax - t_xmin;
   t_xmin -= range/5.0; // add 20% to the range
   if (t_xmin < 0.0) t_xmin = 0.0;
   t_xmax += range/5.0;
   range = t_xmax - t_xmin;
   tmp_sigma = sigma*range;
   inc = range/(double)(points-1);
   tmp_hplist.clear();
   for (i=0; i<points; i++)
   {
      hp.parm = t_xmin + i * inc;
      hp.conc = 0.0;
      tmp_hplist.push_back(hp);
   }
   double amp = 1.0/(tmp_sigma * pow(2.0 * M_PI, 0.5));
   double sigsqr = tmp_sigma * tmp_sigma;
   hp_distro.clear();
   for (i=0; i<orig_list.size(); i++)
   {
      hp_distro.push_back(orig_list.at(i));
   }

   for (j=0; j<hp_distro.size(); j++)
   {
      for (i=0; i<points; i++)
      {
         tmp_hplist[i].conc += hp_distro[j].conc * amp 
            * exp(-0.5 * pow(tmp_hplist[i].parm - hp_distro[j].parm, 2.0)
            / sigsqr);
      }
   }
   hp_distro.clear();
   for (i=0; i<points; i++)
   {
      hp_distro << tmp_hplist[i];
      tmp_tc += tmp_hplist[i].conc;
   }
   for (i=0; i<points; i++)
   {
      hp_distro[i].conc *= tc/tmp_tc; // rescale everything to original total concentration
   }
   plot_data();
}

void US_ModelMetrics::set_dval1( double val )
{
   dval1 = val;
   plot_data();
}

void US_ModelMetrics::set_dval2( double val )
{
   dval2 = val;
   plot_data();
}

void US_ModelMetrics::set_dval3( double val )
{
   dval3 = val;
   plot_data();
}


void US_ModelMetrics::set_sigma( double val )
{
   sigma = val;
   if (sigma == 0.0)
   {
     calc(); //need to update hp_distro dimensions
   }
   else
   {
      update_sigma();
   }
}

