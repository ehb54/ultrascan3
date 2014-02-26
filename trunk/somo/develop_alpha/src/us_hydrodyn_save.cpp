#include "../include/us3_defines.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include <qtabwidget.h>
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QLabel>
#include <Q3Frame>
#include <Q3VBoxLayout>
#include <QCloseEvent>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_Hydrodyn_Save::US_Hydrodyn_Save(
                                   save_info *save, 
                                   void *us_hydrodyn, 
                                   bool *save_widget, 
                                   QWidget *p, 
                                   const char *name
                                   ) : Q3Frame(p, name)
{
   this->save = save;
   this->us_hydrodyn = us_hydrodyn;
   // build vectors, maps
   {
      QString data[] =
         {
            "__SECTION__",
            "Main hydro results:",

            "results.name", 
            "Model name", 
            "Model name", 

            // "results.num_models", 
            // "Number of models", 
            // "Number of models", 

            "__BREAK__",

            "results.total_beads", 
            "Total beads in model",
            "Total beads in model",

            // "results.total_beads_sd", 
            // "Total beads in model s.d.", 
            // "Total beads in model s.d.", 

            "__BREAK__",

            "results.used_beads", 
            "Used beads in model", 
            "Used beads in model", 

            // "results.used_beads_sd", 
            // "Used beads in model s.d.", 
            // "Used beads in model s.d.", 

            "__BREAK__",

            "results.mass", 
            "Molecular mass [Da]", 
            "Molecular mass [Da]", 

            "__BREAK__",

            "results.vbar", 
            "Partial specific volume [cm^3/g]", 
            "Partial specific volume [cm^3/g]", 

            "__BREAK__",

            "results.s20w", 
            "Sedimentation coefficient s [S]",
            "Sedimentation coefficient s [S]",

            // "results.s20w_sd", 
            // "s(20,w) standard deviation", 
            // "s(20,w) standard deviation", 

            "__BREAK__",
            "results.D20w", 
            "Translational diffusion coefficient D [cm/sec^2]",
            "Translational diffusion coefficient D [cm/sec^2]",

            // "results.D20w_sd", 
            // "D(20,w) transl. s.d.", 
            // "D(20,w) transl. s.d.", 

            "__BREAK__",

            "results.rs", 
            "Stokes radius [nm]", 
            "Stokes radius [nm]", 

            // "results.rs_sd", 
            // "Stokes radius s.d.", 
            // "Stokes radius s.d.", 

            "__BREAK__",

            "results.ff0", 
            "Frictional ratio", 
            "Frictional ratio", 

            // "results.ff0_sd", 
            // "Fricitional ratio s.d.", 
            // "Fricitional ratio s.d.", 

            "__BREAK__",

            "results.rg", 
            "Radius of gyration [nm] (from bead model)", 
            "Radius of gyration [nm] (from bead model)", 

            // "results.rg_sd", 
            // "Radius of gyration s.d.", 
            // "Radius of gyration s.d.", 

            "__BREAK__",

            "results.tau", 
            "Relaxation Time, tau(h) [ns]", 
            "Relaxation Time, tau(h) [ns]", 

            // "results.tau_sd", 
            // "Relaxation Time, tau(h) s.d.", 
            // "Relaxation Time, tau(h) s.d.", 

            "__BREAK__",

            "results.viscosity", 
            "Intrinsic viscosity [cm^3/g]", 
            "Intrinsic viscosity [cm^3/g]", 

            // "results.viscosity_sd", 
            // "Intrisic viscosity s.d.", 
            // "Intrisic viscosity s.d.", 

            "__SECTION__",
            "Additional hydro results:",

            "tot_surf_area",
            "Total surface area of beads in the model [nm^2]",
            "Total surface area of beads in the model [nm^2]",

            "tot_volume_of",
            "Total volume of beads in the model [nm^3]",
            "Total volume of beads in the model [nm^3]",

            "num_of_unused",
            "Number of unused beads",
            "Number of unused beads",

            "use_beads_vol",
            "Used beads volume [nm^3]",
            "Used beads volume [nm^3]",

            "use_beads_surf",
            "Used beads surface area [nm^2]",
            "Used beads surface area [nm^2]",

            "use_bead_mass",
            "Used bead mass [Da]",
            "Used bead mass [Da]",

            "con_factor",
            "Conversion Factor",
            "Conversion Factor",

            "tra_fric_coef", 
            "Translational frictional coefficient [g/s]",
            "Translational frictional coefficient",

            "rot_fric_coef",
            "Rotational frictional coefficient [g*cm^2/s]",
            "Rotational frictional coefficient [g*cm^2/s]",

            "rot_diff_coef",
            "Rotational diffusion coefficient [1/s]",
            "Rotational diffusion coefficient [1/s]",

            "rot_fric_coef_x",
            "Rotational frictional coefficient [ X, Y, Z ] [g*cm^2/s]",
            "Rotational frictional coefficient [ X, Y, Z ] [g*cm^2/s]",

            "rot_diff_coef_x",
            "Rotational diffusion coefficient [ X, Y, Z ] [1/s]",
            "Rotational diffusion coefficient [ X, Y, Z ] [1/s]",

            "rot_stokes_rad_x",
            "Rotational Stokes' radius [ X, Y, Z ] [nm]",
            "Rotational Stokes' radius [ X, Y, Z ] [nm]",

            "cen_of_res_x",
            "Centre of resistance [ X, Y, Z ] [nm]",
            "Centre of resistance [ X, Y, Z ] [nm]",

            "cen_of_mass_x",
            "Centre of mass [ X, Y, Z ] [nm]",
            "Centre of mass [ X, Y, Z ] [nm]",

            "cen_of_diff_x",
            "Centre of diffusion [ X, Y, Z ] [nm]",
            "Centre of diffusion [ X, Y, Z ] [nm]",

            "cen_of_visc_x",
            "Centre of viscosity [ X, Y, Z ] [nm]",
            "Centre of viscosity [ X, Y, Z ] [nm]",

            "unc_int_visc",
            "Uncorrected intrinsic viscosity [cm^3/g]",
            "Uncorrected intrinsic viscosity [cm^3/g]",

            "unc_einst_rad",
            "Uncorrected Einstein's radius [nm]",
            "Uncorrected Einstein's radius [nm]",

            "cor_int_visc",
            "Corrected intrinsic viscosity [cm^3/g]",
            "Corrected intrinsic viscosity [cm^3/g]",

            "cor_einst_rad",
            "Corrected Einstein's radius [nm]",
            "Corrected Einstein's radius [nm]",

            "rel_times_tau_1",
            "Relaxation times, tau(1) [ns]",
            "Relaxation times, tau(1) [ns]",

            "rel_times_tau_2",
            "Relaxation times, tau(2) [ns]",
            "Relaxation times, tau(2) [ns]",

            "rel_times_tau_3",
            "Relaxation times, tau(3) [ns]",
            "Relaxation times, tau(3) [ns]",

            "rel_times_tau_4",
            "Relaxation times, tau(4) [ns]",
            "Relaxation times, tau(4) [ns]",

            "rel_times_tau_5",
            "Relaxation times, tau(5) [ns]",
            "Relaxation times, tau(5) [ns]",

            "rel_times_tau_m",
            "Relaxation times, tau(m) [ns]",
            "Relaxation times, tau(m) [ns]",

            "rel_times_tau_h",
            "Relaxation times, tau(h) [ns]",
            "Relaxation times, tau(h) [ns]",

            "max_ext_x",
            "Maximum extensions [ X, Y, Z ] [nm]",
            "Maximum extensions [ X, Y, Z ] [nm]",

            "axi_ratios_xz",
            "Axial ratios [ X:Z, X:Y, Y:Z ] ",
            "Axial ratios [ X:Z, X:Y, Y:Z ] ",

            "__SECTION__",
            "Solvent conditions:",

            "hydro.solvent_name", 
            "Solvent name", 
            "Solvent name",

            "hydro.solvent_acronym", 
            "Solvent acronym", 
            "Solvent acronym", 

            "__BREAK__",

            "hydro.temperature", 
            "Solvent Temperature [C]", 
            "Solvent Temperature [C]", 

            "hydro.solvent_viscosity", 
            "Solvent viscosity [cP]", 
            "Solvent viscosity [cP]", 

            "hydro.solvent_density", 
            "Solvent density [g/ml]", 
            "solvent_density [g/ml]",

            "__SECTION__",
            "ASA results:",

            "__BREAK__",

            "results.asa_rg_pos", 
            "Radius of gyration (+r) [A] (from PDB atomic structure)",
            "Radius of gyration (+r) [A] (from PDB atomic structure)",

            "results.asa_rg_neg", 
            "Radius of gyration (-r) [A] (from PDB atomic structure)",
            "Radius of gyration (-r) [A] (from PDB atomic structure)",

            "__END__"
         };

      
      field.clear();
      descriptive_name.clear();
      short_name.clear();
      field_to_pos.clear();
      descriptive_name_to_pos.clear();
      section_name.clear();
      row_break.clear();
      descriptive_name_to_section.clear();

      QString this_section;
      unsigned int pos;
      bool is_section;
      bool is_row_break;

      unsigned int i = 0;
      int s = -1;
      while ( data[i] != "__END__" )
      {
         if ( ((US_Hydrodyn *)us_hydrodyn)->advanced_config.debug_4 )
         {
            printf("in while, data[%u] is <%s>\n", i, data[i].ascii());
         }
         if ( data[i].length() == 0 ) 
         {
            printf("us_hydrodyn_save DATA ERROR!\n");
            exit(-1);
         }
         is_section = false;
         is_row_break = false;
         if ( data[i] == "__SECTION__" ) 
         {
            s++;
            this_section = data[++i];
            if ( ((US_Hydrodyn *)us_hydrodyn)->advanced_config.debug_4 )
            {
               printf("in while, section is <%s>\n", this_section.ascii());
            }
            i++;
            is_section = true;
         }
         if ( data[i] == "__BREAK__" ) 
         {
            i++;
            is_row_break = true;
         }
         pos = field.size();
         if ( ((US_Hydrodyn *)us_hydrodyn)->advanced_config.debug_4 )
         {
            printf("in while, field[%u] is <%s>\n", i, data[i].ascii());
         }
         field.push_back(data[i++]);
         descriptive_name.push_back(data[i++]);
         short_name.push_back(data[i++]);
         field_to_pos[field[pos]] = pos;
         descriptive_name_to_pos[descriptive_name[pos]] = pos;
         descriptive_name_to_section[descriptive_name[pos]] = s;
         if ( is_section )
         {
            section_name[field[pos]] = this_section;
         }
         if ( is_row_break )
         {
            row_break[field[pos]] = true;
         }
      }
   }
   
   // build field_to_save_data, ..._type
   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      if ( field[i] == "results.name" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.name);
         field_to_save_data_type[field[i]] = DT_QSTRING;
         continue;
      }

      if ( field[i] == "results.num_models" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.num_models);
         field_to_save_data_type[field[i]] = DT_UNSIGNED_INT;
         continue;
      }

      if ( field[i] == "results.total_beads" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.total_beads);
         field_to_save_data_type[field[i]] = DT_FLOAT;
         field_to_precision[field[i]] = 0;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.total_beads_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.total_beads_sd);
         field_to_save_data_type[field[i]] = DT_FLOAT;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.used_beads" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.used_beads);
         field_to_save_data_type[field[i]] = DT_FLOAT;
         field_to_precision[field[i]] = 0;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.used_beads_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.used_beads_sd);
         field_to_save_data_type[field[i]] = DT_FLOAT;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.mass" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.mass);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.vbar" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.vbar);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.s20w" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.s20w);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.s20w_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.s20w_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.D20w" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.D20w);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'e';
         continue;
      }

      if ( field[i] == "results.D20w_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.D20w_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'e';
         continue;
      }

      if ( field[i] == "results.rs" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.rs);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.rs_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.rs_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.ff0" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.ff0);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.ff0_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.ff0_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.rg" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.rg);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.rg_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.rg_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.tau" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.tau);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.tau_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.tau_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.viscosity" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.viscosity);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.viscosity_sd" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.viscosity_sd);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "tot_surf_area" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.tot_surf_area);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "tot_volume_of" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.tot_volume_of);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "num_of_unused" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.num_of_unused);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "use_beads_vol" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.use_beads_vol);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "use_beads_surf" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.use_beads_surf);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "use_bead_mass" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.use_bead_mass);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "con_factor" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.con_factor);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = -1;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "tra_fric_coef" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.tra_fric_coef);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'e';
         continue;
      }

      if ( field[i] == "rot_fric_coef" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rot_fric_coef);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'e';
         continue;
      }

      if ( field[i] == "rot_diff_coef" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rot_diff_coef);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 0;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rot_fric_coef_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rot_fric_coef_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 3;
         field_to_format[field[i]] = 'e';
         continue;
      }

      if ( field[i] == "rot_diff_coef_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rot_diff_coef_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 0;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rot_stokes_rad_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rot_stokes_rad_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cen_of_res_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cen_of_res_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cen_of_mass_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cen_of_mass_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cen_of_diff_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cen_of_diff_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cen_of_visc_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cen_of_visc_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "unc_int_visc" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.unc_int_visc);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "unc_einst_rad" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.unc_einst_rad);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cor_int_visc" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cor_int_visc);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "cor_einst_rad" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.cor_einst_rad);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_1" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_1);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_2" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_2);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_3" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_3);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_4" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_4);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_5" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_5);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_m" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_m);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "rel_times_tau_h" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.rel_times_tau_h);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "max_ext_x" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.max_ext_x);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "axi_ratios_xz" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.axi_ratios_xz);
         field_to_save_data_type[field[i]] = DT_TRIPLE_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "hydro.solvent_name" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.hydro.solvent_name);
         field_to_save_data_type[field[i]] = DT_QSTRING;
         continue;
      }

      if ( field[i] == "hydro.solvent_acronym" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.hydro.solvent_acronym);
         field_to_save_data_type[field[i]] = DT_QSTRING;
         continue;
      }

      if ( field[i] == "hydro.temperature" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.hydro.temperature);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "hydro.solvent_viscosity" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.hydro.solvent_viscosity);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 6;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "hydro.solvent_density" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.hydro.solvent_density);
         field_to_save_data_type[field[i]] = DT_DOUBLE;
         field_to_precision[field[i]] = 6;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.asa_rg_pos" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.asa_rg_pos);
         field_to_save_data_type[field[i]] = DT_DOUBLE_NA;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }

      if ( field[i] == "results.asa_rg_neg" )
      {
         field_to_save_data[field[i]] = (void *)&(save->data.results.asa_rg_neg);
         field_to_save_data_type[field[i]] = DT_DOUBLE_NA;
         field_to_precision[field[i]] = 2;
         field_to_format[field[i]] = 'f';
         continue;
      }
   }

   this->save_widget = save_widget;
   if ( !save_widget )
   {
      return;
   }
   *save_widget = true;
   USglobal = new US_Config();
   setPalette( PALET_FRAME );
   setCaption(tr("Hydrodynamic Parameters to be Saved"));
   // for now:  until we save/restore
   //   save->field.clear();
   save->field_flag.clear();
   for ( unsigned int i = 0; i < save->field.size(); i++ )
   {
      save->field_flag[save->field[i]] = true;
   }
   header();

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      printf("read field %u <%s>\n", i, field[i].ascii());
   }

   if ( !save->field_flag.count("Model name") )
   {
      save->field_flag["Model name"] = true;
   }

   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 640, 480);
}

