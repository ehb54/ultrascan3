#include "../include/us_saxs_util.h"
#include "../include/us_saxs_util_asab1.h"
#include "../include/us_hydrodyn_results.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#if defined( CMDLINE )

#if defined( FINISHED_PORTING )
# include "../include/us_surfracer.h"
#endif

// #define DEBUG_DIHEDRAL
// #define DEBUG_TO_HYDRATE_DIHEDRAL
#define MAX_WATER_POSITIONING_ATOMS 4

ostream& operator<<(ostream& out, const rotamer& c)
{
   out << "rotamer name: " << c.name 
       << " residue:"      << c.residue 
       << " extension:" << c.extension << endl;
   
   out << QString( " dihedral angles: %1\n" ).arg( c.dihedral_angles.size() );

   out << QString( " side chain size: %1 elements:" ).arg( c.dihedral_angles.size() );
   for ( unsigned int i = 0; i < c.side_chain.size(); i++ )
   {
      out << 
         QString( " side chain element %1 name %2 %3 %4 %5\n" )
         .arg( i )
         .arg( c.side_chain[ i ].name )
         .arg( c.side_chain[ i ].coordinate.axis[ 0 ] )
         .arg( c.side_chain[ i ].coordinate.axis[ 1 ] )
         .arg( c.side_chain[ i ].coordinate.axis[ 2 ] );
   }

   out << QString( " waters size: %1 elements:" ).arg( c.waters.size() );
   for ( unsigned int i = 0; i < c.waters.size(); i++ )
   {
      out << 
         QString( " waters element %1 name %2 %3 %4 %5\n" )
         .arg( i )
         .arg( c.waters[ i ].name )
         .arg( c.waters[ i ].coordinate.axis[ 0 ] )
         .arg( c.waters[ i ].coordinate.axis[ 1 ] )
         .arg( c.waters[ i ].coordinate.axis[ 2 ] );
   }

   out << QString( " water positioning atoms vector: %1\n" ).arg( c.water_positioning_atoms.size() );
   for ( unsigned int i = 0; i < c.water_positioning_atoms.size(); i++ )
   {
      out << QString( " water positioning atoms pos %1 elements:" ).arg( i );
      for ( unsigned int j = 0; j < c.water_positioning_atoms[ i ].size(); j++ )
      {
         out << " " << c.water_positioning_atoms[ i ][ j ];
      }
      out << endl;
   }

   out << " atom map:\n";
   rotamer tc = c;
   for ( map < QString, rotamer_atom >::iterator it = tc.atom_map.begin();
         it != tc.atom_map.end();
         it++ )
   {
      out << QString( " %1 maps to %2 %3 %4 %5\n" ).arg( it->first )
         .arg( it->second.name )
         .arg( it->second.coordinate.axis[ 0 ] )
         .arg( it->second.coordinate.axis[ 1 ] )
         .arg( it->second.coordinate.axis[ 2 ] )
         ;
   }
   out << " Rotamer end\n";
   return out;
}

bool US_Saxs_Util::pdb_hydrate()
{

   //    if ( !align_test() )
   //    {
   //       return false;
   //    }

   // validate required parameters
   QStringList qsl_required;
   QString missing_required;

   // we don't need all of these for external crysol & foxs (fix later)
   {
      qsl_required << "hydrationfile";
      qsl_required << "residuefile";
      qsl_required << "atomfile";
      qsl_required << "hybridfile";
      qsl_required << "saxsfile";
      qsl_required << "inputfile";
      qsl_required << "asahydratethresh";
      qsl_required << "asathreshpct";
      qsl_required << "asahydrateproberadius";
      qsl_required << "asastep";
      qsl_required << "asacalculation";
      
      for ( unsigned int i = 0; i < (unsigned int) qsl_required.size(); i++ )
      {
         if ( !control_parameters.count( qsl_required[ i ] ) )
         {
            missing_required += " " + qsl_required[ i ];
         }
      }
   }

   asa.hydrate_probe_radius  = control_parameters[ "asahydrateproberadius" ].toFloat();
   asa.asab1_step            = control_parameters[ "asastep"               ].toFloat();
   asa.threshold_percent     = control_parameters[ "asathreshpct"          ].toFloat();

   our_saxs_options.steric_clash_distance = 20.0f;
   our_saxs_options.steric_clash_recheck_distance = 0.0f;
   our_saxs_options.alt_hydration = control_parameters.count( "hydratealt" ) ? true : false;
   our_saxs_options.hydration_rev_asa = control_parameters.count( "hydraterevasa" ) ? true : false;

   if ( control_parameters.count( "hydrationscd" ) )
   {
      our_saxs_options.steric_clash_distance = control_parameters[ "hydrationscd" ].toFloat();
   }

   if ( control_parameters.count( "hydrationrscd" ) )
   {
      our_saxs_options.steric_clash_recheck_distance = control_parameters[ "hydrationrscd" ].toFloat();
   }

   if ( !missing_required.isEmpty() )
   {
      errormsg = QString( "Error: Hydrate requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   // if certain parameters are not set, set them to defaults

   misc_pb_rule_on = control_parameters.count( "pbruleon" ) != 0;

   QStringList checks;
   QStringList vals;

   checks << "pdbmissingatoms";
   vals   << "0";
   checks << "pdbmissingresidues";
   vals   << "0";
   checks << "asamethod";
   vals   << "1";
   
   validate_control_parameters_set_one( checks, vals );

   // cout << rotamers( false );
   // cout << list_pointmaps();

   model_vector = model_vector_as_loaded;

   if ( !residue_list.size() ||
        !model_vector.size() )
   {
      errormsg = "Calculations can not be run until residue & pdb files are read!";
      return false;
   }

   for ( current_model = 0; current_model < model_vector.size(); current_model++ )
   {
      cout << QString( "Hydrating model: %1\n" ).arg( current_model );
      if( !pdb_asa_for_saxs_hydrate() )
      {
         return false;
      }
      // cout << list_exposed();
      // view_exposed();
      build_to_hydrate();
      // cout << list_to_hydrate();
      if ( !compute_to_hydrate_dihedrals() )
      {
         errormsg = "Error computing dihedrals of exposed side chain: " + errormsg;
         return false;
      }
      // cout << list_to_hydrate_dihedrals();
      if ( !compute_best_fit_rotamer() )
      {
         errormsg =  "Error finding best fit rotamer: " + errormsg;
         return false;
      }
      // cout << list_best_fit_rotamer();
      
      if ( !setup_pointmap_rotamers() )
      {
         errormsg = "Error setting up pointmaps: " + errormsg;
         return false;
      }
      // cout << list_pointmap_rotamers();
      
      if ( our_saxs_options.alt_hydration )
      {
         if ( !compute_waters_to_add_alt() )
         {
            errormsg = "Error trying to add waters: " + errormsg;
            return false;
         }
      } else {
         if ( !compute_waters_to_add() )
         {
            errormsg = "Error trying to add waters: " + errormsg;
            return false;
         }
      }

#if defined( USUH_DEBUG_ALIGN )
      cout << list_waters_to_add();
      {
         QString out;
         out = "Waters to add:\n";
         for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
               it != waters_to_add.end();
               it++ )
         {
            out += QString( "%1: " ).arg( it->first );
            for ( unsigned int i = 0; i < it->second.size(); i++ )
            {
               out += QString( " [%1,%2,%3]" )
                  .arg( it->second[ i ].axis[ 0 ] )
                  .arg( it->second[ i ].axis[ 1 ] )
                  .arg( it->second[ i ].axis[ 2 ] )
                  ;
            }
            out += "\n";
         }
         cout << out;
      }
#endif
      if ( !buffer_pdb_with_waters() )
      {
         errormsg = "Error trying to write pdb with waters: " + errormsg;
         return false;
      }
   }
   if ( !flush_pdb() )
   {
      return false;
   }

   // reload as input:
   control_parameters[ "inputfile" ] = last_hydrated_pdb_name;

   misc_pb_rule_on = control_parameters.count( "pbruleon" ) != 0;

   if ( !read_pdb( last_hydrated_pdb_name ) )
   {
      return false;
   }
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

   return true;
}


void US_Saxs_Util::build_to_hydrate()
{
   to_hydrate.clear( );
   to_hydrate_pointmaps.clear( );

   residue_asa.clear( );
   residue_asa_sc.clear( );
   residue_asa_mc.clear( );

   unsigned int i = current_model;

   // pass 1 identify exposed sc's

   map < QString, bool > exposed_sc;
   map < QString, bool > exposed_for_pointmap;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 )
         {
            if ( this_atom->chain == 1 )
            {
               exposed_sc[ QString( "%1~%2~%3" )
                           .arg( this_atom->resName )
                           .arg( this_atom->resSeq )
                           .arg( this_atom->chainID ) ] = true;
            }
            if ( pointmap_atoms.count( this_atom->resName ) )
            {
               exposed_for_pointmap[ QString( "%1~%2~%3" )
                                     .arg( this_atom->resName )
                                     .arg( this_atom->resSeq )
                                     .arg( this_atom->chainID ) ] = true;
            }
         }
      }
   }

   // pass 2 add side chain to to_hydrate map
   QRegExp rx_main_chain("^(N|O)$");

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {

         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         if ( rx_main_chain.indexIn( this_atom->resName ) == -1 
              && exposed_sc.count( mapkey )
              && rotamers.count( this_atom->resName )
              )
         {
            to_hydrate[ mapkey ][ this_atom->name ] = this_atom->coordinate;

            // cout << QString("adding %1 %2 %3 %4\n")
            // .arg(this_atom->resName)
            // .arg(this_atom->resSeq)
            // .arg(this_atom->chainID)
            // .arg(this_atom->name);
         }
         if ( exposed_for_pointmap.count( mapkey ) )
         {
            to_hydrate_pointmaps[ mapkey ][ this_atom->name ] = this_atom->coordinate;
         }
      }
   }

   // pass 3 add up ASA's
   for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
   {
      for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++ )
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         residue_asa[ mapkey ] +=  this_atom->asa;

         if ( this_atom->chain == 1 )
         {
            residue_asa_sc[ mapkey ] +=  this_atom->asa;
         } else {
            residue_asa_mc[ mapkey ] +=  this_atom->asa;
         }
      }
   }  

   // pass 4 compute possible hydration
   hydrate_max_waters_no_asa = 0;

   map < QString, bool > accumulated_residue;

   for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
   {
      for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++ )
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         QString mapkey =
            QString( "%1~%2~%3" )
            .arg( this_atom->resName )
            .arg( this_atom->resSeq )
            .arg( this_atom->chainID );

         if ( !accumulated_residue.count( mapkey ) )
         {
            accumulated_residue[ mapkey ] = true;
            if ( hydrate_count.count( this_atom->resName ) )
            {
               hydrate_max_waters_no_asa += hydrate_count[ this_atom->resName ];
            }
         }
      }
   }

}

bool US_Saxs_Util::compute_to_hydrate_dihedrals()
{
   puts("computing to hydrate dihedrals");
   to_hydrate_dihedrals.clear( );

   vector < point > p(4);
   float dihedral;

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for (  map < QString, map < QString, point > >::iterator it = to_hydrate.begin();
          it != to_hydrate.end();
          it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !dihedral_atoms.count( resName ) )
      {
         errormsg = QString( "No dihedral group found for residue %1." ).arg( resName );
         return false;
      }
#if defined( DEBUG_TO_HYDRATE_DIHEDRAL )
      cout << 
         QString("computation for to hydrate dihedral %1 %2\n")
         .arg( it->first )
         .arg( resName );
#endif
      for ( unsigned int i = 0; i < dihedral_atoms[ resName ].size(); i++ )
      {
         if ( dihedral_atoms[ resName ][ i ].size() != 4 )
         {
            errormsg = 
               QString( "Dihedral table size incorrect, should be 4 but is %1 for %2th dihedral group of %3" )
               .arg( dihedral_atoms[ resName ][ i ].size() )
               .arg( i + 1 )
               .arg( resName );
            return false;
         }

         for ( unsigned int j = 0; j < dihedral_atoms[ resName ][ i ].size(); j++ )
         {
            if ( !( it->second.count( dihedral_atoms[ resName ][ i ][ j ] ) ) )
            {
               errormsg = 
                  QString( "Side chain %1 is missing required dihedral atom %2" )
                  .arg( it->first )
                  .arg( dihedral_atoms[ it->first ][ i ][ j ] );
               return false;
            }
            p[ j ] = it->second[ dihedral_atoms[ resName ][ i ][ j ] ];
         }
         dihedral = acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                               plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) );
         to_hydrate_dihedrals[ it->first ].push_back( dihedral );
      }
   }
   puts("done computing to hydrate dihedrals");
   return true;
}

QString US_Saxs_Util::list_to_hydrate_dihedrals()
{
   QString out = "";
   for ( map < QString, vector < float > >::iterator it = to_hydrate_dihedrals.begin();
         it != to_hydrate_dihedrals.end();
         it++ )
   {
      out += QString( "Dihedrals for: %1 " ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString( " %1" ).arg( it->second[ i ] );
      }
      out += "\n";
   }
   return out;
}

QString US_Saxs_Util::list_to_hydrate( bool coords )
{
   QString out = "to hydrate via dihedrals:\n";
   for ( map < QString, map < QString, point > >::iterator it = to_hydrate.begin();
         it != to_hydrate.end();
         it++ )
   {
      out += QString( "Side chains for: %1\n" ).arg( it->first );
      for ( map < QString, point >::iterator it2 = it->second.begin();
            it2 != it->second.end();
            it2++ )
      {
         out += QString(" %1").arg( it2->first );
         if ( coords )
         {
            out += QString( " [%1,%2,%3]" )
               .arg( it2->second.axis[ 0 ] )
               .arg( it2->second.axis[ 1 ] )
               .arg( it2->second.axis[ 2 ] );
         }
      }
      out += "\n";
   }

   out += "to hydrate via pointmaps:\n";
   for ( map < QString, map < QString, point > >::iterator it = to_hydrate_pointmaps.begin();
         it != to_hydrate_pointmaps.end();
         it++ )
   {
      out += QString( "pointmap chains for: %1\n" ).arg( it->first );
      for ( map < QString, point >::iterator it2 = it->second.begin();
            it2 != it->second.end();
            it2++ )
      {
         out += QString(" %1").arg( it2->first );
         if ( coords )
         {
            out += QString( " [%1,%2,%3]" )
               .arg( it2->second.axis[ 0 ] )
               .arg( it2->second.axis[ 1 ] )
               .arg( it2->second.axis[ 2 ] );
         }
      }
      out += "\n";
   }


   // asa's

   out += "ASA:\n";
   for ( map < QString, float >::iterator it = residue_asa.begin();
         it != residue_asa.end();
         it++ )
   {
      out += QString( "%1 tot=%2 sc=%3 mc=%4\n" )
         .arg( it->first )
         .arg( it->second )
         .arg( residue_asa_sc.count( it->first ) ?
               residue_asa_sc[ it->first ] : 0.0 )
         .arg( residue_asa_mc.count( it->first ) ?
               residue_asa_mc[ it->first ] : 0.0 )
         ;
   }

   return out;
}         

QString US_Saxs_Util::list_exposed()
{
   unsigned int i = current_model;
   QString out = "exposed side chain atom list:\n";
   map < QString, bool > exposed_sc;
   map < QString, bool > exposed_for_pointmap;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
   {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
      {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( this_atom->exposed_code == 1 )
         {
            if ( this_atom->chain == 1 )
            {
               out +=
                  QString("%1 %2 %3 %4 %5\n")
                  .arg(this_atom->serial)
                  .arg(this_atom->name)
                  .arg(this_atom->resName)
                  .arg(this_atom->chainID)
                  .arg(this_atom->resSeq);
               exposed_sc[ QString( "%1~%2~%3" )
                           .arg( this_atom->resName )
                           .arg( this_atom->resSeq )
                           .arg( this_atom->chainID ) ] = true;
            }
            if ( pointmap_atoms.count( this_atom->resName ) )
            {
               out +=
                  QString("%1 %2 %3 %4 %5 pointmap\n")
                  .arg(this_atom->serial)
                  .arg(this_atom->name)
                  .arg(this_atom->resName)
                  .arg(this_atom->chainID)
                  .arg(this_atom->resSeq);
               exposed_for_pointmap[ QString( "%1~%2~%3" )
                                     .arg( this_atom->resName )
                                     .arg( this_atom->resSeq )
                                     .arg( this_atom->chainID ) ] = true;
            }
         }
      }
   }

   for ( map < QString,bool >::iterator it = exposed_sc.begin();
         it != exposed_sc.end();
         it++ )
   {
      out += it->first + "\n";
   }

   for ( map < QString,bool >::iterator it = exposed_for_pointmap.begin();
         it != exposed_for_pointmap.end();
         it++ )
   {
      out += it->first + "\n";
   }
   return out;
}


bool US_Saxs_Util::load_rotamer( QString filename )
{
   rotamers.clear( );
   dihedral_atoms.clear( );

   pointmap_atoms.clear( );
   pointmap_atoms_dest.clear( );
   pointmap_atoms_ref_residue.clear( );

   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg =  QString( "Error: rotamer file not found %1:" ).arg( filename );
      return false;
   }
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg =  QString( "Error: rotamer file %1 is not readable. Check permissions" ).arg( filename );
      return false;
   }      

   QStringList qsl;
   QTextStream ts( &f );
   
   while ( !ts.atEnd() )
   {
      QString qs = ts.readLine();
      if ( qs.toLower().contains( QRegExp( "^end-file" ) ) )
      {
         break;
      }
      qsl << qs;
   }

   f.close();

   QRegExp rx_whitespace("\\s+");
   QRegExp rx_skip("^(#|\\s*$)");
   QRegExp rx_main_chain("^(N|O)$");
   QRegExp rx_atom("^ATOM");

   bool in_rotamer = false;
   bool in_rotamer_waters = false;

   rotamer tmp_rotamer;

   for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
   {
      if ( rx_skip.indexIn( qsl[ i ] ) != -1 )
      {
         continue;
      }
      if ( qsl[ i ].length() > 30 && qsl[ i ].contains( rx_atom ) )
      {
#if QT_VERSION >= 0x040000
         qsl[ i ].data()[ 22 ] = ' ';
#else
         qsl[ i ].at( 22 ) = ' ';
#endif
      }
      QStringList qsl_line = (qsl[ i ] ).split( rx_whitespace , Qt::SkipEmptyParts );
      if ( qsl_line[ 0 ] == "multiple-rotate:" )
      {
         if ( in_rotamer )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate token can not occur within a rotamer definition" )
               .arg( i + 1 );
            return false;
         }            

         qsl_line.pop_front();

         if ( qsl_line.size() < 5 )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate: requires at least 5 arguments" )
               .arg( i + 1 );
            return false;
         }

         if ( ( qsl_line.size() - 2 ) % 3 )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate: requires exactly 2 + 3n arguments, n >= 1" )
               .arg( i + 1 );
            return false;
         }

         QString name                = qsl_line[ 0 ];
         QString residue             = name.left( 3 );
         unsigned int rotate         = qsl_line[ 1 ].toUInt();

         if ( rotated_rotamers.count( name ) )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate: %2 previously multiple-rotate'd" )
               .arg( i + 1 )
               .arg( name );
            return false;
         }

         if ( !rotamers.count( residue ) )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 must be previously defined" )
               .arg( i + 1 )
               .arg( residue );
            return false;
         }

         if ( rotate <= 1 )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  multiple-rotate: rotate count %2, must be 2 or greater" )
               .arg( i + 1 )
               .arg( rotate );
            return false;
         }

         unsigned int water_count = ( qsl_line.size() - 2 ) / 3;
#if defined( UHH_DEBUG_RR )
         cout << QString( "multiple-rotate %1 water count %2\n" ).arg( name ).arg( water_count );
#endif
         // // we need the rotamer name matched one
         // this info is not available yet, needs compute water_positioning atoms to complete
         
         //          if ( water_count != rotamers[ residue ][ 0 ].water_positioning_atoms.size() )
         //          {
         //             errormsg = 
         //                QString( "Error in hydrated rotamer file line %1.  multiple-rotate: difference in water count (%2) with reference rotamer water count (%3)" )
         //                .arg( i + 1 )
         //                .arg( water_count )
         //                .arg( rotamers[ residue ][ 0 ].water_positioning_atoms.size() );
         //             return false;
         //          }

         rotated_rotamers[ name ].resize( water_count );

         for ( unsigned int water_number = 0; water_number < water_count; water_number++ )
         {
            vector < QString >  atoms( 3 );
            atoms[ 0 ]                  = qsl_line[ 2 + 3 * water_number ];
            atoms[ 1 ]                  = qsl_line[ 3 + 3 * water_number ];
            atoms[ 2 ]                  = qsl_line[ 4 + 3 * water_number ];

            bool         found_match = false;
            unsigned int pos;
            for ( unsigned int j = 0; j < rotamers[ residue ].size(); j++ )
            {
               // cout << QString( "rotamers '%1' %2 name '%3'\n" )
               // .arg( residue )
               // .arg( j )
               // .arg( rotamers[ residue ][ j ].name );
               if ( rotamers[ residue ][ j ].name == name )
               {
                  found_match = true;
                  pos = j;
                  break;
               }
            }
            if ( !found_match )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, but full name %3 missing" )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( name );
               return false;
            }
            
#if defined( UHH_DEBUG_RR )
            cout << QString( "multiple-rotate %1 x%2 found rotamer match %3 pos %4 for water %5\n" )
               .arg( name )
               .arg( rotate )
               .arg( residue )
               .arg( pos )
               .arg( water_number )
               ;
#endif
         
            rotamer ref_rotamer = rotamers[ residue ][ pos ];

            // check existance & set atom pos for first two
            vector < unsigned int > atom_pos( 3 );

            for ( unsigned int j = 0; j < 2; j++ )
            {
               bool found_sc_atom = false;
               for ( unsigned int k = 0; k < ref_rotamer.side_chain.size(); k++ )
               {
                  if ( ref_rotamer.side_chain[ k ].name == atoms[ j ] )
                  {
                     found_sc_atom = true;
                     atom_pos[ j ] = k;
                     break;
                  }
               }
               if ( !found_sc_atom )
               {
                  errormsg = 
                     QString( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, atom %3 missing" )
                     .arg( i + 1 )
                     .arg( residue )
                     .arg( atoms[ j ] );
                  return false;
               }
            }               
               
            if ( ref_rotamer.waters.size() <= water_number )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found reference rotamer, but it has %3 waters, less than %4 requested" )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( ref_rotamer.waters.size() )
                  .arg( water_number + 1 )
                  ;
               return false;
            }
            
            // check existance of atoms[ 2 ]
         
            if ( ref_rotamer.waters[ water_number ].name != atoms[ 2 ] )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  multiple-rotate: residue: %2 found, atom %3 missing from waters" )
                  .arg( i + 1 )
                  .arg( residue )
                  .arg( atoms[ 2 ] );
               return false;
            }
            
            // now build up rotation into

            // compute distance & angle

            vector < point > p( 3 );
            p[ 0 ] = ref_rotamer.side_chain[ atom_pos[ 0 ] ].coordinate;
            p[ 1 ] = ref_rotamer.side_chain[ atom_pos[ 1 ] ].coordinate;
            p[ 2 ] = ref_rotamer.waters    [ water_number  ].coordinate;

            float d01 = dist( p[ 0 ], p[ 1 ] );
            float d12 = dist( p[ 1 ], p[ 2 ] );

