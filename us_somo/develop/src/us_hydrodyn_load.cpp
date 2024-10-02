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
#include "../include/us_vvv.h"
#include "../include/us_unicode.h"

#define TSO QTextStream(stdout)

#if defined(Q_OS_WIN)
// hmm. unicode difference for Windows between QWidgets and QTextStream?
# undef DEGREE_SYMBOL
# define DEGREE_SYMBOL QString::fromStdWString( L"\u00b0" )
#endif

void US_Hydrodyn::read_hybrid_file( QString filename ) {
   
   if ( filename.isEmpty() ) {
      filename = saxs_options.default_hybrid_filename;
   }

   QFile f(filename);
   hybrid_to_protons  .clear( );
   hybrid_to_electrons.clear( );

   QRegExp count_hydrogens("H(\\d)");
   QRegExp net_charge("((?:\\+|-)\\d*)$");

   if ( f.open(QIODevice::ReadOnly|QIODevice::Text ) ) {
      struct hybridization current_hybrid;
      QTextStream ts( &f );
      while (!ts.atEnd()) {
         ts >> current_hybrid.saxs_name;
         ts >> current_hybrid.name;
         ts >> current_hybrid.mw;
         ts >> current_hybrid.radius;
         ts >> current_hybrid.scat_len;
         ts >> current_hybrid.exch_prot;
         ts >> current_hybrid.num_elect;
         current_hybrid.hydrogens = 0;
         if ( count_hydrogens.indexIn( current_hybrid.name ) != -1 )
         {
            current_hybrid.hydrogens = count_hydrogens.cap(1).toUInt();
         }
         ts.readLine(); // read rest of line

         double protons = current_hybrid.num_elect;
         if ( net_charge.indexIn( current_hybrid.saxs_name ) != -1 ) {
            double delta = net_charge.cap( 1 ).toDouble();
            protons += delta;
            // QTextStream( stdout ) << "saxs_name " << current_hybrid.saxs_name << " delta " << delta << Qt::endl;
            // } else {
            // QTextStream( stdout ) << "saxs_name " << current_hybrid.saxs_name << " delta 0" << Qt::endl;
         }
         hybrid_to_electrons[ current_hybrid.name ] = current_hybrid.num_elect;
         hybrid_to_protons  [ current_hybrid.name ] = protons;
         // QTextStream( stdout ) << "hybrid.name " << current_hybrid.name << " protons " << protons << " electrons " << current_hybrid.num_elect << Qt::endl;
      }
      f.close();
   } else {
      QMessageBox::critical(this, us_tr( windowTitle() ),
                            QString( us_tr("Please note:\nThe somo.hybrid file '%1' could not be read.\n"
                                           "This is a critical issue" ) )
                            .arg( filename ),
                            QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
      exit(-1);
   }
   // add ABB defaults
   {
      hybridization abb_hybrid;
      abb_hybrid.saxs_name        = "ABB";
      abb_hybrid.name             = "ABB";
      abb_hybrid.mw               = misc.avg_mass;
      abb_hybrid.ionized_mw_delta = 0;
      abb_hybrid.radius           = misc.avg_radius;
      abb_hybrid.scat_len         = 0;
      abb_hybrid.num_elect        = misc.avg_num_elect;
      abb_hybrid.protons          = misc.avg_protons;
      hybrid_to_electrons[ abb_hybrid.name ] = abb_hybrid.num_elect;
      hybrid_to_protons  [ abb_hybrid.name ] = abb_hybrid.protons;
   }

}

void US_Hydrodyn::read_residue_file() {
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
      QTextStream( stdout ) << "residue file name: " << residue_filename << Qt::endl;
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
   res_mw.clear();

   QRegExp rx_spaces = QRegExp( "\\s+" ); 
   // i=1;
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
   {
      QTextStream ts(&f);
      while (!ts.atEnd())
      {
         new_residue.comment = ts.readLine();
         line_count++;
         // QTextStream( stdout ) << (residue_list.size() + 1 ) << " " << new_residue.comment << Qt::endl; 
         {
            QStringList qsl = ( ts.readLine() ).split( rx_spaces , Qt::SkipEmptyParts );
            if ( ( qsl.size() - 7 ) % 2 ) {
               // intermediate way
               // if ( qsl.size() != 7 && qsl.size() != 9 ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                    us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                          "Line contains the wrong number of elements:\n" ) +
                                    qsl.join( " " ),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
               return;
            }
               
            new_residue.name             = qsl.front();            qsl.pop_front();
            new_residue.type             = qsl.front().toUInt();   qsl.pop_front();
            new_residue.molvol           = qsl.front().toDouble(); qsl.pop_front();
            new_residue.asa              = qsl.front().toDouble(); qsl.pop_front();
            numatoms                     = qsl.front().toUInt();   qsl.pop_front();
            numbeads                     = qsl.front().toUInt();   qsl.pop_front();
            new_residue.vbar             = qsl.front().toDouble(); qsl.pop_front();
            res_vbar[ new_residue.name ] = new_residue.vbar;
            new_residue.pH               = -1;
            new_residue.vbar_at_pH       = -1;
            new_residue.ionized_mw_delta = 0;
            new_residue.mw               = 0;
            new_residue.vbars            .clear();
            new_residue.pKas             .clear();
            new_residue.r_atom_0         .clear();
            new_residue.r_atom_1         .clear();

            while ( qsl.size() ) {
               new_residue.vbars.push_back( qsl.front().toDouble() );  qsl.pop_front();
               new_residue.pKas .push_back( qsl.front().toDouble() );  qsl.pop_front();
            }

            if ( new_residue.pKas.size() > 2 ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                     us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                           "Line contains too many pKa/vbar paris, currently only 2 are supported:\n"
                                           "Please contact the developers to enable 3 pKas/vbars for a residue.\n"
                                           ) +
                                     QString( us_tr( "Residue name %1 on line %2" ) )
                                     .arg( new_residue.name )
                                     .arg( line_count )
                                     ,
                                     QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
               return;
            }

            // intermediate way
            
            // if ( qsl.size() ) {
            //    new_residue.vbar2 = qsl.front().toDouble();  qsl.pop_front();
            //    new_residue.pKa   = qsl.front().toDouble();  qsl.pop_front();
            // } else {
            //    new_residue.vbar2 = 0;
            //    new_residue.pKa   = 0;
            // }
               
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

         set < int > pKas_to_match;
         for ( int m = 0; m < (int) new_residue.vbars.size(); ++m ) {
            pKas_to_match.insert( m + 1 );
         }

         set < unsigned int > atom_serial_to_match;
         for ( unsigned int m = 0; m < numatoms; ++m ) {
            atom_serial_to_match.insert( m );
         }

         for ( j = 0; j < numatoms; ++j ) {
            QString linein = ts.readLine();
            QStringList qsl = linein.split( rx_spaces , Qt::SkipEmptyParts );
            if ( qsl.size() != 8 && qsl.size() != 16 ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                    us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                          "Line contains the wrong number of elements:\n" ) +
                                    linein,
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
               return;
            }

            new_atom.name                     = qsl.front();            qsl.pop_front();
            new_atom.hybrid.name              = qsl.front();            qsl.pop_front();
            new_atom.hybrid.mw                = qsl.front().toFloat();  qsl.pop_front();
            new_atom.hybrid.radius            = qsl.front().toFloat();  qsl.pop_front();
            new_atom.bead_assignment          = qsl.front().toUInt();   qsl.pop_front();
            new_atom.ionization_index         = 0;
            if ( !hybrid_to_protons  .count( new_atom.hybrid.name ) ||
                 !hybrid_to_electrons.count( new_atom.hybrid.name ) ) {
               editor_msg( "red", QString( us_tr( "Hybridization information missing for %1, net charge, proton count and isoelectric point will be incorrect!" ) ).arg( new_atom.hybrid.name ) );
            } else {
               new_atom.hybrid.protons           = hybrid_to_protons [ new_atom.hybrid.name ];
               new_atom.hybrid.num_elect         = hybrid_to_electrons[ new_atom.hybrid.name ];
            }
            
            new_atom.hybrid.ionized_mw_delta  = 0e0;
            // OLD WAY
            // ts >> new_atom.name;
            // ts >> new_atom.hybrid.name;
            // ts >> new_atom.hybrid.mw;
            // ts >> new_atom.hybrid.radius;
            // ts >> new_atom.bead_assignment;
            res_mw  [ new_residue.name ] += new_atom.hybrid.mw;
            new_residue.mw += new_atom.hybrid.mw;
            
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

            if ( new_atom.bead_assignment >= numbeads ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                     QString( us_tr( 
                                                    "\nThe atom's bead assignment has exceeded the number of beads.\n"
                                                    "For residue: %1 and Atom: %2 "
                                                    "on line %3 of the residue file.\n"
                                                     ) )
                                     .arg(new_residue.comment)
                                     .arg(new_atom.name)
                                     .arg(line_count)
                                     , QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton
                                     );
               return;
            }

            new_atom.positioner       = (bool) qsl.front().toInt();    qsl.pop_front();
            new_atom.serial_number    = qsl.front().toUInt();          qsl.pop_front();
            new_atom.hydration        = qsl.front().toFloat();         qsl.pop_front();

            if ( !atom_serial_to_match.count( new_atom.serial_number ) ) {
               QMessageBox::critical(this, us_tr( windowTitle() ),
                                     QString( us_tr( 
                                                    "\nThe atom's serial number is out of range or duplicated.\n"
                                                    "For residue: %1 and Atom: %2 "
                                                    "on line %3 of the residue file.\n"
                                                     ) )
                                     .arg(new_residue.comment)
                                     .arg(new_atom.name)
                                     .arg(line_count)
                                     , QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton
                                     );
               return;
            }
            atom_serial_to_match.erase( new_atom.serial_number );
            
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
               this_vdwf.mw               = new_atom.hybrid.mw;
               this_vdwf.ionized_mw_delta = 0;
               this_vdwf.r                = new_atom.hybrid.radius;
               this_vdwf.w                = new_atom.hydration;
               QString name = QString( "%1|%2" )
                  .arg( new_atom.name != "OXT" ? new_residue.name : "OXT" )
                  .arg( new_atom.name );
               if ( !vdwf.count( name ) ) {
                  vdwf[ name ] = this_vdwf;
                  bead_assignment_to_vdwf[ new_atom.bead_assignment ].insert( name );
               }
               // us_qdebug( QString( "vdwf[%1].mw %2 .r %3" ).arg( name ).arg( vdwf[ name ].mw ).arg( vdwf[ name ].r ) );
            }

            if ( qsl.size() ) {
               struct atom new_atom_1;

               int index                           = qsl.front().toInt();        qsl.pop_front();
               new_atom_1.name                     = new_atom.name;
               new_atom_1.hybrid.name              = qsl.front();                qsl.pop_front();
               new_atom_1.hybrid.mw                = qsl.front().toFloat();      qsl.pop_front();
               new_atom_1.hybrid.radius            = qsl.front().toFloat();      qsl.pop_front();
               new_atom_1.bead_assignment          = qsl.front().toUInt();       qsl.pop_front();
               new_atom_1.hybrid.ionized_mw_delta  = 0e0;
               new_atom_1.positioner               = (bool) qsl.front().toInt(); qsl.pop_front();
               new_atom_1.serial_number            = qsl.front().toUInt();       qsl.pop_front();
               new_atom_1.hydration                = qsl.front().toFloat();      qsl.pop_front();
               if ( !hybrid_to_protons.count( new_atom_1.hybrid.name ) ) {
                  editor_msg( "red", QString( us_tr( "Hybridization information missing for %1, net charge, proton count and isoelectric point will be incorrect!" ) ).arg( new_atom_1.hybrid.name ) );
               } else {
                  new_atom_1.hybrid.protons           = hybrid_to_protons[ new_atom_1.hybrid.name ];
               }
               
               if ( new_residue.r_atom_0.count( index ) ||
                    new_residue.r_atom_1.count( index ) ) {
                  QMessageBox::critical(this, us_tr( windowTitle() ),
                                        us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                              "Duplicate ionization index:\n" ) +
                                        linein,
                                        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                  return;
               }
               if ( !pKas_to_match.count( index ) ) {
                  QMessageBox::critical(this, us_tr( windowTitle() ),
                                        us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                              "pKa match index invalid:\n" ) +
                                        linein,
                                        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                  return;
               }
               if ( new_atom_1.bead_assignment != new_atom.bead_assignment ||
                    new_atom_1.positioner      != new_atom.positioner ||
                    new_atom_1.serial_number   != new_atom.serial_number ) {
                  QMessageBox::critical(this, us_tr( windowTitle() ),
                                        us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                              "mismatch in ionized bead assignment, positioner and/or serial number:\n" ) +
                                        linein,
                                        QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
                  return;
               }                  

               pKas_to_match.erase( index );
                  
               new_residue.r_atom_0[ index ] = new_atom;
               new_residue.r_atom_1[ index ] = new_atom_1;
               new_atom.ionization_index     = index;
            }

            // intermediate way
            // if ( qsl.size() ) {
            //    new_atom.ionization_mass_change = qsl.front().toFloat();  qsl.pop_front();
            //    // kept for now as reference
            //    // if ( res_ionization_mass_change.count( new_residue.name ) ) {
            //    //    QMessageBox::critical(this, us_tr( windowTitle() ),
            //    //                          us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\nLine contains ionization information,\nbut there is atom in this residue already defining ionization information:\n" ) +
            //    //                          linein,
            //    //                          QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            //    //    return;
            //    // }
            //    // res_ionization_mass_change[ new_residue.name ] = new_atom.ionization_mass_change;
            //    new_residue.acid_residue = ( new_atom.ionization_mass_change < 0 );
            //    qDebug() << "found ionization mass change for " << new_residue.name << " value " << new_atom.ionization_mass_change;
            // } else {
            //    new_atom.ionization_mass_change = 0;
            // }

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
                                    us_tr("Please note:\n\nThere was an error reading the selected Residue File!\n"
                                          "\nAtom "
                                       + new_atom.name + " cannot be read and will be deleted from List."),
                                    QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            }
         }

         if ( atom_serial_to_match.size() ) {
            QMessageBox::critical(this, us_tr( windowTitle() ),
                                  us_tr("Please note:\n\n"
                                        "There was an error reading the selected Residue File!\n"
                                        "The are missing atom entries for this residue\n"
                                        "Residue name : " + new_residue.name )
                                  , QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            return;
         }

         if ( pKas_to_match.size() ) {
            QMessageBox::critical(this, us_tr( windowTitle() ),
                                  us_tr("Please note:\n\n"
                                        "There was an error reading the selected Residue File!\n"
                                        "The are missing atom entries for the given number of pKa/vbars\n"
                                        "Residue name : " + new_residue.name )
                                  , QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            return;
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
   // qDebug() << "residues " << residue_list.size();
   // for ( int i = 0; i < (int) residue_list.size(); ++i ) {
   //    QTextStream( stdout ) << ( i + 1 ) << " " << residue_list[ i ].name << " " << residue_list[ i ].unique_name << " " << residue_list[ i ].comment << Qt::endl;
   // }
   compute_residues_with_atomic_vs_bead_hydration_differences();
   QTextStream(stdout) << list_residues_with_atomic_vs_bead_hydration_differences();
}

// #define DEBUG_VBAR
// #define DEBUG_VBAR2

double US_Hydrodyn::calc_vbar_updated( struct PDB_model & model ) {
   int chains   = (int) model.molecule.size();
   int residues = (int) model.residue .size();
   // spec residue info
   map < QString, QString > spec_residue = { { "N1", "N1" },
                                             { "N1-", "N1-" },
                                             { "OXT", "OXT" } };
   map < QString, double > spec_mw;
   map < QString, double > spec_psv;

   for ( auto it = spec_residue.begin();
         it != spec_residue.end();
         ++it ) {
      if ( multi_residue_map.count( it->first ) ) {
         for ( int j = 0; j < (int) multi_residue_map[ it->first ].size(); ++j ) {
            struct residue residue_entry = residue_list[ multi_residue_map[ it->first ][ j ] ];
            map < QString, struct atom * > res_atom_map = residue_atom_map( residue_entry );
            if ( res_atom_map.count( it->second ) ) {
               spec_mw[ it->first ] =
                     res_atom_map[ it->second ]->hybrid.mw +
                     res_atom_map[ it->second ]->hybrid.ionized_mw_delta
                     ;
               spec_psv[ it->first ] =
                     residue_entry.vbar_at_pH
                     ;
            } else {
               QTextStream( stdout )
                  << "WARNING: residue list atom lookup name missing: " << it->second << Qt::endl
                  ;
            }
         }
      } else {
         QTextStream( stdout )
            <<  "WARNING:  multi residue map is missing this residue: " << it->first << Qt::endl
            ;
      }
   }

   map < QString, QString > hybrid_name_to_N = { { "N3H0", "N1-" },
                                                 { "N3H1", "N1" } };

   map < QString, double > delta_mw;
   map < QString, double > delta_mv;

   {
      map < QString, int    > delta_counts;
      
      for ( int j = 0; j < chains; ++j ) {
         int atoms = (int) model.molecule[ j ].atom.size();
         if ( atoms ) {
            {
               map < QString, struct atom * > first_atom_map = first_residue_atom_map( model.molecule[ j ] );
               if ( first_atom_map.count( "N" ) ) {
                  if ( hybrid_name_to_N.count( first_atom_map[ "N" ]->hybrid.name ) ) {
                     QString use_name = hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ];
                     ++delta_counts[ use_name ];

                     double this_mw =
                        spec_mw[ use_name ] -
                        first_atom_map[ "N" ]->hybrid.mw - first_atom_map[ "N" ]->hybrid.ionized_mw_delta;
                     double this_mv =
                        this_mw * spec_psv[ use_name ];
                     delta_mw[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ] += this_mw;
                     delta_mv[ hybrid_name_to_N[ first_atom_map[ "N" ]->hybrid.name ] ] += this_mv;
                  } else {
                     QTextStream( stdout ) << " WARNING: first N in chain has unexpected hybridization " << first_atom_map[ "N" ]->hybrid.name  << Qt::endl;
                  }
               }
            }
            {
               map < QString, struct atom * > last_atom_map  = last_residue_atom_map( model.molecule[ j ] );
               QString use_name = "OXT";
               if ( last_atom_map.count( use_name ) ) {
                  ++delta_counts[ use_name ];

                  double this_mw = spec_mw[ use_name ];
                  double this_mv = this_mw * spec_psv[ use_name ];
                  delta_mw[ use_name ] += this_mw;
                  delta_mv[ use_name ] += this_mv;
               }
            }
         }
      }
   }
      
   double mw = 0;
   double mv = 0;

   for ( int j = 0; j < residues; ++j ) {
      // struct residue
      double this_mw = model.residue[ j ].mw + model.residue[ j ].ionized_mw_delta;
      double this_mv = model.residue[ j ].vbar_at_pH * this_mw;
#if defined( DEBUG_VBAR2 )
      QTextStream( stdout )
         << "--------------------------------------------------------------------------------\n"
         << "residue     : " << j << "\n"
         << ".mw         : " << this_mw << "\n"
         << ".vbar_at_pH : " << model.residue[ j ].vbar_at_pH << "\n"
         << ".mv         : " << this_mv << "\n"
         ;
#endif
      
      mw += this_mw;
      mv += this_mv;
   }
   double vbar         = 0;
   double molar_volume = 0;

   {
      double covolume = gparams.count( "covolume" ) ? gparams[ "covolume" ].toDouble() : 0e0;
      double total_mw = mw;
      double total_mv = mv;
      
#if defined( DEBUG_VBAR )
      QTextStream( stdout )
         << "--------------------------------------------------------------------------------\n"
         << "calc_vbar_updated() vbar calculation" << Qt::endl
         << "mw residues        : " << mw << Qt::endl
         << "mv residues        : " << mv << Qt::endl
         ;
#endif

      for ( auto it = delta_mw.begin();
            it != delta_mw.end();
            ++it ) {
#if defined( DEBUG_VBAR )
         QTextStream( stdout )
            <<  "mw " << it->first << "'s            : " << it->second << Qt::endl
            <<  "mv " << it->first << "'s            : " << delta_mv[ it->first ] << Qt::endl
            ;
#endif
         total_mw += it->second;
         total_mv += delta_mv[ it->first ];
      }
      
#if defined( DEBUG_VBAR )
      QTextStream( stdout )
         << "total mw            : " << total_mw << Qt::endl
         << "total mv            : " << total_mv << Qt::endl
         << "covolume            : " << covolume << Qt::endl
         << "total mv + covolume : " << ( total_mv + covolume ) << Qt::endl
         << "vbar                : " << (( total_mv + covolume ) / total_mw ) << Qt::endl
         ;
#endif
      molar_volume = total_mv + covolume;
      vbar = molar_volume / total_mw;
   }
   model.molar_volume = molar_volume;
   return vbar;
}

void US_Hydrodyn::calc_vbar( struct PDB_model *model, bool use_p_atom ) {
   float mw_vbar_sum = 0.0;
   float mw_sum = 0.0;
   float mw;
   model->molar_volume = 0;
   
   // in attic: info_model_p_residue( "::calc_vbar() start", *model, false );

   // redo for p_residue instead ? set < struct residue * > ?
   // do we have any duplicate residues at all?
#if defined( DEBUG_VBAR )
   QString lname = QString( "/tmp/vbar-%1-pH%2.csv" ).arg( QFileInfo( le_pdb_file->text() ).fileName() ).arg( le_pH->text() );
   qDebug() <<  "log active : '" << lname << "'"; 
   US_Log *usl = new US_Log( lname );
   QStringList qsl;
   qsl
      << "residue name"
      << "atom name" 
      << "used residue name" 
      << "vbar at pH" 
      << "mw"
      << "vbar_at_pH * mw "
      ;
   usl->log( qsl.join( "," ) );
   qsl.clear();
#endif

   if ( use_p_atom ) {
#if !defined(OLD_VBAR_WAY)
      model->vbar = calc_vbar_updated( *model );
      return;
#else
      int chains = (int) model->molecule.size();
      for ( int j = 0; j < chains; ++j ) {
         int atoms = (int) model->molecule[ j ].atom.size();
         for ( int k = 0; k < atoms; ++k ) {
            // might need special handling for N1
            double this_mw =
               model->molecule[ j ].atom[ k ].p_atom->hybrid.mw +
               model->molecule[ j ].atom[ k ].p_atom->hybrid.ionized_mw_delta;
            mw_sum      += this_mw;
            mw_vbar_sum += this_mw * model->molecule[ j ].atom[ k ].p_residue->vbar_at_pH;
# if defined( DEBUG_VBAR )
            qsl
               << model->molecule[ j ].atom[ k ].resName
               << model->molecule[ j ].atom[ k ].name
               << model->molecule[ j ].atom[ k ].p_residue->name
               << QString( "%1" ).arg( model->molecule[ j ].atom[ k ].p_residue->vbar_at_pH )
               << QString( "%1" ).arg( model->molecule[ j ].atom[ k ].p_atom->hybrid.mw +
                    model->molecule[ j ].atom[ k ].p_atom->hybrid.ionized_mw_delta )
               << QString( "%1" ).arg( model->molecule[ j ].atom[ k ].p_residue->vbar_at_pH * 
                    ( model->molecule[ j ].atom[ k ].p_atom->hybrid.mw +
                      model->molecule[ j ].atom[ k ].p_atom->hybrid.ionized_mw_delta ) )
               ;
               
            usl->log( qsl.join( "," ) );
            qsl.clear();
# endif
         }
      }
#endif
   } else {
      int residues = (int) model->residue.size();
      for ( int i = 0; i < residues; ++i ) {
         if ( model->residue[ i ].name != "WAT" ) {
            mw = 0.0;

            int atoms = (int) model->residue[i].r_atom.size();
            for ( int j = 0; j < atoms; ++j ) {
               mw += model->residue[i].r_atom[j].hybrid.mw + model->residue[i].r_atom[j].hybrid.ionized_mw_delta;
            }
            mw_sum      += mw;
            mw_vbar_sum += mw * model->residue[i].vbar_at_pH;
         }
      }
   }
   double covolume = gparams.count( "covolume" ) ? gparams[ "covolume" ].toDouble() : 0e0;
   model->vbar = (double)floor(0.5 + ( ( ( mw_vbar_sum + covolume ) / mw_sum ) * 1000e0 ) ) / 1000e0; //  - 0.002125;

#if defined( DEBUG_VBAR )
   qsl
      << "pH :"
      << le_pH->text()
      << "covolume :"
      << QString( "%1" ).arg( covolume )
      ;
   usl->log( qsl.join( "," ) );
   qsl.clear();
   delete usl;
#endif
}

bool US_Hydrodyn::residue_atomic_vs_bead_hydration_differences( const struct residue & r ) {
   // for each bead

   // TSO << "residue_atomic_vs_bead_hydration_differences() : name " << r.name << "\n";

   // is there any atomic hydration? if so, then bead hydration should match

   for ( auto & a : r.r_atom ) {
      if ( a.ionization_index
           || a.hydration
           || a.hydration2
           ) {
         return false;
      }
      
   }

   // no atomic hydration, any bead hydration?
   for ( auto & b : r.r_bead ) {
      if ( b.hydration ) {
         return true;
      }
   }

   return false;
}
   
void US_Hydrodyn::compute_residues_with_atomic_vs_bead_hydration_differences( const vector < struct residue > & rl ) {
   residues_with_atomic_vs_bead_hydration_differences.clear();
   
   const vector < struct residue > *use_rl = &rl;
   
   if ( !use_rl->size() ) {
      use_rl = &residue_list;
   }
   
   for ( auto &r : *use_rl ) {
      if ( residue_atomic_vs_bead_hydration_differences( r ) ) {
         // {
         //    struct residue r_tmp = r;
         //    info_residue( r_tmp, "no atomic hydration but has bead hydration" );
         // }
         residues_with_atomic_vs_bead_hydration_differences.insert( r.name );
      }
   }
}

QString US_Hydrodyn::list_residues_with_atomic_vs_bead_hydration_differences() {
   QStringList qsl;

   for ( auto &n : residues_with_atomic_vs_bead_hydration_differences ) {
      qsl << n;
   }
   return qsl.join( "\n" ) + "\n";
}

bool US_Hydrodyn::model_vector_has_hydration_differences( const vector < struct PDB_model > & m ) {
   for ( auto &model : m ) {
      for ( auto &chain : model.molecule ) {
         for ( auto &a : chain.atom ) {
            if ( residues_with_atomic_vs_bead_hydration_differences.count( a.resName ) ) {
               return true;
            }
         }
      }
   }
   return false;
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
            // QTextStream( stdout ) << res->name << " bead " << i << " atom " << res->r_atom[j].name << " mw " << res->r_atom[j].hybrid.mw << Qt::endl;
         }
      }
      if ( advanced_config.debug_1 )
      {
         QTextStream( stdout ) << res->name << " bead " << i << " mw " << res->r_bead[i].mw << Qt::endl;
      }
   }
   if ( advanced_config.debug_1 )
   {
      QTextStream( stdout ) << res->name << " mw " << rmw << Qt::endl;
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
   temp_atom.ionized_mw_delta = 0e0;
   temp_atom.si = 0e0;
   temp_atom.model_residue_pos = -1;
   temp_atom.model_residue_atom_pos = -1;
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
   // info_residue_vector( "read_pdb():: begin", residue_list );
   set_ionized_residue_vector( residue_list );
   // info_residue_vector( "read_pdb():: after set_ionized_residue_vector", residue_list );

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
   temp_model.hydration = 0;
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
   skip_waters.insert( "CIM" );

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
               temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            }
            if (temp_model.molecule.size() ) {
               SS_apply( temp_model, QString( "%1_model_%2").arg( project ).arg( temp_model.model_id ) );
               editor_msg( "black", "\nResidue sequence from " + project +".pdb model " +
                           QString("%1").arg( temp_model.model_id ) + ":");
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
                  QFont courier = QFont( "Courier", USglobal->config_list.fontSize - 1 );
                  editor_msg( "black", us_tr("\nSequence in one letter code:"));
                  editor_msg( "black", courier, sstr + "\n");
                  editor_msg( "black", courier, "Residue\t count\tpercent\t Theoretical waters\n" );
                  temp_model.hydration_gg = 0;

                  if ( resname_counts_nonwat_total ) {
                     for ( map < QString, int >::iterator it = resname_counts_nonwat.begin();
                           it != resname_counts_nonwat.end();
                           ++it ) {
                        editor_msg( "black", courier, QString( "%1\t %2\t%3%\t %4\n" )
                                    .arg( it->first )
                                    .arg( it->second )
                                    .arg( floor( 100 * 100.0 * (double) it->second / (double) resname_counts_nonwat_total ) / 100, 0, 'g', 3 )
                                    .arg( (double)(resname_theo_wat.count( it->first ) ? resname_theo_wat[ it->first ] : 0), 0, 'f', 0 )
                                    );
                     }
                     if ( resname_counts_nonwat_total && resname_counts_nonwat.size() > 1 ) {
                        editor_msg( "black", courier, QString( "All\t %1\t%2%\t %3\n" )
                                    .arg( resname_counts_nonwat_total )
                                    .arg( 100 )
                                    .arg( round( tot_theo_wat ) )
                                    );
                     }

                     if ( resname_counts_wat_total ) {
                        editor_msg( "blue", courier, "\nWater\t count\tAvg. Waters\n" );
                        for ( map < QString, int >::iterator it = resname_counts_wat.begin();
                              it != resname_counts_wat.end();
                              ++it ) {
                           editor_msg( "blue", courier, QString( "%1\t%2\t%3\n" ).arg( it->first ).arg( it->second ).arg(  (double) it->second / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
                        }
                        if ( resname_counts_wat_total && resname_counts_wat.size() > 1 ) {
                           editor_msg( "blue", courier, QString( "All\t%1\t%2\n" ).arg( resname_counts_wat_total ).arg(  (double) resname_counts_wat_total / (double) resname_counts_nonwat_total, 0, 'g', 2 ) );
                        }
                     }
                     if ( mw_nonwat ) {
                        if ( mw_wat ) {
                           temp_model.hydration_gg = mw_wat / mw_nonwat;
                           editor_msg( "dark blue", QString( "\nHydration [g/g] %1 (explicit waters)" ).arg( mw_wat / mw_nonwat, 0, 'g', 3 ) );
                        } else if ( tot_theo_wat ) {
                           temp_model.hydration_gg = tot_theo_wat * 18.01528 / mw_nonwat;
                           editor_msg( "dark blue", QString( "\nHydration [g/g] %1" ).arg( tot_theo_wat * 18.01528 / mw_nonwat, 0, 'g', 3 ) );
                        }
                     }
                     temp_model.hydration = tot_theo_wat;
                  }
                  editor_msg( "black", "\n" );
               }
               qApp->processEvents();
            
               // calc_vbar is wrong if there unknown residues, fixed later in check_for_missing_atoms()
               calc_vbar(&temp_model); // update the calculated vbar for this model
               model_vector.push_back(temp_model); // save the model in the model vector.
            } else {
               editor_msg( "dark red", QString( us_tr( "Warning: Empty model in PDB ignored\n" ) ) );
            }
            clear_temp_chain(&temp_chain); // we are done with this molecule and can delete it
         }
         if (str1.left(4) == "ATOM" || str1.left(6) == "HETATM") // need to add TER
         {
            if(
               str1.mid(12,1) != "H" &&
               str1.mid(13,1) != "H" &&
               !str1.mid(12,5).trimmed().startsWith( "H" ) &&
               !str1.mid(12,5).trimmed().contains( QRegularExpression( "^\\dH" ) ) &&
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
      SS_apply( temp_model, project );

      editor->append("\nResidue sequence from " + project +".pdb:\n");
      str = "";
      QString sstr = "";
      int sstr_pos = 0;
      // the residue list is wrong if there are unknown residues

      map < QString, int > resname_counts_nonwat;
      map < QString, double > resname_theo_wat;
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
      qDebug() << "tot_theo_wat " << tot_theo_wat;
      
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
         temp_model.hydration_gg = 0;

         if ( resname_counts_nonwat_total ) {
            for ( map < QString, int >::iterator it = resname_counts_nonwat.begin();
                  it != resname_counts_nonwat.end();
                  ++it ) {
               editor_msg( "black", QString( "%1\t %2\t%3%\t %4\n" )
                           .arg( it->first )
                           .arg( it->second )
                           .arg( floor( 100 * 100.0 * (double) it->second / (double) resname_counts_nonwat_total ) / 100, 0, 'g', 3 )
                           .arg( (double)(resname_theo_wat.count( it->first ) ? resname_theo_wat[ it->first ] : 0), 0, 'f', 0 )
                           );
            }
            if ( resname_counts_nonwat_total && resname_counts_nonwat.size() > 1 ) {
               editor_msg( "black", QString( "All\t %1\t%2%\t %3\n" )
                           .arg( resname_counts_nonwat_total )
                           .arg( 100 )
                           .arg( round( tot_theo_wat ) )
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
            if ( mw_nonwat ) {
               if ( mw_wat ) {
                  temp_model.hydration_gg = mw_wat / mw_nonwat;
                  editor_msg( "dark blue", QString( "\nHydration [g/g] %1 (explicit waters)" ).arg( mw_wat / mw_nonwat, 0, 'g', 3 ) );
               } else if ( tot_theo_wat ) {
                  temp_model.hydration_gg = tot_theo_wat * 18.01528 / mw_nonwat;
                  editor_msg( "dark blue", QString( "\nHydration [g/g] %1" ).arg( tot_theo_wat * 18.01528 / mw_nonwat, 0, 'g', 3 ) );
               }
            }
            temp_model.hydration = tot_theo_wat;
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

   // info_model_vector( QString( "source: model_vector" ), model_vector, { "GLU", "HIS", "CGU", "ALA" } );
   // info_model_vector_mw( QString( "read_pdb() : model_vector" ), model_vector, true );
   QTextStream(stdout) << "end of read_pdb()\n";
   info_residue_protons_electrons_at_pH( 7, model_vector[0] );

   model_vector_as_loaded = model_vector;
   if ( advanced_config.debug_2 )
   {
      list_model_vector(&model_vector_as_loaded);
   }
   // QTextStream( stdout ) << list_chainIDs(model_vector);
   // QTextStream( stdout ) << list_chainIDs(model_vector_as_loaded);
   if ( model_vector_has_hydration_differences( model_vector ) ) {
      editor_msg( "darkred", us_tr( "WARNING: PDB contains residues with bead hydration without atomic hydration,\nvdW models should not be used a they rely on atomic hydration\n\n" ) );
   }
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

double US_Hydrodyn::use_vbar( double vbar ) {
   return tc_vbar( vbar );
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


QString US_Hydrodyn::vbar_msg( double vbar, bool only_used ) {
   double partvol = tc_vbar( vbar );

   if ( only_used ) {
      return
         QString(
                 us_tr(
                       "Vbar used                        : %1 [cm^3/g] @ %2%3C\n"
                       )
                 )
         .arg( partvol, 5, 'f', 3, '0' )
         .arg( hydro.temperature )
         .arg( DEGREE_SYMBOL )
         ;
   }

   if (!misc.compute_vbar) {
      return
         QString(
                 us_tr(
                       "Vbar calculated                  : %1 [cm^3/g] @ %2%3C\n"
                       "Vbar measured                    : %4 [cm^3/g] @ %5%6C\n"
                       "Vbar used                        : %7 [cm^3/g] @ %8%9C\n"
                       )
                 )
         .arg( vbar, 5, 'f', 3, '0' )
         .arg( 20 )
         .arg( DEGREE_SYMBOL )
         .arg( misc.vbar, 5, 'f', 3, '0' )
         .arg( misc.vbar_temperature )
         .arg( DEGREE_SYMBOL )
         .arg( partvol, 5, 'f', 3, '0' )
         .arg( hydro.temperature )
         .arg( DEGREE_SYMBOL )
         ;
   }
   
      
   return
      QString(
              us_tr(
                    "Vbar calculated                  : %1 [cm^3/g] @ %2%3C\n"
                    "Vbar used                        : %4 [cm^3/g] @ %5%6C\n"
                    )
              )
      .arg( vbar, 5, 'f', 3, '0' )
      .arg( 20 )
      .arg( DEGREE_SYMBOL )
      .arg( partvol, 5, 'f', 3, '0' )
      .arg( hydro.temperature )
      .arg( DEGREE_SYMBOL )
      ;
}

double US_Hydrodyn::use_solvent_visc() {
   if ( hydro.manual_solvent_conditions ) {
      return hydro.solvent_viscosity;
   } else {
      return tc_solvent_visc();
   }
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

double US_Hydrodyn::use_solvent_dens() {
   if ( hydro.manual_solvent_conditions ) {
      return hydro.solvent_density;
   } else {
      return tc_solvent_dens();
   }
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

QString US_Hydrodyn::pH_msg() {
   return
      QString( "pH %1\n" ).arg( hydro.pH );
}

QString US_Hydrodyn::visc_dens_msg( bool only_used ) {

   if ( hydro.manual_solvent_conditions ) {
      return
         QString(
                 us_tr(
                       "Manual flag set\n"
                       "Stored solvent name      %1\n"
                       "Stored solvent viscosity %2 [cP]\n"
                       "Stored solvent density   %3 [g/ml] @ %4%5C\n"
                       )
                 )
         .arg( hydro.solvent_name )
         .arg( hydro.solvent_viscosity, 9, 'f', 7, '0' ) 
         .arg( hydro.solvent_density, 9, 'f', 7, '0' ) 
         .arg( hydro.temperature )
         .arg( DEGREE_SYMBOL )
         ;
   } else {
      if ( only_used ) {
         return
            QString(
                    us_tr(
                          "Stored solvent name      %1\n"
                          "Used   solvent viscosity %2 [cP] @ %3%4C\n"
                          "Used   solvent density   %5 [g/ml] @ %6[%7C]\n"
                          )
                    )
            .arg( hydro.solvent_name )
            .arg( tc_solvent_visc(), 9, 'f', 7, '0' )
            .arg( hydro.temperature )
            .arg( DEGREE_SYMBOL )
            .arg( tc_solvent_dens(), 9, 'f', 7, '0' )
            .arg( hydro.temperature )
            .arg( DEGREE_SYMBOL )
            ;
      }
      return
         QString(
                 us_tr(
                       "Stored solvent name      %1\n"
                       "Stored solvent viscosity %2 [cP]\n"
                       "Used   solvent viscosity %3 [cP] @ %4%5C\n"
                       "Stored solvent density   %6 [g/ml]\n"
                       "Used   solvent density   %7 [g/ml] @ %8%9C\n"
                       )
                 )
         .arg( hydro.solvent_name )
         .arg( hydro.solvent_viscosity, 9, 'f', 7, '0' ) 
         .arg( tc_solvent_visc(), 9, 'f', 7, '0' )
         .arg( hydro.temperature )
         .arg( DEGREE_SYMBOL )
         .arg( hydro.solvent_density, 9, 'f', 7, '0' ) 
         .arg( tc_solvent_dens(), 9, 'f', 7, '0' )
         .arg( hydro.temperature )
         .arg( DEGREE_SYMBOL )
         ;
   }
}

QString US_Hydrodyn::model_summary_msg( const QString & msg, struct PDB_model *model ) {

   // qDebug() << "model->hydration:" << model->hydration;
   // qDebug() << "misc.hydrovol:" << misc.hydrovol;

   QString qs;

   qs += "\n";

   if ( msg.length() ) {
      qs += msg + "\n";
   } else {
      qs +=  us_tr( "Model summary information:\n" );
   }

   if ( pdb_parse.find_sh ) {
      qs +=
         QString(
                 us_tr(
                       "Number of disulfide bonds        : %1\n"
                       "Number of free SH                : %2\n"
                       )
                 )
         .arg( model->num_SS_bonds )
         .arg( model->num_SH_free )
         ;
   }

   qs +=
      QString(
              us_tr(
                    "Calculation done at pH           : %1\n"
                    )
              )
      .arg( le_pH->text() )
      ;

   qs +=
      QString(
              us_tr(
                    "Molecular weight                 : %1 [Da]\n"
                    )
              )
            .arg( model->mw + model->ionized_mw_delta )
      ;

   if ( model->molar_volume > 0 ) {
      qs +=
         QString(
                 us_tr(
                       "Molar volume                     : %1 [cm^3/mol] @ %2%3C\n"
                       )
                 )
         .arg( model->molar_volume )
         .arg( 20 )
         .arg( DEGREE_SYMBOL )
         ;
   }

   qs += vbar_msg( model->vbar );

   qs += 
      QString(
              us_tr(
                    "SAXS excluded volume (anhydrous) : %1 [%2^3]\n"
                    )
              )
      .arg( model->volume )
      .arg( UNICODE_ANGSTROM )
      ;
   
   if ( hydro.temperature != 20 ) {
      qs +=
         QString(
                 us_tr(
                       "Anh. Molecular vol. (from vbar)  : %1 [" + UNICODE_ANGSTROM_QS + "^3] @ %2%3C\n"
                       )
                 )
         .arg( mw_to_volume( model->mw + model->ionized_mw_delta, model->vbar ) )
         .arg( 20 )
         .arg( DEGREE_SYMBOL )
         ;
   }

   qs +=
      QString(
              us_tr(
                    "Anh. Molecular vol. (from vbar)  : %1 [%12^3] @ %2%3C\n"
                    "Hyd. Molecular vol. (from vbar)  : %4 [%13^3] @ %5%6C\n"
                    "Radius of gyration               : %7 [%14]\n"
                    "Number of electrons              : %8\n"
                    "Number of protons                : %9\n"
                    "Net charge                       : %10\n"
                    "Isoelectric point                : %11\n"
                    )
              )
      .arg( mw_to_volume( model->mw + model->ionized_mw_delta, tc_vbar( model->vbar ) ) )
      .arg( hydro.temperature )
      .arg( DEGREE_SYMBOL )
      .arg( mw_to_volume( model->mw + model->ionized_mw_delta, tc_vbar( model->vbar ) ) + model->hydration * misc.hydrovol )
      .arg( hydro.temperature )
      .arg( DEGREE_SYMBOL )
      .arg( model->Rg, 0, 'f', 2 )
      .arg( model->num_elect )
      .arg( model->protons, 0, 'f', 1 )
      .arg( model->protons - model->num_elect, 0, 'f', 1 )
      .arg( model->isoelectric_point, 0, 'f', 2 )
      .arg( UNICODE_ANGSTROM )
      .arg( UNICODE_ANGSTROM )
      .arg( UNICODE_ANGSTROM )
      ;

   if ( model->volume ) {
      qs +=
         QString(
                 us_tr(
                       "Average electron density         : %1 [%2^-3]\n"
                       )
                 )
         .arg( model->num_elect / model->volume, 0, 'f', 3 )
         .arg( UNICODE_ANGSTROM )
      ;
   }

   // qDebug() << "model->hydration:" << model->hydration;


   return qs;
}

float US_Hydrodyn::mw_to_volume( float mw , float vbar )
{
   return 
      mw * vbar * 1e24 / AVOGADRO;
      ;
}

void US_Hydrodyn::calc_mw() 
{
   // cout << "calc_mw chains:\n";
   // cout << list_chainIDs(model_vector);
   // info_model_vector_mw( QString( "before calc_mw() : model_vector" ), model_vector, true );

   saxs_util->setup_saxs_options();

   // relevant when running hydronamic calcs
   // {
   //     QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
   //     editor_msg( "dark blue", courier, visc_dens_msg() );
   // }

   unsigned int save_current_model = current_model;
   QString error_string;

   last_pdb_load_calc_mw_msg.clear( );

   US_Saxs_Util usu;
   bool do_excl_vol = true;
   //    if ( !usu.setup_saxs_maps( 
   //                              saxs_options.default_atom_filename ,
   //                              saxs_options.default_hybrid_filename ,
   //                              saxs_options.default_saxs_filename 
   //                              ) )
   //    {
   //       editor_msg( "red", 
   //                   QString( us_tr("error: could not open %1, %2 or %3, no atomic excluded volume calc") )
   //                   .arg( saxs_options.default_atom_filename )
   //                   .arg( saxs_options.default_hybrid_filename )
   //                   .arg( saxs_options.default_saxs_filename ) );
   //       do_excl_vol = false;
   //    }

   bool do_vvv = asa.vvv;
   QFile       vvv_file;
   QTextStream vvv_ts;

   bool do_scol = ( gparams.count( "save_csv_on_load_pdb" ) &&
                    gparams[ "save_csv_on_load_pdb" ] == "true" );

   QFile       scol_file;
   QTextStream scol_ts;

   for (unsigned int i = 0; i < model_vector.size(); i++) {
      // editor->append( QString(us_tr("\nModel: %1 vbar %2 cm^3/g\n") )
      //                 .arg( model_vector[i].model_id )
      //                 .arg( QString("").sprintf("%.3f", model_vector[i].vbar) ) );
      // {
      //    QFont courier = QFont( "Courier", USglobal->config_list.fontSize );
      //    editor_msg( "dark blue", courier, QString( us_tr("\nModel: %1\n") ).arg( model_vector[i].model_id ) );
      //    editor_msg( "dark blue", courier, vbar_msg( model_vector[i].vbar ) );
      //    editor_msg( "dark blue", courier, visc_dens_msg() );
      // }

      current_model = i;

      double protons                    = 0.0;
      double electrons                  = 0.0;
      model_vector[i].mw                = 0.0;
      model_vector[i].ionized_mw_delta  = 0.0;
      model_vector[i].volume            = 0.0;
      double tot_excl_vol               = 0.0;
      double tot_scaled_excl_vol        = 0.0;
      unsigned int total_e              = 0;
      // unsigned int total_e_noh   = 0;
      point cm;
      cm.axis[ 0 ] = 0.0;
      cm.axis[ 1 ] = 0.0;
      cm.axis[ 2 ] = 0.0;
      double total_cm_mw = 0e0;

      // info_model_vector_mw( QString( "before create_beads calc_mw() : model_vector" ), model_vector, true );
      create_beads(&error_string, true);
      // info_model_vector_mw( QString( "after create_beads calc_mw() : model_vector" ), model_vector, true );
      // info_model_residues( "before calc_vbar in calc_mw()", model_vector[ i ] );
      // info_mw( QString( "after create_beads in calc_mw() : model_vector[ %1 ]" ).arg( i ), model_vector[ i ], true );
      calc_vbar( & model_vector[ i ], true );
      // info_model_residues( "after calc_vbar in calc_mw()", model_vector[ i ] );
      // info_mw( QString( "after calc_vbar in calc_mw() : model_vector[ %1 ]" ).arg( i ), model_vector[ i ], true );

      double model_mw               = 0e0;
      double model_ionized_mw_delta = 0e0;

      if( !error_string.length() ) {

         if ( do_vvv ) {
            vvv_file.setFileName( somo_dir + "/" + "tmp" + "/" + project + QString( "_%1.xyzr" ).arg( model_vector[i].model_id ) );
            if ( !vvv_file.open( QIODevice::WriteOnly ) ) {
               editor_msg( "red", QString( us_tr( "Error: VVV requested but can not open %1 for writing" ) ).arg( vvv_file.fileName() ) );
               do_vvv = false;
            } else {
               vvv_ts.setDevice( &vvv_file );
            }
         }

         if ( do_scol ) {
            scol_file.setFileName( somo_dir + "/" + "tmp" + "/" + project + QString( "_%1.csv" ).arg( model_vector[i].model_id ) );
            if ( !scol_file.open( QIODevice::WriteOnly ) ) {
               editor_msg( "red", QString( us_tr( "Error: save as CSV on load PDB requested but can not open %1 for writing" ) ).arg( scol_file.fileName() ) );
               do_scol = false;
            } else {
               scol_ts.setDevice( &scol_file );
               scol_ts << "\"Atom number\",\"Atom name\",\"Residue number\",\"Residue name\",\"Radius\",\"Mass [Da]\",\"x\",\"y\",\"z\"\n";
            }
         }

         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
            double chain_excl_vol          = 0.0;
            double chain_scaled_excl_vol   = 0.0;
            model_vector[i].molecule[j].mw = 0.0;
            unsigned int chain_total_e     = 0;
            unsigned int chain_total_e_noh = 0;
            double molecule_mw             = 0e0;

            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if( this_atom->active) {
                  
                  if ( do_vvv ) {
                     vvv_ts << QString( "%1\t%2\t%3\t%4\n" )
                        .arg( this_atom->coordinate.axis[ 0 ] )
                        .arg( this_atom->coordinate.axis[ 1 ] )
                        .arg( this_atom->coordinate.axis[ 2 ] )
                        .arg( this_atom->radius );
                  }

                  if ( do_scol ) {
                     scol_ts << QString( "%1,%2,%3,%4,%5,%6,%7,%8,%9\n" )
                        .arg( this_atom->serial )
                        .arg( this_atom->name )
                        .arg( this_atom->resSeq )
                        .arg( this_atom->resName )
                        .arg( this_atom->radius )
                        .arg( this_atom->mw + this_atom->ionized_mw_delta )
                        .arg( this_atom->coordinate.axis[ 0 ] )
                        .arg( this_atom->coordinate.axis[ 1 ] )
                        .arg( this_atom->coordinate.axis[ 2 ] )
                        ;
                  }

                  // printf("model %u chain %u atom %u mw %g\n",
                  //       i, j, k, this_atom->mw);
                  if ( this_atom->resName != "WAT" ) {
                     double atom_mw_w_delta = this_atom->mw + this_atom->ionized_mw_delta;

                     model_vector[i].mw               += this_atom->mw;
                     model_vector[i].ionized_mw_delta += this_atom->ionized_mw_delta;
                     molecule_mw                      += this_atom->mw;
                     model_mw                         += this_atom->mw;
                     model_ionized_mw_delta           += this_atom->ionized_mw_delta;

                     cm.axis[ 0 ] += atom_mw_w_delta * this_atom->coordinate.axis[ 0 ];
                     cm.axis[ 1 ] += atom_mw_w_delta * this_atom->coordinate.axis[ 1 ];
                     cm.axis[ 2 ] += atom_mw_w_delta * this_atom->coordinate.axis[ 2 ];
                     total_cm_mw                                  += atom_mw_w_delta;
                     model_vector[i].molecule[j].mw               += this_atom->mw;
                     model_vector[i].molecule[j].ionized_mw_delta += this_atom->ionized_mw_delta;
#if defined( DEBUG_ABB_ELECT_PROT )
                     qDebug() <<
                        QString(
                                "resName %1\nprotons %2\nelectrons %3\n"
                                )
                        .arg( this_atom->resName )
                        .arg( this_atom->p_atom->hybrid.protons )
                        .arg( this_atom->p_atom->hybrid.num_elect )
                        ;
#endif
                     protons   += this_atom->p_atom->hybrid.protons;
                     electrons += this_atom->p_atom->hybrid.num_elect;
                  }

                  if ( do_excl_vol ) {
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
                                                           si) ) {
                        editor_msg( "dark red", saxs_util->errormsg );
                     } else {
                        chain_excl_vol        += excl_vol;
                        chain_scaled_excl_vol += scaled_excl_vol;
                        chain_total_e         += this_e;
                        chain_total_e_noh     += this_e_noh;
                        this_atom->si          = si;
                        model_vector_as_loaded[ i ].molecule[ j ].atom[ k ].si = si;
                        if ( this_atom->resName != "WAT" )
                        {
                           tot_excl_vol          += excl_vol;
                           tot_scaled_excl_vol   += scaled_excl_vol;
                           total_e               += this_e;
                        }
                        model_vector[i].volume += excl_vol;
                     }
                  }
               }
            }
            
            model_vector[i].molecule[j].mw = molecule_mw;

            // printf("model %u chain %u mw %g\n",
            //i, j, model_vector[i].molecule[j].mw);
            // if (model_vector[i].molecule[j].mw != 0.0 ) {
            //    QString qs = 
            //       QString(us_tr("\nModel: %1 Chain: %2 Molecular weight %3 Daltons, Volume (from vbar) %4 A^3%5")
            //               .arg(model_vector[i].model_id)
            //               .arg(model_vector[i].molecule[j].chainID)
            //               .arg(model_vector[i].molecule[j].mw)
            //               .arg( mw_to_volume( model_vector[i].molecule[j].mw, model_vector[i].vbar ) )
            //               .arg( do_excl_vol ?
            //                     QString(", atomic volume %1 A^3%2 average electron density %3 A^-3")
            //                     .arg( chain_excl_vol )
            //                     .arg( chain_excl_vol != chain_scaled_excl_vol ?
            //                           QString(", scaled atomic volume %1 A^2")
            //                           .arg( chain_scaled_excl_vol )
            //                           :
            //                           ""
            //                           )
            //                     .arg( chain_total_e / chain_excl_vol )
            //                     :
            //                     ""
            //                     )
            //               );
            //    // qs += "\n" + vbar_msg( model_vector[i].vbar );
            //    editor->append( qs );
            //    last_pdb_load_calc_mw_msg << qs.replace( "\n", "\nREMARK " ) + QString("\n");
            // }
         }
         
         cm.axis[ 0 ] /= total_cm_mw;
         cm.axis[ 1 ] /= total_cm_mw;
         cm.axis[ 2 ] /= total_cm_mw;

         // now compute Rg
         double Rg2 = 0e0;
         
         for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
            for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
               PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
               if( this_atom->active )  {
                  //       i, j, k, this_atom->mw);
                  if ( this_atom->resName != "WAT" ) {
                     Rg2 += ( this_atom->mw + this_atom->ionized_mw_delta ) * 
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
         // QString qs =  QString( "\nModel %1 Rg: %2 nm" )
         //    .arg( model_vector[ i ].model_id )
         //    .arg( Rg / 10.0, 0, 'f', 2 );

         model_vector[i].Rg      = Rg;
         model_vector[i].protons = protons;

         // editor->append( qs );
         // last_pdb_load_calc_mw_msg << qs;
         if ( do_vvv ) {
            vvv_file.close();
            double volume;
            double surf;
            
            vvv::compute_vol_surf( vvv_file.fileName().toLatin1().data(),
                                   asa.vvv_probe_radius,
                                   asa.vvv_grid_dR,
                                   volume,
                                   surf );
            editor_msg( "black",
                        QString( "VVV: probe %1 (A) grid side %2 (A) volume %3 (A^3) surface area %4 (A^2)" )
                        .arg( asa.vvv_probe_radius )
                        .arg( asa.vvv_grid_dR )
                        .arg( volume )
                        .arg( surf )
                        );
         }
         if ( do_scol ) {
            scol_file.close();
            editor_msg( "blue",
                        QString( "Created CSV atomic file: %1" ).arg( scol_file.fileName() )
                        );
         }
      }

      model_vector[ i ].mw                = model_mw;
      model_vector[ i ].ionized_mw_delta  = model_ionized_mw_delta;
      model_vector[ i ].num_elect         = electrons; // electrons; // total_e;
      model_vector[ i ].isoelectric_point = compute_isoelectric_point( model_vector[ i ] );

      {
         dammix_remember_mw_source[ QString( "%1 Model %2" ).arg( project ).arg( i + 1 ) ] = "computed from pdb";
         dammix_remember_mw[ QString( "%1 Model %2" ).arg( project ).arg( i + 1 ) ] = model_vector[i].mw + model_vector[ i ].ionized_mw_delta;
         // QTextStream(stdout) << QString("dammix remember mw set to %1\n").arg( model_vector[i].mw + model_vector[ i ].ionized_mw_delta );
         // QString qs = 
         //    QString( us_tr( "\nModel: %1 Molecular weight %2 Daltons, Volume (from vbar) %3 A^3%4" ) )
         //    .arg(model_vector[i].model_id)
         //    .arg(model_vector[i].mw )
         //    .arg( mw_to_volume( model_vector[i].mw, model_vector[i].vbar ) )
         //    .arg( do_excl_vol ?
         //          QString(", atomic volume %1 A^3%2 average electron density %3 A^-3")
         //          .arg( tot_excl_vol )
         //          .arg( tot_excl_vol != tot_scaled_excl_vol ?
         //                QString(", scaled atomic volume %1 A^2")
         //                .arg( tot_scaled_excl_vol )
         //                :
         //                ""
         //                )
         //          .arg( total_e / tot_excl_vol )
         //          :
         //          ""
         //          );
#if defined(U_EXPT)
         { 
            // for Javier's attempt at Rg
            // double Rg = model_vector[i].Rg;
            // qs += QString( "\n%1 model %2 %3 kD, Rg %4 A,  (Rg/6.5)^3: %5 %6 %" )
            //    .arg( project )
            //    .arg( model_vector[i].model_id )
            //    .arg( model_vector[i].mw / 1000e0, 0, 'f', 2 )
            //    .arg( Rg, 0, 'f', 2 )
            //    .arg( pow( Rg / 6.5e0, 3e0 ), 0, 'f', 2 )
            //    .arg( 100.0 * ( ( model_vector[i].mw / 1000e0 ) - pow( Rg / 6.5e0, 3e0 ) ) / ( model_vector[i].mw / 1000e0 ), 0, 'f', 1 )
            //    ;
         }
#endif
         // // qs += "\n" + vbar_msg( model_vector[i].vbar );
         // editor->append( qs );

         update_model_chain_ionization( model_vector[ i ], true );
         editor_msg( "black", QFont( "Courier", USglobal->config_list.fontSize ),
                     model_summary_msg( QString( "Summary information for model %1:" ).arg( i + 1 ), &model_vector[i] ) );
         last_pdb_load_calc_mw_msg << model_summary_msg( QString( "Summary information for model %1:" ).arg( i + 1 ), &model_vector[i] );
      }

      if ( do_excl_vol && misc.set_target_on_load_pdb ) {
         misc.target_e_density = total_e / tot_excl_vol;
         misc.target_volume = tot_excl_vol;
         editor_msg("blue", us_tr("Target excluded volume and electron density set"));
      }

      // printf("model %u  mw %g\n",
      //        i, model_vector[i].mw);

      // {
      //    double tmp_mw = 0e0;
      //    for ( int j = 0; j < (int) model_vector[ i ].molecule.size(); ++j )
      //    {
      //       tmp_mw += model_vector[ i ].molecule[ j ].mw;
      //       us_qdebug( QString( "model %1 molecule %2 mw %3" ).arg( i ).arg( j ).arg( model_vector[ i ].molecule[ j ].mw ) );
      //    }
      //    us_qdebug( QString( "" ).sprintf( 
      //                                  "model %d total from molecules mw %.2f as model mw %.2f cm mw %.2f model_mw  %.2f", 
      //                                  i,
      //                                  tmp_mw, 
      //                                  model_vector[ i ].mw,
      //                                  total_cm_mw,
      //                                  model_mw
      //                                   ) );
      // }

      update_model_chain_ionization( model_vector[ i ], true );

      if ( model_vector_as_loaded.size() > i ) {
         model_vector_as_loaded[ i ].mw               = model_vector[i].mw;
         model_vector_as_loaded[ i ].ionized_mw_delta = model_vector[i].ionized_mw_delta;
         model_vector_as_loaded[ i ].volume           = model_vector[i].volume;
         model_vector_as_loaded[ i ].num_elect        = model_vector[i].num_elect;
         model_vector_as_loaded[ i ].num_SS_bonds     = model_vector[i].num_SS_bonds;
         model_vector_as_loaded[ i ].num_SH_free      = model_vector[i].num_SH_free;
         model_vector_as_loaded[ i ].vbar             = model_vector[i].vbar;
      }
      // info_model_residues( "after update_model_chain_ionization in calc_mw()", model_vector[ i ] );
   }
   editor_msg( "black", "" );
   current_model = save_current_model;
   // info_model_vector_mw( QString( "after calc_mw() : model_vector" ), model_vector, true );
   // info_model_vector( QString( "after calc_mw() : model_vector" ), model_vector );
   // info_mw( QString( "after calc_mw() : model_vector" ), model_vector, true );
   // info_residue_protons_electrons_at_pH( le_pH->text().toDouble(),  model_vector[ 0 ] );
   QTextStream(stdout) << "end of calc_mw()\n";
   info_residue_protons_electrons_at_pH( 7, model_vector[0] );
}

void US_Hydrodyn::update_model_chain_ionization( struct PDB_model & model, bool quiet ) {
   if ( !quiet ) {
      QTextStream( stdout )
         << "========================================================================================" << Qt::endl
         << "US_Hydrodyn::update_model_chain_ionization()" << Qt::endl
         << "========================================================================================" << Qt::endl
         ;
   }

   int chains   = (int) model.molecule.size();
   int residues = (int) model.residue.size();

   model.ionized_mw_delta = 0e0;
   // model.hydration        = 0e0;
   
   for ( int j = 0; j < chains; ++j ) {
      int atoms = (int) model.molecule[ j ].atom.size();
      model.molecule[ j ].ionized_mw_delta = 0e0;
      for ( int k = 0; k < atoms; ++k ) {
         
         // add to chain & model
         model.ionized_mw_delta += model.molecule[ j ].atom[ k ].ionized_mw_delta ;
         // model.hydration        += 
         //    model.residue[ model.molecule[ j ].atom[ k ].model_residue_pos ].r_atom[ model.molecule[ j ].atom[ k ].atom_assignment ].hydration;
         model.molecule[ j ].ionized_mw_delta += model.molecule[ j ].atom[ k ].ionized_mw_delta ;

         if ( quiet ) {
            continue;
         }
         
         if ( !model.molecule[ j ].atom[ k ].p_residue ) {
            QTextStream( stdout ) << "******** p_residue not set" << Qt::endl;
            continue;
         }
         
         QTextStream( stdout )
            << "----------------------------------------------------------------------------------------" << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].name              = " << model.molecule[ j ].atom[ k ].name              << Qt::endl
            // not useful << " model.molecule[ " << j << " ].atom[ " << k << " ].bead_number      = " << model.molecule[ j ].atom[ k ].bead_number      << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].model_residue_pos = " << model.molecule[ j ].atom[ k ].model_residue_pos << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].bead_assignment   = " << model.molecule[ j ].atom[ k ].bead_assignment   << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].atom_assignment   = " << model.molecule[ j ].atom[ k ].atom_assignment   << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].mw                = " << model.molecule[ j ].atom[ k ].mw                << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].ionized_mw_delta  = " << model.molecule[ j ].atom[ k ].ionized_mw_delta  << Qt::endl
            ;


         if ( (int) model.molecule[ j ].atom[ k ].p_residue->r_atom.size() <= model.molecule[ j ].atom[ k ].atom_assignment ) {
            QTextStream( stdout ) << "******** p_residue->r_atom too small for atom_assignment" << Qt::endl;
            continue;
         }
            
         QTextStream( stdout )
            << " model.molecule[ " << j << " ].atom[ " << k << " ].p_residue->r_atom[ " << model.molecule[ j ].atom[ k ].atom_assignment
            << " ].hybrid.mw               = " << model.molecule[ j ].atom[ k ].p_residue->r_atom[ model.molecule[ j ].atom[ k ].atom_assignment ].hybrid.mw
            << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].p_residue->r_atom[ " << model.molecule[ j ].atom[ k ].atom_assignment
            << " ].hybrid.ionized_mw_delta = " << model.molecule[ j ].atom[ k ].p_residue->r_atom[ model.molecule[ j ].atom[ k ].atom_assignment ].hybrid.ionized_mw_delta
            << Qt::endl
            ;
            
         if ( (int) model.molecule[ j ].atom[ k ].p_residue->r_bead.size() <= model.molecule[ j ].atom[ k ].bead_assignment ) {
            QTextStream( stdout ) << "******** p_residue->r_bead too small for bead_assignment" << Qt::endl;
            continue;
         }

         QTextStream( stdout )
            << " model.molecule[ " << j << " ].atom[ " << k << " ].p_residue->r_bead[ " << model.molecule[ j ].atom[ k ].bead_assignment
            << " ].mw                      = " << model.molecule[ j ].atom[ k ].p_residue->r_bead[ model.molecule[ j ].atom[ k ].bead_assignment ].mw
            << Qt::endl
            << " model.molecule[ " << j << " ].atom[ " << k << " ].p_residue->r_bead[ " << model.molecule[ j ].atom[ k ].bead_assignment
            << " ].ionized_mw_delta        = " << model.molecule[ j ].atom[ k ].p_residue->r_bead[ model.molecule[ j ].atom[ k ].bead_assignment ].ionized_mw_delta
            << Qt::endl
            ;

         if ( residues <= model.molecule[ j ].atom[ k ].model_residue_pos ) {
            continue;
         }

         QTextStream( stdout )
            << " model.molecule[ " << j << " ].residue[ " << model.molecule[ j ].atom[ k ].model_residue_pos << " ].r_atom[ " << model.molecule[ j ].atom[ k ].atom_assignment
            << " ].hybrid.mw               = " << model.residue[ model.molecule[ j ].atom[ k ].model_residue_pos ].r_atom[ model.molecule[ j ].atom[ k ].atom_assignment ].hybrid.mw
            << Qt::endl
            << " model.molecule[ " << j << " ].residue[ " << model.molecule[ j ].atom[ k ].model_residue_pos << " ].r_atom[ " << model.molecule[ j ].atom[ k ].atom_assignment
            << " ].hybrid.ionized_mw_delta = " << model.residue[ model.molecule[ j ].atom[ k ].model_residue_pos ].r_atom[ model.molecule[ j ].atom[ k ].atom_assignment ].hybrid.ionized_mw_delta
            << Qt::endl
            ;
      }
   }
   if ( !quiet ) {
      QTextStream( stdout )
         << "========================================================================================" << Qt::endl;
   }
}

// ionization notes
// for 0 pKa  -> nothing to do
// for 1 pKa  -> [A]<->[B]
//  ep1 = pow( 10, pH - pKa1 )
//  frac A =  1 / (ep1 + 1 )
//  frac B = 1 - frac A
// for 2 pKas -> [A]<->[B]<->[C]
//  ep1 = pow( 10, pH - pKa1 )
//  ep2 = pow( 10, pH - pKa2 )
//  frac C = ep1 * ep2 / ( ep1 * ( ep2 + 1 ) + 1 )
//  frac A = ( ( 1 - c ) * ep2 - c ) / ep2
//  or frac A = ( 1 - c ) / ( ep1 + 1 )
//  frac B = 1 - frac A - frac C

#define DEBUG_PKAS

void US_Hydrodyn::reset_ionized_residue_vectors() {
   set_ionized_residue_vector( residue_list );
}

vector < double > US_Hydrodyn::basic_fractions( float pH, struct residue * res ) {
   int pKas = (int) res->pKas.size();
   vector < double > fractions;
   if ( !pKas ) {
      fractions.push_back( 1e0 );
      return fractions;
   }
   double ep1 = pow( 10, pH - res->pKas[ 0 ] );
   if ( pKas == 1 ) {
      fractions.push_back( 1e0 / ( ep1 + 1e0 ) );
      fractions.push_back( 1e0 - fractions[ 0 ] );
      return fractions;
   }
   double ep2 = pow( 10, pH - res->pKas[ 1 ] );
   if ( pKas == 2 ) {
      double c = ep1 * ep2 / ( ep1 * ( ep2 + 1e0 ) + 1e0 );
      double a = ( ( 1e0 - c ) * ep2 - c ) / ep2;
      double b = 1e0 - a - c;
#if defined( DEBUG_PKAS )
      double a_debug = ( 1e0 - c ) / ( ep1 + 1e0 );
      if ( fabs( a_debug - a ) > 1e-10 ) {
         qDebug() <<
            QString( "US_Hydrodyn::basic_fractions(): Internal error - frac a varies %1 %2 " )
            .arg( a_debug )
            .arg( a )
            ;
      }         
      if ( fabs( a + b + c - 1 ) > 1e-10 ) {
         qDebug() <<
            QString( "US_Hydrodyn::basic_fractions(): Internal error - frac total != 1:  %1 + %2 + %3 = %4 " )
            .arg( a )
            .arg( b )
            .arg( c )
            .arg( a + b + c )
            ;
      }         
      if ( a < 0 || b < 0 || c < 0 ) {
         qDebug() <<
            QString( "US_Hydrodyn::basic_fractions(): Internal error - negative fraction != 1:  %1  %2  %3 " )
            .arg( a )
            .arg( b )
            .arg( c )
            ;
      }         
#endif
      
      fractions.push_back( a );
      fractions.push_back( b );
      fractions.push_back( c );
      return fractions;
   }
   QString msg = "US_Hydrodyn::basic_fractions(): Internal error - too many pKas - unsupported";
   qDebug() << msg;
   editor_msg( "red", msg );
   return fractions;
}

double US_Hydrodyn::ionized_residue_vbar( vector < double > & fractions, struct residue * res ) {
   int size = (int) fractions.size();
   if ( size == 1 ) {
      return res->vbar;
   }
   if ( size == 2 ) {
      return fractions[ 0 ] * res->vbar + fractions[ 1 ] * res->vbars[ 0 ];
   }
   if ( size == 3 ) {
      return fractions[ 0 ] * res->vbar + fractions[ 1 ] * res->vbars[ 0 ] + fractions[ 2 ] * res->vbars[ 1 ];
   }
   QString msg = "US_Hydrodyn::ionized_residue_vbar(): Internal error - too many fractions - unsupported";
   qDebug() << msg;
   editor_msg( "red", msg );
   return 0e0;
}

double US_Hydrodyn::ionized_residue_atom_mw( vector < double > & fractions, struct residue *res, struct atom *atom ) {
   int size = (int) fractions.size();
   if ( size == 1 || !atom->ionization_index ) {
      return 0e0;
   }

#if defined( DEBUG_PKAS )
   if ( !res->r_atom_0.count( atom->ionization_index ) ||
        !res->r_atom_1.count( atom->ionization_index ) ) {
      QString msg = "US_Hydrodyn::ionized_residue_atom_mw(): Internal error - r_atom maps missing";
      qDebug() << msg;
      editor_msg( "red", msg );
      return 0e0;
   }

   if ( atom->ionization_index > size ) {
      QString msg = "US_Hydrodyn::ionized_residue_atom_mw(): Internal error - atom's ionization index is out of bounds";
      qDebug() << msg;
      editor_msg( "red", msg );
      return 0e0;
   }
      
#endif

   if ( size == 2 ) {
      return
         fractions[ 1 ] * 
         ( res->r_atom_1[ atom->ionization_index ].hybrid.mw - res->r_atom_0[ atom->ionization_index ].hybrid.mw )
         ;
   }

   return
      ( fractions[ atom->ionization_index ] / ( fractions[ atom->ionization_index ] + fractions[ atom->ionization_index - 1 ] ) ) *
      ( res->r_atom_1[ atom->ionization_index ].hybrid.mw - res->r_atom_0[ atom->ionization_index ].hybrid.mw )
      ;
}

// #define DEBUG_RADIUS

double US_Hydrodyn::ionized_residue_atom_radius( vector < double > & fractions, struct residue *res, struct atom *atom ) {
   int size = (int) fractions.size();
   if ( size == 1 || !atom->ionization_index ) {
      return atom->hybrid.radius;
   }

   double r0_3 =
      (double) res->r_atom_0[ atom->ionization_index ].hybrid.radius *
      res->r_atom_0[ atom->ionization_index ].hybrid.radius *
      res->r_atom_0[ atom->ionization_index ].hybrid.radius;
      
   double r1_3 =
      (double) res->r_atom_1[ atom->ionization_index ].hybrid.radius *
      res->r_atom_1[ atom->ionization_index ].hybrid.radius *
      res->r_atom_1[ atom->ionization_index ].hybrid.radius;

   if ( size == 2 ) {
#if defined( DEBUG_RADIUS )
      QTextStream( stdout )
         << res->name << ","
         << atom->name << ","
         <<  res->r_atom_0[ atom->ionization_index ].hybrid.radius << ","
         <<  res->r_atom_1[ atom->ionization_index ].hybrid.radius << ","
         << fractions[ atom->ionization_index - 1 ] << ","
         << fractions[ atom->ionization_index ] << ","
         << pow( 
                fractions[ atom->ionization_index - 1 ] * r0_3 +
                fractions[ atom->ionization_index     ] * r1_3,
                1/3e0
                 ) << ","
         << Qt::endl
         ;
#endif
      return
         pow( 
             fractions[ atom->ionization_index - 1 ] * r0_3 +
             fractions[ atom->ionization_index     ] * r1_3,
             1/3e0
              );
   }

#if defined( DEBUG_RADIUS )
   QTextStream( stdout )
      << res->name << ","
      << atom->name << ","
      << res->r_atom_0[ atom->ionization_index ].hybrid.radius << ","
      << res->r_atom_1[ atom->ionization_index ].hybrid.radius << ","
      << fractions[ atom->ionization_index - 1 ] << ","
      << fractions[ atom->ionization_index ] << ","
      << pow( 
             (
              fractions[ atom->ionization_index - 1 ] * r0_3 +
              fractions[ atom->ionization_index     ] * r1_3
              ) /
             ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
             ,
             1/3e0
              ) << ","
      << Qt::endl
      ;
#endif

   return
      pow( 
          (
           fractions[ atom->ionization_index - 1 ] * r0_3 +
           fractions[ atom->ionization_index     ] * r1_3
           ) /
          ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
          ,
          1/3e0
           );

}   

// #define DEBUG_HYDRATION

double US_Hydrodyn::ionized_residue_atom_hydration( vector < double > & fractions, struct residue *res, struct atom *atom ) {
   int size = (int) fractions.size();
   if ( size == 1 || !atom->ionization_index ) {
      return atom->hydration;
   }

   if ( size == 2 ) {
#if defined( DEBUG_HYDRATION )
      QTextStream( stdout )
         << res->name << ","
         << atom->name << ","
         <<  res->r_atom_0[ atom->ionization_index ].hydration << ","
         <<  res->r_atom_1[ atom->ionization_index ].hydration << ","
         << fractions[ atom->ionization_index - 1 ] << ","
         << fractions[ atom->ionization_index ] << ","
         << (
             fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hydration +
             fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hydration
             ) << ","
         << Qt::endl
         ;
#endif
      return
         fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hydration +
         fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hydration
         ;
   }

#if defined( DEBUG_HYDRATION )
   QTextStream( stdout )
      << res->name << ","
      << atom->name << ","
      << res->r_atom_0[ atom->ionization_index ].hydration << ","
      << res->r_atom_1[ atom->ionization_index ].hydration << ","
      << fractions[ atom->ionization_index - 1 ] << ","
      << fractions[ atom->ionization_index ] << ","
      << ( 
          ( fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hydration +
            fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hydration
            ) /
          ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
           )
      << ","
      << Qt::endl
      ;
#endif

   return
      ( fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hydration +
        fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hydration
        ) /
      ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
      ;

}   

// #define DEBUG_PROTONS

double US_Hydrodyn::ionized_residue_atom_protons( vector < double > & fractions, struct residue *res, struct atom *atom ) {
   int size = (int) fractions.size();
   if ( size == 1 || !atom->ionization_index ) {
      return atom->hybrid.protons;
   }

   if ( size == 2 ) {
#if defined( DEBUG_PROTONS )
      QTextStream( stdout )
         << res->name << ","
         << atom->name << ","
         << res->r_atom_0[ atom->ionization_index ].hybrid.protons << ","
         << res->r_atom_0[ atom->ionization_index ].hybrid.name << ","
         << res->r_atom_1[ atom->ionization_index ].hybrid.protons << ","
         << res->r_atom_1[ atom->ionization_index ].hybrid.name << ","
         << fractions[ atom->ionization_index - 1 ] << ","
         << fractions[ atom->ionization_index ] << ","
         << (
             fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hybrid.protons +
             fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hybrid.protons
             ) << ","
         << Qt::endl
         ;
#endif
      return
         fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hybrid.protons +
         fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hybrid.protons
         ;
   }

#if defined( DEBUG_PROTONS )
   QTextStream( stdout )
      << res->name << ","
      << atom->name << ","
      << res->r_atom_0[ atom->ionization_index ].hybrid.protons << ","
      << res->r_atom_0[ atom->ionization_index ].hybrid.name << ","
      << res->r_atom_1[ atom->ionization_index ].hybrid.protons << ","
      << res->r_atom_1[ atom->ionization_index ].hybrid.name << ","
      << fractions[ atom->ionization_index - 1 ] << ","
      << fractions[ atom->ionization_index ] << ","
      << ( 
          ( fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hybrid.protons +
            fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hybrid.protons
            ) /
          ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
           )
      << ","
      << Qt::endl
      ;
#endif

   return
      ( fractions[ atom->ionization_index - 1 ] * res->r_atom_0[ atom->ionization_index ].hybrid.protons +
        fractions[ atom->ionization_index     ] * res->r_atom_1[ atom->ionization_index ].hybrid.protons
        ) /
      ( fractions[ atom->ionization_index - 1 ] + fractions[ atom->ionization_index ] )
      ;

}   

void US_Hydrodyn::set_ionized_residue_vector( vector < struct residue > & residue_v ) {
   // this will have to be changed later to true, as we are changing the format of residue
   // QTextStream( stdout )
   //    << "========================================================================================" << Qt::endl
   //    << "US_Hydrodyn::set_ionized_residue_vector_apply()" << Qt::endl
   //    << "========================================================================================" << Qt::endl
   //    ;

   // force use_pH
   gparams[ "use_pH" ] = "true";

   bool use_pH = gparams.count( "use_pH" ) && gparams[ "use_pH" ] == "true";

   if ( !use_pH ) {
      return;
   }

   float pH = le_pH->text().toFloat();
   int residues = (int) residue_v.size();
#if defined( DEBUG_RADIUS )
   QTextStream( stdout )
      << "pH" << "," << pH << Qt::endl
      << "resname" << ","
      << "atomname" << ","
      << "r_0 radius" << ","
      << "r_1 radius" << ","
      << "fraction prior" << ","
      << "fraction this" << ","
      << "computed radius" << Qt::endl;
#endif
#if defined( DEBUG_HYDRATION )
   QTextStream( stdout )
      << "pH" << "," << pH << Qt::endl
      << "resname" << ","
      << "atomname" << ","
      << "r_0 hydration" << ","
      << "r_1 hydration" << ","
      << "fraction prior" << ","
      << "fraction this" << ","
      << "computed hydration" << Qt::endl;
#endif
#if defined( DEBUG_PROTONS )
   QTextStream( stdout )
      << "pH" << "," << pH << Qt::endl
      << "resname" << ","
      << "atomname" << ","
      << "r_0 protons" << ","
      << "r_0 hybrid name" << ","
      << "r_1 protons" << ","
      << "r_1 hybrid name" << ","
      << "fraction prior" << ","
      << "fraction this" << ","
      << "computed protons" << Qt::endl;
#endif

   
   for ( int j = 0; j < residues; ++j ) {
      vector < double > fractions = basic_fractions( pH, & residue_v[ j ] );

      int atoms = (int) residue_v[ j ].r_atom.size();
      int beads = (int) residue_v[ j ].r_bead.size();
      
      map < int, double > bead_ionized_mw;
      map < int, double > bead_atom_hydration;
      set < int >         bead_has_atom_with_ionization_index;

      residue_v[ j ].pH               = pH;
      residue_v[ j ].vbar_at_pH       = ionized_residue_vbar( fractions, & residue_v[ j ] );
      residue_v[ j ].ionized_mw_delta = 0e0;


      for ( int k = 0; k < atoms; ++k ) {
         double ionized_mw_for_atom                                        = ionized_residue_atom_mw       ( fractions, & residue_v[ j ], & residue_v[ j ].r_atom[ k ] );
         double ionized_hydration                                          = ionized_residue_atom_hydration( fractions, & residue_v[ j ], & residue_v[ j ].r_atom[ k ] );
         residue_v[ j ].r_atom[ k ].hybrid.ionized_mw_delta                = ionized_mw_for_atom;
         residue_v[ j ].r_atom[ k ].hybrid.radius                          = ionized_residue_atom_radius   ( fractions, & residue_v[ j ], & residue_v[ j ].r_atom[ k ] );
         residue_v[ j ].r_atom[ k ].hydration                              = ionized_hydration;
         residue_v[ j ].r_atom[ k ].hybrid.protons                         = ionized_residue_atom_protons  ( fractions, & residue_v[ j ], & residue_v[ j ].r_atom[ k ] );
         residue_v[ j ].ionized_mw_delta                                   += ionized_mw_for_atom;
         bead_ionized_mw[ residue_v[ j ].r_atom[ k ].bead_assignment ]     += ionized_mw_for_atom;
         bead_atom_hydration[ residue_v[ j ].r_atom[ k ].bead_assignment ] += ionized_hydration;
         if ( residue_v[ j ].r_atom[ k ].ionization_index ) {
            bead_has_atom_with_ionization_index.insert( residue_v[ j ].r_atom[ k ].bead_assignment );
         }

         // vdwf
         {
            QString name = QString( "%1|%2" )
               .arg( residue_v[ j ].r_atom[ k ].name != "OXT" ?  residue_v[ j ].name : "OXT" )
               .arg( residue_v[ j ].r_atom[ k ].name );
            if ( vdwf.count( name ) ) {
               vdwf[ name ].ionized_mw_delta = ionized_mw_for_atom;
               vdwf[ name ].r                = residue_v[ j ].r_atom[ k ].hybrid.radius;
               vdwf[ name ].w                = residue_v[ j ].r_atom[ k ].hydration;
            } else {
               _vdwf this_vdwf;
               this_vdwf.mw               = residue_v[ j ].r_atom[ k ].hybrid.mw;
               this_vdwf.ionized_mw_delta = ionized_mw_for_atom;
               this_vdwf.r                = residue_v[ j ].r_atom[ k ].hybrid.radius;
               this_vdwf.w                = residue_v[ j ].r_atom[ k ].hydration;
               vdwf[ name ] = this_vdwf;
            }
         }
      }

      for ( int k = 0; k < beads; ++k ) {
         residue_v[ j ].r_bead[ k ].hydration_flag = bead_has_atom_with_ionization_index.count( k ) == 0;

         if ( bead_ionized_mw.count( k ) ) {
            residue_v[ j ].r_bead[ k ].ionized_mw_delta = bead_ionized_mw[ k ];
            residue_v[ j ].r_bead[ k ].atom_hydration   = bead_atom_hydration[ k ];
            bead_ionized_mw.erase( k );
            bead_atom_hydration.erase( k );
         } else {
            residue_v[ j ].r_bead[ k ].ionized_mw_delta = 0e0;
            residue_v[ j ].r_bead[ k ].atom_hydration   = 0e0;
         }

#if defined( DEBUG_HYDRATION )
         if ( !residue_v[ j ].r_bead[ k ].hydration_flag ) {
            QTextStream( stdout )
               << "residue name " << residue_v[j].name
               << " bead " << ( k + 1 )
               << " atom_hydration " <<  residue_v[ j ].r_bead[ k ].atom_hydration << Qt::endl
               ;
         }
#endif
      }
      if ( bead_ionized_mw.size() ) {
         QTextStream( stdout )
            << "**** US_Hydrodyn::set_ionized_residue_vector_apply(): unassigned bead_ionized_mw! ****" << Qt::endl
            ;
      }
   }

   // reset for fasta cache maps

   res_vbar.clear();
   res_mw  .clear();

   for ( int j = 0; j < residues; ++j ) {
      res_vbar[ residue_v[j].name ] = residue_v[j].vbar_at_pH;
      res_mw  [ residue_v[j].name ] = residue_v[j].mw + residue_v[j].ionized_mw_delta;
   }
      
   create_fasta_vbar_mw();

   // {
   //    set < QString > only_res = { "LYS", "GLU" };
   //    // set < QString > only_res = { "DA" };
      
   //    // QTextStream( stdout ) << "******** begin res_vbar, mw map ********" << Qt::endl;

   //    for ( auto it = res_vbar.begin();
   //          it != res_vbar.end();
   //          ++it ) {
   //       if ( only_res.count( it->first ) ) {
   //          QTextStream( stdout ) << it->first << " vbar " << it->second << " mw " << res_mw[ it->first ] << Qt::endl;
   //       }
   //    }
   //    // QTextStream( stdout ) << "******** end res_vbar, mw map ********" << Qt::endl;
   // }

   // QTextStream( stdout ) << "******** begin vdwf map ********" << Qt::endl;
   // for ( auto it = vdwf.begin();
   //       it != vdwf.end();
   //       ++it ) {
   //    if ( it->second.ionized_mw_delta ) {
   //       QTextStream( stdout ) << it->first << " mw " << it->second.mw << " ionized_mw_delta " << it->second.ionized_mw_delta << Qt::endl;
   //    }
   // }
   // QTextStream( stdout ) << "******** end vdwf map ********" << Qt::endl;
}

bool US_Hydrodyn::model_summary_csv( const QString & filename ) {
   // QTextStream( stdout ) << "model_summary_csv(1) " << filename << "\n";
   // QTextStream( stdout ) << "cwd " << QDir::currentPath() << "\n";

   if ( !model_vector.size() ) {
      return false;
   }
   return model_summary_csv( &(model_vector[0]), filename );
}

bool US_Hydrodyn::model_summary_csv( struct PDB_model *model, const QString & filename ) {
   QStringList header;
   QStringList data;
   // QTextStream( stdout ) << "model_summary_csv " << filename << "\n";
   // QTextStream( stdout ) << "cwd " << QDir::currentPath() << "\n";

   header << "structure";
   data   << le_pdb_file->text();

   if ( pdb_parse.find_sh ) {
      header
         << "Number of disulfide bonds"
         << "Number of free SH"
         ;
      data
         << QString( "%1" ).arg( model->num_SS_bonds )
         << QString( "%1" ).arg( model->num_SH_free )
         ;
   }

   header
      << "Calculation done at pH"
      << "Molecular weight [Da]"
      ;
   data
      << QString( "%1" ).arg( le_pH->text() )
      << QString( "%1" ).arg( model->mw + model->ionized_mw_delta )
      ;


   if ( model->molar_volume > 0 ) {
      header << "Molar volume [cm^3/mol]";
      data   << QString("%1").arg( model->molar_volume );
   }

   // tbd. qs += vbar_msg( model->vbar );

   header << "SAXS excluded volume (anhydrous) [" << UNICODE_ANGSTROM << "^3]";
   data   << QString( "%1" ).arg( model->volume );
   
   
   // if ( hydro.temperature != 20 ) {
   //    qs +=
   //       QString(
   //               us_tr(
   //                     "Anh. Molecular vol. (from vbar)  : %1 [A^3] @ %2%3C\n"
   //                     )
   //               )
   //       .arg( mw_to_volume( model->mw + model->ionized_mw_delta, model->vbar ) )
   //       .arg( 20 )
   //       .arg( DEGREE_SYMBOL )
   //       ;
   // }

   header
      << "Anh. Molecular vol. (from vbar) [" << UNICODE_ANGSTROM << "^3]"
      << "Hyd. Molecular vol. (from vbar) [" << UNICODE_ANGSTROM << "^3]"
      << "Radius of gyration [" << UNICODE_ANGSTROM << "]"
      << "Number of electrons"
      << "Number of protons"
      << "Net charge"
      << "Isoelectric point"
      << "Hydration [g/g]"
      ;

   data
      << QString( "%1" ).arg( mw_to_volume( model->mw + model->ionized_mw_delta, tc_vbar( model->vbar ) ) )
      << QString( "%1" ).arg( mw_to_volume( model->mw + model->ionized_mw_delta, tc_vbar( model->vbar ) ) + model->hydration * misc.hydrovol )
      << QString( "%1" ).arg( model->Rg, 0, 'f', 2 )
      << QString( "%1" ).arg( model->num_elect )
      << QString( "%1" ).arg( model->protons, 0, 'f', 1 )
      << QString( "%1" ).arg( model->protons - model->num_elect, 0, 'f', 1 )
      << QString( "%1" ).arg( model->isoelectric_point, 0, 'f', 2 )
      << QString( "%1" ).arg( model->hydration_gg, 0, 'g', 3 )
      ;      
      
   if ( model->volume ) {
      header << "Average electron density [" << UNICODE_ANGSTROM << "^-3]";
      data   << QString( "%1" ).arg( model->num_elect / model->volume, 0, 'f', 3 );
   }

   QString error;
   QString qs =
      "\"" + header.join( "\",\"") + "\"\n"
      + data.join( "," ) + "\n";
   
   return US_File_Util::putcontents( filename, qs, error );
}   


void US_Hydrodyn::select_atom_file(const QString &filename)
{
   QString str1;
   QFileInfo fi(filename);
   // lbl_atom_table->setText(fi.baseName() + "." + fi.completeSuffix());
   atom_list.clear( );
   atom_map.clear( );
   QFile f(filename);
   struct atom current_atom;
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
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
   }
   // add generic ABB
   {
      atom abb_atom;
      abb_atom.name                    = "ABB";
      abb_atom.hybrid.name             = "ABB";
      abb_atom.hybrid.mw               = misc.avg_mass;
      abb_atom.hybrid.ionized_mw_delta = 0;
      abb_atom.hybrid.radius           = misc.avg_radius;
      abb_atom.hybrid.scat_len         = 0;
      abb_atom.hybrid.saxs_name        = "ABB";
      abb_atom.hybrid.num_elect        = misc.avg_num_elect;
      abb_atom.hybrid.protons          = misc.avg_protons;
      abb_atom.saxs_excl_vol           = (4/3)*M_PI*pow(misc.avg_radius, 3 );

      atom_list.push_back(abb_atom);
      atom_map[abb_atom.name + "~" + abb_atom.hybrid.name] = abb_atom;
   }   
}
