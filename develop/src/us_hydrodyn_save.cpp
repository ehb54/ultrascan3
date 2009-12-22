#include "../include/us_hydrodyn.h"
#include "../include/us_revision.h"
#include <qtabwidget.h>

US_Hydrodyn_Save::US_Hydrodyn_Save(
                                   save_info *save, 
                                   bool *save_widget, 
                                   void *us_hydrodyn, 
                                   QWidget *p, 
                                   const char *name
                                   ) : QFrame(p, name)
{
   this->save_widget = save_widget;
   this->save = save;
   this->us_hydrodyn = us_hydrodyn;
   *save_widget = true;
   USglobal = new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Hydrodynamic Parameters to be Saved"));
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

            // "used_beads_sd", 
            // "Used beads in model s.d.", 
            // "Used beads in model s.d.", 

            "__BREAK__",

            "results.mass", 
            "Molecular mass", 
            "Molecular mass",

            "__BREAK__",

            "results.vbar", 
            "Partial specific volume", 
            "Partial specific volume", 

            "__BREAK__",

            "results.s20w", 
            "Sedimentation coefficient s",
            "Sedimentation coefficient s",

            // "results.s20w_sd", 
            // "s(20,w) standard deviation", 
            // "s(20,w) standard deviation", 

            "__BREAK__",
            "results.D20w", 
            "Translational diffusion coefficient D",
            "Translational diffusion coefficient D",

            // "results.D20w_sd", 
            // "D(20,w) transl. s.d.", 
            // "D(20,w) transl. s.d.", 

            "__BREAK__",

            "results.rs", 
            "Stokes radius", 
            "Stokes radius", 

            // "results.rs_sd", 
            // "Stokes radius s.d.", 
            // "Stokes radius s.d.", 

            "__BREAK__",

            "results.ff0", 
            "Fricitional ratio", 
            "Fricitional ratio", 

            // "ff0_sd", 
            // "Fricitional ratio s.d.", 
            // "Fricitional ratio s.d.", 

            "__BREAK__",

            "results.rg", 
            "Radius of gyration (from bead model)", 
            "Radius of gyration (from bead model)", 

            // "results.rg_sd", 
            // "Radius of gyration s.d.", 
            // "Radius of gyration s.d.", 

            "__BREAK__",

            "results.tau", 
            "Relaxation Time, tau(h)", 
            "Relaxation Time, tau(h)", 

            // "results.tau_sd", 
            // "Relaxation Time, tau(h) s.d.", 
            // "Relaxation Time, tau(h) s.d.", 

            "__BREAK__",

            "results.viscosity", 
            "Intrisic viscosity", 
            "Intrisic viscosity", 

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
            "Translational frictional coefficient",
            "Translational frictional coefficient",

            "rot_fric_coef",
            "Rotational frictional coefficient [g*cm^2/s]",
            "Rotational frictional coefficient [g*cm^2/s]",

            "rot_diff_coef",
            "Rotational diffusion coefficient [1/s]",
            "Rotational diffusion coefficient [1/s]",

            "rot_fric_coef",
            "Rotational frictional coefficient [ X, Y, Z ] [g*cm^2/s]",
            "Rotational frictional coefficient [ X, Y, Z ] [g*cm^2/s]",

            "rot_diff_coef",
            "Rotational diffusion coefficient [ X, Y, Z ] [1/s]",
            "Rotational diffusion coefficient [ X, Y, Z ] [1/s]",

            "rot_stokes_rad",
            "Rotational Stokes' radius [ X, Y, Z ] [nm]",
            "Rotational Stokes' radius [ X, Y, Z ] [nm]",

            "cen_of_res",
            "Centre of resistance [ X, Y, Z ] [nm]",
            "Centre of resistance [ X, Y, Z ] [nm]",

            "cen_of_mass",
            "Centre of mass [ X, Y, Z ] [nm]",
            "Centre of mass [ X, Y, Z ] [nm]",

            "cen_of_diff",
            "Centre of diffusion [ X, Y, Z ] [nm]",
            "Centre of diffusion [ X, Y, Z ] [nm]",

            "cen_of_visc",
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

            "max_ext",
            "Maximum extensions [ X, Y, Z ] nm",
            "Maximum extensions [ X, Y, Z ] nm",

            "axi_ratios",
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
            "Solvent Temperature (C)", 
            "Solvent Temperature (C)", 

            "hydro.solvent_viscosity", 
            "Solvent viscosity (cP)", 
            "Solvent viscosity (cP)", 

            "hydro.solvent_density", 
            "Solvent density (g/ml)", 
            "solvent_density (g/ml)",

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
         printf("in while, data[%u] is <%s>\n", i, data[i].ascii());
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
            printf("in while, section is <%s>\n", this_section.ascii());
            i++;
            is_section = true;
         }
         if ( data[i] == "__BREAK__" ) 
         {
            i++;
            is_row_break = true;
         }
         pos = field.size();
         printf("in while, field[%u] is <%s>\n", i, data[i].ascii());
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
            
   // for now:  until we save/restore
   save->field.clear();
   save->field_flag.clear();

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      printf("read field %u <%s>\n", i, field[i].ascii());
   }

   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 640, 480);
}

