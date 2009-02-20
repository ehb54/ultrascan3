//! \file us_widgets_dialog.h
#ifndef US_WIDGETS_DIALOG_H
#define US_WIDGETS_DIALOG_H

#include <QtGui>

#include "qwt_counter.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_curve.h"

#include "us_extern.h"

/*! \brief Set up widgets the UltraScan way.
 
    This class is designed to be the parent class to almost all UltraScan
    windows.  It allows easy creation of widgets and applies the appropriate
    palette and font acording to the user's settings (or the UltraScan default).
    Depending on the widget, other parameters may be set.
*/
class US_EXTERN US_WidgetsDialog : public QDialog
{
  public: 
    US_WidgetsDialog( QWidget*, Qt::WindowFlags );
    ~US_WidgetsDialog() {};
    
    //! \param labelString - contents of label
    //! \param fontAdjust  - adjustment to default point size 
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}\n
    //! * Alignment is set to Qt::AlignVCenter | Qt::AlignLeft\n
    //! * Margin is set to 5\n
    //! * Palette is set to US_GuiSettings::labelColor()
    QLabel*      us_label     ( const QString&, int = 0,  int = QFont::Normal );

    //! Same as us_label except:\n
    //! * different default font size (-1 point)\n
    //! * color set to US_GuiSettings::editColor()
    QLabel*      us_textlabel ( const QString&, int = -1, int = QFont::Normal );

    //! Same as us_label except:\n
    //! * different default font weight (Bold )\n
    //! * color set to US_GuiSettings::frameColor()\n
    //! * alignment set to Qt::AlignCenter\n
    //! * style set to QFrame::WinPanel | QFrame::Raised\n
    //! * MidLineWidth set to 2 for frame border
    QLabel*      us_banner    ( const QString&, int = 0,  int = QFont::Bold );

    //! \param labelString - text in pushbutton
    //! \param enabled
    //! \param fontAdjust -  adjustment to default font size\n
    //! * Standard Font family and size\n
    //! * color set to US_GuiSettings::pushbColor()\n
    //! * button is enabled\n
    //! * AutoDefault is set false
    QPushButton* us_pushbutton( const QString&, bool = true, int = 0 ); 
    
    //! * Font size is set to default - 1\n
    //! * Color is set to US_GuiSettings::normalColor()\n
    //! * Style is WinPanel | Sunken\n
    //! * AcceptRichText is true\n
    //! * ReadOnly is true
    QTextEdit*   us_textedit  ( void );

    //! \param text - starting text in line edit box
    //! \param fontAdjust -  adjustment to default point size\n
    //! * Color is set to US_GuiSettings::editColor()\n
    //! * ReadOnly is false
    QLineEdit*   us_lineedit  ( const QString& = 0, int = -1 );

    //! \param fontAdjust -  adjustment to default font size\n
    //! * Color is set to US_GuiSettings::editColor()
    QListWidget* us_listwidget ( int = 0 );

    //! \param text - Text to set with checkbox
    //! \param state - Qt::{Unchecked,PartiallyChecked,Checked}\n
    //! * Font weight is bold\n
    //! * Color is set to US_GuiSettings::normalColor()
    QCheckBox*   us_checkbox  ( const QString&, Qt::CheckState = Qt::Unchecked );

    //! \param text - Text to set with radiobutton
    //! \param state - Checked ( true or false )\n
    //! * Color is set to US_GuiSettings::editColor()
    QRadioButton* us_radiobutton( const QString&, bool = false );

    //! \param low   - Lower bound of progress
    //! \param high  - Upper bound of progress
    //! \param value - Initial value between low and high\n
    //! * Color is set to US_GuiSettings::normalColor()\n
    //! * Font weight is bold
    QProgressBar* us_progressBar( int, int, int = 0 );

    //! * Color is set to US_GuiSettings::normalColor()
    QComboBox*    us_comboBox( void );

    //! Create an LCD style disply
    //! \param digits - Number of digits to display
    //! \param value  - Initial value to set\n
    //! * Color is set to US_GuiSettings::lcdColor()
    QLCDNumber*   us_lcd( int, int = 0 );
                  
    //! \param buttons - Number of buttons to use ( 1 to 3 )
    //! \param low     - Lower bound of progress
    //! \param high    - Upper bound of progress
    //! \param value   - Initial value to set\n
    //! * Color is set to US_GuiSettings::normalColor()
    QwtCounter*   us_counter( int, double, double, double = 0.0 );

    //! \param title  - Plot Title
    //! \param x_axis - Bottom Axis Title
    //! \param y_axis - Left Axis Title\n
    //! * Color is set to US_GuiSettings::plotColor()\n
    //! * Convas background is set to US_GuiSettings::plotConvasBG()
    QwtPlot*      us_plot( const QString&, const QString& = QString(), 
                           const QString& = QString() );

    //! \param plot*   - Pointer to plot\n
    //! * Major Pen color is set to US_GuiSettings::plotMajGrid()\n
    //! * Minor Pen color is set to US_GuiSettings::plotMinGrid()
    QwtPlotGrid*  us_grid( QwtPlot* );

    //! \param plot*   - Pointer to plot\n
    //! \param title   - Title of the curve\n
    //! * Curve color is set to US_GuiSettings::plotCurve()
    QwtPlotCurve* us_curve( QwtPlot*, const QString& );

    //! \param plot*   - Pointer to plot\n
    //! * Rubber Band pen and Tracker pen color is set to\n
    //! * US_GuiSettings::plotPicker()
    QwtPlotPicker* us_picker( QwtPlot* );
};
#endif
