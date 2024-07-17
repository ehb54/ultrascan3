// (this) us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines 
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// us_hydrodyn_load.cpp contains code to load files 
// us_hydrodyn_grpy.cpp contains code for grpy interface
// us_hydrodyn_beads.cpp contains top-level code for generating bead models
// us_hydrodyn_settings.cpp contains code for reading/writing settings
// us_hydrodyn_write.cpp contains code for writes (bead models etc)

// includes and defines need cleanup
 
#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_revision.h"
#include "../include/us3_defines.h"
#include "../include/us_hydrodyn_best.h"
#include "../include/us_ffd.h"
#include "../include/us_hydrodyn_vdw_overlap.h"
// #include "../include/us_hydrodyn_saxs_hplc_options.h"
#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
 //#include <Q3PopupMenu>
#include <QHBoxLayout>
#include <QTextStream>
#include <QGridLayout>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qtimer.h>

// #define NO_BD

#undef DEBUG

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#   define __open open
#else
#   define chdir _chdir
#   define dup2 _dup2
#   define __open _open
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#   define STDOUT_FILENO 1
#   define STDERR_FILENO 2
#endif
#define DOTSOMO      ""
#define DOTSOMOCAP   ""

// #define AVG_TEST
#if defined( AVG_TEST )
# include "../include/us_average.h"
#endif

// #define PINV_TEST
#if defined( PINV_TEST )
#include "../include/us_svd.h"
#endif
// #define SVD_TEST
#if defined( SVD_TEST )
#include "../include/us_svd.h"
#endif
// #define USE_H

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define GRID_HYDRATE_DEBUG

// static bool no_rr;
// #define EIGEN_TEST
#if defined( EIGEN_TEST )
# include "../include/us_eigen.h"
#endif

// #define AVGSD_TEST

US_Hydrodyn::US_Hydrodyn(vector < QString > batch_file,
                         QString gui_script_file,
                         bool    init_configs_silently,
                         QWidget *p, 
                         const char *) : QFrame( p )
{
   us_container_grpy = (US_Container_Grpy *)0;
   grpy_parallel_pulled = false;
   
   stopFlag = false;
   
#if defined( AVG_TEST )
   US_Average avg;
   avg.test();
   // exit(-1);
#endif

#if defined( AVGSD_TEST )
   {
      vector < vector < double > > ys =
         {
            { 1 }
            ,{ 2 }
            ,{ 3 }
            ,{ 4 }
            ,{ 5 }
         };
      vector < double > y_avg;
      vector < double > y_sd;
      QString error_msg;

      for ( auto const & y : ys ) {
         US_Vector::printvector( "y", y );
      }
      US_Saxs_Util::average( ys, y_avg, y_sd, error_msg );
      US_Vector::printvector2( "avg, sd", y_avg, y_sd );
      qDebug() << error_msg;
   }
   exit(-1);
#endif
      

#if defined( EIGEN_TEST )
   {
      US_Eigen eigen;
      eigen.test();
      /*
      vector < double > x =
         {
            0.00228835,
            0.00236877,
            0.00242899,
            0.00247294,
            0.00228047,
            0.00250852,
            0.00273657,
            0.00296461
         };

      vector < double > y = {
         473426.37,
         469031.56,
         457997.95,
         460234.26,
         1112054,
         1167329,
         930548.5,
         850115
      };

      vector < double > e = {
         50.64054,
         90.27224,
         156.1049,
         251.0008,
         967975.4,
         418368.3,
         257145.3,
         188731.4
      };
      */

      vector < double > x =
         {
      1, 2, 3, 4, 5, 6, 7
         };

      // 1.37, .254, 3.26, .55
      vector < double > y = {
      5.434
         ,19.318
         ,46.322
         ,89.746
         ,152.89
         ,239.054
         ,351.538
      };

      vector < double > e = {
      1,1,1,1,1,1,1
      };
      
      vector < double > coeff;
      double            chi2;
      int               degree = 5;
   
      US_Vector::printvector3( "data for fitting, q,I,e", x, y, e );

# define TSO QTextStream(stdout)
      
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_SVD_JACOBI );
      US_Vector::printvector( QString( "coefficients EIGEN_SVD_JACOBI chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "g" );
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_SVD_BDC );
      US_Vector::printvector( QString( "coefficients EIGEN_SVD_BDC chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "h" );
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "i" );
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR_PIVOT_COL );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR_PIVOT_COL chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "j" );
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR_PIVOT_FULL );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR_PIVOT_FULL chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "k" );
      eigen.polyfit( x, y, e, degree, coeff, chi2, EIGEN_NORMAL);
      US_Vector::printvector( QString( "coefficients EIGEN_NORMALC chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "l" );

      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_SVD_JACOBI );
      US_Vector::printvector( QString( "coefficients EIGEN_SVD_JACOBI chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "gp" );
      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_SVD_BDC );
      US_Vector::printvector( QString( "coefficients EIGEN_SVD_BDC chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "hp" );
      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "ip" );
      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR_PIVOT_COL );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR_PIVOT_COL chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "jp" );
      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_HOUSEHOLDER_QR_PIVOT_FULL );
      US_Vector::printvector( QString( "coefficients EIGEN_HOUSEHOLDER_QR_PIVOT_FULL chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "kp" );
      eigen.polyfit( x, y, degree, coeff, chi2, EIGEN_NORMAL);
      US_Vector::printvector( QString( "coefficients EIGEN_NORMALC chi2 %1" ).arg( chi2 ), coeff );
      TSO << eigen.gnuplot_poly( coeff, "lp" );
      
      // if ( eigen.evaluate_polynomial( coeff, 0.002, 0.003, 100, x, y ) ) {
      //    US_Vector::printvector2( "fitting curve", x, y );
      // } else {
      //    qDebug() << eigen.errors();
      // }
   }
   exit(-1);
#endif

   // #define FFD_TEST

#if defined( FFD_TEST )
# define TSO QTextStream(stdout)
   US_FFD ffd(5);
   US_Saxs_Util usu;
   if ( !usu.select_saxs_file( US_Config::get_home_dir() + "etc" + SLASH + "somo.saxs_atoms" ) ) {
      TSO << usu.errormsg << "\n";
   } else {
      TSO << "select_saxs_file OK\n";
      TSO << "usu.saxs_list.size() " << usu.saxs_list.size() << "\n";
      if ( !ffd.set_saxs_coeffs( usu.saxs_list ) ) {
         TSO << ffd.errormsg << "\n";
      } else {
         // TSO << ffd.list_saxs_coeffs();
      }
      ffd.set_q_grid( 0, 1, .0005 );
      TSO << "calc_fq()\n";
      if ( !ffd.calc_fq() ) {
         TSO << ffd.errormsg << "\n";
      } else {
         TSO << "calc_fq() OK\n";
         if ( !ffd.calc_fifjq() ) {
            TSO << ffd.errormsg << "\n";
         } else {
            TSO << "calc_fifjq() OK\n";
         }
      }
      // ffd.test();
   }
   exit(-1);
#endif

// #define DP_TEST

#if defined( DP_TEST )
   {
      QTextStream tso(stdout);

      tso << "dp test\n";

      vector < double > vals = { 0, 1, 1.2, 1.23, 1.234, 1.2345, 1.55555, 1e10, -5.2736 };
      vector < int >    dps  = { 0, 1, 2, 3 };

      for ( int i = 0; i < (int)dps.size(); ++i ) {
         for ( int j = 0; j < (int)vals.size(); ++j ) {
            tso << QString( "decimal places %1, value %2, converted %3\n" )
               .arg( dps[i] )
               .arg( vals[j] )
               .arg( us_double_decimal_places( vals[j], dps[i] ) )
               ;
         }
      }
   }
   
#endif

   // #define PROCESS_TEST

#if defined( PROCESS_TEST )
   QProcess * process = new QProcess( this );
   QString prog = "/usr/lib/rasmol/rasmol.16";
   QString file = "/root/testprocess.input";
   QStringList args;
   args << "-nodisplay";
   process->setWorkingDirectory( QFileInfo( file ).path() );
   //   process->setReadChannelMode(QProcess::SeparateChannels);
   qDebug() << "filename " << QFileInfo( file ).fileName();
   process->setStandardInputFile( file );
   process->setStandardOutputFile( file + ".rmout" );
   process->setStandardErrorFile( file + ".rmerr" );
   qDebug() << "rasmol no display START****************";
   process->start( prog, args );
   if ( !process->waitForStarted() ) {
      qDebug() << "process wait for started failed";
      qDebug() << "error:" << process->error();
      exit(-1);
   }
      
   gui_script = false;

   gparams[ "use_pH" ] == "true";
   
   qDebug() << "rasmol no display WAIT****************";
   process->waitForFinished();
   qDebug() << "rasmol no display FINISHED****************";
   qDebug() << "process finished";
   process->close();
   qDebug() << "rasmol no display CLOSED****************";
   delete process;
   exit(0);
#endif

   this->init_configs_silently = init_configs_silently;

   if ( !gui_script_file.isEmpty() ) {
      qDebug() << "script active " << gui_script_file;
      this->gui_script_file = gui_script_file;
      gui_script = true;
   } else {
      gui_script = false;
   }

#if defined( PINV_TEST )
   qDebug() << "PINV_TEST";

   vector < vector < double > > A =
      { { 1, 2, 3 },
        { 4, 5, 6 } }
      // { { 1, 4 },
      //   { 2, 5 },
      //   { 3, 6 } }
   ;
   vector < vector < double > > Ainv;

   SVD::pinv( A, Ainv );
   qDebug() << "PINV_TEST done";
   exit(0);
#endif

#if defined( SVD_TEST )
   qDebug() << "SVD_TEST";

   vector < vector < double > > A2x3 =
        { { 1, 2, 3 },
          { 4, 5, 6 } };
   
   vector < vector < double > > A3x2 =
      { { 1, 4 },
        { 2, 5 },
        { 3, 6 } }
   ;

   vector < vector < double > > A = A2x3;

   int m = (int) A.size();
   int n = (int) A[0].size();
   
   qDebug() << "SVD_TEST 1";
   
   double a[ m ][ n ];
   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j) {
         a[ i ][ j ] = A[ i ][ j ];
      }
   }

   qDebug() << "SVD_TEST 2";
   double *ause[ m ];
   for ( int j = 0; j < m; ++j ) {
      ause[ j ] = a[ j ];
   }
   
   qDebug() << "SVD_TEST 3";

   SVD::cout_dpp( "a before svd", ause, m, n );

   qDebug() << "SVD_TEST 4";
   // w can stay the same?

   double w[ n ];
         
   // allocate vt
   double v[ n ][ n ];

   double *vuse[ n ];

   for ( int i = 0; i < n; ++i ) {
      vuse[ i ] = v[ i ];
   }
   qDebug() << "SVD_TEST 5";

   if ( !SVD::dsvd( ause, m, n, w, vuse ) ) {
      qDebug() << "SVD failed";
      exit(0);
   }

   // convert to vvd, vd

   vector < vector < double > > u_vvd;
   SVD::dpp_to_vvd( ause, m, m, u_vvd );
   vector < vector < double > > v_vvd;
   SVD::dpp_to_vvd( vuse, n, n, v_vvd );
   vector < double > w_vd;
   SVD::dp_to_vd( w, n, w_vd );
   
   SVD::cout_vvd( "u", u_vvd );
   SVD::cout_vd( "w", w_vd );
   SVD::cout_vvd( "v", v_vvd );

   vector < vector < double > > uw = SVD::vvd_usmult( u_vvd, w_vd );
   SVD::cout_vvd( "uw", uw );

   vector < vector < double > > vt = SVD::vvd_transpose( v_vvd );

   vector < vector < double > > uwvt = SVD::vvd_mult( uw, v_vvd );
   SVD::cout_vvd( "uwvt", uwvt );

   qDebug() << "max norm org - uwvt " << SVD::vvd2_maxnorm( uwvt, A );

#if defined( OLD_SVD_TEST )
   SVD::cout_dpp( "u", ause, m, m );
   SVD::cout_dp( "w", w, n );
   SVD::cout_dpp( "v", vuse, n, n );

   vector < vector < double > > us( m );
   for ( int i = 0; i < m; ++i ) {
      us[ i ].resize( n );
   }

   vector < vector < double > > afinal = us;

   // mult u * w
   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j ) {
         us[ i ][ j ] = ause[ i ][ j ] * w[ j ];
      }
   }

   SVD::cout_vvd( "us(u*s) ", us );

   for ( int i = 0; i < m; ++i ) {
      for ( int j = 0; j < n; ++j ) {
         afinal[ i ][ j ] = 0;
         for ( int k = 0; k < n; ++k ) {
            afinal[ i ][ j ] += us[ i ][ k ] * vuse[ j ][ k ] ;
         }
      }
   }

   SVD::cout_vvd( "afinal ", afinal );

#endif
   
   exit(0);
#endif

   USglobal = new US_Config();
   // // test color override
   // {
   //    QPalette p = PALET_PUSHB;
   //    p.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::red    ); // windowText/foreground
   //    US_GuiSettings::set_pushbColor( p );
   // }

   this->batch_file = batch_file;
   numThreads = USglobal->config_list.numThreads;
   extra_saxs_coefficients.clear( );
   hullrad_running = false;

   last_pdb_filename = "";
   last_pdb_title.clear( );
   last_pdb_header.clear( );
   last_no_model_selected = false;
   last_bead_model = "";

   misc.auto_calc_hydro_method = AUTO_CALC_HYDRO_ZENO;

   SS_setup();

   // no_rr = false;

   // int r_stdout = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stdout.txt").toLatin1().data(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stdout, STDOUT_FILENO);

   // int r_stderr = __open(QString(somo_tmp_dir +
   //           SLASH + "last_stderr.txt").toLatin1().data(),
   //        O_WRONLY | O_CREAT | O_TRUNC, 0666);
   // dup2(r_stderr, STDERR_FILENO);

   {
      QString tmp_dir = USglobal->config_list.root_dir + SLASH + "etc";
      QDir dir1( tmp_dir );
      if (!dir1.exists())
      {
         dir1.mkdir(tmp_dir);
      }
   }

   somo_dir = USglobal->config_list.root_dir + SLASH +  "somo";
   QDir dir1(somo_dir);
   if (!dir1.exists())
   {
      dir1.mkdir(somo_dir);
   }
   somo_pdb_dir = somo_dir + SLASH + "structures";
   QDir dir2(somo_pdb_dir);
   if (!dir2.exists())
   {
      dir2.mkdir(somo_pdb_dir);
   }
   somo_tmp_dir = somo_dir + SLASH + "tmp";
   QDir dir3(somo_tmp_dir);
   if (!dir3.exists())
   {
      dir3.mkdir(somo_tmp_dir);
   }
   QString somo_saxs_dir = somo_dir + SLASH + "saxs";
   QDir dir4(somo_saxs_dir);
   if (!dir4.exists())
   {
      dir4.mkdir(somo_saxs_dir);
   }

   QString somo_saxs_tmp_dir = somo_dir + SLASH + "saxs" + SLASH + "tmp";
   QDir dir5(somo_saxs_tmp_dir);
   if (!dir5.exists())
   {
      dir5.mkdir(somo_saxs_tmp_dir);
   }

   QString somo_bd_dir = somo_dir + SLASH + "bd";
   QDir dir6(somo_bd_dir);
   if (!dir6.exists())
   {
      dir6.mkdir(somo_bd_dir);
   }

   setPalette( PALET_FRAME );
   setWindowTitle(us_tr("SOMO Solution Bead Modeler"));
   advanced_config.auto_view_pdb        = true;
   advanced_config.scroll_editor        = false;
   advanced_config.auto_calc_somo       = false;
   advanced_config.auto_show_hydro      = false;
   advanced_config.pbr_broken_logic     = true;
   advanced_config.use_sounds           = false;
   advanced_config.expert_mode          = false;
   advanced_config.experimental_threads = false;
   advanced_config.experimental_renum   = false;
   advanced_config.debug_1              = false;
   advanced_config.debug_2              = false;
   advanced_config.debug_3              = false;
   advanced_config.debug_4              = false;
   advanced_config.debug_5              = false;
   advanced_config.debug_6              = false;
   advanced_config.debug_7              = false;
   atom_widget                          = false;
   best_widget                          = false;
   residue_widget                       = false;
   hybrid_widget                        = false;
   saxs_widget                          = false;
   saxs_options_widget                  = false;
   sas_options_saxs_widget              = false;
   sas_options_sans_widget              = false;
   sas_options_curve_widget             = false;
   sas_options_bead_model_widget        = false;
   sas_options_hydration_widget         = false;
   sas_options_guinier_widget           = false;
   sas_options_xsr_widget               = false;
   sas_options_misc_widget              = false;
   sas_options_experimental_widget      = false;
   saxs_plot_widget                     = false;
   fractal_dimension_options_widget     = false;
   asa_widget                           = false;
   misc_widget                          = false;
   vdw_overlap_widget                   = false;
   grid_widget                          = false;
   hydro_widget                         = false;
   hydro_zeno_widget                    = false;
   overlap_widget                       = false;
   grid_overlap_widget                  = false;
   bead_output_widget                   = false;
   results_widget                       = false;
   pdb_visualization_widget             = false;
   pdb_parsing_widget                   = false;
   advanced_config_widget               = false;
   saxs_search_widget                   = false;
   saxs_screen_widget                   = false;
   saxs_buffer_widget                   = false;
   saxs_hplc_widget                     = false;
   saxs_1d_widget                       = false;
   saxs_2d_widget                       = false;
   bd_widget                            = false;
   bd_options_widget                    = false;
   dmd_options_widget                   = false;
   anaflex_options_widget               = false;
   batch_widget                         = false;
   save_widget                          = false;
   comparative_widget                   = false;
   if ( !install_new_version() )
   {
      exit(0);
   }
   set_default();   // setup configuration defaults before reading initial config
   read_config(""); // specify default configuration by leaving argument empty

   calcAutoHydro                        = false;
   overwrite                            = false;
   overwrite_hydro                      = false;
   saveParams                           = false;
   setSuffix                            = true;
   guiFlag                              = true;
   bead_model_selected_filter           = "";
   residue_filename                     = US_Config::get_home_dir() + "etc/somo.residue";
   editor                               = (mQTextEdit *)0;

#if QT_VERSION >= 0x040000
   gparams[ "zeno_cxx" ]                = "true";
