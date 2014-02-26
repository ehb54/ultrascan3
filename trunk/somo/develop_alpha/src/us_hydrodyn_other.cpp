#include "../include/us3_defines.h"
// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// (this) us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"
#include "../include/us_hydrodyn_supc.h"
#include "../include/us_hydrodyn_pat.h"
#include "../include/us_hydrodyn_asab1.h"
#include "../include/us_hydrodyn_grid_atob.h"
#include "../include/us_math.h"
#include "../include/us_dirhist.h"
#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <Q3TextStream>
#include <QCloseEvent>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <qsound.h>

#undef DEBUG
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
#   define isnan _isnan
#endif

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define TOLERANCE overlap_tolerance

void US_Hydrodyn::read_residue_file()
{
   QString str1, str2;
   unsigned int numatoms, numbeads, /* i, */ j, positioner;
   QFile f(residue_filename);
   int error_count = 0;
   int line_count = 1;
   QString error_msg = tr("Residue file errors:\n");
   QString error_text = tr("Residue file errors:\n");
   if ( advanced_config.debug_1 )
   {
      cout << "residue file name: " << residue_filename << endl;
   }
   residue_list.clear();
   residue_list_no_pbr.clear();
   multi_residue_map.clear();
   residue_atom_hybrid_map.clear();
   new_residues.clear();
   map < QString, int > dup_residue_map;
   map < QString, bool > pbr_override_map; // maps positioner for overwrite
   unknown_residues.clear(); // keep track of unknown residues

   msroll_radii.clear();
   msroll_names.clear();

   // i=1;
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      Q3TextStream ts(&f);
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
         vector < vector < atom > > new_atoms;
         new_atoms.resize(numbeads);
         vector < atom > alt_atoms;
         
         for (j=0; j<numatoms; j++)
         {
            ts >> new_atom.name;
            ts >> new_atom.hybrid.name;
            ts >> new_atom.hybrid.mw;
            ts >> new_atom.hybrid.radius;
            ts >> new_atom.bead_assignment;
            if ( /* misc.export_msroll && */
                 new_residue.name.length() < 4 )
            {
               QString use_residue_name = new_residue.name;
               if ( new_atom.name == "OXT" )
               {
                  use_residue_name = "*";
               }
               
               unsigned int pos = ( unsigned int ) msroll_radii.size() + 1;
               double covalent_radius = new_atom.hybrid.radius / 2e0;
               if ( new_atom.name.contains( QRegExp( "^C" ) ) )
               {
                  covalent_radius = 0.77e0;
               }
               if ( new_atom.name.contains( QRegExp( "^N" ) ) )
               {
                  covalent_radius = 0.70e0;
               }
               if ( new_atom.name.contains( QRegExp( "^P" ) ) )
               {
                  covalent_radius = 0.95e0;
               }
               if ( new_atom.name.contains( QRegExp( "^S" ) ) )
               {
                  covalent_radius = 1.04e0;
               }
               if ( new_atom.name.contains( QRegExp( "^O" ) ) )
               {
                  covalent_radius = new_atom.hybrid.radius / 2.68e0;
               }

               msroll_radii.push_back( QString( "%1 %2 %3 %4\n" )
                                       .arg( pos )
                                       .arg( new_atom.hybrid.radius )
                                       .arg( covalent_radius )
                                       .arg( QString( "%1%2" ).arg( new_residue.name ).arg( new_atom.name ).left( 5 ) )
                                       );
               msroll_names.push_back( QString( "%1 %2 %3 %4\n" )
                                       .arg( use_residue_name )
                                       .arg( new_atom.name )
                                       .arg( pos )
                                       .arg( QString( "%1%2" ).arg( new_residue.name ).arg( new_atom.name ).left( 5 ) )
                                       );
            }

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
            ts >> new_atom.hydration;
            str2 = ts.readLine(); // read rest of line
            line_count++;
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               residue_atom_hybrid_map[
                                       QString("%1|%2")
                                       .arg(new_residue.name).arg(new_atom.name)] 
                  = new_atom.hybrid.name;
               new_residue.r_atom.push_back(new_atom);
               new_atoms[new_atom.bead_assignment].push_back(new_atom);
               if ( new_residue.name.contains(QRegExp("^PBR-")) )
               {
                  pbr_override_map[ QString("%1|%2|%3|%4")
                                    .arg(new_residue.name == "PBR-P" ? "P" : "NP" )
                                    .arg(new_atom.name)
                                    .arg(new_atom.hybrid.name)
                                    .arg(new_atom.bead_assignment) ] = new_atom.positioner;
               }                   
            }
            else
            {
               QMessageBox::warning(this, tr("UltraScan Warning"),
                                    tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
                                       + new_atom.name + " cannot be read and will be deleted from List."),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
         }
         {
            unsigned int serial = 0;
            for ( unsigned int i = 0; i < new_atoms.size(); i++ )
            {
               for ( unsigned int j = 0; j < new_atoms[i].size(); j++ )
               {
                  new_atoms[i][j].serial_number = serial++;
                  alt_atoms.push_back(new_atoms[i][j]);
               }
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
            if ( advanced_config.debug_1 )
            {
               printf("residue name %s loading bead %d placing method %d\n",
                      new_residue.name.ascii(),
                      j, new_bead.placing_method); fflush(stdout);
            }
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
            if ( advanced_config.debug_1 )
            {
               printf("residue name %s unique name %s atom size %u alt size %u pos %u\n"
                      ,new_residue.name.ascii()
                      ,new_residue.unique_name.ascii()
                      ,(unsigned int) new_residue.r_atom.size()
                      ,(unsigned int) alt_atoms.size()
                      ,(unsigned int) residue_list.size()
                      ); fflush(stdout);
            }
            multi_residue_map[new_residue.name].push_back(residue_list.size());
            struct residue alt_residue;
            alt_residue = new_residue;
            alt_residue.r_atom = alt_atoms;
            residue_list_no_pbr.push_back(alt_residue);
            if ( new_residue.type )
            {
               residue_list.push_back(alt_residue); // non-aa's
            } else {
               residue_list.push_back(new_residue);
            }               
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

   // process residue list for adjustments pbr vs non-pbr
   for ( unsigned int i = 0; i < residue_list.size(); i++ )
   {
      // only AA's
      if ( residue_list[i].type == 0 &&
           !residue_list[i].name.contains(QRegExp("^PBR-")) )
      {
         for ( unsigned int j = 0; j < residue_list[i].r_atom.size(); j++ )
         {
            QString arg = 
               QString("%1|%2|%3|%4")
               .arg(residue_list[i].name == "PRO" ? "P" : "NP" )
               .arg(residue_list[i].r_atom[j].name)
               .arg(residue_list[i].r_atom[j].hybrid.name)
               .arg(residue_list[i].r_atom[j].bead_assignment);
            if ( pbr_override_map.count(arg) ) 
            {
               residue_list[i].r_atom[j].positioner = pbr_override_map[arg];
            }
         }
      }
   }
   // save OXT to NPBR-OXT
   if ( multi_residue_map["OXT"].size() == 1 )
   {
      int posOXT = multi_residue_map["OXT"][0];
      int posNPBR_OXT = residue_list.size();
      residue_list.push_back(residue_list[posOXT]);
      residue_list[posNPBR_OXT].name = "NPBR-OXT";
      residue_list_no_pbr.push_back(residue_list[posNPBR_OXT]);
      multi_residue_map["NPBR-OXT"].push_back(posNPBR_OXT);
      if ( residue_list.size() != residue_list_no_pbr.size() )
      {
         printf("Inconsistant residue lists - internal error.\n");
      }
   } // else errors handled below

   // point OXT to PBR-OXT for pbr rule
   if ( multi_residue_map["OXT"].size() == 1 &&
        multi_residue_map["PBR-OXT"].size() == 1 )
   {
      int posOXT = multi_residue_map["OXT"][0];
      int posPBR_OXT = multi_residue_map["PBR-OXT"][0];
      residue_list[posOXT] = residue_list[posPBR_OXT];
      residue_list[posOXT].name = "OXT";
   } 
   else
   {
      error_count++;
      QString msg = "Warning: PBR rule OXT not replaced with OXT-P because there is not exactly 1 OXT and 1 PBR-OXT\n";
      error_text += msg;
      error_msg += msg;
   }

   if (error_count)
   {
      QMessageBox::message(tr("ERRORS:"), error_msg);
      if (editor)
      {
         editor->append(error_text);
      }
   }

   save_residue_list = residue_list;
   save_residue_list_no_pbr = residue_list_no_pbr;
   save_multi_residue_map = multi_residue_map;

   if ( misc.export_msroll )
   {
      QString dir = somo_dir + QDir::separator() + "tmp" + QDir::separator();
         
      QFile f_radii( dir + "msroll_radii.txt" );
      if ( !f_radii.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: can not create MSROLL radii file: %1" ) ).arg( f_radii.name() ) );
      } else {
         Q3TextStream ts( &f_radii );
         for ( unsigned int i = 0; i < (unsigned int) msroll_radii.size(); i++ )
         {
            ts << msroll_radii[ i ];
         }
         f_radii.close();
         editor_msg( "blue", QString( tr( "Notice: created MSROLL radii file: %1" ) ).arg( f_radii.name() ) );
      }

      QFile f_names( dir + "msroll_names.txt" );
      if ( !f_names.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( tr( "Error: can not create MSROLL names file: %1" ) ).arg( f_names.name() ) );
      } else {
         Q3TextStream ts( &f_names );
         for ( unsigned int i = 0; i < (unsigned int) msroll_names.size(); i++ )
         {
            ts << msroll_names[ i ];
         }
         f_names.close();
         editor_msg( "blue", QString( tr( "Notice: created MSROLL names file: %1" ) ).arg( f_names.name() ) );
      }
   }
}

void US_Hydrodyn::calc_vbar(struct PDB_model *model)
{
   float mw_vbar_sum = 0.0;
   float mw_sum = 0.0;
   float mw;
   for (unsigned int i=0; i<(*model).residue.size(); i++)
   {
      if ( model->residue[ i ].name != "WAT" )
      {
         mw = 0.0;
         for (unsigned int j=0; j<(*model).residue[i].r_atom.size(); j++)
         {
            mw += (*model).residue[i].r_atom[j].hybrid.mw;
         }
         mw_sum += mw;
         mw_vbar_sum += mw * (*model).residue[i].vbar;
      }
   }
   (*model).vbar = (mw_vbar_sum/mw_sum); //  - 0.002125;
   cout << "calc VBAR: " << (*model).vbar << endl;
}

void US_Hydrodyn::calc_bead_mw(struct residue *res)
{
   double rmw = 0.0;
   for (unsigned int i=0; i<(*res).r_bead.size(); i++)
   {
      (*res).r_bead[i].mw = 0.0;
      for (unsigned int j=0; j<(*res).r_atom.size(); j++)
      {
         if ((*res).r_atom[j].bead_assignment == i)
         {
            (*res).r_bead[i].mw += (*res).r_atom[j].hybrid.mw;
            rmw += (*res).r_atom[j].hybrid.mw;
            // cout << res->name << " bead " << i << " atom " << res->r_atom[j].name << " mw " << res->r_atom[j].hybrid.mw << endl;
         }
      }
      if ( advanced_config.debug_1 )
      {
         cout << res->name << " bead " << i << " mw " << res->r_bead[i].mw << endl;
      }
   }
   if ( advanced_config.debug_1 )
   {
      cout << res->name << " mw " << rmw << endl;
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
   temp_atom.si = 0e0;
   temp_atom.model_residue_pos = -1;
   str2 = str1.mid(6, 5);
   temp_atom.serial = str2.toUInt();

   str2 = str1.mid(11, 5);
   temp_atom.orgName = str2;
   temp_atom.name = str2.stripWhiteSpace();

   if ( temp_atom.name == "OT1" )
   {
      temp_atom.name = "O";
      temp_atom.orgName.replace( "OT1", "O  " );
   } else {
      if ( temp_atom.name == "OT2" )
      {
         temp_atom.name = "OXT";
         temp_atom.orgName.replace( "OT2", "OXT" );
      }
   }

   temp_atom.altLoc = str1.mid(16, 1);

   str2 = str1.mid(17,3);
   temp_atom.resName = str2.stripWhiteSpace();
   if ( temp_atom.resName == "SWH" )
   {
      temp_atom.resName = "WAT";
   }

   temp_atom.chainID = str1.mid(20, 2).stripWhiteSpace();

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
   temp_atom.saxs_data.saxs_name = "";
   (*temp_chain).atom.push_back(temp_atom);
   bool found = false;
   for (unsigned int m = 0; m < residue_list.size(); m++)
   {
      if (temp_atom.resName == residue_list[m].name)
      {
         // this can be wrong, as there can be multiple entries for the same residue name
         // fixed up with reset_chain_residues()
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

int US_Hydrodyn::read_pdb( const QString &filename )
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
   bool currently_aa_chain = false; // do we have an amino acid chain (pbr)
   bool last_was_ENDMDL = false;    // to fix pdbs with missing MODEL tag

   if ( hydro.unit != -10 )
   {
      QFileInfo fi(bd_last_file);
      switch (
              QMessageBox::question(
                                    this,
                                    tr("Load PDB file"),
                                    QString(tr("The default PDB unit is Angstrom (1e-10).\n"
                                               "Your current setting is 1e%1\n"
                                               "Do you want to reset the unit to Angstrom?")).arg(hydro.unit),
                                    QMessageBox::Yes, 
                                    QMessageBox::No,
                                    QMessageBox::NoButton
                                    ) )
      {
      case QMessageBox::Yes : 
         hydro.unit = -10;
         display_default_differences();
         break;
      case QMessageBox::No : 
      default :
         break;
      }
   }

   QRegExp rx_water_multiplier( "^REMARK Multiply water Iq by (\\d+)", false );
   if ( f.open( QIODevice::ReadOnly ) )
   {
      multiply_iq_by_atomic_volume_last_water_multiplier = 0;      
      last_pdb_header.clear();
      last_pdb_title .clear();
      last_pdb_filename = f.name();
      Q3TextStream ts(&f);
      while ( !ts.atEnd() )
      {
         str1 = ts.readLine();
         if ( saxs_options.multiply_iq_by_atomic_volume &&
              rx_water_multiplier.search( str1 ) != -1 )
         {
            multiply_iq_by_atomic_volume_last_water_multiplier = rx_water_multiplier.cap( 1 ).toUInt();
            cout << QString( "found water multiplier %1 in pdb\n" ).arg( multiply_iq_by_atomic_volume_last_water_multiplier );
         }

         if ( str1.left(3) == "TER" )
         {
            // push back previous chain if it exists
            if ( chain_flag )
            {
               if ( advanced_config.debug_2 )
               {
                  printf("ter break <%s>\n", str1.ascii());
               }
               temp_model.molecule.push_back(temp_chain);
               clear_temp_chain(&temp_chain);
               chain_flag = true;
            }
         }
         if ( str1.left(6) == "HEADER" ||
              str1.left(5) == "TITLE" )
         {
               
            QString tmp_str = str1.mid(10,62);
            tmp_str.replace(QRegExp("\\s+")," ");
            if ( str1.left(5) == "TITLE" )
            {
               last_pdb_title << tmp_str;
            } else {
               last_pdb_header << tmp_str;
            }
            QColor save_color = editor->color();
            editor->setColor("dark green");
            editor->append(QString("PDB %1: %2").arg(str1.left(6)).arg(tmp_str));
            editor->setColor(save_color);
            qApp->processEvents();
         }
         if (str1.left(5) == "MODEL" ||
             (str1.left(4) == "ATOM" && last_was_ENDMDL) ) // we have a new model in a multi-model file
         {
            last_was_ENDMDL = false;
            model_flag = true; // we are using model descriptions (possibly multiple models)
            // str2 = str1.mid(6, 15);
            // temp_model.model_id = str2.toUInt();
            QRegExp rx_get_model( "^MODEL\\s+(\\S+)" );
            if ( rx_get_model.search( str1 ) != -1 )
            {
               temp_model.model_id = rx_get_model.cap( 1 );
            } else {
               temp_model.model_id = str1.mid( 6, 15 );
            }
            chain_flag = false; // we are starting a new molecule
            temp_model.molecule.clear();
            temp_model.residue.clear();
            clear_temp_chain(&temp_chain);
         }
         if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
         {
            last_was_ENDMDL = true;
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            editor->append("\nResidue sequence from " + project +".pdb model " +
                           QString("%1").arg( temp_model.model_id ) + ": \n");
            str = "";
            QString sstr = "";
            int sstr_pos = 0;
            
            // the residue list is wrong if there are unknown residues
            for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
            {
               QString lastResSeq = "";
               for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
               {
                  if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
                  {
                     str += temp_model.molecule[i].atom[j].resName + " ";
                     sstr += 
                        residue_short_names.count(temp_model.molecule[i].atom[j].resName) ? 
                        QString(residue_short_names[temp_model.molecule[i].atom[j].resName]) : "?"; 
                     sstr_pos++;
                     if ( !( sstr_pos % 42 ) )
                     {
                        sstr += "\n";
                     }
                     lastResSeq = temp_model.molecule[i].atom[j].resSeq;
                  }
               }
            }
            // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
            // {
            //   str += temp_model.residue[m].name + " ";
            // }
            editor->append(str);
            {
               QFont save_font = editor->currentFont();
               QFont new_font = QFont("Courier");
               editor->append(tr("\nSequence in one letter code:\n"));
               editor->setCurrentFont(new_font);
               editor->append(sstr + "\n\n");
               editor->setCurrentFont(save_font);
            }
            qApp->processEvents();
            
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
                     bool break_chain = ( temp_chain.chainID != str1.mid(21, 1) );
                     QString thisResName = str1.mid(17,3).stripWhiteSpace();
                     if ( thisResName == "SWH" )
                     {
                        thisResName = "WAT";
                     }
                     bool known_residue = ( multi_residue_map.count(thisResName) );
                     bool this_is_aa =  ( known_residue &&
                                         residue_list[multi_residue_map[thisResName][0]].type == 0 );
                     if ( !break_chain  && 
                          currently_aa_chain &&
                          known_residue &&
                          !this_is_aa )
                     {
                        if ( advanced_config.debug_2 )
                        {
                           printf("chain break forced  !break_chain && currently_aa_chain && known_residue && !this_is_aa\n");
                        }
                        break_chain = true;
                     } 
                     if ( break_chain )
                     {
                        if ( advanced_config.debug_2 )
                        {
                           printf("chain break <%s>\n", str1.ascii());
                        }
                        if ( temp_chain.atom.size() ) 
                        {
                           temp_model.molecule.push_back(temp_chain);
                        }
                        clear_temp_chain(&temp_chain);
                        temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                        str2 = str1.mid(72, 4);
                        temp_chain.segID = str2.stripWhiteSpace();
                     }
                     currently_aa_chain = (!known_residue || this_is_aa);
                  }
                  if (assign_atom(str1, &temp_chain, &last_resSeq)) // parse the current line and add it to temp_chain
                  { // if true, we have new residue and need to add it to the residue vector
                     temp_model.residue.push_back(current_residue); // add the next residue of this model
                  }
                  temp_chain.atom.back().model_residue_pos = temp_model.residue.size() - 1;
               }
            }
         }
      }
      f.close();
   } else {
      lb_model->clear();
      model_vector.clear();
      bead_model.clear();
      QColor save_color = editor->color();
      editor->setColor("red");
      editor->append(QString("Error reading file %1").arg(filename));
      editor->setColor(save_color);
      return -1;
   }
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      temp_model.molecule.push_back(temp_chain);
      editor->append("\nResidue sequence from " + project +".pdb:\n");
      str = "";
      QString sstr = "";
      int sstr_pos = 0;
      // the residue list is wrong if there are unknown residues
      for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
      {
         QString lastResSeq = "";
         for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
         {
            if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
            {
               str += temp_model.molecule[i].atom[j].resName + " ";
               sstr += 
                  residue_short_names.count(temp_model.molecule[i].atom[j].resName) ? 
                  QString(residue_short_names[temp_model.molecule[i].atom[j].resName]) : "?"; 
               sstr_pos++;
               if ( !( sstr_pos % 42 ) )
               {
                  sstr += "\n";
               }
               lastResSeq = temp_model.molecule[i].atom[j].resSeq;
            }
         }
      }
      // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
      // {
      //   str += temp_model.residue[m].name + " ";
      // }
      editor->append(str);
      {
         QFont save_font = editor->currentFont();
         QFont new_font = QFont("Courier");
         editor->append(tr("\nSequence in one letter code:\n"));
         editor->setCurrentFont(new_font);
         editor->append(sstr + "\n\n");
         editor->setCurrentFont(save_font);
      }
      temp_model.model_id = "1";
      // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
      calc_vbar(&temp_model); // update the calculated vbar for this model
      model_vector.push_back(temp_model);
      clear_temp_chain(&temp_chain);
   }
   for (unsigned int i=0; i<model_vector.size(); i++)
   {
      lb_model->insertItem( "Model: " + model_vector[i].model_id );
   }
   lb_model->setEnabled(true);
   lb_model->setSelected(0, true);
   current_model = 0;
   dna_rna_resolve();
   model_vector_as_loaded = model_vector;
   if ( advanced_config.debug_2 )
   {
      list_model_vector(&model_vector_as_loaded);
   }
   // cout << list_chainIDs(model_vector);
   // cout << list_chainIDs(model_vector_as_loaded);
   return 0;
}

void US_Hydrodyn::dna_rna_resolve()
{
   // check each chain of each model for DNA type AA's

   QRegExp rx_dna("^T$");
   QRegExp rx_dna_and_rna("^(A|G|C|T|U)$");
   QRegExp rx_dna_or_rna("^(A|G|C)$");
   QRegExp rx_rna("^U$");

   // this can cause spurious chain breaks in the load
   // work around it with a map

   map < QString, bool > already_asked;
   map < QString, bool > already_messaged;

   for ( unsigned int i = 0; i < model_vector.size(); i++ )
   {
      for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
      {
         bool convert_this = false;
         bool ask_convert = model_vector[i].molecule[j].atom.size() > 0;
         QString chainID;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
         {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            QString thisres = this_atom->resName.stripWhiteSpace();
            chainID = this_atom->chainID;
            if ( rx_dna_and_rna.search(thisres) == -1 )
            {
               // not either:
               ask_convert = false;
               break;
            }
            if ( rx_dna.search(thisres) != -1 )
            {
               // we definitely have DNA, correct this residue!
               ask_convert = false;
               convert_this = true;
               break;
            }
            if ( rx_rna.search(thisres) != -1 )
            {
               // we definitely have RNA, the residue is ok
               ask_convert = false;
               convert_this = false;
               break;
            }
         }
         if ( ask_convert && already_asked.count(QString("%1|%2").arg(i).arg(chainID)) )
         {
            ask_convert = already_asked[QString("%1|%2").arg(i).arg(chainID)];
         }
         if ( already_messaged.count(QString("%1|%2").arg(i).arg(chainID)) )
         {
            ask_convert = false;
            convert_this = true;
         }
         if ( ask_convert )
         {
            switch( QMessageBox::information( this, 
                                              tr("UltraScan"),
                                              tr(QString("Chain %1 Molecule %2 only contains A, G & C residues (so far)\n"
                                                         "Is it DNA or RNA?")
                                                 .arg(chainID)
                                                 .arg(i+1))
                                              ,
                                              "&DNA", 
                                              "&RNA", 
                                              0,
                                              0,      // Enter == button 0
                                              1 ) ) { // Escape == button 2
            case 0: // DNA: convert
               convert_this = true;
               break;
            case 1: // RNA: leave it alone
               break;
            }
            already_asked[QString("%1|%2").arg(i).arg(chainID)] = convert_this;
         }
         if ( convert_this )
         {
            if ( !already_messaged.count(QString("%1|%2").arg(i).arg(chainID)) )
            {
               already_messaged[QString("%1|%2").arg(i).arg(chainID)] = true;
               editor->append(tr(QString("Converting Chain %1 Molecule %2 to standard DNA residue names\n")
                                 .arg(chainID)
                                 .arg(i+1)
                                 ));
            }
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
            {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               this_atom->resName = "D" + this_atom->resName.stripWhiteSpace();
            }
         }
      }
   }
}