#if defined( UHH_DEBUG_RR )
            float d02 = dist( p[ 0 ], p[ 2 ] );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( d01 );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( d12 );

            cout << QString( "distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 2 ] )
               .arg( d02 );
#endif

            float a = acosf( dot( normal( minus( p[ 0 ], p[ 1 ] ) ),
                                  normal( minus( p[ 2 ], p[ 1 ] ) ) ) );
      
#if defined( UHH_DEBUG_RR )
            cout << QString( "Angle %1,%2,%3 = %4 or %5 degrees\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( a )
               .arg( a * 180.0 / M_PI )
               ;
#endif

            // transform reference rotamer from p(3) to 
            // (0,0,-d01), (0,0,0), ( d12 * cos( a ), 0, d12 * sin( a ) )
            float xdist = d12 * cos( a - M_PI / 2.0 );

            vector < point > np( 3 );
            np[ 1 ].axis[ 0 ] = 0.0;
            np[ 1 ].axis[ 1 ] = 0.0;
            np[ 1 ].axis[ 2 ] = 0.0;

            np[ 0 ]           = np[ 1 ];
            np[ 0 ].axis[ 2 ] = - d01;

            np[ 2 ].axis[ 0 ] = xdist;
            np[ 2 ].axis[ 1 ] = 0.0;
            np[ 2 ].axis[ 2 ] = d12 * sin( a - M_PI / 2.0 );
         
#if defined( UHH_DEBUG_RR )
            cout << QString( "transform from: " ) <<  p[ 0 ] << " " <<  p[ 1 ] << " " <<  p[ 2 ] << endl;
            cout << QString( "transform to: " )   << np[ 0 ] << " " << np[ 1 ] << " " << np[ 2 ] << endl;

            float nd01 = dist( np[ 0 ], np[ 1 ] );
            float nd12 = dist( np[ 1 ], np[ 2 ] );
            float nd02 = dist( np[ 0 ], np[ 2 ] );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( nd01 );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( nd12 );

            cout << QString( "new distance %1 to %2 = %3 A\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 2 ] )
               .arg( nd02 );

            float na = acosf( dot( normal( minus( np[ 0 ], np[ 1 ] ) ),
                                   normal( minus( np[ 2 ], np[ 1 ] ) ) ) );
      
            cout << QString( "new: Angle %1,%2,%3 = %4 or %5 degrees\n" )
               .arg( atoms[ 0 ] )
               .arg( atoms[ 1 ] )
               .arg( atoms[ 2 ] )
               .arg( na )
               .arg( na * 180.0 / M_PI )
               ;
#endif

            // now setup and do transformation
            vector < point > ref_rotamer_p;
            for ( unsigned int j = 0; j < ref_rotamer.side_chain.size(); j++ )
            {
               ref_rotamer_p.push_back( ref_rotamer.side_chain[ j ].coordinate );
            }
            for ( unsigned int j = 0; j < ref_rotamer.waters.size(); j++ )
            {
               ref_rotamer_p.push_back( ref_rotamer.waters[ j ].coordinate );
            }

            vector < point > ref_rotamer_np( ref_rotamer_p.size() );
            if ( !atom_align( p, np, ref_rotamer_p, ref_rotamer_np ) )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  multiple-rotate: " ).
                  arg( i + 1 ) + errormsg;
               return false;
            }
            rotamer new_ref_rotamer = ref_rotamer;
            // not avail yet
            // new_ref_rotamer.water_positioning_atoms.push_back( ref_rotamer.water_positioning_atoms[ water_number ] );
            for ( unsigned int j = 0; j < ref_rotamer.side_chain.size(); j++ )
            {
               new_ref_rotamer.side_chain[ j ].coordinate = ref_rotamer_np[ j ];
               new_ref_rotamer.atom_map[ new_ref_rotamer.side_chain[ j ].name ].coordinate =
                  new_ref_rotamer.side_chain[ j ].coordinate;
            }
            
            // now we are putting each water in its own ref rotamer since
            // each is being multiply rotated and
            // it is a water of memory to store rot**waters variants
            // for ( unsigned int j = 0; j < ref_rotamer.waters.size(); j++ )
            // {
            new_ref_rotamer.waters.resize( 1 );
            new_ref_rotamer.waters[ 0 ].coordinate = ref_rotamer_np[ ref_rotamer.side_chain.size() + water_number ];
            // }

            float delta_rho = ( 2.0 * M_PI ) / (float) rotate;

            for ( unsigned int j = 0; j < rotate; j++ )
            {
               float rho =  (float) j * delta_rho;
            
               new_ref_rotamer.waters[ 0 ].coordinate.axis[ 0 ] = xdist * cos( rho );
               new_ref_rotamer.waters[ 0 ].coordinate.axis[ 1 ] = xdist * sin( rho );

               // FIX THIS!!!    check rotamer for internal steric clash!

               rotated_rotamers[ name ][ water_number ].push_back( new_ref_rotamer );
            }

#if defined( UHH_DEBUG_RR )
            cout << QString( "multiple-rotate: created %1 rotamers for %2\n" )
               .arg( rotated_rotamers[ name ].size() )
               .arg( name );
#endif
         } // water number

         // #define UHH_LIST_NMR_MODELS
#if defined( UHH_LIST_NMR_MODELS )
         // create nmr style pdb for rotamer

         QString out;

         // now we have a vector < vector < rotamer > > for this name 
         // where the waters are each top level vector
         // i.e. we need to linearize the choices of this 
         // but each chain ref is rotated into a different coordinate frame, so
         // each one needs to be added separately, so
         // we will just list each vector as a separate model


         for ( unsigned int jj = 0; jj < rotated_rotamers[ name ].size(); jj++ )
         {
            out += QString( "REMARK  water %1\n" ).arg( jj + 1 );
            for ( unsigned int j = 0; j < rotated_rotamers[ name ][ jj ].size(); j++ )
            {
               out += QString( "MODEL     %1\n" ).arg( j + 1 );

               unsigned int apos = 0;

               for ( unsigned int k = 0; k < rotated_rotamers[ name ][ jj ][ j ].side_chain.size(); k++ )
               {
                  out += QString( "" )
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                              ++apos,
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].name.toLatin1().data(),
                              name.left( 3 ).toLatin1().data(),
                              "A",
                              1,
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 0 ],
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 1 ],
                              rotated_rotamers[ name ][ jj ][ j ].side_chain[ k ].coordinate.axis[ 2 ],
                              1.00,
                              0.00,
                              " "
                              );
               }
               out += "TER\n";
               for ( unsigned int k = 0; k < rotated_rotamers[ name ][ jj ][ j ].waters.size(); k++ )
               {
                  out += QString( "" )
                     .sprintf(     
                              "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                              ++apos,
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].name.toLatin1().data(),
                              "WAT",
                              "B",
                              2,
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 0 ],
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 1 ],
                              rotated_rotamers[ name ][ jj ][ j ].waters[ k ].coordinate.axis[ 2 ],
                              1.00,
                              0.00,
                              " "
                              );
               }
               out += "TER\nENDMDL\n";
            }
            out += "END\n";
            out += "REMARK  --- split models -----------\n";
         }

         cout << "--- start nmr style PDB-----------\n" 
              << out 
              << "--- end nmr style PDB-----------\n";
#endif //  defined( UHH_LIST_NMR_MODELS )
         continue;
      } // multiple-rotate

      if ( qsl_line[ 0 ] == "pointmap:" )
      {
         qsl_line.pop_front();
         QString res = qsl_line[ 0 ];
         qsl_line.pop_front();
         if ( pointmap_atoms.count( res ) )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  %2 was previously defined as a pointmap" )
               .arg( i + 1 )
               .arg( res );
            return false;
         }
            
         while ( qsl_line.size() )
         {
            unsigned int points = qsl_line[ 0 ].toUInt();
            qsl_line.pop_front();
            if ( points < 3 )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  Invalid number of points for a map on pointmap line" )
                  .arg( i + 1 );
               return false;
            }
            vector < QString > atoms;
            for ( unsigned int j = 0; j < points; j++ )
            {
               if ( !qsl_line.size() )
               {
                  errormsg = 
                     QString( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" )
                     .arg( i + 1 );
                  return false;
               }
               atoms.push_back( qsl_line[ 0 ] );
               qsl_line.pop_front();
            }
            pointmap_atoms[ res ].push_back( atoms );

            if ( !qsl_line.size() )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" )
                  .arg( i + 1 );
               return false;
            }
            pointmap_atoms_ref_residue[ res ].push_back( qsl_line[ 0 ] );
            qsl_line.pop_front();

            vector < QString > atoms_dest;
            for ( unsigned int j = 0; j < points; j++ )
            {
               if ( !qsl_line.size() )
               {
                  errormsg = 
                     QString( "Error in hydrated rotamer file line %1.  Invalid number of tokens on pointmap line" )
                     .arg( i + 1 );
                  return false;
               }
               atoms_dest.push_back( qsl_line[ 0 ] );
               qsl_line.pop_front();
            }
            pointmap_atoms_dest[ res ].push_back( atoms_dest );
         }
         continue;
      }  

      if ( qsl_line[ 0 ] == "dihedral:" )
      {
         qsl_line.pop_front();
         QString res = qsl_line[ 0 ];
         qsl_line.pop_front();
         vector < QString > four_atoms;
         for ( unsigned int j = 0; j < (unsigned int) qsl_line.size(); j++ )
         {
            four_atoms.push_back( qsl_line[ j ] );
            if ( four_atoms.size() == 4 )
            {
               dihedral_atoms[ res ].push_back( four_atoms );
               four_atoms.clear( );
            }
         }
         if ( four_atoms.size() )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2 on dihedral line" )
               .arg( i + 1 )
               .arg( qsl_line.size() + 2 );
            return false;
         }
         continue;
      }  
               
      if ( !in_rotamer )
      {
         if ( qsl_line.size() != 2 )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  Invalid number of tokens %2" )
               .arg( i + 1 )
               .arg( qsl_line.size() );
            return false;
         }
         
         if ( qsl_line[ 0 ] != "start-rotamer:" )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  Unrecognized token %2" )
               .arg( i + 1 )
               .arg( qsl_line[ 0 ] );
            return false;
         }
         tmp_rotamer.name = qsl_line[ 1 ];
         tmp_rotamer.residue = tmp_rotamer.name.left( 3 );
         tmp_rotamer.extension = tmp_rotamer.name.right( 7 );
         tmp_rotamer.side_chain.clear( );
         tmp_rotamer.waters.clear( );
         tmp_rotamer.atom_map.clear( );
         in_rotamer = true;
         in_rotamer_waters = false;
         continue;
      }
      if ( qsl_line.size() == 1 )
      {
         if ( qsl_line[ 0 ] == "end-rotamer" )
         {
            // push back rotamer info here
            if ( !tmp_rotamer.side_chain.size() ||
                 !tmp_rotamer.waters.size() )
            {
               errormsg = 
                  QString( "Error in hydrated rotamer file line %1.  Empty rotamer" )
                  .arg( i + 1 );
               return false;
            }
            rotamers[ tmp_rotamer.residue ].push_back( tmp_rotamer );
            in_rotamer = false;
            in_rotamer_waters = false;
            continue;
         }
         if ( qsl_line[ 0 ] != "TER" )
         {
            errormsg = 
               QString( "Error in hydrated rotamer file line %1.  Unrecognized token %2 in this context" )
               .arg( i + 1 )
               .arg( qsl_line[ 0 ] );
            return false;
         }
         in_rotamer_waters = true;
         continue;
      }
      if ( qsl_line.size() != 12 )
      {
         errormsg = 
            QString( "Error in hydrated rotamer file line %1:\n"
                          "%2\n"
                          "Unexpected number of tokens %3\n" )
            .arg( i + 1 )
            .arg( qsl[ i ] )
            .arg( qsl_line.size() );
         return false;
      }
      if ( qsl_line[ 0 ] != "ATOM" )
      {
         errormsg = 
            QString( "Error in hydrated rotamer file line %1.  Unrecognized token %2" )
            .arg( i + 1 )
            .arg( qsl_line[ 0 ] );
         return false;
      }

      if ( qsl[ i ].at( 13 ) == 'H' )
      {
         // skip hydrogens for now
         continue;
      }
      
      if ( !in_rotamer_waters &&
           rx_main_chain.indexIn( qsl_line[ 2 ] ) != -1 )
      {
         // skip main chain 
         continue;
      }
         
      if ( qsl_line[ 3 ] == "GLY" )
      {
         // skip GLY to support old romater file format 
         continue;
      }
      rotamer_atom ra;
      ra.name = qsl_line[ 2 ];
      ra.coordinate.axis[ 0 ] = qsl_line[ 6  ].toFloat();
      ra.coordinate.axis[ 1 ] = qsl_line[ 7  ].toFloat();
      ra.coordinate.axis[ 2 ] = qsl_line[ 8  ].toFloat();
      // ra.occupancy            = qsl_line[ 9  ].toFloat();
      // ra.tempFactor           = qsl_line[ 10 ];
            
      if ( in_rotamer_waters )
      {
         tmp_rotamer.waters.push_back( ra );
      } else {
         tmp_rotamer.atom_map[ ra.name ] = ra;
         tmp_rotamer.side_chain.push_back( ra );
      }
   }

   if ( in_rotamer )
   {
      errormsg = 
         "Error in hydrated rotamer file at end, no final end-rotamer.";
      return false;
   }

   if ( !validate_pointmap() )
   {
      return false;
   }

   cout << "Done reading hydrated rotamer file\n";
   if ( !compute_rotamer_dihedrals() )
   {
      return false;
   }
   // cout << list_rotamer_dihedrals();
   if ( !compute_water_positioning_atoms() )
   {
      return false;
   }
   // cout << list_water_positioning_atoms();

   // compute totals
   hydrate_count.clear( );
   for ( map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms.begin();
         it != pointmap_atoms.end();
         it++ )
   {
      if ( hydrate_count.count( it->first ) )
      {
         hydrate_count[ it->first ] += ( unsigned int ) it->second.size();
      } else {
         hydrate_count[ it->first ] =  ( unsigned int ) it->second.size();
      }
   }

   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size() && i < 1; i++ )
      {
         if ( hydrate_count.count( it->first ) )
         {
            hydrate_count[ it->first ] += ( unsigned int ) it->second[ i ].waters.size();
         } else {
            hydrate_count[ it->first ] =  ( unsigned int ) it->second[ i ].waters.size();
         }
      }
   }

   //    for ( map < QString, unsigned int >::iterator it = hydrate_count.begin();
   //          it != hydrate_count.end();
   //          it++ )
   //    {
   //       cout << QString( "res %1 hydrate count %2\n" ).arg( it->first ).arg( it->second );
   //    }

   return true;
}

bool US_Saxs_Util::compute_rotamer_dihedrals()
{
   cout << "Calculating dihedrals of rotamers\n";

   // a dihedral computation takes 4 points
   vector < point > p(4);
   float dihedral;

   // for each residue
   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      // for each rotamer for the residue
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
#if defined( DEBUG_DIHEDRAL )
         cout << 
            QString("computation for dihedral %1 %2:\n")
            .arg( it->first )
            .arg( it->second[ i ].extension );
#endif
         it->second[ i ].dihedral_angles.clear( );
         // process each dihedral chain, giving 4 points
         for ( unsigned int j = 0; j < dihedral_atoms[ it->first ].size(); j++ )
         {
#if defined( DEBUG_DIHEDRAL )
            cout << QString("  dihedral chain pos %1:\n").arg( j );
#endif
            if ( dihedral_atoms[ it->first ][ j ].size() != 4 )
            {
               errormsg = 
                  QString( "Dihedral table size incorrect, should be 4 but is %1 for %2th dihedral group of %3" )
                  .arg( dihedral_atoms[ it->first ][ j ].size() )
                  .arg( j + 1 )
                  .arg( it->first );
               return false;
            }
            for ( unsigned int k = 0; k < dihedral_atoms[ it->first ][ j ].size(); k++ )
            {
#if defined( DEBUG_DIHEDRAL )
               cout << QString("  %1: ").arg( dihedral_atoms[ it->first ][ j ][ k ] );
#endif
               if ( !( it->second[ i ].atom_map.count( dihedral_atoms[ it->first ][ j ][ k ] ) ) )
               {
                  errormsg = 
                     QString( "Rotamer for %1 is missing dihedral atom %2" )
                     .arg( it->first )
                     .arg( dihedral_atoms[ it->first ][ j ][ k ] );
                  return false;
               }
               p[ k ] = it->second[ i ].atom_map[ dihedral_atoms[ it->first ][ j ][ k ] ].coordinate;
#if defined( DEBUG_DIHEDRAL )
               cout << p[ k ];
#endif
            }
#if defined( DEBUG_DIHEDRAL )
            cout << "\n"
                    "   plane1:  " << plane( p[ 0 ], p[ 1 ], p[ 2 ] ) << endl;
            cout << "   minus32: " << minus( p[ 2 ], p [ 1 ] ) << endl;
            cout << "   minus12: " << minus( p[ 0 ], p [ 1 ] ) << endl;
            cout << "   cross:   " << cross( minus( p[ 2 ], p [ 1 ] ), minus( p[ 0 ], p [ 1 ] ) ) << endl;
            cout << "   plane2: " << plane( p[ 1 ], p[ 2 ], p[ 3 ] ) << endl;
            cout << "   dot:    " << dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                          plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) << endl;
            cout << "   acos:   " << acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                                plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) ) << endl;
#endif

            dihedral = acos( dot( plane( p[ 0 ], p[ 1 ], p[ 2 ] ),
                                  plane( p[ 1 ], p[ 2 ], p[ 3 ] ) ) );
            it->second[ i ].dihedral_angles.push_back( dihedral );
         }
      }
   }
   cout << "Done calculating dihedrals of rotamers\n";
   return true;
}

QString US_Saxs_Util::list_rotamer_dihedrals()
{
   QString out;

   out = "rotamer dihedrals:\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "Rotamers for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].dihedral_angles.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].dihedral_angles[ j ] );
         }
         out += "\n";
      }
      out += "\n";
   }
   return out;
}

QString US_Saxs_Util::list_pointmaps()
{
   QString out;

   out = "pointmaps:\n";

   for (  map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms.begin();
          it != pointmap_atoms.end();
          it++ )
   {
      out += 
         QString( "Pointmaps for residue: %1  number of maps %2\n" )
         .arg( it->first )
         .arg( it->second.size() );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += 
            QString( "    mapping to %1\n" )
            .arg( pointmap_atoms_ref_residue.count( it->first ) 
                  ?
                  ( pointmap_atoms_ref_residue[ it->first ].size() > i 
                    ?
                    pointmap_atoms_ref_residue[ it->first ][ i ]
                    :
                    "ERROR missing reference residue map" )
                  :
                  "ERROR missing reference residue" );

         if ( pointmap_atoms_ref_residue.count( it->first ) &&
              pointmap_atoms_ref_residue[ it->first ].size() > i )
         {
            for ( unsigned int j = 0; j < it->second[ i ].size(); j++ )
            {
               out += QString("    %1 -> %2\n")
                  .arg( it->second[ i ][ j ] )
                  .arg( pointmap_atoms_dest.count( it->first )
                        ?
                        ( pointmap_atoms_dest[ it->first ].size() > i 
                          ?
                          ( pointmap_atoms_dest[ it->first ][ i ].size() > j 
                            ?
                            pointmap_atoms_dest[ it->first ][ i ][ j ] 
                            :
                            "ERROR missing destination residue map elememt" )
                          :
                          "ERROR missing destination residue map" )
                        :
                        "ERROR missing destination residue" );
            }
         }
      }
   }
   return out;
}

QString US_Saxs_Util::list_rotamers( bool coords )
{
   QString out;

   out = "dihedral atoms:\n";
   for (  map < QString, vector < vector < QString > > >::iterator it = dihedral_atoms.begin();
          it != dihedral_atoms.end();
          it++ )
   {
      out += QString(" %1:\n").arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < it->second[i].size(); j++ )
         {
            out += QString(" %1").arg( it->second[i][j] );
         }
         out += " ; ";
      }
      out += "\n";
   }
   out += "\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "Rotamers for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString(" %1: sc").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].side_chain.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].side_chain[ j ].name );
            if ( coords )
            {
               out += QString( " [%1,%2,%3]" )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 0 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 1 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 2 ] );
            }
         }
         out += " w";
         for ( unsigned int j = 0; j < it->second[ i ].waters.size(); j++ )
         {
            out += QString(" %1").arg( it->second[ i ].waters[ j ].name );
            if ( coords )
            {
               out += QString( " [%1,%2,%3]" )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 0 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 1 ] )
                  .arg( it->second[ i ].side_chain[ j ].coordinate.axis[ 2 ] );
            }
         }
         out += "\n";
      }
   }
   return out;
}

bool US_Saxs_Util::compute_best_fit_rotamer()
{
   best_fit_rotamer.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   // go through the computed dihedrals, compute sum of abs differences, choose best one
   for ( map < QString, vector < float > >::iterator it = to_hydrate_dihedrals.begin();
         it != to_hydrate_dihedrals.end();
         it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // check dihedrals for this residue
      if ( !rotamers.count( resName ) )
      {
         errormsg = QString( "No rotamer group found for residue %1." ).arg( resName );
         return false;
      }
      
      float        best_fitness     = 1e30f;
      unsigned int best_fitness_pos = 0;

      for ( unsigned int i = 0; i < rotamers[ resName ].size(); i++ )
      {
         if ( it->second.size() != rotamers[ resName ][ i ].dihedral_angles.size() )
         {
            errormsg = 
               QString( "Number of dihedrals do not match between side chain %1 (%2) and rotamer %3 (%4)" )
               .arg( it->first )
               .arg( it->second.size() )
               .arg( rotamers[ resName ][ i ].name )
               .arg( rotamers[ resName ][ i ].dihedral_angles.size() );
            return false;
         }
         float fitness = 0.0;
         for ( unsigned int j = 0; j < it->second.size(); j++ )
         {
            // the dihedral angles are computed by acos, and range of acos is [0,pi], 
            // so I think we are ok
            fitness += fabs( it->second[ j ] - rotamers[ resName ][ i ].dihedral_angles[ j ] );
         }
         if ( !i || best_fitness > fitness )
         {
            best_fitness = fitness;
            best_fitness_pos = i;
         }
      }
      
      best_fit_rotamer[ it->first ] = rotamers[ resName ][ best_fitness_pos ];
   }
   return true;
}

