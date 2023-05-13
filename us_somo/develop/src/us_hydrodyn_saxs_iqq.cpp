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
//Added by qt3to4:
#include <QTextStream>
#  undef SLASH
#  define SLASH "\\"
// #  define isnan _isnan
#endif

#define SAXS_MIN_Q 0e0
// #define SAXS_MIN_Q 1e-6
// #define SAXS_DEBUG
// #define SAXS_DEBUG_F
// #define SAXS_DEBUG_FF
// #define SAXS_DEBUG2

static bool save_calc_to_csv = true;

bool US_Hydrodyn_Saxs::compute_scale_excl_vol()
{
   if ( !our_saxs_options->use_iq_target_ev )
   {
      return true;
   }

   if ( our_saxs_options->set_iq_target_ev_from_vbar )
   {
      for ( unsigned int i = 0; i < selected_models.size(); i++ )
      {
         current_model = selected_models[i];
         our_saxs_options->iq_target_ev = 0e0;
         
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
                  if ( our_saxs_options->swh_excl_vol > 0e0 )
                  {
                     our_saxs_options->iq_target_ev += (double) our_saxs_options->swh_excl_vol * swh_count;
                     editor_msg( "blue", 
                                 QString( us_tr( "Adding water to excluded volume using preset excl vol %1 number of waters %2 -> excluded volume %3\n" ) )
                                 .arg( our_saxs_options->swh_excl_vol )
                                 .arg( swh_count )
                                 .arg( (double) our_saxs_options->swh_excl_vol * swh_count ) );
                  } else {
                     QString mapkey = "WAT|OW";
                     QString hybrid_name = residue_atom_hybrid_map[mapkey];
                     if ( hybrid_name.isEmpty() || !hybrid_name.length() || !hybrid_map.count( hybrid_name ) 
                          || !atom_map.count( "OW~" + hybrid_name) )
                     {
                        cout << "error: hybrid or map or atom name missing for WAT|OW" << endl; 
                        editor_msg( "red",  QString("Molecule %1 Residue WAT OW Hybrid/map/atom name missing. Atom skipped.\n")
                                    .arg( j + 1 ) );
                        continue;
                     }
                     double swh_excl_vol = atom_map["OW~" + hybrid_name].saxs_excl_vol;
                     our_saxs_options->iq_target_ev += swh_excl_vol * swh_count;
                     editor_msg( "blue", 
                                 QString( us_tr( "Adding water to excluded volume using defined excl vol %1 number of waters %2 -> excluded volume %3\n" ) )
                                 .arg( swh_excl_vol )
                                 .arg( swh_count )
                                 .arg( swh_excl_vol * swh_count ) );
                  }
               } else {
                  if ( swh_count )
                  {
                     editor_msg( "red", us_tr( "Error: a chain with WAT/OW and other residues is currently unsupported\n" ) );
                     return false;
                  }
                  
                  our_saxs_options->iq_target_ev += US_Hydrodyn::mw_to_volume( model_vector[current_model].molecule[j].mw, model_vector[i].vbar );
                  editor_msg( "blue", 
                              QString( us_tr( "Adding molecule to excluded volume from mw/vbar computation: mw %1 vbar %2 -> excluded volume %3\n" ) )
                              .arg( model_vector[current_model].molecule[j].mw )
                              .arg( model_vector[current_model].vbar )
                              .arg( US_Hydrodyn::mw_to_volume( model_vector[current_model].molecule[j].mw, model_vector[current_model].vbar ) ) );
               }
            }
         }
      }
   }

   editor_msg( "blue", 
               QString( us_tr( "Total computed excluded volume: %1\n" ) ).arg( our_saxs_options->iq_target_ev ) );

   double tot_excl_vol      = 0e0;
   double tot_excl_vol_noh  = 0e0;
   
   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      current_model = selected_models[i];

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
               editor_msg( "red", QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq));
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return false;
               }
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               editor_msg( "red", QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
                              .arg(this_atom->resSeq)
                              .arg(hybrid_name)
                              );
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return false;
               }
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq 1) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               editor_msg( "red", 
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom_name)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
               qApp->processEvents();
               if ( stopFlag ) 
               {
                  editor->append(us_tr("Terminated by user request.\n"));
                  progress_saxs->reset();
                  lbl_core_progress->setText("");
                  pb_plot_saxs_sans->setEnabled(true);
                  pb_plot_pr->setEnabled(true);
                  return false;
               }
               continue;
            }

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            
            // this is probably correct but FoXS uses the saxs table excluded volume
            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom_name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options->use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
               }
            }

            if ( this_atom->name == "OW" && our_saxs_options->swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options->swh_excl_vol;
            }
            if ( our_saxs_options->hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = (4.0/3.0) * M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
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


   // ok, we need to target our_saxs_options->iq_target_ev
   // noh_:  only includes unscalable 
   // target = scaler * tot_excl_vol_noh  + ( tot_excl_vol - tot_excl_vol_noh )

   if ( tot_excl_vol_noh > 0e0 )
   {
      our_saxs_options->scale_excl_vol =
         ( our_saxs_options->iq_target_ev - tot_excl_vol + tot_excl_vol_noh ) / tot_excl_vol_noh;
   } else {
      our_saxs_options->scale_excl_vol = 1e0;
   }

   editor_msg( "blue", QString( us_tr( "Scaling excluded volume to match target (%1), default scattering center sum without WAT %2, total %3, new scaler = %4" ) )
               .arg( our_saxs_options->iq_target_ev )
               .arg( tot_excl_vol_noh )
               .arg( tot_excl_vol )
               .arg( our_saxs_options->scale_excl_vol ) );
   return true;
}

