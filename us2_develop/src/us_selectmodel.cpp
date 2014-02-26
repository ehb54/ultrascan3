#include "../include/us_selectmodel.h"

US_SelectModel::US_SelectModel(int *temp_model_selected, bool temp_show_equation, 
                               QWidget *p, const char *name) : QDialog( p, name, true)
{
   buttonh = 26;
   border = 4;
   int width = 300;
   xpos = border;
   ypos = border;
   show_equation = temp_show_equation;   // false for velocity
   USglobal = new US_Config();
   
   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("Model Selection - UltraScan Analysis"));

   model_selected = temp_model_selected;
   *model_selected = 0;
   
   lbl_info = new QLabel(tr("Please select a Model:"), this);
   Q_CHECK_PTR(lbl_info);
   lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_info->setAlignment(AlignCenter|AlignVCenter);
   lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_info->setMinimumSize(width, 2*buttonh);
   lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
   
   ypos += buttonh + spacing;
   
   lb_model = new QListBox(this, "Models");
   lb_model->setSelected(0, true);
   lb_model->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   lb_model->setMinimumSize(width, 6*buttonh);
   lb_model->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   for (unsigned int i=0; i<modelString.size(); i++)
   {
      lb_model->insertItem(modelString[i]);
   }

   lb_model->setSelected(0, true);
   lb_model->setCurrentItem(0);
   connect(lb_model, SIGNAL(selected(int)), SLOT(select_model(int)));
   
   ypos += 4 * buttonh + spacing;
   xpos = border;

   pb_select = new QPushButton(tr("Select Model"), this);
   Q_CHECK_PTR(pb_select);
   pb_select->setAutoDefault(false);
   pb_select->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_select->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_select, SIGNAL(clicked()), SLOT(select_model()));

   xpos += 101;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += 100;

   pb_cancel = new QPushButton(tr("Cancel"), this);
   Q_CHECK_PTR(pb_cancel);
   pb_cancel->setAutoDefault(false);
   pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

   ypos += buttonh + border;

   global_Xpos += 30;
   global_Ypos += 30;
   
   setMinimumSize(width+8, ypos);
   setGeometry(global_Xpos, global_Ypos, width+8, ypos);
   
   setup_GUI();
}

US_SelectModel::~US_SelectModel()
{
}

void US_SelectModel::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_SelectModel::setup_GUI()
{
   int spacing = 2, columns = 3, rows = 1;
   
   QBoxLayout *topbox = new QVBoxLayout(this, spacing);
   topbox->addWidget(lbl_info);
   topbox->addWidget(lb_model);
   QGridLayout *buttonGrid = new QGridLayout(topbox, rows, columns, spacing);
   buttonGrid->addWidget(pb_select, 0, 0);
   buttonGrid->addWidget(pb_help, 0, 1);
   buttonGrid->addWidget(pb_cancel, 0, 2);
}

void US_SelectModel::cancel()
{
   *model_selected = -1;
   reject();
}

void US_SelectModel::select_model()
{
   int item;
   item = lb_model->currentItem();
   select_model(item); 
}

void US_SelectModel::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/equil2.html");
}

