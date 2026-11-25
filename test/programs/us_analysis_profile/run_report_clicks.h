//! \file run_report_clicks.h
#ifndef RUN_REPORT_CLICKS_H
#define RUN_REPORT_CLICKS_H
#include <QStringList>

//! Clicks on the Run and Report boxes of three channels, and the values they give
struct RunReportClicks
{
      bool        abde;
      QStringList clicks;
      QString     expected;
};

//! Recorded before #1336. In ABDE the load clears the first B channel's Run box.
inline QList<RunReportClicks> run_report_clicks()
{
   return {
      { false, { "0: Run" }, "run 0,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { false, { "0: Run", "0: Run" }, "run 1,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { false, { "0: Run", "1: Run" }, "run 0,0,1 report 0,0,1 ref 1,2,0 use 0,0,0" },
      { false, { "0: Run", "1: RunReport" }, "run 0,1,1 report 0,0,1 ref 1,2,0 use 0,0,0" },
      { false, { "0: Run", "2: Run" }, "run 0,1,0 report 0,1,0 ref 1,0,2 use 0,0,0" },
      { false, { "0: Run", "2: RunReport" }, "run 0,1,1 report 0,1,0 ref 1,0,2 use 0,0,0" },
      { false, { "0: RunReport" }, "run 1,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { false, { "0: RunReport", "0: Run" }, "run 0,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { false, { "0: RunReport", "0: RunReport" }, "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0" },
      { false, { "0: RunReport", "1: Run" }, "run 1,0,1 report 0,0,1 ref 1,2,0 use 0,0,0" },
      { false, { "0: RunReport", "1: RunReport" }, "run 1,1,1 report 0,0,1 ref 1,2,0 use 0,0,0" },
      { false, { "0: RunReport", "2: Run" }, "run 1,1,0 report 0,1,0 ref 1,0,2 use 0,0,0" },
      { false, { "0: RunReport", "2: RunReport" }, "run 1,1,1 report 0,1,0 ref 1,0,2 use 0,0,0" },
      { false, { "1: Run" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { false, { "1: Run", "0: Run" }, "run 0,0,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { false, { "1: Run", "0: RunReport" }, "run 1,0,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { false, { "1: Run", "1: Run" }, "run 1,1,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { false, { "1: Run", "2: Run" }, "run 1,0,0 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { false, { "1: Run", "2: RunReport" }, "run 1,0,1 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { false, { "1: RunReport" }, "run 1,1,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { false, { "1: RunReport", "0: Run" }, "run 0,1,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { false, { "1: RunReport", "0: RunReport" }, "run 1,1,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { false, { "1: RunReport", "1: Run" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { false, { "1: RunReport", "1: RunReport" }, "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0" },
      { false, { "1: RunReport", "2: Run" }, "run 1,1,0 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { false, { "1: RunReport", "2: RunReport" }, "run 1,1,1 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { false, { "2: Run" }, "run 1,1,0 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { false, { "2: Run", "0: Run" }, "run 0,1,0 report 0,1,0 ref 2,0,1 use 0,0,0" },
      { false, { "2: Run", "0: RunReport" }, "run 1,1,0 report 0,1,0 ref 2,0,1 use 0,0,0" },
      { false, { "2: Run", "1: Run" }, "run 1,0,0 report 1,0,0 ref 0,2,1 use 0,0,0" },
      { false, { "2: Run", "1: RunReport" }, "run 1,1,0 report 1,0,0 ref 0,2,1 use 0,0,0" },
      { false, { "2: Run", "2: Run" }, "run 1,1,1 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { false, { "2: RunReport" }, "run 1,1,1 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { false, { "2: RunReport", "0: Run" }, "run 0,1,1 report 0,1,0 ref 2,0,1 use 0,0,0" },
      { false, { "2: RunReport", "0: RunReport" }, "run 1,1,1 report 0,1,0 ref 2,0,1 use 0,0,0" },
      { false, { "2: RunReport", "1: Run" }, "run 1,0,1 report 1,0,0 ref 0,2,1 use 0,0,0" },
      { false, { "2: RunReport", "1: RunReport" }, "run 1,1,1 report 1,0,0 ref 0,2,1 use 0,0,0" },
      { false, { "2: RunReport", "2: Run" }, "run 1,1,0 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { false, { "2: RunReport", "2: RunReport" }, "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0" },
      { true, { "0: Run" }, "run 0,0,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { true, { "0: Run", "0: Run" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 0,0,1" },
      { true, { "0: Run", "1: Run" }, "run 0,1,1 report 0,1,1 ref 1,0,0 use 0,0,1" },
      { true, { "0: Run", "2: Run" }, "run 0,0,0 report 0,0,0 ref 2,1,3 use 0,0,0" },
      { true, { "0: Run", "2: RunReport" }, "run 0,0,1 report 0,0,0 ref 2,1,3 use 0,0,0" },
      { true, { "0: RunReport" }, "run 1,0,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { true, { "0: RunReport", "0: Run" }, "run 0,0,1 report 0,0,1 ref 2,1,0 use 0,0,0" },
      { true, { "0: RunReport", "0: RunReport" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 0,0,1" },
      { true, { "0: RunReport", "1: Run" }, "run 1,1,1 report 0,1,1 ref 1,0,0 use 0,0,1" },
      { true, { "0: RunReport", "2: Run" }, "run 1,0,0 report 0,0,0 ref 2,1,3 use 0,0,0" },
      { true, { "0: RunReport", "2: RunReport" }, "run 1,0,1 report 0,0,0 ref 2,1,3 use 0,0,0" },
      { true, { "1: Run" }, "run 1,1,1 report 1,1,1 ref 0,0,0 use 0,0,0" },
      { true, { "1: Run", "0: Run" }, "run 0,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { true, { "1: Run", "0: RunReport" }, "run 1,1,1 report 0,1,1 ref 1,0,0 use 0,1,1" },
      { true, { "1: Run", "1: Run" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { true, { "1: Run", "1: RunReport" }, "run 1,1,1 report 1,0,1 ref 0,1,0 use 1,0,1" },
      { true, { "1: Run", "2: Run" }, "run 1,1,0 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { true, { "1: Run", "2: RunReport" }, "run 1,1,1 report 1,1,0 ref 0,0,1 use 1,1,0" },
      { true, { "2: Run" }, "run 1,0,0 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { true, { "2: Run", "0: Run" }, "run 0,0,0 report 0,0,0 ref 3,1,2 use 0,0,0" },
      { true, { "2: Run", "0: RunReport" }, "run 1,0,0 report 0,0,0 ref 3,1,2 use 0,0,0" },
      { true, { "2: Run", "1: Run" }, "run 1,1,0 report 1,1,0 ref 0,0,1 use 1,0,0" },
      { true, { "2: Run", "2: Run" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 1,0,0" },
      { true, { "2: RunReport" }, "run 1,0,1 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { true, { "2: RunReport", "0: Run" }, "run 0,0,1 report 0,0,0 ref 3,1,2 use 0,0,0" },
      { true, { "2: RunReport", "0: RunReport" }, "run 1,0,1 report 0,0,0 ref 3,1,2 use 0,0,0" },
      { true, { "2: RunReport", "1: Run" }, "run 1,1,1 report 1,1,0 ref 0,0,1 use 1,0,0" },
      { true, { "2: RunReport", "2: Run" }, "run 1,0,0 report 1,0,0 ref 0,1,2 use 0,0,0" },
      { true, { "2: RunReport", "2: RunReport" }, "run 1,0,1 report 1,0,1 ref 0,1,0 use 1,0,0" },
   };
}
#endif
