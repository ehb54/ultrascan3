#include "../include/us_ga_model_editor.h"

US_GAModelEditor::US_GAModelEditor(struct ModelSystem *ms, QWidget *parent, const char *name) : US_ModelEditor(false, ms, parent, name)
{
   this->ms = ms;
   current_component = 0;
   current_assoc = 0;
   msc.simpoints = 200;    // number of radial grid points used in simulation
   msc.mesh = 0;           // 0 = ASTFEM, 1 = Claverie, 2 = moving hat,
                           // 3 = user-selected mesh, 4 = nonuniform constant mesh
   msc.moving_grid = 1;    // Use moving (1) or fixed time grid (0)
   msc.band_volume = (float) 0.015;

   connect(this, SIGNAL(componentChanged(unsigned int)), this, SLOT(update_constraints(unsigned int)));
   connect(this, SIGNAL(modelLoaded()), this, SLOT(initialize_msc()));
   connect(le_mw, SIGNAL(returnPressed()), this, SLOT(verify_constraints()));
   connect(le_conc, SIGNAL(returnPressed()), this, SLOT(verify_constraints()));
   connect(le_f_f0, SIGNAL(returnPressed()), this, SLOT(verify_constraints()));
   connect(le_keq, SIGNAL(returnPressed()), this, SLOT(verify_constraints()));
   connect(le_koff, SIGNAL(returnPressed()), this, SLOT(verify_constraints()));
   setup_GUI();
   initialize_msc();
   select_component((int) current_component);

   global_Xpos += 30;
   global_Ypos += 30;

   move(global_Xpos, global_Ypos);
}

US_GAModelEditor::~US_GAModelEditor()
{
}

