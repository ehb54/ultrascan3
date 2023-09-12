#include "us_gui_settings.h"
#include "us_defines.h"

#define c_bluegreen  QColor( 0x00, 0xe0, 0xe0 )
#define c_black      QColor( Qt::black        )
#define c_white      QColor( Qt::white        )
#define c_darkGray   QColor( Qt::darkGray     )
#define c_lightGray  QColor( Qt::lightGray    )
#define c_red        QColor( Qt::red          )
#define c_darkBlue   QColor( Qt::darkBlue     )
#define c_yellow     QColor( Qt::yellow       )
#define c_green      QColor( Qt::green        )
#define c_cyan       QColor( Qt::cyan         )
#define c_darkCyan   QColor( Qt::darkCyan     )

// Fonts
QString US_GuiSettings::fontFamily( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "fontFamily",  "Helvetica" ).toString();
}

void US_GuiSettings::set_fontFamily( const QString& fontFamily )
{
  QSettings settings( US3, "UltraScan" );
  if ( fontFamily == "Helvetica" )
    settings.remove( "fontFamily" );
  else
    settings.setValue( "fontFamily", fontFamily );
}

int US_GuiSettings::fontSize( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "fontSize", 10 ).toInt();
}

void US_GuiSettings::set_fontSize( int fontSize )
{
  QSettings settings( US3, "UltraScan" );
  if ( fontSize == 10 )
    settings.remove( "fontSize" );
  else
    settings.setValue( "fontSize", fontSize );
}

QString US_GuiSettings::guiStyle( void )
{
#ifdef Q_OS_LINUX
#if QT_VERSION < 0x050000
  const QString defaultStyle( "Plastique" );
#else
  const QString defaultStyle( "Fusion" );
#endif
#endif
#ifdef Q_OS_MAC
  const QString defaultStyle( "Macintosh" );
#endif
#ifdef Q_OS_WIN
  const QString defaultStyle( "Windows" );
#endif
  QSettings settings( US3, "UltraScan" );

  return settings.value( "guiStyle", defaultStyle ).toString();
}

void US_GuiSettings::set_guiStyle( const QString& style )
{
#ifdef Q_OS_LINUX
#if QT_VERSION < 0x050000
  const QString defaultStyle( "Plastique" );
#else
  const QString defaultStyle( "Fusion" );
#endif
#endif
#ifdef Q_OS_MAC
  const QString defaultStyle( "Macintosh" );
#endif
#ifdef Q_OS_WIN
  const QString defaultStyle( "Windows" );
#endif
  QSettings settings( US3, "UltraScan" );

  if ( style == defaultStyle ) 
    settings.remove( "guiStyle" );
  else
    settings.setValue( "guiStyle", style );
}

// Misc
int US_GuiSettings::plotMargin( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "plotMargin", 10 ).toInt();
}

void US_GuiSettings::set_plotMargin( int fontSize )
{
  QSettings settings( US3, "UltraScan" );
  if ( fontSize == 10 )
    settings.remove( "plotMargin" );
  else
    settings.setValue( "plotMargin", fontSize );
}

// Palettes

// Label
QPalette US_GuiSettings::labelColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/labelColor" ) )
    return settings.value( "palettes/labelColor" ).value<QPalette>();
  return labelColorDefault(); 
}

QPalette US_GuiSettings::labelColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, c_white ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     c_black ); // background
  p.setColor( QPalette::Active, QPalette::Light,      c_black ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       c_black ); // border
  p.setColor( QPalette::Active, QPalette::Text,       c_white ); // text w/ Base
  p.setColor( QPalette::Active, QPalette::Base,       c_black ); // bg for text entry

  p.setColor( QPalette::Disabled, QPalette::WindowText, c_white ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     c_black ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      c_black ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       c_black ); // border
  p.setColor( QPalette::Disabled, QPalette::Text,       c_white ); // text w/ Base
  p.setColor( QPalette::Disabled, QPalette::Base,       c_black ); // bg for text entry

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_white ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_black ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_black ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       c_black ); // border
  p.setColor( QPalette::Inactive, QPalette::Text,       c_white ); // text w/ Base
  p.setColor( QPalette::Inactive, QPalette::Base,       c_black ); // bg for text entry

  return p; 
}

void US_GuiSettings::set_labelColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/labelColor", palette );
}

// Edit
QPalette US_GuiSettings::editColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/editColor" ) )
    return settings.value( "palettes/editColor" ).value<QPalette>();
  return editColorDefault();
}
  
