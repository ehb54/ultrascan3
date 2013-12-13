#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

bool US_Saxs_Util::run_best()
{
   errormsg = "";
   QStringList required;
   required 
      << "inputfilenoread"
      << "bestrcoalnmin"
      << "bestrcoalnmax"
      << "bestrcoaln"
      << "bestmsrradiifile"
      << "bestmsrpatternfile"
      << "bestmsrmaxtriangles"
      ;

   for ( int i = 0; i < (int) required.size(); ++i )
   {
      if ( !control_parameters.count( required[ i ] ) )
      {
         errormsg += QString( "BEST: %1 parameter must be defined\n" ).arg( required[ i ] );
      }
   }      

   QString prog_base =
      env_ultrascan + QDir::separator() +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + QDir::separator()
      ;

   vector < QString > progs;
   progs.push_back( prog_base + "msroll" );
   progs.push_back( prog_base + "rcoal" );
   progs.push_back( prog_base + "best" );

   for ( int i = 0; i < (int)progs.size(); ++i )
   {
      QFileInfo qfi( progs[ i ] );
      if ( !qfi.exists() )
      {
         errormsg += QString( "BEST: program '%1' does not exist\n" ).arg( progs[ i ] );
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   // strip pdb

   QString pdb_stripped;
   QStringList exclude_atoms_list;
   QStringList exclude_residues_list;
   exclude_residues_list 
      << "HOH"
      ;

   if ( !strip_pdb( pdb_stripped,
                    control_parameters[ "inputfilenoread" ],
                    exclude_atoms_list,
                    exclude_residues_list,
                    true ) )
   {
      return false;
   }

   QString inputbase = QFileInfo( pdb_stripped ).baseName();
   
   // run msroll

   int p = 0;
   double  start_fineness   = ( control_parameters.count( "bestmsrfinenessangle" ) &&
                               control_parameters[ "bestmsrfinenessangle" ].toDouble() >= .5 ) ? control_parameters[ "bestmsrfinenessangle" ].toDouble() : (double) 0.8;
   int     found_triangles  = 0;
   int     best_triangles   = 0;
   int     max_triangles    = control_parameters[ "bestmsrmaxtriangles" ].toInt();
   QString best_inputbase   = "";

   for( double use_fineness = start_fineness; found_triangles <= max_triangles && use_fineness >= 0.3; use_fineness -= 0.02 )
   {
      QString msr_inputbase = QString( "%1_f%2" ).arg( inputbase ).arg( QString( "%1" ).arg( use_fineness ).replace( ".", "_" ) );
      if ( best_inputbase.isEmpty() )
      {
         best_inputbase = msr_inputbase;
      }
      QString cmd = 
         QString( "%1 -m %2 -r %3 -y %4 -t %5.c3p -v %6.c3v" )
         .arg( progs[ p ] )
         .arg( pdb_stripped )
         .arg( control_parameters[ "bestmsrradiifile" ] )
         .arg( control_parameters[ "bestmsrpatternfile" ] )
         .arg( msr_inputbase )
         .arg( msr_inputbase )
         ;

      if ( control_parameters.count( "bestmsrprober" ) )
      {
         cmd += QString( " -p %1" ).arg( control_parameters[ "bestmsrprober" ] );
      }

      //      if ( control_parameters.count( "bestmsrfinenessangle" ) )
      //      {
      //         cmd += QString( " -f %1" ).arg( control_parameters[ "bestmsrfinenessangle" ] );
      //      }
      cmd += QString( " -f %1" ).arg( use_fineness );

      if ( control_parameters.count( "bestmsrcoalescer" ) )
      {
         cmd += QString( " -l %1" ).arg( control_parameters[ "bestmsrcoalescer" ] );
      }

      cmd += QString( " 2> msr_%1.stderr > msr_%2.stdout" ).arg( msr_inputbase ).arg( msr_inputbase );
      qDebug( QString( "best cmd = %1" ).arg( cmd ) );

      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      QStringList expected;
      expected 
         << msr_inputbase + ".c3p"
         << msr_inputbase + ".c3v"
         << "msr_" + msr_inputbase + ".stdout"
         << "msr_" + msr_inputbase + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         } else {
            // output_files << expected[ i ];
         }
      }
      // count triangles produced
      {
         QFile f( "msr_" + msr_inputbase + ".stderr" );
         f.open( IO_ReadOnly );
         QTextStream ts( &f );
         while ( !ts.atEnd() )
         {
            QString qs = ts.readLine();
            if ( qs.contains( "triangles written to disk" ) )
            {
               QStringList qsl = QStringList::split( " ", qs.stripWhiteSpace() );
               if ( qsl.size() )
               {
                  found_triangles = qsl[ 0 ].toInt();
                  qDebug( QString( "found triangles %1 with fineness %2" ).arg( found_triangles ).arg( use_fineness ) );
                  if ( found_triangles > best_triangles && found_triangles <= max_triangles )
                  {
                     best_triangles = found_triangles;
                     best_inputbase = msr_inputbase;
                  }
                  break;
               }
            }
         }
         f.close();
      }
   }

   inputbase = best_inputbase;

   {
      QStringList expected;
      expected 
         << inputbase + ".c3p"
         << inputbase + ".c3v"
         << "msr_" + inputbase + ".stdout"
         << "msr_" + inputbase + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         } else {
            output_files << expected[ i ];
         }
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   // run rcoal
   p++;
   {
      QString cmd = 
         QString( "%1 -f %2.c3p -nmax %3 -nmin %4 -n %5" )
         .arg( progs[ p ] )
         .arg( inputbase )
         .arg( control_parameters[ "bestrcoalnmax" ] )
         .arg( control_parameters[ "bestrcoalnmin" ] )
         .arg( control_parameters[ "bestrcoaln" ] )
         ;

      cmd += QString( " 2> rcoal_%1.stderr > rcoal_%2.stdout" ).arg( inputbase ).arg( inputbase );

      qDebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      // run & check output
      QStringList expected;
      expected 
         << "rcoal_" + inputbase + ".stdout"
         << "rcoal_" + inputbase + ".stderr"
         ;

      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         } else {
            output_files << expected[ i ];
         }
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   QStringList outfiles;
   {
      QFile f( "rcoal_" + inputbase + ".stdout" );
      f.open( IO_ReadOnly );
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         if ( qs.contains( "Output files written:" ) )
         {
            do 
            {
               qs = QString( ts.readLine() ).stripWhiteSpace();
            } while ( qs.isEmpty() && !ts.atEnd() );

            if ( !qs.isEmpty() )
            {
               outfiles << qs;
               output_files << qs;
               // qDebug( QString( "outfiles.back() is %1" ).arg( outfiles.back() ) );
            } else {
               errormsg += "could not correctly find output files in rcoal stdout\n";
            }
         }
      }
      f.close();
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   // run best
   p++;
   QStringList csvfiles;
   QStringList triangles;
   vector < double > one_over_triangles;

   for ( int i = 0; i < (int) outfiles.size(); ++i )
   {
      qDebug( QString( "processing outfile %1" ).arg( outfiles[ i ] ) );
      QString cmd = 
         QString( "%1 -f %2 -mw %3" )
         .arg( progs[ p ] )
         .arg( outfiles[ i ] )
         .arg( control_parameters[ "bestbestmw" ] )
         ;

      if ( control_parameters.count( "bestbestna" ) )
      {
         cmd += " -na";
      }
      if ( control_parameters.count( "bestbestp" ) )
      {
         cmd += " -p";
      }
      if ( control_parameters.count( "bestbestv" ) )
      {
         cmd += " -v";
      }
      if ( control_parameters.count( "bestbestvc" ) )
      {
         cmd += " -vc";
      }

      cmd += QString( " 2> best_%1.stderr > best_%2.stdout" ).arg( outfiles[ i ] ).arg( outfiles[ i ] );

      qDebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

      QStringList expected;
      expected 
         << "best_" + outfiles[ i ] + ".stdout"
         << "best_" + outfiles[ i ] + ".stderr"
         ;

      QString expected_base;

      if ( control_parameters.count( "bestbestv" ) &&
           !control_parameters.count( "bestbestvc" ) )
      {
         expected_base += "v";
      }

      if ( control_parameters.count( "bestbestvc" ) )
      {
         expected_base += "vc";
      }

      expected_base += "m";

      if ( control_parameters.count( "bestbestna" ) )
      {
         expected_base += "n";
      }

      if ( control_parameters.count( "bestbestp" ) )
      {
         expected_base += "p";
      }

      expected
         << outfiles[ i ] + expected_base + ".log" 
         << outfiles[ i ] + expected_base + ".be" 
         ;

      csvfiles  << outfiles[ i ] + expected_base + ".be";
      triangles << QString( outfiles[ i ] ).replace( QRegExp( QString( "^%1_" ).arg( inputbase ) ), "" ).replace( QRegExp( "^0*" ) , "" );
      one_over_triangles.push_back( triangles.back().toDouble() != 0e0 ?
                                    1e0 / triangles.back().toDouble() : -1e0 );

      // qDebug( QString( "triangles %1 %2\n" ).arg( triangles.back() ).arg( one_over_triangles.back() ) );
      
      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
         } else {
            output_files << expected[ i ];
         }
      }
   }

   bool do_linear_fit = outfiles.size() > 2;

   {
      QFile f( QString( "%1.csv" ).arg( inputbase ) );

      qDebug( QString( "output file %1\n" ).arg( f.name() ) );

      vector < QStringList > csvresults;
      QStringList csv_header;
      csv_header
            << "Area (A^2)"
            << "True Area (A^2)"
            << "Areafactor"
            << "True Volume (A^3)"
            << "Molecular Weight (g/mol)"
            << "Temperature (K)"
            << "Solvent Viscosity (Poise)"
            << "Number of Surface Patches"
            << "Centroid Coordinates (A) [x]"
            << "Centroid Coordinates (A) [y]"
            << "Centroid Coordinates (A) [z]"
            << "Vector between Center of Resistance/Centroid [x]"
            << "Vector between Center of Resistance/Centroid [y]"
            << "Vector between Center of Resistance/Centroid [z]"
            << "Vector between Centers of Diffusion/Resistance [x]"
            << "Vector between Centers of Diffusion/Resistance [y]"
            << "Vector between Centers of Diffusion/Resistance [z]"
            << "Trace of Dtt Tensor (1/A) [1,1]"
            << "Trace of Dtt Tensor (1/A) [2,2]"
            << "Trace of Dtt Tensor (1/A) [3,3]"
            << "Trace of Symmetric Dtr Tensor (1/A^2) [1,1]"
            << "Trace of Symmetric Dtr Tensor (1/A^2) [2,2]"
            << "Trace of Symmetric Dtr Tensor (1/A^2) [3,3]"
            << "Trace of Drr TENSOR (1/A^3) [1,1]"
            << "Trace of Drr TENSOR (1/A^3) [2,2]"
            << "Trace of Drr TENSOR (1/A^3) [3,3]"
            << "EIGENVALUES OF Drr TENSOR (1/A^3) [1]"
            << "EIGENVALUES OF Drr TENSOR (1/A^3) [2]"
            << "EIGENVALUES OF Drr TENSOR (1/A^3) [3]"
            << "EIGENVALUES OF Drr (1/A^3) 1/3 Trace"
            << "EIGENVALUES OF Drr Anisotropy"
            << "EIGENVALUES OF Drr TENSOR (1/s) [1]"
            << "EIGENVALUES OF Drr TENSOR (1/s) [2]"
            << "EIGENVALUES OF Drr TENSOR (1/s) [3]"
            << "EIGENVALUES OF Drr(Exact) (1/s) 1/3 Trace"
            << "EIGENVALUES OF Dtt TENSOR (1/A) [1]"
            << "EIGENVALUES OF Dtt TENSOR (1/A) [2]"
            << "EIGENVALUES OF Dtt TENSOR (1/A) [3]"
            << "EIGENVALUES OF Dtt (1/A) 1/3 Trace"
            << "EIGENVALUES OF Dtt Anisotropy"
            << "EIGENVALUES OF Dtt TENSOR (cm^2/s) [1]"
            << "EIGENVALUES OF Dtt TENSOR (cm^2/s) [2]"
            << "EIGENVALUES OF Dtt TENSOR (cm^2/s) [3]"
            << "EIGENVALUES OF Dtt(Exact) (cm^2/s) 1/3 Trace"
            << "Vector between Center of Viscosity/Centroid [Dx]"
            << "Vector between Center of Viscosity/Centroid [Dy]"
            << "Vector between Center of Viscosity/Centroid [Dz]"
            << "Intrinsic Viscosity at the Center of Viscosity Chi"
            << "Intrinsic Viscosity at the Center of Viscosity Chi*Volume (A^3)"
            << "Eta (cm^3/g)"
            ;

      set < int > extrapolate;
      for ( int i = 17; i < 44; ++i )
      {
         extrapolate.insert( i );
      }
      for ( int i = 47; i < 50; ++i )
      {
         extrapolate.insert( i );
      }

      for ( int i = 0 ; i < (int) csvfiles.size(); ++i )
      {
         qDebug( QString( "csv input file %1 triangles %2\n" ).arg( csvfiles[ i ] ).arg( triangles[ i ] ) );
         QStringList qsl = best_output_column( csvfiles[ i ] );
         csvresults.push_back( qsl );
      }

      if ( !f.open( IO_WriteOnly ) )
      {
         errormsg = QString( "error opening %1 for output" ).arg( f.name() );
         return false;
      }
      QTextStream ts( &f );
      if ( do_linear_fit )
      {
         ts << ",";
         for ( int j = 0; j < (int) csvresults.size(); ++j )
         {
            ts << ",";
         }
         ts << "\"Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\"" << endl;
      }

      ts << "\"Triangles used\",";
      for ( int i = 0; i < (int) triangles.size(); ++i )
      {
         ts << "=" << triangles[ i ] << ",";
      }
      ts << endl;
      ts << "\"1/Triangles used\",";
      for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
      {
         ts <<  QString( "=%1," ).arg( one_over_triangles[ i ] );
      }
      ts << endl;

      for ( int i = 0; i < (int) csv_header.size(); ++i )
      {
         ts << QString( "\"%1\"," ).arg( csv_header[ i ] );

         bool missing_data = false;

         vector < double > y;
         double a;
         double b;
         double siga;
         double sigb;
         double chi2;

         for ( int j = 0; j < (int) csvresults.size(); ++j )
         {
            if ( (int) csvresults[ j ].size() > i )
            {
               y.push_back( csvresults[ j ][ i ].toDouble() );
               ts << QString( "=%1" ).arg( y.back(), 0, 'g', 8 ) << ",";
            } else {
               ts << "?,";
               missing_data = true;
            }
            qDebug( QString( "i %1 missing_data %2 do_linear_fit %3 extrapolate.count( i ) %4" )
                    .arg( i )
                    .arg( missing_data ? "true" : "false" )
                    .arg( do_linear_fit ? "true" : "false" )
                    .arg( extrapolate.count( i ) ) 
                    );
         }
         if ( !missing_data && do_linear_fit && extrapolate.count( i ) )
         {
            linear_fit( one_over_triangles,
                        y,
                        a,
                        b,
                        siga,
                        sigb,
                        chi2 );
            ts << QString( "=%1,=%2,=%3,=%4,=%5,=%6,=%7" )
               .arg( a,    0, 'g', 8 )
               .arg( siga, 0, 'g', 8 )
               .arg( a != 0 ? fabs( 100.0 * siga / a ) : (double) 0, 0, 'g', 8 )
               .arg( b,    0, 'g', 8 )
               .arg( sigb, 0, 'g', 8 )
               .arg( b != 0 ? fabs( 100.0 * sigb / b ) : (double) 0, 0, 'g', 8 )
               .arg( chi2, 0, 'g', 8 )
               ;
         }
         ts << endl;
      }
      f.close();
      output_files << f.name();
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   return true;
}

