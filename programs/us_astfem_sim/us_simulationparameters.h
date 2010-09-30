//! \file us_simulationparameters.h
#ifndef US_SIMULATIONPARAMETERS_GUI_H
#define US_SIMULATIONPARAMETERS_GUI_H

#include <QtGui>

#include <qwt_counter.h>

#include "us_widgets_dialog.h"
#include "us_simparms.h"
#include "us_help.h"

//! \brief A window for editing simulation parameters
class US_SimulationParametersGui : public US_WidgetsDialog
{
	Q_OBJECT
	public:
      //! \param params - Location for simulation parameters to be updated
      //! \param p      - Parent widget, normally not specified
      //! \param f      - Window flags, normally not specified
		US_SimulationParametersGui( US_SimulationParameters& );

   signals:
      void complete( void );

	private:

		US_SimulationParameters& simparams;
		US_SimulationParameters  simparams_backup;
      
      US_Help       showhelp;
                   
      int           current_speed_step;
                   
      QComboBox*    cmb_speeds;
      QComboBox*    cmb_moving;
      QComboBox*    cmb_mesh;  
                   
      QwtCounter*   cnt_speeds;
      QwtCounter*   cnt_duration_hours;
      QwtCounter*   cnt_duration_mins;
      QwtCounter*   cnt_delay_hours;
      QwtCounter*   cnt_delay_mins;
      QwtCounter*   cnt_rotorspeed;
      QwtCounter*   cnt_acceleration;
      QwtCounter*   cnt_scans;
      QwtCounter*   cnt_selected_speed;
      QwtCounter*   cnt_lamella;
      QwtCounter*   cnt_meniscus;
      QwtCounter*   cnt_bottom;
      QwtCounter*   cnt_simpoints;
      QwtCounter*   cnt_radial_res;
      QwtCounter*   cnt_rnoise;
      QwtCounter*   cnt_tinoise;
      QwtCounter*   cnt_rinoise;
                   
      QCheckBox*    cb_acceleration_flag;
      
      QRadioButton* rb_band;
      QRadioButton* rb_standard;

      void update_combobox( void );
      void backup_parms   ( void );
      void check_delay    ( void );

   private slots:
      void update_duration_hours( double );
      void update_duration_mins ( double );
      void update_delay_hours   ( double );
      void update_delay_mins    ( double );
      void update_rotorspeed    ( double );
      void acceleration_flag    ( void   );
      void update_speeds        ( double );
      void update_acceleration  ( double );
      void update_scans         ( double );
      void update_mesh          ( int    );
      void select_speed_profile ( int    );
      void update_speed_profile ( double );
      void load                 ( void   );
      void save                 ( void   );
      void accepted             ( void   );
      void revert               ( void   );
      void disconnect_all       ( void   );
      void reconnect_all        ( void   );

      void update_lamella       ( double lamella )
         { simparams.band_volume = lamella; };

      void update_meniscus      ( double meniscus )
         { simparams.meniscus    = meniscus; };

      void update_bottom        ( double bottom )
         { simparams.bottom      = bottom; };

      void update_simpoints     ( double simpoints )
         { simparams.simpoints   = (int) simpoints; };
 
      void update_radial_res    ( double radial_res )
         { simparams.radial_resolution = radial_res; };

      void update_rnoise        ( double rnoise )
         { simparams.rnoise      = rnoise; };

      void update_tinoise       ( double tinoise )
         { simparams.tinoise     = tinoise; };

      void update_rinoise       ( double rinoise )
         { simparams.rinoise     = rinoise; };
      
      void update_moving        ( int grid )
         { simparams.gridType    = (US_SimulationParameters::GridType) grid; };

      void select_centerpiece   ( bool )
         { simparams.band_forming = rb_band->isChecked(); };

      void help                 ( void )
         { showhelp.show_help( "manual/simulation_parameters.html" ); };
};
#endif

