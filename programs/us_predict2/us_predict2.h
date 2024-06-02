//! \file us_predict2.h
#ifndef US_PREDICT2_H
#define US_PREDICT2_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_math2.h"
#include "us_analyte.h"

//! \class US_Predict2
//! \brief Class for predicting molecular weight, sedimentation, and diffusion coefficients.
class US_Predict2 : public US_Widgets
{
	Q_OBJECT

    public:
        //! \brief Constructor for US_Predict2
        US_Predict2();

	private:

      enum { None, MW_SEDIMENTATION, MW_DIFFUSION, SED_DIFF } model;
      enum { PROLATE, OBLATE, ROD };

        int analyte_number;          //!< Analyte number
        double sed_coeff;            //!< Sedimentation coefficient
        double mw;                   //!< Molecular weight
        double diff_coeff;           //!< Diffusion coefficient
        double temperature;          //!< Temperature
        US_Help showHelp;            //!< Help dialog

        US_Math2::SolutionData d;    //!< Solution data

        QLabel* lb_param1;           //!< Label for parameter 1
        QLabel* lb_param2;           //!< Label for parameter 2
        QLabel* lb_param3;           //!< Label for parameter 3

        QLineEdit* le_density;       //!< Line edit for density
        QLineEdit* le_viscosity;     //!< Line edit for viscosity
        QLineEdit* le_vbar;          //!< Line edit for vbar

        QLineEdit* le_param1;        //!< Line edit for parameter 1
        QLineEdit* le_param2;        //!< Line edit for parameter 2
        QLineEdit* le_param3;        //!< Line edit for parameter 3

        QLineEdit* le_prolate_a;     //!< Line edit for prolate a
        QLineEdit* le_prolate_b;     //!< Line edit for prolate b
        QLineEdit* le_prolate_ab;    //!< Line edit for prolate ab

        QLineEdit* le_oblate_a;      //!< Line edit for oblate a
        QLineEdit* le_oblate_b;      //!< Line edit for oblate b
        QLineEdit* le_oblate_ab;     //!< Line edit for oblate ab

        QLineEdit* le_rod_a;         //!< Line edit for rod a
        QLineEdit* le_rod_b;         //!< Line edit for rod b
        QLineEdit* le_rod_ab;        //!< Line edit for rod ab

        QLineEdit* le_fCoef;         //!< Line edit for frictional coefficient
        QLineEdit* le_r0;            //!< Line edit for r0
        QLineEdit* le_f0;            //!< Line edit for f0
        QLineEdit* le_volume;        //!< Line edit for volume
        QLineEdit* le_ff0;           //!< Line edit for f/f0

        QPushButton* pb_mw_s;        //!< Button for MW and sedimentation calculation
        QPushButton* pb_mw_d;        //!< Button for MW and diffusion calculation
        QPushButton* pb_s_d;         //!< Button for sedimentation and diffusion calculation

        //! \brief Checks if the given value is valid
        //! \param value Value to check
        //! \return True if valid, false otherwise
        bool check_valid(double value);

        //! \brief Calculates the root of the given value
        //! \param order Order of the root
        //! \param value Value to calculate the root of
        //! \return Root of the value
        double root(int order, double value);

    public slots:
        //! \brief Updates the parameters
        void update(void);

        //! \brief Performs MW and sedimentation calculation
        void do_mw_s(void);

        //! \brief Performs MW and diffusion calculation
        void do_mw_d(void);

        //! \brief Performs sedimentation and diffusion calculation
        void do_s_d(void);

        //! \brief Updates parameter 1
        //! \param text New value for parameter 1
        void update_param1(const QString& text);

        //! \brief Updates parameter 2
        //! \param text New value for parameter 2
        void update_param2(const QString& text);

        //! \brief Updates the density
        //! \param text New value for density
        void density(const QString& text);

        //! \brief Updates the viscosity
        //! \param text New value for viscosity
        void viscosity(const QString& text);

        //! \brief Updates the vbar
        //! \param text New value for vbar
        void vbar(const QString& text);

        //! \brief Updates the temperature
        //! \param text New value for temperature in degrees Celsius
        void degC(const QString& text);

        //! \brief Gets the buffer data
        void get_buffer(void);

        //! \brief Gets the solution data
        void get_solution(void);

        //! \brief Gets the peptide data
        void get_peptide(void);

        //! \brief Updates the buffer parameters
        //! \param density New density value
        //! \param viscosity New viscosity value
        void update_buffer(double density, double viscosity);

        //! \brief Updates the vbar parameter
        //! \param analyte Analyte data
        void update_vbar(US_Analyte analyte);

        //! \brief Updates the solution parameters
        //! \param solution Solution data
        void update_solution(US_Solution solution);

        //! \brief Chooses the calculation model
        //! \param model Model type
        void choose(int model);

        //! \brief Shows help information
        void help(void)
        { showHelp.show_help("manual/predict2.html"); };
};

#endif // US_PREDICT2_H