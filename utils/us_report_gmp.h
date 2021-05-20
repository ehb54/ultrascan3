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

      
    private:
      
};
#endif

