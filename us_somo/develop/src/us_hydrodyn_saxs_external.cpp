#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_hydrodyn.h"
#include "../include/us_cmdline_app.h"
#include "../include/us_hydrodyn_saxs_ift.h"
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

#define SLASH QDir::separator()

void US_Hydrodyn_Saxs::editor_msg( QColor color, QColor bgcolor, QString msg )
{
   msg.replace( QRegExp( "\n*$" ), "" );
   msg += "\n";
   // QColor save_color_bg = editor->paragraphBackgroundColor( editor->paragraphs() - 1 );
#if QT_VERSION < 0x040000
   editor->setParagraphBackgroundColor( editor->paragraphs() - 1,  bgcolor );
#else
   editor->setTextBackgroundColor( bgcolor );
#endif
   editor_msg( color, msg );
#if QT_VERSION < 0x040000
   editor->setParagraphBackgroundColor( editor->paragraphs() - 1,  "white" );
#else
   editor->setTextBackgroundColor( QColor( "white" ) );
#endif
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, QString msg )
{
   QColor save_color = editor->textColor();
   editor->setTextColor(color);
   editor->append(msg);
   editor->setTextColor(save_color);
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QString msg )
{
   editor_msg( QColor( color ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char * color, QString msg )
{
   editor_msg( QString( color ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QColor bgcolor, QString msg )
{
   editor_msg( QColor( color ), bgcolor, msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char *color, QColor bgcolor, QString msg )
{
   editor_msg( QColor( color ), bgcolor, msg );
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, QString bgcolor, QString msg )
{
   editor_msg( color, QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QColor color, const char * bgcolor, QString msg )
{
   editor_msg( color, QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( QString color, QString bgcolor, QString msg )
{
   editor_msg( QColor( color ), QColor( bgcolor ), msg );
}

void US_Hydrodyn_Saxs::editor_msg( const char * color, const char * bgcolor, QString msg )
{
   editor_msg( QColor( color ), QColor( bgcolor ), msg );
}

// -------------------- IFT ------------------------------

void US_Hydrodyn_Saxs::call_ift( bool rerun )
{
   us_qdebug( "ift()" );
   if ( external_running ) {
      US_Static::us_message( us_tr("US-SOMO SAXS IFT"), 
                            QString( us_tr("Please wait until the running job completes" ) ) );
      return;
   }

   if ( !qsl_plotted_iq_names.size() ) {
      US_Static::us_message( us_tr("US-SOMO SAXS IFT"), 
                            QString( us_tr("No I(q) curves loaded to process!" ) ) );
      return;
   }

   // check for zero error data, only add non-zero
   if ( rerun && !ift_to_process.size() ) {
      US_Static::us_message( us_tr("US-SOMO SAXS IFT"), 
                            QString( us_tr("Nothing selected to process" ) ) );
      return;
   }
      
   if ( !rerun ) {
      ift_to_process.clear( );
      for ( int i = 0; i < (int) qsl_plotted_iq_names.size(); ++i ) {
         if ( !plotted_iq_names_to_pos.count( qsl_plotted_iq_names[ i ] ) ) {
            editor_msg( "red", QString( "internal error: no plotted position info for curve %1" ).arg( qsl_plotted_iq_names[ i ] ) );
            return;
         }

         int ift_pos = plotted_iq_names_to_pos[ qsl_plotted_iq_names[ i ] ];

         if ( (int) plotted_q.size() <= ift_pos ||
              (int) plotted_I.size() <= ift_pos ||
              (int) plotted_I_error.size() <= ift_pos ) {
            editor_msg( "red", QString( "internal error: no plotted data for curve %1" ).arg( qsl_plotted_iq_names[ i ] ) );
            return;
         }

         if ( plotted_I_error[ ift_pos ].size() != plotted_I[ ift_pos ].size() ) {
            editor_msg( "dark red", QString( us_tr( "Notice: curve %1 skipped as it has no errors" ) ).arg( qsl_plotted_iq_names[ i ] ) );
            continue;
         }
         if ( !US_Saxs_Util::is_nonzero_vector( plotted_I_error[ ift_pos ] ) ) {
            editor_msg( "dark red", QString( us_tr( "Notice: curve %1 skipped as it has some zero value errors" ) ).arg( qsl_plotted_iq_names[ i ] ) );
            continue;
         }
         ift_to_process << qsl_plotted_iq_names[ i ];
      }

      if ( !ift_to_process.size() ) {
         US_Static::us_message( us_tr("US-SOMO SAXS IFT"), 
                                QString( us_tr("No I(q) curves with full errors available to process.\nCheck text area for details." ) ) );
         return;
      }

      if ( ift_to_process.size() > 1) {
         bool ok;
         QString res = US_Static::getItem(
                                          us_tr("US-SOMO SAXS IFT")
                                          ,us_tr( "Select the curve you wish to process:" )
                                          ,ift_to_process
                                          ,0
                                          ,false
                                          ,&ok
                                          ,this
                                          );
                                  
         if ( !ok ) {
            return;
         }
         ift_to_process.clear( );
         ift_to_process << res;
      }
   }

   // make sure program exists

   ift_prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "iftci" 
#if defined(Q_OS_WIN)
      + "_win64.exe"
#else
# if defined(Q_OS_MAC)
      + "_osx10.8"
# else
      + "_linux64"
# endif      
#endif      
         ;

   {
      QFileInfo qfi(ift_prog);
      if ( !qfi.exists() && ift_prog.contains( "bin64" ) ) {
         ift_prog = 
            USglobal->config_list.system_dir + SLASH +
            "bin"
            + SLASH
            + "iftci" 
#if defined(Q_OS_WIN)
            + "_win64.exe"
#else
# if defined(Q_OS_MAC)
            + "_osx10.8"
# else
            + "_linux64"
# endif      
#endif      
            ;
      }
   }

   {
      QFileInfo qfi(ift_prog);
      if ( !qfi.exists() )
      {
         editor_msg( (QString) "red", QString("IFT program '%1' does not exist\n").arg(ift_prog));
         return;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg( "red", QString("IFT program '%1' is not executable\n").arg(ift_prog));
         return;
      }
   }

   ift_parameters.clear( );
   if ( rerun ) {
      ift_parameters[ "rerun" ] = "true";
      if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
         ift_parameters[ "overwriteoff" ] = "true";
         ((US_Hydrodyn *) us_hydrodyn )->overwrite = true;
         qDebug() << "****> overwrite on";
      }
   }
   ift_last_processed = ift_to_process[ 0 ];
   if ( !plotted_iq_names_to_pos.count( ift_last_processed ) ) {
      editor_msg( "red", QString( "internal error: no plotted position info for curve %1" ).arg( ift_last_processed ) );
      return;
   }

   int ift_pos = plotted_iq_names_to_pos[ ift_last_processed ];

   if ( (int) plotted_q.size() <= ift_pos ||
        (int) plotted_I.size() <= ift_pos ) {
      editor_msg( "red", QString( "internal error: no plotted data for curve %1" ).arg( ift_last_processed ) );
      return;
   }

   if ( !plotted_q[ ift_pos ].size() ) {
      editor_msg( "red", QString( "internal error: empty plotted data for curve %1" ).arg( ift_last_processed ) );
      return;
   }
      
   ift_parameters[ "qmin" ] = QString( "%1" ).arg( plotted_q[ ift_pos ].front() );
   ift_parameters[ "qmax" ] = QString( "%1" ).arg( plotted_q[ ift_pos ].back() );
   ift_parameters[ "prpoints" ] = "50";

   // setup any preliminary params

   US_Hydrodyn_Saxs_Ift * ift_instance = 
      new US_Hydrodyn_Saxs_Ift( us_hydrodyn, & ift_parameters, this );
   US_Hydrodyn::fixWinButtons( ift_instance );
   ift_instance->exec();
   delete ift_instance;

   ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "buffer_nth_shown" ] =
      ift_parameters.count( "buffer_nth_shown" ) ? ift_parameters[ "buffer_nth_shown" ] : "";

   if ( !ift_parameters.count( "button" ) ||
        ift_parameters[ "button" ] != "go" )
   {
      return;
   }

   ift_process_next();
}
   
#define IFT_SHORTEN 25

void US_Hydrodyn_Saxs::ift_process_next() {
   
   if ( !ift_to_process.size() ) {
      if ( ift_parameters.count( "overwriteoff" ) && ift_parameters[ "overwriteoff" ] == "true" ) {
         ((US_Hydrodyn *) us_hydrodyn )->overwrite = false;
         qDebug() << "****> overwrite off";
      }
      return;
   }

   // setup temporary directory

   ift_tmp_path = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   while ( QDir( ift_tmp_path ).exists() ) {
      US_Saxs_Util::us_usleep( 1000 );
      ift_tmp_path = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + "/tmp/" + QDateTime::currentDateTime().toString( "yyMMddhhmmsszzz" );
   }

   {
      QDir qd;
      if ( !qd.mkdir( ift_tmp_path ) ) {
         US_Static::us_message( us_tr("US-SOMO SAXS IFT"), 
                               QString( us_tr("Could not create directory %1, check permissions" ) ).arg( ift_tmp_path ) );
         return;
      }
   }

   QDir::setCurrent( ift_tmp_path );
   us_qdebug( QString( "ift_tmpdir %1\n" ).arg( ift_tmp_path ) );
   
   ift_last_processed = ift_to_process[ 0 ];
   ift_to_process.pop_front();

   // create data file

   if ( !plotted_iq_names_to_pos.count( ift_last_processed ) ) {
      editor_msg( "red", QString( "internal error: no plotted position info for curve %1" ).arg( ift_last_processed ) );
      return;
   }

   int ift_pos = plotted_iq_names_to_pos[ ift_last_processed ];

   if ( (int) plotted_q.size() <= ift_pos ||
        (int) plotted_I.size() <= ift_pos ) {
      editor_msg( "red", QString( "internal error: no plotted data for curve %1" ).arg( ift_last_processed ) );
      return;
   }
   
   {
      US_Saxs_Util usu;
      usu.wave["data"].filename = ift_last_processed;
      usu.wave["data"].q = plotted_q[ ift_pos ];
      usu.wave["data"].r = plotted_I[ ift_pos ];
      if ( plotted_I_error.size() ) {
         usu.wave["data"].s = plotted_I_error[ ift_pos ];
      }
      if ( !usu.write( ift_last_processed.left( IFT_SHORTEN ), "data" ) ) {
         editor_msg( "red", QString( "Error: Could not write file %1. Disk space issue?" ).arg( ift_last_processed ) );
         return;
      }
         
   }

   us_qdebug( ift_last_processed );
      
   // setup inputfile.d (see bayesapp wrapper.pl)
   QFile f( "inputfile.d" );
   
   if ( !f.open( QIODevice::WriteOnly ) ) {
      editor_msg( "red", QString( "Error: Could not write file %1. Disk space issue?" ).arg( f.fileName() ) );
      return;
   }
   
   QTextStream ts( &f );
   ts
      //    # l1
      << ift_last_processed.left( IFT_SHORTEN ) << "\n"
      //    # l2
      << ( ift_parameters.count( "qmin" ) ? ift_parameters[ "qmin" ] : "" ) << "\n"
      //    # l3
      << ( ift_parameters.count( "qmax" ) ? ift_parameters[ "qmax" ] : "" ) << "\n"
      //    # l4
      << ( ( ift_parameters.count( "dmaxfixed" ) && ift_parameters[ "dmaxfixed" ] == "true" ) ? "f" : "" ) << ift_parameters[ "dmax" ] << "\n"
      //    # l5
      << ( ift_parameters.count( "eta" ) ? ift_parameters[ "eta" ] : "" ) << "\n"
      //    # l6
      << ( ( ift_parameters.count( "alphafixed" ) && ift_parameters[ "alphafixed" ] == "true" ) ? "f" : "" ) << ift_parameters[ "alpha" ] << "\n"
      //    # l7
      << ( ift_parameters.count( "smearing" ) ? ift_parameters[ "smearing" ] : "" ) << "\n"
      //    # l8
      << ( ( ift_parameters.count( "estimateratiofixed" ) && ift_parameters[ "estimateratiofixed" ] == "true" ) ? "f" : "" ) << ift_parameters[ "estimateratio" ] << "\n"
      //    # l9
      << ( ift_parameters.count( "fitratio" ) ? ift_parameters[ "fitratio" ] : "" ) << "\n"
      //    # l10
      << ( ift_parameters.count( "prpoints" ) ? ift_parameters[ "prpoints" ] : "" ) << "\n"
      //    # l11
      << ( ift_parameters.count( "noextracalc" ) ? ift_parameters[ "noextracalc" ] : "" ) << "\n"
      //    # l12
      << ( ift_parameters.count( "transform" ) ? ift_parameters[ "transform" ] : "" ) << "\n"
      //    # l13
      << ( ( ift_parameters.count( "fitbackground" ) && ift_parameters[ "fitbackground" ] == "true" )  ? "y" : "n" )  << "\n"
      //    # l14
      << ( ( ift_parameters.count( "smallplot" ) && ift_parameters[ "smallplot" ] == "true" ) ? "S" : "L" ) << "\n"
      ;

   f.close();
   
   // bayesapp perl version:
   // {
   //    # l1
   //    $files{'data'} = $$ref{ 'datafile' }[0];
   //    $files{'data'} =~ s/^.*\/([^\/]+)$/$1/;
   //    $inputfile .= $files{'data'} . "\n";
   //    # l2
   //    $inputfile .= $$ref{ 'qmin' } . "\n";
   //    # l3
   //    $inputfile .= $$ref{ 'qmax' } . "\n";
   //    # l4
   //    $inputfile .= ( $$ref{ 'dmaxfixed' } ? "f" : "" ) . $$ref{ 'dmax' } . "\n";
   //    # l5
   //    $inputfile .= $$ref{ 'eta' } . "\n";
   //    # l6
   //    $inputfile .= ( $$ref{ 'alphafixed' } ? "f" : "" ) . $$ref{ 'alpha' } . "\n";
   //    # l7
   //    $inputfile .= $$ref{ 'smearing' } . "\n";
   //    # l8
   //    $inputfile .= ( $$ref{ 'estimateratiofixed' } ? "f" : "" ) . $$ref{ 'estimateratio' } . "\n";
   //    # l9
   //    $inputfile .= $$ref{ 'fitratio' } . "\n";
   //    # l10
   //    $inputfile .= $$ref{ 'prpoints' } . "\n";
   //    # l11
   //    $inputfile .= $$ref{ 'noextracalc' } . "\n";
   //    # l12
   //    $inputfile .= $$ref{ 'transform' } . "\n";
   //    # l13
   //    $inputfile .= ( $$ref{ 'fitbackground' } ? "y" : "n" ) . "\n";
   //    # l14
   //    $inputfile .= ( $$ref{ 'smallplot' } ? "S" : "L" ) . "\n";
   // }

   // open OUT, ">inputfile.d";
   // print OUT $inputfile;
   // close OUT;

   ift_stdout = "";

   ift = new QProcess( this );
   //   ift->setWorkingDirectory( dir );
   us_qdebug( "prog is " + ift_prog );
#if QT_VERSION < 0x040000
   ift->addArgument( ift_prog );

   connect( ift, SIGNAL(readyReadStandardOutput()), this, SLOT(ift_readFromStdout()) );
   connect( ift, SIGNAL(readyReadStandardError()), this, SLOT(ift_readFromStderr()) );
   connect( ift, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(ift_finished( int, QProcess::ExitStatus )) );
   connect( ift, SIGNAL(started()), this, SLOT(ift_started()) );

   editor_msg( "black", "\nStarting IFT\n");
   ift->start();
   external_running = true;
#else
   {
      QStringList args;

      connect( ift, SIGNAL(readyReadStandardOutput()), this, SLOT(ift_readFromStdout()) );
      connect( ift, SIGNAL(readyReadStandardError()), this, SLOT(ift_readFromStderr()) );
      connect( ift, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(ift_finished( int, QProcess::ExitStatus )) );
      connect( ift, SIGNAL(started()), this, SLOT(ift_started()) );

      editor_msg( "black", "\nStarting IFT\n");
      ift->start( ift_prog, args, QIODevice::ReadOnly );
      external_running = true;
   }
#endif
   
   return;
}

void US_Hydrodyn_Saxs::ift_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( ift->canReadLineStdout() )
   {
      QString qs = ift->readLineStdout() + "\n";
      ift_stdout += qs;
      editor_msg("brown", qs );
   }
#else
   QString qs = QString( ift->readAllStandardOutput() );
   ift_stdout += qs;
   editor_msg( "brown", qs );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::ift_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( ift->canReadLineStderr() )
   {
      editor_msg("red", ift->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( ift->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}
   
bool US_Hydrodyn_Saxs::last_pr_rebin_save(
                                          const QString & header
                                          ,const QString & rxstr
                                          ,QStringList & created_files
                                          ) {
   vector < double > rebin_r;
   // new bin
   for ( double x = 0; x <= plotted_r.back().back(); x += our_saxs_options->bin_size ) {
      rebin_r.push_back( x );
   }
   vector < double > rebin_pr;
   vector < double > rebin_sd;

   interpolate( plotted_r.back(), rebin_r, plotted_pr.back(), rebin_pr );
   interpolate( plotted_r.back(), rebin_r, plotted_pr_error.back(), rebin_sd );
               
   // US_Vector::printvector3( "rebin'd r,pr,sd", rebin_r, rebin_pr, rebin_sd );

   QString dest_rebin =
      USglobal->config_list.root_dir
      + "/somo/saxs/"
      + QString( "%1" )
      .arg( ift_last_processed ).replace( QRegExp( rxstr )
                                          , QString("_bin%1_%2ift.dat" )
                                          .arg( our_saxs_options->bin_size )
                                          .arg( cb_normalize->isChecked() ? "normed_" : "" )
                                          )
      ;

   qDebug() << "dest_rebin: " << dest_rebin;

   QStringList rebin_pr_contents;
   for ( int i = 0; i < (int)rebin_r.size(); ++i ) {
      rebin_pr_contents << QString( "%1 %2 %3" )
         .arg( rebin_r[i], 0, 'g', 9 )
         .arg( rebin_pr[i], 0, 'g', 9 )
         .arg( rebin_sd[i], 0, 'g', 9 )
         ;
   }

   QString rebin_pr_out = header + rebin_pr_contents.join( "\n" ) + "\n";
   if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
      dest_rebin = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest_rebin, 0, this );
   }
   {
      QString error;
      if ( !US_File_Util::putcontents( dest_rebin, rebin_pr_out, error ) ) {
         editor_msg( "red", error );
         return false;
      } else {
         created_files << dest_rebin;
      }
   }
   return true;
}


void US_Hydrodyn_Saxs::ift_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   ift_readFromStderr();
   ift_readFromStdout();
      //   }
   disconnect( ift, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( ift, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( ift, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("IFT finished.\n");

   // post process the files

   external_running = false;

   QStringList caps;
   caps
      << "Rg, I\\(0\\) and Dmax"
      << "Parameters"
      << "p_1\\(r\\)"
      << "Data used"
      << "Fit of data"
      ;

   map < QString, QString > files;

   for ( int i = 0; i < (int) caps.size(); ++i ) {
      QRegExp rx( caps[ i ] + " in\\s+:\\s*(\\S+)" );

      if ( rx.indexIn( ift_stdout ) == -1 ) {
         editor_msg( "red", QString( us_tr( "Could not find %1 file in IFT output" ) ).arg( caps[ i ].replace( "\\", "" ) ) );
      } else {
         files[ caps[ i ] ] = rx.cap( 1 );
         us_qdebug( QString( "%1 : '%2'\n" ).arg( caps[ i ] ).arg( files[ caps[ i ] ] ) );
      }
   }

   // show parameters in textarea
      
   if ( files.count( caps[ 1 ] ) ) {
      QFile f( files[ caps[ 1 ] ] );
      if ( !f.open( QIODevice::ReadOnly ) ) {
         editor_msg( "red", QString( "Error: Could not open file %1 for reading." ).arg( files[ caps[ 1 ] ] ) );
      } else {
         QTextStream ts( &f );
#if QT_VERSION < 0x040000
         editor_msg( "black", ts.read() );
#else
         editor_msg( "black", ts.readAll() );
#endif
      }
   }

   QStringList created_files;
   QString rxstr = "\\..*$";
   if ( !ift_last_processed.contains( QRegExp( rxstr ) ) ) {
      rxstr = "$";
   }

   // p(r) file
#define PR_ZERO_SD_MULTIPLIER 1e-3

   if ( files.count( caps[ 2 ] ) ) {
      // copy this to our created files

      double mw = get_mw( QString( "%1" ).arg( ift_last_processed ).replace( QRegExp( "\\..*$" ), "_ift P(r)" ), false, true );
      cb_normalize->setChecked( mw != -1 );

      vector < double > r;
      vector < double > pr;
      vector < double > pre;

      // adjust to create non-zero sds and get/put contents
      QString prcontents;
      QString prcontents_normed;
      QString error;
      bool ok_to_replace = false;
      if ( !US_File_Util::getcontents( files[ caps[ 2 ] ], prcontents, error ) ) {
         editor_msg( "red", error );
      } else {
         QStringList qsl = prcontents.split( "\n" ).replaceInStrings( QRegExp( "^\\s*" ), "" );
            
         // QTextStream( stdout ) << "--- pr contents ---\n";
         double min_sd = 1e99;
         double min_pr = 1e99;
         for ( int i = 0; i < qsl.size(); ++i ) {
            QStringList line = qsl[i].split( QRegExp( "\\s+" ) );
            // QTextStream( stdout ) << line.join( " " ) << Qt::endl;
            if ( line.size() < 3 ) {
               continue;
            } else {
               r  .push_back( line[0].toDouble() );
               pr .push_back( line[1].toDouble() );
               pre.push_back( line[2].toDouble() );
               
               double this_pr = line[1].toDouble();
               if ( this_pr > 0 && min_pr > this_pr ) {
                  min_pr = this_pr;
               }

               double this_sd = line[2].toDouble();
               if ( this_sd > 0 && min_sd > this_sd ) {
                  min_sd = this_sd;
               }
               // QTextStream( stdout ) << QString( "this_pr %1 min_pr %2 this_sd %3 min_sd %4\n" ).arg( this_pr ).arg( min_pr ).arg( this_sd ).arg( min_sd );
            }
         }
         // QTextStream( stdout ) << "--- end pr contents ---\n";
         if ( min_pr == 1e99 && min_sd == 1e99 ) {
            editor_msg( "red", "no data apparent in generated P(r)" );
         } else {
            ok_to_replace = true;
            double zero_sd;
            if ( min_sd != 1e99 ) {
               zero_sd = min_sd * PR_ZERO_SD_MULTIPLIER;
            } else {
               zero_sd = min_pr * PR_ZERO_SD_MULTIPLIER * 1e-2;
            }
            // replace contents
            QStringList newcontents;
            QStringList newcontents_normed;
            for ( int i = 0; i < qsl.size(); ++i ) {
               QStringList line = qsl[i].split( QRegExp( "\\s+" ) );
               if ( line.size() < 3 ) {
                  continue;
               } else {
                  if ( line[2].toDouble() == 0 ) {
                     line[2] = QString( "%1" ).arg( zero_sd );
                     pre [i] = zero_sd;
                  }
                  newcontents << line.join( " " );
               }
            }
            prcontents        = newcontents.join( "\n" ) + "\n";

            if ( mw != -1 ) {
               // US_Vector::printvector3( QString( "ift %1 pr r, pr, pre" ).arg( , r, pr, pre );
               normalize_pr( r, &pr, &pre, mw );
               for ( int i = 0; i < (int)r.size(); ++i ) {
                  newcontents_normed << QString( "%1 %2 %3" ).arg( r[i], 0, 'g', 9 ).arg( pr[i], 0, 'g', 9 ).arg( pre[i], 0, 'g', 9 );
               }
               prcontents_normed = newcontents_normed.join( "\n" ) + "\n";
            }
               
            // QTextStream( stdout ) << "--- new pr contents ---\n";
            // QTextStream( stdout ) << prcontents;
            // QTextStream( stdout ) << "--- end new pr contents ---\n";
         }
      }
         
      QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( ift_last_processed ).replace( QRegExp( rxstr ), "_ift.sprr" );
      if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
         dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
      }

      QString header =
         QString( "# IFT P(r) from " + ift_last_processed + "%1\nR\tP(r)\tSD\n" )
         .arg(
              mw == -1e0
              ? QString( "" )
              : QString( " mw %1 Daltons" ).arg( mw )
              )
         ;

      if ( ok_to_replace ) {
         prcontents = header + prcontents;
         QString error;
         if ( !US_File_Util::putcontents( dest, prcontents, error ) ) {
            editor_msg( "red", error );
         } else {
            created_files << dest;
            if ( prcontents_normed.isEmpty() || !cb_normalize->isChecked() ) {
               cb_normalize->setChecked( false );
               load_pr( false, dest, mw == -1e0 );
               last_pr_rebin_save( header, rxstr, created_files );
            }
         }            
         if ( !prcontents_normed.isEmpty() ) {
            prcontents_normed = header + prcontents_normed;
            QString dest_normed = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( ift_last_processed ).replace( QRegExp( rxstr ), "_ift.dat" );
            if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
               dest_normed = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest_normed, 0, this );
            }
            if ( !US_File_Util::putcontents( dest_normed, prcontents_normed, error ) ) {
               editor_msg( "red", error );
            } else {
               created_files << dest_normed;
               if ( cb_normalize->isChecked() ) {
                  load_pr( false, dest_normed, mw == -1e0 );
               } else {
                  load_pr( false, dest, mw == -1e0 );
               }
               last_pr_rebin_save( header, rxstr, created_files );
            }
         }
      } else {
         US_File_Util ufu;
         if ( !ufu.copy( files[ caps[ 2 ] ], dest, true, header ) ) {
            editor_msg( "red", ufu.errormsg );
         } else {
            created_files << dest;
            load_pr( false, dest, mw == -1e0 );
            last_pr_rebin_save( header, rxstr, created_files );
         }
      }
   }

   // "out" file
   if ( files.count( caps[ 0 ] ) ) {
      // copy this to our created files
      QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( ift_last_processed ).replace( QRegExp( rxstr ), "_ift_summary.txt" );
      if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
         dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
      }
      US_File_Util ufu;
      if ( !ufu.copy( files[ caps[ 0 ] ], dest, true ) ) {
         editor_msg( "red", ufu.errormsg );
      } else {
         created_files << dest;
      }
   }

   // fit file
   if ( files.count( caps[ 4 ] ) ) {
      // copy this to our created files
      QString dest = USglobal->config_list.root_dir + "/somo/saxs/" + QString( "%1" ).arg( ift_last_processed ).replace( QRegExp( rxstr ), "_fit.ssaxs" );
      if ( !((US_Hydrodyn *) us_hydrodyn )->overwrite ) {
         dest = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( dest, 0, this );
      }
      US_File_Util ufu;
      if ( !ufu.copy( files[ caps[ 4 ] ], dest, true, "# IFT I(q) fitting from " + ift_last_processed + "\nq\tI(q)\tSD\n" ) ) {
         editor_msg( "red", ufu.errormsg );
      } else {
         created_files << dest;
         if ( !ift_parameters.count( "rerun" ) || ift_parameters[ "rerun" ] != "true" ) {
            load_saxs( dest, false, ift_last_processed );
         }
      }
   }

   if ( created_files.size() ) {
      editor_msg( "dark blue", QString( "Created files:\n%1\n" ).arg( created_files.join( "\n" ) ) );
   }

   ift_process_next();
}
   
void US_Hydrodyn_Saxs::ift_started()
{
   editor_msg("brown", "IFT launch exited\n");
   disconnect( ift, SIGNAL(started()), 0, 0);
}

// -------------------- FoXS ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_foxs( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
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
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg( (QString) "red", QString("FoXS program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg( "red", QString("FoXS program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("FoXS called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   pb_plot_saxs_sans->setEnabled(false);

   foxs_last_pdb = pdb;

   foxs = new QProcess( this );
   //   foxs->setWorkingDirectory( dir );
#if QT_VERSION < 0x040000
   foxs->addArgument( prog );

   foxs->addArgument( "-q" );
   foxs->addArgument( QString("%1").arg( our_saxs_options->end_q ) );

   foxs->addArgument( "-s" );
   foxs->addArgument( QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );

   foxs->addArgument( pdb );

   cout << 
      QString("foxs -q %1 -s %2 %3\n")
      .arg( our_saxs_options->end_q )
      .arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) )
      .arg( pdb );

   connect( foxs, SIGNAL(readyReadStandardOutput()), this, SLOT(foxs_readFromStdout()) );
   connect( foxs, SIGNAL(readyReadStandardError()), this, SLOT(foxs_readFromStderr()) );
   connect( foxs, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(foxs_finished( int, QProcess::ExitStatus )) );
   connect( foxs, SIGNAL(started()), this, SLOT(foxs_started()) );

   editor_msg( "black", "\nStarting FoXS\n");
   foxs->start();
   external_running = true;
#else
   {
      QStringList args;

      args
         << "-q"
         << QString("%1").arg( our_saxs_options->end_q )
         << "-s"
         << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q))
         << pdb
         ;

      cout << 
         QString("foxs -q %1 -s %2 %3\n")
         .arg( our_saxs_options->end_q )
         .arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) )
         .arg( pdb );

      connect( foxs, SIGNAL(readyReadStandardOutput()), this, SLOT(foxs_readFromStdout()) );
      connect( foxs, SIGNAL(readyReadStandardError()), this, SLOT(foxs_readFromStderr()) );
      connect( foxs, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(foxs_finished( int, QProcess::ExitStatus )) );
      connect( foxs, SIGNAL(started()), this, SLOT(foxs_started()) );

      editor_msg( "black", "\nStarting FoXS\n");
      foxs->start( prog, args, QIODevice::ReadOnly );
      external_running = true;
   }
#endif
   
   return 0;
}

