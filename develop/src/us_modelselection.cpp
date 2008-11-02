#include "../include/us_modelselection.h"

US_ModelSelection::US_ModelSelection(QObject *parent, const char *name) : QObject(parent, name)
{
}

US_ModelSelection::~US_ModelSelection()
{
}
// select a model from the predefined list and initialize the model variables with default values

void US_ModelSelection::select_model(struct ModelSystem *ms)
{
	QString str;
	bool model3_vbarflag=true;
	US_SelectModel *SelectModel;
	SelectModel = new US_SelectModel(&(*ms).model, false);
	(*ms).description = "Default ASTFEM Model";
	(*ms).component_vector.clear();
	(*ms).assoc_vector.clear();
	float mw_upperLimit=1.0e5, mw_lowerLimit=1.0e4;
	float model3_vbar = (float) 0.72;
	if (SelectModel->exec())
	{
		switch ((*ms).model)
		{
			case 0:
			{
				(*ms).component_vector.resize(1);
				(*ms).component_vector[0].name = "Component 1";
				break;
			}
			case 1:
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].name = "Component 1";
				(*ms).component_vector[1].name = "Component 2";
				break;
			}
			case 2:
			{
				(*ms).component_vector.resize(3);
				(*ms).component_vector[0].name = "Component 1";
				(*ms).component_vector[1].name = "Component 2";
				(*ms).component_vector[2].name = "Component 3";
				break;
			}
			case 3:	// fixed molecular weight distribution, we need some addtl. info from the user
			{
				US_SelectModel3 *SelectModel3;
				unsigned int species;
				SelectModel3 = new US_SelectModel3(&mw_upperLimit, &mw_lowerLimit, &species,
						&model3_vbar, &model3_vbarflag, -1);
				if (!SelectModel3->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(species);
					QString str;
					for (unsigned int i=0; i<species; i++)
					{
						(*ms).component_vector[i].name = str.sprintf("Component %d", i+1);
					}
				}
				delete SelectModel3;
				break;
			}
			case 4: // Monomer-Dimer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Dimer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 5: // Monomer-Trimer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Trimer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 3;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 6: // Monomer-Tetramer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Tetramer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 4;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 7: // Monomer-Pentamer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Pentamer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 5;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 8: // Monomer-Hexamer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Hexamer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 6;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 9: // Monomer-Heptamer Equilibrium
			{
				(*ms).component_vector.resize(2);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Heptamer";
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				break;
			}
			case 10:	// User-Defined Monomer - N-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(2);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich);
					(*ms).assoc_vector.resize(1);
					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);
				}
				delete SelectModel10;
				break;
			}
			case 11: // Monomer-Dimer-Trimer Equilibrium
			{
				(*ms).component_vector.resize(3);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[2].shape = "not defined";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[2].show_component.clear();
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Dimer";
				(*ms).component_vector[2].name = "Trimer";
				(*ms).assoc_vector.resize(2);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				(*ms).assoc_vector[1].keq = 1.0;
				(*ms).assoc_vector[1].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[1].units = "OD";
				(*ms).assoc_vector[1].component1 = 0;
				(*ms).assoc_vector[1].component2 = 2;
				(*ms).assoc_vector[1].component3 = -1;
				(*ms).assoc_vector[1].stoichiometry1 = 1;
				(*ms).assoc_vector[1].stoichiometry2 = 3;
				(*ms).assoc_vector[1].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				initializeAssociation1(ms, (*ms).assoc_vector[1].component1,
												 (*ms).assoc_vector[1].component2,
												 (*ms).assoc_vector[1].stoichiometry2);
				break;
			}
			case 12: // Monomer-Dimer-Tetramer Equilibrium
			{
				(*ms).component_vector.resize(3);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[2].shape = "not defined";
				(*ms).component_vector[0].name = "Monomer";
				(*ms).component_vector[1].name = "Dimer";
				(*ms).component_vector[2].name = "Tetramer";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[2].show_component.clear();
				(*ms).assoc_vector.resize(2);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				(*ms).assoc_vector[1].keq = 1.0;
				(*ms).assoc_vector[1].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[1].units = "OD";
				(*ms).assoc_vector[1].component1 = 0;
				(*ms).assoc_vector[1].component2 = 2;
				(*ms).assoc_vector[1].component3 = -1;
				(*ms).assoc_vector[1].stoichiometry1 = 1;
				(*ms).assoc_vector[1].stoichiometry2 = 4;
				(*ms).assoc_vector[1].stoichiometry3 = 0;
				initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
												 (*ms).assoc_vector[0].component2,
												 (*ms).assoc_vector[0].stoichiometry2);
				initializeAssociation1(ms, (*ms).assoc_vector[1].component1,
												 (*ms).assoc_vector[1].component2,
												 (*ms).assoc_vector[1].stoichiometry2);
				break;
			}
			case 13:	// User-Defined Monomer - N-mer - M-mer Equilibrium, we need some addtl. info from the user
			{
				US_SelectModel13 *SelectModel13;
				float stoich1=1, stoich2=2;
				SelectModel13 = new US_SelectModel13(&stoich1, &stoich2);
				if (!SelectModel13->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(3);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[2].shape = "not defined";
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[2].show_component.clear();
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich1);
					(*ms).component_vector[2].name = str.sprintf("M-mer (%d)", (unsigned int) stoich2);
					(*ms).assoc_vector.resize(2);
					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich1;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					(*ms).assoc_vector[1].keq = 1.0;
					(*ms).assoc_vector[1].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[1].units = "OD";
					(*ms).assoc_vector[1].component1 = 0;
					(*ms).assoc_vector[1].component2 = 2;
					(*ms).assoc_vector[1].component3 = -1;
					(*ms).assoc_vector[1].stoichiometry1 = 1;
					(*ms).assoc_vector[1].stoichiometry2 = (unsigned int) stoich2;
					(*ms).assoc_vector[1].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);
					initializeAssociation1(ms, (*ms).assoc_vector[1].component1,
													 (*ms).assoc_vector[1].component2,
													 (*ms).assoc_vector[1].stoichiometry2);
				}
				delete SelectModel13;
				break;
			}
			case 14:	// 2 component hetero association.
			{
				(*ms).component_vector.resize(3);
				(*ms).component_vector[0].shape = "not defined";
				(*ms).component_vector[1].shape = "not defined";
				(*ms).component_vector[2].shape = "not defined";
				(*ms).component_vector[0].name = "Component A";
				(*ms).component_vector[1].name = "Component B";
				(*ms).component_vector[2].name = "Component AB";
				(*ms).component_vector[0].show_component.clear();
				(*ms).component_vector[1].show_component.clear();
				(*ms).component_vector[2].show_component.clear();
				(*ms).assoc_vector.resize(1);
				(*ms).assoc_vector[0].keq = 1.0;
				(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = 2;
				(*ms).assoc_vector[0].stoichiometry1 = 1; // for hetero-associating (*ms)s the stoichiometry is zero
				(*ms).assoc_vector[0].stoichiometry2 = 1;
				(*ms).assoc_vector[0].stoichiometry3 = 1;
				initializeAssociation2(ms);
				(*ms).component_vector[0].show_conc = true;
				(*ms).component_vector[0].show_stoich = 0;
				(*ms).component_vector[0].show_keq = false;
				(*ms).component_vector[0].show_koff = false;
				(*ms).component_vector[0].show_component.push_back(1);
				(*ms).component_vector[0].show_component.push_back(2);

				(*ms).component_vector[1].show_conc = true;
				(*ms).component_vector[1].show_stoich = 0;
				(*ms).component_vector[1].show_keq = false;
				(*ms).component_vector[1].show_koff = false;
				(*ms).component_vector[1].show_component.push_back(0);
				(*ms).component_vector[1].show_component.push_back(2);

				(*ms).component_vector[2].show_conc = false;
				(*ms).component_vector[2].show_stoich = -1; // -1 means hetero - association
				(*ms).component_vector[2].show_keq = true;
				(*ms).component_vector[2].show_koff = true;
				(*ms).component_vector[2].show_component.push_back(0);
				(*ms).component_vector[2].show_component.push_back(1);
				break;
			}
			case 15:	// 2 component hetero association, one of the components self-associates.
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(4);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[2].shape = "not defined";
					(*ms).component_vector[3].shape = "not defined";
					(*ms).component_vector[0].name = "Component A";
					(*ms).component_vector[1].name = "Component B";
					(*ms).component_vector[2].name = "Component AB";
					(*ms).component_vector[3].name = str.sprintf("N-mer of component A (%d)", (unsigned int) stoich);
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[2].show_component.clear();
					(*ms).component_vector[3].show_component.clear();
					(*ms).assoc_vector.resize(2);
// Hetero-Association:
					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = 2;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = 1;
					(*ms).assoc_vector[0].stoichiometry3 = 1;
// Self-Association:
					(*ms).assoc_vector[1].keq = 1.0;
					(*ms).assoc_vector[1].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[1].units = "OD";
					(*ms).assoc_vector[1].component1 = 0;
					(*ms).assoc_vector[1].component2 = 3;
					(*ms).assoc_vector[1].component3 = -1;
					(*ms).assoc_vector[1].stoichiometry1 = 1;
					(*ms).assoc_vector[1].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[1].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[1].component1,
													 (*ms).assoc_vector[1].component2,
													 (*ms).assoc_vector[1].stoichiometry2);
					initializeAssociation2(ms);
					(*ms).component_vector[0].show_conc = true;
					(*ms).component_vector[0].show_stoich = 0;
					(*ms).component_vector[0].show_keq = false;
					(*ms).component_vector[0].show_koff = false;
					(*ms).component_vector[0].show_component.push_back(1);
					(*ms).component_vector[0].show_component.push_back(2);

					(*ms).component_vector[1].show_conc = true;
					(*ms).component_vector[1].show_stoich = 0;
					(*ms).component_vector[1].show_keq = false;
					(*ms).component_vector[1].show_koff = false;
					(*ms).component_vector[1].show_component.push_back(0);
					(*ms).component_vector[1].show_component.push_back(2);

					(*ms).component_vector[2].show_conc = false;
					(*ms).component_vector[2].show_stoich = -1; // -1 means hetero - association
					(*ms).component_vector[2].show_keq = true;
					(*ms).component_vector[2].show_koff = true;
					(*ms).component_vector[2].show_component.push_back(0);
					(*ms).component_vector[2].show_component.push_back(1);

					(*ms).component_vector[3].show_conc = false;
					(*ms).component_vector[3].show_stoich = (int) stoich;
					(*ms).component_vector[3].show_keq = true;
					(*ms).component_vector[3].show_koff = true;
				}
				delete SelectModel10;
				break;
			}
			case 16:	// User-Defined Monomer - N-mer Equilibrium, some incompetent monomer, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(3);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[2].shape = "not defined";
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich);
					(*ms).component_vector[2].name = "Incompetent Monomer";
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[2].show_component.clear();
					(*ms).assoc_vector.resize(2);

					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[0].stoichiometry3 = 0;

					(*ms).assoc_vector[1].keq = 0.0;     // irreversible components have zero equilibrium constant
					(*ms).assoc_vector[1].k_off = 0.0;
					(*ms).assoc_vector[1].units = "OD";
					(*ms).assoc_vector[1].component1 = 0;
					(*ms).assoc_vector[1].component2 = 2;
					(*ms).assoc_vector[1].component3 = -1;
					(*ms).assoc_vector[1].stoichiometry1 = 1;
					(*ms).assoc_vector[1].stoichiometry2 = 1;
					(*ms).assoc_vector[1].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);

					(*ms).component_vector[2].mw = (*ms).component_vector[0].mw;
					(*ms).component_vector[2].s = (*ms).component_vector[0].s;
					(*ms).component_vector[2].D = (*ms).component_vector[0].D;
					(*ms).component_vector[2].f_f0 = (*ms).component_vector[0].f_f0;
					(*ms).component_vector[2].vbar20 = (*ms).component_vector[0].vbar20;
					(*ms).component_vector[2].sigma = (*ms).component_vector[0].sigma;
					(*ms).component_vector[2].delta = (*ms).component_vector[0].delta;
					(*ms).component_vector[2].shape = "";
					(*ms).component_vector[2].concentration = (float) 0.3;
					(*ms).component_vector[2].extinction = (*ms).component_vector[0].extinction;
					(*ms).component_vector[2].show_conc = true;
					(*ms).component_vector[2].show_stoich = 1;
					(*ms).component_vector[2].show_keq = false;
					(*ms).component_vector[2].show_koff = false;
					(*ms).component_vector[2].show_component.push_back(0);
					(*ms).component_vector[0].show_component.push_back(2);
				}
				delete SelectModel10;
				break;
			}
			case 17:	// User-Defined Monomer - N-mer Equilibrium, some incompetent N-mer, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(3);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[2].shape = "not defined";
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[2].show_component.clear();
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich);
					(*ms).component_vector[2].name = "Incompetent N-mer";
					(*ms).assoc_vector.resize(2);
					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);

					(*ms).assoc_vector[1].keq = 0.0;   // irreversible components have zero equilibrium constants
					(*ms).assoc_vector[1].k_off = 0.0;
					(*ms).assoc_vector[1].units = "OD";
					(*ms).assoc_vector[1].component1 = 0;
					(*ms).assoc_vector[1].component2 = 2;
					(*ms).assoc_vector[1].component3 = -1;
					(*ms).assoc_vector[1].stoichiometry1 = 1;
					(*ms).assoc_vector[1].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[1].stoichiometry3 = 0;
					(*ms).component_vector[2].mw = (*ms).component_vector[1].mw;
					(*ms).component_vector[2].s = (*ms).component_vector[1].s;
					(*ms).component_vector[2].D = (*ms).component_vector[1].D;
					(*ms).component_vector[2].f_f0 = (*ms).component_vector[1].f_f0;
					(*ms).component_vector[2].vbar20 = (*ms).component_vector[1].vbar20;
					(*ms).component_vector[2].sigma = (*ms).component_vector[1].sigma;
					(*ms).component_vector[2].delta = (*ms).component_vector[1].delta;
					(*ms).component_vector[2].shape = "";
					(*ms).component_vector[2].concentration = (float) 0.3;
					(*ms).component_vector[2].extinction = (*ms).component_vector[1].extinction;
					(*ms).component_vector[2].show_conc = true;
					(*ms).component_vector[2].show_stoich = (int) stoich;
					(*ms).component_vector[2].show_keq = false;
					(*ms).component_vector[2].show_koff = false;
					(*ms).component_vector[2].show_component.push_back(0);
					(*ms).component_vector[0].show_component.push_back(2);
				}
				delete SelectModel10;
				break;
			}
			case 18:	// User-Defined irreversible Monomer - N-mer
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(2);
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich);
					(*ms).assoc_vector.resize(1);
					(*ms).assoc_vector[0].keq = 0.0;  // irreversible components have zero equilibrium constants
					(*ms).assoc_vector[0].k_off = 0.0;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);
					(*ms).component_vector[0].concentration = (float) 0.7;
					(*ms).component_vector[1].concentration = (float) 0.3;
					(*ms).component_vector[0].show_conc = true;
					(*ms).component_vector[0].show_stoich = 0;
					(*ms).component_vector[0].show_keq = false;
					(*ms).component_vector[0].show_koff = false;
					(*ms).component_vector[1].show_conc = true;
					(*ms).component_vector[1].show_stoich = (int) stoich;
					(*ms).component_vector[1].show_keq = false;
					(*ms).component_vector[1].show_koff = false;
				}
				delete SelectModel10;
				break;
			}
			case 19:	// User-Defined Monomer - N-mer Equilibrium, plus some contaminant, we need some addtl. info from the user
			{
				US_SelectModel10 *SelectModel10;
				float stoich=2;
				SelectModel10 = new US_SelectModel10(&stoich);
				if (!SelectModel10->exec())
				{
					(*ms).model = -1;
				}
				else
				{
					(*ms).component_vector.resize(3);
					(*ms).component_vector[0].show_component.clear();
					(*ms).component_vector[1].show_component.clear();
					(*ms).component_vector[2].show_component.clear();
					(*ms).component_vector[0].name = "Monomer";
					(*ms).component_vector[1].name = str.sprintf("N-mer (%d)", (unsigned int) stoich);
					(*ms).component_vector[2].name = "Contaminant";
					(*ms).component_vector[0].shape = "not defined";
					(*ms).component_vector[1].shape = "not defined";
					(*ms).component_vector[2].shape = "not defined";
					(*ms).assoc_vector.resize(1);
					(*ms).assoc_vector[0].keq = 1.0;
					(*ms).assoc_vector[0].k_off = (float) 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					initializeAssociation1(ms, (*ms).assoc_vector[0].component1,
													 (*ms).assoc_vector[0].component2,
													 (*ms).assoc_vector[0].stoichiometry2);
					(*ms).component_vector[2].mw = 22000.0;
					(*ms).component_vector[2].s      = (float) 1.9802e-13;
					(*ms).component_vector[2].D      = (float) 7.7981e-7;
					(*ms).component_vector[2].f_f0   = (float) 1.4893;
					(*ms).component_vector[2].vbar20 = (float) 0.72;
					(*ms).component_vector[2].sigma  = 0.0;
					(*ms).component_vector[2].delta  = 0.0;
					(*ms).component_vector[2].shape  = "prolate";
					(*ms).component_vector[2].concentration = (float) 0.3;
					(*ms).component_vector[2].extinction    = 0.5;
					(*ms).component_vector[2].show_conc     = true;
					(*ms).component_vector[2].show_stoich   = 0;
					(*ms).component_vector[2].show_keq      = false;
					(*ms).component_vector[2].show_koff     = false;
				}
				delete SelectModel10;
				break;
			}
		}
		delete SelectModel;
	}
	else
	{
		delete SelectModel;
		(*ms).model = -1;
		return;
	}
	if ((*ms).model >= 0 && (*ms).model < 4) // noninteracting multicomponent (*ms)
	{
		QString str;
		for (unsigned int i=0; i<(*ms).component_vector.size(); i++)
		{
			(*ms).component_vector[i].show_conc = true;
			(*ms).component_vector[i].show_stoich = 0;
			(*ms).component_vector[i].show_keq = false;
			(*ms).component_vector[i].show_koff = false;
			(*ms).component_vector[i].show_component.clear();
			(*ms).component_vector[i].shape = "";
			(*ms).component_vector[i].s = 0.0;
			(*ms).component_vector[i].D = 0.0;
			(*ms).component_vector[i].concentration = 0.0;
			(*ms).component_vector[i].mw = 0.0;
			(*ms).component_vector[i].c0.radius.clear();
			(*ms).component_vector[i].c0.concentration.clear();

			if((*ms).model <= 3 && model3_vbar != 0.0)
			{
				(*ms).component_vector[i].vbar20 = model3_vbar;
				if ((*ms).component_vector.size() == 1)
				{
					(*ms).component_vector[i].mw = mw_lowerLimit;
					(*ms).component_vector[i].f_f0 = 1.25;
					(*ms).component_vector[i].s = ((*ms).component_vector[i].mw * (1.0 - (*ms).component_vector[i].vbar20 * DENS_20W))
					/ (AVOGADRO * (*ms).component_vector[i].f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO)
							* (*ms).component_vector[i].mw * (*ms).component_vector[i].vbar20 * M_PI * M_PI, 1.0/3.0));

					(*ms).component_vector[i].D = (R * K20)/(AVOGADRO * (*ms).component_vector[i].f_f0
							* 9.0 * VISC_20W * M_PI * pow((2.0 * (*ms).component_vector[i].s * (*ms).component_vector[i].f_f0
							* (*ms).component_vector[i].vbar20 * VISC_20W) / (1.0-(*ms).component_vector[i].vbar20 * DENS_20W), 0.5));
					(*ms).component_vector[i].concentration = 1.0;
				}
				else
				{
					(*ms).component_vector[i].mw = mw_lowerLimit + (i * (mw_upperLimit - mw_lowerLimit)/((*ms).component_vector.size() - 1));
					(*ms).component_vector[i].f_f0 = 1.25;
					(*ms).component_vector[i].s = ((*ms).component_vector[i].mw * (1.0 - (*ms).component_vector[i].vbar20 * DENS_20W))
					/ (AVOGADRO * (*ms).component_vector[i].f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO)
							* (*ms).component_vector[i].mw * (*ms).component_vector[i].vbar20 * M_PI * M_PI, 1.0/3.0));

					(*ms).component_vector[i].D = (R * K20)/(AVOGADRO * (*ms).component_vector[i].f_f0
							* 9.0 * VISC_20W * M_PI * pow((2.0 * (*ms).component_vector[i].s * (*ms).component_vector[i].f_f0
							* (*ms).component_vector[i].vbar20 * VISC_20W) / (1.0-(*ms).component_vector[i].vbar20 * DENS_20W), 0.5));

					(*ms).component_vector[i].concentration = 1.0/(*ms).component_vector.size();;
				}
			}
			else
			{
				(*ms).component_vector[i].vbar20 = (float) 0.72;
			}
			(*ms).component_vector[i].sigma = 0.0;
			(*ms).component_vector[i].delta = 0.0;
			(*ms).component_vector[i].extinction = 1.0;
			(*ms).component_vector[i].shape = "not defined";
		}
	}
}

