#include "../include/us_equilreport.h"

US_Report_Equil::US_Report_Equil(bool flag, QWidget *p, const char *name) : QWidget(p, name)
{
   USglobal = new US_Config();
   if(flag)
   {
      load();
   }
}

US_Report_Equil::~US_Report_Equil()
{
}

void US_Report_Equil::load()
{
   data_control = new Data_Control_W(8);
   data_control->load_data();
   generate(data_control);
}

void US_Report_Equil::view(QString fileName)
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_html_file(fileName);
}
   
void US_Report_Equil::generate(Data_Control_W *data_control)
{
   bool flag, pflag;
   QString str1, str2;
   QString fileName, baseName, run_id, htmlDir, printerFile;
   run_id = data_control->run_inf.run_id;   
   htmlDir = data_control->htmlDir;
   baseName = USglobal->config_list.result_dir + "/" + run_id + ".";
   fileName = htmlDir + "/index.html";
   printerFile = htmlDir + "/printer.html";
   QFile f1(fileName), testfile, f2(printerFile);
   if(f1.open(IO_WriteOnly | IO_Translate) && f2.open(IO_WriteOnly | IO_Translate))
   {
      pflag = false;
      flag = false;
      str1 = USglobal->config_list.help_dir + "/graphics/ultrascan1.png";
      str2 = htmlDir + "/ultrascan1.png";
      copy(str1, str2);
      str1 = USglobal->config_list.help_dir + "/graphics/printer.gif";
      str2 = htmlDir + "/printer.gif";
      copy(str1, str2);
      str1 = USglobal->config_list.help_dir + "/graphics/netscape.gif";
      str2 = htmlDir + "/netscape.gif";
      copy(str1, str2);
      str1 = USglobal->config_list.help_dir + "/mainstyle.css";
      str2 = htmlDir + "/mainstyle.css";
      copy(str1, str2);
      QTextStream ts(&f1);
      QTextStream ps(&f2);
      str1 = tr("Equilibrium Data Report for Run: ") + run_id;
      getHeader(&str2, str1);
      str1 = tr("Equilibrium Data Report for Run: ") + "<i>" + run_id + "</i>";
      ts << str2;
      ts << "<p><h2>" << str1 << "</h2>\n<p>\n";
      ts << tr("<b>Please note:</b> ASCII data files are formatted in <i><b>UltraScan II</b></i> format,\n");
      ts << tr("more information on this format, including row/column descriptions, can\n");
      ts << tr("be found ") << "<a href=http://www.ultrascan.uthscsa.edu/manual/file_format.html>" << tr("here") << "</a>.<p>\n";
      ts << tr("<b>Quick Index:</b><p>\n<ul>\n");
      for (unsigned int i=0; i<8; i++)
      {
         for  (unsigned int j=0; j<3; j++)
         {
            if (data_control->run_inf.scans[i][j] != 0)
            {
               ts << "   <li><a href=\"#cell" << (i+1) << (j+1) << "\">"<< tr("Cell ") << (i+1) << tr(", Wavelength ") << (j+1) << "</a>\n";
            }
         }
      }
      ts << tr("</ul>\n<p><b>Run Information:</b>\n<p>\n<ul>\n");
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
         ts << "   <li><a href=rotorspeed_data.png>" << tr("Plot of Rotorspeed vs. Scan Number</a>\n");
         ps << "<img src=rotorspeed_data.png>\n<p>\n";
         pflag = true;
         flag = true;
      }
      str1 = htmlDir + "/time_data.png";
      testfile.setName(str1);
      if (testfile.exists())
      {
         ts << "   <li><a href=time_data.png>" << tr("Plot of Time Differences between Scans vs. Scan Number</a>\n");
         ps << "<img src=time_data.png>\n<p>\n";
         pflag = true;
         flag = true;
      }
      str1 = baseName + "extinction.res";
      testfile.setName(str1);
      if (testfile.exists())
      {
         str2 = htmlDir + "/" +run_id + ".extinction.res";
         copy(str1, str2);
         ts << "   <li><a href=" << run_id << ".extinction.res" << tr(">Wavelength Scan/Extinction Calculation Results</a>\n");
      }
      str1 = htmlDir + "/" + run_id + ".extinction.png";
      testfile.setName(str1);
      if (testfile.exists())
      {
         str1 = run_id + ".extinction.png";
         ts << "   <li><a href=" << str1 << tr(">Extinction Profile Plot</a>\n");
         ps << "   <img src=" << str1 << ">\n<p>\n";
      }
      str1 = baseName + "extinction.dat";
      testfile.setName(str1);
      if (testfile.exists())
      {
         str2 = htmlDir + "/" + run_id + ".extinction.dat";
         copy(str1, str2);
         ts << "   <li><a href=" << run_id << ".extinction.dat" << tr(">ASCII Data for Extinction Profile Plot</a>\n");
      }
      ts << "   </ul>\n<p>\n";
      for (unsigned int i=0; i<8; i++)
      {
         for  (unsigned int j=0; j<3; j++)
         {
            if (data_control->run_inf.scans[i][j] != 0)
            {
               ts << "\n<a name=\"cell" << (i+1) << (j+1) << "\">\n"; 
               str1.sprintf("<b>Cell %d (%d nm): " + data_control->run_inf.cell_id[i] + "</b>\n<p>\n", i+1, data_control->run_inf.wavelength[i][j]);
               ts << str1 << tr("<ul>\n   <b>Experimental Data:</b>\n   <p>\n   <ul>\n");
               ps << "<p><hr><p>" << str1 << tr("\n<b>Experimental Data:</b>\n<p>\n");
               for (unsigned int k=0; k<4; k++)
               {
                  // Raw Data Plot Image:
                  str1.sprintf(htmlDir + "/raw_%d%d%d.png", i+1, j+1, k+1);
                  testfile.setName(str1);
                  if (testfile.exists())
                  {
                     ts << tr("Channel ") << str2.sprintf("%d", k+1) << ":<br>\n";
                     str1.sprintf("raw_%d%d%d.png", i+1, j+1, k+1);
                     ts << "      <li><a href=" << str1 << tr(">Experimental Data Plot</a>\n");
                     ps << "<img src=" << str1 << tr("> (channel ") << str2.sprintf("%d", k+1) << ")\n<p>\n";
                  }
                  // Raw Data Ascii Data:
                  str1.sprintf(baseName + "raw.%d%d%d", i+1, j+1, k+1);
                  testfile.setName(str1);
                  if (testfile.exists())
                  {
                     flag = true;
                     str2.sprintf(htmlDir + "/" + run_id + ".raw.%d%d%d", i+1, j+1, k+1);
                     copy(str1, str2);
                     ts << "      <li><a href=" << str1.sprintf(run_id + ".raw.%d%d%d", i+1, j+1, k+1) << tr(">ASCII File of Experimental Data</a>\n<p>\n");
                  }
               }
               ts << "   </ul>\n";
               ts << "</ul><p>\n";
            }
         }
      }
      ts << "<p>\n<a href=printer.html><img src=printer.gif align=center border=0>" << tr(" Printer-friendly Version - Graphs only</a> (Note: This may take a long time to load if you are on a slow link!)\n<p>\n");
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
