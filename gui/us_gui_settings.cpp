// UltraScan III GUI settings + default theme palettes (Light Professional, teal-accent)

#include "us_gui_settings.h"
#include "us_defines.h"

#include <QSettings>
#include <QPalette>
#include <QColor>
#include <QStyleFactory>
#include <QApplication>
#include <QFont>
#include <QWidget>
#include <algorithm>

// ---------------------------------------------------------------------------
// Legacy base colors (retained for LCD/other helpers and compatibility)
// ---------------------------------------------------------------------------
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

// Small helper
static inline QColor qhex(const char* hex) { return QColor(QString::fromLatin1(hex)); }

// ===========================================================================
// Fonts
// ===========================================================================
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

QString US_GuiSettings::guiStyle()
{
#ifdef Q_OS_LINUX
    const QString defaultStyle("Fusion");
#endif
#ifdef Q_OS_MAC
    const QString defaultStyle("Macintosh");
#endif
#ifdef Q_OS_WIN
    const QString defaultStyle("Windows");
#endif

    QSettings settings(US3, "UltraScan");
    return settings.value("guiStyle", defaultStyle).toString();
}

void US_GuiSettings::set_guiStyle(const QString& style)
{
#ifdef Q_OS_LINUX
    const QString defaultStyle("Fusion");
#endif
#ifdef Q_OS_MAC
    const QString defaultStyle("Macintosh");
#endif
#ifdef Q_OS_WIN
    const QString defaultStyle("Windows");
#endif

    QSettings settings(US3, "UltraScan");
    if (style == defaultStyle)
        settings.remove("guiStyle");
    else
        settings.setValue("guiStyle", style);
}

// ===========================================================================
// Misc
// ===========================================================================
int US_GuiSettings::plotMargin( void )
{
    QSettings settings( US3, "UltraScan" );
    return settings.value( "plotMargin", 10 ).toInt();
}

void US_GuiSettings::set_plotMargin( int m )
{
    QSettings settings( US3, "UltraScan" );
    if ( m == 10 )
        settings.remove( "plotMargin" );
    else
        settings.setValue( "plotMargin", m );
}

// ===========================================================================
// Palettes: getters/setters with defaults (persisted in QSettings)
// ===========================================================================