#endif

   last_saxs_search_csv.name            = "__empty__";
   last_saxs_screen_csv.name            = "__empty__";
   last_saxs_buffer_csv.name            = "__empty__";
   last_saxs_hplc_csv.name              = "__empty__";

   misc.restore_pb_rule                 = false;

   if ( saxs_options.default_hybrid_filename.isEmpty() ) {
      saxs_options.default_hybrid_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hybrid";
   }
   read_hybrid_file( "" );
   read_residue_file();

   bd_ready_to_run = false;
   anaflex_ready_to_run = false;
   bd_last_file = "";
   anaflex_last_file = "";
   state = NOTHING_LOADED;
   setupGUI();
   //   global_Xpos += 30;
   //   global_Ypos += 30;
   //   setGeometry(global_Xpos, global_Ypos, 0, 0);
   create_beads_normally = true;
   alt_method = false;
   rasmol = NULL;
   browflex = NULL;
   anaflex = NULL;
   anaflex_return_to_bd_load_results = false;
   bd_anaflex_enables(false);
   grpy = NULL;

   last_read_bead_model = "";
   last_hydro_res = "";
   QDir::setCurrent(somo_tmp_dir);
   if ( advanced_config.debug_5 )
   {
      printf("%s\n", QString(somo_tmp_dir).toLatin1().data());
   }
   results.total_beads = 0;
   results.used_beads = 0;
   results.mass = 0.0;
   results.s20w = 0.0;
   results.D20w = 0.0;
   results.viscosity = 0.0;
   results.ff0 = 0.0;
   results.rs = 0.0;
   results.rg = 0.0;
   results.vbar = 0.72;
   results.tau = 0.0;
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.ff0_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;
   results.asa_rg_pos = 0.0;
   results.asa_rg_neg = 0.0;

   // move to save/restore
   // put in set_default!!!!!

   // this should be stored in the residue file, hardcoded for now

   residue_short_names["GLY"] = 'G';
   residue_short_names["ALA"] = 'A';
   residue_short_names["VAL"] = 'V';
   residue_short_names["LEU"] = 'L';
   residue_short_names["ILE"] = 'I';
   residue_short_names["MET"] = 'M';
   residue_short_names["PHE"] = 'F';
   residue_short_names["TRP"] = 'W';
   residue_short_names["PRO"] = 'P';
   residue_short_names["SER"] = 'S';
   residue_short_names["THR"] = 'T';
   residue_short_names["CYS"] = 'C';
   residue_short_names["CYH"] = 'C';
   residue_short_names["TYR"] = 'Y';
   residue_short_names["ASN"] = 'N';
   residue_short_names["GLN"] = 'Q';
   residue_short_names["ASP"] = 'D';
   residue_short_names["GLU"] = 'E';
   residue_short_names["LYS"] = 'K';
   residue_short_names["ARG"] = 'R';
   residue_short_names["HIS"] = 'H';
   residue_short_names["WAT"] = '~';

   residue_short_names["G"] = 'g';
   residue_short_names["A"] = 'a';
   residue_short_names["C"] = 'c';
   residue_short_names["T"] = 't';
   residue_short_names["U"] = 'u';
   residue_short_names["DG"] = 'g';
   residue_short_names["DA"] = 'a';
   residue_short_names["DC"] = 'c';
   residue_short_names["DT"] = 't';

   create_fasta_vbar_mw();

   options_log = "";
   last_abb_msgs = "";

   comparative = US_Hydrodyn_Comparative::empty_comparative_info();

   rasmol = new QProcess(this);
   rasmol->setWorkingDirectory(
#if QT_VERSION < 0x040000
                               QDir(
#endif
                                    USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
                                     "bin64"
#else
                                    "/bin/"
#endif
				    + SLASH
#if QT_VERSION < 0x040000
                                    )
#endif
                               );

   bead_model_from_file = false;
   QString RMP = "RASMOLPATH=" + USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
     + SLASH
     ;
   bead_model_prefix = "";
   if (!getenv("RASMOLPATH"))
   {
      int n = RMP.length();
      // note this is not free'd because it becomes part of the environment....
      char* rmp = (char*) malloc (n + 1);
      if ( rmp )
      {
         strncpy( rmp, RMP.toLatin1().data(), n );
         *( rmp + n ) = 0;
         putenv( rmp );
      }
   }
   play_sounds(1);
   editor->append(QString(us_tr("\n\nWelcome to SOMO UltraScan %1 %2\nProduced on: %3\n"))
		  .arg(US_Version)
		  .arg(REVISION)
		  .arg(REVISION_DATE)
		  );
   if ( numThreads > 1 )
   {
      editor->append(QString(us_tr("Multi-threading enabled using %1 threads\n")).arg(numThreads));
   }
   QColor save_color = editor->textColor();
   editor->setTextColor("red");
   if (!dir1.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_dir + us_tr(" does not exist.\n"));
   }
   if (!dir2.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_pdb_dir + us_tr(" does not exist.\n"));
   }
   if (!dir3.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_tmp_dir + us_tr(" does not exist.\n"));
   }
   if (!dir4.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_saxs_dir + us_tr(" does not exist.\n"));
   }
   if (!dir5.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_saxs_tmp_dir + us_tr(" does not exist.\n"));
   }
   if (!dir6.exists())
   {
      editor->append(us_tr("Warning: Directory ") + somo_bd_dir + us_tr(" does not exist.\n"));
   }
   editor->setTextColor(save_color);

   if ( batch_file.size() )
   {
      batch.file = batch_file;
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->show();
      batch_window->raise();
   }
   
   save_util = new US_Hydrodyn_Save(&save_params, this);
   fixWinButtons( save_util );
 
   if ( !gparams.count( "zeno_repeats" ) )
   {
      gparams[ "zeno_repeats" ] = "1";
   }
   if ( !gparams.count( "zeno_max_cap" ) )
   {
      gparams[ "zeno_max_cap" ] = "false";
   }
   if ( !gparams.count( "zeno_max_cap_pct" ) )
   {
      gparams[ "zeno_max_cap_pct" ] = "0.5";
   }

   if ( saxs_options.default_atom_filename.isEmpty() )
   {
      saxs_options.default_atom_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.atom";
   }
   if ( saxs_options.default_saxs_filename.isEmpty() )
   {
      saxs_options.default_saxs_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.saxs_atoms";
   }
   if ( saxs_options.default_rotamer_filename.isEmpty() )
   {
      saxs_options.default_rotamer_filename = US_Config::get_home_dir() + "etc" + SLASH + "somo.hydrated_rotamer";
   }
   if ( saxs_options.default_ff_filename.isEmpty() )
   {
      saxs_options.default_ff_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.ff";
   }

   saxs_util = new US_Saxs_Util();
   if ( 
       !saxs_util->setup_saxs_maps( 
                                   saxs_options.default_atom_filename ,
                                   saxs_options.default_hybrid_filename ,
                                   saxs_options.default_saxs_filename 
                                   )
       &&
       saxs_options.compute_saxs_coeff_for_bead_models
       )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr(
                        "Warning: Error setting up SAXS structure factor files.\n"
                        "Bead model SAXS disabled.\n"
                        "Check to make sure the files in SOMO->SAXS/SANS Options->Miscellaneous are correct.\n"
                        ));
      editor->setTextColor(save_color);
      saxs_options.compute_saxs_coeff_for_bead_models = false;
   } else {
      saxs_util->setup_saxs_options();
      saxs_util->our_saxs_options = saxs_options;
   }

   if ( 
       !saxs_util->load_mw_json( USglobal->config_list.system_dir + 
                                 QDir::separator() + "etc" +
                                 QDir::separator() + "mw.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: mw.json not read" ) );
   }

   if ( 
       !saxs_util->load_vdw_json( USglobal->config_list.system_dir + 
                                  QDir::separator() + "etc" +
                                  QDir::separator() + "vdw.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: vdw.json not read" ) );
   }

   if ( 
       !saxs_util->load_vcm_json( USglobal->config_list.system_dir + 
                                  QDir::separator() + "etc" +
                                  QDir::separator() + "vcm.json" ) )
   {
      editor_msg( "red", us_tr( "Warning: vcm.json not read" ) );
   }

   // now loaded with residue file
   // if ( 
   //     load_vdwf_json( USglobal->config_list.system_dir + 
   //                     QDir::separator() + "etc" +
   //                     QDir::separator() + "vdwf.json" ) )
   // {
   //    editor_msg( "red", us_tr( "Warning: vdwf.json not read" ) );
   // }

   if ( saxs_options.wavelength == 0 )
   {
      saxs_options.start_q = 
         saxs_options.end_q = 
         saxs_options.delta_q = 0;
   }
   else
   {
      saxs_options.start_q = 4.0 * M_PI * 
         sin(saxs_options.start_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.start_q =  floor(saxs_options.start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options.end_q = 4.0 * M_PI * 
         sin(saxs_options.end_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.end_q =  floor(saxs_options.end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options.delta_q = 4.0 * M_PI * 
         sin(saxs_options.delta_angle * M_PI / 360.0) / 
         saxs_options.wavelength;
      saxs_options.delta_q =  floor(saxs_options.delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         
   add_to_directory_history( somo_pdb_dir, false );
   add_to_directory_history( somo_saxs_dir, false );
   {
      QString demo_dir = USglobal->config_list.system_dir + SLASH + "somo" + SLASH + "demo";
      if ( QFile( demo_dir ).exists() ) {
         add_to_directory_history( demo_dir, false );
      } else {
         qDebug() << "Notice: demo dir " << demo_dir << " not added to directory history as it doesn't exist";
      }
   }

#if defined( HB_TEST )
   // hb test

   {
      vector < double >  P;
      P.push_back( .0007 );
      P.push_back( .0003 );
      P.push_back( .00001 );
      P.push_back( .006 );
      P.push_back( .006 );
      P.push_back( .00002 );

      us_qdebug( QString( "holm bonferroni returns %1" ).arg( US_Saxs_Util::holm_bonferroni( P, 0.01 ) ) );
   }
#endif

#warning - perhaps add select_atom_file() to input selections

   select_atom_file( US_Config::get_home_dir() + "etc/somo.atom" );
   clear_temp_dirs();

   if ( gui_script ) {
      emit gui_script_run();
   }
}

US_Hydrodyn::~US_Hydrodyn()
{
}

void US_Hydrodyn::setupGUI()
{
   int minHeight1 = 24;
   bead_model_file = "";

#if QT_VERSION < 0x040000
 //   lookup_tables = new Q3PopupMenu;
   lookup_tables->insertItem(us_tr("Add/Edit &Hybridization"), this, SLOT(hybrid()));
   lookup_tables->insertItem(us_tr("Add/Edit &Atom"), this, SLOT(edit_atom()));
   lookup_tables->insertItem(us_tr("Add/Edit &Residue"), this, SLOT(residue()));
   lookup_tables->insertItem(us_tr("Add/Edit &SAXS coefficients"), this, SLOT(do_saxs()));

 //   somo_options = new Q3PopupMenu;
   somo_options->insertItem(us_tr("&ASA Calculation"), this, SLOT(show_asa()));
   somo_options->insertItem(us_tr("&SoMo Overlap Reduction"), this, SLOT(show_overlap()));
   somo_options->insertItem(us_tr("AtoB (Grid) &Overlap Reduction"), this, SLOT(show_grid_overlap()));
   somo_options->insertItem(us_tr("&vdW Overlap Parameters"), this, SLOT(show_vdw_overlap()));
   somo_options->insertItem(us_tr("&Hydrodynamic Calculations"), this, SLOT(show_hydro()));
   somo_options->insertItem(us_tr("Hydrodynamic Calculations &Zeno"), this, SLOT(show_zeno_options()));
   somo_options->insertItem(us_tr("&Miscellaneous Options"), this, SLOT(show_misc()));
   somo_options->insertItem(us_tr("&Bead Model Output"), this, SLOT(show_bead_output()));
   somo_options->insertItem(us_tr("&Grid Functions (AtoB)"), this, SLOT(show_grid()));
   somo_options->insertItem(us_tr("SA&XS/SANS Options"), this, SLOT(show_saxs_options()));
   somo_options->insertItem(us_tr("&Fractal Dimension Options"), this, SLOT(show_fractal_dimension_options()));

 //   md_options = new Q3PopupMenu;
   // md_options->insertItem(us_tr("&DMD Options"), this, SLOT(show_dmd_options()));
   md_options->insertItem(us_tr("&Browflex Options"), this, SLOT(show_bd_options()));
   md_options->insertItem(us_tr("&Anaflex Options"), this, SLOT(show_anaflex_options()));

 //   pdb_options = new Q3PopupMenu;
   pdb_options->insertItem(us_tr("&Parsing"), this, SLOT(pdb_parsing()));
   pdb_options->insertItem(us_tr("&Visualization"), this, SLOT(pdb_visualization()));

 //   configuration = new Q3PopupMenu;
   configuration->insertItem(us_tr("&Load Configuration"), this, SLOT(load_config()));
   configuration->insertItem(us_tr("&Save Current Configuration"), this, SLOT(write_config()));
   configuration->insertItem(us_tr("&Reset to Default Configuration"), this, SLOT(reset()));
   configuration->insertItem(us_tr("&Advanced Configuration"), this, SLOT(show_advanced_config()));
   configuration->insertItem(us_tr("S&ystem Configuration"), this, SLOT(run_us_config()));
   // configuration->insertItem(us_tr("A&dministrator"), this, SLOT(run_us_admin()));

   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);

# if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   menu = new QMenuBar(frame);
# else
   menu = new QMenuBar( this );
# endif
   menu->setPalette( PALET_NORMAL );
   AUTFBACK( menu );
   menu->insertItem(us_tr("&Lookup Tables"), lookup_tables);
   menu->insertItem(us_tr("&SOMO"), somo_options);
   menu->insertItem(us_tr("&MD"), md_options);
   menu->insertItem(us_tr("&PDB"), pdb_options);
   menu->insertItem(us_tr("&Configuration"), configuration);

# if QT_VERSION >= 0x040000 && defined(Q_OS_MAC)
   {
 //      Q3PopupMenu * file = new Q3PopupMenu;
      file->insertItem( us_tr("&Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
      file->insertItem( us_tr("&Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );
#  ifndef NO_EDITOR_PRINT
      file->insertItem( us_tr("&Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
      file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
      menu->insertItem(us_tr("&Messages"), file );
   }
# endif
#else
   QFrame *frame;
   frame = new QFrame(this);
   frame->setMinimumHeight(minHeight1);
   frame->setPalette( PALET_NORMAL );
   AUTFBACK( frame );

# if !defined(Q_OS_MAC) || QT_VERSION > 0x050000
   menu = new QMenuBar(frame);
# else
   menu = new QMenuBar( this );
# endif
   {
      QMenu *submenu = new QMenu( us_tr("&Lookup Tables") );
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Hybridization") );
         connect( qa, SIGNAL(triggered()), this, SLOT( hybrid() ) );
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Atom") );
         connect( qa, SIGNAL(triggered()),  this, SLOT(edit_atom()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &Residue") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(residue()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Add/Edit &SAXS coefficients") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(do_saxs()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&SOMO") );
      {
         QAction *qa = submenu->addAction( us_tr("&ASA Calculation") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_asa()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&SoMo Overlap Reduction") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_overlap()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("AtoB (Grid) &Overlap Reduction") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_grid_overlap()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&vdW Overlap Parameters" ) );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_vdw_overlap()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Hydrodynamic Calculations") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_hydro()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("Hydrodynamic Calculations &Zeno") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_zeno_options()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Miscellaneous Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_misc()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Bead Model Output") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_bead_output()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Grid Functions (AtoB)") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_grid()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("SA&XS/SANS Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_saxs_options()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Fractal Dimension Options" ) );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_fractal_dimension_options()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&MD") );
      // {
      //    QAction *qa = submenu->addAction( us_tr("&DMD Options") );
      //    connect( qa, SIGNAL( triggered() ), this, SLOT(show_dmd_options()));
      // }
      {
         QAction *qa = submenu->addAction( us_tr("&Browflex Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_bd_options()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Anaflex Options") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_anaflex_options()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&PDB") );
      {
         QAction *qa = submenu->addAction( us_tr("&Parsing") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(pdb_parsing()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("&Visualization") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(pdb_visualization()));
      }
      menu->addMenu( submenu );
   }
   {
      QMenu *submenu = new QMenu( us_tr("&Configuration") );
      {
         QAction *qa = submenu->addAction( us_tr( "&Load Configuration" ) );
         connect( qa, SIGNAL( triggered() ), this, SLOT(load_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Save Current Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(write_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Reset to Default Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(reset()));
      }
      {
         QAction *qa = submenu->addAction( us_tr( "&Advanced Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(show_advanced_config()));
      }
      {
         QAction *qa = submenu->addAction( us_tr("S&ystem Configuration") );
         connect( qa, SIGNAL( triggered() ), this, SLOT(run_us_config()));
      }
      // {
      //    QAction *qa = submenu->addAction( us_tr("A&dministrator") );
      //    connect( qa, SIGNAL( triggered() ), this, SLOT(run_us_admin()));
      // }
      menu->addMenu( submenu );
   }
# if defined(Q_OS_MAC)
   {
#  if QT_VERSION >= 0x050000
      QMenu * submenu = new QMenu( us_tr( "&File" ) );
#  else
      QMenu * submenu = m->addMenu( us_tr( "&File" ) );
#  endif

      QAction *qa1 = submenu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = submenu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = submenu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );

      menu->addMenu( submenu );
   }
# endif
#endif

   lbl_info1 = new QLabel(us_tr("PDB Functions:"), this);
   Q_CHECK_PTR(lbl_info1);
   lbl_info1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info1->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info1->setMinimumHeight(minHeight1);
   lbl_info1->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info1 );
   lbl_info1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info2 = new QLabel(us_tr("Bead Model Functions:"), this);
   Q_CHECK_PTR(lbl_info2);
   lbl_info2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info2->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info2->setMinimumHeight(minHeight1);
   lbl_info2->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info2 );
   lbl_info2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   lbl_info3 = new QLabel(us_tr("Hydrodynamic Calculations:"), this);
   Q_CHECK_PTR(lbl_info3);
   lbl_info3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info3->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info3->setMinimumHeight(minHeight1);
   lbl_info3->setPalette( PALET_FRAME );
   AUTFBACK( lbl_info3 );
   lbl_info3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   pb_select_residue_file = new QPushButton(us_tr("Select Lookup Table"), this);
   Q_CHECK_PTR(pb_select_residue_file);
   pb_select_residue_file->setMinimumHeight(minHeight1);
   pb_select_residue_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_residue_file->setPalette( PALET_PUSHB );
   connect(pb_select_residue_file, SIGNAL(clicked()), SLOT(select_residue_file()));

   lbl_table = new QLabel( QDir::toNativeSeparators( residue_filename ), this );
   lbl_table->setMinimumHeight(minHeight1);
   lbl_table->setFrameStyle(QFrame::WinPanel|Sunken);
   lbl_table->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_table->setPalette( PALET_EDIT );
   AUTFBACK( lbl_table );
   lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

   pb_batch = new QPushButton(us_tr("Batch Mode/Cluster Operation"), this);
   Q_CHECK_PTR(pb_batch);
   pb_batch->setMinimumHeight(minHeight1);
   pb_batch->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch->setPalette( PALET_PUSHB );
   pb_batch->setAutoDefault( false );
   connect(pb_batch, SIGNAL(clicked()), SLOT(show_batch()));

   pb_view_bead_model = new QPushButton(us_tr("View Bead Model File"), this);
   Q_CHECK_PTR(pb_view_bead_model);
   pb_view_bead_model->setMinimumHeight(minHeight1);
   pb_view_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_bead_model->setPalette( PALET_PUSHB );
   connect(pb_view_bead_model, SIGNAL(clicked()), SLOT(view_bead_model()));

   lbl_temperature = new QLabel( us_tr( QString( " Temp. (%1C): " ).arg( DEGREE_SYMBOL ) ), this);
   Q_CHECK_PTR(lbl_temperature);
   lbl_temperature->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_temperature->setPalette( PALET_LABEL );
   AUTFBACK( lbl_temperature );
   lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   le_temperature = new QLineEdit( this );    le_temperature->setObjectName( "Temperature Line Edit" );
   le_temperature->setText(QString("").sprintf("%4.2f",hydro.temperature));
   le_temperature->setAlignment(Qt::AlignVCenter);
   le_temperature->setPalette( PALET_NORMAL );
   AUTFBACK( le_temperature );
   le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_temperature->setValidator( new QDoubleValidator( -273.15, 200, 2, le_temperature ) );
   le_temperature->setEnabled(true);
   connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature(const QString &)));

   cb_pH = new QCheckBox(this);
   cb_pH->setText(us_tr(" pH "));
   cb_pH->setChecked( gparams.count( "use_pH" ) && gparams[ "use_pH" ] == "true" );
   cb_pH->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_pH->setPalette( PALET_NORMAL );
   AUTFBACK( cb_pH );
   connect(cb_pH, SIGNAL(clicked()), this, SLOT(set_pH()));
   cb_pH->setToolTip( us_tr( "<html><body> enable pH dependent ionization and psv. You will need to reload the PDB if the pH is adjusted.</body></html>" ) );
   // eventually cb_pH->hide();

   le_pH = new QLineEdit( this );    le_pH->setObjectName( "PH Line Edit" );
   le_pH->setText(QString("").sprintf("%4.2f",hydro.pH));
   le_pH->setAlignment(Qt::AlignVCenter);
   le_pH->setPalette( PALET_NORMAL );
   AUTFBACK( le_pH );
   le_pH->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_pH->setValidator( new QDoubleValidator( 0.01, 14, 2, le_pH ) );
   le_pH->setEnabled( cb_pH->isChecked() );
   connect(le_pH, SIGNAL(textChanged(const QString &)), SLOT(update_pH(const QString &)));
   le_pH->setToolTip( us_tr( "<html><body>enter the pH for ionization and psv calculations. You will need to reload the PDB if the pH is adjusted.</body></html>" ) );

   pb_load_pdb = new QPushButton(us_tr("Load Single PDB File"), this);
   Q_CHECK_PTR(pb_load_pdb);
   pb_load_pdb->setMinimumHeight(minHeight1);
   pb_load_pdb->setEnabled(true);
   pb_load_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_pdb->setPalette( PALET_PUSHB );
   connect(pb_load_pdb, SIGNAL(clicked()), SLOT(load_pdb()));

   pb_reload_pdb = new QPushButton(us_tr("Reload"), this);
   Q_CHECK_PTR(pb_reload_pdb);
   pb_reload_pdb->setMinimumHeight(minHeight1);
   pb_reload_pdb->setEnabled(true);
   pb_reload_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reload_pdb->setPalette( PALET_PUSHB );
   connect(pb_reload_pdb, SIGNAL(clicked()), SLOT(reload_pdb()));
   pb_reload_pdb->setToolTip( us_tr( "<html><body>Use this to reload the PDB, primarily useful when the pH has been adjusted.</body></html>" ) );

   le_pdb_file = new mQLineEdit( this );
   le_pdb_file->setText( us_tr( "not selected" ) );
#if QT_VERSION < 0x040000
   le_pdb_file->setFrameStyle(QFrame::WinPanel|Sunken);
#endif
   le_pdb_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_pdb_file->setMinimumHeight(minHeight1);
   le_pdb_file->setPalette( PALET_EDIT );
   AUTFBACK( le_pdb_file );
   le_pdb_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect( le_pdb_file, SIGNAL( focussed( bool ) ), SLOT( le_pdb_file_focus( bool ) ) );
   connect( le_pdb_file, SIGNAL( textChanged( const QString & ) ), SLOT( le_pdb_file_changed( const QString & ) ) );

   lbl_model = new QLabel(us_tr(" Please select a PDB Structure:"), this);
   Q_CHECK_PTR(lbl_model);
   lbl_model->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_model->setMinimumHeight(minHeight1);
   lbl_model->setMargin( 2 );
   lbl_model->setPalette( PALET_LABEL );
   AUTFBACK( lbl_model );
   lbl_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   pb_view_pdb = new QPushButton(us_tr("View/Edit PDB File"), this);
   Q_CHECK_PTR(pb_view_pdb);
   pb_view_pdb->setMinimumHeight(minHeight1);
   pb_view_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_pdb->setPalette( PALET_PUSHB );
   connect(pb_view_pdb, SIGNAL(clicked()), SLOT(view_pdb()));

   pb_pdb_tool = new QPushButton(us_tr("PDB Editor"), this);
   pb_pdb_tool->setMinimumHeight(minHeight1);
   pb_pdb_tool->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_tool->setPalette( PALET_PUSHB );
   connect(pb_pdb_tool, SIGNAL(clicked()), SLOT(pdb_tool()));

   lb_model = new QListWidget( this );
   lb_model->setPalette( PALET_LISTB );
   lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //lb_model->setEnabled(false);
   lb_model->setEnabled( true );
   lb_model->setSelectionMode(QAbstractItemView::ExtendedSelection);
 //   lb_model->setHScrollBarMode(QScrollView::Auto);
 //   lb_model->setVScrollBarMode(QScrollView::Auto);
   // connect(lb_model, SIGNAL(currentRowChanged(int)), this, SLOT(select_model(int)));
   connect(lb_model, SIGNAL(itemSelectionChanged()), this, SLOT(model_selection_changed()));

   pb_load_bead_model = new QPushButton(us_tr("Load Single Bead Model File"), this);
   Q_CHECK_PTR(pb_load_bead_model);
   pb_load_bead_model->setMinimumHeight(minHeight1);
   pb_load_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_bead_model->setPalette( PALET_PUSHB );
   connect(pb_load_bead_model, SIGNAL(clicked()), SLOT(load_bead_model()));

   le_bead_model_file = new QLineEdit( this );    le_bead_model_file->setObjectName( "bead_model_file Line Edit" );
   le_bead_model_file->setText(us_tr(" not selected "));
   le_bead_model_file->setMinimumHeight(minHeight1);
   le_bead_model_file->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_file->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_file );
   le_bead_model_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_file, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_file(const QString &)));

   lbl_bead_model_prefix = new QLabel(us_tr(" Bead Model Suffix:"), this);
   Q_CHECK_PTR(lbl_bead_model_prefix);
   lbl_bead_model_prefix->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_bead_model_prefix->setMinimumHeight( minHeight1 * 2 );
   lbl_bead_model_prefix->setPalette( PALET_LABEL );
   AUTFBACK( lbl_bead_model_prefix );
   lbl_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cb_setSuffix = new QCheckBox(this);
   cb_setSuffix->setText(us_tr(" Add auto-generated suffix "));
   cb_setSuffix->setChecked(setSuffix);
   cb_setSuffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_setSuffix->setPalette( PALET_NORMAL );
   AUTFBACK( cb_setSuffix );
   connect(cb_setSuffix, SIGNAL(clicked()), this, SLOT(set_setSuffix()));

   cb_overwrite = new QCheckBox(this);
   cb_overwrite->setText(us_tr(" Overwrite existing filenames "));
   cb_overwrite->setChecked(overwrite);
   cb_overwrite->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_overwrite->setPalette( PALET_NORMAL );
   AUTFBACK( cb_overwrite );
   connect(cb_overwrite, SIGNAL(clicked()), this, SLOT(set_overwrite()));

   le_bead_model_prefix = new QLineEdit( this );    le_bead_model_prefix->setObjectName( "bead_model_prefix Line Edit" );
   le_bead_model_prefix->setText(us_tr(""));
   le_bead_model_prefix->setMinimumHeight( minHeight1 * 2 );
   le_bead_model_prefix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_prefix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_prefix );
   le_bead_model_prefix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   connect(le_bead_model_prefix, SIGNAL(textChanged(const QString &)), SLOT(update_bead_model_prefix(const QString &)));

   le_bead_model_suffix = new QTextEdit( this );    le_bead_model_suffix->setObjectName( "bead_model_suffix Line Edit" );
   le_bead_model_suffix->setText(us_tr(""));
   le_bead_model_suffix->setMinimumHeight( minHeight1 * 2 );
   le_bead_model_suffix->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   le_bead_model_suffix->setPalette( PALET_EDIT );
   AUTFBACK( le_bead_model_suffix );
   le_bead_model_suffix->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   le_bead_model_suffix->setReadOnly(true);
   le_bead_model_suffix->setFixedHeight( minHeight1 * 2 );
   
   pb_somo = new QPushButton(us_tr("Build SoMo Bead Model"), this);
   Q_CHECK_PTR(pb_somo);
   pb_somo->setMinimumHeight(minHeight1);
   pb_somo->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_somo->setEnabled(false);
   pb_somo->setPalette( PALET_PUSHB );
   connect(pb_somo, SIGNAL(clicked()), SLOT(calc_somo()));

   pb_somo_o = new QPushButton(us_tr("Build SoMo Overlap Bead Model"), this);
   Q_CHECK_PTR(pb_somo_o);
   pb_somo_o->setMinimumHeight(minHeight1);
   pb_somo_o->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   pb_somo_o->setEnabled(false);
   pb_somo_o->setPalette( PALET_PUSHB );
   connect(pb_somo_o, SIGNAL(clicked()), SLOT(calc_somo_o()));

#if defined(USE_H)
   pb_pdb_hydrate_for_saxs = new QPushButton(us_tr("Hydrate"), this);
   pb_pdb_hydrate_for_saxs->setMinimumHeight(minHeight1);
   pb_pdb_hydrate_for_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_hydrate_for_saxs->setEnabled(true);
   pb_pdb_hydrate_for_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_hydrate_for_saxs, SIGNAL(clicked()), SLOT(pdb_hydrate_for_saxs()));
#endif

   pb_pdb_saxs = new QPushButton(us_tr("SAXS/SANS/MALS Functions"), this);
   Q_CHECK_PTR(pb_pdb_saxs);
   pb_pdb_saxs->setMinimumHeight(minHeight1);
   pb_pdb_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pdb_saxs->setEnabled(true);
   pb_pdb_saxs->setPalette( PALET_PUSHB );
   connect(pb_pdb_saxs, SIGNAL(clicked()), SLOT(pdb_saxs()));

   pb_bead_saxs = new QPushButton(us_tr("SAXS/SANS/MALS Functions"), this);
   Q_CHECK_PTR(pb_bead_saxs);
   pb_bead_saxs->setMinimumHeight(minHeight1);
   pb_bead_saxs->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_bead_saxs->setEnabled(false);
   pb_bead_saxs->setPalette( PALET_PUSHB );
   connect(pb_bead_saxs, SIGNAL(clicked()), SLOT(bead_saxs()));

   pb_rescale_bead_model = new QPushButton(us_tr("Rescale/Equalize Bead Model"), this);
   pb_rescale_bead_model->setMinimumHeight(minHeight1);
   pb_rescale_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_rescale_bead_model->setEnabled(false);
   pb_rescale_bead_model->setPalette( PALET_PUSHB );
   connect(pb_rescale_bead_model, SIGNAL(clicked()), SLOT(rescale_bead_model()));

   pb_equi_grid_bead_model = new QPushButton(us_tr("Simple grid"), this);
   pb_equi_grid_bead_model->setMinimumHeight(minHeight1);
   pb_equi_grid_bead_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_equi_grid_bead_model->setEnabled(false);
   pb_equi_grid_bead_model->setPalette( PALET_PUSHB );
   connect(pb_equi_grid_bead_model, SIGNAL(clicked()), SLOT(equi_grid_bead_model()));

   pb_grid_pdb = new QPushButton(us_tr("Build AtoB (Grid) Bead Model"), this);
   Q_CHECK_PTR(pb_grid_pdb);
   pb_grid_pdb->setMinimumHeight(minHeight1);
   pb_grid_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid_pdb->setEnabled(false);
   pb_grid_pdb->setPalette( PALET_PUSHB );
   connect(pb_grid_pdb, SIGNAL(clicked()), SLOT(calc_grid_pdb()));

   // pb_grid_pdb_o = new QPushButton(us_tr("Build AtoB (Grid) Overlap Bead Model"), this);
   // pb_grid_pdb_o->setMinimumHeight(minHeight1);
   // pb_grid_pdb_o->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   // pb_grid_pdb_o->setEnabled(false);
   // pb_grid_pdb_o->setPalette( PALET_PUSHB );
   // connect(pb_grid_pdb_o, SIGNAL(clicked()), SLOT(calc_grid_pdb_o()));

   pb_vdw_beads = new QPushButton(us_tr("Build vdW Overlap Bead Model"), this);
   pb_vdw_beads->setMinimumHeight(minHeight1);
   pb_vdw_beads->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_vdw_beads->setEnabled(false);
   pb_vdw_beads->setPalette( PALET_PUSHB );
   connect(pb_vdw_beads, SIGNAL(clicked()), SLOT(calc_vdw_beads()));

   pb_grid = new QPushButton(us_tr("Grid Existing Bead Model"), this);
   Q_CHECK_PTR(pb_grid);
   pb_grid->setMinimumHeight(minHeight1);
   pb_grid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_grid->setEnabled(false);
   pb_grid->setPalette( PALET_PUSHB );
   connect(pb_grid, SIGNAL(clicked()), SLOT(calc_grid()));

   cb_calcAutoHydro = new QCheckBox(this);
   cb_calcAutoHydro->setText(us_tr(" Automatically Calculate Hydrodynamics "));
   cb_calcAutoHydro->setChecked(calcAutoHydro);
   cb_calcAutoHydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_calcAutoHydro->setPalette( PALET_NORMAL );
   AUTFBACK( cb_calcAutoHydro );
   connect(cb_calcAutoHydro, SIGNAL(clicked()), this, SLOT(set_calcAutoHydro()));

   pb_view_asa = new QPushButton(us_tr("View ASA Results"), this);
   Q_CHECK_PTR(pb_view_asa);
   pb_view_asa->setMinimumHeight(minHeight1);
   pb_view_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_view_asa->setPalette( PALET_PUSHB );
   connect(pb_view_asa, SIGNAL(clicked()), SLOT(view_asa()));

   pb_visualize = new QPushButton(us_tr("Visualize Bead Model"), this);
   Q_CHECK_PTR(pb_visualize);
   pb_visualize->setMinimumHeight(minHeight1);
   pb_visualize->setEnabled(false);
   pb_visualize->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_visualize->setPalette( PALET_PUSHB );
   connect(pb_visualize, SIGNAL(clicked()), SLOT(visualize()));

   pb_batch2 = new QPushButton(us_tr("Batch Mode/Cluster Operation"), this);
   Q_CHECK_PTR(pb_batch2);
   pb_batch2->setMinimumHeight(minHeight1);
   pb_batch2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_batch2->setPalette( PALET_PUSHB );
   connect(pb_batch2, SIGNAL(clicked()), SLOT(show_batch()));

   lbl_rbh = new QLabel(us_tr(" Calculate RB Hydrodynamics:"), this);
   Q_CHECK_PTR(lbl_rbh);
   lbl_rbh->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_rbh->setMinimumHeight(minHeight1);
   lbl_rbh->setMargin( 2 );
   lbl_rbh->setPalette( PALET_LABEL );
   AUTFBACK( lbl_rbh );
   lbl_rbh->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   pb_calc_hydro = new QPushButton(us_tr("SMI"), this);
   Q_CHECK_PTR(pb_calc_hydro);
   pb_calc_hydro->setEnabled(false);
   pb_calc_hydro->setMinimumHeight(minHeight1);
   pb_calc_hydro->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_hydro->setPalette( PALET_PUSHB );
   connect(pb_calc_hydro, SIGNAL(clicked()), SLOT(calc_hydro()));

   pb_calc_zeno = new QPushButton(us_tr("ZENO"), this);
   Q_CHECK_PTR(pb_calc_zeno);
   pb_calc_zeno->setEnabled(false);
   pb_calc_zeno->setMinimumHeight(minHeight1);
   pb_calc_zeno->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_zeno->setPalette( PALET_PUSHB );
   connect(pb_calc_zeno, SIGNAL(clicked()), SLOT(calc_zeno_hydro()));

   pb_calc_grpy = new QPushButton(us_tr("GRPY"), this);
   Q_CHECK_PTR(pb_calc_grpy);
   pb_calc_grpy->setEnabled(false);
   pb_calc_grpy->setMinimumHeight(minHeight1);
   pb_calc_grpy->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_grpy->setPalette( PALET_PUSHB );
   connect(pb_calc_grpy, SIGNAL(clicked()), SLOT(calc_grpy_hydro()));

   pb_calc_hullrad = new QPushButton(us_tr("Hullrad"), this);
   Q_CHECK_PTR(pb_calc_hullrad);
   pb_calc_hullrad->setEnabled(true);
   pb_calc_hullrad->setMinimumHeight(minHeight1);
   pb_calc_hullrad->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calc_hullrad->setPalette( PALET_PUSHB );
   connect(pb_calc_hullrad, SIGNAL(clicked()), SLOT(calc_hullrad_hydro()));
   pb_calc_hullrad->hide();

   pb_show_hydro_results = new QPushButton(us_tr("Show Hydrodynamic Calculations"), this);
   Q_CHECK_PTR(pb_show_hydro_results);
   pb_show_hydro_results->setMinimumHeight(minHeight1);
   pb_show_hydro_results->setEnabled(false);
   pb_show_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_show_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_show_hydro_results, SIGNAL(clicked()), SLOT(show_hydro_results()));

   pb_comparative = new QPushButton(us_tr("Model classifier"), this);
   pb_comparative->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_comparative->setEnabled(true);
   pb_comparative->setPalette( PALET_PUSHB );
   connect(pb_comparative, SIGNAL(clicked()), SLOT(select_comparative()));

   pb_best = new QPushButton(us_tr("BEST"), this);
   pb_best->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_best->setEnabled(true);
   pb_best->setPalette( PALET_PUSHB );
   connect(pb_best, SIGNAL(clicked()), SLOT( best_analysis()));
   // pb_best->hide();

   pb_open_hydro_results = new QPushButton(us_tr("Open Hydrodynamic Calculations File"), this);
   Q_CHECK_PTR(pb_open_hydro_results);
   pb_open_hydro_results->setMinimumHeight(minHeight1);
   pb_open_hydro_results->setEnabled(true);
   pb_open_hydro_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_open_hydro_results->setPalette( PALET_PUSHB );
   connect(pb_open_hydro_results, SIGNAL(clicked()), SLOT(open_hydro_results()));

   pb_select_save_params = new QPushButton(us_tr("Select Parameters to be Saved"), this);
   Q_CHECK_PTR(pb_select_save_params);
   pb_select_save_params->setMinimumHeight(minHeight1);
   pb_select_save_params->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select_save_params->setEnabled(true);
   pb_select_save_params->setPalette( PALET_PUSHB );
   connect(pb_select_save_params, SIGNAL(clicked()), SLOT(select_save_params()));

   cb_saveParams = new QCheckBox(this);
   cb_saveParams->setText(us_tr(" Save parameters to file "));
   cb_saveParams->setChecked(saveParams);
   cb_saveParams->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_saveParams->setPalette( PALET_NORMAL );
   AUTFBACK( cb_saveParams );
   connect(cb_saveParams, SIGNAL(clicked()), this, SLOT(set_saveParams()));

   // ***** dmd *******
   pb_dmd_run = new QPushButton(us_tr("Run DMD"), this);
   pb_dmd_run->setMinimumHeight(minHeight1);
   pb_dmd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_dmd_run->setEnabled( true );
   pb_dmd_run->setPalette( PALET_PUSHB );
   connect(pb_dmd_run, SIGNAL(clicked()), SLOT(dmd_run()));

   pb_bd = new QPushButton(us_tr("BD"), this);
   Q_CHECK_PTR(pb_bd);
   pb_bd->setMinimumHeight(minHeight1);
   pb_bd->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
#ifdef NO_BD
   pb_bd->setEnabled(false);
#else
   pb_bd->setEnabled(true);
#endif
   pb_bd->setPalette( PALET_PUSHB );
   connect(pb_bd, SIGNAL(clicked()), SLOT(show_bd()));

   pb_fractal_dimension = new QPushButton(us_tr("FD"), this);
   Q_CHECK_PTR(pb_fractal_dimension);
   pb_fractal_dimension->setMinimumHeight(minHeight1);
   pb_fractal_dimension->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_fractal_dimension->setPalette( PALET_PUSHB );
   connect(pb_fractal_dimension, SIGNAL(clicked()), SLOT(fractal_dimension()));
   pb_fractal_dimension->hide();
   
   // ***** bd *******
   //   pb_bd_prepare = new QPushButton(us_tr("Create Browflex files"), this);
   //   pb_bd_prepare->setMinimumHeight(minHeight1);
   //   pb_bd_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_prepare->setEnabled(false);
   //   pb_bd_prepare->setPalette( PALET_PUSHB );
   //   connect(pb_bd_prepare, SIGNAL(clicked()), SLOT(bd_prepare()));

   //   pb_bd_load = new QPushButton(us_tr("Load Browflex files"), this);
   //   pb_bd_load->setMinimumHeight(minHeight1);
   //   pb_bd_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load->setEnabled(false);
   //   pb_bd_load->setPalette( PALET_PUSHB );
   //   connect(pb_bd_load, SIGNAL(clicked()), SLOT(bd_load()));

   //   pb_bd_edit = new QPushButton(us_tr("View/Edit Browflex files"), this);
   //   pb_bd_edit->setMinimumHeight(minHeight1);
   //   pb_bd_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_edit->setEnabled(false);
   //   pb_bd_edit->setPalette( PALET_PUSHB );
   //   connect(pb_bd_edit, SIGNAL(clicked()), SLOT(bd_edit()));

   //   pb_bd_run = new QPushButton(us_tr("Run Browflex"), this);
   //   pb_bd_run->setMinimumHeight(minHeight1);
   //   pb_bd_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_run->setEnabled(false);
   //   pb_bd_run->setPalette( PALET_PUSHB );
   //   connect(pb_bd_run, SIGNAL(clicked()), SLOT(bd_run()));

   //   pb_bd_load_results = new QPushButton(us_tr("Load/Process Browflex results"), this);
   //   pb_bd_load_results->setMinimumHeight(minHeight1);
   //   pb_bd_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_bd_load_results->setEnabled(false);
   //   pb_bd_load_results->setPalette( PALET_PUSHB );
   //   connect(pb_bd_load_results, SIGNAL(clicked()), SLOT(bd_load_results()));

   // ***** anaflex *******
   //   pb_anaflex_prepare = new QPushButton(us_tr("Create Anaflex files"), this);
   //   pb_anaflex_prepare->setMinimumHeight(minHeight1);
   //   pb_anaflex_prepare->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_prepare->setEnabled(false);
   //   pb_anaflex_prepare->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_prepare, SIGNAL(clicked()), SLOT(anaflex_prepare()));

   //   pb_anaflex_load = new QPushButton(us_tr("Load Anaflex files"), this);
   //   pb_anaflex_load->setMinimumHeight(minHeight1);
   //   pb_anaflex_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load->setEnabled(false);
   //   pb_anaflex_load->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_load, SIGNAL(clicked()), SLOT(anaflex_load()));

   //   pb_anaflex_edit = new QPushButton(us_tr("View/Edit Anaflex files"), this);
   //   pb_anaflex_edit->setMinimumHeight(minHeight1);
   //   pb_anaflex_edit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_edit->setEnabled(false);
   //   pb_anaflex_edit->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_edit, SIGNAL(clicked()), SLOT(anaflex_edit()));

   //   pb_anaflex_run = new QPushButton(us_tr("Run Anaflex"), this);
   //   pb_anaflex_run->setMinimumHeight(minHeight1);
   //   pb_anaflex_run->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_run->setEnabled(false);
   //   pb_anaflex_run->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_run, SIGNAL(clicked()), SLOT(anaflex_run()));

   //   pb_anaflex_load_results = new QPushButton(us_tr("Load Anaflex results"), this);
   //   pb_anaflex_load_results->setMinimumHeight(minHeight1);
   //   pb_anaflex_load_results->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   //   pb_anaflex_load_results->setEnabled(false);
   //   pb_anaflex_load_results->setPalette( PALET_PUSHB );
   //   connect(pb_anaflex_load_results, SIGNAL(clicked()), SLOT(anaflex_load_results()));

   pb_help = new QPushButton(us_tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));


   pb_config = new QPushButton(us_tr("Config"), this);
   pb_config->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_config->setMinimumHeight(minHeight1);
   pb_config->setPalette( PALET_PUSHB );
   connect(pb_config, SIGNAL(clicked()), SLOT(config()));

   pb_stop_calc = new QPushButton(us_tr("Stop"), this);
   Q_CHECK_PTR(pb_stop_calc);
   pb_stop_calc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_stop_calc->setMinimumHeight(minHeight1);
   pb_stop_calc->setPalette( PALET_PUSHB );
   connect(pb_stop_calc, SIGNAL(clicked()), SLOT(stop_calc()));
   pb_stop_calc->setEnabled(false);

   pb_cancel = new QPushButton(us_tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   progress = new mQProgressBar( this );
   progress->setPalette( PALET_NORMAL );
   progress->set_cli_progress( cli_progress );
   AUTFBACK( progress );
   progress->reset();

   mprogress = new mQProgressBar( this );
   mprogress->setPalette( PALET_NORMAL );
   mprogress->set_cli_progress( cli_progress );
   AUTFBACK( mprogress );
   mprogress->reset();
   mprogress->hide();

   editor = new mQTextEdit(this);
   editor->setPalette( PALET_NORMAL );
   editor->set_cli_progress( cli_progress );
   editor->set_cli_prefix  ( "somo" );
   editor->setReadOnly(true);
   editor->setMinimumWidth(600);

#if QT_VERSION < 0x040000
# if QT_VERSION < 0x040000 || !defined(Q_OS_MAC)
   m = new QMenuBar( editor );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1);
   m->setPalette( PALET_NORMAL );
   AUTFBACK( m );
 //   Q3PopupMenu * file = new Q3PopupMenu(editor);
   m->insertItem( us_tr("&File"), file );
   file->insertItem( us_tr("Font"),  this, SLOT(update_font()),    Qt::ALT+Qt::Key_F );
   file->insertItem( us_tr("Save"),  this, SLOT(save()),    Qt::ALT+Qt::Key_S );

#  ifndef NO_EDITOR_PRINT
   file->insertItem( us_tr("Print"), this, SLOT(print()),   Qt::ALT+Qt::Key_P );
#  endif
   file->insertItem( us_tr("Clear Display"), this, SLOT(clear_display()),   Qt::ALT+Qt::Key_X );
# endif
#else
# if !defined(Q_OS_MAC)
   m = new QMenuBar( editor );    m->setObjectName( "menu" );
   m->setMinimumHeight(minHeight1);
   {
      QMenu * new_menu = m->addMenu( us_tr( "&File" ) );

      QAction *qa1 = new_menu->addAction( us_tr( "Font" ) );
      qa1->setShortcut( Qt::ALT+Qt::Key_F );
      connect( qa1, SIGNAL(triggered()), this, SLOT( update_font() ) );

      QAction *qa2 = new_menu->addAction( us_tr( "Save" ) );
      qa2->setShortcut( Qt::ALT+Qt::Key_S );
      connect( qa2, SIGNAL(triggered()), this, SLOT( save() ) );

      QAction *qa3 = new_menu->addAction( us_tr( "Clear Display" ) );
      qa3->setShortcut( Qt::ALT+Qt::Key_X );
      connect( qa3, SIGNAL(triggered()), this, SLOT( clear_display() ) );
   }
# endif
#endif
   
   editor->setWordWrapMode (advanced_config.scroll_editor ? QTextOption::NoWrap : QTextOption::WordWrap);

   lbl_core_progress = new QLabel("", this);
   Q_CHECK_PTR(lbl_core_progress);
   lbl_core_progress->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
   lbl_core_progress->setMinimumHeight(minHeight1);
   lbl_core_progress->setPalette( PALET_LABEL );
   AUTFBACK( lbl_core_progress );
   lbl_core_progress->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize+1, QFont::Bold));

   clear_display();

   int /* rows=20, columns = 3, */ spacing = 2, j=0, margin=4;
   QGridLayout * background = new QGridLayout( this ); background->setContentsMargins( 0, 0, 0, 0 ); background->setSpacing( 0 ); background->setSpacing( spacing ); background->setContentsMargins( margin, margin, margin, margin );

   background->addWidget( frame , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   background->addWidget( editor , j , 2 , 1 + ( j+24 ) - ( j ) , 1 + ( 2 ) - ( 2 ) );
   j++;
   background->addWidget( lbl_info1 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   background->addWidget(pb_select_residue_file, j, 0);
   background->addWidget(lbl_table, j, 1);
   j++;
   background->addWidget(pb_batch, j, 0);
   {
      QHBoxLayout *hbl = new QHBoxLayout();
      hbl->addWidget( lbl_temperature );
      hbl->addWidget( le_temperature );
      hbl->addWidget( cb_pH );
      hbl->addWidget( le_pH );
      background->addLayout( hbl, j, 1 );
   }
   j++;
   {
      QHBoxLayout *hbl = new QHBoxLayout();
      hbl->addWidget( pb_load_pdb );
      hbl->addWidget( pb_reload_pdb );
      background->addLayout( hbl, j, 0);
   }
   background->addWidget(le_pdb_file, j, 1);
   j++;
   background->addWidget(lbl_model, j, 0);
   background->addWidget( lb_model , j , 1 , 1 + ( j+4 ) - ( j ) , 1 + ( 1 ) - ( 1 ) );
   j++;
   QHBoxLayout * hbl_pdb = new QHBoxLayout(); hbl_pdb->setContentsMargins( 0, 0, 0, 0 ); hbl_pdb->setSpacing( 0 );
   hbl_pdb->addWidget(pb_view_pdb);
   hbl_pdb->addWidget(pb_pdb_tool);
   background->addLayout(hbl_pdb, j, 0);
   j++;
   QHBoxLayout * hbl_pdb_saxs = new QHBoxLayout; hbl_pdb_saxs->setContentsMargins( 0, 0, 0, 0 ); hbl_pdb_saxs->setSpacing( 0 );
   hbl_pdb_saxs->addWidget(pb_pdb_saxs);
#if defined(USE_H)
   hbl_pdb_saxs->addWidget(pb_pdb_hydrate_for_saxs);
#endif
   background->addLayout(hbl_pdb_saxs, j, 0);
   j++;
   background->addWidget(pb_dmd_run, j, 0);
   j++;
   {
      QHBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_fractal_dimension );
      hbl->addWidget( pb_bd );
      background->addLayout(hbl, j, 0);
   }
   // background->addWidget(pb_bd, j, 0);
   j++;
   background->addWidget( lbl_info2 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;
   //   background->addWidget(lbl_bead_model_prefix, j, 0);
   QHBoxLayout * hbl_prefix_suffix = new QHBoxLayout; hbl_prefix_suffix->setContentsMargins( 0, 0, 0, 0 ); hbl_prefix_suffix->setSpacing( 0 );
   hbl_prefix_suffix->addWidget(lbl_bead_model_prefix);
   hbl_prefix_suffix->addWidget(le_bead_model_prefix);
   // hbl_prefix_suffix->addWidget(le_bead_model_suffix);
   background->addLayout(hbl_prefix_suffix, j, 0);
   background->addWidget(le_bead_model_suffix, j, 1);
   // QGridLayout * gl_prefix_suffix = new QGridLayout; gl_prefix_suffix->setContentsMargins( 0, 0, 0, 0 ); gl_prefix_suffix->setSpacing( 0 );
   // gl_prefix_suffix->addWidget( le_bead_model_prefix , 0 , 0 , 1 + ( 0 ) - ( 0 ) , 1 + ( 0 ) - ( 0 ) );
   // gl_prefix_suffix->addWidget( le_bead_model_suffix , 0 , 1 , 1 + ( 0 ) - ( 0 ) , 1 + ( 2 ) - ( 1 ) );
   // background->addLayout(gl_prefix_suffix, j, 1);   
   j++;
   background->addWidget(cb_overwrite, j, 0);
   background->addWidget(cb_setSuffix, j, 1);
   j++;
   background->addWidget(pb_somo, j, 0);
   background->addWidget(pb_grid_pdb, j, 1);
   j++;

   background->addWidget(pb_somo_o, j, 0);
   // background->addWidget(pb_grid_pdb_o, j, 1);
   background->addWidget(pb_vdw_beads, j, 1);
   j++;


   //   background->addWidget(pb_bd_prepare, j, 0);
   //   QHBoxLayout * qhl_bd_1 = new QHBoxLayout; qhl_bd_1->setContentsMargins( 0, 0, 0, 0 ); qhl_bd_1->setSpacing( 0 );
   //   qhl_bd_1->addWidget(pb_bd_load);
   //   qhl_bd_1->addWidget(pb_bd_edit);
   //   background->addLayout(qhl_bd_1, j, 1);
   //   j++;
   //   background->addWidget(pb_bd_run, j, 0);
   //   background->addWidget(pb_bd_load_results, j, 1);
   //   j++;
   background->addWidget(pb_view_asa, j, 0);
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_grid );
      hbl->addWidget( pb_visualize );
      background->addLayout( hbl, j, 1 );
   }
   j++;

   background->addWidget(pb_batch2, j, 0);
   background->addWidget(pb_view_bead_model, j, 1);
   j++;

   background->addWidget(pb_load_bead_model, j, 0);
   background->addWidget(le_bead_model_file, j, 1);
   j++;

   background->addWidget(pb_bead_saxs, j, 0);
   background->addWidget(cb_calcAutoHydro, j, 1);
   j++;

   background->addWidget(pb_rescale_bead_model, j, 0);
   background->addWidget(pb_equi_grid_bead_model, j, 1);
   j++;

   background->addWidget( lbl_info3 , j , 0 , 1 + ( j ) - ( j ) , 1 + ( 1 ) - ( 0 ) );
   j++;

   background->addWidget(lbl_rbh, j, 0);
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_calc_hydro);
      hbl->addWidget( pb_calc_zeno);
      hbl->addWidget( pb_calc_grpy);
      hbl->addWidget( pb_calc_hullrad );
      background->addLayout(hbl, j, 1);
   }
   j++;

   background->addWidget(pb_show_hydro_results, j, 0);
   background->addWidget(pb_open_hydro_results, j, 1);
   j++;

   background->addWidget(pb_select_save_params, j, 0);
   background->addWidget(cb_saveParams, j, 1);
   j++;

   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_best );
      hbl->addWidget( pb_comparative );
      background->addLayout( hbl, j, 0 );
   }
   {
      QBoxLayout * hbl = new QHBoxLayout(); hbl->setContentsMargins( 0, 0, 0, 0 ); hbl->setSpacing( 0 );
      hbl->addWidget( pb_stop_calc );
      hbl->addWidget( pb_cancel );
      background->addLayout( hbl, j, 1 );
   }
   j++;
   QBoxLayout * bl_help_config = new QHBoxLayout; bl_help_config->setContentsMargins( 0, 0, 0, 0 ); bl_help_config->setSpacing( 0 );
   bl_help_config->addWidget( pb_help );
   bl_help_config->addWidget( pb_config );

   background->addLayout( bl_help_config, j, 0);
   {
      QBoxLayout * bl_progress = new QHBoxLayout; bl_progress->setContentsMargins( 0, 0, 0, 0 ); bl_progress->setSpacing( 0 );
      bl_progress->addWidget( progress );
      bl_progress->addWidget( mprogress );
      background->addLayout( bl_progress, j, 1 );
   }
   //   background->addWidget(progress, j, 1);
   background->addWidget(lbl_core_progress, j, 2);

   fixWinButtons( this );

   connect( this, SIGNAL( progress_updated( int, int ) ), SLOT( update_progress( int, int ) ) );
}