void US_GAModelEditor::setup_GUI()
{
   int minHeight1 = 30, minHeight2 = 26;
   delete pb_accept;
   delete pb_save;
   delete pb_cancel;

   lbl_constraints = new QLabel(tr("Constraints for Current Component:"), this);
   lbl_constraints->setAlignment(AlignHCenter|AlignVCenter);
   lbl_constraints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_constraints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_constraints->setMinimumHeight(minHeight2);

   lbl_low = new QLabel(tr("Low:"), this);
   lbl_low->setAlignment(AlignHCenter|AlignVCenter);
   lbl_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_low->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_low->setMinimumHeight(minHeight2);

   lbl_high = new QLabel(tr("High:"), this);
   lbl_high->setAlignment(AlignHCenter|AlignVCenter);
   lbl_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_high->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_high->setMinimumHeight(minHeight2);

   lbl_fit = new QLabel(tr("Fit?"), this);
   lbl_fit->setAlignment(AlignHCenter|AlignVCenter);
   lbl_fit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_fit->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_fit->setMinimumHeight(minHeight2);

   lbl_bandVolume = new QLabel(tr(" Band-loading Volume (ml):"), this);
   lbl_bandVolume->setAlignment(AlignLeft|AlignVCenter);
   lbl_bandVolume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_bandVolume->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_bandVolume->setMinimumHeight(minHeight2);

   lbl_simpoints = new QLabel(tr(" # of Simulation Points:"), this);
   lbl_simpoints->setAlignment(AlignLeft|AlignVCenter);
   lbl_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_simpoints->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_simpoints->setMinimumHeight(minHeight2);

   cc_mw = new US_ConstraintControl(this);
   connect(cc_mw, SIGNAL(constraintChanged(struct constraint)), this, SLOT(mw_constraintChanged(struct constraint)));
   cc_mw->hide();

   cc_f_f0 = new US_ConstraintControl(this);
   connect(cc_f_f0, SIGNAL(constraintChanged(struct constraint)), this, SLOT(f_f0_constraintChanged(struct constraint)));
   cc_f_f0->hide();

   cc_conc = new US_ConstraintControl(this);
   connect(cc_conc, SIGNAL(constraintChanged(struct constraint)), this, SLOT(conc_constraintChanged(struct constraint)));
   cc_conc->hide();

   cc_keq = new US_ConstraintControl(this);
   connect(cc_keq, SIGNAL(constraintChanged(struct constraint)), this, SLOT(keq_constraintChanged(struct constraint)));
   cc_keq->hide();

   cc_koff = new US_ConstraintControl(this);
   connect(cc_koff, SIGNAL(constraintChanged(struct constraint)), this, SLOT(koff_constraintChanged(struct constraint)));
   cc_koff->hide();

   cmb_radialGrid = new QComboBox(false, this, "Radial Grid" );
   cmb_radialGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cmb_radialGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_radialGrid->setSizeLimit(5);
   cmb_radialGrid->setMinimumHeight(minHeight1);
   cmb_radialGrid->insertItem("Adaptive Space Mesh (ASTFEM)", -1);
   cmb_radialGrid->insertItem("Claverie Fixed Mesh", -1);
   cmb_radialGrid->insertItem("Moving Hat Mesh", -1);
   cmb_radialGrid->insertItem("File: \"$ULTRASCAN/mesh.dat\"", -1);
   cmb_radialGrid->setCurrentItem(msc.mesh);
   connect(cmb_radialGrid, SIGNAL(activated(int)), this, SLOT(update_radialGrid(int)));

   cmb_timeGrid = new QComboBox(false, this, "Time Grid" );
   cmb_timeGrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cmb_timeGrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cmb_timeGrid->setSizeLimit(5);
   cmb_timeGrid->setMinimumHeight(minHeight1);
   cmb_timeGrid->insertItem("Constant Time Grid (Claverie/Acceleration)", -1);
   cmb_timeGrid->insertItem("Moving Time Grid (ASTFEM/Moving Hat)", -1);
   cmb_timeGrid->setCurrentItem(msc.moving_grid);
   connect(cmb_timeGrid, SIGNAL(activated(int)), this, SLOT(update_timeGrid(int)));

   cnt_simpoints= new QwtCounter(this);
   cnt_simpoints->setNumButtons(3);
   cnt_simpoints->setRange(50, 5000, 10);
   cnt_simpoints->setValue(msc.simpoints);
   cnt_simpoints->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_simpoints->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_simpoints->setMinimumHeight(minHeight1);
   connect(cnt_simpoints, SIGNAL(valueChanged(double)), SLOT(update_simpoints(double)));

   cnt_band_volume = new QwtCounter(this);
   cnt_band_volume->setRange(0.001, 0.1, 0.0001);
   cnt_band_volume->setNumButtons(3);
   cnt_band_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_band_volume->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_band_volume->setValue(msc.band_volume);
   cnt_band_volume->setMinimumHeight(minHeight1);
   connect(cnt_band_volume, SIGNAL(valueChanged(double)), SLOT(update_band_volume(double)));

   pb_selectModel = new QPushButton( tr("Select Model"), this );
   pb_selectModel->setAutoDefault(false);
   pb_selectModel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_selectModel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_selectModel->setMinimumHeight(minHeight1);
   connect(pb_selectModel, SIGNAL(clicked()), SLOT(select_model()) );

   pb_loadInit = new QPushButton( tr("Load Initialization"), this );
   pb_loadInit->setAutoDefault(false);
   pb_loadInit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_loadInit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_loadInit->setMinimumHeight(minHeight1);
   connect(pb_loadInit, SIGNAL(clicked()), SLOT(load_constraints()) );

   pb_saveInit = new QPushButton( tr("Save Initialization"), this );
   pb_saveInit->setAutoDefault(false);
   pb_saveInit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_saveInit->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_saveInit->setMinimumHeight(minHeight1);
   connect(pb_saveInit, SIGNAL(clicked()), SLOT(save_constraints()) );

   pb_close = new QPushButton( tr("Close"), this );
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setMinimumHeight(minHeight1);
   connect(pb_close, SIGNAL(clicked()), SLOT(accept()));

   le_sed->setReadOnly(true);
   le_diff->setReadOnly(true);

   unsigned int j;
   QGridLayout *grid = new QGridLayout(this, 17, 5, 4, 2);
   grid->addMultiCellWidget(lbl_model, 0, 0, 0, 4, 0);
   grid->addMultiCellWidget(lbl_current, 1, 1, 0, 1, 0);
   grid->addMultiCellWidget(lbl_linked, 1, 1, 2, 4, 0);
   grid->addMultiCellWidget(cmb_component1, 2, 2, 0, 1, 0);
   grid->addMultiCellWidget(cmb_component2, 2, 2, 2, 4, 0);
   j = 3;
   grid->addWidget(pb_simulateComponent, j, 0, 0);
   grid->addWidget(cnt_item, j, 1, 0);
   grid->addWidget(lbl_stoich, j, 2, 0);
   grid->addMultiCellWidget(le_stoich, j, j, 3, 4, 0);
   j++;
   grid->addWidget(cb_prolate, j, 0, 0);
   grid->addWidget(cb_oblate, j, 1, 0);
   grid->addWidget(pb_load_c0, j, 2, 0);
   grid->addMultiCellWidget(lbl_load_c0, j, j, 3, 4, 0);
   j++;
   grid->addWidget(cb_rod, j, 0, 0);
   grid->addWidget(cb_sphere, j, 1, 0);
   grid->addWidget(pb_vbar, j, 2, 0);
   grid->addMultiCellWidget(le_vbar, j, j, 3, 4, 0);
   j++;
   grid->addWidget(lbl_sed, j, 0, 0);
   grid->addWidget(le_sed, j, 1, 0);
   grid->addWidget(lbl_sigma, j, 2, 0);
   grid->addMultiCellWidget(le_sigma, j, j, 3, 4, 0);
   j++;
   grid->addWidget(lbl_diff, j, 0, 0);
   grid->addWidget(le_diff, j, 1, 0);
   grid->addWidget(lbl_delta, j, 2, 0);
   grid->addMultiCellWidget(le_delta, j, j, 3, 4, 0);
   j++;
   grid->addWidget(lbl_extinction, j, 0, 0);
   grid->addWidget(le_extinction, j, 1, 0);
   grid->addMultiCellWidget(lbl_extinction2, j, j, 2, 4, 0);
   j++;
   grid->addMultiCellWidget(lbl_constraints, j, j, 0, 1, 0);
   grid->addWidget(lbl_low, j, 2, 0);
   grid->addWidget(lbl_high, j, 3, 0);
   grid->addWidget(lbl_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_mw, j, 0, 0);
   grid->addWidget(le_mw, j, 1, 0);
   grid->addWidget(cc_mw->le_low, j, 2, 0);
   grid->addWidget(cc_mw->le_high, j, 3, 0);
   grid->addWidget(cc_mw->cb_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_f_f0, j, 0, 0);
   grid->addWidget(le_f_f0, j, 1, 0);
   grid->addWidget(cc_f_f0->le_low, j, 2, 0);
   grid->addWidget(cc_f_f0->le_high, j, 3, 0);
   grid->addWidget(cc_f_f0->cb_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_conc, j, 0, 0);
   grid->addWidget(le_conc, j, 1, 0);
   grid->addWidget(cc_conc->le_low, j, 2, 0);
   grid->addWidget(cc_conc->le_high, j, 3, 0);
   grid->addWidget(cc_conc->cb_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_keq, j, 0, 0);
   grid->addWidget(le_keq, j, 1, 0);
   grid->addWidget(cc_keq->le_low, j, 2, 0);
   grid->addWidget(cc_keq->le_high, j, 3, 0);
   grid->addWidget(cc_keq->cb_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_koff, j, 0, 0);
   grid->addWidget(le_koff, j, 1, 0);
   grid->addWidget(cc_koff->le_low, j, 2, 0);
   grid->addWidget(cc_koff->le_high, j, 3, 0);
   grid->addWidget(cc_koff->cb_fit, j, 4, 0);
   j++;
   grid->addWidget(lbl_simpoints, j, 0, 0);
   grid->addWidget(cnt_simpoints, j, 1, 0);
   grid->addMultiCellWidget(cmb_radialGrid, j, j, 2, 3, 0);
   grid->addWidget(pb_help, j, 4, 0);
   j++;
   grid->addWidget(lbl_bandVolume, j, 0, 0);
   grid->addWidget(cnt_band_volume, j, 1, 0);
   grid->addMultiCellWidget(cmb_timeGrid, j, j, 2, 3, 0);
   grid->addWidget(pb_close, j, 4, 0);
   j++;
   grid->addWidget(pb_selectModel, j, 0, 0);
   grid->addWidget(pb_load_model, j, 1, 0);
   grid->addWidget(pb_loadInit, j, 2, 0);
   grid->addMultiCellWidget(pb_saveInit, j, j, 3, 4, 0);
}

