//! \file us_sim_params_gui.h
#ifndef US_SIMPARAMS_GUI_H
#define US_SIMPARAMS_GUI_H

#include "us_extern.h"
#include "us_help.h"
#include "us_simparms.h"
#include "us_widgets_dialog.h"

#include <qwt_counter.h>

//! \brief A window for editing simulation parameters
class US_GUI_EXTERN US_SimParamsGui : public US_WidgetsDialog {
      Q_OBJECT
   public:
      //! \param params Location for simulation parameters to be updated
      //! \param load_file File to load simulation parameters from
      US_SimParamsGui(US_SimulationParameters &params);

      bool load_params(const QString &load_init, US_SimulationParameters &params);

   signals:
      void complete(void);

   private:
      QString load_init;
      US_SimulationParameters &simparams;
      US_SimulationParameters simparams_backup;

      US_Help showhelp;

      int current_speed_step;
      int dbg_level;

      QComboBox *cmb_speeds;
      QComboBox *cmb_moving;
      QComboBox *cmb_mesh;

      QwtCounter *cnt_speeds;
      QwtCounter *cnt_duration_hours;
      QwtCounter *cnt_duration_mins;
      QwtCounter *cnt_delay_hours;
      QwtCounter *cnt_delay_mins;
      QwtCounter *cnt_rotorspeed;
      QwtCounter *cnt_acceleration;
      QwtCounter *cnt_scans;
      QwtCounter *cnt_selected_speed;
      QwtCounter *cnt_lamella;
      QwtCounter *cnt_meniscus;
      QwtCounter *cnt_bottom;
      QwtCounter *cnt_simpoints;
      QwtCounter *cnt_radial_res;
      QwtCounter *cnt_rnoise;
      QwtCounter *cnt_lrnoise;
      QwtCounter *cnt_tinoise;
      QwtCounter *cnt_rinoise;
      QwtCounter *cnt_baseline;
      QwtCounter *cnt_temperature;

      QCheckBox *cb_acceleration_flag;

      QRadioButton *rb_band;
      QRadioButton *rb_standard;

      QLineEdit *le_status;

      void update_combobox(void);
      void backup_parms(void);
      void check_delay(void);

   private slots:
      void update_duration_hours(double);
      void update_duration_mins(double);
      void update_delay_hours(double);
      void update_delay_mins(double);
      void update_rotorspeed(double);
      void acceleration_flag(void);
      void update_speeds(double);
      void update_acceleration(double);
      void update_scans(double);
      void update_mesh(int);
      void select_speed_profile(int);
      void update_speed_profile(double);
      void load(void);
      void save(void);
      void accepted(void);
      void revert(void);
      void disconnect_all(void);
      void reconnect_all(void);
      void report_mods(void);

      void update_lamella(double lamella);
      void update_meniscus(double meniscus);
      void update_bottom(double bottom);
      void update_simpoints(double simpoints);
      void update_radial_res(double radial_res);
      void update_rnoise(double rnoise);
      void update_lrnoise(double lrnoise);
      void update_tinoise(double tinoise);
      void update_rinoise(double rinoise);
      void update_baseline(double baseline);
      void update_moving(int grid);
      void select_centerpiece(bool);
      void update_temp(double temp);

      void help(void) { showhelp.show_help("manual/simparams.html"); };
};
#endif
