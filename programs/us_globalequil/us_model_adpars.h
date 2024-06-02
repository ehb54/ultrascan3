//! \file us_model_adpars.h
//! \brief Defines the US_ModelAdPars class for additional model parameters in global equilibrium analysis.
#ifndef US_MODELADPARS_H
#define US_MODELADPARS_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! \class US_ModelAdPars
//! \brief A class to handle additional model parameters for global equilibrium analysis.
class US_ModelAdPars : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_ModelAdPars.
        //! \param napars The number of additional parameters.
        //! \param aud_pars The list of additional user-defined parameters.
        US_ModelAdPars( int napars, QList< double >& aud_pars );

    private:
        int                napars;      //!< Number of additional parameters
        QList< double >&   aud_pars;    //!< Additional user-defined parameters

        QLineEdit*         le_stoich1;  //!< LineEdit for Stoichiometry 1
        QLineEdit*         le_stoich2;  //!< LineEdit for Stoichiometry 2
        QLineEdit*         le_nmwslot;  //!< LineEdit for Number of MW Slots
        QLineEdit*         le_llimmwr;  //!< LineEdit for Lower Limit MW range
        QLineEdit*         le_ulimmwr;  //!< LineEdit for Upper Limit MW range
        QLineEdit*         le_cmnvbar;  //!< LineEdit for Common vbar

        QCheckBox*         ck_cmnvbar;  //!< CheckBox for using common vbar

        US_Help            showHelp;    //!< Help system

    private slots:
        //! \brief Slot to handle the selection of parameters.
        void selected          ( void );

        //! \brief Slot to handle the cancellation of parameter selection.
        void cancelled         ( void );

        //! \brief Slot to show help documentation.
        void help              ( void )
        { showHelp.show_help( "global_equil-modapar.html" ); };
};

#endif
