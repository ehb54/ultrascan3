#include "../include/us_saxs_util.h"
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

bool US_Saxs_Util::select_residue_file( QString filename )
{
   errormsg = "";
   noticemsg = "";

   QString str1;
   QString str2;
   unsigned int numatoms;
   unsigned int numbeads;
   // unsigned int i;
   unsigned int j;
   unsigned int positioner;

   QFile f( filename );
   int line_count = 1;

   residue_list.clear( );
   residue_list_no_pbr.clear( );
   multi_residue_map.clear( );
   residue_atom_hybrid_map.clear( );
   new_residues.clear( );
   map < QString, int > dup_residue_map;
   map < QString, bool > pbr_override_map; // maps positioner for overwrite
   unknown_residues.clear( ); // keep track of unknown residues
   // i = 1;

   residue new_residue;
   atom new_atom;
   bead new_bead;

   if ( f.open( QIODevice::ReadOnly ) )
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
         new_residue.r_atom.clear( );
         new_residue.r_bead.clear( );
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
            if (new_atom.bead_assignment >= numbeads)
            {
               noticemsg += 
                  QString(
                          "Warning: The atom's bead assignment has exceeded the number of beads."
                          "For residue: %1 and Atom: %2 "
                          "on line %3 of the residue file.\n" )
                  .arg(new_residue.comment)
                  .arg(new_atom.name)
                  .arg(line_count)
                  ;
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
               noticemsg += "Notice: There was an error reading the selected Residue File. Atom"
                  + new_atom.name + " cannot be read and will be deleted from List.\n";
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
         errormsg = "Inconsistant residue lists - internal error.";
         return false;
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
      noticemsg += "Warning: PBR rule OXT not replaced with OXT-P because there is not exactly 1 OXT and 1 PBR-OXT\n";
   }

   save_residue_list = residue_list;
   save_residue_list_no_pbr = residue_list_no_pbr;
   save_multi_residue_map = multi_residue_map;

   return true;
}

void US_Saxs_Util::calc_bead_mw( residue *res )
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
         }
      }
   }
}

void US_Saxs_Util::calc_vbar( PDB_model *model )
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
   //cout << "VBAR: " << (*model).vbar << endl;
}

bool US_Saxs_Util::setup_saxs_maps( QString atom_file, QString hybrid_file, QString saxs_file )
{
   return 
      select_atom_file( atom_file ) &&
      select_hybrid_file( hybrid_file ) &&
      select_saxs_file( saxs_file );
}

bool US_Saxs_Util::select_atom_file( QString filename )
{
   errormsg = "";
   QString str1;
   QFileInfo fi(filename);
   atom_list.clear( );
   atom_map.clear( );
   QFile f(filename);
   if ( f.open(QIODevice::ReadOnly ) )
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_atom.name;
         ts >> current_atom.hybrid.name;
         ts >> current_atom.hybrid.mw;
         ts >> current_atom.hybrid.radius;
         ts >> current_atom.saxs_excl_vol;
         str1 = ts.readLine(); // read rest of line
         if (!current_atom.name.isEmpty() && current_atom.hybrid.radius > 0.0 && current_atom.hybrid.mw > 0.0)
         {
            atom_list.push_back(current_atom);
            atom_map[current_atom.name + "~" + current_atom.hybrid.name] = current_atom;
         }
      }
      f.close();
      return true;
   } else {
      errormsg = QString( "Error can not read atom file %1" ).arg( filename );
      return false;
   }
}

bool US_Saxs_Util::select_hybrid_file( QString filename )
{
   errormsg = "";
   QString str1;
   QFileInfo fi(filename);
   QFile f(filename);
   hybrid_list.clear( );
   hybrid_map.clear( );
   if ( f.open(QIODevice::ReadOnly) )
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         ts >> current_hybrid.saxs_name;
         ts >> current_hybrid.name;
         ts >> current_hybrid.mw;
         ts >> current_hybrid.radius;
         ts >> current_hybrid.scat_len;
         ts >> current_hybrid.exch_prot;
         ts >> current_hybrid.num_elect;
         str1 = ts.readLine(); // read rest of line
         if (!current_hybrid.name.isEmpty() && current_hybrid.radius > 0.0 && current_hybrid.mw > 0.0)
         {
            hybrid_list.push_back(current_hybrid);
            hybrid_map[current_hybrid.name] = current_hybrid;
         }
      }
      f.close();
      return true;
   } else {
      errormsg = QString( "Error can not read hybrid file %1" ).arg( filename );
      return false;
   }
}