void US_Hydrodyn_Saxs::foxs_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( foxs->canReadLineStdout() )
   {
      editor_msg("brown", foxs->readLineStdout() + "\n");
   }
#else
   editor_msg( "brown", QString( foxs->readAllStandardOutput() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::foxs_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( foxs->canReadLineStderr() )
   {
      editor_msg("red", foxs->readLineStderr() + "\n");
   }
#else
   editor_msg( "red", QString( foxs->readAllStandardError() ) );
#endif   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::foxs_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   foxs_readFromStderr();
   foxs_readFromStdout();
      //   }
   disconnect( foxs, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( foxs, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( foxs, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("FoXS finished.\n");

   // foxs creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = foxs_last_pdb + ".dat";
   QString created_plt = foxs_last_pdb;
   created_plt.replace(QRegExp("\\.(pdb|PDB)$"),".plt");

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(us_tr("Error: FoXS did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   if ( !QFile::exists( created_plt ) )
   {
      editor_msg("dark red", QString(us_tr("Notice: FoXS did not create file %1")).arg( created_dat ));
   } else {
      if ( !QFile::remove( created_plt ) )
      {
         editor_msg("red", QString(us_tr("Notice: remove of FoXS created file %1 failed")).arg( created_dat ));
      }
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + QFileInfo(foxs_last_pdb).fileName() + iqq_suffix() + ".dat";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(us_tr("Notice: overwriting %1")).arg( new_created_dat ));
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(us_tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );
   pb_plot_saxs_sans->setEnabled(true);
   external_running = false;
}
   
void US_Hydrodyn_Saxs::foxs_started()
{
   editor_msg("brown", "FoXS launch exited\n");
   disconnect( foxs, SIGNAL(started()), 0, 0);
}

// -------------------- crysol ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_crysol( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + ( our_saxs_options->crysol_version_3 ? "crysol3" : "crysol" )
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   crysol_stdout.clear( );
   crysol_stderr.clear( );

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Crysol program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Crysol program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Crysol called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   QString dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp";

   QDir qd_dir( dir );

   if ( !qd_dir.exists() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' does not exist\n").arg(qd_dir.path()));
      return -1;
   }

   if ( !qd_dir.isReadable() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir.path()));
      return -1;
   }

   QString uniqstr = QDateTime::currentDateTime().toString( "yyyyMMddhhmmsszzz" );
   QDir qd_dir2( dir + QDir::separator() + uniqstr );
   if ( qd_dir2.exists() )
   {
      unsigned int ext = 1;
      do {
         qd_dir2.setPath( dir + QDir::separator() + uniqstr + QString( "-%1" ).arg( ext++ ) );
      } while ( qd_dir2.exists() );
   }

   if ( !qd_dir2.mkdir( qd_dir2.path() ) )
   {
      editor_msg("red", QString("Crysol called but could not create the temporary directory '%1'. Check permissions\n").arg(qd_dir2.path()));
      return -1;
   }
   if ( !qd_dir2.exists() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' does not exist\n").arg(qd_dir2.path()));
      return -1;
   }

   if ( !qd_dir2.isReadable() )
   {
      editor_msg("red", QString("Crysol called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir2.path()));
      return -1;
   }

   dir = qd_dir2.path();

   crysol_last_pdb = pdb;
   if ( our_saxs_options->crysol_version_26 )
   {
      crysol_last_pdb_base = dir + SLASH + QFileInfo(crysol_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"").left(6) + ".pdb";
   } else {
      crysol_last_pdb_base = dir + SLASH + QFileInfo(crysol_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"") + ".pdb";
   }
      
   QString use_pdb = pdb;
   
   // always copy pdb
   //   if (  our_saxs_options->crysol_version_26 || 
   //        QFileInfo(crysol_last_pdb).fileName() != QFileInfo(crysol_last_pdb_base).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( pdb ));
         return -1;
      }

      QFile f2( crysol_last_pdb_base );
      if ( !f2.open( QIODevice::WriteOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( crysol_last_pdb_base ));
         return -1;
      }

      QString qs;
      QTextStream ts( &f );
      QTextStream ts2( &f2 );

      if ( selected_models[ 0 ] != 0 )
      {
         QRegExp rx_model( "^MODEL\\s+(\\S+)(\\s+|$)" );
         
         bool found_model = false;
         while ( !ts.atEnd() )
         {
            qs = ts.readLine();
            if ( rx_model.indexIn( qs ) != -1 )
            {
               if ( rx_model.cap( 1 ).toUInt() == selected_models[ 0 ] + 1 )
               {
                  found_model = true;
               }
            }
            if ( found_model )
            {
               ts2 << qs << Qt::endl;
               if ( qs.left( 6 ) == "ENDMDL" )
               {
                  break;
               }
            }
         }                  
      } else {
         while ( !ts.atEnd() )
         {
            qs = ts.readLine();
            ts2 << qs << Qt::endl;
            if ( qs.left( 6 ) == "ENDMDL" )
            {
               break;
            }
         }
      }
      f.close();
      f2.close();
      use_pdb = crysol_last_pdb_base;
   }
   //else {
   //      use_pdb = pdb;
   //   }

   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "crysol_last_pdb_base: <" << crysol_last_pdb_base << ">\n";

   editor_msg( "dark blue", QString( "Crysol: use pdb [%1]" ).arg( use_pdb ) );
   editor_msg( "dark blue", QString( "Crysol: last pdb base [%1]" ).arg( crysol_last_pdb_base ) );
   editor_msg( "dark blue", QString( "Crysol: working dir [%1]" ).arg( dir ) );

   // clean up so we have new files

   {
      QString base = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"");
      cout << "base: <" << base << ">\n";

      QString to_remove = base + "00.alm";
      cout << "to_remove: <" << to_remove << ">\n";
      QFile::remove( to_remove );

      to_remove = base + "00.log";
      QFile::remove( to_remove );

      to_remove = base + "00.int";
      QFile::remove( to_remove );

      to_remove = base + "00.fit";
      QFile::remove( to_remove );
   }      

   pb_plot_saxs_sans->setEnabled(false);

#if QT_VERSION >= 0x040000
   QStringList args;
#endif
   
   crysol_manual_mode = false;
   crysol_manual_input.clear( );

   if ( U_EXPT &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
        !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
   {
      if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
           (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 )
      {
         editor_msg( "dark red", QString( us_tr( "Note: Manual average atomic radius %1 (A)" ) )
                     .arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] ) );
         crysol_manual_mode = true;
      }
      if ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
           (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() >  0e0 )
      {
         editor_msg( "dark red", QString( us_tr( "Note: Manual excluded volume %1 (A^3)" ) )
                     .arg( (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] ) );
         crysol_manual_mode = true;
      }
   }

   if ( crysol_manual_mode )
   {
      editor_msg( "dark red", QString( us_tr( "Note: crysol running in interactive mode" ) ) );
#if defined( UHSE_APP_RESPONSE_WAY )
      crysol_app_text .clear( );
      crysol_response .clear( );

      crysol_app_text << "Enter your option ...................... <";
      crysol_response << "0";
      crysol_app_text << "Brookhaven file name ................... <";
      crysol_response << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );
      crysol_app_text << "Maximum order of  harmonics ........... <";
      crysol_response << QString("%1").arg( our_saxs_options->sh_max_harmonics );
      crysol_app_text << "Order of Fibonacci grid ............... <";
      crysol_response << QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order );
      crysol_app_text << "Maximum s value ........................ <";
      crysol_response << QString("%1").arg( our_saxs_options->end_q );
      crysol_app_text << "Number of points ....................... <";
      crysol_response << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) );
      crysol_app_text << "Account for explicit hydrogens? [ Y / N ] <";
      crysol_response << ( our_saxs_options->crysol_explicit_hydrogens ? "Y" : "N" );
      crysol_app_text << "Fit the experimental curve [ Y / N ] .. <";
      crysol_response << "Y";
      crysol_app_text << "Enter data file ........................ <";
      crysol_response << ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ];
      crysol_app_text << "Subtract constant ...................... <";
      crysol_response << "N";
      crysol_app_text << "sin(theta)/lambda [1/nm]  (4) ..... <";
      crysol_response << "1";
      crysol_app_text << "Electron density of the solvent, e/A**3  <";
      crysol_response << QString("%1").arg( our_saxs_options->water_e_density, 0, 'f', 4 );
      crysol_app_text << "Plot the fit [ Y / N ] ................. <";
      crysol_response << "N";
      crysol_app_text << "Another set of parameters [ Y / N ] .... <";
      crysol_response << "Y";
      crysol_app_text << "Minimize again with new limits [ Y / N ] <";
      crysol_response << "N";
      crysol_app_text << "Contrast of the solvation shell ........ <";
      crysol_response << QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast, 0, 'f', 4 ); // hydration shell contrast
      crysol_app_text << "Average atomic radius .................. <";
      crysol_response << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] : ""  )
         ;
      crysol_app_text << "Excluded volume ........................ <";
      crysol_response << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] : "" )
         ;
      crysol_app_text << "Plot the fit [ Y / N ] ................. <";
      crysol_response << "N";
      crysol_app_text << "Another set of parameters [ Y / N ] .... <";
      crysol_response << "N";
      crysol_app_text << "Intensities    saved to file";
      crysol_response << "___run___";
