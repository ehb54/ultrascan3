#include "../include/us_femglobal.h"
#if defined(USE_MPI)
# include "../include/us_version.h"
#endif

#include <qregexp.h>

#if defined(USE_MPI)
# include "../include/us_version.h"
#endif

US_FemGlobal::US_FemGlobal(QObject *parent, const char *name) : QObject(parent, name)
{
}

US_FemGlobal::~US_FemGlobal()
{
}

int US_FemGlobal::read_modelSystem(struct ModelSystem *ms, QString filename, bool flag)
{
   QString str;
   vector <QString> qsv;
   QFile f;
   f.setName(filename);
   if (f.open(IO_ReadOnly | IO_Translate))
   {
      QTextStream ts(&f);
      while (str = ts.readLine())
      {
	 str.replace(QRegExp("\\s+#.*"), ""); // removes everything from the whitespace before the first # to the end of the line
	 qsv.push_back(str);
      }
      f.close();
      return(read_modelSystem(ms, qsv, flag));
   }
   else
   {
      return(-40); // can't open input file
   }
}

int US_FemGlobal::read_modelSystem(vector<ModelSystem> *vms, QString filename)
{
   QString str;
   vector <QString> qsv;
   QFile f;
   f.setName(filename);
   ModelSystem ms;
   int retval = -1;
   vms->clear();
   vector<unsigned int> offset;
   unsigned int i;
   if (f.open(IO_ReadOnly | IO_Translate))
   {
      QTextStream ts(&f);
      while (str = ts.readLine())
      {
	 str.replace(QRegExp("\\s+#.*"), ""); // removes everything from the whitespace before the first # to the end of the line
	 qsv.push_back(str);
	 if (str == "#!__Begin_ModelSystem__!")
	 {
	    offset.push_back(qsv.size());
	 }
      }
      f.close();
      if (offset.size())
      {
	 for (i = 0; i < offset.size(); i++)
	 {
	    retval = read_modelSystem(&ms, qsv, false, offset[i]);
	    if (retval < 0)
	    {
	       cerr << "Reading modelsystem " << i << " failed with return value: " << retval << endl;
	    }
	    vms->push_back(ms);
	    if (retval)
	    {
	       return(retval);
	    }
	 }
      }
      else
      {
	 retval = read_modelSystem(&ms, qsv, false, 0);
	 if (retval < 0)
	 {
	    cerr << "Reading modelsystem failed with return value: " << retval << endl;
	    return(retval);
	 }
	 vms->push_back(ms);
      }
      return(retval);
   }
   else
   {
      return(-40); // can't open input file
   }
}