bool US_Saxs_Util::select_saxs_file( QString filename )
{
   errormsg = "";
   QString str1;
   QFileInfo fi(filename);
   QFile f(filename);
   saxs_list.clear( );
   saxs_map.clear( );
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      int line = 0;
      while (!ts.atEnd())
      {
         QString    qs  = ts.readLine();
         line++;
         if ( qs.contains( QRegExp( "^\\s+#" ) ) )
         {
            continue;
         }
         qs.trimmed();
         QStringList qsl = (qs ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
         int pos = 0;
         if ( qsl.size() == 11 )
         {
            current_saxs.saxs_name = qsl[ pos++ ];
            if ( saxs_map.count( current_saxs.saxs_name ) )
            {
               current_saxs = saxs_map[ current_saxs.saxs_name ];
            }
            for ( int j = 0; j < 4; j++ )
            {
               current_saxs.a[ j ] = qsl[ pos++ ].toFloat();
               current_saxs.b[ j ] = qsl[ pos++ ].toFloat();
            }
            current_saxs.c      = qsl[ pos++ ].toFloat();
            current_saxs.volume = qsl[ pos++ ].toFloat();
            saxs_map[ current_saxs.saxs_name ] = current_saxs;
            continue;
         } 

         if ( qsl.size() == 13 )
         {
            current_saxs.saxs_name = qsl[ pos++ ];
            if ( saxs_map.count( current_saxs.saxs_name ) )
            {
               current_saxs = saxs_map[ current_saxs.saxs_name ];
            }
            for ( int j = 0; j < 5; j++ )
            {
               current_saxs.a5[ j ] = qsl[ pos++ ].toFloat();
               current_saxs.b5[ j ] = qsl[ pos++ ].toFloat();
            }
            current_saxs.c5     = qsl[ pos++ ].toFloat();
            current_saxs.volume = qsl[ pos++ ].toFloat();
            saxs_map[ current_saxs.saxs_name ] = current_saxs;
            continue;
         } 

         cout <<  
            QString(  "Warning: %1 on line %2, invalid number of tokens, ignoring" )
            .arg( filename )
            .arg( line );
      }
      
      for ( map < QString, saxs >::iterator it = saxs_map.begin();
            it != saxs_map.end();
            it++ )
      {
         it->second.si = 
            it->second.c +
            it->second.a[ 0 ] +
            it->second.a[ 1 ] +
            it->second.a[ 2 ] +
            it->second.a[ 3 ];
         it->second.si *= it->second.si;
         saxs_list.push_back( it->second );
      }
      f.close();
      return true;
   } else {
      errormsg = QString( "Error can not read saxs atom file %1" ).arg( filename );
      return false;
   }
}

