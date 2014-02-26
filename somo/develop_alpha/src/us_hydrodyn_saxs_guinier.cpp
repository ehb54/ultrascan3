// #include "../include/us_sas_dammin.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_options.h"
#include "../include/us_hydrodyn_saxs_load_csv.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn_saxs_screen.h"
#include "../include/us_hydrodyn_saxs_search.h"
#include "../include/us_hydrodyn_saxs_buffer.h"
#include "../include/us_hydrodyn_saxs_hplc.h"
#include "../include/us_hydrodyn_saxs_1d.h"
#include "../include/us_hydrodyn_saxs_2d.h"
#include "../include/us_hydrodyn_xsr.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_math.h"
#include <qwaitcondition.h>
#include "qwt_symbol.h"
#ifdef QT4
# include <qwt_scale_engine.h>
//Added by qt3to4:
#include <Q3TextStream>
#endif

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
#  define isnan _isnan
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

static vector < QString >                    guinier_names;
static vector < vector  < double > >         guinier_q2;
static vector < vector  < double > >         guinier_lnI;
static vector < vector  < double > >         guinier_sd;
static vector < map     < double, double > > guinier_removed;

static void clear_csv_data()
{
   guinier_names  .clear();
   guinier_q2     .clear();
   guinier_lnI    .clear();
   guinier_sd     .clear();
   guinier_removed.clear();
}

static void write_csv_data( QFile * f, QString I_tag )
{
   int maxlen = 0;
   for ( int i = 0; i < (int)guinier_q2.size(); ++i )
   {
      if ( maxlen < (int)guinier_q2[ i ].size() )
      {
         maxlen = (int)guinier_q2[ i ].size();
      }
   }

   vector < QStringList > out( maxlen + 1 ); // +1 for header

   // output a column at a time

   for ( int i = 0; i < (int)guinier_names.size(); ++i )
   {
      out[ 0 ] << QString( "\"%1 q^2\"" ).arg( guinier_names[ i ] );
      for ( int j = 0; j < maxlen; ++j )
      {
         out[ j + 1 ] << ( j < (int) guinier_q2[ i ].size() ? QString( "%1" ).arg( guinier_q2[ i ][ j ], 0, 'g', 8 ) : QString( "" ) );
      }

      out[ 0 ] << QString( "\"%1 %2\"" ).arg( guinier_names[ i ] ).arg( I_tag );

      for ( int j = 0; j < maxlen; ++j )
      {
         out[ j + 1 ] << ( j < (int) guinier_lnI[ i ].size() ? QString( "%1" ).arg( guinier_lnI[ i ][ j ], 0, 'g', 8 ) : QString( "" ) );
      }

      if ( guinier_sd[ i ].size() )
      {
         out[ 0 ] << QString( "\"%1 S.D.\"" ).arg( guinier_names[ i ] );
         for ( int j = 0; j < maxlen; ++j )
         {
            out[ j + 1 ] << ( j < (int) guinier_sd[ i ].size() ? QString( "%1" ).arg( guinier_sd[ i ][ j ], 0, 'g', 8 ) : QString( "" ) );
         }
         if ( guinier_removed[ i ].size() )
         {
            out[ 0 ] << QString( "\"%1 removed\"" ).arg( guinier_names[ i ] );
            for ( int j = 0; j < maxlen; ++j )
            {
               out[ j + 1 ] <<
                  ( j < (int) guinier_q2[ i ].size() && guinier_removed[ i ].count( guinier_q2[ i ][ j ] ) ? "\"X\"" : "" );
            }
         }
      }
   }

   Q3TextStream ts( f );

   for ( int i = 0; i <= maxlen; ++i )
   {
      ts << out[ i ].join(",") << endl;
   }

   f->close();
   clear_csv_data();
}

void US_Hydrodyn_Saxs::run_guinier_Rt()
{
   clear_csv_data();
   editor->append("TV Guinier analysis:\n");
   clear_Rt_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
      "\"Rt^2\","
      "\"Rt^2 sd\","
      "\"Rt\","
      "\"Rt sd\","
      "\"I(0)\","
      "\"I(0) sd\","
      "\"M/A\","
      "\"M/A sd\","
      "\"Delta_e(p-s)\","
      "\"q min\","
      "\"q max\","
      "\"q*Rc min\","
      "\"q*Rc max\","
      "\"starting point\","
      "\"ending point\","
      "\"points used\","
      "\"rejected points\","
      "\"chi^2\","
      "\"reduced chi\","
      "\"Conc. (mg/ml)\","
      "\"PSV (ml/g)\","
      "\"I0 std. expt. (ml/g)\","
      "\"Slope\","
      "\"Slope SD\","
      "\"SD weighting\","
      ;

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" )
   {
      csvlog += "\"Automatic range fitting used\",";
   } else {
      csvlog += "\"Manual range fitting used\",";
   }      
   csvlog += "\n";
   
   for ( unsigned int i = 0; 
#ifndef QT4
         i < plotted_Iq.size();
#else
         i < plotted_Iq_curves.size();
#endif
         i++ )
   {
      Rt_guinier_analysis(i, csvlog);
   }
   cb_guinier->setChecked(true);
   cb_cs_guinier->setChecked(false);
   cb_Rt_guinier->setChecked(true);
   set_guinier();

   if ( our_saxs_options->guinier_csv )
   {
      QString filename = 
         USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
         our_saxs_options->guinier_csv_filename + "_tv.csv";

      QString fname = filename;
      if ( QFile::exists(fname) )
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
      }

      QFile f( fname );

      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
      } else {
         Q3TextStream ts(&f);
         ts << csvlog;
         f.close();
         editor->append(QString(tr("Created file %1\n")).arg(f.name()));
      }
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
      {
         QString filename = 
            USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
            our_saxs_options->guinier_csv_filename + "_tv_data.csv";

         QString fname = filename;
         if ( QFile::exists(fname) )
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
         }

         QFile f( fname );

         if ( !f.open(QIODevice::WriteOnly) )
         {
            editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
         } else {
            write_csv_data( & f, "ln q*I" );
            editor->append(QString(tr("Created file %1\n")).arg(f.name()));
         }
      }
   }
}

void US_Hydrodyn_Saxs::run_guinier_cs()
{
   clear_csv_data();
   editor->append("CS Guinier analysis:\n");
   clear_cs_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
      "\"Rc^2\","
      "\"Rc^2 sd\","
      "\"Rc\","
      "\"Rc sd\","
      "\"I(0)\","
      "\"I(0) sd\","
      "\"M/L\","
      "\"M/L sd\","
      "\"Delta_e(p-s)\","
      "\"q min\","
      "\"q max\","
      "\"q*Rc min\","
      "\"q*Rc max\","
      "\"starting point\","
      "\"ending point\","
      "\"points used\","
      "\"rejected points\","
      "\"chi^2\","
      "\"reduced chi\","
      "\"Conc. (mg/ml)\","
      "\"PSV (ml/g)\","
      "\"I0 std. expt. (ml/g)\","
      "\"Slope\","
      "\"Slope SD\","
      "\"SD weighting\","
      ;

   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" )
   {
      csvlog += "\"Automatic range fitting used\",";
   } else {
      csvlog += "\"Manual range fitting used\",";
   }      
   csvlog += "\n";
   
   for ( unsigned int i = 0; 
#ifndef QT4
         i < plotted_Iq.size();
#else
         i < plotted_Iq_curves.size();
#endif
         i++ )
   {
      cs_guinier_analysis(i, csvlog);
   }
   cb_guinier->setChecked(true);
   cb_Rt_guinier->setChecked(false);
   cb_cs_guinier->setChecked(true);
   set_guinier();

   if ( our_saxs_options->guinier_csv )
   {
      QString filename = 
         USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
         our_saxs_options->guinier_csv_filename + "_cs.csv";

      QString fname = filename;
      if ( QFile::exists(fname) )
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
      }

      QFile f( fname );

      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
      } else {
         Q3TextStream ts(&f);
         ts << csvlog;
         f.close();
         editor->append(QString(tr("Created file %1\n")).arg(f.name()));
      }
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
      {
         QString filename = 
            USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
            our_saxs_options->guinier_csv_filename + "_cs_data.csv";

         QString fname = filename;
         if ( QFile::exists(fname) )
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
         }

         QFile f( fname );

         if ( !f.open(QIODevice::WriteOnly) )
         {
            editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
         } else {
            write_csv_data( & f, "ln q*I" );
            editor->append(QString(tr("Created file %1\n")).arg(f.name()));
         }
      }
   }
}

void US_Hydrodyn_Saxs::run_guinier_analysis()
{
   clear_csv_data();
   editor->append("Guinier analysis:\n");
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, plot_saxs->canvasBackground() );
   clear_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
      "\"Rg^2\","
      "\"Rg^2 sd\","
      "\"Rg\","
      "\"Rg sd\","
      "\"I(0)\","
      "\"I(0) sd\","
      "\"MW\","
      "\"MW sd\","
      "\"Delta_e(p-s)\","
      "\"q min\","
      "\"q max\","
      "\"q*Rg min\","
      "\"q*Rg max\","
      "\"starting point\","
      "\"ending point\","
      "\"points used\","
      "\"rejected points\","
      "\"chi^2\","
      "\"reduced chi\","
      "\"Conc. (mg/ml)\","
      "\"PSV (ml/g)\","
      "\"I0 std. expt. (ml/g)\","
      "\"Slope\","
      "\"Slope SD\","
      "\"SD weighting\","
      ;
   
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" )
   {
      csvlog += "\"Automatic range fitting used\",";
   } else {
      csvlog += "\"Manual range fitting used\",";
   }      
   csvlog += "\n";

   for ( unsigned int i = 0; 
#ifndef QT4
         i < plotted_Iq.size();
#else
         i < plotted_Iq_curves.size();
#endif
         i++ )
   {
      guinier_analysis(i, csvlog);
   }
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
   cb_guinier->setChecked(true);
   cb_Rt_guinier->setChecked(false);
   cb_cs_guinier->setChecked(false);
   set_guinier();

   if ( our_saxs_options->guinier_csv )
   {
      QString filename = 
         USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
         our_saxs_options->guinier_csv_filename + ".csv";

      QString fname = filename;
      if ( QFile::exists(fname) )
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
      }

      QFile f( fname );

      if ( !f.open(QIODevice::WriteOnly) )
      {
         editor->append(QString(tr("Can not create file %1\n")).arg(f.name()));
      } else {
         Q3TextStream ts(&f);
         ts << csvlog;
         f.close();
         editor->append(QString(tr("Created file %1\n")).arg(f.name()));
      }

      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
      {
         QString filename = 
            USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "saxs" + SLASH + 
            our_saxs_options->guinier_csv_filename + "_data.csv";

         QString fname = filename;
         if ( QFile::exists(fname) )
         {
            fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname, 0, this);
         }

         QFile f( fname );

         if ( !f.open(QIODevice::WriteOnly) )
         {
            editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
         } else {
            write_csv_data( & f, "ln I" );
            editor->append(QString(tr("Created file %1\n")).arg(f.name()));
         }
      }
   }
}