int US_FemGlobal::read_modelSystem(struct ModelSystem *ms, vector <QString> qsv, bool flag, int offset)
{
   QString str="";
   unsigned int i, j;
   unsigned int pos = 0;
   bool ok;
   if (flag) // if we are reading this model as part of a different file, we need
   { // to find the beginning point first so the file is correctly off-set:
      while (str != "#!__Begin_ModelSystem__!" && pos < qsv.size())
      {
	 str = qsv[pos++];
      }
   }
   if (offset) // we are given an offset position to start in the string
   {
      pos = offset;
   }
   printf("read_modelsystem pos = %d\n", pos);
   printf("qsv[%d]=%s\n", pos, qsv[pos].ascii()); fflush(stdout);
   pos++; // FE, SA2D, COFS, SIM or GA
   if (pos >= qsv.size()) return -1;
   (*ms).description = qsv[pos++];
   if ((*ms).description.isNull())
   {
      return(-100);
   }
   if (pos >= qsv.size()) return -1;
   str = qsv[pos++];
   if (str.find("#", 0, true) == 0) // a new model has a comment line in the second line starting with "#"
   {
      float fval;
      if (pos >= qsv.size()) return -1;
      fval = qsv[pos++].toDouble(&ok); // UltraScan version
      if (pos >= qsv.size()) return -2;
      str = qsv[pos++];
      if (str.isNull())
      {
	 return(-2);
      }
      (*ms).model = str.toInt();
      if (pos >= qsv.size()) return -3;
      str = qsv[pos++];
      if (str.isNull())
      {
	 return(-3);
      }
      (*ms).component_vector.resize(str.toInt());
      for (i=0; i<(*ms).component_vector.size(); i++)
      {
	 if (pos >= qsv.size()) return -3;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-4);
	 }
	 // the qsv vector has these already stripped off..
	 // int lpos = str.find("#", 0, true);
	 // str.truncate(lpos);
	 (*ms).component_vector[i].name = str.stripWhiteSpace();
	 if (pos >= qsv.size()) return -5;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-5);
	 }
	 (*ms).component_vector[i].concentration = str.toDouble();
	 if (pos >= qsv.size()) return -6;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-6);
	 }
	 (*ms).component_vector[i].s = str.toDouble();
	 if (pos >= qsv.size()) return -7;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-7);
	 }
	 (*ms).component_vector[i].D = str.toDouble();
	 if (pos >= qsv.size()) return -8;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-8);
	 }
	 (*ms).component_vector[i].sigma = str.toDouble();
	 if (pos >= qsv.size()) return -9;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-9);
	 }
	 (*ms).component_vector[i].delta = str.toDouble();
	 if (pos >= qsv.size()) return -10;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-10);
	 }
	 (*ms).component_vector[i].mw = str.toDouble();
	 if (pos >= qsv.size()) return -11;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-11);
	 }
	 (*ms).component_vector[i].vbar20 = str.toDouble();
	 if (pos >= qsv.size()) return -12;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-12);
	 }
	 (*ms).component_vector[i].shape = str;
	 if (pos >= qsv.size()) return -13;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-13);
	 }
	 (*ms).component_vector[i].f_f0 = str.toDouble();
	 if (pos >= qsv.size()) return -14;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-14);
	 }
	 (*ms).component_vector[i].extinction = str.toDouble();
	 if (pos >= qsv.size()) return -15;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-15);
	 }
	 (*ms).component_vector[i].show_conc = (bool) str.toInt();
	 if (pos >= qsv.size()) return -16;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-16);
	 }
	 (*ms).component_vector[i].show_stoich = str.toInt();
	 if (pos >= qsv.size()) return -17;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-17);
	 }
	 (*ms).component_vector[i].show_keq = (bool) str.toInt();
	 if (pos >= qsv.size()) return -18;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-18);
	 }
	 (*ms).component_vector[i].show_koff = (bool) str.toInt();
	 if (pos >= qsv.size()) return -19;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-19);
	 }
	 (*ms).component_vector[i].show_component.resize(str.toUInt());
	 for (j=0; j<(*ms).component_vector[i].show_component.size(); j++)
	 {
	    if (pos >= qsv.size()) return -20;
	    str = qsv[pos++];
	    if (str.isNull())
	    {
	       return(-20);
	    }
	    (*ms).component_vector[i].show_component[j] = str.toInt();
	 }
	 if ((*ms).component_vector[i].concentration < 0)
	 {
	    (*ms).component_vector[i].c0.radius.clear();
	    (*ms).component_vector[i].c0.concentration.clear();
	    if (pos >= qsv.size()) return -21;
	    str = qsv[pos++];
	    if (str.isNull())
	    {
	       return(-21);
	    }
	    unsigned int ival = str.toUInt();
	    for (j=0; j<ival; j++)
	    {
	       if (pos >= qsv.size()) return -22;
	       str = qsv[pos++];
	       if (str.isNull())
	       {
		  return(-22);
	       }
	       (*ms).component_vector[i].c0.radius.push_back(str.toDouble());
	       if (pos >= qsv.size()) return -23;
	       str = qsv[pos++];
	       if (str.isNull())
	       {
		  return(-23);
	       }
	       (*ms).component_vector[i].c0.concentration.push_back(str.toDouble());
	    }
	 }
      }
      if (pos >= qsv.size()) return -24;
      str = qsv[pos++];
      if (str.isNull())
      {
	 return(-24);
      }
      (*ms).assoc_vector.resize(str.toUInt());
      for (i=0; i<(*ms).assoc_vector.size(); i++)
      {
	 if (pos >= qsv.size()) return -25;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-25);
	 }
	 (*ms).assoc_vector[i].keq = str.toDouble();
	 if (pos >= qsv.size()) return -26;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-26);
	 }
	 (*ms).assoc_vector[i].units = str;
	 if (pos >= qsv.size()) return -27;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-27);
	 }
	 (*ms).assoc_vector[i].k_off = str.toDouble();
	 if (pos >= qsv.size()) return -28;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-28);
	 }
	 (*ms).assoc_vector[i].component1 = str.toInt();
	 if (pos >= qsv.size()) return -29;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-29);
	 }
	 (*ms).assoc_vector[i].component2 = str.toInt();
	 if (pos >= qsv.size()) return -30;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-30);
	 }
	 (*ms).assoc_vector[i].component3 = str.toInt();
	 if (pos >= qsv.size()) return -31;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-31);
	 }
	 (*ms).assoc_vector[i].stoichiometry1 = str.toUInt();
	 if (pos >= qsv.size()) return -32;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-32);
	 }
	 (*ms).assoc_vector[i].stoichiometry2 = str.toUInt();
	 if (pos >= qsv.size()) return -33;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-33);
	 }
	 (*ms).assoc_vector[i].stoichiometry3 = str.toUInt();
      }
      return(0);
   }
   else // load an old-style model file for noninteracting models
   {
      (*ms).model = str.toInt();
      if ((*ms).model > 3) // we can only read noninteracting models
	 (*ms).model = 3; // set to fixed molecular weight distribution by default
      if (pos >= qsv.size()) return -34;
      str = qsv[pos++];
      if (str.isNull())
      {
	 return(-34);
      }
      (*ms).component_vector.resize(str.toInt()); // number of components
      for (i=0; i<(*ms).component_vector.size(); i++)
      {
	 if (pos >= qsv.size()) return -35;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-35);
	 }
	 (*ms).component_vector[i].concentration = str.toDouble();
	 if (pos >= qsv.size()) return -36;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-36);
	 }
	 (*ms).component_vector[i].s = str.toDouble();
	 if (pos >= qsv.size()) return -37;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-37);
	 }
	 (*ms).component_vector[i].D = str.toDouble();
	 if (pos >= qsv.size()) return -38;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-38);
	 }
	 (*ms).component_vector[i].sigma = str.toDouble();
	 if (pos >= qsv.size()) return -39;
	 str = qsv[pos++];
	 if (str.isNull())
	 {
	    return(-39);
	 }
	 (*ms).component_vector[i].delta = str.toDouble();
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
   return(1); // loaded an old-style model
}


