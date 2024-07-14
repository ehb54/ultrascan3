#include "../include/us_util.h"
#include "../include/us_write_config.h"
#include "../include/us_version.h"
#if QT_VERSION >= 0x040000
#include <QtCore>
#endif
#include <qregexp.h>
//Added by qt3to4:
#include <QTextStream>

// note: this program uses cout and/or cerr and this should be replaced

static std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) { 
   return os << qPrintable(str);
}

US_Config::US_Config( QObject* parent )
   : QObject ( parent )
{
#if QT_VERSION >= 0x040000
   const QString RevSvn( SOMO_Revision );
#endif
   US_Version = US_Version_string; // Defined in us_util.h
   
   if ( ! read() )
   {
      // This should never happen.  Created in us main program.
      setDefault();
      US_Write_Config *w_config;
      w_config = new US_Write_Config(this);
      if ( ! w_config->write_config( config_list ) )
      {
         delete w_config;
         exit( 1 );
      }
   }

   col_exists();
   setModelString();
}


// This is for us_register only
US_Config::US_Config( QString /* dummy */, QObject* parent )
   : QObject ( parent )
{
   //  config_list.tar = dummy;  // Dummy to avoid compiler complaint
}


US_Config::~US_Config()
{
}

bool US_Config::write_default_colors()
{
   QString  colfile = US_Config::get_home_dir() + USCOLORS;

   color_defaults();
   QFile f( colfile );
   if ( f.open( QIODevice::WriteOnly ) )
   {
      QDataStream ds( &f );

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
      return( true );
   }
   else
   {
      return( false );
   }
}

void US_Config::setModelString()
{
   vector <QString>& m = modelString; // convenience

   m.clear( );
   m.push_back( us_tr( "1-Component, Ideal" ) );                        // model 0
   m.push_back( us_tr( "2-Component, Ideal, Noninteracting" ) );        // model 1
   m.push_back( us_tr( "3-Component, Ideal, Noninteracting" ) );        // model 2
   m.push_back( us_tr( "Fixed Molecular Weight Distribution" ) );       // model 3
   m.push_back( us_tr( "Monomer-Dimer Equilibrium" ) );                 // model 4
   m.push_back( us_tr( "Monomer-Trimer Equilibrium" ) );                // model 5
   m.push_back( us_tr( "Monomer-Tetramer Equilibrium" ) );              // model 6
   m.push_back( us_tr( "Monomer-Pentamer Equilibrium" ) );              // model 7
   m.push_back( us_tr( "Monomer-Hexamer Equilibrium" ) );               // model 8
   m.push_back( us_tr( "Monomer-Heptamer Equilibrium" ) );              // model 9
   m.push_back( us_tr( "User-Defined Monomer-Nmer Equilibrium" ) );     // model 10
   m.push_back( us_tr( "Monomer-Dimer-Trimer Equilibrium" ) );          // model 11
   m.push_back( us_tr( "Monomer-Dimer-Tetramer Equilibrium" ) );        // model 12
   m.push_back( us_tr( "User-Defined Monomer - N-mer - M-mer Equilibrium" ) );
   // model 13
   m.push_back( us_tr( "2-Component Hetero-Association: A + B <=> AB" ) );
   // model 14
   m.push_back( us_tr( "User-defined self/Hetero-Association: A + B <=> AB, nA <=> An" ) );
   // model 15
   m.push_back( us_tr( "User-Defined Monomer-Nmer, some monomer is incompetent" ) );
   // model 16
   m.push_back( us_tr( "User-Defined Monomer-Nmer, some Nmer is incompetent" ) );
   // model 17
   m.push_back( us_tr( "User-Defined irreversible Monomer-Nmer" ) );    // model 18
   m.push_back( us_tr( "User-Defined Monomer-Nmer plus contaminant" ) );// model 19
}

bool US_Config::col_exists()
{
   QString version = "0.0";
   bool    flag    = false;

   QString  colfile = US_Config::get_home_dir() + USCOLORS;

   QFile f( colfile );

   if ( f.open( QIODevice::ReadOnly ) )
   {
      QDataStream ds( &f );

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

      if ( ! f.atEnd() )
      {
         ds >> version;
         if ( version.toFloat() > 7.0 )
         {
            flag = true;
         }
      }
   }

   if ( ! flag )
   {
      if ( write_default_colors() )
      {
         flag = true;
      }
   }

   return ( flag );

}

