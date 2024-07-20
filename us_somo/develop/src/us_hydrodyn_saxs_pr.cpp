// #define DEBUG_MW2 // for testing "preset values on guinier_window()"
// #define DEBUG_MW

#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn.h"
#include <qregexp.h>

#define SLASH QDir::separator()
double US_Hydrodyn_Saxs::get_mw( QString filename, bool display_mw_msg, bool allow_none )
{
   // enter MW and PSV
   filename = QFileInfo(filename).fileName();
   float mw = 0.0;
   bool remember = true;
   bool use_partial = false;
   QString partial = filename;
   QString msg = QString(us_tr(" Enter values for total molecular weight: "));
   QString source = "";
   bool found = false;
   // QTextStream(stdout) << QString( "saxs::get_mw() remember_mw:\n" );
   // for ( auto it = remember_mw->begin();
   //       it != remember_mw->end();
   //       ++it ) {
   //    QTextStream(stdout) << it->first << " " << it->second << " source:";
   //    if ( (*remember_mw_source).count(it->first) ) {
   //       QTextStream(stdout) << (*remember_mw_source)[it->first];
   //    }
   //    QTextStream(stdout) << "\n";
   // }
   
   if ( (*remember_mw).count(filename) )
   {
      mw = (*remember_mw)[filename];
      if ( (*remember_mw_source).count(filename) )
      {
         source = (*remember_mw_source)[filename];
      }
      found = true;
   } else {
      if ( !(*match_remember_mw).empty() )
      {
         // puts("dammix_match_remember not empty");
         for (map < QString, float >::iterator it = (*match_remember_mw).begin();
              it != (*match_remember_mw).end();
              it++)
         {
            // printf("iterator first %s\n", it->first.toLatin1().data());
            // printf("iterator second %f\n", it->second);
            
            if ( filename.contains(it->first) )
            {
               mw = (*match_remember_mw)[it->first];
               source = QString(us_tr("file name matched \"%1\"")).arg(it->first);
               found = true;
               break;
            }
         }
      }
   }
   
   if ( found ) 
   {
      // cout <<  QString("mw: %1 %2 %3\n").arg(filename).arg(mw).arg(source);
      if ( display_mw_msg )
      {
         editor->append(QString("%1 Molecular weight %2 (%3)\n")
                        .arg(filename)
                        .arg(mw)
                        .arg(source)
                        );
      }
   } else {
      US_Hydrodyn_Saxs_Mw *smw = new US_Hydrodyn_Saxs_Mw(
                                                         msg,
                                                         &mw,
                                                         &last_used_mw,
                                                         &remember,
                                                         &use_partial,
                                                         &partial,
                                                         remember_mw,
                                                         remember_mw_source,
                                                         allow_none,
                                                         this
                                                         );
      US_Hydrodyn::fixWinButtons( smw );
      do {
         smw->exec();
         if ( allow_none && mw == -1e0 ) {
            break;
         }
      } while ( mw <= 0.0 );
      if ( mw > 0e0 ) {
         last_used_mw = mw;
      
         delete smw;
         this->isVisible() ? this->raise() : this->show();
      
         if ( remember ) 
         {
            (*remember_mw)[filename] = mw;
            (*remember_mw_source)[filename] = "manually entered value";
         }
         if ( use_partial ) 
         {
            (*match_remember_mw)[partial] = mw;
         }
      } else {
         return mw;
      }
   }
   //   printf( "%s is %g\n",  QString( "get mw filename %1" ).arg( filename ).toLatin1().data(), mw );
   return mw;
}

void US_Hydrodyn_Saxs::normalize_pr( vector < double > r, vector < double > *pr , double mw ) {
   qDebug() << "normalize_pr() old style call, DEPRECATE!";
   vector < double > pre(r.size(),0);
   return normalize_pr( r, pr, &pre, mw );
}

