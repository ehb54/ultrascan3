#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
#include "../include/us_pm.h"
#include "../include/us_timer.h"
#include <sys/time.h>

// note: this program uses cout and/or cerr and this should be replaced

static US_Timer usupm_timer;

bool US_Saxs_Util::run_pm( QString controlfile )
{
   // for now, everyone reads the control file & sets things up to the point of nsa run
   usupm_timer.init_timer ( "pm_init" );
   usupm_timer.init_timer ( "pm_run" );
   usupm_timer.start_timer ( "pm_init" );

   QString qs_base_dir = QDir::currentDirPath();

   QString outputData = QString( "%1" ).arg( getenv( "outputData" ) );
   if ( outputData.isEmpty() )
   {
      outputData = "../outputData";
   }

   int errorno = -1;

   errormsg = "";

   if ( !QFile::exists( controlfile ) )
   {
      errormsg = QString( "controlfile %1 does not exist" ).arg( controlfile );
      return false;
   }         
   errorno--;

   QStringList qslt;

   QString dest = controlfile;
   int result;

   // copy here
   QDir qd1 = QDir::current();
   qd1.convertToAbs();
   QDir qd2( QFileInfo( controlfile ).dir( true ) );
   
   US_File_Util ufu;

   if ( qd1 != qd2 )
   {
      ufu.copy( controlfile, QDir::currentDirPath() + QDir::separator() + QFileInfo( controlfile ).fileName() );
      cout << QString( "copying %1 %2 <%3>\n" )
         .arg( controlfile )
         .arg( QDir::currentDirPath() + QDir::separator() + QFileInfo( controlfile ).fileName() )
         .arg( ufu.errormsg );
      dest = QFileInfo( controlfile ).fileName();

      cout << QString( "dest is now %1\n" )
         .arg( dest );
   }

   bool use_tar = false;

   if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
   {
      // gunzip controlfile, must be renamed for us_gzip
      
      // rename
      dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
      QDir qd;
      qd.remove( dest );
      if ( !qd.rename( controlfile, dest ) )
      {
         errormsg = QString("Error renaming %1 to %2\n").arg( controlfile ).arg( dest );
         return false;
      }
      errorno--;
      
      controlfile = dest;

      US_Gzip usg;
      result = usg.gunzip( controlfile );
      if ( GZIP_OK != result )
      {
         errormsg = QString("Error: %1 problem gunzipping (%2)\n").arg( controlfile ).arg( usg.explain( result ) );
         return false;
      }
      errorno--;

      controlfile.replace( QRegExp( "\\.gz$" ), "" );
   }

   QStringList qsl;

   if ( controlfile.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      use_tar = true;
      // tar open controlfile
      US_Tar ust;
      result = ust.extract( controlfile, &qslt );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)\n").arg( controlfile ).arg( ust.explain( result ) );
         return false;
      }
      if ( !ufu.read( qslt[ 0 ], qsl ) )
      {
         errormsg = QString("Error: %1").arg( ufu.errormsg );
         return false;
      }
   } else {
      // open controlfile as regular text file contain commands
      if ( !ufu.read( controlfile, qsl ) )
      {
         errormsg = QString("Error: %1").arg( ufu.errormsg );
         return false;
      }
   }      

   cout << QString( "control file contains:\n%1\n-------\n" ).arg( qsl.join( "\n" ) ) << endl << flush;
   usupm_timer.end_timer ( "pm_init" );
   usupm_timer.start_timer ( "pm_run" );

   if ( !run_pm( qsl ) )
   {
      cout << errormsg << endl;
   }

   usupm_timer.end_timer( "pm_run" );

   cout << 
      QString( "my output files:<%1>\ncontrolfile %2\n" )
      .arg( output_files.join( ":" ) ) 
      .arg( controlfile )
        << flush;

   QFile f( "runinfo" );
   if ( f.open( IO_WriteOnly ) )
   {
      QTextStream ts( &f );
      ts << "timings:\n";
      ts << usupm_timer.list_times();;
      ts << "end-timings\n";
      QFile fc( controlfile );
      if ( fc.open( IO_ReadOnly ) )
      {
         QTextStream tsc( &fc );
         ts << "controlfile:\n";
         while( !tsc.atEnd() )
         {
            ts << tsc.readLine() << endl;
         }
         ts << "end-controlfile\n";
         fc.close();
      }
      f.close();
      output_files << "runinfo";
   } else {
      cout << "Warning: could not create timings\n" << flush;
   }

   if ( use_tar )
   {
      // package output
      if ( !create_tgz_output( controlfile + "_out.tgz" ) )
      {
         return false;
      }
   }

   return true;
}

