#ifndef US_GUI_SETTINGS_H
#define US_GUI_SETTINGS_H

#include <QtGui>

class US_GuiSettings
{
  public:
    US_GuiSettings (){};
    ~US_GuiSettings(){};

    // Fonts
    static QString  fontFamily      ( void );
    static void     set_fontFamily  ( const QString& );
                                   
    static int      fontSize        ( void );
    static void     set_fontSize    ( int );
                                   
    // Palettes and colors         
    static QPalette labelColor      ( void );
    static void     set_labelColor  ( const QPalette& );
                                   
    static QPalette editColor       ( void );
    static void     set_editColor   ( const QPalette& );
                                   
    static QPalette frameColor      ( void );
    static void     set_frameColor  ( const QPalette& );
                                   
    static QPalette pushbColor      ( void );
    static void     set_pushbColor  ( const QPalette& );
                                   
    static QPalette normalColor     ( void );
    static void     set_normalColor ( const QPalette& );
                                   
    static QPalette lcdColor        ( void );
    static void     set_lcdColor    ( const QPalette& );
                                   
    static QPalette plotColor       ( void );
    static void     set_plotColor   ( const QPalette& );
                                   
    static QColor   plotCurve       ( void );
    static void     set_plotCurve   ( const QColor& );

    static QColor   plotCanvasBG    ( void );
    static void     set_plotCanvasBG( const QColor& );

    static QColor   plotMajGrid     ( void );
    static void     set_plotMajGrid ( const QColor& );

    static QColor   plotMinGrid     ( void );
    static void     set_plotMinGrid ( const QColor& );

    static QColor   plotPicker      ( void );
    static void     set_plotPicker  ( const QColor& );

    // Misc
    static int      plotMargin      ( void );
    static void     set_plotMargin  ( int );

    // Defaults
    static QPalette labelColorDefault ( void );
    static QPalette editColorDefault  ( void );
    static QPalette frameColorDefault ( void );
    static QPalette pushbColorDefault ( void );
    static QPalette normalColorDefault( void );
    static QPalette lcdColorDefault   ( void );
    static QPalette plotColorDefault  ( void );
};

#endif