void US_Config::color_defaults()
{
   // Default Colors for UltraScan
   QColor temp_col;

   temp_col.setRgb                  ( 0x00, 0x00, 0x80 );  // dark blue
   global_colors.pushb.setRgb       ( 0x00, 0xe0, 0xe0 );  // light green-blue
   global_colors.pushb_active.setRgb( 0x00, 0xa0, 0xa0 );  // med   green-blue
   global_colors.frames.setRgb      ( 0x00, 0x77, 0x77 );  // light green-blue
   global_colors.labels.setRgb      ( 0x00, 0x00, 0x00 );  // black
   global_colors.dk_green.setRgb    ( 0x00, 0x44, 0x00 );  // dark green
   global_colors.dk_red.setRgb      ( 0x55, 0x00, 0x00 );  // dark red
   global_colors.gray.setRgb        ( 0x88, 0x88, 0x88 );  // gray
   global_colors.gray1.setRgb       ( 0xef, 0xef, 0xef );  // gray1
   global_colors.gray2.setRgb       ( 0xde, 0xde, 0xde );  // gray2
   global_colors.gray3.setRgb       ( 0xc6, 0xc6, 0xc6 );  // gray3
   global_colors.gray4.setRgb       ( 0x84, 0x84, 0x84 );  // gray4
   global_colors.gray5.setRgb       ( 0xaa, 0xaa, 0xaa );  // gray5
   global_colors.gray6.setRgb       ( 0xbb, 0xbb, 0xbb );  // gray6
   global_colors.gray7.setRgb       ( 0xcc, 0xcc, 0xcc );  // gray7
   global_colors.gray8.setRgb       ( 0xdd, 0xdd, 0xdd );  // gray8
   global_colors.gray9.setRgb       ( 0xee, 0xee, 0xee );  // gray9
   global_colors.plot.setRgb        ( 0x00, 0x00, 0x00 );  // black
   global_colors.major_ticks.setRgb ( 0xff, 0xff, 0xff );  // white
   global_colors.minor_ticks.setRgb ( 0xaa, 0xaa, 0xaa );  // gray5

   QPalette& cg_frame = global_colors.cg_frame;  // Convenience

   cg_frame.setColor( QPalette::WindowText, Qt::white );
   cg_frame.setColor( QPalette::Window,     global_colors.frames );
   cg_frame.setColor( QPalette::Light,      global_colors.gray2 );
   cg_frame.setColor( QPalette::Dark,       global_colors.gray4 );
   cg_frame.setColor( QPalette::Mid,        Qt::cyan );
   cg_frame.setColor( QPalette::Text,       Qt::white );
   cg_frame.setColor( QPalette::Base,       global_colors.gray4 );

   QPalette& cg_pushb = global_colors.cg_pushb;  // Convenience

   cg_pushb.setColor( QPalette::ButtonText, Qt::black );
   cg_pushb.setColor( QPalette::Button,     global_colors.pushb );
   cg_pushb.setColor( QPalette::Light,      Qt::white );
   cg_pushb.setColor( QPalette::Dark,       global_colors.gray4 );
   cg_pushb.setColor( QPalette::Shadow,     Qt::black );
   // For splash screen window
   cg_pushb.setColor( QPalette::Window,     global_colors.pushb );

   QPalette& cg_pushb_disabled = global_colors.cg_pushb_disabled;  // Convenience

   cg_pushb_disabled.setColor( QPalette::ButtonText, Qt::white );
   cg_pushb_disabled.setColor( QPalette::Button,     global_colors.pushb );
   cg_pushb_disabled.setColor( QPalette::Light,      Qt::white );
   cg_pushb_disabled.setColor( QPalette::Dark,       global_colors.gray4 );
   cg_pushb_disabled.setColor( QPalette::Shadow,     Qt::black );
   // For splash screen shadow
   cg_pushb_disabled.setColor( QPalette::Window,     global_colors.pushb );
   cg_pushb_disabled.setColor( QPalette::Text,       global_colors.gray3 );

   QPalette& cg_pushb_active = global_colors.cg_pushb_active;  // Convenience

   cg_pushb_active.setColor( QPalette::ButtonText, Qt::black );
   cg_pushb_active.setColor( QPalette::Button,     global_colors.pushb );
   cg_pushb_active.setColor( QPalette::Light,      Qt::white );
   cg_pushb_active.setColor( QPalette::Dark,       global_colors.gray4 );
   cg_pushb_active.setColor( QPalette::Shadow,     Qt::black );
   // For splash screen shadow
   cg_pushb_active.setColor( QPalette::Window,     global_colors.pushb );

   QPalette& cg_label = global_colors.cg_label;  // Convenience

   cg_label.setColor( QPalette::WindowText, Qt::white );
   cg_label.setColor( QPalette::Window,     global_colors.labels );
   cg_label.setColor( QPalette::Light,      Qt::black );
   cg_label.setColor( QPalette::Dark,       Qt::black );
   cg_label.setColor( QPalette::Mid,        Qt::black );
   cg_label.setColor( QPalette::Text,       Qt::white );
   cg_label.setColor( QPalette::Base,       Qt::black );

   QPalette& cg_label_warn = global_colors.cg_label_warn;  // Convenience

   cg_label_warn.setColor( QPalette::WindowText, Qt::red );
   cg_label_warn.setColor( QPalette::Button,     global_colors.labels );
   cg_label_warn.setColor( QPalette::Light,      Qt::black );
   cg_label_warn.setColor( QPalette::Dark,       Qt::black );
   cg_label_warn.setColor( QPalette::Mid,        Qt::black );
   cg_label_warn.setColor( QPalette::Text,       Qt::red );
   cg_label_warn.setColor( QPalette::BrightText, Qt::red );
   cg_label_warn.setColor( QPalette::Base,       Qt::black );
   cg_label_warn.setColor( QPalette::Window,     Qt::black );

   QPalette& label_disabled = global_colors.cg_label_disabled; // Convenience

   label_disabled.setColor( QPalette::WindowText, Qt::black );
   label_disabled.setColor( QPalette::Window,     global_colors.labels );
   label_disabled.setColor( QPalette::Light,      Qt::black );
   label_disabled.setColor( QPalette::Dark,       Qt::black );
   label_disabled.setColor( QPalette::Mid,        Qt::black );
   label_disabled.setColor( QPalette::Text,       global_colors.gray );
   label_disabled.setColor( QPalette::Base,       Qt::black );

   QPalette& cg_edit = global_colors.cg_edit;  // Convenience

   cg_edit.setColor( QPalette::WindowText,      Qt::black );
   cg_edit.setColor( QPalette::Window,          Qt::white );
   cg_edit.setColor( QPalette::Light,           Qt::white );
   cg_edit.setColor( QPalette::Dark,            global_colors.gray );
   cg_edit.setColor( QPalette::Mid,             Qt::black );
   cg_edit.setColor( QPalette::Text,            Qt::black );
   cg_edit.setColor( QPalette::Base,            Qt::white );
   cg_edit.setColor( QPalette::HighlightedText, Qt::white );
   cg_edit.setColor( QPalette::Highlight,       temp_col );
   cg_edit.setColor( QPalette::Button,          global_colors.gray3 );
   cg_edit.setColor( QPalette::Midlight,        global_colors.gray3 );
   cg_edit.setColor( QPalette::BrightText,      Qt::red );
   cg_edit.setColor( QPalette::ButtonText,      Qt::black );
   cg_edit.setColor( QPalette::Shadow,          Qt::black );

   QPalette& cg_edit_warn = global_colors.cg_edit_warn;  // Convenience

   cg_edit_warn.setColor( QPalette::WindowText, Qt::red );
   cg_edit_warn.setColor( QPalette::Button,     Qt::white );
   cg_edit_warn.setColor( QPalette::Light,      Qt::black );
   cg_edit_warn.setColor( QPalette::Dark,       global_colors.gray );
   cg_edit_warn.setColor( QPalette::Mid,        Qt::white );
   cg_edit_warn.setColor( QPalette::Text,       Qt::red );
   cg_edit_warn.setColor( QPalette::BrightText, Qt::red );
   cg_edit_warn.setColor( QPalette::Base,       Qt::black );
   cg_edit_warn.setColor( QPalette::Window,     Qt::white );

   QPalette& cg_dk_green = global_colors.cg_dk_green;  // Convenience

   cg_dk_green.setColor( QPalette::WindowText, Qt::black );
   cg_dk_green.setColor( QPalette::Window,     global_colors.dk_green );
   cg_dk_green.setColor( QPalette::Light,      Qt::white );
   cg_dk_green.setColor( QPalette::Dark,       global_colors.gray );
   cg_dk_green.setColor( QPalette::Mid,        Qt::black );
   cg_dk_green.setColor( QPalette::Text,       Qt::black );
   cg_dk_green.setColor( QPalette::Base,       Qt::black );

   QPalette& cg_dk_red = global_colors.cg_dk_red;  // Convenience

   cg_dk_red.setColor( QPalette::WindowText, Qt::black );
   cg_dk_red.setColor( QPalette::Window,     global_colors.dk_red );
   cg_dk_red.setColor( QPalette::Light,      Qt::white );
   cg_dk_red.setColor( QPalette::Dark,       global_colors.gray );
   cg_dk_red.setColor( QPalette::Mid,        Qt::black );
   cg_dk_red.setColor( QPalette::Text,       Qt::black );
   cg_dk_red.setColor( QPalette::Base,       Qt::black );

   QPalette& cg_green = global_colors.cg_green;  // Convenience

   cg_green.setColor( QPalette::WindowText, Qt::white );
   cg_green.setColor( QPalette::Window,     Qt::green );
   cg_green.setColor( QPalette::Light,      Qt::white );
   cg_green.setColor( QPalette::Dark,       global_colors.gray7 );
   cg_green.setColor( QPalette::Mid,        Qt::darkRed );
   cg_green.setColor( QPalette::Text,       Qt::darkYellow );
   cg_green.setColor( QPalette::Base,       Qt::darkCyan );

   QPalette& cg_red = global_colors.cg_red;  // Convenience

   cg_red.setColor( QPalette::WindowText, Qt::red );
   cg_red.setColor( QPalette::Window,     Qt::black );
   cg_red.setColor( QPalette::Light,      Qt::white );
   cg_red.setColor( QPalette::Dark,       global_colors.gray7 );
   cg_red.setColor( QPalette::Mid,        Qt::black );
   cg_red.setColor( QPalette::Text,       Qt::black );
   cg_red.setColor( QPalette::Base,       Qt::black );

   QPalette& cg_gray = global_colors.cg_gray;  // Convenience

   cg_gray.setColor( QPalette::WindowText, Qt::black );
   cg_gray.setColor( QPalette::Window,     Qt::gray );
   cg_gray.setColor( QPalette::Light,      Qt::white );
   cg_gray.setColor( QPalette::Dark,       Qt::black );
   cg_gray.setColor( QPalette::Mid,        Qt::black );
   cg_gray.setColor( QPalette::Text,       Qt::black );
   cg_gray.setColor( QPalette::Base,       Qt::white );

   QPalette& cg_normal = global_colors.cg_normal;  // Convenience

   cg_normal.setColor( QPalette::WindowText,      Qt::black );
   cg_normal.setColor( QPalette::Window,          global_colors.gray3 );
   cg_normal.setColor( QPalette::Light,           Qt::white );
   cg_normal.setColor( QPalette::Dark,            global_colors.gray4 );
   cg_normal.setColor( QPalette::Mid,             global_colors.gray3 );
   cg_normal.setColor( QPalette::Text,            Qt::black );
   cg_normal.setColor( QPalette::Base,            Qt::white );
   cg_normal.setColor( QPalette::HighlightedText, Qt::white );
   cg_normal.setColor( QPalette::Highlight,       temp_col );
   cg_normal.setColor( QPalette::Button,          global_colors.gray3 );
   cg_normal.setColor( QPalette::Midlight,        global_colors.gray3 );
   cg_normal.setColor( QPalette::BrightText,      Qt::red );
   cg_normal.setColor( QPalette::ButtonText,      Qt::black );
   cg_normal.setColor( QPalette::Shadow,          Qt::black );

   QPalette& cg_plot = global_colors.cg_plot;  // Convenience

   cg_plot.setColor( QPalette::WindowText,      Qt::black );
   cg_plot.setColor( QPalette::Window,          global_colors.gray3 );
   cg_plot.setColor( QPalette::Light,           Qt::white );
   cg_plot.setColor( QPalette::Dark,            Qt::white );
   cg_plot.setColor( QPalette::Mid,             global_colors.gray5 );
   cg_plot.setColor( QPalette::Text,            Qt::black );
   cg_plot.setColor( QPalette::Base,            Qt::white );
   cg_plot.setColor( QPalette::HighlightedText, Qt::white );
   cg_plot.setColor( QPalette::Highlight,       temp_col );
   cg_plot.setColor( QPalette::Button,          Qt::black );
   cg_plot.setColor( QPalette::Midlight,        Qt::black );
   cg_plot.setColor( QPalette::BrightText,      Qt::black );
   cg_plot.setColor( QPalette::ButtonText,      Qt::white );
   cg_plot.setColor( QPalette::Shadow,          Qt::black );

   QPalette& cg_lcd = global_colors.cg_lcd;  // Convenience

   cg_lcd.setColor( QPalette::WindowText, Qt::green );
   cg_lcd.setColor( QPalette::Window,     Qt::black );
   cg_lcd.setColor( QPalette::Light,      Qt::green );
   cg_lcd.setColor( QPalette::Dark,       global_colors.frames );
   cg_lcd.setColor( QPalette::Mid,        Qt::cyan );
   cg_lcd.setColor( QPalette::Text,       global_colors.dk_red );
   cg_lcd.setColor( QPalette::Base,       Qt::blue );

   QPalette& cg_bunt = global_colors.cg_bunt;  // Convenience

   cg_bunt.setColor( QPalette::WindowText, Qt::black );
   cg_bunt.setColor( QPalette::Window,     Qt::yellow );
   cg_bunt.setColor( QPalette::Light,      Qt::white );
   cg_bunt.setColor( QPalette::Dark,       Qt::red );
   cg_bunt.setColor( QPalette::Mid,        Qt::cyan );
   cg_bunt.setColor( QPalette::Text,       Qt::green );
   cg_bunt.setColor( QPalette::Base,       Qt::blue );
}