US_Hydrodyn_Save::~US_Hydrodyn_Save()
{
   if ( save_widget )
   {
      *save_widget = false;
   }
}

void US_Hydrodyn_Save::setupGUI()
{

   int minHeight1 = 30;
   int minWidth1 = 150;

   lbl_possible = new QLabel(tr("Parameters available"), this);
   Q_CHECK_PTR(lbl_possible);
   lbl_possible->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_possible->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_possible->setMinimumHeight(minHeight1);
   lbl_possible->setPalette( PALET_FRAME );
   AUTFBACK( lbl_possible );
   lbl_possible->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   tw_possible = new QTabWidget(this);
   // tw_possible->setTabBar(tb_possible);
   // tw_possible->tabBar()
   Q3ListBox *this_lb;

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      if ( section_name.count(field[i]) )
      {
         this_lb = new Q3ListBox(this);
         Q_CHECK_PTR(this_lb);
         this_lb->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
         this_lb->setMinimumHeight(minHeight1 * 3);
         this_lb->setMinimumWidth(minWidth1);
         this_lb->setPalette( PALET_EDIT );
         this_lb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
         this_lb->setEnabled(true);
         lb_possible.push_back(this_lb);
         tw_possible->addTab(this_lb, section_name[field[i]]);
         connect(this_lb, SIGNAL(selectionChanged()), SLOT(update_enables_possible()));
         this_lb->setSelectionMode(Q3ListBox::Multi);
      }
      if ( !save->field_flag.count(field[i]) )
      {
         this_lb->insertItem(descriptive_name[i]);
      }
   }
   for ( unsigned int i = 0; i < lb_possible.size(); i++ )
   {
      lb_possible[i]->setCurrentItem(0);
      lb_possible[i]->setSelected(0, false);
   }
   tw_possible->setPalette( PALET_NORMAL );
   AUTFBACK( tw_possible );
   connect(tw_possible, SIGNAL(currentChanged(QWidget *)), SLOT(tab_changed(QWidget *)));

   lbl_selected = new QLabel(tr("Parameters selected"), this);
   Q_CHECK_PTR(lbl_selected);
   lbl_selected->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lbl_selected->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_selected->setMinimumHeight(minHeight1);
   lbl_selected->setPalette( PALET_FRAME );
   AUTFBACK( lbl_selected );
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_selected = new Q3ListBox(this);
   Q_CHECK_PTR(lb_selected);
   lb_selected->setFrameStyle(Q3Frame::WinPanel|Q3Frame::Raised);
   lb_selected->setMinimumHeight(minHeight1 * 3);
   lb_selected->setMinimumWidth(minWidth1);
   lb_selected->setPalette( PALET_EDIT );
   AUTFBACK( lb_selected );
   lb_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_selected->setEnabled(true);
   for ( unsigned int i = 0; i < field.size(); i++ ) 
   {
      if ( save->field_flag.count(field[i]) )
      {
         lb_selected->insertItem(descriptive_name[i]);
      }
   }
   lb_selected->setSelectionMode(Q3ListBox::Multi);
   connect(lb_selected, SIGNAL(selectionChanged()), SLOT(update_enables_selected()));

   pb_add = new QPushButton("-->", this);
   Q_CHECK_PTR(pb_add);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3));
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setPalette( PALET_PUSHB );
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_remove = new QPushButton("<--", this);
   Q_CHECK_PTR(pb_remove);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( PALET_PUSHB );
   connect(pb_remove, SIGNAL(clicked()), SLOT(remove()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( PALET_PUSHB );
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( PALET_PUSHB );
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   // left box / possible

   Q3VBoxLayout *vbl_possible = new Q3VBoxLayout;
   vbl_possible->addWidget(lbl_possible);
   vbl_possible->addWidget(tw_possible);

   Q3VBoxLayout *vbl_add_remove = new Q3VBoxLayout;
   vbl_add_remove->addSpacing(20);
   vbl_add_remove->addWidget(pb_add);
   vbl_add_remove->addWidget(pb_remove);
   vbl_add_remove->addSpacing(20);

   Q3VBoxLayout *vbl_selected = new Q3VBoxLayout;
   vbl_selected->addWidget(lbl_selected);
   vbl_selected->addWidget(lb_selected);

   Q3HBoxLayout *hbl_top = new Q3HBoxLayout;
   hbl_top->addSpacing(5);
   hbl_top->addLayout(vbl_possible);
   hbl_top->addSpacing(20);
   hbl_top->addLayout(vbl_add_remove);
   hbl_top->addSpacing(20);
   hbl_top->addLayout(vbl_selected);
   hbl_top->addSpacing(5);

   Q3HBoxLayout *hbl_bottom = new Q3HBoxLayout;
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(5);

   Q3VBoxLayout *background = new Q3VBoxLayout(this);
   background->addSpacing(5);
   background->addLayout(hbl_top);
   background->addSpacing(5);
   background->addLayout(hbl_bottom);
   background->addSpacing(5);

   update_enables_possible();
   update_enables_selected();
}

void US_Hydrodyn_Save::cancel()
{
   close();
}

void US_Hydrodyn_Save::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_save.html");
}

