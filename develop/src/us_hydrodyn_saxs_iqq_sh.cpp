#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include "../include/us_saxs_util.h"
#include "../include/us_sh.h"
#include "../include/us_timer.h"

#include <qregexp.h>

// note: this program uses cout and/or cerr and this should be replaced

// static bool save_calc_to_csv = false;

#define SLASH QDir::separator()

void US_Hydrodyn_Saxs::calc_saxs_iq_native_sh()
{
   US_Timer us_timers;

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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }

      us_timers.init_timer( "atoms" );
      us_timers.init_timer( "rtp" );
      us_timers.init_timer( "ff" );

      progress_saxs->setProgress( 0, our_saxs_options->sh_max_harmonics + 3 );
      qApp->processEvents();

      vector < saxs_atom > atoms;
      saxs_atom new_atom;
      us_timers.start_timer( "atoms" );
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
               QColor save_color = editor->color();
               editor->setColor("red");
               editor->append(QString("%1Molecule %2 Residue %3 %4 Hybrid name missing. Atom skipped.\n")
                              .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                              .arg(j+1)
                              .arg(use_resname)
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
                              .arg(use_resname)
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
                              .arg(use_resname)
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

//             cout << QString("atom %1 hybrid %2 excl vol %3 by hybrid radius %4\n")
//                .arg(this_atom->name)
//                .arg(this_atom->hybrid_name)
//                .arg(atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol)
//                .arg(M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius)
//                ;

            new_atom.excl_vol = atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;

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
               new_atom.excl_vol = M_PI * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius * hybrid_map[hybrid_name].radius;
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
                 count_hydrogens.search(hybrid_name) != -1 )
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
                              .arg(use_resname)
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
      
      us_timers.end_timer( "atoms" );
      // save the atoms to a temporary file
      QString fsaxs_atoms_name = 
         USglobal->config_list.root_dir + 
         SLASH + "somo" + SLASH + "saxs" + SLASH + "tmp" + SLASH + QString("%1").arg(te_filename2->text()) +
         QString("_%1").arg( model_vector[ current_model ].model_id ) + 
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
         

      progress_saxs->setProgress( 1, our_saxs_options->sh_max_harmonics + 3 );
      qApp->processEvents();

      // recenter
      us_timers.start_timer( "rtp" );

      point cx;
      for ( unsigned int j = 0; j < 3; j++ )
      {
         cx.axis[ j ] = 0.0;
      }

      for ( unsigned int i = 0; i < atoms.size(); i++ )
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
      for ( unsigned int i = 1; i < atoms.size(); i++ )
      {
         for ( unsigned int j = 0; j < 3; j++ )
         {
            atoms[ i ].pos[ j ] -= cx.axis[ j ];
         }
      }
      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         atoms[ i ].rtp[ 0 ] = sqrt ( atoms[ i ].pos[ 0 ] * atoms[ i ].pos[ 0 ] +
                                      atoms[ i ].pos[ 1 ] * atoms[ i ].pos[ 1 ] +
                                      atoms[ i ].pos[ 2 ] * atoms[ i ].pos[ 2 ] );
         if ( atoms[ i ].rtp[ 0 ] == 0e0 )
         {
            atoms[ i ].rtp[ 1 ] = 0e0;
            atoms[ i ].rtp[ 2 ] = 0e0;
         } else {
            atoms[ i ].rtp[ 1 ] = acos ( atoms[ i ].pos[ 2 ] / atoms[ i ].rtp[ 0 ] );

            if ( atoms[ i ].pos[ 0 ] < 0.0 )
            {
               atoms[ i ].rtp[ 2 ] = M_PI - asin( atoms[ i ].pos[ 1 ] / sqrt( atoms[ i ].pos[ 0 ] * 
                                                                              atoms[ i ].pos[ 0 ] +
                                                                              atoms[ i ].pos[ 1 ] * 
                                                                              atoms[ i ].pos[ 1 ] ) );
            } else {
               if ( atoms[ i ].pos[ 1 ] < 0.0 )
               {
                  atoms[ i ].rtp[ 2 ] = M_2PI + asin( atoms[ i ].pos[ 1 ] / sqrt( atoms[ i ].pos[ 0 ] * 
                                                                                  atoms[ i ].pos[ 0 ] +
                                                                                  atoms[ i ].pos[ 1 ] * 
                                                                                  atoms[ i ].pos[ 1 ] ) );
               } else {
                  atoms[ i ].rtp[ 2 ] = asin( atoms[ i ].pos[ 1 ] / sqrt( atoms[ i ].pos[ 0 ] * 
                                                                          atoms[ i ].pos[ 0 ] +
                                                                          atoms[ i ].pos[ 1 ] * 
                                                                          atoms[ i ].pos[ 1 ] ) );
               }               
            }
         }               
      }
      us_timers.end_timer( "rtp" );
      progress_saxs->setProgress( 2, our_saxs_options->sh_max_harmonics + 3 );
      qApp->processEvents();

      vector < vector < double > > fib_grid;
      sh::build_grid( fib_grid, our_saxs_options->sh_fibonacci_grid_order );
      printf( "fib of order %u is %u fib_grid.size() %u\n", our_saxs_options->sh_fibonacci_grid_order, 
              sh::fibonacci( our_saxs_options->sh_fibonacci_grid_order ),
              fib_grid.size() );


      unsigned int q_points = 
         (unsigned int)floor(((our_saxs_options->end_q - our_saxs_options->start_q) / our_saxs_options->delta_q) + .5) + 1;

      if ( our_saxs_options->iq_exact_q )
      {
         editor_msg( "blue", QString( tr( "Using exact q" ) ) );
         if ( !exact_q.size() )
         {
            editor_msg( "dark red", QString( tr( "Notice: exact q is empty, computing based upon current q range " ) ) );
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
         editor->append(tr("Terminated by user request.\n"));
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }
      

      us_timers.start_timer( "ff" );
      vector < vector < double > > f;  // f(q,i) / atomic
      vector < vector < double > > fc;  // excluded volume
      vector < vector < double > > fp;  // f - fc
      vector < double > q;  // store q grid
      vector < double > q2; // store q^2
      vector < double > q_over_4pi_2; // store (q over 4pi)^2
      q.resize(q_points);

      double one_over_4pi = 1.0 / (4.0 * M_PI);
      double one_over_4pi_2 = one_over_4pi * one_over_4pi;

      if ( our_saxs_options->iq_exact_q )
      {
         q = exact_q;
         q_points = q.size();
      }
      q2.resize          ( q_points );
      q_over_4pi_2.resize( q_points );
      f.resize           ( q_points );
      fc.resize          ( q_points );
      fp.resize          ( q_points );

      for ( unsigned int j = 0; j < q_points; j++ )
      {
         f[j].resize(atoms.size());
         fc[j].resize(atoms.size());
         fp[j].resize(atoms.size());
         if ( !our_saxs_options->iq_exact_q )
         {
            q[j] = our_saxs_options->start_q + j * our_saxs_options->delta_q;
            // if ( q[j] < SAXS_MIN_Q ) 
            // {
            // q[j] = SAXS_MIN_Q;
            // }
         }
         q2[j] = q[j] * q[j];
         q_over_4pi_2[j] = q[j] * q[j] * one_over_4pi_2;
      }

      saxs saxsH = saxs_map["H"];
      cout << QString( "atoms.size() %1\n" ).arg( atoms.size() );
      if ( !atoms.size() )
      {
         editor_msg( "red", "No useable atoms found" );
         progress_saxs->reset();
         lbl_core_progress->setText("");
         pb_plot_saxs_sans->setEnabled(true);
         pb_plot_pr->setEnabled(true);
         return;
      }

      double vi_23_4pi;
      double vi; // excluded water vol
      double vie; // excluded water * e density

      for ( unsigned int i = 0; i < atoms.size(); i++ )
      {
         saxs saxs = saxs_map[atoms[i].saxs_name];

         vi = atoms[i].excl_vol;
         vie = vi * our_saxs_options->water_e_density;
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
                                      

            fc[j][i] = vie * exp( vi_23_4pi * q2[ j ] * our_saxs_options->ev_exp_mult );
            fp[j][i] = f[j][i] - fc[j][i];
         }
      }
      // f, f' computed
      us_timers.end_timer( "ff" );

      progress_saxs->setProgress( 3, our_saxs_options->sh_max_harmonics + 3 );
      qApp->processEvents();

      vector < double > I( q_points );

      if ( !our_saxs_options->alt_sh2 )
      {
         us_timers.init_timer( "legendre" );
         us_timers.init_timer( "combined" );

         if ( our_saxs_options->alt_sh1 )
         {
            us_timers.start_timer( "combined" );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[ j ] = 0e0;
            }

            complex < double > Z0( 0e0, 0e0 );
            double  J;
            vector < vector < complex < double > > >  A( our_saxs_options->sh_max_harmonics + 1 ); // A[ l ][ m ]
            complex < double > i_( 0e0, 1e0 );
         
            us_timers.start_timer( "legendre" );
            // setup associated legendre functions

            vector < vector < vector < complex < double > > > > Y; // Y[atom][l][m]
            Y.resize( atoms.size() );

            for ( unsigned int i = 0; i < atoms.size(); i++ )
            {
               Y[ i ].resize( our_saxs_options->sh_max_harmonics + 1 );

               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  Y[ i ][ l ].resize( m_size );

                  for ( int m = - (int) l ; m <= (int) l; m++ )
                  {
                     complex < double > result;
                     if ( !sh::conj_spherical_harmonic( l, 
                                                        m, 
                                                        atoms[ i ].rtp[ 1 ],
                                                        atoms[ i ].rtp[ 2 ],
                                                        Y[ i ][ l ][ m + l ] ) )
                     {
                        editor_msg( "red", "sh::spherical_harmonic failed" );
                        progress_saxs->reset();
                        lbl_core_progress->setText("");
                        pb_plot_saxs_sans->setEnabled(true);
                        pb_plot_pr->setEnabled(true);
                        return;
                     }
                  }
               }            
            }

            us_timers.end_timer( "legendre" );

            for ( unsigned int j = 0; j < q_points; j++ )
            {
               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  A[ l ].resize( m_size );
                  for ( unsigned int m = 0; m < m_size; m++ )
                  {
                     A[ l ][ m ] = Z0;
                  }
               }

               for ( unsigned int i = 0; i < atoms.size(); i++ )
               {
                  for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
                  {
                     complex < double > i_pow_l = pow( i_, l );
                     if ( !nr::sphbes( l, q[ j ] * atoms[ i ].rtp[ 0 ], J ) )
                     {
                        editor_msg( "red", "nr::shbes failed" );
                        progress_saxs->reset();
                        lbl_core_progress->setText("");
                        pb_plot_saxs_sans->setEnabled(true);
                        pb_plot_pr->setEnabled(true);
                        return;
                     }
                     for ( int m = - (int) l ; m <= (int) l; m++ )
                     {
                        A[ l ][ m + l ] += J * Y[ i ][ l ][ m + l ] * fp[ j ][ i ] * i_pow_l;
                     }
                  }
               }
               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  for ( unsigned int m = 0 ; m <= m_size; m++ )
                  {
                     I[ j ] += norm( A[ l ][ m ] );
                  }
               }
               I[ j ] *= 4e0 * M_PI;
            }
            us_timers.end_timer( "combined" );
         } else {
            us_timers.start_timer( "combined" );
            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[ j ] = 0e0;
            }

            complex < float > Z0( 0e0, 0e0 );
            double  J;
            vector < vector < complex < float > > >  A( our_saxs_options->sh_max_harmonics + 1 ); // A[ l ][ m ]
            complex < float > i_( 0e0, 1e0 );
         
            us_timers.start_timer( "legendre" );
            // setup associated legendre functions

            vector < vector < vector < complex < float > > > > Y; // Y[atom][l][m]
            Y.resize( atoms.size() );

            for ( unsigned int i = 0; i < atoms.size(); i++ )
            {
               Y[ i ].resize( our_saxs_options->sh_max_harmonics + 1 );

               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  Y[ i ][ l ].resize( m_size );

                  for ( int m = - (int) l ; m <= (int) l; m++ )
                  {
                     complex < double > result;
                     if ( !sh::conj_spherical_harmonic( l, 
                                                        m, 
                                                        atoms[ i ].rtp[ 1 ],
                                                        atoms[ i ].rtp[ 2 ],
                                                        result) )
                     {
                        editor_msg( "red", "sh::spherical_harmonic failed" );
                        progress_saxs->reset();
                        lbl_core_progress->setText("");
                        pb_plot_saxs_sans->setEnabled(true);
                        pb_plot_pr->setEnabled(true);
                        return;
                     }
                     Y[ i ][ l ][ m + l ] = result;
                  }
               }            
            }

            us_timers.end_timer( "legendre" );

            for ( unsigned int j = 0; j < q_points; j++ )
            {
               progress_saxs->setProgress( j, q_points );
               qApp->processEvents();

               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  A[ l ].resize( m_size );
                  for ( unsigned int m = 0; m < m_size; m++ )
                  {
                     A[ l ][ m ] = Z0;
                  }
               }

               for ( unsigned int i = 0; i < atoms.size(); i++ )
               {
                  for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
                  {
                     complex < float > i_pow_l = pow( i_, l );
                     if ( !nr::sphbes( l, q[ j ] * atoms[ i ].rtp[ 0 ], J ) )
                     {
                        editor_msg( "red", "nr::shbes failed" );
                        progress_saxs->reset();
                        lbl_core_progress->setText("");
                        pb_plot_saxs_sans->setEnabled(true);
                        pb_plot_pr->setEnabled(true);
                        return;
                     }
                     for ( int m = - (int) l ; m <= (int) l; m++ )
                     {
                        A[ l ][ m + l ] += (float) J * Y[ i ][ l ][ m + l ] * (float) fp[ j ][ i ] * i_pow_l;
                     }
                  }
               }
               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  unsigned int m_size = 1 + l * 2;
                  for ( unsigned int m = 0 ; m <= m_size; m++ )
                  {
                     I[ j ] += norm( A[ l ][ m ] );
                  }
               }
               I[ j ] *= 4e0 * M_PI;
            }
            us_timers.end_timer( "combined" );
         }
      } else {
         us_timers.init_timer( "bessel" );
         us_timers.init_timer( "legendre" );
         us_timers.init_timer( "amplitudes" );
         us_timers.init_timer( "I" );
         us_timers.init_timer( "combined" );

         us_timers.start_timer( "bessel" );
         us_timers.start_timer( "combined" );
         cout << "compute spherical bessel functions\n";
         // setup spherical bessel functions
         vector < vector < vector < double > > >  J; // J[atom][n][q_point]
         J.resize( atoms.size() );
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            J[ i ].resize( our_saxs_options->sh_max_harmonics + 1 );
            for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
            {
               J[ i ][ l ].resize( q_points );
               for ( unsigned int j = 0; j < q_points; j++ )
               {
                  if ( !nr::sphbes( l, q[ j ] * atoms[ i ].rtp[ 0 ], J[ i ][ l ][ j ] ) )
                  {
                     editor_msg( "red", "nr::shbes failed" );
                     progress_saxs->reset();
                     lbl_core_progress->setText("");
                     pb_plot_saxs_sans->setEnabled(true);
                     pb_plot_pr->setEnabled(true);
                     return;
                  }
               }
            }
         }      
         us_timers.end_timer( "bessel" );

         cout << "compute associated legendre functions\n";
         us_timers.start_timer( "legendre" );
         // setup associated legendre functions

         vector < map < unsigned int, map < int , complex < double > > > > Y; // Y[atom][l][m]
         Y.resize( atoms.size() );

         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
            {
               for ( int m = - (int) l ; m <= (int) l; m++ )
               {
                  complex < double > result;
                  if ( !sh::conj_spherical_harmonic( l, 
                                                     m, 
                                                     atoms[ i ].rtp[ 1 ],
                                                     atoms[ i ].rtp[ 2 ],
                                                     result ) )
                  {
                     editor_msg( "red", "sh::spherical_harmonic failed" );
                     progress_saxs->reset();
                     lbl_core_progress->setText("");
                     pb_plot_saxs_sans->setEnabled(true);
                     pb_plot_pr->setEnabled(true);
                     return;
                  }
                  Y[ i ][ l ][ m ] = result;
               }
            }            
         }
         us_timers.end_timer( "legendre" );

         cout << "compute amplitudes\n";
         // compute A

         if ( !our_saxs_options->alt_sh1 )
         {
            us_timers.start_timer( "amplitudes" );
            complex < double > i_( 0e0, 1e0 );

            map < unsigned int, map < int , vector < complex < double > > > > A; // A[l][m][q_point]

            for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
            {
               complex < double > i_pow_l = pow( i_, l );
               for ( int m = - (int) l ; m <= (int) l; m++ )
               {
                  A[ l ][ m ].resize( q_points );
                  for ( unsigned int j = 0; j < q_points; j++ )
                  {
                     A[ l ][ m ][ j ] = complex < double > ( 0e0, 0e0 );
                     for ( unsigned int i = 0; i < atoms.size(); i++ )
                     {
                        A[ l ][ m ][ j ] += J[ i ][ l ][ j ] * Y[ i ][ l ][ m ] * fp[ j ][ i ];
                     }
                     A[ l ][ m ][ j ] *= i_pow_l;
                  }
               }
            }
            us_timers.end_timer( "amplitudes" );
            cout << "compute I\n";
            us_timers.start_timer( "I" );

            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[ j ] = 0e0;
               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  for ( int m = - (int) l ; m <= (int) l; m++ )
                  {
                     I[ j ] += norm( A[ l ][ m ][ j ] );
                  }
               }
               I[ j ] *= 4e0 * M_PI;
            }
            us_timers.end_timer( "I" );
            us_timers.end_timer( "combined" );
         } else {
            us_timers.start_timer( "amplitudes" );
            us_timers.start_timer( "combined" );
            complex < float > i_( 0e0, 1e0 );

            map < unsigned int, map < int , vector < complex < float > > > > A; // A[l][m][q_point]

            for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
            {
               complex < float > i_pow_l = pow( i_, l );
               for ( int m = - (int) l ; m <= (int) l; m++ )
               {
                  A[ l ][ m ].resize( q_points );
                  for ( unsigned int j = 0; j < q_points; j++ )
                  {
                     A[ l ][ m ][ j ] = complex < float > ( 0e0, 0e0 );
                     for ( unsigned int i = 0; i < atoms.size(); i++ )
                     {
                        A[ l ][ m ][ j ] += J[ i ][ l ][ j ] * Y[ i ][ l ][ m ] * fp[ j ][ i ];
                     }
                     A[ l ][ m ][ j ] *= i_pow_l;
                  }
               }
            }

            us_timers.end_timer( "amplitudes" );
            cout << "compute I\n";
            us_timers.start_timer( "I" );

            for ( unsigned int j = 0; j < q_points; j++ )
            {
               I[ j ] = 0e0;
               for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
               {
                  for ( int m = - (int) l ; m <= (int) l; m++ )
                  {
                     I[ j ] += norm( A[ l ][ m ][ j ] );
                  }
               }
               I[ j ] *= 4e0 * M_PI;
            }
            us_timers.end_timer( "I" );
            us_timers.end_timer( "combined" );
         }
      }

      cout << "list times:\n" << us_timers.list_times() << endl << flush;

      unsigned int pts = 0;
      for ( unsigned int l = 0; l <= our_saxs_options->sh_max_harmonics; l++ )
      {
         for ( int m = - (int) l ; m <= (int) l; m++ )
         {
            pts++;
         }
      }
      printf( "pts %u\n", pts );
      
      //QString name = 
      //          QString("%1%2_%3%4")
      //          .arg( QFileInfo(te_filename2->text()).fileName() )
      //          .arg( specname )
      //          .arg( model_vector[ current_model ].model_id )
      //          .arg( iqq_suffix() );

      
      //       name += our_saxs_options->alt_sh1 ? "_f" : "_d";
      //       name += our_saxs_options->alt_sh2 ? "_fast" : "_org";

      // plot_one_iqq( q, I, name );

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

      editor->append(QString(tr("Total excluded volume %1\n")).arg(tot_excl_vol));
      editor->append(QString(tr("Average electron density %1\n")).arg(total_e / tot_excl_vol, 4));
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         editor->append(QString(tr("Total unhydrated excluded volume %1\n")).arg(tot_excl_vol_noh));
         editor->append(QString(tr("Average unhydrated electron density %1\n")).arg(total_e_noh / tot_excl_vol_noh));
         editor->append(QString(tr("Electron density of hydration %1\n")).arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh)));
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