void US_Hydrodyn::set_expert( bool expert )
{
   if ( expert )
   {
      //      pb_best->show();
#if QT_VERSION < 0x040000
      lookup_tables->insertItem(us_tr("Make test set"), this, SLOT( make_test_set() ) );
#endif
   }
   expert ? pb_equi_grid_bead_model->show() : pb_equi_grid_bead_model->hide();
   expert ? pb_rescale_bead_model->show() : pb_rescale_bead_model->hide();
}

void US_Hydrodyn::set_disabled( bool clear_bead_model_file )
{
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   pb_visualize->setEnabled(false);
   //   pb_pdb_saxs->setEnabled(false);

   bd_anaflex_enables(false);

   pb_bead_saxs->setEnabled(false);
   if ( clear_bead_model_file ) {
      le_bead_model_file->setText(" not selected ");
   }
   pb_rescale_bead_model->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
}

void US_Hydrodyn::set_enabled()
{
   qDebug() << "set_enabled() bead model from file : " << ( bead_model_from_file ? "true" : "false" );

   pb_somo->setEnabled( !bead_model_from_file );
   pb_somo_o->setEnabled( !bead_model_from_file );
   pb_grid_pdb->setEnabled( !bead_model_from_file );
   pb_vdw_beads->setEnabled( !bead_model_from_file );
   pb_grid->setEnabled( true );
   pb_visualize->setEnabled( true );

   // needs more work
   // pb_show_hydro_results->setEnabled( false);
   // pb_calc_hydro->setEnabled(false);
   // pb_calc_zeno->setEnabled(false);
   // pb_calc_grpy->setEnabled(false);
   // pb_calc_hullrad->setEnabled(false);
   // pb_visualize->setEnabled(false);
   // //   pb_pdb_saxs->setEnabled(false);

   // bd_anaflex_enables(false);

   // pb_bead_saxs->setEnabled(false);
   // le_bead_model_file->setText(" not selected ");
   // pb_rescale_bead_model->setEnabled(false);
   // pb_equi_grid_bead_model->setEnabled(false);
}

