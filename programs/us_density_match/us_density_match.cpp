//! \file us_density_match.cpp

#include <QApplication>

#include "us_density_match.h"
#include "us_remove_models.h"
#include "us_model_params.h"
#include "us_select_runs.h"
#include "us_model.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_solution_vals.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_gui_util.h"
#include "us_math2.h"
#include "us_matrix.h"
#include "us_sleep.h"
#include "us_passwd.h"
#include "us_report.h"
#include "us_constants.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()

// main program
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_Density_Match w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

// qSort LessThan method for S_Solute sort
bool distro_lessthan( const S_Solute &solu1, const S_Solute &solu2 )
{  // TRUE iff  (s1<s2) || (s1==s2 && d1<d2)
   return ( solu1.s < solu2.s ) ||
          ( ( solu1.s == solu2.s ) && ( solu1.d < solu2.d ) );
}

// US_Density_Match class constructor
US_Density_Match::US_Density_Match() : US_Widgets()
{
   // Set up the GUI

   setWindowTitle( tr( "Density Matching" ) );
   setPalette( US_GuiSettings::frameColor() );


   // Primary layouts
   QHBoxLayout* main = new QHBoxLayout( this );
   QVBoxLayout* left = new QVBoxLayout();
   QGridLayout* spec = new QGridLayout();
   main->setSpacing        ( 2 );
   main->setContentsMargins( 2, 2, 2, 2 );
   left->setSpacing        ( 0 );
   left->setContentsMargins( 0, 1, 0, 1 );
   spec->setSpacing        ( 1 );
   spec->setContentsMargins( 0, 0, 0, 0 );

   int s_row = 0;
   dbg_level = US_Settings::us_debug();
DbgLv(1) << "MD: main: AA";
//   clean_etc_dir();

   // Top banner
   QLabel* lb_info1      = us_banner( tr( "Model Selection Controls" ) );


   us_checkbox( tr( "Save Plot(s)"    ), ck_savepl,  false );
   us_checkbox( tr( "Local Save Only" ), ck_locsave, true  );

   te_distr_info = us_textedit();
   te_distr_info->setText    ( tr( "Run:  runID.triple (method)\n" )
            + tr( "    analysisID" ) );
   us_setReadOnly( te_distr_info, true );

   plot_x      = 0;

   QLabel* lb_x_axis   = us_label( tr( "Plot X:" ) );
           bg_x_axis   = new QButtonGroup( this );
   QGridLayout*  gl_x_mass = us_radiobutton( tr( "m.mass"   ), rb_x_mass, true  );
   QGridLayout*  gl_x_ff0  = us_radiobutton( tr( "ff0" ), rb_x_ff0,  false );
   QGridLayout*  gl_x_rh   = us_radiobutton( tr( "Rh"  ), rb_x_rh,   false );
   QGridLayout*  gl_x_vbar = us_radiobutton( tr( "vbar"), rb_x_vbar, false );
   QGridLayout*  gl_x_s    = us_radiobutton( tr( "s"   ), rb_x_s,    false );
   QGridLayout*  gl_x_d    = us_radiobutton( tr( "D"   ), rb_x_d,    false );
   bg_x_axis->addButton( rb_x_mass, ATTR_W );
   bg_x_axis->addButton( rb_x_ff0,  ATTR_K );
   bg_x_axis->addButton( rb_x_rh,   ATTR_R );
   bg_x_axis->addButton( rb_x_vbar, ATTR_V );
   bg_x_axis->addButton( rb_x_s,    ATTR_S );
   bg_x_axis->addButton( rb_x_d,    ATTR_D );
   rb_x_mass->setChecked( true  );
   rb_x_mass->setToolTip( tr( "Set X axis to Molar Mass"                ) );
   rb_x_ff0 ->setToolTip( tr( "Set X axis to Frictional Ratio"          ) );
   rb_x_rh  ->setToolTip( tr( "Set X axis to Hydrodynamic Radius"       ) );
   rb_x_vbar->setToolTip( tr( "Set X axis to Partial Specific Volume"   ) );
   rb_x_s   ->setToolTip( tr( "Set X axis to Sedimentation Coefficient" ) );
   rb_x_d   ->setToolTip( tr( "Set X axis to Diffusion Coefficient"     ) );
   connect( bg_x_axis,  SIGNAL( buttonReleased( int ) ),
            this,       SLOT  ( select_x_axis ( int ) ) );

   pb_refresh    = us_pushbutton( tr( "Refresh Plot" ) );
   pb_refresh->setEnabled(  false );
   connect( pb_refresh, SIGNAL( clicked() ),
            this,       SLOT( plot_data() ) );

   pb_reset      = us_pushbutton( tr( "Reset" ) );
   pb_reset->setEnabled( true );
   connect( pb_reset,   SIGNAL( clicked() ),
            this,       SLOT( reset() ) );

   dkdb_cntrls   = new US_Disk_DB_Controls(
         US_Settings::default_data_location() );
   connect( dkdb_cntrls, SIGNAL( changed( bool ) ),
            this,   SLOT( update_disk_db( bool ) ) );

   pb_prefilt    = us_pushbutton( tr( "Select PreFilter" ) );

   le_prefilt    = us_lineedit( tr( "" ), -1, true );
   connect( pb_prefilt, SIGNAL( clicked() ),
            this,       SLOT( select_prefilt() ) );

   pb_lddistr    = us_pushbutton( tr( "Load Distribution(s)" ) );
   pb_lddistr->setEnabled( true );
   connect( pb_lddistr, SIGNAL( clicked() ),
            this,       SLOT( load_distro() ) );

   pb_rmvdist    = us_pushbutton( tr( "Remove Distribution(s)" ) );
   pb_rmvdist->setEnabled( true );
   connect( pb_rmvdist, SIGNAL( clicked() ),
            this,       SLOT( remove_distro() ) );

   pb_mdlpars    = us_pushbutton( tr( "Set Model Parameters" ) );
   pb_mdlpars->setEnabled( true );
   connect( pb_mdlpars, SIGNAL( clicked() ),
            this,       SLOT( set_mparms()   ) );

   pb_save       = us_pushbutton( tr( "Save" ) );
   pb_save   ->setEnabled( true );
   connect( pb_save,    SIGNAL( clicked() ),
            this,       SLOT( save()         ) );

   pb_help       = us_pushbutton( tr( "Help" ) );
   pb_help->setEnabled( true );
   connect( pb_help,    SIGNAL( clicked() ),
            this,       SLOT( help() ) );

   pb_close      = us_pushbutton( tr( "Close" ) );
   pb_close->setEnabled( true );
   connect( pb_close,   SIGNAL( clicked() ),
            this,       SLOT( close() ) );

   // Order plot components on the left side
   spec->addWidget( lb_info1,      s_row++, 0, 1, 8 );
   spec->addLayout( dkdb_cntrls,   s_row++, 0, 1, 8 );
   spec->addWidget( le_prefilt,    s_row++, 0, 1, 8 );
   spec->addWidget( pb_prefilt,    s_row++, 0, 1, 4 );
   spec->addWidget( pb_lddistr,    s_row,   0, 1, 4 );
   spec->addWidget( pb_rmvdist,    s_row++, 4, 1, 4 );
   spec->addWidget( pb_refresh,    s_row,   0, 1, 4 );
   spec->addWidget( pb_reset,      s_row++, 4, 1, 4 );
   spec->addWidget( pb_mdlpars,    s_row,   0, 1, 4 );
   spec->addWidget( pb_save,       s_row++, 4, 1, 4 );
   spec->addWidget( lb_x_axis,     s_row,   0, 1, 2 );
   spec->addLayout( gl_x_mass,     s_row,   2, 1, 2 );
   spec->addLayout( gl_x_ff0,      s_row,   4, 1, 2 );
   spec->addLayout( gl_x_rh,       s_row++, 6, 1, 2 );
   spec->addLayout( gl_x_vbar,     s_row,   2, 1, 2 );
   spec->addLayout( gl_x_s,        s_row,   4, 1, 2 );
   spec->addLayout( gl_x_d,        s_row++, 6, 1, 2 );
   spec->addWidget( ck_savepl,     s_row,   0, 1, 4 );
   spec->addWidget( ck_locsave,    s_row++, 4, 1, 4 );
   spec->addWidget( te_distr_info, s_row,   0, 2, 8 ); s_row += 2;

   // Set up analysis controls
   QLabel* lb_analysis     = us_banner( tr( "Analysis Controls"      ) );
   QLabel* lb_boundPct     = us_label ( tr( "% of Boundary:"         ) );
   QLabel* lb_boundPos     = us_label ( tr( "Boundary Position (%):" ) );
   QLabel* lb_smoothing    = us_label ( tr( "Data Smoothing:"        ) );

   lb_division     = us_label( tr( "Divisions:" ) );
   lb_division->setAlignment( Qt::AlignVCenter | Qt::AlignLeft );

   ct_division     = us_counter( 2, 0, 100, 50 );
   ct_division->setSingleStep( 1 );
   ct_division->setEnabled( true );
   connect( ct_division, SIGNAL( valueChanged(  double ) ),
            this,         SLOT(  update_divis(  double ) ) );
   ct_boundaryPct  = us_counter( 2, 10, 100, 1 );
   ct_boundaryPos  = us_counter( 2, 0, 100, 1 );
   ct_smoothing    = us_counter( 2, 1, 100, 1 );

   ct_boundaryPct->setSingleStep(  1 );
   ct_boundaryPos->setSingleStep(  1 );
   ct_smoothing  ->setSingleStep(  1 );
   ct_boundaryPct->setValue( 90 );
   ct_boundaryPos->setValue(  5 );
   ct_smoothing  ->setValue(  1 );

   spec->addWidget( lb_analysis       , s_row++, 0, 1, 8 );
   spec->addWidget( lb_division       , s_row,   0, 1, 4 );
   spec->addWidget( ct_division       , s_row++, 4, 1, 4 );
   spec->addWidget( lb_boundPct       , s_row,   0, 1, 4 );
   spec->addWidget( ct_boundaryPct    , s_row++, 4, 1, 4 );
   spec->addWidget( lb_boundPos       , s_row,   0, 1, 4 );
   spec->addWidget( ct_boundaryPos    , s_row++, 4, 1, 4 );
   spec->addWidget( lb_smoothing      , s_row,   0, 1, 4 );
   spec->addWidget( ct_smoothing      , s_row++, 4, 1, 4 );


   spec->addWidget( pb_help,          ++s_row,   0, 1, 4 );
   spec->addWidget( pb_close,           s_row++, 4, 1, 4 );

   // Set up plot component window on right side
   xa_title    = anno_title( ATTR_S );
   ya_title    = anno_title( ATTR_F );
   QBoxLayout* plot = new US_Plot( data_plot, 
      tr( "Density Matching Data" ), xa_title, ya_title );

   data_plot->setMinimumSize( 600, 600 );

   data_plot->enableAxis( QwtPlot::xBottom, true );
   data_plot->enableAxis( QwtPlot::yLeft,   true );
   data_plot->setAxisScale( QwtPlot::xBottom, -200.0, 200.0 );
   data_plot->setAxisScale( QwtPlot::yLeft,      0.0,   1.0 );
   data_plot->setCanvasBackground( Qt::white );

   pick = new US_PlotPicker( data_plot );
   pick->setRubberBand( QwtPicker::RectRubberBand );

   // Put layouts together for overall layout
   left->addLayout( spec );
   left->addStretch();

   main->addLayout( left );
   main->addLayout( plot );
   main->setStretchFactor( left, 3 );
   main->setStretchFactor( plot, 5 );

   mfilter    = "";
   plt_zmin   = 1e+8;
   plt_zmax   = -1e+8;
   runsel     = true;
   latest     = true;

   // Set up variables and initial state of GUI
   reset();
}

