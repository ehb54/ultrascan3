//! \file us3i_plot.h
#ifndef US_PLOT_H
#define US_PLOT_H

#include "us3i_widgets_dialog.h"
#include "us3i_extern.h"

#if QT_VERSION > 0x050000
#include "qwt_compat.h"
#endif
#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_canvas.h"
#include "qwt_symbol.h"
#include "qwt/scrollbar.h"
#include "qwt/scrollzoomer.h"
#include "us_mqt.h"


//! \brief A class to implement plot zooming

class US_Zoomer: public QwtPlotZoomer
{
   public:
      //! \param xAxis - The title of the x (bottom) axis
      //! \param yAxis - The title of the y (left) axis
      //! \param canvas - A pointer to the plot's canvas
      US_Zoomer( int, int, QwtPlotCanvas* );
};

//! \brief Customize plot widgets
class US_PlotConfig;

//! \brief A class to configure the axis elements of the plot
class US_PlotAxisConfig;

//! \brief A class to configure the x/y grid of a plot
class US_PlotGridConfig;

//! \brief A class to configure the appearance of a curve
class US_PlotCurveConfig;

//! \brief A class that provides plot widgets
/*! \class US_Plot
  Provides functions to allow configuration of plot widgets
*/
class US_EXTERN US_Plot : public QHBoxLayout
{
   Q_OBJECT

   public:
      //! \param plot   - The plot component created for the caller
      //! \param title  - The title of the plot
      //! \param x_axis - The title of the x (bottom) axis
      //! \param y_axis - The title of the y (left) axis

      US_Plot( mQwtPlot*& plot, const QString&, const QString&, const QString&, QWidget * parent = 0 );

      //! Make access to the zoom button public
      QToolButton* btnZoom;

      enum plot_rescale_mode {
         MODE_NO_SCALE,
         MODE_RESID,
         MODE_IQ,
         MODE_GAUSSIAN,
      };

      plot_rescale_mode rescale_mode;
      ScrollZoomer * scroll_zoomer;

   signals:
      //! \brief A signal that provides the bounding rectangle of a zoomed area
#if QT_VERSION < 0x050000
      void zoomedCorners( QwtDoubleRect );
#else
      void zoomedCorners( QRectF        );
#endif

   private:
      US_PlotConfig* configWidget;
      mQwtPlot*       plot;

      QwtPlotZoomer* zoomer;
      QwtPlotPicker* picker;
      QwtPlotPanner* panner;

   public slots:
      void zoom  ( bool );
      void print ( void );
      void svg   ( void );
      void png   ( void );
      void config( void );

      void scaleDivChangedXSlot();
      void scaleDivChangedYSlot();
      void scaleDivChangedSlot();
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
      void us3i_plotClicked( void );
};

//! \brief A window to allow customization of plots initialized
//         via US_Plot

class US_PlotConfig : public US3i_widgetsDialog
{
   Q_OBJECT

   public:
      //! \param current_plot - The plot to be configured
      US_PlotConfig( QwtPlot*, QWidget* = 0 );

   //signals:
      //! \brief A signal to tell the parent that the window is closed
      //void plotConfigClosed( void );

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
      //void axisConfigFinish ( void );
      //void gridConfigFinish ( void );
      //void curveConfigFinish( void );
      //void closeEvent       ( QCloseEvent* );
};

class US_PlotLabel;

//! \brief A window to customize plot curves

class US_PlotCurveConfig : public US3i_widgetsDialog
{
   Q_OBJECT

   friend class US_PlotLabel;

   public:
     US_PlotCurveConfig( QwtPlot*, const QStringList&, QWidget* = 0 );

   //signals:
      //! \brief A signal to tell the parent that the window is closed
      //void curveConfigClosed( void );

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
      //void closeEvent               ( QCloseEvent* );
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
      US_PlotLabel( US_PlotCurveConfig*, QWidget* = 0 );
      
   private:
      US_PlotCurveConfig* data;
      QLabel*             label;

   private slots:
      void paintEvent( QPaintEvent* );
};

//! \brief  A window to customize plot axes
class US_PlotAxisConfig : public US3i_widgetsDialog
{
   Q_OBJECT

   public:
     US_PlotAxisConfig( int axis, QwtPlot*, QWidget* = 0 );
      
   //signals:
      //! \brief A signal to ensure the parent knows the window is closed.
      //void axisConfigClosed( void );

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
      //void closeEvent          ( QCloseEvent* );
      void selectTitleFont     ( void );
      void selectAxisTitleColor( void );
      void selectScaleFont     ( void );
      void selectScaleColor    ( void );
      void selectTickColor     ( void );
      void apply               ( void );
};

//! \brief A window to customize a plot's grid
class US_PlotGridConfig : public US3i_widgetsDialog
{
   Q_OBJECT

   public:
     US_PlotGridConfig( QwtPlot*, QWidget* = 0 );
      ~US_PlotGridConfig() {};

   //signals:
      //! \brief A signal to tell the parent that the window has been closed
      //void gridConfigClosed( void );

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

/*! \brief Customize plot picker characteristics and mouse events
    \param plot The plot to attach to
*/
class US_EXTERN US_PlotPicker : public QwtPlotPicker
{
   Q_OBJECT

   public:
      US_PlotPicker( QwtPlot* );

   signals:
      //! \brief Signal mouse down (unmodified)
      void mouseDown    ( const QwtDoublePoint& );
      //! \brief Signal mouse down (modified with control key)
      void cMouseDown   ( const QwtDoublePoint& );
      //! \brief Signal mouse down (modified with control key, raw event)
      void cMouseDownRaw( QMouseEvent* );
      //! \brief Signal mouse up (unmodified)
      void mouseUp      ( const QwtDoublePoint& );
      //! \brief Signal mouse up (modified with control key)
      void cMouseUp     ( const QwtDoublePoint& );
      //! \brief Signal mouse drag (unmodified)
      void mouseDrag    ( const QwtDoublePoint& );
      //! \brief Signal mouse drag (modified with control key)
      void cMouseDrag   ( const QwtDoublePoint& );

   protected:
      //! \brief Slot to handle mouse press event
      void widgetMousePressEvent  ( QMouseEvent* ); 
      //! \brief Slot to handle mouse release event
      void widgetMouseReleaseEvent( QMouseEvent* ); 
      //! \brief Slot to handle mouse move event
      void widgetMouseMoveEvent   ( QMouseEvent* ); 
};

class US_EXTERN US_PlotChoices : public QDialog
{
   Q_OBJECT

   public:
      US_PlotChoices( US_Plot *usp, QWidget * p = 0 );

   private:
      US_Plot *usp;

   private slots:
      void config();
      void svg();
      void png();
};

#include <vector>

class US_Plot_Colors {
 public:
   US_Plot_Colors( const QColor & background_color ); // background_color to try and avoid low contrast
   QColor color( size_t i );
   void rotate();

 private:
   void                   push_back_color_if_ok( QColor bg, QColor set );
   std::vector < QColor > plot_colors;
};

#endif

