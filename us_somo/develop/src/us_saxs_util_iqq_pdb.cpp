#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

// #define IQQ_TIMER

#if defined(IQQ_TIMER)
#  include "../include/us_timer.h"
//Added by qt3to4:
#include <QTextStream>
#endif

#define SAXS_MIN_Q 0e0
#define SLASH QDir::separator();

static bool save_calc_to_csv = false;

bool US_Saxs_Util::calc_saxs_iq_native_fast()
{
   errormsg = "";
   noticemsg = "";
   
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   QRegExp count_hydrogens("H(\\d)");
   
   if ( !compute_scale_excl_vol() )
   {
      return false;
   }

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
#if defined(IQQ_TIMER)
      US_Timer iqq_timers;
      iqq_timers.init_timer ( "iqq fast debye" );
      iqq_timers.init_timer ( "iqq fast debye setup" );
      iqq_timers.init_timer ( "iqq fast debye compute f" );
      iqq_timers.init_timer ( "iqq fast debye compute I" );
      iqq_timers.start_timer( "iqq fast debye" );
      iqq_timers.start_timer( "iqq fast debye setup" );
#endif
      double tot_excl_vol        = 0e0;
      double tot_excl_vol_noh    = 0e0;
      unsigned int total_e       = 0;
      unsigned int total_e_noh   = 0;
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
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq);
               continue;
            }
            
            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }
            
            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq_pdb 1) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom_name)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }
            
            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }
            // cout << QString("in %1 hydrogens %2\n").arg( hybrid_name ).arg( new_atom.hydrogens );
            
            // this is probably correct but FoXS uses the saxs table excluded volume
            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options.use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
//                   cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
//                      .arg( new_atom.atom_name )
//                      .arg( this_ev );
               }
            }

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
                  .arg(use_resname)
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
      
#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq fast debye setup" );
      iqq_timers.start_timer( "iqq fast debye compute f" );
#endif
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      if ( our_saxs_options.iq_exact_q )
      {
         // editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            // editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }

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
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);

      if ( our_saxs_options.iq_exact_q )
      {
         q = exact_q;
         q_points = q.size();
      }
      q2.resize          ( q_points );
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         if ( !our_saxs_options.iq_exact_q )
         {
            q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            if ( q[j] < SAXS_MIN_Q ) 
            {
               q[j] = SAXS_MIN_Q;
            }
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
            compute_ff(
                       saxs,
                       saxsH,
                       atoms[ i ].residue_name,
                       atoms[ i ].saxs_name,
                       atoms[ i ].atom_name,
                       atoms[ i ].hydrogens,
                       0.0,
                       0.0 );
//          f[0][i] = 
//             saxs.c +
//             saxs.a[0] +
//             saxs.a[1] +
//             saxs.a[2] +
//             saxs.a[3] +
//             atoms[i].hydrogens * 
//             ( saxsH.c +
//               saxsH.a[0] +
//               saxsH.a[1] +
//               saxsH.a[2] +
//               saxsH.a[3] );

         fc[0][i] = vie;
         fp[0][i] = f[0][i] - fc[0][i];
      }
#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq fast debye compute f" );
      iqq_timers.start_timer( "iqq fast debye compute I" );
#endif
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
      contrib_array.clear( );
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
         .arg( model_vector[ current_model ].model_id )
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
               FILE *fpr = us_fopen(fpr_name, "w");
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
                              , control_parameters[ "inputfile" ].toLatin1().data()
                              , US_Version.toLatin1().data()
                              , REVISION
                              , delta
                              , get_mw(control_parameters[ "inputfile" ], false)
                              , compute_pr_area(pr, r)
                              );
                  fprintf(fpr, "%s",
                          last_saxs_header.toLatin1().data() );
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
                        , control_parameters[ "inputfile" ].toLatin1().data()
                        , US_Version.toLatin1().data()
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
         
#if defined(IQQ_TIMER)
      {
         iqq_timers.end_timer  ( "iqq fast debye compute I" );
         iqq_timers.end_timer  ( "iqq fast debye" );
         cout << iqq_timers.list_times() << flush;
         timings = iqq_timers.list_times();
         QString file = 
            QString( "%1_%2_q%3-%4.timing" )
            .arg( control_parameters[ "outputfile" ] )
            .arg( model_vector[ current_model ].model_id )
            .arg( q_points )
            .arg( control_parameters[ "iqmethod" ] )
            ;
         write_timings( file, file );
      }
#endif
      noticemsg += "I(q) computed.\n";

      // save the data to a file
      if ( !write_output( model_vector[ current_model ].model_id, q, I ) )
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

   if ( !compute_scale_excl_vol() )
   {
      return false;
   }

#if defined(IQQ_TIMER)
   cout << "timer enabled\n";
#endif

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
#if defined(IQQ_TIMER)
      US_Timer iqq_timers;
      iqq_timers.init_timer ( "iqq native debye" );
      iqq_timers.init_timer ( "iqq native debye setup" );
      iqq_timers.init_timer ( "iqq native debye compute f" );
      iqq_timers.init_timer ( "iqq native debye compute I" );
      iqq_timers.start_timer( "iqq native debye" );
      iqq_timers.start_timer( "iqq native debye setup" );