void US_Hydrodyn_Saxs::calc_saxs_iq_native_fast()
{
   int WAT_Tf_used = 0;
   bool use_WAT_Tf =
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "use_WAT_Tf_pdb" ) &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams[ "use_WAT_Tf_pdb" ] == "true";

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

   if ( !compute_scale_excl_vol() )
   {
      return;
   }

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      bool is_only_one_swh     = false;

      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg( model_vector[ current_model ].model_id ) );
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
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
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
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq));
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
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq 2) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom_name)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
            if ( use_WAT_Tf &&
                 this_atom->resName == "WAT" &&
                 this_atom->tempFactor ) {
               new_atom.excl_vol =
                  this_atom->tempFactor *
                  this_atom->tempFactor *
                  this_atom->tempFactor *
                  M_PI * 4e0 / 3e0;
               WAT_Tf_used++;
            }

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options->use_somo_ff )
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
            if ( this_atom->name == "OW" && our_saxs_options->swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options->swh_excl_vol;
            }
            if ( our_saxs_options->hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = (4.0/3.0) * M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }
            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
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
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
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
               continue;
            } else {
               // for FoXS method
               // new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << use_resname
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

      if ( atoms.size() == 1 )
      {
         editor_msg( "dark blue", us_tr( "Notice: using full Debye since only 1 atom available" ) );
         return calc_saxs_iq_native_debye();
      }
      
      is_only_one_swh = ( atoms.size() == 1 && 
                          ( atoms[ 0 ].atom_name == "OW" ||
                            atoms[ 0 ].atom_name == "O" ) );

      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH "somo" + SLASH "saxs" + "SLASH" + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg( model_vector[ current_model ].model_id ) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
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
         
      if ( our_saxs_options->iq_exact_q )
      {
         editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }
         
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
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      q.resize(q_points);

      if ( our_saxs_options->iq_exact_q )
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
         if ( !our_saxs_options->iq_exact_q )
         {
            q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
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
                  if ( our_saxs_options->subtract_radius )
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
               if ( our_saxs_options->autocorrelate )
               {
                  hist[0] += fp[0][i] * fp[0][i];
               }
            }
            if ( our_saxs_options->autocorrelate )
            {
               hist[0] += fp[0][as1] * fp[0][as1];
            }
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
                  if ( our_saxs_options->subtract_radius )
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
               if ( our_saxs_options->autocorrelate )
               {
                  hist[0] += fp[0][i] * fp[0][i];
               }
            }
            if ( our_saxs_options->autocorrelate )
            {
               hist[0] += fp[0][as1] * fp[0][as1];
            }
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
               if ( our_saxs_options->subtract_radius )
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
            if ( our_saxs_options->autocorrelate )
            {
               hist[0] += fp[0][i] * fp[0][i];
            }
         }
         if ( our_saxs_options->autocorrelate )
         {
            hist[0] += fp[0][as1] * fp[0][as1];
         }
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
      editor->append(QString(us_tr("Average electron density %1\n")).arg(total_e / tot_excl_vol, 4));
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         editor->append(QString(us_tr("Total unhydrated excluded volume %1\n")).arg(tot_excl_vol_noh));
         editor->append(QString(us_tr("Average unhydrated electron density %1\n")).arg(total_e_noh / tot_excl_vol_noh));
         editor->append(QString(us_tr("Electron density of hydration %1\n")).arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh)));
      }

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg( model_vector[ current_model ].model_id )
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
            r[i] = (double)i * delta_pr;
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
      if ( our_saxs_options->multiply_iq_by_atomic_volume )
      {
         if ( model_vector[ current_model ].volume == 0.0 )
         {
            editor_msg("red", us_tr( "Volume is zero so NOT multiplying I(q) by atomic volume\n" ) );
         } else {
            editor_msg("blue", QString( us_tr( "Multiplying I(q) by atomic volume of %1\n" ) )
                       .arg( model_vector[ current_model ].volume ) );
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[ i ] *= model_vector[ current_model ].volume;
            }
            if ( is_only_one_swh && ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier > 0 )
            {
               editor_msg("blue", QString( us_tr( "Multiplying I(q) by water multiplier of %1\n" ) )
                          .arg( ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier ) );
               for ( unsigned int i = 0; i < I.size(); i++ )
               {
                  I[ i ] *= ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier;
               }
            }
         }
      }        

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
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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

   if ( WAT_Tf_used ) {
      editor_msg( "dark blue", QString( us_tr( "Notice: %1 WATs using PDB's Tf radius recognized\n" ) ).arg( WAT_Tf_used ) );
   }


   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

//  ------------------------------------------------------------------------------------------------------
void US_Hydrodyn_Saxs::calc_saxs_iq_native_debye()
{
   int WAT_Tf_used = 0;
   bool use_WAT_Tf =
      ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "use_WAT_Tf_pdb" ) &&
      ((US_Hydrodyn *)us_hydrodyn)->gparams[ "use_WAT_Tf_pdb" ] == "true";

   cout << "csind\n";
   // don't forget to later merge deleted waters into model_vector
   // right now we are going with first residue map entry
   stopFlag = false;
   pb_stop->setEnabled(true);
   pb_plot_saxs_sans->setEnabled(false);
   pb_plot_pr->setEnabled(false);
   progress_saxs->reset();
   QRegExp count_hydrogens("H(\\d)");

   if ( our_saxs_options->iqq_use_atomic_ff )
   {
      editor_msg( "dark red", "using explicit hydrogens" );
   }
   if ( our_saxs_options->iqq_use_saxs_excl_vol  )
   {
      editor_msg( "dark red", "using excluded volume from saxs atoms" );
   }

   if ( !compute_scale_excl_vol() )
   {
      return;
   }

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      bool is_only_one_swh     = false;
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg( model_vector[ current_model ].model_id ));
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
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
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
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq));
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
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq 3) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               editor_msg( "red", 
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom_name)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
               continue;
            }

#if defined( DEBUG_SAXS )
            cout << QString("atom %1 hybrid %2 excl vol %3 by hybrid radius %4\n")
               .arg(this_atom_name)
               .arg(this_atom->hybrid_name)
               .arg(atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol)
               .arg(M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius)
               ;