void US_Hydrodyn_Saxs::normalize_pr( vector < double > r, vector < double > *pr , vector < double > *pre, double mw ) {
   if ( !our_saxs_options->normalize_by_mw )
   {
      mw = 1e0;
   }
#if defined(NORMALIZE_OLD_WAY)
   // set distribution to a 1 peak
   double max = 0e0;
   if ( pr->size() )
   {
      max = (*pr)[0];
   }
   for ( unsigned int i = 1; i < pr->size(); i++ )
   {
      if ( (*pr)[i] > max )
      {
         max = (*pr)[i];
      }
   }
   if ( max > 0e0 ) {
      if ( pre->size() && pre->size() == pr->size() ) {
         for ( unsigned int i = 0; i < pr->size(); i++ ) {
            (*pr)[i]  /= max;
            (*pre)[i] /= max;
         }
      } else {
         for ( unsigned int i = 0; i < pr->size(); i++ ) {
            (*pr)[i] /= max;
         }
      }
   }
#else
   // integrate
   // assuming regular grid:
   double area = 0e0;
   if ( pr->size() > 1 )
   {
      double gridspacing = r[1] - r[0];
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         area += (*pr)[i] * gridspacing;
      }
      if ( area > 0e0 )
      {
         area /= mw;
         if ( pre->size() && pre->size() == pr->size() ) {
            for ( unsigned int i = 0; i < pr->size(); i++ ) {
               (*pr) [i] /= area;
               (*pre)[i] /= area;
            }
         } else {
            for ( unsigned int i = 0; i < pr->size(); i++ ) {
               (*pr) [i] /= area;
            }
         }
      }
      // cout << "normalize_pr area " << area << "\n" << flush;
      {
         double area = 0e0;
         for ( unsigned int i = 0; i < pr->size(); i++ )
         {
            area += (*pr)[i] * gridspacing;
         }
         // cout << "after normalize_pr area " << area << "\n" << flush;
      }
   }
   /* riemann sum?
   double area = 0e0;
   for ( unsigned int i = 1; i < pr->size(); i++ )
   {
      area += ( (*pr)[i-1] + (*pr)[i] ) / ( 2e0 * ( r[i] - r[i-1] ) );
   }
   if ( area > 0e0 )
   {

      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         (*pr )[i] *= pr->size() / area ;
         (*pre)[i] *= pr->size() / area ;
      }
   }
   // cout << "normalize_pr area " << area << "\n" << flush;
   {
      double area = 0e0;
      for ( unsigned int i = 1; i < pr->size(); i++ )
      {
         area += ( (*pr)[i-1] + (*pr)[i] ) / ( 2e0 * ( r[i] - r[i-1] ) );
      }
      // cout << "after normalize_pr area " << area << "\n" << flush;
   }
   */
#endif
}

void US_Hydrodyn_Saxs::check_pr_grid( vector < double > &r, vector < double > &pr )
{
   qDebug() << "check_pr_grid() old style call, DEPRECATE!";
   vector < double > pr_error;
   return check_pr_grid( r, pr, pr_error );
}

void US_Hydrodyn_Saxs::check_pr_grid( vector < double > &r, vector < double > &pr, vector < double > &pr_error ) {
   if ( r.size() < 3 )
   {
      return;
   }

   if ( r.size() != pr.size() )
   {
      editor_msg( "red" , 
                  QString( "Internal error: the r grid has %1 points, put the P(r) values have %2 points!" )
                  .arg( r.size() )
                  .arg( pr.size() ) );
      return;
   }

   double delta = r[ 1 ] - r[ 0 ];

   bool ok = true;
   for ( unsigned int i = 1; i < r.size() - 1; i++ )
   {
      if ( r[ i + 1 ] - r[ i ] != delta )
      {
         ok = false;
         break;
      }
   }
   if ( ok )
   {
      return;
   }

   editor_msg( "dark red" , "The P(r) grid does not have uniform spacing: interpolating to current bin size" );

   vector < double > new_r;
   double start_r = r[ 0 ];
   double end_r   = r[ r.size() - 1 ];

   for ( double this_r = start_r; 
         this_r <= end_r + our_saxs_options->bin_size; 
         this_r += our_saxs_options->bin_size )
   {
      new_r.push_back( this_r );
   }

   // fix r for duplicate values:

   vector < double > org_r;
   vector < double > org_pr;

   {
      map < double, double       > fixed_pr;
      map < double, unsigned int > fixed_pr_count;
      
      for ( unsigned int i = 0; i < r.size(); i++ )
      {
         fixed_pr      [ r[ i ] ] += pr[ i ];
         fixed_pr_count[ r[ i ] ] ++;
      }
      for ( map < double, double >::iterator it = fixed_pr.begin();
            it != fixed_pr.end();
            it++ )
      {
         
         org_r .push_back( it->first );
         org_pr.push_back( it->second / fixed_pr_count[ it->first ] );
      }
      r  = org_r;
      pr = org_pr;
   }
   
   vector < double > new_pr;
   if ( !interpolate( new_r, r, pr, new_pr ) ) {
      editor_msg( "red", us_tr( "Error attempting to interpolate" ) );
      return;
   }

   // US_Saxs_Util usu;
   // vector < double > y2;
   // usu.natural_spline( r, pr, y2 );

   // vector < double > new_pr( new_r.size() );

   // for ( unsigned int i = 0; i < new_r.size(); i++ )
   // {
   //    if ( !usu.apply_natural_spline( r, pr, y2, new_r[ i ], new_pr[ i ] ) )
   //    {
   //       editor_msg( "red", usu.errormsg );
   //       editor_msg( "red", us_tr( "Error attempting to interpolate" ) );
   //       return;
   //    }
   // }

   if ( pr_error.size() ) {
      if ( r.size() != pr_error.size() ) {
         editor_msg( "red", us_tr( "Error r pr_error length mismatch" ) );
         return;
      }
         
      vector < double > new_pr_error;
      if ( !interpolate( new_r, r, pr_error, new_pr_error ) ) {
         editor_msg( "red", us_tr( "Error attempting to interpolate" ) );
         return;
      }

      // vector < double > y2;
      // usu.natural_spline( r, pr_error, y2 );

      // vector < double > new_pr_error( new_r.size() );
      
      // for ( unsigned int i = 0; i < new_r.size(); i++ )
      // {
      //    if ( !usu.apply_natural_spline( r, pr_error, y2, new_r[ i ], new_pr_error[ i ] ) )
      //    {
      //       editor_msg( "red", usu.errormsg );
      //       editor_msg( "red", us_tr( "Error attempting to interpolate" ) );
      //       return;
      //    }
      // }
      pr_error = new_pr_error;
   }

   r = new_r;
   pr = new_pr;
   return;
}

