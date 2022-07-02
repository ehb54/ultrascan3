//! \file us3i_widgets.h
#ifndef US_WIDGETS_H
#define US_WIDGETS_H

#include <QtCore>
#if QT_VERSION > 0x050000
#include <QtWidgets>
#include "qwt_picker_machine.h"
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#endif
#include "qwt_counter.h"
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_curve.h"

#include "us3i_global.h"
#include "us3i_extern.h"
#include "us3i_gui_settings.h"

/*! \brief Set up widgets the UltraScan way.

    This class is designed to be the parent class to almost all UltraScan
    windows.  It allows easy creation of widgets and applies the appropriate
    palette and font acording to the user's settings (or the UltraScan default).
    Depending on the widget, other parameters may be set.
*/
class US_EXTERN US3i_widgets : public QFrame
{
  public:
    //! The constructor connects to global memory and moves the screen to
    //! the point saved there. It adjusts the location point by QPoint( 30, 30).
    US3i_widgets( bool = true, QWidget* p = 0 );

    //! The destructor readjusts the location point in global memory to
    //! a value of QPoint( -30, -30 )
    ~US3i_widgets( void );

    //! Connect to global memory and make the values accessible.
    US3i_Global g;

    //! Very light gray palette for read-only line edits
    QPalette vlgray;

    //! \param labelString - contents of label
    //! \param fontAdjust  - adjustment to default point size
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}\n
    //! * Alignment is set to Qt::AlignVCenter | Qt::AlignLeft\n
    //! * Margin is set to 5\n
    //! * Palette is set to US3i_GuiSettings::labelColor()
    QLabel*      us_label     ( const QString&, int = 0,  int = QFont::Bold );

    //! Same as us_label except:\n
    //! * different default font size (-1 point)\n
    //! * color set to US3i_GuiSettings::editColor()
    QLabel*      us_textlabel ( const QString&, int = -1, int = QFont::Bold );

    //! Same as us_label except:\n
    //! * color set to US3i_GuiSettings::frameColor()\n
    //! * alignment set to Qt::AlignCenter\n
    //! * style set to QFrame::WinPanel | QFrame::Raised\n
    //! * MidLineWidth set to 2 for frame border
    QLabel*      us_banner    ( const QString&, int = 0,  int = QFont::Bold );

    //! \param labelString - text in pushbutton
    //! \param enabled
    //! \param fontAdjust -  adjustment to default font size\n
    //! * Standard Font family and size\n
    //! * color set to US3i_GuiSettings::pushbColor()\n
    //! * button is enabled\n
    //! * AutoDefault is set false
    QPushButton* us_pushbutton( const QString&, bool = true, int = 0 );

    //! * Font size is set to default - 1\n
    //! * Color is set to US3i_GuiSettings::normalColor()\n
    //! * Style is WinPanel | Sunken\n
    //! * AcceptRichText is true\n
    //! * ReadOnly is true
    QTextEdit*   us_textedit  ( void );

    //! \param text - starting text in line edit box
    //! \param fontAdjust -  adjustment to default point size\n
    //! \param readonly -  flag if read-only, default uses editColor()
    QLineEdit*   us_lineedit  ( const QString& = 0, int = -1, bool = false );

    //! \param le - line edit for which to set ReadOnly and color
    //! \param readonly -  flag if read-only, default uses editColor()
    void         us_setReadOnly( QLineEdit*, bool = true );

    //! \param te - text edit for which to set ReadOnly and color
    //! \param readonly -  flag if read-only, default uses editColor()
    void         us_setReadOnly( QTextEdit*, bool = true );

    //! \param fontAdjust -  adjustment to default font size\n
    //! * Color is set to US3i_GuiSettings::editColor()
    QListWidget* us_listwidget ( int = 0 );

    //! \param text - Text to set with checkbox
    //! \param state - Initial check state\n
    //! \param cb - reference to a pointer of the checkbox\n
    //! * Font weight is bold\n
    //! * Color is set to US3i_GuiSettings::normalColor()
    QGridLayout* us_checkbox  ( const QString&, QCheckBox*&, bool = false );

    //! Create a radiobutton with a specifice palette (normal) and
    //! indentation of the button
    //! \param text - Text to set with radiobutton
    //! \param rb - pointer to the radio button
    //! \param state - Checked ( true or false )
    //! * Color is set to US3i_GuiSettings::editColor()
    //QRadioButton* us_radiobutton( const QString&, bool = false );
    QGridLayout* us_radiobutton(
          const QString& text, QRadioButton*& rb, bool state = false );
    //! \param low   - Lower bound of progress
    //! \param high  - Upper bound of progress
    //! \param value - Initial value between low and high\n
    //! * Color is set to US3i_GuiSettings::normalColor()\n
    //! * Font weight is bold
    QProgressBar* us_progressBar( int, int, int = 0 );

    //! * Color is set to US3i_GuiSettings::normalColor()
    QComboBox*    us_comboBox( void );

    //! Create an LCD style disply
    //! \param digits - Number of digits to display
    //! \param value  - Initial value to set\n
    //! * Color is set to US3i_GuiSettings::lcdColor()
    QLCDNumber*   us_lcd( int, int = 0 );

    //! \param buttons - Number of buttons to use ( 1 to 3 )
    //! \param low     - Lower bound of progress
    //! \param high    - Upper bound of progress
    //! \param value   - Initial value to set\n
    //! * Color is set to US3i_GuiSettings::normalColor()
    QwtCounter*   us_counter( int, double, double, double = 0.0 );

    //! \param title  - Plot Title
    //! \param x_axis - Bottom Axis Title
    //! \param y_axis - Left Axis Title\n
    //! * Color is set to US3i_GuiSettings::plotColor()\n
    //! * Convas background is set to US3i_GuiSettings::plotConvasBG()
    QwtPlot*      us3i_plot( const QString&, const QString& = QString(),
                           const QString& = QString() );

    //! \param plot*   - Pointer to plot\n
    //! * Major Pen color is set to US3i_GuiSettings::plotMajGrid()\n
    //! * Minor Pen color is set to US3i_GuiSettings::plotMinGrid()
    QwtPlotGrid*  us_grid( QwtPlot* );

    //! \param plot*   - Pointer to plot\n
    //! \param title   - Title of the curve\n
    //! * Curve color is set to US3i_GuiSettings::plotCurve()
    QwtPlotCurve* us_curve( QwtPlot*, const QString& );

    //! \param plot*   - Pointer to plot\n
    //! * Rubber Band pen and Tracker pen color is set to\n
    //! * US3i_GuiSettings::plotPicker()
    QwtPlotPicker* us_picker( QwtPlot* );

    //! * returns a good match for a fixed font
    static QFont fixedFont( void );

    //! \param fontAdjust -  adjustment to default font size\n
    //! \param weight      - QFont::{Light,Normal,DemiBold,Bold,Black}\n
    //! * Color is set to US3i_GuiSettings::normalColor()
    QTabWidget* us_tabwidget( int = 0,
                              int = QFont::Bold );

    //! \param tedt       - Pointer to QTimeEdit
    //! \param fontAdjust - Adjustment to default font size
    //! \param sbox       - Optional pointer to days QSpinBox
    //! * Color is set to US3i_GuiSettings::normalColor()
    QHBoxLayout* us_timeedit( QTimeEdit*&, const int = 0,
                              QSpinBox** = (QSpinBox**)NULL );

    //! \param fontAdjust - Adjustment to default font size
    //! \param dd         - Optional pointer to days QSpinBox pointer
    //! \param hh         - Optional pointer to hrs  QSpinBox pointer
    //! \param mm         - Optional pointer to min  QSpinBox pointer
    //! \param ss         - Optional pointer to sec  QSpinBox pointer
    QHBoxLayout* us_ddhhmmss( const int = 0,
                              QSpinBox** = (QSpinBox**)NULL,
                              QSpinBox** = (QSpinBox**)NULL,
                              QSpinBox** = (QSpinBox**)NULL,
                              QSpinBox** = (QSpinBox**)NULL );

    //! \param fontAdjust -  adjustment to default font size\n
    //! * Color is set to US3i_GuiSettings::normalEdit()
    QSpinBox* us_spinbox( int = 0 );

    //! \brief Write a plot file (PNG or SVG+PNG) from a plot
    //! \param fname Full path name of the plot file to create
    //! \param plot  Pointer to plot
    void write_plot( const QString&, const QwtPlot* );

    //! \brief Clean install and work ./etc directories
    //! \param report Optional flag to report any file mods
    //! \return       Number of install-etc files modified
    int  clean_etc_dir( bool = false );
};