#endif
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      unsigned int current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(model_vector[ current_model ].model_id);
         
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
               cout << "error: hybrid name missing for " << use_resname << "|" << this_atom->name << endl; 
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq);
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                  .arg(j+1)
                  .arg(this_atom_name)
                  .arg(use_resname)
                  .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            // cout << QString("atom %1 hybrid %2 excl vol %3 by hybrid radius %4\n")
            // .arg(this_atom->name)
            // .arg(this_atom->hybrid_name)
            // .arg(atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol)
            // .arg(M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius)
            // ;

            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options.use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
//                   cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
//                      .arg( new_atom.atom_name )
//                      .arg( this_ev );
               }
            }


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
            if ( !our_saxs_options.iqq_use_atomic_ff &&
                 count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name);
               continue;
            }

            atoms.push_back(new_atom);
         }
      }
               
#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq native debye setup" );
      iqq_timers.start_timer( "iqq native debye compute f" );
#endif
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      if ( our_saxs_options.iq_exact_q )
      {
         // editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            // editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }

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

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);

      if ( our_saxs_options.iq_exact_q )
      {
         q = exact_q;
         q_points = q.size();
      }
      q2.resize          ( q_points );
      q_over_4pi_2.resize( q_points );
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);


      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         if ( !our_saxs_options.iq_exact_q )
         {
            q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            if ( q[j] < SAXS_MIN_Q ) 
            {
               q[j] = SAXS_MIN_Q;
            }
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

            f[ j ][ i ] = compute_ff( saxs,
                                      saxsH,
                                      atoms[ i ].residue_name,
                                      atoms[ i ].saxs_name,
                                      atoms[ i ].atom_name,
                                      atoms[ i ].hydrogens,
                                      q[ j ],
                                      q_over_4pi_2[ j ] );
            if ( !noticemsg.isEmpty() )
            {
               cout << QString( "Notice: %1\n" ).arg( noticemsg );
            }

//             f[j][i] = saxs.c + 
//                saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
//                saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
//                saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
//                saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]) +
//                atoms[i].hydrogens * 
//                ( saxsH.c + 
//                  saxsH.a[0] * exp(-saxsH.b[0] * q_over_4pi_2[j]) +
//                  saxsH.a[1] * exp(-saxsH.b[1] * q_over_4pi_2[j]) +
//                  saxsH.a[2] * exp(-saxsH.b[2] * q_over_4pi_2[j]) +
//                  saxsH.a[3] * exp(-saxsH.b[3] * q_over_4pi_2[j]) );

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
                       "\"%2\","
                       "\"%3\","
                       "%4,"
                       "%5,"
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
         if ( f.open( QIODevice::WriteOnly ) )
         {
            QTextStream t( &f );
            t << out;
            f.close();
            noticemsg += QString("created %1\n").arg(f.fileName());
         }
      }            

#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq native debye compute f" );
      iqq_timers.start_timer( "iqq native debye compute I" );
#endif
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
      if ( control_parameters.count( "iqcuda" ) )
      {
         if ( !iqq_cuda( q, atoms, fp, I, 
                         control_parameters[ "iqcuda" ].toUInt() > 0 ?
                         control_parameters[ "iqcuda" ].toUInt() : 32 ) )
         {
            return false;
         }
      } else {
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
      }

#if defined(IQQ_TIMER)
      {
         iqq_timers.end_timer  ( "iqq native debye compute I" );
         iqq_timers.end_timer  ( "iqq native debye" );
         cout << iqq_timers.list_times() << flush;
         timings = iqq_timers.list_times();
         QString file = 
            QString( "%1_%2_q%3-%4%5.timing" )
            .arg( control_parameters[ "outputfile" ] )
            .arg( model_vector[ current_model ].model_id )
            .arg( q_points )
            .arg( control_parameters[ "iqmethod" ] )
            .arg( control_parameters.count( "iqcuda" ) ?
                  QString( "cuda%1" )
                  .arg( control_parameters[ "iqcuda" ].toUInt() > 0 ?
                        control_parameters[ "iqcuda" ].toUInt() : 32  ) : QString( "" ) )
            ;
         write_timings( file, file );
      }
#endif
      noticemsg += "I(q) computed.\n";

      // save the data to a file
      if ( control_parameters.count( "iqcuda" ) )
      {
         control_parameters[ "cuda_tag" ] = 
            QString( "cuda%1" )
            .arg( control_parameters[ "iqcuda" ].toUInt() > 0 ?
                  control_parameters[ "iqcuda" ].toUInt() : 32  );
      } else {
         control_parameters.erase( "cuda_tag" );
      }
      if ( !write_output( model_vector[ current_model ].model_id, q, I ) )
      {
         return false;
      }
      control_parameters.erase( "cuda_tag" );
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

   if ( !compute_scale_excl_vol() )
   {
      return false;
   }

#if defined(IQQ_TIMER)
   cout << "timer enabled\n";
#endif

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
#if defined(IQQ_TIMER)
      US_Timer iqq_timers;
      iqq_timers.init_timer ( "iqq hybrid debye" );
      iqq_timers.init_timer ( "iqq hybrid debye setup" );
      iqq_timers.init_timer ( "iqq hybrid debye compute f" );
      iqq_timers.init_timer ( "iqq hybrid debye compute I" );
      iqq_timers.start_timer( "iqq hybrid debye" );
      iqq_timers.start_timer( "iqq hybrid debye setup" );
