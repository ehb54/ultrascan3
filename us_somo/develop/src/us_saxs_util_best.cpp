#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define MIN_TRIANGLES 12000

bool US_Saxs_Util::run_best()
{
   QStringList save_output_files = output_files;
   QStringList rm_output_files;

   errormsg  = "";
   noticemsg = "";
   QStringList required;
   required 
      << "inputfilenoread"
      << "bestrcoalnmin"
      << "bestrcoalnmax"
      << "bestrcoaln"
      << "bestmsrradiifile"
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
   double  best_fineness    = start_fineness;

   for( double use_fineness = start_fineness; found_triangles <= max_triangles && use_fineness >= 0.3; use_fineness -= 0.02 )
   {
      QString msr_inputbase = QString( "%1_f%2" ).arg( inputbase ).arg( QString( "%1" ).arg( use_fineness ).replace( ".", "_" ) );
      if ( best_inputbase.isEmpty() )
      {
         best_inputbase = msr_inputbase;
      }
      
      QString cmd;

      if ( control_parameters.count( "bestmsrpatternfile" ) )
      {
         cmd = 
            QString( "ulimit -s 16535; %1 -m %2 -r %3 -y %4 -t %5.c3p -v %6.c3v" )
            .arg( progs[ p ] )
            .arg( pdb_stripped )
            .arg( control_parameters[ "bestmsrradiifile" ] )
            .arg( control_parameters[ "bestmsrpatternfile" ] )
            .arg( msr_inputbase )
            .arg( msr_inputbase )
            ;
      } else {
         cmd = 
            QString( "ulimit -s 16535; %1 -m %2 -r %3 -t %4.c3p -v %5.c3v" )
            .arg( progs[ p ] )
            .arg( pdb_stripped )
            .arg( control_parameters[ "bestmsrradiifile" ] )
            .arg( msr_inputbase )
            .arg( msr_inputbase )
            ;
      }         
       
      if ( control_parameters.count( "bestmsrprober" ) )
      {
         cmd += QString( " -p %1" ).arg( control_parameters[ "bestmsrprober" ] );
      }

      //      if ( control_parameters.count( "bestmsrfinenessangle" ) )
      //      {
      //         cmd += QString( " -f %1" ).arg( control_parameters[ "bestmsrfinenessangle" ] );
      //      }
      cmd += QString( " -f %1" ).arg( use_fineness, 0, 'f', 3 );

      if ( control_parameters.count( "bestmsrcoalescer" ) )
      {
         cmd += QString( " -l %1" ).arg( control_parameters[ "bestmsrcoalescer" ] );
      }

      cmd += QString( " 2> msr_%1.stderr > msr_%2.stdout" ).arg( msr_inputbase ).arg( msr_inputbase );
      // us_qdebug( QString( "best cmd = %1" ).arg( cmd ) );

      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.toLatin1().data() );
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
            rm_output_files << expected[ i ];
         }
      }
      // count triangles produced
      {
         QFile f( "msr_" + msr_inputbase + ".stderr" );
         f.open( QIODevice::ReadOnly );
         QTextStream ts( &f );
         bool did_find_triangles = false;
         while ( !ts.atEnd() )
         {
            QString qs = ts.readLine();
            if ( qs.contains( "cannot open fineness file" ) )
            {
               errormsg = QString( "BEST: msroll error with names/radii files %1" ).arg( control_parameters[ "inputfilenoread" ] );
               return false;
            }
               
            if ( qs.contains( "triangles written to disk" ) )
            {
               did_find_triangles = true;
               // QStringList qsl = (qs.trimmed().split( " " , Qt::SkipEmptyParts ) );
               QStringList qsl;
               {
                  QString qs2 = qs.trimmed(); 
                  qsl = (qs2 ).split( " " , Qt::SkipEmptyParts );
               }
               if ( qsl.size() )
               {
                  found_triangles = qsl[ 0 ].toInt();
                  // us_qdebug( QString( "found triangles %1 with fineness %2 on %3" )
                  //         .arg( found_triangles )
                  //         .arg( use_fineness )
                  //         .arg( control_parameters[ "inputfilenoread" ] )
                  //         );
                  if ( use_fineness == start_fineness &&
                       found_triangles > max_triangles )
                  {
                     control_parameters[ "bestmsrfinenessangle" ] =
                        QString( "%1" ).arg( control_parameters[ "bestmsrfinenessangle" ].toDouble() + 0.1e0 );
                     // us_qdebug( QString( "too many triangles with largest fineness, incrementing max fineness to %1 on %2" )
                     //         .arg( control_parameters[ "bestmsrfinenessangle" ] )
                     //         .arg( control_parameters[ "inputfilenoread" ] ) );
                     QFile::remove( pdb_stripped );
                     output_files = save_output_files;
                     for ( int i = 0; i < (int) rm_output_files.size(); ++i )
                     {
                        // us_qdebug( QString( "removing %1" ).arg( rm_output_files[ i ] ) );
                        QFile::remove( rm_output_files[ i ] );
                     }
                     if ( control_parameters[ "bestmsrfinenessangle" ].toDouble() > 2e0 )
                     {
                        errormsg = QString( "BEST: starting fineness too large %1" ).arg( control_parameters[ "inputfilenoread" ] );
                        return false;
                     }
                     return run_best();
                  }

                  if ( found_triangles > best_triangles && found_triangles <= max_triangles )
                  {
                     best_triangles = found_triangles;
                     best_inputbase = msr_inputbase;
                     best_fineness  = use_fineness;
                  }
                  break;
               }
            }
         }
         f.close();
         if ( !did_find_triangles &&
              !found_triangles )
         {
            control_parameters[ "bestmsrfinenessangle" ] =
               QString( "%1" ).arg( control_parameters[ "bestmsrfinenessangle" ].toDouble() + 0.1e0 );
            // us_qdebug( QString( "msroll failed and failed for previous values of fineness, incrementing max fineness to %1 on %2" )
            //         .arg( control_parameters[ "bestmsrfinenessangle" ] ) 
            //         .arg( control_parameters[ "inputfilenoread" ] )
            //         );
            QFile::remove( pdb_stripped );
            output_files = save_output_files;
            for ( int i = 0; i < (int) rm_output_files.size(); ++i )
            {
               // us_qdebug( QString( "removing %1" ).arg( rm_output_files[ i ] ) );
               QFile::remove( rm_output_files[ i ] );
            }
            if ( control_parameters[ "bestmsrfinenessangle" ].toDouble() > 2e0 )
            {
               errormsg = QString( "BEST: starting fineness too large %1" ).arg( control_parameters[ "inputfilenoread" ] );
               return false;
            }

            return run_best();
         }
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
            rm_output_files << expected[ i ];
         }
      }
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   // run rcoal
   QString inputbase23 = QString( "%1_f%2" ).arg( inputbase.left( 17 ) ).arg( QString( "%1" ).arg( best_fineness ).replace( ".", "_" ) );
   p++;
   {
      US_File_Util ufu;
      ufu.copy( QString( "%1.c3p" ).arg( inputbase ), QString( "%1.c3p" ).arg( inputbase23 ), true );
      ufu.copy( QString( "%1.c3v" ).arg( inputbase ), QString( "%1.c3v" ).arg( inputbase23 ), true );

      QString cmd = 
         QString( "%1 -f %2.c3p -nmax %3 -nmin %4 -n %5" )
         .arg( progs[ p ] )
         .arg( inputbase23 )
         .arg( control_parameters[ "bestrcoalnmax" ] )
         .arg( control_parameters[ "bestrcoalnmin" ] )
         .arg( control_parameters[ "bestrcoaln" ] )
         ;

      cmd += QString( " 2> rcoal_%1.stderr > rcoal_%2.stdout" ).arg( inputbase ).arg( inputbase );

      // us_qdebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.toLatin1().data() );
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
            rm_output_files << expected[ i ];
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
      f.open( QIODevice::ReadOnly );
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         if ( qs.contains( "Output files written:" ) )
         {
            do 
            {
               qs = QString( ts.readLine() ).trimmed();
            } while ( qs.isEmpty() && !ts.atEnd() );

            if ( !qs.isEmpty() )
            {
               outfiles << qs;
               output_files << qs;
               rm_output_files << qs;
               // us_qdebug( QString( "outfiles.back() is %1" ).arg( outfiles.back() ) );
            } else {
               errormsg += "could not correctly find output files in rcoal stdout\n";
               // lower max triangles
            }
         }
      }
      f.close();
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }

   if ( !outfiles.size() )
   {
      if ( max_triangles > MIN_TRIANGLES )
      {
         control_parameters[ "bestmsrmaxtriangles" ] = 
            QString( "%1" ).arg( max_triangles - 2500 );
         QFile::remove( pdb_stripped );
         output_files = save_output_files;
         for ( int i = 0; i < (int) rm_output_files.size(); ++i )
         {
            // us_qdebug( QString( "removing %1" ).arg( rm_output_files[ i ] ) );
            QFile::remove( rm_output_files[ i ] );
         }
         return run_best();
      }
      errormsg += QString( "too few triangles %1 after rcoal fail" ).arg( max_triangles );
      return false;
   }

   // make bead models
   {
      double psv         = 0e0;
      float  bead_radius = 0.2f;
      for ( int i = 0; i < (int) outfiles.size(); ++i )
      {
         QFile f( outfiles[ i ] );
         if ( !f.open( QIODevice::ReadOnly ) )
         {
            errormsg += QString( "Could not open rcoal output file %1\n" ).arg( f.fileName() );
            return false;
         }

         QString out;
         int vertices;
         int beads = 0;
         {
            QTextStream ts( &f );
            {
               QString qs = ts.readLine();
               QRegExp rx_vert( "^\\s*(\\d+)\\s+" );
               if ( rx_vert.indexIn( qs ) == -1 )
               {
                  errormsg += QString( "Rcoal output file %1 improper format on line 1 %1 <%2>\n" ).arg( f.fileName() ).arg( qs );
                  f.close();
                  return false;
               }
            
               vertices = rx_vert.cap( 1 ).toInt();
            }
            // out += QString( "%1 %2\n" ).arg( vertices ).arg( psv );
            
            {
               set < QString > used;
               QRegExp rx_data( "^\\s*(\\S+)\\s+(\\S+)\\s+(\\S+)\\s+" );
               for ( int i = 0; i < vertices; ++i )
               {
                  QString qs = ts.readLine();
                  if ( rx_data.indexIn( qs ) == -1 )
                  {
                     errormsg += QString( "Rcoal output file %1 improper format on line %1 %2 <%3>\n" ).arg( i + 2 ).arg( f.fileName() ).arg( qs );
                     f.close();
                     return false;
                  }
                  QString key = QString( "%1 %2 %3" )
                     .arg( rx_data.cap( 1 ) )
                     .arg( rx_data.cap( 2 ) )
                     .arg( rx_data.cap( 3 ) )
                     ;
                  if ( !used.count( key ) )
                  {
                     used.insert( key );
                     beads++;
                     out += QString( "%1 %2 1 1 x 1\n" )
                        .arg( key )
                        .arg( bead_radius )
                        ;
                  }
               }
            }
            f.close();
         }
      
         QFile fo( f.fileName() + ".bead_model" );
         if ( !fo.open( QIODevice::WriteOnly ) )
         {
            errormsg += QString( "Could not open bead model output file %1\n" ).arg( fo.fileName() );
            return false;
         }
         {
            QTextStream ts( &fo );
            ts << QString( "%1 %2\n" ).arg( beads ).arg( psv );
            ts << out;
            fo.close();
         }
         output_files    << fo.fileName();
         rm_output_files << fo.fileName();
      }
   }

   // run best
   p++;
   QStringList csvfiles;
   QStringList triangles;
   vector < double > one_over_triangles;

   QStringList use_outfiles;

   for ( int i = 0; i < (int) outfiles.size(); ++i )
   {
      // us_qdebug( QString( "processing outfile %1" ).arg( outfiles[ i ] ) );
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

      // us_qdebug( QString( "best cmd = %1" ).arg( cmd ) );
      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.toLatin1().data() );
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

      bool is_nan = true;
      {
         QFile f( outfiles[ i ] + expected_base + ".be" );
         if ( f.exists() && f.open( QIODevice::ReadOnly ) )
         {
            is_nan = false;
            QTextStream ts( &f );
            while ( !is_nan && !ts.atEnd() )
            {
               QString qs = ts.readLine();
               if ( qs.contains( "NaN" ) )
               {
                  is_nan = true;
                  noticemsg += QString( "BEST run for %1 produced NaN results\n" ).arg( f.fileName() );
               }
            }
            f.close();
         }
      }

      expected
         << outfiles[ i ] + expected_base + ".log" 
         << outfiles[ i ] + expected_base + ".be" 
         ;

      if ( !is_nan )
      {
         use_outfiles << outfiles[ i ];
         csvfiles  << outfiles[ i ] + expected_base + ".be";
         // us_qdebug( QString( "outfiles[ i ] '%1' inputbase '%2' inputbase23 '%3'" )
         //         .arg( outfiles[ i ] ).arg( inputbase ).arg( inputbase23 ) );
         triangles << QString( outfiles[ i ] ).replace( QRegExp( QString( "^%1_" ).arg( inputbase23 ) ), "" ).replace( QRegExp( "^0*" ) , "" );
         one_over_triangles.push_back( triangles.back().toDouble() != 0e0 ?
                                       1e0 / triangles.back().toDouble() : -1e0 );

         // us_qdebug( QString( "triangles %1 %2 on %3\n" )
         //         .arg( triangles.back() )
         //         .arg( one_over_triangles.back() )
         //         .arg( control_parameters[ "inputfilenoread" ] ) 
         //         );
      } else {
         // us_qdebug( QString( "NaN found on %3\n" )
         //         .arg( control_parameters[ "inputfilenoread" ] ) 
         //         );
         errormsg += QString( "BEST: %1 did produced NaN results in %2 (suggest lowering MSROLL max triangles)\n" )
            .arg( progs[ p ] )
            .arg( outfiles[ i ] + expected_base + ".be" )
            ;
         if ( max_triangles > MIN_TRIANGLES )
         {
            control_parameters[ "bestmsrmaxtriangles" ] = 
               QString( "%1" ).arg( max_triangles - 2500 );
            QFile::remove( pdb_stripped );
            output_files = save_output_files;
            for ( int i = 0; i < (int) rm_output_files.size(); ++i )
            {
               // us_qdebug( QString( "removing %1" ).arg( rm_output_files[ i ] ) );
               QFile::remove( rm_output_files[ i ] );
            }
            return run_best();
         }
      }         
      
      for ( int i = 0; i < (int) expected.size(); ++i )
      {
         if ( !QFile( expected[ i ] ).exists() )
         {
            errormsg += QString( "BEST: %1 did not produce expected output file %2\n" )
               .arg( progs[ p ] )
               .arg( expected[ i ] )
               ;
            if ( max_triangles > MIN_TRIANGLES )
            {
               control_parameters[ "bestmsrmaxtriangles" ] = 
                  QString( "%1" ).arg( max_triangles - 2500 );
               QFile::remove( pdb_stripped );
               output_files = save_output_files;
               for ( int i = 0; i < (int) rm_output_files.size(); ++i )
               {
                  // us_qdebug( QString( "removing %1" ).arg( rm_output_files[ i ] ) );
                  QFile::remove( rm_output_files[ i ] );
               }
               return run_best();
            }
         } else {
            output_files << expected[ i ];
            use_outfiles << outfiles[ i ];
         }
      }
   }

   // us_qdebug( QString( "outfiles %1" ).arg( outfiles.join( ":" ) ) );
   // us_qdebug( QString( "use_outfiles %1" ).arg( use_outfiles.join( ":" ) ) );

   outfiles = use_outfiles;

   bool do_linear_fit = outfiles.size() > 1;

   {
      QFile f( QString( "%1.csv" ).arg( inputbase ) );

      // us_qdebug( QString( "output file %1\n" ).arg( f.fileName() ) );

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
            << "Tau(1) (ns)"
            << "Tau(2) (ns)"
            << "Tau(3) (ns)"
            << "Tau(4) (ns)"
            << "Tau(5) (ns)"
            << "Tau(h) (ns)"
            << "Tau(m) (ns)"
         ;

      set < int > source_for_tau;
      source_for_tau.insert( 31 );
      source_for_tau.insert( 32 );
      source_for_tau.insert( 33 );

      vector < vector < double > > tau_results;

      set < int > extrapolate;
      for ( int i = 17; i < 44; ++i )
      {
         extrapolate.insert( i );
      }
      for ( int i = 47; i < 57; ++i )
      {
         extrapolate.insert( i );
      }

      for ( int i = 0 ; i < (int) csvfiles.size(); ++i )
      {
         // us_qdebug( QString( "csv input file %1 triangles %2\n" ).arg( csvfiles[ i ] ).arg( triangles[ i ] ) );
         QStringList qsl = best_output_column( csvfiles[ i ] );
         csvresults.push_back( qsl );

         // compute tau

         vector < double > this_tau_input;
         vector < double > this_tau_results;
         for ( int j = 0; j < (int) qsl.size(); j++ )
         {
            if ( source_for_tau.count( j ) )
            {
               this_tau_input.push_back( qsl[ j ].toDouble() * 1e-3 );
            }
         }
         if ( this_tau_input.size() != 3 )
         {
            errormsg = QString( "incorrect # of tau inputs %2" ).arg( this_tau_input.size() );
            return false;
         }
         compute_tau( this_tau_input[ 0 ],
                      this_tau_input[ 1 ],
                      this_tau_input[ 2 ],
                      .1,
                      this_tau_results );
         for ( int j = 0; j < (int) this_tau_results.size(); ++j )
         {
            csvresults.back() << QString( "%1" ).arg( this_tau_results[ j ] );
         }
      }

      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg = QString( "error opening %1 for output" ).arg( f.fileName() );
         return false;
      }
      QTextStream ts( &f );
      if ( 1 || do_linear_fit )
      {
         ts << ",";
         for ( int j = 0; j < (int) csvresults.size(); ++j )
         {
            ts << ",";
         }
         ts << "\"Extrapolation to zero triangles (a)\",\"Sigma a\",\"Sigma a %\",\"Slope (b)\",\"Sigma b\",\"Sigma b %\",\"chi^2\"" << Qt::endl;
      }

      ts << "\"Triangles used\",";
      for ( int i = 0; i < (int) triangles.size(); ++i )
      {
         ts << "=" << triangles[ i ] << ",";
      }
      ts << Qt::endl;
      ts << "\"1/Triangles used\",";
      for ( int i = 0; i < (int) one_over_triangles.size(); ++i )
      {
         ts <<  QString( "=%1," ).arg( one_over_triangles[ i ] );
      }
      ts << Qt::endl;

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
               ts << "=-1,";
               missing_data = true;
            }
            // us_qdebug( QString( "i %1 missing_data %2 do_linear_fit %3 extrapolate.count( i ) %4" )
            //         .arg( i )
            //         .arg( missing_data ? "true" : "false" )
            //         .arg( do_linear_fit ? "true" : "false" )
            //         .arg( extrapolate.count( i ) ) 
            //         );
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
         ts << Qt::endl;
      }
      f.close();
      output_files << f.fileName();
   }

   // if ( !errormsg.isEmpty() )
   // {
   //    return false;
   // }

   return true;
}

