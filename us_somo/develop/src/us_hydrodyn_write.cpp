// this is part of the class US_Hydrodyn
// listing of other files is in us_hydrodyn.cpp
// (this) us_hydrodyn_write.cpp contains code for writing various files

#include "../include/us_hydrodyn.h"
#define SLASH "/"

void US_Hydrodyn::write_bead_spt(QString fname, 
                                 vector<PDB_atom> *model, 
                                 bool movie_frame, 
                                 float scale, 
                                 bool black_background) 
{
   const char *colormap[] =
      {
         "black",        // 0 black
         "blue",         // 1 blue
         "greenblue",    // 2 greenblue
         "cyan",         // 3 cyan
         "red",          // 4 red
         "magenta",      // 5 magenta
         "orange",       // 6 orange
         "white",        // 7 white
         "[200,80,0]",   // 8 gray
         "purple",       // 9 purple
         "green",        // 10 green
         "cyan",         // 11 cyan
         "redorange",    // 12 redorange
         "violet",       // 13 violet
         "yellow",       // 14 yellow
         "red",          // 15 red
      };
#define COLORMAP_SIZE 16

#if defined(DEBUG)
   printf("write bead spt %s\n", fname.toLatin1().data()); fflush(stdout);
#endif

   FILE *fspt = us_fopen(QString("%1.spt").arg(fname).toLatin1().data(), "w");
   FILE *fbms = us_fopen(QString("%1.bms").arg(fname).toLatin1().data(), "w");

   int beads = 0;
   if(!(fspt) ||
      !(fbms)) {
      printf("file write error!\n"); fflush(stdout);
   }

   float max_radius = 0;
   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         beads++;
         if (max_radius < (*model)[i].bead_computed_radius) {
            max_radius = (*model)[i].bead_computed_radius;
         }
      }
   }

# define MAX_RADIUS 10.0
   float scaling = 1.0;
   if(max_radius > MAX_RADIUS) {
      scaling = max_radius / MAX_RADIUS;
   }
   scaling = 5.0 / scale;
   fprintf(fbms,
           "%d\n%s\n",
           beads + 1,
           QFileInfo(fname).fileName().toLatin1().data()
           );
   //   fprintf(fspt,
   //           "load xyz %s\nselect all\nwireframe off\nset background white\n",
   //           QString("%1.bms").arg(QFileInfo(fname).fileName()).toLatin1().data()
   //           );

   last_spt_text = 
      QString("").sprintf("load xyz %s\nselect all\nwireframe off\nset background %s\n",
                          QString("%1.bms").arg(QFileInfo(fname).fileName()).toLatin1().data(),
                          black_background ? "black" : "white"
                          );

   int atomno = 0;
   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         fprintf(fbms,
                 "Pb %.2f %.2f %.2f\n",
                 (*model)[0].bead_coordinate.axis[0] / scaling,
                 (*model)[0].bead_coordinate.axis[1] / scaling,
                 (*model)[0].bead_coordinate.axis[2] / scaling
                 );
         break;
      }
   }

   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         if ((*model)[i].bead_color >= COLORMAP_SIZE ) {
            printf("ERROR: bead color for bead %u is to large %u\n",
                   (*model)[i].serial,
                   get_color(&(*model)[i])); fflush(stdout);
         }
         fprintf(fbms,
                 "Pb %.2f %.2f %.2f\n",
                 (*model)[i].bead_coordinate.axis[0] / scaling,
                 (*model)[i].bead_coordinate.axis[1] / scaling,
                 (*model)[i].bead_coordinate.axis[2] / scaling
                 );
         last_spt_text += QString("")
            .sprintf(
                     "select atomno=%d\nspacefill %.2f\ncolour %s\n",
                     ++atomno,
                     (*model)[i].bead_computed_radius / scaling,
                     colormap[get_color(&(*model)[i])]
                     );
      }
   }

   if ( movie_frame )
   {
      last_spt_text += QString("")
         .sprintf(
                  "save ppm %s.ppm\n"
                  "exit\n",
                  fname.toLatin1().data()
                  );
      movie_text.push_back(fname);
   }
   // fprintf(fspt, ( last_spt_text.isNull() ? "" : last_spt_text.toLatin1().data() ) );
   fputs( ( last_spt_text.isNull() ? "" : last_spt_text.toLatin1().data() ), fspt );
   fclose(fspt);
   fclose(fbms);
}

