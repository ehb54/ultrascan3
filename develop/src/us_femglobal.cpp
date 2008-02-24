#include "../include/us_femglobal.h"

US_FemGlobal::US_FemGlobal(QObject *parent, const char *name) : QObject(parent, name)
{
}

US_FemGlobal::~US_FemGlobal()
{
}

int US_FemGlobal::read_modelSystem(struct ModelSystem *ms, QString filename)
{
	QFile f(filename);
	QString str;
	unsigned int i, j;
	if (f.open(IO_ReadOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts.readLine(); // FE, SA2D, COFS, SIM or GA
		(*ms).description = ts.readLine();
		if ((*ms).description.isNull())
		{
			f.close();
			return(-1);
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
				f.close();
				return(-2);
			}
			(*ms).model = str.toInt();
			ts >> str;
			if (str.isNull())
			{
				f.close();
				return(-3);
			}
			ts.readLine();
			(*ms).component_vector.resize(str.toInt());
			for (i=0; i<(*ms).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-4);
				}
				int pos = str.find("#", 0, true);
				str.truncate(pos);
				(*ms).component_vector[i].name = str.stripWhiteSpace();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-5);
				}
				ts.readLine();
				(*ms).component_vector[i].concentration = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-6);
				}
				ts.readLine();
				(*ms).component_vector[i].s = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-7);
				}
				ts.readLine();
				(*ms).component_vector[i].D = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-8);
				}
				ts.readLine();
				(*ms).component_vector[i].sigma = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-9);
				}
				ts.readLine();
				(*ms).component_vector[i].delta = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-10);
				}
				ts.readLine();
				(*ms).component_vector[i].mw = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-11);
				}
				ts.readLine();
				(*ms).component_vector[i].vbar20 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-12);
				}
				ts.readLine();
				(*ms).component_vector[i].shape = str;
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-13);
				}
				ts.readLine();
				(*ms).component_vector[i].f_f0 = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-14);
				}
				ts.readLine();
				(*ms).component_vector[i].extinction = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-15);
				}
				ts.readLine();
				(*ms).component_vector[i].show_conc = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-16);
				}
				ts.readLine();
				(*ms).component_vector[i].show_stoich = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-17);
				}
				ts.readLine();
				(*ms).component_vector[i].show_keq = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-18);
				}
				ts.readLine();
				(*ms).component_vector[i].show_koff = (bool) str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-19);
				}
				ts.readLine();
				(*ms).component_vector[i].show_component.resize(str.toUInt());
				for (j=0; j<(*ms).component_vector[i].show_component.size(); j++)
				{
					ts >> str;
					if (str.isNull())
					{
						f.close();
						return(-20);
					}
					ts.readLine();
					(*ms).component_vector[i].show_component[j] = str.toInt();
				}
				if ((*ms).component_vector[i].concentration < 0)
				{
					(*ms).component_vector[i].c0.radius.clear();
					(*ms).component_vector[i].c0.concentration.clear();
					ts >> str;
					if (str.isNull())
					{
						f.close();
						return(-21);
					}
					ts.readLine();
					unsigned int ival = str.toUInt();
					for (j=0; j<ival; j++)
					{
						ts >> str;
						if (str.isNull())
						{
							f.close();
							return(-22);
						}
						(*ms).component_vector[i].c0.radius.push_back(str.toDouble());
						ts >> str;
						if (str.isNull())
						{
							f.close();
							return(-23);
						}
						(*ms).component_vector[i].c0.concentration.push_back(str.toDouble());
					}
					ts.readLine(); //read the rest of the last linee
				}
			}
			ts >> str;
			if (str.isNull())
			{
				f.close();
				return(-24);
			}
			ts.readLine();
			(*ms).assoc_vector.resize(str.toUInt());
			for (i=0; i<(*ms).assoc_vector.size(); i++)
			{
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-25);
				}
				ts.readLine();
				(*ms).assoc_vector[i].keq = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-26);
				}
				ts.readLine();
				(*ms).assoc_vector[i].units = str;
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-27);
				}
				ts.readLine();
				(*ms).assoc_vector[i].k_off = str.toFloat();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-28);
				}
				ts.readLine();
				(*ms).assoc_vector[i].component1 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-29);
				}
				ts.readLine();
				(*ms).assoc_vector[i].component2 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-30);
				}
				ts.readLine();
				(*ms).assoc_vector[i].component3 = str.toInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-31);
				}
				ts.readLine();
				(*ms).assoc_vector[i].stoichiometry1 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-32);
				}
				ts.readLine();
				(*ms).assoc_vector[i].stoichiometry2 = str.toUInt();
				ts >> str;
				if (str.isNull())
				{
					f.close();
					return(-33);
				}
				ts.readLine();
				(*ms).assoc_vector[i].stoichiometry3 = str.toUInt();
			}
		}
		else // load an old-style model file for noninteracting models
		{
			(*ms).model = str.toInt();
			if ((*ms).model > 3) // we can only read noninteracting models
			(*ms).model = 3; // set to fixed molecular weight distribution by default
			str = ts.readLine();
			if (str.isNull())
			{
				f.close();
				return(-34);
			}
			(*ms).component_vector.resize(str.toInt()); // number of components
			for (i=0; i<(*ms).component_vector.size(); i++)
			{
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-35);
				}
				(*ms).component_vector[i].concentration = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-36);
				}
				(*ms).component_vector[i].s = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-37);
				}
				(*ms).component_vector[i].D = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-38);
				}
				(*ms).component_vector[i].sigma = str.toFloat();
				str = ts.readLine();
				if (str.isNull())
				{
					f.close();
					return(-39);
				}
				(*ms).component_vector[i].delta = str.toFloat();
				(*ms).component_vector[i].vbar20 = 0.72;
				(*ms).component_vector[i].extinction = 1.0;
				(*ms).component_vector[i].name = str.sprintf("Component %d", i+1);
				(*ms).component_vector[i].mw = ((*ms).component_vector[i].s/(*ms).component_vector[i].D)
						*((R*K20)/(1.0 - (*ms).component_vector[i].vbar20 * DENS_20W));
				(*ms).component_vector[i].f_f0 = (((*ms).component_vector[i].mw *
						(1.0 - (*ms).component_vector[i].vbar20 * DENS_20W))/((*ms).component_vector[i].s * AVOGADRO))
				/(6 * VISC_20W * pow(((*ms).component_vector[i].mw * M_PI * M_PI * 3.0
						* (*ms).component_vector[i].vbar20)/(4.0 * AVOGADRO), 1.0/3.0));
				(*ms).component_vector[i].show_conc = true;
				(*ms).component_vector[i].show_keq = false;
				(*ms).component_vector[i].show_koff = false;
				(*ms).component_vector[i].show_stoich = 0;
			}
		}
		f.close();
		return(0);
	}
	else
	{
		return(-40);
	}
}