QString US_Saxs_Util::list_best_fit_rotamer()
{
   QString out;

   out = "Best fit rotamers:\n";

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
         it != best_fit_rotamer.end();
         it++ )
   {
      out += 
         QString( "%1 %2\n" )
         .arg( it->first )
         .arg( it->second.name );
   }
   return out;
}

bool US_Saxs_Util::setup_pointmap_rotamers()
{
   pointmap_rotamers.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for ( map < QString, map < QString, point > >::iterator it = to_hydrate_pointmaps.begin();
         it != to_hydrate_pointmaps.end();
         it++ )
   {
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      // for each entry in pointmap_atoms_ref_residue
      if ( !pointmap_atoms_ref_residue.count( resName ) )
      {
         errormsg = QString( "Internal error: could not find rotamer for pointmap key %1" ).arg( it->first );
         return false;
      }

      for ( unsigned int i = 0; i < pointmap_atoms_ref_residue[ resName ].size(); i++ )
      {
         if ( !rotamers.count( pointmap_atoms_ref_residue[ resName ][ i ] ) )
         {
            errormsg = QString( "No rotamer group found for pointmap key %1 reference residue %2." )
               .arg( it->first )
               .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
            return false;
         }
         // use first one
         pointmap_rotamers[ it->first ].push_back( rotamers[ pointmap_atoms_ref_residue[ resName ][ i ] ][ 0 ] );
      }
   }
   return true;
}

QString US_Saxs_Util::list_pointmap_rotamers()
{
   QString out;

   out = "Pointmap rotamers:\n";

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      out += QString( "%1:" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString( " %1" ).arg( it->second[ i ].name );
      }
      out += "\n";
   }
   return out;
}

bool US_Saxs_Util::compute_water_positioning_atoms()
{
   cout << "Calculating water positioning atoms for each rotamer\n";

   list < sortable_float > lsf;
   sortable_float          sf;
   vector < QString >      wpa;

   map < QString, rotamer * > back_ref;

   for ( map < QString, vector < rotamer > >::iterator it = rotamers.begin();
         it != rotamers.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         it->second[ i ].water_positioning_atoms.clear( );

         // for each water

         for ( unsigned int j = 0; j < it->second[ i ].waters.size(); j++ )
         {
            lsf.clear( );
            wpa.clear( );
            
            // find the distance to each side chain atom
            for ( unsigned int k = 0; k < it->second[ i ].side_chain.size(); k++ )
            {
               sf.f = dist( it->second[ i ].waters[ j ].coordinate, it->second[ i ].side_chain[ k ].coordinate );
               sf.index = k;
               lsf.push_back( sf );
            }

            lsf.sort();
            
            unsigned int pushed = 0;
            for ( list < sortable_float >::iterator it2 = lsf.begin();
                  it2 != lsf.end();
                  it2++ )
            {
               wpa.push_back( it->second[ i ].side_chain[ it2->index ].name );
               pushed++;
               if ( pushed >= MAX_WATER_POSITIONING_ATOMS )
               {
                  break;
               }
            }
            it->second[ i ].water_positioning_atoms.push_back( wpa );
         }
         back_ref[ it->second[ i ].name ] = &(it->second[ i ]);
      }
   }
   cout << "Done calculating water positioning atoms for each rotamer\n";
   cout << "Copy over water positioning info for rotated rotamers\n";

   for ( map < QString, vector < vector < rotamer > > >::iterator it = rotated_rotamers.begin();
         it != rotated_rotamers.end();
         it++ )
   {
      if ( !back_ref.count( it->first ) )
      {
         errormsg = QString( "Error: rotated_rotamer wth name %1 not found it rotamers" )
            .arg( it->first );
         return false;
      }

      unsigned int water_count = it->second.size();
      
      if ( water_count != back_ref[ it->first ]->water_positioning_atoms.size() )
      {
         errormsg = QString( "Error: rotated_rotamer wth name %1 found but water count differs %2 vs %3" )
            .arg( it->first )
            .arg( water_count )
            .arg( back_ref[ it->first ]->water_positioning_atoms.size() );
         return false;
      }

      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < it->second[ i ].size(); j++ )
         {
            it->second[ i ][ j ].water_positioning_atoms.push_back( back_ref[ it->first ]->water_positioning_atoms[ i ] );
         }
      }
   }
   return true;
}

QString US_Saxs_Util::list_water_positioning_atoms()
{
   QString out = "Water positioning atoms:\n";

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      out += QString( "  Rotamer for residue: %1\n" ).arg( it->first );
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString("  %1:\n").arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].water_positioning_atoms.size(); j++ )
         {
            out += QString( "    water %1 :" ).arg( j );
            for ( unsigned int k = 0; k < it->second[ i ].water_positioning_atoms[ j ].size(); k++ )
            {
               out += QString(" %1").arg( it->second[ i ].water_positioning_atoms[ j ][ k ] );
            }
            out += "\n";
         }
      }
      out += "\n";
   }

   // alternate format

   for (  map < QString, vector < rotamer > >::iterator it = rotamers.begin();
          it != rotamers.end();
          it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         out += QString("%1-%2 ").arg( it->first ).arg( it->second[ i ].extension );
         for ( unsigned int j = 0; j < it->second[ i ].water_positioning_atoms.size(); j++ )
         {
            out += QString( " [%1]" ).arg( j );
            for ( unsigned int k = 0; k < it->second[ i ].water_positioning_atoms[ j ].size(); k++ )
            {
               out += QString(" %1").arg( it->second[ i ].water_positioning_atoms[ j ][ k ] );
            }
         }
         out += "\n";
      }
      out += "\n";
   }

   return out;
}

bool US_Saxs_Util::compute_waters_to_add()
{
   puts("Transforming waters to add to pdb coordinates");

   steric_clash_summary.clear( );

   count_waters           = 0;
   count_waters_added     = 0;
   count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear( );
   waters_source.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   for ( map < QString, rotamer >::iterator it = best_fit_rotamer.begin();
         it != best_fit_rotamer.end();
         it++ )
   {
      if ( !to_hydrate.count( it->first ) )
      {
         errormsg = QString( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" )
            .arg( it->first );
         return false;
      }

      // for each water to add:
      // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );
      if ( it->second.water_positioning_atoms.size() !=
           it->second.waters.size() )
      {
         errormsg = QString( "Internal error: water positioning atom size %1 does not match waters size %2" ) 
            .arg( it->second.water_positioning_atoms.size() )
            .arg( it->second.waters.size() );
         return false;
      }

      for ( unsigned int i = 0; i < it->second.water_positioning_atoms.size(); i++ )
      {
         // get coordinates of 
         p1.resize( it->second.water_positioning_atoms[ i ].size() );
         p2.resize( it->second.water_positioning_atoms[ i ].size() );
            
         for ( unsigned int j = 0; j < it->second.water_positioning_atoms[ i ].size(); j++ )
         {
            if ( !it->second.atom_map.count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               errormsg = QString( "Internal error: water positioning atom %1 not found in atom_map" ) 
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p1[ j ] = it->second.atom_map[ it->second.water_positioning_atoms[ i ][ j ] ].coordinate;

            if ( !to_hydrate[ it->first ].count( it->second.water_positioning_atoms[ i ][ j ] ) )
            {
               errormsg = QString( "Internal error: water positioning atom %1 not found in to_hydrate atoms" )
                  .arg( it->second.water_positioning_atoms[ i ][ j ] );
               return false;
            }
            p2[ j ] = to_hydrate[ it->first ][ it->second.water_positioning_atoms[ i ][ j ] ];

            // cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
            // .arg( it->second.water_positioning_atoms[ i ][ j ] )
            // .arg( p1[ j ].axis[ 0 ] )
            // .arg( p1[ j ].axis[ 1 ] )
            // .arg( p1[ j ].axis[ 2 ] )
            // .arg( p2[ j ].axis[ 0 ] )
            // .arg( p2[ j ].axis[ 1 ] )
            // .arg( p2[ j ].axis[ 2 ] );
         }
         vector < point > rotamer_waters;
         rotamer_waters.push_back( it->second.waters[ i ].coordinate );
         vector < point > new_waters;
         // cout << QString( " and apply it to the point [%1,%2,%3]\n")
         // .arg( it->second.waters[ i ].coordinate.axis[ 0 ] )
         // .arg( it->second.waters[ i ].coordinate.axis[ 1 ] )
         // .arg( it->second.waters[ i ].coordinate.axis[ 2 ] );
         if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
         {
            return false;
         }
         count_waters++;
         if ( !has_steric_clash( new_waters[ 0 ] ) )
         {
            count_waters_added++;
            waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
            waters_source[ it->first ].push_back( QString( "Rtmr:%1" ).arg( it->second.name ) );
#if defined( USUH_DEBUG_ALIGN )
            cout << QString( "p0 adding to waters source %1 %2\n" ).arg( it->first ).arg( it->second.name );
#endif
         } else {
            count_waters_not_added++;
#if defined( USUH_DEBUG_ALIGN )
            cout << QString( "p0 skiping due to clash %1 %2\n" ).arg( it->first ).arg( it->second.name );
#endif
         }
      }
   }

   for ( map < QString, vector < rotamer > >::iterator it = pointmap_rotamers.begin();
         it != pointmap_rotamers.end();
         it++ )
   {
      if ( !to_hydrate_pointmaps.count( it->first ) )
      {
         errormsg = QString( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" )
            .arg( it->first );
         return false;
      }

      // add a waters for each pointmap for this residue
      if ( rx_expand_mapkey.indexIn( it->first ) == -1 )
      {
         errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( it->first );
         return false;
      }
      QString resName = rx_expand_mapkey.cap( 1 );
      if ( !pointmap_atoms.count( resName ) ||
           !pointmap_atoms_dest.count( resName ) ||
           !pointmap_atoms_ref_residue.count( resName ) )
      {
         errormsg = QString( "Internal error: could not find pointmap entries for residue of key %1" ).arg( it->first );
         return false;
      }
         
      if ( it->second.size() != pointmap_atoms[ resName ].size() ||
           it->second.size() != pointmap_atoms_dest[ resName ].size() ||
           it->second.size() != pointmap_atoms_ref_residue[ resName ].size() )
      {
         errormsg = QString( "Internal error: could not find pointmap size inconsistancy for key %1" ).arg( it->first );
         return false;
      }
         
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         // for each water to add:
         // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
         // to the pointmap_atoms in the residue and apply to each water in the rotamer
         // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info

         if ( it->second[ i ].residue != pointmap_atoms_ref_residue[ resName ][ i ] )
         {
            errormsg = QString( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" )
               .arg( it->first )
               .arg( i )
               .arg( it->second[ i ].residue )
               .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
            return false;
         }

         // build up the transformation

         p1.resize( pointmap_atoms     [ resName ][ i ].size() );
         p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
         
         if ( p1.size() != p2.size() )
         {
            errormsg = QString( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" )
               .arg( it->first )
               .arg( i )
               .arg( pointmap_atoms[ resName ][ i ].size() )
               .arg( pointmap_atoms_dest[ resName ][ i ].size() );
            return false;
         }

         for ( unsigned int j = 0; j < p1.size(); j++ )
         {
            //             cout << QString("mapping: %1 %2 dest %3\n")
            //                .arg(it->first)
            //                .arg(i)
            //                .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
            if ( !it->second[ i ].atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
            {
               errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                  .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
               return false;
            }
               
            p1[ j ] = it->second[ i ].atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;

            if ( !to_hydrate_pointmaps[ it->first ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
            {
               errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                  .arg(  pointmap_atoms[ resName ][ i ][ j ] );
               return false;
            }
#if defined( USUH_DEBUG_ALIGN )
            cout << QString("z1 mapping: %1 %2 source %3\n")
               .arg(it->first)
               .arg(i)
               .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif

            p2[ j ] = to_hydrate_pointmaps[ it->first ][ pointmap_atoms[ resName ][ i ][ j ] ];

//             cout << QString( "z1  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
//                .arg( it->second.water_positioning_atoms[ i ][ j ] )
//                .arg( p1[ j ].axis[ 0 ] )
//                .arg( p1[ j ].axis[ 1 ] )
//                .arg( p1[ j ].axis[ 2 ] )
//                .arg( p2[ j ].axis[ 0 ] )
//                .arg( p2[ j ].axis[ 1 ] )
//                .arg( p2[ j ].axis[ 2 ] );
         }

         // now we have p1 & p2, apply to all of the waters
         //          cout << QString("adding %1 waters for %2 %3\n")
         //             .arg( it->second[ i ].waters.size() )
         //             .arg( it->first )
         //             .arg( resName );

         for ( unsigned int j = 0; j < it->second[ i ].waters.size(); j++ )
         {
            vector < point > rotamer_waters;
            rotamer_waters.push_back( it->second[ i ].waters[ j ].coordinate );
            vector < point > new_waters;
#if defined( USUH_DEBUG_ALIGN )
            cout << QString( "aligning: " );
            for ( unsigned int k = 0; k < p1.size(); k++ )
            {
               cout << p1[k] << p2[k] << " ";
            }
            cout << endl;
            cout << QString( "aligning waters: " );
            for ( unsigned int k = 0; k < rotamer_waters.size(); k++ )
            {
               cout << rotamer_waters[k];
            }
            cout << endl;
#endif
            if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
            {
               return false;
            }
#if defined( USUH_MAKE_TEST_CODE )
            cout << QString( "aligning after xfrom waters: " );
            for ( unsigned int k = 0; k < new_waters.size(); k++ )
            {
               cout << new_waters[k];
            }
            cout << endl << flush;
            cout << QString( "th{\n"
                             "th   vector < point > p1( %1 );\n"
                             "th   vector < point > p2( %2 );\n"
                             "th   vector < point > rotamer_waters( %3 );\n"
                             "th   vector < point > new_waters( %4 );\n"
                             "th   vector < point > cmp_waters( %5 );\n"
                             )
               .arg( p1.size() )
               .arg( p2.size() )
               .arg( rotamer_waters.size() )
               .arg( new_waters.size() )
               .arg( new_waters.size() )
               ;
            for ( unsigned int k = 0; k < p1.size(); k++ )
            {
               for ( unsigned int l = 0; l < 3; l++ )
               {
                  cout << QString( "" ).sprintf( "th   p1[ %u ].axis[ %u ] = %.10ff;\n",
                                                 k, l, p1[ k ].axis[ l ] );
               }
            }
            for ( unsigned int k = 0; k < p2.size(); k++ )
            {
               for ( unsigned int l = 0; l < 3; l++ )
               {
                  cout << QString( "" ).sprintf( "th   p2[ %u ].axis[ %u ] = %.10ff;\n",
                                                 k, l, p2[ k ].axis[ l ] );
               }
            }
            for ( unsigned int k = 0; k < rotamer_waters.size(); k++ )
            {
               for ( unsigned int l = 0; l < 3; l++ )
               {
                  cout << QString( "" ).sprintf( "th   rotamer_waters[ %u ].axis[ %u ] = %.10ff;\n",
                                                 k, l, rotamer_waters[ k ].axis[ l ] );
               }
            }
            for ( unsigned int k = 0; k < new_waters.size(); k++ )
            {
               for ( unsigned int l = 0; l < 3; l++ )
               {
                  cout << QString( "" ).sprintf( "th   new_waters[ %u ].axis[ %u ] = %.10ff;\n",
                                                 k, l, new_waters[ k ].axis[ l ] );
               }
            }
            cout << QString( "th   if ( !atom_align( p1, p2, rotamer_waters, cmp_waters ) )\n"
                             "th   {\n"
                             "th      return false;\n"
                             "th   }\n"
                             "th   for ( unsigned int i = 0; i < cmp_waters.size(); i++ )\n"
                             "th   {\n"
                             "th      for ( unsigned int j = 0; j < 3; j++ )\n"
                             "th      {\n"
                             "th         if ( fabs( cmp_waters[ i ].axis[ j ] - new_waters[ i ].axis[ j ] ) > 1e-4 )\n"
                             "th         {\n"
                             "th            errormsg = QString( \"mismatch water %u axis %u\\n\" ).arg( i ).arg( j );\n"
                             "th            return false;\n"
                             "th         }\n"
                             "th      }\n"
                             "th    }\n"
                             "th }\n    // ok\n" );
#endif

            count_waters++;
            if ( !has_steric_clash( new_waters[ 0 ] ) )
            {
               count_waters_added++;
               waters_to_add[ it->first ].push_back( new_waters[ 0 ] );
               waters_source[ it->first ].push_back( QString( "PM:%1" ).arg( it->second[ i ].name ) );
               // cout << QString( "p1 adding to waters source %1 %2\n" ).arg( it->first ).arg( it->second[ i ].name );
            } else {
               count_waters_not_added++;
#if defined( USUH_DEBUG_ALIGN )
               cout << QString( "p1 skipping due to clash %1 %2\n" ).arg( it->first ).arg( it->second[ i ].name ) << flush;
               {
                  point p = new_waters[ 0 ];
                  unsigned int i = current_model;
                  double dist_threshold = 1e0 - ( our_saxs_options.steric_clash_distance / 100e0 );
                  double water_radius   = multi_residue_map.count( "WAT" ) ?
                     residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

                  // cout << QString( "using %1 for water radius %2\n" ).arg( water_radius ).arg(  multi_residue_map.count( "WAT" ) ? "has WAT" : "no WAT" );

                  // check structure:
                  for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
                     for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
                        PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         
                        if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
                        {
                           cout << QString( "clash with structure %1 %2 %3 %4" ).arg( dist( this_atom->coordinate, p ) - ( this_atom->radius + water_radius ) * dist_threshold )
                              .arg( this_atom->name ).arg( this_atom->resName ).arg( this_atom->serial ) << this_atom->coordinate << p << endl << flush;
                        }
                     }
                  }

                  // check already added waters:
                  for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
                        it != waters_to_add.end();
                        it++ )
                  {
                     for ( unsigned int i = 0; i < it->second.size(); i++ )
                     {
                        if ( dist( it->second[ i ], p ) <=  2e0 * water_radius * dist_threshold )
                        {
                           cout << QString( "clash with water %1\n" ).arg( dist( it->second[ i ], p ) -  2e0 * water_radius * dist_threshold ) << flush;
                           cout << it->second[ i ] << p << endl << flush;
                        }
                     }
                  }
               }
#endif
            }
         }
      }
   }
   
   // list_steric_clash();
   if ( !list_steric_clash_recheck() )
   {
      return false;
   }

   cout << "Done transforming waters to add to pdb coordinates\n";
   cout << QString( "%1 waters added. %2 not added due to steric clashes \n")
      .arg( count_waters_added ) 
      .arg( count_waters_not_added ) ;

   return true;
}


float US_Saxs_Util::min_dist_to_waters( point p )
{
   // check already added waters:
   float min_dist = 1.0e15;
   bool first = true;

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         float this_dist = dist( it->second[ i ], p );
         if ( first )
         {
            min_dist = this_dist;
            first = false;
         } else {
            if ( min_dist > this_dist )
            {
               min_dist = this_dist;
            }
         }
      }
   }
   return min_dist;
}

bool US_Saxs_Util::flush_pdb()
{
   cout << "flush_pdb\n";
   QString fname = control_parameters[ "inputfile" ];

   fname = fname.replace( QRegExp( "^../common/" ), "" );

   fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" ) +
      QString( "-c%1-h%2.pdb" )
      .arg( QString( "%1" ).arg( our_saxs_options.steric_clash_distance ).replace( ".", "_" ) )
      .arg( our_saxs_options.alt_hydration ? "a" : "" )
      ;
   QFile f( fname );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errormsg = QString( "Error: can not open file %1 for writing" ).arg( fname );
      return false;
   }

   cout << "flush_pdb name:" << fname << "\n";
   output_files << fname;

   last_hydrated_pdb_header =
      QString( "HEADER  US-SOMO Hydrated pdb file %1\n" ).arg( fname );

   QTextStream ts( &f );
   ts << last_hydrated_pdb_header;
   ts << last_hydrated_pdb_text;
   ts << "END\n";
   f.close();

   last_hydrated_pdb_name = fname;
   last_hydrated_pdb_text   = "";

   return true;
}

bool US_Saxs_Util::buffer_pdb_with_waters()
{
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "No input file found";
      return false;
   }

   last_hydrated_pdb_text +=
      QString( "MODEL    %1\n" ).arg( current_model + 1 );

   last_hydrated_pdb_text += last_pdb_load_calc_mw_msg.replaceInStrings( "\n", "" ).join( "\nREMARK " ) + "\n";
   for ( int i = 0; i < ( int ) last_steric_clash_log.size(); i++ )
   {
      last_hydrated_pdb_text += last_steric_clash_log[ i ];
   }
   last_hydrated_pdb_text += "\n";

   unsigned int i = current_model;

   unsigned int atom_number = 0;
   unsigned int residue_number = 0;
   map < QString, bool > chains_used;

   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

         last_hydrated_pdb_text +=
            QString("")
            .sprintf(     
                     "ATOM  %5d%5s%4s %1s%4d    %8.3f%8.3f%8.3f%6.2f%6.2f          %2s\n",
                     this_atom->serial,
                     this_atom->orgName.toLatin1().data(),
                     this_atom->resName.toLatin1().data(),
                     this_atom->chainID.toLatin1().data(),
                     this_atom->resSeq.toUInt(),
                     this_atom->coordinate.axis[ 0 ],
                     this_atom->coordinate.axis[ 1 ],
                     this_atom->coordinate.axis[ 2 ],
                     this_atom->occupancy,
                     this_atom->tempFactor,
                     this_atom->element.toLatin1().data()
                     );
         chains_used[ this_atom->chainID ] = true;

         if ( atom_number < this_atom->serial )
         {
            atom_number = this_atom->serial;
         }
         if ( residue_number < this_atom->resSeq.toUInt() )
         {
            residue_number = this_atom->resSeq.toUInt();
         }
      }
   }
   last_hydrated_pdb_text +=
      "TER\n";

   QString chainID = "W";
   if ( chains_used.count( chainID ) )
   {
      for ( int i = 9; i >= 0; i-- )
      {
         chainID = QString( "%1" ).arg( i );
         if ( !chains_used.count( chainID ) )
         {
            break;
         }
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "Z";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "Y";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "X";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "w";
      }
      if ( chains_used.count( chainID ) )
      {
         chainID = "W";
      }
   }
      
   // add waters:

   residue_number = 0;
   atom_number    = 0;

   // check already added waters:
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         last_hydrated_pdb_text +=
            QString("")
            .sprintf(     
                     "ATOM  %5d  OW  WAT %1s%4d    %8.3f%8.3f%8.3f  1.00  0.00           O  \n",
                     ++atom_number,
                     chainID.toLatin1().data(),
                     ++residue_number,
                     it->second[ i ].axis[ 0 ],
                     it->second[ i ].axis[ 1 ],
                     it->second[ i ].axis[ 2 ]
                     );

      }
   }

   last_hydrated_pdb_text +=
      "TER\nENDMDL\n";

   return true;
}