bool US_Saxs_Util::read_pdb( QStringList &qsl )
{
   errormsg = "";
   noticemsg = "";

   if ( misc_pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }

   QString str;
   QString str1;
   QString str2;
   QString temp;
   model_vector.clear( );
   bead_model.clear( );
   QString last_resSeq = ""; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
   PDB_chain temp_chain;
   struct PDB_model temp_model;
   bool chain_flag = false;
   bool model_flag = false;
   temp_model.molecule.clear( );
   temp_model.residue.clear( );
   clear_temp_chain(&temp_chain);
   bool currently_aa_chain = false; // do we have an amino acid chain (pbr)
   bool last_was_ENDMDL = false;    // to fix pdbs with missing MODEL tag

   last_pdb_header.clear( );
   last_pdb_title .clear( );
   last_pdb_filename = "FromQStringList";

   for ( unsigned int i = 0; i < (unsigned int) qsl.size(); i++ )
   {
      str1 = qsl[ i ];
      if ( str1.left(3) == "TER" )
      {
         // push back previous chain if it exists
         if ( chain_flag )
         {
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
      }
      if (str1.left(5) == "MODEL" ||
          (str1.left(4) == "ATOM" && last_was_ENDMDL) ) // we have a new model in a multi-model file
      {
         last_was_ENDMDL = false;
         model_flag = true; // we are using model descriptions (possibly multiple models)
         QRegExp rx_get_model( "^MODEL\\s+(\\S+)" );
         // str2 = str1.mid(6, 15);
         // temp_model.model_id = str2.toUInt();
         if ( rx_get_model.indexIn( str1 ) != -1 )
         {
            temp_model.model_id = rx_get_model.cap( 1 );
         } else {
            temp_model.model_id = str1.mid( 6, 15 );
         }
         chain_flag = false; // we are starting a new molecule
         temp_model.molecule.clear( );
         temp_model.residue.clear( );
         clear_temp_chain(&temp_chain);
      }
      if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
      {
         last_was_ENDMDL = true;
         temp_model.molecule.push_back(temp_chain); // add the last chain of this model
         // noticemsg += "Residue sequence from model " +
         // QString("%1").arg( model_vector.size() + 1 ) + ": \n";
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
         // noticemsg += str + "\n";
         
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
                  noticemsg += QString("Atom %1 conformation A selected\n").arg(str1.mid(6,5));
               }
               if (!chain_flag)    // at the first time we encounter the word ATOM
               {             // we don't have a chain yet, so let's start a new one
                  temp_chain.chainID = str1.mid(21, 1);
                  str2 = str1.mid(72, 4);
                  temp_chain.segID = str2.trimmed();
                  chain_flag = true;
               }
               else // we have a chain, let's make sure the chain is still the same
               {
                  bool break_chain = ( temp_chain.chainID != str1.mid(21, 1) );
                  QString thisResName = str1.mid(17,3).trimmed();
                  bool known_residue = ( multi_residue_map.count(thisResName) );
                  bool this_is_aa =  ( known_residue &&
                                       residue_list[multi_residue_map[thisResName][0]].type == 0 );
                  if ( !break_chain  && 
                       currently_aa_chain &&
                       known_residue &&
                       !this_is_aa )
                  {
                     break_chain = true;
                  } 
                  if ( break_chain )
                  {
                     if ( temp_chain.atom.size() ) 
                     {
                        temp_model.molecule.push_back(temp_chain);
                     }
                     clear_temp_chain(&temp_chain);
                     temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                     str2 = str1.mid(72, 4);
                     temp_chain.segID = str2.trimmed();
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
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      temp_model.molecule.push_back(temp_chain);
      noticemsg += "Residue sequence:";
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
      noticemsg += str;
      temp_model.model_id = "1";
      // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
      calc_vbar(&temp_model); // update the calculated vbar for this model
      model_vector.push_back(temp_model);
      clear_temp_chain(&temp_chain);
   }
   if ( !dna_rna_resolve() )
   {
      return false;
   }
   model_vector_as_loaded = model_vector;
   // cout << list_chainIDs(model_vector);
   // cout << list_chainIDs(model_vector_as_loaded);
   if ( !calc_mw() )
   {
      return false;
   }
   return true;
}

bool US_Saxs_Util::read_pdb( QString filename )
{
   errormsg = "";
   noticemsg = "";
   //cout << QString( "read pdb <%1>\n" ).arg( filename );

   if ( misc_pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }

   QString str;
   QString str1;
   QString str2;
   QString temp;
   model_vector.clear( );
   bead_model.clear( );
   QString last_resSeq = ""; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
   PDB_chain temp_chain;
   QFile f(filename);
   struct PDB_model temp_model;
   bool chain_flag = false;
   bool model_flag = false;
   temp_model.molecule.clear( );
   temp_model.residue.clear( );
   clear_temp_chain(&temp_chain);
   bool currently_aa_chain = false; // do we have an amino acid chain (pbr)
   bool last_was_ENDMDL = false;    // to fix pdbs with missing MODEL tag

   if ( f.open(QIODevice::ReadOnly) )
   {
      last_pdb_header.clear( );
      last_pdb_title .clear( );
      last_pdb_filename = f.fileName();
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         str1 = ts.readLine();
         if ( str1.left(3) == "TER" )
         {
            // push back previous chain if it exists
            if ( chain_flag )
            {
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
         }
         if (str1.left(5) == "MODEL" ||
             (str1.left(4) == "ATOM" && last_was_ENDMDL) ) // we have a new model in a multi-model file
         {
            last_was_ENDMDL = false;
            model_flag = true; // we are using model descriptions (possibly multiple models)
            // str2 = str1.mid(6, 15);
            // temp_model.model_id = str2.toUInt();
            QRegExp rx_get_model( "^MODEL\\s+(\\S+)" );
            if ( rx_get_model.indexIn( str1 ) != -1 )
            {
               temp_model.model_id = rx_get_model.cap( 1 );
            } else {
               temp_model.model_id = str1.mid( 6, 15 );
            }
            chain_flag = false; // we are starting a new molecule
            temp_model.molecule.clear( );
            temp_model.residue.clear( );
            clear_temp_chain(&temp_chain);
         }
         if (str1.left(6) == "ENDMDL") // we need to save the previously recorded molecule
         {
            last_was_ENDMDL = true;
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            // noticemsg += "Residue sequence from model " +
            // QString("%1").arg( model_vector.size() + 1 ) + ": \n";
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
            // noticemsg += str + "\n";
            
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
                     noticemsg += QString("Atom %1 conformation A selected\n").arg(str1.mid(6,5));
                  }
                  if (!chain_flag)    // at the first time we encounter the word ATOM
                  {             // we don't have a chain yet, so let's start a new one
                     temp_chain.chainID = str1.mid(21, 1);
                     str2 = str1.mid(72, 4);
                     temp_chain.segID = str2.trimmed();
                     chain_flag = true;
                  }
                  else // we have a chain, let's make sure the chain is still the same
                  {
                     bool break_chain = ( temp_chain.chainID != str1.mid(21, 1) );
                     QString thisResName = str1.mid(17,3).trimmed();
                     bool known_residue = ( multi_residue_map.count(thisResName) );
                     bool this_is_aa =  ( known_residue &&
                                         residue_list[multi_residue_map[thisResName][0]].type == 0 );
                     if ( !break_chain  && 
                          currently_aa_chain &&
                          known_residue &&
                          !this_is_aa )
                     {
                        break_chain = true;
                     } 
                     if ( break_chain )
                     {
                        if ( temp_chain.atom.size() ) 
                        {
                           temp_model.molecule.push_back(temp_chain);
                        }
                        clear_temp_chain(&temp_chain);
                        temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                        str2 = str1.mid(72, 4);
                        temp_chain.segID = str2.trimmed();
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
      model_vector.clear( );
      bead_model.clear( );
      errormsg = QString("Error reading file %1").arg(filename);
      return false;
   }
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      temp_model.molecule.push_back(temp_chain);
      noticemsg += "Residue sequence:";
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
      noticemsg += str;
      temp_model.model_id = "1";
      // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
      calc_vbar(&temp_model); // update the calculated vbar for this model
      model_vector.push_back(temp_model);
      clear_temp_chain(&temp_chain);
   }
   if ( !dna_rna_resolve() )
   {
      return false;
   }
   model_vector_as_loaded = model_vector;

   // /* IF THE 1st MODEL DEFINED ***************/
   // if (parameters_set_first_model)
   //   {
   //     model_vector.resize(1);
   //     model_vector_as_loaded.resize(1);
   //   }
   // /* IF THE 1st MODEL DEFINED ***************/ 

   // cout << list_chainIDs(model_vector);
   //cout << list_chainIDs(model_vector_as_loaded);
   if ( !calc_mw() )
   {
      return false;
   }
   return true;
}

bool US_Saxs_Util::dna_rna_resolve()
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
            QString thisres = this_atom->resName.trimmed();
            chainID = this_atom->chainID;
            if ( rx_dna_and_rna.indexIn(thisres) == -1 )
            {
               // not either:
               ask_convert = false;
               break;
            }
            if ( rx_dna.indexIn(thisres) != -1 )
            {
               // we definitely have DNA, correct this residue!
               ask_convert = false;
               convert_this = true;
               break;
            }
            if ( rx_rna.indexIn(thisres) != -1 )
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
            errormsg = 
               QString("Error: Chain %1 Molecule %2 only contains A, G & C residues (so far), can not determine DNA or RNA")
               .arg(chainID)
               .arg(i+1);
            return false;
         }
         if ( convert_this )
         {
            if ( !already_messaged.count(QString("%1|%2").arg(i).arg(chainID)) )
            {
               already_messaged[QString("%1|%2").arg(i).arg(chainID)] = true;

               noticemsg += QString("Converting Chain %1 Molecule %2 to standard DNA residue names\n")
                  .arg(chainID)
                  .arg(i+1);
            }
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
            {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               this_atom->resName = "D" + this_atom->resName.trimmed();
            }
         }
      }
   }
   return true;
}

bool US_Saxs_Util::assign_atom(const QString &str1, struct PDB_chain *temp_chain, QString *last_resSeq)
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
   temp_atom.orgName = str2;
   temp_atom.name = str2.trimmed();

   temp_atom.altLoc = str1.mid(16, 1);

   str2 = str1.mid(17, 3);
   temp_atom.orgResName = str2;
   temp_atom.resName = str2.trimmed();

   temp_atom.chainID = str1.mid(20, 2).trimmed();
   temp_atom.orgChainID = str1.mid(21, 1);

   temp_atom.resSeq = str1.mid(22, 5);
   temp_atom.orgResSeq = str1.mid(22, 4);
   temp_atom.resSeq.replace(QRegExp(" *"),"");
   if ( temp_atom.resSeq == *last_resSeq )
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

   temp_atom.p_residue = 0;
   temp_atom.p_atom    = 0;

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
      //   printError(us_tr("The residue " + temp_atom.resName + " listed in this PDB file is not found in the residue table!"));
   }

   if ( cystine_residues.count( temp_atom.resName ) ) {
      sulfur_pdb_chain_atom_idx[ temp_atom.chainID ][ temp_atom.resSeq ].push_back( (unsigned int)( temp_chain->atom.size() - 1 ) );
      if ( sulfur_atoms.count( temp_atom.name ) ) {
         sulfur_coordinates.push_back( temp_atom.coordinate );
         sulfur_pdb_line   .push_back( str1 );
      }
   }

   return(flag);
}

