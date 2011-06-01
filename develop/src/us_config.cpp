#include "../include/us_util.h"
#include "../include/us_write_config.h"
#include "../include/us_version.h"

US_Config::US_Config( QObject* parent, const char* name)
   : QObject (parent, name)
{
   US_Version = US_Version_string; // Defined in us_util.h
   
   if ( ! read( ) )
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
US_Config::US_Config( QString /* dummy */, QObject* parent, const char* name )
   : QObject (parent, name)
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
   if ( f.open( IO_WriteOnly ) )
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

   m.clear(  );
   m.push_back( tr( "1-Component, Ideal" ) );                        // model 0
   m.push_back( tr( "2-Component, Ideal, Noninteracting" ) );        // model 1
   m.push_back( tr( "3-Component, Ideal, Noninteracting" ) );        // model 2
   m.push_back( tr( "Fixed Molecular Weight Distribution" ) );       // model 3
   m.push_back( tr( "Monomer-Dimer Equilibrium" ) );                 // model 4
   m.push_back( tr( "Monomer-Trimer Equilibrium" ) );                // model 5
   m.push_back( tr( "Monomer-Tetramer Equilibrium" ) );              // model 6
   m.push_back( tr( "Monomer-Pentamer Equilibrium" ) );              // model 7
   m.push_back( tr( "Monomer-Hexamer Equilibrium" ) );               // model 8
   m.push_back( tr( "Monomer-Heptamer Equilibrium" ) );              // model 9
   m.push_back( tr( "User-Defined Monomer-Nmer Equilibrium" ) );     // model 10
   m.push_back( tr( "Monomer-Dimer-Trimer Equilibrium" ) );          // model 11
   m.push_back( tr( "Monomer-Dimer-Tetramer Equilibrium" ) );        // model 12
   m.push_back( tr( "User-Defined Monomer - N-mer - M-mer Equilibrium" ) );
   // model 13
   m.push_back( tr( "2-Component Hetero-Association: A + B <=> AB" ) );
   // model 14
   m.push_back( tr( "User-defined self/Hetero-Association: A + B <=> AB, nA <=> An" ) );
   // model 15
   m.push_back( tr( "User-Defined Monomer-Nmer, some monomer is incompetent" ) );
   // model 16
   m.push_back( tr( "User-Defined Monomer-Nmer, some Nmer is incompetent" ) );
   // model 17
   m.push_back( tr( "User-Defined irreversible Monomer-Nmer" ) );    // model 18
   m.push_back( tr( "User-Defined Monomer-Nmer plus contaminant" ) );// model 19
}

