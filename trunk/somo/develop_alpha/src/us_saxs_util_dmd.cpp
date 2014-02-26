#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"
//Added by qt3to4:
#include <Q3TextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

// chain DMD runs (output from last needs to be input to next
// i.e. prepare sets it up and then multi runs chain
// this is done via last_state_file
// fix PDB output file
// If nmr pdb all, try to reload the pdb, but if it's too big ...
// otherwise, make a bunch of individuals
// and load as a "stack" ?
// also, potential renumbering of pdb

bool US_Saxs_Util::dmd_findSS()
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

   QString base_pdb = QFileInfo( pdb ).baseName();
   QString constraints_file = base_pdb + ".SS";

   // outputs
   // FIX THIS: should be renamed ? and renamed in output_files
   QFile::remove( constraints_file );

   QString cmd = 
      QString( "%1 %2 > %3\n" )
      .arg( prog )
      .arg( pdb )
      .arg( constraints_file );

   cout << "Starting " + prog + "\n";
   cout << cmd << endl;
   system( cmd.ascii() );
   cout << "Finished " + prog + "\n";

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
      Q3TextStream ts( &f );
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         control_parameters[ "dmd:ss" ] += qs + "\n";
      }
      f.close();
   }
   QFile::remove( constraints_file );
   // output_files << constraints_file;
   cout << "dmd:ss:\n" << control_parameters[ "dmd:ss" ] << endl;
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

   QString base_pdb = QFileInfo( pdb ).baseName();
   QString constraints_file = base_pdb + ".constr";

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
         
      Q3TextStream ts( &f );
      if ( control_parameters.count( "dmd:ss" ) )
      {
         ts << control_parameters[ "dmd:ss" ];
      }
      if ( control_parameters.count( "dmdstatic" ) &&
           !control_parameters[ "dmdstatic" ].isEmpty() &&
           control_parameters[ "dmdstatic" ] != "none" )
      {
         QStringList qsl = QStringList::split( ",", control_parameters[ "dmdstatic" ] );
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

   cout << "range string: " << qs_range << endl;

   // outputs
   // FIX THIS: should be renamed ? and renamed in output_files
   QString param_file = base_pdb + ".param";
   QString state_file = base_pdb + ".state";
   QString const_file = base_pdb + ".const";

   QFile::remove( param_file );
   QFile::remove( state_file );
   QFile::remove( const_file );

   QString cmd = 
      QString( "%1 ../common %2 /dev/null %3 %4 %5 %6 %7\n" )
      .arg( prog )
      .arg( pdb )
      .arg( qs_range )
      .arg( param_file )
      .arg( state_file )
      .arg( constraints_file )
      .arg( const_file )
      ;
   
   cout << "Starting " + prog + "\n";
   cout << cmd << endl;
   system( cmd.ascii() );
   cout << "Finished " + prog + "\n";

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
      if ( control_parameters[ "dmdboxspacing" ].lower().contains( QRegExp( "^cubic" ) ) )
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
         if ( rx_cap.search( control_parameters[ "dmdboxspacing" ].lower() ) != -1 )
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
         if ( rx_cap_1.search( control_parameters[ "dmdboxspacing" ].lower() ) == -1 &&
              rx_cap_3.search( control_parameters[ "dmdboxspacing" ].lower() ) == -1 )
         {
            errormsg = "Parameter for DMDBoxSpacing recognized, must in #,#,# or +#: was: " + control_parameters[ "dmdboxspacing" ];
            return false;
         }
         if ( rx_cap_1.search( control_parameters[ "dmdboxspacing" ].lower() ) != -1 )
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
      cout << "Warning: no dmdboxspacing defined, using a default of +10\n";
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

// TODO: should replace with general strip_pdb() function
bool US_Saxs_Util::dmd_strip_pdb()
{
   // remove dmd unrecognized atoms from pdb
   // add -stripped to name
   // insert TER for unmarked chain breaks

   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "DMDStrip: InputFile must be defined";
      return false;
   }

   QString strip_tag = "_s";

   QString pdb = control_parameters[ "inputfile" ];
   QString base_pdb = QFileInfo( pdb ).baseName();
   QString pdb_stripped = base_pdb + strip_tag;

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
   
   // don't know everything yet, but will modify as necessarry

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

   Q3TextStream tsi ( &fi );
   Q3TextStream tso ( &fo );
   Q3TextStream tsol( &fol );

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
         if ( exclude_atoms.count( atom ) ||
              exclude_residues.count( residue ) )
         {
            keep = false;
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
            cout << QString( "chain_id [%1] last [%2] residue_no [%3] last [%4] key [%5] last [%6]\n" )
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
   control_parameters[ "inputfile" ] = pdb_stripped;
   return true;
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
   if ( run_description.lower() == "relax" )
   {
      if ( control_parameters.count( "dmdrelaxheatxc" ) )
      {
         dmd_heat_xc = control_parameters[ "dmdrelaxheatxc" ];
      } else {
         cout << "dmd_run: relax heat exchange using default value of 10.0\n";
         dmd_heat_xc = "10.0";
      }
   } else {
      if ( run_description.lower() == "equi" )
      {
         if ( control_parameters.count( "dmdequiheatxc" ) )
         {
            dmd_heat_xc = control_parameters[ "dmdequiheatxc" ];
         } else {
            cout << "dmd_run: equi heat exchange using default value of 0.1\n";
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

   QString base_pdb     = QFileInfo( pdb ).baseName();

   // required input files:

   QString param_file   = base_pdb + ".param";
   QString state_file   = last_state_file;;
   QString const_file   = base_pdb + ".const";

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

   QString task_file    = base_pdb + "." + run_description + "_task";
   QString restart_file = base_pdb + "." + run_description + "_restart";
   QString echo_file    = base_pdb + "." + run_description + "_echo";
   QString movie_file   = base_pdb + "." + run_description + "_movie";

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
                 "#T_LIMIT is the targeting temprature\n"
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
      
      Q3TextStream ts( &f );
      ts << task;
      f.close();
   }

   // $DMD/bin/xDMD.linux -p xxx.param -s xxx.state -c xxx.const -i relax.task

   QString cmd = 
      QString( "%1 -p %2 -s %3 -c %4 -i %5" )
      .arg( prog )
      .arg( param_file )
      .arg( state_file )
      .arg( const_file )
      .arg( task_file )
      ;
   
   cout << "Starting " + prog + "\n";
   cout << cmd << endl;
   system( cmd.ascii() );
   cout << "Finished " + prog + "\n";

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

   // we are going to go ahead and combine the extraction:
   if ( control_parameters.count( "dmdtimestep" ) )
   {
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
      
      // $DMD/bin/complex_M2P.linux $DMD/param/ xxx.pdb /dev/null relax.dmd_movie relax.pdbs
      
      QString pdb_out_file        = base_pdb + "_" + run_description + ".pdb";
      QString pdb_out_to_fix_file = base_pdb + "_" + run_description + ".pdb-to-fix";
      
      cmd = 
         QString( "%1 ../common %2 /dev/null %3 %4" )
         .arg( prog )
         .arg( pdb )
         .arg( movie_file )
         .arg( pdb_out_to_fix_file )
         ;
      
      cout << "Starting " + prog + "\n";
      cout << cmd << endl;
      system( cmd.ascii() );
      cout << "Finished " + prog + "\n";
      
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
            cout << "keeping as nmr style pdb\n";
         } else {
            pdb_out_file = base_pdb + "_" + run_description + QString( "_m-%1" ).arg( models ) + ".pdb";
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

         Q3TextStream tsi( &fi );
         Q3TextStream *tso;
         tso = new Q3TextStream( fo );

         *tso << QString( "MODEL        %1\n" ).arg( models );
         
         while ( !tsi.atEnd() )
         {
            QString qs = tsi.readLine();
            *tso << qs << endl;
            if ( qs.contains( QRegExp( "^ENDMDL" ) ) &&
                 !tsi.atEnd() )
            {
               models++;
               if ( control_parameters.count( "pdballmodels" ) )
               {
                  *tso << QString( "MODEL        %1\n" ).arg( models );
               } else {
                  fo->close();
                  delete tso;
                  delete fo;
                  output_dmd_pdbs << pdb_out_file;
                  output_files << pdb_out_file;
                  pdb_out_file = base_pdb + "_" + run_description + QString( "_m-%1" ).arg( models ) + ".pdb";
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
                  tso = new Q3TextStream( fo );
                  *tso << QString( "MODEL        %1\n" ).arg( models );
               }
            }
         }

         fi.close();
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
   }
      
   return true;
}
      
