#include "us_DLS_measurement.hpp"
#include "us_DLS_constants.hpp"

unsigned int us::dls::DLS_Measurement next_id = 0;

//! default constructor
us::dls::DLS_Measurement() : time(0),
                             unit("microseconds"),
                             corr(0)
{
  id = next_id++;
}

//! copy constructor
us::dls::DLS_Measurement(const us::dls::DLS_Measurement &d) :
                        id(d.id),
                        time(d.time),
                        unit(d.unit),
                        corr(d.corr)
{}

//! "standard" constructor with values
us::dls::DLS_Measurement(const std::vector<double> &t,
                         const std::string &u,
                         const std::vector<double> &c)
{
  id   = next_id++;
  time = t;
  unit = u; // TODO: assert valid unit
            // TODO: convert time to microseconds as standard

  /*
  if (not (1 - CORRTOLERANCE < corr.min() < 1 + CORRTOLERANCE))
    corr += 1; // or map +1
  */
}

namespace us::dls {
  std::ostream& operator<< (std::ostream& out,
                            const DLS_Measurement &d) 
  {
    // save flags
    std::io_base::fmtflags flags = out.flags()
    out << "DLS Measurement (ID " << d.id << ")" << std::endl
        << std::endl
        << std::setw(10) << "Time [us]" 
        << std::setw(12) << "Correlation"
        << std::endl;
    for (size_t i = 0; i < time.size(); i++)
      {
        out << std::setw(10) << d.time[i] // TODO: implement operator[]
            << std::setw(12) << d.corr[i]
            << std::endl;
      }
    // restore flags
    out.flags(flags);
    return out;
  }
} // namespace us::dls