void US_Hydrodyn_Save::closeEvent(QCloseEvent *e)
{
   *save_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

void US_Hydrodyn_Save::update_enables_possible()
{
   bool any_selected = false;
   int t = tw_possible->currentPageIndex();

   for ( int i = 0; i < lb_possible[t]->numRows(); i++ )
   {
      if ( lb_possible[t]->isSelected(i) )
      {
         any_selected = true;
         break;
      }
   }
   pb_add->setEnabled(any_selected);
}

void US_Hydrodyn_Save::update_enables_selected()
{
   bool any_selected = false;
   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      if ( lb_selected->isSelected(i) )
      {
         any_selected = true;
         break;
      }
   }
   pb_remove->setEnabled(any_selected);
}

void US_Hydrodyn_Save::add()
{
   
   int t = tw_possible->currentPageIndex();
   for ( int i = 0; i < lb_possible[t]->numRows(); i++ )
   {
      if ( lb_possible[t]->isSelected(i) )
      {
         lb_selected->insertItem(lb_possible[t]->text(i));
         lb_possible[t]->removeItem(i);
         i--;
      }
   }
   rebuild();
}

void US_Hydrodyn_Save::remove()
{
   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      if ( lb_selected->isSelected(i) &&
           lb_selected->text(i) != "Model name" )
      {
         lb_possible[descriptive_name_to_section[lb_selected->text(i)]]->insertItem(lb_selected->text(i));
         lb_selected->removeItem(i);
         i--;
      }
   }
   rebuild();
}

