#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_saxs_util.h"

#define SLASH "/"
#if defined(WIN32)
#  include <dos.h>
#  include <stdlib.h>
#  include <float.h>
#  undef SLASH
#  define SLASH "\\"
#  define isnan _isnan
#endif

#define SAXS_MIN_Q 1e-6
// #define SAXS_DEBUG
// #define SAXS_DEBUG_F
// #define SAXS_DEBUG_FF
// #define SAXS_DEBUG2

void US_Hydrodyn_Saxs::calc_saxs_iq_native_fast()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry

   // cout << "plotted_I.size() " << plotted_I.size() << endl;
   // cout << "plotted_q.size() " << plotted_q.size() << endl;
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
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
            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            } else {
               // for FoXS method
               // new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.ascii()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol);
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
         editor->append(tr("Terminated by user request.\n"));
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
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
      // compute form factors
      saxs saxsH = saxs_map["H"];
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
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
         
         // note: since there are only a few 'saxs' coefficient sets
         // the saxs.c + saxs.a[i] * exp() can be precomputed
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
         

#if defined(SAXS_DEBUG_FF)
         cout << 
            QString("ff[%1] %2 %3 %4\n")
            .arg( i )
            .arg( f[0][i] )
            .arg( fc[0][i] )
            .arg( fp[0][i] )
            ;
#endif
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
      contrib_array.clear();
      contrib_file = ((US_Hydrodyn *)us_hydrodyn)->pdb_file;
      // cout << "contrib_file " << contrib_file << endl;
      
      progress_saxs->setTotalSteps((int)(as1 * 1.15));
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
               progress_saxs->setProgress(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
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
               progress_saxs->setProgress(i+1);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
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
            progress_saxs->setProgress(i+1);
            qApp->processEvents();
            if ( stopFlag ) 
            {
               editor->append(tr("Terminated by user request.\n"));
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

      QString name = 
         QString("%1_%2")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1);

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
               
               fpr_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fpr_name);
               ok_to_write = true;
            }
            
            if ( ok_to_write )
            {
               FILE *fpr = fopen(fpr_name, "w");
               if ( fpr ) 
               {
                  editor->append(tr("P(r) curve file: ") + fpr_name + tr(" created.\n"));
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
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
                  normalize_pr(r, &pr_n, get_mw(te_filename2->text(), false));
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                     QString("")
                     .sprintf(
                              "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                              , model_filename.ascii()
                              , US_Version.ascii()
                              , REVISION
                              , delta
                              , get_mw(te_filename2->text(), false)
                              , compute_pr_area(pr, r)
                              );
                  fprintf(fpr, "%s",
                          ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
                  editor->append(tr("WARNING: Could not create PR curve file: ") + fpr_name + "\n");
                  QMessageBox mb(tr("UltraScan Warning"),
                                 tr("The output file ") + fpr_name + tr(" could not be created."), 
                                 QMessageBox::Critical,
                                 QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
                  mb.exec();
               }
            }
         } else {
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_mw = get_mw(te_filename2->text(), false);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
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
            normalize_pr(r, &pr_n, get_mw(te_filename2->text(), false));
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
               QString("")
               .sprintf(
                        "SOMO p(r) vs r data generated from %s by US_SOMO %s %s bin size %f mw %.2f Daltons area %.2f\n"
                        , model_filename.ascii()
                        , US_Version.ascii()
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
            r[i] = i * delta_pr;
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
         
      } // compute_pr

      progress_saxs->setProgress( 1, 2 );

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
         editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
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
      double scaling_a;
      double scaling_b;
      double scaling_siga;
      double scaling_sigb;
      double scaling_chi2;

      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         scaling_target = QInputDialog::getItem(
                                                tr("Scale I(q) Curve"),
                                                tr("Select the target plotted data set for scaling the loaded data:\n"
                                                   "or Cancel of you do not wish to scale")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                FALSE, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            scaling_target = "";
         }
      }         

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
         cout << "scaling target pos is " << iq_pos << endl;
         double target_q_min = plotted_q[iq_pos][0];
         double target_q_max = plotted_q[iq_pos][plotted_q[iq_pos].size() - 1];
         double source_q_min = q[0];
         double source_q_max = q[q.size() - 1];
         double q_min = target_q_min;
         if ( q_min < source_q_min )
         {
            q_min = source_q_min;
         }
         double q_max = target_q_max;
         if ( q_max > source_q_max )
         {
            q_max = source_q_max;
         }

         cout << QString(
                         "target q_min %1 max %2\n"
                         "source q_min %3 max %4\n"
                         "select q_min %5 max %6\n"
                         )
            .arg(target_q_min)
            .arg(target_q_max)
            .arg(source_q_min)
            .arg(source_q_max)
            .arg(q_min)
            .arg(q_max)
            ;

         vector < double > use_q;
         vector < double > use_I;
         for ( unsigned int i = 0; i < plotted_q[iq_pos].size(); i++ )
         {
            if ( plotted_q[iq_pos][i] >= q_min &&
                 plotted_q[iq_pos][i] <= q_max )
            {
               use_q.push_back(plotted_q[iq_pos][i]);
               use_I.push_back(plotted_I[iq_pos][i]);
            }
         }
         
         cout << QString("After cropping q to overlap region:\n"
                         "use_q.size == %1\n").arg(use_q.size());

         if ( !use_q.size() )
         {
            QMessageBox::warning( this, "UltraScan",
                                  QString(tr("Could not find sufficient q range overlap\n"
                                             "to scale the loaded data to the selected target")) );
         } else {
            vector < double > use_source_I = interpolate(use_q, q, I);

            US_Saxs_Util usu;

            usu.linear_fit(
                           use_source_I, 
                           use_I, 
                           scaling_a,
                           scaling_b,
                           scaling_siga,
                           scaling_sigb,
                           scaling_chi2
                           );

            QString results = 
               QString("Scaling factor: %1  Offset: %2  Chi^2: %3\n")
               .arg(scaling_b)
               .arg(scaling_a)
               .arg(scaling_chi2);
            editor->append(results);
            // cout << "saxs curve unscaled " << vector_double_to_csv(I) << endl;
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[i] = scaling_a + scaling_b * I[i];
               if ( I[i] <= 0e0 )
               {
                  I.resize(i);
                  q.resize(i);
                  break;
               }
            }
         }
      }