//! \brief Set a custom list widget that can override the default size

class US_EXTERN US_ListWidget : public QListWidget
{
   Q_OBJECT

   public:
      US_ListWidget()
      {
         setAutoFillBackground( true );
         setPalette( US3i_GuiSettings::editColor() );
         setFont   ( QFont( US3i_GuiSettings::fontFamily(),
                            US3i_GuiSettings::fontSize  () ) );

         setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum );
      };

      //!  A placeholder to specify the desired size of the widget.
      //!  Currently it returns an invalid size for the default rendering.
      QSize sizeHint() const
      {
         return QSize( -1, -1 );  // Allow default in the class
      };

      //!  An override of the base function to allow passing a custom
      //!  signal for the parent widget.
      void mousePressEvent( QMouseEvent* );

   signals:
      //!  A signal than can be used when the right mouse button is clicked
      //!  in the list widget.
      void rightClick( void );

};

//! Create a Lyaout with radio buttons for selection of disk or DB selection

class US_EXTERN US_Disk_DB_Controls : public QHBoxLayout
{
   Q_OBJECT

   public:
      //! Define a type of data access
      enum type { Default, DB, Disk };

      //! \brief Build a widget with radio buttons for selection of disk or
      //!        database access
      //! \param state Indicates the button to be initially set.  The
      //               Default set gets the setting from US3i_Settings
      US_Disk_DB_Controls( int= Default );

      //! \brief A function to determine if the DB radio buttone is set
      //! \returns True if the DB radio button is set
      bool db( void );

      //! \brief A utility routine to set the disk radio button
      void set_disk( void );

      //! \brief A utility routine to set the DB radio button
      void set_db  ( void );

   signals:
      //!  A signal to indicate that the current selection has changed
      void changed( bool );

   private:
      QRadioButton* rb_disk;
      QRadioButton* rb_db;

      QGridLayout* us_radiobutton(
            const QString& text, QRadioButton*& rb, bool state = false );

   public slots:
      void rb_changed( bool );
};

#endif