void US_Hydrodyn_Save::rebuild()
{
   map < QString, bool > possible_selected;
   map < QString, bool > selected;

   possible_selected.clear();
   selected.clear();

   // save possible selection status
   for ( unsigned int t = 0; t < lb_possible.size(); t++ )
   {
      for ( int i = 0; i < lb_possible[t]->numRows(); i++ )
      {
         possible_selected[lb_possible[t]->text(i)] = lb_possible[t]->isSelected(i);
      }
   }

   // save current 'selected' list

   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      selected[lb_selected->text(i)] = lb_selected->isSelected(i);
   }

   // clear tab lists

   for ( unsigned int t = 0; t < lb_possible.size(); t++ )
   {
      lb_possible[t]->clear();
   }

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      if ( !selected.count(descriptive_name[i]) )
      {
         lb_possible[descriptive_name_to_section[descriptive_name[i]]]->insertItem(descriptive_name[i]);
      }
   }

   // restore selections
   for ( unsigned int t = 0; t < lb_possible.size(); t++ )
   {
      for ( int i = 0; i < lb_possible[t]->numRows(); i++ )
      {
         lb_possible[t]->setSelected(i, possible_selected[lb_possible[t]->text(i)]);
      }
   }

   // recreate the save info
   save->field.clear();
   save->field_flag.clear();
   printf("save field list:\n");
   for ( int i = 0; i < lb_selected->numRows(); i++ )
   {
      save->field_flag[field[descriptive_name_to_pos[lb_selected->text(i)]]] = true;
      save->field.push_back(field[descriptive_name_to_pos[lb_selected->text(i)]]);
      printf("%s\n", field[descriptive_name_to_pos[lb_selected->text(i)]].ascii());
   }
   header();
}

void US_Hydrodyn_Save::tab_changed(QWidget *)
{
   update_enables_possible();
}

QString US_Hydrodyn_Save::header()
{
   QString result;
   QString this_field;
   for ( unsigned int i = 0; i < save->field.size(); i++ )
   {
      this_field = descriptive_name[field_to_pos[save->field[i]]].replace("\"", "''");
      if ( this_field.contains("[ X, Y, Z ]") ||
           this_field.contains("[ X:Z, X:Y, Y:Z ]") )
      {
         QString field_x = this_field;
         QString field_y = this_field;
         QString field_z = this_field;

         result += QString("%1\"%2\",\"%3\",\"%4\"")
            .arg(i ? "," : "")
            .arg(field_x.replace("[ X, Y, Z ]","X").replace("[ X:Z, X:Y, Y:Z ]","X:Z"))
            .arg(field_y.replace("[ X, Y, Z ]","Y").replace("[ X:Z, X:Y, Y:Z ]","X:Y"))
            .arg(field_z.replace("[ X, Y, Z ]","Z").replace("[ X:Z, X:Y, Y:Z ]","Y:Z"));
      } else {
         result += QString("%1\"%2\"")
            .arg(i ? "," : "")
            .arg(this_field);
      }
   }
   printf("header() <%s>\n", result.ascii());
   return result + "\n";
}

// #define ZARG(s) arg(s,0,field_to_format[save->field[i]],field_to_precision[save->field[i]]).replace(QRegExp("\\.0+$"),"")
// #define DBFARG(s) printf("i = %u _field <%s> _format <%c> _precision <%d>\n",i,save->field[i].ascii(), field_to_format[save->field[i]],field_to_precision[save->field[i]]);fflush(stdout)
#define FARG(s) arg(s,0,field_to_format[save->field[i]],field_to_precision[save->field[i]])

