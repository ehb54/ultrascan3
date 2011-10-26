#include "../include/us_saxs_util.h"

#define SAXS_MIN_Q 1e-6
#define SLASH QDir::separator();


static bool save_calc_to_csv = false;

bool US_Saxs_Util::calc_saxs_iq_native_fast()
{
   errormsg = "";
   noticemsg = "";
   
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   QRegExp count_hydrogens("H(\\d)");
   
   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      unsigned int current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
      
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];
            
            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }
            
            QString hybrid_name = residue_atom_hybrid_map[mapkey];
            
            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq);
               continue;
            }
            
            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }
            
            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->name)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }
            
            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }
            // cout << QString("in %1 hydrogens %2\n").arg( hybrid_name ).arg( new_atom.hydrogens );
            
            // this is probably correct but FoXS uses the saxs table excluded volume
            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options.swh_excl_vol;
            }
            if ( our_saxs_options.hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }
            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options.scale_excl_vol;
               tot_excl_vol_noh  += new_atom.excl_vol;
               total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }
            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name);
               continue;
            } else {
               // for FoXS method
               // new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

            atoms.push_back(new_atom);
         }
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
      float vi; // excluded water vol
      float vie; // excluded water * e density

      // compute form factors
      saxs saxsH = saxs_map["H"];
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         // note: since there are only a few 'saxs' coefficient sets
         // the our_saxs_options.c + our_saxs_options.a[i] * exp() can be precomputed
         // possibly saving time... but this isn't our most computationally intensive step
         // so I'm holding off for now.
         
         f[0][i] = 
            saxs.c +
            saxs.a[0] +
            saxs.a[1] +
            saxs.a[2] +
            saxs.a[3] +
            atoms[i].hydrogens * 
            ( saxsH.c +
              saxsH.a[0] +
              saxsH.a[1] +
              saxsH.a[2] +
              saxsH.a[3] );

         fc[0][i] = vie;
         fp[0][i] = f[0][i] - fc[0][i];
      }

      // foxs method: compute real space distribution

      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double rik2; // square distance from atom i to k 
      float delta = our_saxs_options.fast_bin_size;
      float one_over_delta = 1.0 / delta;
      unsigned int pos;
      vector < float > hist;
      vector < float > hist_pr;
      
#if defined( DO_COMPUTE_PR )
      unsigned int pos_pr;
      float one_over_delta_pr = 1.0 / delta_pr;
      float delta_pr = our_saxs_options.bin_size;
      contrib_array.clear();
      contrib_file = control_parameters[ "inputfile" ];
      // cout << "contrib_file " << contrib_file << endl;

      if ( control_parameters.count( "fdcomputepr" ) )
      {
         if ( cb_pr_contrib->isChecked() &&
              !source &&
              contrib_file.contains(QRegExp("(PDB|pdb)$")) )
         {
            contrib_array.resize(atoms.size());
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
                  rik = sqrt( rik2 );
                  if ( our_saxs_options.subtract_radius )
                  {
                     rik = rik - atoms[i].radius - atoms[k].radius;
                     if ( rik < 0e0 )
                     {
                        rik = 0e0;
                     }
                     rik2 = rik * rik;
                  }

                  pos = (unsigned int)floor(rik2 * one_over_delta);
                  pos_pr = (unsigned int)floor(rik * one_over_delta_pr);
                  
                  if ( hist.size() <= pos )
                  {
                     hist.resize(pos + 1024);
                  }
                  if ( hist_pr.size() <= pos_pr )
                  {
                     hist_pr.resize(pos_pr + 1024);
                     for ( unsigned int l = 0; l < atoms.size(); l++ )
                     {
                        contrib_array[l].resize(pos_pr + 1024);
                     }
                  }
                  hist_pr[pos_pr] += 2.0 * fp[0][i] * fp[0][k];
                  
                  hist[pos] += 2.0 * fp[0][i] * fp[0][k];
                  contrib_array[i][pos] += fp[0][i];
                  contrib_array[k][pos] += fp[0][k];
               }
               if ( our_saxs_options.autocorrelate )
               {
                  hist[0] += fp[0][i] * fp[0][i];
               }
            }
            if ( our_saxs_options.autocorrelate )
            {
               hist[0] += fp[0][as1] * fp[0][as1];
            }
         } else {
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
                  rik = sqrt( rik2 );
                  if ( our_saxs_options.subtract_radius )
                  {
                     rik = rik - atoms[i].radius - atoms[k].radius;
                     if ( rik < 0e0 )
                     {
                        rik = 0e0;
                     }
                     rik2 = rik * rik;
                  }

                  pos = (unsigned int)floor(rik2 * one_over_delta);
                  pos_pr = (unsigned int)floor(rik * one_over_delta_pr);
                  
                  if ( hist.size() <= pos )
                  {
                     hist.resize(pos + 1024);
                  }
                  hist[pos] += 2.0 * fp[0][i] * fp[0][k];

                  if ( hist_pr.size() <= pos_pr )
                  {
                     hist_pr.resize(pos_pr + 1024);
                  }
                  hist_pr[pos_pr] += 2.0 * fp[0][i] * fp[0][k];
               }
               if ( our_saxs_options.autocorrelate )
               {
                  hist[0] += fp[0][i] * fp[0][i];
               }
            }
            if ( our_saxs_options.autocorrelate )
            {
               hist[0] += fp[0][as1] * fp[0][as1];
            }
         }         
         while( hist_pr.size() && !hist_pr[hist_pr.size()-1] ) 
         {
            hist_pr.pop_back();
         }
      } else 