bool US_Saxs_Util::selected_models_contain( QString residue )
{
   for ( unsigned int i = 0; i < model_vector.size(); i++) 
   {
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
      {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
         {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            if ( this_atom->resName == residue )
            {
               return true;
            }
         }
      }
   }
   return false;
}

bool US_Saxs_Util::validate_pointmap()
{
   // make sure all ref_residue's have a rotamer

   errormsg = "";

   map < QString, bool > to_erase;

   for ( map < QString, vector < QString > >::iterator it = pointmap_atoms_ref_residue.begin();
         it != pointmap_atoms_ref_residue.end();
         it++ )
   {
      if ( !to_erase.count( it->first ) )
      {
         if ( !pointmap_atoms     .count( it->first ) ||
              !pointmap_atoms_dest.count( it->first ) ||
              pointmap_atoms[ it->first ]     .size() != it->second.size() ||
              pointmap_atoms_dest[ it->first ].size() != it->second.size() )
         {
            errormsg += QString( "Internal error: inconsistancy with Pointmap %1\n" )
               .arg( it->first );
            to_erase[ it->first ] = true;
         } else {
            for ( unsigned int i = 0; i < it->second.size(); i++ )
            {
               if ( !rotamers.count( it->second[ i ] ) )
               {
                  errormsg += QString( "Pointmap %1 contains invalid target rotamer %2, removing from loaded pointmaps\n" )
                     .arg( it->first )
                     .arg( it->second[ i ] );
                  to_erase[ it->first ] = true;
               } else {
                  if ( !rotamers[ it->second[ i ] ].size() )
                  {
                     errormsg += QString( "Pointmap %1 contains empty target rotamer %2, removing from loaded pointmaps\n" )
                        .arg( it->first )
                        .arg( it->second[ i ] );
                     to_erase[ it->first ] = true;
                  }
               }
            }
         }
      }
   }

   for ( map < QString, vector < vector < QString > > >::iterator it = pointmap_atoms_dest.begin();
         it != pointmap_atoms_dest.end();
         it++ )
   {
      if ( !to_erase.count( it->first ) )
      {
         for ( unsigned int i = 0; i < it->second.size(); i++ )
         {
            for ( unsigned int j = 0; j < it->second[ i ].size(); j++ )
            {
               if ( !rotamers[ pointmap_atoms_ref_residue[ it->first ][ i ] ][ 0 ].atom_map.count( it->second[ i ][ j ] ) )
               {
                  errormsg += QString( "Pointmap %1 target rotamer %2 has an invalid atom %3 (not found in rotamer), removing from loaded pointmaps\n" )
                     .arg( it->first )
                     .arg( pointmap_atoms_ref_residue[ it->first ][ i ] )
                     .arg( it->second[ i ][ j ] );
                  to_erase[ it->first ] = true;
               }
            }
         }
      }
   }
   
   for ( map < QString, bool >::iterator it = to_erase.begin();
         it != to_erase.end();
         it++ )
   {
      pointmap_atoms            .erase( it->first );
      pointmap_atoms_dest       .erase( it->first );
      pointmap_atoms_ref_residue.erase( it->first );
   }

   if ( !errormsg.isEmpty() )
   {
      return false;
   }
   return true;
}

#endif // CMDLINE

bool US_Saxs_Util::pdb_asa_for_saxs_hydrate()
{
   if ( !control_parameters.count( "inputfile" ) )
   {
      errormsg = "No input file found";
      return false;
   }

   cout << QString( "Hydrating the pdb for %1 model %2\n" )
      .arg( control_parameters[ "inputfile "] )
      .arg( current_model + 1 );
   cout << "Checking the pdb structure\n";

   if ( !check_for_missing_atoms( &model_vector[current_model] ) ) 
   {
      errormsg = "Encountered errors with your PDB structure" + errormsg;
      return false;
   }

   reset_chain_residues( &model_vector[ current_model ] );

   // probably don't need this:
   hydro_results results;
   results.asa_rg_pos = 0.0;
   results.asa_rg_neg = 0.0;
   cout << "PDB structure ok\n";

   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[ current_model ].molecule.size();
      int i;
      for ( i = 0; i < no_of_molecules; i++ ) 
      {
         no_of_atoms += model_vector[ current_model ].molecule[ i ].atom.size();
      }

      cout << QString( "There are %1 atoms in %2 chain(s) in this model\n" )
         .arg( no_of_atoms ).arg( no_of_molecules );
   }

   if ( !create_beads() )
   {
      return false;
   }

   if( errormsg.length() ) 
   {
      errormsg = "Encountered unknown atom(s) error:\n" + errormsg;
      return false;
   }

   if( control_parameters[ "asamethod" ].toUInt() == 0 ) 
   {
      // surfracer
      errormsg = "Vornoi tesselation not currently implemented";
      return false;
#if defined( NOT_IMPLEMENTED )
      editor->append("Computing ASA via SurfRacer\n");
      int retval = surfracer_main( asa.hydrate_probe_radius,
                                   active_atoms,
                                   false,
                                   progress,
                                   editor
                                   );

      editor->append("Return from Computing ASA\n");
      if ( retval )
      {
         editor->append("Errors found during ASA calculation\n");
         switch ( retval )
         {
         case US_SURFRACER_ERR_MISSING_RESIDUE:
            {
               errormsg = "US_SURFRACER encountered an unknown residue:" + errormsg;
               return false;
               break;
            }
         case US_SURFRACER_ERR_MISSING_ATOM:
            {
               errormsg = "US_SURFRACER encountered a unknown atom:\n" + errormsg;
               return false;
               break;
            }
         case US_SURFRACER_ERR_MEMORY_ALLOC:
            {
               errormsg = "US_SURFRACER encountered a memory allocation error";
               return false;
               break;
            }
         default:
            {
               errormsg = "US_SURFRACER encountered an unknown error";
               // unknown error
               return false;
               break;
            }
         }
      }
      if(errormsg.length()) {
         printError("US_SURFRACER encountered unknown atom(s) error:\n" +
                    errormsg);
         return US_SURFRACER_ERR_MISSING_ATOM;
      }
#endif
   }

   if( control_parameters[ "asamethod" ].toUInt() == 1 ) 
   {
      cout << "Computing ASA via ASAB1\n";

      float save_radius = asa.probe_radius;
      asa.probe_radius  = asa.hydrate_probe_radius; 
      cout << QString(
                      "asa.hydrate_probe_radius %1\n"
                      "asa.probe_radius         %2\n"
                      "asa.asab1_step           %3\n"
                      "asa.threshold_percent    %4\n"
                      "misc.pb_rule_on          %5\n"
                      )
         .arg( asa.hydrate_probe_radius )
         .arg( asa.probe_radius )
         .arg( asa.asab1_step )
         .arg( asa.threshold_percent )
         .arg( misc_pb_rule_on ? "yes" : "no" )
         ;
      int retval = us_saxs_util_asab1_main( active_atoms,
                                            &asa,
                                            &results,
                                            false );
      asa.probe_radius  = save_radius;

      cout << "Return from Computing ASA\n";
      if ( retval )
      {
         cout << "Errors found during ASA calculation\n";
         switch ( retval )
         {
         case US_SAXS_UTIL_ASAB1_ERR_MEMORY_ALLOC:
            {
               errormsg = "US_HYDRODYN_ASAB1 encountered a memory allocation error";
               return false;
               break;
            }
         default:
            {
               errormsg = "US_HYDRODYN_ASAB1 encountered an unknown error";
               return false;
               break;
            }
         }
      }
   }

   // pass 1 assign bead #'s, chain #'s, initialize data

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p1 i j k %d %d %d %lx %s\n", i, j, k, (long unsigned int)this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

            this_atom->normalized_ot_is_valid = false;
            for (unsigned int m = 0; m < 3; m++) {
               this_atom->bead_cog_coordinate.axis[m] = 0;
               this_atom->bead_position_coordinate.axis[m] = 0;
               this_atom->bead_coordinate.axis[m] = 0;
            }
         }
      }
   }


   // #define DEBUG
   // pass 2 determine beads, cog_position, fixed_position, molecular cog phase 1.

   int count_actives;
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;
   QString cog_msg = "COG calc summary information\n";

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         int last_bead_assignment = -1;
         int last_chain = -1;
         QString last_resName = "not a residue";
         QString last_resSeq = "";
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         PDB_atom *last_main_bead = (PDB_atom *) 0;
         PDB_atom *sidechain_N = (PDB_atom *) 0;

         count_actives = 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p2 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            // this_atom->bead_positioner = false;
            if (this_atom->active) {
#if defined( USUH_DEBUG_ASA )
               cout << QString( "active atom asa %1 %2 %3 %4\n" )
                  .arg( this_atom->name )
                  .arg( this_atom->resName )
                  .arg( this_atom->serial )
                  .arg( this_atom->asa )
                  ;
#endif
               molecular_mw += this_atom->mw;
               for (unsigned int m = 0; m < 3; m++) {
                  molecular_cog[m] += this_atom->coordinate.axis[m] * this_atom->mw;
               }

               this_atom->bead_mw = 0;
               this_atom->bead_asa = 0;
               this_atom->bead_recheck_asa = 0;

               // do we have a new bead?
               // we want to put the N on a previous bead unless it is the first one of the molecule
               // ONLY FOR residue type = 0! (amino acid)
               // AND ONLY for residues not part of the 'auto bead builder'

               if (!create_beads_normally ||
                   (
                    (this_atom->bead_assignment != last_bead_assignment ||
                     this_atom->chain != last_chain ||
                     this_atom->resName != last_resName ||
                     this_atom->resSeq != last_resSeq) &&
                    !(misc_pb_rule_on &&
                      this_atom->chain == 0 &&
                      this_atom->name == "N" &&
                      count_actives &&
                      !broken_chain_head.count(QString("%1|%2")
                                               .arg(this_atom->resSeq)
                                               .arg(this_atom->resName))
                      ) ) ) {

                  // this_atom->bead_positioner = true;
                  this_atom->is_bead = true;
                  last_main_bead = this_atom;
                  last_bead_assignment = this_atom->bead_assignment;
                  last_chain = this_atom->chain;
                  last_resName = this_atom->resName;
                  last_resSeq = this_atom->resSeq;
                  if (create_beads_normally && 
                      misc_pb_rule_on &&
                      !this_atom->type) {
                     if(sidechain_N &&
                        this_atom->chain == 1) {
                        if(this_atom->name == "N") {
                           printf("ERROR double N on sequential sidechains! PRO PRO?\n");
                        }
                        this_atom->bead_asa += sidechain_N->bead_asa;
                        this_atom->bead_mw += sidechain_N->bead_mw;
                        sidechain_N->bead_mw = 0;
                        sidechain_N->bead_asa = 0;
                        sidechain_N = (PDB_atom *) 0;
                     }
                     if(this_atom->name == "N" &&
                        this_atom->chain == 1) {
                        sidechain_N = this_atom;
                        this_atom->is_bead = false;
                     }
                  }
               }
               else 
               {
                  if (this_atom->bead_positioner) {

                     if (last_main_bead->bead_positioner &&
                         this_atom->placing_method == 1) {
                        fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n",
                                last_main_bead->name.toLatin1().data(),
                                last_main_bead->resName.toLatin1().data(),
                                last_main_bead->serial);
                     }
                     last_main_bead->bead_positioner = true;
                     last_main_bead->bead_position_coordinate = this_atom->coordinate;
                  }

                  this_atom->is_bead = false;
                  // this_atom->bead_cog_mw = 0;
               }

               this_atom->bead_cog_mw = 0;

               // special nitrogen asa handling
               PDB_atom *use_atom;
               if (misc_pb_rule_on &&
                   create_beads_normally &&
                   this_atom->chain == 0 &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                           .arg(this_atom->resSeq)
                                           .arg(this_atom->resName)) &&
                   last_main_chain_bead) {
                  use_atom = last_main_chain_bead;
               }
               else 
               {
                  use_atom = last_main_bead;
               }

               use_atom->bead_asa += this_atom->asa;
               use_atom->bead_mw += this_atom->mw;
               // accum
               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  cog_msg += QString("adding %1 to %2\n").arg(this_atom->serial).arg(use_atom->serial);
                  use_atom->bead_cog_mw += this_atom->mw;
                  for (unsigned int m = 0; m < 3; m++) {
                     use_atom->bead_cog_coordinate.axis[m] +=
                        this_atom->coordinate.axis[m] * this_atom->mw;
                  }
               }
               else 
               {
               }

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if (use_atom->bead_positioner &&
                      this_atom->placing_method == 1) {
                     fprintf(stderr, "warning: 2 or more positioners in bead %s %s %d\n",
                             use_atom->name.toLatin1().data(),
                             use_atom->resName.toLatin1().data(),
                             use_atom->serial);
                  }
                  use_atom->bead_positioner = true;
                  use_atom->bead_position_coordinate = this_atom->coordinate;
               }

               if (this_atom->chain == 0 &&
                   misc_pb_rule_on &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) && 
                   !count_actives)
               {
                  last_resName = "first N";
               }

               if (this_atom->chain == 0 &&
                   this_atom->name == "CA") {
                  last_main_chain_bead = this_atom;
               }
               count_actives++;
            }
            else 
            {
               this_atom->is_bead = false;
            }
         }
      }
   }

   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
   }
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

   for (unsigned int m = 0; m < 3; m++) {
      last_molecular_cog.axis[m] = molecular_cog[m];
   }

   // pass 2b move bead_ref_volume, ref_mw, computed_radius from
   // next main chain back one including adjustments for GLY, PRO, OXT

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      bool placed_N1 = false;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         bool first_is_pro = false;
         unsigned int main_chain_beads = 0;
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            if ( this_atom->active &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 )
            {
               main_chain_beads++;
            }
            if ( 
                misc_pb_rule_on &&
                !k &&
                this_atom->resName == "PRO" 
                )
            {
               first_is_pro = true;
            }
            if ( placed_N1 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName))
                 )
            {
               placed_N1 = false;
            }

            if ( 
                misc_pb_rule_on &&
                !k &&
                this_atom->resName != "PRO" &&
                this_atom->name == "N" &&
                !broken_chain_head.count(QString("%1|%2")
                                         .arg(this_atom->resSeq)
                                         .arg(this_atom->resName))
                )
            {
               placed_N1 = true;
            }
            if ( 
                first_is_pro &&
                this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 1 &&
                this_atom->resName == "PRO"
                )
            {
               this_atom->bead_ref_mw += 1.0;
               // what about a volume adjustment?
               first_is_pro = false;
            }
                 
            if (this_atom->name == "OXT" &&
                last_main_chain_bead) {
               this_atom->is_bead = false;
               // override broken head OXT residue
               if ( misc_pb_rule_on &&
                    this_atom->resName != "PRO" &&
                    broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                    multi_residue_map.count("NPBR-OXT") )
               {
                  int posNPBR_OXT = multi_residue_map["NPBR-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[posNPBR_OXT].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[posNPBR_OXT].r_bead[0].mw;
               }
                  
               last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
               last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
               if (last_main_chain_bead->resName == "GLY") 
               {
                  last_main_chain_bead->bead_ref_mw += 1.01f;
               }
               if ( !misc_pb_rule_on &&
                    main_chain_beads == 1 &&
                    this_atom->resName != "PRO" )
               {
                  last_main_chain_bead->bead_ref_mw += 1.0;
               }
               last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
            } // OXT

            if (this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 0) {

               if (misc_pb_rule_on &&
                   last_main_chain_bead &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                   (this_atom->resName == "PRO" ||
                    last_main_chain_bead->resName == "PRO")
                   ) {

                  last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
                  last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
                  if (this_atom->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw -= 1.01f;
                  }
                  if (last_main_chain_bead->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw += 1.01f;
                  }
               } // PRO
               last_main_chain_bead = this_atom;
            }

            // fix up mw, vol at end for broken end when PBR rule is on
            if ( misc_pb_rule_on &&
                 this_atom->p_residue->type == 0 &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 !broken_chain_head.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO" &&
                 (k || this_atom->name != "N")
                 )
            {
               if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
               {
                  int pos = multi_residue_map["PBR-NO-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[pos].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[pos].r_bead[0].mw;
                  this_atom->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
                  if (this_atom->resName == "GLY") {
                     this_atom->bead_ref_mw += 1.01f;
                  }
               }
               else
               {
                  cout << "Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.\n";
               }
            }
         } // for k < atom.size()
         // fix up mw, vol at end for no OXT when PBR rule is on
         if ( misc_pb_rule_on &&
              last_main_chain_bead &&
              last_main_chain_bead->p_residue->type == 0 &&
              last_main_chain_bead->name != "PRO" &&
              !has_OXT[QString("%1|%2|%3")
               .arg(j)
               .arg(last_main_chain_bead->resName)
               .arg(last_main_chain_bead->resSeq)]  &&
              !broken_chain_head.count(QString("%1|%2")
                                       .arg(last_main_chain_bead->resSeq)
                                       .arg(last_main_chain_bead->resName)) )
         {
            if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
            {
               int pos = multi_residue_map["PBR-NO-OXT"][0];
               last_main_chain_bead->bead_ref_volume = residue_list[pos].r_bead[0].volume;
               last_main_chain_bead->bead_ref_mw = residue_list[pos].r_bead[0].mw;
               last_main_chain_bead->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
               if (last_main_chain_bead->resName == "GLY") {
                  last_main_chain_bead->bead_ref_mw += 1.01f;
               }
            }
            else
            {
               noticemsg += "Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.\n";
	       noticemsg_udp += "Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.\\n";
            }
         }
      } // for j < molecule.size()
   }


   // pass 2c hydration

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->is_bead) {
               this_atom->bead_ref_volume_unhydrated = this_atom->bead_ref_volume;
               this_atom->bead_ref_volume += misc_hydrovol * this_atom->bead_hydration;
               this_atom->bead_computed_radius = pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
            }
         }
      }
   }

   // pass 3 determine visibility, exposed code, normalize cog position, final position determination
   // compute com of entire molecule

   // for (unsigned int i = 0; i < model_vector.size (); i++) {
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {

         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {


            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p3 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            this_atom->exposed_code = -1;
            if (this_atom->active &&
                this_atom->is_bead) {

               for (unsigned int m = 0; m < 3; m++) {
                  if (this_atom->bead_cog_mw) {
                     this_atom->bead_cog_coordinate.axis[m] /= this_atom->bead_cog_mw;
                  }
                  else 
                  {
                     this_atom->bead_cog_coordinate.axis[m] = 0;
                  }
               }

               if (this_atom->p_residue && this_atom->p_atom) {

                  switch (this_atom->placing_method) {

                  case 0 : // cog
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     // if (this_atom->bead_positioner) {
                     // fprintf(stderr, "warning: this bead had a atom claiming position & a bead placing method of cog! %s %s %d\n",
                     //   this_atom->name.toLatin1().data(),
                     //   this_atom->resName.toLatin1().data(),
                     //   this_atom->serial);
                     // }
                     break;
                  case 1 : // positioner
                     this_atom->bead_coordinate = this_atom->bead_position_coordinate;
                     break;
                  case 2 : // no positioning necessary
                     this_atom->bead_coordinate = this_atom->coordinate;
                     break;
                  default :
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     fprintf(stderr, "warning: unknown bead placing method %d %s %s %d <using cog!>\n",
                             this_atom->placing_method,
                             this_atom->name.toLatin1().data(),
                             this_atom->resName.toLatin1().data(),
                             this_atom->serial);
                     break;
                  }
               }
               else 
               {
                  errormsg = QString("").sprintf( "serious internal error 1 on %s %s %d, quitting\n",
                                                  this_atom->name.toLatin1().data(),
                                                  this_atom->resName.toLatin1().data(),
                                                  this_atom->serial );
                  return false;
                  break;
               }
               this_atom->visibility = 
                  ( this_atom->bead_asa >= control_parameters[ "asahydratethresh" ].toDouble() );

#if defined( USUH_DEBUG_ASA )
               cout << QString( "bead_asa %1 %2 %3 %4 %5\n" ).arg( this_atom->name ).arg( this_atom->resName ).arg( this_atom->serial ).arg( this_atom->bead_asa ).arg( control_parameters[ "asahydratethresh" ].toDouble() ) ;
#endif

               if (!create_beads_normally ||
                   this_atom->visibility ||
                   !control_parameters[ "asacalculation" ].toUInt() ) {
                  this_atom->exposed_code = 1;  // exposed
               }
               else 
               {
                  if (this_atom->chain == 0) {
                     this_atom->exposed_code = 10;  // main chain, buried
                  }
                  if (this_atom->chain == 1) {
                     this_atom->exposed_code = 6;   // side chain, buried
                  }
               }
            }
            else 
            {
               this_atom->placing_method = -1;
            }
         }
      }
   }
		    
   return true;
}

class sortable_PDB_atom {
public:
   PDB_atom pdb_atom;
   bool PRO_N_override;
   bool pb_rule_on;
   bool operator < (const sortable_PDB_atom& objIn) const
   {
      if (
          //   (PRO_N_override ? 0 : pdb_atom.bead_assignment)
          ( !pb_rule_on &&
            ( pdb_atom.bead_assignment < 
              objIn.pdb_atom.bead_assignment ) ) ? 1 : (
                                                pdb_atom.atom_assignment
                                                <
          //   (objIn.PRO_N_override ? 0 : objIn.pdb_atom.bead_assignment)
                                                objIn.pdb_atom.atom_assignment
                                                )
          )
      {
         return (true);
      }
      else
      {
         return (false);
      }
   }
};

