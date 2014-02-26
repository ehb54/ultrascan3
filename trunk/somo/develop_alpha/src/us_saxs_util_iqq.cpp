#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <Q3TextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#if defined( USE_MPI )
    extern int myrank;
#endif
#if defined( CUDA )
#   include "us_cuda.h"
#endif

bool US_Saxs_Util::read_control( QString controlfile )
{
   output_files          .clear();
   job_output_files      .clear();
   saxs_inputfile_for_csv.clear();
   saxs_model_for_csv    .clear();
   saxs_method_for_csv   .clear();
   saxs_q_for_csv        .clear();
   saxs_I_for_csv        .clear();
   file_write_count      .clear();
   write_output_count    = 0;
   timings               = "";
   
   env_ultrascan = getenv( "ULTRASCAN" );
#if !defined( USE_MPI )
   cout << "$ULTRASCAN = " << env_ultrascan << endl;
#else
   if ( !nsa_mpi || !myrank )
   {
      cout << "$ULTRASCAN = " << env_ultrascan << endl << flush;
   }
#endif

   QFile f( controlfile );
   errormsg = "";
   if ( !f.exists() )
   {
      errormsg = QString( "Error: %1 file does not exist" ).arg( controlfile );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
      return false;
   }

   if ( controlfile.contains( QRegExp( "\\.(tgz|TGZ)$" ) ) )
   {
      f.close();
      US_Gzip usg;
      // rename
      QString dest = controlfile;
      dest.replace( QRegExp( "\\.(tgz|TGZ)$" ), ".tar.gz" );
      QDir qd;
      qd.remove( dest );
      
      if ( !qd.rename( controlfile, dest ) )
      {
         errormsg = QString("Error: renaming %1 to %2 ").arg( controlfile ).arg( dest );
         return false;
      }
         
      controlfile = dest;
      
      qd.remove( dest.replace( QRegExp("\\.(gz|GZ)$"), "" ) );
      cout << "controlfile sent to gunzip: " << controlfile << endl;
      int result = usg.gunzip( controlfile );
      cout << "last_written_name from gunzip: " << usg.last_written_name << endl;
      if ( GZIP_OK != result )
      {
         errormsg = QString("Error: %1 problem ungzipping (%2)").arg( controlfile ).arg( usg.explain( result ) );
         return false;
      }

      controlfile = usg.last_written_name;

      cout << "controlfile is now " << controlfile << endl;
      controlfile = QFileInfo( controlfile ).fileName();
      f.setName( controlfile );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   if ( controlfile.contains( QRegExp( "\\.(tar|TAR)$" ) ) )
   {
      f.close();
      cout << "extracting tar archive\n";
      US_Tar ust;
      QStringList qslt;
      int result = ust.list( controlfile, qslt, true );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      result = ust.extract( controlfile );
      if ( TAR_OK != result )
      {
         errormsg = QString("Error: %1 problem extracting tar archive (%2)").arg( controlfile ).arg( result );
         return false;
      }
      if ( !qslt.size() )
      {
         errormsg = QString("Error: tar archive %1 appears empty").arg( controlfile );
         return false;
      }
         
      cout << QString( "from tar file %1, new control file %2\n" ).arg( controlfile ).arg( qslt[ 0 ] );
      controlfile = qslt[ 0 ];

      f.setName( controlfile );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   // read and setup control

   Q3TextStream ts( &f );
   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );
   QRegExp rx_valid  ( 
                      "^("
                      "residuefile|"
                      "atomfile|"
                      "hybridfile|"
                      "hydrationfile|"
                      "pbruleon|"
                      "pdbmissingatoms|"
                      "pdbmissingresidues|"
                      "asahydratethresh|"
                      "asathreshpct|"
                      "asahydrateproberadius|"
                      "asacalculation|"
                      "asastep|"
                      "hydrate|"
                      "hydratealt|"
                      "hydraterevasa|"
                      "saxsfile|"
                      "fffile|"
                      "testingff|"
                      "hydrationscd|"
                      "hydrationrscd|"
                      "saxs|"
                      "iqmethod|"
                      "iqcuda|"
                      "cudareset|"
                      "fdbinsize|"
                      "fdmodulation|"
                      "hypoints|"
                      "crysolharm|"
                      "crysolgrid|"
                      "crysolchs|"
                      "explicith|"
                      "wateredensity|"
                      "targetedensity|"
                      "swhexclvol|"
                      "scaleexclvol|"
                      "startq|"
                      "endq|"
                      "deltaq|"
                      "pdballmodels|"
                      "experimentgrid|"
                      "exactq|"
                      "additionalexperimentgrid|"
                      "inputfile|"
                      "inputfilenoread|"
                      "tag|"
                      "output|"
                      "outputfile|"
                      "process|"
                      "taroutput|"
                      "tgzoutput|"
                      "dmdsupportfile|"
                      "dmdboxspacing|"
                      "dmdfindss|"
                      "dmdprepare|"
                      "dmdstrippdb|"
                      "dmdtime|"
                      "dmdtemp|"
                      "dmdtimestep|"
                      "dmdrun|"
                      "dmdrelaxheatxc|"
                      "dmdequiheatxc|"
                      "dmdstatic|"

                      "gridsearch|"

                      "sgppopulation|"
                      "sgpgenerations|"
                      "sgpearlytermination|"
                      "sgpremoveduplicates|"
                      "sgpelitism|"
                      "sgpcrossover|"
                      "sgpmutate|"
                      "sgpplague|"
                      "sgpdistancequantum|"
                      "sgpdistancemin|"
                      "sgpdistancemax|"
                      "sgpradiusmin|"
                      "sgpradiusmax|"
                      "sgpbranchmax|"
                      "sgprandomseed|"
                      "sgprun|"
                      "sgptest|"

                      "fivetermgaussian|"

                      "nsaspheres|"
                      "nsaess|"
                      "nsaexcl|"
                      "bfnb_nsarun|"
                      "nsaga|"
                      "nsasga|"
                      "nsaiterations|"
                      "nsaepsilon|"
                      "nsagsm|"
                      "nsascale|"
                      "nsagainit|"

                      "nsaga|"
                      "nsagenerations|"
                      "nsapopulation|"
                      "nsaelitism|"
                      "nsamutate|"
                      "nsacrossover|"
                      "nsaearlytermination|"

                      "nsasgaincrement|"

                      "bsplinetest|"

                      "sleep|"

                      "damminrun|"
                      "dammingnomfile|"
                      "damminmode|"
                      "damminname|"
                      "dammindescription|"
                      "damminangularunits|"
                      "damminfitcurvelimit|"
                      "damminknotstofit|"
                      "damminconstantsubtractionprocedure|"
                      "damminmaxharmonics|"
                      "dammininitialdamtype|"
                      "damminsymmetry|"
                      "damminspherediameter|"
                      "damminpackingradius|"
                      "damminradius1stcoordinationsphere|"
                      "damminloosenesspenaltyweight|"
                      "dammindisconnectivitypenaltyweight|"
                      "damminperipheralpenaltyweight|"
                      "damminfixingthersholdslosandrf|"
                      "damminrandomizestructure|"
                      "damminweight|"
                      "dammininitialscalefactor|"
                      "damminfixscalefactor|"
                      "dammininitialannealingtemperature|"
                      "damminannealingschedulefactor|"
                      "damminnumberofindependentatomstomodify|"
                      "damminmaxnumberiterationseacht|"
                      "damminmaxnumbersuccesseseacht|"
                      "damminminnumbersuccessestocontinue|"
                      "damminmaxnumberannealingsteps|"
                      "damminexpectedshape|"

                      "crysolrun|"
                      "crysolpdb|"
                      "crysolmode|"
                      "crysolharmonics|"
                      "crysolfibgrid|"
                      "crysolmaxq|"
                      "crysolpoints|"
                      "crysolexplicithydrogens|"
                      "crysolfitexperiment|"
                      "crysoledensity|"
                      "crysolcontrast|"
                      "crysolatomicradius|"
                      "crysolexcludedvolume|"

                      "a2sbrun|"
                      "a2sbcubeside|"
                      "a2sbequalize|"
                      "a2sbcubesize|"

                      "1dlambda|"
                      "1ddetectordistance|"
                      "1ddetectorwidth|"
                      "1ddetectorpixelswidth|"
                      "1drho0|"
                      "1ddeltar|"
                      "1dproberadius|"
                      "1dthreshold|"
                      "1dsamplerotations|"
                      "1drotationsuserand|"
                      "1daxisrotations|"
                      "1drotationfile|"
                      "1drun|"
                      "onedrun|"
                      "1dintermediatesaves|"
                      "1dspecmult|"

                      "bestmsrprober|"
                      "bestmsrfinenessangle|"
                      "bestmsrmaxtriangles|"
                      "bestmsrcoalescer|"
                      "bestrcoalnmin|"
                      "bestrcoalnmax|"
                      "bestrcoaln|"
                      "bestbestwatr|"
                      "bestbestna|"
                      "bestbestp|"
                      "bestbestv|"
                      "bestbestvc|"
                      "bestbestmw|"
                      "bestbestmw|"
                      "bestmsrradiifile|"
                      "bestmsrpatternfile|"
                      "bestexpand|"
                      "bestrun|"

                      "useiqtargetev|"
                      "setiqtargetevfromvbar|"
                      "hybridradiusexclvol|"
                      "iqtargetev|"

                      "prbinsize|"
                      "prcurve|"

                      "c2check|"
                      "c2checkcaonly|"

                      "remark)$"
                      );

   QRegExp rx_file   ( 
                      "^("
                      "dammingnomfile|"
                      "residuefile|"
                      "atomfile|"
                      "hybridfile|"
                      "hydrationfile|"
                      "saxsfile|"
                      "fffile|"
                      "dammingnomfile|"
                      "experimentgrid|"
                      "additionalexperimentgrid|"
                      "dmdsupportfile|"
                      "1drotationfile|"
                      "bestmsrradiifile|"
                      "bestmsrpatternfile|"
                      "inputfilenoread|"
                      "inputfile)$"
                      );

   QRegExp rx_arg_1  ( 
                      "^("
                      "residuefile|"
                      "atomfile|"
                      "hybridfile|"
                      "hydrationfile|"
                      "pdbmissingatoms|"
                      "pdbmissingresidues|"
                      "asahydratethresh|"
                      "asathreshpct|"
                      "asahydrateproberadius|"
                      "asacalculation|"
                      "asastep|"
                      "saxsfile|"
                      "fffile|"
                      "hydrationscd|"
                      "hydrationrscd|"
                      "iqmethod|"
                      "fdbinsize|"
                      "fdmodulation|"
                      "hypoints|"
                      "crysolharm|"
                      "crysolgrid|"
                      "crysolchs|"
                      "wateredensity|"
                      "targetedensity|"
                      "swhexclvol|"
                      "scaleexclvol|"
                      "startq|"
                      "endq|"
                      "deltaq|"
                      "experimentgrid|"
                      "additionalexperimentgrid|"
                      "inputfile|"
                      "inputfilenoread|"
                      "tag|"
                      "output|"
                      "taroutput|"
                      "tgzoutput|"
                      "dmdboxspacing|"
                      "dmdsupportfile|"
                      "dmdtime|"
                      "dmdtemp|"
                      "dmdtimestep|"
                      "dmdrun|"
                      "dmdrelaxheatxc|"
                      "dmdequiheatxc|"
                      "dmdstatic|"

                      "sleep|"

                      "sgppopulation|"
                      "sgpgenerations|"
                      "sgpearlytermination|"
                      "sgpelitism|"
                      "sgpcrossover|"
                      "sgpmutate|"
                      "sgpplague|"
                      "sgpdistancequantum|"
                      "sgpdistancemin|"
                      "sgpdistancemax|"
                      "sgpradiusmin|"
                      "sgpradiusmax|"
                      "sgpbranchmax|"
                      "sgprandomseed|"

                      "nsaspheres|"
                      "nsaiterations|"
                      "nsaepsilon|"
                      "nsagsm|"

                      "nsagenerations|"
                      "nsapopulation|"
                      "nsaelitism|"
                      "nsamutate|"
                      "nsacrossover|"
                      "nsaearlytermination|"

                      "nsasgaincrement|"

                      "a2sbcubeside|"

                      "crysolpdb|"

                      "1dlambda|"
                      "1ddetectordistance|"
                      "1ddetectorwidth|"
                      "1ddetectorpixelswidth|"
                      "1drho0|"
                      "1ddeltar|"
                      "1dproberadius|"
                      "1dthreshold|"
                      "1dsamplerotations|"
                      "1daxisrotations|"
                      "1drotationfile|"
                      "1dintermediatesaves|"

                      "bestmsrprober|"
                      "bestmsrfinenessangle|"
                      "bestmsrcoalescer|"
                      "bestmsrmaxtriangles|"
                      "bestrcoalnmin|"
                      "bestrcoalnmax|"
                      "bestrcoaln|"
                      "bestbestwatr|"
                      "bestbestna|"
                      "bestbestp|"
                      "bestbestv|"
                      "bestbestvc|"
                      "bestbestmw|"
                      "bestbestmw|"
                      "bestmsrradiifile|"
                      "bestmsrpatternfile|"
                      "bestexpand|"

                      "iqtargetev|"

                      "prbinsize|"
                      "prcurve|"

                      "c2check|"

                      "outputfile)$"
                      );

   QRegExp rx_arg_2  ( 
                      "^("
                      "c2check)$"
                       );


   QRegExp rx_valid_saxs_iqmethod (
                                   "^("
                                   "db|"
                                   "hy|"
                                   "hya|"
                                   "h2|"
                                   "h2a|"
                                   "h3|"
                                   "h3a|"
                                   "foxs|"
                                   "crysol|"
                                   "fd)$"
                                   );

   QRegExp rx_flush  ( 
                      "^("
                      // "experimentgrid|"
                      // "startq|"
                      // "endq|"
                      // "deltaq)$"
                      "noflush)$"
                      );

   unsigned int line = 0;

   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine().replace( rx_comment, "" ).replace( "^\\s+", "" ).replace( "\\s+$", "" );

      line++;

      if ( qs.contains( rx_blank ) )
      {
         continue;
      }

      QStringList qsl = QStringList::split( QRegExp("\\s+"), qs );

      if ( !qsl.size() )
      {
         continue;
      }

      if ( rx_valid.search( qsl[ 0 ].lower() ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( controlfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return false;
      }

      QString option = qsl[ 0 ].lower();
      qsl.pop_front();
      control_parameters[ option ] = qsl.join(" ");

      if ( rx_arg_1.search( option ) != -1 && 
           qsl.size() < 1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }

      if ( rx_arg_2.search( option ) != -1 && 
           qsl.size() < 2 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }


      if ( rx_file.search( option ) != -1 )
      {
         QFile qfc( qsl[ 0 ] );
         if ( !qfc.exists() )
         {
            errormsg = QString( "Error reading %1 line %2 : File %3 does not exist " )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
      }         

      if ( rx_flush.search( option ) != -1 )
      {
         if ( !flush_output() )
         {
            return false;
         }
      }         

      if ( option == "crysolrun" )
      {
         if ( !run_crysol() )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "damminrun" )
      {
         unsigned int retries = 3;
         if ( control_parameters.count( "damminrun" ) &&
              control_parameters[ "damminrun" ].toUInt() > 1 )
         {
            retries = control_parameters[ "damminrun" ].toUInt();
         }

         bool         ok    = false;
         unsigned int trial = 0;
         while( !ok && trial < retries )
         {
            trial++;
            ok = run_dammin();
            if ( !ok && trial < retries )
            {
               cout << QString( "DAMMIN: failed, retrying %1 of %2" ).arg( trial ).arg( retries );
#ifndef WIN32
               sleep( trial * trial * 60 );
#endif
            }
         }
         if ( !ok )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "sleep" )
      {
#if !defined( Q_WS_WIN ) || defined( MINGW )
         // 999999999
         double secs = qsl[ 0 ].toDouble();
         secs *= 2e0;
         // secs *= 1e-1; // finer resolution if nanosleep available
         cout << QString( "sleep %1\n" ).arg( secs ).ascii();
         timespec ns;
         timespec ns_ret;
         ns.tv_sec  = ( long ) secs;
         ns.tv_nsec = ( long ) ( 1e9 * ( secs - ns.tv_sec ) );
         cout << QString( "sleep s %1\n" ).arg( ns.tv_sec ).ascii();
         cout << QString( "sleep ns %1\n" ).arg( ns.tv_nsec ).ascii();
         nanosleep(&ns, &ns_ret);
#else
         int secs = qsl[ 0 ].toInt();
         _sleep( secs );
#endif
      }
         
      if ( option == "cudareset" )
      {
#if defined( CUDA )
         if ( !cuda_reset() )
         {
            return false;
         }
#else
         errormsg = "Error: program not compiled with CUDA capability";
         return false;
#endif
      }

#if !defined( CUDA )
      if ( option == "iqcuda" )
      {
         errormsg = "Error: program not compiled with CUDA capability";
         return false;
      }
#endif

      if ( option == "residuefile" )
      {
#if !defined( USE_MPI )
         cout << QString("read residue %1\n").arg( qsl[ 0 ] );
#endif
         if ( !select_residue_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
      }         

#if defined( CMDLINE )
      if ( option == "hydrationfile" )
      {
#if !defined( USE_MPI )
         cout << QString("read hydration %1\n").arg( qsl[ 0 ] );
#endif
         if ( !load_rotamer( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
      }

      if ( option == "c2check" )
      {
         if ( !c2check( qsl[ 0 ] , qsl[ 1 ] ) )
         {
            return false;
         }
      }

      if ( option == "hydrate" )
      {
         if ( output_dmd_pdbs.size() )
         {
            QString save_input_file = control_parameters[ "inputfile" ];
            QString save_output_file = 
               ( control_parameters.count( "outputfile" ) ?
                 control_parameters[ "outputfile" ] : "" );
               
            for ( unsigned int i = 0; i < output_dmd_pdbs.size(); i++ )
            {
               control_parameters[ "inputfile" ] = output_dmd_pdbs[ i ];
               control_parameters[ "outputfile" ] = QFileInfo( output_dmd_pdbs[ i ] ).baseName();
               misc_pb_rule_on = control_parameters.count( "pbruleon" ) != 0;

               if ( !read_pdb( control_parameters[ "inputfile" ] ) )
               {
                  return false;
               }
               if ( !pdb_hydrate() )
               {
                  errormsg = QString( "Error %1 line %2 : %3" )
                     .arg( controlfile )
                     .arg( line )
                     .arg( errormsg );
                  return false;
               }
               output_dmd_pdbs[ i ] = control_parameters[ "inputfile" ];
            }
            control_parameters[ "inputfile" ] = save_input_file;
            if ( save_output_file.isEmpty() )
            {
               control_parameters.erase( "outputfile" );
            } else {
               control_parameters[ "outputfile" ] = save_output_file;
            }
         } else {
            if ( !pdb_hydrate() )
            {
               errormsg = QString( "Error %1 line %2 : %3" )
                  .arg( controlfile )
                  .arg( line )
                  .arg( errormsg );
               return false;
            }
         }
      }
#endif

      if ( option == "atomfile" )
      {
#if !defined( USE_MPI )
         cout << QString("read atom %1\n").arg( qsl[ 0 ] );
#endif
         if ( !select_atom_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "hybridfile" )
      {
#if !defined( USE_MPI )
         cout << QString("read hybrid %1\n").arg( qsl[ 0 ] );
#endif
         if ( !select_hybrid_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }         

      if ( option == "saxsfile" )
      {
#if !defined( USE_MPI )
         cout << QString("read saxs atom %1\n").arg( qsl[ 0 ] );
#endif
         if ( !select_saxs_file( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }

      if ( option == "fffile" )
      {
#if !defined( USE_MPI )
         cout << QString("read ff table %1\n").arg( qsl[ 0 ] );
#endif
         if ( !load_ff_table( qsl[ 0 ] ) )
         {
            errormsg = QString( "Error %1 line %2 : %3" )
               .arg( controlfile )
               .arg( line )
               .arg( errormsg );
            return false;
         }
      }
      
      if ( option == "iqmethod" )
      {
         if ( rx_valid_saxs_iqmethod.search( qsl[ 0 ] ) == -1 )
         {
            errormsg = QString( "Error %1 line %2 : invalid %3 %4" )
               .arg( controlfile )
               .arg( line )
               .arg( option )
               .arg( qsl[ 0 ] );
            return false;
         }
      }            

      if ( option == "experimentgrid" )
      {
         experimental_grids.clear();
         experimental_grids << qsl[ 0 ];
         if ( !set_control_parameters_from_experiment_file( qsl[ 0 ] ) )
         {
            return false;
         }
      }            

      if ( option == "additionalexperimentgrid" )
      {
         experimental_grids << qsl[ 0 ];
      }

      if ( option == "inputfile" )
      {
         cout << "inputfile\n" << flush;
         // restating inputfile clears the previous dmd run files
         output_dmd_pdbs.clear();
         // read pdb, needs residue file
         QString ext = QFileInfo( qsl[ 0 ] ).extension( false ).lower();

         if ( ext != "pdb" )
         {
            errormsg = QString( "Error %1 line %2 : only PDB files currently supported <%3>" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }

         misc_pb_rule_on = control_parameters.count( "pbruleon" ) != 0;

         cout << "inputfile 0\n" << flush;
         setup_saxs_options();
         cout << "inputfile 1\n" << flush;

         if ( !read_pdb( qsl[ 0 ] ) )
         {
            return false;
         }
         cout << "inputfile 2\n" << flush;
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg << endl;
         }
         if ( model_vector.size() > 1 &&
              !control_parameters.count( "pdballmodels" ) )
         {
            cout << "Notice: an NMR style model was loaded, but \"PDBAllModels\" was not selected, so only the first model will be loaded\n";
            model_vector.resize( 1 );
            model_vector_as_loaded = model_vector;
         }
      }

      if ( option == "nsagainit" )
      {
         // each of these will be a list of numbers, which we will use for initialization if the size matches
         if ( !qsl.size() )
         {
            errormsg = "nsagainit requires arguments";
            return false;
         }

         nsa_ga_individual individual;
         for ( int i = 0; i < ( int )qsl.size(); i++ )
         {
            individual.v.push_back( qsl[ i ].toDouble() );
         }
         nsa_ga_inits[ (unsigned int) qsl.size() ].push_back( individual );
      }

      if ( option == "taroutput" )
      {
#if defined( USE_MPI )
         if ( !nsa_mpi || !myrank )
         {
#endif
         flush_output();
         if ( !output_files.size() )
         {
            errormsg = QString( "Error %1 line %2 : no output files available to collect in tar file %3" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
         if ( !create_tar_output( qsl[ 0 ] ) )
         {
            return false;
         }
#if defined( USE_MPI )
         }
#endif
      }

      if ( option == "tgzoutput" )
      {
#if defined( USE_MPI )
         if ( !nsa_mpi || !myrank )
         {
#endif
         flush_output();
         if ( !output_files.size() )
         {
            errormsg = QString( "Error %1 line %2 : no output files available to collect in gzipped tar file %3" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
            return false;
         }
         if ( !create_tgz_output( qsl[ 0 ] ) )
         {
            return false;
         }
#if defined( USE_MPI )
         }
#endif
      }

      if ( option == "dmdstrippdb" )
      {
         if ( !dmd_strip_pdb() )
         {
            return false;
         }
      }

      if ( option == "dmdfindss" )
      {
         if ( !dmd_findSS() )
         {
            return false;
         }
      }

      if ( option == "dmdprepare" )
      {
         if ( !dmd_prepare() )
         {
            return false;
         }
      }

      if ( option == "dmdrun" )
      {
         if ( !dmd_run( qsl[0] ) )
         {
            return false;
         }
      }

      if ( option == "sgprun" )
      {
         if ( !sgp_run() )
         {
            return false;
         }
      }

      if ( option == "bfnb_nsarun" )
      {
         if ( !nsa_run() )
         {
            return false;
         }
#if defined( USE_MPI )
         if ( !myrank ) 
         {
            extern bool       timed_out;
            if ( timed_out )
            {
               QFile f( QString( "timeout-%1" ).arg( myrank ) );
               if( f.open( QIODevice::WriteOnly | QIODevice::Append ) )
               {
                  Q3TextStream ts( &f );
                  ts << QString( "%1: Timed out\n" ).arg( myrank ) << flush;
                  f.close();
                  output_files << f.name();
               }
            }
         }
#endif    
      }

      if ( option == "nsasga" )
      {
         if ( !nsa_sga_run() )
         {
            return false;
         }
      }

      if ( option == "bsplinetest" )
      {
         if ( !bspline_test() )
         {
            return false;
         }
      }

      if ( option == "a2sbrun" )
      {
         if ( !a2sb_run() )
         {
            return false;
         }
         cout << "back from a2sbrun\n" << flush;
      }

      if ( option == "bestrun" )
      {
         //         control_parameters[ "bestmsrmaxtriangles" ] = "85000";
         if ( !run_best() )
         {
            return false;
         }
      }

      if ( option == "1drun" ||
           option == "onedrun" )
      {
         setup_saxs_options();
         if ( !read_pdb( control_parameters[ "inputfile" ] ) )
         {
            return false;
         }
#if defined( USE_MPI )
         if ( !compute_1d_mpi() )
         {
            return false;
         }
#else 
         if ( !compute_1d() )
         {
            return false;
         }
#endif
      }

      if ( option == "sgptest" )
      {
         sgp.test();
      }

      if ( option == "process" )
      {
         if ( experimental_grids.size() < 2 )
         {
            if ( !process_one_iqq() )
            {
               errormsg = QString( "Error %1 line %2 : %3" )
                  .arg( controlfile )
                  .arg( line )
                  .arg( errormsg );
               return false;
            }
         } else {
            map < QString, bool > tag_names;
            for ( unsigned int i = 0; i < (unsigned int) experimental_grids.size(); i++ )
            {
               unsigned ext = 0;
               QString grid_tag_base = "_g" + QFileInfo( experimental_grids[ i ] ).baseName();
               QString grid_tag = grid_tag_base;
               while ( tag_names.count( grid_tag ) )
               {
                  grid_tag = QString( "%1-%2" ).arg( grid_tag_base ).arg( ++ext );
               }
               control_parameters[ "grid_tag" ] = grid_tag;
               if ( !set_control_parameters_from_experiment_file( experimental_grids[ i ] ) )
               {
                  errormsg = QString( "Error %1 line %2 : %3" )
                     .arg( controlfile )
                     .arg( line )
                     .arg( errormsg );
                  return false;
               }
               if ( !process_one_iqq() )
               {
                  errormsg = QString( "Error %1 line %2 : %3" )
                     .arg( controlfile )
                     .arg( line )
                     .arg( errormsg );
                  return false;
               }
            }
         }
      }
      if ( option == "gridsearch" )
      {
         if ( experimental_grids.size() != 1 )
         {
            errormsg = QString( "Error %1 line %2 : exactly one experiment grid needed" )
               .arg( controlfile )
               .arg( line );
            return false;
         }
         if ( !control_parameters.count( "inputfile" ) )
         {
            errormsg = QString( "Error %1 line %2 : InputFile not defined" )
               .arg( controlfile )
               .arg( line );
            return false;
         }
         if ( qsl.size() != 8 )
         {
            errormsg = QString( "Error %1 line %2 : 8 parameters needed" )
               .arg( controlfile )
               .arg( line );
            return false;
         }

         if ( model_vector.size() != 1 )
         {
            errormsg = QString( "Error %1 line %2 : single model inputfile required" )
               .arg( controlfile )
               .arg( line );
            return false;
         }

         if ( model_vector[ 0 ].molecule.size() < 1 )
         {
            errormsg = QString( "Error %1 line %2 : model has no molecules!" )
               .arg( controlfile )
               .arg( line );
            return false;
         }

         unsigned int atom_num = qsl[ 0 ].toUInt();

         if ( !atom_num )
         {
            errormsg = QString( "Error %1 line %2 : selected atom numbers are relative and start with 1" )
               .arg( controlfile )
               .arg( line );
            return false;
         }

         if ( !validate_control_parameters() )
         {
            return false;
         }
         setup_saxs_options();
         cout << QString("Grid before run_iqq q(%1:%2) deltaq %3\n")
            .arg( control_parameters[ "startq" ] )
            .arg( control_parameters[ "endq" ] )
            .arg( control_parameters[ "deltaq" ] );
         
         if ( !read_pdb( control_parameters[ "inputfile" ] ) )
         {
            return false;
         }
         
         if ( ( unsigned int ) model_vector[ 0 ].molecule[ 0 ].atom.size() < atom_num )
         {
            errormsg = QString( "Error %1 line %2 : model's 1st molecule has insufficient atoms!" )
               .arg( controlfile )
               .arg( line );
            return false;
         }

         atom_num--; // start with offset 0

         double startx = qsl[ 1 ].toDouble();
         double endx   = qsl[ 2 ].toDouble();
         double starty = qsl[ 3 ].toDouble();
         double endy   = qsl[ 4 ].toDouble();
         double startz = qsl[ 5 ].toDouble();
         double endz   = qsl[ 6 ].toDouble();
         double delta  = qsl[ 7 ].toDouble();

         double x;
         double y;
         double z;

         QString org_model = model_vector[ 0 ].model_id;

         control_parameters[ "gridsearch_running" ] = "running"; 
         double best_msd = 9e99;
         // double best_x   = 0e0;
         // double best_y   = 0e0;
         // double best_z   = 0e0;

         for ( x = startx; x <= endx; x += delta )
         {
            cout << QString( "new x %1\n" ).arg( x );
            for ( y = starty; y <= endy; y += delta )
            {
               for ( z = startz; z <= endz; z += delta )
               {
                  model_vector[ 0 ].molecule[ 0 ].atom[ atom_num ].coordinate.axis[ 0 ] = x;
                  model_vector[ 0 ].molecule[ 0 ].atom[ atom_num ].coordinate.axis[ 1 ] = y;
                  model_vector[ 0 ].molecule[ 0 ].atom[ atom_num ].coordinate.axis[ 2 ] = z;
                  // need to do this silently
                  // cout << QString( "running for %1 %2 %3\n" ).arg( x ).arg( y ).arg( z ) << flush;
                  model_vector[ 0 ].model_id = org_model + QString( "p%1_%2_%3" ).arg( x ).arg( y ).arg( z );
                  if ( !run_iqq() )
                  {
                     control_parameters.erase( "gridsearch_running" );
                     return false;
                  }
                  // compute fitness and report
                  // compare sgp_exp_q, I to sgp_last_q, sgp_last_I
                  double msd = 0e0;
                  
                  for ( unsigned int i = 0; i < sgp_last_q.size(); i++ )
                  {
                     msd += ( sgp_last_I[ i ] - sgp_exp_I[ i ] ) * ( sgp_last_I[ i ] - sgp_exp_I[ i ] );
                  }
                  // cout << QString( "%1 at %2,%3,%4\n" ).arg( msd ).arg( x ).arg( y ).arg( z );
                  if ( best_msd > msd )
                  {
                     best_msd = msd;
                     // best_x = x;
                     // best_y = y;
                     // best_z = z;
                     cout << QString( "new best: %1 at %2,%3,%4\n" ).arg( msd ).arg( x ).arg( y ).arg( z );
                  }
               }
            }
         }
         control_parameters.erase( "gridsearch_running" );
      }
   }
    
   f.close();
   cout << "before flush\n" << endl;
   flush_output();
   return true;
}

bool US_Saxs_Util::process_one_iqq()
{
   if ( !validate_control_parameters() )
   {
      return false;
   }
   setup_saxs_options();
   cout << QString("Grid before run_iqq q(%1:%2) deltaq %3\n")
      .arg( control_parameters[ "startq" ] )
      .arg( control_parameters[ "endq" ] )
      .arg( control_parameters[ "deltaq" ] );
   if ( output_dmd_pdbs.size() )
   {
      QString save_input_file = control_parameters[ "inputfile" ];
      QString save_output_file = 
         ( control_parameters.count( "outputfile" ) ?
           control_parameters[ "outputfile" ] : "" );
      
      for ( unsigned int i = 0; i < (unsigned int) output_dmd_pdbs.size(); i++ )
      {
         control_parameters[ "inputfile" ] = output_dmd_pdbs[ i ];
         control_parameters[ "outputfile" ] = QFileInfo( output_dmd_pdbs[ i ] ).baseName();
         misc_pb_rule_on = control_parameters.count( "pbruleon" ) != 0;
         
         if ( !read_pdb( control_parameters[ "inputfile" ] ) )
         {
            return false;
         }
         if ( !run_iqq() )
         {
            return false;
         } 
         if ( !noticemsg.isEmpty() )
         {
            cout << noticemsg;
         }
      }
      control_parameters[ "inputfile" ] = save_input_file;
      if ( save_output_file.isEmpty() )
      {
         control_parameters.erase( "outputfile" );
      } else {
         control_parameters[ "outputfile" ] = save_output_file;
      }
   } else {
      if ( !run_iqq() )
      {
         return false;
      } 
      if ( !noticemsg.isEmpty() )
      {
         cout << noticemsg;
      }
   }
   return true;
}
   
bool US_Saxs_Util::set_control_parameters_from_experiment_file( QString filename, bool load_without_interp )
{
   errormsg = "";

   sgp_exp_q      .clear();
   sgp_exp_q_index.clear();
   sgp_exp_I      .clear();
   sgp_exp_e      .clear();

   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   
   QString ext = QFileInfo( filename ).extension( false ).lower();

   QRegExp rx_valid_ext (
                         "^("
                         "csv|"
                         "dat|"
                         "int|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.search( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   Q3TextStream ts(&f);
   vector < QString > qv;
   QStringList qsl;
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      qv.push_back( qs );
      qsl << qs;
   }
   f.close();

   if ( !qv.size() )
   {
      errormsg = QString("Error: the file %1 is empty ").arg( filename );
      return false;
   }

   vector < double > q;
   vector < double > I;
   vector < double > e;

   if ( ext == "csv" )
   {
      QStringList qsl_headers = qsl.grep("\"Name\",\"Type; q:\"");
      if ( qsl_headers.size() == 0 )
      {
         errormsg = QString("Error: the file %1 has no valid q grid lines").arg( filename );
         return false;
      }
      QStringList qsl_q = QStringList::split( ",", qsl_headers[ 0 ], true );
      if ( qsl_q.size() < 3 )
      {
         errormsg = QString("Error: the file %1 q grid line has insufficient columns").arg( filename );
         return false;
      }

      q.push_back(qsl_q[2].toDouble());

      QStringList::iterator it = qsl_q.begin();
      it += 3;
      for ( ; it != qsl_q.end(); it++ )
      {
         if ( (*it).toDouble() > q[q.size() - 1] )
         {
            q.push_back((*it).toDouble());
         } else {
            break;
         }
      }

      if ( q.size() < 2 )
      {
         errormsg = QString("Error: the file %1 q grid line has insufficient columns").arg( filename );
         return false;
      }

   } else {
      QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.search( qv[i] ) == -1 )
         {
            continue;
         }
         
         QStringList tokens = QStringList::split(QRegExp("\\s+"), qv[i].replace(QRegExp("^\\s+"),""));

         if ( tokens.size() > 1 )
         {
            double this_q = tokens[ 0 ].toDouble();
            if ( q.size() && this_q <= q[ q.size() - 1 ] )
            {
               cout << QString(" breaking %1 %2\n").arg( this_q ).arg( q[ q.size() - 1 ] );
               break;
            }
            q.push_back( this_q );
            double this_I = tokens[ 1 ].toDouble();
            I.push_back( this_I );
            if ( tokens.size() > 2  )
            {
               double this_e = tokens[ 2 ].toDouble();
               e.push_back( this_e );
            }
         }
      }

      if ( q.size() < 2 )
      {
         errormsg = QString("Error: file %1 error trying to get q grid").arg( filename );
         return false;
      }
   }

   sgp_exp_q = q;
   exact_q   = q;
   sgp_exp_I = I;
   sgp_exp_e = e;
   for ( unsigned int i = 0; i < sgp_exp_q.size(); i++ )
   {
      sgp_exp_q_index[ sgp_exp_q[ i ] ] = i;
   }

   cout << QString( "experiment grid sizes %1 %2 %3\n" ).arg( q.size() ).arg( I.size() ).arg( e.size() );

   control_parameters[ "startq" ] = QString("%1").arg( q[ 0 ] );
   control_parameters[ "endq" ]   = QString("%1").arg( q[ q.size() - 1 ] );
   control_parameters[ "deltaq" ] = QString("%1").arg( ( q[ ( q.size() - 1 ) ] - q[ 0 ] ) / ( q.size() - 1 ));

   if ( control_parameters[ "deltaq" ].toDouble() <= 0e0 )
   {
      errormsg = QString("Error: the file %1 q grid apparent deltaq is zero or negative %2").arg( filename ).arg( control_parameters[ "deltaq" ] );
      return false;
   }

   if ( load_without_interp )
   {
      return true;
   }

   // spacing computation
   double min_delta_q = q[ 1 ] - q[ 0 ];
   double max_delta_q = q[ 1 ] - q[ 0 ];
   double avg_delta_q = q[ 1 ] - q[ 0 ];

   for ( unsigned int i = 1; i < q.size() - 1; i++ )
   {
      double this_delta_q = q[ i + 1 ] - q[ i ];
      if ( this_delta_q < min_delta_q )
      {
         min_delta_q = this_delta_q;
      }
      if ( this_delta_q > max_delta_q )
      {
         max_delta_q = this_delta_q;
      }
      avg_delta_q += this_delta_q;
   }

   avg_delta_q /= q.size() - 1;

   if ( max_delta_q - min_delta_q > 2e-7 )
   {
#if defined( USE_MPI )
      if ( !nsa_mpi || !myrank )
      {
#endif
      cout << 
         QString( "Warning: the file %1 q grid appears to have a somewhat irregular grid with an average spacing of %2\n"
                  "Warning: the experimental grid has a minimum spacing of %3, max %4 and average %5\n"
                  "Warning: computations will occur on a regular grid with an equal range and number of points\n" )
         .arg( filename )
         .arg( control_parameters[ "deltaq" ] )
         .arg( min_delta_q )
         .arg( max_delta_q )
         .arg( avg_delta_q )
         ;
#if defined( USE_MPI )
      }
#endif

   }

   cout << QString("Grid from file %1 q(%2:%3) deltaq %4\n")
      .arg( filename )
      .arg( control_parameters[ "startq" ] )
      .arg( control_parameters[ "endq" ] )
      .arg( control_parameters[ "deltaq" ] );

   // put sgp_exp_I onto regular grid:
   vector < double > rq;
   for ( double d = control_parameters[ "startq" ].toDouble(); 
         d <= ( control_parameters[ "endq" ].toDouble() + 5e-4 ); 
         d += control_parameters[ "deltaq" ].toDouble() )
   {
      rq.push_back( d );
   }
   vector < double > org_I = sgp_exp_I;
   vector < double > org_e = sgp_exp_e;

   {
      unsigned int cropped_pts = 0;
      while ( rq.size() && rq.back() > sgp_exp_q.back() + 1e-6 )
      {
         cout << QString( "cropping point with value %1 which is larger than %2 by %3\n" )
            .arg( rq.back() ).arg( sgp_exp_q.back() ).arg( rq.back() - sgp_exp_q.back() ) << flush;

         cropped_pts++;
         rq.pop_back();
      }
      if ( cropped_pts )
      {
         cout << QString( "Notice: cropped %1 points off originally computed grid\n" ).arg( cropped_pts );
      }
   }

   if ( rq.back() > sgp_exp_q.back() )
   {
      rq.back() = sgp_exp_q.back();
   }

   cout << QString( "exp q [%1:%2] rq [%3:%4]\n" )
      .arg( sgp_exp_q[ 0 ] )
      .arg( sgp_exp_q[ sgp_exp_q.size() - 1 ] )
      .arg( rq[ 0 ] )
      .arg( rq[ rq.size() - 1 ] );


   cout << QString( "grid sizes experimental %1\n" ).arg( sgp_exp_I.size() );

   if ( !interpolate_iqq_by_case( sgp_exp_q,
                                  org_I,
                                  org_e,
                                  rq,
                                  sgp_exp_I,
                                  sgp_exp_e ) )
   {
      return false;
   }
   cout << QString( "grid size after interpolate original     %1\n" ).arg( org_I.size() );
   cout << QString( "grid size after interpolate experimental %1\n" ).arg( sgp_exp_I.size() );

//    while ( sgp_exp_I.size() < org_I.size() )
//    {
//       sgp_exp_I.push_back( sgp_exp_I[ sgp_exp_I.size() - 1 ] );

//       if ( sgp_exp_e.size() )
//       {
//          sgp_exp_e.push_back( sgp_exp_e[ sgp_exp_e.size() - 1 ] );
//       }
//    }

   sgp_exp_q = rq;
   if ( !org_e.size() )
   {
      sgp_exp_e = org_e;
   }
   return true;
}

void US_Saxs_Util::validate_control_parameters_set_one( QStringList &checks, 
                                                        QStringList &vals )
{
   for ( unsigned int i = 0; i < (unsigned int) checks.size(); i++ )
   {
      if ( !control_parameters.count( checks[ i ] ) )
      {
         control_parameters[ checks[ i ] ] = vals [ i ];
#if defined( USE_MPI ) 
         if ( !nsa_mpi || !myrank ) {
#endif
         cout << QString("Notice: %1 set to default of %2\n").arg( checks[ i ] ).arg( vals[ i ] );
#if defined( USE_MPI ) 
         }
#endif
      }
   }

   checks.clear();
   vals  .clear();
}


bool US_Saxs_Util::validate_control_parameters( bool for_sgp )
{
   errormsg = "";
   noticemsg = "";

   // first check required parameters

   QString missing_required;

   QStringList qsl_required;

   // we don't need all of these for external crysol & foxs (fix later)
   {
      qsl_required << "residuefile";
      qsl_required << "atomfile";
      qsl_required << "hybridfile";
      qsl_required << "saxsfile";
      qsl_required << "startq";
      qsl_required << "endq";
      qsl_required << "deltaq";
      qsl_required << "iqmethod";
      qsl_required << "output";
      if ( !for_sgp )
      {
         qsl_required << "inputfile";
      } else {
         qsl_required << "experimentgrid";
      }         

      qsl_required << "outputfile";
      
      for ( unsigned int i = 0; i < (unsigned int) qsl_required.size(); i++ )
      {
         if ( !control_parameters.count( qsl_required[ i ] ) )
         {
            missing_required += " " + qsl_required[ i ];
         }
      }
   }

   if ( !missing_required.isEmpty() )
   {
      errormsg = QString( "Error: Process requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   // if certain parameters are not set, set them to defaults
   QStringList checks;
   QStringList vals;

   {
      checks << "wateredensity";
      vals   << "0.334";
      checks << "swhexclvol";
      vals   << "0.0";
      checks << "scaleexclvol";
      vals   << "1.0";
      checks << "pdbmissingatoms";
      vals   << "0";
      checks << "pdbmissingresidues";
      vals   << "0";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_fd_params(
                        "^("
                        "hy|"
                        "hya|"
                        "h2|"
                        "h2a|"
                        "h3|"
                        "h3a|"
                        "fd)$" );

   if ( rx_fd_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "fdbinsize";
      vals   << "0.5";
      checks << "fdmodulation";
      vals   << "0.23";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_hy_params(
                        "^("
                        "hy|"
                        "hya|"
                        "h2|"
                        "h2a|"
                        "h3|"
                        "h3a)$"
                        );
                        
   if ( rx_hy_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "hypoints";
      vals   << "15";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_crysol_params(
                            "^("
                            "crysol)$"
                            );
                        
   if ( rx_crysol_params.search( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "crysolharm";
      vals   << "15";
      checks << "crysolgrid";
      vals   << "17";
      checks << "crysolcsh";
      vals   << "15";

      validate_control_parameters_set_one( checks, vals );
   }

   return true;
}

bool US_Saxs_Util::create_tar_output( QString filename )
{
   errormsg = "";
   US_Tar ust;
   int result;
   QStringList list;
   result = ust.create( filename, output_files, &list );

   if ( result != TAR_OK )
   {
      errormsg = QString("Error: Problem creating tar archive %1").arg( filename );
      return false;
   }
   job_output_files << filename;

   return true;
}

bool US_Saxs_Util::create_tgz_output( QString filename )
{
   errormsg = "";
   US_Tar ust;
   int result;
   QStringList list;
   QString tgz_filename = filename;
   filename.replace( QRegExp( "\\.(tgz|TGZ))$" ), ".tar" );
   QString tar_dot_gz_filename = filename + ".gz";

   cout << QString( "tgz output <%1><%2><%3>\n" ).arg( filename ).arg( tgz_filename ).arg( tar_dot_gz_filename ) << flush;

   result = ust.create( filename, output_files, &list );

   if ( result != TAR_OK )
   {
      errormsg = QString("Error: Problem creating tar archive %1. %2").arg( filename ).arg( ust.explain( result ) );
      errormsg += QString( "Attempted file list:%2:\n" ).arg( output_files.join( ":" ) );
      return false;
   }

   US_Gzip usg;
   result = usg.gzip( filename );
   if ( result != GZIP_OK )
   {
      errormsg = QString("Error: Problem gzipping tar archive %1. %2").arg( filename ).arg( usg.explain( result ) );
      return false;
   }

   QDir qd;
   qd.remove( tgz_filename );
   if ( !qd.rename( tar_dot_gz_filename, tgz_filename ) )
   {
      errormsg = QString("Error renaming %1 to %2").arg( tar_dot_gz_filename ).arg( tgz_filename );
      return false;
   }
   job_output_files << tgz_filename;

   return true;
}

QString US_Saxs_Util::vector_double_to_csv( vector < double > &vd )
{
   QString result;
   for ( unsigned int i = 0; i < vd.size(); i++ )
   {
      result += QString("%1,").arg(vd[i]);
   }
   return result;
}

bool US_Saxs_Util::write_output( QString model, vector < double > &q, vector < double > &I )
{
   // cout << "write output\n";
   if ( control_parameters.count( "sgp_running" ) ||
        control_parameters.count( "a2sb_running" ) ||
        control_parameters.count( "gridsearch_running" )
        )
   {
      sgp_last_q = q;
      sgp_last_I = I;
      return true;
   }

   if ( control_parameters.count( "output" ) &&
        control_parameters[ "output" ] == "csv" )
   {
      // cout << "write output for csv\n";
      saxs_inputfile_for_csv.push_back( control_parameters[ "inputfile" ] );
      saxs_model_for_csv.push_back( model );
      saxs_tag_for_csv.push_back( control_parameters.count( "tag" ) ?
                                  control_parameters[ "tag" ] : "" );
      saxs_grid_tag_for_csv.push_back( control_parameters.count( "grid_tag" ) ?
                                       control_parameters[ "grid_tag" ] : "" );
      saxs_cuda_tag_for_csv.push_back( control_parameters.count( "cuda_tag" ) ?
                                       control_parameters[ "cuda_tag" ] : "" );
      saxs_method_for_csv.push_back( control_parameters[ "iqmethod" ]  );
      saxs_q_for_csv.push_back( q );
      saxs_I_for_csv.push_back( I );
   }

   if ( control_parameters.count( "output" ) &&
        ( control_parameters[ "output" ] == "ssaxs" ||
          control_parameters[ "output" ] == "dat" ) )
   {
      // cout << "write output for ssaxs or dat\n";
      QString fsaxs_part_1_name = 
         control_parameters[ "outputfile" ] +
         QString("_%1").arg( model );

      QString fsaxs_part_2_name =
         ( control_parameters.count( "grid_tag" ) ? control_parameters[ "grid_tag" ] : "" ) +
         ( control_parameters.count( "tag" ) ? control_parameters[ "tag" ] : "" ) +
         ( control_parameters.count( "cuda_tag" ) ? control_parameters[ "cuda_tag" ] : "" ) +
         iqq_suffix() + "." + 
         control_parameters[ "output" ];

      QString fsaxs_name;
      do {
         if ( file_write_count.count( fsaxs_part_1_name + fsaxs_part_2_name ) )
         {
            // need to increment count
            fsaxs_name = QString( "%1_mp%2%3" )
               .arg( fsaxs_part_1_name )
               .arg( file_write_count[ fsaxs_part_1_name + fsaxs_part_2_name ] )
               .arg( fsaxs_part_2_name );
            file_write_count[ fsaxs_part_1_name + fsaxs_part_2_name ]++;
         } else {
            file_write_count[ fsaxs_part_1_name + fsaxs_part_2_name ] = 1;
            fsaxs_name = fsaxs_part_1_name + fsaxs_part_2_name;
         }
      } while ( QFile::exists( fsaxs_name ) );

      FILE *fsaxs = fopen(fsaxs_name, "w");
      if ( fsaxs ) 
      {
         output_files << fsaxs_name;
         noticemsg += "SAXS curve file: " + fsaxs_name + " created.\n";
         QString last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
                     , control_parameters[ "inputfile" ].ascii()
                     , US_Version.ascii()
                     , REVISION
                     , our_saxs_options.start_q
                     , our_saxs_options.end_q
                     , our_saxs_options.delta_q
                     );
         fprintf(fsaxs, "%s",
                 last_saxs_header.ascii() );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            fprintf(fsaxs, "%.6e\t%.6e\n", q[i], I[i]);
         }
         fclose(fsaxs);
      } 
      else
      {
         errormsg = "Error: Could not create SAXS curve file: " + fsaxs_name + "\n";
         return false;
      }
   }
   return true;
}

bool US_Saxs_Util::flush_output()
{
   if ( experimental_grids.size() < 2 )
   {
      control_parameters.erase( "this_grid_tag" );
      if ( !flush_output_one() )
      {
         return false;
      }
      saxs_inputfile_for_csv.clear();
      saxs_model_for_csv    .clear();
      saxs_tag_for_csv      .clear();
      saxs_grid_tag_for_csv .clear();
      saxs_cuda_tag_for_csv .clear();
      saxs_method_for_csv   .clear();
      saxs_q_for_csv        .clear();
      saxs_I_for_csv        .clear();
   }
   map < QString, bool > tag_names;
   for ( unsigned int i = 0; i < saxs_grid_tag_for_csv.size(); i++ )
   {
      tag_names[ saxs_grid_tag_for_csv[ i ] ] = true;
   }

   for ( map < QString, bool >::iterator it = tag_names.begin();
         it != tag_names.end();
         it++ )
   {
      control_parameters[ "this_grid_tag" ] = it->first;
      if ( !flush_output_one() )
      {
         return false;
      }
   }
   saxs_inputfile_for_csv.clear();
   saxs_model_for_csv    .clear();
   saxs_tag_for_csv      .clear();
   saxs_grid_tag_for_csv .clear();
   saxs_cuda_tag_for_csv .clear();
   saxs_method_for_csv   .clear();
   saxs_q_for_csv        .clear();
   saxs_I_for_csv        .clear();
   control_parameters.erase( "this_grid_tag" );
   return true;
}
   
bool US_Saxs_Util::flush_output_one()
{
   cout << "flush output\n";
   if ( saxs_inputfile_for_csv.size() &&
        control_parameters.count( "output" ) &&
        control_parameters[ "output" ] == "csv" )
   {
      map < QString, unsigned int > methods_used;
      bool multi_models_per_method = false;
      bool external_programs_included = false;
      for ( unsigned int i = 0; i < saxs_method_for_csv.size(); i++ )
      {
         if ( control_parameters.count( "this_grid_tag" ) &&
              saxs_grid_tag_for_csv[ i ] != control_parameters[ "this_grid_tag" ] )
         {
            continue;
         }
         if ( saxs_method_for_csv[ i ] == "crysol" ||
              saxs_method_for_csv[ i ] == "foxs" )
         {
            external_programs_included = true;
         }
         if ( methods_used.count( saxs_method_for_csv[ i ] ) )
         {
            methods_used[ saxs_method_for_csv[ i ] ]++;
            multi_models_per_method = true;
         } else {
            methods_used[ saxs_method_for_csv[ i ] ] = 1;
         }
      }
      
      if ( !multi_models_per_method && 
           !external_programs_included && 
           methods_used.size() > 1 )
      {
         methods_used.clear();
         methods_used[ "mm" ] = true;
      }

      map < QString, map < vector < double >, unsigned int > > method_q_pieces;

      for ( map < QString, unsigned int >::iterator it = methods_used.begin();
            it != methods_used.end();
            it++ )
      {
         for ( unsigned int i = 0; i < saxs_inputfile_for_csv.size(); i++ )
         {
            if ( control_parameters.count( "this_grid_tag" ) &&
                 saxs_grid_tag_for_csv[ i ] != control_parameters[ "this_grid_tag" ] )
            {
               continue;
            }
            if ( it->first == "mm" ||
                 it->first == saxs_method_for_csv[ i ] )
            {
               if ( method_q_pieces.count( it->first ) )
               {
                  if ( method_q_pieces[ it->first ].count( saxs_q_for_csv[ i ] ) )
                  {
                     cout << QString( "multiple pieces for %1 q(%2:%3)\n" )
                        .arg( it->first )
                        .arg( saxs_q_for_csv[ i ][ 0 ] )
                        .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] );
                     method_q_pieces[ it->first ][ saxs_q_for_csv[ i ] ]++;
                  } else {
                     cout << QString( "additional pieces for %1 q(%2:%3)\n" )
                        .arg( it->first )
                        .arg( saxs_q_for_csv[ i ][ 0 ] )
                        .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] );
                     method_q_pieces[ it->first ][ saxs_q_for_csv[ i ] ] = 1;
                  }
               } else {
                  cout << QString( "first piece for %1 q(%2:%3)\n" )
                     .arg( it->first )
                     .arg( saxs_q_for_csv[ i ][ 0 ] )
                     .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] );
                  map < vector < double >, unsigned int > tmp_q_map;
                  tmp_q_map[ saxs_q_for_csv[ i ] ] = 1;
                  method_q_pieces[ it->first ] = tmp_q_map;
               }
            }
         }
      }               

      map < QString, unsigned int > method_piece_pos;

      for ( map < QString, unsigned int >::iterator it = methods_used.begin();
            it != methods_used.end();
            it++ )
      {
         method_piece_pos[ it->first ] = 0;

         cout << QString( "number of pieces of %1 %2\n")
            .arg( it->first )
            .arg( method_q_pieces[ it->first ].size() );

         for (  map < vector < double >, unsigned int >::iterator it2 = method_q_pieces[ it->first ].begin();
               it2 != method_q_pieces[ it->first ].end();
               it2++ )
         {

            QString fname_part_1 = 
               control_parameters[ "outputfile" ];

            QString fname_part_2 = 
               ( method_q_pieces[ it->first ].size() > 1 ?
                 QString( "_mg%1" ).arg( ++method_piece_pos[ it->first ] ) : "" ) +
               ( control_parameters.count( "this_grid_tag" ) ? 
                 control_parameters[ "this_grid_tag" ] : "" ) +
               "_" + it->first +
               "_iqq" + ".csv";
            
            QString fname;
            do {
               if ( file_write_count.count( fname_part_1 + fname_part_2 ) )
               {
                  // need to increment count
                  fname = QString( "%1_mp%2%3" )
                     .arg( fname_part_1 )
                     .arg( file_write_count[ fname_part_1 + fname_part_2 ] )
                     .arg( fname_part_2 );
                  file_write_count[ fname_part_1 + fname_part_2 ]++;
               } else {
                  file_write_count[ fname_part_1 + fname_part_2 ] = 1;
                  fname = fname_part_1 + fname_part_2;
               }
            } while ( QFile::exists( fname ) );

            bool any_for_this = false;
            for ( unsigned int i = 0; i < saxs_inputfile_for_csv.size(); i++ )
            {
               if ( ( it->first == "mm" ||
                      it->first == saxs_method_for_csv[ i ] ) &&
                    it2->first == saxs_q_for_csv[ i ] )
               {
                  if ( control_parameters.count( "this_grid_tag" ) &&
                       saxs_grid_tag_for_csv[ i ] != control_parameters[ "this_grid_tag" ] )
                  {
                     continue;
                  }
                  any_for_this = true;
               }
            }

            if ( any_for_this )
            {
               write_output_count++;
            
               FILE *of = fopen(fname, "wb");
               if ( of )
               {
                  output_files << fname;
                  QString header = QString("")
                     .sprintf(
                              "Simulated SAXS data generated by US_SOMO %s"
                              , US_Version.ascii()
                              );
                  
                  bool q_data_added = false;
                  for ( unsigned int i = 0; i < saxs_inputfile_for_csv.size(); i++ )
                  {
                     if ( control_parameters.count( "this_grid_tag" ) &&
                          saxs_grid_tag_for_csv[ i ] != control_parameters[ "this_grid_tag" ] )
                     {
                        continue;
                     }
                     
                     if ( ( it->first == "mm" ||
                            it->first == saxs_method_for_csv[ i ] ) &&
                          it2->first == saxs_q_for_csv[ i ] )
                     {
                        if ( !q_data_added )
                        {
                           fprintf(of, "\"Name\",\"Type; q:\",%s,\"%s\"\n", 
                                   vector_double_to_csv( saxs_q_for_csv[ i ] ).ascii(),
                                   header.ascii());
                           q_data_added = true;
                        }
                        QString name = QString("%1 Model %2 %3 %4")
                           .arg( saxs_inputfile_for_csv[ i ] )
                           .arg( saxs_model_for_csv[ i ] )
                           .arg( saxs_tag_for_csv[ i ] )
                           .arg( saxs_method_for_csv[ i ] );
                        
                        fprintf(of, "\"%s\",\"%s\",%s\n", 
                                name.ascii(),
                                "I(q)",
                                vector_double_to_csv(saxs_I_for_csv[i]).ascii());
                     }
                  }            
                  fclose( of );
                  noticemsg += QString("file %1 written\n").arg( fname );
               } else {
                  errormsg = QString("Error: could not open %1 for writing").arg( fname );
                  return false;
               }
            }
         }
      }
   }
   return true;
}

bool US_Saxs_Util::write_timings( QString file, QString msg )
{
   if ( timings.isEmpty() )
   {
      return true;
   }

   unsigned int ext = 0;
   QString out_file = file;
   while ( QFile::exists( out_file ) )
   {
      out_file = QString( "%1-%2" ).arg( file ).arg( ++ext );
   }

   QFile f( out_file );
   if ( f.open( QIODevice::WriteOnly ) )
   {
      Q3TextStream ts( &f );
      ts << msg << endl;
      ts << timings;
      f.close();
      timings = "";
      output_files << out_file;
      return true;
   }
   timings  = "";
   errormsg = QString( "Error: could not open timing output file %1" ).arg( out_file );
   return false;
}