#endif // for compute pr
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
               if ( our_saxs_options.subtract_radius )
               {
                  rik = sqrt( rik2 );
                  rik = rik - atoms[i].radius - atoms[k].radius;
                  if ( rik < 0e0 )
                  {
                     rik = 0e0;
                  }
                  rik2 = rik * rik;
               }

               pos = (unsigned int)floor(rik2 * one_over_delta);
               
               if ( hist.size() <= pos )
               {
                  hist.resize(pos + 1024);
               }
               hist[pos] += 2.0 * fp[0][i] * fp[0][k];
            }
            if ( our_saxs_options.autocorrelate )
            {
               hist[0] += fp[0][i] * fp[0][i];
            }
         }
         if ( our_saxs_options.autocorrelate )
         {
            hist[0] += fp[0][as1] * fp[0][as1];
         }
      }

      while( hist.size() && !hist[hist.size()-1] ) 
      {
         hist.pop_back();
      }
#if defined( DO_COMPUTE_PR )
      if ( contrib_array.size() ) 
      {
         for ( unsigned int k = 0; k < contrib_array.size(); k++ )
         {
            contrib_array[k].resize(hist.size());
         }
      }
#endif

      noticemsg += QString("Total excluded volume %1\n").arg(tot_excl_vol);
      noticemsg += QString("Average electron density %1\n").arg(total_e / tot_excl_vol, 4);
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         noticemsg += QString("Total unhydrated excluded volume %1\n").arg(tot_excl_vol_noh);
         noticemsg += QString("Average unhydrated electron density %1\n").arg(total_e_noh / tot_excl_vol_noh);
         noticemsg += QString("Electron density of hydration %1\n").arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh));
      }

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(control_parameters[ "inputfile" ]).fileName())
         .arg(current_model + 1)
         .arg(iqq_suffix());

      // save the data to a file
