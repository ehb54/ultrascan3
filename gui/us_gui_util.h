//! \file us_gui_util.h
#ifndef US_GUIUTIL_H
#define US_GUIUTIL_H

#if QT_VERSION > 0x050000
#include <QtWidgets>
#define dataPlotClear(a) a->detachItems(QwtPlotItem::Rtti_PlotItem,false)
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#define dataPlotClear(a) a->clear()
#endif
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_series_data.h"
#include <qwt_plot_spectrogram.h>
#include <qwt_raster_data.h>
#include <qwt_matrix_raster_data.h>
#include "us_extern.h"

//! \brief General GUI utilities for UltraScan
//!
//! This class provides a number of general utility functions.
//! All methods are static.
//!
class US_GUI_EXTERN US_GuiUtil
{
   public:
      //! \brief Save a plot to an SVG and/or PNG file
      //!
      //! \param filename Full path name of the file to produce
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok
      static int save_plot( const QString&, const QwtPlot* );

      //! \brief Save a plot to an SVG file
      //!
      //! \param filename Full path name of the file to produce
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok
      static int save_svg( const QString&, const QwtPlot* );

      //! \brief Save a plot to a PNG file
      //!
      //! \param filename Full path name of the file to produce
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok
      static int save_png( const QString&, const QwtPlot* );

   //! \brief Save a plot to a CSV file
   //!
   //! \param filename Full path name of the file to produce
   //! \param plot     A pointer to the plot to save
   //! \returns A status flag:  0 if all-ok
   static int save_csv( const QString&, const QwtPlot* );

};
#endif
