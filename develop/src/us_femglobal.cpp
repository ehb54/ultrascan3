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
				(*ms).component_vector[i].vbar20 =  (float) 0.72;
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

// read a model system, and the associated constraints needed for initialization of the fitting process.
// all associated files should start with the "filename" string

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

// read a model system, and the associated constraints needed for initialization of the fitting process.
// all associated files should start with the "filename" string

int US_FemGlobal::read_constraints(struct ModelSystem *ms, struct ModelSystemConstraints *msc, QString filename)
{
	QString str;
	unsigned int i, j, k;
	int flag1;
	QFile f;
	f.setName(filename);
	if (f.open(IO_ReadOnly))
	{
		QTextStream ts(&f);
		ts >> str;
		flag1 = read_modelSystem(ms, str);
		if (flag1 < 0)
		{
			f.close();
			return (flag1);
		}
		ts >> j;
		ts.readLine(); // j is the number of components in this model
		(*msc).component_vector_constraints.resize(j);
		for (i=0; i<j; i++)
		{
			ts >> k;
			ts.readLine();
			(*msc).component_vector_constraints[i].vbar20.fit = k;
			ts >> (*msc).component_vector_constraints[i].vbar20.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].vbar20.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].mw.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].mw.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].mw.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].s.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].s.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].s.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].D.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].D.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].D.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].sigma.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].sigma.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].sigma.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].delta.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].delta.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].delta.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].concentration.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].concentration.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].concentration.high;
			ts.readLine();
			ts >> k;
			(*msc).component_vector_constraints[i].f_f0.fit = k;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].f_f0.low;
			ts.readLine();
			ts >> (*msc).component_vector_constraints[i].f_f0.high;
			ts.readLine();
		}
		ts >> j; // j is the number of reactions in this model
		ts.readLine();
		(*msc).assoc_vector_constraints.resize(j);
		for (i=0; i<j; i++)
		{
			ts >> k;
			ts.readLine();
			(*msc).assoc_vector_constraints[i].keq.fit = k;
			ts >> (*msc).assoc_vector_constraints[i].keq.low;
			ts.readLine();
			ts >> (*msc).assoc_vector_constraints[i].keq.high;
			ts.readLine();
			ts >> k;
			ts.readLine();
			(*msc).assoc_vector_constraints[i].koff.fit = k;
			ts >> (*msc).assoc_vector_constraints[i].koff.low;
			ts.readLine();
			ts >> (*msc).assoc_vector_constraints[i].koff.high;
			ts.readLine();
		}
		ts >> (*msc).simpoints;
		ts.readLine();
		ts >> (*msc).mesh;
		ts.readLine();
		ts >> (*msc).moving_grid;
		ts.readLine();
		ts >> (*msc).band_volume;
		f.close();
		return(0);
	}
	else
	{
		return(-1); // can't read input file
	}
}

// write a model system, and the associated constraints needed for initialization of the fitting process.
// all associated files should start with the "filename" string

