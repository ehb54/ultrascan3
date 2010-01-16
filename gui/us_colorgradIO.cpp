//! \file us_colorgradIO.cpp

#include "us_colorgradIO.h"
#include "us_defines.h"

/*! \brief Public static function to create a gradient colors list
           from a color steps XML file.
    \param xmlfilename The full path name to an XML color gradient file.
    \param gcolors     Returned list of gradient colors
    \return            Return code: 0 (XFS_OK)    -> ok;
                                    1 (XFS_EROPN) -> can't open;
                                    2 (XFS_ERXML) -> not xml;
                                    3 (XFS_ERCST) -> not color steps.
*/
int US_ColorGradIO::read_color_gradient( QString          xmlfilename,
                                         QList< QColor >& gcolors )
{
   int rc = XFS_OK;
   gcolors.clear();                 // clear out the list

   QFile filei( xmlfilename );

   if ( filei.open( QIODevice::ReadOnly ) )
   {	// able to open file
      QXmlStreamReader xmli( &filei );
      bool is_uscs = false;
      int sred;		// start RGB of step
      int sgrn;
      int sblu;
      int ered;		// end RGB of step
      int egrn;
      int eblu;

      while( ! xmli.atEnd() )
      {	// read each element of the XML
         xmli.readNext();

         if ( xmli.isComment() )
         {  // comment line:  verify that we have color steps type
            QString comm = xmli.text().toString();

            if ( comm.contains( "UltraScanColorSteps" ) )
            { // mark that we have the right type
               is_uscs   = true;
            }
         }

         if ( xmli.isStartElement()  &&  xmli.name() == "color" )
         {	// start of "color" entry
            // get entries for a color step
            QXmlStreamAttributes ats = xmli.attributes();
            ered      = ats.value( "red" ).toString().toInt();
            egrn      = ats.value( "green" ).toString().toInt();
            eblu      = ats.value( "blue" ).toString().toInt();
            int npts  = ats.value( "points" ).toString().toInt();

            if ( npts > 0 )
            {  // a step entry:  expand all color points
               qreal dpts   = (qreal)npts;
               // get delta values between color points in the step
               qreal dred   = (qreal)( ered - sred ) / dpts;
               qreal dgrn   = (qreal)( egrn - sgrn ) / dpts;
               qreal dblu   = (qreal)( eblu - sblu ) / dpts;
               // start with the previous step's end
               qreal ored   = (qreal)sred;
               qreal ogrn   = (qreal)sgrn;
               qreal oblu   = (qreal)sblu;

               for ( int ii = 0; ii < npts; ii++ )
               {	// output colors from interpolated RGB
                  ored        += dred;             // bump color values
                  ogrn        += dgrn;
                  oblu        += dblu;
                  int ired     = qRound( ored );   // integer equivalent
                  int igrn     = qRound( ogrn );
                  int iblu     = qRound( oblu );
                  // add to list
                  gcolors.append( QColor( ired, igrn, iblu ) );
               }     
            }
            else
            { // first element (npts=0):  start color
               gcolors.append( QColor( ered, egrn, eblu ) );
            }

            // set start values for next step
            sred         = ered;
            sgrn         = egrn;
            sblu         = eblu;
         }
      }

      filei.close();                                // close file
      rc        = is_uscs ? rc : XFS_ERCST;         // verify file color step
      rc        = xmli.hasError() ? XFS_ERXML : rc; // verify file xml
   }
   else
   { // error: unable to open file
      rc        = XFS_EROPN;
   }
   return rc;           // return code
}