int US_FemGlobal::write_modelSystem(struct ModelSystem *ms, QString filename, bool flag)
{
   QFile f(filename);
   QString str;
   unsigned int i, j;
   if (flag)
   {
      if (!f.open(IO_WriteOnly | IO_Translate | IO_Append))
      {
	 return -35;
      }
   }
   else
   {
      if (!f.open(IO_WriteOnly | IO_Translate))
      {
	 return -35;
      }
   }
   QTextStream ts(&f);
   if (flag)
   {
      ts << "#!__Begin_ModelSystem__!\n";
   }
   ts << "SIM" << "\n";
   ts << "Model written by US_FEMGLOBAL\n";
   ts << "# This file is computer-generated, please do not edit unless you know what you are doing\n";
#if defined(USE_MPI)
   ts << US_Version_string << "\t\t# UltraScan Version Number\n";
#else
   ts << US_Version << "\t\t# UltraScan Version Number\n";
#endif
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

int US_FemGlobal::read_simulationParameters(struct SimulationParameters *sp, QString filename)
{
   QString str;
   vector <QString> qsv;
   QFile f;
   f.setName(filename);
   cout << "FILE NAME: " << filename << endl;
   if (f.open(IO_ReadOnly | IO_Translate))
   {
      QTextStream ts(&f);
      while (str = ts.readLine())
      {
	 str.replace(QRegExp("\\s+#.*"), ""); // removes everything from the whitespace before the first # to the end of the line
	 qsv.push_back(str);
      }
      f.close();
      return(read_simulationParameters(sp, qsv));
   }
   else
   {
      return(-72); // can't open input file
   }
}

int US_FemGlobal::read_simulationParameters(struct SimulationParameters *sp, vector <QString> qsv)
{
   int ival;
   unsigned int pos = 0;
   bool ok;
   if (pos < qsv.size())
   {
      unsigned int ival;
      ival = qsv[pos++].toUInt(&ok);
      (*sp).speed_step.resize(ival);
   }
   else
   {
      return (-51);
   }
   for (unsigned int i=0; i<(*sp).speed_step.size(); i++)
   {
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].duration_hours = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-52);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].duration_minutes = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-53);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].delay_hours = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-54);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].delay_minutes = qsv[pos++].toDouble(&ok);
      }
      else
      {
	 return(-55);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].rotorspeed = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-56);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].acceleration = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-57);
      }
      if (pos < qsv.size())
      {
	 int ival;
	 ival = qsv[pos++].toInt(&ok);
	 (*sp).speed_step[i].acceleration_flag = ival;
      }
      else
      {
	 return(-58);
      }
      if (pos < qsv.size())
      {
	 (*sp).speed_step[i].scans = qsv[pos++].toUInt(&ok);
      }
      else
      {
	 return(-59);
      }
   }
   if (pos < qsv.size())
   {
      (*sp).simpoints = qsv[pos++].toUInt(&ok);
   }
   else
   {
      return(-60);
   }
   if (pos < qsv.size())
   {
      (*sp).radial_resolution = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-61);
   }
   if (pos < qsv.size())
   {
      (*sp).meniscus = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-62);
   }
   if (pos < qsv.size())
   {
      (*sp).bottom = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-63);
   }
   if (pos < qsv.size())
   {
      (*sp).rnoise = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-64);
   }
   if (pos < qsv.size())
   {
      (*sp).tinoise = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-65);
   }
   if (pos < qsv.size())
   {
      (*sp).rinoise = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-66);
   }
   if (pos < qsv.size())
   {
      (*sp).mesh = qsv[pos++].toUInt(&ok);
   }
   else
   {
      return(-67);
   }
   if (pos < qsv.size())
   {
      (*sp).moving_grid = qsv[pos++].toInt(&ok);
   }
   else
   {
      return(-68);
   }
   if (pos < qsv.size())
   {
      (*sp).rotor = qsv[pos++].toInt(&ok);
   }
   else
   {
      return(-70);
   }
   if (pos < qsv.size())
   {
      ival = qsv[pos++].toInt(&ok);
      if (ival == 1)
      {
	 (*sp).band_forming = true;
      }
      else
      {
	 (*sp).band_forming = false;
      }
   }
   if (pos < qsv.size())
   {
      (*sp).band_volume = qsv[pos++].toDouble(&ok);
   }
   else
   {
      return(-71);
   }
   return(0);
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
      ts << (*sp).tinoise << "\t\t# time invariant systematic noise (in percent OD)" << "\n";
      ts << (*sp).rinoise << "\t\t# radial invariant systematic noise (in percent OD)" << "\n";
      ts << (*sp).mesh << "\t\t# radial grid" << "(0 = adaptive (ASTFEM), 1 = fixed (Claverie), 2 = moving hat (sedfit), 3 = file: $ULTRASCAN/mesh.dat)\n";
      ts << (*sp).moving_grid << "\t\t# time grid (0 = fixed (Claverie), 1 = adaptive (ASTFEM)" << "\n";
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
      ts << (*sp).band_volume << "\t\t# band loading volume\n";
      f.close();
      return(0);
   }
   else
   {
      return(-1000);
   }
}

int US_FemGlobal::read_experiment(struct ModelSystem *ms, struct SimulationParameters *sp, QString filename)
{
   QString str;
   int flag1, flag2;
   QFile f;
   f.setName(filename);
   printf("read exp ms trying to open %s\n", filename.ascii());
   if (filename.contains("us_system") && f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      ts >> str;
      flag1 = read_simulationParameters(sp, str);
      ts >> str;
      flag2 = read_modelSystem(ms, str);
      f.close();
      if (flag1 < 0)
      {
	 cerr << filename << ", couldn't read simulation parameters..."<<endl;
	 return (flag1);
      }
      if (flag2 < 0)
      {
	 cerr << filename << ", couldn't read models..."<<endl;
	 return (flag2);
      }
      return(0);
   }
   else if (filename.contains("model") && f.open(IO_ReadOnly))
   {
      QFileInfo fi(filename);
#ifdef WIN32
      str = fi.dirPath() + "\\" + fi.baseName() + "." + fi.fileName().right(2) + ".simulation_parameters";
#else
      str = fi.dirPath() + "/" + fi.baseName() + "." + fi.fileName().right(2) + ".simulation_parameters";
#endif
      flag1 = read_simulationParameters(sp, str);
      flag2 = read_modelSystem(ms, filename);
      if (flag1 < 0)
      {
	 cerr << filename << ", couldn't read simulation parameters..."<<endl;
	 return (flag1);
      }
      if (flag2 < 0)
      {
	 cerr << filename << ", couldn't read models..."<<endl;
	 return (flag2);
      }
      return(0);
   }
   else
   {
      return(-200); // can't read input file
   }
}