#endif
            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;
            if ( use_WAT_Tf &&
                 this_atom->resName == "WAT" &&
                 this_atom->tempFactor ) {
               new_atom.excl_vol =
                  this_atom->tempFactor *
                  this_atom->tempFactor *
                  this_atom->tempFactor *
                  M_PI * 4e0 / 3e0;
               WAT_Tf_used++;
            }

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options->use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
                  cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
                     .arg( new_atom.atom_name )
                     .arg( this_ev );
               }
            }

            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options->swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options->swh_excl_vol;
            }
//             if ( this_atom->name == "XH" )
//             {
//                // skip excl vol for now
//                // new_atom.excl_vol = 0e0;
//             }

            if ( our_saxs_options->hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = (4.0/3.0) * M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }
               
            if ( our_saxs_options->iqq_use_saxs_excl_vol )
            {
               new_atom.excl_vol = saxs_map[hybrid_map[hybrid_name].saxs_name].volume;
            }

            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
               tot_excl_vol_noh  += new_atom.excl_vol;
               total_e_noh       += hybrid_map[ hybrid_name ].num_elect;
            }

            new_atom.radius = hybrid_map[hybrid_name].radius;
            tot_excl_vol += new_atom.excl_vol;

            new_atom.saxs_name = hybrid_map[hybrid_name].saxs_name; 
            new_atom.hybrid_name = hybrid_name;
            new_atom.hydrogens = 0;
            if ( !our_saxs_options->iqq_use_atomic_ff &&
                 count_hydrogens.indexIn(hybrid_name) != -1 )
            {
               new_atom.hydrogens = count_hydrogens.cap(1).toInt();
            }
            // cout << QString("in %1 hydrogens %2\n").arg( hybrid_name ).arg( new_atom.hydrogens );

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
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
               continue;
            }