QPalette US_GuiSettings::editColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, c_black ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     c_white ); // background
  p.setColor( QPalette::Active, QPalette::Light,      c_white ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        c_black ); // Between light and dark
  p.setColor( QPalette::Active, QPalette::Text,       c_black ); // text w/ Base
  p.setColor( QPalette::Active, QPalette::Base,       c_white ); // bg for text entry
  p.setColor( QPalette::Active, QPalette::HighlightedText, c_white ); // bg for selected text
  p.setColor( QPalette::Active, QPalette::Highlight,  c_darkBlue ); // selected text
  p.setColor( QPalette::Active, QPalette::Button,     c_lightGray ); // bg for button
  p.setColor( QPalette::Active, QPalette::Midlight,   c_lightGray ); // bg for text entry
  p.setColor( QPalette::Active, QPalette::BrightText, c_red   ); // contrast to WindowText
  p.setColor( QPalette::Active, QPalette::ButtonText, c_black ); // fg for button
  p.setColor( QPalette::Active, QPalette::Shadow,     c_black ); // very dark

  p.setColor( QPalette::Disabled, QPalette::WindowText, c_black ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     c_lightGray ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      c_white ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        c_black ); // Between light and dark
  p.setColor( QPalette::Disabled, QPalette::Text,       c_black ); // text w/ Base
  p.setColor( QPalette::Disabled, QPalette::Base,       c_lightGray ); // bg for text entry
  p.setColor( QPalette::Disabled, QPalette::HighlightedText, c_white ); // bg for selected text
  p.setColor( QPalette::Disabled, QPalette::Highlight,  c_darkBlue ); // selected text
  p.setColor( QPalette::Disabled, QPalette::Button,     c_darkGray ); // bg for button
  p.setColor( QPalette::Disabled, QPalette::Midlight,   c_darkGray ); // bg for text entry
  p.setColor( QPalette::Disabled, QPalette::BrightText, c_red   ); // contrast to WindowText
  p.setColor( QPalette::Disabled, QPalette::ButtonText, c_black ); // fg for button
  p.setColor( QPalette::Disabled, QPalette::Shadow,     c_black ); // very dark

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_black ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_white ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_white ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       c_darkBlue ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        c_black ); // Between light and dark
  p.setColor( QPalette::Inactive, QPalette::Text,       c_black ); // text w/ Base
  p.setColor( QPalette::Inactive, QPalette::Base,       c_white ); // bg for text entry
  p.setColor( QPalette::Inactive, QPalette::HighlightedText, c_white ); // bg for selected text
  p.setColor( QPalette::Inactive, QPalette::Highlight,  c_darkBlue ); // selected text
  p.setColor( QPalette::Inactive, QPalette::Button,     c_lightGray ); // bg for button
  p.setColor( QPalette::Inactive, QPalette::Midlight,   c_lightGray ); // bg for text entry
  p.setColor( QPalette::Inactive, QPalette::BrightText, c_red   ); // contrast to WindowText
  p.setColor( QPalette::Inactive, QPalette::ButtonText, c_black ); // fg for button
  p.setColor( QPalette::Inactive, QPalette::Shadow,     c_black ); // very dark

  return p; 
}

void US_GuiSettings::set_editColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/editColor", palette );
}

// Frame

QPalette US_GuiSettings::frameColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/frameColor" ) )
      return settings.value( "palettes/frameColor" ).value<QPalette>();
  return frameColorDefault();
}

QPalette US_GuiSettings::frameColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, c_white      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     c_darkCyan   ); // background
  p.setColor( QPalette::Active, QPalette::Light,      c_lightGray  ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       c_darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        c_cyan       ); // Between light and dark
  p.setColor( QPalette::Active, QPalette::Midlight,   c_white      ); // 
  p.setColor( QPalette::Active, QPalette::Base,       c_darkGray   ); // bg for text entry

  p.setColor( QPalette::Disabled, QPalette::WindowText, c_white    ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     c_darkCyan ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      c_lightGray); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        c_cyan     ); // Between light and dark
  p.setColor( QPalette::Disabled, QPalette::Midlight,   c_white    ); // 
  p.setColor( QPalette::Disabled, QPalette::Base,       c_darkGray ); // bg for text entry

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_white    ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_darkCyan ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_lightGray); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        c_cyan     ); // Between light and dark
  p.setColor( QPalette::Inactive, QPalette::Midlight,   c_white    ); // 
  p.setColor( QPalette::Inactive, QPalette::Base,       c_darkGray ); // bg for text entry

  return p; 
}

void US_GuiSettings::set_frameColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/frameColor", palette );
}

// Pushbutton

QPalette US_GuiSettings::pushbColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/pushbColor" ) )
      return settings.value( "palettes/pushbColor" ).value<QPalette>();
  return pushbColorDefault();
}