#ifndef QT4
      long Iq = plot_saxs->insertCurve("I(q) vs q");
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
#endif
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#ifndef QT4
      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();
#else
      plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();
#endif

#ifndef QT4
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq_curves.push_back( curve );
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
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      {
         // plotted_I[p][i] = log10(plotted_I[p][i]);
      }
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
      curve->attach( plot_saxs );
#endif
      //      for ( unsigned int i = 0; i < q.size(); i++ )
      //      {
      //         cout << QString("plotted_q[%1][%1] = %1  plotted_I[%1]I[%1] = %1\n")
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              tr("Overwrite file:") + "SAXS I(q) vs. q" + tr("output file"),
                                              tr("The file named \"") + 
                                              saxs_filestring() +
                                              + tr("\" will be overwriten"),
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
            FILE *fsaxs = fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
               editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.ascii()
                     , US_Version.ascii()
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
      editor->setParagraphBackgroundColor ( editor->paragraphs() - 1, QColor("white") );
      plotted = false;
   }

   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

//  ------------------------------------------------------------------------------------------------------
void US_Hydrodyn_Saxs::calc_saxs_iq_native_debye()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.ascii()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol);
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
         editor->append(tr("Terminated by user request.\n"));
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
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
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
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
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
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
      progress_saxs->setTotalSteps((int)(as1 * 1.15));
      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setProgress(i+1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
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
      double scaling_a;
      double scaling_b;
      double scaling_siga;
      double scaling_sigb;
      double scaling_chi2;

      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         scaling_target = QInputDialog::getItem(
                                                tr("Scale I(q) Curve"),
                                                tr("Select the target plotted data set for scaling the loaded data:\n"
                                                   "or Cancel of you do not wish to scale")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                FALSE, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            scaling_target = "";
         }
      }         

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
         cout << "scaling target pos is " << iq_pos << endl;
         double target_q_min = plotted_q[iq_pos][0];
         double target_q_max = plotted_q[iq_pos][plotted_q[iq_pos].size() - 1];
         double source_q_min = q[0];
         double source_q_max = q[q.size() - 1];
         double q_min = target_q_min;
         if ( q_min < source_q_min )
         {
            q_min = source_q_min;
         }
         double q_max = target_q_max;
         if ( q_max > source_q_max )
         {
            q_max = source_q_max;
         }

         cout << QString(
                         "target q_min %1 max %2\n"
                         "source q_min %3 max %4\n"
                         "select q_min %5 max %6\n"
                         )
            .arg(target_q_min)
            .arg(target_q_max)
            .arg(source_q_min)
            .arg(source_q_max)
            .arg(q_min)
            .arg(q_max)
            ;

         vector < double > use_q;
         vector < double > use_I;
         for ( unsigned int i = 0; i < plotted_q[iq_pos].size(); i++ )
         {
            if ( plotted_q[iq_pos][i] >= q_min &&
                 plotted_q[iq_pos][i] <= q_max )
            {
               use_q.push_back(plotted_q[iq_pos][i]);
               use_I.push_back(plotted_I[iq_pos][i]);
            }
         }
         
         cout << QString("After cropping q to overlap region:\n"
                         "use_q.size == %1\n").arg(use_q.size());

         if ( !use_q.size() )
         {
            QMessageBox::warning( this, "UltraScan",
                                  QString(tr("Could not find sufficient q range overlap\n"
                                             "to scale the loaded data to the selected target")) );
         } else {
            vector < double > use_source_I = interpolate(use_q, q, I);

            US_Saxs_Util usu;

            usu.linear_fit(
                           use_source_I, 
                           use_I, 
                           scaling_a,
                           scaling_b,
                           scaling_siga,
                           scaling_sigb,
                           scaling_chi2
                           );

            QString results = 
               QString("Scaling factor: %1  Offset: %2  Chi^2: %3\n")
               .arg(scaling_b)
               .arg(scaling_a)
               .arg(scaling_chi2);
            editor->append(results);
            // cout << "saxs curve unscaled " << vector_double_to_csv(I) << endl;
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[i] = scaling_a + scaling_b * I[i];
               if ( I[i] <= 0e0 )
               {
                  I.resize(i);
                  q.resize(i);
                  break;
               }
            }
         }
      }


