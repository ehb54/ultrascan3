#include "../include/us_velocreport.h"

US_Report_Veloc::US_Report_Veloc(bool flag, QWidget *p, const char *name) : QWidget(p, name)
{
	USglobal = new US_Config();
	if(flag)
	{
		load();
	}
}

US_Report_Veloc::~US_Report_Veloc()
{
}

void US_Report_Veloc::load()
{
	data_control = new Data_Control_W(7);
	data_control->load_data(false);
	write_file(data_control);
}

void US_Report_Veloc::view(QString fileName)
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_html_file(fileName);
}

void US_Report_Veloc::write_file(Data_Control_W *data_control)
{
	bool flag, pflag;
	QString str1, str2, str3;
	QString fileName, baseName, run_id, htmlDir, printerFile;
	vector <QString> analysis_type;
	analysis_type.clear();
	analysis_type.push_back("sa2d");
	analysis_type.push_back("ga");
	analysis_type.push_back("fe");
	analysis_type.push_back("cofs");
	analysis_type.push_back("sa2d_mc");
	analysis_type.push_back("ga_mc");
	analysis_type.push_back("global");
	analysis_type.push_back("sa2d_mw");
	analysis_type.push_back("ga_mw");
	analysis_type.push_back("sa2d_mw_mc");
	analysis_type.push_back("ga_mw_mc");
	htmlDir = data_control->htmlDir;
	fileName = htmlDir + "/index.html";
	printerFile = htmlDir + "/printer.html";
	baseName = data_control->USglobal->config_list.result_dir + "/" + data_control->run_inf.run_id + ".";

	QFile f1(fileName), testfile, f2(printerFile);
	if(f1.open(IO_WriteOnly | IO_Translate) && f2.open(IO_WriteOnly | IO_Translate))
	{
		pflag = false;
		flag = false;
		str1 = data_control->USglobal->config_list.help_dir + "/graphics/ultrascan1.png";
		str2 = htmlDir + "/ultrascan1.png";
	   copy(str1, str2);
		str1 = data_control->USglobal->config_list.help_dir + "/graphics/printer.gif";
		str2 = htmlDir + "/printer.gif";
		copy(str1, str2);
		str1 = data_control->USglobal->config_list.help_dir + "/graphics/netscape.gif";
		str2 = htmlDir + "/netscape.gif";
		copy(str1, str2);
		str1 = data_control->USglobal->config_list.help_dir + "/mainstyle.css";
		str2 = htmlDir + "/mainstyle.css";
		copy(str1, str2);
		QTextStream ts(&f1);
		QTextStream ps(&f2);
		str1 = tr("Data Analysis Report for Run: ") + data_control->run_inf.run_id;
		getHeader(&str2, str1);
		str1 = tr("Data Analysis Report for Run: ") + "<i>" + data_control->run_inf.run_id + "</i>";
		ts << str2;
		ts << "<p><h2>" << str1 << "</h2>\n<p>\n";
		ts << tr("<b>Please note:</b> ASCII data files are formatted in <i><b>UltraScan II</b></i> format,\n");
		ts << tr("more information on this format, including row/column descriptions,\n");
		ts << tr("can be found ") << "<a href=http://www.ultrascan.uthscsa.edu/manual/file_format.html>"<<tr("here")<<"</a>.<p>\n";
		ts << tr("<b>Quick Index:</b><p>\n<ul>\n");
		for (unsigned int i=0; i<8; i++)
		{
			for  (unsigned int j=0; j<3; j++)
			{
				if (data_control->run_inf.scans[i][j] != 0)
				{
					ts << "   <li><a href=\"#cell" << (i+1) << (j+1) << "\">Cell " << (i+1) << ", Wavelength " << (j+1) <<
					" (" << data_control->run_inf.cell_id[i] << ")</a>\n";
				}
			}
		}
		ts << tr("</ul>\n<p><b>Run Information:</b>\n<ul>\n");
		ps << str2;
		ps << "<p><center><h2>" << str1 << "</h2>\n<p>\n";
		ps << tr("<b><hr><p>Run Information:</b>\n<p>");
		str1 = htmlDir + "/temperature_data.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			ts << "   <li><a href=temperature_data.png>" << tr("Plot of Temperature Variation vs. Scan Number</a>\n");
			ps << "<img src=temperature_data.png>\n<p>\n";
			pflag = true;
			flag = true;
		}
		str1 = htmlDir + "/rotorspeed_data.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			ts << "   <li><a href=rotorspeed_data.png>"<< tr("Plot of Rotorspeed vs. Scan Number</a>\n");
			ps << "<img src=rotorspeed_data.png>\n<p>\n";
			pflag = true;
			flag = true;
		}
		str1 = htmlDir + "/time_data.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			ts << "   <li><a href=time_data.png>" << tr("Plot of Time Differences between Scans vs. Scan Number</a>\n<p>\n");
			ps << "<img src=time_data.png>\n<p>\n";
			pflag = true;
			flag = true;
		}
		if (!flag)
		{
			ts << "      (not available)\n";
		}
		if (!pflag)
		{
			ps << "<p>(not available)<p>\n";
		}
		str1 = baseName + "extinction.res";
		testfile.setName(str1);
		if (testfile.exists())
		{
			ts << "   </ul>\n<p>\n";
			ts << "<b>Extinction Results:</b><p>\n<ul>\n";
			str2 = htmlDir + "/" + data_control->run_inf.run_id + ".extinction.res";
			copy(str1, str2);
			ts << "   <li><a href=" << data_control->run_inf.run_id << ".extinction.res" << tr(">Wavelength Scan/Extinction Calculation Results</a>\n");
		}
		str1 = htmlDir + "/" + data_control->run_inf.run_id + ".extinction.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			str1 = data_control->run_inf.run_id + ".extinction.png";
			ts << "   <li><a href=" << str1 << tr(">Extinction Profile Plot</a>\n");
			ps << "   <img src=" << str1 << ">\n<p>\n";
		}
		str1 = baseName + "extinction.dat";
		testfile.setName(str1);
		if (testfile.exists())
		{
			str2 = htmlDir + "/" + data_control->run_inf.run_id + ".extinction.dat";
			copy(str1, str2);
			ts << "   <li><a href=" << data_control->run_inf.run_id << ".extinction.dat" << tr(">ASCII Data for Extinction Profile Plot</a>\n");
		}
		ts << "   </ul>\n<p><b>Combined Distributions:</b><p><ul>\n";

		ts << tr("<b>van Holde - Weischet Analysis:</b>\n<p>\n<ul>\n");
		ps << tr("<p><hr><p><b>van Holde - Weischet Analysis:</b>\n<p>\n<ul>\n");
		ts << tr("<b>s<sub>20,W</sub> Combined Distribution:</b>\n<ul>\n");
		ps << tr("<p><hr><p><b>s<sub>20,W</sub> Combined Distribution:</b>\n<p>\n");
		flag = false;
		pflag = false;
		str1 = htmlDir + "/distribution.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			pflag = true;
			ts << "   <li><a href=distribution.png>" << tr("van Holde - Weischet combined Distribution Plot</a>\n");
			ps << "<img src=distribution.png>\n<p>\n";
		}
		str1 = htmlDir + "/distrib.dat";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			ts << "   <li><a href=distrib.dat>"<< tr("ASCII File for Distribution Plot Data</a>\n");
		}
		str1 = htmlDir + "/distrib.txt";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			ts << "   <li><a href=distrib.txt>" << tr("Text Listing for included Data sets</a>\n");
		}
		if (!flag)
		{
			ts << tr("      (not available)\n");
		}
		if (!pflag)
		{
			ps << tr("<p>(not available)<p>\n");
		}
		ts << "   </ul>\n<p>\n";
		ts << tr("<b>Molecular Weight Transformations of van Holde - Weischet Combined Distributions:</b>\n<ul>\n");
		ps << tr("<p><hr><p><b>Molecular Weight Transformations of van Holde - Weischet Combined Distributions:</b>\n<p>\n");
		flag = false;
		pflag = false;
		str1 = htmlDir + "/mw_distribution.png";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			pflag = true;
			ts << "   <li><a href=mw_distribution.png>" << tr("Combined Molecular Weight Distributions</a>\n");
			ps << "<img src=mw_distribution.png>\n<p>\n";
		}
		str1 = htmlDir + "/mw_distrib.dat";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			ts << "   <li><a href=mw_distrib.dat>"<< tr("ASCII File for Molecular Weight Distribution Data</a>\n");
		}
		str1 = htmlDir + "/mw_distrib.txt";
		testfile.setName(str1);
		if (testfile.exists())
		{
			flag = true;
			ts << "   <li><a href=mw_distrib.txt>" << tr("Text Listing for included Data sets</a>\n");
		}
		if (!flag)
		{
			ts << tr("      (not available)\n");
		}
		if (!pflag)
		{
			ps << tr("<p>(not available)<p>\n");
		}
		ts << "</ul></ul>\n<p>\n";
		for (unsigned int k=0; k<analysis_type.size(); k++)
		{
			flag = false;
			pflag = false;
			switch (k)
			{
				case 0:
				{
					str3 = "2-dimensional Spectrum Analysis:";
					break;
				}
				case 1:
				{
					str3 = "Genetic Algorithm Analysis:";
					break;
				}
				case 2:
				{
					str3 = "Finite Element Analysis:";
					break;
				}
				case 3:
				{
					str3 = "C(s) Analysis:";
					break;
				}
				case 4:
				{
					str3 = "2-Dimensional Spectrum Analysis - Monte Carlo Analysis:";
					break;
				}
				case 5:
				{
					str3 = "Genetic Algorithm - Monte Carlo Analysis:";
					break;
				}
				case 6:
				{
					str3 = "Global Analysis:";
					break;
				}
				case 7:
				{
					str3 = "2-Dimensional Spectrum Analysis with Molecular Weight constraints:";
					break;
				}
				case 8:
				{
					str3 = "Genetic Algorithm with Molecular Weight constraints";
					break;
				}
				case 9:
				{
					str3 = "2-Dimensional Spectrum Analysis with Molecular Weight constraints - Monte Carlo Analysis:";
					break;
				}
				case 10:
				{
					str3 = "Genetic Algorithm with Molecular Weight constraints - Monte Carlo Analysis:";
					break;
				}
			}
			ts << tr("\n<p>\n   <b>" + str3 + "</b>\n<p>\n<ul>\n");
			ps << tr("<p><hr><p>\n<b>" + str3 + "</b>\n<p>\n");
			ts << tr("<b>s<sub>20,W</sub> Combined Distribution:</b>\n<ul>\n");
			ps << tr("<p><hr><p><b>s<sub>20,W</sub> Combined Distribution:</b>\n<p>\n");
			str1 = htmlDir + "/" + analysis_type[k] + "_s_distribution.png";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				pflag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_s_distribution.png>") << tr("s<sub>20,W</sub> Combined Distribution Plot</a>\n");
				ps << ("<img src=" + analysis_type[k] + "_s_distribution.png>\n<p>\n");
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_s_distrib.dat";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_s_distrib.dat>") << tr("ASCII File for s<sub>20,W</sub> Combined Distribution Plot Data</a>\n");
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_s_distrib.txt";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_s_distrib.txt>") << tr("Text Listing for included Data sets</a>\n");
			}
			if (!flag)
			{
				ts << tr("      (not available)\n");
			}
			if (!pflag)
			{
				ps << tr("<p>(not available)<p>\n");
			}
			ts << "   </ul>\n<p>\n";

			ts << tr("<b>Molecular Weight Combined Distribution:</b>\n<ul>\n");
			ps << tr("<p><hr><p><b>Molecular Weight Combined Distribution:</b>\n<p>\n");
			flag = false;
			pflag = false;
			str1 = htmlDir + "/" + analysis_type[k] + "_mw_distribution.png";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				pflag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_mw_distribution.png>") << tr("Molecular Weight Combined Distribution Plot</a>\n");
				ps << "<img src=distribution.png>\n<p>\n";
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_mw_distrib.dat";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_mw_distrib.dat>") << tr("ASCII File for Molecular Weight Combined Distribution Plot Data</a>\n");
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_mw_distrib.txt";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_mw_distrib.txt>") << tr("Text Listing for included Data sets</a>\n");
			}
			if (!flag)
			{
				ts << tr("      (not available)\n");
			}
			if (!pflag)
			{
				ps << tr("<p>(not available)<p>\n");
			}
			ts << "   </ul>\n<p>\n";
			ts << tr("<b>D<sub>20,W</sub> Combined Distribution:</b>\n<ul>\n");
			ps << tr("<p><hr><p><b>D<sub>20,W</sub> Combined Distribution:</b>\n<p>\n");
			flag = false;
			pflag = false;
			str1 = htmlDir + "/" + analysis_type[k] + "_d_distribution.png";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				pflag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_d_distribution.png>") << tr("D<sub>20,W</sub> combined Distribution Plot</a>\n");
				ps << ("<img src=" + analysis_type[k] + "_d_distribution.png>\n<p>\n");
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_d_distrib.dat";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_d_distrib.dat>") << tr("ASCII File for Distribution Plot Data</a>\n");
			}
			str1 = htmlDir + "/" + analysis_type[k] + "_d_distrib.txt";
			testfile.setName(str1);
			if (testfile.exists())
			{
				flag = true;
				ts << ("   <li><a href=" + analysis_type[k] + "_d_distrib.txt>") << tr("Text Listing for included Data sets</a>\n");
			}
			if (!flag)
			{
				ts << tr("      (not available)\n");
			}
			if (!pflag)
			{
				ps << tr("<p>(not available)<p>\n");
			}
			ts << "   </ul>\n<p>\n</ul>\n<p>\n";
		}
		ts << "   </ul>\n<p>\n";
		pflag = false;
		flag = false;
		for (unsigned int i=0; i<8; i++)
		{
			for  (unsigned int j=0; j<3; j++)
			{
				if (data_control->run_inf.scans[i][j] != 0)
				{
					ts << "\n<a name=\"cell" << (i+1) << (j+1) << "\">\n";
					str1.sprintf(tr("<b>Cell %d (%d nm): ") + data_control->run_inf.cell_id[i] + "</b>\n<p>\n", i+1, data_control->run_inf.wavelength[i][j]);
					ts << str1;
					ts << "<ul><b>Hydrodynamic Information:</b><p><ul>\n";
					str1.sprintf(data_control->USglobal->config_list.result_dir + "/%d.pep_res", data_control->run_inf.peptide_serialnumber[i][j][0]);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str2.sprintf(htmlDir + "/%d-pep.dat", data_control->run_inf.peptide_serialnumber[i][j][0]);
					 	copy(str1, str2);
						str1.sprintf("%d-pep.dat", data_control->run_inf.peptide_serialnumber[i][j][0]);
						ts << "      <li><a href=" << str1 << tr(">Peptide Sequence Information</a> (Database ID: "
								+ QString::number(data_control->run_inf.peptide_serialnumber[i][j][0]) + ")\n");
					}
					str1.sprintf(data_control->USglobal->config_list.result_dir + "/%d.buf_res", data_control->run_inf.buffer_serialnumber[i][j]);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str2.sprintf(htmlDir + "/%d-buf.dat", data_control->run_inf.buffer_serialnumber[i][j]);
					 	copy(str1, str2);
						str1.sprintf("%d-buf.dat", data_control->run_inf.buffer_serialnumber[i][j]);
						ts << "      <li><a href=" << str1 << tr(">Buffer Information</a> (Database ID: "
								+ QString::number(data_control->run_inf.buffer_serialnumber[i][j]) + ")\n");
					}
					ts << "</ul></ul><p>\n";
					
					flag = false;
					ts << tr("<ul>\n   <b>Experimental Data:</b>\n   <p>\n   <ul>\n");
					ps << "<p><hr><p>" << str1 << tr("\n<b>Experimental Data:</b>\n<p>\n");
// Raw Data Plot Image:
					str1.sprintf(htmlDir + "/raw_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str1.sprintf("raw_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Experimental Data Plot</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Raw Data Ascii Data:
					str1.sprintf(baseName + "raw.%d%d1", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".raw.%d%d1", i+1, j+1);
					 	copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".raw.%d%d1", i+1, j+1) << ">ASCII File of Experimental Data</a>\n";
					}
					ts << tr("\n   </ul>\n   <p>\n   <b>van Holde - Weischet Analysis:</b>\n   <p>\n   <ul>\n");
					ps << tr("<p><hr><p>\n<b>van Holde - Weischet Analysis:</b>\n<p>\n");