#endif
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      unsigned int current_model = i;
      noticemsg += QString("Preparing model %1 for SAXS plot.\n").arg(model_vector[ current_model ].model_id);
         
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
               cout << "error: hybrid name missing for " << use_resname << "|" << this_atom->name << endl; 
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
                              .arg(this_atom->resSeq);
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
                              .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               noticemsg += QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom_name)
                              .arg(use_resname)
                              .arg(this_atom->resSeq)
                  .arg(hybrid_name);
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options.use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
//                   cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
//                      .arg( new_atom.atom_name )
//                      .arg( this_ev );
               }
            }

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
            if ( count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               noticemsg += QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                  .arg(hybrid_map[hybrid_name].saxs_name);
               continue;
            }

            atoms.push_back(new_atom);
         }
      }
#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq hybrid debye setup" );
      iqq_timers.start_timer( "iqq hybrid debye compute f" );
#endif
         
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options.end_q - our_saxs_options.start_q) / our_saxs_options.delta_q) + .5) + 1;
         
      if ( our_saxs_options.iq_exact_q )
      {
         // editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            // editor_msg( "dark red", QString( "Notice: exact q is empty, computing based upon current q range " ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }

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
      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);

      if ( our_saxs_options.iq_exact_q )
      {
         q = exact_q;
         q_points = q.size();
      }
      q2.resize          ( q_points );
      q_over_4pi_2.resize( q_points );
      f.resize(q_points);
      fc.resize(q_points);
      fp.resize(q_points);


      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         if ( !our_saxs_options.iq_exact_q )
         {
            q[j] = our_saxs_options.start_q + j * our_saxs_options.delta_q;
            if ( q[j] < SAXS_MIN_Q ) 
            {
               q[j] = SAXS_MIN_Q;
            }
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

            f[ j ][ i ] = 
               compute_ff( saxs,
                           saxsH,
                           atoms[ i ].residue_name,
                           atoms[ i ].saxs_name,
                           atoms[ i ].atom_name,
                           atoms[ i ].hydrogens,
                           q[ j ],
                           q_over_4pi_2[ j ] );

//             f[j][i] = saxs.c + 
//                saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
//                saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
//                saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
//                saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]) +
//                atoms[i].hydrogens * 
//                ( saxsH.c + 
//                  saxsH.a[0] * exp(-saxsH.b[0] * q_over_4pi_2[j]) +
//                  saxsH.a[1] * exp(-saxsH.b[1] * q_over_4pi_2[j]) +
//                  saxsH.a[2] * exp(-saxsH.b[2] * q_over_4pi_2[j]) +
//                  saxsH.a[3] * exp(-saxsH.b[3] * q_over_4pi_2[j]) );

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
#if defined(IQQ_TIMER)
      iqq_timers.end_timer  ( "iqq hybrid debye compute f" );
      iqq_timers.start_timer( "iqq hybrid debye compute I" );
#endif
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

#if defined(IQQ_TIMER)
      {
         iqq_timers.end_timer  ( "iqq hybrid debye compute I" );
         iqq_timers.end_timer  ( "iqq hybrid debye" );
         cout << iqq_timers.list_times() << flush;
         timings = iqq_timers.list_times();
         QString file = 
            QString( "%1_%2_q%3-%4.timing" )
            .arg( control_parameters[ "outputfile" ] )
            .arg( model_vector[ current_model ].model_id )
            .arg( q_points )
            .arg( control_parameters[ "iqmethod" ] )
            ;
         write_timings( file, file );
      }
#endif
      noticemsg += "I(q) computed.\n";

      if ( !write_output( model_vector[ current_model ].model_id, q, I ) )
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
            .arg( our_saxs_options.sh_max_harmonics )
            .arg( our_saxs_options.sh_fibonacci_grid_order )
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
  //cout << "setup saxs options\n";

   hybrid_coords.clear( );
   point p;

   // #define UHS_ALEXANDROS_POINTS

#if defined( UHS_ALEXANDROS_POINTS )

   p.axis[ 0 ] = 0.0;
   p.axis[ 1 ] = 0.0;
   p.axis[ 2 ] = 0.0;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.005;
   p.axis[ 1 ] = 1.012;
   p.axis[ 2 ] = 0.341;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 1.032;
   p.axis[ 1 ] = -0.268;
   p.axis[ 2 ] = -0.099;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.469;
   p.axis[ 1 ] = -0.652;
   p.axis[ 2 ] = 0.706;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.067;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.438;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = 0.825;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.495;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = -0.792;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.934;
   p.axis[ 1 ] = -0.312;
   p.axis[ 2 ] = -0.033;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.960;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 0.000;
   hybrid_coords[ "S" ].push_back( p );

   p.axis[ 0 ] = 0.000;
   p.axis[ 1 ] = 0.000;
   p.axis[ 2 ] = 1.340;
   hybrid_coords[ "S" ].push_back( p );
#else
   p.axis[ 0 ] = 0.0;
   p.axis[ 1 ] = 0.0;
   p.axis[ 2 ] = 0.0;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 1.070f;
   p.axis[ 1 ] = 0.0f;
   p.axis[ 2 ] = 0.0f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.366f;
   p.axis[ 1 ] = 1.039f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = -0.222f;
   p.axis[ 1 ] = -0.722f;
   p.axis[ 2 ] = -0.724f;
   hybrid_coords[ "C" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 1.000f;
   p.axis[ 1 ] = 0.0f;
   p.axis[ 2 ] = 0.0f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.531f;
   p.axis[ 1 ] = 0.868f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = -0.314f;
   p.axis[ 1 ] = 1.039f;
   p.axis[ 2 ] = -0.414f;
   hybrid_coords[ "N" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.960f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.00f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = -0.240f;
   p.axis[ 1 ] = 0.929f;
   p.axis[ 2 ] = 0.00f;
   hybrid_coords[ "O" ].push_back( p );

   p.axis[ 0 ] = 0.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "S" ].push_back( p );

   p.axis[ 0 ] = 1.000f;
   p.axis[ 1 ] = 0.000f;
   p.axis[ 2 ] = 0.000f;
   hybrid_coords[ "S" ].push_back( p );
#endif
   // compute pairwise distances

   hybrid_r.clear( );

   for ( map < QString, vector < point > >::iterator it = hybrid_coords.begin();
         it != hybrid_coords.end();
         it++ )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) it->second.size(); i++ )
      {
         for ( unsigned int j = 0; j < ( unsigned int ) it->second.size(); j++ )
         {
            hybrid_r[ it->first ][ i ][ j ] =
               sqrt( 
                    ( it->second[ i ].axis[ 0 ] - it->second[ j ].axis[ 0 ] ) *
                    ( it->second[ i ].axis[ 0 ] - it->second[ j ].axis[ 0 ] ) 
                    +
                    ( it->second[ i ].axis[ 1 ] - it->second[ j ].axis[ 1 ] ) *
                    ( it->second[ i ].axis[ 1 ] - it->second[ j ].axis[ 1 ] ) 
                    +
                    ( it->second[ i ].axis[ 2 ] - it->second[ j ].axis[ 2 ] ) *
                    ( it->second[ i ].axis[ 2 ] - it->second[ j ].axis[ 2 ] ) 
                    );
         }
      }                    
   }


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

   our_saxs_options.sh_max_harmonics = 15;
   our_saxs_options.sh_fibonacci_grid_order = 17;
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

   our_saxs_options.iq_exact_q = false;
   if ( control_parameters.count( "exactq" ) )
   {
      our_saxs_options.iq_exact_q = true;
   }

   our_saxs_options.use_somo_ff = false;
   if ( control_parameters.count( "fffile" ) )
   {
      our_saxs_options.use_somo_ff = true;
   }

   our_saxs_options.alt_ff = true;

   our_saxs_options.five_term_gaussians = false;
   if ( control_parameters.count( "fivetermgaussian" ) )
   {
      our_saxs_options.five_term_gaussians = true;
   }

   our_saxs_options.iqq_use_atomic_ff = false;
   if ( control_parameters.count( "explicith" ) )
   {
      our_saxs_options.iqq_use_atomic_ff = true;
   }

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
      our_saxs_options.sh_max_harmonics = control_parameters[ "crysolharm" ].toUInt();
   }
   if ( control_parameters.count( "crysolgrid" ) )
   {
      our_saxs_options.sh_fibonacci_grid_order = control_parameters[ "crysolgrid" ].toUInt();
   }
   if ( control_parameters.count( "crysolchs" ) )
   {
      our_saxs_options.crysol_hydration_shell_contrast = control_parameters[ "crysolchs" ].toFloat();
   }

   if ( control_parameters.count( "hydrationscd" ) )
   {
      our_saxs_options.steric_clash_distance = control_parameters[ "hydrationscd" ].toFloat();
   }

   our_saxs_options.iq_target_ev = 0e0;
   if ( control_parameters.count( "iqtargetev" ) )
   {
      our_saxs_options.iq_target_ev = control_parameters[ "iqtargetev" ].toDouble();
   }

   our_saxs_options.hybrid_radius_excl_vol     = control_parameters.count( "hybridradiusexclvol" ) ? true : false;
   our_saxs_options.use_iq_target_ev           = control_parameters.count( "useiqtargetev" ) ? true : false;
   our_saxs_options.set_iq_target_ev_from_vbar = control_parameters.count( "setiqtargetevfromvbar" ) ? true : false;

   if ( control_parameters.count( "prbinsize" ) )
   {
      our_saxs_options.bin_size = control_parameters[ "prbinsize" ].toFloat();
   }
   if ( control_parameters.count( "prcurve" ) )
   {
      our_saxs_options.curve = control_parameters[ "prcurve" ].toInt();
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

// #define USUIP_COMPUTE_FF_DEBUG
double US_Saxs_Util::compute_ff(
                                saxs     &sa,        // gaussian decomposition for the main atom
                                saxs     &sh,        // gaussian decomposition for hydrogen
                                QString  &nr,     // name of residue
                                QString  &na,     // name of atom
                                QString  &naf,    // full name of atom
                                unsigned int h,      // number of hydrogens
                                double   q,
                                double   q_o_4pi2 
                                )
{
   errormsg  = "";
   noticemsg = "";

#if defined( USUIP_COMPUTE_FF_DEBUG )
   cout << QString( "compute_ff: q: %1 nr:%2 na:%3 naf:%4 h:%5 use_somo_ff %6 alt_ff %7\n" )
      .arg( q )
      .arg( nr )
      .arg( na )
      .arg( naf )
      .arg( h )
      .arg( our_saxs_options.use_somo_ff ? "yes" : "no" )
      .arg( our_saxs_options.alt_ff ? "yes" : "no" );
#endif

   if ( our_saxs_options.use_somo_ff )
   {
      QString ffkey = nr + "|" + naf;
      if ( ff_table.count( ffkey ) )
      {
         // ok
         double  ff;
         if ( !apply_natural_spline( ff_q [ ff_table[ ffkey ] ],
                                     ff_ff[ ff_table[ ffkey ] ],
                                     ff_y2[ ff_table[ ffkey ] ],
                                     q,
                                     ff ) )
         {
            noticemsg += QString( "Warning: error applying natural spline to %1 at %2 <%3>" )
               .arg( ffkey ) 
               .arg( q ) 
               .arg( errormsg );
            errormsg = "";
         } else {
            return ff;
         }
      } else {
         if ( !ff_sent_msg1.count( ffkey ) )
         {
            noticemsg += QString( "Warning: key %1 not found in ff_table" ).arg( ffkey );
            ff_sent_msg1[ ffkey ] = true;
         }
      }
   }

   if ( !h )
   {
      if ( our_saxs_options.five_term_gaussians )
      {
         // cout << "five term gaussian\n";
         return 
            sa.c5 +
            sa.a5[ 0 ] * exp( -sa.b5[ 0 ] * q_o_4pi2 ) +
            sa.a5[ 1 ] * exp( -sa.b5[ 1 ] * q_o_4pi2 ) +
            sa.a5[ 2 ] * exp( -sa.b5[ 2 ] * q_o_4pi2 ) +
            sa.a5[ 3 ] * exp( -sa.b5[ 3 ] * q_o_4pi2 ) +
            sa.a5[ 4 ] * exp( -sa.b5[ 4 ] * q_o_4pi2 );
      } else {
         // cout << "four term gaussian\n";
         return 
            sa.c +
            sa.a[ 0 ] * exp( -sa.b[ 0 ] * q_o_4pi2 ) +
            sa.a[ 1 ] * exp( -sa.b[ 1 ] * q_o_4pi2 ) +
            sa.a[ 2 ] * exp( -sa.b[ 2 ] * q_o_4pi2 ) +
            sa.a[ 3 ] * exp( -sa.b[ 3 ] * q_o_4pi2 );
      }
   }

   if ( !our_saxs_options.alt_ff )
   {
      cout << "Warning: old !alt_ff no longer supported\n";
      return 0.0;
//       return sa.c + 
//          sa.a[0] * exp(-sa.b[0] * q_o_4pi2) +
//          sa.a[1] * exp(-sa.b[1] * q_o_4pi2) +
//          sa.a[2] * exp(-sa.b[2] * q_o_4pi2) +
//          sa.a[3] * exp(-sa.b[3] * q_o_4pi2) +
//          h *
//          ( sh.c + 
//            sh.a[0] * exp(-sh.b[0] * q_o_4pi2) +
//            sh.a[1] * exp(-sh.b[1] * q_o_4pi2) +
//            sh.a[2] * exp(-sh.b[2] * q_o_4pi2) +
//            sh.a[3] * exp(-sh.b[3] * q_o_4pi2) );
   }

//    q_o_4pi2 = 
//       ( q / ( 2.0 * 3.1415926535 ) ) / ( 4.0 * 3.1415926535 ) *
//       ( q / ( 2.0 * 3.1415926535 ) ) / ( 4.0 * 3.1415926535 );

//    cout << "q|q/4pi2|name|hydrogens|a1|b1|a2|b2|a3|b3|a4|b4|c|ff\n";
//    cout << 
//       QString( "%1|%2|%3|%4|" )
//       .arg( q )
//       .arg( q_o_4pi2 )
//       .arg( na )
//       .arg( h );

//    for ( int i = 0; i < 4; i++ )
//    {
//       cout << QString( "%1|%2|" ).arg( sa.a[ i ] ).arg( sa.b[ i ] );
//    }

//    cout << QString( "%1|%2\n" )
//       .arg( sa.c )
//       .arg( sa.c +
//             sa.a[ 0 ] + exp( -sa.b[ 0 ] * q_o_4pi2 ) +
//             sa.a[ 1 ] + exp( -sa.b[ 1 ] * q_o_4pi2 ) +
//             sa.a[ 2 ] + exp( -sa.b[ 2 ] * q_o_4pi2 ) +
//             sa.a[ 3 ] + exp( -sa.b[ 3 ] * q_o_4pi2 ) );

   if ( !hybrid_coords.count( na ) )
   {
      noticemsg +=  QString( "ERROR: No hybrid info for atom %1, computing for ZERO hydrogens" ).arg( na );
      return compute_ff( sa, sh, nr, na, naf, 0, q, q_o_4pi2 );
   }

   if ( hybrid_coords[ na ].size() <= h )
   {
      noticemsg += 
         QString( "ERROR: More hydrogens requested (%1) than available (%2) for atom %2, computing for available hydrogens" )
         .arg( h ) 
         .arg( hybrid_coords[ na ].size() - 1 )
         .arg( na ) ;
      h = hybrid_coords[ na ].size();
   }

   // now compute debye for "mini" system at q, return square root of intensity

   // cout << "mini debye\n";
   double ff_a = compute_ff( sa, sh, nr, na, naf, 0, q, q_o_4pi2 );
   QString nh = "H";
   double ff_h = compute_ff( sh, sh, nr, nh, nh,  0, q, q_o_4pi2 );

   vector < double > ff( h + 1 );
   ff[ 0 ] = ff_a;
   for ( unsigned int i = 1; i <= h; i++ )
   {
      ff[ i ] = ff_h;
   }

   double I = 0e0;

   double qrik;
   double sqrikd;

   for ( unsigned int i = 0; i < h; i++ )
   {
      for ( unsigned int k = i + 1; k <= h; k++ )
      {
         qrik = hybrid_r[ na ][ i ][ k ] * q;
         sqrikd = ( fabs( qrik ) < 1e-30 ) ? 1.0 : sin(qrik) / qrik;
         I += 2.0 * ff[ i ] * ff[ k ] * sqrikd;
      }
   }

   if ( our_saxs_options.autocorrelate )
   {
      for ( unsigned int i = 0; i <= h; i++ )
      {
         I += ff[ i ] * ff[ i ];
      }
   }

   return sqrt( I );
}                                    

bool US_Saxs_Util::load_ff_table( QString filename )
{
   if ( ff_table_loaded &&
        last_ff_filename == filename )
   {
      return true;
   }

   noticemsg += QString(  "Loading ff table %1" ).arg( filename );

   QFile f( filename );
   if ( !f.exists() )
   {
      errormsg =  QString( "Error: load ff table %1 does not exist" ).arg( filename );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString("Error: load ff table %1 can not be opened" ).arg( filename );
      return false;
   }

   ff_table_loaded = false;

   ff_table.clear( );
   ff_q    .clear( );
   ff_ff   .clear( );
   ff_y2   .clear( );
   ff_ev   .clear( );

   QTextStream ts( &f );
   
   unsigned int line = 0;

   QRegExp rx_blank  ( "^\\s*$" );
   QRegExp rx_comment( "#.*$" );

   while ( !ts.atEnd() )
   {
      QString     qs  = ts.readLine();

      line++;

      if ( qs.contains( rx_blank ) || qs.contains( rx_comment ) )
      {
         continue;
      }

      QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

      // expect:
      //   residueatom (possibly multiple)
      //   excludedvolume (once)
      //   startdata
      // ... data ...
      //   enddata

      unsigned int pos = ff_q.size();

      if ( qsl[ 0 ] == "residueatom" )
      {
         qsl.pop_front();
         while ( qsl.count() > 1 )
         {
            QString res = qsl[ 0 ];
            qsl.pop_front();
            QString atm = qsl[ 0 ];
            qsl.pop_front();

            QString this_key = QString( "%1|%2" ).arg( res ).arg( atm );
            ff_table[ this_key ] = pos;
         }
         if ( qsl.count() )
         {
            errormsg =  QString( "Error: load_ff line %1, residue/atoms not paired" ).arg( line );
            return false;
         }
         continue;
      }

      if ( qsl[ 0 ] == "excludedvolume" )
      {
         qsl.pop_front();
         if ( qsl.count() !=  1 )
         {
            errormsg = QString( "Error: load_ff line %1, excluded volume requires exactly one parameter" ).arg( line );
            return false;
         }

         if ( ff_ev.size() != pos )
         {
            errormsg = QString( "Error: load_ff line %1, excluded volume multiply defined" ).arg( line );
            return false;
         }
         
         ff_ev.push_back( qsl[ 0 ].toDouble() );
         continue;
      }

      if ( qsl[ 0 ] == "startdata" )
      {
         vector < double > new_q;
         vector < double > new_ff;

         bool ok = false;

         if ( ff_ev.size() != ff_ff.size() + 1 )
         {
            errormsg = QString( "Error: load_ff line %1, startdata with no excluded volume defined" ).arg( line );
            return false;
         }

         cout << QString( "pos %1 ev %2\n" ).arg( ff_ev.size() ).arg( ff_ev.back() );

         while ( !ts.atEnd() )
         {
            QString     qs  = ts.readLine();
            line++;

            if ( qs.contains( rx_blank ) || qs.contains( rx_comment ) )
            {
               continue;
            }

            QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

            if ( qsl[ 0 ] == "enddata" )
            {
               if ( new_q.size() < 10 )
               {
                  errormsg = QString( "Error: load_ff line %1, empty or insufficient data" ).arg( line );
                  return false;
               }
                  
               ff_q .push_back( new_q  );
               ff_ff.push_back( new_ff );
               // compute natural spline info
               vector < double > new_y2;
               US_Saxs_Util::natural_spline( new_q, new_ff, new_y2 );
               ff_y2.push_back( new_y2 );
               ok = true;
               break;
            }

            if ( qsl.size() != 2 )
            {
               errormsg = QString( "Error: load_ff line %1, data lines must have exactly two fields" ).arg( line );
               return false;
            }

            new_q.push_back ( qsl[ 0 ].toDouble() );
            new_ff.push_back( qsl[ 1 ].toDouble() );

         }
         if ( !ok )
         {
            errormsg = QString( "Error: load_ff line %1, permature eof: no enddata token found" ).arg( line );
            return false;
         }
         continue;
      }
      errormsg = QString( "Error: load_ff line %1, unknown token %2" ).arg( line ).arg( qsl[ 0 ] );
      return false;
   }
   f.close();

   errormsg = QString( "ff table %1 loaded ok" ).arg( filename );

   ff_table_loaded  = true;
   last_ff_filename = filename;

   cout << ff_info();

   return true;
}


QString US_Saxs_Util::ff_info()
{
   QString qs;
   
   for ( map < QString, unsigned int >::iterator it = ff_table.begin();
         it != ff_table.end();
         it++ )
   {
      qs += QString( "%1 %2\n" ).arg( it->first ).arg( it->second );
   }

   for ( unsigned int i = 0; i < ff_q.size(); i++ )
   {
      qs += QString( "id:%1 q:%2 I:%3 y2:%4 ev:%5\n" )
         .arg( i )
         .arg( ff_q [ i ].size() )
         .arg( ff_ff[ i ].size() )
         .arg( ff_y2[ i ].size() )
         .arg( ff_ev[ i ] );
   }
   return qs;
}

double US_Saxs_Util::get_ff_ev( QString res, QString atm )
{
   QString ffkey = res + "|" + atm;
   if ( !ff_table.count( ffkey ) )
   {
      errormsg = QString( "Warning: key %1 not found in ff_table" ).arg( ffkey );
      return 0e0;
   }

   return ff_ev[ ff_table[ ffkey ] ];
}

map < QString, unsigned int > US_Saxs_Util::get_atom_summary_counts( PDB_model *model,
                                                                     map < QString, QString > &residue_atom_hybrid_map,
                                                                     saxs_options *use_saxs_options
                                                                     )
{
   map < QString, unsigned int > asc;
   
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int j = 0; j < model->molecule.size(); j++ )
   {
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &( model->molecule[ j ].atom[ k ] );
         if ( this_atom->name == "XH" && !use_saxs_options->iqq_use_atomic_ff )
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
            cout << QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
               .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
               .arg(j+1)
               .arg(use_resname)
               .arg(this_atom->resSeq);
            continue;
         }
            
         int hydrogens = 0;
         if ( count_hydrogens.indexIn( hybrid_name ) != -1 )
         {
            hydrogens = count_hydrogens.cap( 1 ).toInt();
         }
         QString atom_name = this_atom->name;
         if ( atom_name.contains( "^H" ) )
         {
            asc[ "H" ]++;
         } else {
            asc[ atom_name.left(1) ]++;
            asc[ "H" ] += hydrogens;
         }
      }
   }
   return asc;
}

