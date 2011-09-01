/*****************************************************
 *                                                   *
 * US Dynamic Light Scattering                       *
 *                                                   *
 * This file contains the declaration of a single    *
 * measurement class.                                *
 *                                                   *
 ****************************************************/

// $Rev$
// $LastChangeDate$
// $Author$

#include<iostreams>
#include<vector>
#include<string> // TODO: probably to be replaced by Qt-string

#infdef US_DLS_MEASUREMENT
#define US_DLS_MEASUREMENT

namespace us::dls {
  /*!
    Single DLS Measurement
  */
  class DLS_Measurement 
  {  
     //! overloading the << operator for shell output 
     friend std::ostream& operator<< (std::ostream& out, const DLS_Measurement& meas);  
     
     protected:
       //! every DLS_Measurement instance will get a unique id
       static unsigned int next_id;
     
     public:
       unsigned int id;
       DLS_Measurement();
       ~DLS_Measurement();

       DLS_Measurement& operator=(const DLS_Measurement &d)
       {
         id   = d.id;
         time = d.time;
         unit = d.unit;
         corr = d.corr;

         return *this;
       }

       // TODO: implement operator[] for time and corr

     private:
       //! measured time values 
       std::vector<double> time;
       //! time unit
       std::string unit;
       //! measured correlation values
       std::vector<double> corr;
  } // end of DLS_Measurement declaration

} // namespace us::dls

#endif // ~US_DLS_MEASUREMENT
