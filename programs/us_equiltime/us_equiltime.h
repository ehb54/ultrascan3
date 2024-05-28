//! \file us_equiltime.h
//! \brief Contains the declaration of the US_EquilTime class and its members.
#ifndef US_EQUILTIME_H
#define US_EQUILTIME_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_editor.h"
#include "us_model.h"
#include "us_simparms.h"
#include "us_astfem_math.h"
#include "us_astfem_rsa.h"

#include "qwt_counter.h"

//! \class US_EquilTime
//! \brief A class for estimating equilibrium times in UltraScan.
class US_EquilTime : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EquilTime class.
        US_EquilTime();

    private:
        US_Model model; //!< Model for the simulation.
        US_SimulationParameters simparams; //!< Simulation parameters.
        US_DataIO::RawData astfem_data; //!< Raw data for ASTFEM.

        enum { PROLATE, OBLATE, ROD, SPHERE }; //!< Shape types.
        enum { INNER, OUTER, CENTER, CUSTOM }; //!< Position types.
        enum { SIGMA, RPM }; //!< Speed types.

        int current_shape; //!< Current shape type.
        int current_position; //!< Current position type.
        int speed_type; //!< Current speed type.

        double rpm_start; //!< Start RPM.
        double rpm_stop; //!< Stop RPM.
        double sigma_start; //!< Start sigma.
        double sigma_stop; //!< Stop sigma.
        int speed_count; //!< Speed count.
        int radius_points; //!< Number of radius points.

        QVector<double> sim_radius; //!< Simulation radius values.
        QVector<double> concentration; //!< Concentration values.

        double current_time; //!< Current time.
        double step_time; //!< Step time.
        double next_scan_time; //!< Next scan time.

        QList<double> speed_steps; //!< List of speed steps.

        US_Help showHelp; //!< Help object.
        QwtPlot* equilibrium_plot; //!< Plot for equilibrium.
        QwtPlotCurve* current_curve; //!< Curve for current data.
        US_Astfem_RSA* astfem_rsa; //!< ASTFEM RSA object.

        QLabel* lb_lowspeed; //!< Label for low speed.
        QLabel* lb_highspeed; //!< Label for high speed.

        QwtCounter* cnt_top; //!< Counter for top position.
        QwtCounter* cnt_bottom; //!< Counter for bottom position.
        QwtCounter* cnt_lowspeed; //!< Counter for low speed.
        QwtCounter* cnt_highspeed; //!< Counter for high speed.
        QwtCounter* cnt_speedsteps; //!< Counter for speed steps.
        QwtCounter* cnt_tolerance; //!< Counter for tolerance.
        QwtCounter* cnt_timeIncrement; //!< Counter for time increment.

        QTextEdit* te_speedlist; //!< Text edit for speed list.
        US_Editor* te_info; //!< Editor for information.

        QPushButton* pb_changeModel; //!< Button to change the model.
        QPushButton* pb_estimate; //!< Button to estimate equilibrium time.

        //! \brief Initialize simulation parameters.
        void init_simparams(void);

        //! \brief Initialize ASTFEM data.
        void init_astfem_data(void);

        //! \brief Convert sigma to RPM.
        //! \param sigma The sigma value.
        //! \return The corresponding RPM value.
        double rpmFromSigma(double sigma);

        //! \brief Convert RPM to sigma.
        //! \param rpm The RPM value.
        //! \return The corresponding sigma value.
        double sigmaFromRpm(double rpm);

    private slots:
        //! \brief Slot to update speeds.
        //! \param value The new speed value.
        void update_speeds(int value);

        //! \brief Slot to handle new channel selection.
        //! \param value The new channel value.
        void new_channel(int value);

        //! \brief Slot to update the low speed value.
        //! \param value The new low speed value.
        void new_lowspeed(double value);

        //! \brief Slot to update the high speed value.
        //! \param value The new high speed value.
        void new_highspeed(double value);

        //! \brief Slot to update the speed step value.
        //! \param value The new speed step value.
        void new_speedstep(double value);

        //! \brief Slot to perform the simulation.
        void simulate(void);

        //! \brief Slot to change the model.
        void change_model(void);

        //! \brief Slot to set the model.
        //! \param model The new model.
        void set_model(US_Model model);

        //! \brief Slot to check for equilibrium.
        //! \param radius Pointer to the radius values.
        //! \param conc Pointer to the concentration values.
        void check_equil(QVector<double>* radius, double* conc);

        //! \brief Slot to set the time increment.
        //! \param time The new time increment.
        void set_time(double time)
        {
            step_time = time;
        };

        //! \brief Slot to show help documentation.
        void help(void)
        {
            showHelp.show_help("manual/us_equiltime.html");
        };
};

#endif // US_EQUILTIME_H