#endif

      crysol_manual_input << ""; // option
      crysol_manual_input << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );
      crysol_manual_input << QString("%1").arg( our_saxs_options->sh_max_harmonics );
      crysol_manual_input << QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order );
      crysol_manual_input << QString("%1").arg( our_saxs_options->end_q );
      crysol_manual_input << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q) );
      crysol_manual_input << ( our_saxs_options->crysol_explicit_hydrogens ? "Y" : "N" );
      crysol_manual_input << "Y"; // fit expt curve
      crysol_manual_input << QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName();
      crysol_manual_input << "N"; // subtract constant
      crysol_manual_input << "1"; // angular units
      crysol_manual_input << QString("%1").arg( our_saxs_options->water_e_density, 0, 'f', 4 ) ;
      crysol_manual_input << "N"; // plot fit
      crysol_manual_input << "Y"; // another set
      crysol_manual_input << "N"; // minimize again
      crysol_manual_input << QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast, 0, 'f', 4 ); // hydration shell contrast
      crysol_manual_input << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_ra" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_ra" ] : ""  )
         ;
      crysol_manual_input << 
         QString( ( (( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "sas_crysol_vol" ) &&
                    (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ].toDouble() > 0e0 ) ?
                  (( US_Hydrodyn * ) us_hydrodyn )->gparams[ "sas_crysol_vol" ] : "" )
         ;
      crysol_manual_input << "N";
      crysol_manual_input << "N";
      // us_qdebug( "crysol input\n------\n" + crysol_manual_input.join( "\n" ) + "\n----" );

      // create input log file, run system command into output 
      {
         {
            QFile f( dir + QDir::separator() + "input" );
            if ( !f.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( us_tr( "Error: trying to create input file %1" ) ).arg( f.fileName() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            QTextStream ts( &f );
            ts << crysol_manual_input.join( "\n" ) << Qt::endl;
            f.close();
         }

         {
            US_File_Util ufu;
            if ( !ufu.copy( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ], dir ) )
            {
               editor_msg( "red", ufu.errormsg );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
         }

         QString cmd = QString( "\"%1\" < input > output" ).arg( prog );
#if defined( WIN32 )
         {
            QFile f( dir + QDir::separator() + "run.bat" );
            if ( !f.open( QIODevice::WriteOnly ) )
            {
               editor_msg( "red", QString( us_tr( "Error: trying to create batch file %1" ) ).arg( f.fileName() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            QTextStream ts( &f );
            ts << cmd << Qt::endl;
            f.close();

            crysol = new QProcess( this );
            crysol->setWorkingDirectory( dir );
#if QT_VERSION < 0x040000
            crysol->addArgument( f.fileName() );
#else
            prog = f.fileName();
#endif
         }

// attempt to create job with no "cmd" box
//          {
//             PROCESS_INFORMATION processInformation = {0};
//             STARTUPINFO startupInfo                = {0};
//             startupInfo.cb                         = sizeof(startupInfo);
 
//             wchar_t wtext[ 2048 ];
//             mbstowcs( wtext, cmd.toLatin1().data(), strlen( cmd.toLatin1().data() ) + 1 ); //Plus null

//             // Create the process
//             BOOL result = CreateProcess(NULL, wtext,
//                                         NULL, NULL, false, 
//                                         NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 
//                                         NULL, NULL, &startupInfo, &processInformation);


//             us_qdebug( "result: " + result );

//             // STARTUPINFOW si;
//             // PROCESS_INFORMATION pi;

//             // ZeroMemory(&si, sizeof(si));
//             // si.cb = sizeof(si);
//             // ZeroMemory(&pi, sizeof(pi));

//             // wchar_t wtext[ 1024 ];
//             // mbstowcs(wtext, cmd.toLatin1().data(), strlen(cmd.toLatin1().data())+1);//Plus null

//             // if (CreateProcessW( NULL, wtext, NULL, NULL, false, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
//             // {
//             //    WaitForSingleObject(pi.hProcess, INFINITE);
//             //    CloseHandle(pi.hProcess);
//             //    CloseHandle(pi.hThread);
//             // }
//          }

#else
         editor_msg( "blue", "\nStarting Crysol\n" + cmd );
         qApp->processEvents();
         QString savedir = QDir::currentPath();
         QDir::setCurrent( dir );
         if ( system( qPrintable( cmd ) ) ) {};
         QDir::setCurrent( savedir );
         {
            QFile f( dir + QDir::separator() + "output" );
            if ( !f.open( QIODevice::ReadOnly ) )
            {
               editor_msg( "red", QString( us_tr( "Error: trying to read output file %1" ) ).arg( f.fileName() ) );
               pb_plot_saxs_sans->setEnabled(true);
               return -1;
            }
            QTextStream ts( &f );
            while ( !ts.atEnd() )
            {
               crysol_stdout << ts.readLine();
            }
            f.close();
            editor_msg( "brown", crysol_stdout.join( "\n" ) );
         }
         crysol_finishup();
         return 0;
#endif
      }
   } else {
      crysol = new QProcess( this );
      crysol->setWorkingDirectory( dir );
      {
         if ( our_saxs_options->crysol_version_3 ) {
            if ( our_saxs_options->crysol_water_dummy_beads ) {
               args
                  << "--shell=water"
                  ;
            }
            args
               << QString("--smax=%1").arg( our_saxs_options->end_q )
               << QString("--ns=%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q))
               << QString("--dns=%1").arg( our_saxs_options->water_e_density )
               << QString("--dro=%1").arg( our_saxs_options->crysol_hydration_shell_contrast )
               << QString("--lm=%1").arg( our_saxs_options->sh_max_harmonics )
               << QString("--fb=%1").arg( our_saxs_options->sh_fibonacci_grid_order )
               ;

            if ( our_saxs_options->crysol_explicit_hydrogens ) {
               args << "--explicit-hydrogens";
            }
            args << QFileInfo( use_pdb ).fileName();
         } else {
            args
               << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb )

               << "/sm"
               <<  QString("%1").arg( our_saxs_options->end_q )
         
               << "/ns"
               << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q))

               << "/dns"
               << QString("%1").arg( our_saxs_options->water_e_density )

               << "/dro"
               << QString("%1").arg( our_saxs_options->crysol_hydration_shell_contrast )

               << "/lm"
               << QString("%1").arg( our_saxs_options->sh_max_harmonics )
      
               << "/fb"
               << QString("%1").arg( our_saxs_options->sh_fibonacci_grid_order )
               ;

            if ( our_saxs_options->crysol_explicit_hydrogens ) {
               args << "/eh";
            }
         }

         if ( U_EXPT &&
              ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
              !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
         {
            US_File_Util ufu;
            if ( !ufu.copy( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ], dir ) )
            {
               editor_msg( "red", ufu.errormsg );
               if ( ufu.errormsg.contains( QRegExp( "exists$" ) ) )
               {
                  args 
                     << QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName()
                     ;
               }
            } else {
               args
                  << QFileInfo( ( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ] ).fileName()
                  ;
            }
         }
      }
   }
   connect( crysol, SIGNAL(readyReadStandardOutput()), this, SLOT(crysol_readFromStdout()) );
   connect( crysol, SIGNAL(readyReadStandardError()), this, SLOT(crysol_readFromStderr()) );
   connect( crysol, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(crysol_finished( int, QProcess::ExitStatus )) );

#if defined( UHSE_APP_RESPONSE_WAY )
   if ( crysol_manual_mode )
   {
      connect( &crysol_timer,  SIGNAL( timeout()         ), this, SLOT( crysol_timeout()        ) );
      crysol_query_response_pos = 0;
      crysol_run_to_end         = false;
      crysol_timer_delay_ms     = 240000;
   }
#endif
   
   editor->append("\n\nStarting Crysol\n");
   
#if QT_VERSION < 0x040000
   editor_msg( "dark blue", crysol->arguments().join( " " ) );
   crysol->start();
#else
   editor_msg( "dark blue", args.join( " " ) );
   crysol->start( prog, args, QIODevice::ReadOnly );
#endif
   external_running = true;
   return 0;
}

void US_Hydrodyn_Saxs::crysol_timeout()
{
   editor_msg( "red", us_tr( "Error: out of responses to queries (timeout)\n" ) );
   // us_qdebug( "timeout" );
   crysol->kill();
}

void US_Hydrodyn_Saxs::crysol_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( crysol->canReadLineStdout() )
   {
      QString qs = crysol->readLineStdout();
      crysol_stdout << qs;
      editor_msg("brown", qs );
   }
#else
   QString qs( crysol->readAllStandardOutput() );
   crysol_stdout << qs;
   editor_msg("brown", qs );
#endif

#if defined( UHSE_APP_RESPONSE_WAY )
   if ( !crysol_manual_mode )
   {
      while ( crysol->canReadLineStdout() )
      {
         QString qs = crysol->readLineStdout();
         crysol_stdout << qs;
         editor_msg("brown", qs );
      }
      return;
   }
   crysol_timer.stop();

   // us_qdebug( "readFromStdout()" );
   QString qs;
   QString text;
   //   do {
   // while ( crysol->canReadLineStdout() )
   // {
   //    qs = crysol->readLineStdout();
   //    crysol_stdout << qs;
   //    editor_msg("brown", qs );
   //    text += qs;
   // };
      //   } while ( qs != QString() );

   //do {
      QString read = crysol->readStdout();
      qs = QString( "%1" ).arg( read );
      crysol_stdout << qs;
      editor_msg("brown", qs );
      text += qs;
      // } while ( qs.length() );
   
      // us_qdebug( QString( "received <%1>" ).arg( text ) );

   if ( !crysol_run_to_end && crysol_app_text.size() )
   {
      // if not at first entry, read data to find match
      int previous_pos = crysol_query_response_pos;
      while ( ( int ) crysol_app_text.size() > crysol_query_response_pos &&
              !text.contains( crysol_app_text[ crysol_query_response_pos ] ) && 
              crysol_query_response_pos )
      {
         crysol_query_response_pos++;
      }
      if ( crysol_query_response_pos >= ( int ) crysol_app_text.size() )
      {
         crysol_query_response_pos = previous_pos;
         if ( crysol_timer_delay_ms )
         {
            // us_qdebug( QString( "starting timer for %1 seconds" ).arg( ( double )crysol_timer_delay_ms / 1000e0 ) );
            crysol_timer.start( crysol_timer_delay_ms );
         } else {
            // us_qdebug( us_tr( "Error: out of responses to queries" ) );
            crysol->kill();
         }
         return;
      }         

      // do we have a match?
      if ( ( int ) crysol_app_text.size() > crysol_query_response_pos &&
           text.contains( crysol_app_text[ crysol_query_response_pos ] ) )
      {
         // us_qdebug( QString( "received <%1> from application" ).arg( crysol_app_text[ crysol_query_response_pos ] ) );
         if ( crysol_response[ crysol_query_response_pos ] != "___run___" )
         {
            if ( crysol_response[ crysol_query_response_pos ].left( 2 ).contains( "__" ) )
            {
               // us_qdebug(  
               //        QString( us_tr( "Error: undefined response <%1> to query <%2>" ) )
               //        .arg( crysol_response[ crysol_query_response_pos ] )
               //        .arg( crysol_app_text[ crysol_query_response_pos ] ) )
               //    ;
               crysol->kill();
               return;
            }
            // us_qdebug( QString( "sent     <%1> to application"   ).arg( crysol_response[ crysol_query_response_pos ] ) );
            crysol->writeToStdin( crysol_response[ crysol_query_response_pos ] + "\n" );
            crysol_query_response_pos++;
         } else {
            // us_qdebug( "now run to end of application" );
            crysol_run_to_end = true;
         }
      }
   }
#endif
}
   
void US_Hydrodyn_Saxs::crysol_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( crysol->canReadLineStderr() )
   {
      QString qs = crysol->readLineStderr();
      crysol_stderr << qs;
      editor_msg("red", qs + "\n");
   }
#else
   QString qs( crysol->readAllStandardError() );
   crysol_stderr << qs;
   editor_msg("red", qs );
#endif
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::crysol_finished( int, QProcess::ExitStatus )
{

   //   for ( int i = 0; i < 10000; i++ )
   //   {
   crysol_readFromStderr();
   crysol_readFromStdout();
      //   }
   disconnect( crysol, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( crysol, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( crysol, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);

   if ( crysol_manual_mode )
   {
#if QT_VERSION < 0x040000
      QFile f( crysol->workingDirectory().dirName() + QDir::separator() + "output" );
#else
      QFile f( crysol->workingDirectory() + QDir::separator() + "output" );
#endif
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg( "red", QString( us_tr( "Error: trying to read output file %1" ) ).arg( f.fileName() ) );
         pb_plot_saxs_sans->setEnabled(true);
         return;
      }
      QTextStream ts( &f );
      while ( !ts.atEnd() )
      {
         crysol_stdout << ts.readLine();
      }
      f.close();
      editor_msg( "brown", crysol_stdout.join( "\n" ) );
   }

   // us_qdebug( "crysolstdout: " + crysol_stdout.join("\n") );
   // us_qdebug( "crysolstderr: " + crysol_stderr.join("\n") );

   // crysol creates 4 files:
   // crysol_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   // crysol
   delete crysol;
   crysol_finishup();
}

void US_Hydrodyn_Saxs::crysol_finishup()
{
   QStringList intensity_lines = crysol_stdout.filter( QRegExp( "Intensities\\s+saved to file" ) );
   QStringList fit_lines       = crysol_stdout.filter( QRegExp( "Data fit\\s+saved to file" ) );

   QString new_intensity_file;
   QString new_fit_file;

   if ( intensity_lines.size() == 1 )
   {
      QRegExp rx( "Intensities\\s+saved to file (\\S+)" );
      if ( rx.indexIn( intensity_lines[ 0 ] ) != -1 )
      {
         new_intensity_file = rx.cap( 1 );
      }
   }
   if ( fit_lines.size() == 1 )
   {
      QRegExp rx( "Data fit\\s+saved to file (\\S+)" );
      if ( rx.indexIn( fit_lines[ 0 ] ) != -1 )
      {
         new_fit_file = rx.cap( 1 );
      }
   }

   // us_qdebug( "intensity_file: " + new_intensity_file );
   // us_qdebug( "fit_file: " + new_fit_file );

   QString type = ".int";
   if ( U_EXPT &&
        ( ( US_Hydrodyn * ) us_hydrodyn )->gparams.count( "saxs_crysol_target" ) &&
        !( ( US_Hydrodyn * ) us_hydrodyn )->gparams[ "saxs_crysol_target" ].isEmpty() )
   {
      type = ".fit";
   }

   QString created_dat;
   if ( our_saxs_options->crysol_version_3 ) {
      created_dat = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") + type;
   } else {
      created_dat = crysol_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") + "00" + type;
   }

   // us_qdebug( "created_dat: " + created_dat );

   if ( !new_intensity_file.isEmpty() )
   {
      created_dat = QFileInfo( created_dat ).path() + QDir::separator() + new_intensity_file;
   }
   if ( !new_fit_file.isEmpty() )
   {
      created_dat = QFileInfo( created_dat ).path() + QDir::separator() + new_fit_file;
   }

   // us_qdebug( "created_dat after rplc: " + created_dat );

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(us_tr("Error: Crysol did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      QFileInfo( crysol_last_pdb.replace(QRegExp("\\.(pdb|PDB)$"),"") ).fileName() + 
      ( selected_models[ 0 ] == 0 ? QString( "" ) : QString( "_m%1" ).arg( selected_models[ 0 ] + 1 ) )
      + iqq_suffix() + type;

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(us_tr("Notice: overwriting %1")).arg( new_created_dat ));
         // windows requires removing previous file
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(us_tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );

   pb_plot_saxs_sans->setEnabled( true );
   editor->append("Crysol finished.\n");
   external_running = false;
}
   
void US_Hydrodyn_Saxs::crysol_wroteToStdin()
{
   // us_qdebug( "Crysol wroteToStdin" );
}

void US_Hydrodyn_Saxs::crysol_started()
{
   editor_msg("brown", "Crysol launch exited\n");
   disconnect( crysol, SIGNAL(started()), 0, 0);
   // us_qdebug( "crysol started" );
}

// -------------------- cryson ------------------------------

int US_Hydrodyn_Saxs::run_sans_iq_cryson( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "/bin/"
#endif
      + SLASH
      + "cryson" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Cryson program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Cryson program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Cryson called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   QString dir = ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "tmp";

   QDir qd_dir( dir );

   if ( !qd_dir.exists() )
   {
      editor_msg("red", QString("Cryson called but the directory '%1' does not exist\n").arg(qd_dir.path()));
      return -1;
   }

   if ( !qd_dir.isReadable() )
   {
      editor_msg("red", QString("Cryson called but the directory '%1' is not readable. Check permissions\n").arg(qd_dir.path()));
      return -1;
   }

   cryson_last_pdb = pdb;
   cryson_last_pdb_base = dir + SLASH + QFileInfo(cryson_last_pdb).fileName().replace(QRegExp("\\.(pdb|PDB)$"),"").left(6) + ".pdb";
   QString use_pdb = pdb;
   
   // copy pdb if the name is too long
   if ( our_saxs_options->crysol_version_26 &&
        QFileInfo(cryson_last_pdb).fileName() != QFileInfo(cryson_last_pdb_base).fileName() )
   {
      QFile f( pdb );
      if ( !f.open( QIODevice::ReadOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( pdb ));
         return -1;
      }

      QFile f2( cryson_last_pdb_base );
      if ( !f2.open( QIODevice::WriteOnly ) )
      {
         editor_msg("red", QString("Could not open file %1. Check permissions\n").arg( cryson_last_pdb_base ));
         return -1;
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
      use_pdb = cryson_last_pdb_base;
   } else {
      use_pdb = pdb;
   }

   cout << "use_pdb: <" << use_pdb << ">\n";
   cout << "cryson_last_pdb_base: <" << cryson_last_pdb_base << ">\n";

   // clean up so we have new files

   // {
   //    QString base = cryson_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"");
   //    cout << "base: <" << base << ">\n";

   //    QString to_remove = base + "00.alm";
   //    cout << "to_remove: <" << to_remove << ">\n";
   //    QFile::remove( to_remove );

   //    to_remove = base + "00.log";
   //    QFile::remove( to_remove );

   //    to_remove = base + "00.int";
   //    QFile::remove( to_remove );
   // }      

   pb_plot_saxs_sans->setEnabled(false);

   cryson = new QProcess( this );
   cryson->setWorkingDirectory( dir );
#if QT_VERSION < 0x040000
   cryson->addArgument( prog );
   cryson->addArgument( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb );

   cryson->addArgument( "/sm" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->end_q ) );

   cryson->addArgument( "/ns" );
   cryson->addArgument( QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );

   cryson->addArgument( "/D2O" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->d2o_conc ) );

   if ( our_saxs_options->cryson_manual_hs )
   {
      cryson->addArgument( "/dro" );
      cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_hydration_shell_contrast ) );
   }

   if ( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "perdeuteration" ) )
   {
      cryson->addArgument( "/per" );
      cryson->addArgument( ((US_Hydrodyn *)us_hydrodyn)->gparams[ "perdeuteration" ] );
   }

   cryson->addArgument( "/lm" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_sh_max_harmonics ) );

   cryson->addArgument( "/fb" );
   cryson->addArgument( QString("%1").arg( our_saxs_options->cryson_sh_fibonacci_grid_order ) );
