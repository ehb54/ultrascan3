#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <QTextStream>

#include <signal.h> // for kill()
#include <array>    // for std::array

#if !defined( Q_OS_WIN )
# include <sys/wait.h>
#endif

#define DMD_LINK_RANGE_DEFAULT_PERCENT 1
#define DMD_MAX_BASENAME_LENGTH        30
// #define DMD_DEBUG_MAP_DUMPS

#define TSO QTextStream( stdout )

#define SLASH QDir::separator()

// chain DMD runs (output from last needs to be input to next
// i.e. prepare sets it up and then multi runs chain
// this is done via last_state_file
// fix PDB output file
// If nmr pdb all, try to reload the pdb, but if it's too big ...
// otherwise, make a bunch of individuals
// and load as a "stack" ?
// also, potential renumbering of pdb

#if defined( Q_OS_WIN )

bool US_Saxs_Util::dmd_run_with_log( const QString & 
                                     ,const QString &
                                     ,const QString & ) {
   TSO << "DMD not supported under windows\n";
   exit( -1 );
   return false;
}

#else

bool US_Saxs_Util::dmd_run_with_log( const QString & tag
                                     ,const QString & cmd
                                     ,const QString & log ) {
   if ( !control_parameters.count( "dmdtime" ) || !us_udp_msg ) {
      // no point in forking, as no messaging or no dmdtime provided
      if ( !system( cmd.toLatin1().data() ) ) {
         // dmd commands don't seem to honor zero status exit standards
         // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
         // return false;
      }
      return true;
   }

   if ( int pid = fork() ) {
      int status = system( cmd.toLatin1().data() );
      kill( pid, SIGKILL );
      int options = 0;
      waitpid( pid, &options, 0 );
      if ( !status ) {
         // dmd commands don't seem to honor zero status exit standards
         // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
         // return false;
      }
   } else {
      int max_time = control_parameters[ "dmdtime" ].toInt();
      while( !QFile( log ).exists() ) {
         sleep( 10 );
      }
      FILE *pipe = popen( QString( "tail -f %1" ).arg( log ).toLatin1().data(), "r" );
      array < char, 256 > buffer;
      int last_time = 0;
      while  ( fgets(buffer.data(), 256, pipe) != NULL) {
         QString qs = QString( "%1" ).arg( buffer.data() ).trimmed();
         QStringList qsl = qs.split( QRegExp( "\\s+" ) );
         double this_time = qsl[ 0 ].toFloat();
         if ( (int) this_time != last_time ) {
            double pct = 100.0 * this_time / max_time;
            QString msg = QString( "%1 : %2%" ).arg( tag ).arg( pct, 0, 'g', 2 );
            us_udp_msg->send_json( { { "_progressmsg", msg } } );
            last_time = (int) this_time;
         }
      }
      pclose( pipe );
      exit(0);
   }      
   
   return true;
}   
#endif

