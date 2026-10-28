#include "../include/us_saxs_util.h"
#include "../include/us_lm.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SAXS_MIN_Q 1e-6
#define SLASH QDir::separator();

bool US_Saxs_Util::calc_saxs_iq_native_fast_bead_model()
{
   errormsg = "";
   noticemsg = "";

   for ( unsigned int i = 0; i < bead_models.size(); i++ )
   {
      double tot_excl_vol = 0e0;
      current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < bead_models[current_model].size(); j++)
      {
         PDB_atom *this_atom = &(bead_models[current_model][j]);
         new_atom.pos[0] = this_atom->bead_coordinate.axis[0];
         new_atom.pos[1] = this_atom->bead_coordinate.axis[1];
         new_atom.pos[2] = this_atom->bead_coordinate.axis[2];

         new_atom.saxs_name = this_atom->saxs_name;
         new_atom.hybrid_name = this_atom->saxs_data.saxs_name;
         new_atom.hydrogens = 0;
         new_atom.radius = this_atom->bead_computed_radius;
         
         // this is probably correct but FoXS uses the saxs table excluded volume
         new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }
      
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      noticemsg += QString("Number of atoms %1.\n"
                           "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
         .arg(atoms.size())
         .arg(our_saxs_options.start_q)
         .arg(our_saxs_options.end_q)
         .arg(our_saxs_options.delta_q)
         .arg(q_points);

      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);
      q2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
      }

      float vi; // excluded water vol
      float vie; // excluded water * e density

      // compute form factors

      double delta_rho = control_parameters[ "targetedensity" ].toDouble() - our_saxs_options.water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options.bead_model_rayleigh )
      {
         noticemsg += QString("using Rayleigh structure factors, delta_rho %1\n").arg(delta_rho);
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options.scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options.scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;

         bool rayleigh_ok = false;
         if ( our_saxs_options.bead_model_rayleigh )
         {
            vector < double > F;
            vector < double > q(1);
            q[ 0 ] = 0;
      
            QString save_errormsg = errormsg;
            if ( compute_rayleigh_structure_factors( 
                                                    atoms[ i ].radius * scaling_root,
                                                    delta_rho,
                                                    q,
                                                    F
                                                    ) )
            {
               fp[ 0 ][ i ] = F[ 0 ];
               rayleigh_ok = true;
            } else {
               noticemsg += "using Rayleigh structure factors failed:" + errormsg;
            }
            errormsg = save_errormsg;
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               noticemsg += "NOT using Rayleigh structure factors\n";
               told_you = true;
            }
         
            f[0][i] = 
               // atoms[i].srv * 
               ( 
                atoms[i].saxs_data.c +
                atoms[i].saxs_data.a[0] +
                atoms[i].saxs_data.a[1] +
                atoms[i].saxs_data.a[2] +
                atoms[i].saxs_data.a[3] );
            
            fc[0][i] = vie;
            fp[0][i] = f[0][i] - fc[0][i];
         }
      }

      // foxs method: compute real space distribution

      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      // double rik; // distance from atom i to k 
      double rik2; // square distance from atom i to k 
      float delta = our_saxs_options.fast_bin_size;
      float one_over_delta = 1.0 / delta;
      // float delta_pr = our_saxs_options.bin_size;
      // float one_over_delta_pr = 1.0 / delta_pr;
      unsigned int pos;
      // unsigned int pos_pr;
      vector < float > hist;
      vector < float > hist_pr;
      
      {
         for ( unsigned int i = 0; i < as1; i++ )
         {
            for ( unsigned int k = i + 1; k < as; k++ )
            {
               rik2 = 
                  (atoms[i].pos[0] - atoms[k].pos[0]) *
                  (atoms[i].pos[0] - atoms[k].pos[0]) +
                  (atoms[i].pos[1] - atoms[k].pos[1]) *
                  (atoms[i].pos[1] - atoms[k].pos[1]) +
                  (atoms[i].pos[2] - atoms[k].pos[2]) *
                  (atoms[i].pos[2] - atoms[k].pos[2]);
               // rik = sqrt( rik2 );
               pos = (unsigned int)floor(rik2 * one_over_delta);
               
               if ( hist.size() <= pos )
               {
                  hist.resize(pos + 1024);
               }
               hist[pos] += 2.0 * fp[0][i] * fp[0][k];
            }
            hist[0] += fp[0][i] * fp[0][i];
         }
         hist[0] += fp[0][as1] * fp[0][as1];
      }         
      
      while( hist.size() && !hist[hist.size()-1] ) 
      {
         hist.pop_back();
      }

      noticemsg += QString( "Total excluded volume %1\n" ).arg( tot_excl_vol );

      // hist = save_hist;

      vector < double > I;
      vector < double > dist(hist.size());
      I.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
      }
      {
         double d = 0.0;
         
         for ( unsigned int j = 0; j < hist.size(); j++, d += delta )
         {
            dist[j] = sqrt( d );
            // cout << QString("dist[%1] = %2\n").arg(j).arg(dist[j]);
         }
      }

      double x;
      for ( unsigned int i = 0; i < q.size(); i++ )
      {
         // cout << "q[i] " << q[i] << endl;
         
         for ( unsigned int j = 0; j < hist.size(); j++ )
         {
            x = dist[j] * q[i];
            // cout << "dist[ " <<  j << "] = " << dist[j] << endl;
            // cout << "sinc(x) = "  << (( fabs(x) < 1e-16 ) ? 1.0 : sin(x) / x) << endl;
            // cout << "r_dist[" << j << "] = " << hist[j] << endl;
            I[i] += hist[j] * (( fabs(x) < 1e-16 ) ? 1.0 : sin(x) / x);
         }
         I[i] *= exp( - our_saxs_options.fast_modulation * q[i] * q[i] );
         // cout << "I[" << i << "] = " << I[i] << endl;
      }
         
      noticemsg += "I(q) computed.\n";

      // save the data to a file
      if ( !write_output( QString( "%1" ).arg( current_model ), q, I ) )
      {
         return false;
      }
   } // models
   return true;
}