void US_Density_Match::reset( void )
{
DbgLv(1) << "MD:   reset: AA";
   dataPlotClear( data_plot );
   data_plot->replot();
 
   need_save  = false;

   plot_x     = ATTR_V;
//   resolu     = 90.0;
//   ct_resolu->setRange( 1.0, 100.0 );
//   ct_resolu->setSingleStep( 1.0 );
//   ct_resolu->setValue( resolu );  

#if 0
   xreso      = 300.0;
   yreso      = 300.0;
   ct_xreso->setRange( 10.0, 1000.0 );
   ct_xreso->setSingleStep( 1.0 );
   ct_xreso->setValue( (double)xreso );
   ct_yreso->setRange( 10.0, 1000.0 );
   ct_yreso->setSingleStep( 1.0 );
   ct_yreso->setValue( (double)yreso );

   zfloor     = 100.0;
   ct_zfloor->setRange( 50.0, 150.0 );
   ct_zfloor->setSingleStep( 1.0 );
   ct_zfloor->setValue( (double)zfloor );

   auto_sxy   = true;
   ck_autosxy->setChecked( auto_sxy );
   auto_scz   = true;
   ck_autoscz->setChecked( auto_scz );
   cont_loop  = false;
   ck_conloop->setChecked( cont_loop );
   ck_savepl ->setChecked( false     );
   ck_locsave->setChecked( true      );

   plt_kmin   = 0.8;
   plt_kmax   = 4.2;
   ct_plt_kmin->setRange( 0.0, 50.0 );
   ct_plt_kmin->setSingleStep( 0.01 );
   ct_plt_kmin->setValue( plt_kmin );
   ct_plt_kmin->setEnabled( false );
   ct_plt_kmax->setRange( 1.0, 50.0 );
   ct_plt_kmax->setSingleStep( 0.01 );
   ct_plt_kmax->setValue( plt_kmax );
   ct_plt_kmax->setEnabled( false );

   plt_smin   = 1.0;
   plt_smax   = 10.0;
   ct_plt_smin->setRange( -10.0, 10000.0 );
   ct_plt_smin->setSingleStep( 0.01 );
   ct_plt_smin->setValue( plt_smin );
   ct_plt_smin->setEnabled( false );
   ct_plt_smax->setRange(   0.0, 10000.0 );
   ct_plt_smax->setSingleStep( 0.01 );
   ct_plt_smax->setValue( plt_smax );
   ct_plt_smax->setEnabled( false );

   curr_distr = 0;
   ct_curr_distr->setRange( 1.0, 1.0 );
   ct_curr_distr->setSingleStep( 1.0 );
   ct_curr_distr->setValue( curr_distr + 1 );
   ct_curr_distr->setEnabled( false );
#endif

   alldis.clear();
   pfilts.clear();
   pb_refresh->setEnabled( false );
   pb_rmvdist->setEnabled( false );
   le_prefilt->setText( tr( "(no prefilter)" ) );
}