void US_Hydrodyn::hybrid()
{
   if (hybrid_widget)
   {
      addHybrid->raise();
   }
   else
   {
      addHybrid = new US_AddHybridization(&hybrid_widget, 0);
      fixWinButtons( addHybrid );
      addHybrid->show();
   }
}

void US_Hydrodyn::edit_atom()
{
   if (atom_widget)
   {
      addAtom->raise();
   }
   else
   {
      addAtom = new US_AddAtom(&atom_widget, 0);
      fixWinButtons( addAtom );
      addAtom->show();
   }
}

void US_Hydrodyn::residue()
{
   if (residue_widget)
   {
      addResidue->raise();
   }
   else
   {
      addResidue = new US_AddResidue(&residue_widget, misc.hydrovol, 0);
      fixWinButtons( addResidue );
      addResidue->show();
   }
}

void US_Hydrodyn::do_saxs()
{
   if (saxs_widget)
   {
      addSaxs->raise();
   }
   else
   {
      addSaxs = new US_AddSaxs(&saxs_widget, 0);
      fixWinButtons( addSaxs );
      addSaxs->show();
   }
}

void US_Hydrodyn::show_asa()
{
   if (asa_widget)
   {
      if (asa_window->isVisible())
      {
         asa_window->raise();
      }
      else
      {
         asa_window->show();
      }
      return;
   }
   else
   {
      asa_window = new US_Hydrodyn_ASA(&asa, &asa_widget, this);
      fixWinButtons( asa_window );
      asa_window->show();
   }
}

void US_Hydrodyn::show_overlap()
{
   if (overlap_widget)
   {
      if (overlap_window->isVisible())
      {
         overlap_window->raise();
      }
      else
      {
         overlap_window->show();
      }
      return;
   }
   else
   {
      overlap_window = new US_Hydrodyn_Overlap(&sidechain_overlap,
                                               &mainchain_overlap,
                                               &buried_overlap,
                                               &replicate_o_r_method_somo,
                                               &grid_exposed_overlap,
                                               &grid_buried_overlap,
                                               &grid_overlap,
                                               &replicate_o_r_method_grid,
                                               &overlap_tolerance,
                                               &overlap_widget,
                                               this);
      fixWinButtons( overlap_window );
      overlap_window->show();
   }
}

void US_Hydrodyn::show_grid_overlap()
{
   if (grid_overlap_widget)
   {
      if (grid_overlap_window->isVisible())
      {
         grid_overlap_window->raise();
      }
      else
      {
         grid_overlap_window->show();
      }
      return;
   }
   else
   {
      grid_overlap_window = new US_Hydrodyn_Overlap(&grid_exposed_overlap,
                                                    &grid_buried_overlap,
                                                    &grid_overlap,
                                                    &replicate_o_r_method_grid,
                                                    &overlap_tolerance,
                                                    &grid_overlap_widget,
                                                    this);
      fixWinButtons( grid_overlap_window );
      grid_overlap_window->show();
   }
}

void US_Hydrodyn::show_hydro()
{
   if (hydro_widget)
   {
      if (hydro_window->isVisible())
      {
         hydro_window->raise();
      }
      else
      {
         hydro_window->show();
      }
      return;
   }
   else
   {
      hydro_window = new US_Hydrodyn_Hydro(&hydro, &hydro_widget, this);
      fixWinButtons( hydro_window );
      hydro_window->show();
   }
}

void US_Hydrodyn::show_misc()
{
   if (misc_widget)
   {
      if (misc_window->isVisible())
      {
         misc_window->raise();
      }
      else
      {
         misc_window->show();
      }
      return;
   }
   else
   {
      if ( misc.restore_pb_rule ) {
         // us_qdebug( "show_misc() restoring pb rule" );
         misc.pb_rule_on      = true;
         misc.restore_pb_rule = false;
      }

      misc_window = new US_Hydrodyn_Misc(&misc, &misc_widget, this);
      fixWinButtons( misc_window );
      connect(misc_window, SIGNAL(vbar_changed()), this, SLOT(update_vbar()));
      misc_window->show();
   }
   update_vbar();
}

void US_Hydrodyn::show_vdw_overlap() {
   if ( vdw_overlap_widget ) {
      if (vdw_overlap_window->isVisible()) {
         vdw_overlap_window->raise();
      } else {
         vdw_overlap_window->show();
      }
      return;
   } else {
      vdw_overlap_window = new US_Hydrodyn_Vdw_Overlap( &misc, &vdw_overlap_widget, this );
      fixWinButtons( vdw_overlap_window );
      vdw_overlap_window->show();
   }
   update_vbar();
}


void US_Hydrodyn::show_saxs_options()
{
   if (saxs_options_widget)
   {
      if (saxs_options_window->isVisible())
      {
         saxs_options_window->raise();
      }
      else
      {
         saxs_options_window->show();
      }
      return;
   }
   else
   {
      saxs_options_window = new US_Hydrodyn_SaxsOptions( &saxs_options, 
                                                         &saxs_options_widget, 
                                                         &sas_options_saxs_widget,
                                                         &sas_options_sans_widget,
                                                         &sas_options_curve_widget,
                                                         &sas_options_bead_model_widget,
                                                         &sas_options_hydration_widget,
                                                         &sas_options_guinier_widget,
                                                         &sas_options_xsr_widget,
                                                         &sas_options_misc_widget,
                                                         &sas_options_experimental_widget,
                                                         this );
      fixWinButtons( saxs_options_window );
      saxs_options_window->show();
   }
}

void US_Hydrodyn::show_dmd_options()
{
   if (dmd_options_widget)
   {
      if (dmd_options_window->isVisible())
      {
         dmd_options_window->raise();
      }
      else
      {
         dmd_options_window->show();
      }
      return;
   }
   else
   {
      dmd_options_window = new US_Hydrodyn_DMD_Options(&dmd_options, &dmd_options_widget, this);
      fixWinButtons( dmd_options_window );
      dmd_options_window->show();
   }
}

