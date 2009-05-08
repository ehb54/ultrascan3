#include "../include/us_equilprojectreport.h"

US_Report_EquilProject::US_Report_EquilProject(bool flag, QWidget *p, const char *name) : QWidget(p, name)
{
   USglobal = new US_Config();
   if(flag)
   {
      load();
   }
}

US_Report_EquilProject::~US_Report_EquilProject()
{
}

void US_Report_EquilProject::view(QString fileName)
{
   US_Help *online_help; 
   online_help = new US_Help();
   online_help->show_html_file(fileName);
}

void US_Report_EquilProject::load()
{
   QString fileName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.eq-project", 0);
   if (fileName.isEmpty())
   {
      close();
   }
   else
   {
      generate(fileName);
   }
}

void US_Report_EquilProject::generate(QString fileName)
{
   QString str1, str2, str3, str4, baseName, htmlDir, printerFile, indexFile, projectName, modelDir;
   QDir mD;
   bool flag, pflag;
   QFile projectFile(fileName), testFile;
   int sets = 0;
   projectFile.open(IO_ReadOnly);
   QTextStream project_ts(&projectFile);
   projectName = project_ts.readLine();
   projectFile.close();
   htmlDir = USglobal->config_list.html_dir + "/" + projectName;
   indexFile = htmlDir + "/index.html";
   printerFile = htmlDir + "/printer.html";
   baseName = USglobal->config_list.result_dir + "/" + projectName;
   QFile f1(indexFile), f2(printerFile);
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
      
      // generate Header:

      str1 = tr("Equilibrium Fitting Report for Project: ") + projectName;
      getHeader(&str2, str1);
      ts << str2;
      ps << str2;

      str1 = tr("Equilibrium Fitting Report for Project: ") + "<i>" + projectName + "</i>";

      ts << "<p><h2>" << str1 << "</h2>\n<p>\n";
      ps << "<p><h2>" << str1 << "</h2>\n<p>\n";
      ts << tr("<b>Please note:</b> ASCII data files are formatted in <i><b>UltraScan II</b></i> format,\n");
      ts << tr("more information on this format, including row/column descriptions, can\n");
      ts << tr("be found ") << "<a href=http://www.ultrascan.uthscsa.edu/manual/file_format.html>" << tr("here") << "</a>.<p>\n";
      ts << "\n<p>\n";
      ts << tr("<b>Fitting Results:</b><p>\n<ul>\n");
      ps << tr("<b>Fitting Results:</b><p>\n<ul>\n");
      for (unsigned int i=0; i<modelString.size(); i++)
      {
         str1 = baseName + str2.sprintf("-%d", i) + ".res";
         testFile.setName(str1);
         if (testFile.exists())
         {
            str3 = htmlDir + "/" + projectName + str2 + ".res";

            copy(str1, str3);
            ts << tr("<p>\n   <li>Results for Model: ") << modelString[i] << "\n<p>\n\t<ul>\n";
            ps << tr("<p>\n   <li>Results for Model: ") << modelString[i] << "\n<p>\n\t<ul>\n";
            str1 = htmlDir + "/" + projectName + str2 + ".scans.html";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".scans.html>" << tr("Listing of included/excluded scans and their properties</a>\n<p>\n");
            }
            str1 = htmlDir + "/" + projectName + str2 + ".comments.html";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".comments.html>" << tr("Comments for this fit</a>\n");
            }
            str1 = htmlDir + "/" + projectName + str2 + ".conc_histogram.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".conc_histogram.png>" << tr("Molar Concentration Histogram Plot</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".conc_histogram.png>\n";
               ts << "\t\t<li><a href=" << projectName << str2 << ".conc_histogram.dat>" << tr("Molar Concentration Histogram Plot, ASCII plot data</a>\n");
            }
            str1 = htmlDir + "/" + projectName + str2 + ".od_histogram.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".od_histogram.png>" << tr("Optical Density Histogram Plot</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".od_histogram.png>\n";
               ts << "\t\t<li><a href=" << projectName << str2 << ".od_histogram.dat>" << tr("Optical Density Histogram Plot, ASCII plot data</a>\n");
            }
            ts << "\t\t<li><a href=" << projectName << str2 << ".res>" << tr("Fitting Report</a>\n");
            str1 = htmlDir + "/" + projectName + str2 + ".overlays-combined.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".overlays-combined.png>" << tr("Combined Overlays</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".overlays-combined.png>\n";
            }
            str1 = htmlDir + "/" + projectName + str2 + ".lnc-vs-r2.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".lnc-vs-r2.png>Log(C) vs. r<sup>2</sup></a>\n";
               ps << "\t\t<img src=" << projectName << str2 << ".lnc-vs-r2.png>\n";
            }
            str1 = htmlDir + "/" + projectName + str2 + ".mw-vs-r2.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".mw-vs-r2.png>" << tr("Molecular Weight vs. r<sup>2</sup></a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".mw-vs-r2.png>\n";
            }
            str1 = htmlDir + "/" + projectName + str2 + ".mw-vs-c.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".mw-vs-c.png>" << tr("Molecular Weight vs. Concentration</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".mw-vs-c.png>\n";
            }
            str1 = htmlDir + "/" + projectName + str2 + ".mw_histogram.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".mw_histogram.png>" << tr("Molecular Weight Histogram Plot</a>\n");
               ts << "\t\t<li><a href=" << projectName << str2 << ".mw_histogram.dat>" << tr("Molecular Weight Histogram Plot, ASCII plot data</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".mw_histogram.png>\n";
            }
            sets = 1;
            while (true)
            {
               str1 = htmlDir + "/" + projectName + str2 + ".overlays-set" + str3.sprintf("%d", sets) + ".png";
               if (testFile.exists(str1))
               {
                  ts << "\t\t<li><a href=" << projectName << str2 << ".overlays-set" << str3.sprintf("%d", sets)
                     << ".png>Grouped Overlays, Scans " << str4.sprintf("%d - %d", ((sets - 1) * 5 + 1),
                                                                        ((sets - 1) * 5 + 5)) << "</a>\n";
                  ps << "\t\t<img src=" << projectName << str2 << ".overlays-set" << str3.sprintf("%d", sets)
                     << ".png>\n";
                  sets++;
               }
               else
               {
                  break;
               }
            }
            str1 = htmlDir + "/" + projectName + str2 + ".residuals-combined.png";
            testFile.setName(str1);
            if (testFile.exists())
            {
               ts << "\t\t<li><a href=" << projectName << str2 << ".residuals-combined.png>" << tr("Combined Residuals</a>\n");
               ps << "\t\t<img src=" << projectName << str2 << ".residuals-combined.png>\n";
            }
            sets = 1;
            while (true)
            {
               str1 = htmlDir + "/" + projectName + str2 + ".residuals-set" + str3.sprintf("%d", sets) + ".png";
               if (testFile.exists(str1))
               {
                  ts << "\t\t<li><a href=" << projectName << str2 << ".residuals-set" << str3.sprintf("%d", sets)
                     << ".png>" << tr("Individual Residuals, Scans ") << str4.sprintf("%d - %d", ((sets - 1) * 5 + 1),
                                                                                      ((sets - 1) * 5 + 5)) << "</a>\n";
                  ps << "\t\t<img src=" << projectName << str2 << ".residuals-set" << str3.sprintf("%d", sets)
                     << ".png>\n";
                  sets++;
               }
               else
               {
                  break;
               }
            }
            modelDir = htmlDir+ "/" + projectName + str2;
            mD.mkdir(modelDir, true);
            sets = 1;
            QFile ascii;
            ascii.setName(modelDir + "/index.html");
            ts << "\t\t<li><a href=" << projectName << str2 << "/index.html>ASCII data files for scans, fits and residuals</a>";
            str1 = USglobal->config_list.help_dir + "/graphics/ultrascan1.png";
            str3 = modelDir + "/ultrascan1.png";
            copy(str1, str3);
            if (ascii.open(IO_WriteOnly | IO_Translate))
            {
               QTextStream ts_ascii(&ascii);
               // generate Header:

               str1 = tr("ASCII data for Scans from " + modelString[i] + ":");
               getHeader(&str3, str1);
               ts_ascii << str3;
               ts_ascii << "<p><b>"<< str1 << "</b>";
               ts_ascii << "<p>\n<ul>\n";
               while (true)
               {
                  str1 = baseName + str2 + "." + str3.sprintf("%d", sets) + ".dat";
                  if (testFile.exists(str1))
                  {
                     copy(str1, modelDir + "/" + projectName + str2 + "." + str3.sprintf("%d", sets) + ".dat");
                     ts_ascii << "\t<li><a href=" << projectName << str2 << "." << str3.sprintf("%d", sets) << ".dat>Scan " << str3 << "</a>\n";
                     sets++;
                  }
                  else
                  {
                     break;
                  }
               }
               ts_ascii << "<p>\n</ul>\n";
               getFooter(&str1);
               ts_ascii << str1;
               ascii.close();
            }
            if (i >= 4 && i <= 19)   // self-associating model, there might be a distribution file, lets check
            {
               str1 = baseName + str2 + ".dis";
               testFile.setName(str1);
               if (testFile.exists())
               {
                  str3 = htmlDir + "/" + projectName + str2 + ".dis";
                  copy(str1, str3);
                  ts << "\t\t<li><a href=" << projectName << str2 << ".dis>" << tr("Equilibrium Distribution of self-associating species</a> (ASCII data)\n");
               }
               str1 = htmlDir + "/" + projectName + str2 + ".distribution.png";
               testFile.setName(str1);
               if (testFile.exists())
               {
                  ts << "\t\t<li><a href=" << projectName << str2 << ".distribution.png>" << tr("Equilibrium Distribution of self-associating species</a> (Data Plot)\n");
                  ps << "\t\t<img src=" << projectName << str2 << ".distribution.png>\n";
               }
            }
            ts << "\t</ul>\n";
            ps << "\t</ul>\n";
         }
      }
      ts << "</ul>\n";
      ps << "</ul>\n";
      ts << "<p>\n<a href=printer.html><img src=printer.gif align=center border=0>" << tr("Printer-friendly Version - Graphs only</a> (Note: This may take a long time to load if you are on a slow link!)\n<p>\n");
      ps << "\n<p>\n<a href=index.html><img src=netscape.gif align=center border=0>" << tr("Browser-friendly Version</a> (Links to Graphs and Text Files)\n</center>\n<p>\n");
      getFooter(&str1);
      ts << str1;
      ps << str1;
      f1.close();
      f2.close();
   }
   fileName = indexFile;
   qApp->processEvents();
   QMessageBox mb("UltraScan", 
                  "The report has been written\n"
                  "Do you want to start the browser\nto view the Report?",
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No,
                  QMessageBox::Cancel | QMessageBox::Escape);
   mb.setButtonText(QMessageBox::Yes, "View Report");
   mb.setButtonText(QMessageBox::No, "Close");
   switch(mb.exec())
   {
   case QMessageBox::Yes:
      {
         view(fileName);
         break;
      }
   }
}

