#include "../include/us_ga_model_editor.h"

US_GAModelEditor::US_GAModelEditor(struct ModelSystem *system, struct ModelSystemConstraints *constraints,
QWidget *parent, const char *name) : US_ModelEditor(system, parent, name)
{

	setup_GUI();
	select_component((int) 0);
	
	global_Xpos += 30;
	global_Ypos += 30;

	move(global_Xpos, global_Ypos);
}

US_GAModelEditor::~US_GAModelEditor()
{
}

void US_GAModelEditor::setup_GUI()
{
	unsigned int j=3;
	QGridLayout *grid = new QGridLayout(this, 11, 4, 4, 2);
	grid->addMultiCellWidget(lbl_model, 0, 0, 0, 3, 0);
	grid->addMultiCellWidget(lbl_current, 1, 1, 0, 1, 0);
	grid->addMultiCellWidget(lbl_linked, 1, 1, 2, 3, 0);
	grid->addMultiCellWidget(cmb_component1, 2, 2, 0, 1, 0);
	grid->addMultiCellWidget(cmb_component2, 2, 2, 2, 3, 0);
	grid->addWidget(lbl_sed, j, 0, 0);
	grid->addWidget(le_sed, j, 1, 0);
	grid->addWidget(lbl_conc, j, 2, 0);
	grid->addWidget(le_conc, j, 3, 0);
	j++;
	grid->addWidget(lbl_diff, j, 0, 0);
	grid->addWidget(le_diff, j, 1, 0);
	grid->addWidget(pb_load_c0, j, 2, 0);
	grid->addWidget(lbl_load_c0, j, 3, 0);
	j++;
	grid->addWidget(lbl_vbar1, j, 0, 0);
	grid->addWidget(lbl_vbar2, j, 1, 0);
	grid->addWidget(lbl_keq, j, 2, 0);
	grid->addWidget(le_keq, j, 3, 0);
	j++;
	grid->addWidget(lbl_mw1, j, 0, 0);
	grid->addWidget(lbl_mw2, j, 1, 0);
	grid->addWidget(lbl_koff, j, 2, 0);
	grid->addWidget(le_koff, j, 3, 0);
	j++;
	grid->addWidget(lbl_f_f01, j, 0, 0);
	grid->addWidget(lbl_f_f02, j, 1, 0);
	grid->addWidget(lbl_stoich, j, 2, 0);
	grid->addWidget(le_stoich, j, 3, 0);
	j++;
	grid->addWidget(pb_simulateComponent, j, 0, 0);
	grid->addWidget(cnt_item, j, 1, 0);
	grid->addWidget(lbl_sigma, j, 2, 0);
	grid->addWidget(le_sigma, j, 3, 0);
	j++;
	grid->addWidget(cb_prolate, j, 0, 0);
	grid->addWidget(cb_oblate, j, 1, 0);
	grid->addWidget(lbl_delta, j, 2, 0);
	grid->addWidget(le_delta, j, 3, 0);
	j++;
	grid->addWidget(cb_rod, j, 0, 0);
	grid->addWidget(cb_sphere, j, 1, 0);
	grid->addWidget(pb_help, j, 2, 0);
	grid->addWidget(pb_cancel, j, 3, 0);
	j++;
	grid->addMultiCellWidget(pb_load_model, j, j, 0, 1, 0);
	grid->addWidget(pb_save, j, 2, 0);
	grid->addWidget(pb_accept, j, 3, 0);
}