#ifndef QT4
      long Iq = plot_saxs->insertCurve("I(q) vs q");
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
#endif

      QString name = 
         QString("%1_%2")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1);
      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#ifndef QT4
      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();
#else
      plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();
#endif

#ifndef QT4
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq_curves.push_back( curve );
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
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      //      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      //      {
      //         plotted_I[p][i] = log10(plotted_I[p][i]);
      //      }
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              tr("Overwrite file:") + "SAXS I(q) vs. q" + tr("output file"),
                                              tr("The file named \"") + 
                                              saxs_filestring() +
                                              + tr("\" will be overwriten"),
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
            FILE *fsaxs = fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
               editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.ascii()
                     , US_Version.ascii()
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
void US_Hydrodyn_Saxs::calc_saxs_iq_native_hybrid()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.ascii()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol);
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
         editor->append(tr("Terminated by user request.\n"));
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
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
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
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
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
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
      
      progress_saxs->setTotalSteps((int)(as * 2 + 2));

      vector < vector < double > > rik_array(as);

      for ( unsigned int i = 0; i < rik_array.size(); i++ )
      {
         rik_array[i].resize(atoms.size());
      }

      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setProgress(i+1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
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
            rik_array[i][k] = sqrt(rik2);

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

      unsigned int q_start = 
         (unsigned int)floor(((our_saxs_options->hybrid_q_point - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) - 1;

      vector < double > fast_I = I;
      for ( unsigned int j = q_start; j < q_points; j++ )
      {
         I[j] = 0.0f;
      }
      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik
      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setProgress(as + i + 1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = rik_array[i][k]; 
            for ( unsigned int j = q_start; j < q_points; j++ )
            {
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
         for ( unsigned int j = q_start; j < q_points; j++ )
         {
            I[j] += fp[j][i] * fp[j][i];
         }
      }
      for ( unsigned int j = q_start; j < q_points; j++ )
      {
         I[j] += fp[j][as1] * fp[j][as1];
      }

      unsigned int merge_points = 5;       // 5 point merge (probably should configure)
      if ( q_start + merge_points < q_points ) {
         for ( unsigned int j = 0; j < merge_points; j++ )
         {
            double x = (double)j / merge_points;
            I[q_start + j] = fast_I[q_start + j] * (1.0 - x) + I[q_start + j] * x;
         }
      }

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
      double scaling_a;
      double scaling_b;
      double scaling_siga;
      double scaling_sigb;
      double scaling_chi2;

      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         scaling_target = QInputDialog::getItem(
                                                tr("Scale I(q) Curve"),
                                                tr("Select the target plotted data set for scaling the loaded data:\n"
                                                   "or Cancel of you do not wish to scale")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                FALSE, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            scaling_target = "";
         }
      }         

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
         cout << "scaling target pos is " << iq_pos << endl;
         double target_q_min = plotted_q[iq_pos][0];
         double target_q_max = plotted_q[iq_pos][plotted_q[iq_pos].size() - 1];
         double source_q_min = q[0];
         double source_q_max = q[q.size() - 1];
         double q_min = target_q_min;
         if ( q_min < source_q_min )
         {
            q_min = source_q_min;
         }
         double q_max = target_q_max;
         if ( q_max > source_q_max )
         {
            q_max = source_q_max;
         }

         cout << QString(
                         "target q_min %1 max %2\n"
                         "source q_min %3 max %4\n"
                         "select q_min %5 max %6\n"
                         )
            .arg(target_q_min)
            .arg(target_q_max)
            .arg(source_q_min)
            .arg(source_q_max)
            .arg(q_min)
            .arg(q_max)
            ;

         vector < double > use_q;
         vector < double > use_I;
         for ( unsigned int i = 0; i < plotted_q[iq_pos].size(); i++ )
         {
            if ( plotted_q[iq_pos][i] >= q_min &&
                 plotted_q[iq_pos][i] <= q_max )
            {
               use_q.push_back(plotted_q[iq_pos][i]);
               use_I.push_back(plotted_I[iq_pos][i]);
            }
         }
         
         cout << QString("After cropping q to overlap region:\n"
                         "use_q.size == %1\n").arg(use_q.size());

         if ( !use_q.size() )
         {
            QMessageBox::warning( this, "UltraScan",
                                  QString(tr("Could not find sufficient q range overlap\n"
                                             "to scale the loaded data to the selected target")) );
         } else {
            vector < double > use_source_I = interpolate(use_q, q, I);

            US_Saxs_Util usu;

            usu.linear_fit(
                           use_source_I, 
                           use_I, 
                           scaling_a,
                           scaling_b,
                           scaling_siga,
                           scaling_sigb,
                           scaling_chi2
                           );

            QString results = 
               QString("Scaling factor: %1  Offset: %2  Chi^2: %3\n")
               .arg(scaling_b)
               .arg(scaling_a)
               .arg(scaling_chi2);
            editor->append(results);
            // cout << "saxs curve unscaled " << vector_double_to_csv(I) << endl;
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[i] = scaling_a + scaling_b * I[i];
               if ( I[i] <= 0e0 )
               {
                  I.resize(i);
                  q.resize(i);
                  break;
               }
            }
         }
      }


#ifndef QT4
      long Iq = plot_saxs->insertCurve("I(q) vs q");
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
#endif

      QString name = 
         QString("%1_%2")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1);
      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#ifndef QT4
      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();
