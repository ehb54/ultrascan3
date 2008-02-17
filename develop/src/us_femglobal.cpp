#include "../include/us_femglobal.h"

US_FemGlobal::US_FemGlobal()
{
}

US_FemGlobal::~US_FemGlobal()
{
}

int US_FemGlobal::read(struct ModelSystem *ms, struct SimulationParameters *sp, QString filename)
{
	QFile f(filename);
	if (f.open(IO_ReadOnly))
	{
		return (0);
	}
	else
	{
		return (-1); // couldn't read file
	}
}

int US_FemGlobal::write(struct ModelSystem *ms, struct SimulationParameters *sp, QString filename)
{
	QFile f(filename);
	if (f.open(IO_ReadWrite))
	{
		return (0);
	}
	else
	{
		return (-2); // couldn't write file
	}
}

