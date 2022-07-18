#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_pat.h"
//Added by qt3to4:
#include <QTextStream>

bool US_Saxs_Util::run_pat(
#if  defined( CMDLINE ) // to silence compiler warning

                           map < QString, QString >           & parameters,
                           map < QString, QString >           & results
#else
                           map < QString, QString >           &,
                           map < QString, QString >           &
#endif
                           )
{
#if  defined( CMDLINE )
   //results[ "errors" ] = " ";
   //USglobal = new US_Config();
 
   paths += getenv("ULTRASCAN");

   if ( !set_default(results, parameters) )         // setup configuration defaults before reading initial config
      return false;
  
  
   misc.restore_pb_rule = false;

   residue_filename = paths + "/" + "etc" + "/" + "somo.residue";
   QFileInfo fi_res_filename( residue_filename );
   if ( !fi_res_filename.exists() )
   {
      results[ "errors" ] += QString( " file %1 does not exist." ).arg( residue_filename );
      return false;
   }
  
   accumulated_msgs = "";
  
   read_residue_file();
  
   //   reset_hydro_res(results_hydro);

   // Read pdb_file name ////////////////////////////////////////////

   if ( !parameters.count( "pdbfile" ) ) {
      results[ "errors" ] = "no pdbfile specified";
      return false;
   }

   if ( !parameters.count( "_base_directory" ) ) {
      parameters[ "_base_directory" ] = ".";
   }
  
   QString file;
    
   QString files_try = parameters[ "pdbfile" ].replace('"', "").replace("\\/","/") ;
   parameters[ "_base_directory" ] = parameters[ "_base_directory" ].replace("\\/","/");

   //    results["check_file"] = files_try;

   QFileInfo fi( files_try );
   if ( !fi.exists() ) {
      results[ "errors" ] += QString( " file %1 does not exist." ).arg( files_try );
      return false;
   } else {
      if ( !fi.isReadable() ) {
         results[ "errors" ] += QString( " file %1 exists but is not readable." ).arg( files_try );
         return false;
      } else {
         file = fi.filePath(); //files_try;
      }
   }

   bool parameters_set_first_model;
   if ( !parameters.count( "first_model" ) ) {
      parameters_set_first_model = false;
   } else {
      parameters_set_first_model = true;
   } 
    
   bead_model_from_file = false;
   new_residues.clear( );

   if ( misc.pb_rule_on ) {
      residue_list = save_residue_list;
   } else {
      residue_list = save_residue_list_no_pbr;
   }

   multi_residue_map = save_multi_residue_map;
   if ( read_pdb_hydro( file, parameters_set_first_model) ) {
      return false;
   }

   if ( !model_vector.size() ) {
      results[ "errors" ] += QString( " file %1: empty model vector?" ).arg( file );
      return false;
   }
      
   if ( !pat_model( model_vector[ 0 ] ) ) {
      results[ "errors" ] += QString( " pat error" ).arg( file );
      return false;
   }

   results[ "progress" ] += "pat finished ok.";

   QString outfile = file + ".pat";
   results[ "errors" ] += QString( " write pdb not yet implemented in saxs util" );
#endif
   return false;

   // needs better write pdb
   // if ( write_pdb_hydro( outfile, model_vector[ 0 ] ) ) {
   //    results[ "errors" ] += QString( " write pdb error" ).arg( outfile );
   //    return false;
   // }

   
   
   // results[ "progress" ] += " write ok."
   
   // return true;
}

