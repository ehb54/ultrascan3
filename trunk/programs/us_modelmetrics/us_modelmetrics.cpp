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

   QLabel* lbl_experiment = us_label( tr( "Edit + Model #: " ), -1 );
   top->addWidget( lbl_experiment, row, 0 );

   le_experiment= us_lineedit( "", 1, true );
   top->addWidget( le_experiment, row++, 1 );

   QLabel* lbl_param = us_label( tr( "Select Parameter: " ), -1 );
   top->addWidget( lbl_param, row, 0 );

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

   QHBoxLayout* hbl;
   hbl = new QHBoxLayout();
   hbl->addLayout(gl_s);
   hbl->addLayout(gl_d);
   hbl->addLayout(gl_m);
   hbl->addLayout(gl_k);
   hbl->addLayout(gl_f);
   hbl->addLayout(gl_v);
   top->addLayout(hbl, row++, 1);

   lbl_dval1 = us_label( tr( "D10 value: " ), -1 );
   lbl_dval2 = us_label( tr( "D50 value: " ), -1 );
   lbl_dval3 = us_label( tr( "D90 value: " ), -1 );
   lbl_span  = us_label( tr( "Span ( D90 - D10 ) / D50: " ), -1 );

   le_dval1= us_lineedit( "", 1, true );
   le_dval2= us_lineedit( "", 1, true );
   le_dval3= us_lineedit( "", 1, true );
   le_span = us_lineedit( "", 1, true );

   ct_dval1 = us_counter( 3, 0.0, 100.0, 10.0 );
   ct_dval1->setStep( 1 );
   ct_dval1->setEnabled( false );
   connect (ct_dval1, SIGNAL(valueChanged(double)), this, SLOT(set_dval1(double)));

   ct_dval2 = us_counter( 3, 0.0, 100.0, 50.0 );
   ct_dval2->setStep( 1 );
   ct_dval2->setEnabled( false );
   connect (ct_dval2, SIGNAL(valueChanged(double)), this, SLOT(set_dval2(double)));

   ct_dval3 = us_counter( 3, 0.0, 100.0, 90.0 );
   ct_dval3->setStep( 1 );
   ct_dval3->setEnabled( false );
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

   QGridLayout* gl1;
   gl1 = new QGridLayout();

   gl1->addWidget(lbl_dval1,    0, 0, 1, 1);
   gl1->addWidget(ct_dval1,     0, 1, 1, 1);
   gl1->addWidget(le_dval1,     0, 2, 1, 1);
   gl1->addWidget(lbl_dval2,    0, 3, 1, 1);
   gl1->addWidget(ct_dval2,     0, 4, 1, 1);
   gl1->addWidget(le_dval2,     0, 5, 1, 1);
   gl1->addWidget(lbl_dval3,    1, 0, 1, 1);
   gl1->addWidget(ct_dval3,     1, 1, 1, 1);
   gl1->addWidget(le_dval3,     1, 2, 1, 1);
   gl1->addWidget(lbl_span,     1, 3, 1, 2);
   gl1->addWidget(le_span,      1, 5, 1, 1);
   gl1->addWidget(lbl_minimum,  2, 0, 1, 1);
   gl1->addWidget(le_minimum,   2, 1, 1, 1);
   gl1->addWidget(lbl_mode,     2, 2, 1, 1);
   gl1->addWidget(le_mode,      2, 3, 1, 1);
   gl1->addWidget(lbl_skew,     2, 4, 1, 1);
   gl1->addWidget(le_skew,      2, 5, 1, 1);
   gl1->addWidget(lbl_maximum,  3, 0, 1, 1);
   gl1->addWidget(le_maximum,   3, 1, 1, 1);
   gl1->addWidget(lbl_median,   3, 2, 1, 1);
   gl1->addWidget(le_median,    3, 3, 1, 1);
   gl1->addWidget(lbl_kurtosis, 3, 4, 1, 1);
   gl1->addWidget(le_kurtosis,  3, 5, 1, 1);
   gl1->addWidget(lbl_mean,     4, 0, 1, 1);
   gl1->addWidget(le_mean,      4, 1, 1, 1);
   gl1->addWidget(pb_write,     4, 2, 1, 1);
   gl1->addWidget(pb_reset,     4, 3, 1, 1);
   gl1->addWidget(pb_help,      4, 4, 1, 1);
   gl1->addWidget(pb_accept,    4, 5, 1, 1);

   top->addLayout(gl1, row, 0, 2, 2);

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

         DbgLv(2) << "Solute jj s w k c d" << jj << sol_sk.s << sol_sk.w << sol_sk.k
                  << sol_sk.c << sol_sk.d << " vb" << model.components[jj].vbar20;

         sk_distro << sol_sk;
      }

      DbgLv(2) << "sk_distro.size() before reduction: " << sk_distro.size();
      
      // sort and reduce distribution
      sort_distro( sk_distro, true );

      DbgLv(2) << "sk_distro.size() after reduction: " << sk_distro.size();
      for ( int jj=0;jj<sk_distro.size();jj++ ) 
      {
         DbgLv(2) << " jj" << jj << " s k" << sk_distro[jj].s << sk_distro[jj].k
                  << " w v" << sk_distro[jj].w << sk_distro[jj].v; 
      }
   }

   // Determine which attribute is fixed
   if      ( equivalent( vmin, vmax, 0.001 )) fixed = HPv;
   else if ( equivalent( kmin, kmax, 0.001 )) fixed = HPk;
   else if ( equivalent( smin, smax, 0.001 )) fixed = HPs;
   else if ( equivalent( wmin, wmax, 0.001 )) fixed = HPm;
   else if ( equivalent( dmin, dmax, 0.001 )) fixed = HPd;
   else if ( equivalent( fmin, fmax, 0.001 )) fixed = HPf;

   DbgLv(2) << "dmin, dmax: " << dmin << dmax;
   DbgLv(2) << "kmin, kmax: " << kmin << kmax;
   DbgLv(2) << "wmin, wmax: " << wmin << wmax;
   DbgLv(2) << "vmin, vmax: " << vmin << vmax;
   DbgLv(2) << "fmin, fmax: " << fmin << fmax;

   if (HPs != fixed) rb_s->setEnabled( true );
   if (HPd != fixed) rb_d->setEnabled( true );
   if (HPk != fixed) rb_k->setEnabled( true );
   if (HPf != fixed) rb_f->setEnabled( true );
   if (HPm != fixed) rb_m->setEnabled( true );
   if (HPv != fixed) rb_v->setEnabled( true );
   DbgLv(0) << "Total concentration, array size original: " << total_conc << sk_distro.size();
   calc();

   ct_dval1->setEnabled( true );
   ct_dval2->setEnabled( true );
   ct_dval3->setEnabled( true );
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
DbgLv(1) << "DUP: sval svpr jj" << sol1.s << sol2.s << jj;
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
   lbl_span->setText( "Span ( D" + str3.setNum((int) dval3) + " - D" 
		                 + str1.setNum((int) dval1) + " ) / D" 
				 + str2.setNum((int) dval2) + ": ");
   HydroParm val1;
   HydroParm val2;
   QList <HydroParm> temp_list;
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
   }
   else if (calc_val == HPd)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].d;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
   }
   else if (calc_val == HPm)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].w;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
   }
   else if (calc_val == HPk)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].k;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
   }
   else if (calc_val == HPf)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].f;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
   }
   else if (calc_val == HPv)
   {
      for (i=0; i<sk_distro.size(); i++)
      {
         val1.parm = sk_distro[i].v;
         val1.conc = sk_distro[i].c;
         temp_list.append(val1);
      }
   }
   qSort(temp_list.begin(), temp_list.end()); // sort the list so reduction works.

   for (i=0; i<temp_list.size(); i++)
   {
      DbgLv(0) << temp_list[i].parm;
   }

   double tc = 0.0;

   val1.parm = temp_list[0].parm;
   val1.conc = temp_list[0].conc;
   hp_distro.append(val1);
   tc += val1.conc;

   DbgLv(0) << "after first value: " << tc;
   for (i=1; i<temp_list.size(); i++)
   {
      val2.parm = temp_list[i].parm;
      val2.conc = temp_list[i].conc;
      if (val1.parm != val2.parm) //not a duplicate, so append
      {
         hp_distro.append(val2);
	 tc += val2.conc;
      }
      else //a duplicate, so add concentrations
      {
         val2.conc = val1.conc + val2.conc;
	 tc += val2.conc - val1.conc;
         hp_distro.replace(hp_distro.size() - 1,  val2);
      }
      val1 = val2;
   }
   int points = hp_distro.size();
   DbgLv(0) << "Distro size after reduction, total conc2: " << points << tc;
   double sum1=0.0, sum2, sum3, mode, median, skew, kurtosis;
   QString str;
   i=0;
   while (sum1 < tc * dval1/100.0 && i < points)
   {
      sum1 += hp_distro[i].conc;
      i++; 
   }
   xval1 = hp_distro[i-1].parm;
   sum2 = sum1;
   while (sum2 < tc * dval2/100.0 && i < points)
   {
      sum2 += hp_distro[i].conc;
      i++; 
   }
   xval2 = hp_distro[i-1].parm;
   sum3 = sum2;
   while (sum3 < tc * dval3/100.0 && i < points)
   {
      sum3 += hp_distro[i].conc;
      i++; 
   }
   xval3 = hp_distro[i-1].parm;
   median = 0.0;
   i=0;
   while (median < tc * 0.5 && i < points)
   {
      median += hp_distro[i].conc;
      i++; 
   }
   median = hp_distro[i-1].parm;

   sum1 = 0.0;
   double mode_conc = 0.0;
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
   sum1 /= tc;
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
   DbgLv(0) << xval1 << xval2 << xval3 << dval1 << dval2 << dval3;
}

void US_ModelMetrics::set_dval1( double val )
{
   dval1 = val;
   calc();
}
void US_ModelMetrics::set_dval2( double val )
{
   dval2 = val;
   calc();
}
void US_ModelMetrics::set_dval3( double val )
{
   dval3 = val;
   calc();
}


