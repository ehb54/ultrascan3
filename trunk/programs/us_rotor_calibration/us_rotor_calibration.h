#ifndef US_EDIT_H
#define US_EDIT_H

#include <uuid/uuid.h>

#include <QtGui>
#include <QApplication>
#include <QDomDocument>

#include <qwt_plot_marker.h>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO2.h"
#include "us_matrix.h"
#include "us_editor.h"

struct Average
{
   double top, bottom;
   int cell, rpm, channel, top_count, bottom_count;
};

class US_EXTERN US_RotorCalibration : public US_Widgets
{
	Q_OBJECT

	public:
		US_RotorCalibration();

	private:

      US_DataIO2::RawData            data;
      QVector< US_DataIO2::RawData > allData;
      QVector <Average> avg;
      QVector <QVector <double> > reading;
      QVector <double> stretch_factors, std_dev;
      bool leftCB, rightCB, leftCL, rightCL, newlimit;

      double             left, right, top, bottom, coef[3];
      double             *x, *y, *sd1, *sd2;
      int                step, maxcell;
      QString            rotor, fileText;
      
      US_Help            showHelp;

      QIcon              check;

      QPushButton*       pb_reset;
      QPushButton*       pb_accept;
      QPushButton*       pb_leftCells;
      QPushButton*       pb_leftCounterbalance;
      QPushButton*       pb_rightCells;
      QPushButton*       pb_rightCounterbalance;
      QPushButton*       pb_save;
      QPushButton*       pb_loadRotor;
      QPushButton*       pb_view;
            
      QString            workingDir;
      QString            runID;
      QString            editID;
      QString            dataType;
      QStringList        files;
      QStringList        triples;
                      
      QwtPlot*           data_plot;
      QwtPlotCurve*      fit_curve;
      QwtPlotCurve*      v_line;
      QwtPlotCurve*      minimum_curve;
      QwtPlotGrid*       grid;
      QwtPlotMarker*     marker;
      QwtDoubleRect      limits[4];

      US_PlotPicker*     pick;
      US_Plot*           plot;
      
//      QLabel*            lbl_instructions;
//      QLabel*            lbl_spacer;

      QLineEdit*         le_instructions;
      QLineEdit*         le_rotorInfo;
                        
      QRadioButton*      rb_counterbalance;
      QRadioButton*      rb_cells;
      QRadioButton*      rb_all;

   public slots:
      void help (void)
      {
         showHelp.show_help( "manual/rotor_calibration.html" );
      };
      void reset (void);
      void load (void);
      void plotAll (void);
      void currentRect (QwtDoubleRect);
      void leftCounterbalance (void);
      void rightCounterbalance (void);
      void leftCells (void);
      void rightCells (void);
      void showAll (void);
      void showCells (void);
      void showCounterbalance (void);
      void next(void);
      void accept(void);
      void calculate(void);
      double findAverage(QwtDoubleRect, US_DataIO2::RawData, int);
      void checkAccept(void);
      void save(void);
      void view(void);
      void loadRotor(void);
};
#endif