QString US_Saxs_Util::list_atom_summary_counts( PDB_model *model,
                                                map < QString, QString > &residue_atom_hybrid_map,
                                                saxs_options *use_saxs_options
                                                )
{
   map < QString, unsigned int > asc = get_atom_summary_counts( model, residue_atom_hybrid_map, use_saxs_options );
   QString qs;
   for ( map < QString, unsigned int >::iterator it = asc.begin();
         it != asc.end();
         it++ )
   {
      qs += QString( "%1 %2\n" ).arg( it->first ).arg( it->second );
   }
   return qs;
}

bool US_Saxs_Util::compute_scale_excl_vol()
{
   errormsg = "";
   noticemsg = "";

   if ( !our_saxs_options.use_iq_target_ev )
   {
      return true;
   }

   if ( our_saxs_options.set_iq_target_ev_from_vbar )
   {
      // really only needs 1st one, as NMR style should all have the same atoms
      for ( unsigned int i = 0; i < model_vector.size() && i < 1; i++ )
      {
         current_model = i;
         our_saxs_options.iq_target_ev = 0e0;
         
         for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
         {
            bool swh_chain = true;
            unsigned int swh_count = 0;
            if ( model_vector[current_model].molecule[j].atom.size() )
            {
               for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
               {
                  PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
                  if ( this_atom->name != "OW" )
                  {
                     swh_chain = false;
                     break;
                  }
                  swh_count++;
               }

               if ( swh_chain )
               {
                  if ( our_saxs_options.swh_excl_vol > 0e0 )
                  {
                     our_saxs_options.iq_target_ev += (double) our_saxs_options.swh_excl_vol * swh_count;
                     scale_excl_vol_msgs += 
                        QString( "Adding water to excluded volume using preset excl vol %1 number of waters %2 -> excluded volume %3\n" )
                        .arg( our_saxs_options.swh_excl_vol )
                        .arg( swh_count )
                        .arg( (double) our_saxs_options.swh_excl_vol * swh_count );
                  } else {
                     QString mapkey = "WAT|OW";
                     QString hybrid_name = residue_atom_hybrid_map[mapkey];
                     if ( hybrid_name.isEmpty() || !hybrid_name.length() || !hybrid_map.count( hybrid_name ) 
                          || !atom_map.count( "OW~" + hybrid_name) )
                     {
                        noticemsg += QString("Molecule %1 Residue WAT OW Hybrid/map/atom name missing. Atom skipped.\n")
                           .arg( j + 1 );
                        continue;
                     }
                     double swh_excl_vol = atom_map["OW~" + hybrid_name].saxs_excl_vol;
                     our_saxs_options.iq_target_ev += swh_excl_vol * swh_count;
                     scale_excl_vol_msgs += 
                        QString( "Adding water to excluded volume using defined excl vol %1 number of waters %2 -> excluded volume %3\n" )
                        .arg( swh_excl_vol )
                        .arg( swh_count )
                        .arg( swh_excl_vol * swh_count );
                  }
               } else {
                  if ( swh_count )
                  {
                     errormsg = "a chain with WAT/OW and other residues is currently unsupported\n";
                     return false;
                  }
                  
                  our_saxs_options.iq_target_ev += mw_to_volume( model_vector[current_model].molecule[j].mw, model_vector[i].vbar );
                  scale_excl_vol_msgs += 
                              QString( "Adding molecule to excluded volume from mw/vbar computation: mw %1 vbar %2 -> excluded volume %3\n" )
                              .arg( model_vector[current_model].molecule[j].mw )
                              .arg( model_vector[current_model].vbar )
                              .arg( mw_to_volume( model_vector[current_model].molecule[j].mw, model_vector[current_model].vbar ) );
               }
            }
         }
      }
   }
   
   
   QString( "Total computed excluded volume: %1\n" ).arg( our_saxs_options.iq_target_ev );

   double tot_excl_vol      = 0e0;
   double tot_excl_vol_noh  = 0e0;
   
   // really only needs 1st one, as NMR style should all have the same atoms
   for ( unsigned int i = 0; i < model_vector.size() && i < 1; i++ )
   {
      current_model = i;

      saxs_atom new_atom;

      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {

            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

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
               cout << "error: hybrid name missing for " << use_resname << "|" << this_atom->name << endl; 
               scale_excl_vol_msgs += 
                  QString( "%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n" )
                  .arg( this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " " ) )
                  .arg( j + 1 )
                  .arg( use_resname )
                  .arg( this_atom->resSeq )
                  ;
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               scale_excl_vol_msgs += 
                  QString( "%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n" )
                  .arg( this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " " ) )
                  .arg( j + 1 )
                  .arg( use_resname )
                  .arg( this_atom->resSeq )
                  .arg( hybrid_name )
                  ;
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq_pdb 2) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               scale_excl_vol_msgs += 
                  QString( "%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n" )
                  .arg( this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " " ) )
                  .arg( j + 1 )
                  .arg( this_atom_name )
                  .arg( use_resname )
                  .arg( this_atom->resSeq )
                  .arg( hybrid_name )
                  ;
               continue;
            }

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            
            // this is probably correct but FoXS uses the saxs table excluded volume
            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

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
               tot_excl_vol_noh  += new_atom.excl_vol;
            }
            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;
         }
      }
   }


   // ok, we need to target our_saxs_options.iq_target_ev
   // noh_:  only includes unscalable 
   // target = scaler * tot_excl_vol_noh  + ( tot_excl_vol - tot_excl_vol_noh )

   if ( tot_excl_vol_noh > 0e0 )
   {
      our_saxs_options.scale_excl_vol =
         ( our_saxs_options.iq_target_ev - tot_excl_vol + tot_excl_vol_noh ) / tot_excl_vol_noh;
   } else {
      our_saxs_options.scale_excl_vol = 1e0;
   }

   scale_excl_vol_msgs += 
      QString( "Scaling excluded volume to match target (%1), default scattering center sum without WAT %2, total %3, new scaler = %4" )
      .arg( our_saxs_options.iq_target_ev )
      .arg( tot_excl_vol_noh )
      .arg( tot_excl_vol )
      .arg( our_saxs_options.scale_excl_vol )
      ;
   return true;
}