#if defined( DO_COMPUTE_PR )
      if (  control_parameters.count( "fdcomputepr" ) )
      {
         if ( create_native_saxs )
         {
            QString fpr_name = 
               USglobal->config_list.root_dir + 
               SLASH + "somo" + SLASH + "saxs" + SLASH + sprr_filestring();
            
            bool ok_to_write = true;
            
            if ( ok_to_write )
            {
               FILE *fpr = fopen(fpr_name, "w");
               if ( fpr ) 
               {
                  output_files << fpr_name;
                  noticemsg += "P(r) curve file: " + fpr_name + " created.\n";
                  vector < double > r;
                  vector < double > pr;
                  vector < double > pr_n;
                  r.resize(hist_pr.size());
                  pr.resize(hist_pr.size());
                  pr_n.resize(hist.size());
                  for ( unsigned int i = 0; i < hist_pr.size(); i++) 
                  {
                     r[i] = i * delta_pr;
                     pr[i] = (double) hist_pr[i];
                     pr_n[i] = (double) hist_pr[i];
                  }
                  normalize_pr(r, &pr_n, get_mw(control_parameters[ "inputfile" ], false));
                  QString last_saxs_header =
                     QString("")
                     .sprintf(
                              "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                              , control_parameters[ "inputfile" ].ascii()
                              , US_Version.ascii()
                              , REVISION
                              , delta
                              , get_mw(control_parameters[ "inputfile" ], false)
                              , compute_pr_area(pr, r)
                              );
                  fprintf(fpr, "%s",
                          last_saxs_header.ascii() );
                  fprintf(fpr, "r\tp(r)\tnorm. p(r)\n");
                  for ( unsigned int i = 0; i < hist_pr.size(); i++ )
                  {
                     if ( hist_pr[i] ) {
                        fprintf(fpr, "%.6e\t%.6e\t%.6e\n", r[i], pr[i], pr_n[i]);
                     }
                  }
                  fclose(fpr);
               }
               else
               {
                  noticemsg += "WARNING: Could not create PR curve file: " + fpr_name + "\n";
               }
            }
         } else {
            vector < double > r;
            vector < double > pr;
            vector < double > pr_n;
            r.resize(hist_pr.size());
            pr.resize(hist_pr.size());
            pr_n.resize(hist_pr.size());
            for ( unsigned int i = 0; i < hist_pr.size(); i++) 
            {
               r[i] = i * delta_pr;
               pr[i] = (double) hist_pr[i];
               pr_n[i] = (double) hist_pr[i];
            }
            normalize_pr(r, &pr_n, get_mw(control_parameters[ "inputfile" ], false));
            QString last_saxs_header =
               QString("")
               .sprintf(
                        "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                        , control_parameters[ "inputfile" ].ascii()
                        , US_Version.ascii()
                        , REVISION
                        , delta
                        , get_mw(control_parameters[ "inputfile" ], false)
                        , compute_pr_area(pr, r)
                        );
         }

         vector < double > r;
         vector < double > pr;
         r.resize(hist_pr.size());
         pr.resize(hist_pr.size());
         for ( unsigned int i = 0; i < hist_pr.size(); i++) 
         {
            r[i] = i * delta_pr;
            pr[i] = (double) hist_pr[i];
         }
         plotted_pr_not_normalized.push_back(pr);
         plotted_pr_mw.push_back((float)get_mw(control_parameters[ "inputfile" ], false));
         if ( cb_normalize->isChecked() )
         {
            normalize_pr(r, &pr, get_mw(control_parameters[ "inputfile" ],false));
         }

         plot_one_pr( r, pr, control_parameters[ "inputfile" ] );
         
      } // compute_pr
#endif

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
      if ( !write_output( current_model, q, I ) )
      {
         return false;
      }
   } // models
   return true;
}

