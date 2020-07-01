// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o
// us_hydrodyn_info.cpp contains code to report structures for debugging
// us_hydrodyn_util.cpp contains other various code, such as disulfide code
// (this) us_hydrodyn_load.cpp contains code to load files 

#include "../include/us_hydrodyn.h"

void US_Hydrodyn::read_residue_file()
{
   QString str1, str2;
   unsigned int numatoms, numbeads, /* i, */ j;
   // unsigned int positioner;
   QFile f(residue_filename);
   int error_count = 0;
   int line_count = 1;
   QString error_msg = us_tr("Residue file errors:\n");
   QString error_text = us_tr("Residue file errors:\n");
   if ( advanced_config.debug_1 )
   {
      QTextStream( stdout ) << "residue file name: " << residue_filename << endl;
   }
   residue_list.clear( );
   residue_list_no_pbr.clear( );
   multi_residue_map.clear( );
   residue_atom_hybrid_map.clear( );
   residue_atom_abb_hybrid_map.clear( );
   new_residues.clear( );
   map < QString, int > dup_residue_map;
   map < QString, bool > pbr_override_map; // maps positioner for overwrite
   unknown_residues.clear( ); // keep track of unknown residues

   msroll_radii.clear( );
   msroll_names.clear( );

   SS_init();
   
   vdwf.clear();

   res_vbar.clear();
   res_vbar2.clear();
   res_pKa.clear();
   res_mw.clear();
   res_ionization_mass_change.clear();

   QRegExp rx_spaces = QRegExp( "\\s+" ); 
   // i=1;
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         new_residue.comment = ts.readLine();
         line_count++;
         {
            QStringList qsl = ( ts.readLine() ).split( rx_spaces , QString::SkipEmptyParts );
            if ( qsl.size() != 7 && qsl.size() != 9 ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                    us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\nLine contains the wrong number of elements:\n" ) +
                                    qsl.join( " " ),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
               return;
            }
               
            new_residue.name   = qsl.front();            qsl.pop_front();
            new_residue.type   = qsl.front().toUInt();   qsl.pop_front();
            new_residue.molvol = qsl.front().toDouble(); qsl.pop_front();
            new_residue.asa    = qsl.front().toDouble(); qsl.pop_front();
            numatoms           = qsl.front().toUInt();   qsl.pop_front();
            numbeads           = qsl.front().toUInt();   qsl.pop_front();
            new_residue.vbar   = qsl.front().toDouble(); qsl.pop_front();
            res_vbar[ new_residue.name ] = new_residue.vbar;

            if ( qsl.size() ) {
               new_residue.vbar2 = qsl.front().toDouble();  qsl.pop_front();
               new_residue.pKa   = qsl.front().toDouble();  qsl.pop_front();
               res_vbar2[ new_residue.name ] = new_residue.vbar2;
               res_pKa  [ new_residue.name ] = new_residue.pKa;
            } else {
               new_residue.vbar2 = 0;
               new_residue.pKa   = 0;
            }
               
            // OLD WAY
            // ts >> new_residue.name;
            // ts >> new_residue.type;
            // ts >> new_residue.molvol;
            // ts >> new_residue.asa;
            // ts >> numatoms;
            // ts >> numbeads;
            // ts >> new_residue.vbar;
         }
         res_mw  [ new_residue.name ] = 0e0;
         // ts.readLine(); // read rest of line
         line_count++;
         new_residue.r_atom.clear( );
         new_residue.r_bead.clear( );
         vector < vector < atom > > new_atoms;
         new_atoms.resize(numbeads);
         vector < atom > alt_atoms;
         
         map < int, set < QString > > bead_assignment_to_vdwf;

         for ( j=0; j<numatoms; j++ ) {
            QString linein = ts.readLine();
            QStringList qsl = linein.split( rx_spaces , QString::SkipEmptyParts );
            if ( qsl.size() != 8 && qsl.size() != 9 ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                    us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\nLine contains the wrong number of elements:\n" ) +
                                    linein,
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
               return;
            }

            new_atom.name               = qsl.front();            qsl.pop_front();
            new_atom.hybrid.name        = qsl.front();            qsl.pop_front();
            new_atom.hybrid.mw          = qsl.front().toFloat();  qsl.pop_front();
            new_atom.hybrid.radius      = qsl.front().toFloat();  qsl.pop_front();
            new_atom.bead_assignment    = qsl.front().toUInt();   qsl.pop_front();
            
            // OLD WAY
            // ts >> new_atom.name;
            // ts >> new_atom.hybrid.name;
            // ts >> new_atom.hybrid.mw;
            // ts >> new_atom.hybrid.radius;
            // ts >> new_atom.bead_assignment;
            res_mw  [ new_residue.name ] += new_atom.hybrid.mw;
            
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
                  us_tr(QString(
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
            new_atom.positioner       = (bool) qsl.front().toInt();    qsl.pop_front();
            new_atom.serial_number    = qsl.front().toUInt();          qsl.pop_front();
            new_atom.hydration        = qsl.front().toFloat();         qsl.pop_front();

            // OLD WAY
            // ts >> positioner;
            // if(positioner == 0)
            // {
            //    new_atom.positioner = false;
            // }
            // else
            // {
            //    new_atom.positioner = true;
            // }
            // ts >> new_atom.serial_number;
            // ts >> new_atom.hydration;

            {
               _vdwf this_vdwf;
               this_vdwf.mw = new_atom.hybrid.mw;
               this_vdwf.r  = new_atom.hybrid.radius;
               this_vdwf.w  = new_atom.hydration;
               QString name = QString( "%1|%2" ).arg( new_residue.name ).arg( new_atom.name );
               if ( !vdwf.count( name ) ) {
                  vdwf[ name ] = this_vdwf;
                  bead_assignment_to_vdwf[ new_atom.bead_assignment ].insert( name );
               }
               // us_qdebug( QString( "vdwf[%1].mw %2 .r %3" ).arg( name ).arg( vdwf[ name ].mw ).arg( vdwf[ name ].r ) );
            }

            if ( qsl.size() ) {
               new_atom.ionization_mass_change = qsl.front().toFloat();  qsl.pop_front();
               if ( res_ionization_mass_change.count( new_residue.name ) ) {
                  QMessageBox::critical(this, us_tr( windowTitle() ),
                                        us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\nLine contains ionization information,\nbut there is atom in this residue already defining ionization information:\n" ) +
                                        linein,
                                        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                  return;
               }
               res_ionization_mass_change[ new_residue.name ] = new_atom.ionization_mass_change;
               new_residue.acid_residue = ( new_atom.ionization_mass_change < 0 );
               qDebug() << "found ionization mass change for " << new_residue.name << " value " << new_atom.ionization_mass_change;
            } else {
               new_atom.ionization_mass_change = 0;
            }

            // OLD WAY
            // str2 = ts.readLine(); // read rest of line
            line_count++;
            if (!new_atom.name.isEmpty() && new_atom.hybrid.radius > 0.0 && new_atom.hybrid.mw > 0.0)
            {
               residue_atom_hybrid_map[
                                       QString("%1|%2")
                                       .arg(new_residue.name).arg(new_atom.name)] 
                  = new_atom.hybrid.name;
               residue_atom_abb_hybrid_map[ new_atom.name ] = new_atom.hybrid.name;
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
               QMessageBox::warning(this, us_tr("UltraScan Warning"),
                                    us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
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
            if ( bead_assignment_to_vdwf.count( j ) ) {
               for ( set < QString >::iterator it = bead_assignment_to_vdwf[ j ].begin();
                     it != bead_assignment_to_vdwf[ j ].end();
                     ++it ) {
                  vdwf[ *it ].color = new_bead.color;
               }
            }
            if ( advanced_config.debug_1 )
            {
               printf("residue name %s loading bead %d placing method %d\n",
                      new_residue.name.toLatin1().data(),
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
                      ,new_residue.name.toLatin1().data()
                      ,new_residue.unique_name.toLatin1().data()
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
                it->first.toLatin1().data(), it->second[i]);
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
      US_Static::us_message(us_tr("ERRORS:"), error_msg);
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
         editor_msg( "red", QString( us_tr( "Error: can not create MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
      } else {
         QTextStream ts( &f_radii );
         for ( unsigned int i = 0; i < (unsigned int) msroll_radii.size(); i++ )
         {
            ts << msroll_radii[ i ];
         }
         f_radii.close();
         editor_msg( "blue", QString( us_tr( "Notice: created MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
      }

      QFile f_names( dir + "msroll_names.txt" );
      if ( !f_names.open( QIODevice::WriteOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: can not create MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
      } else {
         QTextStream ts( &f_names );
         for ( unsigned int i = 0; i < (unsigned int) msroll_names.size(); i++ )
         {
            ts << msroll_names[ i ];
         }
         f_names.close();
         editor_msg( "blue", QString( us_tr( "Notice: created MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
      }
   }
}

double US_Hydrodyn::basic_fraction( float pH, float pKa ) {
   if ( !pKa ) {
      return 1.0;
   }
   double ten2phmpka = pow( 10, pH - pKa );
   return ten2phmpka / ( 1.0 + ten2phmpka );
}

double US_Hydrodyn::ionized_residue_vbar( float bf, struct residue *res ) {
   if ( !res->vbar2 ) {
      return res->vbar;
   }
   return bf * res->vbar + ( 1.0 - bf ) * res->vbar2;
}

double US_Hydrodyn::ionized_atom_mw( float bf, struct atom *atom ) {
   // might need some sign logic here
   return atom->hybrid.mw + (1.0 - bf) * atom->ionization_mass_change;
}

double US_Hydrodyn::ionized_num_elect( float bf, struct atom *atom ) {
   // might need some sign logic here
   return atom->hybrid.num_elect + bf;
}

double US_Hydrodyn::ionized_hydrogens( float bf, struct atom *atom ) {
   // might need some sign logic here
   return atom->hybrid.hydrogens + (1.0 - bf);
}

// #define DEBUG_VBAR

void US_Hydrodyn::calc_vbar(struct PDB_model *model)
{
   float mw_vbar_sum = 0.0;
   float mw_sum = 0.0;
   float mw;
   float pH = le_pH->text().toFloat();
#if defined( DEBUG_VBAR )
   qDebug() << "calc_vbar(): pH = " << pH;
#endif
   
   bool use_pH = gparams.count( "use_pH" ) && gparams[ "use_pH" ] == "true";

   if ( use_pH ) {
      for (unsigned int i=0; i<(*model).residue.size(); i++) {
         if ( model->residue[ i ].name != "WAT" ) {
            mw = 0.0;
         
            float bf = basic_fraction( pH, model->residue[ i ].pKa );
            if ( !model->residue[ i ].pKa ) {
               bf = 1.0;
            } else {
               if ( !model->residue[ i ].acid_residue ) {
                  bf = 1.0 - bf;
               }
            }
#if defined( DEBUG_VBAR )
            qDebug() << "calc_vbar(): residue.name = " << model->residue[ i ].name;
            qDebug() << "calc_vbar(): pKa = " << model->residue[ i ].pKa;
            qDebug() << "calc_vbar(): acid_residue = " << ( model->residue[ i ].acid_residue ? "true" : "false" );
            qDebug() << "calc_vbar(): fraction = " << bf;
#endif
            for (unsigned int j=0; j<(*model).residue[i].r_atom.size(); j++) {
               // mw += (*model).residue[i].r_atom[j].hybrid.mw;
               mw += ionized_atom_mw( bf, &((*model).residue[i].r_atom[j] ) );
#if defined( DEBUG_VBAR )
               qDebug() << "calc_vbar(): ionized_atom_mw = " << ionized_atom_mw( bf, &((*model).residue[i].r_atom[j]) );
#endif
            }
            mw_sum += mw;
            // mw_vbar_sum += mw * (*model).residue[i].vbar;
            mw_vbar_sum += mw * ionized_residue_vbar( bf, &((*model).residue[i]) );
#if defined( DEBUG_VBAR )
            qDebug() << "calc_vbar(): mw = " << mw;
            qDebug() << "calc_vbar(): ionized_residue_vbar = " << ionized_residue_vbar( bf, &((*model).residue[i]) );
#endif
         }
      }
   } else {
      for (unsigned int i=0; i<(*model).residue.size(); i++) {
         if ( model->residue[ i ].name != "WAT" ) {
            mw = 0.0;
         
            for (unsigned int j=0; j<(*model).residue[i].r_atom.size(); j++) {
               mw += (*model).residue[i].r_atom[j].hybrid.mw;
            }
            mw_sum += mw;
            mw_vbar_sum += mw * (*model).residue[i].vbar;
         }
      }
   }
   double covolume = gparams.count( "covolume" ) ? gparams[ "covolume" ].toDouble() : 0e0;
   (*model).vbar = (double)floor(0.5 + ( ( ( mw_vbar_sum + covolume ) / mw_sum ) * 1000e0 ) ) / 1000e0; //  - 0.002125;
#if defined( DEBUG_VBAR )
   qDebug() << "calc VBAR: " << (*model).vbar << endl;
#endif
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
            // QTextStream( stdout ) << res->name << " bead " << i << " atom " << res->r_atom[j].name << " mw " << res->r_atom[j].hybrid.mw << endl;
         }
      }
      if ( advanced_config.debug_1 )
      {
         QTextStream( stdout ) << res->name << " bead " << i << " mw " << res->r_bead[i].mw << endl;
      }
   }
   if ( advanced_config.debug_1 )
   {
      QTextStream( stdout ) << res->name << " mw " << rmw << endl;
   }
}

void US_Hydrodyn::clear_temp_chain(struct PDB_chain *temp_chain) // clear all the memory from the vectors in temp_chain
{
   (*temp_chain).atom.clear( );
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
   temp_atom.name = str2.trimmed();

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
   temp_atom.resName = str2.trimmed();
   if ( temp_atom.resName == "SWH" )
   {
      temp_atom.resName = "WAT";
   }

   temp_atom.chainID = str1.mid(20, 2).trimmed();

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
   temp_atom.p_residue = 0;
   temp_atom.p_atom    = 0;
   (*temp_chain).atom.push_back(temp_atom);

   // replaced with map lookup below 
   // // bool found = false;
   // // for (unsigned int m = 0; m < residue_list.size(); m++)
   // // {
   // //    if (temp_atom.resName == residue_list[m].name)
   // //    {
   // //       // this can be wrong, as there can be multiple entries for the same residue name
   // //       // fixed up with reset_chain_residues()
   // //       current_residue = residue_list[m];
   // //       found = true;
   // //       break;
   // //    }
   // // }
   // // commented out since we check this later
   // // if (!found)
   // // {
   // //    //   printError(us_tr("The residue " + temp_atom.resName + " listed in this PDB file is not found in the residue table!"));
   // // }

   // this can be wrong, as there can be multiple entries for the same residue name,
   // we get the 1st residue reference
   // fixed up with reset_chain_residues()

   if ( multi_residue_map.count( temp_atom.resName ) && multi_residue_map[ temp_atom.resName ].size() ) {
      current_residue = residue_list[ multi_residue_map[ temp_atom.resName ][ 0 ] ];
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

int US_Hydrodyn::read_pdb( const QString &filename ) {
   lb_model->clear( );
   QString str, str1, str2, temp;
   model_vector.clear( );
   bead_model.clear( );
   QString last_resSeq = ""; // keeps track of residue sequence number, initialize to zero, first real one will be "1"
   struct PDB_chain temp_chain;
   QFile f(filename);
   struct PDB_model temp_model;
   bool chain_flag = false;
   bool model_flag = false;
   temp_model.molecule.clear( );
   temp_model.residue.clear( );
   clear_temp_chain(&temp_chain);
   bool currently_aa_chain = false; // do we have an amino acid chain (pbr)
   bool last_was_ENDMDL = false;    // to fix pdbs with missing MODEL tag

   if ( hydro.unit != -10 )
   {
      QFileInfo fi(bd_last_file);
      switch (
              QMessageBox::question(
                                    this,
                                    us_tr("Load PDB file"),
                                    QString(us_tr("The default PDB unit is Angstrom (1e-10).\n"
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

   set < QString > waters;
   waters.insert( "SOL" );
   waters.insert( "WAT" );

   set < QString > skip_waters;
   skip_waters.insert( "HOH" );
   skip_waters.insert( "DOD" );
   skip_waters.insert( "SOL" );

   QRegExp rx_water_multiplier( "^REMARK Multiply water Iq by (\\d+)", Qt::CaseInsensitive );
   if ( f.open( QIODevice::ReadOnly ) )
   {
      multiply_iq_by_atomic_volume_last_water_multiplier = 0;      
      last_pdb_header.clear( );
      last_pdb_title .clear( );
      last_pdb_filename = f.fileName();
      QTextStream ts(&f);

      int exp_resnum = 1;
      int exp_prev_resnum = -1;

      while ( !ts.atEnd() )
      {
         str1 = ts.readLine();
         
         if ( advanced_config.experimental_renum ) {
            if ( str1.left(5) == "MODEL" ||
                 str1.left(6) == "ENDMDL" ) {
               exp_resnum = 1;
               exp_prev_resnum = -1;
            }
               
            if ( str1.left(4) == "ATOM" ||
                 str1.left(6) == "HETATM" ) {
               int thisresnum = str1.mid( 22, 4 ).trimmed().toInt();

               if ( exp_prev_resnum == -1 ) {
                  exp_prev_resnum = thisresnum;
               }

               if ( thisresnum != exp_prev_resnum ) {
                  exp_resnum++;
                  exp_prev_resnum = thisresnum;
               }
               QString alt_str1 = str1.left( 22 ) + QString( "" ).sprintf( "%4d", exp_resnum ) + str1.right( str1.length() - 22 - 4 );
               // us_qdebug( QString( "<:%1\n>:%2" ).arg( str1 ).arg( alt_str1 ) );
               str1 = alt_str1;
            }
         }

         if ( saxs_options.multiply_iq_by_atomic_volume &&
              rx_water_multiplier.indexIn( str1 ) != -1 )
         {
            multiply_iq_by_atomic_volume_last_water_multiplier = rx_water_multiplier.cap( 1 ).toUInt();
            QTextStream( stdout ) << QString( "found water multiplier %1 in pdb\n" ).arg( multiply_iq_by_atomic_volume_last_water_multiplier );
         }

         if ( str1.left(3) == "TER" )
         {
            // push back previous chain if it exists
            if ( chain_flag )
            {
               if ( advanced_config.debug_2 )
               {
                  printf("ter break <%s>\n", str1.toLatin1().data());
               }
               if ( temp_chain.atom.size() ) {
                  sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
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
            QColor save_color = editor->textColor();
            editor->setTextColor("dark green");
            editor->append(QString("PDB %1: %2").arg(str1.left(6)).arg(tmp_str));
            editor->setTextColor(save_color);
            qApp->processEvents();
         }
         if (str1.left(5) == "MODEL" ||
             (str1.left(4) == "ATOM" && last_was_ENDMDL) ) // we have a new model in a multi-model file
         {
            // SS_apply( temp_model );
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
            if ( temp_chain.atom.size() ) {
               sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
            }
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            SS_apply( temp_model );
            editor->append("\nResidue sequence from " + project +".pdb model " +
                           QString("%1").arg( temp_model.model_id ) + ": \n");
            str = "";
            QString sstr = "";
            int sstr_pos = 0;
            
            map < QString, int > resname_counts_nonwat;
            map < QString, int > resname_theo_wat;
            map < QString, int > resname_counts_wat;
            int resname_counts_nonwat_total = 0;
            int resname_counts_wat_total    = 0;
      
            double mw_nonwat     = 0e0;
            double mw_wat        = 0e0;
            double tot_theo_wat  = 0e0;

            // the residue list is wrong if there are unknown residues
            for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
            {
               QString lastResSeq = "";
               for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
               {
                  PDB_atom *this_atom = &(temp_model.molecule[i].atom[j]);
                  QString resname = this_atom->resName;

                  QString res_idx =
                     QString("%1|%2")
                     .arg(this_atom->name != "OXT" ? resname : "OXT" )
                     .arg(this_atom->name);

                  if ( vdwf.count( res_idx ) ) {
                     if ( waters.count( resname ) ) {
                        mw_wat += vdwf[ res_idx ].mw;
                     } else {
                        mw_nonwat    += vdwf[ res_idx ].mw;
                        tot_theo_wat += vdwf[ res_idx ].w;
                        resname_theo_wat[ resname ] += vdwf[ res_idx ].w;
                     }
                  }

                  if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
                  {
                     if ( waters.count( resname ) ) {
                        resname_counts_wat[ resname ]++;
                        resname_counts_wat_total++;
                     } else {
                        resname_counts_nonwat[ resname ]++;
                        resname_counts_nonwat_total++;
                     }

                     str += resname + " ";
                     sstr += 
                        residue_short_names.count( resname ) ? 
                        QString(residue_short_names[ resname ] ) : "?"; 
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
               editor->append(us_tr("\nSequence in one letter code:\n"));
               editor->setCurrentFont(new_font);
               editor->append(sstr + "\n\n");

               editor_msg( "black", "Residue\t count\tpercent\t Theoretical waters\n" );

               if ( resname_counts_nonwat_total ) {
                  for ( map < QString, int >::iterator it = resname_counts_nonwat.begin();
                        it != resname_counts_nonwat.end();
                        ++it ) {
                     editor_msg( "black", QString( "%1\t%2\t%3%\t%4\n" )
                                 .arg( it->first )
                                 .arg( it->second )
                                 .arg( 100.0 * (double) it->second / (double) resname_counts_nonwat_total, 0, 'g', 2 )
                                 .arg( resname_theo_wat.count( it->first ) ? resname_theo_wat[ it->first ] : 0 )
                                 );
                  }
                  if ( resname_counts_nonwat_total && resname_counts_nonwat.size() > 1 ) {
                     editor_msg( "black", QString( "All\t%1\t%2%\t%3\n" )
                                 .arg( resname_counts_nonwat_total )
                                 .arg( 100 )
                                 .arg( tot_theo_wat )
                                 );
                  }

                  if ( resname_counts_wat_total ) {
                     editor_msg( "blue", "\nWater\t count\tAvg. Waters\n" );
                     for ( map < QString, int >::iterator it = resname_counts_wat.begin();
                           it != resname_counts_wat.end();
                           ++it ) {
                        editor_msg( "blue", QString( "%1\t%2\t%3\n" ).arg( it->first ).arg( it->second ).arg(  (double) it->second / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
                     }
                     if ( resname_counts_wat_total && resname_counts_wat.size() > 1 ) {
                        editor_msg( "blue", QString( "All\t%1\t%2\n" ).arg( resname_counts_wat_total ).arg(  (double) resname_counts_wat_total / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
                     }
                  }
                  if ( mw_nonwat && mw_wat ) {
                     editor_msg( "dark blue", QString( "\ngr WAT / gr nonWAT %1" ).arg( mw_wat / mw_nonwat, 0, 'g', 3 ) );
                  }
               }
               editor_msg( "black", "\n" );

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
            if(
               str1.mid(12,1) != "H" &&
               str1.mid(13,1) != "H" &&
               !str1.mid(12,5).trimmed().startsWith( "H" ) &&
               !skip_waters.count( str1.mid(17,3).trimmed() )
               )
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
                     temp_chain.segID = str2.trimmed();
                     chain_flag = true;
                  }
                  else // we have a chain, let's make sure the chain is still the same
                  {
                     bool break_chain = ( temp_chain.chainID != str1.mid(21, 1) );
                     QString thisResName = str1.mid(17,3).trimmed();
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
                           printf("chain break <%s>\n", str1.toLatin1().data());
                        }
                        if ( temp_chain.atom.size() ) 
                        {
                           if ( temp_chain.atom.size() ) {
                              sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
                           }
                           temp_model.molecule.push_back(temp_chain);
                        }
                        clear_temp_chain(&temp_chain);
                        temp_chain.chainID = str1.mid(21, 1); // we have to start a new chain
                        str2 = str1.mid(72, 4);
                        temp_chain.segID = str2.trimmed();
                     }
                     currently_aa_chain = (!known_residue || this_is_aa);
                  }
                  if ( assign_atom( str1, &temp_chain, &last_resSeq ) ) // parse the current line and add it to temp_chain
                  { // if true, we have new residue and need to add it to the residue vector
                     temp_model.residue.push_back(current_residue); // add the next residue of this model
                  }
                  temp_chain.atom.back().model_residue_pos = temp_model.residue.size() - 1;
               }
            }
         }
      }
      //      SS_apply( temp_model );
      f.close();
   } else {
      lb_model->clear( );
      model_vector.clear( );
      bead_model.clear( );
      QColor save_color = editor->textColor();
      editor->setTextColor("red");
      editor->append(QString("Error reading file %1").arg(filename));
      editor->setTextColor(save_color);
      return -1;
   }
   if(!model_flag)   // there were no model definitions, just a single molecule,
   {                  // we still need to save the results
      if ( temp_chain.atom.size() ) {
         if ( temp_chain.atom.size() ) {
            sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
         }
         temp_model.molecule.push_back(temp_chain);
      }
      SS_apply( temp_model );
      editor->append("\nResidue sequence from " + project +".pdb:\n");
      str = "";
      QString sstr = "";
      int sstr_pos = 0;
      // the residue list is wrong if there are unknown residues

      map < QString, int > resname_counts_nonwat;
      map < QString, int > resname_theo_wat;
      map < QString, int > resname_counts_wat;
      int resname_counts_nonwat_total = 0;
      int resname_counts_wat_total    = 0;
      
      double mw_nonwat     = 0e0;
      double mw_wat        = 0e0;
      double tot_theo_wat  = 0e0;

      // the residue list is wrong if there are unknown residues
      for ( unsigned int i = 0; i < temp_model.molecule.size(); i++ )
      {
         QString lastResSeq = "";
         for ( unsigned int j = 0; j < temp_model.molecule[i].atom.size(); j++ )
         {
            PDB_atom *this_atom = &(temp_model.molecule[i].atom[j]);
            QString resname = this_atom->resName;

            QString res_idx =
               QString("%1|%2")
               .arg(this_atom->name != "OXT" ? resname : "OXT" )
               .arg(this_atom->name);

            if ( vdwf.count( res_idx ) ) {
               if ( waters.count( resname ) ) {
                  mw_wat += vdwf[ res_idx ].mw;
               } else {
                  mw_nonwat    += vdwf[ res_idx ].mw;
                  tot_theo_wat += vdwf[ res_idx ].w;
                  resname_theo_wat[ resname ] += vdwf[ res_idx ].w;
               }
            }

            if ( temp_model.molecule[i].atom[j].resSeq != lastResSeq )
            {
               if ( waters.count( resname ) ) {
                  resname_counts_wat[ resname ]++;
                  resname_counts_wat_total++;
               } else {
                  resname_counts_nonwat[ resname ]++;
                  resname_counts_nonwat_total++;
               }

               str += resname + " ";
               sstr += 
                  residue_short_names.count( resname ) ? 
                  QString(residue_short_names[ resname ] ) : "?"; 
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
         editor->append(us_tr("\nSequence in one letter code:\n"));
         editor->setCurrentFont(new_font);
         editor->append(sstr + "\n\n");

         editor_msg( "black", "Residue\t count\tpercent\t Theoretical waters\n" );

         if ( resname_counts_nonwat_total ) {
            for ( map < QString, int >::iterator it = resname_counts_nonwat.begin();
                  it != resname_counts_nonwat.end();
                  ++it ) {
               editor_msg( "black", QString( "%1\t%2\t%3%\t%4\n" )
                           .arg( it->first )
                           .arg( it->second )
                           .arg( 100.0 * (double) it->second / (double) resname_counts_nonwat_total, 0, 'g', 2 )
                           .arg( resname_theo_wat.count( it->first ) ? resname_theo_wat[ it->first ] : 0 )
                           );
            }
            if ( resname_counts_nonwat_total && resname_counts_nonwat.size() > 1 ) {
               editor_msg( "black", QString( "All\t%1\t%2%\t%3\n" )
                           .arg( resname_counts_nonwat_total )
                           .arg( 100 )
                           .arg( tot_theo_wat )
                           );
            }

            if ( resname_counts_wat_total ) {
               editor_msg( "blue", "\nWater\t count\tAvg. Waters\n" );
               for ( map < QString, int >::iterator it = resname_counts_wat.begin();
                     it != resname_counts_wat.end();
                     ++it ) {
                  editor_msg( "blue", QString( "%1\t%2\t%3\n" ).arg( it->first ).arg( it->second ).arg(  (double) it->second / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
               }
               if ( resname_counts_wat_total && resname_counts_wat.size() > 1 ) {
                  editor_msg( "blue", QString( "All\t%1\t%2\n" ).arg( resname_counts_wat_total ).arg(  (double) resname_counts_wat_total / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
               }
            }
            if ( mw_nonwat && mw_wat ) {
               editor_msg( "dark blue", QString( "\ngr WAT / gr nonWAT %1" ).arg( mw_wat / mw_nonwat, 0, 'g', 3 ) );
           }
         }
         editor_msg( "black", "\n" );

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
      lb_model->addItem( "Model: " + model_vector[i].model_id );
   }
   lb_model->setEnabled(true);
   lb_model->item(0)->setSelected( true);
   current_model = 0;
   dna_rna_resolve();

   info_model_vector( QString( "source: model_vector" ), model_vector, { "CYS", "CYH" } );

   model_vector_as_loaded = model_vector;
   if ( advanced_config.debug_2 )
   {
      list_model_vector(&model_vector_as_loaded);
   }
   // QTextStream( stdout ) << list_chainIDs(model_vector);
   // QTextStream( stdout ) << list_chainIDs(model_vector_as_loaded);
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
            switch( QMessageBox::information( this, 
                                              us_tr("UltraScan"),
                                              us_tr(QString("Chain %1 Molecule %2 only contains A, G & C residues (so far)\n"
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
               editor->append(us_tr(QString("Converting Chain %1 Molecule %2 to standard DNA residue names\n")
                                 .arg(chainID)
                                 .arg(i+1)
                                 ));
            }
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) 
            {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               this_atom->resName = "D" + this_atom->resName.trimmed();
            }
         }
      }
   }
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
#if defined( DEBUG_TESTING_JML )
            us_qdebug( QString( "rcr idx %1 p_r %2 %3 %4" ).arg( idx ).arg( (long) this_atom->p_residue ).arg( this_atom->model_residue_pos ).arg( model->residue.size() ) );
#endif
            if ( this_atom->p_residue &&
                 this_atom->model_residue_pos != -1 &&
                 ( int ) model->residue.size() < this_atom->model_residue_pos )
            {
               if ( model->residue[ this_atom->model_residue_pos ].unique_name != this_atom->p_residue->unique_name )
               {
                  QTextStream( stdout ) << QString( "found residue difference %1 %2, fixing\n" )
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

double US_Hydrodyn::tc_vbar( double vbar ) {
   double TE = K0 + hydro.temperature;
   double partvol = (int)((
                           (
                            vbar +
                            (4.25e-4 * (TE - K20))
                            )
                           * 1000) + 0.5) / 1000.0;

   if (!misc.compute_vbar) {
      partvol = (int)((
                       (
                        misc.vbar -
                        (4.25e-4 * (K0 + misc.vbar_temperature - K20)) +
                        (4.25e-4 * (TE - K20))
                        )
                       * 1000) + 0.5) / 1000.0;
      }

   return partvol;
}


QString US_Hydrodyn::vbar_msg( double vbar ) {
   double partvol = tc_vbar( vbar );

   if (!misc.compute_vbar) {
      return
         QString(
                 us_tr(
                       "Calculated vbar: %1 [cm^3/g] @ %2 [°C]\n"
                       "Measured   vbar: %3 [cm^3/g] @ %4 [°C]\n"
                       "Used       vbar: %5 [cm^3/g] @ %6 [°C]\n"
                       )
                 )
         .arg( vbar, 5, 'f', 3, '0' )
         .arg( 20 )
         .arg( misc.vbar, 5, 'f', 3, '0' )
         .arg( misc.vbar_temperature )
         .arg( partvol, 5, 'f', 3, '0' )
         .arg( hydro.temperature )
         ;
   }
   
      
   return
      QString(
              us_tr(
                    "Calculated vbar: %1 [cm^3/g] @ %2 [°C]\n"
                    "Used       vbar: %3 [cm^3/g] @ %4 [°C]\n"
                    )
              )
      .arg( vbar, 5, 'f', 3, '0' )
      .arg( 20 )
      .arg( partvol, 5, 'f', 3, '0' )
      .arg( hydro.temperature )
      ;
}


double US_Hydrodyn::tc_solvent_visc() {
   // // y = exp( a + bx^(0.5) ln x )
   // static const double a = 0.586537098;
   // static const double b = -0.0436148;
   // return exp( a + b * sqrt( hydro.temperature) * log( hydro.temperature ) );

   // updated 2020-07-01
   // y = a + b * exp(-x/c)
   static const double a = 0.386918359;
   static const double b = 1.375011475;
   static const double mcinv = -1.0 / 24.86121945;

   if ( hydro.temperature == 20.0 ) {
      return hydro.solvent_viscosity;
   }
   return hydro.solvent_viscosity - default_hydro.solvent_viscosity + a + b * exp( hydro.temperature * mcinv );
}

double US_Hydrodyn::tc_solvent_dens() {
   // // y = exp( a + b x^2 )
   // static const double a = 0.000151523;
   // static const double b = -0.00000491;
   // return exp( a + b * hydro.temperature * hydro.temperature );

   // updated 2020-07-01
   // y = exp( a + b x^2 )
   static const double a = 0.000233816;
   static const double b = -0.0000050385;
   
   if ( hydro.temperature == 20.0 ) {
      return hydro.solvent_density;
   }
   return hydro.solvent_density - default_hydro.solvent_density + exp( a + b * hydro.temperature * hydro.temperature );
}

QString US_Hydrodyn::visc_dens_msg() {
   if ( hydro.manual_solvent_conditions ) {
      return
         QString(
                 us_tr(
                       "Manual flag set\n"
                       "Stored solvent viscosity %1 [cP]\n"
                       "Stored solvent density   %2 [cP] @ %3 [°C]\n"
                       )
                 )
         .arg( hydro.solvent_viscosity, 9, 'f', 7, '0' ) 
         .arg( hydro.solvent_density, 9, 'f', 7, '0' ) 
         .arg( hydro.temperature )
         ;
   } else {
      return
         QString(
                 us_tr(
                       "Stored solvent viscosity %1 [cP]\n"
                       "Used   solvent viscosity %2 [cP] @ %3 [°C]\n"
                       "Stored solvent density   %4 [cP]\n"
                       "Used   solvent density   %5 [cP] @ %6 [°C]\n"
                       )
                 )
         .arg( hydro.solvent_viscosity, 9, 'f', 7, '0' ) 
         .arg( tc_solvent_visc(), 9, 'f', 7, '0' )
         .arg( hydro.temperature )
         .arg( hydro.solvent_density, 9, 'f', 7, '0' ) 
         .arg( tc_solvent_dens(), 9, 'f', 7, '0' )
         .arg( hydro.temperature )
         ;
   }
}