void US_Config::setDefault()
{
   QString str;

   config_list.version = US_Version_string;  // Defined in us_util.h

   config_list.browser = "/usr/bin/firefox";

#ifdef WIN32
   // internet explorer is past EOL
   // config_list.browser = "C:/Program Files (x86)/Internet Explorer/iexplore.exe";
   config_list.browser = QSettings( "HKEY_CLASSES_ROOT\\http\\shell\\open\\command", QSettings::NativeFormat ).value( "Default" ).toString();
#endif

#ifdef MAC
   config_list.browser = "/Applications/Safari.app";
#if QT_VERSION < 0x040000
   config_list.system_dir = "/Applications/UltraScanII";
#else
   QString ultrascan = getenv( "ULTRASCAN" );
   if ( ultrascan != "" )
      config_list.system_dir = QDir::toNativeSeparators( ultrascan );
   else
   {
      QString base = qApp->applicationDirPath().remove( QRegExp( "/bin(|64)$" ) );
      if ( base.contains( ".app/Contents" ) )
      {
         int jj = base.lastIndexOf( "/bin/" );
         base   = ( jj > 0 ) ? base.left( jj ) : base;
      }
      config_list.system_dir = base.remove( QRegExp( "/somo$" ) );
   }
#endif
#else

   QString ultrascan = getenv( "ULTRASCAN" );

   // Set system directory
   if ( ultrascan == "" )
   {
      ultrascan = qApp->applicationDirPath().remove( QRegExp( "/bin$" ) );
      // QString warning = us_tr( "Error:" ), message;
      // message = us_tr( "The required environment variable ULTRASCAN is not set.\n"
      //               "Please add it and restart the program.  Exiting." );
      // cerr << warning << "\n" << message << endl;
      // emit errorMessage(warning, message);
      // exit( -1 );
   }

   us_qdebug( QString( "ultrascan %1\n" ).arg( ultrascan ) );
   config_list.system_dir = QDir::toNativeSeparators( ultrascan );
#endif

   // Doc Directory
#if QT_VERSION < 0x040000
   config_list.help_dir =
      QDir::toNativeSeparators( config_list.system_dir + "/doc" );
#else
   config_list.help_dir =
      QDir::toNativeSeparators( config_list.system_dir + "/somo/doc" );
#endif

   // Set the per user directory and subdirectories
   config_list.root_dir = get_home_dir();

   config_list.data_dir =
      QDir::toNativeSeparators( config_list.root_dir + "data" );
   config_list.archive_dir =
      QDir::toNativeSeparators( config_list.root_dir + "archive" );
   config_list.result_dir =
      QDir::toNativeSeparators( config_list.root_dir + "results" );
   config_list.html_dir =
      QDir::toNativeSeparators( config_list.root_dir + "reports" );
   config_list.tmp_dir =
      QDir::toNativeSeparators( config_list.root_dir + "tmp" );

   config_list.temperature_tol = 0.5;  // Allow a max of 0.5 degrees variation
   // over course of run
   config_list.beckman_bug     = 0;    // Default: False, time doesn't have to
   // multiplied by 10
   config_list.fontFamily      = "Helvetica";
   config_list.fontSize        = 10;
   config_list.margin          = 10;
   config_list.numThreads      = 1;   // Default: 1 thread
#if QT_VERSION >= 0x050000
   config_list.numThreads      = QThread::idealThreadCount();
#endif
   if ( config_list.numThreads > 16 ) {
      config_list.numThreads = 16;
   }
}