// initialize self-Association models:

void US_ModelSelection::initializeAssociation1(struct ModelSystem *ms, unsigned int comp1, unsigned int comp2, unsigned int stoich1)
{
	(*ms).component_vector[comp1].show_conc = true;
	(*ms).component_vector[comp1].show_stoich = 0;
	(*ms).component_vector[comp1].show_keq = false;
	(*ms).component_vector[comp1].show_koff = false;
	(*ms).component_vector[comp1].show_component.push_back(comp2);
	(*ms).component_vector[comp2].show_conc = false;
	(*ms).component_vector[comp2].show_stoich = stoich1;
	(*ms).component_vector[comp2].show_keq = true;
	(*ms).component_vector[comp2].show_koff = true;
	(*ms).component_vector[comp2].show_component.push_back(comp1);
	(*ms).component_vector[comp1].mw = 5.0e4;
	(*ms).component_vector[comp2].mw = 5.0e4 * stoich1;
	(*ms).component_vector[comp1].f_f0 = 1.25;
	(*ms).component_vector[comp1].vbar20 = (float) 0.72;
	(*ms).component_vector[comp2].vbar20 = (float) 0.72;
	(*ms).component_vector[comp1].sigma = 0.0;
	(*ms).component_vector[comp2].sigma = 0.0;
	(*ms).component_vector[comp1].delta = 0.0;
	(*ms).component_vector[comp2].delta = 0.0;
	(*ms).component_vector[comp1].shape = "not defined";
	(*ms).component_vector[comp2].shape = "not defined";
	(*ms).component_vector[comp1].concentration = 1.0;
	(*ms).component_vector[comp2].concentration = 0.0;
	(*ms).component_vector[comp1].extinction = 1.0;
	(*ms).component_vector[comp2].extinction = stoich1;
	(*ms).component_vector[comp1].c0.radius.clear();
	(*ms).component_vector[comp1].c0.concentration.clear();
	(*ms).component_vector[comp2].c0.radius.clear();
	(*ms).component_vector[comp2].c0.concentration.clear();

	(*ms).component_vector[comp1].s = ((*ms).component_vector[comp1].mw * (1.0 - (*ms).component_vector[comp1].vbar20 * DENS_20W))
	/ (AVOGADRO * (*ms).component_vector[comp1].f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO)
			* (*ms).component_vector[comp1].mw * (*ms).component_vector[comp1].vbar20 * M_PI * M_PI, 1.0/3.0));

	(*ms).component_vector[comp1].D = (R * K20)/(AVOGADRO * (*ms).component_vector[comp1].f_f0
			* 9.0 * VISC_20W * M_PI * pow((2.0 * (*ms).component_vector[comp1].s * (*ms).component_vector[comp1].f_f0
			* (*ms).component_vector[comp1].vbar20 * VISC_20W) / (1.0-(*ms).component_vector[comp1].vbar20 * DENS_20W), 0.5));

	(*ms).component_vector[comp2].s = (*ms).component_vector[comp1].s * pow((double) stoich1, 0.75);  // apply M^2/3 rule ( s2/s1 = (M2/M1)^2/3)
	(*ms).component_vector[comp2].f_f0 = 1.25; // f/f0 stays constant for M^2/3 rule, f changes M^1/3
	(*ms).component_vector[comp2].D = (R * K20)/(AVOGADRO * (*ms).component_vector[comp2].f_f0
			* 9.0 * VISC_20W * M_PI * pow((2.0 * (*ms).component_vector[comp2].s * (*ms).component_vector[comp2].f_f0
			* (*ms).component_vector[comp2].vbar20 * VISC_20W) / (1.0-(*ms).component_vector[comp2].vbar20 * DENS_20W), 0.5));
}

