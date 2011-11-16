#include "../include/us_saxs_util.h"
#include "../include/us_saxs_gp.h"

bool US_Saxs_Util::sgp_run()
{
   if ( !sgp_validate() )
   {
      return false;
   }

   setup_saxs_options();

   control_parameters[ "sgp_running" ] = "yes";
   if ( !sgp_init() )
   {
      control_parameters.erase( "sgp_running" );
      return false;
   }

   errormsg = "not yet";
   control_parameters.erase( "sgp_running" );
   return false;
}

bool US_Saxs_Util::sgp_init()
{
   errormsg = "";
   noticemsg = "";

   QStringList param;
   QStringList sgp_param;
   param 
      << "sgpdistancequantum"
      << "sgpdistancemin"
      << "sgpdistancemax"
      << "sgpradiusmin"
      << "sgpradiusmax"
      << "sgpbranchmax";

   sgp_param
      << "distancequantum"
      << "distancemin"
      << "distancemax"
      << "radiusmin"
      << "radiusmax"
      << "branchmax";

   for ( unsigned int i = 0; i < param.size(); i++ )
   {
      sgp_params[ sgp_param[ i ] ] = control_parameters[ param[ i ] ].toDouble();
   }

   if ( !sgp_node::validate_params().isEmpty() )
   {
      errormsg = sgp_node::validate_params();
      return false;
   }

   population.clear();

   for ( unsigned int i = 0; i < control_parameters[ "sgppopulation" ].toUInt(); i++ )
   {
      population.push_back( sgp.random( 10 + i ) );
      if ( i && ! (i % 100 ) )
      {
         cout << QString( "creating population %1 of %2\n" ).arg( i ).arg( control_parameters[ "sgppopulation" ] );
      }
   }

   cout << QString( "population of %1 created\n" ).arg( population.size() );
   sgp_calculate_population_fitness();

   return true;
}

bool US_Saxs_Util::sgp_validate()
{
   errormsg = "";
   noticemsg = "";

   // need to be able to process iq:
   if ( !validate_control_parameters( true ) )
   {
      return false;
   }

   // first check required parameters

   QString missing_required;

   QStringList qsl_required;

    {
      qsl_required << "sgpgenerations";
      qsl_required << "sgppopulation";
      qsl_required << "targetedensity";
      
      for ( unsigned int i = 0; i < qsl_required.size(); i++ )
      {
         if ( !control_parameters.count( qsl_required[ i ] ) )
         {
            missing_required += " " + qsl_required[ i ];
         }
      }
   }

   if ( !missing_required.isEmpty() )
   {
      errormsg = QString( "Error: GP requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   // if certain parameters are not set, set them to defaults
   QStringList checks;
   QStringList vals;

   {
      checks << "sgpdistancequantum";
      vals   << "2.5";
      checks << "sgpdistancemin";
      vals   << "2.0";
      checks << "sgpdistancemax";
      vals   << "10.0";
      checks << "sgpradiusmin";
      vals   << "1.0";
      checks << "sgpradiusmax";
      vals   << "5.0";
      checks << "sgpbranchmax";
      vals   << "4.0";

      validate_control_parameters_set_one( checks, vals );
   }

   if ( !sgp_exp_q.size() ||
        sgp_exp_q.size() != sgp_exp_I.size() )
   {
      errormsg = "Error: empty or inconsistant grid found experimental data, note:csv experiment grids not currently supported for sgp\n";
      return false;
   }

   if ( sgp_exp_e.size() && 
        sgp_exp_e.size() == sgp_exp_q.size() &&
        is_nonzero_vector( sgp_exp_q ) )
   {
      sgp_use_e = true;
      cout << "Notice: sgp using sd's for fitting\n";
   } else {
      sgp_use_e = false;
      cout << "Notice: sgp NOT using sd's for fitting\n";
   }

   return true;
}

double US_Saxs_Util::sgp_fitness( sgp_node *node )
{
   // take node & run current bead model iq on its bead model and compute chi2 (if errors present) or rmsd & return
      
   vector < PDB_atom > bm = node->bead_model();
   bead_models.clear();
   bead_models.push_back( bm );

   // compute iq:

   run_iqq_bead_model();

   // values stored in last_q, last_I, experiment in sgp_exp_q,I,e

   double k;
   double chi2;
   if ( sgp_use_e )
   {
      scaling_fit( sgp_last_I, sgp_exp_I, sgp_exp_e, k, chi2 );
   } else {
      scaling_fit( sgp_last_I, sgp_exp_I, k, chi2 );
   }

   return chi2;
}

void US_Saxs_Util::sgp_calculate_population_fitness()
{
   for ( unsigned int i = 0; i < population.size(); i++ )
   {
      population[ i ]->fitness = sgp_fitness( population[ i ] );
      cout << QString( "sgp_fitness for %1 %2\n" ).arg( i ).arg( population[ i ]->fitness );
   }
   sgp_sort_population();
}

void US_Saxs_Util::sgp_sort_population()
{
   
   list < sortable_sgp_node > ssns;

   for ( unsigned int i = 0; i < population.size(); i++ )
   {
      sortable_sgp_node ssn;
      ssn.node = population[ i ];
      ssns.push_back( ssn );
   }

   ssns.sort();

   vector < sgp_node * > sorted_population;

   for ( list < sortable_sgp_node >::iterator it = ssns.begin();
         it != ssns.end();
         it++ )
   {
      sorted_population.push_back( (*it).node );
   }

   population = sorted_population;

   cout << "after sort:\n";
   for ( unsigned int i = 0; i < population.size(); i++ )
   {
      cout << QString( "sgp_fitness for %1 %2\n" ).arg( i ).arg( population[ i ]->fitness );
   }
}