#if defined(SAXS_DEBUG2)
            QTextStream( stderr ) << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << use_resname
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
 
      is_only_one_swh = ( atoms.size() == 1 && 
                          ( atoms[ 0 ].atom_name == "OW" ||
                            atoms[ 0 ].atom_name == "O" ) );
      
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg( model_vector[ current_model ].model_id ) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
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

      if ( our_saxs_options->iq_exact_q )
      {
         editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }
         
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

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);

      if ( our_saxs_options->iq_exact_q )
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
         if ( !our_saxs_options->iq_exact_q )
         {
            q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            if ( q[j] < SAXS_MIN_Q ) 
            {
               q[j] = SAXS_MIN_Q;
            }
         }
         q2[j] = q[j] * q[j];
         q_over_4pi_2[j] = q[j] * q[j] * one_over_4pi_2;
      }

      if ( ( ( US_Hydrodyn * )us_hydrodyn)->gparams.count( "create_shd" ) &&
           ( ( US_Hydrodyn * )us_hydrodyn)->gparams[ "create_shd" ] == "1" )
      {
         create_shd( atoms, q, q2, q_over_4pi_2 );
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
      cout << QString( "atoms.size() %1\n" ).arg( atoms.size() );
      if ( !atoms.size() )
      {
         editor_msg( "red", "No useable atoms found" );
         return;
      }

      // spec calc for now
      {
#if defined( DEBUG_SAXS )
         cout << "atomname pos h n ve\n";
#endif
         vector < double > n;
         vector < double > v;

         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            saxs sm = saxs_map[atoms[i].saxs_name];
            double this_n = 
               compute_ff(
                          sm,
                          saxsH,
                          atoms[ i ].residue_name,
                          atoms[ i ].saxs_name,
                          atoms[ i ].atom_name,
                          atoms[ i ].hydrogens,
                          0e0,
                          0e0 );
#if defined( DEBUG_SAXS )
            cout << QString( "%1 %2 %3 %4 %5\n" )
               .arg( atoms[ i ].saxs_name )
               .arg( i )
               .arg( atoms[ i ].hydrogens )
               .arg( this_n )
               .arg( atoms[ i ].excl_vol )
               ;
#endif
            n.push_back( this_n );
            v.push_back( atoms[ i ].excl_vol );
         }
         // compute I0 debye & its components
         double I0rho0 = 0e0;
         double I0     = 0e0;
         double rho0   = our_saxs_options->water_e_density;
         double evx    = 0e0;

         for ( unsigned int i = 0; i < n.size(); i++ )
         {
            for ( unsigned int j = 0; j < n.size(); j++ )
            {
               I0rho0 += n[ i ] * n[ j ];
               I0     += n[ i ] * n[ j ] - rho0 * ( n[ i ] * v[ j ] + n[ j ] * v[ i ] - rho0 * ( v[ i ] * v[ j ] ) );
               evx    += rho0 * ( v[ i ] * v[ j ] );
            }
         }

         cout << QString( "I0 (rho0 == 0) %1 I0 %2 diff %3 sum vivj %4\n" )
            .arg( I0rho0 )
            .arg( I0 )
            .arg( I0rho0 - I0 )
            .arg( evx )
            ;
         
         // compute exact:
         
         double sumn    = 0e0;
         double V       = 0e0;

         for ( unsigned int i = 0; i < n.size(); i++ )
         {
            V    += v[ i ];
            sumn += n[ i ];
         }

         double eI0rho0 = sumn * sumn;
         double eI0     = sumn * sumn - rho0 * ( 2e0 * V * sumn - rho0 * V * V );
         double eevx    = rho0 * V * V;

         cout << QString( "exact I0 (rho0 == 0) %1 I0 %2 diff %3 rho0 V^2 %4 ev total %5\n" )
            .arg( eI0rho0 )
            .arg( eI0 )
            .arg( eI0rho0 - eI0 )
            .arg( eevx )
            .arg( V )
            ;
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {

         saxs saxs = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
         // m_pi_vi23 = -M_PI * pow((double)vi,2.0/3.0); // - pi * pow(v,2/3)
         vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;

#if defined(SAXS_DEBUG_FX)
         cout << QString( "%1 atom %2 hydrogens %3 vie %4 vi_23_4pi %5\n" )
            .arg( i )
            .arg( atoms[ i ].saxs_name )
            .arg( atoms[ i ].hydrogens )
            .arg( vie )
            .arg( vi_23_4pi )
            ;
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

            f[ j ][ i ] = 
               compute_ff( saxs,
                           saxsH,
                           atoms[ i ].residue_name,
                           atoms[ i ].saxs_name,
                           atoms[ i ].atom_name,
                           atoms[ i ].hydrogens,
                           q[ j ],
                           q_over_4pi_2[ j ] );
                                      

            fc[j][i] = vie * exp( vi_23_4pi * q2[ j ] * our_saxs_options->ev_exp_mult );
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
                    << vi_23_4pi
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

         // cout << QString( "atoms.size() %1\n" ).arg( atoms.size() );
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

         QFile f( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp" + SLASH + "last_iqq.csv" );
         if ( f.open( QIODevice::WriteOnly ) )
         {
            QTextStream t( &f );
            t << out;
            f.close();
            editor->append(QString("created %1\n").arg(f.fileName()));
         }
      }            

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
            if ( our_saxs_options->subtract_radius )
            {
               puts( "no!!!" );
               rik = rik - atoms[i].radius - atoms[k].radius;
               if ( rik < 0e0 )
               {
                  rik = 0e0;
               }
            }


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
               sqrikd = ( fabs(qrik) < 1e-30 ) ? 1.0 : sin(qrik) / qrik;
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
         if ( our_saxs_options->autocorrelate )
         {
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[j] += fp[j][i] * fp[j][i];
               Ia[j] += f[j][i] * f[j][i];
               Ic[j] += fc[j][i] * fc[j][i];
            }
         }
      }
      if ( our_saxs_options->autocorrelate )
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

      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      if ( our_saxs_options->multiply_iq_by_atomic_volume )
      {
         if ( model_vector[ current_model ].volume == 0.0 )
         {
            editor_msg("red", us_tr( "Volume is zero so NOT multiplying I(q) by atomic volume\n" ) );
         } else {
            editor_msg("blue", QString( us_tr( "Multiplying I(q) by atomic volume of %1\n" ) )
                       .arg( model_vector[ current_model ].volume ) );
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[ i ] *= model_vector[ current_model ].volume;
            }
            if ( is_only_one_swh && ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier > 0 )
            {
               editor_msg("blue", QString( us_tr( "Multiplying I(q) by water multiplier of %1\n" ) )
                          .arg( ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier ) );
               for ( unsigned int i = 0; i < I.size(); i++ )
               {
                  I[ i ] *= ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier;
               }
            }
         }
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

      editor->append(QString(us_tr("Total excluded volume %1\n")).arg(tot_excl_vol));
      editor->append(QString(us_tr("Average electron density %1\n")).arg(total_e / tot_excl_vol, 4));
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         editor->append(QString(us_tr("Total unhydrated excluded volume %1\n")).arg(tot_excl_vol_noh));
         editor->append(QString(us_tr("Average unhydrated electron density %1\n")).arg(total_e_noh / tot_excl_vol_noh));
         editor->append(QString(us_tr("Electron density of hydration %1\n")).arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh)));
      }

      QString name = 
         QString("%1%2_%3%4")
         .arg( QFileInfo(te_filename2->text()).fileName() )
         .arg( specname )
         .arg( model_vector[ current_model ].model_id )
         .arg( iqq_suffix() );

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
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
   if ( WAT_Tf_used ) {
      editor_msg( "dark blue", QString( us_tr( "Notice: %1 WATs using PDB's Tf radius recognized\n" ) ).arg( WAT_Tf_used ) );
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

   if ( !compute_scale_excl_vol() )
   {
      return;
   }

   for ( unsigned int i = 0; i < selected_models.size(); i++ )
   {
      bool is_only_one_swh     = false;
      double tot_excl_vol      = 0e0;
      double tot_excl_vol_noh  = 0e0;
      unsigned int total_e     = 0;
      unsigned int total_e_noh = 0;
      current_model = selected_models[i];
#if defined(SAXS_DEBUG)
      printf("creating sax_atoms %u\n", current_model);
#endif
      editor->append(QString("\n\nPreparing file %1 model %2 for SAXS plot.\n\n")
                     .arg(te_filename2->text())
                     .arg( model_vector[ current_model ].model_id ));
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
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
         {
            PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);
            new_atom.pos[0] = this_atom->coordinate.axis[0];
            new_atom.pos[1] = this_atom->coordinate.axis[1];
            new_atom.pos[2] = this_atom->coordinate.axis[2];

            if ( this_atom->name == "XH" && !our_saxs_options->iqq_use_atomic_ff )
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
               editor_msg( "red", 
                           QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq));
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
               continue;
            }

            if ( !hybrid_map.count(hybrid_name) )
            {
               cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
               continue;
            }

            QString this_atom_name = hybrid_name == "ABB" ? "ABB" : this_atom->name;
            if ( !atom_map.count(this_atom_name + "~" + hybrid_name) )
            {
               cout << "error: (saxs_iqq 4) atom_map missing for hybrid_name "
                    << hybrid_name 
                    << " atom name "
                    << this_atom_name
                    << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->name)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           );
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
               continue;
            }

            new_atom.excl_vol = atom_map[this_atom_name + "~" + hybrid_name].saxs_excl_vol;

            new_atom.atom_name = this_atom->name;
            new_atom.residue_name = use_resname;

            if ( our_saxs_options->use_somo_ff )
            {
               double this_ev = get_ff_ev( new_atom.residue_name, new_atom.atom_name );
               if ( this_ev )
               {
                  new_atom.excl_vol = this_ev;
                  cout << QString( "found ev from ff %1 %2 %3\n" ).arg( new_atom.residue_name )
                     .arg( new_atom.atom_name )
                     .arg( this_ev );
               }
            }

            total_e += hybrid_map[ hybrid_name ].num_elect;
            if ( this_atom->name == "OW" && our_saxs_options->swh_excl_vol > 0e0 )
            {
               new_atom.excl_vol = our_saxs_options->swh_excl_vol;
            }
            if ( our_saxs_options->hybrid_radius_excl_vol )
            {
               new_atom.excl_vol = (4.0/3.0) * M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
            }
            if ( this_atom->name != "OW" )
            {
               new_atom.excl_vol *= our_saxs_options->scale_excl_vol;
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
            // cout << QString("in %1 hydrogens %2\n").arg( hybrid_name ).arg( new_atom.hydrogens );

            if ( !saxs_map.count(hybrid_map[hybrid_name].saxs_name) )
            {
               cout << "error: saxs_map missing for hybrid_name "
                    << hybrid_name 
                    << " saxs name "
                    << hybrid_map[hybrid_name].saxs_name
                    << endl;
               editor_msg( "red",
                           QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(use_resname)
                           .arg(this_atom->resSeq)
                           .arg(hybrid_name)
                           .arg(hybrid_map[hybrid_name].saxs_name)
                           );
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
               continue;
            }

#if defined(SAXS_DEBUG2)
            cout << "Atom: "
                 << this_atom->name
                 << " Residue: "
                 << use_resname
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

      if ( atoms.size() == 1 )
      {
         editor_msg( "dark blue", us_tr( "Notice: using full Debye since only 1 atom available" ) );
         return calc_saxs_iq_native_debye();
      }

      is_only_one_swh = ( atoms.size() == 1 && 
                          ( atoms[ 0 ].atom_name == "OW" ||
                            atoms[ 0 ].atom_name == "O" ) );

      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg(model_vector[ current_model ].model_id ) + 
         ".atoms";

      FILE *fsaxs_atoms = us_fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %.3f %.3f %.3f %.2f\n"
                    , atoms[i].saxs_name.toLatin1().data()
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

      if ( our_saxs_options->iq_exact_q )
      {
         editor_msg( "blue", QString( us_tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            editor_msg( "dark red", QString( us_tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
            exact_q.resize( q_points );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               exact_q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            }
         } else {
            q_points = ( unsigned int ) exact_q.size();
         }
      }

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

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);

      if ( our_saxs_options->iq_exact_q )
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
         if ( !our_saxs_options->iq_exact_q )
         {
            q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            if ( q[j] < SAXS_MIN_Q ) 
            {
               q[j] = SAXS_MIN_Q;
            }
         }
         q2[j] = q[j] * q[j];
         q_over_4pi_2[j] = q[j] * q[j] * one_over_4pi_2;
      }

      if ( ( ( US_Hydrodyn * )us_hydrodyn)->gparams.count( "create_shd" ) &&
           ( ( US_Hydrodyn * )us_hydrodyn)->gparams[ "create_shd" ] == "1" )
      {
         create_shd( atoms, q, q2, q_over_4pi_2 );
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

            fc[j][i] = vie * exp( vi_23_4pi * q2[ j ] * our_saxs_options->ev_exp_mult );
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
      //    rik_array[i].resize(atoms.size());
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
            rik = sqrt( rik2 );
            // rik_array[i][k] = sqrt(rik2);
            if ( our_saxs_options->subtract_radius )
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
         if ( our_saxs_options->autocorrelate )
         {
            hist[0] += fp[0][i] * fp[0][i];
         }
      }
      if ( our_saxs_options->autocorrelate )
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
                       (atoms[i].pos[2] - atoms[k].pos[2]) );

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
         if ( our_saxs_options->autocorrelate )
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
         if ( our_saxs_options->autocorrelate )
         {
            I[j] += fp[j][as1] * fp[j][as1];
         }
         deltaI[l] = I[j] - fast_I[j];
      }

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

      lbl_core_progress->setText("");
      qApp->processEvents();
      progress_saxs->reset();
