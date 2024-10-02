#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_math.h"

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
// #  define isnan _isnan
#endif

void US_Hydrodyn_Saxs::calc_iqq_nnls_fit( QString /* title */, QString csv_filename )
{
   // setup nnls run:
   // editor->append("setting up nnls run\n");
   // unify dimension of nnls_A vectors
   unsigned int max_iqq_len = 0;

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_nnls", Qt::CaseInsensitive )) )
   {
      tagged_csv_filename += "_nnls";
   }

   // #define DEBUG_NNLS

   for ( map < QString, vector < double > >::iterator it = nnls_A.begin();
        it != nnls_A.end();
        it++ )
   {
      if ( it == nnls_A.begin() )
      {
         max_iqq_len = it->second.size();
      }
      if ( it->second.size() != max_iqq_len ) 
      {
         editor_msg("red", QString(us_tr("NNLS failed, length mismatch %1 %2\n")).arg( it->second.size()).arg(max_iqq_len));
         nnls_csv_footer << QString(us_tr("\"NNLS failed, length mismatch %1 %2\"")).arg( it->second.size()).arg(max_iqq_len);
         return;
      }
   }

   // cout << "max_iqq_len " << max_iqq_len << endl;

   vector < double > use_A;

   vector < QString > model_names;

   for ( map < QString, vector < double > >::iterator it = nnls_A.begin();
        it != nnls_A.end();
        it++ )
   {
      model_names.push_back(it->first);
      for ( unsigned int i = 0; i < max_iqq_len; i++ )
      {
         use_A.push_back(it->second[i]);
      }
   }

   unsigned int org_size = nnls_B.size();
   if ( org_size != max_iqq_len )
   {
      editor_msg("red", QString(us_tr("NNLS failed, target length mismatch %1 %2\n")).arg(org_size).arg(max_iqq_len));
      nnls_csv_footer << QString(us_tr("\"NNLS failed, target length mismatch %1 %2\"")).arg(org_size).arg(max_iqq_len);
      return;
   }

   bool use_errors = is_nonzero_vector( nnls_errors );
   if ( our_saxs_options->iqq_scale_nnls )
   {
      use_errors = false;
   }
   if ( use_errors && !use_SDs_for_fitting_iqq ) {
      use_errors = false;
   }
   
   unsigned int org_errors_size = nnls_errors.size();
   if ( use_errors && org_errors_size != max_iqq_len )
   {
      editor_msg("red", QString(us_tr("NNLS failed, target length mismatch %1 %2\n")).arg(org_errors_size).arg(max_iqq_len));
      nnls_csv_footer << QString(us_tr("\"NNLS failed, target length mismatch %1 %2\"")).arg(org_errors_size).arg(max_iqq_len);
      return;
   }

   editor_msg("dark blue", 
              use_errors ? 
              us_tr( "using standard deviations to compute NNLS\n" ) :
              us_tr( "NOT using standard deviations to compute NNLS\n" ) );

   nnls_csv_footer <<
      ( use_errors ? 
        us_tr( "\"using standard deviations to compute NNLS\"" ) :
        us_tr( "\"NOT using standard deviations to compute NNLS\"" ) )
      ;
   
   vector < double > use_B = nnls_B;
   vector < double > use_q = nnls_q;
   vector < double > use_x(nnls_A.size());
   vector < double > nnls_wp(nnls_A.size());
   vector < double > nnls_zzp(use_B.size());
   vector < int > nnls_indexp(nnls_A.size());

   if ( our_saxs_options->iqq_kratky_fit )
   {
      // first check for non-zero
      editor_msg( "blue",
                  "Notice: Kratky fit (q^2*I)\n" );
      nnls_csv_footer <<
         "\"Notice: Kratky fit (q^2*I)\"";
      for ( unsigned int i = 0; i < use_A.size(); i++ )
      {
         double q2 = use_q[ i % use_B.size() ] * use_q[ i % use_B.size() ];
         use_A[ i ] *= q2;
      }
      for ( unsigned int i = 0; i < use_B.size(); i++ )
      {
         use_B[ i ] *= use_q[ i ] * use_q[ i ];
      }
   }

   if ( use_errors )
   {
      for ( unsigned int i = 0; i < nnls_errors.size(); i++ )
      {
         use_B[ i ] /= nnls_errors[ i ];
      }
      for ( unsigned int i = 0; i < use_A.size(); i++ )
      {
         use_A[ i ] /= nnls_errors[ i % max_iqq_len ];
      }
   }         

   //   editor->append(QString("running nnls %1 %2\n").arg(nnls_A.size()).arg(use_B.size()));
   if ( our_saxs_options->iqq_log_fitting )
   {
      // first check for positive
      bool log_ok = true;
      for ( unsigned int i = 0; i < use_B.size(); i++ )
      {
         if ( use_B[ i ] <= 0 ) 
         {
            log_ok = false;
            break;
         }
      }

      if ( log_ok )
      {
         for ( unsigned int i = 0; i < use_A.size(); i++ )
         {
            if ( use_A[ i ] <= 0 ) 
            {
               log_ok = false;
               break;
            }
         }
      }

      if ( !log_ok )
      {
         editor_msg( "red",
                     "Warning: Log fitting requested but some of the values are less than or equal to zero, so log fitting disabled\n" );
         nnls_csv_footer <<
            "\"Warning: Log fitting requested but some of the values are less than or equal to zero, so log fitting disabled\"";
      } else {
         // compute log10 on A & B
         editor_msg( "blue",
                     "Notice: Log fitting\n" );
         nnls_csv_footer <<
            "\"Notice: Log fitting\"";
         for ( unsigned int i = 0; i < use_B.size(); i++ )
         {
            use_B[ i ] = log10( use_B[ i ] );
         }
         for ( unsigned int i = 0; i < use_A.size(); i++ )
         {
            use_A[ i ] = log10( use_A[ i ] );
         }
      }
   }

   editor->append("Running NNLS\n");
   
   int result =
      nnls(
           (double *)&use_A[0],
           (int) max_iqq_len,
           (int) max_iqq_len,
           (int) nnls_A.size(),
           (double *)&use_B[0],
           (double *)&use_x[0],
           &nnls_rmsd,
           (double *)&nnls_wp[0],
           (double *)&nnls_zzp[0],
           (int *)&nnls_indexp[0]);

   if ( result != 0 )
   {
      editor->append("NNLS error!\n");
      nnls_csv_footer <<
         "\"NNLS error!\"";
   }
   
   editor->append(QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd));

   nnls_csv_footer <<
      QString("\"Residual Euclidian norm of NNLS fit\",%1").arg(nnls_rmsd);

   vector < double > rescaled_x = our_saxs_options->disable_nnls_scaling ? use_x : rescale(use_x);

   if ( our_saxs_options->disable_nnls_scaling ) {
      editor_msg( "darkred", "NNLS scaling disabled\n" );
      nnls_csv_footer <<
         "\"NNLS scaling disabled\"";
   }

   // list models & concs
   
   QString use_color = "black";
   bool nnls_zero_list =
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "nnls_zero_list" ) ?
      ((US_Hydrodyn *)us_hydrodyn)->gparams[ "nnls_zero_list" ] == "true" : false;


   // sort names
   class sortable_model {
   public:
      QString      name;
      unsigned int number;

      bool operator < (const sortable_model & objIn) const {
         static QRegularExpression rx( "^(.*) Model: (\\d+)" );

         // does the objIn contain a model
         QRegularExpressionMatch matchIn = rx.match( objIn.name );
         if ( !matchIn.hasMatch() ) {
            return name < objIn.name;
         }

         // does the name contain a model
         QRegularExpressionMatch match = rx.match( name );
         if ( !match.hasMatch() ) {
            return name < objIn.name;
         }
         
         if ( match.captured(1) == matchIn.captured(1) ) {
            return match.captured(2).toUInt() < matchIn.captured(2).toUInt();
         }
         return name < objIn.name;
      }
   };
   
   list < sortable_model > sort_models;
   for ( unsigned int i = 0; i < use_x.size(); i++ ) {
      sortable_model m;
      m.name = model_names[i];
      m.number = i;
      sort_models.push_back( m );
   }

   sort_models.sort();
   
   // for ( auto it = sort_models.begin();
   //       it != sort_models.end();
   //       ++it ) {
   //    QTextStream(stdout) << QString( "model name %1 number %2\n" ).arg( it->name ).arg( it->number );
   // }

   // for ( unsigned int i = 0; i < use_x.size(); i++ ) {

   QRegularExpression rx( "^(.*) Model: (\\d+)" );
   QRegularExpression rx2( "^(.*)\\D(\\d+)\\.[^.]+$" );

   vector < int > contrib_to_plot;

   for ( auto it = sort_models.begin();
         it != sort_models.end();
         ++it ) {
      unsigned int i = it->number;
      if ( rescaled_x[i] == 0 )
      {
         if ( !nnls_zero_list ) {
            continue;
         }
         use_color = "gray";
      } else {
         if ( rescaled_x[i] < .1 )
         {
            use_color = "darkCyan";
         } else {
            if ( rescaled_x[i] < .2 )
            {
               use_color = "blue";
            } else {
               use_color = "darkBlue";
            }
         }
      }
      {
         QString model_name = model_names[i];
         model_name.replace( "\"", "" );

         QRegularExpressionMatch match = rx.match( model_name );
         
         if ( match.hasMatch() ) {
            nnls_csv_data <<
               QString("\"%1\",%2,%3").arg(match.captured(1)).arg(match.captured(2)).arg(rescaled_x[i]);
         } else {
            QRegularExpressionMatch match = rx2.match( model_name );
            if ( match.hasMatch() ) {
               nnls_csv_data <<
                  QString("\"%1\",%2,%3").arg(model_name).arg(match.captured(2)).arg(rescaled_x[i]);
            } else {
               nnls_csv_data <<
                  QString("\"%1\",,%2").arg(model_name.replace( "\"", "" )).arg(rescaled_x[i]);
            }
         }
      }      

      editor_msg( use_color, QString("%1 %2\n").arg(model_names[i]).arg(rescaled_x[i] ) );

      if ( nnls_plot_contrib && rescaled_x[i] > 0 ) {
         contrib_to_plot.push_back( i );
      }
   }

   nnls_csv_footer
      << QString( "\"Number of curves in the fit\",%1" ).arg( use_x.size() )
      ;
   
   // build model & residuals
   vector < double > model(use_B.size());
   vector < double > residual(use_B.size());
   vector < double > difference(use_B.size());
   
   for ( unsigned int i = 0; i < use_B.size(); i++ )
   {
      model[i] = 0e0;
      for ( unsigned int j = 0; j < use_x.size(); j++ )
      {
         model[i] += use_x[j] * nnls_A[model_names[j]][i];
      }
      residual[i] = fabs(model[i] - nnls_B[i]);
      difference[i] = nnls_B[i] - model[i];
   }
   
   // plot 
   
   if ( is_nonzero_vector( nnls_errors ) )
   {
      plot_one_iqq(nnls_r, nnls_B, nnls_errors, csv_filename + " Target");
   } else {
      plot_one_iqq(nnls_r, nnls_B, csv_filename + " Target");
   }

   QString target_name = qsl_plotted_iq_names[ qsl_plotted_iq_names.size() - 1 ];
   rescale_iqq_curve( target_name, nnls_r, model );
   plot_one_iqq(nnls_r, model, csv_filename + " Model");
   
   for ( int i = 0; i < (int) contrib_to_plot.size(); ++i ) {
      vector < double > rescaled_A = nnls_A[model_names[contrib_to_plot[i]]];
      vector < double > no_errors;
      rescale_iqq_curve( target_name, nnls_r, rescaled_A, false );
      plot_one_iqq( nnls_r, rescaled_A, no_errors, model_names[contrib_to_plot[i]] );
   }

   if ( plotted )
   {
      set_eb();
      editor_msg( "black", "I(q) plot done\n");
      plotted = false;
   }
   
   // save as csv
   if ( !csv_filename.isEmpty() )
   {
      // cout << "save_to_csv\n";
      QString fname = 
         ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
         tagged_csv_filename + "_iqq.csv";
      
      if ( QFile::exists(fname) )
         // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
      }         
      FILE *of = us_fopen(fname, "wb");
      if ( of )
      {
         fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).toLatin1().data(),
                 QString("NNLS fit residual %1 : %2").arg(nnls_rmsd).arg(nnls_header_tag).toLatin1().data());
         // original models
         for ( unsigned int i = 0; i < use_x.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rescaled_x[i]).toLatin1().data(),
                    "I(q)",
                    vector_double_to_csv(nnls_A[model_names[i]]).toLatin1().data());
         }
         // target
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                 "I(q)",
                 vector_double_to_csv(nnls_B).toLatin1().data());
         
         // are there errors?
         if ( is_nonzero_vector( nnls_errors ) )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                    "I(q) sd",
                    vector_double_to_csv(nnls_errors).toLatin1().data());
         }

         // best fit model
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 "Model",
                 "I(q)",
                 vector_double_to_csv(model).toLatin1().data());
         
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 "Residual",
                 "I(q)",
                 vector_double_to_csv(residual).toLatin1().data());
         
         fclose(of);
         if ( plotted )
         {
            set_eb();
            editor_msg( "black", "I(q) plot done\n" );
            plotted = false;
         }
         editor->append(us_tr("Created file: " + fname + "\n"));
      } else {
         editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n" ) );
      }
   }
}