//  ------------------------------------------------------------------------------------------------------
bool US_Saxs_Util::calc_saxs_iq_native_debye_bead_model()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry

   errormsg = "";
   noticemsg = "";

   for ( unsigned int i = 0; i < bead_models.size(); i++ )
   {
      // double tot_excl_vol = 0e0;
      current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < bead_models[current_model].size(); j++)
      {
         PDB_atom *this_atom = &(bead_models[current_model][j]);
         new_atom.pos[0] = this_atom->bead_coordinate.axis[0];
         new_atom.pos[1] = this_atom->bead_coordinate.axis[1];
         new_atom.pos[2] = this_atom->bead_coordinate.axis[2];
         
         new_atom.saxs_name = this_atom->saxs_name;
         new_atom.hybrid_name = this_atom->saxs_data.saxs_name;
         new_atom.hydrogens = 0;
         new_atom.radius = this_atom->bead_computed_radius;

         // this is probably correct but FoXS uses the saxs table excluded volume
         new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         // tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }
      
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      noticemsg += QString("Number of atoms %1.\n"
                           "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
         .arg(atoms.size())
         .arg(our_saxs_options.start_q)
         .arg(our_saxs_options.end_q)
         .arg(our_saxs_options.delta_q)
         .arg(q_points);

      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);
      q2.resize(q_points);
      q_over_4pi_2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
         q_over_4pi_2[j] = q[j] * q[j] * one_over_4pi_2;
      }

      double vi_23_4pi;
      float vi; // excluded water vol
      float vie; // excluded water * e density

      // compute form factors

      double delta_rho = control_parameters[ "targetedensity" ].toDouble() - our_saxs_options.water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options.bead_model_rayleigh )
      {
         noticemsg += QString("using Rayleigh structure factors, delta_rho %1\n").arg(delta_rho);
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options.scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options.scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;
         
         bool rayleigh_ok = false;
         if ( our_saxs_options.bead_model_rayleigh )
         {
            vector < double > F;
            QString save_errormsg = errormsg;
            if (compute_rayleigh_structure_factors( 
                                                   atoms[ i ].radius * scaling_root,
                                                   delta_rho,
                                                   q,
                                                   F
                                                   ) )
            {
               for ( unsigned int j = 0; j < q_points; j++ )
               {
                  fp[ j ][ i ] = F[ j ];
               }
               rayleigh_ok = true;
            } else {
               noticemsg += "using Rayleigh structure factors failed:" + errormsg;
            }
            errormsg = save_errormsg;
            
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               noticemsg += "NOT using Rayleigh structure factors\n";
               told_you = true;
            }
               
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               // note: since there are only a few 'saxs' coefficient sets
               // the saxs.c + saxs.a[i] * exp() can be precomputed
               // possibly saving time... but this isn't our most computationally intensive step
               // so I'm holding off for now.
               f[j][i] = 
                  // atoms[i].srv * 
                  (
                   saxs.c + 
                   saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
                   saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
                   saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
                   saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]) );

               fc[j][i] =  vie * exp(vi_23_4pi * q2[j]);
               fp[j][i] = f[j][i] - fc[j][i];
#if defined(ONLY_PHYSICAL_F)
               if ( fp[j][i] < 0.0f ) 
               {
                  fp[j][i] = 0.0f;
               }
#endif
            }
         }
      }
      noticemsg += "f' computed, starting computation of I(q)\n";
      vector < double > I;
      vector < double > Ia;
      vector < double > Ic;
      I.resize(q_points);
      Ia.resize(q_points);
      Ic.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
         Ia[j] = 0.0f;
         Ic[j] = 0.0f;
      }
      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik

      for ( unsigned int i = 0; i < as1; i++ )
      {
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = 
               sqrt(
                    (atoms[i].pos[0] - atoms[k].pos[0]) *
                    (atoms[i].pos[0] - atoms[k].pos[0]) +
                    (atoms[i].pos[1] - atoms[k].pos[1]) *
                    (atoms[i].pos[1] - atoms[k].pos[1]) +
                    (atoms[i].pos[2] - atoms[k].pos[2]) *
                    (atoms[i].pos[2] - atoms[k].pos[2])
                    );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * q[j];
               sqrikd = ( fabs(qrik) < 1e-16 ) ? 1.0 : sin(qrik) / qrik;
               I[j] += 2.0 * fp[j][i] * fp[j][k] * sqrikd;
               Ia[j] += 2.0 * f[j][i] * f[j][k] * sqrikd;
               Ic[j] += 2.0 * fc[j][i] * fc[j][k] * sqrikd;
            } // j
         } // k
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            I[j] += fp[j][i] * fp[j][i];
            Ia[j] += f[j][i] * f[j][i];
            Ic[j] += fc[j][i] * fc[j][i];
         }
      }
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] += fp[j][as1] * fp[j][as1];
         Ia[j] += f[j][as1] * f[j][as1];
         Ic[j] += fc[j][as1] * fc[j][as1];
      }

      noticemsg += "I(q) computed.\n";

      // save the data to a file
      if ( !write_output( QString( "%1" ).arg( current_model ), q, I ) )
      {
         return false;
      }
   } // models
   return true;
}