bool US_Hydrodyn_Saxs::mw_from_I0( QString name, double I0_exp, double &MW, double &internal_contrast )
{
   double conc;
   double psv;
   double I0_std_exp;

   if ( name.contains( QRegularExpression( "_Istarq_" ) ) ) {
      MW = I0_exp;
      return true;
   }

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   if ( conc == 0e0 ) {
      errormsg = us_tr( "Error: Concentration is 0, can not compute MW" );
      MW = 0e0;
      return false;
   }

   if ( name.contains( QRegularExpression( "_Ihashq_" ) ) ) {
      MW = I0_exp / (conc * 1e-3);
      return true;
   }

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      // cout << "mw_from_I0: using standards\n";
      if ( I0_std_exp == 0e0 )
      {
         errormsg = us_tr( "Error: I0 standard experimental is 0, can not compute MW" );
         MW = 0e0;
         return false;
      }
      I0_exp_to_theo_mult = our_saxs_options->I0_theo / I0_std_exp;
   } else {
      // cout << "mw_from_I0: standards not used\n";
   }

   double I0_prot_theo = I0_exp * I0_exp_to_theo_mult;

   if ( our_saxs_options->nucleon_mass == 0e0 )
   {
      errormsg = us_tr( "Error: Mass of nucleon is 0, can not compute MW" );
      MW = 0e0;
      return false;
   }


   double guinier_electron_nucleon_ratio =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_electron_nucleon_ratio" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_electron_nucleon_ratio" ].toDouble() : 1.87e0;

   internal_contrast = 
      our_saxs_options->diffusion_len * 
      ( 1e0 / ( guinier_electron_nucleon_ratio * our_saxs_options->nucleon_mass ) - psv * ( 1e24 * our_saxs_options->water_e_density ) );

   MW = I0_prot_theo * AVOGADRO / ( conc * 1e-3 ) / ( internal_contrast * internal_contrast );

#if defined( DEBUG_MW )
   cout << QString( 
                   "I0_std_theo         %1\n"
                   "I0_std_exp          %2\n"
                   )
      .arg( our_saxs_options->I0_theo, 0, 'e', 8  )
      .arg( I0_std_exp, 0, 'e', 8  )
      ;
                   
   cout << QString( 
                   "I0_exp_to_theo_mult %1\n"
                   "diffusion len       %2\n"
                   "psv                 %3\n"
                   "water_e_density     %4\n"
                   "I0_prot_theo        %5\n"
                   "Avogadro            %6\n"
                   "conc                %7\n"
                   "internal contrast   %8\n"
                   "nucleon mass        %9\n"
                   )

      .arg( I0_exp_to_theo_mult, 0, 'e', 8  )
      .arg( our_saxs_options->diffusion_len, 0, 'e', 8  )
      .arg( psv, 0, 'e', 8  )
      .arg( our_saxs_options->water_e_density * 1e24, 0, 'e', 8  )
      .arg( I0_prot_theo, 0, 'e', 8  )
      .arg( AVOGADRO, 0, 'e', 8  )
      .arg( conc, 0, 'e', 8  )
      .arg( internal_contrast, 0, 'e', 8  )
      .arg( our_saxs_options->nucleon_mass, 0, 'e', 8  );
#endif

   return true;
}

bool US_Hydrodyn_Saxs::ml_from_qI0( QString name, double I0_exp, double &ML, double &internal_contrast )
{
   double conc;
   double psv;
   double I0_std_exp;

   if ( name.contains( QRegularExpression( "_Istarq_" ) ) ) {
      ML = I0_exp / M_PI;
      return true;
   }

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   if ( conc == 0e0 ){
      errormsg = us_tr( "Error: Concentration is 0, can not compute ML" );
      ML = 0e0;
      return false;
   }

   if ( name.contains( QRegularExpression( "_Ihashq_" ) ) ) {
      ML = I0_exp / (conc * 1e-3) / M_PI;
      return true;
   }

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      if ( I0_std_exp == 0e0 )
      {
         errormsg = us_tr( "Error: I0 standard experimental is 0, can not compute ML" );
         ML = 0e0;
         return false;
      }
      I0_exp_to_theo_mult = our_saxs_options->I0_theo / I0_std_exp;
   }

   double I0_prot_theo = I0_exp * I0_exp_to_theo_mult;

   if ( our_saxs_options->nucleon_mass == 0e0 )
   {
      errormsg = us_tr( "Error: Mass of nucleon is 0, can not compute ML" );
      ML = 0e0;
      return false;
   }


   double use_psv = our_saxs_options->use_cs_psv ? our_saxs_options->cs_psv : psv;

   double guinier_electron_nucleon_ratio =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_electron_nucleon_ratio" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_electron_nucleon_ratio" ].toDouble() : 1.87e0;

   internal_contrast = 
      our_saxs_options->diffusion_len * 
      ( 1e0 / ( guinier_electron_nucleon_ratio * our_saxs_options->nucleon_mass ) - use_psv * ( 1e24 * our_saxs_options->water_e_density ) );
   
   ML = I0_prot_theo * AVOGADRO / ( conc * 1e-3 ) / ( internal_contrast * internal_contrast ) / M_PI;

