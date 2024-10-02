#include "../include/us_hydrodyn.h"
#include "../include/us_surfracer.h"

#include "../include/us_hydrodyn_grid_atob_hydro.h"
#include "../include/us_hydrodyn_asab1_hydro.h"

#include "../include/us_hydrodyn_pat_hydro.h"
#include "../include/us_hydrodyn_supc_hydro.h"

#include "../include/us_hydrodyn_zeno_hydro.h"

#include "../include/us_vvv.h"
#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"
#include "../include/us_pm.h"
#include "../include/us_timer.h"

#include <qregexp.h>
#include <qfont.h>
//Added by qt3to4:
#include <QTextStream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#ifdef WIN32
# include <sys/timeb.h>
#else
# include <sys/time.h>
#endif

#define SLASH "/"
#define DOTSOMO      ""

#undef DEBUG
#undef DEBUG1
#undef DEBUG2
#undef DEBUG3

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

void reset_hydro_res(hydro_results results_hydro);

bool US_Saxs_Util::run_hydro(
			     map < QString, QString >           & parameters,
			     map < QString, QString >           & results
			     )
{
  //results[ "errors" ] = " ";
  //USglobal = new US_Config();
 
  paths += getenv("ULTRASCAN");

  if ( !set_default(results, parameters) )         // setup configuration defaults before reading initial config
    return false;
  
  
  misc.restore_pb_rule = false;

  residue_filename = paths + SLASH + "etc" + SLASH + "somo.residue";
  QFileInfo fi_res_filename( residue_filename );
  if ( !fi_res_filename.exists() )
    {
      results[ "errors" ] += QString( " file %1 does not exist." ).arg( residue_filename );
      return false;
    }
  
  accumulated_msgs = "";
  
  read_residue_file();
  
  reset_hydro_res(results_hydro);

  residue_short_names["GLY"] = 'G';
  residue_short_names["ALA"] = 'A';
  residue_short_names["VAL"] = 'V';
  residue_short_names["LEU"] = 'L';
  residue_short_names["ILE"] = 'I';
  residue_short_names["MET"] = 'M';
  residue_short_names["PHE"] = 'F';
  residue_short_names["TRP"] = 'W';
  residue_short_names["PRO"] = 'P';
  residue_short_names["SER"] = 'S';
  residue_short_names["THR"] = 'T';
  residue_short_names["CYS"] = 'C';
  residue_short_names["CYH"] = 'B';
  residue_short_names["TYR"] = 'Y';
  residue_short_names["ASN"] = 'N';
  residue_short_names["GLN"] = 'Q';
  residue_short_names["ASP"] = 'D';
  residue_short_names["GLU"] = 'E';
  residue_short_names["LYS"] = 'K';
  residue_short_names["ARG"] = 'R';
  residue_short_names["HIS"] = 'H';
  residue_short_names["WAT"] = '~';
  
  residue_short_names["G"] = 'g';
  residue_short_names["A"] = 'a';
  residue_short_names["C"] = 'c';
  residue_short_names["T"] = 't';
  residue_short_names["U"] = 'u';
  residue_short_names["DG"] = 'g';
  residue_short_names["DA"] = 'a';
  residue_short_names["DC"] = 'c';
  residue_short_names["DT"] = 't';


  
  // saxs_util = new US_Saxs_Util();

  if (setup_saxs_maps( saxs_options_hydro.default_atom_filename ,
                                   saxs_options_hydro.default_hybrid_filename ,
                                   saxs_options_hydro.default_saxs_filename 
                                   )
       &&
       saxs_options_hydro.compute_saxs_coeff_for_bead_models
       )
   {
      saxs_options_hydro.compute_saxs_coeff_for_bead_models = false;
   } else {
      setup_saxs_options();
      our_saxs_options = saxs_options_hydro;
   }

   // if ( 
   //     !saxs_util->load_mw_json( USglobal->config_list.system_dir + 
   //                               QDir::separator() + "etc" +
   //                               QDir::separator() + "mw.json" ) )
   // {
   //    editor_msg( "red", us_tr( "Warning: mw.json not read" ) );
   // }

   // if ( 
   //     !saxs_util->load_vdw_json( USglobal->config_list.system_dir + 
   //                                QDir::separator() + "etc" +
   //                                QDir::separator() + "vdw.json" ) )
   // {
   //    editor_msg( "red", us_tr( "Warning: vdw.json not read" ) );
   // }

   // if ( 
   //     !saxs_util->load_vcm_json( USglobal->config_list.system_dir + 
   //                                QDir::separator() + "etc" +
   //                                QDir::separator() + "vcm.json" ) )
   // {
   //    editor_msg( "red", us_tr( "Warning: vcm.json not read" ) );
   // }

   if ( saxs_options_hydro.wavelength == 0 )
   {
      saxs_options_hydro.start_q = 
         saxs_options_hydro.end_q = 
         saxs_options_hydro.delta_q = 0;
   }
   else
   {
      saxs_options_hydro.start_q = 4.0 * M_PI * 
         sin(saxs_options_hydro.start_angle * M_PI / 360.0) / 
         saxs_options_hydro.wavelength;
      saxs_options_hydro.start_q =  floor(saxs_options_hydro.start_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options_hydro.end_q = 4.0 * M_PI * 
         sin(saxs_options_hydro.end_angle * M_PI / 360.0) / 
         saxs_options_hydro.wavelength;
      saxs_options_hydro.end_q =  floor(saxs_options_hydro.end_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
      saxs_options_hydro.delta_q = 4.0 * M_PI * 
         sin(saxs_options_hydro.delta_angle * M_PI / 360.0) / 
         saxs_options_hydro.wavelength;
      saxs_options_hydro.delta_q =  floor(saxs_options_hydro.delta_q * SAXS_Q_ROUNDING + 0.5) / SAXS_Q_ROUNDING;
   }         

   // Read pdb_file name ////////////////////////////////////////////

  if ( !parameters.count( "pdbfile" ) )
    {
      results[ "errors" ] = "no pdbfile specified";
      return false;
    }
  
    QString file;
    
    QString files_try = parameters[ "pdbfile" ].replace('"', "").replace("\\/","/") ;
    parameters[ "_base_directory" ] = parameters[ "_base_directory" ].replace("\\/","/");

     //    results["check_file"] = files_try;


    QFileInfo fi( files_try );
    if ( !fi.exists() )
      {
	results[ "errors" ] += QString( " file %1 does not exist." ).arg( files_try );
      } else {
      if ( !fi.isReadable() )
	{
	  results[ "errors" ] += QString( " file %1 exists but is not readable." ).arg( files_try );
	} else {
	file = fi.filePath(); //files_try;
      }
    }
    
 
    
    bool parameters_set_first_model;
    if ( !parameters.count( "first_model" ) )
      {
	parameters_set_first_model = false;
      } else {
      parameters_set_first_model = true;
    } 
    
 
    screen_pdb( file, parameters_set_first_model );
 
      
    QString method = "";  
       
    if( parameters.count( "bead_model_list_box" ))
      {
	if (parameters[ "bead_model_list_box" ] == "atob")
	  {
	    reset_hydro_res(results_hydro);
	    method = QString();
	    calc_grid_pdb(parameters_set_first_model);
	    
	    if( parameters[ "atob_list_box" ] == "smi") 
	      {
		method = QString();
		calc_hydro();
		method = "SMI";
		results[ "progress_output" ] = "Hydro (SMI) calculation: 100%";
	      }
	    if( parameters[ "atob_list_box" ] == "zeno") 
	      {
		method = QString();
		calc_zeno_hydro();
		method = "Zeno";
		results[ "progress_output" ] = "Hydro (Zeno) calculation: 100%";
	      }
	    if( parameters[ "atob_list_box" ] == "none") 
	      {
		method = QString();
		method = "None";
		results[ "progress_output" ] = "AtoB model calculation: 100%";
	      }
	  }
	
	if (parameters[ "bead_model_list_box" ] == "somo")
	  {
	    reset_hydro_res(results_hydro);
	    method = QString();
	    calc_somo(false, parameters_set_first_model);
	    
	    if( parameters[ "somo_list_box" ] == "smi") 
	      {
		method = QString();
		calc_hydro();
		method = "SMI";
		results[ "progress_output" ] = "Hydro (SMI) calculation: 100%";
	      }
	    if( parameters[ "somo_list_box" ] == "zeno") 
	      {
		method = QString();
		calc_zeno_hydro();
		method = "Zeno";
		results[ "progress_output" ] = "Hydro (Zeno) calculation: 100%";
	      }
	    if( parameters[ "somo_list_box" ] == "none") 
	      {
		method = QString();
		method = "None";
		results[ "progress_output" ] = "SOMO model calculation: 100%";
	      }
	    
	  }

	if (parameters[ "bead_model_list_box" ] == "somo_o")
	  {
	    reset_hydro_res(results_hydro);
	    method = QString();
	    calc_somo_o(parameters_set_first_model);
	    
	    if( parameters[ "somo_o_list_box" ] == "zeno") 
	      {
		method = QString();
		calc_zeno_hydro();
		method = "Zeno";
	      }
	    if( parameters[ "somo_o_list_box" ] == "none") 
	      {
		method = QString();
		method = "None";
		results[ "progress_output" ] = "SOMO model calculation: 100%";
	      }
	  }
      }

 
    QString model_name = project + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" );

    QString model_name_file = parameters[ "_base_directory" ] + QDir::separator() + list_of_models[0]; // 1st model only
        
    QString hydro_name_file_try = model_name_file;
    
    QString hydro_name_file;

    if (method == "SMI")
      hydro_name_file = hydro_name_file_try.replace(".bead_model", ".hydro_res" );

    if (method == "Zeno")
      hydro_name_file = hydro_name_file_try.replace(".bead_model", ".zno" );

    // if (method == "None")
    //   hydro_name_file = "";

    QString log_name_file =  parameters[ "_base_directory" ] + QDir::separator() + "runlog.txt"; 
    
    
    QString bead_pdb_name_file = model_name_file;
    bead_pdb_name_file.replace(".bead_model", ".pdb");
    us_qdebug(bead_pdb_name_file);
    
    QString orig_pdb_name_file = parameters[ "_base_directory" ] + QDir::separator() + project + ".pdb";
    
   
    for (QStringList::iterator it = list_of_models.begin();
         it != list_of_models.end(); ++it) {           
        QString current = *it;               
	//us_qdebug(current);
    }

    results[ "progress1" ] = QString::number(1.0);

    results[ "_textarea" ] = "__reset__\\n" + accumulated_msgs;


    results[ "model_name_file" ] = model_name_file;
    results[ "log_name_file" ] = log_name_file;
    results[ "bead_pdb" ] = 
       "{\"file\":\"" + bead_pdb_name_file + "\",\"script\":\"" + pdb_jsmol_script( & bead_model ) + "\"}";

    //us_qdebug( results[ "bead_pdb" ] );

    if( parameters.count( "show_atomistic_model" ) ) 
       results[ "orig_model_pdb" ] = file; // orig_pdb_name_file;

    // results[ "model_name" ] = model_name;
    results[ "model_name" ] = results_hydro.name;
    results[ "method_used" ] = method; // QString("%1").arg(method);

     if (method != "None")
      {
	if ( fabs(results_hydro.total_beads_sd) <= 1e-100 )
	  {
	    results[ "total_beads" ] =  QString::number(results_hydro.total_beads);		
	  }
	else
	  {
	    results[ "total_beads" ] = (QString("").sprintf("%u (%4.2e)", 
							(int)(results_hydro.total_beads + .5),
							results_hydro.total_beads_sd));
	  }

	if ( fabs(results_hydro.used_beads_sd) <= 1e-100 )
	  {
	    results[ "used_beads" ] =  QString::number(results_hydro.used_beads);			
	  }
	else
	  {
	    results[ "used_beads" ] = (QString("").sprintf("%u (%4.2e)", 
                                                  (int)(results_hydro.used_beads + .5),
                                                  results_hydro.used_beads_sd));
	  }
	
	if (fabs(results_hydro.s20w_sd) <= 1e-100)
	  {
	    results[ "s20w" ] =  QString::number(results_hydro.s20w, 'e', 2) + " S";				    
	  }
	else
	  {
	    results[ "s20w" ] = (QString("").sprintf("%4.2e S (%4.2e)", results_hydro.s20w, results_hydro.s20w_sd));
	  }

	if (fabs(results_hydro.D20w_sd) <= 1e-100)
	  {
	    results[ "D20w" ] =  QString::number(results_hydro.D20w, 'e', 2) + " cm^2/sec";			   
	  }
	else
	  {
	    results[ "D20w" ] = (QString("").sprintf("%4.2e cm^2/sec (%4.2e)", results_hydro.D20w, results_hydro.D20w_sd));
	  }
	
	if (fabs(results_hydro.rs_sd) <= 1e-100)
	  {
	    results[ "rs" ] =  QString::number(results_hydro.rs, 'e', 2) + " nm";			
	  }
	else
	  {
	    results[ "rs" ] = (QString("").sprintf("%4.2e nm (%4.2e)", results_hydro.rs, results_hydro.rs_sd));
	  }
	
	if (fabs(results_hydro.ff0_sd) <= 1e-100)
	  {
	    results[ "ff0" ] =  QString::number(results_hydro.ff0, 'f', 2);			
	  }
	else
	  {
	    results[ "ff0" ] =  (QString("").sprintf("%3.2f nm (%3.2e)", results_hydro.ff0, results_hydro.ff0_sd));
	  }

	if (fabs(results_hydro.rg_sd) <= 1e-100)
	  {
	    results[ "rg" ] =  QString::number(results_hydro.rg, 'e', 2) + " nm";			
	  }
	else
	  {
	    results[ "rg" ] =  (QString("").sprintf("%4.2e nm (%4.2e)", results_hydro.rg, results_hydro.rg_sd));
	  }

	if ( method == "Zeno" ) {
	  results[ "tau" ] = "n/a" ; 
	} else {
	  if (fabs(results_hydro.tau_sd) <= 1e-100)
	    {
	      results[ "tau" ] =  QString::number(results_hydro.tau, 'e', 2) + " ns";			
	    } else {
	    results[ "tau" ] = (QString("").sprintf("%4.2e ns (%4.2e)", results_hydro.tau, results_hydro.tau_sd));
	  }
	}
	

	if (fabs(results_hydro.viscosity_sd) <= 1e-100)
	  {
	    results[ "viscosity" ] = QString::number(results_hydro.viscosity, 'e', 2) + " cm^3/g";		
	  }
	else
	  {
	    results[ "viscosity" ] = (QString("").sprintf("%4.2e cm^3/g (%4.2e)", results_hydro.viscosity, results_hydro.viscosity_sd));
	  }

	results[ "mass" ] =  QString::number(results_hydro.mass, 'e', 4) + " Da";			
	results[ "vbar" ] =  QString::number(results_hydro.vbar) + " cm^3/g";			
	results[ "hydro_name_file" ] = hydro_name_file;
	
      }
    else 
      {
	results[ "total_beads" ] = ""; 
	results[ "mass" ] =  "";
	results[ "vbar" ] =  "";
	results[ "rg" ] =  "";
	results[ "hydro_name_file" ] = "";
	results[ "used_beads" ] = ""; 
	results[ "s20w" ] =  "";
	results[ "D20w" ] =  "";
	results[ "viscosity" ] = "";
	results[ "ff0" ] =  "";
	results[ "rs" ] =  "";
	results[ "tau" ] = "";
      }
   
       
    // QString method = QString("%1").arg(((parameters.count( "atob" )) ? "AtoB model" : ( (parameters.count( "zeno" )) ? "Zeno model" : "AtoB model")) );

    // QString output = "Hydrodynamic Calculations";

    // results[ "hydro_output" ] += output + " (" + method + "):" + "<br>"
    //   "<br><tr><td> total_beads  </tr></td>"  + QString::number(results_hydro.total_beads) + 
    //   "<br><tr><td> used_beads  </tr></td>"  + QString::number(results_hydro.used_beads)  +
    //   "<br><tr><td> mass  </tr></td>"  + QString::number(results_hydro.mass) + 
    //   "<br><tr><td> Sedimentaiton   </tr></td>"  + QString::number(results_hydro.s20w) + 
    //   "<br><tr><td> Diffusion  </tr></td>"  + QString::number(results_hydro.D20w) +
    //   "<br><tr><td> viscosity  </tr></td>"  + QString::number(results_hydro.viscosity) + 
    //   "<br><tr><td> ff0  </tr></td>"  + QString::number(results_hydro.ff0) + "<br>";
 
      // + "mass" + QString::number(results_hydro.mass);
        
    // calc_zeno_hydro();

    // cout << "FINISH! " << endl;

     return true;
    
}


void US_Saxs_Util::read_residue_file()
{
   QString str1, str2;
   unsigned int numatoms, numbeads, /* i, */ j, positioner;
   QFile f(residue_filename);
   int error_count = 0;
   int line_count = 1;
   QString error_msg = us_tr("Residue file errors:\n");
   QString error_text = us_tr("Residue file errors:\n");
 
  // if ( advanced_config.debug_1 )
  //  {
  //     cout << "residue file name: " << residue_filename << endl;
  //  }

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

   // i=1;
   if (f.open(QIODevice::ReadOnly|QIODevice::Text))
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
            // else
            // {
            //    QMessageBox::warning(this, us_tr("UltraScan Warning"),
            //                         us_tr("Please note:\n\nThere was an error reading\nthe selected Residue File!\n\nAtom "
            //                            + new_atom.name + " cannot be read and will be deleted from List."),
            //                         QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
            // }
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
            // if ( advanced_config.debug_1 )
            // {
            //    printf("residue name %s loading bead %d placing method %d\n",
            //           new_residue.name.toLatin1().data(),
            //           j, new_bead.placing_method); fflush(stdout);
            // }
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
            // if ( advanced_config.debug_1 )
            // {
            //    printf("residue name %s unique name %s atom size %u alt size %u pos %u\n"
            //           ,new_residue.name.toLatin1().data()
            //           ,new_residue.unique_name.toLatin1().data()
            //           ,(unsigned int) new_residue.r_atom.size()
            //           ,(unsigned int) alt_atoms.size()
            //           ,(unsigned int) residue_list.size()
            //           ); fflush(stdout);
            // }
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
     // US_Static::us_message(us_tr("ERRORS:"), error_msg);
     //if (editor)
     //{
     QString error_text_udp = error_text;
     error_text_udp.replace("\n","\\n");
     us_log->log(error_text);
     if ( us_udp_msg )
       {
	 map < QString, QString > msging;
	 msging[ "_textarea" ] = QString(error_text_udp + "\\n");
	 us_udp_msg->send_json( msging );
	 //sleep(1);
       }   
     
     //	}
   }

   save_residue_list = residue_list;
   save_residue_list_no_pbr = residue_list_no_pbr;
   save_multi_residue_map = multi_residue_map;

   // if ( misc.export_msroll )
   // {
   //    QString dir = somo_dir + QDir::separator() + "tmp" + QDir::separator();
         
   //    QFile f_radii( dir + "msroll_radii.txt" );
   //    if ( !f_radii.open( IO_WriteOnly ) )
   //    {
   //       editor_msg( "red", QString( us_tr( "Error: can not create MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
   //    } else {
   //       QTextStream ts( &f_radii );
   //       for ( unsigned int i = 0; i < (unsigned int) msroll_radii.size(); i++ )
   //       {
   //          ts << msroll_radii[ i ];
   //       }
   //       f_radii.close();
   //       editor_msg( "blue", QString( us_tr( "Notice: created MSROLL radii file: %1" ) ).arg( f_radii.fileName() ) );
   //    }

   //    QFile f_names( dir + "msroll_names.txt" );
   //    if ( !f_names.open( IO_WriteOnly ) )
   //    {
   //       editor_msg( "red", QString( us_tr( "Error: can not create MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
   //    } else {
   //       QTextStream ts( &f_names );
   //       for ( unsigned int i = 0; i < (unsigned int) msroll_names.size(); i++ )
   //       {
   //          ts << msroll_names[ i ];
   //       }
   //       f_names.close();
   //       editor_msg( "blue", QString( us_tr( "Notice: created MSROLL names file: %1" ) ).arg( f_names.fileName() ) );
   //    }
   // }
}


bool US_Saxs_Util::screen_pdb(QString filename, bool parameters_set_first_model ) //, bool display_pdb, bool skipclearissue )
{
   // if ( !skipclearissue ) {
  issue_info.clear( );
   // }
   // clear_pdb_info( "screen_pdb" );
   // qDebug() << "screen_pdb:: 0";

   // if ( misc.restore_pb_rule ) {
   //    // us_qdebug( "screen_pdb() restoring pb rule" );
   //    if ( misc_widget ) {
   //       misc_window->close();
   //       delete misc_window;
   //       misc_widget = false;
   //    }
   //    misc.pb_rule_on      = true;
   //    misc.restore_pb_rule = false;
   // }

   // cout << QString( "screen pdb display is %1\n" ).arg( display_pdb ? "true" : "false" );
   
  pdb_file = filename;

   // if ( QFileInfo(filename).fileName().contains(" ") )
   // {
   //    printError(us_tr("Filenames containing spaces are not currently supported.\n"
   //                  "Please rename the file to remove spaces."));
   //    return false;
   // }

   // if ( is_dammin_dammif(filename) )
   // {
   //    return screen_bead_model(filename);
   // }

   // options_log = "";
   // last_abb_msgs = "";
   bead_model_from_file = false;
   int errors_found = 0;
   // le_pdb_file_save_text = QDir::toNativeSeparators( filename );
   // le_pdb_file->setText( QDir::toNativeSeparators( filename ) );

   // bead_model_suffix = "";
   // le_bead_model_suffix->setText( bead_model_suffix );

// #if defined(START_RASMOL)
//    if ( display_pdb )
//    {
//       QStringList argument;
// #if !defined(WIN32) && !defined(MAC)
//       // maybe we should make this a user defined terminal window?
//       argument.append("xterm");
//       argument.append("-e");
// #endif
// #if defined(BIN64)
//       argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
// #else
//       argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
// #endif
//       argument.append(QFileInfo(filename).fileName());
//       rasmol->setWorkingDirectory(QFileInfo(filename).path());
//       rasmol->setArguments(argument);
//       if (advanced_config.auto_view_pdb &&
//           !rasmol->start())
//       {
//          US_Static::us_message(us_tr("Please note:"), us_tr("There was a problem starting RASMOL\n"
//                                                      "Please check to make sure RASMOL is properly installed..."));
//       }
//    }
// #endif

   // qDebug() << "screen_pdb:: 1";
   QFileInfo fi(filename);
   project = fi.baseName();

   new_residues.clear( );

   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
   residue_list = save_residue_list_no_pbr;
   }

   bool use_parameters_set_first_model =  parameters_set_first_model;
 
   multi_residue_map = save_multi_residue_map;
   if ( read_pdb_hydro(filename, use_parameters_set_first_model) )
   {
      return false;
   }

   // qDebug() << "Current Model: " << current_model << Qt::endl;
 
   // qDebug() << "screen_pdb:: 2";
   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++) {

      // qDebug() << "screen_pdb:: 3";
      multi_residue_map = save_multi_residue_map;
      
      us_log->log(QString("Checking the pdb structure for model %1\n").arg(  i+1  ) );
  
      if ( us_udp_msg )
 	{
       	  map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("Checking the pdb structure for model %1\\n").arg(  i+1  );
	  us_udp_msg->send_json( msging );
       	  //sleep(2);
	}
      accumulated_msgs +=  QString("Checking the pdb structure for model %1\\n").arg(  i+1  );
      

      // qDebug() << "screen_pdb:: 4";
      if ( check_for_missing_atoms_hydro( &error_string, &model_vector[i], parameters_set_first_model ) ) {
         // qDebug() << "screen_pdb::check_for_missing_atoms failed";

      // if ( !(check_for_missing_atoms( &model_vector[i]) ) )
      // 	{
	    errors_found++;
	    us_log->log(QString("Encountered errors with your PDB structure for model %1:\n").
			arg(  i  ) + error_string);
	    

	    QString error_string_udp = error_string;
	    error_string_udp.replace("\n","\\n");
	    if ( us_udp_msg )
	      {
		map < QString, QString > msging;
		msging[ "_textarea" ] = QString(QString("Encountered errors with your PDB structure for model %1:\\n").
						arg(  i  ) + error_string_udp);
		us_udp_msg->send_json( msging );
		//sleep(2);
	      }
	    accumulated_msgs += QString(QString("Encountered errors with your PDB structure for model %1:\\n").arg(  i  ) + error_string_udp);
	    
	    // printError(QString("Encountered errors with your PDB structure for model %1:\n").
	    //            arg( model_name( i ) ) + "please check the text window");
	    
	  }

	  reset_chain_residues( &model_vector[i] );
	  
 
      //cout << "Check: model#: " << i << " " << model_vector[i].vbar << endl;
   }

   // qDebug() << "screen_pdb::after check_for_missing_atoms";

   

   if ( errors_found )
     {
       calc_vol_for_saxs();
     } else {
     calc_mw_hydro();
   }
   //cout << "Current_model!: " << current_model << endl;

   // qDebug() << "screen_pdb:: 5";
   model_vector_as_loaded = model_vector;
   // set_pdb_info( "screen_pdb" );
   if ( !model_vector.size() ||
        !model_vector[0].molecule.size() )
   {
     //QColor save_color = editor->textColor();
     //editor->setTextColor("red");
     us_log->log(us_tr("ERROR : PDB file contains no atoms!"));
     
     if ( us_udp_msg )
       {
	 map < QString, QString > msging;
	 msging[ "_textarea" ] = us_tr("ERROR : PDB file contains no atoms!");
	 us_udp_msg->send_json( msging );
	 //sleep(1);
       }
     accumulated_msgs += us_tr("ERROR : PDB file contains no atoms!");
     
     //editor->setTextColor(save_color);
     errors_found++;
   }

   // qDebug() << "screen_pdb:: 6";

   us_log->log(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok\n"));
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("\\nLoaded pdb file : %1\\n").arg(errors_found ? "ERRORS PRESENT" : "ok\\n");
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
   accumulated_msgs += QString("\\nLoaded pdb file : %1\\n").arg(errors_found ? "ERRORS PRESENT" : "ok\\n");

   bead_models.clear( );
   somo_processed.clear( );
   update_vbar();
   // cout << "Check: model#: " << " 0 " << model_vector[0].vbar << endl;
   // cout << "Check: model#: "  << " 1 " << model_vector[1].vbar << endl;
   

   // if (results_widget)
   // {
   //    results_window->close();
   //    delete results_window;
   //    results_widget = false;
   // }
   // bead_model_prefix = "";
   // pb_somo->setEnabled(errors_found ? false : true);
   // pb_somo_o->setEnabled(errors_found ? false : true);
   // pb_grid_pdb->setEnabled(errors_found ? false : true);
   // pb_grid->setEnabled(false);
   // pb_show_hydro_results->setEnabled(false);
   // pb_calc_hydro.setEnabled(false);
   // pb_calc_zeno->setEnabled(false);
   // pb_bead_saxs->setEnabled(false);
   // pb_rescale_bead_model->setEnabled(false);
   // pb_visualize->setEnabled(false);
   // pb_equi_grid_bead_model->setEnabled(false);
   // le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
   // if ( lb_model->count() == 1 )
   // {
   //    select_model(0);
   // }
   return errors_found ? false : true;
}   


int US_Saxs_Util::check_for_missing_atoms_hydro(QString *error_string, PDB_model *model, bool parameters_set_first_model)
{
   // go through molecules, build vector of residues
   // expand vector of residues to atom list from residue file
   // compare expanded list of residues to model ... list missing atoms missing
  residue_errors.clear( );
   int errors_found = 0;
   get_atom_map(model);

   bool use_parameters_set_first_model = parameters_set_first_model;

   // RELOAD PDB ??? Qt slots ///////////////////////////////////////////////////

     if ( misc.pb_rule_on &&
          (broken_chain_head.size() || broken_chain_end.size()) )
     {
        misc.pb_rule_on = false;
        misc.restore_pb_rule = true;
        // us_qdebug( "check_for_missing_atoms() setting restore_pb_rule" );
        us_log->log("Broken chain turns off Peptide Bond Rule.\n");
	if ( us_udp_msg )
	  {
	    map < QString, QString > msging;
	    msging[ "_textarea" ] = "Broken chain turns off Peptide Bond Rule.\\n";
	    us_udp_msg->send_json( msging );
       //sleep(1);
	  }
	accumulated_msgs += "Broken chain turns off Peptide Bond Rule.\\n";


        // if ( misc_widget ) {
        //    misc_window->close();
        //    delete misc_window;
        //    misc_widget = false;
        // }
        // if ( advanced_config.debug_1 )
        // {
        //    printf("Broken chain turns off Peptide Bond Rule.\n");
        // }
        //      display_default_differences();
        reload_pdb(use_parameters_set_first_model);
        get_atom_map(model);
     }

   int failure_errors = 0;
   bead_exceptions.clear( );

   // residue types are for automatic build builder to determine
   // if we have a protein, so that a special 'temporary' residue can
   // be created

   vector < map < unsigned int, unsigned int > > residue_types;
   vector < unsigned int > last_residue_type;

   residue_types.resize(model->molecule.size());
   last_residue_type.resize(model->molecule.size());

   build_molecule_maps(model);
   QString abb_msgs = "";
   QString abb_msgs_udp = "";

   // keep track of errors shown
   map < QString, bool > error_shown;

 
  for (unsigned int j = 0; j < model->molecule.size(); j++)
   {
     
     QString lastOrgResSeq = "";
     QString lastOrgResName = "";
     QString lastOrgChainID = "";

      QString lastResSeq = "";
      int lastResPos = -1;
      QString lastChainID = " ";
      bool spec_N1 = false;
      QString last_count_idx;
      unsigned int residues_found = 0;
      unsigned int N1s_placed = 0;
      for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      {
         PDB_atom *this_atom = &(model->molecule[j].atom[k]);
         this_atom->active = false;
         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         if (!bead_exceptions[count_idx])
         {
            bead_exceptions[count_idx] = 1;
         }

         // find residue in residues
         // if ( advanced_config.debug_1 )
         // {
         //    printf("check_for_missing_atoms search name %s resName %s\n",
         //           this_atom->name.toLatin1().data(),
         //           this_atom->resName.toLatin1().data());
         // }
         int respos = -1;
         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         for ( unsigned int m = 0; m < residue_list.size(); m++ )
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               // if ( advanced_config.debug_1 )
               // {
	      //cout << 
	      // QString("(check for missing atoms) skipping %1 %2\n").arg(res_idx).arg(m) << endl;
               // }
               continue;
            }
            if ((residue_list[m].name == this_atom->resName &&
                 // (int)residue_list[m].r_atom.size() ==
                 // atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" || 
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc.pb_rule_on)) ||
                
                // if pb_rule is off, final OXT before P needs to use OXT-P

                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc.pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc.pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&
                 misc.pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               residue_types[j][residue_list[m].type]++;
               last_residue_type[j] = residue_list[m].type;
	       //cout << "Inside check:" << endl;
	       this_atom->p_residue = &(residue_list[m]);
	       cout << k  << " " << this_atom->p_residue->unique_name << endl;
	       // cout << (int)residue_list[m].r_atom.size() << " " <<  atom_counts[count_idx] - has_OXT[count_idx]  << endl;
               // if ( advanced_config.debug_1 )
               // {
                  
               //    printf("residue match %d resName %s \n", m, residue_list[m].name.toLatin1().data());
               //    printf("resname %s respos %d mappos %d mapsize %u\n"
               //           ,this_atom->resName.toLatin1().data()
               //           ,m
               //           ,multi_residue_map[this_atom->resName][0]
               //           ,(unsigned int)multi_residue_map[this_atom->resName].size()
               //           );
               // }
               if (lastResSeq != this_atom->resSeq)
	       {
		 //cout << "Inside missing atom 1 " << endl;
                  // new residue
                  // printf("new residue %s\n", this_atom->resSeq.toLatin1().data());
                  residues_found++;
                  if (lastResPos != -1)
                  {
		    //cout << "Inside missing atom 1a " << endl;
                     // check for false entries in last residue and warn about them
                     for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
                     {
		       //cout << "Inside missing atom 1aa" << endl;
                        if (spec_N1 &&
                            misc.pb_rule_on &&
                            residue_list[lastResPos].r_atom[l].name == "N") {
                           residue_list[lastResPos].r_atom[l].tmp_flag = true;
                           spec_N1 = false;
			   //cout << "Inside missing atom 1b" << endl;
                        }

                        if (!residue_list[lastResPos].r_atom[l].tmp_flag)
                        {

			  // cout << "Inside missing atoms 2" << endl;
                           // missing atoms
                           errors_found++;
                           if (pdb_parse.missing_atoms == 0)
                           {
                              failure_errors++;
                           }
                           if (pdb_parse.missing_atoms == 1)
                           {
                              bead_exceptions[last_count_idx] = 2;
                           }
                           if (pdb_parse.missing_atoms == 2)
                           {
                              if ( bead_exceptions[last_count_idx] == 1 ) 
                              {
                                 bead_exceptions[last_count_idx] = 4;
                              }
                           }
                           if (!error_shown[last_count_idx]) 
                           {
                              QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                                 .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                                 .arg(j + 1)
                                 .arg(residue_list[lastResPos].name)
                                 .arg(lastResSeq);
                              QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
			      residue_errors[ lastOrgChainID + "~" + 
                                              lastOrgResName
                                              + " " +
                                              lastOrgResSeq ] = true;

                              switch (molecules_residue_errors[idx].size())
                              {
                              case 0: 
                                 this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                                 break;
                              case 1:
                                 this_error += molecules_residue_errors[idx][0] + "\n";
                                 break;
                              default :
                                 {
                                    this_error += "\n";
                                    for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                                    {
                                       // if ( advanced_config.debug_1 )
                                       // {
                                       //    this_error += " d1";
                                       // }
                                       this_error += QString("    Residue file entry %1: %2\n").
                                          arg(t+1).arg(molecules_residue_errors[idx][t]);
                                    }
                                 }
                                 break;
                              }
                              error_string->append(this_error);
                              //cout << QString("dbg 1: idx <%1> msg <%2>\n").arg(last_count_idx).arg(this_error);
                              error_shown[last_count_idx] = true;
                           }
                           // error_string->
                           // append(QString("").
                           //     sprintf("Missing atom: chain %s molecule %d atom %s residue %s %s\n",
                           //        lastChainID.toLatin1().data(),
                           //        j + 1,
                           //        residue_list[lastResPos].r_atom[l].name.toLatin1().data(),
                           //        lastResSeq.toLatin1().data(),
                           //        residue_list[lastResPos].name.toLatin1().data()));
                        }
			//cout << "not inside" << endl;
                     }
                  }

                  // reset residue list
                  // printf("reset residue list for residue_list[%d]\n", m);
                  for (unsigned int l = 0; l < residue_list[m].r_atom.size(); l++)
                  {
                     residue_list[m].r_atom[l].tmp_flag = false;
                  }
                  lastResSeq = this_atom->resSeq;
                  lastChainID = this_atom->chainID;
                  lastResPos = (int) m;
                  last_count_idx = count_idx;
		  
		  lastOrgResSeq  = this_atom->orgResSeq;
                  lastOrgChainID = this_atom->orgChainID;
                  lastOrgResName = this_atom->orgResName;
	       }

               if (residue_list[m].name == "N1")
               {
                  lastResSeq = "";
                  lastResPos = -1;
                  spec_N1 = true;
                  N1s_placed++;
                  residues_found--;
               }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               QString msg_tag;
               bool do_error_msg = true;
               if (pdb_parse.missing_residues == 0 &&
                   pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
                  msg_tag = "Missing residue or atom";
               } 
               else 
               {
                  // ok, we have three cases here:
                  // 1. residue does exist & residue/atom doesn't
                  //    1.1 skip missing atoms controls
                  // 2. residue does exist & residue/atom does
                  //    2.1 there must be a missing atom since count doesn't match so atom controls
                  // 3. residue does not exist
                  //    3.1 skip missing residue control
                  // ---------------------
                  //
                  // note: we're just checking the 1st of multiple possibilities
                  puts("cases---");
                  if (multi_residue_map[this_atom->resName].size()) 
                  {
                     // residue exists, does residue/atom?
                     QString idx = QString("%1|%2|%3")
                        .arg(this_atom->resName)
                        .arg(this_atom->name)
                        .arg(0); 
                     printf("cases residue found: idx %s\n", idx.toLatin1().data());
                     if (valid_atom_map[idx].size()) 
                     {
                        puts("case 2.1");
                        msg_tag = "Missing or extra atom in residue";
                     } 
                     else
                     {
                        // atom does not exist, skip missing atoms controls
                        puts("case 1.1");
                        msg_tag = "Missing atom";
                     }
              
		     /* TURNED OFF *******************************************/
                     // switch( issue_missing_atom_hydro() ) {
                     // case ISSUE_RESPONSE_STOP :
                     //    failure_errors++;
                     //    break;
                        
                     // case ISSUE_RESPONSE_MA_SKIP :
                     //    bead_exceptions[count_idx] = 2;
                     //    break;

                     // case ISSUE_RESPONSE_MA_MODEL :
                     //    if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 ) {
                     //       bead_exceptions[count_idx] = 3;
                     //    } else {
                     //       if ( bead_exceptions[count_idx] == 1 ) 
                     //       {
                     //          bead_exceptions[count_idx] = 4;
                     //       }
                     //    }
                     //    break;
                     // }
		     /*************************************************/

                     if (pdb_parse.missing_atoms == 0)
                     {
                        failure_errors++;
                     }
                     if (pdb_parse.missing_atoms == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (pdb_parse.missing_atoms == 2)
                     {
                        if ( molecules_residue_min_missing[QString("%1|%2").arg(j).arg(this_atom->resSeq)] == -1 )
                        {
                           // fall back to non-coded residue 
                           switch ( pdb_parse.missing_residues )
                           {
                           case 0 :
                              failure_errors++;
                              break;
                           case 1 :
                              bead_exceptions[count_idx] = 2;
                              break;
                           case 2 :
                              bead_exceptions[count_idx] = 3;
                              break;
                           default :
                              failure_errors++;
                              // US_Static::us_message(us_tr("Internal error:"), 
                              //                      us_tr("Unexpected pdb_parse.missing residues type ") +
                              //                      QString("%1\n").arg(pdb_parse.missing_residues));
                              exit(-1);
                              break;
                           }
                        } 
                        else
                        {
                           if ( bead_exceptions[count_idx] == 1 ) 
                           {
			     bead_exceptions[count_idx] = 4;
			     // bead_exceptions[count_idx] = 3;
                           }
                        }
                     }
                  } 
                  else 
                  {
                     // residue does not exist, skip missing residue controls
                   
		    /* TURNED OFF **************************************/
		    // puts("case 3.1");
                     // msg_tag = "Non-coded residue";
                     // switch( issue_non_coded_hydro() ) {
                     // case ISSUE_RESPONSE_STOP :
                     //    failure_errors++;
                     //    break;
                        
                     // case ISSUE_RESPONSE_NC_SKIP :
                     //    bead_exceptions[count_idx] = 2;
                     //    break;

                     // case ISSUE_RESPONSE_NC_REPLACE :
                     //    bead_exceptions[count_idx] = 3;
                     //    break;
                        
                     // }
		    /****************************************************/

                     if (pdb_parse.missing_residues == 0)
                     {
                        failure_errors++;
                     }
                     if (pdb_parse.missing_residues == 1)
                     {
                        bead_exceptions[count_idx] = 2;
                     }
                     if (pdb_parse.missing_residues == 2)
                     {
                        bead_exceptions[count_idx] = 3;
                     }
                  }
               }
	       
               if (do_error_msg) {
                  errors_found++;
                  if (!error_shown[count_idx]) 
                  {
                     QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                        .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                        .arg(j + 1)
                        .arg(this_atom->resName)
                        .arg(this_atom->resSeq);
                     QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                     switch (molecules_residue_errors[idx].size())
                     {
                     case 0: 
                        this_error += msg_tag;
                        break;
                     case 1:
                        this_error += molecules_residue_errors[idx][0] + "\n";
                        break;
                     default :
                        {
                           this_error += "\n";
                           for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                           {
                              // if ( advanced_config.debug_1 )
                              // {
                              //    this_error += " d2";
                              // }
                              this_error += QString("    Residue file entry %1: %2\n").
                                 arg(t+1).arg(molecules_residue_errors[idx][t]);
                           }
                        }
                        break;
                     }
                     error_string->append(this_error);
                     //cout << QString("dbg 2: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                     error_shown[count_idx] = true;
                  }
        
                  // error_string->append(QString("").sprintf("%s: chain %s molecule %d atom %s residue %s %s\n",
                  //               msg_tag.toLatin1().data(),
                  //               this_atom->chainID.toLatin1().data(),
                  //               j + 1,
                  //               this_atom->name.toLatin1().data(),
                  //               this_atom->resSeq.toLatin1().data(),
                  //               this_atom->resName.toLatin1().data()
                  //               ));
               }
            }
         }
         else 
         {
            // find atom in residues atoms
            int atompos = -1;
            // if ( advanced_config.debug_1 )
            // {
            //    printf("now find atom in residues atoms\n");
            // }
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if (residue_list[respos].r_atom[m].name == this_atom->name ||
                   (
                    this_atom->name == "N" &&
                    misc.pb_rule_on &&
                    !k &&
                    residue_list[respos].r_atom[m].name == "N1" &&
                    !broken_chain_head.count(QString("%1|%2")
                                             .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                    this_atom->resName != "PRO"
                    )
                   )
               {
                  this_atom->active = true;
                  residue_list[respos].r_atom[m].tmp_flag = true;
                  atompos = (int) m;
                  break;
               }
            }
            if (atompos == -1)
            {
               errors_found++;
               // valid residue, but non-coded atom
               switch ( pdb_parse.missing_atoms )
               {
               case 0 :
                  failure_errors++;
                  break;
               case 1 :
                  bead_exceptions[count_idx] = 2;
                  break;
               case 2 :
                  // resort to missing_residue controls
                  if ( bead_exceptions[count_idx] == 1 ) 
                  {
                     switch ( pdb_parse.missing_residues )
                     {
                     case 0 :
                        failure_errors++;
                        break;
                     case 1 :
                        bead_exceptions[count_idx] = 2;
                        break;
                     case 2 :
                        bead_exceptions[count_idx] = 3;
                        break;
                     default :
                        failure_errors++;
                        // US_Static::us_message(us_tr("Internal error:"), 
                        //                      us_tr("Unexpected pdb_parse.missing residues type ") +
                        //                      QString("%1\n").arg(pdb_parse.missing_residues));
                        exit(-1);
                        break;
                     }
                  } 
                  break;
               default :
                  failure_errors++;
                  // US_Static::us_message(us_tr("Internal error:"), 
                  //                      us_tr("Unexpected pdb_parse.missing residues type ") +
                  //                      QString("%1\n").arg(pdb_parse.missing_residues));
                  exit(-1);
                  break;
               }

               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                     .arg(j + 1)
                     .arg(this_atom->resName)
                     .arg(this_atom->resSeq);
                  QString idx = QString("%1|%2").arg(j).arg(this_atom->resSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Unknown atom %1.\n").arg(this_atom->name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           // if ( advanced_config.debug_1 )
                           // {
                           //    this_error += " d3";
                           // }
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  cout << QString("dbg 3: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
        

               // error_string->append(QString("").sprintf("unknown atom chain %s molecule %d atom %s residue %s %s\n",
               //            this_atom->chainID.toLatin1().data(),
               //            j + 1,
               //            this_atom->name.toLatin1().data(),
               //            this_atom->resSeq.toLatin1().data(),
               //            this_atom->resName.toLatin1().data()
               //            ));
            }
         }
      } // end for ( unsigned int k = 0; k < model->molecule[j].atom.size(); k++ )
      if (lastResPos != -1)
      {
         // check for false entries in last residue and warn about them
         // printf("check for false entries in last residue %d, residues_found %d, N1s_placed %d\n", lastResPos, residues_found, N1s_placed);
         for (unsigned int l = 0; l < residue_list[lastResPos].r_atom.size(); l++)
         {
            if ( !residue_list[lastResPos].r_atom[l].tmp_flag &&
                 !(misc.pb_rule_on && residues_found == 1 && N1s_placed == 1) )
            {

	      residue_errors[ lastOrgChainID + "~" + 
                               lastOrgResName
                               + " " +
                               lastOrgResSeq ] = true;
               errors_found++;
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(residue_list[lastResPos].r_atom[l].name)
                  .arg(lastResSeq);
               // if ( advanced_config.debug_1 )
               // {
               //    printf("dbg x pdb_parse.missing_atoms %d bead_exceptions[%s] %d residue_list[%d].r_atom[%d].tmp_flag %d\n"
               //           , pdb_parse.missing_atoms
               //           , count_idx.toLatin1().data()
               //           , bead_exceptions[count_idx]
               //           , lastResPos
               //           , l
               //           , residue_list[lastResPos].r_atom[l].tmp_flag
               //           );
               // }
                      
               if (pdb_parse.missing_atoms == 0)
               {
                  failure_errors++;
               } 
               else 
               {
                  if (pdb_parse.missing_atoms == 1)
                  {
                     bead_exceptions[count_idx] = 2;
                  }
                  if (pdb_parse.missing_atoms == 2)
                  {
                     if ( bead_exceptions[count_idx] == 1 ) 
                     {
                        bead_exceptions[count_idx] = 4;
                     }
                  }
               }
               if (!error_shown[count_idx]) 
               {
                  QString this_error = QString("%1Molecule %2 Residue %3 %4: ")
                     .arg(lastChainID == " " ? "" : ("Chain " + lastChainID + " "))
                     .arg(j + 1)
                     .arg(residue_list[lastResPos].name)
                     .arg(lastResSeq);
                  QString idx = QString("%1|%2").arg(j).arg(lastResSeq);
                  switch (molecules_residue_errors[idx].size())
                  {
                  case 0: 
                     this_error += QString("Missing atom %1.\n").arg(residue_list[lastResPos].r_atom[l].name);
                     break;
                  case 1:
                     this_error += molecules_residue_errors[idx][0] + "\n";
                     break;
                  default :
                     {
                        this_error += "\n";
                        for (unsigned int t = 0; t < molecules_residue_errors[idx].size(); t++)
                        {
                           // if ( advanced_config.debug_1 )
                           // {
                           //    this_error += " d4";
                           // }
                           this_error += QString("    Residue file entry %1: %2\n").
                              arg(t+1).arg(molecules_residue_errors[idx][t]);
                        }
                     }
                     break;
                  }
                  error_string->append(this_error);
                  //cout << QString("dbg 4: idx <%1> msg <%2>\n").arg(count_idx).arg(this_error);
                  error_shown[count_idx] = true;
               }
               // error_string->
               //  append(QString("").
               // sprintf("missing atom chain %s molecule %d atom %s residue %s %s\n",
               //    lastChainID.toLatin1().data(),
               //    j + 1,
               //    residue_list[lastResPos].r_atom[l].name.toLatin1().data(),
               //    lastResSeq.toLatin1().data(),
               //    residue_list[lastResPos].name.toLatin1().data()));
            }
         }
      }
   } // j


   if (error_string->length())
   {
     // cout << "Rectifying: " << endl;
     // us_qdebug(*error_string);
     
      // to remove from unknown those that have 2 beads and a correct main chain
      vector < QString > unknown_aa_residues_to_check; 
      if (failure_errors > 0) 
      {
         return errors_found;
      }
      else 
      {

	residue new_residue;
	atom new_atom;
	bead new_bead;

	// cout << "Encountered the following warnings with your PDB structure:\n" + *error_string << endl;
	us_log->log("Encountered the following warnings with your PDB structure:\n" + *error_string);
	QString error_string_udp = *error_string;
	error_string_udp.replace("\n","\\n");
	if ( us_udp_msg )
	  {
	    map < QString, QString > msging;
	    msging[ "_textarea" ] = "Encountered the following warnings with your PDB structure:\\n" + error_string_udp;
	    us_udp_msg->send_json( msging );
	    //sleep(1);
	  }
	accumulated_msgs += "Encountered the following warnings with your PDB structure:\\n" + error_string_udp;
	

         *error_string = "";
         // repair model...
         PDB_model org_model = *model;
         model->molecule.clear( );
         // we may need to redo the residues also
         model->residue.clear( );
         //printf("vbar before: %g\n", model->vbar);
         QString new_residue_name = "";
         map < QString, bool > abb_msg_done;  // keep 1 message for the residue
         for (unsigned int j = 0; j < org_model.molecule.size(); j++)
         {
            PDB_chain tmp_chain;
            QString lastResSeq = "";
            QString lastResName = "";

	    QString lastOrgResSeq = "";
            QString lastOrgResName = "";
            QString lastOrgChainID = "";

            bool auto_bb_aa = false;             // are we doing special amino acid handling?
            map < QString, int > aa_main_chain;  // to make sure we have a good main chain
            int current_bead_assignment = 0;
            bool one_bead = false;               // fall back to one bead for missing atoms
            if (last_residue_type[j] == 0 &&      
                residue_types[j].size() == 1) 
            {
               // only amino acids, so we can create two beads
               // if ( advanced_config.debug_2  )
               // {
               //    puts("auto_bb_amino_acids");
               // }
               auto_bb_aa = true;
            }
  
	    
            for (unsigned int k = 0; k < org_model.molecule[j].atom.size(); k++)
            {
               QString count_idx =
                  QString("%1|%2|%3")
                  .arg(j)
                  .arg(org_model.molecule[j].atom[k].resName)
                  .arg(org_model.molecule[j].atom[k].resSeq);

	       cout <<  count_idx << " "  <<  bead_exceptions[count_idx] << endl;

               switch (bead_exceptions[count_idx])
               {
               case 1:
                  tmp_chain.atom.push_back(org_model.molecule[j].atom[k]);
                  if (org_model.molecule[j].atom[k].resSeq != lastResSeq) 
                  {
                     lastResSeq = org_model.molecule[j].atom[k].resSeq;
                     lastResName = org_model.molecule[j].atom[k].resName;
                     model->residue.push_back(residue_list[multi_residue_map[lastResName][0]]);

		     lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                     lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                     lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                  }
                  break;
               case 2:
               case 5: // extra or non-coded atom
                  {
                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     if ( !abb_msg_done[count_idx] ) 
                     {
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Skipped\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq);
			
			abb_msgs_udp += QString("ABB: %1Molecule %2 Residue %3 %4 Skipped\\n")
			  .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq);
 
			
                        abb_msg_done[count_idx] = true;
                     }
                     //printf("removing molecule %u atom %u from model\n", 
                     //       j, k);
                  }
                  break;
               case 3:
                  {
                     // create the temporary residue
                     // is this a new residue

                     PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString new_residue_idx = this_atom->resName;  // we could add atom_count to the idx for counting by unique atom counts...
                     // if ( advanced_config.debug_2  )
                     // {
                     //    printf("1.0 <%s> residue types %u, last_type %u\n", 
                     //           new_residue_idx.toLatin1().data(),
                     //           (unsigned int)residue_types[j].size(),
                     //           last_residue_type[j]
                     //           );
                     // }           
                     if (this_atom->resSeq != lastResSeq) 
                     {

                        current_bead_assignment = 0;
                        if (auto_bb_aa) 
                        {
                           // reset the main chain counts
                           aa_main_chain.clear( );
                           aa_main_chain["N"] = 0;
                           aa_main_chain["CA"] = 0;
                           aa_main_chain["C"] = 0;
                           aa_main_chain["O"] = 0;
                        }
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;

			lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                        // this is a new unknown residue
                        // each instance of the residue gets a unique name, so we don't have
                        // to worry about duplicates and alternate duplicates
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 created.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name);
                        
			abb_msgs_udp += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 created.\\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name);
                        
			// if ( advanced_config.debug_2  )
                        // {
                        //    printf("1.1 <%s>\n", new_residue_name.toLatin1().data());
                        // }
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        // create new_residue
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = 8;  // other
                        new_residue.molvol = misc.avg_volume * atom_counts[count_idx];
                        // new_residue.asa = misc.avg_asa * atom_counts[count_idx];
                        new_residue.asa = 0;
                        new_residue.vbar = misc.avg_vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );
                        new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                        new_bead.color = 10;         // light green
                        new_bead.placing_method = 0; // cog
                        new_bead.chain = 1;          // side chain
                        new_bead.volume = misc.avg_volume * atom_counts[count_idx];
                        new_bead.mw = misc.avg_mass * atom_counts[count_idx];
                        new_residue.r_bead.push_back(new_bead);
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // if ( advanced_config.debug_2  )
                        // {
                        //    printf("1.1b <%s>\n", new_residue.name.toLatin1().data());
                        // }
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
			lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                     }
                     else
                     {
                        new_residue_name = QString("%1_NC%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        // if ( advanced_config.debug_2  )
                        // {
                        //    printf("1.2 <%s>\n", new_residue_name.toLatin1().data());
                        // }
                        if (new_residue_name != new_residue.name)
                        {
                           // US_Static::us_message(us_tr("Internal error:"), 
                           //                      us_tr("Unexpected new residue name inconsistancy.\n") +
                           //                      QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        // US_Static::us_message(us_tr("Internal error:"), 
                        //                      us_tr("Unexpected new residue name residue_list inconsistancy."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     if (auto_bb_aa)
                     {
                        if (residue_list[respos].r_atom.size() < 4) 
                        {
                           aa_main_chain[this_atom->name]++;
                           if (residue_list[respos].r_atom.size() == 3) 
                           {
                              // early handling in case of no side chain
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 residue_list[respos].type = 0;
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                              }
                           }
                        } 
                        else 
                        {
                           if (residue_list[respos].r_atom.size() == 4) 
                           {
                              if (aa_main_chain["N"] == 1 &&
                                  aa_main_chain["CA"] == 1 &&
                                  aa_main_chain["C"] == 1 &&
                                  aa_main_chain["O"] == 1)
                              {
                                 // ok, we have a proper backbone
                                 // so we have to redo the beads etc.
                                 abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 PB found, 2 beads created.\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(new_residue_name);
				 
				 abb_msgs_udp += QString("ABB: %1Molecule %2 Residue %3 %4 Non-coded residue %5 PB found, 2 beads created.\\n")
                                    .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                                    .arg(j+1)
                                    .arg(this_atom->resName)
                                    .arg(this_atom->resSeq)
                                    .arg(new_residue_name);

                                 current_bead_assignment = 1;
                                 // redo 1st bead
                                 residue_list[respos].type = 0;

                                 residue_list[respos].r_bead[0].hydration = 
                                    (unsigned int)(misc.avg_hydration * 4 + .5);
                                 residue_list[respos].r_bead[0].chain = 0;  // main chain
                                 residue_list[respos].r_bead[0].volume = misc.avg_volume * 4;
                                 residue_list[respos].r_bead[0].mw = misc.avg_mass * 4;

                                 // create a 2nd bead
                                 residue_list[respos].r_bead.push_back(residue_list[respos].r_bead[0]);
                                 residue_list[respos].r_bead[1].hydration = 
                                    (unsigned int)(misc.avg_hydration * (atom_counts[count_idx] - 4) + .5);
                                 residue_list[respos].r_bead[1].chain = 1;  // side chain
                                 residue_list[respos].r_bead[1].volume =
                                    misc.avg_volume * (atom_counts[count_idx] - 4);
                                 residue_list[respos].r_bead[1].mw = 
                                    misc.avg_mass * (atom_counts[count_idx] - 4);
                              }
                           }
                        }
                     }
            
                     new_atom.name = (this_atom->name == "OXT" ? "OXT'" : this_atom->name);
                     new_atom.hybrid.name = this_atom->name;
                     new_atom.hybrid.mw = misc.avg_mass;
                     new_atom.hybrid.radius = misc.avg_radius;
                     new_atom.bead_assignment = current_bead_assignment; 
                     new_atom.positioner = true;
                     new_atom.serial_number = residue_list[respos].r_atom.size();
                     new_atom.hydration = misc.avg_hydration;
                     residue_list[respos].r_atom.push_back(new_atom);
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     // if ( advanced_config.debug_2  )
                     // {
                     //    printf("1.3 <%s> adding <%s> respos %u\n", new_residue.name.toLatin1().data(), atom_to_add.name.toLatin1().data(), respos);
                     // }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                     {
                        QString mapkey = QString("%1|%2").arg(atom_to_add.resName).arg(atom_to_add.name);
                        if ( atom_to_add.name == "OXT" )
                        {
                           mapkey = "OXT|OXT";
                        }
                        if ( !residue_atom_hybrid_map.count( mapkey ) &&
                             residue_atom_abb_hybrid_map.count( atom_to_add.name ) ) {
                           residue_atom_hybrid_map[ mapkey ] = residue_atom_abb_hybrid_map[ atom_to_add.name ];
                           // us_qdebug( QString( "hybrid map created for %1 created as %2" ).arg( mapkey ).arg( residue_atom_abb_hybrid_map[atom_to_add.name ] ) );
                        }
                     }
                  }
                  break;
               case 4: // missing atoms
                  {
		    PDB_atom *this_atom = &org_model.molecule[j].atom[k];
                     QString idx = QString("%1|%2").arg(j).arg(org_model.molecule[j].atom[k].resSeq);
                     QString new_residue_idx = this_atom->resName;  
                     int pos = molecules_residue_min_missing[idx];
                     if ( pos < 0 )
                     {
                         // cout << QString("Internal error: idx %1 Missing atom error %2 ! < 0!\n")
                         //   .arg(idx)
                         //   .arg(pos);
                        exit(-1);
                     }
                     //cout << QString("ta->resseq %1, pos %2\n").arg(this_atom->resName).arg(pos);

		     int orgrespos = multi_residue_map[this_atom->resName][pos];
                     if (this_atom->resSeq != lastResSeq) 
                     {
                        // new residue to create with existing atoms, but increased values...
                        // possibly for a multi-bead residue
                        one_bead = false;
                        current_bead_assignment = 0;
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;

			lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                        // setup how many atoms are missing from each bead
                        vector < int > beads_missing_atom_count;
                        //printf("orgrespos %d\n", orgrespos);
                        beads_missing_atom_count.resize(residue_list[orgrespos].r_bead.size());

                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           beads_missing_atom_count[i] = 0;
                        }
                        // accumulate missing atom count for each bead
                        for ( unsigned int i = 0; i < molecules_residue_missing_atoms[idx][pos].size(); i++ )
                        {
                           beads_missing_atom_count[molecules_residue_missing_atoms_beads[idx][pos][i]]++;
                        }
                        // do any of these leave an empty bead?
                        // first, we need to know how many atoms are in each bead...
                        vector < int > atoms_per_bead;
                        atoms_per_bead.resize(residue_list[orgrespos].r_bead.size());
                        for ( unsigned int i = 0; i < atoms_per_bead.size(); i++ )
                        {
                           atoms_per_bead[i] = 0;
                        }
                        for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                        {
                           atoms_per_bead[residue_list[orgrespos].r_atom[i].bead_assignment]++;
                        }
                        
                        for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                        {
                           if ( atoms_per_bead[i] - beads_missing_atom_count[i] < 1 )
                           {
                              // ugh, fall back to one bead!
                              one_bead = true;
                              //printf("notice: fallback to one bead for missing atom\n");
                           }
                        }

                        vector < double > atoms_scale_weight; // multiplier for atoms weight
                        vector < double > atoms_scale_radius; // multiplier for atoms radii
                        atoms_scale_weight.resize(residue_list[orgrespos].r_bead.size());
                        atoms_scale_radius.resize(residue_list[orgrespos].r_bead.size());
                        if ( one_bead )
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           double tot_mw = 0;
                           double tot_mw_missing = 0;
                           double tot_radii3 = 0;
                           double tot_radii3_missing = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           atoms_scale_weight[0] = 
                              tot_mw / tot_mw_missing;
                           atoms_scale_radius[0] = 
                              pow(tot_radii3 / tot_radii3_missing, 1.0/3.0);
                        }
                        else
                        {
                           // sum molecular weight & radii cubed for total bead & bead sans missing
                           vector < double > tot_mw;
                           vector < double > tot_mw_missing;
                           vector < double > tot_radii3;
                           vector < double > tot_radii3_missing;
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              tot_mw.push_back(0.0);
                              tot_mw_missing.push_back(0.0);
                              tot_radii3.push_back(0.0);
                              tot_radii3_missing.push_back(0.0);
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              tot_mw[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += residue_list[orgrespos].r_atom[i].hybrid.mw;
                              tot_radii3[residue_list[orgrespos].r_atom[i].bead_assignment]
                                 += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 tot_mw_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += residue_list[orgrespos].r_atom[i].hybrid.mw;
                                 tot_radii3_missing[residue_list[orgrespos].r_atom[i].bead_assignment]
                                    += pow(residue_list[orgrespos].r_atom[i].hybrid.radius, 3);
                              }
                           }
                           for ( unsigned int i = 0; i < beads_missing_atom_count.size(); i++ )
                           {
                              atoms_scale_weight[i] = 
                                 tot_mw[i] / tot_mw_missing[i];
                              atoms_scale_radius[i] = 
                                 pow(tot_radii3[i] / tot_radii3_missing[i], 1.0/3.0);
                           }
                        }

                        // create the new residue
                        new_residues[new_residue_idx]++;
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        if ( residue_list[orgrespos].type == 0 ) // aa
                        {
                           unknown_aa_residues_to_check.push_back(new_residue_name);
                        }

                        abb_msgs += QString("ABB: %1Molecule %2 Residue %3 %4 Missing atom residue copy %5 created%6.\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name)
                           .arg(one_bead ? " as one bead" : "");


                        abb_msgs_udp += QString("ABB: %1Molecule %2 Residue %3 %4 Missing atom residue copy %5 created%6.\\n")
                           .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
                           .arg(j+1)
                           .arg(this_atom->resName)
                           .arg(this_atom->resSeq)
                           .arg(new_residue_name)
                           .arg(one_bead ? " as one bead" : "");

                        // if ( advanced_config.debug_2  )
                        // {
                        //    printf("a1.1 <%s>\n", new_residue_name.toLatin1().data());
                        // }
                        atom_counts[QString("%1|%2|%3")
                                    .arg(j)
                                    .arg(new_residue_name)
                                    .arg(this_atom->resSeq)] = atom_counts[count_idx];
                        new_residue.comment = QString("Temporary residue %1").arg(new_residue_name);
                        new_residue.name = new_residue_name;
                        new_residue.unique_name = new_residue_name;
                        new_residue.type = residue_list[orgrespos].type;
                        new_residue.molvol = residue_list[orgrespos].molvol;
                        new_residue.asa = residue_list[orgrespos].asa;
                        new_residue.vbar = residue_list[orgrespos].vbar;
                        new_residue.r_atom.clear( );
                        new_residue.r_bead.clear( );

                        // create the beads
                        if ( one_bead )
                        {
                           new_bead.volume = 0;
                           new_bead.mw = 0;
                           new_bead.hydration = 0;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead.volume += residue_list[orgrespos].r_bead[i].volume;
                              new_bead.mw += residue_list[orgrespos].r_bead[i].mw;
                              new_bead.hydration += residue_list[orgrespos].r_bead[i].hydration;
                           }
                           // new_bead.hydration = (unsigned int)(misc.avg_hydration * atom_counts[count_idx] + .5);
                           new_bead.color = 10;         // light green
                           new_bead.placing_method = 0; // cog
                           new_bead.chain = 1;          // side chain
                           // do we have an AA with a complete PB?
                           if ( auto_bb_aa &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(0)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(1)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(2)] &&
                                !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                      .arg(idx)
                                                                      .arg(pos)
                                                                      .arg(3)] )
                           {
                              new_bead.chain = 0;      // main chain
                           }

                           new_residue.r_bead.push_back(new_bead);
                           // check for positioner
                           bool any_positioner = false;
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner = true;
                                    break;
                                 }
                              }
                           }
                           // create the atoms
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( new_atom.name == "OXT" )
                                 {
                                    new_atom.name = "OXT'";
                                 }
                                 new_atom.hybrid.mw *= atoms_scale_weight[0]; // misc.avg_mass;
                                 new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                 new_atom.hybrid.radius *= atoms_scale_radius[0]; // misc.avg_radius;
                                 new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                 new_atom.bead_assignment = 0; 
                                 if ( !any_positioner )
                                 {
                                    new_atom.positioner = true;
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        } 
                        else
                        {
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_bead.size(); i++ )
                           {
                              new_bead = residue_list[orgrespos].r_bead[i];
                              if ( beads_missing_atom_count[i] )
                              {
                                 // only set the beads with missing atoms
                                 new_bead.color = 10;
                                 new_bead.placing_method = 0;
                              }
                              new_residue.r_bead.push_back(new_bead);
                           }
                           // check for positioners
                           vector < bool > any_positioner;
                           any_positioner.resize(residue_list[orgrespos].r_bead.size());
                           for ( unsigned int i = 0; i < any_positioner.size(); i++ )
                           {
                              any_positioner[i] = false;
                           }
                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 if ( residue_list[orgrespos].r_atom[i].positioner )
                                 {
                                    any_positioner[residue_list[orgrespos].r_atom[i].bead_assignment] = true;
                                 }
                              }
                           }

                           for ( unsigned int i = 0; i < residue_list[orgrespos].r_atom.size(); i++ )
                           {
                              if ( !molecules_residue_missing_atoms_skip[QString("%1|%2|%3")
                                                                         .arg(idx)
                                                                         .arg(pos)
                                                                         .arg(i)] )
                              {
                                 new_atom = residue_list[orgrespos].r_atom[i];
                                 if ( beads_missing_atom_count[new_atom.bead_assignment] )
                                 {
                                    new_atom.hybrid.mw *= atoms_scale_weight[new_atom.bead_assignment]; // misc.avg_mass;
                                    new_atom.hybrid.mw = (int)(new_atom.hybrid.mw * 100 + .5) / 100.0;
                                    new_atom.hybrid.radius *= atoms_scale_radius[new_atom.bead_assignment]; // misc.avg_radius;
                                    new_atom.hybrid.radius = (int)(new_atom.hybrid.radius * 100 + .5) / 100.0;
                                    if ( !any_positioner[new_atom.bead_assignment] )
                                    {
                                       new_atom.positioner = true;
                                    }
                                 }
                                 new_residue.r_atom.push_back(new_atom);
                              }
                           }
                        }
                        multi_residue_map[new_residue.name].push_back(residue_list.size());
                        residue_list.push_back(new_residue);
                        lastResSeq = org_model.molecule[j].atom[k].resSeq;
                        lastResName = org_model.molecule[j].atom[k].resName;
                        // if ( advanced_config.debug_2  )
                        // {
                        //    printf("a1.1b <%s>\n", new_residue.name.toLatin1().data());
                        // }
                        model->residue.push_back(residue_list[multi_residue_map[new_residue.name][0]]);
			lastOrgResSeq  = org_model.molecule[j].atom[k].orgResSeq;
                        lastOrgChainID = org_model.molecule[j].atom[k].orgChainID;
                        lastOrgResName = org_model.molecule[j].atom[k].orgResName;
                     }
                     else
                     {
                        new_residue_name = QString("%1_MA%2").arg(this_atom->resName).arg(new_residues[new_residue_idx]);
                        unknown_residues[new_residue_name] = true;
                        // if ( advanced_config.debug_2  )
                        // {
                        //    printf("a1.2 <%s>\n", new_residue_name.toLatin1().data());
                        // }
                        if (new_residue_name != new_residue.name)
                        {
                           // US_Static::us_message(us_tr("Internal error:"), 
                           //                      us_tr("Unexpected new residue name inconsistancy (a).\n") +
                           //                      QString("\"%1\" != \"%2\"").arg(new_residue_name).arg(new_residue.name));
                           exit(-1);
                        }
                     }
                     // the new residue should always be the last one, since we just added it
                     int respos = residue_list.size() - 1;
                     if (new_residue.name != residue_list[respos].name)
                     {
                        // US_Static::us_message(us_tr("Internal error:"), 
                        //                      us_tr("Unexpected new residue name residue_list inconsistancy (a)."));
                        exit(-1);
                     }
                     // ok, now we can push back the modified atom
                     PDB_atom atom_to_add = org_model.molecule[j].atom[k];
                     atom_to_add.resName = new_residue.name;
                     if ( atom_to_add.name == "OXT" )
                     {
                        atom_to_add.name = "OXT'";
                     }
                     // if ( advanced_config.debug_2  )
                     // {
                     //    printf("a1.3 <%s>\n", new_residue.name.toLatin1().data());
                     // }
                     tmp_chain.atom.push_back(atom_to_add);
                     model->residue[model->residue.size() - 1] = residue_list[residue_list.size() - 1];
                     {
                        QString mapkey = QString("%1|%2").arg(atom_to_add.resName).arg(atom_to_add.name);
                        if ( atom_to_add.name == "OXT" )
                        {
                           mapkey = "OXT|OXT";
                        }
                        if ( !residue_atom_hybrid_map.count( mapkey ) &&
                             residue_atom_abb_hybrid_map.count( atom_to_add.name ) ) {
                           residue_atom_hybrid_map[ mapkey ] = residue_atom_abb_hybrid_map[ atom_to_add.name ];
                           // us_qdebug( QString( "hybrid map created for %1 created as %2" ).arg( mapkey ).arg( residue_atom_abb_hybrid_map[atom_to_add.name ] ) );
                        }
                     }
                  }
                  break;
                     
               default:
                  // US_Static::us_message(us_tr("Internal error:"), 
                  //                      us_tr("Unhandled bead_exception code."));
                  exit(-1);
                  break;
               } // switch
	    
            } // atoms
            if ( tmp_chain.atom.size() )
            {
               tmp_chain.chainID = tmp_chain.atom[0].chainID;
            }
            model->molecule.push_back(tmp_chain);
            abb_msgs += "\n";
	    abb_msgs_udp += "\\n";
         } // molecules

         // {
         //    QColor save_color = editor->textColor();
         //    QFont save_font = editor->currentFont();
         //    QFont new_font = save_font;
         //    new_font.setStretch(70);
         //    editor->setCurrentFont(new_font);
         //    editor->setTextColor("blue");
         us_log->log("\n" + abb_msgs);
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = "\\n" + abb_msgs_udp;
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }
	 accumulated_msgs += "\\n" + abb_msgs_udp;
         //    editor->setTextColor(save_color);
         //    editor->setCurrentFont(save_font);
         last_abb_msgs = "\n\nAutomatic Bead Builder messages:\n" + abb_msgs.replace("ABB: ","  ");
         // }
         
	 calc_vbar(model);
         
	 update_vbar();
         
	 //printf("vbar after: %g\n", model->vbar);

         // remove from unknown any aa residues that have 2 beads and a side chain
         // with a N,C,CA,O MC
         for ( unsigned int i = 0; i < unknown_aa_residues_to_check.size(); i++ )
         {
            // if ( advanced_config.debug_2  )
            // {
            //    printf("unknown residue %s checking for removal from broken\n",
            //           unknown_aa_residues_to_check[i].toLatin1().data());
            // }
            unsigned int respos = multi_residue_map[unknown_aa_residues_to_check[i]][0];
            if ( residue_list[respos].r_bead.size() == 2 &&
                 residue_list[respos].r_atom.size() > 4 ) 
            {
               map < QString, int > aa_main_chain;  // to make sure we have a good main chain
               aa_main_chain.clear( );
               aa_main_chain["N"] = 0;
               aa_main_chain["CA"] = 0;
               aa_main_chain["C"] = 0;
               aa_main_chain["O"] = 0;
               for ( unsigned int j = 0; j < residue_list[respos].r_atom.size(); j++ )
               {
                  aa_main_chain[residue_list[respos].r_atom[j].name]++;
               }
               if (aa_main_chain["N"] == 1 &&
                   aa_main_chain["CA"] == 1 &&
                   aa_main_chain["C"] == 1 &&
                   aa_main_chain["O"] == 1)
               {
                  // if ( advanced_config.debug_2  )
                  // {
                  //    printf("unknown residue %s removed from broken override\n",
                  //           unknown_aa_residues_to_check[i].toLatin1().data());
                  // }
                  unknown_residues.erase(unknown_aa_residues_to_check[i]);
               }
            }
         }
      }
      cout << "After else" << endl;
   }
// #if defined(AUTO_BB_DEBUG) || 1
//    QString str1;
//    QFile f(somo_tmp_dir + SLASH + "tmp.somo.residue");
//    if (f.open(IO_WriteOnly|IO_Translate))
//    {
//       QTextStream ts(&f);
//       for (unsigned int i=0; i<residue_list.size(); i++)
//       {
// 	ts << residue_list[i].comment << Qt::endl;
//          ts << residue_list[i].name.toUpper()
//             << "\t" << residue_list[i].type
//             << "\t" << str1.sprintf("%7.2f", residue_list[i].molvol)
//             << "\t" << residue_list[i].asa
//             << "\t" << residue_list[i].r_atom.size()
//             << "\t" << residue_list[i].r_bead.size()
//             << "\t" << residue_list[i].vbar << Qt::endl;
//          for (unsigned int j=0; j<residue_list[i].r_atom.size(); j++)
//          {
//             ts << residue_list[i].r_atom[j].name.toUpper()
//                << "\t" << residue_list[i].r_atom[j].hybrid.name
//                << "\t" << residue_list[i].r_atom[j].hybrid.mw
//                << "\t" << residue_list[i].r_atom[j].hybrid.radius
//                << "\t" << residue_list[i].r_atom[j].bead_assignment
//                << "\t" << (unsigned int) residue_list[i].r_atom[j].positioner
//                << "\t" << residue_list[i].r_atom[j].serial_number 
//                << "\t" << residue_list[i].r_atom[j].hydration
//                << Qt::endl;
//          }
//          for (unsigned int j=0; j<residue_list[i].r_bead.size(); j++)
//          {
//             ts << residue_list[i].r_bead[j].hydration
//                << "\t" << residue_list[i].r_bead[j].color
//                << "\t" << residue_list[i].r_bead[j].placing_method
//                << "\t" << residue_list[i].r_bead[j].chain
//                << "\t" << residue_list[i].r_bead[j].volume << Qt::endl;
//          }
//          str1.sprintf("%d: ", i+1);
//          str1 += residue_list[i].name.toUpper();
//       }
//       f.close();
//    }
// #endif

   qDebug() << "debug 1\n";
   if ( !misc.compute_vbar || hydro.mass_correction )
   {
     QString str = us_tr("ATTENTION: User entered");
      if ( !misc.compute_vbar )
      {
         str += QString(" vbar = %1").arg(misc.vbar);
      }
      if ( hydro.mass_correction )
      {
         str += QString(" MW = %1").arg(hydro.mass);
      }
      //QColor save_color = editor->textColor();
      //editor->setTextColor("red");
      us_log->log(str + "\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = str + "\\n";
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	} 
      accumulated_msgs += str + "\\n";
      //editor->setTextColor(save_color);
   }
   qDebug() << "debug 2\n";
   return 0;
}

void US_Saxs_Util::update_vbar()
{
  if (misc.compute_vbar && model_vector.size() > 0) // after reading the pdb file, the vbar is calculated.
   {// If we computed vbar, we assign this to result.vbar, which should be used in the calculation.
      if (current_model >= model_vector.size())
      {
         current_model = 0;
      }
      results_hydro.vbar = model_vector[current_model].vbar;
      
   }
   else
   {
      results_hydro.vbar = 
         misc.vbar - 
         (4.25e-4 * (K0 + misc.vbar_temperature - K20));
      
   }

   
  // cout << "VBAR: res " << results_hydro.vbar << endl;
}

bool US_Saxs_Util::hard_coded_defaults( map < QString, QString > &results, map < QString, QString > & parameters)
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

   asa_hydro.probe_radius = (float) 1.4;
   asa_hydro.probe_recheck_radius = (float) 1.4;
   asa_hydro.threshold = 20.0;
   asa_hydro.threshold_percent = 50.0;
   asa_hydro.grid_threshold = 10.0;
   asa_hydro.grid_threshold_percent = 30.0;
   asa_hydro.calculation = true;
   asa_hydro.recheck_beads = true;
   asa_hydro.method = true; // by default use ASAB1
   asa_hydro.asab1_step = 1.0;

   grid_hydro.cubic = true;       // apply cubic grid
   grid_hydro.hydrate = true;    // true: hydrate model
   grid_hydro.center = 0;    // 1: center of cubelet, 0: center of mass, 2: center of scattering
   grid_hydro.tangency = false;   // true: Expand beads to tangency
   grid_hydro.cube_side = 5.0;
   grid_hydro.enable_asa = true;   // true: enable asa

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
   pdb_parse.missing_residues = 2;
   pdb_parse.missing_atoms = 2;

   saxs_options_hydro.water_e_density = 0.334f; // water electron density in e/A^3

   saxs_options_hydro.h_scat_len = -0.3742f;        // H scattering length (*10^-12 cm)
   saxs_options_hydro.d_scat_len = 0.6671f ;        // D scattering length (*10^-12 cm)
   saxs_options_hydro.h2o_scat_len_dens = -0.562f;  // H2O scattering length density (*10^-10 cm^2)
   saxs_options_hydro.d2o_scat_len_dens = 6.404f;   // D2O scattering length density (*10^-10 cm^2)
   saxs_options_hydro.d2o_conc = 0.16f;             // D2O concentration (0 to 1)
   saxs_options_hydro.frac_of_exch_pep = 0.1f;      // Fraction of exchanged peptide H (0 to 1)

   saxs_options_hydro.wavelength = 1.5;         // scattering wavelength
   saxs_options_hydro.start_angle = 0.014f;     // start angle
   saxs_options_hydro.end_angle = 8.214f;       // ending angle
   saxs_options_hydro.delta_angle = 0.2f;       // angle stepsize
   saxs_options_hydro.max_size = 40.0;          // maximum size (A)
   saxs_options_hydro.bin_size = 1.0f;          // Bin size (A)
   saxs_options_hydro.hydrate_pdb = false;      // Hydrate the PDB model? (true/false)
   saxs_options_hydro.curve = 1;                // 0 = raw, 1 = saxs, 2 = sans
   saxs_options_hydro.saxs_sans = 0;            // 0 = saxs, 1 = sans

   saxs_options_hydro.guinier_csv = false;
   saxs_options_hydro.guinier_csv_filename = "guinier";
   saxs_options_hydro.qRgmax = 1.3e0;
   saxs_options_hydro.qstart = 1e-7;
   saxs_options_hydro.qend = .5e0;
   saxs_options_hydro.pointsmin = 10;
   saxs_options_hydro.pointsmax = 100;

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
      
   batch.missing_atoms = 2;
   batch.missing_residues = 2;
   batch.somo = true;
   batch.grid = false;
   batch.hydro = true;
   batch.zeno = false;
   batch.avg_hydro = false;
   batch.avg_hydro_name = "results";
   batch.height = 0;
   batch.width = 0;
   batch.file.clear( );

   path_load_pdb = "";
   path_view_pdb = "";
   path_load_bead_model = "";
   path_view_asa_res = "";
   path_view_bead_model = "";
   path_open_hydro_res = "";
   saxs_options_hydro.path_load_saxs_curve = "";
   saxs_options_hydro.path_load_gnom = "";
   saxs_options_hydro.path_load_prr = "";

   save_params.field.clear( );

   asa_hydro.hydrate_probe_radius = 1.4f;
   asa_hydro.hydrate_threshold = 10.0f;

   misc.target_e_density       = 0e0;
   misc.target_volume          = 0e0;
   misc.set_target_on_load_pdb = false;
   misc.equalize_radii         = false;


   dmd_options.force_chem = true;
   dmd_options.pdb_static_pairs = false;
   dmd_options.threshold_pb_pb = 5;
   dmd_options.threshold_pb_sc = 5;
   dmd_options.threshold_sc_sc = 5;
   
   saxs_options_hydro.normalize_by_mw = true;

   saxs_options_hydro.saxs_iq_native_debye = false;
   saxs_options_hydro.saxs_iq_native_hybrid = false;
   saxs_options_hydro.saxs_iq_native_hybrid2 = false;
   saxs_options_hydro.saxs_iq_native_hybrid3 = true;
   saxs_options_hydro.saxs_iq_native_fast = false;
   saxs_options_hydro.saxs_iq_native_fast_compute_pr = false;
   saxs_options_hydro.saxs_iq_foxs = false;
   saxs_options_hydro.saxs_iq_crysol = false;

   saxs_options_hydro.sans_iq_native_debye = true;
   saxs_options_hydro.sans_iq_native_hybrid = false;
   saxs_options_hydro.sans_iq_native_hybrid2 = false;
   saxs_options_hydro.sans_iq_native_hybrid3 = false;
   saxs_options_hydro.sans_iq_native_fast = false;
   saxs_options_hydro.sans_iq_native_fast_compute_pr = false;
   saxs_options_hydro.sans_iq_cryson = false;

   saxs_options_hydro.hybrid2_q_points = 15;

   saxs_options_hydro.iq_ask = false;

   saxs_options_hydro.iq_scale_ask = false;
   saxs_options_hydro.iq_scale_angstrom = true;
   saxs_options_hydro.iq_scale_nm = false;

   saxs_options_hydro.sh_max_harmonics = 15;
   saxs_options_hydro.sh_fibonacci_grid_order = 17;
   saxs_options_hydro.crysol_hydration_shell_contrast = 0.03f;
   saxs_options_hydro.crysol_default_load_difference_intensity = true;
   saxs_options_hydro.crysol_version_26 = true;

   saxs_options_hydro.fast_bin_size = 0.5f;
   saxs_options_hydro.fast_modulation = 0.23f;

   saxs_options_hydro.compute_saxs_coeff_for_bead_models = true;
   saxs_options_hydro.compute_sans_coeff_for_bead_models = false;

   saxs_options_hydro.default_atom_filename = "";
   saxs_options_hydro.default_hybrid_filename = "";
   saxs_options_hydro.default_saxs_filename = "";
   saxs_options_hydro.default_rotamer_filename = "";

   saxs_options_hydro.steric_clash_distance         = 20.0;
   saxs_options_hydro.steric_clash_recheck_distance = 0.0;

   saxs_options_hydro.disable_iq_scaling = false;
   saxs_options_hydro.autocorrelate = true;
   saxs_options_hydro.hybrid_radius_excl_vol = false;
   saxs_options_hydro.scale_excl_vol = 1.0f;
   saxs_options_hydro.subtract_radius = false;
   saxs_options_hydro.iqq_scale_minq = 0.0f;
   saxs_options_hydro.iqq_scale_maxq = 0.0f;

   saxs_options_hydro.iqq_scale_nnls = false;
   saxs_options_hydro.iqq_scale_linear_offset = false;
   saxs_options_hydro.iqq_scale_chi2_fitting = true;
   saxs_options_hydro.iqq_expt_data_contains_variances = false;
   saxs_options_hydro.iqq_ask_target_grid = true;
   saxs_options_hydro.iqq_scale_play = false;
   saxs_options_hydro.swh_excl_vol = 0.0f;
   saxs_options_hydro.iqq_default_scaling_target = "";

   saxs_options_hydro.saxs_iq_hybrid_adaptive = true;
   saxs_options_hydro.sans_iq_hybrid_adaptive = true;

   saxs_options_hydro.bead_model_rayleigh   = true;
   saxs_options_hydro.iqq_log_fitting       = false;
   saxs_options_hydro.iqq_kratky_fit        = false;
   saxs_options_hydro.iqq_use_atomic_ff     = false;
   saxs_options_hydro.iqq_use_saxs_excl_vol = false;
   saxs_options_hydro.alt_hydration         = false;

   saxs_options_hydro.xsr_symmop                = 2;
   saxs_options_hydro.xsr_nx                    = 32;
   saxs_options_hydro.xsr_ny                    = 32;
   saxs_options_hydro.xsr_griddistance          = 3e0;
   saxs_options_hydro.xsr_ncomponents           = 1;
   saxs_options_hydro.xsr_compactness_weight    = 10e0;
   saxs_options_hydro.xsr_looseness_weight      = 10e0;
   saxs_options_hydro.xsr_temperature           = 1e-3;

   hydro.zeno_zeno              = true;
   hydro.zeno_interior          = false;
   hydro.zeno_surface           = false;
   hydro.zeno_zeno_steps        = 1000;
   hydro.zeno_interior_steps    = 1000;
   hydro.zeno_surface_steps     = 1000;
   hydro.zeno_surface_thickness = 0.0f;

   misc.hydro_supc              = true;
   misc.hydro_zeno              = false;

   rotamer_changed = true;  // force on-demand loading of rotamer file

   batch.saxs_search = false;
   batch.zeno        = false;

   saxs_options_hydro.ignore_errors                      = false;
   saxs_options_hydro.alt_ff                             = true;
   saxs_options_hydro.crysol_explicit_hydrogens          = false;
   saxs_options_hydro.use_somo_ff                        = false;
   saxs_options_hydro.five_term_gaussians                = true;
   saxs_options_hydro.iq_exact_q                         = false;
   saxs_options_hydro.use_iq_target_ev                   = false;
   saxs_options_hydro.set_iq_target_ev_from_vbar         = false;
   saxs_options_hydro.iq_target_ev                       = 0e0;
   saxs_options_hydro.hydration_rev_asa                  = false;
   saxs_options_hydro.compute_exponentials               = false;
   saxs_options_hydro.compute_exponential_terms          = 5;
   saxs_options_hydro.dummy_saxs_name                    = "DAM";
   saxs_options_hydro.dummy_saxs_names                   .clear( );
   saxs_options_hydro.dummy_saxs_names                   .push_back( saxs_options_hydro.dummy_saxs_name );
   saxs_options_hydro.multiply_iq_by_atomic_volume       = false;
   saxs_options_hydro.dummy_atom_pdbs_in_nm              = false;
   saxs_options_hydro.iq_global_avg_for_bead_models      = false;
   saxs_options_hydro.apply_loaded_sf_repeatedly_to_pdb  = false;
   saxs_options_hydro.bead_models_use_var_len_sf         = false;
   saxs_options_hydro.bead_models_var_len_sf_max         = 10;
   saxs_options_hydro.bead_models_use_gsm_fitting        = false;
   saxs_options_hydro.bead_models_use_quick_fitting      = true;
   saxs_options_hydro.bead_models_use_bead_radius_ev     = true;
   saxs_options_hydro.bead_models_rho0_in_scat_factors   = true;
   saxs_options_hydro.smooth                             = 0;
   saxs_options_hydro.ev_exp_mult                        = 1e0;
   saxs_options_hydro.sastbx_method                      = 0;
   saxs_options_hydro.saxs_iq_sastbx                     = false;
   saxs_options_hydro.saxs_iq_native_sh                  = false;
   saxs_options_hydro.sans_iq_native_sh                  = false;

   saxs_options_hydro.alt_sh1                            = false;
   saxs_options_hydro.alt_sh2                            = false;

   grid_hydro.create_nmr_bead_pdb                        = false;

   batch.compute_iq_only_avg                       = false;

   asa_hydro.vvv                                         = false;
   asa_hydro.vvv_probe_radius                            = 1.4f;
   asa_hydro.vvv_grid_dR                                 = 0.5f;

   misc.export_msroll                              = false;

   saxs_options_hydro.cs_qRgmax                          = 1e0;
   saxs_options_hydro.cs_qstart                          = saxs_options_hydro.qstart * saxs_options_hydro.qstart;
   saxs_options_hydro.cs_qend                            = saxs_options_hydro.qend   * saxs_options_hydro.qend;

   saxs_options_hydro.conc                               = 1e0;
   saxs_options_hydro.psv                                = 7.2e-1;
   saxs_options_hydro.use_cs_psv                         = false;
   saxs_options_hydro.cs_psv                             = 0e0;
   saxs_options_hydro.I0_exp                             = 5.4e-5;
   saxs_options_hydro.I0_theo                            = 1.633e-2;
   saxs_options_hydro.diffusion_len                      = 2.82e-13;
   saxs_options_hydro.nucleon_mass                       = 1.674e-24;

   saxs_options_hydro.guinier_outlier_reject             = false;
   saxs_options_hydro.guinier_outlier_reject_dist        = 2e0;
   saxs_options_hydro.guinier_use_sd                     = false;
   saxs_options_hydro.guinier_use_standards              = false;

   saxs_options_hydro.cryson_sh_max_harmonics            = 15;
   saxs_options_hydro.cryson_sh_fibonacci_grid_order     = 17;
   saxs_options_hydro.cryson_hydration_shell_contrast    = 
      saxs_options_hydro.d2o_conc * saxs_options_hydro.d2o_scat_len_dens +
      ( 1e0 - saxs_options_hydro.d2o_conc ) * ( saxs_options_hydro.h2o_scat_len_dens );
   saxs_options_hydro.cryson_manual_hs                   = false;

   gparams                                         .clear( );
   gparams[ "guinier_auto_fit" ]                   = "1";
   gparams[ "perdeuteration" ]                     = "0";
   gparams[ "guinier_qRtmax" ]                     = "1";
   gparams[ "guinier_electron_nucleon_ratio" ]     = "1.87e0";

   gparams[ "guinier_mwt_k" ]                      = "1";
   gparams[ "guinier_mwt_c" ]                      = "-2.095";
   gparams[ "guinier_mwt_qmax" ]                   = "0.2";

   gparams[ "hplc_bl_linear"             ]         = "false";
   gparams[ "hplc_bl_integral"           ]         = "true";
   gparams[ "hplc_bl_save"               ]         = "false";
   gparams[ "hplc_bl_smooth"             ]         = "0";
   gparams[ "hplc_bl_reps"               ]         = "5";
   gparams[ "hplc_zi_window"             ]         = "25";
   gparams[ "hplc_discard_it_sd_mult"    ]         = "2";
   gparams[ "hplc_cb_discard_it_sd_mult" ]         = "false";
   gparams[ "hplc_dist_max"              ]         = "50.0";

   gparams[ "save_csv_on_load_pdb"       ]         = "false";

   
   /* ADVANCED OPTIONS IN GenApp *********************************************************/

   if ( parameters.count( "cube_side" ) )
     grid_hydro.cube_side    = parameters[ "cube_side" ].toDouble();
   
   if ( grid_hydro.cube_side <= 0 )
     {
       results[ "errors" ] += "Cube side cannot be negative or zero!";
       return false;
     }
   
 
   
   if ( parameters[ "bead_model_list_box" ] == "atob" )
     {
       if( parameters[ "atob_list_box" ] == "zeno") 
	 {
	   if ( parameters[ "zeno_steps_atob" ].toInt() > 0 )
	     hydro.zeno_zeno_steps   = parameters[ "zeno_steps_atob" ].toInt();
	   else
	     {
	       results[ "errors" ] += "Number of steps cannot be negative or zero!";
	       return false;
	     }
	 }
     }


   if ( parameters[ "bead_model_list_box" ] == "somo" )
     {
       if( parameters[ "somo_list_box" ] == "zeno") 
	 {
	   if ( parameters[ "zeno_steps_somo" ].toInt() > 0 )
	     hydro.zeno_zeno_steps   = parameters[ "zeno_steps_somo" ].toInt();
	   else
	     {
	       results[ "errors" ] += "Number of steps cannot be negative or zero!";
	       return false;
	     }
	 }
     }
   
   if ( parameters[ "bead_model_list_box" ] == "somo_o" )
     {
       if( parameters[ "somo_o_list_box" ] == "zeno") 
	 {
	   if ( parameters[ "zeno_steps_somo_o" ].toInt() > 0 )
	     hydro.zeno_zeno_steps   = parameters[ "zeno_steps_somo_o" ].toInt();
	   else
	     {
	       results[ "errors" ] += "Number of steps cannot be negative or zero!";
	       return false;
	     }
	 }
     }

    
   if ( parameters.count( "hydro_options" ) )
     {
       hydro.solvent_name      = parameters[ "solvent" ]; 
       hydro.solvent_acronym   = parameters[ "solvent_acronym" ];
       hydro.temperature       = parameters[ "solvent_temperature" ].toDouble(); //K20 - K0;
       hydro.solvent_viscosity = parameters[ "solvent_viscosity" ].toDouble();   // VISC_20W * 100;
       hydro.solvent_density   = parameters[ "solvent_density" ].toDouble();// DENS_20W; 
     }
   // else 
   //   {
   //     hydro.solvent_name = "Water";
   //     hydro.solvent_acronym = "w";
   //     hydro.temperature = K20 - K0;
   //     hydro.solvent_viscosity = VISC_20W * 100;
   //     hydro.solvent_density = DENS_20W;
   //   }
   
   if ( parameters.count( "bead_model_options" ) )
     overlap_tolerance = parameters[ "bead_overlap_tolerance" ].toDouble();

   if ( parameters.count( "miscellaneous_options" ) ) 
     {
       misc.hydrovol = parameters [ "hydr_water_vol" ].toDouble();
       // if (parameters[ "vbar_options" ] == "c1")
       //    continue;
       if (parameters[ "vbar_options" ] == "c2")
	 {
	   misc.compute_vbar = false;
	   misc.vbar = parameters[ "entered_vbar_value" ].toDouble();
	   misc.vbar_temperature = parameters[ "entered_temp_value" ].toDouble();
	 }
     }
   
   return true;

}



bool US_Saxs_Util::set_default(map < QString, QString > &results, map < QString, QString > &parameters)
{
   // QFile f;
   // QString str;
   // int j;
   // only keep one copy of defaults in system root dir

   /* Commented for now - if no config file*/
   // f.setFileName(USglobal->config_list.system_dir + "/etc/somo.defaults"); 
   // bool config_read = false;
   // if (f.open(IO_ReadOnly)) // read system directory
   // {
   //    j=read_config(f);
   //    if ( j )
   //    {
   //       cout << "read config returned " << j << endl;
   //       US_Static::us_message(us_tr("Please note:"),
   //                            us_tr("The somo.default configuration file was found to be corrupt.\n"
   //                               "Resorting to hard-coded defaults."));
   //    }
   //    else
   //    {
   //       config_read = true;
   //    }
   // }
   // else
   // {
   //    US_Static::us_message(us_tr("Notice:"),
   //                         us_tr("Configuration defaults file ") +
   //                         f.fileName() + us_tr(" not found\nUsing hard-coded defaults."));
   // }

   //   if ( !config_read )
   //{
  
  if( !hard_coded_defaults(results, parameters) )
    return false;

      //}

   // // defaults that SHOULD NOT BE MOVED INTO somo.config

   //  if ( pdb_vis->filename.isEmpty() )
   //  {
   //     pdb_vis->filename = string(getenv("ULTRASCAN")) + "/etc/rasmol.spt"; //default color file
   //  }

     if ( saxs_options_hydro.default_atom_filename.isEmpty() )
     {
       saxs_options_hydro.default_atom_filename = paths + SLASH + "etc" + SLASH + "somo.atom";
       QString atom_fi = saxs_options_hydro.default_atom_filename;
       QFileInfo fi_atom_filename( atom_fi );
       if ( !fi_atom_filename.exists() )
	 {
	   results[ "errors" ] += QString( " file %1 does not exist." ).arg( atom_fi );
	 }
  
       //saxs_options_hydro.default_atom_filename = "/src/ultrascan/etc/somo.atom";
     }
     if ( saxs_options_hydro.default_hybrid_filename.isEmpty() )
     {
       saxs_options_hydro.default_hybrid_filename = paths + SLASH + "etc" + SLASH + "somo.hybrid";
       QString hybrid_fi = saxs_options_hydro.default_hybrid_filename;
       QFileInfo fi_hybrid_filename( hybrid_fi );
       if ( !fi_hybrid_filename.exists() )
	 {
	   results[ "errors" ] += QString( " file %1 does not exist." ).arg( hybrid_fi );
	 }
       //saxs_options_hydro.default_hybrid_filename = "/src/ultrascan/etc/somo.hybrid";
     }
     if ( saxs_options_hydro.default_saxs_filename.isEmpty() )
     {
       saxs_options_hydro.default_saxs_filename = paths + SLASH + "etc" + SLASH + "somo.saxs_atoms";
       QString saxs_fi = saxs_options_hydro.default_saxs_filename;
       QFileInfo fi_saxs_filename( saxs_fi );
       if ( !fi_saxs_filename.exists() )
	 {
	   results[ "errors" ] += QString( " file %1 does not exist." ).arg( saxs_fi );
	 }

       //saxs_options_hydro.default_saxs_filename = "/src/ultrascan/etc/somo.saxs_atoms";
     }
     if ( saxs_options_hydro.default_rotamer_filename.isEmpty() )
     {
       saxs_options_hydro.default_rotamer_filename = paths + SLASH + "etc" + SLASH + "somo.hydrated_rotamer";
       QString rotamer_fi = saxs_options_hydro.default_rotamer_filename;
       QFileInfo fi_rotamer_filename( rotamer_fi );
       if ( !fi_rotamer_filename.exists() )
	 {
	   results[ "errors" ] += QString( " file %1 does not exist." ).arg( rotamer_fi );
	 }
       //saxs_options_hydro.default_rotamer_filename = "/src/ultrascan/etc/somo.hydrated_rotamer";
     }
     if ( saxs_options_hydro.default_ff_filename.isEmpty() )
     {
       saxs_options_hydro.default_ff_filename = paths + SLASH + "etc" + SLASH + "somo.ff";
       QString ff_fi = saxs_options_hydro.default_ff_filename;
       QFileInfo fi_ff_filename( ff_fi );
       if ( !fi_ff_filename.exists() )
	 {
	   results[ "errors" ] += QString( " file %1 does not exist." ).arg( ff_fi );
	 }            
       //saxs_options_hydro.default_ff_filename = "/src/ultrascan/etc/somo.ff";
     }

   rotamer_changed = true;  // force on-demand loading of rotamer file

   default_sidechain_overlap = sidechain_overlap;
   default_mainchain_overlap = mainchain_overlap;
   default_buried_overlap = buried_overlap;
   default_grid_exposed_overlap = grid_exposed_overlap;
   default_grid_buried_overlap = grid_buried_overlap;
   default_grid_overlap = grid_overlap;
   default_bead_output = bead_output;
   default_asa = asa_hydro;
   default_misc = misc;

   default_overlap_tolerance = overlap_tolerance;
   default_hydro = hydro;
   default_pdb_vis = pdb_vis;
   default_pdb_parse = pdb_parse;
   default_grid_hydro = grid_hydro;
   default_saxs_options = saxs_options_hydro;
   default_batch = batch;
   default_save_params = save_params;
   default_bd_options = bd_options;
   default_anaflex_options = anaflex_options;
   default_gparams         = gparams;

   return true;
}


void US_Saxs_Util::calc_vol_for_saxs()
{
   // cout << QString( "calc_vol_for_saxs() model_vector.size() %1\n" ).arg( model_vector.size() );
   //saxs_util->setup_saxs_options();

   for ( unsigned int i = 0; i < model_vector.size(); i++  )
   {
      model_vector[i].volume = 0;
      for ( unsigned int j = 0; j < model_vector[i].molecule.size (); j++ )
      {
         for ( unsigned int k = 0; k < model_vector[i].molecule[j].atom.size(); k++ )
         {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            double excl_vol = 0e0;
            // double scaled_excl_vol;
            // unsigned int this_e;
            // unsigned int this_e_noh;
            double si = 0e0;
            // if ( !saxs_util->set_excluded_volume( *this_atom, 
            //                                       excl_vol, 
            //                                       scaled_excl_vol, 
            //                                       saxs_options_hydro, 
            //                                       residue_atom_hybrid_map,
            //                                       this_e,
            //                                       this_e_noh,
            //                                       si ) )
            // {
	    //   //editor_msg( "dark red", saxs_util->errormsg );
            // } else {
               this_atom->saxs_excl_vol = excl_vol;
               this_atom->si            = si;
               model_vector_as_loaded[ i ].molecule[ j ].atom[ k ].si = si;
               model_vector[ i ].volume += excl_vol;
	       //} 
         }
      }
      // editor_msg( "dark blue", QString( "Model %1 total volume %2" )
      //             .arg( model_vector[ i ].model_id )
      //             .arg( model_vector[ i ].volume ) );
   }
}


bool US_Saxs_Util::calc_mw_hydro() 
{
   // cout << list_chainIDs(model_vector);

   unsigned int save_current_model = current_model;
   // QString error_string;

   last_pdb_load_calc_mw_msg.clear( );

   US_Saxs_Util usu;
   bool do_excl_vol = true;
   
   // if ( !control_parameters.count( "saxsfile" ) ||
   //      !control_parameters.count( "hybridfile" ) ||
   //      !control_parameters.count( "atomfile" ) )
   // {
   //    errormsg = "calc_mw(): requires saxsfile, hybridfile & atomfile defined";
   //    do_excl_vol = false;
   //    return false;
   // }

   // cout << "Calc_mw!!" << endl;

   for (unsigned int i = 0; i < model_vector.size(); i++)
   {
     us_log->log( QString( "\nModel: %1 vbar %2 cm^3/g\n" )
                       .arg( model_vector[i].model_id )
                       .arg( QString("").sprintf("%.3f", model_vector[i].vbar) ) );
      
     if ( us_udp_msg )
       {
	 map < QString, QString > msging;
	 msging[ "_textarea" ] = QString( "\\nModel: %1 vbar %2 cm^3/g\\n" )
                       .arg( model_vector[i].model_id )
                       .arg( QString("").sprintf("%.3f", model_vector[i].vbar) );
      
	 us_udp_msg->send_json( msging );
	 //sleep(1);
       } 
     accumulated_msgs += QString( "\\nModel: %1 vbar %2 cm^3/g\\n" )
       .arg( model_vector[i].model_id )
       .arg( QString("").sprintf("%.3f", model_vector[i].vbar) );

      current_model = i;

      //cout << "Current_model! " << current_model << endl; 

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
	       QString qs_udp = qs;
	       qs_udp.replace("\n","\\n");
	       us_log->log( qs );
	       if ( us_udp_msg )
		 {
		   map < QString, QString > msging;
		   msging[ "_textarea" ] = qs_udp;
		   
		   us_udp_msg->send_json( msging );
		   //sleep(1);
		 } 
	       accumulated_msgs += qs_udp;
	       
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
                     Rg2 += (double) this_atom->mw * 
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

         us_log->log( qs );
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString( "\\nModel %1 Rg: %2 nm" )
            .arg( model_vector[ i ].model_id )
            .arg( Rg / 10.0, 0, 'f', 2 );
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   } 
	 accumulated_msgs += QString( "\\nModel %1 Rg: %2 nm" )
            .arg( model_vector[ i ].model_id )
            .arg( Rg / 10.0, 0, 'f', 2 );

         //cout << qs << endl << flush;
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
         us_log->log( qs );
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString( "\\nModel: %1 Molecular weight %2 Daltons, Volume (from vbar) %3 A^3%4" )
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
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   } 
	 accumulated_msgs += QString( "\\nModel: %1 Molecular weight %2 Daltons, Volume (from vbar) %3 A^3%4" )
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
	 
         //cout << qs << endl << flush;
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



bool US_Saxs_Util::calc_grid_pdb(bool /* parameters_set_first_model */ )
{
   //    if ( selected_models_contain( "WAT" ) )
   //    {
   //       QMessageBox::warning( this,
   //                             us_tr( "Selected model contains WAT residue" ),
   //                             us_tr( 
   //                                "Can not process models that contain the WAT residue.\n"
   //                                "These are currently generated only for SAXS/SANS computations"
   //                                )
   //                             );
   //       return -1;
   //    }

  // ?? /////////////

   // if ( selected_models_contain( "XHY" ) )
   // {
   //    QMessageBox::warning( this,
   //                          us_tr( "Selected model contains XHY residue" ),
   //                          us_tr( 
   //                             "Can not process models that contain the XHY residue.\n"
   //                             "These are currently generated only for SAXS/SANS computations\n"
   //                             )
   //                          );
   //    return -1;
   // }

   overlap_reduction org_grid_exposed_overlap = grid_exposed_overlap;
   overlap_reduction org_grid_overlap         = grid_overlap;
   overlap_reduction org_grid_buried_overlap  = grid_buried_overlap;
   grid_options      org_grid                 = grid_hydro;

   if ( grid_hydro.create_nmr_bead_pdb )
   {
      if ( 
          (
           grid_exposed_overlap.remove_overlap ||
           grid_overlap.        remove_overlap ||
           grid_buried_overlap. remove_overlap
           ) &&
          (
           grid_exposed_overlap.translate_out ||
           grid_overlap.translate_out         ||
           grid_buried_overlap.translate_out 
           )
          )
      {
         grid_exposed_overlap.translate_out = false;
         grid_overlap.translate_out         = false;
         grid_buried_overlap.translate_out  = false;
         //QString msg = "Temporarily turning off outward translation";
         if ( !grid_hydro.enable_asa )
         {
            grid_hydro.enable_asa = true;
            //msg += " and enabling ASA screening";
         }
      
         // QMessageBox::information( this,
         //                           "US-SOMO: Build AtoB models with structure factors",
         //                           us_tr( msg ) );

         // display_default_differences();
      }
   }

   //stopFlag = false;
   //pb_stop_calc->setEnabled(true);

   model_vector = model_vector_as_loaded;

   // sync_pdb_info( "calc_grid_pdb" );
   // editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );

   us_log->log( QString( us_tr( "Peptide Bond Rule is %1 for this PDB\n\n" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString( us_tr( "Peptide Bond Rule is %1 for this PDB\\n\\n" ) ).arg( misc.pb_rule_on ? "on" : "off" );
       //msging[ "_textarea" ] = QString::fromLatin1("<font color=red>%1</font>").arg(us_tr( "Peptide Bond Rule is "));
       us_udp_msg->send_json( msging );
       //sleep(1);
     } 
   accumulated_msgs += QString( us_tr( "Peptide Bond Rule is %1 for this PDB\\n\\n" ) ).arg( misc.pb_rule_on ? "on" : "off" );
   
   options_log = "";
   append_options_log_atob();
   // display_default_differences();
   // int flag = 0;
   bool any_errors = false;
   bool any_models = false;
   // pb_grid->setEnabled(false);
   // pb_grid_pdb->setEnabled(false);
   // pb_somo->setEnabled(false);
   // pb_somo_o->setEnabled(false);
   // pb_visualize->setEnabled(false);
   // pb_equi_grid_bead_model->setEnabled(false);
   // pb_show_hydro_results->setEnabled(false);
   // pb_calc_hydro->setEnabled(false);
   // pb_calc_zeno->setEnabled(false);

   // if (results_widget)
   // {
   //    results_window->close();
   //    delete results_window;
   //    results_widget = false;
   // }

   bead_model_suffix = getExtendedSuffix(false, false);
   
   // le_bead_model_suffix->setText(bead_model_suffix);
   // if ( !overwrite )
   // {
   //    setSomoGridFile(false);
   // }


   /* CHANGE cycle over current_model to "i"  - DO NOT Check for selections*/

   // for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
   //    if (lb_model->item(current_model)->isSelected()) {

   for ( unsigned int curr_m = 0; curr_m < model_vector.size(); curr_m++  )
     {
       
         // printf("in calc_grid: is selected current model %d\n", current_model); fflush(stdout);
         // {
         QString error_string;
         //    printf("in calc_grid: !somo_processed %d\n", current_model); fflush(stdout);
         //    // create bead model from atoms
         us_log->log(QString("Gridding atom model %1\n").arg(curr_m + 1));
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString("\\nGridding atom model %1\\n").arg(curr_m + 1);
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   } 
	 
	 accumulated_msgs += QString("\\nGridding atom model %1\\n").arg(curr_m + 1);

	 //    qApp->processEvents();
         //    if (stopFlag)
         //    {
         //       us_log->log("Stopped by user\n\n");
         //       pb_grid_pdb->setEnabled(true);
         //       pb_somo->setEnabled(true);
         //       pb_somo_o->setEnabled(true);
         //       progress->reset();
         //       grid_exposed_overlap = org_grid_exposed_overlap;
         //       grid_overlap         = org_grid_overlap;
         //       grid_buried_overlap  = org_grid_buried_overlap;
         //       grid                 = org_grid;
         //       return -1;
         //    }


	 // compute maximum position for progress
         int mppos =
            2 // create beads
            + 4 // grid_atob
            + (grid_hydro.enable_asa ?
               ( 1 
                 + 10 // radial reduction
                 + ( asa_hydro.recheck_beads ? 2 : 0 )
                 )
               : 0 )
            + ( ( !grid_hydro.enable_asa && grid_overlap.remove_overlap ) ?
                ( 10 // radial reduction
                  + ( asa_hydro.recheck_beads ? 2 : 0 )
                  )
                : 0 )
            + 1 // finish off
            ;
         
	 int ppos = 0;
         // progress->reset();
         // progress->setMaximum(mpos);
         // progress->setValue(progress->value() + 1);
        
	 /* Reset Progress bar */
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "progress_output" ] = QString("");
	     msging[ "progress1" ] = QString::number(0.0);
	     
	     us_udp_msg->send_json( msging );
	   }   

	 ppos++;   

 	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( (int(double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
	     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }
	 
	 int retval = create_beads_hydro(&error_string);

	 //	 progress->setValue(progress->value() + 1);

         ppos++;
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int ((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
	     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }

	 // us_qdebug(QString::number(  int( ( double(ppos)/double(mppos) ) * 100.0) ) );
	 
	 //    if (stopFlag)
         //    {
         //       us_log->log("Stopped by user\n\n");
         //       pb_grid_pdb->setEnabled(true);
         //       pb_somo->setEnabled(true);
         //       pb_somo_o->setEnabled(true);
         //       progress->reset();
         //       grid_exposed_overlap = org_grid_exposed_overlap;
         //       grid_overlap         = org_grid_overlap;
         //       grid_buried_overlap  = org_grid_buried_overlap;
         //       grid                 = org_grid;
         //       return -1;
         //    }
         
	 if ( retval )
	   {
             us_log->log("Errors found during the initial creation of beads\n");
	     if ( us_udp_msg )
	       {
		 map < QString, QString > msging;
		 msging[ "_textarea" ] = "Errors found during the initial creation of beads\\n";
		 
		 us_udp_msg->send_json( msging );
		 //sleep(1);
	       } 
	     accumulated_msgs += "Errors found during the initial creation of beads\\n";

             //qApp->processEvents();
             any_errors = true;
             switch ( retval )
	       {
	       case US_SURFRACER_ERR_MISSING_RESIDUE:
	     	 {
                   //printError("Encountered an unknown residue:\n" +
		   //          error_string);
                   break;
	     	 }
	       case US_SURFRACER_ERR_MISSING_ATOM:
	     	 {
                   //printError("Encountered a unknown atom:\n" +
		   //          error_string);
                   break;
	     	 }
	       case US_SURFRACER_ERR_MEMORY_ALLOC:
	     	 {
                   //printError("Encountered a memory allocation error");
                   break;
	     	 }
	       default:
	     	 {
		   // printError("Encountered an unknown error");
                   // unknown error
                   break;
	     	 }
	       }
	   }
	 else
	   {
             if(error_string.length()) {
	       // printError("Encountered unknown atom(s) error:\n" +
	       // 		  error_string);
	       any_errors = true;
             }
             else
	       {
		 // ok, we have the basic "bead" info loaded...
		 
		  unsigned int i = curr_m;
                  bead_model.clear( );
                  bool any_zero_si = false;
                  for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
                     for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
                        PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
                        if(this_atom->active) {
                           for (unsigned int m = 0; m < 3; m++) {
                              this_atom->bead_coordinate.axis[m] = this_atom->coordinate.axis[m];
                           }
                           this_atom->bead_number = bead_model.size();
                           this_atom->bead_computed_radius = this_atom->radius;
                           if ( grid_hydro.hydrate && this_atom->atom_hydration ) 
                           {
                              double additional_radius = 
                                 pow(3 * this_atom->atom_hydration * misc.hydrovol / (M_PI * 4), 1.0/3.0);
                              this_atom->bead_computed_radius += additional_radius;
// #if defined(GRID_HYDRATE_DEBUG)
//                               printf("hydrating atom %s %s %s hydration %f radius %f + %f -> %f\n"
//                                      , this_atom->name.toLatin1().data()
//                                      , this_atom->resName.toLatin1().data()
//                                      , this_atom->resSeq.toLatin1().data()
//                                      , this_atom->atom_hydration
//                                      , this_atom->radius
//                                      , additional_radius
//                                      , this_atom->bead_computed_radius 
//                                      );
// #endif
                           }

                           this_atom->bead_actual_radius = this_atom->bead_computed_radius;
                           this_atom->bead_mw = this_atom->mw;
                           if ( this_atom->si == 0e0 )
                           {
                              any_zero_si = true;
                           }
                           bead_model.push_back(*this_atom);
                        }
                     }
		  }
		  
		  /* Changed current_model for i */
                  //if (bead_models.size() < current_model + 1) {
		  //  bead_models.resize(current_model + 1);
                  //}
		  if (bead_models.size() < curr_m + 1) {
		    bead_models.resize(curr_m + 1);
                  }

                  //progress->setValue( progress->value() + 1 );
                  // int save_progress = progress->value();
                  // int save_total_steps = progress->maximum();
		  ppos++;
		  if ( us_udp_msg )
		    {
		      map < QString, QString > msging;
		      msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
		      msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		      
		      us_udp_msg->send_json( msging );
		      //sleep(1);
		    }
		  
		  
                  if ( grid_hydro.center == 2  && any_zero_si ) // ssi
                  {
                     // editor_msg( "red", "Center of scattering requested, but zero scattering intensities are present" );
                     // pb_grid_pdb->setEnabled(true);
                     // pb_somo->setEnabled(true);
                     // pb_somo_o->setEnabled(true);
		    // progress->reset();
		    if ( us_udp_msg )
		      {
			map < QString, QString > msging;
			msging[ "progress_output" ] = "";
			msging[ "progress1" ] = QString::number(0.0);
			
			us_udp_msg->send_json( msging );
		      //sleep(1);
		      }
		    
		    if ( us_udp_msg )
		      {
			map < QString, QString > msging;
			msging[ "progress_output" ] = "";
			msging[ "progress1" ] = QString::number(0.0);
			
			us_udp_msg->send_json( msging );
			//sleep(1);
		    }
		    grid_exposed_overlap = org_grid_exposed_overlap;
		    grid_overlap         = org_grid_overlap;
		    grid_buried_overlap  = org_grid_buried_overlap;
		    grid_hydro           = org_grid;
		    return -1;
                  }
		  
	
		  /* OMG! ***************************** */
                  bead_models[i] =
                     us_hydrodyn_grid_atob_hydro(&bead_model,
						 &grid_hydro,
						 // progress,
						 // editor,
						 // this);
						 us_log,
						 us_udp_msg,
						 &accumulated_msgs);
		  /* ********************************** */

		  

                  // progress->setMaximum( save_total_steps );
                  // progress->setValue( save_progress + 1 );
		  ppos++;
		  if ( us_udp_msg )
		    {
		      map < QString, QString > msging;
		      msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
		      msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		      
		      us_udp_msg->send_json( msging );
		      //sleep(1);
		    }
		  

                  // if (stopFlag)
                  // {
                  //    us_log->log("Stopped by user\n\n");
                  //    pb_grid_pdb->setEnabled(true);
                  //    pb_somo->setEnabled(true);
                  //    pb_somo_o->setEnabled(true);
                  //    progress->reset();
                  //    grid_exposed_overlap = org_grid_exposed_overlap;
                  //    grid_overlap         = org_grid_overlap;
                  //    grid_buried_overlap  = org_grid_buried_overlap;
                  //    grid                 = org_grid;
                  //    return -1;
                  // }
                  // if (errorFlag)
                  // {
                  //    us_log->log("Error occured\n\n");
                  //    pb_grid_pdb->setEnabled(true);
                  //    pb_somo->setEnabled(true);
                  //    pb_somo_o->setEnabled(true);
                  //    progress->reset();
                  //    grid_exposed_overlap = org_grid_exposed_overlap;
                  //    grid_overlap         = org_grid_overlap;
                  //    grid_buried_overlap  = org_grid_buried_overlap;
                  //    grid                 = org_grid;
                  //    return -1;
                  // }
                  // printf("back from grid_atob 0\n"); fflush(stdout);
		  
                  // if (somo_processed.size() < current_model + 1) {
                  //    somo_processed.resize(current_model + 1);
                  // }
                  // bead_model = bead_models[current_model];


		  if (somo_processed.size() < curr_m + 1) {
                     somo_processed.resize(curr_m + 1);
                  }
                  bead_model = bead_models[curr_m];
		  
		  
                  any_models = true;

                  // somo_processed[current_model] = 1;

		  somo_processed[curr_m] = 1;

// #if defined(DEBUG)
//                   printf("back from grid_atob 1\n"); fflush(stdout);
// #endif


		  /* BEGIN ASA ENABLED **********************************************************************/

                  if ( grid_hydro.enable_asa )
                  {
                     us_log->log("ASA check\n");
		     if ( us_udp_msg )
		       {
			 map < QString, QString > msging;
			 msging[ "_textarea" ] = "ASA check\\n";
		 
			 us_udp_msg->send_json( msging );
			 //sleep(1);
		       } 
		     accumulated_msgs += "ASA check\\n";

                     // qApp->processEvents();
                     // set all beads buried
                     for(unsigned int i = 0; i < bead_model.size(); i++) {
                        bead_model[i].exposed_code = 6;
                        bead_model[i].bead_color = 6;
                        bead_model[i].chain = 1; // all 'side' chain
                     }
                     double save_threshold = asa_hydro.threshold;
                     double save_threshold_percent = asa_hydro.threshold_percent;
                     asa_hydro.threshold = asa_hydro.grid_threshold;
                     asa_hydro.threshold_percent = asa_hydro.grid_threshold_percent;
		     
		     // progress->setValue(progress->value() + 1);
		     ppos++;
		     if ( us_udp_msg )
		       {
			 map < QString, QString > msging;
			 msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			 msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		      
			 us_udp_msg->send_json( msging );
			 //sleep(1);
		       }


		     /* OMG!! */
		     bead_check(true, true);
                     /* *********************** */

		     //progress->setValue(progress->value() + 1);

		     ppos++;
		     if ( us_udp_msg )
		       {
			 map < QString, QString > msging;
			 msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			 msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			 
			 us_udp_msg->send_json( msging );
			 //sleep(1);
		       }

                     asa_hydro.threshold = save_threshold;
                     asa_hydro.threshold_percent = save_threshold_percent;
		     bead_models[curr_m] = bead_model;
		     //bead_models[current_model] = bead_model;
// #if defined(DEBUG)
//                      for(unsigned int i = 0; i < bead_model.size(); i++) {
//                         printf("after asa bead %d exposed %d color %d chain %d active %d mw %f vol %f cr %f [%f,%f,%f]\n",
//                                i,
//                                bead_model[i].exposed_code,
//                                bead_model[i].bead_color,
//                                bead_model[i].chain,
//                                bead_model[i].active,
//                                bead_model[i].bead_ref_mw,
//                                bead_model[i].bead_ref_volume,
//                                bead_model[i].bead_computed_radius,
//                                bead_model[i].bead_coordinate.axis[0],
//                                bead_model[i].bead_coordinate.axis[1],
//                                bead_model[i].bead_coordinate.axis[2]

//                                );
//                      }
// #endif
                     // now apply radial reduction with outward translation using

                     // grid_exposed/buried_overlap
                     overlap_reduction save_sidechain_overlap = sidechain_overlap;
                     overlap_reduction save_mainchain_overlap = mainchain_overlap;
                     overlap_reduction save_buried_overlap = buried_overlap;
                     sidechain_overlap = grid_exposed_overlap;
                     mainchain_overlap = grid_exposed_overlap;
                     buried_overlap = grid_buried_overlap;

                     //progress->setValue(progress->value() + 1);
		     ppos++;
		     if ( us_udp_msg )
		       {
			 map < QString, QString > msging;
			 msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			 msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			 
			 us_udp_msg->send_json( msging );
			 //sleep(1);
		       }
		     
		     //cout << "Before: "  <<  ppos << endl;
                     /* OMG! ******************* */
		     ppos = radial_reduction( true, ppos, mppos );
                     /* ************************* */
		     
		     //cout << "After: "  <<  ppos << endl;

		     sidechain_overlap = save_sidechain_overlap;
                     mainchain_overlap = save_mainchain_overlap;
                     buried_overlap = save_buried_overlap;

		     // bead_models[current_model] = bead_model;
		     bead_models[curr_m] = bead_model;
		     // grid_buried_overlap


		     /* OMG */
		     if (asa_hydro.recheck_beads)
                     {
                        us_log->log("Rechecking beads\n");
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "_textarea" ] = "Rechecking beads\\n";
		 
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  } 
			accumulated_msgs += "Rechecking beads\\n";

                        // qApp->processEvents();
                        double save_threshold = asa_hydro.threshold;
                        double save_threshold_percent = asa_hydro.threshold_percent;
                        asa_hydro.threshold = asa_hydro.grid_threshold;
                        asa_hydro.threshold_percent = asa_hydro.grid_threshold_percent;
                       
			//progress->setValue(progress->value() + 1);
			ppos++;
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			    msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			    
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  }

		     	bead_check(false, false);
                  
		     	//progress->setValue(progress->value() + 1);
			ppos++;
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			    msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			    
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  }
			
                        asa_hydro.threshold = save_threshold;
                        asa_hydro.threshold_percent = save_threshold_percent;
                        //bead_models[current_model] = bead_model;
		     	bead_models[curr_m] = bead_model;
                     }

                  }
                  else
                  {
                     if (grid_overlap.remove_overlap)
                     {
		       //progress->setValue(progress->value() + 1);
                     
		       ppos++;
		       if ( us_udp_msg )
			 {
			   map < QString, QString > msging;
			   msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			   msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			   
			   us_udp_msg->send_json( msging );
			 //sleep(1);
			 }
		       
		       ppos = radial_reduction( true, ppos, mppos );
		
		       // progress->setValue(progress->value() + 1);
		       ppos++;
		       if ( us_udp_msg )
			 {
			   map < QString, QString > msging;
			   msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			 msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			 
			 us_udp_msg->send_json( msging );
			 //sleep(1);
			 }
		       
                        //bead_models[current_model] = bead_model;
			bead_models[curr_m] = bead_model;
                     }

                     // if (stopFlag)
                     // {
                     //    us_log->log("Stopped by user\n\n");
                     //    pb_grid_pdb->setEnabled(true);
                     //    pb_somo->setEnabled(true);
                     //    pb_somo_o->setEnabled(true);
                     //    progress->reset();
                     //    grid_exposed_overlap = org_grid_exposed_overlap;
                     //    grid_overlap         = org_grid_overlap;
                     //    grid_buried_overlap  = org_grid_buried_overlap;
                     //    grid                 = org_grid;
                     //    return -1;
                     // }
                     if (asa_hydro.recheck_beads)
                     {
                        us_log->log("Rechecking beads\n");
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "_textarea" ] = "Rechecking beads\\n";
		 
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  } 
			accumulated_msgs += "Rechecking beads\\n";

                        // qApp->processEvents();
                        // all buried
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 6;
                           bead_model[i].bead_color = 6;
                        }
                        double save_threshold = asa_hydro.threshold;
                        double save_threshold_percent = asa_hydro.threshold_percent;
                        asa_hydro.threshold = asa_hydro.grid_threshold;
                        asa_hydro.threshold_percent = asa_hydro.grid_threshold_percent;

                        //progress->setValue(progress->value() + 1);
			ppos++;
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			    msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			    
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  }
                        bead_check(false, false);
                        //progress->setValue(progress->value() + 1);
			ppos++;
			if ( us_udp_msg )
			  {
			    map < QString, QString > msging;
			    msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
			    msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
			    
			    us_udp_msg->send_json( msging );
			    //sleep(1);
			  }
			
                        asa_hydro.threshold = save_threshold;
                        asa_hydro.threshold_percent = save_threshold_percent;
                        //bead_models[current_model] = bead_model;
			bead_models[curr_m] = bead_model;
                     }
                     else
                     {
                        // all exposed
                        for(unsigned int i = 0; i < bead_model.size(); i++) {
                           bead_model[i].exposed_code = 1;
                           bead_model[i].bead_color = 8;
                        }
			bead_models[curr_m] = bead_model;
                        //bead_models[current_model] = bead_model;
                     }
                     // if (stopFlag)
                     // {
                     //    us_log->log("Stopped by user\n\n");
                     //    pb_grid_pdb->setEnabled(true);
                     //    pb_somo->setEnabled(true);
                     //    pb_somo_o->setEnabled(true);
                     //    progress->reset();
                     //    grid_exposed_overlap = org_grid_exposed_overlap;
                     //    grid_overlap         = org_grid_overlap;
                     //    grid_buried_overlap  = org_grid_buried_overlap;
                     //    grid                 = org_grid;
                     //    return -1;
                     // }
                  }

		  /* END ASA ENABLED **********************************************************************/


		  QString extra_text = "";
                  // if ( grid_hydro.create_nmr_bead_pdb &&
                  //      sf_factors.saxs_name != "undefined" &&
                  //      !sf_factors.saxs_name.isEmpty() )
                  // {
                  //    extra_text = 
                  //       QString( "\nSAXS exponential fitting information\n"
                  //                "    Global average 4 term fit: %1\n"
                  //                "    Global average 5 term fit: %2\n\n" )
                  //       .arg( sf_4term_notes )
                  //       .arg( sf_5term_notes )
                  //       ;

                  //    float tot_excl_vol = 0.0f;
                  //    for ( unsigned int k = 0; k < sf_bead_factors.size(); k++ )
                  //    {
                  //       extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name.toUpper();
                  //       for ( unsigned int i = 0; i < 4; i++ )
                  //       {
                  //          extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a[ i ] );
                  //          extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b[ i ] );
                  //       }
                  //       extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c ).arg( bead_model[ k ].saxs_excl_vol );
                 
                  //       extra_text += "BSAXS:: " + sf_bead_factors[ k ].saxs_name;
                  //       for ( unsigned int i = 0; i < 5; i++ )
                  //       {
                  //          extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].a5[ i ] );
                  //          extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].b5[ i ] );
                  //       }
                  //       extra_text += QString( " %1 %2\n" ).arg( sf_bead_factors[ k ].c5 ).arg( bead_model[ k ].saxs_excl_vol );
                  //       tot_excl_vol += bead_model[ k ].saxs_excl_vol;

                  //       extra_text += "BSAXSV:: " + sf_bead_factors[ k ].saxs_name;
                  //       for ( unsigned int i = 0; i < sf_bead_factors[ k ].vcoeff.size(); i++ )
                  //       {
                  //          extra_text += QString( " %1" ).arg( sf_bead_factors[ k ].vcoeff[ i ] );
                  //       }
                  //       extra_text += QString( " %1\n" ).arg( bead_model[ k ].saxs_excl_vol );
                  //       tot_excl_vol += bead_model[ k ].saxs_excl_vol;
                  //    }

                  //    extra_text += "\nSAXS:: " + sf_factors.saxs_name.toUpper();
                  //    for ( unsigned int i = 0; i < 4; i++ )
                  //    {
                  //       extra_text += QString( " %1" ).arg( sf_factors.a[ i ] );
                  //       extra_text += QString( " %1" ).arg( sf_factors.b[ i ] );
                  //    }
                  //    extra_text += QString( " %1 %2\n" ).arg( sf_factors.c ).arg( tot_excl_vol );
                 
                  //    extra_text += "SAXS:: " + sf_factors.saxs_name;
                  //    for ( unsigned int i = 0; i < 5; i++ )
                  //    {
                  //       extra_text += QString( " %1" ).arg( sf_factors.a5[ i ] );
                  //       extra_text += QString( " %1" ).arg( sf_factors.b5[ i ] );
                  //    }
                  //    extra_text += QString( " %1 %2\n" ).arg( sf_factors.c5 ).arg( tot_excl_vol );

                  //    if ( extra_saxs_coefficients.count( sf_factors.saxs_name ) )
                  //    {
		  //      //editor_msg( "dark red", QString( "Notice: extra saxs coefficients %1 replaced\n" ).arg( sf_factors.saxs_name ) );
                  //    } else {
                  //       saxs_options_hydro.dummy_saxs_names.push_back( sf_factors.saxs_name );
                  //    }
                  //    extra_saxs_coefficients[ sf_factors.saxs_name ] = sf_factors;
                  //    saxs_options_hydro.dummy_saxs_name = sf_factors.saxs_name;
                  //    //editor_msg( "blue", QString( "Saxs name for dummy atom models set to %1" ).arg( saxs_options_hydro.dummy_saxs_name ) );

                  //    // if ( saxs_map.count( sf_factors.saxs_name ) )
                  //    // {
                  //    //    editor_msg( "dark red", 
                  //    //                QString( us_tr( "Notice: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                  //    //                .arg( sf_factors.saxs_name ) );
                  //    // } else {
                  //    //    saxs_list.push_back( sf_factors );
                  //    //    editor_msg( "dark blue", 
                  //    //                QString( us_tr( "Notice: added coefficients for %1 from newly loaded values\n" ) )
                  //    //                .arg( sf_factors.saxs_name ) );
                  //    // }
               
		  //    /* Changed */
		  //    //saxs_util->saxs_map[ sf_factors.saxs_name ] = sf_factors;
		  //    saxs_map[ sf_factors.saxs_name ] = sf_factors;
		     
                  //    // if ( saxs_plot_widget )
                  //    // {
                  //    //    if ( saxs_plot_window->saxs_map.count( sf_factors.saxs_name ) )
                  //    //    {
                  //    //       editor_msg( "dark red", 
                  //    //                   QString( us_tr( "Notice: saxs window: saxs coefficients for %1 replaced by newly loaded values\n" ) )
                  //    //                   .arg( sf_factors.saxs_name ) );
                  //    //    } else {
                  //    //       saxs_plot_window->saxs_list.push_back( sf_factors );
                  //    //       editor_msg( "dark blue", 
                  //    //                   QString( us_tr( "Notice: saxs window: added coefficients for %1 from newly loaded values\n" ) )
                  //    //                   .arg( sf_factors.saxs_name ) );
                  //    //    }
                  //    //    saxs_plot_window->saxs_map[ sf_factors.saxs_name ] = sf_factors;
                  //    // }
                  // }

                  // if ( saxs_options_hydro.compute_saxs_coeff_for_bead_models && grid_hydro.create_nmr_bead_pdb )
                  // {
                  //    if ( !saxs_options_hydro.iq_global_avg_for_bead_models && sf_bead_factors.size() != bead_model.size() )
                  //    {
                  //       // editor_msg( "red", 
                  //       //             QString( us_tr( "Overriding setting to use global structure factors since bead model doesn't contain the correct number of structure factors (%1) for the beads (%2)" ) )
                  //       //             .arg( sf_bead_factors.size() )
                  //       //             .arg( bead_model.size() )
                  //       //             );
                  //    }
                  //    if ( saxs_options_hydro.iq_global_avg_for_bead_models || sf_bead_factors.size() != bead_model.size() )
                  //    {
                  //       if ( !saxs_map.count( saxs_options_hydro.dummy_saxs_name ) )
                  //       {
                  //          // editor_msg( "red", QString( us_tr("Warning: No '%1' SAXS atom found.\n" ) )
                  //          //             .arg( saxs_options_hydro.dummy_saxs_name ) );
                  //          for(unsigned int i = 0; i < bead_model.size(); i++) {
                  //             bead_model[i].saxs_data.saxs_name = "";
                  //          }
                  //       } else {
                  //          // editor_msg( "blue", QString( us_tr("Notice: Loading beads with saxs coefficients '%1'" ) )
                  //          //             .arg( saxs_options_hydro.dummy_saxs_name ) );
                  //          for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                  //          {
                  //             bead_model[i].saxs_name = saxs_options_hydro.dummy_saxs_name;
                  //             bead_model[i].saxs_data = saxs_map[ saxs_options_hydro.dummy_saxs_name ];
                  //             bead_model[i].hydrogens = 0;
                  //          }
                  //       }
                  //    } else {
                  //       if ( !saxs_options_hydro.iq_global_avg_for_bead_models && sf_bead_factors.size() == bead_model.size() )
                  //       {
		  // 	  //editor_msg( "blue", us_tr("Notice: Loading beads with bead computed structure factors" ) );
                  //          for( unsigned int i = 0; i < bead_model.size(); i++ ) 
                  //          {
                  //             bead_model[i].saxs_name = sf_bead_factors[ i ].saxs_name;
                  //             bead_model[i].saxs_data = sf_bead_factors[ i ];
                  //             bead_model[i].hydrogens = 0;
                  //          }
                  //       }
                  //    }
                  //    bead_models[current_model] = bead_model;
                  // }                    

                  us_log->log( QString( "Volume of bead model %1\n" ).arg( total_volume_of_bead_model( bead_model ) ) );
		  if ( us_udp_msg )
		    {
		      map < QString, QString > msging;
		      msging[ "_textarea" ] = QString( "Volume of bead model %1\\n" ).arg( total_volume_of_bead_model( bead_model ) );
		      
		      us_udp_msg->send_json( msging );
		      //sleep(1);
		    } 
		  accumulated_msgs += QString( "Volume of bead model %1\\n" ).arg( total_volume_of_bead_model( bead_model ) );

                  // progress->setValue(progress->value() + 1);

		  ppos++;
		  if ( us_udp_msg )
		    {
		      map < QString, QString > msging;
		      msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
		      msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		      
		      us_udp_msg->send_json( msging );
		      //sleep(1);
		    }
		  
                  // // write_bead_spt(somo_dir + SLASH + project +
                  // //       (bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
                  // //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
                  // //       DOTSOMO, &bead_model, bead_model_from_file);




		  /* NEED TO WRITE IT DOWN ************************************************************************/

                   QString filename = 
                      project + 
                      QString( "_%1" ).arg( curr_m + 1 );

		   // le_bead_model_file->setText( filename );

                   write_bead_model( 
                                    //somo_dir + SLASH + filename +
                                    filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                                    DOTSOMO, 
                                    &bead_model,
                                    extra_text
                                    );
		   
		   //QStringList list_of_models; 
		   list_of_models << filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
		     DOTSOMO + ".bead_model";
		   
		   write_pdb_hydro( filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
                                    DOTSOMO, &bead_model );
		   

		  /* NEED TO WRITE IT DOWN ************************************************************************/

	 // if ( parameters_set_first_model )
	 //   break;
	 
	       }

	   }
     }
// //progress->setValue(progress->maximum());

   
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number(100)).arg(100); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(1.0);
		      
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
   
   // }
   //}
   

   grid_exposed_overlap = org_grid_exposed_overlap;
   grid_overlap         = org_grid_overlap;
   grid_buried_overlap  = org_grid_buried_overlap;
   grid_hydro           = org_grid;

   // if (stopFlag)
   // {
   //    us_log->log("Stopped by user\n\n");
   //    pb_grid_pdb->setEnabled(true);
   //    pb_somo->setEnabled(true);
   //    pb_somo_o->setEnabled(true);
   //    progress->reset();
   //    return -1;
   // }

   if (any_models && !any_errors)
   {
      us_log->log("Build bead model completed\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Build bead model completed\\n";
	  
	  us_udp_msg->send_json( msging );
		      //sleep(1);
	} 
      accumulated_msgs += "Build bead model completed\\n";

      // qApp->processEvents();
      // pb_visualize->setEnabled(true);
      // pb_equi_grid_bead_model->setEnabled(true);
      // pb_calc_hydro->setEnabled(true);
      // pb_calc_zeno->setEnabled(true);
      // pb_bead_saxs->setEnabled(true);
      // pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
     us_log->log("Errors encountered\n");
     if ( us_udp_msg )
       {
	 map < QString, QString > msging;
	 msging[ "_textarea" ] = "Errors encountered\\n";
	 
	 us_udp_msg->send_json( msging );
	 //sleep(1);
       } 
     accumulated_msgs += "Errors encountered\\n";
   }

   // pb_grid_pdb->setEnabled(true);
   // pb_grid->setEnabled(true);
   // pb_somo->setEnabled(true);
   // pb_somo_o->setEnabled(true);
   // pb_stop_calc->setEnabled(false);


   /* ???????????? */

   // if (calcAutoHydro)
   // {
   //    calc_hydro();
   // }
   // else
   // {
   //    play_sounds(1);
   // }

   //return (flag);
   return true;
}

void US_Saxs_Util::bead_check( bool use_threshold, bool message_type )
{
   // recheck beads here

   //printf("bead recheck use threshold%s\n", use_threshold ? "" : " percent");
   active_atoms.clear( );
   for(unsigned int i = 0; i < bead_model.size(); i++) {
      active_atoms.push_back(&bead_model[i]);
   }

   QString error_string = "";
   int retval = us_hydrodyn_asab1_main_hydro(active_atoms,
                                       &asa_hydro,
                                       &results_hydro,
					     true ); // ,
                                       // progress,
                                       // editor,
                                       // this
                                       // );
   // if (stopFlag)
   // {
   //    return;
   // }
   if ( retval ) {
      switch ( retval ) {
      case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
         {
	   //printError("US_HYDRODYN_ASAB1 on bead recheck encountered a memory allocation error");
            fprintf(stderr, "bead recheck: memory alloc error\n");
            return;
            break;
         }
      default:
         {
            // unknown error
            //printError("US_HYDRODYN_ASAB1 encountered an unknown error");
            fprintf(stderr, "bead recheck: unknown error %d\n", retval);
            return;
            break;
         }
      }
   }

   int b2e = 0;
#if defined(EXPOSED_TO_BURIED)
   int e2b = 0;
#endif

   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) + "_pre_recheck" + DOTSOMO, &bead_model);
   //  write_bead_model(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) + "_pre_recheck" + DOTSOMO, &bead_model );

   for (unsigned int i = 0; i < bead_model.size(); i++)
   {
      float surface_area =
         (asa_hydro.probe_radius + bead_model[i].bead_computed_radius) *
         (asa_hydro.probe_radius + bead_model[i].bead_computed_radius) * 4 * M_PI;
      QString msg = "";
      if( use_threshold ?
          ( bead_model[i].bead_recheck_asa > asa_hydro.threshold )        
          :
          ( bead_model[i].bead_recheck_asa > (asa_hydro.threshold_percent / 100.0) * surface_area )
          )
      {
         // now exposed
         if(bead_model[i].exposed_code != 1) {
            // was buried
            msg = "buried->exposed";
            b2e++;
            bead_model[i].exposed_code = 1;
            bead_model[i].bead_color = 8;
         }
      }
#if defined(EXPOSED_TO_BURIED)
      else {
         // now buried
         if(bead_model[i].exposed_code == 1) {
            // was exposed
            msg = "exposed->buried";
            e2b++;
            bead_model[i].exposed_code = 6;
            bead_model[i].bead_color = 6;
         }
      }
#endif

#if defined(DEBUG)
      printf("bead %d %.2f %.2f %.2f %s %s bead mw %.2f bead ref mw %.2f\n",
             i,
             bead_model[i].bead_computed_radius,
             surface_area,
             bead_model[i].bead_recheck_asa,
             (
              use_threshold ?
              ( bead_model[i].bead_recheck_asa > asa_hydro.threshold )        
              :
              ( bead_model[i].bead_recheck_asa > (asa_hydro.threshold_percent / 100.0) * surface_area )
              ) ?
             "exposed" : "buried",
             msg.toLatin1().data(),
             bead_model[i].bead_mw,
             bead_model[i].bead_ref_mw
             );
#endif
   }
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) +
   //         QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //       DOTSOMO, &bead_model);


   /* WRITE BEAD MODEL - COMMENT FOR NOW **************************************************************************** */
   // QString filename = 
   //   project + 
   //   QString( "_%1" ).arg(  current_model  );

   // // le_bead_model_file->setText( filename );
   
   // write_bead_model(//somo_dir + SLASH + 
   // 		    project + QString("_%1").arg( model_name( current_model ) ) +
   //                QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //                DOTSOMO, &bead_model);



 #if defined(EXPOSED_TO_BURIED)
    us_log->log(QString("%1 exposed beads became buried\n").arg(e2b));
    if ( us_udp_msg )
      {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("%1 exposed beads became buried\\n").arg(e2b); 
       
       us_udp_msg->send_json( msging );
       //sleep(1);
      } 
    accumulated_msgs += QString("%1 exposed beads became buried\\n").arg(e2b); 
#endif
    if ( message_type ) 
      {
  us_log->log(QString(us_tr("%1 beads are exposed\n")).arg(b2e));
  if ( us_udp_msg )
    {
      map < QString, QString > msging;
      msging[ "_textarea" ] = QString(us_tr("%1 beads are exposed\\n")).arg(b2e);
      
      us_udp_msg->send_json( msging );
      //sleep(1);
     } 
    accumulated_msgs += QString(us_tr("%1 beads are exposed\\n")).arg(b2e);
} 
    else
    {
       us_log->log(QString(us_tr("%1 previously buried beads are exposed by rechecking\n")).arg(b2e));
       if ( us_udp_msg )
	 {
          map < QString, QString > msging;
	  msging[ "_textarea" ] = QString(us_tr("%1 previously buried beads are exposed by rechecking\\n")).arg(b2e); 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	 } 
       accumulated_msgs += QString(us_tr("%1 previously buried beads are exposed by rechecking\\n")).arg(b2e); 
    }
}


//--------- radial reduction for beads ---------------------------------------

#define TOLERANCE overlap_tolerance

# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0


static void outward_translate_2_spheres(float *r1, // radius of sphere 1
                                        float *r2, // radius of sphere 2
                                        float *x1, // center of sphere 1
                                        float *x2, // center of sphere 2
                                        float *v1, // normalized vector to sphere 1 from COG
                                        float *v2  // normalized vector to sphere 1 from COG
                                        ) {
#if defined(DEBUG) || defined(DEBUG1)
   printf("outward_translate_2_spheres\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f] cog vector [%.3f,%.3f,%.3f]\n",
          *r1, x1[0], x1[1], x1[2], v1[0], v1[1], v1[2],
          *r2, x2[0], x2[1], x2[2], v2[0], v2[1], v2[2]);
#endif

   float k;
   float denom =
      2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) -
      *r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -
      *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);

   float sfact =
      sqrt(
           (double) (*r1 + *r2) * (*r1 + *r2) *
           ((double) (*r1 * *r1 * (-1 + v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) -

             2 * *r1 * *r2 * (1 + v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) +

             *r2 * *r2 * (-1 + v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2])) *

            ((double) (*r1 + *r2) * (*r1 + *r2) -
             (double) (x1[0] - x2[0]) * (x1[0] - x2[0]) -
             (double) (x1[1] - x2[1]) * (x1[1] - x2[1]) -
             (double) (x1[2] - x2[2]) * (x1[2] - x2[2])) +

            pow(*r1 * *r1 +
                *r1 * (
                       2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
                       v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
                *r2 * (
                       *r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] +
                       v2[0] * x2[0] + v2[1] * x2[1] + v2[2] * x2[2]),2)));

   float b =
      (*r1 + *r2) *
      (*r1 * *r1 +
       *r1 * (2 * *r2 + v1[0] * x1[0] + v1[1] * x1[1] + v1[2] * x1[2] -
              v1[0] * x2[0] - v1[1] * x2[1] - v1[2] * x2[2]) +
       *r2 * (*r2 - v2[0] * x1[0] - v2[1] * x1[1] - v2[2] * x1[2] + v2[0] * x2[0] +
              v2[1] * x2[1] + v2[2] * x2[2]));

   k = (b - sfact) / denom;

   float k1 = k * *r1 / (*r1 + *r2);
   float k2 = k * *r2 / (*r1 + *r2);

   x1[0] += k1 * v1[0];
   x1[1] += k1 * v1[1];
   x1[2] += k1 * v1[2];

   x2[0] += k2 * v2[0];
   x2[1] += k2 * v2[1];
   x2[2] += k2 * v2[2];

   *r1 -= k1;
   *r2 -= k2;

#if defined(DEBUG) || defined(DEBUG1)

   printf("k %.3f\n", k);

   printf("new coordinates, radius\n"
          "sphere 1 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "sphere 2 radius %.3f center [%.3f,%.3f,%.3f]\n"
          "r1 + r2 - distance between spheres = %f\n",
          *r1,
          x1[0],
          x1[1],
          x1[2],
          *r2,
          x2[0],
          x2[1],
          x2[2],
          *r1 + *r2 -
          sqrt(
               (x1[0] - x2[0]) * (x1[0] - x2[0]) +
               (x1[1] - x2[1]) * (x1[1] - x2[1]) +
               (x1[2] - x2[2]) * (x1[2] - x2[2]))
          );
#endif
}



int US_Saxs_Util::radial_reduction( bool from_grid, int use_ppos, int mppos )
{
   // popping radial reduction

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;
   //int end_progress = progress->value() + 10;
   int ppos = use_ppos;
   // cout << ppos << endl;

   int end_progress = use_ppos + 10;

   //cout << QString("radial reduction, tolerance is %1\n").arg(TOLERANCE);

   for (unsigned int i = 0; i < bead_model.size(); i++) {
      PDB_atom *this_atom = &bead_model[i];
      if (this_atom->active) {
         molecular_mw += this_atom->bead_mw;
         for (unsigned int m = 0; m < 3; m++) {
            molecular_cog[m] += this_atom->bead_coordinate.axis[m] * this_atom->bead_mw;
         }
      }
   }
   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
   } 
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or grid fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);
#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   //   if (no_rr) {
   //      methods[0] = 0;
   //      methods[1] = 0;
   //      methods[2] = 0;
   //      methods[3] = 0;
   //   }

   if (sidechain_overlap.fuse_beads) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if (mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if (buried_overlap.fuse_beads) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if (grid_overlap.fuse_beads) {
      methods[3] |= POP_ALL | POP_BURIED;
   }

   if (sidechain_overlap.remove_hierarch) {
      methods[0] |= RR_HIERC;
   }

   if (mainchain_overlap.remove_hierarch) {
      methods[1] |= RR_HIERC;
   }

   if (buried_overlap.remove_hierarch) {
      methods[2] |= RR_HIERC;
   }

   if (grid_overlap.remove_hierarch) {
      methods[3] |= RR_HIERC;
   }

   if (sidechain_overlap.translate_out) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if (mainchain_overlap.translate_out) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if (buried_overlap.translate_out) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if (grid_overlap.translate_out) {
      methods[3] |= OUTWARD_TRANSLATION;
   }

   if (!sidechain_overlap.remove_overlap) {
      methods[0] = 0;
   }

   if (!mainchain_overlap.remove_overlap) {
      methods[1] = 0;
   }

   if (!buried_overlap.remove_overlap) {
      methods[2] = 0;
   }

   if (!grid_overlap.remove_overlap) {
      methods[3] = 0;
   }

   float overlap[] =
      {
         (float) ( sidechain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( mainchain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( buried_overlap.fuse_beads_percent / 100.0 ),
         (float) ( grid_overlap.fuse_beads_percent / 100.0 )
      };

   float rr_overlap[] =
      {
         (float)( ( sidechain_overlap.remove_hierarch ?
                    sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( mainchain_overlap.remove_hierarch ?
                    mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( buried_overlap.remove_hierarch ?
                    buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0 ),
         (float)( ( grid_overlap.remove_hierarch ?
                    grid_overlap.remove_hierarch_percent : grid_overlap.remove_sync_percent) / 100.0 )
      };


// #if defined(WRITE_EXTRA_FILES)
//    write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
//    write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
// #endif
   for ( unsigned int k = ( grid_hydro.enable_asa ? 0 : 3 ); 
         k < (unsigned int)( grid_hydro.enable_asa ? 3 : 4 );
         k++ )
   {
      // only grid method
      if ( !methods[k] ||
           ( ( from_grid || grid_hydro.enable_asa ) && k == 1 ) )
      {
	//printf("skipping stage %d\n", k);
         continue;
      }

   stage_loop:

      // qApp->processEvents();

      // if (stopFlag)
      // {
      //    return;
      // }
      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
       printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
              k,
              (methods[k] & POP_MC) ? "main chain " : "",
              (methods[k] & POP_SC) ? "side chain " : "",
              (methods[k] & POP_MCSC) ? "main & side chain " : "",
              (methods[k] & POP_EXPOSED) ? "exposed " : "",
              (methods[k] & POP_BURIED) ? "buried " : "",
              (methods[k] & POP_ALL) ? "all " : "",
              (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
              (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
              (methods[k] & RR_MC) ? "main chain " : "",
              (methods[k] & RR_SC) ? "side chain " : "",
              (methods[k] & RR_MCSC) ? "main & side chain " : "",
              (methods[k] & RR_EXPOSED) ? "exposed " : "",
              (methods[k] & RR_BURIED) ? "buried " : "",
              (methods[k] & RR_ALL) ? "all " : "",
              (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
              overlap[k],
              rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
      // unsigned iter = 0;
      bool overlaps_exist;
// #if defined(TIMING)
//       gettimeofday(&start_tv, NULL);
// #endif
      us_log->log(QString("Begin popping stage %1\n").arg(k + 1));
      if ( us_udp_msg )
	{
          map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("Begin popping stage %1\\n").arg(k + 1);
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}  
      accumulated_msgs += QString("Begin popping stage %1\\n").arg(k + 1);

//       progress->setValue(progress->value() + 1); 
//       qApp->processEvents();
      
      ppos++;
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
	  msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}

      do {
         // qApp->processEvents();
         // if (stopFlag)
         // {
         //    return;
         // }
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
	 // int k_tot = (unsigned int)( grid_hydro.enable_asa ? 3 : 4);
	 // if ( us_udp_msg )
	 //   {
	 //     map < QString, QString > msging;
	 //     msging[ "progress_output" ] =  QString("Stage %1 popping iteration %2").arg(k+1).arg(iter);
	 //     msging[ "progress1" ] = QString::number(double(k+1)/double(k_tot));
	 //     //msging[ "_progress" ] = QString::number(double(i+1)/double(npoints_x));
	     
	 //     us_udp_msg->send_json( msging );
	 //     //sleep(1);
	 //   }
         // lbl_core_progress->setText(QString("Stage %1 popping iteration %2").arg(k+1).arg(iter));
         // qApp->processEvents();

         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
		  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
	       
	       // if ( us_udp_msg )
	       // 	 {
	       // 	   map < QString, QString > msging;
	       // 	   msging[ "progress_output" ] = QString("Gridding %1 of %2").arg(i+1).arg(npoints_x);
	       // 	   msging[ "progress1" ] = QString::number(double(i+1)/double(npoints_x));
	       // 	   //msging[ "_progress" ] = QString::number(double(i+1)/double(npoints_x));
		   
	       // 	   us_udp_msg->send_json( msging );
	       // 	   //sleep(1);
	       // 	 }
               // lbl_core_progress->setText(QString("Stage %1 popping iteration %2 beads popped %3").arg(k+1).arg(iter).arg(beads_popped));
               // qApp->processEvents();
               // //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *= bead_model[max_bead1].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * bead_model[max_bead2].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] /= bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               }
               bead_model[max_bead1].bead_ref_mw = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_volume = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
		 us_log->log(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "_textarea" ] = QString("Beads popped %1, Go back to stage %2\\n").arg(beads_popped).arg(k);
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1);
		   } 
		 accumulated_msgs += QString("Beads popped %1, Go back to stage %2\\n").arg(beads_popped).arg(k);
		 
		 // printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
		 k = 0;
		 //progress->setValue(progress->value() - 4);
		 
		 ppos -= 4;
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
		     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1);
		   }
		 
		 goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);

//          qApp->processEvents();
//          if (stopFlag)
//          {
//             return;
//          }

   } while(overlaps_exist);

// #if defined(TIMING)
//       gettimeofday(&end_tv, NULL);
//       printf("popping %d time %lu\n",
//              k,
//              1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
//              start_tv.tv_usec);
//       fflush(stdout);
// #endif

// #if defined(WRITE_EXTRA_FILES)
//       write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
//       write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
// #endif
//       printf("stage %d beads popped %d\n", k, beads_popped);
//       progress->setValue(progress->value() + 1);

      ppos++;
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
	  msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}

      
      us_log->log(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("Beads popped %1.\\nBegin radial reduction stage %2\\n").arg(beads_popped).arg(k + 1); 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}   
      accumulated_msgs += QString("Beads popped %1.\\nBegin radial reduction stage %2\\n").arg(beads_popped).arg(k + 1); 

//       qApp->processEvents();

      // radial reduction phase

// #if defined(TIMING)
//       gettimeofday(&start_tv, NULL);
// #endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif

	    // if ( us_udp_msg )
	    //   {
	    // 	map < QString, QString > msging;
	    // 	msging[ "progress_output" ] = QString("Stage %1 hierarchical radial reduction").arg(k+1);
	    // 	msging[ "progress1" ] = QString::number(double(k + 1)/double(npoints_x));
	    // 	//msging[ "_progress" ] = QString::number(double(i+1)/double(npoints_x));
		
	    // 	us_udp_msg->send_json( msging );
	    // 	//sleep(1);
	    //   }
	       
            // lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            // qApp->processEvents();

            max_intersection_length = 0;
            pairs.clear( );
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f %s %s %s %s\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length,
                         (methods[k] & RR_BURIED) ? "RR_BURIED" : "!RR_BURIED",
                         (methods[k] & RR_SC) ? "RR_SC" : "!RR_SC",
                         (methods[k] & RR_MCSC) ? "RR_MCSC" : "!RR_MCSC",
                         (bead_model[i].active &&
                          bead_model[j].active &&
                          (methods[k] & RR_MCSC ||
                           ((methods[k] & RR_SC) &&
                            bead_model[i].chain == 1 &&
                            bead_model[j].chain == 1)) &&
                          ((methods[k] & RR_BURIED) ||
                           (bead_model[i].exposed_code == 1 &&
                            bead_model[j].exposed_code == 1)) &&
                          bead_model[i].bead_computed_radius > TOLERANCE &&
                          bead_model[j].bead_computed_radius > TOLERANCE ) ? "logic true" : "logic false"
                         );
#endif
                  //                  bool active = bead_model[i].active && bead_model[j].active;
                     
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (
                       methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)
                       ) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }

            // ok, now we have the list of pairs

            // lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            // qApp->processEvents();

            max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter);
#endif

               // lbl_core_progress->setText(QString("Stage %1 hierarch. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               // qApp->processEvents();

               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }
#if defined(DEBUG) 
               printf("1: max intersection length %f\n", max_intersection_length);
#endif


               if (max_intersection_length > TOLERANCE) {
                  count++;
#if defined(DEBUG) 
                  printf("1: count %d\n", count);
#endif
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
#if defined(DEBUG3) 
                     puts("rr0");
#endif
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
#if defined(DEBUG3) 
                        puts("rr1");
#endif
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
#if defined(DEBUG3) 
                           puts("rr2");
#endif
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           } 
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
#if defined(DEBUG3) 
                           printf("rr4 notv %d\n", bead_model[use_bead].normalized_ot_is_valid);
#endif
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              } 
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
#if defined(DEBUG3) 
                        puts("rr5");
#endif
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           // exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           // exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
#if defined(DEBUG3) 
                           puts("rr6");
#endif
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
	       }// if max intersection length > TOLERANCE

               // qApp->processEvents();
               // if (stopFlag)
               // {
               //    return;
               // }

            } while(count);
#if defined(DEBUG2)
            printf("out of while 2 count = %d\n", count);
#endif
         }
         else 
         {
            // simultaneous reduction

            // lbl_core_progress->setText(QString("Stage %1 sychronous radial reduction").arg(k+1));
            // qApp->processEvents();

// #if defined(USE_THREADS)
//             unsigned int threads = numThreads;
//             vector < radial_reduction_thr_t* > radial_reduction_thr_threads(threads);
//             for ( unsigned int j = 0; j < threads; j++ )
//             {
//                radial_reduction_thr_threads[j] = new radial_reduction_thr_t(j);
//                radial_reduction_thr_threads[j]->start();
//             }
// #endif

            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               //printf("processing synchronous radial reduction iteration %d\n", iter);
               us_log->log(QString(" %1").arg(iter));
	       if ( us_udp_msg )
		 {
		   map < QString, QString > msging;
		   msging[ "_textarea" ] = QString(" %1").arg(iter);
		   
		   us_udp_msg->send_json( msging );
		   //sleep(1);
		 }   
	       accumulated_msgs += QString(" %1").arg(iter);
#endif
               //   lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction iteration %2").arg(k+1).arg(iter));
               //               qApp->processEvents();
               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear( );
               count = 0;
               reduced[bead_model.size() - 1] = false;

// #if defined(USE_THREADS)
//                {
//                   unsigned int i;
//                   unsigned int j;

//                   vector < BPair > my_pairs[ MAX_THREADS ];
//                   for ( j = 0; j < threads; j++ )
//                   {
// # if defined(DEBUG_THREAD)
//                      cout << "thread " << j << endl;
// # endif
//                      radial_reduction_thr_threads[j]->radial_reduction_thr_setup(
//                                                         methods[k],
//                                                         &bead_model,
//                                                         &last_reduced,
//                                                         &reduced,
//                                                         &my_pairs[j],
//                                                         threads,
//                                                         overlap_tolerance
//                                                         );

//                   }

//                   for ( j = 0; j < threads; j++ )
//                   {
//                      radial_reduction_thr_threads[j]->radial_reduction_thr_wait();
//                   }

//                   // merge results
//                   for ( j = 0; j < threads; j++ )
//                   {
//                      for ( i = 0; i < my_pairs[j].size(); i++ ) 
//                      {
//                         pairs.push_back(my_pairs[j][i]);
                        
//                         if (my_pairs[j][i].separation > max_intersection_length) {
//                            max_intersection_length = my_pairs[j][i].separation;
//                            max_bead1 = my_pairs[j][i].i;
//                            max_bead2 = my_pairs[j][i].j;
//                         }
//                         count++;
//                      }
//                   }
//                }

	   
// #else // !defined(USE_THREADS)
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
#endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

#if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
#endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  } // if last_reduced[i]
               }
// #endif // !defined(USE_THERADS)

#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               
	       // lbl_core_progress->setText(QString("Stage %1 synchron. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               // qApp->processEvents();
 
	       if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (k == 3 ||
                          (
                           (bead_model[pairs[i].i].exposed_code != 1 ||
                            methods[k] & RR_EXPOSED ||
                            methods[k] & RR_ALL) &&
                           (!(methods[k] & RR_MCSC) ||
                            bead_model[pairs[i].i].chain == 0 ||
                            (methods[k] & RR_BURIED &&
                             bead_model[pairs[i].i].exposed_code != 1))
                           ) )
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                  }
               }
               last_reduced = reduced;
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
	       //               qApp->processEvents();

//                if (stopFlag)
//                {
// #if defined(USE_THREADS)
//                   {
//                      unsigned int j;
//                      // destroy
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
//                      }
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         radial_reduction_thr_threads[j]->wait();
//                      }
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         delete radial_reduction_thr_threads[j];
//                      }
//                   }
// #endif
//                   return;
//                }
            } while(count);


// #if defined(USE_THREADS)
//             {
//                unsigned int j;
//                // destroy
               
//                for ( j = 0; j < threads; j++ )
//                {
//                   radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
//                }
               
//                for ( j = 0; j < threads; j++ )
//                {
//                   radial_reduction_thr_threads[j]->wait();
//                }
               
//                for ( j = 0; j < threads; j++ )
//                {
//                   delete radial_reduction_thr_threads[j];
//                }
//             }
// #endif

         }

// #if defined(TIMING)
//          gettimeofday(&end_tv, NULL);
//          printf("radial reduction %d time %lu\n",
//                 k,
//                 1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
//                 start_tv.tv_usec);
//          fflush(stdout);
// #endif

         // progress->setValue(progress->value() + 1);
         // qApp->processEvents();
	 ppos++;
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
	     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }


         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif


// #if defined(WRITE_EXTRA_FILES)
//       write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
//       write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
// #endif
      
   } // methods


// #if defined(WRITE_EXTRA_FILES)
//    write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
//    write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
// #endif
//    //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg(current_model + 1) +
//    //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
//    //       DOTSOMO, &bead_model);

//    QString filename = 
//       project + 
//       QString( "_%1" ).arg( model_name( current_model ) );

//    le_bead_model_file->setText( filename );

//    write_bead_model(somo_dir + SLASH + filename +
//                     QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") + DOTSOMO
//                     , &bead_model);
   us_log->log("Finished with popping and radial reduction\n");
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = "Finished with popping and radial reduction\\n"; 
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }   
    accumulated_msgs += "Finished with popping and radial reduction\\n"; 

//    progress->setValue(end_progress); 
   
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("AtoB model calculation: %1\%").arg(QString::number( int((double(end_progress)/double(mppos))*100.0) ) ).arg(100); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(end_progress)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

    return ppos;
   
}
//------------------------------ end of radial reduction ------------------------------------------------------



int US_Saxs_Util::overlap_check(bool sc, bool mc, bool buried, double tolerance, int limit )
{
   int retval = 0;
#if defined(DEBUG_OVERLAP)
   printf("overlap checking--overlap tolerance %f--%s-%s-%s--------------------\n",
          tolerance,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "");
#endif
   for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
      for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG_OVERLAP_2)
         if(i == 2 && (j == 34 || j == 60))
            printf("x1 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                   i, j,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[j].bead_computed_radius,
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   bead_model[i].bead_computed_radius +
                   bead_model[j].bead_computed_radius -
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2)),
                   sqrt(
                        pow(bead_model[i].bead_coordinate.axis[0] -
                            bead_model[j].bead_coordinate.axis[0], 2) +
                        pow(bead_model[i].bead_coordinate.axis[1] -
                            bead_model[j].bead_coordinate.axis[1], 2) +
                        pow(bead_model[i].bead_coordinate.axis[2] -
                            bead_model[j].bead_coordinate.axis[2], 2))
                   <= tolerance ? "ok" : "needs reduction"
                   );
#endif
         if (bead_model[i].active &&
             bead_model[j].active
             &&
             (mc ||
              (sc &&
               bead_model[i].chain == 1 &&
               bead_model[j].chain == 1))
             &&
             (buried ||
              (bead_model[i].exposed_code == 1 &&
               bead_model[j].exposed_code == 1)) &&
             bead_model[i].bead_computed_radius > tolerance &&
             bead_model[j].bead_computed_radius > tolerance
             ) {
            float separation =
               bead_model[i].bead_computed_radius +
               bead_model[j].bead_computed_radius -
               sqrt(
                    pow(bead_model[i].bead_coordinate.axis[0] -
                        bead_model[j].bead_coordinate.axis[0], 2) +
                    pow(bead_model[i].bead_coordinate.axis[1] -
                        bead_model[j].bead_coordinate.axis[1], 2) +
                    pow(bead_model[i].bead_coordinate.axis[2] -
                        bead_model[j].bead_coordinate.axis[2], 2));
#if defined(DEBUG_OVERLAP_2)
            if(i == 2 && (j == 34 || j == 60))
               printf("x2 overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - %s\n",
                      i, j,
                      bead_model[i].chain,
                      bead_model[j].chain,
                      bead_model[i].exposed_code,
                      bead_model[j].exposed_code,
                      bead_model[i].active ? "Y" : "N",
                      bead_model[j].active ? "Y" : "N",
                      bead_model[i].bead_computed_radius,
                      bead_model[j].bead_computed_radius,
                      bead_model[i].bead_coordinate.axis[0],
                      bead_model[i].bead_coordinate.axis[1],
                      bead_model[i].bead_coordinate.axis[2],
                      bead_model[j].bead_coordinate.axis[0],
                      bead_model[j].bead_coordinate.axis[1],
                      bead_model[j].bead_coordinate.axis[2],
                      separation,
                      separation <= tolerance ? "ok" : "needs reduction"
                      );
#endif
            if (separation <= tolerance) {
               continue;
            }
            if ( bead_model[i].bead_computed_radius > tolerance * 1.001 &&
                 bead_model[j].bead_computed_radius > tolerance * 1.001 )
            {
               retval++;
               if ( limit && retval > limit ) {
		 //editor_msg( "red", us_tr( "There are more than %1 overlaps greater than tolerance of %2\nWe suggest you use ZENO to calculate hydrodynamics for this model" ).arg( limit ).arg( tolerance ) );
                  return retval;
               }
               // QColor save_color = editor->textColor();
               // editor->setTextColor("red");
               us_log->log(QString(us_tr("WARNING: Bead model has an overlap violation on beads %1 %2 overlap %3 A\n"))
                               .arg(i + 1)
                               .arg(j + 1)
                               .arg(separation));
	       if ( us_udp_msg )
		 {
		   map < QString, QString > msging;
		   msging[ "_textarea" ] = QString(us_tr("WARNING: Bead model has an overlap violation on beads %1 %2 overlap %3 A\\n"))
		     .arg(i + 1)
		     .arg(j + 1)
		     .arg(separation);
		   
		   us_udp_msg->send_json( msging );
		   //sleep(1);
		 } 
	       accumulated_msgs += QString(us_tr("WARNING: Bead model has an overlap violation on beads %1 %2 overlap %3 A\\n"))
		     .arg(i + 1)
		     .arg(j + 1)
		     .arg(separation);

               // editor->setTextColor(save_color);
#if defined(DEBUG_OVERLAP)
            printf("overlap check  beads %d %d on chains %d %d exposed code %d %d active %s %s : radii %f(%s) %f(%s) with coordinates [%f,%f,%f] [%f,%f,%f] sep of %f - needs reduction\n",
                   i + 1, j + 1,
                   bead_model[i].chain,
                   bead_model[j].chain,
                   bead_model[i].exposed_code,
                   bead_model[j].exposed_code,
                   bead_model[i].active ? "Y" : "N",
                   bead_model[j].active ? "Y" : "N",
                   bead_model[i].bead_computed_radius,
                   bead_model[i].bead_computed_radius > tolerance * 1.001 ? "gt tol" : "not gt tol",
                   bead_model[j].bead_computed_radius,
                   bead_model[j].bead_computed_radius > tolerance * 1.001 ? "gt tol" : "not gt tol",
                   bead_model[i].bead_coordinate.axis[0],
                   bead_model[i].bead_coordinate.axis[1],
                   bead_model[i].bead_coordinate.axis[2],
                   bead_model[j].bead_coordinate.axis[0],
                   bead_model[j].bead_coordinate.axis[1],
                   bead_model[j].bead_coordinate.axis[2],
                   separation
                   );
#endif
            }
         }
      }
   }
#if defined(DEBUG_OVERLAP)
   printf("end of overlap checking--overlap tolerance %f--%s-%s-%s--violations %d------------------\n",
          tolerance,
          sc ? "side chain" : "",
          mc ? "main & side chain" : "",
          buried ? "buried" : "",
          retval);
#endif
   return retval;
}


/* calc hydro */

int US_Saxs_Util::calc_hydro()
{
   return do_calc_hydro();
}

int US_Saxs_Util::do_calc_hydro()
{
   // if ( !overwrite )
   // {
   //    setHydroFile();
   // }

   // stopFlag = false;
   // pb_stop_calc->setEnabled(true);
   // pb_calc_hydro->setEnabled(false);
   // pb_calc_zeno->setEnabled(false);
   // //   puts("calc hydro (supc)");
   // display_default_differences();
  us_log->log("\nBegin hydrodynamic calculations\n\n");
  if ( us_udp_msg )
    {
      map < QString, QString > msging;
      msging[ "_textarea" ] = "\\nBegin hydrodynamic calculations\\n\\n";
      
      us_udp_msg->send_json( msging );
      //sleep(1);
    }   
  accumulated_msgs += "\\nBegin hydrodynamic calculations\\n\\n";

   results_hydro.s20w_sd = 0.0;
   results_hydro.D20w_sd = 0.0;
   results_hydro.viscosity_sd = 0.0;
   results_hydro.rs_sd = 0.0;
   results_hydro.rg_sd = 0.0;
   results_hydro.tau_sd = 0.0;

   int models_to_proc = 0;
   int first_model_no = 0;
   vector < QString > model_names;
   
   /* CHANGE  current_model to curr_m */
   // for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++) {
   //    if (lb_model->item(current_model)->isSelected()) {

   for ( unsigned int curr_m = 0; curr_m < model_vector.size(); curr_m++  )
     {
            // if (somo_processed[current_model]) {
            // if (!first_model_no) {
            //    first_model_no = current_model + 1;

	    if (somo_processed[curr_m]) {
	      if (!first_model_no) {
		first_model_no = curr_m + 1;
	      }
	      models_to_proc++;
	      us_log->log( QString( "Model %1 will be included\n").arg( curr_m +1  ) );
	      if ( us_udp_msg )
		{
		  map < QString, QString > msging;
		  msging[ "_textarea" ] = QString( "Model %1 will be included\\n").arg( curr_m + 1); 
		  
		  us_udp_msg->send_json( msging );
		  //sleep(1);
		}   
	      accumulated_msgs += QString( "Model %1 will be included\\n").arg( curr_m + 1); 
	      // model_names.push_back( model_name( current_model ) );
	      // bead_model = bead_models[current_model];
	      
	      model_names.push_back( QString::number( curr_m ) );
	      bead_model = bead_models[curr_m];
	      
	      // write_bead_spt(somo_dir + SLASH + project +
	      //          (bead_model_from_file ? "" : QString("_%1").arg( model_name( current_model ) ) ) +
	      //          QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
	      //          DOTSOMO, &bead_model, bead_model_from_file);

	      

	    }
	    else
	      {
		// cout << "Model %1 - selected but bead model not built\n" << endl;
		us_log->log(QString("Model %1 - selected but bead model not built\n").arg(  curr_m + 1 ) );
		if ( us_udp_msg )
		  {
		    map < QString, QString > msging;
		    msging[ "_textarea" ] = QString("Model %1 - selected but bead model not built\\n").arg(  curr_m + 1 );
		    
		    us_udp_msg->send_json( msging );
		    //sleep(1);
		  } 
		accumulated_msgs += QString("Model %1 - selected but bead model not built\\n").arg(  curr_m + 1 );
	      }
     }
   // }

   // QDir::setCurrent(somo_dir);

   us_log->log(QString("%1")
                   //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                   .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                        ? QString("\nNotice: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\n")
                        .arg(overlap_tolerance).arg(hydro.overlap) : ""));
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("%1")
                   //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                   .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                        ? QString("\nNotice: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\\n")
                        .arg(overlap_tolerance).arg(hydro.overlap) : "");
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     } 
   accumulated_msgs += QString("%1")
                   //       .arg(hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance)
                   .arg((fabs((hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance) - overlap_tolerance) > 1e-5)
                        ? QString("\nNotice: Overlap reduction bead overlap tolerance %1 does not equal the manually selected hydrodynamic calculations bead overlap cut-off %2\\n")
                        .arg(overlap_tolerance).arg(hydro.overlap) : "");
   

   // qApp->processEvents();
   // if (stopFlag)
   // {
   //    us_log->log("Stopped by user\n\n");
   //    pb_calc_hydro->setEnabled(true);
   //    pb_calc_zeno->setEnabled(true);
   //    pb_bead_saxs->setEnabled(true);
   //    pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   //    pb_show_hydro_results->setEnabled(false);
   //    progress->reset();
   //    return -1;
   // }

   //le_bead_model_suffix->setText(bead_model_suffix);
   
   //qDebug (accumulated_msgs);

   int retval = us_hydrodyn_supc_main_hydro(bead_model_from_file, 
					    misc,
					    &results_hydro,                                  
					    &hydro,
					    hydro.overlap_cutoff ? hydro.overlap : overlap_tolerance,
					    &bead_models,
					    &somo_processed,
					    &model_vector,
                                 // lb_model,
					    QString(project +
						    //(bead_model_from_file ? "" : QString("_%1").arg(current_model + 1)) +
						    //(bead_model_from_file ? "" : (models_to_proc == 1 ? "_1" : "_%1")) +
						    //(bead_model_from_file ? "" : "_%1") +
						    "_%1" +
						    QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
						    DOTSOMO + ".beams").toLatin1().data(),
					    QString(project +
						    //(bead_model_from_file ? "" : QString("_%1").arg( model_name( first_model_no - 1 ) ) ) +
						    (QString("_%1").arg( first_model_no ) ) +
						    QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
						    DOTSOMO + ".beams").toLatin1().data(),
					    model_names,
					    // progress,
					    // editor,
					    // this,
					    us_log,
					    us_udp_msg,
					    &accumulated_msgs
					    );


   //qDebug (accumulated_msgs);

   // QDir::setCurrent(somo_tmp_dir);
   // if (stopFlag)
   // {
   //    us_log->log("Stopped by user\n\n");
   //    pb_calc_hydro->setEnabled(true);
   //    pb_calc_zeno->setEnabled(true);
   //    pb_bead_saxs->setEnabled(true);
   //    pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   //    pb_show_hydro_results->setEnabled(false);
   //    progress->reset();
   //    return -1;
   // }

   // printf("back from supc retval %d\n", retval);
   // pb_show_hydro_results->setEnabled(retval ? false : true);
   // pb_calc_hydro->setEnabled(true);
   // pb_calc_zeno->setEnabled(true);
   // pb_bead_saxs->setEnabled(true);
   // pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
 
   if ( retval )
   {
      us_log->log("Calculate hydrodynamics failed\n\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Calculate hydrodynamics failed\\n\\n"; 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	} 
      accumulated_msgs += "Calculate hydrodynamics failed\\n\\n";  
      //qApp->processEvents();
      switch ( retval )
      {
      case US_HYDRODYN_SUPC_FILE_NOT_FOUND:
         {
	   //printError("Calculate RB Hydrodynamics SMI encountered a file not found error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_OVERLAPS_EXIST:
         {
	   //printError("Calculate RB Hydrodynamics SMI encountered overlaps in the bead model\nPerhaps you should Calculate RB Hydrodynamics ZENO which supports bead models with overlaps");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_ERR_MEMORY_ALLOC:
         {
	   //printError("Calculate RB Hydrodynamics SMI encountered a memory allocation error");
            return retval;
            break;
         }
      case US_HYDRODYN_SUPC_NO_SEL_MODELS:
         {
	   //printError("Calculate RB Hydrodynamics SMI was called with no processed models selected");
            return retval;
            break;
         }
      case US_HYDRODYN_PAT_ERR_MEMORY_ALLOC:
         {
	   //printError("US_HYDRODYN_PAT encountered a memory allocation error");
            return retval;
            break;
         }
      default:
         {
	   //printError("Calculate RB Hydrodynamics SMI encountered an unknown error");
            // unknown error
            return retval;
            break;
         }
      }
   }

   // pb_stop_calc->setEnabled(false);
   us_log->log("Calculate hydrodynamics completed\n");
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = "Calculate hydrodynamics completed\\n";
       
       us_udp_msg->send_json( msging );
	  //sleep(1);
     }   
   accumulated_msgs += "Calculate hydrodynamics completed\\n";
   // if ( advanced_config.auto_show_hydro ) 
   // {
   //    show_hydro_results();
   // }
   // play_sounds(1);
   // qApp->processEvents();
   return 0;
}



QString US_Saxs_Util::getExtendedSuffix(bool prerun, bool somo)
{
   // produce a suffix based upon settings
   // e.g.
   // A20 for ASA screen 20 
   // R50 for ASA recheck 50% 
   // hi or sy (if ALL 3 panels set to the same thing) 
   // OT outward translation
   // hy = hydration on in grid models,
   // G4 grid setting in grid models & 
   // -so for somo models

  QString result;// = le_bead_model_prefix->text();

   // if ( setSuffix )
   // {
      result += result.length() ? "-" : "";

      if ( asa_hydro.calculation )
      {
         result += QString("A%1").arg(somo ? asa_hydro.threshold : asa_hydro.grid_threshold);
      }
      
      if ( asa_hydro.recheck_beads )
      {
         result += QString("R%1").arg(somo ? asa_hydro.threshold_percent : asa_hydro.grid_threshold_percent);
      }
      
      if ( somo ) 
      {
         if ( sidechain_overlap.remove_overlap &&
              mainchain_overlap.remove_overlap &&
              buried_overlap.remove_overlap &&
              (sidechain_overlap.remove_sync == 
               mainchain_overlap.remove_sync) && 
              (mainchain_overlap.remove_sync == 
               buried_overlap.remove_sync ) &&
              (sidechain_overlap.remove_hierarch == 
               mainchain_overlap.remove_hierarch) &&
              (mainchain_overlap.remove_hierarch == 
               buried_overlap.remove_hierarch ) )
         {
            result += QString("%1").arg(sidechain_overlap.remove_sync ? "sy" : "hi");
         }
         
         if ( sidechain_overlap.remove_overlap &&
              sidechain_overlap.translate_out )
         {
            result += "OT";
         }
      }
      if ( !somo ) 
      {
         if ( grid_exposed_overlap.remove_overlap &&
              grid_buried_overlap.remove_overlap &&
              grid_overlap.remove_overlap &&
              (grid_exposed_overlap.remove_sync == 
               grid_buried_overlap.remove_sync) &&
              (grid_buried_overlap.remove_sync ==
               grid_overlap.remove_sync ) &&
              (grid_exposed_overlap.remove_hierarch == 
               grid_buried_overlap.remove_hierarch) &&
              (grid_buried_overlap.remove_hierarch == 
               grid_overlap.remove_hierarch ) )
         {
            result += QString("%1").arg(grid_exposed_overlap.remove_sync ? "sy" : "hi");
         }
         
         if ( grid_exposed_overlap.remove_overlap &&
              grid_exposed_overlap.translate_out )
         {
            result += "OT";
         }
         
         if ( grid_hydro.hydrate )
         {
            result += "hy";
         }
         
         if ( grid_hydro.cubic )
         {
            result += QString("G%1").arg(grid_hydro.cube_side);
         }
      }
      // }
   if ( !prerun )
   { 
      result += QString(result.length() ? "-" : "") + QString(somo ? "so" : "a2b");
   }
   us_log->log(result);
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = result;
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     } 
   accumulated_msgs += result;
   result.replace( ".", "_" );
   return result;
}



QString US_Saxs_Util::getExtendedSuffix_somo(bool prerun, bool somo, bool no_ovlp_removal )
{
   // produce a suffix based upon settings
   // e.g.
   // A20 for ASA screen 20 
   // R50 for ASA recheck 50% 
   // hi or sy (if ALL 3 panels set to the same thing) 
   // OT outward translation
   // hy = hydration on in grid models,
   // G4 grid setting in grid models & 
   // -so for somo models

  QString result;// = le_bead_model_prefix->text();

   // if ( setSuffix )
   //   {
      result += result.length() ? "-" : "";

      if ( ( asa_hydro.calculation && somo ) ||
           ( grid_hydro.enable_asa && !somo ) )
      {
         result += QString("A%1").arg(somo ? asa_hydro.threshold : asa_hydro.grid_threshold);
      }
      
      if ( asa_hydro.recheck_beads )
      {
         result += QString("R%1").arg(somo ? asa_hydro.threshold_percent : asa_hydro.grid_threshold_percent);
      }
      
      if ( somo ) 
      {
         if ( !no_ovlp_removal ) 
         {
            if ( sidechain_overlap.remove_overlap &&
                 mainchain_overlap.remove_overlap &&
                 buried_overlap.remove_overlap &&
                 (sidechain_overlap.remove_sync == 
                  mainchain_overlap.remove_sync) && 
                 (mainchain_overlap.remove_sync == 
                  buried_overlap.remove_sync ) &&
                 (sidechain_overlap.remove_hierarch == 
                  mainchain_overlap.remove_hierarch) &&
                 (mainchain_overlap.remove_hierarch == 
                  buried_overlap.remove_hierarch ) )
            {
               result += QString("%1").arg(sidechain_overlap.remove_sync ? "sy" : "hi");
            }
         
            if ( sidechain_overlap.remove_overlap &&
                 sidechain_overlap.translate_out )
            {
               result += "OT";
            }
         }
      }
      if ( !somo ) 
      {
         if ( grid_exposed_overlap.remove_overlap &&
              grid_buried_overlap.remove_overlap &&
              grid_overlap.remove_overlap &&
              (grid_exposed_overlap.remove_sync == 
               grid_buried_overlap.remove_sync) &&
              (grid_buried_overlap.remove_sync ==
               grid_overlap.remove_sync ) &&
              (grid_exposed_overlap.remove_hierarch == 
               grid_buried_overlap.remove_hierarch) &&
              (grid_buried_overlap.remove_hierarch == 
               grid_overlap.remove_hierarch ) )
         {
            result += QString("%1").arg(grid_exposed_overlap.remove_sync ? "sy" : "hi");
         }
         
         if ( grid_exposed_overlap.remove_overlap &&
              grid_exposed_overlap.translate_out )
         {
            result += "OT";
         }
         
         if ( grid_hydro.hydrate )
         {
            result += "hy";
         }
         
         if ( grid_hydro.cubic )
         {
            result += QString("G%1").arg(grid_hydro.cube_side);
         }
      }
      //    }

   if ( !prerun )
   { 
      result += QString(result.length() ? "-" : "") + QString(somo ? "so" : "a2b");
      if ( somo && no_ovlp_removal )
      {
         result += "_ovlp";
      }
   }
   us_log->log(result);
   if ( us_udp_msg )
     {
      map < QString, QString > msging;
      msging[ "_textarea" ] = result;
      us_udp_msg->send_json( msging );
      //sleep(1);
     }
   accumulated_msgs += result;
   result.replace( ".", "_" );
   return result;
}


void US_Saxs_Util::write_bead_model( QString fname, 
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

   if (bead_output.output & US_HYDRODYN_OUTPUT_SOMO) {
      fsomo = us_fopen(QString("%1.bead_model").arg(fname).toLatin1().data(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_BEAMS) {
      fbeams = us_fopen(QString("%1.beams").arg(fname).toLatin1().data(), "w");
      frmc = us_fopen(QString("%1.rmc").arg(fname).toLatin1().data(), "w");
      frmc1 = us_fopen(QString("%1.rmc1").arg(fname).toLatin1().data(), "w");
   }
   if (bead_output.output & US_HYDRODYN_OUTPUT_HYDRO) {
      fhydro = us_fopen(QString("%1.dat").arg(fname).toLatin1().data(), "w");
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
              results_hydro.vbar
              );
   }

   if (fbeams) {
      fprintf(fbeams,
              "%d\t-2.000000\t%s.rmc\t%.3f\n",
              beads,
              QFileInfo(fname).fileName().toLatin1().data(),
              results_hydro.vbar
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
                    fstring_somo.toLatin1().data(),
                    use_model[i]->bead_coordinate.axis[0],
                    use_model[i]->bead_coordinate.axis[1],
                    use_model[i]->bead_coordinate.axis[2],
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color_util_saxs(use_model[i]),
                    residues.toLatin1().data(),
                    use_model[i]->bead_recheck_asa
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
                    use_model[i]->bead_ref_mw,
                    get_color_util_saxs(use_model[i]));
            fprintf(frmc1,
                    fstring_rmc1.toLatin1().data(),
                    use_model[i]->bead_computed_radius,
                    use_model[i]->bead_ref_mw,
                    get_color_util_saxs(use_model[i]),
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
      fprintf(fsomo, ( options_log.isNull() ? "" : options_log.toLatin1().data() ) );
      fprintf(fsomo, ( last_abb_msgs.isNull() ? "" : last_abb_msgs.toLatin1().data() ) );

      if ( !extra_text.isEmpty() )
      {
         fprintf(fsomo, "%s", extra_text.toLatin1().data() );
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

void US_Saxs_Util::append_options_log_atob()
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

             ,asa_hydro.calculation ? "On" : "Off"
             ,asa_hydro.recheck_beads ? "On" : "Off"
             ,asa_hydro.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa_hydro.probe_radius
             ,asa_hydro.probe_recheck_radius
             ,asa_hydro.grid_threshold
             ,asa_hydro.grid_threshold_percent
             ,asa_hydro.asab1_step
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

             ,grid_hydro.center ? "Center of Mass" : ( grid_hydro.center == 2 ? "Center of Scattering Intensity" : "Center of Cubelet" )
             ,grid_hydro.cube_side
             ,grid_hydro.cubic ? "On" : "Off"
             ,grid_hydro.hydrate ? "On" : "Off"
             ,grid_hydro.tangency ? "On" : "Off"
             ,grid_hydro.enable_asa ? "On" : "Off"

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


void US_Saxs_Util::append_options_log_somo()
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

             ,asa_hydro.calculation ? "On" : "Off"
             ,asa_hydro.recheck_beads ? "On" : "Off"
             ,asa_hydro.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa_hydro.probe_radius
             ,asa_hydro.probe_recheck_radius
             ,asa_hydro.threshold
             ,asa_hydro.threshold_percent
             ,asa_hydro.asab1_step
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

void US_Saxs_Util::append_options_log_somo_ovlp()
{
   QString s;

   s.sprintf("Bead model built with the following options:\n");
   options_log += s;

   s.sprintf(
             "ASA Calculation:\n"
             "  Perform ASA Calculation:    %s\n"
             "  ASA Method:                 %s\n"
             "  ASA Probe Radius (A):       %.2f\n"
             "  SOMO ASA Threshold (A^2):   %.1f\n"
             "  SOMO Bead ASA Threshold %%:  %.1f\n"
             "  ASAB1 Step Size (A):        %.1f\n"
             "\n"

             ,asa_hydro.calculation ? "On" : "Off"
             ,asa_hydro.method ? "Rolling Sphere" : "Voronoi Tesselation"
             ,asa_hydro.probe_radius
             ,asa_hydro.threshold
             ,asa_hydro.threshold_percent
             ,asa_hydro.asab1_step
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


int US_Saxs_Util::get_color_util_saxs(PDB_atom *a) {
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



void US_Saxs_Util::reload_pdb(bool parameters_set_first_model)
{
   // if ( advanced_config.debug_1 )
   // {
   //    printf("Reload PDB file called %d %d.\n", pdb_file.isEmpty(), !pb_somo->isEnabled());
   // }
   // if ( pdb_file.isEmpty() ) // why did we ever do this ? || !pb_somo->isEnabled() )
   // {
   //    return;
   // }
   us_log->log("\nReloading PDB file.\n");
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = "\\nReloading PDB file.\\n";
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
   accumulated_msgs += "\\nReloading PDB file.\\n";

   // if ( advanced_config.debug_1 )
   // {
   //    printf("Reloading PDB file.\n");
   // }
   int errors_found = 0;
   if ( misc.pb_rule_on )
   {
      residue_list = save_residue_list;
   }
   else
   {
      residue_list = save_residue_list_no_pbr;
   }
   multi_residue_map = save_multi_residue_map;
 
   bool use_parameters_set_first_model = parameters_set_first_model;

   read_pdb_hydro(pdb_file, use_parameters_set_first_model);
   QString error_string = "";
   for(unsigned int i = 0; i < model_vector.size(); i++)
   {
      multi_residue_map = save_multi_residue_map;
      us_log->log(QString("Checking the pdb structure for model %1\n").arg(  i  + 1 ) );
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("Checking the pdb structure for model %1\\n").arg(  i + 1  );
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}
      accumulated_msgs += QString("Checking the pdb structure for model %1\\n").arg(  i + 1  );
      
      if (check_for_missing_atoms_hydro(&error_string, &model_vector[i], parameters_set_first_model))
      {
         errors_found++;
         us_log->log(QString("Encountered errors with your PDB structure for model %1:\n").
                         arg( i ) + error_string);

	 QString error_string_udp = error_string;
	 error_string_udp.replace("\n","\\n");

	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString("Encountered errors with your PDB structure for model %1:\\n").
	       arg( i ) + error_string_udp;
 
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }
	 accumulated_msgs += QString("Encountered errors with your PDB structure for model %1:\\n").
	       arg( i ) + error_string_udp;
         // printError(QString("Encountered errors with your PDB structure for model %1:\n").
         //            arg( model_name( i ) ) + "please check the text window");
      }
      
          
      //reset_chain_residues( &model_vector[ i ] );
      
   }
   model_vector_as_loaded = model_vector;
   // set_pdb_info( "reload pdb" );
   us_log->log(QString("Loaded pdb file : %1\n").arg(errors_found ? "ERRORS PRESENT" : "ok\n"));
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("Loaded pdb file : %1\\n").arg(errors_found ? "ERRORS PRESENT" : "ok\\n");
       
       us_udp_msg->send_json( msging );
       //sleep(1); 
     }
   accumulated_msgs += QString("Loaded pdb file : %1\\n").arg(errors_found ? "ERRORS PRESENT" : "ok\\n");

   if ( errors_found )
   {
      calc_vol_for_saxs();
   } else {
      calc_mw_hydro();
   }
   bead_models.resize( model_vector.size() );
   somo_processed.resize( model_vector.size() );
   update_vbar();
   // pb_somo->setEnabled(true);
   // pb_somo_o->setEnabled(true);
   // pb_grid_pdb->setEnabled(true);
   // pb_grid->setEnabled(false);
   // pb_show_hydro_results->setEnabled(false);
   // pb_calc_hydro->setEnabled(false);
   // pb_calc_zeno->setEnabled(false);
   // pb_bead_saxs->setEnabled(false);
   // pb_rescale_bead_model->setEnabled(false);
   // pb_pdb_saxs->setEnabled(true);
   // pb_visualize->setEnabled(false);
   // pb_equi_grid_bead_model->setEnabled(false);
   // le_bead_model_file->setText(" not selected ");
   bead_models_as_loaded = bead_models;
}


/* Calculate SoMo Bead Model ************************************* */

bool US_Saxs_Util::calc_somo_o(bool parameters_set_first_model)
{
  bool use_parameters_set_first_model =  parameters_set_first_model;
  return calc_somo( true, use_parameters_set_first_model);
}


int US_Saxs_Util::calc_somo( bool no_ovlp_removal, bool parameters_set_first_model)
{
   // if ( selected_models_contain( "WAT" ) )
   // {
   //    QMessageBox::warning( this,
   //                          us_tr( "Selected model contains WAT residue" ),
   //                          us_tr( 
   //                             "Can not process models that contain the WAT residue.\n"
   //                             "These are currently generated only for SAXS/SANS computations\n"
   //                             )
   //                          );
   //    return -1;
   // }

   // if ( selected_models_contain( "XHY" ) )
   // {
   //    QMessageBox::warning( this,
   //                          us_tr( "Selected model contains XHY residue" ),
   //                          us_tr( 
   //                             "Can not process models that contain the XHY residue.\n"
   //                             "These are currently generated only for SAXS/SANS computations\n"
   //                             )
   //                          );
   //    return -1;
   // }

   // stopFlag = false;
   // pb_stop_calc->setEnabled(true);
   // pb_somo->setEnabled(false);
   // pb_somo_o->setEnabled(false);

   // bd_anaflex_enables(false);

   // pb_grid_pdb->setEnabled(false);
   // pb_grid->setEnabled(false);

   model_vector = model_vector_as_loaded;

   // sync_pdb_info( "calc_somo" );
   // editor_msg( "dark blue", QString( us_tr( "Peptide Bond Rule is %1 for this PDB" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
 
   us_log->log( QString( us_tr( "Peptide Bond Rule is %1 for this PDB\n\n" ) ).arg( misc.pb_rule_on ? "on" : "off" ) );
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString( us_tr( "Peptide Bond Rule is %1 for this PDB\\n\\n" ) ).arg( misc.pb_rule_on ? "on" : "off" );
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     } 
   accumulated_msgs += QString( us_tr( "Peptide Bond Rule is %1 for this PDB\\n\\n" ) ).arg( misc.pb_rule_on ? "on" : "off" );

   options_log = "";
   no_ovlp_removal ? append_options_log_somo_ovlp() : append_options_log_somo();

   //display_default_differences();

   bead_model_suffix = getExtendedSuffix_somo(false, true, no_ovlp_removal );

   //   le_bead_model_suffix->setText(bead_model_suffix);
   // if ( !overwrite )
   // {
   //    setSomoGridFile(true);
   // }

   // if (stopFlag)
   // {
   //    us_log->log("Stopped by user\n\n");
   //    bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
   //                          ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
   //    pb_somo->setEnabled(true);
   //    pb_somo_o->setEnabled(true);
   //    pb_grid_pdb->setEnabled(true);
   //    progress->reset();
   //    return -1;
   // }

   // if (!residue_list.size() ||
   //     !model_vector.size())
   // {
   //    fprintf(stderr, "calculations can not be run until residue & pdb files are read!\n");
   //    pb_stop_calc->setEnabled(false);
   //    return -1;
   // }

   // pb_visualize->setEnabled(false);
   // pb_equi_grid_bead_model->setEnabled(false);
   // pb_show_hydro_results->setEnabled(false);
   // pb_calc_hydro->setEnabled(false);
   // pb_calc_zeno->setEnabled(false);
   // if (results_widget)
   // {
   //    results_window->close();
   //    delete results_window;
   //    results_widget = false;
   // }
   bool any_errors = false;
   bool any_models = false;

   // somo_processed.resize(lb_model->count());
   // bead_models.resize(lb_model->count());
   
   somo_processed.resize(model_vector.size());
   bead_models.resize(model_vector.size());


   QString msg = QString("\n%1 models selected:").arg(project);
   QString msg_udp = QString("\\n%1 models selected:").arg(project);

   //   for(int i = 0; i < lb_model->count(); i++) {
   for(int i = 0; i < (int) model_vector.size(); i++) {
     somo_processed[i] = 0;
     //if (lb_model->item(i)->isSelected()) {
         
     current_model = i;
     // msg += QString( " %1" ).arg( i + 1 );
     msg += " " + ( i + 1 );
     msg_udp += " " + ( i + 1 );
     // }
   }
   msg += "\n";
   msg_udp += "\\n";
   
   us_log->log(msg);
   if ( us_udp_msg )
     {
      map < QString, QString > msging;
      msging[ "_textarea" ] = msg_udp;
      
      us_udp_msg->send_json( msging );
      //sleep(1); 
     }
   accumulated_msgs += msg_udp;

   //   for (current_model = 0; current_model < (unsigned int)lb_model->count(); current_model++)
  
   QString extra_text = "";
   bool  use_parameters_set_first_model =  parameters_set_first_model;
   for (current_model = 0; current_model < model_vector.size(); current_model++)
   {
     if (!any_errors) //&& lb_model->item(current_model)->isSelected())
       {
	 any_models = true;
	 if(!compute_asa( false, no_ovlp_removal,  use_parameters_set_first_model))
	   {
	     somo_processed[current_model] = 1;
	     if ( asa_hydro.recheck_beads && !no_ovlp_removal )
	       {
		 // // puts("recheck beads disabled");
		 us_log->log("Rechecking beads\n");
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "_textarea" ] = "Rechecking beads\\n";
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1); 
		   }
		 accumulated_msgs += "Rechecking beads\\n";
		 // qApp->processEvents();
		 
		 bead_check(false, false);
		 us_log->log("Finished rechecking beads\n");
		 if ( us_udp_msg )
		   {
		      map < QString, QString > msging;
		      msging[ "_textarea" ] = "Finished rechecking beads\\n";
		      
		      us_udp_msg->send_json( msging );
		      //sleep(1); 
		   }
		 accumulated_msgs += "Finished rechecking beads\\n";
		 
		 // progress->setValue(19);
		 int ppos = 19;
		 int mppos = 18 + (asa_hydro.recheck_beads ? 1 : 0);
		 
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
		     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1);
		   }
	       }
	     else
	       {
		 us_log->log("No rechecking of beads\n");
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "_textarea" ] = "No rechecking of beads\\n";
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1); 
		   }
		 accumulated_msgs += "No rechecking of beads\\n";
		 //qApp->processEvents();
	       }
	     bead_models[current_model] = bead_model;
	   }
	 else
	   {
             any_errors = true;
	   }
       }		  
	 
	 /* NEED TO WRITE IT DOWN ************************************************************************/
	 
	 QString filename = 
	   project + 
	   QString( "_%1" ).arg( current_model + 1 );
	 
	 // le_bead_model_file->setText( filename );
	 
	 write_bead_model( 
			  //somo_dir + SLASH + filename +
			  filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
			  DOTSOMO, 
			  &bead_model,
			  extra_text  );
	 

	 list_of_models << filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
		     DOTSOMO + ".bead_model";
	 		   
	 write_pdb_hydro( filename + QString( bead_model_suffix.length() ? ( "-" + bead_model_suffix ) : "" ) +
			  DOTSOMO, &bead_model );


	 /* NEED TO WRITE IT DOWN ************************************************************************/
	 
 
	 

     // if (stopFlag)
     //  {
     //     us_log->log("Stopped by user\n\n");
     //     pb_somo->setEnabled(true);
     //     pb_somo_o->setEnabled(true);
     //     bd_anaflex_enables( ( ( browflex && browflex->state() == QProcess::Running ) ||
     //                           ( anaflex && anaflex->state() == QProcess::Running ) ) ? false : true );
     //     pb_grid_pdb->setEnabled(true);
     //     progress->reset();
     //     return -1;
     //  }

     //  // calculate bead model and generate hydrodynamics calculation output
     //  // if successful, enable follow-on buttons:
   }

   if (any_models && !any_errors)
   {
      us_log->log("Build bead model completed\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Build bead model completed\\n"; 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Build bead model completed\\n"; 
      // qApp->processEvents();
      // pb_visualize->setEnabled(true);
      // pb_equi_grid_bead_model->setEnabled(true);
      // pb_calc_hydro->setEnabled( !no_ovlp_removal );
      // pb_calc_zeno->setEnabled(true);
      // pb_bead_saxs->setEnabled(true);
      // pb_rescale_bead_model->setEnabled( misc.target_volume != 0e0 || misc.equalize_radii );
   }
   else
   {
      us_log->log("Errors encountered\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Errors encountered\\n";
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Errors encountered\\n";
   }

   // pb_somo->setEnabled(true);
   // pb_somo_o->setEnabled(true);
   // pb_grid_pdb->setEnabled(true);
   // pb_grid->setEnabled(true);
   // pb_stop_calc->setEnabled(false);
   // if (calcAutoHydro)
   // {
   //    no_ovlp_removal ? calc_zeno_hydro() : calc_hydro();
   // } 
   // else
   // {
   //    play_sounds(1);
   // }
   return 0;
}

/* Compute ASA ******************************************************** */

int US_Saxs_Util::compute_asa( bool bd_mode, bool no_ovlp_removal, bool parameters_set_first_model )
{
  //cout << "CURRENT_MODEL: " << current_model << endl;

   QString error_string = "";
   // progress->reset();
   /* Reset Progress bar */
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("");
       msging[ "progress1" ] = QString::number(0.0);
       
       us_udp_msg->send_json( msging );
     }   
 

   us_log->log(QString("\nBuilding the bead model for %1 model %2\n").arg(project).arg( current_model +1  ) );
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("\\nBuilding the bead model for %1 model %2\\n").arg(project).arg( current_model +1  );
       
       us_udp_msg->send_json( msging );
       //sleep(1); 
     }
   accumulated_msgs += QString("\\nBuilding the bead model for %1 model %2\\n").arg(project).arg( current_model +1  );

   us_log->log("Checking the pdb structure\n");
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = "Checking the pdb structure\\n";
       
       us_udp_msg->send_json( msging );
       //sleep(1); 
     }
   accumulated_msgs += "Checking the pdb structure\\n";

   bool use_parameters_set_first_model = parameters_set_first_model;

   if (check_for_missing_atoms_hydro(&error_string, &model_vector[current_model], use_parameters_set_first_model)) {
      us_log->log("Encountered the following errors with your PDB structure:\n" +
		  error_string);

      QString error_string_udp = error_string;
      error_string_udp.replace("\n","\\n");

      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Encountered the following errors with your PDB structure:\\n" +
	    error_string_udp; 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Encountered the following errors with your PDB structure:\\n" +
	    error_string_udp; 
      // printError("Encountered errors with your PDB structure:\n"
      //            "please check the text window");
      return -1;
   }

   results_hydro.asa_rg_pos = 0.0;
   results_hydro.asa_rg_neg = 0.0;
   us_log->log("PDB structure ok\n");
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = "PDB structure ok\\n";
       
       us_udp_msg->send_json( msging );
       //sleep(1); 
     }
   accumulated_msgs += "PDB structure ok\\n";
     
   int mppos = 18 + (asa_hydro.recheck_beads ? 1 : 0);
   //progress->setMaximum(mppos);
   int ppos = 1;
   //progress->setValue(ppos++); // 1
   //qApp->processEvents();
   
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
	 
   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      us_log->log(QString("There are %1 atoms in %2 chain(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("There are %1 atoms in %2 chain(s) in this model\\n").arg(no_of_atoms).arg(no_of_molecules);
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += QString("There are %1 atoms in %2 chain(s) in this model\\n").arg(no_of_atoms).arg(no_of_molecules);
   }
   // if (stopFlag)
   // {
   //    return -1;
   // }

   int retval = create_beads_hydro(&error_string);

   if ( retval )
   {
       us_log->log("Errors found during the initial creation of beads\n");
       if ( us_udp_msg )
	 {
	   map < QString, QString > msging;
	   msging[ "_textarea" ] = "Errors found during the initial creation of beads\\n";
	   
	   us_udp_msg->send_json( msging );
	   //sleep(1); 
	 }
       accumulated_msgs += "Errors found during the initial creation of beads\\n";
      // progress->setValue(mppos);
      // qApp->processEvents();
      // if (stopFlag)
      // {
      //    return -1;
      // }
      switch ( retval )
      {
      case US_SURFRACER_ERR_MISSING_RESIDUE:
         {
   	   //printError("Encountered an unknown residue:\n" +
           //            error_string);
            return US_SURFRACER_ERR_MISSING_RESIDUE;
            break;
         }
      case US_SURFRACER_ERR_MISSING_ATOM:
         {
   	   //printError("Encountered a unknown atom:\n" +
           //            error_string);
            return US_SURFRACER_ERR_MISSING_ATOM;
            break;
         }
      case US_SURFRACER_ERR_MEMORY_ALLOC:
         {
   	   //printError("Encountered a memory allocation error");
            return US_SURFRACER_ERR_MEMORY_ALLOC;
            break;
         }
      default:
         {
   	   //printError("Encountered an unknown error");
            // unknown error
            return -1;
            break;
         }
      }
   }

   if(error_string.length()) {
     //progress->setValue(mppos);
     //qApp->processEvents();
     //printError("Encountered unknown atom(s) error:\n" +
     //            error_string);
      return US_SURFRACER_ERR_MISSING_ATOM;
   }


   /* TEMPORARILY COMMENTED OUT *************************************/

   // if(asa_hydro.method == 0) {
   //    // // surfracer
   //    // us_log->log("Computing ASA via SurfRacer\n");
   //    // qApp->processEvents();
   //    // if (stopFlag)
   //    // {
   //    //    return -1;
   //    // }
   //    int retval = surfracer_main(asa_hydro.probe_radius,
   //                                active_atoms,
   //                                false,
   //                                progress,
   //                                editor
   //                                );

   //    // progress->setValue(ppos++); // 2
   //    // qApp->processEvents();
   //    // us_log->log("Return from Computing ASA\n");
   //    // if (stopFlag)
   //    // {
   //    //    return -1;
   //    // }
   //    // if ( retval )
   //    // {
   //    //    us_log->log("Errors found during ASA calculation\n");
   //    //    progress->setValue(mppos);
   //    //    qApp->processEvents();
   //    //    if (stopFlag)
   //    //    {
   //    //       return -1;
   //    //    }
   //    //    switch ( retval )
   //    //    {
   //    //    case US_SURFRACER_ERR_MISSING_RESIDUE:
   //    //       {
   //    //          printError("US_SURFRACER encountered an unknown residue:\n" +
   //    //                     error_string);
   //    //          return US_SURFRACER_ERR_MISSING_RESIDUE;
   //    //          break;
   //    //       }
   //    //    case US_SURFRACER_ERR_MISSING_ATOM:
   //    //       {
   //    //          printError("US_SURFRACER encountered a unknown atom:\n" +
   //    //                     error_string);
   //    //          return US_SURFRACER_ERR_MISSING_ATOM;
   //    //          break;
   //    //       }
   //    //    case US_SURFRACER_ERR_MEMORY_ALLOC:
   //    //       {
   //    //          printError("US_SURFRACER encountered a memory allocation error");
   //    //          return US_SURFRACER_ERR_MEMORY_ALLOC;
   //    //          break;
   //    //       }
   //    //    default:
   //    //       {
   //    //          printError("US_SURFRACER encountered an unknown error");
   //    //          // unknown error
   //    //          return -1;
   //    //          break;
   //    //       }
   //    //    }
   //    // }
   //    // if(error_string.length()) {
   //    //    progress->setValue(mppos);
   //    //    qApp->processEvents();
   //    //    printError("US_SURFRACER encountered unknown atom(s) error:\n" +
   //    //               error_string);
   //    //    return US_SURFRACER_ERR_MISSING_ATOM;
   //    // }
   // }

  /* ***********************************************************************************************/

   if(asa_hydro.method == 1) {
      // // surfracer
     us_log->log("Computing ASA via ASAB1\n");
     if ( us_udp_msg )
       {
	 map < QString, QString > msging;
	 msging[ "_textarea" ] = "Computing ASA via ASAB1\\n"; 
	 
	 us_udp_msg->send_json( msging );
	 //sleep(1); 
       }
     accumulated_msgs += "Computing ASA via ASAB1\\n"; 
     
      // qApp->processEvents();
      // if (stopFlag)
      // {
      //    return -1;
      // }

    
      int retval = us_hydrodyn_asab1_main_hydro(active_atoms,
                                          &asa_hydro,
                                          &results_hydro,
						false); // ,
                                          // progress,
                                          // editor,
                                          // this
                                          // );

      // progress->setValue(ppos++); // 2
      // qApp->processEvents();
      ppos++;

      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
	  msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}
	 

      us_log->log("Return from Computing ASA\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Return from Computing ASA\\n"; 
	 
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Return from Computing ASA\\n"; 
      // if (stopFlag)
      // {
      //    return -1;
      // }
     

      // cout << "RETVAL: " <<  retval << endl;

      if ( retval )
      {
	us_log->log("Errors found during ASA calculation\n");
	if ( us_udp_msg )
	  {
	    map < QString, QString > msging;
	    msging[ "_textarea" ] = "Errors found during ASA calculation\\n";
	    
	    us_udp_msg->send_json( msging );
	    //sleep(1); 
	  }
	accumulated_msgs += "Errors found during ASA calculation\\n";
      //    progress->setValue(mppos);
      //    qApp->processEvents();
      //    if (stopFlag)
      //    {
      //       return -1;
      //    }

         switch ( retval )
         {
         case US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC:
            {
	      //printError("US_HYDRODYN_ASAB1 encountered a memory allocation error");
               return US_HYDRODYN_ASAB1_ERR_MEMORY_ALLOC;
               break;
            }
         default:
            {
	      //printError("US_HYDRODYN_ASAB1 encountered an unknown error");
               // unknown error
               return -1;
               break;
            }
         }
      }
   }

   // pass 1 assign bead #'s, chain #'s, initialize data

   FILE *asaf = us_fopen(QString("atom_tmp.asa"), "w");


   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p1 i j k %d %d %d %lx %s\n", i, j, k, (long unsigned int)this_atom->p_atom, this_atom->active ? "active" : "not active"); fflush(stdout);

            //   this_atom->bead_assignment =
            //     (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            //   printf("this_atom->bead_assignment %d\n", this_atom->bead_assignment); fflush(stdout);
            //   this_atom->chain =
            //     ((this_atom->p_residue && this_atom->p_atom) ?
            //      (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);

            // initialize data
            // this_atom->bead_positioner = false;
            this_atom->normalized_ot_is_valid = false;
            fprintf(asaf, "%s\t%s\t%s\t%.2f\n",
                    this_atom->name.toLatin1().data(),
                    this_atom->resName.toLatin1().data(),
                    this_atom->resSeq.toLatin1().data(),
                    this_atom->asa);

            for (unsigned int m = 0; m < 3; m++) {
               this_atom->bead_cog_coordinate.axis[m] = 0;
               this_atom->bead_position_coordinate.axis[m] = 0;
               this_atom->bead_coordinate.axis[m] = 0;
            }
         }
      }
   }
   fclose(asaf);

   // progress->setValue(ppos++); // 3
   // qApp->processEvents();

   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

   // if (stopFlag)
   // {
   //    return -1;
   // }

   // #define DEBUG
   // pass 2 determine beads, cog_position, fixed_position, molecular cog phase 1.

   int count_actives;
   float molecular_cog[3] = { 0, 0, 0 };
   float molecular_mw = 0;
   QString cog_msg = "COG calc summary information\n";

   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         int last_bead_assignment = -1;
         int last_chain = -1;
         QString last_resName = "not a residue";
         QString last_resSeq = "";
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         PDB_atom *last_main_bead = (PDB_atom *) 0;
         PDB_atom *sidechain_N = (PDB_atom *) 0;

         count_actives = 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p2 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            // this_atom->bead_positioner = false;
            if (this_atom->active) {
               // if ( advanced_config.debug_1 )
               // {
               //    printf("pass 2 active %s %s %d pm %d %d\n",
               //           this_atom->name.toLatin1().data(),
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->serial,
               //           this_atom->placing_method,
               //           this_atom->bead_assignment); fflush(stdout);
               // }
               molecular_mw += this_atom->mw;
               for (unsigned int m = 0; m < 3; m++) {
                  molecular_cog[m] += this_atom->coordinate.axis[m] * this_atom->mw;
               }

               this_atom->bead_mw = 0;
               this_atom->bead_asa = 0;
               this_atom->bead_recheck_asa = 0;

               // do we have a new bead?
               // we want to put the N on a previous bead unless it is the first one of the molecule
               // ONLY FOR residue type = 0! (amino acid)
               // AND ONLY for residues not part of the 'auto bead builder'

               if (!create_beads_normally ||
                   (
		    (this_atom->bead_assignment != last_bead_assignment ||
                     this_atom->chain != last_chain ||
                     this_atom->resName != last_resName ||
                     this_atom->resSeq != last_resSeq) &&
                    !(misc.pb_rule_on &&
                      this_atom->chain == 0 &&
                      this_atom->name == "N" &&
                      count_actives &&
                      !broken_chain_head.count(QString("%1|%2")
                                               .arg(this_atom->resSeq)
                                               .arg(this_atom->resName))
                      ) ) ) {
                  // if ( advanced_config.debug_1 ||
                  //      advanced_config.debug_2 )
                  // {
                  //    printf("pass 2 active %s %s %d new bead chain %d\n",
                  //           this_atom->name.toLatin1().data(),
                  //           this_atom->resName.toLatin1().data(),
                  //           this_atom->serial,
                  //           this_atom->chain); fflush(stdout);
                  // }

                  // this_atom->bead_positioner = true;

		 this_atom->is_bead = true;
                  last_main_bead = this_atom;
                  last_bead_assignment = this_atom->bead_assignment;
                  last_chain = this_atom->chain;
                  last_resName = this_atom->resName;
                  last_resSeq = this_atom->resSeq;
                  if (create_beads_normally && 
                      misc.pb_rule_on &&
                      !this_atom->type) {
                     if(sidechain_N &&
                        this_atom->chain == 1) {
                        if(this_atom->name == "N") {
                           printf("ERROR double N on sequential sidechains! PRO PRO?\n");
                        }
                        // if ( advanced_config.debug_1 ||
                        //      advanced_config.debug_2 )
                        // {
                        //    printf("adding sidechain N %f %f to this_atom\n",
                        //           sidechain_N->asa,
                        //           sidechain_N->mw
                        //           );
                        // }
                        this_atom->bead_asa += sidechain_N->bead_asa;
                        this_atom->bead_mw += sidechain_N->bead_mw;
                        sidechain_N->bead_mw = 0;
                        sidechain_N->bead_asa = 0;
                        sidechain_N = (PDB_atom *) 0;
                     }
                     if(this_atom->name == "N" &&
                        this_atom->chain == 1) {
                        sidechain_N = this_atom;
                        this_atom->is_bead = false;
                     }
                  }
               }
               else 
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    printf("pass 2 active %s %s %d not a new bead\n",
                  //           this_atom->name.toLatin1().data(),
                  //           this_atom->resName.toLatin1().data(),
                  //           this_atom->serial); fflush(stdout);
                  // }
                  if (this_atom->bead_positioner) {

                     if (last_main_bead->bead_positioner &&
                         this_atom->placing_method == 1) {
                        fprintf(stderr, "warning: 2 positioners in bead %s %s %d\n",
                                last_main_bead->name.toLatin1().data(),
                                last_main_bead->resName.toLatin1().data(),
                                last_main_bead->serial);
                     }
                     last_main_bead->bead_positioner = true;
                     last_main_bead->bead_position_coordinate = this_atom->coordinate;
                  }

                  this_atom->is_bead = false;
                  // this_atom->bead_cog_mw = 0;
               }

               this_atom->bead_cog_mw = 0;

               // special nitrogen asa handling
               PDB_atom *use_atom;
               if (misc.pb_rule_on &&
                   create_beads_normally &&
                   this_atom->chain == 0 &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                           .arg(this_atom->resSeq)
                                           .arg(this_atom->resName)) &&
                   last_main_chain_bead) {
                  use_atom = last_main_chain_bead;
               }
               else 
               {
                  use_atom = last_main_bead;
               }

               use_atom->bead_asa += this_atom->asa;
               use_atom->bead_mw += this_atom->mw;
               // if ( advanced_config.debug_1 )
               // {
               //    printf("atom %s %s p_atom.hybrid.mw %f atom.mw %f\n",
               //           this_atom->name.toLatin1().data(),
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->p_atom->hybrid.mw,
               //           this_atom->mw
               //           );
               // }

               // accum
               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  // if ( advanced_config.debug_3 )
                  // {
                  //    printf("adding cog from %d to %d mw %f totmw %f (this pos [%f,%f,%f], org pos [%f,%f,%f])\n", 
                  //           this_atom->serial, 
                  //           use_atom->serial, 
                  //           this_atom->mw, 
                  //           use_atom->bead_cog_mw,
                  //           this_atom->coordinate.axis[0],
                  //           this_atom->coordinate.axis[1],
                  //           this_atom->coordinate.axis[2],
                  //           use_atom->bead_cog_coordinate.axis[0],
                  //           use_atom->bead_cog_coordinate.axis[1],
                  //           use_atom->bead_cog_coordinate.axis[2]
                  //           );
                  // }
                  cog_msg += QString("adding %1 to %2\n").arg(this_atom->serial).arg(use_atom->serial);
                  use_atom->bead_cog_mw += this_atom->mw;
                  for (unsigned int m = 0; m < 3; m++) {
                     use_atom->bead_cog_coordinate.axis[m] +=
                        this_atom->coordinate.axis[m] * this_atom->mw;
                  }
                  // if ( advanced_config.debug_3 )
                  // {
                  //    printf("afterwards: target mw %f pos [%f,%f,%f]\n",
                  //           use_atom->bead_cog_mw,
                  //           use_atom->bead_cog_coordinate.axis[0],
                  //           use_atom->bead_cog_coordinate.axis[1],
                  //           use_atom->bead_cog_coordinate.axis[2]);
                  // }
               }
               // else 
               // {
               //    if ( advanced_config.debug_3 )
               //    {
               //       printf("notice: atom %s %s %d excluded from cog calculation in bead %s %s %d\n",
               //              this_atom->name.toLatin1().data(),
               //              this_atom->resName.toLatin1().data(),
               //              this_atom->serial,
               //              use_atom->name.toLatin1().data(),
               //              use_atom->resName.toLatin1().data(),
               //              use_atom->serial);
               //    }
               // }

               if (!create_beads_normally ||
                   this_atom->bead_positioner) {
                  if (use_atom->bead_positioner &&
                      this_atom->placing_method == 1) {
                     fprintf(stderr, "warning: 2 or more positioners in bead %s %s %d\n",
                             use_atom->name.toLatin1().data(),
                             use_atom->resName.toLatin1().data(),
                             use_atom->serial);
                  }
                  use_atom->bead_positioner = true;
                  use_atom->bead_position_coordinate = this_atom->coordinate;
               }

               if (this_atom->chain == 0 &&
                   misc.pb_rule_on &&
                   this_atom->name == "N" &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) && 
                   !count_actives)
               {
                  last_resName = "first N";
               }

               if (this_atom->chain == 0 &&
                   this_atom->name == "CA") {
                  last_main_chain_bead = this_atom;
               }
               count_actives++;
            }
            else 
            {
               this_atom->is_bead = false;
            }
         }
      }
   }
   // if ( advanced_config.debug_3 )
   // {
   //    cout << cog_msg;
   // }

   if (molecular_mw) {
      for (unsigned int m = 0; m < 3; m++) {
         molecular_cog[m] /= molecular_mw;
      }
      // if ( advanced_config.debug_3 )
      // {
      //    printf("molecular cog [%f,%f,%f] mw %f\n",
      //           molecular_cog[0],
      //           molecular_cog[1],
      //           molecular_cog[2],
      //           molecular_mw);
      // }
   }
   else 
   {
      printf("ERROR: this molecule has zero mw!\n");
   }

   for (unsigned int m = 0; m < 3; m++) {
      last_molecular_cog.axis[m] = molecular_cog[m];
   }

   // progress->setValue(ppos++); // 4
   // qApp->processEvents();
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

   // if (stopFlag)
   // {
   //    return -1;
   // }

   // pass 2b move bead_ref_volume, ref_mw, computed_radius from
   // next main chain back one including adjustments for GLY, PRO, OXT

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      bool placed_N1 = false;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         bool first_is_pro = false;
         unsigned int main_chain_beads = 0;
         PDB_atom *last_main_chain_bead = (PDB_atom *) 0;
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // if ( advanced_config.debug_1 )
            // {
            //    printf("pass 2b active %d is_bead %d chain %d atom %s %s %d pm %d\n",
            //           this_atom->active,
            //           this_atom->is_bead,
            //           this_atom->chain,
            //           this_atom->name.toLatin1().data(),
            //           this_atom->resName.toLatin1().data(),
            //           this_atom->serial,
            //           this_atom->placing_method); fflush(stdout);
            // }
            if ( this_atom->active &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 )
            {
               main_chain_beads++;
            }
            if ( 
                misc.pb_rule_on &&
                !k &&
                this_atom->resName == "PRO" 
                )
            {
               // if ( advanced_config.debug_1 )
               // {
               //    puts("pass 2b PRO 1st is pro recognized");
               // }
               first_is_pro = true;
            }
            if ( placed_N1 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName))
                 )
            {
               placed_N1 = false;
               // if ( advanced_config.debug_1 )
               // {
               //    printf("placed N1, found break, turned off placed N1 %s %s\n", 
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->resSeq.toLatin1().data()
               //           );
               // }
            }

            if ( 
                misc.pb_rule_on &&
                !k &&
                this_atom->resName != "PRO" &&
                this_atom->name == "N" &&
                !broken_chain_head.count(QString("%1|%2")
                                         .arg(this_atom->resSeq)
                                         .arg(this_atom->resName))
                )
            {
               placed_N1 = true;
               // if ( advanced_config.debug_1 )
               // {
               //    puts("placed N1");
               // }
            }
            if ( 
                first_is_pro &&
                this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 1 &&
                this_atom->resName == "PRO"
                )
            {
               // if ( advanced_config.debug_1 )
               // {
               //    puts("pass 2b PRO 1st N +1 mw adjustment");
               // }
               this_atom->bead_ref_mw += 1.0;
               // what about a volume adjustment?
               first_is_pro = false;
            }
                 
            if (this_atom->name == "OXT" &&
                last_main_chain_bead) {
               // if ( advanced_config.debug_1 )
               // {
               //    printf("pass 2b active OXT %s %s %d last %s %s %d mw org %f mw new %f\n",
               //           this_atom->name.toLatin1().data(),
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->serial,
               //           last_main_chain_bead->name.toLatin1().data(),
               //           last_main_chain_bead->resName.toLatin1().data(),
               //           last_main_chain_bead->serial,
               //           last_main_chain_bead->bead_ref_mw,
               //           this_atom->bead_ref_mw
               //           );
               //    fflush(stdout);
               // }
               this_atom->is_bead = false;
               // override broken head OXT residue
               if ( misc.pb_rule_on &&
                    this_atom->resName != "PRO" &&
                    broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                    multi_residue_map.count("NPBR-OXT") )
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("pass 2b broken head OXT NPBR replacement");
                  // }
                  int posNPBR_OXT = multi_residue_map["NPBR-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[posNPBR_OXT].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[posNPBR_OXT].r_bead[0].mw;
               }
                  
               last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
               last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
               if (last_main_chain_bead->resName == "GLY") 
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("pass 2b GLY adjustment +1");
                  // }
                  last_main_chain_bead->bead_ref_mw += 1.01f;
               }
               if ( !misc.pb_rule_on &&
                    main_chain_beads == 1 &&
                    this_atom->resName != "PRO" )
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("pass 2b main_chain bead adjustment +1");
                  // }
                  last_main_chain_bead->bead_ref_mw += 1.0;
               }
               last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
            } // OXT

            if (this_atom->active &&
                this_atom->is_bead &&
                this_atom->chain == 0) {
               // if ( advanced_config.debug_1 )
               // {
               //    printf("pass 2b active, bead, chain == 0 %s %s %d pm %d\n",
               //           this_atom->name.toLatin1().data(),
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->serial,
               //           this_atom->placing_method); fflush(stdout);
               // }

               if (misc.pb_rule_on &&
                   last_main_chain_bead &&
                   !broken_chain_head.count(QString("%1|%2")
                                            .arg(this_atom->resSeq)
                                            .arg(this_atom->resName)) &&
                   (this_atom->resName == "PRO" ||
                    last_main_chain_bead->resName == "PRO")
                   ) {

                  // if ( advanced_config.debug_1 )
                  // {
                  //    printf("pass 2b active PRO %s %s %d pm %d\n",
                  //           this_atom->name.toLatin1().data(),
                  //           this_atom->resName.toLatin1().data(),
                  //           this_atom->serial,
                  //           this_atom->placing_method); fflush(stdout);
                  // }

                  last_main_chain_bead->bead_ref_volume = this_atom->bead_ref_volume;
                  last_main_chain_bead->bead_ref_mw = this_atom->bead_ref_mw;
                  last_main_chain_bead->bead_computed_radius = this_atom->bead_computed_radius;
                  if (this_atom->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw -= 1.01f;
                     // if ( advanced_config.debug_1 )
                     // {
                     //    puts("pass 2b GLY adjustment -1");
                     // }
                  }
                  if (last_main_chain_bead->resName == "GLY") {
                     last_main_chain_bead->bead_ref_mw += 1.01f;
                     // if ( advanced_config.debug_1 )
                     // {
                     //    puts("pass 2b GLY adjustment +1");
                     // }
                  }
               } // PRO
               last_main_chain_bead = this_atom;
            }

            // fix up mw, vol at end for broken end when PBR rule is on
            if ( misc.pb_rule_on &&
                 this_atom->p_residue->type == 0 &&
                 this_atom->is_bead &&
                 this_atom->chain == 0 &&
                 broken_chain_end.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 !broken_chain_head.count(QString("%1|%2")
                                        .arg(this_atom->resSeq)
                                        .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO" &&
                 (k || this_atom->name != "N")
                 )
            {
               // if ( advanced_config.debug_1 )
               // {
               //    printf("pass 2b broken end adjustment %s %s %s org mw %.4f vol %.4f\n",
               //           this_atom->name.toLatin1().data(),
               //           this_atom->resName.toLatin1().data(),
               //           this_atom->resSeq.toLatin1().data(),
               //           this_atom->bead_ref_mw,
               //           this_atom->bead_ref_volume);
               // }
               if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
               {
                  int pos = multi_residue_map["PBR-NO-OXT"][0];
                  this_atom->bead_ref_volume = residue_list[pos].r_bead[0].volume;
                  this_atom->bead_ref_mw = residue_list[pos].r_bead[0].mw;
                  this_atom->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
                  if (this_atom->resName == "GLY") {
                     this_atom->bead_ref_mw += 1.01f;
                     // if ( advanced_config.debug_1 )
                     // {
                     //    puts("pass 2b GLY adjustment +1 on broken end");
                     // }
                  }
                  // if ( advanced_config.debug_1 )
                  // {
                  //    printf("pass 2b broken end adjustment %s %s %s new mw %.4f vol %.4f\n",
                  //           this_atom->name.toLatin1().data(),
                  //           this_atom->resName.toLatin1().data(),
                  //           this_atom->resSeq.toLatin1().data(),
                  //           this_atom->bead_ref_mw,
                  //           this_atom->bead_ref_volume);
                  // }
               }
	       else
		 {
		   //    QColor save_color = editor->textColor();
		   //    editor->setTextColor("red");
		   us_log->log("Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.");
		   if ( us_udp_msg )
		     {
		       map < QString, QString > msging;
		       msging[ "_textarea" ] = "Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.";
		       
		       us_udp_msg->send_json( msging );
		       //sleep(1); 
		     }
		   accumulated_msgs += "Chain has broken end and PBR-NO-OXT isn't uniquely defined in the residue file.";
		   //    editor->setTextColor(save_color);
		 }
            }
         } // for k < atom.size()
         // fix up mw, vol at end for no OXT when PBR rule is on
         if ( misc.pb_rule_on &&
              last_main_chain_bead &&
              last_main_chain_bead->p_residue->type == 0 &&
              last_main_chain_bead->name != "PRO" &&
              !has_OXT[QString("%1|%2|%3")
               .arg(j)
               .arg(last_main_chain_bead->resName)
               .arg(last_main_chain_bead->resSeq)]  &&
              !broken_chain_head.count(QString("%1|%2")
                                       .arg(last_main_chain_bead->resSeq)
                                       .arg(last_main_chain_bead->resName)) )
         {
            // if ( advanced_config.debug_1 )
            // {
            //    puts("pass 2b missing OXT adjustment");
            // }
            if ( multi_residue_map["PBR-NO-OXT"].size() == 1 )
            {
               int pos = multi_residue_map["PBR-NO-OXT"][0];
               last_main_chain_bead->bead_ref_volume = residue_list[pos].r_bead[0].volume;
               last_main_chain_bead->bead_ref_mw = residue_list[pos].r_bead[0].mw;
               last_main_chain_bead->bead_computed_radius = pow(3 * last_main_chain_bead->bead_ref_volume / (4.0*M_PI), 1.0/3);
               if (last_main_chain_bead->resName == "GLY") {
                  last_main_chain_bead->bead_ref_mw += 1.01f;
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("pass 2b GLY adjustment +1 on last MC of atom");
                  // }
               }
            }
            else
	      {
		//    QColor save_color = editor->textColor();
		//    editor->setTextColor("red");
                us_log->log("Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.");
		if ( us_udp_msg )
		  {
		    map < QString, QString > msging;
		    msging[ "_textarea" ] = "Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.";
		    
		    us_udp_msg->send_json( msging );
		    //sleep(1); 
		  }
		accumulated_msgs += "Chain has no terminating OXT and PBR-NO-OXT isn't uniquely defined in the residue file.";
		//    editor->setTextColor(save_color);
	      }
         }
      } // for j < molecule.size()
   }

   // progress->setValue(ppos++); // 5
   // qApp->processEvents();
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

   // if (stopFlag)
   // {
   //    return -1;
   // }

   // pass 2c hydration

   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->is_bead) {
#if defined(DEBUG)
               printf("pass 2c hydration %s %s %d pm %d\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
#endif
               this_atom->bead_ref_volume_unhydrated = this_atom->bead_ref_volume;
               this_atom->bead_ref_volume += misc.hydrovol * this_atom->bead_hydration;
               this_atom->bead_computed_radius = pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
            }
         }
      }
   }

   // pass 3 determine visibility, exposed code, normalize cog position, final position determination
   // compute com of entire molecule

   // progress->setValue(ppos++); // 6
   // qApp->processEvents();
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
   

   // if (stopFlag)
   // {
   //    return -1;
   // }


#if defined(OLD_ASAB1_SC_COMPUTE)
   // pass 2d compute mc asa
   vector <float> bead_mc_asa;
# if defined(DEBUG1) || defined(DEBUG)
   printf("pass 2d\n"); fflush(stdout);
# endif
   // for (unsigned int i = 0; i < model_vector.size (); i++)   {
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);

            if (this_atom->active &&
                this_atom->chain == 0) {
# if defined(DEBUG)
               printf("pass 2d mc_asa %s %s %d pm %d\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial,
                      this_atom->placing_method); fflush(stdout);
# endif
               if(bead_mc_asa_hydro.size() < this_atom->resSeq + 1) {
                  bead_mc_asa_hydro.resize(this_atom->resSeq + 32);
               }
#warning broken by resSeq->QString
               bead_mc_asa[this_atom->resSeq] += this_atom->asa;
            }
         }
      }
   }
#endif // OLD_ASABA_SC_COMPUTE

#if defined(DEBUG1) || defined(DEBUG)
   printf("pass 3\n"); fflush(stdout);
#endif

   // for (unsigned int i = 0; i < model_vector.size (); i++) {
   {
      unsigned int i = current_model;

      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {

#if defined(OLD_ASAB1_SC_COMPUTE)
         float mc_asab1 = 0;
         QString mc_resname = "";
         unsigned int mc_resSeq = 0;
#endif

         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {


            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p3 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);
            this_atom->exposed_code = -1;
            if (this_atom->active &&
                this_atom->is_bead) {

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial); fflush(stdout);
#endif
               for (unsigned int m = 0; m < 3; m++) {
                  if (this_atom->bead_cog_mw) {
                     this_atom->bead_cog_coordinate.axis[m] /= this_atom->bead_cog_mw;
                  }
                  else 
                  {
                     this_atom->bead_cog_coordinate.axis[m] = 0;
                  }
               }

               if (this_atom->p_residue && this_atom->p_atom) {

#if defined(DEBUG)
                  printf("pass 3 active is bead %s %s %d bead assignment %d placing method %d\n",
                         this_atom->name.toLatin1().data(),
                         this_atom->resName.toLatin1().data(),
                         this_atom->serial,
                         this_atom->bead_assignment,
                         this_atom->placing_method
                         ); fflush(stdout);
#endif
                  switch (this_atom->placing_method) {

                  case 0 : // cog
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     // if (this_atom->bead_positioner) {
                     // fprintf(stderr, "warning: this bead had a atom claiming position & a bead placing method of cog! %s %s %d\n",
                     //   this_atom->name.toLatin1().data(),
                     //   this_atom->resName.toLatin1().data(),
                     //   this_atom->serial);
                     // }
                     break;
                  case 1 : // positioner
                     this_atom->bead_coordinate = this_atom->bead_position_coordinate;
                     break;
                  case 2 : // no positioning necessary
                     this_atom->bead_coordinate = this_atom->coordinate;
                     break;
                  default :
                     this_atom->bead_coordinate = this_atom->bead_cog_coordinate;
                     fprintf(stderr, "warning: unknown bead placing method %d %s %s %d <using cog!>\n",
                             this_atom->placing_method,
                             this_atom->name.toLatin1().data(),
                             this_atom->resName.toLatin1().data(),
                             this_atom->serial);
                     break;
                  }
               }
               else 
               {
                  fprintf(stderr, "serious internal error 1 on %s %s %d, quitting\n",
                          this_atom->name.toLatin1().data(),
                          this_atom->resName.toLatin1().data(),
                          this_atom->serial);
                  exit(-1);
                  break;
               }
#if defined(DEBUG) || defined(OLD_ASAB1_SC_COMPUTE)
               printf("pass 3 active is bead %s %s %s checkpoint 1\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->resSeq.toLatin1().data()); fflush(stdout);
#endif
               this_atom->visibility = (this_atom->bead_asa >= asa_hydro.threshold);
#if defined(OLD_ASAB1_SC_COMPUTE)
               if (this_atom->chain == 1) {
                  printf("visibility was %d is ", this_atom->visibility);
                  this_atom->visibility = (this_atom->bead_asa + bead_mc_asa[this_atom->resSeq] >= asa_hydro.threshold);
                  printf("%d\n", this_atom->visibility);
               }
#endif

#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 2\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial); fflush(stdout);
#endif
               if (!create_beads_normally ||
                   this_atom->visibility ||
                   !asa_hydro.calculation) {
                  this_atom->exposed_code = 1;  // exposed
               }
               else 
               {
                  if (this_atom->chain == 0) {
                     this_atom->exposed_code = 10;  // main chain, buried
                  }
                  if (this_atom->chain == 1) {
                     this_atom->exposed_code = 6;   // side chain, buried
                  }
               }
#if defined(DEBUG)
               printf("pass 3 active is bead %s %s %d checkpoint 3\n",
                      this_atom->name.toLatin1().data(),
                      this_atom->resName.toLatin1().data(),
                      this_atom->serial); fflush(stdout);
#endif

#if defined(OLD_ASAB1_SC_COMPUTE)
               if(this_atom->chain == 0) {
                  mc_asab1 = this_atom->bead_asa;
                  mc_resname = this_atom->resName;
                  mc_resSeq = this_atom->resSeq;
                  printf("saving last mc asa %.2f\n", mc_asab1);
               }
#endif
            }
            else 
            {
               this_atom->placing_method = -1;
            }
         }
      }
   }

   // // pass 4 print results
   // progress->setValue(ppos++); // 7
   // qApp->processEvents();
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

   // if (stopFlag)
   // {
   //    return -1;
   // }

#if defined(DEBUG)
   printf("model~molecule~atom~name~residue~chainID~"
          "position~active~radius~asa~mw~"
          "bead #~chain~serial~is_bead~bead_asa~visible~code/color~"
          "bead mw~position controlled?~bead positioner~baric method~bead hydration~bead color~"
          "bead ref volume~bead ref mw~bead computed radius~mw match?~"
          "position_coordinate~cog position~use position\n");
   // for (unsigned int i = 0; i < model_vector.size (); i++)
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
            // printf("p4 i j k %d %d %d %lx\n", i, j, k, this_atom->p_atom); fflush(stdout);

            // printf("model %d mol %d atm %d nam %s res %s xyz [%f,%f,%f] act %s rads %f asa %f bead # %d chain %d serl %d is_bead %s bead_asa %f vis %s code %d pos? %s pos_co [%f,%f,%f] cog [%f,%f,%f] use [%f, %f, %f]\n",
            printf("%d~%d~%d~%s~%s~%s~"
                   "[%f,%f,%f]~%s~%f~%f~%f~"
                   "%d~%d~%d~%s~%f~%s~"
                   "%d~%f~%s~%s~%d~%u~%u~"
                   "%f~%f~%f~%s~"
                   "[%f,%f,%f]~[%f,%f,%f]~[%f, %f, %f]\n",

                   i, j, k,
                   this_atom->name.toLatin1().data(),
                   this_atom->resName.toLatin1().data(),
                   this_atom->chainID.toLatin1().data(),

                   this_atom->coordinate.axis[0],
                   this_atom->coordinate.axis[1],
                   this_atom->coordinate.axis[2],
                   this_atom->active ? "Y" : "N",
                   this_atom->radius,
                   this_atom->asa,
                   this_atom->mw,

                   this_atom->bead_assignment,
                   this_atom->chain,
                   this_atom->serial,
                   this_atom->is_bead ? "Y" : "N",
                   this_atom->bead_asa,
                   this_atom->visibility ? "Y" : "N",

                   this_atom->exposed_code,
                   this_atom->bead_mw,
                   this_atom->bead_positioner ? "Y" : "N",
                   this_atom->active ? (this_atom->bead_positioner ? "Y" : "N") : "Inactive",
                   this_atom->placing_method,
                   this_atom->bead_hydration,
                   this_atom->bead_color,

                   this_atom->bead_ref_volume,
                   this_atom->bead_ref_mw,
                   this_atom->bead_computed_radius,
                   fabs(this_atom->bead_ref_mw - this_atom->bead_mw) < .03 ? "Y" : "N",

                   this_atom->bead_position_coordinate.axis[0],
                   this_atom->bead_position_coordinate.axis[1],
                   this_atom->bead_position_coordinate.axis[2],
                   this_atom->bead_cog_coordinate.axis[0],
                   this_atom->bead_cog_coordinate.axis[1],
                   this_atom->bead_cog_coordinate.axis[2],
                   this_atom->bead_coordinate.axis[0],
                   this_atom->bead_coordinate.axis[1],
                   this_atom->bead_coordinate.axis[2]
                   ); fflush(stdout);
         }
      }
   }
#endif
   // build vector of beads
   bead_model.clear( );
   // #define DEBUG_MOD
#if defined(DEBUG_MOD)
   vector<PDB_atom> dbg_model;
#endif
   //  for (unsigned int i = 0; i < model_vector.size (); i++)
#if defined(DEBUG_MW)
   double tot_atom_mw = 0e0;
   double tot_bead_mw = 0e0;
   double prev_bead_mw = 0e0;
   double prev_atom_mw = 0e0;
   int bead_count = 0;
   int atom_count = 0;
   int last_asgn = -1;
   QString last_res = "";
   QString last_resName = "";
#endif
   {
      unsigned int i = current_model;
      for (unsigned int j = 0; j < model_vector[i].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[i].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &(model_vector[i].molecule[j].atom[k]);
#if defined(DEBUG_MOD)
            this_atom->bead_number = 0;
            dbg_model.push_back(*this_atom);
#endif
            if(this_atom->active &&
               this_atom->is_bead) {
               this_atom->bead_number = bead_model.size();
               this_atom->bead_actual_radius = this_atom->bead_computed_radius;
#if defined(DEBUG_MW)
               if(bead_count) {
                  printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
                         last_res.toLatin1().data(),
                         last_resName.toLatin1().data(),
                         last_asgn,
                         prev_bead_mw,
                         prev_atom_mw,
                         prev_bead_mw -
                         prev_atom_mw
                         );
               }
               prev_bead_mw = 0;
               prev_atom_mw = 0;
               bead_count++;
               tot_bead_mw += this_atom->bead_ref_mw;
               prev_bead_mw += this_atom->bead_ref_mw;
               last_asgn = (int)this_atom->bead_assignment;
               last_res = this_atom->resSeq;
               last_resName = this_atom->resName;
#endif
               bead_model.push_back(*this_atom);
            }
#if defined(DEBUG_MW)
            if(this_atom->active) {
               atom_count++;
               tot_atom_mw += this_atom->mw;
               prev_atom_mw += this_atom->mw;
            }
#endif
         }
      }
   }
#if defined(DEBUG_MW)
   printf("res %d %s bead %d bead_mw %f sum atom mw %f diff %f\n",
          last_res,
          last_resName.toLatin1().data(),
          last_asgn,
          prev_bead_mw,
          prev_atom_mw,
          prev_bead_mw -
          prev_atom_mw
          );
   printf("~~tot atom %d mw %f tot bead %d mw %f beads_size %u\n",
          atom_count,
          tot_atom_mw,
          bead_count,
          tot_bead_mw,
          (unsigned int)bead_model.size());
#endif

   // write_bead_asa(somo_dir + SLASH +
   //                project + QString("_%1").arg( model_name( current_model ) ) +
   //                QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
   //                + DOTSOMO + ".asa_res", &bead_model);

   // write_bead_asa(//somo_dir + SLASH +
   //                project + QString("_%1").arg(  current_model  ) +
   //                QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
   //                + DOTSOMO + ".asa_res", &bead_model);


// #if defined(DEBUG_MOD)
//    write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_debug" + DOTSOMO + ".tsv", &dbg_model);
// #endif
   us_log->log(QString("There are %1 beads in this model%2\n")
	       .arg(bead_model.size())
	       .arg(bd_mode ? "" : " before popping"));

   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "_textarea" ] = QString("There are %1 beads in this model%2\\n")
	       .arg(bead_model.size())
	       .arg(bd_mode ? "" : " before popping");
       
       us_udp_msg->send_json( msging );
       //sleep(1); 
     }
   accumulated_msgs +=  QString("There are %1 beads in this model%2\\n")
	       .arg(bead_model.size())
	       .arg(bd_mode ? "" : " before popping");

   
//    progress->setValue(ppos++); // 8
//    qApp->processEvents();
   ppos++;
   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }

//    if (stopFlag)
//    {
//       return -1;
//    }

   // #define DEBUG

   // popping radial reduction
#if !defined(POP_MC)
# define POP_MC              (1 << 0)
# define POP_SC              (1 << 1)
# define POP_MCSC            (1 << 2)
# define POP_EXPOSED         (1 << 3)
# define POP_BURIED          (1 << 4)
# define POP_ALL             (1 << 5)
# define RADIAL_REDUCTION    (1 << 6)
# define RR_MC               (1 << 7)
# define RR_SC               (1 << 8)
# define RR_MCSC             (1 << 9)
# define RR_EXPOSED          (1 << 10)
# define RR_BURIED           (1 << 11)
# define RR_ALL              (1 << 12)
# define OUTWARD_TRANSLATION (1 << 13)
# define RR_HIERC            (1 << 14)
# define MIN_OVERLAP 0.0
#endif

   // or sc fb Y rh Y rs Y to Y st Y ro Y 70.000000 1.000000 0.000000
   // or scmc fb Y rh N rs N to Y st N ro Y 1.000000 0.000000 70.000000
   // or bb fb Y rh Y rs N to N st Y ro N 0.000000 0.000000 0.000000

#if defined(DEBUG)
   printf("or sc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or scmc fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or bb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n"
          "or gb fb %s rh %s rs %s to %s st %s ro %s %f %f %f\n",
          sidechain_overlap.fuse_beads ? "Y" : "N",
          sidechain_overlap.remove_hierarch ? "Y" : "N",
          sidechain_overlap.remove_sync ? "Y" : "N",
          sidechain_overlap.translate_out ? "Y" : "N",
          sidechain_overlap.show_translate ? "Y" : "N",
          sidechain_overlap.remove_overlap ? "Y" : "N",
          sidechain_overlap.fuse_beads_percent,
          sidechain_overlap.remove_sync_percent,
          sidechain_overlap.remove_hierarch_percent,

          mainchain_overlap.fuse_beads ? "Y" : "N",
          mainchain_overlap.remove_hierarch ? "Y" : "N",
          mainchain_overlap.remove_sync ? "Y" : "N",
          mainchain_overlap.translate_out ? "Y" : "N",
          mainchain_overlap.show_translate ? "Y" : "N",
          mainchain_overlap.remove_overlap ? "Y" : "N",
          mainchain_overlap.fuse_beads_percent,
          mainchain_overlap.remove_sync_percent,
          mainchain_overlap.remove_hierarch_percent,

          buried_overlap.fuse_beads ? "Y" : "N",
          buried_overlap.remove_hierarch ? "Y" : "N",
          buried_overlap.remove_sync ? "Y" : "N",
          buried_overlap.translate_out ? "Y" : "N",
          buried_overlap.show_translate ? "Y" : "N",
          buried_overlap.remove_overlap ? "Y" : "N",
          buried_overlap.fuse_beads_percent,
          buried_overlap.remove_sync_percent,
          buried_overlap.remove_hierarch_percent,

          grid_overlap.fuse_beads ? "Y" : "N",
          grid_overlap.remove_hierarch ? "Y" : "N",
          grid_overlap.remove_sync ? "Y" : "N",
          grid_overlap.translate_out ? "Y" : "N",
          grid_overlap.show_translate ? "Y" : "N",
          grid_overlap.remove_overlap ? "Y" : "N",
          grid_overlap.fuse_beads_percent,
          grid_overlap.remove_sync_percent,
          grid_overlap.remove_hierarch_percent);

#endif

   int methods[] =
      {
         RADIAL_REDUCTION | RR_SC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_EXPOSED,
         RADIAL_REDUCTION | RR_MCSC | RR_BURIED,
      };

   if ( !bd_mode &&
        sidechain_overlap.fuse_beads ) {
      methods[0] |= POP_SC | POP_EXPOSED;
   }

   if ( !bd_mode &&
        mainchain_overlap.fuse_beads) {
      methods[1] |= POP_MCSC | POP_EXPOSED;
   }

   if ( !bd_mode &&
        buried_overlap.fuse_beads ) {
      methods[2] |= POP_ALL | POP_BURIED;
   }

   if ( sidechain_overlap.remove_hierarch ) {
      methods[0] |= RR_HIERC;
   }

   if ( mainchain_overlap.remove_hierarch ) {
      methods[1] |= RR_HIERC;
   }

   if ( buried_overlap.remove_hierarch ) {
      methods[2] |= RR_HIERC;
   }

   if ( sidechain_overlap.translate_out ) {
      methods[0] |= OUTWARD_TRANSLATION;
   }

   if ( mainchain_overlap.translate_out ) {
      methods[1] |= OUTWARD_TRANSLATION;
   }

   if ( buried_overlap.translate_out ) {
      methods[2] |= OUTWARD_TRANSLATION;
   }

   if ( !sidechain_overlap.remove_overlap ) {
      methods[0] = 0;
   }

   if ( !mainchain_overlap.remove_overlap ) {
      methods[1] = 0;
   }

   if ( !buried_overlap.remove_overlap ) {
      methods[2] = 0;
   }

   if ( no_ovlp_removal ||
        ( bd_mode && 
          !bd_options.do_rr ) ) {
      methods[0] = 0;
      methods[1] = 0;
      methods[2] = 0;
      // cout << "BD MODE REMOVED RR!\n"; 
   }

   float overlap[] =
      {
         (float) ( sidechain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( mainchain_overlap.fuse_beads_percent / 100.0 ),
         (float) ( buried_overlap.fuse_beads_percent / 100.0 )
      };

   float rr_overlap[] =
      {
         (float) ( ( sidechain_overlap.remove_hierarch ?
                     sidechain_overlap.remove_hierarch_percent : sidechain_overlap.remove_sync_percent) / 100.0 ),
         (float) ( ( mainchain_overlap.remove_hierarch ?
                     mainchain_overlap.remove_hierarch_percent : mainchain_overlap.remove_sync_percent) / 100.0 ),
         (float) ( ( buried_overlap.remove_hierarch ?
                     buried_overlap.remove_hierarch_percent : buried_overlap.remove_sync_percent) / 100.0 )
      };


// #if defined(WRITE_EXTRA_FILES)
//    write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO + ".tsv", &bead_model);
//    write_bead_spt(somo_tmp_dir + SLASH + "bead_model_start" + DOTSOMO, &bead_model);
// #endif
   for(unsigned int k = 0; k < sizeof(methods) / sizeof(int); k++) {

   stage_loop:

      int beads_popped = 0;

#if defined(DEBUG1) || defined(DEBUG)
      printf("popping stage %d %s%s%s%s%s%s%s%s%s%s%s%s%s%s%soverlap_reduction %f rroverlap %f\n",
             k,
             (methods[k] & POP_MC) ? "main chain " : "",
             (methods[k] & POP_SC) ? "side chain " : "",
             (methods[k] & POP_MCSC) ? "main & side chain " : "",
             (methods[k] & POP_EXPOSED) ? "exposed " : "",
             (methods[k] & POP_BURIED) ? "buried " : "",
             (methods[k] & POP_ALL) ? "all " : "",
             (methods[k] & RADIAL_REDUCTION) ? "radial reduction " : "",
             (methods[k] & RR_HIERC) ? "hierarchically " : "synchronously ",
             (methods[k] & RR_MC) ? "main chain " : "",
             (methods[k] & RR_SC) ? "side chain " : "",
             (methods[k] & RR_MCSC) ? "main & side chain " : "",
             (methods[k] & RR_EXPOSED) ? "exposed " : "",
             (methods[k] & RR_BURIED) ? "buried " : "",
             (methods[k] & RR_ALL) ? "all " : "",
             (methods[k] & OUTWARD_TRANSLATION) ? "outward translation " : "",
             overlap[k],
             rr_overlap[k]);
#endif

      if (overlap[k] < MIN_OVERLAP) {
         printf("using %f as minimum overlap\n", MIN_OVERLAP);
         overlap[k] = MIN_OVERLAP;
      }

      float max_intersection_volume;
      float intersection_volume = 0;
      int max_bead1 = 0;
      int max_bead2 = 0;
      unsigned iter = 0;
      bool overlaps_exist;
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif
      if ( !bd_mode && !no_ovlp_removal )
	{
	  us_log->log(QString("Begin popping stage %1\n").arg(k + 1));
	  if ( us_udp_msg )
	    {
	      map < QString, QString > msging;
	      msging[ "_textarea" ] = QString("Begin popping stage %1\\n").arg(k + 1); 
	      
	      us_udp_msg->send_json( msging );
	      //sleep(1); 
	    }
	  accumulated_msgs +=  QString("Begin popping stage %1\\n").arg(k + 1); 
	}
      
      // progress->setValue(ppos++); // 9, 10, 11
      // qApp->processEvents();

      ppos++;
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
	  msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}
      
      // if (stopFlag)
      // {
      //    return -1;
      // }

      do {
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_bp-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-bp-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
         iter++;
#if defined(DEBUG1) || defined(DEBUG)
         printf("popping iteration %d\n", iter++);
#endif
	 // int k_tot = k < sizeof(methods) / sizeof(int);
         
	 // if ( !no_ovlp_removal ) {
	 //   {
	 //     if ( us_udp_msg )
	 //       {
	 // 	 map < QString, QString > msging;
	 // 	 msging[ "progress_output" ] = QString("Stage %1 popping iteration %2").arg(k+1).arg(iter);
	 // 	 msging[ "progress1" ] = QString::number(double(k+1)/double(k_tot));
	 // 	 //msging[ "_progress" ] = QString::number(double(i+1)/double(npoints_x));
		 
	 // 	 us_udp_msg->send_json( msging );
	 // 	 //sleep(1);
	 //       }
	 //     //  lbl_core_progress->setText(QString("Stage %1 popping iteration %2").arg(k+1).arg(iter));
	 //   }
	 
         // qApp->processEvents();
         max_intersection_volume = -1;
         overlaps_exist = false;
         if (methods[k] & (POP_MC | POP_SC | POP_MCSC | POP_EXPOSED | POP_BURIED | POP_ALL)) {
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking popping stage %d beads %d %d on chains %d %d exposed %d %d active %s %s max iv %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_volume
                         );
#endif
                  if (bead_model[i].active &&
                      bead_model[j].active &&
                      ( ((methods[k] & POP_SC) &&
                         bead_model[i].chain == 1 &&
                         bead_model[j].chain == 1) ||
                        ((methods[k] & POP_MC) &&
                         bead_model[i].chain == 0 &&
                         bead_model[j].chain == 0) ||
                        ((methods[k] & POP_MCSC)
                         // &&
                         // (bead_model[i].chain != 1 ||
                         // bead_model[j].chain != 1))
                         ) ) &&
                      ( ((methods[k] & POP_EXPOSED) &&
                         bead_model[i].exposed_code == 1 &&
                         bead_model[j].exposed_code == 1) ||
                        ((methods[k] & POP_BURIED) &&
                         (bead_model[i].exposed_code != 1 ||
                          bead_model[j].exposed_code != 1)) ||
                        (methods[k] & POP_ALL) )) {
                     intersection_volume =
                        int_vol_2sphere(
                                        bead_model[i].bead_computed_radius,
                                        bead_model[j].bead_computed_radius,
                                        sqrt(
                                             pow(bead_model[i].bead_coordinate.axis[0] -
                                                 bead_model[j].bead_coordinate.axis[0], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[1] -
                                                 bead_model[j].bead_coordinate.axis[1], 2) +
                                             pow(bead_model[i].bead_coordinate.axis[2] -
                                                 bead_model[j].bead_coordinate.axis[2], 2)) );
#if defined(DEBUG)
                     printf("this overlap bead %u %u vol %f rv1 %f rv2 %f r1 %f r2 %f p1 [%f,%f,%f] p2 [%f,%f,%f]\n",
                            bead_model[i].serial,
                            bead_model[j].serial,
                            intersection_volume,
                            bead_model[i].bead_ref_volume,
                            bead_model[j].bead_ref_volume,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2]
                            );
#endif
                     if (intersection_volume > bead_model[i].bead_ref_volume * overlap[k] ||
                         intersection_volume > bead_model[j].bead_ref_volume * overlap[k]) {
                        overlaps_exist = true;
                        if (intersection_volume > max_intersection_volume) {
#if defined(DEBUG)
                           printf("best overlap so far bead %u %u vol %f\n",
                                  bead_model[i].serial,
                                  bead_model[j].serial,
                                  intersection_volume);
#endif
                           max_intersection_volume = intersection_volume;
                           max_bead1 = i;
                           max_bead2 = j;
                        }
                     }
                  }
               }
            }

            bool back_to_zero = false;
            if (overlaps_exist) {
               beads_popped++;
               // if ( !no_ovlp_removal ) {
               //    lbl_core_progress->setText(QString("Stage %1 popping iteration %2 beads popped %3").arg(k+1).arg(iter).arg(beads_popped));
               // }
               // qApp->processEvents();
               // //#define DEBUG_FUSED
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
               printf("popping beads %u %u int vol %f mw1 %f mw2 %f v1 %f v2 %f c1 [%f,%f,%f] c2 [%f,%f,%f]\n",
                      max_bead1,
                      max_bead2,
                      max_intersection_volume,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_mw,
                      bead_model[max_bead1].bead_ref_volume,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2],
                      bead_model[max_bead2].bead_coordinate.axis[0],
                      bead_model[max_bead2].bead_coordinate.axis[1],
                      bead_model[max_bead2].bead_coordinate.axis[2]
                      );
#endif
               if (bead_model[max_bead1].chain == 0 &&
                   bead_model[max_bead2].chain == 1) {
                  // always select the sc!
#if defined(DEBUG1) || defined(DEBUG) || defined(DEBUG_FUSED)
                  puts("swap beads");
#endif
                  int tmp = max_bead2;
                  max_bead2 = max_bead1;
                  max_bead1 = tmp;
               }
               if (bead_model[max_bead1].chain != bead_model[max_bead2].chain &&
                   k == 1) {
                  back_to_zero = true;
               }
               // bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead1]); ??
               bead_model[max_bead1].all_beads.push_back(&bead_model[max_bead2]);
               for (unsigned int n = 0; n < bead_model[max_bead2].all_beads.size(); n++) {
                  bead_model[max_bead1].all_beads.push_back(bead_model[max_bead2].all_beads[n]);
               }

               bead_model[max_bead2].active = false;
               for (unsigned int m = 0; m < 3; m++) {
                  bead_model[max_bead1].bead_coordinate.axis[m] *= bead_model[max_bead1].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] +=
                     bead_model[max_bead2].bead_coordinate.axis[m] * bead_model[max_bead2].bead_ref_mw;
                  bead_model[max_bead1].bead_coordinate.axis[m] /= bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               }
               bead_model[max_bead1].bead_ref_mw = bead_model[max_bead1].bead_ref_mw + bead_model[max_bead2].bead_ref_mw;
               bead_model[max_bead1].bead_ref_volume = bead_model[max_bead1].bead_ref_volume + bead_model[max_bead2].bead_ref_volume;
               // - max_intersection_volume;
               bead_model[max_bead1].bead_actual_radius =
                  bead_model[max_bead1].bead_computed_radius =
                  pow(3 * bead_model[max_bead1].bead_ref_volume / (4.0*M_PI), 1.0/3);
               // if fusing with a side chain bead, make sure the fused is side-chain
               // if (bead_model[max_bead2].chain) {
               //   bead_model[max_bead1].chain = 1;
               // }
               bead_model[max_bead1].normalized_ot_is_valid = false;
               bead_model[max_bead2].normalized_ot_is_valid = false;
#if defined(DEBUG)
               printf("after popping beads %d %d int volume %f radius %f mw %f vol %f coordinate [%f,%f,%f]\n",
                      bead_model[max_bead1].serial,
                      bead_model[max_bead2].serial,
                      intersection_volume,
                      bead_model[max_bead1].bead_computed_radius,
                      bead_model[max_bead1].bead_ref_mw,
                      bead_model[max_bead2].bead_ref_volume,
                      bead_model[max_bead1].bead_coordinate.axis[0],
                      bead_model[max_bead1].bead_coordinate.axis[1],
                      bead_model[max_bead1].bead_coordinate.axis[2]
                      );
#endif
               if (back_to_zero) {
		 us_log->log(QString("Beads popped %1, Go back to stage %2\n").arg(beads_popped).arg(k));
		 if ( us_udp_msg )
		   {
		     map < QString, QString > msging;
		     msging[ "_textarea" ] = QString("Beads popped %1, Go back to stage %2\\n").arg(beads_popped).arg(k); 
		     
		     us_udp_msg->send_json( msging );
		     //sleep(1); 
		   }
		 accumulated_msgs += QString("Beads popped %1, Go back to stage %2\\n").arg(beads_popped).arg(k); 
		 // printf("fused sc/mc bead in stage SC/MC, back to stage SC\n");
                  k = 0;
                  ppos -= 4;
                  goto stage_loop;
               }
            }
         } // if pop method
         // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ap-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
         // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ap-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
      } while(overlaps_exist);
#if defined(TIMING)
      gettimeofday(&end_tv, NULL);
      printf("popping %d time %lu\n",
             k,
             1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
             start_tv.tv_usec);
      fflush(stdout);
#endif
// #if defined(WRITE_EXTRA_FILES)
//       write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
//       write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_pop-%1").arg(k) + DOTSOMO, &bead_model);
// #endif
      

      // printf("stage %d beads popped %d\n", k, beads_popped);
      
      //progress->setValue(ppos++); // 12,13,14

      ppos++;
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
	}

      if ( !no_ovlp_removal && ( !bd_mode || bd_options.do_rr  ) )
      {
         us_log->log(QString("Beads popped %1.\nBegin radial reduction stage %2\n").arg(beads_popped).arg(k + 1));
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString("Beads popped %1.\\nBegin radial reduction stage %2\\n").arg(beads_popped).arg(k + 1);
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1); 
	   }
	 accumulated_msgs += QString("Beads popped %1.\\nBegin radial reduction stage %2\\n").arg(beads_popped).arg(k + 1);

      }
      // qApp->processEvents();
      // if (stopFlag)
      // {
      //    return -1;
      // }


      // radial reduction phase
#if defined(TIMING)
      gettimeofday(&start_tv, NULL);
#endif

      if (methods[k] & RADIAL_REDUCTION) {
         BPair pair;
         vector <BPair> pairs;

         vector <bool> reduced;
         vector <bool> reduced_any; // this is for a final recomputation of the volumes
         vector <bool> last_reduced; // to prevent rescreening
         reduced.resize(bead_model.size());
         reduced_any.resize(bead_model.size());
         last_reduced.resize(bead_model.size());

         for (unsigned int i = 0; i < bead_model.size(); i++) {
            reduced_any[i] = false;
            last_reduced[i] = true;
         }

         int iter = 0;
         int count;
         float max_intersection_length;
         // bool tb[bead_model.size() * bead_model.size()];
         // printf("sizeof tb %d, bm.size^2 %d\n",
         //     sizeof(tb), bead_model.size() * bead_model.size());
#if defined(DEBUG_OVERLAP)
         overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance);
#endif
         if (methods[k] & RR_HIERC) {
#if defined(DEBUG1) || defined(DEBUG)
            printf("preprocessing processing hierarchical radial reduction\n");
#endif
            // lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            // qApp->processEvents();
            max_intersection_length = 0;
            pairs.clear( );
            count = 0;
            // build list of intersecting pairs
            for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
               for (unsigned int j = i + 1; j < bead_model.size(); j++) {
#if defined(DEBUG)
                  printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                         k, i, j,
                         bead_model[i].chain,
                         bead_model[j].chain,
                         bead_model[i].exposed_code,
                         bead_model[j].exposed_code,
                         bead_model[i].active ? "Y" : "N",
                         bead_model[j].active ? "Y" : "N",
                         max_intersection_length
                         );
#endif
                  if (
                      bead_model[i].active &&
                      bead_model[j].active &&
                      (methods[k] & RR_MCSC ||
                       ((methods[k] & RR_SC) &&
                        bead_model[i].chain == 1 &&
                        bead_model[j].chain == 1)) &&
                      ((methods[k] & RR_BURIED) ||
                       (bead_model[i].exposed_code == 1 ||
                        bead_model[j].exposed_code == 1)) &&
                      bead_model[i].bead_computed_radius > TOLERANCE &&
                      bead_model[j].bead_computed_radius > TOLERANCE
                      ) {

                     float separation =
                        bead_model[i].bead_computed_radius +
                        bead_model[j].bead_computed_radius -
                        sqrt(
                             pow(bead_model[i].bead_coordinate.axis[0] -
                                 bead_model[j].bead_coordinate.axis[0], 2) +
                             pow(bead_model[i].bead_coordinate.axis[1] -
                                 bead_model[j].bead_coordinate.axis[1], 2) +
                             pow(bead_model[i].bead_coordinate.axis[2] -
                                 bead_model[j].bead_coordinate.axis[2], 2));

#if defined(DEBUG)
                     printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                            i, j,
                            bead_model[i].bead_computed_radius,
                            bead_model[j].bead_computed_radius,
                            bead_model[i].bead_coordinate.axis[0],
                            bead_model[i].bead_coordinate.axis[1],
                            bead_model[i].bead_coordinate.axis[2],
                            bead_model[j].bead_coordinate.axis[0],
                            bead_model[j].bead_coordinate.axis[1],
                            bead_model[j].bead_coordinate.axis[2],
                            separation);
#endif
                     if (separation <= TOLERANCE) {
                        continue;
                     }

                     pair.i = i;
                     pair.j = j;
                     pair.separation = separation;
                     pair.active = true;
                     pairs.push_back(pair);
                     count++;
                  }
               }
            }
            // // ok, now we have the list of pairs

	    // int k_tot = int( sizeof(methods));
          
	    // if ( us_udp_msg )
	    //   {
	    // 	map < QString, QString > msging;
	    // 	msging[ "progress_output" ] = QString("Stage %1 hierarchical radial reduction").arg(k+1);
	    // 	msging[ "progress1" ] = QString::number(double(k+1)/double(k_tot));
	 	
	    // 	us_udp_msg->send_json( msging );
	    // 	//sleep(1);
	    //   }
	    // lbl_core_progress->setText(QString("Stage %1 hierarchical radial reduction").arg(k+1));
            // qApp->processEvents();
            
	    max_bead1 =
               max_bead2 = -1;
            float radius_delta;
            do {
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing hierarchical radial reduction iteration %d\n", iter);
#endif
               // lbl_core_progress->setText(QString("Stage %1 hierarch. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
               // qApp->processEvents();
               max_intersection_length = 0;
               int max_pair = -1;
               count = 0;
               for (unsigned int i = 0; i < pairs.size(); i++) {
#if defined(DEBUG_OVERLAP)
                  printf("pair %d %d sep %f %s %s\n",
                         pairs[i].i, pairs[i].j, pairs[i].separation, pairs[i].active ? "active" : "not active",
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2 ? "needs recompute of separation" : "separation valid");
#endif
                  if (pairs[i].active) {
                     if (
                         pairs[i].i == max_bead1 ||
                         pairs[i].j == max_bead1 ||
                         pairs[i].i == max_bead2 ||
                         pairs[i].j == max_bead2
                         ) {
                        pairs[i].separation =
                           bead_model[pairs[i].i].bead_computed_radius +
                           bead_model[pairs[i].j].bead_computed_radius -
                           sqrt(
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[0] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[0], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[1] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[1], 2) +
                                pow(bead_model[pairs[i].i].bead_coordinate.axis[2] -
                                    bead_model[pairs[i].j].bead_coordinate.axis[2], 2));
                        pairs[i].active = true;
                     }
                     if (pairs[i].separation > max_intersection_length) {
                        max_intersection_length = pairs[i].separation;
                        max_pair = i;
                     }
                  }
               }

               if (max_intersection_length > TOLERANCE) {
                  count++;
                  pairs[max_pair].active = false;
                  max_bead1 = pairs[max_pair].i;
                  max_bead2 = pairs[max_pair].j;
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction hierc iteration %d pair %d processed %d\n", iter, max_pair, count);
                  printf("reducing beads %d %d\n", max_bead1, max_bead2);
#endif
                  do {
                     if (methods[k] & OUTWARD_TRANSLATION ||
                         ((bead_model[max_bead1].chain == 1 ||
                           bead_model[max_bead2].chain == 1) &&
                          methods[0] & OUTWARD_TRANSLATION)) {
                        // new 1 step ot
                        if((methods[k] & RR_MCSC &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))
                           ) {
                           // new 1 bead 1 OT / treat as no ot...
                           int use_bead;
                           if (bead_model[max_bead1].chain == 1) {
                              use_bead = max_bead2;
                           }
                           else 
                           {
                              use_bead = max_bead1;
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           int use_bead = max_bead1;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           use_bead = max_bead2;
                           if (!bead_model[use_bead].normalized_ot_is_valid) {
                              float norm = 0.0;
                              for (unsigned int l = 0; l < 3; l++) {
                                 bead_model[use_bead].normalized_ot.axis[l] =
                                    bead_model[use_bead].bead_coordinate.axis[l] -
                                    molecular_cog[l];
                                 norm +=
                                    bead_model[use_bead].normalized_ot.axis[l] *
                                    bead_model[use_bead].normalized_ot.axis[l];
                              }
                              norm = sqrt(norm);
                              if (norm) {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                 }
                                 bead_model[use_bead].normalized_ot_is_valid = true;
                              }
                              else 
                              {
                                 printf("wow! bead %d is at the molecular cog!\n", use_bead);
                              }
                           }
                           // we need to handle 1 fixed case and
                           // the slight potential of one being at the molecular cog
                           reduced_any[max_bead1] = true;
                           reduced_any[max_bead2] = true;
                           outward_translate_2_spheres(
                                                       &bead_model[max_bead1].bead_computed_radius,
                                                       &bead_model[max_bead2].bead_computed_radius,
                                                       bead_model[max_bead1].bead_coordinate.axis,
                                                       bead_model[max_bead2].bead_coordinate.axis,
                                                       bead_model[max_bead1].normalized_ot.axis,
                                                       bead_model[max_bead2].normalized_ot.axis
                                                       );
                        }
                     }
                     else 
                     {
                        // no outward translation is required for either bead
                        // are we shrinking just 1 bead ... if we are dealing with buried beads, then
                        // only buried beads should be shrunk, not exposed beads
#if defined(DEBUG2)
                        printf("no outward translation is required\n");
#endif
                        if(methods[k] & RR_BURIED &&
                           bead_model[max_bead1].exposed_code == 1 &&
                           bead_model[max_bead2].exposed_code == 1) {
                           printf("what are we doing here?  buried and two exposed??\n");
                           // exit(-1);
                        }
                        if(methods[k] & RR_MCSC &&
                           !(methods[k] & RR_BURIED) &&
                           bead_model[max_bead1].chain == 1 &&
                           bead_model[max_bead2].chain == 1) {
                           printf("what are we doing here?  dealing with 2 SC's on the MCSC run??\n");
                           // exit(-1);
                        }
                        if((methods[k] & RR_BURIED &&
                            (bead_model[max_bead1].exposed_code == 1 ||
                             bead_model[max_bead2].exposed_code == 1)) ||
                           (methods[k] & RR_MCSC &&
                            !(methods[k] & RR_BURIED) &&
                            (bead_model[max_bead1].chain == 1 ||
                             bead_model[max_bead2].chain == 1))) {
                           // only one bead to shrink, since
                           // we are either buried with one of the beads exposed or
                           // on the MCSC and one of the beads is SC
                           int use_bead;
                           if (methods[k] & RR_BURIED) {
                              if (bead_model[max_bead1].exposed_code == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           else 
                           {
                              if (bead_model[max_bead1].chain == 1) {
                                 use_bead = max_bead2;
                              }
                              else 
                              {
                                 use_bead = max_bead1;
                              }
                           }
                           // one bead to shrink
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta = max_intersection_length;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required, one bead to shrink, radius delta %f  cr %f %f\n",
                                  use_bead,
                                  radius_delta,
                                  bead_model[use_bead].bead_computed_radius,
                                  bead_model[use_bead].bead_computed_radius - radius_delta
                                  );
#endif
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                        else 
                        {
                           // two beads to shrink
                           int use_bead = max_bead1;
#if defined(DEBUG2)
                           printf("use bead %d no outward translation is required\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           radius_delta =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead1].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
                           float radius_delta2 =
                              // bead_model[use_bead].bead_actual_radius * rr_overlap[k];
                              max_intersection_length * bead_model[max_bead2].bead_computed_radius /
                              (bead_model[max_bead1].bead_computed_radius + bead_model[max_bead2].bead_computed_radius);
#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE / 2);
                              reduced[use_bead] = false;
                           }

#if defined(DEBUG2)
                           printf("intersection len %f recomputed %f radius delta %f r1 %f r2 %f\n",
                                  max_intersection_length,
                                  bead_model[max_bead1].bead_computed_radius +
                                  bead_model[max_bead2].bead_computed_radius -
                                  sqrt(
                                       pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                           bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                           bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                       pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                           bead_model[max_bead2].bead_coordinate.axis[2], 2)),
                                  radius_delta,
                                  bead_model[max_bead1].bead_computed_radius,
                                  bead_model[max_bead2].bead_computed_radius
                                  );
#endif
                           use_bead = max_bead2;
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = radius_delta2;
                           if (radius_delta > bead_model[use_bead].bead_computed_radius) {
                              radius_delta = bead_model[use_bead].bead_computed_radius;
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= 0) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)(TOLERANCE - 1e-5);
                              reduced[use_bead] = false;
                           }
                        }
                     }
#if defined(DEBUG2)
                     printf("b1r %f b2r %f current separation %f\n",
                            bead_model[max_bead1].bead_computed_radius,
                            bead_model[max_bead2].bead_computed_radius,
                            bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)));
                     printf("flags %s %s %s %s\n",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE ? "Y" : "N",
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N",
                            bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                            bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                            (bead_model[max_bead1].bead_computed_radius +
                             bead_model[max_bead2].bead_computed_radius -
                             sqrt(
                                  pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                      bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                      bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                  pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                      bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE) ? "Y" : "N");
#endif
                  } while (
                           bead_model[max_bead1].bead_computed_radius > TOLERANCE &&
                           bead_model[max_bead2].bead_computed_radius > TOLERANCE &&
                           (bead_model[max_bead1].bead_computed_radius +
                            bead_model[max_bead2].bead_computed_radius -
                            sqrt(
                                 pow(bead_model[max_bead1].bead_coordinate.axis[0] -
                                     bead_model[max_bead2].bead_coordinate.axis[0], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[1] -
                                     bead_model[max_bead2].bead_coordinate.axis[1], 2) +
                                 pow(bead_model[max_bead1].bead_coordinate.axis[2] -
                                     bead_model[max_bead2].bead_coordinate.axis[2], 2)) > TOLERANCE));
#if defined(DEBUG2)
                  printf("out of while 1\n");
#endif
               } // if max intersection length > TOLERANCE
            } while(count);
#if defined(DEBUG2)
            printf("out of while 2\n");
#endif
         }
         else 
         {
            // // synchronous reduction
            // lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction").arg(k+1));
            // qApp->processEvents();
#if defined(USE_THREADS)
            unsigned int threads = numThreads;
            vector < radial_reduction_thr_t* > radial_reduction_thr_threads(threads);
            for ( unsigned int j = 0; j < threads; j++ )
            {
               radial_reduction_thr_threads[j] = new radial_reduction_thr_t(j);
               radial_reduction_thr_threads[j]->start();
            }
#endif
            do {
#if defined(DEBUG_OVERLAP)
               overlap_check(methods[k] & RR_SC ? true : false,
                             methods[k] & RR_MCSC ? true : false,
                             methods[k] & RR_BURIED ? true : false,
                             overlap_tolerance);
#endif
               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_br-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-br-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
               iter++;
#if defined(DEBUG1) || defined(DEBUG)
               printf("processing synchronous radial reduction iteration %d\n", iter);
#endif
               // lbl_core_progress->setText(QString("Stage %1 synchronous radial reduction iteration %2").arg(k+1).arg(iter));
               // qApp->processEvents();

               if(iter > 10000) {
                  printf("too many iterations\n");
                  exit(-1);
               }
               max_intersection_length = 0;
               pairs.clear( );
               count = 0;
               reduced[bead_model.size() - 1] = false;

#if defined(USE_THREADS)
               {
                  unsigned int i;
                  unsigned int j;

                  vector < BPair > my_pairs[ MAX_THREADS ];
                  for ( j = 0; j < threads; j++ )
                  {
# if defined(DEBUG_THREAD)
                     cout << "thread " << j << endl;
# endif
                     radial_reduction_thr_threads[j]->radial_reduction_thr_setup(
                                                        methods[k],
                                                        &bead_model,
                                                        &last_reduced,
                                                        &reduced,
                                                        &my_pairs[j],
                                                        threads,
                                                        overlap_tolerance
                                                        );

                  }

                  for ( j = 0; j < threads; j++ )
                  {
                     radial_reduction_thr_threads[j]->radial_reduction_thr_wait();
                  }

                  // merge results
                  for ( j = 0; j < threads; j++ )
                  {
                     for ( i = 0; i < my_pairs[j].size(); i++ ) 
                     {
                        pairs.push_back(my_pairs[j][i]);
                        
                        if (my_pairs[j][i].separation > max_intersection_length) {
                           max_intersection_length = my_pairs[j][i].separation;
                           max_bead1 = my_pairs[j][i].i;
                           max_bead2 = my_pairs[j][i].j;
                        }
                        count++;
                     }
                  }
               }
#else // !defined(USE_THREADS)
               for (unsigned int i = 0; i < bead_model.size() - 1; i++) {
                  reduced[i] = false;
                  if (1 || last_reduced[i]) {
                     for (unsigned int j = i + 1; j < bead_model.size(); j++) {
# if defined(DEBUGX)
                        printf("checking radial stage %d beads %d %d on chains %d %d exposed code %d %d active %s %s max il %f\n",
                               k, i, j,
                               bead_model[i].chain,
                               bead_model[j].chain,
                               bead_model[i].exposed_code,
                               bead_model[j].exposed_code,
                               bead_model[i].active ? "Y" : "N",
                               bead_model[j].active ? "Y" : "N",
                               max_intersection_length
                               );
# endif
                        if ((1 || last_reduced[j]) &&
                            bead_model[i].active &&
                            bead_model[j].active &&
                            (methods[k] & RR_MCSC ||
                             ((methods[k] & RR_SC) &&
                              bead_model[i].chain == 1 &&
                              bead_model[j].chain == 1)) &&
                            ((methods[k] & RR_BURIED) ||
                             (bead_model[i].exposed_code == 1 ||
                              bead_model[j].exposed_code == 1)) &&
                            bead_model[i].bead_computed_radius > TOLERANCE &&
                            bead_model[j].bead_computed_radius > TOLERANCE
                            ) {
                           float separation =
                              bead_model[i].bead_computed_radius +
                              bead_model[j].bead_computed_radius -
                              sqrt(
                                   pow(bead_model[i].bead_coordinate.axis[0] -
                                       bead_model[j].bead_coordinate.axis[0], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[1] -
                                       bead_model[j].bead_coordinate.axis[1], 2) +
                                   pow(bead_model[i].bead_coordinate.axis[2] -
                                       bead_model[j].bead_coordinate.axis[2], 2));


                           if (separation <= TOLERANCE) {
                              continue;
                           }

# if defined(DEBUG)
                           printf("beads %d %d with radii %f %f with coordinates [%f,%f,%f] [%f,%f,%f] have a sep of %f\n",
                                  i, j,
                                  bead_model[i].bead_computed_radius,
                                  bead_model[j].bead_computed_radius,
                                  bead_model[i].bead_coordinate.axis[0],
                                  bead_model[i].bead_coordinate.axis[1],
                                  bead_model[i].bead_coordinate.axis[2],
                                  bead_model[j].bead_coordinate.axis[0],
                                  bead_model[j].bead_coordinate.axis[1],
                                  bead_model[j].bead_coordinate.axis[2],
                                  separation);
# endif

                           if (separation > max_intersection_length) {
                              max_intersection_length = separation;
                              max_bead1 = i;
                              max_bead2 = j;
                           }
                           pair.i = i;
                           pair.j = j;
                           pair.separation = separation;
                           pairs.push_back(pair);
                           count++;
                        }
                     }
                  } // if last_reduced[i]
               }
#endif // !defined(USE_THERADS)

#if defined(DEBUG1) || defined(DEBUG)
               printf("processing radial reduction sync iteration %d pairs to process %d max int len %f\n", iter, count, max_intersection_length);
#endif
               if (max_intersection_length > TOLERANCE) {
#if defined(DEBUG1) || defined(DEBUG)
                  printf("processing radial reduction sync iteration %d pairs to process %d\n", iter, count);
#endif
                  // lbl_core_progress->setText(QString("Stage %1 synchron. red. it. %2 with %3 couples").arg(k+1).arg(iter).arg(pairs.size()));
                  // qApp->processEvents();
                  for (unsigned int i = 0; i < pairs.size(); i++) {
                     if (
                         !reduced[pairs[i].i] &&
                         (bead_model[pairs[i].i].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].i].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].i].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].i;
                        /*      if ( !(methods[k] & RR_MCSC) ||
                                bead_model[use_bead].exposed_code != 1 ||
                                bead_model[use_bead].chain == 0 ||
                                (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1) ) */
                        if(1) {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                     if (
                         !reduced[pairs[i].j] &&
                         (bead_model[pairs[i].j].exposed_code != 1 ||
                          methods[k] & RR_EXPOSED ||
                          methods[k] & RR_ALL) &&
                         (!(methods[k] & RR_MCSC) ||
                          bead_model[pairs[i].j].chain == 0 ||
                          (methods[k] & RR_BURIED &&
                           bead_model[pairs[i].j].exposed_code != 1))
                         ) {
                        int use_bead = pairs[i].j;
                        /* if ( !(methods[k] & RR_MCSC) ||
                           bead_model[use_bead].chain == 0 ||
                           bead_model[use_bead].exposed_code != 1 ||
                           (bead_model[pairs[i].i].chain == 1 &&
                           bead_model[pairs[i].j].chain == 1) ) */
                        {
#if defined(DEBUG)
                           printf("reducing beads %d\n", use_bead);
#endif
                           reduced[use_bead] = true;
                           reduced_any[use_bead] = true;
                           float radius_delta = bead_model[use_bead].bead_computed_radius * rr_overlap[k];
                           if (radius_delta < TOLERANCE) {
                              radius_delta = (float)TOLERANCE;
                           }
                           if (methods[k] & OUTWARD_TRANSLATION ||
                               (bead_model[pairs[i].i].chain == 1 &&
                                bead_model[pairs[i].j].chain == 1 &&
                                methods[0] & OUTWARD_TRANSLATION) ) {
#if defined(DEBUG)
                              printf("outward translation from [%f,%f,%f] to ",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                              if (!bead_model[use_bead].normalized_ot_is_valid) {
                                 float norm = 0.0;
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].normalized_ot.axis[l] =
                                       bead_model[use_bead].bead_coordinate.axis[l] -
                                       molecular_cog[l];
                                    norm +=
                                       bead_model[use_bead].normalized_ot.axis[l] *
                                       bead_model[use_bead].normalized_ot.axis[l];
                                 }
                                 norm = sqrt(norm);
                                 if (norm) {
                                    for (unsigned int l = 0; l < 3; l++) {
                                       bead_model[use_bead].normalized_ot.axis[l] /= norm;
                                       bead_model[use_bead].bead_coordinate.axis[l] +=
                                          radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                    }
                                    bead_model[use_bead].normalized_ot_is_valid = true;
                                 }
                                 else 
                                 {
                                    printf("wow! bead %d is at the molecular cog!\n", use_bead);
                                 }
                              }
                              else 
                              {
                                 for (unsigned int l = 0; l < 3; l++) {
                                    bead_model[use_bead].bead_coordinate.axis[l] +=
                                       radius_delta * bead_model[use_bead].normalized_ot.axis[l];
                                 }
                              }
#if defined(DEBUG)
                              printf(" [%f,%f,%f]\n",
                                     bead_model[use_bead].bead_coordinate.axis[0],
                                     bead_model[use_bead].bead_coordinate.axis[1],
                                     bead_model[use_bead].bead_coordinate.axis[2]);
#endif
                           }
                           bead_model[use_bead].bead_computed_radius -= radius_delta;
                           if (bead_model[use_bead].bead_computed_radius <= TOLERANCE) {
                              // this is to ignore this bead for further radial reduction regardless
                              bead_model[use_bead].bead_computed_radius = (float)TOLERANCE * 0.9999;
                              reduced[use_bead] = false;
                           }
                        }
                     }
                  }
               }
               last_reduced = reduced;

 //               qApp->processEvents();
//                if ( stopFlag )
//                {
// #if defined(USE_THREADS)
//                   {
//                      unsigned int j;
//                      // destroy
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
//                      }
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         radial_reduction_thr_threads[j]->wait();
//                      }
                     
//                      for ( j = 0; j < threads; j++ )
//                      {
//                         delete radial_reduction_thr_threads[j];
//                      }
//                   }
// #endif
//                   return -1;
//                }

               // write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_ar-%1-%2").arg(k).arg(iter) + DOTSOMO + ".tsv", &bead_model);
               // write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model-ar-%1-%2").arg(k).arg(iter) + DOTSOMO, &bead_model);
            } while(count);
#if defined(USE_THREADS)
            {
               unsigned int j;
               // destroy
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->radial_reduction_thr_shutdown();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  radial_reduction_thr_threads[j]->wait();
               }
               
               for ( j = 0; j < threads; j++ )
               {
                  delete radial_reduction_thr_threads[j];
               }
            }
#endif
         }

#if defined(TIMING)
         gettimeofday(&end_tv, NULL);
         printf("radial reduction %d time %lu\n",
                k,
                1000000l * (end_tv.tv_sec - start_tv.tv_sec) + end_tv.tv_usec -
                start_tv.tv_usec);
         fflush(stdout);
#endif
         // progress->setValue(ppos++); // 15,16,17
         // qApp->processEvents();

	 ppos++;
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(ppos)/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
	     msging[ "progress1" ] = QString::number(double(ppos)/double(mppos));
	     
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   }

         // if (stopFlag)
         // {
         //    return -1;
         // }

         // recompute volumes
         for (unsigned int i = 0; i < bead_model.size(); i++) {
            if (reduced_any[i]) {
#if defined(DEBUG1) || defined(DEBUG)
               printf("recomputing volume bead %d\n", i);
#endif
               bead_model[i].bead_ref_volume =
                  (4.0*M_PI/3.0) * pow(bead_model[i].bead_computed_radius, 3);
            }
         }
      }
#if defined(DEBUG_OVERLAP)
      if(overlap_check(methods[k] & RR_SC ? true : false,
                       methods[k] & RR_MCSC ? true : false,
                       methods[k] & RR_BURIED ? true : false,
                       overlap_tolerance)) {
         printf("internal over lap error... exiting!");
         exit(-1);
      }
#endif

// #if defined(WRITE_EXTRA_FILES)
//       write_bead_tsv(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO + ".tsv", &bead_model);
//       write_bead_spt(somo_tmp_dir + SLASH + QString("bead_model_rr-%1").arg(k) + DOTSOMO, &bead_model);
// #endif

   } // methods


// #if defined(WRITE_EXTRA_FILES)
//    write_bead_tsv(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO + ".tsv", &bead_model);
//    write_bead_spt(somo_tmp_dir + SLASH + "bead_model_end" + DOTSOMO, &bead_model);
// #endif
 
   //  write_bead_spt(somo_dir + SLASH + project + QString("_%1").arg( model_name( current_model ) ) +
   //       QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") +
   //       DOTSOMO, &bead_model);

   QString filename = 
      project + 
      QString( "_%1" ).arg( current_model + 1 );

   //   le_bead_model_file->setText( filename );

   write_bead_model(//somo_dir + SLASH + filename  +
                    filename + QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "") + DOTSOMO
                    , &bead_model);

   if ( !no_ovlp_removal && ( !bd_mode  ||  bd_options.do_rr  ) )
   {
      us_log->log("Finished with popping and radial reduction\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Finished with popping and radial reduction\\n";
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Finished with popping and radial reduction\\n"; 
   }
   // progress->setValue(mppos - (asa_hydro.recheck_beads ? 1 : 0));

   if ( us_udp_msg )
     {
       map < QString, QString > msging;
       msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number( int((double(mppos - (asa_hydro.recheck_beads ? 1 : 0))/double(mppos))*100.0) ) ); // arg(ppos).arg(mppos);
       msging[ "progress1" ] = QString::number(double(mppos - (asa_hydro.recheck_beads ? 1 : 0))/double(mppos));
       
       us_udp_msg->send_json( msging );
       //sleep(1);
     }
   
   
   if ( bd_mode || no_ovlp_removal )
     {
       if ( us_udp_msg )
	 {
	   map < QString, QString > msging;
	   msging[ "progress_output" ] = QString("SOMO model calculation: %1\%").arg(QString::number(100)); // arg(ppos).arg(mppos);
	   msging[ "progress1" ] = QString::number(1);
	   
	   us_udp_msg->send_json( msging );
	   //sleep(1);
	 }
       
       //progress->setValue(1,1);
       //lbl_core_progress->setText("");
     }
      
   // qApp->processEvents();
   // if (stopFlag)
   // {
   //    return -1;
   // }
   return 0;
}

bool US_Saxs_Util::read_pdb_hydro( QString filename, bool parameters_set_first_model)
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
         
	 
	 us_log->log(QString("PDB %1: %2").arg(str1.left(6)).arg(tmp_str));
	 if ( us_udp_msg )
	   {
	     map < QString, QString > msging;
	     msging[ "_textarea" ] = QString("PDB %1: %2\\n").arg(str1.left(6)).arg(tmp_str);
	     us_udp_msg->send_json( msging );
	     //sleep(1);
	   } 
	  accumulated_msgs += QString("PDB %1: %2\\n").arg(str1.left(6)).arg(tmp_str);
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
            if ( temp_chain.atom.size() ) {
               sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
            }
            temp_model.molecule.push_back(temp_chain); // add the last chain of this model
            // noticemsg += "Residue sequence from model " +
            // QString("%1").arg( model_vector.size() + 1 ) + ": \n";
	    
	    us_log->log("\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \n");
	    if ( us_udp_msg )
	      {
		map < QString, QString > msging;
		msging[ "_textarea" ] = "\\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \\n";
		us_udp_msg->send_json( msging );
		//sleep(1);
	      }   
	    accumulated_msgs +=  "\\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \\n";

	    // editor->append("\nResidue sequence from " + project +".pdb model " +
	    //    QString("%1").arg( temp_model.model_id ) + ": \n");
	    
            str = "";
            QString sstr = "";
            int sstr_pos = 0;
	    QString sstr_udp = "";
            int sstr_pos_udp = 0;

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
		     
		     sstr_udp += 
		       residue_short_names.count(temp_model.molecule[i].atom[j].resName) ? 
		       QString(residue_short_names[temp_model.molecule[i].atom[j].resName]) : "?"; 
		     sstr_pos_udp++;
                     if ( !( sstr_pos % 42 ) )
                     {
                        sstr += "\n";
                     }
		     if ( !( sstr_pos_udp % 42 ) )
                     {
                        sstr_udp += "\\n";
                     }
                     lastResSeq = temp_model.molecule[i].atom[j].resSeq;
                  }
               }
            }
            // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
            // {
            //   str += temp_model.residue[m].name + " ";
            // }
            // noticemsg += str + "\n";

	    //editor->append(us_tr("\nSequence in one letter code:\n"));
	    //editor->append(sstr + "\n\n");
	   
	    us_log->log(str);
	    if ( us_udp_msg )
	      {
		map < QString, QString > msging;
		msging[ "_textarea" ] = str;
		us_udp_msg->send_json( msging );
		//sleep(1);
	      }   
	    accumulated_msgs += str; 

	    us_log->log(us_tr("\nSequence in one letter code:\n"));
	    if ( us_udp_msg )
	      {
		map < QString, QString > msging;
		msging[ "_textarea" ] = us_tr("\\n\\nSequence in one letter code:\\n");
		us_udp_msg->send_json( msging );
		//sleep(1);
	      }   
	     accumulated_msgs += us_tr("\\n\\nSequence in one letter code:\\n");  
	    
	    us_log->log(sstr + "\n\n");
	    if ( us_udp_msg )
	      {
		map < QString, QString > msging;
		msging[ "_textarea" ] = sstr_udp + "\\n\\n";
		us_udp_msg->send_json( msging );
		//sleep(1);
	      }   
	    accumulated_msgs += sstr_udp + "\\n\\n";
	    

            
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
		    //noticemsg += QString("Atom %1 conformation A selected\n").arg(str1.mid(6,5));
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
      if ( temp_chain.atom.size() ) {
         if ( temp_chain.atom.size() ) {
            sulfur_pdb_chain_idx[ temp_chain.atom[ 0 ].chainID ].push_back( (unsigned int) temp_model.molecule.size() );
         }
         temp_model.molecule.push_back(temp_chain);
      }
      // SS_apply( temp_model, project );

      //editor->append("\nResidue sequence from " + project +".pdb:\n");
      us_log->log("\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "\\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \\n";
	    us_udp_msg->send_json( msging );
	  //sleep(1);
	}   
      accumulated_msgs += "\\nResidue sequence from " + project +".pdb model " + QString("%1").arg( temp_model.model_id ) + ": \\n";

      
      str = "";
      QString sstr = "";
      QString sstr_udp = "";
      int sstr_pos = 0;
      int sstr_pos_udp = 0;
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
	       
	       sstr_udp += 
		 residue_short_names.count(temp_model.molecule[i].atom[j].resName) ? 
		 QString(residue_short_names[temp_model.molecule[i].atom[j].resName]) : "?"; 
               sstr_pos_udp++;
               if ( !( sstr_pos % 42 ) )
               {
                  sstr += "\n";
               }
	       if ( !( sstr_pos_udp % 42 ) )
               {
                  sstr_udp += "\\n";
               }
               lastResSeq = temp_model.molecule[i].atom[j].resSeq;
            }
         }
      }
      // for (unsigned int m=0; m<temp_model.residue.size(); m++ )
      // {
      //   str += temp_model.residue[m].name + " ";
      // }

      // editor->append(str);
      // editor->append(us_tr("\nSequence in one letter code:\n"));
      // editor->append(sstr + "\n\n");
      
      us_log->log(str);
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = str;
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}   
      accumulated_msgs += str;
      
      us_log->log(us_tr("\nSequence in one letter code:\n"));
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = us_tr("\\n\\nSequence in one letter code:\\n");
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}   
      accumulated_msgs += us_tr("\\n\\nSequence in one letter code:\\n");
      
      us_log->log(sstr + "\n\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = sstr_udp + "\\n\\n";
	  us_udp_msg->send_json( msging );
	  //sleep(1);
	}   
      accumulated_msgs += sstr_udp + "\\n\\n";
	    
     
      //noticemsg += str;
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

   /* IF THE 1st MODEL DEFINED ***************/
   if (parameters_set_first_model)
     {
       model_vector.resize(1);
       model_vector_as_loaded.resize(1);
     }
   /* IF THE 1st MODEL DEFINED ***************/ 

   // cout << list_chainIDs(model_vector);
   //cout << list_chainIDs(model_vector_as_loaded);
   if ( !calc_mw() )
   {
      return false;
   }
   return true;
}

/* Write pdb */

// write a pdb file with connections
class sortable_uint {
public:
   unsigned int x;
   bool operator < (const sortable_uint& objIn) const
   {
      return x < objIn.x;
   }
};

QString US_Saxs_Util::pdb_jsmol_script( vector < PDB_atom > *model ) {
   QString qs;
   QStringList colormap;
   colormap
      << "0,0,0"         // 0 black
      << "0,0,255"       // 1 blue
      << "0,128,128"     // 2 greenblue
      << "0,255,255"     // 3 cyan
      << "255,0,0"       // 4 red
      << "255,0,255"     // 5 magenta
      << "255,165,0"     // 6 orange
      << "255,255,255"   // 7 white
      << "254,73,2"      // 8 redorange
      << "128.0,128"     // 9 purple
      << "0,255,0"       // 10 green
      << "0,255,255"     // 11 cyan
      << "254,73,2"      // 12 redorange
      << "238,130,238"   // 13 violet
      << "255,255,0"     // 14 yellow
      ;
   

   for ( unsigned int i = 0; i < model->size(); i++ ) {
      qs += QString( "{atomno=%1}.radius=%2;{atomno=%3}.color={%4};" )
         .arg( i + 1 )
         .arg( (*model)[i].bead_computed_radius )
         .arg( i + 1 )
         .arg( colormap[ get_color_util_saxs( &(*model)[i]) % (int) colormap.size() ] )
         ;
   }
   return qs;
}

int US_Saxs_Util::write_pdb_hydro( QString fname, vector < PDB_atom > *model )
{
   QFile fpdb;
   fpdb.setFileName(QString("%1.pdb").arg(fname));
   if ( !fpdb.open(QIODevice::WriteOnly) )
   {
      return -1;
   }
   QTextStream ts(&fpdb);
   ts << 
      QString("HEADER  US-SOMO BD Connection file %1\n").arg(fname);

   // write all the beads

   for ( unsigned int i = 0; i < model->size(); i++ ) 
   {
      if ( 
          (*model)[i].active
          // &&
          // ( bd_options.include_sc ||
          //  (*model)[i].chain == 0 )
          )
      {
         ts << 
            QString("")
            .sprintf(     
                     "ATOM   %4d  PB  UNK A%4d    %8.3f%8.3f%8.3f  1.00 10.00           PB  \n",
                     i + 1, i + 1,
                     (*model)[i].bead_coordinate.axis[0],
                     (*model)[i].bead_coordinate.axis[1],
                     (*model)[i].bead_coordinate.axis[2]
                     );
      }
   }

   // consolidate & symmetrize connections

   QRegExp rx("^(\\d+)~(\\d+)$");

   sortable_uint tmp_suint_i;
   sortable_uint tmp_suint_j;

   list < sortable_uint > connect_parents;
   map < sortable_uint, bool > parent_pushed;
   map < unsigned int, list < sortable_uint > > connect_list;

   for ( map < QString, bool >::iterator it = connection_active.begin();
         it != connection_active.end();
         it++ )
   {
      if ( connection_active[it->first] ) 
      {
         if ( rx.indexIn(it->first) == -1 ) 
         {
	   us_log->log("unexpected regexp extract failure (write_pdb)!\n");
	   if ( us_udp_msg )
	     {
	       map < QString, QString > msging;
	       msging[ "_textarea" ] = "unexpected regexp extract failure (write_pdb)!\\n"; 
	       
	       us_udp_msg->send_json( msging );
	       //sleep(1); 
	     }
	   accumulated_msgs += "unexpected regexp extract failure (write_pdb)!\\n"; 
	   return -1;
         }
         tmp_suint_i.x = rx.cap(1).toInt();
         tmp_suint_j.x = rx.cap(2).toInt();

         connect_list[tmp_suint_i.x].push_back(tmp_suint_j);
         connect_list[tmp_suint_j.x].push_back(tmp_suint_i);

         if ( !parent_pushed.count(tmp_suint_i) ) 
         {
            connect_parents.push_back(tmp_suint_i);
            parent_pushed[tmp_suint_i] = true;
         }
         if ( !parent_pushed.count(tmp_suint_j) ) 
         {
            connect_parents.push_back(tmp_suint_j);
            parent_pushed[tmp_suint_j] = true;
         }

      }
   }
   
   connect_parents.sort();

   for (
        list < sortable_uint > ::const_iterator it = connect_parents.begin(); 
        it != connect_parents.end(); 
        ++it 
        )
   {
      connect_list[it->x].sort();
   }

   for (
        list < sortable_uint > ::const_iterator it = connect_parents.begin(); 
        it != connect_parents.end(); 
        ++it 
        )
   {
      QString cbase = QString("").sprintf("CONECT%5d",it->x + 1);
      QString out;
      QString tmp = "";
      unsigned int j = 0;
      for (
           list < sortable_uint > ::const_iterator itj = connect_list[it->x].begin(); 
           itj != connect_list[it->x].end(); 
           ++itj
           )
      {
         tmp += QString("").sprintf("%5d",itj->x + 1);
         if ( !((j + 1) % 4) )
         {
            out += cbase + tmp + "\n";
            tmp = "";
         }
         j++;
      }
      if ( tmp != "" )
      {
         out += cbase + tmp + "\n";
      }
      ts << out;
   }
   ts << "END\n";
   fpdb.close();

//    if ( bd_options.show_pdb )
//    { // display pdb
//       QStringList argument;
// #if !defined(WIN32) && !defined(MAC)
//       argument.append("xterm");
//       argument.append("-e");
// #endif
// #if defined(BIN64)
//       argument.append(USglobal->config_list.system_dir + SLASH + "bin64" + SLASH + "rasmol");
// #else
//       argument.append(USglobal->config_list.system_dir + SLASH + "bin" + SLASH + "rasmol");
// #endif
//       argument.append(QFileInfo(fpdb.fileName()).fileName());
//       rasmol->setWorkingDirectory(QFileInfo(fpdb.fileName()).path());
//       rasmol->setArguments(argument);
//       if (!rasmol->start())
//       {
//          US_Static::us_message(us_tr("Please note:"), us_tr("There was a problem starting RASMOL\n"
//                                                      "Please check to make sure RASMOL is properly installed..."));
//       }
//    }
   
   return 0;
}

class sortable_PDB_atom {
public:
   PDB_atom pdb_atom;
   bool PRO_N_override;
   bool pb_rule_on;
   bool operator < (const sortable_PDB_atom& objIn) const
   {
      if (
          //   (PRO_N_override ? 0 : pdb_atom.bead_assignment)
          ( !pb_rule_on &&
            ( pdb_atom.bead_assignment < 
              objIn.pdb_atom.bead_assignment ) ) ? 1 : (
                                                pdb_atom.atom_assignment
                                                <
          //   (objIn.PRO_N_override ? 0 : objIn.pdb_atom.bead_assignment)
                                                objIn.pdb_atom.atom_assignment
                                                )
          )
      {
         return (true);
      }
      else
      {
         return (false);
      }
   }
};


int US_Saxs_Util::create_beads_hydro(QString *error_string, bool quiet)
{
   // us_qdebug( "create_beads"
   //         "\nresidue_atom_hybrid_map" );
   
   // for ( map<QString,QString>::iterator it = residue_atom_hybrid_map.begin();
   //       it != residue_atom_hybrid_map.end();
   //       ++it ) {
   //    us_qdebug( QString( "%1 -> %2" ).arg( it->first ).arg( it->second ) );
   // }

   // us_qdebug( "end of residue_atom_hybrid_map" );

   if ( !quiet ) 
   {
      us_log->log("Creating beads from atomic model\n");
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = "Creating beads from atomic model\\n";
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
      accumulated_msgs += "Creating beads from atomic model\\n";
      //qApp->processEvents();
   }
   active_atoms.clear( );

   // #define DEBUG_MM
#if defined(DEBUG_MM)
   {
      int no_of_atoms = 0;
      int no_of_molecules = model_vector[current_model].molecule.size();
      int i;
      for (i = 0; i < no_of_molecules; i++) {
         no_of_atoms +=  model_vector[current_model].molecule[i].atom.size();
      }

      us_log->log(QString("There are %1 atoms in %2 molecule(s) in this model\n").arg(no_of_atoms).arg(no_of_molecules));
      if ( us_udp_msg )
	{
	  map < QString, QString > msging;
	  msging[ "_textarea" ] = QString("There are %1 atoms in %2 molecule(s) in this model\\n").arg(no_of_atoms).arg(no_of_molecules); 
	  
	  us_udp_msg->send_json( msging );
	  //sleep(1); 
	}
       accumulated_msgs += QString("There are %1 atoms in %2 molecule(s) in this model\\n").arg(no_of_atoms).arg(no_of_molecules); 
   }
#endif
   get_atom_map(&model_vector[current_model]);

   QRegExp count_hydrogens("H(\\d)");

   for (unsigned int j = 0; j < model_vector[current_model].molecule.size(); j++)
   {
      QString last_resSeq = "";
      QString last_resName = "";
      for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size(); k++)
      {
         PDB_atom *this_atom = &(model_vector[current_model].molecule[j].atom[k]);

         QString count_idx =
            QString("%1|%2|%3")
            .arg(j)
            .arg(this_atom->resName)
            .arg(this_atom->resSeq);

         // initialize data
         this_atom->active = false;
         this_atom->asa = 0;
         this_atom->p_residue = 0;
         this_atom->p_atom = 0;
         this_atom->radius = 0;
         this_atom->bead_hydration = 0;
         this_atom->bead_color = 0;
         this_atom->bead_ref_volume = 0;
         this_atom->bead_ref_mw = 0;
         this_atom->bead_assignment = -1;
         this_atom->atom_assignment = -1;
         this_atom->chain = -1;
         this_atom->type = 99;

         // find residue in residues
         int respos = -1;
         // if ( advanced_config.debug_1 ||
         //      advanced_config.debug_2 )
         // {
         //    printf("residue search name %s resName %s\n",
         //           this_atom->name.toLatin1().data(),
         //           this_atom->resName.toLatin1().data());
         // }

         int restype = 99;
         if ( multi_residue_map.count(this_atom->resName) &&
              multi_residue_map[this_atom->resName].size() )
         {
            restype = residue_list[multi_residue_map[this_atom->resName][0]].type;
         }

         QString res_idx =
            QString("%1|%2")
            .arg(j)
            .arg(this_atom->resSeq);

         for (unsigned int m = 0; m < residue_list.size(); m++)
         {
            if ( residue_list[m].name == this_atom->resName &&
                 skip_residue.count(QString("%1|%2").arg(res_idx).arg(m)) &&
                 skip_residue[QString("%1|%2").arg(res_idx).arg(m)] == true )
            {
               // if ( advanced_config.debug_1 )
               // {
               //    cout << 
               //       QString("(create beads) skipping %1 %2\n").arg(res_idx).arg(m) << endl;
               // }
               continue;
            }

            if ((residue_list[m].name == this_atom->resName &&
                 (int)residue_list[m].r_atom.size() ==
                 atom_counts[count_idx] - has_OXT[count_idx] &&
                 this_atom->name != "OXT" &&
                 (k ||
                  this_atom->name != "N" ||
                  restype != 0 ||
                  broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) ||
                  this_atom->resName == "PRO" ||
                  !misc.pb_rule_on)) ||

                // if pb_rule is off, final OXT before P needs to use OXT-P
                (residue_list[m].name == "OXT"
                 && this_atom->name == "OXT" && (misc.pb_rule_on || this_atom->resName != "PRO")) ||
                (residue_list[m].name == "OXT-P"
                 && (this_atom->name == "OXT" && !misc.pb_rule_on && this_atom->resName == "PRO")) ||

                (!k &&
                 this_atom->name == "N" &&
                 restype == 0 &&                  
                 misc.pb_rule_on &&
                 residue_list[m].name == "N1" &&
                 !broken_chain_head.count(QString("%1|%2")
                                          .arg(this_atom->resSeq)
                                          .arg(this_atom->resName)) &&
                 this_atom->resName != "PRO"))
            {
               respos = (int) m;
               this_atom->p_residue = &(residue_list[m]);
               // if ( advanced_config.debug_1 ||
               //      advanced_config.debug_2 )
               // {
               //    printf("residue match %d resName %s \n", m, residue_list[m].name.toLatin1().data());
               // }
               break;
            }
         }
         if (respos == -1)
         {
            if ((this_atom->name != "H" && this_atom->name != "D"
                 && this_atom->resName != "DOD"
                 && this_atom->resName != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " ")))
            {
               error_string->append(QString("").sprintf("unknown residue molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.toLatin1().data(),
                                                        this_atom->resName.toLatin1().data(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
               return (US_SURFRACER_ERR_MISSING_RESIDUE);
            }
         } 
//          else 
//          {
//                if ( advanced_config.debug_1 ||
//                     advanced_config.debug_2 )
//                {
// #if defined(DEBUG)
//             printf("found residue pos %d\n", respos);
// #endif
//                }
//          }
         int atompos = -1;

         if (respos != -1)
         {
            // clear tmp_used if new resSeq
#if defined(DEBUG)
            printf("respos %d != -1 last used %s %s\n", respos, this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
            if (this_atom->resSeq != last_resSeq ||
                this_atom->resName != last_resName ||
                residue_list[respos].name == "OXT" ||
                residue_list[respos].name == "OXT-P" ||
                residue_list[respos].name == "N1")
            {
#if defined(DEBUG)
               printf("clear last used %s %s\n", this_atom->resSeq.toLatin1().data(), last_resSeq.toLatin1().data());
#endif
               for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
               {
                  residue_list[respos].r_atom[m].tmp_used = false;
               }
               last_resSeq = this_atom->resSeq;
               last_resName = this_atom->resName;
               if(residue_list[respos].name == "OXT" ||
                  residue_list[respos].name == "OXT-P" ||
                  residue_list[respos].name == "N1") {
                  last_resSeq = "";
               }
            }
#if defined(DEBUG)
            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
               if(residue_list[respos].r_atom[m].tmp_used) {
                  printf("used %u %u\n", respos, m);
               }
            }
#endif

            for (unsigned int m = 0; m < residue_list[respos].r_atom.size(); m++)
            {
#if defined(DEBUG)
               if(this_atom->name == "N" && !k && misc.pb_rule_on) {
                  printf("this_atom->name == N/N1 this residue_list[%d].r_atom[%d].name == %s\n",
                         respos, m, residue_list[respos].r_atom[m].name.toLatin1().data());
               }
#endif
               if (!residue_list[respos].r_atom[m].tmp_used &&
                   (residue_list[respos].r_atom[m].name == this_atom->name ||
                    (
                     this_atom->name == "N" &&
                     misc.pb_rule_on &&
                     !k &&
                     residue_list[respos].r_atom[m].name == "N1" &&
                     !broken_chain_head.count(QString("%1|%2")
                                              .arg(this_atom->resSeq)
                                              .arg(this_atom->resName)) &&
                     this_atom->resName != "PRO"
                     )
                    )
                   )
               {
                  residue_list[respos].r_atom[m].tmp_used = true;
                  this_atom->p_atom = &(residue_list[respos].r_atom[m]);
                  atompos = (int) m;
                  break;
               }
            }

            if (atompos == -1)
            {
               error_string->append(QString("").sprintf("unknown atom molecule %d atom %d name %s resname %s coord [%f,%f,%f]\n",
                                                        j + 1, k, this_atom->name.toLatin1().data(),
                                                        this_atom->resName.toLatin1().data(),
                                                        this_atom->coordinate.axis[0], this_atom->coordinate.axis[1], this_atom->coordinate.axis[2]));
            } 
            else 
            {
               this_atom->active = true;
               this_atom->radius = residue_list[respos].r_atom[atompos].hybrid.radius;
               this_atom->mw = residue_list[respos].r_atom[atompos].hybrid.mw;
                  
               this_atom->placing_method =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method;
               this_atom->bead_hydration =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].hydration;
               this_atom->bead_color =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].color;
               this_atom->bead_ref_volume =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].volume;
               this_atom->bead_ref_mw =  this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].mw;
               this_atom->ref_asa =  this_atom->p_residue->asa;
               this_atom->bead_computed_radius =  pow(3 * this_atom->bead_ref_volume / (4.0*M_PI), 1.0/3);
               this_atom->bead_assignment = this_atom->p_atom->bead_assignment;
               this_atom->atom_assignment = atompos;
               this_atom->chain = (int) this_atom->p_residue->r_bead[this_atom->bead_assignment].chain;
               this_atom->atom_hydration =  residue_list[respos].r_atom[atompos].hydration;
               this_atom->type = this_atom->p_residue->type;

 
              // if ( saxs_options.compute_saxs_coeff_for_bead_models )
              //  {
              //     QString mapkey = QString("%1|%2").arg(this_atom->resName).arg(this_atom->name);
              //     if ( this_atom->name == "OXT" )
              //     {
              //        mapkey = "OXT|OXT";
              //     }
                  
              //     if ( !residue_atom_hybrid_map.count(mapkey) )
              //     {
	      // 	    //cout << "error: hybrid name missing for " << this_atom->resName << "|" << this_atom->name << endl; 
              //       // QColor save_color = editor->textColor();
              //       // editor->setTextColor("red");
              //        us_log->log(QString("%1Molecule %2 Residue %3|%4 %5 Hybrid name missing. Atom skipped for SAS.\n")
              //                       .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
              //                       .arg(j+1)
              //                       .arg(this_atom->resName)
              //                       .arg(this_atom->name)
              //                       .arg(this_atom->resSeq));
              //        //editor->setTextColor(save_color);
              //     } else {
              //        QString hybrid_name = residue_atom_hybrid_map[mapkey];

              //        if ( hybrid_name.isEmpty() || !hybrid_name.length() )
              //        {
	      // 	       //cout << "error: hybrid name empty for " << this_atom->resName << "|" << this_atom->name << endl; 
              //          // QColor save_color = editor->textColor();
              //          // editor->setTextColor("red");
              //           us_log->log(QString("%1Molecule %2 Residue %3|%4 %5 Hybrid name missing. Atom skipped for SAS.\n")
              //                          .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
              //                          .arg(j+1)
              //                          .arg(this_atom->resName)
              //                          .arg(this_atom->name)
              //                          .arg(this_atom->resSeq));
              //           //editor->setTextColor(save_color);
              //        } else {
              //           if ( !saxs_util->hybrid_map.count(hybrid_name) )
              //           {
	      // 		  //cout << "error: hybrid_map name missing for hybrid_name " << hybrid_name << endl;
	      // 		  //QColor save_color = editor->textColor();
	      // 		  //editor->setTextColor("red");
              //              us_log->log(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 name missing from Hybrid file. Atom skipped for SAS.\n")
              //                             .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
              //                             .arg(j+1)
              //                             .arg(this_atom->resName)
              //                             .arg(this_atom->resSeq)
              //                             .arg(hybrid_name)
              //                             );
              //              //editor->setTextColor(save_color);
              //           } else {
              //              if ( !saxs_util->atom_map.count(this_atom->name + "~" + hybrid_name) )
              //              {
              //                 // cout << "error: atom_map missing for hybrid_name "
              //                 //      << hybrid_name 
              //                 //      << " atom name "
              //                 //      << this_atom->name
              //                 //      << endl;
              //                 // QColor save_color = editor->textColor();
              //                 // editor->setTextColor("red");
              //                 us_log->log(QString("%1Molecule %2 Atom %3 Residue %4 %5 Hybrid %6 name missing from Atom file. Atom skipped for SAS.\n")
              //                                .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
              //                                .arg(j+1)
              //                                .arg(this_atom->name)
              //                                .arg(this_atom->resName)
              //                                .arg(this_atom->resSeq)
              //                                .arg(hybrid_name)
              //                                );
              //                 // editor->setTextColor(save_color);
              //              } else {
              //                 this_atom->saxs_name = saxs_util->hybrid_map[hybrid_name].saxs_name; 
              //                 this_atom->hybrid_name = hybrid_name;
              //                 this_atom->hydrogens = 0;
              //                 if ( count_hydrogens.indexIn(hybrid_name) != -1 )
              //                 {
              //                    this_atom->hydrogens = count_hydrogens.cap(1).toInt();
              //                 }
              //                 this_atom->saxs_excl_vol = saxs_util->atom_map[this_atom->name + "~" + hybrid_name].saxs_excl_vol;
              //                 if ( !saxs_util->saxs_map.count(saxs_util->hybrid_map[hybrid_name].saxs_name) )
              //                 {
              //                    // cout << "error: saxs_map missing for hybrid_name "
              //                    //      << hybrid_name 
              //                    //      << " saxs name "
              //                    //      << saxs_util->hybrid_map[hybrid_name].saxs_name
              //                    //      << endl;
              //                    // QColor save_color = editor->textColor();
              //                    // editor->setTextColor("red");
              //                    us_log->log(QString("%1Molecule %2 Residue %3 %4 Hybrid %5 Saxs name %6 name missing from SAXS atom file. Atom skipped.\n")
              //                                   .arg(this_atom->chainID == " " ? "" : ("Chain " + this_atom->chainID + " "))
              //                                   .arg(j+1)
              //                                   .arg(this_atom->resName)
              //                                   .arg(this_atom->resSeq)
              //                                   .arg(hybrid_name)
              //                                   .arg(saxs_util->hybrid_map[hybrid_name].saxs_name)
              //                                   );
              //                    // editor->setTextColor(save_color);
              //                 } else {
              //                    this_atom->saxs_data = saxs_util->saxs_map[saxs_util->hybrid_map[hybrid_name].saxs_name];
              //                 }
              //              }
              //           }
              //        }
              //     }
              //  } // saxs setup



               if ( misc.pb_rule_on &&
                    this_atom->resName == "PRO" &&
                    this_atom->name == "N" &&
                    !k )
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("handled +1 mw for first N on PRO");
                  // }
                  this_atom->mw += 1.0;
                  // bead ref mw handled in pass 2b
               }
               if ( !misc.pb_rule_on &&
                    this_atom->name == "N" &&
                    !k )
               {
                  // if ( advanced_config.debug_1 )
                  // {
                  //    puts("handled +1 mw for first N");
                  // }
                  this_atom->mw += 1.0;
                  this_atom->bead_ref_mw += 1.0;
               }

               // if ( advanced_config.debug_1 ||
               //      advanced_config.debug_2 )
               // {
               //    printf("found atom %s %s in residue %d pos %d bead asgn %d %d placing info %d mw %f bead_ref_mw %f hybrid name %s %s ba %d chain %d\n",
               //           this_atom->name.toLatin1().data(), this_atom->resName.toLatin1().data(), respos, atompos,
               //           this_atom->p_atom->bead_assignment,
               //           this_atom->bead_assignment,
               //           this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].placing_method,
               //           this_atom->mw,
               //           this_atom->bead_ref_mw,
               //           residue_list[respos].r_atom[atompos].hybrid.name.toLatin1().data(),
               //           this_atom->p_atom->hybrid.name.toLatin1().data(),
               //           this_atom->p_atom->bead_assignment,
               //           this_atom->chain
               //           );
               // }
            }
         }

         if (this_atom->active)
         {
            // if ( advanced_config.debug_1 ||
            //      advanced_config.debug_2 )
            // {
            //    puts("active atom"); fflush(stdout);
            // }
            this_atom->active = false;
            if (this_atom->name != "H" &&
                this_atom->name != "D" &&
                this_atom->p_residue->name != "DOD" &&
                this_atom->p_residue->name != "HOH" && (this_atom->altLoc == "A" || this_atom->altLoc == " "))
            {
               this_atom->active = true;
               active_atoms.push_back(this_atom);
            }
            else 
            {
#if defined(DEBUG)
               printf
                  ("skipped bound waters & H %s %s rad %f resseq %s\n",
                   this_atom->name.toLatin1().data(), this_atom->resName.toLatin1().data(), this_atom->radius, this_atom->resSeq.toLatin1().data());
               fflush(stdout);
#endif
            }
         }
      }
   }
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
            
            // if ( advanced_config.debug_1 ||
            //      advanced_config.debug_2 )
            // {
            //    printf("p1 %s j k %d %d %lx %s %d chain %d\n", 
            //           this_atom->name.toLatin1().data(),
            //           j, k,
            //           (long unsigned int)this_atom->p_atom,
            //           this_atom->active ? "active" : "not active",
            //           this_atom->bead_assignment,
            //           this_atom->chain
            //           ); fflush(stdout);
            // }
            
            // // this_atom->bead_assignment =
            // // (this_atom->p_atom ? (int) this_atom->p_atom->bead_assignment : -1);
            // // this_atom->chain =
            // // ((this_atom->p_residue && this_atom->p_atom) ?
            // //  (int) this_atom->p_residue->r_bead[this_atom->p_atom->bead_assignment].chain : -1);
            this_atom->org_chain = this_atom->chain;
            this_atom->bead_positioner = this_atom->p_atom ? this_atom->p_atom->positioner : false;
         }
      }
   }
#define DO_SORT
#if defined(DO_SORT)
   // reorder residue
   {
      for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
         QString last_resSeq = "";
         list <sortable_PDB_atom> last_residue_atoms;
         unsigned int k;
         for (k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
            PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];

            if (this_atom->resSeq != last_resSeq) {
               if (last_resSeq != "") {
                  // reorder previous residue
                  last_residue_atoms.sort();
                  int base_ofs = (int) last_residue_atoms.size();
                  // printf("resort last residue... size/base %d k %u\n", base_ofs, k);
                  for (unsigned int m = k - base_ofs; m < k; m++) {
                     // printf("resort m = %u size lra %u\n", m, last_residue_atoms.size());
                     model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
                     last_residue_atoms.pop_front();
                  }
               }
               last_resSeq = this_atom->resSeq;
            }
            sortable_PDB_atom tmp_sortable_pdb_atom;
            tmp_sortable_pdb_atom.pdb_atom = *this_atom;
            tmp_sortable_pdb_atom.PRO_N_override = false;
            tmp_sortable_pdb_atom.pb_rule_on = misc.pb_rule_on;
            if(misc.pb_rule_on &&
               this_atom->resName == "PRO" &&
               this_atom->name == "N") {
               tmp_sortable_pdb_atom.PRO_N_override = true;
            }
            last_residue_atoms.push_back(tmp_sortable_pdb_atom);
         }
         if (last_resSeq != "") {
            // reorder 'last' residue
            last_residue_atoms.sort();
            int base_ofs = (int) last_residue_atoms.size();
            // printf("final resort last residue... size/base %d k %u\n", base_ofs, k);
            for (unsigned int m = k - base_ofs; m < k; m++) {
               //       printf("finalresort m = %u size lra %u\n", m, last_residue_atoms.size());
               model_vector[current_model].molecule[j].atom[m] = (last_residue_atoms.front()).pdb_atom;
               last_residue_atoms.pop_front();
            }
         }
      }
   }
#endif
#if !defined(DO_SORT)
   puts("sorting disabled");
#endif

   // if ( advanced_config.debug_1 )
   // {
   //    puts("after sort::");
   //    // list them again...
   //    {
   //       for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
   //          for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
   //             PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
               
   //             printf("p1 %s j k %d %d %lx %s %d %d\n", 
   //                    this_atom->name.toLatin1().data(),
   //                    j, k,
   //                    (long unsigned int)this_atom->p_atom,
   //                    this_atom->active ? "active" : "not active",
   //                    this_atom->bead_assignment,
   //                    this_atom->atom_assignment
   //                 ); fflush(stdout);
   //          }
   //       }
   //    }
   // }

   // if ( advanced_config.debug_1 )
   // {
   //    puts("mw totals:::");
   //    double tot_atom_mw = 0e0;
   //    double tot_bead_mw = 0e0;
   //    double prev_bead_mw = 0e0;
   //    double prev_atom_mw = 0e0;
   //    int bead_count = 0;
   //    int atom_count = 0;
   //    int last_asgn = -1;
   //    QString last_res = "";
   //    QString last_resName = "";
   //    {
   //       for (unsigned int j = 0; j < model_vector[current_model].molecule.size (); j++) {
   //          for (unsigned int k = 0; k < model_vector[current_model].molecule[j].atom.size (); k++) {
   //             PDB_atom *this_atom = &model_vector[current_model].molecule[j].atom[k];
   //             if(this_atom->active) {
   //                if(last_asgn != (int)this_atom->bead_assignment ||
   //                   last_res != this_atom->resSeq) {
   //                   if(bead_count) {
   //                      printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
   //                             last_res.toLatin1().data(),
   //                             last_resName.toLatin1().data(),
   //                             last_asgn,
   //                             prev_bead_mw,
   //                             prev_atom_mw,
   //                             prev_bead_mw -
   //                             prev_atom_mw
   //                             );
   //                   }
   //                   prev_bead_mw = 0;
   //                   prev_atom_mw = 0;
                     
   //                   bead_count++;
   //                   tot_bead_mw += this_atom->bead_ref_mw;
   //                   prev_bead_mw += this_atom->bead_ref_mw;
   //                   last_asgn = (int)this_atom->bead_assignment;
   //                   last_res = this_atom->resSeq;
   //                   last_resName = this_atom->resName;
   //                }
   //                atom_count++;
   //                tot_atom_mw += this_atom->mw;
   //                prev_atom_mw += this_atom->mw;
   //             }
   //          }
   //       }
   //       printf("res %s %s bead %d bead_mw %f sum atom mw %f diff %f\n",
   //              last_res.toLatin1().data(),
   //              last_resName.toLatin1().data(),
   //              last_asgn,
   //              prev_bead_mw,
   //              prev_atom_mw,
   //              prev_bead_mw -
   //              prev_atom_mw
   //              );
   //    }
   //    printf("~~tot atom %d mw %f tot bead %d mw %f\n",
   //           atom_count,
   //           tot_atom_mw,
   //           bead_count,
   //           tot_bead_mw);
   // }

   return 0;
}


double US_Saxs_Util::total_volume_of_bead_model( vector < PDB_atom > &bead_model )
{
   double tot_vol = 0e0;

   double pi43 = M_PI * 4e0 / 3e0;

   for ( unsigned int i = 0; i < bead_model.size(); i++ )
   {
      if ( bead_model[ i ].active ) 
      {
         tot_vol += (double) bead_model[ i ].bead_computed_radius * bead_model[ i ].bead_computed_radius * bead_model[ i ].bead_computed_radius;
      }
   }
   return tot_vol * pi43;
}


/* COMMENTED 
int US_Saxs_Util::issue_non_coded_hydro( bool quiet ) {
   us_qdebug( "issue_non_coded()" );
   // if ( quiet || advanced_config.expert_mode ) {
   //    us_qdebug( "issue_non_coded() returning quiet or expert" );
   //    switch ( pdb_parse.missing_residues ) {
   //    case 0 : // list & stop op
   //       return ISSUE_RESPONSE_STOP;
   //       break;
   //    case 1 : // list and skip
   //       return ISSUE_RESPONSE_NC_SKIP;
   //       break;
   //    case 2 : // abb
   //    default :
   //       return ISSUE_RESPONSE_NC_REPLACE;
   //       break;
   //    }
   // }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_non_coded() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "nc_skip" ) ) {
      us_qdebug( "issue_non_coded() auto return skip" );
      return ISSUE_RESPONSE_NC_SKIP;
   }
   if ( issue_info.count( "nc_replace" ) ) {
      us_qdebug( "issue_non_coded() auto return replace" );
      return ISSUE_RESPONSE_NC_REPLACE;
   }

   switch ( pdb_parse.missing_residues ) {
   case 0 :
      us_qdebug( "issue_non_coded() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_non_coded() setting skip q" );
         // switch( QMessageBox::question(
         //                               this
         //                               ,us_tr( "US-SOMO: Non coded residue" )
         //                               ,us_tr("A non-coded residue was encountered\n"
         //                                   "It can be skipped or you can stop processing.\n"
         //                                   "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
         //                                   "For specific residue details, please review the main panel text area after making your selection.\n"
         //                                   "\n"
         //                                   "If your model contains non-coded residues, the calculated molecular weight\n"
         //                                   "and vbar may be incorrect. Therefore, you could manually enter a global\n"
         //                                   "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
         //                                   "global value for the vbar in the SOMO Miscellaneous options.\n"
         //                                   "\n"
         //                                   "Please select your option below"
         //                                   )
         //                               ,us_tr( "Skip non-coded residues" )
         //                               ,us_tr( "Stop processing" )
         //                               ,QString()
         //                               ,1
         //                               ,1
         //                               ) ) {
         // case 0 : 
         //    issue_info.insert( "nc_skip" );
         //    return ISSUE_RESPONSE_NC_SKIP;
         //    break;

         // case 1 : 
         // default :
         issue_info.insert( "stop" );
         return ISSUE_RESPONSE_STOP;
         // break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_non_coded() setting abb q" );
         // switch( QMessageBox::question(
         //                               this
         //                               ,us_tr( "US-SOMO: Non coded residue" )
         //                               ,us_tr("A non-coded residue was encountered\n"
         //                                   "It can be replaced with an average residue, skipped or you can stop processing.\n"
         //                                   "This choice will be maintained for all remaining non coded residues encountered in this PDB file.\n"
         //                                   "For specific residue details, please review the main panel text area after making your selection.\n"
         //                                   "\n"
         //                                   "If your model contains non-coded residues, the calculated molecular weight\n"
         //                                   "and vbar may be incorrect. Therefore, you could manually enter a global\n"
         //                                   "value for the molecular weight in the SOMO hydrodynamic options, and a\n"
         //                                   "global value for the vbar in the SOMO Miscellaneous options. You can also\n"
         //                                   "review the average residue settings in the SOMO Miscellaneous options.\n"
         //                                   "\n"
         //                                   "Please select your option below"
         //                                   )
         //                               ,us_tr( "Replace non-coded with average residues" )
         //                               ,us_tr( "Skip non-coded residues" )
         //                               ,us_tr( "Stop processing" )
         //                               ,2
         //                               ,2
         //                               ) ) {
         // case 0 : 
         //    issue_info.insert( "nc_replace" );
         //    return ISSUE_RESPONSE_NC_REPLACE;
         //    break;

         // case 1 : 
         //    issue_info.insert( "nc_skip" );
         //    return ISSUE_RESPONSE_NC_SKIP;
         //    break;

         // case 2 : 
         // default :
         issue_info.insert( "stop" );
         return ISSUE_RESPONSE_STOP;
         // break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_non_coded() fall thru" );

   return ISSUE_RESPONSE_STOP;
}


int US_Saxs_Util::issue_missing_atom_hydro( bool quiet ) {

   us_qdebug( "issue_missing_atom()" );
   // if ( quiet || advanced_config.expert_mode ) {
   //    us_qdebug( "issue_missing_atom() returning quiet or expert" );
   //    switch ( pdb_parse.missing_atoms ) {
   //    case 0 : // list & stop op
   //       return ISSUE_RESPONSE_STOP;
   //       break;
   //    case 1 : // list and skip
   //       return ISSUE_RESPONSE_MA_SKIP;
   //       break;
   //    case 2 : // approx
   //    default :
   //       return ISSUE_RESPONSE_MA_MODEL;
   //       break;
   //    }
   // }

   if ( issue_info.count( "stop" ) ) {
      us_qdebug( "issue_missing_atom() auto return stop" );
      return ISSUE_RESPONSE_STOP;
   }
   if ( issue_info.count( "ma_skip" ) ) {
      us_qdebug( "issue_missing_atom() auto return skip" );
      return ISSUE_RESPONSE_MA_SKIP;
   }
   if ( issue_info.count( "ma_model" ) ) {
      us_qdebug( "issue_missing_atom() auto return model" );
      return ISSUE_RESPONSE_MA_MODEL;
   }

   switch ( pdb_parse.missing_atoms ) {
   case 0 :
      us_qdebug( "issue_missing_atom() return stop" );
      return ISSUE_RESPONSE_STOP;
      break;
   case 1 :
      {
         us_qdebug( "issue_missing_atom() setting skip q" );
         // switch( QMessageBox::question(
         //                               this
         //                               ,us_tr( "US-SOMO: Missing or extra atoms" )
         //                               ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
         //                                   "It can be skipped or you can stop processing.\n"
         //                                   "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
         //                                   "For specific residue details, please review the main panel text area after making your selection.\n"
         //                                   "\n"
         //                                   "If your model contains missing atoms, the calculated molecular\n"
         //                                   "weight and vbar may be incorrect, and you should manually enter\n"
         //                                   "a global value for the molecular weight in the SOMO hydrodynamic\n"
         //                                   "options, and a global value for the vbar in the SOMO Miscellaneous\n"
         //                                   "options.\n"
         //                                   "\n"
         //                                   "Please select your option below"
         //                                   )
         //                               ,us_tr( "Skip residues with missing atoms" )
         //                               ,us_tr( "Stop processing" )
         //                               ,QString()
         //                               ,1
         //                               ,1
         //                               ) ) {
         // case 0 : 
         //    issue_info.insert( "ma_skip" );
         //    return ISSUE_RESPONSE_MA_SKIP;
         //    break;

         // case 1 : 
         // default :
         issue_info.insert( "stop" );
         return ISSUE_RESPONSE_STOP;
         // break;
         }
      }
      break;
   case 2 :
      {
         us_qdebug( "issue_missing_atom() setting approx q" );
         // switch( QMessageBox::question(
         //                               this
         //                               ,us_tr( "US-SOMO: Missing or extra atoms" )
         //                               ,us_tr("A missing or extra atom was encountered when comparing with the residue table\n"
         //                                   "It can be modeled with an approximate method, skipped or you can stop processing.\n"
         //                                   "This choice will be maintained for all remaining missing atoms encountered in this PDB file.\n"
         //                                   "For specific residue details, please review the main panel text area after making your selection.\n"
         //                                   "\n"
         //                                   "If you model with an approximate method, the shape of this residue will\n"
         //                                   "be different from an exact residue which can effect the hydrodynamic computations\n"
         //                                   "\n"
         //                                   "If you skip missing atoms, the calculated molecular\n"
         //                                   "weight and vbar may be incorrect, and you should manually enter\n"
         //                                   "a global value for the molecular weight in the SOMO hydrodynamic\n"
         //                                   "options, and a global value for the vbar in the SOMO Miscellaneous\n"
         //                                   "options.\n"
         //                                   "\n"
         //                                   "Please select your option below"
         //                                   )
         //                               ,us_tr( "Model with the approximate method" )
         //                               ,us_tr( "Skip residues with missing atoms" )
         //                               ,us_tr( "Stop processing" )
         //                               ,2
         //                               ,2
         //                               ) ) {
         // case 0 : 
         //    issue_info.insert( "ma_model" );
         //    return ISSUE_RESPONSE_MA_MODEL;
         //    break;

         // case 1 : 
         //    issue_info.insert( "ma_skip" );
         //    return ISSUE_RESPONSE_MA_SKIP;
         //    break;

         // case 2 : 
         // default :
         issue_info.insert( "stop" );
         return ISSUE_RESPONSE_STOP;
         // break;
         }
      }
      break;
   default :
      break;

   }

   us_qdebug( "issue_missing_atom() fall thru" );

   return ISSUE_RESPONSE_STOP;
}
COMMENTED */

 void reset_hydro_res(hydro_results results_hydro)
  {
    results_hydro.total_beads = 0;
    results_hydro.used_beads = 0;
    results_hydro.mass = 0.0;
    results_hydro.s20w = 0.0;
    results_hydro.D20w = 0.0;
    results_hydro.viscosity = 0.0;
    results_hydro.ff0 = 0.0;
    results_hydro.rs = 0.0;
    results_hydro.rg = 0.0;
    results_hydro.vbar = 0.72;
    results_hydro.tau = 0.0;
    results_hydro.s20w_sd = 0.0;
    results_hydro.D20w_sd = 0.0;
    results_hydro.viscosity_sd = 0.0;
    results_hydro.ff0_sd = 0.0;
    results_hydro.rs_sd = 0.0;
    results_hydro.rg_sd = 0.0;
    results_hydro.tau_sd = 0.0;
    results_hydro.asa_rg_pos = 0.0;
    results_hydro.asa_rg_neg = 0.0; 
    
  }