//  ------------------------------------------------------------------------------------------------------
bool US_Saxs_Util::calc_saxs_iq_native_hybrid_bead_model()
{
   errormsg = "";
   noticemsg = "";

   for ( unsigned int i = 0; i < bead_models.size(); i++ )
   {
      // double tot_excl_vol = 0e0;
      current_model = i;

      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < bead_models[current_model].size(); j++)
      {
         PDB_atom *this_atom = &(bead_models[current_model][j]);
         new_atom.pos[0] = this_atom->bead_coordinate.axis[0];
         new_atom.pos[1] = this_atom->bead_coordinate.axis[1];
         new_atom.pos[2] = this_atom->bead_coordinate.axis[2];

         new_atom.saxs_name = this_atom->saxs_name;
         new_atom.hybrid_name = this_atom->saxs_data.saxs_name;
         new_atom.hydrogens = 0;
         new_atom.radius = this_atom->bead_computed_radius;
         
         // this is probably correct but FoXS uses the saxs table excluded volume
         new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         // tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }
      
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      noticemsg += QString("Number of atoms %1.\n"
                           "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
         .arg(atoms.size())
         .arg(our_saxs_options.start_q)
         .arg(our_saxs_options.end_q)
         .arg(our_saxs_options.delta_q)
         .arg(q_points);
      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);
      q2.resize(q_points);
      q_over_4pi_2.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
         q_over_4pi_2[j] = q[j] * q[j] * one_over_4pi_2;
      }

      double vi_23_4pi;
      float vi; // excluded water vol
      float vie; // excluded water * e density

      saxs saxsH = saxs_map["H"];

      // compute form factors

      double delta_rho = control_parameters[ "targetedensity" ].toDouble() - our_saxs_options.water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options.bead_model_rayleigh )
      {
         noticemsg += QString( "using Rayleigh structure factors, delta_rho %1\n" ).arg( delta_rho );
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options.scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options.scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;

         bool rayleigh_ok = false;
         if ( our_saxs_options.bead_model_rayleigh )
         {
            vector < double > F;
            QString save_errormsg = errormsg;
            if ( compute_rayleigh_structure_factors( 
                                                    atoms[ i ].radius * scaling_root,
                                                    delta_rho,
                                                    q,
                                                    F
                                                    ) )
            {
               for ( unsigned int j = 0; j < q_points; j++ )
               {
                  fp[ j ][ i ] = F[ j ];
               }
               rayleigh_ok = true;
            } else {
               noticemsg += "using Rayleigh structure factors failed:" + errormsg;
            }
            errormsg = save_errormsg;
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               noticemsg += "NOT using Rayleigh structure factors\n";
               told_you = true;
            }
               
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               // note: since there are only a few 'saxs' coefficient sets
               // the saxs.c + saxs.a[i] * exp() can be precomputed
               // possibly saving time... but this isn't our most computationally intensive step
               // so I'm holding off for now.
               
               f[j][i] = saxs.c + 
                  saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
                  saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
                  saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
                  saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]) +
                  atoms[i].hydrogens * 
                  ( saxsH.c + 
                    saxsH.a[0] * exp(-saxsH.b[0] * q_over_4pi_2[j]) +
                    saxsH.a[1] * exp(-saxsH.b[1] * q_over_4pi_2[j]) +
                    saxsH.a[2] * exp(-saxsH.b[2] * q_over_4pi_2[j]) +
                    saxsH.a[3] * exp(-saxsH.b[3] * q_over_4pi_2[j]) );
               fc[j][i] =  vie * exp(vi_23_4pi * q2[j]);
               fp[j][i] = f[j][i] - fc[j][i];
#if defined(ONLY_PHYSICAL_F)
               if ( fp[j][i] < 0.0f ) 
               {
                  fp[j][i] = 0.0f;
               }
#endif
            }
         }
      }
      noticemsg += "f' computed, starting computation of I(q)\n";

      // foxs method: compute real space distribution

      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double rik2; // square distance from atom i to k 
      float delta = our_saxs_options.fast_bin_size;
      float one_over_delta = 1.0 / delta;
      unsigned int pos;
      vector < float > hist;
      
      for ( unsigned int i = 0; i < as1; i++ )
      {
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik2 = 
               (atoms[i].pos[0] - atoms[k].pos[0]) *
               (atoms[i].pos[0] - atoms[k].pos[0]) +
               (atoms[i].pos[1] - atoms[k].pos[1]) *
               (atoms[i].pos[1] - atoms[k].pos[1]) +
               (atoms[i].pos[2] - atoms[k].pos[2]) *
               (atoms[i].pos[2] - atoms[k].pos[2]);
            // rik = sqrt( rik2 );
            // rik_array[i][k] = sqrt(rik2);

            pos = (unsigned int)floor(rik2 * one_over_delta);
            
            if ( hist.size() <= pos )
            {
               hist.resize(pos + 1024);
            }
            hist[pos] += 2.0 * fp[0][i] * fp[0][k];
         }
         hist[0] += fp[0][i] * fp[0][i];
      }
      hist[0] += fp[0][as1] * fp[0][as1];
      
      while( hist.size() && !hist[hist.size()-1] ) 
      {
         hist.pop_back();
      }

      // hist = save_hist;

      vector < double > I;
      vector < double > dist(hist.size());
      I.resize(q_points);
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
      }
      {
         double d = 0.0;
         
         for ( unsigned int j = 0; j < hist.size(); j++, d += delta )
         {
            dist[j] = sqrt( d );
         }
      }

      double x;
      for ( unsigned int i = 0; i < q.size(); i++ )
      {
         
         for ( unsigned int j = 0; j < hist.size(); j++ )
         {
            x = dist[j] * q[i];
            I[i] += hist[j] * (( fabs(x) < 1e-16 ) ? 1.0 : sin(x) / x);
         }
         I[i] *= exp( - our_saxs_options.fast_modulation * q[i] * q[i] );
      }

      // -------------------------------
      vector < unsigned int > r;
      bool adaptive_ok = false;

      if ( our_saxs_options.saxs_iq_hybrid_adaptive )
      {
         QString save_errormsg = errormsg;
         QString save_noticemsg = noticemsg;

         if ( !create_adaptive_grid( q, I, our_saxs_options.hybrid2_q_points, r ) )
         {
            noticemsg += errormsg;
         } else {
            adaptive_ok = true;
         }
         if ( !noticemsg.isEmpty() )
         {
            save_noticemsg += noticemsg;
         }
         errormsg = save_errormsg;
         noticemsg = save_noticemsg;
      }

      unsigned int q_delta = q_points / our_saxs_options.hybrid2_q_points;
      vector < unsigned int > use_q;

      if ( q_delta < 2 )
      {
         q_delta = 2;
      }

      vector < double > fast_I = I;
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         I[j] = 0.0f;
      }

      for ( unsigned int j = 0; j < q_points; j += q_delta )
      {
         use_q.push_back(j);
      }

      if ( use_q[use_q.size() - 1] != q_points - 1 )
      {
         use_q.push_back(q_points - 1);
      }

      if ( our_saxs_options.saxs_iq_hybrid_adaptive && adaptive_ok )
      {
         use_q = r;
      }

      unsigned int use_q_size = use_q.size();

      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik
      for ( unsigned int i = 0; i < as1; i++ )
      {
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            // rik = rik_array[i][k]; 
            rik = sqrt(
                       (atoms[i].pos[0] - atoms[k].pos[0]) *
                       (atoms[i].pos[0] - atoms[k].pos[0]) +
                       (atoms[i].pos[1] - atoms[k].pos[1]) *
                       (atoms[i].pos[1] - atoms[k].pos[1]) +
                       (atoms[i].pos[2] - atoms[k].pos[2]) *
                       (atoms[i].pos[2] - atoms[k].pos[2])
                       );
                       
            for ( unsigned int l = 0; l < use_q_size; l++ )
            {
               unsigned int j = use_q[l];
               qrik = rik * q[j];
               sqrikd = ( fabs(qrik) < 1e-16 ) ? 1.0 : sin(qrik) / qrik;
               I[j] += 2.0 * fp[j][i] * fp[j][k] * sqrikd;
            } // j
         } // k
         for ( unsigned int l = 0; l < use_q_size; l++ )
         {
            unsigned int j = use_q[l];
            I[j] += fp[j][i] * fp[j][i];
         }
      }
      vector < double > deltaI(use_q_size);
      for ( unsigned int l = 0; l < use_q_size; l++ )
      {
         unsigned int j = use_q[l];
         I[j] += fp[j][as1] * fp[j][as1];
         deltaI[l] = I[j] - fast_I[j];
      }
      
      {
         QString save_errormsg = errormsg;

         if ( our_saxs_options.saxs_iq_native_hybrid )
         {
            if ( !linear_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
            {
               cout << errormsg << endl;
            }
         } else {
            if ( our_saxs_options.saxs_iq_native_hybrid2 )
            {
               if ( !quadratic_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << errormsg << endl;
               }
            } else {
               if ( !cubic_spline_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << errormsg << endl;
               }
            }            
         }
         errormsg = save_errormsg;
      }

      noticemsg += "I(q) computed.\n";
      
      // save the data to a file
      if ( !write_output( QString( "%1" ).arg( current_model ), q, I ) )
      {
         return false;
      }
   } // models
   return true;
}

