//! \file us3i_color.h
#ifndef US3I_COLOR_H
#define US3I_COLOR_H

#include "us3i_widgets.h"
// #include "us_help.h"
#include "us3i_plot.h"

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "qwt_counter.h"

//! \brief Allow the user to set or change the colors used in UltraScan

//! A class to allow the user to set or change the look of UltraScan 
//! windows via maipulation of the color components of widget palettes.
//! It also allows the control of plot margins.
class US3i_Color : public US3i_widgets
{
  Q_OBJECT
  
  public:
    //! Construct the window to manage the user's color choices.
    //! The settings are saved for each user via \ref US_Settings.
    US3i_Color( QWidget* = 0 );
    //! A destructor to clean up 
    //~US3i_Color();

  private:
    // US_Help showhelp;

    // Banners
    QLabel* lbl_background;
    QLabel* lbl_margin;
    QLabel* lbl_example;
    QLabel* lbl_banner;
    QLabel* lbl_progress;
    QLabel* lbl_lcd;
    QLabel* lbl_counter;
    QLabel* lbl_assign;
    QLabel* lbl_style;

    // Labels

    QLabel* lbl_text;
    QLabel* lbl_choices;
    QLabel* lbl_select;
    QLabel* lbl_color1;
    QLabel* lbl_color2;
    QLabel* lbl_color3;
    QLabel* lbl_color4;
    QLabel* lbl_color5;
    QLabel* lbl_color6;
    QLabel* lbl_select_scheme;
    QLabel* lbl_select_element;

    // Color Fields
    QLabel* color_field1;
    QLabel* color_field2;
    QLabel* color_field3;
    QLabel* color_field4;
    QLabel* color_field5;
    QLabel* color_field6;
    
    // Pushbuttons

    QPushButton* pb_color1;
    QPushButton* pb_color2;
    QPushButton* pb_color3;
    QPushButton* pb_color4;
    QPushButton* pb_color5;
    QPushButton* pb_color6;
                 
    QPushButton* pb_normal;
    QPushButton* pb_active;
    QPushButton* pb_disabled;
                 
    QPushButton* pb_save_as;
    QPushButton* pb_apply;
    QPushButton* pb_reset;
    QPushButton* pb_delete;
    QPushButton* pb_help;
    QPushButton* pb_quit;
    

    // Edit Boxes
    QLineEdit*     le_choice;
    QLineEdit*     le_edit;
    QLineEdit*     le_save_as;
    QListWidget*   schemes;
    QListWidget*   elements;
    
    // Other Widgets
    QComboBox*     cmbb_margin;
    QComboBox*     cmbb_style;
    QProgressBar*  progress;
    QwtCounter*    cnt;

    // LCD
    QLCDNumber*    lcd;
    
    // Plot Widgets
    mQwtPlot*      plot;
    QwtPlotGrid*   grid;
    QwtPlotCurve*  curve;
    QwtPlotPicker* pick;

    US_Plot*       us_plot;

    struct
    {
      int      plotMargin;
      
      QColor   plotCurve;
      QColor   plotBg;
      QColor   plotMajorGrid;
      QColor   plotMinorGrid;
      QColor   plotPicker;

      QString  guiStyle;

      QPalette  frameColor;
      QPalette  pushbColor;
      QPalette  labelColor;
      QPalette   editColor;
      QPalette normalColor;
      QPalette    lcdColor;
      QPalette   plotColor;
    } current;

    enum { FRAME, NORMAL_PB, DISABLED_PB, PLOT_FRAME, PLOT_CANVAS, LABELS, 
           OTHER_WIDGETS, LCD, EDIT_BOXES };

    void getCurrentSettings( void );
    void updateScreen      ( void );
    void updateSchemeList  ( const QString& = QString() );

    void resetFrames       ( void );
    void resetButtons      ( void );
    void resetLabels       ( void );
    void resetWidgets      ( void );
    void resetEditBoxes    ( void );

  private slots:
    void updateWidgets  ( double );
    void selected_scheme( void   );
    void save_as        ( void   );
    void selMargin      ( int    );
    void selectedElement( int    );
    void selectStyle    ( const QString& );

    void apply          ( void   );  // set as current
    void reset          ( void   );
    void delete_scheme  ( void   );
    void help           ( void   );

    void pick_color1    ( void   );
    void pick_color2    ( void   );
    void pick_color3    ( void   );
    void pick_color4    ( void   );
    void pick_color5    ( void   );
    void pick_color6    ( void   );
};

#endif
