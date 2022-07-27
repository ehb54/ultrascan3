#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <QTextStream>

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
   output_files          .clear( );
   job_output_files      .clear( );
   saxs_inputfile_for_csv.clear( );
   saxs_model_for_csv    .clear( );
   saxs_method_for_csv   .clear( );
   saxs_q_for_csv        .clear( );
   saxs_I_for_csv        .clear( );
   file_write_count      .clear( );
   write_output_count    = 0;
   timings               = "";
   
   if ( !us_log ) {
#if !defined( USE_MPI )
      us_log = new US_Log( "runlog.txt" );
#else
      us_log = new US_Log( QString( "runlog-%1.txt" ).arg( myrank ) );
#endif
      output_files << us_log->f.fileName();
   }

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
      f.setFileName( controlfile );
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

      f.setFileName( controlfile );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg = QString( "Error: %1 can not be opened.  Check permissions" ).arg( controlfile );
         return false;
      }
   }

   // read and setup control

   QTextStream ts( &f );
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
                      "bestrcoalautominmax|"
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
                      "bestrcoalautominmax|"
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

      QStringList qsl = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );

      if ( !qsl.size() )
      {
         continue;
      }

      if ( rx_valid.indexIn( qsl[ 0 ].toLower() ) == -1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Unrecognized token %3" )
            .arg( controlfile )
            .arg( line )
            .arg( qsl[ 0 ] );
         return false;
      }

      QString option = qsl[ 0 ].toLower();
      qsl.pop_front();
      control_parameters[ option ] = qsl.join(" ");

      if ( rx_arg_1.indexIn( option ) != -1 && 
           qsl.size() < 1 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }

      if ( rx_arg_2.indexIn( option ) != -1 && 
           qsl.size() < 2 )
      {
         errormsg = QString( "Error reading %1 line %2 : Missing argument " )
            .arg( controlfile )
            .arg( line );
         return false;
      }


      if ( rx_file.indexIn( option ) != -1 )
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

      if ( rx_flush.indexIn( option ) != -1 )
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
#if !defined( Q_OS_WIN ) || defined( MINGW )
# if __GNUC__ == 5
         unsigned int usec = (unsigned int)( qsl[ 0 ].toDouble() * 1000 );
         us_usleep( usec );
# else
         // 999999999
         double secs = qsl[ 0 ].toDouble();
         secs *= 2e0;
         // secs *= 1e-1; // finer resolution if nanosleep available
         cout << QString( "sleep %1\n" ).arg( secs ).toLatin1().data();
         timespec ns;
         timespec ns_ret;
         ns.tv_sec  = ( long ) secs;
         ns.tv_nsec = ( long ) ( 1e9 * ( secs - ns.tv_sec ) );
         cout << QString( "sleep s %1\n" ).arg( ns.tv_sec ).toLatin1().data();
         cout << QString( "sleep ns %1\n" ).arg( ns.tv_nsec ).toLatin1().data();
         nanosleep(&ns, &ns_ret);