bool US_Saxs_Util::run_iqq_bead_model()
{
   errormsg = "";
   noticemsg = "";
   if ( !control_parameters.count( "targetedensity" ) )
   {
      errormsg = "Error: TargetEDensity must be defined";
      return false;
   }

   if ( control_parameters.count( "iqmethod" ) )
   {
      QString qs = control_parameters[ "iqmethod" ];
      if ( qs == "db" )
      {
         calc_saxs_iq_native_debye_bead_model();
      }
      if ( qs == "hy" || qs == "hya" ||
           qs == "h2" || qs == "h2a" ||
           qs == "h3" || qs == "h3a" )
      {
         calc_saxs_iq_native_hybrid_bead_model();
      }
      if ( qs == "fd" )
      {
         calc_saxs_iq_native_fast_bead_model();
      }
      if ( qs == "foxs" )
      {
         errormsg = "Error: FoXs method not supported for bead models";
         return false;
      }
      if ( qs == "crysol" )
      {
         errormsg = "Error: Crysol method not supported for bead models";
         return false;
      }
      if ( qs == "cryson" )
      {
         errormsg = "Error: Cryson method not supported for bead models";
         return false;
      }
   }
   return true;
}

namespace LM {
   unsigned int exponential_terms;
   double       minusoneoverfourpisq;
   double compute_exponential_f( double t, const double *par )
   {
      double result = par[ 0 ];
      //       cout << QString( "t %1 minusoneoverfourisq %2 minusoneoverfourpisq * t * t %3\n" )
      //          .arg( t )
      //          .arg( minusoneoverfourpisq )
      //          .arg( minusoneoverfourpisq * t * t );
         
      for ( unsigned int i = 0; i < exponential_terms; i++ )
      {
         // cout << QString( "term %1 using parameters a=%2 b=%3\n" ).arg( i ).arg( par[ i * 2 ] ).arg( par[ 1 + i * 2 ] );
         result += 
            par[ 1 + i * 2 ] * 
            exp( par[ 2 + i * 2 ] * minusoneoverfourpisq * t * t );
      }
      // cout << QString( "term constant c=%1\n" ).arg( par[ exponential_terms * 2 ] );
      return result;
   }
}

bool US_Saxs_Util::compute_exponential( 
                                       vector < double > &q, 
                                       vector < double > &I,
                                       vector < double > &coeff4,
                                       vector < double > &coeff5,
                                       double            &norm4,
                                       double            &norm5
                                       )
{
   vector < double > coeffv;
   double            normv;
   return compute_exponential( q, I, coeff4, coeff5, coeffv, norm4, norm5, normv, 0 );
}

// static void printvector( QString qs, vector < double > x )
// {
//    cout << QString( "%1: size %2:" ).arg( qs ).arg( x.size() );
//    for ( unsigned int i = 0; i < x.size(); i++ )
//    {
//       cout << QString( " %1" ).arg( x[ i ] );
//    }
//    cout << endl;
// }

