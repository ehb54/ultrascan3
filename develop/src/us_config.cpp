#include "../include/us_util.h"
#include "../include/us_write_config.h"

//US_Config::US_Config(QWidget *parent, const char *name) : QFrame( parent, name)
US_Config::US_Config(QObject *parent, const char *name) : QObject (parent, name)
{
	US_Version = "9.5";

#ifdef UNIX
	home = (getenv("HOME"));
	if (home != "/")
	{
		home += "/";
	}
#endif
#ifdef WIN32
	home = "C:\\";
#endif
//cout << "Reading config...\n";
  	if (!read())
  	{
//cout << "Setting default...\n";
		setDefault();
		US_Write_Config *WConfig;
//cout << "writing default...\n";
  		WConfig = new US_Write_Config();
		WConfig->write_config(config_list);
  	}
	col_exists();
	setModelString();
}

US_Config::~US_Config()
{
}

bool US_Config::write_default_colors()
{
   QString  colfile;
   colfile = home.copy();
#ifdef UNIX
   colfile.append(".uscolors");
#endif
#ifdef WIN32
   colfile.append("Program Files/ultrascan/etc/uscolors");
#endif
   QFile f(colfile);
   color_defaults();
   if(f.open(IO_WriteOnly))
   {
      QDataStream ds(&f);
      ds << global_colors.pushb;
      ds << global_colors.frames;
      ds << global_colors.labels;
      ds << global_colors.pushb_active;
      ds << global_colors.dk_red;
      ds << global_colors.dk_green;
      ds << global_colors.plot;
      ds << global_colors.major_ticks;
      ds << global_colors.minor_ticks;
      ds << global_colors.gray;
      ds << global_colors.gray1;
      ds << global_colors.gray2;
      ds << global_colors.gray3;
      ds << global_colors.gray4;
      ds << global_colors.gray5;
      ds << global_colors.gray6;
      ds << global_colors.gray7;
      ds << global_colors.gray8;
      ds << global_colors.gray9;
      ds << global_colors.cg_frame;
      ds << global_colors.cg_pushb;
      ds << global_colors.cg_pushb_active;
      ds << global_colors.cg_pushb_disabled;
      ds << global_colors.cg_label;
      ds << global_colors.cg_label_disabled;
      ds << global_colors.cg_label_warn;
      ds << global_colors.cg_edit;
      ds << global_colors.cg_edit_warn;
      ds << global_colors.cg_dk_red;
      ds << global_colors.cg_dk_green;
      ds << global_colors.cg_red;
      ds << global_colors.cg_green;
      ds << global_colors.cg_gray;
      ds << global_colors.cg_normal;
      ds << global_colors.cg_plot;
      ds << global_colors.cg_lcd;
      ds << global_colors.cg_bunt;
      ds << US_Version;
      f.close();
      return(true);
   }
   else
   {
      return(false);
   }
}

void US_Config::setModelString()
{
	modelString.clear();
	modelString.push_back(tr("1-Component, Ideal"));																// model 0
	modelString.push_back(tr("2-Component, Ideal, Noninteracting"));											// model 1
	modelString.push_back(tr("3-Component, Ideal, Noninteracting"));											// model 2
	modelString.push_back(tr("Fixed Molecular Weight Distribution"));											// model 3
	modelString.push_back(tr("Monomer-Dimer Equilibrium"));														// model 4
	modelString.push_back(tr("Monomer-Trimer Equilibrium"));														// model 5
	modelString.push_back(tr("Monomer-Tetramer Equilibrium"));													// model 6
	modelString.push_back(tr("Monomer-Pentamer Equilibrium"));													// model 7
	modelString.push_back(tr("Monomer-Hexamer Equilibrium"));													// model 8
	modelString.push_back(tr("Monomer-Heptamer Equilibrium"));													// model 9
	modelString.push_back(tr("User-Defined Monomer-Nmer Equilibrium"));										// model 10
	modelString.push_back(tr("Monomer-Dimer-Trimer Equilibrium"));												// model 11
	modelString.push_back(tr("Monomer-Dimer-Tetramer Equilibrium"));											// model 12
	modelString.push_back(tr("User-Defined Monomer - N-mer - M-mer Equilibrium"));						// model 13
	modelString.push_back(tr("2-Component Hetero-Association: A + B <=> AB"));								// model 14
	modelString.push_back(tr("User-defined self/Hetero-Association: A + B <=> AB, nA <=> An"));		// model 15
	modelString.push_back(tr("User-Defined Monomer-Nmer, some monomer is incompetent"));				// model 16
	modelString.push_back(tr("User-Defined Monomer-Nmer, some Nmer is incompetent"));					// model 17
	modelString.push_back(tr("User-Defined irreversible Monomer-Nmer"));										// model 18
	modelString.push_back(tr("User-Defined Monomer-Nmer plus contaminant"));								// model 19
}