bool US_Saxs_Util::pat_model( PDB_model & model, bool bead_coordinates )
{
   if ( !model.molecule.size() )
   {
      return false;
   }

   // run pat? i.e. do a principal axis transformation

   if ( bead_coordinates ) {
      {
         vector < dati1_supc > in_dt;
         int dt_pos = 0;
         for ( int j = 0; j < (int) model.molecule.size (); ++j ) 
         {
            for ( int k = 0; k < (int) model.molecule[ j ].atom.size (); ++k ) 
            {
               PDB_atom *this_atom = &( model.molecule[ j ].atom[ k ] );
            
               dati1_supc dt;
               
               dt.x  = this_atom->bead_coordinate.axis[ 0 ];
               dt.y  = this_atom->bead_coordinate.axis[ 1 ];
               dt.z  = this_atom->bead_coordinate.axis[ 2 ];
               dt.r  = this_atom->bead_computed_radius;
               dt.ru = this_atom->bead_computed_radius;
               dt.m  = this_atom->bead_mw;
            
               in_dt.push_back( dt );

               // us_qdebug( QString( "atom %1 coords %2 %3 %4 radius %5 mw %6" )
               //         .arg( dt_pos )
               //         .arg( dt.x )
               //         .arg( dt.y )
               //         .arg( dt.z )
               //         .arg( dt.r )
               //         .arg( dt.m )
               //         );
               ++dt_pos;
            }
         }

         int out_nat;
         vector < dati1_pat > out_dt( in_dt.size() + 1 );

         if ( !us_hydrodyn_pat_main( ( int ) in_dt.size(),
                                     ( int ) in_dt.size(),
                                     &( in_dt[ 0 ] ),
                                     &out_nat,
                                     &( out_dt[ 0 ] ) ) )
         {
            // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
            int dt_pos = 0;

            for ( int j = 0; j < (int) model.molecule.size (); ++j ) 
            {
               for ( int k = 0; k < (int) model.molecule[ j ].atom.size (); ++k ) 
               {
                  PDB_atom *this_atom = &( model.molecule[ j ].atom[ k ] );
            
                  this_atom->bead_coordinate.axis[ 0 ] = out_dt[ dt_pos ].x;
                  this_atom->bead_coordinate.axis[ 1 ] = out_dt[ dt_pos ].y;
                  this_atom->bead_coordinate.axis[ 2 ] = out_dt[ dt_pos ].z;
                  // us_qdebug( QString( "results atom %1 coords %2 %3 %4" )
                  //         .arg( dt_pos )
                  //         .arg( out_dt[ dt_pos ].x )
                  //         .arg( out_dt[ dt_pos ].y )
                  //         .arg( out_dt[ dt_pos ].z )
                  //         );

                  ++dt_pos;
               }
            }
         }
      }
   } else {
      {
         vector < dati1_supc > in_dt;
         int dt_pos = 0;
         for ( int j = 0; j < (int) model.molecule.size (); ++j ) 
         {
            for ( int k = 0; k < (int) model.molecule[ j ].atom.size (); ++k ) 
            {
               PDB_atom *this_atom = &( model.molecule[ j ].atom[ k ] );
            
               dati1_supc dt;
               
               dt.x  = this_atom->coordinate.axis[ 0 ];
               dt.y  = this_atom->coordinate.axis[ 1 ];
               dt.z  = this_atom->coordinate.axis[ 2 ];
               dt.r  = this_atom->radius;
               dt.ru = this_atom->radius;
               dt.m  = this_atom->mw;
            
               in_dt.push_back( dt );

               // us_qdebug( QString( "atom %1 coords %2 %3 %4 radius %5 mw %6" )
               //         .arg( dt_pos )
               //         .arg( dt.x )
               //         .arg( dt.y )
               //         .arg( dt.z )
               //         .arg( dt.r )
               //         .arg( dt.m )
               //         );
               ++dt_pos;
            }
         }

         int out_nat;
         vector < dati1_pat > out_dt( in_dt.size() + 1 );

         if ( !us_hydrodyn_pat_main( ( int ) in_dt.size(),
                                     ( int ) in_dt.size(),
                                     &( in_dt[ 0 ] ),
                                     &out_nat,
                                     &( out_dt[ 0 ] ) ) )
         {
            // cout << QString( "pat ok, out_nat %1\n" ).arg( out_nat );
            int dt_pos = 0;

            for ( int j = 0; j < (int) model.molecule.size (); ++j ) 
            {
               for ( int k = 0; k < (int) model.molecule[ j ].atom.size (); ++k ) 
               {
                  PDB_atom *this_atom = &( model.molecule[ j ].atom[ k ] );
            
                  this_atom->coordinate.axis[ 0 ] = out_dt[ dt_pos ].x;
                  this_atom->coordinate.axis[ 1 ] = out_dt[ dt_pos ].y;
                  this_atom->coordinate.axis[ 2 ] = out_dt[ dt_pos ].z;
                  // us_qdebug( QString( "results atom %1 coords %2 %3 %4" )
                  //         .arg( dt_pos )
                  //         .arg( out_dt[ dt_pos ].x )
                  //         .arg( out_dt[ dt_pos ].y )
                  //         .arg( out_dt[ dt_pos ].z )
                  //         );

                  ++dt_pos;
               }
            }
         }
      }
   }

   return true;
}

