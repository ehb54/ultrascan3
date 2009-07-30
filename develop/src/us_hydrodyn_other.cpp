// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// (this) us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include <qregexp.h>
#include <qfont.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#   define __open open
#else
#   define chdir _chdir
#   define dup2 _dup2
#   define __open _open
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#   define STDOUT_FILENO 1
#   define STDERR_FILENO 2
#endif

#define TOLERANCE overlap_tolerance

void US_Hydrodyn::read_residue_file()
{
   QString str1, str2;
   unsigned int numatoms, numbeads, i, j, positioner;
   QFile f(residue_filename);
   int error_count = 0;
   int line_count = 1;
   QString error_msg = tr("Residue file errors:\n");
   QString error_text = tr("Residue file errors:\n");
   cout << "residue file name: " << residue_filename << endl;
   residue_list.clear();
   multi_residue_map.clear();
   new_residues.clear();
   map < QString, int > dup_residue_map;
   i=1;
   if (f.open(IO_ReadOnly|IO_Translate))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         new_residue.comment = ts.readLine();
         line_count++;
         ts >> new_residue.name;
         ts >> new_residue.type;
         ts >> new_residue.molvol;
         ts >> new_residue.asa;
         ts >> numatoms;
         ts >> numbeads;
         ts >> new_residue.vbar;
         ts.readLine(); // read rest of line
         line_count++;
         new_residue.r_atom.clear();
         new_residue.r_bead.clear();
         for (j=0; j<numatoms; j++)
         {
            ts >> new_atom.name;
            ts >> new_atom.hybrid.name;
            ts >> new_atom.hybrid.mw;
            ts >> new_atom.hybrid.radius;
            ts >> new_atom.bead_assignment;
            if (new_atom.bead_assignment >= numbeads)
            {
               error_count++;
               QString tmp_msg =
                  tr(QString(
                             "\nThe atom's bead assignment has exceeded the number of beads.\n"
                             "For residue: %1 and Atom: %2 "
                             "on line %3 of the residue file.\n")
                     .arg(new_residue.comment)
                     .arg(new_atom.name)
                     .arg(line_count)
                     );
               error_text += tmp_msg;
               if (error_count < 5)
               {
                  error_msg += tmp_msg;
               }
               else
               {
                  if (error_count == 5)
                  {
                     error_msg += "Further errors not listed\n";
                  }
               }
            }
            ts >> positioner;
            if(positioner == 0)
            {
               new_atom.positioner = false;
            }
            else
            {
               new_atom.positioner = true;
            }
            ts >> new_atom.serial_number;
            str2 = ts.readLine(); // read rest of line
            line_count++;
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               new_residue.r_atom.push_back(new_atom);
            }
            else
            {
               QMessageBox::warning(this, tr("UltraScan Warning"),
                                    tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
                                       + new_atom.name + " cannot be read and will be deleted from List."),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
         }
         for (j=0; j<numbeads; j++)
         {
            ts >> new_bead.hydration;
            ts >> new_bead.color;
            ts >> new_bead.placing_method;
            ts >> new_bead.chain;
            ts >> new_bead.volume;
            str2 = ts.readLine(); // read rest of line
            line_count++;
            printf("residue name %s loading bead %d placing method %d\n",
                   new_residue.name.ascii(),
                   j, new_bead.placing_method); fflush(stdout);
            new_residue.r_bead.push_back(new_bead);
         }
         calc_bead_mw(&new_residue);
         if ( !new_residue.name.isEmpty()
              && new_residue.molvol > 0.0
              && new_residue.asa > 0.0)
         {
            new_residue.unique_name = QString("%1%2")
               .arg(new_residue.name)
               .arg(dup_residue_map[new_residue.name] ?
                    QString("_%1").arg(dup_residue_map[new_residue.name]) : "");
            dup_residue_map[new_residue.name]++;
            printf("residue name %s unique name %s\n"
                   ,new_residue.name.ascii()
                   ,new_residue.unique_name.ascii()
                   ); fflush(stdout);
            multi_residue_map[new_residue.name].push_back(residue_list.size());
            residue_list.push_back(new_residue);
            for (unsigned int k = 0; k < new_residue.r_atom.size(); k++) {
               QString idx = QString("%1|%2|%3")
                  .arg(new_residue.name)
                  .arg(new_residue.r_atom[k].name)
                  .arg(multi_residue_map[new_residue.name].size() - 1);
               valid_atom_map[idx].push_back(k);
            }
         }
      }
      f.close();
   }
#if defined(DEBUG_MULTI_RESIDUE)
   printf("map index:\n");
   for (map < QString, vector < int > >::iterator it = multi_residue_map.begin();
        it != multi_residue_map.end();
        it++)
   {
      for (unsigned int i = 0; i < it->second.size(); i++)
      {
         printf("residue %s map pos %u\n",
                it->first.ascii(), it->second[i]);
      }
   }
#endif
   if (error_count)
   {
      QMessageBox::message(tr("ERRORS:"), error_msg);
      if (editor)
      {
         editor->append(error_text);
      }
   }
   save_residue_list = residue_list;
   save_multi_residue_map = multi_residue_map;
}

void US_Hydrodyn::calc_vbar(struct PDB_model *model)
{
   float mw_vbar_sum = 0.0;
   float mw_sum = 0.0;
   float mw;
   for (unsigned int i=0; i<(*model).residue.size(); i++)
   {
      mw = 0.0;
      for (unsigned int j=0; j<(*model).residue[i].r_atom.size(); j++)
      {
         mw += (*model).residue[i].r_atom[j].hybrid.mw;
      }
      mw_sum += mw;
      mw_vbar_sum += mw * (*model).residue[i].vbar;
   }
   (*model).vbar = (mw_vbar_sum/mw_sum); //  - 0.002125;
   //cout << "VBAR: " << (*model).vbar << endl;
}

void US_Hydrodyn::calc_bead_mw(struct residue *res)
{
   for (unsigned int i=0; i<(*res).r_bead.size(); i++)
   {
      (*res).r_bead[i].mw = 0.0;
      for (unsigned int j=0; j<(*res).r_atom.size(); j++)
      {
         if ((*res).r_atom[j].bead_assignment == i)
         {
            (*res).r_bead[i].mw += (*res).r_atom[j].hybrid.mw;
         }
      }
   }
}

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
   (*temp_chain).atom.clear();
   (*temp_chain).chainID = "";
   (*temp_chain).segID = "";
}

bool US_Hydrodyn::assign_atom(const QString &str1, struct PDB_chain *temp_chain, QString *last_resSeq)
{
   /*
     http://www.rcsb.org/pdb/docs/format/pdbguide2.2/part_11.html

     ATOM record:
     COLUMNS        DATA TYPE       FIELD         DEFINITION
     ---------------------------------------------------------------------------------
     1 -  6        Record name     "ATOM  "
     7 - 11        Integer         serial        Atom serial number.
     13 - 16        Atom            name          Atom name. (sometimes starts at 12)
     17             Character       altLoc        Alternate location indicator.
     18 - 20        Residue name    resName       Residue name.
     22             Character       chainID       Chain identifier.
     23 - 26        Integer         resSeq        Residue sequence number.
     27             AChar           iCode         Code for insertion of residues.
     31 - 38        Real(8.3)       x             Orthogonal coordinates for X in Angstroms.
     39 - 46        Real(8.3)       y             Orthogonal coordinates for Y in Angstroms.
     47 - 54        Real(8.3)       z             Orthogonal coordinates for Z in Angstroms.
     55 - 60        Real(6.2)       occupancy     Occupancy.
     61 - 66        Real(6.2)       tempFactor    Temperature factor.
     73 - 76        LString(4)      segID         Segment identifier, left-justified.
     77 - 78        LString(2)      element       Element symbol, right-justified.
     79 - 80        LString(2)      charge        Charge on the atom.
   */
   QString str2;
   bool flag;
   struct PDB_atom temp_atom;
   str2 = str1.mid(6, 5);
   temp_atom.serial = str2.toUInt();

   str2 = str1.mid(11, 5);
   temp_atom.name = str2.stripWhiteSpace();

   temp_atom.altLoc = str1.mid(16, 1);

   str2 = str1.mid(17, 3);
   temp_atom.resName = str2.stripWhiteSpace();

   temp_atom.chainID = str1.mid(21, 1);

   temp_atom.resSeq = str1.mid(22, 5);
   temp_atom.resSeq.replace(QRegExp(" *"),"");
   if (temp_atom.resSeq == *last_resSeq)
   {
      flag = false;
   }
   else
   {
      flag = true;
   }
   if (temp_atom.resName == "HOH" || // we dont want to add waters to the sequence
       temp_atom.resName == "DOD")
   {
      flag = false;
   }
   *last_resSeq = temp_atom.resSeq; //update last residue sequence number
   temp_atom.iCode = str1.mid(26, 1);

   str2 = str1.mid(30, 8);
   temp_atom.coordinate.axis[0] = str2.toFloat();
   str2 = str1.mid(38, 8);
   temp_atom.coordinate.axis[1] = str2.toFloat();
   str2 = str1.mid(46, 8);
   temp_atom.coordinate.axis[2] = str2.toFloat();

   str2 = str1.mid(54, 6);
   temp_atom.occupancy = str2.toFloat();

   str2 = str1.mid(60, 6);
   temp_atom.tempFactor = str2.toFloat();

   temp_atom.element = str1.mid(76, 2);
   if (str1.length() >= 80)
   {
      temp_atom.charge = str1.mid(78, 2);
   }
   else
   {
      temp_atom.charge = "  ";
   }
   (*temp_chain).atom.push_back(temp_atom);
   bool found = false;
   for (unsigned int m = 0; m < residue_list.size(); m++)
   {
      if (temp_atom.resName == residue_list[m].name)
      {
         current_residue = residue_list[m];
         found = true;
         break;
      }
   }
   if (!found)
   {
      //   printError(tr("The residue " + temp_atom.resName + " listed in this PDB file is not found in the residue table!"));
   }

   return(flag);
}

