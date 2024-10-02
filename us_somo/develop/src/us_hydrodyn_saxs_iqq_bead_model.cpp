#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_saxs_util.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
// #  define isnan _isnan
#endif

#define SAXS_MIN_Q 1e-6
// #define SAXS_DEBUG
// #define SAXS_DEBUG_F
// #define SAXS_DEBUG_FF
// #define SAXS_DEBUG2

void US_Hydrodyn_Saxs::calc_saxs_iq_native_fast_bead_model()
{
   cout << "calc_saxs_iq_native_fast_bead_model\n";
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry

   // cout << "plotted_I.size() " << plotted_I.size() << endl;
   // cout << "plotted_q.size() " << plotted_q.size() << endl;
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      double tot_excl_vol = 0e0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
         // new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.excl_vol = this_atom->saxs_excl_vol;
         if ( our_saxs_options->bead_models_use_bead_radius_ev )
         {
            new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow( this_atom->bead_computed_radius, 3 );
         }
         if ( our_saxs_options->bead_models_rho0_in_scat_factors )
         {
            new_atom.excl_vol = 0e0;
         }
         new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }
      
      if ( atoms.size() == 1 )
      {
         editor_msg( "dark blue", us_tr( "Notice: using full Debye since only 1 bead available" ) );
         return calc_saxs_iq_native_debye_bead_model();
      }

      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %s %.3f %.3f %.3f %.2f %.2f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
                    , atoms[i].hybrid_name.toLatin1().data()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol
                    , atoms[i].srv
                    , atoms[i].saxs_data.c
                    );
         }
         fclose(fsaxs_atoms);
      }
      cout << "should have written: " << fsaxs_atoms_name << endl;
         
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         
      editor->append(QString("Number of atoms %1.\n"
                             "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
                     .arg(atoms.size())
                     .arg(our_saxs_options->start_q)
                     .arg(our_saxs_options->end_q)
                     .arg(our_saxs_options->delta_q)
                     .arg(q_points));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
#if defined(SAXS_DEBUG)
      cout << "start q:" << our_saxs_options->start_q
           << " end q:" << our_saxs_options->end_q
           << " delta q:" << our_saxs_options->delta_q
           << " q points:" << q_points
           << endl;
#endif
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
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
         if ( q[j] < SAXS_MIN_Q ) 
         {
            q[j] = SAXS_MIN_Q;
         }
         q2[j] = q[j] * q[j];
      }

      float vi; // excluded water vol
      float vie; // excluded water * e density

      // compute form factors

      US_Saxs_Util usu;

      double delta_rho = ((US_Hydrodyn *)us_hydrodyn)->misc.target_e_density - our_saxs_options->water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options->bead_model_rayleigh )
      {
         editor_msg("blue", QString("using Rayleigh structure factors, delta_rho %1\n").arg(delta_rho));
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options->scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options->scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = bead_models[ current_model ][ i ].saxs_data; // = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;

         bool rayleigh_ok = false;
         if ( our_saxs_options->bead_model_rayleigh )
         {
            vector < double > F;
            vector < double > q(1);
            q[ 0 ] = 0;

      
            if ( usu.compute_rayleigh_structure_factors( 
                                                        atoms[ i ].radius * scaling_root,
                                                        delta_rho,
                                                        q,
                                                        F
                                                        ) )
            {
               fp[ 0 ][ i ] = F[ 0 ];
               rayleigh_ok = true;
            } else {
               editor_msg("red", "using Rayleigh structure factors failed:" + usu.errormsg );
            }
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               editor_msg("blue", "NOT using Rayleigh structure factors\n");
               told_you = true;
            }
         
            f[ 0 ][ i ] = compute_ff_bead_model( atoms[ i ].saxs_data, 0 );


            //             if ( our_saxs_options->five_term_gaussians )
            //             {
            //                f[0][i] = 
            //                   atoms[i].saxs_data.c5 +
            //                   atoms[i].saxs_data.a5[0] +
            //                   atoms[i].saxs_data.a5[1] +
            //                   atoms[i].saxs_data.a5[2] +
            //                   atoms[i].saxs_data.a5[3] +
            //                   atoms[i].saxs_data.a5[4]
            //                   ;
            //             } else {
            //                f[0][i] = 
            //                   atoms[i].saxs_data.c +
            //                   atoms[i].saxs_data.a[0] +
            //                   atoms[i].saxs_data.a[1] +
            //                   atoms[i].saxs_data.a[2] +
            //                   atoms[i].saxs_data.a[3];
            //             }

            
            fc[0][i] = vie;
            fp[0][i] = f[0][i] - fc[0][i];
         }
      }

      // foxs method: compute real space distribution

      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double rik2; // square distance from atom i to k 
      float delta = our_saxs_options->fast_bin_size;
      float one_over_delta = 1.0 / delta;
      float delta_pr = our_saxs_options->bin_size;
      float one_over_delta_pr = 1.0 / delta_pr;
      unsigned int pos;
      unsigned int pos_pr;
      vector < float > hist;
      vector < float > hist_pr;
      contrib_array.clear( );
      contrib_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
      // cout << "contrib_file " << contrib_file << endl;
      
      progress_saxs->setMaximum((int)(as1 * 1.15));
      if ( our_saxs_options->saxs_iq_native_fast_compute_pr )
      {
         pb_pr_contrib->setEnabled(false);
         if ( cb_pr_contrib->isChecked() &&
              !source &&
              contrib_file.contains(QRegExp("(PDB|pdb)$")) )
         {
            contrib_array.resize(atoms.size());
            for ( unsigned int i = 0; i < as1; i++ )
            {
               // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
               // cout << lcp << endl;
               // lbl_core_progress->setText(lcp);
               progress_saxs->setValue(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
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
               hist[0] += fp[0][i] * fp[0][i];
            }
            hist[0] += fp[0][as1] * fp[0][as1];
            pb_pr_contrib->setEnabled(true);
         } else {
            for ( unsigned int i = 0; i < as1; i++ )
            {
               // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
               // cout << lcp << endl;
               // lbl_core_progress->setText(lcp);
               progress_saxs->setValue(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
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
               hist[0] += fp[0][i] * fp[0][i];
            }
            hist[0] += fp[0][as1] * fp[0][as1];
         }         
         while( hist_pr.size() && !hist_pr[hist_pr.size()-1] ) 
         {
            hist_pr.pop_back();
         }
      } else {
         for ( unsigned int i = 0; i < as1; i++ )
         {
            // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
            // cout << lcp << endl;
            // lbl_core_progress->setText(lcp);
            progress_saxs->setValue(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(us_tr("Terminated by user request.\n"));
               progress_saxs->reset();
               lbl_core_progress->setText("");
               pb_plot_saxs_sans->setEnabled(true);
               pb_plot_pr->setEnabled(true);
               return;
            }
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
      if ( contrib_array.size() ) 
      {
         for ( unsigned int k = 0; k < contrib_array.size(); k++ )
         {
            contrib_array[k].resize(hist.size());
         }
      }
      //      for ( unsigned int i = 0; i < hist.size(); i++ )
      //      {
      //         cout << "hist[" << i << "] = " << hist[i] << endl;
      //      }

      // vector < float > save_hist = hist;
      // hist[0] = 0.0;

      editor->append(QString(us_tr("Total excluded volume %1\n")).arg(tot_excl_vol));

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1)
         .arg(iqq_suffix());

      QString plot_name = name;

      // save the data to a file
      if ( our_saxs_options->saxs_iq_native_fast_compute_pr )
      {
         if ( create_native_saxs )
         {
            QString fpr_name = 
               USglobal->config_list.root_dir + 
               SLASH + "somo" + SLASH + "saxs" + SLASH + sprr_filestring();
            
            bool ok_to_write = true;
            if ( QFile::exists(fpr_name) &&
                 !((US_Hydrodyn *)us_hydrodyn)->overwrite ) 
            {
               
               fpr_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fpr_name, 0, this);
               ok_to_write = true;
            }
            
            if ( ok_to_write )
            {
               FILE *fpr = us_fopen(fpr_name, "w");
               if ( fpr ) 
               {
                  editor->append(us_tr("P(r) curve file: ") + fpr_name + us_tr(" created.\n"));
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
                  vector < double > r;
                  vector < double > pr;
                  vector < double > pr_n;
                  r.resize(hist_pr.size());
                  pr.resize(hist_pr.size());
                  pr_n.resize(hist.size());
                  for ( unsigned int i = 0; i < hist_pr.size(); i++) 
                  {
                     r[i] = (double) i * delta_pr;
                     pr[i] = (double) hist_pr[i];
                     pr_n[i] = (double) hist_pr[i];
                  }
                  normalize_pr(r, &pr_n, get_mw(te_filename2->text(), false));
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                     QString("")
                     .sprintf(
                              "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                              , model_filename.toLatin1().data()
                              , US_Version.toLatin1().data()
                              , REVISION
                              , delta
, get_mw(te_filename2->text(), false)
                              , compute_pr_area(pr, r)
                              );
                  fprintf(fpr, "%s",
                          ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
                  fprintf(fpr, "r\tp(r)\tnorm. p(r)\n");
                  for ( unsigned int i = 0; i < hist_pr.size(); i++ )
                  {
                     if ( hist_pr[i] ) {
                        fprintf(fpr, "%.6e\t%.6e\t%.6e\n", r[i], pr[i], pr_n[i]);
                        ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
                        ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
                        ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
                     }
                  }
                  fclose(fpr);
               }
               else
               {
#if defined(PR_DEBUG)
                  cout << "can't create " << fpr_name << endl;
#endif
                  editor->append(us_tr("WARNING: Could not create PR curve file: ") + fpr_name + "\n");
                  // QMessageBox mb(us_tr("UltraScan Warning"),
                  //                us_tr("The output file ") + fpr_name + us_tr(" could not be created."), 
                  //                QMessageBox::Critical,
                  //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  // mb.exec();
                  QMessageBox::critical( this,
                                         us_tr("UltraScan Warning"),
                                         us_tr("The output file ") + fpr_name + us_tr(" could not be created."), 
                                         QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
               }
            }
         } else {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
            vector < double > r;
            vector < double > pr;
            vector < double > pr_n;
            r.resize(hist_pr.size());
            pr.resize(hist_pr.size());
            pr_n.resize(hist_pr.size());
            for ( unsigned int i = 0; i < hist_pr.size(); i++) 
            {
               r[i] = (double) i * delta_pr;
               pr[i] = (double) hist_pr[i];
               pr_n[i] = (double) hist_pr[i];
            }
            normalize_pr(r, &pr_n, get_mw(te_filename2->text(), false));
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
               QString("")
               .sprintf(
                        "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                        , model_filename.toLatin1().data()
                        , US_Version.toLatin1().data()
                        , REVISION
                        , delta
, get_mw(te_filename2->text(), false)
                        , compute_pr_area(pr, r)
                        );
            for ( unsigned int i = 0; i < hist_pr.size(); i++ )
            {
               if ( hist_pr[i] ) {
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.push_back(r[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.push_back(pr[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.push_back(pr_n[i]);
               }
            }
         }

         vector < double > r;
         vector < double > pr;
         r.resize(hist_pr.size());
         pr.resize(hist_pr.size());
         for ( unsigned int i = 0; i < hist_pr.size(); i++) 
         {
            r[i] = (double) i * delta_pr;
            pr[i] = (double) hist_pr[i];
#if defined(PR_DEBUG)
            printf("%e %e\n", r[i], pr[i]);
#endif
         }
         plotted_pr_not_normalized.push_back(pr);
         plotted_pr_mw.push_back((float)get_mw(te_filename2->text(), false));
         if ( cb_normalize->isChecked() )
         {
            normalize_pr(r, &pr, get_mw(te_filename2->text(),false));
         }

         plot_one_pr( r, pr, te_filename2->text() );
         compute_rg_to_progress( r, pr, te_filename2->text() );
      } // compute_pr

      progress_saxs->setValue( 1 ); progress_saxs->setMaximum( 2 );

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
         I[i] *= exp( - our_saxs_options->fast_modulation * q[i] * q[i] );
         // cout << "I[" << i << "] = " << I[i] << endl;
      }
         
      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      if ( plotted )
      {
         plotted = false;
      }
      editor->append("I(q) computed.\n");
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 e" << endl;
      sleep(1);
      cout << " sleep 1 e done" << endl;
#endif

      // scaling fields
      QString scaling_target = "";
      set_scaling_target( scaling_target );

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }

      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#if QT_VERSION < 0x040000
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();

#if QT_VERSION < 0x040000
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq.push_back( curve );
      curve->setStyle( QwtPlotCurve::Lines );
#endif
      plotted_q.push_back(q);
      {
         vector < double > q2(q.size());
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            q2[i] = q[i] * q[i];
         }
         plotted_q2.push_back(q2);
      }
      //      for ( unsigned int i = 0; i < q.size(); i++ )
      //      {
      //         cout << QString("q[%1] = %2  I[%3] = %4\n")
      //            .arg(i).arg(q[i]).arg(i).arg(I[i]);
      //      }
      plotted_I.push_back(I);
      push_back_zero_I_error();      
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      {
         // plotted_I[p][i] = log10(plotted_I[p][i]);
      }
      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

#if QT_VERSION < 0x040000
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
      curve->setSamples(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif
      //      for ( unsigned int i = 0; i < q.size(); i++ )
      //      {
      //         cout << QString("plotted_q[%1][%2] = %3  plotted_I[%4]I[%5] = %6\n")
      //    .arg(p)
      //            .arg(i)
      //            .arg(plotted_q[p][i])
      //            .arg(p)
      //            .arg(i)
      //            .arg(plotted_I[p][i]);
      //      }
      // plot_saxs->replot();
      cb_user_range->setChecked(false);
      cb_guinier->setChecked(true);
      rescale_plot();
      cb_guinier->setChecked(false);
      rescale_plot();

      // save the data to a file
      if ( create_native_saxs )
      {
         QString fsaxs_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + saxs_filestring();
#if defined(SAXS_DEBUG)
         cout << "output file " << fsaxs_name << endl;
#endif
         bool ok_to_write = true;
         if ( QFile::exists(fsaxs_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite )
         {
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              us_tr("Overwrite file:") + "SAXS I(q) vs. q" + us_tr("output file"),
                                              us_tr("The file named \"") + 
                                              saxs_filestring() +
                                              + us_tr("\" will be overwriten"),
                                              "&Ok",  "&Cancel", 0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 1
            case 0: // just go ahead
               ok_to_write = true;
               break;
            case 1: // Cancel clicked or Escape pressed
               ok_to_write = false;
               break;
            }
#endif
         }
         
         if ( ok_to_write )
         {
            FILE *fsaxs = us_fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(us_tr("SAXS curve file: ") + fsaxs_name + us_tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.toLatin1().data()
                           , US_Version.toLatin1().data()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
               for ( unsigned int i = 0; i < q.size(); i++ )
               {
                  fprintf(fsaxs, "%.6e\t%.6e\n", q[i], I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
               }
               fclose(fsaxs);
            } 
            else
            {
#if defined(SAXS_DEBUG)
               cout << "can't create " << fsaxs_name << endl;
#endif
               editor->append(us_tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
               //                QMessageBox::Critical,
               //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical(this,
                                     us_tr("UltraScan Warning"),
                                     us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
                                     QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , our_saxs_options->start_q
                     , our_saxs_options->end_q
                     , our_saxs_options->delta_q
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
         }
      }
   } // models

   if ( plotted )
   {
      plotted = false;
   }

   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

//  ------------------------------------------------------------------------------------------------------
void US_Hydrodyn_Saxs::calc_saxs_iq_native_debye_bead_model()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      double tot_excl_vol = 0e0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
         // new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.excl_vol = this_atom->saxs_excl_vol;
         if ( our_saxs_options->bead_models_use_bead_radius_ev )
         {
            new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow( this_atom->bead_computed_radius, 3 );
         }
         if ( our_saxs_options->bead_models_rho0_in_scat_factors )
         {
            new_atom.excl_vol = 0e0;
         }
         new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %s %.3f %.3f %.3f %.2f %.2f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
                    , atoms[i].hybrid_name.toLatin1().data()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol
                    , atoms[i].srv
                    , atoms[i].saxs_data.c
                    );
         }
         fclose(fsaxs_atoms);
      }
         
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         
      editor->append(QString("Number of atoms %1.\n"
                             "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
                     .arg(atoms.size())
                     .arg(our_saxs_options->start_q)
                     .arg(our_saxs_options->end_q)
                     .arg(our_saxs_options->delta_q)
                     .arg(q_points));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }

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
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
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

      US_Saxs_Util usu;

      double delta_rho = ((US_Hydrodyn *)us_hydrodyn)->misc.target_e_density - our_saxs_options->water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options->bead_model_rayleigh )
      {
         editor_msg("blue", QString("using Rayleigh structure factors, delta_rho %1\n").arg(delta_rho));
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options->scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options->scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = bead_models[ current_model ][ i ].saxs_data; // = saxs_map[atoms[i].saxs_name];
         //          if ( !i )
         //          {
         //             QString qs;
         //             if ( our_saxs_options->five_term_gaussians )
         //             {
         //                cout << QString( "atom 0 saxs_name %1 c %2\n" ).arg( saxs.saxs_name ).arg( saxs.c5 );
         //                for ( unsigned int i = 0; i < 5; i++ )
         //                {
         //                   qs += QString( "a%1=%2 b%3=%4 " ).arg( i ).arg( saxs.a5[ i ] ).arg( i ).arg( saxs.b5[ i ] );
         //                }
         //             } else {
         //                cout << QString( "atom 0 saxs_name %1 c %2\n" ).arg( saxs.saxs_name ).arg( saxs.c );
         //                for ( unsigned int i = 0; i < 4; i++ )
         //                {
         //                   qs += QString( "a%1=%2 b%3=%4 " ).arg( i ).arg( saxs.a[ i ] ).arg( i ).arg( saxs.b[ i ] );
         //                }
         //             }
         //             cout << qs << endl << flush;
         //          }
               
         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;
         
         bool rayleigh_ok = false;
         if ( our_saxs_options->bead_model_rayleigh )
         {
            vector < double > F;
            if ( usu.compute_rayleigh_structure_factors( 
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
               editor_msg("red", "using Rayleigh structure factors failed:" + usu.errormsg );
            }
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               editor_msg("blue", "NOT using Rayleigh structure factors\n");
               told_you = true;
            }
               
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               // note: since there are only a few 'saxs' coefficient sets
               // the saxs.c + saxs.a[i] * exp() can be precomputed
               // possibly saving time... but this isn't our most computationally intensive step
               // so I'm holding off for now.


               f[ j ][ i ] = compute_ff_bead_model( saxs, q_over_4pi_2[ j ] );

               //                if ( our_saxs_options->five_term_gaussians )
               //                {
               //                   f[j][i] = 
               //                      saxs.c5 + 
               //                      saxs.a5[0] * exp(-saxs.b5[0] * q_over_4pi_2[j]) +
               //                      saxs.a5[1] * exp(-saxs.b5[1] * q_over_4pi_2[j]) +
               //                      saxs.a5[2] * exp(-saxs.b5[2] * q_over_4pi_2[j]) +
               //                      saxs.a5[3] * exp(-saxs.b5[3] * q_over_4pi_2[j]) +
               //                      saxs.a5[4] * exp(-saxs.b5[4] * q_over_4pi_2[j]);
               //                } else {
               //                   f[j][i] = 
               //                      saxs.c + 
               //                      saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
               //                      saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
               //                      saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
               //                      saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]);
               //                }
               fc[j][i] =  vie * exp(vi_23_4pi * q2[j]);
               fp[j][i] = f[j][i] - fc[j][i];
#if defined(SAXS_DEBUG_F)
               if (1 || (q[j] > .0099 && q[j] < .0101)) {
                  cout << q[j] 
                       << "\t" 
                       << q2[j] 
                       << "\t" 
                       << f[j][i]
                       << "\n";
               }
#endif
#if defined(SAXS_DEBUG_FV)
               if (1 || (q[j] > .0099 && q[j] < .0101)) {
                  cout << q[j] 
                       << "\t" 
                       << q2[j] 
                       << "\t" 
                       << vi
                       << "\t" 
                       << vie
                       << "\t" 
                       << m_pi_vi23
                       << "\t" 
                       << m_pi_vi23 * q2[j]
                       << "\t" 
                       << vie * exp(m_pi_vi23 * q2[j])
                       << "\t" 
                       << fp[j][i]
                       << "\n";
               }
#endif
#if defined(ONLY_PHYSICAL_F)
               if ( fp[j][i] < 0.0f ) 
               {
                  fp[j][i] = 0.0f;
               }
#endif
            }
         }
#if defined(SAXS_DEBUG_F)
         cout << endl;
#endif
      }
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
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
      progress_saxs->setMaximum((int)(as1 * 1.15));
      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setValue(i+1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(us_tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }
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
#if defined(SAXS_DEBUG_F)
            cout << "dist atoms:  "
                 << i
                 << " "
                 << atoms[i].saxs_name
                 << ","
                 << k
                 << " "
                 << atoms[k].saxs_name
                 << " "
                 << rik
                 << endl;
#endif
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               qrik = rik * q[j];
               sqrikd = ( fabs(qrik) < 1e-16 ) ? 1.0 : sin(qrik) / qrik;
               I[j] += 2.0 * fp[j][i] * fp[j][k] * sqrikd;
               Ia[j] += 2.0 * f[j][i] * f[j][k] * sqrikd;
               Ic[j] += 2.0 * fc[j][i] * fc[j][k] * sqrikd;
#if defined(SAXS_DEBUG_F)
               cout << QString("").sprintf("I[%f] += (%f * %f) * (sin(%f) / %f) == %f\n"
                                           , q[j]
                                           , fp[j][i]
                                           , fp[j][k]
                                           , qrik
                                           , qrik
                                           , I[j]);
#endif
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

      //for ( unsigned int j = 0; j < q_points; j++ )
      //      {
      //         I[j] = I[j] > 0.0 ? log10(I[j]) : 0.0;
      //         Ia[j] = Ia[j] > 0.0 ? log10(Ia[j]) : 0.0;
      //         Ic[j] = Ic[j] > 0.0 ? log10(Ic[j]) : 0.0;
      //      }

      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      editor->append("I(q) computed.\n");
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 e" << endl;
      sleep(1);
      cout << " sleep 1 e done" << endl;
#endif

      // scaling fields
      QString scaling_target = "";
      set_scaling_target( scaling_target );

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }


      editor->append(QString(us_tr("Total excluded volume %1\n")).arg(tot_excl_vol));

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1)
         .arg(iqq_suffix());

      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#if QT_VERSION < 0x040000
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();

#if QT_VERSION < 0x040000
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq.push_back( curve );
      curve->setStyle( QwtPlotCurve::Lines );
#endif
      plotted_q.push_back(q);
      {
         vector < double > q2(q.size());
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            q2[i] = q[i] * q[i];
         }
         plotted_q2.push_back(q2);
      }
      plotted_I.push_back(I);
      push_back_zero_I_error();      
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      //      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      //      {
      //         plotted_I[p][i] = log10(plotted_I[p][i]);
      //      }

      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

#if QT_VERSION < 0x040000
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
      curve->setSamples(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif
      cb_user_range->setChecked(false);
      cb_guinier->setChecked(true);
      rescale_plot();
      cb_guinier->setChecked(false);
      rescale_plot();
      cb_guinier->setChecked(true);
      rescale_plot();
      cb_guinier->setChecked(false);
      rescale_plot();

      // save the data to a file
      if ( create_native_saxs )
      {
         QString fsaxs_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + saxs_filestring();
#if defined(SAXS_DEBUG)
         cout << "output file " << fsaxs_name << endl;
#endif
         bool ok_to_write = true;
         if ( QFile::exists(fsaxs_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite )
         {
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              us_tr("Overwrite file:") + "SAXS I(q) vs. q" + us_tr("output file"),
                                              us_tr("The file named \"") + 
                                              saxs_filestring() +
                                              + us_tr("\" will be overwriten"),
                                              "&Ok",  "&Cancel", 0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 1
            case 0: // just go ahead
               ok_to_write = true;
               break;
            case 1: // Cancel clicked or Escape pressed
               ok_to_write = false;
               break;
            }
#endif
         }
         
         if ( ok_to_write )
         {
            FILE *fsaxs = us_fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(us_tr("SAXS curve file: ") + fsaxs_name + us_tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.toLatin1().data()
                           , US_Version.toLatin1().data()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
               for ( unsigned int i = 0; i < q.size(); i++ )
               {
                  fprintf(fsaxs, "%.6e\t%.6e\t%.6e\t%.6e\n", q[i], I[i], Ia[i], Ic[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(Ia[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(Ic[i]);
               }
               fclose(fsaxs);
            } 
            else
            {
#if defined(SAXS_DEBUG)
               cout << "can't create " << fsaxs_name << endl;
#endif
               editor->append(us_tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
               //                QMessageBox::Critical,
               //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical( this,
                                      us_tr("UltraScan Warning"),
                                      us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
                                      QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , our_saxs_options->start_q
                     , our_saxs_options->end_q
                     , our_saxs_options->delta_q
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(Ia[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(Ic[i]);
         }
      }
   }
   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

//  ------------------------------------------------------------------------------------------------------
void US_Hydrodyn_Saxs::calc_saxs_iq_native_hybrid2_bead_model()
{
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      double tot_excl_vol = 0e0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg(current_model + 1));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
         // new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow(this_atom->bead_computed_radius, 3);
         new_atom.excl_vol = this_atom->saxs_excl_vol;
         if ( our_saxs_options->bead_models_use_bead_radius_ev )
         {
            new_atom.excl_vol = ( 4.0 / 3.0 ) * M_PI * pow( this_atom->bead_computed_radius, 3 );
         }
         if ( our_saxs_options->bead_models_rho0_in_scat_factors )
         {
            new_atom.excl_vol = 0e0;
         }
         new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
         new_atom.srv = sqrt( new_atom.excl_vol / this_atom->saxs_data.volume );
         new_atom.saxs_data = this_atom->saxs_data;
         tot_excl_vol += new_atom.excl_vol;
         atoms.push_back(new_atom);
      }

      if ( atoms.size() == 1 )
      {
         editor_msg( "dark blue", us_tr( "Notice: using full Debye since only 1 bead available" ) );
         return calc_saxs_iq_native_debye_bead_model();
      }

      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %s %.3f %.3f %.3f %.2f %.2f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
                    , atoms[i].hybrid_name.toLatin1().data()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol
                    , atoms[i].srv
                    , atoms[i].saxs_data.c
                    );
         }
         fclose(fsaxs_atoms);
      }
         
      // ok now we have all the atoms
      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
         
      editor->append(QString("Number of atoms %1.\n"
                             "q range %2 to %3 with a stepsize of %4 giving %5 q-points.\n")
                     .arg(atoms.size())
                     .arg(our_saxs_options->start_q)
                     .arg(our_saxs_options->end_q)
                     .arg(our_saxs_options->delta_q)
                     .arg(q_points));
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
#if defined(SAXS_DEBUG)
      cout << "start q:" << our_saxs_options->start_q
           << " end q:" << our_saxs_options->end_q
           << " delta q:" << our_saxs_options->delta_q
           << " q points:" << q_points
           << endl;
#endif
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
         q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
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

#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 c" << endl;
      sleep(1);
      cout << " sleep 1 c done" << endl;
#endif
#if defined(SAXS_DEBUG_F)
      cout << "atom #\tsaxs name\tq:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q[j] << "\t";
         }
      }
      cout << endl;
      cout << "\t\tq^2:";
      for ( unsigned int j = 0; j < q_points; j++ )
      {
         if (1 || (q[j] > .0099 && q[j] < .0101)) {
            cout << q2[j] << "\t";
         }
      }
      cout << endl;
#endif
      saxs saxsH = saxs_map["H"];

      // compute form factors

      US_Saxs_Util usu;

      double delta_rho = ((US_Hydrodyn *)us_hydrodyn)->misc.target_e_density - our_saxs_options->water_e_density;
      if ( fabs(delta_rho) < 1e-5 )
      {
         delta_rho = 0e0;
      }

      if ( our_saxs_options->bead_model_rayleigh )
      {
         editor_msg("blue", QString("using Rayleigh structure factors, delta_rho %1\n").arg(delta_rho));
      }

      bool told_you = false;

      double scaling_root = 1e0;
      if ( our_saxs_options->scale_excl_vol != 1e0 )
      {
         scaling_root = pow( (double) our_saxs_options->scale_excl_vol, 1e0/3e0 );
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = bead_models[ current_model ][ i ].saxs_data; // = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;

#if defined(SAXS_DEBUG_FX)
         cout << i << "\t"
              << atoms[i].saxs_name << "\t";
         cout << QString("").sprintf("a1 %f b1 %f a2 %f b2 %f a3 %f b3 %f a4 %f b4 %f c %f\n"
                                     , saxs.a[0] , saxs.b[0]
                                     , saxs.a[1] , saxs.b[1]
                                     , saxs.a[2] , saxs.b[2]
                                     , saxs.a[3] , saxs.b[3]
                                     , saxs.c);
#endif
         
         bool rayleigh_ok = false;
         if ( our_saxs_options->bead_model_rayleigh )
         {
            vector < double > F;
            if ( usu.compute_rayleigh_structure_factors( 
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
               editor_msg("red", "using Rayleigh structure factors failed:" + usu.errormsg );
            }
         }

         if ( !rayleigh_ok )
         {
            if ( !told_you )
            {
               editor_msg("blue", "NOT using Rayleigh structure factors\n");
               told_you = true;
            }
               
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               // note: since there are only a few 'saxs' coefficient sets
               // the saxs.c + saxs.a[i] * exp() can be precomputed
               // possibly saving time... but this isn't our most computationally intensive step
               // so I'm holding off for now.
               
               f[ j ][ i ] = compute_ff_bead_model( saxs, q_over_4pi_2[ j ] );

               //                if ( our_saxs_options->five_term_gaussians )
               //                {
               //                   f[j][i] = 
               //                      saxs.c5 + 
               //                      saxs.a5[0] * exp(-saxs.b5[0] * q_over_4pi_2[j]) +
               //                      saxs.a5[1] * exp(-saxs.b5[1] * q_over_4pi_2[j]) +
               //                      saxs.a5[2] * exp(-saxs.b5[2] * q_over_4pi_2[j]) +
               //                      saxs.a5[3] * exp(-saxs.b5[3] * q_over_4pi_2[j]) +
               //                      saxs.a5[4] * exp(-saxs.b5[4] * q_over_4pi_2[j]);
               //                } else {
               //                   f[j][i] = 
               //                      saxs.c + 
               //                      saxs.a[0] * exp(-saxs.b[0] * q_over_4pi_2[j]) +
               //                      saxs.a[1] * exp(-saxs.b[1] * q_over_4pi_2[j]) +
               //                      saxs.a[2] * exp(-saxs.b[2] * q_over_4pi_2[j]) +
               //                      saxs.a[3] * exp(-saxs.b[3] * q_over_4pi_2[j]);
               //                }

               fc[j][i] =  vie * exp(vi_23_4pi * q2[j]);
               fp[j][i] = f[j][i] - fc[j][i];
#if defined(SAXS_DEBUG_F)
               if (1 || (q[j] > .0099 && q[j] < .0101)) {
                  cout << q[j] 
                       << "\t" 
                       << q2[j] 
                       << "\t" 
                       << f[j][i]
                       << "\n";
               }
#endif
#if defined(SAXS_DEBUG_FV)
               if (1 || (q[j] > .0099 && q[j] < .0101)) {
                  cout << q[j] 
                       << "\t" 
                       << q2[j] 
                       << "\t" 
                       << vi
                       << "\t" 
                       << vie
                       << "\t" 
                       << m_pi_vi23
                       << "\t" 
                       << m_pi_vi23 * q2[j]
                       << "\t" 
                       << vie * exp(m_pi_vi23 * q2[j])
                       << "\t" 
                       << fp[j][i]
                       << "\n";
               }
#endif
#if defined(ONLY_PHYSICAL_F)
               if ( fp[j][i] < 0.0f ) 
               {
                  fp[j][i] = 0.0f;
               }
#endif
            }
#if defined(SAXS_DEBUG_F)
            cout << endl;
#endif
         }
      }
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(us_tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }

      // foxs method: compute real space distribution

      unsigned int as = atoms.size();
      unsigned int as1 = as - 1;
      double rik; // distance from atom i to k 
      double rik2; // square distance from atom i to k 
      float delta = our_saxs_options->fast_bin_size;
      float one_over_delta = 1.0 / delta;
      unsigned int pos;
      vector < float > hist;
      
      progress_saxs->setMaximum((int)(as * 2 + 2));

      // vector < vector < double > > rik_array(as);

      // for ( unsigned int i = 0; i < rik_array.size(); i++ )
      // {
      // rik_array[i].resize(atoms.size());
      // }

      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setValue(i+1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(us_tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }
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
         I[i] *= exp( - our_saxs_options->fast_modulation * q[i] * q[i] );
      }

      // -------------------------------
      vector < unsigned int > r;
      bool adaptive_ok = false;

      if ( our_saxs_options->saxs_iq_hybrid_adaptive )
      {
         US_Saxs_Util usu;
         if ( !usu.create_adaptive_grid( q, I, our_saxs_options->hybrid2_q_points, r ) )
         {
            editor_msg( "red", usu.errormsg );
         } else {
            adaptive_ok = true;
         }
         if ( !usu.noticemsg.isEmpty() )
         {
            editor_msg( "black", usu.noticemsg );
         }
      }

      unsigned int q_delta = q_points / our_saxs_options->hybrid2_q_points;
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

      if ( our_saxs_options->saxs_iq_hybrid_adaptive && adaptive_ok )
      {
         use_q = r;
      }

      unsigned int use_q_size = use_q.size();

      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik
      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setValue(as + i + 1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(us_tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }
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
#if defined(SAXS_DEBUG_F)
               cout << QString("").sprintf("I[%f] += (%f * %f) * (sin(%f) / %f) == %f\n"
                                           , q[j]
                                           , fp[j][i]
                                           , fp[j][k]
                                           , qrik
                                           , qrik
                                           , I[j]);
#endif
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

#if defined(MAKE_TRIAL_PLOT_FOR_BEAD_MODEL_HYBRID)
      {
         vector < double > trialI;
         vector < double > griduseq;
         for ( unsigned int l = 0; l < use_q_size; l++ )
         {
            unsigned int j = use_q[l];
            griduseq.push_back(q[j]);
            trialI.push_back(I[j]);
         }
         plot_one_iqq( griduseq, trialI, "trial");
      }
#endif

      {
         US_Saxs_Util usu;
         if ( our_saxs_options->saxs_iq_native_hybrid )
         {
            if ( !usu.linear_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
            {
               cout << usu.errormsg << endl;
            }
         } else {
            if ( our_saxs_options->saxs_iq_native_hybrid2 )
            {
               if ( !usu.quadratic_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << usu.errormsg << endl;
               }
            } else {
               if ( !usu.cubic_spline_interpolate_iq_curve( q, use_q, fast_I, I, I ) )
               {
                  cout << usu.errormsg << endl;
               }
            }               
         }
      }

      // for ( unsigned int l = 0; l < use_q_size - 1; l++ )
      // {
      // double merge_points = (double)(1 + use_q[l + 1] - use_q[l]);
      // for ( unsigned int j = use_q[l]; j < use_q[l+1]; j++ )
      // {
      // double x = (double)(j - use_q[l]) / merge_points;
      // I[j] = fast_I[j] + deltaI[l] * (1.0 - x) + deltaI[l+1] * x;
      // }
      // }

      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      editor->append("I(q) computed.\n");
#if defined(BUG_DEBUG)
      qApp->processEvents();
      cout << " sleep 1 e" << endl;
      sleep(1);
      cout << " sleep 1 e done" << endl;
#endif

      // scaling fields
      QString scaling_target = "";
      set_scaling_target( scaling_target );

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }


      editor->append(QString(us_tr("Total excluded volume %1\n")).arg(tot_excl_vol));

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1)
         .arg(iqq_suffix());

      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#if QT_VERSION < 0x040000
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();

#if QT_VERSION < 0x040000
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq.push_back( curve );
      curve->setStyle( QwtPlotCurve::Lines );
#endif
      plotted_q.push_back(q);
      {
         vector < double > q2(q.size());
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            q2[i] = q[i] * q[i];
         }
         plotted_q2.push_back(q2);
      }
      plotted_I.push_back(I);
      push_back_zero_I_error();      
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      //      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      //      {
      //         plotted_I[p][i] = log10(plotted_I[p][i]);
      //      }

      vector < double > q2I;
      if ( cb_kratky->isChecked() )
      {
         for ( unsigned int i = 0; i < plotted_q[ p ].size(); i++ )
         {
            q2I.push_back( plotted_q2[ p ][ i ] * plotted_I[ p ][ i ] );
         }
      }

#if QT_VERSION < 0x040000
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], pen_width, SolidLine));
#else
      curve->setSamples(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], pen_width, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif
      cb_user_range->setChecked(false);
      cb_guinier->setChecked(true);
      rescale_plot();
      cb_guinier->setChecked(false);
      rescale_plot();
      cb_guinier->setChecked(true);
      rescale_plot();
      cb_guinier->setChecked(false);
      rescale_plot();

      // save the data to a file
      if ( create_native_saxs )
      {
         QString fsaxs_name = 
            USglobal->config_list.root_dir + 
            SLASH + "somo" + SLASH + "saxs" + SLASH + saxs_filestring();
#if defined(SAXS_DEBUG)
         cout << "output file " << fsaxs_name << endl;
#endif
         bool ok_to_write = true;
         if ( QFile::exists(fsaxs_name) &&
              !((US_Hydrodyn *)us_hydrodyn)->overwrite )
         {
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              us_tr("Overwrite file:") + "SAXS I(q) vs. q" + us_tr("output file"),
                                              us_tr("The file named \"") + 
                                              saxs_filestring() +
                                              + us_tr("\" will be overwriten"),
                                              "&Ok",  "&Cancel", 0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 1
            case 0: // just go ahead
               ok_to_write = true;
               break;
            case 1: // Cancel clicked or Escape pressed
               ok_to_write = false;
               break;
            }
#endif
         }
         
         if ( ok_to_write )
         {
            FILE *fsaxs = us_fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(us_tr("SAXS curve file: ") + fsaxs_name + us_tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.toLatin1().data()
                           , US_Version.toLatin1().data()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.toLatin1().data() );
               for ( unsigned int i = 0; i < q.size(); i++ )
               {
                  fprintf(fsaxs, "%.6e\t%.6e\n", q[i], I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
               }
               fclose(fsaxs);
            } 
            else
            {
#if defined(SAXS_DEBUG)
               cout << "can't create " << fsaxs_name << endl;
#endif
               editor->append(us_tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               // QMessageBox mb(us_tr("UltraScan Warning"),
               //                us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
               //                QMessageBox::Critical,
               //                QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               // mb.exec();
               QMessageBox::critical( this,
                                      us_tr("UltraScan Warning"),
                                      us_tr("The output file ") + fsaxs_name + us_tr(" could not be created."), 
                                      QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton );
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear( );
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.toLatin1().data()
                     , US_Version.toLatin1().data()
                     , REVISION
                     , our_saxs_options->start_q
                     , our_saxs_options->end_q
                     , our_saxs_options->delta_q
                     );
         for ( unsigned int i = 0; i < q.size(); i++ )
         {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
         }
      }
   }
   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