int US_FemGlobal::write_modelSystem(struct ModelSystem *ms, QString filename)
{
	QFile f(filename);
	QString str;
	unsigned int i, j;
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << "SIM" << "\n";
		ts << "Model written by US_FEMGLOBAL\n";
		ts << "# This file is computer-generated, please do not edit unless you know what you are doing\n";
		ts << US_Version << "\t\t# UltraScan Version Number\n";
		ts << (*ms).model << "\t\t# model number/identifier\n";
		ts << (*ms).component_vector.size() << "\t\t# number of components in the model\n";
		for (i=0; i<(*ms).component_vector.size(); i++)
		{
			ts << (*ms).component_vector[i].name << "\t\t# name of component\n";
			ts << (*ms).component_vector[i].concentration << "\t\t# concentration\n";
			ts << (*ms).component_vector[i].s << "\t\t# sedimentation coefficient\n";
			ts << (*ms).component_vector[i].D << "\t\t# diffusion coefficient\n";
			ts << (*ms).component_vector[i].sigma << "\t\t# sigma\n";
			ts << (*ms).component_vector[i].delta << "\t\t# delta\n";
			ts << (*ms).component_vector[i].mw << "\t\t# molecular Weight \n";
			ts << (*ms).component_vector[i].vbar20 << "\t\t# vbar at 20C \n";
			ts << (*ms).component_vector[i].shape << "\t\t# shape \n";
			ts << (*ms).component_vector[i].f_f0 << "\t\t# frictional ratio \n";
			ts << (*ms).component_vector[i].extinction << "\t\t# extinction\n";
			ts << (int) (*ms).component_vector[i].show_conc << "\t\t# show concentration?\n";
			ts << (*ms).component_vector[i].show_stoich << "\t\t# show Stoichiometry?\n";
			ts << (int) (*ms).component_vector[i].show_keq << "\t\t# show k equilibrium?\n";
			ts << (int) (*ms).component_vector[i].show_koff << "\t\t# show k_off?\n";
			ts << (*ms).component_vector[i].show_component.size() << "\t\t# number of linked components\n";
			for (j=0; j<(*ms).component_vector[i].show_component.size(); j++)
			{
				ts << (*ms).component_vector[i].show_component[j] << str.sprintf("\t\t# linked component (%d)\n", j+1);
			}
			if((*ms).component_vector[i].concentration < 0)
			{
				ts << (*ms).component_vector[i].c0.radius.size() << "\t\t# number of initial concentration points\n";
				for (j=0; j<(*ms).component_vector[i].c0.radius.size(); j++)
				{
					ts << (*ms).component_vector[i].c0.radius[j] << " "
							<< (*ms).component_vector[i].c0.concentration[j] << endl;
				}
			}
		}
		ts << (*ms).assoc_vector.size() << "\t\t# number of association reactions in the model\n";
		for (i=0; i<(*ms).assoc_vector.size(); i++)
		{
			ts << (*ms).assoc_vector[i].keq << "\t\t# equilibrium constant\n";
			ts << (*ms).assoc_vector[i].units << "\t\t# units for equilibrium constant\n";
			ts << (*ms).assoc_vector[i].k_off << "\t\t# rate constant\n";
			ts << (*ms).assoc_vector[i].component1 << "\t\t# component 1 in this association\n";
			ts << (*ms).assoc_vector[i].component2 << "\t\t# component 2 in this association\n";
			ts << (*ms).assoc_vector[i].component3 << "\t\t# component 3 in this association\n";
			ts << (*ms).assoc_vector[i].stoichiometry1 << "\t\t# stoichiometry for component 1 in this association\n";
			ts << (*ms).assoc_vector[i].stoichiometry2 << "\t\t# stoichiometry for component 2 in this association\n";
			ts << (*ms).assoc_vector[i].stoichiometry3 << "\t\t# stoichiometry for component 3 in this association\n";
		}
		f.close();
		return(0);
	}
	else
	{
		return(-35);
	}
}

