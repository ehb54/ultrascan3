//! \f us_model_selection.cpp
#include "us_model_selection.h"
#include "us_selectmodel.h"
#include "us_selectmodel3.h"
#include "us_selectmodel10.h"
#include "us_selectmodel13.h"
#include "us_constants.h"

// Select a model from the predefined list and initialize the model variables
// with default values

using namespace US_ConstantModels;

void US_ModelSelection::selectModel( struct ModelSystem& ms )
{
   US_SelectModel* select = new US_SelectModel( ms.model );
  
   ms.description = "Default ASTFEM Model";
   ms.component_vector.clear();
   ms.assoc_vector.clear();

   bool  model3_vbarflag = true; 
   float mw_upperLimit   = 1.0e5;
   float mw_lowerLimit   = 1.0e4;
   float model3_vbar     = (float) 0.72;
   
   ms.component_vector.clear();
   struct SimulationComponent sc;

   struct SimulationComponent* cv0;
   struct SimulationComponent* cv1;
   struct SimulationComponent* cv2;
   struct SimulationComponent* cv3;

   struct Association  assoc;
   struct Association* av0;
   struct Association* av1;

   if ( select->exec() )
   {
      switch ( ms.model )
      {
         case Ideal1Comp:
            ms.component_vector << sc;
            
            cv0 = &ms.component_vector[ 0 ];
            
            cv0->name = "Component 1";
            break;

         case Ideal2Comp:
            ms.component_vector << sc << sc;
            
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->name = "Component 1";
            cv1->name = "Component 2";
            break;

         case Ideal3Comp:
            ms.component_vector << sc << sc << sc;
            
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            cv2 = &ms.component_vector[ 2 ];
            
            cv0->name = "Component 1";
            cv1->name = "Component 2";
            cv2->name = "Component 3";
            break;

         // Fixed molecular weight distribution, we need some more 
         // info from the user
         case Fixed:
            {
               int species;
               
               US_SelectModel3* SelectModel3 
                  = new US_SelectModel3( mw_upperLimit, 
                                         mw_lowerLimit, 
                                         species, 
                                         model3_vbar, 
                                         model3_vbarflag, -1 );
               
               if ( ! SelectModel3->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  for ( int i = 0; i < species; i++ )
                  {
                     ms.component_vector << sc;
                     struct SimulationComponent* cv = &ms.component_vector[ i ];

                     cv->name = "Component " + QString::number( i + 1 );
                  }
               }

               break;
            }
         case MonoDimer: // Monomer-Dimer Equilibrium

            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv1->name  = "Dimer";
            cv1->show_component.clear();
            
            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 2;
            av0->stoichiometry3 = 0;

            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;

         case MonoTrimer: // Monomer-Trimer Equilibrium
         
            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();
            
            cv1->shape = "not defined";
            cv1->name  = "Trimer";
            cv1->show_component.clear();
            
            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 3;
            av0->stoichiometry3 = 0;
            
            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;
         
         case MonoTetramer: // Monomer-Tetramer Equilibrium
         
            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();
            
            cv1->shape = "not defined";
            cv1->name  = "Tetramer";
            cv1->show_component.clear();
            
            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 4;
            av0->stoichiometry3 = 0;
            
            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;
         
         case MonoPentamer: // Monomer-Pentamer Equilibrium
         
            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv1->name  = "Pentamer";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv0->name  = "Monomer";
            cv1->show_component.clear();

            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 5;
            av0->stoichiometry3 = 0;
            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;
         
         case MonoHexamer: // Monomer-Hexamer Equilibrium
         
            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv1->name  = "Hexamer";
            cv1->show_component.clear();

            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 6;
            av0->stoichiometry3 = 0;

            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;

         case MonoHeptamer: // Monomer-Heptamer Equilibrium
         
            ms.component_vector << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();
            
            cv1->shape = "not defined";
            cv1->name  = "Heptamer";
            cv1->show_component.clear();
     
            ms.assoc_vector << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            
            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 2;
            av0->stoichiometry3 = 0;

            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );
            break;
         
         // User-Defined Monomer - N-mer Equilibrium, 
         // we need some additional info from the user
         case UserMonoNmer:
            {
               float stoich = 2;
               
               US_SelectModel10* SelectModel10 = new US_SelectModel10( stoich );
               
               if ( ! SelectModel10->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
               
                  cv0->shape = "not defined";
                  cv0->name  = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "N-mer (" + QString::number( (int) stoich ) + ")";
                  cv1->show_component.clear();

                  ms.assoc_vector << assoc;
                  av0 = &ms.assoc_vector[ 0 ];
               
                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich;
                  av0->stoichiometry3 = 0;

                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2 );
               }
            }
            break;

         case MonoDiTrimer: // Monomer-Dimer-Trimer Equilibrium
         
            ms.component_vector << sc << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            cv2 = &ms.component_vector[ 2 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv1->name = "Dimer";
            cv1->show_component.clear();

            cv2->shape = "not defined";
            cv2->name = "Trimer";
            cv2->show_component.clear();
            
            ms.assoc_vector << assoc << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            av1 = &ms.assoc_vector[ 1 ];

            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 2;
            av0->stoichiometry3 = 0;

            av1->keq            = 1.0;
            av1->k_off          = (float) 1.0e-4;
            av1->units          = "OD";
            av1->component1     = 0;
            av1->component2     = 2;
            av1->component3     = -1;
            av1->stoichiometry1 = 1;
            av1->stoichiometry2 = 3;
            av1->stoichiometry3 = 0;
            
            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2);
            
            initAssoc1( ms, av1->component1,
                            av1->component2,
                            av1->stoichiometry2);
            break;

         case MonoDiTetramer: // Monomer-Dimer-Tetramer Equilibrium
         
            ms.component_vector << sc << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            cv2 = &ms.component_vector[ 2 ];
            
            cv0->shape = "not defined";
            cv0->name  = "Monomer";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv1->name  = "Dimer";
            cv1->show_component.clear();

            cv2->shape = "not defined";
            cv2->name  = "Tetramer";
            cv2->show_component.clear();
            
            ms.assoc_vector << assoc << assoc;
            av0 = &ms.assoc_vector[ 0 ];
            av1 = &ms.assoc_vector[ 1 ];

            av0->keq            = 1.0;
            av0->k_off          = (float) 1.0e-4;
            av0->units          = "OD";
            av0->component1     = 0;
            av0->component2     = 1;
            av0->component3     = -1;
            av0->stoichiometry1 = 1;
            av0->stoichiometry2 = 2;
            av0->stoichiometry3 = 0;

            av1->keq            = 1.0;
            av1->k_off          = (float) 1.0e-4;
            av1->units          = "OD";
            av1->component1     = 0;
            av1->component2     = 2;
            av1->component3     = -1;
            av1->stoichiometry1 = 1;
            av1->stoichiometry2 = 4;
            av1->stoichiometry3 = 0;

            initAssoc1( ms, av0->component1,
                            av0->component2,
                            av0->stoichiometry2 );

            initAssoc1( ms, av1->component1,
                            av1->component2,
                            av1->stoichiometry2 );
            break;
               
         // User-Defined Monomer - N-mer - M-mer Equilibrium, we need some
         // additional info from the user
         
         case UserMonoNmerNmer:
            {         
               float stoich1 = 1;
               float stoich2 = 2;
               
               US_SelectModel13* SelectModel13 
                  = new US_SelectModel13( stoich1, stoich2);
               
               if ( ! SelectModel13->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
                  cv2 = &ms.component_vector[ 2 ];
               
                  cv0->shape = "not defined";
                  cv0->name  = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "N-mer (" + QString::number( (int) stoich1 ) + " )";
                  cv1->show_component.clear();

                  cv2->shape = "not defined";
                  cv2->name  = "N-mer (" + QString::number( (int) stoich2 ) + " )";
                  cv2->show_component.clear();

                  ms.assoc_vector << assoc << assoc;
                  av0 = &ms.assoc_vector[ 0 ];
                  av1 = &ms.assoc_vector[ 1 ];

                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich1;
                  av0->stoichiometry3 = 0;
                  
                  av1->keq            = 1.0;
                  av1->k_off          = (float) 1.0e-4;
                  av1->units          = "OD";
                  av1->component1     = 0;
                  av1->component2     = 2;
                  av1->component3     = -1;
                  av1->stoichiometry1 = 1;
                  av1->stoichiometry2 = (int) stoich2;
                  av1->stoichiometry3 = 0;

                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2);

                  initAssoc1( ms, av1->component1,
                                  av1->component2,
                                  av1->stoichiometry2);
               }
            }
            break;
         
         case TwoComponent: // 2 component hetero association.
         
            ms.component_vector << sc << sc << sc;
            cv0 = &ms.component_vector[ 0 ];
            cv1 = &ms.component_vector[ 1 ];
            cv2 = &ms.component_vector[ 2 ];
               
            cv0->shape = "not defined";
            cv0->name  = "Component A";
            cv0->show_component.clear();

            cv1->shape = "not defined";
            cv1->name  = "Component B";
            cv1->show_component.clear();

            cv2->shape = "not defined";
            cv2->name  = "Component AB";
            cv2->show_component.clear();
            
            ms.assoc_vector << assoc ;
            av0 = &ms.assoc_vector[ 0 ];

            av0->keq        = 1.0;
            av0->k_off      = (float) 1.0e-4;
            av0->units      = "OD";
            av0->component1 = 0;
            av0->component2 = 1;
            av0->component3 = 2;

            // For hetero-associating ms's the stoichiometry are zero
            av0->stoichiometry1 = 1; 
            av0->stoichiometry2 = 1;
            av0->stoichiometry3 = 1;
            
            initAssoc2( ms );

            cv0->show_conc   = true;
            cv0->show_stoich = 0;
            cv0->show_keq    = false;
            cv0->show_koff   = false;
            cv0->show_component << 1 << 2;

            cv1->show_conc   = true;
            cv1->show_stoich = 0;
            cv1->show_keq    = false;
            cv1->show_koff   = false;
            cv1->show_component << 0 << 2;

            cv2->show_conc   = false;
            cv2->show_stoich = -1; // -1 means hetero - association
            cv2->show_keq    = true;
            cv2->show_koff   = true;
            cv1->show_component << 0 << 1;
            
            break;
         
         // 2 component hetero association, one of the components 
         // self-associates.
         
         case UserHetero: 
            {         
               float stoich = 2;

               US_SelectModel10* SelectModel10 = new US_SelectModel10( stoich );

               if ( ! SelectModel10->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
                  cv2 = &ms.component_vector[ 2 ];
                  cv3 = &ms.component_vector[ 3 ];
               
                  cv0->shape = "not defined";
                  cv0->name  = "Component A";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "Component B";
                  cv1->show_component.clear();
                  
                  cv2->shape = "not defined";
                  cv2->name  = "Component AB";
                  cv2->show_component.clear();
                  
                  cv3->shape = "not defined";
                  cv3->name  = "N-mer of component A (" 
                                + QString::number( (int) stoich ) + ")";
                  cv3->show_component.clear();

                  ms.assoc_vector << assoc << assoc ;
                  av0 = &ms.assoc_vector[ 0 ];
                  av1 = &ms.assoc_vector[ 1 ];
                  
                  // Hetero-Association:
                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = 2;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = 1;
                  av0->stoichiometry3 = 1;
                  
                  // Self-Association:
                  av1->keq            = 1.0;
                  av1->k_off          = (float) 1.0e-4;
                  av1->units          = "OD";
                  av1->component1     = 0;
                  av1->component2     = 3;
                  av1->component3     = -1;
                  av1->stoichiometry1 = 1;
                  av1->stoichiometry2 = (unsigned int) stoich;
                  av1->stoichiometry3 = 0;

                  initAssoc1( ms, av1->component1,
                                  av1->component2,
                                  av1->stoichiometry2 );

                  initAssoc2( ms );
                  
                  cv0->show_conc   = true;
                  cv0->show_stoich = 0;
                  cv0->show_keq    = false;
                  cv0->show_koff   = false;
                  cv0->show_component << 1 << 2;

                  cv1->show_conc   = true;
                  cv1->show_stoich = 0;
                  cv1->show_keq    = false;
                  cv1->show_koff   = false;
                  cv1->show_component << 0 << 2;

                  cv2->show_conc   = false;
                  cv2->show_stoich = -1; // -1 means hetero - association
                  cv2->show_keq    = true;
                  cv2->show_koff   = true;
                  cv1->show_component << 0 << 1;

                  cv3->show_conc   = false;
                  cv3->show_stoich = (int) stoich;
                  cv3->show_keq    = true;
                  cv3->show_koff   = true;
               }
            }
            break;
   
         // User-Defined Monomer - N-mer Equilibrium, some incompetent monomer,
         // we need some additional info from the user
         
         case UserMonoIncompMono:
            {
               float stoich = 2 ;
               
               US_SelectModel10* select = new US_SelectModel10( stoich );
               
               if ( ! select->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
                  cv2 = &ms.component_vector[ 2 ];

                  cv0->shape = "not defined";
                  cv0->name  = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "N-mer (" + QString::number( (int)stoich ) + ")";
                  cv1->show_component.clear();

                  cv2->shape = "not defined";
                  cv2->name  = "Incompetent Monomer";
                  cv2->show_component.clear();
                  
                  ms.assoc_vector << assoc << assoc ;
                  av0 = &ms.assoc_vector[ 0 ];
                  av1 = &ms.assoc_vector[ 1 ];
                  
                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich;
                  av0->stoichiometry3 = 0;

                  // Irreversible components have zero equilibrium constant
                  av1->keq            = 0.0;    
                  av1->k_off          = 0.0;
                  av1->units          = "OD";
                  av1->component1     = 0;
                  av1->component2     = 2;
                  av1->component3     = -1;
                  av1->stoichiometry1 = 1;
                  av1->stoichiometry2 = 1;
                  av1->stoichiometry3 = 0;
                  
                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2);

                  cv2->mw            = cv0->mw;
                  cv2->s             = cv0->s;
                  cv2->D             = cv0->D;
                  cv2->f_f0          = cv0->f_f0;
                  cv2->vbar20        = cv0->vbar20;
                  cv2->sigma         = cv0->sigma;
                  cv2->delta         = cv0->delta;
                  cv2->shape         = "";
                  cv2->concentration = (float) 0.3;
                  cv2->extinction    = cv0->extinction;
                  cv2->show_conc     = true;
                  cv2->show_stoich   = 1;
                  cv2->show_keq      = false;
                  cv2->show_koff     = false;
                  cv2->show_component << 0 << 2;
               }
            }
            break;
        
         // User-Defined Monomer - N-mer Equilibrium, some incompetent N-mer,
         // we need some additional info from the user
         
         case UserMonoIncompNmer: 
            { 
               float stoich = 2;

               US_SelectModel10* select = new US_SelectModel10( stoich );
               
               if ( ! select->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
                  cv2 = &ms.component_vector[ 2 ];

                  cv0->shape = "not defined";
                  cv0->name = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name = "N-mer (" + QString::number( (int)stoich ) + ")";
                  cv1->show_component.clear();

                  cv2->shape = "not defined";
                  cv2->name = "Incompetent N-mer";
                  cv2->show_component.clear();

                  ms.assoc_vector << assoc << assoc ;
                  av0 = &ms.assoc_vector[ 0 ];
                  av1 = &ms.assoc_vector[ 1 ];
                  
                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich;
                  av0->stoichiometry3 = 0;

                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2);

                  // Irreversible components have zero equilibrium constants
                  av1->keq            = 0.0; 
                  av1->k_off          = 0.0;
                  av1->units          = "OD";
                  av1->component1     = 0;
                  av1->component2     = 2;
                  av1->component3     = -1;
                  av1->stoichiometry1 = 1;
                  av1->stoichiometry2 = (int) stoich;
                  av1->stoichiometry3 = 0;
                  
                  cv2->mw            = cv1->mw;
                  cv2->s             = cv1->s;
                  cv2->D             = cv1->D;
                  cv2->f_f0          = cv1->f_f0;
                  cv2->vbar20        = cv1->vbar20;
                  cv2->sigma         = cv1->sigma;
                  cv2->delta         = cv1->delta;
                  cv2->shape         = "";
                  cv2->concentration = (float) 0.3;
                  cv2->extinction    = cv1->extinction;
                  cv2->show_conc     = true;
                  cv2->show_stoich   = (int) stoich;
                  cv2->show_keq      = false;
                  cv2->show_koff     = false;

                  cv2->show_component << 0;
                  cv0->show_component << 2;
               }
            }
            break;
        
         case UserIrreversible: // User-Defined irreversible Monomer - N-mer
            {         
               float stoich = 2;

               US_SelectModel10* select = new US_SelectModel10( stoich );
               
               if ( ! select->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];

                  cv0->shape = "not defined";
                  cv0->name  = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "N-mer (" + QString::number( (int)stoich ) + ")";
                  cv1->show_component.clear();
                  
                  ms.assoc_vector << assoc;
                  av0 = &ms.assoc_vector[ 0 ];
                  
                  // Irreversible components have zero equilibrium constants
                  
                  av0->keq            = 0.0;
                  av0->k_off          = 0.0;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich;
                  av0->stoichiometry3 = 0;
                  
                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2);
                  
                  cv0->concentration = (float) 0.7;
                  cv0->show_conc     = true;
                  cv0->show_stoich   = 0;
                  cv0->show_keq      = false;
                  cv0->show_koff     = false;
                  
                  cv1->concentration = (float) 0.3;
                  cv1->show_conc     = true;
                  cv1->show_stoich   = (int) stoich;
                  cv1->show_keq      = false;
                  cv1->show_koff     = false;
               }
            }

            break;
         
         // User-Defined Monomer - N-mer Equilibrium, plus some contaminant, we
         // need some additional info from the user

         case 19: 
            {
               float stoich=2;

               US_SelectModel10* select = new US_SelectModel10( stoich );
               
               if ( ! select->exec() )
               {
                  ms.model = -1;
               }
               else
               {
                  ms.component_vector << sc << sc;
                  cv0 = &ms.component_vector[ 0 ];
                  cv1 = &ms.component_vector[ 1 ];
                  cv2 = &ms.component_vector[ 2 ];

                  cv0->shape = "not defined";
                  cv0->name  = "Monomer";
                  cv0->show_component.clear();

                  cv1->shape = "not defined";
                  cv1->name  = "N-mer (" + QString::number( (int)stoich ) + ")";
                  cv1->show_component.clear();

                  cv2->name  = "Contaminant";
                  cv2->shape = "not defined";
                  cv2->show_component.clear();
                  
                  ms.assoc_vector << assoc;
                  av0 = &ms.assoc_vector[ 0 ];
                  
                  av0->keq            = 1.0;
                  av0->k_off          = (float) 1.0e-4;
                  av0->units          = "OD";
                  av0->component1     = 0;
                  av0->component2     = 1;
                  av0->component3     = -1;
                  av0->stoichiometry1 = 1;
                  av0->stoichiometry2 = (int) stoich;
                  av0->stoichiometry3 = 0;

                  initAssoc1( ms, av0->component1,
                                  av0->component2,
                                  av0->stoichiometry2 );

                  cv2->mw            = 22000.0;
                  cv2->s             = (float) 1.9802e-13;
                  cv2->D             = (float) 7.7981e-7;
                  cv2->f_f0          = (float) 1.4893;
                  cv2->vbar20        = (float) 0.72;
                  cv2->sigma         = 0.0;
                  cv2->delta         = 0.0;
                  cv2->shape         = "prolate";
                  cv2->concentration = (float) 0.3;
                  cv2->extinction    = 0.5;
                  cv2->show_conc     = true;
                  cv2->show_stoich   = 0;
                  cv2->show_keq      = false;
                  cv2->show_koff     = false;
               }
            }

            break;
      }  // End switch
   }
   else
   {
      ms.model = -1;
      return;
   }

   if ( ms.model >= 0 && ms.model < 4 ) // noninteracting multicomponent ms
   {
      int components =  ms.component_vector.size();

      for ( int i = 0; i < components; i++)
      {
         struct SimulationComponent* cv = &ms.component_vector[ i ];

         cv->show_conc     = true;
         cv->show_stoich   = 0;
         cv->show_keq      = false;
         cv->show_koff     = false;
         cv->shape         = "";
         cv->s             = 0.0;
         cv->D             = 0.0;
         cv->concentration = 0.0;
         cv->mw            = 0.0;
         cv->show_component.clear();
         cv->c0.radius.clear();
         cv->c0.concentration.clear();

         if ( ms.model <= 3 && model3_vbar != 0.0 )
         {
            double base;
            cv->vbar20 = model3_vbar;

            if ( ms.component_vector.size() == 1 )
            {
               cv->mw   = mw_lowerLimit;
               cv->f_f0 = 1.25;
               
               base = ( 0.75 / AVOGADRO ) * cv->mw * cv->vbar20 * M_PI * M_PI;

               cv->s = cv->mw * ( 1.0 - cv->vbar20 * DENS_20W ) 
                       / ( AVOGADRO * cv->f_f0 * 6.0 * VISC_20W 
                           * pow( base,  1.0 / 3.0 ) );

               base = 2.0 * cv->s * cv->f_f0 * cv->vbar20 * VISC_20W 
                      / ( 1.0 - cv->vbar20 * DENS_20W );

               cv->D = ( R * K20 ) 
                       / ( AVOGADRO * cv->f_f0  * 9.0 * VISC_20W * M_PI 
                           * pow( base, 0.5 ) );

               cv->concentration = 1.0;
            }
            else
            {
               cv->mw = mw_lowerLimit + 
                  ( i * ( mw_upperLimit - mw_lowerLimit) / ( components - 1 ) );

               cv->f_f0 = 1.25;
               
               base = ( 0.75 / AVOGADRO ) * cv->mw * cv->vbar20 * M_PI * M_PI;

               cv->s = cv->mw * ( 1.0 - cv->vbar20 * DENS_20W )
                       / ( AVOGADRO * cv->f_f0 * 6.0 * VISC_20W * 
                           pow( base, 1.0 / 3.0 ) 
                         );

               base = 2.0 * cv->s * cv->f_f0 * cv->vbar20 * VISC_20W 
                      / ( 1.0 - cv->vbar20 * DENS_20W );

               cv->D = ( R * K20 ) 
                       / ( AVOGADRO * cv->f_f0 * 9.0 * VISC_20W * M_PI * 
                           pow( base, 0.5 ) 
                         );

               cv->concentration = 1.0 / components;
            } 
         }
         else
         {
            cv->vbar20 = (float) 0.72;
         }

         cv->sigma      = 0.0;
         cv->delta      = 0.0;
         cv->extinction = 1.0;
         cv->shape      = "not defined";
      }
   }

}

