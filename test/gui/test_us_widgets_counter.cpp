//! \file test_us_widgets_counter.cpp
#include <limits>

#include <qwt_counter.h>

#include "us3_gui_test_main.h"
#include "us_gui_util.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

class US_WidgetsCounterTest : public QObject
{
      Q_OBJECT

   private slots:
      void digitsFollowTheInitialValue()
      {
         QCOMPARE( US_GuiUtil::counter_digits( 0.0 ), 14 );
         QCOMPARE( US_GuiUtil::counter_digits( -0.0 ), 14 );
         QCOMPARE( US_GuiUtil::counter_digits( std::numeric_limits<double>::quiet_NaN() ), 14 );
         QCOMPARE( US_GuiUtil::counter_digits( std::numeric_limits<double>::infinity() ), 14 );
         QCOMPARE( US_GuiUtil::counter_digits( 1.0 ), 1 );
         QCOMPARE( US_GuiUtil::counter_digits( 50.0 ), 2 );
         QCOMPARE( US_GuiUtil::counter_digits( -50.0 ), 3 );
         QCOMPARE( US_GuiUtil::counter_digits( 0.5 ), 1 );
         QCOMPARE( US_GuiUtil::counter_digits( 0.05 ), 4 );
         QCOMPARE( US_GuiUtil::counter_digits( 0.01 ), 5 );
         QCOMPARE( US_GuiUtil::counter_digits( 1.0e20 ), 14 );
      }

      // A counter created at 0 is as wide as one created at a 14-digit value,
      // in a window and in a dialog.
      void zeroGivesTheFullWidthInWindowsAndDialogs()
      {
         US_Widgets       window( false );
         US_WidgetsDialog dialog( nullptr, Qt::WindowFlags() );
         QwtCounter*      w_zero = window.us_counter( 2, 0, 100, 0 );
         QwtCounter*      w_full = window.us_counter( 2, 0, 1.0e14, 1.0e13 );
         QwtCounter*      w_one  = window.us_counter( 2, 0, 100, 1 );
         QwtCounter*      d_zero = dialog.us_counter( 2, 0, 100, 0 );
         QCOMPARE( w_zero->minimumWidth(), w_full->minimumWidth() );
         QVERIFY( w_zero->minimumWidth() > w_one->minimumWidth() );
         QCOMPARE( d_zero->minimumWidth(), w_zero->minimumWidth() );
         delete w_zero;
         delete w_full;
         delete w_one;
         delete d_zero;
      }
};

US3_GUI_TEST_MAIN( US_WidgetsCounterTest )

#include "test_us_widgets_counter.moc"
