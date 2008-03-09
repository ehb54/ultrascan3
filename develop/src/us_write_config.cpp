#include "../include/us_write_config.h"

US_Write_Config::US_Write_Config(QObject *parent, const char *name) : QObject(parent, name)
{
}

US_Write_Config::~US_Write_Config()
{
}

bool US_Write_Config::write_config(struct Config config_list)
{
	QDir temp_dir;
	temp_dir = config_list.help_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.help_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Help File Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	temp_dir = config_list.root_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.root_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Root Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	temp_dir = config_list.data_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.data_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Data Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	temp_dir = config_list.archive_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.archive_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Archive Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	temp_dir = config_list.system_dir;
	if (!temp_dir.exists())
	{
		QMessageBox::message(tr("Warning"), tr("The UltraScan System Directory could not be found!\n\nPlease check your settings!"));
		return(false);
	}
	temp_dir = config_list.html_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.html_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Reports Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	temp_dir = config_list.result_dir;
	if (!temp_dir.exists())
	{
		if (!temp_dir.mkdir(config_list.result_dir, true))
		{
			QMessageBox::message(tr("Warning"), tr("Could not create the Result Directory!\n\nPlease check your write permissions!"));
			return(false);
		}
	}
	QString rcfile;
#ifdef UNIX
	rcfile = (getenv("HOME"));
	if (rcfile != "/")
	{
		rcfile += "/";
	}
	rcfile.append("/.usrc");
#endif
#ifdef WIN32
	rcfile = "C:\\";
	rcfile.append("\\Program Files\\Ultrascan\\etc\\usrc");
#endif
	QFile f(rcfile);
	QTextStream ts (&f);
	if (f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts (&f);
		ts << US_Version << "\n";
		ts << config_list.browser << "\n";
		ts << config_list.tar << "\n";
		ts << config_list.gzip << "\n";
		ts << config_list.help_dir << "\n";
		ts << config_list.data_dir << "\n";
		ts << config_list.root_dir << "\n";
		ts << config_list.archive_dir << "\n";
		ts << config_list.result_dir << "\n";
		ts << config_list.beckman_bug << "\n";
		ts << config_list.temperature_tol << "\n";
		ts << config_list.html_dir << "\n";
		ts << config_list.system_dir << "\n";
		ts << config_list.fontFamily << "\n";
		ts << config_list.fontSize << "\n";
		ts << config_list.margin << "\n";
		ts << config_list.numThreads << "\n";
		f.close();
	}
	else
	{
		QMessageBox::message(tr("Warning"), tr("Could not open Configuration File!\n\nPlease check your write permissions!"));
		return(false);
	}
	return (true);
}


