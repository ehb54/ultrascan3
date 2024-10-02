#include "../include/us_saxs_util.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

bool US_Saxs_Util::compute_1d()
{
   errormsg = "";
   noticemsg = "";

   QString missing_required;

   puts( "compute 1d" );
   QStringList qsl_required;

   {
      qsl_required << "residuefile";
      qsl_required << "1dlambda";
      qsl_required << "1ddetectordistance";
      qsl_required << "1ddetectorwidth";
      qsl_required << "1ddetectorpixelswidth";
      qsl_required << "1drho0";
      qsl_required << "1ddeltar";
      qsl_required << "1dproberadius";
      qsl_required << "1dthreshold";
      qsl_required << "1dsamplerotations";
      qsl_required << "1daxisrotations";
      qsl_required << "1drotationfile";
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
      errormsg = QString( "Error: Process1d requires prior definition of:%1" )
         .arg( missing_required );
      return false;
   }

   detector_pixels_width         = control_parameters[ "1ddetectorpixelswidth" ].toInt();
   detector_distance             = control_parameters[ "1ddetectordistance"    ].toDouble();
   detector_width                = control_parameters[ "1ddetectorwidth"       ].toDouble() * 1e-3;
   lambda                        = control_parameters[ "1dlambda"              ].toDouble();
   rho0                          = control_parameters[ "1drho0"                ].toDouble();
   deltaR                        = control_parameters[ "1ddeltar"              ].toDouble();
   probe_radius                  = control_parameters[ "1dproberadius"         ].toDouble();
   threshold                     = control_parameters[ "1dthreshold"           ].toDouble();
   sample_rotations              = control_parameters[ "1dsamplerotations"     ].toUInt();
   int axis_rotations            = control_parameters[ "1daxisrotations"       ].toUInt();

   detector_width_per_pixel      = detector_width  / detector_pixels_width;

   map < unsigned int, bool > intermediate_saves;
   if ( control_parameters.count( "1dintermediatesaves" ) )
   {
      QString     qs  = control_parameters[ "1dintermediatesaves" ];
      qs.replace( ",", " " );
      qs.trimmed();
      QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl.size(); i++ )
      {
         unsigned int pos = qsl[ i ].toUInt();
         if ( !pos || pos >= sample_rotations )
         {
            errormsg =
               QString( "1drun: 1dintermediatesaves bad value %1 found at pos %2 in line <%3>\n"
                        "either not a positive natural number or value larger than total sample rotations %4" )
               .arg( pos )
               .arg( i + 1 )
               .arg( qsl[ i ] )
               .arg( sample_rotations )
               ;
            return false;
         }
         intermediate_saves[ pos ] = true;
      }
   }

   double spec_multiplier = 1e0;
   if ( control_parameters.count( "1dspecmult" ) )
   {
      spec_multiplier = control_parameters[ "1dspecmult" ].toDouble();
   }

   QString qs_total_plots = QString( "%1" ).arg( sample_rotations * axis_rotations );

   // compute complex curves, display modulus on 1d array
   // compute for each point on detector
   plot_count = 0;

   bool is_ok = true;

   if ( detector_pixels_width <= 0 )
   {
      errormsg += "Detector pixel count must be positive\n";
      is_ok = false;
   }

   if ( lambda <= 0e0 )
   {
      errormsg +=  "The wavelength must be positive\n";
      is_ok = false;
   }
      
   if ( detector_width <= 0 )
   {
      errormsg +=  "Detector width must be positive\n";
      is_ok = false;
   }

   if ( detector_distance <= 0 )
   {
      errormsg +=  "Detector distance must be positive\n";
      is_ok = false;
   }

   if ( !is_ok )
   {
      return false;
   }

   //    if ( !setup_excluded_volume_map() )
   //    {
   //       errormsg +=  errormsg;
   //       return;
   //    }

   vector < vector < double > > rotations;

   if ( !load_rotations( sample_rotations, rotations ) )
   {
      return false;
   }

   // setup atoms
   QRegExp count_hydrogens("H(\\d)");

   if ( our_saxs_options.iqq_use_atomic_ff )
   {
      cout << "using explicit hydrogens\n";
   }

   vector < saxs_atom > atoms;

   double atomic_scaler = 1e0; // pdbs and Q in Angstrom units
   double atomic_scaler_inv = 1e0; // / atomic_scaler;


   unsigned int current_model = 0;

   QString name = 
      QString( "%1_tp%2_rot%3_ar%4_d3R%5_rho0%6_pr%7_t%8" )
      .arg( model_vector[ current_model ].model_id )
      .arg( sample_rotations * axis_rotations ) 
      .arg( sample_rotations ) 
      .arg( axis_rotations )
      .arg( deltaR ) 
      .arg( rho0 ) 
      .arg( probe_radius ) 
      .arg( threshold ) 
      ;
   cout << QString( "starting 1d for %1\n" ).arg( name );

   {

      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;

      saxs_atom new_atom;

      for ( unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

            // keep everything in angstroms!
            new_atom.pos[ 0 ] = this_atom->coordinate.axis[ 0 ] * atomic_scaler;
            new_atom.pos[ 1 ] = this_atom->coordinate.axis[ 1 ] * atomic_scaler;
            new_atom.pos[ 2 ] = this_atom->coordinate.axis[ 2 ] * atomic_scaler;

            if ( this_atom->name == "XH" && !our_saxs_options.iqq_use_atomic_ff )
            {
               continue;
            }

            QString use_resname = this_atom->resName;
            use_resname.replace( QRegExp( "_.*$" ), "" );

            QString mapkey = QString("%1|%2")
               .arg( use_resname )
               .arg( this_atom->name );

            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq )
                  ;
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  ;
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->name)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  ;
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options.use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
               }
            }

            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options.swh_excl_vol;
            }
            if ( this_atom->name == "XH" )
            {
               // skip excl vol for now
               new_atom.excl_vol = 0e0;
            }

            if ( our_saxs_options.hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }

            if ( our_saxs_options.iqq_use_saxs_excl_vol )
            {
               new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options.scale_excl_vol;
               tot_excl_vol_noh  += new_atom.excl_vol;
               total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }

            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( !our_saxs_options.iqq_use_atomic_ff &&
                 count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += 
                  QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name)
                  ;
               continue;
            }

            atoms.push_back(new_atom);
         }
      }
      // ok now we have all the atoms
   }

   if ( !atoms.size() )
   {
      errormsg = "Error: no atoms found!\n";
      return false;
   }

   // place 1st atom at 0,0,0

   // save value for excluded volume
   point zerooffset;
   zerooffset.axis[ 0 ] = atoms[ 0 ].pos[ 0 ];
   zerooffset.axis[ 1 ] = atoms[ 0 ].pos[ 1 ];
   zerooffset.axis[ 2 ] = atoms[ 0 ].pos[ 2 ];

   if ( atoms.size() > 1 )
   {
      for ( unsigned int a = 1; a < atoms.size(); a++ )
      {
         atoms[ a ].pos[ 0 ] -= atoms[ 0 ].pos[ 0 ];
         atoms[ a ].pos[ 1 ] -= atoms[ 0 ].pos[ 1 ];
         atoms[ a ].pos[ 2 ] -= atoms[ 0 ].pos[ 2 ];
      }
   }

   atoms[ 0 ].pos[ 0 ] = 0.0f;
   atoms[ 0 ].pos[ 1 ] = 0.0f;
   atoms[ 0 ].pos[ 2 ] = 0.0f;

   vector < point > atom_positions;

   for ( unsigned int a = 0; a < atoms.size(); a++ )
   {
      point this_point;
      this_point.axis[ 0 ] = atoms[ a ].pos[ 0 ] * atomic_scaler_inv;
      this_point.axis[ 1 ] = atoms[ a ].pos[ 1 ] * atomic_scaler_inv;
      this_point.axis[ 2 ] = atoms[ a ].pos[ 2 ] * atomic_scaler_inv;
      atom_positions.push_back( this_point );
   }

   vector < point > transform_from;
   {
      point this_point;
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 1.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = -1.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = -1.0f;
      this_point.axis[ 2 ] = 0.0f;
      transform_from.push_back( this_point );
      this_point.axis[ 0 ] = 0.0f;
      this_point.axis[ 1 ] = 0.0f;
      this_point.axis[ 2 ] = -1.0f;
      transform_from.push_back( this_point );
   }
   vector < point > transform_to = transform_from;

   saxs saxsH = saxs_map["H"];
   double one_over_4pi   = 1.0 / (4.0 * M_PI);
   double one_over_4pi_2 = one_over_4pi * one_over_4pi;

   total_modulii.resize( detector_pixels_width );
   for ( int i = 0; i < ( int ) total_modulii.size(); i++ )
   {
      total_modulii[ i ] = 0e0;
   }

   double deltapsi = 2.0 * M_PI / ( double ) axis_rotations;

   for ( unsigned int r = 0; r < rotations.size(); r++ )
   {
      if ( !( r % 200 ) )
      {
         cout << QString( "processing rotation %1 of %2\n" ).arg( r + 1 ).arg( rotations.size() );
      }
      if ( !get_excluded_volume_map() )
      {
         return false;
      }

      for ( unsigned int i = 0; i < ( unsigned int )excluded_volume.size(); i++ )
      {
         excluded_volume[ i ].axis[ 0 ] -= zerooffset.axis[ 0 ];
         excluded_volume[ i ].axis[ 1 ] -= zerooffset.axis[ 1 ];
         excluded_volume[ i ].axis[ 2 ] -= zerooffset.axis[ 2 ];
      }


      //       data.resize( detector_pixels_width );
      //       for ( int i = 0; i < ( int ) data.size(); i++ )
      //       {
      //          data[ i ] = complex < double > ( 0.0, 0.0 );
      //       }

      if ( rotations.size() > 1 )
      {
         transform_to[ 1 ].axis[ 0 ] = rotations[ r ][ 0 ];
         transform_to[ 1 ].axis[ 1 ] = rotations[ r ][ 1 ];
         transform_to[ 1 ].axis[ 2 ] = rotations[ r ][ 2 ];
         
         transform_to[ 2 ].axis[ 0 ] = -rotations[ r ][ 1 ];
         transform_to[ 2 ].axis[ 1 ] = rotations[ r ][ 0 ];
         // check this!
         transform_to[ 2 ].axis[ 2 ] = 0.0f; // rotations[ r ][ 2 ];

         transform_to[ 3 ] = 
            normal( cross( transform_to[ 1 ], transform_to[ 2 ] ) );

         transform_to[ 4 ].axis[ 0 ] = -transform_to[ 1 ].axis[ 0 ];
         transform_to[ 4 ].axis[ 1 ] = -transform_to[ 1 ].axis[ 1 ];
         transform_to[ 4 ].axis[ 2 ] = -transform_to[ 1 ].axis[ 2 ];

         transform_to[ 5 ].axis[ 0 ] = -transform_to[ 2 ].axis[ 0 ];
         transform_to[ 5 ].axis[ 1 ] = -transform_to[ 2 ].axis[ 1 ];
         transform_to[ 5 ].axis[ 2 ] = -transform_to[ 2 ].axis[ 2 ];

         transform_to[ 6 ].axis[ 0 ] = -transform_to[ 3 ].axis[ 0 ];
         transform_to[ 6 ].axis[ 1 ] = -transform_to[ 3 ].axis[ 1 ];
         transform_to[ 6 ].axis[ 2 ] = -transform_to[ 3 ].axis[ 2 ];

         vector < point > result;

         if ( !atom_align (
                           transform_from, 
                           transform_to, 
                           atom_positions,
                           result ) )
         {
            errormsg = "alignment failure";
            return false;
         }

         for ( unsigned int a = 0; a < atoms.size(); a++ )
         {
            atoms[ a ].pos[ 0 ] = result[ a ].axis[ 0 ] * atomic_scaler;
            atoms[ a ].pos[ 1 ] = result[ a ].axis[ 1 ] * atomic_scaler;
            atoms[ a ].pos[ 2 ] = result[ a ].axis[ 2 ] * atomic_scaler;
         }
         
         // and rotate excluded volume (this may have to be done piecemeal to save memory

         if ( !atom_align (
                           transform_from, 
                           transform_to, 
                           excluded_volume,
                           result 
                           ) )
         {
            errormsg = "alignment 2 failure";
            return false;
         }

         excluded_volume = result;
         result.clear( );
      }

      for ( int t = 0; t < axis_rotations; t++ )
      {

         double psi = ( double ) t * deltapsi;
         double cospsi = cos( psi );
         double sinpsi = sin( psi );

         s1d_data.resize( detector_pixels_width );
         for ( int i = 0; i < ( int ) s1d_data.size(); i++ )
         {
            s1d_data[ i ] = complex < double > ( 0.0, 0.0 );
         }
         // for each atom, compute scattering factor for each element on the detector


         for ( unsigned int a = 0; a < atoms.size(); a++ )
         {
            for ( unsigned int i = 0; i < s1d_data.size(); i++ )
            {
               double pixpos = ( double ) i * detector_width_per_pixel;
            
               double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );
            
               vector < double > Q( 3 );
               Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
               Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               Q[ 2 ] = 0e0;
               
               vector < double > Rv( 3 );
               Rv[ 0 ] = (cospsi + (1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 0 ]) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] - rotations[ r ][ 2 ] * sinpsi) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] + rotations[ r ][ 1 ] * sinpsi) * ( double ) atoms[ a ].pos[ 2 ];

               Rv[ 1 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] + rotations[ r ][ 2 ] * sinpsi) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 1 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 1 ]) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 1 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] - rotations[ r ][ 0 ] * sinpsi) * ( double ) atoms[ a ].pos[ 2 ];

               Rv[ 2 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] - rotations[ r ][ 1 ] * sinpsi) * ( double ) atoms[ a ].pos[ 0 ];
               Rv[ 2 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] + rotations[ r ][ 0 ] * sinpsi) * ( double ) atoms[ a ].pos[ 1 ];
               Rv[ 2 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 2 ] * rotations[ r ][ 2 ]) * ( double ) atoms[ a ].pos[ 2 ];

               //                vector < double > Rv( 3 );
               //                Rv[ 0 ] = ( double ) atoms[ a ].pos[ 0 ];
               //                Rv[ 1 ] = ( double ) atoms[ a ].pos[ 1 ];
               //                Rv[ 2 ] = ( double ) atoms[ a ].pos[ 2 ];
               
               double QdotR = 
                  Q[ 0 ] * Rv[ 0 ] +
                  Q[ 1 ] * Rv[ 1 ] +
                  Q[ 2 ] * Rv[ 2 ];
               
               complex < double > iQdotR = complex < double > ( 0e0, -QdotR );
            
               complex < double > expiQdotR = exp( iQdotR );
               
               // F_atomic
               
               saxs saxs = saxs_map[ atoms[ a ].saxs_name ];
               
               double q = sqrt( Q[ 0 ] * Q[ 0 ] + Q[ 1 ] * Q[ 1 ] + Q[ 2 ] * Q[ 2 ] );

               double q_2_over_4pi = q * q * one_over_4pi_2;

               double F_at =
                  compute_ff( saxs,
                              saxsH,
                              atoms[ a ].residue_name,
                              atoms[ a ].saxs_name,
                              atoms[ a ].atom_name,
                              atoms[ a ].hydrogens,
                              q,
                              q_2_over_4pi );
            
               s1d_data[ i ] += complex < double > ( F_at, 0e0 ) * expiQdotR;
            }
         }

         // now subtract excluded volume

         if ( rho0 )
         {
            for ( unsigned int i = 0; i < s1d_data.size(); i++ )
            {
               double pixpos = ( double ) i * detector_width_per_pixel;

               double S_length = sqrt( detector_distance * detector_distance + pixpos * pixpos );

               vector < double > Q( 3 );
               Q[ 0 ] = 2.0 * M_PI * ( ( pixpos / S_length ) / lambda );
               Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               Q[ 2 ] = 0e0;
               
               for ( unsigned int j = 0; j < ( unsigned int )excluded_volume.size(); j++ )
               {
                  vector < double > Rv( 3 );
                  Rv[ 0 ] = (cospsi + (1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 0 ]) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] - rotations[ r ][ 2 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 0 ] += ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] + rotations[ r ][ 1 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  Rv[ 1 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 1 ] + rotations[ r ][ 2 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 1 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 1 ]) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 1 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] - rotations[ r ][ 0 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  Rv[ 2 ] = ((1.0 - cospsi) * rotations[ r ][ 0 ] * rotations[ r ][ 2 ] - rotations[ r ][ 1 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 0 ];
                  Rv[ 2 ] += ((1.0 - cospsi) * rotations[ r ][ 1 ] * rotations[ r ][ 2 ] + rotations[ r ][ 0 ] * sinpsi) * ( double ) excluded_volume[ j ].axis[ 1 ];
                  Rv[ 2 ] += (cospsi + (1.0 - cospsi) * rotations[ r ][ 2 ] * rotations[ r ][ 2 ]) * ( double ) excluded_volume[ j ].axis[ 2 ];

                  double QdotR = 
                     Q[ 0 ] * Rv[ 0 ] +
                     Q[ 1 ] * Rv[ 1 ] +
                     Q[ 2 ] * Rv[ 2 ];

                  QdotR *= spec_multiplier;

                  complex < double > iQdotR = complex < double > ( 0e0, -QdotR );

                  complex < double > expiQdotR = exp( iQdotR );

                  complex < double > rho0expiQdotR = complex < double > ( rho0, 0e0 ) * expiQdotR;

                  s1d_data[ i ] -= rho0expiQdotR * complex < double > ( deltaR * deltaR * deltaR, 0 );
               }
            }
         }

         if ( !update_image() )
         {
            return false;
         }
         if ( intermediate_saves.count( r + 1 ) && t == axis_rotations - 1 )
         {
            vector < double > q( detector_pixels_width );
            vector < double > I = total_modulii;
            for ( int i = 0; i < ( int ) q.size(); i++ )
            {
               q[ i ] = q_of_pixel( i );
               I[ i ] /= ( double ) plot_count;
            }

            QString qs_plot_count = QString( "%1" ).arg( plot_count );
            while ( qs_plot_count.length() < qs_total_plots.length() )
            {
               qs_plot_count = "0" + qs_plot_count;
            }

            QString name = 
               QString( "%1_tp%2_rot%3_ar%4_d3R%5_rho0%6_pr%7_t%8" )
               .arg( model_vector[ current_model ].model_id )
               .arg( qs_plot_count ) 
               .arg( plot_count / axis_rotations ) 
               .arg( axis_rotations ) 
               .arg( deltaR ) 
               .arg( rho0 ) 
               .arg( probe_radius ) 
               .arg( threshold ) 
               ;

            if ( !write_output( name, q, I ) )
            {
               return false;
            }
         }         
      } // axis rotations
   } // rotations


   // save the data

   vector < double > q( detector_pixels_width );
   vector < double > I = total_modulii;
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
      I[ i ] /= ( double ) plot_count;
   }

   if ( !write_output( name, q, I ) )
   {
      return false;
   }
   return true;
}

