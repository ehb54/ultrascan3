#include "../include/us_montecarloreport.h"

US_Report_MonteCarlo::US_Report_MonteCarlo( 
                                           bool flag, QWidget* p, const char* name): QWidget( p, name )
{
   USglobal = new US_Config();

   if ( flag ) load(); 
}

US_Report_MonteCarlo::US_Report_MonteCarlo( 
                                           QString project, QWidget* p, const char* name): QWidget( p, name )
{
   USglobal = new US_Config();

   QString resultDir = USglobal->config_list.result_dir + "/";

   generate( resultDir + project + ".Monte-Carlo" );
}

void US_Report_MonteCarlo::view( const QString& fileName )
{
   US_Help* online_help = new US_Help( this );
   online_help->show_html_file( fileName );
}

void US_Report_MonteCarlo::load()
{
   QString resultDir = USglobal->config_list.result_dir;

   QString fileName = 
      QFileDialog::getOpenFileName( resultDir, "*.Monte-Carlo", 0 );
   
   if ( fileName.isEmpty() )
   {
      close();
   }
   else
   {
      generate( fileName );
   }
}

void US_Report_MonteCarlo::generate( const QString& fileName )
{
   QString str2;

   QFile projectFile( fileName );

   projectFile.open( IO_ReadOnly );
   QTextStream project_ts( &projectFile );
   
   QString projectName = project_ts.readLine();
   QString str1        = project_ts.readLine();
   
   vector <QString> parameter_name;
   unsigned int     parameters = str1.toUInt();
   
   for ( unsigned int i = 0; i < parameters; i++ )
   {
      parameter_name.push_back( project_ts.readLine() );
   }
   
   projectFile.close();

   QString reportDir = USglobal->config_list.html_dir + "/" + projectName + ".mc";
   QString helpDir   = USglobal->config_list.help_dir + "/";

   QString indexFile = reportDir + "/index.html";
   QFile f1( indexFile );
   
   QString printerFile = reportDir + "/printer.html";
   QFile f2( printerFile );

   QString baseName    = reportDir + "/parameter-";
   
   if ( f1.open( IO_WriteOnly | IO_Translate ) && 
        f2.open( IO_WriteOnly | IO_Translate ) )
   {
      QString helpDir = USglobal->config_list.help_dir + "/";

      str1 = helpDir   + "graphics/ultrascan1.png";
      str2 = reportDir + "/ultrascan1.png";
      copy( str1, str2 );
      
      str1 = helpDir   + "graphics/printer.gif";
      str2 = reportDir + "/printer.gif";
      copy( str1, str2 );

      str1 = helpDir   + "graphics/netscape.gif";
      str2 = reportDir + "/netscape.gif";
      copy( str1, str2 );
      
      str1 = USglobal->config_list.help_dir + "/mainstyle.css";
      str2 = reportDir + "/mainstyle.css";
      copy( str1, str2 );
      
      QTextStream ts( &f1 );
      QTextStream ps( &f2 );

      // generate Header:

      str1 = tr( "Monte Carlo Report for: " ) + projectName;
      getHeader( &str2, str1 );
      
      ts << str2;
      ps << str2;

      str1 = tr( "Monte Carlo Report for: " ) + "<i>" + projectName + "</i>";

      ts << "<p><h2>" << str1 << "</h2>\n<p>\n";
      ps << "<p><h2>" << str1 << "</h2>\n<p>\n";
      
      ts << tr( "<b>Please note:</b> ASCII data files are formatted in "
                "<i><b>UltraScan II</b></i> format,\n" );
      
      ts << tr( "more information on this format, including row/column "
                "descriptions, can\n"
                "be found" ) 
         << " <a href=http://www.ultrascan.uthscsa.edu/manual/file_format.html>" 
         << tr( "here" ) 
         << "</a>.<p>\n";

      ts << tr( "<b>Monte Carlo Results:</b><p>\n<ul>\n" );
      ps << tr( "<b>Monte Carlo Results:</b><p>\n<ul>\n" );

      str1 = reportDir + projectName + ".res";

      QFile testFile( str1 );

      if ( testFile.exists() )
      {
         ts << "\t<li><a href=" << projectName <<".res>" 
            << tr("Monte Carlo Settings") << "</a>\n\t<p>\n";
      }

      if ( projectName.right( 1 ) == "3" )
      {
         ts << "\t<li>" 
            << tr( "Variance for Fixed Molecular Weight Distribution: <br>\n"
                   "\t<ul>\n" );

         testFile.setName( reportDir + "variance.res" );
         
         if (testFile.exists())
         {
            ts << "\t\t<li><a href=variance.res>" 
               << tr("Statistics for Variance</a>\n");
         }

         testFile.setName( reportDir + "variance.png");

         if ( testFile.exists() )
         {
            ps << "\t<li>" << tr("Histogram for Variance:\n<p>\n");
            ps << "\t<img src=variance.png><p>\n";
            ts << "\t\t<li><a href=variance.png>" 
               << tr("Variance Histogram Plot") << "</a>\n";
         }

         testFile.setName( reportDir + "/parameter-0.dat" );

         if ( testFile.exists() )
         {
            ts << "\t\t<li><a href=parameter-0.dat>" 
               << tr("ASCII Data for Histogram Plot") << "</a>\n";
         }

         ts << "\t</ul>\n\t<p>\n";
         ts << "\t<li>" << tr("Molecular Weight Distribution:<br>\n\t<ul>\n");

         testFile.setName( reportDir + "/variance.png" );

         if ( testFile.exists() )
         {
            ps << "\t<li>" 
               << tr("Histogram for Molecular Weight Distribution:\n<p>\n");
            ps << "\t<img src=mw-distro.png><p>\n";
            ts << "\t\t<li><a href=mw-distro.png>" 
               << tr("Molecular Weight Distribution Histogram Plot") << "</a>\n";
         }

         testFile.setName( reportDir + "/mw-distro.dat" );

         if ( testFile.exists() )
         {
            ts << "\t\t<li><a href=mw-distro.dat>" 
               << tr("ASCII Data for Molecular Weight Histogram Plot") << "</a>\n";
         }

         testFile.setName( reportDir + "/mw-distro.res");

         if ( testFile.exists() )
         {
            ts << "\t\t<li><a href=mw-distro.res>" 
               << tr("Molecular Weight Averages") << "</a>\n";
         }

         ts << "\t</ul>\n\t<p>\n";
      }
      else
      {
         for ( unsigned int i = 0; i < parameter_name.size(); i++ )
         {
            ts << "\t<li>" << tr("Results for Parameter ") 
               << parameter_name[i] << ":<br>\n\t<ul>\n";

            str1 = baseName + str2.sprintf( "%d.", i );

            testFile.setName( str1 + "res" );

            if ( testFile.exists() )
            {
               ts << "\t\t<li><a href=parameter-" << str2 
                  << "res>" << tr("Parameter Statistics</a>\n");
            }

            testFile.setName( str1 + "png" );

            if ( testFile.exists() )
            {
               ps << "\t<li>" << tr("Histogram for Parameter ") 
                  << parameter_name[i] << ":\n<p>\n";

               ps << "\t<img src=parameter-" << str2 << "png><p>\n";
               ts << "\t\t<li><a href=parameter-" << str2 << "png>" 
                  << tr("Histogram Plot") << "</a>\n";
            }

            testFile.setName( str1 + "dat" );

            if ( testFile.exists() )
            {
               ts << "\t\t<li><a href=parameter-" << str2 << "dat>" 
                  << tr("ASCII Data for Histogram Plot") << "</a>\n";
            }

            ts << "\t</ul>\n\t<p>\n";
         }
      }

      ts << "</ul>\n";
      ps << "</ul>\n";
      ts << "<p>\n<a href='printer.html'><img src='printer.gif' "
         "align='center' border='0'>" 
         << tr( "Printer-friendly Version - Graphs only</a> (Note: This may "
                "take a long time to load if you are on a slow link!)\n<p>\n" );

      ps << "\n<p>\n<a href='index.html'><img src='netscape.gif' align='center' "
         "border='0'>" 
         << tr("Browser-friendly Version</a> (Links to Graphs and Text Files)\n"
               "</center>\n<p>\n");

      getFooter( &str1 );

      ts << str1;
      ps << str1;

      f1.close();
      f2.close();
   }

   qApp->processEvents();
   
   QMessageBox mb(
                  tr( "UltraScan" ), 
                  tr( "The report has been written\n"
                      "Do you want to start the browser\nto view the Report?"),
   
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No,
                  QMessageBox::Cancel | QMessageBox::Escape );

   mb.setButtonText( QMessageBox::Yes, tr( "View Report" ) );
   mb.setButtonText( QMessageBox::No,  tr( "Close"       ) );
   
   switch ( mb.exec() )
   {
   case QMessageBox::Yes:
      {
         view( indexFile );
         break;
      }
   }
}

