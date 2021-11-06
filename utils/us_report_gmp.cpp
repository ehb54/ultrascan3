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
  
  initItem.type             = QString("s");
  initItem.method           = QString("2DSA-IT");
  initItem.range_low        = 3.2;
  initItem.range_high       = 3.7;
  initItem.integration_val  = 0.57;
  initItem.tolerance        = 10; 
  initItem.total_percent    = 95;

  reportItems.push_back( initItem );

  //Channel name
  channel_name = QString("1A:UV/vis.:(unspecified)");

  //Main params
  tot_conc     = 0.6;
  rmsd_limit   = 0.002;
  av_intensity = 10000;

  wavelength   = 180;
  experiment_duration = 19800;

  tot_conc_tol = 10;
  experiment_duration_tol = 10;

  exp_time_changed = false;
  DBread = false;
  interf_report_changed = false;
}
 
