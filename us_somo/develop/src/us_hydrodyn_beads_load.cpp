// this is part of the class US_Hydrodyn
// listing of other files is in us_hydrodyn.cpp
// (this) us_hydrodyn_beads_load.cpp contains code for reading the bead model file

#include "../include/us_hydrodyn.h"
#define SLASH "/"

#define TOLERANCE overlap_tolerance

int US_Hydrodyn::read_bead_model( QString filename, bool &only_overlap )
{
   last_read_bead_model = filename;
   lb_model->clear( );
   le_pdb_file_save_text = "not selected";
   le_pdb_file->setText(us_tr( "not selected" ));
   project = filename;
   //   project.replace(QRegExp(".*(/|\\\\)"), "");
   //   project.replace(QRegExp("\\.(somo|SOMO)\\.(bead_model|BEAD_MODEL)$"), "");
   project = QFileInfo(QFileInfo(filename).fileName()).baseName();
   QString ftype = QFileInfo(filename).suffix().toLower();
   editor->setText("\n\nLoading bead model " + project + " of type " + ftype + "\n");
   bead_model.clear( );
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

   bool         read_vdw                       = false;
   double       read_vdw_theo_waters           = -1;
   int          read_vdw_count_exposed         = -1;
   double       read_vdw_theo_waters_exposed   = -1;
   float        read_asa_hydrate_probe_radius  = -1;
   float        read_asa_hydrate_threshold     = -1;

   if ( ftype == "bead_model" )
   {
      // check for vdw data
      {
         QStringList qsl;
         qDebug() << "check for vdw data";
         US_File_Util ufu;
         
         if ( ufu.read( filename, qsl ) && qsl.filter( "vdW model parameters:" ).size() ) {
            QStringList qsl_check = qsl.filter( QRegularExpression( "^  (Hydrate probe radius|Hydrate threshold|Theoretical waters|Exposed|Theoretical waters exposed).*:" ) );
            if ( qsl_check.size() == 5 ) {
               double res[qsl_check.size()];
               for ( int i = 0; i < (int) qsl_check.size(); ++i ) {
                  res[i] = qsl_check[i].replace( QRegularExpression( "^.*: " ), "" ).toDouble();
               }
               read_vdw_theo_waters           = res[0];
               read_vdw_count_exposed         = (int)res[1];
               read_vdw_theo_waters_exposed   = res[2];
               read_asa_hydrate_probe_radius  = (float)res[3];
               read_asa_hydrate_threshold     = (float)res[4];
               read_vdw                       = true;

               QTextStream( stdout )
                  << "vdw data found in bead model\n"
                  << "-----------\n"
                  << "read_vdw_theo_waters : " << read_vdw_theo_waters << "\n"
                  << "read_vdw_count_exposed : " << read_vdw_count_exposed << "\n"
                  << "read_vdw_theo_waters_exposed : " << read_vdw_theo_waters_exposed << "\n"
                  << "read_asa_hydrate_probe_radius : " << read_asa_hydrate_probe_radius << "\n"
                  << "read_asa_hydrate_threshold : " << read_asa_hydrate_threshold << "\n"
                  << "-----------\n"
                  ;
                  
            } else {
               qDebug() << "vdw bead model, but missing data\n";
            } 
         } else {
            qDebug() << "not a vdw bead model with generating data\n";
         }
      }


      if ( f.open( QIODevice::ReadOnly ) )
      {
         bool so_ovlp = QFileInfo( f ).completeBaseName().contains( "so_ovlp" );
         us_qdebug( QString( "so_ovlp %1" ).arg( so_ovlp ? "true" : "false" ) );
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
               tmp_atom.bead_ref_ionized_mw_delta = 0;
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
               ts >> tmp_atom.bead_asa;
            } else {
               tmp_atom.bead_asa = 0;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.num_elect;
            }
            else
            {
               tmp_atom.num_elect = 0;
            }
            if (!ts.atEnd()) {
               ts >>  tmp_atom.saxs_excl_vol;
            }
            else
            {
               tmp_atom.saxs_excl_vol = 0;
            }
            if (!ts.atEnd()) {
               ts >> tmp_atom.bead_hydration;
            }
            else
            {
               tmp_atom.bead_hydration = 0;
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
            tmp_atom.all_beads.clear( );
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
         if ( read_vdw && bead_model.size() ) {
            bead_model[0].is_vdw                   = "vdw";
            bead_model[0].vdw_theo_waters          = read_vdw_theo_waters;
            bead_model[0].vdw_count_exposed        = read_vdw_count_exposed;
            bead_model[0].vdw_theo_waters_exposed  = read_vdw_theo_waters_exposed;
            bead_model[0].asa_hydrate_probe_radius = read_asa_hydrate_probe_radius;
            bead_model[0].asa_hydrate_threshold    = read_asa_hydrate_threshold;
            qDebug() << "loaded vdw parameters";
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
               if ( str.left( 6 ).toLower().contains( "__json" ) )
               {
                  continue;
               }
               if ( str.left( 6 ).toLower() == "saxs::" )
               {
                  ssaxs << str;
                  continue;
               }

               if ( str.left( 7 ).toLower() == "bsaxs::" )
               {
                  bsaxs << str;
                  continue;
               }

               if ( str.left( 8 ).toLower() == "bsaxsv::" )
               {
                  bsaxsv << str;
                  continue;
               }

               editor->append(str);
               if ( rx.indexIn(str) != -1 )
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
                           QString( us_tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                           .arg( bsaxs.size() )
                           .arg( bead_model.size() )
                           );
            }
               

            if ( ssaxs.size() && ( saxs_options.iq_global_avg_for_bead_models || ( bsaxs.size() && ( bead_model.size() % ( bsaxs.size() / 2 ) ) ) ) )
            {
               editor_msg( "dark blue", 
                           QString( us_tr( "Found %1 saxs coefficient lines in bead model file\n" ) )
                           .arg( ssaxs.size() ) );
               if ( ssaxs.size() > 2 )
               {
                  editor_msg( "red", 
                              QString( us_tr( "Error: saxs coefficients found in file, but incorrect # of lines %1 vs 2 expected" ) )
                              .arg( ssaxs.size() ) );
               } else {
                  saxs tmp_saxs;
                  QStringList qsl = (ssaxs[ 0 ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
                  if ( qsl.size() != 12 )
                  {
                     editor_msg( "red", 
                                 QString( us_tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 12 required" ) )
                                 .arg( qsl.size() ) );
                  } else {
                     // editor_msg( "dark blue",  us_tr( "Four term saxs coefficients found\n" ) );
                     tmp_saxs.saxs_name = qsl[ 1 ].toUpper();
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
                        qsl = (ssaxs[ 1 ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );

                        if ( qsl.size() != 14 )
                        {
                           editor_msg( "red", 
                                       QString( us_tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 14 required" ) )
                                       .arg( qsl.size() ) );
                        } else {
                           // editor_msg( "dark blue",  us_tr( "Five term saxs coefficients found\n" ) );
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
                                    QString( us_tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                    .arg( tmp_saxs.saxs_name ) );
                     } else {
                        saxs_util->saxs_list.push_back( tmp_saxs );
                        editor_msg( "dark blue", 
                                    QString( us_tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                                    .arg( tmp_saxs.saxs_name ) );
                     }
                     saxs_util->saxs_map[ tmp_saxs.saxs_name ] = tmp_saxs;

                     if ( saxs_plot_widget )
                     {
                        if ( saxs_plot_window->saxs_map.count( tmp_saxs.saxs_name ) )
                        {
                           editor_msg( "dark red", 
                                       QString( us_tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                                       .arg( tmp_saxs.saxs_name ) );
                        } else {
                           saxs_plot_window->saxs_list.push_back( tmp_saxs );
                           editor_msg( "dark blue", 
                                       QString( us_tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
                                       .arg( tmp_saxs.saxs_name ) );
                        }
                        saxs_plot_window->saxs_map[ tmp_saxs.saxs_name ] = tmp_saxs;
                     }

                     if ( saxs_options.compute_saxs_coeff_for_bead_models && 
                          saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                     {
                        if ( !saxs_util->saxs_map.count( saxs_options.dummy_saxs_name ) )
                        {
                           editor_msg( "red", QString( us_tr("Warning: No '%1' SAXS atom found. Bead model SAXS disabled.\n" ) )
                                       .arg( saxs_options.dummy_saxs_name ) );
                        } else {
                           editor_msg( "blue", QString( us_tr("Notice: Loading dummy atoms with saxs coefficients '%1'" ) )
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
               editor_msg( "blue", us_tr( "Notice: using individual bead structure factors\n" ) );
               sf_bead_factors.resize( bsaxs.size() );
               bool do_bsaxsv = false;
               if ( bsaxsv.size() )
               {
                  if ( bsaxsv.size() * 2 == bsaxs.size() )
                  {
                     editor_msg( "blue", us_tr( "Notice: found correct # of variable length structure factors\n" ) );
                     do_bsaxsv = true;
                  } else {
                     editor_msg( "red", us_tr( "Notice: found incorrect # of variable length structure factors, variable length not loaded\n" ) );
                  }
               }
                     
               for ( unsigned int j = 0; j < ( unsigned int ) bsaxs.size(); j += 2 )
               {
                  saxs tmp_saxs;
                  QStringList qsl = (bsaxs[ j ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
                  if ( qsl.size() != 12 )
                  {
                     editor_msg( "red", 
                                 QString( us_tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 12 required" ) )
                                 .arg( qsl.size() ) );
                  } else {
                     // editor_msg( "dark blue",  us_tr( "Four term saxs coefficients found\n" ) );
                     tmp_saxs.saxs_name = qsl[ 1 ].toUpper();
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
                        qsl = (bsaxs[ j + 1 ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
                        if ( tmp_saxs.saxs_name != qsl[ 1 ].toUpper() )
                        {
                           editor_msg( "red", us_tr( "Error: bead saxs coefficients bead number inconsistancy" ) );
                        } else {
                           if ( qsl.size() != 14 )
                           {
                              editor_msg( "red", 
                                          QString( us_tr( "Error: saxs coefficients found in file, but incorrect # of tokens %1 vs 14 required" ) )
                                          .arg( qsl.size() ) );
                           } else {
                              // editor_msg( "dark blue",  us_tr( "Five term saxs coefficients found\n" ) );
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
                     tmp_saxs.vcoeff.clear( );
                     if ( do_bsaxsv )
                     {
                        qsl = (bsaxsv[ j / 2 ] ).split( QRegExp( "\\s+" ) , Qt::SkipEmptyParts );
                        QTextStream( stdout ) << QString( "loading: bvsaxs qsl size %1\n" ).arg( qsl.size() );
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
                     
               editor_msg( "dark blue", us_tr( "Bead model structure factors saved for reapplication" ) );
            }

            if ( !units_loaded )
            {
               editor_msg("red", 
                          us_tr("\nNote: The model scale (nm, angstrom, etc.) was not found in this bead model file.\n"
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
         lb_model->clear( );
         lb_model->addItem("Model 1 from bead_model file");
         lb_model->item(0)->setSelected( true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         somo_processed.resize(lb_model->count());
         bead_models.resize(lb_model->count());
         current_model = 0;
         bead_check( false, false, false, true );
         bead_models[0] = bead_model;
         somo_processed[0] = 1;
         bead_models_as_loaded = bead_models;
         editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
         if ( so_ovlp ) {
            only_overlap = true;
            return 0;
         } else {
            int overlap_check_results = overlap_check( true, true, true,
                                                       hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance, 20 );
            only_overlap = true;
            return( misc.hydro_zeno ? 0 : overlap_check_results );
         }
      }
   }

   if (ftype == "beams")
   {
      if (f.open(QIODevice::ReadOnly))
      {
         QTextStream ts(&f);
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
         QFile frmc(QFileInfo(filename).path() + SLASH + rmcfile);
         if (frmc.open(QIODevice::ReadOnly))
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
                  tmp_atom.bead_ref_ionized_mw_delta = 0;
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
               tmp_atom.all_beads.clear( );
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

         lb_model->clear( );
         lb_model->addItem("Model 1 from beams file");
         lb_model->item(0)->setSelected( true);
         lb_model->setEnabled(false);
         model_vector.resize(1);
         model_vector[0].vbar = results.vbar;
         model_vector[0].Rg = results.rg;
         somo_processed.resize(lb_model->count());
         bead_models.resize(lb_model->count());
         current_model = 0;
         bead_models[0] = bead_model;
         somo_processed[0] = 1;
         bead_models_as_loaded = bead_models;
         editor->append(QString("\nMolecular weight: %1 Daltons\n\n").arg(tmp_mw));
         editor->append( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
         int overlap_check_results = overlap_check( true, true, true,
                                                    hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance, 20 );
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
            editor_msg( "red", QString( us_tr("Warning: No '%1' SAXS atom found. Bead model SAXS disabled.\n" ) )
                        .arg( saxs_options.dummy_saxs_name ) );
         } else {
            editor_msg( "blue", QString( us_tr("Notice: Loading dummy atoms with saxs coefficients '%1'" ) )
                        .arg( saxs_options.dummy_saxs_name ) );
         }               
      }

      QRegExp rx_model( "^MODEL\\s+(\\S+)" );
      model_count = 0;
      model_names.clear( );

      unsigned int unit_mult = 1;

      if (f.open(QIODevice::ReadOnly))
      {

         QRegExp rx_psv( "^REMARK\\s+PSV(\\s*:|)\\s+(\\S+)", Qt::CaseInsensitive );
         QRegExp rx_mw ( "^REMARK\\s+MW(\\s*:|)\\s+(\\S+)", Qt::CaseInsensitive );
         QRegExp rx_unit( "^REMARK\\s+Units exponent(\\s*:|)\\s+(\\S+)", Qt::CaseInsensitive );
         QRegExp rx_mult( "^REMARK\\s+Units conversion factor(\\s*:|)\\s+(\\S+)", Qt::CaseInsensitive );
         double loaded_psv = 0e0;
         double loaded_mw  = 0e0;
         unsigned int loaded_unit = 0;
         bool loaded_mult = false;

         QStringList qsl;
         {
            QTextStream ts(&f);
            do {
               QString qs = ts.readLine();
               qsl << qs;
               if ( rx_model.indexIn( qs ) != -1 )
               {
                  model_count++;
                  model_names.push_back( rx_model.cap( 1 ) );
               }
               if ( rx_psv.indexIn( qs ) != -1 )
               {
                  loaded_psv = rx_psv.cap( 2 ).toDouble();
                  editor_msg( "blue", QString( us_tr( "Found PSV %1 in PDB" ) ).arg( loaded_psv ) );
               }
               if ( rx_mw.indexIn( qs ) != -1 )
               {
                  loaded_mw = rx_mw.cap( 2 ).toDouble();
                  editor_msg( "blue", QString( us_tr( "Found MW %1 in PDB" ) ).arg( loaded_mw ) );
               }
               if ( rx_unit.indexIn( qs ) != -1 )
               {
                  loaded_unit = rx_unit.cap( 2 ).toUInt();
                  editor_msg( "blue", QString( us_tr( "Found Units exponent %1 in PDB" ) ).arg( loaded_unit ) );
                  if ( loaded_unit == 10 )
                  {
                     saxs_options.dummy_atom_pdbs_in_nm = true;
                  }
                  if ( loaded_unit == 9 )
                  {
                     saxs_options.dummy_atom_pdbs_in_nm = false;
                  }
               }
               if ( rx_mult.indexIn( qs ) != -1 )
               {
                  loaded_mult = true;
                  unit_mult = rx_mult.cap( 2 ).toUInt();
                  editor_msg( "blue", QString( us_tr( "Found Units conversion %1 in PDB" ) ).arg( unit_mult ) );
               }
            } while (!ts.atEnd());
         }
         f.close();


         if ( loaded_mult &&
              !loaded_unit &&
              saxs_options.dummy_atom_pdbs_in_nm )
         {
            editor_msg( "blue", us_tr( "Notice: Unit conversion factor in PDB forces Angstrom units" ) );
            saxs_options.dummy_atom_pdbs_in_nm = false;
         }

         if ( saxs_options.dummy_atom_pdbs_in_nm )
         {
            editor_msg( "blue", us_tr("Notice: This PDB is being loaded in NM units" ) );
         }

         model_count = model_count ? model_count : 1;
         QTextStream( stdout ) << QString( "models: %1\n" ).arg( model_count );

         if ( !f.open(QIODevice::ReadOnly) )
         {
            editor->append("File read error\n");
            return -2;
         }
         
         // make sure it is a valid file

         bool damaver = (qsl.filter("Number of atoms written").count() > 0 &&
                         !qsl.filter("Atomic radius").count() );
         bool dammin = (qsl.filter("Number of particle atoms").count() > 0);
         bool dammif = (qsl.filter("Dummy atoms in output phase").count() > 0);
         bool damfilt = (qsl.filter("Number of atoms written").count() > 0 &&
                         qsl.filter("Atomic radius").count() );
         bool damfilt5 = (qsl.filter("Filtered number of atoms").count() > 0 &&
                          qsl.filter("Atomic Radius").count() );

         bool em2dam = (qsl.filter("Number of dummy atoms").count() > 0 &&
                        qsl.filter("Dummy atom radius").count());

         if ( !damaver &&
              !dammin &&
              !dammif &&
              !damfilt &&
              !damfilt5 &&
              !em2dam
              )
         {
            editor->append("Error in DAMMIN/DAMMIF file: couldn't find 'Dummy atoms in output phase' or 'Number of particle atoms' of 'Number of atoms written'\n");
            f.close();
            return 1;
         }

         // find last atom number
         QRegExp rx;

         QStringList qsl_atom = qsl.filter(QRegExp("^ATOM "));
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
         if ( rx.indexIn(last_atom) ) 
         {
            editor->append("Couldn't find a last atom number in the DAMMIN/DAMMIF/DAMAVER file\n");
            f.close();
            return 1;
         }
         
         // determine and compare bead count
         int tmp_bead_count = rx.cap(1).toInt();
         printf("read bead count %d\n", tmp_bead_count);

         // count atom lines
         int atom_line_count = qsl.filter(QRegExp("^ATOM ")).count();
         printf("atom line count %d\n", atom_line_count);

         bead_count = atom_line_count;
         bead_count /= model_count;

         QTextStream ts(&f);
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
            if ( rx.indexIn(tmp) == -1 ) 
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
            
            if ( rx.indexIn(tmp) == -1 ) 
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
            if ( rx.indexIn(tmp) == -1 ) 
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
            
            if ( rx.indexIn(tmp) == -1 ) 
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
            
            if ( rx.indexIn(tmp) == -1 ) 
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
                      !tmp.toLower().contains("atomic radius") );
               
            if ( ts.atEnd() )
            {
               editor->append("Error in DAMFILT file: couldn't find 'Atomic radius'\n");
               return 1;
            }
            
            rx.setPattern("atomic radius \\.*\\s*:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.indexIn(tmp.toLower()) == -1 ) 
            {
               editor->append("Error in DAMFILT file: couldn't find radius in 'Atomic radius' line\n");
               return 1;
            }
            radius = rx.cap(1).toFloat();
         }

         if ( em2dam ) {
            puts("em2dam");
            
            do {
               tmp = ts.readLine();
               ++linepos;
            } while ( !ts.atEnd() && 
                      !tmp.toLower().contains("dummy atom radius") );
               
            if ( ts.atEnd() )
            {
               editor->append("Error in EM2DAM file: couldn't find 'Dummy atom radius'\n");
               return 1;
            }
            
            rx.setPattern("dummy atom radius \\.*\\s*:\\s*(\\d+\\.\\d+)\\s*");
            
            if ( rx.indexIn(tmp.toLower()) == -1 ) 
            {
               editor->append("Error EM2DAM DAMFILT file: couldn't find radius in 'Dummy atom radius' line\n");
               return 1;
            }
            radius = rx.cap(1).toFloat();
         }

         if ( saxs_options.dummy_atom_pdbs_in_nm )
         {
            radius *= 10.0;
         }

         radius *= (float) unit_mult;

         editor->append(QString("DAMMIN/DAMMIF/DAMAVER/DAMFILT/EM2DAM model atom radius %1 Angstrom\n").arg(radius));
         
         // enter MW and PSV
         float mw = loaded_mw;
         float psv = loaded_psv;
         bool do_write_bead_model = true;
         bool remember = true;
         bool use_partial = false;
         QString partial = filename;
         QString msg = QString(us_tr(" Enter values for vbar and total molecular weight: "));

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
                  printf("iterator first %s\n", it->first.toLatin1().data());
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
            editor_msg( "red", us_tr( "Warning: application of preloaded structure factors requested but no factors are loaded" ) );
            do_bead_saxs_assign = false;
         }
         if ( do_bead_saxs_assign && bead_count % sf_bead_factors.size() )
         {
            editor_msg( "red", 
                        QString( us_tr( "Warning: application of preloaded structure factors requested but the number of factors loaded %1 does not divide the number of beads %2" ) )
                        .arg( sf_bead_factors.size() )
                        .arg( bead_count )
                        );
            do_bead_saxs_assign = false;
         }
         if ( do_bead_saxs_assign )
         {
            editor_msg( "blue", us_tr( "applying preloaded structure factors" ) );
         }

         while ( (unsigned int )model_names.size() < model_count )
         {
            model_names.push_back( QString( "%1" ).arg( model_names.size() + 1 ) );
         }
         for ( unsigned int i = 0; i < model_count; i++ )
         {
            editor_msg( "gray", QString( us_tr( "Loading from model %1" ) ).arg( model_names[ i ] ) );
            int beads_loaded = 0;
            bead_model.clear( );
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
               tmp_atom.bead_ionized_mw_delta = 0;
               tmp_atom.bead_ref_mw = tmp_atom.bead_mw;
               tmp_atom.bead_ref_ionized_mw_delta = 0;
               tmp_atom.bead_color = 8;

               tmp_atom.exposed_code = 1;
               tmp_atom.all_beads.clear( );
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
               // QTextStream( stdout ) << QString("bead loaded serial %1\n").arg(tmp_atom.serial);
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
         lb_model->clear( );
         model_vector.resize( model_count );
         somo_processed.resize( model_count );
         bead_models.resize( model_count );
         for ( unsigned int i = 0; i < model_count; i++ )
         {
            lb_model->addItem( QString( "Model %1 (from bead model)" ).arg( model_names[ i ] ) );
            model_vector[ i ].vbar = results.vbar;
            model_vector[ i ].model_id = model_names[ i ];
            model_vector[ i ].mw = mw;
            model_vector[ i ].ionized_mw_delta = 0;
            somo_processed[ i ] = 1;
         }
         lb_model->item(0)->setSelected( true);
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
         if ( em2dam )
         {
            bead_model_suffix = "em2dam";
         }

         le_bead_model_suffix->setText( "<center>" + bead_model_suffix + "</center>" );

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
            editor_msg( "gray", QString( us_tr( "checking overlap for Model %1" ) ).arg( model_names[ i ] ) );
            bead_model = bead_models[ i ];
            overlap_check_results += overlap_check( true, true, true,
                                                    hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance, 20 );
         }
         bead_model = bead_models[ 0 ];
         only_overlap = true;
         return( misc.hydro_zeno ? 0 : overlap_check_results );
      }
   }

   editor->append("File read error\n");
   return -2;
}

void US_Hydrodyn::load_bead_model()
{
   QString use_dir = 
      path_load_bead_model.isEmpty() ?
      somo_dir :
      path_load_bead_model;

   select_from_directory_history( use_dir, this );

   QString filename = QFileDialog::getOpenFileName( this , "Open" , use_dir , "Bead models (*.bead_model *.BEAD_MODEL);;"
                                                   "BEAMS (*.beams *.BEAMS);;"
                                                   "DAMMIN/DAMMIF/DAMAVER (*.pdb)" , &bead_model_selected_filter );


   if ( !filename.isEmpty() )
   {
      path_load_bead_model = QFileInfo(filename).absolutePath();
   }

   if ( QFileInfo(filename).fileName().contains(" ") )
   {
      printError(us_tr("Filenames containing spaces are not currently supported.\n"
                    "Please rename the file to remove spaces."));
      return;
   }

   if (!filename.isEmpty())
   {
      add_to_directory_history( filename );
      citation_load_bead_model( filename );

      options_log = "";
      pb_somo->setEnabled(false);
      pb_somo_o->setEnabled(false);
      pb_visualize->setEnabled(false);
      pb_equi_grid_bead_model->setEnabled(false);
      pb_calc_hydro->setEnabled(false);
      pb_calc_zeno->setEnabled(false);
      pb_calc_grpy->setEnabled( false);
      pb_calc_hullrad->setEnabled( false );
      pb_show_hydro_results->setEnabled(false);
      pb_grid_pdb->setEnabled(false);
      pb_vdw_beads->setEnabled(false);
      pb_grid->setEnabled(false);
      bead_model_prefix = "";
      le_bead_model_prefix->setText(bead_model_prefix);
      bead_model_suffix = "";

      if (results_widget)
      {
         results_window->close();
         delete results_window;
         results_widget = false;
      }

      bead_model_file = filename;
      le_bead_model_file->setText( QDir::toNativeSeparators( filename ) );

      if ( is_dammin_dammif(filename) &&
           advanced_config.auto_view_pdb ) 
      {
#if defined(START_RASMOL)
         model_viewer( filename );
#endif
      }

      bool only_overlap = false;
      if (!read_bead_model(filename, only_overlap ))
      {
         bool so_ovlp = QFileInfo( filename ).completeBaseName().contains( "so_ovlp" );
         us_qdebug( QString( "load bead model so_ovlp %1" ).arg( so_ovlp ? "true" : "false" ) );
         state = BEAD_MODEL_LOADED;
         pb_visualize->setEnabled(true);
         pb_equi_grid_bead_model->setEnabled(true);
         pb_calc_hydro->setEnabled( !so_ovlp );
         pb_calc_zeno->setEnabled(true);
         pb_calc_grpy->setEnabled( true );
         pb_calc_hullrad->setEnabled( true );
         pb_grid->setEnabled(true);
         pb_bead_saxs->setEnabled(true);
         pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
         pb_pdb_saxs->setEnabled(false);
         bd_anaflex_enables(false);
         pb_somo     ->setEnabled( false );
         pb_somo_o   ->setEnabled( false );
         pb_somo     ->setEnabled( false );
         pb_grid_pdb ->setEnabled( false );
         pb_vdw_beads->setEnabled( false );
      }
      else
      {
         if ( only_overlap )
         {
            state = BEAD_MODEL_LOADED;
            pb_visualize->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_calc_hydro->setEnabled( false );
            pb_calc_zeno->setEnabled( true );
            pb_calc_grpy->setEnabled( true );
            pb_calc_hullrad->setEnabled( true );
            pb_grid->setEnabled(true);
            pb_bead_saxs->setEnabled(true);
            pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
            pb_pdb_saxs->setEnabled(false);
            bd_anaflex_enables(false);
            pb_somo     ->setEnabled( false );
            pb_somo_o   ->setEnabled( false );
            pb_somo     ->setEnabled( false );
            pb_grid_pdb ->setEnabled( false );
            pb_vdw_beads->setEnabled( false );

         } else {            
            pb_visualize->setEnabled(true);
            pb_equi_grid_bead_model->setEnabled(true);
            pb_bead_saxs->setEnabled(false);
            pb_rescale_bead_model->setEnabled(false);
            pb_pdb_saxs->setEnabled(true);
         }
      }
      // bead_model_prefix = "";
   }
}