void US_Hydrodyn::write_bead_tsv(QString fname, vector<PDB_atom> *model) {
   FILE *f = us_fopen(fname.toLatin1().data(), "w");
   fprintf(f, "name~residue~chainID~"
           "position~active~radius~asa~mw~"
           "bead #~chain~serial~is_bead~bead_asa~visible~code/color~"
           "bead mw~position controlled?~bead positioner~baric method~bead hydration~bead color~"
           "bead ref volume~bead ref mw~bead computed radius~"
           "position_coordinate~cog position~use position~sizeof(parents)~beads referenced\n");

   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         QString beads_referenced = "";
         for (unsigned int j = 0; j < (*model)[i].all_beads.size(); j++) {

            beads_referenced +=
               QString("%1-%2-%3-%4;").
               arg(((*model)[i].all_beads)[j]->serial).
               arg(((*model)[i].all_beads)[j]->name).
               arg(((*model)[i].all_beads)[j]->resName).
               arg(((*model)[i].all_beads)[j]->chainID);

            //     ((*model)[i].all_beads)[j]->serial + "-" +
            //     ((*model)[i].all_beads)[j]->name + "-" +
            //     ((*model)[i].all_beads)[j]->resName + "-" +
            //     ((*model)[i].all_beads)[j]->chainID + ";";
         }
         fprintf(f,
                 "%s~%s~%s~"
                 "[%f,%f,%f]~%s~%f~%f~%f~"
                 "%d~%d~%d~%s~%f~%s~"
                 "%d~%f~%s~%s~%d~%f~%u~"
                 "%f~%f~%f~"
                 "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]~%u~%s\n",

                 (*model)[i].name.toLatin1().data(),
                 (*model)[i].resName.toLatin1().data(),
                 (*model)[i].chainID.toLatin1().data(),

                 (*model)[i].coordinate.axis[0],
                 (*model)[i].coordinate.axis[1],
                 (*model)[i].coordinate.axis[2],
                 (*model)[i].active ? "Y" : "N",
                 (*model)[i].radius,
                 (*model)[i].asa,
                 (*model)[i].mw + (*model)[i].ionized_mw_delta,

                 (*model)[i].bead_number,
                 (*model)[i].chain,
                 (*model)[i].serial,
                 (*model)[i].is_bead ? "Y" : "N",
                 (*model)[i].bead_asa,
                 (*model)[i].visibility ? "Y" : "N",

                 (*model)[i].exposed_code,
                 (*model)[i].bead_ref_mw + (*model)[i].bead_ref_ionized_mw_delta,
                 (*model)[i].bead_positioner ? "Y" : "N",
                 (*model)[i].active ? ((*model)[i].bead_positioner ? "Y" : "N") : "Inactive",
                 (*model)[i].placing_method,
                 (*model)[i].bead_hydration,
                 (*model)[i].bead_color,

                 (*model)[i].bead_ref_volume,
                 (*model)[i].bead_ref_mw + (*model)[i].bead_ref_ionized_mw_delta,
                 (*model)[i].bead_computed_radius,

                 (*model)[i].bead_position_coordinate.axis[0],
                 (*model)[i].bead_position_coordinate.axis[1],
                 (*model)[i].bead_position_coordinate.axis[2],
                 (*model)[i].bead_cog_coordinate.axis[0],
                 (*model)[i].bead_cog_coordinate.axis[1],
                 (*model)[i].bead_cog_coordinate.axis[2],
                 (*model)[i].bead_coordinate.axis[0],
                 (*model)[i].bead_coordinate.axis[1],
                 (*model)[i].bead_coordinate.axis[2],
                 (unsigned int)(*model)[i].all_beads.size(),
                 beads_referenced.toLatin1().data()
                 );
      }
   }
   fclose(f);
}