// Label
QPalette US_GuiSettings::labelColor( void )
{
    QSettings settings( US3, "UltraScan" );
    if ( settings.contains( "palettes/labelColor" ) )
        return settings.value( "palettes/labelColor" ).value<QPalette>();
    return labelColorDefault();
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

void US_GuiSettings::set_editColor( const QPalette& palette )
{
    QSettings settings( US3, "UltraScan" );
    settings.setValue( "palettes/editColor", palette );
}

// Frame (general widget)
QPalette US_GuiSettings::frameColor( void )
{
    QSettings settings( US3, "UltraScan" );
    if ( settings.contains( "palettes/frameColor" ) )
        return settings.value( "palettes/frameColor" ).value<QPalette>();
    return frameColorDefault();
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

void US_GuiSettings::set_pushbColor( const QPalette& palette )
{
    QSettings settings( US3, "UltraScan" );
    settings.setValue( "palettes/pushbColor", palette );
}

// Normal (fallback)
QPalette US_GuiSettings::normalColor( void )
{
    QSettings settings( US3, "UltraScan" );
    if ( settings.contains( "palettes/normalColor" ) )
        return settings.value( "palettes/normalColor" ).value<QPalette>();
    return normalColorDefault();
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

void US_GuiSettings::set_plotColor( const QPalette& palette )
{
    QSettings settings( US3, "UltraScan" );
    settings.setValue( "palettes/plotColor", palette );
}

// Plot curve color
QColor US_GuiSettings::plotCurve( void )
{
    QSettings settings( US3, "UltraScan" );
    return settings.value( "palettes/plotCurve", QColor("#007BA7") ).value<QColor>();
}

void US_GuiSettings::set_plotCurve( const QColor& color )
{
    QSettings settings( US3, "UltraScan" );
    if ( color == QColor("#007BA7") )
        settings.remove( "palettes/plotCurve" );
    else
        settings.setValue( "palettes/plotCurve", color );
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

// ---------------------------------------------------------------------------
// UltraScan III "Light Professional" theme
// - Comfortable neutral surfaces for long analysis sessions
// - White plot canvases for maximum readability
// - Teal accent from splash for focus/selections only
// ---------------------------------------------------------------------------

// Core palette colors (new defaults)
static const QColor c_bg         ("#F7F8FA");  // app window background
static const QColor c_base       ("#FFFFFF");  // input fields, tables, canvas base
static const QColor c_panel      ("#ECEEF1");  // grouped control panels
static const QColor c_alt        ("#ECEEF1");  // alternate rows / raised blocks
static const QColor c_text       ("#2E2E2E");  // primary text
static const QColor c_textMuted  ("#5A5E66");  // secondary/disabled text
static const QColor c_border     ("#D2D5DA");  // dividers, frames, 1px lines
static const QColor c_btn        ("#E9F6F5");  // enabled button face (light-teal wash)
static const QColor c_teal       ("#009C9E");  // accent (selection/focus/primary)

// ========================== Default palette builders =========================
QPalette US_GuiSettings::frameColorDefault()
{
    // Neutral gray tones aligned with Qwt default palette
    const QColor c_bg         ("#e6e6e6"); // window background
    const QColor c_base       ("#f0f0f0"); // widget base background
    const QColor c_alt        ("#f8f8f8"); // alternate row / subtle highlight
    const QColor c_text       ("#1e1e1e"); // main text
    const QColor c_textMuted  ("#7a7a7a"); // disabled / muted text
    const QColor c_btn        ("#d6d6d6"); // button background
    const QColor c_highlight  ("#b0b0b0"); // selection highlight (neutral gray)
    const QColor c_midline    ("#cccccc"); // splitter groove / light borders
    const QColor c_darkline   ("#999999"); // splitter edge / darker borders
    const QColor c_shadow     ("#666666"); // strong frame shadows

    QPalette p(QPalette::Active);

    // ----- Active group -----
    p.setColor(QPalette::Window,         c_bg);
    p.setColor(QPalette::WindowText,     c_text);
    p.setColor(QPalette::Base,           c_base);
    p.setColor(QPalette::AlternateBase,  c_alt);
    p.setColor(QPalette::Text,           c_text);
    p.setColor(QPalette::ToolTipBase,    c_base);
    p.setColor(QPalette::ToolTipText,    c_text);

    // Buttons
    p.setColor(QPalette::Button,         c_btn);
    p.setColor(QPalette::ButtonText,     QColor("#000000"));

    // Selection / focus / links (neutral, no teal)
    p.setColor(QPalette::Highlight,         c_highlight);
    p.setColor(QPalette::HighlightedText,   Qt::white);
    p.setColor(QPalette::Link,              QColor("#404040"));
    p.setColor(QPalette::LinkVisited,       QColor("#606060"));
    p.setColor(QPalette::BrightText,        QColor("#000000"));

    // Lines / frames (affects QSplitter handle in Fusion)
    p.setColor(QPalette::Light,        QColor("#FFFFFF"));
    p.setColor(QPalette::Midlight,     QColor("#f2f2f2"));
    p.setColor(QPalette::Mid,          c_midline);
    p.setColor(QPalette::Dark,         c_darkline);
    p.setColor(QPalette::Shadow,       c_shadow);

    // ----- Disabled group -----
    p.setColor(QPalette::Disabled, QPalette::Window,        c_bg);
    p.setColor(QPalette::Disabled, QPalette::WindowText,    c_textMuted);
    p.setColor(QPalette::Disabled, QPalette::Base,          c_base);
    p.setColor(QPalette::Disabled, QPalette::AlternateBase, c_alt);
    p.setColor(QPalette::Disabled, QPalette::Text,          c_textMuted);
    p.setColor(QPalette::Disabled, QPalette::Button,        QColor("#dadada"));
    p.setColor(QPalette::Disabled, QPalette::ButtonText,    QColor("#8a8a8a"));
    p.setColor(QPalette::Disabled, QPalette::Highlight,     QColor("#cccccc"));
    p.setColor(QPalette::Disabled, QPalette::HighlightedText, QColor("#f0f0f0"));

    // ----- Inactive mirrors Active for consistency -----
    const QPalette a = p;
    for (int role = 0; role < QPalette::NColorRoles; ++role)
        p.setColor(QPalette::Inactive, static_cast<QPalette::ColorRole>(role),
                   a.color(QPalette::Active, static_cast<QPalette::ColorRole>(role)));

    return p;
}

// Standard widget palette (fallback to frame defaults)
QPalette US_GuiSettings::normalColorDefault( void )
{
    return frameColorDefault();
}

// Push button palette
QPalette US_GuiSettings::pushbColorDefault( void )
{
    QPalette p = frameColorDefault();
    p.setColor(QPalette::Button,     c_btn);
    p.setColor(QPalette::ButtonText, c_text);
    return p;
}

// Line edit / text field palette
QPalette US_GuiSettings::editColorDefault( void )
{
    QPalette p = frameColorDefault();
    p.setColor(QPalette::Base, c_base);
    p.setColor(QPalette::Text, c_text);
    return p;
}

// Label palette
QPalette US_GuiSettings::labelColorDefault( void )
{
    QPalette p = frameColorDefault();
    p.setColor(QPalette::WindowText, c_text);
    p.setColor(QPalette::Text,       c_text);
    return p;
}

// Plot / Qwt default colors — white canvas, dark axes, light grid
QPalette US_GuiSettings::plotColorDefault( void )
{
    QPalette p;
    p.setColor(QPalette::Window,      Qt::white);   // canvas
    p.setColor(QPalette::Base,        Qt::white);
    p.setColor(QPalette::WindowText,  Qt::black);   // axes/ticks
    p.setColor(QPalette::Text,        Qt::black);
    p.setColor(QPalette::Highlight,   c_teal);      // selections/rubberband
    p.setColor(QPalette::Shadow,      c_border);    // frame lines
    return p;
}

// LCD
QPalette US_GuiSettings::lcdColorDefault( void )
{
    QPalette p;
    p.setColor( QPalette::Active,   QPalette::WindowText, QColor("#4CAF50") );
    p.setColor( QPalette::Active,   QPalette::Window,     Qt::black );
    p.setColor( QPalette::Inactive, QPalette::WindowText, QColor("#4CAF50") );
    p.setColor( QPalette::Inactive, QPalette::Window,     Qt::black );
    return p;
}

// ---------------------------------------------------------------------------
// Plot canvas supporting colors (getters with defaults)
// ---------------------------------------------------------------------------
QColor US_GuiSettings::plotCanvasBG( void )
{
    QSettings s( US3, "UltraScan" );
    return s.value( "palettes/plotCanvasBG", QColor("#FFFFFF") ).value<QColor>();
}

QColor US_GuiSettings::plotMajGrid( void )
{
    QSettings s( US3, "UltraScan" );
    return s.value( "palettes/plotMajGrid", QColor("#D6D8DC") ).value<QColor>();
}

QColor US_GuiSettings::plotMinGrid( void )
{
    QSettings s( US3, "UltraScan" );
    return s.value( "palettes/plotMinGrid", QColor("#ECEFF3") ).value<QColor>();
}

// ---------------------------------------------------------------------------
// Plot canvas supporting colors (SETTERS)
// ---------------------------------------------------------------------------
void US_GuiSettings::set_plotCanvasBG( const QColor& c )
{
    QSettings s( US3, "UltraScan" );
    if ( c == QColor("#FFFFFF") )
        s.remove( "palettes/plotCanvasBG" );
    else
        s.setValue( "palettes/plotCanvasBG", c );
}

void US_GuiSettings::set_plotMajGrid( const QColor& c )
{
    QSettings s( US3, "UltraScan" );
    if ( c == QColor("#D6D8DC") )
        s.remove( "palettes/plotMajGrid" );
    else
        s.setValue( "palettes/plotMajGrid", c );
}

void US_GuiSettings::set_plotMinGrid( const QColor& c )
{
    QSettings s( US3, "UltraScan" );
    if ( c == QColor("#ECEFF3") )
        s.remove( "palettes/plotMinGrid" );
    else
        s.setValue( "palettes/plotMinGrid", c );
}

// ---------------------------------------------------------------------------
// NEW: Apply the default palette app-wide (safe, optional).
// If your header already declares set_GUI_Colors(QApplication*), this
// will be picked up automatically by all programs that call it.
// ---------------------------------------------------------------------------
void US_GuiSettings::set_GUI_Colors(QApplication* app)
{
    if (!app) return;

    // Normalize look across platforms
    app->setStyle(QStyleFactory::create("Fusion"));

    // Use our default frame palette as the application palette
    app->setPalette(frameColorDefault());

    // Remove any lingering global QSS that could fight the palette
    app->setStyleSheet(QString());

    // Slightly larger, readable default font
    QFont f = app->font();
    if (f.pointSize() > 0) f.setPointSize(std::max(10, f.pointSize() + 1));
    app->setFont(f);
}

// ---------------------------------------------------------------------------
// OPTIONAL: Clear stale per-widget stylesheets that override palette.
// Call once in a window ctor after setupUi(...), if needed.
// ---------------------------------------------------------------------------
void US_GuiSettings::scrubChildStyleSheets(QWidget* root)
{
    if (!root) return;

    const QList<QWidget*> all = root->findChildren<QWidget*>();
    for (QWidget* w : all)
    {
        if (!w) continue;
        if (!w->styleSheet().isEmpty())
            w->setStyleSheet(QString());
        w->update();
    }
    root->update();
}