//  ------------------------------------------------------------------------------------------------------
bool US_Saxs_Util::calc_saxs_iq_native_debye()
{
   errormsg = "";
   noticemsg = "";

   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      unsigned int current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq);
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom->name)
                  .arg(this_atom->resName)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            cout << QString("atom %1 hybrid %1 excl vol %1 by hybrid radius %1\n")
               .arg(this_atom->name)
               .arg(this_atom->hybrid_name)
               .arg(atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol)
               .arg(M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius)
               ;


            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options.swh_excl_vol;
            }
            if ( our_saxs_options.hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
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
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name);
               continue;
            }

            atoms.push_back(new_atom);
         }
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
      
      // double m_pi_vi23; // - pi * pow(v,2/3)
      double vi_23_4pi;
      float vi; // excluded water vol
      float vie; // excluded water * e density

      saxs saxsH = saxs_map["H"];
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;
         
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            // note: since there are only a few 'saxs' coefficient sets
            // the our_saxs_options.c + our_saxs_options.a[i] * exp() can be precomputed
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
      if ( save_calc_to_csv )
      {
         QString out = ",,,,\"q:\",";
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            out += QString("%1,").arg(q[i]);
         }
         out += "\n\n\n\"f':\"\n";
         out += 
            "\"number\","
            "\"name\","
            "\"hybrid name\","
            "\"excluded volume\","
            "\"number of hydrogens\","
            "\"structure factors\","
            "\n";

         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            out += 
               QString(
                       "%1,"
                       "\"%1\","
                       "\"%1\","
                       "%1,"
                       "%1,"
                       )
               .arg(i + 1)
               .arg(atoms[i].saxs_name)
               .arg(atoms[i].hybrid_name)
               .arg(atoms[i].excl_vol)
               .arg(atoms[i].hydrogens)
               ;
            for ( unsigned int j = 0; j < q.size(); j++ )
            {
               out += QString("%1,").arg(fp[j][i]);
            }
            out += "\n";
         }

         QFile f( "last_iqq.csv" );
         if ( f.open( IO_WriteOnly ) )
         {
            QTextStream t( &f );
            t << out;
            f.close();
            noticemsg += QString("created %1\n").arg(f.name());
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
            if ( our_saxs_options.subtract_radius )
            {
               rik = rik - atoms[i].radius - atoms[k].radius;
               if ( rik < 0e0 )
               {
                  rik = 0e0;
               }
            }

            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * q[j];
               sqrikd = ( fabs(qrik) < 1e-30 ) ? 1.0 : sin(qrik) / qrik;
               I[j] += 2.0 * fp[j][i] * fp[j][k] * sqrikd;
               Ia[j] += 2.0 * f[j][i] * f[j][k] * sqrikd;
               Ic[j] += 2.0 * fc[j][i] * fc[j][k] * sqrikd;
            } // j
         } // k
         if ( our_saxs_options.autocorrelate )
         {
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[j] += fp[j][i] * fp[j][i];
               Ia[j] += f[j][i] * f[j][i];
               Ic[j] += fc[j][i] * fc[j][i];
            }
         }
      }
      if ( our_saxs_options.autocorrelate )
      {
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            I[j] += fp[j][as1] * fp[j][as1];
            Ia[j] += f[j][as1] * f[j][as1];
            Ic[j] += fc[j][as1] * fc[j][as1];
         }
      }

      //for ( unsigned int j = 0; j < q_points; j++ )
      //      {
      //         I[j] = I[j] > 0.0 ? log10(I[j]) : 0.0;
      //         Ia[j] = Ia[j] > 0.0 ? log10(Ia[j]) : 0.0;
      //         Ic[j] = Ic[j] > 0.0 ? log10(Ic[j]) : 0.0;
      //      }

      noticemsg += "I(q) computed.\n";

      // save the data to a file
      if ( !write_output( current_model, q, I ) )
      {
         return false;
      }
   }
   return true;
}