QStringList US_Saxs_Util::best_output_column( QString fname )
{
   QFile f( fname );
   QStringList qsl;
   if ( !f.open( IO_ReadOnly ) )
   {
      qsl << QString( "error opening %1" ).arg( f.name() );
      return qsl;
   }

   QRegExp rx_skip( "^\\s*$" );
   QRegExp rx_1 ( "\\s+([0-9.+-E]+)\\s*$" );
   QRegExp rx_3 ( "\\s+([0-9.+-E]+)\\s+([0-9.+-E]+)\\s+([0-9.+-E]+)\\s*$" );
   QRegExp rx_3s( "Dx =\\s+([0-9.+-E]+)\\s+Dy =\\s+([0-9.+-E]+)\\s+Dz =\\s+([0-9.+-E]+)\\s*$" );

   QTextStream ts( &f );
   for ( int i = 0; i < 4; ++i )
   {
      ts.readLine();
   }
   for ( int i = 0; i < 8; ++i )
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( i + 5 );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   for ( int i = 0; i < 2; ++i )
   {
      ts.readLine();
   }   
   for ( int i = 0; i < 3; ++i )
   {
      ts.readLine();
      QString qs = ts.readLine();
      if ( rx_3.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( 2 * i + 5 + 2 + 8 );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }   
   for ( int i = 0; i < 3; ++i )
   {
      ts.readLine();
   }   

   for ( int i = 0; i < 3; ++i )
   {
      ts.readLine();
      for ( int j = 0; j < 3; ++j )
      {
         QString qs = ts.readLine();
         if ( rx_3.search( qs ) == -1 )
         {
            qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( j + i * 5 + 2 * 3 + 5 + 2 + 8 );
            f.close();
            return qsl;
         }
         qsl << rx_3.cap( j + 1 );
      }
      ts.readLine();
   }   
   ts.readLine();
   { // eigenvalues of Drr (1/a^3)
      QString qs = ts.readLine();
      if ( rx_3.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "eigenvalues of Drr (1/a^3)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }      
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Drr (1/A^3) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Drr (1/A^3) Anisotropy" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Drr (1/s)
      QString qs = ts.readLine();
      if ( rx_3.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "eigenvalues of Drr (1/s)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }
   {
      QString qs = ts.readLine();
      qs.replace( QRegExp( "\\s+1/s\\s*$" ), "" );
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Drr (1/s) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Dtt (1/a^3)
      QString qs = ts.readLine();
      if ( rx_3.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "eigenvalues of Dtt (1/a^3)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }      
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Dtt (1/A) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Dtt (1/A) Anisotropy" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Dtt (cm^2/s)
      QString qs = ts.readLine();
      if ( rx_3.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "eigenvalues of Dtt (cm^2/s)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }      
   {
      QString qs = ts.readLine();
      qs.replace( QRegExp( "\\s+cm\\^2/s\\s*$" ), "" );
      qDebug( QString( "qs dtt cm^2/s <%1>\n" ).arg( qs ) );
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Dtt (cm^2/s) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   ts.readLine();
   { // Vector between Center of Viscosity/Centroid
      QString qs = ts.readLine();
      if ( rx_3s.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "Vector between Center of Viscosity/Centroid" );
         f.close();
         return qsl;
      }
      qsl << rx_3s.cap( 1 );
      qsl << rx_3s.cap( 2 );
      qsl << rx_3s.cap( 3 );
   }      
   ts.readLine();
   for ( int i = 0; i < 2; ++i )
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2 %3" ).arg( f.name() ).arg( i ).arg( "CHI" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   {
      QString qs = ts.readLine();
      if ( rx_1.search( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.name() ).arg( "ETA" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   f.close();
   return qsl;
}

bool US_Saxs_Util::strip_pdb( 
                             QString & pdb_stripped,
                             const QString & pdb,
                             const QStringList & exclude_atoms_list,
                             const QStringList & exclude_residues_list,
                             bool exclude_hydrogens
                              )
{
   QString strip_tag = "_s";

   QString base_pdb = QFileInfo( pdb ).baseName();
   pdb_stripped = base_pdb + strip_tag;

   unsigned int ext = 0;
   while ( QFile::exists( pdb_stripped + ".pdb" ) )
   {
      pdb_stripped = base_pdb + strip_tag + QString( "_%1" ).arg( ++ext );
   }
   QString stripped_log = pdb_stripped + "-removed.pdb";
   pdb_stripped +=  ".pdb";
   
   QFile fi( pdb );
   if ( !fi.open( IO_ReadOnly ) )
   {
      errormsg =  QString( "Error: can not read file %1" )
         .arg( pdb );
      return false;
   }

   // outputs
   // ? FIX THIS: should be renamed ? and renamed in output_files

   QFile fo( pdb_stripped );
   if ( !fo.open( IO_WriteOnly ) )
   {
      errormsg =  QString( "Error: can not create file %1" )
         .arg( pdb_stripped );
      fi.close();
      return false;
   }

   QFile fol( stripped_log );
   if ( !fol.open( IO_WriteOnly ) )
   {
      errormsg =  QString( "Error: can not create file %1" )
         .arg( stripped_log );
      fi.close();
      fo.close();
      return false;
   }
   
   // don't know everything yet, but will modify as necessarry

   map < QString, bool > exclude_atoms;
   map < QString, bool > exclude_residues;

   for ( unsigned int i = 0; i < exclude_atoms_list.size(); i++ )
   {
      exclude_atoms[ exclude_atoms_list[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < exclude_residues_list.size(); i++ )
   {
      exclude_residues[ exclude_residues_list[ i ] ] = true;
   }

   QTextStream tsi ( &fi );
   QTextStream tso ( &fo );
   QTextStream tsol( &fol );

   QRegExp rx_check_line( "^(ATOM|HETATM)" );

   QRegExp rx_ter       ( "^(TER)" );

   unsigned int last_chain_residue_no = 0;
   QString      last_key;
   QString      last_chain_id;

   while ( !tsi.atEnd() )
   {
      QString qs = tsi.readLine();
      bool keep = true;
      if ( rx_check_line.search( qs ) != -1 )
      {
         QString residue = qs.mid( 17, 3 );
         QString atom    = qs.mid( 12, 4 );

         if ( exclude_hydrogens &&
              ( qs.mid(12,1) == "H" || qs.mid(13,1) == "H" ) )
         {
            keep = false;
         } else {
            if ( exclude_atoms.count( atom ) ||
                 exclude_residues.count( residue ) )
            {
               keep = false;
            }
         }
      }
      if ( rx_ter.search( qs ) != -1 )
      {
         keep = false;
      }
      if ( keep )
      {
         if ( rx_check_line.search( qs ) != -1 )
         {
            QString      chain_id   = qs.mid( 21, 1 );
            unsigned int residue_no = qs.mid( 22, 4 ).stripWhiteSpace().toUInt();
            QString      this_key   = chain_id + qs.mid( 22, 4 ).stripWhiteSpace();
            // if we start a new chain, we're ok
            if ( chain_id != last_chain_id )
            {
               if ( !last_chain_id.isEmpty() )
               {
                  tso << "TER\n";
               }
               last_chain_id         = chain_id;
               last_key              = this_key;
               last_chain_residue_no = residue_no;
            }
            if ( last_key != this_key )
            {
               last_chain_residue_no++;
               if ( last_chain_residue_no != residue_no )
               {
                  tso << "TER\n";
                  last_chain_id         = chain_id;
                  last_chain_residue_no = residue_no;
               }
               last_key              = this_key;
            }
         }
         tso << qs << endl;
      } else {
         tsol << qs << endl;
      }
   }
   fi.close();
   fo.close();
   fol.close();
   output_files << pdb_stripped;
   output_files << stripped_log;
   return true;
}
