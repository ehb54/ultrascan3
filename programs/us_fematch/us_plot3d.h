//! \file us_plot3d.h
#ifndef US_PLOT3D_H
#define US_PLOT3D_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_plot.h"
#include "us_resplot.h"
#include "us_model.h"
#include "us_help.h"
#include "qwt3d_surfaceplot.h"
#include "qwt3d_function.h"
#include "qwt3d_plot.h"

//! \brief A class to provide a window for a qwtplot3d surface plot

class US_EXTERN US_Plot3D : public QMainWindow
{
   Q_OBJECT

   public:
      //! \param parent  A pointer to the parent widget of this one
      //! \param modl    A pointer to the model whose data is to be plotted
      US_Plot3D( QWidget*, US_Model* );

      //! \brief Public function to set 3 coordinate type flags
      void setTypes     ( int, int, int );
      //! \brief Public function to set axis titles
      void setAxisTitles( QString, QString, QString );
      //! \brief Public function to set X and Y dimensions
      void setDimensions( int, int );
      //! \brief Public function to set 3 sets of ranges
      //! \param xmin    X minimum value
      void setRanges    ( double, double, double, double, double, double );
      //! \brief Public function to set plot control parameters
      void setParameters( double, double, double, double, double );
      //! \brief Public function to (re)calculate Z values at fixed increments
      void calculateData( QVector< QVector< double > >& );
      //! \brief Public function to replot the 3D data
      void replot       ( void );

   private:
      QWidget*      centralWidget;
      QGridLayout*  mainLayout;
      QVBoxLayout*  rightLayout;
      QHBoxLayout*  bottomLayout;
      QGridLayout*  rsliderLayout;
      QGridLayout*  bsliderLayout;

      QSlider*      poffsSlider;
      QSlider*      resolSlider;
      QSlider*      normqSlider;
      QSlider*      normlSlider;

      QAction*      openAct;
      QAction*      openffAct;
      QAction*      movieAct;
      QAction*      saveAct;
      QAction*      boxAct;
      QAction*      fdmeshAct;
      QAction*      fldataAct;
      QAction*      flemptAct;
      QAction*      flisoAct;
      QAction*      flmeshAct;
      QAction*      frameAct;
      QAction*      gridbAct;
      QAction*      gridcAct;
      QAction*      gridfAct;
      QAction*      gridfrAct;
      QAction*      gridAct;
      QAction*      gridrAct;
      QAction*      gridlAct;
      QAction*      hiddlnAct;
      QAction*      iconAct;
      QAction*      nodataAct;
      QAction*      noneAct;
      QAction*      normsAct;
      QAction*      polygnAct;
      QAction*      qwtpltAct;
      QAction*      scattdAct;
      QAction*      wirefrAct;
      QAction*      exitAct;
      QAction*      coaxisAct;
      QAction*      cobackAct;
      QAction*      comeshAct;
      QAction*      conbrsAct;
      QAction*      colablAct;
      QAction*      cocaptAct;
      QAction*      codataAct;
      QAction*      corsetAct;
      QAction*      fnscalAct;
      QAction*      fnaxisAct;
      QAction*      fncaptAct;
      QAction*      fnrsetAct;

      QActionGroup* frameGroup;
      QActionGroup* gridGroup;
      QActionGroup* dataGroup;
      QActionGroup* floorGroup;

      QMenu*        fileMenu;
      QMenu*        colorMenu;
      QMenu*        fontMenu;

      QToolBar*     fileToolBar;

      QComboBox*    cb_ifmt;

      QPushButton*  pb_std;
      QPushButton*  pb_light;

      QCheckBox*    ck_light;
      QCheckBox*    ck_ortho;
      QCheckBox*    ck_legend;
      QCheckBox*    ck_autosc;
      QCheckBox*    ck_mouse;
      QCheckBox*    ck_shade;

      US_Plot*      plotLayout1;

      bool          have_ed;
      bool          skip_plot;

      int           nrows;
      int           ncols;
      int           typex;
      int           typey;
      int           typez;

      double        xmin;
      double        xmax;
      double        ymin;
      double        ymax;
      double        zmin;
      double        zmax;
      double        zfloor;
      double        zscale;
      double        gridres;
      double        pksmooth;
      double        pkwidth;

      QString       xatitle;
      QString       yatitle;
      QString       zatitle;

      QFrame*       frame;

      QVector< QVector< double > > zdata;

      Qwt3D::SurfacePlot* dataWidget;

   protected:
      QwtPlot*      data_plot1;

      US_Model*     model;

      QCheckBox*  ck_plteda;


   private slots:

      QString xyAxisTitle( int );
      QString zAxisTitle(  int );
      void    setStandardView( void );
      void    createActions(   void );
      void    createMenus(     void );
      void    createToolBar(   void );
      double  comp_value( US_Model::SimulationComponent*, int );

};
#endif