bool US_Saxs_Util::compute_exponential( 
                                       vector < double > &q, 
                                       vector < double > &I,
                                       vector < double > &coeff4,
                                       vector < double > &coeff5,
                                       vector < double > &coeffv,
                                       double            &norm4,
                                       double            &norm5,
                                       double            &normv,
                                       // compute increased coeff size (when maxv > 5)
                                       unsigned int      maxv,
                                       bool              use_gsm_fitting,
                                       bool              use_quick_fitting
                                       )
{
   if ( q.size() != I.size() )
   {
      errormsg = QString( "compute_exponential() I.size() %1 does not equal q.size() %2\n" )
         .arg( I.size() ).arg( q.size() );
      return false;
   }

   // default 4 & 5 terms or maxv is larger
   unsigned int terms = maxv > 5 ? maxv : 5;
   vector < double > use_I = I;
   for ( unsigned int i = 0; i < use_I.size(); i++ )
   {
      use_I[ i ] = sqrt( use_I[ i ] );
   }

   using namespace LM;
   {
      unsigned int n_par = 1 + 2 * terms;
      unsigned int m_dat = ( unsigned int ) q.size();
      cout << QString( "compute exponential terms %1 n_par %2 m_dat %3\n" )
         .arg( terms ).arg( n_par ).arg( m_dat ).toLatin1().data();

      minusoneoverfourpisq  = -1e0 / ( 16e0 * M_PI * M_PI );

      vector < double > par( n_par );
      // not much of an initial guess
      for ( unsigned int i = 0; i < n_par; i++ )
      {
         par[ i ] = 1e0;
      }
      //       if ( n_par == 9 )
      //       {
      //          par[ 0 ] = 2.31;
      //          par[ 1 ] = 20.8439;
      //          par[ 2 ] = 1.02;
      //          par[ 3 ] = 10.2075;
      //          par[ 4 ] = 1.5886;
      //          par[ 5 ] = 0.5687;
      //          par[ 6 ] = 0.865;
      //          par[ 7 ] = 51.6512;
      //          par[ 8 ] = 0.2156;
      //          cout << "used presets\n";
      //       }

      //       if ( n_par == 11 )
      //       {
      //          par[ 0 ] = 2.657506;
      //          par[ 1 ] = 0.713791;
      //          par[ 2 ] = 1.078079;
      //          par[ 3 ] = 14.780758;
      //          par[ 4 ] = 1.490909;
      //          par[ 5 ] = 0.776775;
      //          par[ 6 ] = -4.24107;
      //          par[ 7 ] = 42.086842;
      //          par[ 8 ] = 0.713791;
      //          par[ 9 ] = -0.000294;
      //          par[ 10 ] =  4.297983	;
      //          cout << "used presets\n";
      //       }
      lm_control_struct control = lm_control_double;
      control.printflags = 0; // 3; // monitor status (+1) and parameters (+2)
      control.stepbound  = 1000;
      control.maxcall    = 1000;

      lm_status_struct status;

      // incremental approach
      if ( use_quick_fitting )
      {
         for ( unsigned int this_terms = 0; this_terms <= terms; this_terms++ )
         {
            unsigned int this_n_par = 1 + 2 * this_terms;
            compute_exponential_t = q;
            compute_exponential_y = use_I;
            exponential_terms     = this_terms;
         
            // printvector( "running lmcurve par", par );
            lmcurve_fit( ( int )      this_n_par,
                         ( double * ) &( par[ 0 ] ),
                         ( int )      m_dat,
                         ( double * ) &( compute_exponential_t[ 0 ] ),
                         ( double * ) &( compute_exponential_y[ 0 ] ),
                         compute_exponential_f,
                         (const lm_control_struct *)&control,
                         &status );
            //printvector( QString( "par is now (norm %1)" ).arg( status.fnorm ), par );

            //             // check sign of f( q==0 )
            //             double sum = par[ 0 ];
            //             for ( unsigned int i = 1; i < this_n_par; i += 2 )
            //             {
            //                sum += par[ i ];
            //             }
            //             if ( sum < 0e0 )
            //             {
            //                cout << QString( "WARNING: negative scattering at q=0\n" );
            //             }

            if ( !this_terms || normv > status.fnorm )
            {
               coeffv = par;
               normv  = status.fnorm;
            } 

            if ( this_terms == 4 )
            {
               coeff4 = par;
               norm4  = status.fnorm;
               // printvector( "coeff4 set to", coeff4 );
            }
            if ( this_terms == 5 )
            {
               coeff5 = par;
               norm5  = status.fnorm;
               // printvector( "coeff5 set to", coeff5 );
            }
         }
      } else {
         for ( unsigned int this_terms = 0; this_terms <= terms; this_terms++ )
         {
            unsigned int this_n_par = 1 + 2 * this_terms;
            compute_exponential_t = q;
            compute_exponential_y = use_I;
            exponential_terms     = this_terms;
         
            vector < double > save_par = par;

            vector < double > best_par;
            double            best_norm = 1e99;  // only set to avoid compiler warning

            for ( unsigned int i = 0; i <= this_n_par; i++ )
            {
               cout << QString( "computing %1 term exponential fit try %2 of %3\n" )
                  .arg( this_terms )
                  .arg( i + 1 )
                  .arg( this_n_par + 1 )
                    << flush ;
               par = save_par;
               if ( i )
               {
                  par[ i - 1 ] = -par[ i - 1 ];
               }

               if ( use_gsm_fitting )
               {
                  compute_gsm_exponentials_setup( q, I );
                  double start_rmsd;
                  double end_rmsd;
                  unsigned int gsm_repeat = 0;
                  lmcurve_fit( ( int )      this_n_par,
                               ( double * ) &( par[ 0 ] ),
                               ( int )      m_dat,
                               ( double * ) &( compute_exponential_t[ 0 ] ),
                               ( double * ) &( compute_exponential_y[ 0 ] ),
                               compute_exponential_f,
                               (const lm_control_struct *)&control,
                               &status );
                  vector < double > par_before_gsm      = par;
                  double            par_before_gsm_norm = status.fnorm;

                  do {
                     vector < double > gsm_par = par;
                     gsm_par.resize( this_n_par );

                     {
                        our_vector *v = new_our_vector( gsm_par.size() );
                  
                        for ( int ii = 0; ii < v->len; ii++ )
                        {
                           v->d[ ii ] = gsm_par[ ii ];
                        }
                        start_rmsd = compute_gsm_exponentials_f( v );

                        free_our_vector( v );
                     }

                     double r = drand48();
                     if ( r < .334 )
                     {
                        compute_gsm_exponentials( gsm_par, q, I, "ih" );
                     } else {
                        if ( r < .667 )
                        {
                           compute_gsm_exponentials( gsm_par, q, I, "cg" );
                        } else {
                           compute_gsm_exponentials( gsm_par, q, I, "sd" );
                        }
                     }

                     lmcurve_fit( ( int )      this_n_par,
                                  ( double * ) &( gsm_par[ 0 ] ),
                                  ( int )      m_dat,
                                  ( double * ) &( compute_exponential_t[ 0 ] ),
                                  ( double * ) &( compute_exponential_y[ 0 ] ),
                                  compute_exponential_f,
                                  (const lm_control_struct *)&control,
                                  &status );
                     r = drand48();
                     if ( r < .334 )
                     {
                        compute_gsm_exponentials( gsm_par, q, I, "ih" );
                     } else {
                        if ( r < .667 )
                        {
                           compute_gsm_exponentials( gsm_par, q, I, "cg" );
                        } else {
                           compute_gsm_exponentials( gsm_par, q, I, "sd" );
                        }
                     }

                     {
                        our_vector *v = new_our_vector( gsm_par.size() );
                  
                        for ( int ii = 0; ii < v->len; ii++ )
                        {
                           v->d[ ii ] = gsm_par[ ii ];
                        }
                        end_rmsd = compute_gsm_exponentials_f( v );

                        free_our_vector( v );
                     }
                     par = gsm_par;
                     if ( start_rmsd > end_rmsd )
                     {
                        cout << QString( "gsm repeat %1 for improvement, %2 %3\n" )
                           .arg( ++gsm_repeat )
                           .arg( start_rmsd )
                           .arg( end_rmsd );
                     }
                  } while ( start_rmsd > end_rmsd && gsm_repeat < 50 && end_rmsd / 500e0 < par_before_gsm_norm );

                  if ( end_rmsd > par_before_gsm_norm )
                  {
                     cout << "reverting to par before gsm norm\n";
                     par = par_before_gsm;
                     end_rmsd = par_before_gsm_norm;
                  }
               
                  if ( !i )
                  {
                     best_par  = par;
                     best_norm = end_rmsd;
                  } else {
                     if ( best_norm > end_rmsd )
                     {
                        best_par  = par;
                        best_norm = end_rmsd;
                     }
                  }
               }
            
               lmcurve_fit( ( int )      this_n_par,
                            ( double * ) &( par[ 0 ] ),
                            ( int )      m_dat,
                            ( double * ) &( compute_exponential_t[ 0 ] ),
                            ( double * ) &( compute_exponential_y[ 0 ] ),
                            compute_exponential_f,
                            (const lm_control_struct *)&control,
                            &status );

               if ( !i && !use_gsm_fitting )
               {
                  best_par  = par;
                  best_norm = status.fnorm;
               } else {
                  if ( best_norm > status.fnorm )
                  {
                     best_par  = par;
                     best_norm = status.fnorm;
                  }
               }
               cout << QString( " norm %1\n" ).arg( status.fnorm );
            }
         
            par          = best_par;
            status.fnorm = best_norm;
            cout << QString( " final norm %1\n" ).arg( status.fnorm );

            // save the best one in normv
            if ( !this_terms || normv > status.fnorm )
            {
               coeffv = par;
               normv  = status.fnorm;
            } 
            if ( this_terms == 4 )
            {
               coeff4 = par;
               norm4  = status.fnorm;
            }
            if ( this_terms == 5 )
            {
               coeff5 = par;
               norm5  = status.fnorm;
            }
            if ( this_terms >= 5 && status.fnorm < 0.005 )
            {
               cout << "early termination\n";
               break;
            }
         }
      }
   }      
   return true;
}

