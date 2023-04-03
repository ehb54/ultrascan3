#include "../include/us_hydrodyn.h"
#include "../include/us_write_config.h"
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

void process_script(QString, US_Hydrodyn *);

int main (int argc, char **argv)
{
   QDir *dir = new QDir(QDir::currentPath());
   QLocale::setDefault( QLocale::c() );
   QApplication a(argc, argv);

   bool expert                = false;
   bool debug                 = false;
   bool residue_file          = false;
   bool script                = false;
   bool init_configs_silently = false;
   bool cli_progress          = false;

   QString residue_filename;
   QString script_filename;
   QString gui_script_filename = "";

   US_Hydrodyn *hydrodyn;
   vector < QString > batch_file;
   int argcbase = 1;
#ifdef Q_OS_MAC
   argcbase     = 2;
#endif
   while ( a.arguments().size() > argcbase &&
      	  QString(a.arguments()[argcbase]).contains(QRegExp("^-")) )
   {
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-e")) )
      {
         if ( debug )
         {
            puts("expert mode");
         }
         argcbase++;
         expert = true;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-d")) )
      {
         puts("debug mode");
         argcbase++;
         debug = true;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-r")) )
      {
         if ( debug )
         {
            puts("residue file");
         }
         argcbase++;
         residue_file = true;
         residue_filename = a.arguments()[argcbase];
         argcbase++;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-c")) )
      {
         if ( debug )
         {
            puts("script file");
         }
         argcbase++;
         script = true;
         script_filename = a.arguments()[argcbase];
         argcbase++;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-I")) )
      {
         if ( debug )
         {
            puts("init configs silently");
         }
         argcbase++;
         init_configs_silently = true;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-p")) )
      {
         if ( debug )
         {
            puts("cli progress");
         }
         argcbase++;
         cli_progress = true;
         continue;
      }
      if ( QString(a.arguments()[argcbase]).contains(QRegExp("^-g")) )
      {
         if ( debug )
         {
            puts("gui script file");
         }
         argcbase++;
         // gui_script = true;
         gui_script_filename = a.arguments()[argcbase];
         if ( !QFile( gui_script_filename ).exists() ) {
            QTextStream( stderr ) << QString( "file %1 does not exist\n" ).arg( gui_script_filename );
            exit(-1);
         }
         gui_script_filename = QFileInfo( gui_script_filename ).absoluteFilePath();
         argcbase++;
         continue;
      }
   }
   for ( int i = argcbase; i < a.arguments().size(); i++ ) 
   {
      batch_file.push_back( dir->filePath(a.arguments()[i]) );      
   }
   hydrodyn = new US_Hydrodyn(batch_file, gui_script_filename, init_configs_silently );
      
   hydrodyn->setWindowTitle("SOMO Solution Modeler");
   if ( residue_file )
   {
      hydrodyn->residue_filename = residue_filename;
      hydrodyn->read_residue_file();
      hydrodyn->lbl_table->setText( QDir::toNativeSeparators( residue_filename ) );
   }
   if ( expert )
   {
      hydrodyn->advanced_config.auto_view_pdb = false;
      hydrodyn->advanced_config.scroll_editor = true;
      hydrodyn->advanced_config.expert_mode   = true;
   }
   hydrodyn->set_expert( expert );
   hydrodyn->cli_progress = cli_progress;

   if ( debug )
   {
      hydrodyn->advanced_config.debug_1 = true;
      hydrodyn->advanced_config.debug_2 = true;
      hydrodyn->advanced_config.debug_3 = true;
      hydrodyn->advanced_config.debug_4 = true;
      hydrodyn->advanced_config.debug_5 = true;
      hydrodyn->advanced_config.debug_6 = true;
      hydrodyn->advanced_config.debug_7 = true;
   }
   if ( script )
   {
      process_script(script_filename,hydrodyn);
      hydrodyn->guiFlag = true;
   }
   hydrodyn->show();
 //   a.setMainWidget(hydrodyn);
   a.setDesktopSettingsAware(false);
#if QT_VERSION >= 0x040000
   {
      QString icon = 
         hydrodyn->USglobal->config_list.system_dir + "/etc/" + "somo3_icon_128x128.ico";
      if ( QFile( icon ).exists() )
      {
         a.setWindowIcon( QIcon( icon ) );
      }
   }
#endif
   return a.exec();
}

void process_script(QString script_filename, US_Hydrodyn *h)
{
   h->numThreads = 1;
   QFile f(script_filename);
   if ( f.open(QIODevice::ReadOnly) )
   {
      h->read_residue_file();
      QTextStream ts(&f);
      QRegExp rx0("^(\\S+)(\\s+|$)");
      QRegExp rx1("^(\\S+)\\s*$");
      QRegExp rx2("^(\\S+)\\s*(\\S+)\\s*$");
      QRegExp rx3("^(\\S+)\\s*(\\S+)\\s*(\\S+)\\s*$");
      h->guiFlag = false;
      h->overwrite = true;
      h->setSuffix = false;
      int line = 0;
      QString loadfiletype = "";
      QString c;
      while ( !(c = ts.readLine()).isNull() )
      {
         cout << "somo> " << c << "\n";
         bool ok = false;

         line++;
         rx0.indexIn(c);
         rx1.indexIn(c);
         rx2.indexIn(c);
         rx3.indexIn(c);
         //         printf("rx0.cap(1) <%s> rx1.cap(1) <%s> rx2.cap(1) <%s>\n"
         //                ,rx0.cap(1).toLatin1().data()
         //                ,rx1.cap(1).toLatin1().data()
         //                ,rx2.cap(1).toLatin1().data()
         //                );
         if ( rx0.cap(1) == "exit" )
         {
            exit(0);
         }
         if ( rx0.cap(1) == "reset" )
         {
            cout << "resetting to default configuration" << endl;
            ok = true;
         }
         if ( rx2.cap(1) == "load" )
         {
            cout << QString("loading \"%1\"\t").arg(rx2.cap(2));
            bool result = false;
            if ( rx2.cap(2).toLower().contains(QRegExp("\\.config$")) ) 
            {
               ok = true;
               int result2 = h->read_config(rx2.cap(2));
               result = !result2;
            }
            if ( rx2.cap(2).toLower().contains(QRegExp("\\.residue$")) ) 
            {
               ok = true;
               h->residue_filename = rx2.cap(2);
               h->read_residue_file();
               h->lbl_table->setText( QDir::toNativeSeparators( rx2.cap(2) ) );
            }
            if ( rx2.cap(2).toLower().contains(QRegExp(".pdb$")) ) 
            {
               ok = true;
               // no save/restore settings for load into somo
               result = h->screen_pdb(rx2.cap(2), false);
               loadfiletype = "pdb";
            }
            if ( rx2.cap(2).toLower().contains(QRegExp(".(bead_model|beams)$")) ) 
            {
               ok = true;
               result = h->screen_bead_model(rx2.cap(2));
               loadfiletype = "bead_model";
            }
            if ( ok )
            {
               cout << QString("%1\n").arg(result ? "ok" : "not ok");
            } else {
               cout << QString("unknown file load type on line %1: \"%2\"\n").arg(line).arg(c);
               ok = true;
            }
         }
         if ( rx0.cap(1) == "somo" )
         {
            ok = true;
            cout << QString("somo\t");
            bool result = h->calc_somo() ? false : true;
            cout << QString("%1\n").arg(result ? "ok" : "not ok");
         }
         if ( rx0.cap(1) == "grid" )
         {
            ok = true;
            cout << QString("grid\t");
            bool result =
               ( loadfiletype == "pdb" ? 
                 h->calc_grid_pdb() : h->calc_grid() )
               ? false : true;
            cout << QString("%1\n").arg(result ? "ok" : "not ok");
         }
         if ( rx0.cap(1) == "hydro" )
         {
            ok = true;
            cout << QString("hydro\t");
            bool result = h->calc_hydro() ? false : true;
            cout << QString("%1\n").arg(result ? "ok" : "not ok");
         }
         if ( rx0.cap(1) == "p(r)" )
         {
            ok = true;
            cout << QString("p(r) not yet implemented\t");
         }
         if ( rx0.cap(1) == "threads" )
         {
            ok = true;
            cout << QString("threads\t");
            US_Config *USglobal = new US_Config();
            cout << QString("Number of threads was %1, now %2\n").arg(USglobal->config_list.numThreads).arg(rx2.cap(2));
            USglobal->config_list.numThreads = rx2.cap(2).toInt();
            US_Write_Config *WConfig;
            WConfig = new US_Write_Config();
            bool result = WConfig->write_config(USglobal->config_list);
            cout << QString("%1\n").arg(result ? "ok" : "not ok");
         }
         if ( !ok )
         {
            cout << QString("Unknown command on line %1: \"%2\"\n").arg(line).arg(c);
         }
      }
      exit(0);
   } else {
      cerr << "Can not open file:" << script_filename << endl;
      exit(-1);
   }
}