// initialize self-Association models:

void US_ModelSelection::initAssoc1( struct ModelSystem& ms, 
                                    unsigned int        comp1, 
                                    unsigned int        comp2, 
                                    unsigned int        stoich1 )
{
   struct SimulationComponent* sc1 = &ms.component_vector[ comp1 ];
   struct SimulationComponent* sc2 = &ms.component_vector[ comp2 ];
   
   sc1->show_conc     = true;
   sc1->show_stoich   = 0;
   sc1->show_keq      = false;
   sc1->show_koff     = false;
   sc1->mw            = 5.0e4;

   sc1->show_component << comp2;
   sc1->f_f0          = 1.25;
   sc1->vbar20        = (float) 0.72;
   sc1->sigma         = 0.0;
   sc1->delta         = 0.0;
   sc1->shape         = "not defined";
   sc1->concentration = 1.0;
   sc1->extinction    = 1.0;
   sc1->c0.radius.clear();
   sc1->c0.concentration.clear();

   sc2->show_conc     = false;
   sc2->show_stoich   = stoich1;
   sc2->show_keq      = true;
   sc2->show_koff     = true;
   sc2->show_component << comp1;
   sc2->mw            = 5.0e4 * stoich1;
   sc2->vbar20        = (float) 0.72;
   sc2->sigma         = 0.0;
   sc2->delta         = 0.0;
   sc2->shape         = "not defined";
   sc2->concentration = 0.0;
   sc2->extinction    = stoich1;
   sc2->c0.radius.clear();
   sc2->c0.concentration.clear();

   double base = ( 0.75 / AVOGADRO ) * sc1->mw * sc1->vbar20 * M_PI * M_PI;

   sc1->s = ( sc1->mw * ( 1.0 - sc1->vbar20 * DENS_20W ) )
            / ( AVOGADRO * sc1->f_f0 * 6.0 * VISC_20W * pow( base, 1.0 / 3.0 ) );

   base = 2.0 * sc1->s * sc1->f_f0 * sc1->vbar20 * VISC_20W  
          / ( 1.0 - sc1->vbar20 * DENS_20W );

   sc1->D = ( R * K20 ) 
            / ( AVOGADRO * sc1->f_f0 * 9.0 * VISC_20W * M_PI * pow( base, 0.5 ) );

   sc2->s = sc1->s * pow ( (double) stoich1, 0.75 );  
   // apply M^2/3 rule ( s2/s1 = (M2/M1)^2/3)
   
   sc2->f_f0 = 1.25; // f/f0 stays constant for M^2/3 rule, f changes M^1/3
   
   base = 2.0 * sc2->s * sc2->f_f0 * sc2->vbar20 * VISC_20W 
          / ( 1.0 - sc2->vbar20 * DENS_20W );

   sc2->D = R * K20  
            / ( AVOGADRO * sc2->f_f0 * 9.0 * VISC_20W * M_PI * pow( base, 0.5 ) );
}