// vHW Analysis Report:
					str1.sprintf(baseName + "vhw_res.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".vhw_res.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".vhw_res.%d%d", i+1, j+1) << tr(">Analysis Report</a>\n");
					}
// Extrapolation Plot Image:
					str1.sprintf(htmlDir + "/vhw_ext_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str1.sprintf("vhw_ext_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Extrapolation Plot</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Extrapolation Plot Data:
					str1.sprintf(baseName + "vhw_ext.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".vhw_ext.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".vhw_ext.%d%d", i+1, j+1) << tr(">ASCII File of Extrapolation Plot Data</a>\n");
					}
// Distribution Plot Image:
					str1.sprintf(htmlDir + "/vhw_dis_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("vhw_dis_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Distribution Plot (G(S))</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Distribution Plot Data:
					str1.sprintf(baseName + "vhw_dis.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".vhw_dis.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".vhw_dis.%d%d", i+1, j+1) << tr(">ASCII File of Distribution Plot Data</a>\n");
					}
// Histogram Plot Image:
					str1.sprintf(htmlDir + "/vhw_his_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("vhw_his_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">S-value Histogram and Envelope Plot (G(S))</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Histogram Plot Data:
					str1.sprintf(baseName + "vhw_his.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".vhw_his.%d%d", i+1, j+1);
					 	copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".vhw_his.%d%d", i+1, j+1) << ">ASCII File of Histogram and Envelope Data</a>\n";
					}
