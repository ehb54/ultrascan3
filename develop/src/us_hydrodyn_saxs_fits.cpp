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
#  define isnan _isnan
#endif

void US_Hydrodyn_Saxs::calc_iqq_nnls_fit( QString /* title */, QString csv_filename )
{
   // setup nnls run:
   // editor->append("setting up nnls run\n");
   // unify dimension of nnls_A vectors
   unsigned int max_iqq_len = 0;

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_nnls", false)) )
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
         editor_msg("red", QString(tr("NNLS failed, length mismatch %1 %2\n")).arg( it->second.size()).arg(max_iqq_len));
         return;
      }
   }

   cout << "max_iqq_len " << max_iqq_len << endl;

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
      editor_msg("red", QString(tr("NNLS failed, target length mismatch %1 %2\n")).arg(org_size).arg(max_iqq_len));
      return;
   }

   bool use_errors = is_nonzero_vector( nnls_errors );
   if ( our_saxs_options->iqq_scale_nnls )
   {
      use_errors = false;
   }
   unsigned int org_errors_size = nnls_errors.size();
   if ( use_errors &&  org_errors_size != max_iqq_len )
   {
      editor_msg("red", QString(tr("NNLS failed, target length mismatch %1 %2\n")).arg(org_errors_size).arg(max_iqq_len));
      return;
   }

   editor_msg("dark blue", 
              use_errors ? 
              tr( "using standard deviations to compute NNLS\n" ) :
              tr( "NOT using standard deviations to compute NNLS\n" ) );

   vector < double > use_B = nnls_B;
   vector < double > use_q = nnls_q;
   vector < double > use_x(nnls_A.size());
   vector < double > nnls_wp(nnls_A.size());
   vector < double > nnls_zzp(use_B.size());
   vector < int > nnls_indexp(nnls_A.size());

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
      } else {
         // compute log10 on A & B
         editor_msg( "blue",
                     "Notice: Log fitting\n" );
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

   if ( our_saxs_options->iqq_scaled_fitting )
   {
      // first check for non-zero
      editor_msg( "blue",
                  "Notice: q^2*I Scaled fitting\n" );
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
   }
   
   editor->append(QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd));
   
   vector < double > rescaled_x = rescale(use_x);
   // list models & concs
   
   QColor save_color = editor->color();
   for ( unsigned int i = 0; i < use_x.size(); i++ )
   {
      if ( rescaled_x[i] == 0 )
      {
         editor->setColor("gray");
      } else {
         if ( rescaled_x[i] < .1 )
         {
            editor->setColor("darkCyan");
         } else {
            if ( rescaled_x[i] < .2 )
            {
               editor->setColor("blue");
            } else {
               editor->setColor("darkBlue");
            }
         }
      }
      editor->append(QString("%1 %2\n").arg(model_names[i]).arg(rescaled_x[i]));
   }
   editor->setColor(save_color);
   
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

   rescale_iqq_curve( qsl_plotted_iq_names[ qsl_plotted_iq_names.size() - 1 ], nnls_r, model );
   plot_one_iqq(nnls_r, model, csv_filename + " Model");
   
   if ( plotted )
   {
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      editor->append("I(q) plot done\n");
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
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
      }         
      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).ascii(),
                 QString("NNLS fit residual %1 : %2").arg(nnls_rmsd).arg(nnls_header_tag).ascii());
         // original models
         for ( unsigned int i = 0; i < use_x.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rescaled_x[i]).ascii(),
                    "I(q)",
                    vector_double_to_csv(nnls_A[model_names[i]]).ascii());
         }
         // target
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).ascii(),
                 "I(q)",
                 vector_double_to_csv(nnls_B).ascii());
         
         // are there errors?
         if ( is_nonzero_vector( nnls_errors ) )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("Target %1").arg(nnls_B_name).ascii(),
                    "I(q) sd",
                    vector_double_to_csv(nnls_errors).ascii());
         }

         // best fit model
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 "Model",
                 "I(q)",
                 vector_double_to_csv(model).ascii());
         
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 "Residual",
                 "I(q)",
                 vector_double_to_csv(residual).ascii());
         
         fclose(of);
         if ( plotted )
         {
            editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
            editor->append("I(q) plot done\n");
            plotted = false;
         }
         editor->append(tr("Created file: " + fname + "\n"));
      } else {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(tr("ERROR creating file: " + fname + "\n"));
         editor->setColor(save_color);
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
        !tagged_csv_filename.contains(QRegExp("_best_fit", false)) )
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
         editor_msg("red", QString(tr("Best fit failed, length mismatch %1 %2\n")).arg( it->second.size()).arg(max_iqq_len));
         return;
      }
   }

   bool use_errors = is_nonzero_vector( nnls_errors );
   if ( our_saxs_options->iqq_scale_nnls )
   {
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
      editor_msg("red", QString(tr("Best fit failed, target length mismatch %1 %2\n")).arg(org_size).arg(max_iqq_len));
      return;
   }

   vector < double > b(best_fit_models.size());
   vector < double > chi2s(best_fit_models.size());
   vector < double > model;

   double lowest_chi2     = 9e99;
   int    lowest_chi2_pos = 0;

   //   editor->append(QString("running best fit %1 %2\n").arg(best_fit_models.size()).arg(best_fit_target.size()));
   editor->append("Running best fit\n");
   clear_plot_saxs( true );

   if ( is_nonzero_vector( nnls_errors ) )
   {
      plot_one_iqq(nnls_r, nnls_B, nnls_errors, csv_filename + " Target");
   } else {
      plot_one_iqq(nnls_r, nnls_B, csv_filename + " Target");
   }
   QString use_target = qsl_plotted_iq_names[ qsl_plotted_iq_names.size() - 1 ];

   for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
   {
      editor->append(model_names[i] + " :");
      rescale_iqq_curve( use_target, nnls_r, best_fit_models[model_names[i]] );
      chi2s[i] = last_rescaling_chi2;
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

   // clear residuals window

   if ( saxs_iqq_residuals_windows.count(use_target) )
   {
      saxs_iqq_residuals_windows[use_target]->close();
   }
      
   clear_plot_saxs( true );
      
   // best is model with lowest_chi2_pos
   QColor save_color = editor->color();
   editor->setColor("darkBlue");
   editor->append(QString("Best fit model: %1 %2 %3\n")
                  .arg(model_names[lowest_chi2_pos])
                  .arg(use_errors ? "nchi" : "RMSD" )
                  .arg(chi2s[lowest_chi2_pos])
                  );
   editor->setColor(save_color);

   // plot 
   
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
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      editor->append("I(q) plot done\n");
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
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
      }         
      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"Type; q:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).ascii(),
                 QString("Best fit NRMSD %1% Chi^2 %2 : %3")
                 .arg(use_errors ? "nchi" : "RMSD" )
                 .arg(chi2s[lowest_chi2_pos])
                 .arg(nnls_header_tag).ascii()
                 );
         // original models
         for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(chi2s[i]).ascii(),
                    "I(q)",
                    vector_double_to_csv(best_fit_models[model_names[i]]).ascii());
         }
         // target
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).ascii(),
                 "I(q)",
                 vector_double_to_csv(best_fit_target).ascii());

         // are there errors?
         if ( is_nonzero_vector( nnls_errors ) )
         {
            fprintf(of, "\"%s\",\"%s\",%s\n", 
                    QString("Target %1").arg(nnls_B_name).ascii(),
                    "I(q) sd",
                    vector_double_to_csv(nnls_errors).ascii());
         }
         
         // best fit model
         fprintf(of, "\"%s\",\"%s\",%s\n", 
                 QString("Best Fit Model " + model_names[lowest_chi2_pos]).ascii(),
                 "I(q)",
                 vector_double_to_csv(model).ascii());
         fclose(of);
         if ( plotted )
         {
            editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
            editor->append("I(q) plot done\n");
            plotted = false;
         }
         editor->append(tr("Created file: " + fname + "\n"));
      } else {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(tr("ERROR creating file: " + fname + "\n"));
         editor->setColor(save_color);
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
        !tagged_csv_filename.contains(QRegExp("_nnls", false)) )
   {
      tagged_csv_filename += "_nnls";
   }

   // #define DEBUG_NNLS