void US_Density_Match::save( void )
{
}

// plot the data
void US_Density_Match::plot_data( void )
{
   int syssiz = alldis.size();

   if ( syssiz < 1 )
      return;

   if ( curr_distr < 0  ||  curr_distr >= syssiz )
   {   // current distro index somehow out of valid range
      qDebug() << "curr_distr=" << curr_distr
         << "  ( sys.size()=" << syssiz << " )";
      syssiz--;
      curr_distr     = qBound( curr_distr, 0, syssiz );
   }

//   zpcent   = ck_zpcent->isChecked();

   // Get current distro and (if need be) rebuild BF distro
   DisSys* tsys   = (DisSys*)&alldis.at( curr_distr );
   QList< S_Solute >* sol_d = &tsys->in_distro;

   build_bf_distro();

   QString tstr = tsys->run_name + "\n" + tsys->analys_name
                  + "\n" + tsys->method;
   data_plot->setTitle( tstr );
   data_plot->detachItems( QwtPlotItem::Rtti_PlotSpectrogram );
   QColor bg   = QColor( Qt::white );
   data_plot->setCanvasBackground( bg );
   int    csum = bg.red() + bg.green() + bg.blue();
   pick->setTrackerPen( QPen( csum > 600 ? QColor( Qt::black ) :
                                           QColor( Qt::white ) ) );

   QwtDoubleRect drect;

   if ( auto_sxy )
      drect = QwtDoubleRect( 0.0, 0.0, 0.0, 0.0 );

   else
   {
      drect = QwtDoubleRect( plt_smin, plt_kmin,
            ( plt_smax - plt_smin ), ( plt_kmax - plt_kmin ) );
   }

   plt_zmin = 1e+8;
   plt_zmax = -1e+8;

   if ( auto_scz )
   {  // Find Z min,max for current distribution
      for ( int jj = 0; jj < sol_d->size(); jj++ )
      {
         double zval = sol_d->at( jj ).c;
         plt_zmin    = qMin( plt_zmin, zval );
         plt_zmax    = qMax( plt_zmax, zval );
      }
   }
   else
   {  // Find Z min,max for all distributions
      for ( int ii = 0; ii < alldis.size(); ii++ )
      {
         DisSys* tsys = (DisSys*)&alldis.at( ii );
         QList< S_Solute >* sol_z  = &tsys->in_distro;

         for ( int jj = 0; jj < sol_z->size(); jj++ )
         {
            double zval = sol_z->at( jj ).c;
            plt_zmin    = qMin( plt_zmin, zval );
            plt_zmax    = qMax( plt_zmax, zval );
         }
      }
   }

   // Set axis settings
   xa_title    = anno_title( plot_x );
   ya_title    = anno_title( ATTR_F );
   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   if ( auto_sxy )
   { // Auto scale x and y
      data_plot->setAxisAutoScale( QwtPlot::yLeft   );
      data_plot->setAxisAutoScale( QwtPlot::xBottom );
   }
   else
   { // Manual limits on x and y
      double lStep = data_plot->axisStepSize( QwtPlot::yLeft   );
      double bStep = data_plot->axisStepSize( QwtPlot::xBottom );
      data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax, bStep );
      data_plot->setAxisScale( QwtPlot::yLeft,   plt_kmin, plt_kmax, lStep );
   }

   data_plot->replot();

   //QString dtext = te_distr_info->toPlainText().section( "\n", 0, 1 );
   QString dtext  = tr( "Run:  " ) + tsys->run_name
         + " (" + tsys->method + ")\n    " + tsys->analys_name;

   bool sv_plot = ck_savepl->isChecked();
