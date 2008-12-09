#ifndef US_COLOR_H
#define US_COLOR_H

#include <QtGui>
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_picker.h"
#include "qwt_counter.h"

#include "us_widgets.h"
#include "us_help.h"

class US_Color : public US_Widgets
{
  Q_OBJECT
  
  public:
    US_Color( QWidget* = 0, Qt::WindowFlags = 0 );
    ~US_Color() {};

  private:
    US_Help showhelp;

    // Banners
    QLabel* lbl_background;
    QLabel* lbl_margin;
    QLabel* lbl_example;
    QLabel* lbl_banner;
    QLabel* lbl_progress;
    QLabel* lbl_lcd;
    QLabel* lbl_counter;
    QLabel* lbl_assign;

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
    QProgressBar*  progress;
    QwtCounter*    cnt;

    // LCD
    QLCDNumber*    lcd;
    
    // Plot Widgets
    QwtPlot*       plot;
    QwtPlotGrid*   grid;
    QwtPlotCurve*  curve;
    QwtPlotPicker* pick;

    struct
    {
      int      plotMargin;
      
      QColor   plotCurve;
      QColor   plotBg;
      QColor   plotMajorGrid;
      QColor   plotMinorGrid;
      QColor   plotPicker;

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
    void selMargin      ( int    );
    void selectedElement( int    );
    void selected_scheme( void   );
    void save_as        ( void   );

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
