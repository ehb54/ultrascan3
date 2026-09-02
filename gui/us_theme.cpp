//! \file us_theme.cpp
#include "us_theme.h"
#include "us_style.h"
#include "us_gui_settings.h"
#include "us_defines.h"

#if QT_VERSION >= QT_VERSION_CHECK( 6, 5, 0 )
#include <QStyleHints>
#endif

namespace
{
   QString  applied_signature;
   bool     watcher_installed = false;

   int      sampled_system_scheme = -1;
}

// The two token tables.  These are *the* place to edit UltraScan's colors.
US_ThemeTokens US_Theme::tokens(const Scheme s )
{
   US_ThemeTokens t;

   if ( s == Dark )
   {
      t.window        = QColor( 0x21, 0x26, 0x2d );
      t.windowAlt     = QColor( 0x2a, 0x30, 0x38 );
      t.windowText    = QColor( 0xe4, 0xe9, 0xee );
      t.mutedText     = QColor( 0x9b, 0xa6, 0xb2 );
      t.disabledText  = QColor( 0x8d, 0x97, 0xa3 );

      t.base          = QColor( 0x17, 0x1b, 0x21 );
      t.baseAlt       = QColor( 0x1d, 0x22, 0x2a );
      t.baseReadOnly  = QColor( 0x26, 0x2c, 0x34 );
      t.text          = QColor( 0xe4, 0xe9, 0xee );

      t.button        = QColor( 0x2d, 0x33, 0x3b );
      t.buttonHover   = QColor( 0x39, 0x41, 0x4b );
      t.buttonPressed = QColor( 0x47, 0x50, 0x5b );
      t.buttonText    = QColor( 0xe4, 0xe9, 0xee );

      t.border        = QColor( 0x3d, 0x44, 0x4d );
      t.borderStrong  = QColor( 0x52, 0x5b, 0x66 );
      t.shadow        = QColor( 0x0d, 0x10, 0x14 );

      t.accent        = QColor( 0x1f, 0x6e, 0x88 );
      t.accentText    = QColor( 0xff, 0xff, 0xff );
      t.accentBright  = QColor( 0x4e, 0xb8, 0xd6 );
      t.attention     = QColor( 0xff, 0x8a, 0x7a );

      t.bannerBg      = QColor( 0x1f, 0x6e, 0x88 );
      t.bannerText    = QColor( 0xf2, 0xfa, 0xfd );

      t.tipBg         = QColor( 0x2a, 0x30, 0x38 );
      t.tipText       = QColor( 0xe4, 0xe9, 0xee );

      t.lcdBg         = QColor( 0x10, 0x16, 0x1d );
      t.lcdText       = QColor( 0x3d, 0xdc, 0x97 );
      t.lcdHi1        = QColor( 0x3d, 0xdc, 0x97 );
      t.lcdHi2        = QColor( 0x14, 0x58, 0x6b );
   }

   else
   {
      t.window        = QColor( 0xf2, 0xf4, 0xf7 );
      t.windowAlt     = QColor( 0xe8, 0xec, 0xf1 );
      t.windowText    = QColor( 0x1b, 0x27, 0x33 );
      t.mutedText     = QColor( 0x5a, 0x66, 0x72 );
      t.disabledText  = QColor( 0x98, 0xa2, 0xae );

      t.base          = QColor( 0xff, 0xff, 0xff );
      t.baseAlt       = QColor( 0xf7, 0xf9, 0xfb );
      t.baseReadOnly  = QColor( 0xe9, 0xed, 0xf1 );
      t.text          = QColor( 0x1b, 0x27, 0x33 );

      t.button        = QColor( 0xff, 0xff, 0xff );
      t.buttonHover   = QColor( 0xe4, 0xed, 0xf5 );
      t.buttonPressed = QColor( 0xd3, 0xdf, 0xea );
      t.buttonText    = QColor( 0x1b, 0x27, 0x33 );

      t.border        = QColor( 0xc9, 0xd2, 0xdb );
      t.borderStrong  = QColor( 0xa9, 0xb5, 0xc1 );
      t.shadow        = QColor( 0x8c, 0x99, 0xa6 );

      t.accent        = QColor( 0x0e, 0x6e, 0x8c );
      t.accentText    = QColor( 0xff, 0xff, 0xff );
      t.accentBright  = QColor( 0x0e, 0x6e, 0x8c );
      t.attention     = QColor( 0xc0, 0x39, 0x2b );

      t.bannerBg      = QColor( 0x0e, 0x6e, 0x8c );
      t.bannerText    = QColor( 0xff, 0xff, 0xff );

      t.tipBg         = QColor( 0x25, 0x31, 0x3d );
      t.tipText       = QColor( 0xf4, 0xf7, 0xfa );

      t.lcdBg         = QColor( 0x10, 0x16, 0x1d );
      t.lcdText       = QColor( 0x2f, 0xbf, 0x82 );
      t.lcdHi1        = QColor( 0x2f, 0xbf, 0x82 );
      t.lcdHi2        = QColor( 0x0e, 0x6e, 0x8c );
   }

   // The plot canvas stays dark in both schemes.  Many analysis programs set
   // a black canvas and bright curve colors explicitly, so a light canvas
   // would make UltraScan's plots inconsistent with each other.
   t.plotBg        = QColor( 0x10, 0x18, 0x22 );
   t.plotMajGrid   = QColor( 0x6a, 0x76, 0x83 );
   t.plotMinGrid   = QColor( 0x41, 0x4b, 0x57 );
   t.plotPicker    = QColor( 0xe8, 0xee, 0xf4 );
   t.plotCurve     = QColor( Qt::yellow );

   return t;
}