#if defined(SAXS_DEBUG)
      cout << "I computed\n";
#endif
      if ( our_saxs_options->multiply_iq_by_atomic_volume )
      {
         if ( model_vector[ current_model ].volume == 0.0 )
         {
            editor_msg("red", us_tr( "Volume is zero so NOT multiplying I(q) by atomic volume\n" ) );
         } else {
            editor_msg("blue", QString( us_tr( "Multiplying I(q) by atomic volume of %1\n" ) )
                       .arg( model_vector[ current_model ].volume ) );
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[ i ] *= model_vector[ current_model ].volume;
            }
            if ( is_only_one_swh && ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier > 0 )
            {
               editor_msg("blue", QString( us_tr( "Multiplying I(q) by water multiplier of %1\n" ) )
                          .arg( ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier ) );
               for ( unsigned int i = 0; i < I.size(); i++ )
               {
                  I[ i ] *= ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier;
               }
            }
         }
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


      editor->append(QString(us_tr("Total excluded volume %1\n")).arg(tot_excl_vol));
      editor->append(QString(us_tr("Average electron density %1\n")).arg(total_e / tot_excl_vol, 4));
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         editor->append(QString(us_tr("Total unhydrated excluded volume %1\n")).arg(tot_excl_vol_noh));
         editor->append(QString(us_tr("Average unhydrated electron density %1\n")).arg(total_e_noh / tot_excl_vol_noh));
         editor->append(QString(us_tr("Electron density of hydration %1\n")).arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh)));
      }

      QString name = 
         QString("%1_%2%3")
         .arg(QFileInfo(te_filename2->text()).fileName())
         .arg( model_vector[ current_model ].model_id )
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
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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

void US_Hydrodyn_Saxs::set_scaling_target( QString &scaling_target )
{
   if ( !our_saxs_options->iqq_default_scaling_target.isEmpty() &&
        qsl_plotted_iq_names.filter(QRegExp(QString("^%1$").arg(our_saxs_options->iqq_default_scaling_target))).size() )
   {
      scaling_target = our_saxs_options->iqq_default_scaling_target;
   } else {
      if ( !our_saxs_options->disable_iq_scaling &&
           qsl_plotted_iq_names.size() )
      {
         bool ok;
         scaling_target = US_Static::getItem(
                                                us_tr("Scale I(q) Curve"),
                                                us_tr("Select the target plotted data set for scaling the loaded data:\n"
                                                   "or Cancel of you do not wish to scale")
                                                , 
                                                qsl_plotted_iq_names, 
                                                0, 
                                                false, 
                                                &ok,
                                                this );
         if ( ok ) {
            // user selected an item and pressed OK
         } else {
            scaling_target = "";
         }
      }
   }
}