void US_Saxs_Util::compute_gsm_exponentials_setup( vector < double > &t,
                                                   vector < double > &y )
{
   compute_gsm_exponentials_mtto4pi2.resize( t.size() );
   compute_gsm_exponentials_y = y;
   double minus1over4pi2  = -1e0 / ( 16e0 * M_PI * M_PI );
   for ( unsigned int i = 0; i < t.size(); i++ )
   {
      compute_gsm_exponentials_mtto4pi2[ i ] = t[ i ] * t[ i ] * minus1over4pi2;
   }
}

bool US_Saxs_Util::compute_gsm_exponentials( vector < double > &par,
                                             vector < double > &t,
                                             vector < double > &y,
                                             QString           method )
{
   compute_gsm_exponentials_setup( t, y );

   compute_gsm_exponentials_is_on = true;

   double gsmepsilon    = 1e-4;
   double gsmiterations = 25;

   nsa_gsm_delta    = 1; 
   nsa_gsm_delta2_r = 1e0 / ( 2e0 * nsa_gsm_delta );

   global_iter = 0;

   our_vector *v = new_our_vector( par.size() );
   
   for ( int i = 0; i < v->len; i++ )
   {
      v->d[ i ] = par[ i ];
   }

   int gsm_method = -1;

   if ( method == "cg" )
   {
      gsm_method = CONJUGATE_GRADIENT;
   }
   if ( method == "sd" )
   {
      gsm_method = STEEPEST_DESCENT;
   }
   if ( method == "ih" )
   {
      gsm_method = INVERSE_HESSIAN;
   }

   // print_our_vector( v ); 
   double org_norm = compute_gsm_exponentials_norm = nsa_gsm_f( v );
   cout << QString( "start gsm method %1 org norm %2\n" ).arg( method ).arg( org_norm );

   switch( gsm_method ) {
   case CONJUGATE_GRADIENT :
      nsa_min_fr_pr_cgd   ( v,
                            gsmepsilon,
                            gsmiterations );
      break;
   case STEEPEST_DESCENT :      
      nsa_min_gsm_5_1     ( v,
                            gsmepsilon,
                            gsmiterations );
      break;
   case INVERSE_HESSIAN :  
      nsa_min_hessian_bfgs( v,
                            gsmepsilon,
                            gsmiterations );
      break;
   default : 
      {
         errormsg = QString( "unknown gsm method %1 " ).arg( gsm_method );
         free_our_vector(v); 
         return false; 
      }
      break;
   }

   compute_gsm_exponentials_norm = nsa_gsm_f( v );
   cout << QString( "after gsm norm %1\n" ).arg( compute_gsm_exponentials_norm );

   if ( compute_gsm_exponentials_norm < org_norm )
   {
      for ( int i = 0; i < v->len; i++ )
      {
         par[ i ] = v->d[ i ];
      }
   } else {
      cout << "gsm made fitness worse, reverting\n";
      compute_gsm_exponentials_norm = org_norm;
   }
   //    print_our_vector( v ); 
   free_our_vector( v );

   compute_gsm_exponentials_is_on = false;
   return true;
}