QString US_Hydrodyn_Save::dataString(save_data *data)
{
   save->data = *data;
   QString result;
   for ( unsigned int i = 0; i < save->field.size(); i++ )
   {
      switch(field_to_save_data_type[save->field[i]]) 
      {
      case DT_QSTRING       :
         result += QString("%1\"%2\"")
            .arg(i ? "," : "")
            .arg(*((QString *)(field_to_save_data[save->field[i]])));
         break;
      case DT_BOOL          :
         result += QString("%1%2")
            .arg(i ? "," : "")
            .arg(*((bool *)(field_to_save_data[save->field[i]])) ? 1 : 0 );
         break;
      case DT_FLOAT         :
         result += QString("%1%2")
            .arg(i ? "," : "")
            .FARG((*(float *)(field_to_save_data[save->field[i]])));
         // DBFARG((*(float *)(field_to_save_data[save->field[i]])));
         break;
      case DT_DOUBLE        :
         result += QString("%1%2")
            .arg(i ? "," : "")
            .FARG(*((double *)(field_to_save_data[save->field[i]])));
         // DBFARG((*(double *)(field_to_save_data[save->field[i]])));
         break;
      case DT_INT           :
         result += QString("%1%2")
            .arg(i ? "," : "")
            .arg(*((int *)(field_to_save_data[save->field[i]])));
         break;
      case DT_UNSIGNED_INT  :
         result += QString("%1%2")
            .arg(i ? "," : "")
            .arg(*((unsigned int *)(field_to_save_data[save->field[i]])));
         break;
      case DT_TRIPLE_DOUBLE :
         result += QString("%1%2,%3,%4")
            .arg(i ? "," : "")
            .FARG(*((double *)(field_to_save_data[save->field[i]])))
            .FARG(*((double *)(field_to_save_data[save->field[i]]) + 1))
            .FARG(*((double *)(field_to_save_data[save->field[i]]) + 2));
         // DBFARG((*(double *)(field_to_save_data[save->field[i]])));
         break;
      case DT_DOUBLE_NA     :
         {
            double tmp = *((double *)(field_to_save_data[save->field[i]]));
            if ( tmp >= 0 )
            {
               result += QString("%1%2")
                  .arg(i ? "," : "")
                  .FARG(*((double *)(field_to_save_data[save->field[i]])));
               // DBFARG((*(double *)(field_to_save_data[save->field[i]])));
            } else {
               result += ",NA";
            }
         }
         break;
      default               :
         result += QString("%1\"???\"")
            .arg(i ? "," : "");
         break;
      }
   }
   printf("data() <%s>\n", result.ascii());
   return result + "\n";
}

