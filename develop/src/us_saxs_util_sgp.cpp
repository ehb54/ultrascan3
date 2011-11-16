#include "../include/us_saxs_util.h"
#include "../include/us_saxs_gp.h"

bool US_Saxs_Util::sgp_run()
{
   if ( !sgp_validate() )
   {
      return false;
   }

   if ( !sgp_init() )
   {
      return false;
   }

   errormsg = "not yet";
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

   return true;
}

bool US_Saxs_Util::sgp_validate()
{
   errormsg = "";
   noticemsg = "";

   // first check required parameters

   QString missing_required;

   QStringList qsl_required;

    {
      qsl_required << "sgpgenerations";
      qsl_required << "sgppopulation";
      
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

   return true;
}

double US_Saxs_Util::sgp_fitness( sgp_node *node )
{
   // take node & run current bead model iq on its bead model and compute chi2 (if errors present) or rmsd & return
   return 0e0;
}