bool US_Saxs_Util::create_beads()
{
   create_beads_normally = true;

   active_atoms.clear( );

   // #define DEBUG_MM
   get_atom_map(&model_vector[current_model]);

   QRegExp count_hydrogens("H(\\d)");

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
   {
      QString last_resSeq = "";
      QString last_resName = "";
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         // initialize data
         this_atom->active = false;
         this_atom->asa = 0;
         this_atom->p_residue = 0;
         this_atom->p_atom = 0;
         this_atom->radius = 0;
         this_atom->bead_hydration = 0;
         this_atom->bead_color = 0;
         this_atom->bead_ref_volume = 0;
         this_atom->bead_ref_mw = 0;
         this_atom->bead_assignment = -1;
         this_atom->atom_assignment = -1;
         this_atom->chain = -1;
         this_atom->type = 99;

         // find residue in residues
         int respos = -1;

         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         for (unsigned int m = 0; m < residue_list.size(); m++)
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               continue;
            }

            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" ||
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc_pb_rule_on)) ||

                // if pb_rule is off, final OXT before P needs to use OXT-P
                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc_pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc_pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&                  
                 misc_pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               this_atom->p_residue = &(residue_list[m]);
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               errormsg.append(QString("").sprintf("unknown residue molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.toLatin1().data(),
                                                        this_atom->resName.toLatin1().data(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
               return false;
            }
         } 
         else 
         {
         }
         int atompos = -1;

         if (respos != -1)
         {
            // clear tmp_used if new resSeq
#if defined(DEBUG)
            printf("respos %d != -1 last used %s %s\n", respos, this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
            if (this_atom->resSeq != last_resSeq ||
                this_atom->resName != last_resName ||
                residue_list[respos].name == "OXT" ||
                residue_list[respos].name == "OXT-P" ||
                residue_list[respos].name == "N1")
            {
#if defined(DEBUG)
               printf("clear last used %s %s\n", this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
               for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
               {
                  residue_list[respos].r_atom[m].tmp_used = false;
               }
               last_resSeq = this_atom->resSeq;
               last_resName = this_atom->resName;
               if(residue_list[respos].name == "OXT" ||
                  residue_list[respos].name == "OXT-P" ||
                  residue_list[respos].name == "N1") {
                  last_resSeq = "";
               }
            }
#if defined(DEBUG)
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if(residue_list[respos].r_atom[m].tmp_used) {
                  printf("used %u %u\n", respos, m);
               }
            }
#endif

            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
#if defined(DEBUG)
               if(this_atom->name == "N" && !k && misc_pb_rule_on) {
                  printf("this_atom->name == N/N1 this residue_list[%d].r_atom[%d].name == %s\n",
                         respos, m, residue_list[respos].r_atom[m].name.toLatin1().data());
               }
#endif
               if (!residue_list[respos].r_atom[m].tmp_used &&
                   (residue_list[respos].r_atom[m].name == this_atom->name ||
                    (
                     this_atom->name == "N" &&
                     misc_pb_rule_on &&
                     !k &&
                     residue_list[respos].r_atom[m].name == "N1" &&
                     !broken_chain_head.count(QString("%1|%2")
                                              .arg(this_atom->resSeq)
                                              .arg(this_atom->resName)) &&
                     this_atom->resName != "PRO"
                     )
                    )
                   )
               {
                  residue_list[respos].r_atom[m].tmp_used = true;
                  this_atom->p_atom = &(residue_list[respos].r_atom[m]);
                  atompos = (int) m;
                  break;
               }
            }

            if (atompos == -1)
            {
               errormsg.append(QString("").sprintf("unknown atom molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                    j + 1, k, this_atom->name.toLatin1().data(),
                                                    this_atom->resName.toLatin1().data(),
                                                    this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
            } 
            else 
            {
               this_atom->active = true;
               this_atom->radius = residue_list[respos].r_atom[atompos].hybrid.radius;
               this_atom->mw = residue_list[respos].r_atom[atompos].hybrid.mw;
                  
               this_atom->placing_method =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method;
               this_atom->bead_hydration =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].hydration;
               this_atom->bead_color =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].color;
               this_atom->bead_ref_volume =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].volume;
               this_atom->bead_ref_mw =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].mw;
               this_atom->ref_asa =  this_atom->p_residue->asa;
               this_atom->bead_computed_radius =  pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
               this_atom->bead_assignment = this_atom->p_atom->bead_assignment;
               this_atom->atom_assignment = atompos;
               this_atom->chain = (int) this_atom->p_residue->r_bead[this_atom->bead_assignment].chain;
               this_atom->atom_hydration =  residue_list[respos].r_atom[atompos].hydration;
               this_atom->type = this_atom->p_residue->type;

#if defined( SAXS_FOR_BEAD_MODELS )
               if ( our_saxs_options.compute_saxs_coeff_for_bead_models )
               {
                  QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
                  if ( this_atom->name == "OXT" )
                  {
                     mapkey = "OXT|OXT";
                  }
                  
                  if ( !residue_atom_hybrid_map.count(mapkey) )
                  {
                     cout << QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                        .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                        .arg(j+1)
                        .arg(this_atom->resName)
                        .arg(this_atom->resSeq);
                  } else {
                     QString hybrid_name = residue_atom_hybrid_map[mapkey];

                     if ( hybrid_name.isEmpty() || !hybrid_name.length() )
                     {
                        cout << QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq);
                     } else {
                        if ( !saxs_util->hybrid_map.count(hybrid_name) )
                        {
                           cout << QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name);
                        } else {
                           if ( !saxs_util->atom_map.count(this_atom->name + "~" + hybrid_name) )
                           {
                              cout << QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                                 .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                 .arg(j+1)
                                 .arg(this_atom->name)
                                 .arg(this_atom->resName)
                                 .arg(this_atom->resSeq)
                                 .arg(hybrid_name);
                           } else {
                              this_atom->saxs_name = saxs_util->hybrid_map[hybrid_name].saxs_name; 
                              this_atom->hybrid_name = hybrid_name;
                              this_atom->hydrogens = 0;
                              if ( count_hydrogens.indexIn(hybrid_name) != -1 )
                              {
                                 this_atom->hydrogens = count_hydrogens.cap(1).toInt();
                              }
                              this_atom->saxs_excl_vol = saxs_util->atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
                              if ( !saxs_util->saxs_map.count(saxs_util->hybrid_map[hybrid_name].saxs_name) )
                              {
                                 cout << QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(hybrid_name)
                                    .arg(saxs_util->hybrid_map[hybrid_name].saxs_name);
                              } else {
                                 this_atom->saxs_data = saxs_util->saxs_map[saxs_util->hybrid_map[hybrid_name].saxs_name];
                              }
                           }
                        }
                     }
                  }
               } // saxs setup
#endif

               if ( misc_pb_rule_on &&
                    this_atom->resName == "PRO" &&
                    this_atom->name == "N" &&
                    !k )
               {
                  this_atom->mw += 1.0;
                  // bead ref mw handled in pass 2b
               }
               if ( !misc_pb_rule_on &&
                    this_atom->name == "N" &&
                    !k )
               {
                  this_atom->mw += 1.0;
                  this_atom->bead_ref_mw += 1.0;
               }
            }
         }

         if (this_atom->active)
         {
            this_atom->active = false;
            if (this_atom->name != "H" &&
                this_atom->name != "D" &&
                this_atom->p_residue->name != "DOD" &&
                this_atom->p_residue->name != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " "))
            {
               this_atom->active = true;
               active_atoms.push_back(this_atom);
            }
            else 
            {
#if defined(DEBUG)
               printf
                  ("skipped bound waters & H %s %s rad %f resseq %s\n",
                   this_atom->name.toLatin1().data(), this_atom->resName.toLatin1().data(), this_atom->radius, this_atom->resSeq.toLatin1().data());
               fflush(stdout);
#endif
            }
         }
      }
   }
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
                        
            // this_atom->bead_assignment =
            // (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            // this_atom->chain =
            // ((this_atom->p_residue && this_atom->p_atom) ?
            //  (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);
            this_atom->org_chain = this_atom->chain;
            this_atom->bead_positioner = this_atom->p_atom ? this_atom->p_atom->positioner : false;
         }
      }
   }
#define DO_SORT
#if defined(DO_SORT)
   // reorder residue
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         QString last_resSeq = "";
         list <sortable_PDB_atom> last_residue_atoms;
         unsigned int k;
         for (k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

            if (this_atom->resSeq != last_resSeq) {
               if (last_resSeq != "") {
                  // reorder previous residue
                  last_residue_atoms.sort();
                  int base_ofs = (int) last_residue_atoms.size();
                  // printf("resort last residue... size/base %d k %u\n", base_ofs, k);
                  for (unsigned int m = k - base_ofs; m < k; m++) {
                     // printf("resort m = %u size lra %u\n", m, last_residue_atoms.size());
                     model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
                     last_residue_atoms.pop_front();
                  }
               }
               last_resSeq = this_atom->resSeq;
            }
            sortable_PDB_atom tmp_sortable_pdb_atom;
            tmp_sortable_pdb_atom.pdb_atom = *this_atom;
            tmp_sortable_pdb_atom.PRO_N_override = false;
            tmp_sortable_pdb_atom.pb_rule_on = misc_pb_rule_on;
            if(misc_pb_rule_on &&
               this_atom->resName == "PRO" &&
               this_atom->name == "N") {
               tmp_sortable_pdb_atom.PRO_N_override = true;
            }
            last_residue_atoms.push_back(tmp_sortable_pdb_atom);
         }
         if (last_resSeq != "") {
            // reorder 'last' residue
            last_residue_atoms.sort();
            int base_ofs = (int) last_residue_atoms.size();
            // printf("final resort last residue... size/base %d k %u\n", base_ofs, k);
            for (unsigned int m = k - base_ofs; m < k; m++) {
               //       printf("finalresort m = %u size lra %u\n", m, last_residue_atoms.size());
               model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
               last_residue_atoms.pop_front();
            }
         }
      }
   }
#endif
#if !defined(DO_SORT)
   puts("sorting disabled");
#endif

   return true;
}

void US_Saxs_Util::get_atom_map( PDB_model *model )
{
   atom_counts.clear( );
   has_OXT.clear( );

   for ( unsigned int j = 0; j < model->molecule.size(); j++ )
   {
      QString lastResSeq = "";
      QString lastResName = "";
      int atom_count = 0;
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if (lastResSeq != this_atom->resSeq ||
             lastResName != this_atom->resName)
         {
            // new residue
            if ( lastResSeq != "" )
            {
               atom_counts[QString("%1|%2|%3")
                           .arg(j)
                           .arg(lastResName)
                           .arg(lastResSeq)] = atom_count;
            }
            lastResSeq = this_atom->resSeq;
            lastResName = this_atom->resName;
            atom_count = 0;
         }
         if(this_atom->name == "OXT") {
            has_OXT[QString("%1|%2|%3")
                    .arg(j)
                    .arg(this_atom->resName)
                    .arg(this_atom->resSeq)]++;
         }
         atom_count++;
      }
      if ( lastResSeq != "" )
      {
         atom_counts[QString("%1|%2|%3")
                     .arg(j)
                     .arg(lastResName)
                     .arg(lastResSeq)] = atom_count;
      }
   }

   // pass for broken chains
   // later add distance check for CA-N ?
   broken_chain_end.clear( );
   broken_chain_head.clear( );
   int breaks = 0;
   int total_aa = 0;
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastResSeq = "";
      QString lastResName = "";
      QString lastChainID = "";
      // count non AA types
      int non_aa = 0;
      int aa = 0;
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         if ( lastResSeq != this_atom->resSeq ||
              lastResName != this_atom->resName )
         {
            if ( multi_residue_map.count(this_atom->resName) &&
                 multi_residue_map[this_atom->resName][0] >= 0 &&
                 multi_residue_map[this_atom->resName][0] < (int)residue_list.size() &&
                 residue_list[multi_residue_map[this_atom->resName][0]].type == 0 )
            {
               aa++;
            }
            else
            {
               non_aa++;
            }
         }
      }
      if ( aa ) 
      {
         lastResSeq = "";
         lastResName = "";
         non_aa = 0;
         aa = 0;
         for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model->molecule[j].atom[k]);
            
            if ( lastResSeq != this_atom->resSeq ||
                 lastResName != this_atom->resName )
            {
               if ( multi_residue_map.count(this_atom->resName) &&
                    residue_list[multi_residue_map[this_atom->resName][0]].type == 0 )
               {
                  aa++;
                  total_aa++;
               }
               else
               {
                  non_aa++;
               }
               if ( 
                   ( lastResSeq != "" &&
                     ( lastResSeq.toInt() + 1 !=  this_atom->resSeq.toInt() ||
                       !multi_residue_map.count(this_atom->resName) ||
                       unknown_residues.count(this_atom->resName) ||
                       unknown_residues.count(lastResName) ) ) ||
                   ( lastResSeq == "" &&
                     unknown_residues.count(this_atom->resName) 
                     )
                   )
               {
                  breaks++;
                  if ( lastResSeq != "" )
                  {
                     broken_chain_end[QString("%1|%2")
                                      .arg(lastResSeq)
                                      .arg(lastResName)] = true;
                  } 
                  else
                  {
                     broken_chain_end[QString("%1|%2")
                                      .arg(this_atom->resSeq)
                                      .arg(this_atom->resName)] = true;
                  }
                  
                  broken_chain_head[QString("%1|%2")
                                    .arg(this_atom->resSeq)
                                    .arg(this_atom->resName)] = true;
                  
                  //               if ( multi_residue_map.count(this_atom->resName) &&
                  //   residue_list[multi_residue_map[this_atom->resName][0]].type == 0 &&
                  //   multi_residue_map.count(lastResName) &&
                  //   residue_list[multi_residue_map[lastResName][0]].type == 0 )
                  {

		    us_log->log(lastResSeq != "" ? 
				QString( "Warning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4 & %5 %6.\n" )
				.arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
				.arg(j + 1)
				.arg(lastResName)
				.arg(lastResSeq)
				.arg(this_atom->resName)
				.arg(this_atom->resSeq) 
				:
				QString( "Warning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4.\n")
				.arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
				.arg(j + 1)
				.arg(this_atom->resName)
				.arg(this_atom->resSeq));

		    if ( us_udp_msg )
		      {
			map < QString, QString > msging;
			msging[ "_textarea" ] = lastResSeq != "" ? 
			  QString( "\\nWarning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4 & %5 %6.\\n" )
			  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
			  .arg(j + 1)
			  .arg(lastResName)
			  .arg(lastResSeq)
			  .arg(this_atom->resName)
			  .arg(this_atom->resSeq) 
			  :
			  QString( "\\nWarning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4.\\n")
			  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
			  .arg(j + 1)
			  .arg(this_atom->resName)
			  .arg(this_atom->resSeq);

			us_udp_msg->send_json( msging );
		    //sleep(1);
		      }
		    accumulated_msgs += lastResSeq != "" ? 
			  QString( "\\nWarning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4 & %5 %6.\\n" )
			  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
			  .arg(j + 1)
			  .arg(lastResName)
			  .arg(lastResSeq)
			  .arg(this_atom->resName)
			  .arg(this_atom->resSeq) 
			  :
			  QString( "\\nWarning: break in residue sequence or unknown residue: %1Molecule %2 Residue %3 %4.\\n")
			  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
			  .arg(j + 1)
			  .arg(this_atom->resName)
			  .arg(this_atom->resSeq);

                  }
               }
               lastChainID = this_atom->chainID;
               lastResSeq = this_atom->resSeq;
               lastResName = this_atom->resName;
            }
         }
      }
      if ( aa && non_aa )
	{	 
	  us_log->log(QString( "Notice: %1found %2 non or unknown Amino Acids in a chain containing %3 AA Residues.\n")
		      .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
		      .arg(non_aa)
		      .arg(aa));
	  if ( us_udp_msg )
	    {
	      map < QString, QString > msging;
	      msging[ "_textarea" ] = QString( "Notice: %1found %2 non or unknown Amino Acids in a chain containing %3 AA Residues. \\n")
		.arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
		.arg(non_aa)
		.arg(aa);
	      us_udp_msg->send_json( msging );
			  //sleep(1);
	    }
	  accumulated_msgs += QString( "Notice: %1found %2 non or unknown Amino Acids in a chain containing %3 AA Residues. \\n")
		.arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
		.arg(non_aa)
		.arg(aa);
	    
	  
	}
   }
   

   // us_log->log(noticemsg);
   // if ( us_udp_msg )
   //   {
   //     map < QString, QString > msging;
   //     msging[ "_textarea" ] = noticemsg_udp;
   //     us_udp_msg->send_json( msging );
   // 	     //sleep(1);
   //   }
   
   // should be chain based
   if ( !total_aa ) // || !advanced_config.pbr_broken_logic )
   {
      broken_chain_end.clear( );
      broken_chain_head.clear( );
   }

   // end of atom_counts & has_OXT map create
}

void US_Saxs_Util::build_molecule_maps( PDB_model *model )
{
   // creates molecules_residues_atoms map
   // molecules_residues_atoms maps molecule #|resSeq to vector of atom names
   molecules_residues_atoms.clear( );
   // molecules_residue_name maps molecule #|resSeq to residue name
   molecules_residue_name.clear( );
   // molecules_idx_seq is a vector of the idx's
   molecules_idx_seq.clear( );
   molecules_residue_errors.clear( );
   molecules_residue_missing_counts.clear( );
   molecules_residue_min_missing.clear( );
   molecules_residue_missing_atoms.clear( );
   molecules_residue_missing_atoms_beads.clear( );
   molecules_residue_missing_atoms_skip.clear( );
   use_residue.clear( );
   skip_residue.clear( );

   map < QString, QString > chains;
   map < QString, QString > resseqs;
   map < QString, QString > resnames;

   // pass 1 setup molecule basic maps
   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      for (unsigned int k = 0; k < model->molecule[j].atom.size(); k++)
      {
         QString idx = QString("%1|%2").arg(j).arg(model->molecule[j].atom[k].resSeq);
         chains[ idx ] = model->molecule[j].atom[k].orgChainID;
         resseqs[ idx ] = model->molecule[j].atom[k].orgResSeq;
         resnames[ idx ] = model->molecule[j].atom[k].orgResName;
         molecules_residues_atoms[idx].push_back(model->molecule[j].atom[k].name);
         if ( !molecules_residue_name.count(idx) )
         {
            molecules_residue_name[idx] = model->molecule[j].atom[k].resName;
            molecules_idx_seq.push_back(idx);
         }
      }
   }

   // pass 2 setup error maps
   for (unsigned int i = 0; i < molecules_idx_seq.size(); i++)
   {
      QString idx = molecules_idx_seq[i];
      QString resName = molecules_residue_name[idx];
      if (multi_residue_map.count(resName))
      {
         int minimum_missing = INT_MAX; // find the entry with the minimum # of missing atoms
         int minimum_missing_pos = 0;   // and its position
         for (unsigned int j = 0; j < multi_residue_map[resName].size(); j++)
         {
            QString errormsg = "";
            int missing_count = 0;
            // for this residue, clear flags
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = false;
            }
            skip_residue[QString("%1|%2").arg(idx).arg(multi_residue_map[resName][j])] = false;

            // now set flags
            // first check for non-coded atoms
            for (unsigned int k = 0; k < molecules_residues_atoms[idx].size(); k++)
            {
               bool found = false;
               bool any = false;
               for (unsigned int r = 0; 
                    !found && r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                    r++)
               {
                  if (residue_list[multi_residue_map[resName][j]].r_atom[r].name == 
                      molecules_residues_atoms[idx][k]) 
                  {
                     any = true;
                     if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
                     {
                        residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used = true;
                        found = true;
                     }
                  }
               }
               if (!found)
               {
                  missing_count = -1;
                  errormsg += QString("%1coded atom %2. ")
                     .arg(any ? "Duplicate " : "Non-")
                     .arg(molecules_residues_atoms[idx][k]);
                  residue_errors[ chains[ idx ] + "~" + resnames[ idx ] + " " + resseqs[ idx ] ] = true;
               }
            }
            // now check for missing atoms
            vector < QString > missing_atoms;
            vector < unsigned int > missing_atoms_beads;
            for (unsigned int r = 0; 
                 r < residue_list[multi_residue_map[resName][j]].r_atom.size(); 
                 r++)
            {
               if (!residue_list[multi_residue_map[resName][j]].r_atom[r].tmp_used)
               {
                  errormsg += QString("Missing atom %1. ")
                     .arg(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                  if ( missing_count != -1 )
                  {
                     missing_atoms.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].name);
                     missing_atoms_beads.push_back(residue_list[multi_residue_map[resName][j]].r_atom[r].bead_assignment);
                     molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                          .arg(idx)
                                                          .arg(j)
                                                          .arg(r)] = true;
                     missing_count++;
                  }
               }
            }
            molecules_residue_errors[idx].push_back(errormsg);
            molecules_residue_missing_counts[idx].push_back(missing_count);
            molecules_residue_missing_atoms[idx].push_back(missing_atoms);
            molecules_residue_missing_atoms_beads[idx].push_back(missing_atoms_beads);
            if ( missing_count >= 0 && missing_count < minimum_missing )
            {
               minimum_missing = missing_count;
               minimum_missing_pos = j;
            }
         }
         if ( minimum_missing < INT_MAX )
         {
            molecules_residue_min_missing[idx] = minimum_missing_pos;
         } 
         else
         {
            molecules_residue_min_missing[idx] = -1;
         }
      }
      else 
      {
         molecules_residue_errors[idx].push_back("Non-coded residue. ");
         molecules_residue_missing_counts[idx].push_back(-1);
         molecules_residue_min_missing[idx] = -1;
         unknown_residues[resName] = true;
         residue_errors[ chains[ idx ] + "~" + resnames[ idx ] + " " + resseqs[ idx ] ] = true;
      }
      {
         int k = molecules_residue_min_missing[idx];
         if ( k > -1  &&
              molecules_residue_missing_atoms[idx][k].size() == 0 &&
              molecules_residue_missing_atoms_beads[idx][k].size() == 0
              )
         {
            use_residue[idx] = multi_residue_map[resName][k];
            // skip the rest
            for (unsigned int j = 0; j < multi_residue_map[resName].size(); j++)
            {
               if ( j != (unsigned int) k )
               {
                  skip_residue[QString("%1|%2").arg(idx).arg(multi_residue_map[resName][j])] = true;
               }
            }
            molecules_residue_errors[idx].clear( );
            molecules_residue_missing_counts[idx].clear( );
            molecules_residue_missing_atoms[idx].clear( );
            molecules_residue_missing_atoms_beads[idx].clear( );
         } else {
            use_residue[idx] = -1;
         }
      }
   }
}

bool US_Saxs_Util::check_for_missing_atoms( PDB_model *model )
{
   QStringList qsl;
   return check_for_missing_atoms( model, qsl );
}

