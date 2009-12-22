#ifndef US_HYDRODYN_SAVE_H
#define US_HYDRODYN_SAVE_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qwt_counter.h>
#include <qbuttongroup.h>
#include <qtextedit.h>
#include <qprogressbar.h>
#include <qmenubar.h>
#include <qfileinfo.h>
#include <qprinter.h>
#include <qlistbox.h>
#include <qtabwidget.h>

#include "us_util.h"

//standard C and C++ defs:

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "../include/us_hydrodyn_results.h"
#include "../include/us_hydrodyn_hydro.h"

using namespace std;

struct save_data
{
   hydro_options hydro;
   hydro_results results;
   QString       hydro_res;         // copy of what is written to hydro_res file (model specific)
   
   QString       model_idx;
   double        tot_surf_area;     // "Total surface area of beads in the model"
   double        tot_volume_of;     // "Total volume of beads in the model"
   double        num_of_unused;     // "Number of unused beads"
   double        use_beads_vol;     // "Used beads volume [nm^3]"
   double        use_beads_surf;    // "Used beads surface area [nm^2]"
   double        use_bead_mass;     // "Used bead mass [Da]"
   double        con_factor;        // "Conversion Factor"
   double        tra_fric_coef;     // "Translational frictional coefficient [g*cm^2/s]"
   double        rot_fric_coef;     // "Rotational frictional coefficient [g*cm^2/s]"
   double        rot_diff_coef;     // "Rotational diffusion coefficient [1/s]"
   double        rot_fric_coef_x;   // "Rotational frictional coefficient [ X ]"
   double        rot_fric_coef_y;   // "Rotational frictional coefficient [ Y ]"
   double        rot_fric_coef_z;   // "Rotational frictional coefficient [ Z ]"
   double        rot_diff_coef_x;   // "Rotational diffusion coefficient [ X ] [1/s]"
   double        rot_diff_coef_y;   // "Rotational diffusion coefficient [ Y ] [1/s]"
   double        rot_diff_coef_z;   // "Rotational diffusion coefficient [ Z ] [1/s]"
   double        rot_stokes_rad_x;  // "Rotational Stokes' radius [ X ] [nm]"
   double        rot_stokes_rad_y;  // "Rotational Stokes' radius [ Y ] [nm]"
   double        rot_stokes_rad_z;  // "Rotational Stokes' radius [ Z ] [nm]"
   double        cen_of_res_x;      // "Centre of resistance [ X ] [nm]"
   double        cen_of_res_y;      // "Centre of resistance [ Y ] [nm]"
   double        cen_of_res_z;      // "Centre of resistance [ Z ] [nm]"
   double        cen_of_mass_x;     // "Centre of mass [ X ] [nm]"
   double        cen_of_mass_y;     // "Centre of mass [ Y ] [nm]"
   double        cen_of_mass_z;     // "Centre of mass [ Z ] [nm]"
   double        cen_of_diff_x;     // "Centre of diffusion [ X ] [nm]"
   double        cen_of_diff_y;     // "Centre of diffusion [ Y ] [nm]"
   double        cen_of_diff_z;     // "Centre of diffusion [ Z ] [nm]"
   double        cen_of_visc_x;     // "Centre of viscosity [ X ] [nm]"
   double        cen_of_visc_y;     // "Centre of viscosity [ Y ] [nm]"
   double        cen_of_visc_z;     // "Centre of viscosity [ Z ] [nm]"
   double        unc_int_visc;      // "Uncorrected intrinsic viscosity	[cm^3/g]"
   double        unc_einst_rad;     // "Uncorrected Einstein's radius [nm]"
   double        cor_int_visc;      // "Corrected intrinsic viscosity [cm^3/g]"
   double        cor_einst_rad;     // "Corrected Einstein's radius [nm]"
   double        rel_times_tau_1;   // "Relaxation times, tau(1) [ns]"
   double        rel_times_tau_2;   // "Relaxation times, tau(2) [ns]"
   double        rel_times_tau_3;   // "Relaxation times, tau(3) [ns]"
   double        rel_times_tau_4;   // "Relaxation times, tau(4) [ns]"
   double        rel_times_tau_5;   // "Relaxation times, tau(5) [ns]"
   double        rel_times_tau_m;   // "Relaxation times, tau(m) [ns]"
   double        rel_times_tau_h;   // "Relaxation times, tau(h) [ns]"
   double        max_ext_x;         // "Maximum extensions [ X ] nm"
   double        max_ext_y;         // "Maximum extensions [ Y ] nm"
   double        max_ext_z;         // "Maximum extensions [ Z ] nm"
   double        axi_ratios_xz;     // "Axial ratios [ X:Z ] "
   double        axi_ratios_xy;     // "Axial ratios [ X:Y ] "
   double        axi_ratios_yz;     // "Axial ratios [ Y:Z ] "
};

struct save_info 
{
   QString file;
   vector < QString >    field;
   map < QString, bool > field_flag;
   vector < save_data >  data;
};

class US_EXTERN US_Hydrodyn_Save : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Save(save_info *save, 
                        bool *save_widget, 
                        void *us_hydrodyn, 
                        QWidget *p = 0, 
                        const char *name = 0);
      ~US_Hydrodyn_Save();

   private:

      save_info                     *save;
      bool                          *save_widget;
      void                          *us_hydrodyn;

      US_Config                     *USglobal;

      QLabel                        *lbl_possible;
      QLabel                        *lbl_selected;


      QTabWidget                    *tw_possible;
      QListBox                      *lb_selected;

      QPushButton                   *pb_add;
      QPushButton                   *pb_remove;

#ifdef WIN32
  #pragma warning ( disable: 4251 )
#endif
      vector < QListBox * >         lb_possible;

      vector < QString >            field;
      vector < QString >            descriptive_name;
      vector < QString >            short_name;
      map < QString, unsigned int > field_to_pos;
      map < QString, unsigned int > descriptive_name_to_pos;
      map < QString, QString >      section_name;
      map < QString, bool >         row_break;
      map < QString, int >          descriptive_name_to_section;

#ifdef WIN32
  #pragma warning ( default: 4251 )
#endif

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;

   private slots:
      
      void setupGUI();

      void add();
      void remove();

      void rebuild();

      void tab_changed(QWidget *);

      void update_enables_selected();
      void update_enables_possible();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