int US_FemGlobal::read_experiment(vector <struct ModelSystem> *vms, struct SimulationParameters *sp, QString filename)
{
   QString str;
   int flag1, flag2;
   QFile f;
   f.setName(filename);
   printf("read exp msv trying to open %s\n", filename.ascii());
   if (filename.contains("us_system") && f.open(IO_ReadOnly))
   {
      QTextStream ts(&f);
      ts >> str;
      flag1 = read_simulationParameters(sp, str);
      ts >> str;
      flag2 = read_modelSystem(vms, str);
      f.close();
      if (flag1 < 0)
      {
	 cerr << filename << ", couldn't read simulation parameters..."<<endl;
	 return (flag1);
      }
      if (flag2 < 0)
      {
	 cerr << filename << ", couldn't read models..."<<endl;
	 return (flag2);
      }
      return(0);
   }
   else if (filename.contains("model") && f.open(IO_ReadOnly))
   {
      QFileInfo fi(filename);
#ifdef WIN32
      str = fi.dirPath() + "\\" + fi.baseName() + "." + fi.fileName().right(2) + ".simulation_parameters";
#else
      str = fi.dirPath() + "/" + fi.baseName() + "." + fi.fileName().right(2) + ".simulation_parameters";
#endif
      flag1 = read_simulationParameters(sp, str);
      flag2 = read_modelSystem(vms, filename);
      if (flag1 < 0)
      {
	 cerr << filename << ", couldn't read simulation parameters..."<<endl;
	 return (flag1);
      }
      if (flag2 < 0)
      {
	 cerr << filename << ", couldn't read models..."<<endl;
	 return (flag2);
      }
      return(0);
   }
   else
   {
      return(-200); // can't read input file
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
      return(-300); // can't open output file
   }
}

// read a model system, and the associated constraints needed for initialization of the fitting process.
// all associated files should start with the "filename" string

int US_FemGlobal::read_constraints(struct ModelSystem *ms, struct ModelSystemConstraints *msc, QString filename)
{
   QString str;
   vector <QString> qsv;
   QFile f;
   f.setName(filename);
   if (f.open(IO_ReadOnly | IO_Translate))
   {
      QTextStream ts(&f);
      while (str = ts.readLine())
      {
	 str.replace(QRegExp("\\s+#.*"), ""); // removes everything from the whitespace before the first # to the end of the line
	 qsv.push_back(str);
      }
      f.close();
      return(read_constraints(ms, msc, qsv));
   }
   else
   {
      return(-200); // can't open input file
   }
}

int US_FemGlobal::read_constraints(struct ModelSystem *ms, struct ModelSystemConstraints *msc, vector <QString> qsv)
{
   QString str;
   unsigned int i, j, k;
   unsigned int pos = 0;
   bool ok;
   if (pos >= qsv.size()) return -2;
   j = qsv[pos++].toUInt(&ok);
   (*msc).component_vector_constraints.resize(j);
   for (i=0; i<j; i++)
   {
      if (pos >= qsv.size()) return -3;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].vbar20.fit = k;
      if (pos >= qsv.size()) return -4;
      (*msc).component_vector_constraints[i].vbar20.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -5;
      (*msc).component_vector_constraints[i].vbar20.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -6;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].mw.fit = k;
      if (pos >= qsv.size()) return -7;
      (*msc).component_vector_constraints[i].mw.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -8;
      (*msc).component_vector_constraints[i].mw.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -9;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].s.fit = k;
      if (pos >= qsv.size()) return -10;
      (*msc).component_vector_constraints[i].s.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -11;
      (*msc).component_vector_constraints[i].s.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -12;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].D.fit = k;
      if (pos >= qsv.size()) return -13;
      (*msc).component_vector_constraints[i].D.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -14;
      (*msc).component_vector_constraints[i].D.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -15;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].sigma.fit = k;
      if (pos >= qsv.size()) return -16;
      (*msc).component_vector_constraints[i].sigma.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -17;
      (*msc).component_vector_constraints[i].sigma.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -18;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].delta.fit = k;
      if (pos >= qsv.size()) return -19;
      (*msc).component_vector_constraints[i].delta.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -20;
      (*msc).component_vector_constraints[i].delta.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -21;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].concentration.fit = k;
      if (pos >= qsv.size()) return -22;
      (*msc).component_vector_constraints[i].concentration.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -23;
      (*msc).component_vector_constraints[i].concentration.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -24;
      k = qsv[pos++].toUInt(&ok);
      (*msc).component_vector_constraints[i].f_f0.fit = k;
      if (pos >= qsv.size()) return -25;
      (*msc).component_vector_constraints[i].f_f0.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -26;
      (*msc).component_vector_constraints[i].f_f0.high = qsv[pos++].toDouble(&ok);
   }
   if (pos >= qsv.size()) return -27;
   j = qsv[pos++].toUInt(&ok); // j is the number of reactions in this model
   (*msc).assoc_vector_constraints.resize(j);
   for (i=0; i<j; i++)
   {
      if (pos >= qsv.size()) return -28;
      k = qsv[pos++].toUInt(&ok);
      (*msc).assoc_vector_constraints[i].keq.fit = k;
      if (pos >= qsv.size()) return -29;
      (*msc).assoc_vector_constraints[i].keq.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -30;
      (*msc).assoc_vector_constraints[i].keq.high = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -31;
      k = qsv[pos++].toUInt(&ok);
      (*msc).assoc_vector_constraints[i].koff.fit = k;
      if (pos >= qsv.size()) return -32;
      (*msc).assoc_vector_constraints[i].koff.low = qsv[pos++].toDouble(&ok);
      if (pos >= qsv.size()) return -33;
      (*msc).assoc_vector_constraints[i].koff.high = qsv[pos++].toDouble(&ok);
   }
   if (pos >= qsv.size()) return -34;
   (*msc).simpoints = qsv[pos++].toUInt(&ok);
   if (pos >= qsv.size()) return -35;
   (*msc).mesh = qsv[pos++].toUInt(&ok);
   if (pos >= qsv.size()) return -36;
   (*msc).moving_grid = qsv[pos++].toInt(&ok);
   if (pos >= qsv.size()) return -37;
   (*msc).band_volume = qsv[pos++].toDouble(&ok);

   int flag1 = read_modelSystem(ms, qsv, true);
   if (flag1 < 0)
   {
      return (flag1);
   }
   return(0);
}