bool US_Saxs_Util::check_for_missing_atoms( PDB_model *model, QStringList &qsl )
{
   // go through molecules, build vector of residues
   // expand vector of residues to atom list from residue file
   // compare expanded list of residues to model ... list missing atoms missing
   residue_errors.clear( );
   int errors_found = 0;
   get_atom_map(model);

   if ( misc_pb_rule_on &&
        (broken_chain_head.size() || broken_chain_end.size()) )
   {
      misc_pb_rule_on = false;
      cout << "Notice: Broken chain turns off Peptide Bond Rule.\n";
      if ( qsl.size() )
      {
         if ( !read_pdb( qsl ) )
         {
            return false;
         }
      } else {
         if ( !read_pdb( control_parameters[ "inputfile" ] ) )
         {
            return false;
         }
      }
      get_atom_map( model );
   }
         
   int failure_errors = 0;
   bead_exceptions.clear( );

   // residue types are for automatic build builder to determine
   // if we have a protein, so that a special 'temporary' residue can
   // be created

   vector < map < unsigned int, unsigned int > > residue_types;
   vector < unsigned int > last_residue_type;

   residue_types.resize(model->molecule.size());
   last_residue_type.resize(model->molecule.size());

   build_molecule_maps(model);
   QString abb_msgs = "";

   // keep track of errors shown
   map < QString, bool > error_shown;

   for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
      QString lastOrgResSeq = "";
      QString lastOrgResName = "";
      QString lastOrgChainID = "";

      QString lastResSeq = "";
      int lastResPos = -1;
      QString lastChainID = " ";
      bool spec_N1 = false;
      QString last_count_idx;
      unsigned int residues_found = 0;
      unsigned int N1s_placed = 0;
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         this_atom->active = false;
         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         if (!bead_exceptions[count_idx])
         {
            bead_exceptions[count_idx] = 1;
         }

         // find residue in residues
         int respos = -1;
         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         for ( unsigned int m = 0; m < residue_list.size(); m++ )
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               continue;
            }
            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" || 
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc_pb_rule_on)) ||
                
                // if pb_rule is off, final OXT before P needs to use OXT-P

                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc_pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc_pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&
                 misc_pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               residue_types[j][residue_list[m].type]++;
               last_residue_type[j] = residue_list[m].type;
               this_atom->p_residue = &(residue_list[m]);
               if (lastResSeq != this_atom->resSeq)
               {
                  // new residue
                  // printf("new residue %s\n", this_atom->resSeq.toLatin1().data());
                  residues_found++;
                  if (lastResPos != -1)
                  {
                     // check for false entries in last residue and warn about them
                     for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
                     {
                        if (spec_N1 &&
                            misc_pb_rule_on &&
                            residue_list[lastResPos].r_atom[l].name == "N") {
                           residue_list[lastResPos].r_atom[l].tmp_flag = true;
                           spec_N1 = false;
                        }

                        if (!residue_list[lastResPos].r_atom[l].tmp_flag)
                        {
                           // missing atoms
                           errors_found++;
                           if (control_parameters[ "pdbmissingatoms" ].toUInt() == 0)
                           {
                              failure_errors++;
                           }
                           if (control_parameters[ "pdbmissingatoms" ].toUInt() == 1)
                           {
                              bead_exceptions[last_count_idx] = 2;
                           }
                           if (control_parameters[ "pdbmissingatoms" ].toUInt() == 2)
                           {
                              if ( bead_exceptions[last_count_idx] == 1 ) 
                              {
                                 bead_exceptions[last_count_idx] = 4;
                              }
                           }
                           if (!error_shown[last_count_idx]) 
                           {
                              QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                                 .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                                 .arg(j + 1)
                                 .arg(residue_list[lastResPos].name)
                                 .arg(lastResSeq);
                              QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                              residue_errors[ lastOrgChainID + "~" + 
                                              lastOrgResName
                                              + " " +
                                              lastOrgResSeq ] = true;
                              switch (molecules_residue_errors[idx].size())
                              {
                              case 0: 
                                 this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                                 break;
                              case 1:
                                 this_error += molecules_residue_errors[idx][0] + "\n";
                                 break;
                              default :
                                 {
                                    this_error += "\n";
                                    for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                                    {
                                       this_error += QString("    Residue file entry %1: %2\n").
                                          arg(t+1).arg(molecules_residue_errors[idx][t]);
                                    }
                                 }
                                 break;
                              }
                              errormsg += this_error;
                              // cout << QString("dbg 1: idx <%1> msg <%2>\n").arg(last_count_idx).arg(this_error);
                              error_shown[last_count_idx] = true;
                           }
                        }
                     }
                  }

                  // reset residue list
                  // printf("reset residue list for residue_list[%d]\n", m);
                  for (unsigned int l = 0; l < residue_list[m].r_atom.size(); l++)
                  {
                     residue_list[m].r_atom[l].tmp_flag = false;
                  }
                  lastResSeq = this_atom->resSeq;
                  lastChainID = this_atom->chainID;
                  lastResPos = (int) m;
                  last_count_idx = count_idx;
                  lastOrgResSeq  = this_atom->orgResSeq;
                  lastOrgChainID = this_atom->orgChainID;
                  lastOrgResName = this_atom->orgResName;
               }

               if (residue_list[m].name == "N1")
               {
                  lastResSeq = "";
                  lastResPos = -1;
                  spec_N1 = true;
                  N1s_placed++;
                  residues_found--;
               }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               QString msg_tag;
               bool do_errormsg = true;
               residue_errors[ this_atom->orgChainID + "~" + this_atom->orgResName + " " + this_atom->orgResSeq ] = true;
               if (control_parameters[ "pdbmissingresidues" ].toUInt() == 0 &&
                   control_parameters[ "pdbmissingatoms" ].toUInt() == 0)
               {
                  failure_errors++;
                  msg_tag = "Missing residue or atom";
               } 
               else 
               {
                  // ok, we have three cases here:
                  // 1. residue does exist & residue/atom doesn't
                  //    1.1 skip missing atoms controls
                  // 2. residue does exist & residue/atom does
                  //    2.1 there must be a missing atom since count doesn't match so atom controls
                  // 3. residue does not exist
                  //    3.1 skip missing residue control
                  // ---------------------
                  //
                  // note: we're just checking the 1st of multiple possibilities
                  puts("cases---");
                  if (multi_residue_map[this_atom->resName].size()) 
                  {
                     // residue exists, does residue/atom?
                     QString idx = QString("%1|%2|%3")
                        .arg(this_atom->resName)
                        .arg(this_atom->name)
                        .arg(0); 
                     printf("cases residue found: idx %s\n", idx.toLatin1().data());
                     if (valid_atom_map[idx].size()) 
                     {
                        puts("case 2.1");
                        msg_tag = "Missing or extra atom in residue";
                     } 
                     else
                     {
                        // atom does not exist, skip missing atoms controls
                        puts("case 1.1");
                        msg_tag = "Missing atom";
                     }
                     if (control_parameters[ "pdbmissingatoms" ].toUInt() == 0)
                     {
                        failure_errors++;
                     }
                     if (control_parameters[ "pdbmissingatoms" ].toUInt() == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (control_parameters[ "pdbmissingatoms" ].toUInt() == 2)
                     {
                        if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 )
                        {
                           // fall back to non-coded residue 
                           switch ( control_parameters[ "pdbmissingresidues" ].toUInt() )
                           {
                           case 0 :
                              failure_errors++;
                              break;
                           case 1 :
                              bead_exceptions[count_idx] = 2;
                              break;
                           case 2 :
                              bead_exceptions[count_idx] = 3;
                              break;
                           default :
                              failure_errors++;
                              errormsg = "Internal error: Unexpected PdbMissingResidues type " +
                                                   QString("%1\n").arg(control_parameters[ "pdbmissingresidues" ].toUInt());
                              return false;
                              break;
                           }
                        } 
                        else
                        {
                           if ( bead_exceptions[count_idx] == 1 ) 
                           {
                              bead_exceptions[count_idx] = 4;
                           }
                        }
                     }
                  } 
                  else 
                  {
                     // residue does not exist, skip missing residue controls
                     puts("case 3.1");
                     msg_tag = "Non-coded residue";
                     if (control_parameters[ "pdbmissingresidues" ].toUInt() == 0)
                     {
                        failure_errors++;
                     }
                     if (control_parameters[ "pdbmissingresidues" ].toUInt() == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (control_parameters[ "pdbmissingresidues" ].toUInt() == 2)
                     {
                        bead_exceptions[count_idx] = 3;
                     }
                  }
               }
               if (do_errormsg) {
                  errors_found++;
                  if (!error_shown[count_idx]) 
                  {
                     QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                        .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                        .arg(j + 1)
                        .arg(this_atom->resName)
                        .arg(this_atom->resSeq);
                     QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                     switch (molecules_residue_errors[idx].size())
                     {
                     case 0: 
                        this_error += msg_tag;
                        break;
                     case 1:
                        this_error += molecules_residue_errors[idx][0] + "\n";
                        break;
                     default :
                        {
                           this_error += "\n";
                           for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                           {
                              this_error += QString("    Residue file entry %1: %2\n").
                                 arg(t+1).arg(molecules_residue_errors[idx][t]);
                           }
                        }
                        break;
                     }
                     errormsg += this_error;
                     // cout << QString("dbg 2: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                     error_shown[count_idx] = true;
                  }
               }
            }
         }
         else 
         {
            // find atom in residues atoms
            int atompos = -1;
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if (residue_list[respos].r_atom[m].name == this_atom->name ||
                   (
                    this_atom->name == "N" &&
                    misc_pb_rule_on &&
                    !k &&
                    residue_list[respos].r_atom[m].name == "N1" &&
                    !broken_chain_head.count(QString("%1|%2")
                                             .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                    this_atom->resName != "PRO"
                    )
                   )
               {
                  this_atom->active = true;
                  residue_list[respos].r_atom[m].tmp_flag = true;
                  atompos = (int) m;
                  break;
               }
            }
            if (atompos == -1)
            {
               residue_errors[ this_atom->orgChainID + "~" + this_atom->orgResName + " " + this_atom->orgResSeq ] = true;
               errors_found++;
               // valid residue, but non-coded atom
               switch ( control_parameters[ "pdbmissingatoms" ].toUInt() )
               {
               case 0 :
                  failure_errors++;
                  break;
               case 1 :
                  bead_exceptions[count_idx] = 2;
                  break;
               case 2 :
                  // resort to missing_residue controls
                  if ( bead_exceptions[count_idx] == 1 ) 
                  {
                     switch ( control_parameters[ "pdbmissingresidues" ].toUInt() )
                     {
                     case 0 :
                        failure_errors++;
                        break;
                     case 1 :
                        bead_exceptions[count_idx] = 2;
                        break;
                     case 2 :
                        bead_exceptions[count_idx] = 3;
                        break;
                     default :
                        failure_errors++;
                        errormsg = "Internal error: Unexpected PdbMissingResidues type " +
                           QString("%1\n").arg( control_parameters[ "pdbmissingresidues" ].toUInt() );
                        return false;
                        break;
                     }
                  } 
                  break;
               default :
                  failure_errors++;
                  errormsg = "Internal error: Unexpected PdbMissingResidues type " +
                     QString("%1\n").arg(control_parameters[ "pdbmissingresidues" ].toUInt());
                  return false;
                  break;
               }

               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                     .arg(j + 1)
                     .arg(this_atom->resName)
                     .arg(this_atom->resSeq);
                  QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Unknown atom %1.\n").arg(this_atom->name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  errormsg += this_error;
                  // cout << QString("dbg 3: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
            }
         }
      } // end for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      if (lastResPos != -1)
      {
         // check for false entries in last residue and warn about them
         // printf("check for false entries in last residue %d, residues_found %d, N1s_placed %d\n", lastResPos, residues_found, N1s_placed);
         for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
         {
            if ( !residue_list[lastResPos].r_atom[l].tmp_flag &&
                 !(misc_pb_rule_on && residues_found == 1 && N1s_placed == 1) )
            {
               residue_errors[ lastOrgChainID + "~" + 
                               lastOrgResName
                               + " " +
                               lastOrgResSeq ] = true;
               errors_found++;
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(residue_list[lastResPos].r_atom[l].name)
                  .arg(lastResSeq);
               if (control_parameters[ "pdbmissingatoms" ].toUInt() == 0)
               {
                  failure_errors++;
               } 
               else 
               {
                  if (control_parameters[ "pdbmissingatoms" ].toUInt() == 1)
                  {
                     bead_exceptions[count_idx] = 2;
                  }
                  if (control_parameters[ "pdbmissingatoms" ].toUInt() == 2)
                  {
                     if ( bead_exceptions[count_idx] == 1 ) 
                     {
                        bead_exceptions[count_idx] = 4;
                     }
                  }
               }
               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                     .arg(j + 1)
                     .arg(residue_list[lastResPos].name)
                     .arg(lastResSeq);
                  QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  errormsg += this_error;
                  // cout << QString("dbg 4: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
            }
         }
      }
   } // j

  
   if ( errormsg.length() )
   {
  
      // to remove from unknown those that have 2 beads and a correct main chain
      vector < QString > unknown_aa_residues_to_check; 
      if ( failure_errors > 0 ) 
      {
	 cout << "HUJ!" << endl;
         // cout << "check for miss atms ( failure errors ):\n";
         // cout << errormsg << endl;
         return false;
      }
      else 
      {
         residue new_residue;
         atom new_atom;
         bead new_bead;
         
         cout << "Encountered the following warnings with your PDB structure:\n" + errormsg;
         errormsg = "";
         // repair model...
         PDB_model org_model = *model;
         model->molecule.clear( );
         // we may need to redo the residues also
         model->residue.clear( );
         printf("vbar before: %g\n", model->vbar);
         QString new_residue_name = "";
         map < QString, bool > abb_msg_done;  // keep 1 message for the residue
         for (unsigned int j = 0; j < org_model.molecule.size(); j++)
         {
            PDB_chain tmp_chain;
            QString lastResSeq = "";
            QString lastResName = "";

            QString lastOrgResSeq = "";
            QString lastOrgResName = "";
            QString lastOrgChainID = "";
            
            bool auto_bb_aa = false;             // are we doing special amino acid handling?
            map < QString, int > aa_main_chain;  // to make sure we have a good main chain
            int current_bead_assignment = 0;
            bool one_bead = false;               // fall back to one bead for missing atoms
            if (last_residue_type[j] == 0 &&      
                residue_types[j].size() == 1) 
            {
               // only amino acids, so we can create two beads
               auto_bb_aa = true;
            }

            for (unsigned int k = 0; k < org_model.molecule[j].atom.size(); k++)
            {
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(org_model.molecule[j].atom[k].resName)
                  .arg(org_model.molecule[j].atom[k].resSeq);
               switch (bead_exceptions[count_idx])
               {
               case 1:
                  tmp_chain.atom.push_back(org_model.molecule[j].atom[k]);
                  if (org_model.molecule[j].atom[k].resSeq != lastResSeq) 
                  {
                     lastResSeq = org_model.molecule[j].atom[k].resSeq;
                     lastResName = org_model.molecule[j].atom[k].resName;
                     model->residue.push_back(residue_list[multi_residue_map[lastResName][0]]);

                     lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                     lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                     lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                  }
                  break;
               case 2:
               case 5: // extra or non-coded atom
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     if ( !abb_msg_done[count_idx] ) 
                     {
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Skipped\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq);
                        abb_msg_done[count_idx] = true;
                     }
                     printf("removing molecule %u atom %u from model\n", 
                            j, k);
                  }
                  break;
               case 3:
                  {
                     // create the temporary residue
                     // is this a new residue

                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString new_residue_idx = this_atom->resName;  // we could add atom_count to the idx for counting by unique atom counts...
                     if (this_atom->resSeq != lastResSeq) 
                     {

                        current_bead_assignment = 0;
                        if (auto_bb_aa) 
                        {
                           // reset the main chain counts
                           aa_main_chain.clear( );
                           aa_main_chain["N"] = 0;
                           aa_main_chain["CA"] = 0;
                           aa_main_chain["C"] = 0;
                           aa_main_chain["O"] = 0;
                        }
                        lastResSeq     = org_model.molecule[j].atom[k].resSeq;
                        lastResName    = org_model.molecule[j].atom[k].resName;
                        lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                        // this is a new unknown residue
                        // each instance of the residue gets a unique name, so we don't have
                        // to worry about duplicates and alternate duplicates
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 created.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name);
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        // create new_residue
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = 8;  // other
                        new_residue.molvol = misc_avg_volume * atom_counts[count_idx];
                        // new_residue.asa = misc.avg_asa * atom_counts[count_idx];
                        new_residue.asa = 0;
                        new_residue.vbar = misc_avg_vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );
                        new_bead.hydration = (unsigned int)(misc_avg_hydration * atom_counts[count_idx] + .5);
                        new_bead.color = 10;         // light green
                        new_bead.placing_method = 0; // cog
                        new_bead.chain = 1;          // side chain
                        new_bead.volume = misc_avg_volume * atom_counts[count_idx];
                        new_bead.mw = misc_avg_mass * atom_counts[count_idx];
                        new_residue.r_bead.push_back(new_bead);
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                        lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                     }
                     else
                     {
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if (new_residue_name != new_residue.name)
                        {
                           errormsg = "Internal error: Unexpected new residue name inconsistancy.\n" +
                              QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name);
                           return false;
                        }
                     }

                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        errormsg = "Internal error: Unexpected new residue name residue_list inconsistancy.";
                        return false;
                     }
                     // ok, now we can push back the modified atom
                     if (auto_bb_aa)
                     {
                        if (residue_list[respos].r_atom.size() < 4) 
                        {
                           aa_main_chain[this_atom->name]++;
                           if (residue_list[respos].r_atom.size() == 3) 
                           {
                              // early handling in case of no side chain
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 residue_list[respos].type = 0;
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                              }
                           }
                        } 
                        else 
                        {
                           if (residue_list[respos].r_atom.size() == 4) 
                           {
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 // ok, we have a proper backbone
                                 // so we have to redo the beads etc.
                                 abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 PB found, 2 beads created.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(new_residue_name);
                                 current_bead_assignment = 1;
                                 // redo 1st bead
                                 residue_list[respos].type = 0;

                                 residue_list[respos].r_bead[0].hydration = 
                                    (unsigned int)(misc_avg_hydration * 4 + .5);
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                                 residue_list[respos].r_bead[0].volume = misc_avg_volume * 4;
                                 residue_list[respos].r_bead[0].mw = misc_avg_mass * 4;

                                 // create a 2nd bead
                                 residue_list[respos].r_bead.push_back(residue_list[respos].r_bead[0]);
                                 residue_list[respos].r_bead[1].hydration = 
                                    (unsigned int)(misc_avg_hydration * (atom_counts[count_idx] - 4) + .5);
                                 residue_list[respos].r_bead[1].chain = 1;  // side chain
                                 residue_list[respos].r_bead[1].volume =
                                    misc_avg_volume * (atom_counts[count_idx] - 4);
                                 residue_list[respos].r_bead[1].mw = 
                                    misc_avg_mass * (atom_counts[count_idx] - 4);
                              }
                           }
                        }
                     }
            
                     new_atom.name = (this_atom->name == "OXT" ? "OXT'" : this_atom->name);
                     new_atom.hybrid.name = this_atom->name;
                     new_atom.hybrid.mw = misc_avg_mass;
                     new_atom.hybrid.radius = misc_avg_radius;
                     new_atom.bead_assignment = current_bead_assignment; 
                     new_atom.positioner = true;
                     new_atom.serial_number = residue_list[respos].r_atom.size();
                     new_atom.hydration = misc_avg_hydration;
                     residue_list[respos].r_atom.push_back(new_atom);
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                  }
                  break;
               case 4: // missing atoms
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString idx = QString("%1|%2").arg(j).arg(org_model.molecule[j].atom[k].resSeq);
                     QString new_residue_idx = this_atom->resName;  
                     int pos = molecules_residue_min_missing[idx];
                     if ( pos < 0 )
                     {
                        errormsg = QString("Internal error: idx %1 Missing atom error %2 ! < 0!\n")
                           .arg(idx)
                           .arg(pos);
                        return false;
                     }
                     cout << QString("ta->resseq %1, pos %2\n").arg(this_atom->resName).arg(pos);
                     int orgrespos = multi_residue_map[this_atom->resName][pos];
                     if (this_atom->resSeq != lastResSeq) 
                     {
                        // new residue to create with existing atoms, but increased values...
                        // possibly for a multi-bead residue
                        one_bead = false;
                        current_bead_assignment = 0;
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                        // setup how many atoms are missing from each bead
                        vector < int > beads_missing_atom_count;
                        printf("orgrespos %d\n", orgrespos);
                        beads_missing_atom_count.resize(residue_list[orgrespos].r_bead.size());

                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           beads_missing_atom_count[i] = 0;
                        }
                        // accumulate missing atom count for each bead
                        for ( unsigned int i = 0; i < molecules_residue_missing_atoms[idx][pos].size(); i++ )
                        {
                           beads_missing_atom_count[molecules_residue_missing_atoms_beads[idx][pos][i]]++;
                        }
                        // do any of these leave an empty bead?
                        // first, we need to know how many atoms are in each bead...
                        vector < int > atoms_per_bead;
                        atoms_per_bead.resize(residue_list[orgrespos].r_bead.size());
                        for ( unsigned int i = 0; i < atoms_per_bead.size(); i++ )
                        {
                           atoms_per_bead[i] = 0;
                        }
                        for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                        {
                           atoms_per_bead[residue_list[orgrespos].r_atom[i].bead_assignment]++;
                        }
                        
                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           if ( atoms_per_bead[i] - beads_missing_atom_count[i] < 1 )
                           {
                              // ugh, fall back to one bead!
                              one_bead = true;
                              printf("notice: fallback to one bead for missing atom\n");
                           }
                        }

                        vector < double > atoms_scale_weight; // multiplier for atoms weight
                        vector < double > atoms_scale_radius; // multiplier for atoms radii
                        atoms_scale_weight.resize(residue_list[orgrespos].r_bead.size());
                        atoms_scale_radius.resize(residue_list[orgrespos].r_bead.size());
                        if ( one_bead )
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           double tot_mw = 0;
                           double tot_mw_missing = 0;
                           double tot_radii3 = 0;
                           double tot_radii3_missing = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           atoms_scale_weight[0] = 
                              tot_mw / tot_mw_missing;
                           atoms_scale_radius[0] = 
                              pow(tot_radii3 / tot_radii3_missing, 1.0/3.0);
                        }
                        else
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           vector < double > tot_mw;
                           vector < double > tot_mw_missing;
                           vector < double > tot_radii3;
                           vector < double > tot_radii3_missing;
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              tot_mw.push_back(0.0);
                              tot_mw_missing.push_back(0.0);
                              tot_radii3.push_back(0.0);
                              tot_radii3_missing.push_back(0.0);
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              atoms_scale_weight[i] = 
                                 tot_mw[i] / tot_mw_missing[i];
                              atoms_scale_radius[i] = 
                                 pow(tot_radii3[i] / tot_radii3_missing[i], 1.0/3.0);
                           }
                        }

                        // create the new residue
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if ( residue_list[orgrespos].type == 0 ) // aa
                        {
                           unknown_aa_residues_to_check.push_back(new_residue_name);
                        }
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Missing atom residue copy %5 created%6.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name)
                           .arg(one_bead ? " as one bead" : "");

                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = residue_list[orgrespos].type;
                        new_residue.molvol = residue_list[orgrespos].molvol;
                        new_residue.asa = residue_list[orgrespos].asa;
                        new_residue.vbar = residue_list[orgrespos].vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );

                        // create the beads
                        if ( one_bead )
                        {
                           new_bead.volume = 0;
                           new_bead.mw = 0;
                           new_bead.hydration = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead.volume += residue_list[orgrespos].r_bead[i].volume;
                              new_bead.mw += residue_list[orgrespos].r_bead[i].mw;
                              new_bead.hydration += residue_list[orgrespos].r_bead[i].hydration;
                           }
                           // new_bead.hydration = (unsigned int)(misc_avg_hydration * atom_counts[count_idx] + .5);
                           new_bead.color = 10;         // light green
                           new_bead.placing_method = 0; // cog
                           new_bead.chain = 1;          // side chain
                           // do we have an AA with a complete PB?
                           if ( auto_bb_aa &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(0)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(1)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(2)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(3)] )
                           {
                              new_bead.chain = 0;      // main chain
                           }

                           new_residue.r_bead.push_back(new_bead);
                           // check for positioner
                           bool any_positioner = false;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner = true;
                                    break;
                                 }
                              }
                           }
                           // create the atoms
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( new_atom.name == "OXT" )
                                 {
                                    new_atom.name = "OXT'";
                                 }
                                 new_atom.hybrid.mw *= atoms_scale_weight[0]; // misc_avg_mass;
                                 new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                 new_atom.hybrid.radius *= atoms_scale_radius[0]; // misc_avg_radius;
                                 new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                 new_atom.bead_assignment = 0; 
                                 if ( !any_positioner )
                                 {
                                    new_atom.positioner = true;
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        } 
                        else
                        {
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead = residue_list[orgrespos].r_bead[i];
                              if ( beads_missing_atom_count[i] )
                              {
                                 // only set the beads with missing atoms
                                 new_bead.color = 10;
                                 new_bead.placing_method = 0;
                              }
                              new_residue.r_bead.push_back(new_bead);
                           }
                           // check for positioners
                           vector < bool > any_positioner;
                           any_positioner.resize(residue_list[orgrespos].r_bead.size());
                           for ( unsigned int i = 0; i < any_positioner.size(); i++ )
                           {
                              any_positioner[i] = false;
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner[residue_list[orgrespos].r_atom[i].bead_assignment] = true;
                                 }
                              }
                           }

                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( beads_missing_atom_count[new_atom.bead_assignment] )
                                 {
                                    new_atom.hybrid.mw *= atoms_scale_weight[new_atom.bead_assignment]; // misc_avg_mass;
                                    new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                    new_atom.hybrid.radius *= atoms_scale_radius[new_atom.bead_assignment]; // misc_avg_radius;
                                    new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                    if ( !any_positioner[new_atom.bead_assignment] )
                                    {
                                       new_atom.positioner = true;
                                    }
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        }
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
                        lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                     }
                     else
                     {
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if (new_residue_name != new_residue.name)
                        {
                           errormsg = "Internal error: Unexpected new residue name inconsistancy (a).\n" +
                              QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name);
                           return false;
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        errormsg = "Internal error: Unexpected new residue name residue_list inconsistancy (a).";
                        return false;
                     }
                     // ok, now we can push back the modified atom
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                  }
                  break;
                     
               default:
                  errormsg = "Internal error: Unhandled bead_exception code.";
                  return false;
                  break;
               } // switch
            } // atoms
            if ( tmp_chain.atom.size() )
            {
               tmp_chain.chainID = tmp_chain.atom[0].chainID;
            }
            model->molecule.push_back(tmp_chain);
            abb_msgs += "\n";
         } // molecules

         last_abb_msgs = "\n\nAutomatic Bead Builder messages:\n" + abb_msgs.replace("ABB: ","  ");
         cout << last_abb_msgs;
         
         calc_vbar(model);
         // update_vbar();
         printf("vbar after: %g\n", model->vbar);

         // remove from unknown any aa residues that have 2 beads and a side chain
         // with a N,C,CA,O MC
         for ( unsigned int i = 0; i < unknown_aa_residues_to_check.size(); i++ )
         {
            unsigned int respos = multi_residue_map[unknown_aa_residues_to_check[i]][0];
            if ( residue_list[respos].r_bead.size() == 2 &&
                 residue_list[respos].r_atom.size() > 4 ) 
            {
               map < QString, int > aa_main_chain;  // to make sure we have a good main chain
               aa_main_chain.clear( );
               aa_main_chain["N"] = 0;
               aa_main_chain["CA"] = 0;
               aa_main_chain["C"] = 0;
               aa_main_chain["O"] = 0;
               for ( unsigned int j = 0; j < residue_list[respos].r_atom.size(); j++ )
               {
                  aa_main_chain[residue_list[respos].r_atom[j].name]++;
               }
               if (aa_main_chain["N"] == 1 &&
                   aa_main_chain["CA"] == 1 &&
                   aa_main_chain["C"] == 1 &&
                   aa_main_chain["O"] == 1)
               {
                  unknown_residues.erase(unknown_aa_residues_to_check[i]);
               }
            }
         }
      }
   }
   cout << "check for miss atms:\n";
   // cout << errormsg << endl;
   return true;
}