void US_Hydrodyn::show_bd()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif

   if (bd_widget)
   {
      if (bd_window->isVisible())
      {
         bd_window->raise();
      }
      else
      {
         bd_window->show();
      }
      return;
   }
   else
   {
      bd_window = new US_Hydrodyn_BD(&bd_widget, &bd_options, &anaflex_options, this);
      fixWinButtons( bd_window );
      bd_anaflex_enables(false);
      bd_window->show();
   }
}

void US_Hydrodyn::show_bd_options()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif
   if (bd_options_widget)
   {
      if (bd_options_window->isVisible())
      {
         bd_options_window->raise();
      }
      else
      {
         bd_options_window->show();
      }
      return;
   }
   else
   {
      bd_options_window = new US_Hydrodyn_BD_Options(&bd_options, &bd_options_widget, this);
      fixWinButtons( bd_options_window );
      bd_options_window->show();
   }
}

void US_Hydrodyn::show_anaflex_options()
{
#ifdef NO_BD
   US_Static::us_message(us_tr("Please note:"),
                        us_tr("Function not available in this version." ) );
   return;
#endif
   if (anaflex_options_widget)
   {
      if (anaflex_options_window->isVisible())
      {
         anaflex_options_window->raise();
      }
      else
      {
         anaflex_options_window->show();
      }
      return;
   }
   else
   {
      anaflex_options_window = new US_Hydrodyn_Anaflex_Options(&anaflex_options, &anaflex_options_widget, this);
      fixWinButtons( anaflex_options_window );
      anaflex_options_window->show();
   }
}

void US_Hydrodyn::show_bead_output()
{
   if (bead_output_widget)
   {
      if (bead_output_window->isVisible())
      {
         bead_output_window->raise();
      }
      else
      {
         bead_output_window->show();
      }
      return;
   }
   else
   {
      bead_output_window = new US_Hydrodyn_Bead_Output(&bead_output, &bead_output_widget, this);
      fixWinButtons( bead_output_window );
      bead_output_window->show();
   }
}

void US_Hydrodyn::show_grid()
{
   if (grid_widget)
   {
      if (grid_window->isVisible())
      {
         grid_window->raise();
      }
      else
      {
         grid_window->show();
      }
      return;
   }
   else
   {
      grid_window = new US_Hydrodyn_Grid(&grid_exposed_overlap,
                                         &grid_buried_overlap,
                                         &grid_overlap,
                                         &replicate_o_r_method_grid,
                                         &grid,
                                         &overlap_tolerance,
                                         &grid_widget,
                                         this);
      fixWinButtons( grid_window );
      grid_window->show();
   }
}

void US_Hydrodyn::pdb_parsing()
{
   if (pdb_parsing_widget)
   {
      if (pdb_parsing_window->isVisible())
      {
         pdb_parsing_window->raise();
      }
      else
      {
         pdb_parsing_window->show();
      }
      return;
   }
   else
   {
      pdb_parsing_window = new US_Hydrodyn_PDB_Parsing(&pdb_parse, &pdb_parsing_widget, this);
      fixWinButtons( pdb_parsing_window );
      pdb_parsing_window->show();
   }
}

void US_Hydrodyn::pdb_visualization()
{
   if (pdb_visualization_widget)
   {
      if (pdb_visualization_window->isVisible())
      {
         pdb_visualization_window->raise();
      }
      else
      {
         pdb_visualization_window->show();
      }
      return;
   }
   else
   {
      pdb_visualization_window = new US_Hydrodyn_PDB_Visualization(&pdb_vis, &pdb_visualization_widget);
      fixWinButtons( pdb_visualization_window );
      pdb_visualization_window->show();
   }
}

void US_Hydrodyn::load_config()
{
   QString fname = QFileDialog::getOpenFileName( 0 , "Please select a SOMO configuration file..." , US_Config::get_home_dir() + "/etc", "*.config" , 0 );
   if ( fname == QString() )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("\nLoad configuration canceled."));
      editor->setTextColor(save_color);
      display_default_differences();
      return;
   }

   if ( read_config(fname) )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("The configuration file was found to be corrupt.\n"
                              "Resorting to default values."));
      set_default();
   }
   clear_display();
}

void US_Hydrodyn::write_config()
{
   QString fname = US_Config::get_home_dir() + "etc/somo.config";
   switch (
           QMessageBox::question(
                                 this,
                                 windowTitle() + us_tr(": Save configuration "),
                                 us_tr( "Save the current configuration as 'startup' configuration?" ),
                                 QMessageBox::Yes, 
                                 QMessageBox::No,
                                 QMessageBox::Cancel
                                 ) )
   {
   case QMessageBox::Cancel :
      return;
      break;
   case QMessageBox::Yes : 
      break;
   case QMessageBox::No : 
      fname = QFileDialog::getSaveFileName( 0 , "Please name your SOMO configuration file..." , US_Config::get_home_dir() + "etc" , "*.config" , 0 );
      break;
   default :
      return;
      break;
   }

   if ( fname.isEmpty() )
   {
      return;
   }

   if (fname.right(7) != ".config")
   {
      fname += ".config";
   }

   write_config( fname );
}

void US_Hydrodyn::do_reset()
{
   sidechain_overlap = default_sidechain_overlap;
   mainchain_overlap = default_mainchain_overlap;
   buried_overlap = default_buried_overlap;
   grid_exposed_overlap = default_grid_exposed_overlap;
   grid_buried_overlap = default_grid_buried_overlap;
   grid_overlap = default_grid_overlap;
   bead_output = default_bead_output;
   asa = default_asa;
   misc = default_misc;
   overlap_tolerance = default_overlap_tolerance;
   hydro = default_hydro;
   pdb_vis = default_pdb_vis;
   pdb_parse = default_pdb_parse;
   grid = default_grid;
   saxs_options = default_saxs_options;
   batch = default_batch;
   bd_options = default_bd_options;
   dmd_options = default_dmd_options;
   anaflex_options = default_anaflex_options;
   gparams = default_gparams;
   //  save = default_save;
   le_temperature->setText(QString("").sprintf("%4.2f",hydro.temperature));
   le_pH->setText(QString("").sprintf("%4.2f",hydro.pH));
   cb_pH->setChecked( gparams.count( "use_pH" ) && gparams[ "use_pH" ] == "true" );
   le_pH->setEnabled( cb_pH->isChecked() );
   if ( batch_widget ) {
      batch_window->close();
   }
}

void US_Hydrodyn::reset()
{
   QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\nAre you sure you want to reset to the default options?\nAll currently defined options will be reset."),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::No);
   mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
   mb.setButtonText(QMessageBox::No, us_tr("Save Current Options"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         return;
      }
   case QMessageBox::No:
      {
         write_config();
      }
   }
   do_reset();
   clear_display();
}

void US_Hydrodyn::select_residue_file()
{
   QString old_filename = residue_filename;
   residue_filename = QFileDialog::getOpenFileName( this , windowTitle() , US_Config::get_home_dir() + "/etc" , "*.residue *.RESIDUE" );
   if (residue_filename.isEmpty())
   {
      residue_filename = old_filename;
      return;
   }
   read_residue_file();
   create_fasta_vbar_mw();
   set_disabled();
   lbl_table->setText( QDir::toNativeSeparators( residue_filename ) );
}

void US_Hydrodyn::reload_pdb()
{
   // {
   //    qDebug() << "test write pdb";
   //    QString errors;
   //    write_pdb_from_model( model_vector[current_model], errors, "my header note", "my_suffex", "outputfile" );
   // }
   citation_load_pdb();
   if ( advanced_config.debug_1 )
   {
      printf("Reload PDB file called %d %d.\n", pdb_file.isEmpty(), !pb_somo->isEnabled());
   }
   if ( pdb_file.isEmpty() ) // why did we ever do this ? || !pb_somo->isEnabled() )
   {
      return;
   }
   editor->append("\nReloading PDB file.\n");
   if ( advanced_config.debug_1 )
   {
      printf("Reloading PDB file.\n");
   }
   int errors_found = 0;
   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }
   multi_residue_map = save_multi_residue_map;
   read_pdb(pdb_file);
   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
      multi_residue_map = save_multi_residue_map;
      editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
      if (check_for_missing_atoms(&error_string, &model_vector[i]))
      {
         errors_found++;
         editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                        arg( model_name( i ) ) + error_string);
         printError(QString("Encountered errors with your PDB structure for model %1:\n").
                    arg( model_name( i ) ) + "please check the text window");
      }
      reset_chain_residues( &model_vector[ i ] );
   }
   model_vector_as_loaded = model_vector;
   set_pdb_info( "reload pdb" );
   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   bead_models.resize( model_vector.size() );
   somo_processed.resize( model_vector.size() );
   update_vbar();
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   fractal_dimension( true );
}

void US_Hydrodyn::clear_pdb_info( QString /* msg */ ) {
   // us_qdebug( QString( "clear_pdb_info() %1" ). arg( msg ) );
   pdb_info.clear( );
}

void US_Hydrodyn::set_pdb_info( QString /* msg */ ) {
   if ( misc.pb_rule_on ) {
      // us_qdebug( QString( "set_pdb_info() %1 setting pb_rule ON" ). arg( msg ) );
      pdb_info.insert( "pb_rule" );
   } else {
      // us_qdebug( QString( "set_pdb_info() %1 setting pb_rule OFF" ). arg( msg ) );
      pdb_info.erase( "pb_rule" );
   }
}

void US_Hydrodyn::sync_pdb_info( QString /* msg */ ) {
   // us_qdebug( QString( "sync_pdb_info() %1 called misc.pb_rule_on %2 pdb_info.count( 'pb_rule' ) %3" )
   //         .arg( msg )
   //         .arg( misc.pb_rule_on ? "true" : "false" )
   //         .arg( pdb_info.count( "pb_rule" ) ? "true" : "false" )
   //         );
   
   if ( (  misc.pb_rule_on && !pdb_info.count( "pb_rule" ) ) ||
        ( !misc.pb_rule_on && pdb_info.count( "pb_rule" ) ) ) {
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
           
      if ( misc.pb_rule_on && !pdb_info.count( "pb_rule" ) ) {
         // us_qdebug( QString( "sync_pdb_info() %1 misc.pb_rule was ON, now OFF" ). arg( msg ) );
         misc.pb_rule_on      = false;
         misc.restore_pb_rule = true;
      } else {
         // us_qdebug( QString( "sync_pdb_info() %1 misc.pb_rule was OFF, now ON" ). arg( msg ) );
         misc.pb_rule_on      = true;
         misc.restore_pb_rule = false;
      }
   }
}

// add store & reset code for these in state_info ?

int US_Hydrodyn::issue_non_coded( bool quiet ) {
   us_qdebug( "issue_non_coded()" );
   if ( quiet || advanced_config.expert_mode ) {
      us_qdebug( "issue_non_coded() returning quiet or expert" );
      switch ( pdb_parse.missing_residues ) {
      case 0 : // list & stop op
         return ISSUE_RESPONSE_STOP;
         break;
      case 1 : // list and skip
         return ISSUE_RESPONSE_NC_SKIP;
         break;
      case 2 : // abb
      default :
         return ISSUE_RESPONSE_NC_REPLACE;
         break;
      }
   }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_non_coded() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "nc_skip" ) ) {
      us_qdebug( "issue_non_coded() auto return skip" );
      return ISSUE_RESPONSE_NC_SKIP;
   }
   if ( issue_info.count( "nc_replace" ) ) {
      us_qdebug( "issue_non_coded() auto return replace" );
      return ISSUE_RESPONSE_NC_REPLACE;
   }

   switch ( pdb_parse.missing_residues ) {
   case 0 :
      us_qdebug( "issue_non_coded() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_non_coded() setting skip q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Non coded residue" )
                                       ,us_tr("A non-coded residue was encountered\n"
                                           "It can be skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains non-coded residues, the calculated molecular weight\n"
                                           "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                                           "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                                           "global value for the vbar in the SOMO Miscellaneous options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Skip non-coded residues" )
                                       ,us_tr( "Stop processing" )
                                       ,QString()
                                       ,1
                                       ,1
                                       ) ) {
         case 0 : 
            issue_info.insert( "nc_skip" );
            return ISSUE_RESPONSE_NC_SKIP;
            break;

         case 1 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_non_coded() setting abb q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Non coded residue" )
                                       ,us_tr("A non-coded residue was encountered\n"
                                           "It can be replaced with an average residue, skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains non-coded residues, the calculated molecular weight\n"
                                           "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                                           "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                                           "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
                                           "review the average residue settings in the SOMO Miscellaneous options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Replace non-coded with average residues" )
                                       ,us_tr( "Skip non-coded residues" )
                                       ,us_tr( "Stop processing" )
                                       ,2
                                       ,2
                                       ) ) {
         case 0 : 
            issue_info.insert( "nc_replace" );
            return ISSUE_RESPONSE_NC_REPLACE;
            break;

         case 1 : 
            issue_info.insert( "nc_skip" );
            return ISSUE_RESPONSE_NC_SKIP;
            break;

         case 2 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_non_coded() fall thru" );

   return ISSUE_RESPONSE_STOP;
}

int US_Hydrodyn::issue_missing_atom( bool quiet ) {

   us_qdebug( "issue_missing_atom()" );
   if ( quiet || advanced_config.expert_mode ) {
      us_qdebug( "issue_missing_atom() returning quiet or expert" );
      switch ( pdb_parse.missing_atoms ) {
      case 0 : // list & stop op
         return ISSUE_RESPONSE_STOP;
         break;
      case 1 : // list and skip
         return ISSUE_RESPONSE_MA_SKIP;
         break;
      case 2 : // approx
      default :
         return ISSUE_RESPONSE_MA_MODEL;
         break;
      }
   }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_missing_atom() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "ma_skip" ) ) {
      us_qdebug( "issue_missing_atom() auto return skip" );
      return ISSUE_RESPONSE_MA_SKIP;
   }
   if ( issue_info.count( "ma_model" ) ) {
      us_qdebug( "issue_missing_atom() auto return model" );
      return ISSUE_RESPONSE_MA_MODEL;
   }

   switch ( pdb_parse.missing_atoms ) {
   case 0 :
      us_qdebug( "issue_missing_atom() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_missing_atom() setting skip q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Missing or extra atoms" )
                                       ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
                                           "It can be skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If your model contains missing atoms, the calculated molecular\n"
                                           "weight and vbar may be incorrect, and you should manually enter\n"
                                           "a global value for the molecular weight in the SOMO hydrodynamic\n"
                                           "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                                           "options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Skip residues with missing atoms" )
                                       ,us_tr( "Stop processing" )
                                       ,QString()
                                       ,1
                                       ,1
                                       ) ) {
         case 0 : 
            issue_info.insert( "ma_skip" );
            return ISSUE_RESPONSE_MA_SKIP;
            break;

         case 1 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_missing_atom() setting approx q" );
         switch( QMessageBox::question(
                                       this
                                       ,us_tr( "US-SOMO: Missing or extra atoms" )
                                       ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
                                           "It can be modeled with an approximate method, skipped or you can stop processing.\n"
                                           "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
                                           "For specific residue details, please review the main panel text area after making your selection.\n"
                                           "\n"
                                           "If you model with an approximate method, the shape of this residue will\n"
                                           "be different from an exact residue which can effect the hydrodynamic computations\n"
                                           "\n"
                                           "If you skip missing atoms, the calculated molecular\n"
                                           "weight and vbar may be incorrect, and you should manually enter\n"
                                           "a global value for the molecular weight in the SOMO hydrodynamic\n"
                                           "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                                           "options.\n"
                                           "\n"
                                           "Please select your option below"
                                           )
                                       ,us_tr( "Model with the approximate method" )
                                       ,us_tr( "Skip residues with missing atoms" )
                                       ,us_tr( "Stop processing" )
                                       ,2
                                       ,2
                                       ) ) {
         case 0 : 
            issue_info.insert( "ma_model" );
            return ISSUE_RESPONSE_MA_MODEL;
            break;

         case 1 : 
            issue_info.insert( "ma_skip" );
            return ISSUE_RESPONSE_MA_SKIP;
            break;

         case 2 : 
         default :
            issue_info.insert( "stop" );
            return ISSUE_RESPONSE_STOP;
            break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_missing_atom() fall thru" );

   return ISSUE_RESPONSE_STOP;
}

void US_Hydrodyn::load_pdb()
{
   issue_info.clear( );
   citation_load_pdb();
   clear_pdb_info( "load_pdb" );
   if ( misc.restore_pb_rule ) {
      us_qdebug( "load_pdb() restoring pb rule" );
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
         
      misc.pb_rule_on      = true;
      misc.restore_pb_rule = false;
   }

   QString message = "";
   if ( 0 && pdb_parse.missing_residues == 1 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to skip missing residues. If your model contains missing\n"
                    "residues, the calculated molecular weight and vbar may be incorrect, and\n"
                    "you should manually enter a global value for the molecular weight in the\n"
                    "SOMO hydrodynamic options, and a global value for the vbar in the SOMO\n"
                    "Miscellaneous options.\n\nAre you sure you want to proceed?");
   }
   if ( 0 && pdb_parse.missing_residues == 2 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to replace non-coded residues with an average residue.\n"
                    "If your model contains non-coded residues, the calculated molecular weight\n"
                    "and vbar may be incorrect. Therefore, you could manually enter a global\n"
                    "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
                    "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
                    "review the average residue settings in the SOMO Miscellaneous options.\n\n"
                    "Are you sure you want to proceed?");
   }
   if (message != "")
   {
      QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
         case QMessageBox::Cancel:
         {
            return;
         }
      }
   }
   message = "";
   if ( 0 && pdb_parse.missing_atoms == 1 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to skip coded residues containing missing atoms.\n"
                    "If your model contains missing atoms, the calculated molecular\n"
                    "weight and vbar may be incorrect, and you should manually enter\n"
                    "a global value for the molecular weight in the SOMO hydrodynamic\n"
                    "options, and a global value for the vbar in the SOMO Miscellaneous\n"
                    "options.\n\nAre you sure you want to proceed?");
   }
   if ( 0 && pdb_parse.missing_atoms == 2 &&
        !advanced_config.expert_mode )
   {
      message += us_tr("You have selected to model coded residues with missing atoms\n"
                    "with an approximate method.  For best results, you should complete\n"
                    "the structure.\n\n"
                    "Do you want to proceed anyway?");
   }
   if (message != "")
   {
      QMessageBox mb(us_tr("UltraScan"), us_tr("Attention:\n" + message),
                     QMessageBox::Information,
                     QMessageBox::Yes | QMessageBox::Default,
                     QMessageBox::Cancel | QMessageBox::Escape,
                     Qt::NoButton);
      mb.setButtonText(QMessageBox::Yes, us_tr("Yes"));
      mb.setButtonText(QMessageBox::Cancel, us_tr("Cancel"));
      switch(mb.exec())
      {
      case QMessageBox::Cancel:
         {
            return;
         }
      }
   }
   // cout << somo_pdb_dir << endl;
   
   QString use_dir = 
      path_load_pdb.isEmpty() ?
      somo_pdb_dir :
      path_load_pdb;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , "Please select a PDB file..." , use_dir , "Structures (*.pdb *.PDB)" );

   if ( !filename.isEmpty() )
   {
      path_load_pdb = QFileInfo(filename).absolutePath();
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return;
   }


   int errors_found = 0;
   if (!filename.isEmpty())
   {
      add_to_directory_history( filename );
      if ( is_dammin_dammif(filename) ) 
      {
         screen_bead_model(filename);
#if defined(START_RASMOL)
         if ( advanced_config.auto_view_pdb ) {
            model_viewer( filename );
         }
#endif
         return;
      }
      pdb_file = filename;
      options_log = "";
      last_abb_msgs = "";
      bead_model_from_file = false;
      le_pdb_file_save_text = QDir::toNativeSeparators( filename );
      le_pdb_file->setText( QDir::toNativeSeparators( filename ) );
      clear_display();

#if defined(START_RASMOL)
      if ( advanced_config.auto_view_pdb ) {
         model_viewer( filename );
      }
#endif
      QFileInfo fi(filename);
      project = fi.baseName();
      new_residues.clear( );
      if ( misc.pb_rule_on )
      {
         residue_list = save_residue_list;
      }
      else
      {
         residue_list = save_residue_list_no_pbr;
      }
      multi_residue_map = save_multi_residue_map;
      read_pdb(filename);
      QString error_string = "";
      for(unsigned int i = 0; i < model_vector.size(); i++)
      {
         multi_residue_map = save_multi_residue_map;
         editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
         if (check_for_missing_atoms(&error_string, &model_vector[i]))
         {
            errors_found++;
            editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                           arg( model_name( i ) ) + error_string);
            printError(QString("Encountered errors with your PDB structure for model %1:\n").
                       arg( model_name( i ) ) + "please check the text window");
         }
         reset_chain_residues( &model_vector[ i ] );
      }
      model_vector_as_loaded = model_vector;
      set_pdb_info( "load_pdb" );
         

      editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
      bead_models.clear( );
      somo_processed.clear( );
      state = PDB_LOADED;
   }
   else
   {
      return; // user canceled loading PDB file
   }
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   update_vbar();
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   // bead_model_prefix = "";
   pb_somo->setEnabled(true);
   pb_somo_o->setEnabled(true);
   pb_grid_pdb->setEnabled(true);
   pb_vdw_beads->setEnabled(true);
   pb_grid->setEnabled(false);
   bd_anaflex_enables(true);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   pb_pdb_saxs->setEnabled(true);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;

   // if ( lb_model->count() )
   // {
   //    select_model(0);
   // }

   fractal_dimension( true );

   if ( dmd_options.pdb_static_pairs )
   {
      dmd_static_pairs();
   }

   if ( advanced_config.auto_calc_somo  &&
        !errors_found )
   {
      calc_somo();
   }
}

