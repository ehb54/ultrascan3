#include "us_gui_settings.h"

// Fonts
QString US_GuiSettings::fontFamily( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "fontFamily",  "Helvetica" ).toString();
}

void US_GuiSettings::set_fontFamily( const QString& fontFamily )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "fontFamily", fontFamily );
}

int US_GuiSettings::fontSize( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  return settings.value( "fontSize", 10 ).toInt();
}

void US_GuiSettings::set_fontSize( int fontSize )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "fontSize", fontSize );
}

// Palettes

// Label
QPalette US_GuiSettings::labelColor( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( settings.contains( "palettes/labelColor" ) )
    return settings.value( "palettes/labelColor" ).value<QPalette>();
  return labelColorDefault(); 
}

QPalette US_GuiSettings::labelColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     Qt::black ); // background
  p.setColor( QPalette::Active, QPalette::Light,      Qt::black ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       Qt::black ); // border
  p.setColor( QPalette::Active, QPalette::Text,       Qt::white ); // text w/ Base
  p.setColor( QPalette::Active, QPalette::Base,       Qt::black ); // bg for text entry

  p.setColor( QPalette::Disabled, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     Qt::black ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      Qt::black ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       Qt::black ); // border
  p.setColor( QPalette::Disabled, QPalette::Text,       Qt::white ); // text w/ Base
  p.setColor( QPalette::Disabled, QPalette::Base,       Qt::black ); // bg for text entry

  p.setColor( QPalette::Inactive, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     Qt::black ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      Qt::black ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       Qt::black ); // border
  p.setColor( QPalette::Inactive, QPalette::Text,       Qt::white ); // text w/ Base
  p.setColor( QPalette::Inactive, QPalette::Base,       Qt::black ); // bg for text entry

  return p; 
}

void US_GuiSettings::set_labelColor( const QPalette& palette )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "palettes/labelColor", palette );
}

// Edit
QPalette US_GuiSettings::editColor( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( settings.contains( "palettes/editColor" ) )
    return settings.value( "palettes/editColor" ).value<QPalette>();
  return editColorDefault();
}
  
QPalette US_GuiSettings::editColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     Qt::white ); // background
  p.setColor( QPalette::Active, QPalette::Light,      Qt::white ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        Qt::black ); // Between light and dark
  p.setColor( QPalette::Active, QPalette::Text,       Qt::black ); // text w/ Base
  p.setColor( QPalette::Active, QPalette::Base,       Qt::white ); // bg for text entry
  p.setColor( QPalette::Active, QPalette::HighlightedText, Qt::white ); // bg for selected text
  p.setColor( QPalette::Active, QPalette::Highlight,  Qt::darkBlue ); // selected text
  p.setColor( QPalette::Active, QPalette::Button,     Qt::lightGray ); // bg for button
  p.setColor( QPalette::Active, QPalette::Midlight,   Qt::lightGray ); // bg for text entry
  p.setColor( QPalette::Active, QPalette::BrightText, Qt::red   ); // contrast to WindowText
  p.setColor( QPalette::Active, QPalette::ButtonText, Qt::black ); // fg for button
  p.setColor( QPalette::Active, QPalette::Shadow,     Qt::black ); // very dark

  p.setColor( QPalette::Disabled, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     Qt::white ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      Qt::white ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        Qt::black ); // Between light and dark
  p.setColor( QPalette::Disabled, QPalette::Text,       Qt::black ); // text w/ Base
  p.setColor( QPalette::Disabled, QPalette::Base,       Qt::white ); // bg for text entry
  p.setColor( QPalette::Disabled, QPalette::HighlightedText, Qt::white ); // bg for selected text
  p.setColor( QPalette::Disabled, QPalette::Highlight,  Qt::darkBlue ); // selected text
  p.setColor( QPalette::Disabled, QPalette::Button,     Qt::darkGray ); // bg for button
  p.setColor( QPalette::Disabled, QPalette::Midlight,   Qt::darkGray ); // bg for text entry
  p.setColor( QPalette::Disabled, QPalette::BrightText, Qt::red   ); // contrast to WindowText
  p.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::black ); // fg for button
  p.setColor( QPalette::Disabled, QPalette::Shadow,     Qt::black ); // very dark

  p.setColor( QPalette::Inactive, QPalette::WindowText, Qt::black ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     Qt::white ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      Qt::white ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       Qt::darkBlue ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        Qt::black ); // Between light and dark
  p.setColor( QPalette::Inactive, QPalette::Text,       Qt::black ); // text w/ Base
  p.setColor( QPalette::Inactive, QPalette::Base,       Qt::white ); // bg for text entry
  p.setColor( QPalette::Inactive, QPalette::HighlightedText, Qt::white ); // bg for selected text
  p.setColor( QPalette::Inactive, QPalette::Highlight,  Qt::darkBlue ); // selected text
  p.setColor( QPalette::Inactive, QPalette::Button,     Qt::lightGray ); // bg for button
  p.setColor( QPalette::Inactive, QPalette::Midlight,   Qt::lightGray ); // bg for text entry
  p.setColor( QPalette::Inactive, QPalette::BrightText, Qt::red   ); // contrast to WindowText
  p.setColor( QPalette::Inactive, QPalette::ButtonText, Qt::black ); // fg for button
  p.setColor( QPalette::Inactive, QPalette::Shadow,     Qt::black ); // very dark

  return p; 
}

