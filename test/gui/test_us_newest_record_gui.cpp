//! \file test_us_newest_record_gui.cpp
// The record that autoflow analysis and the GMP reporter take as the newest
// noise or model of a stage.

#include "us3_gui_test_main.h"

#include "us_auto_noise.h"

class US_NewestRecordTest : public QObject
{
      Q_OBJECT

   private slots:
      // The smallest age wins, the first of equal ages is kept
      void keepsTheNewest()
      {
         US_NewestRecord newest;
         newest.offer( 1, 300 );
         newest.offer( 2, 100 );
         newest.offer( 3, 100 );
         newest.offer( 4, 500 );
         QCOMPARE( newest.id(), 2 );
      }

      // A record from years ago is still taken when it is the only one
      void takesAnOldRecord()
      {
         US_NewestRecord newest;
         newest.offer( 7, 300000000 );
         QCOMPARE( newest.id(), 7 );
      }

      void noneOffered()
      {
         US_NewestRecord newest;
         QCOMPARE( newest.id(), 0 );
      }
};

US3_GUI_TEST_MAIN( US_NewestRecordTest )

#include "test_us_newest_record_gui.moc"
