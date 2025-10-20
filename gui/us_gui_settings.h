#ifndef US_GUI_SETTINGS_H
#define US_GUI_SETTINGS_H

#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#endif
#include "us_extern.h"

//! \brief Fetch and set Gui values via QSettings.  All functions are static.

class US_GUI_EXTERN US_GuiSettings
{
  public:

    //! \brief Null constructor.
    US_GuiSettings (){};
    //! \brief Null destructor.
    ~US_GuiSettings(){};

    //! \brief set colors
    static void set_GUI_Colors(QApplication* app);
    //! \brief clear child styles
    static void scrubChildStyleSheets(QWidget* root);

    //! \brief Get font family setting
    static QString  fontFamily      ( void );
    //! \brief Set font family setting
    static void     set_fontFamily  ( const QString& );
                                   
    //! \brief Get font size setting
    static int      fontSize        ( void );
    //! \brief Set font size setting
    static void     set_fontSize    ( int );
                                   
    //! \brief Get style setting
    static QString  guiStyle        ( void );
    //! \brief Set style setting
    static void     set_guiStyle    ( const QString& );

    // Palettes and colors         

    //! \brief Get current label palette
    static QPalette labelColor      ( void );
    //! \brief Set current label palette
    static void     set_labelColor  ( const QPalette& );
                                   
    //! \brief Get current edit palette
    static QPalette editColor       ( void );
    //! \brief Set current edit palette
    static void     set_editColor   ( const QPalette& );
                                   
    //! \brief Get current frame palette
    static QPalette frameColor      ( void );
    //! \brief Set current frame palette
    static void     set_frameColor  ( const QPalette& );
                                   
    //! \brief Get current pushbutton palette
    static QPalette pushbColor      ( void );
    //! \brief Set current pushbutton palette
    static void     set_pushbColor  ( const QPalette& );
                                   
    //! \brief Get current normal (other widgets) palette
    static QPalette normalColor     ( void );
    //! \brief Set current normal (other widgets) palette
    static void     set_normalColor ( const QPalette& );
                                   
    //! \brief Get current LCD palette
    static QPalette lcdColor        ( void );
    //! \brief Set current LCD palette
    static void     set_lcdColor    ( const QPalette& );
                                   
    //! \brief Get current plot palette
    static QPalette plotColor       ( void );
    //! \brief Set current plot palette
    static void     set_plotColor   ( const QPalette& );
                                   
    //! \brief Get current plot curve color
    static QColor   plotCurve       ( void );
    //! \brief Set current plot curve color
    static void     set_plotCurve   ( const QColor& );

    //! \brief Get current plot background color
    static QColor   plotCanvasBG    ( void );
    //! \brief Set current plot background color
    static void     set_plotCanvasBG( const QColor& );

    //! \brief Get current major gridline color
    static QColor   plotMajGrid     ( void );
    //! \brief Set current major gridline color
    static void     set_plotMajGrid ( const QColor& );

    //! \brief Get current minor gridline color
    static QColor   plotMinGrid     ( void );
    //! \brief Set current minor gridline color
    static void     set_plotMinGrid ( const QColor& );

    //! \brief Get current plot picker/crosshairs color
    static QColor   plotPicker      ( void );
    //! \brief Set current plot picker/crosshairs color
    static void     set_plotPicker  ( const QColor& );

    // Misc
    //! \brief Get current plot margin in pixels
    static int      plotMargin      ( void );
    //! \brief Set current plot margin in pixels
    static void     set_plotMargin  ( int );

    // Defaults

    //! \retval UltraScan default label color palette
    static QPalette labelColorDefault ( void );
    
    //! \retval UltraScan default edit color palette
    static QPalette editColorDefault  ( void );
    
    //! \retval UltraScan default frame color palette
    static QPalette frameColorDefault ( void );
    
    //! \retval UltraScan default pushbutton color palette
    static QPalette pushbColorDefault ( void );
    
    //! \retval UltraScan default normal (other widgets) color palette
    static QPalette normalColorDefault( void );
    
    //! \retval UltraScan default LCD color palette
    static QPalette lcdColorDefault   ( void );
    
    //! \retval UltraScan default plot color palette
    static QPalette plotColorDefault  ( void );
};

#endif