// Edited Data Plot Image:
					str1.sprintf(htmlDir + "/vhw_edited_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("vhw_edited_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Edited Data Plot</a>\n");
						ps << tr("Edited Data used for analysis:") <<"<p>\n<img src=" << str1 << ">\n<p>\n";
					}
					if (!flag)
					{
						ts << tr("      (not available)\n");
					}
					if (!pflag)
					{
						ps << tr("<p>(not available)<p>\n");
					}
					for (unsigned int k=0; k<analysis_type.size(); k++)
					{
						flag = false;
						pflag = false;
						switch (k)
						{
							case 0:
							{
								str3 = "2-dimensional Spectrum Analysis:";
								break;
							}
							case 1:
							{
								str3 = "Genetic Algorithm Analysis:";
								break;
							}
							case 2:
							{
								str3 = "Finite Element Analysis:";
								break;
							}
							case 3:
							{
								str3 = "C(s) Analysis:";
								break;
							}
							case 4:
							{
								str3 = "2-Dimensional Spectrum Analysis - Monte Carlo Analysis:";
								break;
							}
							case 5:
							{
								str3 = "Genetic Algorithm - Monte Carlo Analysis:";
								break;
							}
							case 6:
							{
								str3 = "Global Analysis:";
								break;
							}
							case 7:
							{
								str3 = "2-Dimensional Spectrum Analysis with Molecular Weight constraints:";
								break;
							}
							case 8:
							{
								str3 = "Genetic Algorithm with Molecular Weight constraints";
								break;
							}
							case 9:
							{
								str3 = "2-Dimensional Spectrum Analysis with Molecular Weight constraints - Monte Carlo Analysis:";
								break;
							}
							case 10:
							{
								str3 = "Genetic Algorithm with Molecular Weight constraints - Monte Carlo Analysis:";
								break;
							}
						}
						ts << tr("\n   </ul>\n   <p>\n   <b>" + str3 + "</b>\n   <p>\n   <ul>\n");
						ps << tr("<p><hr><p>\n<b>" + str3 + "</b>\n<p>\n");

//Analysis Report:
						str1.sprintf(baseName + analysis_type[k] + "_res.%d%d", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							flag = true;
							str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + "." + analysis_type[k] + "_res.%d%d", i+1, j+1);
							copy(str1, str2);
							ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + "." + analysis_type[k] + "_res.%d%d", i+1, j+1) << tr(">Analysis Report</a>\n");
						}
						if (analysis_type[k] == "fe")
						{
//Analysis Report 2 for FE:
							str1.sprintf(baseName + "fef_res.%d%d", i+1, j+1);
							testfile.setName(str1);
							if (testfile.exists())
							{
								flag = true;
								str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".fef_res.%d%d", i+1, j+1);
								copy(str1, str2);
								ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".fef_res.%d%d", i+1, j+1) << tr(">Analysis Report 2</a>\n");
							}
						}
						if (analysis_type[k] == "ga_mc")
						{
//Monte Carlo Stats:
							str1.sprintf(baseName + "ga_stats.%d%d", i+1, j+1);
							testfile.setName(str1);
							if (testfile.exists())
							{
								flag = true;
								str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".ga_stats.%d%d", i+1, j+1);
								copy(str1, str2);
								ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".ga_stats.%d%d", i+1, j+1) << tr(">GA Monte Carlo Statistics</a>\n");
							}
						}
						if (analysis_type[k] == "sa2d_mc")
						{
//Monte Carlo Stats:
							str1.sprintf(baseName + "sa2d_stats.%d%d", i+1, j+1);
							testfile.setName(str1);
							if (testfile.exists())
							{
								flag = true;
								str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".sa2d_stats.%d%d", i+1, j+1);
								copy(str1, str2);
								ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".sa2d_stats.%d%d", i+1, j+1) << tr(">2-D Spectrum Analysis Monte Carlo Statistics</a>\n");
							}
						}