void US_GAModelEditor::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/ga_model_editor.html");
}

void US_GAModelEditor::update_radialGrid(int val)
{
   msc.mesh = val;
}

void US_GAModelEditor::update_timeGrid(int val)
{
   msc.moving_grid = val;
}

void US_GAModelEditor::update_simpoints(double val)
{
   msc.simpoints = (unsigned int) val;
}

void US_GAModelEditor::update_band_volume(double val)
{
   msc.band_volume = (float) val;
}

void US_GAModelEditor::load_constraints()
{
   QString fn = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.constraints", 0);
   if ( !fn.isEmpty() )
   {
      US_FemGlobal *fg;
      fg = new US_FemGlobal(this);
      (*ms).component_vector.clear();
      (*ms).assoc_vector.clear();
      msc.component_vector_constraints.clear();
      msc.assoc_vector_constraints.clear();
      fg->read_constraints(ms, &msc, fn);
      delete fg;
      cnt_simpoints->setValue(msc.simpoints);
      cnt_band_volume->setValue(msc.band_volume);
      cmb_timeGrid->setCurrentItem(msc.moving_grid);
      cmb_radialGrid->setCurrentItem(msc.mesh);
      lbl_model->setText(modelString[(*ms).model]);
      cmb_component1->clear();
      for (unsigned int i=0; i<(*ms).component_vector.size(); i++)
      {
         cmb_component1->insertItem((*ms).component_vector[i].name);
      }
      cnt_item->setRange(1, (*ms).component_vector.size(), 1);
      // after selecting a model we need to allocate memory for msc in initialize_msc
      current_component = 0; // reset to the first component
      current_assoc = 0;
      select_component((int) current_component);
   }
}