void US_Saxs_Util::reset_chain_residues( PDB_model *model )
{
  build_molecule_maps( model );
   map < QString, bool > checked;

  

   for ( unsigned int j = 0; j < model->molecule.size(); j++ )
   {
      for (unsigned int k = 0; k < model->molecule[ j ].atom.size(); k++)
      {
	
	//cout << "Test1" << endl;
       PDB_atom *this_atom = &(model->molecule[ j ].atom[ k ]);

       QString idx = QString("%1|%2").arg( j ).arg( this_atom->resSeq );

         if ( !checked.count( idx ) )
         {
            checked[ idx ] = true;
	    
	    //cout << "Test2" << endl;
	    
            if ( this_atom->p_residue &&
                 this_atom->model_residue_pos != -1 )
            {
	      //cout << "Test3" << endl; // Problem after that with pdbs with missing atoms - does not go to both Test3a and Test3b!!!
	      
               if ( ( int ) model->residue.size() <= this_atom->model_residue_pos )
               {

		 //cout << "Test3a" << endl;
		 //cout << QString( "model->residue does not contain model residue pos %1\n" ).arg( this_atom->model_residue_pos ) << flush;
                  continue;
               }
                 
	       //qDebug (QString (" rp %1 sr %2 pr %3" ).arg(this_atom->model_residue_pos).arg(model->residue.size()).arg((long) this_atom->p_residue));
	       
	       //qDebug (QString ( "%1  %2" ).arg(model->residue[ this_atom->model_residue_pos ].unique_name).arg(this_atom->p_residue->unique_name));
	       //qDebug (QString ( "%1 ").arg(model->residue[ this_atom->model_residue_pos ].unique_name));

	       //cout << "Test3aa" << endl;

	    
               if ( model->residue[ this_atom->model_residue_pos ].unique_name != (this_atom->p_residue)->unique_name )
               {
		 cout << "Test3b" << endl;
                  // cout << QString( "found residue difference %1 %2, fixing\n" )
                  //    .arg( model->residue[ this_atom->model_residue_pos ].unique_name )
                  //    .arg( this_atom->p_residue->unique_name );
                  model->residue[ this_atom->model_residue_pos ] = *this_atom->p_residue;   
		  //qDebug (QString ( "%1  %2" ).arg(model->residue[ this_atom->model_residue_pos ].unique_name).arg(this_atom->p_residue->unique_name));
                     
               }
	       
	    }
	    //cout << "Test4" << endl;
         }
      }
   }
   calc_vbar( model );
}

// new way
class sortable_float_qs {
public:
   float             f;
   QString           index;
   bool operator < (const sortable_float_qs& objIn) const
   {
      return ( f < objIn.f );
   }
};