bool US_Config::col_exists()
{
	QString version="0.0";
	bool flag=false;
	QString  colfile;
	colfile = home.copy();
#ifdef UNIX
	colfile.append(".uscolors");
#endif
#ifdef WIN32
	colfile.append("Program Files/ultrascan/etc/uscolors");
#endif
	QFile f(colfile);
	if(f.open(IO_ReadOnly))
	{
		QDataStream ds(&f);
		ds >> global_colors.pushb;
		ds >> global_colors.frames;
		ds >> global_colors.labels;
		ds >> global_colors.pushb_active;
		ds >> global_colors.dk_red;
		ds >> global_colors.dk_green;
		ds >> global_colors.plot;
		ds >> global_colors.major_ticks;
		ds >> global_colors.minor_ticks;
		ds >> global_colors.gray;
		ds >> global_colors.gray1;
		ds >> global_colors.gray2;
		ds >> global_colors.gray3;
		ds >> global_colors.gray4;
		ds >> global_colors.gray5;
		ds >> global_colors.gray6;
		ds >> global_colors.gray7;
		ds >> global_colors.gray8;
		ds >> global_colors.gray9;
		ds >> global_colors.cg_frame;
		ds >> global_colors.cg_pushb;
		ds >> global_colors.cg_pushb_active;
		ds >> global_colors.cg_pushb_disabled;
		ds >> global_colors.cg_label;
		ds >> global_colors.cg_label_disabled;
		ds >> global_colors.cg_label_warn;
		ds >> global_colors.cg_edit;
		ds >> global_colors.cg_edit_warn;
		ds >> global_colors.cg_dk_red;
		ds >> global_colors.cg_dk_green;
		ds >> global_colors.cg_red;
		ds >> global_colors.cg_green;
		ds >> global_colors.cg_gray;
		ds >> global_colors.cg_normal;
		ds >> global_colors.cg_plot;
		ds >> global_colors.cg_lcd;
		ds >> global_colors.cg_bunt;
		if (!f.atEnd())
		{
			ds >> version;
			if (version.toFloat() > 7.0)
			{
				flag = true;
			}
		}
	}
	if (!flag)
	{
		if (write_default_colors())
		{
			flag = true;
		}
	}
	return (flag);
}