#else
      plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();
#endif

#ifndef QT4
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq_curves.push_back( curve );
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
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      //      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      //      {
      //         plotted_I[p][i] = log10(plotted_I[p][i]);
      //      }
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              tr("Overwrite file:") + "SAXS I(q) vs. q" + tr("output file"),
                                              tr("The file named \"") + 
                                              saxs_filestring() +
                                              + tr("\" will be overwriten"),
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
            FILE *fsaxs = fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
               editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.ascii()
                     , US_Version.ascii()
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

//  ------------------------------------------------------------------------------------------------------
void US_Hydrodyn_Saxs::calc_saxs_iq_native_hybrid2()
{
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();
   QRegExp count_hydrogens("H(\\d)");

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
         
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq));
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            if ( !atom_map.count(this_atom->name + "~" + hybrid_name) )
            {
               cout << "error: atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom->name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->name)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( count_hydrogens.search(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(this_atom->resName)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              .arg(hybrid_map[hybrid_name].saxs_name)
                              );
               editor->setColor(save_color);
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return;
               }
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << this_atom->resName
                 << " SAXS atom: "
                 << new_atom.saxs_name 
                 << " Coordinates: "
                 << new_atom.pos[0] << " , "
                 << new_atom.pos[1] << " , "
                 << new_atom.pos[2] 
                 << " Excl vol: "
                 << new_atom.excl_vol
                 << endl;