// write a model system, and the associated constraints needed for initialization of the fitting process.
// all associated files should start with the "filename" string

int US_FemGlobal::write_constraints(struct ModelSystem *ms, struct ModelSystemConstraints *msc, QString filename)
{
   QFile f;
   //	QFileInfo fi(filename);
   //	filename = fi.fileName(); // strip leading path
   QString str;
   unsigned int i;
   if (filename.right(12) != ".constraints")
   {
      filename += ".constraints";
   }
   f.setName(filename);
   if (f.open(IO_WriteOnly | IO_Translate))
   {
      QTextStream ts(&f);
      //		str.sprintf(filename + ".constraints.model-%d.00", (*ms).model);
      //		ts << str << endl;
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
      write_modelSystem(ms, filename, true); // append the corresponding model to the constraints file
      str.sprintf(filename + ".model-%d.00", (*ms).model);
      write_modelSystem(ms, str); // write the corresponding model to a separate file
      return(0);
   }
   else
   {
      return(-400); // can't open output file
   }
   return 0;
}

int US_FemGlobal::read_model_data(vector <mfem_data> *model, QString filename, bool ignore_errors) {
   unsigned int no_of_models;
   unsigned int no_of_radial_points;
   unsigned int no_of_scans;

   // short int short_int_val;
   double double_val;

   QFile f(filename);
   if (!f.open(IO_ReadOnly))
   {
      if (!ignore_errors)
      {
	 cout << "Could not open data file: " << filename << " for input\n";
	 cout << "Please check the path, file name and read permissions...\n\n";
      }
      return (-1);
   }
   (*model).clear();
   struct mfem_data temp_model;
   struct mfem_scan temp_scan;
   vector <double> concentration;

   unsigned int i;
   unsigned int j;
   unsigned int k;

   QDataStream ds(&f);

   ds >> no_of_models;
   cout << "no of models: " << no_of_models << endl;
   for (i = 0; i < no_of_models; i++)
   {
      ds >> temp_model.id;
      ds >> temp_model.cell;
      ds >> temp_model.channel;
      ds >> temp_model.wavelength;
      ds >> temp_model.meniscus;
      ds >> temp_model.bottom;
      ds >> temp_model.rpm;
      ds >> temp_model.s20w_correction;
      ds >> temp_model.D20w_correction;
      ds >> no_of_radial_points;
      temp_model.radius.clear();
      for (j = 0; j < no_of_radial_points; j++)
      {
	 ds >> double_val;
	 temp_model.radius.push_back(double_val);
      }
      ds >> no_of_scans;
      temp_model.scan.clear();
      for (j = 0; j < no_of_scans; j++)
      {
	 ds >> temp_scan.time;
	 ds >> temp_scan.omega_s_t;
	 temp_scan.conc.clear();
	 for (k = 0; k < temp_model.radius.size(); k++)
	 {
	    ds >> double_val;
	    temp_scan.conc.push_back(double_val);
	    // ds >> short_int_val;
	    // temp_scan.ignore.push_back(short_int_val);
	 }
	 temp_model.scan.push_back(temp_scan);
      }
      ds >> temp_model.viscosity;
      ds >> temp_model.density;
      ds >> temp_model.vbar;
      ds >> temp_model.vbar20;
      ds >> temp_model.avg_temperature;
#if defined(DEBUG_HYDRO)

      printf("model time %g avg_temp %.12g vbar %.12g vbar20 %.12g visc %.12g density %.12g\n",
	     temp_model.scan[temp_model.scan.size()-1].time,
	     temp_model.avg_temperature,
	     temp_model.vbar,
	     temp_model.vbar20,
	     temp_model.viscosity,
	     temp_model.density);

#endif
      (*model).push_back(temp_model);
   }
   return 0;
}

int US_FemGlobal::write_model_data(vector <mfem_data> *model, QString filename) {
   unsigned int i;
   unsigned int j;
   unsigned int k;
   QFile f(filename);
   if (!f.open(IO_WriteOnly))
   {
      cout << "Could not open data file: " << filename << " for output\n";
      cout << "Please check the path, file name and write permissions...\n\n";
      return (-1);
   }

   QDataStream ds(&f);
   ds << (unsigned int)(*model).size();
   for (i=0; i<(*model).size(); i++)
   {
      ds << (*model)[i].id;
      ds << (*model)[i].cell;
      ds << (*model)[i].channel;
      ds << (*model)[i].wavelength;
      ds << (*model)[i].meniscus;
      ds << (*model)[i].bottom;
      ds << (*model)[i].rpm;
      ds << (*model)[i].s20w_correction;
      ds << (*model)[i].D20w_correction;
      ds << (unsigned int) (*model)[i].radius.size();
      for (j=0; j<(*model)[i].radius.size(); j++)
      {
	 ds << (*model)[i].radius[j];
      }
      ds << (unsigned int)(*model)[i].scan.size();

      for (j=0; j<(*model)[i].scan.size(); j++)
      {
	 ds << (*model)[i].scan[j].time;
	 ds << (*model)[i].scan[j].omega_s_t;
	 for (k=0; k<(*model)[i].radius.size(); k++)
	 {
	    ds << (*model)[i].scan[j].conc[k];
	    // ds << (*model)[i].scan[j].ignore[k];
	 }
      }

      fprintf(stderr,"model last time %g avg_temp %g vbar %g visc %g density %g vbar %g vbar20 %g rpm %u bottom %g meniscus %g scorr %g Dcorr %g\n",
	      (*model)[i].scan[(*model)[i].scan.size()-1].time,
	      (*model)[i].avg_temperature,
	      (*model)[i].vbar,
	      (*model)[i].viscosity,
	      (*model)[i].density,
	      (*model)[i].vbar,
	      (*model)[i].vbar20,
	      (*model)[i].rpm,
	      (*model)[i].bottom,
	      (*model)[i].meniscus,
	      (*model)[i].s20w_correction,
	      (*model)[i].D20w_correction);
      ds << (*model)[i].viscosity;
      ds << (*model)[i].density;
      ds << (*model)[i].vbar;
      ds << (*model)[i].vbar20;
      ds << (*model)[i].avg_temperature;
   }
   f.close();
   return 0;
}