US_ThemeTokens US_Theme::tokens( void )
{
   return tokens( scheme() );
}

QString US_Theme::schemeSetting( void )
{
   const QSettings settings( US3, "UltraScan" );
   return settings.value( "colorScheme", "auto" ).toString().toLower();
}

void US_Theme::set_schemeSetting( const QString& pref )
{
   QSettings settings( US3, "UltraScan" );

   if ( pref.compare( "auto", Qt::CaseInsensitive ) == 0 )
      settings.remove  ( "colorScheme" );
   else
      settings.setValue( "colorScheme", pref.toLower() );

   invalidate();
}

US_Theme::Scheme US_Theme::systemScheme( void )
{
#if QT_VERSION >= QT_VERSION_CHECK( 6, 5, 0 )
   // Qt 6.5 and later know what the desktop asks for and keep the answer up
   // to date, so it can be queried every time.
   if ( QGuiApplication::styleHints() != nullptr )
   {
      const Qt::ColorScheme cs = QGuiApplication::styleHints()->colorScheme();

      if ( cs == Qt::ColorScheme::Dark )
         return Dark;

      if ( cs == Qt::ColorScheme::Light )
         return Light;
   }
#endif

   // Otherwise fall back to the lightness of the system window color.  It is
   // sampled only once, since after apply() the application palette is ours.
   if ( sampled_system_scheme < 0 )
   {
      if ( QGuiApplication::instance() == nullptr )
         return Light;

      const QColor win = QGuiApplication::palette().color( QPalette::Window );
      sampled_system_scheme = ( win.lightness() < 128 ) ? Dark : Light;
   }

   return ( sampled_system_scheme == Dark ) ? Dark : Light;
}

US_Theme::Scheme US_Theme::scheme( void )
{
   const QString pref = schemeSetting();

   if ( pref == "light" ) return Light;
   if ( pref == "dark"  ) return Dark;

   return systemScheme();
}

bool US_Theme::isDark( void )
{
   return ( scheme() == Dark );
}

QString US_Theme::defaultStyle( void )
{
   // Fusion is available on every platform Qt supports and is the only style
   // that renders identically on Linux, Windows and macOS.  It is also the
   // only built-in style that honors an application palette everywhere, which
   // is what makes the UltraScan color configuration work at all.
   return QString( "Fusion" );
}