#if defined(DEBUG_NNLS)
   nnls_A.clear();
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
   
   nnls_B.clear();
   // ideal conc .5 1 2
   nnls_B.push_back(3.5e0);
   nnls_B.push_back(4.5e0);
   nnls_B.push_back(8.5e0);
   nnls_B.push_back(8.5e0);
#endif

   for ( map < QString, vector < double > >::iterator it = nnls_A.begin();
        it != nnls_A.end();
        it++ )
   {
      if ( it->second.size() > max_pr_len ) 
      {
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
      unsigned int org_size = it->second.size();
      it->second.resize(max_pr_len);
      for ( unsigned int i = org_size; i < max_pr_len; i++ )
      {
         it->second[i] = 0e0;
      }
      model_names.push_back(it->first);
      for ( unsigned int i = 0; i < max_pr_len; i++ )
      {
         use_A.push_back(it->second[i]);
#if defined(DEBUG_NNLS)
         cout << "use_A.push_back " << it->second[i] << endl;
#endif
      }
   }

   unsigned int org_size = nnls_B.size();
   nnls_B.resize(max_pr_len);
   for ( unsigned int i = org_size; i < max_pr_len; i++ )
   {
      nnls_B[i] = 0e0;
   }
   vector < double > use_B = nnls_B;
   vector < double > use_x(nnls_A.size());
   vector < double > nnls_wp(nnls_A.size());
   vector < double > nnls_zzp(use_B.size());
   vector < int > nnls_indexp(nnls_A.size());

   //   editor->append(QString("running nnls %1 %2\n").arg(nnls_A.size()).arg(use_B.size()));
   editor->append("Running NNLS\n");
#if defined(DEBUG_NNLS)
   cout << "use A size " << use_A.size() << endl;
   int a_size = nnls_A.size();
   int b_size = nnls_B.size();
   cout << "matrix: \n";
   for ( int i = 0; i < a_size; i++ )
   {
      for ( int j = 0; j < b_size; j++ )
      {
         cout << use_A[i * b_size + j] << ",";
      }
      cout << endl;
   }
   cout << "b: \n";
   for ( int j = 0; j < b_size; j++ )
   {
      cout << use_B[j] << ",";
   }
   cout << endl;
#endif
   
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
   }
   
   editor->append(QString("Residual Euclidian norm of NNLS fit %1\n").arg(nnls_rmsd));
   
   vector < double > rescaled_x = rescale(use_x);
   // list models & concs
   
   QColor save_color = editor->color();
   for ( unsigned int i = 0; i < use_x.size(); i++ )
   {
      if ( rescaled_x[i] == 0 )
      {
         editor->setColor("gray");
      } else {
         if ( rescaled_x[i] < .1 )
         {
            editor->setColor("darkCyan");
         } else {
            if ( rescaled_x[i] < .2 )
            {
               editor->setColor("blue");
            } else {
               editor->setColor("darkBlue");
            }
         }
      }
      editor->append(QString("%1 %2\n").arg(model_names[i]).arg(rescaled_x[i]));
   }
   editor->setColor(save_color);
   
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
   
   // plot 
   
   plot_one_pr(nnls_r, model, csv_filename + " Model");
   
   // plot_one_pr(nnls_r, residual, csv_filename + " Residual");
   
   plot_one_pr(nnls_r, nnls_B, csv_filename + " Target");
   
   if ( plotted )
   {
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      editor->append("P(r) plot done\n");
      plotted = false;
   }

   if ( saxs_residuals_widget )
   {
      saxs_residuals_window->close();
   }
   
   saxs_residuals_window = 
      new US_Hydrodyn_Saxs_Residuals(
                                     &saxs_residuals_widget,
                                     plot_pr->width(),
                                     tr("NNLS residuals & difference targeting:\n") + title,
                                     nnls_r,
                                     difference,
                                     residual,
                                     nnls_B,
                                     true,
                                     true,
                                     true
                                     );
   saxs_residuals_window->show();
   
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
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
      }         
      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).ascii(),
                 QString("NNLS fit residual %1 : %2").arg(nnls_rmsd).arg(nnls_header_tag).ascii());
         // original models
         for ( unsigned int i = 0; i < use_x.size(); i++ )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rescaled_x[i]).ascii(),
                    get_mw(model_names[i], false),
                    compute_pr_area(nnls_A[model_names[i]], nnls_r),
                    "P(r)",
                    vector_double_to_csv(nnls_A[model_names[i]]).ascii());
         }
         // target
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).ascii(),
                 get_mw(model_names[0], false),
                 compute_pr_area(nnls_B, nnls_r),
                 "P(r)",
                 vector_double_to_csv(nnls_B).ascii());
         
         // best fit model
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Model",
                 get_mw(model_names[0], false),
                 compute_pr_area(model, nnls_r),
                 "P(r)",
                 vector_double_to_csv(model).ascii());
         
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Residual",
                 get_mw(model_names[0], false),
                 compute_pr_area(residual, nnls_r),
                 "P(r)",
                 vector_double_to_csv(residual).ascii());
         
         fclose(of);
         if ( plotted )
         {
            editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
            editor->append("P(r) plot done\n");
            plotted = false;
         }
         editor->append(tr("Created file: " + fname + "\n"));
      } else {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(tr("ERROR creating file: " + fname + "\n"));
         editor->setColor(save_color);
      }
   }
}