int US_FemGlobal::write_ascii_model_data(vector <mfem_data> *model, QString filename) {
   unsigned int i;
   unsigned int j;
   unsigned int k;
   QFile f(filename);
   if (!f.open(IO_WriteOnly))
   {
      cout << "Could not open data file: " << filename << " for output\n";
      cout << "Please check the path, file name and write permissions...\n\n";
      return (-1);
   }

   QTextStream ts(&f);
   ts << "# Ascii dump of experimental data\n";
   ts << (unsigned int)(*model).size() << "\t# number of models\n";
   for (i=0; i<(*model).size(); i++)
   {
      ts << (*model)[i].id << "\t# id\n";
      ts << (*model)[i].cell << "\t# cell\n";
      ts << (*model)[i].channel << "\t# channel\n";
      ts << (*model)[i].wavelength << "\t# wavelength\n";
      ts << (*model)[i].meniscus << "\t# meniscus\n";
      ts << (*model)[i].bottom << "\t# bottom\n";
      ts << (*model)[i].rpm << "\t# rpm\n";
      ts << (*model)[i].s20w_correction << "\t# s20w_correction\n";
      ts << (*model)[i].D20w_correction << "\t# D20w_correction\n";
      ts << (unsigned int) (*model)[i].radius.size() << "\t# radius.size()\n";
      for (j=0; j<(*model)[i].radius.size(); j++)
      {
	 ts << (*model)[i].radius[j] << "\t# radius[j]\n";
      }
      ts << (unsigned int)(*model)[i].scan.size() << "\t# scan.size()\n";

      for (j=0; j<(*model)[i].scan.size(); j++)
      {
	 ts << (*model)[i].scan[j].time << "\t# time\n";
	 ts << (*model)[i].scan[j].omega_s_t << "\t# omega_s_t\n";
	 for (k=0; k<(*model)[i].radius.size(); k++)
	 {
	    ts << (*model)[i].scan[j].conc[k] << "\t# conc[k]\n";
	    // ts << (*model)[i].scan[j].ignore[k] << "\t# ignore[k]\n";
	 }
      }

      fprintf(stderr,"model last time %g avg_temp %g vbar %g visc %g density %g vbar %g vbar20 %g rpm %u bottom %g meniscus %g scorr %g Dcorr %g\n",
	      (*model)[i].scan[(*model)[i].scan.size()-1].time,
	      (*model)[i].avg_temperature,
	      (*model)[i].vbar,
	      (*model)[i].viscosity,
	      (*model)[i].density,
	      (*model)[i].vbar,
	      (*model)[i].vbar20,
	      (*model)[i].rpm,
	      (*model)[i].bottom,
	      (*model)[i].meniscus,
	      (*model)[i].s20w_correction,
	      (*model)[i].D20w_correction);
      ts << (*model)[i].viscosity << "\t# viscosity\n";
      ts << (*model)[i].density << "\t# density\n";
      ts << (*model)[i].vbar << "\t# vbar\n";
      ts << (*model)[i].vbar20 << "\t# vbar20\n";
      ts << (*model)[i].avg_temperature << "\t# avg_temperature\n";
   }
   f.close();
   return 0;
}

int US_FemGlobal::accumulate_model_monte_carlo_data(vector <mfem_data> *accumulated_model, vector <mfem_data> *source_model, unsigned int monte_carlo_iterations)
{
   if (!monte_carlo_iterations)
   {
      cout << "Notice: US_FemGlobal::accumulate_model_monte_carlo_data monte carlo iterations == 0, setting to 1\n";
      monte_carlo_iterations = 1;
   }

   if ((*accumulated_model).size() &&
       (*accumulated_model).size() != (*source_model).size())
   {
      cout << "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data model size incompability\n";
      return (-2);
   }

   unsigned int i;
   unsigned int j;
   unsigned int k;

   if (!(*accumulated_model).size())
   {
      struct mfem_data temp_model;
      for (i = 0; i < (*source_model).size(); i++)
      {
	 temp_model = (*source_model)[i];
	 for (j=0; j<temp_model.scan.size(); j++)
	 {
	    for (k=0; k<temp_model.radius.size(); k++)
	    {
	       temp_model.scan[j].conc[k] /= (double) monte_carlo_iterations;
	    }
	 }
	 accumulated_model->push_back(temp_model);
      }
      return 0;
   }

   for (i=0; i<(*source_model).size(); i++)
   {
      if ((*accumulated_model)[i].scan.size() != (*source_model)[i].scan.size())
      {
	 cout << "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data scan size incompability\n";
	 return (-3);
      }
      if ((*accumulated_model)[i].radius.size() != (*source_model)[i].radius.size())
      {
	 cout << "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data radius size incompability\n";
	 return (-4);
      }
      if ((*accumulated_model)[i].radius != (*source_model)[i].radius)
      {
	 cout << "Internal Error: US_FemGlobal::accumulate_model_monte_carlo_data radius value incompability\n";
	 return (-5);
      }
      for (j=0; j<(*source_model)[i].scan.size(); j++)
      {
	 for (k=0; k<(*source_model)[i].radius.size(); k++)
	 {
	    (*accumulated_model)[i].scan[j].conc[k] += (*source_model)[i].scan[j].conc[k] / (double) monte_carlo_iterations;
	 }
      }
   }
   return 0;
}