void US_Hydrodyn_Saxs::calc_iqq_best_fit( QString /* title */, QString csv_filename )
{
   // setup best_fit run, idea:
   //    use US_Saxs_Util::liner_fit
   //    give each model a chi^2
   //    sort by chi^2

   map < QString, vector < double > > best_fit_models = nnls_A;
   vector < double >                  best_fit_target = nnls_B;

   // editor->append("setting up best fit run\n");
   // unify dimension of best_fit_models vectors

   unsigned int max_iqq_len = 0;

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_best_fit", Qt::CaseInsensitive )) )
   {
      tagged_csv_filename += "_best_fit";
   }

   for ( map < QString, vector < double > >::iterator it = best_fit_models.begin();
        it != best_fit_models.end();
        it++ )
   {
      if ( it == best_fit_models.begin() )
      {
         max_iqq_len = it->second.size();
      }
      if ( it->second.size() != max_iqq_len ) 
      {
         editor_msg("red", QString(us_tr("Best fit failed, length mismatch %1 %2\n")).arg( it->second.size()).arg(max_iqq_len));
         return;
      }
   }

   bool use_errors = is_nonzero_vector( nnls_errors );
   if ( our_saxs_options->iqq_scale_nnls )
   {
      use_errors = false;
   }
   if ( use_errors && !use_SDs_for_fitting_iqq ) {
      use_errors = false;
   }

   vector < QString > model_names;

   for ( map < QString, vector < double > >::iterator it = best_fit_models.begin();
        it != best_fit_models.end();
        it++ )
   {
      model_names.push_back(it->first);
   }

   unsigned int org_size = best_fit_target.size();
   if ( org_size != max_iqq_len )
   {
      editor_msg("red", QString(us_tr("Best fit failed, target length mismatch %1 %2\n")).arg(org_size).arg(max_iqq_len));
      return;
   }

   vector < double > b(best_fit_models.size());
   vector < double > chi2s(best_fit_models.size());
   vector < double > model;

   double lowest_chi2     = 9e99;
   int    lowest_chi2_pos = 0;

   //   editor->append(QString("running best fit %1 %2\n").arg(best_fit_models.size()).arg(best_fit_target.size()));
   // save saxs plot
   vector < vector < double > > qs;
   vector < vector < double > > Is;
   vector < vector < double > > I_errors;
   vector < QString >           names;

   for ( unsigned int i = 0; i < plotted_I_error.size(); i++ )
   {
      qs.      push_back( plotted_q           [ i ] );
      Is.      push_back( plotted_I           [ i ] );
      I_errors.push_back( plotted_I_error     [ i ] );
      names.   push_back( qsl_plotted_iq_names[ i ] );
   }

   editor->append("Running best fit\n");
   clear_plot_saxs( true );

   if ( is_nonzero_vector( nnls_errors ) )
   {
      plot_one_iqq(nnls_r, nnls_B, nnls_errors, csv_filename + " Target");
   } else {
      plot_one_iqq(nnls_r, nnls_B, csv_filename + " Target");
   }
   QString use_target = qsl_plotted_iq_names[ qsl_plotted_iq_names.size() - 1 ];

   map < double, vector < QString > > map_sorted_fits;

   for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
   {
      editor->append(model_names[i] + " :");
      rescale_iqq_curve( use_target, nnls_r, best_fit_models[model_names[i]] );
      chi2s[i] = last_rescaling_chi2;
      map_sorted_fits[ chi2s[i] ].push_back( model_names[i] );
      if ( !i )
      {
         lowest_chi2 = chi2s[i];
         lowest_chi2_pos = 0;
         model = best_fit_models[model_names[i]];
      } else {
         if ( lowest_chi2 > chi2s[i] )
         {
            lowest_chi2 = chi2s[i];
            lowest_chi2_pos = i;
            model = best_fit_models[model_names[i]];
         }
      }
   }

   {
      QString msg;
      for ( map < double, vector < QString > >::iterator it = map_sorted_fits.begin();
            it != map_sorted_fits.end();
            ++it ) {
         for ( int i = 0; i < (int) it->second.size(); ++i ) {
            msg += QString( "%1 : %2\n" ).arg( it->first ).arg( it->second[ i ] );
         }
      }
      editor_msg( "black", us_tr( "Sorted best fit results\n" ) + msg );
   }

   // clear residuals window

   if ( saxs_iqq_residuals_windows.count(use_target) )
   {
      saxs_iqq_residuals_windows[use_target]->close();
   }
      
   clear_plot_saxs( true );
      
   // best is model with lowest_chi2_pos
   editor_msg( "dark blue", 
               QString("Best fit model: %1 %2 %3\n")
               .arg(model_names[lowest_chi2_pos])
               .arg(use_errors ? "nchi" : "RMSD" )
               .arg(chi2s[lowest_chi2_pos])
               );

   // plot 
   plotted = false;
   // replot previous plot
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      plot_one_iqq( qs[ i ], Is[ i ], I_errors[ i ], names[ i ] );
   }
   
   if ( is_nonzero_vector( nnls_errors ) )
   {
      plot_one_iqq(nnls_r, best_fit_target, nnls_errors, csv_filename + " Target");
   } else {
      plot_one_iqq(nnls_r, best_fit_target, csv_filename + " Target");
   }

   rescale_iqq_curve( qsl_plotted_iq_names[ qsl_plotted_iq_names.size() - 1 ], nnls_r, model );

   plot_one_iqq(nnls_r, model, csv_filename + " Best Fit Model " + model_names[lowest_chi2_pos]);

   if ( plotted )
   {
      set_eb();
      editor_msg( "black", "I(q) plot done\n");
      plotted = false;
   }
   
   // save as csv
   
   if ( !csv_filename.isEmpty() )
   {
      // cout << "save_to_csv\n";
      QString fname = 
         ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
         tagged_csv_filename + "_iqq.csv";
      
      if ( QFile::exists(fname) )
         // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
      }         
      FILE *of = us_fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).toLatin1().data(),
                 QString("Best fit NRMSD %1% Chi^2 %2 : %3")
                 .arg(use_errors ? "nchi" : "RMSD" )
                 .arg(chi2s[lowest_chi2_pos])
                 .arg(nnls_header_tag).toLatin1().data()
                 );
         // original models
         for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(chi2s[i]).toLatin1().data(),
                    "I(q)",
                    vector_double_to_csv(best_fit_models[model_names[i]]).toLatin1().data());
         }
         // target
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                 "I(q)",
                 vector_double_to_csv(best_fit_target).toLatin1().data());

         // are there errors?
         if ( is_nonzero_vector( nnls_errors ) )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                    "I(q) sd",
                    vector_double_to_csv(nnls_errors).toLatin1().data());
         }
         
         // best fit model
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Best Fit Model " + model_names[lowest_chi2_pos]).toLatin1().data(),
                 "I(q)",
                 vector_double_to_csv(model).toLatin1().data());
         fclose(of);
         if ( plotted )
         {
            set_eb();
            editor_msg( "black", "I(q) plot done\n");
            plotted = false;
         }
         editor->append(us_tr("Created file: " + fname + "\n"));
      } else {
         editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n" ) );
      }
   }
}

