// #define DEBUG_MW2 // for testing "preset values on guinier_window()"
// #define DEBUG_MW

#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn_saxs_mw.h"
#include "../include/us_hydrodyn.h"
#include <qregexp.h>

#define SLASH QDir::separator()
double US_Hydrodyn_Saxs::get_mw( QString filename, bool display_mw_msg )
{
   // enter MW and PSV
   filename = QFileInfo(filename).fileName();
   float mw = 0.0;
   bool remember = true;
   bool use_partial = false;
   QString partial = filename;
   QString msg = QString(tr(" Enter values for total molecular weight: "));
   QString source = "";
   bool found = false;
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
            // printf("iterator first %s\n", it->first.ascii());
            // printf("iterator second %f\n", it->second);
            
            if ( filename.contains(it->first) )
            {
               mw = (*match_remember_mw)[it->first];
               source = QString(tr("file name matched \"%1\"")).arg(it->first);
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
                                                         this
                                                         );
      do {
         smw->exec();
      } while ( mw <= 0.0 );
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
   }
   //   printf( "%s is %g\n",  QString( "get mw filename %1" ).arg( filename ).ascii(), mw );
   return mw;
}

void US_Hydrodyn_Saxs::normalize_pr( vector < double > r, vector < double > *pr , double mw )
{
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
   if ( max > 0e0 )
   {
      for ( unsigned int i = 0; i < pr->size(); i++ )
      {
         (*pr)[i] /= max;
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
         for ( unsigned int i = 0; i < pr->size(); i++ )
         {
            (*pr)[i] /= area;
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
         (*pr)[i] *= pr->size() / area ;
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

   US_Saxs_Util usu;
   vector < double > y2;
   usu.natural_spline( r, pr, y2 );

   vector < double > new_pr( new_r.size() );

   for ( unsigned int i = 0; i < new_r.size(); i++ )
   {
      if ( !usu.apply_natural_spline( r, pr, y2, new_r[ i ], new_pr[ i ] ) )
      {
         editor_msg( "red", usu.errormsg );
         editor_msg( "red", tr( "Error attempting to interpolate" ) );
         return;
      }
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

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      // cout << "mw_from_I0: using standards\n";
      if ( I0_std_exp == 0e0 )
      {
         errormsg = tr( "Error: I0 standard experimental is 0, can not compute MW" );
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
      errormsg = tr( "Error: Mass of nucleon is 0, can not compute MW" );
      MW = 0e0;
      return false;
   }

   if ( conc == 0e0 )
   {
      errormsg = tr( "Error: Concentration is 0, can not compute MW" );
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

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      if ( I0_std_exp == 0e0 )
      {
         errormsg = tr( "Error: I0 standard experimental is 0, can not compute MW" );
         ML = 0e0;
         return false;
      }
      I0_exp_to_theo_mult = our_saxs_options->I0_theo / I0_std_exp;
   }

   double I0_prot_theo = I0_exp * I0_exp_to_theo_mult;

   if ( our_saxs_options->nucleon_mass == 0e0 )
   {
      errormsg = tr( "Error: Mass of nucleon is 0, can not compute MW" );
      ML = 0e0;
      return false;
   }

   if ( conc == 0e0 )
   {
      errormsg = tr( "Error: Concentration is 0, can not compute MW" );
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

#if defined( DEBUG_MW2 )
   conc = our_saxs_options->conc;
   psv = our_saxs_options->psv;
   I0_std_exp = our_saxs_options->I0_exp;
#else
   get_conc_csv_values( name, conc, psv, I0_std_exp );
#endif

   double I0_exp_to_theo_mult = 1e0;
   if ( our_saxs_options->guinier_use_standards )
   {
      if ( I0_std_exp == 0e0 )
      {
         errormsg = tr( "Error: I0 standard experimental is 0, can not compute MW" );
         MA = 0e0;
         return false;
      }
      I0_exp_to_theo_mult = our_saxs_options->I0_theo / I0_std_exp;
   }

   double I0_prot_theo = I0_exp * I0_exp_to_theo_mult;

   if ( our_saxs_options->nucleon_mass == 0e0 )
   {
      errormsg = tr( "Error: Mass of nucleon is 0, can not compute MW" );
      MA = 0e0;
      return false;
   }

   if ( conc == 0e0 )
   {
      errormsg = tr( "Error: Concentration is 0, can not compute MW" );
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

void US_Hydrodyn_Saxs::ift()
{
}


void US_Hydrodyn_Saxs::sync_conc_csv() // removes deleted curves, adds non-extant curves with default
{
   if ( !conc_csv.data.size() )
   {
      // setup & add all
      conc_csv.name = "Set Curve Concentration, PSV and I0 standard experimental";

      conc_csv.header.clear();
      conc_csv.header_map.clear();
      conc_csv.data.clear();
      conc_csv.num_data.clear();
      conc_csv.prepended_names.clear();
      
      conc_csv.header.push_back("File");
      conc_csv.header.push_back("Concentration (mg/ml)");
      conc_csv.header.push_back("PSV (ml/g)");
      conc_csv.header.push_back("I0 standard expt. (a.u.)");
   }

   // delete any non-plotted ones

   map < QString, bool > current_files;
   map < QString, bool > csv_files;
   
   for ( unsigned int i = 0; 
#ifndef QT4
         i < ( unsigned int ) plotted_Iq.size();
#else
         i < ( unsigned int ) plotted_Iq_curves.size();
#endif
         i++ )
   {
      current_files[ qsl_plotted_iq_names[ i ] ] = true;
   }
   csv new_csv = conc_csv;
   new_csv.data.clear();
   new_csv.num_data.clear();
   new_csv.prepended_names.clear();
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
#ifndef QT4
         i < ( unsigned int ) plotted_Iq.size();
#else
         i < ( unsigned int ) plotted_Iq_curves.size();
#endif
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