#else
   QStringList args;
   
   args
      << ( our_saxs_options->crysol_version_26 ? QFileInfo(use_pdb).fileName() : use_pdb )

      << "/sm"
      << QString("%1").arg( our_saxs_options->end_q )

      << "/ns"
      << QString("%1").arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q))

      << "/D2O"
      << QString("%1").arg( our_saxs_options->d2o_conc )
      ;
   if ( our_saxs_options->cryson_manual_hs )
   {
      args
         << "/dro"
         << QString("%1").arg( our_saxs_options->cryson_hydration_shell_contrast )
         ;
   }

   if ( ((US_Hydrodyn *)us_hydrodyn)->gparams.count( "perdeuteration" ) )
   {
      args
         << "/per"
         << ((US_Hydrodyn *)us_hydrodyn)->gparams[ "perdeuteration" ]
         ;
   }

   args
      << "/lm"
      << QString("%1").arg( our_saxs_options->cryson_sh_max_harmonics )

      << "/fb"
      << QString("%1").arg( our_saxs_options->cryson_sh_fibonacci_grid_order )
      ;
#endif
   
   //    if ( our_saxs_options->cryson_explicit_hydrogens )
   //    {
   //       cryson->addArgument( "/eh" );
   //    }

   connect( cryson, SIGNAL(readyReadStandardOutput()), this, SLOT(cryson_readFromStdout()) );
   connect( cryson, SIGNAL(readyReadStandardError()), this, SLOT(cryson_readFromStderr()) );
   connect( cryson, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(cryson_finished( int, QProcess::ExitStatus )) );
   connect( cryson, SIGNAL(started()), this, SLOT(cryson_started()) );

   editor->append("\n\nStarting Cryson\n");