void US_GuiSettings::set_editColor( const QPalette& palette )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "palettes/editColor", palette );
}

// Frame

QPalette US_GuiSettings::frameColor( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( settings.contains( "palettes/frameColor" ) )
      return settings.value( "palettes/frameColor" ).value<QPalette>();
  return frameColorDefault();
}

#define gray2 QColor( 0xde, 0xde, 0xde )

QPalette US_GuiSettings::frameColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, Qt::white      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     Qt::darkCyan   ); // background
  p.setColor( QPalette::Active, QPalette::Light,      gray2          ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       Qt::darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        Qt::cyan       ); // Between light and dark
  p.setColor( QPalette::Active, QPalette::Text,       Qt::white      ); // text w/ Base
  p.setColor( QPalette::Active, QPalette::Base,       Qt::darkGray   ); // bg for text entry

  p.setColor( QPalette::Disabled, QPalette::WindowText, Qt::white    ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     Qt::darkCyan ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      gray2        ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        Qt::cyan     ); // Between light and dark
  p.setColor( QPalette::Disabled, QPalette::Text,       Qt::white    ); // text w/ Base
  p.setColor( QPalette::Disabled, QPalette::Base,       Qt::darkGray ); // bg for text entry

  p.setColor( QPalette::Inactive, QPalette::WindowText, Qt::white    ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     Qt::darkCyan ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      gray2        ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        Qt::cyan     ); // Between light and dark
  p.setColor( QPalette::Inactive, QPalette::Text,       Qt::white    ); // text w/ Base
  p.setColor( QPalette::Inactive, QPalette::Base,       Qt::darkGray ); // bg for text entry

  return p; 
}

void US_GuiSettings::set_frameColor( const QPalette& palette )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "palettes/frameColor", palette );
}

// Pushbutton

QPalette US_GuiSettings::pushbColor( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( settings.contains( "palettes/pushbColor" ) )
      return settings.value( "palettes/pushbColor" ).value<QPalette>();
  return pushbColorDefault();
}

#define bluegreen QColor( 0x00, 0xe0, 0xe0 )

QPalette US_GuiSettings::pushbColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::ButtonText, Qt::black      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Button,     bluegreen      ); // background
  p.setColor( QPalette::Active, QPalette::Light,      Qt::white      ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       Qt::darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Shadow,     Qt::black      ); // Between light and dark

  p.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::white    ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Button,     bluegreen    ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      Qt::white    ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Shadow,     Qt::black    ); // Between light and dark

  p.setColor( QPalette::Inactive, QPalette::ButtonText, Qt::black    ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Button,     bluegreen    ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      Qt::white    ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       Qt::darkGray ); // border
  p.setColor( QPalette::Inactive, QPalette::Shadow,     Qt::black    ); // Between light and dark

  return p; 
}