bool US_Saxs_Util::run_pm( QStringList qsl_commands )
{
   output_files          .clear();
   job_output_files      .clear();
   write_output_count    = 0;
   timings               = "";

   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );

   QRegExp rx_valid  ( 
                      "^("
                      "pmgridsize|"
                      "pmharmonics|"
                      "pmmaxdimension|"

                      "pmrayleighdrho|"

                      "pmmemory|"
                      "pmdebug|"
                      "pmoutname|"

                      "pmf|"
                      "pmq|"
                      "pmi|"
                      "pme|"

                      "pmgapopulation|"
                      "pmgagenerations|"
                      "pmgamutate|"
                      "pmgacrossover|"
                      "pmgaelitism|"
                      "pmgaearlytermination|"
                      "pmgapointsmax|"

                      "pmbestmd0stepstoga|"
                      "pmbestmd0|"

                      // these are for grid size range
                      "pmbestfinestconversion|"
                      "pmbestcoarseconversion|"
                      "pmbestrefinementrangepct|"
                      "pmbestconversiondivisor|"

                      // these are for parameter search deltas in the current grid size
                      "pmbestdeltastart|"
                      "pmbestdeltadivisor|"
                      "pmbestdeltamin|"

                      "pmtypes|"
                      "pmparams"
                      ")$" 
                      );

   QRegExp rx_arg    ( 
                      "^("
                      "pmgridsize|"
                      "pmharmonics|"
                      "pmmaxdimension|"

                      "pmrayleighdrho|"

                      "pmmemory|"
                      "pmdebug|"
                      "pmoutname|"

                      "pmf|"
                      "pmq|"
                      "pmi|"
                      "pme|"

                      "pmgapopulation|"
                      "pmgagenerations|"
                      "pmgamutate|"
                      "pmgacrossover|"
                      "pmgaelitism|"
                      "pmgaearlytermination|"
                      "pmgapointsmax|"

                      "pmbestmd0stepstoga|"

                      "pmbestfinestconversion|"
                      "pmbestcoarseconversion|"
                      "pmbestrefinementrangepct|"
                      "pmbestconversiondivisor|"

                      "pmbestdeltastart|"
                      "pmbestdeltadivisor|"
                      "pmbestdeltamin|"

                      "pmtypes|"
                      "pmparams"
                      ")$" 
                      );

   QRegExp rx_vector ( 
                      "^("
                      "pmf|"
                      "pmq|"
                      "pmi|"
                      "pme|"

                      "pmtypes|"
                      "pmparams"
                      ")$" 
                      );


   vector < double > params;
   set < pm_point >  model;

   for ( int i = 0; i < (int) qsl_commands.size(); i++ )
   {
      QString qs = qsl_commands[ i ];
      qs.replace( rx_comment, "" ).replace( "^\\s+", "" ).replace( "\\s+$", "" );
      
      if ( qs.contains( rx_blank ) )
      {
         continue;
      }

      QStringList qsl = QStringList::split( QRegExp("\\s+"), qs );

      if ( !qsl.size() )
      {
         continue;
      }

      QString option = qsl[ 0 ].lower();
      qsl.pop_front();

      if ( rx_valid.search( option ) == -1 )
      {
         errormsg = QString( "Error controlfile line %1 : Unrecognized token %2" )
            .arg( i + 1 )
            .arg( qsl[ 0 ] );
         return false;
      }

      if ( rx_arg.search( option ) != -1 && 
           qsl.size() < 1 )
      {
         errormsg = QString( "Error reading controlfile line %1 : Missing argument " )
            .arg( i + 1 );
         return false;
      }

      if ( rx_arg.search( option ) != -1 )
      {
         control_parameters[ option ] = qsl.join( " " );
      }

      if ( rx_vector.search( option ) != -1 )
      {
         control_vectors[ option ].clear();
         cout << QString( "qsl currently: %1\n" ).arg( qsl.join( "~" ) );
         for ( int j = 0; j < (int) qsl.size(); j++ )
         {
            QStringList qsl2 = QStringList::split( QRegExp( "(\\s+|(\\s*(,|:)\\s*))" ), qsl[ j ] );
            cout << QString( "qsl2 currently: %1\n" ).arg( qsl2.join( "~" ) );
            for ( int k = 0; k < (int) qsl2.size(); k++ )
            {
               control_vectors[ option ].push_back( qsl2[ k ].toDouble() );
            }
         }
         US_Vector::printvector( option, control_vectors[ option ] );
      }

      if ( option == "pmrayleighdrho" )
      {
         if ( !control_vectors.count( "pmq" ) )
         {
            errormsg = "pmq must be defined before pmrayleighdro";
            return false;
         }
         if ( !control_parameters.count( "pmgridsize" ) )
         {
            errormsg = "pmgridsize must be defined before pmrayleighdro";
            return false;
         }

         if ( !compute_rayleigh_structure_factors( 
                                                  pow( pow( control_parameters[ "pmgridsize" ].toDouble(), 3e0 ) / M_PI, 1e0/3e0 ),
                                                  control_parameters[ "pmrayleighdrho" ].toDouble(),
                                                  control_vectors   [ "pmq" ],
                                                  control_vectors   [ "pmf" ] ) )
         {
            errormsg = QString( "Error controlfile line %1 : %2" ).arg( i + 1 ).arg( errormsg );
            return false;
         }
         cout << "Rayleigh structure factors computed\n";
         
         US_Vector::printvector2( "pmq pmf", control_vectors[ "pmq" ], control_vectors[ "pmf" ] );
         continue;
      }                                                  

      if ( option == "pmbestmd0" )
      {
         if ( !run_pm_ok() )
         {
            errormsg = QString( "Error controlfile line %1 : %2" ).arg( i + 1 ).arg( errormsg );
            return false;
         }            

         if ( control_vectors.count( "pmtypes" ) &&
              control_vectors[ "pmtypes" ].size() != 1 )
         {
            errormsg = QString( "Error controlfile line %1 : pmtypes must have exactly one parameter for pmbestmd0" ).arg( i + 1 );
            return false;
         }

         US_PM pm(
                  control_parameters [ "pmgridsize"     ].toDouble(),
                  control_parameters [ "pmmaxdimension" ].toInt(),
                  control_parameters [ "pmharmonics"    ].toUInt(),
                  control_vectors    [ "pmf"            ],
                  control_vectors    [ "pmq"            ],
                  control_vectors    [ "pmi"            ],
                  control_vectors    [ "pme"            ],
                  control_parameters [ "pmmemory"       ].toUInt(),
                  control_parameters [ "pmdebug"        ].toInt()
                  );

#if defined( USE_MPI )
         {

            pm.pm_workers_registered.clear();
            pm.pm_workers_busy      .clear();
            pm.pm_workers_waiting   .clear();
   
            for ( int i = 1; i < npes; ++i )
            {
               pm.pm_workers_registered.insert( i );
               pm.pm_workers_waiting   .insert( i );
            }

            pm_msg msg;
            int errorno                = -28000;
            msg.type                   = PM_NEW_PM;
            msg.flags                  = pm.use_errors ? PM_USE_ERRORS : 0;
            msg.vsize                  = (uint32_t) control_vectors[ "pmq" ].size();
            msg.grid_conversion_factor = control_parameters[ "pmgridsize" ].toDouble();
            msg.max_dimension          = (uint32_t) control_parameters[ "pmmaxdimension" ].toUInt();
            msg.max_harmonics          = (uint32_t) control_parameters[ "pmharmonics" ].toUInt();
            msg.max_mem_in_MB          = (uint32_t) control_parameters[ "pmmemory" ].toUInt();

            unsigned int tot_vsize = msg.vsize * ( pm.use_errors ? 4 : 3 );
            vector < double > d( tot_vsize );

            if ( pm.use_errors )
            {
               for ( int i = 0; i < msg.vsize; i++ )
               {
                  d[ i ]                 = control_vectors[ "pmf" ][ i ];
                  d[ msg.vsize + i ]     = control_vectors[ "pmq" ][ i ];
                  d[ 2 * msg.vsize + i ] = control_vectors[ "pmi" ][ i ];
                  d[ 3 * msg.vsize + i ] = control_vectors[ "pme" ][ i ];
               }
            } else {
               for ( int i = 0; i < msg.vsize; i++ )
               {
                  d[ i ]                 = control_vectors[ "pmf" ][ i ];
                  d[ msg.vsize + i ]     = control_vectors[ "pmq" ][ i ];
                  d[ 2 * msg.vsize + i ] = control_vectors[ "pmi" ][ i ];
               }
            }

            for ( int i = 1; i < npes; ++i )
            {
               if ( MPI_SUCCESS != MPI_Send( &msg,
                                             sizeof( pm_msg ),
                                             MPI_CHAR, 
                                             i,
                                             PM_MSG, 
                                             MPI_COMM_WORLD ) )
               {
                  cout << QString( "%1: MPI send failed in best_md0_ga() PM_NEW_PM\n" ).arg( myrank ) << flush;
                  MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
                  exit( errorno - myrank );
               }

               if ( MPI_SUCCESS != MPI_Send( &(d[0]),
                                             tot_vsize * sizeof( double ),
                                             MPI_CHAR, 
                                             i,
                                             PM_NEW_PM, 
                                             MPI_COMM_WORLD ) )
               {
                  cout << QString( "%1: MPI send failed in best_md0_ga() PM_NEW_PM_DATA\n" ).arg( myrank ) << flush;
                  MPI_Abort( MPI_COMM_WORLD, errorno - myrank );
                  exit( errorno - myrank );
               }
            }
         }
#endif
         pm.ga_set_params( 
                          control_parameters[ "pmgapopulation"       ].toUInt(),
                          control_parameters[ "pmgagenerations"      ].toUInt(),
                          control_parameters[ "pmgamutate"           ].toDouble(),
                          control_parameters[ "pmgacrossover"        ].toDouble(),
                          control_parameters[ "pmgaelitism"          ].toUInt(),
                          control_parameters[ "pmgaearlytermination" ].toUInt()
                          );

         pm.set_best_delta(
                           control_parameters[ "pmbestdeltastart"   ].toDouble(),
                           control_parameters[ "pmbestdeltadivisor" ].toDouble(),
                           control_parameters[ "pmbestdeltamin"     ].toDouble()
                           );

         if ( control_vectors.count( "pmparams" ) )
         {
            params = control_vectors[ "pmparams" ];
         } else {
            if ( control_vectors.count( "pmtypes" ) )
            {
               vector < int > types;
               for ( int i = 0; i < (int) control_vectors[ "pmtypes" ].size(); i++ )
               {
                  types.push_back( (int) control_vectors[ "pmtypes" ][ i ] );
               }
               pm.zero_params( params, types );
            } else {
               errormsg = QString( "Error controlfile line %1 : pmparams or pmtypes must be defined" ).arg( i + 1 );
               return false;
            }
         }
         if ( !pm.best_md0_ga( 
                              params,
                              model,
                              control_parameters[ "pmbestmd0stepstoga"       ].toUInt(),
                              control_parameters[ "pmgapointsmax"            ].toUInt(),
                              control_parameters[ "pmbestfinestconversion"   ].toDouble(),
                              control_parameters[ "pmbestcoarseconversion"   ].toDouble(),
                              control_parameters[ "pmbestrefinementrangepct" ].toDouble(),
                              control_parameters[ "pmbestconversiondivisor"  ].toDouble()
                              ) )
         {
            errormsg = "Error:" + pm.error_msg;
            return false;
         }

         QString outname = control_parameters[ "pmoutname" ];
         pm.write_model( outname, model, params, false );
         output_files << QString( outname + ".bead_model" );
         pm.write_I    ( outname, model, false );
         output_files << QString( outname + ".dat" );
         continue;
      }         
   }

   return true;
}