#if QT_VERSION < 0x040000
   cryson->start();
#else
   cryson->start( prog, args, QIODevice::ReadOnly );
#endif
   external_running = true;

   return 0;
}

void US_Hydrodyn_Saxs::cryson_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( cryson->canReadLineStdout() )
   {
      editor_msg("brown", cryson->readLineStdout() + "\n");
   }
#else
   editor_msg("brown", QString( cryson->readAllStandardOutput() ) );
#endif
   
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::cryson_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( cryson->canReadLineStderr() )
   {
      editor_msg("red", cryson->readLineStderr() + "\n");
   }
#else
   editor_msg("red", QString( cryson->readAllStandardError() ) );
#endif
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::cryson_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   cryson_readFromStderr();
   cryson_readFromStdout();
      //   }
   disconnect( cryson, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( cryson, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( cryson, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);

   // cryson creates 4 files:
   // cryson_summary.txt
   // pdb##.alm
   // pdb##.log
   // pdb##.int

   // we just want the .int, the rest will be removed if needed

   QString created_dat = cryson_last_pdb_base.replace(QRegExp("\\.(pdb|PDB)$"),"") +  "00.int";

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(us_tr("Error: Cryson did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + 
      QFileInfo( cryson_last_pdb.replace(QRegExp("\\.(pdb|PDB)$"),"") ).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(us_tr("Notice: overwriting %1")).arg( new_created_dat ));
         // windows requires removing previous file
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(us_tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );

   pb_plot_saxs_sans->setEnabled( true );
   editor->append("Cryson finished.\n");
   external_running = false;
}
   
void US_Hydrodyn_Saxs::cryson_started()
{
   editor_msg("brown", "Cryson launch exited\n");
   disconnect( cryson, SIGNAL(started()), 0, 0);
}

// -------------------- Sastbx ------------------------------

int US_Hydrodyn_Saxs::run_saxs_iq_sastbx( QString pdb )
{
   QString prog = 
      USglobal->config_list.system_dir + SLASH +
#if defined(BIN64)
      "bin64"
#else
      "bin"
#endif
      + SLASH
      + "sastbx.she" 
#if defined(WIN32)
      + ".exe"
#endif      
      ;

   {
      QFileInfo qfi(prog);
      if ( !qfi.exists() )
      {
         editor_msg("red", QString("Sastbx program '%1' does not exist\n").arg(prog));
         return -1;
      }
      if ( !qfi.isExecutable() )
      {
         editor_msg("red", QString("Sastbx program '%1' is not executable\n").arg(prog));
         return -1;
      }
   }

   QFileInfo fi( pdb );
   if ( !fi.exists() )
   {
      editor_msg("red", QString("Sastbx called but PDB file '%1' does not exist\n").arg(pdb));
      return -1;
   }

   pb_plot_saxs_sans->setEnabled(false);

   sastbx_last_pdb = pdb;

   sastbx = new QProcess( this );
   //   sastbx->setWorkingDirectory( dir );
#if QT_VERSION < 0x040000
   sastbx->addArgument( prog );

   sastbx->addArgument( QString( "structure=%1" ).arg( pdb ) );
#else
   QStringList args;
   args
      << QString( "structure=%1" ).arg( pdb )
      ;
#endif
   
   QString method;
   switch ( our_saxs_options->sastbx_method )
   {
   case 1:
      method = "debye";
      break;
   case 2:
      method = "zernike";
      break;
   case 0:
   default:
      method = "she";
      break;
   }

#if QT_VERSION < 0x040000
   sastbx->addArgument( QString( "method=%1"    ).arg( method ) );
   sastbx->addArgument( QString( "q_start=%1"   ).arg( our_saxs_options->start_q ) );
   sastbx->addArgument( QString( "q_stop=%1"    ).arg( our_saxs_options->end_q ) );
   sastbx->addArgument( QString( "n_step=%1"    ).arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q)) );
   sastbx->addArgument( QString( "rho=%1"       ).arg( our_saxs_options->water_e_density ) );
   sastbx->addArgument( QString( "drho=%1"      ).arg( our_saxs_options->crysol_hydration_shell_contrast ) );
   sastbx->addArgument( QString( "max_i =%1"    ).arg( our_saxs_options->sh_fibonacci_grid_order ) );
   sastbx->addArgument( QString( "max_L =%1"    ).arg( our_saxs_options->sh_max_harmonics ) );
   sastbx->addArgument( QString( "output=%1"    ).arg( sastbx_last_pdb + ".int" ) );
#else
   args
      << QString( "method=%1"    ).arg( method )
      << QString( "q_start=%1"   ).arg( our_saxs_options->start_q )
      << QString( "q_stop=%1"    ).arg( our_saxs_options->end_q )
      << QString( "n_step=%1"    ).arg( (unsigned int)(our_saxs_options->end_q / our_saxs_options->delta_q))
      << QString( "rho=%1"       ).arg( our_saxs_options->water_e_density )
      << QString( "drho=%1"      ).arg( our_saxs_options->crysol_hydration_shell_contrast )
      << QString( "max_i =%1"    ).arg( our_saxs_options->sh_fibonacci_grid_order )
      << QString( "max_L =%1"    ).arg( our_saxs_options->sh_max_harmonics )
      << QString( "output=%1"    ).arg( sastbx_last_pdb + ".int" )
      ;
#endif
   connect( sastbx, SIGNAL(readyReadStandardOutput()), this, SLOT(sastbx_readFromStdout()) );
   connect( sastbx, SIGNAL(readyReadStandardError()), this, SLOT(sastbx_readFromStderr()) );
   connect( sastbx, SIGNAL(finished( int, QProcess::ExitStatus )), this, SLOT(sastbx_finished( int, QProcess::ExitStatus )) );
   connect( sastbx, SIGNAL(started()), this, SLOT(sastbx_started()) );

   editor->append("\n\nStarting Sastbx\n");
   

#if QT_VERSION < 0x040000
   sastbx->start();
#else
   sastbx->start( prog, args, QIODevice::ReadOnly );
#endif
   external_running = true;

   return 0;
}