void US_Config::color_defaults()
{
// Default Colors for UltraScan
	QColor temp_col;
	temp_col.setRgb(0x00, 0x00, 0x80);
	global_colors.pushb.setRgb(0x00, 0xe0, 0xe0);
	global_colors.pushb_active.setRgb(0x00, 0xa0, 0xa0);
	global_colors.frames.setRgb(0x00, 0x77, 0x77);
	global_colors.labels.setRgb(0x00, 0x00, 0x00);
	global_colors.dk_green.setRgb(0x00, 0x44, 0x00);
	global_colors.dk_red.setRgb(0x55, 0x00, 0x00);
	global_colors.gray.setRgb(0x88, 0x88, 0x88);
	global_colors.gray1.setRgb(0xef, 0xef, 0xef);
	global_colors.gray2.setRgb(0xde, 0xde, 0xde);
	global_colors.gray3.setRgb(0xc6, 0xc6, 0xc6);
	global_colors.gray4.setRgb(0x84, 0x84, 0x84);
	global_colors.gray5.setRgb(0xaa, 0xaa, 0xaa);
	global_colors.gray6.setRgb(0xbb, 0xbb, 0xbb);
	global_colors.gray7.setRgb(0xcc, 0xcc, 0xcc);
	global_colors.gray8.setRgb(0xdd, 0xdd, 0xdd);
	global_colors.gray9.setRgb(0xee, 0xee, 0xee);
	global_colors.plot.setRgb(0x00, 0x00, 0x00);
	global_colors.major_ticks.setRgb(0xff, 0xff, 0xff);
	global_colors.minor_ticks.setRgb(0xaa, 0xaa, 0xaa);

	global_colors.cg_frame.setColor(QColorGroup::Foreground, Qt::white);
	global_colors.cg_frame.setColor(QColorGroup::Background, global_colors.frames);
	global_colors.cg_frame.setColor(QColorGroup::Light, global_colors.gray2);
	global_colors.cg_frame.setColor(QColorGroup::Dark, global_colors.gray4);
	global_colors.cg_frame.setColor(QColorGroup::Mid, Qt::cyan);
	global_colors.cg_frame.setColor(QColorGroup::Text, Qt::white);
	global_colors.cg_frame.setColor(QColorGroup::Base, global_colors.gray4);

	global_colors.cg_pushb.setColor(QColorGroup::ButtonText, Qt::black);
	global_colors.cg_pushb.setColor(QColorGroup::Button, global_colors.pushb);
	global_colors.cg_pushb.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_pushb.setColor(QColorGroup::Dark, global_colors.gray4);
	global_colors.cg_pushb.setColor(QColorGroup::Shadow, Qt::black);
	global_colors.cg_pushb.setColor(QColorGroup::Background, global_colors.pushb); // for splash screen shadow
	global_colors.cg_pushb_disabled.setColor(QColorGroup::ButtonText, Qt::white);
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Button, global_colors.pushb);
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Dark, global_colors.gray4);
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Shadow, Qt::black);
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Background, global_colors.pushb); // for splash screen shadow
	global_colors.cg_pushb_disabled.setColor(QColorGroup::Text, global_colors.gray3);


	global_colors.cg_pushb_active.setColor(QColorGroup::ButtonText, Qt::black);
	global_colors.cg_pushb_active.setColor(QColorGroup::Button, global_colors.pushb);
	global_colors.cg_pushb_active.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_pushb_active.setColor(QColorGroup::Dark, global_colors.gray4);
	global_colors.cg_pushb_active.setColor(QColorGroup::Shadow, Qt::black);
	global_colors.cg_pushb_active.setColor(QColorGroup::Background, global_colors.pushb); // for splash screen shadow

	global_colors.cg_label.setColor(QColorGroup::Foreground, Qt::white);
	global_colors.cg_label.setColor(QColorGroup::Background, global_colors.labels);
	global_colors.cg_label.setColor(QColorGroup::Light, Qt::black);
	global_colors.cg_label.setColor(QColorGroup::Dark, Qt::black);
	global_colors.cg_label.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_label.setColor(QColorGroup::Text, Qt::white);
	global_colors.cg_label.setColor(QColorGroup::Base, Qt::black);

	global_colors.cg_label_warn.setColor(QColorGroup::Foreground, Qt::red);
	global_colors.cg_label_warn.setColor(QColorGroup::Button, global_colors.labels);
	global_colors.cg_label_warn.setColor(QColorGroup::Light, Qt::black);
	global_colors.cg_label_warn.setColor(QColorGroup::Dark, Qt::black);
	global_colors.cg_label_warn.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_label_warn.setColor(QColorGroup::Text, Qt::red);
	global_colors.cg_label_warn.setColor(QColorGroup::BrightText, Qt::red);
	global_colors.cg_label_warn.setColor(QColorGroup::Base, Qt::black);
	global_colors.cg_label_warn.setColor(QColorGroup::Background, Qt::black);

	global_colors.cg_label_disabled.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_label_disabled.setColor(QColorGroup::Background, global_colors.labels);
	global_colors.cg_label_disabled.setColor(QColorGroup::Light, Qt::black);
	global_colors.cg_label_disabled.setColor(QColorGroup::Dark, Qt::black);
	global_colors.cg_label_disabled.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_label_disabled.setColor(QColorGroup::Text, global_colors.gray);
	global_colors.cg_label_disabled.setColor(QColorGroup::Base, Qt::black);

	global_colors.cg_edit.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_edit.setColor(QColorGroup::Background, Qt::white);
	global_colors.cg_edit.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_edit.setColor(QColorGroup::Dark, global_colors.gray);
	global_colors.cg_edit.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_edit.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_edit.setColor(QColorGroup::Base, Qt::white);
	global_colors.cg_edit.setColor(QColorGroup::HighlightedText, Qt::white);
	global_colors.cg_edit.setColor(QColorGroup::Highlight, temp_col);
	global_colors.cg_edit.setColor(QColorGroup::Button, global_colors.gray3);

	global_colors.cg_edit.setColor(QColorGroup::Midlight, global_colors.gray3);
	global_colors.cg_edit.setColor(QColorGroup::BrightText, Qt::red);
	global_colors.cg_edit.setColor(QColorGroup::ButtonText, Qt::black);
	global_colors.cg_edit.setColor(QColorGroup::Shadow, Qt::black);

	global_colors.cg_edit_warn.setColor(QColorGroup::Foreground, Qt::red);
	global_colors.cg_edit_warn.setColor(QColorGroup::Button, Qt::white);
	global_colors.cg_edit_warn.setColor(QColorGroup::Light, Qt::black);
	global_colors.cg_edit_warn.setColor(QColorGroup::Dark, global_colors.gray);
	global_colors.cg_edit_warn.setColor(QColorGroup::Mid, Qt::white);
	global_colors.cg_edit_warn.setColor(QColorGroup::Text, Qt::red);
	global_colors.cg_edit_warn.setColor(QColorGroup::BrightText, Qt::red);
	global_colors.cg_edit_warn.setColor(QColorGroup::Base, Qt::black);
	global_colors.cg_edit_warn.setColor(QColorGroup::Background, Qt::white);

	global_colors.cg_dk_green.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_dk_green.setColor(QColorGroup::Background, global_colors.dk_green);
	global_colors.cg_dk_green.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_dk_green.setColor(QColorGroup::Dark, global_colors.gray);
	global_colors.cg_dk_green.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_dk_green.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_dk_green.setColor(QColorGroup::Base, Qt::black);

	global_colors.cg_dk_red.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_dk_red.setColor(QColorGroup::Background, global_colors.dk_red);
	global_colors.cg_dk_red.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_dk_red.setColor(QColorGroup::Dark, global_colors.gray);
	global_colors.cg_dk_red.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_dk_red.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_dk_red.setColor(QColorGroup::Base, Qt::black);

	global_colors.cg_green.setColor(QColorGroup::Foreground, Qt::white);
	global_colors.cg_green.setColor(QColorGroup::Background, Qt::green);
	global_colors.cg_green.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_green.setColor(QColorGroup::Dark, global_colors.gray7);
	global_colors.cg_green.setColor(QColorGroup::Mid, Qt::darkRed);
	global_colors.cg_green.setColor(QColorGroup::Text, Qt::darkYellow);
	global_colors.cg_green.setColor(QColorGroup::Base, Qt::darkCyan);

	global_colors.cg_red.setColor(QColorGroup::Foreground, Qt::red);
	global_colors.cg_red.setColor(QColorGroup::Background, Qt::black);
	global_colors.cg_red.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_red.setColor(QColorGroup::Dark, global_colors.gray7);
	global_colors.cg_red.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_red.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_red.setColor(QColorGroup::Base, Qt::black);

	global_colors.cg_gray.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_gray.setColor(QColorGroup::Background, Qt::gray);
	global_colors.cg_gray.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_gray.setColor(QColorGroup::Dark, Qt::black);
	global_colors.cg_gray.setColor(QColorGroup::Mid, Qt::black);
	global_colors.cg_gray.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_gray.setColor(QColorGroup::Base, Qt::white);

	global_colors.cg_normal.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_normal.setColor(QColorGroup::Background, global_colors.gray3);
	global_colors.cg_normal.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_normal.setColor(QColorGroup::Dark, global_colors.gray4);
	global_colors.cg_normal.setColor(QColorGroup::Mid, global_colors.gray3);
	global_colors.cg_normal.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_normal.setColor(QColorGroup::Base, Qt::white);
	global_colors.cg_normal.setColor(QColorGroup::HighlightedText, Qt::white);
	global_colors.cg_normal.setColor(QColorGroup::Highlight, temp_col);
	global_colors.cg_normal.setColor(QColorGroup::Button, global_colors.gray3);

	global_colors.cg_normal.setColor(QColorGroup::Midlight, global_colors.gray3);
	global_colors.cg_normal.setColor(QColorGroup::BrightText, Qt::red);
	global_colors.cg_normal.setColor(QColorGroup::ButtonText, Qt::black);
	global_colors.cg_normal.setColor(QColorGroup::Shadow, Qt::black);

	global_colors.cg_plot.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_plot.setColor(QColorGroup::Background,global_colors.gray3);
	global_colors.cg_plot.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_plot.setColor(QColorGroup::Dark, Qt::white);
	global_colors.cg_plot.setColor(QColorGroup::Mid, global_colors.gray5);
	global_colors.cg_plot.setColor(QColorGroup::Text, Qt::black);
	global_colors.cg_plot.setColor(QColorGroup::Base, Qt::white);
	global_colors.cg_plot.setColor(QColorGroup::HighlightedText, Qt::white);
	global_colors.cg_plot.setColor(QColorGroup::Highlight, temp_col);
	global_colors.cg_plot.setColor(QColorGroup::Button, Qt::black);

	global_colors.cg_plot.setColor(QColorGroup::Midlight, Qt::black);
	global_colors.cg_plot.setColor(QColorGroup::BrightText, Qt::black);
	global_colors.cg_plot.setColor(QColorGroup::ButtonText, Qt::white);
	global_colors.cg_plot.setColor(QColorGroup::Shadow, Qt::black);

	global_colors.cg_lcd.setColor(QColorGroup::Foreground, Qt::green);
	global_colors.cg_lcd.setColor(QColorGroup::Background, Qt::black);
	global_colors.cg_lcd.setColor(QColorGroup::Light, Qt::green);
	global_colors.cg_lcd.setColor(QColorGroup::Dark, global_colors.frames);
	global_colors.cg_lcd.setColor(QColorGroup::Mid, Qt::cyan);
	global_colors.cg_lcd.setColor(QColorGroup::Text, global_colors.dk_red);
	global_colors.cg_lcd.setColor(QColorGroup::Base, Qt::blue);

	global_colors.cg_bunt.setColor(QColorGroup::Foreground, Qt::black);
	global_colors.cg_bunt.setColor(QColorGroup::Background, Qt::yellow);
	global_colors.cg_bunt.setColor(QColorGroup::Light, Qt::white);
	global_colors.cg_bunt.setColor(QColorGroup::Dark, Qt::red);
	global_colors.cg_bunt.setColor(QColorGroup::Mid, Qt::cyan);
	global_colors.cg_bunt.setColor(QColorGroup::Text, Qt::green);
	global_colors.cg_bunt.setColor(QColorGroup::Base, Qt::blue);
}

