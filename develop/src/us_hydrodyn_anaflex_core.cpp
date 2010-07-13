// us_hydrodyn.cpp contains class creation & gui connected functions
// us_hydrodyn_core.cpp contains the main computational routines
// us_hydrodyn_bd_core.cpp contains the main computational routines for brownian dynamic browflex computations
// (this) us_hydrodyn_anaflex_core.cpp contains the main computational routines for brownian dynamic (anaflex) computations
// us_hydrodyn_dmd_core.cpp contains the main computational routines for molecular dynamic (dmd) computations
// us_hydrodyn_other.cpp contains other routines such as file i/o

// includes and defines need cleanup

#include "../include/us_hydrodyn.h"
#include <qregexp.h>
#include <values.h>

#ifndef WIN32
#   include <unistd.h>
#   define SLASH "/"
#else
#   include <direct.h>
#   include <io.h>
#   define SLASH "\\"
#endif

// ---------- create anaflex files
// ---------- create one for each mode that is selected
int US_Hydrodyn::create_anaflex_files()
{
   // right now we are just going to support one-mode per run
   // in future we will support multiple, which will require the
   // following code
   //   if ( anaflex_options.run_mode_1 )
   //   {
   //      create_anaflex_files(1);
   //   }
   //   if ( anaflex_options.run_mode_2 )
   //   {
   //      create_anaflex_files(2);
   //   }
   //   if ( anaflex_options.run_mode_3 )
   //   {
   //      create_anaflex_files(3);
   //   }
   //   if ( anaflex_options.run_mode_4 )
   //   {
   //      create_anaflex_files(4);
   //   }
   //   if ( anaflex_options.run_mode_9 )
   //   {
   //      create_anaflex_files(9);
   //   }
   switch ( anaflex_options.run_mode )
   {
   case 0 : 
   case 1 : 
   case 2 : 
   case 3 : 
      return create_anaflex_files( anaflex_options.run_mode + 1);
      break;
   case 4 : 
      return create_anaflex_files( 9 );
      break;
   default :
      editor->append("unexpected case type (create_anaflex_files)!\n");
      return -1;
   }
   return -2; // shouldn't get here!
}

int US_Hydrodyn::create_anaflex_files( int use_mode )
{
   editor->append(tr(QString("Creating anaflex files (mode %1)\n").arg(use_mode)));

   QString filename = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + QString("-af%1").arg(use_mode);
   QString bffilename = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + "-bf";
   QString basename = 
      somo_dir + SLASH + filename;

   // anaflex-main.txt
   QFile f;
   // main file
   {
      f.setName(basename + "-main.txt");
      if ( !f.open(IO_WriteOnly) )
      {
         editor->append(QString("File write error: can't create %1\n").arg(f.name()));
         return -1;
      }
      QTextStream ts(&f);
      ts <<
         QString(
                 "%1-log.txt            !outputfile 1\n"
                 "%2-res.txt            !outputfile 2\n"
                 "%3-sum.txt            !outputfile 3\n"
                 "%4                    !sampling frequency\n"
                 "%5                    !instprofiles\n"
                 "%6                    !mode\n"
                 )
         .arg(filename)
         .arg(filename)
         .arg(filename)
         .arg(anaflex_options.nfrec)
         .arg(anaflex_options.instprofiles)
         .arg(use_mode)
         ;
      ts << "mode dependent stuff goes here\n";

      ts <<
         QString(
                 "%1-tra.txt                         !trajectory file\n" 
                 "*\n\n\n"
                 )
         .arg(bffilename);

      f.close();
   }
   return 0;
}

// ------------- run anaflex  --------------

int US_Hydrodyn::run_anaflex()
{
   // possible setup a new text window for the anaflex runs?
   QString dir = somo_dir;
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
#endif
      + SLASH
      + "anaflex2a-"
#if defined(WIN32)
      + "msd"
#else
      + "lnx"
#endif
      + ".exe";

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(QString("Anaflex program '%1' does not exist\n").arg(prog));
         editor->setColor(save_color);
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         QColor save_color = editor->color();
         editor->setColor("red");
         editor->append(QString("Anaflex program '%1' is not executable\n").arg(prog));
         editor->setColor(save_color);
         return -1;
      }
   }

   QString anafile = 
      project + QString("_%1").arg(current_model + 1) +
      QString(bead_model_suffix.length() ? ("-" + bead_model_suffix) : "")
      + "-af-main.txt\n" ;

   cout << QString("run anaflex dir <%1> prog <%2> stdin <%3>\n")
      .arg(dir)
      .arg(prog)
      .arg(anafile);
   anaflex = new QProcess( this );
   anaflex->setWorkingDirectory( dir );
   anaflex->addArgument( prog );
   connect( anaflex, SIGNAL(readyReadStdout()), this, SLOT(anaflex_readFromStdout()) );
   connect( anaflex, SIGNAL(readyReadStderr()), this, SLOT(anaflex_readFromStderr()) );
   connect( anaflex, SIGNAL(processExited()), this, SLOT(anaflex_processExited()) );
   connect( anaflex, SIGNAL(launchFinished()), this, SLOT(anaflex_launchFinished()) );

   anaflex->launch( anafile );

   return 0;
}

void US_Hydrodyn::anaflex_readFromStdout()
{
   QColor save_color = editor->color();
   while ( anaflex->canReadLineStdout() )
   {
      editor->setColor("brown");
      editor->append(anaflex->readLineStdout() + "\n");
      editor->setColor(save_color);
   }
   qApp->processEvents();
}
   
void US_Hydrodyn::anaflex_readFromStderr()
{
   QColor save_color = editor->color();
   while ( anaflex->canReadLineStderr() )
   {
      editor->setColor("red");
      editor->append(anaflex->readLineStderr() + "\n");
      editor->setColor(save_color);
   }
   qApp->processEvents();
}
   
void US_Hydrodyn::anaflex_processExited()
{
   QColor save_color = editor->color();
   editor->setColor("brown");
   editor->append("Anaflex process exited\n");
   editor->setColor(save_color);
   anaflex_readFromStderr();
   anaflex_readFromStdout();
   disconnect( anaflex, SIGNAL(readyReadStdout()), 0, 0);
   disconnect( anaflex, SIGNAL(readyReadStderr()), 0, 0);
   disconnect( anaflex, SIGNAL(processExited()), 0, 0);
   for ( current_model = 0; 
         current_model < (unsigned int)lb_model->numRows(); 
         current_model++)
   {
      if ( lb_model->isSelected(current_model) )
      {
         pb_bd->setEnabled( true );
         break;
      }
   }
}
   
void US_Hydrodyn::anaflex_launchFinished()
{
   QColor save_color = editor->color();
   editor->setColor("brown");
   editor->append("Anaflex launch exited\n");
   editor->setColor(save_color);
   disconnect( anaflex, SIGNAL(launchFinished()), 0, 0);
}