void US_GuiSettings::set_pushbColor( const QPalette& palette )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "palettes/pushbColor", palette );
}

// Normal

QPalette US_GuiSettings::normalColor( void )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  if ( settings.contains( "palettes/normalColor" ) )
      return settings.value( "palettes/normalColor" ).value<QPalette>();
  return normalColorDefault();
}

QPalette US_GuiSettings::normalColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::ButtonText, Qt::black      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Button,     Qt::lightGray  ); // background
  p.setColor( QPalette::Active, QPalette::Light,      Qt::white      ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       Qt::darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        Qt::lightGray  );
  p.setColor( QPalette::Active, QPalette::Text,       Qt::black      );
  p.setColor( QPalette::Active, QPalette::Base,       Qt::white      );
  p.setColor( QPalette::Active, QPalette::HighlightedText,  Qt::white  );
  p.setColor( QPalette::Active, QPalette::Highlight,  Qt::darkBlue   );
  p.setColor( QPalette::Active, QPalette::Button,     Qt::lightGray  );
  p.setColor( QPalette::Active, QPalette::Midlight,   Qt::lightGray  );
  p.setColor( QPalette::Active, QPalette::BrightText, Qt::red        );
  p.setColor( QPalette::Active, QPalette::ButtonText, Qt::black      );
  p.setColor( QPalette::Active, QPalette::Shadow,     Qt::black      ); // Between light and dark

  p.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::black      ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Button,     Qt::lightGray  ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      Qt::white      ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       Qt::darkGray   ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        Qt::lightGray  );
  p.setColor( QPalette::Disabled, QPalette::Text,       Qt::black      );
  p.setColor( QPalette::Disabled, QPalette::Base,       Qt::white      );
  p.setColor( QPalette::Disabled, QPalette::HighlightedText,  Qt::white );
  p.setColor( QPalette::Disabled, QPalette::Highlight,  Qt::darkBlue   );
  p.setColor( QPalette::Disabled, QPalette::Button,     Qt::lightGray  );
  p.setColor( QPalette::Disabled, QPalette::Midlight,   Qt::lightGray  );
  p.setColor( QPalette::Disabled, QPalette::BrightText, Qt::red        );
  p.setColor( QPalette::Disabled, QPalette::ButtonText, Qt::black      );
  p.setColor( QPalette::Disabled, QPalette::Shadow,     Qt::black      ); // Between light and dark

  p.setColor( QPalette::Inactive, QPalette::ButtonText, Qt::black      ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Button,     Qt::lightGray  ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      Qt::white      ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       Qt::darkGray   ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        Qt::lightGray  );
  p.setColor( QPalette::Inactive, QPalette::Text,       Qt::black      );
  p.setColor( QPalette::Inactive, QPalette::Base,       Qt::white      );
  p.setColor( QPalette::Inactive, QPalette::HighlightedText,  Qt::white  );
  p.setColor( QPalette::Inactive, QPalette::Highlight,  Qt::darkBlue   );
  p.setColor( QPalette::Inactive, QPalette::Button,     Qt::lightGray  );
  p.setColor( QPalette::Inactive, QPalette::Midlight,   Qt::lightGray  );
  p.setColor( QPalette::Inactive, QPalette::BrightText, Qt::red        );
  p.setColor( QPalette::Inactive, QPalette::ButtonText, Qt::black      );
  p.setColor( QPalette::Inactive, QPalette::Shadow,     Qt::black      ); // Between light and dark

  return p; 
}

void US_GuiSettings::set_normalColor( const QPalette& palette )
{
  QSettings settings( "UTHSCSA", "UltraScan" );
  settings.setValue( "palettes/normalColor", palette );
}