double US_Hydrodyn_Saxs::compute_ff(
                                    saxs     &sa,        // gaussian decomposition for the main atom
                                    saxs     &sh,        // gaussian decomposition for hydrogen
                                    QString  &nr,        // name of residue
                                    QString  &na,        // name of atom
                                    QString  &naf,       // full name of atom
                                    unsigned int h,      // number of hydrogens
                                    double   q,
                                    double   q_o_4pi2 
                                    )
{

   //   cout << QString( "compute_ff q %1 q_o_4pi2 %2\n" ).arg( q ).arg( q_o_4pi2 );
   // cout << "compute_ff: saxs_name:" << sa.saxs_name << endl;

   // #define UHSI_COMPUTE_FF_DEBUG

#if defined( UHSI_COMPUTE_FF_DEBUG )
   cout << QString( "compute_ff: q: %1 nr:%2 na:%3 naf:%4 h:%5 use_somo_ff %6 alt_ff %7\n" )
      .arg( q )
      .arg( nr )
      .arg( na )
      .arg( naf )
      .arg( h )
      .arg( our_saxs_options->use_somo_ff ? "yes" : "no" )
      .arg( our_saxs_options->alt_ff ? "yes" : "no" );
#endif

   if ( our_saxs_options->use_somo_ff )
   {
      QString ffkey = nr + "|" + naf;
      if ( ff_table.count( ffkey ) )
      {
         // ok
         QString errormsg;
         double  ff;
         if ( !US_Saxs_Util::static_apply_natural_spline( ff_q [ ff_table[ ffkey ] ],
                                                          ff_ff[ ff_table[ ffkey ] ],
                                                          ff_y2[ ff_table[ ffkey ] ],
                                                          q,
                                                          ff,
                                                          errormsg ) )
         {
            editor_msg( "red", QString( us_tr( "Warning: error applying natural spline to %1 at %2 <%3>" ) )
                        .arg( ffkey ) 
                        .arg( q ) 
                        .arg( errormsg ) );
            qApp->processEvents();
         } else {
            return ff;
         }
      } else {
         if ( !ff_sent_msg1.count( ffkey ) )
         {
            editor_msg( "dark red", QString( us_tr( "Warning: key %1 not found in ff_table" ) ).arg( ffkey ) );
            ff_sent_msg1[ ffkey ] = true;
            qApp->processEvents();
         }
      }
   }

   if ( !h )
   {
      if ( our_saxs_options->five_term_gaussians )
      {
         return 
            sa.c5 +
            sa.a5[ 0 ] * exp( -sa.b5[ 0 ] * q_o_4pi2 ) +
            sa.a5[ 1 ] * exp( -sa.b5[ 1 ] * q_o_4pi2 ) +
            sa.a5[ 2 ] * exp( -sa.b5[ 2 ] * q_o_4pi2 ) +
            sa.a5[ 3 ] * exp( -sa.b5[ 3 ] * q_o_4pi2 ) +
            sa.a5[ 4 ] * exp( -sa.b5[ 4 ] * q_o_4pi2 );
      } else {
         //          for ( unsigned int i = 0; i < 4; i++ )
         //          {
         //             cout << QString( "ff using for term %1 a=%2 b=%3\n" )
         //                .arg( i ).arg( sa.a[ i ] ).arg( sa.b[ i ] );
         //          }
         //          cout << QString( "ff using for constant term %1\n" ).arg( sa.c );
                   
         return 
            sa.c +
            sa.a[ 0 ] * exp( -sa.b[ 0 ] * q_o_4pi2 ) +
            sa.a[ 1 ] * exp( -sa.b[ 1 ] * q_o_4pi2 ) +
            sa.a[ 2 ] * exp( -sa.b[ 2 ] * q_o_4pi2 ) +
            sa.a[ 3 ] * exp( -sa.b[ 3 ] * q_o_4pi2 );
      }
   }

   if ( !our_saxs_options->alt_ff )
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
      editor_msg( "red", QString( "ERROR: No hybrid info for atom %1, computing for ZERO hydrogens" ).arg( na ) );
      return compute_ff( sa, sh, nr, na, naf, 0, q, q_o_4pi2 );
   }

   if ( hybrid_coords[ na ].size() <= h )
   {
      editor_msg( "red", QString( "ERROR: More hydrogens requested (%1) than available (%2) for atom %2, computing for available hydrogens" )
                  .arg( h ) 
                  .arg( hybrid_coords[ na ].size() - 1 )
                  .arg( na ) 
                  );
      h = hybrid_coords[ na ].size();
   }

   // now compute debye for "mini" system at q, return square root of intensity

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

   if ( our_saxs_options->autocorrelate )
   {
      for ( unsigned int i = 0; i <= h; i++ )
      {
         I += ff[ i ] * ff[ i ];
      }
   }

   return sqrt( I );
}                                    