void US_Hydrodyn::write_bead_asa(QString fname, vector<PDB_atom> *model) {
   FILE *f = us_fopen(fname.toLatin1().data(), "w");
   fprintf(f, " N.      Res.       ASA        MAX ASA         %%\n");

   float total_asa = 0.0;
   float total_ref_asa = 0.0;
   float total_vol = 0.0;
   float total_mass = 0.0;

   QString last_residue = "";
   int seqno = 0;
   float residue_asa = 0;
   float residue_ref_asa = 0;

   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         total_asa += (*model)[i].bead_asa;
         total_ref_asa += (*model)[i].ref_asa;
         total_mass += (*model)[i].bead_ref_mw + (*model)[i].bead_ref_ionized_mw_delta;
         // printf("write_bead_asa model[%d].bead_ref_mw %g\n",
         //        i, ((*model)[i].bead_ref_mw) + (*model)[i].bead_ref_ionized_mw_delta);
         total_vol += (*model)[i].bead_ref_volume_unhydrated;

         QString residue =
            (*model)[i].resName + "_" +
            ((*model)[i].chainID == " " ? "_" : (*model)[i].chainID) +
            QString("_%1").arg((*model)[i].resSeq);
         if (residue != last_residue) {
            if (last_residue != "") {
               fprintf(f,
                       " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
                       seqno, last_residue.toLatin1().data(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
            }
            residue_asa = 0;
            residue_ref_asa = 0;
            last_residue = residue;
            seqno++;
         }
         residue_asa += (*model)[i].bead_asa;
         residue_ref_asa = (*model)[i].ref_asa;
      }
   }
   if (last_residue != "") {
      fprintf(f,
              " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
              seqno, last_residue.toLatin1().data(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
   }

   fprintf(f,
           "\n\n\n"
           "\tTOTAL ASA OF THE MOLECULE    = %.0f\t[A^2] (Threshold used: %.1f A^2]\n"
           // "\tTOTAL VOLUME OF THE MOLECULE = %-.2f\t[A^3]\n"
           "\tRADIUS OF GYRATION (+r) =  %-.2f   [A]\n"
           "\tRADIUS OF GYRATION (-r) =  %-.2f   [A]\n"
           "\tMASS OF THE MOLECULE    =  %.0f   [Da]\n"
           "\tCENTRE OF MASS          =  %.4f %.4f %.4f [A]\n"
           ,
           total_asa,
           asa.threshold,
           // total_vol,
           results.asa_rg_pos,
           results.asa_rg_neg,
           total_mass,
           last_molecular_cog.axis[0], last_molecular_cog.axis[1], last_molecular_cog.axis[2]
           );

   fclose(f);
   editor_msg("dark blue", QString("").sprintf("Anhydrous volume %.2f A^3", total_vol));
}

void US_Hydrodyn::write_bead_model(
                                   QString fname,
                                   vector < PDB_atom > *model,
                                   int bead_model_output = US_HYDRODYN_OUTPUT_SOMO,
                                   QString extra_text
                                   ) {
   int save_output = bead_output.output;
   bead_output.output = bead_model_output;
   write_bead_model( fname, model, extra_text );
   bead_output.output = save_output;
}

void US_Hydrodyn::write_bead_model( QString fname, 
                                    vector < PDB_atom > *model,
                                    QString extra_text )
{
   last_bead_model = "";
   // write corresopdence file also
   int decpts = -(int)log10(overlap_tolerance/9.9999) + 1;
   if (decpts < 4) {
      decpts = 4;
   }

   QString fstring_somo =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\t%.6f\t%d\t%s\t%.4f\t%f\t%.4f\t%f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts).
      arg(decpts);

   QString fstring_beams =
      QString("%.%1f\t%.%2f\t%.%3f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts);

   QString fstring_rmc =
      QString("%.%1f\t%.0f\t%d\n"). // zero digit MW
      arg(decpts);

   QString fstring_rmc1 =
      QString("%.%1f\t%.0f\t%d\t%s\n"). // zero digit MW
      arg(decpts);

   QString fstring_hydro =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\n").
      arg(decpts).
      arg(decpts).
      arg(decpts).
      arg(decpts);

#if defined(DEBUG)
   printf("write bead model %s\n", fname.toLatin1().data()); fflush(stdout);
   printf("decimal points to use: %d\n", decpts);
#endif

   vector <PDB_atom *> use_model;
   switch (bead_output.sequence) {
   case 0: // as in original pdb file
   case 2: // include bead-original residue correspondence
      for (unsigned int i = 0; i < model->size(); i++) {
         use_model.push_back(&(*model)[i]);
      }
      break;
   case 1: // exposed sidechain -> exposed main chain -> buried
      {
         map < unsigned int, bool > used;
         for (unsigned int i = 0; i < model->size(); i++) {
            if ((*model)[i].visibility == 1 &&
                (*model)[i].chain == 1) {
               use_model.push_back(&(*model)[i]);
               used[ i ] = true;
            }
         }
         for (unsigned int i = 0; i < model->size(); i++) {
            if ((*model)[i].visibility == 1 &&
                (*model)[i].chain == 0) {
               use_model.push_back(&(*model)[i]);
               used[ i ] = true;
            }
         }
         for (unsigned int i = 0; i < model->size(); i++) {
            if ((*model)[i].visibility == 0 &&
                (*model)[i].chain == 1) {
               use_model.push_back(&(*model)[i]);
               used[ i ] = true;
            }
         }
         for (unsigned int i = 0; i < model->size(); i++) {
            if ((*model)[i].visibility == 0 &&
                (*model)[i].chain == 0) {
               use_model.push_back(&(*model)[i]);
               used[ i ] = true;
            }
         }
         for (unsigned int i = 0; i < model->size(); i++) {
            if ( !used.count( i ) )
            {
               use_model.push_back(&(*model)[i]);
            }
         }
      }

   default :
      break;
   }

   FILE *fsomo = (FILE *)0;
   FILE *fbeams = (FILE *)0;
   FILE *frmc = (FILE *)0;
   FILE *frmc1 = (FILE *)0;
   FILE *fhydro = (FILE *)0;
   FILE *fgrpy = (FILE *)0;

   if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
      fsomo = us_fopen(QString("%1.bead_model").arg(fname).toLatin1().data(), "w");
      last_bead_model = QString("%1.bead_model").arg(fname);
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
      fbeams = us_fopen(QString("%1.beams").arg(fname).toLatin1().data(), "w");
      frmc = us_fopen(QString("%1.rmc").arg(fname).toLatin1().data(), "w");
      frmc1 = us_fopen(QString("%1.rmc1").arg(fname).toLatin1().data(), "w");
      if ( last_bead_model.isEmpty() ) {
         last_bead_model = QString("%1.beams").arg(fname);
      }
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
      fhydro = us_fopen(QString("%1.dat").arg(fname).toLatin1().data(), "w");
      if ( last_bead_model.isEmpty() ) {
         last_bead_model = QString("%1.dat").arg(fname);
      }
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_GRPY) {
      fgrpy = us_fopen(QString("%1.grpy").arg(fname).toLatin1().data(), "w");
      if ( last_bead_model.isEmpty() ) {
         last_bead_model = QString("%1.grpy").arg(fname);
      }
   }

   int beads = 0;

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         beads++;
      }
   }

   if (fsomo) {
      fprintf(fsomo,
              "%d\t%.3f\n",
              beads,
              results.vbar
              );
   }

   if (fbeams) {
      fprintf(fbeams,
              "%d\t-2.000000\t%s.rmc\t%.3f\n",
              beads,
              QFileInfo(fname).fileName().toLatin1().data(),
              results.vbar
              );
   }

   if (fhydro) {
      fprintf(fhydro,
              "1.E%d,\t!Unit of length for coordinates and radii, cm\n"
              "%d,\t!Number of beads\n",
              hydro.unit + 2,
              beads
              );
   }

   if (fgrpy) {
      double tot_mw = model_mw( use_model );

      fprintf(fgrpy,
              "%-30s\tTitle\n"
              "%-30g\tTemperature\n"
              // "%-30g\tpH\n"
              "%-30g\tSolvent viscosity\n"
              "%-30g\tMolecular weight\n"
              "%-30g\tSpecific volume of macromolecule\n"
              "%-30g\tSolution Density\n"
              "1.E%d\t\t\t\tUnit of length for coordinates and radii, cm\n"
              "%-30d\tNumber of beads\n"

              ,QFileInfo(fname).fileName().toLatin1().data()
              ,hydro.temperature
              // ,hydro.pH
              ,use_solvent_visc() * 0.01
              ,tot_mw
              ,results.vbar
              ,use_solvent_dens()
              ,hydro.unit + 2
              ,grpy_used_beads_count( use_model )
              
              );
   }


   double summary_mw = 0e0;

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         QString tmp_serial = use_model[i]->resSeq; // was serial
         QString residues;

         if (!bead_model_from_file) {
            if ( use_model[i]->bead_model_code.isEmpty() ) {
               residues =
                  use_model[i]->resName +
                  (use_model[i]->org_chain ? ".SC." : 
                   ((misc.pb_rule_on && !use_model[i]->type) ? ".PB." : ".MC.")) +
                  (use_model[i]->chainID == " " ? "" : (use_model[i]->chainID + "."));
               // a compiler error forced this kludge using tmp_serial
               //   + QString("%1").arg((*use_model)[i].serial);
               residues += QString("%1").arg(tmp_serial);

               for (unsigned int j = 0; j < use_model[i]->all_beads.size(); j++)
               {
                  QString tmp_serial = use_model[i]->all_beads[j]->resSeq;

                  residues += "," +
                     (use_model[i]->all_beads[j]->resName +
                      (use_model[i]->all_beads[j]->org_chain ? ".SC." : 
                       ((misc.pb_rule_on && !use_model[i]->type) ? ".PB." : ".MC.")) +
                      (use_model[i]->all_beads[j]->chainID == " " ? "" : (use_model[i]->all_beads[j]->chainID + ".")));
                  // a compiler error forced this kludge using tmp_serial
                  //  + QString("%1").arg((*use_model)[i].all_beads[j].serial);
                  residues += QString("%1").arg(tmp_serial);
               }
            } else {
               residues = use_model[i]->bead_model_code;
            }
         }
         else
         {
            residues = use_model[i]->residue_list;
         }
         summary_mw += use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta;
         // QTextStream( stdout ) << "bead ref " << i << " mw " << use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta << Qt::endl;

         if (fsomo) {
            fprintf(fsomo,
                    fstring_somo.toLatin1().data(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta,
                    get_color(use_model[i]),
                    residues.toLatin1().data(),
                    use_model[i]->bead_recheck_asa,
                    use_model[i]->num_elect,
                    use_model[i]->saxs_excl_vol,
                    use_model[i]->bead_hydration
                    );
         }
         if (fbeams) {
            fprintf(fbeams,
                    fstring_beams.toLatin1().data(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2]
                    );
            fprintf(frmc,
                    fstring_rmc.toLatin1().data(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta,
                    get_color(use_model[i]));
            fprintf(frmc1,
                    fstring_rmc1.toLatin1().data(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta,
                    get_color(use_model[i]),
                    residues.toLatin1().data()
                    );
         }
         if (fhydro) {
            fprintf(fhydro,
                    fstring_hydro.toLatin1().data(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius);
         }
         if (fgrpy) {
            int color = get_color( use_model[ i ] );
            if ( hydro.grpy_bead_inclusion || color != 6 ) {
               fprintf(fgrpy,
                       fstring_hydro.toLatin1().data(),
                       use_model[i]->bead_coordinate.axis[0],
                       use_model[i]->bead_coordinate.axis[1],
                       use_model[i]->bead_coordinate.axis[2],
                       use_model[i]->bead_computed_radius);
            }
         }
      // } else {
      //    qDebug() << "bead " << i << " inactive";
      }
   }
   if (fsomo) {
      fprintf(fsomo,
              "\n"
              "Field contents:\n"
              "  Line 1: Number of beads, Global partial specific volume\n"
              "  From line 2 on: X, Y, Z coordinates, Radius, Mass, Color coding, "
              "Correspondence with original residues, ASA, electrons, scattering_factor, hydration waters\n"

              "\n"
              "Bead Model Output:\n"
              "  Sequence:                   %s\n"
              "\n"
              "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : %d\n"

              ,bead_output.sequence ?
              (bead_output.sequence == 1 ?
               "exposed sidechain -> exposed main chain -> buried" :
               "include bead-original residue correspondence") :
              "as in original PDB file"
              ,-hydro.unit
              );
      // fprintf(fsomo, ( options_log.isNull() ? "" : options_log.toLatin1().data() ) );
      // fprintf(fsomo, ( last_abb_msgs.isNull() ? "" : last_abb_msgs.toLatin1().data() ) );
      fputs(( options_log.isNull() ? "" : options_log.toLatin1().data() ), fsomo );
      fputs(( last_abb_msgs.isNull() ? "" : last_abb_msgs.toLatin1().data() ), fsomo );
      fprintf(fsomo, "\nMolecular weight total: %.2f [Da]\n", summary_mw );
      if ( !extra_text.isEmpty() )
      {
         // fprintf(fsomo, extra_text.toLatin1().data() );
         fputs(extra_text.toLatin1().data(), fsomo );
      }
      qDebug() << "write bead model total mw " << summary_mw;

      if ( model->size() && (*model)[0].is_vdw == "vdw" ) {
         QString qs =
            QString(
                    "\nvdW model parameters:\n"
                    "  Hydrate probe radius   [A] : %1\n"
                    "  Hydrate threshold    [A^2] : %2\n"
                    "  Theoretical waters         : %3\n"
                    "  Exposed residues           : %4\n"
                    "  Theoretical waters exposed : %5\n"
                    )
            .arg( (*model)[0].asa_hydrate_probe_radius )
            .arg( (*model)[0].asa_hydrate_threshold )
            .arg( (*model)[0].vdw_theo_waters, 0, 'f', 0 )
            .arg( (*model)[0].vdw_count_exposed )
            .arg( (*model)[0].vdw_theo_waters_exposed, 0, 'f', 0 )
            ;
         fputs(( qs.toLatin1().data() ), fsomo );
      }         
      fclose(fsomo);
   }
   if (fbeams) {
      fclose(fbeams);
      fclose(frmc);
      fclose(frmc1);

   }
   if (fhydro) {
      fclose(fhydro);
   }
   if (fgrpy) {
      fclose(fgrpy);
   }
}

void US_Hydrodyn::write_corr( QString fname, vector<PDB_atom> *model ) 
{
   FILE *fcorr = (FILE *)0;
   fcorr = us_fopen(QString("%1.corr").arg(fname).toLatin1().data(), "w");
   vector <PDB_atom *> use_model;
   for (unsigned int i = 0; i < model->size(); i++) {
      use_model.push_back(&(*model)[i]);
   }

   if ( fcorr )
   {
      fprintf(fcorr, "%.3f\n", results.vbar);
   }

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         QString tmp_serial = use_model[i]->resSeq; // was serial
         QString residues;

         if (!bead_model_from_file) {
            residues =
               use_model[i]->resName +
               (use_model[i]->org_chain ? ".SC." : 
                ((misc.pb_rule_on && !use_model[i]->type) ? ".PB." : ".MC.")) +
               (use_model[i]->chainID == " " ? "" : (use_model[i]->chainID + "."));
            // a compiler error forced this kludge using tmp_serial
            //   + QString("%1").arg((*use_model)[i].serial);
            residues += QString("%1").arg(tmp_serial);

            for (unsigned int j = 0; j < use_model[i]->all_beads.size(); j++)
            {
               QString tmp_serial = use_model[i]->all_beads[j]->resSeq;

               residues += "," +
                  (use_model[i]->all_beads[j]->resName +
                   (use_model[i]->all_beads[j]->org_chain ? ".SC." : 
                    ((misc.pb_rule_on && !use_model[i]->type) ? ".PB." : ".MC.")) +
                   (use_model[i]->all_beads[j]->chainID == " " ? "" : (use_model[i]->all_beads[j]->chainID + ".")));
               // a compiler error forced this kludge using tmp_serial
               //  + QString("%1").arg((*use_model)[i].all_beads[j].serial);
               residues += QString("%1").arg(tmp_serial);
            }
         }
         else
         {
            residues = use_model[i]->residue_list;
         }
         if (fcorr) {
            fprintf(fcorr,
                    "%s\n%s\n%s\n%s\n%s\n%f\n%f\n%f\n%d\n%u\n%d\n%u\n",
                    use_model[i]->name.toLatin1().data(),
                    use_model[i]->resName.toLatin1().data(),
                    use_model[i]->chainID.toLatin1().data(),
                    use_model[i]->resSeq.toLatin1().data(),
                    use_model[i]->iCode.toLatin1().data(),
                    use_model[i]->bead_ref_mw + use_model[i]->bead_ref_ionized_mw_delta,
                    use_model[i]->bead_ref_volume,
                    use_model[i]->bead_recheck_asa,
                    use_model[i]->chain,
                    use_model[i]->serial,
                    use_model[i]->exposed_code,
                    use_model[i]->bead_color
                    );
         }
      }
   }
   fclose(fcorr);
}