vector < save_data > US_Hydrodyn_Save::stats(vector < save_data > *data)
{
   // create zero data record
   // switch to offsets 
   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      //      printf("stats field %u of %u\n", i, field.size()); fflush(stdout);
      //      printf("field[i] %s type %d\n",field[i].ascii(), field_to_save_data_type[field[i]]); fflush(stdout);
      //      printf("save->field[i] %s\n",save->field[i].ascii()); fflush(stdout);

      switch(field_to_save_data_type[field[i]]) 
      {
      case DT_QSTRING       :
         *((QString *)(field_to_save_data[field[i]])) = "";
         break;
      case DT_BOOL          :
         *((bool *)(field_to_save_data[field[i]])) = 0;
         break;
      case DT_FLOAT         :
         *((float *)(field_to_save_data[field[i]])) = 0;
         break;
      case DT_DOUBLE        :
         *((double *)(field_to_save_data[field[i]])) = 0;
         break;
      case DT_INT           :
         *((int *)(field_to_save_data[field[i]])) = 0;
         break;
      case DT_UNSIGNED_INT  :
         *((unsigned int *)(field_to_save_data[field[i]])) = 0;
         break;
      case DT_TRIPLE_DOUBLE :
         *((double *)(field_to_save_data[field[i]])) = 0;
         *((double *)(field_to_save_data[field[i]]) + 1) = 0;
         *((double *)(field_to_save_data[field[i]]) + 2) = 0;
         break;
      case DT_DOUBLE_NA     :
         *((double *)(field_to_save_data[field[i]])) = 0;
         break;
      }
   }
   save_data zero = save->data;
   save_data sum = zero;
   save_data sum2 = zero;
   save_data count = zero; // note counts may be different for n/a fields;

   QString tmp_qstring;
   bool tmp_bool;
   float tmp_float;
   double tmp_double;
   double tmp_double_2;
   double tmp_double_3;
   int tmp_int;
   unsigned int tmp_uint;
   float tmp_count_float;
   double tmp_count_double;
   int tmp_count_int;
   unsigned int tmp_count_uint;

   // we're going to have to be careful here, we only have one map

   for ( unsigned int j = 0; j < data->size(); j++ )
   {
      for ( unsigned int i = 0; i < field.size(); i++ )
      {
         switch(field_to_save_data_type[field[i]]) 
         {
         case DT_QSTRING       :
            save->data = (*data)[j];
            tmp_qstring = *((QString *)(field_to_save_data[field[i]]));

            save->data = sum;
            if ( j )
            {
               *((QString *)(field_to_save_data[field[i]])) += ", ";
            } else {
               *((QString *)(field_to_save_data[field[i]])) = tr("Average: ");
            }
            *((QString *)(field_to_save_data[field[i]])) += tmp_qstring;
            sum = save->data;

            save->data = sum2;
            if ( j )
            {
               *((QString *)(field_to_save_data[field[i]])) += ", ";
            } else {
               *((QString *)(field_to_save_data[field[i]])) = tr("Standard deviation: ");
            }
            *((QString *)(field_to_save_data[field[i]])) += tmp_qstring;
            sum2 = save->data;

            break;
         case DT_BOOL          :
            save->data = (*data)[j];
            tmp_bool = *((bool *)(field_to_save_data[field[i]]));

            save->data = sum;
            *((bool *)(field_to_save_data[field[i]])) &= tmp_bool;
            sum = save->data;

            save->data = sum2;
            *((bool *)(field_to_save_data[field[i]])) |= tmp_bool;
            sum2 = save->data;

            break;
         case DT_FLOAT         :
            save->data = (*data)[j];
            tmp_float = *((float *)(field_to_save_data[field[i]]));

            save->data = sum;
            *((float *)(field_to_save_data[field[i]])) += tmp_float;
            sum = save->data;

            save->data = sum2;
            *((float *)(field_to_save_data[field[i]])) += tmp_float * tmp_float;
            sum2 = save->data;

            save->data = count;
            (*((float *)(field_to_save_data[field[i]])))++;
            count = save->data;

            break;
         case DT_DOUBLE        :
            save->data = (*data)[j];
            tmp_double = *((double *)(field_to_save_data[field[i]]));

            save->data = sum;
            *((double *)(field_to_save_data[field[i]])) += tmp_double;
            sum = save->data;

            save->data = sum2;
            *((double *)(field_to_save_data[field[i]])) += tmp_double * tmp_double;
            sum2 = save->data;

            save->data = count;
            (*((double *)(field_to_save_data[field[i]])))++;
            count = save->data;

            break;
         case DT_INT           :
            save->data = (*data)[j];
            tmp_int = *((int *)(field_to_save_data[field[i]]));

            save->data = sum;
            *((int *)(field_to_save_data[field[i]])) += tmp_int;
            sum = save->data;

            save->data = sum2;
            *((int *)(field_to_save_data[field[i]])) += tmp_int * tmp_int;
            sum2 = save->data;

            save->data = count;
            (*((int *)(field_to_save_data[field[i]])))++;
            count = save->data;

            break;
         case DT_UNSIGNED_INT  :
            save->data = (*data)[j];
            tmp_uint = *((unsigned int *)(field_to_save_data[field[i]]));

            save->data = sum;
            *((unsigned int *)(field_to_save_data[field[i]])) += tmp_uint;
            sum = save->data;

            save->data = sum2;
            *((unsigned int *)(field_to_save_data[field[i]])) += tmp_uint * tmp_uint;
            sum2 = save->data;

            save->data = count;
            (*((unsigned int *)(field_to_save_data[field[i]])))++;
            count = save->data;

            break;

         case DT_TRIPLE_DOUBLE :
            save->data = (*data)[j];
            tmp_double = *((double *)(field_to_save_data[field[i]]));
            tmp_double_2 = *((double *)(field_to_save_data[field[i]]) + 1);
            tmp_double_3 = *((double *)(field_to_save_data[field[i]]) + 2);

            save->data = sum;
            *((double *)(field_to_save_data[field[i]])) += tmp_double;
            *((double *)(field_to_save_data[field[i]]) + 1) += tmp_double_2;
            *((double *)(field_to_save_data[field[i]]) + 2) += tmp_double_3;
            sum = save->data;

            save->data = sum2;
            *((double *)(field_to_save_data[field[i]])) += tmp_double * tmp_double;
            *((double *)(field_to_save_data[field[i]]) + 1) += tmp_double_2 * tmp_double_2;
            *((double *)(field_to_save_data[field[i]]) + 2) += tmp_double_2 * tmp_double_3;
            sum2 = save->data;

            save->data = count;
            (*((double *)(field_to_save_data[field[i]])))++;
            (*((double *)(field_to_save_data[field[i]]) + 1))++;
            (*((double *)(field_to_save_data[field[i]]) + 2))++;
            count = save->data;

            break;
         case DT_DOUBLE_NA     :
            tmp_double = *((double *)(field_to_save_data[field[i]]));

            if ( tmp_double >= 0 )
            {
               save->data = sum;
               *((double *)(field_to_save_data[field[i]])) += tmp_double;
               sum = save->data;

               save->data = sum2;
               *((double *)(field_to_save_data[field[i]])) += tmp_double * tmp_double;
               sum2 = save->data;

               save->data = count;
               (*((double *)(field_to_save_data[field[i]])))++;
               count = save->data;
            }
            break;
         }
      }
   }

   // now compute averages etc.

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      switch(field_to_save_data_type[field[i]]) 
      {
      case DT_QSTRING       :
         break;
      case DT_BOOL          :
         break;
      case DT_FLOAT         :
         save->data = count;
         tmp_count_float = (*((float *)(field_to_save_data[field[i]])));

         save->data = sum;
         *((float *)(field_to_save_data[field[i]])) /= tmp_count_float;
         tmp_float = *((float *)(field_to_save_data[field[i]]));
         sum = save->data;
         
         save->data = sum2;
         *((float *)(field_to_save_data[field[i]])) = 
            sqrt(fabs((*((float *)(field_to_save_data[field[i]])) - tmp_float * tmp_float * tmp_count_float) / 
            (tmp_count_float - 1)));
         sum2 = save->data;
                  
         break;
      case DT_DOUBLE        :
         save->data = count;
         tmp_count_double = (*((double *)(field_to_save_data[field[i]])));

         save->data = sum;
         *((double *)(field_to_save_data[field[i]])) /= tmp_count_double;
         tmp_double = *((double *)(field_to_save_data[field[i]]));
         sum = save->data;
         
         save->data = sum2;
         *((double *)(field_to_save_data[field[i]])) = 
            sqrt(fabs((*((double *)(field_to_save_data[field[i]])) - tmp_double * tmp_double * tmp_count_double  )
            / (tmp_count_double - 1)));
         sum2 = save->data;
         
         break;
      case DT_INT           :
         save->data = count;
         tmp_count_int = (*((int *)(field_to_save_data[field[i]])));

         save->data = sum;
         *((int *)(field_to_save_data[field[i]])) /= tmp_count_int;
         tmp_int = *((int *)(field_to_save_data[field[i]]));
         sum = save->data;
         
         save->data = sum2;
         *((int *)(field_to_save_data[field[i]])) = 
            (*((int *)(field_to_save_data[field[i]])) - tmp_int * tmp_int * tmp_count_int) 
            / (tmp_count_int - 1);
         sum2 = save->data;
         
         break;
      case DT_UNSIGNED_INT  :
         save->data = count;
         tmp_count_uint = (*((unsigned int *)(field_to_save_data[field[i]])));

         save->data = sum;
         *((unsigned int *)(field_to_save_data[field[i]])) /= tmp_count_uint;
         tmp_uint = *((unsigned int *)(field_to_save_data[field[i]]));
         sum = save->data;
         
         save->data = sum2;
         *((unsigned int *)(field_to_save_data[field[i]])) = 
            (*((unsigned int *)(field_to_save_data[field[i]])) - tmp_uint * tmp_uint * tmp_count_uint) 
            / (tmp_count_uint - 1);
         sum2 = save->data;
         
         break;
         
      case DT_TRIPLE_DOUBLE :
         save->data = count;
         tmp_count_double = (*((double *)(field_to_save_data[field[i]])));

         save->data = sum;
         *((double *)(field_to_save_data[field[i]])) /= tmp_count_double;
         *((double *)(field_to_save_data[field[i]]) + 1) /= tmp_count_double;
         *((double *)(field_to_save_data[field[i]]) + 2) /= tmp_count_double;
         tmp_double = *((double *)(field_to_save_data[field[i]]));
         tmp_double_2 = *((double *)(field_to_save_data[field[i]]) + 1);
         tmp_double_3 = *((double *)(field_to_save_data[field[i]]) + 2);
         sum = save->data;
         
         save->data = sum2;
         *((double *)(field_to_save_data[field[i]])) = 
            sqrt(fabs((*((double *)(field_to_save_data[field[i]])) - tmp_double * tmp_double * tmp_count_double)
            / (tmp_count_double - 1)));
         *((double *)(field_to_save_data[field[i]]) + 1) = 
            sqrt(fabs((*((double *)(field_to_save_data[field[i]]) + 1) - tmp_double_2 * tmp_double_2 * tmp_count_double)
            / (tmp_count_double - 1)));
         *((double *)(field_to_save_data[field[i]]) + 2) = 
            sqrt(fabs((*((double *)(field_to_save_data[field[i]]) + 2) - tmp_double_3 * tmp_double_3 * tmp_count_double)
            / (tmp_count_double - 1)));
         sum2 = save->data;
         
         break;
      case DT_DOUBLE_NA     :
         save->data = count;
         tmp_count_double = (*((double *)(field_to_save_data[field[i]])));

         if ( tmp_count_double >= 1 )
         {
            
            save->data = sum;
            *((double *)(field_to_save_data[field[i]])) /= tmp_count_double;
            tmp_double = *((double *)(field_to_save_data[field[i]]));
            sum = save->data;
         
            save->data = sum2;
            *((double *)(field_to_save_data[field[i]])) = 
               sqrt(fabs((*((double *)(field_to_save_data[field[i]])) - tmp_double * tmp_double * tmp_count_double)
               / (tmp_count_double - 1)));
            sum2 = save->data;
         }
         break;
      }
   }

   vector < save_data > result;
   result.push_back(sum);
   result.push_back(sum2);
   return result;
}

