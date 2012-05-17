#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

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
