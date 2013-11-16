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
   {
      QString cmd = 
         QString( "%1 -m %2 -r %3 -y %4 -t %5.c3p -v %6.c3v" )
         .arg( progs[ p ] )
         .arg( pdb_stripped )
         .arg( control_parameters[ "bestmsrradiifile" ] )
         .arg( control_parameters[ "bestmsrpatternfile" ] )
         .arg( inputbase )
         .arg( inputbase )
         ;

      if ( control_parameters.count( "bestmsrprober" ) )
      {
         cmd += QString( " -p %1" ).arg( control_parameters[ "bestmsrprober" ] );
      }

      if ( control_parameters.count( "bestmsrfinenessangle" ) )
      {
         cmd += QString( " -f %1" ).arg( control_parameters[ "bestmsrfinenessangle" ] );
      }

      if ( control_parameters.count( "bestmsrcoalescer" ) )
      {
         cmd += QString( " -l %1" ).arg( control_parameters[ "bestmsrcoalescer" ] );
      }

      cmd += QString( " 2> msr_%1.stderr > msr_%2.stdout" ).arg( inputbase ).arg( inputbase );
      qDebug( QString( "best cmd = %1" ).arg( cmd ) );

      cout << "Starting " + progs[ p ] + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + progs[ p ] + "\n";

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
            outfiles << QString( ts.readLine() ).stripWhiteSpace();
            output_files << QString( ts.readLine() ).stripWhiteSpace();
         }
      }
      f.close();
   }

   // run best
   p++;
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
         << outfiles[ i ] + expected_base + "log" 
         << outfiles[ i ] + expected_base + "be" 
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

   return true;
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