DbgLv(2) << "(1) sv_plot" << sv_plot;

DbgLv(2) << "(3)   need_save sv_plot" << need_save << sv_plot;
   //if ( need_save  &&  sv_plot )
   if ( sv_plot )
   {  // Automatically save plot image in a PNG file
      const QString s_attrs[] = { "s", "ff0", "MW", "vbar", "D", "f" };
      QPixmap plotmap = ((QWidget*)data_plot)->grab();

      QString runid   = tsys->run_name.section( ".",  0, -2 );
      QString triple  = tsys->run_name.section( ".", -1, -1 );
      QString report  = QString( "pseudo3d_" ) + s_attrs[ plot_x ]
         + "_" + s_attrs[ ATTR_F ];

      QString ofdir   = US_Settings::reportDir() + "/" + runid;
      QDir dirof( ofdir );
      if ( !dirof.exists( ) )
         QDir( US_Settings::reportDir() ).mkdir( runid );
      QString ofname = tsys->method + "." + triple + "." + report + ".png";
      QString ofpath = ofdir + "/" + ofname;

      plotmap.save( ofpath );
      dtext          = dtext + tr( "\nPLOT %1 SAVED to local" )
         .arg( curr_distr + 1 );

      if ( dkdb_cntrls->db()  &&  !ck_locsave->isChecked() )
      {  // Save a copy to the database
QDateTime time0=QDateTime::currentDateTime();
         US_Passwd   pw;
         US_DB2      db( pw.getPasswd() );
         QStringList query;
         query << "get_editID" << tsys->editGUID;
         db.query( query );
         db.next();
         int         idEdit   = db.value( 0 ).toString().toInt();
         US_Report   freport;
         freport.runID        = runid;
         freport.saveDocumentFromFile( ofdir, ofname, &db, idEdit );
QDateTime time1=QDateTime::currentDateTime();
qDebug() << "DB-save: currdist" << curr_distr
 << "svtime:" << time0.msecsTo(time1);
         dtext          = dtext + tr( " and DB" );
      }
   }

   else
      dtext          = dtext + tr( "\n(no plot saved)" );

   te_distr_info->setText( dtext );

}

void US_Density_Match::plot_data( int )
{
   plot_data();
}

void US_Density_Match::update_resolu( double dval )
{
   resolu = dval;
}

void US_Density_Match::update_xreso( double dval )
{
   xreso  = dval;
}

void US_Density_Match::update_yreso( double dval )
{
   yreso  = dval;
}

void US_Density_Match::update_curr_distr( double dval )
{
   curr_distr   = qRound( dval ) - 1;
DbgLv(1) << "upd_curr_distr" << curr_distr;
}

void US_Density_Match::update_plot_smin( double dval )
{
   plt_smin = dval;
DbgLv(1) << "plt_smin" << plt_smin;
}

void US_Density_Match::update_plot_smax( double dval )
{
   plt_smax = dval;
DbgLv(1) << "plt_smax" << plt_smax;
}

void US_Density_Match::update_plot_kmin( double dval )
{
   plt_kmin = dval;
}

void US_Density_Match::update_plot_kmax( double dval )
{
   plt_kmax = dval;
}

void US_Density_Match::select_autosxy()
{
   auto_sxy   = ck_autosxy->isChecked();
   ct_plt_kmin->setEnabled( !auto_sxy );
   ct_plt_kmax->setEnabled( !auto_sxy );
   ct_plt_smin->setEnabled( !auto_sxy );
   ct_plt_smax->setEnabled( !auto_sxy );

   set_limits();
}

void US_Density_Match::select_autoscz()
{
   auto_scz   = ck_autoscz->isChecked();

   set_limits();
}

void US_Density_Match::load_distro()
{
   // Get a model description or set of descriptions for distribution data
   QList< US_Model > models;
   bool              loadDB = dkdb_cntrls->db();

   QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
   US_ModelLoader dialog( loadDB, mfilter, models, mdescs, pfilts );
   dialog.move( this->pos() + QPoint( 200, 200 ) );

   connect( &dialog, SIGNAL(   changed( bool ) ),
            this, SLOT( update_disk_db( bool ) ) );
   QApplication::restoreOverrideCursor();

   if ( dialog.exec() != QDialog::Accepted )
      return;  // no selection made

   need_save  = false;

   for ( int jj = 0; jj < models.count(); jj++ )
   {  // Load each selected distribution model
      load_distro( models[ jj ], mdescs[ jj ] );
   }

   curr_distr = alldis.size() - 1;
   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
   pb_rmvdist->setEnabled( models.count() > 0 );

   // Notify user of need to set D2O-percent, label, density model values
   QString qmsg = tr( "%1 models are loaded. In the dialog to follow,\n"
                      "you must set D2O Percent values for each,\n"
                      "then review and set Label and Density values for them.\n"
                      "Begin with the model(s) with 0% D2O." )
                      .arg( alldis.size() );
   QMessageBox::warning( this, tr( "Model Parameters" ), qmsg );

   // Set model distributions parameters
   set_mparms();
}

