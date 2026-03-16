//! \file us_plot.h
#ifndef US_PLOT_H
#define US_PLOT_H

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_extern.h"

#include "qwt_plot.h"
#include "qwt_plot_grid.h"
#include "qwt_plot_picker.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_canvas.h"
#include "qwt_symbol.h"
#include "qwt_text.h"
#include <QMetaEnum>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>

struct US_GUI_EXTERN CurveDistance {
   QwtPlotCurve* curve = nullptr;
   double distance = std::numeric_limits<double>::min();
   int    closest_point_index = -1;
};

//! \brief A class to implement plot zooming with double-click support

class US_GUI_EXTERN US_Zoomer: public QwtPlotZoomer
{
   Q_OBJECT

   public:
      //! \param xAxis - The title of the x (bottom) axis
      //! \param yAxis - The title of the y (left) axis
      //! \param canvas - A pointer to the plot's canvas
      US_Zoomer( int xAxis, int yAxis, QwtPlotCanvas* canvas );
      static void toggle_zoom_events( QwtPlotZoomer* zoomer, bool active_zoom_button);
      virtual void begin() override;
   private:
      bool zoom_base_set {false};
};

//! \brief Customize plot widgets
class US_GUI_EXTERN US_PlotConfig;

//! \brief A class to configure the axis elements of the plot
class US_GUI_EXTERN US_PlotAxisConfig;

//! \brief A class to configure the x/y grid of a plot
class US_GUI_EXTERN US_PlotGridConfig;

//! \brief A class to configure the appearance of a curve
class US_GUI_EXTERN US_PlotCurveConfig;

class US_GUI_EXTERN US_DoubleClickEventFilter: public QObject
{
   Q_OBJECT
   public:
      explicit US_DoubleClickEventFilter( QwtPlot* plot, QObject* parent = nullptr, const double& threshold = 20.0 );
   signals:
      void curveDoubleClicked( QwtPlotCurve* curve );
      void axisDoubleClicked( int axis );
      void curvesDoubleClicked( QList< QwtPlotCurve* > );
      void axesDoubleClicked( QList< int > );
   protected:
      //! \brief Event filter for canvas double-click detection
      bool eventFilter( QObject* object, QEvent* event ) override;
   private:
      QwtPlot* plot;
      double threshold;
};

//! \brief A class that provides plot widgets with modern Qt/Qwt API
/*! \class US_Plot
  Provides a comprehensive API for plot configuration and interaction.

  Key features:
  - Virtual methods for customizing zoom, pan, and picker behavior
  - Signal-based architecture for easy extension
  - Double-click support for curves and axes
  - Default implementations that can be easily overridden

  Usage example:
  \code
  US_Plot* plot = new US_Plot("Title", "X Axis", "Y Axis");
  QwtPlot* qwtPlot = plot->qwtPlot();

  // Connect to signals for custom behavior
  connect(plot, &US_Plot::curveDoubleClicked, this, &MyClass::onCurveClicked);
  connect(plot, &US_Plot::axisDoubleClicked, this, &MyClass::onAxisClicked);
  \endcode
*/
class US_GUI_EXTERN US_Plot : public QHBoxLayout
{
   Q_OBJECT

   public:
      //! \param title   - The title of the plot
      //! \param x_axis  - The title of the x (bottom) axis
      //! \param y_axis  - The title of the y (left) axis
      //! \param cmEnab  - Flag to enable/show CMap button
      //! \param cmMatch - Curve title pattern to match for gradient curves
      //! \param cmName  - Color map name for default gradient (e.g., "rainbow")
      US_Plot( QwtPlot*& plot, const QString& title, const QString& x_axis, const QString& y_axis,
               bool cmEnab = false, const QString& cmMatch = QString(""),
               const QString& cmName = QString("") );

      //! \brief Access to the underlying QwtPlot
      QwtPlot* qwtPlot() const { return plot; }

      //! \brief Public method to return map colors list and count
      //! \param mcolors - Map colors reference for colors list return
      //! \returns Count of colors in color gradient list
      int map_colors( QList< QColor >& mcolors ) const;

      //! \brief Virtual factory methods for customization
      //! Override these to provide custom zoomer, panner, or picker implementations
      //! \returns Newly created zoomer instance (caller takes ownership)
      virtual QwtPlotZoomer* createZoomer();

      //! \returns Newly created panner instance (caller takes ownership)
      virtual QwtPlotPanner* createPanner();

      //! \returns Newly created picker instance (caller takes ownership)
      virtual QwtPlotPicker* createPicker();

      //! \brief Access to interaction components
      //! \returns Current zoomer instance (may be nullptr if zoom not enabled)
      QwtPlotZoomer* getZoomer() const { return zoomer; }