#if defined( DEBUG_MW )
   cout << QString( 
                   "I0_std_theo         %1\n"
                   "I0_std_exp          %2\n"
                   "I0                  %3\n"
                   "ML                  %4\n"
                   )
      .arg( our_saxs_options->I0_theo, 0, 'e', 8  )
      .arg( I0_std_exp, 0, 'e', 8  )
      .arg( I0_exp, 0, 'e', 8  )
      .arg( ML, 0, 'e', 8  )
      ;
                   
   cout << QString( 
                   "I0_exp_to_theo_mult %1\n"
                   "diffusion len       %2\n"
                   "psv                 %3\n"
                   "water_e_density     %4\n"
                   "I0_prot_theo        %5\n"
                   "Avogadro            %6\n"
                   "conc                %7\n"
                   "internal contrast   %8\n"
                   "nucleon mass        %9\n"
                   )

      .arg( I0_exp_to_theo_mult, 0, 'e', 8 )
      .arg( our_saxs_options->diffusion_len, 0, 'e', 8  )
      .arg( use_psv, 0, 'e', 8  )
      .arg( our_saxs_options->water_e_density * 1e24, 0, 'e', 8  )
      .arg( I0_prot_theo, 0, 'e', 8  )
      .arg( AVOGADRO, 0, 'e', 8  )
      .arg( conc, 0, 'e', 8  )
      .arg( internal_contrast, 0, 'e', 8  )
      .arg( our_saxs_options->nucleon_mass, 0, 'e', 8  );
#endif

   return true;
}

bool US_Hydrodyn_Saxs::ma_from_q2I0( QString name, double I0_exp, double &MA, double &internal_contrast )
{
   double conc;
   double psv;
   double I0_std_exp;

   if ( name.contains( QRegularExpression( "_Istarq_" ) ) ) {
      MA = I0_exp / (2. * M_PI);
      return true;
   }

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   if ( conc == 0e0 ) {
      errormsg = us_tr( "Error: Concentration is 0, can not compute MA" );
      MA = 0e0;
      return false;
   }

   if ( name.contains( QRegularExpression( "_Ihashq_" ) ) ) {
      MA = I0_exp / (conc * 1e-3) / (2. * M_PI);
      return true;
   }

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      if ( I0_std_exp == 0e0 )
      {
         errormsg = us_tr( "Error: I0 standard experimental is 0, can not compute MA" );
         MA = 0e0;
         return false;
      }
      I0_exp_to_theo_mult = our_saxs_options->I0_theo / I0_std_exp;
   }

   double I0_prot_theo = I0_exp * I0_exp_to_theo_mult;

   if ( our_saxs_options->nucleon_mass == 0e0 )
   {
      errormsg = us_tr( "Error: Mass of nucleon is 0, can not compute MA" );
      MA = 0e0;
      return false;
   }

   double use_psv = our_saxs_options->use_cs_psv ? our_saxs_options->cs_psv : psv;

   double guinier_electron_nucleon_ratio =
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "guinier_electron_nucleon_ratio" ) ?
      ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "guinier_electron_nucleon_ratio" ].toDouble() : 1.87e0;

   internal_contrast = 
      our_saxs_options->diffusion_len * 
      ( 1e0 / ( guinier_electron_nucleon_ratio * our_saxs_options->nucleon_mass ) - use_psv * ( 1e24 * our_saxs_options->water_e_density ) );
   
   MA = I0_prot_theo * AVOGADRO / ( conc * 1e-3 ) / ( internal_contrast * internal_contrast ) / (2e0 * M_PI);

#if defined( DEBUG_MW )
   cout << QString( 
                   "I0_std_theo         %1\n"
                   "I0_std_exp          %2\n"
                   "I0                  %3\n"
                   "MA                  %4\n"
                   )
      .arg( our_saxs_options->I0_theo, 0, 'e', 8  )
      .arg( I0_std_exp, 0, 'e', 8  )
      .arg( I0_exp, 0, 'e', 8  )
      .arg( MA, 0, 'e', 8  )
      ;
                   
   cout << QString( 
                   "I0_exp_to_theo_mult %1\n"
                   "diffusion len       %2\n"
                   "psv                 %3\n"
                   "water_e_density     %4\n"
                   "I0_prot_theo        %5\n"
                   "Avogadro            %6\n"
                   "conc                %7\n"
                   "internal contrast   %8\n"
                   "nucleon mass        %9\n"
                   )

      .arg( I0_exp_to_theo_mult, 0, 'e', 8 )
      .arg( our_saxs_options->diffusion_len, 0, 'e', 8  )
      .arg( use_psv, 0, 'e', 8  )
      .arg( our_saxs_options->water_e_density * 1e24, 0, 'e', 8  )
      .arg( I0_prot_theo, 0, 'e', 8  )
      .arg( AVOGADRO, 0, 'e', 8  )
      .arg( conc, 0, 'e', 8  )
      .arg( internal_contrast, 0, 'e', 8  )
      .arg( our_saxs_options->nucleon_mass, 0, 'e', 8  );
