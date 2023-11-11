//! \file us_report_gmp.h
#ifndef US_REPORTGMP_H
#define US_REPORTGMP_H

#include "us_extern.h"
#include "us_db2.h"



class US_UTIL_EXTERN US_ReportGMP
{
   public:

      //! \brief Generic constructor
      US_ReportGMP();

      //! A null destructor. 
      ~US_ReportGMP() {};

      struct ReportItem
      {
	//need also ID?
	QString type;
	QString method;
	double  range_low;
	double  range_high;
	double  integration_val;
	double  tolerance;
	double  total_percent;
	int     combined_plot;
	int     ind_combined_plot;
	
	//Integration results
	double  integration_val_sim;
	double  total_percent_sim;
	QString passed;
      };

      QVector< ReportItem > reportItems;

      //need also ID?
      
      //Channel name
      QString channel_name;

      //Main parameters set manually in reference reports
      double tot_conc;
      double tot_conc_tol;
      double rmsd_limit;
      double av_intensity;
      double wavelength;
      double experiment_duration;
      double experiment_duration_tol;

      //Bool flag for changed exp. time & if read form DB
      bool exp_time_changed;
      bool DBread;
      bool interf_report_changed;

      //report mask params
      bool tot_conc_mask;
      bool rmsd_limit_mask;
      bool av_intensity_mask;
      bool experiment_duration_mask;
      bool integration_results_mask;
      bool plots_mask;

      //Pseudo3D mask parameters
      bool pseudo3d_mask;
      bool pseudo3d_2dsait_s_ff0;
      bool pseudo3d_2dsait_s_d;
      bool pseudo3d_2dsait_mw_ff0;
      bool pseudo3d_2dsait_mw_d;
      bool pseudo3d_2dsamc_s_ff0;
      bool pseudo3d_2dsamc_s_d;
      bool pseudo3d_2dsamc_mw_ff0;
      bool pseudo3d_2dsamc_mw_d;
      bool pseudo3d_pcsa_s_ff0;
      bool pseudo3d_pcsa_s_d;
      bool pseudo3d_pcsa_mw_ff0;
      bool pseudo3d_pcsa_mw_d;

      
      //Rest of parameters followign from protocol: not used so far
      int     scan_number;
      double  load_volume;
      double  av_rotor_speed;
      QString instrument_used;
      QString technician;
      QString data_owner;
      double  av_temperature;
      QString rotor;
      QString calibration;

      QString score;

   private:
      
};
#endif

