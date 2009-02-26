//! \file us_selectmodel.cpp

#include "us_selectmodel.h"
#include "us_gui_settings.h"
#include "us_constants.h"

using namespace US_Constants;

US_SelectModel::US_SelectModel( int& selection, bool show_equation ) 
   : US_WidgetsDialog( 0, 0 ), modelSelected( selection )
{
   setWindowTitle( tr( "Model Selection - UltraScan Analysis" ) );
   setPalette( US_GuiSettings::frameColor() );
   setAttribute( Qt::WA_DeleteOnClose );
   
   show = show_equation;
   
   QBoxLayout* main = new QVBoxLayout( this );
   main->setSpacing         ( 2 );
   main->setContentsMargins ( 2, 2, 2, 2 );

   QLabel* lb_info = us_banner( tr( "Please select a Model" ) );
   main->addWidget( lb_info );

   lw_models = us_listwidget();

   models = US_Constants::modelStrings();

   for ( int i = 0; i < models.size(); i++ )
      lw_models->addItem( models[ i ] );

   QListWidgetItem* item = lw_models->item( 0 );
   item->setSelected( true ) ;

   main->addWidget( lw_models );

   QPushButton* pb_help = us_pushbutton( tr( "Help" ) );
   connect( pb_help, SIGNAL( clicked() ), SLOT( help() ) );

   QPushButton* pb_cancel = us_pushbutton( tr( "Cancel" ) );
   connect( pb_cancel, SIGNAL( clicked() ), SLOT( cancel() ) );

   QPushButton* pb_select = us_pushbutton( tr( "Select Model" ) );
   connect( pb_select, SIGNAL( clicked() ), SLOT( select_model() ) );


   QBoxLayout* buttons = new QHBoxLayout;
   buttons->addWidget( pb_help   );
   buttons->addWidget( pb_cancel );
   buttons->addWidget( pb_select );

   main->addLayout( buttons );
}

void US_SelectModel::cancel()
{
   modelSelected = -1;
   done( 0 ); // Close and retrun 0/false
}

void US_SelectModel::select_model()
{
   select_model( lw_models->currentRow() ); 
}

void US_SelectModel::help()
{
   showhelp.show_help( "manual/equil2.html" );
}

