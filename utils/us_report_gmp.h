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
	QString type;
	QString method;
	double  range_low;
	double  range_high;
	double  integration_val;
	double  tolerance;
	double  total_percent;
      };

      QVector< ReportItem > reportItems;

      //Channel name
      QString channel_name;

      //Main parameters set manually in reference reports
      double tot_conc;
      double rmsd_limit;
      double av_intensity;
      double wavelength;
      double experiment_duration;

      //Rest of parameters followign from protocol
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