// ------------------------------------ P(r) fits -----------------------------------------
void US_Hydrodyn_Saxs::calc_nnls_fit( QString title, QString csv_filename )
{
   // setup nnls run:
   // editor->append("setting up nnls run\n");
   // unify dimension of nnls_A vectors
   unsigned int max_pr_len = 0;

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_nnls", Qt::CaseInsensitive )) )
   {
      tagged_csv_filename += "_nnls";
   }

   // #define DEBUG_NNLS
#if defined(DEBUG_NNLS)
   nnls_A.clear( );
   nnls_A["one"].push_back(1e0);
   nnls_A["one"].push_back(1e0);
   nnls_A["one"].push_back(1e0);
   nnls_A["one"].push_back(1e0);
   
   nnls_A["two"].push_back(1e0);
   nnls_A["two"].push_back(2e0);
   nnls_A["two"].push_back(2e0);
   nnls_A["two"].push_back(2e0);

   nnls_A["three"].push_back(1e0);
   nnls_A["three"].push_back(1e0);
   nnls_A["three"].push_back(3e0);
   nnls_A["three"].push_back(3e0);
   
   nnls_B.clear( );
   // ideal conc .5 1 2
   nnls_B.push_back(3.5e0);
   nnls_B.push_back(4.5e0);
   nnls_B.push_back(8.5e0);
   nnls_B.push_back(8.5e0);
#endif

   int dmax_croplen = 0;
   
   if ( our_saxs_options->trunc_pr_dmax_target ) {
      vector < double > cropd_B = nnls_B;
      vector < double > cropd_r = nnls_r;
      vector < double > cropd_e = nnls_errors;

      crop_pr_tail( cropd_r, cropd_B, cropd_e );
      
      if ( nnls_B.size() > cropd_B.size() ) {
         editor_msg( "darkred", QString( us_tr( "Notice: cropping models to target Dmax %1\n" ) ).arg( cropd_r.back() ) );
         qDebug() << QString( us_tr( "Notice: cropping models to target Dmax %1" ) ).arg( cropd_r.back() );
         nnls_csv_footer << QString( us_tr( "\"Cropping models to target Dmax\",%1" ) ).arg( cropd_r.back() );

         dmax_croplen = (int) cropd_B.size();
         max_pr_len = dmax_croplen;
         nnls_B      = cropd_B;
         nnls_r      = cropd_r;
         nnls_errors = cropd_e;

         for ( auto it = nnls_A.begin();
               it != nnls_A.end();
               ++it ) {
            it->second.resize( max_pr_len );
         }
      }
   }

   for ( map < QString, vector < double > >::iterator it = nnls_A.begin();
        it != nnls_A.end();
        it++ )
   {
      if ( it->second.size() > max_pr_len ) 
      {
         // US_Vector::printvector2( QString( "curve length increase %1 %2\n" ).arg( it->first ).arg( it->second.size() ), nnls_r, it->second );
         max_pr_len = it->second.size();
      }
   }

   // editor->append(QString("a size %1\n").arg(nnls_A.size()));
   // editor->append(QString("b size %1\n").arg(max_pr_len));
   
   vector < double > use_A;

   vector < QString > model_names;

   for ( map < QString, vector < double > >::iterator it = nnls_A.begin();
        it != nnls_A.end();
        it++ )
   {
      it->second.resize(max_pr_len, 0);
      model_names.push_back(it->first);
      for ( unsigned int i = 0; i < max_pr_len; i++ )
      {
         use_A.push_back(it->second[i]);
#if defined(DEBUG_NNLS)
         cout << "use_A.push_back " << it->second[i] << endl;
#endif
      }
   }

   bool use_errors = is_nonzero_vector( nnls_errors );
   if ( !use_SDs_for_fitting_prr ) {
      use_errors = false;
   }
   unsigned int org_errors_size = nnls_errors.size();

   nnls_B.resize(max_pr_len, 0);

   if ( use_errors && org_errors_size < max_pr_len && !dmax_croplen )
   {
      editor_msg("darkred", QString(us_tr("NNLS propagating last error to extended zeros to match maximum model length. Errors size:%1 Maximum model length:%2\n")).arg(org_errors_size).arg(max_pr_len));
      nnls_csv_footer << QString(us_tr("\"NNLS propagating last error to extended zeros to match maximum model length. Errors size:%1 Maximum model length:%2\"")).arg(org_errors_size).arg(max_pr_len);
   }

   if ( use_errors ) {
      prop_pr_sd_tail( nnls_errors, max_pr_len );
   }

   editor_msg("dark blue", 
              use_errors ? 
              us_tr( "using standard deviations to compute NNLS\n" ) :
              us_tr( "NOT using standard deviations to compute NNLS\n" ) );

   nnls_csv_footer <<
      ( use_errors ? 
        us_tr( "\"using standard deviations to compute NNLS\"" ) :
        us_tr( "\"NOT using standard deviations to compute NNLS\"" ) )
      ;

   vector < double > use_B = nnls_B;
   vector < double > use_x(nnls_A.size());
   vector < double > nnls_wp(nnls_A.size());
   vector < double > nnls_zzp(use_B.size());
   vector < int > nnls_indexp(nnls_A.size());

   if ( use_errors ) {
      qDebug() << "pr nnls using errors";
      for ( unsigned int i = 0; i < nnls_errors.size(); i++ ) {
         use_B[ i ] /= nnls_errors[ i ];
      }
      for ( unsigned int i = 0; i < use_A.size(); i++ ) {
         use_A[ i ] /= nnls_errors[ i % max_pr_len ];
      }
   }         

   //   editor->append(QString("running nnls %1 %2\n").arg(nnls_A.size()).arg(use_B.size()));
   editor->append("Running NNLS\n");
   
   {
      // patch up nnls_r in case of truncated vector
      double nnls_delta = nnls_r[1] - nnls_r[0];
      while ( nnls_r.size() > 2 && nnls_r.size() < max_pr_len ) {
         nnls_r.push_back( nnls_r.back() + nnls_delta );
      }
   }                           

   // QTextStream( stdout )
   //    << QString(
   //               "nnls_r length %1\n"
   //               "max_pr_len    %2\n"
   //               "nnls_B length %3\n" 
   //               "use_B  length %4\n"
   //               )
   //    .arg( nnls_r.size() )
   //    .arg( max_pr_len )
   //    .arg( nnls_B.size() )
   //    .arg( use_B.size() )
   //    ;
   
   int result =
      nnls(
           (double *)&use_A[0],
           (int) max_pr_len,
           (int) max_pr_len,
           (int) nnls_A.size(),
           (double *)&use_B[0],
           (double *)&use_x[0],
           &nnls_rmsd,
           (double *)&nnls_wp[0],
           (double *)&nnls_zzp[0],
           (int *)&nnls_indexp[0]);
   
   if ( result != 0 )
   {
      editor->append("NNLS error!\n");
      nnls_csv_footer <<
         "\"NNLS error!\"";
   }
   
   editor->append(QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd));
   
   nnls_csv_footer <<
      QString("\"Residual Euclidian norm of NNLS fit\",%1").arg(nnls_rmsd);

   vector < double > rescaled_x = our_saxs_options->disable_nnls_scaling ? use_x : rescale(use_x);
   // list models & concs
   if ( our_saxs_options->disable_nnls_scaling ) {
      editor_msg( "darkred", "NNLS scaling disabled\n" );
   }

   QString use_color = "black";
   bool nnls_zero_list =
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "nnls_zero_list" ) ?
      ((US_Hydrodyn *)us_hydrodyn)->gparams[ "nnls_zero_list" ] == "true" : false;

   // sort names
   class sortable_model {
   public:
      QString      name;
      unsigned int number;

      bool operator < (const sortable_model & objIn) const {
         static QRegularExpression rx( "^(.*) Model: (\\d+)" );

         // does the objIn contain a model
         QRegularExpressionMatch matchIn = rx.match( objIn.name );
         if ( !matchIn.hasMatch() ) {
            return name < objIn.name;
         }

         // does the name contain a model
         QRegularExpressionMatch match = rx.match( name );
         if ( !match.hasMatch() ) {
            return name < objIn.name;
         }
         
         if ( match.captured(1) == matchIn.captured(1) ) {
            return match.captured(2).toUInt() < matchIn.captured(2).toUInt();
         }
         return name < objIn.name;
      }
   };
   
   list < sortable_model > sort_models;
   for ( unsigned int i = 0; i < use_x.size(); i++ ) {
      sortable_model m;
      m.name = model_names[i];
      m.number = i;
      sort_models.push_back( m );
   }

   sort_models.sort();
   
   // for ( auto it = sort_models.begin();
   //       it != sort_models.end();
   //       ++it ) {
   //    QTextStream(stdout) << QString( "model name %1 number %2\n" ).arg( it->name ).arg( it->number );
   // }

   // for ( unsigned int i = 0; i < use_x.size(); i++ ) {

   QRegularExpression rx( "^(.*) Model: (\\d+)" );
   QRegularExpression rx2( "^(.*)\\D(\\d+)\\.[^.]+$" );

   vector < int > contrib_to_plot;

   for ( auto it = sort_models.begin();
         it != sort_models.end();
         ++it ) {
      unsigned int i = it->number;
      if ( rescaled_x[i] == 0 )
      {
         if ( !nnls_zero_list ) {
            continue;
         }
         use_color = "gray";
      } else {
         if ( rescaled_x[i] < .1 )
         {
            use_color = "darkCyan";
         } else {
            if ( rescaled_x[i] < .2 )
            {
               use_color = "blue";
            } else {
               use_color = "darkBlue";
            }
         }
      }
      {
         QString model_name = model_names[i];

         // compute Rg, dmax
         QString rg_msg = "\"missing model for Rg computation\"";
         double dmax = nnls_r.back();
         if ( nnls_A.count( model_name ) ) {
            {
               double Rg;
               QString errormsg;
               if ( US_Saxs_Util::compute_rg_from_pr( nnls_r, nnls_A[model_name], Rg, errormsg ) ) {
                  rg_msg = QString( "%1" ).arg( Rg, 0, 'f', 2 );
               } else {
                  rg_msg = "\"" + errormsg + "\"";
               }
            }
            {
               int i = (int) nnls_r.size() - 1;
               if ( i > (int) nnls_A[model_name].size() - 1 ) {
                  i = (int) nnls_A[model_name].size() - 1;
               }
               while ( --i >= 0 && nnls_A[model_name][i] == 0 ) {
                  dmax = nnls_r[i];
               }
            }               
         }

         model_name.replace( "\"", "" );

         QRegularExpressionMatch match = rx.match( model_name );
         
         if ( match.hasMatch() ) {
            nnls_csv_data <<
               QString("\"%1\",%2,%3,%4,%5").arg(match.captured(1)).arg(match.captured(2)).arg(rescaled_x[i]).arg( rg_msg ).arg( dmax );
         } else {
            QRegularExpressionMatch match = rx2.match( model_name );
            if ( match.hasMatch() ) {
               nnls_csv_data <<
                  QString("\"%1\",%2,%3,%4,%5").arg(model_name).arg(match.captured(2)).arg(rescaled_x[i]).arg( rg_msg ).arg( dmax );
            } else {
               nnls_csv_data <<
                  QString("\"%1\",,%2,%3,%4").arg(model_name.replace( "\"", "" )).arg(rescaled_x[i]).arg( rg_msg ).arg( dmax );
            }
         }
      }      

      editor_msg( use_color, QString("%1 %2\n").arg(model_names[i]).arg( rescaled_x[i] ) );
      if ( nnls_plot_contrib && rescaled_x[i] > 0 ) {
         contrib_to_plot.push_back( i );
      }
   }
   
   nnls_csv_footer
      << QString( "\"Number of curves in the fit\",%1" ).arg( use_x.size() )
      ;

   // build model & residuals
   double model_mw = 0e0;
   vector < double > model(use_B.size());
   vector < double > residual(use_B.size());
   vector < double > difference(use_B.size());
   
   for ( unsigned int j = 0; j < rescaled_x.size(); j++ )
   {
      model_mw += rescaled_x[j] * nnls_mw[model_names[j]];
      // cout << QString("model source %1 contrib %2 mw %3\n").arg(j).arg(rescaled_x[j]).arg(nnls_mw[model_names[j]]);
   }
   // cout << QString("model mw %1\n").arg(model_mw);
   (*remember_mw)[csv_filename + " Model"] = model_mw;
   (*remember_mw_source)[csv_filename + " Model"] = "weighted average from NNLS csv models";
   
   for ( unsigned int i = 0; i < use_B.size(); i++ )
   {
      model[i] = 0e0;
      for ( unsigned int j = 0; j < use_x.size(); j++ )
      {
         model[i] += use_x[j] * nnls_A[model_names[j]][i];
      }
      residual[i] = fabs(model[i] - nnls_B[i]);
      difference[i] = nnls_B[i] - model[i];
   }
   
   editor_msg( "dark blue",
               QString( us_tr( "fitting range: %1 to %2 with %3 points\n" ) )
               .arg( nnls_r[ 0 ] )
               .arg( nnls_r.back() )
               .arg( nnls_r.size() ) );

   nnls_csv_footer <<
      QString( us_tr( "\"fitting range start, end, points:\",%1,%2,%3" ) )
      .arg( nnls_r[ 0 ] )
      .arg( nnls_r.back() )
      .arg( nnls_r.size() )
      ;

   // compute chi^2
   QString fit_msg = "";

   {
      vector < double > use_source_I = model;
      vector < double > use_I        = nnls_B;
      vector < double > use_I_error  = nnls_errors;

      double k;
      double chi2;

      US_Saxs_Util usu;

      bool use_errors_for_chi2 = use_errors || use_I_error.size() == use_I.size();

      if ( use_errors_for_chi2 ) {
         usu.scaling_fit( 
                         use_source_I, 
                         use_I, 
                         use_I_error,
                         k, 
                         chi2
                         );
         fit_msg = 
            QString("chi^2=%1 df=%2 nchi=%3 nchi^2=%4")
            .arg(chi2, 6)
            .arg(use_I.size() - 1 )
            .arg(sqrt( chi2 / ( use_I.size() - 1 ) ), 5 )
            .arg(chi2 / ( use_I.size() - 1 ), 5 )
            ;

         nnls_csv_footer
            << QString( "\"chi^2\",%1" ) .arg( chi2, 6 )
            << QString( "\"df\",%1" )    .arg(use_I.size() - 1 )
            << QString( "\"nchi^2\",%1" ).arg(chi2 / ( use_I.size() - 1 ), 5 )
            << QString( "\"nchi\",%1" )  .arg(sqrt( chi2 / ( use_I.size() - 1 ) ), 5 )
            ;

      } else {
         usu.scaling_fit( 
                         use_source_I, 
                         use_I, 
                         k, 
                         chi2
                         );
      }

      // if ( avg_std_dev_frac )
      // {
      //    fit_msg += QString( " r_sigma=%1 nchi*r_sigma=%2 " )
      //       .arg( avg_std_dev_frac ) 
      //       .arg( avg_std_dev_frac * sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 );

      //    nnls_csv_footer
      //       << QString( "\"r_sigma\",%1" )     .arg( avg_std_dev_frac ) 
      //       << QString( "\"nchi*r_sigma\",%1" ).arg( avg_std_dev_frac * sqrt( chi2 / ( use_I.size() - ( do_scale_linear_offset ? 2 : 1 ) ) ), 5 )
      //       ;
      // }
   }

   // compute p value
   
   // doesn't make sense for p(r)
   // US_Vector::printvector2( "iqq fit model, target", model, nnls_B );
   // {
   //    vector < double > I1 = model;
   //    vector < double > I2 = nnls_B;
   //    vector < double > q  = nnls_r;
   //    q.resize( nnls_B.size() );
   //    double p;
   //    QString emsg;

   //    if ( pvalue( q, I1, I2, p, emsg ) ) {
   //       QString p_status = "bad";
   //       if ( p >= 0.05 ) {
   //          p_status = "good";
   //       } else if ( p >= 0.01 ) {
   //          p_status = "fair";
   //       }
         
   //       editor_msg( "black", QString( " P-value=%1 (%2)\n" ).arg( p ).arg( p_status ) );
   //       nnls_csv_footer
   //          << QString( "\"P value\",%1,\"%2\"" ).arg( p ).arg( p_status );
   //          ;
   //    } else {
   //       qDebug() << emsg;
   //    }
   // }

   editor_msg( "black", fit_msg + "\n" );

   // plot 

   plot_one_pr(nnls_r, nnls_B, nnls_errors, csv_filename + " Target");
   compute_rg_to_progress( nnls_r, nnls_B, csv_filename + " Target");

   {
      vector < double > no_errors;
      plot_one_pr(nnls_r, model, no_errors, csv_filename + " Model");
   }

   compute_rg_to_progress( nnls_r, model, csv_filename + " Model");

   // plot_one_pr(nnls_r, residual, csv_filename + " Residual");
   
   for ( int i = 0; i < (int) contrib_to_plot.size(); ++i ) {
      vector < double > no_errors;
      plot_one_pr( nnls_r, nnls_A[model_names[contrib_to_plot[i]]], no_errors, model_names[contrib_to_plot[i]] );
      compute_rg_to_progress( nnls_r, nnls_A[model_names[contrib_to_plot[i]]], model_names[contrib_to_plot[i]] );
   }

   if ( plotted )
   {
      editor_msg( "black", "P(r) plot done\n");
      plotted = false;
   }

   if ( saxs_residuals_widget )
   {
      saxs_residuals_window->close();
   }
   
   {
      vector < double > use_nnls_r     = nnls_r;
      vector < double > use_model      = model;
      vector < double > use_nnls_B     = nnls_B;
      vector < double > use_difference = difference;
      // vector < double > use_residual   = residual;
      vector < double > use_error      = nnls_errors;
      vector < double > no_error;

      crop_pr_tail( use_nnls_r, use_nnls_B, use_error );
      int max_len = (int) use_nnls_r.size();
      use_nnls_r  = nnls_r;
      crop_pr_tail( use_nnls_r, use_model, no_error );
      max_len     = (int)use_nnls_r.size() > max_len ? (int)use_nnls_r.size() : max_len;
      use_nnls_r  = nnls_r;
      use_error   = nnls_errors;

      use_nnls_r    .resize( max_len );
      use_difference.resize( max_len );
      // use_residual  .resize( max_len );
      use_nnls_B    .resize( max_len );
      
      if ( use_error.size() ) {
         use_error.resize( max_len );
      }

      saxs_residuals_window = 
         new US_Hydrodyn_Saxs_Residuals(
                                        &saxs_residuals_widget,
                                        plot_pr->width(),
                                        us_tr("NNLS residuals & difference targeting:\n") + title,
                                        use_nnls_r,
                                        use_difference,
                                        // use_residual,
                                        use_nnls_B,
                                        use_error,
                                        // true,
                                        // true,
                                        // true,
                                        // use_error.size(),
                                        pen_width
                                        );
      saxs_residuals_window->show();
   }
   
   // save as csv
   
   if ( !csv_filename.isEmpty() )
   {
      // cout << "save_to_csv\n";
      QString fname = 
         ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
         tagged_csv_filename + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
      
      if ( QFile::exists(fname) )
         // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
      }         
      FILE *of = us_fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).toLatin1().data(),
                 QString("NNLS fit residual %1 : %2").arg(nnls_rmsd).arg(nnls_header_tag).toLatin1().data());
         // original models
         for ( unsigned int i = 0; i < use_x.size(); i++ )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rescaled_x[i]).toLatin1().data(),
                    get_mw(model_names[i], false),
                    compute_pr_area(nnls_A[model_names[i]], nnls_r),
                    "P(r)",
                    vector_double_to_csv(nnls_A[model_names[i]]).toLatin1().data());
         }
         // target
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                 get_mw(model_names[0], false),
                 compute_pr_area(nnls_B, nnls_r),
                 "P(r)",
                 vector_double_to_csv(nnls_B).toLatin1().data());
         
         // best fit model
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Model",
                 get_mw(model_names[0], false),
                 compute_pr_area(model, nnls_r),
                 "P(r)",
                 vector_double_to_csv(model).toLatin1().data());
         
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Residual",
                 get_mw(model_names[0], false),
                 compute_pr_area(residual, nnls_r),
                 "P(r)",
                 vector_double_to_csv(residual).toLatin1().data());
         
         fclose(of);
         if ( plotted )
         {
            editor_msg( "black", "P(r) plot done\n");
            plotted = false;
         }
         editor->append(us_tr("Created file: " + fname + "\n"));
      } else {
         editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n" ) );
      }
   }
}

