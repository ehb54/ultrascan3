//! \file us_astfem_sim.h
#ifndef US_ASTFEM_SIM_H
#define US_ASTFEM_SIM_H

#include <qwt_counter.h>
#include <qwt_plot.h>
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"
#include "us_buffer.h"
#include "us_dataIO.h"
#include "us_help.h"
#include "us_lamm_astfvm.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_rotor_gui.h"
#include "us_simparms.h"
#include "us_widgets.h"

#ifndef DbgLv
#define DbgLv(a) \
   if (dbg_level >= a) \
   qDebug()
#endif

//! \brief Main window to control and display an ultracentrifugation
//!        simulation

class US_Astfem_Sim : public US_Widgets {
      Q_OBJECT

   public:
      //! \param p Parent widget, normally not specified
      //! \param f Window flags, normally not specified
      US_Astfem_Sim(QWidget *p = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

      //! Initialize the us_astfem_sim with command line arguments
      //! \param flags Command line arguments
      //! \returns int: 0 on success no gui needed, 1 on success but gui needed, -1 on error
      int init_from_args(const QMap<QString, QString> &flags);

      //! Write a timestate file based on auc data
      //! \attention Not implemented
      //! \param dir Location to save timestate to
      //! \param data Data used to construct the timestate from
      //! \return status code
      int writetimestate(const QString &dir, US_DataIO::RawData &data);

   signals:
      void new_time(double);

   private:
      bool stopFlag; //!< Flag to stop the simulation
      bool movieFlag; //!< Flag to show a movie during simulation
      bool save_movie; //!< Flag to save the generated frames
      bool time_correctionFlag; //!< Flag to apply time correction - Currently not implemented/connected
      double total_conc; //!< Sum of all components concentration
      double meniscus_ar; //!< Meniscus position at rest in cm
      double times_comp; //!< Current time of the simulation
      int icomponent; //!< Index of the component, which is currently simulated
      int ncomponent; //!< Number of components
      int curve_count; //!< Number of currently simulated scans
      int image_count; //!< Current number of saved movie frames
      int dbg_level; //!< Debug level
      QString imagedir; //!< Path to the image dir
      QString imageName; //!< Full path template for movie frames ({imagedir}/frame{image_count}.png)
      QString tmst_tfpath; //!< Path to the timestate of the simulation in a temporary location

      QCheckBox *ck_movie; //!< Pointer to QCheckbox for movie display
      QCheckBox *ck_savemovie; //!< Pointer to QCheckbox for saving movie frames
      QCheckBox *ck_timeCorr; //!< Pointer to QCheckbox for applying time correction
      QPushButton *pb_saveSim; //!< Pointer to "Save Simulation" button
      QPushButton *pb_buffer; //!< Pointer to "Define Buffer" button
      QPushButton *pb_simParms; //!< Pointer to "Simulation Parameters" button
      QPushButton *pb_rotor; //!< Pointer to "Select Rotor" button
      QPushButton *pb_changeModel; //!< Pointer to "Model Control" button
      QPushButton *pb_start; //!< Pointer to "Start Simulation" button
      QPushButton *pb_stop; //!< Pointer to "Stop Simulation" button

      QTextEdit *te_status; //!< Pointer to status text box

      QLabel *lb_component; //!< Pointer to "Component" label
      QLabel *lb_progress; //!< Pointer to "Progress" label

      QLCDNumber *lcd_time; //!< Pointer to time lcd
      QLCDNumber *lcd_speed; //!< Pointer to speed lcd
      QLCDNumber *lcd_component; //!< Pointer to component lcd
      QProgressBar *progress; //!< Pointer to progress bar

      QwtPlot *moviePlot; //!< Pointer to upper plot QwtWidget
      QwtPlot *scanPlot; //!< Pointer to lower plot QwtWidget

      US_Plot *plot1; //!< Pointer to upper plot
      US_Plot *plot2; //!< Pointer to lower plot

      US_Help showhelp; //!< Help button

      QString progress_text; //!< Current progress text
      int progress_value; //!< Current progress value
      int progress_maximum; //!< Progress maximum value

      US_Astfem_RSA *astfem; //!< Pointer to the astfem simulation engine
      US_LammAstfvm *astfvm; //!< Pointer to the astfvm simulation engine
      US_Model system; //!< Model object
      US_Buffer buffer; //!< Buffer object
      US_Rotor::Rotor rotor; //!< Rotor object
      US_Rotor::RotorCalibration rotor_calibration; //!< Rotor calibration object
      US_SimulationParameters simparams; //!< Simulation parameters
      US_AstfemMath::AstFemParameters af_params; //!< Fixed grid simulation parameters used for exporting
      QVector<US_DataIO::RawData> sim_datas; //!< Vector of simulated data for each speedstep
      US_DataIO::RawData sim_data_all; //!< Overall simulated data

      void init_simparams(void);
      void adjust_limits(double);
      double stretch(double *, double);
      void save_xla(const QString &, US_DataIO::RawData, int, bool supress_dialog = false);
      void save_ultrascan(const QString &);
      void finish(void);
      void ri_noise(void);
      void baseline(void);
      void random_noise(void);
      void ti_noise(void);
      void plot(int);
      // debug
      void dump_system(void);
      void dump_simparms(void);
      void dump_astfem_data(void);
      void dump_simComponent(US_Model::SimulationComponent &);
      void dump_association(US_Model::Association &);
      void dump_mfem_initial(US_Model::MfemInitial &);
      void dump_ss(US_SimulationParameters::SpeedProfile &);
      void dump_mfem_scan(US_DataIO::Scan &);
      bool save_simulation(QString odir, bool supress_dialog = false);

   private slots:

      void new_model(void);
      void change_model(US_Model);
      void new_buffer(void);
      void change_buffer(US_Buffer);
      void change_status(void);
      void set_parameters(void);
      void sim_parameters(void);
      void select_rotor(void);
      void assignRotor(US_Rotor::Rotor &, US_Rotor::RotorCalibration &);
      void start_simulation(void);
      void stop_simulation(void);
      void save_scans(void);
      void update_progress(int);
      void update_component(int);
      void start_calc(int);
      void show_progress(int);
      void calc_over(void);

      void update_movie_plot(QVector<double> *, double *);
      void update_save_movie(bool);
      void update_time(double time);

      void update_speed(int speed) { lcd_speed->display(( int ) speed); };

      void help(void) { showhelp.show_help("manual/astfem_sim.html"); };

      void update_time_corr(void) { time_correctionFlag = ck_timeCorr->isChecked(); };
};
#endif
