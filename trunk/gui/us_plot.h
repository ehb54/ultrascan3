//! \file us_plot.h
#ifndef US_PLOT_H
#define US_PLOT_H

#include <QtGui>

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_curve.h"
#include "qwt_symbol.h"

#include "us_widgets.h"
#include "us_extern.h"


//! \brief A class to implement plot zooming

class US_Zoomer: public QwtPlotZoomer
{
   public:
      //! \param x_axis - The title of the x (bottom) axis
      //! \param y_axis - The title of the y (left) axis
      //! \param y_axis - A pointer to the plot's canvas
      US_Zoomer( int, int, QwtPlotCanvas* );
};

//! \brief Customize plot widgits

/*! \class US_Plot
  Provides functions to allow configuration of plot widgets
*/

class US_PlotConfig;
class US_PlotAxisConfig;
class US_PlotGridConfig;
class US_PlotCurveConfig;

//class US_EXTERN US_Plot : public QwtPlot
class US_EXTERN US_Plot : public QVBoxLayout
{
   Q_OBJECT

   public:
      //! \param title  - The title of the plot
      //! \param x_axis - The title of the x (bottom) axis
      //! \param y_axis - The title of the y (left) axis

      //US_Plot( const QString&, const QString&, const QString& );
      US_Plot( QwtPlot*& plot, const QString&, const QString&, const QString& );

      void quit( void );

   private:
      US_PlotConfig* configWidget;
      QwtPlot*       plot;

      QwtPlotZoomer* zoomer;
      QwtPlotPicker* picker;
      QwtPlotPanner* panner;

   private slots:
      void zoom              ( bool );
      void print             ( void );
      void svg               ( void );
      void config            ( void );
      void plotConfigFinished( void );
};

//! \brief A specialized push button class for US_Plot to automatically
//!        tie a specialed signal to a the push button
class US_PlotPushbutton : public QPushButton
{
   Q_OBJECT

   public:
      //! \param labelString -  Text in the button
      //! \param w           -  Parent widget
      //! \param index       -  Index value to pass in the signal when pushed
      US_PlotPushbutton( const QString&, QWidget*, int );

   signals:
      //! \brief A signal that passes the index value when the pushbutton was
      //!        created
      void US_PlotPbPushed( int );

   private:
      int pb_index;
   
   private slots:
      void us_plotClicked( void );
};

//! \brief A window to allow customization of plots initialized
//         via US_Plot

class US_PlotConfig : public US_Widgets
{
   Q_OBJECT

   public:
      //! \param current_plot - The plot to be configured
      US_PlotConfig( QwtPlot*, QWidget* = 0, Qt::WindowFlags = 0 );

   signals:
      //! \brief A signal to tell the parent that the window iis closed
      void plotConfigClosed( void );

   private:
      QwtPlot*     plot;

      QLineEdit*   le_titleText;
      QLineEdit*   le_titleFont;
      QLineEdit*   le_legendFont;

      QLabel*      lb_showFrameColor;
      QLabel*      lb_showCanvasColor;

      QComboBox*   cmbb_legendPos;

      QListWidget* lw_curves;

      US_PlotAxisConfig*  axisWidget;
      US_PlotGridConfig*  gridWidget;
      US_PlotCurveConfig* curveWidget;

      void setLegendFontString( void );

   private slots:
      void updateTitleText  ( const QString& );
      void updateTitleFont  ( void );
      void selectFrameColor ( void );
      void selectCanvasColor( void );
      void selectMargin     ( int  );
      void selectLegendPos  ( int  );
      void updateLegendFont ( void );
      void updateAxis       ( int  );
      void updateCurve      ( void );
      void updateGrid       ( void );
      void axisConfigFinish ( void );
      void gridConfigFinish ( void );
      void curveConfigFinish( void );
      void closeEvent       ( QCloseEvent* );
};

class US_PlotLabel;

//! \brief A window to customize plot curves

class US_PlotCurveConfig : public US_Widgets
{
   Q_OBJECT

   friend class US_PlotLabel;

   public:
      US_PlotCurveConfig( QwtPlot*, const QStringList&, QWidget* = 0, 
            Qt::WindowFlags = 0 );

   signals:
      //! \brief A signal to tell the parent that the window is closed
      void curveConfigClosed( void );

   private:
      QwtPlot*      plot;
                   
      QStringList   selectedItems;

      US_PlotLabel* lb_sample2;
      QLabel*       lb_showCurveColor;
      QLabel*       lb_showSymbolOutlineColor;
      QLabel*       lb_showSymbolInteriorColor;
                   