bool US_Config::read()
{
   QFile f( get_home_dir() + USRC );

   if ( f.exists() )
   {
      QString str;

      f.open( QIODevice::ReadOnly );
      QTextStream ts( &f );
      config_list.version = ts.readLine();

#if QT_VERSION <= 0x050000
      if ( config_list.version.toFloat() < 6.0 )
      {
         f.close();
         f.remove();
         return ( false );
      }
#endif
      
      QString dummy;
      config_list.browser     = ts.readLine();
      dummy                   = ts.readLine();
      dummy                   = ts.readLine();
      config_list.help_dir    = ts.readLine();
      config_list.data_dir    = ts.readLine();
      config_list.root_dir    = ts.readLine();
      config_list.archive_dir = ts.readLine();
      config_list.result_dir  = ts.readLine();

      str = ts.readLine();
      config_list.beckman_bug = str.toInt();

      str = ts.readLine();
      config_list.temperature_tol = str.toFloat();

      // If we have an old version of the config file, file caps and/or aren't
      // present, we need to initialize this variable and write out a new
      // version that incorporates the new USglobal variable for filecaps:

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.html_dir = str;
         }
         else
         {
            return ( false );  // Bad html_dir
         }
      }

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.system_dir = str;
         }
         else
         {
            return ( false );  // Bad system_dir
         }
      }

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.fontFamily = str;
         }
         else
         {
            return ( false );  // Bad fontFamily
         }
      }

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.fontSize = str.toInt();
         }
         else
         {
            return ( false );  // Bad fontSize
         }
      }

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.margin = str.toInt();
         }
         else
         {
            return ( false );  // Bad margin
         }
      }

      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.numThreads = str.toUInt();
