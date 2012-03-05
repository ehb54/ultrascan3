//! \file us_gui_util.h
#ifndef US_GUIUTIL_H
#define US_GUIUTIL_H

#include <QtGui>

#include "qwt_plot.h"
#include "us_extern.h"

//! \brief General GUI utilities for UltraScan
//!
//! This class provides a number of general utility functions.
//! All methods are static.
//!
class US_UTIL_EXTERN US_GuiUtil
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

};
#endif