void US_Hydrodyn_Saxs::calc_best_fit( QString title, QString csv_filename )
{
   // setup best_fit run, idea:
   //    use US_Saxs_Util::liner_fit
   //    give each model a chi^2
   //    sort by chi^2

   unsigned int max_pr_len = 0;
   int dmax_croplen = 0;

   if ( our_saxs_options->trunc_pr_dmax_target ) {
      vector < double > cropd_B = nnls_B;
      vector < double > cropd_r = nnls_r;
      vector < double > cropd_e = nnls_errors;

      crop_pr_tail( cropd_r, cropd_B, cropd_e );
      
      if ( nnls_B.size() > cropd_B.size() ) {
         editor_msg( "darkred", QString( us_tr( "Notice: cropping models to target Dmax %1\n" ) ).arg( cropd_r.back() ) );
         qDebug() << QString( us_tr( "Notice: cropping models to target Dmax %1" ) ).arg( cropd_r.back() );
         nnls_csv_footer << QString( us_tr( "\"Cropping models to target Dmax\",%1" ) ).arg( cropd_r.back() );

         dmax_croplen = (int) cropd_B.size();
         max_pr_len = dmax_croplen;
         nnls_B      = cropd_B;
         nnls_r      = cropd_r;
         nnls_errors = cropd_e;

         for ( auto it = nnls_A.begin();
               it != nnls_A.end();
               ++it ) {
            it->second.resize( max_pr_len );
         }
      }
   }

   map < QString, vector < double > > best_fit_models        = nnls_A;
   vector < double >                  best_fit_target        = nnls_B;
   vector < double >                  best_fit_target_errors = nnls_errors;

   // editor->append("setting up best fit run\n");
   // unify dimension of best_fit_models vectors

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_best_fit", Qt::CaseInsensitive )) )
   {
      tagged_csv_filename += "_best_fit";
   }

   for ( map < QString, vector < double > >::iterator it = best_fit_models.begin();
        it != best_fit_models.end();
        it++ )
   {
      if ( it->second.size() > max_pr_len ) 
      {
         max_pr_len = it->second.size();
      }
   }

   bool use_errors = is_nonzero_vector( nnls_errors );

   if ( !use_SDs_for_fitting_prr ) {
      use_errors = false;
   }

   vector < QString > model_names;

   for ( map < QString, vector < double > >::iterator it = best_fit_models.begin();
        it != best_fit_models.end();
        it++ )
   {
      // make sure all models are the same length

      unsigned int org_size = it->second.size();
      it->second.resize(max_pr_len);
      for ( unsigned int i = org_size; i < max_pr_len; i++ )
      {
         it->second[i] = 0e0;
      }
      model_names.push_back(it->first);
   }

   best_fit_target.resize(max_pr_len, 0);

   unsigned int org_errors_size = nnls_errors.size();

   if ( use_errors && org_errors_size < max_pr_len )
   {
      editor_msg("darkred", QString(us_tr("Best propagating last error to extended zeros to match maximum model length. Errors size:%1 Maximum model length:%2\n")).arg(org_errors_size).arg(max_pr_len));
      nnls_csv_footer << QString(us_tr("\"Best fit propagating last error to extended zeros to match maximum model length. Errors size:%1 Maximum model length:%2\"")).arg(org_errors_size).arg(max_pr_len);
   }
   if ( use_errors ) {
      prop_pr_sd_tail( best_fit_target_errors, max_pr_len );
   }

   vector < double > a(best_fit_models.size());
   vector < double > b(best_fit_models.size());
   vector < double > siga(best_fit_models.size());
   vector < double > sigb(best_fit_models.size());
   vector < unsigned int > df(best_fit_models.size());
   vector < double > chi2(best_fit_models.size());
   vector < double > prob(best_fit_models.size());

   vector < double > model(best_fit_models.size());
   vector < double > rmsds(best_fit_models.size());
   double lowest_rmsd     = 9e99;
   int    lowest_rmsd_pos = 0;

   //   editor->append(QString("running best fit %1 %2\n").arg(best_fit_models.size()).arg(best_fit_target.size()));
   editor->append("Running best fit\n");

   if ( use_errors ) {
      editor_msg( "black", "Using target curve errors for best fit\n" );
   } else {
      editor_msg( "black", "Not using target curve errors for best fit\n" );
   }

   US_Saxs_Util usu;

   double lowest_chi2     = 9e99;
   // int    lowest_chi2_pos = 0;

   vector < double > use_target = best_fit_target;
   if ( use_errors ) {
      for ( int i = 0; i < (int) use_target.size(); ++i ) {
         use_target[i] /= best_fit_target_errors[i];
      }
   }

   for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
   {
      vector < double > use_model  = best_fit_models[model_names[i]];
      
      // US_Vector::printvector2( QString( "best fit model %1" ).arg( model_names[i] ), nnls_r, use_model );

      if ( use_errors ) {
         for ( int j = 0; j < (int) use_model.size(); ++j ) {
            use_model[j] /= best_fit_target_errors[j];
         }
      }
      
#if defined( OLDSTYLE_PR_LINEAR_BEST_FIT )
      usu.linear_fit(
                     use_model,
                     use_target,
                     a[i],
                     b[i],
                     siga[i],
                     sigb[i],
                     chi2[i]
                     );
      // rescale model to target, compute rmsd
      vector < double > tmp_model(best_fit_target.size());
      for ( unsigned int j = 0; j < best_fit_target.size(); j++ )
      {
         tmp_model[j] = a[i] + b[i] * best_fit_models[model_names[i]][j];
      }
      // US_Vector::printvector3( QString( "best fit model %1, nnls_r, use_model, tmp_model(scaled)\n" ).arg( model_names[i] ), nnls_r, use_model, tmp_model );
      rmsds[i] = US_Saxs_Util::calc_nrmsd( tmp_model, best_fit_target );
      if ( !US_Saxs_Util::calc_chisq2( best_fit_models[model_names[i]],
                                       best_fit_target, 
                                       df[i],
                                       chi2[i],
                                       prob[i] ) )
      {
         editor_msg( "red", us_tr("Internal error computing chi2" ) );
      }

      if ( !i )
      {
         lowest_chi2 = chi2[i];
         // lowest_chi2_pos = 0;
         lowest_rmsd = rmsds[i];
         lowest_rmsd_pos = 0;
         model = tmp_model;
      } else {
         if ( lowest_chi2 > chi2[i] )
         {
            lowest_chi2 = chi2[i];
            // lowest_chi2_pos = i;
         }
         if ( lowest_rmsd > rmsds[i] )
         {
            lowest_rmsd = rmsds[i];
            lowest_rmsd_pos = i;
            model = tmp_model;
         }
      }
      
      editor->append(QString("%1 NRMSD %2% Chi^2 %3 nu %4\n")
                     .arg(model_names[i])
                     .arg(rmsds[i])
                     .arg(chi2[i])
                     .arg(df[i])
                     // .arg(prob[i])
                     );
#else // SCALING FIT INSTEAD
      usu.scaling_fit(
                     use_model,
                     use_target,
                     a[i],
                     chi2[i]
                     );
      // rescale model to target, compute rmsd
      vector < double > tmp_model(best_fit_target.size());
      for ( unsigned int j = 0; j < best_fit_target.size(); j++ )
      {
         tmp_model[j] = a[i] * best_fit_models[model_names[i]][j];
      }
      // US_Vector::printvector3( QString( "best fit model %1, nnls_r, use_model, tmp_model(scaled)\n" ).arg( model_names[i] ), nnls_r, use_model, tmp_model );
      rmsds[i] = US_Saxs_Util::calc_nrmsd( tmp_model, best_fit_target );
      if ( !US_Saxs_Util::calc_chisq2( best_fit_models[model_names[i]],
                                       best_fit_target, 
                                       df[i],
                                       chi2[i],
                                       prob[i] ) )
      {
         editor_msg( "red", us_tr("Internal error computing chi2" ) );
      }

      if ( !i )
      {
         lowest_chi2 = chi2[i];
         // lowest_chi2_pos = 0;
         lowest_rmsd = rmsds[i];
         lowest_rmsd_pos = 0;
         model = tmp_model;
      } else {
         if ( lowest_chi2 > chi2[i] )
         {
            lowest_chi2 = chi2[i];
            // lowest_chi2_pos = i;
         }
         if ( lowest_rmsd > rmsds[i] )
         {
            lowest_rmsd = rmsds[i];
            lowest_rmsd_pos = i;
            model = tmp_model;
         }
      }
      
#endif
   }
      
   double model_mw = nnls_mw[model_names[lowest_rmsd_pos]];

   // best is model with lowest_rmsd_pos
   editor_msg( "darkBlue",
               QString("Best fit model: %1 NRMSD %2% Chi^2 %3 nu %4\n")
               .arg(model_names[lowest_rmsd_pos])
               .arg(rmsds[lowest_rmsd_pos])
               .arg(chi2[lowest_rmsd_pos])
               .arg(df[lowest_rmsd_pos])
               // .arg(prob[lowest_rmsd_pos])
               );

   vector < double > residual(best_fit_target.size());
   vector < double > difference(best_fit_target.size());
   
   for ( unsigned int i = 0; i < best_fit_target.size(); i++ )
   {
      residual[i] = fabs(model[i] - best_fit_target[i]);
      difference[i] = best_fit_target[i] - model[i];
   }

   // plot 
   (*remember_mw)[csv_filename + " Best Fit Model " + model_names[lowest_rmsd_pos]] = model_mw;
   (*remember_mw_source)[csv_filename + " Model " + model_names[lowest_rmsd_pos]] = "weight of best fit model";
   
   plot_one_pr(nnls_r, model, csv_filename + " Best Fit Model " + model_names[lowest_rmsd_pos]);
   compute_rg_to_progress( nnls_r, model, csv_filename + " Best Fit Model " + model_names[lowest_rmsd_pos] );
   
   plot_one_pr(nnls_r, best_fit_target, best_fit_target_errors, csv_filename + " Target" );
   compute_rg_to_progress( nnls_r, best_fit_target, csv_filename + " Target" );

   if ( plotted )
   {
      editor_msg( "black", "P(r) plot done\n");
      plotted = false;
   }
   

   if ( saxs_residuals_widget )
   {
      saxs_residuals_window->close();
   }
   
   {
      vector < double > use_nnls_r          = nnls_r;
      vector < double > use_model           = model;
      vector < double > use_best_fit_target = best_fit_target;
      vector < double > use_difference      = difference;
      // vector < double > use_residual        = residual;
      vector < double > use_error           = best_fit_target_errors;
      vector < double > no_error;

      crop_pr_tail( use_nnls_r, use_best_fit_target, no_error );
      int max_len = (int) use_nnls_r.size();
      use_nnls_r  = nnls_r;

      crop_pr_tail( use_nnls_r, use_model, no_error );
      max_len     = (int)use_nnls_r.size() > max_len ? (int)use_nnls_r.size() : max_len;
      use_nnls_r  = nnls_r;
      use_error   = best_fit_target_errors;

      use_nnls_r         .resize( max_len );
      use_difference     .resize( max_len );
      // use_residual       .resize( max_len );
      use_best_fit_target.resize( max_len );

      if ( use_error.size() ) {
         use_error.resize( max_len );
      }

      saxs_residuals_window = 
         new US_Hydrodyn_Saxs_Residuals(
                                        &saxs_residuals_widget,
                                        plot_pr->width(),
                                        us_tr("Best fit residuals & difference targeting:\n") + title,
                                        use_nnls_r,
                                        use_difference,
                                        // use_residual,
                                        use_best_fit_target,
                                        use_error,
                                        // true,
                                        // true,
                                        // true,
                                        // use_error.size(),
                                        pen_width
                                        );
      saxs_residuals_window->show();
   }
   
   // save as csv
   
   if ( !csv_filename.isEmpty() )
   {
      // cout << "save_to_csv\n";
      QString fname = 
         ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
         tagged_csv_filename + "_sprr_" + ((US_Hydrodyn *)us_hydrodyn)->saxs_sans_ext() + ".csv";
      
      if ( QFile::exists(fname) )
         // && !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
      {
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( fname, 0, this );
      }         
      FILE *of = us_fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).toLatin1().data(),
                 QString("Best fit NRMSD %1% Chi^2 %2 : %3")
                 .arg(rmsds[lowest_rmsd_pos])
                 .arg(chi2[lowest_rmsd_pos])
                 .arg(nnls_header_tag).toLatin1().data()
                 );
         // original models
         for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rmsds[i]).toLatin1().data(),
                    get_mw(model_names[i], false),
                    compute_pr_area(best_fit_models[model_names[i]], nnls_r),
                    "P(r)",
                    vector_double_to_csv(best_fit_models[model_names[i]]).toLatin1().data());
         }
         // target
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).toLatin1().data(),
                 model_mw,
                 compute_pr_area(best_fit_target, nnls_r),
                 "P(r)",
                 vector_double_to_csv(best_fit_target).toLatin1().data());
         
         // best fit model
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Best Fit Model " + model_names[lowest_rmsd_pos]).toLatin1().data(),
                 model_mw,
                 compute_pr_area(model, nnls_r),
                 "P(r)",
                 vector_double_to_csv(model).toLatin1().data());
         
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Residual",
                 model_mw,
                 compute_pr_area(residual, nnls_r),
                 "P(r)",
                 vector_double_to_csv(residual).toLatin1().data());
         
         fclose(of);
         if ( plotted )
         {
            editor_msg( "black", "P(r) plot done\n");
            plotted = false;
         }
         editor->append(us_tr("Created file: " + fname + "\n"));
      } else {
         editor_msg( "red", us_tr("ERROR creating file: " + fname + "\n" ) );
      }
   }
}