void US_SelectModel::select_model( int item )
{
   modelSelected = item;
   
   if ( ! show )
   {
      accept();
      return;
   }

   switch( modelSelected )
   {
      case Ideal1Comp:  // 1-Component, Ideal
         QMessageBox::information( this, models[ Ideal1Comp ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n" 

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
                "* indicates that this parameter can be floated" ) );
         break;

      case Ideal2Comp: // 2-Component, Ideal, Noninteracting
         QMessageBox::information( this, models[  Ideal2Comp ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"  
                
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
                "* indicates that this parameter can be floated" ) );

         break;

      case Ideal3Comp: // 3-Component, Ideal, Noninteracting
         QMessageBox::information( this, models[ Ideal3Comp ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(A[3]) + M[3] * omega^2 * (1 - vbar[3] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n" 

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
                "* indicates that this parameter can be floated" ) );
         break;

      case Fixed: // Fixed Molecular Weight Distribution
         QMessageBox::information( this, models[ Fixed ],
            tr( "Fitting to Function:\n\n"
                "C (X) = A[i] * SUM [ (exp [(M[i] * omega^2 * (1 - vbar[i] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] ] + B\n\n" 

                "where:\n\n"
                "         X    = Radius\n"
                "         Xr   = Reference Radius\n"
                "         A[i] = Amplitude of Component \"i\"*\n"
                "         M[i] = Molecular Weight of Component \"i\"\n"
                "         D    = Density\n"
                "         R    = Gas Constant\n"
                "         T    = Temperature\n"
                "         B    = Baseline *\n\n"
                "* indicates that this parameter can be floated\n\n" ) );
         break;
      
      case MonoDimer: // Monomer-Dimer Equilibrium
         QMessageBox::information( this, models[ MonoDimer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M "
                "* omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoTrimer: // Monomer-Trimer Equilibrium
         QMessageBox::information( this, models[ MonoTrimer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (3 * ln(A) + ln(3/(e * l)^2) + ln(K1,3) + 3 * M "
                "* omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] "
                "+ B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoTetramer: // Monomer-Tetramer Equilibrium
         QMessageBox::information( this, models[ MonoTetramer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (4 * ln(A) + ln(4/(e * l)^3) + ln(K1,4) + 4 * "
                "M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoPentamer: // Monomer-Pentamer Equilibrium
         QMessageBox::information( this, models[ MonoPentamer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (5 * ln(A) + ln(5/(e * l)^4) + ln(K1,5) + 5 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] "
                "+ B\n\n"

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoHexamer: // Monomer-Hexamer Equilibrium
         QMessageBox::information( this, models[ MonoHexamer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (6 * ln(A) + ln(6/(e * l)^5) + ln(K1,6) + 6 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] "
                "+ B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoHeptamer: // Monomer-Heptamer Equilibrium
         QMessageBox::information( this, models[ MonoHeptamer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (7 * ln(A) + ln(7/(e * l)^6) + ln(K1,7) + 7 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] "
                "+ B\n\n"

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
                "Model Control Window.\n" ) );
         break;

      case UserMonoNmer: // User-Defined Monomer-Nmer Equilibrium
         QMessageBox::information( this, models[ UserMonoNmer],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * "
                "M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoDiTrimer: // Monomer-Dimer-Trimer Equilibrium
         QMessageBox::information( this, models[ MonoDiTrimer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (3 * ln(A) + ln(3/(e * l)^2) + ln(K1,3) + 3 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / (2 * R * T) ] "
                "+ B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      case MonoDiTetramer: // Monomer-Dimer-Tetramer Equilibrium
         QMessageBox::information( this, models[ MonoDiTetramer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (2 * ln(A) + ln(2/(e * l)) + ln(K1,2) + 2 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (4 * ln(A) + ln(4/(e * l)^3) + ln(K1,4) + 4 * M * "
                "omega^2 * (1 - vbar * D) * (X^2 - Xr^2)) / (2 * R * T) ] "
                "+ B\n\n"

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
                "Model Control Window.\n" ) );
         break;
      
      case UserMonoNmerNmer: // Monomer - N-mer - M-mer Equilibrium
         QMessageBox::information( this, models[ UserMonoNmerNmer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N1 * ln(A) + ln(N1/(e * l)^(N1 - 1)) + ln(K1,N1) + "
                "N1 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ]\n"
                
                "+ exp [ (N2 * ln(A) + ln(N2/(e * l)^(N2 - 1)) + ln(K1,N2) + "
                "N2 * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n"

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
                "Model Control Window.\n" ) );
         break;
      
      case TwoComponent: // 2-Component Hetero-Associating Equilibrium
         QMessageBox::information( this, models[ TwoComponent ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M_A * omega^2 * (1 - vbar_A * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(B) + M_B * omega^2 * (1 - vbar_B * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(A) + ln(B) + ln(K_AB) + ln (e_AB/(e_A * e_B * l)) "
                "+ (M_A + M_B) * omega^2\n"
                
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
                "Model Control Window.\n" ) );
         break;
      
      // 2-Component Hetero-Associating Equilibrium with self-association
      case UserHetero: 
         QMessageBox::information( this, models[ UserHetero ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A) + M_A * omega^2 * (1 - vbar_A * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(B) + M_B * omega^2 * (1 - vbar_B * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (ln(A) + ln(B) + ln(K_AB) + ln (e_AB/(e_A * e_B * l)) "
                "+ (M_A + M_B) * omega^2\n"
                
                "* (1 - vbar_AB * D) * (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * "
                "M_A * omega^2 * (1 - vbar_A * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n\n"
                "***vbar_AB = (vbar_A * M_A + vbar_B * M_B)/(M_A + M_B)" ) );
         break;
      
      // User-Defined Monomer-Nmer, some monomer is incompetent
      case UserMonoIncompMono: 
         QMessageBox::information( this, models[ UserMonoIncompMono ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + N * "
                "M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ] + B\n\n"
                
                "+ exp [ (ln(A2) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
      
      // User-Defined Monomer-Nmer, some N-mer is incompetent
      case UserMonoIncompNmer: 
         QMessageBox::information( this, models[ UserMonoIncompNmer ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + "
                "N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ]\n"
                
                "+ exp [ (ln(A2) + N * M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                "where:\n\n" 

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
                "Model Control Window.\n" ) );
         break;

      // User-Defined Monomer-Nmer, some monomer and some N-mer are incompetent
      case UserIrreversible: 
         QMessageBox::information( this, models[ UserIrreversible ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A1) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A1) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + "
                "N * M * omega^2 * (1 - vbar * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ]\n"
                
                "+ exp [ (ln(A2) + M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n"
                
                "+ exp [ (ln(A3) + N * M * omega^2 * (1 - vbar * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;

      // User-Defined Monomer-Nmer, some monomer and some N-mer are incompetent
      case UserMonoPlusContaminant: 
         QMessageBox::information( this, models[ UserMonoPlusContaminant ],
            tr( "Fitting to Function:\n\n"
                "C (X) = exp [ (ln(A[1]) + M[1] * omega^2 * (1 - vbar[1] * D) *"
                " (X^2 - Xr ^2)) / (2 * R * T) ]\n"
                
                "+ exp [ (N * ln(A[1]) + ln(N/(e * l)^(N - 1)) + ln(K1,N) + "
                "N * M * omega^2 * (1 - vbar[1] * D) * (X^2 - Xr ^2)) / "
                "(2 * R * T) ]\n"
                
                "+ exp [ (ln(A[2]) + M[2] * omega^2 * (1 - vbar[2] * D) * "
                "(X^2 - Xr ^2)) / (2 * R * T) ] + B\n\n" 

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
                "Model Control Window.\n" ) );
         break;
   }
   
   accept();
}