US_Hydrodyn_Save::~US_Hydrodyn_Save()
{
   *save_widget = false;
}

void US_Hydrodyn_Save::setupGUI()
{

   int minHeight1 = 30;
   int minWidth1 = 150;

   lbl_possible = new QLabel(tr("Parameters available"), this);
   Q_CHECK_PTR(lbl_possible);
   lbl_possible->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_possible->setAlignment(AlignCenter|AlignVCenter);
   lbl_possible->setMinimumHeight(minHeight1);
   lbl_possible->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_possible->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   tw_possible = new QTabWidget(this);
   // tw_possible->setTabBar(tb_possible);
   // tw_possible->tabBar()
   tw_possible->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(tw_possible, SIGNAL(currentChanged(QWidget *)), SLOT(tab_changed(QWidget *)));
   QListBox *this_lb;

   for ( unsigned int i = 0; i < field.size(); i++ )
   {
      if ( section_name.count(field[i]) )
      {
         this_lb = new QListBox(this);
         Q_CHECK_PTR(this_lb);
         this_lb->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
         this_lb->setMinimumHeight(minHeight1 * 3);
         this_lb->setMinimumWidth(minWidth1);
         this_lb->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
         this_lb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
         this_lb->setEnabled(true);
         lb_possible.push_back(this_lb);
         tw_possible->addTab(this_lb, section_name[field[i]]);
         connect(this_lb, SIGNAL(selectionChanged()), SLOT(update_enables_possible()));
         this_lb->setSelectionMode(QListBox::Multi);
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

   lbl_selected = new QLabel(tr("Parameters selected"), this);
   Q_CHECK_PTR(lbl_selected);
   lbl_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_selected->setAlignment(AlignCenter|AlignVCenter);
   lbl_selected->setMinimumHeight(minHeight1);
   lbl_selected->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lb_selected = new QListBox(this);
   Q_CHECK_PTR(lb_selected);
   lb_selected->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lb_selected->setMinimumHeight(minHeight1 * 3);
   lb_selected->setMinimumWidth(minWidth1);
   lb_selected->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
   lb_selected->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));
   lb_selected->setEnabled(true);
   for ( unsigned int i = 0; i < field.size(); i++ ) 
   {
      if ( save->field_flag.count(field[i]) )
      {
         lb_selected->insertItem(descriptive_name[i]);
      }
   }
   lb_selected->setSelectionMode(QListBox::Multi);
   connect(lb_selected, SIGNAL(selectionChanged()), SLOT(update_enables_selected()));

   pb_add = new QPushButton("-->", this);
   Q_CHECK_PTR(pb_add);
   pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3));
   pb_add->setMinimumHeight(minHeight1);
   pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_add, SIGNAL(clicked()), SLOT(add()));

   pb_remove = new QPushButton("<--", this);
   Q_CHECK_PTR(pb_remove);
   pb_remove->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 3));
   pb_remove->setMinimumHeight(minHeight1);
   pb_remove->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_remove, SIGNAL(clicked()), SLOT(remove()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   // build layout
   // left box / possible

   QVBoxLayout *vbl_possible = new QVBoxLayout;
   vbl_possible->addWidget(lbl_possible);
   vbl_possible->addWidget(tw_possible);

   QVBoxLayout *vbl_add_remove = new QVBoxLayout;
   vbl_add_remove->addSpacing(20);
   vbl_add_remove->addWidget(pb_add);
   vbl_add_remove->addWidget(pb_remove);
   vbl_add_remove->addSpacing(20);

   QVBoxLayout *vbl_selected = new QVBoxLayout;
   vbl_selected->addWidget(lbl_selected);
   vbl_selected->addWidget(lb_selected);

   QHBoxLayout *hbl_top = new QHBoxLayout;
   hbl_top->addSpacing(5);
   hbl_top->addLayout(vbl_possible);
   hbl_top->addSpacing(20);
   hbl_top->addLayout(vbl_add_remove);
   hbl_top->addSpacing(20);
   hbl_top->addLayout(vbl_selected);
   hbl_top->addSpacing(5);

   QHBoxLayout *hbl_bottom = new QHBoxLayout;
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_help);
   hbl_bottom->addSpacing(5);
   hbl_bottom->addWidget(pb_cancel);
   hbl_bottom->addSpacing(5);

   QVBoxLayout *background = new QVBoxLayout(this);
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
      if ( lb_selected->isSelected(i) )
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
}

void US_Hydrodyn_Save::tab_changed(QWidget *)
{
   update_enables_possible();
}