bool US_Config::col_exists()
{
   QString version = "0.0";
   bool    flag    = false;

   QString  colfile = US_Config::get_home_dir() + USCOLORS;

   QFile f( colfile );

   if ( f.open( IO_ReadOnly ) )
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

      if ( ! f.atEnd(  ) )
      {
         ds >> version;
         if ( version.toFloat(  ) > 7.0 )
         {
            flag = true;
         }
      }
   }

   if ( ! flag )
   {
      if ( write_default_colors(  ) )
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

   QColorGroup& cg_frame = global_colors.cg_frame;  // Convenience

   cg_frame.setColor( QColorGroup::Foreground, Qt::white );
   cg_frame.setColor( QColorGroup::Background, global_colors.frames );
   cg_frame.setColor( QColorGroup::Light,      global_colors.gray2 );
   cg_frame.setColor( QColorGroup::Dark,       global_colors.gray4 );
   cg_frame.setColor( QColorGroup::Mid,        Qt::cyan );
   cg_frame.setColor( QColorGroup::Text,       Qt::white );
   cg_frame.setColor( QColorGroup::Base,       global_colors.gray4 );

   QColorGroup& cg_pushb = global_colors.cg_pushb;  // Convenience

   cg_pushb.setColor( QColorGroup::ButtonText, Qt::black );
   cg_pushb.setColor( QColorGroup::Button,     global_colors.pushb );
   cg_pushb.setColor( QColorGroup::Light,      Qt::white );
   cg_pushb.setColor( QColorGroup::Dark,       global_colors.gray4 );
   cg_pushb.setColor( QColorGroup::Shadow,     Qt::black );
   // For splash screen window
   cg_pushb.setColor( QColorGroup::Background, global_colors.pushb );

   QColorGroup& cg_pushb_disabled = global_colors.cg_pushb_disabled;  // Convenience

   cg_pushb_disabled.setColor( QColorGroup::ButtonText, Qt::white );
   cg_pushb_disabled.setColor( QColorGroup::Button,     global_colors.pushb );
   cg_pushb_disabled.setColor( QColorGroup::Light,      Qt::white );
   cg_pushb_disabled.setColor( QColorGroup::Dark,       global_colors.gray4 );
   cg_pushb_disabled.setColor( QColorGroup::Shadow,     Qt::black );
   // For splash screen shadow
   cg_pushb_disabled.setColor( QColorGroup::Background, global_colors.pushb );
   cg_pushb_disabled.setColor( QColorGroup::Text,       global_colors.gray3 );

   QColorGroup& cg_pushb_active = global_colors.cg_pushb_active;  // Convenience

   cg_pushb_active.setColor( QColorGroup::ButtonText, Qt::black );
   cg_pushb_active.setColor( QColorGroup::Button,     global_colors.pushb );
   cg_pushb_active.setColor( QColorGroup::Light,      Qt::white );
   cg_pushb_active.setColor( QColorGroup::Dark,       global_colors.gray4 );
   cg_pushb_active.setColor( QColorGroup::Shadow,     Qt::black );
   // For splash screen shadow
   cg_pushb_active.setColor( QColorGroup::Background, global_colors.pushb );

   QColorGroup& cg_label = global_colors.cg_label;  // Convenience

   cg_label.setColor( QColorGroup::Foreground, Qt::white );
   cg_label.setColor( QColorGroup::Background, global_colors.labels );
   cg_label.setColor( QColorGroup::Light,      Qt::black );
   cg_label.setColor( QColorGroup::Dark,       Qt::black );
   cg_label.setColor( QColorGroup::Mid,        Qt::black );
   cg_label.setColor( QColorGroup::Text,       Qt::white );
   cg_label.setColor( QColorGroup::Base,       Qt::black );

   QColorGroup& cg_label_warn = global_colors.cg_label_warn;  // Convenience

   cg_label_warn.setColor( QColorGroup::Foreground, Qt::red );
   cg_label_warn.setColor( QColorGroup::Button,     global_colors.labels );
   cg_label_warn.setColor( QColorGroup::Light,      Qt::black );
   cg_label_warn.setColor( QColorGroup::Dark,       Qt::black );
   cg_label_warn.setColor( QColorGroup::Mid,        Qt::black );
   cg_label_warn.setColor( QColorGroup::Text,       Qt::red );
   cg_label_warn.setColor( QColorGroup::BrightText, Qt::red );
   cg_label_warn.setColor( QColorGroup::Base,       Qt::black );
   cg_label_warn.setColor( QColorGroup::Background, Qt::black );

   QColorGroup& label_disabled = global_colors.cg_label_disabled; // Convenience

   label_disabled.setColor( QColorGroup::Foreground, Qt::black );
   label_disabled.setColor( QColorGroup::Background, global_colors.labels );
   label_disabled.setColor( QColorGroup::Light,      Qt::black );
   label_disabled.setColor( QColorGroup::Dark,       Qt::black );
   label_disabled.setColor( QColorGroup::Mid,        Qt::black );
   label_disabled.setColor( QColorGroup::Text,       global_colors.gray );
   label_disabled.setColor( QColorGroup::Base,       Qt::black );

   QColorGroup& cg_edit = global_colors.cg_edit;  // Convenience

   cg_edit.setColor( QColorGroup::Foreground,      Qt::black );
   cg_edit.setColor( QColorGroup::Background,      Qt::white );
   cg_edit.setColor( QColorGroup::Light,           Qt::white );
   cg_edit.setColor( QColorGroup::Dark,            global_colors.gray );
   cg_edit.setColor( QColorGroup::Mid,             Qt::black );
   cg_edit.setColor( QColorGroup::Text,            Qt::black );
   cg_edit.setColor( QColorGroup::Base,            Qt::white );
   cg_edit.setColor( QColorGroup::HighlightedText, Qt::white );
   cg_edit.setColor( QColorGroup::Highlight,       temp_col );
   cg_edit.setColor( QColorGroup::Button,          global_colors.gray3 );
   cg_edit.setColor( QColorGroup::Midlight,        global_colors.gray3 );
   cg_edit.setColor( QColorGroup::BrightText,      Qt::red );
   cg_edit.setColor( QColorGroup::ButtonText,      Qt::black );
   cg_edit.setColor( QColorGroup::Shadow,          Qt::black );

   QColorGroup& cg_edit_warn = global_colors.cg_edit_warn;  // Convenience

   cg_edit_warn.setColor( QColorGroup::Foreground, Qt::red );
   cg_edit_warn.setColor( QColorGroup::Button,     Qt::white );
   cg_edit_warn.setColor( QColorGroup::Light,      Qt::black );
   cg_edit_warn.setColor( QColorGroup::Dark,       global_colors.gray );
   cg_edit_warn.setColor( QColorGroup::Mid,        Qt::white );
   cg_edit_warn.setColor( QColorGroup::Text,       Qt::red );
   cg_edit_warn.setColor( QColorGroup::BrightText, Qt::red );
   cg_edit_warn.setColor( QColorGroup::Base,       Qt::black );
   cg_edit_warn.setColor( QColorGroup::Background, Qt::white );

   QColorGroup& cg_dk_green = global_colors.cg_dk_green;  // Convenience

   cg_dk_green.setColor( QColorGroup::Foreground, Qt::black );
   cg_dk_green.setColor( QColorGroup::Background, global_colors.dk_green );
   cg_dk_green.setColor( QColorGroup::Light,      Qt::white );
   cg_dk_green.setColor( QColorGroup::Dark,       global_colors.gray );
   cg_dk_green.setColor( QColorGroup::Mid,        Qt::black );
   cg_dk_green.setColor( QColorGroup::Text,       Qt::black );
   cg_dk_green.setColor( QColorGroup::Base,       Qt::black );

   QColorGroup& cg_dk_red = global_colors.cg_dk_red;  // Convenience

   cg_dk_red.setColor( QColorGroup::Foreground, Qt::black );
   cg_dk_red.setColor( QColorGroup::Background, global_colors.dk_red );
   cg_dk_red.setColor( QColorGroup::Light,      Qt::white );
   cg_dk_red.setColor( QColorGroup::Dark,       global_colors.gray );
   cg_dk_red.setColor( QColorGroup::Mid,        Qt::black );
   cg_dk_red.setColor( QColorGroup::Text,       Qt::black );
   cg_dk_red.setColor( QColorGroup::Base,       Qt::black );

   QColorGroup& cg_green = global_colors.cg_green;  // Convenience

   cg_green.setColor( QColorGroup::Foreground, Qt::white );
   cg_green.setColor( QColorGroup::Background, Qt::green );
   cg_green.setColor( QColorGroup::Light,      Qt::white );
   cg_green.setColor( QColorGroup::Dark,       global_colors.gray7 );
   cg_green.setColor( QColorGroup::Mid,        Qt::darkRed );
   cg_green.setColor( QColorGroup::Text,       Qt::darkYellow );
   cg_green.setColor( QColorGroup::Base,       Qt::darkCyan );

   QColorGroup& cg_red = global_colors.cg_red;  // Convenience

   cg_red.setColor( QColorGroup::Foreground, Qt::red );
   cg_red.setColor( QColorGroup::Background, Qt::black );
   cg_red.setColor( QColorGroup::Light,      Qt::white );
   cg_red.setColor( QColorGroup::Dark,       global_colors.gray7 );
   cg_red.setColor( QColorGroup::Mid,        Qt::black );
   cg_red.setColor( QColorGroup::Text,       Qt::black );
   cg_red.setColor( QColorGroup::Base,       Qt::black );

   QColorGroup& cg_gray = global_colors.cg_gray;  // Convenience

   cg_gray.setColor( QColorGroup::Foreground, Qt::black );
   cg_gray.setColor( QColorGroup::Background, Qt::gray );
   cg_gray.setColor( QColorGroup::Light,      Qt::white );
   cg_gray.setColor( QColorGroup::Dark,       Qt::black );
   cg_gray.setColor( QColorGroup::Mid,        Qt::black );
   cg_gray.setColor( QColorGroup::Text,       Qt::black );
   cg_gray.setColor( QColorGroup::Base,       Qt::white );

   QColorGroup& cg_normal = global_colors.cg_normal;  // Convenience

   cg_normal.setColor( QColorGroup::Foreground,      Qt::black );
   cg_normal.setColor( QColorGroup::Background,      global_colors.gray3 );
   cg_normal.setColor( QColorGroup::Light,           Qt::white );
   cg_normal.setColor( QColorGroup::Dark,            global_colors.gray4 );
   cg_normal.setColor( QColorGroup::Mid,             global_colors.gray3 );
   cg_normal.setColor( QColorGroup::Text,            Qt::black );
   cg_normal.setColor( QColorGroup::Base,            Qt::white );
   cg_normal.setColor( QColorGroup::HighlightedText, Qt::white );
   cg_normal.setColor( QColorGroup::Highlight,       temp_col );
   cg_normal.setColor( QColorGroup::Button,          global_colors.gray3 );
   cg_normal.setColor( QColorGroup::Midlight,        global_colors.gray3 );
   cg_normal.setColor( QColorGroup::BrightText,      Qt::red );
   cg_normal.setColor( QColorGroup::ButtonText,      Qt::black );
   cg_normal.setColor( QColorGroup::Shadow,          Qt::black );

   QColorGroup& cg_plot = global_colors.cg_plot;  // Convenience

   cg_plot.setColor( QColorGroup::Foreground,      Qt::black );
   cg_plot.setColor( QColorGroup::Background,      global_colors.gray3 );
   cg_plot.setColor( QColorGroup::Light,           Qt::white );
   cg_plot.setColor( QColorGroup::Dark,            Qt::white );
   cg_plot.setColor( QColorGroup::Mid,             global_colors.gray5 );
   cg_plot.setColor( QColorGroup::Text,            Qt::black );
   cg_plot.setColor( QColorGroup::Base,            Qt::white );
   cg_plot.setColor( QColorGroup::HighlightedText, Qt::white );
   cg_plot.setColor( QColorGroup::Highlight,       temp_col );
   cg_plot.setColor( QColorGroup::Button,          Qt::black );
   cg_plot.setColor( QColorGroup::Midlight,        Qt::black );
   cg_plot.setColor( QColorGroup::BrightText,      Qt::black );
   cg_plot.setColor( QColorGroup::ButtonText,      Qt::white );
   cg_plot.setColor( QColorGroup::Shadow,          Qt::black );

   QColorGroup& cg_lcd = global_colors.cg_lcd;  // Convenience

   cg_lcd.setColor( QColorGroup::Foreground, Qt::green );
   cg_lcd.setColor( QColorGroup::Background, Qt::black );
   cg_lcd.setColor( QColorGroup::Light,      Qt::green );
   cg_lcd.setColor( QColorGroup::Dark,       global_colors.frames );
   cg_lcd.setColor( QColorGroup::Mid,        Qt::cyan );
   cg_lcd.setColor( QColorGroup::Text,       global_colors.dk_red );
   cg_lcd.setColor( QColorGroup::Base,       Qt::blue );

   QColorGroup& cg_bunt = global_colors.cg_bunt;  // Convenience

   cg_bunt.setColor( QColorGroup::Foreground, Qt::black );
   cg_bunt.setColor( QColorGroup::Background, Qt::yellow );
   cg_bunt.setColor( QColorGroup::Light,      Qt::white );
   cg_bunt.setColor( QColorGroup::Dark,       Qt::red );
   cg_bunt.setColor( QColorGroup::Mid,        Qt::cyan );
   cg_bunt.setColor( QColorGroup::Text,       Qt::green );
   cg_bunt.setColor( QColorGroup::Base,       Qt::blue );
}