bool US_Saxs_Util::pdb_mw( QString file, double & mw )
{
   errormsg = "";
   QFile f( file );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "US_Saxs_Util::pdb_pw(): could not open file %1" ).arg( file );
      return false;
   }
   QTextStream ts( &f );
   QStringList qsl;
   while ( !ts.atEnd() )
   {
      qsl << ts.readLine();
   }
   return pdb_mw( qsl, mw );
}

bool US_Saxs_Util::pdb_mw( QStringList &qsl, double & mw )
{
   if ( !atom_mw.size() )
   {
      errormsg = "US_Saxs_Util:: mw table not loaded";
      return false;
   }
      
   map < QString, int > atom_counts;

   QRegExp rx_atom( "^(HETATM|ATOM)" );

   for ( int i = 0; i < (int) qsl.size(); ++i )
   {
      if ( rx_atom.indexIn( qsl[ i ] ) != -1 )
      {
         QString qs = qsl[ i ].mid( 76, 2 ).trimmed();
         if ( qs.isEmpty() )
         {
            errormsg = "US_Saxs_Util::pdb_pw(): atom names must be defined in columns 77, 78 of pdb";
            return false;
         }
         if ( atom_counts.count( qs ) )
         {
            atom_counts[ qs ]++;
         } else {
            atom_counts[ qs ] = 1;
         }
      }
   }

   mw = 0e0;
   for ( map < QString, int >::iterator it = atom_counts.begin();
         it != atom_counts.end();
         it++ )
   {
      if ( !atom_mw.count( it->first ) )
      {
         errormsg = QString( "US_Saxs_Util::pdb_pw(): atom_mw map does not contain atom %1" ).arg( it->first );
         return false;
      }
      mw += atom_mw[ it->first ] * it->second;
   }

   // us_qdebug( QString( "total mw %1" ).arg( mw ) );

   return true;
}
