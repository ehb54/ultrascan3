//! \file us_simparms.cpp

#include "us_simparms.h"

US_SimulationParameters::US_SimulationParameters()
{
   mesh_radius.clear(); 
   speed_step.clear();

   simpoints         = 1400;  
   meshType          = ADAPTIVE;
   gridType          = MOVING;
   radial_resolution = 0.001;
   meniscus          = 5.8;
   bottom            = 7.2;
   rnoise            = 0.0;  
   tinoise           = 0.0;      
   rinoise           = 0.0;     
   rotor             = 0;      
   band_forming      = false;   
   band_volume       = 15.0;   
}

US_SimulationParameters::SpeedProfile::SpeedProfile()
{
   duration_hours    = 5;
   duration_minutes  = 0;
   delay_hours       = 0;
   delay_minutes     = 0;
   scans             = 30;
   acceleration      = 400;
   rotorspeed        = 50000;
   acceleration_flag = true;
}