void US_Config::setDefault()
{
	QString str;
	config_list.version = US_Version;
#ifdef UNIX
	config_list.browser = "/usr/bin/mozilla";
	config_list.tar = "/bin/tar";
	config_list.gzip = "/bin/gzip";
	config_list.system_dir = getenv("ULTRASCAN");
	if (config_list.system_dir.isEmpty())
	{
		config_list.system_dir = "/usr/share/ultrascan";
	}
	config_list.help_dir = config_list.system_dir + "/doc";
	config_list.root_dir = home.copy();
	config_list.root_dir.append("us");
	config_list.data_dir = config_list.root_dir + "/data";
	config_list.archive_dir = config_list.root_dir + "/archive";
	config_list.result_dir = config_list.root_dir + "/results";
	config_list.html_dir = config_list.root_dir + "/reports";
#endif
#ifdef WIN32
	//config_list.browser = "C:\\Program\ Files\\Netscape\\Netscape\ 6\\netscp6.exe";
  	config_list.browser = "C:/Program Files/Internet Explorer/IEXPLORE.EXE";
	config_list.tar = "C:/Program Files/Ultrascan/bin/tar.exe";
	config_list.gzip = "C:/Program Files/Ultrascan/bin/gzip.exe";
	config_list.system_dir = getenv("ULTRASCAN");
	if (config_list.system_dir.isEmpty())
	{
		config_list.system_dir = "C:/Program Files/Ultrascan";
	}
	config_list.help_dir = config_list.system_dir + "/doc";
	config_list.root_dir = home;
	config_list.root_dir.append("us");
	config_list.data_dir = config_list.root_dir + "/data";
	config_list.archive_dir = config_list.root_dir + "/archive";
	config_list.result_dir = config_list.root_dir + "/results";
	config_list.html_dir = config_list.root_dir + "/reports";
#endif
	config_list.temperature_tol = 0.5; //allow a max of 0.5 degrees variation over course of run
	config_list.beckman_bug = 0;	//Default: False, time doesn't have to multiplied by 10
	config_list.fontFamily = "Helvetica";
	config_list.fontSize = 10;
	config_list.margin = 10;
	config_list.numThreads = 1;	//Default: 1 thread
}