double US_Hydrodyn_Saxs::compute_ff_bead_model
(
 saxs     &s, 
 double   q_o_4pi2 
 )
{
   // cout << "compute_ff_bead_model\n";
   if ( our_saxs_options->bead_models_use_var_len_sf && s.vcoeff.size() )
   {
      // cout << "using vcoeff\n";
      double val = s.vcoeff[ 0 ];
      for ( unsigned int i = 1; i < ( unsigned int )s.vcoeff.size() - 1; i += 2 )
      {
         val += s.vcoeff[ i ] * exp( - s.vcoeff[ i + 1 ] * q_o_4pi2 );
      }
      return val;
   } else {
      //       if ( s.vcoeff.size() == 0 )
      //       {
      //          cout << "NOT using vcoeff (vcoeff size 0)\n";
      //       } else {
      //          cout << "NOT using vcoeff\n";
      //       }
      return our_saxs_options->five_term_gaussians ?
         s.c5 + 
         s.a5[ 0 ] * exp( - s.b5[ 0 ] * q_o_4pi2 ) +
         s.a5[ 1 ] * exp( - s.b5[ 1 ] * q_o_4pi2 ) +
         s.a5[ 2 ] * exp( - s.b5[ 2 ] * q_o_4pi2 ) +
         s.a5[ 3 ] * exp( - s.b5[ 3 ] * q_o_4pi2 ) +
         s.a5[ 4 ] * exp( - s.b5[ 4 ] * q_o_4pi2 ) 
         :
         s.c + 
         s.a[ 0 ] * exp( - s.b[ 0 ] * q_o_4pi2 ) +
         s.a[ 1 ] * exp( - s.b[ 1 ] * q_o_4pi2 ) +
         s.a[ 2 ] * exp( - s.b[ 2 ] * q_o_4pi2 ) +
         s.a[ 3 ] * exp( - s.b[ 3 ] * q_o_4pi2 )
         ;
   }
}
