//! \file test_us_experiment_disk_gui.cpp
// The experiment window with the data location set to Disk, where there is no
// database connection to read the lab's instruments from.

#include "us3_gui_test_main.h"
#include "us_experiment_gui_optima.h"

#include <QMessageBox>
#include <QTimer>

class US_ExperimentDiskTest : public QObject
{
      Q_OBJECT

   private slots:
      void opensWithTheDataLocationSetToDisk()
      {
         US_Settings::set_default_data_location( 2 );

         // The window shows messages about the missing database while it opens
         QTimer closeMessages;
         connect( &closeMessages,
                  &QTimer::timeout,
                  []
                  {
                     for ( QWidget* widget : QApplication::topLevelWidgets() )
                     {
                        QMessageBox* message = qobject_cast<QMessageBox*>( widget );

                        if ( message != nullptr && message->isVisible() )
                        {
                           message->accept();
                        }
                     }
                  } );
         closeMessages.start( 20 );

         // Not deleted, as in the program's main()
         US_ExperimentMain* window = new US_ExperimentMain;

         QCOMPARE( window->windowTitle(), QString( "Define An Experiment" ) );
      }
      void refusesSaveAndSubmitWithoutACurrentProfile()
      {
         US_Settings::set_default_data_location( 2 );
         int    refusals = 0;
         QTimer closeMessages;
         connect( &closeMessages,
                  &QTimer::timeout,
                  [&]
                  {
                     for ( QWidget* widget : QApplication::topLevelWidgets() )
                     {
                        auto* message = qobject_cast<QMessageBox*>( widget );
                        if ( message && message->isVisible() )
                        {
                           if ( message->text().contains( "Review the analysis profile for the current protocol" ) )
                           {
                              ++refusals;
                           }
                           message->accept();
                        }
                     }
                  } );
         closeMessages.start( 20 );

         auto* window = new US_ExperimentMain;
         auto* upload = window->findChild<US_ExperGuiUpload*>();
         QVERIFY( upload != nullptr );
         QVERIFY( !window->aprofile_usable() );
         bool aborted = false;
         QVERIFY( QMetaObject::invokeMethod( upload, "saveRunProtocol", Qt::DirectConnection, Q_RETURN_ARG( bool, aborted ) ) );
         QVERIFY( aborted );
         QVERIFY( QMetaObject::invokeMethod( upload, "submitExperiment_confirm_dataDisk", Qt::DirectConnection ) );
         QCOMPARE( refusals, 2 );
         QVERIFY( window->currProto.rpSubmt.us_xml.isEmpty() );
      }
};

US3_GUI_TEST_MAIN( US_ExperimentDiskTest )
#include "test_us_experiment_disk_gui.moc"
