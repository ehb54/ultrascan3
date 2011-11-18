#include "../include/us_saxs_util.h"
#include "../include/us_saxs_gp.h"


bool US_Saxs_Util::nsa_validate()
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
      errormsg = QString( "Error: nsa requires prior definition of:%1" )
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

   cout << QString( "setup experiment grid sizes %1 %2 %3\n" ).arg( sgp_exp_q.size() ).arg( sgp_exp_I.size() ).arg( sgp_exp_e.size() );

   if ( sgp_exp_e.size() && 
        sgp_exp_e.size() == sgp_exp_q.size() &&
        is_nonzero_vector( sgp_exp_q ) )
   {
      sgp_use_e = true;
      cout << "Notice: nsa using sd's for fitting\n";
   } else {
      sgp_use_e = false;
      cout << "Notice: nsa NOT using sd's for fitting\n";
   }

   return true;
}

bool US_Saxs_Util::nsa_run()
{
   if ( !nsa_validate() )
   {
      return false;
   }
   setup_saxs_options();
   if ( !sgp_init_sgp() )
   {
      return false;
   }

   sgp_node * node = new sgp_node();

   node->normal.axis[ 0 ] = 1e0;

   control_parameters[ "sgp_running" ] = "yes";

   // first find 1sa minimum

   bool         first          = true;
   double       bestfitness;
   unsigned int bestfitnesspos;

   // 1sa

   for ( node->radius = sgp_params[ "radiusmin" ];
         node->radius <= sgp_params[ "radiusmax" ];
         node->radius++ )
   {
      double this_fitness = sgp_fitness( node );
      // cout << QString( "radius %1 fitness %2\n" )
      // .arg( node->radius * sgp_params[ "distancequantum" ] )
      // .arg( this_fitness );

      if ( first )
      {
         first = false;
         bestfitness = this_fitness;
         bestfitnesspos = node->radius;
      } else {
         if ( bestfitness > this_fitness )
         {
            bestfitness = this_fitness;
            bestfitnesspos = node->radius;
         }
      }
   }

   node->radius = bestfitnesspos;

   {
      QFile f( "1sa.bead_model" );
      if ( f.open( IO_WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << node->qs_bead_model();
         ts << sgp_physical_stats( node );
         f.close();
         cout << QString( "written: %1\n" ).arg( f.name() );
      }
      control_parameters.erase( "sgp_running" );
      sgp_fitness( node );
      control_parameters[ "sgp_running" ] = "yes";
   }

   // 2sa

   sgp_node * node2 = new sgp_node();
   node2->normal = node->normal;
   node->insert_copy( 0, node2 );
   delete node2;
   node2 = node->ref( 1 );


   first        = true;
   unsigned int bestfitnesspos2;
   unsigned int bestfitnesspos3;

   for ( node->radius = sgp_params[ "radiusmin" ];
         node->radius <= sgp_params[ "radiusmax" ];
         node->radius++ )
   {
      for ( node2->radius = sgp_params[ "radiusmin" ];
            node2->radius <= sgp_params[ "radiusmax" ];
            node2->radius++ )
      {
         for ( node2->distance = sgp_params[ "distancemin" ];
               node2->distance <= sgp_params[ "distancemax" ];
               node2->distance++ )
         {
            // cout << node->qs_bead_model();
            double this_fitness = sgp_fitness( node );
            // cout << QString( "radius %1, %2, distance %3  fitness %4\n" )
            // .arg( node->radius * sgp_params[ "distancequantum" ] )
            // .arg( node2->radius * sgp_params[ "distancequantum" ] )
            // .arg( node2->distance * sgp_params[ "distancequantum" ] )
            // .arg( this_fitness );
            
            if ( first )
            {
               first = false;
               bestfitness = this_fitness;
               bestfitnesspos = node->radius;
               bestfitnesspos2 = node2->radius;
               bestfitnesspos3 = node2->distance;
            } else {
               if ( bestfitness > this_fitness )
               {
                  bestfitness = this_fitness;
                  bestfitnesspos = node->radius;
                  bestfitnesspos2 = node2->radius;
                  bestfitnesspos3 = node2->distance;
               }
            }
         }
      }
   }


   node->radius = bestfitnesspos;
   node2->radius = bestfitnesspos2;
   node2->distance = bestfitnesspos3;

   {
      QFile f("2sa.bead_model" );
      if ( f.open( IO_WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << node->qs_bead_model();
         ts << sgp_physical_stats( node );
         f.close();
         cout << QString( "written: %1\n" ).arg( f.name() );
      }
      control_parameters.erase( "sgp_running" );
      sgp_fitness( node );
      control_parameters[ "sgp_running" ] = "yes";
   }
   control_parameters.erase( "sgp_running" );
   return true;
}