void US_Config::setDefault()
{
   QString str;

   config_list.version = US_Version_string;  // Defined in us_util.h

   config_list.browser = "/usr/bin/firefox";

#ifdef WIN32
   config_list.browser = "C:/Program Files/Internet Explorer/iexplore.exe";
#endif

#ifdef OSX
   config_list.browser = "open";
#endif

   QString ultrascan = getenv( "ULTRASCAN" );

   // Set system directory
   if ( ultrascan == "" )
   {
      QString warning = tr( "Error:" ), message;
      message = tr( "The required environment variable ULTRASCAN is not set.\n"
                    "Please add it and restart the program.  Exiting." );
      cerr << warning << "\n" << message << endl;
      emit errorMessage(warning, message);
      exit( -1 );
   }

   config_list.system_dir = QDir::convertSeparators( ultrascan );

   // Doc Directory
   config_list.help_dir =
      QDir::convertSeparators( config_list.system_dir + "/doc" );

   // Set the per user directory and subdirectories
   config_list.root_dir = get_home_dir(  );

   config_list.data_dir =
      QDir::convertSeparators( config_list.root_dir + "data" );
   config_list.archive_dir =
      QDir::convertSeparators( config_list.root_dir + "archive" );
   config_list.result_dir =
      QDir::convertSeparators( config_list.root_dir + "results" );
   config_list.html_dir =
      QDir::convertSeparators( config_list.root_dir + "reports" );
   config_list.tmp_dir =
      QDir::convertSeparators( config_list.root_dir + "tmp" );

   config_list.temperature_tol = 0.5;  // Allow a max of 0.5 degrees variation
   // over course of run
   config_list.beckman_bug     = 0;    // Default: False, time doesn't have to
   // multiplied by 10
   config_list.fontFamily      = "Helvetica";
   config_list.fontSize        = 10;
   config_list.margin          = 10;
   config_list.numThreads      = 1;   // Default: 1 thread
}

