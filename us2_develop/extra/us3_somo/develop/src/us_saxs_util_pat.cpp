#include "../include/us_saxs_util.h"
#include "../include/us_hydrodyn_pat.h"
//Added by qt3to4:
#include <Q3TextStream>

bool US_Saxs_Util::pat_model( PDB_model & model )
{
   if ( !model.molecule.size() )
   {
      return false;
   }

   // run pat? i.e. do a principal axis transformation

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

            // qDebug( QString( "atom %1 coords %2 %3 %4 radius %5 mw %6" )
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
               // qDebug( QString( "results atom %1 coords %2 %3 %4" )
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

   return true;
}

bool US_Saxs_Util::write_model( PDB_model & model, QString filename )
{
   QFile f( filename );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      return false;
   }

   Q3TextStream ts( &f );

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
                     this_atom->orgName.ascii(),
                     this_atom->resName.ascii(),
                     this_atom->chainID.ascii(),
                     this_atom->resSeq.toUInt(),
                     this_atom->coordinate.axis[ 0 ],
                     this_atom->coordinate.axis[ 1 ],
                     this_atom->coordinate.axis[ 2 ],
                     this_atom->occupancy,
                     this_atom->tempFactor,
                     this_atom->element.ascii()
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

   // qDebug( QString( "compute_tau: input ev's: %1 %2 %3 fconv %4" )
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
   //    qDebug( QString( "compute_tau: tau results %1 %2" ).arg( i+1 ).arg( tau[ i ] ) );
   // }
}