#ifndef QT4
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

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

#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
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
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
                  fprintf(fsaxs, "%.6e\t%.6e\n", q[i], I[i] );
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_q.push_back(q[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqq.push_back(I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(I[i]);
                  ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(0e0);
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
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqa.push_back(I[i]);
            ((US_Hydrodyn *)us_hydrodyn)->last_saxs_iqqc.push_back(0e0);
         }
      }
   }


   progress_saxs->reset();
   lbl_core_progress->setText("");
   pb_plot_saxs_sans->setEnabled(true);
   pb_plot_pr->setEnabled(true);
}

#if defined( OLD_WAY )
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
            editor_msg("red", tr( "Volume is zero so NOT multiplying I(q) by atomic volume\n" ) );
         } else {
            editor_msg("blue", QString( tr( "Multiplying I(q) by atomic volume of %1\n" ) )
                       .arg( model_vector[ current_model ].volume ) );
            for ( unsigned int i = 0; i < I.size(); i++ )
            {
               I[ i ] *= model_vector[ current_model ].volume;
            }
            if ( is_only_one_swh && ((US_Hydrodyn *)us_hydrodyn)->multiply_iq_by_atomic_volume_last_water_multiplier > 0 )
            {
               editor_msg("blue", QString( tr( "Multiplying I(q) by water multiplier of %1\n" ) )
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

      editor->append(QString(tr("Total excluded volume %1\n")).arg(tot_excl_vol));
      editor->append(QString(tr("Average electron density %1\n")).arg(total_e / tot_excl_vol, 4));
      if ( tot_excl_vol_noh != tot_excl_vol ||
           total_e_noh      != total_e )
      {
         editor->append(QString(tr("Total unhydrated excluded volume %1\n")).arg(tot_excl_vol_noh));
         editor->append(QString(tr("Average unhydrated electron density %1\n")).arg(total_e_noh / tot_excl_vol_noh));
         editor->append(QString(tr("Electron density of hydration %1\n")).arg((total_e - total_e_noh) / (tot_excl_vol - tot_excl_vol_noh)));
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

#ifndef QT4
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

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

#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
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
                           "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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
                     "Simulated SAXS data generated from %s by US_SOMO %s %s q(%f:%f) step %f\n"
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

#endif // OLD_WAY

void US_Hydrodyn_Saxs::calc_saxs_iq_native_sh_bead_model()
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
         editor->append(tr("Terminated by user request.\n"));
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

      FILE *fsaxs_atoms = fopen(fsaxs_atoms_name, "w");
      if ( fsaxs_atoms ) 
      {
         for ( unsigned int i = 0; i < atoms.size(); i++ )
         {
            fprintf(fsaxs_atoms, "%s %s %.3f %.3f %.3f %.2f %.2f %.2f\n"
                    , atoms[i].saxs_name.ascii()
                    , atoms[i].hybrid_name.ascii()
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
         editor->append(tr("Terminated by user request.\n"));
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
         // if ( q[j] < SAXS_MIN_Q ) 
         // {
         // q[j] = SAXS_MIN_Q;
         // }
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
      set_scaling_target( scaling_target );

      // crop_iq_data(q, I);
         
      if ( q.size() &&
           !scaling_target.isEmpty() && 
           plotted_iq_names_to_pos.count(scaling_target) )
      {
         rescale_iqq_curve( scaling_target, q, I );
      }


      editor->append(QString(tr("Total excluded volume %1\n")).arg(tot_excl_vol));

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

#ifndef QT4
      long Iq = plot_saxs->insertCurve( plot_name );
#else
      QwtPlotCurve *curve = new QwtPlotCurve( plot_name );
#endif

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

#ifndef QT4
      plot_saxs->setCurveData(Iq, 
                              cb_guinier->isChecked() ?
                              (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                              cb_kratky ->isChecked() ?
                              (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
                              q_points );
      plot_saxs->setCurvePen(Iq, QPen(plot_colors[p % plot_colors.size()], 2, SolidLine));
#else
      curve->setData(
                     cb_guinier->isChecked() ?
                     (double *)&(plotted_q2[p][0]) : (double *)&(plotted_q[p][0]), 
                     cb_kratky ->isChecked() ?
                     (double *)&(q2I[0])           : (double *)&(plotted_I[p][0]),
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
            fsaxs_name = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck(fsaxs_name, 0, this);
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

