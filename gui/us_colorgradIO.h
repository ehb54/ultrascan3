#ifndef US_COLORGRIO_H
#define US_COLORGRIO_H

#include <QtGui>
#include <QtXml>

//! \brief Get colors list from color steps XML file. All functions are static.

class US_ColorGradIO
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
      */
      static int read_color_gradient( QString, QList< QColor >& );
};

#endif