      QLineEdit*    le_curveTitle;
                   
      QSpinBox*     sb_curveWidth;
      QSpinBox*     sb_symbolWidth;
      QSpinBox*     sb_symbolHeight;
                   
      QComboBox*    cmbb_curveStyle;
      QComboBox*    cmbb_symbolStyle;

      QwtPlotCurve* firstSelectedCurve;

      QwtPlotCurve::CurveStyle  curveStyle;
      QwtSymbol::Style          symbolStyle;

   private slots:
      void closeEvent               ( QCloseEvent* );
      void curveStyleChanged        ( int  );
      void updateSample             ( int  );
      void selectCurveColor         ( void );
      void symbolStyleChanged       ( int  );
      void selectSymbolInteriorColor( void );
      void selectSymbolOutlineColor ( void );
      void apply                    ( void );
};

//! \brief  A window to customize plot labels
class US_PlotLabel : public QWidget
{
   Q_OBJECT

   public:
      //! \brief Custom class to display curve configuration
      //! \param caller - Parent configuration window
      //! \param p      - Parent widget, generally can be the default
      //! \param f      - Window flags to be passed, normally the default
      US_PlotLabel( US_PlotCurveConfig*, QWidget* = 0, Qt::WindowFlags = 0 );
      
   private:
      US_PlotCurveConfig* data;
      QLabel*             label;

   private slots:
      void paintEvent( QPaintEvent* );
};

//! \brief  A window to customize plot axes
class US_PlotAxisConfig : public US_Widgets
{
   Q_OBJECT

   public:
      US_PlotAxisConfig( int axis, QwtPlot*, QWidget* = 0, Qt::WindowFlags = 0 );
      
   signals:
      //! \brief A signal to ensure the parent knows the window is closed.
      void axisConfigClosed( void );

   private:
      QwtPlot*      plot;
      int           axis;
                   
      QLineEdit*    le_titleText;
      QLineEdit*    le_axisTitleFont;
      QLineEdit*    le_axisScaleFont;
      QLineEdit*    le_scaleFrom;
      QLineEdit*    le_scaleTo;
      QLineEdit*    le_scaleStep;
      QLineEdit*    le_reference;
                   
      QRadioButton* rb_linear;
      QRadioButton* rb_log;

      QLabel*       lb_showAxisTitleColor;
      QLabel*       lb_showScaleColor;
      QLabel*       lb_showTickColor;

      QCheckBox*    cb_present;
      QCheckBox*    cb_refValue;
      QCheckBox*    cb_symmetric;
      QCheckBox*    cb_floating;
      QCheckBox*    cb_inverted;
      QCheckBox*    cb_autoscale;

      QFont         axisTitleFont;
      QFont         axisScaleFont;

   private slots:
      void closeEvent          ( QCloseEvent* );
      void selectTitleFont     ( void );
      void selectAxisTitleColor( void );
      void selectScaleFont     ( void );
      void selectScaleColor    ( void );
      void selectTickColor     ( void );
      void apply               ( void );
};

//! \brief A window to customize a plot's grid
class US_PlotGridConfig : public US_Widgets
{
   Q_OBJECT

   public:
      US_PlotGridConfig( QwtPlot*, QWidget* = 0, Qt::WindowFlags = 0 );
      ~US_PlotGridConfig() {};

   signals:
      //! \brief A signal to tell the parent that the window has been closed
      void gridConfigClosed( void );

   private:
      QwtPlot*      plot;
      QwtPlotGrid*  grid;

      QCheckBox*    cb_enableX;
      QCheckBox*    cb_enableY;
      QCheckBox*    cb_enableXminor;
      QCheckBox*    cb_enableYminor;

      QLabel*       lb_showMajorColor;
      QLabel*       lb_showMinorColor;
      
      QComboBox*    cmbb_majorStyle;
      QComboBox*    cmbb_minorStyle;

      QSpinBox*     sb_majorWidth;
      QSpinBox*     sb_minorWidth;

   private slots:
      void selectMajorColor( void );
      void selectMinorColor( void );
      void apply           ( void );
};

class US_PlotPicker : public QwtPlotPicker
{
   Q_OBJECT

   public:
      US_PlotPicker( QwtPlot* );

   signals:
      void mouseDown( const QwtDoublePoint& );
      void mouseUp  ( const QwtDoublePoint& );

   protected:
      void widgetMousePressEvent  ( QMouseEvent* ); 
      void widgetMouseReleaseEvent( QMouseEvent* ); 
};
#endif