      //! \returns Current panner instance (may be nullptr if zoom not enabled)
      QwtPlotPanner* getPanner() const { return panner; }

      //! \returns Current picker instance (may be nullptr)
      QwtPlotPicker* getPicker() const { return picker; }

      //! \brief Enable or disable zoom mode
      //! \param enable - true to enable zoom, false to disable
      virtual void setZoomEnabled( bool enable );

      //! \brief Check if zoom mode is enabled
      //! \returns true if zoom is currently enabled
      bool isZoomEnabled() const { return zoomEnabled; }

      //! \brief Public access to toolbar buttons for customization
      QToolButton* btnZoom;

   public slots:
      //! \brief Toggle zoom mode
      void zoom( bool on );

      //! \brief Export and print functions
      void print() const;
      void svg() const;
      void png() const;
      void csv() const;

      //! \brief Open configuration dialog
      void config();

      //! \brief Open color map dialog
      void colorMap();

      //! \brief Replot the graph
      void replot();

      //! \brief Open axis configuration for specific axis
      //! \param axis - The axis to configure (QwtPlot::Axis enum)
      void configureAxis( int axis );

      //! \brief Open curve configuration for specific curve
      //! \param curve - The curve to configure
      void configureCurve( QwtPlotCurve* curve );

      QList< CurveDistance > findCurvesAtPosition( const QPoint& pos,
                                              const double& threshold = 20.0 ) const;
      CurveDistance findCurveAtPosition( const QPoint& pos, const double& threshold = 20.0 ) const;

   signals:
      //! \brief Signal emitted when zoom rectangle is selected
      //! \param rect - The bounding rectangle of the zoomed area
      void zoomedCorners( const QRectF& rect );

      //! \brief Signal emitted when a curve is double-clicked
      //! \param curve - The curve that was double-clicked
      void curveDoubleClicked( QwtPlotCurve* curve );

      //! \brief Signal emitted when an axis is double-clicked
      //! \param axis - The axis that was double-clicked
      void axisDoubleClicked( int axis );

      //! \brief Signal emitted when zoom mode is toggled
      //! \param enabled - true if zoom is now enabled
      void zoomModeChanged( bool enabled );

   protected:
      //! \brief Setup default zoom behavior
      //! Override to customize zoom setup
      virtual void setupZoom();

      //! \brief Cleanup zoom components
      //! Override to customize zoom cleanup
      virtual void cleanupZoom();

   private:
      void init( const QString& title, const QString& x_axis, const QString& y_axis,
                 bool cmEnab, const QString& cmMatch, const QString& cmName );

      void setupConnections();
      void setupCanvas();

      void configureCurves( const QList< QwtPlotCurve* >& curves );

      US_PlotConfig* configWidget;
      QwtPlot*       plot;

      QwtPlotZoomer* zoomer;
      QwtPlotPicker* picker;
      QwtPlotPanner* panner;

      bool           zoomEnabled;
      bool           cmapEnab;
      QString        cmapMatch;
      QString        cmfpath;
      QToolButton*   btnCMap;
      QVector<double> yLeftRange;
      QVector<double> yRightRange;

   private slots:
      void scale_yRight( const QRectF& rect ) const;
};

//! \brief A specialized push button class for US_Plot to automatically
//!        tie a specialized signal to a push button
class US_GUI_EXTERN US_PlotPushbutton : public QPushButton
{
   Q_OBJECT

   public:
      //! \param labelString -  Text in the button
      //! \param w           -  Parent widget
      //! \param index       -  Index value to pass in the signal when pushed
      US_PlotPushbutton( const QString& labelString, QWidget* w, int index);

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

class US_GUI_EXTERN US_PlotConfig : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param current_plot - The plot to be configured
      US_PlotConfig( QwtPlot* current_plot, QWidget* = nullptr, Qt::WindowFlags = Qt::Dialog );

      QColor global_canvas_color;

      Q_ENUM(QwtPlot::Axis);
  //QPalette global_canvas_palette;

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

  
      QComboBox*   cmbb_margin;
      QComboBox*   cmbb_legendPos;

      QListWidget* lw_curves;

      US_PlotAxisConfig*  axisWidget;
      US_PlotGridConfig*  gridWidget;
      US_PlotCurveConfig* curveWidget;

      void setLegendFontString( void ) const;
      QJsonObject getGridJson( void ) const;
      QJsonObject getAxisJson( int ) const;

      static QJsonObject getFontJson( const QFont& );
      void setTitleJson( const QJsonObject& ) const;
      static QFont jsonToFont( const QJsonObject& );
      static QMap<QString, bool> parseGridJson( const QJsonObject&, QPen*);
      void setGridJson( const QJsonObject& ) const;
      void setAxisJson( int, const QJsonObject& ) const;