// Distribution Plot Image:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_3ddis_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_3ddis_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">3D Distribution Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// Distribution ASCII Data:
						str1.sprintf(baseName + analysis_type[k] + "_dis.%d%d", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + "." + analysis_type[k] + "_dis.%d%d", i+1, j+1);
							copy(str1, str2);
							ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + "." + analysis_type[k] + "_dis.%d%d", i+1, j+1) << tr(">ASCII File of 3D Distribution Plot Data</a>\n");
						}
// s-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_dis_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_dis_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Sedimentation Coefficient Distribution Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// MW-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_mw_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_mw_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Molecular Weight Distribution Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// f/f0 vs s-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_fs_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_fs_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Frictional Ratio f/f0 vs Sedimentation Coefficient</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// f/f0 vs s pseudo-3D Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_pseudo3d.%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_pseudo3d.%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Frictional Ratio f/f0 vs Sedimentation Coefficient, Pseudo-3D Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// f/f0 vs s pseudo-3D Distribution Plot (later versions):
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_pseudo3d_fs.%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_pseudo3d_fs.%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Frictional Ratio f/f0 vs Sedimentation Coefficient, Pseudo-3D Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// f/f0 vs MW pseudo-3D Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_pseudo3d_fmw.%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_pseudo3d_fmw.%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Frictional Ratio f/f0 vs Molecular Weight, Pseudo-3D Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// f/f0 vs MW-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_fmw_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_fmw_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Frictional Ratio f/f0 vs. Molecular Weight</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// D vs. s-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_ds_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_ds_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Diffusion Coefficient vs. Sedimentation Coefficient</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// D vs MW-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_dmw_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_dmw_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Diffusion Coefficient vs. Molecular Weight</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// D-Distribution Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_d_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_d_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Diffusion Coefficient Distribution Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// Residuals Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_resid_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_resid_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Residuals Plot</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// Residuals Pixmap Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_pixmap_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_pixmap_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Residuals Pixel Map</a>\n");
							ps << "<img src=" << str1 << ">\n<p>\n";
						}