bool US_Hydrodyn_Saxs::load_ff_table( QString filename )
{
    if ( ff_table_loaded &&
         last_ff_filename == filename )
    {
       editor_msg( "dark red", "Notice: this version is doing an unneeded reload for debugging purposes" );
       // return true;
    }

   editor_msg( "blue", QString( us_tr( "Loading ff table %1" ) ).arg( filename ) );

   QFile f( filename );
   if ( !f.exists() )
   {
      editor_msg( "red", QString( us_tr( "Error: load ff table %1 does not exist" ) ).arg( filename ) );
      return false;
   }

   if ( !f.open( QIODevice::ReadOnly ) )
   {
      editor_msg( "red", QString( us_tr( "Error: load ff table %1 can not be opened" ) ).arg( filename ) );
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
            editor_msg( "red", QString( us_tr( "Error: load_ff line %1, residue/atoms not paired" ) ).arg( line ) );
            return false;
         }
         continue;
      }

      if ( qsl[ 0 ] == "excludedvolume" )
      {
         qsl.pop_front();
         if ( qsl.count() !=  1 )
         {
            editor_msg( "red", QString( us_tr( "Error: load_ff line %1, excluded volume requires exactly one parameter" ) ).arg( line ) );
            return false;
         }

         if ( ff_ev.size() != pos )
         {
            editor_msg( "red", QString( us_tr( "Error: load_ff line %1, excluded volume multiply defined" ) ).arg( line ) );
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
            editor_msg( "red", QString( us_tr( "Error: load_ff line %1, startdata with no excluded volume defined" ) ).arg( line ) );
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
                  editor_msg( "red", QString( us_tr( "Error: load_ff line %1, empty or insufficient data" ) ).arg( line ) );
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
               editor_msg( "red", QString( us_tr( "Error: load_ff line %1, data lines must have exactly two fields" ) ).arg( line ) );
               return false;
            }

            new_q.push_back ( qsl[ 0 ].toDouble() );
            new_ff.push_back( qsl[ 1 ].toDouble() );

         }
         if ( !ok )
         {
            editor_msg( "red", QString( us_tr( "Error: load_ff line %1, permature eof: no enddata token found" ) ).arg( line ) );
            return false;
         }
         continue;
      }
      editor_msg( "red", QString( us_tr( "Error: load_ff line %1, unknown token %2" ) ).arg( line ).arg( qsl[ 0 ] ) );
      return false;
   }
   f.close();

   editor_msg( "blue", QString( us_tr( "ff table %1 loaded ok" ) ).arg( filename ) );

   ff_table_loaded  = true;
   last_ff_filename = filename;

   cout << ff_info();

   return true;
}


QString US_Hydrodyn_Saxs::ff_info()
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

double US_Hydrodyn_Saxs::get_ff_ev( QString res, QString atm )
{
   QString ffkey = res + "|" + atm;
   if ( !ff_table.count( ffkey ) )
   {
      editor_msg( "dark red", QString( us_tr( "Warning: key %1 not found in ff_table" ) ).arg( ffkey ) );
      return 0e0;
   }

   return ff_ev[ ff_table[ ffkey ] ];
}

