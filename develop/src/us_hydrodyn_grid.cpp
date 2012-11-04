#include "../include/us_hydrodyn_grid.h"
#include "../include/us_hydrodyn.h"

US_Hydrodyn_Grid::US_Hydrodyn_Grid(struct overlap_reduction *grid_exposed_overlap,
                                   struct overlap_reduction *grid_buried_overlap,
                                   struct overlap_reduction *grid_overlap,
                                   bool *replicate_o_r_method_grid,
                                   struct grid_options *grid,
                                   double *overlap_tolerance,
                                   bool *grid_widget,
                                   void *us_hydrodyn,
                                   QWidget *p, 
                                   const char *name) : QFrame(p, name)
{
   this->grid_exposed_overlap = grid_exposed_overlap;
   this->grid_buried_overlap = grid_buried_overlap;
   this->grid_overlap = grid_overlap;
   this->replicate_o_r_method_grid = replicate_o_r_method_grid;
   this->grid = grid;
   this->grid_widget = grid_widget;
   this->overlap_tolerance = overlap_tolerance;
   this->us_hydrodyn = us_hydrodyn;
   *grid_widget = true;
   USglobal=new US_Config();
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("SOMO Grid Function Options (AtoB)"));
   setupGUI();
   global_Xpos += 30;
   global_Ypos += 30;
   setGeometry(global_Xpos, global_Ypos, 0, 0);
   overlap_widget = false;
}

US_Hydrodyn_Grid::~US_Hydrodyn_Grid()
{
   *grid_widget = false;
}

void US_Hydrodyn_Grid::setupGUI()
{
   int minHeight1 = 30;
   QString str;   
   lbl_info = new QLabel(tr("SOMO Grid Function Options (AtoB):"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(Qt::AlignCenter|Qt::AlignVCenter);
   lbl_info->setMinimumHeight(minHeight1);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

   lbl_cube_side = new QLabel(tr(" Cube Side (Angstrom): "), this);
   Q_CHECK_PTR(lbl_cube_side);
   lbl_cube_side->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
   lbl_cube_side->setMinimumHeight(minHeight1);
   lbl_cube_side->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_cube_side->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-1, QFont::Bold));

   cnt_cube_side= new QwtCounter(this);
   US_Hydrodyn::sizeArrows( cnt_cube_side );
   Q_CHECK_PTR(cnt_cube_side);
   cnt_cube_side->setRange(0.1, 100, 0.1);
   cnt_cube_side->setValue((*grid).cube_side);
   cnt_cube_side->setMinimumHeight(minHeight1);
   cnt_cube_side->setEnabled(true);
   cnt_cube_side->setNumButtons(3);
   cnt_cube_side->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_cube_side->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cnt_cube_side, SIGNAL(valueChanged(double)), SLOT(update_cube_side(double)));

#if defined( CSI_TEST )
   bg_center = new QButtonGroup(3, Qt::Horizontal, "Computations Relative to:", this);
#else
   bg_center = new QButtonGroup(2, Qt::Horizontal, "Computations Relative to:", this);
#endif
   bg_center->setExclusive(true);
   connect(bg_center, SIGNAL(clicked(int)), this, SLOT(select_center(int)));

   cb_center_mass = new QCheckBox(bg_center);
   cb_center_mass->setText(tr(" Center of Mass "));
   cb_center_mass->setEnabled(true);
   cb_center_mass->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_center_mass->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   cb_center_cubelet = new QCheckBox(bg_center);
   cb_center_cubelet->setText(tr(" Center of Cubelet "));
   cb_center_cubelet->setEnabled(true);
   cb_center_cubelet->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_center_cubelet->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

#if defined( CSI_TEST )
   cb_center_si = new QCheckBox(bg_center);
   cb_center_si->setText(tr(" Center of scattering intensity"));
   cb_center_si->setEnabled(true);
   cb_center_si->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_center_si->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