double US_Saxs_Util::compute_gsm_exponentials_f( our_vector *v )
{
   // compute curves then determine rmsd 
   // cout << "compute_gsm_exponentials_f:" << flush;
   //   print_our_vector( v );
   vector < double > cy( compute_gsm_exponentials_y.size() );

   for ( unsigned int j = 0; j < compute_gsm_exponentials_mtto4pi2.size(); j++ )
   {
      cy[ j ] = v->d[ 0 ];
      for ( unsigned int i = 1; i < ( unsigned int ) v->len - 1; i += 2 )
      {
         cy[ j ] += v->d[ i ] * exp( v->d[ i + 1 ] * compute_gsm_exponentials_mtto4pi2[ j ] );
      }
   }

   //    double rmsd = 0e0;
   //    for ( unsigned int j = 0; j < cy.size(); j++ )
   //    {
   //       double d = cy[ j ] - compute_gsm_exponentials_y[ j ];
   //       rmsd += d * d;
   //    }
   // cout << QString( "rmsd %1\n" ).arg( sqrt( rmsd ) );
   // return sqrt( rmsd );
   
   vector < double > x( cy.size() );
   for ( unsigned int j = 0; j < cy.size(); j++ )
   {   
      x[ j ] = fabs( cy[ j ] - compute_gsm_exponentials_y[ j ] );
   }
   double lmnorm = LM::lm_enorm( x.size(), &( x[0] ) );
   return lmnorm;
}