QStyle* US_Theme::createStyle( const QString& name )
{
   QStyle* base = QStyleFactory::create( name );

   if ( base == nullptr )
      base = QStyleFactory::create( defaultStyle() );

   if ( base == nullptr )
      return nullptr;

   // US_Style adds the UltraScan shapes on top of whichever style the user
   // selected, taking its colors from each widget's own palette.
   return new US_Style( base );
}

QFont US_Theme::baseFont( void )
{
   return QFont( US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() );
}

QPalette US_Theme::applicationPalette( void )
{
   const US_ThemeTokens t = tokens();

   // Start from the "Other Widgets" palette so that a user who redefines it
   // also restyles menus, dialogs and everything else that has no palette of
   // its own, then fill in the roles that palette does not cover.
   QPalette p = US_GuiSettings::normalColor();

   const QList< QPalette::ColorGroup > groups = QList< QPalette::ColorGroup >()
      << QPalette::Active << QPalette::Inactive << QPalette::Disabled;

   for (const auto g : groups)
   {
      p.setColor( g, QPalette::AlternateBase, t.baseAlt      );
      p.setColor( g, QPalette::ToolTipBase  , t.tipBg        );
      p.setColor( g, QPalette::ToolTipText  , t.tipText      );
      p.setColor( g, QPalette::Link         , t.accentBright );
      p.setColor( g, QPalette::LinkVisited  , t.accentBright );
   }

   p.setColor( QPalette::Active  , QPalette::PlaceholderText, t.mutedText    );
   p.setColor( QPalette::Inactive, QPalette::PlaceholderText, t.mutedText    );
   p.setColor( QPalette::Disabled, QPalette::PlaceholderText, t.disabledText );

   return p;
}

QString US_Theme::styleSheet( void )
{
   const US_ThemeTokens t = tokens();

   // What belongs in this style sheet and what does not
   // -------------------------------------------------
   // A style sheet rule that touches a widget's box takes the painting of
   // that widget away from the style, and Qt then ignores the QPalette that
   // was set on the widget: inside a rule, palette() always resolves against
   // the *application* palette, at widget level just as much as here.
   //
   // Everything the user can recolor - labels, banners, push buttons, entry
   // fields, item views, LCDs, plots - and everything a program may recolor
   // per widget is therefore shaped by US_Style, which is handed the widget's
   // own palette.  Only top level chrome that never carries a palette of its
   // own is left to the rules below.
   QString qss;

   // ---- Tool tips -------------------------------------------------------
   qss += QString(
      "QToolTip {"
      " color: %1; background-color: %2; border: 1px solid %3;"
      " border-radius: 4px; padding: 4px 6px; }\n" )
      .arg( t.tipText.name(), t.tipBg.name(), t.border.name() );

   // ---- Menus -----------------------------------------------------------
   qss +=
      "QMenuBar {"
      " background-color: palette(window); color: palette(window-text);"
      " border-bottom: 1px solid palette(mid); }\n"
      "QMenuBar::item {"
      " background: transparent; padding: 5px 10px; border-radius: 4px; }\n"
      "QMenuBar::item:selected {"
      " background-color: palette(highlight); color: palette(highlighted-text); }\n"
      "QMenu {"
      " background-color: palette(base); color: palette(text);"
      " border: 1px solid palette(mid); border-radius: 4px; padding: 4px; }\n"
      "QMenu::item { padding: 5px 22px; border-radius: 4px; }\n"
      "QMenu::item:selected {"
      " background-color: palette(highlight); color: palette(highlighted-text); }\n"
      "QMenu::separator {"
      " height: 1px; background: palette(mid); margin: 4px 8px; }\n";

   qss += QString(
      "QMenu::item:disabled, QMenuBar::item:disabled { color: %1; }\n" )
      .arg( t.disabledText.name() );

   // ---- Splitters -------------------------------------------------------
   qss +=
      "QSplitter::handle { background: palette(mid); }\n"
      "QSplitter::handle:horizontal { width: 3px; }\n"
      "QSplitter::handle:vertical { height: 3px; }\n";

   return qss;
}