bool US_Saxs_Util::load_rotations( unsigned int number, vector < vector < double > > &rotations )
{
   QFile f( control_parameters[ "1drotationfile" ] );
   if ( !f.exists() )
   {
      errormsg = QString( "Notice: cached rotations file %1 does not exist" )
         .arg( f.fileName() );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "Notice: found cached rotations file %1 but could not open it" )
         .arg( f.fileName() );
      return false;
   }

   QTextStream ts( &f );

   unsigned int line = 0;

   vector < double > p(3);
   rotations.clear( );

   while ( !ts.atEnd() && rotations.size() < number )
   {
      QString     qs  = ts.readLine();
      line++;

      QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

      if ( qsl.size() != 3 )
      {
         errormsg =  QString( "Notice: error in found cached rotations file %1 line %2, does not contain 3 tokens" )
            .arg( f.fileName() )
            .arg( line )
            ;
         f.close();
         return false;
      }
      p[ 0 ] = qsl[ 0 ].toDouble();
      p[ 1 ] = qsl[ 1 ].toDouble();
      p[ 2 ] = qsl[ 2 ].toDouble();
      rotations.push_back( p );
   }
   f.close();
   if ( ( unsigned int )rotations.size() != number )
   {
      errormsg = 
         QString( "Notice: error:  cached rotations file %1 line %2, does not contain sufficient rotations (%3 requested vs %4 found)" )
         .arg( f.fileName() )
         .arg( line )
         .arg( number )
         .arg( rotations.size() )
         ;
      rotations.clear( );
      return false;
   }
   return true;
}