bool US_Saxs_Util::compute_waters_to_add_alt()
{
   puts("Transforming waters to add to pdb coordinates");

   // editor_msg( "blue", "NOTICE: only main chain CA-C-N currenly implemented" );

   steric_clash_summary.clear( );

   count_waters           = 0;
   count_waters_added     = 0;
   count_waters_not_added = 0;

   vector < point > p1;
   vector < point > p2;
   waters_to_add.clear( );
   waters_source.clear( );

   QRegExp rx_expand_mapkey("^(.+)~(.+)~(.*)$");

   // phase zero: build up list in ASA order and process in that order potentially
   // doing any type of pointmap/best fit etc.

   list < sortable_float_qs > lsf;
   sortable_float_qs          sf;
   vector < QString >         residues_in_order;

   for ( map < QString, float >::iterator it = residue_asa.begin();
         it != residue_asa.end();
         it++ )
   {
      if ( best_fit_rotamer.count( it->first ) ||
           pointmap_rotamers.count( it->first ) )
      {
         sf.f = it->second;
         sf.index = it->first;
         lsf.push_back( sf );
      }
   }

   lsf.sort();

   if ( our_saxs_options.hydration_rev_asa )
   {
      lsf.reverse();
      cout << "NOTICE: Hydration ASA sort order reversed" ;
   }

   for ( list < sortable_float_qs >::iterator it = lsf.begin();
         it != lsf.end();
         it++ )
   {
      residues_in_order.push_back( it->index );
   }
   
   // later, we could separate by bead, mc/sc or map reference set of atoms

   for ( unsigned int i = 0; i < residues_in_order.size(); i++ )
   {
      
      QString this_residue = residues_in_order[ i ];
      cout << QString( "asa sorted residues %1 asa %2 bfr %3 pm %4\n" )
         .arg( this_residue )
         .arg( residue_asa[ this_residue ] )
         .arg( best_fit_rotamer.count( this_residue ) ? "yes" : "no" )
         .arg( pointmap_rotamers.count( this_residue ) ? "yes" : "no" )
         ;

      // should be do pointmap or best fit first ?
      // probably should add up asa's and compare

      if ( best_fit_rotamer.count( this_residue ) )
      {
         rotamer best_fit = best_fit_rotamer[ this_residue ];
         cout << QString( "best fit rotamer name %1\n" ).arg( best_fit.name );

         if ( !to_hydrate.count( this_residue ) )
         {
            errormsg = QString( "Internal error: best_fit_rotamer key %1 not found in to_hydrate" )
               .arg( this_residue );
            return false;
         }
      
         if ( best_fit.water_positioning_atoms.size() !=
              best_fit.waters.size() )
         {
            errormsg = QString( "Internal error: water positioning atom size %1 does not match waters size %2" )
               .arg( best_fit.water_positioning_atoms.size() )
               .arg( best_fit.waters.size() );
            return false;
         }

         if ( rotated_rotamers.count( best_fit.name ) )
         {
            // find best choice
            cout << QString( "this best fit rotamer %1 is in with rotated rotamers %2\n" ).arg( this_residue ).arg( best_fit.name );
            // for each water
            if ( rotated_rotamers[ best_fit.name ].size() != best_fit.water_positioning_atoms.size() )
            {
               errormsg = QString( "Internal error: water positioning atom size %1 does not match best fit rotamer size %2" )
                  .arg( best_fit.water_positioning_atoms.size() )
                  .arg( best_fit.waters.size() );
               return false;
            }
               
            // for each water:
            for ( unsigned int ii = 0; ii < best_fit.water_positioning_atoms.size(); ii++ )
            {

               // find clash distance of each one, then exit for now:

               // float        min_dist        = 1e30f;
               unsigned int min_dist_pos    = 0;
               bool         found_non_clash = false;
               vector < point > min_water;

               // for each rotated rotamer

               for ( unsigned int m = 0; m < rotated_rotamers[ best_fit.name ][ ii ].size(); m++ )
               {
                  rotamer this_ref_rotamer = rotated_rotamers[ best_fit.name ][ ii ][ m ];

                  // get coordinates of 
                  p1.resize( best_fit.water_positioning_atoms[ ii ].size() );
                  p2.resize( best_fit.water_positioning_atoms[ ii ].size() );

                  if ( best_fit.water_positioning_atoms[ ii ].size() !=
                       this_ref_rotamer.water_positioning_atoms[ 0 ].size() )
                  {
                     errormsg = QString( "Internal error: best fit waters count %1 does not match reference %2" )
                        .arg( best_fit.water_positioning_atoms[ ii ].size() )
                        .arg( this_ref_rotamer.water_positioning_atoms[ 0 ].size() )
                        ;
                     return false;
                  }
            
                  for ( unsigned int jj = 0; jj < this_ref_rotamer.water_positioning_atoms[ 0 ].size(); jj++ )
                  {
                     if ( !this_ref_rotamer.atom_map.count( this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] ) )
                     {
                        errormsg = QString( "Internal error: water positioning atom %1 not found in atom_map" )
                           .arg( this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] );
                        return false;
                     }
                     // this must change to the atom map of the reference rotamer
                     p1[ jj ] = this_ref_rotamer.atom_map[ this_ref_rotamer.water_positioning_atoms[ 0 ][ jj ] ].coordinate;

                     if ( !to_hydrate[ this_residue ].count( best_fit.water_positioning_atoms[ ii ][ jj ] ) )
                     {
                        errormsg = QString( "Internal error: water positioning atom %1 not found in to_hydrate atoms" )
                           .arg( best_fit.water_positioning_atoms[ ii ][ jj ] );
                        return false;
                     }
                     p2[ jj ] = to_hydrate[ this_residue ][ best_fit.water_positioning_atoms[ ii ][ jj ] ];
                  }

                  vector < point > rotamer_waters;
                  // this is probably not correct:
                  rotamer_waters.push_back( this_ref_rotamer.waters[ 0 ].coordinate );
                  vector < point > new_waters;
                  if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
                  {
                     return false;
                  }
                  float this_dist = min_dist_to_struct_and_waters( new_waters[ 0 ] );
                  if ( this_dist > 0 )
                  {
                     // min_dist = this_dist;
                     min_dist_pos = m;
                     found_non_clash = true;
                     min_water = new_waters;
                  }
               }

               count_waters++;
               cout << QString( "Notice: best_fit rotamer %1 water %2 has %3 rotated rotamers and %4\n" )
                  .arg( best_fit.name )
                  .arg( ii )
                  .arg( rotated_rotamers[ best_fit.name ][ ii ].size() )
                  .arg( found_non_clash ? 
                        QString( "the best one is number %1 " ).arg( min_dist_pos ) :
                        QString( "there was a clash at all tries" ) )
                  ;

               if ( found_non_clash && !has_steric_clash( min_water[ 0 ] ) )
               {
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( min_water[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "RtmrMC:%1" ).arg( best_fit.name ) );
                  cout << QString( "p2 adding to waters source %1 %2\n" ).arg( this_residue ).arg( best_fit.name );
               } else {
                  count_waters_not_added++;
               }
            }
         } else {
            // std add one
            cout << QString( "this best fit rotamer %1 is NOT a rotated rotamers %2\n" ).arg( this_residue ).arg( best_fit.name );

            // for each water to add:
            // cout << QString( "need to compute best transform matrix for %1:\n" ).arg( it->first );

            //             if ( best_fit.water_positioning_atoms.size() !=
            //                  best_fit.waters.size() )
            //             {
            //                errormsg = QString( "Internal error: water positioning atom size %1 does not match waters size %2" )
            //                   .arg( best_fit.water_positioning_atoms.size() )
            //                   .arg( best_fit.waters.size() );
            //                return false;
            //             }

            for ( unsigned int i = 0; i < best_fit.water_positioning_atoms.size(); i++ )
            {
               // get coordinates of 
               p1.resize( best_fit.water_positioning_atoms[ i ].size() );
               p2.resize( best_fit.water_positioning_atoms[ i ].size() );
            
               for ( unsigned int j = 0; j < best_fit.water_positioning_atoms[ i ].size(); j++ )
               {
                  if ( !best_fit.atom_map.count( best_fit.water_positioning_atoms[ i ][ j ] ) )
                  {
                     errormsg = QString( "Internal error: water positioning atom %1 not found in atom_map" )
                        .arg( best_fit.water_positioning_atoms[ i ][ j ] );
                     return false;
                  }
                  p1[ j ] = best_fit.atom_map[ best_fit.water_positioning_atoms[ i ][ j ] ].coordinate;

                  if ( !to_hydrate[ this_residue ].count( best_fit.water_positioning_atoms[ i ][ j ] ) )
                  {
                     errormsg = QString( "Internal error: water positioning atom %1 not found in to_hydrate atoms" )
                        .arg( best_fit.water_positioning_atoms[ i ][ j ] );
                     return false;
                  }
                  p2[ j ] = to_hydrate[ this_residue ][ best_fit.water_positioning_atoms[ i ][ j ] ];

                  // cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                  // .arg( best_fit.water_positioning_atoms[ i ][ j ] )
                  // .arg( p1[ j ].axis[ 0 ] )
                  // .arg( p1[ j ].axis[ 1 ] )
                  // .arg( p1[ j ].axis[ 2 ] )
                  // .arg( p2[ j ].axis[ 0 ] )
                  // .arg( p2[ j ].axis[ 1 ] )
                  // .arg( p2[ j ].axis[ 2 ] );
               }
               vector < point > rotamer_waters;
               rotamer_waters.push_back( best_fit.waters[ i ].coordinate );
               vector < point > new_waters;
               // cout << QString( " and apply it to the point [%1,%2,%3]\n")
               // .arg( best_fit.waters[ i ].coordinate.axis[ 0 ] )
               // .arg( best_fit.waters[ i ].coordinate.axis[ 1 ] )
               // .arg( best_fit.waters[ i ].coordinate.axis[ 2 ] );
               if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
               {
                  return false;
               }
               count_waters++;
               if ( !has_steric_clash( new_waters[ 0 ] ) )
               {
                  cout << QString( "Notice: best fit for %1 has no rotated rotamers and no clash\n" )
                     .arg( best_fit.name );
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( new_waters[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "Rtmr:%1" ).arg( best_fit.name ) );
                  cout << QString( "p3 adding to waters source %1 %2\n" ).arg( this_residue ).arg( best_fit.name );
               } else {
                  cout << QString( "Notice: best fit for %1 has no rotated rotamers and clashes\n" )
                     .arg( best_fit.name );
                  count_waters_not_added++;
               }
            }
         } // end of std non-multiple best_fit_rotamer
      } // end if best_fit_rotamer
      // #define DEBUG_POINTMAP
      if ( pointmap_rotamers.count( this_residue ) )
      {
         if ( !to_hydrate_pointmaps.count( this_residue ) )
         {
            errormsg = QString( "Internal error: pointmap_rotamers key %1 not found in to_hydrate_pointmaps" )
               .arg( this_residue );
            return false;
         }

         // add waters for each pointmap for this residue
         if ( rx_expand_mapkey.indexIn( this_residue ) == -1 )
         {
            errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( this_residue );
            return false;
         }
         QString resName = rx_expand_mapkey.cap( 1 );
         if ( !pointmap_atoms.count( resName ) ||
              !pointmap_atoms_dest.count( resName ) ||
              !pointmap_atoms_ref_residue.count( resName ) )
         {
            errormsg = QString( "Internal error: could not find pointmap entries for residue of key %1" ).arg( this_residue );
            return false;
         }
         
         if ( pointmap_rotamers[ this_residue ].size() != pointmap_atoms[ resName ].size() ||
              pointmap_rotamers[ this_residue ].size() != pointmap_atoms_dest[ resName ].size() ||
              pointmap_rotamers[ this_residue ].size() != pointmap_atoms_ref_residue[ resName ].size() )
         {
            errormsg = QString( "Internal error: could not find pointmap size inconsistancy for key %1" ).arg( this_residue );
            return false;
         }
         
         // this is for each water
         for ( unsigned int i = 0; i < pointmap_rotamers[ this_residue ].size(); i++ )
         {
            // for each water to add:
            // compute transformation matrix from the pointmap_atoms_dest in the pointmap_atom_ref_residue
            // to the pointmap_atoms in the residue and apply to each water in the rotamer
            // the rotamer's atom_map(equivalently side chain) & waters have the "dest" info
         
            rotamer ref_rotamer = pointmap_rotamers[ this_residue ][ i ];
#if defined(DEBUG_POINTMAP)
            cout << QString( "Note: in compute_waters_to_add_alt(), current name '%1' has alternates %2\n" )
               .arg( ref_rotamer.name )
               .arg( rotated_rotamers.count( ref_rotamer.name ) ? "yes" : "no" );
#endif

            if ( rotated_rotamers.count( ref_rotamer.name ) )
            {
               // multiple rotated rotamers for pointmap

               if ( rx_expand_mapkey.indexIn( this_residue ) == -1 )
               {
                  errormsg = QString( "Internal error: could not expand mapkey %1" ).arg( this_residue );
                  return false;
               }

               QString resName = rx_expand_mapkey.cap( 1 );

               if ( !pointmap_atoms.count( resName ) ||
                    !pointmap_atoms_dest.count( resName ) ||
                    !pointmap_atoms_ref_residue.count( resName ) )
               {
                  errormsg = QString( "Internal error: could not find pointmap entries for residue of key %1" ).arg( resName );
                  return false;
               }
         
               // float        min_dist        = 1e30f;
               unsigned int min_dist_pos    = 0;
               bool         found_non_clash = false;
               vector < point > min_water;

               // for each rotated rotamer:

               for ( unsigned int k = 0; k < rotated_rotamers[ ref_rotamer.name ][ i ].size(); k++ )
               {
                  rotamer this_ref_rotamer = rotated_rotamers[ ref_rotamer.name ][ i ][ k ];

                  if ( this_ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
                  {
                     errormsg = QString( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4) " )
                        .arg( this_residue )
                        .arg( i )
                        .arg( this_ref_rotamer.residue )
                        .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
                     return false;
                  }
            
                  // build up the transformation
                  
                  p1.resize( pointmap_atoms     [ resName ][ i ].size() );
                  p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
                  
                  if ( p1.size() != p2.size() )
                  {
                     errormsg = QString( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" )
                        .arg( this_residue )
                        .arg( i )
                        .arg( pointmap_atoms[ resName ][ i ].size() )
                        .arg( pointmap_atoms_dest[ resName ][ i ].size() );
                     return false;
                  }
            
                  for ( unsigned int j = 0; j < p1.size(); j++ )
                  {
                     // cout << QString("mapping: %1 %2 dest %3\n")
                     // .arg(this_residue)
                     // .arg(i)
                     // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
                     if ( !this_ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
                     {
                        errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                           .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                        return false;
                     }
               
                     p1[ j ] = this_ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
               
                     if ( !to_hydrate_pointmaps[ this_residue ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
                     {
                        errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                           .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                        return false;
                     }
#if defined(DEBUG_POINTMAP)
                     cout << QString("mapping: %1 %2 source %3\n")
                        .arg(this_residue)
                        .arg(i)
                        .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
                     p2[ j ] = to_hydrate_pointmaps[ this_residue ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
                     cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                        .arg( this_residue )
                        .arg( p1[ j ].axis[ 0 ] )
                        .arg( p1[ j ].axis[ 1 ] )
                        .arg( p1[ j ].axis[ 2 ] )
                        .arg( p2[ j ].axis[ 0 ] )
                        .arg( p2[ j ].axis[ 1 ] )
                        .arg( p2[ j ].axis[ 2 ] );
#endif
                  }

                  // expect only one at this time
                  if ( rotated_rotamers[ this_ref_rotamer.name ].size() != 1 )
                  {
                     errormsg = QString( "Error: multiple rotated pointmap rotamer %1 has %2 waters and only one water is currently supported for this scheme " )
                        .arg( this_ref_rotamer.name ).arg( rotated_rotamers[ this_ref_rotamer.name ].size() );
                     return false;
                  }
               

                  for ( unsigned int m = 0; m < rotated_rotamers[ this_ref_rotamer.name ][ 0 ].size(); m++ )
                  {
                     vector < point > rotamer_waters;
                     // this is probably not correct:
                     rotamer_waters.push_back( rotated_rotamers[ this_ref_rotamer.name ][ 0 ][ m ].waters[ 0 ].coordinate );
                     vector < point > new_waters;
                     if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
                     {
                        return false;
                     }
                     float this_dist = min_dist_to_struct_and_waters( new_waters[ 0 ] );
                     if ( this_dist > 0 )
                     {
                        // min_dist = this_dist;
                        min_dist_pos = m;
                        found_non_clash = true;
                        min_water = new_waters;
                     }
                  }
               }

               count_waters++;

               cout << QString( "Notice: pointmap for %1 has %2 rotated rotamers and %3\n" )
                  .arg( ref_rotamer.name )
                  .arg( rotated_rotamers[ ref_rotamer.name ][ 0 ].size() )
                  .arg( found_non_clash ? 
                        QString( "the best one is number %1 " ).arg( min_dist_pos ) :
                        QString( "there was a clash at all tries" ) )
                  ;

               if ( found_non_clash && !has_steric_clash( min_water[ 0 ] ) )
               {
                  count_waters_added++;
                  waters_to_add[ this_residue ].push_back( min_water[ 0 ] );
                  waters_source[ this_residue ].push_back( QString( "PMmc:%1" ).arg( ref_rotamer.name ) );
                  cout << QString( "p4 adding to waters source %1 %2\n" ).arg( this_residue ).arg( ref_rotamer.name );
               } else {
                  count_waters_not_added++;
               }

            } else {
               // no multiple rotated rotamers

               if ( ref_rotamer.residue != pointmap_atoms_ref_residue[ resName ][ i ] )
               {
                  errormsg = QString( "Internal error: inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" )
                     .arg( this_residue )
                     .arg( i )
                     .arg( ref_rotamer.residue )
                     .arg( pointmap_atoms_ref_residue[ resName ][ i ] );
                  return false;
               }
            
               // build up the transformation
            
               p1.resize( pointmap_atoms     [ resName ][ i ].size() );
               p2.resize( pointmap_atoms_dest[ resName ][ i ].size() );
            
               if ( p1.size() != p2.size() )
               {
                  errormsg = QString( "Internal error: size inconsistancy for reference residue name for key %1 pos %2 (%3 != %4)" )
                     .arg( this_residue )
                     .arg( i )
                     .arg( pointmap_atoms[ resName ][ i ].size() )
                     .arg( pointmap_atoms_dest[ resName ][ i ].size() );
                  return false;
               }
            
               for ( unsigned int j = 0; j < p1.size(); j++ )
               {
                  // cout << QString("mapping: %1 %2 dest %3\n")
                  // .arg(this_residue)
                  // .arg(i)
                  // .arg(pointmap_atoms_dest[ resName ][ i ][ j ] );
                  if ( !ref_rotamer.atom_map.count( pointmap_atoms_dest[ resName ][ i ][ j ] ) )
                  {
                     errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                        .arg(  pointmap_atoms_dest[ resName ][ i ][ j ]  );
                     return false;
                  }
               
                  p1[ j ] = ref_rotamer.atom_map[ pointmap_atoms_dest[ resName ][ i ][ j ] ].coordinate;
               
                  if ( !to_hydrate_pointmaps[ this_residue ].count( pointmap_atoms[ resName ][ i ][ j ] ) )
                  {
                     errormsg = QString( "Internal error: atom %1 not found in to_hydrate_pointmaps atoms" )
                        .arg(  pointmap_atoms[ resName ][ i ][ j ] );
                     return false;
                  }
#if defined(DEBUG_POINTMAP)
                  cout << QString("mapping: %1 %2 source %3\n")
                     .arg(this_residue)
                     .arg(i)
                     .arg(pointmap_atoms[ resName ][ i ][ j ] );
#endif
                  p2[ j ] = to_hydrate_pointmaps[ this_residue ][ pointmap_atoms[ resName ][ i ][ j ] ];
               
#if defined(DEBUG_POINTMAP)
                  cout << QString( "  %1 [%2,%3,%4] to [%5,%6,%7]\n" )
                     .arg( this_residue )
                     .arg( p1[ j ].axis[ 0 ] )
                     .arg( p1[ j ].axis[ 1 ] )
                     .arg( p1[ j ].axis[ 2 ] )
                     .arg( p2[ j ].axis[ 0 ] )
                     .arg( p2[ j ].axis[ 1 ] )
                     .arg( p2[ j ].axis[ 2 ] );
#endif
               }
            
               // now we have p1 & p2, apply to all of the waters
#if defined(DEBUG_POINTMAP)
               cout << QString("adding %1 waters for %2 %3\n")
                  .arg( ref_rotamer.waters.size() )
                  .arg( this_residue )
                  .arg( resName );
#endif
            
               for ( unsigned int j = 0; j < ref_rotamer.waters.size(); j++ )
               {
                  vector < point > rotamer_waters;
                  rotamer_waters.push_back( ref_rotamer.waters[ j ].coordinate );
                  vector < point > new_waters;
                  if ( !atom_align( p1, p2, rotamer_waters, new_waters ) )
                  {
                     return false;
                  }
                  count_waters++;

                  if ( !has_steric_clash( new_waters[ 0 ] ) )
                  {
                     cout << QString( "Notice: pointmap for %1 has no rotated rotamers and no clash\n" )
                        .arg( ref_rotamer.name );
                     count_waters_added++;
                     waters_to_add[ this_residue ].push_back( new_waters[ 0 ] );
                     waters_source[ this_residue ].push_back( QString( "PM:%1" ).arg( ref_rotamer.name ) );
                     cout << QString( "p5 adding to waters source %1 %2\n" ).arg( this_residue ).arg( ref_rotamer.name );
                  } else {
                     cout << QString( "Notice: pointmap for %1 has no rotated rotamers and has a clash\n" )
                        .arg( ref_rotamer.name );
                     count_waters_not_added++;
                  }
               }
            } // end of else no rotated rotamers
         } // end of search over pointmap_rotamers[ this_residue ]
      } // end if pointmap_rotamers
   }

   cout << "Done transforming waters to add to pdb coordinates\n";
   cout << 
      QString("%1 waters added. %2 not added due to steric clashes \n")
      .arg( count_waters_added ) 
      .arg( count_waters_not_added )
      ;
   // list_steric_clash();
   if ( !list_steric_clash_recheck() )
   {
      return false;
   }
   puts("Done transforming waters to add to pdb coordinates");
   return true;
}

bool US_Saxs_Util::has_steric_clash( point p )
{
   unsigned int i = current_model;

   double dist_threshold = 1e0 - ( our_saxs_options.steric_clash_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

   // check structure:
   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
         {
            return true;
         }
      }
   }

   // check already added waters:
   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         if ( dist( it->second[ i ], p ) <=  2e0 * water_radius * dist_threshold )
         {
            return true;
         }
      }
   }
   return false;
}

bool US_Saxs_Util::list_steric_clash_recheck()
{
   puts( "list_steric_clash_recheck()" );
   QString pdb_file = control_parameters[ "inputfile" ];

   pdb_file = pdb_file.replace( QRegExp( "^../common/" ), "" );

   unsigned int i = current_model;
   double dist_threshold = 1e0 - ( our_saxs_options.steric_clash_recheck_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;

   map < QString, unsigned int > steric_clash_recheck_summary;
   hydrate_clash_log.clear( );
   QString hydrate_clash_detail;
   // cout << "steric clash recheck:\n";

   hydrate_clash_detail +=
      "Water,clash,radius water,radius clash,sum radii,distance,overlap,percent overlap,water x,clash x, water y, clash y, water z, clash z, source, source2\n";

   hydrate_clash_map_structure .clear( );
   hydrate_clash_map_rtmr_water.clear( );
   hydrate_clash_map_pm_water  .clear( );

   map < QString, bool > counted;

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      // cout << QString( "checking steric clash water to add %1\n" ).arg( it->first );
      for ( unsigned int pos = 0; pos < it->second.size(); pos++ )
      {
         point p = it->second[ pos ];

         // check structure:
         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if ( dist( this_atom->coordinate, p ) < ( this_atom->radius + water_radius ) * dist_threshold )
               {
                  hydrate_clash_map_structure[ 
                                              QString( "%1%2:%3.%4" )
                                              .arg( this_atom->resName )
                                              .arg( this_atom->resSeq )
                                              .arg( this_atom->chainID )
                                              .arg( this_atom->name )
                  ] = true;
                  
                  if ( !waters_source.count( it->first ) )
                  {
                     cout << QString( "waters_source does not contain %1\n" ).arg( it->first );
                     cout << QString( "current waters source map:\n" );
                     for ( map < QString, vector < QString > >::iterator it2 =  waters_source.begin();
                           it2 != waters_source.end();
                           it2++ )
                     {
                        cout << QString( "key: %1 size %2\n" ).arg( it2->first ).arg( it2->second.size() );
                     }
                     errormsg = QString( "Internal error: list_steric_clash_recheck(): waters_source does not contain %1" ).arg( it->first );
                     return false;
                  }
                  if ( ( unsigned int ) waters_source[ it->first ].size() <= pos )
                  {
                     cout << QString( "waters_source[ %1 ] does not contain pos %2 size only %3\n" ).arg( it->first ).arg( pos ).arg( waters_source[ it->first ].size() );
                     errormsg = QString( "Internal error: list_steric_clash_recheck(): waters_source[ %1 ] does not contain pos %2" ).arg( it->first ).arg( pos );
                     return false;
                  }
                  if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  }                     

                  hydrate_clash_log << 
                     QString( "Water %1 number %2 clashes with %3~%4~%5 atom %6 by %7 A or %8%\n" )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( this_atom->resName )
                     .arg( this_atom->resSeq )
                     .arg( this_atom->chainID )
                     .arg( this_atom->name )
                     .arg( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) )
                     .arg( 100.0 * ( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) ) 
                           / ( this_atom->radius + water_radius ) )
                     ;
                        
                  hydrate_clash_detail +=
                     QString( "Water:%1~%2,%3~%4~%5~%6," )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( this_atom->resName )
                     .arg( this_atom->resSeq )
                     .arg( this_atom->chainID )
                     .arg( this_atom->name )
                         ;
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4," )
                     .arg( water_radius )
                     .arg( this_atom->radius )
                     .arg( this_atom->radius + water_radius )
                     .arg( dist( this_atom->coordinate, p ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "%1,%2," )
                     .arg( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) )
                     .arg( 100.0 * ( ( this_atom->radius + water_radius ) - dist( this_atom->coordinate, p ) ) 
                           / ( this_atom->radius + water_radius ) )
                     ;
                     
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4,%5,%6," )
                     .arg( p.axis[ 0 ] )
                     .arg( this_atom->coordinate.axis[ 0 ] )
                     .arg( p.axis[ 1 ] )
                     .arg( this_atom->coordinate.axis[ 1 ] )
                     .arg( p.axis[ 2 ] )
                     .arg( this_atom->coordinate.axis[ 2 ] )
                     ;

                  hydrate_clash_detail += 
                     QString( "%1" )
                     .arg( waters_source[ it->first ][ pos ] );

                  hydrate_clash_detail += "\n";
                        
                  if ( this_atom->chain == 1 )
                  {
                     steric_clash_recheck_summary[ " structure side chain" ]++;
                     if ( !counted.count( QString( "%1~%2" ).arg( it->first ).arg( pos ) ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes total" ]++;
                        if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                        {
                           steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                        } else {
                           steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                        }
                        counted[ QString( "%1~%2" ).arg( it->first ).arg( pos ) ] = true;
                     }
                  } else {
                     steric_clash_recheck_summary[ " structure main chain" ]++;
                  }
                  steric_clash_recheck_summary[ " structure total" ]++;
                  steric_clash_recheck_summary[ "structure or other water" ]++;
               }
            }
         }

         // check other added waters:
         // map < QString, vector < point > >::iterator it2 = waters_to_add.begin();
         // while ( *it2 != *it &&
         // it2 != waters_to_add.end() )
         // {
         // it2++;
         // }
         for ( map < QString, vector < point > >::iterator it2 = it;
               it2 != waters_to_add.end();
               it2++ )
         {
            for ( unsigned int j = ( it2 == it ? pos + 1 : 0 ); j < it2->second.size(); j++ )
            {
               if ( dist( it2->second[ j ], p ) < 2e0 * water_radius * dist_threshold )
               {
                  if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it->first )
                                                  .arg( pos ) 
                     ] = true;
                  }                     


                  if ( waters_source[ it2->first ][ j ].startsWith( "PM" ) )
                  {
                     hydrate_clash_map_pm_water[ 
                                                QString( "%1~%2" )
                                                .arg( it2->first )
                                                .arg( j ) 
                     ] = true;
                  } else {
                     hydrate_clash_map_rtmr_water[ 
                                                  QString( "%1~%2" )
                                                  .arg( it2->first )
                                                  .arg( j ) 
                     ] = true;
                  }                     

                  hydrate_clash_log << 
                     QString( "Water %1 number %2 clashes with water from %3 number %4 by %5 A or %6%\n" )
                     .arg( it->first )
                     .arg( pos + 1 )
                     .arg( it2->first )
                     .arg( j + 1 )
                     .arg( 2e0 * water_radius -  dist( it2->second[ j ], p ) )
                     .arg( 100.0 * ( 2e0 * water_radius - dist( it2->second[ j ], p ) )
                           / ( 2e0 * water_radius ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "Water:%1~%2,Water:%3~%4," )
                     .arg( it->first )
                     .arg( i + 1 )
                     .arg( it2->first )
                     .arg( j + 1 )
                         ;

                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4," )
                     .arg( water_radius )
                     .arg( water_radius )
                     .arg( 2e0 * water_radius )
                     .arg( dist( it2->second[ j ], p ) )
                     ;

                  hydrate_clash_detail +=
                     QString( "%1,%2," )
                     .arg( 2e0 * water_radius -  dist( it2->second[ j ], p ) )
                     .arg( 100.0 * ( 2e0 * water_radius - dist( it2->second[ j ], p ) )
                           / ( 2e0 * water_radius ) )
                     ;
                     
                  hydrate_clash_detail +=
                     QString( "%1,%2,%3,%4,%5,%6," )
                     .arg( p.axis[ 0 ] )
                     .arg( it2->second[ j ].axis[ 0 ] )
                     .arg( p.axis[ 1 ] )
                     .arg( it2->second[ j ].axis[ 1 ] )
                     .arg( p.axis[ 2 ] )
                     .arg( it2->second[ j ].axis[ 2 ] )
                     ;

                  hydrate_clash_detail += 
                     QString( "%1,%2" )
                     .arg( waters_source[ it ->first ][ pos ] )
                     .arg( waters_source[ it2->first ][ j   ] );

                  hydrate_clash_detail += "\n";

                  cout << QString( "Water %1 clashes with water from %2\n" )
                     .arg( it->first )
                     .arg( it2->first );
                  steric_clash_recheck_summary[ " other water" ]++;
                  steric_clash_recheck_summary[ "structure or other water" ]++;
                  if ( !counted.count( QString( "%1~%2" ).arg( it->first ).arg( pos ) ) )
                  {
                     steric_clash_recheck_summary[ "waters with clashes total" ]++;
                     if ( waters_source[ it->first ][ pos ].startsWith( "PM" ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                     } else {
                        steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                     }
                     counted[ QString( "%1~%2" ).arg( it->first ).arg( pos ) ] = true;
                  }
                  if ( !counted.count( QString( "%1~%2" ).arg( it2->first ).arg( j ) ) )
                  {
                     steric_clash_recheck_summary[ "waters with clashes total" ]++;
                     if ( waters_source[ it2->first ][ j ].startsWith( "PM" ) )
                     {
                        steric_clash_recheck_summary[ "waters with clashes (Pointmap)" ]++;
                     } else {
                        steric_clash_recheck_summary[ "waters with clashes (Rotamer)" ]++;
                     }
                     counted[ QString( "%1~%2" ).arg( it2->first ).arg( j ) ] = true;
                  }
               }
            }
         }
      }
   }

   QString qs = QString( "Steric clash recheck tolerance %1%\n" ).arg( our_saxs_options.steric_clash_recheck_distance );
   for ( map < QString, unsigned int >::iterator it = steric_clash_recheck_summary.begin();
         it != steric_clash_recheck_summary.end();
         it++ )
   {
      qs += QString( "Steric recheck clash %1 : %2\n" ).arg( it->first ).arg( it->second );
   }

   //    editor_msg( "blue",
   //                QString( "Maximum possible water to place %1, degree of hydration %2\n" )
   //                .arg( hydrate_max_waters_no_asa )
   //                .arg( QString( "").sprintf( "%.2f", ( double ) hydrate_max_waters_no_asa * 18e0 / model_vector[ current_model ].mw ) )
   //                +
   //                QString( "Maximum exposed possible water to place %1, degree of hydration %2\n" )
   //                .arg( count_waters_added + count_waters_not_added )
   //                .arg( QString( "" ).sprintf( "%.2f", ( double ) ( count_waters_added + count_waters_not_added )  * 18e0 / model_vector[ current_model ].mw ) )
   //                + 
   //                QString( "Waters placed %1, degree of hydration %2\n" )
   //                .arg( count_waters_added )
   //                .arg( QString( "" ).sprintf( "%.2f", ( double ) count_waters_added * 18e0 / model_vector[ current_model ].mw ) )
   //                );

   last_steric_clash_log.clear( );
   last_steric_clash_log <<
      QString( "REMARK MW %1 Daltons\n" ).arg( model_vector[ current_model ].mw );

   last_steric_clash_log <<
      QString( "REMARK Maximum possible water to place         %1   degree of hydration %2\n" )
      .arg( hydrate_max_waters_no_asa )
      .arg( QString( "").sprintf( "%.2f", ( double ) hydrate_max_waters_no_asa * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log <<
      QString( "REMARK Maximum exposed possible water to place %1   degree of hydration %2\n" )
      .arg( count_waters_added + count_waters_not_added )
      .arg( QString( "" ).sprintf( "%.2f", ( double ) ( count_waters_added + count_waters_not_added )  * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log <<
      QString( "REMARK Waters placed                           %1   degree of hydration %2\n" )
      .arg( count_waters_added )
      .arg( QString( "" ).sprintf( "%.2f", ( double ) count_waters_added * 18e0 / model_vector[ current_model ].mw ) );

   last_steric_clash_log << 
         QString(
                 "REMARK Hydration of                  %1\n"
                 "REMARK Hydration file                %2\n"
                 "REMARK Steric clash tolerance        %3%\n"
                 )
         .arg( pdb_file )
         .arg( control_parameters[ "hydrationfile" ] )
         .arg( our_saxs_options.steric_clash_distance )
         ;
   {
      QString qs2 = "REMARK " + qs;
      qs2.replace( "\n", "\nREMARK " );
      last_steric_clash_log << qs2;
   }

   // cout << "Steric clash log detail:\n";
   //  cout << hydrate_clash_log.join( "" ) << endl;
   QString fname = 
      QString( "%1_%2_clash.txt" )
      .arg( QFileInfo( pdb_file ).baseName() )
      .arg( model_vector[ current_model ].model_id )
      ;
   fname.replace( "//", "/" );
   QFile f(fname);
   if ( f.open( QIODevice::WriteOnly ) )
   {
      QTextStream ts( &f );
      ts << 
         QString(
                 "Hydration of                  %1\n"
                 "Hydration file                %2\n"
                 "Steric clash tolerance        %3%\n"
                 )
         .arg( pdb_file )
         .arg( our_saxs_options.default_rotamer_filename )
         .arg( our_saxs_options.steric_clash_distance )
         ;              
      ts << qs;
      ts << "\nDetailed clash info:\n\n";
      for ( unsigned int i = 0; i < (unsigned int) hydrate_clash_log.size(); i++ )
      {
         ts << hydrate_clash_log[ i ];
      }
      ts << "\nEnd clash info\n";
      qs += QString( "Steric clash report in: %1\n" ).arg( fname );
      f.close();
      output_files << fname;
      //      if ( !saxs_options.alt_hydration )
      //      {
      //       if ( !quiet )
      //       {
      //          view_file( fname );
      //       }
      //      }
   } else {
      errormsg = QString( "Error: could not create %1" ).arg( fname );
      return false;
   }
   
   {
      QString fname = 
         QString( "%1_%2_clash.csv" )
         .arg( QFileInfo( pdb_file ).baseName() )
         .arg( model_vector[ current_model ].model_id )
         ;

      fname.replace( "//", "/" );
      QFile f(fname);
      if ( f.open( QIODevice::WriteOnly ) )
      {
         QTextStream ts( &f );
         ts << hydrate_clash_detail;
         f.close();
         output_files << fname;
         qs += QString( "Steric clash report csv in: %1\n" ).arg( fname );
      } else {
         errormsg = QString( "Error: could not create %1" ).arg( fname );
         return false;
      }
   }

   hydrate_clash_log.clear( );
   return true;
}

float US_Saxs_Util::min_dist_to_struct_and_waters( point p )
{
   unsigned int i = current_model;
   double dist_threshold = 1e0 - ( our_saxs_options.steric_clash_distance / 100e0 );
   double water_radius   = multi_residue_map.count( "WAT" ) ?
      residue_list[ multi_residue_map[ "WAT" ][ 0 ] ].r_atom[ 0 ].hybrid.radius : 1.401;
   
   float min_dist = 1e30f;
   float this_dist;

   // check structure:
   for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
      for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
         
         this_dist = dist( this_atom->coordinate, p ) - ( this_atom->radius + water_radius ) * dist_threshold;
         if ( this_dist < 0e0 )
         {
            return this_dist;
         }
         if ( min_dist > this_dist )
         {
            min_dist = this_dist;
         }
      }
   }

   for ( map < QString, vector < point > >::iterator it = waters_to_add.begin();
         it != waters_to_add.end();
         it++ )
   {
      for ( unsigned int i = 0; i < it->second.size(); i++ )
      {
         this_dist = dist( it->second[ i ], p ) -  2e0 * water_radius * dist_threshold;
         if ( this_dist < 0e0 )
         {
            return -1.0;
         }
         if ( min_dist > this_dist )
         {
            min_dist = this_dist;
         }
      }
   }
   return min_dist;
}