QPalette US_Theme::paletteFor(const Role r )
{
   switch ( r )
   {
      case Frame:      return US_GuiSettings::frameColor   ();
      case Banner:     return US_GuiSettings::bannerColor  ();
      case Label:      return US_GuiSettings::labelColor   ();
      case Pushbutton: return US_GuiSettings::pushbColor   ();
      case Edit:       return US_GuiSettings::editColor    ();
      case ReadOnly:   return US_GuiSettings::readonlyColor();
      case Lcd:        return US_GuiSettings::lcdColor     ();
      case PlotFrame:  return US_GuiSettings::plotColor    ();
      default:         return US_GuiSettings::normalColor  ();
   }
}

void US_Theme::tag( QWidget* w, const Role r )
{
   if ( w == nullptr )
      return;

   w->setProperty( roleProperty(), static_cast< int >( r ) );
   w->setPalette ( paletteFor( r ) );
}

void US_Theme::restyle( void )
{
   if ( qApp == nullptr )
      return;

   const QWidgetList widgets = QApplication::allWidgets();

   for ( QWidget* w : widgets )
   {
      if ( w == nullptr )
         continue;

      const QVariant v = w->property( roleProperty() );

      if ( ! v.isValid() )
         continue;

      const auto r = static_cast< Role >( v.toInt() );

      if ( r == NoRole )
         continue;

      const QPalette p = paletteFor( r );

      w->setPalette( p );

      // A scroll area paints the surface behind its rows through the
      // viewport, which is a child widget with a palette of its own.
      if ( const auto* sa = qobject_cast< QAbstractScrollArea* >( w );
           sa != nullptr  &&  sa->viewport() != nullptr )
         sa->viewport()->setPalette( p );

      w->update();
   }
}

int US_Theme::serial( void )
{
   const QSettings settings( US3, "UltraScan" );
   return settings.value( "themeSerial", 0 ).toInt();
}

void US_Theme::bumpSerial( void )
{
   QSettings settings( US3, "UltraScan" );
   settings.setValue( "themeSerial", settings.value( "themeSerial", 0 ).toInt() + 1 );
   settings.sync();
}

void US_Theme::invalidate( void )
{
   applied_signature.clear();
}

void US_Theme::apply(const bool force )
{
   if ( qApp == nullptr )
      return;

   // Sample the desktop's scheme before we replace the application palette
   systemScheme();

   const QString signature = QString( "%1|%2|%3|%4|%5" )
      .arg( US_GuiSettings::guiStyle () )
      .arg( US_GuiSettings::fontFamily() )
      .arg( US_GuiSettings::fontSize  () )
      .arg( static_cast< int >( scheme() ) )
      .arg( serial() );

   if ( ! force  &&  signature == applied_signature )
      return;

   applied_signature = signature;

   QStyle* style = createStyle( US_GuiSettings::guiStyle() );

   if ( style != nullptr )
      QApplication::setStyle( style );

   // The palette has to follow the style: setStyle() installs the style's
   // own standard palette.
   QApplication::setPalette  ( applicationPalette() );
   QApplication::setFont     ( baseFont() );
   qApp->setStyleSheet       ( styleSheet() );

   // Windows that are already on screen carry their own widget palettes and
   // would otherwise keep the colors of the scheme they were built under.
   // restyle();

#if QT_VERSION >= QT_VERSION_CHECK( 6, 5, 0 )
   if ( ! watcher_installed  &&  QGuiApplication::styleHints() != nullptr )
   {
      watcher_installed = true;

      QObject::connect( QGuiApplication::styleHints(),
                        &QStyleHints::colorSchemeChanged,
                        qApp,
                        []( Qt::ColorScheme )
                        {
                           // Follow the desktop switching between light and
                           // dark while UltraScan is running.
                           US_Theme::invalidate();
                           US_Theme::apply();
                        } );
   }
#endif
}