bool US_Saxs_Util::set_excluded_volume( 
                                       PDB_atom                 &this_atom, 
                                       double                   &vol, 
                                       double                   &scaled_vol, 
                                       saxs_options             &our_saxs_options, 
                                       map < QString, QString > &residue_atom_hybrid_map,
                                       unsigned int             &total_electrons,
                                       unsigned int             &total_electrons_noh,
                                       double                   &scattering_intensity
                                       )
{
   errormsg = "";

   this->our_saxs_options = our_saxs_options;

   QString mapkey = QString("%1|%2").arg(this_atom.resName).arg(this_atom.name);
   if ( this_atom.name == "OXT" )
   {
      mapkey = "OXT|OXT";
   }
   QString hybrid_name = residue_atom_hybrid_map[mapkey];

   if ( hybrid_name.isEmpty() || !hybrid_name.length() )
   {
      errormsg = QString("Warning: hybrid name missing for %1|%2, not added to excluded volume").arg(this_atom.resName).arg(this_atom.name);
      return false;
   }

   if ( !hybrid_map.count(hybrid_name) )
   {
      errormsg = QString("Warning: hybrid_map name missing for hybrid_name %1, not added to excluded volume").arg(hybrid_name);
      return false;
   }

   if ( !atom_map.count(this_atom.name + "~" + hybrid_name) )
   {
      errormsg = QString("Warning: atom_map missing for hybrid_name %1 atom name %2, not added to excluded volume").arg(hybrid_name).arg(this_atom.name);
      return false;
   }

   double use_vol  = atom_map[this_atom.name + "~" + hybrid_name].saxs_excl_vol;
   total_electrons = hybrid_map[ hybrid_name ].num_elect;
   if ( this_atom.name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
   {
      use_vol = our_saxs_options.swh_excl_vol;
   }
   vol = use_vol;
   if ( our_saxs_options.hybrid_radius_excl_vol )
   {
      use_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
   }
   if ( this_atom.name == "OW" )
   {
      scaled_vol  = use_vol;
      total_electrons_noh = hybrid_map[ hybrid_name ].num_elect;
   } else {
      scaled_vol = use_vol * our_saxs_options.scale_excl_vol;
   } 

   scattering_intensity = 0e0;
   QRegularExpression count_hydrogens("H(\\d)");
   unsigned int hydrogens = 0;
   QRegularExpressionMatch count_hydrogens_m = count_hydrogens.match(hybrid_name);
   if ( count_hydrogens_m.hasMatch() )
   {
      hydrogens = count_hydrogens_m.captured(1).toInt();
   }
   saxs saxsH;
   if ( hydrogens && !saxs_map.count( "H" ) )
   {
      errormsg = "Warning: no SAXS H defined, so no hydrogens included";
      hydrogens = 0;
   } else {
      saxsH = saxs_map[ "H" ];
   }
      
   saxsH = saxs_map[ "H" ];
   scattering_intensity = compute_ff( this_atom.saxs_data,
                                      saxsH,
                                      this_atom.resName,
                                      this_atom.saxs_name,
                                      this_atom.name,
                                      hydrogens,
                                      0e0,
                                      0e0 );
   scattering_intensity *= scattering_intensity;
   // cout << QString( "compute saxs for atom %1 si %2\n" ).arg( this_atom.name ).arg( scattering_intensity );
   return true;
}

// returns a bead model with dR elements
bool US_Saxs_Util::grid( 
                        vector < PDB_atom > &bm,
                        vector < PDB_atom > &result,
                        double              dR, 
                        bool                accum_overlaps,  // if true, beads will increase in size
                        bool                allow_partials   // if true attempt "partial" smaller beads for edges
                        )
{
   vector < PDB_atom > results;

   errormsg = "";

   if ( !bm.size() )
   {
      errormsg = "error: grid(): empty bead model";
      return false;
   }

   if ( allow_partials )
   {
      errormsg = "error: grid(): allow partials not yet implemented";
      return false;
   }
      
   result.clear( );

   // determine max extents

   point pmin = bm[ 0 ].bead_coordinate;
   point pmax = bm[ 0 ].bead_coordinate;

   for ( unsigned int j = 0; j < 3; j++ )
   {
      pmin.axis[ j ] -= bm[ 0 ].bead_computed_radius;
      pmax.axis[ j ] += bm[ 0 ].bead_computed_radius;
   }

   float total_mw = bm[ 0 ].bead_ref_mw;

   for ( unsigned int i = 1; i < ( unsigned int ) bm.size(); i++ )
   {
      total_mw += bm[ i ].bead_ref_mw;
      for ( unsigned int j = 0; j < 3; j++ )
      {
         if ( pmin.axis[ j ] > bm[ i ].bead_coordinate.axis[ j ] - bm[ i ].bead_computed_radius )
         {
            pmin.axis[ j ] = bm[ i ].bead_coordinate.axis[ j ] - bm[ i ].bead_computed_radius;
         }
         if ( pmax.axis[ j ] < bm[ i ].bead_coordinate.axis[ j ] + bm[ i ].bead_computed_radius )
         {
            pmax.axis[ j ] = bm[ i ].bead_coordinate.axis[ j ] + bm[ i ].bead_computed_radius;
         }
      }
   }

   cout << QString( "extents min" ) << pmin << endl;
   cout << QString( "extents max" ) << pmax << endl;
   cout << QString( "Total mw %1\n" ).arg( total_mw );

   // compute dR

   if ( dR <= 0e0 )
   {
      // look at cube root of volume
      double box_vol = 1e0;
      for ( unsigned int j = 0; j < 3; j++ )
      {
         box_vol *= pmax.axis[ j ] - pmin.axis[ j ];
      }
      // say max 100000 to fill assuming 50% or less (?)
      double ele_vol = box_vol / 100000e0;
      dR = pow( ele_vol, 1e0/3e0 );
      if ( dR < 1e0 )
      {
         dR = 1e0;
      }
   }

   double vol    = dR * dR * dR;
   double radius = pow( vol * 3e0 / ( 4e0 * M_PI ), 1e0 / 3e0 );  // 4/3 pi r^3
   cout << QString( "dR %1 vol %2 radius %3\n" ).arg( dR ).arg( vol ).arg( radius );

   point pgrid;
   unsigned int total_close_count = 0;
   
   for ( pgrid.axis[ 0 ] = pmin.axis[ 0 ]; pgrid.axis[ 0 ] <= pmax.axis[ 0 ]; pgrid.axis[ 0 ] += dR )
   {
      for ( pgrid.axis[ 1 ] = pmin.axis[ 1 ]; pgrid.axis[ 1 ] <= pmax.axis[ 1 ]; pgrid.axis[ 1 ] += dR )
      {
         for ( pgrid.axis[ 2 ] = pmin.axis[ 2 ]; pgrid.axis[ 2 ] <= pmax.axis[ 2 ]; pgrid.axis[ 2 ] += dR )
         {
            unsigned int close_count = 0;
            for ( unsigned int i = 0; i < ( unsigned int ) bm.size(); i++ )
            {
               if ( dist( pgrid, bm[ i ].bead_coordinate ) <= bm[ i ].bead_computed_radius )
               {
                  close_count++;
                  if ( !accum_overlaps )
                  {
                     close_count = 1;
                     break;
                  }
               }
            }
            if ( close_count )
            {
               PDB_atom tmp_atom;
               tmp_atom.bead_coordinate      = pgrid;
               tmp_atom.bead_computed_radius = radius * ( float ) close_count;

               // overloading:
               tmp_atom.accessibility        = close_count;
               tmp_atom.exposed_code         = 1;
               tmp_atom.bead_color           = 8;

               // set to null values just in case
               tmp_atom.bead_recheck_asa     = 0.0;

               tmp_atom.serial = ( unsigned int )result.size() + 1;
               
               result.push_back( tmp_atom );
               total_close_count += close_count;
            }
         }
      }
   }
      
   // set mw's

   float mw_per_count = total_mw / total_close_count;
   cout << QString( "mw per count %1 (total %2 count %3) total beads %4\n" )
      .arg( mw_per_count )
      .arg( total_mw )
      .arg( total_close_count )
      .arg( result.size() )
      ;

   for ( unsigned int i = 0; i < ( unsigned int ) result.size(); i++ )
   {
      result[ i ].bead_ref_mw = mw_per_count * result[ i ].accessibility;
      result[ i ].bead_mw     = result[ i ].bead_ref_mw;
   }

   return true;
}

bool US_Saxs_Util::smooth(
                          vector < double > &x,
                          vector < double > &result,
                          unsigned int      points,
                          bool              smooth_endpoints
                          )
{
   errormsg = "";
   if ( !x.size() )
   {
      errormsg = "smooth(): empty vector";
      return false;
   }

   if ( !points )
   {
      result = x;
      return true;
   }

   // get normal distribution

   vector < double > dist;
   points++;
   for ( unsigned int i = 0; i <= points; i++ ) 
   {
      double x0 = ( 2e0 / points ) * ( double ) i;
      dist.push_back( exp( -( x0 * x0 ) ) / sqrt( 2e0 * 3.14159265358978323e0 ) );
   }
   double tot = dist[ 0 ];
   for ( unsigned int j = 1; j < dist.size(); j++ ) 
   {
      tot += 2 * dist[ j ];
   }
   for ( unsigned int j = 1; j < dist.size(); j++ ) 
   {
      dist[ j ] /= tot;
   }

   // do smoothing

   vector < double > y ( x.size() );
   vector < double > yn( x.size() );
   result.resize       ( x.size() );
         
   for( unsigned int i = 0; i < x.size(); i++ )
   {
      y[ i ] = x[ i ];
   }
            
   for( unsigned int i = 0; i < x.size(); i++ )
   {
      yn[ i ]      = 0e0;
      double tdist = 0e0;
      for( int j = -( int ) points; j <= ( int ) points; j++ ) 
      {
         int jp = ( int ) i + j;
         if ( jp >= 0 && jp < ( int ) y.size() ) 
         {
            int uj = j;
            if ( j < 0 ) 
            {
               uj = -j;
            }
            yn[ i ] +=  y[ jp ] * dist[ uj ];
            tdist += dist[ uj ];
         }
      }
      yn[ i ] /= tdist;
   }

   {
      unsigned int low  = 0;
      unsigned int high = x.size();
      if ( !smooth_endpoints )
      {
         low++;
         high--;
         result[ 0 ] = x[ 0 ];
         result[ high ] = x[ high ];
      }
      for( unsigned int i = low; i < high; i++ )
      {
         result[ i ] = yn[ i ];
      }
   }
   return true;
}