void US_Hydrodyn::read_pdb(const QString &filename)
{
   lb_model->clear();
   QString str, str1, str2, temp;
   model_vector.clear();
   bead_model.clear();
   QString last_resSeq = ""; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
   struct PDB_chain temp_chain;
   QFile f(filename);
   struct PDB_model temp_model;
   bool chain_flag = false;
   bool model_flag = false;
   temp_model.molecule.clear();
   temp_model.residue.clear();
   clear_temp_chain(&temp_chain);
   if (f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         str1 = ts.readLine();
         if ( str1.left(6) == "HEADER" ||
              str1.left(5) == "TITLE" )
         {
            QString tmp_str = str1.mid(10,62);
            tmp_str.replace(QRegExp("\\s+")," ");
            QColor save_color = editor->color();
            editor->setColor("dark green");
            editor->append(QString("PDB %1: %2").arg(str1.left(6)).arg(tmp_str));
            editor->setColor(save_color);
         }
         if (str1.left(5) == "MODEL") // we have a new model in a multi-model file
         {
            model_flag = true; // we are using model descriptions (possibly multiple models)
            str2 = str1.mid(6, 15);
            temp_model.model_id = str2.toUInt();
            chain_flag = false; // we are starting a new molecule
            temp_model.molecule.clear();
            temp_model.residue.clear();
            clear_temp_chain(&temp_chain);
         }
         if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
         {
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            editor->append("\nResidue sequence from " + project +".pdb model " +
                           QString("%1").arg(model_vector.size() + 1) + ": \n");
            str = "";
            // the residue list is wrong if there are unknown residues
            for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
            {
               QString lastResSeq = "";
               for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
               {
                  if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
                  {
                     str += temp_model.molecule[i].atom[j].resName + " ";
                     lastResSeq = temp_model.molecule[i].atom[j].resSeq;
                  }
               }
            }
            // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
            // {
            //   str += temp_model.residue[m].name + " ";
            // }
            editor->append(str);
            // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
            calc_vbar(&temp_model); // update the calculated vbar for this model
            model_vector.push_back(temp_model); // save the model in the model vector.
            clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
         }
         if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
         {
            if(str1.mid(12,1) != "H" && str1.mid(13,1) != "H" &&
               str1.mid(17,3) != "HOH")
            {
               if (str1.mid(16,1) == " " || str1.mid(16,1) == "A")
               {
                  if (str1.mid(16,1) == "A")
                  {
                     editor->append(QString("Atom %1 conformation A selected").arg(str1.mid(6,5)));
                  }
                  if (!chain_flag)    // at the first time we encounter the word ATOM
                  {             // we don't have a chain yet, so let's start a new one
                     temp_chain.chainID = str1.mid(21, 1);
                     str2 = str1.mid(72, 4);
                     temp_chain.segID = str2.stripWhiteSpace();
                     chain_flag = true;
                  }
                  else // we have a chain, let's make sure the chain is still the same
                  {
                     if(temp_chain.chainID != str1.mid(21, 1)) // then we just started a new chain
                     {
                        temp_model.molecule.push_back(temp_chain);
                        clear_temp_chain(&temp_chain);
                        temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                        str2 = str1.mid(72, 4);
                        temp_chain.segID = str2.stripWhiteSpace();
                     }
                  }
                  if (assign_atom(str1, &temp_chain, &last_resSeq)) // parse the current line and add it to temp_chain
                  { // if true, we have new residue and need to add it to the residue vector
                     temp_model.residue.push_back(current_residue); // add the next residue of this model
                  }
               }
            }
         }
      }
      f.close();
   }
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      temp_model.molecule.push_back(temp_chain);
      editor->append("\nResidue sequence from " + project +".pdb:\n");
      str = "";
      // the residue list is wrong if there are unknown residues
      for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
      {
         QString lastResSeq = "";
         for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
         {
            if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
            {
               str += temp_model.molecule[i].atom[j].resName + " ";
               lastResSeq = temp_model.molecule[i].atom[j].resSeq;
            }
         }
      }
      // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
      // {
      //   str += temp_model.residue[m].name + " ";
      // }
      editor->append(str);
      temp_model.model_id = 1;
      // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
      calc_vbar(&temp_model); // update the calculated vbar for this model
      model_vector.push_back(temp_model);
      clear_temp_chain(&temp_chain);
   }
   for (unsigned int i=0; i<model_vector.size(); i++)
   {
      str1.sprintf("Model: %d", model_vector[i].model_id);
      lb_model->insertItem(str1);
   }
   lb_model->setEnabled(true);
   lb_model->setSelected(0, true);
   current_model = 0;
}