// initialize Heteroassociation models:

void US_ModelSelection::initAssoc2( struct ModelSystem& ms )
{
   struct SimulationComponent* cv0 = &ms.component_vector[ 0 ];
   struct SimulationComponent* cv1 = &ms.component_vector[ 1 ];
   struct SimulationComponent* cv2 = &ms.component_vector[ 2 ];

// Component A:
   cv0->mw            = 5.0e4;
   cv0->f_f0          = 1.25;
   cv0->vbar20        = (float) 0.72;
   cv0->sigma         = 0.0;
   cv0->shape         = "not defined";
   cv0->concentration = 1.0;
   cv0->extinction    = 1.0;
   
   cv0->c0.radius.clear();
   cv0->c0.concentration.clear();

   double base = ( 0.75 / AVOGADRO ) * cv0->mw * cv0->vbar20 * M_PI * M_PI;

   cv0->s = ( cv0->mw * ( 1.0 - cv0->vbar20 * DENS_20W ) )
            / ( AVOGADRO * cv0->f_f0 * 6.0 * VISC_20W 
               * pow( base, 1.0 / 3.0 ) );
   
   base =  2.0 * cv0->s * cv0->f_f0 * cv0->vbar20 * VISC_20W  
           / ( 1.0 - cv0->vbar20 * DENS_20W );

   cv0->D = R * K20 
            / ( AVOGADRO * cv0->f_f0 * 9.0 * VISC_20W * M_PI * pow( base, 0.5 ) );

// Component B:
   cv1->mw            = 22000.0;
   cv1->s             = (float) 1.9802e-13;
   cv1->D             = (float) 7.7981e-7;
   cv1->f_f0          = (float) 1.4893;
   cv1->vbar20        = (float) 0.72;
   cv1->sigma         = 0.0;
   cv1->delta         = 0.0;
   cv1->shape         = "prolate";
   cv1->concentration = (float) 0.3;
   cv1->extinction    = 0.5;
   
   cv1->c0.radius.clear();
   cv1->c0.concentration.clear();

// Component AB:
   cv2->mw            = 72000.0;
   cv2->s             = (float) 4.7276e-13;
   cv2->D             = (float) 5.6886e-7;
   cv2->f_f0          = (float) 1.3751;
   cv2->vbar20        = (float) 0.72;
   cv2->sigma         = 0.0;
   cv2->delta         = 0.0;
   cv2->shape         = "prolate";
   cv2->concentration = 0.0;
   cv2->extinction    = 1.5;

   cv2->c0.radius.clear();
   cv2->c0.concentration.clear();
}
