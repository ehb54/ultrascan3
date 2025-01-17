//! \file us_images.cpp
#include "us_images.h"

// Get image as a pixmap
QPixmap US_Images::getImage( const QString& bname )
{
   return QPixmap( expanded_name( bname) );
}

// Get image as a pixmap from type flag
QPixmap US_Images::getImage( int itype )
{
   return US_Images::getImage( image_name( itype ) );
}

// Get image as an icon
QIcon US_Images::getIcon( const QString& bname )
{
   return QIcon( US_Images::getImage( bname ) );
}

// Get image as an icon from type flag
QIcon US_Images::getIcon( int itype )
{
   return QIcon( US_Images::getImage( itype ) );
}


// Private method to find a base image name for an image type
const QString US_Images::image_name( int itype )
{
   struct nameMap
   {
      ImagesType  itype;
      QString     iname;
   };

   static const struct nameMap nameMaps[] =
   {
      { US3_SPLASH,      "us3-splash"     },
      { CHECK,           "check"          },
      { ARROW_BLUE,      "bluearrow"      },
      { ARROW_GREEN,     "greenarrow"     },
      { ARROW_RED,       "redarrow"       },
      { AXES_BOX,        "box"            },
      { AXES_FRAME,      "frame"          },
      { AXES_NONE,       "none"           },
      { FILE_CELL,       "filecell"       },
      { FILE_OPEN,       "fileopen"       },
      { FLOOR_DATA,      "floordata"      },
      { FLOOR_EMPTY,     "floorempty"     },
      { FLOOR_ISO,       "flooriso"       },
      { FLOOR_MESH,      "floormesh"      },
      { GRID_NONE,       "grid"           },
      { GRID_BACK,       "gridb"          },
      { GRID_CEILING,    "gridc"          },
      { GRID_FLOOR,      "gridf"          },
      { GRID_FRONT,      "gridfr"         },
      { GRID_LEFT,       "gridl"          },
      { GRID_RIGHT,      "gridr"          },
      { MESH_FILLED,     "filledmesh"     },
      { MESH_HIDDEN,     "hiddenline"     },
      { MESH_NODATA,     "nodata"         },
      { MESH_NORMALS,    "normals"        },
      { MESH_POLYGON,    "polygon"        },
      { MESH_SCATTERED,  "scattered"      },
      { MESH_WIRE,       "wireframe"      },
      { MOVIE,           "movie"          },
      { SAVE_CONTENT,    "savecontent"    },
      { ICON_PROP,       "icon"           },
      { QWTPLOT,         "qwtplot"        },
      { US3_ICON,        "us3-icon-48x48" },
      { ARROW_LEFT,      "arrow_left"     },
      { ARROW_RIGHT,     "arrow_right"    },
      { SETUP_COM,       "setup"          },
      { ANALYSIS_COM,    "analysis"       },
      { EDITING_COM,     "editing"        },
      { ANALYSIS_COM_2,  "analysis_2"     },
      { LIVE_UPDATE_COM, "live_update"    },
      { IMPORT_COM_1,    "import_1"       },
      { IMPORT_COM_2,    "import_2"       },
      { REPORT_COM,      "report"         },
      { NEW_ITEM_COM,    "new_item2"      },
      { TABLE,           "table"         }
   }; 

   static const int nimages = sizeof( nameMaps ) / sizeof( nameMaps[ 0 ] );
   
   Q_ASSERT( itype >= 0 && itype < nimages );

   QString iname( "_NOT_FOUND_" );

   for ( int jj = 0; jj < nimages; jj++ )
   {  // Find a match to the given type
      if ( (ImagesType)itype == nameMaps[ jj ].itype )
      {  // When a match is found, return the corresponding base image name
         iname = nameMaps[ jj ].iname;
         break;
      }
   }

   return iname;
}

// Private method to get a full expanded embedded image name from a base name
const QString US_Images::expanded_name( const QString& bname )
{
   QString iname = bname;
   iname  = iname.replace( "\\", "/" );   // Insure forward slashes in name

   if ( ! iname.contains( "/" ) )
   {
      iname  = ":/images/" + bname;       // Add the standard embedded prefix
   }

   if ( ! iname.contains( "." ) )
   {
      iname  = iname + ".png";            // Add the PNG extension
   }

   return iname;
}