#endif

   return true;
}


void US_Hydrodyn_Saxs::guinier_window()
{
#if defined( DEBUG_MW2 )
   saxs_options sav_saxs_options = *our_saxs_options;

   double I0;
   double MW;
   double ML;
   double ICL;

   I0 = 3.6e-4;
   our_saxs_options->psv = 0.73;
   our_saxs_options->water_e_density = 0.3365;
   our_saxs_options->conc = 4.47e-1;
   our_saxs_options->I0_exp = 5.4e-5;

   if ( mw_from_I0( "n/a", I0, MW, ICL ) )
   {
      cout << QString( "I0 %1 MW %2 ICL %3\n" ).arg( I0 ).arg( MW ).arg( ICL );
   } else {
      cout << errormsg << endl;
   }

   I0 = 1.4174e-6;
   our_saxs_options->psv = 7.458756e-1;
   our_saxs_options->conc = 0.35;
   our_saxs_options->I0_exp = 4.65e-5;
   if ( ml_from_qI0( "n/a", I0, ML, ICL ) )
   {
      cout << QString( "I0 %1 ML %2 ICL %3\n" ).arg( I0 ).arg( ML ).arg( ICL );
   } else {
      cout << errormsg << endl;
   }

   *our_saxs_options = sav_saxs_options;
#endif
   if ( ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_widget )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->isVisible() )
      {
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->raise();
      }
      else
      {
         ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->show();
      }
   }
   else
   {
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window = new US_Hydrodyn_SasOptionsGuinier( &((US_Hydrodyn *)us_hydrodyn)->saxs_options, 
                                                                                                    &((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_widget, 
                                                                                                    us_hydrodyn );
      US_Hydrodyn::fixWinButtons( ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window );
      ((US_Hydrodyn *)us_hydrodyn)->sas_options_guinier_window->show();
   }
}

void US_Hydrodyn_Saxs::sync_conc_csv() // removes deleted curves, adds non-extant curves with default
{
   if ( !conc_csv.data.size() )
   {
      // setup & add all
      conc_csv.name = "Set Curve Concentration, PSV and I0 standard experimental";

      conc_csv.header.clear( );
      conc_csv.header_map.clear( );
      conc_csv.data.clear( );
      conc_csv.num_data.clear( );
      conc_csv.prepended_names.clear( );
      
      conc_csv.header.push_back("File");
      conc_csv.header.push_back("Concentration (mg/ml)");
      conc_csv.header.push_back("PSV (ml/g)");
      conc_csv.header.push_back("I0 standard expt. (a.u.)");
   }

   // delete any non-plotted ones

   map < QString, bool > current_files;
   map < QString, bool > csv_files;
   
   for ( unsigned int i = 0; 
         i < ( unsigned int ) plotted_Iq.size();
         i++ )
   {
      current_files[ qsl_plotted_iq_names[ i ] ] = true;
   }
   csv new_csv = conc_csv;
   new_csv.data.clear( );
   new_csv.num_data.clear( );
   new_csv.prepended_names.clear( );
   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      csv_files[ conc_csv.data[ i ][ 0 ] ] = true;
      if ( current_files.count( conc_csv.data[ i ][ 0 ] ) )
      {
         new_csv.data.push_back( conc_csv.data[ i ] );
         new_csv.prepended_names.push_back( conc_csv.data[ i ][ 0 ] );
      }
   }
   conc_csv = new_csv;

   // add new ones
   for ( unsigned int i = 0; 
         i < ( unsigned int ) plotted_Iq.size();
         i++ )
   {
      if ( !csv_files.count( qsl_plotted_iq_names[ i ] ) )
      {
         add_conc_csv( qsl_plotted_iq_names[ i ] );
      }
   }

   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      vector < double > tmp_num_data;
      for ( unsigned int j = 0; j < conc_csv.data[ i ].size(); j++ )
      {
         tmp_num_data.push_back( conc_csv.data[ i ][ j ].toDouble() );
      }
      conc_csv.num_data.push_back( tmp_num_data );
   }

   // #define DEBUG_CSV
#if defined( DEBUG_CSV )
   puts( "sync_csv" );
   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < conc_csv.data[ i ].size(); j++ )
      {
         cout << conc_csv.data[ i ][ j ] << " : ";
      }
      cout << endl;
   }
#endif   
}      