// initialize Heteroassociation models:

void US_ModelSelection::initializeAssociation2(struct ModelSystem *ms)
{
// Component A:
	(*ms).component_vector[0].mw = 5.0e4;
	(*ms).component_vector[0].f_f0 = 1.25;
	(*ms).component_vector[0].vbar20 = (float) 0.72;
	(*ms).component_vector[0].sigma = 0.0;
	(*ms).component_vector[0].shape = "not defined";
	(*ms).component_vector[0].concentration = 1.0;
	(*ms).component_vector[0].extinction = 1.0;
	(*ms).component_vector[0].s = ((*ms).component_vector[0].mw * (1.0 - (*ms).component_vector[0].vbar20 * DENS_20W))
	/ (AVOGADRO * (*ms).component_vector[0].f_f0 * 6.0 * VISC_20W * pow((0.75/AVOGADRO)
			* (*ms).component_vector[0].mw * (*ms).component_vector[0].vbar20 * M_PI * M_PI, 1.0/3.0));
	(*ms).component_vector[0].D = (R * K20)/(AVOGADRO * (*ms).component_vector[0].f_f0
			* 9.0 * VISC_20W * M_PI * pow((2.0 * (*ms).component_vector[0].s * (*ms).component_vector[0].f_f0
			* (*ms).component_vector[0].vbar20 * VISC_20W) / (1.0-(*ms).component_vector[0].vbar20 * DENS_20W), 0.5));

// Component B:
	(*ms).component_vector[1].mw = 22000.0;
	(*ms).component_vector[1].s = (float) 1.9802e-13;
	(*ms).component_vector[1].D = (float) 7.7981e-7;
	(*ms).component_vector[1].f_f0 = (float) 1.4893;
	(*ms).component_vector[1].vbar20 = (float) 0.72;
	(*ms).component_vector[1].sigma = 0.0;
	(*ms).component_vector[1].delta = 0.0;
	(*ms).component_vector[1].shape = "prolate";
	(*ms).component_vector[1].concentration = (float) 0.3;
	(*ms).component_vector[1].extinction = 0.5;

// Component AB:
	(*ms).component_vector[2].mw = 72000.0;
	(*ms).component_vector[2].s = (float) 4.7276e-13;
	(*ms).component_vector[2].D = (float) 5.6886e-7;
	(*ms).component_vector[2].f_f0 = (float) 1.3751;
	(*ms).component_vector[2].vbar20 = (float) 0.72;
	(*ms).component_vector[2].sigma = 0.0;
	(*ms).component_vector[2].delta = 0.0;
	(*ms).component_vector[2].shape = "prolate";
	(*ms).component_vector[2].concentration = 0.0;
	(*ms).component_vector[2].extinction = 1.5;
	for (unsigned int i=0; i<3; i++)
	{
		(*ms).component_vector[i].c0.radius.clear();
		(*ms).component_vector[i].c0.concentration.clear();
	}
}