#endif
            atoms.push_back(new_atom);
         }
      }
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(current_model + 1) + 
         ".atoms";

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.ascii()
                    , atoms[i].pos[0]
                    , atoms[i].pos[1]
                    , atoms[i].pos[2]
                    , atoms[i].excl_vol);
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
         editor->append(tr("Terminated by user request.\n"));
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

      // double m_pi_vi23; // - pi * pow(v,2/3)
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
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];
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
#if defined(SAXS_DEBUG)
      cout << "f' computed, now compute I\n";
#endif
      editor->append("f' computed, starting computation of I(q)\n");
      qApp->processEvents();
      if ( stopFlag ) 
      {
         editor->append(tr("Terminated by user request.\n"));
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
      
      progress_saxs->setTotalSteps((int)(as * 2 + 2));

      vector < vector < double > > rik_array(as);

      for ( unsigned int i = 0; i < rik_array.size(); i++ )
      {
         rik_array[i].resize(atoms.size());
      }

      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setProgress(i+1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
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
            rik_array[i][k] = sqrt(rik2);

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

      unsigned int use_q_size = use_q.size();

      double qrik; // q * rik
      double sqrikd; // sin * q * rik / qrik

      for ( unsigned int i = 0; i < as1; i++ )
      {
         // QString lcp = QString("Atom %1 of %2").arg(i+1).arg(as);
         // cout << lcp << endl;
         // lbl_core_progress->setText(lcp);
         progress_saxs->setProgress(as + i + 1);
         qApp->processEvents();
         if ( stopFlag ) 
         {
            editor->append(tr("Terminated by user request.\n"));
            progress_saxs->reset();
            lbl_core_progress->setText("");
            pb_plot_saxs_sans->setEnabled(true);
            pb_plot_pr->setEnabled(true);
            return;
         }
         for ( unsigned int k = i + 1; k < as; k++ )
         {
            rik = rik_array[i][k]; 
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

      for ( unsigned int l = 0; l < use_q_size - 1; l++ )
      {
         double merge_points = (double)(1 + use_q[l + 1] - use_q[l]);
         for ( unsigned int j = use_q[l]; j < use_q[l+1]; j++ )
         {
            double x = (double)(j - use_q[l]) / merge_points;
            I[j] = fast_I[j] + deltaI[l] * (1.0 - x) + deltaI[l+1] * x;
         }
      }

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
      double scaling_a;
      double scaling_b;
      double scaling_siga;
      double scaling_sigb;
      double scaling_chi2;

      if ( qsl_plotted_iq_names.size() )
      {
         bool ok;
         scaling_target = QInputDialog::getItem(
                                                tr("Scale I(q) Curve"),
                                                tr("Select the target plotted data set for scaling the loaded data:\n"
                                                   "or Cancel of you do not wish to scale")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                FALSE, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            scaling_target = "";
         }
      }         

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         unsigned int iq_pos = plotted_iq_names_to_pos[scaling_target];
         cout << "scaling target pos is " << iq_pos << endl;
         double target_q_min = plotted_q[iq_pos][0];
         double target_q_max = plotted_q[iq_pos][plotted_q[iq_pos].size() - 1];
         double source_q_min = q[0];
         double source_q_max = q[q.size() - 1];
         double q_min = target_q_min;
         if ( q_min < source_q_min )
         {
            q_min = source_q_min;
         }
         double q_max = target_q_max;
         if ( q_max > source_q_max )
         {
            q_max = source_q_max;
         }

         cout << QString(
                         "target q_min %1 max %2\n"
                         "source q_min %3 max %4\n"
                         "select q_min %5 max %6\n"
                         )
            .arg(target_q_min)
            .arg(target_q_max)
            .arg(source_q_min)
            .arg(source_q_max)
            .arg(q_min)
            .arg(q_max)
            ;

         vector < double > use_q;
         vector < double > use_I;
         for ( unsigned int i = 0; i < plotted_q[iq_pos].size(); i++ )
         {
            if ( plotted_q[iq_pos][i] >= q_min &&
                 plotted_q[iq_pos][i] <= q_max )
            {
               use_q.push_back(plotted_q[iq_pos][i]);
               use_I.push_back(plotted_I[iq_pos][i]);
            }
         }
         
         cout << QString("After cropping q to overlap region:\n"
                         "use_q.size == %1\n").arg(use_q.size());

         if ( !use_q.size() )
         {
            QMessageBox::warning( this, "UltraScan",
                                  QString(tr("Could not find sufficient q range overlap\n"
                                             "to scale the loaded data to the selected target")) );
         } else {
            vector < double > use_source_I = interpolate(use_q, q, I);

            US_Saxs_Util usu;

            usu.linear_fit(
                           use_source_I, 
                           use_I, 
                           scaling_a,
                           scaling_b,
                           scaling_siga,
                           scaling_sigb,
                           scaling_chi2
                           );

            QString results = 
               QString("Scaling factor: %1  Offset: %2  Chi^2: %3\n")
               .arg(scaling_b)
               .arg(scaling_a)
               .arg(scaling_chi2);
            editor->append(results);
            // cout << "saxs curve unscaled " << vector_double_to_csv(I) << endl;
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[i] = scaling_a + scaling_b * I[i];
               if ( I[i] <= 0e0 )
               {
                  I.resize(i);
                  q.resize(i);
                  break;
               }
            }
         }
      }


#ifndef QT4
      long Iq = plot_saxs->insertCurve("I(q) vs q");
#else
      QwtPlotCurve *curve = new QwtPlotCurve( "I(q) vs q" );
#endif

      QString name = 
         QString("%1_%2")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg(current_model + 1);
      QString plot_name = name;
      int extension = 0;

      while ( dup_plotted_iq_name_check.count(plot_name) )
      {
         plot_name = QString("%1-%2").arg(name).arg(++extension);
      }
      qsl_plotted_iq_names << plot_name;
      dup_plotted_iq_name_check[plot_name] = true;

#ifndef QT4
      plotted_iq_names_to_pos[plot_name] = plotted_Iq.size();
#else
      plotted_iq_names_to_pos[plot_name] = plotted_Iq_curves.size();
#endif

#ifndef QT4
      plotted_Iq.push_back(Iq);
      plot_saxs->setCurveStyle(Iq, QwtCurve::Lines);
#else
      plotted_Iq_curves.push_back( curve );
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
      unsigned int p = plotted_q.size() - 1;
#if defined(SAXS_DEBUG)
      cout << "plot # " << p << endl;
#endif
      //      for ( unsigned int i = 0; i < plotted_I[p].size(); i++ ) 
      //      {
      //         plotted_I[p][i] = log10(plotted_I[p][i]);
      //      }
#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              (double *)&(plotted_I[p][0]), q_points);
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     (double *)&(plotted_I[p][0]),
                     q_points
                     );
      curve->setPen( QPen( plot_colors[ p % plot_colors.size() ], 2, Qt::SolidLine ) );
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name);
            ok_to_write = true;
            
