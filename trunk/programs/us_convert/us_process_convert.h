//! \file us_process_convert.h
#ifndef US_PROCESS_CONVERT_H
#define US_PROCESS_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_help.h"
#include "us_convert.h"
#include "us_expinfo.h"

/*! \class US_ProcessConvert
           This class provides the ability to convert raw data in the
           Beckman format to the file format used by US3. 
*/
class US_EXTERN US_ProcessConvert : public US_WidgetsDialog
{
  Q_OBJECT

   public:
// Generic constructor; establishes dialog
      /*! \brief Generic constructor for the US_ProcessConvert class. This
                 constructor establishes the dialog and its relationship to
                 the parent dialog. Other US_ProcessConvert constructors 
                 inherit from this.

          \param parent Establishes the US_ProcessConvert dialog as a child
                        of the specified parent.
      */
      US_ProcessConvert( QWidget* );

      /*! \brief A constructor for the US_ProcessConvert class, intended for
                 reading the legacy raw data from disk into the program. 
                 Inherits from the generic US_ProcessConvert() class.

          \param parent Establishes the US_ProcessConvert dialog as a child
                        of the specified parent.
          \param dir    The directory in which the program will look for the
                        raw data files.
          \param rawLegacyData A reference to a structure provided by the calling
                        function that will be used to store the imported raw data.
          \param runType A reference to a variable that will contain the type
                        of data that is being read ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This determination already affects how some data is
                        handled when read.
      */
      US_ProcessConvert( QWidget* ,
                         QString ,
                         QList< US_DataIO::beckmanRawScan >& ,
                         QString& );

      /*! \brief A constructor for the US_ProcessConvert class, intended for
                 initiating the conversion of legacy raw data into US3 data. 
                 This function will convert existing datapoints and cell/
                 channel/wavelength combinations in the data.
                 Inherits from the generic US_ProcessConvert() class.

          \param parent Establishes the US_ProcessConvert dialog as a child
                 of the specified parent.
          \param rawLegacyData A reference to a structure provided by the calling
                        function that already contains the imported raw data.
          \param rawConvertedData A reference to a structure provided by the calling
                        function that will be used to store the US3 raw converted data.
          \param triples A reference to a structure provided by the calling
                        function that will be used to store all the different
                        cell/channel/wavelength combinations found in the data. 
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        converted.
          \param tolerance How far apart the wavelength readings can be and still
                        be considered part of the same cell/channel/wavelength.
          \param ss_limits In the case of RA data only, sometimes the data needs
                        to be split into multiple datasets. In this case, ss_limits
                        should already contain a list of radius values that define
                        where one dataset ends and the next one begins.
      */
      US_ProcessConvert( QWidget* ,
                         QList< US_DataIO::beckmanRawScan >& ,
                         QVector< US_DataIO::rawData    >& ,
                         QStringList& ,
                         QString ,
                         double ,
                         QList< double >& );

      /*! \brief A constructor for the US_ProcessConvert class, intended for
                 writing the converted US3 data to disk. 
                 Inherits from the generic US_ProcessConvert() class.

          \param parent Establishes the US_ProcessConvert dialog as a child
                 of the specified parent.
          \param status  A reference to an integer that will contain the status of the
                         operation when it is completed.
          \param rawConvertedData A reference to a structure provided by the calling
                        function that already contains the US3 raw converted data.
          \param ExpData A reference to a structure provided by the calling function
                         that already contains the hardware and other database
                         connection information relevant to this experiment.
          \param triples A reference to a structure provided by the calling
                        function that already contains all the different
                        cell/channel/wavelength combinations in the data. 
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
      */
      US_ProcessConvert( QWidget* ,
                         int& status,
                         QVector< US_DataIO::rawData >& ,
                         US_ExpInfo::ExperimentInfo& ,
                         QStringList& ,
                         QString ,
                         QString ,
                         QString );

   private:
      QLabel*            lb_progress;
      QProgressBar*      progress;
      bool               canceled;

      US_Help            showHelp;
   
   private slots:
      void convert       ( QList< US_DataIO::beckmanRawScan >& rawLegacyData,
                                  US_DataIO::rawData&          newRawData,
                                  QString                      triple, 
                                  QString                      runType,
                                  double                       tolerance );

      void splitRAData   ( QList< US_DataIO::beckmanRawScan >& rawLegacyData,
                           QList< double >&                    ss_limits );

      void setTriples( QList< US_DataIO::beckmanRawScan >& rawLegacyData,
                       QStringList&                        triples,
                       QString                             runType,
                       double                              tolerance );
      
      void setCcwTriples( QList< US_DataIO::beckmanRawScan >& rawLegacyData,
                          QStringList&                        triples,
                          double                              tolerance );
      
      void setCcrTriples( QList< US_DataIO::beckmanRawScan >& rawLegacyData,
                          QStringList&                        triples,
                          double                              tolerance );
      
      void setInterpolated ( unsigned char*, int );
      
      int writeXmlFile( US_ExpInfo::ExperimentInfo& ExpData,
                        QStringList&                triples,
                        QString                     runType,
                        QString                     runID,
                        QString                     dirname );
      
      void createDialog  ( void );
      void reset         ( void );
      void cancel        ( void );
      void help          ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