#if QT_VERSION >= 0x050000
            if ( config_list.numThreads <= 1 ) {
               config_list.numThreads = QThread::idealThreadCount();
            }
#endif
         }
         else
         {
            return ( false );  // Bad numThreads
         }
         if ( config_list.numThreads > 16 ) {
            config_list.numThreads = 16;
         }
      }
      if ( ! ts.atEnd() )
      {
         str = ts.readLine();
         if ( ! str.isNull() && ! str.isEmpty() )
         {
            config_list.tmp_dir = str;
         }
         else
         {
            return ( false );  // Bad temporary directory
         }
      }
      else
      {
         return ( false );  // We were not able to read everything
      }
      f.close();
   }
   else // ! f.exists()
   {
      return ( false );
   }

   // compare config_list.system_dir with applicationdirpath()
#ifndef MAC
   QString real_ultrascan = QDir::toNativeSeparators( qApp->applicationDirPath().remove( QRegExp( "/bin(|64)$" ) ) );
   QString match = config_list.system_dir != real_ultrascan ? "different" : "the same";
   
   // QMessageBox::information( 0
   //                           , "us_config::read()"
   //                           , QString( "real_ultrascan is %1\n" ).arg( real_ultrascan ) 
   //                           + QString( "system_dir is %1\n" ).arg( config_list.system_dir )
   //                           + QString( "they are %1" ).arg( match )
   //                           );

   if ( config_list.system_dir != real_ultrascan ) {
      config_list.system_dir = real_ultrascan;
#if QT_VERSION < 0x040000
      config_list.help_dir =
         QDir::toNativeSeparators( config_list.system_dir + "/doc" );
#else
      config_list.help_dir =
         QDir::toNativeSeparators( config_list.system_dir + "/somo/doc" );
#endif
      US_Write_Config *w_config;
      w_config = new US_Write_Config(this);
      if ( ! w_config->write_config( config_list ) )
      {
         QMessageBox::warning( 0, "Write error", "Error attemption to write new configuration file!" );
         delete w_config;
         exit( 1 );
      }
      // QMessageBox::information( 0, "updated", "updated configuration to reflect new program directory" );
   }