#endif

   bg_center->setButton((*grid).center);

   cb_cubic = new QCheckBox(this);
   cb_cubic->setText(tr(" Apply Cubic Grid "));
   cb_cubic->setChecked((*grid).cubic);
   cb_cubic->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_cubic->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_cubic, SIGNAL(clicked()), this, SLOT(set_cubic()));

   cb_hydrate = new QCheckBox(this);
   cb_hydrate->setText(tr(" Add theoretical hydration (PDB only)"));
   cb_hydrate->setChecked((*grid).hydrate);
   cb_hydrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_hydrate->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_hydrate->setEnabled(true);
   connect(cb_hydrate, SIGNAL(clicked()), this, SLOT(set_hydrate()));

   cb_tangency = new QCheckBox(this);
   cb_tangency->setText(tr(" Expand Beads to Tangency "));
   cb_tangency->setChecked((*grid).tangency);
   cb_tangency->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_tangency->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_tangency->setEnabled(false);
   connect(cb_tangency, SIGNAL(clicked()), this, SLOT(set_tangency()));

   cb_enable_asa = new QCheckBox(this);
   cb_enable_asa->setText(tr(" Enable ASA screening "));
   cb_enable_asa->setChecked((*grid).enable_asa);
   cb_enable_asa->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_enable_asa->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_enable_asa->setEnabled(true);
   connect(cb_enable_asa, SIGNAL(clicked()), this, SLOT(set_enable_asa()));

   cb_create_nmr_bead_pdb = new QCheckBox(this);
   cb_create_nmr_bead_pdb->setText(tr(" Compute structure factors for beads"));
   cb_create_nmr_bead_pdb->setChecked((*grid).create_nmr_bead_pdb);
   cb_create_nmr_bead_pdb->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_create_nmr_bead_pdb->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_create_nmr_bead_pdb->setEnabled(true);
   connect(cb_create_nmr_bead_pdb, SIGNAL(clicked()), this, SLOT(set_create_nmr_bead_pdb()));

   cb_equalize_radii_constant_volume = new QCheckBox(this);
   cb_equalize_radii_constant_volume->setText(tr(" Equalize radii, constant volume"));
   cb_equalize_radii_constant_volume->setChecked((*grid).equalize_radii_constant_volume);
   cb_equalize_radii_constant_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_equalize_radii_constant_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cb_equalize_radii_constant_volume->setEnabled(true);
   connect(cb_equalize_radii_constant_volume, SIGNAL(clicked()), this, SLOT(set_equalize_radii_constant_volume()));

   pb_overlaps = new QPushButton(tr(" Adjust Overlap Options "), this);
   Q_CHECK_PTR(pb_overlaps);
   pb_overlaps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_overlaps->setMinimumHeight(minHeight1);
   pb_overlaps->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_overlaps, SIGNAL(clicked()), SLOT(overlaps()));

   pb_cancel = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_cancel->setMinimumHeight(minHeight1);
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
   pb_help->setMinimumHeight(minHeight1);
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   int rows=7, columns = 2, spacing = 2, j=0, margin=4;
   QGridLayout *background=new QGridLayout(this, rows, columns, margin, spacing);

   background->addMultiCellWidget(lbl_info, j, j, 0, 1);
   j++;
   background->addMultiCellWidget(bg_center, j, j+2, 0, 1);
   j+=3;
   background->addWidget(lbl_cube_side, j, 0);
   background->addWidget(cnt_cube_side, j, 1);
   j++;
   background->addWidget(cb_cubic, j, 0);
   background->addWidget(cb_hydrate, j, 1);
   j++;
   background->addWidget(pb_overlaps, j, 0);
   background->addWidget(cb_tangency, j, 1);
   j++;
   background->addWidget(cb_enable_asa, j, 1);
   j++;
   background->addWidget(cb_create_nmr_bead_pdb, j, 1);
   j++;
   background->addWidget(cb_equalize_radii_constant_volume, j, 1);
   j++;
   background->addWidget(pb_help, j, 0);
   background->addWidget(pb_cancel, j, 1);
}

void US_Hydrodyn_Grid::update_cube_side(double val)
{
   (*grid).cube_side = val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::select_center(int val)
{
   (*grid).center = (bool) val;
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_hydrate()
{
   (*grid).hydrate = cb_hydrate->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_cubic()
{
   (*grid).cubic = cb_cubic->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_tangency()
{
   (*grid).tangency = cb_tangency->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_enable_asa()
{
   (*grid).enable_asa = cb_enable_asa->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_create_nmr_bead_pdb()
{
   (*grid).create_nmr_bead_pdb = cb_create_nmr_bead_pdb->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::set_equalize_radii_constant_volume()
{
   (*grid).equalize_radii_constant_volume = cb_equalize_radii_constant_volume->isChecked();
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();
}

void US_Hydrodyn_Grid::cancel()
{
   close();
}

void US_Hydrodyn_Grid::overlaps()
{
   if (overlap_widget)
   {
      if (overlap_window->isVisible())
      {
         overlap_window->raise();
      }
      else
      {
         overlap_window->show();
      }
      return;
   }
   else
   {
      overlap_window = new US_Hydrodyn_Overlap(grid_exposed_overlap,
                                               grid_buried_overlap,
                                               grid_overlap, 
                                               replicate_o_r_method_grid,
                                               overlap_tolerance, 
                                               &overlap_widget, 
                                               us_hydrodyn);
      overlap_window->show();
   }
   ((US_Hydrodyn *)us_hydrodyn)->display_default_differences();

}

void US_Hydrodyn_Grid::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/somo_grid.html");
}

void US_Hydrodyn_Grid::closeEvent(QCloseEvent *e)
{
   if (overlap_widget)
   {
      overlap_window->close();
   }
   *grid_widget = false;
   global_Xpos -= 30;
   global_Ypos -= 30;
   e->accept();
}

