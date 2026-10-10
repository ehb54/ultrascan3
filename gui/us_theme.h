//! \file us_theme.h
#ifndef US_THEME_H
#define US_THEME_H

#include <QtWidgets>
#include <QtGui>

#include "us_extern.h"

//! \brief The set of design tokens that make up one UltraScan color scheme.
struct US_GUI_EXTERN US_ThemeTokens
{
   // Surfaces and text
   QColor window;         //!< Window / dialog background
   QColor windowAlt;      //!< Slightly raised surface on top of the window
   QColor windowText;     //!< Default foreground on \ref window
   QColor mutedText;      //!< De-emphasized foreground
   QColor disabledText;   //!< Foreground of disabled widgets

   // Data entry
   QColor base;           //!< Background of editable fields and lists
   QColor baseAlt;        //!< Alternating row background
   QColor baseReadOnly;   //!< Background of read-only fields
   QColor text;           //!< Foreground on \ref base

   // Buttons
   QColor button;         //!< Push button background
   QColor buttonHover;    //!< Push button background, mouse over
   QColor buttonPressed;  //!< Push button background, pressed
   QColor buttonText;     //!< Push button foreground

   // Lines
   QColor border;         //!< Ordinary 1px separators and widget outlines
   QColor borderStrong;   //!< Emphasized separators
   QColor shadow;         //!< Darkest line color

   // Accent
   QColor accent;         //!< Selection / focus / primary accent fill
   QColor accentText;     //!< Foreground on \ref accent
   QColor accentBright;   //!< Accent variant that stays legible as a line
   QColor attention;      //!< "Bright text" - warnings and error emphasis

   // Section banners
   QColor bannerBg;       //!< Section header background
   QColor bannerText;     //!< Section header foreground

   // Tool tips
   QColor tipBg;          //!< Tool tip background
   QColor tipText;        //!< Tool tip foreground

   // Plots
   QColor plotBg;         //!< Plot canvas background
   QColor plotMajGrid;    //!< Major grid lines
   QColor plotMinGrid;    //!< Minor grid lines
   QColor plotPicker;     //!< Picker / tracker cross hairs
   QColor plotCurve;      //!< Default curve color

   // LCD panels
   QColor lcdBg;          //!< LCD background
   QColor lcdText;        //!< LCD digits
   QColor lcdHi1;         //!< LCD highlight 1
   QColor lcdHi2;         //!< LCD highlight 2
};

//! \brief The central look-and-feel definition for all UltraScan programs.
/*!  US_Theme owns three things:
     - the token table for the light and the dark color scheme,
     - the decision which of the two is currently in effect,
     - the application wide QStyle, palette, font and style sheet.
*/
class US_GUI_EXTERN US_Theme
{
   public:

      //! \brief The two color schemes UltraScan ships with
      enum Scheme
      {
         Light,   //!< Light scheme
         Dark     //!< Dark scheme
      };

      //! \brief The palette family a widget belongs to
      /*!  Every UltraScan widget is built with a palette of its own so that
           the color configuration panel can recolor each kind of widget
           separately.  A widget palette does not follow
           QApplication::setPalette(), so a widget that is already on screen
           when the color scheme changes would keep its old colors.  Tagging
           it with the family it came from is what lets restyle() rebuild it.
      */
      enum Role
      {
         NoRole = 0,   //!< Not tagged - restyle() leaves the widget alone
         Frame,        //!< Window or dialog background
         Banner,       //!< Section banner
         Label,        //!< Form field caption
         Pushbutton,   //!< Push button
         Edit,         //!< Editable entry field, list or table
         ReadOnly,     //!< Read-only entry field
         Normal,       //!< Every other widget
         Lcd,          //!< LCD panel
         PlotFrame     //!< Plot frame and its tool bar
      };

      //! \brief Name of the dynamic property that carries the Role
      static const char* roleProperty() { return "usRole"; }

      //! \brief Name of the dynamic property that tags a section banner
      //! \note Distinct from roleProperty(): this one is a style sheet hook
      static const char* bannerProperty() { return "usBanner"; }

      //! \brief The palette a Role currently resolves to
      static QPalette paletteFor( Role );

      //! \brief Tag a widget with its palette family and apply that palette
      /*!  This is what the us_* widget factories call in place of a bare
           setPalette(), and it is what makes the widget follow a later
           light/dark switch.
      */
      static void    tag             ( QWidget*, Role );

      //! \brief Re-apply the current palettes to every tagged widget
      /*!  Called by apply().  Safe to call at any time; widgets that were
           never tagged, and colors a program set itself, are left alone.
      */
      static void    restyle         ( void );

      //! \brief The color scheme currently in effect
      static Scheme  scheme          ( void );

      //! \brief Convenience test for US_Theme::scheme() == Dark
      static bool    isDark          ( void );

      //! \brief The token table of the scheme currently in effect
      static US_ThemeTokens tokens   ( void );

      //! \brief The token table of a specific scheme
      static US_ThemeTokens tokens   ( Scheme );

      //! \brief User preference for the color scheme: "auto", "light", "dark"
      static QString schemeSetting    ( void );

      //! \brief Store the user preference for the color scheme
      static void set_schemeSetting   ( const QString& );

      //! \brief The color scheme the desktop environment asks for
      static Scheme  systemScheme    ( void );

      //! \brief The widget style UltraScan uses unless the user picks another
      static QString defaultStyle    ( void );

      //! \brief Corner radius of buttons, entry fields and frames, in pixels
      static int     radius          ( void ) { return 5; }

      //! \brief Corner radius of small elements (items, indicators), in pixels
      static int     smallRadius     ( void ) { return 3; }

      //! \brief Build the named style wrapped in US_Style
      //! \param name A QStyleFactory key; the default style is used if unknown
      //! \returns A new style, owned by the caller, or 0 if none can be made
      static QStyle* createStyle     ( const QString& name );

      //! \brief The application font built from the US_GuiSettings values
      static QFont   baseFont        ( void );

      //! \brief The application wide palette (menus, dialogs, tool tips, ...)
      static QPalette applicationPalette( void );

      //! \brief The application wide style sheet (shape only, no fixed colors)
      static QString styleSheet      ( void );

      //! \brief Apply style, palette, font and style sheet to the application
      //! \param force Re-apply even when nothing appears to have changed
      static void    apply           ( bool force = false );

      //! \brief Forget what was applied last, so the next apply() rebuilds all
      static void    invalidate      ( void );

      //! \brief Counter that changes whenever the stored colors are rewritten
      /*!  apply() folds this into the signature it compares, which is how a
           color change made in one UltraScan program reaches the others.
      */
      static int     serial          ( void );

      //! \brief Advance serial(), announcing that the stored colors changed
      static void    bumpSerial      ( void );
};

#endif