void US_Hydrodyn_Saxs::add_conc_csv( QString name, double conc, double psv, double I0_std )
{
#if defined( DEBUG_CSV )
   cout << QString( "add_conc_csv %1 %2 %3 %4\n" ).arg( name ).arg( conc ).arg( psv ).arg( I0_std );
#endif
   vector < QString > tmp_data;
   tmp_data.push_back( name );
   tmp_data.push_back( QString( "%1" ).arg( conc ) );
   tmp_data.push_back( QString( "%1" ).arg( psv ) );
   tmp_data.push_back( QString( "%1" ).arg( I0_std ) );
            
   conc_csv.prepended_names.push_back(tmp_data[0]);
   conc_csv.data.push_back(tmp_data);

   vector < double > tmp_num_data;
   for ( unsigned int j = 0; j < conc_csv.data.back().size(); j++ )
   {
      tmp_num_data.push_back( conc_csv.data.back()[ j ].toDouble() );
   }
   conc_csv.num_data.push_back( tmp_num_data );
#if defined( DEBUG_CSV )
   puts( "after add" );
   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      for ( unsigned int j = 0; j < conc_csv.data[ i ].size(); j++ )
      {
         cout << conc_csv.data[ i ][ j ] << " : ";
      }
      cout << endl;
   }
#endif   
}

void US_Hydrodyn_Saxs::update_conc_csv( QString name, double conc, double psv, double I0_std )
{
#if defined( DEBUG_CSV )
   cout << QString( "update_conc_csv %1 %2 %3 %4\n" ).arg( name ).arg( conc ).arg( psv ).arg( I0_std );
#endif

   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      if ( name == conc_csv.data[ i ][ 0 ] )
      {
         conc_csv.data[ i ][ 1 ] = QString( "%1" ).arg( conc );
         conc_csv.data[ i ][ 2 ] = QString( "%1" ).arg( psv );
         conc_csv.data[ i ][ 3 ] = QString( "%1" ).arg( I0_std );
         vector < double > tmp_num_data;
         for ( unsigned int j = 0; j < conc_csv.data[ i ].size(); j++ )
         {
            conc_csv.num_data[ i ][ j ] = conc_csv.data[ i ][ j ].toDouble();
         }
#if defined( DEBUG_CSV )
         puts( "after update" );
         for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
         {
            for ( unsigned int j = 0; j < conc_csv.data[ i ].size(); j++ )
            {
               cout << conc_csv.data[ i ][ j ] << " : ";
            }
            cout << endl;
         }
#endif   
         return;
      }
   }   
#if defined( DEBUG_CSV )
   cout << QString( "not found, adding\n" );
#endif
   add_conc_csv( name, conc, psv, I0_std );
}

void US_Hydrodyn_Saxs::update_conc_csv( QString name, double conc, double psv )
{
   update_conc_csv( name, conc, psv, our_saxs_options->I0_exp );
}

void US_Hydrodyn_Saxs::update_conc_csv( QString name, double conc )
{
   update_conc_csv( name, conc, our_saxs_options->psv, our_saxs_options->I0_exp );
}

void US_Hydrodyn_Saxs::update_conc_csv( QString name )
{
   update_conc_csv( name, our_saxs_options->conc, our_saxs_options->psv, our_saxs_options->I0_exp );
}

void US_Hydrodyn_Saxs::add_conc_csv( QString name )
{
   add_conc_csv( name, our_saxs_options->conc, our_saxs_options->psv, our_saxs_options->I0_exp );
}

bool US_Hydrodyn_Saxs::get_conc_csv_values( QString name, double &conc, double &psv, double &I0_std_exp )
{
   for ( unsigned int i = 0; i < conc_csv.data.size(); i++ )
   {
      if ( name == conc_csv.data[ i ][ 0 ] )
      {
         conc       = conc_csv.num_data[ i ][ 1 ];
         psv        = conc_csv.num_data[ i ][ 2 ];
         I0_std_exp = conc_csv.num_data[ i ][ 3 ];
         return true;
      }
   }   
   conc       = our_saxs_options->conc;
   psv        = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;

   return false;
}


#define TSO QTextStream(stdout)

void US_Hydrodyn_Saxs::pr_info2( const QString & msg ) {
   pr_info( msg, true );
}