// Edited Data Plot:
						str1.sprintf(htmlDir + "/" + analysis_type[k] + "_edited_%d%d.png", i+1, j+1);
						testfile.setName(str1);
						if (testfile.exists())
						{
							pflag = true;
							str1.sprintf(analysis_type[k] + "_edited_%d%d.png", i+1, j+1);
							ts << "      <li><a href=" << str1 << tr(">Edited Data Plot</a>\n");
							ps << tr("Edited Data used for analysis:") <<"<p>\n<img src=" << str1 << ">\n<p>\n";
						}
						if (!flag)
						{
							ts << tr("      (not available)\n");
						}
						if (!pflag)
						{
							ps << tr("<p>(not available)<p>\n");
						}
					}
					flag = false;
					pflag = false;

					ts << tr("   </ul>\n   <p>\n   <b>Second Moment Analysis:</b>\n   <p>\n   <ul>\n");
					ps << tr("<p><hr><p>\n<b>Second Moment Analysis:</b>\n<p>\n");
// 2. Moment Analysis Report:
					str1.sprintf(baseName + "sm_res.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".sm_res.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".sm_res.%d%d", i+1, j+1) << tr(">Analysis Report</a>\n");
					}
// 2. Moment Plot Image:
					str1.sprintf(htmlDir + "/sm_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("sm_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Second Moment Analysis Plot</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// 2. Moment Plot Data:
					str1.sprintf(baseName + "sm_dat.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".sm_dat.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".sm_dat.%d%d", i+1, j+1) << tr(">ASCII File of Second Moment Plot Data</a>\n");
					}