// Create distributions from a loaded model
void US_Density_Match::load_distro( US_Model model, QString mdescr )
{
   DisSys      tsys;
   S_Solute    sol_in;
   S_Solute    sol_nm;
   S_Solute    sol_bf;
   QList< S_Solute >   wk_distro;

   model.update_coefficients();          // fill in any missing coefficients

   QString mdesc     = mdescr.section( mdescr.left( 1 ), 1, 1 );

   tsys.run_name     = mdesc.section( ".",  0, -3 );
   QString asys      = mdesc.section( ".", -2, -2 );
   tsys.analys_name  = asys.section( "_",  0, -4 ) + "_"
                     + asys.section( "_", -2, -1 );
   tsys.method       = model.typeText();
   tsys.method       = ( mdesc.contains( "-CG" ) )
                       ? tsys.method.replace( "2DSA", "2DSA-CG" )
                       : tsys.method;
   if ( model.global == US_Model::GLOBAL )
   {
      tsys.method       = QString( mdesc ).section( '.', 2, 2 )
                                          .section( '_', 2, 2 );
   }
   tsys.editGUID     = model.editGUID;
   tsys.plot_x       = plot_x;
DbgLv(1) << "LD: method" << tsys.method << "mdesc" << mdesc;

   if ( tsys.method == "Manual"  ||  tsys.method == "CUSTOMGRID" )
   {
      int jj            = mdesc.indexOf( ".model" );
      mdesc             = ( jj < 1 ) ? mdesc : mdesc.left( jj );
      tsys.run_name     = mdesc + ".0Z280";
      tsys.analys_name  = "e00_a00_" + tsys.method + "_local";
DbgLv(1) << "LD:  run_name" << tsys.run_name;
DbgLv(1) << "LD:  analys_name" << tsys.analys_name;
   }

DbgLv(1) << "LD:  model:" << model.description;
   tsys.distro_type  = (int)model.analysis;
   QString edir      = US_Settings::tmpDir();
   QString efname    = tsys.run_name + ".xml";

   // Read in edit for this model
   US_DataIO::EditedData edata;
   US_DB2* dbP       = NULL;

   if ( dkdb_cntrls->db() )
   {  // Set up to read from database
      US_Passwd pw;
      dbP               = new US_DB2( pw.getPasswd() );
      QStringList qry;
      qry << "get_editID" << model.editGUID;
      dbP->query( qry );
      dbP->next();
      QString editID    = dbP->value( 0 ).toString();
      int idEdit        = editID.toInt();
DbgLv(1) << "LD:   idEdit" << idEdit;
      qry.clear();
      qry << "get_editedData" << editID;
      dbP->query( qry );
      dbP->next();
      int idRaw         = dbP->value( 0 ).toString().toInt();
      efname            = dbP->value( 3 ).toString();
DbgLv(1) << "LD:   edir" << edir;
DbgLv(1) << "LD:   efname" << efname;
      QString epath     = edir + "/" + efname;
      QString rpath     = edir + "/" + efname.section( ".", 0, -7 ) + "."
                          + efname.section( ".", -5, -2 ) + ".auc";

      dbP->readBlobFromDB( epath, "download_editData", idEdit );
      dbP->readBlobFromDB( rpath, "download_aucData",  idRaw );
   }

   // Read in edit
   US_DataIO::loadData( edir, efname, edata );
DbgLv(1) << "LD:  edata: desc run cell chan"
 << edata.description << edata.runID << edata.cell << edata.channel;
   tsys.label    = edata.description;
   tsys.d2opct   = -1.0;

   // Now, get associated solution,buffer values
   QString soluID;
   QString cvbar20;
   QString bdens;
   QString bvisc;
   QString cmprss;
   QString bmanu;
   QString errmsg;
DbgLv(1) << "LD:  solvals CALL";
   US_SolutionVals::values( dbP, &edata,
      soluID, cvbar20, bdens, bvisc, cmprss, bmanu, errmsg );
   tsys.bdensity = bdens.toDouble();
DbgLv(1) << "LD:  solval: bdens soluID bdensity"
 << bdens << soluID << tsys.bdensity;

   
   te_distr_info->setText( tr( "Run:  " ) + tsys.run_name
      + " (" + tsys.method + ")\n    " + tsys.analys_name );
   int nsolmc  = model.components.size();

   // Read in and set distribution s,k,c,... values
   double tot_conc = 0.0;

   for ( int jj = 0; jj < nsolmc; jj++ )
   {
      sol_in.s  = model.components[ jj ].s * 1.0e13;
      sol_in.k  = model.components[ jj ].f_f0;
      sol_in.c  = model.components[ jj ].signal_concentration;
      sol_in.w  = model.components[ jj ].mw;
      sol_in.v  = model.components[ jj ].vbar20;
      sol_in.d  = model.components[ jj ].D * 1.0e7;
      sol_in.f  = model.components[ jj ].f;

      tsys.in_distro << sol_in;
      wk_distro << sol_in;

      tot_conc += sol_in.c;
if ( jj<3   ||  (jj+4)>nsolmc )
 DbgLv(1) << "LD:    jj" << jj << "soli s,d,c,t"
  << sol_in.s << sol_in.d << sol_in.c << tot_conc;
   }

   // Sort and reduce distributions, then normalize
   sort_distro( wk_distro, true  );

   tsys.nm_distro.clear();
   int nsolin = tsys.in_distro.size();
   int nsolnm = wk_distro.size();
DbgLv(1) << "LD: totconc" << tot_conc << "nsolin nsolnm" << nsolin << nsolnm;
   for ( int jj = 0; jj < nsolnm; jj++ )
   {
      sol_nm      = wk_distro[ jj ];
      sol_nm.c   /= tot_conc;
      tsys.nm_distro << sol_nm;
if ( jj<3   ||  (jj+4)>nsolnm )
 DbgLv(1) << "LD:    jj" << jj << "soln s,d,c"
  << sol_nm.s << sol_nm.d << sol_nm.c;
   }
DbgLv(1) << "LD: nsolin nsolnm" << nsolin << nsolnm << tsys.nm_distro.size();

   // Create version of distribution with boundary fraction

   tsys.bo_distro.clear();
   double sum_co  = 0.0;

   for ( int jj = 0; jj < nsolnm; jj++ )
   {
      sol_nm      = tsys.nm_distro[ jj ];
      sum_co     += sol_nm.c;
      sol_bf      = sol_nm;
      sol_bf.f    = sum_co;
      tsys.bo_distro << sol_bf;
if ( jj<3   ||  (jj+4)>nsolnm )
 DbgLv(1) << "LD:    jj" << jj << "solb s,d,c,f"
  << sol_bf.s << sol_bf.d << sol_bf.c << sol_bf.f;
   }

   // Update current distribution record
DbgLv(1) << "LD:  call alldis.append";
   alldis.append( tsys );
DbgLv(1) << "LD:   retn fr alldis.append";

#if 0
   if ( auto_sxy )
   {
DbgLv(1) << "LD:  auto_sxy call set_limits";
      set_limits();
   }
   data_plot->setAxisScale( QwtPlot::xBottom, plt_smin, plt_smax );
   data_plot->setAxisScale( QwtPlot::yLeft,   plt_kmin, plt_kmax );

   pb_pltall ->setEnabled( true );
   pb_refresh->setEnabled( true );
   pb_reset  ->setEnabled( true );

   if ( cont_loop )
      pb_pltall->setText( tr( "Plot All Distros in a Loop" ) );
   else
      pb_pltall->setText( tr( "Plot All Distros" ) );
#endif
DbgLv(1) << "LD: RETURN";
}