int US_Hydrodyn::read_bead_model( QString filename, bool &only_overlap )
{
   last_read_bead_model = filename;
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
   results.asa_rg_pos = -1e0;
   results.asa_rg_neg = -1e0;
   float tmp_mw = 0e0;
   unsigned int model_count = 1;
   vector < QString > model_names;
   model_names.push_back( "1" );

   if ( ftype == "bead_model" )
   {
      if ( f.open( QIODevice::ReadOnly ) )
      {
         Q3TextStream ts(&f);
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
               tmp_mw += tmp_atom.bead_mw;
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
               QString tmp_string;
               // strip extra fields
               tmp_string = ts.readLine();
               tmp_string.replace(QRegExp("^\\s*"),"");
               tmp_string.replace(QRegExp("\\s.*"),"");
               tmp_atom.bead_recheck_asa = tmp_string.toFloat();
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
            tmp_atom.saxs_data.saxs_name = "";
            if ( saxs_options.compute_saxs_coeff_for_bead_models && 
                 saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
            {
               tmp_atom.saxs_name = saxs_options.dummy_saxs_name;
               tmp_atom.saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
               tmp_atom.hydrogens = 0;
            }

            bead_model.push_back(tmp_atom);
         }
         QFont save_font = editor->currentFont();
         QFont new_font = QFont("Courier");
         new_font.setStretch(75);
         editor->setCurrentFont(new_font);
         {
            bool units_loaded = false;
            QRegExp rx(", where x is : (\\d+)");
            QStringList ssaxs;
            QStringList bsaxs;
            QStringList bsaxsv;
            while ( !ts.atEnd() )
            {
               QString str = ts.readLine();
               if ( str.left( 6 ).lower().contains( "__json" ) )
               {
                  continue;
               }
               if ( str.left( 6 ).lower() == "saxs::" )
               {
                  ssaxs << str;
                  continue;
               }

               if ( str.left( 7 ).lower() == "bsaxs::" )
               {
                  bsaxs << str;
                  continue;
               }

               if ( str.left( 8 ).lower() == "bsaxsv::" )
               {
                  bsaxsv << str;
                  continue;
               }

               editor->append(str);
               if ( rx.search(str) != -1 )
               {
                  units_loaded = true;
                  hydro.unit = - rx.cap(1).toInt();
                  display_default_differences();
               }
            }

            editor->setCurrentFont( save_font );

            if ( ssaxs.size() && !saxs_options.iq_global_avg_for_bead_models && bsaxs.size() && ( bead_model.size() % ( bsaxs.size() / 2 ) ) )
            {
               editor_msg( "red", 
                           QString( tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                           .arg( bsaxs.size() )
                           .arg( bead_model.size() )
                           );
            }
               

            if ( ssaxs.size() && ( saxs_options.iq_global_avg_for_bead_models || ( bsaxs.size() && ( bead_model.size() % ( bsaxs.size() / 2 ) ) ) ) )
            {
               editor_msg( "dark blue", 
                           QString( tr( "Found %1 saxs coefficient lines in bead model file\n" ) )
                           .arg( ssaxs.size() ) );
               if ( ssaxs.size() > 2 )
               {
                  editor_msg( "red", 
                              QString( tr( "Error: saxs coefficients found in file, but incorrect # of lines %1 vs 2 expected" ) )
                              .arg( ssaxs.size() ) );
               } else {
                  saxs tmp_saxs;
                  QStringList qsl = QStringList::split( QRegExp( "\\s+" ), ssaxs[ 0 ] );
                  if ( qsl.size() != 12 )
                  {
                     editor_msg( "red", 
                                 QString( tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 12 required" ) )
                                 .arg( qsl.size() ) );
                  } else {
                     // editor_msg( "dark blue",  tr( "Four term saxs coefficients found\n" ) );
                     tmp_saxs.saxs_name = qsl[ 1 ].upper();
                     for ( unsigned int i = 0; i < 4; i++ )
                     {
                        tmp_saxs.a[ i ] = qsl[ 2 + i * 2 ].toFloat();
                        tmp_saxs.b[ i ] = qsl[ 3 + i * 2 ].toFloat();
                        tmp_saxs.a5[ i ] = qsl[ 2 + i * 2 ].toFloat();
                        tmp_saxs.b5[ i ] = qsl[ 3 + i * 2 ].toFloat();
                     }
                     tmp_saxs.a5[ 4 ] =
                        tmp_saxs.b5[ 4 ] = 0.0f;
                     tmp_saxs.c = qsl[ 10 ].toFloat();
                     tmp_saxs.c5 = qsl[ 10 ].toFloat();
                     tmp_saxs.volume = qsl[ 11 ].toFloat();
                     if ( ssaxs.size() == 2 )
                     {
                        qsl = QStringList::split( QRegExp( "\\s+" ), ssaxs[ 1 ] );

                        if ( qsl.size() != 14 )
                        {
                           editor_msg( "red", 
                                       QString( tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 14 required" ) )
                                       .arg( qsl.size() ) );
                        } else {
                           // editor_msg( "dark blue",  tr( "Five term saxs coefficients found\n" ) );
                           for ( unsigned int i = 0; i < 5; i++ )
                           {
                              tmp_saxs.a5[ i ] = qsl[ 2 + i * 2 ].toFloat();
                              tmp_saxs.b5[ i ] = qsl[ 3 + i * 2 ].toFloat();
                           }
                           tmp_saxs.c5 = qsl[ 12 ].toFloat();
                        }
                     }
                     saxs_options.dummy_saxs_name = tmp_saxs.saxs_name;
                     if ( extra_saxs_coefficients.count( tmp_saxs.saxs_name ) )
                     {
                        editor_msg( "dark red", QString( "Notice: extra saxs coefficients %1 replaced\n" ).arg( tmp_saxs.saxs_name ) );
                     } else {
                        saxs_options.dummy_saxs_names.push_back( tmp_saxs.saxs_name );
                     }

                     extra_saxs_coefficients[ tmp_saxs.saxs_name ] = tmp_saxs;

                     if ( saxs_util->saxs_map.count( tmp_saxs.saxs_name ) )
                     {
                        editor_msg( "dark red", 
                                    QString( tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                    .arg( tmp_saxs.saxs_name ) );
                     } else {
                        saxs_util->saxs_list.push_back( tmp_saxs );
                        editor_msg( "dark blue", 
                                    QString( tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                                    .arg( tmp_saxs.saxs_name ) );
                     }
                     saxs_util->saxs_map[ tmp_saxs.saxs_name ] = tmp_saxs;

                     if ( saxs_plot_widget )
                     {
                        if ( saxs_plot_window->saxs_map.count( tmp_saxs.saxs_name ) )
                        {
                           editor_msg( "dark red", 
                                       QString( tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                       .arg( tmp_saxs.saxs_name ) );
                        } else {
                           saxs_plot_window->saxs_list.push_back( tmp_saxs );
                           editor_msg( "dark blue", 
                                       QString( tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
                                       .arg( tmp_saxs.saxs_name ) );
                        }
                        saxs_plot_window->saxs_map[ tmp_saxs.saxs_name ] = tmp_saxs;
                     }

                     if ( saxs_options.compute_saxs_coeff_for_bead_models && 
                          saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                     {
                        if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                        {
                           editor_msg( "red", QString( tr("Warning: No '%1' SAXS atom found. Bead model SAXS disabled.\n" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                        } else {
                           editor_msg( "blue", QString( tr("Notice: Loading dummy atoms with saxs coefficients '%1'" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                        }               
                        for ( unsigned int i = 0; i < ( unsigned int ) bead_model.size(); i++ )
                        {
                           bead_model[ i ].saxs_name = saxs_options.dummy_saxs_name;
                           bead_model[ i ].saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
                        }
                     }
                  }
               }
            }
            if ( bsaxs.size() && !saxs_options.iq_global_avg_for_bead_models )
            {
               editor_msg( "blue", tr( "Notice: using individual bead structure factors\n" ) );
               sf_bead_factors.resize( bsaxs.size() );
               bool do_bsaxsv = false;
               if ( bsaxsv.size() )
               {
                  if ( bsaxsv.size() * 2 == bsaxs.size() )
                  {
                     editor_msg( "blue", tr( "Notice: found correct # of variable length structure factors\n" ) );
                     do_bsaxsv = true;
                  } else {
                     editor_msg( "red", tr( "Notice: found incorrect # of variable length structure factors, variable length not loaded\n" ) );
                  }
               }
                     
               for ( unsigned int j = 0; j < ( unsigned int ) bsaxs.size(); j += 2 )
               {
                  saxs tmp_saxs;
                  QStringList qsl = QStringList::split( QRegExp( "\\s+" ), bsaxs[ j ] );
                  if ( qsl.size() != 12 )
                  {
                     editor_msg( "red", 
                                 QString( tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 12 required" ) )
                                 .arg( qsl.size() ) );
                  } else {
                     // editor_msg( "dark blue",  tr( "Four term saxs coefficients found\n" ) );
                     tmp_saxs.saxs_name = qsl[ 1 ].upper();
                     for ( unsigned int i = 0; i < 4; i++ )
                     {
                        tmp_saxs.a[ i ] = qsl[ 2 + i * 2 ].toFloat();
                        tmp_saxs.b[ i ] = qsl[ 3 + i * 2 ].toFloat();
                        tmp_saxs.a5[ i ] = qsl[ 2 + i * 2 ].toFloat();
                        tmp_saxs.b5[ i ] = qsl[ 3 + i * 2 ].toFloat();
                     }
                     tmp_saxs.a5[ 4 ] =
                        tmp_saxs.b5[ 4 ] = 0.0f;
                     tmp_saxs.c = qsl[ 10 ].toFloat();
                     tmp_saxs.c5 = qsl[ 10 ].toFloat();
                     tmp_saxs.volume = qsl[ 11 ].toFloat();
                     if ( (unsigned int) bsaxs.size() > j + 1 )
                     {
                        qsl = QStringList::split( QRegExp( "\\s+" ), bsaxs[ j + 1 ] );
                        if ( tmp_saxs.saxs_name != qsl[ 1 ].upper() )
                        {
                           editor_msg( "red", tr( "Error: bead saxs coefficients bead number inconsistancy" ) );
                        } else {
                           if ( qsl.size() != 14 )
                           {
                              editor_msg( "red", 
                                          QString( tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 14 required" ) )
                                          .arg( qsl.size() ) );
                           } else {
                              // editor_msg( "dark blue",  tr( "Five term saxs coefficients found\n" ) );
                              for ( unsigned int i = 0; i < 5; i++ )
                              {
                                 tmp_saxs.a5[ i ] = qsl[ 2 + i * 2 ].toFloat();
                                 tmp_saxs.b5[ i ] = qsl[ 3 + i * 2 ].toFloat();
                              }
                              tmp_saxs.c5 = qsl[ 12 ].toFloat();
                           }
                        } 
                     }
                     bead_model     [ j / 2 ].saxs_name     = tmp_saxs.saxs_name;
                     bead_model     [ j / 2 ].saxs_data     = tmp_saxs;
                     bead_model     [ j / 2 ].saxs_excl_vol = tmp_saxs.volume;
                     tmp_saxs.vcoeff.clear();
                     if ( do_bsaxsv )
                     {
                        qsl = QStringList::split( QRegExp( "\\s+" ), bsaxsv[ j / 2 ] );
                        cout << QString( "loading: bvsaxs qsl size %1\n" ).arg( qsl.size() );
                        for ( int i = 2; i < (int) qsl.size() - 1; i++ )
                        {
                           tmp_saxs.vcoeff.push_back( qsl[ i ].toDouble() );
                        }
                        bead_model     [ j / 2 ].saxs_data     = tmp_saxs;
                     }
                        
                     sf_bead_factors[ j / 2 ]               = tmp_saxs;
                  }
               }
               if ( bead_model.size() > sf_bead_factors.size() )
               {
                  for ( unsigned int i = sf_bead_factors.size(); i < bead_model.size(); i++ )
                  {
                     bead_model[ i ].saxs_data     = sf_bead_factors[ i % sf_bead_factors.size() ];
                     bead_model[ i ].saxs_name     = bead_model[ i ].saxs_data.saxs_name;
                     bead_model[ i ].saxs_excl_vol = bead_model[ i ].saxs_data.volume;
                  }
               }
                     
               editor_msg( "dark blue", tr( "Bead model structure factors saved for reapplication" ) );
            }

            if ( !units_loaded )
            {
               editor_msg("red", 
                          tr("\nNote: The model scale (nm, angstrom, etc.) was not found in this bead model file.\n"
                             "Please make sure the scale is correct before computing hydrodynamic parameters.\n"));
               editor_msg("dark red", 
                          QString("Current model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : %1\n").arg(-hydro.unit));
            }
         }
         editor->setCurrentFont(save_font);
         editor->append(QString("\nvbar: %1\n").arg(results.vbar));
         f.close();
         editor->append(QString("\nMolecular weight: %1 Daltons\n\n").arg(tmp_mw));
         if (bead_count != (int)bead_model.size())
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
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
         bead_models_as_loaded = bead_models;
         editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
         int overlap_check_results = overlap_check( true, true, true,
                                                     hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance);
         only_overlap = true;
         return( misc.hydro_zeno ? 0 : overlap_check_results );
      }
   }

   if (ftype == "beams")
   {
      if (f.open(QIODevice::ReadOnly))
      {
         Q3TextStream ts(&f);
         if (!ts.atEnd()) {
            ts >> bead_count;
         }
         else
         {
            editor->append("Error in line 1!\n");
            f.close();
            return 1;
         }
         if (!ts.atEnd()) {
            ts >> results.vbar;
         }
         else
         {
            editor->append("Error in line 1!\n");
            f.close();
            return 1;
         }
         QString rmcfile;
         if (!ts.atEnd()) {
            ts >> rmcfile;
         }
         else
         {
            editor->append("Error in line 1!\n");
            f.close();
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
               f.close();
               return 1;
            }
         }
         QFile frmc(QFileInfo(filename).dirPath() + SLASH + rmcfile);
         if (frmc.open(QIODevice::ReadOnly))
         {
            Q3TextStream tsrmc(&frmc);

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
                     f.close();
                     frmc.close();
                     return linepos;
                  }
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_computed_radius;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  f.close();
                  frmc.close();
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_mw;
                  tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
                  tmp_mw += tmp_atom.bead_mw;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  f.close();
                  frmc.close();
                  return linepos;
               }
               if (!tsrmc.atEnd()) {
                  tsrmc >>  tmp_atom.bead_color;
               }
               else
               {
                  editor->append(QString("\nError in line %1!\n").arg(linepos));
                  f.close();
                  frmc.close();
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
               tmp_atom.saxs_data.saxs_name = "";
               if ( saxs_options.compute_saxs_coeff_for_bead_models && 
                    saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
               {
                  tmp_atom.saxs_name = saxs_options.dummy_saxs_name;
                  tmp_atom.saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
                  tmp_atom.hydrogens = 0;
               }

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
         //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
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
         bead_models_as_loaded = bead_models;
         editor->append(QString("\nMolecular weight: %1 Daltons\n\n").arg(tmp_mw));
         editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
         int overlap_check_results = overlap_check( true, true, true,
                                                    hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance);
         only_overlap = true;
         return( misc.hydro_zeno ? 0 : overlap_check_results );
      }
   }

   
   if (ftype == "pdb") // DAMMIN;DAMMIF;DAMAVER
   {
      if ( saxs_options.compute_saxs_coeff_for_bead_models )
      {
         if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
         {
            editor_msg( "red", QString( tr("Warning: No '%1' SAXS atom found. Bead model SAXS disabled.\n" ) )
                        .arg( saxs_options.dummy_saxs_name ) );
         } else {
            editor_msg( "blue", QString( tr("Notice: Loading dummy atoms with saxs coefficients '%1'" ) )
                        .arg( saxs_options.dummy_saxs_name ) );
         }               
      }

      QRegExp rx_model( "^MODEL\\s+(\\S+)" );
      model_count = 0;
      model_names.clear();

      unsigned int unit_mult = 1;

      if (f.open(QIODevice::ReadOnly))
      {

         QRegExp rx_psv( "^REMARK\\s+PSV(\\s*:|)\\s+(\\S+)", false );
         QRegExp rx_mw ( "^REMARK\\s+MW(\\s*:|)\\s+(\\S+)", false );
         QRegExp rx_unit( "^REMARK\\s+Units exponent(\\s*:|)\\s+(\\S+)", false );
         QRegExp rx_mult( "^REMARK\\s+Units conversion factor(\\s*:|)\\s+(\\S+)", false );
         double loaded_psv = 0e0;
         double loaded_mw  = 0e0;
         unsigned int loaded_unit = 0;
         bool loaded_mult = false;

         QStringList qsl;
         {
            Q3TextStream ts(&f);
            do {
               QString qs = ts.readLine();
               qsl << qs;
               if ( rx_model.search( qs ) != -1 )
               {
                  model_count++;
                  model_names.push_back( rx_model.cap( 1 ) );
               }
               if ( rx_psv.search( qs ) != -1 )
               {
                  loaded_psv = rx_psv.cap( 2 ).toDouble();
                  editor_msg( "blue", QString( tr( "Found PSV %1 in PDB" ) ).arg( loaded_psv ) );
               }
               if ( rx_mw.search( qs ) != -1 )
               {
                  loaded_mw = rx_mw.cap( 2 ).toDouble();
                  editor_msg( "blue", QString( tr( "Found MW %1 in PDB" ) ).arg( loaded_mw ) );
               }
               if ( rx_unit.search( qs ) != -1 )
               {
                  loaded_unit = rx_unit.cap( 2 ).toUInt();
                  editor_msg( "blue", QString( tr( "Found Units exponent %1 in PDB" ) ).arg( loaded_unit ) );
                  if ( loaded_unit == 10 )
                  {
                     saxs_options.dummy_atom_pdbs_in_nm = true;
                  }
                  if ( loaded_unit == 9 )
                  {
                     saxs_options.dummy_atom_pdbs_in_nm = false;
                  }
               }
               if ( rx_mult.search( qs ) != -1 )
               {
                  loaded_mult = true;
                  unit_mult = rx_mult.cap( 2 ).toUInt();
                  editor_msg( "blue", QString( tr( "Found Units conversion %1 in PDB" ) ).arg( unit_mult ) );
               }
            } while (!ts.atEnd());
         }
         f.close();


         if ( loaded_mult &&
              !loaded_unit &&
              saxs_options.dummy_atom_pdbs_in_nm )
         {
            editor_msg( "blue", tr( "Notice: Unit conversion factor in PDB forces Angstrom units" ) );
            saxs_options.dummy_atom_pdbs_in_nm = false;
         }

         if ( saxs_options.dummy_atom_pdbs_in_nm )
         {
            editor_msg( "blue", tr("Notice: This PDB is being loaded in NM units" ) );
         }

         model_count = model_count ? model_count : 1;
         cout << QString( "models: %1\n" ).arg( model_count );

         if ( !f.open(QIODevice::ReadOnly) )
         {
            editor->append("File read error\n");
            return -2;
         }
         
         // make sure it is a valid file

         bool damaver = (qsl.grep("Number of atoms written").count() > 0 &&
                         !qsl.grep("Atomic radius").count() );
         bool dammin = (qsl.grep("Number of particle atoms").count() > 0);
         bool dammif = (qsl.grep("Dummy atoms in output phase").count() > 0);
         bool damfilt = (qsl.grep("Number of atoms written").count() > 0 &&
                         qsl.grep("Atomic radius").count() );
         bool damfilt5 = (qsl.grep("Filtered number of atoms").count() > 0 &&
                          qsl.grep("Atomic Radius").count() );

         if ( !damaver &&
              !dammin &&
              !dammif &&
              !damfilt &&
              !damfilt5
              )
         {
            editor->append("Error in DAMMIN/DAMMIF file: couldn't find 'Dummy atoms in output phase' or 'Number of particle atoms' of 'Number of atoms written'\n");
            f.close();
            return 1;
         }

         // find last atom number
         QRegExp rx;

         QStringList qsl_atom = qsl.grep(QRegExp("^ATOM "));
         QString last_atom = "";
         if ( !qsl_atom.empty() )
         {
            last_atom = qsl_atom[qsl_atom.count() - 1];
         } else {
            editor->append("No ATOM lines found in DAMMIN/DAMMIF/DAMAVER file\n");
            f.close();
            return 1;
         }

         rx.setPattern("^ATOM\\s*(\\d+)\\s*CA");
         if ( rx.search(last_atom) ) 
         {
            editor->append("Couldn't find a last atom number in the DAMMIN/DAMMIF/DAMAVER file\n");
            f.close();
            return 1;
         }
         
         // determine and compare bead count
         int tmp_bead_count = rx.cap(1).toInt();
         printf("read bead count %d\n", tmp_bead_count);

         // count atom lines
         int atom_line_count = qsl.grep(QRegExp("^ATOM ")).count();
         printf("atom line count %d\n", atom_line_count);

         bead_count = atom_line_count;
         bead_count /= model_count;

         Q3TextStream ts(&f);
         QString tmp;
         if ( dammin || dammif )
         {
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.contains("Dummy atoms in output phase") &&
                      !tmp.contains("Number of particle atoms")
                      );
         }

         float radius = 0.0;

         if ( dammin ) 
         {
            puts("dammin");
            rx.setPattern( "Number of particle atoms \\.*\\s*:\\s*(\\d+)\\s*" );
            if ( rx.search(tmp) == -1 ) 
            {
               editor->append("Error in DAMMIN file: couldn't find number of atoms in 'Number of particle atoms' line\n");
               f.close();
               return 1;
            }
            bead_count = rx.cap(1).toInt();
            // editor->append(QString("DAMMIN model has %1 beads\n").arg(bead_count));
            
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.contains("DAM packing radius") );
            
            if ( ts.atEnd() )
            {
               editor->append("Error in DAMMIN file: couldn't find 'DAM packing radius'\n");
               f.close();
               return 1;
            }
            
            rx.setPattern("DAM packing radius \\.*\\s*:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.search(tmp) == -1 ) 
            {
               editor->append("Error in DAMMIN file: couldn't find radius in 'Dummy atom radius' line\n");
               f.close();
               return 1;
            }
            radius = rx.cap(1).toFloat();
         } 

         if ( dammif ) {
            puts("dammif");
            rx.setPattern( "Dummy atoms in output phase\\s*:\\s*(\\d+)\\s*" );
            if ( rx.search(tmp) == -1 ) 
            {
               editor->append("Error in DAMMIF file: couldn't find number of atoms in 'Dummy atoms in output phase' line\n");
               f.close();
               return 1;
            }
            bead_count = rx.cap(1).toInt();
            // editor->append(QString("DAMMIF model has %1 beads\n").arg(bead_count));
            
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.contains("Dummy atom radius") );
            
            if ( ts.atEnd() )
            {
               editor->append("Error in DAMMIF file: couldn't find 'Dummy atom radius'\n");
               f.close();
               return 1;
            }
            
            rx.setPattern("Dummy atom radius\\s *:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.search(tmp) == -1 ) 
            {
               editor->append("Error in DAMMIF file: couldn't find radius in 'Dummy atom radius' line\n");
               f.close();
               return 1;
            }
            radius = rx.cap(1).toFloat();
         }

         if ( damaver ) {
            puts("damaver");
            
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.contains("DAM packing radius") );
               
            if ( ts.atEnd() )
            {
               editor->append("Error in DAMAVER file: couldn't find 'DAM packing radius'\n");
               return 1;
            }
            
            rx.setPattern("DAM packing radius \\.*\\s*:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.search(tmp) == -1 ) 
            {
               editor->append("Error in DAMAVER file: couldn't find radius in 'DAM packing radius' line\n");
               return 1;
            }
            radius = rx.cap(1).toFloat();
         }

         if ( damfilt || damfilt5 ) {
            puts("damfilt");
            
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.lower().contains("atomic radius") );
               
            if ( ts.atEnd() )
            {
               editor->append("Error in DAMFILT file: couldn't find 'Atomic radius'\n");
               return 1;
            }
            
            rx.setPattern("atomic radius \\.*\\s*:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.search(tmp.lower()) == -1 ) 
            {
               editor->append("Error in DAMFILT file: couldn't find radius in 'Atomic radius' line\n");
               return 1;
            }
            radius = rx.cap(1).toFloat();
         }

         if ( saxs_options.dummy_atom_pdbs_in_nm )
         {
            radius *= 10.0;
         }

         radius *= (float) unit_mult;

         editor->append(QString("DAMMIN/DAMMIF/DAMAVER/DAMFILT model atom radius %1 Angstrom\n").arg(radius));
         
         // enter MW and PSV
         float mw = loaded_mw;
         float psv = loaded_psv;
         bool do_write_bead_model = true;
         bool remember = true;
         bool use_partial = false;
         QString partial = filename;
         // QString msg = QString(tr("\n  DAMMIN/DAMMIF file %1  \n  Enter values for vbar and total molecular weight:  \n"))
         // .arg(filename);
         QString msg = QString(tr(" Enter values for vbar and total molecular weight: "));

         bool found = false;
         if ( dammix_remember_psv.count(filename) )
         {
            psv = dammix_remember_psv[filename];
            mw = dammix_remember_mw[filename];
            found = true;
            do_write_bead_model = false; // we should have already created it if we wanted the first time
         } else {
            if ( !dammix_match_remember_psv.empty() )
            {
               puts("dammix_match_remember not empty");
               for (map < QString, float >::iterator it = dammix_match_remember_psv.begin();
                    it != dammix_match_remember_psv.end();
                    it++)
               {
                  printf("iterator first %s\n", it->first.ascii());
                  printf("iterator second %f\n", it->second);

                  if ( filename.contains(it->first) )
                  {
                     psv = dammix_match_remember_psv[it->first];
                     mw = dammix_match_remember_mw[it->first];
                     do_write_bead_model = dammix_match_remember_write_bead_model[it->first];
                     found = true;
                     break;
                  }
               }
            }
         }

         if ( found ) 
         {
            editor->append(QString("Recalled: psv %1, mw %2 Daltons\n").arg(psv).arg(mw));
         } else {
            US_Hydrodyn_Dammin_Opts *hdo = new US_Hydrodyn_Dammin_Opts(
                                                                       msg,
                                                                       &psv,
                                                                       &mw,
                                                                       &do_write_bead_model,
                                                                       &remember,
                                                                       &use_partial,
                                                                       &partial
                                                                       );
            US_Hydrodyn::fixWinButtons( hdo );
            do {
               hdo->exec();
            } while ( mw <= 0.0 || psv <= 0.0 );
            
            delete hdo;
            
            if ( remember ) 
            {
               dammix_remember_psv[filename] = psv;
               dammix_remember_mw[filename] = mw;
               dammix_remember_mw_source[filename] =
                  "manually entered upon load of dummy atom model";
            }
            if ( use_partial ) 
            {
               dammix_match_remember_psv[partial] = psv;
               dammix_match_remember_mw[partial] = mw;
               dammix_match_remember_write_bead_model[partial] = do_write_bead_model;
            }
         }

         editor->append(QString("Molecular weight %1 Daltons\n").arg(mw));
         results.vbar = psv;
         mw /= bead_count;

         // skip rest of remarks
         // set overlap tolerance
         editor->append("Setting overlap tolerance to .002 for DAMMIN/DAMMIF models\n");
         overlap_tolerance = 0.002;

         editor->append( QString( "%1 Models each with %1 beads\n" )
                         .arg( model_count ) 
                         .arg( bead_count ) 
                         );
         bead_models.resize( model_count );
         bool do_bead_saxs_assign = 
            saxs_options.apply_loaded_sf_repeatedly_to_pdb &&
            saxs_options.compute_saxs_coeff_for_bead_models;
         if ( do_bead_saxs_assign && !sf_bead_factors.size() )
         {
            editor_msg( "red", tr( "Warning: application of preloaded structure factors requested but no factors are loaded" ) );
            do_bead_saxs_assign = false;
         }
         if ( do_bead_saxs_assign && bead_count % sf_bead_factors.size() )
         {
            editor_msg( "red", 
                        QString( tr( "Warning: application of preloaded structure factors requested but the number of factors loaded %1 does not divide the number of beads %2" ) )
                        .arg( sf_bead_factors.size() )
                        .arg( bead_count )
                        );
            do_bead_saxs_assign = false;
         }
         if ( do_bead_saxs_assign )
         {
            editor_msg( "blue", tr( "applying preloaded structure factors" ) );
         }

         while ( (unsigned int )model_names.size() < model_count )
         {
            model_names.push_back( QString( "%1" ).arg( model_names.size() + 1 ) );
         }
         for ( unsigned int i = 0; i < model_count; i++ )
         {
            editor_msg( "gray", QString( tr( "Loading from model %1" ) ).arg( model_names[ i ] ) );
            int beads_loaded = 0;
            bead_model.clear();
            while (!ts.atEnd() && beads_loaded < bead_count)
            {
               ++linepos;
               ++beads_loaded;
               // ATOM     20  CA  ASP A   1      -8.226   5.986 215.196   1.0  20.0 0 2 201    

               QString str;
               if (!ts.atEnd()) {
                  str = ts.readLine();
                  if ( str.mid(0,4) != "ATOM" )
                  {
                     do {
                        str = ts.readLine();
                        ++linepos;
                     } while ( !ts.atEnd() &&
                               str.mid(0,4) != "ATOM" ) ;
                  }
                  if ( ts.atEnd() &&
                       str.mid(0,4) != "ATOM" )
                  {
                     editor->append(QString("\nError in line %1. Expected 'ATOM' before end of file\n")
                                    .arg(linepos)
                                    );
                     return linepos;
                  }
               
               } else {
                  editor->append(QString("\nError in line %1. premature end of file'\n").arg(linepos));
                  return linepos;
               }
            
               tmp_atom.serial = str.mid(8,4).toInt();

               for (unsigned int i = 0; i < 3; i++)
               {
                  tmp_atom.bead_coordinate.axis[i] = str.mid(30 + i * 8, 8).toFloat();
                  if ( saxs_options.dummy_atom_pdbs_in_nm )
                  {
                     tmp_atom.bead_coordinate.axis[i] *= 10.0;
                  }
                  tmp_atom.bead_coordinate.axis[i] *= (float) unit_mult;
               }

               tmp_atom.bead_computed_radius = radius;
               tmp_atom.bead_mw = mw;
               tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
               tmp_atom.bead_color = 8;

               tmp_atom.exposed_code = 1;
               tmp_atom.all_beads.clear();
               tmp_atom.active = true;
               tmp_atom.name = "ATOM";
               tmp_atom.resName = "RESIDUE";
               tmp_atom.iCode = "ICODE";
               tmp_atom.chainID = "CHAIN";
               tmp_atom.saxs_data.saxs_name = "";
               if ( !do_bead_saxs_assign &&
                    saxs_options.compute_saxs_coeff_for_bead_models && 
                    saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
               {
                  tmp_atom.saxs_name = saxs_options.dummy_saxs_name;
                  tmp_atom.saxs_data = saxs_util->saxs_map[ saxs_options.dummy_saxs_name ];
                  tmp_atom.hydrogens = 0;
               }
               if ( do_bead_saxs_assign )
               {
                  tmp_atom.saxs_data     = sf_bead_factors[ bead_model.size() % sf_bead_factors.size() ];
                  tmp_atom.saxs_name     = tmp_atom.saxs_data.saxs_name;
                  tmp_atom.saxs_excl_vol = tmp_atom.saxs_data.volume;
                  tmp_atom.hydrogens = 0;
               }                  

               bead_model.push_back(tmp_atom);
               // cout << QString("bead loaded serial %1\n").arg(tmp_atom.serial);
            }
            bead_models[ i ] = bead_model;
         }
         
         // remove TER line
         if (!ts.atEnd()) {
            ts.readLine();
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
         if ( bead_count != (int)bead_model.size() )
         {
            editor->append(QString("Error: bead count %1 does not match # of beads read from file (%2) \n").arg(bead_count).arg(bead_model.size()));
            return -1;
         }
         bead_model_from_file = true;
         editor->append("Bead model loaded\n\n");
         // write_bead_spt(somo_dir + SLASH + project +
         //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
         //          DOTSOMO, &bead_model, true);
         lb_model->clear();
         model_vector.resize( model_count );
         somo_processed.resize( model_count );
         bead_models.resize( model_count );
         for ( unsigned int i = 0; i < model_count; i++ )
         {
            lb_model->insertItem( QString( "Model %1 (from bead model)" ).arg( model_names[ i ] ) );
            model_vector[ i ].vbar = results.vbar;
            model_vector[ i ].model_id = model_names[ i ];
            model_vector[ i ].mw = mw;
            somo_processed[ i ] = 1;
         }
         lb_model->setSelected(0, true);
         lb_model->setEnabled( model_count > 1 );
         current_model = 0;
         if ( model_count > 1 )
         {
            bead_model = bead_models[ 0 ];
         }
         bead_models[0] = bead_model;
         bead_models_as_loaded = bead_models;
         if ( dammin )
         {
            bead_model_suffix = "dammin";
         }
         if ( dammif )
         {
            bead_model_suffix = "dammif";
         }
         if ( damaver )
         {
            bead_model_suffix = "damaver";
         }
         if ( damfilt )
         {
            bead_model_suffix = "damfilt";
         }

         le_bead_model_suffix->setText(bead_model_suffix);
         if ( do_write_bead_model && model_count == 1 ) 
         {
            if ( !overwrite )
            {
               setSomoGridFile(false);
            }
            write_bead_model( somo_dir + SLASH + project + fix_file_name( QString("_%1").arg( model_name( current_model ) ) ) +
                              QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
                              , &bead_model);
         }
         editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
         int overlap_check_results = 0;
         for ( unsigned int i = 0; i < model_count; i++ )
         {
            editor_msg( "gray", QString( tr( "checking overlap for Model %1" ) ).arg( model_names[ i ] ) );
            bead_model = bead_models[ i ];
            overlap_check_results += overlap_check( true, true, true,
                                                    hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance );
         }
         bead_model = bead_models[ 0 ];
         only_overlap = true;
         return( misc.hydro_zeno ? 0 : overlap_check_results );
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
      results.vbar = 
         misc.vbar - 
         (4.25e-4 * (K0 + misc.vbar_temperature - K20));
   }
}


int US_Hydrodyn::read_config(QFile& f)
{
   QString str;
   cout << "read config\n";
   if ( f.name() == QString::null ||
        f.name().isEmpty() )
   {
      return -1;
   }
   if ( f.handle() == -1 && !f.open(QIODevice::ReadOnly) )
   {
      return -2;
   }

   Q3TextStream ts(&f);

   // this is a really silly way to do this, carried over from legacy code
   // the config file should be free format
   // either XML, JSON or simply param value lines
   // this will provide easy updates and allow easy upgrading without removing previous parameters
   // i.e. via merging
   // similar things should also be done with the somo.residue, hybrid, atom & saxs_atoms files

   str = ts.readLine();

   if ( str == QString::null ) return -10000; // first line is comment
   if ( str.contains( "JSON" ) )
   {
      QString qs;
      while ( !ts.atEnd() )
      {
         qs += ts.readLine() + "\n";
      }
      f.close();
      return load_config_json( qs ) ? 0 : -3;
   }

   hard_coded_defaults();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10001;
   replicate_o_r_method_somo = (bool) str.toInt();

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
   replicate_o_r_method_grid = (bool) str.toInt();

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
   grid.center = str.toInt();
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
   if ( ts.readLine() == QString::null ) return -10092;
   misc.vbar_temperature = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10093;
   misc.pb_rule_on = (bool) str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10094;
   misc.avg_radius = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10095;
   misc.avg_mass = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10096;
   misc.avg_hydration = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10097;
   misc.avg_volume = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10098;
   misc.avg_vbar = str.toDouble();

   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.unit = str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.solvent_name = str;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.solvent_acronym = str.left(5);
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.temperature = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.solvent_viscosity = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -10100;
   hydro.solvent_density = str.toDouble();
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

   if ( ( str = ts.readLine() ) == QString::null ) return -10121;
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

   ts >> str; // water electron density
   if ( ts.readLine() == QString::null ) return -10140;
   saxs_options.water_e_density = str.toFloat();

   ts >> str; // H scattering length (*10^-12 cm)
   if ( ts.readLine() == QString::null ) return -10141;
   saxs_options.h_scat_len = str.toFloat();
   ts >> str; // D scattering length (*10^-12 cm)
   if ( ts.readLine() == QString::null ) return -10142;
   saxs_options.d_scat_len = str.toFloat();
   ts >> str; // H2O scattering length density (*10^-10 cm^2)
   if ( ts.readLine() == QString::null ) return -10143;
   saxs_options.h2o_scat_len_dens = str.toFloat();
   ts >> str; // D2O scattering length density (*10^-10 cm^2)
   if ( ts.readLine() == QString::null ) return -10144;
   saxs_options.d2o_scat_len_dens = str.toFloat();
   ts >> str; // D2O concentration (0 to 1)
   if ( ts.readLine() == QString::null ) return -10145;
   saxs_options.d2o_conc = str.toFloat();
   ts >> str; // Fraction of exchanged peptide H (0 to 1)
   if ( ts.readLine() == QString::null ) return -10146;
   saxs_options.frac_of_exch_pep = str.toFloat();

   ts >> str; // wavelength
   if ( ts.readLine() == QString::null ) return -10148;
   saxs_options.wavelength = str.toFloat();
   ts >> str; // start angle
   if ( ts.readLine() == QString::null ) return -10149;
   saxs_options.start_angle = str.toFloat();
   ts >> str; // end angle
   if ( ts.readLine() == QString::null ) return -10150;
   saxs_options.end_angle = str.toFloat();
   ts >> str; // delta angle
   if ( ts.readLine() == QString::null ) return -10151;
   saxs_options.delta_angle = str.toFloat();
   ts >> str; // maximum size
   if ( ts.readLine() == QString::null ) return -10152;
   saxs_options.max_size = str.toFloat();
   ts >> str; // bin size
   if ( ts.readLine() == QString::null ) return -10153;
   saxs_options.bin_size = str.toFloat();
   ts >> str; // hydrate pdb model?
   if ( ts.readLine() == QString::null ) return -10154;
   saxs_options.hydrate_pdb = (bool) str.toInt();
   ts >> str; // curve
   if ( ts.readLine() == QString::null ) return -10155;
   saxs_options.curve = str.toInt();
   ts >> str; // saxs_sans
   if ( ts.readLine() == QString::null ) return -10156;
   saxs_options.saxs_sans = str.toInt();

   // should be saved
   // saxs_options.guinier_csv = false;
   // saxs_options.guinier_csv_filename = "guinier";
   // saxs_options.qRgmax = 1.3e0;
   // saxs_options.qstart = 1e-7;
   // saxs_options.qend = .5e0;
   // saxs_options.pointsmin = 10;
   // saxs_options.pointsmax = 100;

   // bd_options
   {
      int i = -12000;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.threshold_pb_pb = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.threshold_pb_sc = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.threshold_sc_sc = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.do_rr = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.force_chem = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.bead_size_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.show_pdb = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.run_browflex = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.tprev = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.ttraj = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.deltat = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.npadif = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.nconf = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.inter = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.iorder = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.iseed = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.icdm = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_pb_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_pb_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_pb_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_pb_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_pb_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_pb_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_pb_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_pb_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_pb_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_sc_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_sc_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_sc_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_sc_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_sc_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_chem_sc_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.chem_sc_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_pb_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_pb_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_pb_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_pb_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_pb_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_pb_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_pb_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_pb_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.pb_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.sc_sc_bond_type = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_sc_sc_force_constant = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.sc_sc_force_constant = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_sc_sc_equilibrium_dist = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.sc_sc_equilibrium_dist = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.compute_sc_sc_max_elong = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.sc_sc_max_elong = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      bd_options.nmol = str.toInt();
      i--;
   }

   // anaflex_options
   {
      int i = -13000;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_anaflex = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.nfrec = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.instprofiles = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_12 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_13 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_18 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_20 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_1_24 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_2 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_12 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_13 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_18 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_20 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_2_24 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_5 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_9 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_10 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_14 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_15 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_16 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_4 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_4_1 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_4_6 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_4_7 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_4_8 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_9 = (bool)str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.ntimc = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.tmax = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_5_iii = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_5_jjj = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_10_theta = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_10_refractive_index = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_10_lambda = str.toFloat();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_14_iii = str.toInt();
      i--;
      ts >> str;
      if ( ts.readLine() == QString::null ) return i;
      anaflex_options.run_mode_3_14_jjj = str.toInt();
      i--;
   }

   ts >> str; // batch missing atom handling
   if ( ts.readLine() == QString::null ) return -11000;
   batch.missing_atoms = str.toInt();
   ts >> str; // batch missing residue handling
   if ( ts.readLine() == QString::null ) return -11001;
   batch.missing_residues = str.toInt();
   ts >> str; // batch run somo
   if ( ts.readLine() == QString::null ) return -11002;
   batch.somo = (bool) str.toInt();
   ts >> str; // batch run grid
   if ( ts.readLine() == QString::null ) return -11003;
   batch.grid = (bool) str.toInt();
   ts >> str; // batch run hydro
   if ( ts.readLine() == QString::null ) return -11004;
   batch.hydro = (bool) str.toInt();
   ts >> str; // batch avg hydro
   if ( ts.readLine() == QString::null ) return -11005;
   batch.avg_hydro = (bool) str.toInt();
   ts >> str; // batch avg hydro name
   if ( ts.readLine() == QString::null ) return -11006;
   batch.avg_hydro_name = str;
   ts >> str; // batch height
   if ( ts.readLine() == QString::null ) return -11007;
   batch.height = str.toInt();
   ts >> str; // batch width
   if ( ts.readLine() == QString::null ) return -11008;
   batch.width = str.toInt();

   batch.file.clear();
   ts >> str; // batch file list
   if ( ts.readLine() == QString::null ) return -11010;
   {
      int number_of_files = str.toInt();
      for ( int i = 0; i < number_of_files; i++ )
      {
         if ( (str = ts.readLine() ) == QString::null ) return -(11100 + i);
         batch.file.push_back(str);
      }
   }

   save_params.field.clear();
   ts >> str; // save field list
   if ( ts.readLine() == QString::null ) return -11500;
   {
      int number_of_fields = str.toInt();
      for ( int i = 0; i < number_of_fields; i++ )
      {
         if ( (str = ts.readLine() ) == QString::null ) return -(11501 + i);
         save_params.field.push_back(str);
      }
   }

   if ( ( str = ts.readLine() ) == QString::null ) return -11600;
   path_load_pdb = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11601;
   path_view_pdb = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11602;
   path_load_bead_model = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11603;
   path_view_asa_res = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11604;
   path_view_bead_model = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11605;
   path_open_hydro_res = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11606;
   saxs_options.path_load_saxs_curve = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11607;
   saxs_options.path_load_gnom = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11608;
   saxs_options.path_load_prr = str;

   ts >> str;
   if ( ts.readLine()  == QString::null ) return -11609;
   asa.hydrate_probe_radius = str.toFloat();
   ts >> str;
   if ( ts.readLine()  == QString::null ) return -11610;
   asa.hydrate_threshold = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11611;
   misc.target_e_density       = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11612;
   misc.target_volume          = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11613;
   misc.set_target_on_load_pdb = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11614;
   misc.equalize_radii         = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11615;
   dmd_options.force_chem = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11616;
   dmd_options.pdb_static_pairs = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11617;
   dmd_options.threshold_pb_pb = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11618;
   dmd_options.threshold_pb_sc = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11619;
   dmd_options.threshold_sc_sc = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11620;
   saxs_options.normalize_by_mw = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11621;
   saxs_options.saxs_iq_native_debye = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11622;
   saxs_options.saxs_iq_native_hybrid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11623;
   saxs_options.saxs_iq_native_hybrid2 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11624;
   saxs_options.saxs_iq_native_hybrid3 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11625;
   saxs_options.saxs_iq_native_fast = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11626;
   saxs_options.saxs_iq_native_fast_compute_pr = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11627;
   saxs_options.saxs_iq_foxs = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11628;
   saxs_options.saxs_iq_crysol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11629;
   saxs_options.sans_iq_native_debye = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11630;
   saxs_options.sans_iq_native_hybrid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11631;
   saxs_options.sans_iq_native_hybrid2 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11632;
   saxs_options.sans_iq_native_hybrid3 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11633;
   saxs_options.sans_iq_native_fast = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11634;
   saxs_options.sans_iq_native_fast_compute_pr = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11635;
   saxs_options.sans_iq_cryson = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11636;
   saxs_options.hybrid2_q_points = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11637;
   saxs_options.iq_ask = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11638;
   saxs_options.iq_scale_ask = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11639;
   saxs_options.iq_scale_angstrom = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11640;
   saxs_options.iq_scale_nm = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11641;
   saxs_options.sh_max_harmonics = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11642;
   saxs_options.sh_fibonacci_grid_order = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11643;
   saxs_options.crysol_hydration_shell_contrast = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11644;
   saxs_options.crysol_default_load_difference_intensity = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11645;
   saxs_options.crysol_version_26 = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11646;
   saxs_options.fast_bin_size = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11647;
   saxs_options.fast_modulation = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11648;
   saxs_options.compute_saxs_coeff_for_bead_models = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11649;
   saxs_options.compute_sans_coeff_for_bead_models = (bool)str.toInt();

   if ( ( str = ts.readLine() ) == QString::null ) return -11650;
   saxs_options.default_atom_filename = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11651;
   saxs_options.default_hybrid_filename = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11652;
   saxs_options.default_saxs_filename = str;
   if ( ( str = ts.readLine() ) == QString::null ) return -11653;
   saxs_options.default_rotamer_filename = str;

   ts >> str;
   if ( ts.readLine() == QString::null ) return -11654;
   saxs_options.steric_clash_distance         = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11655;
   saxs_options.steric_clash_recheck_distance = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11656;
   saxs_options.disable_iq_scaling = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11657;
   saxs_options.autocorrelate = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11658;
   saxs_options.hybrid_radius_excl_vol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11659;
   saxs_options.scale_excl_vol = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11660;
   saxs_options.subtract_radius = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11661;
   saxs_options.iqq_scale_minq = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11662;
   saxs_options.iqq_scale_maxq = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11663;
   saxs_options.iqq_scale_nnls = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11664;
   saxs_options.iqq_scale_linear_offset = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11665;
   saxs_options.iqq_scale_chi2_fitting = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11666;
   saxs_options.iqq_expt_data_contains_variances = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11667;
   saxs_options.iqq_ask_target_grid = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11668;
   saxs_options.iqq_scale_play = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11669;
   saxs_options.swh_excl_vol = str.toFloat();
   if ( ( str = ts.readLine() ) == QString::null ) return -11670;
   saxs_options.iqq_default_scaling_target = str;
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11671;
   saxs_options.saxs_iq_hybrid_adaptive = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11672;
   saxs_options.sans_iq_hybrid_adaptive = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11673;
   saxs_options.bead_model_rayleigh   = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11674;
   saxs_options.iqq_log_fitting       = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11675;
   saxs_options.iqq_kratky_fit        = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11676;
   saxs_options.iqq_use_atomic_ff     = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11677;
   saxs_options.iqq_use_saxs_excl_vol = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11678;
   saxs_options.alt_hydration         = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11679;
   saxs_options.xsr_symmop                = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11680;
   saxs_options.xsr_nx                    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11681;
   saxs_options.xsr_ny                    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11682;
   saxs_options.xsr_griddistance          = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11683;
   saxs_options.xsr_ncomponents           = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11684;
   saxs_options.xsr_compactness_weight    = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11685;
   saxs_options.xsr_looseness_weight      = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11686;
   saxs_options.xsr_temperature           = str.toDouble();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11687;
   hydro.zeno_zeno              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11688;
   hydro.zeno_interior          = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11689;
   hydro.zeno_surface           = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11690;
   hydro.zeno_zeno_steps        = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11691;
   hydro.zeno_interior_steps    = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11692;
   hydro.zeno_surface_steps     = str.toUInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11693;
   hydro.zeno_surface_thickness = str.toFloat();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11694;
   misc.hydro_supc              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11695;
   misc.hydro_zeno              = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11696;
   batch.saxs_search = (bool)str.toInt();
   ts >> str;
   if ( ts.readLine() == QString::null ) return -11697;
   batch.zeno        = (bool)str.toInt();

   if ( !ts.atEnd() ) return -20000;

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
   if (!f.open(QIODevice::ReadOnly)) // first try user's directory for default settings
   {
      if ( !guiFlag )
      {
         return -1;
      }
      f.setName(USglobal->config_list.system_dir + "/etc/somo.config");
      if (!f.open(QIODevice::ReadOnly)) // read system directory
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
   f.setName( fname );
   cout << fname << endl;
   if ( f.open( QIODevice::WriteOnly ) ) // first try user's directory for default settings
   {
      Q3TextStream ts(&f);
      
      map < QString, QString > parameters;

      ts << "US-SOMO JSON Config file\n";
      
      parameters[ "replicate_o_r_method_somo" ] = QString( "%1" ).arg( replicate_o_r_method_somo );
      parameters[ "sidechain_overlap.remove_overlap" ] = QString( "%1" ).arg( sidechain_overlap.remove_overlap );
      parameters[ "sidechain_overlap.fuse_beads" ] = QString( "%1" ).arg( sidechain_overlap.fuse_beads );
      parameters[ "sidechain_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( sidechain_overlap.fuse_beads_percent );
      parameters[ "sidechain_overlap.remove_hierarch" ] = QString( "%1" ).arg( sidechain_overlap.remove_hierarch );
      parameters[ "sidechain_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( sidechain_overlap.remove_hierarch_percent );
      parameters[ "sidechain_overlap.remove_sync" ] = QString( "%1" ).arg( sidechain_overlap.remove_sync );
      parameters[ "sidechain_overlap.remove_sync_percent" ] = QString( "%1" ).arg( sidechain_overlap.remove_sync_percent );
      parameters[ "sidechain_overlap.translate_out" ] = QString( "%1" ).arg( sidechain_overlap.translate_out );
      parameters[ "sidechain_overlap.show_translate" ] = QString( "%1" ).arg( sidechain_overlap.show_translate );
      parameters[ "mainchain_overlap.remove_overlap" ] = QString( "%1" ).arg( mainchain_overlap.remove_overlap );
      parameters[ "mainchain_overlap.fuse_beads" ] = QString( "%1" ).arg( mainchain_overlap.fuse_beads );
      parameters[ "mainchain_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( mainchain_overlap.fuse_beads_percent );
      parameters[ "mainchain_overlap.remove_hierarch" ] = QString( "%1" ).arg( mainchain_overlap.remove_hierarch );
      parameters[ "mainchain_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( mainchain_overlap.remove_hierarch_percent );
      parameters[ "mainchain_overlap.remove_sync" ] = QString( "%1" ).arg( mainchain_overlap.remove_sync );
      parameters[ "mainchain_overlap.remove_sync_percent" ] = QString( "%1" ).arg( mainchain_overlap.remove_sync_percent );
      parameters[ "mainchain_overlap.translate_out" ] = QString( "%1" ).arg( mainchain_overlap.translate_out );
      parameters[ "mainchain_overlap.show_translate" ] = QString( "%1" ).arg( mainchain_overlap.show_translate );
      parameters[ "buried_overlap.remove_overlap" ] = QString( "%1" ).arg( buried_overlap.remove_overlap );
      parameters[ "buried_overlap.fuse_beads" ] = QString( "%1" ).arg( buried_overlap.fuse_beads );
      parameters[ "buried_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( buried_overlap.fuse_beads_percent );
      parameters[ "buried_overlap.remove_hierarch" ] = QString( "%1" ).arg( buried_overlap.remove_hierarch );
      parameters[ "buried_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( buried_overlap.remove_hierarch_percent );
      parameters[ "buried_overlap.remove_sync" ] = QString( "%1" ).arg( buried_overlap.remove_sync );
      parameters[ "buried_overlap.remove_sync_percent" ] = QString( "%1" ).arg( buried_overlap.remove_sync_percent );
      parameters[ "buried_overlap.translate_out" ] = QString( "%1" ).arg( buried_overlap.translate_out );
      parameters[ "buried_overlap.show_translate" ] = QString( "%1" ).arg( buried_overlap.show_translate );
      parameters[ "replicate_o_r_method_grid" ] = QString( "%1" ).arg( replicate_o_r_method_grid );
      parameters[ "grid_exposed_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_overlap );
      parameters[ "grid_exposed_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_exposed_overlap.fuse_beads );
      parameters[ "grid_exposed_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.fuse_beads_percent );
      parameters[ "grid_exposed_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_hierarch );
      parameters[ "grid_exposed_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_hierarch_percent );
      parameters[ "grid_exposed_overlap.remove_sync" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_sync );
      parameters[ "grid_exposed_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_exposed_overlap.remove_sync_percent );
      parameters[ "grid_exposed_overlap.translate_out" ] = QString( "%1" ).arg( grid_exposed_overlap.translate_out );
      parameters[ "grid_exposed_overlap.show_translate" ] = QString( "%1" ).arg( grid_exposed_overlap.show_translate );
      parameters[ "grid_buried_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_buried_overlap.remove_overlap );
      parameters[ "grid_buried_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_buried_overlap.fuse_beads );
      parameters[ "grid_buried_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_buried_overlap.fuse_beads_percent );
      parameters[ "grid_buried_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_buried_overlap.remove_hierarch );
      parameters[ "grid_buried_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_buried_overlap.remove_hierarch_percent );
      parameters[ "grid_buried_overlap.remove_sync" ] = QString( "%1" ).arg( grid_buried_overlap.remove_sync );
      parameters[ "grid_buried_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_buried_overlap.remove_sync_percent );
      parameters[ "grid_buried_overlap.translate_out" ] = QString( "%1" ).arg( grid_buried_overlap.translate_out );
      parameters[ "grid_buried_overlap.show_translate" ] = QString( "%1" ).arg( grid_buried_overlap.show_translate );
      parameters[ "grid_overlap.remove_overlap" ] = QString( "%1" ).arg( grid_overlap.remove_overlap );
      parameters[ "grid_overlap.fuse_beads" ] = QString( "%1" ).arg( grid_overlap.fuse_beads );
      parameters[ "grid_overlap.fuse_beads_percent" ] = QString( "%1" ).arg( grid_overlap.fuse_beads_percent );
      parameters[ "grid_overlap.remove_hierarch" ] = QString( "%1" ).arg( grid_overlap.remove_hierarch );
      parameters[ "grid_overlap.remove_hierarch_percent" ] = QString( "%1" ).arg( grid_overlap.remove_hierarch_percent );
      parameters[ "grid_overlap.remove_sync" ] = QString( "%1" ).arg( grid_overlap.remove_sync );
      parameters[ "grid_overlap.remove_sync_percent" ] = QString( "%1" ).arg( grid_overlap.remove_sync_percent );
      parameters[ "grid_overlap.translate_out" ] = QString( "%1" ).arg( grid_overlap.translate_out );
      parameters[ "grid_overlap.show_translate" ] = QString( "%1" ).arg( grid_overlap.show_translate );
      parameters[ "overlap_tolerance" ] = QString( "%1" ).arg( overlap_tolerance );
      parameters[ "bead_output.output" ] = QString( "%1" ).arg( bead_output.output );
      parameters[ "bead_output.sequence" ] = QString( "%1" ).arg( bead_output.sequence );
      parameters[ "bead_output.correspondence" ] = QString( "%1" ).arg( bead_output.correspondence );
      parameters[ "asa.probe_radius" ] = QString( "%1" ).arg( asa.probe_radius );
      parameters[ "asa.probe_recheck_radius" ] = QString( "%1" ).arg( asa.probe_recheck_radius );
      parameters[ "asa.threshold" ] = QString( "%1" ).arg( asa.threshold );
      parameters[ "asa.threshold_percent" ] = QString( "%1" ).arg( asa.threshold_percent );
      parameters[ "asa.grid_threshold" ] = QString( "%1" ).arg( asa.grid_threshold );
      parameters[ "asa.grid_threshold_percent" ] = QString( "%1" ).arg( asa.grid_threshold_percent );
      parameters[ "asa.calculation" ] = QString( "%1" ).arg( asa.calculation );
      parameters[ "asa.recheck_beads" ] = QString( "%1" ).arg( asa.recheck_beads );
      parameters[ "asa.method" ] = QString( "%1" ).arg( asa.method );
      parameters[ "asa.asab1_step" ] = QString( "%1" ).arg( asa.asab1_step );
      parameters[ "grid.cubic" ] = QString( "%1" ).arg( grid.cubic );
      parameters[ "grid.hydrate" ] = QString( "%1" ).arg( grid.hydrate );
      parameters[ "grid.center" ] = QString( "%1" ).arg( grid.center );
      parameters[ "grid.tangency" ] = QString( "%1" ).arg( grid.tangency );
      parameters[ "grid.cube_side" ] = QString( "%1" ).arg( grid.cube_side );
      parameters[ "grid.enable_asa" ] = QString( "%1" ).arg( grid.enable_asa );
      parameters[ "misc.hydrovol" ] = QString( "%1" ).arg( misc.hydrovol );
      parameters[ "misc.compute_vbar" ] = QString( "%1" ).arg( misc.compute_vbar );
      parameters[ "misc.vbar" ] = QString( "%1" ).arg( misc.vbar );
      parameters[ "misc.vbar_temperature" ] = QString( "%1" ).arg( misc.vbar_temperature );
      parameters[ "misc.pb_rule_on" ] = QString( "%1" ).arg( misc.pb_rule_on );
      parameters[ "misc.avg_radius" ] = QString( "%1" ).arg( misc.avg_radius );
      parameters[ "misc.avg_mass" ] = QString( "%1" ).arg( misc.avg_mass );
      parameters[ "misc.avg_hydration" ] = QString( "%1" ).arg( misc.avg_hydration );
      parameters[ "misc.avg_volume" ] = QString( "%1" ).arg( misc.avg_volume );
      parameters[ "misc.avg_vbar" ] = QString( "%1" ).arg( misc.avg_vbar );
      parameters[ "hydro.unit" ] = QString( "%1" ).arg( hydro.unit );
      parameters[ "hydro.solvent_name" ] = QString( "%1" ).arg( hydro.solvent_name );
      parameters[ "hydro.solvent_acronym" ] = QString( "%1" ).arg( hydro.solvent_acronym );
      parameters[ "hydro.temperature" ] = QString( "%1" ).arg( hydro.temperature );
      parameters[ "hydro.solvent_viscosity" ] = QString( "%1" ).arg( hydro.solvent_viscosity );
      parameters[ "hydro.solvent_density" ] = QString( "%1" ).arg( hydro.solvent_density );
      parameters[ "hydro.reference_system" ] = QString( "%1" ).arg( hydro.reference_system );
      parameters[ "hydro.boundary_cond" ] = QString( "%1" ).arg( hydro.boundary_cond );
      parameters[ "hydro.volume_correction" ] = QString( "%1" ).arg( hydro.volume_correction );
      parameters[ "hydro.volume" ] = QString( "%1" ).arg( hydro.volume );
      parameters[ "hydro.mass_correction" ] = QString( "%1" ).arg( hydro.mass_correction );
      parameters[ "hydro.mass" ] = QString( "%1" ).arg( hydro.mass );
      parameters[ "hydro.bead_inclusion" ] = QString( "%1" ).arg( hydro.bead_inclusion );
      parameters[ "hydro.rotational" ] = QString( "%1" ).arg( hydro.rotational );
      parameters[ "hydro.viscosity" ] = QString( "%1" ).arg( hydro.viscosity );
      parameters[ "hydro.overlap_cutoff" ] = QString( "%1" ).arg( hydro.overlap_cutoff );
      parameters[ "hydro.overlap" ] = QString( "%1" ).arg( hydro.overlap );
      parameters[ "pdb_vis.visualization" ] = QString( "%1" ).arg( pdb_vis.visualization );
      parameters[ "pdb_vis.filename" ] = QString( "%1" ).arg( pdb_vis.filename );
      parameters[ "pdb_parse.skip_hydrogen" ] = QString( "%1" ).arg( pdb_parse.skip_hydrogen );
      parameters[ "pdb_parse.skip_water" ] = QString( "%1" ).arg( pdb_parse.skip_water );
      parameters[ "pdb_parse.alternate" ] = QString( "%1" ).arg( pdb_parse.alternate );
      parameters[ "pdb_parse.find_sh" ] = QString( "%1" ).arg( pdb_parse.find_sh );
      parameters[ "pdb_parse.missing_residues" ] = QString( "%1" ).arg( pdb_parse.missing_residues );
      parameters[ "pdb_parse.missing_atoms" ] = QString( "%1" ).arg( pdb_parse.missing_atoms );
      parameters[ "saxs_options.water_e_density" ] = QString( "%1" ).arg( saxs_options.water_e_density );
      parameters[ "saxs_options.h_scat_len" ] = QString( "%1" ).arg( saxs_options.h_scat_len );
      parameters[ "saxs_options.d_scat_len" ] = QString( "%1" ).arg( saxs_options.d_scat_len );
      parameters[ "saxs_options.h2o_scat_len_dens" ] = QString( "%1" ).arg( saxs_options.h2o_scat_len_dens );
      parameters[ "saxs_options.d2o_scat_len_dens" ] = QString( "%1" ).arg( saxs_options.d2o_scat_len_dens );
      parameters[ "saxs_options.d2o_conc" ] = QString( "%1" ).arg( saxs_options.d2o_conc );
      parameters[ "saxs_options.frac_of_exch_pep" ] = QString( "%1" ).arg( saxs_options.frac_of_exch_pep );
      parameters[ "saxs_options.wavelength" ] = QString( "%1" ).arg( saxs_options.wavelength );
      parameters[ "saxs_options.start_angle" ] = QString( "%1" ).arg( saxs_options.start_angle );
      parameters[ "saxs_options.end_angle" ] = QString( "%1" ).arg( saxs_options.end_angle );
      parameters[ "saxs_options.delta_angle" ] = QString( "%1" ).arg( saxs_options.delta_angle );
      parameters[ "saxs_options.max_size" ] = QString( "%1" ).arg( saxs_options.max_size );
      parameters[ "saxs_options.bin_size" ] = QString( "%1" ).arg( saxs_options.bin_size );
      parameters[ "saxs_options.hydrate_pdb" ] = QString( "%1" ).arg( saxs_options.hydrate_pdb );
      parameters[ "saxs_options.curve" ] = QString( "%1" ).arg( saxs_options.curve );
      parameters[ "saxs_options.saxs_sans" ] = QString( "%1" ).arg( saxs_options.saxs_sans );
      parameters[ "bd_options.threshold_pb_pb" ] = QString( "%1" ).arg( bd_options.threshold_pb_pb );
      parameters[ "bd_options.threshold_pb_sc" ] = QString( "%1" ).arg( bd_options.threshold_pb_sc );
      parameters[ "bd_options.threshold_sc_sc" ] = QString( "%1" ).arg( bd_options.threshold_sc_sc );
      parameters[ "bd_options.do_rr" ] = QString( "%1" ).arg( bd_options.do_rr );
      parameters[ "bd_options.force_chem" ] = QString( "%1" ).arg( bd_options.force_chem );
      parameters[ "bd_options.bead_size_type" ] = QString( "%1" ).arg( bd_options.bead_size_type );
      parameters[ "bd_options.show_pdb" ] = QString( "%1" ).arg( bd_options.show_pdb );
      parameters[ "bd_options.run_browflex" ] = QString( "%1" ).arg( bd_options.run_browflex );
      parameters[ "bd_options.tprev" ] = QString( "%1" ).arg( bd_options.tprev );
      parameters[ "bd_options.ttraj" ] = QString( "%1" ).arg( bd_options.ttraj );
      parameters[ "bd_options.deltat" ] = QString( "%1" ).arg( bd_options.deltat );
      parameters[ "bd_options.npadif" ] = QString( "%1" ).arg( bd_options.npadif );
      parameters[ "bd_options.nconf" ] = QString( "%1" ).arg( bd_options.nconf );
      parameters[ "bd_options.inter" ] = QString( "%1" ).arg( bd_options.inter );
      parameters[ "bd_options.iorder" ] = QString( "%1" ).arg( bd_options.iorder );
      parameters[ "bd_options.iseed" ] = QString( "%1" ).arg( bd_options.iseed );
      parameters[ "bd_options.icdm" ] = QString( "%1" ).arg( bd_options.icdm );
      parameters[ "bd_options.chem_pb_pb_bond_type" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_bond_type );
      parameters[ "bd_options.compute_chem_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_force_constant );
      parameters[ "bd_options.chem_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_force_constant );
      parameters[ "bd_options.compute_chem_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_equilibrium_dist );
      parameters[ "bd_options.chem_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_equilibrium_dist );
      parameters[ "bd_options.compute_chem_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_pb_max_elong );
      parameters[ "bd_options.chem_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.chem_pb_pb_max_elong );
      parameters[ "bd_options.chem_pb_sc_bond_type" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_bond_type );
      parameters[ "bd_options.compute_chem_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_force_constant );
      parameters[ "bd_options.chem_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_force_constant );
      parameters[ "bd_options.compute_chem_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_equilibrium_dist );
      parameters[ "bd_options.chem_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_equilibrium_dist );
      parameters[ "bd_options.compute_chem_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_pb_sc_max_elong );
      parameters[ "bd_options.chem_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.chem_pb_sc_max_elong );
      parameters[ "bd_options.chem_sc_sc_bond_type" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_bond_type );
      parameters[ "bd_options.compute_chem_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_force_constant );
      parameters[ "bd_options.chem_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_force_constant );
      parameters[ "bd_options.compute_chem_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_equilibrium_dist );
      parameters[ "bd_options.chem_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_equilibrium_dist );
      parameters[ "bd_options.compute_chem_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_chem_sc_sc_max_elong );
      parameters[ "bd_options.chem_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.chem_sc_sc_max_elong );
      parameters[ "bd_options.pb_pb_bond_type" ] = QString( "%1" ).arg( bd_options.pb_pb_bond_type );
      parameters[ "bd_options.compute_pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_force_constant );
      parameters[ "bd_options.pb_pb_force_constant" ] = QString( "%1" ).arg( bd_options.pb_pb_force_constant );
      parameters[ "bd_options.compute_pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_equilibrium_dist );
      parameters[ "bd_options.pb_pb_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.pb_pb_equilibrium_dist );
      parameters[ "bd_options.compute_pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.compute_pb_pb_max_elong );
      parameters[ "bd_options.pb_pb_max_elong" ] = QString( "%1" ).arg( bd_options.pb_pb_max_elong );
      parameters[ "bd_options.pb_sc_bond_type" ] = QString( "%1" ).arg( bd_options.pb_sc_bond_type );
      parameters[ "bd_options.compute_pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_force_constant );
      parameters[ "bd_options.pb_sc_force_constant" ] = QString( "%1" ).arg( bd_options.pb_sc_force_constant );
      parameters[ "bd_options.compute_pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_equilibrium_dist );
      parameters[ "bd_options.pb_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.pb_sc_equilibrium_dist );
      parameters[ "bd_options.compute_pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_pb_sc_max_elong );
      parameters[ "bd_options.pb_sc_max_elong" ] = QString( "%1" ).arg( bd_options.pb_sc_max_elong );
      parameters[ "bd_options.sc_sc_bond_type" ] = QString( "%1" ).arg( bd_options.sc_sc_bond_type );
      parameters[ "bd_options.compute_sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_force_constant );
      parameters[ "bd_options.sc_sc_force_constant" ] = QString( "%1" ).arg( bd_options.sc_sc_force_constant );
      parameters[ "bd_options.compute_sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_equilibrium_dist );
      parameters[ "bd_options.sc_sc_equilibrium_dist" ] = QString( "%1" ).arg( bd_options.sc_sc_equilibrium_dist );
      parameters[ "bd_options.compute_sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.compute_sc_sc_max_elong );
      parameters[ "bd_options.sc_sc_max_elong" ] = QString( "%1" ).arg( bd_options.sc_sc_max_elong );
      parameters[ "bd_options.nmol" ] = QString( "%1" ).arg( bd_options.nmol );
      parameters[ "anaflex_options.run_anaflex" ] = QString( "%1" ).arg( anaflex_options.run_anaflex );
      parameters[ "anaflex_options.nfrec" ] = QString( "%1" ).arg( anaflex_options.nfrec );
      parameters[ "anaflex_options.instprofiles" ] = QString( "%1" ).arg( anaflex_options.instprofiles );
      parameters[ "anaflex_options.run_mode_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_1 );
      parameters[ "anaflex_options.run_mode_1_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_1 );
      parameters[ "anaflex_options.run_mode_1_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_2 );
      parameters[ "anaflex_options.run_mode_1_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_3 );
      parameters[ "anaflex_options.run_mode_1_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_4 );
      parameters[ "anaflex_options.run_mode_1_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_5 );
      parameters[ "anaflex_options.run_mode_1_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_7 );
      parameters[ "anaflex_options.run_mode_1_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_8 );
      parameters[ "anaflex_options.run_mode_1_12" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_12 );
      parameters[ "anaflex_options.run_mode_1_13" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_13 );
      parameters[ "anaflex_options.run_mode_1_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_14 );
      parameters[ "anaflex_options.run_mode_1_18" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_18 );
      parameters[ "anaflex_options.run_mode_1_20" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_20 );
      parameters[ "anaflex_options.run_mode_1_24" ] = QString( "%1" ).arg( anaflex_options.run_mode_1_24 );
      parameters[ "anaflex_options.run_mode_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_2 );
      parameters[ "anaflex_options.run_mode_2_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_1 );
      parameters[ "anaflex_options.run_mode_2_2" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_2 );
      parameters[ "anaflex_options.run_mode_2_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_3 );
      parameters[ "anaflex_options.run_mode_2_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_4 );
      parameters[ "anaflex_options.run_mode_2_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_5 );
      parameters[ "anaflex_options.run_mode_2_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_7 );
      parameters[ "anaflex_options.run_mode_2_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_8 );
      parameters[ "anaflex_options.run_mode_2_12" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_12 );
      parameters[ "anaflex_options.run_mode_2_13" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_13 );
      parameters[ "anaflex_options.run_mode_2_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_14 );
      parameters[ "anaflex_options.run_mode_2_18" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_18 );
      parameters[ "anaflex_options.run_mode_2_20" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_20 );
      parameters[ "anaflex_options.run_mode_2_24" ] = QString( "%1" ).arg( anaflex_options.run_mode_2_24 );
      parameters[ "anaflex_options.run_mode_3" ] = QString( "%1" ).arg( anaflex_options.run_mode_3 );
      parameters[ "anaflex_options.run_mode_3_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_1 );
      parameters[ "anaflex_options.run_mode_3_5" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5 );
      parameters[ "anaflex_options.run_mode_3_9" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_9 );
      parameters[ "anaflex_options.run_mode_3_10" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10 );
      parameters[ "anaflex_options.run_mode_3_14" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14 );
      parameters[ "anaflex_options.run_mode_3_15" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_15 );
      parameters[ "anaflex_options.run_mode_3_16" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_16 );
      parameters[ "anaflex_options.run_mode_4" ] = QString( "%1" ).arg( anaflex_options.run_mode_4 );
      parameters[ "anaflex_options.run_mode_4_1" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_1 );
      parameters[ "anaflex_options.run_mode_4_6" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_6 );
      parameters[ "anaflex_options.run_mode_4_7" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_7 );
      parameters[ "anaflex_options.run_mode_4_8" ] = QString( "%1" ).arg( anaflex_options.run_mode_4_8 );
      parameters[ "anaflex_options.run_mode_9" ] = QString( "%1" ).arg( anaflex_options.run_mode_9 );
      parameters[ "anaflex_options.ntimc" ] = QString( "%1" ).arg( anaflex_options.ntimc );
      parameters[ "anaflex_options.tmax" ] = QString( "%1" ).arg( anaflex_options.tmax );
      parameters[ "anaflex_options.run_mode_3_5_iii" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5_iii );
      parameters[ "anaflex_options.run_mode_3_5_jjj" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_5_jjj );
      parameters[ "anaflex_options.run_mode_3_10_theta" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_theta );
      parameters[ "anaflex_options.run_mode_3_10_refractive_index" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_refractive_index );
      parameters[ "anaflex_options.run_mode_3_10_lambda" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_10_lambda );
      parameters[ "anaflex_options.run_mode_3_14_iii" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14_iii );
      parameters[ "anaflex_options.run_mode_3_14_jjj" ] = QString( "%1" ).arg( anaflex_options.run_mode_3_14_jjj );
      parameters[ "batch.missing_atoms" ] = QString( "%1" ).arg( batch.missing_atoms );
      parameters[ "batch.missing_residues" ] = QString( "%1" ).arg( batch.missing_residues );
      parameters[ "batch.somo" ] = QString( "%1" ).arg( batch.somo );
      parameters[ "batch.grid" ] = QString( "%1" ).arg( batch.grid );
      parameters[ "batch.hydro" ] = QString( "%1" ).arg( batch.hydro );
      parameters[ "batch.avg_hydro" ] = QString( "%1" ).arg( batch.avg_hydro );
      parameters[ "batch.avg_hydro_name" ] = QString( "%1" ).arg( batch.avg_hydro_name );
      parameters[ "batch.height" ] = QString( "%1" ).arg( batch.height );
      parameters[ "batch.width" ] = QString( "%1" ).arg( batch.width );
      parameters[ "path_load_pdb" ] = QString( "%1" ).arg( path_load_pdb );
      parameters[ "path_view_pdb" ] = QString( "%1" ).arg( path_view_pdb );
      parameters[ "path_load_bead_model" ] = QString( "%1" ).arg( path_load_bead_model );
      parameters[ "path_view_asa_res" ] = QString( "%1" ).arg( path_view_asa_res );
      parameters[ "path_view_bead_model" ] = QString( "%1" ).arg( path_view_bead_model );
      parameters[ "path_open_hydro_res" ] = QString( "%1" ).arg( path_open_hydro_res );
      parameters[ "saxs_options.path_load_saxs_curve" ] = QString( "%1" ).arg( saxs_options.path_load_saxs_curve );
      parameters[ "saxs_options.path_load_gnom" ] = QString( "%1" ).arg( saxs_options.path_load_gnom );
      parameters[ "saxs_options.path_load_prr" ] = QString( "%1" ).arg( saxs_options.path_load_prr );
      parameters[ "asa.hydrate_probe_radius" ] = QString( "%1" ).arg( asa.hydrate_probe_radius );
      parameters[ "asa.hydrate_threshold" ] = QString( "%1" ).arg( asa.hydrate_threshold );
      parameters[ "misc.target_e_density" ] = QString( "%1" ).arg( misc.target_e_density );
      parameters[ "misc.target_volume" ] = QString( "%1" ).arg( misc.target_volume );
      parameters[ "misc.set_target_on_load_pdb" ] = QString( "%1" ).arg( misc.set_target_on_load_pdb );
      parameters[ "misc.equalize_radii" ] = QString( "%1" ).arg( misc.equalize_radii );
      parameters[ "dmd_options.force_chem" ] = QString( "%1" ).arg( dmd_options.force_chem );
      parameters[ "dmd_options.pdb_static_pairs" ] = QString( "%1" ).arg( dmd_options.pdb_static_pairs );
      parameters[ "dmd_options.threshold_pb_pb" ] = QString( "%1" ).arg( dmd_options.threshold_pb_pb );
      parameters[ "dmd_options.threshold_pb_sc" ] = QString( "%1" ).arg( dmd_options.threshold_pb_sc );
      parameters[ "dmd_options.threshold_sc_sc" ] = QString( "%1" ).arg( dmd_options.threshold_sc_sc );
      parameters[ "saxs_options.normalize_by_mw" ] = QString( "%1" ).arg( saxs_options.normalize_by_mw );
      parameters[ "saxs_options.saxs_iq_native_debye" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_debye );
      parameters[ "saxs_options.saxs_iq_native_hybrid" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid );
      parameters[ "saxs_options.saxs_iq_native_hybrid2" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid2 );
      parameters[ "saxs_options.saxs_iq_native_hybrid3" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_hybrid3 );
      parameters[ "saxs_options.saxs_iq_native_fast" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_fast );
      parameters[ "saxs_options.saxs_iq_native_fast_compute_pr" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_fast_compute_pr );
      parameters[ "saxs_options.saxs_iq_foxs" ] = QString( "%1" ).arg( saxs_options.saxs_iq_foxs );
      parameters[ "saxs_options.saxs_iq_crysol" ] = QString( "%1" ).arg( saxs_options.saxs_iq_crysol );
      parameters[ "saxs_options.sans_iq_native_debye" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_debye );
      parameters[ "saxs_options.sans_iq_native_hybrid" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid );
      parameters[ "saxs_options.sans_iq_native_hybrid2" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid2 );
      parameters[ "saxs_options.sans_iq_native_hybrid3" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_hybrid3 );
      parameters[ "saxs_options.sans_iq_native_fast" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_fast );
      parameters[ "saxs_options.sans_iq_native_fast_compute_pr" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_fast_compute_pr );
      parameters[ "saxs_options.sans_iq_cryson" ] = QString( "%1" ).arg( saxs_options.sans_iq_cryson );
      parameters[ "saxs_options.hybrid2_q_points" ] = QString( "%1" ).arg( saxs_options.hybrid2_q_points );
      parameters[ "saxs_options.iq_ask" ] = QString( "%1" ).arg( saxs_options.iq_ask );
      parameters[ "saxs_options.iq_scale_ask" ] = QString( "%1" ).arg( saxs_options.iq_scale_ask );
      parameters[ "saxs_options.iq_scale_angstrom" ] = QString( "%1" ).arg( saxs_options.iq_scale_angstrom );
      parameters[ "saxs_options.iq_scale_nm" ] = QString( "%1" ).arg( saxs_options.iq_scale_nm );
      parameters[ "saxs_options.sh_max_harmonics" ] = QString( "%1" ).arg( saxs_options.sh_max_harmonics );
      parameters[ "saxs_options.sh_fibonacci_grid_order" ] = QString( "%1" ).arg( saxs_options.sh_fibonacci_grid_order );
      parameters[ "saxs_options.crysol_hydration_shell_contrast" ] = QString( "%1" ).arg( saxs_options.crysol_hydration_shell_contrast );
      parameters[ "saxs_options.crysol_default_load_difference_intensity" ] = QString( "%1" ).arg( saxs_options.crysol_default_load_difference_intensity );
      parameters[ "saxs_options.crysol_version_26" ] = QString( "%1" ).arg( saxs_options.crysol_version_26 );
      parameters[ "saxs_options.fast_bin_size" ] = QString( "%1" ).arg( saxs_options.fast_bin_size );
      parameters[ "saxs_options.fast_modulation" ] = QString( "%1" ).arg( saxs_options.fast_modulation );
      parameters[ "saxs_options.compute_saxs_coeff_for_bead_models" ] = QString( "%1" ).arg( saxs_options.compute_saxs_coeff_for_bead_models );
      parameters[ "saxs_options.compute_sans_coeff_for_bead_models" ] = QString( "%1" ).arg( saxs_options.compute_sans_coeff_for_bead_models );
      parameters[ "saxs_options.default_atom_filename" ] = QString( "%1" ).arg( saxs_options.default_atom_filename );
      parameters[ "saxs_options.default_hybrid_filename" ] = QString( "%1" ).arg( saxs_options.default_hybrid_filename );
      parameters[ "saxs_options.default_saxs_filename" ] = QString( "%1" ).arg( saxs_options.default_saxs_filename );
      parameters[ "saxs_options.default_rotamer_filename" ] = QString( "%1" ).arg( saxs_options.default_rotamer_filename );
      parameters[ "saxs_options.steric_clash_distance" ] = QString( "%1" ).arg( saxs_options.steric_clash_distance );
      parameters[ "saxs_options.steric_clash_recheck_distance" ] = QString( "%1" ).arg( saxs_options.steric_clash_recheck_distance );
      parameters[ "saxs_options.disable_iq_scaling" ] = QString( "%1" ).arg( saxs_options.disable_iq_scaling );
      parameters[ "saxs_options.autocorrelate" ] = QString( "%1" ).arg( saxs_options.autocorrelate );
      parameters[ "saxs_options.hybrid_radius_excl_vol" ] = QString( "%1" ).arg( saxs_options.hybrid_radius_excl_vol );
      parameters[ "saxs_options.scale_excl_vol" ] = QString( "%1" ).arg( saxs_options.scale_excl_vol );
      parameters[ "saxs_options.subtract_radius" ] = QString( "%1" ).arg( saxs_options.subtract_radius );
      parameters[ "saxs_options.iqq_scale_minq" ] = QString( "%1" ).arg( saxs_options.iqq_scale_minq );
      parameters[ "saxs_options.iqq_scale_maxq" ] = QString( "%1" ).arg( saxs_options.iqq_scale_maxq );
      parameters[ "saxs_options.iqq_scale_nnls" ] = QString( "%1" ).arg( saxs_options.iqq_scale_nnls );
      parameters[ "saxs_options.iqq_scale_linear_offset" ] = QString( "%1" ).arg( saxs_options.iqq_scale_linear_offset );
      parameters[ "saxs_options.iqq_scale_chi2_fitting" ] = QString( "%1" ).arg( saxs_options.iqq_scale_chi2_fitting );
      parameters[ "saxs_options.iqq_expt_data_contains_variances" ] = QString( "%1" ).arg( saxs_options.iqq_expt_data_contains_variances );
      parameters[ "saxs_options.iqq_ask_target_grid" ] = QString( "%1" ).arg( saxs_options.iqq_ask_target_grid );
      parameters[ "saxs_options.iqq_scale_play" ] = QString( "%1" ).arg( saxs_options.iqq_scale_play );
      parameters[ "saxs_options.swh_excl_vol" ] = QString( "%1" ).arg( saxs_options.swh_excl_vol );
      parameters[ "saxs_options.iqq_default_scaling_target" ] = QString( "%1" ).arg( saxs_options.iqq_default_scaling_target );
      parameters[ "saxs_options.saxs_iq_hybrid_adaptive" ] = QString( "%1" ).arg( saxs_options.saxs_iq_hybrid_adaptive );
      parameters[ "saxs_options.sans_iq_hybrid_adaptive" ] = QString( "%1" ).arg( saxs_options.sans_iq_hybrid_adaptive );
      parameters[ "saxs_options.bead_model_rayleigh" ] = QString( "%1" ).arg( saxs_options.bead_model_rayleigh );
      parameters[ "saxs_options.iqq_log_fitting" ] = QString( "%1" ).arg( saxs_options.iqq_log_fitting );
      parameters[ "saxs_options.iqq_kratky_fit" ] = QString( "%1" ).arg( saxs_options.iqq_kratky_fit );
      parameters[ "saxs_options.iqq_use_atomic_ff" ] = QString( "%1" ).arg( saxs_options.iqq_use_atomic_ff );
      parameters[ "saxs_options.iqq_use_saxs_excl_vol" ] = QString( "%1" ).arg( saxs_options.iqq_use_saxs_excl_vol );
      parameters[ "saxs_options.alt_hydration" ] = QString( "%1" ).arg( saxs_options.alt_hydration );
      parameters[ "saxs_options.xsr_symmop" ] = QString( "%1" ).arg( saxs_options.xsr_symmop );
      parameters[ "saxs_options.xsr_nx" ] = QString( "%1" ).arg( saxs_options.xsr_nx );
      parameters[ "saxs_options.xsr_ny" ] = QString( "%1" ).arg( saxs_options.xsr_ny );
      parameters[ "saxs_options.xsr_griddistance" ] = QString( "%1" ).arg( saxs_options.xsr_griddistance );
      parameters[ "saxs_options.xsr_ncomponents" ] = QString( "%1" ).arg( saxs_options.xsr_ncomponents );
      parameters[ "saxs_options.xsr_compactness_weight" ] = QString( "%1" ).arg( saxs_options.xsr_compactness_weight );
      parameters[ "saxs_options.xsr_looseness_weight" ] = QString( "%1" ).arg( saxs_options.xsr_looseness_weight );
      parameters[ "saxs_options.xsr_temperature" ] = QString( "%1" ).arg( saxs_options.xsr_temperature );
      parameters[ "hydro.zeno_zeno" ] = QString( "%1" ).arg( hydro.zeno_zeno );
      parameters[ "hydro.zeno_interior" ] = QString( "%1" ).arg( hydro.zeno_interior );
      parameters[ "hydro.zeno_surface" ] = QString( "%1" ).arg( hydro.zeno_surface );
      parameters[ "hydro.zeno_zeno_steps" ] = QString( "%1" ).arg( hydro.zeno_zeno_steps );
      parameters[ "hydro.zeno_interior_steps" ] = QString( "%1" ).arg( hydro.zeno_interior_steps );
      parameters[ "hydro.zeno_surface_steps" ] = QString( "%1" ).arg( hydro.zeno_surface_steps );
      parameters[ "hydro.zeno_surface_thickness" ] = QString( "%1" ).arg( hydro.zeno_surface_thickness );
      parameters[ "misc.hydro_supc" ] = QString( "%1" ).arg( misc.hydro_supc );
      parameters[ "misc.hydro_zeno" ] = QString( "%1" ).arg( misc.hydro_zeno );
      parameters[ "batch.saxs_search" ] = QString( "%1" ).arg( batch.saxs_search );
      parameters[ "batch.zeno" ] = QString( "%1" ).arg( batch.zeno );

      parameters[ "saxs_options.ignore_errors" ] = QString( "%1" ).arg( saxs_options.ignore_errors );
      parameters[ "saxs_options.alt_ff" ] = QString( "%1" ).arg( saxs_options.alt_ff );
      parameters[ "saxs_options.crysol_explicit_hydrogens" ] = QString( "%1" ).arg( saxs_options.crysol_explicit_hydrogens );
      parameters[ "saxs_options.use_somo_ff" ] = QString( "%1" ).arg( saxs_options.use_somo_ff );
      parameters[ "saxs_options.five_term_gaussians" ] = QString( "%1" ).arg( saxs_options.five_term_gaussians );
      parameters[ "saxs_options.iq_exact_q" ] = QString( "%1" ).arg( saxs_options.iq_exact_q );
      parameters[ "saxs_options.use_iq_target_ev" ] = QString( "%1" ).arg( saxs_options.use_iq_target_ev );
      parameters[ "saxs_options.set_iq_target_ev_from_vbar" ] = QString( "%1" ).arg( saxs_options.set_iq_target_ev_from_vbar );
      parameters[ "saxs_options.iq_target_ev" ] = QString( "%1" ).arg( saxs_options.iq_target_ev );
      parameters[ "saxs_options.hydration_rev_asa" ] = QString( "%1" ).arg( saxs_options.hydration_rev_asa );
      parameters[ "saxs_options.compute_exponentials" ] = QString( "%1" ).arg( saxs_options.compute_exponentials );
      parameters[ "saxs_options.compute_exponential_terms" ] = QString( "%1" ).arg( saxs_options.compute_exponential_terms );
      parameters[ "saxs_options.dummy_saxs_name" ] = QString( "%1" ).arg( saxs_options.dummy_saxs_name );
      parameters[ "saxs_options.multiply_iq_by_atomic_volume" ] = QString( "%1" ).arg( saxs_options.multiply_iq_by_atomic_volume );
      parameters[ "saxs_options.dummy_atom_pdbs_in_nm" ] = QString( "%1" ).arg( saxs_options.dummy_atom_pdbs_in_nm );
      parameters[ "saxs_options.iq_global_avg_for_bead_models" ] = QString( "%1" ).arg( saxs_options.iq_global_avg_for_bead_models );
      parameters[ "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ] = QString( "%1" ).arg( saxs_options.apply_loaded_sf_repeatedly_to_pdb );
      parameters[ "saxs_options.bead_models_use_var_len_sf" ] = QString( "%1" ).arg( saxs_options.bead_models_use_var_len_sf );
      parameters[ "saxs_options.bead_models_var_len_sf_max" ] = QString( "%1" ).arg( saxs_options.bead_models_var_len_sf_max );
      parameters[ "saxs_options.bead_models_use_gsm_fitting" ] = QString( "%1" ).arg( saxs_options.bead_models_use_gsm_fitting );
      parameters[ "saxs_options.bead_models_use_quick_fitting" ] = QString( "%1" ).arg( saxs_options.bead_models_use_quick_fitting );
      parameters[ "saxs_options.bead_models_use_bead_radius_ev" ] = QString( "%1" ).arg( saxs_options.bead_models_use_bead_radius_ev );
      parameters[ "saxs_options.bead_models_rho0_in_scat_factors" ] = QString( "%1" ).arg( saxs_options.bead_models_rho0_in_scat_factors );
      parameters[ "saxs_options.smooth" ] = QString( "%1" ).arg( saxs_options.smooth );
      parameters[ "saxs_options.ev_exp_mult" ] = QString( "%1" ).arg( saxs_options.ev_exp_mult );
      parameters[ "saxs_options.sastbx_method" ] = QString( "%1" ).arg( saxs_options.sastbx_method );
      parameters[ "saxs_options.saxs_iq_sastbx" ] = QString( "%1" ).arg( saxs_options.saxs_iq_sastbx );
      parameters[ "saxs_options.saxs_iq_native_sh" ] = QString( "%1" ).arg( saxs_options.saxs_iq_native_sh );
      parameters[ "saxs_options.sans_iq_native_sh" ] = QString( "%1" ).arg( saxs_options.sans_iq_native_sh );
      parameters[ "saxs_options.alt_sh1" ] = QString( "%1" ).arg( saxs_options.alt_sh1 );
      parameters[ "saxs_options.alt_sh2" ] = QString( "%1" ).arg( saxs_options.alt_sh2 );
      parameters[ "grid.create_nmr_bead_pdb" ] = QString( "%1" ).arg( grid.create_nmr_bead_pdb );
      parameters[ "batch.compute_iq_only_avg" ] = QString( "%1" ).arg( batch.compute_iq_only_avg );
      parameters[ "asa.vvv" ] = QString( "%1" ).arg( asa.vvv );
      parameters[ "asa.vvv_probe_radius" ] = QString( "%1" ).arg( asa.vvv_probe_radius );
      parameters[ "asa.vvv_grid_dR" ] = QString( "%1" ).arg( asa.vvv_grid_dR );
      parameters[ "misc.export_msroll" ] = QString( "%1" ).arg( misc.export_msroll );

      parameters[ "saxs_options.qstart" ] = QString( "%1" ).arg( saxs_options.qstart );
      parameters[ "saxs_options.qend" ] = QString( "%1" ).arg( saxs_options.qend );
      parameters[ "saxs_options.guinier_csv" ] = QString( "%1" ).arg( saxs_options.guinier_csv ); // = false;
      parameters[ "saxs_options.guinier_csv_filename" ] = QString( "%1" ).arg( saxs_options.guinier_csv_filename ); // = "guinier";
      parameters[ "saxs_options.qRgmax" ] = QString( "%1" ).arg( saxs_options.qRgmax ); // = 1.3e0;
      parameters[ "saxs_options.pointsmin" ] = QString( "%1" ).arg( saxs_options.pointsmin ); // = 10;
      parameters[ "saxs_options.pointsmax" ] = QString( "%1" ).arg( saxs_options.pointsmax ); // = 100;

      parameters[ "saxs_options.cs_qRgmax" ] = QString( "%1" ).arg( saxs_options.cs_qRgmax );
      parameters[ "saxs_options.cs_qstart" ] = QString( "%1" ).arg( saxs_options.cs_qstart );
      parameters[ "saxs_options.cs_qend" ] = QString( "%1" ).arg( saxs_options.cs_qend );
      parameters[ "saxs_options.conc" ] = QString( "%1" ).arg( saxs_options.conc );
      parameters[ "saxs_options.psv" ] = QString( "%1" ).arg( saxs_options.psv );
      parameters[ "saxs_options.use_cs_psv" ] = QString( "%1" ).arg( saxs_options.use_cs_psv );
      parameters[ "saxs_options.cs_psv" ] = QString( "%1" ).arg( saxs_options.cs_psv );
      parameters[ "saxs_options.I0_exp" ] = QString( "%1" ).arg( saxs_options.I0_exp );
      parameters[ "saxs_options.I0_theo" ] = QString( "%1" ).arg( saxs_options.I0_theo );
      parameters[ "saxs_options.diffusion_len" ] = QString( "%1" ).arg( saxs_options.diffusion_len );
      parameters[ "saxs_options.nucleon_mass" ] = QString( "%1" ).arg( saxs_options.nucleon_mass );
      parameters[ "saxs_options.guinier_outlier_reject" ] = QString( "%1" ).arg( saxs_options.guinier_outlier_reject );
      parameters[ "saxs_options.guinier_outlier_reject_dist" ] = QString( "%1" ).arg( saxs_options.guinier_outlier_reject_dist );
      parameters[ "saxs_options.guinier_use_sd" ] = QString( "%1" ).arg( saxs_options.guinier_use_sd );
      parameters[ "saxs_options.guinier_use_standards" ] = QString( "%1" ).arg( saxs_options.guinier_use_standards );

      parameters[ "saxs_options.cryson_sh_max_harmonics" ] = QString( "%1" ).arg( saxs_options.cryson_sh_max_harmonics ); //            = 15;
      parameters[ "saxs_options.cryson_sh_fibonacci_grid_order" ] = QString( "%1" ).arg( saxs_options.cryson_sh_fibonacci_grid_order ); //     = 17;
      parameters[ "saxs_options.cryson_hydration_shell_contrast" ] = QString( "%1" ).arg( saxs_options.cryson_hydration_shell_contrast ); //    = 0.03f;
      parameters[ "saxs_options.cryson_manual_hs" ] = QString( "%1" ).arg( saxs_options.cryson_manual_hs ); //    = 0.03f;

      // vectors to write:
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) saxs_options.dummy_saxs_names.size(); i++ )
         {
            qsl_tmp << saxs_options.dummy_saxs_names[ i ];
         }
         parameters[ "saxs_options.dummy_saxs_names" ] = qsl_tmp.join( "\n" );
      }
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) batch.file.size(); i++ )
         {
            qsl_tmp << batch.file[ i ];
         }
         parameters[ "batch.file" ] = qsl_tmp.join( "\n" );
      }
      {
         QStringList qsl_tmp;
         for ( unsigned int i = 0; i < (unsigned int) save_params.field.size(); i++ )
         {
            qsl_tmp << save_params.field[ i ];
         }
         parameters[ "save_params.field" ] = qsl_tmp.join( "\n" );
      }
      
      for ( map < QString, QString >::iterator it = gparams.begin();
            it != gparams.end();
            it++ )
      {
         parameters[ "gparam:" + it->first ] = it->second;
      }

      {
         QStringList qsl_tmp1;
         QStringList qsl_tmp2;
         QStringList qsl_tmp3;
         for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
         {
            if ( directory_last_access.count( directory_history[ i ] ) )
            { 
               qsl_tmp1 << directory_history[ i ];
               qsl_tmp2 << QString( "%1" ).arg( (unsigned int)directory_last_access[ directory_history[ i ] ].toTime_t() );
               qsl_tmp3 << 
                  ( directory_last_filetype.count( directory_history[ i ] ) ?                  
                    directory_last_filetype[ directory_history[ i ] ] : "____" );
            }
         }
         if ( qsl_tmp1.size() )
         {
            parameters[ "directory_history"       ] = qsl_tmp1.join( "\n" );
            parameters[ "directory_last_access"   ] = qsl_tmp2.join( "\n" );
            parameters[ "directory_last_filetype" ] = qsl_tmp3.join( "\n" );
         }
      }

      ts << US_Json::compose( parameters );
      f.close();
   }
}

bool US_Hydrodyn::load_config_json ( QString &json )
{
   map < QString, QString > parameters = US_Json::split( json );

   // first set to default parameters
   hard_coded_defaults();

   if ( parameters.count( "replicate_o_r_method_somo" ) ) replicate_o_r_method_somo = parameters[ "replicate_o_r_method_somo" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_overlap" ) ) sidechain_overlap.remove_overlap = parameters[ "sidechain_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "sidechain_overlap.fuse_beads" ) ) sidechain_overlap.fuse_beads = parameters[ "sidechain_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "sidechain_overlap.fuse_beads_percent" ) ) sidechain_overlap.fuse_beads_percent = parameters[ "sidechain_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.remove_hierarch" ) ) sidechain_overlap.remove_hierarch = parameters[ "sidechain_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_hierarch_percent" ) ) sidechain_overlap.remove_hierarch_percent = parameters[ "sidechain_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.remove_sync" ) ) sidechain_overlap.remove_sync = parameters[ "sidechain_overlap.remove_sync" ] == "1";
   if ( parameters.count( "sidechain_overlap.remove_sync_percent" ) ) sidechain_overlap.remove_sync_percent = parameters[ "sidechain_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "sidechain_overlap.translate_out" ) ) sidechain_overlap.translate_out = parameters[ "sidechain_overlap.translate_out" ] == "1";
   if ( parameters.count( "sidechain_overlap.show_translate" ) ) sidechain_overlap.show_translate = parameters[ "sidechain_overlap.show_translate" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_overlap" ) ) mainchain_overlap.remove_overlap = parameters[ "mainchain_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "mainchain_overlap.fuse_beads" ) ) mainchain_overlap.fuse_beads = parameters[ "mainchain_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "mainchain_overlap.fuse_beads_percent" ) ) mainchain_overlap.fuse_beads_percent = parameters[ "mainchain_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.remove_hierarch" ) ) mainchain_overlap.remove_hierarch = parameters[ "mainchain_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_hierarch_percent" ) ) mainchain_overlap.remove_hierarch_percent = parameters[ "mainchain_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.remove_sync" ) ) mainchain_overlap.remove_sync = parameters[ "mainchain_overlap.remove_sync" ] == "1";
   if ( parameters.count( "mainchain_overlap.remove_sync_percent" ) ) mainchain_overlap.remove_sync_percent = parameters[ "mainchain_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "mainchain_overlap.translate_out" ) ) mainchain_overlap.translate_out = parameters[ "mainchain_overlap.translate_out" ] == "1";
   if ( parameters.count( "mainchain_overlap.show_translate" ) ) mainchain_overlap.show_translate = parameters[ "mainchain_overlap.show_translate" ] == "1";
   if ( parameters.count( "buried_overlap.remove_overlap" ) ) buried_overlap.remove_overlap = parameters[ "buried_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "buried_overlap.fuse_beads" ) ) buried_overlap.fuse_beads = parameters[ "buried_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "buried_overlap.fuse_beads_percent" ) ) buried_overlap.fuse_beads_percent = parameters[ "buried_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.remove_hierarch" ) ) buried_overlap.remove_hierarch = parameters[ "buried_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "buried_overlap.remove_hierarch_percent" ) ) buried_overlap.remove_hierarch_percent = parameters[ "buried_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.remove_sync" ) ) buried_overlap.remove_sync = parameters[ "buried_overlap.remove_sync" ] == "1";
   if ( parameters.count( "buried_overlap.remove_sync_percent" ) ) buried_overlap.remove_sync_percent = parameters[ "buried_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "buried_overlap.translate_out" ) ) buried_overlap.translate_out = parameters[ "buried_overlap.translate_out" ] == "1";
   if ( parameters.count( "buried_overlap.show_translate" ) ) buried_overlap.show_translate = parameters[ "buried_overlap.show_translate" ] == "1";
   if ( parameters.count( "replicate_o_r_method_grid" ) ) replicate_o_r_method_grid = parameters[ "replicate_o_r_method_grid" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_overlap" ) ) grid_exposed_overlap.remove_overlap = parameters[ "grid_exposed_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.fuse_beads" ) ) grid_exposed_overlap.fuse_beads = parameters[ "grid_exposed_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.fuse_beads_percent" ) ) grid_exposed_overlap.fuse_beads_percent = parameters[ "grid_exposed_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.remove_hierarch" ) ) grid_exposed_overlap.remove_hierarch = parameters[ "grid_exposed_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_hierarch_percent" ) ) grid_exposed_overlap.remove_hierarch_percent = parameters[ "grid_exposed_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.remove_sync" ) ) grid_exposed_overlap.remove_sync = parameters[ "grid_exposed_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.remove_sync_percent" ) ) grid_exposed_overlap.remove_sync_percent = parameters[ "grid_exposed_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_exposed_overlap.translate_out" ) ) grid_exposed_overlap.translate_out = parameters[ "grid_exposed_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_exposed_overlap.show_translate" ) ) grid_exposed_overlap.show_translate = parameters[ "grid_exposed_overlap.show_translate" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_overlap" ) ) grid_buried_overlap.remove_overlap = parameters[ "grid_buried_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_buried_overlap.fuse_beads" ) ) grid_buried_overlap.fuse_beads = parameters[ "grid_buried_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_buried_overlap.fuse_beads_percent" ) ) grid_buried_overlap.fuse_beads_percent = parameters[ "grid_buried_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.remove_hierarch" ) ) grid_buried_overlap.remove_hierarch = parameters[ "grid_buried_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_hierarch_percent" ) ) grid_buried_overlap.remove_hierarch_percent = parameters[ "grid_buried_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.remove_sync" ) ) grid_buried_overlap.remove_sync = parameters[ "grid_buried_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_buried_overlap.remove_sync_percent" ) ) grid_buried_overlap.remove_sync_percent = parameters[ "grid_buried_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_buried_overlap.translate_out" ) ) grid_buried_overlap.translate_out = parameters[ "grid_buried_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_buried_overlap.show_translate" ) ) grid_buried_overlap.show_translate = parameters[ "grid_buried_overlap.show_translate" ] == "1";
   if ( parameters.count( "grid_overlap.remove_overlap" ) ) grid_overlap.remove_overlap = parameters[ "grid_overlap.remove_overlap" ] == "1";
   if ( parameters.count( "grid_overlap.fuse_beads" ) ) grid_overlap.fuse_beads = parameters[ "grid_overlap.fuse_beads" ] == "1";
   if ( parameters.count( "grid_overlap.fuse_beads_percent" ) ) grid_overlap.fuse_beads_percent = parameters[ "grid_overlap.fuse_beads_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.remove_hierarch" ) ) grid_overlap.remove_hierarch = parameters[ "grid_overlap.remove_hierarch" ] == "1";
   if ( parameters.count( "grid_overlap.remove_hierarch_percent" ) ) grid_overlap.remove_hierarch_percent = parameters[ "grid_overlap.remove_hierarch_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.remove_sync" ) ) grid_overlap.remove_sync = parameters[ "grid_overlap.remove_sync" ] == "1";
   if ( parameters.count( "grid_overlap.remove_sync_percent" ) ) grid_overlap.remove_sync_percent = parameters[ "grid_overlap.remove_sync_percent" ].toDouble();
   if ( parameters.count( "grid_overlap.translate_out" ) ) grid_overlap.translate_out = parameters[ "grid_overlap.translate_out" ] == "1";
   if ( parameters.count( "grid_overlap.show_translate" ) ) grid_overlap.show_translate = parameters[ "grid_overlap.show_translate" ] == "1";
   if ( parameters.count( "overlap_tolerance" ) ) overlap_tolerance = parameters[ "overlap_tolerance" ].toDouble();
   if ( parameters.count( "bead_output.output" ) ) bead_output.output = parameters[ "bead_output.output" ].toInt();
   if ( parameters.count( "bead_output.sequence" ) ) bead_output.sequence = parameters[ "bead_output.sequence" ].toInt();
   if ( parameters.count( "bead_output.correspondence" ) ) bead_output.correspondence = parameters[ "bead_output.correspondence" ] == "1";
   if ( parameters.count( "asa.probe_radius" ) ) asa.probe_radius = parameters[ "asa.probe_radius" ].toFloat();
   if ( parameters.count( "asa.probe_recheck_radius" ) ) asa.probe_recheck_radius = parameters[ "asa.probe_recheck_radius" ].toFloat();
   if ( parameters.count( "asa.threshold" ) ) asa.threshold = parameters[ "asa.threshold" ].toFloat();
   if ( parameters.count( "asa.threshold_percent" ) ) asa.threshold_percent = parameters[ "asa.threshold_percent" ].toFloat();
   if ( parameters.count( "asa.grid_threshold" ) ) asa.grid_threshold = parameters[ "asa.grid_threshold" ].toFloat();
   if ( parameters.count( "asa.grid_threshold_percent" ) ) asa.grid_threshold_percent = parameters[ "asa.grid_threshold_percent" ].toFloat();
   if ( parameters.count( "asa.calculation" ) ) asa.calculation = parameters[ "asa.calculation" ] == "1";
   if ( parameters.count( "asa.recheck_beads" ) ) asa.recheck_beads = parameters[ "asa.recheck_beads" ] == "1";
   if ( parameters.count( "asa.method" ) ) asa.method = parameters[ "asa.method" ].toInt();
   if ( parameters.count( "asa.asab1_step" ) ) asa.asab1_step = parameters[ "asa.asab1_step" ].toFloat();
   if ( parameters.count( "grid.cubic" ) ) grid.cubic = parameters[ "grid.cubic" ] == "1";
   if ( parameters.count( "grid.hydrate" ) ) grid.hydrate = parameters[ "grid.hydrate" ] == "1";
   if ( parameters.count( "grid.center" ) ) grid.center = parameters[ "grid.center" ].toInt();
   if ( parameters.count( "grid.tangency" ) ) grid.tangency = parameters[ "grid.tangency" ] == "1";
   if ( parameters.count( "grid.cube_side" ) ) grid.cube_side = parameters[ "grid.cube_side" ].toDouble();
   if ( parameters.count( "grid.enable_asa" ) ) grid.enable_asa = parameters[ "grid.enable_asa" ] == "1";
   if ( parameters.count( "misc.hydrovol" ) ) misc.hydrovol = parameters[ "misc.hydrovol" ].toDouble();
   if ( parameters.count( "misc.compute_vbar" ) ) misc.compute_vbar = parameters[ "misc.compute_vbar" ] == "1";
   if ( parameters.count( "misc.vbar" ) ) misc.vbar = parameters[ "misc.vbar" ].toDouble();
   if ( parameters.count( "misc.vbar_temperature" ) ) misc.vbar_temperature = parameters[ "misc.vbar_temperature" ].toDouble();
   if ( parameters.count( "misc.pb_rule_on" ) ) misc.pb_rule_on = parameters[ "misc.pb_rule_on" ] == "1";
   if ( parameters.count( "misc.avg_radius" ) ) misc.avg_radius = parameters[ "misc.avg_radius" ].toDouble();
   if ( parameters.count( "misc.avg_mass" ) ) misc.avg_mass = parameters[ "misc.avg_mass" ].toDouble();
   if ( parameters.count( "misc.avg_hydration" ) ) misc.avg_hydration = parameters[ "misc.avg_hydration" ].toDouble();
   if ( parameters.count( "misc.avg_volume" ) ) misc.avg_volume = parameters[ "misc.avg_volume" ].toDouble();
   if ( parameters.count( "misc.avg_vbar" ) ) misc.avg_vbar = parameters[ "misc.avg_vbar" ].toDouble();
   if ( parameters.count( "hydro.unit" ) ) hydro.unit = parameters[ "hydro.unit" ].toInt();
   if ( parameters.count( "hydro.solvent_name" ) ) hydro.solvent_name = parameters[ "hydro.solvent_name" ];
   if ( parameters.count( "hydro.solvent_acronym" ) ) hydro.solvent_acronym = parameters[ "hydro.solvent_acronym" ];
   if ( parameters.count( "hydro.temperature" ) ) hydro.temperature = parameters[ "hydro.temperature" ].toDouble();
   if ( parameters.count( "hydro.solvent_viscosity" ) ) hydro.solvent_viscosity = parameters[ "hydro.solvent_viscosity" ].toDouble();
   if ( parameters.count( "hydro.solvent_density" ) ) hydro.solvent_density = parameters[ "hydro.solvent_density" ].toDouble();
   if ( parameters.count( "hydro.reference_system" ) ) hydro.reference_system = parameters[ "hydro.reference_system" ] == "1";
   if ( parameters.count( "hydro.boundary_cond" ) ) hydro.boundary_cond = parameters[ "hydro.boundary_cond" ] == "1";
   if ( parameters.count( "hydro.volume_correction" ) ) hydro.volume_correction = parameters[ "hydro.volume_correction" ] == "1";
   if ( parameters.count( "hydro.volume" ) ) hydro.volume = parameters[ "hydro.volume" ].toDouble();
   if ( parameters.count( "hydro.mass_correction" ) ) hydro.mass_correction = parameters[ "hydro.mass_correction" ] == "1";
   if ( parameters.count( "hydro.mass" ) ) hydro.mass = parameters[ "hydro.mass" ].toDouble();
   if ( parameters.count( "hydro.bead_inclusion" ) ) hydro.bead_inclusion = parameters[ "hydro.bead_inclusion" ] == "1";
   if ( parameters.count( "hydro.rotational" ) ) hydro.rotational = parameters[ "hydro.rotational" ] == "1";
   if ( parameters.count( "hydro.viscosity" ) ) hydro.viscosity = parameters[ "hydro.viscosity" ] == "1";
   if ( parameters.count( "hydro.overlap_cutoff" ) ) hydro.overlap_cutoff = parameters[ "hydro.overlap_cutoff" ] == "1";
   if ( parameters.count( "hydro.overlap" ) ) hydro.overlap = parameters[ "hydro.overlap" ].toDouble();
   if ( parameters.count( "pdb_vis.visualization" ) ) pdb_vis.visualization = parameters[ "pdb_vis.visualization" ].toInt();
   if ( parameters.count( "pdb_vis.filename" ) ) pdb_vis.filename = parameters[ "pdb_vis.filename" ];
   if ( parameters.count( "pdb_parse.skip_hydrogen" ) ) pdb_parse.skip_hydrogen = parameters[ "pdb_parse.skip_hydrogen" ] == "1";
   if ( parameters.count( "pdb_parse.skip_water" ) ) pdb_parse.skip_water = parameters[ "pdb_parse.skip_water" ] == "1";
   if ( parameters.count( "pdb_parse.alternate" ) ) pdb_parse.alternate = parameters[ "pdb_parse.alternate" ] == "1";
   if ( parameters.count( "pdb_parse.find_sh" ) ) pdb_parse.find_sh = parameters[ "pdb_parse.find_sh" ] == "1";
   if ( parameters.count( "pdb_parse.missing_residues" ) ) pdb_parse.missing_residues = parameters[ "pdb_parse.missing_residues" ].toInt();
   if ( parameters.count( "pdb_parse.missing_atoms" ) ) pdb_parse.missing_atoms = parameters[ "pdb_parse.missing_atoms" ].toInt();
   if ( parameters.count( "saxs_options.water_e_density" ) ) saxs_options.water_e_density = parameters[ "saxs_options.water_e_density" ].toFloat();
   if ( parameters.count( "saxs_options.h_scat_len" ) ) saxs_options.h_scat_len = parameters[ "saxs_options.h_scat_len" ].toFloat();
   if ( parameters.count( "saxs_options.d_scat_len" ) ) saxs_options.d_scat_len = parameters[ "saxs_options.d_scat_len" ].toFloat();
   if ( parameters.count( "saxs_options.h2o_scat_len_dens" ) ) saxs_options.h2o_scat_len_dens = parameters[ "saxs_options.h2o_scat_len_dens" ].toFloat();
   if ( parameters.count( "saxs_options.d2o_scat_len_dens" ) ) saxs_options.d2o_scat_len_dens = parameters[ "saxs_options.d2o_scat_len_dens" ].toFloat();
   if ( parameters.count( "saxs_options.d2o_conc" ) ) saxs_options.d2o_conc = parameters[ "saxs_options.d2o_conc" ].toFloat();
   if ( parameters.count( "saxs_options.frac_of_exch_pep" ) ) saxs_options.frac_of_exch_pep = parameters[ "saxs_options.frac_of_exch_pep" ].toFloat();
   if ( parameters.count( "saxs_options.wavelength" ) ) saxs_options.wavelength = parameters[ "saxs_options.wavelength" ].toFloat();
   if ( parameters.count( "saxs_options.start_angle" ) ) saxs_options.start_angle = parameters[ "saxs_options.start_angle" ].toFloat();
   if ( parameters.count( "saxs_options.end_angle" ) ) saxs_options.end_angle = parameters[ "saxs_options.end_angle" ].toFloat();
   if ( parameters.count( "saxs_options.delta_angle" ) ) saxs_options.delta_angle = parameters[ "saxs_options.delta_angle" ].toFloat();
   if ( parameters.count( "saxs_options.max_size" ) ) saxs_options.max_size = parameters[ "saxs_options.max_size" ].toFloat();
   if ( parameters.count( "saxs_options.bin_size" ) ) saxs_options.bin_size = parameters[ "saxs_options.bin_size" ].toFloat();
   if ( parameters.count( "saxs_options.hydrate_pdb" ) ) saxs_options.hydrate_pdb = parameters[ "saxs_options.hydrate_pdb" ] == "1";
   if ( parameters.count( "saxs_options.curve" ) ) saxs_options.curve = parameters[ "saxs_options.curve" ].toInt();
   if ( parameters.count( "saxs_options.saxs_sans" ) ) saxs_options.saxs_sans = parameters[ "saxs_options.saxs_sans" ].toInt();
   if ( parameters.count( "bd_options.threshold_pb_pb" ) ) bd_options.threshold_pb_pb = parameters[ "bd_options.threshold_pb_pb" ].toFloat();
   if ( parameters.count( "bd_options.threshold_pb_sc" ) ) bd_options.threshold_pb_sc = parameters[ "bd_options.threshold_pb_sc" ].toFloat();
   if ( parameters.count( "bd_options.threshold_sc_sc" ) ) bd_options.threshold_sc_sc = parameters[ "bd_options.threshold_sc_sc" ].toFloat();
   if ( parameters.count( "bd_options.do_rr" ) ) bd_options.do_rr = parameters[ "bd_options.do_rr" ] == "1";
   if ( parameters.count( "bd_options.force_chem" ) ) bd_options.force_chem = parameters[ "bd_options.force_chem" ] == "1";
   if ( parameters.count( "bd_options.bead_size_type" ) ) bd_options.bead_size_type = parameters[ "bd_options.bead_size_type" ].toInt();
   if ( parameters.count( "bd_options.show_pdb" ) ) bd_options.show_pdb = parameters[ "bd_options.show_pdb" ] == "1";
   if ( parameters.count( "bd_options.run_browflex" ) ) bd_options.run_browflex = parameters[ "bd_options.run_browflex" ] == "1";
   if ( parameters.count( "bd_options.tprev" ) ) bd_options.tprev = parameters[ "bd_options.tprev" ].toDouble();
   if ( parameters.count( "bd_options.ttraj" ) ) bd_options.ttraj = parameters[ "bd_options.ttraj" ].toDouble();
   if ( parameters.count( "bd_options.deltat" ) ) bd_options.deltat = parameters[ "bd_options.deltat" ].toDouble();
   if ( parameters.count( "bd_options.npadif" ) ) bd_options.npadif = parameters[ "bd_options.npadif" ].toInt();
   if ( parameters.count( "bd_options.nconf" ) ) bd_options.nconf = parameters[ "bd_options.nconf" ].toInt();
   if ( parameters.count( "bd_options.inter" ) ) bd_options.inter = parameters[ "bd_options.inter" ].toInt();
   if ( parameters.count( "bd_options.iorder" ) ) bd_options.iorder = parameters[ "bd_options.iorder" ].toInt();
   if ( parameters.count( "bd_options.iseed" ) ) bd_options.iseed = parameters[ "bd_options.iseed" ].toInt();
   if ( parameters.count( "bd_options.icdm" ) ) bd_options.icdm = parameters[ "bd_options.icdm" ].toInt();
   if ( parameters.count( "bd_options.chem_pb_pb_bond_type" ) ) bd_options.chem_pb_pb_bond_type = parameters[ "bd_options.chem_pb_pb_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_force_constant" ) ) bd_options.compute_chem_pb_pb_force_constant = parameters[ "bd_options.compute_chem_pb_pb_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_force_constant" ) ) bd_options.chem_pb_pb_force_constant = parameters[ "bd_options.chem_pb_pb_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_equilibrium_dist" ) ) bd_options.compute_chem_pb_pb_equilibrium_dist = parameters[ "bd_options.compute_chem_pb_pb_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_equilibrium_dist" ) ) bd_options.chem_pb_pb_equilibrium_dist = parameters[ "bd_options.chem_pb_pb_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_pb_max_elong" ) ) bd_options.compute_chem_pb_pb_max_elong = parameters[ "bd_options.compute_chem_pb_pb_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_pb_max_elong" ) ) bd_options.chem_pb_pb_max_elong = parameters[ "bd_options.chem_pb_pb_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.chem_pb_sc_bond_type" ) ) bd_options.chem_pb_sc_bond_type = parameters[ "bd_options.chem_pb_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_force_constant" ) ) bd_options.compute_chem_pb_sc_force_constant = parameters[ "bd_options.compute_chem_pb_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_force_constant" ) ) bd_options.chem_pb_sc_force_constant = parameters[ "bd_options.chem_pb_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_equilibrium_dist" ) ) bd_options.compute_chem_pb_sc_equilibrium_dist = parameters[ "bd_options.compute_chem_pb_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_equilibrium_dist" ) ) bd_options.chem_pb_sc_equilibrium_dist = parameters[ "bd_options.chem_pb_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_pb_sc_max_elong" ) ) bd_options.compute_chem_pb_sc_max_elong = parameters[ "bd_options.compute_chem_pb_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_pb_sc_max_elong" ) ) bd_options.chem_pb_sc_max_elong = parameters[ "bd_options.chem_pb_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.chem_sc_sc_bond_type" ) ) bd_options.chem_sc_sc_bond_type = parameters[ "bd_options.chem_sc_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_force_constant" ) ) bd_options.compute_chem_sc_sc_force_constant = parameters[ "bd_options.compute_chem_sc_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_force_constant" ) ) bd_options.chem_sc_sc_force_constant = parameters[ "bd_options.chem_sc_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_equilibrium_dist" ) ) bd_options.compute_chem_sc_sc_equilibrium_dist = parameters[ "bd_options.compute_chem_sc_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_equilibrium_dist" ) ) bd_options.chem_sc_sc_equilibrium_dist = parameters[ "bd_options.chem_sc_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_chem_sc_sc_max_elong" ) ) bd_options.compute_chem_sc_sc_max_elong = parameters[ "bd_options.compute_chem_sc_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.chem_sc_sc_max_elong" ) ) bd_options.chem_sc_sc_max_elong = parameters[ "bd_options.chem_sc_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.pb_pb_bond_type" ) ) bd_options.pb_pb_bond_type = parameters[ "bd_options.pb_pb_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_pb_pb_force_constant" ) ) bd_options.compute_pb_pb_force_constant = parameters[ "bd_options.compute_pb_pb_force_constant" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_force_constant" ) ) bd_options.pb_pb_force_constant = parameters[ "bd_options.pb_pb_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_pb_equilibrium_dist" ) ) bd_options.compute_pb_pb_equilibrium_dist = parameters[ "bd_options.compute_pb_pb_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_equilibrium_dist" ) ) bd_options.pb_pb_equilibrium_dist = parameters[ "bd_options.pb_pb_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_pb_max_elong" ) ) bd_options.compute_pb_pb_max_elong = parameters[ "bd_options.compute_pb_pb_max_elong" ] == "1";
   if ( parameters.count( "bd_options.pb_pb_max_elong" ) ) bd_options.pb_pb_max_elong = parameters[ "bd_options.pb_pb_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.pb_sc_bond_type" ) ) bd_options.pb_sc_bond_type = parameters[ "bd_options.pb_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_pb_sc_force_constant" ) ) bd_options.compute_pb_sc_force_constant = parameters[ "bd_options.compute_pb_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_force_constant" ) ) bd_options.pb_sc_force_constant = parameters[ "bd_options.pb_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_sc_equilibrium_dist" ) ) bd_options.compute_pb_sc_equilibrium_dist = parameters[ "bd_options.compute_pb_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_equilibrium_dist" ) ) bd_options.pb_sc_equilibrium_dist = parameters[ "bd_options.pb_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_pb_sc_max_elong" ) ) bd_options.compute_pb_sc_max_elong = parameters[ "bd_options.compute_pb_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.pb_sc_max_elong" ) ) bd_options.pb_sc_max_elong = parameters[ "bd_options.pb_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.sc_sc_bond_type" ) ) bd_options.sc_sc_bond_type = parameters[ "bd_options.sc_sc_bond_type" ].toInt();
   if ( parameters.count( "bd_options.compute_sc_sc_force_constant" ) ) bd_options.compute_sc_sc_force_constant = parameters[ "bd_options.compute_sc_sc_force_constant" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_force_constant" ) ) bd_options.sc_sc_force_constant = parameters[ "bd_options.sc_sc_force_constant" ].toFloat();
   if ( parameters.count( "bd_options.compute_sc_sc_equilibrium_dist" ) ) bd_options.compute_sc_sc_equilibrium_dist = parameters[ "bd_options.compute_sc_sc_equilibrium_dist" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_equilibrium_dist" ) ) bd_options.sc_sc_equilibrium_dist = parameters[ "bd_options.sc_sc_equilibrium_dist" ].toFloat();
   if ( parameters.count( "bd_options.compute_sc_sc_max_elong" ) ) bd_options.compute_sc_sc_max_elong = parameters[ "bd_options.compute_sc_sc_max_elong" ] == "1";
   if ( parameters.count( "bd_options.sc_sc_max_elong" ) ) bd_options.sc_sc_max_elong = parameters[ "bd_options.sc_sc_max_elong" ].toFloat();
   if ( parameters.count( "bd_options.nmol" ) ) bd_options.nmol = parameters[ "bd_options.nmol" ].toInt();
   if ( parameters.count( "anaflex_options.run_anaflex" ) ) anaflex_options.run_anaflex = parameters[ "anaflex_options.run_anaflex" ] == "1";
   if ( parameters.count( "anaflex_options.nfrec" ) ) anaflex_options.nfrec = parameters[ "anaflex_options.nfrec" ].toInt();
   if ( parameters.count( "anaflex_options.instprofiles" ) ) anaflex_options.instprofiles = parameters[ "anaflex_options.instprofiles" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1" ) ) anaflex_options.run_mode_1 = parameters[ "anaflex_options.run_mode_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_1" ) ) anaflex_options.run_mode_1_1 = parameters[ "anaflex_options.run_mode_1_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_2" ) ) anaflex_options.run_mode_1_2 = parameters[ "anaflex_options.run_mode_1_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_3" ) ) anaflex_options.run_mode_1_3 = parameters[ "anaflex_options.run_mode_1_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_4" ) ) anaflex_options.run_mode_1_4 = parameters[ "anaflex_options.run_mode_1_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_5" ) ) anaflex_options.run_mode_1_5 = parameters[ "anaflex_options.run_mode_1_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_7" ) ) anaflex_options.run_mode_1_7 = parameters[ "anaflex_options.run_mode_1_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_8" ) ) anaflex_options.run_mode_1_8 = parameters[ "anaflex_options.run_mode_1_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_12" ) ) anaflex_options.run_mode_1_12 = parameters[ "anaflex_options.run_mode_1_12" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_13" ) ) anaflex_options.run_mode_1_13 = parameters[ "anaflex_options.run_mode_1_13" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_14" ) ) anaflex_options.run_mode_1_14 = parameters[ "anaflex_options.run_mode_1_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_18" ) ) anaflex_options.run_mode_1_18 = parameters[ "anaflex_options.run_mode_1_18" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_20" ) ) anaflex_options.run_mode_1_20 = parameters[ "anaflex_options.run_mode_1_20" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_1_24" ) ) anaflex_options.run_mode_1_24 = parameters[ "anaflex_options.run_mode_1_24" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2" ) ) anaflex_options.run_mode_2 = parameters[ "anaflex_options.run_mode_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_1" ) ) anaflex_options.run_mode_2_1 = parameters[ "anaflex_options.run_mode_2_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_2" ) ) anaflex_options.run_mode_2_2 = parameters[ "anaflex_options.run_mode_2_2" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_3" ) ) anaflex_options.run_mode_2_3 = parameters[ "anaflex_options.run_mode_2_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_4" ) ) anaflex_options.run_mode_2_4 = parameters[ "anaflex_options.run_mode_2_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_5" ) ) anaflex_options.run_mode_2_5 = parameters[ "anaflex_options.run_mode_2_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_7" ) ) anaflex_options.run_mode_2_7 = parameters[ "anaflex_options.run_mode_2_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_8" ) ) anaflex_options.run_mode_2_8 = parameters[ "anaflex_options.run_mode_2_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_12" ) ) anaflex_options.run_mode_2_12 = parameters[ "anaflex_options.run_mode_2_12" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_13" ) ) anaflex_options.run_mode_2_13 = parameters[ "anaflex_options.run_mode_2_13" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_14" ) ) anaflex_options.run_mode_2_14 = parameters[ "anaflex_options.run_mode_2_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_18" ) ) anaflex_options.run_mode_2_18 = parameters[ "anaflex_options.run_mode_2_18" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_20" ) ) anaflex_options.run_mode_2_20 = parameters[ "anaflex_options.run_mode_2_20" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_2_24" ) ) anaflex_options.run_mode_2_24 = parameters[ "anaflex_options.run_mode_2_24" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3" ) ) anaflex_options.run_mode_3 = parameters[ "anaflex_options.run_mode_3" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_1" ) ) anaflex_options.run_mode_3_1 = parameters[ "anaflex_options.run_mode_3_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_5" ) ) anaflex_options.run_mode_3_5 = parameters[ "anaflex_options.run_mode_3_5" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_9" ) ) anaflex_options.run_mode_3_9 = parameters[ "anaflex_options.run_mode_3_9" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_10" ) ) anaflex_options.run_mode_3_10 = parameters[ "anaflex_options.run_mode_3_10" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_14" ) ) anaflex_options.run_mode_3_14 = parameters[ "anaflex_options.run_mode_3_14" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_15" ) ) anaflex_options.run_mode_3_15 = parameters[ "anaflex_options.run_mode_3_15" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_3_16" ) ) anaflex_options.run_mode_3_16 = parameters[ "anaflex_options.run_mode_3_16" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4" ) ) anaflex_options.run_mode_4 = parameters[ "anaflex_options.run_mode_4" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_1" ) ) anaflex_options.run_mode_4_1 = parameters[ "anaflex_options.run_mode_4_1" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_6" ) ) anaflex_options.run_mode_4_6 = parameters[ "anaflex_options.run_mode_4_6" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_7" ) ) anaflex_options.run_mode_4_7 = parameters[ "anaflex_options.run_mode_4_7" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_4_8" ) ) anaflex_options.run_mode_4_8 = parameters[ "anaflex_options.run_mode_4_8" ] == "1";
   if ( parameters.count( "anaflex_options.run_mode_9" ) ) anaflex_options.run_mode_9 = parameters[ "anaflex_options.run_mode_9" ] == "1";
   if ( parameters.count( "anaflex_options.ntimc" ) ) anaflex_options.ntimc = parameters[ "anaflex_options.ntimc" ].toInt();
   if ( parameters.count( "anaflex_options.tmax" ) ) anaflex_options.tmax = parameters[ "anaflex_options.tmax" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_5_iii" ) ) anaflex_options.run_mode_3_5_iii = parameters[ "anaflex_options.run_mode_3_5_iii" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_5_jjj" ) ) anaflex_options.run_mode_3_5_jjj = parameters[ "anaflex_options.run_mode_3_5_jjj" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_10_theta" ) ) anaflex_options.run_mode_3_10_theta = parameters[ "anaflex_options.run_mode_3_10_theta" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_10_refractive_index" ) ) anaflex_options.run_mode_3_10_refractive_index = parameters[ "anaflex_options.run_mode_3_10_refractive_index" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_10_lambda" ) ) anaflex_options.run_mode_3_10_lambda = parameters[ "anaflex_options.run_mode_3_10_lambda" ].toFloat();
   if ( parameters.count( "anaflex_options.run_mode_3_14_iii" ) ) anaflex_options.run_mode_3_14_iii = parameters[ "anaflex_options.run_mode_3_14_iii" ].toInt();
   if ( parameters.count( "anaflex_options.run_mode_3_14_jjj" ) ) anaflex_options.run_mode_3_14_jjj = parameters[ "anaflex_options.run_mode_3_14_jjj" ].toInt();
   if ( parameters.count( "batch.missing_atoms" ) ) batch.missing_atoms = parameters[ "batch.missing_atoms" ].toInt();
   if ( parameters.count( "batch.missing_residues" ) ) batch.missing_residues = parameters[ "batch.missing_residues" ].toInt();
   if ( parameters.count( "batch.somo" ) ) batch.somo = parameters[ "batch.somo" ] == "1";
   if ( parameters.count( "batch.grid" ) ) batch.grid = parameters[ "batch.grid" ] == "1";
   if ( parameters.count( "batch.hydro" ) ) batch.hydro = parameters[ "batch.hydro" ] == "1";
   if ( parameters.count( "batch.avg_hydro" ) ) batch.avg_hydro = parameters[ "batch.avg_hydro" ] == "1";
   if ( parameters.count( "batch.avg_hydro_name" ) ) batch.avg_hydro_name = parameters[ "batch.avg_hydro_name" ];
   if ( parameters.count( "batch.height" ) ) batch.height = parameters[ "batch.height" ].toInt();
   if ( parameters.count( "batch.width" ) ) batch.width = parameters[ "batch.width" ].toInt();
   if ( parameters.count( "path_load_pdb" ) ) path_load_pdb = parameters[ "path_load_pdb" ];
   if ( parameters.count( "path_view_pdb" ) ) path_view_pdb = parameters[ "path_view_pdb" ];
   if ( parameters.count( "path_load_bead_model" ) ) path_load_bead_model = parameters[ "path_load_bead_model" ];
   if ( parameters.count( "path_view_asa_res" ) ) path_view_asa_res = parameters[ "path_view_asa_res" ];
   if ( parameters.count( "path_view_bead_model" ) ) path_view_bead_model = parameters[ "path_view_bead_model" ];
   if ( parameters.count( "path_open_hydro_res" ) ) path_open_hydro_res = parameters[ "path_open_hydro_res" ];
   if ( parameters.count( "saxs_options.path_load_saxs_curve" ) ) saxs_options.path_load_saxs_curve = parameters[ "saxs_options.path_load_saxs_curve" ];
   if ( parameters.count( "saxs_options.path_load_gnom" ) ) saxs_options.path_load_gnom = parameters[ "saxs_options.path_load_gnom" ];
   if ( parameters.count( "saxs_options.path_load_prr" ) ) saxs_options.path_load_prr = parameters[ "saxs_options.path_load_prr" ];
   if ( parameters.count( "asa.hydrate_probe_radius" ) ) asa.hydrate_probe_radius = parameters[ "asa.hydrate_probe_radius" ].toFloat();
   if ( parameters.count( "asa.hydrate_threshold" ) ) asa.hydrate_threshold = parameters[ "asa.hydrate_threshold" ].toFloat();
   if ( parameters.count( "misc.target_e_density" ) ) misc.target_e_density = parameters[ "misc.target_e_density" ].toDouble();
   if ( parameters.count( "misc.target_volume" ) ) misc.target_volume = parameters[ "misc.target_volume" ].toDouble();
   if ( parameters.count( "misc.set_target_on_load_pdb" ) ) misc.set_target_on_load_pdb = parameters[ "misc.set_target_on_load_pdb" ] == "1";
   if ( parameters.count( "misc.equalize_radii" ) ) misc.equalize_radii = parameters[ "misc.equalize_radii" ] == "1";
   if ( parameters.count( "dmd_options.force_chem" ) ) dmd_options.force_chem = parameters[ "dmd_options.force_chem" ] == "1";
   if ( parameters.count( "dmd_options.pdb_static_pairs" ) ) dmd_options.pdb_static_pairs = parameters[ "dmd_options.pdb_static_pairs" ] == "1";
   if ( parameters.count( "dmd_options.threshold_pb_pb" ) ) dmd_options.threshold_pb_pb = parameters[ "dmd_options.threshold_pb_pb" ].toFloat();
   if ( parameters.count( "dmd_options.threshold_pb_sc" ) ) dmd_options.threshold_pb_sc = parameters[ "dmd_options.threshold_pb_sc" ].toFloat();
   if ( parameters.count( "dmd_options.threshold_sc_sc" ) ) dmd_options.threshold_sc_sc = parameters[ "dmd_options.threshold_sc_sc" ].toFloat();
   if ( parameters.count( "saxs_options.normalize_by_mw" ) ) saxs_options.normalize_by_mw = parameters[ "saxs_options.normalize_by_mw" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_debye" ) ) saxs_options.saxs_iq_native_debye = parameters[ "saxs_options.saxs_iq_native_debye" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid" ) ) saxs_options.saxs_iq_native_hybrid = parameters[ "saxs_options.saxs_iq_native_hybrid" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid2" ) ) saxs_options.saxs_iq_native_hybrid2 = parameters[ "saxs_options.saxs_iq_native_hybrid2" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_hybrid3" ) ) saxs_options.saxs_iq_native_hybrid3 = parameters[ "saxs_options.saxs_iq_native_hybrid3" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_fast" ) ) saxs_options.saxs_iq_native_fast = parameters[ "saxs_options.saxs_iq_native_fast" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_fast_compute_pr" ) ) saxs_options.saxs_iq_native_fast_compute_pr = parameters[ "saxs_options.saxs_iq_native_fast_compute_pr" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_foxs" ) ) saxs_options.saxs_iq_foxs = parameters[ "saxs_options.saxs_iq_foxs" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_crysol" ) ) saxs_options.saxs_iq_crysol = parameters[ "saxs_options.saxs_iq_crysol" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_debye" ) ) saxs_options.sans_iq_native_debye = parameters[ "saxs_options.sans_iq_native_debye" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid" ) ) saxs_options.sans_iq_native_hybrid = parameters[ "saxs_options.sans_iq_native_hybrid" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid2" ) ) saxs_options.sans_iq_native_hybrid2 = parameters[ "saxs_options.sans_iq_native_hybrid2" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_hybrid3" ) ) saxs_options.sans_iq_native_hybrid3 = parameters[ "saxs_options.sans_iq_native_hybrid3" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_fast" ) ) saxs_options.sans_iq_native_fast = parameters[ "saxs_options.sans_iq_native_fast" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_fast_compute_pr" ) ) saxs_options.sans_iq_native_fast_compute_pr = parameters[ "saxs_options.sans_iq_native_fast_compute_pr" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_cryson" ) ) saxs_options.sans_iq_cryson = parameters[ "saxs_options.sans_iq_cryson" ] == "1";
   if ( parameters.count( "saxs_options.hybrid2_q_points" ) ) saxs_options.hybrid2_q_points = parameters[ "saxs_options.hybrid2_q_points" ].toUInt();
   if ( parameters.count( "saxs_options.iq_ask" ) ) saxs_options.iq_ask = parameters[ "saxs_options.iq_ask" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_ask" ) ) saxs_options.iq_scale_ask = parameters[ "saxs_options.iq_scale_ask" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_angstrom" ) ) saxs_options.iq_scale_angstrom = parameters[ "saxs_options.iq_scale_angstrom" ] == "1";
   if ( parameters.count( "saxs_options.iq_scale_nm" ) ) saxs_options.iq_scale_nm = parameters[ "saxs_options.iq_scale_nm" ] == "1";
   if ( parameters.count( "saxs_options.sh_max_harmonics" ) ) saxs_options.sh_max_harmonics = parameters[ "saxs_options.sh_max_harmonics" ].toDouble();
   if ( parameters.count( "saxs_options.sh_fibonacci_grid_order" ) ) saxs_options.sh_fibonacci_grid_order = parameters[ "saxs_options.sh_fibonacci_grid_order" ].toDouble();
   if ( parameters.count( "saxs_options.crysol_hydration_shell_contrast" ) ) saxs_options.crysol_hydration_shell_contrast = parameters[ "saxs_options.crysol_hydration_shell_contrast" ].toFloat();
   if ( parameters.count( "saxs_options.crysol_default_load_difference_intensity" ) ) saxs_options.crysol_default_load_difference_intensity = parameters[ "saxs_options.crysol_default_load_difference_intensity" ] == "1";
   if ( parameters.count( "saxs_options.crysol_version_26" ) ) saxs_options.crysol_version_26 = parameters[ "saxs_options.crysol_version_26" ] == "1";
   if ( parameters.count( "saxs_options.fast_bin_size" ) ) saxs_options.fast_bin_size = parameters[ "saxs_options.fast_bin_size" ].toFloat();
   if ( parameters.count( "saxs_options.fast_modulation" ) ) saxs_options.fast_modulation = parameters[ "saxs_options.fast_modulation" ].toFloat();
   if ( parameters.count( "saxs_options.compute_saxs_coeff_for_bead_models" ) ) saxs_options.compute_saxs_coeff_for_bead_models = parameters[ "saxs_options.compute_saxs_coeff_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.compute_sans_coeff_for_bead_models" ) ) saxs_options.compute_sans_coeff_for_bead_models = parameters[ "saxs_options.compute_sans_coeff_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.default_atom_filename" ) ) saxs_options.default_atom_filename = parameters[ "saxs_options.default_atom_filename" ];
   if ( parameters.count( "saxs_options.default_hybrid_filename" ) ) saxs_options.default_hybrid_filename = parameters[ "saxs_options.default_hybrid_filename" ];
   if ( parameters.count( "saxs_options.default_saxs_filename" ) ) saxs_options.default_saxs_filename = parameters[ "saxs_options.default_saxs_filename" ];
   if ( parameters.count( "saxs_options.default_rotamer_filename" ) ) saxs_options.default_rotamer_filename = parameters[ "saxs_options.default_rotamer_filename" ];
   if ( parameters.count( "saxs_options.steric_clash_distance" ) ) saxs_options.steric_clash_distance = parameters[ "saxs_options.steric_clash_distance" ].toDouble();
   if ( parameters.count( "saxs_options.steric_clash_recheck_distance" ) ) saxs_options.steric_clash_recheck_distance = parameters[ "saxs_options.steric_clash_recheck_distance" ].toDouble();
   if ( parameters.count( "saxs_options.disable_iq_scaling" ) ) saxs_options.disable_iq_scaling = parameters[ "saxs_options.disable_iq_scaling" ] == "1";
   if ( parameters.count( "saxs_options.autocorrelate" ) ) saxs_options.autocorrelate = parameters[ "saxs_options.autocorrelate" ] == "1";
   if ( parameters.count( "saxs_options.hybrid_radius_excl_vol" ) ) saxs_options.hybrid_radius_excl_vol = parameters[ "saxs_options.hybrid_radius_excl_vol" ] == "1";
   if ( parameters.count( "saxs_options.scale_excl_vol" ) ) saxs_options.scale_excl_vol = parameters[ "saxs_options.scale_excl_vol" ].toFloat();
   if ( parameters.count( "saxs_options.subtract_radius" ) ) saxs_options.subtract_radius = parameters[ "saxs_options.subtract_radius" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_minq" ) ) saxs_options.iqq_scale_minq = parameters[ "saxs_options.iqq_scale_minq" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_scale_maxq" ) ) saxs_options.iqq_scale_maxq = parameters[ "saxs_options.iqq_scale_maxq" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_scale_nnls" ) ) saxs_options.iqq_scale_nnls = parameters[ "saxs_options.iqq_scale_nnls" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_linear_offset" ) ) saxs_options.iqq_scale_linear_offset = parameters[ "saxs_options.iqq_scale_linear_offset" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_chi2_fitting" ) ) saxs_options.iqq_scale_chi2_fitting = parameters[ "saxs_options.iqq_scale_chi2_fitting" ] == "1";
   if ( parameters.count( "saxs_options.iqq_expt_data_contains_variances" ) ) saxs_options.iqq_expt_data_contains_variances = parameters[ "saxs_options.iqq_expt_data_contains_variances" ] == "1";
   if ( parameters.count( "saxs_options.iqq_ask_target_grid" ) ) saxs_options.iqq_ask_target_grid = parameters[ "saxs_options.iqq_ask_target_grid" ] == "1";
   if ( parameters.count( "saxs_options.iqq_scale_play" ) ) saxs_options.iqq_scale_play = parameters[ "saxs_options.iqq_scale_play" ] == "1";
   if ( parameters.count( "saxs_options.swh_excl_vol" ) ) saxs_options.swh_excl_vol = parameters[ "saxs_options.swh_excl_vol" ].toFloat();
   if ( parameters.count( "saxs_options.iqq_default_scaling_target" ) ) saxs_options.iqq_default_scaling_target = parameters[ "saxs_options.iqq_default_scaling_target" ];
   if ( parameters.count( "saxs_options.saxs_iq_hybrid_adaptive" ) ) saxs_options.saxs_iq_hybrid_adaptive = parameters[ "saxs_options.saxs_iq_hybrid_adaptive" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_hybrid_adaptive" ) ) saxs_options.sans_iq_hybrid_adaptive = parameters[ "saxs_options.sans_iq_hybrid_adaptive" ] == "1";
   if ( parameters.count( "saxs_options.bead_model_rayleigh" ) ) saxs_options.bead_model_rayleigh = parameters[ "saxs_options.bead_model_rayleigh" ] == "1";
   if ( parameters.count( "saxs_options.iqq_log_fitting" ) ) saxs_options.iqq_log_fitting = parameters[ "saxs_options.iqq_log_fitting" ] == "1";
   if ( parameters.count( "saxs_options.iqq_kratky_fit" ) ) saxs_options.iqq_kratky_fit = parameters[ "saxs_options.iqq_kratky_fit" ] == "1";
   if ( parameters.count( "saxs_options.iqq_use_atomic_ff" ) ) saxs_options.iqq_use_atomic_ff = parameters[ "saxs_options.iqq_use_atomic_ff" ] == "1";
   if ( parameters.count( "saxs_options.iqq_use_saxs_excl_vol" ) ) saxs_options.iqq_use_saxs_excl_vol = parameters[ "saxs_options.iqq_use_saxs_excl_vol" ] == "1";
   if ( parameters.count( "saxs_options.alt_hydration" ) ) saxs_options.alt_hydration = parameters[ "saxs_options.alt_hydration" ] == "1";
   if ( parameters.count( "saxs_options.xsr_symmop" ) ) saxs_options.xsr_symmop = parameters[ "saxs_options.xsr_symmop" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_nx" ) ) saxs_options.xsr_nx = parameters[ "saxs_options.xsr_nx" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_ny" ) ) saxs_options.xsr_ny = parameters[ "saxs_options.xsr_ny" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_griddistance" ) ) saxs_options.xsr_griddistance = parameters[ "saxs_options.xsr_griddistance" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_ncomponents" ) ) saxs_options.xsr_ncomponents = parameters[ "saxs_options.xsr_ncomponents" ].toUInt();
   if ( parameters.count( "saxs_options.xsr_compactness_weight" ) ) saxs_options.xsr_compactness_weight = parameters[ "saxs_options.xsr_compactness_weight" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_looseness_weight" ) ) saxs_options.xsr_looseness_weight = parameters[ "saxs_options.xsr_looseness_weight" ].toDouble();
   if ( parameters.count( "saxs_options.xsr_temperature" ) ) saxs_options.xsr_temperature = parameters[ "saxs_options.xsr_temperature" ].toDouble();
   if ( parameters.count( "hydro.zeno_zeno" ) ) hydro.zeno_zeno = parameters[ "hydro.zeno_zeno" ] == "1";
   if ( parameters.count( "hydro.zeno_interior" ) ) hydro.zeno_interior = parameters[ "hydro.zeno_interior" ] == "1";
   if ( parameters.count( "hydro.zeno_surface" ) ) hydro.zeno_surface = parameters[ "hydro.zeno_surface" ] == "1";
   if ( parameters.count( "hydro.zeno_zeno_steps" ) ) hydro.zeno_zeno_steps = parameters[ "hydro.zeno_zeno_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_interior_steps" ) ) hydro.zeno_interior_steps = parameters[ "hydro.zeno_interior_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_surface_steps" ) ) hydro.zeno_surface_steps = parameters[ "hydro.zeno_surface_steps" ].toUInt();
   if ( parameters.count( "hydro.zeno_surface_thickness" ) ) hydro.zeno_surface_thickness = parameters[ "hydro.zeno_surface_thickness" ].toFloat();
   if ( parameters.count( "misc.hydro_supc" ) ) misc.hydro_supc = parameters[ "misc.hydro_supc" ] == "1";
   if ( parameters.count( "misc.hydro_zeno" ) ) misc.hydro_zeno = parameters[ "misc.hydro_zeno" ] == "1";
   if ( parameters.count( "batch.saxs_search" ) ) batch.saxs_search = parameters[ "batch.saxs_search" ] == "1";
   if ( parameters.count( "batch.zeno" ) ) batch.zeno = parameters[ "batch.zeno" ] == "1";

   if ( parameters.count( "saxs_options.ignore_errors" ) ) saxs_options.ignore_errors = parameters[ "saxs_options.ignore_errors" ] == "1";
   if ( parameters.count( "saxs_options.alt_ff" ) ) saxs_options.alt_ff = parameters[ "saxs_options.alt_ff" ] == "1";
   if ( parameters.count( "saxs_options.crysol_explicit_hydrogens" ) ) saxs_options.crysol_explicit_hydrogens = parameters[ "saxs_options.crysol_explicit_hydrogens" ] == "1";
   if ( parameters.count( "saxs_options.use_somo_ff" ) ) saxs_options.use_somo_ff = parameters[ "saxs_options.use_somo_ff" ] == "1";
   if ( parameters.count( "saxs_options.five_term_gaussians" ) ) saxs_options.five_term_gaussians = parameters[ "saxs_options.five_term_gaussians" ] == "1";
   if ( parameters.count( "saxs_options.iq_exact_q" ) ) saxs_options.iq_exact_q = parameters[ "saxs_options.iq_exact_q" ] == "1";
   if ( parameters.count( "saxs_options.use_iq_target_ev" ) ) saxs_options.use_iq_target_ev = parameters[ "saxs_options.use_iq_target_ev" ] == "1";
   if ( parameters.count( "saxs_options.set_iq_target_ev_from_vbar" ) ) saxs_options.set_iq_target_ev_from_vbar = parameters[ "saxs_options.set_iq_target_ev_from_vbar" ] == "1";
   if ( parameters.count( "saxs_options.iq_target_ev" ) ) saxs_options.iq_target_ev = parameters[ "saxs_options.iq_target_ev" ].toDouble();
   if ( parameters.count( "saxs_options.hydration_rev_asa" ) ) saxs_options.hydration_rev_asa = parameters[ "saxs_options.hydration_rev_asa" ] == "1";
   if ( parameters.count( "saxs_options.compute_exponentials" ) ) saxs_options.compute_exponentials = parameters[ "saxs_options.compute_exponentials" ] == "1";
   if ( parameters.count( "saxs_options.compute_exponential_terms" ) ) saxs_options.compute_exponential_terms = parameters[ "saxs_options.compute_exponential_terms" ].toUInt();
   if ( parameters.count( "saxs_options.dummy_saxs_name" ) ) saxs_options.dummy_saxs_name = parameters[ "saxs_options.dummy_saxs_name" ];
   if ( parameters.count( "saxs_options.multiply_iq_by_atomic_volume" ) ) saxs_options.multiply_iq_by_atomic_volume = parameters[ "saxs_options.multiply_iq_by_atomic_volume" ] == "1";
   if ( parameters.count( "saxs_options.dummy_atom_pdbs_in_nm" ) ) saxs_options.dummy_atom_pdbs_in_nm = parameters[ "saxs_options.dummy_atom_pdbs_in_nm" ] == "1";
   if ( parameters.count( "saxs_options.iq_global_avg_for_bead_models" ) ) saxs_options.iq_global_avg_for_bead_models = parameters[ "saxs_options.iq_global_avg_for_bead_models" ] == "1";
   if ( parameters.count( "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ) ) saxs_options.apply_loaded_sf_repeatedly_to_pdb = parameters[ "saxs_options.apply_loaded_sf_repeatedly_to_pdb" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_var_len_sf" ) ) saxs_options.bead_models_use_var_len_sf = parameters[ "saxs_options.bead_models_use_var_len_sf" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_var_len_sf_max" ) ) saxs_options.bead_models_var_len_sf_max = parameters[ "saxs_options.bead_models_var_len_sf_max" ].toUInt();
   if ( parameters.count( "saxs_options.bead_models_use_gsm_fitting" ) ) saxs_options.bead_models_use_gsm_fitting = parameters[ "saxs_options.bead_models_use_gsm_fitting" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_quick_fitting" ) ) saxs_options.bead_models_use_quick_fitting = parameters[ "saxs_options.bead_models_use_quick_fitting" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_use_bead_radius_ev" ) ) saxs_options.bead_models_use_bead_radius_ev = parameters[ "saxs_options.bead_models_use_bead_radius_ev" ] == "1";
   if ( parameters.count( "saxs_options.bead_models_rho0_in_scat_factors" ) ) saxs_options.bead_models_rho0_in_scat_factors = parameters[ "saxs_options.bead_models_rho0_in_scat_factors" ] == "1";
   if ( parameters.count( "saxs_options.smooth" ) ) saxs_options.smooth = parameters[ "saxs_options.smooth" ].toUInt();
   if ( parameters.count( "saxs_options.ev_exp_mult" ) ) saxs_options.ev_exp_mult = parameters[ "saxs_options.ev_exp_mult" ].toDouble();
   if ( parameters.count( "saxs_options.sastbx_method" ) ) saxs_options.sastbx_method = parameters[ "saxs_options.sastbx_method" ].toUInt();
   if ( parameters.count( "saxs_options.saxs_iq_sastbx" ) ) saxs_options.saxs_iq_sastbx = parameters[ "saxs_options.saxs_iq_sastbx" ] == "1";
   if ( parameters.count( "saxs_options.saxs_iq_native_sh" ) ) saxs_options.saxs_iq_native_sh = parameters[ "saxs_options.saxs_iq_native_sh" ] == "1";
   if ( parameters.count( "saxs_options.sans_iq_native_sh" ) ) saxs_options.sans_iq_native_sh = parameters[ "saxs_options.sans_iq_native_sh" ] == "1";
   if ( parameters.count( "saxs_options.alt_sh1" ) ) saxs_options.alt_sh1 = parameters[ "saxs_options.alt_sh1" ] == "1";
   if ( parameters.count( "saxs_options.alt_sh2" ) ) saxs_options.alt_sh2 = parameters[ "saxs_options.alt_sh2" ] == "1";
   if ( parameters.count( "grid.create_nmr_bead_pdb" ) ) grid.create_nmr_bead_pdb = parameters[ "grid.create_nmr_bead_pdb" ] == "1";
   if ( parameters.count( "batch.compute_iq_only_avg" ) ) batch.compute_iq_only_avg = parameters[ "batch.compute_iq_only_avg" ] == "1";
   if ( parameters.count( "asa.vvv" ) ) asa.vvv = parameters[ "asa.vvv" ] == "1";
   if ( parameters.count( "asa.vvv_probe_radius" ) ) asa.vvv_probe_radius = parameters[ "asa.vvv_probe_radius" ].toFloat();
   if ( parameters.count( "asa.vvv_grid_dR" ) ) asa.vvv_grid_dR = parameters[ "asa.vvv_grid_dR" ].toFloat();
   if ( parameters.count( "misc.export_msroll" ) ) misc.export_msroll = parameters[ "misc.export_msroll" ] == "1";

   if ( parameters.count( "saxs_options.qstart" ) ) saxs_options.qstart = parameters[ "saxs_options.qstart" ].toDouble();
   if ( parameters.count( "saxs_options.qend" ) ) saxs_options.qend = parameters[ "saxs_options.qend" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_csv" ) ) saxs_options.guinier_csv = parameters[ "saxs_options.guinier_csv" ] == "1";
   if ( parameters.count( "saxs_options.guinier_csv_filename" ) ) saxs_options.guinier_csv_filename = parameters[ "saxs_options.guinier_csv_filename" ];
   if ( parameters.count( "saxs_options.qRgmax" ) ) saxs_options.qRgmax = parameters[ "saxs_options.qRgmax" ].toDouble();
   if ( parameters.count( "saxs_options.pointsmin" ) ) saxs_options.pointsmin = parameters[ "saxs_options.pointsmin" ].toUInt();
   if ( parameters.count( "saxs_options.pointsmax" ) ) saxs_options.pointsmax = parameters[ "saxs_options.pointsmax" ].toUInt();

   if ( parameters.count( "saxs_options.cs_qRgmax" ) ) saxs_options.cs_qRgmax = parameters[ "saxs_options.cs_qRgmax" ].toDouble();
   if ( parameters.count( "saxs_options.cs_qstart" ) ) saxs_options.cs_qstart = parameters[ "saxs_options.cs_qstart" ].toDouble();
   if ( parameters.count( "saxs_options.cs_qend" ) ) saxs_options.cs_qend = parameters[ "saxs_options.cs_qend" ].toDouble();
   if ( parameters.count( "saxs_options.conc" ) ) saxs_options.conc = parameters[ "saxs_options.conc" ].toDouble();
   if ( parameters.count( "saxs_options.psv" ) ) saxs_options.psv = parameters[ "saxs_options.psv" ].toDouble();
   if ( parameters.count( "saxs_options.use_cs_psv" ) ) saxs_options.use_cs_psv = parameters[ "saxs_options.use_cs_psv" ] == "1";
   if ( parameters.count( "saxs_options.cs_psv" ) ) saxs_options.cs_psv = parameters[ "saxs_options.cs_psv" ].toDouble();
   if ( parameters.count( "saxs_options.I0_exp" ) ) saxs_options.I0_exp = parameters[ "saxs_options.I0_exp" ].toDouble();
   if ( parameters.count( "saxs_options.I0_theo" ) ) saxs_options.I0_theo = parameters[ "saxs_options.I0_theo" ].toDouble();
   if ( parameters.count( "saxs_options.diffusion_len" ) ) saxs_options.diffusion_len = parameters[ "saxs_options.diffusion_len" ].toDouble();
   if ( parameters.count( "saxs_options.nucleon_mass" ) ) saxs_options.nucleon_mass = parameters[ "saxs_options.nucleon_mass" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_outlier_reject" ) ) saxs_options.guinier_outlier_reject = parameters[ "saxs_options.guinier_outlier_reject" ] == "1";
   if ( parameters.count( "saxs_options.guinier_outlier_reject_dist" ) ) saxs_options.guinier_outlier_reject_dist = parameters[ "saxs_options.guinier_outlier_reject_dist" ].toDouble();
   if ( parameters.count( "saxs_options.guinier_use_sd" ) ) saxs_options.guinier_use_sd = parameters[ "saxs_options.guinier_use_sd" ] == "1";
   if ( parameters.count( "saxs_options.guinier_use_standards" ) ) saxs_options.guinier_use_standards = parameters[ "saxs_options.guinier_use_standards" ] == "1";

   if ( parameters.count( "saxs_options.cryson_sh_max_harmonics" ) ) saxs_options.cryson_sh_max_harmonics = parameters[ "saxs_options.cryson_sh_max_harmonics" ].toUInt();
   if ( parameters.count( "saxs_options.cryson_sh_fibonacci_grid_order" ) ) saxs_options.cryson_sh_fibonacci_grid_order = parameters[ "saxs_options.cryson_sh_fibonacci_grid_order" ].toUInt();
   if ( parameters.count( "saxs_options.cryson_hydration_shell_contrast" ) ) saxs_options.cryson_hydration_shell_contrast = parameters[ "saxs_options.cryson_hydration_shell_contrast" ].toFloat();
   if ( parameters.count( "saxs_options.cryson_manual_hs" ) ) saxs_options.cryson_manual_hs = parameters[ "saxs_options.cryson_manual_hs" ] == "1";

   // vectors to read:

   saxs_options.dummy_saxs_names.clear();
   if ( parameters.count( "saxs_options.dummy_saxs_names" ) )
   {
      QStringList qsl_tmp = QStringList::split( "\n", parameters[ "saxs_options.dummy_saxs_names" ] );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         saxs_options.dummy_saxs_names.push_back( qsl_tmp[ i ] );
      }
   }

   batch.file.clear();
   if ( parameters.count( "batch.file" ) )
   {
      QStringList qsl_tmp = QStringList::split( "\n", parameters[ "batch.file" ] );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         batch.file.push_back( qsl_tmp[ i ] );
      }
   }

   save_params.field.clear();
   if ( parameters.count( "save_params.field" ) )
   {
      QStringList qsl_tmp = QStringList::split( "\n", parameters[ "save_params.field" ] );
      for ( unsigned int i = 0; i < ( unsigned int ) qsl_tmp.size(); i++ )
      {
         save_params.field.push_back( qsl_tmp[ i ] );
      }
   }

   {
      gparams.clear();
      QRegExp rx_gparam( "^gparam:(.*)$" );
      
      for ( map < QString, QString >::iterator it = parameters.begin();
            it != parameters.end();
            it++ )
      {
         if ( rx_gparam.search( it->first ) != -1 )
         {
            gparams[ rx_gparam.cap( 1 ) ] = it->second;
         }
      }
   }

   if ( parameters.count( "directory_history" ) &&
        parameters.count( "directory_last_access" ) )
   {
      directory_history      .clear();
      directory_last_access  .clear();
      directory_last_filetype.clear();
      QStringList qsl_tmp1 = QStringList::split( "\n", parameters[ "directory_history"     ] );
      QStringList qsl_tmp2 = QStringList::split( "\n", parameters[ "directory_last_access" ] );
      QStringList qsl_tmp3;
      if ( parameters.count( "directory_last_filetype" ) )
      {
         qsl_tmp3 = QStringList::split( "\n", parameters[ "directory_last_filetype" ] );
         if ( qsl_tmp3.size() != qsl_tmp2.size() )
         {
            // qDebug( QString( "tmp3 cleared %1 %2" ).arg( qsl_tmp3.size() ).arg( qsl_tmp2.size() ) );
            qsl_tmp3.clear();
         }
      }

      if ( qsl_tmp1.size() == qsl_tmp2.size() )
      {
         QDateTime dt;
         for ( int i = 0; i < (int) qsl_tmp1.size(); ++i )
         {
            directory_history.push_back( qsl_tmp1[ i ] );
            dt.setTime_t( qsl_tmp2[ i ].toUInt() );
            directory_last_access[ directory_history.back() ] = dt;
            if ( qsl_tmp3.size() )
            {
               // qDebug( QString( "adding to history tmp3 %1" ).arg( qsl_tmp3[ i ] ) );
               directory_last_filetype[ directory_history.back() ] = qsl_tmp3[ i ].replace( "____", "" );
            }
         }
      }
   }
      
   // fix up consistancy parameters

   saxs_options.cs_qstart                          = saxs_options.qstart * saxs_options.qstart;
   saxs_options.cs_qend                            = saxs_options.qend   * saxs_options.qend;

   if ( !saxs_options.cryson_manual_hs )
   {
      saxs_options.cryson_hydration_shell_contrast    = 
         saxs_options.d2o_conc * saxs_options.d2o_scat_len_dens +
         ( 1e0 - saxs_options.d2o_conc ) * ( saxs_options.h2o_scat_len_dens );
   }

   if ( !gparams.count( "guinier_qRtmax" ) ||
        gparams[ "guinier_qRtmax" ].toDouble() == 0e0 )
   {
      gparams[ "guinier_qRtmax" ]                     = "1";
   }

   if ( !gparams.count( "guinier_electron_nucleon_ratio" ) ||
        gparams[ "guinier_electron_nucleon_ratio" ].toDouble() == 0e0 )
   {
      gparams[ "guinier_electron_nucleon_ratio" ]     = "1.87e0";
   }

   return true;
}

#if defined( OLD_WAY )
{
   ts << "SOMO Config file - computer generated, please do not edit...\n";

   ts << replicate_o_r_method_somo << "\t\t# Replicate overlap removal method flag\n";
      
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

   ts << replicate_o_r_method_grid << "\t\t# Replicate overlap removal method flag\n";

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
   ts << misc.vbar_temperature << "\t\t# manual vbar temperature \n";
   ts << misc.pb_rule_on << "\t\t# flag for usage of peptide bond rule\n";
   ts << misc.avg_radius << "\t\t# Average atomic radius value\n";
   ts << misc.avg_mass << "\t\t# Average atomic mass value\n";
   ts << misc.avg_hydration << "\t\t# Average atomic hydration value\n";
   ts << misc.avg_volume << "\t\t# Average bead/atom volume value\n";
   ts << misc.avg_vbar << "\t\t# Average vbar value\n";

   ts << hydro.unit << "\t\t# exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)\n";
   ts << hydro.solvent_name << "\t\t# solvent name\n";
   ts << hydro.solvent_acronym << "\t\t# solvent acronym\n";
   ts << hydro.temperature << "\t\t# solvent temperature in degrees C\n";
   ts << hydro.solvent_viscosity << "\t\t# viscosity of the solvent in cP\n";
   ts << hydro.solvent_density << "\t\t# denisty of the solvent (g/ml)\n";
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
   ts << pdb_vis.filename << endl; // "\t\t# RasMol color filename\n";

   ts << pdb_parse.skip_hydrogen << "\t\t# skip hydrogen atoms?\n";
   ts << pdb_parse.skip_water << "\t\t# skip water molecules?\n";
   ts << pdb_parse.alternate << "\t\t# skip alternate conformations?\n";
   ts << pdb_parse.find_sh << "\t\t# find SH groups?\n";
   ts << pdb_parse.missing_residues << "\t\t# how to handle missing residues\n";
   ts << pdb_parse.missing_atoms << "\t\t# how to handle missing atoms\n";

   ts << saxs_options.water_e_density << "\t\t# Water electron density\n";

   ts << saxs_options.h_scat_len << "\t\t# H scattering length (*10^-12 cm)\n";
   ts << saxs_options.d_scat_len << "\t\t# D scattering length (*10^-12 cm)\n";
   ts << saxs_options.h2o_scat_len_dens << "\t\t# H2O scattering length density (*10^-10 cm^2)\n";
   ts << saxs_options.d2o_scat_len_dens << "\t\t# D2O scattering length density (*10^-10 cm^2)\n";
   ts << saxs_options.d2o_conc << "\t\t# D2O concentration (0 to 1)\n";
   ts << saxs_options.frac_of_exch_pep << "\t\t# Fraction of exchanged peptide H (0 to 1)\n";

   ts << saxs_options.wavelength << "\t\t# scattering wavelength\n";
   ts << saxs_options.start_angle << "\t\t# starting angle\n";
   ts << saxs_options.end_angle << "\t\t# ending angle\n";
   ts << saxs_options.delta_angle << "\t\t# angle stepsize\n";
   ts << saxs_options.max_size << "\t\t# maximum size\n";
   ts << saxs_options.bin_size << "\t\t# bin size\n";
   ts << saxs_options.hydrate_pdb << "\t\t# hydrate PDB model? true = yes\n";
   ts << saxs_options.curve << "\t\t# 0 = raw, 1 = saxs, 2 = sans\n";
   ts << saxs_options.saxs_sans << "\t\t# 0 = saxs, 1 = sans\n";

   ts << bd_options.threshold_pb_pb << "\t\t# bd_options.threshold_pb_pb\n";
   ts << bd_options.threshold_pb_sc << "\t\t# bd_options.threshold_pb_sc\n";
   ts << bd_options.threshold_sc_sc << "\t\t# bd_options.threshold_sc_sc\n";
   ts << bd_options.do_rr << "\t\t# bd_options.do_rr\n";
   ts << bd_options.force_chem << "\t\t# bd_options.force_chem\n";
   ts << bd_options.bead_size_type << "\t\t# bd_options.bead_size_type\n";
   ts << bd_options.show_pdb << "\t\t# bd_options.show_pdb\n";
   ts << bd_options.run_browflex << "\t\t# bd_options.run_browflex\n";
   ts << bd_options.tprev << "\t\t# bd_options.tprev\n";
   ts << bd_options.ttraj << "\t\t# bd_options.ttraj\n";
   ts << bd_options.deltat << "\t\t# bd_options.deltat\n";
   ts << bd_options.npadif << "\t\t# bd_options.npadif\n";
   ts << bd_options.nconf << "\t\t# bd_options.nconf\n";
   ts << bd_options.inter << "\t\t# bd_options.inter\n";
   ts << bd_options.iorder << "\t\t# bd_options.iorder\n";
   ts << bd_options.iseed << "\t\t# bd_options.iseed\n";
   ts << bd_options.icdm << "\t\t# bd_options.icdm\n";
   ts << bd_options.chem_pb_pb_bond_type << "\t\t# bd_options.chem_pb_pb_bond_type\n";
   ts << bd_options.compute_chem_pb_pb_force_constant << "\t\t# bd_options.compute_chem_pb_pb_force_constant\n";
   ts << bd_options.chem_pb_pb_force_constant << "\t\t# bd_options.chem_pb_pb_force_constant\n";
   ts << bd_options.compute_chem_pb_pb_equilibrium_dist << "\t\t# bd_options.compute_chem_pb_pb_equilibrium_dist\n";
   ts << bd_options.chem_pb_pb_equilibrium_dist << "\t\t# bd_options.chem_pb_pb_equilibrium_dist\n";
   ts << bd_options.compute_chem_pb_pb_max_elong << "\t\t# bd_options.compute_chem_pb_pb_max_elong\n";
   ts << bd_options.chem_pb_pb_max_elong << "\t\t# bd_options.chem_pb_pb_max_elong\n";
   ts << bd_options.chem_pb_sc_bond_type << "\t\t# bd_options.chem_pb_sc_bond_type\n";
   ts << bd_options.compute_chem_pb_sc_force_constant << "\t\t# bd_options.compute_chem_pb_sc_force_constant\n";
   ts << bd_options.chem_pb_sc_force_constant << "\t\t# bd_options.chem_pb_sc_force_constant\n";
   ts << bd_options.compute_chem_pb_sc_equilibrium_dist << "\t\t# bd_options.compute_chem_pb_sc_equilibrium_dist\n";
   ts << bd_options.chem_pb_sc_equilibrium_dist << "\t\t# bd_options.chem_pb_sc_equilibrium_dist\n";
   ts << bd_options.compute_chem_pb_sc_max_elong << "\t\t# bd_options.compute_chem_pb_sc_max_elong\n";
   ts << bd_options.chem_pb_sc_max_elong << "\t\t# bd_options.chem_pb_sc_max_elong\n";
   ts << bd_options.chem_sc_sc_bond_type << "\t\t# bd_options.chem_sc_sc_bond_type\n";
   ts << bd_options.compute_chem_sc_sc_force_constant << "\t\t# bd_options.compute_chem_sc_sc_force_constant\n";
   ts << bd_options.chem_sc_sc_force_constant << "\t\t# bd_options.chem_sc_sc_force_constant\n";
   ts << bd_options.compute_chem_sc_sc_equilibrium_dist << "\t\t# bd_options.compute_chem_sc_sc_equilibrium_dist\n";
   ts << bd_options.chem_sc_sc_equilibrium_dist << "\t\t# bd_options.chem_sc_sc_equilibrium_dist\n";
   ts << bd_options.compute_chem_sc_sc_max_elong << "\t\t# bd_options.compute_chem_sc_sc_max_elong\n";
   ts << bd_options.chem_sc_sc_max_elong << "\t\t# bd_options.chem_sc_sc_max_elong\n";
   ts << bd_options.pb_pb_bond_type << "\t\t# bd_options.pb_pb_bond_type\n";
   ts << bd_options.compute_pb_pb_force_constant << "\t\t# bd_options.compute_pb_pb_force_constant\n";
   ts << bd_options.pb_pb_force_constant << "\t\t# bd_options.pb_pb_force_constant\n";
   ts << bd_options.compute_pb_pb_equilibrium_dist << "\t\t# bd_options.compute_pb_pb_equilibrium_dist\n";
   ts << bd_options.pb_pb_equilibrium_dist << "\t\t# bd_options.pb_pb_equilibrium_dist\n";
   ts << bd_options.compute_pb_pb_max_elong << "\t\t# bd_options.compute_pb_pb_max_elong\n";
   ts << bd_options.pb_pb_max_elong << "\t\t# bd_options.pb_pb_max_elong\n";
   ts << bd_options.pb_sc_bond_type << "\t\t# bd_options.pb_sc_bond_type\n";
   ts << bd_options.compute_pb_sc_force_constant << "\t\t# bd_options.compute_pb_sc_force_constant\n";
   ts << bd_options.pb_sc_force_constant << "\t\t# bd_options.pb_sc_force_constant\n";
   ts << bd_options.compute_pb_sc_equilibrium_dist << "\t\t# bd_options.compute_pb_sc_equilibrium_dist\n";
   ts << bd_options.pb_sc_equilibrium_dist << "\t\t# bd_options.pb_sc_equilibrium_dist\n";
   ts << bd_options.compute_pb_sc_max_elong << "\t\t# bd_options.compute_pb_sc_max_elong\n";
   ts << bd_options.pb_sc_max_elong << "\t\t# bd_options.pb_sc_max_elong\n";
   ts << bd_options.sc_sc_bond_type << "\t\t# bd_options.sc_sc_bond_type\n";
   ts << bd_options.compute_sc_sc_force_constant << "\t\t# bd_options.compute_sc_sc_force_constant\n";
   ts << bd_options.sc_sc_force_constant << "\t\t# bd_options.sc_sc_force_constant\n";
   ts << bd_options.compute_sc_sc_equilibrium_dist << "\t\t# bd_options.compute_sc_sc_equilibrium_dist\n";
   ts << bd_options.sc_sc_equilibrium_dist << "\t\t# bd_options.sc_sc_equilibrium_dist\n";
   ts << bd_options.compute_sc_sc_max_elong << "\t\t# bd_options.compute_sc_sc_max_elong\n";
   ts << bd_options.sc_sc_max_elong << "\t\t# bd_options.sc_sc_max_elong\n";
   ts << bd_options.nmol << "\t\t# bd_options.nmol\n";

   ts << anaflex_options.run_anaflex << "\t\t# anaflex_options.run_anaflex\n";
   ts << anaflex_options.nfrec << "\t\t# anaflex_options.nfrec\n";
   ts << anaflex_options.instprofiles << "\t\t# anaflex_options.instprofiles\n";
   ts << anaflex_options.run_mode_1 << "\t\t# anaflex_options.run_mode_1\n";
   ts << anaflex_options.run_mode_1_1 << "\t\t# anaflex_options.run_mode_1_1\n";
   ts << anaflex_options.run_mode_1_2 << "\t\t# anaflex_options.run_mode_1_2\n";
   ts << anaflex_options.run_mode_1_3 << "\t\t# anaflex_options.run_mode_1_3\n";
   ts << anaflex_options.run_mode_1_4 << "\t\t# anaflex_options.run_mode_1_4\n";
   ts << anaflex_options.run_mode_1_5 << "\t\t# anaflex_options.run_mode_1_5\n";
   ts << anaflex_options.run_mode_1_7 << "\t\t# anaflex_options.run_mode_1_7\n";
   ts << anaflex_options.run_mode_1_8 << "\t\t# anaflex_options.run_mode_1_8\n";
   ts << anaflex_options.run_mode_1_12 << "\t\t# anaflex_options.run_mode_1_12\n";
   ts << anaflex_options.run_mode_1_13 << "\t\t# anaflex_options.run_mode_1_13\n";
   ts << anaflex_options.run_mode_1_14 << "\t\t# anaflex_options.run_mode_1_14\n";
   ts << anaflex_options.run_mode_1_18 << "\t\t# anaflex_options.run_mode_1_18\n";
   ts << anaflex_options.run_mode_1_20 << "\t\t# anaflex_options.run_mode_1_20\n";
   ts << anaflex_options.run_mode_1_24 << "\t\t# anaflex_options.run_mode_1_24\n";
   ts << anaflex_options.run_mode_2 << "\t\t# anaflex_options.run_mode_2\n";
   ts << anaflex_options.run_mode_2_1 << "\t\t# anaflex_options.run_mode_2_1\n";
   ts << anaflex_options.run_mode_2_2 << "\t\t# anaflex_options.run_mode_2_2\n";
   ts << anaflex_options.run_mode_2_3 << "\t\t# anaflex_options.run_mode_2_3\n";
   ts << anaflex_options.run_mode_2_4 << "\t\t# anaflex_options.run_mode_2_4\n";
   ts << anaflex_options.run_mode_2_5 << "\t\t# anaflex_options.run_mode_2_5\n";
   ts << anaflex_options.run_mode_2_7 << "\t\t# anaflex_options.run_mode_2_7\n";
   ts << anaflex_options.run_mode_2_8 << "\t\t# anaflex_options.run_mode_2_8\n";
   ts << anaflex_options.run_mode_2_12 << "\t\t# anaflex_options.run_mode_2_12\n";
   ts << anaflex_options.run_mode_2_13 << "\t\t# anaflex_options.run_mode_2_13\n";
   ts << anaflex_options.run_mode_2_14 << "\t\t# anaflex_options.run_mode_2_14\n";
   ts << anaflex_options.run_mode_2_18 << "\t\t# anaflex_options.run_mode_2_18\n";
   ts << anaflex_options.run_mode_2_20 << "\t\t# anaflex_options.run_mode_2_20\n";
   ts << anaflex_options.run_mode_2_24 << "\t\t# anaflex_options.run_mode_2_24\n";
   ts << anaflex_options.run_mode_3 << "\t\t# anaflex_options.run_mode_3\n";
   ts << anaflex_options.run_mode_3_1 << "\t\t# anaflex_options.run_mode_3_1\n";
   ts << anaflex_options.run_mode_3_5 << "\t\t# anaflex_options.run_mode_3_5\n";
   ts << anaflex_options.run_mode_3_9 << "\t\t# anaflex_options.run_mode_3_9\n";
   ts << anaflex_options.run_mode_3_10 << "\t\t# anaflex_options.run_mode_3_10\n";
   ts << anaflex_options.run_mode_3_14 << "\t\t# anaflex_options.run_mode_3_14\n";
   ts << anaflex_options.run_mode_3_15 << "\t\t# anaflex_options.run_mode_3_15\n";
   ts << anaflex_options.run_mode_3_16 << "\t\t# anaflex_options.run_mode_3_16\n";
   ts << anaflex_options.run_mode_4 << "\t\t# anaflex_options.run_mode_4\n";
   ts << anaflex_options.run_mode_4_1 << "\t\t# anaflex_options.run_mode_4_1\n";
   ts << anaflex_options.run_mode_4_6 << "\t\t# anaflex_options.run_mode_4_6\n";
   ts << anaflex_options.run_mode_4_7 << "\t\t# anaflex_options.run_mode_4_7\n";
   ts << anaflex_options.run_mode_4_8 << "\t\t# anaflex_options.run_mode_4_8\n";
   ts << anaflex_options.run_mode_9 << "\t\t# anaflex_options.run_mode_9\n";
   ts << anaflex_options.ntimc << "\t\t# anaflex_options.ntimc\n";
   ts << anaflex_options.tmax << "\t\t# anaflex_options.tmax\n";
   ts << anaflex_options.run_mode_3_5_iii << "\t\t# anaflex_options.run_mode_3_5_iii\n";
   ts << anaflex_options.run_mode_3_5_jjj << "\t\t# anaflex_options.run_mode_3_5_jjj\n";
   ts << anaflex_options.run_mode_3_10_theta << "\t\t# anaflex_options.run_mode_3_10_theta\n";
   ts << anaflex_options.run_mode_3_10_refractive_index << "\t\t# anaflex_options.run_mode_3_10_refractive_index\n";
   ts << anaflex_options.run_mode_3_10_lambda << "\t\t# anaflex_options.run_mode_3_10_lambda\n";
   ts << anaflex_options.run_mode_3_14_iii << "\t\t# anaflex_options.run_mode_3_14_iii\n";
   ts << anaflex_options.run_mode_3_14_jjj << "\t\t# anaflex_options.run_mode_3_14_jjj\n";
      
   ts << batch.missing_atoms << "\t\t# batch missing atom handling\n";
   ts << batch.missing_residues << "\t\t# batch missing residue handling\n";
   ts << batch.somo << "\t\t# batch run somo\n";
   ts << batch.grid << "\t\t# batch run grid\n";
   ts << batch.hydro << "\t\t# batch run hydro\n";
   ts << batch.avg_hydro << "\t\t# batch avg hydro\n";
   ts << batch.avg_hydro_name << "\t\t# batch avg hydro name\n";
   ts << batch.height << "\t\t# batch window last height\n";
   ts << batch.width << "\t\t# batch window last width\n";
   ts << batch.file.size() << "\t\t# batch number of files to follow\n";
   for ( unsigned int i = 0; i < batch.file.size(); i++ )
   {
      ts << batch.file[i] << endl;
   }

   ts << save_params.field.size() << "\t\t# save params number of fields to follow\n";
   for ( unsigned int i = 0; i < save_params.field.size(); i++ )
   {
      ts << save_params.field[i] << endl;
   }

   ts << path_load_pdb << endl;
   ts << path_view_pdb << endl;
   ts << path_load_bead_model << endl;
   ts << path_view_asa_res << endl;
   ts << path_view_bead_model << endl;
   ts << path_open_hydro_res << endl;
   ts << saxs_options.path_load_saxs_curve << endl;
   ts << saxs_options.path_load_gnom << endl;
   ts << saxs_options.path_load_prr << endl;

   ts << asa.hydrate_probe_radius << "\t\t#asa.hydrate_probe_radius\n";
   ts << asa.hydrate_threshold << "\t\t#asa.hydrate_threshold\n";

   ts << misc.target_e_density       << "\t\t#misc.target_e_density      \n";
   ts << misc.target_volume          << "\t\t#misc.target_volume         \n";
   ts << misc.set_target_on_load_pdb << "\t\t#misc.set_target_on_load_pdb\n";
   ts << misc.equalize_radii         << "\t\t#misc.equalize_radii        \n";

   ts << dmd_options.force_chem << "\t\t#dmd_options.force_chem\n";
   ts << dmd_options.pdb_static_pairs << "\t\t#dmd_options.pdb_static_pairs\n";
   ts << dmd_options.threshold_pb_pb << "\t\t#dmd_options.threshold_pb_pb\n";
   ts << dmd_options.threshold_pb_sc << "\t\t#dmd_options.threshold_pb_sc\n";
   ts << dmd_options.threshold_sc_sc << "\t\t#dmd_options.threshold_sc_sc\n";

   ts << saxs_options.normalize_by_mw << "\t\t#saxs_options.normalize_by_mw\n";

   ts << saxs_options.saxs_iq_native_debye << "\t\t#saxs_options.saxs_iq_native_debye\n";
   ts << saxs_options.saxs_iq_native_hybrid << "\t\t#saxs_options.saxs_iq_native_hybrid\n";
   ts << saxs_options.saxs_iq_native_hybrid2 << "\t\t#saxs_options.saxs_iq_native_hybrid2\n";
   ts << saxs_options.saxs_iq_native_hybrid3 << "\t\t#saxs_options.saxs_iq_native_hybrid3\n";
   ts << saxs_options.saxs_iq_native_fast << "\t\t#saxs_options.saxs_iq_native_fast\n";
   ts << saxs_options.saxs_iq_native_fast_compute_pr << "\t\t#saxs_options.saxs_iq_native_fast_compute_pr\n";
   ts << saxs_options.saxs_iq_foxs << "\t\t#saxs_options.saxs_iq_foxs\n";
   ts << saxs_options.saxs_iq_crysol << "\t\t#saxs_options.saxs_iq_crysol\n";

   ts << saxs_options.sans_iq_native_debye << "\t\t#saxs_options.sans_iq_native_debye\n";
   ts << saxs_options.sans_iq_native_hybrid << "\t\t#saxs_options.sans_iq_native_hybrid\n";
   ts << saxs_options.sans_iq_native_hybrid2 << "\t\t#saxs_options.sans_iq_native_hybrid2\n";
   ts << saxs_options.sans_iq_native_hybrid3 << "\t\t#saxs_options.sans_iq_native_hybrid3\n";
   ts << saxs_options.sans_iq_native_fast << "\t\t#saxs_options.sans_iq_native_fast\n";
   ts << saxs_options.sans_iq_native_fast_compute_pr << "\t\t#saxs_options.sans_iq_native_fast_compute_pr\n";
   ts << saxs_options.sans_iq_cryson << "\t\t#saxs_options.sans_iq_cryson\n";

   ts << saxs_options.hybrid2_q_points << "\t\t#saxs_options.hybrid2_q_points\n";

   ts << saxs_options.iq_ask << "\t\t#saxs_options.iq_ask\n";

   ts << saxs_options.iq_scale_ask << "\t\t#saxs_options.iq_scale_ask\n";
   ts << saxs_options.iq_scale_angstrom << "\t\t#saxs_options.iq_scale_angstrom\n";
   ts << saxs_options.iq_scale_nm << "\t\t#saxs_options.iq_scale_nm\n";

   ts << saxs_options.sh_max_harmonics << "\t\t#saxs_options.sh_max_harmonics\n";
   ts << saxs_options.sh_fibonacci_grid_order << "\t\t#saxs_options.sh_fibonacci_grid_order\n";
   ts << saxs_options.crysol_hydration_shell_contrast << "\t\t#saxs_options.crysol_hydration_shell_contrast\n";
   ts << saxs_options.crysol_default_load_difference_intensity << "\t\t#saxs_options.crysol_default_load_difference_intensity\n";
   ts << saxs_options.crysol_version_26 << "\t\t#saxs_options.crysol_version_26\n";

   ts << saxs_options.fast_bin_size << "\t\t#saxs_options.fast_bin_size\n";
   ts << saxs_options.fast_modulation << "\t\t#saxs_options.fast_modulation\n";

   ts << saxs_options.compute_saxs_coeff_for_bead_models << "\t\t#saxs_options.compute_saxs_coeff_for_bead_models\n";
   ts << saxs_options.compute_sans_coeff_for_bead_models << "\t\t#saxs_options.compute_sans_coeff_for_bead_models\n";

   ts << saxs_options.default_atom_filename << endl;
   ts << saxs_options.default_hybrid_filename << endl;
   ts << saxs_options.default_saxs_filename << endl;
   ts << saxs_options.default_rotamer_filename << endl;

   ts << saxs_options.steric_clash_distance         << "\t\t#saxs_options.steric_clash_distance        \n";
   ts << saxs_options.steric_clash_recheck_distance << "\t\t#saxs_options.steric_clash_recheck_distance\n";

   ts << saxs_options.disable_iq_scaling << "\t\t#saxs_options.disable_iq_scaling\n";
   ts << saxs_options.autocorrelate << "\t\t#saxs_options.autocorrelate\n";
   ts << saxs_options.hybrid_radius_excl_vol << "\t\t#saxs_options.hybrid_radius_excl_vol\n";
   ts << saxs_options.scale_excl_vol << "\t\t#saxs_options.scale_excl_vol\n";
   ts << saxs_options.subtract_radius << "\t\t#saxs_options.subtract_radius\n";
   ts << saxs_options.iqq_scale_minq << "\t\t#saxs_options.iqq_scale_minq\n";
   ts << saxs_options.iqq_scale_maxq << "\t\t#saxs_options.iqq_scale_maxq\n";

   ts << saxs_options.iqq_scale_nnls << "\t\t#saxs_options.iqq_scale_nnls\n";
   ts << saxs_options.iqq_scale_linear_offset << "\t\t#saxs_options.iqq_scale_linear_offset\n";
   ts << saxs_options.iqq_scale_chi2_fitting << "\t\t#saxs_options.iqq_scale_chi2_fitting\n";
   ts << saxs_options.iqq_expt_data_contains_variances << "\t\t#saxs_options.iqq_expt_data_contains_variances\n";
   ts << saxs_options.iqq_ask_target_grid << "\t\t#saxs_options.iqq_ask_target_grid\n";
   ts << saxs_options.iqq_scale_play << "\t\t#saxs_options.iqq_scale_play\n";
   ts << saxs_options.swh_excl_vol << "\t\t#saxs_options.swh_excl_vol\n";
   ts << saxs_options.iqq_default_scaling_target << endl;

   ts << saxs_options.saxs_iq_hybrid_adaptive << "\t\t#saxs_options.saxs_iq_hybrid_adaptive\n";
   ts << saxs_options.sans_iq_hybrid_adaptive << "\t\t#saxs_options.sans_iq_hybrid_adaptive\n";

   ts << saxs_options.bead_model_rayleigh   << "\t\t#saxs_options.bead_model_rayleigh  \n";
   ts << saxs_options.iqq_log_fitting       << "\t\t#saxs_options.iqq_log_fitting      \n";
   ts << saxs_options.iqq_kratky_fit        << "\t\t#saxs_options.iqq_kratky_fit       \n";
   ts << saxs_options.iqq_use_atomic_ff     << "\t\t#saxs_options.iqq_use_atomic_ff    \n";
   ts << saxs_options.iqq_use_saxs_excl_vol << "\t\t#saxs_options.iqq_use_saxs_excl_vol\n";
   ts << saxs_options.alt_hydration         << "\t\t#saxs_options.alt_hydration        \n";

   ts << saxs_options.xsr_symmop                << "\t\t#saxs_options.xsr_symmop               \n";
   ts << saxs_options.xsr_nx                    << "\t\t#saxs_options.xsr_nx                   \n";
   ts << saxs_options.xsr_ny                    << "\t\t#saxs_options.xsr_ny                   \n";
   ts << saxs_options.xsr_griddistance          << "\t\t#saxs_options.xsr_griddistance         \n";
   ts << saxs_options.xsr_ncomponents           << "\t\t#saxs_options.xsr_ncomponents          \n";
   ts << saxs_options.xsr_compactness_weight    << "\t\t#saxs_options.xsr_compactness_weight   \n";
   ts << saxs_options.xsr_looseness_weight      << "\t\t#saxs_options.xsr_looseness_weight     \n";
   ts << saxs_options.xsr_temperature           << "\t\t#saxs_options.xsr_temperature          \n";

   ts << hydro.zeno_zeno              << "\t\t#hydro.zeno_zeno             \n";
   ts << hydro.zeno_interior          << "\t\t#hydro.zeno_interior         \n";
   ts << hydro.zeno_surface           << "\t\t#hydro.zeno_surface          \n";
   ts << hydro.zeno_zeno_steps        << "\t\t#hydro.zeno_zeno_steps       \n";
   ts << hydro.zeno_interior_steps    << "\t\t#hydro.zeno_interior_steps   \n";
   ts << hydro.zeno_surface_steps     << "\t\t#hydro.zeno_surface_steps    \n";
   ts << hydro.zeno_surface_thickness << "\t\t#hydro.zeno_surface_thickness\n";

   ts << misc.hydro_supc              << "\t\t#misc.hydro_supc             \n";
   ts << misc.hydro_zeno              << "\t\t#misc.hydro_zeno             \n";

   ts << batch.saxs_search << "\t\t#batch.saxs_search\n";
   ts << batch.zeno        << "\t\t#batch.zeno       \n";

   f.close();
}
#endif

void US_Hydrodyn::hard_coded_defaults()
{
   // hard coded defaults
   replicate_o_r_method_somo = false;

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

   replicate_o_r_method_grid = false;

   grid_exposed_overlap.remove_overlap = true;
   grid_exposed_overlap.fuse_beads = false;
   grid_exposed_overlap.fuse_beads_percent = 0.0;
   grid_exposed_overlap.remove_hierarch = false;
   grid_exposed_overlap.remove_hierarch_percent = 1.0;
   grid_exposed_overlap.remove_sync = true;
   grid_exposed_overlap.remove_sync_percent = 1.0;
   grid_exposed_overlap.translate_out = true;
   grid_exposed_overlap.show_translate = true;

   grid_buried_overlap.remove_overlap = true;
   grid_buried_overlap.fuse_beads = false;
   grid_buried_overlap.fuse_beads_percent = 0.0;
   grid_buried_overlap.remove_hierarch = false;
   grid_buried_overlap.remove_hierarch_percent = 1.0;
   grid_buried_overlap.remove_sync = true;
   grid_buried_overlap.remove_sync_percent = 1.0;
   grid_buried_overlap.translate_out = false;
   grid_buried_overlap.show_translate = false;

   grid_overlap.remove_overlap = true;
   grid_overlap.fuse_beads = false;
   grid_overlap.fuse_beads_percent = 0.0;
   grid_overlap.remove_hierarch = false;
   grid_overlap.remove_hierarch_percent = 1.0;
   grid_overlap.remove_sync = true;
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
   grid.hydrate = true;    // true: hydrate model
   grid.center = 0;    // 1: center of cubelet, 0: center of mass, 2: center of scattering
   grid.tangency = false;   // true: Expand beads to tangency
   grid.cube_side = 5.0;
   grid.enable_asa = true;   // true: enable asa

   misc.hydrovol = 24.041;
   misc.compute_vbar = true;
   misc.vbar = 0.72;
   misc.vbar_temperature = 20.0;
   misc.pb_rule_on = true;
   misc.avg_radius = 1.68;
   misc.avg_mass = 16.0;
   misc.avg_hydration = 0.4;
   misc.avg_volume = 15.3;
   misc.avg_vbar = 0.72;
   overlap_tolerance = 0.001;

   hydro.unit = -10;                // exponent from units in meter (example: -10 = Angstrom, -9 = nanometers)

   hydro.solvent_name = "Water";
   hydro.solvent_acronym = "w";
   hydro.temperature = K20 - K0;
   hydro.solvent_viscosity = VISC_20W * 100;
   hydro.solvent_density = DENS_20W;

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

   pdb_parse.skip_hydrogen = true;
   pdb_parse.skip_water = true;
   pdb_parse.alternate = true;
   pdb_parse.find_sh = false;
   pdb_parse.missing_residues = 0;
   pdb_parse.missing_atoms = 0;

   saxs_options.water_e_density = 0.334f; // water electron density in e/A^3

   saxs_options.h_scat_len = -0.3742f;        // H scattering length (*10^-12 cm)
   saxs_options.d_scat_len = 0.6671f ;        // D scattering length (*10^-12 cm)
   saxs_options.h2o_scat_len_dens = -0.562f;  // H2O scattering length density (*10^-10 cm^2)
   saxs_options.d2o_scat_len_dens = 6.404f;   // D2O scattering length density (*10^-10 cm^2)
   saxs_options.d2o_conc = 0.16f;             // D2O concentration (0 to 1)
   saxs_options.frac_of_exch_pep = 0.1f;      // Fraction of exchanged peptide H (0 to 1)

   saxs_options.wavelength = 1.5;         // scattering wavelength
   saxs_options.start_angle = 0.014f;     // start angle
   saxs_options.end_angle = 8.214f;       // ending angle
   saxs_options.delta_angle = 0.2f;       // angle stepsize
   saxs_options.max_size = 40.0;          // maximum size (A)
   saxs_options.bin_size = 1.0f;          // Bin size (A)
   saxs_options.hydrate_pdb = false;      // Hydrate the PDB model? (true/false)
   saxs_options.curve = 1;                // 0 = raw, 1 = saxs, 2 = sans
   saxs_options.saxs_sans = 0;            // 0 = saxs, 1 = sans

   saxs_options.guinier_csv = false;
   saxs_options.guinier_csv_filename = "guinier";
   saxs_options.qRgmax = 1.3e0;
   saxs_options.qstart = 1e-7;
   saxs_options.qend = .5e0;
   saxs_options.pointsmin = 10;
   saxs_options.pointsmax = 100;

   bd_options.threshold_pb_pb = 5;
   bd_options.threshold_pb_sc = 5;
   bd_options.threshold_sc_sc = 5;
   bd_options.do_rr = true;
   bd_options.force_chem = true;
   bd_options.bead_size_type = 0;
   bd_options.show_pdb = true;
   bd_options.run_browflex = true;
   bd_options.tprev = 8.0e-9;
   bd_options.ttraj = 8.0e-6;
   bd_options.deltat = 1.6e-13;
   bd_options.npadif = 10;
   bd_options.nconf = 1000;
   bd_options.inter = 2;
   bd_options.iorder = 1;
   bd_options.iseed = 1234;
   bd_options.icdm = 0;
   bd_options.chem_pb_pb_bond_type = 0;
   bd_options.compute_chem_pb_pb_force_constant = false;
   bd_options.chem_pb_pb_force_constant = 10.0;
   bd_options.compute_chem_pb_pb_equilibrium_dist = true;
   bd_options.chem_pb_pb_equilibrium_dist = 0.0;
   bd_options.compute_chem_pb_pb_max_elong = true;
   bd_options.chem_pb_pb_max_elong = 0.0;
   bd_options.chem_pb_sc_bond_type = 0;
   bd_options.compute_chem_pb_sc_force_constant = false;
   bd_options.chem_pb_sc_force_constant = 10.0;
   bd_options.compute_chem_pb_sc_equilibrium_dist = true;
   bd_options.chem_pb_sc_equilibrium_dist = 0.0;
   bd_options.compute_chem_pb_sc_max_elong = true;
   bd_options.chem_pb_sc_max_elong = 0.0;
   bd_options.chem_sc_sc_bond_type = 0;
   bd_options.compute_chem_sc_sc_force_constant = false;
   bd_options.chem_sc_sc_force_constant = 10.0;
   bd_options.compute_chem_sc_sc_equilibrium_dist = true;
   bd_options.chem_sc_sc_equilibrium_dist = 0.0;
   bd_options.compute_chem_sc_sc_max_elong = true;
   bd_options.chem_sc_sc_max_elong = 0.0;
   bd_options.pb_pb_bond_type = 0;
   bd_options.compute_pb_pb_force_constant = false;
   bd_options.pb_pb_force_constant = 10.0;
   bd_options.compute_pb_pb_equilibrium_dist = true;
   bd_options.pb_pb_equilibrium_dist = 0.0;
   bd_options.compute_pb_pb_max_elong = true;
   bd_options.pb_pb_max_elong = 0.0;
   bd_options.pb_sc_bond_type = 0;
   bd_options.compute_pb_sc_force_constant = false;
   bd_options.pb_sc_force_constant = 10.0;
   bd_options.compute_pb_sc_equilibrium_dist = true;
   bd_options.pb_sc_equilibrium_dist = 0.0;
   bd_options.compute_pb_sc_max_elong = true;
   bd_options.pb_sc_max_elong = 0.0;
   bd_options.sc_sc_bond_type = 0;
   bd_options.compute_sc_sc_force_constant = false;
   bd_options.sc_sc_force_constant = 10.0;
   bd_options.compute_sc_sc_equilibrium_dist = true;
   bd_options.sc_sc_equilibrium_dist = 0.0;
   bd_options.compute_sc_sc_max_elong = true;
   bd_options.sc_sc_max_elong = 0.0;
   bd_options.nmol = 1;

   anaflex_options.run_anaflex = true;
   anaflex_options.nfrec = 10;
   anaflex_options.instprofiles = false;
   anaflex_options.run_mode_1 = false;
   anaflex_options.run_mode_1_1 = false;
   anaflex_options.run_mode_1_2 = false;
   anaflex_options.run_mode_1_3 = false;
   anaflex_options.run_mode_1_4 = false;
   anaflex_options.run_mode_1_5 = false;
   anaflex_options.run_mode_1_7 = false;
   anaflex_options.run_mode_1_8 = false;
   anaflex_options.run_mode_1_12 = false;
   anaflex_options.run_mode_1_13 = false;
   anaflex_options.run_mode_1_14 = false;
   anaflex_options.run_mode_1_18 = true;
   anaflex_options.run_mode_1_20 = false;
   anaflex_options.run_mode_1_24 = false;
   anaflex_options.run_mode_2 = false;
   anaflex_options.run_mode_2_1 = false;
   anaflex_options.run_mode_2_2 = false;
   anaflex_options.run_mode_2_3 = false;
   anaflex_options.run_mode_2_4 = false;
   anaflex_options.run_mode_2_5 = false;
   anaflex_options.run_mode_2_7 = false;
   anaflex_options.run_mode_2_8 = false;
   anaflex_options.run_mode_2_12 = false;
   anaflex_options.run_mode_2_13 = false;
   anaflex_options.run_mode_2_14 = false;
   anaflex_options.run_mode_2_18 = true;
   anaflex_options.run_mode_2_20 = false;
   anaflex_options.run_mode_2_24 = false;
   anaflex_options.run_mode_3 = true;
   anaflex_options.run_mode_3_1 = true;
   anaflex_options.run_mode_3_5 = false;
   anaflex_options.run_mode_3_9 = false;
   anaflex_options.run_mode_3_10 = false;
   anaflex_options.run_mode_3_14 = false;
   anaflex_options.run_mode_3_15 = false;
   anaflex_options.run_mode_3_16 = false;
   anaflex_options.run_mode_4 = false;
   anaflex_options.run_mode_4_1 = false;
   anaflex_options.run_mode_4_6 = false;
   anaflex_options.run_mode_4_7 = false;
   anaflex_options.run_mode_4_8 = true;
   anaflex_options.run_mode_9 = false;
   anaflex_options.ntimc = 21;
   anaflex_options.tmax = (float)1.6e-6;
   anaflex_options.run_mode_3_5_iii = 1;
   anaflex_options.run_mode_3_5_jjj = 99999;
   anaflex_options.run_mode_3_10_theta = 90.0;
   anaflex_options.run_mode_3_10_refractive_index = (float)1.3312;
   anaflex_options.run_mode_3_10_lambda = 633.0;
   anaflex_options.run_mode_3_14_iii = 1;
   anaflex_options.run_mode_3_14_jjj = 99999;
      
   batch.missing_atoms = 0;
   batch.missing_residues = 0;
   batch.somo = true;
   batch.grid = false;
   batch.hydro = true;
   batch.avg_hydro = false;
   batch.avg_hydro_name = "results";
   batch.height = 0;
   batch.width = 0;
   batch.file.clear();

   path_load_pdb = "";
   path_view_pdb = "";
   path_load_bead_model = "";
   path_view_asa_res = "";
   path_view_bead_model = "";
   path_open_hydro_res = "";
   saxs_options.path_load_saxs_curve = "";
   saxs_options.path_load_gnom = "";
   saxs_options.path_load_prr = "";

   save_params.field.clear();

   asa.hydrate_probe_radius = 1.4f;
   asa.hydrate_threshold = 10.0f;

   misc.target_e_density       = 0e0;
   misc.target_volume          = 0e0;
   misc.set_target_on_load_pdb = false;
   misc.equalize_radii         = false;

   dmd_options.force_chem = true;
   dmd_options.pdb_static_pairs = false;
   dmd_options.threshold_pb_pb = 5;
   dmd_options.threshold_pb_sc = 5;
   dmd_options.threshold_sc_sc = 5;

   saxs_options.normalize_by_mw = true;

   saxs_options.saxs_iq_native_debye = false;
   saxs_options.saxs_iq_native_hybrid = false;
   saxs_options.saxs_iq_native_hybrid2 = false;
   saxs_options.saxs_iq_native_hybrid3 = true;
   saxs_options.saxs_iq_native_fast = false;
   saxs_options.saxs_iq_native_fast_compute_pr = false;
   saxs_options.saxs_iq_foxs = false;
   saxs_options.saxs_iq_crysol = false;

   saxs_options.sans_iq_native_debye = true;
   saxs_options.sans_iq_native_hybrid = false;
   saxs_options.sans_iq_native_hybrid2 = false;
   saxs_options.sans_iq_native_hybrid3 = false;
   saxs_options.sans_iq_native_fast = false;
   saxs_options.sans_iq_native_fast_compute_pr = false;
   saxs_options.sans_iq_cryson = false;

   saxs_options.hybrid2_q_points = 15;

   saxs_options.iq_ask = false;

   saxs_options.iq_scale_ask = false;
   saxs_options.iq_scale_angstrom = true;
   saxs_options.iq_scale_nm = false;

   saxs_options.sh_max_harmonics = 15;
   saxs_options.sh_fibonacci_grid_order = 17;
   saxs_options.crysol_hydration_shell_contrast = 0.03f;
   saxs_options.crysol_default_load_difference_intensity = true;
   saxs_options.crysol_version_26 = true;

   saxs_options.fast_bin_size = 0.5f;
   saxs_options.fast_modulation = 0.23f;

   saxs_options.compute_saxs_coeff_for_bead_models = true;
   saxs_options.compute_sans_coeff_for_bead_models = false;

   saxs_options.default_atom_filename = "";
   saxs_options.default_hybrid_filename = "";
   saxs_options.default_saxs_filename = "";
   saxs_options.default_rotamer_filename = "";

   saxs_options.steric_clash_distance         = 20.0;
   saxs_options.steric_clash_recheck_distance = 0.0;

   saxs_options.disable_iq_scaling = false;
   saxs_options.autocorrelate = true;
   saxs_options.hybrid_radius_excl_vol = false;
   saxs_options.scale_excl_vol = 1.0f;
   saxs_options.subtract_radius = false;
   saxs_options.iqq_scale_minq = 0.0f;
   saxs_options.iqq_scale_maxq = 0.0f;

   saxs_options.iqq_scale_nnls = false;
   saxs_options.iqq_scale_linear_offset = false;
   saxs_options.iqq_scale_chi2_fitting = true;
   saxs_options.iqq_expt_data_contains_variances = false;
   saxs_options.iqq_ask_target_grid = true;
   saxs_options.iqq_scale_play = false;
   saxs_options.swh_excl_vol = 0.0f;
   saxs_options.iqq_default_scaling_target = "";

   saxs_options.saxs_iq_hybrid_adaptive = true;
   saxs_options.sans_iq_hybrid_adaptive = true;

   saxs_options.bead_model_rayleigh   = true;
   saxs_options.iqq_log_fitting       = false;
   saxs_options.iqq_kratky_fit        = false;
   saxs_options.iqq_use_atomic_ff     = false;
   saxs_options.iqq_use_saxs_excl_vol = false;
   saxs_options.alt_hydration         = false;

   saxs_options.xsr_symmop                = 2;
   saxs_options.xsr_nx                    = 32;
   saxs_options.xsr_ny                    = 32;
   saxs_options.xsr_griddistance          = 3e0;
   saxs_options.xsr_ncomponents           = 1;
   saxs_options.xsr_compactness_weight    = 10e0;
   saxs_options.xsr_looseness_weight      = 10e0;
   saxs_options.xsr_temperature           = 1e-3;

   hydro.zeno_zeno              = true;
   hydro.zeno_interior          = true;
   hydro.zeno_surface           = true;
   hydro.zeno_zeno_steps        = 1000;
   hydro.zeno_interior_steps    = 1000;
   hydro.zeno_surface_steps     = 1000;
   hydro.zeno_surface_thickness = 0.0f;

   misc.hydro_supc              = true;
   misc.hydro_zeno              = false;

   rotamer_changed = true;  // force on-demand loading of rotamer file

   batch.saxs_search = false;
   batch.zeno        = false;

   saxs_options.ignore_errors                      = false;
   saxs_options.alt_ff                             = true;
   saxs_options.crysol_explicit_hydrogens          = false;
   saxs_options.use_somo_ff                        = false;
   saxs_options.five_term_gaussians                = true;
   saxs_options.iq_exact_q                         = false;
   saxs_options.use_iq_target_ev                   = false;
   saxs_options.set_iq_target_ev_from_vbar         = false;
   saxs_options.iq_target_ev                       = 0e0;
   saxs_options.hydration_rev_asa                  = false;
   saxs_options.compute_exponentials               = false;
   saxs_options.compute_exponential_terms          = 5;
   saxs_options.dummy_saxs_name                    = "DAM";
   saxs_options.dummy_saxs_names                   .clear();
   saxs_options.dummy_saxs_names                   .push_back( saxs_options.dummy_saxs_name );
   saxs_options.multiply_iq_by_atomic_volume       = false;
   saxs_options.dummy_atom_pdbs_in_nm              = false;
   saxs_options.iq_global_avg_for_bead_models      = false;
   saxs_options.apply_loaded_sf_repeatedly_to_pdb  = false;
   saxs_options.bead_models_use_var_len_sf         = false;
   saxs_options.bead_models_var_len_sf_max         = 10;
   saxs_options.bead_models_use_gsm_fitting        = false;
   saxs_options.bead_models_use_quick_fitting      = true;
   saxs_options.bead_models_use_bead_radius_ev     = true;
   saxs_options.bead_models_rho0_in_scat_factors   = true;
   saxs_options.smooth                             = 0;
   saxs_options.ev_exp_mult                        = 1e0;
   saxs_options.sastbx_method                      = 0;
   saxs_options.saxs_iq_sastbx                     = false;
   saxs_options.saxs_iq_native_sh                  = false;
   saxs_options.sans_iq_native_sh                  = false;

   saxs_options.alt_sh1                            = false;
   saxs_options.alt_sh2                            = false;

   grid.create_nmr_bead_pdb                        = false;

   batch.compute_iq_only_avg                       = false;

   asa.vvv                                         = false;
   asa.vvv_probe_radius                            = 1.4f;
   asa.vvv_grid_dR                                 = 0.5f;

   misc.export_msroll                              = false;

   saxs_options.cs_qRgmax                          = 1e0;
   saxs_options.cs_qstart                          = saxs_options.qstart * saxs_options.qstart;
   saxs_options.cs_qend                            = saxs_options.qend   * saxs_options.qend;

   saxs_options.conc                               = 1e0;
   saxs_options.psv                                = 7.2e-1;
   saxs_options.use_cs_psv                         = false;
   saxs_options.cs_psv                             = 0e0;
   saxs_options.I0_exp                             = 5.4e-5;
   saxs_options.I0_theo                            = 1.633e-2;
   saxs_options.diffusion_len                      = 2.82e-13;
   saxs_options.nucleon_mass                       = 1.674e-24;

   saxs_options.guinier_outlier_reject             = false;
   saxs_options.guinier_outlier_reject_dist        = 2e0;
   saxs_options.guinier_use_sd                     = false;
   saxs_options.guinier_use_standards              = false;

   saxs_options.cryson_sh_max_harmonics            = 15;
   saxs_options.cryson_sh_fibonacci_grid_order     = 17;
   saxs_options.cryson_hydration_shell_contrast    = 
      saxs_options.d2o_conc * saxs_options.d2o_scat_len_dens +
      ( 1e0 - saxs_options.d2o_conc ) * ( saxs_options.h2o_scat_len_dens );
   saxs_options.cryson_manual_hs                   = false;

   gparams                                         .clear();
   gparams[ "guinier_auto_fit" ]                   = "1";
   gparams[ "perdeuteration" ]                     = "0";
   gparams[ "guinier_qRtmax" ]                     = "1";
   gparams[ "guinier_electron_nucleon_ratio" ]     = "1.87e0";

   gparams[ "hplc_bl_linear"   ]                   = "true";
   gparams[ "hplc_bl_integral" ]                   = "false";
   gparams[ "hplc_bl_save"     ]                   = "false";
   gparams[ "hplc_bl_smooth"   ]                   = "10";
   gparams[ "hplc_bl_reps"     ]                   = "1";
}

void US_Hydrodyn::set_default()
{
   QFile f;
   QString str;
   int j;
   // only keep one copy of defaults in system root dir
   f.setName(USglobal->config_list.system_dir + "/etc/somo.defaults");
   bool config_read = false;
   if (f.open(QIODevice::ReadOnly)) // read system directory
   {
      j=read_config(f);
      if ( j )
      {
         cout << "read config returned " << j << endl;
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
      hard_coded_defaults();

   }

   // defaults that SHOULD NOT BE MOVED INTO somo.config

   if ( pdb_vis.filename.isEmpty() )
   {
      pdb_vis.filename = USglobal->config_list.system_dir + "/etc/rasmol.spt"; //default color file
   }

   if ( saxs_options.default_atom_filename.isEmpty() )
   {
      saxs_options.default_atom_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.atom";
   }
   if ( saxs_options.default_hybrid_filename.isEmpty() )
   {
      saxs_options.default_hybrid_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.hybrid";
   }
   if ( saxs_options.default_saxs_filename.isEmpty() )
   {
      saxs_options.default_saxs_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.saxs_atoms";
   }
   if ( saxs_options.default_rotamer_filename.isEmpty() )
   {
      saxs_options.default_rotamer_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.hydrated_rotamer";
   }
   if ( saxs_options.default_ff_filename.isEmpty() )
   {
      saxs_options.default_ff_filename = USglobal->config_list.system_dir + SLASH + "etc" + SLASH + "somo.ff";
   }

   rotamer_changed = true;  // force on-demand loading of rotamer file

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
   default_batch = batch;
   default_save_params = save_params;
   default_bd_options = bd_options;
   default_anaflex_options = anaflex_options;
   default_gparams         = gparams;
}

void US_Hydrodyn::view_file(const QString &filename, QString title)
{
   //   US_Editor *edit;
   //   edit = new US_Editor(1);
   TextEdit *edit;
   edit = new TextEdit();
   edit->setFont(QFont("Courier"));
   edit->setPalette( PALET_NORMAL );
   AUTFBACK( edit );
   edit->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
   //   edit->setTitle(title);
   edit->load(filename, title, true, Qt::PlainText);
   //   edit->setTextFormat( PlainText );
   edit->show();
}

void US_Hydrodyn::closeEvent(QCloseEvent *e)
{
   QMessageBox mb(tr("UltraScan"), tr("Attention:\nAre you sure you want to exit?"),
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::Cancel | QMessageBox::Escape,
                  Qt::NoButton);
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
   if ( guiFlag )
   {
      if ( batch_widget &&
           batch_window->batch_job_running )
      {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(str);
         editor->setColor(save_color);
      } else {
         QMessageBox::warning(this, tr("UltraScan Warning"), tr("Please note:\n\n") +
                              tr(str), QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      }
   }
}

void US_Hydrodyn::closeAttnt(Q3Process *proc, QString message)
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
   scaling = 5.0 / scale;
   fprintf(fbms,
           "%d\n%s\n",
           beads + 1,
           QFileInfo(fname).fileName().ascii()
           );
   //   fprintf(fspt,
   //           "load xyz %s\nselect all\nwireframe off\nset background white\n",
   //           QString("%1.bms").arg(QFileInfo(fname).fileName()).ascii()
   //           );

   last_spt_text = 
      QString("").sprintf("load xyz %s\nselect all\nwireframe off\nset background %s\n",
                          QString("%1.bms").arg(QFileInfo(fname).fileName()).ascii(),
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
                  "write ppm %s.ppm\n"
                  "exit\n",
                  fname.ascii()
                  );
      movie_text.push_back(fname);
   }
   fprintf(fspt, ( last_spt_text.isNull() ? "" : last_spt_text.ascii() ) );
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
                 "%d~%f~%s~%s~%d~%f~%u~"
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
         printf("write_bead_asa model[%d].bead_ref_mw %g\n",
                i, (*model)[i].bead_ref_mw);
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


void US_Hydrodyn::write_corr( QString fname, vector<PDB_atom> *model ) 
{
   FILE *fcorr = (FILE *)0;
   fcorr = fopen(QString("%1.corr").arg(fname).ascii(), "w");
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
                    use_model[i]->name.ascii(),
                    use_model[i]->resName.ascii(),
                    use_model[i]->chainID.ascii(),
                    use_model[i]->resSeq.ascii(),
                    use_model[i]->iCode.ascii(),
                    use_model[i]->bead_ref_mw,
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
      Q3TextStream ts(&f);
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
            (*model)[i].bead_ref_volume = new_model[i].bead_ref_volume;
            (*model)[i].bead_recheck_asa = new_model[i].bead_recheck_asa;
            (*model)[i].chain = new_model[i].chain;
            (*model)[i].serial = new_model[i].serial;
            (*model)[i].exposed_code = new_model[i].exposed_code;
            (*model)[i].bead_color = new_model[i].bead_color;
         }
         result = true;
         editor->append("Correspondence file ok\n");
         cout << "read_corr: Correspondence file ok\n";
      } else {
         editor->append(QString("Correspondence file didn't match %1\n").arg(fname));
         cout << "read_corr: Correspondence file not ok\n";
      }
   }
   f.close();
   return result;
}

QString US_Hydrodyn::fix_file_name( QString f )
{
   f.replace( " " , "_" ).replace( "(", "" ).replace( ")", "" );
   return f;
}

void US_Hydrodyn::write_bead_model( QString fname, 
                                    vector < PDB_atom > *model,
                                    QString extra_text )
{
   // write corresopdence file also
   int decpts = -(int)log10(overlap_tolerance/9.9999) + 1;
   if (decpts < 4) {
      decpts = 4;
   }

   QString fstring_somo =
      QString("%.%1f\t%.%2f\t%.%3f\t%.%4f\t%.6f\t%d\t%s\t%.4f\n").
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
              "Field contents:\n"
              "  Line 1: Number of beads, Global partial specific volume\n"
              "  From line 2 on: X, Y, Z coordinates, Radius, Mass, Color coding, "
              "Correspondence with original residues, ASA\n"

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
      fprintf(fsomo, ( options_log.isNull() ? "" : options_log.ascii() ) );
      fprintf(fsomo, ( last_abb_msgs.isNull() ? "" : last_abb_msgs.ascii() ) );

      if ( !extra_text.isEmpty() )
      {
         fprintf(fsomo, extra_text.ascii() );
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

   s.sprintf(
             "Miscellaneous options:\n"
             "  Calculate vbar                 %s\n"
             ,misc.compute_vbar ? "On" : "Off"
             );
   options_log += s;

   if ( !misc.compute_vbar )
   {
      s.sprintf(
                "  Entered vbar value             %.3f\n"
                "  Vbar measured/computed at T=   %.2f\n"
                ,misc.vbar
                ,misc.vbar_temperature
                );
      options_log += s;
   }

   s.sprintf(
             "  Enable Peptide Bond Rule       %s\n"
             ,misc.pb_rule_on ? "On" : "Off"
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
             "  Computations Relative to:             %s\n"
             "  Cube Side (Angstrom):                 %.1f\n"
             "  Apply Cubic Grid:                     %s\n"
             "  Add theoretical hydration (PDB only): %s\n"
             "  Expand Beads to Tangency:             %s\n"
             "  Enable ASA options:                   %s\n"
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

             ,grid.center ? "Center of Mass" : ( grid.center == 2 ? "Center of Scattering Intensity" : "Center of Cubelet" )
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

   s.sprintf(
             "Miscellaneous options:\n"
             "  Calculate vbar                 %s\n"
             ,misc.compute_vbar ? "On" : "Off"
             );
   options_log += s;

   if ( !misc.compute_vbar )
   {
      s.sprintf(
                "  Entered vbar value             %.3f\n"
                "  Vbar measured/computed at T=   %.2f\n"
                ,misc.vbar
                ,misc.vbar_temperature
                );
      options_log += s;
   }
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
   if ( buried_overlap.remove_overlap != default_buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps: %1\n")
         .arg(buried_overlap.remove_overlap ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync != default_buried_overlap.remove_sync && 
        buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps synchronously: %1\n")
         .arg(buried_overlap.remove_sync ? "On" : "Off");
   }
   if ( buried_overlap.remove_sync_percent != default_buried_overlap.remove_sync_percent &&
        buried_overlap.remove_overlap && buried_overlap.remove_sync )
   {
      str += QString(base + sub + sub2 + "Synchronous Overlap Reduction Step Size %: %1\n").arg(buried_overlap.remove_sync_percent);
   }
   if ( buried_overlap.remove_hierarch != default_buried_overlap.remove_hierarch &&
        buried_overlap.remove_overlap )
   {
      str += QString(base + sub + sub2 + "Remove Overlaps hierarchically: %1\n")
         .arg(buried_overlap.remove_hierarch ? "On" : "Off");
   }
   if ( buried_overlap.remove_hierarch_percent != default_buried_overlap.remove_hierarch_percent &&
        buried_overlap.remove_overlap && buried_overlap.remove_hierarch )
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
         .arg(grid.center ? ( grid.center == 2 ? "Scattering intensity" : "Cublet" ) : "Mass");
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
      str += QString(base + " Add theoretical hydration (PDB only): %1\n")
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
      str += QString(base + "Model scale (10^-x m) (10 = Angstrom, 9 = nanometer), where x is : %1\n").arg(-hydro.unit);
   }
   if ( hydro.solvent_name != default_hydro.solvent_name )
   {
      str += QString(base + "Solvent name: %1\n").arg(hydro.solvent_name);
   }
   if ( hydro.solvent_acronym != default_hydro.solvent_acronym )
   {
      str += QString(base + "Solvent acronym: %1\n").arg(hydro.solvent_acronym);
   }
   if ( hydro.temperature != default_hydro.temperature )
   {
      str += QString(base + "Temperature (C): %1\n").arg(hydro.temperature);
   }
   if ( hydro.solvent_viscosity != default_hydro.solvent_viscosity )
   {
      str += QString(base + "Solvent viscosity (cP): %1\n").arg(hydro.solvent_viscosity);
   }
   if ( hydro.solvent_density != default_hydro.solvent_density )
   {
      str += QString(base + "Solvent density (g/ml): %1\n").arg(hydro.solvent_density);
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
   if ( hydro.rotational != default_hydro.rotational &&
        !hydro.bead_inclusion )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Rotational Diffusion: %1\n")
         .arg(hydro.rotational ? "Include" : "Exclude" );
   }
   if ( hydro.viscosity != default_hydro.viscosity  &&
        !hydro.bead_inclusion )
   {
      str += QString(base + "Include Buried Beads in Volume Correction, Intrinsic Viscosity: %1\n")
         .arg(hydro.viscosity ? "Include" : "Exclude");
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
   if ( misc.vbar_temperature != default_misc.vbar_temperature &&
        !misc.compute_vbar )
   {
      str += QString(base + "Vbar measured/computed at T= %1\n").arg(misc.vbar_temperature);
   }
   if ( misc.pb_rule_on != default_misc.pb_rule_on )
   {
      str += QString(base + "Peptide bond rule: %1\n")
         .arg(misc.pb_rule_on ? "On" : "Off");
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
   if ( misc.hydro_zeno )
   {
      str += QString(base + "Hydrodynamic method Zeno\n" );
   }
   return str;
}

QString US_Hydrodyn::default_differences_saxs_options()
{
   QString str = "";
   QString base = "SOMO Options -> SAXS/SANS Options -> ";
   if ( saxs_options.water_e_density != default_saxs_options.water_e_density )
   {
      str += QString(base + "Water electron density value: %1\n").arg(saxs_options.water_e_density );
   }

   if ( saxs_options.h_scat_len != default_saxs_options.h_scat_len )
   {
      str += QString(base + "H scattering length (*10^-12 cm): %1\n").arg(saxs_options.h_scat_len);
   }
   if ( saxs_options.d_scat_len != default_saxs_options.d_scat_len )
   {
      str += QString(base + "*10^-12 cm): %1\n").arg(saxs_options.d_scat_len);
   }
   if ( saxs_options.h2o_scat_len_dens != default_saxs_options.h2o_scat_len_dens )
   {
      str += QString(base + "H2O scattering length density (*10^-10 cm^2): %1\n").arg(saxs_options.h2o_scat_len_dens);
   }
   if ( saxs_options.d2o_scat_len_dens != default_saxs_options.d2o_scat_len_dens )
   {
      str += QString(base + "D2O scattering length density (*10^-10 cm^2): %1\n").arg(saxs_options.d2o_scat_len_dens);
   }
   if ( saxs_options.d2o_conc != default_saxs_options.d2o_conc )
   {
      str += QString(base + "Buffer D2O fraction (0 to 1): %1\n").arg(saxs_options.d2o_conc);
   }
   if ( saxs_options.frac_of_exch_pep != default_saxs_options.frac_of_exch_pep )
   {
      str += QString(base + "Fraction of non-exchanged peptide H (0 to 1): %1\n").arg(saxs_options.frac_of_exch_pep);
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
   if ( saxs_options.delta_angle != default_saxs_options.delta_angle )
   {
      str += QString(base + "Angle Stepsize: %1\n").arg(saxs_options.delta_angle );
   }
   if ( saxs_options.max_size != default_saxs_options.max_size )
   {
      str += QString(base + "Maximum size: %1\n").arg(saxs_options.max_size );
   }
   if ( saxs_options.bin_size != default_saxs_options.bin_size )
   {
      str += QString(base + "Bin size: %1\n").arg(saxs_options.bin_size );
   }
   if ( saxs_options.hydrate_pdb != default_saxs_options.hydrate_pdb )
   {
      str += QString(base + "Hydrate Original PDB Model: %1\n")
         .arg(saxs_options.hydrate_pdb ? "On" : "Off");
   }
   //   if ( saxs_options.curve != default_saxs_options.curve )
   //   {
   //      str += QString(base + "Curve type: %1\n")
   //         .arg(
   //              saxs_options.curve ? ((saxs_options.curve == 1) ? "SAXS" : "SANS") : "Raw");
   //   }
   //   if ( saxs_options.saxs_sans != default_saxs_options.saxs_sans )
   //   {
   //      str += QString(base + "SAXS or SANS mode: %1\n")
   //         .arg(saxs_options.saxs_sans ? "SANS" : "SAXS");
   //   }
   return str;
}

void US_Hydrodyn::display_default_differences()
{
   QString str =
      default_differences_misc() +
      default_differences_load_pdb() +
      default_differences_somo() +
      default_differences_hydro() +
      default_differences_grid() +
      default_differences_saxs_options();

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
   le_bead_model_suffix->setText(
                                 setSuffix ? (getExtendedSuffix(true, true) + " / " +
                                              getExtendedSuffix(true, false)) : "");
}


#define USE_MPLAYER
// probably should replace with enum of event types
// and a more through matrix of sound events
void US_Hydrodyn::play_sounds(int type)
{
   if ( advanced_config.use_sounds &&
        QSound::available() )
   {
      QString sound_base = USglobal->config_list.root_dir + "sounds/";
      switch (type)
      {
      case 1 :
         {
            QString sf = sound_base + "somo_done.wav";
            if ( QFileInfo(sf).exists() )
            {
#if defined(USE_MPLAYER)
	      QString cmd = QString("mplayer %1&").arg(sf).ascii();
	      cout << cmd << endl;
	      system(cmd);
#else
	      QSound::play(sf);
#endif
            }
            else
            {
               cout << "Can't locate sound file " << sf << endl;
            }
         }
         break;
      default :
         break;
      }
   }
   else
   {
      if ( advanced_config.use_sounds )
      {
         puts("sound not available");
      }
   }
}

void US_Hydrodyn::list_model_vector(vector < PDB_model > *mv)
{
   for ( unsigned int i = 0; i < mv->size(); i++ )
   {
      for ( unsigned int j = 0; j < (*mv)[i].molecule.size(); j++)
      {
         for (unsigned int k = 0; k < (*mv)[i].molecule[j].atom.size(); k++)
         {
            printf("model %u chain %u atom %u atom %s seq %u resName %s chainId %s resSeq %s\n",
                   i, j, k, 
                   (*mv)[i].molecule[j].atom[k].name.ascii(),
                   (*mv)[i].molecule[j].atom[k].serial,
                   (*mv)[i].molecule[j].atom[k].resName.ascii(),
                   (*mv)[i].molecule[j].atom[k].chainID.ascii(),
                   (*mv)[i].molecule[j].atom[k].resSeq.ascii()
                   );
         }
      }
   }
}

bool US_Hydrodyn::install_new_version()
{
   vector < QString > names;
   names.push_back("defaults");
   names.push_back("config");
   names.push_back("hybrid");
   names.push_back("atom");
   names.push_back("saxs_atoms");
   names.push_back("residue");
   names.push_back("hydrated_rotamer");

   vector < bool > install; 
   install.resize(names.size());

   vector < bool > backup;
   backup.resize(names.size());

   vector < QString > fnew;
   fnew.resize(names.size());

   vector < QString > fcur;
   fcur.resize(names.size());

   vector < QString > fprev;
   fprev.resize(names.size());

   // check what needs to be upgraded

   bool any_upgrade = false;
   bool any_backup = false;

   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      fnew[i] = USglobal->config_list.system_dir + "/etc/somo." + names[i] + ".new";
      fcur[i] = USglobal->config_list.system_dir + "/etc/somo." + names[i];
      install[i] = QFile::exists(fnew[i]);
      any_upgrade |= install[i];
      backup[i] = install[i] && QFile::exists(fcur[i]);
      any_backup |= backup[i];
   }

   if ( !any_upgrade ) 
   {
      return true;
   }

   // make sure we come up with a new previous version # so we don't overwrite anything

   bool no_version_increment = false;

   unsigned int version = 1;

   while ( !no_version_increment )
   {
      no_version_increment = true;
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( backup[i] )
         {
            while ( QFile::exists(QString("%1/etc/somo-prev-%2.%3")
                                  .arg(USglobal->config_list.system_dir)
                                  .arg(version)
                                  .arg(names[i])) )
            {
               version++;
               no_version_increment = false;
            }
         }
      }
   }

   // ask to proceed

   QString msg = tr("New versions will be installed for the following files:\n");
#ifdef WIN32
   msg += tr("Note: This step may require you to run as Administrator.\n");
#endif
   for ( unsigned int i = 0; i < names.size(); i++ )
   {
      if ( install[i] )
      {
         msg += QString("\t%1\n").arg(fcur[i]);
      }
   }

   if ( any_backup )
   {
      msg += tr("\nThe existing versions of these files will be backed up as:\n");
      for ( unsigned int i = 0; i < names.size(); i++ )
      {
         if ( backup[i] )
         {
            fprev[i] = 
               QString("%1/etc/somo-prev-%2.%3")
               .arg(USglobal->config_list.system_dir)
               .arg(version)
               .arg(names[i]);
            msg += QString("\t%1\n").arg(fprev[i]);
         }
      }
   }

   msg += tr("\nDo you wish to proceed?");

   switch( QMessageBox::warning( 
                                this, 
                                tr("New version detected"),
                                msg,
                                tr("&OK"),
                                tr("&Cancel"),
                                0, 0, 1 ) ) 
   {
   case 0: 
      {
         QDir qd;
         for ( unsigned int i = 0; i < names.size(); i++ )
         {
            if ( backup[i] )
            {
               printf("backing up %u (<%s> to <%s>\n", i, fcur[i].ascii(), fprev[i].ascii());
               if (!qd.rename(fcur[i], fprev[i]) )
               {
                  QMessageBox::critical( 0, 
                                         tr("Could not rename file"),
                                         QString("An error occured when trying to rename file\n"
                                                 "%1 to %2\n"
                                                 "Please check your permissions and try again\n")
                                         .arg(fcur[i])
                                         .arg(fprev[i])
                                         );
                  exit(-1);
               }
            }
            if ( install[i] )
            {
               printf("installing %u (<%s> to <%s>\n", i, fnew[i].ascii(), fcur[i].ascii());
               if (!qd.rename(fnew[i], fcur[i]) )
               {
                  QMessageBox::critical( 0, 
                                         tr("Could not rename file"),
                                         QString("An error occured when trying to rename file\n"
                                                 "%1 to %2\n"
                                                 "Please check your permissions and try again\n")
                                         .arg(fnew[i])
                                         .arg(fcur[i])
                                         );
                  exit(-1);
               }
            }
         }
      }
      return false;
      break;
   case 1: 
      return false;
      break;
   }
   return false;
}

void US_Hydrodyn::editor_msg( QString color, QString msg )
{
   QColor save_color = editor->color();
   editor->setColor(color);
   editor->append(msg);
   editor->setColor(save_color);
}

bool US_Hydrodyn::check_bead_model_for_nan()
{
   bool issues = false;
   for ( unsigned int i = 0; i < bead_model.size() - 1; i++ ) 
   {
      for ( int j = 0; j < 3; j++ )
      {
         if ( isnan(bead_model[i].bead_coordinate.axis[j]) )
         {
            issues = true;
            editor_msg("red", 
                       QString("WARNING: bead %1 coordinate %2 is NAN!\n")
                       .arg(i+1)
                       .arg(j+1)
                       );
         }
      }
   }
   return issues;
}

bool US_Hydrodyn::is_dammin_dammif(QString filename)
{
   if ( !filename.isEmpty() )
   {
      QFileInfo fi(filename);
      QString dir = fi.dirPath();
      // check for file format
      QFile f( filename );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         return false;
      }
      Q3TextStream ts( &f );

      QString tmp;
      do {
         tmp = ts.readLine();
         if ( tmp.contains(QRegExp("^ATOM ")) &&
              !tmp.contains("CA  ASP ") ) {
            f.close();
            return false;
         }
      } while ( !ts.atEnd() && 
                !tmp.contains("Dummy atoms in output phase") &&
                !tmp.contains("Number of particle atoms") &&
                !tmp.contains("Number of atoms written") &&
                !tmp.contains("Filtered number of atoms") &&
                !tmp.contains("Dummy atom radius")
                );

      if ( ts.atEnd() &
           !tmp.contains("Dummy atoms in output phase") &&
           !tmp.contains("Number of particle atoms") &&
           !tmp.contains("Number of atoms written") &&
           !tmp.contains("Filtered number of atoms") &&
           !tmp.contains("Dummy atom radius")
           )
      {
         f.close();
         return false;
      }
      
      while ( !ts.atEnd() )
      {
         tmp = ts.readLine();
         if ( tmp.contains(QRegExp("^ATOM ")) &&
              !tmp.contains("CA  ASP ") ) {
            f.close();
            return false;
         }
      }
      f.close();
      return true;
   }
   return false;
}

QString US_Hydrodyn::saxs_sans_ext()
{
   QString result;
   switch ( saxs_options.curve )
   {
   case 0 : 
      result = "r";
      break;
   case 1 :
      result = "x";
      break;
   case 2 :
      result = "n";
      break;
   default :
      result = "u";
      break;
   }
   return result;
}

QString US_Hydrodyn::list_chainIDs( vector < PDB_model > &mv )
{
   QString qs;

   for ( unsigned int i = 0; i < mv.size(); i++ )
   {
      for ( unsigned int j = 0; j < mv[i].molecule.size(); j++ )
      {
         if ( mv[i].molecule[j].atom.size() )
         {
            qs += QString("mv %1 mol %2 chainID %3\n")
               .arg(i).arg(j).arg(mv[i].molecule[j].chainID);
         }
         for ( unsigned int k = 0; k < mv[i].molecule[j].atom.size(); j++ )
         {
            PDB_atom *this_atom = &(mv[i].molecule[j].atom[k]);
            qs += QString("mv %1 mol %2 atom 0 chainID %3\n")
               .arg(i).arg(j).arg(this_atom->chainID);
            break;
         }
      }
   }
   return qs;
}


void US_Hydrodyn::save_state()
{
   state_bead_model = bead_model;
   state_bead_models = bead_models;
   state_bead_models_as_loaded = bead_models_as_loaded;
   state_active_atoms = active_atoms;
   state_residue_list = residue_list;
   state_residue_list_no_pbr = residue_list_no_pbr;
   state_multi_residue_map = multi_residue_map;
   state_valid_atom_map = valid_atom_map;
   state_residue_atom_hybrid_map = residue_atom_hybrid_map;
   state_atom_counts = atom_counts;
   state_has_OXT = has_OXT;
   state_bead_exceptions = bead_exceptions;
   state_save_residue_list = save_residue_list;
   state_save_residue_list_no_pbr = save_residue_list_no_pbr;
   state_save_multi_residue_map = save_multi_residue_map;
   state_new_residues = new_residues;
   state_molecules_residues_atoms = molecules_residues_atoms;
   state_molecules_residue_name = molecules_residue_name;
   state_molecules_idx_seq = molecules_idx_seq;
   state_molecules_residue_errors = molecules_residue_errors;
   state_molecules_residue_missing_counts = molecules_residue_missing_counts;
   state_molecules_residue_missing_atoms = molecules_residue_missing_atoms;
   state_molecules_residue_missing_atoms_beads = molecules_residue_missing_atoms_beads;
   state_molecules_residue_missing_atoms_skip = molecules_residue_missing_atoms_skip;
   state_molecules_residue_min_missing = molecules_residue_min_missing;
   state_broken_chain_end = broken_chain_end;
   state_broken_chain_head = broken_chain_head;
   state_unknown_residues = unknown_residues;
   state_use_residue = use_residue;
   state_skip_residue = skip_residue;
   state_last_abb_msgs = last_abb_msgs;
   state_model_vector = model_vector;
   state_model_vector_as_loaded = model_vector_as_loaded;
   state_somo_processed = somo_processed;
   state_options_log = options_log;
   state_pdb_file = pdb_file;
   state_project = project;
   state_current_model = current_model;

   state_lbl_pdb_file = lbl_pdb_file->text();

   state_lb_model_rows.clear();
   for ( unsigned int i = 0; i < (unsigned int)lb_model->numRows(); i++ )
   {
      state_lb_model_rows.push_back(lb_model->text(i));
   }
   editor_msg("dark blue", "State saved\n");
}

void US_Hydrodyn::restore_state()
{
   bead_model = state_bead_model;
   bead_models = state_bead_models;
   bead_models_as_loaded = state_bead_models_as_loaded;
   active_atoms = state_active_atoms;
   residue_list = state_residue_list;
   residue_list_no_pbr = state_residue_list_no_pbr;
   multi_residue_map = state_multi_residue_map;
   valid_atom_map = state_valid_atom_map;
   residue_atom_hybrid_map = state_residue_atom_hybrid_map;
   atom_counts = state_atom_counts;
   has_OXT = state_has_OXT;
   bead_exceptions = state_bead_exceptions;
   save_residue_list = state_save_residue_list;
   save_residue_list_no_pbr = state_save_residue_list_no_pbr;
   save_multi_residue_map = state_save_multi_residue_map;
   new_residues = state_new_residues;
   molecules_residues_atoms = state_molecules_residues_atoms;
   molecules_residue_name = state_molecules_residue_name;
   molecules_idx_seq = state_molecules_idx_seq;
   molecules_residue_errors = state_molecules_residue_errors;
   molecules_residue_missing_counts = state_molecules_residue_missing_counts;
   molecules_residue_missing_atoms = state_molecules_residue_missing_atoms;
   molecules_residue_missing_atoms_beads = state_molecules_residue_missing_atoms_beads;
   molecules_residue_missing_atoms_skip = state_molecules_residue_missing_atoms_skip;
   molecules_residue_min_missing = state_molecules_residue_min_missing;
   broken_chain_end = state_broken_chain_end;
   broken_chain_head = state_broken_chain_head;
   unknown_residues = state_unknown_residues;
   use_residue = state_use_residue;
   skip_residue = state_skip_residue;
   last_abb_msgs = state_last_abb_msgs;
   model_vector = state_model_vector;
   model_vector_as_loaded = state_model_vector_as_loaded;
   somo_processed = state_somo_processed;
   options_log = state_options_log;
   pdb_file = state_pdb_file;
   project = state_project;
   current_model = state_current_model;

   lbl_pdb_file->setText( state_lbl_pdb_file );

   lb_model->clear();
   for ( unsigned int i = 0; i < state_lb_model_rows.size(); i++ )
   {
      lb_model->insertItem(state_lb_model_rows[i]);
   }
   if ( state_lb_model_rows.size() )
   {
      lb_model->setSelected(0, true);
   }
   editor_msg("dark blue", "Saved state restored\n");
}

void US_Hydrodyn::clear_state()
{
   state_bead_model.clear();
   state_bead_models.clear();
   state_bead_models_as_loaded.clear();
   state_active_atoms.clear();
   state_residue_list.clear();
   state_residue_list_no_pbr.clear();
   state_multi_residue_map.clear();
   state_valid_atom_map.clear();
   state_residue_atom_hybrid_map.clear();
   state_atom_counts.clear();
   state_has_OXT.clear();
   state_bead_exceptions.clear();
   state_save_residue_list.clear();
   state_save_residue_list_no_pbr.clear();
   state_save_multi_residue_map.clear();
   state_new_residues.clear();
   state_molecules_residues_atoms.clear();
   state_molecules_residue_name.clear();
   state_molecules_idx_seq.clear();
   state_molecules_residue_errors.clear();
   state_molecules_residue_missing_counts.clear();
   state_molecules_residue_missing_atoms.clear();
   state_molecules_residue_missing_atoms_beads.clear();
   state_molecules_residue_missing_atoms_skip.clear();
   state_molecules_residue_min_missing.clear();
   state_broken_chain_end.clear();
   state_broken_chain_head.clear();
   state_unknown_residues.clear();
   state_use_residue.clear();
   state_skip_residue.clear();
   state_model_vector.clear();
   state_model_vector_as_loaded.clear();
   state_somo_processed.clear();
   state_lb_model_rows.clear();

   state_last_abb_msgs = "";
   state_options_log = "";
   state_pdb_file = "";
   state_project = "";

   state_lbl_pdb_file = "";
   state_current_model = 0;
   editor_msg("dark blue", "Saved state cleared\n");
}


void US_Hydrodyn::rescale_bead_model()
{
   for ( current_model = 0; current_model < (unsigned int)lb_model->numRows(); current_model++ ) 
   {
      if ( lb_model->isSelected(current_model) &&
           somo_processed[current_model] ) 
      {
         bead_model = bead_models[current_model];
         if ( misc.target_volume != 0e0 )
         {
            editor->append(QString("Rescaling bead model %1\n").arg( model_name( current_model ) ) );
            double current_volume = total_volume_of_bead_model( bead_model );
            editor_msg("black", 
                       QString( tr( "Current volume %1 A^3, target volume %2 A^3\n") )
                       .arg( current_volume )
                       .arg( misc.target_volume ) );
            if ( QString("%1").arg( current_volume ) == QString("%1").arg( misc.target_volume ) )
            {
               editor_msg("blue", tr("Skipped, volume already equal") );
            } else {
               double multiplier = pow( misc.target_volume / current_volume, 1e0 / 3e0 );
               for ( unsigned int i = 0; i < bead_model.size(); i++ )
               {
                  if ( bead_model[ i ].active ) 
                  {
                     bead_model[ i ].bead_computed_radius *= multiplier;
                  }
               }
               current_volume = total_volume_of_bead_model( bead_model );
               editor_msg("black", 
                          QString( tr( "After rescaling: current volume %1 A^3, target volume %2 A^3\n") )
                          .arg( current_volume )
                          .arg( misc.target_volume ) );
               bead_models[ current_model ] = bead_model;
            }
         }
         if ( misc.equalize_radii )
         {
            editor->append(QString("Equalizing radii for bead model %1\n").arg( model_name( current_model ) ) );
            if ( radii_all_equal( bead_model ) )
            {
               editor_msg("blue", tr("Skipped, radii already equalized") );
            } else {
               double current_volume = total_volume_of_bead_model( bead_model );
               unsigned int beads    = number_of_active_beads( bead_model );
               double pi43           = M_PI * 4e0 / 3e0;
               
               // current volume = beads * pi43 * constant-r^3
               float radius = (float)pow( current_volume / ( beads * pi43 ), 1e0 / 3e0 );
               
               editor_msg("black", 
                          QString( tr( "Volume %1 A^3, Number of beads %2, Radius %3 A\n" ) )
                          .arg( current_volume )
                          .arg( beads )
                          .arg( radius )
                          );

               for ( unsigned int i = 0; i < bead_model.size(); i++ )
               {
                  if ( bead_model[ i ].active ) 
                  {
                     bead_model[ i ].bead_computed_radius = radius;
                  }
               }
            
               current_volume = total_volume_of_bead_model( bead_model );
               editor_msg("black", 
                          QString( tr( "After equalizing: current volume %1 A^3\n" ) )
                          .arg( current_volume ) );
               bead_models[ current_model ] = bead_model;
            }
         }
      }
   }
}


csv US_Hydrodyn::pdb_to_csv( vector < PDB_model > &model_vector )
{
   csv csv1;
   
   csv1.name        = last_pdb_filename;
   csv1.filename    = last_pdb_filename;
   csv1.title_text  = last_pdb_title;
   csv1.header_text = last_pdb_header;

   csv1.header.push_back("Model");
   csv1.header.push_back("Chain");
   csv1.header.push_back("Residue");
   csv1.header.push_back("Residue Number");
   csv1.header.push_back("Atom");
   csv1.header.push_back("Atom Number");
   csv1.header.push_back("Alt");
   csv1.header.push_back("iC");
   csv1.header.push_back("X");
   csv1.header.push_back("Y");
   csv1.header.push_back("Z");
   csv1.header.push_back("Occ");
   csv1.header.push_back("TF");
   csv1.header.push_back("Ele");
   // csv1.header.push_back("Charge");
   // ?? csv1.header.push_back("Accessibility");

   for (unsigned int i = 0; i < model_vector.size(); i++)
   {
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
      {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
         {
            vector < QString > data;
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            
            data.push_back( QString("%1").arg( i + 1 ) );
            data.push_back( this_atom->chainID );
            data.push_back( this_atom->resName );
            data.push_back( this_atom->resSeq );
            data.push_back( this_atom->orgName );
            data.push_back( QString("%1").arg( this_atom->serial ) );
            data.push_back( this_atom->altLoc );
            data.push_back( this_atom->iCode );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 0 ] ) );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 1 ] ) );
            data.push_back( QString("%1").arg( this_atom->coordinate.axis[ 2 ] ) );
            data.push_back( QString("%1").arg( this_atom->occupancy ) );
            data.push_back( QString("%1").arg( this_atom->tempFactor ) );
            data.push_back( this_atom->element );
            // data.push_back( this_atom->charge );
            // ?? data.push_back( QString("%1").arg( this_atom->accessibility ) );

            csv1.data.push_back( data );
         }
      }
   }
   return csv1;
}

void US_Hydrodyn::save_pdb_csv( csv &csv1 )
{

   QString use_dir = 
      USglobal->config_list.root_dir + SLASH + "somo" + SLASH + "structures";
   QString filename = Q3FileDialog::getSaveFileName(
                                                   use_dir,
                                                   "*.pdc *.pdc",
                                                   this,
                                                   "save file dialog",
                                                   tr("Choose a filename to save the pdc") );


   if ( filename.isEmpty() )
   {
      return;
   }

   if ( !filename.contains(QRegExp(".pdc$",false)) )
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
                            QString(tr("Could not open %1 for writing!")).arg(filename) );
      return;
   }

   Q3TextStream t( &f );

   QString qs;

   for ( unsigned int i = 0; i < csv1.header.size(); i++ )
   {
      qs += QString("%1\"%2\"").arg(i ? "," : "").arg(csv1.header[i]);
   }

   t << qs << endl;

   for ( unsigned int i = 0; i < csv1.data.size(); i++ )
   {
      qs = "";
      for ( unsigned int j = 0; j < csv1.data[i].size(); j++ )
      {
         qs += QString("%1%2").arg(j ? "," : "").arg(csv1.data[i][j]);
      }
      t << qs << endl;
   }
   f.close();
}

void US_Hydrodyn::pdb_tool()
{
   csv csv1 = pdb_to_csv( model_vector );
   // save_pdb_csv( csv1 );
   US_Hydrodyn_Pdb_Tool 
      *pdb_tool_window =
      new US_Hydrodyn_Pdb_Tool( csv1, this );
   US_Hydrodyn::fixWinButtons( pdb_tool_window );
   pdb_tool_window->show();
}


void US_Hydrodyn::config()
{
   QStringList menu_opts;
   static int last_menu = 0;

   menu_opts
      << tr( "Lookup Tables -> Add/Edit Hybridization" ) // hybrid()
      << tr( "Lookup Tables -> Add/Edit Atom" ) // edit_atom()
      << tr( "Lookup Tables -> Add/Edit Residue" ) // residue()
      << tr( "Lookup Tables -> Add/Edit SAXS coefficients" ) // saxs()
      << tr( "SOMO Options -> ASA Calculation" ) // show_asa()
      << tr( "SOMO Options -> SoMo Overlap Reduction" ) // show_overlap()
      << tr( "SOMO Options -> AtoB (Grid) Overlap Reduction" ) // show_grid_overlap()
      << tr( "SOMO Options -> Hydrodynamic Calculations" ) // show_hydro()
      << tr( "SOMO Options -> Hydrodynamic Calculations Zeno" ) // show_zeno_options()
      << tr( "SOMO Options -> Miscellaneous Options" ) // show_misc()
      << tr( "SOMO Options -> Bead Model Output" ) // show_bead_output()
      << tr( "SOMO Options -> Grid Functions (AtoB)" ) // show_grid()
      << tr( "SOMO Options -> SAXS/SANS Options" ) // show_saxs_options()
      // << tr( "MD Options -> DMD Options" ) // show_dmd_options()
      << tr( "MD Options -> Browflex Options" ) // show_bd_options()
      << tr( "MD Options -> Anaflex Options" ) // show_anaflex_options()
      << tr( "PDB Options -> Parsing" ) // pdb_parsing()
      << tr( "PDB Options -> Visualization" ) // pdb_visualization()
      << tr( "Configuration -> Load Configuration" ) // load_config()
      << tr( "Configuration -> Save Current Configuration" ) // write_config()
      << tr( "Configuration -> Reset to Default Configuration" ) // reset()
      << tr( "Configuration -> Advanced Configuration" ) // show_advanced_config()
      << tr( "Configuration -> System Configuration" ) // run_us_config()
      << tr( "Configuration -> Administrator" ) // run us_admin()
      ;

   bool ok;
   QString res = QInputDialog::getItem(
                                       tr( "US-SOMO Configuration Options" ), 
                                       tr( "Make a selection or press CANCEL" ),
                                       menu_opts, 
                                       last_menu, 
                                       TRUE, 
                                       &ok,
                                       this );

   if ( ok ) {
      int pos   = 0;
      if ( res == tr( "Lookup Tables -> Add/Edit Hybridization" ) )
      {
         last_menu = pos;
         hybrid();
      }
      pos++;
      if ( res == tr( "Lookup Tables -> Add/Edit Atom" ) )
      {
         last_menu = pos;
         edit_atom();
      }
      pos++;
      if ( res == tr( "Lookup Tables -> Add/Edit Residue" ) )
      {
         last_menu = pos;
         residue();
      }
      pos++;
      if ( res == tr( "Lookup Tables -> Add/Edit SAXS coefficients" ) )
      {
         last_menu = pos;
         saxs();
      }
      pos++;
      if ( res == tr( "SOMO Options -> ASA Calculation" ) )
      {
         last_menu = pos;
         show_asa();
      }
      pos++;
      if ( res == tr( "SOMO Options -> SoMo Overlap Reduction" ) )
      {
         last_menu = pos;
         show_overlap();
      }
      pos++;
      if ( res == tr( "SOMO Options -> AtoB (Grid) Overlap Reduction" ) )
      {
         last_menu = pos;
         show_grid_overlap();
      }
      pos++;
      if ( res == tr( "SOMO Options -> Hydrodynamic Calculations" ) )
      {
         last_menu = pos;
         show_hydro();
      }
      pos++;
      if ( res == tr( "SOMO Options -> Hydrodynamic Calculations Zeno" ) )
      {
         last_menu = pos;
         show_zeno_options();
      }
      pos++;
      if ( res == tr( "SOMO Options -> Miscellaneous Options" ) )
      {
         last_menu = pos;
         show_misc();
      }
      pos++;
      if ( res == tr( "SOMO Options -> Bead Model Output" ) )
      {
         last_menu = pos;
         show_bead_output();
      }
      pos++;
      if ( res == tr( "SOMO Options -> Grid Functions (AtoB)" ) )
      {
         last_menu = pos;
         show_grid();
      }
      pos++;
      if ( res == tr( "SOMO Options -> SAXS/SANS Options" ) )
      {
         last_menu = pos;
         show_saxs_options();
      }
      pos++;
      if ( res == tr( "MD Options -> DMD Options" ) )
      {
         last_menu = pos;
         show_dmd_options();
      }
      pos++;
      if ( res == tr( "MD Options -> Browflex Options" ) )
      {
         last_menu = pos;
         show_bd_options();
      }
      pos++;
      if ( res == tr( "MD Options -> Anaflex Options" ) )
      {
         last_menu = pos;
         show_anaflex_options();
      }
      pos++;
      if ( res == tr( "PDB Options -> Parsing" ) )
      {
         last_menu = pos;
         pdb_parsing();
      }
      pos++;
      if ( res == tr( "PDB Options -> Visualization" ) )
      {
         last_menu = pos;
         pdb_visualization();
      }
      pos++;
      if ( res == tr( "Configuration -> Load Configuration" ) )
      {
         last_menu = pos;
         load_config();
      }
      pos++;
      if ( res == tr( "Configuration -> Save Current Configuration" ) )
      {
         last_menu = pos;
         write_config();
      }
      pos++;
      if ( res == tr( "Configuration -> Reset to Default Configuration" ) )
      {
         last_menu = pos;
         reset();
      }
      pos++;
      if ( res == tr( "Configuration -> Advanced Configuration" ) )
      {
         last_menu = pos;
         show_advanced_config();
      }
      pos++;
      if ( res == tr( "Configuration -> System Configuration" ) )
      {
         last_menu = pos;
         run_us_config();
      }
      pos++;
      if ( res == tr( "Configuration -> Administrator" ) )
      {
         last_menu = pos;
         run_us_admin();
      }
   }
}

void US_Hydrodyn::reset_chain_residues( PDB_model *model )
{
   // build_molecule_maps( &model_vector[ i ] );
   map < QString, bool > checked;

   for ( unsigned int j = 0; j < model->molecule.size(); j++ )
   {
      for (unsigned int k = 0; k < model->molecule[ j ].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model->molecule[ j ].atom[ k ]);
         QString idx = QString("%1|%2").arg( j ).arg( this_atom->resSeq );
         if ( !checked.count( idx ) )
         {
            checked[ idx ] = true;
            if ( this_atom->p_residue &&
                 this_atom->model_residue_pos != -1 &&
                 ( int ) model->residue.size() < this_atom->model_residue_pos )
            {
               if ( model->residue[ this_atom->model_residue_pos ].unique_name != this_atom->p_residue->unique_name )
               {
                  cout << QString( "found residue difference %1 %2, fixing\n" )
                     .arg( model->residue[ this_atom->model_residue_pos ].unique_name )
                     .arg( this_atom->p_residue->unique_name );
                  model->residue[ this_atom->model_residue_pos ] = *this_atom->p_residue;                        
               }
            }
         }
      }
   }
   calc_vbar( model );
}

void US_Hydrodyn::make_test_set()
{
   cout << "make test set\n";
   QRegExp count_hydrogens("H(\\d)");

   QString test_dir = somo_dir + QDir::separator() + "testset";
   QDir dir1( test_dir );
   if ( !dir1.exists() )
   {
      editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( test_dir ) );
      dir1.mkdir( test_dir );
   }

   QStringList ds;
   ds 
      << "05"
      << "10"
      << "20"
      << "40"
      << "80"
      << "99"
      ;
      
   map < QString, vector < QString > > pair_summary;

   for ( unsigned int d = 0; d < ( unsigned int ) ds.size(); d++ )
   {
      for ( unsigned int i = 0; i < ( unsigned int ) residue_list.size(); i++ )
      {
         QString residue_name = residue_list[ i ].name;
         if ( residue_short_names.count( residue_name ) &&
              residue_short_names[ residue_name ] != '~' )
         {
            QString residue_short_name = residue_short_names[ residue_name ];
            for ( unsigned int j = 0; j < ( unsigned int ) residue_list[ i ].r_atom.size(); j++ )
            {
               int hydrogens = 0;
               QString atom_name   = residue_list[ i ].r_atom[ j ].name;
               if ( !atom_name.contains( "'" ) &&
                    !atom_name.contains( "*" ) )
               {
                  QString atom_name_1 = atom_name.left( 1 );
                  QString hybrid_name = residue_list[ i ].r_atom[ j ].hybrid.name;
                  if ( count_hydrogens.search( hybrid_name ) != -1 )
                  {
                     hydrogens = count_hydrogens.cap( 1 ).toInt();
                  }

                  QDir::setCurrent( test_dir );

                  QString atom_dir = test_dir + QDir::separator() + atom_name_1;
                  {
                     QDir dir1( atom_dir );
                     if ( !dir1.exists() )
                     {
                        editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( atom_dir ) );
                        dir1.mkdir( atom_dir );
                     }
                     QDir::setCurrent( atom_dir );
                  }
                  
                  QString hydrogen_dir = atom_dir + QDir::separator() + QString( "%1" ).arg( hydrogens );
                  {
                     QDir dir1( hydrogen_dir  );
                     if ( !dir1.exists() )
                     {
                        editor_msg( "black", QString( tr( "Created directory %1" ) ).arg( hydrogen_dir ) );
                        dir1.mkdir( hydrogen_dir );
                     }
               
                     QDir::setCurrent( hydrogen_dir );
                  }

                  QString fname = QString( "%1%2%3%4.pdb" )
                     .arg( hydrogens )
                     .arg( QString( "%1" ).arg( ds[ d ] ).stripWhiteSpace() )
                     .arg( residue_short_name )
                     .arg( atom_name )
                     ;

                  cout << QString( "%1 %2 %3 %4 %5 %6 %7\n" )
                     .arg( fname )
                     .arg( residue_name )
                     .arg( residue_short_name )
                     .arg( atom_name )
                     .arg( atom_name_1 )
                     .arg( hybrid_name )
                     .arg( hydrogens );

                  QFile f( fname );
                  if ( f.open( QIODevice::WriteOnly ) )
                  {
                     Q3TextStream ts( &f );
                     ts << 
                        QString( "" )
                        .sprintf(     
                                 "ATOM      1  %-3s %3s     1       0.000   0.000   0.000  1.00 14.00           %1s\n"
                                 , atom_name.ascii()
                                 , residue_name.ascii()
                                 , atom_name_1.ascii() );

                     ts << 
                        QString( "" )
                        .sprintf(     
                                 "ATOM      2  %-3s %3s     2      %2s.000   0.000   0.000  1.00 14.00           %1s\n"
                                 , atom_name.ascii()
                                 , residue_name.ascii()
                                 , ds[ d ].ascii()
                                 , atom_name_1.ascii() );
                     f.close();
                  }
                  if ( !d )
                  {
                     pair_summary[ QString( "%1H%2" ).arg( atom_name_1 ).arg( hydrogens ) ]
                        .push_back( QString( "%1 %2" ).arg( residue_name ).arg( atom_name ) );
                  }
               }
            }
         }
      }
   }

   // save pair summary
   QDir::setCurrent( test_dir );
   QFile f( "pair_summary" );
   if ( f.open( QIODevice::WriteOnly ) )
   {
      Q3TextStream ts( &f );
      for ( map < QString, vector < QString > >::iterator it = pair_summary.begin();
            it != pair_summary.end();
            it++ )
      {
         ts << "# " << it->first << endl;
         ts << "residueatom ";
         for ( unsigned int i = 0; i < ( unsigned int ) it->second.size(); i++ )
         {
            ts << it->second[ i ] << " ";
         }
         ts << endl;
      }
      f.close();
   }
}

void US_Hydrodyn::calc_vol_for_saxs()
{
   // cout << QString( "calc_vol_for_saxs() model_vector.size() %1\n" ).arg( model_vector.size() );
   saxs_util->setup_saxs_options();

   for ( unsigned int i = 0; i < model_vector.size(); i++  )
   {
      model_vector[i].volume = 0;
      for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            double excl_vol;
            double scaled_excl_vol;
            unsigned int this_e;
            unsigned int this_e_noh;
            double si = 0e0;
            if ( !saxs_util->set_excluded_volume( *this_atom, 
                                                  excl_vol, 
                                                  scaled_excl_vol, 
                                                  saxs_options, 
                                                  residue_atom_hybrid_map,
                                                  this_e,
                                                  this_e_noh,
                                                  si ) )
            {
               editor_msg( "dark red", saxs_util->errormsg );
            } else {
               this_atom->saxs_excl_vol = excl_vol;
               this_atom->si            = si;
               model_vector_as_loaded[ i ].molecule[ j ].atom[ k ].si = si;
               model_vector[ i ].volume += excl_vol;
            } 
         }
      }
      editor_msg( "dark blue", QString( "Model %1 total volume %2" )
                  .arg( model_vector[ i ].model_id )
                  .arg( model_vector[ i ].volume ) );
   }
}

bool US_Hydrodyn::select_from_directory_history( QString &dir, QWidget *parent, bool select_dir )
{
   if ( !directory_history.size() ||
        ( directory_history.size() == 1 && 
          directory_history.contains( dir ) ) )
   {
      return false;
   }

   // QStringList use_history;

   bool    is_ok;
   QString selected = dir;

   QFileInfo fi( dir );
   if ( fi.exists() )
   {
      selected = QDir::cleanDirPath( fi.isDir() ? fi.filePath() : fi.dirPath() );
   }
         
   for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
   {
      if ( directory_history[ i ] == selected )
      {
         selected = "";
         break;
      }
   }

   if ( !selected.isEmpty() )
   {
      add_to_directory_history( selected, false );
   }

   if ( !select_dir )
   {
      selected = "";
   }

   US_Dirhist * usd = new US_Dirhist( directory_history,
                                      directory_last_access,
                                      directory_last_filetype,
                                      selected,
                                      is_ok,
                                      parent ? parent : this
                                      );
   usd->exec();
   if ( is_ok )
   {
      dir = selected;
      return true;
   } 
   return false;

   // bool ok;
   // QString res = QInputDialog::getItem(
   //                                     tr("Previous directories"),
   //                                     QString( tr("Select the directory or Cancel for the default directory of\n%1") )
   //                                     .arg( dir )
   //                                     , 
   //                                     use_history,
   //                                     current, 
   //                                     FALSE, 
   //                                     &ok,
   //                                     parent ? parent : this );
   // if ( ok ) {
   //    dir = res;
   //    return true;
   // } 
   // return false;
}
   
void US_Hydrodyn::add_to_directory_history( QString filename, bool accessed )
{
   QFileInfo fi( filename );

   QString dir = QDir::cleanDirPath( fi.isDir() ? fi.filePath() : fi.dirPath() );

   // qDebug( QString( "add to dir history %1 %2 %3" ).arg( filename ).arg( dir ).arg( accessed ? "true" : "false" ) );
   if ( dir.isEmpty() ||
        dir.contains( QRegExp( "^\\." ) ) )
   {
      return;
   }

   // push to top
   QStringList new_dir_history;
   new_dir_history << dir;
   if ( accessed )
   {
      directory_last_access  [ dir ] = QDateTime::currentDateTime();
      directory_last_filetype[ dir ] = fi.extension( false );
   }
   for ( unsigned int i = 0; i < (unsigned int) directory_history.size(); i++ )
   {
      if ( directory_history[ i ] != dir )
      {
         new_dir_history << directory_history[ i ];
      }
   }
   directory_history = new_dir_history;
   //   qDebug( "new directory history: " + directory_history.join(":") );
}