int US_FemGlobal::read_mwl_model_data(vector <mfem_data> *model, QString filenamebase)
{
   unsigned int i;
   unsigned int no_of_models_loaded = 0;
   vector <mfem_data> temp_model;
   for (i = 0; i < 2048; i++)
   {
      if (!read_model_data(&temp_model, filenamebase + QString("-model-%1.dat").arg(i), true))
      {
	 model->push_back(temp_model[0]);
	 no_of_models_loaded++;
      }
   }
   return no_of_models_loaded;
}

int US_FemGlobal::convert_analysis_data(QString infile, QString outfile)
{
   QFile f(infile);
   if (!f.open(IO_ReadOnly))
   {
      cout << tr("Could not open data file: ") << infile <<
	 tr(" for input\n");
      cout <<
	 tr
	 ("Please check the path, file name and read permissions...\n\n");
      return -1;
   }
   QFile f2(outfile);
   if (!f2.open(IO_WriteOnly))
   {
      cout << tr("Could not open data file: ") << infile <<
	 tr(" for output\n");
      cout <<
	 tr
	 ("Please check the path, file name and read permissions...\n\n");
      return -2;
   }
   QDataStream ds(&f);
   QTextStream ts(&f2);
   ts << "# ascii format of analysis data .dat file\n";

   QString analysis_type;
   QString str1;
   unsigned int count1;
   unsigned int unsigned_int_val;
   // short int short_int_val;
   double double_val;
   int int_val;
   float float_val;
   
   ds >> str1;
   ts << str1 << "\t# email\n";
   ds >> analysis_type;
   if (analysis_type == "SA2D")
   {
      analysis_type = "2DSA";
   }
   ts << analysis_type << "\t# analysis_type\n";
   ds >> int_val;
   ts << int_val << "\t# fit_tinoise\n";
   ds >> int_val;
   ts << int_val << "\t# fit_rinoise\n";
   ds >> int_val;
   ts << int_val << "\t# union_results\n";
   ds >> float_val;
   ts << float_val << "\t# meniscus range\n";
   ds >> int_val;
   ts << int_val << "\t# fit_meniscus\n";
   ds >> unsigned_int_val;
   ts << unsigned_int_val << "\t# meniscus_gridpoints\n";
   ds >> int_val;
   ts << int_val << "\t# use_iterative\n";
   ds >> unsigned_int_val;
   ts << unsigned_int_val << "\t# max_iterations\n";
   ds >> float_val;
   ts << float_val << "\t# regularization\n";
   ds >> unsigned_int_val;
   ts << unsigned_int_val << "\t# no of experiments\n";
   unsigned int no_of_exps = unsigned_int_val;
   if (analysis_type == "2DSA" ||
       analysis_type == "2DSA_RA" ||
       analysis_type == "2DSA_RA_MWL")
   {
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# monte_carlo\n";
      ds >> float_val;
      ts << float_val << "\t#ff0_min\n";
      ds >> float_val;
      ts << float_val << "\t#ff0_max\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# ff0_resolution\n";
      ds >> float_val;
      ts << float_val << "\t# s_min\n";
      ds >> float_val;
      ts << float_val << "\t# s_max\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# s_resolution\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# uniform_grid_repetition\n";
      if (analysis_type == "2DSA_RA" ||
	  analysis_type == "2DSA_RA_MWL")
      {
	 if (analysis_type == "2DSA_RA_MWL") 
	 {
	    ds >> float_val;
	    ts << float_val << "\t#wavelength\n";
	 }
	 for(unsigned int e = 0; e < no_of_exps; e++) 
	 {
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# simpoints\n";
	    ds >> double_val;
	    ts << double_val << "\t# band_volume\n";
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# radial_grid\n";
	    ds >> int_val;
	    ts << int_val << "\t# moving_grid\n";
	    {
	       unsigned int i, j;
	       ds >> i;
	       ts << i << "\t# simulation_parameters lines\n";
	       for (j = 0; j < i; j++) 
	       {
		  ds >> str1;
		  ts << str1 << "\n";
	       }
	    }
	 }
      }
   }
   if (analysis_type == "2DSA_MW" ||
       analysis_type == "2DSA_MW_RA")
   {
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# monte_carlo\n";
      ds >> float_val;
      ts << float_val << "\t# ff0_min\n";
      ds >> float_val;
      ts << float_val << "\t# ff0_max\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# ff0_resolution\n";
      ds >> float_val;
      ts << float_val << "\t# mw_min\n";
      ds >> float_val;
      ts << float_val << "\t# mw_max\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# grid_resolution\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# uniform_grid_repetition\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# max_mer\n";

      if (analysis_type == "2DSA_MW_RA")
      {
	 for(unsigned int e = 0; e < no_of_exps; e++) 
	 {
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# simpoints\n";
	    ds >> double_val;
	    ts << double_val << "\t# band_volume\n";
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# radial_grid\n";
	    ds >> int_val;
	    ts << int_val << "\t# moving_grid\n";
	    {
	       unsigned int i, j;
	       ds >> i;
	       ts << i << "\t# simulation_parameters lines\n";
	       for (j = 0; j < i; j++) 
	       {
		  ds >> str1;
		  ts << str1 << "\n";
	       }
	    }
	 }
      }
   }
   if (analysis_type == "GA" ||
       analysis_type == "GA_RA" ||
       analysis_type == "GA_RA_MWL")
   {
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# monte_carlo\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# demes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# generations\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# crossover\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# mutation\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# plague\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# elitism\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# migration_rate\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# genes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# initial_solutes\n";
      unsigned int initial_solutes = unsigned_int_val;
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# random_seed\n";
      for (count1 = 0; count1 < initial_solutes; count1++)
      {
	 ds >> float_val;
	 ts << float_val << "\t# bucket.s\n";
	 ds >> float_val;
	 ts << float_val << "\t# bucket.s_min\n";
	 ds >> float_val;
	 ts << float_val << "\t# bucket.s_max\n";
	 ds >> float_val;
	 ts << float_val << "\t# bucket.ff0\n";
	 ds >> float_val;
	 ts << float_val << "\t# bucket.ff0_min\n";
	 ds >> float_val;
	 ts << float_val << "\t# bucket.ff0_max\n";
      }
      if (analysis_type == "GA_RA")
      {
	 for(unsigned int e = 0; e < no_of_exps; e++) 
	 {
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# simpoints\n";
	    ds >> double_val;
	    ts << double_val << "\t# band_volume\n";
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# radial_grid\n";
	    ds >> int_val;
	    ts << int_val << "\t# moving_grid\n";
	    {
	       unsigned int i, j;
	       ds >> i;
	       ts << i << "\t# simulation_parameters lines\n";
	       for (j = 0; j < i; j++) 
	       {
		  ds >> str1;
		  ts << str1 << "\n";
	       }
	    }
	 }
      }
      if (analysis_type == "GA_RA_MWL")
      {
	 ds >> int_val;
	 ts << int_val << "\t# MWL scans to test\n";
	 {
	    unsigned int i, j;
	    ds >> i;
	    ts << i << "\t# simulation_parameters lines\n";
	    for (j = 0; j < i; j++) 
	    {
	       ds >> str1;
	       ts << str1 << "\n";
	    }
	 }
      }
   }
   if (analysis_type == "GA_MW" ||
       analysis_type == "GA_MW_RA")
   {
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# monte_carlo\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# demes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# generations\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# crossover\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# mutation\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# plague\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# elitism\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# migration_rate\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# genes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# largest_oligomer\n";
      ds >> str1;
      ts << str1 << "\t# largest_oligomer_string\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# random_seed\n";
      ds >> float_val;
      ts << float_val << "\t# mw_min\n";
      ds >> float_val;
      ts << float_val << "\t# mw_max\n";
      ds >> float_val;
      ts << float_val << "\t# ff0_min\n";
      ds >> float_val;
      ts << float_val << "\t# ff0_max\n";

      if (analysis_type == "GA_MW_RA")
      {
	 for(unsigned int e = 0; e < no_of_exps; e++) 
	 {
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# simpoints\n";
	    ds >> double_val;
	    ts << double_val << "\t# band_volume\n";
	    ds >> unsigned_int_val;
	    ts << unsigned_int_val << "\t# radial_grid\n";
	    ds >> int_val;
	    ts << int_val << "\t# moving_grid\n";
	    {
	       unsigned int i, j;
	       ds >> i;
	       ts << i << "\t# simulation_parameters lines\n";
	       for (j = 0; j < i; j++) 
	       {
		  ds >> str1;
		  ts << str1 << "\n";
	       }
	    }
	 }
      }
   }
   if (analysis_type == "GA_SC")
   {
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# monte_carlo\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# demes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# generations\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# crossover\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# mutation\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# plague\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# elitism\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# migration_rate\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# genes\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# random_seed\n";

      {
	 unsigned int i, j;
	 ds >> i;
	 ts << i << "\t# contraints lines\n";
	 for (j = 0; j < i; j++) 
	 {
	    ds >> str1;
	    ts << str1 << "\n";
	 }
      }
      {
	 unsigned int i, j;
	 ds >> i;
	 ts << i << "\t# simulation_parameters lines\n";
	 for (j = 0; j < i; j++) 
	 {
	    ds >> str1;
	    ts << str1 << "\n";
	 }
      }
   }
   ts << "# Ascii dump of experimental data\n";
   ts << no_of_exps << "\t# number of models\n";
   for (unsigned int i = 0; i < no_of_exps; i++)
   {
      ds >> str1;
      ts << str1 << "\t# experiment.id\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# experiment.cell\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# experiment.channel\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# experiment.wavelength\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.meniscus\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.bottom\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# temp_experiment.rpm\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.s20w_correction\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.D20w_correction\n";
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# radius points\n";
      unsigned int radius_pts = unsigned_int_val;
      for (unsigned int j = 0; j < radius_pts; j++)
      {
	 ds >> double_val;
	 ts << double_val << "\t# radius value\n";
      }
      ds >> unsigned_int_val;
      ts << unsigned_int_val << "\t# scans\n";
      unsigned int scans = unsigned_int_val;
      for (unsigned int j = 0; j < scans; j++)
      {
	 ds >> double_val;
	 ts << double_val << "\t# scan.time\n";
	 ds >> double_val;
	 ts << double_val << "\t# scan.omega_t\n";
	 for (unsigned int k = 0; k < radius_pts; k++) 
	 {
	    ds >> double_val;
	    ts << double_val << "\t# conc[k]\n";
	    //	    ds >> short_int_val;
	    //	    ts << short_int_val << "\t# ignore[k]\n";
	 }
      }
      ds >> double_val;
      ts << double_val << "\t# experiment.viscosity\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.density\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.vbar\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.vbar20\n";
      ds >> double_val;
      ts << double_val << "\t# experiment.avg_temperature\n";
   }
   f.close();
   f2.close();
   return 0;
}