// Start a loop of plotting all distros
void US_Density_Match::plotall()
{
   pb_stopplt->setEnabled( true );
   curr_distr = 0;
   plot_data();
   patm_dlay  = qRound( ct_plt_dlay->value() * 1000.0 );

   patm_id    = startTimer( patm_dlay );

   if ( curr_distr == alldis.size() )
      curr_distr--;

//   need_save  = ck_savepl->isChecked()  &&  !cont_loop;
}

void US_Density_Match::set_limits()
{
   double smin = 1.0e30;
   double smax = -1.0e30;
   double kmin = 1.0e30;
   double kmax = -1.0e30;
   double sinc;
   double kinc;
   xa_title    = anno_title( plot_x );
   ya_title    = anno_title( ATTR_F );

   data_plot->setAxisTitle( QwtPlot::xBottom, xa_title );
   data_plot->setAxisTitle( QwtPlot::yLeft,   ya_title );

   if ( alldis.size() < 1 )
      return;

   // find min,max for X,Y distributions
   for ( int ii = 0; ii < alldis.size(); ii++ )
   {
      DisSys* tsys = (DisSys*)&alldis.at( ii );

      for ( int jj = 0; jj < tsys->nm_distro.size(); jj++ )
      {
         double sval = tsys->nm_distro.at( jj ).s;
         double kval = tsys->nm_distro.at( jj ).k;
         smin        = qMin( smin, sval );
         smax        = qMax( smax, sval );
         kmin        = qMin( kmin, kval );
         kmax        = qMax( kmax, kval );
      }
   }

   // adjust minima, maxima
   sinc      = ( smax - smin ) / 10.0;
   kinc      = ( kmax - kmin ) / 10.0;
   sinc      = ( sinc <= 0.0 ) ? ( smin * 0.05 ) : sinc;
   kinc      = ( kinc <= 0.0 ) ? ( kmin * 0.05 ) : kinc;
DbgLv(1) << "SL: real smin smax kmin kmax" << smin << smax << kmin << kmax;
   smin     -= sinc;
   smax     += sinc;
   kmin     -= kinc;
   kmax     += kinc;
DbgLv(1) << "SL: adjusted smin smax kmin kmax" << smin << smax << kmin << kmax;

   if ( auto_sxy )
   {  // Set auto limits on X and Y
      sinc        = pow( 10.0, qFloor( log10( smax ) ) - 3.0 );
      kinc        = pow( 10.0, qFloor( log10( kmax ) ) - 3.0 );
      if ( qAbs( ( smax - smin ) / smax ) < 0.001 )
      {  // Put padding around virtually constant value
         smin     -= sinc;
         smax     += sinc;
      }
      if ( qAbs( ( kmax - kmin ) / kmax ) < 0.001 )
      {  // Put padding around virtually constant value
         kmin     -= kinc;
         kmax     += kinc;
      }
      // Make sure limits are nearest reasonable values
      smin        = qFloor( smin / sinc ) * sinc;
      smax        = qFloor( smax / sinc ) * sinc + sinc;
      smin        = ( plot_x != ATTR_S ) ? qMax( smin, 0.0 ) : smin;
      smin        = ( plot_x == ATTR_K ) ? qMax( smin, 0.5 ) : smin;
      kmin        = qFloor( kmin / kinc ) * kinc;
      kmax        = qFloor( kmax / kinc ) * kinc + kinc;

DbgLv(1) << "SL: setVal kmin kmax" << kmin << kmax;
#if 0
      ct_plt_smin->setValue( smin );
      ct_plt_smax->setValue( smax );
      ct_plt_kmin->setValue( kmin );
      ct_plt_kmax->setValue( kmax );
#endif

      plt_smin    = smin;
      plt_smax    = smax;
      plt_kmin    = kmin;
      plt_kmax    = kmax;
   }
   else
   {
#if 0
      plt_smin    = ct_plt_smin->value();
      plt_smax    = ct_plt_smax->value();
      plt_kmin    = ct_plt_kmin->value();
      plt_kmax    = ct_plt_kmax->value();
#endif
   }
DbgLv(1) << "SL: plt_smin _smax _kmin _kmax" << plt_smin << plt_smax
 << plt_kmin << plt_kmax;
}