bool US_Hydrodyn::screen_pdb(QString filename, bool display_pdb, bool skipclearissue )
{
   if ( !skipclearissue ) {
      issue_info.clear( );
   }
   clear_pdb_info( "screen_pdb" );

   if ( misc.restore_pb_rule ) {
      // us_qdebug( "screen_pdb() restoring pb rule" );
      if ( misc_widget ) {
         misc_window->close();
         delete misc_window;
         misc_widget = false;
      }
      misc.pb_rule_on      = true;
      misc.restore_pb_rule = false;
   }

   cout << QString( "screen pdb display is %1\n" ).arg( display_pdb ? "true" : "false" );
   pdb_file = filename;

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return false;
   }

   if ( is_dammin_dammif(filename) )
   {
      return screen_bead_model(filename);
   }

   options_log = "";
   last_abb_msgs = "";
   bead_model_from_file = false;
   int errors_found = 0;
   le_pdb_file_save_text = QDir::toNativeSeparators( filename );
   le_pdb_file->setText( QDir::toNativeSeparators( filename ) );

   bead_model_suffix = "";
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );

#if defined(START_RASMOL)
   if ( display_pdb && advanced_config.auto_view_pdb ) {
      model_viewer( filename );
   }
#endif

   QFileInfo fi(filename);
   project = fi.baseName();
   new_residues.clear( );
   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }
   multi_residue_map = save_multi_residue_map;
   if ( read_pdb(filename) )
   {
      return false;
   }
   if ( batch_widget
        && batch_window->batch_job_running
        && model_vector_has_hydration_differences( model_vector ) ) {
      batch_window->editor_msg( "darkred",
                                QString(
                                        us_tr(
                                              "Screening: %1 : WARNING: PDB contains residues with bead hydration without atomic hydration,\nvdW models should not be used as they rely on atomic hydration\n"
                                              )
                                        ).arg( filename )
                                );
   }

#if defined( DEBUG_TESTING_JML )
   us_qdebug( "extra reset0" );
   reset_chain_residues( &model_vector[0]);
   us_qdebug( "after extra reset0" );
#endif

   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
      multi_residue_map = save_multi_residue_map;
      editor->append(QString("Checking the pdb structure for model %1\n").arg( model_name( i ) ) );
#if defined( DEBUG_TESTING_JML )
      us_qdebug( "extra reset" );
      reset_chain_residues( &model_vector[i]);
      us_qdebug( "after extra reset" );
#endif
      if (check_for_missing_atoms(&error_string, &model_vector[i]))
      {
         errors_found++;
         editor->append(QString("Encountered errors with your PDB structure for model %1:\n").
                        arg( model_name( i ) ) + error_string);
         printError(QString("Encountered errors with your PDB structure for model %1:\n").
                    arg( model_name( i ) ) + "please check the text window");
      }
      reset_chain_residues( &model_vector[ i ] );
   }
   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw();
   }
   model_vector_as_loaded = model_vector;
   set_pdb_info( "screen_pdb" );
   if ( !model_vector.size() ||
        !model_vector[0].molecule.size() )
   {
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(us_tr("ERROR : PDB file contains no atoms!"));
      editor->setTextColor(save_color);
      errors_found++;
   }

   editor->append(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok"));
   bead_models.clear( );
   somo_processed.clear( );
   update_vbar();
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   // bead_model_prefix = "";
   pb_somo->setEnabled(errors_found ? false : true);
   pb_somo_o->setEnabled(errors_found ? false : true);
   pb_grid_pdb->setEnabled(errors_found ? false : true);
   pb_vdw_beads->setEnabled(errors_found ? false : true);
   pb_grid->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   pb_bead_saxs->setEnabled(false);
   pb_rescale_bead_model->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   // if ( lb_model->count() == 1 )
   // {
   //    select_model(0);
   // }
   return errors_found ? false : true;
}   

bool US_Hydrodyn::screen_bead_model( QString filename )
{
   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return false;
   }
   options_log = "";
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled(false);
   pb_calc_hullrad->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   bead_model_prefix = "";
   le_bead_model_prefix->setText( bead_model_prefix );
   bead_model_suffix = "";
   le_bead_model_suffix->setText( "" );

   if ( results_widget )
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   bead_model_file = filename;
   le_bead_model_file->setText( QDir::toNativeSeparators( filename ) );
   bool only_overlap = false;
   if ( !read_bead_model(filename, only_overlap ))
   {
      citation_load_bead_model( filename );
      bool so_ovlp = QFileInfo( filename ).completeBaseName().contains( "so_ovlp" );
      us_qdebug( QString( "screen bead model so_ovlp %1" ).arg( so_ovlp ? "true" : "false" ) );
      state = BEAD_MODEL_LOADED;
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled( !so_ovlp );
      pb_calc_zeno->setEnabled( true );
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_grid->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_pdb_saxs->setEnabled(false);
      bd_anaflex_enables(false);
      return true;
   }
   else
   {
      if ( only_overlap )
      {
         citation_load_bead_model( filename );
         state = BEAD_MODEL_LOADED;
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_calc_hydro->setEnabled( false );
         pb_calc_zeno->setEnabled( true );
         pb_calc_grpy->setEnabled( true );
         pb_calc_hullrad->setEnabled( true );
         pb_grid->setEnabled(true);
         pb_bead_saxs->setEnabled(true);
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_pdb_saxs->setEnabled(false);
         bd_anaflex_enables(false);
         return true;
      } else {
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_bead_saxs->setEnabled(false);
         pb_rescale_bead_model->setEnabled(false);
         pb_pdb_saxs->setEnabled(true);
         return false;
      }
   }
}
   
void US_Hydrodyn::view_pdb()
{
   
   QString use_dir = 
      path_view_pdb.isEmpty() ?
      somo_pdb_dir :
      path_view_pdb;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.pdb *.PDB" );
   
   if (!filename.isEmpty())
   {
      path_view_pdb = QFileInfo(filename).absolutePath();
      view_file(filename);
   }
}

QString US_Hydrodyn::model_name( int val )
{
   return QString( "%1" ).arg( lb_model->item( val )->text() ).replace( "Model: ", "" );
}

void US_Hydrodyn::model_selection_changed()
{
   select_model();
   // QString msg = QString( "\n%1 models selected:" ).arg( project );
   // for( int i = 0; i < lb_model->count(); i++ )
   // {
   //    if ( lb_model->item( i )->isSelected() )
   //    {
   //       current_model = i;
   //       // msg += QString( " %1" ).arg( i + 1 );
   //       msg += " " + model_name( i );
   //    }
   // }
   // msg += "\n";
   // editor->append( msg );
}

void US_Hydrodyn::select_model( int )
{
   // current_model = val;
   QString msg = QString( "\n%1 models selected:" ).arg( project );
   int selected_count = 0;
   for( int i = 0; i < lb_model->count(); i++ )
   {
      if ( lb_model->item( i )->isSelected() )
      {
         selected_count++;
         current_model = i;
         // msg += QString( " %1" ).arg( i + 1 );
         msg += " " + model_name( i );
      }
   }
   if ( selected_count ) {
      last_no_model_selected = false;
      msg += "\n";
      editor->append( msg );
   } else {
      if ( !last_no_model_selected ) {
         editor_msg( "black", "\nNo models selected." );
      }
      last_no_model_selected = true;
   }

   // check integrity of PDB file and confirm that all residues are correctly defined in residue table
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }
   pb_somo->setEnabled( selected_count );
   pb_grid_pdb->setEnabled( selected_count );
   pb_vdw_beads->setEnabled( selected_count );
   pb_somo_o->setEnabled( selected_count );
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false);
   pb_calc_hullrad->setEnabled( false );
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   //   pb_pdb_saxs->setEnabled(true);
   bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
                         ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
}

void US_Hydrodyn::write_bead_ebf(QString fname, vector<PDB_atom> *model)
{
   {
      FILE *f = us_fopen(fname.toLatin1().data(), "w");
      for (unsigned int i = 0; i < model->size(); i++)
      {
         if ((*model)[i].active)
         {
            fwrite(&((*model)[i].bead_cog_coordinate.axis[0]), sizeof(float), 3, f);
            float color[3] = { .2f, .2f, ((int)(*model)[i].all_beads.size() / 5) > 1 ? 1 : ((float)(*model)[i].all_beads.size() / 5.0f) };
            fwrite(color, sizeof(float), 3, f);
            fwrite(&(*model)[i].bead_computed_radius, sizeof(float), 1, f);
         }
      }
      fclose(f);
   }
   {
      FILE *f = us_fopen(QString("%1-info").arg(fname).toLatin1().data(), "w");
      for (unsigned int i = 0; i < model->size(); i++)
      {
         if ((*model)[i].active)
         {
            float color[3] = { .2f, .2f, ((*model)[i].all_beads.size() / 5) > 1 ? 1 : ((float)(*model)[i].all_beads.size() / 5.0f) };
            fprintf(f,"%f %f %f %f %f %f %f\n",
                    (*model)[i].bead_coordinate.axis[0],
                    (*model)[i].bead_coordinate.axis[1],
                    (*model)[i].bead_coordinate.axis[2],
                    color[0],
                    color[1],
                    color[2],
                    (*model)[i].bead_computed_radius);
         }
      }
      fclose(f);
   }
}

void US_Hydrodyn::update_bead_model_file(const QString &str)
{
   bead_model_file = str;
}

void US_Hydrodyn::update_bead_model_prefix(const QString &str)
{
   bead_model_prefix = str;
}

void US_Hydrodyn::update_temperature(const QString &str, bool update_hydro )
{
   hydro.temperature = str.toDouble();
   display_default_differences();
   if ( update_hydro ) {
      if ( hydro_widget ) {
         hydro_window->update_temperature( str, false );
      }
   } else {
      le_temperature->setText( str );
   }
}

void US_Hydrodyn::update_pH(const QString &str)
{
   if ( hydro.pH == str.toDouble() ) {
      return;
   }
   set_disabled();
   
   hydro.pH = str.toDouble();
   display_default_differences();

   // not needed since no entry allowed when the options widget is up
   // if ( saxs_hplc_widget &&
   //      saxs_hplc_window &&
   //      saxs_hplc_window->saxs_hplc_options_widget ) {
   //    ((US_Hydrodyn_Saxs_Hplc_Options*)saxs_hplc_window->saxs_hplc_options_widget)->le_fasta_pH->setText( QString( "" ).sprintf( "%4.2f", hydro.pH ) );
   // }
}

void US_Hydrodyn::set_pH()
{
   // force on
   cb_pH->setChecked( true );
   le_pH->setText( "7" );

   gparams[ "use_pH" ] = cb_pH->isChecked() ? "true" : "false";
   le_pH->setEnabled( cb_pH->isChecked() );
   // not needed now
   // if ( !cb_pH->isChecked() ) {
   //    le_pH->setText( "7" );
   // }
   display_default_differences();
}


void US_Hydrodyn::set_calcAutoHydro()
{
   calcAutoHydro = cb_calcAutoHydro->isChecked();
}

void US_Hydrodyn::set_setSuffix()
{
   setSuffix = cb_setSuffix->isChecked();
   display_default_differences();
}

void US_Hydrodyn::set_overwrite()
{
   overwrite = cb_overwrite->isChecked();
}

void US_Hydrodyn::set_saveParams()
{
   saveParams = cb_saveParams->isChecked();
   if ( batch_widget )
   {
      batch_window->cb_saveParams->setChecked(saveParams);
   }
}

void US_Hydrodyn::view_asa()
{
   QString use_dir = 
      path_view_asa_res.isEmpty() ?
      somo_dir :
      path_view_asa_res;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.asa_res *.ASA_RES" );
   if (!filename.isEmpty())
   {
      path_view_asa_res = QFileInfo(filename).absolutePath();
      view_file(filename);
   }
}

void US_Hydrodyn::view_bead_model()
{
   QString filename = "";
   bool chose_last_bead_model = false;
   if ( last_bead_model != "" ) {
      QFileInfo fi(last_bead_model);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("View Bead Model File"),
                                    QString(us_tr("View last produced bead model ") + fi.fileName() + " ?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes :
         chose_last_bead_model = true;
         filename = last_bead_model;
         break;
      case QMessageBox::Cancel :
         return;
         break;
      default :
         break;
      }
   }
   if ( !chose_last_bead_model ) {
      if ( last_read_bead_model != "" ) {

         QFileInfo fi(last_read_bead_model);
         switch (
                 QMessageBox::question(
                                       this,
                                       us_tr("View Bead Model File"),
                                       QString(us_tr("View last read bead model ") + fi.fileName() + " ?"),
                                       QMessageBox::Yes, 
                                       QMessageBox::No,
                                       QMessageBox::Cancel
                                       ) )
         {
         case QMessageBox::Yes : 
            filename = last_read_bead_model;
            break;
         case QMessageBox::No : 
            {
               QString use_dir = 
                  path_view_bead_model.isEmpty() ?
                  somo_dir :
                  path_view_bead_model;
            
               select_from_directory_history( use_dir, this );

               filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                                        "BEAMS (*.beams *.BEAMS);;"
                                                        "DAMMIN/DAMMIF (*.pdb)" , &bead_model_selected_filter );

               if ( !filename.isEmpty() )
               {
                  path_view_bead_model = QFileInfo(filename).absolutePath();
               }
            }
            break;
         case QMessageBox::Cancel :
         default :
            return;
            break;
         }
      } else {
         QString use_dir = 
            path_view_bead_model.isEmpty() ?
            somo_dir :
            path_view_bead_model;

         select_from_directory_history( use_dir, this );

         filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                                  "BEAMS (*.beams *.BEAMS);;"
                                                  "DAMMIN/DAMMIF (*.pdb)" , &bead_model_selected_filter );

         if ( !filename.isEmpty() )
         {
            path_view_bead_model = QFileInfo(filename).absolutePath();
         }
      }
   }

   if (!filename.isEmpty()) {
      view_file(filename);
   }
}

void US_Hydrodyn::visualize( bool movie_frame, 
                             QString dir, 
                             float scale, 
                             bool black_background,
                             bool do_pat )
{
   QString use_dir = ( dir == "" ? somo_dir : dir );

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            bead_model = bead_models[current_model];
            QString spt_name = 
               project +
               ( bead_model_from_file ? 
                 "" 
                 : 
                 ( QString("_%1").arg( current_model + 1 ) +
                   QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" ) ) );

            spt_name = spt_name.left( 30 );

            if ( do_pat ) {
               PDB_chain tmp_chain;
               tmp_chain.atom = bead_model;
               PDB_model tmp_model;
               tmp_model.molecule.push_back( tmp_chain );
               if ( US_Saxs_Util::pat_model( tmp_model, true ) ) {
                  write_bead_spt( use_dir + SLASH + spt_name,
                                  &tmp_model.molecule[ 0 ].atom, 
                                  movie_frame, 
                                  scale, 
                                  black_background );
               } else {
                  editor_msg( "red", QString( us_tr( "PAT failed for model %1" ) ).arg( current_model ) );
                  write_bead_spt( use_dir + SLASH + spt_name,
                                  &bead_model, 
                                  movie_frame, 
                                  scale, 
                                  black_background );
               }
            } else {
               write_bead_spt( use_dir + SLASH + spt_name,
                               &bead_model, 
                               movie_frame, 
                               scale, 
                               black_background );
            }
            
            editor->append(QString("Visualizing model %1\n").arg(current_model + 1));
            model_viewer( use_dir + SLASH + spt_name + ".spt", "-script", movie_frame );

#if defined( TODO_FIX_MOVIE_FRAME )
            if ( movie_frame )
            {
               if (!rasmol->launch(last_spt_text))
               {
                  US_Static::us_message(us_tr("Please note:"), us_tr("There was a problem starting RASMOL\n"
                                                              "Please check to make sure RASMOL is properly installed..."));
                  return;
               }
               cout << "movie text [" << last_spt_text << "]\n";
               //               rasmol->writeToStdin(last_spt_text);
               //               rasmol->closeStdin();
               //               qApp->processEvents();
               while ( rasmol && rasmol->state() == QProcess::Running )
               {
# if defined(WIN32) && !defined( MINGW )
                  _sleep(1);
# else
                  US_Saxs_Util::us_usleep(1000);
# endif
                  qApp->processEvents();
               }
            }
#endif
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg(current_model + 1));
         }
      }
   }
}

int US_Hydrodyn::calc_hydro()
{
   return do_calc_hydro();
}

