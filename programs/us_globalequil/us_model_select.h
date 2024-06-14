//! \file us_model_select.h
//! \brief Defines the US_ModelSelect class for selecting models in global equilibrium analysis.

#ifndef US_MODELSELECT_H
#define US_MODELSELECT_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! \class US_ModelSelect
//! \brief A class to handle the selection of models for global equilibrium analysis.
class US_ModelSelect : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_ModelSelect.
        //! \param smodelx Reference to the index of the selected model.
        //! \param models Reference to the list of all possible models.
        //! \param aud_pars Reference to the list of additional user-defined parameters.
        US_ModelSelect( int& smodelx, QStringList& models, QList< double >& aud_pars );

    private:
        int&               smodelx;     //!< Index of the selected model
        QStringList&       models;      //!< List of all possible models
        QList< double >&   aud_pars;    //!< Additional user-defined parameters

        QListWidget*       lw_models;   //!< List widget for displaying models

        QString            model_sel;   //!< Selected model string

        US_Help            showHelp;    //!< Help system

    private slots:
        //! \brief Slot to handle the selection of a model.
        void selected          ( void );

        //! \brief Slot to handle the cancellation of model selection.
        void cancelled         ( void );

        //! \brief Slot to handle the selection of model parameters for model type 0.
        void model_pars_0      ( void );

        //! \brief Slot to handle the selection of model parameters for model type 1.
        void model_pars_1      ( void );

        //! \brief Slot to handle the selection of model parameters for model type 2.
        void model_pars_2      ( void );

        //! \brief Slot to handle the selection of model parameters for model type 4.
        void model_pars_4      ( void );

        //! \brief Slot to display the function dialog.
        void function_dialog   ( void );

        //! \brief Creates a function equation string from a list of components.
        //! \param components List of function components.
        //! \return The function equation as a QString.
        QString function_equation  ( QStringList& components );

        //! \brief Creates a function components string from a list of components and a count.
        //! \param components List of function components.
        //! \param count Reference to an integer to store the count of components.
        //! \return The function components as a QString.
        QString function_components( QStringList& components, int& count );

        //! \brief Slot to show help documentation.
        void help              ( void )
        { showHelp.show_help("global_equil-modsel.html"); };
};

#endif