bool US_Hydrodyn::read_corr(QString fname, vector<PDB_atom> *model) {
   QFile f(fname);
   vector < PDB_atom > new_model;
   new_model.resize(model->size());
   bool result = false;
   if ( f.open(QIODevice::ReadOnly) )
   {
      editor->append(QString("Reading correspondence file %1\n").arg(fname));
      QTextStream ts(&f);
      results.vbar = ts.readLine().toFloat();
      unsigned int i = 0;
      while( !ts.atEnd() && i < model->size() )
      {
         new_model[i].name = ts.readLine();
         new_model[i].resName = ts.readLine();
         new_model[i].chainID = ts.readLine();
         new_model[i].resSeq = ts.readLine();
         new_model[i].iCode = ts.readLine();
         new_model[i].bead_ref_mw = ts.readLine().toFloat();
         new_model[i].bead_ionized_mw_delta = 0;
         new_model[i].bead_ref_volume = ts.readLine().toFloat();
         new_model[i].bead_recheck_asa = ts.readLine().toFloat();
         new_model[i].chain = ts.readLine().toInt();
         new_model[i].serial = ts.readLine().toUInt();
         new_model[i].exposed_code = ts.readLine().toInt();
         new_model[i].bead_color = ts.readLine().toUInt();
         i++;
      }
      // only update if read matches
      if( ts.atEnd() && i == model->size() )
      {
         for ( i = 0; i < model->size(); i++ )
         {
            (*model)[i].name = new_model[i].name;
            (*model)[i].resName = new_model[i].resName;
            (*model)[i].chainID = new_model[i].chainID;
            (*model)[i].resSeq = new_model[i].resSeq;
            (*model)[i].iCode = new_model[i].iCode;
            (*model)[i].bead_ref_mw = new_model[i].bead_ref_mw;
            (*model)[i].bead_ref_ionized_mw_delta = new_model[i].bead_ref_ionized_mw_delta;
            (*model)[i].bead_ref_volume = new_model[i].bead_ref_volume;
            (*model)[i].bead_recheck_asa = new_model[i].bead_recheck_asa;
            (*model)[i].chain = new_model[i].chain;
            (*model)[i].serial = new_model[i].serial;
            (*model)[i].exposed_code = new_model[i].exposed_code;
            (*model)[i].bead_color = new_model[i].bead_color;
         }
         result = true;
         editor->append("Correspondence file ok\n");
         QTextStream( stdout ) << "read_corr: Correspondence file ok\n";
      } else {
         editor->append(QString("Correspondence file didn't match %1\n").arg(fname));
         QTextStream( stdout ) << "read_corr: Correspondence file not ok\n";
      }
   }
   f.close();
   return result;
}

