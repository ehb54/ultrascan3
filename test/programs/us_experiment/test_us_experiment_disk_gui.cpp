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
};

US3_GUI_TEST_MAIN( US_ExperimentDiskTest )
#include "test_us_experiment_disk_gui.moc"
