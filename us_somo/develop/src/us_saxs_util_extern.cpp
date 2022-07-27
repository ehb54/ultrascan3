#include "../include/us_saxs_util.h"
#include "../include/us_file_util.h"

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define IQQ_TIMER

#if defined(IQQ_TIMER)
#  include "../include/us_timer.h"
//Added by qt3to4:
#include <QTextStream>
#endif

#define SLASH QDir::separator()

bool US_Saxs_Util::run_saxs_iq_foxs()
{
   errormsg = "";
   noticemsg = "";
      
   QString pdb = control_parameters[ "inputfile" ];

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "foxs" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi( prog );
      if ( !qfi.exists() )
      {
         errormsg = QString( "FoXS program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString( "FoXS program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "FoXS called but PDB file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString cmd = 
      QString( "%1 -q %2 -s %3 %4\n" )
      .arg( prog )
      .arg( our_saxs_options.end_q )
      .arg( (unsigned int)( 1 + our_saxs_options.end_q / our_saxs_options.delta_q ) )
      .arg( pdb );

#if defined(IQQ_TIMER)
   cout << "timer enabled\n";
#endif

   cout << "Starting FoXS\n";
   cout << cmd << endl;
#if defined(IQQ_TIMER)
   US_Timer iqq_timers;
   iqq_timers.init_timer ( "iqq foxs" );
   iqq_timers.start_timer( "iqq foxs" );
#endif
   system( cmd.toLatin1().data() );
#if defined(IQQ_TIMER)
   {
      iqq_timers.end_timer  ( "iqq foxs" );
      cout << iqq_timers.list_times() << flush;
      timings = iqq_timers.list_times();
      QString file = 
         QString( "%1-q%2-%3.timing" )
         .arg( control_parameters[ "outputfile" ] )
         .arg( (unsigned int)( 1 + our_saxs_options.end_q / our_saxs_options.delta_q ) )
         .arg( control_parameters[ "iqmethod" ] )
         ;
      write_timings( file, file );
   }
#endif
   cout << "FoXS finished.\n";

   // foxs creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = pdb + ".dat";
   QString created_plt = pdb;
   created_plt.replace( QRegExp( "\\.(pdb|PDB)$" ), ".plt" );
   
   if ( !QFile::exists( created_dat ) )
   {
      errormsg =  QString( "Error: FoXS did not create file %1" ).arg( created_dat );
      return false;
   }

   if ( !QFile::exists( created_plt ) )
   {
      errormsg = QString( "Notice: FoXS did not create file %1" ).arg( created_dat );
      return false;
   } else {
      if ( !QFile::remove( created_plt ) )
      {
         noticemsg += QString( "Notice: remove of FoXS created file %1 failed" ).arg( created_dat );
      }
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      QFileInfo( pdb ).fileName() + iqq_suffix() + ".dat";

   if ( QFile::exists( new_created_dat ) )
   {
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( created_dat != new_created_dat &&
        !qd.rename( created_dat, new_created_dat ) )
   {
      noticemsg +=
         QString( "Notice: could not rename %1 to %2" )
         .arg( created_dat )
         .arg( new_created_dat );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   if ( !load_saxs( new_created_dat ) )
   {
      return false;
   }
   return true;
}

// -------------------- crysol ------------------------------

bool US_Saxs_Util::run_saxs_iq_crysol()
{
   QString pdb = control_parameters[ "inputfile" ];
   if ( pdb.contains( "../common/" ) )
   {
      QString pdb_local = QFileInfo( pdb ).fileName();
      US_File_Util  ufu;
      ufu.copy( pdb, pdb_local );
      pdb = pdb_local;
   }

   QString prog = 
      env_ultrascan + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "crysol" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         errormsg = QString("Crysol program '%1' does not exist\n" ).arg( prog );
         return false;
      }
      if ( !qfi.isExecutable() )
      {
         errormsg = QString("Crysol program '%1' is not executable\n" ).arg( prog );
         return false;
      }
   }
   
   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      errormsg = QString( "Crysol called but PDB file '%1' does not exist\n" ).arg( pdb );
      return false;
   }

   QString crysol_last_pdb = pdb;
   crysol_last_pdb.replace( QRegExp( "\\.(pdb|PDB)$" ), ".pdb" );
   QString crysol_last_pdb_base = QFileInfo( crysol_last_pdb ).fileName().replace( QRegExp( "\\.(pdb|PDB)$" ), "").left( 6 ) + ".pdb";
   QString use_pdb = pdb;
   
   // copy pdb if the name is too long
   if ( our_saxs_options.crysol_version_26 &&
        QFileInfo( crysol_last_pdb ).fileName() != QFileInfo( crysol_last_pdb_base ).fileName() )
   {
      cout << QString(  "copying pdb: %1 %2\n" ).arg( crysol_last_pdb ).arg( crysol_last_pdb_base );
      QFile f( pdb );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         errormsg = QString( "Could not open file %1. Check permissions\n" ).arg( pdb );
         return false;
      }

      QFile f2( crysol_last_pdb_base );
      if ( !f2.open( QIODevice::WriteOnly ) )
      {
         errormsg = QString( "Could not open file %1. Check permissions\n" ).arg( crysol_last_pdb_base );
         return false;
      }

      QString qs;
      QTextStream ts( &f );
      QTextStream ts2( &f2 );

      while ( !ts.atEnd() )
      {
         qs = ts.readLine();
         ts2 << qs << Qt::endl;
      }
      f.close();
      f2.close();
      use_pdb = crysol_last_pdb_base;
   } else {
      use_pdb = pdb;
   }
   
   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "crysol_last_pdb_base: <" << crysol_last_pdb_base << ">\n";

   // clean up so we have new files

   {
      QString base = crysol_last_pdb_base.replace( QRegExp("\\.(pdb|PDB)$" ), "" );
      cout << "base: <" << base << ">\n";

      QString to_remove = base + "00.alm";
      cout << "to_remove: <" << to_remove << ">\n";
      QFile::remove( to_remove );

      to_remove = base + "00.log";
      QFile::remove( to_remove );

      to_remove = base + "00.int";
      QFile::remove( to_remove );
   }      

   QString cmd = 
      QString( "%1" ).arg( prog );

   cmd += 
      QString( " %1" )
      .arg( our_saxs_options.crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );

   cmd += 
      QString( " /sm %1" )
      .arg( our_saxs_options.end_q );

   cmd += 
      QString( " /ns %1" )
      .arg( (unsigned int)( 1 + our_saxs_options.end_q / our_saxs_options.delta_q ) );

   cmd += 
      QString( " /dns %1" )
      .arg( our_saxs_options.water_e_density );

   cmd += 
      QString( " /dro %1" )
      .arg( our_saxs_options.crysol_hydration_shell_contrast );

   cmd += 
      QString( " /lm %1" )
      .arg( our_saxs_options.sh_max_harmonics );

   cmd += 
      QString( " /fb %1" )
      .arg( our_saxs_options.sh_fibonacci_grid_order );

#if defined(IQQ_TIMER)
   cout << "timer enabled\n";
#endif

   cout << "Starting Crysol\n";
   cout << cmd << endl;
#if defined(IQQ_TIMER)
   US_Timer iqq_timers;
   iqq_timers.init_timer ( "iqq crysol" );
   iqq_timers.start_timer( "iqq crysol" );
#endif
   system( cmd.toLatin1().data() );
#if defined(IQQ_TIMER)
   {
      iqq_timers.end_timer  ( "iqq crysol" );
      cout << iqq_timers.list_times() << flush;
      timings = iqq_timers.list_times();
      QString file = 
         QString( "%1-q%2-%3.timing" )
         .arg( control_parameters[ "outputfile" ] )
         .arg( (unsigned int)( 1 + our_saxs_options.end_q / our_saxs_options.delta_q ) )
         .arg( control_parameters[ "iqmethod" ] )
         ;
      write_timings( file, file );
   }
#endif
   cout << "Crysol Finished\n";
   
   // crysol creates 4 files:
   // crysol_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   QString created_dat = crysol_last_pdb_base.replace( QRegExp( "\\.(pdb|PDB)$" ), "" ) +  "00.int";

   if ( !QFile::exists( created_dat ) )
   {
      errormsg = QString( "Error: Crysol did not create file %1" ).arg( created_dat );
      return false;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      QFileInfo( crysol_last_pdb.replace( QRegExp( "\\.(pdb|PDB)$" ), "" ) ).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( created_dat != new_created_dat &&
        !qd.rename( created_dat, new_created_dat ) )
   {
      noticemsg += 
         QString( "Notice: could not rename %1 to %2" )
         .arg( created_dat )
         .arg( new_created_dat );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   if ( !load_saxs( new_created_dat ) )
   {
      return false;
   }
   return true;
}
   

// -------------------- cryson ------------------------------

bool US_Saxs_Util::run_sans_iq_cryson()
{
   errormsg = "cryson currently unimplemented";
   return false;
}

bool US_Saxs_Util::load_saxs( QString filename  )
{
   errormsg = "";
   if ( filename.isEmpty() )
   {
      errormsg = "load_saxs() called with empty filename";
      return false;
   }

   QFile f(filename);
   QString ext = QFileInfo( filename ).suffix().toLower();

   if ( ext == "pdb" || ext == "PDB" )
   {
      errormsg = QString( "Error: load_saxs() can not load a PDB file as a curve: %1" ).arg( filename );
      return false;
   }

   if ( ext == "csv" || ext == "CSV" )
   {
      errormsg = QString( "Error: load_saxs() can not load a csv file as a curve: %1" ).arg( filename );
      return false;
   }

   vector < double > I;
   vector < double > I_error;
   vector < double > I2;
   vector < double > q;
   vector < double > q2;
   double new_I;
   double new_q;
   unsigned int Icolumn = 1;
   // unsigned int I_errorcolumn = 0;
   bool dolog10 = false;
   QString res = "";
   QString tag1;
   QString tag2;

   // bool do_crop = false;

   if ( f.open( QIODevice::ReadOnly ) )
   {
      QTextStream ts(&f);
      vector < QString > qv;
      QStringList qsl;
      while ( !ts.atEnd() )
      {
         QString qs = ts.readLine();
         qv.push_back(qs);
         qsl << qs;
      }
      f.close();
      if ( !qv.size() )
      {
         errormsg = QString( "Error: load_saxs() The file %1 is empty" ).arg( filename );
         return false;
      }

      // unsigned int number_of_fields = 0;
      if ( qv.size() > 3 )
      {
         QString test_line = qv[2];
         test_line.replace(QRegExp("^\\s+"),"");
         test_line.replace(QRegExp("\\s+$"),"");
         QStringList test_list = (test_line).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         // number_of_fields = test_list.size();
         // cout << "number of fields: " << number_of_fields << endl;
      }

      if ( ext == "int" ) 
      {
         //         dolog10 = true;
         Icolumn = 1;
      }
      if ( ext == "dat" ) 
      {
         // foxs?
         // do_crop = false;

         Icolumn = 1;
         // I_errorcolumn = 2;
         // if ( qsl.filter("exp_intensity").size() )
         // {
         //    I_errorcolumn = 0;
         // }
      }
      if ( ext == "fit" ) 
      {
         // do_crop = false;

         Icolumn = 2;
         // I_errorcolumn = 0;
      }
      if ( ext == "ssaxs" ) 
      {
         Icolumn = 1;
      }
      double units = 1.0;

      QRegExp rx_ok_line("^(\\s+|\\d+|\\.|\\d(E|e)(\\+|-|\\d))+$");
      rx_ok_line.setMinimal( true );
      for ( unsigned int i = 1; i < (unsigned int) qv.size(); i++ )
      {
         if ( qv[i].contains(QRegExp("^#")) ||
              rx_ok_line.indexIn( qv[i] ) == -1 )
         {
            // cout << "not ok: " << qv[i] << endl; 
            continue;
         }
         
         // QStringList tokens = (qv[i].replace(QRegExp("^\\s+").split( QRegExp("\\s+") , Qt::SkipEmptyParts ),""));
         QStringList tokens;
         {
            QString qs = qv[i].replace(QRegExp("^\\s+"),"");
            tokens = (qs ).split( QRegExp("\\s+") , Qt::SkipEmptyParts );
         }
         if ( (unsigned int) tokens.size() > Icolumn )
         {
            new_q = tokens[0].toDouble();
            new_I = tokens[Icolumn].toDouble();
            if ( dolog10 )
            {
               new_I = log10(new_I);
            }
            I.push_back(new_I);
            q.push_back(new_q * units);
         }
      }

      cout << "q_range after load: " << q[0] << " , " << q[q.size() - 1] << endl;

      cout << QFileInfo(filename).fileName() << endl;

      if ( q.size() )
      {
         if ( !write_output( "0", q, I ) )
         {
            return false;
         }
      }
   }
   return true;
}