void US_Hydrodyn::save_pdb_csv( csv &csv1 )
{

   QString use_dir = 
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "structures";
   QString filename = QFileDialog::getSaveFileName( this , us_tr("Choose a filename to save the pdc") , use_dir , "*.pdc *.pdc" );


   if ( filename.isEmpty() )
   {
      return;
   }

   if ( !filename.contains(QRegExp(".pdc$", Qt::CaseInsensitive )) )
   {
      filename += ".pdc";
   }

   if ( QFile::exists(filename) )
   {
      filename = fileNameCheck(filename);
   }

   QFile f(filename);

   if ( !f.open( QIODevice::WriteOnly ) )
   {
      QMessageBox::warning( this, "UltraScan",
                            QString(us_tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   QTextStream t( &f );

   QString qs;

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(csv1.header[i]);
   }

   t << qs << Qt::endl;

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(csv1.data[i][j]);
      }
      t << qs << Qt::endl;
   }
   f.close();
}

#include "us_hydrodyn_pat.h"

void US_Hydrodyn::write_dati1_supc_bead_model( QString filename,
                                               int size,
                                               void * dt ) {
   filename = "/tmp/" + filename;
   vector < PDB_atom > bm( size );
   PDB_atom tmp;
   tmp.active                    = 1;
   tmp.exposed_code              = 1;
   tmp.bead_ref_ionized_mw_delta = 0;
   tmp.bead_recheck_asa          = 0;
   tmp.bead_color                = 1;
   
   struct dati1_supc * udt = (struct dati1_supc *)dt;

   for ( int i = 0; i < size; ++i ) {
      tmp.bead_coordinate.axis[ 0 ] = udt[ i ].x;
      tmp.bead_coordinate.axis[ 1 ] = udt[ i ].y;
      tmp.bead_coordinate.axis[ 2 ] = udt[ i ].z;
      tmp.bead_computed_radius      = udt[ i ].r;
      tmp.bead_ref_mw               = udt[ i ].m;
      bm.push_back( tmp );
   }

   write_bead_model( filename, &bm, US_HYDRODYN_OUTPUT_SOMO );
}