void US_Hydrodyn_Saxs::calc_best_fit( QString title, QString csv_filename )
{
   // setup best_fit run, idea:
   //    use US_Saxs_Util::liner_fit
   //    give each model a chi^2
   //    sort by chi^2

   map < QString, vector < double > > best_fit_models = nnls_A;
   vector < double >                  best_fit_target = nnls_B;

   // editor->append("setting up best fit run\n");
   // unify dimension of best_fit_models vectors

   unsigned int max_pr_len = 0;

   QString tagged_csv_filename = csv_filename;

   if ( !tagged_csv_filename.isEmpty() &&
        !tagged_csv_filename.contains(QRegExp("_best_fit", false)) )
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

   unsigned int org_size = best_fit_target.size();
   best_fit_target.resize(max_pr_len);
   for ( unsigned int i = org_size; i < max_pr_len; i++ )
   {
      best_fit_target[i] = 0e0;
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

   US_Saxs_Util usu;

   double lowest_chi2     = 9e99;
   int    lowest_chi2_pos = 0;

   for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
   {
      usu.linear_fit(
                     best_fit_models[model_names[i]],
                     best_fit_target, 
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
      rmsds[i] = US_Saxs_Util::calc_nrmsd( tmp_model, best_fit_target );
      if ( !US_Saxs_Util::calc_chisq2( best_fit_models[model_names[i]],
                                       best_fit_target, 
                                       df[i],
                                       chi2[i],
                                       prob[i] ) )
      {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(tr("Internal error computing chi2"));
         editor->setColor(save_color);
      }

      if ( !i )
      {
         lowest_chi2 = chi2[i];
         lowest_chi2_pos = 0;
         lowest_rmsd = rmsds[i];
         lowest_rmsd_pos = 0;
         model = tmp_model;
      } else {
         if ( lowest_chi2 > chi2[i] )
         {
            lowest_chi2 = chi2[i];
            lowest_chi2_pos = i;
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
   }
      
   double model_mw = nnls_mw[model_names[lowest_rmsd_pos]];

   // best is model with lowest_rmsd_pos
   QColor save_color = editor->color();
   editor->setColor("darkBlue");
   editor->append(QString("Best fit model: %1 NRMSD %2% Chi^2 %3 nu %4\n")
                  .arg(model_names[lowest_rmsd_pos])
                  .arg(rmsds[lowest_rmsd_pos])
                  .arg(chi2[lowest_rmsd_pos])
                  .arg(df[lowest_rmsd_pos])
                  // .arg(prob[lowest_rmsd_pos])
                  );
   editor->setColor(save_color);

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
   
   plot_one_pr(nnls_r, best_fit_target, csv_filename + " Target");

   if ( plotted )
   {
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      editor->append("P(r) plot done\n");
      plotted = false;
   }
   

   if ( saxs_residuals_widget )
   {
      saxs_residuals_window->close();
   }
   
   saxs_residuals_window = 
      new US_Hydrodyn_Saxs_Residuals(
                                     &saxs_residuals_widget,
                                     plot_pr->width(),
                                     tr("Best fit residuals & difference targeting:\n") + title,
                                     nnls_r,
                                     difference,
                                     residual,
                                     best_fit_target,
                                     true,
                                     true,
                                     true
                                     );
   saxs_residuals_window->show();
   
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
         fname = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fname);
      }         
      FILE *of = fopen(fname, "wb");
      if ( of )
      {
         //  header: "name","type",r1,r2,...,rn, header info
         fprintf(of, "\"Name\",\"MW (Daltons)\",\"Area\",\"Type; r:\",%s,%s\n", 
                 vector_double_to_csv(nnls_r).ascii(),
                 QString("Best fit NRMSD %1% Chi^2 %2 : %3")
                 .arg(rmsds[lowest_rmsd_pos])
                 .arg(chi2[lowest_rmsd_pos])
                 .arg(nnls_header_tag).ascii()
                 );
         // original models
         for ( unsigned int i = 0; i < best_fit_models.size(); i++ )
         {
            fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                    QString("%1 %2").arg(model_names[i]).arg(rmsds[i]).ascii(),
                    get_mw(model_names[i], false),
                    compute_pr_area(best_fit_models[model_names[i]], nnls_r),
                    "P(r)",
                    vector_double_to_csv(best_fit_models[model_names[i]]).ascii());
         }
         // target
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Target %1").arg(nnls_B_name).ascii(),
                 model_mw,
                 compute_pr_area(best_fit_target, nnls_r),
                 "P(r)",
                 vector_double_to_csv(best_fit_target).ascii());
         
         // best fit model
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 QString("Best Fit Model " + model_names[lowest_rmsd_pos]).ascii(),
                 model_mw,
                 compute_pr_area(model, nnls_r),
                 "P(r)",
                 vector_double_to_csv(model).ascii());
         
         fprintf(of, "\"%s\",%.2f,%.2f,\"%s\",%s\n", 
                 "Residual",
                 model_mw,
                 compute_pr_area(residual, nnls_r),
                 "P(r)",
                 vector_double_to_csv(residual).ascii());
         
         fclose(of);
         if ( plotted )
         {
            editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
            editor->append("P(r) plot done\n");
            plotted = false;
         }
         editor->append(tr("Created file: " + fname + "\n"));
      } else {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(tr("ERROR creating file: " + fname + "\n"));
         editor->setColor(save_color);
      }
   }
}