#if defined(OLD_WAY)
            switch( QMessageBox::information( this, 
                                              tr("Overwrite file:") + "SAXS I(q) vs. q" + tr("output file"),
                                              tr("The file named \"") + 
                                              saxs_filestring() +
                                              + tr("\" will be overwriten"),
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
            FILE *fsaxs = fopen(fsaxs_name, "w");
            if ( fsaxs ) 
            {
#if defined(SAXS_DEBUG)
               cout << "writing " << fsaxs_name << endl;
#endif
               editor->append(tr("SAXS curve file: ") + fsaxs_name + tr(" created.\n"));
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
               ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
                  QString("")
                  .sprintf(
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                           , model_filename.ascii()
                           , US_Version.ascii()
                           , REVISION
                           , our_saxs_options->start_q
                           , our_saxs_options->end_q
                           , our_saxs_options->delta_q
                           );
               fprintf(fsaxs, "%s",
                       ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header.ascii() );
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
               editor->append(tr("WARNING: Could not create SAXS curve file: ") + fsaxs_name + "\n");
               QMessageBox mb(tr("UltraScan Warning"),
                              tr("The output file ") + fsaxs_name + tr(" could not be created."), 
                              QMessageBox::Critical,
                              QMessageBox::NoButton, QMessageBox::NoButton, QMessageBox::NoButton, 0, 0, 1);
               mb.exec();
            }
         }
      } else {
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_r.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_prr_norm.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.clear();
         ((US_Hydrodyn *)us_hydrodyn)->last_saxs_header =
            QString("")
            .sprintf(
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%.3f:%.3f) step %.3f\n"
                     , model_filename.ascii()
                     , US_Version.ascii()
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