QPalette US_GuiSettings::pushbColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::ButtonText, c_black      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Button,     c_bluegreen  ); // background
  p.setColor( QPalette::Active, QPalette::Light,      c_white      ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       c_darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Shadow,     c_black      ); // Between light and dark

  p.setColor( QPalette::Disabled, QPalette::ButtonText, c_white    ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Button,     c_bluegreen  ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      c_white    ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Disabled, QPalette::Shadow,     c_black    ); // Between light and dark

  p.setColor( QPalette::Inactive, QPalette::ButtonText, c_black    ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Button,     c_bluegreen    ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_white    ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       c_darkGray ); // border
  p.setColor( QPalette::Inactive, QPalette::Shadow,     c_black    ); // Between light and dark

  return p; 
}

void US_GuiSettings::set_pushbColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/pushbColor", palette );
}

// Normal

QPalette US_GuiSettings::normalColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/normalColor" ) )
      return settings.value( "palettes/normalColor" ).value<QPalette>();
  return normalColorDefault();
}

QPalette US_GuiSettings::normalColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active, QPalette::WindowText, c_black      ); // windowText/foreground
  p.setColor( QPalette::Active, QPalette::Window,     c_lightGray  ); // background
  p.setColor( QPalette::Active, QPalette::Light,      c_white      ); // border
  p.setColor( QPalette::Active, QPalette::Dark,       c_darkGray   ); // border
  p.setColor( QPalette::Active, QPalette::Mid,        c_lightGray  );
  p.setColor( QPalette::Active, QPalette::Text,       c_black      );
  p.setColor( QPalette::Active, QPalette::Base,       c_white      );
  p.setColor( QPalette::Active, QPalette::HighlightedText,  c_white  );
  p.setColor( QPalette::Active, QPalette::Highlight,  c_darkBlue   );
  p.setColor( QPalette::Active, QPalette::Button,     c_lightGray  );
  p.setColor( QPalette::Active, QPalette::Midlight,   c_lightGray  );
  p.setColor( QPalette::Active, QPalette::BrightText, c_red        );
  p.setColor( QPalette::Active, QPalette::ButtonText, c_black      );
  p.setColor( QPalette::Active, QPalette::Shadow,     c_black      ); // Between light and dark

  p.setColor( QPalette::Disabled, QPalette::WindowText, c_black      ); // windowText/foreground
  p.setColor( QPalette::Disabled, QPalette::Window,     c_lightGray  ); // background
  p.setColor( QPalette::Disabled, QPalette::Light,      c_white      ); // border
  p.setColor( QPalette::Disabled, QPalette::Dark,       c_darkGray   ); // border
  p.setColor( QPalette::Disabled, QPalette::Mid,        c_lightGray  );
  p.setColor( QPalette::Disabled, QPalette::Text,       c_black      );
  p.setColor( QPalette::Disabled, QPalette::Base,       c_lightGray  );
  p.setColor( QPalette::Disabled, QPalette::HighlightedText,  c_white );
  p.setColor( QPalette::Disabled, QPalette::Highlight,  c_darkBlue   );
  p.setColor( QPalette::Disabled, QPalette::Button,     c_lightGray  );
  p.setColor( QPalette::Disabled, QPalette::Midlight,   c_lightGray  );
  p.setColor( QPalette::Disabled, QPalette::BrightText, c_red        );
  p.setColor( QPalette::Disabled, QPalette::ButtonText, c_black      );
  p.setColor( QPalette::Disabled, QPalette::Shadow,     c_black      ); // Between light and dark

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_black      ); // windowText/foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_lightGray  ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_white      ); // border
  p.setColor( QPalette::Inactive, QPalette::Dark,       c_darkGray   ); // border
  p.setColor( QPalette::Inactive, QPalette::Mid,        c_lightGray  );
  p.setColor( QPalette::Inactive, QPalette::Text,       c_black      );
  p.setColor( QPalette::Inactive, QPalette::Base,       c_white      );
  p.setColor( QPalette::Inactive, QPalette::HighlightedText,  c_white  );
  p.setColor( QPalette::Inactive, QPalette::Highlight,  c_darkBlue   );
  p.setColor( QPalette::Inactive, QPalette::Button,     c_lightGray  );
  p.setColor( QPalette::Inactive, QPalette::Midlight,   c_lightGray  );
  p.setColor( QPalette::Inactive, QPalette::BrightText, c_red        );
  p.setColor( QPalette::Inactive, QPalette::ButtonText, c_black      );
  p.setColor( QPalette::Inactive, QPalette::Shadow,     c_black      ); // Between light and dark

  return p; 
}

void US_GuiSettings::set_normalColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/normalColor", palette );
}

// LCD 
QPalette US_GuiSettings::lcdColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/lcdColor" ) )
      return settings.value( "palettes/lcdColor" ).value<QPalette>();
  return lcdColorDefault();
}