# endif
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
               
            for ( unsigned int i = 0; i < (unsigned int) output_dmd_pdbs.size(); i++ )
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
         if ( rx_valid_saxs_iqmethod.indexIn( qsl[ 0 ] ) == -1 )
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
         experimental_grids.clear( );
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
         output_dmd_pdbs.clear( );
         // read pdb, needs residue file
         QString ext = QFileInfo( qsl[ 0 ] ).suffix().toLower();

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
            errormsg += QString( "\nError %1 line %2 : PDB file has an error. <%3>\n" )
               .arg( controlfile )
               .arg( line )
               .arg( qsl[ 0 ] );
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
         if ( us_udp_msg ) {
            us_udp_msg->send_json( { { "_progressmsg", "Assembling output" } } );
         }
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

      if ( option == "dmdsupportfile" ) {
         QString filename = control_parameters[ option ];
         // static QRegExp rx_mol2 = QRegExp( "([^/ .]+)\\.mol2$" );
         // if ( rx_mol2.indexIn( filename, 0 ) != -1 ) {
         //    QString mol2 = rx_mol2.cap(1);
         //    dmd_mol2.insert( mol2 );
         // }
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
                  QTextStream ts( &f );
                  ts << QString( "%1: Timed out\n" ).arg( myrank ) << flush;
                  f.close();
                  output_files << f.fileName();
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

   sgp_exp_q      .clear( );
   sgp_exp_q_index.clear( );
   sgp_exp_I      .clear( );
   sgp_exp_e      .clear( );

   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg = QString("Error: %1 does not exist").arg( filename );
      return false;
   }
   
   QString ext = QFileInfo( filename ).suffix().toLower();

   QRegExp rx_valid_ext (
                         "^("
                         "csv|"
                         "dat|"
                         "int|"
                         // "out|"
                         "ssaxs)$" );

   if ( rx_valid_ext.indexIn( ext ) == -1 )
   {
      errormsg = QString("Error: %1 unsupported file extension %2").arg( filename ).arg( ext );
      return false;
   }
      
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString("Error: can not open %1, check permissions ").arg( filename );
      return false;
   }

   QTextStream ts(&f);
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
      QStringList qsl_headers = qsl.filter("\"Name\",\"Type; q:\"");
      if ( qsl_headers.size() == 0 )
      {
         errormsg = QString("Error: the file %1 has no valid q grid lines").arg( filename );
         return false;
      }
      QStringList qsl_q = (qsl_headers[ 0 ]).split( "," );
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
              rx_ok_line.indexIn( qv[i] ) == -1 )
         {
            continue;
         }
         
         // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
         QStringList tokens;
         {
            QString qs = qv[i].replace(QRegExp("^\\s+"),"");
            tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         }

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

   checks.clear( );
   vals  .clear( );
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

   if ( rx_fd_params.indexIn( control_parameters[ "iqmethod" ] ) != -1 )
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
                        
   if ( rx_hy_params.indexIn( control_parameters[ "iqmethod" ] ) != -1 )
   {
      checks << "hypoints";
      vals   << "15";

      validate_control_parameters_set_one( checks, vals );
   }

   QRegExp rx_crysol_params(
                            "^("
                            "crysol)$"
                            );
                        
   if ( rx_crysol_params.indexIn( control_parameters[ "iqmethod" ] ) != -1 )
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

      FILE *fsaxs = us_fopen(fsaxs_name, "w");
      if ( fsaxs ) 
      {
         output_files << fsaxs_name;
         noticemsg += "SAXS curve file: " + fsaxs_name + " created.\n";
         QString last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
                     , control_parameters[ "inputfile" ].toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , our_saxs_options.start_q
                     , our_saxs_options.end_q
                     , our_saxs_options.delta_q
                     );
         fprintf(fsaxs, "%s",
                 last_saxs_header.toLatin1().data() );
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
      saxs_inputfile_for_csv.clear( );
      saxs_model_for_csv    .clear( );
      saxs_tag_for_csv      .clear( );
      saxs_grid_tag_for_csv .clear( );
      saxs_cuda_tag_for_csv .clear( );
      saxs_method_for_csv   .clear( );
      saxs_q_for_csv        .clear( );
      saxs_I_for_csv        .clear( );
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
   saxs_inputfile_for_csv.clear( );
   saxs_model_for_csv    .clear( );
   saxs_tag_for_csv      .clear( );
   saxs_grid_tag_for_csv .clear( );
   saxs_cuda_tag_for_csv .clear( );
   saxs_method_for_csv   .clear( );
   saxs_q_for_csv        .clear( );
   saxs_I_for_csv        .clear( );
   control_parameters.erase( "this_grid_tag" );
   return true;
}
   
