#ifndef US_PROCESS_CONVERT_H
#define US_PROCESS_CONVERT_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_help.h"
#include "us_convert.h"

class US_EXTERN US_ProcessConvert : public US_WidgetsDialog
{
  Q_OBJECT

   public:
      US_ProcessConvert( QWidget* parent );
      US_ProcessConvert( QWidget* parent,
                         QString dir,
                         QList< US_DataIO::beckmanRaw >& rawLegacyData,
                         QString& runType );
      US_ProcessConvert( QWidget* parent,
                         QList< US_DataIO::beckmanRaw >& rawLegacyData,
                         QList< US_DataIO::rawData    >& rawConvertedData,
                         QStringList& triples,
                         QString runType,
                         double tolerance,
                         QList< double >& ss_limits );
      US_ProcessConvert( QWidget* parent,
                         int& status,
                         QList< US_DataIO::rawData >& rawConvertedData,
                         US_Convert::ExperimentInfo& ExpData,
                         QStringList& triples,
                         QString runType,
                         QString runID,
                         QString dirname );

   private:
      QLabel*            lb_progress;
      QProgressBar*      progress;
      bool               canceled;

      US_Help            showHelp;
   
   private slots:
      void convert       ( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                  US_DataIO::rawData& newRawData,
                                  QString triple, 
                                  QString runType,
                                  double tolerance );
      void splitRAData   ( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                                     QList< double >& ss_limits );
      void setTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                       QStringList& triples,
                       QString runType,
                       double tolerance );
      void setCcwTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                          QStringList& triples,
                          double tolerance );
      void setCcrTriples( QList< US_DataIO::beckmanRaw >& rawLegacyData,
                          QStringList& triples,
                          double tolerance );
      void setInterpolated ( unsigned char*, int );
      int writeXmlFile( US_Convert::ExperimentInfo& ExpData,
                        QStringList& triples,
                        QString runType,
                        QString runID,
                        QString dirname );
      void createDialog  ( void );
      void reset         ( void );
      void cancel        ( void );
      void help          ( void )
        { showHelp.show_help( "manual/us_convert.html" ); };
};
#endif