// Sort distribution solute list by s,d values and optionally reduce
void US_Density_Match::sort_distro( QList< S_Solute >& listsols,
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
      QList< S_Solute >::iterator jj = listsols.begin();
      sol1     = *jj;
      reduced.append( *jj );     // output first entry
      int kdup = 0;
      int jdup = 0;

      while ( (++jj) != listsols.end() )
      {     // loop to compare each entry to previous
          sol2    = *jj;         // solute entry

          if ( sol1.s != sol2.s  ||  sol1.d != sol2.d )
          {   // not a duplicate, so output to temporary list
             reduced.append( sol2 );
             jdup    = 0;
          }

          else
          {  // duplicate, so sum c value;
             sol2.c += sol1.c;   // sum c value
             sol2.s  = ( sol1.s + sol2.s ) * 0.5;  // average s,d
             sol2.d  = ( sol1.d + sol2.d ) * 0.5;
             reduced.replace( reduced.size() - 1, sol2 );
             kdup    = max( kdup, ++jdup );
          }

          sol1    = sol2;        // save entry for next iteration
      }

      if ( kdup > 0 )
      {   // if some reduction happened, replace list with reduced version
         double sc = 1.0 / (double)( kdup + 1 );
DbgLv(1) << "KDUP" << kdup;
//sc = 1.0;

         for ( int ii = 0; ii < reduced.size(); ii++ )
         {  // first scale c values by reciprocal of maximum replicate count
            reduced[ ii ].c *= sc;
         }

         listsols = reduced;
DbgLv(1) << " reduced-size" << reduced.size();
      }
   }
DbgLv(1) << " sol-size" << listsols.size();
   return;
}

// Reset Disk_DB control whenever data source is changed in any dialog
void US_Density_Match::update_disk_db( bool isDB )
{
   isDB ? dkdb_cntrls->set_db() : dkdb_cntrls->set_disk();
}

// Select a prefilter for model distributions list
void US_Density_Match::select_prefilt( void )
{
   QString pfmsg;
   int nruns  = 0;
   pfilts.clear();

   US_SelectRuns srdiag( dkdb_cntrls->db(), pfilts );
   srdiag.move( this->pos() + QPoint( 200, 200 ) );
   connect( &srdiag, SIGNAL( dkdb_changed  ( bool ) ),
            this,    SLOT  ( update_disk_db( bool ) ) );

   if ( srdiag.exec() == QDialog::Accepted )
      nruns         = pfilts.size();
   else
      pfilts.clear();

   if ( nruns == 0 )
      pfmsg = tr( "(no prefilter)" );

   else if ( nruns > 1 )
      pfmsg = tr( "RunID prefilter - %1 runs: " ).arg( nruns )
              + QString( pfilts[ 0 ] ).left( 20 ) + "*, ...";

   else
      pfmsg = tr( "RunID prefilter - 1 run: " )
              + QString( pfilts[ 0 ] ).left( 20 ) + " ...";

   le_prefilt->setText( pfmsg );
}


// Remove distribution(s) from the models list
void US_Density_Match::remove_distro( void )
{
qDebug() << "rmvdis:Remove Distros";
   US_RemoveModels rmvd( alldis );

   if ( rmvd.exec() == QDialog::Accepted )
   {
      int jd     = alldis.size();

      if ( jd < 1 )
      {
         reset();
         return;
      }

      curr_distr = 0;
qDebug() << "rmvdis:Accepted";
   }

qDebug() << "rmvdis:plot_data";
   plot_data();
qDebug() << "rmvdis:DONE";
}

// Set/modify model distribution parameters
void US_Density_Match::set_mparms( void )
{
qDebug() << "mdlpar:Set Model Parameters";
   US_ModelParams mpdiag( alldis, this );

   if ( mpdiag.exec() == QDialog::Accepted )
   {  // Redo text box summarizing models; calculate vectors
      QString dinfo;
      QString mdesc     = mdescs[ 0 ].section( mdescs[ 0 ].left( 1 ), 1, 1 );
//      int kk            = mdesc.indexOf( "-run" );
//      mdesc             = ( kk > 0 ) ? QString( mdesc ).mid( ( kk + 1 ) )
//                                     : mdesc;
      mdesc             = QString( mdesc ).left( 50 );
      dinfo             = tr( "Run:\n  " ) + mdesc + "...\n\n"
                        + tr( "  D2O Percent  Density  Label  MDescr.\n" );

      for ( int jj = 0; jj < alldis.size(); jj++ )
      {
         double d2opct     = alldis[ jj ].d2opct;
         double bdens      = alldis[ jj ].bdensity;
         QString mlab      = alldis[ jj ].label;
         mdesc             = mdescs[ jj ].section( mdescs[ jj ].left( 1 ), 1, 1 );
         mdesc             = alldis[ jj ].run_name;
         int kk            = mdesc.indexOf( "-run" );
         mdesc             = ( kk > 0 ) 
                            ? "..." + QString( mdesc ).mid( ( kk + 1 ), 20 )
                              + "..."
                            : mdesc;
         dinfo            += QString().sprintf( "%.1f  %f  ", d2opct, bdens )
                             + mlab + "  " + mdesc + "\n";
      }

      te_distr_info->setText( dinfo );
   }
}