void US_Hydrodyn::write_dati1_pat_bead_model( QString filename,
                                              int size,
                                              void * dtp,
                                              void * dts ) {
   filename = "/tmp/" + filename;
   struct dati1_pat  * udt  = (struct dati1_pat *)dtp;
   struct dati1_supc * udts = (struct dati1_supc *)dts;
   vector < PDB_atom > bm( size );
   PDB_atom tmp;
   tmp.active                    = 1;
   tmp.exposed_code              = 1;
   tmp.bead_ref_ionized_mw_delta = 0;
   tmp.bead_recheck_asa          = 0;
   tmp.bead_color                = 1;
   
   for ( int i = 0; i < size; ++i ) {
      // QTextStream( stdout ) <<
      //    QString()
      //    .sprintf(
      //             "xyz %f %f %f rg %f r %f m %f\n"
      //             ,udt[ i ].x
      //             ,udt[ i ].y
      //             ,udt[ i ].z
      //             ,udt[ i ].rg
      //             ,udt[ i ].r
      //             ,udt[ i ].m
      //             )
      //    ;
                           
                                                 
      tmp.bead_coordinate.axis[ 0 ] = udt[ i ].x;
      tmp.bead_coordinate.axis[ 1 ] = udt[ i ].y;
      tmp.bead_coordinate.axis[ 2 ] = udt[ i ].z;
      tmp.bead_computed_radius      = udts[ i ].r;
      tmp.bead_ref_mw               = udts[ i ].m;
      bm.push_back( tmp );
   }

   write_bead_model( filename, &bm, US_HYDRODYN_OUTPUT_SOMO );
}