bool US_Hydrodyn_Saxs::pvalue( const vector < double > &q, vector < double > &I, vector < double > &G, double &P, QString &errormsg ) {
   errormsg = "";

   vector < vector < double > > Icm( 2 );
   vector < vector < double > > rkl;
   int                          N;
   int                          S;
   int                          C;

   Icm[ 0 ] = I;
   Icm[ 1 ] = G;

   if ( !((US_Hydrodyn *)us_hydrodyn)->saxs_util->cormap( q, Icm, rkl, N, S, C, P ) ) {
      errormsg = ((US_Hydrodyn *)us_hydrodyn)->saxs_util->errormsg;
      return false;
   }

   return true;
}

bool US_Hydrodyn_Saxs::compute_rg_to_progress(
                                              const vector < double >  & r
                                              ,const vector < double > & pr
                                              ,const QString &           filename
                                              ) {
   // QTextStream(stdout)
   //    << QString( "compute_rg_to_progress() r length %1 pr length %2 for file %3\n" ).arg( r.size() ).arg( pr.size() ).arg( filename )
   //    ;

   vector < double > use_r = r;
   if ( pr.size() < r.size() ) {
      use_r.resize( pr.size() );
      qDebug() << QString( "resized vectors! compute_rg_to_progress() r length %1 pr length %2 for file %3\n" ).arg( r.size() ).arg( pr.size() ).arg( filename );
   }

   double Rg;
   if ( US_Saxs_Util::compute_rg_from_pr( use_r, pr, Rg, errormsg ) ) {
      editor_msg(
                 "black"
                 ,QString( us_tr( "Rg computed from p(r) for %1 = %2 [A]\n" ) )
                 .arg( filename )
                 .arg( Rg, 0, 'f', 2 )
                 );
      return true;
   } else {
      editor_msg(
                 "red"
                 ,QString( us_tr( "Error computing Rg from p(r) for %1 = %2\n" ) )
                 .arg( filename )
                 .arg( errormsg )
                 );
      US_Vector::printvector2( "error'd r pr", r, pr );
      return false;
   }
}