bool US_Hydrodyn_Saxs::guinier_analysis( unsigned int i, QString &csvlog )
{
   if (
#ifndef QT4 
       i > plotted_Iq.size() 
#else
       i > plotted_Iq_curves.size() 
#endif
       )
   {
      editor->append("internal error: invalid plot selected\n");
      return false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q.clear();
   usu.wave["data"].r.clear();
   usu.wave["data"].s.clear();

   bool use_SD_weighting = our_saxs_options->guinier_use_sd;
   bool use_guinier_outlier_reject = our_saxs_options->guinier_outlier_reject;

   if ( use_SD_weighting )
   {
      if ( plotted_I_error[ i ].size() != plotted_q[ i ].size() )
      {
         editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are not fully present" ) )
                     .arg( qsl_plotted_iq_names[ i ] ) );
         use_SD_weighting = false;
      } else {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                 plotted_q[ i ][ j ] <= our_saxs_options->qend )
            {
               if ( plotted_I_error[ i ][ j ] <= 0e0 )
               {
                  editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since at least one SD is zero or negative in the selected q range" ) )
                              .arg( qsl_plotted_iq_names[ i ] ) );
                  use_SD_weighting = false;
                  break;
               }
            }
         }
      }
   }
      
   if ( use_guinier_outlier_reject && !use_SD_weighting )
   {
      editor_msg( "dark red", QString( tr( "Notice: Turning off outlier rejection for Guinier fit of %1 since SD weighting is off" ) )
                  .arg( qsl_plotted_iq_names[ i ] ) );

      use_guinier_outlier_reject = false;
   }

   //    if ( use_SD_weighting )
   //    {
   //       cout << "uhsg: using sd weighting\n";
   //    } else {
   //       cout << "uhsg: not using sd weighting\n";
   //    }

   // vector < double > q2;
   for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
   {
      if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
           plotted_q[ i ][ j ] <= our_saxs_options->qend )
      {
         usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
         // q2.push_back(  plotted_q2[ i ][ j ] );
         usu.wave["data"].r.push_back( plotted_I[ i ][ j ] );
         if ( use_SD_weighting )
         {
            usu.wave["data"].s.push_back( plotted_I_error[ i ][ j ] );
         }
      }
   }
   
   // US_Vector::printvector3( "q2 I e", q2 /* usu.wave["data"].q */, usu.wave["data"].r, usu.wave["data"].s );

   QString this_log;

   unsigned int pointsmin = our_saxs_options->pointsmin;
   if ( pointsmin < 2 )
   {
      pointsmin = 2;
   }
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = our_saxs_options->qRgmax;
   double pointweightpower = 3e0;
   // double p_guinier_minq = our_saxs_options->qstart;
   double p_guinier_maxq = our_saxs_options->qend;
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double I0;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[i].size() <= 5;

   unsigned int pts_removed = 0;
   QString      qs_removed  = "";
   QString      internal_error = "";

   if ( !too_few_points )
   {
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" &&
           (unsigned int) usu.wave[ "data" ].q.size() > pointsmin )
      {
         if ( 
             !usu.guinier_plot(
                               "guinier",
                               "data"
                               )   ||
             !usu.guinier_fit2(
                               this_log,
                               "guinier", 
                               pointsmin,
                               pointsmax,
                               sRgmaxlimit,
                               pointweightpower,
                               p_guinier_maxq,
                               a,
                               b,
                               siga,
                               sigb,
                               chi2,
                               Rg,
                               I0,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend
                               ) )
         {
            editor->append(QString("Error performing Guinier analysis on %1\n" + usu.errormsg + "\n")
                           .arg(qsl_plotted_iq_names[i]));
            return false;
         }
         // cout << QString( "after af: smin^2 %1 smax^2 %2\n" ).arg( smin * smin ).arg( smax * smax );
      } else {
         bool do_decrease = 
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_use_qRlimit" ) &&
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_use_qRlimit" ] == "1";
         unsigned int pts_decrease = 0;
         //          if ( do_decrease )
         //          {
         //             printf( "do decrease, pts_decrease %u\n", pts_decrease );
         //          }
         do {
            usu.wave["data"].q.clear();
            usu.wave["data"].r.clear();
            usu.wave["data"].s.clear();
            qs_removed = "";
            // vector < double > q2;
            for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
            {
               if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                    plotted_q[ i ][ j ] <= our_saxs_options->qend )
               {
                  usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
                  // q2.push_back(  plotted_q2[ i ][ j ] );
                  usu.wave["data"].r.push_back( plotted_I[ i ][ j ] );
                  if ( use_SD_weighting )
                  {
                     usu.wave["data"].s.push_back( plotted_I_error[ i ][ j ] );
                  }
               }
            }

            if ( pts_decrease )
            {
               if ( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease < 5 )
               {
                  internal_error = QString( tr( "Too few points left (%1) after %2 points removed for qRgmax limit" ) )
                     .arg( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease ).arg( pts_decrease ); 
                  // editor->append( QString( "Error performing Guinier analysis on %1: %2\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( internal_error ) );
                  break;
               }

               usu.wave[ "data" ].q.resize( usu.wave[ "data" ].q.size() - pts_decrease );
               usu.wave[ "data" ].r.resize( usu.wave[ "data" ].q.size() );
               if ( use_SD_weighting )
               {
                  usu.wave[ "data" ].s.resize( usu.wave[ "data" ].q.size() );
               }
            }

            //             if ( do_decrease )
            //             {
            //                US_Vector::printvector3( "q I e", usu.wave["data"].q, usu.wave["data"].r, usu.wave["data"].s );
            //             }

            unsigned int pstart = 0;
            unsigned int pend   = usu.wave[ "data" ].q.size() ? usu.wave[ "data" ].q.size() - 1 : 0;
            bestend = pend;
            beststart = pstart;

            if ( use_guinier_outlier_reject )
            {
               // puts( "outlier reject" );
               US_Vector::printvector( "'data'.q orig", usu.wave[ "data" ].q ); 
               map < double, double > removed;
               if ( 
                   !usu.guinier_plot(
                                     "guinier",
                                     "data"
                                     )   ||
                   !usu.guinier_fit_with_removal(
                                                 this_log,
                                                 "guinier", 
                                                 pstart,
                                                 pend,
                                                 a,
                                                 b,
                                                 siga,
                                                 sigb,
                                                 chi2,
                                                 Rg,
                                                 I0,
                                                 smax, // don't know why these are flipped
                                                 smin,
                                                 sRgmin,
                                                 sRgmax,
                                                 our_saxs_options->guinier_outlier_reject_dist,
                                                 removed,
                                                 pts_removed,
                                                 false
                                                 ) )
               {
                  editor->append(QString("Error performing Guinier analysis on %1\n" + usu.errormsg + "\n")
                                 .arg(qsl_plotted_iq_names[i]));
                  return false;
               }
               // cout << QString( "removed size: %1 pts_removed %2\n" ).arg( removed.size() ).arg( pts_removed );
               if ( removed.size() )
               {
                  plotted_guinier_pts_removed[ i ] = removed;
                  // US_Vector::printvector( "'data'.q", usu.wave[ "data" ].q ); 
                  // US_Vector::printvector( "'guinier'.q", usu.wave[ "guinier" ].q ); 
                  // vector < double > f;
                  // vector < double > q2;

                  map < QString, bool > removed_match;
                  for ( map < double, double >::iterator it = removed.begin();
                        it != removed.end();
                        it++ )
                  {
                     // f.push_back( it->first );
                     removed_match[ QString( "%1" ).arg( it->first ) ] = true;
                  }
                  // US_Vector::printvector( "removed first value", f ); 

                  for ( int j = 0; j < (int)usu.wave[ "data" ].q.size(); ++j )
                  {
                     // q2.push_back( usu.wave[ "data" ].q[ j ] * usu.wave[ "data" ].q[ j ] );
                     if ( removed_match.count( QString( "%1" ).arg( usu.wave[ "data" ].q[ j ] * usu.wave[ "data" ].q[ j ] ) ) )
                     {
                        qs_removed += QString( "%1%2" ).arg( qs_removed.isEmpty() ? "" : "_" ).arg( j + 1 );
                     }
                  }
                  // cout << "qs_removed:" << qs_removed << endl;
                  // US_Vector::printvector( "q2 computed from wave['data']", q2 ); 
               } else {
                  plotted_guinier_pts_removed.erase( i );
               }
               // cout << QString( "after gor: smin^2 %1 smax^2 %2\n" ).arg( smin * smin ).arg( smax * smax );
            } else {
               plotted_guinier_pts_removed.erase( i );
               if ( 
                   !usu.guinier_plot(
                                     "guinier",
                                     "data"
                                     )   ||
                   !usu.guinier_fit(
                                    this_log,
                                    "guinier", 
                                    pstart,
                                    pend,
                                    a,
                                    b,
                                    siga,
                                    sigb,
                                    chi2,
                                    Rg,
                                    I0,
                                    smax, // don't know why these are flipped
                                    smin,
                                    sRgmin,
                                    sRgmax
                                    ) )
               {
                  editor->append(QString("Error performing Guinier analysis on %1\n" + usu.errormsg + "\n")
                                 .arg(qsl_plotted_iq_names[i]));
                  return false;
               }
               // cout << QString( "after g: smin^2 %1 smax^2 %2\n" ).arg( smin * smin ).arg( smax * smax );
            }
            //             if ( do_decrease )
            //             {
            //                printf( "do decrease, sRgmax %g sRgmaxlimit %g\n", sRgmax, sRgmaxlimit );
            //             }
               
            do_decrease = do_decrease && ( sRgmax > sRgmaxlimit );
            if ( do_decrease )
            {
               // puts( "decreasing one point" );
               pts_decrease++;
            }
         } while ( do_decrease );
      }

      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }
      
   QString report;

   if ( too_few_points ||
        ( bestend - beststart + 1 - pts_removed < pointsmin ) ||
        !internal_error.isEmpty() )
   {
      if ( !internal_error.isEmpty() )
      {
         report =
            QString(
                    "Guinier analysis of %1:\n"
                    "**** %2 ****\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
         csvlog += 
            QString(
                    "\"%1\","
                    "\"%2\"\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
      } else {        
         if ( pts_removed )
         {
            report =
               QString(
                       "Guinier analysis of %1:\n"
                       "**** Could not compute Rg, too few data points %2 after removal ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2) after removal\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
         } else {
            report =
               QString(
                       "Guinier analysis of %1:\n"
                       "**** Could not compute Rg, too few data points %2 ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;

            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2)\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;
         }
      }
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         //          if ( isnan( Rg ) )
         //          {
         //             cout << "rg isnan\n";
         //          }
         //          if ( b>= 0e0 )
         //          {
         //             cout << "b >= 0e0";
         //          }
         plotted_guinier_valid[ i ] = false;
         report =
            QString(
                    "Guinier analysis of %1:\n"
                    "**** Could not compute Rg ****\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rg\"\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

      } else {
         double slope = Rg * Rg / -3e0;
         double slopesd = Rg != 0e0 ? -2e0 * slope * sigb / Rg : 0e0;
         // cout << QString( "slope %1 slope sd %2\n" ).arg( slope ).arg( slopesd );

         double MW;
         double MW_sd;
         double ICL;

         //          if ( mw_from_I0( qsl_plotted_iq_names[ i ], siga, MW_sd, ICL ) )
         //          {
         //             // cout << QString( "siga %1 MW_sd %2 ICL %3\n" ).arg( siga ).arg( MW_sd ).arg( ICL );
         //          } else {
         //             // cout << errormsg << endl;
         //          }

         if ( !mw_from_I0( qsl_plotted_iq_names[ i ], I0, MW, ICL ) )
         {
            editor_msg( "red", errormsg );
            // cout << errormsg;
         } else {
            // cout << QString( "I0 %1 MW %2 ICL %3\n" ).arg( I0 ).arg( MW ).arg( ICL );
         }

         MW_sd = I0 ? MW * siga / I0 : 0e0;

         report = 
            QString("")
            .sprintf(
                     "Guinier analysis of %s:\n"
                     "Rg %.1f (%.1f) (A) I(0) %.2e (%.2e) MW %.2e (%.2e) q^2 [ %.5f:%.5f] qRgmin %.3f qRgmax %.3f points used %u chi^2 %.2e reduced-chi %.2e\n"
                     , qsl_plotted_iq_names[ i ].ascii()
                     , Rg
                     , sigb
                     , I0
                     , siga
                     , MW
                     , MW_sd
                     , smin * smin
                     , smax * smax
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1 - pts_removed
                     , chi2
                     , sqrt( chi2 / (bestend - beststart - 1 - pts_removed ) )
                     );
         
         plotted_guinier_valid[ i ] = true;
         plotted_guinier_lowq2[ i ] = smin * smin;
         plotted_guinier_highq2[ i ] = smax * smax;
         plotted_guinier_a[ i ] = a;
         plotted_guinier_b[ i ] = b;
         plotted_guinier_plotted[ i ] = false;
         
         plotted_guinier_x[ i ].clear();
         plotted_guinier_x[ i ].push_back(plotted_guinier_lowq2[ i ]);
         plotted_guinier_x[ i ].push_back(plotted_guinier_highq2[ i ]);
         
         plotted_guinier_y[ i ].clear();
         plotted_guinier_y[ i ].push_back(exp(plotted_guinier_a[ i ] + plotted_guinier_b[ i ] * plotted_guinier_lowq2[ i ]));
         plotted_guinier_y[ i ].push_back(exp(plotted_guinier_a[ i ] + plotted_guinier_b[ i ] * plotted_guinier_highq2[ i ]));

         /*
         cout << QString( "guinier line range q %1 %2 q^2 %3 %4 a %5 b %6\n" )
            .arg( smin  )
            .arg( smax )
            .arg( plotted_guinier_lowq2[ i ] )
            .arg( plotted_guinier_highq2[ i ] )
            .arg( a )
            .arg( b )
            ;
         */
            
         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\"," )
            .arg( qsl_plotted_iq_names[ i ] )
            ;

         csvlog += 
            QString(
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    "%9,"
                    )
            .arg( Rg * Rg )
            .arg( 3e0 * slopesd )
            .arg( Rg )
            .arg( sigb )
            .arg( I0 )
            .arg( siga )
            .arg( MW )
            .arg( MW_sd )
            .arg( ICL )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    )
            .arg( smin )
            .arg( smax )
            .arg( sRgmin )
            .arg( sRgmax )
            .arg( beststart + 1 )
            .arg( bestend + 1 )
            .arg( bestend - beststart + 1 - pts_removed )
            .arg( qs_removed )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    )
            .arg( chi2 )
            .arg( sqrt( chi2 / ( bestend - beststart - 1 - pts_removed ) ) )
            ;

         {
            double conc;
            double psv;
            double I0_std_exp;
            
            get_conc_csv_values( qsl_plotted_iq_names[ i ], conc, psv, I0_std_exp );
            csvlog += 
               QString( 
                       "%1,"
                       "%2,"
                       "%3,"
                       "%4,"
                       "%5,"
                       "\"%6\","
                       )
               .arg( conc )
               .arg( psv )
               .arg( I0_std_exp )
               .arg( slope  )
               .arg( slopesd )
               .arg( tr( use_SD_weighting ? "used" : "not used" ) )
               ;
         }

         csvlog += "\n";
      }
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
   {
      guinier_names.push_back( qsl_plotted_iq_names[ i ] );
      vector < double > q2;
      vector < double > lnI;
      vector < double > sd;
      map < double, double > empty_removed;
      double mw;
      double ICL;
      for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
      {
         q2 .push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] );
         lnI.push_back( plotted_I[ i ][ j ] > 0e0 ? log( plotted_I[ i ][ j ] ) : 0e0 );
         if ( !mw_from_I0( qsl_plotted_iq_names[ i ], exp( lnI.back() ), mw, ICL ) )
         {
            cout << QString( "mw from i0 issue %1 %2 %3\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( lnI.back() ).arg( mw );
         }
         lnI.back() = log( mw );
         if ( use_SD_weighting )
         {
            sd.push_back( plotted_I[ i ][ j ] > 0e0 ? 
                          ( lnI.back() / log( plotted_I[ i ][ j ] ) ) * fabs( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
                          // fabs( lnI.back() ) * ( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
         }
      }
      
      guinier_q2 .push_back( q2  );
      guinier_lnI.push_back( lnI );
      guinier_sd .push_back( sd  );
      if ( use_guinier_outlier_reject )
      {
         guinier_removed.push_back( plotted_guinier_pts_removed.count( i ) ?
                                    plotted_guinier_pts_removed[ i ] : empty_removed 
                                    );
      } else {
         guinier_removed.push_back( empty_removed );
      }         
   }
   editor_msg( plot_colors[i % plot_colors.size()], report );

   //   cout << csvlog;
   return true;
}

bool US_Hydrodyn_Saxs::cs_guinier_analysis( unsigned int i, QString &csvlog )
{
   if (
#ifndef QT4 
       i > plotted_Iq.size() 
#else
       i > plotted_Iq_curves.size() 
#endif
       )
   {
      editor->append("internal error: invalid plot selected\n");
      return false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q.clear();
   usu.wave["data"].r.clear();
   usu.wave["data"].s.clear();

   bool use_SD_weighting = our_saxs_options->guinier_use_sd;
   bool use_guinier_outlier_reject = our_saxs_options->guinier_outlier_reject;

   if ( use_SD_weighting )
   {
      if ( plotted_I_error[ i ].size() != plotted_q[ i ].size() )
      {
         editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are not fully present" ) )
                     .arg( qsl_plotted_iq_names[ i ] ) );
         use_SD_weighting = false;
      } else {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                 plotted_q[ i ][ j ] <= our_saxs_options->qend )
            {
               if ( plotted_I_error[ i ][ j ] <= 0e0 )
               {
                  editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since at least one SD is zero or negative in the selected q range" ) )
                              .arg( qsl_plotted_iq_names[ i ] ) );
                  use_SD_weighting = false;
                  break;
               }
            }
         }
      }
   }

   if ( use_guinier_outlier_reject && !use_SD_weighting )
   {
      editor_msg( "dark red", QString( tr( "Notice: Turning off outlier rejection for Guinier fit of %1 since SD weighting is off" ) )
                  .arg( qsl_plotted_iq_names[ i ] ) );

      use_guinier_outlier_reject = false;
   }

   // vector < double > q2;

   for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
   {
      if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
           plotted_q[ i ][ j ] <= our_saxs_options->qend )
      {
         usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
         // q2.push_back(  plotted_q2[ i ][ j ] );
         usu.wave["data"].r.push_back( plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
         if ( use_SD_weighting )
         {
            usu.wave["data"].s.push_back( plotted_q[ i ][ j ] * plotted_I_error[ i ][ j ] );
         }
      }
   }

   // US_Vector::printvector3( "cs1 q2 I e", q2 /* usu.wave["data"].q */, usu.wave["data"].r, usu.wave["data"].s );

   QString this_log;

   unsigned int pointsmin = our_saxs_options->pointsmin;
   if ( pointsmin < 2 )
   {
      pointsmin = 2;
   }
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = our_saxs_options->cs_qRgmax;
   double pointweightpower = 3e0;
   // double p_guinier_minq = our_saxs_options->qstart;
   double p_guinier_maxq = our_saxs_options->qend;

   // cout << QString( "qstart %1 qend %2\n" ).arg( p_guinier_minq ).arg( p_guinier_maxq );
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double I0;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[ i ].size() <= 5;

   unsigned int pts_removed = 0;
   QString      qs_removed  = "";
   QString      internal_error = "";

   if ( !too_few_points )
   {
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" &&
           (unsigned int)usu.wave[ "data" ].q.size() > pointsmin )
      {
         if ( 
             !usu.guinier_plot(
                               "cs_guinier",
                               "data"
                               )   ||
             !usu.guinier_fit2(
                               this_log,
                               "cs_guinier", 
                               pointsmin,
                               pointsmax,
                               sRgmaxlimit,
                               pointweightpower,
                               p_guinier_maxq,
                               a,
                               b,
                               siga,
                               sigb,
                               chi2,
                               Rg,
                               I0,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend,
                               true
                               ) )
         {
            editor_msg( "red",
                        QString("Error performing CS Guinier analysis on %1\n" + usu.errormsg + "\n")
                        .arg(qsl_plotted_iq_names[ i ]));
            return false;
         }
      } else {
         bool do_decrease = 
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_use_qRlimit" ) &&
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_use_qRlimit" ] == "1";
         unsigned int pts_decrease = 0;
         //          if ( do_decrease )
         //          {
         //             printf( "do decrease, pts_decrease %u\n", pts_decrease );
         //          }
         do {
            usu.wave["data"].q.clear();
            usu.wave["data"].r.clear();
            usu.wave["data"].s.clear();
            qs_removed = "";

            // vector < double > q2;

            for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
            {
               if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                    plotted_q[ i ][ j ] <= our_saxs_options->qend )
               {
                  usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
                  // q2.push_back(  plotted_q2[ i ][ j ] );
                  usu.wave["data"].r.push_back( plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
                  if ( use_SD_weighting )
                  {
                     usu.wave["data"].s.push_back( plotted_q[ i ][ j ] * plotted_I_error[ i ][ j ] );
                  }
               }
            }

            if ( pts_decrease )
            {
               if ( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease < 5 )
               {
                  internal_error = QString( tr( "Too few points left (%1) after %2 points removed for qRgmax limit" ) )
                     .arg( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease ).arg( pts_decrease ); 
                  // editor->append( QString( "Error performing CS Guinier analysis on %1: %2\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( internal_error ) );
                  break;
               }

               usu.wave[ "data" ].q.resize( usu.wave[ "data" ].q.size() - pts_decrease );
               usu.wave[ "data" ].r.resize( usu.wave[ "data" ].q.size() );
               if ( use_SD_weighting )
               {
                  usu.wave[ "data" ].s.resize( usu.wave[ "data" ].q.size() );
               }
            }

            // US_Vector::printvector3( "cs2 q2 I e", q2 /* usu.wave["data"].q */, usu.wave["data"].r, usu.wave["data"].s );

            unsigned int pstart = 0;
            unsigned int pend   = usu.wave[ "data" ].q.size() ? usu.wave[ "data" ].q.size() - 1 : 0;
            bestend = pend;
            beststart = pstart;

            if ( use_guinier_outlier_reject )
            {
               // puts( "outlier reject" );
               map < double, double > removed;
               if ( 
                   !usu.guinier_plot(
                                     "guinier",
                                     "data"
                                     )   ||
                   !usu.guinier_fit_with_removal(
                                                 this_log,
                                                 "guinier", 
                                                 pstart,
                                                 pend,
                                                 a,
                                                 b,
                                                 siga,
                                                 sigb,
                                                 chi2,
                                                 Rg,
                                                 I0,
                                                 smax, // don't know why these are flipped
                                                 smin,
                                                 sRgmin,
                                                 sRgmax,
                                                 our_saxs_options->guinier_outlier_reject_dist,
                                                 removed,
                                                 pts_removed,
                                                 true
                                                 ) )
               {
                  editor->append(QString("Error performing CS Guinier analysis on %1\n" + usu.errormsg + "\n")
                                 .arg(qsl_plotted_iq_names[ i ]));
                  return false;
               }
               // cout << QString( "removed size: %1 pts_removed %2\n" ).arg( removed.size() ).arg( pts_removed );
               if ( removed.size() )
               {
                  plotted_cs_guinier_pts_removed[ i ] = removed;

                  map < QString, bool > removed_match;
                  for ( map < double, double >::iterator it = removed.begin();
                        it != removed.end();
                        it++ )
                  {
                     removed_match[ QString( "%1" ).arg( it->first ) ] = true;
                  }

                  for ( int j = 0; j < (int)usu.wave[ "data" ].q.size(); ++j )
                  {
                     if ( removed_match.count( QString( "%1" ).arg( usu.wave[ "data" ].q[ j ] * usu.wave[ "data" ].q[ j ] ) ) )
                     {
                        qs_removed += QString( "%1%2" ).arg( qs_removed.isEmpty() ? "" : "_" ).arg( j + 1 );
                     }
                  }
               } else {
                  plotted_cs_guinier_pts_removed.erase( i );
               }
               // cout << QString( "after gor: smin^2 %1 smax^2 %2\n" ).arg( smin * smin ).arg( smax * smax );
            } else {
               plotted_cs_guinier_pts_removed.erase( i );
               if ( 
                   !usu.guinier_plot(
                                     "guinier",
                                     "data"
                                     )   ||
                   !usu.guinier_fit(
                                    this_log,
                                    "guinier", 
                                    pstart,
                                    pend,
                                    a,
                                    b,
                                    siga,
                                    sigb,
                                    chi2,
                                    Rg,
                                    I0,
                                    smax, // don't know why these are flipped
                                    smin,
                                    sRgmin,
                                    sRgmax,
                                    true
                                    ) )
               {
                  editor->append(QString("Error performing CS Guinier analysis on %1\n" + usu.errormsg + "\n")
                                 .arg(qsl_plotted_iq_names[ i ]));
                  return false;
               }
            }
            do_decrease = do_decrease && ( sRgmax > sRgmaxlimit );
            if ( do_decrease )
            {
               // puts( "decreasing one point" );
               pts_decrease++;
            }
         } while ( do_decrease );
      }
      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   QString report;
   if ( too_few_points ||
        ( bestend - beststart + 1 - pts_removed < pointsmin ) ||
        !internal_error.isEmpty() )
   {
      if ( !internal_error.isEmpty() )
      {
         report =
            QString(
                    "CS Guinier analysis of %1:\n"
                    "**** %2 ****\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
         csvlog += 
            QString(
                    "\"%1\","
                    "\"%2\"\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
      } else {        
         if ( pts_removed )
         {
            report =
               QString(
                       "CS Guinier analysis of %1:\n"
                       "**** Could not compute Rc, too few data points %2 after removal ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2) after removal\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
         } else {
            report =
               QString(
                       "CS Guinier analysis of %1:\n"
                       "**** Could not compute Rc, too few data points %2 ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;
            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2)\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;
         }
      }
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         plotted_cs_guinier_valid[ i ] = false;
         report =
            QString(
                    "CS Guinier analysis of %1:\n"
                    "**** Could not compute Rc ****\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rc\"\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

      } else {
         double slope = Rg * Rg / -2e0;
         double slopesd = Rg != 0e0 ? -2e0 * slope * sigb / Rg : 0e0;
         // cout << QString( "slope %1 slope sd %2\n" ).arg( slope ).arg( slopesd );

         double ML;
         double ML_sd;
         double ICL;

         // ml_from_qI0( qsl_plotted_iq_names[ i ], I0, ML_sd, ICL );
         if ( !ml_from_qI0( qsl_plotted_iq_names[ i ], I0, ML, ICL ) )
         {
            editor_msg( "red", errormsg );
         }
         ML_sd = I0 ? ML * siga / I0 : 0e0;

         report = 
            QString("")
            .sprintf(
                     "CS Guinier analysis of %s:\n"
                     "Rc %.1f (%.1f) (A) I(0) %.2e (%.2e) M/L %.2e (%.2e) q^2 [%.5f:%.5f] qRcmin %.3f qRcmax %.3f points used %u chi^2 %.2e reduced-chi %.2e\n"
                     , qsl_plotted_iq_names[ i ].ascii()
                     , Rg
                     , sigb 
                     , I0
                     , siga
                     , ML
                     , ML_sd
                     , smin * smin
                     , smax * smax
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1 - pts_removed
                     , chi2
                     , sqrt( chi2 / (bestend - beststart - 1 - pts_removed ) )
                     );
         
         plotted_cs_guinier_valid[ i ] = true;
         plotted_cs_guinier_lowq2[ i ] = smin * smin;
         plotted_cs_guinier_highq2[ i ] = smax * smax;
         plotted_cs_guinier_a[ i ] = a;
         plotted_cs_guinier_b[ i ] = b;
         plotted_cs_guinier_plotted[ i ] = false;
         
         plotted_cs_guinier_x[ i ].clear();
         plotted_cs_guinier_x[ i ].push_back(plotted_cs_guinier_lowq2[ i ]);
         plotted_cs_guinier_x[ i ].push_back(plotted_cs_guinier_highq2[ i ]);
         
         plotted_cs_guinier_y[ i ].clear();
         plotted_cs_guinier_y[ i ].push_back(exp(plotted_cs_guinier_a[ i ] + plotted_cs_guinier_b[ i ] * plotted_cs_guinier_lowq2[ i ]));
         plotted_cs_guinier_y[ i ].push_back(exp(plotted_cs_guinier_a[ i ] + plotted_cs_guinier_b[ i ] * plotted_cs_guinier_highq2[ i ]));

         /*
         cout << QString( "cs guinier line range q %1 %2 q^2 %3 %4 a %5 b %6\n" )
            .arg( smin  )
            .arg( smax )
            .arg( plotted_cs_guinier_lowq2[ i ] )
            .arg( plotted_cs_guinier_highq2[ i ] )
            .arg( a )
            .arg( b )
            ;
         */ 

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\"," )
            .arg( qsl_plotted_iq_names[ i ] )
            ;

         csvlog += 
            QString(
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    "%9,"
                    )
            .arg( Rg * Rg )
            .arg( 2e0 * slopesd )
            .arg( Rg )
            .arg( sigb )
            .arg( I0 )
            .arg( siga )
            .arg( ML )
            .arg( ML_sd )
            .arg( ICL )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    )
            .arg( smin )
            .arg( smax )
            .arg( sRgmin )
            .arg( sRgmax )
            .arg( beststart + 1 )
            .arg( bestend + 1 )
            .arg( bestend - beststart + 1 - pts_removed )
            .arg( qs_removed )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    )
            .arg( chi2 )
            .arg( sqrt( chi2 / ( bestend - beststart - 1 - pts_removed ) ) )
            ;

         {
            double conc;
            double psv;
            double I0_std_exp;

            
            get_conc_csv_values( qsl_plotted_iq_names[ i ], conc, psv, I0_std_exp );
            csvlog += 
               QString( 
                       "%1,"
                       "%2,"
                       "%3,"
                       "%4,"
                       "%5,"
                       "\"%6\","
                       )
               .arg( conc )
               .arg( psv )
               .arg( I0_std_exp )
               .arg( slope  )
               .arg( slopesd )
               .arg( tr( use_SD_weighting ? "used" : "not used" ) )
               ;
         }

         csvlog += "\n";
      }
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
   {
      guinier_names.push_back( qsl_plotted_iq_names[ i ] );
      vector < double > q2;
      vector < double > lnI;
      vector < double > sd;
      map < double, double > empty_removed;
      double ml;
      double ICL;
      for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
      {
         q2 .push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] );
         lnI.push_back( plotted_I[ i ][ j ] > 0e0 ? log(  plotted_q[ i ][ j ] * plotted_I[ i ][ j ] ) : 0e0 );
         if ( !ml_from_qI0( qsl_plotted_iq_names[ i ], exp( lnI.back() ), ml, ICL ) )
         {
            cout << QString( "ml from i0 issue %1 %2 %3\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( lnI.back() ).arg( ml );
         }
         lnI.back() = log( ml );
         if ( use_SD_weighting )
         {
            sd.push_back( plotted_I[ i ][ j ] > 0e0 ? 
                          ( lnI.back() / log( plotted_q[ i ][ j ] * plotted_I[ i ][ j ] ) ) * fabs( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
            // fabs( lnI.back() ) * ( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
         }
      }
      
      guinier_q2 .push_back( q2  );
      guinier_lnI.push_back( lnI );
      guinier_sd .push_back( sd  );
      if ( use_guinier_outlier_reject )
      {
         guinier_removed.push_back( plotted_cs_guinier_pts_removed.count( i ) ?
                                    plotted_cs_guinier_pts_removed[ i ] : empty_removed 
                                    );
      } else {
         guinier_removed.push_back( empty_removed );
      }         
   }
   editor_msg( plot_colors[i % plot_colors.size()], plot_saxs->canvasBackground(), report );

   //   cout << csvlog;
   return true;
}

bool US_Hydrodyn_Saxs::Rt_guinier_analysis( unsigned int i, QString &csvlog )
{
   if (
#ifndef QT4 
       i > plotted_Iq.size() 
#else
       i > plotted_Iq_curves.size() 
#endif
       )
   {
      editor->append("internal error: invalid plot selected\n");
      return false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q.clear();
   usu.wave["data"].r.clear();
   usu.wave["data"].s.clear();

   bool use_SD_weighting = our_saxs_options->guinier_use_sd;
   bool use_guinier_outlier_reject = our_saxs_options->guinier_outlier_reject;

   if ( use_SD_weighting )
   {
      if ( plotted_I_error[ i ].size() != plotted_q[ i ].size() )
      {
         editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are not fully present" ) )
                     .arg( qsl_plotted_iq_names[ i ] ) );
         use_SD_weighting = false;
      } else {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                 plotted_q[ i ][ j ] <= our_saxs_options->qend )
            {
               if ( plotted_I_error[ i ][ j ] <= 0e0 )
               {
                  editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since at least one SD is zero or negative in the selected q range" ) )
                              .arg( qsl_plotted_iq_names[ i ] ) );
                  use_SD_weighting = false;
                  break;
               }
            }
         }
      }
   }

   if ( use_guinier_outlier_reject && !use_SD_weighting )
   {
      editor_msg( "dark red", QString( tr( "Notice: Turning off outlier rejection for Guinier fit of %1 since SD weighting is off" ) )
                  .arg( qsl_plotted_iq_names[ i ] ) );

      use_guinier_outlier_reject = false;
   }

   // vector < double > q2;

   for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
   {
      if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
           plotted_q[ i ][ j ] <= our_saxs_options->qend )
      {
         usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
         // q2.push_back(  plotted_q2[ i ][ j ] );
         usu.wave["data"].r.push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
         if ( use_SD_weighting )
         {
            usu.wave["data"].s.push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I_error[ i ][ j ] );
         }
      }
   }

   // US_Vector::printvector3( "cs1 q2 I e", q2 /* usu.wave["data"].q */, usu.wave["data"].r, usu.wave["data"].s );

   QString this_log;

   unsigned int pointsmin = our_saxs_options->pointsmin;
   if ( pointsmin < 2 )
   {
      pointsmin = 2;
   }
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_qRtmax" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_qRtmax" ].toDouble() : 1e0;
      
   double pointweightpower = 3e0;
   // double p_guinier_minq = our_saxs_options->qstart;
   double p_guinier_maxq = our_saxs_options->qend;

   // cout << QString( "qstart %1 qend %2\n" ).arg( p_guinier_minq ).arg( p_guinier_maxq );
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double I0;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[ i ].size() <= 5;

   unsigned int pts_removed = 0;
   QString      qs_removed  = "";
   QString      internal_error = "";

   if ( !too_few_points )
   {
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" &&
           (unsigned int)usu.wave[ "data" ].q.size() > pointsmin )
      {
         if ( 
             !usu.guinier_plot(
                               "tv_guinier",
                               "data"
                               )   ||
             !usu.guinier_fit2(
                               this_log,
                               "tv_guinier", 
                               pointsmin,
                               pointsmax,
                               sRgmaxlimit,
                               pointweightpower,
                               p_guinier_maxq,
                               a,
                               b,
                               siga,
                               sigb,
                               chi2,
                               Rg,
                               I0,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend,
                               false,
                               true
                               ) )
         {
            editor_msg( "red",
                        QString("Error performing TV Guinier analysis on %1\n" + usu.errormsg + "\n")
                        .arg(qsl_plotted_iq_names[ i ]));
            return false;
         }
      } else {
         bool do_decrease = 
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_use_qRlimit" ) &&
            ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_use_qRlimit" ] == "1";
         unsigned int pts_decrease = 0;
         //          if ( do_decrease )
         //          {
         //             printf( "do decrease, pts_decrease %u\n", pts_decrease );
         //          }
         do {
            usu.wave["data"].q.clear();
            usu.wave["data"].r.clear();
            usu.wave["data"].s.clear();
            qs_removed = "";

            // vector < double > q2;

            for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
            {
               if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart &&
                    plotted_q[ i ][ j ] <= our_saxs_options->qend )
               {
                  usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
                  // q2.push_back(  plotted_q2[ i ][ j ] );
                  usu.wave["data"].r.push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
                  if ( use_SD_weighting )
                  {
                     usu.wave["data"].s.push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I_error[ i ][ j ] );
                  }
               }
            }

            if ( pts_decrease )
            {
               if ( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease < 5 )
               {
                  internal_error = QString( tr( "Too few points left (%1) after %2 points removed for qRgmax limit" ) )
                     .arg( (int) usu.wave[ "data" ].q.size() - (int) pts_decrease ).arg( pts_decrease ); 
                  // editor->append( QString( "Error performing TV Guinier analysis on %1: %2\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( internal_error ) );
                  break;
               }

               usu.wave[ "data" ].q.resize( usu.wave[ "data" ].q.size() - pts_decrease );
               usu.wave[ "data" ].r.resize( usu.wave[ "data" ].q.size() );
               if ( use_SD_weighting )
               {
                  usu.wave[ "data" ].s.resize( usu.wave[ "data" ].q.size() );
               }
            }

            unsigned int pstart = 0;
            unsigned int pend   = usu.wave[ "data" ].q.size() ? usu.wave[ "data" ].q.size() - 1 : 0;
            bestend = pend;
            beststart = pstart;

            if ( use_guinier_outlier_reject )
            {
               // puts( "outlier reject" );
               map < double, double > removed;
               if ( 
                   !usu.guinier_plot(
                                     "guinier",
                                     "data"
                                     )   ||
                   !usu.guinier_fit_with_removal(
                                                 this_log,
                                                 "guinier", 
                                                 pstart,
                                                 pend,
                                                 a,
                                                 b,
                                                 siga,
                                                 sigb,
                                                 chi2,
                                                 Rg,
                                                 I0,
                                                 smax, // don't know why these are flipped
                                                 smin,
                                                 sRgmin,
                                                 sRgmax,
                                                 our_saxs_options->guinier_outlier_reject_dist,
                                                 removed,
                                                 pts_removed,
                                                 false,
                                                 true
                                                 ) )
               {
                  editor->append(QString("Error performing TV Guinier analysis on %1\n" + usu.errormsg + "\n")
                                 .arg(qsl_plotted_iq_names[ i ]));
                  return false;
               }
               // cout << QString( "removed size: %1 pts_removed %2\n" ).arg( removed.size() ).arg( pts_removed );
               if ( removed.size() )
               {
                  plotted_Rt_guinier_pts_removed[ i ] = removed;

                  map < QString, bool > removed_match;
                  for ( map < double, double >::iterator it = removed.begin();
                        it != removed.end();
                        it++ )
                  {
                     removed_match[ QString( "%1" ).arg( it->first ) ] = true;
                  }

               for ( int j = 0; j < (int)usu.wave[ "data" ].q.size(); ++j )
               {
                  if ( removed_match.count( QString( "%1" ).arg( usu.wave[ "data" ].q[ j ] * usu.wave[ "data" ].q[ j ] ) ) )
                  {
                     qs_removed += QString( "%1%2" ).arg( qs_removed.isEmpty() ? "" : "_" ).arg( j + 1 );
                  }
               }
            } else {
               plotted_Rt_guinier_pts_removed.erase( i );
            }
            // cout << QString( "after gor: smin^2 %1 smax^2 %2\n" ).arg( smin * smin ).arg( smax * smax );
         } else {
            plotted_Rt_guinier_pts_removed.erase( i );
            if ( 
                !usu.guinier_plot(
                                  "guinier",
                                  "data"
                                  )   ||
                !usu.guinier_fit(
                                 this_log,
                                 "guinier", 
                                 pstart,
                                 pend,
                                 a,
                                 b,
                                 siga,
                                 sigb,
                                 chi2,
                                 Rg,
                                 I0,
                                 smax, // don't know why these are flipped
                                 smin,
                                 sRgmin,
                                 sRgmax,
                                 false,
                                 true
                                 ) )
            {
               editor->append(QString("Error performing TV Guinier analysis on %1\n" + usu.errormsg + "\n")
                              .arg(qsl_plotted_iq_names[ i ]));
               return false;
            }
         }
            do_decrease = do_decrease && ( sRgmax > sRgmaxlimit );
            if ( do_decrease )
            {
               // puts( "decreasing one point" );
               pts_decrease++;
            }
         } while ( do_decrease );
      }
      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   QString report;
   if ( too_few_points ||
        ( bestend - beststart + 1 - pts_removed < pointsmin )  ||
        !internal_error.isEmpty() )
   {
      if ( !internal_error.isEmpty() )
      {
         report = 
            QString(
                    "TV Guinier analysis of %1:\n"
                    "**** %2 ****\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
         csvlog += 
            QString(
                    "\"%1\","
                    "\"%2\"\n"
                    )
            .arg( qsl_plotted_iq_names[ i ] )
            .arg( internal_error )
            ;
      } else {        
         if ( pts_removed )
         {
            report =
               QString(
                       "TV Guinier analysis of %1:\n"
                       "**** Could not compute Rt, too few data points %2 after removal ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2) after removal\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 - pts_removed )
               ;
         } else {
            report =
               QString(
                       "TV Guinier analysis of %1:\n"
                       "**** Could not compute Rt, too few data points %2 ****\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;
            csvlog += 
               QString(
                       "\"%1\","
                       "\"Too few data points (%2)\"\n"
                       )
               .arg( qsl_plotted_iq_names[ i ] )
               .arg( bestend - beststart + 1 )
               ;
         }
      }
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         plotted_Rt_guinier_valid[ i ] = false;
         report =
            QString(
                    "TV Guinier analysis of %1:\n"
                    "**** Could not compute Rt ****\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rc\"\n"
                    )
            .arg(qsl_plotted_iq_names[ i ]);

      } else {
         double slope = Rg * Rg / -1e0;
         double slopesd = Rg != 0e0 ? -2e0 * slope * sigb / Rg : 0e0;
         // cout << QString( "slope %1 slope sd %2\n" ).arg( slope ).arg( slopesd );

         double MA;
         double MA_sd;
         double ICL;

         if ( !ma_from_q2I0( qsl_plotted_iq_names[ i ], I0, MA, ICL ) )
         {
            editor_msg( "red", errormsg );
         }
         MA_sd = I0 ? MA * siga / I0 : 0e0;

         report = 
            QString("")
            .sprintf(
                     "TV Guinier analysis of %s:\n"
                     "Rt %.1f (%.1f) (A) I(0) %.2e (%.2e) M/A %.2e (%.2e) q^2 [%.5f:%.5f] qRcmin %.3f qRcmax %.3f points used %u chi^2 %.2e reduced-chi %.2e\n"
                     , qsl_plotted_iq_names[ i ].ascii()
                     , Rg
                     , sigb 
                     , I0
                     , siga
                     , MA
                     , MA_sd
                     , smin * smin
                     , smax * smax
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1 - pts_removed
                     , chi2
                     , sqrt( chi2 / (bestend - beststart - 1 - pts_removed ) )
                     );
         
         plotted_Rt_guinier_valid[ i ] = true;
         plotted_Rt_guinier_lowq2[ i ] = smin * smin;
         plotted_Rt_guinier_highq2[ i ] = smax * smax;
         plotted_Rt_guinier_a[ i ] = a;
         plotted_Rt_guinier_b[ i ] = b;
         plotted_Rt_guinier_plotted[ i ] = false;
         
         plotted_Rt_guinier_x[ i ].clear();
         plotted_Rt_guinier_x[ i ].push_back(plotted_Rt_guinier_lowq2[ i ]);
         plotted_Rt_guinier_x[ i ].push_back(plotted_Rt_guinier_highq2[ i ]);
         
         plotted_Rt_guinier_y[ i ].clear();
         plotted_Rt_guinier_y[ i ].push_back(exp(plotted_Rt_guinier_a[ i ] + plotted_Rt_guinier_b[ i ] * plotted_Rt_guinier_lowq2[ i ]));
         plotted_Rt_guinier_y[ i ].push_back(exp(plotted_Rt_guinier_a[ i ] + plotted_Rt_guinier_b[ i ] * plotted_Rt_guinier_highq2[ i ]));

         /*
         cout << QString( "cs guinier line range q %1 %2 q^2 %3 %4 a %5 b %6\n" )
            .arg( smin  )
            .arg( smax )
            .arg( plotted_Rt_guinier_lowq2[ i ] )
            .arg( plotted_Rt_guinier_highq2[ i ] )
            .arg( a )
            .arg( b )
            ;
         */ 

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\"," )
            .arg( qsl_plotted_iq_names[ i ] )
            ;

         csvlog += 
            QString(
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    "%9,"
                    )
            .arg( Rg * Rg )
            .arg( slopesd )
            .arg( Rg )
            .arg( sigb )
            .arg( I0 )
            .arg( siga )
            .arg( MA )
            .arg( MA_sd )
            .arg( ICL )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    )
            .arg( smin )
            .arg( smax )
            .arg( sRgmin )
            .arg( sRgmax )
            .arg( beststart + 1 )
            .arg( bestend + 1 )
            .arg( bestend - beststart + 1 - pts_removed )
            .arg( qs_removed )
            ;

         csvlog += 
            QString( 
                    "%1,"
                    "%2,"
                    )
            .arg( chi2 )
            .arg( sqrt( chi2 / ( bestend - beststart - 1 - pts_removed ) ) )
            ;

         {
            double conc;
            double psv;
            double I0_std_exp;
            
            get_conc_csv_values( qsl_plotted_iq_names[ i ], conc, psv, I0_std_exp );
            csvlog += 
               QString( 
                       "%1,"
                       "%2,"
                       "%3,"
                       "%4,"
                       "%5,"
                       "\"%6\","
                       )
               .arg( conc )
               .arg( psv )
               .arg( I0_std_exp )
               .arg( slope  )
               .arg( slopesd )
               .arg( tr( use_SD_weighting ? "used" : "not used" ) )
               ;
         }

         csvlog += "\n";
      }
   }
   if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_csv_save_data" ) &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_csv_save_data" ] == "1" )
   {
      guinier_names.push_back( qsl_plotted_iq_names[ i ] );
      vector < double > q2;
      vector < double > lnI;
      vector < double > sd;
      map < double, double > empty_removed;
      double ma;
      double ICL;
      for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
      {
         q2 .push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] );
         lnI.push_back( plotted_I[ i ][ j ] > 0e0 ? log( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I[ i ][ j ] ) : 0e0 );
         if ( !ma_from_q2I0( qsl_plotted_iq_names[ i ], exp( lnI.back() ), ma, ICL ) )
         {
            cout << QString( "ma from i0 issue %1 %2 %3\n" ).arg( qsl_plotted_iq_names[ i ] ).arg( lnI.back() ).arg( ma );
         }
         lnI.back() = log( ma );
         if ( use_SD_weighting )
         {
            sd.push_back( plotted_I[ i ][ j ] > 0e0 ? 
                          ( lnI.back() / log( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I[ i ][ j ] ) ) * fabs( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
            //fabs( lnI.back() ) * ( plotted_I_error[ i ][ j ] / plotted_I[ i ][ j ] ) : 0e0 );
         }
      }
      
      guinier_q2 .push_back( q2  );
      guinier_lnI.push_back( lnI );
      guinier_sd .push_back( sd  );
      if ( use_guinier_outlier_reject )
      {
         guinier_removed.push_back( plotted_Rt_guinier_pts_removed.count( i ) ?
                                    plotted_Rt_guinier_pts_removed[ i ] : empty_removed 
                                    );
      } else {
         guinier_removed.push_back( empty_removed );
      }         
   }
   editor_msg( plot_colors[i % plot_colors.size()], plot_saxs->canvasBackground(), report );

   //   cout << csvlog;
   return true;
}