void US_Saxs_Util::clear_temp_chain( PDB_chain *temp_chain ) // clear all the memory from the vectors in temp_chain
{
   (*temp_chain).atom.clear( );
   (*temp_chain).chainID = "";
   (*temp_chain).segID = "";
}

float US_Saxs_Util::mw_to_volume( float mw , float vbar )
{
   return 
      mw * vbar * 1e24 / AVOGADRO;
      ;
}

bool US_Saxs_Util::calc_mw() 
{
   // cout << list_chainIDs(model_vector);

   unsigned int save_current_model = current_model;
   // QString error_string;

   last_pdb_load_calc_mw_msg.clear( );

   US_Saxs_Util usu;
   bool do_excl_vol = true;
   if ( !control_parameters.count( "saxsfile" ) ||
        !control_parameters.count( "hybridfile" ) ||
        !control_parameters.count( "atomfile" ) )
   {
      errormsg = "calc_mw(): requires saxsfile, hybridfile & atomfile defined";
      do_excl_vol = false;
      return false;
   }


   for (unsigned int i = 0; i < model_vector.size(); i++)
   {
      //       editor->append( QString( "\nModel: %1 vbar %2 cm^3/g\n" )
      //                       .arg( model_vector[i].model_id )
      //                       .arg( QString("").sprintf("%.3f", model_vector[i].vbar) ) );
                     
      current_model = i;

      model_vector[i].mw         = 0.0;
      double tot_excl_vol        = 0.0;
      double tot_scaled_excl_vol = 0.0;
      unsigned int total_e       = 0;
      // unsigned int total_e_noh   = 0;
      point cm;
      cm.axis[ 0 ] = 0.0;
      cm.axis[ 1 ] = 0.0;
      cm.axis[ 2 ] = 0.0;
      double total_cm_mw = 0e0;

      
      if ( !create_beads() )
      {
         return false;
      }

      // if( !error_string.length() ) 
      {
         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
         {
            double chain_excl_vol          = 0.0;
            double chain_scaled_excl_vol   = 0.0;
            model_vector[i].molecule[j].mw = 0.0;
            unsigned int chain_total_e     = 0;
            unsigned int chain_total_e_noh = 0;

            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
            {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if(this_atom->active) {
                  // printf("model %u chain %u atom %u mw %g\n",
                  //       i, j, k, this_atom->mw);
                  if ( this_atom->resName != "WAT" )
                  {
                     model_vector[i].mw += this_atom->mw;
                     cm.axis[ 0 ] += this_atom->mw * this_atom->coordinate.axis[ 0 ];
                     cm.axis[ 1 ] += this_atom->mw * this_atom->coordinate.axis[ 1 ];
                     cm.axis[ 2 ] += this_atom->mw * this_atom->coordinate.axis[ 2 ];
                     total_cm_mw += this_atom->mw;
                  }

                  model_vector[i].molecule[j].mw += this_atom->mw;
                  if ( do_excl_vol )
                  {
                     double excl_vol;
                     double scaled_excl_vol;
                     unsigned int this_e;
                     unsigned int this_e_noh;
                     if ( !set_excluded_volume( *this_atom, 
                                               excl_vol, 
                                               scaled_excl_vol, 
                                               our_saxs_options, 
                                               residue_atom_hybrid_map,
                                               this_e,
                                               this_e_noh ) )
                     {
                        return false;
                     } else {
                        chain_excl_vol        += excl_vol;
                        chain_scaled_excl_vol += scaled_excl_vol;
                        chain_total_e         += this_e;
                        chain_total_e_noh     += this_e_noh;

                        if ( this_atom->resName != "WAT" )
                        {
                           tot_excl_vol          += excl_vol;
                           tot_scaled_excl_vol   += scaled_excl_vol;
                           total_e               += this_e;
                        }
                     }
                  }
               }
            }
            // printf("model %u chain %u mw %g\n",
            //i, j, model_vector[i].molecule[j].mw);
            if (model_vector[i].molecule[j].mw != 0.0 )
            {
               QString qs = 
                  QString( "\nModel: %1 Chain: %2 Molecular weight %3 Daltons, Volume (from vbar) %4 A^3%5" )
                          .arg(model_vector[i].model_id)
                          .arg(model_vector[i].molecule[j].chainID)
                          .arg(model_vector[i].molecule[j].mw)
                          .arg( mw_to_volume( model_vector[i].molecule[j].mw, model_vector[i].vbar ) )
                          .arg( do_excl_vol ?
                                QString(", atomic volume %1 A^3%2 average electron density %3 A^-3")
                                .arg( chain_excl_vol )
                                .arg( chain_excl_vol != chain_scaled_excl_vol ?
                                      QString(", scaled atomic volume %1 A^2")
                                      .arg( chain_scaled_excl_vol )
                                      :
                                      ""
                                      )
                                .arg( chain_total_e / chain_excl_vol )
                                :
                                ""
                                )
                  ;
               //cout << qs << endl << flush;
               last_pdb_load_calc_mw_msg << qs.replace( "\n", "\nREMARK " ) + QString("\n");
            }
         }
         
         cm.axis[ 0 ] /= total_cm_mw;
         cm.axis[ 1 ] /= total_cm_mw;
         cm.axis[ 2 ] /= total_cm_mw;

         // now compute Rg
         double Rg2 = 0e0;
         
         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) 
         {
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
            {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if( this_atom->active ) 
               {
                  //       i, j, k, this_atom->mw);
                  if ( this_atom->resName != "WAT" )
                  {
                     Rg2 += this_atom->mw * 
                        ( 
                         (double) ( this_atom->coordinate.axis[ 0 ] - cm.axis[ 0 ] ) *
                         ( this_atom->coordinate.axis[ 0 ] - cm.axis[ 0 ] ) +
                         (double) ( this_atom->coordinate.axis[ 1 ] - cm.axis[ 1 ] ) *
                         ( this_atom->coordinate.axis[ 1 ] - cm.axis[ 1 ] ) +
                         (double) ( this_atom->coordinate.axis[ 2 ] - cm.axis[ 2 ] ) *
                         ( this_atom->coordinate.axis[ 2 ] - cm.axis[ 2 ] ) 
                         );
                  }
               }
            }
         }

         double Rg = sqrt( Rg2 / total_cm_mw );
         QString qs =  QString( "\nModel %1 Rg: %2 nm" )
            .arg( model_vector[ i ].model_id )
            .arg( Rg / 10.0, 0, 'f', 2 );

         // editor->append( qs );
         cout << qs << endl << flush;
         last_pdb_load_calc_mw_msg << qs;
      }

      if ( model_vector_as_loaded.size() > i )
      {
         model_vector_as_loaded[ i ].mw = model_vector[i].mw;
      }
      {
         QString qs = 
            QString( "\nModel: %1 Molecular weight %2 Daltons, Volume (from vbar) %3 A^3%4" )
            .arg(model_vector[i].model_id)
            .arg(model_vector[i].mw )
            .arg( mw_to_volume( model_vector[i].mw, model_vector[i].vbar ) )
            .arg( do_excl_vol ?
                  QString(", atomic volume %1 A^3%2 average electron density %3 A^-3")
                  .arg( tot_excl_vol )
                  .arg( tot_excl_vol != tot_scaled_excl_vol ?
                        QString(", scaled atomic volume %1 A^2")
                        .arg( tot_scaled_excl_vol )
                        :
                        ""
                        )
                  .arg( total_e / tot_excl_vol )
                  :
                  ""
                  );
         // editor->append( qs );
         cout << qs << endl << flush;
         last_pdb_load_calc_mw_msg << qs;
      }

      // put this back!
      //       if ( do_excl_vol && misc.set_target_on_load_pdb )
      //       {
      //          misc.target_e_density = total_e / tot_excl_vol;
      //          misc.target_volume = tot_excl_vol;
      //          cout << "Target excluded volume and electron density set\n";
      //       }

      // printf("model %u  mw %g\n",
      //       i, model_vector[i].mw);
   }

   current_model = save_current_model;
   return true;
}

