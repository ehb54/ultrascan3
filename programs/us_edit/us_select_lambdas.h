//! \file us_sel_lambdas.h
//! \brief Contains the declaration of the US_SelectLambdas class and its members.
#ifndef US_SEL_LAMBDAS_H
#define US_SEL_LAMBDAS_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

#ifndef DbgLv
//! \def DbgLv(a)
//! \brief Macro for debug level logging.
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_SelectLambdas
//! \brief A class for selecting lambda (wavelength) values in UltraScan.
class US_SelectLambdas : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_SelectLambdas class.
        //! \param lambdas The vector of lambda values.
        US_SelectLambdas(QVector<int> lambdas);

        signals:
                //! \brief Signal emitted with the new list of selected lambdas.
                //! \param lambdas The new list of selected lambda values.
                void new_lambda_list(QVector<int> lambdas);

    private:
        QLineEdit* le_original; //!< Line edit for original lambdas.
        QLineEdit* le_selected; //!< Line edit for selected lambdas.

        QListWidget* lw_original; //!< List widget for original lambdas.
        QListWidget* lw_selected; //!< List widget for selected lambdas.

        QPushButton* pb_add; //!< Button to add lambdas.
        QPushButton* pb_remove; //!< Button to remove lambdas.
        QPushButton* pb_accept; //!< Button to accept the selection.

        int dbg_level; //!< Debug level.
        int nbr_orig; //!< Number of original lambdas.
        int nbr_select; //!< Number of selected lambdas.

        QVector<int> original; //!< Vector of original lambdas.
        QVector<int> selected; //!< Vector of selected lambdas.

        US_Help showHelp; //!< Help object.

    private slots:
        //! \brief Slot to add selected lambdas to the selection.
        void add_selections(void);

        //! \brief Slot to remove selected lambdas from the selection.
        void rmv_selections(void);

        //! \brief Slot to cancel the selection.
        void cancel(void);

        //! \brief Slot to mark the selection as done.
        void done(void);

        //! \brief Slot to reset the selection.
        void reset(void);

        //! \brief Slot to show the help documentation.
        void help(void)
        {
            showHelp.show_help("manual/edit_select_lambdas.html");
        }
};

#endif // US_SEL_LAMBDAS_H