QStringList US_Saxs_Util::best_output_column( QString fname )
{
   QFile f( fname );
   QStringList qsl;
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      qsl << QString( "error opening %1" ).arg( f.fileName() );
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
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( i + 5 );
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
      if ( rx_3.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( 2 * i + 5 + 2 + 8 );
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
         if ( rx_3.indexIn( qs ) == -1 )
         {
            qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( j + i * 5 + 2 * 3 + 5 + 2 + 8 );
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
      if ( rx_3.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "eigenvalues of Drr (1/a^3)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }      
   {
      QString qs = ts.readLine();
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Drr (1/A^3) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   {
      QString qs = ts.readLine();
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Drr (1/A^3) Anisotropy" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Drr (1/s)
      QString qs = ts.readLine();
      if ( rx_3.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "eigenvalues of Drr (1/s)" );
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
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Drr (1/s) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Dtt (1/a^3)
      QString qs = ts.readLine();
      if ( rx_3.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "eigenvalues of Dtt (1/a^3)" );
         f.close();
         return qsl;
      }
      qsl << rx_3.cap( 1 );
      qsl << rx_3.cap( 2 );
      qsl << rx_3.cap( 3 );
   }      
   {
      QString qs = ts.readLine();
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Dtt (1/A) 1/3 trace" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   {
      QString qs = ts.readLine();
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Dtt (1/A) Anisotropy" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   ts.readLine();
   { // eigenvalues of Dtt (cm^2/s)
      QString qs = ts.readLine();
      if ( rx_3.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "eigenvalues of Dtt (cm^2/s)" );
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
      // us_qdebug( QString( "qs dtt cm^2/s <%1>\n" ).arg( qs ) );
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Dtt (cm^2/s) 1/3 trace" );
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
      if ( rx_3s.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "Vector between Center of Viscosity/Centroid" );
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
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2 %3" ).arg( f.fileName() ).arg( i ).arg( "CHI" );
         f.close();
         return qsl;
      }
      qsl << rx_1.cap( 1 );
   }
   ts.readLine();
   {
      QString qs = ts.readLine();
      if ( rx_1.indexIn( qs ) == -1 )
      {
         qsl << QString( "error in %1 could not read data pos %2" ).arg( f.fileName() ).arg( "ETA" );
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
   if ( !fi.open( QIODevice::ReadOnly ) )
   {
      errormsg =  QString( "Error: can not read file %1" )
         .arg( pdb );
      return false;
   }

   // outputs
   // ? FIX THIS: should be renamed ? and renamed in output_files

   QFile fo( pdb_stripped );
   if ( !fo.open( QIODevice::WriteOnly ) )
   {
      errormsg =  QString( "Error: can not create file %1" )
         .arg( pdb_stripped );
      fi.close();
      return false;
   }

   QFile fol( stripped_log );
   if ( !fol.open( QIODevice::WriteOnly ) )
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

   for ( unsigned int i = 0; i < (unsigned int) exclude_atoms_list.size(); i++ )
   {
      exclude_atoms[ exclude_atoms_list[ i ] ] = true;
   }

   for ( unsigned int i = 0; i < (unsigned int) exclude_residues_list.size(); i++ )
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
      if ( rx_check_line.indexIn( qs ) != -1 )
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
      if ( rx_ter.indexIn( qs ) != -1 )
      {
         keep = false;
      }
      if ( keep )
      {
         if ( rx_check_line.indexIn( qs ) != -1 )
         {
            QString      chain_id   = qs.mid( 21, 1 );
            unsigned int residue_no = qs.mid( 22, 4 ).trimmed().toUInt();
            QString      this_key   = chain_id + qs.mid( 22, 4 ).trimmed();
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
         tso << qs << Qt::endl;
      } else {
         tsol << qs << Qt::endl;
      }
   }
   fi.close();
   fo.close();
   fol.close();
   output_files << pdb_stripped;
   output_files << stripped_log;
   return true;
}