bool US_Saxs_Util::run_pm_ok() 
{
   // required params
   {
      QStringList params_required;
      params_required
         << "pmgridsize"
         ;
      
      for ( int i = 0; i < (int) params_required.size(); i++ )
      {
         if ( !control_parameters.count( params_required[ i ] ) )
         {
            errormsg = QString( "missing required parameter %1" ).arg( params_required[ i ] );
            return false;
         }
      }
   }

   // defaultable params
   {
      QStringList default_params;
      default_params 
         << "pmharmonics"
         << "pmmindelta"

         << "pmmaxdimension"

         << "pmmemory"
         << "pmdebug"
         << "pmoutname"

         << "pmgapopulation"
         << "pmgagenerations"
         << "pmgamutate"
         << "pmgacrossover"
         << "pmgaelitism"
         << "pmgaearlytermination"
         << "pmgapointsmax"

         << "pmbestmd0stepstoga"

         << "pmbestfinestconversion"
         << "pmbestcoarseconversion"
         << "pmbestrefinementrangepct"
         << "pmbestconversiondivisor"

         << "pmbestdeltastart"
         << "pmbestdeltadivisor"
         << "pmbestdeltamin"

         ;

      map < QString, QString > defaults;
      defaults[ "pmharmonics"              ] = "15";
      defaults[ "pmmindelta"               ] = "0.01";

      defaults[ "pmmaxdimension"           ] = QString( "%1" ).arg( USPM_MAX_VAL );

      defaults[ "pmdebug"                  ] = "0";
      defaults[ "pmmemory"                 ] = "1024";
      defaults[ "pmoutname"                ] = "pm_output";

      defaults[ "pmgapopulation"           ] = "100";
      defaults[ "pmgagenerations"          ] = "100";
      defaults[ "pmgamutate"               ] = "0.4e0";
      defaults[ "pmgacrossover"            ] = "0.4e0";
      defaults[ "pmgaelitism"              ] = "1";
      defaults[ "pmgaearlytermination"     ] = "5";
      defaults[ "pmgapointsmax"            ] = "100";

      defaults[ "pmbestmd0stepstoga"       ] = "1";

      defaults[ "pmbestfinestconversion"   ] = control_parameters[ "pmgridsize" ];
      defaults[ "pmbestcoarseconversion"   ] = "10e0";
      defaults[ "pmbestrefinementrangepct" ] = "5e0";
      defaults[ "pmbestconversiondivisor"  ] = "2.5e0";

      defaults[ "pmbestdeltastart"         ] = "1e0";
      defaults[ "pmbestdeltadivisor"       ] = "10e0";
      defaults[ "pmbestdeltamin"           ] = "1e-2";

      for ( int i = 0; i < (int) default_params.size(); i++ )
      {
         if ( !control_parameters.count( default_params[ i ] ) )
         {
            control_parameters[ default_params[ i ] ] = defaults[ default_params[ i ] ];
            cout << QString( "Notice: setting '%1' to default value of %2\n" )
               .arg( default_params[ i ] )
               .arg( defaults[ default_params[ i ] ] )
               ;
         }
      }
   }

   // the fixed common length vectors
   {
      QStringList qslv_required;
      qslv_required 
         << "pmi"
         << "pmq"
         << "pmf"
         ;
      
      QStringList qslv_optional;
      qslv_optional
         << "pme"
         ;
   
      int vector_size = 0;

      for ( int i = 0; i < (int) qslv_required.size(); i++ )
      {
         if ( !control_vectors.count( qslv_required[ i ] ) )
         {
            errormsg = QString( "missing required vector %1" ).arg( qslv_required[ i ] );
            return false;
         }
         if ( !i )
         {
            vector_size = (int) control_vectors[ qslv_required[ i ] ].size();
         } else {
            if ( vector_size != (int)control_vectors[ qslv_required[ i ] ].size() )
            {
               errormsg = QString( "required vector %1 incompatible size %2 vs %3" )
                  .arg( qslv_required[ i ] )
                  .arg( (int)control_vectors[ qslv_required[ i ] ].size() )
                  .arg( vector_size )
                  ;
               return false;
            }
         }
      }

      if ( vector_size < 3 )
      {
         errormsg = QString( "vector %1 size %2 less than minimum size 3" )
            .arg( qslv_required[ 0 ] )
            .arg( vector_size )
            ;
         return false;
      }

      for ( int i = 0; i < (int) qslv_optional.size(); i++ )
      {
         if ( control_vectors.count( qslv_optional[ i ] ) &&
              vector_size != (int)control_vectors[ qslv_optional[ i ] ].size() )
         {
            errormsg = QString( "optional vector %1 incompatible size %2 vs %3" )
               .arg( qslv_optional[ i ] )
               .arg( (int)control_vectors[ qslv_required[ 0 ] ].size() )
               .arg( vector_size )
               ;
            return false;
         }
      }
   }

   // the parameters/types
   {
      // only one of these required
      QStringList qslv_required;
      qslv_required 
         << "pmparams"
         << "pmtypes"
         ;

      int number_of_params_found = 0;
      for ( int i = 0; i < (int) qslv_required.size(); i++ )
      {
         if ( control_vectors.count( qslv_required[ i ] ) )
         {
            number_of_params_found++;
         }
      }
      if ( !number_of_params_found )
      {
         errormsg = QString( "missing required vector one of: %1" ).arg( qslv_required.join( "," ) );
         return false;
      }
      if ( number_of_params_found > 1 )
      {
         cout << "Warning: pmparams superceded pmtypes (both were defined)\n"; 
      }
   }

   for ( map < QString, QString >::iterator it = control_parameters.begin();
         it != control_parameters.end();
         it++ )
   {
      cout << QString( "\"%1\"\t%2\t%3\n" ).arg( it->first ).arg( it->second ).arg( it->second.toDouble() );
   }

   return true;
}
         