int US_Hydrodyn::do_calc_hydro()
{
   if ( !overwrite )
   {
      setHydroFile();
   }

   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false );
   pb_calc_hullrad->setEnabled( false );
   //   puts("calc hydro (supc)");
   display_default_differences();
   editor->append("\nBegin hydrodynamic calculations\n\n");
   results.s20w_sd = 0.0;
   results.D20w_sd = 0.0;
   results.viscosity_sd = 0.0;
   results.rs_sd = 0.0;
   results.rg_sd = 0.0;
   results.tau_sd = 0.0;

   int models_to_proc = 0;
   int first_model_no = 0;
   vector < QString > model_names;
   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         if (somo_processed[current_model]) {
            if (!first_model_no) {
               first_model_no = current_model + 1;
            }
            models_to_proc++;
            editor->append( QString( "Model %1 will be included\n").arg( model_name( current_model ) ) );
            model_names.push_back( model_name( current_model ) );
            bead_model = bead_models[current_model];

            // write_bead_spt(somo_dir + SLASH + project +
            //          (bead_model_from_file ? "" : QString("_%1").arg( model_name( current_model ) ) ) +
            //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
            //          DOTSOMO, &bead_model, bead_model_from_file);
         }
         else
         {
            editor->append(QString("Model %1 - selected but bead model not built\n").arg( model_name( current_model ) ) );
         }
      }
   }

   QDir::setCurrent(somo_dir);

   editor->append(QString("%1")
                  //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                  .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                       ? QString("\nNotice: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\n")
                       .arg(overlap_tolerance).arg(hydro.overlap) : ""));

   qApp->processEvents();
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_bead_saxs->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      progress->reset();
      return -1;
   }
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
   // qDebug() << "us_hydrodyn::do_calc_hydro()_rg +/- " << results.asa_rg_pos << " " << results.asa_rg_neg;

   int retval = us_hydrodyn_supc_main(&results,
                                      &hydro,
                                      hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance,
                                      &bead_models,
                                      &somo_processed,
                                      &model_vector,
                                      lb_model,
                                      QString(project +
                                              // (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                                              // (bead_model_from_file ? "" : (models_to_proc == 1 ? "_1" : "_%1")) +
                                              (bead_model_from_file ? "" : "_%1") +
                                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                                              DOTSOMO + ".beams").toLatin1().data(),
                                      QString(project +
                                              (bead_model_from_file ? "" : QString("_%1").arg( model_name( first_model_no - 1 ) ) ) +
                                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                                              DOTSOMO + ".beams").toLatin1().data(),
                                      model_names,
                                      progress,
                                      editor,
                                      this);

   progress->reset();
   mprogress->reset();
   mprogress->hide();
   
   QDir::setCurrent(somo_tmp_dir);
   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
      pb_show_hydro_results->setEnabled(false);
      return -1;
   }

   // printf("back from supc retval %d\n", retval);
   pb_show_hydro_results->setEnabled(retval ? false : true);
   pb_calc_hydro->setEnabled(true);
   pb_calc_zeno->setEnabled(true);
   pb_bead_saxs->setEnabled(true);
   pb_calc_grpy->setEnabled( true );
   pb_calc_hullrad->setEnabled( true );
   pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   if ( retval )
   {
      editor->append("Calculate hydrodynamics failed\n\n");
      qApp->processEvents();
      switch ( retval )
      {
      case US_HYDRODYN_SUPC_FILE_NOT_FOUND:
         {
            printError("Calculate RB Hydrodynamics SMI encountered a file not found error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_OVERLAPS_EXIST:
         {
            printError("Calculate RB Hydrodynamics SMI encountered overlaps in the bead model\nPerhaps you should Calculate RB Hydrodynamics ZENO or GRPY which support bead models with overlaps");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC:
         {
            printError("Calculate RB Hydrodynamics SMI encountered a memory allocation error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_NO_SEL_MODELS:
         {
            printError("Calculate RB Hydrodynamics SMI was called with no processed models selected");
            return retval;
            break;
         }
      case US_HYDRODYN_PAT_ERR_MEMORY_ALLOC:
         {
            printError("US_HYDRODYN_PAT encountered a memory allocation error");
            return retval;
            break;
         }
      default:
         {
            printError("Calculate RB Hydrodynamics SMI encountered an unknown error");
            // unknown error
            return retval;
            break;
         }
      }
   }

   pb_stop_calc->setEnabled(false);
   editor_msg( "black", "Calculate RB hydrodynamics SMI completed\n");
   editor_msg( "dark blue", info_cite( "smi" ) );
   
   if ( advanced_config.auto_show_hydro ) 
   {
      show_hydro_results();
   }
   play_sounds(1);
   qApp->processEvents();
   return 0;
}

void US_Hydrodyn::show_hydro_results()
{
   if ( misc.hydro_supc )
   {
      if (results_widget)
      {
         results_window->close();
         delete results_window;
         results_widget = false;
      }
      {
         results_window = new US_Hydrodyn_Results(&results, &results_widget, this );
         fixWinButtons( results_window );
         results_window->show();
      }
      return;
   }
   if ( misc.hydro_zeno )
   {
      show_zeno();
      return;
   }
}

void US_Hydrodyn::select_comparative()
{
   if (comparative_widget)
   {
      if (comparative_window->isVisible())
      {
         comparative_window->raise();
      }
      else
      {
         comparative_window->show();
      }
      return;
   }
   else
   {
      comparative_window = 
         new US_Hydrodyn_Comparative(&comparative, this, &comparative_widget);
      fixWinButtons( comparative_window );
      comparative_window->show();
   }
}

void US_Hydrodyn::best_analysis()
{
   if ( best_widget )
   {
      if ( best_window->isVisible() )
      {
         best_window->raise();
      }
      else
      {
         best_window->show();
      }
      return;
   }
   else
   {
      best_window = 
         (QWidget *) new US_Hydrodyn_Best( this, & best_widget );
      fixWinButtons( best_window );
      best_window->show();
   }
}

void US_Hydrodyn::open_hydro_results()
{
   QString filename = "";
   if ( last_hydro_res != "" && QFile( somo_dir + SLASH + last_hydro_res ).exists() ) {
      QFileInfo fi(last_hydro_res);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Open Hydrodynamic Calculations File"),
                                    QString(us_tr("View last results file ") + fi.fileName() + " ?"),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::Cancel
                                    ) )
      {
      case QMessageBox::Yes : 
         filename = somo_dir + SLASH + last_hydro_res;
         break;
      case QMessageBox::No : 
         {
            QString use_dir = 
               path_open_hydro_res.isEmpty() ?
               somo_dir :
               path_open_hydro_res;
            
            select_from_directory_history( use_dir, this );

            filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.hydro_res *.HYDRO_RES *.zno *.ZNO *.grpy_res *.GRPY_RES" );

            if ( !filename.isEmpty() )
            {
               path_open_hydro_res = QFileInfo(filename).absolutePath();
            }
         }
         break;
      case QMessageBox::Cancel :
      default :
         return;
         break;
      }
   } else {
      QString use_dir = 
         path_open_hydro_res.isEmpty() ?
         somo_dir :
         path_open_hydro_res;

      select_from_directory_history( use_dir, this );

      filename = QFileDialog::getOpenFileName( this , windowTitle() , use_dir , "*.hydro_res *.HYDRO_RES *.zno *.ZNO *.grpy_res *.GRPY_RES" );

      if ( !filename.isEmpty() )
      {
         path_open_hydro_res = QFileInfo(filename).absolutePath();
      }
   }
   if (!filename.isEmpty())
   {
      view_file(filename);
   }
}

void US_Hydrodyn::select_save_params()
{
   if (save_widget)
   {
      if (save_window->isVisible())
      {
         save_window->raise();
      }
      else
      {
         save_window->show();
      }
      return;
   }
   else
   {
      save_window = new US_Hydrodyn_Save(&save_params, this, &save_widget);
      fixWinButtons( save_window );
      save_window->show();
   }
}

void US_Hydrodyn::show_advanced_config()
{
   if (advanced_config_widget)
   {
      if (advanced_config_window->isVisible())
      {
         advanced_config_window->raise();
      }
      else
      {
         advanced_config_window->show();
      }
      return;
   }
   else
   {
      advanced_config_window = new US_Hydrodyn_AdvancedConfig(&advanced_config, &advanced_config_widget, this);
      fixWinButtons( advanced_config_window );
      advanced_config_window->show();
   }
}

void US_Hydrodyn::show_batch()
{
   if (batch_widget)
   {
      if (batch_window->isVisible())
      {
         batch_window->raise();
      }
      else
      {
         batch_window->show();
      }
      return;
   }
   else
   {
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->show();
   }
}

void US_Hydrodyn::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo/somo.html");
}

void US_Hydrodyn::stop_calc()
{
   // qDebug() << "US_Hydrodyn::stop_calc()";
   stopFlag = true;
   anaflex_return_to_bd_load_results = false;
   if ( batch_widget &&
        batch_window->pb_stop->isEnabled() ) {
      emit batch_window->stop();
   }
   if ( browflex && browflex->state() == QProcess::Running )
   {
      browflex->terminate();
      QTimer::singleShot( 1000, browflex, SLOT( kill() ) );
   }
   if ( anaflex && anaflex->state() == QProcess::Running )
   {
      anaflex->terminate();
      QTimer::singleShot( 1000, anaflex, SLOT( kill() ) );
   }
   if ( grpy_running && grpy && grpy->state() == QProcess::Running )
   {
      grpy->terminate();
      QTimer::singleShot( 10000, grpy, SLOT( kill() ) );
   }
   pb_stop_calc->setEnabled(false);
}

void US_Hydrodyn::cancel()
{
   close();
}

void US_Hydrodyn::update_font()
{
   bool ok;
   QFont newFont;
   newFont = QFontDialog::getFont( &ok, ft, this );
   if ( ok )
   {
      ft = newFont;
   }
   editor->setFont(ft);
}

void US_Hydrodyn::save()
{
   QString fn;
   fn = QFileDialog::getSaveFileName( this , windowTitle() , QString() , QString() );
   if(!fn.isEmpty() )
   {
      QString text = editor->toPlainText();
      QFile f( fn );
      if ( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
      {
         return;
      }
      QTextStream t( &f );
      t << text;
      f.close();
 //      editor->setModified( false );
      setWindowTitle( fn );
   }
}

void US_Hydrodyn::print()
{
#ifndef NO_EDITOR_PRINT
   const int MARGIN = 10;
   printer.setPageSize(QPrinter::Letter);

   if ( printer.setup(this) ) {      // opens printer dialog
      QPainter p;
      p.begin( &printer );         // paint on printer
      p.setFont(editor->font() );
      int yPos      = 0;         // y position for each line
      QFontMetrics fm = p.fontMetrics();
      //  QPaintDeviceMetrics metrics( &printer ); // need width/height
      // of printer surface
      for( int i = 0 ; i < editor->lines() ; i++ ) {
         if ( MARGIN + yPos > printer.height() - MARGIN ) {
            printer.newPage();      // no more room on this page
            yPos = 0;         // back to top of page
         }
         p.drawText( MARGIN, MARGIN + yPos,
                     printer.width(), fm.lineSpacing(),
                     ExpandTabs | DontClip,
                     editor->toPlainText( i ) );
         yPos = yPos + fm.lineSpacing();
      }
      p.end();            // send job to printer
   }
#endif
}

void US_Hydrodyn::clear_display()
{
   editor->clear( );
   editor->setText("\n");
   display_default_differences();
}

int US_Hydrodyn::calc_iqq( bool bead_model, bool create_native_saxs, bool do_raise )
{
   bead_model ? bead_saxs( create_native_saxs, do_raise ) : pdb_saxs( create_native_saxs, do_raise );
   saxs_plot_window->show_plot_saxs_sans();
   return 0;
}

int US_Hydrodyn::calc_prr( bool bead_model, bool create_native_saxs, bool do_raise )
{
   bead_model ? bead_saxs( create_native_saxs, do_raise ) : pdb_saxs( create_native_saxs, do_raise );
   saxs_plot_window->show_plot_pr();
   return 0;
}

void US_Hydrodyn::pdb_saxs( bool create_native_saxs, bool do_raise )
{
   //   cout << QString("ext %1 sans_sans %2 curve %3\n")
   //      .arg(saxs_sans_ext())
   //      .arg(saxs_options.saxs_sans)
   //      .arg(saxs_options.curve);

   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ ) 
   {
      if ( lb_model->item(i)->isSelected() ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() > 1 )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("You must select exactly one model to perform SAXS functions."));
   } 
   else
   {
      QString filename = project;
      // probably want to set specially for batch mode?
      if ( filename.length() )
      {
         if ( dammix_remember_mw.count(QFileInfo(filename).fileName()) &&
              !saxs_options.multiply_iq_by_atomic_volume )
         {
            if ( dammix_remember_mw[QFileInfo(filename).fileName()] != 
                 model_vector[selected_models[0]].mw
                 && ( !batch_widget ||
                      !batch_window->batch_job_running ) )
            {
               switch ( QMessageBox::question(this, 
                                              us_tr("UltraScan Notice"),
                                              QString(us_tr("Please note:\n\n"
                                                         "You have remembered a molecular weight of %1 Daltons\n"
                                                         "but the loaded pdb has a computed molecular weight of %2 Daltons\n"
                                                         "What would you like to do?\n"))
                                              .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                              .arg(model_vector[selected_models[0]].mw)
                                              ,
                                              us_tr("&Set to the newly computed value"),
                                              us_tr("&Keep the remembered value"), 
                                              us_tr("&Enter it manually later"),
                                              0, // Stop == button 0
                                              0 // Escape == button 0
                                              )
                        )
               {
               case 0 : 
                  dammix_remember_mw[QFileInfo(filename).fileName()] =
                     model_vector[selected_models[0]].mw;
                  dammix_remember_mw_source[QFileInfo(filename).fileName()] =
                     "computed from pdb";
                  break;
               case 1 : 
                  break;
               case 2 : 
                  dammix_remember_mw.erase(QFileInfo(filename).fileName());
                  dammix_remember_mw_source.erase(QFileInfo(filename).fileName());
                  break;
               default :
                  break;
               }
            }
         } else {
            dammix_remember_mw[QFileInfo(filename).fileName()] =
               model_vector[selected_models[0]].mw + model_vector[selected_models[0]].ionized_mw_delta;
            dammix_remember_mw_source[QFileInfo(filename).fileName()] =
               "computed from pdb";
         }
      }
            
      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   pdb_file,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   0,
                                   create_native_saxs
                                   );
         if ( do_raise )
         {
            saxs_plot_window->raise();
         }
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 pdb_file,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 0, 
                                                 create_native_saxs,
                                                 this,
                                                 0
                                                 );
         fixWinButtons( saxs_plot_window );
         // always show
         saxs_plot_window->show();
      }
   }
}

void US_Hydrodyn::bead_saxs( bool create_native_saxs, bool do_raise )
{
   vector < unsigned int > selected_models;
   for ( unsigned int i = 0; i < (unsigned int)lb_model->count(); i++ ) 
   {
      if ( lb_model->item(i)->isSelected() ) 
      {
         selected_models.push_back(i);
      }
   }
   if ( selected_models.size() != 1 )
   {
      US_Static::us_message(us_tr("Please note:"),
                           us_tr("You must select exactly one model to perform SAXS functions.."));
   } 
   else
   {
      QString filename = QFileInfo(bead_model_file).completeBaseName() +
         QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
      if ( !QFileInfo(bead_model_file).completeBaseName().length() )
      {
         filename = project + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "");
      }
      if ( !filename.length() )
      {
         filename = "unknown";
      }
      
      printf("selected models size %u bead_models.size %u beads in selected_models[0] %u\n",
             (unsigned int)bead_models.size(),
             (unsigned int)selected_models.size(),
             (unsigned int)bead_models[selected_models[0]].size()
             );

      // compute mw
      float tmp_mw = 0.0;
      for ( unsigned int i = 0; i < bead_models[selected_models[0]].size(); i++ )
      {
         tmp_mw    += bead_models[selected_models[0]][i].bead_mw + bead_models[selected_models[0]][i].bead_ionized_mw_delta;
      }

      if ( tmp_mw != 0.0 &&
           dammix_remember_mw.count(QFileInfo(filename).fileName()) )
      {
         if ( dammix_remember_mw[QFileInfo(filename).fileName()] != 
              tmp_mw )
         {
            switch ( QMessageBox::question(this, 
                                           us_tr("UltraScan Notice"),
                                           QString(us_tr("Please note:\n\n"
                                                      "You have remembered a molecular weight of %1 Daltons\n"
                                                      "but the loaded bead model a computed molecular weight of %2 Daltons\n"
                                                      "What would you like to do?\n"))
                                           .arg(dammix_remember_mw[QFileInfo(filename).fileName()])
                                           .arg(tmp_mw)
                                           ,
                                           us_tr("&Set to the newly computed value"),
                                           us_tr("&Keep the remembered value"), 
                                           us_tr("&Enter it manually later"),
                                           0, // Stop == button 0
                                           0 // Escape == button 0
                                           )
                     )
            {
            case 0 : 
               dammix_remember_mw[QFileInfo(filename).fileName()] =
                  tmp_mw;
               dammix_remember_mw_source[QFileInfo(filename).fileName()] =
                  "computed from bead model";
               break;
            case 1 : 
               break;
            case 2 : 
               dammix_remember_mw.erase(QFileInfo(filename).fileName());
               dammix_remember_mw_source.erase(QFileInfo(filename).fileName());
               break;
            default :
               break;
            }
         }
      } else {
         dammix_remember_mw[QFileInfo(filename).fileName()] = tmp_mw;
         dammix_remember_mw_source[QFileInfo(filename).fileName()] =
            "computed from bead model";
      }

      if (saxs_plot_widget)
      {
         saxs_plot_window->refresh(
                                   filename,
                                   pdb_file,
                                   residue_list,
                                   model_vector,
                                   bead_models,
                                   selected_models,
                                   multi_residue_map,
                                   residue_atom_hybrid_map,
                                   1,
                                   create_native_saxs
                                   );
         if ( do_raise )
         {
            saxs_plot_window->raise();
         }
      }
      else
      {
         saxs_plot_window = new US_Hydrodyn_Saxs(
                                                 &saxs_plot_widget,
                                                 &saxs_options,
                                                 filename,
                                                 pdb_file,
                                                 residue_list,
                                                 model_vector,
                                                 bead_models,
                                                 selected_models,
                                                 multi_residue_map,
                                                 residue_atom_hybrid_map,
                                                 1, 
                                                 create_native_saxs,
                                                 this,
                                                 0
                                                 );
         fixWinButtons( saxs_plot_window );
         // always show
         saxs_plot_window->show();
      }
   }
}

QString US_Hydrodyn::getExtendedSuffix(bool prerun, bool somo, bool no_ovlp_removal, bool vdw )
{
   // produce a suffix based upon settings
   // e.g.
   // A20 for ASA screen 20 
   // R50 for ASA recheck 50% 
   // hi or sy (if ALL 3 panels set to the same thing) 
   // OT outward translation
   // hy = hydration on in grid models,
   // G4 grid setting in grid models & 
   // -so for somo models
   // pH for ph value

   QString result = le_bead_model_prefix->text();

   if ( vdw ) {
      result += result.length() ? "-" : "";
      bead_model_suffix = "";
      {
         double vdw_ot_mult = gparams.count( "vdw_ot_mult" ) ? gparams[ "vdw_ot_mult" ].toDouble() : 0;
         double vdw_ot_dpct = gparams.count( "vdw_ot_dpct" ) ? gparams[ "vdw_ot_dpct" ].toDouble() : 0;
         bool vdw_ot_alt = gparams.count( "vdw_ot_alt" ) && gparams[ "vdw_ot_alt" ] == "true";
         if ( vdw_ot_mult ) {
            if ( vdw_ot_dpct ) {
               bead_model_suffix += QString( "OT%1%2DP%3_").arg( vdw_ot_alt ? "alt" : "" ).arg( vdw_ot_mult ).arg( vdw_ot_dpct ).replace( ".", "_" );
            } else {
               bead_model_suffix += QString( "OT%1%2_").arg( vdw_ot_alt ? "alt" : "" ).arg( vdw_ot_mult ).replace( ".", "_" );
            }
         }
         bead_model_suffix += QString( "PR%1_TH%2_" ).arg( asa.hydrate_probe_radius ).arg( asa.hydrate_threshold ).replace( ".", "_" );
         bead_model_suffix += QString( "pH%1").arg( hydro.pH ).replace( ".", "_" );
         bead_model_suffix += "-vdw";
      }
      result += bead_model_suffix;

      return result;
   }

   if ( setSuffix )
   {
      result += result.length() ? "-" : "";

      if ( ( asa.calculation && somo ) ||
           ( grid.enable_asa && !somo ) )
      {
         result += QString("A%1").arg(somo ? asa.threshold : asa.grid_threshold);
      }
      
      if ( asa.recheck_beads )
      {
         result += QString("R%1").arg(somo ? asa.threshold_percent : asa.grid_threshold_percent);
      }

      if ( somo ) 
      {
         if ( !no_ovlp_removal ) 
         {
            if ( sidechain_overlap.remove_overlap &&
                 mainchain_overlap.remove_overlap &&
                 buried_overlap.remove_overlap &&
                 (sidechain_overlap.remove_sync == 
                  mainchain_overlap.remove_sync) && 
                 (mainchain_overlap.remove_sync == 
                  buried_overlap.remove_sync ) &&
                 (sidechain_overlap.remove_hierarch == 
                  mainchain_overlap.remove_hierarch) &&
                 (mainchain_overlap.remove_hierarch == 
                  buried_overlap.remove_hierarch ) )
            {
               result += QString("%1").arg(sidechain_overlap.remove_sync ? "sy" : "hi");
            }
         
            if ( sidechain_overlap.remove_overlap &&
                 sidechain_overlap.translate_out )
            {
               result += "OT";
            }
         }
      }
      if ( !somo ) 
      {
         if ( !no_ovlp_removal ) 
         {
            if ( grid_exposed_overlap.remove_overlap &&
                 grid_buried_overlap.remove_overlap &&
                 grid_overlap.remove_overlap &&
                 (grid_exposed_overlap.remove_sync == 
                  grid_buried_overlap.remove_sync) &&
                 (grid_buried_overlap.remove_sync ==
                  grid_overlap.remove_sync ) &&
                 (grid_exposed_overlap.remove_hierarch == 
                  grid_buried_overlap.remove_hierarch) &&
                 (grid_buried_overlap.remove_hierarch == 
                  grid_overlap.remove_hierarch ) )
            {
               result += QString("%1").arg(grid_exposed_overlap.remove_sync ? "sy" : "hi");
            }
         
            if ( grid_exposed_overlap.remove_overlap &&
                 grid_exposed_overlap.translate_out )
            {
               result += "OT";
            }
         }

         if ( grid.hydrate )
         {
            result += "hy";
         }
         
         if ( grid.cubic )
         {
            result += QString("G%1").arg(grid.cube_side);
         }
      }
      result += QString( "pH%1").arg( hydro.pH );
   }
   if ( !prerun )
   { 
      result += QString(result.length() ? "-" : "") + QString(somo ? "so" : "a2b");
      if ( somo && no_ovlp_removal )
      {
         result += "_ovlp";
      }
   }
   //   editor->append(result);
   result.replace( ".", "_" );
   return result;
}