QPalette US_GuiSettings::lcdColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active,   QPalette::WindowText, c_green    ); // foreground
  p.setColor( QPalette::Active,   QPalette::Window,     c_black    ); // background
  p.setColor( QPalette::Active,   QPalette::Light,      c_green    ); // highlight1
  p.setColor( QPalette::Active,   QPalette::Shadow,     c_darkCyan ); // highlight2

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_green    ); // foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_black    ); // background
  p.setColor( QPalette::Inactive, QPalette::Light,      c_green    ); // highlight1
  p.setColor( QPalette::Inactive, QPalette::Shadow,     c_darkCyan ); // highlight2

  return p; 
}

void US_GuiSettings::set_lcdColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/lcdColor", palette );
}

// Plot frame
QPalette US_GuiSettings::plotColor( void )
{
  QSettings settings( US3, "UltraScan" );
  if ( settings.contains( "palettes/plotColor" ) )
      return settings.value( "palettes/plotColor" ).value<QPalette>();
  return plotColorDefault();
}

QPalette US_GuiSettings::plotColorDefault( void )
{
  QPalette p;
  p.setColor( QPalette::Active,   QPalette::WindowText, c_black     ); // foreground
  p.setColor( QPalette::Active,   QPalette::Window,     c_lightGray ); // background
  p.setColor( QPalette::Active,   QPalette::Shadow,     c_black     ); // highlights

  p.setColor( QPalette::Inactive, QPalette::WindowText, c_black     ); // foreground
  p.setColor( QPalette::Inactive, QPalette::Window,     c_lightGray ); // background
  p.setColor( QPalette::Inactive, QPalette::Shadow,     c_black     ); // highlights

  return p; 
}

void US_GuiSettings::set_plotColor( const QPalette& palette )
{
  QSettings settings( US3, "UltraScan" );
  settings.setValue( "palettes/plotColor", palette );
}


// Plot curve color
QColor US_GuiSettings::plotCurve( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "palettes/plotCurve", c_yellow ).value<QColor>();
}

void US_GuiSettings::set_plotCurve( const QColor& color )
{
  QSettings settings( US3, "UltraScan" );
  if ( color == Qt::yellow )
    settings.remove( "palettes/plotCurve" );
  else
    settings.setValue( "palettes/plotCurve", color );
}

// Canvas background
QColor US_GuiSettings::plotCanvasBG( void )
{
  QSettings settings( US3, "UltraScan" );
  //return settings.value( "palettes/normalColor", c_darkBlue ).value<QColor>();
  return settings.value( "palettes/plotCanvasBG", c_darkBlue ).value<QColor>();
}

void US_GuiSettings::set_plotCanvasBG( const QColor& color )
{
  QSettings settings( US3, "UltraScan" );
  if ( color == Qt::darkBlue )
    settings.remove( "palettes/plotCanvasBG" );
  else
    settings.setValue( "palettes/plotCanvasBG", color );
}

// Canvas major gridlines
QColor US_GuiSettings::plotMajGrid( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "palettes/plotMajGrid", c_white ).value<QColor>();
}

void US_GuiSettings::set_plotMajGrid( const QColor& color )
{
  QSettings settings( US3, "UltraScan" );
  if ( color == c_white )
    settings.remove( "palettes/plotMajGrid" );
  else
    settings.setValue( "palettes/plotMajGrid", color );
}

// Canvas minor gridlines
QColor US_GuiSettings::plotMinGrid( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "palettes/plotMinGrid", c_lightGray ).value<QColor>();
}

void US_GuiSettings::set_plotMinGrid( const QColor& color )
{
  QSettings settings( US3, "UltraScan" );
  if ( color == Qt::lightGray )
    settings.remove( "palettes/plotMinGrid" );
  else
    settings.setValue( "palettes/plotMinGrid", color );
}

// Plot Picker rubber band pen and Tracker pen color
QColor US_GuiSettings::plotPicker( void )
{
  QSettings settings( US3, "UltraScan" );
  return settings.value( "palettes/plotPicker", c_white ).value<QColor>();
}

void US_GuiSettings::set_plotPicker( const QColor& color )
{
  QSettings settings( US3, "UltraScan" );
  if ( color == c_white )
    settings.remove( "palettes/plotPicker" );
  else
    settings.setValue( "palettes/plotPicker", color );
}

void US_GuiSettings::set_gui_env(void) {
   QSettings settings(US3, "UltraScan");
   auto test = Qt::HighDpiScaleFactorRoundingPolicy(settings.value("HDPIP", 5).toInt());
   QString highDpiscaling = settings.value("HDPIS", "0").toString();
   QString autoScaling = settings.value("HDPIAS", "0").toString();
   qputenv("QT_ENABLE_HIGHDPI_SCALING",highDpiscaling.toLatin1());
   QApplication::setHighDpiScaleFactorRoundingPolicy(test);
   QApplication::setAttribute(Qt::AA_EnableHighDpiScaling, (autoScaling == "1"));
   QApplication::setAttribute(Qt::AA_DisableHighDpiScaling, (autoScaling == "2"));
}