bool US_Hydrodyn_Saxs::create_somo_ff()
{
   errormsg = "";
   QFile f ( ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp" + SLASH + "somo.ff.new" );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errormsg = QString( us_tr( "Error: can not create %1" ) ).arg( f.fileName() );
      return false;
   }
   
   bool save_somo_ff = our_saxs_options->use_somo_ff;
   our_saxs_options->use_somo_ff = false;

   QTextStream ts( &f );

   unsigned int q_points = 
      (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;
   
   vector < double > q           ( q_points );
   vector < double > q_over_4pi_2( q_points ); 

   double one_over_4pi = 1.0 / (4.0 * M_PI);
   double one_over_4pi_2 = one_over_4pi * one_over_4pi;

   for ( unsigned int j = 0; j < q_points; j++ )
   {
      q           [ j ] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
      q_over_4pi_2[ j ] = q[ j ] * q[ j ] * one_over_4pi_2;
   }

   saxs saxsH = saxs_map[ "H" ];
   
   // add hybrids also

   QString res = "";
   for ( map < QString, saxs >::iterator it = saxs_map.begin();
         it != saxs_map.end();
         it++ )
   {
      QString ta = it->first;
      unsigned int max_h = 0;
      if ( hybrid_coords.count( ta ) )
      {
         max_h = ( unsigned int ) hybrid_coords[ ta ].size() - 1;
      }

      for ( unsigned int h = 0; h <= max_h; h++ )
      {
         saxs saxsA = it->second;
         cout << QString( "create_somo_ff: %1%2\n" ).arg( it->first ).arg( h ? QString( "H%1" ).arg( h ) : "" );
         ts << QString( "# %1%2\n" ).arg( ta ).arg( h ? QString( "H%1" ).arg( h ) : "" );
         ts << "# ";
         for ( int i = 0; i < 4; i++ )
         {
            ts << QString( "a[%1]=%2, b[%3]=%4, " )
               .arg( i )
               .arg( saxsA.a[ i ] )
               .arg( i )
               .arg( saxsA.b[ i ] );
         }
         ts << QString( "c=%1\n" ).arg( saxsA.c );
         ts << "startdata\n";
         for ( unsigned int j = 0; j < q_points; j++ )
         {
            double I = compute_ff(
                                  saxsA,
                                  saxsH,
                                  res,
                                  ta,
                                  ta,
                                  h,
                                  q[ j ],
                                  q_over_4pi_2[ j ] 
                                  );
            ts << QString( "%1 %2\n" ).arg( q[ j ], 0, 'g', 17 ).arg( I, 0, 'g', 17 );
         }
         ts << "enddata\n";
      }
   }
   f.close();
   our_saxs_options->use_somo_ff = save_somo_ff;
   return true;
}

void US_Hydrodyn_Saxs::create_shd( vector < saxs_atom > & org_atoms,
                                   vector < double >    & q,
                                   vector < double >    & q2,
                                   vector < double >    & q_over_4pi_2
                                   )
{
   puts( "create_shd" );

   shd_point tmp_shd;

   vector < shd_point >         model;
   vector < vector < double > > F;

   map < QString, int16_t > types;

   // build up the model

   vector < double > Fa( q.size() );

   double one_over_4pi = 1.0 / (4.0 * M_PI);
   saxs saxsH = saxs_map["H"];

   vector < saxs_atom > atoms = org_atoms;

   point cx;
   for ( unsigned int j = 0; j < 3; j++ )
   {
      cx.axis[ j ] = 0.0;
   }

   for ( unsigned int i = 0; i < (unsigned int)atoms.size(); i++ )
   {
      for ( unsigned int j = 0; j < 3; j++ )
      {
         cx.axis[ j ] += atoms[ i ].pos[ j ];
      }
   }
   for ( unsigned int j = 0; j < 3; j++ )
   {
      cx.axis[ j ] /= ( float ) atoms.size();
   }
   for ( unsigned int i = 1; i < (unsigned int)atoms.size(); i++ )
   {
      for ( unsigned int j = 0; j < 3; j++ )
      {
         atoms[ i ].pos[ j ] -= cx.axis[ j ];
      }
   }

   for ( int i = 0; i < (int)atoms.size(); i++ )
   {
      tmp_shd.x[ 0 ] = atoms[ i ].pos[ 0 ];
      tmp_shd.x[ 1 ] = atoms[ i ].pos[ 1 ];
      tmp_shd.x[ 2 ] = atoms[ i ].pos[ 2 ];

      QString key = atoms[ i ].atom_name + QString( "%1" ).arg( atoms[ i ].hydrogens ? QString( "H%1" ).arg( atoms[ i ].hydrogens ) : QString( "" ) );
      if ( !types.count( key ) )
      {
         {
            int16_t pos = types.size();
            types[ key ] = pos;
         }

         // build up F
         saxs saxs = saxs_map[atoms[i].saxs_name];
         double vi = atoms[ i ].excl_vol;
         double vie = vi * our_saxs_options->water_e_density;
         double vi_23_4pi = - pow((double)vi,2.0/3.0) * one_over_4pi;

         for ( int j = 0; j < (int) q.size(); j++ )
         {
            Fa[ j ] = 
               compute_ff( saxs,
                           saxsH,
                           atoms[ i ].residue_name,
                           atoms[ i ].saxs_name,
                           atoms[ i ].atom_name,
                           atoms[ i ].hydrogens,
                           q[ j ],
                           q_over_4pi_2[ j ] ) -
               vie * exp( vi_23_4pi * q2[ j ] * our_saxs_options->ev_exp_mult );
         }
         F.push_back( Fa );
      }
      tmp_shd.ff_type = types[ key ];
      model.push_back( tmp_shd );
   }

   shd_input_data id;

   id.max_harmonics = (uint32_t) our_saxs_options->sh_max_harmonics;
   id.model_size    = (uint32_t) model.size();
   id.q_size        = (uint32_t) q.size();
   id.F_size        = (uint32_t) F.size();

   // now write it all out
   {
      QString fname =  ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + "tmp" + SLASH + "shd.dat";
      fname.replace( "//", "/" );
      ofstream ofs( fname.toLatin1().data(), ios::out | ios::binary );

      if ( !ofs.is_open() )
      {
         editor_msg( "red", QString( us_tr( "Error: could not open %1" ) ).arg( fname ) );
         return;
      }

      if ( !ofs.write( (char *)&id, sizeof( id ) ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write id to %1" ) ).arg( fname ) );
         return;
      }

      for ( uint32_t i = 0; i < id.F_size; ++i )
      {
         if ( !ofs.write( (char *)(&F[ i ][ 0 ]), sizeof( double ) * id.q_size ) )
         {
            ofs.close();
            editor_msg( "red", QString( us_tr( "Error: could not write F to %1" ) ).arg( fname ) );
            return;
         }
      }

      if ( !ofs.write( (char*)(&q[ 0 ]), sizeof( double ) * id.q_size ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write q to %1" ) ).arg( fname ) );
         return;
      }

      if ( !ofs.write( (char *)(&model[ 0 ]), sizeof( struct shd_point ) * id.model_size ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write model to %1" ) ).arg( fname ) );
         return;
      }

      ofs.close();

      editor_msg( "blue", QString( us_tr( "Created shd %1" ) ).arg( fname ) );
      printf( "0: F (%d):\n", (int) F.size() );
      for ( int i = 0; i < (int) F.size(); i++ )
      {
         printf( "0: %d (%d):", i, (int) F[ i ].size() );
         for ( int j = 0; j < (int) F[ i ].size(); j++ )
         {
            printf( " %f ", F[ i ][ j ] );
         }
         printf( "\n" );
      }
   }
   // and the float version
   {
      QString fname =  ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + "tmp" + SLASH + "shd_f.dat";
      fname.replace( "//", "/" );
      ofstream ofs( fname.toLatin1().data(), ios::out | ios::binary );

      vector < vector < float > > F_f( F.size() );
      vector < float >            q_f( q.size() );

      for ( int i  = 0; i < (int)F.size(); ++i )
      {
         F_f[ i ].resize( F[ i ].size() );
         for ( int j = 0; j < (int)F[ i ].size(); ++j )
         {
            F_f[ i ][ j ] = (float)F[ i ][ j ];
         }
      }

      for ( int j = 0; j < (int)q.size(); ++j )
      {
         q_f[ j ] = (float) q[ j ];
      }

      if ( !ofs.is_open() )
      {
         editor_msg( "red", QString( us_tr( "Error: could not open %1" ) ).arg( fname ) );
         return;
      }

      if ( !ofs.write( (char *)&id, sizeof( id ) ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write id to %1" ) ).arg( fname ) );
         return;
      }

      for ( uint32_t i = 0; i < id.F_size; ++i )
      {
         if ( !ofs.write( (char *)(&F_f[ i ][ 0 ]), sizeof( float ) * id.q_size ) )
         {
            ofs.close();
            editor_msg( "red", QString( us_tr( "Error: could not write F to %1" ) ).arg( fname ) );
            return;
         }
      }

      if ( !ofs.write( (char*)(&q_f[ 0 ]), sizeof( float ) * id.q_size ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write q to %1" ) ).arg( fname ) );
         return;
      }

      if ( !ofs.write( (char *)(&model[ 0 ]), sizeof( struct shd_point ) * id.model_size ) )
      {
         ofs.close();
         editor_msg( "red", QString( us_tr( "Error: could not write model to %1" ) ).arg( fname ) );
         return;
      }

      //       printf( "0: F_f (%d):\n", (int) F_f.size() );
      //       for ( int i = 0; i < (int) F_f.size(); i++ )
      //       {
      //          printf( "0: %d (%d):", i, (int) F_f[ i ].size() );
      //          for ( int j = 0; j < (int) F_f[ i ].size(); j++ )
      //          {
      //             printf( " %f ", F_f[ i ][ j ] );
      //          }
      //          printf( "\n" );
      //       }

      ofs.close();

      editor_msg( "blue", QString( us_tr( "Created shd %1" ) ).arg( fname ) );
   }

   return;
}