bool US_Saxs_Util::dmd_findSS()
{
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "dmd findSS: InputFile must be defined";
      return false;
   }
      
   if ( us_udp_msg ) {
      us_udp_msg->send_json( { { "_progressmsg", "Finding disulphides" } } );
   }

   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "findSS.linux" 
      ;     

   {
      QFileInfo qfi( prog );
      if ( !qfi.exists() )
      {
         errormsg = QString( "DMD program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString( "DMD program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "InputFile file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString constraints_file = dmd_basename + ".SS";

   // outputs
   // FIX THIS: should be renamed ? and renamed in output_files
   QFile::remove( constraints_file );

   QString cmd = 
      QString( "%1 %2 > %3\n" )
      .arg( prog )
      .arg( pdb )
      .arg( constraints_file );

   TSO << "Starting " + prog + "\n";
   TSO << cmd << Qt::endl;
   if ( !system( cmd.toLatin1().data() ) ) {
      // findSS.linux returns non-zero even on success
      // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
      // return false;
   }
   TSO << "Finished " + prog + "\n";

   // findSS creates 1 file: constraints_file

   if ( !QFile::exists( constraints_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( constraints_file );
      return false;
   }

   // read constraints_file into control_parameters[ "DMD:SS" ];

   {
      QFile f( constraints_file );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg =  QString( "Error: %1 can not read file %2" )
            .arg( prog )
            .arg( constraints_file );
         return false;
      }

      control_parameters[ "dmd:ss" ] = "";
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         control_parameters[ "dmd:ss" ] += qs + "\n";
      }
      f.close();
   }
   QFile::remove( constraints_file );
   // output_files << constraints_file;
   TSO << "dmd:ss:\n" << control_parameters[ "dmd:ss" ] << Qt::endl;
   return true;
}

bool US_Saxs_Util::dmd_prepare()
{
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "dmd findSS: InputFile must be defined";
      return false;
   }
      
   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "complex.linux" 
      ;     

   {
      QFileInfo qfi( prog );
      if ( !qfi.exists() )
      {
         errormsg = QString( "DMD program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString( "DMD program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "InputFile file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString constraints_file = dmd_basename + ".constr";

   // if ( control_parameters.count( "dmdstatic" ) )
   // {
   // errormsg = QString( "DMDStatic not currently supported\n" ).arg( pdb );
   // return false;
   // }
      
   // create constraints file
   {
      QFile f( constraints_file );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg =  QString( "Error: %1 can not create file %2" )
            .arg( prog )
            .arg( constraints_file );
         return false;
      }
         
      QTextStream ts( &f );
      if ( control_parameters.count( "dmd:ss" ) )
      {
         ts << control_parameters[ "dmd:ss" ];
      }
      if ( control_parameters.count( "dmd:link" ) )
      {
         ts << control_parameters[ "dmd:link" ];
      }
      if ( control_parameters.count( "dmdstatic" ) &&
           !control_parameters[ "dmdstatic" ].isEmpty() &&
           control_parameters[ "dmdstatic" ] != "none" )
      {
         QStringList qsl = (control_parameters[ "dmdstatic" ] ).split( "," , Qt::SkipEmptyParts );
         for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
         {
            ts << QString( "Static %1\n" ).arg( qsl[ i ] );
         }
      }
      f.close();
   }

   point range;
   if ( !input_dimensions( range ) )
   {
      return false;
   }

   QString qs_range = QString( "%1,%2,%3" )
      .arg( range.axis[ 0 ] )
      .arg( range.axis[ 1 ] )
      .arg( range.axis[ 2 ] );

   TSO << "range string: " << qs_range << Qt::endl;

   // outputs
   // FIX THIS: should be renamed ? and renamed in output_files
   QString param_file    = dmd_basename + ".param";
   QString state_file    = dmd_basename + ".state";
   QString const_file    = dmd_basename + ".const";
   QString topparam_file = dmd_basename + ".topparam";

   QFile::remove( param_file );
   QFile::remove( state_file );
   QFile::remove( const_file );

   // PDMD TODO handle topparam file

   QString cmd = 
      QString( "%1 -P ../common -I %2 -D %3 -p %4 -s %5 -T %6 -C %7 -c %8\n" )
      .arg( prog )
      .arg( pdb )
      .arg( qs_range )
      .arg( param_file )
      .arg( state_file )
      .arg( topparam_file )
      .arg( constraints_file )
      .arg( const_file )
      ;
   
   TSO << "Starting " + prog + "\n";
   TSO << cmd << Qt::endl;
   if ( !system( cmd.toLatin1().data() ) ) {
      // complex.linux returns non-zero even on success
      // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
      // return false;
   }
   TSO << "Finished " + prog + "\n";

   if ( !QFile::exists( param_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( param_file );
      return false;
   }

   if ( !QFile::exists( state_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( state_file );
      return false;
   }

   if ( !QFile::exists( const_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( const_file );
      return false;
   }
   // output_files << param_file;
   // output_files << state_file;
   output_files << const_file;
   last_state_file = state_file;
   return true;
}

bool US_Saxs_Util::input_dimensions( point &range )
{
   point min;
   point max;
   // right now, only 1 model of nmr style file

   current_model = 0;

   if ( current_model > model_vector.size() )
   {
      errormsg = QString( "input dimensions called with current model %1 greater than the number of models loaded %2" )
         .arg( current_model )
         .arg( model_vector.size() );
      return false;
   }

   bool any_set = false;
   {
      unsigned int i = current_model;

      for ( unsigned int j = 0; j < model_vector[ i ].molecule.size(); j++ ) 
      {
         for ( unsigned int k = 0; k < model_vector[ i ].molecule[ j ].atom.size (); k++ )
         {
            PDB_atom *this_atom = &( model_vector[ i ].molecule[ j ].atom[ k ] );

            if ( !any_set )
            {
               for ( unsigned int m = 0; m < 3; m++ ) 
               {
                  min.axis[ m ] = max.axis[ m ] = this_atom->coordinate.axis[ m ] = 0;
               }
               any_set = true;
            } else {
               for ( unsigned int m = 0; m < 3; m++ ) 
               {
                  if ( min.axis[ m ] > this_atom->coordinate.axis[ m ] )
                  {
                     min.axis[ m ] = this_atom->coordinate.axis[ m ];
                  }
                  if ( max.axis[ m ] < this_atom->coordinate.axis[ m ] )
                  {
                     max.axis[ m ] = this_atom->coordinate.axis[ m ];
                  }
               }
            }
         }
      }
   }

   for ( unsigned int m = 0; m < 3; m++ ) 
   {
      range.axis[ m ] = max.axis[ m ] - min.axis[ m ];
   }   

   if ( control_parameters.count( "dmdboxspacing" ) )
   {
      if ( control_parameters[ "dmdboxspacing" ].toLower().contains( QRegExp( "^cubic" ) ) )
      {
         float max_range = range.axis[ 0 ];
         for ( unsigned int m = 1; m < 3; m++ ) 
         {
            if ( max_range < range.axis[ m ] )
            {
               max_range = range.axis[ m ];
            }
         }
         QRegExp rx_cap( "^cubic\\s+(\\+|)(\\S+)$" );
         if ( rx_cap.indexIn( control_parameters[ "dmdboxspacing" ].toLower() ) != -1 )
         {
            if ( rx_cap.cap( 1 ) == "+" )
            {
               max_range += rx_cap.cap( 2 ).toFloat();
            } else {
               max_range = rx_cap.cap( 2 ).toFloat();
            }
         }
         for ( unsigned int m = 0; m < 3; m++ ) 
         {
            range.axis[ m ] = max_range;
         }
      } else {
         QRegExp rx_cap_1( "^\\+(\\S+)$" );
         QRegExp rx_cap_3( "^(\\S+),(\\S+),(\\S+)$" );
         if ( rx_cap_1.indexIn( control_parameters[ "dmdboxspacing" ].toLower() ) == -1 &&
              rx_cap_3.indexIn( control_parameters[ "dmdboxspacing" ].toLower() ) == -1 )
         {
            errormsg = "Parameter for DMDBoxSpacing recognized, must in #,#,# or +#: was: " + control_parameters[ "dmdboxspacing" ];
            return false;
         }
         if ( rx_cap_1.indexIn( control_parameters[ "dmdboxspacing" ].toLower() ) != -1 )
         {
            for ( unsigned int m = 0; m < 3; m++ ) 
            {
               range.axis[ m ] += rx_cap_1.cap( 1 ).toFloat();
            }
         } else {
            for ( unsigned int m = 0; m < 3; m++ ) 
            {
               range.axis[ m ] = rx_cap_3.cap( m + 1 ).toFloat();
            }
         }
      }            
   } else {
      TSO << "Warning: no dmdboxspacing defined, using a default of +10\n";
      for ( unsigned int m = 0; m < 3; m++ ) 
      {
         range.axis[ m ] += 10;
      }
   }

   // add 50%

   for ( unsigned int m = 0; m < 3; m++ )
   {
      range.axis[ m ] *= 1.5;
   }
   
   return true;
}

bool US_Saxs_Util::dmd_strip_pdb()
{
   if ( us_udp_msg ) {
      us_udp_msg->send_json( { { "_progressmsg", "Preparing pdb" } } );
   }

   // dmdstrip is the entry point for dmd runs, so let's try alternative save strategy before exposing via gui
   control_parameters[ "dmdmmlastout" ] = "on";
   control_parameters[ "dmdremoveH"   ] = "on";
   if ( control_parameters.count( "dmdmmlastout" ) ) {
      control_parameters.erase( "pdballmodels" );
   }

   // remove dmd unrecognized atoms from pdb
   // add -stripped to name
   // insert TER for unmarked chain breaks
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "DMDStrip: InputFile must be defined";
      return false;
   }


   QString strip_tag = "_s";

   QString pdb          = control_parameters[ "inputfile" ];
   {
      QString basename = QFileInfo( pdb ).baseName();
      // could get fancier here, perhaps REMARKs could be added or incorporate condensed prior run count
      basename.replace( QRegExp( "_s_(equi|relax_).*$" ), "" );
      dmd_basename     = basename.left( DMD_MAX_BASENAME_LENGTH ) + strip_tag;
   }

   QString pdb_stripped = dmd_basename;

   unsigned int ext = 0;
   while ( QFile::exists( pdb_stripped + ".pdb" ) )
   {
      pdb_stripped = dmd_basename + strip_tag + QString( "_%1" ).arg( ++ext );
   }
   QString topparam     = pdb_stripped + ".topparam";
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
   // FIX THIS: should be renamed ? and renamed in output_files

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

   QFile fot( topparam );
   if ( !fot.open( QIODevice::WriteOnly ) )
   {
      errormsg =  QString( "Error: can not create file %1" )
         .arg( topparam );
      fi.close();
      fo.close();
      fol.close();
      return false;
   }
   
   // read in full tsi
   QStringList qsl_pdb;
   {
      QTextStream tsi ( &fi );
      while ( !tsi.atEnd() ) {
         qsl_pdb << tsi.readLine();
      }
      fi.close();
   }

   QStringList qsl_pdb_removed;
   QStringList qsl_link_constraints;
   if ( !dmd_pdb_prepare( qsl_pdb, qsl_pdb_removed, qsl_link_constraints ) ) {
      errormsg += " dmd_pdb_prepare() failed";
      return false;
   }
   
   if ( qsl_link_constraints.size() ) {
      control_parameters[ "dmd:link" ] = qsl_link_constraints.join( "\n" ) + "\n";
   } else {
      control_parameters.erase( "dmd:link" );
   }

   // create topparam
   {
      QTextStream tsot( &fot );
      for ( auto it = dmd_mol2_res.begin();
            it != dmd_mol2_res.end();
            ++it ) {
         tsot << "MOL " << it->first << " ./" << it->first << ".mol2\n";
      }
      fot.close();
   }

   QTextStream tso ( &fo );
   QTextStream tsol( &fol );
   tso  << qsl_pdb        .join( "\n" ) << "\n";
   tsol << qsl_pdb_removed.join( "\n" ) << "\n";
   fo .close();
   fol.close();
   output_files << pdb_stripped;
   output_files << stripped_log;
   output_files << topparam;
   control_parameters[ "inputfile" ] = pdb_stripped;
   return true;
      
#if defined( OLD_WAY ) 
   // first off: HOH

   QStringList exclude_atoms_list;
   QStringList exclude_residues_list;
   exclude_residues_list
      << "HOH"
      << "WAT"
      ;
   
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

   int qsl_pdb_size = (int) qsl_pdb.size();

   QRegExp rx_check_line( "^(ATOM|HETATM)" );

   QRegExp rx_ter       ( "^(TER)" );

   unsigned int last_chain_residue_no = 0;
   QString      last_key;
   QString      last_chain_id;

   for ( int i = 0; i < qsl_pdb_size; ++i ) {
      QString qs = qsl_pdb[ i ];
      bool keep = true;
      if ( rx_check_line.indexIn( qs ) != -1 )
      {
         QString residue = qs.mid( 17, 3 );
         QString atom    = qs.mid( 12, 4 );
         if ( exclude_atoms.count( atom ) ||
              exclude_residues.count( residue ) )
         {
            keep = false;
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
            TSO << QString( "chain_id [%1] last [%2] residue_no [%3] last [%4] key [%5] last [%6]\n" )
               .arg( chain_id )
               .arg( last_chain_id )
               .arg( residue_no )
               .arg( last_chain_residue_no )
               .arg( this_key )
               .arg( last_key )
               ;
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
   control_parameters[ "inputfile" ] = pdb_stripped;
   return true;
#endif

}

bool US_Saxs_Util::dmd_run( QString run_description )
{
   if ( !control_parameters.count( "dmdtime" ) )
   {
      errormsg = "dmd run: DMDTime must be specified";
      return false;
   }

   if ( !control_parameters.count( "dmdtemp" ) )
   {
      errormsg = "dmd run: DMDTemp must be specified";
      return false;
   }

   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "dmd findSS: InputFile must be defined";
      return false;
   }
      
   QString dmd_heat_xc;
   bool is_relax = run_description.toLower() == "relax";
   
   if ( run_description.toLower() == "relax" )
   {
      if ( control_parameters.count( "dmdrelaxheatxc" ) )
      {
         dmd_heat_xc = control_parameters[ "dmdrelaxheatxc" ];
      } else {
         TSO << "dmd_run: relax heat exchange using default value of 10.0\n";
         dmd_heat_xc = "10.0";
      }
   } else {
      if ( run_description.toLower() == "equi" )
      {
         if ( control_parameters.count( "dmdequiheatxc" ) )
         {
            dmd_heat_xc = control_parameters[ "dmdequiheatxc" ];
         } else {
            TSO << "dmd_run: equi heat exchange using default value of 0.1\n";
            dmd_heat_xc = "0.1";
         }
      } else {
         errormsg = "dmd run: option must be 'relax' or 'equi'";
         return false;
      }
   }
      
   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "xDMD.linux" 
      ;     

   {
      QFileInfo qfi( prog );
      if ( !qfi.exists() )
      {
         errormsg = QString( "DMD program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString( "DMD program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "InputFile file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   // required input files:

   QString param_file   = dmd_basename + ".param";
   QString state_file   = last_state_file;;
   QString const_file   = dmd_basename + ".const";

   if ( !QFile::exists( param_file ) )
   {
      errormsg =  QString( "Error: dmd_run: file %1 does not exist" )
         .arg( param_file );
      return false;
   }

   if ( !QFile::exists( state_file ) )
   {
      errormsg =  QString( "Error: dmd_run: file %1 does not exist" )
         .arg( state_file );
      return false;
   }

   if ( !QFile::exists( const_file ) )
   {
      errormsg =  QString( "Error: dmd_run: file %1 does not exist" )
         .arg( const_file );
      return false;
   }

   // outputs
   // FIX THIS: should be renamed ? and renamed in output_files
   run_description += 
      QString( "_tp%1_tm%2" )
      .arg( control_parameters[ "dmdtemp" ] )
      .arg( control_parameters[ "dmdtime" ] ).replace( ".", "_" );
   last_dmd_description = run_description;

   QString task_file    = dmd_basename + "." + run_description + "_task";
   QString restart_file = dmd_basename + "." + run_description + "_restart";
   QString echo_file    = dmd_basename + "." + run_description + "_echo";
   QString movie_file   = dmd_basename + "." + run_description + "_movie";

   QFile::remove( task_file    );
   QFile::remove( restart_file );
   QFile::remove( echo_file    );
   QFile::remove( movie_file   );

   // create task file
   {
      QFile f( task_file );
      if ( !f.open( QIODevice::WriteOnly ) )
      {
         errormsg =  QString( "Error: %1 can not create file %2" )
            .arg( prog )
            .arg( task_file );
         return false;
      }
         
      QString task;
      
      task +=
         QString( 
                 "#THIS IS THE EXAMPLE INPUT FILE FOR xDMD SIMULATIONS.\n"
                 "#THE COMMENT LINE starts with the 1st non whitespace character as \"#\" or \"//\".\n"
                 "#\n"
                 "#THERMOSTAT: ANDERSON or BERENDSON, right now BERENDSON is not implemented yet\n"
                 "#\n"
                 "#T_NEW is the instant temperature at the moment of simulation. If specified, the\n"
                 "# velocities of input state/restart willbe re-scaled\n"
                 "#\n"
                 "#T_LIMIT is the targeting temperature\n"
                 "#\n"
                 "#HEAT_X_C determines how often the exchange takes place betwen IMAGINARY\n"
                 "# solvent and system atoms: dT = 1/HEAT_X_C.\n"
                 "#\n"
                 "THERMOSTAT              ANDERSON\n"
                 "T_NEW                   %1\n"
                 "T_LIMIT                 %2\n"
                 "HEAT_X_C                %3\n"
                 )
         .arg( control_parameters[ "dmdtemp" ] )
         .arg( control_parameters[ "dmdtemp" ] )
         .arg( dmd_heat_xc )
         ;
      
      task +=
         QString( 
                 "#\n"
                 "#RESTART_FILE specifies the name of the OUTPUT restart file, DEFAULT \"dmd_restart\"\n"
                 "#\n"
                 "#RESTART_DT specifies the delta-time to save each restart file, the file will be\n"
                 "# overwritten, DEFAULT 1000\n"
                 "#\n"
                 "RESTART_FILE            %1\n"
                 "RESTART_DT              10\n"
                 "#\n"
                 "#ECHO_FILE specifies the name of the output file to write the output\n"
                 "# parameters(energy,pressure,tempeature etc.)\n"
                 "#\n"
                 "#ECHO_DT specifies the delta-time to save the output parameters\n"
                 "#\n"
                 "ECHO_FILE               %2\n"
                 "ECHO_DT                 0.1\n"
                 )
         .arg( restart_file )
         .arg( echo_file )
         ;
      
      task += 
         QString(
                 "#\n"
                 "#MOVIE_FILE specifies the movie output file\n"
                 "#\n"
                 "#MOVIE_DT specifies the delta time to save the snapshort\n"
                 "#\n"
                 "#MOVIE_SAVE_START specifies the starting atom to be saved, DEFAULT 1\n"
                 "#\n"
                 "#MOVIE_SAVE_END specifies the endinig atom to be saveed, DEAULT the last atom in the system\n"
                 "#if you has no idea of the system, PLEASE do not specify MOVIE_SAVE_START and MOVIE_SAVE_END\n"
                 "#\n"
                 "MOVIE_FILE              %1\n"
                 "MOVIE_DT                %2\n"
                 "#MOVIE_SAVE_START       1\n"
                 "#MOVIE_SAVE_END         100\n"
                 )
         .arg( movie_file )
         .arg( control_parameters.count( "dmdtimestep" ) ?
               control_parameters[ "dmdtimestep" ] :  
               control_parameters[ "dmdtime" ] )
         ;

      task += 
         QString(
                 "#\n"
                 "#START_TIME specifies the initial time when the simulation starts, DEFAULT 0\n"
                 "#\n"
                 "#MAX_TIME specifies the maximum time for the simulation to last\n"
                 "#\n"
                 "START_TIME              0\n"
                 "MAX_TIME                %1\n"
                 )
         .arg( control_parameters[ "dmdtime" ] );
      
      QTextStream ts( &f );
      ts << task;
      f.close();
   }

   // *************** RUN DMD ****************
   // $DMD/bin/xDMD.linux -p xxx.param -s xxx.state -c xxx.const -i relax.task
   // PDMD TODO : handle cores
   // $DMD/bin/pdmd.linux -m $cores -i $start -p param -s state -c constraint

   // todo 
   QString cmd = 
      QString( "%1 -p %2 -s %3 -c %4 -i %5" )
      .arg( prog )
      .arg( param_file )
      .arg( state_file )
      .arg( const_file )
      .arg( task_file )
      ;
   
   TSO << "Starting " + prog + "\n";
   TSO << cmd << Qt::endl;
   if ( !dmd_run_with_log( is_relax ? "Relaxation" : "Equilibrium",  cmd, echo_file ) ) {
   // if ( !system( cmd.toLatin1().data() ) ) {
      // xDMD.linux returns non-zero even on success
      // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
      // return false;
   }
   TSO << "Finished " + prog + "\n";

   if ( !QFile::exists( restart_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( restart_file );
      return false;
   }

   if ( !QFile::exists( echo_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( echo_file );
      return false;
   }

   if ( !QFile::exists( movie_file ) )
   {
      errormsg =  QString( "Error: %1 did not create file %2" )
         .arg( prog )
         .arg( movie_file );
      return false;
   }
   // output_files << restart_file;
   // output_files << echo_file;
   // output_files << movie_file;

   last_state_file = restart_file;

   // *************** EXTRACT PDBS ****************
   // we are going to go ahead and combine the extraction:
   if ( us_udp_msg ) {
      us_udp_msg->send_json( { { "_progressmsg", QString( "Extracting %1 result pdbs" ).arg( is_relax ? "Relaxation" : "Equilibrium" ) } } );
   }
   if ( control_parameters.count( "dmdtimestep" ) )
   {
      bool dmdmmlastout = control_parameters.count( "dmdmmlastout" );
      QStringList allmodels;
         
      prog =
         env_ultrascan + SLASH +
#if defined(BIN64)
         "bin64"
#else
         "bin"
#endif
         + SLASH
         + "complex_M2P.linux"
         ;     
      
      {
         QFileInfo qfi( prog );
         if ( !qfi.exists() )
         {
            errormsg = QString( "DMD program '%1' does not exist\n" ).arg( prog );
            return false;
         }
         if ( !qfi.isExecutable() )
         {
            errormsg = QString( "DMD program '%1' is not executable\n" ).arg( prog );
            return false;
         }
      }
      
      // $DMD/bin/complex_M2P.linux $DMD/lib/parameter $pdb $topparam dmd_movie out.pdb constraint";
      // $DMD/bin/complex_M2P.linux $DMD/param/ xxx.pdb /dev/null relax.dmd_movie relax.pdbs
      
      QString pdb_out_file        = dmd_basename + "_" + run_description + ".pdb";
      QString pdb_out_to_fix_file = dmd_basename + "_" + run_description + ".pdb-to-fix";
      QString topparam_file       = dmd_basename + ".topparam";
      
      cmd = 
         QString( "%1 ../common %2 %3 %4 %5" )
         .arg( prog )
         .arg( pdb )
         .arg( topparam_file )
         .arg( movie_file )
         .arg( pdb_out_to_fix_file )
         ;
      
      TSO << "Starting " + prog + "\n";
      TSO << cmd << Qt::endl;
      if ( !system( cmd.toLatin1().data() ) ) {
         // complex_M2P.linux returns non-zero even on success
         // errormsg =  QString( "Error: command %1 did not return a successful exit code" ).arg( cmd );
         // return false;
      }
      TSO << "Finished " + prog + "\n";
      
      if ( !QFile::exists( pdb_out_to_fix_file ) )
      {
         errormsg =  QString( "Error: %1 did not create file %2" )
            .arg( prog )
            .arg( pdb_out_to_fix_file );
         return false;
      }

      // fix pdb file, count actual models produced, possibly split into individual pdbs

      unsigned int models = 1;
      {
         QFile fi( pdb_out_to_fix_file );
         if ( !fi.open( QIODevice::ReadOnly ) )
         {
            errormsg =  QString( "Error: %1 can not open file %2 for reading" )
            .arg( prog )
            .arg( pdb_out_to_fix_file );
            return false;
         }

         if ( control_parameters.count( "pdballmodels" ) )
         {
            TSO << "keeping as nmr style pdb\n";
         } else {
            QString omodel = QString( "%1" ).arg( models );
            while ( omodel.length() < 5 )
            {
               omodel = "0" + omodel;
            } 
            pdb_out_file = dmd_basename + "_" + run_description + QString( "_m-%1" ).arg( omodel ) + ".pdb";
         }
         
         QFile *fo;
         fo = new QFile( pdb_out_file );
         if ( !fo->open( QIODevice::WriteOnly ) )
         {
            errormsg =  QString( "Error: %1 can not open file %2 for writing" )
               .arg( prog )
               .arg( pdb_out_file );
            fi.close();
            delete fo;
            return false;
         }

         // read in full tsi
         QStringList qsl_pdb;
         {
            QTextStream tsi ( &fi );
            while ( !tsi.atEnd() ) {
               qsl_pdb << tsi.readLine();
            }
            fi.close();
         }
         {
            QStringList qsl_pdb_restored;
            if ( dmd_pdb_restore( qsl_pdb, qsl_pdb_restored, false ) ) {
               qsl_pdb = qsl_pdb_restored;
            } else {
               return false;
            }
         }
         int qsl_pdb_size = (int) qsl_pdb.size();

         QTextStream *tso;

         tso = new QTextStream( fo );

         *tso << QString( "MODEL        %1\n" ).arg( models );
         if ( dmdmmlastout ) {
            allmodels <<  QString( "MODEL        %1\n" ).arg( models );
         }
         
         for ( int i = 0; i < qsl_pdb_size; ++i ) {
            QString qs = qsl_pdb[ i ];
            *tso << qs << Qt::endl;
            if ( dmdmmlastout ) {
               allmodels << qs << "\n";
            }
            if ( qs.contains( QRegExp( "^ENDMDL" ) ) &&
                 i + 1 < qsl_pdb_size ) {
               models++;
               if ( control_parameters.count( "pdballmodels" ) ) {
                  *tso << QString( "MODEL        %1\n" ).arg( models );
               } else {
                  fo->close();
                  delete tso;
                  delete fo;
                  output_dmd_pdbs << pdb_out_file;
                  if ( !dmdmmlastout ) {
                     output_files << pdb_out_file;
                  }
                  QString omodel = QString( "%1" ).arg( models );
                  while ( omodel.length() < 5 ) {
                     omodel = "0" + omodel;
                  } 
                  pdb_out_file = dmd_basename + "_" + run_description + QString( "_m-%1" ).arg( omodel ) + ".pdb";
                  fo = new QFile( pdb_out_file );
                  if ( !fo->open( QIODevice::WriteOnly ) )
                  {
                     errormsg =  QString( "Error: %1 can not open file %2 for writing" )
                        .arg( prog )
                        .arg( pdb_out_file );
                     delete fo;
                     return false;
                  }
                  tso = new QTextStream( fo );
                  *tso << dmd_pdb_add_back.join("\n") << "\n";
                  *tso << QString( "MODEL        %1\n" ).arg( models );
                  if ( dmdmmlastout ) {
                     allmodels << QString( "MODEL        %1\n" ).arg( models );
                  }
               }
            }
         }

         fo->close();
         delete tso;
         delete fo;

         QFile::remove( pdb_out_to_fix_file );
      }

      if ( !QFile::exists( pdb_out_file ) )
      {
         errormsg =  QString( "Error: %1 did not create file %2" )
            .arg( prog )
            .arg( pdb_out_file );
         return false;
      }
      output_dmd_pdbs << pdb_out_file;
      output_files << pdb_out_file;
      if ( dmdmmlastout ) {
         // also save allmodels
         QRegExp rx_check_line( "^(ATOM|HETATM)" );
         QString pdb_out_file = dmd_basename + "_" + run_description +  "_m-all.pdb";
         QFile fo( pdb_out_file );
         if ( !fo.open( QIODevice::WriteOnly ) ) {
            errormsg =  QString( "Error: %1 can not open file %2 for writing" )
               .arg( prog )
               .arg( pdb_out_file );
            return false;
         }
         QTextStream tso( &fo );
         tso << dmd_pdb_add_back.join( "\n" ) << "\n";
         int allmodels_size = (int) allmodels.size();
         for ( int i = 0; i < allmodels_size; ++i ) {
            QString qs = allmodels[ i ];
            if ( rx_check_line.indexIn( qs ) != -1 &&
                 qs.mid( 76, 2 ).trimmed() == "H" ) {
               continue;
            }
            if ( !qs.trimmed().isEmpty() ) {
               tso << qs.trimmed() << "\n";
            }
         }
         tso <<  "END\n";
         fo.close();
         output_files << pdb_out_file;
      }         
   }
   return true;
}
      
void US_Saxs_Util::dmd_clear() {
   dmd_mol2_res           .clear();
   dmd_chain              .clear();
   dmd_res                .clear();
   dmd_res_link           .clear();
   dmd_org_chain          .clear();
   dmd_org_res            .clear();
   dmd_pdb_prepare_reports.clear();
   dmd_chain_is_hetatm    .clear();
}

QString US_Saxs_Util::dmd_next_res( const QString & source ) {
   // map_dump( "dmd_mol2_res", dmd_mol2_res );
   if ( source.length() != 3 ) {
      errormsg = QString( "dmd_next_res() : resname must be 3 character, given '%1'" ).arg( source );
   }
      
   if ( !dmd_mol2_res.count( source ) ) {
      dmd_mol2_res[ source ] = source;
      return source;
   }

   int size = (int) dmd_mol2_res.size();
   QString res;
   do {
      res = QString( "%1" ).arg( size, 3, 10, QLatin1Char('0'));
      ++size;
   } while( dmd_mol2_res.count( res ) && size < 1000 );
   if ( size >= 1000 ) {
      QTextStream( stderr ) << "ERROR: more than 999 HETATM residues used. Aborting.\n";
      exit(-1);
   }
   dmd_mol2_res[ res ] = source;

   return res;
}

#if defined(DMD_DEBUG_MAP_DUMPS)

static void map_dump( const QString & tag, const map < QString, QString > & mapqsqs ) {
   QTextStream tso(stdout);
   tso << tag << "\n";
   for ( auto it = mapqsqs.begin();
         it != mapqsqs.end();
         ++it ) {
      tso << "'" << it->first << "' : '" << it->second << "'\n";
   }
}

static void map_dump( const QString & tag, const map < QString, map < int, int > > & mapqii ) {
   QTextStream tso(stdout);
   tso << tag << "\n";
   for ( auto it = mapqii.begin();
         it != mapqii.end();
         ++it ) {
      for ( auto it2 = it->second.begin();
            it2 != it->second.end();
            ++it2 ) {

         tso << "'" << it->first << "' : " << it2->first << " -> " << it2->second << "\n";
      }
   }
}

static void map_dump( const QString & tag, const map < QString, QStringList > & mqsqsl ) {
   TSO << tag << "\n";
   for ( auto it = mqsqsl.begin();
         it != mqsqsl.end();
         ++it ) {
      TSO
         << it->first << "\n"
         << it->second.join("\n")
         << "\n"
         ;
   }
}

static void map_dump( const QString & tag, const map < QString, map < int, QString > > & mapqiqs ) {
   QTextStream tso(stdout);
   tso << tag << "\n";
   for ( auto it = mapqiqs.begin();
         it != mapqiqs.end();
         ++it ) {
      for ( auto it2 = it->second.begin();
            it2 != it->second.end();
            ++it2 ) {

         tso << "'" << it->first << "' : " << it2->first << " -> " << it2->second << "\n";
      }
   }
}

static void map_dump( const QString & tag, const map < int, set < int > > & mapisi ) {
   QTextStream tso(stdout);
   tso << tag << "\n";
   for ( auto it = mapisi.begin();
         it != mapisi.end();
         ++it ) {
      tso << it->first << " : ";
      for ( auto it2 = it->second.begin();
            it2 != it->second.end();
            ++it2 ) {
         tso << " " << *it2;
      }
      tso << "\n";
   }
}


static void map_dump( const QString & tag, const set < int > & seti ) {
   QTextStream tso(stdout);
   tso << tag << "\n";
   for ( auto it = seti.begin();
         it != seti.end();
         ++it ) {
      tso << " " << *it;
   }
   tso << "\n";
}

static QString boolstr( const bool & flag ) {
   if ( flag ) {
      return "True";
   }
   return "False";
}

static QString map_dump( const set < int > & seti ) {
   QString result;
   for ( auto it = seti.begin();
         it != seti.end();
         ++it ) {
      result += QString( " %1" ).arg( *it );
   }
   return result;
}

#endif

// map_dump()s needed for return values

static QStringList map_dump( const QString & tag, const map < QString, int > & mqsi, bool also_stdout = true ) {
   QStringList result;
   for ( auto it = mqsi.begin();
         it != mqsi.end();
         ++it ) {
      result << "'" + it->first + "' : '" + QString( "%1" ).arg( it->second );
   }
   if ( also_stdout ) {
      TSO
         << tag
         << "\n"
         << result.join( "\n" )
         << "\n"
         ;
   }
   return result;
}

static QStringList map_dump( const QString & tag, map < QString, map < QString, set < QString > > > & mqsmqssqs, bool also_stdout = true ) {
   QStringList result;
   QString     result_line;
   for ( auto it = mqsmqssqs.begin();
         it != mqsmqssqs.end();
         ++it ) {
      result_line = it->first + ":";
      result << result_line;
      
      for ( auto it2 = it->second.begin();
            it2 != it->second.end();
            ++it2 ) {
         result_line = " chain resseq : " + QString( "%1" ).arg( it2->first ).leftJustified( 6, QChar( ' ' ) ) + " atoms : (" + QString( "%1" ).arg( it2->second.size() ) + ")";
         for ( auto it3 = it2->second.begin();
               it3 != it2->second.end();
               ++it3 ) {
            result_line += " " + *it3;
         }
         result << result_line;
      }
      result << "";
   }
   if ( also_stdout ) {
      TSO
         << tag
         << "\n"
         << result.join( "\n" )
         << "\n"
         ;
   }
   return result;  
}


static QString nth_letter( int n ) {
   if ( n < 1 || n > 26 ) {
      return "*";
   }
   return QString( "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[n-1] );
}

static set < int > visited;

static set < int > connected_vertices( int pos, map < int, set < int > > & chain_links ) {
   // build set of all accessible from pos
   set < int > result = { pos };
   visited.insert( pos );
   if ( !chain_links.count( pos ) ) {
      return result;
   }
   for ( auto it = chain_links[ pos ].begin();
         it != chain_links[ pos ].end();
         ++it ) {
      if ( !visited.count( *it ) ) {
         set < int > linked = connected_vertices( *it, chain_links );
         result.insert( linked.begin(), linked.end() );
         result.insert( *it );
         visited.insert( *it );
      }
   }
   return result;
}

static set < int > connected_vertices_run( int pos, map < int, set < int > > & chain_links ) {
   visited.clear();
   return connected_vertices( pos, chain_links );
}


bool US_Saxs_Util::dmd_pdb_prepare( QString & pdb ) {
   QStringList qsl_pdb;
   QStringList qsl_pdb_removed;
   QStringList qsl_pdb_constraints;

   QFile fi( pdb );
   if ( !fi.open( QIODevice::ReadOnly ) ) {
      errormsg =  QString( "Error: can not read file %1" )
         .arg( pdb );
      return false;
   }

   {
      QTextStream tsi ( &fi );
      while ( !tsi.atEnd() ) {
         qsl_pdb << tsi.readLine();
      }
      fi.close();
   }

   return dmd_pdb_prepare( qsl_pdb, qsl_pdb_removed, qsl_pdb_constraints, false );
}


bool US_Saxs_Util::dmd_pdb_prepare( QStringList & qsl_pdb
                                    ,QStringList & qsl_pdb_removed
                                    ,QStringList & qsl_link_constraints
                                    ,bool production_run ) {
   errormsg  = "";
   noticemsg = "";

   set < QString > remove_hetatms = { "HOH", "WAT" };

   dmd_clear();
   qsl_pdb_removed     .clear();
   qsl_link_constraints.clear();
   dmd_pdb_add_back    .clear();
   QStringList modified_pdb;
   int qsl_pdb_size = (int) qsl_pdb.size();
   if ( !qsl_pdb_size ) {
      errormsg = "dmd_pdb_prepare() : empty pdb provided!";
      return false;
   }

   // link data

   bool link_range_defined   = false;
   bool link_percent_on      = true;
   double link_percent       = DMD_LINK_RANGE_DEFAULT_PERCENT;
   double link_start         = 0;
   double link_end           = 0;

   // HETATM tracking data
   bool first_residue        = true;  // true when we start a model or chain, false after residue
   bool new_residue          = true; 
   int last_resseq           = 0;
   int this_resseq           = 0;
   QString last_chainid      = "";
   QString this_chainid      = "";
   QString this_resname      = "";
   QString hetatm_new_resname;

   // hetatm_struct maps orignal resname and chainid:resseq to the set of atoms contained for consistency validation
   map < QString, map < QString, set < QString > > > hetatm_struct; 
   map < QString, int >                              hetatm_chain_atoms; // (original) chain id to number of atoms
   map < QString, QStringList >                      hetatm_lines;       // lines of HETATM by new resname for mol2 processing
   
   int dmd_resseq_atom       = 0;
   int dmd_resseq_hetatm     = 0;
   int dmd_resseq_atom_link  = 0; // restarts for each chain
  
   int dmd_chain_num         = 1;
   int dmd_chain_num_hetatm  = 1;
   bool last_atom_was_hetatm = false;

   vector < map < QString, QString > > links;
   map < int, QString >                chain_to_id; // for outputing connectivity groups

   for ( int i = 0; i < qsl_pdb_size; ++i ) {
      QString line = qsl_pdb[ i ];
      map < QString, QString > fields = pdb_fields( line );

      // debugging
      // if ( fields[ "recname" ].contains( QRegExp( "^HETATM" ) ) ) {
      //    map_dump( "pdb line: ", fields );
      // }

      if ( fields.count( "error" ) ) {
         errormsg = fields[ "error" ];
         return false;
      }

      // bits that DMD is going to remove anyway
      if ( fields[ "recname" ]
           .contains( QRegExp(
                              "^("
                              "AUTHOR|"
                              "CISPEP|"
                              "COMPND|"
                              "CRYST1|"
                              "DBREF|"
                              "EXPDTA|"
                              "FORMUL|"
                              "HEADER|"
                              "HELIX|"
                              "HET|"
                              "HETNAM|"
                              "JRNL|"
                              "KEYWDS|"
                              "MODRES|"
                              "ORIGX1|"
                              "ORIGX2|"
                              "ORIGX3|"
                              "REVDAT|"
                              "SCALE1|"
                              "SCALE2|"
                              "SCALE3|"
                              "SEQADV|"
                              "SEQRES|"
                              "SHEET|"
                              "SITE|"
                              "SOURCE|"
                              "SSBOND|"
                              "TITLE"
                              ")$" ) ) ) {
         qsl_pdb_removed << line;
         continue;
      }
         
      if ( fields[ "recname" ] == "REMARK" ) {
         if ( fields[ "remarknum" ] == "766" ) {
            links.push_back( fields );
            dmd_pdb_add_back << line;  // could add this for other REMARKS, HELIX, SHEET etc, but needs separate block (!links.push_back)
            modified_pdb << line;
         } else {
            qsl_pdb_removed << line;
         }
         continue;
      }

      if ( fields[ "recname" ].contains( QRegExp( "^(TER|END|ENDMDL)$" ) ) ) {
         first_residue = true;
         last_resseq   = 0;
         last_chainid  = "";
         new_residue   = true;
         
         if ( fields[ "recname" ] == "TER" ) {
            ++dmd_chain_num;
            if ( !last_atom_was_hetatm ) {
               ++dmd_chain_num_hetatm;
               if ( dmd_chain_num_hetatm > 26 ) {
                  errormsg = "too many chains, max 26 currently supported";
                  return false;
               }
            }
         }
      }

      if ( fields[ "recname" ] == "HETATM" ||
           fields[ "recname" ] == "ATOM" ) {

         this_resseq  = fields[ "resseq"  ].toInt();
         this_chainid = fields[ "chainid" ];
         this_resname = fields[ "resname" ];

         if ( first_residue ) {
            first_residue = false;
            new_residue   = true;
            last_resseq   = this_resseq;
            last_chainid  = this_chainid;
            dmd_chain[ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ] = dmd_chain_num;
            if ( fields[ "recname" ] == "ATOM" ) {
               ++dmd_resseq_atom;
               dmd_res      [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]     = dmd_resseq_atom;
               dmd_org_chain[ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_atom ] = fields[ "chainid" ];
               dmd_org_res  [ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_atom ] = fields[ "resseq" ].toInt();
               chain_to_id[ dmd_chain_num ]                                           = fields[ "chainid" ];
            } else {
               ++dmd_resseq_hetatm;
               dmd_res      [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]       = dmd_resseq_hetatm;
               dmd_org_chain[ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_hetatm ] = fields[ "chainid" ];
               dmd_org_res  [ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_hetatm ] = fields[ "resseq" ].toInt();
               chain_to_id[ dmd_chain_num ]                                             = fields[ "chainid" ] + ":" + fields[ "resseq" ];
            }
            dmd_resseq_atom_link = 1;
            dmd_res_link[ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]       = dmd_resseq_atom_link;
         }

         if ( last_resseq != this_resseq ) {
            new_residue = true;
            dmd_chain[ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ] = dmd_chain_num;
            if ( fields[ "recname" ] == "ATOM" ) {
               ++dmd_resseq_atom;
               dmd_res_link [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]     = ++dmd_resseq_atom_link;
               dmd_res      [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]     = dmd_resseq_atom;
               dmd_org_chain[ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_atom ] = fields[ "chainid" ];
               dmd_org_res  [ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_atom ] = fields[ "resseq" ].toInt();
               first_residue = false;
               new_residue   = false;
               last_resseq   = this_resseq;
               last_chainid  = this_chainid;
            } else {
               ++dmd_resseq_hetatm;
               dmd_res_link [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]       = 1;
               dmd_res      [ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ]       = dmd_resseq_hetatm;
               dmd_org_chain[ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_hetatm ] = fields[ "chainid" ];
               dmd_org_res  [ nth_letter( dmd_chain_num_hetatm ) ][ dmd_resseq_hetatm ] = fields[ "resseq" ].toInt();
            }
         }
      } 

      // HETATM processing
      if ( fields[ "recname" ] == "HETATM" ) {

         // check for valid hetatms
         if ( remove_hetatms.count( fields[ "resname" ] ) ) {
            qsl_pdb_removed << line;
            continue;
         }

         dmd_chain_is_hetatm[ fields[ "chainid" ] ].insert( fields[ "resseq" ].toInt() );

         last_atom_was_hetatm = true;
         hetatm_struct[ fields[ "resname" ] ][ fields[ "chainid" ] + ":" + fields[ "resseq" ] ].insert( fields[ "name" ] );
         hetatm_chain_atoms[ fields[ "chainid" ] ]++;

         if ( !first_residue &&
              last_resseq != this_resseq ) {
            modified_pdb << "TER";
            dmd_chain[ fields[ "chainid" ] ][ fields[ "resseq" ].toInt() ] = ++dmd_chain_num;
            chain_to_id[ dmd_chain_num ]                                   = fields[ "chainid" ] + ":" + fields[ "resseq" ];
         }

         if ( new_residue ) {
            hetatm_new_resname = dmd_next_res( this_resname );
            if ( !errormsg.isEmpty() ) {
               return false;
            }
         }

         line.replace( 17, 3, hetatm_new_resname );
         hetatm_lines[ hetatm_new_resname ] << line;

         first_residue = false;
         new_residue   = false;
         last_resseq   = this_resseq;
         last_chainid  = this_chainid;

         modified_pdb << line;
         continue;
      }

      // cache LINKs (we won't know mappings until ATOMs and HETATMs are processed )
      if ( fields[ "recname" ] == "LINK" ) {
         links.push_back( fields );
         dmd_pdb_add_back << line;
      }
      modified_pdb << line;
   }
      
   // hetatm_struct_checks validation
   dmd_pdb_prepare_reports[ "HETATM Chain Detail" ]      = map_dump( "hetatm_struct", hetatm_struct, false );
   dmd_pdb_prepare_reports[ "HETATM Chain Atom Count" ]  = map_dump( "hetatm_chain_atoms", hetatm_chain_atoms, false );
   // map_dump( "hetatm_lines", hetatm_lines );

   // process LINKs
   {
      TSO << "link atompair summary\n";
      
      map < int, set < int > > chain_links;

      int links_size = (int) links.size();
      for ( int i = 0; i < links_size; ++i ) {
         map < QString, QString > fields = links[ i ];
         if ( fields[ "recname" ] == "REMARK" &&
              fields[ "remarknum" ] == "766" ) {
            if ( fields.count( "dmdlinkstart" ) &&
                 fields.count( "dmdlinkend" ) ) {
               link_start         = fields[ "dmdlinkstart"   ].toFloat();
               link_end           = fields[ "dmdlinkend"     ].toFloat();
               link_range_defined = true;
               link_percent_on    = false;
            } else if ( fields.count( "dmdlinkpercent" ) ) {
               link_percent       = fields[ "dmdlinkpercent" ].toFloat();
               link_range_defined = true;
               link_percent_on    = true;
            } else {
               errormsg = "unexpected REMARK 766";
               return false;
            }
            continue;
         }

         // local copies for convenience, resname not needed
         QString name1    = fields[ "name1"    ];
         QString chainid1 = fields[ "chainid1" ];
         int     resseq1  = fields[ "resseq1"  ].toInt();
         QString name2    = fields[ "name2"    ];
         QString chainid2 = fields[ "chainid2" ];
         int     resseq2  = fields[ "resseq2"  ].toInt();
         double  length   = fields[ "length"   ].toFloat();

         // check if valid
         if ( !dmd_chain.count( chainid1 ) ||
              !dmd_chain[ chainid1 ].count( resseq1 ) ) {
            errormsg =
               QString( "dmd_pdb_prepare() : could not find LINK chainid %1 residue sequence %2 in dmd_chain map" )
               .arg( chainid1 )
               .arg( resseq1 )
               ;
            return false;
         }
         if ( !dmd_res_link.count( chainid1 ) ||
              !dmd_res_link[ chainid1 ].count( resseq1 ) ) {
            errormsg =
               QString( "dmd_pdb_prepare() : could not find LINK chainid %1 residue sequence %2 in dmd_res map" )
               .arg( chainid1 )
               .arg( resseq1 )
               ;
            return false;
         }
         if ( !dmd_chain.count( chainid2 ) ||
              !dmd_chain[ chainid2 ].count( resseq2 ) ) {
            errormsg =
               QString( "dmd_pdb_prepare() : could not find LINK chainid %1 residue sequence %2 in dmd_chain map" )
               .arg( chainid2 )
               .arg( resseq2 )
               ;
            return false;
         }
         if ( !dmd_res_link.count( chainid2 ) ||
              !dmd_res_link[ chainid2 ].count( resseq2 ) ) {
            errormsg =
               QString( "dmd_pdb_prepare() : could not find LINK chainid %1 residue sequence %2 in dmd_res map" )
               .arg( chainid2 )
               .arg( resseq2 )
               ;
            return false;
         }

         double use_link_start;
         double use_link_end;

         if ( link_percent_on ) {
            double delta   = length * link_percent / 100;
            use_link_start = length - delta;
            use_link_end   = length + delta;
         } else {
            use_link_start = link_start;
            use_link_end   = link_end;
         }
            
         // build up atomPair line for constraints
         QString constraint =
            QString( "AtomPair %1.%2.%3 %4.%5.%6 %7 %8" )
            .arg( dmd_chain   [ chainid1 ][ resseq1 ] )
            .arg( dmd_res_link[ chainid1 ][ resseq1 ] )
            .arg( name1 )
            .arg( dmd_chain   [ chainid2 ][ resseq2 ] )
            .arg( dmd_res_link[ chainid2 ][ resseq2 ] )
            .arg( name2 )
            .arg( use_link_start, 5, 'f', 3 )
            .arg( use_link_end, 5, 'f', 3 )
            ;

         chain_links[ dmd_chain[ chainid1 ][ resseq1 ] ].insert( dmd_chain[ chainid2 ][ resseq2 ] );
         chain_links[ dmd_chain[ chainid2 ][ resseq2 ] ].insert( dmd_chain[ chainid1 ][ resseq1 ] );
         qsl_link_constraints << constraint;
         // TSO << constraint << Qt::endl;
      }

      // link_check
      {
         // map_dump( "chain_links", chain_links );
         if ( !link_range_defined ) {
            noticemsg += QString( "Notice: using a default link range of %1 centered on the LINK distance provided" )
               .arg( DMD_LINK_RANGE_DEFAULT_PERCENT );
         }

         set < int > found;
         for ( int i = 1; i < dmd_chain_num; ++i ) {
            if ( !found.count( i ) ) {
               set < int > linked = connected_vertices_run( i, chain_links );
               found.insert( linked.begin(), linked.end() );
               QString groupline;
               if ( linked.size() > 1 ) {
                  groupline = "Connected via LINK(s):";
               } else {
                  groupline = "Not connected        :";
               }
               for ( auto it = linked.begin();
                     it != linked.end();
                     ++it ) {
                  if ( chain_to_id.count( *it ) ) {
                     groupline += QString( " %1" ).arg( chain_to_id[ *it ] );
                  } else {
                     groupline += QString( " %1?" ).arg( *it );
                  }
               }
               dmd_pdb_prepare_reports[ "Connectivity" ] << groupline;
            }
         }
      }
   }

   // create pdb files for mol2
   if ( production_run ) {
      for ( auto it = hetatm_lines.begin();
            it != hetatm_lines.end();
            ++it ) {
         QFile fo( QString( "_hetatm_%1.pdb" ).arg( it->first ) );
         if ( !fo.open( QIODevice::WriteOnly ) ) {
            errormsg =
               QString( "Error: dmd_pdb_prepare() can not open file %2 for writing" )
               .arg( fo.fileName() )
               ;
            return false;
         }
         QTextStream tso(&fo);
         tso << it->second.join("\n") << "\n";
         fo.close();
         QString cmd = QString( "babel -ipdb %1 -omol2 > %2.mol2 2> /dev/null" ).arg( fo.fileName() ).arg( it->first );
         if ( system( cmd.toLatin1().data() ) != 0 ) {
            errormsg = QString( "dmd_pdb_prepare() : command %1 failed" ).arg( cmd );
            return false;
         }
         QFile::remove( fo.fileName() ); // clean up
      }
   }

   // QTextStream( stdout ) << "Adjusted:\n" << modified_pdb.join( "\n" ) << "\n";

   // QTextStream( stdout ) << "Removed:\n" << qsl_pdb_removed.join( "\n" ) << "\n";
   // map_dump( "dmd_res", dmd_res );
   // map_dump( "dmd_res_link", dmd_res_link );
   // map_dump( "dmd_chain", dmd_chain );
   // map_dump( "dmd_org_res", dmd_org_res );
   // map_dump( "dmd_org_chain", dmd_org_chain );

   // map_dump( "dmd_pdb_prepare_reports", dmd_pdb_prepare_reports );

   qsl_pdb = modified_pdb;
   return true;
}

bool US_Saxs_Util::dmd_pdb_restore( const QStringList & qsl_pdb, QStringList & qsl_pdb_restored, bool add_back ) {
   int qsl_pdb_size = (int) qsl_pdb.size();
   if ( !qsl_pdb_size ) {
      errormsg = "dmd_pdb_restore() : empty pdb provided!";
      return false;
   }

   bool removeH = false; // control_parameters.count( "dmdremoveH" );

   int startpos = 0;
   if ( add_back ) {
      QString line = qsl_pdb[ 0 ];
      map < QString, QString > fields = pdb_fields( line );
      if ( fields[ "recname" ] == "MODEL" ) {
         ++startpos;
         qsl_pdb_restored << dmd_pdb_add_back;
         qsl_pdb_restored << line;
      } else {
         qsl_pdb_restored = dmd_pdb_add_back;
      }
   }

   for ( int i = startpos; i < qsl_pdb_size; ++i ) {
      QString line = qsl_pdb[ i ];
      map < QString, QString > fields = pdb_fields( line );

      // remove TERs between HETATMs
      if ( fields[ "recname" ] == "TER" &&
           i &&
           i + 1 < qsl_pdb_size ) {
         map < QString, QString > fields_prior = pdb_fields( qsl_pdb[ i - 1 ] );
         map < QString, QString > fields_next  = pdb_fields( qsl_pdb[ i + 1 ] );
         if ( fields_prior[ "recname" ] == "HETATM" &&
              fields_next [ "recname" ] == "HETATM" ) {
            continue;
         }
      }
         
      // right justify element (DMD seems to fail this for HETATMs)
      if ( fields[ "recname" ] == "HETATM" ) {
         line.replace( 76, 2, fields[ "element" ].rightJustified( 2, QChar( ' ' ) ) );
      }

      if ( fields[ "recname" ] == "HETATM" ||
           fields[ "recname" ] == "ATOM" ) {
         QString resname = fields[ "resname" ];
         QString chainid = fields[ "chainid" ];
         int     resseq  = fields[ "resseq"  ].toInt();
         QString element = fields[ "element" ];
         if ( removeH && element == "H" ) {
            continue;
         }

         if ( !dmd_org_chain.count( chainid ) ||
              !dmd_org_chain[ chainid ].count( resseq ) ) {
            errormsg =
               QString( "dmd_pdb_restore() : could not find LINK chainid %1 residue sequence %2 in dmd_org_chain map" )
               .arg( chainid )
               .arg( resseq )
               ;
            return false;
         }
         if ( !dmd_org_res.count( chainid ) ||
              !dmd_org_res[ chainid ].count( resseq ) ) {
            errormsg =
               QString( "dmd_pdb_restore() : could not find LINK chainid %1 residue sequence %2 in dmd_org_res map" )
               .arg( chainid )
               .arg( resseq )
               ;
            return false;
         }

         if ( fields[ "recname" ] == "HETATM" ) {
            if ( !dmd_mol2_res.count( resname ) ) {
               errormsg =
                  QString( "dmd_pdb_restore() : could not find resname %1 in dmd_mol2_res map" )
                  .arg( resname )
                  ;
               return false;
            }
            line.replace( 17, 3, dmd_mol2_res[ resname ].leftJustified( 3, QChar( ' ' ) ) );
         }
         
         line.replace( 21, 1, dmd_org_chain[ chainid ][ resseq ] );
         line.replace( 22, 4, QString( "%1" ).arg( dmd_org_res[ chainid ][ resseq ], 4, 10, QChar( ' ' ) ) );
      }
      qsl_pdb_restored << dmd_fix_hetatm_name_pos( line );
   }

   return true;
}

QString US_Saxs_Util::dmd_fix_hetatm_name_pos( const QString & qs ) {
   auto fields = pdb_fields( qs );
   if ( fields[ "recname" ] != "HETATM" ) {
      return qs;
   }
   QString atomname = fields[ "name" ];
   int len = atomname.length();
   if ( len == 4 ) {
      return qs;
   }
   atomname = " " + atomname;
   while ( atomname.length() < 4 ) {
      atomname += " ";
   }
   QString result = qs.mid( 0, 12 ) + atomname + qs.mid( 16 );
   return result;
}

QStringList US_Saxs_Util::dmd_fix_hetatm_name_pos( const QStringList & qsl ) {
   QStringList result;
   for ( int i = 0; i < (int) qsl.size(); ++i ) {
      result << dmd_fix_hetatm_name_pos( qsl[ i ] );
   }
   return result;
}
      
