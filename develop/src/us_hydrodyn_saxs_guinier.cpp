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

void US_Hydrodyn_Saxs::run_guinier_cs()
{
   editor->append("CS Guinier analysis:\n");
   clear_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
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
      "\"chi^2\","
      "\"Conc. (mg/ml)\","
      "\"PSV (ml/g)\","
      "\"I0 std. expt. (ml/g)\","
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

      if ( !f.open(IO_WriteOnly) )
      {
         editor_msg( "red", QString(tr("Can not create file %1\n")).arg(f.name()));
      }
      QTextStream ts(&f);
      ts << csvlog;
      f.close();
      editor->append(QString(tr("Created file %1\n")).arg(f.name()));
   }
}

void US_Hydrodyn_Saxs::run_guinier_analysis()
{
   editor->append("Guinier analysis:\n");
   editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, plot_saxs->canvasBackground() );
   clear_guinier();

   QString csvlog = 
      "\"Source file\","
      "\"Notes\","
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
      "\"chi^2\","
      "\"Conc. (mg/ml)\","
      "\"PSV (ml/g)\","
      "\"I0 std. expt. (ml/g)\","
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

      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString(tr("Can not create file %1\n")).arg(f.name()));
      }
      QTextStream ts(&f);
      ts << csvlog;
      f.close();
      editor->append(QString(tr("Created file %1\n")).arg(f.name()));
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
   if ( use_SD_weighting &&
        ( plotted_I_error[ i ].size() != plotted_q[ i ].size() ||
          !is_nonzero_vector( plotted_I_error[ i ] ) ) )
   {
      editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are missing or zero" ) )
                  .arg( qsl_plotted_iq_names[ i ] ) );
      use_SD_weighting = false;
   }

   //    if ( use_SD_weighting )
   //    {
   //       cout << "uhsg: using sd weighting\n";
   //    } else {
   //       cout << "uhsg: not using sd weighting\n";
   //    }

   for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
   {
      if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart )
      {
         usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
         usu.wave["data"].r.push_back( plotted_I[ i ][ j ] );
         if ( use_SD_weighting )
         {
            usu.wave["data"].s.push_back( plotted_I_error[ i ][ j ] );
         }
      }
   }
   // US_Vector::printvector( "q", usu.wave["data"].q );
   // US_Vector::printvector( "I", usu.wave["data"].r );
   // US_Vector::printvector( "e", usu.wave["data"].s );

   QString log;

   int pointsmin = our_saxs_options->pointsmin;
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = our_saxs_options->qRgmax;
   double pointweightpower = 3e0;
   double p_guinier_minq = our_saxs_options->qstart;
   double p_guinier_maxq = our_saxs_options->qend;
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double Io;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[i].size() <= 5;

   if ( !too_few_points )
   {
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" )
      {
         if ( 
             !usu.guinier_plot(
                               "guinier",
                               "data"
                               )   ||
             !usu.guinier_fit2(
                               log,
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
                               Io,
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
      } else {
         unsigned int pstart = 0;
         unsigned int pend   = 0;
         {
            bool         pstart_found = false;
            for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
            {
               if ( !pstart_found && plotted_q[ i ][ j ] >= p_guinier_minq )
               {
                  pstart = j;
                  pstart_found = true;
               }
               if ( plotted_q[ i ][ j ] <= p_guinier_maxq )
               {
                  pend = j;
               }
            }
         }
         bestend = pend;
         beststart = pstart;

         if ( 
             !usu.guinier_plot(
                               "guinier",
                               "data"
                               )   ||
             !usu.guinier_fit(
                              log,
                              "guinier", 
                              pstart,
                              pend,
                              a,
                              b,
                              siga,
                              sigb,
                              chi2,
                              Rg,
                              Io,
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
      }         

      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   QString report;
   if ( too_few_points )
   {
      report =
         QString(
                 "Guinier analysis of %1:\n"
                 "**** Could not compute Rg, too few data points %2 ****\n"
                 )
         .arg(plotted_q[i].size());

      csvlog += 
         QString(
                 "\"%1\","
                 "\"Too few data points (%2)\"\n"
                 )
         .arg(qsl_plotted_iq_names[i])
         .arg(plotted_q[i].size());
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         plotted_guinier_valid[i] = false;
         report =
            QString(
                    "Guinier analysis of %1:\n"
                    "**** Could not compute Rg ****\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rg\"\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

      } else {
         double MW;
         double MW_sd;
         double ICL;

         if ( mw_from_I0( qsl_plotted_iq_names[ i ], siga, MW_sd, ICL ) )
         {
            // cout << QString( "siga %1 MW_sd %2 ICL %3\n" ).arg( siga ).arg( MW_sd ).arg( ICL );
         } else {
            // cout << errormsg << endl;
         }

         if ( !mw_from_I0( qsl_plotted_iq_names[ i ], Io, MW, ICL ) )
         {
            editor_msg( "red", errormsg );
            // cout << errormsg;
         } else {
            // cout << QString( "I0 %1 MW %2 ICL %3\n" ).arg( Io ).arg( MW ).arg( ICL );
         }

         report = 
            QString("")
            .sprintf(
                     "Guinier analysis of %s:\n"
                     "Rg %.1f (%.1f) (A) I(0) %.2e (%.2e) MW %.2e (%.2e) qmin %.5f qmax %.5f qRgmin %.3f qRgmax %.3f points used %u chi^2 %.2e\n"
                     , qsl_plotted_iq_names[i].ascii()
                     , Rg
                     , sigb
                     , Io
                     , siga
                     , MW
                     , MW_sd
                     , smin
                     , smax
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1
                     , chi2
                     );
         
         plotted_guinier_valid[i] = true;
         plotted_guinier_lowq2[i] = smin * smin;
         plotted_guinier_highq2[i] = smax * smax;
         plotted_guinier_a[i] = a;
         plotted_guinier_b[i] = b;
         plotted_guinier_plotted[i] = false;
         
         plotted_guinier_x[i].clear();
         plotted_guinier_x[i].push_back(plotted_guinier_lowq2[i]);
         plotted_guinier_x[i].push_back(plotted_guinier_highq2[i]);
         
         plotted_guinier_y[i].clear();
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_lowq2[i]));
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_highq2[i]));

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\","
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    )
            .arg( qsl_plotted_iq_names[i] )
            .arg( Rg )
            .arg( sigb )
            .arg( Io )
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
            .arg(smin)
            .arg(smax)
            .arg(sRgmin)
            .arg(sRgmax)
            .arg(beststart)
            .arg(bestend)
            .arg(bestend - beststart + 1)
            .arg(chi2)
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
                       "\"%4\","
                       )
               .arg( conc )
               .arg( psv )
               .arg( I0_std_exp )
               .arg( tr( use_SD_weighting ? "used" : "not used" ) )
               ;
         }

         csvlog += "\n";
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

   bool use_SD_weighting = our_saxs_options->guinier_use_sd;
   if ( use_SD_weighting &&
        ( plotted_I_error[ i ].size() != plotted_q[ i ].size() ||
          !is_nonzero_vector( plotted_I_error[ i ] ) ) )
   {
      editor_msg( "dark red", QString( tr( "Notice: SD weighting of Guinier fit is off for %1 since SDs are missing or zero" ) )
                  .arg( qsl_plotted_iq_names[ i ] ) );
      use_SD_weighting = false;
   }

   US_Saxs_Util usu;
   usu.wave["data"].q.clear();
   usu.wave["data"].r.clear();
   usu.wave["data"].s.clear();
   for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
   {
      if ( plotted_q[ i ][ j ] >= our_saxs_options->qstart )
      {
         usu.wave["data"].q.push_back( plotted_q[ i ][ j ] );
         usu.wave["data"].r.push_back( plotted_q[ i ][ j ] * plotted_I[ i ][ j ] );
         if ( use_SD_weighting )
         {
            usu.wave["data"].s.push_back( plotted_q[ i ][ j ] * plotted_I_error[ i ][ j ] );
         }
      }
   }
   QString log;

   int pointsmin = our_saxs_options->pointsmin;
   int pointsmax = our_saxs_options->pointsmax;
   double sRgmaxlimit = our_saxs_options->cs_qRgmax;
   double pointweightpower = 3e0;
   double p_guinier_minq = our_saxs_options->qstart;
   double p_guinier_maxq = our_saxs_options->qend;
   
   // these are function output values
   double a;
   double b;
   double siga;
   double sigb;
   double chi2;
   double Rg;
   double Io;
   double smin;
   double smax;
   double sRgmin;
   double sRgmax;

   unsigned int beststart;
   unsigned int bestend;

   bool too_few_points = plotted_q[i].size() <= 5;

   if ( !too_few_points )
   {
      if ( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_auto_fit" ) &&
           ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_auto_fit" ] == "1" )
      {
         if ( 
             !usu.guinier_plot(
                               "cs_guinier",
                               "data"
                               )   ||
             !usu.guinier_fit2(
                               log,
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
                               Io,
                               smax, // don't know why these are flipped
                               smin,
                               sRgmin,
                               sRgmax,
                               beststart,
                               bestend
                               ) )
         {
            editor_msg( "red",
                        QString("Error performing CS Guinier analysis on %1\n" + usu.errormsg + "\n")
                        .arg(qsl_plotted_iq_names[i]));
            return false;
         }
      } else {
         unsigned int pstart = 0;
         unsigned int pend   = 0;
         {
            bool         pstart_found = false;
            for ( unsigned int j = 0; j < plotted_q[ i ].size(); j++ )
            {
               if ( !pstart_found && plotted_q[ i ][ j ] >= p_guinier_minq )
               {
                  pstart = j;
                  pstart_found = true;
               }
               if ( plotted_q[ i ][ j ] <= p_guinier_maxq )
               {
                  pend = j;
               }
            }
         }
         bestend = pend;
         beststart = pstart;

         if ( 
             !usu.guinier_plot(
                               "guinier",
                               "data"
                               )   ||
             !usu.guinier_fit(
                              log,
                              "guinier", 
                              pstart,
                              pend,
                              a,
                              b,
                              siga,
                              sigb,
                              chi2,
                              Rg,
                              Io,
                              smax, // don't know why these are flipped
                              smin,
                              sRgmin,
                              sRgmax
                              ) )
         {
            editor->append(QString("Error performing CS Guinier analysis on %1\n" + usu.errormsg + "\n")
                           .arg(qsl_plotted_iq_names[i]));
            return false;
         }
      }

      // cout << "guinier siga " << siga << endl;
      // cout << "guinier sigb " << sigb << endl;
   
      // cout << log;
   }

   QString report;
   if ( too_few_points )
   {
      report =
         QString(
                 "CS Guinier analysis of %1:\n"
                 "**** Could not compute Rc, too few data points %2 ****\n"
                 )
         .arg(plotted_q[i].size());

      csvlog += 
         QString(
                 "\"%1\","
                 "\"Too few data points (%2)\"\n"
                 )
         .arg(qsl_plotted_iq_names[i])
         .arg(plotted_q[i].size());
   } else {
      if ( isnan(Rg) ||
           b >= 0e0 )
      {
         plotted_guinier_valid[i] = false;
         report =
            QString(
                    "CS Guinier analysis of %1:\n"
                    "**** Could not compute Rc ****\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Could not compute Rc\"\n"
                    )
            .arg(qsl_plotted_iq_names[i]);

      } else {
         double ML;
         double ML_sd;
         double ICL;

         ml_from_qI0( qsl_plotted_iq_names[ i ], siga, ML_sd, ICL );
         if ( !mw_from_I0( qsl_plotted_iq_names[ i ], Io, ML, ICL ) )
         {
            editor_msg( "red", errormsg );
         }

         report = 
            QString("")
            .sprintf(
                     "CS Guinier analysis of %s:\n"
                     "Rc %.1f (%.1f) (A) I(0) %.2e (%.2e) M/L %.2e (%.2e) qRcmin %.3f qRcmax %.3f points used %u chi^2 %.2e\n"
                     
                     , qsl_plotted_iq_names[i].ascii()
                     , Rg
                     , sigb 
                     , Io
                     , siga
                     , ML
                     , ML_sd
                     , sRgmin
                     , sRgmax
                     , bestend - beststart + 1
                     , chi2
                     );
         
         plotted_guinier_valid[i] = true;
         plotted_guinier_lowq2[i] = smin * smin;
         plotted_guinier_highq2[i] = smax * smax;
         plotted_guinier_a[i] = a;
         plotted_guinier_b[i] = b;
         plotted_guinier_plotted[i] = false;
         
         plotted_guinier_x[i].clear();
         plotted_guinier_x[i].push_back(plotted_guinier_lowq2[i]);
         plotted_guinier_x[i].push_back(plotted_guinier_highq2[i]);
         
         plotted_guinier_y[i].clear();
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_lowq2[i]));
         plotted_guinier_y[i].push_back(exp(plotted_guinier_a[i] + plotted_guinier_b[i] * plotted_guinier_highq2[i]));

         csvlog += 
            QString(
                    "\"%1\","
                    "\"Ok\","
                    "%2,"
                    "%3,"
                    "%4,"
                    "%5,"
                    "%6,"
                    "%7,"
                    "%8,"
                    )
            .arg(qsl_plotted_iq_names[i])
            .arg(Rg)
            .arg( sigb )
            .arg(Io)
            .arg(siga)
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
                    "%8"
                    )
            .arg(smin)
            .arg(smax)
            .arg(sRgmin)
            .arg(sRgmax)
            .arg(beststart)
            .arg(bestend)
            .arg(bestend - beststart + 1)
            .arg(chi2)
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
                       "\"%4\","
                       )
               .arg( conc )
               .arg( psv )
               .arg( I0_std_exp )
               .arg( tr( use_SD_weighting ? "used" : "not used" ) )
               ;
         }

         csvlog += "\n";
      }
   }
   editor_msg( plot_colors[i % plot_colors.size()], plot_saxs->canvasBackground(), report );

   //   cout << csvlog;
   return true;
}

