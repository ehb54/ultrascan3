#include "../include/us_saxs_util.h"
#include "../include/us_revision.h"

#define SLASH QDir::separator()

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
      if ( !f.open( IO_ReadOnly ) )
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
   // QFile::remove( constraints_file );
   output_files << constraints_file;
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

   if ( control_parameters.count( "dmdstatic" ) )
   {
      errormsg = QString( "DMDStatic not currently supported\n" ).arg( pdb );
      return false;
   }
      
   // create constraints file
   {
      QFile f( constraints_file );
      if ( !f.open( IO_WriteOnly ) )
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
   QString param_file = base_pdb + ".param";
   QString state_file = base_pdb + ".state";
   QString const_file = base_pdb + ".const";

   QFile::remove( param_file );
   QFile::remove( state_file );
   QFile::remove( const_file );

   QString cmd = 
      QString( "%1 . %2 /dev/null %3 %4 %5 %6 %7\n" )
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
   output_files << param_file;
   output_files << state_file;
   output_files << const_file;

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
   }            
   return true;
}

bool US_Saxs_Util::dmd_strip_pdb()
{
   // remove dmd unrecognized atoms from pdb
   // add -stripped to name
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "DMDStrip: InputFile must be defined";
      return false;
   }

   QString pdb = control_parameters[ "inputfile" ];
   QString base_pdb = QFileInfo( pdb ).baseName();
   QString pdb_stripped = base_pdb + "_stripped";

   unsigned int ext = 0;
   while ( QFile::exists( pdb_stripped + ".pdb" ) )
   {
      pdb_stripped = base_pdb + "_stripped" + QString( "%1" ).arg( ++ext );
   }
   QString stripped_log = pdb_stripped + ".log";
   pdb_stripped +=  ".pdb";
   
   QFile fi( pdb );
   if ( !fi.open( IO_ReadOnly ) )
   {
      errormsg =  QString( "Error: can not read file %1" )
         .arg( pdb );
      return false;
   }

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

   // first off: HOH

   QStringList exclude_atoms_list;
   QStringList exclude_residues_list;
   exclude_residues_list
      << "HOH";
   
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
      if ( keep )
      {
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