void US_Hydrodyn_Saxs::pr_info( const QString & msg, bool detail ) {

   TSO
      << "==== pr_info( " << msg << " ) ==== START ====\n"
      << QString(
                 "plotted_r.size()                          %1\n"
                 "plotted_pr.size()                         %2\n"
                 "plotted_pr_error.size()                   %3\n"
                 "plotted_pr_not_normalized.size()          %4\n"
                 "plotted_pr_not_normalized_error.size()    %5\n"
                 "plotted_pr_mw.size()                      %6\n"
                 "qsl_plotted_pr_names.size()               %7\n"
                 )

      .arg(plotted_r.size())
      .arg(plotted_pr.size())
      .arg(plotted_pr_error.size())
      .arg(plotted_pr_not_normalized.size())
      .arg(plotted_pr_not_normalized_error.size())
      .arg(plotted_pr_mw.size())
      .arg(qsl_plotted_pr_names.size())
      ;

   list < int > sizes;
   sizes.push_back( (int) plotted_r.size() );
   sizes.push_back( (int) plotted_pr.size() );
   sizes.push_back( (int) plotted_pr_error.size() );
   sizes.push_back( (int) plotted_pr_not_normalized.size() );
   sizes.push_back( (int) plotted_pr_not_normalized_error.size() );
   sizes.push_back( (int) qsl_plotted_pr_names.size() );
   sizes.sort();

   vector < double > emptyv;

   for ( int i = 0; i < sizes.back(); ++i ) {
      if ( detail ) {
         US_Vector::printvector5(
                                 QString( "entry %1 mw %2 plotted_r,_pr,_pr_error,_pr_nn,pr_nn_error" )
                                 .arg( (int) qsl_plotted_pr_names.size() > i ? qsl_plotted_pr_names[i] : QString( "?" ) )
                                 .arg( (int) plotted_pr_mw.size() > i ? plotted_pr_mw[i] : -1 )
                                 ,(int) plotted_r.size() > i ? plotted_r[i] : emptyv
                                 ,(int) plotted_pr.size() > i ? plotted_pr[i] : emptyv
                                 ,(int) plotted_pr_error.size() > i ? plotted_pr_error[i] : emptyv
                                 ,(int) plotted_pr_not_normalized.size() > i ? plotted_pr_not_normalized[i] : emptyv
                                 ,(int) plotted_pr_not_normalized_error.size() > i ? plotted_pr_not_normalized_error[i] : emptyv
                                 )
            ;
      } else {
         TSO <<
            QString(
                    "entry %1 [%8]: r %2 pr %3 pr_error %4 pr_nn %5 pr_nn_e %6 pr_mw %7\n"
                    )
            .arg( i )
            .arg( (int) plotted_r.size() > i ? (int) plotted_r[i].size() : -1 )
            .arg( (int) plotted_pr.size() > i ? (int) plotted_pr[i].size() : -1 )
            .arg( (int) plotted_pr_error.size() > i ? (int) plotted_pr_error[i].size() : -1 )
            .arg( (int) plotted_pr_not_normalized.size() > i ? (int) plotted_pr_not_normalized[i].size() : -1 )
            .arg( (int) plotted_pr_not_normalized_error.size() > i ? (int) plotted_pr_not_normalized_error[i].size() : -1 )
            .arg( (int) plotted_pr_mw.size() > i ? plotted_pr_mw[i] : -1 )
            .arg( (int) qsl_plotted_pr_names.size() > i ? qsl_plotted_pr_names[i] : QString( "?" ) )
            ;
      }         
   }

   TSO
      << "==== pr_info( " << msg << " ) ==== END ====\n"
      ;
}

bool US_Hydrodyn_Saxs::pr_to_iq() {
   TSO << "US_Hydrodyn_Saxs::pr_to_iq() slot not yet implemented\n";
   errormsg = "";
   if ( qsl_plotted_pr_names.size() == 0 ) {
      errormsg = us_tr( "No P(r) curves plotted\n" );
      editor_msg( "red", errormsg );
      return false;
   }
   if ( qsl_plotted_pr_names.size() == 1 ) {
      return pr_to_iq( 0, qsl_plotted_pr_names[0] );
   }
   
   QString source = "";
   {
      bool ok;
      source = US_Static::getItem(
                                  us_tr("Compute I(q)")
                                  ,us_tr("Select the P(r) curve to calculate I(q)" )
                                  ,qsl_plotted_pr_names, 
                                  0, 
                                  false, 
                                  &ok,
                                  this );
      if ( ok ) {
         // user selected an item and pressed OK
      } else {
         return false;
      }
   }

   if ( source.isEmpty() ) {
      return false;
   }

   for ( int pos = 0; pos < (int)qsl_plotted_pr_names.size(); ++pos ) {
      if ( source == qsl_plotted_pr_names[pos] ) {
         return pr_to_iq( pos, source );
      }
   }

   errormsg =
      QString( us_tr( "Internal error, could not find source '%1' in plotted names\n" ) )
      .arg( source )
      ;
      
   editor_msg( "red", errormsg );
   return false;
}

bool US_Hydrodyn_Saxs::pr_to_iq( int pos, QString name ) {
   errormsg = "";
   TSO << QString( "pr_to_iq( %1 )\n" ).arg( pos );

   if (
       (int) plotted_pr.size() <= pos
       || (int) plotted_r.size() <= pos
       ) {
      errormsg = QString( "pr_to_iq:: pos %1 not available\n" );
      pr_info( errormsg );
      return false;
   }
   
   vector < double > *r  = &plotted_r[pos];
   vector < double > *pr = &plotted_pr[pos];

   int r_points = (int) r->size();

   if ( r->size() < 2 ) {
      errormsg = QString( "pr_to_iq:: curve has insufficient points\n" );
      pr_info( errormsg );
      return false;
   }
      
   double dr = (*r)[1] - (*r)[0];

   TSO << QString( "pr_to_iq:: dr %1\n" ).arg( dr );
   
   int q_points = 
      (int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;

   if ( our_saxs_options->iq_exact_q )
   {
      editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
      if ( !exact_q.size() ){
         editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
         exact_q.resize( q_points );
         for ( int j = 0; j < q_points; j++ ) {
            exact_q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
         }
      } else {
         q_points = ( int ) exact_q.size();
      }
   }
   
   vector < double > q( q_points ); 
   vector < double > I( q_points ); 

   if ( our_saxs_options->iq_exact_q ) {
      q = exact_q;
      q_points = q.size();
   } else {
      for ( int i = 0; i < q_points; i++ ) {
         q[i] = our_saxs_options->start_q + i * our_saxs_options->delta_q;
      }
   }      

   // calc for i = 0 first
   {
      int i = 0;
      I[i] = 0;

      for ( int j = 0; j < r_points; j++ ) {
         I[i] += (*pr)[j];
      }
      I[i] *= dr; // no modulation needed..., q[0] == 0
   }

   // and then the rest
   for ( int i = 1; i < q_points; ++i ) {
      {
         int j = 0;
         I[i] += (*pr)[j];
      }

      double qr;
      for ( int j = 1; j < r_points; j++ ) {
         qr = q[i] * (*r)[j];
         I[i] += (*pr)[j] * (( fabs(qr) < 1e-16 ) ? 1.0 : sin(qr) / qr);
      }
      I[i] *=
         dr
         * exp( - our_saxs_options->fast_modulation * q[i] * q[i] )
         ;
   }

   // US_Vector::printvector2( "source r, pr", *r, *pr );

   // US_Vector::printvector2( "computed q, I", q, I );
   
   // setup scaling

   {
      QString scaling_target = "";
      set_scaling_target( scaling_target );
   
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }
   }
   
   plot_one_iqq( q, I, name );

   return true;
}