bool US_Saxs_Util::load_mw_json( QString filename )
{
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "US_Saxs_Util::load_mw_json could not open file %1" ).arg( filename );
      return false;
   }
   QString qs;
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine();
   }
   f.close();
   atom_mw.clear( );
   map < QString, QString > parameters = US_Json::split( qs );
   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         it++ )
   {
      atom_mw[ it->first ] = it->second.toDouble();
      // us_qdebug( QString( "atom '%1' mw '%2'" ).arg( it->first ).arg( it->second.toDouble() ) );
   }
   return true;
}

bool US_Saxs_Util::load_vdw_json( QString filename )
{
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "US_Saxs_Util::load_vdw_json could not open file %1" ).arg( filename );
      return false;
   }
   QString qs;
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine();
   }
   f.close();
   atom_vdw.clear( );
   map < QString, QString > parameters = US_Json::split( qs );
   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         it++ )
   {
      atom_vdw[ it->first ] = it->second.toDouble();
   }
   return true;
}

#include "../include/us_vector.h"

bool US_Saxs_Util::load_vcm_json( QString filename )
{
   QFile f( filename );
   if ( !f.open( QIODevice::ReadOnly ) )
   {
      errormsg = QString( "US_Saxs_Util::load_vcm_json could not open file %1" ).arg( filename );
      return false;
   }
   QString qs;
   QTextStream ts( &f );
   while ( !ts.atEnd() )
   {
      qs += ts.readLine();
   }
   f.close();

   set < QString > to_spline;

   map < QString, QString > parameters = US_Json::split( qs );
   for ( map < QString, QString >::iterator it = parameters.begin();
         it != parameters.end();
         it++ )
   {
      // split it->second to double
      //      us_qdebug( QString( "vcm json first <%1>\n second <%2>\n" ).arg( it->first ).arg( it->second ) );
      QStringList qsl = (it->second ).split( "," , Qt::SkipEmptyParts );
      if ( vcm.count( it->first ) )
      {
         vcm[ it->first ].clear( );
      }
      for ( int i = 0; i < (int) qsl.size(); ++i )
      {
         vcm[ it->first ].push_back( qsl[ i ].toDouble() );
      }
      if ( it->first != "q" )
      {
         to_spline.insert( it->first );
      }

      // us_qdebug( US_Vector::qs_vector( "vcm[" + it->first + "]", vcm[ it->first ] ) );
   }

   // compute minimum point ?

   
   // 


   // build natural splines of each

   for ( set < QString >::iterator it = to_spline.begin();
         it != to_spline.end();
         it++ )
   {
      natural_spline( vcm[ "q" ],
                      vcm[ *it ],
                      vcm[ *it + ":y2" ]  );

      // us_qdebug( US_Vector::qs_vector( "vcm[" + *it + ":y2]", vcm[ *it + ":y2" ] ) );
   }

   return true;
}
