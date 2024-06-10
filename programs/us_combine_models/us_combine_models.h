//! \file us_combmodel.h
//! \brief Contains the declaration of the US_CombineModels class and its members.
#ifndef US_COMBMODEL_H
#define US_COMBMODEL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_help.h"
#include "us_model.h"
#include "us_settings.h"

#ifndef DbgLv
//! \def DbgLv(a)
//! \brief Macro for debug level logging.
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_CombineModels
//! \brief A class for combining models in UltraScan.
class US_CombineModels : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_CombineModels class.
        US_CombineModels();

    private:
        QList<US_Model> models; //!< List of selected models.
        QStringList mdescs; //!< List of descriptions of models.
        US_Help showHelp; //!< Help object.
        US_DB2* dbP; //!< Pointer to the database object.

        QPushButton* pb_prefilt; //!< Button to apply pre-filter.
        QPushButton* pb_add; //!< Button to add models.
        QPushButton* pb_reset; //!< Button to reset the selection.
        QPushButton* pb_help; //!< Button to show help.
        QPushButton* pb_close; //!< Button to close the dialog.
        QPushButton* pb_save; //!< Button to save the combined model.

        QLineEdit* le_prefilt; //!< Line edit for pre-filter.

        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk/DB controls.

        QListWidget* lw_models; //!< List widget to display models.

        int ntrows; //!< Number of rows.
        int dbg_level; //!< Debug level.

        bool changed; //!< Flag to indicate changes.
        bool runsel; //!< Flag to indicate if a run is selected.
        bool latest; //!< Flag to indicate if the latest model is selected.

        QString mfilter; //!< Filter for models.
        QString run_name; //!< Name of the run.
        QString cmodel_name; //!< Name of the combined model.

        QStringList pfilts; //!< List of pre-filters.

    private slots:
        //! \brief Slot to add models.
        void add_models();

        //! \brief Slot to reset the selection.
        void reset();

        //! \brief Slot to save the combined model.
        void save();

        //! \brief Slot to update disk/DB selection.
        //! \param disk True if disk is selected, false if DB is selected.
        void update_disk_db(bool disk);

        //! \brief Slot to select the filter.
        void select_filt();

        //! \brief Slot to show help.
        void help()
        {
            showHelp.show_help("combine_models.html");
        };
};

#endif // US_COMBMODEL_H