int US_FemGlobal::read_simulationParameters(struct SimulationParameters *sp, QString filename)
{
	QFile f(filename);
	int ival;
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		if (!ts.atEnd())
		{
			unsigned int ival;
			ts >> ival;
			(*sp).speed_step.resize(ival);
			ts.readLine();
		}
		else
		{
			f.close();
			return (-51);
		}
		for (unsigned int i=0; i<(*sp).speed_step.size(); i++)
		{
			QTextStream ts(&f);
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].duration_hours;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-52);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].duration_minutes;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-53);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].delay_hours;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-54);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].delay_minutes;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-55);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].rotorspeed;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-56);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].acceleration;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-57);
			}
			if (!ts.atEnd())
			{
				int ival;
				ts >> ival;
				(*sp).speed_step[i].acceleration_flag = ival;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-58);
			}
			if (!ts.atEnd())
			{
				ts >> (*sp).speed_step[i].scans;
				ts.readLine();
			}
			else
			{
				f.close();
				return(-59);
			}
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).simpoints;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-60);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).radial_resolution;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-61);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).meniscus;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-62);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).bottom;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-63);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).rnoise;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-64);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).inoise;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-65);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).rinoise;
			ts.readLine();
		}
		else
		{
			f.close();
			return(-66);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).mesh;
		}
		else
		{
			f.close();
			return(-67);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).moving_grid;
		}
		else
		{
			f.close();
			return(-68);
		}
		if (!ts.atEnd())
		{
			ts >> (*sp).rotor;
		}
		else
		{
			f.close();
			return(-70);
		}
		if (!ts.atEnd())
		{
			ts >> ival;
			if (ival == 1)
			{
				(*sp).band_forming = true;
				if (!ts.atEnd())
				{
					ts >> (*sp).band_volume;
				}
				else
				{
					f.close();
					return(-71);
				}
			}
			else
			{
				(*sp).band_forming = false;
			}
		}
		else
		{
			f.close();
			return(-71);
		}
		f.close();
		return(0);
	}
	else
	{
		return(-72);
	}
}

