//! \file us_process_convert.h
#ifndef US_PROCESS_CONVERT_H
#define US_PROCESS_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO2.h"
#include "us_help.h"
#include "us_convert.h"
#include "us_expinfo.h"

/*! \class US_ConvertProgressBar
           This class creates and displays a progress bar to inform
           the user of the progress of the current operation
*/
class US_EXTERN US_ConvertProgressBar : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Generic constructor for the US_ConvertProgressBar class. This
                 constructor establishes the dialog and its relationship to
                 the parent dialog. US_ProcessConvert inherits from this.

          \param parent Establishes the US_ConvertProgressBar dialog as a child
                        of the specified parent.
      */
      US_ConvertProgressBar( QWidget* );

      /*! \brief Sets the contents of a text label displayed with the class

          \param text    The contents of the text label
      */
      void               setLabel    ( QString );

      /*! \brief Sets the range of the progress bar

          \param min     The minimum, or start value
          \param max     The maximum, or ending value
      */
      void               setRange    ( int, int );

      /*! \brief Sets the current value of the progress bar and ensures the
                 dialog will display, unless the action has been canceled.

          \param value   The current value
      */
      void               setValue    ( int );

      /*! \brief Sets the visible property of the progress bar.

          \param visible Whether the progress bar should be visible or not
      */
      void               display     ( bool );

      /*! \brief Returns whether the progress bar is active or not. If the progress
                 bar is not visible, most likely it is because the user has 
                 pressed the cancel button.
      */
      bool               isActive    ( void );

   private:
      QLabel*            lb_progress;
      QProgressBar*      progress;
      bool               canceled;
      bool               visible;
      int                max;

   private slots:
      void cancel        ( void );
};

/*! \class US_ProcessConvert
           This class provides the ability to convert raw data in the
           Beckman format to the file format used by US3. 
*/
class US_EXTERN US_ProcessConvert : public US_ConvertProgressBar
{
  Q_OBJECT

   public:
      /*! \brief Generic constructor for the US_ProcessConvert class. This
                 constructor establishes the dialog and its relationship to
                 the parent dialog.

          \param parent Establishes the US_ProcessConvert dialog as a child
                        of the specified parent.
      */
      US_ProcessConvert( QWidget* );

      /*! \brief Reads the legacy raw data from disk into the program. 

          \param dir    The directory in which the program will look for the
                        raw data files.
          \param rawLegacyData A reference to a structure provided by the calling
                        function that will be used to store the imported raw data.
          \param runType A reference to a variable that will contain the type
                        of data that is being read ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This determination already affects how some data is
                        handled when read.
      */
      void          readLegacyData( 
                    QString ,
                    QList< US_DataIO2::BeckmanRawScan >& ,
                    QString& );

      /*! \brief Converts legacy raw data into US3 data. 
                 This function will convert existing datapoints and cell/
                 channel/wavelength combinations in the data.

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
      void          convertLegacyData(
                    QList  < US_DataIO2::BeckmanRawScan >& ,
                    QVector< US_DataIO2::RawData        >& ,
                    QStringList& ,
                    QString ,
                    double ,
                    QList< double >& );

      /*! \brief Writes the converted US3 data to disk. 

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
          \param tripleMap A reference to a QList of index values indicating
                        which triples are members of the current data set. Triples
                        included in the map will be written, others will not.
          \param allExcludes A reference to a QVector of excluded scans for
                        each c/c/w combination
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        written.
          \param runID  The run ID of the experiment.
          \param dirname The directory in which the files are to be written.
      */
      void          writeConvertedData(
                    int& status,
                    QVector< US_DataIO2::RawData >& ,
                    US_ExpInfo::ExperimentInfo& ,
                    QStringList& ,
                    QList< int >& ,
                    QVector< US_Convert::Excludes >& ,
                    QString ,
                    QString ,
                    QString );

      /*! \brief Reloads converted US3 data back into the program to sync
                 up with the database. 

          \param dir    The directory that contains the auc files
          \param rawConvertedData A reference to a structure provided by the calling
                        function that will be used to store the US3 raw converted data.
          \param triples A reference to a structure provided by the calling
                        function that will be used to store all the different
                        cell/channel/wavelength combinations found in the data. 
          \param tripleMap A reference to a QList of index values indicating
                        which triples are members of the current data set. Triples
                        included in the map will be written, others will not.
          \param runType A reference to a variable that already contains the type
                        of data ( "RA", "IP", "RI", "FI", "WA", or "WI").
                        This information will affect how the data is
                        converted.
      */
      void          reloadUS3Data(
                    QString ,
                    QVector< US_DataIO2::RawData        >& ,
                    QStringList& ,
                    QList< int >& ,
                    QString );

   private:
      US_Help            showHelp;

      void convert       ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           US_DataIO2::RawData&          newRawData,
                           QString                       triple, 
                           QString                       runType,
                           double                        tolerance );

      void splitRAData   ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QList< double >&                     ss_limits );

      void setTriples    ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QStringList&                         triples,
                           QString                              runType,
                           double                               tolerance );
      
      void setCcwTriples ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QStringList&                         triples,
                           double                               tolerance );
      
      void setCcrTriples ( QList< US_DataIO2::BeckmanRawScan >& rawLegacyData,
                           QStringList&                         triples,
                           double                               tolerance );
      
      void setInterpolated ( unsigned char*, int );
      
   private slots:
      void help          ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