void US_Hydrodyn_Saxs::sastbx_readFromStdout()
{
#if QT_VERSION < 0x040000
   while ( sastbx->canReadLineStdout() )
   {
      editor_msg("brown", sastbx->readLineStdout() + "\n");
   }
#else
   editor_msg("brown", QString( sastbx->readAllStandardOutput() ) );
#endif
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::sastbx_readFromStderr()
{
#if QT_VERSION < 0x040000
   while ( sastbx->canReadLineStderr() )
   {
      editor_msg("red", sastbx->readLineStderr() + "\n");
   }
#else
   editor_msg("red", QString( sastbx->readAllStandardError() ) );
#endif
   //  qApp->processEvents();
}
   
void US_Hydrodyn_Saxs::sastbx_finished( int, QProcess::ExitStatus )
{
   //   for ( int i = 0; i < 10000; i++ )
   //   {
   sastbx_readFromStderr();
   sastbx_readFromStdout();
      //   }
   disconnect( sastbx, SIGNAL(readyReadStandardOutput()), 0, 0);
   disconnect( sastbx, SIGNAL(readyReadStandardError()), 0, 0);
   disconnect( sastbx, SIGNAL(finished( int, QProcess::ExitStatus )), 0, 0);
   editor->append("Sastbx finished.\n");

   // sastbx creates 2 files:
   // pdb_pdb.dat
   // pdb.dat

   QString created_dat = sastbx_last_pdb + ".int";

   if ( !QFile::exists( created_dat ) )
   {
      editor_msg("red", QString(us_tr("Error: Sastbx did not create file %1")).arg( created_dat ));
      pb_plot_saxs_sans->setEnabled(true);
      external_running = false;
      return;
   }

   // now move the file to the saxs directory
   QString new_created_dat = 
      ((US_Hydrodyn *)us_hydrodyn)->somo_dir + SLASH + "saxs" + SLASH + QFileInfo(sastbx_last_pdb).fileName() + iqq_suffix() + ".int";

   if ( QFile::exists(new_created_dat) )
   {
      if ( ((US_Hydrodyn *)us_hydrodyn)->overwrite )
      {
         editor_msg("red", QString(us_tr("Notice: overwriting %1")).arg( new_created_dat ));
      } else {
         new_created_dat = ((US_Hydrodyn *)us_hydrodyn)->fileNameCheck( new_created_dat, 0, this );
      }
      QFile::remove( new_created_dat );
   }

   QDir qd;
   if ( !qd.rename( created_dat, new_created_dat ) )
   {
      editor_msg("red", 
                 QString(us_tr("Notice: could not rename %1 to %2"))
                 .arg( created_dat )
                 .arg( new_created_dat )
                 );
      new_created_dat = created_dat;
   }

   // now load & plot this curve
   load_saxs( new_created_dat );
   pb_plot_saxs_sans->setEnabled(true);
   external_running = false;
}
   
void US_Hydrodyn_Saxs::sastbx_started()
{
   editor_msg("brown", "Sastbx launch exited\n");
   disconnect( sastbx, SIGNAL(started()), 0, 0);
}