int US_FemGlobal::write_constraints(struct ModelSystem *ms, struct ModelSystemConstraints *msc, QString filename)
{
	QFile f;
	QString str;
	unsigned int i;
	if (filename.right(10) != ".constraints")
	{
		f.setName(filename + ".constraints");
	}
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&f);
		str.sprintf(filename + ".constraints.model-%d.00", (*ms).model);
		ts << str << endl;
		write_modelSystem(ms, str); // write the corresponding model to disc
		ts << (*msc).component_vector_constraints.size() << "\t\t# Number of components in the model\n";
		for (i=0; i<(*msc).component_vector_constraints.size(); i++)
		{
			ts << (*msc).component_vector_constraints[i].vbar20.fit << "\t\t# Is the vbar20 of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].vbar20.low << "\t\t# vbar20 lower limit\n";
			ts << (*msc).component_vector_constraints[i].vbar20.high << "\t\t# vbar20 upper limit\n";
			ts << (*msc).component_vector_constraints[i].mw.fit << "\t\t# Is molecular weight of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].mw.low << "\t\t# molecular weight lower limit\n";
			ts << (*msc).component_vector_constraints[i].mw.high << "\t\t# molecular weight upper limit\n";
			ts << (*msc).component_vector_constraints[i].s.fit << "\t\t# Is the sedimentation coefficient of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].s.low << "\t\t# sedimentation coefficient lower limit\n";
			ts << (*msc).component_vector_constraints[i].s.high << "\t\t# sedimentation coefficient upper limit\n";
			ts << (*msc).component_vector_constraints[i].D.fit << "\t\t# Is the diffusion coefficient of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].D.low << "\t\t# diffusion coefficient lower limit\n";
			ts << (*msc).component_vector_constraints[i].D.high << "\t\t# diffusion coefficient upper limit\n";
			ts << (*msc).component_vector_constraints[i].sigma.fit << "\t\t# Is concentration dependency of s of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].sigma.low << "\t\t# concentration dependency of s lower limit\n";
			ts << (*msc).component_vector_constraints[i].sigma.high << "\t\t# concentration dependency of s upper limit\n";
			ts << (*msc).component_vector_constraints[i].delta.fit << "\t\t# Is concentration dependency of D of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].delta.low << "\t\t# concentration dependency of D lower limit\n";
			ts << (*msc).component_vector_constraints[i].delta.high << "\t\t# concentration dependency of D upper limit\n";
			ts << (*msc).component_vector_constraints[i].concentration.fit << "\t\t# Is the concentration of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].concentration.low << "\t\t# concentration lower limit\n";
			ts << (*msc).component_vector_constraints[i].concentration.high << "\t\t# concentration upper limit\n";
			ts << (*msc).component_vector_constraints[i].f_f0.fit << "\t\t# Is the frictional ratio of component " << i+1 << " fitted?\n";
			ts << (*msc).component_vector_constraints[i].f_f0.low << "\t\t# frictional ratio lower limit\n";
			ts << (*msc).component_vector_constraints[i].f_f0.high << "\t\t# frictional ratio upper limit\n";
		}
		ts << (*msc).assoc_vector_constraints.size() << "\t\t# Number of reactions in the model\n";
		for (i=0; i<(*msc).assoc_vector_constraints.size(); i++)
		{
			ts << (*msc).assoc_vector_constraints[i].keq.fit << "\t\t# Is the equilibrium constant of association reaction " << i+1 << " fitted?\n";
			ts << (*msc).assoc_vector_constraints[i].keq.low << "\t\t# equilibrium constant lower limit\n";
			ts << (*msc).assoc_vector_constraints[i].keq.high << "\t\t# equilibrium constant upper limit\n";
			ts << (*msc).assoc_vector_constraints[i].koff.fit << "\t\t# Is the k_off rate constant of association reaction " << i+1 << " fitted?\n";
			ts << (*msc).assoc_vector_constraints[i].koff.low << "\t\t# k_off rate constant lower limit\n";
			ts << (*msc).assoc_vector_constraints[i].koff.high << "\t\t# k_off rate constant upper limit\n";
		}
		ts << (*msc).simpoints << "\t\t# the number of grid points\n";
		ts << (*msc).mesh << "\t\t# the type of radial mesh used: 0 = ASTFEM, 1 = Claverie, 2 = moving hat, 3 = user-selected mesh, 4 = nonuniform constant mesh\n";
		ts << (*msc).moving_grid << "\t\t# using moving (0) or fixed time grid (1)\n";
		ts << (*msc).band_volume << "\t\t# loading volume (of lamella) in a band-forming centerpiece, if used\n";
		f.close();
		return(0);
	}
	else
	{
		return(-1); // can't open output file
	}
	return 0;
}

// select a model from the predefined list and initialize the model variables with default values

void US_FemGlobal::select_model(struct ModelSystem *ms)
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
					return;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				(*ms).assoc_vector[1].keq = 1.0;
				(*ms).assoc_vector[1].k_off = 1.0e-4;
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
				(*ms).assoc_vector[0].units = "OD";
				(*ms).assoc_vector[0].component1 = 0;
				(*ms).assoc_vector[0].component2 = 1;
				(*ms).assoc_vector[0].component3 = -1;
				(*ms).assoc_vector[0].stoichiometry1 = 1;
				(*ms).assoc_vector[0].stoichiometry2 = 2;
				(*ms).assoc_vector[0].stoichiometry3 = 0;
				(*ms).assoc_vector[1].keq = 1.0;
				(*ms).assoc_vector[1].k_off = 1.0e-4;
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
					break;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = -1;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = (unsigned int) stoich1;
					(*ms).assoc_vector[0].stoichiometry3 = 0;
					(*ms).assoc_vector[1].keq = 1.0;
					(*ms).assoc_vector[1].k_off = 1.0e-4;
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
					break;
				}
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
				(*ms).assoc_vector[0].k_off = 1.0e-4;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
					(*ms).assoc_vector[0].units = "OD";
					(*ms).assoc_vector[0].component1 = 0;
					(*ms).assoc_vector[0].component2 = 1;
					(*ms).assoc_vector[0].component3 = 2;
					(*ms).assoc_vector[0].stoichiometry1 = 1;
					(*ms).assoc_vector[0].stoichiometry2 = 1;
					(*ms).assoc_vector[0].stoichiometry3 = 1;
// Self-Association:
					(*ms).assoc_vector[1].keq = 1.0;
					(*ms).assoc_vector[1].k_off = 1.0e-4;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
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
					(*ms).assoc_vector[0].k_off = 1.0e-4;
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
				break;
			}
		}
	}
	else
	{
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

void US_FemGlobal::initializeAssociation1(struct ModelSystem *ms, unsigned int comp1, unsigned int comp2, unsigned int stoich1)
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

void US_FemGlobal::initializeAssociation2(struct ModelSystem *ms)
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