void US_Hydrodyn_Saxs::set_guinier()
{
   if ( rb_sans->isChecked() ) 
   {
      plot_saxs->setTitle((cb_guinier->isChecked() ? 
                           ( cb_cs_guinier->isChecked() ?
                             "CS Guinier " : 
                             ( cb_Rt_guinier->isChecked() ?
                               "TV Guinier " : "Guinier " ) )
                           : "") + tr("SANS Curve"));
   } else {
      plot_saxs->setTitle((cb_guinier->isChecked() ? 
                           ( cb_cs_guinier->isChecked() ?
                             "CS Guinier " : 
                             ( cb_Rt_guinier->isChecked() ?
                               "TV Guinier " : "Guinier " ) )
                           : "") + tr("SAXS Curve"));
   }

   do_plot_resid();

   if ( cb_guinier->isChecked() )
   {
      cb_user_range->setChecked(false);
      cb_kratky    ->setChecked(false);
   }

   if ( cb_kratky->isChecked() )
   {
      if ( rb_sans->isChecked() ) 
      {
         plot_saxs->setTitle( "Kratky " + tr("SANS Curve"));
      } else {
         plot_saxs->setTitle( "Kratky " + tr("SAXS Curve"));
      }
   }      

   rescale_plot();

   // clear the plot and replot the curves in correct mode ( kratky or guniner or cs-guinier )
   plot_saxs->clear();
   for ( unsigned int p = 0;
#ifndef QT4
         p < plotted_Iq.size();
#else
         p < plotted_Iq_curves.size();
#endif
         p++ )
   {
      unsigned int q_points = plotted_q[ p ].size();
      
      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

      vector < double > pI;
      if ( cb_guinier->isChecked() && cb_cs_guinier->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            pI.push_back( plotted_q[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

      if ( cb_guinier->isChecked() && cb_Rt_guinier->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            pI.push_back( plotted_q[ p ][ i ] * plotted_q[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

#ifndef QT4
      long Iq = plot_saxs->insertCurve( qsl_plotted_iq_names[ p ] );
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
      plotted_Iq[ p ] = Iq;
#else
      QwtPlotCurve *curve = new QwtPlotCurve( qsl_plotted_iq_names[ p ] );
      curve->setStyle( QwtPlotCurve::Lines );
      plotted_Iq_curves[ p ] = curve;
#endif
      
#ifndef QT4
      plot_saxs->setCurveData(plotted_Iq[ p ],
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])            : // (double *)&(plotted_I[p][0]),
                              ( cb_guinier->isChecked() &&
                                ( cb_cs_guinier->isChecked() ||
                                  cb_Rt_guinier->isChecked() ) ? (double *)&(pI[ 0 ]) : (double *)&(plotted_I[p][0]) ),
                              q_points);
      plot_saxs->setCurvePen( plotted_Iq[ p ], QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
      plotted_Iq_curves[ p ]->setData(
                                      cb_guinier->isChecked() ?
                                      (double *)&(plotted_q2[p][0])  : (double *)&(plotted_q[p][0]), 
                                      cb_kratky ->isChecked() ?
                                      (double *)&(q2I[0])            : // (double *)&(plotted_I[p][0]),
                                      ( cb_guinier->isChecked() &&
                                        ( cb_cs_guinier->isChecked() ||
                                          cb_Rt_guinier->isChecked() ) ? (double *)&(pI[ 0 ]) : (double *)&(plotted_I[p][0]) ),
                                      q_points
                                      );
      plotted_Iq_curves[ p ]->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      plotted_Iq_curves[ p ]->attach( plot_saxs );
#endif
   }

#ifndef QT4
   int niqsize = (int)plotted_Iq.size();
#else
   int niqsize = (int)plotted_Iq_curves.size();
#endif
   clear_guinier_error_bars();
   unsigned int total_points = 0;
   bool do_errorbars = 
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_cb_resid_show_errorbars" ) &&
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_cb_resid_show_errorbars" ] == "1" ;

   if ( cb_guinier->isChecked() )
   {
      if ( do_errorbars )
      {
         for ( int i = 0; i < niqsize; i++ )
         {
            total_points += (unsigned int )plotted_q[ i ].size();
         }
         if ( total_points > 20000 )
         {
            editor_msg( "dark red", "Notice: error bars are turned off due to too many curve-points" );
            do_errorbars = false;
         }
      }

      if ( do_errorbars )
      {
         pb_saxs_legend->setEnabled( false );
#ifndef QT4
         plot_saxs->setAutoLegend( false );
         plot_saxs->enableLegend ( false, -1 );
#else 
         plot_saxs->legend()->setVisible( false );
#endif
      } else {
         pb_saxs_legend->setEnabled( true );
      }
   } else {
      pb_saxs_legend->setEnabled( true );
#ifndef QT4
      plot_saxs->removeMarkers();
#else
      plot_saxs->detachItems( QwtPlotItem::Rtti_PlotMarker );
#endif
   }

   for ( int i = 0; i < niqsize; i++ )
   {
      if ( cb_guinier->isChecked() )
      {
         if ( cb_cs_guinier->isChecked() )
         {
            if ( do_errorbars )
            {
               plot_guinier_error_bars( i, true, false );
            }
            // replot the cs guinier bits
            if ( plotted_cs_guinier_valid.count(i) &&
                 plotted_cs_guinier_valid[ i ] == true )
            {
#ifndef QT4
               plotted_cs_Gp[ i ] = plot_saxs->insertCurve(QString("CS Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
               plot_saxs->setCurveStyle(plotted_cs_Gp[ i ], QwtCurve::Lines);
               plot_saxs->setCurveData(plotted_cs_Gp[ i ], (double *)&(plotted_cs_guinier_x[ i ][0]), (double *)&(plotted_cs_guinier_y[ i ][0]), 2);

               plot_saxs->setCurvePen(plotted_cs_Gp[ i ], QPen("dark red", pen_width, SolidLine));
#else
               plotted_cs_Gp_curves[ i ] = new QwtPlotCurve(
                                                          QString( "CS Gp. %1" ).arg( qsl_plotted_iq_names[ i ] ) );
               plotted_cs_Gp_curves[ i ]->setStyle( QwtPlotCurve::Lines );
               plotted_cs_Gp_curves[ i ]->setData(
                                                (double *)&(plotted_cs_guinier_x[ i ][0]), 
                                                (double *)&(plotted_cs_guinier_y[ i ][0]), 
                                                2
                                                );
               plotted_cs_Gp_curves[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::SolidLine ) );
               plotted_cs_Gp_curves[ i ]->attach( plot_saxs );
#endif            
               plotted_guinier_plotted[ i ] = true;

               if ( plotted_q2[ i ].size() )
               {
                  vector < double > x( 2 );
                  vector < double > y( 2 );
                  x[ 0 ] = plotted_q2[ i ][ 0 ];
                  x[ 1 ] = plotted_q2[ i ].back();
                  y[ 0 ] = exp( plotted_cs_guinier_a[ i ] + plotted_cs_guinier_b[ i ] * x[ 0 ] );
                  y[ 1 ] = exp( plotted_cs_guinier_a[ i ] + plotted_cs_guinier_b[ i ] * x[ 1 ] );
#ifndef QT4
                  plotted_cs_Gp_full[ i ] = plot_saxs->insertCurve(QString(QString("CS Gn. %1").arg( qsl_plotted_iq_names[ i ] ) ) );
                  plot_saxs->setCurveStyle(plotted_cs_Gp_full[ i ], QwtCurve::Lines);
                  plot_saxs->setCurveData (plotted_cs_Gp_full[ i ], (double *)&(x[0]), (double *)&(y[0]), 2);
                  plot_saxs->setCurvePen  (plotted_cs_Gp_full[ i ], QPen("dark red", pen_width, DotLine));
#else
                  plotted_cs_Gp_curves_full[ i ] = new QwtPlotCurve(
                                                                  QString(QString("CS Gn. %1").arg( qsl_plotted_iq_names[ i ] ) ) );
                  plotted_cs_Gp_curves_full[ i ]->setStyle( QwtPlotCurve::Lines );
                  plotted_cs_Gp_curves_full[ i ]->setData(
                                                (double *)&(x[0]), 
                                                (double *)&(y[0]), 
                                                2
                                                );
                  plotted_cs_Gp_curves_full[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::DotLine ) );
                  plotted_cs_Gp_curves_full[ i ]->attach( plot_saxs );
#endif            
               }
               plotted_guinier_plotted[ i ] = true;
            }

            // turn regular iqq curves into points

            QwtSymbol sym;
            sym.setStyle(QwtSymbol::Diamond);
            sym.setSize(6);
            sym.setPen(QPen(plot_colors[i % plot_colors.size()]));
            sym.setBrush(Qt::white);
#ifndef QT4
            plot_saxs->setCurveStyle(plotted_Iq[ i ], QwtCurve::Lines);
            plot_saxs->setCurveSymbol(plotted_Iq[ i ], QwtSymbol(sym));
#else
            plotted_Iq_curves[ i ]->setStyle( QwtPlotCurve::Lines );
            plotted_Iq_curves[ i ]->setSymbol( QwtSymbol( sym ) );
#endif
            plot_guinier_pts_removed( i, true, false );
         } else {
            if ( cb_Rt_guinier->isChecked() )
            {
               if ( do_errorbars )
               {
                  plot_guinier_error_bars( i, false, true );
               }
               // replot the cs guinier bits
               if ( plotted_Rt_guinier_valid.count(i) &&
                    plotted_Rt_guinier_valid[ i ] == true )
               {
#ifndef QT4
                  plotted_Rt_Gp[ i ] = plot_saxs->insertCurve(QString("TV Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
                  plot_saxs->setCurveStyle(plotted_Rt_Gp[ i ], QwtCurve::Lines);
                  plot_saxs->setCurveData(plotted_Rt_Gp[ i ], (double *)&(plotted_Rt_guinier_x[ i ][0]), (double *)&(plotted_Rt_guinier_y[ i ][0]), 2);

                  plot_saxs->setCurvePen(plotted_Rt_Gp[ i ], QPen("dark red", pen_width, SolidLine));
#else
                  plotted_Rt_Gp_curves[ i ] = new QwtPlotCurve(
                                                             QString( "TV Gp. %1" ).arg( qsl_plotted_iq_names[ i ] ) );
                  plotted_Rt_Gp_curves[ i ]->setStyle( QwtPlotCurve::Lines );
                  plotted_Rt_Gp_curves[ i ]->setData(
                                                   (double *)&(plotted_Rt_guinier_x[ i ][0]), 
                                                   (double *)&(plotted_Rt_guinier_y[ i ][0]), 
                                                   2
                                                   );
                  plotted_Rt_Gp_curves[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::SolidLine ) );
                  plotted_Rt_Gp_curves[ i ]->attach( plot_saxs );
#endif            
                  plotted_guinier_plotted[ i ] = true;

                  if ( plotted_q2[ i ].size() )
                  {
                     vector < double > x( 2 );
                     vector < double > y( 2 );
                     x[ 0 ] = plotted_q2[ i ][ 0 ];
                     x[ 1 ] = plotted_q2[ i ].back();
                     y[ 0 ] = exp( plotted_Rt_guinier_a[ i ] + plotted_Rt_guinier_b[ i ] * x[ 0 ] );
                     y[ 1 ] = exp( plotted_Rt_guinier_a[ i ] + plotted_Rt_guinier_b[ i ] * x[ 1 ] );
#ifndef QT4
                     plotted_Rt_Gp_full[ i ] = plot_saxs->insertCurve(QString(QString("TV Gn. %1").arg( qsl_plotted_iq_names[ i ] ) ) );
                     plot_saxs->setCurveStyle(plotted_Rt_Gp_full[ i ], QwtCurve::Lines);
                     plot_saxs->setCurveData (plotted_Rt_Gp_full[ i ], (double *)&(x[0]), (double *)&(y[0]), 2);
                     plot_saxs->setCurvePen  (plotted_Rt_Gp_full[ i ], QPen("dark red", pen_width, DotLine));
#else
                     plotted_Rt_Gp_curves_full[ i ] = new QwtPlotCurve(
                                                                     QString(QString("CS Gn. %1").arg( qsl_plotted_iq_names[ i ] ) ) );
                     plotted_Rt_Gp_curves_full[ i ]->setStyle( QwtPlotCurve::Lines );
                     plotted_Rt_Gp_curves_full[ i ]->setData(
                                                           (double *)&(x[0]), 
                                                           (double *)&(y[0]), 
                                                           2
                                                           );
                     plotted_Rt_Gp_curves_full[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::DotLine ) );
                     plotted_Rt_Gp_curves_full[ i ]->attach( plot_saxs );
#endif            
                  }
                  plotted_guinier_plotted[ i ] = true;
               }

               // turn regular iqq curves into points

               QwtSymbol sym;
               sym.setStyle(QwtSymbol::Diamond);
               sym.setSize(6);
               sym.setPen(QPen(plot_colors[i % plot_colors.size()]));
               sym.setBrush(Qt::white);
#ifndef QT4
               plot_saxs->setCurveStyle(plotted_Iq[ i ], QwtCurve::Lines);
               plot_saxs->setCurveSymbol(plotted_Iq[ i ], QwtSymbol(sym));
#else
               plotted_Iq_curves[ i ]->setStyle( QwtPlotCurve::Lines );
               plotted_Iq_curves[ i ]->setSymbol( QwtSymbol( sym ) );
#endif
               plot_guinier_pts_removed( i, false, true );
            } else {
               // replot the guinier bits
               if ( do_errorbars )
               {
                  plot_guinier_error_bars( i, false, false );
               }
               if ( plotted_guinier_valid.count(i) &&
                    plotted_guinier_valid[ i ] == true )
               {
#ifndef QT4
                  plotted_Gp[ i ] = plot_saxs->insertCurve(QString("Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
                  plot_saxs->setCurveStyle(plotted_Gp[ i ], QwtCurve::Lines);
                  plot_saxs->setCurveData(plotted_Gp[ i ], (double *)&(plotted_guinier_x[ i ][0]), (double *)&(plotted_guinier_y[ i ][0]), 2);

                  plot_saxs->setCurvePen(plotted_Gp[ i ], QPen("dark red", pen_width, SolidLine));
#else
                  plotted_Gp_curves[ i ] = new QwtPlotCurve(
                                                          QString("Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
                  plotted_Gp_curves[ i ]->setStyle( QwtPlotCurve::Lines );
                  plotted_Gp_curves[ i ]->setData(
                                                (double *)&(plotted_guinier_x[ i ][0]), 
                                                (double *)&(plotted_guinier_y[ i ][0]), 
                                                2
                                                );
                  plotted_Gp_curves[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::SolidLine ) );
                  plotted_Gp_curves[ i ]->attach( plot_saxs );
#endif            

                  if ( plotted_q2[ i ].size() )
                  {
                     vector < double > x( 2 );
                     vector < double > y( 2 );
                     x[ 0 ] = plotted_q2[ i ][ 0 ];
                     x[ 1 ] = plotted_q2[ i ].back();
                     y[ 0 ] = exp( plotted_guinier_a[ i ] + plotted_guinier_b[ i ] * x[ 0 ] );
                     y[ 1 ] = exp( plotted_guinier_a[ i ] + plotted_guinier_b[ i ] * x[ 1 ] );
#ifndef QT4
                     plotted_Gp_full[ i ] = plot_saxs->insertCurve(QString("Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
                     plot_saxs->setCurveStyle(plotted_Gp_full[ i ], QwtCurve::Lines);
                     plot_saxs->setCurveData (plotted_Gp_full[ i ], (double *)&(x[0]), (double *)&(y[0]), 2);
                     plot_saxs->setCurvePen  (plotted_Gp_full[ i ], QPen("dark red", pen_width, DotLine));
#else
                     plotted_Gp_curves_full[ i ] = new QwtPlotCurve(
                                                                  QString("Gn. %1").arg( qsl_plotted_iq_names[ i ] ) );
                     plotted_Gp_curves_full[ i ]->setStyle( QwtPlotCurve::Lines );
                     plotted_Gp_curves_full[ i ]->setData(
                                                        (double *)&(x[0]), 
                                                        (double *)&(y[0]), 
                                                        2
                                                        );
                     plotted_Gp_curves_full[ i ]->setPen( QPen( QColor( "dark red" ), pen_width, Qt::DotLine) );
                     plotted_Gp_curves_full[ i ]->attach( plot_saxs );
#endif            
                  }
                  plotted_guinier_plotted[ i ] = true;
               }


               // turn regular iqq curves into points

               QwtSymbol sym;
               sym.setStyle(QwtSymbol::Diamond);
               sym.setSize(6);
               sym.setPen(QPen(plot_colors[i % plot_colors.size()]));
               sym.setBrush(Qt::white);
#ifndef QT4
               // plot_saxs->setCurveStyle(plotted_Iq[ i ], QwtCurve::Lines);
               plot_saxs->setCurveSymbol(plotted_Iq[ i ], QwtSymbol(sym));
#else
               // plotted_Iq_curves[ i ]->setStyle( QwtPlotCurve::Lines );
               plotted_Iq_curves[ i ]->setSymbol( QwtSymbol( sym ) );
#endif
               plot_guinier_pts_removed( i, false, false );
            }
         }
      } else {
         // remove any Guinier lines
         if ( plotted_guinier_plotted.count(i) &&
              plotted_guinier_plotted[ i ] == true )
         {
            plotted_guinier_plotted[ i ] = false;
#ifndef QT4
            plot_saxs->removeCurve(plotted_Gp[ i ]);
            plot_saxs->removeCurve(plotted_Gp_full[ i ]);
#else
            plotted_Gp_curves[ i ]->detach();
            plotted_Gp_curves_full[ i ]->detach();
#endif
         }
         
         if ( plotted_cs_guinier_plotted.count(i) &&
              plotted_cs_guinier_plotted[ i ] == true )
         {
            plotted_cs_guinier_plotted[ i ] = false;
#ifndef QT4
            plot_saxs->removeCurve(plotted_cs_Gp[ i ]);
            plot_saxs->removeCurve(plotted_cs_Gp_full[ i ]);
#else
            plotted_cs_Gp_curves[ i ]->detach();
            plotted_cs_Gp_curves_full[ i ]->detach();
#endif
         }

         if ( plotted_Rt_guinier_plotted.count(i) &&
              plotted_Rt_guinier_plotted[ i ] == true )
         {
            plotted_Rt_guinier_plotted[ i ] = false;
#ifndef QT4
            plot_saxs->removeCurve(plotted_Rt_Gp[ i ]);
            plot_saxs->removeCurve(plotted_Rt_Gp_full[ i ]);
#else
            plotted_Rt_Gp_curves[ i ]->detach();
            plotted_Rt_Gp_curves_full[ i ]->detach();
#endif
         }
         
         // remove the symbols & redraw the line
         QwtSymbol sym;
#ifndef QT4
         sym.setStyle(QwtSymbol::None);
         plot_saxs->setCurveSymbol(plotted_Iq[ i ], sym);
         plot_saxs->setCurveStyle(plotted_Iq[ i ], QwtCurve::Lines);
         plot_saxs->setCurvePen(plotted_Iq[ i ], QPen(plot_colors[i % plot_colors.size()], pen_width, SolidLine));
#else
         sym.setStyle(QwtSymbol::NoSymbol);
         plotted_Iq_curves[ i ]->setSymbol( QwtSymbol( sym ) );
         // plotted_Iq_curves[ i ]->setStyle( QwtPlotCurve::Lines );
         plotted_Iq_curves[ i ]->setPen( QPen( plot_colors[ i % plot_colors.size() ], pen_width, Qt::SolidLine ) );
#endif
      }

      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            q2I.push_back( plotted_q2[ i ][ j ] * plotted_I[ i ][ j ] );
         }
      }

      vector < double > pI;
      if ( cb_guinier->isChecked() && cb_cs_guinier->isChecked() )
      {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            pI.push_back( plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
         }
      }

      if ( cb_guinier->isChecked() && cb_Rt_guinier->isChecked() )
      {
         for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
         {
            pI.push_back( plotted_q[ i ][ j ] * plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
         }
      }

#ifndef QT4
      plot_saxs->setCurveData(
                              plotted_Iq[ i ], 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[ i ][0]) : (double *)&(plotted_q[ i ][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : // (double *)&(plotted_I[ i ][0]),
                              ( cb_guinier->isChecked() &&
                                ( cb_cs_guinier->isChecked() || cb_Rt_guinier->isChecked() ) ? (double *)&(pI[ 0 ]) : (double *)&(plotted_I[ i ][0]) ),
                              plotted_q[ i ].size()
                              );
#else
      plotted_Iq_curves[ i ]->setData(
                                    cb_guinier->isChecked() ?
                                    (double *)&(plotted_q2[ i ][0]) : (double *)&(plotted_q[ i ][0]), 
                                    cb_kratky ->isChecked() ?
                                    (double *)&(q2I[0])           : // (double *)&(plotted_I[ i ][0]),
                                    ( cb_guinier->isChecked() &&
                                      ( cb_cs_guinier->isChecked() || cb_Rt_guinier->isChecked() ) ? (double *)&(pI[ 0 ]) : (double *)&(plotted_I[ i ][0]) ),
                                    plotted_q[ i ].size()
                                    );
#endif
   }

   plot_saxs->setAxisTitle  ( QwtPlot::xBottom, 
                              cb_guinier->isChecked() ? 
                              tr( "q^2 (1/Angstrom^2)" ) : tr( "q (1/Angstrom)" ) );

   plot_saxs->setAxisTitle  ( QwtPlot::yLeft,   
                              cb_kratky ->isChecked() ? 
                              tr( " q^2 * I(q)"        ) : 
                              ( cb_guinier->isChecked() && cb_cs_guinier->isChecked() ? tr( "q*I(q) (log scale)" ) : 
                                ( cb_guinier->isChecked() && cb_Rt_guinier->isChecked() ? tr( "q^2*I(q) (log scale)" ) : tr( "I(q) (log scale)" ) ) )
                              );
#ifndef QT4
   plot_saxs->setAxisOptions( QwtPlot::yLeft, 
                              cb_kratky->isChecked()  ? 
                              QwtAutoScale::None         : QwtAutoScale::Logarithmic );
#else
   plot_saxs->setAxisScaleEngine(QwtPlot::yLeft, 
                                 cb_kratky->isChecked() ?
                                 new QwtLog10ScaleEngine :  // fix this
                                 new QwtLog10ScaleEngine);
#endif
   if ( plot_saxs_zoomer )
   {
      delete plot_saxs_zoomer;
   }
   plot_saxs_zoomer = new ScrollZoomer(plot_saxs->canvas());
#ifndef QT4
   plot_saxs_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
   plot_saxs_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#else
   plot_saxs_zoomer->setRubberBandPen( QPen( Qt::red, 1, Qt::DotLine ) );
   plot_saxs_zoomer->setTrackerPen( QPen( Qt::red ) );
#endif

   plot_saxs->replot();
}

void US_Hydrodyn_Saxs::clear_guinier()
{
   // remove any existing Guinier curves

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_Gp.size();
#else
         g < plotted_Gp_curves.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_Gp[g]);
#else
      plotted_Gp_curves[g]->detach();
#endif
   }

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_Gp_full.size();
#else
         g < plotted_Gp_curves_full.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_Gp_full[g]);
#else
      plotted_Gp_curves_full[g]->detach();
#endif
   }

#ifndef QT4
   plotted_Gp.clear();
   plotted_Gp_full.clear();
#else
   plotted_Gp_curves.clear();
   plotted_Gp_curves_full.clear();
#endif
   plotted_guinier_valid.clear();
   plotted_guinier_lowq2.clear();
   plotted_guinier_highq2.clear();
   plotted_guinier_a.clear();
   plotted_guinier_b.clear();
   plotted_guinier_x.clear();
   plotted_guinier_y.clear();
   plotted_guinier_pts_removed.clear();
}

void US_Hydrodyn_Saxs::clear_cs_guinier()
{
   // remove any existing Guinier curves

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_cs_Gp.size();
#else
         g < plotted_cs_Gp_curves.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_cs_Gp[g]);
#else
      plotted_cs_Gp_curves[g]->detach();
#endif
   }

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_cs_Gp_full.size();
#else
         g < plotted_cs_Gp_curves_full.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_cs_Gp_full[g]);
#else
      plotted_cs_Gp_curves_full[g]->detach();
#endif
   }

#ifndef QT4
   plotted_cs_Gp.clear();
   plotted_cs_Gp_full.clear();
#else
   plotted_cs_Gp_curves.clear();
   plotted_cs_Gp_curves_full.clear();
#endif
   plotted_cs_guinier_valid.clear();
   plotted_cs_guinier_lowq2.clear();
   plotted_cs_guinier_highq2.clear();
   plotted_cs_guinier_a.clear();
   plotted_cs_guinier_b.clear();
   plotted_cs_guinier_x.clear();
   plotted_cs_guinier_y.clear();
   plotted_cs_guinier_pts_removed.clear();
}

void US_Hydrodyn_Saxs::clear_Rt_guinier()
{
   // remove any existing Guinier curves

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_Rt_Gp.size();
#else
         g < plotted_Rt_Gp_curves.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_Rt_Gp[g]);
#else
      plotted_Rt_Gp_curves[g]->detach();
#endif
   }

   for ( unsigned int g = 0;
#ifndef QT4
         g < plotted_Rt_Gp_full.size();
#else
         g < plotted_Rt_Gp_curves_full.size();
#endif
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_Rt_Gp_full[g]);
#else
      plotted_Rt_Gp_curves_full[g]->detach();
#endif
   }

