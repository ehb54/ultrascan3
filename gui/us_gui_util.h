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
      //! \param filename Full path name of the file to produce. A ".svg" name also saves a PNG file.
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok, 1 if filename is not an SVG or PNG name, 2 if only the
      //!          PNG could not be written, 3 if the SVG could not be compressed. A failed SVG does
      //!          not stop the PNG, but hides a PNG failure.
      static int save_plot( const QString&, const QwtPlot* );

      //! \brief Show a warning that names the plot files that could not be written
      //!
      //! \param parent   Parent widget of the warning
      //! \param unsaved  Files that could not be written; the first 10 are named
      //! \returns true if a warning was shown, false if unsaved is empty
      static bool warn_unsaved( QWidget* parent, const QStringList& unsaved );

      //! \brief Save a plot to a gzip-compressed SVG (.svgz) file
      //!
      //! \param filename Full path name of the file to produce. Its extension is replaced by ".svgz".
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok, 1 if filename does not contain ".svg",
      //!          3 if the SVG could not be compressed (the uncompressed .svg is kept)
      static int save_svg( const QString&, const QwtPlot* );

      //! \brief Save a plot to a PNG file
      //!
      //! \param filename Full path name of the file to produce
      //! \param plot     A pointer to the plot to save
      //! \returns A status flag:  0 if all-ok, 1 if filename does not end with ".png",
      //!          2 if the file could not be written
      static int save_png( const QString&, const QwtPlot* );

   //! \brief Save a plot to a CSV file
   //!
   //! \param filename Full path name of the file to produce
   //! \param plot     A pointer to the plot to save
   //! \returns A status flag:  0 if all-ok
   static int save_csv( const QString&, const QwtPlot* );

   //! \brief Digits a counter's width allows for its initial value.
   //!
   //! n = int( log10( |value| ) ) + 1, or 4 - n when n is not positive
   //! (a value below 0.1), plus one for a minus sign, from 1 to 14.  A
   //! value of 0, or one that is not finite, gets the full 14.  Both
   //! US_Widgets::us_counter and US_WidgetsDialog::us_counter use it.
   static int    counter_digits( double value );
};
#endif