// Edited Data Plot Image:
					str1.sprintf(htmlDir + "/sm_edited_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("sm_edited_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Edited Data Plot</a>\n");
						ps << tr("Edited Data used for analysis:") << "<p>\n<img src=" << str1 << ">\n<p>\n";
					}
					if (!flag)
					{
						ts << tr("         (not available)\n");
					}
					if (!pflag)
					{
						ps << tr("<p>(not available)<p>\n");
					}
					flag = false;
					pflag = false;
					ts << "   </ul>\n";
					ts << tr("   <p>\n   <b>Time Derivative Analysis:</b>\n   <p>\n   <ul>\n");
					ps << tr("<p><hr><p>\n<b>Time Derivative Analysis:</b>\n<p>\n");
// Time Derivative Analysis Report:
					str1.sprintf(baseName + "dcdt_res.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdt_res.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdt_res.%d%d", i+1, j+1) << tr(">Analysis Report</a>\n");
					}
// Time Derivative Plot Image:
					str1.sprintf(htmlDir + "/dcdt_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						pflag = true;
						str1.sprintf("dcdt_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Time Derivative Analysis Plot</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Time Derivative Scan Plot Data:
					str1.sprintf(baseName + "dcdt_scans.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdt_scans.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdt_scans.%d%d", i+1, j+1) << tr(">ASCII File of Time Derivative Plot Data (Scans)</a>\n");
					}
// Time Derivative S-value Plot Data:
					str1.sprintf(baseName + "dcdt_sval.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdt_sval.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdt_sval.%d%d", i+1, j+1) << tr(">ASCII File of Time Derivative Plot Data (S-values)</a>\n");
					}