void US_Hydrodyn::setHydroFile()
{
   bool any_changes;
   QString path;
   QString file;
   QString ext;
   QString new_file;

   do {
      any_changes = false;
      for( int i = 0; i < lb_model->count(); i++ )
      {
         if ( lb_model->item(i)->isSelected() )
         {
            path = 
               somo_dir + QDir::separator() +
               project + QString("_%1").arg(i + 1) + 
               (bead_model_suffix.length() ? "-" : "");
            
            ext = ".hydro_res";
            file = path + bead_model_suffix + ext;
            new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
            if ( file != new_file )
            {
               le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
               any_changes = true;
            }

            if ( saveParams )
            {
               ext = ".csv";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
                  any_changes = true;
               }
            }
         }
      }
   } while ( any_changes );
}

void US_Hydrodyn::setSomoGridFile(bool somo)
{
   // check all selected models, all output types

   bool any_changes;
   QString path;
   QString file;
   QString ext;
   QString new_file;

   do {
      any_changes = false;
      for( int i = 0; i < lb_model->count(); i++ )
      {
         if ( lb_model->item(i)->isSelected() )
         {
            path = 
               somo_dir + QDir::separator() +
               project + QString("_%1").arg(i + 1) + 
               (bead_model_suffix.length() ? "-" : "");
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
               ext = ".bead_model";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
                  any_changes = true;
               }
            }
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
               ext = ".beams";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
                  any_changes = true;
               }
               // frmc = us_fopen(QString("%1.rmc").arg(fname).toLatin1().data(), "w");
               // frmc1 = us_fopen(QString("%1.rmc1").arg(fname).toLatin1().data(), "w");
            }
            
            if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
               ext = ".dat";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
                  any_changes = true;
               }
            }

            if ( somo )
            {
               ext = ".asa_res";
               file = path + bead_model_suffix + ext;
               new_file = fileNameCheck( &path, &bead_model_suffix, &ext, 0 );
               if ( file != new_file )
               {
                  le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
                  any_changes = true;
               }
            }
         }
      }
   } while ( any_changes );
}


QString US_Hydrodyn::fileNameCheck( QString filename, int mode, QWidget *p )
{
   // checks to see if file name exists, and if it does, according to 'mode'
   // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 

   if ( !QFile::exists(filename) )
   {
      return filename;
   }

   // file does exist
   // based upon mode, do next stuff

   QFileInfo fi(filename);
   QString path = fi.path() + QDir::separator();
   QString base = fi.completeBaseName();
   QString ext = fi.suffix();
   ext = ext.length() ? "." + ext : ext;
   fileNameCheck( &path, &base, &ext, mode, p);
   return path + base + ext;
}

QString US_Hydrodyn::fileNameCheck( QString *path, QString *base, QString *ext, int mode, QWidget *p )
{
   if ( !QFile::exists(*path + *base + *ext) )
   {
      return *path + *base + *ext;
   }

   if ( mode == 1 )
   {
      // split filename into pieces, do increment until !exists
      QRegExp rx("-(\\d+)$");
      do 
      {
         if ( rx.indexIn(*base) != -1 ) 
         {
            base->replace(rx, QString("-%1").arg(rx.cap(1).toInt() + 1));
         } else {
            *base += "-1";
         }
      } while ( QFile::exists(*path + *base + *ext) );
         
      return *path + *base + *ext;
   }

   // do the fancy window stuff with filename split up into pieces
   // make up a window with notice,
   // filename dirpath (fixed) base (editable) ext (fixed)
   // choices: auto-increment / try again 

   int result;

   US_Hydrodyn_File *hf = new US_Hydrodyn_File( path, base, ext, &result, p );
   fixWinButtons( hf );

   do 
   {
      hf->exec();
   } while ( result == -1 );

   switch ( result )
   {
   case 0 : // overwrite
   case 2 : // try again
      break;
   case 1 : // auto inc
      return fileNameCheck(path, base, ext, 1);
      break;
   }
   delete hf;
   return *path + *base + *ext;
}

QString US_Hydrodyn::fileNameCheck2( QString filename, 
                                     bool &cancel, 
                                     bool &overwrite_all, 
                                     int mode, QWidget *p )
{
   // checks to see if file name exists, and if it does, according to 'mode'
   // mode == 0, stop and ask with the option for an new filename, mode == 1 auto increment, 

   if ( !QFile::exists(filename) )
   {
      return filename;
   }

   // file does exist
   // based upon mode, do next stuff

   QFileInfo fi(filename);
   QString path = fi.path() + QDir::separator();
   QString base = fi.completeBaseName();
   QString ext = fi.suffix();
   ext = ext.length() ? "." + ext : ext;
   fileNameCheck2( &path, &base, &ext, cancel, overwrite_all, mode, p);
   return path + base + ext;
}

QString US_Hydrodyn::fileNameCheck2( QString *path, 
                                     QString *base, 
                                     QString *ext, 
                                     bool &cancel,
                                     bool &overwrite_all,
                                     int mode, 
                                     QWidget *p )
{
   if ( !QFile::exists(*path + *base + *ext) )
   {
      return *path + *base + *ext;
   }

   if ( mode == 1 )
   {
      // split filename into pieces, do increment until !exists
      QRegExp rx("-(\\d+)$");
      do 
      {
         if ( rx.indexIn(*base) != -1 ) 
         {
            base->replace(rx, QString("-%1").arg(rx.cap(1).toInt() + 1));
         } else {
            *base += "-1";
         }
      } while ( QFile::exists(*path + *base + *ext) );
         
      return *path + *base + *ext;
   }

   // do the fancy window stuff with filename split up into pieces
   // make up a window with notice,
   // filename dirpath (fixed) base (editable) ext (fixed)
   // choices: auto-increment / try again 

   int result;

   US_Hydrodyn_File2 *hf2 = new US_Hydrodyn_File2( path, 
                                                   base, 
                                                   ext, 
                                                   cancel,
                                                   overwrite_all,
                                                   &result, 
                                                   p );
   fixWinButtons( hf2 );

   do 
   {
      hf2->exec();
   } while ( result == -1 );

   switch ( result )
   {
   case 0 : // overwrite
   case 2 : // try again
      break;
   case 1 : // auto inc
      return fileNameCheck(path, base, ext, 1);
      break;
   }
   delete hf2;
   return *path + *base + *ext;
}

void US_Hydrodyn::dmd_run()
{
   if ( pdb_file.isEmpty() )
   {
      QMessageBox::information( this,
                                "US-SOMO: Run DMD",
                                us_tr( "You must load a PDB file before Run DMD" ) );
      return;
   }
      
   // save current batch_files
   batch_info save_batch_info = batch;
   batch.file.clear( );
   batch.file.push_back( pdb_file );
   batch.mm_all = true;
   batch.dmd = true;
   batch.somo = false;
   batch.grid = false;
   batch.iqq = false;
   batch.prr = false;
   batch.hydro = false;
   bool created_batch = false;
   if ( !batch_widget )
   {
      created_batch = true;
      batch_window = new US_Hydrodyn_Batch(&batch, &batch_widget, this);
      fixWinButtons( batch_window );
      batch_window->lb_files->item( 0)->setSelected( true );
   } else {
      batch_window->lb_files->clear( );
      batch_window->lb_files->addItem(batch.file[0]);
      batch_window->lb_files->item( 0)->setSelected( true );
   }
   batch_window->cb_mm_first->setChecked( !batch.mm_all );
   batch_window->cb_mm_all  ->setChecked( batch.mm_all );
   batch_window->cb_dmd     ->setChecked( true ); // batch.dmd );
   batch_window->cb_somo    ->setChecked( batch.somo );
   batch_window->cb_grid    ->setChecked( batch.grid );
   batch_window->cb_iqq     ->setChecked( batch.iqq );
   batch_window->cb_prr     ->setChecked( batch.prr );
   batch_window->cb_hydro   ->setChecked( batch.hydro );

   US_Hydrodyn_Cluster *hc = 
      new US_Hydrodyn_Cluster(
                              this,
                              batch_window );
   fixWinButtons( hc );
   hc->exec();
   delete hc;
   batch = save_batch_info;
   if ( created_batch )
   {
      batch_window->close();
   } else {
      batch_window->lb_files->clear( );
      QString load_errors;
      for ( unsigned int i = 0; i < batch.file.size(); i++ ) 
      {
         if ( batch.file[i].contains(QRegExp("(pdb|PDB|bead_model|BEAD_MODEL|beams|BEAMS)$")) )
         {
            bool dup = false;
            if ( i ) 
            {
               for ( unsigned int j = 0; j < i; j++ )
               {
                  if ( batch.file[i] == batch.file[j] )
                  {
                     dup = true;
                     break;
                  }
               }
            }
            if ( !dup )
            {
               batch_window->lb_files->addItem(batch.file[i]);
            } else {
               load_errors += QString(us_tr("File skipped: %1 (already in list)\n")).arg(batch.file[i]);
            }
         } else {
            load_errors += QString(us_tr("File skipped: %1 (not a valid file name)\n")).arg(batch.file[i]);
         }
      }
      if ( load_errors != "" ) 
      {
         QColor save_color = batch_window->editor->textColor();
         batch_window->editor->setTextColor("dark red");
         batch_window->editor->append(load_errors);
         batch_window->editor->setTextColor(save_color);
      }
      batch_window->cb_mm_first->setChecked( !batch.mm_all );
      batch_window->cb_mm_all  ->setChecked( batch.mm_all );
      batch_window->cb_dmd     ->setChecked( batch.dmd );
      batch_window->cb_somo    ->setChecked( batch.grid );
      batch_window->cb_grid    ->setChecked( batch.somo );
      batch_window->cb_iqq     ->setChecked( batch.iqq );
      batch_window->cb_prr     ->setChecked( batch.prr );
      batch_window->cb_hydro   ->setChecked( batch.hydro );
   }
}

void US_Hydrodyn::run_us_config()
{
   QProcess* process = new QProcess( this );
#if QT_VERSION < 0x040000
   QString config_prog = "us_config";
   
   process->setCommunication( 0 );
# ifndef Q_OS_MAC
   process->addArgument( config_prog );
# else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + config_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
# endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_config").toUpper() );
   }
#else
   QString prog = "us3_config";
   QStringList args;
# if defined( Q_OS_MAC )
   QString config_prog = "us3_config";
   prog = "open";

   QString procbin = USglobal->config_list.system_dir + "/bin/" + config_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   args
      <<  "-a"
      << procapp
      ;
# endif
   if ( ! process->startDetached( prog, args ) )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_config").toUpper() );
   }
#endif

}

void US_Hydrodyn::run_us_admin()
{
   QProcess* process = new QProcess( this );
#if QT_VERSION < 0x040000
   process->setCommunication( 0 );
# ifndef Q_OS_MAC
   process->addArgument( "us_admin" );
# else
   QString procbin = USglobal->config_list.system_dir + "/bin/" + "us_admin";
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   process->addArgument( "open" );
   process->addArgument( "-a" );
   process->addArgument( procapp );
# endif

   if ( ! process->start() )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_admin").toUpper() );
   }
#else
   QString prog = "us_admin";
   QStringList args;
# if defined( Q_OS_MAC )
   QString admin_prog = "us_admin";
   prog = "open";

   QString procbin = USglobal->config_list.system_dir + "/bin/" + admin_prog;
   QString procapp = procbin + ".app";

   if ( !QFile( procapp ).exists()  &&  QFile( procbin ).exists() )
      procapp         = procbin;

   args
      <<  "-a"
      << procapp
      ;
# endif
   if ( ! process->startDetached( prog, args ) )
   {
      QMessageBox::information( this,
                                us_tr( "Error" ),
                                us_tr( "There was a problem creating a subprocess\n"
                                    "for " ) + QString("us_admin").toUpper() );
   }
#endif
}


void US_Hydrodyn::update_enables()
{
}

void US_Hydrodyn::sizeArrows( QwtCounter* 
#if QT_VERSION >= 0x040000 && ( defined(Q_OS_MAC) || defined(Q_OS_WIN) )
                              counter 
#endif
)
{
#if QT_VERSION >= 0x040000 && ( defined(Q_OS_MAC) || defined(Q_OS_WIN) )
   QList< QObject* > children = counter->children();
   QStyle* btnstyle = QStyleFactory::create("fusion");
   for ( int jj = 0; jj < children.size(); jj++ )
   {
      QWidget* cwidg = (QWidget*)children.at( jj );
      QString clname = cwidg->metaObject()->className();
      if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
         cwidg->setStyle( btnstyle );
   }
#endif
}

void US_Hydrodyn::fixWinButtons( QWidget* 
#if QT_VERSION >= 0x040000
                                 widg 
#endif
                                 )
{
#if QT_VERSION >= 0x040000
   QList< QObject* > children = widg->children();
   QStyle* btnstyle = QStyleFactory::create("fusion");
   for ( int jj = 0; jj < children.size(); jj++ )
   {
      QObject* cobj  = children.at( jj );
      if ( !cobj->isWidgetType() )  continue;
      QWidget* cwidg = (QWidget*)cobj;
      QString clname = cwidg->metaObject()->className();
      if ( !clname.isEmpty()  &&  clname.contains( "Button" ) )
         cwidg->setStyle( btnstyle );
   }
#endif
}

bool US_Hydrodyn::equi_grid_bead_model( double dR )
{
   stopFlag = false;
   pb_stop_calc->setEnabled(true);
   append_options_log_atob();
   display_default_differences();
   bool any_errors = false;
   bool any_models = false;
   bool grid_pdb_state = pb_grid_pdb->isEnabled();
   bool somo_state = pb_grid_pdb->isEnabled();
   pb_grid_pdb->setEnabled(false);
   pb_vdw_beads->setEnabled(false);
   pb_grid->setEnabled(false);
   pb_somo->setEnabled(false);
   pb_somo_o->setEnabled(false);
   pb_visualize->setEnabled(false);
   pb_equi_grid_bead_model->setEnabled(false);
   pb_show_hydro_results->setEnabled(false);
   pb_calc_hydro->setEnabled(false);
   pb_calc_zeno->setEnabled(false);
   pb_calc_grpy->setEnabled( false);
   pb_calc_hullrad->setEnabled( false );
   if (results_widget)
   {
      results_window->close();
      delete results_window;
      results_widget = false;
   }

   // get dR

   printf( "equi_grid called with dR %g\n", dR );
   if ( !dR )
   {
      bool ok;

      do {
         dR = US_Static::getDouble(
                                      "dR for equi grid:",
                                      us_tr( "Enter a cube side value in Angstroms:" ),
                                      1e0, 
                                      -1e0,
                                      1e4, 
                                      3, 
                                      &ok, 
                                      this );
         if ( !ok )
         {
            editor->append("Stopped by user\n\n");
            pb_grid_pdb->setEnabled(true);
            pb_vdw_beads->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_somo->setEnabled(true);
            pb_somo_o->setEnabled(true);
            progress->reset();
            return false;
         }
      } while ( !dR );
   }

   bead_model_suffix = getExtendedSuffix(false, false).replace( "a2b", QString( "eqm%1" ).arg( dR ).replace( ".","_" ) ) + "g";
   le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );
   if ( !overwrite )
   {
      setSomoGridFile(false);
   }

   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
      if (lb_model->item(current_model)->isSelected()) {
         printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         if (somo_processed.size() > current_model && somo_processed[current_model]) {
            printf("in calc_grid: somo_processed %d\n", current_model); fflush(stdout);
            editor->append(QString("Gridding bead model %1\n").arg(current_model + 1));
            progress->reset();
            qApp->processEvents();

            if ( !saxs_util->grid ( bead_models[current_model],
                                    bead_model,
                                    dR,
                                    false,
                                    false ) )
            {
               editor_msg( "red", saxs_util->errormsg );
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return false;
            }
               
            if (stopFlag)
            {
               editor->append("Stopped by user\n\n");
               pb_grid_pdb->setEnabled(true);
               pb_vdw_beads->setEnabled(true);
               pb_equi_grid_bead_model->setEnabled(true);
               pb_somo->setEnabled(true);
               pb_somo_o->setEnabled(true);
               progress->reset();
               return false;
            }

            for ( unsigned int i = 0; i < ( unsigned int ) bead_model.size(); i++ )
            {
               bead_model[i].exposed_code = 1;
               bead_model[i].bead_color = 8;
               bead_model[i].chain      = 0;
               bead_model[i].visibility = 1;
               bead_model[i].active     = true;
            }

            bead_models[ current_model ] = bead_model;
            
            any_models = true;
            if (somo_processed.size() < current_model + 1) {
               somo_processed.resize(current_model + 1);
            }
            somo_processed[current_model] = 1;
            editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );

            QString filename =
               project + QString("_%1").arg(current_model + 1);

            le_bead_model_file->setText( filename );

            write_bead_model(somo_dir + SLASH + filename +
                             QString( bead_model_suffix.length() ? ("-" + bead_model_suffix) : "" ) +
                             DOTSOMO, &bead_model);
         }
      }
   }

   if (stopFlag)
   {
      editor->append("Stopped by user\n\n");
      pb_grid_pdb->setEnabled(true);
      pb_vdw_beads->setEnabled(true);
      pb_somo->setEnabled(true);
      pb_somo_o->setEnabled(true);
      progress->reset();
      return false;
   }

   if (any_models && !any_errors)
   {
      editor->append("Build bead model completed\n");
      qApp->processEvents();
      pb_visualize->setEnabled(true);
      pb_equi_grid_bead_model->setEnabled(true);
      pb_calc_hydro->setEnabled(true);
      pb_calc_zeno->setEnabled(true);
      pb_calc_grpy->setEnabled( true );
      pb_calc_hullrad->setEnabled( true );
      pb_bead_saxs->setEnabled(true);
      pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      editor->append("Errors encountered\n");
   }

   pb_equi_grid_bead_model->setEnabled(true);
   pb_grid_pdb->setEnabled(grid_pdb_state);
   pb_vdw_beads->setEnabled(grid_pdb_state);
   pb_grid->setEnabled(true);
   pb_somo->setEnabled(somo_state);
   pb_somo_o->setEnabled(somo_state);
   pb_stop_calc->setEnabled(false);
   if (calcAutoHydro) {
      switch ( misc.auto_calc_hydro_method ) {
      case AUTO_CALC_HYDRO_SMI :
         calc_hydro();
         break;
      case AUTO_CALC_HYDRO_ZENO :
         calc_zeno_hydro();
         break;
      case AUTO_CALC_HYDRO_GRPY :
         calc_grpy_hydro();
         break;
      default :
         editor_msg( "red", us_tr( "No known hydrodynamic method set for automatic hydrodynamic calculations\n"
                                   "Check SOMO->Miscellaneous Options->Automatically calculate hydrodynamics method" ) );
         break;
      }
   } else {
      play_sounds(1);
   }

   return true;
}

bool US_Hydrodyn::calc_zeno_hydro()
{
    return (int)(!calc_zeno());
}

void US_Hydrodyn::le_pdb_file_focus( bool )
{
   // us_qdebug( "focused" );
   le_pdb_file_save_text = le_pdb_file->text();
}

void US_Hydrodyn::le_pdb_file_changed( const QString & )
{
   // us_qdebug( "changed" );
   le_pdb_file->setText( le_pdb_file_save_text );
}

void US_Hydrodyn::update_progress( int pos, int total )
{
   // us_qdebug( QString( "update progress %1 %2" ).arg( pos ).arg( total ) );
   if ( mm_mode ) {
      mprogress->setValue( pos ); mprogress->setMaximum( total );
   } else {
      progress->setValue( pos ); progress->setMaximum( total );
   }
}

void US_Hydrodyn::do_update_progress( int pos, int total ) {
   // us_qdebug( QString( "do update progress %1 %2" ).arg( pos ).arg( total ) );
   emit progress_updated( pos, total );
}

void US_Hydrodyn::process_events() {
   us_qdebug( QString( "process_events()?" ) );
   //   qApp->processEvents();
}


QString US_Hydrodyn::get_somo_dir() {
   if ( !batch_widget ||
        !batch.results_dir ||
        batch.results_dir_name.isEmpty() ) {
      return somo_dir;
   }

   //  check && create if needed
   QString dir_path = somo_dir + QDir::separator() + batch.results_dir_name;
   // QTextStream( stderr ) << "somo results dir is '" + dir_path + "'\n";

   QDir dir( dir_path );
   if ( !dir.exists() ) {
      dir.mkdir( dir_path );
   }
   if ( dir.exists() ) {
      return dir_path;
   }
   batch_window->editor_msg( "red"
                             ,QString( us_tr( "WARNING: results directory %1 could not be created, using %2" ) )
                             .arg( dir.path() )
                             .arg( somo_dir )
                             );
   return somo_dir;
}

bool US_Hydrodyn::batch_avg_hydro_active() {
   return batch_widget && batch_window->batch_job_running && batch.avg_hydro;
}

bool US_Hydrodyn::batch_active() {
   return batch_widget && batch_window->batch_job_running;
}