bool US_GAModelEditor::verify_constraints()
{
   unsigned int i;
   QString str, message;
   message = tr("The following constraints did not have the proper range and were adjusted:\n\n");
   bool flag=true;
   float low, high;
   for (i=0; i<(*ms).component_vector.size(); i++)
   {
      if (msc.component_vector_constraints[i].f_f0.fit)
      {
         if ( msc.component_vector_constraints[i].f_f0.low >
              (*ms).component_vector[i].f_f0
              ||   msc.component_vector_constraints[i].f_f0.high <
              (*ms).component_vector[i].f_f0)
         {
            cc_f_f0->update((*ms).component_vector[i].f_f0 - 1.0, 0.5, &low, &high, 1.0);
            msc.component_vector_constraints[i].f_f0.low = low;
            msc.component_vector_constraints[i].f_f0.high = high;
            message += tr(str.sprintf("The constraints for the frictional ratio of component %d\n", i+1));
            flag = false;
         }
      }
      if (msc.component_vector_constraints[i].mw.fit)
      {
         if ( msc.component_vector_constraints[i].mw.low >
              (*ms).component_vector[i].mw
              ||   msc.component_vector_constraints[i].mw.high <
              (*ms).component_vector[i].mw)
         {
            cc_mw->update((*ms).component_vector[i].mw, (float) 0.2, &low, &high, 0.0);
            msc.component_vector_constraints[i].mw.low = low;
            msc.component_vector_constraints[i].mw.high = high;
            message += tr(str.sprintf("The constraints for the molecular weight of component %d\n", i+1));
            flag = false;
         }
      }
      if (msc.component_vector_constraints[i].concentration.fit)
      {
         if ( msc.component_vector_constraints[i].concentration.low >
              (*ms).component_vector[i].concentration
              ||   msc.component_vector_constraints[i].concentration.high <
              (*ms).component_vector[i].concentration)
         {
            cc_conc->update((*ms).component_vector[i].concentration, (float) 0.2, &low, &high, 0.0);
            msc.component_vector_constraints[i].concentration.low = low;
            msc.component_vector_constraints[i].concentration.high = high;
            message += tr(str.sprintf("The constraints for the concentration of component %d\n", i+1));
            flag = false;
         }
      }
   }
   for (i=0; i<(*ms).assoc_vector.size(); i++)
   {
      if (msc.assoc_vector_constraints[i].keq.fit)
      {
         if ( msc.assoc_vector_constraints[i].keq.low >
              (*ms).assoc_vector[i].keq
              ||   msc.assoc_vector_constraints[i].keq.high <
              (*ms).assoc_vector[i].keq)
         {
            cc_keq->update((*ms).assoc_vector[i].keq, (float) 0.9, &low, &high, 0.0);
            msc.assoc_vector_constraints[i].keq.low = low;
            msc.assoc_vector_constraints[i].keq.high = high;
            message += tr(str.sprintf("The constraints for the equilibrium constant of reaction %d\n", i+1));
            flag = false;
         }
         if ( msc.assoc_vector_constraints[i].koff.low >
              (*ms).assoc_vector[i].k_off
              ||   msc.assoc_vector_constraints[i].koff.high <
              (*ms).assoc_vector[i].k_off)
         {
            cc_koff->update((*ms).assoc_vector[i].k_off, (float) 0.99, &low, &high, 0.0);
            msc.assoc_vector_constraints[i].koff.low = low;
            msc.assoc_vector_constraints[i].koff.high = high;
            message += tr(str.sprintf("The constraints for the rate constant of reaction %d\n", i+1));
            flag = false;
         }
      }
   }
   if (!flag)
   {
      message += tr("\nPlease check these constraints and adjust if necessary...");
      QMessageBox::warning(this, tr("UltraScan Warning"), message,
                           QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
   }
   current_component = 0; // reset to the first component
   current_assoc = 0;
   select_component((int) current_component);
   return flag;
}

void US_GAModelEditor::save_constraints()
{
   if (!verify_constraints())
   {
      return;
   }
   QString str, fn = QFileDialog::getSaveFileName(USglobal->config_list.result_dir, "*.constraints", 0);
   if ( !fn.isEmpty() )
   {
      US_FemGlobal *fg;
      fg = new US_FemGlobal(this);
      fg->write_constraints(ms, &msc, fn);
      delete fg;
   }
}

void US_GAModelEditor::mw_constraintChanged(struct constraint c)
{
   msc.component_vector_constraints[current_component].mw.low = c.low;
   msc.component_vector_constraints[current_component].mw.high = c.high;
   msc.component_vector_constraints[current_component].mw.fit = c.fit;
   //cout << "Set constraintChanged for comp. " << current_component << ", mw to " << c.low << ", " << c.high << ", " << c.fit << endl;
}

void US_GAModelEditor::f_f0_constraintChanged(struct constraint c)
{
   if (c.low < 1.0)
   {
      cc_f_f0->le_low->setText("1.000");
      return;
   }
   msc.component_vector_constraints[current_component].f_f0.low = c.low;
   msc.component_vector_constraints[current_component].f_f0.high = c.high;
   msc.component_vector_constraints[current_component].f_f0.fit = c.fit;
   //cout << "Set constraintChanged for comp. " << current_component << ", f_f0 to " << c.low << ", " << c.high << ", " << c.fit << endl;
}

void US_GAModelEditor::conc_constraintChanged(struct constraint c)
{
   msc.component_vector_constraints[current_component].concentration.low = c.low;
   msc.component_vector_constraints[current_component].concentration.high = c.high;
   msc.component_vector_constraints[current_component].concentration.fit = c.fit;
   //cout << "Set constraintChanged for comp. " << current_component << ", conc to " << c.low << ", " << c.high << ", " << c.fit << endl;
}

void US_GAModelEditor::keq_constraintChanged(struct constraint c)
{
   if (msc.assoc_vector_constraints.size() > 0)
   {
      msc.assoc_vector_constraints[current_assoc].keq.low = c.low;
      msc.assoc_vector_constraints[current_assoc].keq.high = c.high;
      msc.assoc_vector_constraints[current_assoc].keq.fit = c.fit;
      //cout << "Set constraintChanged for comp. " << current_component << ", keq to " << c.low << ", " << c.high << ", " << c.fit << endl;
   }
}

void US_GAModelEditor::koff_constraintChanged(struct constraint c)
{
   if (msc.assoc_vector_constraints.size() > 0)
   {
      msc.assoc_vector_constraints[current_assoc].koff.low = c.low;
      msc.assoc_vector_constraints[current_assoc].koff.high = c.high;
      msc.assoc_vector_constraints[current_assoc].koff.fit = c.fit;
      //cout << "Set constraintChanged for comp. " << current_component << ", koff to " << c.low << ", " << c.high << ", " << c.fit << endl;
   }
}

// this slot is triggered by signal component_changed() in us_model_editor
void US_GAModelEditor::update_constraints(unsigned int c)
{
   //cout << "updating constraints for component " << c << endl;
   current_component = c;
   QString str;
   le_f_f0->setReadOnly(false);
   cc_f_f0->update(msc.component_vector_constraints[current_component].f_f0);
   if ((*ms).component_vector[current_component].show_conc)
   {
      le_mw->setEnabled(true);
      pb_simulateComponent->setEnabled(true);
      le_mw->setReadOnly(false);
      cc_mw->update(msc.component_vector_constraints[current_component].mw);
      cc_conc->update(msc.component_vector_constraints[current_component].concentration);
   }
   else
   {
      le_mw->setReadOnly(true);
      le_mw->setEnabled(false);
      pb_simulateComponent->setEnabled(false);
      cc_mw->clear();
      cc_conc->clear();
   }
   if ((*ms).component_vector[current_component].show_keq)
   {
      for (unsigned int i=0; i<(*ms).assoc_vector.size(); i++)
      { // only check the dissociating species to set keq/koff
         if ((*ms).assoc_vector[i].component2 == current_component
             ||  (*ms).assoc_vector[i].component3 == (int) current_component)
         {
            cc_koff->update(msc.assoc_vector_constraints[current_assoc].koff);
            cc_keq->update(msc.assoc_vector_constraints[current_assoc].keq);
         }
      }
   }
   else
   {
      cc_keq->clear();
      cc_koff->clear();
   }
   /*
     for (unsigned int i=0; i<(*ms).assoc_vector.size(); i++)
     { // only check the associating species to set mw
     if ((*ms).assoc_vector[i].component1 == (int) current_component)
     {
     cc_mw->update(msc.component_vector_constraints[current_component].mw);
     }
     else
     {
     cc_mw->clear();
     }
     }
   */
}

void US_GAModelEditor::select_model()
{
   US_ModelSelection *model_sel;
   model_sel = new US_ModelSelection();
   model_sel->select_model(ms);
   delete model_sel;
   if ((*ms).model < 0)
   {
      return;
   }
   lbl_model->setText(modelString[(*ms).model]);
   cmb_component1->clear();
   for (unsigned int i=0; i<(*ms).component_vector.size(); i++)
   {
      cmb_component1->insertItem((*ms).component_vector[i].name);
   }
   cnt_item->setRange(1, (*ms).component_vector.size(), 1);
   // after selecting a model we need to allocate memory for msc in initialize_msc
   current_component = 0;
   current_assoc = 0;
   initialize_msc();
}

void US_GAModelEditor::initialize_msc()
{
   unsigned int i;
   msc.component_vector_constraints.resize((*ms).component_vector.size());
   msc.assoc_vector_constraints.resize((*ms).assoc_vector.size());
   for (i=0; i<(*ms).component_vector.size(); i++)
   {
      current_component = i;
      msc.component_vector_constraints[i].f_f0.fit = false;
      le_f_f0->setReadOnly(false);
      cc_f_f0->setDefault((*ms).component_vector[i].f_f0 - 1.0, 0.5, 1.0);
      if ((*ms).component_vector[i].show_conc)
      {
         msc.component_vector_constraints[i].concentration.fit = false;
         msc.component_vector_constraints[i].mw.fit = false;
         cc_conc->setDefault((*ms).component_vector[i].concentration, (float) 0.2);
         cc_mw->setDefault((*ms).component_vector[i].mw, (float) 0.2);
         //cout << (*ms).component_vector[i].mw << ", " <<msc.component_vector_constraints[i].mw.low << "," << msc.component_vector_constraints[i].mw.high << endl;
      }
   }
   for (i=0; i<(*ms).assoc_vector.size(); i++)
   {
      current_assoc = i;
      msc.assoc_vector_constraints[i].keq.fit = false;
      msc.assoc_vector_constraints[i].koff.fit = false;
      cc_keq->setDefault((*ms).assoc_vector[i].keq, (float) 0.9);
      cc_koff->setDefault((*ms).assoc_vector[i].k_off, (float) 0.99);
   }
   current_component = 0; // reset to the first component
   current_assoc = 0;
   select_component((int) current_component);
}