void US_SelectModel::select_model(int item)
{
   *model_selected = item;
   if (!show_equation)
   {
      accept();
      return;
   }
   switch(*model_selected)
   {
   case 0:  // 1-Component, Ideal
      {
         QMessageBox::message(modelString[0],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude *\n"
                                 "         M  = Molecular Weight *\n"
                                 "         E  = Extinction Coefficient\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated") );
         break;
      }
   case 1: // 2-Component, Ideal, Noninteracting
      {
         QMessageBox::message(modelString[1],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude *\n"
                                 "         M  = Molecular Weight *\n"
                                 "         E  = Extinction Coefficient\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated" ));
         break;
      }
   case 2: // 3-Component, Ideal, Noninteracting
      {
         QMessageBox::message(modelString[2],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A[3]) + M[3] * omega^2 * (1 - vbar[3] * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude *\n"
                                 "         M  = Molecular Weight *\n"
                                 "         E  = Extinction Coefficient\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated" ));
         break;
      }
   case 3: // Fixed Molecular Weight Distribution
      {
         QMessageBox::message(modelString[3],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = A[i] * SUM [ (exp [(M[i] * omega^2 * (1 - vbar[i] * D) * (X^2 - Xr ^2)) / (2 * R * T) ] ] + B\n\n"
                                 "where:\n\n"
                                 "         X    = Radius\n"
                                 "         Xr   = Reference Radius\n"
                                 "         A[i] = Amplitude of Component \"i\"*\n"
                                 "         M[i] = Molecular Weight of Component \"i\"\n"
                                 "         D    = Density\n"
                                 "         R    = Gas Constant\n"
                                 "         T    = Temperature\n"
                                 "         B    = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"));
         break;
      }
   case 4: // Monomer-Dimer Equilibrium
      {
         QMessageBox::message(modelString[4],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         e  = Extinction Coefficient\n"
                                 "         l  = Pathlength\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 5: // Monomer-Trimer Equilibrium
      {
         QMessageBox::message(modelString[5],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (3 * ln(A) + ln(3/(e * l)^2) + ln(K1,3) + 3 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         e  = Extinction Coefficient\n"
                                 "         l  = Pathlength\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 6: // Monomer-Tetramer Equilibrium
      {
         QMessageBox::message(modelString[6],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (4 * ln(A) + ln(4/(e * l)^3) + ln(K1,4) + 4 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 7: // Monomer-Pentamer Equilibrium
      {
         QMessageBox::message(modelString[7],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (5 * ln(A) + ln(5/(e * l)^4) + ln(K1,5) + 5 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 8: // Monomer-Hexamer Equilibrium
      {
         QMessageBox::message(modelString[8],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (6 * ln(A) + ln(6/(e * l)^5) + ln(K1,6) + 6 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 9: // Monomer-Heptamer Equilibrium
      {
         QMessageBox::message(modelString[9],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (7 * ln(A) + ln(7/(e * l)^6) + ln(K1,7) + 7 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 10: // User-Defined Monomer-Nmer Equilibrium
      {
         QMessageBox::message(modelString[10],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         N  = User-Defined Association State\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 11: // Monomer-Dimer-Trimer Equilibrium
      {
         QMessageBox::message(modelString[11],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (3 * ln(A) + ln(3/(e * l)^2) + ln(K1,3) + 3 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 12: // Monomer-Dimer-Tetramer Equilibrium
      {
         QMessageBox::message(modelString[12],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (4 * ln(A) + ln(4/(e * l)^3) + ln(K1,4) + 4 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 13: // Monomer - N-mer - M-mer Equilibrium
      {
         QMessageBox::message(modelString[13],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N1 * ln(A) + ln(N1/(e * l)^(N1 - 1)) + ln(K1,N1) + N1 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N2 * ln(A) + ln(N2/(e * l)^(N2 - 1)) + ln(K1,N2) + N2 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         N1 = Stoichiometry of first Association\n"
                                 "         N2 = Stoichiometry of second Association\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 14: // 2-Component Hetero-Associating Equilibrium
      {
         QMessageBox::message(modelString[14],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M_A * omega^2 * (1 - vbar_A * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(B) + M_B * omega^2 * (1 - vbar_B * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A) + ln(B) + ln(K_AB) + ln (e_AB/(e_A * e_B * l)) + (M_A + M_B) * omega^2\n"
                                 "* (1 - vbar_AB * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of component A*\n"
                                 "         B  = Amplitude of component B*\n"
                                 "         K_AB = AB Equilibrium Constant*,**\n"
                                 "         M_A  = Molecular Weight of A*\n"
                                 "         M_B  = Molecular Weight of B*\n"
                                 "         M_AB  = Molecular Weight of AB\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 15: // 2-Component Hetero-Associating Equilibrium with self-association
      {
         QMessageBox::message(modelString[15],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A) + M_A * omega^2 * (1 - vbar_A * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(B) + M_B * omega^2 * (1 - vbar_B * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A) + ln(B) + ln(K_AB) + ln (e_AB/(e_A * e_B * l)) + (M_A + M_B) * omega^2\n"
                                 "* (1 - vbar_AB * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M_A * omega^2 * (1 - vbar_A * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of component A*\n"
                                 "         B  = Amplitude of component B*\n"
                                 "         K_AB = AB Equilibrium Constant*,**\n"
                                 "         K_AN = A monomer-nmer Equilibrium Constant*,**\n"
                                 "         M_A  = Molecular Weight of A*\n"
                                 "         M_B  = Molecular Weight of B*\n"
                                 "         M_AB  = Molecular Weight of AB\n"
                                 "         vbar_A  = vbar of A*\n"
                                 "         vbar_B  = vbar of B*\n"
                                 "         vbar_AB = vbar of AB***\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"
                                 "\n***vbar_AB = (vbar_A * M_A + vbar_B * M_B)/(M_A + M_B)"));
         break;
      }
   case 16: // User-Defined Monomer-Nmer, some monomer is incompetent
      {
         QMessageBox::message(modelString[16],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "+ exp [ (ln(A2) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         N  = User-Defined Association State\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         A1 = Reference Concentration of monomer*\n"
                                 "         A2 = Reference Concentration of incompetent monomer*\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 17: // User-Defined Monomer-Nmer, some N-mer is incompetent
      {
         QMessageBox::message(modelString[17],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A2) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         N  = User-Defined Association State\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         A1 = Reference Concentration of monomer*\n"
                                 "         A2 = Reference Concentration of incompetent N-mer*\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 18: // User-Defined Monomer-Nmer, some monomer and some N-mer are incompetent
      {
         QMessageBox::message(modelString[18],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A2) + M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "+ exp [ (ln(A3) + N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         N  = User-Defined Association State\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M  = Monomer Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         A1 = Reference Concentration of monomer*\n"
                                 "         A2 = Reference Concentration of incompetent monomer*\n"
                                 "         A3 = Reference Concentration of incompetent N-mer*\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case 19: // User-Defined Monomer-Nmer, some monomer and some N-mer are incompetent
      {
         QMessageBox::message(modelString[19],
                              tr("Fitting to Function:\n\n"
                                 "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (N * ln(A[1]) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * M * omega^2 * (1 - vbar[1] * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                                 "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * (X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                                 "where:\n\n"
                                 "         N  = User-Defined Association State\n"
                                 "         X  = Radius\n"
                                 "         Xr = Reference Radius\n"
                                 "         A  = Amplitude of Monomer*\n"
                                 "         K1 = Monomer - Dimer Equilibrium Constant*,**\n"
                                 "         M1 = Monomer Molecular Weight *\n"
                                 "         M2 = Contaminant Molecular Weight *\n"
                                 "         D  = Density\n"
                                 "         R  = Gas Constant\n"
                                 "         T  = Temperature\n"
                                 "         A1 = Reference Concentration of monomer*\n"
                                 "         A2 = Reference Concentration of contaminant*\n"
                                 "         B  = Baseline *\n\n"
                                 "* indicates that this parameter can be floated\n\n"
                                 "** Equilibrium Constants are calculated in log of molar\n"
                                 "units, conversion to concentration is performed in the\n"
                                 "Model Control Window.\n"));
         break;
      }
   case -1:
      {
         //         lbl_model2->setText(" - none selected - ");
         break;
      }
   }
   accept();
}