void US_GAModelEditor::load_model(const QString &fileName)
{

	QFile f(fileName);
	QString str;
	bool flag;
	unsigned int i, j;
	if (f.open(IO_ReadOnly | IO_Translate))
	{
		flag = true;
		QTextStream ts(&f);
		ts.readLine(); // FE, SA2D, COFS, SIM or GA
		(*system).description = ts.readLine();
		if ((*system).description.isNull())
		{
			printError(3);
			return;
		}
		str = ts.readLine();
		if (str.find("#", 0, true) == 0) // a new model has a comment line in the second line starting with "#"
		{
			float fval;
			ts >> fval; // UltraScan version
			ts.readLine(); // read rest of line
			ts >> str;
			ts.readLine(); // read rest of line
			if (str.isNull())
			{
				printError(0);
				return;
			}
			(*system).model = str.toInt();
			ts >> str;
			if (str.isNull())
			{
				printError(0);
				return;
			}
			ts.readLine();
			(*system).component_vector.resize(str.toInt());
			for (i=0; i<(*system).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				int pos = str.find("#", 0, true);
				str.truncate(pos);
				(*system).component_vector[i].name = str.stripWhiteSpace();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].concentration = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].s = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].D = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].sigma = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].delta = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].mw = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].vbar20 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].shape = str;
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].f_f0 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].extinction = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_conc = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_stoich = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_keq = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_koff = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).component_vector[i].show_component.resize(str.toUInt());
				for (j=0; j<(*system).component_vector[i].show_component.size(); j++)
				{
					ts >> str;
					if (str.isNull())
					{
						printError(0);
						return;
					}
					ts.readLine();
					(*system).component_vector[i].show_component[j] = str.toInt();
				}
				if ((*system).component_vector[i].concentration < 0)
				{
					(*system).component_vector[i].c0.radius.clear();
					(*system).component_vector[i].c0.concentration.clear();
					ts >> str;
					if (str.isNull())
					{
						printError(0);
						return;
					}
					ts.readLine();
					unsigned int ival = str.toUInt();
					for (j=0; j<ival; j++)
					{
						ts >> str;
						if (str.isNull())
						{
							printError(0);
							return;
						}
						(*system).component_vector[i].c0.radius.push_back(str.toDouble());
						ts >> str;
						if (str.isNull())
						{
							printError(0);
							return;
						}
						(*system).component_vector[i].c0.concentration.push_back(str.toDouble());
					}
					ts.readLine(); //read the rest of the last linee
				}			}
			ts >> str;
			if (str.isNull())
			{
				printError(0);
				return;
			}
			ts.readLine();
			(*system).assoc_vector.resize(str.toUInt());
			for (i=0; i<(*system).assoc_vector.size(); i++)
			{
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].keq = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].units = str;
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].k_off = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component1 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component2 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].component3 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry1 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry2 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					printError(0);
					return;
				}
				ts.readLine();
				(*system).assoc_vector[i].stoichiometry3 = str.toUInt();
			}
		}
		else // load an old-style model file for noninteracting models
		{
			(*system).model = str.toInt();
			if ((*system).model > 3) // we can only read noninteracting models
			{
				printError(1);
				return;
			}
			(*system).model = 3; // set to fixed molecular weight distribution by default
			str = ts.readLine();
			if (str.isNull())
			{
				printError(0);
				return;
			}
			(*system).component_vector.resize(str.toInt()); // number of components
			for (i=0; i<(*system).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].concentration = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].s = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].D = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].sigma = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					printError(0);
					return;
				}
				(*system).component_vector[i].delta = str.toFloat();
				(*system).component_vector[i].vbar20 = 0.72;
				(*system).component_vector[i].extinction = 1.0;
				(*system).component_vector[i].name = str.sprintf("Component %d", i+1);
				
				(*system).component_vector[i].mw = ((*system).component_vector[i].s/(*system).component_vector[i].D)
				*((R*K20)/(1.0 - (*system).component_vector[i].vbar20 * DENS_20W));
				
				(*system).component_vector[i].f_f0 = (((*system).component_vector[i].mw * 
				(1.0 - (*system).component_vector[i].vbar20 * DENS_20W))/((*system).component_vector[i].s * AVOGADRO))
				/(6 * VISC_20W * pow(((*system).component_vector[i].mw * M_PI * M_PI * 3.0 
				* (*system).component_vector[i].vbar20)/(4.0 * AVOGADRO), 1.0/3.0));
				(*system).component_vector[i].show_conc = true;
				(*system).component_vector[i].show_keq = false;
				(*system).component_vector[i].show_koff = false;
				(*system).component_vector[i].show_stoich = 0;
			}
		}
		lbl_model->setText(modelString[(*system).model]);
		cmb_component1->clear();
		cmb_component2->clear();
		for (unsigned int i=0; i<(*system).component_vector.size(); i++)
		{
			cmb_component1->insertItem((*system).component_vector[i].name);
		}
		cnt_item->setValue(1);
		select_component((int) 0);
		printError(4);
	}
	else
	{
		printError(2);
	}
}