bool US_Saxs_Util::write_model( PDB_model & model, QString filename )
{
   QFile f( filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return false;
   }

   QTextStream ts( &f );

   for ( int j = 0; j < (int) model.molecule.size (); ++j ) 
   {
      for ( int k = 0; k < (int) model.molecule[ j ].atom.size (); ++k ) 
      {
         PDB_atom *this_atom = &( model.molecule[ j ].atom[ k ] );
         ts <<
            QString("")
            .sprintf(     
                     "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                     this_atom->serial,
                     this_atom->orgName.toLatin1().data(),
                     this_atom->resName.toLatin1().data(),
                     this_atom->chainID.toLatin1().data(),
                     this_atom->resSeq.toUInt(),
                     this_atom->coordinate.axis[ 0 ],
                     this_atom->coordinate.axis[ 1 ],
                     this_atom->coordinate.axis[ 2 ],
                     this_atom->occupancy,
                     this_atom->tempFactor,
                     this_atom->element.toLatin1().data()
                          );
      }
   }
   f.close();
   return true;
}

void US_Saxs_Util::compute_tau( double drrev1,
                                double drrev2,
                                double drrev3,
                                double fconv,
                                vector < double > & tau )
{

   // char pluto;
   double ddd[3];
   // double rrr[3];
   long double ddr[3];
   long double pd[3] = { 0.0, 0.0, 0.0 };
   int a;
   // char pluto1;

   tau.resize( 7 );

   int tauflag = -1;

   // autovalori();

   // us_qdebug( QString( "compute_tau: input ev's: %1 %2 %3 fconv %4" )
   //         .arg( drrev1 )
   //         .arg( drrev2 )
   //         .arg( drrev3 )
   //         .arg( fconv ) );

   ddr[0] = drrev1;
   ddr[1] = drrev2;
   ddr[2] = drrev3;

   for (a = 0; a < 3; a++)
   {
      ddd[a] = 0.0;
      // rrr[a] = 0.0;
   }

   a = (int) 0.0;
   if ((fabs(ddr[0] - ddr[1]) / ddr[0]) < 0.01)
   {
      a = (int) 1.0;
      ddr[1] = (ddr[0] + ddr[1]) / 2;

      pd[0] = ddr[1];
      pd[1] = ddr[1];
      pd[2] = ddr[2];
      if (ddr[2] <= ddr[1])
         a = (int) 2.0;
   }

   if ((fabs(ddr[0] - ddr[2]) / ddr[0]) < 0.01)
   {
      a = (int) 1.0;
      ddr[2] = (ddr[0] + ddr[2]) / 2;

      pd[0] = ddr[2];
      pd[1] = ddr[2];
      pd[2] = ddr[1];
      if (ddr[1] <= ddr[2])
         a = (int) 2.0;
   }

   if ((fabs(ddr[1] - ddr[2]) / ddr[1]) < 0.01)
   {
      a = (int) 1.0;
      ddr[1] = (ddr[1] + ddr[2]) / 2;

      pd[0] = ddr[1];
      pd[1] = ddr[1];
      pd[2] = ddr[0];
      if (ddr[0] <= ddr[1])
         a = (int) 2.0;

      /*      printf("\nCaso 3, valore a= %d\n",a);
              scanf("%s",&pluto1);
              getchar();     */
   }

   if (a == 1.0)
   {
      tau[0] = 6.0 * pd[0];
      tau[1] = 5.0 * pd[0] + pd[2];
      tau[2] = tau[1];
      tau[3] = 2.0 * pd[0] + 4.0 * pd[2];
      tau[4] = tau[3];
      tauflag = 1;
      tau[5] = (tau[0] + (2 * tau[1]) + (2 * tau[3])) / 5;
      tau[5] = 1.0 / tau[5];
      for (a = 0; a < 5; a++)
      {
         tau[a] = tau[a]; // * 1E-09;
      }
      tau[6] = ((1.0 / tau[0]) + (2 * (1.0 / tau[1])) + (2 * (1.0 / tau[3]))) / 5;
   }

   if (a == 2.0)
   {
      tau[4] = 6.0 * pd[0];
      tau[1] = 5.0 * pd[0] + pd[2];
      tau[2] = tau[1];
      tau[3] = 2.0 * pd[0] + 4.0 * pd[2];
      tau[0] = tau[3];
      tauflag = 2;
      tau[5] = ((2 * tau[1]) + (2 * tau[3]) + tau[4]) / 5;
      tau[5] = 1.0 / tau[5];
      for (a = 0; a < 5; a++)
      {
         tau[a] = tau[a]; // * 1E-09;
      }
      tau[6] = ((2 * (1.0 / tau[1])) + (2 * (1.0 / tau[3])) + (1.0 / tau[4])) / 5;
   }

   if (a == 0.0)

   {
      /*      printf("\nCaso 4, valore a= %d\n",a);
              scanf("%s",&pluto1);
              getchar();      */

      ddd[0] = ddr[0];
      ddd[1] = ddr[1];
      ddd[2] = ddr[2];

      if ((ddr[2] > ddr[1]) && (ddr[2] > ddr[0]))
      {
         if (ddr[1] > ddr[0])
         {
            ddr[2] = ddd[2];
            ddr[1] = ddd[1];
            ddr[0] = ddd[0];
         }
         else
         {
            ddr[2] = ddd[2];
            ddr[1] = ddd[0];
            ddr[0] = ddd[1];
         }
      }

      else if ((ddr[1] > ddr[2]) && (ddr[1] > ddr[0]))
      {
         if (ddr[2] > ddr[0])
         {
            ddr[2] = ddd[1];
            ddr[1] = ddd[2];
            ddr[0] = ddd[0];
         }
         else
         {
            ddr[2] = ddd[1];
            ddr[1] = ddd[0];
            ddr[0] = ddd[2];
         }
      }

      else
      {
         if (ddr[2] > ddr[1])
         {
            ddr[2] = ddd[0];
            ddr[1] = ddd[2];
            ddr[0] = ddd[1];
         }
         else
         {
            ddr[2] = ddd[0];
            ddr[1] = ddd[1];
            ddr[0] = ddd[2];
         }
      }
      pd[0] = ddr[0] * ddr[0] + ddr[1] * ddr[1] + ddr[2] * ddr[2] - ddr[0] * ddr[1] - ddr[0] * ddr[2] - ddr[1] * ddr[2];
      pd[0] = pow(pd[0], (long double).5);
      pd[1] = (ddr[0] + ddr[1] + ddr[2]) / 3.0;
      tau[0] = 6.0 * pd[1] - 2.0 * pd[0];
      tau[1] = 3.0 * (pd[1] + ddr[1]);
      tau[2] = 3.0 * (pd[1] + ddr[0]);
      tau[3] = 3.0 * (pd[1] + ddr[2]);
      tau[4] = 6.0 * pd[1] + 2.0 * pd[0];
      tauflag = 0.0;

      tau[6] = 0.0;
      tau[5] = 0.0;

      for (a = 0; a < 5; a++)
      {
         tau[5] += tau[a];
         tau[a] = tau[a]; // * 1E-09;
         tau[6] += 1.0 / tau[a];
      }
      tau[5] = tau[5] / 5.0;
      tau[5] = 1.0 / tau[5];
      tau[6] = tau[6] / 5.0;
   }

   for (a = 0; a < 5; a++)
   {
      tau[a] = 1e0 / tau[a];
   }

   vector < double > save_tau;
   save_tau = tau;

   switch( tauflag )
   {
   case 1 :
      tau[ 2 ] = tau[ 1 ];
      tau[ 4 ] = tau[ 3 ];
      break;
   case 2 :
      tau[ 0 ] = tau[ 4 ];
      tau[ 2 ] = tau[ 1 ];
      tau[ 4 ] = tau[ 3 ];
      break;
   default : 
      break;
   }
      
   for ( int i = 0; i < (int) tau.size(); ++i )
   {
      tau[ i ] *= pow( fconv, 3e0 ) * 1e9;
   }

   // for ( int i = 0; i < (int) tau.size(); ++i )
   // {
   //    us_qdebug( QString( "compute_tau: tau results %1 %2" ).arg( i+1 ).arg( tau[ i ] ) );
   // }
}