//  ------------------------------------------------------------------------------------------------------
bool US_Saxs_Util::calc_saxs_iq_native_hybrid()
{
   errormsg = "";
   noticemsg = "";

   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      unsigned int current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(current_model + 1);
         
      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
            if ( this_atom->name == "OXT" )
            {
               mapkey = "OXT|OXT";
            }

            QString hybrid_name = residue_atom_hybrid_map[mapkey];

            if ( hybrid_name.isEmpty() || !hybrid_name.length() )
            {
               cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq);
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options.swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options.swh_excl_vol;
            }
            if ( our_saxs_options.hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
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
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name);
               continue;
            }

            atoms.push_back(new_atom);
         }
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
      double vi_23_4pi;
      float vi; // excluded water vol
      float vie; // excluded water * e density

      saxs saxsH = saxs_map["H"];
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options.water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;
         
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
      
      // vector < vector < double > > rik_array(as);

      // for ( unsigned int i = 0; i < rik_array.size(); i++ )
      // {
      //    rik_array[i].resize(atoms.size());
      // }

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
            rik = sqrt( rik2 );
            // rik_array[i][k] = sqrt(rik2);
            if ( our_saxs_options.subtract_radius )
            {
               rik = rik - atoms[i].radius - atoms[k].radius;
               if ( rik < 0e0 )
               {
                  rik = 0e0;
               }
               rik2 = rik * rik;
            }
            // rik_array[i][k] = rik;

            pos = (unsigned int)floor(rik2 * one_over_delta);
            
            if ( hist.size() <= pos )
            {
               hist.resize(pos + 1024);
            }
            hist[pos] += 2.0 * fp[0][i] * fp[0][k];
         }
         if ( our_saxs_options.autocorrelate )
         {
            hist[0] += fp[0][i] * fp[0][i];
         }
      }
      if ( our_saxs_options.autocorrelate )
      {
         hist[0] += fp[0][as1] * fp[0][as1];
      }
      
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
            I[i] += hist[j] * (( fabs(x) < 1e-16 ) ? 1.0 : sin(x) / x);
         }
         I[i] *= exp( - our_saxs_options.fast_modulation * q[i] * q[i] );
      }

      // -------------------------------
      vector < unsigned int > r;
      bool adaptive_ok = false;

      if ( our_saxs_options.saxs_iq_hybrid_adaptive )
      {
         QString save_noticemsg = noticemsg;
         if ( !create_adaptive_grid( q, I, our_saxs_options.hybrid2_q_points, r ) )
         {
            noticemsg += "Error:" + errormsg;
         } else {
            adaptive_ok = true;
         }
         noticemsg = save_noticemsg + noticemsg;
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
                       (atoms[i].pos[2] - atoms[k].pos[2]) );

            for ( unsigned int l = 0; l < use_q_size; l++ )
            {
               unsigned int j = use_q[l];
               qrik = rik * q[j];
               sqrikd = ( fabs(qrik) < 1e-16 ) ? 1.0 : sin(qrik) / qrik;
               I[j] += 2.0 * fp[j][i] * fp[j][k] * sqrikd;
            } // j
         } // k
         if ( our_saxs_options.autocorrelate )
         {
            for ( unsigned int l = 0; l < use_q_size; l++ )
            {
               unsigned int j = use_q[l];
               I[j] += fp[j][i] * fp[j][i];
            }
         }
      }
      vector < double > deltaI(use_q_size);
      for ( unsigned int l = 0; l < use_q_size; l++ )
      {
         unsigned int j = use_q[l];
         if ( our_saxs_options.autocorrelate )
         {
            I[j] += fp[j][as1] * fp[j][as1];
         }
         deltaI[l] = I[j] - fast_I[j];
      }

      {
         QString save_errormsg = errormsg;
         if ( our_saxs_options.saxs_iq_native_hybrid )
         {
            if ( !linear_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
            {
               cout << errormsg << endl;
               return false;
            }
         } else {
            if ( our_saxs_options.saxs_iq_native_hybrid2 )
            {
               if ( !quadratic_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << errormsg << endl;
                  return false;
               }
            } else {
               if ( !cubic_spline_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << errormsg << endl;
                  return false;
               }
            }               
         }
         errormsg = save_errormsg;
      }

      noticemsg += "I(q) computed.\n";

      if ( !write_output( current_model, q, I ) )
      {
         return false;
      }
   }
   return true;
}

QString US_Saxs_Util::iqq_suffix()
{
   QString qs;

   if ( !our_saxs_options.saxs_sans )
   {
      if ( our_saxs_options.saxs_iq_crysol )
      {
         qs += "cr";
         qs += QString("_h%1_g%2_hs%3")
            .arg( our_saxs_options.crysol_max_harmonics )
            .arg( our_saxs_options.crysol_fibonacci_grid_order )
            .arg( QString("%1").arg( our_saxs_options.crysol_hydration_shell_contrast ).replace(".", "_" ) );
      } else {
         if ( our_saxs_options.saxs_iq_foxs )
         {
            qs += "fx";
         } else {
            if ( our_saxs_options.saxs_iq_native_debye )
            {
               qs += "db";
            }
            if ( our_saxs_options.saxs_iq_native_fast )
            {
               qs += "fd";
            }
            if ( our_saxs_options.saxs_iq_native_hybrid )
            {
               qs += "hy";
            }
            if ( our_saxs_options.saxs_iq_native_hybrid2 )
            {
               qs += "h2";
            }
            if ( our_saxs_options.saxs_iq_native_hybrid3 )
            {
               qs += "h3";
            }
            if ( ( our_saxs_options.saxs_iq_native_hybrid ||
                   our_saxs_options.saxs_iq_native_hybrid2 ||
                   our_saxs_options.saxs_iq_native_hybrid3 ) && 
                 our_saxs_options.saxs_iq_hybrid_adaptive )
            {
               qs += "a";
            }
            if ( our_saxs_options.scale_excl_vol != 1e0 )
            {
               qs += QString("_evs%1")
                  .arg( QString("%1").arg( our_saxs_options.scale_excl_vol ).replace(".", "_" ) );
            }
            if ( !our_saxs_options.autocorrelate )
            {
               qs += "_nac";
            }
            if ( our_saxs_options.swh_excl_vol != 0e0 )
            {
               qs += QString("_swh%1")
                  .arg( QString("%1").arg( our_saxs_options.swh_excl_vol ).replace(".", "_" ) );
            }
         }
      }
   }
   return qs.length() ? ( "-" + qs ) : "";
}