bool US_Hydrodyn_Saxs::pr_to_iq( const map < double, double > & pr_exact, QString name ) {
   errormsg = "";
   TSO << QString( "pr_to_iq( pr_exact, \"%1\" )\n" ).arg( name );

   if (
       ! pr_exact.size()
       ) {
      errormsg = QString( "pr_to_iq:: empty exact\n" );
      pr_info( errormsg );
      return false;
   }

   int q_points = 
      (int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;

   if ( our_saxs_options->iq_exact_q )
   {
      editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
      if ( !exact_q.size() ){
         editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
         exact_q.resize( q_points );
         for ( int j = 0; j < q_points; j++ ) {
            exact_q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
         }
      } else {
         q_points = ( int ) exact_q.size();
      }
   }
   
   vector < double > q( q_points ); 
   vector < double > I( q_points ); 

   if ( our_saxs_options->iq_exact_q ) {
      q = exact_q;
      q_points = q.size();
   } else {
      for ( int i = 0; i < q_points; i++ ) {
         q[i] = our_saxs_options->start_q + i * our_saxs_options->delta_q;
      }
   }      

   for ( int i = 0; i < q_points; ++i ) {
      double qr;
      for ( auto it = pr_exact.begin();
            it != pr_exact.end();
            ++it ) {
         qr = q[i] * it->first;
         I[i] += it->second * (( fabs(qr) < 1e-16 ) ? 1.0 : sin(qr) / qr);
      }
      I[i] *=
         exp( - our_saxs_options->fast_modulation * q[i] )
         ;
   }

   // US_Vector::printvector2( "source r, pr", *r, *pr );

   // US_Vector::printvector2( "computed q, I", q, I );
   
   // setup scaling

   if ( ((US_Hydrodyn *)us_hydrodyn)->batch_widget && ((US_Hydrodyn *)us_hydrodyn)->batch_window->batch_job_running ) {
      if ( plotted_I.size() == 1 && plotted_I[0].size() && I[0] ) {
         double scale = plotted_I[0][0] / I[0];
         TSO << "Auto scaling to the one Iq target scale:" << scale << "\n";
         for ( int i = 0; i < q_points; ++i ) {
            I[i] *= scale;
         }
         if ( plotted_q[0] == q ) {
            TSO << "Q grids match!\n";
            // create subtracted curve & save the difference curve
            vector < double > Idiff = plotted_I[0];
            QString qsdiff;
            QString qspct;
            for ( int i = 0; i < q_points; ++i ) {
               Idiff[i] -= I[i];
               qsdiff += QString( "%1 %2\n" ).arg( q[i] ).arg( Idiff[i] );
               if ( plotted_I[0][i] ) {
                  Idiff[i] /= plotted_I[0][i];
               }
               qspct += QString( "%1 %2\n" ).arg( q[i] ).arg( 100 * Idiff[i] );
            }
            QString error;
            QString diffname = name + "_Idiff.dat";
            QString pctname = name + "_Idiffpct.dat";
            if ( !US_File_Util::putcontents( diffname, qsdiff, error ) ) {
               TSO << "Error: writing " << diffname << "\n";
            } else {
               TSO << QString( "wrote diff file: %1%2%3\n").arg( QDir::currentPath() ).arg( QDir::separator() ).arg( diffname );
            }
            if ( !US_File_Util::putcontents( pctname, qspct, error ) ) {
               TSO << "Error: writing " << diffname << "\n";
            } else {
               TSO << QString( "wrote diff pct file: %1%2%3\n").arg( QDir::currentPath() ).arg( QDir::separator() ).arg( pctname );
            }
         } else {
            TSO << "Q grids DO NOT match :(\n";
         }
      }
   } else {
      QString scaling_target = "";
      set_scaling_target( scaling_target );

      TSO << "Scaling target " << scaling_target << "\n";
   
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }
   }
   
   

   plot_one_iqq( q, I, name );

   return true;
}