bool US_Hydrodyn_Saxs::log_rebin(
                                 int intervals
                                 ,const vector <double> & q
                                 ,const vector <double> & I
                                 ,vector <double> & rebin_q
                                 ,vector <double> & rebin_I
                                 ,QString & errors
                                 ) {
   vector < double > e;
   vector < double > rebin_e;

   return log_rebin( intervals, q, I, e, rebin_q, rebin_I, rebin_e, errors );
}
   

bool US_Hydrodyn_Saxs::log_rebin(
                                 int intervals
                                 ,const vector <double> & q
                                 ,const vector <double> & I
                                 ,const vector <double> & e
                                 ,vector <double> & rebin_q
                                 ,vector <double> & rebin_I
                                 ,vector <double> & rebin_e
                                 ,QString & errors
                                 ) {
   errors = "";

   errors = "not yet implemented";

   if ( !q.size() ) {
      errors = "log_rebin(): empty q vector";
      return false;
   }
   
   if ( q.size() != I.size() ) {
      errors = "log_rebin(): q I vector size mismatch";
      return false;
   }

   vector < double > use_e = e;
   
   if ( use_e.size() < q.size() ) {
      use_e.resize( q.size(), 0 );
   }

   vector < double > bins;
   double start = q[0];
   double end   = q.back();
   
   // setup bins
   {
      int start_interval_val = 1;
      int end_interval_val   = intervals + 1;
      double min_log         = log2( start );
      double max_log         = log2( end );
      double scale           = (max_log - min_log) / (end_interval_val - start_interval_val);
   
      for ( int i = 1; i <= intervals + 1; ++i )  {
         bins.push_back( pow(2, min_log + scale * ( i - start_interval_val ) ) );
      }
   }

   // rebin data & return

   rebin_q.clear();
   rebin_I.clear();
   rebin_e.clear();

   int bin = 0;

   double qsum  = 0;
   double Isum  = 0;
   double esum2 = 0;
   int    count = 0;
   
   for ( int i = 0; i < (int) q.size() && bin < (int) bins.size(); ++i ) {
      if ( q[i] > bins[ bin ] ) {
         // add any points previously collected 
         if ( count ) {
            rebin_q.push_back( qsum / count );
            rebin_I.push_back( Isum / count );
            rebin_e.push_back( sqrt( esum2 ) / count );
            qsum  = 0;
            Isum  = 0;
            esum2 = 0;
            count = 0;
         }
         ++bin;
      }
      qsum += q[i];
      Isum += I[i];
      esum2 += use_e[i] * use_e[i];
      ++count;
   }

   if ( count ) {
      rebin_q.push_back( qsum / count );
      rebin_I.push_back( Isum / count );
      rebin_e.push_back( sqrt( esum2 ) / count );
   }      

   return true;
}