void US_Saxs_Util::setup_saxs_options()
{
   cout << "setup saxs options\n";
   our_saxs_options.water_e_density = 0.334f; // water electron density in e/A^3

   our_saxs_options.h_scat_len = -0.3742f;        // H scattering length (*10^-12 cm)
   our_saxs_options.d_scat_len = 0.6671f ;        // D scattering length (*10^-12 cm)
   our_saxs_options.h2o_scat_len_dens = -0.562f;  // H2O scattering length density (*10^-10 cm^2)
   our_saxs_options.d2o_scat_len_dens = 6.404f;   // D2O scattering length density (*10^-10 cm^2)
   our_saxs_options.d2o_conc = 0.16f;             // D2O concentration (0 to 1)
   our_saxs_options.frac_of_exch_pep = 0.1f;      // Fraction of exchanged peptide H (0 to 1)
   
   our_saxs_options.wavelength = 1.5;         // scattering wavelength
   our_saxs_options.start_angle = 0.014f;     // start angle
   our_saxs_options.end_angle = 8.214f;       // ending angle
   our_saxs_options.delta_angle = 0.2f;       // angle stepsize
   our_saxs_options.max_size = 40.0;          // maximum size (A)
   our_saxs_options.bin_size = 1.0f;          // Bin size (A)
   our_saxs_options.hydrate_pdb = false;      // Hydrate the PDB model? (true/false)
   our_saxs_options.curve = 0;                // 0 = raw, 1 = saxs, 2 = sans
   our_saxs_options.saxs_sans = 0;            // 0 = saxs, 1 = sans
   
   our_saxs_options.guinier_csv = false;
   our_saxs_options.guinier_csv_filename = "guinier";
   our_saxs_options.qRgmax = 1.3e0;
   our_saxs_options.qstart = 1e-7;
   our_saxs_options.qend = .5e0;
   our_saxs_options.pointsmin = 10;
   our_saxs_options.pointsmax = 100;
   
   our_saxs_options.path_load_saxs_curve = "";
   our_saxs_options.path_load_gnom = "";
   our_saxs_options.path_load_prr = "";

   our_saxs_options.saxs_iq_native_debye = false;
   our_saxs_options.saxs_iq_native_hybrid = false;
   our_saxs_options.saxs_iq_native_hybrid2 = false;
   our_saxs_options.saxs_iq_native_hybrid3 = true;
   our_saxs_options.saxs_iq_native_fast = false;
   our_saxs_options.saxs_iq_native_fast_compute_pr = false;
   our_saxs_options.saxs_iq_foxs = false;
   our_saxs_options.saxs_iq_crysol = false;

   our_saxs_options.sans_iq_native_debye = true;
   our_saxs_options.sans_iq_native_hybrid = false;
   our_saxs_options.sans_iq_native_hybrid2 = false;
   our_saxs_options.sans_iq_native_hybrid3 = false;
   our_saxs_options.sans_iq_native_fast = false;
   our_saxs_options.sans_iq_native_fast_compute_pr = false;
   our_saxs_options.sans_iq_cryson = false;

   our_saxs_options.hybrid2_q_points = 15;

   our_saxs_options.iq_ask = false;

   our_saxs_options.iq_scale_ask = false;
   our_saxs_options.iq_scale_angstrom = true;
   our_saxs_options.iq_scale_nm = false;

   our_saxs_options.crysol_max_harmonics = 15;
   our_saxs_options.crysol_fibonacci_grid_order = 17;
   our_saxs_options.crysol_hydration_shell_contrast = 0.03f;
   our_saxs_options.crysol_default_load_difference_intensity = true;
   our_saxs_options.crysol_version_26 = true;

   our_saxs_options.fast_bin_size = 0.5f;
   our_saxs_options.fast_modulation = 0.23f;

   our_saxs_options.compute_saxs_coeff_for_bead_models = true;
   our_saxs_options.compute_sans_coeff_for_bead_models = false;
   // our_saxs_options.default_atom_filename = "somo.atom";
   // our_saxs_options.default_hybrid_filename =  "somo.hybrid";
   // our_saxs_options.default_saxs_filename =    "somo.saxs_atoms";
   // our_saxs_options.default_rotamer_filename = "somo.hydrated_rotamer";

   our_saxs_options.steric_clash_distance = 1.4;

   our_saxs_options.disable_iq_scaling = false;
   our_saxs_options.autocorrelate = true;
   our_saxs_options.hybrid_radius_excl_vol = false;
   our_saxs_options.scale_excl_vol = 1.0f;
   our_saxs_options.subtract_radius = false;
   our_saxs_options.iqq_scale_minq = 0.0f;
   our_saxs_options.iqq_scale_maxq = 0.0f;

   our_saxs_options.iqq_scale_nnls = false;
   our_saxs_options.iqq_scale_linear_offset = false;
   our_saxs_options.iqq_scale_chi2_fitting = true;
   our_saxs_options.iqq_expt_data_contains_variances = false;
   our_saxs_options.iqq_expt_data_contains_variances = false;
   our_saxs_options.iqq_ask_target_grid = true;
   our_saxs_options.iqq_scale_play = false;
   our_saxs_options.swh_excl_vol = 0.0f;
   our_saxs_options.iqq_default_scaling_target = "";

   our_saxs_options.saxs_iq_hybrid_adaptive = true;
   our_saxs_options.sans_iq_hybrid_adaptive = true;

   our_saxs_options.bead_model_rayleigh = true;

   if ( control_parameters.count( "wateredensity" ) )
   {
      our_saxs_options.water_e_density = control_parameters[ "wateredensity" ].toFloat();
   }

   if ( control_parameters.count( "swhexclvol" ) )
   {
      our_saxs_options.swh_excl_vol = control_parameters[ "swhexclvol" ].toFloat();
   }

   if ( control_parameters.count( "scaleexclvol" ) )
   {
      our_saxs_options.scale_excl_vol = control_parameters[ "scaleexclvol" ].toFloat();
   }

   if ( control_parameters.count( "startq" ) )
   {
      our_saxs_options.start_q = control_parameters[ "startq" ].toFloat();
   }
   if ( control_parameters.count( "endq" ) )
   {
      our_saxs_options.end_q   = control_parameters[ "endq"   ].toFloat();
   }
   if ( control_parameters.count( "deltaq" ) )
   {
      our_saxs_options.delta_q = control_parameters[ "deltaq" ].toFloat();
   }

   if ( control_parameters.count( "iqmethod" ) )
   {
      our_saxs_options.saxs_iq_native_debye = false;
      our_saxs_options.saxs_iq_native_hybrid = false;
      our_saxs_options.saxs_iq_native_hybrid2 = false;
      our_saxs_options.saxs_iq_native_hybrid3 = false;
      our_saxs_options.saxs_iq_native_fast = false;
      // our_saxs_options.saxs_iq_native_fast_compute_pr = false;
      our_saxs_options.saxs_iq_foxs = false;
      our_saxs_options.saxs_iq_crysol = false;
      our_saxs_options.saxs_iq_hybrid_adaptive = false;

      our_saxs_options.sans_iq_native_debye = false;
      our_saxs_options.sans_iq_native_hybrid = false;
      our_saxs_options.sans_iq_native_hybrid2 = false;
      our_saxs_options.sans_iq_native_hybrid3 = false;
      our_saxs_options.sans_iq_native_fast = false;
      // our_saxs_options.sans_iq_native_fast_compute_pr = false;
      our_saxs_options.sans_iq_cryson = false;
      our_saxs_options.sans_iq_hybrid_adaptive = false;

      QString qs = control_parameters[ "iqmethod" ];
      if ( qs == "db" )
      {
         our_saxs_options.saxs_iq_native_debye = true;
         our_saxs_options.sans_iq_native_debye = true;
      }
      if ( qs == "hy" || qs == "hya" )
      {
         our_saxs_options.saxs_iq_native_hybrid = true;
         our_saxs_options.sans_iq_native_hybrid = true;
      }
      if ( qs == "h2" || qs == "h2a" )
      {
         our_saxs_options.saxs_iq_native_hybrid2 = true;
         our_saxs_options.sans_iq_native_hybrid2 = true;
      }
      if ( qs == "h3" || qs == "h3a" )
      {
         our_saxs_options.saxs_iq_native_hybrid3 = true;
         our_saxs_options.sans_iq_native_hybrid3 = true;
      }
      if ( qs == "fd" )
      {
         our_saxs_options.saxs_iq_native_fast = true;
         our_saxs_options.sans_iq_native_fast = true;
      }
      if ( qs == "foxs" )
      {
         our_saxs_options.saxs_iq_foxs = true;
      }
      if ( qs == "crysol" )
      {
         our_saxs_options.saxs_iq_crysol = true;
      }
      if ( qs == "cryson" )
      {
         our_saxs_options.sans_iq_cryson = true;
      }
      if ( qs == "hya" || qs == "h2a" || qs == "h3a" )
      {
         our_saxs_options.saxs_iq_hybrid_adaptive = true;
         our_saxs_options.sans_iq_hybrid_adaptive = true;
      }
   }         

   if ( control_parameters.count( "fdbinsize" ) )
   {
      our_saxs_options.fast_bin_size = control_parameters[ "fdbinsize" ].toFloat();
   }
   if ( control_parameters.count( "fdmodulation" ) )
   {
      our_saxs_options.fast_modulation = control_parameters[ "fdmodulation" ].toFloat();
   }
   if ( control_parameters.count( "hypoints" ) )
   {
      our_saxs_options.hybrid2_q_points = control_parameters[ "hypoints" ].toUInt();
   }
   if ( control_parameters.count( "crysolharm" ) )
   {
      our_saxs_options.crysol_max_harmonics = control_parameters[ "crysolharm" ].toUInt();
   }
   if ( control_parameters.count( "crysolgrid" ) )
   {
      our_saxs_options.crysol_fibonacci_grid_order = control_parameters[ "crysolgrid" ].toUInt();
   }
   if ( control_parameters.count( "crysolchs" ) )
   {
      our_saxs_options.crysol_hydration_shell_contrast = control_parameters[ "crysolchs" ].toFloat();
   }
}

bool US_Saxs_Util::run_iqq()
{
   errormsg = "";
   noticemsg = "";
   if ( control_parameters.count( "iqmethod" ) )
   {
      QString qs = control_parameters[ "iqmethod" ];
      if ( qs == "db" )
      {
         calc_saxs_iq_native_debye();
      }
      if ( qs == "hy" || qs == "hya" ||
           qs == "h2" || qs == "h2a" ||
           qs == "h3" || qs == "h3a" )
      {
         calc_saxs_iq_native_hybrid();
      }
      if ( qs == "fd" )
      {
         calc_saxs_iq_native_fast();
      }
      if ( qs == "foxs" )
      {
         if ( !run_saxs_iq_foxs() )
         {
            return false;
         }
      }
      if ( qs == "crysol" )
      {
         if ( !run_saxs_iq_crysol() )
         {
            return false;
         }
      }
      if ( qs == "cryson" )
      {
         if ( !run_sans_iq_cryson() )
         {
            return false;
         }
      }
   }
   return true;
}