void US_GAModelEditor::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/astfem_component.html");
}

bool US_GAModelEditor::verify_model()
{
	bool flag = true;
	QString str1, str2;
	for (unsigned int i=0; i<(*system).assoc_vector.size(); i++)
	{
		if ((*system).assoc_vector[i].stoichiometry2 > 0 && (*system).assoc_vector[i].stoichiometry3 != 1) // then we need to check if the MWs match
		{
			if (fabs	((*system).component_vector[(*system).assoc_vector[i].component2].mw
			 - ((*system).component_vector[(*system).assoc_vector[i].component1].mw * (*system).assoc_vector[i].stoichiometry2 / (*system).assoc_vector[i].stoichiometry1
				)) > 1.0) // MWs don't match within 1 dalton
			{
				str2.sprintf(tr("The molecular weights of the reacting species\nin reaction %d do not agree:\n\n"), i+1);
				str1 = str2;
				str2.sprintf(tr("Molecular weight of species 1: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component1].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 2: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component2].mw);
				str1 += str2;
				str2.sprintf(tr("Stoichiometry of reaction %d: MW(1) * %d = MW(2)\n\n"), i+1, (*system).assoc_vector[i].stoichiometry2);
				str1 += str2;
				str1 += tr("Please adjust either MW(1) or MW(2) before proceeding...");
				QMessageBox::warning(this, "Model Definition Error", str1, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				flag = false;
			}
		}
		if ((*system).assoc_vector[i].stoichiometry3 == 1) // then we need to check if the sum of MW(1) + MW(2) = MW(3)
		{
			if (fabs	((*system).component_vector[(*system).assoc_vector[i].component3].mw
			 - (*system).component_vector[(*system).assoc_vector[i].component2].mw 
			 - (*system).component_vector[(*system).assoc_vector[i].component1].mw) > 1.0) // MWs don't match within 10 dalton
			{
				str2.sprintf(tr("The molecular weights of the reacting species\nin reaction %d do not agree:\n\n"), i+1);
				str1 = str2;
				str2.sprintf(tr("Molecular weight of species 1: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component1].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 2: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component2].mw);
				str1 += str2;
				str2.sprintf(tr("Molecular weight of species 3: %6.4e\n"), (*system).component_vector[(*system).assoc_vector[i].component3].mw);
				str1 += str2;
				str2.sprintf(tr("Stoichiometry of reaction %d: MW(1) + MW(2) = MW(3)\n\n"), i+1, (*system).assoc_vector[i].stoichiometry2);
				str1 += str2;
				str1 += tr("Please adjust the molecular weight of the appropriate\ncomponent before proceeding...");
				QMessageBox::warning(this, "Model Definition Error", str1, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
				flag = false;
			}
		}
	}
	return (flag);
}

void US_GAModelEditor::savefile(const QString &fileName)
{
	QFile f(fileName);
	QString str;
	unsigned int i, j;
	if (f.exists())
	{
		if(!QMessageBox::query( tr("Warning"), tr("Attention:\nThis file exists already!\n\nDo you want to overwrite it?"), tr("Yes"), tr("No")))
		{
			f.close();
			return;
		}
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		QString message = tr("Please enter a description\nfor your model:");
		OneLiner ol_descr(message);
		ol_descr.parameter1->setText((*system).description);
		ol_descr.show();
		if (ol_descr.exec())
		{
			if (ol_descr.string.isEmpty())
			{
				ol_descr.string = "not specified";
			}
			ts << "SIM" << "\n";
			ts << ol_descr.string << "\n";
			ts << "# This file is computer-generated, please do not edit unless you know what you are doing\n";
		}
		ts << US_Version << "\t\t# UltraScan Version Number\n";
		ts << (*system).model << "\t\t# model number/identifier\n";
		ts << (*system).component_vector.size() << "\t\t# number of components in the model\n";
		for (i=0; i<(*system).component_vector.size(); i++)
		{
			ts << (*system).component_vector[i].name << "\t\t# name of component\n";
			ts << (*system).component_vector[i].concentration << "\t\t# concentration\n";
			ts << (*system).component_vector[i].s << "\t\t# sedimentation coefficient\n";
			ts << (*system).component_vector[i].D << "\t\t# diffusion coefficient\n";
			ts << (*system).component_vector[i].sigma << "\t\t# sigma\n";
			ts << (*system).component_vector[i].delta << "\t\t# delta\n";
			ts << (*system).component_vector[i].mw << "\t\t# molecular Weight \n";
			ts << (*system).component_vector[i].vbar20 << "\t\t# vbar at 20C \n";
			ts << (*system).component_vector[i].shape << "\t\t# shape \n";
			ts << (*system).component_vector[i].f_f0 << "\t\t# frictional ratio \n";
			ts << (*system).component_vector[i].extinction << "\t\t# extinction\n";
			ts << (int) (*system).component_vector[i].show_conc << "\t\t# show concentration?\n";
			ts << (*system).component_vector[i].show_stoich << "\t\t# show Stoichiometry?\n";
			ts << (int) (*system).component_vector[i].show_keq << "\t\t# show k equilibrium?\n";
			ts << (int) (*system).component_vector[i].show_koff << "\t\t# show k_off?\n";
			ts << (*system).component_vector[i].show_component.size() << "\t\t# number of linked components\n";
			for (j=0; j<(*system).component_vector[i].show_component.size(); j++)
			{
				ts << (*system).component_vector[i].show_component[j] << str.sprintf("\t\t# linked component (%d)\n", j+1);
			}
			if((*system).component_vector[i].concentration < 0)
			{
				ts << (*system).component_vector[i].c0.radius.size() << "\t\t# number of initial concentration points\n";
				for (j=0; j<(*system).component_vector[i].c0.radius.size(); j++)
				{
					ts << (*system).component_vector[i].c0.radius[j] << " " 
					<< (*system).component_vector[i].c0.concentration[j] << endl;
				}
			}
		}
		ts << (*system).assoc_vector.size() << "\t\t# number of association reactions in the model\n";
		for (i=0; i<(*system).assoc_vector.size(); i++)
		{
			ts << (*system).assoc_vector[i].keq << "\t\t# equilibrium constant\n";
			ts << (*system).assoc_vector[i].units << "\t\t# units for equilibrium constant\n";
			ts << (*system).assoc_vector[i].k_off << "\t\t# rate constant\n";
			ts << (*system).assoc_vector[i].component1 << "\t\t# component 1 in this association\n";
			ts << (*system).assoc_vector[i].component2 << "\t\t# component 2 in this association\n";
			ts << (*system).assoc_vector[i].component3 << "\t\t# component 3 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry1 << "\t\t# stoichiometry for component 1 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry2 << "\t\t# stoichiometry for component 2 in this association\n";
			ts << (*system).assoc_vector[i].stoichiometry3 << "\t\t# stoichiometry for component 3 in this association\n";
		}
		f.close();
	}
}