#ifndef QT4
   plotted_Rt_Gp.clear();
   plotted_Rt_Gp_full.clear();
#else
   plotted_Rt_Gp_curves.clear();
   plotted_Rt_Gp_curves_full.clear();
#endif
   plotted_Rt_guinier_valid.clear();
   plotted_Rt_guinier_lowq2.clear();
   plotted_Rt_guinier_highq2.clear();
   plotted_Rt_guinier_a.clear();
   plotted_Rt_guinier_b.clear();
   plotted_Rt_guinier_x.clear();
   plotted_Rt_guinier_y.clear();
   plotted_Rt_guinier_pts_removed.clear();
}

void US_Hydrodyn_Saxs::adjust_wheel( double pos )
{
   cout << QString("pos is now %1\n").arg(pos);
}


void US_Hydrodyn_Saxs::manual_guinier_fit_start_text( const QString & text )
{
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
}

void US_Hydrodyn_Saxs::manual_guinier_fit_end_text( const QString & text )
{
   if ( qwtw_wheel->value() != text.toDouble() )
   {
      qwtw_wheel->setValue( text.toDouble() );
   }
}

void US_Hydrodyn_Saxs::manual_guinier_fit_start_focus( bool hasFocus )
{
   cout << QString( "fit_start_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      // disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( join_offset_start, join_offset_end, join_offset_delta );
      // connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_manual_guinier_fit_start->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs::manual_guinier_fit_end_focus( bool hasFocus )
{
   cout << QString( "fit_end_focus %1\n" ).arg( hasFocus ? "true" : "false" );
   if ( hasFocus )
   {
      // disconnect( qwtw_wheel, SIGNAL( valueChanged( double ) ), 0, 0 );
      // qwtw_wheel->setRange( join_offset_start, join_offset_end, join_offset_delta );
      // connect( qwtw_wheel, SIGNAL( valueChanged( double ) ), SLOT( adjust_wheel( double ) ) );
      qwtw_wheel->setValue( le_manual_guinier_fit_end->text().toDouble() );
      qwtw_wheel->setEnabled( true );
   }
}

void US_Hydrodyn_Saxs::plot_guinier_pts_removed( int i, bool cs, bool Rt )
{
   // plot red X's for each removed pt
   if ( cs )
   {
      // puts( "plot_guinier_pts_removed cs" );
      if ( !plotted_cs_guinier_pts_removed.count( i ) ||
           !plotted_cs_guinier_pts_removed[ i ].size() )
      {
         // puts( "no cs pts removed" );
         return;
      }
      QwtSymbol sym;
      sym.setStyle(QwtSymbol::XCross);
      sym.setSize(12);
      sym.setBrush(Qt::red);
      sym.setPen  ( QPen( Qt::red) );
      for ( map < double, double >::iterator it = plotted_cs_guinier_pts_removed[ i ].begin();
            it != plotted_cs_guinier_pts_removed[ i ].end();
            it++ )
      {
         
#ifndef QT4
         long marker = plot_saxs->insertMarker();
         plot_saxs->setMarkerSymbol( marker, sym );
         plot_saxs->setMarkerPos   ( marker, it->first, exp( it->second ) );
#else
         QwtPlotMarker* marker = new QwtPlotMarker;
         marker->setSymbol( sym );
         marker->setValue( it->first, exp( it->second ) );
         marker->attach( plot_saxs );
#endif
      }
   } else {
      if ( Rt )
      {
         // puts( "plot_guinier_pts_removed Rt" );
         if ( !plotted_Rt_guinier_pts_removed.count( i ) ||
              !plotted_Rt_guinier_pts_removed[ i ].size() )
         {
            // puts( "no Rt pts removed" );
            return;
         }
         QwtSymbol sym;
         sym.setStyle(QwtSymbol::XCross);
         sym.setSize(12);
         sym.setBrush(Qt::red);
         sym.setPen  ( QPen( Qt::red) );
         for ( map < double, double >::iterator it = plotted_Rt_guinier_pts_removed[ i ].begin();
               it != plotted_Rt_guinier_pts_removed[ i ].end();
               it++ )
         {
         
#ifndef QT4
            long marker = plot_saxs->insertMarker();
            plot_saxs->setMarkerSymbol( marker, sym );
            plot_saxs->setMarkerPos   ( marker, it->first, exp( it->second ) );
#else
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( sym );
            marker->setValue( it->first, exp( it->second ) );
            marker->attach( plot_saxs );
#endif
         }
      } else {
         // puts( "plot_guinier_pts_removed" );
         if ( !plotted_guinier_pts_removed.count( i ) ||
              !plotted_guinier_pts_removed[ i ].size() )
         {
            // puts( "no pts removed" );
            return;
         }
         // plot red X's 
         QwtSymbol sym;
         sym.setStyle(QwtSymbol::XCross);
         sym.setSize(12);
         sym.setBrush(Qt::red);
         sym.setPen  (QPen( Qt::red) );

         for ( map < double, double >::iterator it = plotted_guinier_pts_removed[ i ].begin();
               it != plotted_guinier_pts_removed[ i ].end();
               it++ )
         {

            // cout << QString( "red marker %1 %2 %3\n" ).arg( it->first ).arg( it->second ).arg( exp( it->second ) );
         
#ifndef QT4
            long marker = plot_saxs->insertMarker();
            plot_saxs->setMarkerSymbol( marker, sym );
            plot_saxs->setMarkerPos   ( marker, it->first, exp( it->second ) );
#else
            QwtPlotMarker* marker = new QwtPlotMarker;
            marker->setSymbol( sym );
            marker->setValue( it->first, exp( it->second ) );
            marker->attach( plot_saxs );
#endif
         }
      }
   }
}

void US_Hydrodyn_Saxs::plot_guinier_error_bars( int i, bool cs, bool Rt )
{
   // take the curve and if sd's exist, plot a little line
   if ( plotted_I[ i ].size() != plotted_I_error[ i ].size() )
   {
      return;
   }

   vector < double > x(2);
   vector < double > y(2);

   double I;
   double e;
      
   if ( cs )
   {
      for ( int q = 0; q < ( int ) plotted_q[ i ].size(); ++q )
      {
         e = plotted_q[ i ][ q ] * plotted_I_error[ i ][ q ];
         if ( e )
         {
            x[ 0 ] = x[ 1 ] = plotted_q2[ i ][ q ];
            I = plotted_q[ i ][ q ] * plotted_I[ i ][ q ];
            y[ 0 ] = I - e;
            y[ 1 ] = I + e;
            // cout << QString( "errorbar q2 %1 r %2 sd %3\n" ).arg(  plotted_q2[ i ][ q ] ).arg( I ).arg( e );
#ifndef QT4
            plotted_guinier_error_bars.push_back( plot_saxs->insertCurve( qsl_plotted_iq_names[ i ] ) );
            plot_saxs->setCurveStyle( plotted_guinier_error_bars.back(), QwtCurve::Lines);
            plot_saxs->setCurveData( plotted_guinier_error_bars.back(),
                                     (double *)&(x[0]),
                                     (double *)&(y[0]),
                                     2 );
            plot_saxs->setCurvePen( plotted_guinier_error_bars.back(), QPen(plot_colors[i % plot_colors.size()], pen_width, SolidLine));
#else
            plotted_guinier_error_bars.push_back( new QwtPlotCurve( qsl_plotted_iq_names[ i ] ) );
            plotted_guinier_error_bars.back()->setStyle( QwtPlotCurve::Lines );
            plotted_guinier_error_bars.back()->setData(
                                                       (double *)&(x[0]),
                                                       (double *)&(y[0]),
                                                       2 );
            plotted_guinier_error_bars.back()->setPen( QPen( plot_colors[ i % plot_colors.size() ], pen_width, Qt::SolidLine ) );
            plotted_guinier_error_bars.back()->attach( plot_saxs );
#endif
         }
      }
   } else {
      if ( Rt )
      {
         for ( int q = 0; q < ( int ) plotted_q[ i ].size(); ++q )
         {
            e = plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I_error[ i ][ q ];
            if ( e )
            {
               x[ 0 ] = x[ 1 ] = plotted_q2[ i ][ q ];
               I = plotted_q[ i ][ q ] * plotted_q[ i ][ q ] * plotted_I[ i ][ q ];
               y[ 0 ] = I - e;
               y[ 1 ] = I + e;
               // cout << QString( "errorbar q2 %1 r %2 sd %3\n" ).arg(  plotted_q2[ i ][ q ] ).arg( I ).arg( e );
#ifndef QT4
               plotted_guinier_error_bars.push_back( plot_saxs->insertCurve( qsl_plotted_iq_names[ i ] ) );
               plot_saxs->setCurveStyle( plotted_guinier_error_bars.back(), QwtCurve::Lines);
               plot_saxs->setCurveData( plotted_guinier_error_bars.back(),
                                        (double *)&(x[0]),
                                        (double *)&(y[0]),
                                        2 );
               plot_saxs->setCurvePen( plotted_guinier_error_bars.back(), QPen(plot_colors[i % plot_colors.size()], pen_width, SolidLine));
#else
               plotted_guinier_error_bars.push_back( new QwtPlotCurve( qsl_plotted_iq_names[ i ] ) );
               plotted_guinier_error_bars.back()->setStyle( QwtPlotCurve::Lines );
               plotted_guinier_error_bars.back()->setData(
                                                          (double *)&(x[0]),
                                                          (double *)&(y[0]),
                                                          2 );
               plotted_guinier_error_bars.back()->setPen( QPen( plot_colors[ i % plot_colors.size() ], pen_width, Qt::SolidLine ) );
               plotted_guinier_error_bars.back()->attach( plot_saxs );
#endif
            }
         }
      } else {
         for ( int q = 0; q < ( int ) plotted_q[ i ].size(); ++q )
         {
            e = plotted_I_error[ i ][ q ];
            if ( e )
            {
               I = plotted_I[ i ][ q ];
               x[ 0 ] = x[ 1 ] = plotted_q2[ i ][ q ];
               y[ 0 ] = I - e;
               y[ 1 ] = I + e;
               // cout << QString( "errorbar q2 %1 r %2 sd %3\n" ).arg(  plotted_q2[ i ][ q ] ).arg( I ).arg( e );

#ifndef QT4
               plotted_guinier_error_bars.push_back( plot_saxs->insertCurve( qsl_plotted_iq_names[ i ] ) );
               plot_saxs->setCurveStyle( plotted_guinier_error_bars.back(), QwtCurve::Lines);
               plot_saxs->setCurveData( plotted_guinier_error_bars.back(),
                                        (double *)&(x[0]),
                                        (double *)&(y[0]),
                                        2 );
               plot_saxs->setCurvePen( plotted_guinier_error_bars.back(), QPen(plot_colors[i % plot_colors.size()], pen_width, SolidLine));
#else
               plotted_guinier_error_bars.push_back( new QwtPlotCurve( qsl_plotted_iq_names[ i ] ) );
               plotted_guinier_error_bars.back()->setStyle( QwtPlotCurve::Lines );
               plotted_guinier_error_bars.back()->setData(
                                                          (double *)&(x[0]),
                                                          (double *)&(y[0]),
                                                          2 );
               plotted_guinier_error_bars.back()->setPen( QPen( plot_colors[ i % plot_colors.size() ], pen_width, Qt::SolidLine ) );
               plotted_guinier_error_bars.back()->attach( plot_saxs );
#endif
            }
         }
      }
   }
}

void US_Hydrodyn_Saxs::clear_guinier_error_bars()
{
   for ( unsigned int g = 0;
         g < plotted_guinier_error_bars.size();
         g++ )
   {
#ifndef QT4
      plot_saxs->removeCurve(plotted_guinier_error_bars[g]);
#else
      plotted_guinier_error_bars[g]->detach();
#endif
   }
   plotted_guinier_error_bars.clear();
}

void US_Hydrodyn_Saxs::manual_guinier_process()
{
}