int US_FemGlobal::write_simulationParameters(struct SimulationParameters *sp, QString filename)
{
	QString str = QString(filename);
	QFile f(str);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		ts << (*sp).speed_step.size() << "\t\t# Number of speed step profiles" << "\n";
		for (unsigned int i=0; i<(*sp).speed_step.size(); i++)
		{
			ts << (*sp).speed_step[i].duration_hours << str.sprintf("\t\t# run duration hours for profile %d\n", i+1);
			ts << (*sp).speed_step[i].duration_minutes << str.sprintf("\t\t# run duration minutes for profile %d\n", i+1);
			ts << (*sp).speed_step[i].delay_hours << str.sprintf("\t\t# run delay hours for profile %d\n", i+1);
			ts << (*sp).speed_step[i].delay_minutes << str.sprintf("\t\t# run delay minutes for profile %d\n", i+1);
			ts << (*sp).speed_step[i].rotorspeed << str.sprintf("\t\t# rotor speed for profile %d\n", i+1);
			ts << (*sp).speed_step[i].acceleration << str.sprintf("\t\t# acceleration profile in revs/sec for profile %d\n", i+1);
			ts << (int) (*sp).speed_step[i].acceleration_flag << str.sprintf("\t\t# flag for checking if rotor acceleration is used for profile %d\n", i+1);
			ts << (*sp).speed_step[i].scans << str.sprintf("\t\t# number of scans to save for profile %d\n", i+1);
		}
		ts << (*sp).simpoints << "\t\t# radial discretization simulation points" << "\n";
		ts << (*sp).radial_resolution << "\t\t# radial resolution (cm)" << "\n";
		ts << (*sp).meniscus << "\t\t# meniscus position (cm)" << "\n";
		ts << (*sp).bottom << "\t\t# bottom of cell position (cm)" << "\n";
		ts << (*sp).rnoise << "\t\t# random noise (in percent OD)" << "\n";
		ts << (*sp).inoise << "\t\t# time invariant systematic noise (in percent OD)" << "\n";
		ts << (*sp).rinoise << "\t\t# radial invariant systematic noise (in percent OD)" << "\n";
		ts << (*sp).mesh << "\t\t# Selected simulation mesh" << "\n";
		ts << (*sp).moving_grid << "\t\t# moving time grid (0 = Astfem, 1 = fixed)" << "\n";
		ts << (*sp).rotor << "\t\t# Rotor serial number" << "\n";
		if ((*sp).band_forming)
		{
			ts << "1\t\t# Band-forming centerpiece is used\n";
			ts << (*sp).band_volume << "\t\t# Band loading volume in ml" << "\n";
		}
		else
		{
			ts << "0\t\t# Standard centerpiece is used\n";
		}
		f.close();
		return(0);
	}
	else
	{
		return(-1);
	}
}

int US_FemGlobal::read_experiment(struct ModelSystem *ms, struct SimulationParameters *sp, QString filename)
{
	QString str;
	int flag1, flag2;
	QFile f;
	f.setName(filename);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		ts >> str;
		flag1 = read_simulationParameters(sp, str);
		ts >> str;
		flag2 = read_modelSystem(ms, str);
		f.close();
		if (flag1 < 0)
		{
			return (flag1);
		}
		if (flag2 < 0)
		{
			return (flag2);
		}
		return(0);
	}
	else
	{
		return(-1); // can't read input file
	}
}

int US_FemGlobal::write_experiment(struct ModelSystem *ms, struct SimulationParameters *sp, QString filename)
{
	QFile f;
	QString str;
	if (filename.right(10) == ".us_system")
	{
		filename.truncate(filename.length()-10);
	}
	f.setName(filename + ".us_system");
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		str = filename + ".us_system.simulation_parameters";
		ts << str << " ";
		write_simulationParameters(sp, str);
		str.sprintf(filename + ".us_system.model-%d.00", (*ms).model);
		ts << str << endl;
		write_modelSystem(ms, str);
		f.close();
		return(0);
	}
	else
	{
		return(-1); // can't open output file
	}
}
