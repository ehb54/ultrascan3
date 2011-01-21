//! \file us_images.h
#ifndef US_IMAGES_H
#define US_IMAGES_H

#include <QtCore>
#include <QtGui>

#include "us_extern.h"

//! \brief Fetch an embedded image as Pixmap or Icon. All functions are static.
//!
//! These methods are primarily designed to return a QPixmap or a QIcon of
//! an embedded image. So the input argument is either a base image name
//! string or an integer enum value. The methods may also take a full image
//! file path name as an argument. The following are equivalent:
//!
//! 1) QIcon check = US_Images::getIcon( US_Images::CHECK );
//!
//! 2) QIcon check = US_Images::getIcon( "check" );
//!
//! 3) QIcon check = US_Images::getIcon( ":/images/check.png" );
//!
//! 4) QIcon check = US_Images::getIcon(
//!                     US_Settings::usHomeDir() + "/etc/check.png" );
//!
//! 5) QPixmap cpix = US_Images::getImage( US_Images::CHECK );
//!    QIcon check  = QIcon( cpix );
//!    int ckwidth  = cpix.width();
//!
//! Note: the last variation addresses the need to both get an image and
//! determine its width.

class US_EXTERN US_Images
{
   public:
      //! \brief Type values for images available
      enum ImagesType
      {
         US3_SPLASH,  CHECK,
         ARROW_BLUE,  ARROW_GREEN,  ARROW_RED,
         AXES_BOX,  AXES_FRAME,  AXES_NONE,
         FILE_CELL,  FILE_OPEN,
         FLOOR_DATA,  FLOOR_EMPTY,  FLOOR_ISO,  FLOOR_MESH,
         GRID_NONE,  GRID_BACK,  GRID_CEILING,  GRID_FLOOR,
         GRID_FRONT,  GRID_LEFT,  GRID_RIGHT,
         MESH_FILLED,  MESH_HIDDEN,  MESH_NODATA,  MESH_NORMALS,
         MESH_POLYGON,  MESH_SCATTERED,  MESH_WIRE,
         MOVIE,  SAVE_CONTENT,  ICON_PROP,
         QWTPLOT
      };

      //! \brief Get image (embedded or from file) as a pixmap
      //! \param  bname Name of embedded image or full path image file name
      //! \return       QPixmap of named image
      static QPixmap getImage( const QString& );

      //! \brief Get image (embedded or from file) as a pixmap
      //! \param  itype Enum type flag of desired embedded image
      //! \return       QPixmap of named image
      static QPixmap getImage( int );
                                   
      //! \brief Get image (embedded or from file) as an icon
      //! \param  bname Name of embedded image or full path image file name
      //! \return       QIcon of named image
      static QIcon getIcon( const QString& );

      //! \brief Get image (embedded or from file) as an icon
      //! \param  itype Enum type flag of desired embedded image
      //! \return       QIcon of named image
      static QIcon getIcon( int );
                
   private:
      //! \brief Get the image name corresponding to a given type flag
      //! \param  itype Enum type flag of desired embedded image
      //! \return       Name of image corresponding to given type
      static const QString image_name( int );

      //! \brief Get the full expanded embedded image name
      //! \param  bname Base name of embedded image (e.g., "check")
      //! \return       Full embedded image name (e.g., ":/images/check.png")
      static const QString expanded_name( const QString& );
};

#endif
