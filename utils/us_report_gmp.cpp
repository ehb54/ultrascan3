//! \file us_report_gmp.cpp

#include <QtCore>

#include "us_settings.h"
#include "us_db2.h"
#include "us_util.h"
#include "us_report_gmp.h"

// The constructor clears out the data structure
US_ReportGMP::US_ReportGMP()
{
  reportItems.clear();

  ReportItem initItem;
  
  initItem.type             = QString("S");
  initItem.method           = QString("2DSA-IT");
  initItem.range_low        = 3.2;
  initItem.range_high       = 3.7;
  initItem.integration_val  = 0.57;
  initItem.tolerance        = 10; 
  initItem.total_percent    = 0.58;

  reportItems.push_back( initItem );

  //Main params
  tot_conc     = 5;
  rmsd_limit   = 0.001;
  av_intensity = 10;
}
 
