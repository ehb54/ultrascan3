#ifndef US_GUI_SETTINGS_H
#define US_GUI_SETTINGS_H

#include <QtGui>

class US_GuiSettings
{
  public:
    US_GuiSettings (){};
    ~US_GuiSettings(){};

    // Fonts
    static QString  fontFamily     ( void );
    static void     set_fontFamily ( const QString& );

    static int      fontSize       ( void );
    static void     set_fontSize   ( int );

    // Palettes
    static QPalette labelColor     ( void );
    static void     set_labelColor ( const QPalette& );

    static QPalette editColor      ( void );
    static void     set_editColor  ( const QPalette& );

    static QPalette frameColor     ( void );
    static void     set_frameColor ( const QPalette& );

    static QPalette pushbColor     ( void );
    static void     set_pushbColor ( const QPalette& );

    static QPalette normalColor    ( void );
    static void     set_normalColor( const QPalette& );

  private:
    static QPalette labelColorDefault ( void );
    static QPalette editColorDefault  ( void );
    static QPalette frameColorDefault ( void );
    static QPalette pushbColorDefault ( void );
    static QPalette normalColorDefault( void );
};

#endif