int US_Hydrodyn::read_bead_model(QString filename)
{
   lb_model->clear();
   lbl_pdb_file->setText(tr(" not selected "));
   project = filename;
   //   project.replace(QRegExp(".*(/|\\\\)"), "");
   //   project.replace(QRegExp("\\.(somo|SOMO)\\.(bead_model|BEAD_MODEL)$"), "");
   project = QFileInfo(QFileInfo(filename).fileName()).baseName();
   QString ftype = QFileInfo(filename).extension(false).lower();
   editor->setText("\n\nLoading bead model " + project + " of type " + ftype + "\n");
   bead_model.clear();
   PDB_atom tmp_atom;
   QFile f(filename);
   int bead_count;
   int linepos = 0;

   if (ftype == "bead_model")
   {
      if (f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd()) {
            ts >> bead_count;
         }
         else
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (!ts.atEnd()) {
            ts >> results.vbar;
         }
         else
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         editor->append(QString("Beads %1\n").arg(bead_count));
         while (!ts.atEnd() && linepos < bead_count)
         {
            ++linepos;
            for (unsigned int i = 0; i < 3; i++)
            {
               if (!ts.atEnd()) {
                  ts >>  tmp_atom.bead_coordinate.axis[i];
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_computed_radius;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_mw;
               tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_color;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               tmp_atom.serial = linepos;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.residue_list;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.bead_recheck_asa;
            }
            else
            {
               editor->append(QString("\nError in line %1!\n").arg(linepos));
               return linepos;
            }
            tmp_atom.exposed_code = 1;
            tmp_atom.all_beads.clear();
            tmp_atom.active = true;
            tmp_atom.name = "ATOM";
            tmp_atom.resName = "RESIDUE";
            tmp_atom.iCode = "ICODE";
            tmp_atom.chainID = "CHAIN";
            bead_model.push_back(tmp_atom);
         }
         QFont save_font = editor->currentFont();
         QFont new_font = QFont("Courier");
         new_font.setStretch(75);
         editor->setCurrentFont(new_font);
         while (!ts.atEnd())
         {
            editor->append(ts.readLine());
         }
         editor->setCurrentFont(save_font);
         editor->append(QString("\nvbar: %1\n\n").arg(results.vbar));
         f.close();
         if (bead_count != (int)bead_model.size())
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
         //          DOTSOMO, &bead_model, true);
         lb_model->clear();
         lb_model->insertItem("Model 1 from bead_model file");
         lb_model->setSelected(0, true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         somo_processed.resize(lb_model->numRows());
         bead_models.resize(lb_model->numRows());
         current_model = 0;
         bead_models[0] = bead_model;
         somo_processed[0] = 1;
         return 0;
      }
   }

   if (ftype == "beams")
   {
      if (f.open(IO_ReadOnly))
      {
         QTextStream ts(&f);
         if (!ts.atEnd()) {
            ts >> bead_count;
         }
         else
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (!ts.atEnd()) {
            ts >> results.vbar;
         }
         else
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         QString rmcfile;
         if (!ts.atEnd()) {
            ts >> rmcfile;
         }
         else
         {
            editor->append("Error in line 1!\n");
            return 1;
         }
         if (results.vbar == -2)
         {
            if (!ts.atEnd()) {
               ts >> results.vbar;
            }
            else
            {
               editor->append("Error in line 1!\n");
               return 1;
            }
         }
         QFile frmc(QFileInfo(filename).dirPath() + SLASH + rmcfile);
         if (frmc.open(IO_ReadOnly))
         {
            QTextStream tsrmc(&frmc);

            editor->append(QString("Beads %1\n").arg(bead_count));
            while (!tsrmc.atEnd() && !ts.atEnd() && linepos < bead_count)
            {
               ++linepos;
               for (unsigned int i = 0; i < 3; i++)
               {
                  if (!ts.atEnd()) {
                     ts >>  tmp_atom.bead_coordinate.axis[i];
                  }
                  else
                  {
                     editor->append(QString("\nError in line %1!\n").arg(linepos));
                     return linepos;
                  }
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_computed_radius;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_mw;
                  tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_color;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  return linepos;
               }
               tmp_atom.serial = linepos;
               tmp_atom.exposed_code = 1;
               tmp_atom.all_beads.clear();
               tmp_atom.active = true;
               tmp_atom.name = "ATOM";
               tmp_atom.resName = "RESIDUE";
               tmp_atom.iCode = "ICODE";
               tmp_atom.chainID = "CHAIN";
               bead_model.push_back(tmp_atom);
            }
            frmc.close();
         }
         f.close();
         if (bead_count != (int)bead_model.size())
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         f.close();
         editor->append(QString("\nvbar: %1\n\n").arg(results.vbar));
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_prefix.length() ? ("-" + bead_model_prefix) : "") +
         //          DOTSOMO, &bead_model, true);

         lb_model->clear();
         lb_model->insertItem("Model 1 from beams file");
         lb_model->setSelected(0, true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         somo_processed.resize(lb_model->numRows());
         bead_models.resize(lb_model->numRows());
         current_model = 0;
         bead_models[0] = bead_model;
         somo_processed[0] = 1;

         return 0;
      }
   }

   editor->append("File read error\n");
   return -2;
}

void US_Hydrodyn::update_vbar()
{
   if (misc.compute_vbar && model_vector.size() > 0) // after reading the pdb file, the vbar is calculated.
   {// If we computed vbar, we assign this to result.vbar, which should be used in the calculation.
      if (current_model >= model_vector.size())
      {
         current_model = 0;
      }
      results.vbar = model_vector[current_model].vbar;
   }
   else
   {
      results.vbar = misc.vbar;
   }
}


int US_Hydrodyn::read_config(QFile& f)
{
   QString str;
   if ( f.name() == QString::null ||
        f.name().isEmpty() )
   {
      return -1;
   }
   if ( f.handle() == -1 && !f.open(IO_ReadOnly) )
   {
      return -2;
   }

   QTextStream ts(&f);
   if ( ts.readLine() == QString::null ) return -10000; // first line is comment
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10001;
   sidechain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10002;
   sidechain_overlap.fuse_beads = (bool) str.toInt();
   ts >> sidechain_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10003;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10004;
   sidechain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> sidechain_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10005;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10006;
   sidechain_overlap.remove_sync = (bool) str.toInt();
   ts >> sidechain_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10007;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10008;
   sidechain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10009;
   sidechain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10010;
   mainchain_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10011;
   mainchain_overlap.fuse_beads = (bool) str.toInt();
   ts >> mainchain_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10012;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10013;
   mainchain_overlap.remove_hierarch = (bool) str.toInt();
   ts >> mainchain_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10014;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10015;
   mainchain_overlap.remove_sync = (bool) str.toInt();
   ts >> mainchain_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10016;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10017;
   mainchain_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10018;
   mainchain_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10020;
   buried_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10021;
   buried_overlap.fuse_beads = (bool) str.toInt();
   ts >> buried_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10022;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10023;
   buried_overlap.remove_hierarch = (bool) str.toInt();
   ts >> buried_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10024;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10025;
   buried_overlap.remove_sync = (bool) str.toInt();
   ts >> buried_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10026;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10027;
   buried_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10028;
   buried_overlap.show_translate = (bool) str.toInt();


   ts >> str;
   if ( ts.readLine() == QString::null ) return -10030;
   grid_exposed_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10031;
   grid_exposed_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_exposed_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10032;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10033;
   grid_exposed_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_exposed_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10034;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10035;
   grid_exposed_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_exposed_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10036;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10037;
   grid_exposed_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10038;
   grid_exposed_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10040;
   grid_buried_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10041;
   grid_buried_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_buried_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10042;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10043;
   grid_buried_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_buried_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10044;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10045;
   grid_buried_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_buried_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10046;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10047;
   grid_buried_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10048;
   grid_buried_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10050;
   grid_overlap.remove_overlap = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10051;
   grid_overlap.fuse_beads = (bool) str.toInt();
   ts >> grid_overlap.fuse_beads_percent;
   if ( ts.readLine() == QString::null ) return -10052;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10053;
   grid_overlap.remove_hierarch = (bool) str.toInt();
   ts >> grid_overlap.remove_hierarch_percent;
   if ( ts.readLine() == QString::null ) return -10054;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10055;
   grid_overlap.remove_sync = (bool) str.toInt();
   ts >> grid_overlap.remove_sync_percent;
   if ( ts.readLine() == QString::null ) return -10056;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10057;
   grid_overlap.translate_out = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10058;
   grid_overlap.show_translate = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10059;
   overlap_tolerance = str.toDouble();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10060;
   bead_output.output = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10061;
   bead_output.sequence = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10062;
   bead_output.correspondence = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10070;
   asa.probe_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10071;
   asa.probe_recheck_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10072;
   asa.threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10073;
   asa.threshold_percent = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10074;
   asa.grid_threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10075;
   asa.grid_threshold_percent = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10076;
   asa.calculation = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10077;
   asa.recheck_beads = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10078;
   asa.method = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10079;
   asa.asab1_step = str.toFloat();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10080;
   grid.cubic = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10081;
   grid.hydrate = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10082;
   grid.center = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10083;
   grid.tangency = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10084;
   grid.cube_side = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10085;
   grid.enable_asa = (bool) str.toInt();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10090;
   misc.hydrovol = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10091;
   misc.compute_vbar = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10092;
   misc.vbar = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10093;
   misc.avg_radius = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10094;
   misc.avg_mass = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10095;
   misc.avg_hydration = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10096;
   misc.avg_volume = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10097;
   misc.avg_vbar = str.toDouble();

   ts >> str;
   ts.readLine();
   hydro.unit = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.reference_system = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10101;
   hydro.boundary_cond = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10102;
   hydro.volume_correction = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10103;
   hydro.volume = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10104;
   hydro.mass_correction = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10105;
   hydro.mass = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10106;
   hydro.bead_inclusion = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10107;
   hydro.rotational = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10108;
   hydro.viscosity = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10109;
   hydro.overlap_cutoff = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10110;
   hydro.overlap = str.toDouble();
   sidechain_overlap.title = "exposed side chain beads";
   mainchain_overlap.title = "exposed main/main and\nmain/side chain beads";
   buried_overlap.title = "buried beads";
   grid_exposed_overlap.title = "exposed grid beads";
   grid_buried_overlap.title = "buried grid beads";
   grid_overlap.title = "grid beads";

   // pdb_visualization options:

   ts >> str; // visualization option
   if ( ts.readLine() == QString::null ) return -10120;
   pdb_vis.visualization = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10121;
   pdb_vis.filename = str; // custom Rasmol script file

   // pdb_parsing options:

   ts >> str; // skip hydrogens?
   if ( ts.readLine() == QString::null ) return -10130;
   pdb_parse.skip_hydrogen = (bool) str.toInt();
   ts >> str; // skip water molecules?
   if ( ts.readLine() == QString::null ) return -10131;
   pdb_parse.skip_water = (bool) str.toInt();
   ts >> str; // skip alternate conformations?
   if ( ts.readLine() == QString::null ) return -10132;
   pdb_parse.alternate = (bool) str.toInt();
   ts >> str; // find free SH atoms?
   if ( ts.readLine() == QString::null ) return -10133;
   pdb_parse.find_sh = (bool) str.toInt();
   ts >> str; // missing residue choice
   if ( ts.readLine() == QString::null ) return -10134;
   pdb_parse.missing_residues = str.toInt();
   ts >> str; // missing atom choice
   if ( ts.readLine() == QString::null ) return -10135;
   pdb_parse.missing_atoms = str.toInt();

   ts >> str; // wavelength
   if ( ts.readLine() == QString::null ) return -100140;
   saxs_options.wavelength = str.toFloat();
   ts >> str; // start angle
   if ( ts.readLine() == QString::null ) return -100141;
   saxs_options.start_angle = str.toFloat();
   ts >> str; // end angle
   if ( ts.readLine() == QString::null ) return -100142;
   saxs_options.end_angle = str.toFloat();
   ts >> str; // water electron density
   if ( ts.readLine() == QString::null ) return -100143;
   saxs_options.water_e_density = str.toFloat();
   ts >> str; // maximum size
   if ( ts.readLine() == QString::null ) return -100144;
   saxs_options.max_size = str.toFloat();
   ts >> str; // bin size
   if ( ts.readLine() == QString::null ) return -100145;
   saxs_options.bin_size = str.toFloat();
   ts >> str; // hydrate pdb model?
   if ( ts.readLine() == QString::null ) return -100146;
   saxs_options.hydrate_pdb = (bool) str.toInt();
   if ( !ts.atEnd() ) return -10150;

   f.close();
   return 0;
}

int US_Hydrodyn::read_config(const QString& fname)
{
   QFile f;
   QString str;
   if (fname.isEmpty()) // this is true if we don't explicitely set a user-defined file name
   {
      f.setName(USglobal->config_list.root_dir + "/etc/somo.config");
   }
   else
   {
      f.setName(fname);
   }
   if (!f.open(IO_ReadOnly)) // first try user's directory for default settings
   {
      f.setName(USglobal->config_list.system_dir + "/etc/somo.config");
      if (!f.open(IO_ReadOnly)) // read system directory
      {
         reset(); // no file available, reset and return
         return 0;
      }
   }
   return read_config(f);
}


void US_Hydrodyn::write_config(const QString& fname)
{
   QFile f;
   QString str;
   f.setName(fname);
   cout << fname << endl;
   if (f.open(IO_WriteOnly | IO_Translate)) // first try user's directory for default settings
   {
      QTextStream ts(&f);
      ts << "SOMO Config file - computer generated, please do not edit...\n";
      ts << sidechain_overlap.remove_overlap << "\t\t# Remove side chain overlaps flag\n";
      ts << sidechain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << sidechain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << sidechain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << sidechain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << sidechain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << sidechain_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << sidechain_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << sidechain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << mainchain_overlap.remove_overlap << "\t\t# Remove mainchain overlaps flag\n";
      ts << mainchain_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << mainchain_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << mainchain_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << mainchain_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << mainchain_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << mainchain_overlap.remove_sync_percent << "\t\t# percent synchronously step\n";
      ts << mainchain_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << mainchain_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << buried_overlap.remove_overlap << "\t\t# Remove buried beads overlaps flag\n";
      ts << buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << buried_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << grid_exposed_overlap.remove_overlap << "\t\t# Remove exposed grid bead overlaps flag\n";
      ts << grid_exposed_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << grid_exposed_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << grid_exposed_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << grid_exposed_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << grid_exposed_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << grid_exposed_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << grid_exposed_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << grid_exposed_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << grid_buried_overlap.remove_overlap << "\t\t# Remove buried grid bead overlaps flag\n";
      ts << grid_buried_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << grid_buried_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << grid_buried_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << grid_buried_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << grid_buried_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << grid_buried_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << grid_buried_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << grid_buried_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << grid_overlap.remove_overlap << "\t\t# Remove grid bead overlaps flag\n";
      ts << grid_overlap.fuse_beads << "\t\t# Fuse beads flag\n";
      ts << grid_overlap.fuse_beads_percent << "\t\t# Bead fusing threshold (%)\n";
      ts << grid_overlap.remove_hierarch << "\t\t# Remove overlaps hierarchical flag\n";
      ts << grid_overlap.remove_hierarch_percent << "\t\t# Percent hierarchical step\n";
      ts << grid_overlap.remove_sync << "\t\t# Remove overlaps synchronously flag\n";
      ts << grid_overlap.remove_sync_percent << "\t\t# Percent synchronously step\n";
      ts << grid_overlap.translate_out << "\t\t# Outward translation flag\n";
      ts << grid_overlap.show_translate << "\t\t# flag for showing outward translation widget\n";

      ts << overlap_tolerance << "\t\t# bead overlap tolerance\n";

      ts << bead_output.output << "\t\t# flag for selecting output format\n";
      ts << bead_output.sequence << "\t\t# flag for selecting sequence format\n";
      ts << bead_output.correspondence << "\t\t# flag for residue correspondence (BEAMS only)\n";
      ts << asa.probe_radius << "\t\t# probe radius in angstrom\n";
      ts << asa.probe_recheck_radius << "\t\t# probe recheck radius in angstrom\n";
      ts << asa.threshold << "\t\t# SOMO ASA threshold\n";
      ts << asa.threshold_percent << "\t\t# SOMO ASA threshold percent\n";
      ts << asa.grid_threshold << "\t\t# Grid ASA threshold\n";
      ts << asa.grid_threshold_percent << "\t\t# Grid ASA threshold percent\n";
      ts << asa.calculation << "\t\t# flag for calculation of ASA\n";
      ts << asa.recheck_beads << "\t\t# flag for rechecking beads\n";
      ts << asa.method << "\t\t# flag for ASAB1/Surfracer method\n";
      ts << asa.asab1_step << "\t\t# ASAB1 step size\n";

      ts << grid.cubic << "\t\t# flag to apply cubic grid\n";
      ts << grid.hydrate << "\t\t# flag to hydrate original model (grid)\n";
      ts << grid.center << "\t\t# flag for positioning bead in center of mass or cubelet (grid)\n";
      ts << grid.tangency << "\t\t# flag for expanding beads to tangency (grid)\n";
      ts << grid.cube_side << "\t\t# Length of cube side (grid)\n";
      ts << grid.enable_asa << "\t\t# flag for enabling asa options (grid)\n";

      ts << misc.hydrovol << "\t\t# hydration volume\n";
      ts << misc.compute_vbar << "\t\t# flag for selecting vbar calculation\n";
      ts << misc.vbar << "\t\t# vbar value\n";
      ts << misc.avg_radius << "\t\t# Average atomic radius value\n";
      ts << misc.avg_mass << "\t\t# Average atomic mass value\n";
      ts << misc.avg_hydration << "\t\t# Average atomic hydration value\n";
      ts << misc.avg_volume << "\t\t# Average bead/atom volume value\n";
      ts << misc.avg_vbar << "\t\t# Average vbar value\n";

      ts << hydro.unit << "\t\t# exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)\n";
      ts << hydro.reference_system << "\t\t# flag for reference system\n";
      ts << hydro.boundary_cond << "\t\t# flag for boundary condition: false: stick, true: slip\n";
      ts << hydro.volume_correction << "\t\t# flag for volume correction - false: Automatic, true: manual\n";
      ts << hydro.volume << "\t\t# volume correction value for manual entry\n";
      ts << hydro.mass_correction << "\t\t# flag for mass correction: false: Automatic, true: manual\n";
      ts << hydro.mass << "\t\t# mass correction value for manual entry\n";
      ts << hydro.bead_inclusion << "\t\t# flag for bead inclusion in computation - false: exclude hidden beads; true: use all beads\n";
      ts << hydro.rotational << "\t\t# flag false: include beads in volume correction for rotational diffusion, true: exclude\n";
      ts << hydro.viscosity << "\t\t# flag false: include beads in volume correction for intrinsic viscosity, true: exclude\n";
      ts << hydro.overlap_cutoff << "\t\t# flag for overlap cutoff: false: same as in model building, true: enter manually\n";
      ts << hydro.overlap << "\t\t# overlap value\n";

      ts << pdb_vis.visualization << "\t\t# PDB visualization option\n";
      ts << pdb_vis.filename << "\t\t# RasMol color filename\n";

      ts << pdb_parse.skip_hydrogen << "\t\t# skip hydrogen atoms?\n";
      ts << pdb_parse.skip_water << "\t\t# skip water molecules?\n";
      ts << pdb_parse.alternate << "\t\t# skip alternate conformations?\n";
      ts << pdb_parse.find_sh << "\t\t# find SH groups?\n";
      ts << pdb_parse.missing_residues << "\t\t# how to handle missing residues\n";
      ts << pdb_parse.missing_atoms << "\t\t# how to handle missing atoms\n";

      ts << saxs_options.wavelength << "\t\t# scattering wavelength\n";
      ts << saxs_options.start_angle << "\t\t# starting angle\n";
      ts << saxs_options.end_angle << "\t\t# ending angle\n";
      ts << saxs_options.water_e_density << "\t\t# Water electron density\n";
      ts << saxs_options.max_size << "\t\t# maximum size\n";
      ts << saxs_options.bin_size << "\t\t# bin size\n";
      ts << saxs_options.hydrate_pdb << "\t\t# hydrate PDB model? true = yes\n";

      f.close();
   }
}

void US_Hydrodyn::set_default()
{
   QFile f;
   QString str;
   int j;
   // only keep one copy of defaults in system root dir
   f.setName(USglobal->config_list.system_dir + "/etc/somo.defaults");
   bool config_read = false;
   if (f.open(IO_ReadOnly)) // read system directory
   {
      j=read_config(f);
            cout << j << endl;
      if ( j )
      {
         QMessageBox::message(tr("Please note:"),
                              tr("The somo.default configuration file was found to be corrupt.\n"
                                 "Resorting to hard-coded defaults."));
      }
      else
      {
         config_read = true;
      }
   }
   else
   {
      QMessageBox::message(tr("Notice:"),
                           tr("Configuration defaults file ") +
                           f.name() + tr(" not found\nUsing hard-coded defaults."));
   }

   if ( !config_read )
   {
      // hard coded defaults
      sidechain_overlap.remove_overlap = true;
      sidechain_overlap.fuse_beads = true;
      sidechain_overlap.fuse_beads_percent = 70.0;
      sidechain_overlap.remove_hierarch = true;
      sidechain_overlap.remove_hierarch_percent = 1.0;
      sidechain_overlap.remove_sync = false;
      sidechain_overlap.remove_sync_percent = 1.0;
      sidechain_overlap.translate_out = true;
      sidechain_overlap.show_translate = true;

      mainchain_overlap.remove_overlap = true;
      mainchain_overlap.fuse_beads = true;
      mainchain_overlap.fuse_beads_percent = 70.0;
      mainchain_overlap.remove_hierarch = true;
      mainchain_overlap.remove_hierarch_percent = 1.0;
      mainchain_overlap.remove_sync = false;
      mainchain_overlap.remove_sync_percent = 1.0;
      mainchain_overlap.translate_out = false;
      mainchain_overlap.show_translate = false;

      buried_overlap.remove_overlap = true;
      buried_overlap.fuse_beads = false;
      buried_overlap.fuse_beads_percent = 0.0;
      buried_overlap.remove_hierarch = true;
      buried_overlap.remove_hierarch_percent = 1.0;
      buried_overlap.remove_sync = false;
      buried_overlap.remove_sync_percent = 1.0;
      buried_overlap.translate_out = false;
      buried_overlap.show_translate = false;

      grid_exposed_overlap.remove_overlap = true;
      grid_exposed_overlap.fuse_beads = false;
      grid_exposed_overlap.fuse_beads_percent = 0.0;
      grid_exposed_overlap.remove_hierarch = true;
      grid_exposed_overlap.remove_hierarch_percent = 1.0;
      grid_exposed_overlap.remove_sync = false;
      grid_exposed_overlap.remove_sync_percent = 1.0;
      grid_exposed_overlap.translate_out = true;
      grid_exposed_overlap.show_translate = true;

      grid_buried_overlap.remove_overlap = true;
      grid_buried_overlap.fuse_beads = false;
      grid_buried_overlap.fuse_beads_percent = 0.0;
      grid_buried_overlap.remove_hierarch = true;
      grid_buried_overlap.remove_hierarch_percent = 1.0;
      grid_buried_overlap.remove_sync = false;
      grid_buried_overlap.remove_sync_percent = 1.0;
      grid_buried_overlap.translate_out = false;
      grid_buried_overlap.show_translate = false;

      grid_overlap.remove_overlap = true;
      grid_overlap.fuse_beads = false;
      grid_overlap.fuse_beads_percent = 0.0;
      grid_overlap.remove_hierarch = true;
      grid_overlap.remove_hierarch_percent = 1.0;
      grid_overlap.remove_sync = false;
      grid_overlap.remove_sync_percent = 1.0;
      grid_overlap.translate_out = false;
      grid_overlap.show_translate = false;

      overlap_tolerance = 0.001;

      sidechain_overlap.title = "exposed side chain beads";
      mainchain_overlap.title = "exposed main/main and main/side chain beads";
      buried_overlap.title = "buried beads";
      grid_exposed_overlap.title = "exposed grid beads";
      grid_buried_overlap.title = "buried grid beads";
      grid_overlap.title = "grid beads";

      bead_output.sequence = 0;
      bead_output.output = 1;
      bead_output.correspondence = true;

      asa.probe_radius = (float) 1.4;
      asa.probe_recheck_radius = (float) 1.4;
      asa.threshold = 20.0;
      asa.threshold_percent = 50.0;
      asa.grid_threshold = 10.0;
      asa.grid_threshold_percent = 30.0;
      asa.calculation = true;
      asa.recheck_beads = true;
      asa.method = true; // by default use ASAB1
      asa.asab1_step = 1.0;

      grid.cubic = true;       // apply cubic grid
      grid.hydrate = false;    // true: hydrate model
      grid.center = false;    // true: center of cubelet, false: center of mass
      grid.tangency = false;   // true: Expand beads to tangency
      grid.cube_side = 5.0;
      grid.enable_asa = true;   // true: enable asa

      misc.hydrovol = 24.041;
      misc.compute_vbar = true;
      misc.vbar = 0.72;
      misc.avg_radius = 1.68;
      misc.avg_mass = 16.0;
      misc.avg_hydration = 0.4;
      misc.avg_volume = 15.3;
      misc.avg_vbar = 0.72;
      overlap_tolerance = 0.001;

      hydro.unit = -10;                // exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)
      hydro.reference_system = false;   // false: diffusion center, true: cartesian origin (default false)
      hydro.boundary_cond = false;      // false: stick, true: slip (default false)
      hydro.volume_correction = false;   // false: Automatic, true: manual (provide value)
      hydro.volume = 0.0;               // volume correction
      hydro.mass_correction = false;      // false: Automatic, true: manual (provide value)
      hydro.mass = 0.0;                  // mass correction
      hydro.bead_inclusion = false;      // false: exclude hidden beads; true: use all beads
      hydro.rotational = false;         // false: include beads in volume correction for rotational diffusion, true: exclude
      hydro.viscosity = false;            // false: include beads in volume correction for intrinsic viscosity, true: exclude
      hydro.overlap_cutoff = false;      // false: same as in model building, true: enter manually
      hydro.overlap = 0.0;               // overlap

      pdb_vis.visualization = 0;                // default RasMol colors
      pdb_vis.filename = USglobal->config_list.system_dir + "/etc/rasmol.spt"; //default color file

      pdb_parse.skip_hydrogen = true;
      pdb_parse.skip_water = true;
      pdb_parse.alternate = true;
      pdb_parse.find_sh = false;
      pdb_parse.missing_residues = 0;
      pdb_parse.missing_atoms = 0;

      saxs_options.wavelength = 1.5;         // scattering wavelength
      saxs_options.start_angle = 0.1f;       // start angle
      saxs_options.end_angle = 4.0;          // ending angle
      saxs_options.water_e_density = 0.334f; // water electron density in e/A^3
      saxs_options.max_size = 40.0;          // maximum size (A)
      saxs_options.bin_size = 0.4f;          // Bin size (A)
      saxs_options.hydrate_pdb = true;       // Hydrate the PDB model? (true/false)
   }

   default_sidechain_overlap = sidechain_overlap;
   default_mainchain_overlap = mainchain_overlap;
   default_buried_overlap = buried_overlap;
   default_grid_exposed_overlap = grid_exposed_overlap;
   default_grid_buried_overlap = grid_buried_overlap;
   default_grid_overlap = grid_overlap;
   default_bead_output = bead_output;
   default_asa = asa;
   default_misc = misc;
   default_overlap_tolerance = overlap_tolerance;
   default_hydro = hydro;
   default_pdb_vis = pdb_vis;
   default_pdb_parse = pdb_parse;
   default_grid = grid;
   default_saxs_options = saxs_options;
}

void US_Hydrodyn::view_file(const QString &filename)
{
   US_Editor *edit;
   edit = new US_Editor(1);
   edit->setFont(QFont("Courier"));
   edit->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   edit->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   edit->load(filename);
   edit->show();
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
   QMessageBox mb(tr("UltraScan"), tr("Attention:\nAre you sure you want to exit?"),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  QMessageBox::NoButton);
   mb.setButtonText(QMessageBox::Yes, tr("Yes"));
   mb.setButtonText(QMessageBox::Cancel, tr("Cancel"));
   switch(mb.exec())
   {
   case QMessageBox::Cancel:
      {
         return;
      }
   }
   global_Xpos -= 30;
   global_Ypos -= 30;
   write_config(USglobal->config_list.root_dir + "/etc/somo.config");
   // removing the temporary directory requires that the temporary directory is empty.
   // we don't know what else is in the directory if it was created previously.
   // tmp_dir.rmdir(USglobal->config_list.root_dir + "/tmp");
   if (rasmol != NULL)
   {
      if (rasmol->isRunning())
      {
         closeAttnt(rasmol, "RASMOL");
      }
   }
   e->accept();
}


void US_Hydrodyn::printError(const QString &str)
{
   QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
                        tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
}

void US_Hydrodyn::closeAttnt(QProcess *proc, QString message)
{
   switch( QMessageBox::information( this,
                                     tr("Attention!"),   message + tr(" is still running.\n"
                                                                      "Do you want to close it?"),
                                     tr("&Kill"), tr("&Close gracefully"), tr("Leave running"),
                                     0,      // Enter == button 0
                                     2 ) )   // Escape == button 2
   {
   case 0:
      {
         proc->kill();
         break;
      }
   case 1:
      {
         proc->tryTerminate();
         break;
      }
   case 2:
      {
         break;
      }
   }
}

// #define DEBUG_COLOR

int US_Hydrodyn::get_color(PDB_atom *a) {
   int color = a->bead_color;
   if (a->all_beads.size()) {
      color = 7;
   }
   if (a->exposed_code != 1) {
      color = 6;
   }
   if (a->bead_computed_radius <= TOLERANCE) {
      color = 0;
   }
   //  color = a->bead_number % 15;
#if defined DEBUG_COLOR
   color = 0;
   if (a->chain == 1) {
      color = 4;
      if (a->exposed_code != 1) {
         color = 6;
      }
   }
   else
   {
      color = 1;
      if (a->exposed_code != 1) {
         color = 10;
      }
   }
#endif
   return color;
}

void US_Hydrodyn::write_bead_spt(QString fname, vector<PDB_atom> *model) {

   char *colormap[] =
      {
         "black",        // 0 black
         "blue",         // 1 blue
         "greenblue",    // 2 greenblue
         "cyan",         // 3 cyan
         "red",          // 4 red
         "magenta",      // 5 magenta
         "orange",       // 6 orange
         "white",        // 7 white
         "redorange",    // 8 redorange
         "purple",       // 9 purple
         "green",        // 10 green
         "cyan",         // 11 cyan
         "redorange",    // 12 redorange
         "violet",       // 13 violet
         "yellow",       // 14 yellow
      };

#if defined(DEBUG)
   printf("write bead spt %s\n", fname.ascii()); fflush(stdout);
#endif

   FILE *fspt = fopen(QString("%1.spt").arg(fname).ascii(), "w");
   FILE *fbms = fopen(QString("%1.bms").arg(fname).ascii(), "w");

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
   scaling = 5.0;
   fprintf(fbms,
           "%d\n%s\n",
           beads,
           QFileInfo(fname).fileName().ascii()
           );
   fprintf(fspt,
           "load xyz %s\nselect all\nwireframe off\nset background white\n",
           QString("%1.bms").arg(QFileInfo(fname).fileName()).ascii()
           );

   int atomno = 0;
   for (unsigned int i = 0; i < model->size(); i++) {
      if ((*model)[i].active) {
         if ((*model)[i].bead_color >= (sizeof(colormap) / sizeof(char))) {
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
         fprintf(fspt,
                 "select atomno=%d\nspacefill %.2f\ncolour %s\n",
                 atomno++,
                 (*model)[i].bead_computed_radius / scaling,
                 colormap[get_color(&(*model)[i])]
                 );
      }
   }
   fclose(fspt);
   fclose(fbms);
}

void US_Hydrodyn::write_bead_tsv(QString fname, vector<PDB_atom> *model) {
   FILE *f = fopen(fname.ascii(), "w");
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
                 "%d~%f~%s~%s~%d~%u~%u~"
                 "%f~%f~%f~"
                 "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]~%u~%s\n",

                 (*model)[i].name.ascii(),
                 (*model)[i].resName.ascii(),
                 (*model)[i].chainID.ascii(),

                 (*model)[i].coordinate.axis[0],
                 (*model)[i].coordinate.axis[1],
                 (*model)[i].coordinate.axis[2],
                 (*model)[i].active ? "Y" : "N",
                 (*model)[i].radius,
                 (*model)[i].asa,
                 (*model)[i].mw,

                 (*model)[i].bead_number,
                 (*model)[i].chain,
                 (*model)[i].serial,
                 (*model)[i].is_bead ? "Y" : "N",
                 (*model)[i].bead_asa,
                 (*model)[i].visibility ? "Y" : "N",

                 (*model)[i].exposed_code,
                 (*model)[i].bead_mw,
                 (*model)[i].bead_positioner ? "Y" : "N",
                 (*model)[i].active ? ((*model)[i].bead_positioner ? "Y" : "N") : "Inactive",
                 (*model)[i].placing_method,
                 (*model)[i].bead_hydration,
                 (*model)[i].bead_color,

                 (*model)[i].bead_ref_volume,
                 (*model)[i].bead_ref_mw,
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
                 beads_referenced.ascii()
                 );
      }
   }
   fclose(f);
}

