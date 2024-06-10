//! \file us_colorgradIO.h
#ifndef US_COLORGRIO_H
#define US_COLORGRIO_H

#if QT_VERSION > 0x050000
#include <QtWidgets>
#else
#include <QtGui>
#define setSingleStep(a) setStep(a)
#define setMinorPen(a) setMinPen(a)
#define setMajorPen(a) setMajPen(a)
#endif
#include <QtXml>

#include "us_extern.h"

//! \brief Get colors list from color steps XML file. All functions are static.

class US_GUI_EXTERN US_ColorGradIO
{
   public:

      static const int XFS_OK    = 0;  //!< xml file opened & successfully read
      static const int XFS_EROPN = 1;  //!< unable to open xml file
      static const int XFS_ERXML = 2;  //!< file is not xml
      static const int XFS_ERCST = 3;  //!< file is not color step

      /*! \brief Public static function to create a gradient colors list
                 from a color steps XML file.
          \param xmlfilename The full path name to an XML color gradient file.
          \param gcolors     Returned list of gradient colors
          \return            Return code: 0 (XFS_OK)    -> ok;
                                          1 (XFS_EROPN) -> can't open;
                                          2 (XFS_ERXML) -> not xml;
                                          3 (XFS_ERCST) -> not color steps.

          This function returns the full list of colors implied by the
          input color steps xml file.
      */
      static int read_color_gradient( QString, QList< QColor >& );

      /*! \brief Public static function to create color steps and step value
                 lists from a color steps XML file.
          \param xmlfilename The full path name to an XML color gradient file.
          \param scolors     Returned list of step colors.
          \param svalues     Returned corresponding list of step values.
          \return            Return code: 0 (XFS_OK)    -> ok;
                                          1 (XFS_EROPN) -> can't open;
                                          2 (XFS_ERXML) -> not xml;
                                          3 (XFS_ERCST) -> not color steps.

          This function returns the list of step colors and the corresponding
          list of step values in the range 0.0 to 1.0. The first of the values
          list will always be 0.0 and the last always 1.0, with values in
          between ordered and inside those ranges. This pair of lists can
          easily be used in QwtLinearColorMap's constructor and it's
          addColorStep method.
      */
      static int read_color_steps( QString, QList< QColor >&,
            QList< double >& );

      /*! \brief Public static function to create a list of the integer
                 parameter values from a color steps XML file.
          \param xmlfilename The full path name to an XML color gradient file.
          \param cparams     Returned list of color step parameters, where each
                             set of 4 is a (red,green,blue,npoints) quadruple.
          \return            Return code: 0 (XFS_OK)    -> ok;
                                          1 (XFS_EROPN) -> can't open;
                                          2 (XFS_ERXML) -> not xml;
                                          3 (XFS_ERCST) -> not color steps.

          This function returns the color gradient attribute values from
          an input color steps xml file. The integer values are ordered
          into (red, green, blue, number_points ) sets, so the number of
          steps is one fourth the returned list's size less one.
      */
      static int read_color_step_params( QString, QList< int >& );
};

#endif