bool US_Saxs_Util::update_image()
{
   if ( !s1d_data.size() )
   {
      errormsg = "Internal error: update_image(): zero s1d_data size";
      return false;
   }

   if ( detector_pixels_width != ( int ) s1d_data.size() )
   {
      errormsg = "Internal error: update_image(): detector s1d_data mismatch";
      return false;
   }

   if ( detector_pixels_width != ( int ) total_modulii.size() )
   {
      errormsg = "Internal error: update_image(): saved q point count mismatch";
      return false;
   }

   // compute modulii
   vector < double > modulii( s1d_data.size() );

   double max_modulii = 0e0;
   double min_modulii = 1e99;
   
   for ( unsigned int i = 0; i < s1d_data.size(); i++ )
   {
      modulii[ i ] = real( s1d_data[ i ] * conj( s1d_data[ i ] ) );
      if ( max_modulii < modulii[ i ] )
      {
         max_modulii = modulii[ i ];
      }
      if ( min_modulii > modulii[ i ] )
      {
         min_modulii = modulii[ i ];
      }
   }

   vector < double > q( detector_pixels_width );
   for ( int i = 0; i < ( int ) q.size(); i++ )
   {
      q[ i ] = q_of_pixel( i );
   }

   plot_count++;

   for ( int i = 0; i < ( int ) modulii.size(); i++ )
   {
      total_modulii [ i ] += modulii[ i ];
   }

   return true;
}