void US_Hydrodyn::write_bead_asa(QString fname, vector<PDB_atom> *model) {
   FILE *f = fopen(fname.ascii(), "w");
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
         total_mass += (*model)[i].bead_ref_mw;
         total_vol += (*model)[i].bead_ref_volume_unhydrated;

         QString residue =
            (*model)[i].resName + "_" +
            ((*model)[i].chainID == " " ? "_" : (*model)[i].chainID) +
            QString("_%1").arg((*model)[i].resSeq);
         if (residue != last_residue) {
            if (last_residue != "") {
               fprintf(f,
                       " [ %-6d %s ]\t%.0f\t%.0f\t%.2f\n",
                       seqno, last_residue.ascii(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
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
              seqno, last_residue.ascii(), residue_asa, residue_ref_asa, 100.0 * residue_asa / residue_ref_asa);
   }

   fprintf(f,
           "\n\n\n"
           "\tTOTAL ASA OF THE MOLECULE    = %.0f\t[A^2] (Threshold used: %.1f A^2]\n"
           "\tTOTAL VOLUME OF THE MOLECULE = %-.2f\t[A^3]\n"
           "\tRADIUS OF GYRATION (+r) =  %-.2f   [A]\n"
           "\tRADIUS OF GYRATION (-r) =  %-.2f   [A]\n"
           "\tMASS OF THE MOLECULE    =  %.0f   [Da]\n"
           "\tCENTRE OF MASS          =  %.4f %.4f %.4f [A]\n"
           ,
           total_asa,
           asa.threshold,
           total_vol,
           results.asa_rg_pos,
           results.asa_rg_neg,
           total_mass,
           last_molecular_cog.axis[0], last_molecular_cog.axis[1], last_molecular_cog.axis[2]
           );

   fclose(f);
}



void US_Hydrodyn::write_bead_model(QString fname, vector<PDB_atom> *model) {

   int decpts = -(int)log10(overlap_tolerance/9.9999) + 1;
   if (decpts < 4) {
      decpts = 4;
   }

   QString fstring_somo =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\t%.2f\t%d\t%s\t%.4f\n").
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
   printf("write bead model %s\n", fname.ascii()); fflush(stdout);
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
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 1 &&
             (*model)[i].chain == 1) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 1 &&
             (*model)[i].chain == 0) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 0 &&
             (*model)[i].chain == 1) {
            use_model.push_back(&(*model)[i]);
         }
      }
      for (unsigned int i = 0; i < model->size(); i++) {
         if ((*model)[i].visibility == 0 &&
             (*model)[i].chain == 0) {
            use_model.push_back(&(*model)[i]);
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

   if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
      fsomo = fopen(QString("%1.bead_model").arg(fname).ascii(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
      fbeams = fopen(QString("%1.beams").arg(fname).ascii(), "w");
      frmc = fopen(QString("%1.rmc").arg(fname).ascii(), "w");
      frmc1 = fopen(QString("%1.rmc1").arg(fname).ascii(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
      fhydro = fopen(QString("%1.dat").arg(fname).ascii(), "w");
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
              QFileInfo(fname).fileName().ascii(),
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

   for (unsigned int i = 0; i < use_model.size(); i++) {
      if (use_model[i]->active) {
         QString tmp_serial = use_model[i]->resSeq; // was serial
         QString residues;

         if (!bead_model_from_file) {
            residues =
               use_model[i]->resName +
               (use_model[i]->org_chain ? ".SC." : (regular_N_handling ? ".PB." : ".MC.")) +
               (use_model[i]->chainID == " " ? "" : (use_model[i]->chainID + "."));
            // a compiler error forced this kludge using tmp_serial
            //   + QString("%1").arg((*use_model)[i].serial);
            residues += QString("%1").arg(tmp_serial);

            for (unsigned int j = 0; j < use_model[i]->all_beads.size(); j++)
            {
               QString tmp_serial = use_model[i]->all_beads[j]->resSeq;

               residues += "," +
                  (use_model[i]->all_beads[j]->resName +
                   (use_model[i]->all_beads[j]->org_chain ? ".SC." : (regular_N_handling ? ".PB." : ".MC.")) +
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

         if (fsomo) {
            fprintf(fsomo,
                    fstring_somo.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]),
                    residues.ascii(),
                    use_model[i]->bead_recheck_asa
                    );
         }
         if (fbeams) {
            fprintf(fbeams,
                    fstring_beams.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2]
                    );
            fprintf(frmc,
                    fstring_rmc.ascii(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]));
            fprintf(frmc1,
                    fstring_rmc1.ascii(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color(use_model[i]),
                    residues.ascii()
                    );
         }
         if (fhydro) {
            fprintf(fhydro,
                    fstring_hydro.ascii(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius);
         }
      }
   }
   if (fsomo) {
      fprintf(fsomo,
              "\n"
              "Bead Model Output:\n"
              "  Sequence:                   %s\n"
              "\n"

              ,bead_output.sequence ?
              (bead_output.sequence == 1 ?
               "exposed sidechain -> exposed main chain -> buried" :
               "include bead-original residue correspondence") :
              "as in original PDB file"
              );

      fprintf(fsomo, options_log.ascii());
      fprintf(fsomo, last_abb_msgs.ascii());
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
}

void US_Hydrodyn::append_options_log_somo()
{
   QString s;

   s.sprintf("Bead model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  SOMO ASA Threshold (A^2):   %.1f\n"
             "  SOMO Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.threshold
             ,asa.threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"
             "    Exposed Side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "      Outward Translation:                        %s\n"

             "    Exposed Main and side chain beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "    Buried beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"

             "\n"

             ,overlap_tolerance

             ,sidechain_overlap.fuse_beads ? "On" : "Off"
             ,sidechain_overlap.fuse_beads_percent
             ,sidechain_overlap.remove_overlap ? "On" : "Off"
             ,sidechain_overlap.remove_sync ? "On" : "Off"
             ,sidechain_overlap.remove_sync_percent
             ,sidechain_overlap.remove_hierarch ? "On" : "Off"
             ,sidechain_overlap.remove_hierarch_percent
             ,sidechain_overlap.translate_out ? "On" : "Off"

             ,mainchain_overlap.fuse_beads ? "On" : "Off"
             ,mainchain_overlap.fuse_beads_percent
             ,mainchain_overlap.remove_overlap ? "On" : "Off"
             ,mainchain_overlap.remove_sync ? "On" : "Off"
             ,mainchain_overlap.remove_sync_percent
             ,mainchain_overlap.remove_hierarch ? "On" : "Off"
             ,mainchain_overlap.remove_hierarch_percent

             ,buried_overlap.fuse_beads ? "On" : "Off"
             ,buried_overlap.fuse_beads_percent
             ,buried_overlap.remove_overlap ? "On" : "Off"
             ,buried_overlap.remove_sync ? "On" : "Off"
             ,buried_overlap.remove_sync_percent
             ,buried_overlap.remove_hierarch ? "On" : "Off"
             ,buried_overlap.remove_hierarch_percent

             );
   options_log += s;
}

void US_Hydrodyn::append_options_log_atob()
{
   QString s;

   s.sprintf("Grid model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  Recheck Bead ASA:           %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  Probe Recheck Radius (A):   %.2f\n"
             "  Grid ASA Threshold (A^2):   %.1f\n"
             "  Grid Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa.calculation ? "On" : "Off"
             ,asa.recheck_beads ? "On" : "Off"
             ,asa.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa.probe_radius
             ,asa.probe_recheck_radius
             ,asa.grid_threshold
             ,asa.grid_threshold_percent
             ,asa.asab1_step
             );
   options_log += s;

   s.sprintf(
             "Grid Functions (AtoB):\n"
             "  Computations Relative to:   %s\n"
             "  Cube Side (Angstrom):       %.1f\n"
             "  Apply Cubic Grid:           %s\n"
             "  Hydrate the Original Model: %s\n"
             "  Expand Beads to Tangency:   %s\n"
             "  Enable ASA options:         %s\n"
             "\n"

             "Grid (AtoB) Overlap Reduction:\n"
             "  Bead Overlap Tolerance:     %f\n"

             "    Exposed beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "      Outward Translation:                        %s\n"
             "\n"

             "    Buried beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "\n"

             "    Grid beads:\n"
             "      Fuse Beads:                                 %s\n"
             "      Fuse Beads that overlap by more than:       %.1f\n"
             "      Remove Overlaps:                            %s\n"
             "      Remove Overlaps synchronously:              %s\n"
             "      Synchronous Overlap Reduction Step Size %%:  %.1f\n"
             "      Remove Overlaps hierarchically:             %s\n"
             "      Hierarchical Overlap Reduction Step Size %%: %.1f\n"
             "\n"

             ,grid.center ? "Center of Mass" : "Center of Cubelet"
             ,grid.cube_side
             ,grid.cubic ? "On" : "Off"
             ,grid.hydrate ? "On" : "Off"
             ,grid.tangency ? "On" : "Off"
             ,grid.enable_asa ? "On" : "Off"

             ,overlap_tolerance

             ,grid_exposed_overlap.fuse_beads ? "On" : "Off"
             ,grid_exposed_overlap.fuse_beads_percent
             ,grid_exposed_overlap.remove_overlap ? "On" : "Off"
             ,grid_exposed_overlap.remove_sync ? "On" : "Off"
             ,grid_exposed_overlap.remove_sync_percent
             ,grid_exposed_overlap.remove_hierarch ? "On" : "Off"
             ,grid_exposed_overlap.remove_hierarch_percent
             ,grid_exposed_overlap.translate_out ? "On" : "Off"

             ,grid_buried_overlap.fuse_beads ? "On" : "Off"
             ,grid_buried_overlap.fuse_beads_percent
             ,grid_buried_overlap.remove_overlap ? "On" : "Off"
             ,grid_buried_overlap.remove_sync ? "On" : "Off"
             ,grid_buried_overlap.remove_sync_percent
             ,grid_buried_overlap.remove_hierarch ? "On" : "Off"
             ,grid_buried_overlap.remove_hierarch_percent

             ,grid_overlap.fuse_beads ? "On" : "Off"
             ,grid_overlap.fuse_beads_percent
             ,grid_overlap.remove_overlap ? "On" : "Off"
             ,grid_overlap.remove_sync ? "On" : "Off"
             ,grid_overlap.remove_sync_percent
             ,grid_overlap.remove_hierarch ? "On" : "Off"
             ,grid_overlap.remove_hierarch_percent
             );

   options_log += s;
}

QString US_Hydrodyn::default_differences_load_pdb()
{
   QString str = "";
   // not supported, could overload ==
   // if ( pdb_parse == default_pdb_parse )
   // {
   //   return str;
   // }
   QString base = "PDB Options -> ";
   QString sub = "Parsing -> ";
   if ( pdb_parse.skip_hydrogen != default_pdb_parse.skip_hydrogen )
   {
      str += QString(base + sub + "Skip hydrogen: %1.\n")
         .arg(pdb_parse.skip_hydrogen ? "Selected" : "Not selected");
   }
   if ( pdb_parse.skip_water != default_pdb_parse.skip_water )
   {
      str += QString(base + sub + "Skip water: %1.\n")
         .arg(pdb_parse.skip_water ? "Selected" : "Not selected");
   }
   if ( pdb_parse.alternate != default_pdb_parse.alternate )
   {
      str += QString(base + sub + "Skip alternate conformations: %1.\n")
         .arg(pdb_parse.alternate ? "Selected" : "Not selected");
   }
   if ( pdb_parse.find_sh != default_pdb_parse.find_sh )
   {
      str += QString(base + sub + "Find free SH, change residue coding: %1.\n")
         .arg(pdb_parse.find_sh ? "Selected" : "Not selected");
   }
   if ( pdb_parse.missing_residues != default_pdb_parse.missing_residues )
   {
      QString opt = "Unknown state";
      switch ( pdb_parse.missing_residues )
      {
      case 0 :
         opt = "List them and stop operation";
         break;
      case 1 :
         opt = "List them, skip residue and proceed";
         break;
      case 2 :
         opt = "Use automatic bead builder (approximate method)";
         break;
      }
      str += QString(base + sub + "If non-coded residues are found: %1.\n")
         .arg(opt);
   }
   if ( pdb_parse.missing_atoms != default_pdb_parse.missing_atoms )
   {
      QString opt = "Unknown state";
      switch ( pdb_parse.missing_atoms )
      {
      case 0 :
         opt = "List them and stop operation";
         break;
      case 1 :
         opt = "List them, skip residue and proceed";
         break;
      case 2 :
         opt = "Use automatic bead builder (approximate method)";
         break;
      }
      str += QString(base + sub + "If missing atoms within a residue are found: %1.\n")
         .arg(opt);
   }
   return str;
}

QString US_Hydrodyn::default_differences_somo()
{
   QString str = "";
   QString base = "SOMO Options -> ";
   QString sub = "ASA Calculation -> ";
   if ( asa.calculation != default_asa.calculation )
   {
      str += QString(base + sub + "Perform ASA calculation: %1.\n")
         .arg(asa.calculation ? "On" : "Off");
   }
   if ( asa.recheck_beads != default_asa.recheck_beads )
   {
      str += QString(base + sub + "Recheck bead ASA: %1.\n")
         .arg(asa.recheck_beads ? "On" : "Off");
   }
   if ( asa.method != default_asa.method )
   {
      str += QString(base + sub + "ASA method: %1.\n")
         .arg(asa.method ? "Rolling Sphere" : "Voronoi Tesselation");
   }
   if ( asa.probe_radius != default_asa.probe_radius )
   {
      str += QString(base + sub + "ASA Probe Radius (A): %1\n").arg(asa.probe_radius);
   }
   if ( asa.probe_recheck_radius != default_asa.probe_recheck_radius &&
        asa.recheck_beads )
   {
      str += QString(base + sub + "Probe Recheck Radius (A): %1\n").arg(asa.probe_recheck_radius);
   }
   if ( asa.threshold != default_asa.threshold )
   {
      str += QString(base + sub + "SOMO ASA Threshold (A^2): %1\n").arg(asa.threshold);
   }
   if ( asa.threshold_percent != default_asa.threshold_percent )
   {
      str += QString(base + sub + "SOMO Bead ASA Threshold %: %1\n").arg(asa.threshold_percent);
   }
   if ( asa.grid_threshold != default_asa.grid_threshold )
   {
      str += QString(base + sub + "Grid ASA Threshold (A^2): %1\n").arg(asa.grid_threshold);
   }
   if ( asa.grid_threshold_percent != default_asa.grid_threshold_percent )
   {
      str += QString(base + sub + "Grid Bead ASA Threshold %: %1\n").arg(asa.grid_threshold_percent);
   }
   if ( asa.asab1_step != default_asa.asab1_step &&
        asa.method )
   {
      str += QString(base + sub + "ASAB1 Step Size (A): %1\n").arg(asa.asab1_step);
   }

   sub = "SoMo Overlap Reduction -> ";
   if ( overlap_tolerance != default_overlap_tolerance )
   {
      str += QString(base + sub + "Bead Overlap Tolerance: %1\n").arg(overlap_tolerance);
   }

   QString sub2 = "Exposed Side Chain Beads -> ";
   if ( sidechain_overlap.fuse_beads != default_sidechain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(sidechain_overlap.fuse_beads ? "On" : "Off");
   }
   if ( sidechain_overlap.fuse_beads_percent != default_sidechain_overlap.fuse_beads_percent &&
        sidechain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(sidechain_overlap.fuse_beads_percent);
   }
   if ( sidechain_overlap.remove_overlap != default_sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(sidechain_overlap.remove_overlap ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_sync != default_sidechain_overlap.remove_sync &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(sidechain_overlap.remove_sync ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_sync_percent != default_sidechain_overlap.remove_sync_percent &&
        sidechain_overlap.remove_overlap && sidechain_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(sidechain_overlap.remove_sync_percent);
   }
   if ( sidechain_overlap.remove_hierarch != default_sidechain_overlap.remove_hierarch &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(sidechain_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( sidechain_overlap.remove_hierarch_percent != default_sidechain_overlap.remove_hierarch_percent &&
        sidechain_overlap.remove_overlap && sidechain_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(sidechain_overlap.remove_hierarch_percent);
   }
   if ( sidechain_overlap.translate_out != default_sidechain_overlap.translate_out &&
        sidechain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Outward Translation: %1\n")
         .arg(sidechain_overlap.translate_out ? "On" : "Off");
   }

   sub2 = "Exposed Main and Side Chain Beads -> ";
   if ( mainchain_overlap.fuse_beads != default_mainchain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(mainchain_overlap.fuse_beads ? "On" : "Off");
   }
   if ( mainchain_overlap.fuse_beads_percent != default_mainchain_overlap.fuse_beads_percent &&
        mainchain_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(mainchain_overlap.fuse_beads_percent);
   }
   if ( mainchain_overlap.remove_overlap != default_mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(mainchain_overlap.remove_overlap ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_sync != default_mainchain_overlap.remove_sync &&
        mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(mainchain_overlap.remove_sync ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_sync_percent != default_mainchain_overlap.remove_sync_percent &&
        mainchain_overlap.remove_overlap && mainchain_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(mainchain_overlap.remove_sync_percent);
   }
   if ( mainchain_overlap.remove_hierarch != default_mainchain_overlap.remove_hierarch  &&
        mainchain_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(mainchain_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( mainchain_overlap.remove_hierarch_percent != default_mainchain_overlap.remove_hierarch_percent &&
        mainchain_overlap.remove_overlap && mainchain_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(mainchain_overlap.remove_hierarch_percent);
   }

   sub2 = "Buried Beads -> ";
   if ( buried_overlap.fuse_beads != default_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(buried_overlap.fuse_beads ? "On" : "Off");
   }
   if ( buried_overlap.fuse_beads_percent != default_buried_overlap.fuse_beads_percent &&
        buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(buried_overlap.fuse_beads_percent);
   }
   if ( buried_overlap.remove_overlap != default_buried_overlap.remove_overlap &&
        buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(buried_overlap.remove_overlap ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync != default_buried_overlap.remove_sync && 
        buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(buried_overlap.remove_sync ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync_percent != default_buried_overlap.remove_sync_percent &&
        buried_overlap.fuse_beads && buried_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(buried_overlap.remove_sync_percent);
   }
   if ( buried_overlap.remove_hierarch != default_buried_overlap.remove_hierarch &&
        buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(buried_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( buried_overlap.remove_hierarch_percent != default_buried_overlap.remove_hierarch_percent &&
        buried_overlap.fuse_beads && buried_overlap.remove_hierarch )
   {
      str += QString(base + sub + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(buried_overlap.remove_hierarch_percent);
   }
   return str;
}

QString US_Hydrodyn::default_differences_grid()
{
   QString str = "";

   QString base = "SOMO Options -> ";
   QString sub = "AtoB (Grid) Overlap Reduction -> ";
   QString sub2 = "Exposed Grid Beads -> ";
   if ( grid_exposed_overlap.fuse_beads != default_grid_exposed_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_exposed_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_exposed_overlap.fuse_beads_percent != default_grid_exposed_overlap.fuse_beads_percent &&
        grid_exposed_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_exposed_overlap.fuse_beads_percent);
   }
   if ( grid_exposed_overlap.remove_overlap != default_grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_exposed_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_sync != default_grid_exposed_overlap.remove_sync &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_exposed_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_sync_percent != default_grid_exposed_overlap.remove_sync_percent &&
        grid_exposed_overlap.remove_overlap && grid_exposed_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_exposed_overlap.remove_sync_percent);
   }
   if ( grid_exposed_overlap.remove_hierarch != default_grid_exposed_overlap.remove_hierarch &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_exposed_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_exposed_overlap.remove_hierarch_percent != default_grid_exposed_overlap.remove_hierarch_percent &&
        grid_exposed_overlap.remove_overlap && grid_exposed_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_exposed_overlap.remove_hierarch_percent);
   }
   if ( grid_exposed_overlap.translate_out != default_grid_exposed_overlap.translate_out &&
        grid_exposed_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Outward Translation: %1\n")
         .arg(grid_exposed_overlap.translate_out ? "On" : "Off");
   }

   sub2 = "Buried Grid Beads -> ";
   if ( grid_buried_overlap.fuse_beads != default_grid_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_buried_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_buried_overlap.fuse_beads_percent != default_grid_buried_overlap.fuse_beads_percent &&
        grid_buried_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_buried_overlap.fuse_beads_percent);
   }
   if ( grid_buried_overlap.remove_overlap != default_grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_buried_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_sync != default_grid_buried_overlap.remove_sync &&
        grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_buried_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_sync_percent != default_grid_buried_overlap.remove_sync_percent &&
        grid_buried_overlap.remove_overlap && grid_buried_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_buried_overlap.remove_sync_percent);
   }
   if ( grid_buried_overlap.remove_hierarch != default_grid_buried_overlap.remove_hierarch &&
        grid_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_buried_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_buried_overlap.remove_hierarch_percent != default_grid_buried_overlap.remove_hierarch_percent &&
        grid_buried_overlap.remove_overlap && grid_buried_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_buried_overlap.remove_hierarch_percent);
   }

   sub2 = "Grid Beads -> ";
   if ( grid_overlap.fuse_beads != default_grid_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads: %1\n")
         .arg(grid_overlap.fuse_beads ? "On" : "Off");
   }
   if ( grid_overlap.fuse_beads_percent != default_grid_overlap.fuse_beads_percent &&
        grid_overlap.fuse_beads )
   {
      str += QString(base + sub + sub2 + "Fuse Beads that overlap by more than: %1\n").arg(grid_overlap.fuse_beads_percent);
   }
   if ( grid_overlap.remove_overlap != default_grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(grid_overlap.remove_overlap ? "On" : "Off");
   }
   if ( grid_overlap.remove_sync != default_grid_overlap.remove_sync &&
        grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(grid_overlap.remove_sync ? "On" : "Off");
   }
   if ( grid_overlap.remove_sync_percent != default_grid_overlap.remove_sync_percent &&
        grid_overlap.remove_overlap && grid_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(grid_overlap.remove_sync_percent);
   }
   if ( grid_overlap.remove_hierarch != default_grid_overlap.remove_hierarch &&
        grid_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(grid_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( grid_overlap.remove_hierarch_percent != default_grid_overlap.remove_hierarch_percent &&
        grid_overlap.remove_overlap && grid_overlap.remove_hierarch )
   {
      str += QString(base + sub + sub2 + "Hierarchical Overlap Reduction Step Size %: %1\n").arg(grid_overlap.remove_hierarch_percent);
   }

   base = "Grid Functions (AtoB) -> ";
   if ( grid.center != default_grid.center )
   {
      str += QString(base +  "Computations Relative to: Center of %1\n")
         .arg(grid.center ? "Cublet" : "Mass");
   }
   if ( grid.cube_side != default_grid.cube_side )
   {
      str += QString(base + "Cube Side (Angstrom): %1\n").arg(grid.cube_side);
   }
   if ( grid.cubic != default_grid.cubic )
   {
      str += QString(base + "Apply Cubic Grid: %1\n")
         .arg(grid.cubic ? "On" : "Off");
   }
   if ( grid.hydrate != default_grid.hydrate )
   {
      str += QString(base + "Hydrate the Original Model: %1\n")
         .arg(grid.hydrate ? "On" : "Off");
   }
   if ( grid.tangency != default_grid.tangency )
   {
      str += QString(base + "Expand Beads to Tangency: %1\n")
         .arg(grid.tangency ? "On" : "Off");
   }
   if ( grid.enable_asa != default_grid.enable_asa )
   {
      str += QString(base + "Enable ASA screening: %1\n")
         .arg(grid.enable_asa ? "On" : "Off");
   }
   return str;
}

QString US_Hydrodyn::default_differences_hydro()
{
   QString str = "";
   QString base = "SOMO Options -> Hydrodynamic Calculations -> ";
   if ( hydro.unit != default_hydro.unit )
   {
      str += QString(base + "Model units (-10 = Angstrom, -9 = nanometer): %1\n").arg(hydro.unit);
   }
   if ( hydro.reference_system != default_hydro.reference_system )
   {
      str += QString(base + "Computations Relative to: %1\n")
         .arg(hydro.reference_system ? "Cartesian Origin" : "Diffusion Center");
   }
   if ( hydro.boundary_cond != default_hydro.boundary_cond )
   {
      str += QString(base + "Boundary Conditions: %1\n")
         .arg(hydro.boundary_cond ? "Slip" : "Stick");
   }
   if ( hydro.mass_correction != default_hydro.mass_correction )
   {
      str += QString(base + "Total Mass of Model: %1\n")
         .arg(hydro.mass_correction ? "Manual" : "Automatic");
   }
   if ( hydro.mass != default_hydro.mass &&
        hydro.mass_correction )
   {
      str += QString(base + "Entered Mass: %1\n").arg(hydro.mass);
   }
   if ( hydro.volume_correction != default_hydro.volume_correction )
   {
      str += QString(base + "Total Volume of Model: %1\n")
         .arg(hydro.volume_correction ? "Manual" : "Automatic");
   }
   if ( hydro.volume != default_hydro.volume &&
        hydro.volume_correction )
   {
      str += QString(base + "Entered Volume: %1\n").arg(hydro.volume);
   }
   if ( hydro.bead_inclusion != default_hydro.bead_inclusion )
   {
      str += QString(base + "Inclusion of Buried Beads in Hydrodynamic Calculations: %1\n")
         .arg(hydro.bead_inclusion ? "Include" : "Exclude");
   }
   if ( hydro.rotational != default_hydro.rotational )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Rotational Diffusion: %1\n")
         .arg(hydro.rotational ? "Exclude" : "Include");
   }
   if ( hydro.viscosity != default_hydro.viscosity )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Intrinsic Viscosity: %1\n")
         .arg(hydro.viscosity ? "Exclude" : "Include");
   }
   if ( hydro.overlap_cutoff != default_hydro.overlap_cutoff )
   {
      str += QString(base + "Overlap cut-off: %1\n")
         .arg(hydro.overlap_cutoff ? "Manual" : "From Bead Model");
   }
   if ( hydro.overlap != default_hydro.overlap &&
        hydro.overlap_cutoff )
   {
      str += QString(base + "Entered overlap cutoff: %1\n").arg(hydro.overlap);
   }
   return str;
}

QString US_Hydrodyn::default_differences_misc()
{
   QString str = "";
   QString base = "SOMO Options -> Miscellaneous Options -> ";

   if ( misc.compute_vbar != default_misc.compute_vbar )
   {
      str += QString(base + "Calculate vbar: %1\n")
         .arg(misc.compute_vbar ? "On" : "Off");
   }
   if ( misc.vbar != default_misc.vbar &&
        !misc.compute_vbar )
   {
      str += QString(base + "Entered vbar value: %1\n").arg(misc.vbar);
   }
   if ( misc.hydrovol != default_misc.hydrovol )
   {
      str += QString(base + "Hydration Water Vol. (A^3): %1\n").arg(misc.hydrovol);
   }
   QString sub = " Automatic Bead Builder -> Average ";
   if ( misc.avg_radius != default_misc.avg_radius )
   {
      str += QString(base + sub + "atomic radius (A): %1\n").arg(misc.avg_radius);
   }
   if ( misc.avg_mass != default_misc.avg_mass )
   {
      str += QString(base + sub + "atomic mass (Da): %1\n").arg(misc.avg_mass);
   }
   if ( misc.avg_hydration != default_misc.avg_hydration )
   {
      str += QString(base + sub + "atomic hydration: %1\n").arg(misc.avg_hydration);
   }
   if ( misc.avg_volume != default_misc.avg_volume )
   {
      str += QString(base + sub + "bead/atom volume (A^3): %1\n").arg(misc.avg_volume);
   }
   if ( misc.avg_vbar != default_misc.avg_vbar )
   {
      str += QString(base + sub + "Residue vbar: %1\n").arg(misc.avg_vbar);
   }
   return str;
}

QString US_Hydrodyn::default_differences_saxs_options()
{
   QString str = "";
   QString base = "SOMO Options -> Saxs Options -> ";
   if ( saxs_options.hydrate_pdb != default_saxs_options.hydrate_pdb )
   {
      str += QString(base + "Hydrate Original PDB Model: %1\n")
         .arg(saxs_options.hydrate_pdb ? "On" : "Off");
   }
   if ( saxs_options.wavelength != default_saxs_options.wavelength )
   {
      str += QString(base + "Entered wavelength value: %1\n").arg(saxs_options.wavelength);
   }
   if ( saxs_options.start_angle != default_saxs_options.start_angle )
   {
      str += QString(base + "Start Angle: %1\n").arg(saxs_options.start_angle );
   }
   if ( saxs_options.end_angle != default_saxs_options.end_angle )
   {
      str += QString(base + "Ending Angle: %1\n").arg(saxs_options.end_angle );
   }
   if ( saxs_options.water_e_density != default_saxs_options.water_e_density )
   {
      str += QString(base + "Water electron density value: %1\n").arg(saxs_options.water_e_density );
   }
   if ( saxs_options.max_size != default_saxs_options.max_size )
   {
      str += QString(base + "Maximum size: %1\n").arg(saxs_options.max_size );
   }
   if ( saxs_options.bin_size != default_saxs_options.bin_size )
   {
      str += QString(base + "Bin size: %1\n").arg(saxs_options.bin_size );
   }
   return str;
}

void US_Hydrodyn::display_default_differences()
{
   QString str =
      default_differences_misc() +
      default_differences_load_pdb() +
      default_differences_somo() +
      default_differences_hydro() +
      default_differences_saxs_options() +
      default_differences_grid();

   if ( str != "" )
   {
      QColor save_color = editor->color();
      editor->setColor("dark red");
      editor->append("\nNon-default options:\n" + str );
      editor->setColor(save_color);
   }
   else
   {
      QColor save_color = editor->color();
      editor->setColor("dark green");
      editor->append("\nAll options set to default values\n");
      editor->setColor(save_color);
   }
}
