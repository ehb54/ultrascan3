//! \file us_abde_main.h
//! \brief Contains the declaration of the US_ABDE_MW class and its members.
#ifndef US_ABDE_MAIN_H
#define US_ABDE_MAIN_H

#include <us_widgets.h>
#include "../us_buoyancy/us_buoyancy.h"
#include "us_norm_profile.h"

//! \class US_ABDE_MW
//! \brief A class for managing the ABDE (Analysis by Density Estimation) main window.
class US_ABDE_MW : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_ABDE_MW class.
        US_ABDE_MW();

    private slots:
        //! \brief Slot to select the fitter.
        void sel_fitter(void);

        //! \brief Slot to select the normalizer.
        void sel_normalizer(void);

        //! \brief Slot to turn on a QPushButton.
        //! \param button The button to be turned on.
        //! \param on The state to set the button (true for on, false for off).
        void turn_on(QPushButton* button, bool on);

    private:
        US_Norm_Profile* normalizer; //!< Pointer to the normalizer object.
        US_Buoyancy* fitter; //!< Pointer to the fitter object.
        QPushButton* pb_fitter; //!< Button to select the fitter.
        QPushButton* pb_normalizer; //!< Button to select the normalizer.
        QVBoxLayout* main_lyt; //!< Main layout for the window.
        int prog_state; //!< Program state indicator.
        QLabel* hline; //!< Horizontal line label.
};

#endif // US_ABDE_MAIN_H

