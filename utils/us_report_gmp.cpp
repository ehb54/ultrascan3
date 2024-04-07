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
  
  initItem.type              = QString("s");
  initItem.method            = QString("2DSA-IT");
  initItem.range_low         = 3.2;
  initItem.range_high        = 3.7;
  initItem.integration_val   = 0.57;
  initItem.tolerance         = 10; 
  initItem.total_percent     = 95;
  initItem.combined_plot     = 1;
  initItem.ind_combined_plot = 1;

  initItem.integration_val_sim   = -1;
  initItem.total_percent_sim     = -1;
  initItem.passed                = QString("N/A");
  
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
  report_changed = false;

  //report mask params
  tot_conc_mask             = true;
  rmsd_limit_mask           = true;
  av_intensity_mask         = true;
  experiment_duration_mask  = true;
  integration_results_mask  = true;
  plots_mask                = true;

  //Pseudo3D mask parameters
  pseudo3d_mask             = true;
  
  pseudo3d_2dsait_s_ff0     = true;
  pseudo3d_2dsait_s_d       = true;
  pseudo3d_2dsait_mw_ff0    = true;
  pseudo3d_2dsait_mw_d      = true;
  pseudo3d_2dsamc_s_ff0     = true;
  pseudo3d_2dsamc_s_d       = true;
  pseudo3d_2dsamc_mw_ff0    = true;
  pseudo3d_2dsamc_mw_d      = true;
  pseudo3d_pcsa_s_ff0       = true;
  pseudo3d_pcsa_s_d         = true;
  pseudo3d_pcsa_mw_ff0      = true;
  pseudo3d_pcsa_mw_d        = true;

  //integration results
  

}
 