   private slots:
      void updateTitleText  ( const QString& ) const;
      void updateTitleFont  ( void );
      void selectFrameColor ( void );
      void selectCanvasColor( void );
      void selectMargin     ( int  ) const;
      void selectLegendPos  ( int  ) const;
      void updateLegendFont ( void );
      void updateAxis       ( int  );
      void updateCurve      ( void );
      void updateGrid       ( void );
      void loadPlotProfile  ( void );
      void savePlotProfile  ( void );
      //void axisConfigFinish ( void );
      //void gridConfigFinish ( void );
      //void curveConfigFinish( void );
      //void closeEvent       ( QCloseEvent* );
};

class US_GUI_EXTERN US_PlotLabel;

//! \brief A window to customize plot curves

class US_GUI_EXTERN US_PlotCurveConfig : public US_WidgetsDialog
{
   Q_OBJECT

   friend class US_PlotLabel;

   public:
      US_PlotCurveConfig( QwtPlot*, const QStringList&, QWidget* = nullptr,
            Qt::WindowFlags = Qt::Dialog );

   //signals:
      //! \brief A signal to tell the parent that the window is closed
      //void curveConfigClosed( void );

   private:
      US_PlotConfig* plotConfigW;
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
      void updateSample             ( int  ) const;
      void selectCurveColor         ( void );
      void symbolStyleChanged       ( int  );
      void selectSymbolInteriorColor( void );
      void selectSymbolOutlineColor ( void );
      void apply                    ( void );
};

//! \brief  A window to customize plot labels
class US_GUI_EXTERN US_PlotLabel : public QWidget
{
   Q_OBJECT

   public:
      //! \brief Custom class to display curve configuration
      //! \param caller - Parent configuration window
      //! \param p      - Parent widget, generally can be the default
      //! \param f      - Window flags to be passed, normally the default
      US_PlotLabel( US_PlotCurveConfig* caller, QWidget* p = nullptr, Qt::WindowFlags f = Qt::Dialog );
      
   private:
      US_PlotCurveConfig* data;
      QLabel*             label;

   private slots:
      void paintEvent( QPaintEvent* );
};

//! \brief  A window to customize plot axes
class US_GUI_EXTERN US_PlotAxisConfig : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_PlotAxisConfig( int axis, QwtPlot*, QWidget* = nullptr, Qt::WindowFlags = Qt::Dialog );
      
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
      void apply               ( void ) const;
};

//! \brief A window to customize a plot's grid
class US_GUI_EXTERN US_PlotGridConfig : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      US_PlotGridConfig( QwtPlot*, QWidget* = nullptr, Qt::WindowFlags = Qt::Dialog );
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
      void apply           ( void ) const;
};

/*! \brief Customize plot picker characteristics and mouse events
    \param plot The plot to attach to

    This enhanced picker supports all standard mouse events plus double-click
    detection for both canvas and plot items (curves, axes).
*/
class US_GUI_EXTERN US_PlotPicker : public QwtPlotPicker
{
   Q_OBJECT

   public:
      US_PlotPicker( QwtPlot* );

   signals:
      //! \brief Signal mouse down (unmodified)
      void mouseDown    ( const QPointF& );
      //! \brief Signal mouse down (modified with control key)
      void cMouseDown   ( const QPointF& );
      //! \brief Signal mouse down (modified with control key, raw event)
      void cMouseDownRaw( QMouseEvent* );
      //! \brief Signal mouse down raw
      void mouseDownRaw( const QPointF&, QMouseEvent* );
      //! \brief Signal mouse up raw
      void mouseUpRaw  ( const QPointF&, QMouseEvent* );
      //! \brief Signal mouse up (unmodified)
      void mouseUp      ( const QPointF& );
      //! \brief Signal mouse up (modified with control key)
      void cMouseUp     ( const QPointF& );
      //! \brief Signal mouse drag (unmodified)
      void mouseDrag    ( const QPointF& );
      void mouseMoving ();
      //! \brief Signal mouse drag (modified with control key)
      void cMouseDrag   ( const QPointF& );
      //! \brief Signal emitted when canvas is double-clicked
      //! \param pos - Position in widget coordinates
      void canvasDoubleClicked( const QPoint& pos );

   protected:
      //! \brief Slot to handle mouse press event
      void widgetMousePressEvent  ( QMouseEvent* );
      //! \brief Slot to handle mouse release event
      void widgetMouseReleaseEvent( QMouseEvent* );
      //! \brief Slot to handle mouse move event
      void widgetMouseMoveEvent   ( QMouseEvent* );
      //! \brief Slot to handle mouse double-click event
      void widgetMouseDoubleClickEvent( QMouseEvent* );
};
#endif

