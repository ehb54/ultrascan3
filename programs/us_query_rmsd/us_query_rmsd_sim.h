#ifndef US_QUERY_RMSD_SIM_H
#define US_QUERY_RMSD_SIM_H

#include "../us_fematch/us_fematch.h"

class US_QueryRmsdSim : public US_FeMatch
{
   Q_OBJECT
   public:
   explicit US_QueryRmsdSim();

   void set_raw_edit_data(US_DataIO::RawData, US_DataIO::EditedData);


   signals:
};

#endif // US_QUERY_RMSD_SIM_H