double US_Saxs_Util::q_of_pixel( int pixels_width )
{
   return q_of_pixel( ( double ) pixels_width * detector_width_per_pixel );
}

double US_Saxs_Util::q_of_pixel( double width )
{
   double S_length = sqrt( detector_distance * detector_distance + width * width);

   vector < double > Q( 2 );
   Q[ 0 ] = 2.0 * M_PI * ( ( width / S_length ) / lambda );
   Q[ 1 ] = 2.0 * M_PI * ( ( ( detector_distance / S_length ) - 1e0 ) / lambda );
               
   return sqrt( Q[ 0 ] * Q[ 0 ] + Q[ 1 ] * Q[ 1 ] );
}

bool US_Saxs_Util::setup_excluded_volume_map()
{
   if ( !rho0 )
   {
      return true;
   }
#if !defined( HAS_CBF )
   errormsg = "No CBF linked in this version\n";
   return false;
#else
   errormsg = "ev maps not yet supported";
   return false;
#endif
}


bool US_Saxs_Util::get_excluded_volume_map()
{
   errormsg = "";
   excluded_volume.clear( );

   if ( !rho0 )
   {
      return true;
   }
#if !defined( HAS_CBF )
   errormsg = "No CBF linked in this version\n";
   return false;
#else
   errormsg = "ev maps not yet supported";
   return false;
#endif
}