// Time Derivative Average Plot Image:
					str1.sprintf(htmlDir + "/dcdt_avg_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("dcdt_avg_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Time Derivative Analysis Plot (Average g(S))</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Time Derivative S-value Average Plot Data:
					str1.sprintf(baseName + "dcdt_avg.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdt_avg.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdt_avg.%d%d", i+1, j+1) << tr(">ASCII File of Time Derivative Plot Data (Average g(S))</a>\n");
					}
// Edited Data Plot Image:
					str1.sprintf(htmlDir + "/dcdt_edited_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("dcdt_edited_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Edited Data Plot</a>\n");
						ps << tr("Edited Data used for analysis:") << "<p>\n<img src=" << str1 << ">\n<p>\n";
					}
					if (!flag)
					{
						ts << tr("         (not available)\n");
					}
					if (!pflag)
					{
						ps << tr("<p>(not available)<p>\n");
					}
					flag = false;
					pflag = false;
					ts << "   </ul>\n";
					ts << tr("   <p>\n   <b>Radial Derivative Analysis:</b>\n   <p>\n   <ul>\n");
					ps << tr("<p><hr><p>\n<b>Radial Derivative Analysis:</b>\n<p>\n");
// Radial Derivative Analysis Report:
					str1.sprintf(baseName + "dcdr_res.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdr_res.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdr_res.%d%d", i+1, j+1) << tr(">Analysis Report</a>\n");
					}