// Select coordinate for horizontal axis
void US_Density_Match::select_x_axis( int ival )
{
DbgLv(1) << "sel_x:  ival" << ival;
#if 0
   const QString xlabs[] = {   "mass", "f/f0",   "rh", "vbar",     "s" };
   const double  xvlos[] = {     2e+4,   1.0,    2e+4,  0.60,      1.0 };
   const double  xvhis[] = {     1e+5,   4.0,    1e+5,  0.80,     10.0 };
   const double  xmins[] = {      0.0,   1.0,     0.0,  0.01, -10000.0 };
   const double  xmaxs[] = {    1e+10,  50.0,   1e+10,  3.00,  10000.0 };
   const double  xincs[] = {   1000.0,  0.01,  1000.0,  0.01,     0.01 };
#endif

   const QString xlabs[] = {      "s", "f/f0", "mass","vbar", "D", "f",  "r"   };
   const double  xvlos[] = {      1.0,   1.0,    2e+4,  0.60, 1e-8, 1e-8, 1e-8 };
   const double  xvhis[] = {     10.0,   4.0,    1e+5,  0.80, 1e-7, 1e-7, 1e-7 };
   const double  xmins[] = { -10000.0,   1.0,     0.0,  0.01, 1e-9, 1e-9, 1e-9 };
   const double  xmaxs[] = {  10000.0,  50.0,   1e+10,  3.00, 1e-5, 1e-5, 1e-4 };
   const double  xincs[] = {     0.01,  0.01,  1000.0,  0.01, 1e-9, 1e-9, 1e-9 };
#if 0
   const QString xlabs[] = {      "s", "f/f0",  "MW", "vbar", "D", "f"  };
   const double  xvlos[] = {      1.0,   1.0,   2e+4,  0.60, 1e-8, 1e-8 };
   const double  xvhis[] = {     10.0,   4.0,   1e+5,  0.80, 1e-7, 1e-7 };
   const double  xmins[] = { -10000.0,   1.0,    0.0,  0.01, 1e-9, 1e-9 };
   const double  xmaxs[] = {  10000.0,  50.0,  1e+10,  3.00, 1e-5, 1e-5 };
   const double  xincs[] = {     0.01,  0.01, 1000.0,  0.01, 1e-9, 1e-9 };
#endif

   plot_x     = ival;

#if 0
   lb_plt_smin->setText( tr( "Plot Limit " ) + xlabs[ plot_x ]
                       + tr( " Minimum:" ) );
   lb_plt_smax->setText( tr( "Plot Limit " ) + xlabs[ plot_x ]
                       + tr( " Maximum:" ) );
   ct_plt_smin->setRange( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_plt_smax->setRange( xmins[ plot_x ], xmaxs[ plot_x ] );
   ct_plt_smin->setSingleStep( xincs[ plot_x ] );
   ct_plt_smax->setSingleStep( xincs[ plot_x ] );
   ct_plt_smin->setValue( xvlos[ plot_x ] );
   ct_plt_smax->setValue( xvhis[ plot_x ] );
#endif
DbgLv(1) << "sel_x:   lab vlos vhis xmin xmax xinc" << xlabs[plot_x]
 << xvlos[plot_x] << xvhis[plot_x] << xmins[plot_x] << xmaxs[plot_x]
 << xincs[plot_x];

   build_bf_distro();

//   set_limits();

   plot_data();
}

// (Re-)generate the BF version of the current distribution
void US_Density_Match::build_bf_distro()
{
   if ( alldis.size() < 1 )
      return;
   DisSys* tsys     = (DisSys*)&alldis.at( curr_distr );
   if ( tsys->plot_x == plot_x )
      return;

   tsys->nm_distro.clear();
   tsys->plot_x     = plot_x;
   double tot_conc  = 0.0;
   int    nsolin    = tsys->in_distro.size();

   // Create solute list with specified x,y
   for ( int ii = 0; ii < nsolin; ii++ )
   {
      S_Solute sol_in  = tsys->in_distro[ ii ];
      S_Solute sol_nm  = sol_in;

      sol_nm.s  = ( plot_x == ATTR_S ) ? sol_in.s : sol_nm.s;
      sol_nm.s  = ( plot_x == ATTR_K ) ? sol_in.k : sol_nm.s;
      sol_nm.s  = ( plot_x == ATTR_W ) ? sol_in.w : sol_nm.s;
      sol_nm.s  = ( plot_x == ATTR_V ) ? sol_in.v : sol_nm.s;
      sol_nm.s  = ( plot_x == ATTR_D ) ? sol_in.d : sol_nm.s;
      sol_nm.s  = ( plot_x == ATTR_F ) ? sol_in.f : sol_nm.s;
      tot_conc += sol_in.c;
      tsys->nm_distro << sol_nm;
   }

   // Sort and possibly reduce XY distro
   sort_distro( tsys->nm_distro, true );

   // Create Z-as-percentage version of xy distribution
   int    nsolnm = tsys->nm_distro.size();
DbgLv(1) << "Bld: nsolin nsolnm" << nsolin << nsolnm;

   for ( int ii = 0; ii < nsolnm; ii++ )
   {
      S_Solute sol_nm  = tsys->nm_distro[ ii ];
      sol_nm.c        /= tot_conc;

      tsys->nm_distro << sol_nm;
   }
}

// Set annotation title for a plot index
QString US_Density_Match::anno_title( int pltndx )
{
   QString a_title;

   if      ( pltndx == ATTR_S )
      a_title  = tr( "Sedimentation Coefficient (1e-13)"
                     " for water at 20" ) + DEGC;
   else if ( pltndx == ATTR_K )
      a_title  = tr( "Frictional Ratio f/f0" );
   else if ( pltndx == ATTR_W )
      a_title  = tr( "Molar Mass (Dalton)" );
   else if ( pltndx == ATTR_V )
      a_title  = tr( "Vbar at 20" ) + DEGC;
   else if ( pltndx == ATTR_D )
      a_title  = tr( "Diffusion Coefficient (1e-7)" );
   else if ( pltndx == ATTR_R )
      a_title  = tr( "Hydrodynamic Radius" );
   else if ( pltndx == ATTR_F )
      a_title  = tr( "Boundary Fraction" );

   return a_title;
}

// Update structures and plot after division change
void US_Density_Match::update_divis( double dval )
{
DbgLv(1) << "UpdDiv:" << dval;
}

