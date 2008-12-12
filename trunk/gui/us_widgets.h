//! \file us_widgets.h
#ifndef US_WIDGETS_H
#define US_WIDGETS_H

#include <QtGui>

#include "qwt_counter.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_curve.h"

#include "us_global.h"

class US_Widgets : public QFrame
{
  public: 
    US_Widgets( QWidget* p = 0, Qt::WindowFlags f = 0 );
    ~US_Widgets( void );

    US_Global g;

    //! \param labelString - contents of label
    //! \param fontAdjust  - adjustment to default point size 
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}
    //! * Alignment is set to Qt::AlignVCenter | Qt::AlignLeft
    //! * Margin is set to 5
    //! * Palette is set to US_GuiSettings::labelColor()
    QLabel*      us_label     ( const QString&, int = 0,  int = QFont::Normal );

    //! Same as us_label except
    //! * different default font size (-1 point)
    //! * color set to US_GuiSettings::editColor()
    QLabel*      us_textlabel ( const QString&, int = -1, int = QFont::Normal );

    //! Same as us_label except  
    //! * different default font weight (Bold )
    //! * color set to US_GuiSettings::frameColor()
    //! * alignment set to Qt::AlignCenter
    //! * style set to QFrame::WinPanel | QFrame::Raised
    //! * MidLineWidth set to 2 for frame border
    QLabel*      us_banner    ( const QString&, int = 0,  int = QFont::Bold );

    //! \param labelString - text in pushbutton
    //! \param enabled
    //! \param fontAdjust -  adjustment to default font size
    //! * Standard Font family and size
    //! * color set to US_GuiSettings::pushbColor()
    //! * button is enabled
    //! * AutoDevault is set false
    QPushButton* us_pushbutton( const QString&, bool = true, int = 0 ); 
    
    //! * Font size is set to default - 1
    //! * Color is set to US_GuiSettings::normalColor()
    //! * Style is WinPanel | Sunken
    //! * AcceptRichText is true
    //! * ReadOnly is true
    QTextEdit*   us_textedit  ( void );

    //! \param test - starting text in line edit box
    //! \param fontAdjust -  adjustment to default point size
    //! * Color is set to US_GuiSettings::editColor()
    //! * ReadOnly is false
    QLineEdit*   us_lineedit  ( const QString& = 0, int = -1 );

    //! \param fontAdjust -  adjustment to default font size
    //! * Color is set to US_GuiSettings::editColor()
    QListWidget* us_listwidget ( int = 0 );

    //! \param text - Text to set with checkbox
    //! \param state - Qt::{Unchecked,PartiallyChecked,Checked}
    //! * Font weight is bold
    //! * Color is set to US_GuiSettings::normalColor()
    QCheckBox*   us_checkbox  ( const QString&, Qt::CheckState = Qt::Unchecked );

    //! \param text - Text to set with radiobutton
    //! \param state - Qt::{Unchecked,PartiallyChecked,Checked}
    //! * Color is set to US_GuiSettings::editColor()
    QRadioButton* us_radiobutton( const QString&, Qt::CheckState = Qt::Unchecked );

    //! \param low   - Lower bound of progress
    //! \param high  - Upper bound of progress
    //! \param value - Initial value between low and high
    //! * Color is set to US_GuiSettings::normalColor()
    //! * Font weight is bold
    QProgressBar* us_progressBar( int, int, int = 0 );

    //! * Color is set to US_GuiSettings::normalColor()
    QComboBox*    us_comboBox( void );

    //! Create an LCD style disply
    //! \param digits - Number of digits to display
    //! \param value  - Initial value to set
    //! * Color is set to US_GuiSettings::lcdColor()
    QLCDNumber*   us_lcd( int, int = 0 );
                  
    //! \param buttons - Number of buttons to use ( 1 to 3 )
    //! \param low     - Lower bound of progress
    //! \param high    - Upper bound of progress
    //! \param value   - Initial value to set
    //! * Color is set to US_GuiSettings::normalColor()
    QwtCounter*   us_counter( int, double, double, double = 0.0 );

    //! \param title  - Plot Title
    //! \param bottom - Bottom Axis Title
    //! \param left   - Left Axis Title
    //! * Color is set to US_GuiSettings::plotColor()
    //! * Convas background is set to US_GuiSettings::plotConvasBG()
    QwtPlot*      us_plot( const QString&, const QString& = QString(), 
                           const QString& = QString() );

    //! \param plot*   - Pointer to plot
    //! * Major Pen color is set to US_GuiSettings::plotMajGrid()
    //! * Minor Pen color is set to US_GuiSettings::plotMinGrid()
    QwtPlotGrid*  us_grid( QwtPlot* );

    //! \param plot*   - Pointer to plot
    //! * Curve color is set to US_GuiSettings::plotCurve()
    QwtPlotCurve* us_curve( QwtPlot* );

    //! \param plot*   - Pointer to plot
    //! * Rubber Band pen and Tracker pen color is set to 
    //! * US_GuiSettings::plotPicker()
    QwtPlotPicker* us_picker( QwtPlot* );

};
#endif