bool US_Config::read()
{
	QString rcfile, str;
	rcfile = home.copy();
#ifdef UNIX
	rcfile.append("/.usrc");
#endif
#ifdef WIN32
	rcfile.append("Program Files/ultrascan/etc/usrc");
#endif
	QFile f(rcfile);
	if (f.exists())
	{
		f.open(IO_ReadOnly);
		QTextStream ts (&f);
		config_list.version = ts.readLine();
		if (config_list.version.toFloat() < 6.0)
		{
			f.close();
			f.remove();
			return(false);
		}
		config_list.browser = ts.readLine();
		config_list.tar = ts.readLine();
		config_list.gzip = ts.readLine();
		config_list.help_dir = ts.readLine();
		config_list.data_dir = ts.readLine();
		config_list.root_dir = ts.readLine();
		config_list.archive_dir = ts.readLine();
		config_list.result_dir = ts.readLine();
		str = ts.readLine();
		config_list.beckman_bug = str.toInt();
		str = ts.readLine();
		config_list.temperature_tol = str.toFloat();
//
// If we have an old version of the config file, file caps and/or  aren't int ere
// and we need to initialize this variable and write out a new version that
// incorporates the new USglobal variable for filecaps:
//
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.html_dir = str;
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.system_dir = str;
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.fontFamily = str;
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.fontSize = str.toInt();
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.margin = str.toInt();
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return (false);
		}
		if (!ts.atEnd())
		{
			str = ts.readLine();
			if (!str.isNull() && !str.isEmpty())
			{
				config_list.numThreads = str.toUInt();
			}
			else
			{
				return (false);
			}
		}
		else
		{
			return(false);
		}
		f.close();
	}
	else
	{
		return (false);
	}
	return(true);
}