bool US_Hydrodyn::write_pdb_from_model(
                                       const PDB_model & model
                                       ,QString & errors
                                       ,QString & writtenname
                                       ,const QString & headernote
                                       ,const QString & suffix
                                       ,const QString & filename
                                       ) {
   QString fname = filename.isEmpty() ? pdb_file : filename;
   fname = fname.replace( QRegExp( "(|-(h|H))\\.(pdb|PDB)$" ), "" );
   fname += suffix + ".pdb";

   if ( !overwrite && QFile::exists( fname ) )
   {
      fname = fileNameCheck( fname, 0, this );
   }

   QFile f( fname );
   if ( !f.open( QIODevice::WriteOnly ) )
   {
      errors = QString( us_tr("can not open file %1 for writing" ) ).arg( fname );
      return false;
   }

   writtenname = fname;

   QString pdb_header =
      QString( "HEADER  US-SOMO %1 File %2\n" ).arg( headernote ).arg( QFileInfo( fname ).fileName() );

   map < QString, bool > chains_used;

   QString pdb_text = "";

   for (unsigned int j = 0; j < (unsigned int) model.molecule.size (); j++) {
      for (unsigned int k = 0; k < (unsigned int) model.molecule[j].atom.size (); k++) {
         PDB_atom *this_atom = (PDB_atom *)&(model.molecule[j].atom[k]);

         pdb_text +=
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
      }
   }

   QTextStream ts( &f );
   ts << pdb_header
      << pdb_text
      << "TER\n"
      << "END\n"
      ;
   f.close();

   return true;
}   
   