#endif
   return ( true );
}

QString US_Config::get_home_dir()
{
   QString home = QDir::homePath() + USER_DIR;
   return ( QDir::toNativeSeparators( home ) );
}

void US_Config::make_home()
{
   QString home    = get_home_dir();
   QString oldhome = QDir::homePath();

   //   create new $HOME/ultrascan directory, if it doesn't exist
   if ( ! QDir(home).exists() )
   {
      QDir tmp;
      tmp.mkdir(home);
   }

   // Make sure user's etc directory exists
   if ( ! QDir( home + ETC_DIR ).exists() )
   {
      QDir etc;
      etc.mkdir( home + ETC_DIR );
   }
}

// Move files from old locations to new if necessary
// At some time in the future, when all users have updated,
// this function can be removed
void  US_Config::move_files()
{
   QString home    = get_home_dir();
   QString oldhome = QDir::homePath();

   make_home();

   // Move 4 files if appropriate

   QString oldfile[] = { "/.usrc", "/.uscolors", "/.uslicense", "/us.db", "" };
   QString newfile[] = { USRC,     USCOLORS,     USLICENSE,     USDB };

   unsigned int i;

   for (i=0; i<4; i++)
   {
      if (   QFile( oldhome + oldfile[i] ).exists()  &&
             ! QFile( home    + newfile[i] ).exists() )
      {
         QDir file;
         file.rename( oldhome + oldfile[i], home + newfile[i] );
         cout << "Moved " << oldhome + oldfile[i] << " to " << home + newfile[i] << endl;
      }
   }

   // Move 4 directories as required

   QString dir[]   = { "archive", "data", "reports", "results", "" };
   bool    moved[] = { false,     false,  false,     false,     false };

   for (i=0; i<4; i++)
   {
      QString olddir = oldhome + "/us/" + dir[i];
      QString newdir = home    + dir[i];

      if ( QDir(olddir).exists()  &&  ! QDir(newdir).exists() )
      {
         QDir movedir;
         movedir.rename( olddir, newdir );
         moved[i] = true;
         moved[4] = true;
      }
   }

   // Rewrite USRC if necessary
   if ( moved[4] )
   {
      read();

      /*      cout <<   " system: " << config_list.system_dir
              << "\n   help: " << config_list.help_dir
              << "\n   data: " << config_list.data_dir
              << "\n   root: " << config_list.root_dir
              << "\n   html: " << config_list.html_dir
              << "\narchive: " << config_list.archive_dir
              << "\n result: " << config_list.result_dir << endl;
      */
      config_list.root_dir = home;
      if ( moved[0] ) config_list.archive_dir = home + "archive";
      if ( moved[1] ) config_list.data_dir    = home + "data";
      if ( moved[2] ) config_list.html_dir    = home + "reports";
      if ( moved[3] ) config_list.result_dir  = home + "results";
      US_Write_Config *w_config;
      w_config = new US_Write_Config(this);
      w_config->write_config( config_list );
      delete w_config;
   }
}