// Radial Derivative Plot Image:
					str1.sprintf(htmlDir + "/dcdr_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("dcdr_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Radial Derivative Analysis Plot</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Radial Derivative Scan Plot Data:
					str1.sprintf(baseName + "dcdr_scans.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdr_scans.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdr_scans.%d%d", i+1, j+1) << tr(">ASCII File of Radial Derivative Plot Data (Scans)</a>\n");
					}
// Radial Derivative S-value Plot Data:
					str1.sprintf(baseName + "dcdr_sval.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdr_sval.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdr_sval.%d%d", i+1, j+1) << tr(">ASCII File of Radial Derivative Plot Data (S-values)</a>\n");
					}
// Radial Derivative Average Plot Image:
					str1.sprintf(htmlDir + "/dcdr_avg_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("dcdr_avg_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Radial Derivative Analysis Plot (Average g(S))</a>\n");
						ps << "<img src=" << str1 << ">\n<p>\n";
					}
// Radial Derivative S-value Average Plot Data:
					str1.sprintf(baseName + "dcdr_avg.%d%d", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						flag = true;
						str2.sprintf(htmlDir + "/" + data_control->run_inf.run_id + ".dcdr_avg.%d%d", i+1, j+1);
						copy(str1, str2);
						ts << "      <li><a href=" << str1.sprintf(data_control->run_inf.run_id + ".dcdr_avg.%d%d", i+1, j+1) << tr(">ASCII File of Radial Derivative Plot Data (Average g(S))</a>\n");
					}
// Edited Data Plot Image:
					str1.sprintf(htmlDir + "/dcdr_edited_%d%d.png", i+1, j+1);
					testfile.setName(str1);
					if (testfile.exists())
					{
						pflag = true;
						flag = true;
						str1.sprintf("dcdr_edited_%d%d.png", i+1, j+1);
						ts << "      <li><a href=" << str1 << tr(">Edited Data Plot</a>\n");
						ps << tr("Edited Data used for analysis:") << "<p>\n<img src=" << str1 << ">\n<p>\n";
					}
					if (!flag)
					{
						ts << tr("         (not available)\n");
					}
					if (!pflag)
					{
						ps << tr("<p>(not available)<p>\n");
					}
					flag = false;
					pflag = false;
					ts << "   </ul>\n";
					ts << "</ul><p>\n";
				}
			}
		}
		ts << "<p>\n<a href=printer.html><img src=printer.gif align=center border=0>" << tr("Printer-friendly Version - Graphs only</a> (Note: This may take a long time to load if you are on a slow link!)\n<p>\n");
		ps << "\n<p>\n<a href=index.html><img src=netscape.gif align=center border=0>" << tr(" Browser-friendly Version</a> (Links to Graphs and Text Files)\n</center>\n<p>\n");
		getFooter(&str1);
		ts << str1;
		ps << str1;
		f1.close();
		f2.close();
	}
	qApp->processEvents();
	QMessageBox mb(tr("UltraScan"),
						tr("The report has been written\n"
						"Do you want to start the browser\nto view the Report?"),
	QMessageBox::Information,
	QMessageBox::Yes | QMessageBox::Default,
	QMessageBox::No,
	QMessageBox::Cancel | QMessageBox::Escape);
	mb.setButtonText(QMessageBox::Yes, tr("View Report"));
	mb.setButtonText(QMessageBox::No, tr("Close"));
	switch(mb.exec())
	{
		case QMessageBox::Yes:
		{
			view(fileName);
			break;
		}
	}
}