bool US_Config::read()
{
   QFile f( get_home_dir() + USRC );

   if ( f.exists() )
   {
      QString str;

      f.open( IO_ReadOnly );
      QTextStream ts( &f );
      config_list.version = ts.readLine(  );

      if ( config_list.version.toFloat(  ) < 6.0 )
      {
         f.close(  );
         f.remove(  );
         return ( false );
      }

      QString dummy;
      config_list.browser     = ts.readLine(  );
      dummy                   = ts.readLine(  );
      dummy                   = ts.readLine(  );
      config_list.help_dir    = ts.readLine(  );
      config_list.data_dir    = ts.readLine(  );
      config_list.root_dir    = ts.readLine(  );
      config_list.archive_dir = ts.readLine(  );
      config_list.result_dir  = ts.readLine(  );

      str = ts.readLine(  );
      config_list.beckman_bug = str.toInt(  );

      str = ts.readLine(  );
      config_list.temperature_tol = str.toFloat(  );

      // If we have an old version of the config file, file caps and/or aren't
      // present, we need to initialize this variable and write out a new
      // version that incorporates the new USglobal variable for filecaps:

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.html_dir = str;
         }
         else
         {
            return ( false );  // Bad html_dir
         }
      }

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.system_dir = str;
         }
         else
         {
            return ( false );  // Bad system_dir
         }
      }

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.fontFamily = str;
         }
         else
         {
            return ( false );  // Bad fontFamily
         }
      }

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.fontSize = str.toInt(  );
         }
         else
         {
            return ( false );  // Bad fontSize
         }
      }

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.margin = str.toInt(  );
         }
         else
         {
            return ( false );  // Bad margin
         }
      }

      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
         {
            config_list.numThreads = str.toUInt(  );
         }
         else
         {
            return ( false );  // Bad numThreads
         }
      }
      if ( ! ts.atEnd(  ) )
      {
         str = ts.readLine(  );
         if ( ! str.isNull(  ) && ! str.isEmpty(  ) )
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

   }
   else // ! f.exists(  )
   {
      return ( false );
   }
   return ( true );
}

QString US_Config::get_home_dir(  )
{
   QString home = QDir::homeDirPath(  ) + USER_DIR;
   return ( QDir::convertSeparators( home ) );
}

// Move files from old locations to new if necessary
// At some time in the future, when all users have updated,
// theis function can be removed
void  US_Config::move_files(  )
{
   QString home    = get_home_dir();
   QString oldhome = QDir::homeDirPath(  );

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
         file.rename( oldhome + oldfile[i], home + newfile[i], true );
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
         movedir.rename( olddir, newdir, true );
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