bool US_Saxs_Util::flush_output_one()
{
   if ( us_log )
   {
      us_log->log( "flush output\n" );
   }
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
         methods_used.clear( );
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
                     if ( us_log )
                     {
                        us_log->log( QString( "multiple pieces for %1 q(%2:%3)\n" )
                                     .arg( it->first )
                                     .arg( saxs_q_for_csv[ i ][ 0 ] )
                                     .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] ) );
                     }
                     method_q_pieces[ it->first ][ saxs_q_for_csv[ i ] ]++;
                  } else {
                     if ( us_log )
                     {
                        us_log->log( QString( "additional pieces for %1 q(%2:%3)\n" )
                                     .arg( it->first )
                                     .arg( saxs_q_for_csv[ i ][ 0 ] )
                                     .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] ) );
                     }
                     method_q_pieces[ it->first ][ saxs_q_for_csv[ i ] ] = 1;
                  }
               } else {
                  if ( us_log )
                  {
                     us_log->log( QString( "first piece for %1 q(%2:%3)\n" )
                                  .arg( it->first )
                                  .arg( saxs_q_for_csv[ i ][ 0 ] )
                                  .arg( saxs_q_for_csv[ i ][ saxs_q_for_csv[ i ].size() - 1 ] ) );
                  }
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

         if ( us_log )
         {
            us_log->log( QString( "number of pieces of %1 %2\n")
            .arg( it->first )
                         .arg( method_q_pieces[ it->first ].size() ) );
         }

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
            
               FILE *of = us_fopen(fname, "wb");
               if ( of )
               {
                  output_files << fname;
                  QString header = QString("")
                     .sprintf(
                              "Simulated SAXS data generated by US_SOMO %s"
                              , US_Version.toLatin1().data()
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
                                   vector_double_to_csv( saxs_q_for_csv[ i ] ).toLatin1().data(),
                                   header.toLatin1().data());
                           q_data_added = true;
                        }
                        QString name = QString("%1 Model %2 %3 %4")
                           .arg( saxs_inputfile_for_csv[ i ] )
                           .arg( saxs_model_for_csv[ i ] )
                           .arg( saxs_tag_for_csv[ i ] )
                           .arg( saxs_method_for_csv[ i ] );
                        
                        fprintf(of, "\"%s\",\"%s\",%s\n", 
                                name.toLatin1().data(),
                                "I(q)",
                                vector_double_to_csv(saxs_I_for_csv[i]).toLatin1().data());
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
      QTextStream ts( &f );
      ts << msg << Qt::endl;
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

bool US_Saxs_Util::mwc( 
                       const vector < double > & q_org,
                       const vector < double > & I,
                       double                    Rg,
                       double                    /* sigRg */,
                       double                    I0,
                       double                    sigI0,
                       double                    mw_per_N,
                       double                  & qm,
                       double                  & Vc,
                       double                  & Qr,
                       double                  & mwc,
                       double                  & mwc_sd,
                       QString                 & messages,
                       QString                 & notes
                        )
{
   // use vcm.json

   messages = "";
   notes    = "";
   
   if ( q_org.size() != I.size() )
   {
      messages = QString( "I, q different length %1 %2" ).arg( q_org.size() ).arg( I.size() );
      return false;
   }

   if ( 
       !vcm.count( "q" ) ||
       !vcm.count( "a" ) ||
       !vcm.count( "b" ) ||
       !vcm.count( "c" ) ||
       !vcm.count( "a:y2" ) ||
       !vcm.count( "b:y2" ) ||
       !vcm.count( "c:y2" )
        )
   {
      messages = "MW[C] vcm.json not loaded";
      return false;
   }

   double maxq = vcm[ "q" ].back();

   // crop to maxq from vcm arrays (see us_saxs_util_loads.cpp::load_vcm_json() )

   vector < double > q = q_org;

   if ( q.back() > maxq )
   {
      for ( int i = (int) q.size() - 1; i >= 4; --i )
      {
         if ( q[ i ] <= maxq )
         {
            q.resize( i + 1 );
            break;
         }
      }
      notes = QString( "MW[C] note: q cropped to %1" ).arg( q.back() );
   }

   if ( q.size() < 5 )
   {
      messages = QString( "too few point %1 < 5" ).arg( q.size() );
      return false;
   }
   qm = q.back();

   vector < double > dq ( q.size(), -1e0 );
   vector < double > sqs( q.size(), -1e0 );

   // first point

   double sqiqdq = q[ 0 ] * I[ 0 ] * ( q[ 1 ] - q[ 0 ] );

   dq[ 0 ]  = q[ 1 ] - q[ 0 ];
   sqs[ 0 ] = sqiqdq;
   
   // middle points

   for ( int i = 1; i < (int) q.size() - 1; ++i )
   {
      sqiqdq += q[ i ] * I[ i ] * ( q[ i + 1 ] - q[ i - 1 ] ) * 5e-1;
      dq[ i ] = ( q[ i + 1 ] - q[ i - 1 ] ) * 5e-1;
      sqs[ i ] = sqiqdq;
   }

   // last point
   
   int i = q.size() - 1;

   sqiqdq += q[ i ] * I[ i ] * ( q[ i ] - q[ i - 1 ] );
   dq[ i ] = q[ i ] - q[ i - 1 ];
   sqs[ i ] = sqiqdq;

   if ( !sqiqdq )
   {
      messages += "sum(q * I(q) * dq ) is zero ";
   }

   if ( !messages.isEmpty() )
   {
      return false;
   }

   // sqiqdq computed, log, invert with splined a,b,c, check limits use gparams

   double a;
   double b;
   double c;
   QString messagesa;
   QString messagesb;
   QString messagesc;

   if ( !static_apply_natural_spline( vcm[ "q" ],
                                      vcm[ "a" ],
                                      vcm[ "a:y2" ],
                                      qm,
                                      a,
                                      messagesa ) ||
        
        !static_apply_natural_spline( vcm[ "q" ],
                                      vcm[ "b" ],
                                      vcm[ "b:y2" ],
                                      qm,
                                      b,
                                      messagesb ) ||
        
        !static_apply_natural_spline( vcm[ "q" ],
                                      vcm[ "c" ],
                                      vcm[ "c:y2" ],
                                      qm,
                                      c,
                                      messagesc )
        )
   {
      messages += messagesa + messagesb + messagesc;
      return false;
   }

   Vc = I0 / sqiqdq;
   double lVc = log( Vc );

   // compute descriminant

   double disc = ( b * b ) - ( 4e0 * c * ( a - lVc ) );
   if ( disc < 0e0 )
   {
      messages += " Vc[c] during inversion: discriminant is negative!";
      return false;
   }

   if ( c == 0e0 )
   {
      messages += " Vc[c] during inversion: c is zero!";
      return false;
   }
      
   double lN = ( -b + sqrt( disc ) ) / ( 2e0 * c );

   double N = exp( lN );

   mwc = N * mw_per_N;

   Qr = Vc * Vc / Rg;

   // sd calc
   // using p.d.'s to propagate uncertainty from maxima:
   // maxima:
   //                                             2
   //          sqrt(4 c log(I0/sqiqdq) - 4 a c + b )
   //          ------------------------------------- - b/c
   //                            c
   //        %e
   //   sqrt(---------------------------------------------) abs(mw_per_N) abs(sigI0)
   //                          I0               2
   //                4 c log(------) - 4 a c + b
   //                        sqiqdq
   //   ----------------------------------------------------------------------------
   //                                     abs(I0)
   // fortran(%);
   //
   //      (sqrt(exp(sqrt(4*c*log(I0/sqiqdq)-4*a*c+b**2)/c-b/c)/(b**2-4*a*c+4*c*log(I0/sqiqdq)))*abs(mw_per_N)*abs(sigI0))/abs(I0)

   {
      double b2m4acp4clogi0sqiqdq = b * b - 4e0 * a * c + 4e0 * c * log( I0 / sqiqdq );

      mwc_sd =
         ( sqrt( exp( sqrt( b2m4acp4clogi0sqiqdq )
                      /
                      c - b / c )
                 / 
                 b2m4acp4clogi0sqiqdq ) * 
           mw_per_N * sigI0 )
         / 
         I0
         ;

      // double mwc_sd_org =
      //    ( sqrt( exp( sqrt( 4e0 * c * log( I0 / sqiqdq ) - 4e0 * a * c + b * b ) 
      //                 /
      //                 c - b / c )
      //            / 
      //            ( b * b - 4e0 * a * c + 4 * c * log( I0 / sqiqdq ) ) ) * 
      //      mw_per_N * sigI0 ) 
      //    / 
      //    I0
      //    ;

      // us_qdebug( QString( "mwc %1 mwc_sd %1 mwc_sd_org %1 rg %1 sigRg %1 I0 %1 sigI0 %1" )
      //         .arg( mwc ).arg( mwc_sd ).arg( mwc_sd_org ).arg( Rg ).arg( sigRg ).arg( I0 ).arg( sigI0 ) );
   }

   // us_qdebug( QString( "splined a %1 b %2 c %3 Vc %4 lVc %5 lN %6 N %7 mwc %8" )
   //         .arg( a )
   //         .arg( b )
   //         .arg( c )
   //         .arg( Vc )
   //         .arg( lVc )
   //         .arg( lN )
   //         .arg( N )
   //         .arg( mwc )
   //         );

   return true;
}

double US_Saxs_Util::prob_of_streak( int n, int c ) {
   if ( prob_of_streak_cache.count( n ) &&
        prob_of_streak_cache[ n ].count( c ) ) {
      return prob_of_streak_cache[ n ][ c ];
   }
   if  ( c > n || n <= 0e0 ) {
      return 0e0;
   }

   double res = pow( 0.5e0, c );

   for ( int i = 1; i <= c; ++i ) {
        double pr = prob_of_streak( n - i, c );
        res += pow( 0.5e0, i - 1 ) * 0.5e0 * pr;
   }

   prob_of_streak_cache[ n ][ c ] = res;
   return res;
}

float US_Saxs_Util::prob_of_streak_f( int n, int c ) {
   if ( prob_of_streak_cache_f.count( n ) &&
        prob_of_streak_cache_f[ n ].count( c ) ) {
      return prob_of_streak_cache_f[ n ][ c ];
   }
   if  ( c > n || n <= 0e0 ) {
      return 0e0;
   }

   float res = pow( 0.5e0, c );

   for ( int i = 1; i <= c; ++i ) {
        float pr = prob_of_streak_f( n - i, c );
        res += pow( 0.5e0, i - 1 ) * 0.5e0 * pr;
   }

   prob_of_streak_cache_f[ n ][ c ] = res;
   return res;
}

bool US_Saxs_Util::cormap( 
                          const vector < double >            & q,
                          const vector < vector < double > > & I,
                          vector < vector < double > >       & rkl,
                          int                                & N,
                          int                                & S,
                          int                                & C,
                          double                             & P
                           ) {

   // US_Timer ust;
   // ust.init_timer( "prob_of_streak" );
   // ust.start_timer( "prob_of_streak" );
   // double ps = prob_of_streak( 25000, 100 );
   // ust.end_timer( "prob_of_streak" );
   // ust.init_timer( "prob_of_streak_f" );
   // ust.start_timer( "prob_of_streak_f" );
   // float psf = prob_of_streak_f( 25000, 100 );
   // ust.end_timer( "prob_of_streak_f" );
   // us_qdebug( ust.list_times() );
   // us_qdebug( QString( "p %1 pf %2" ).arg( ps ).arg( psf ) );
   // errormsg = "";

   int m  = (int) I.size();

   if ( m < 2 ) {
         errormsg = 
            QString( us_tr( "Error in cormap: a minimum of 2 intensity vectors are required and only %1 were given" ) )
            .arg( m );
         return false;
   }

   int qp = (int) q.size();
   double mm1r = 1e0 / ( (double) m - 1e0 );
   double mr   = 1e0 / ( (double) m );

   for ( int i = 0; i < m; ++i ) {
      if ( (int) I[ i ].size() != qp ) {
         errormsg = 
            QString( us_tr( "Error in cormap: the intensity vectors are of different size (%1)  than the q vector size (%2)" ) )
            .arg( I[ i ].size() )
            .arg( q     .size() )
            ;
         return false;
      }
   }

   vector < double > Ibar = I[ 0 ];

   for ( int k = 0; k < qp; ++k ) {
      for ( int i = 1; i < m; ++i ) {
         Ibar[ k ] += I[ i ][ k ];
      }
      Ibar[ k ] *= mr;
   }

   vector < vector < double > > sigma( qp );

   for ( int k = 0; k < qp; ++k ) {
      sigma[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         sigma[ k ][ l ] = 0e0;
         for ( int i = 0; i < m; ++i ) {
            sigma[ k ][ l ] += ( I[ i ][ k ] - Ibar[ k ] ) * ( I[ i ][ l ] - Ibar[ l ] );
         }
         sigma[ k ][ l ] *= mm1r;
      }
   }

   // for ( int i = 0; i < (int) sigma.size(); ++i ) {
   //    US_Vector::printvector( QString( "sigma[%1]" ).arg( i ), sigma[ i ] );
   // }   

   rkl.resize( qp );

   for ( int k = 0; k < qp; ++k ) {
      rkl[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         bool altsigma = false;
         if ( sigma[ k ][ k ] == 0e0 ) {
            // us_qdebug( QString( us_tr( "Notice in cormap: found zero sigma at %1 %2" ) ).arg( k ).arg( k ) );
            altsigma = true;
         }
         if ( sigma[ l ][ l ] == 0e0 ) {
            // us_qdebug( QString( us_tr( "Notice in cormap: found zero sigma at %1 %2" ) ).arg( l ).arg( l ) );
            altsigma = true;
         }
         if ( altsigma ) {
            rkl[ k ][ l ] = -1e0;
         } else {
            rkl[ k ][ l ] = sigma[ k ][ l ] / ( sqrt( sigma[ k ][ k ] ) * sqrt( sigma[ l ][ l ] ) );
         }            
      }
   }

   double min = rkl[ 0 ][ 0 ];
   double max = rkl[ 0 ][ 0 ];

   for ( int k = 0; k < qp; ++k ) {
      rkl[ k ].resize( qp );
      for ( int l = 0; l < qp; ++l ) {
         if ( min > rkl[ k ][ l ] ) {
            min = rkl[ k ][ l ];
         }
         if ( max < rkl[ k ][ l ] ) {
            max = rkl[ k ][ l ];
         }
      }
   }

   // us_qdebug( QString( "min %1 max %2" ).arg( min ).arg( max ) );

   // for ( int k = 0; k < qp; ++k ) {
   //    QString summary;
   //    summary += QString( "k = %1:" ).arg( k );
   //    for ( int l = 0; l < qp; ++l ) {
   //       summary += QString( " %1" ).arg( rkl[ k ][ l ] );
   //    }
   //    summary += "\n";
   //    cout << summary;
   // }
   
   // us_qdebug( "trace:" );
   
   // {
   //    QString summary;
   //    for ( int k = 0; k < qp; ++k ) {
   //       summary += QString( "k = %1:" ).arg( k );
   //       summary += QString( " %1" ).arg( rkl[ k ][ k ] );
   //       summary += "\n";
   //    }
   //    cout << summary;
   // }

   // find largest patch ... going to have to check 2 dimensions
   // go down diagonal and check 1/2 block

   {
      int longest_start_q     = 0;
      int contiguous_pts      = 1;
      bool pos_region         = rkl[ 0 ][ 0 ] > 0;

      int this_start_q        = longest_start_q;
      int this_contiguous_pts = contiguous_pts;

      for ( int k = 1; k < qp; ++k ) {
         bool new_region = false;
         if ( ( rkl[ k ][ k ] > 0 ) != pos_region ) {
            new_region = true;
         } else {
            // check 1/2 block
            for ( int k2 = this_start_q; !new_region && k2 <= k; ++k2 ) {
               for ( int l2 = k2; !new_region && l2 <= k; ++l2 ) {
                  if ( ( rkl[ k2 ][ l2 ] > 0 ) != pos_region ) {
                     new_region = true;
                  }
               }
            }
         }

         if ( !new_region ) {
            this_contiguous_pts++;
         } else {
            if ( contiguous_pts < this_contiguous_pts ) {
               contiguous_pts      = this_contiguous_pts;
               longest_start_q     = this_start_q;
            }
            this_start_q        = k;
            this_contiguous_pts = 1;
            pos_region          = rkl[ k ][ k ] > 0;
         }
      }

      if ( contiguous_pts < this_contiguous_pts ) {
         contiguous_pts      = this_contiguous_pts;
         longest_start_q     = this_start_q;
      }

      N = qp;
      S = longest_start_q + 1;
      C = contiguous_pts;
      P = (double) prob_of_streak_f( N, C );
      // us_qdebug( QString( "N %1 Loc %2 Size %3" ).arg( qp ).arg( longest_start_q + 1 ).arg( contiguous_pts ) );
      // us_qdebug( QString( "N %1 Loc %2 Size %3 PV %4" ).arg( qp ).arg( longest_start_q + 1 ).arg( contiguous_pts ).arg( prob_of_streak( qp, contiguous_pts ) ) );
   }


   return true;
}

bool US_Saxs_Util::average( 
                           vector < double > &q,
                           vector < double > &I,
                           vector < double > &e,
                           double &avg_q,
                           double &avg_I,
                           double &avg_e
                            ) {

   // cout << US_Vector::qs_vector3( "average q,i,e input", q, I, e ) << endl;

   errormsg = "";
   if ( !q.size() || 
        q.size() != I.size() || 
        q.size() != e.size() ) {
      errormsg = "US_Saxs_Util::average Error: incompatible input vector sizes. they must be equal and at least length 1";
      return false;
   }

   avg_q = q[ 0 ];
   avg_I = I[ 0 ];
   avg_e = e[ 0 ];

   for ( int j = 1; j < (int) q.size(); ++j ) {
      avg_q += q[ j ];
      avg_I += I[ j ];
      avg_e += e[ j ];
   }

   avg_q /= (double) q.size();
   avg_I /= (double) q.size();
   avg_e /= (double) q.size();

   // cout << QString( "averages %1 %2 %3\n" ).arg( avg_q ).arg( avg_I ).arg( avg_e );

   return true;
}   
                           
bool US_Saxs_Util::bin(
                       vector < double > &q,
                       vector < double > &I,
                       vector < double > &e,
                       vector < double > &new_q,
                       vector < double > &new_I,
                       vector < double > &new_e,
                       unsigned int      points
                       ) 
{
   // cout << US_Vector::qs_vector3( "bin q,i,e input", q, I, e ) << endl;

   errormsg = "";
   if ( !q.size() || 
        q.size() != I.size() || 
        q.size() != e.size() ) {
      errormsg = "US_Saxs_Util::bin Error: incompatible input vector sizes. they must be equal and at least length 1";
      return false;
   }

   if ( points <= 1 ) {
      errormsg = "US_Saxs_Util::bin Error: binning points must be at least 2";
      return false;
   }

   new_q.clear( );
   new_I.clear( );
   new_e.clear( );

   for ( int i = 0; i < (int) q.size(); i += points ) {
      vector < double > this_q;
      vector < double > this_I;
      vector < double > this_e;
      double this_new_q;
      double this_new_I;
      double this_new_e;
      for ( int j = 0; j < (int) points && i + j < (int) q.size(); ++j ) {
         this_q.push_back( q[ i + j ] );
         this_I.push_back( I[ i + j ] );
         this_e.push_back( e[ i + j ] );
      }
      if ( !average( this_q, this_I, this_e, this_new_q, this_new_I, this_new_e ) ) {
         return false;
      }
      new_q.push_back( this_new_q );
      new_I.push_back( this_new_I );
      new_e.push_back( this_new_e );
   }
      
   // cout << US_Vector::qs_vector3( "final new vectors", new_q, new_I, new_e ) << endl;

   return true;
}