QString US_Hydrodyn_Save::hydroFormatStats(vector < save_data > stats)
{
   QString result;
   
   if ( stats.size() != 2 )
   {
      result.sprintf("INTERNAL ERROR: ******** US_Hydrodyn_Save::hydroFormatStats called with wrong size stats %u\n",
                     (unsigned int) stats.size());
      cout << result;
      return result;
   }
   
   //   float raflag = ((US_Hydrodyn *)us_hydrodyn)->save_params.raflag;
   
   QString tmpString;
   
   result = "\n\t AVERAGE PARAMETERS \n\n\t\t\t\t Mean value\tSt. Dev.\n\n";
   result += QString("").sprintf("- TRANS. FRICT. COEFF.        \t%.3e\t%.3e\t[g/s]\n", 
                                 stats[0].tra_fric_coef,
                                 stats[1].tra_fric_coef);
   
   
   result += QString("").sprintf("- TRANS. DIFF. COEFF.         \t%.2e\t%.3e\t[cm^2/s]\n", 
                                 stats[0].results.D20w,
                                 stats[1].results.D20w);
   
   //   if (raflag == -1.0)
   //   {
   //      result += QString("").sprintf("- SED. COEFF. (psv unhyd.rad.)\t%.2f\t\t%.2f\t\t[S]\n",
   //                                    stats[0].results.s20w,
   //                                    stats[1].results.s20w
   //                                    );
   //   }
   
   QString bead_model_source =
      ((US_Hydrodyn *)us_hydrodyn)->misc.compute_vbar ?
      ( ((US_Hydrodyn *)us_hydrodyn)->bead_model_from_file ?
        "from file" : "computed" ) : "user entered";
   
   
   //   if ((raflag == -2.0) || (raflag == -5.0) || (raflag == -3.0))
   //   {
   result += QString("").sprintf("- SED. COEFF. (psv %s) \t%.2f\t\t%.2f\t\t[S]\n", 
                                 bead_model_source.ascii(),
                                 stats[0].results.s20w,
                                 stats[1].results.s20w);
   //   }
   
   result += QString("").sprintf("- FRICTIONAL RATIO            \t%.2f\t%.3e\n\n",
                                 stats[0].results.ff0,
                                 stats[1].results.ff0);
   
   result += QString("").sprintf("- ROT. FRICT. COEFF.          \t%.3e\t%.3e\t[g*cm^2/s]\n",
                                 stats[0].rot_fric_coef,
                                 stats[1].rot_fric_coef);
   result += QString("").sprintf("- ROT. DIFF. COEFF.           \t%.0f\t\t%.0f\t[1/s]\n",
                                 stats[0].rot_diff_coef,
                                 stats[1].rot_diff_coef);
   result += QString("").sprintf("- ROT. FRICT. COEFF. [ X ]    \t%.3e\t%.3e\t[g*cm^2/s]\n",
                                 stats[0].rot_fric_coef_x,
                                 stats[1].rot_fric_coef_x);
   result += QString("").sprintf("- ROT. FRICT. COEFF. [ Y ]    \t%.3e\t%.3e\t[g*cm^2/s]\n",
                                 stats[0].rot_fric_coef_y,
                                 stats[1].rot_fric_coef_y);
   result += QString("").sprintf("- ROT. FRICT. COEFF. [ Z ]    \t%.3e\t%.3e\t[g*cm^2/s]\n",
                                 stats[0].rot_fric_coef_z,
                                 stats[1].rot_fric_coef_z);
   result += QString("").sprintf("- ROT. DIFF. COEFF. [ X ]     \t%.0f\t\t%.0f\t[1/s]\n",
                                 stats[0].rot_diff_coef_x,
                                 stats[1].rot_diff_coef_x);
   result += QString("").sprintf("- ROT. DIFF. COEFF. [ Y ]     \t%.0f\t\t%.0f\t[1/s]\n",
                                 stats[0].rot_diff_coef_y,
                                 stats[1].rot_diff_coef_y);
   result += QString("").sprintf("- ROT. DIFF. COEFF. [ Z ]     \t%.0f\t\t%.0f\t[1/s]\n",
                                 stats[0].rot_diff_coef_z,
                                 stats[1].rot_diff_coef_z);
   
   result += QString("").sprintf("\n- RADIUS OF GYRATION          \t%.2f\t\t%.2f\t\t[nm]\n", 
                                 stats[0].results.rg,
                                 stats[1].results.rg);
   
   result += QString("").sprintf("- TRANS. STOKES' RADIUS       \t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].results.rs,
                                 stats[1].results.rs);
   result += QString("").sprintf("- ROTAT. STOKES' RADIUS [ X ] \t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].rot_stokes_rad_x,
                                 stats[1].rot_stokes_rad_x);
   result += QString("").sprintf("- ROTAT. STOKES' RADIUS [ Y ] \t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].rot_stokes_rad_y,
                                 stats[1].rot_stokes_rad_y);
   result += QString("").sprintf("- ROTAT. STOKES' RADIUS [ Z ] \t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].rot_stokes_rad_z,
                                 stats[1].rot_stokes_rad_z);
   result += QString("").sprintf("\n- UNCORRECTED INTRINSIC VISC. \t%.2f\t\t%.2f\t\t[cm^3/g]\n",
                                 stats[0].unc_int_visc,
                                 stats[1].unc_int_visc);
   result += QString("").sprintf("- UNCORRECTED EINSTEIN'S RADIUS\t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].unc_einst_rad,
                                 stats[1].unc_einst_rad);
   result += QString("").sprintf("- CORRECTED INTRINSIC VISCOSITY\t%.2f\t\t%.2f\t\t[cm^3/g]\n",
                                 stats[0].cor_int_visc,
                                 stats[1].cor_int_visc);
   result += QString("").sprintf("- CORRECTED EINSTEIN'S RADIUS\t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].cor_einst_rad,
                                 stats[1].cor_einst_rad);
   
#if defined(TSUDA_DOUBLESUM)
   result += QString("").sprintf("- INTRINSIC VISC. (Double Sum)\t%.2f\t\t%.2f\t\t[cm^3/g]\n",
                                 stats[0].,
                                 stats[1].);
   result += QString("").sprintf("- EINSTEIN'S RADIUS (D. Sum)  \t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].,
                                 stats[1].);
   result += QString("").sprintf("- INTRINSIC VISC. (Tsuda CM)  \t%.2f\t\t%.2f\t\t[cm^3/g]\n",
                                 stats[0].,
                                 stats[1].);
   result += QString("").sprintf("- EINSTEIN'S RADIUS (Tsuda CM)\t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].,
                                 stats[1].);
   result += QString("").sprintf("- INTRINSIC VISC. (Tsuda CV)  \t%.2f\t\t%.2f\t\t[cm^3/g]\n",
                                 stats[0].,
                                 stats[1].);
   result += QString("").sprintf("- EINSTEIN'S RADIUS (Tsuda CV)\t%.2f\t\t%.2f\t\t[nm]\n",
                                 stats[0].,
                                 stats[1].);
#endif
   
   result += QString("").sprintf("\nRELAXATION TIMES\n\n");
   
   result += QString("").sprintf(" Tau(1)                       \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_1,
                                 stats[1].rel_times_tau_1);
   result += QString("").sprintf(" Tau(2)                       \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_2,
                                 stats[1].rel_times_tau_2);
   result += QString("").sprintf(" Tau(3)                       \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_3,
                                 stats[1].rel_times_tau_3);
   result += QString("").sprintf(" Tau(4)                       \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_4,
                                 stats[1].rel_times_tau_4);
   result += QString("").sprintf(" Tau(5)                       \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_5,
                                 stats[1].rel_times_tau_5);

   // compute weighted mean average tau(m)
   
   // compute weighted means & V2
   double v[5];
   double rv[5];
   double s[5];
   double rs[5];
   double avgv = 0.0;
   double avgrv = 0.0;

   v[0] = stats[0].rel_times_tau_1;
   v[1] = stats[0].rel_times_tau_2;
   v[2] = stats[0].rel_times_tau_3;
   v[3] = stats[0].rel_times_tau_4;
   v[4] = stats[0].rel_times_tau_5;


   s[0] = stats[1].rel_times_tau_1;
   s[1] = stats[1].rel_times_tau_2;
   s[2] = stats[1].rel_times_tau_3;
   s[3] = stats[1].rel_times_tau_4;
   s[4] = stats[1].rel_times_tau_5;

   double w[5];
   double rw[5];

   double sumw = 0.0;
   double sumrw = 0.0;

   // setup for harmonic means
   for ( unsigned int i = 0; i < 5; i++ )
   {
      rv[i] = 1.0 / v[i];
      rs[i] = s[i];
   }

   // compute weights

   // setup weights
   for ( unsigned int i = 0; i < 5; i++ )
   {
      w[i] = 1.0 / (s[i] * s[i]);
      rw[i] = 1.0 / (rs[i] * rs[i]);
      sumw += w[i];
      sumrw += rw[i];
      //      printf("unnormalized weight [%d] = %f\n", i, w[i]);
      //      printf("unnormalized rweight [%d] = %f\n", i, rw[i]);
   }

   // normalize weights
   for ( unsigned int i = 0; i < 5; i++ )
   {
      w[i] /= sumw;
      rw[i] /= sumrw;
      //      printf("normalized weight [%d] = %f\n", i, w[i]);
      //      printf("normalized rweight [%d] = %f\n", i, rw[i]);
   }

   // compute averages
   for ( unsigned int i = 0; i < 5; i++ )
   {
      avgv += v[i] * w[i];
      avgrv += rv[i] * rw[i];
   }

   //   printf("avgv %f avgrv %f\n", avgv, avgrv);

   // repeat process for s.d.'s

   v[0] = stats[1].rel_times_tau_1;
   v[1] = stats[1].rel_times_tau_2;
   v[2] = stats[1].rel_times_tau_3;
   v[3] = stats[1].rel_times_tau_4;
   v[4] = stats[1].rel_times_tau_5;

   s[0] = stats[1].rel_times_tau_1;
   s[1] = stats[1].rel_times_tau_2;
   s[2] = stats[1].rel_times_tau_3;
   s[3] = stats[1].rel_times_tau_4;
   s[4] = stats[1].rel_times_tau_5;

   // setup for harmonic means
   for ( unsigned int i = 0; i < 5; i++ )
   {
      rv[i] = 1.0 / v[i];
      rs[i] = s[i];
   }

   // compute weights
   sumw = 0.0;
   sumrw = 0.0;
   double sdv = 0.0;
   double sdrv = 0.0;

   // setup weights
   for ( unsigned int i = 0; i < 5; i++ )
   {
      w[i] = 1.0 / (s[i] * s[i]);
      rw[i] = 1.0 / (rs[i] * rs[i]);
      sumw += w[i];
      sumrw += rw[i];
      //      printf("unnormalized weight [%d] = %f\n", i, w[i]);
      //      printf("unnormalized rweight [%d] = %f\n", i, rw[i]);
   }

   // normalize weights
   for ( unsigned int i = 0; i < 5; i++ )
   {
      w[i] /= sumw;
      rw[i] /= sumrw;
      //      printf("normalized weight [%d] = %f\n", i, w[i]);
      //      printf("normalized rweight [%d] = %f\n", i, rw[i]);
   }

   // compute averages

   for ( unsigned int i = 0; i < 5; i++ )
   {
      sdv += v[i] * w[i];
      sdrv += rv[i] * rw[i];
   }

   //   printf("sdv %f sdrv %f\n", sdv, sdrv);

   
   //   result += QString("").sprintf(" Tau(m) (Weighted average)    \t%.2f\t\t\t\t[ns]\n",
   //                                 avgv);
   //   result += QString("").sprintf(" Tau(h) (Weighted average)    \t%.2f\t\t\t\t[ns]\n",
   //                                 1.0 / avgrv);
   
   result += QString("").sprintf("\n Tau(m) (Unweighted average)  \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_m,
                                 stats[1].rel_times_tau_m);
   result += QString("").sprintf(" Tau(h) (Unweighted average)  \t%.2f\t\t%.2f\t\t[ns]\n",
                                 stats[0].rel_times_tau_h,
                                 stats[1].rel_times_tau_h);
   
   result += QString("").sprintf("\n****************************************************************\n");
   return result;
}
