//! \file us_exclude_profile.h
//! \brief Contains the declaration of the US_ExcludeProfile class and its members.
#ifndef US_EXCLUDE_PROFILE_H
#define US_EXCLUDE_PROFILE_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

#include <qwt_counter.h>

#ifndef DbgLv
//! \def DbgLv(a)
//! \brief Macro for debug level logging.
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

//! \class US_ExcludeProfile
//! \brief A class for managing exclusion profiles in UltraScan.
class US_ExcludeProfile : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_ExcludeProfile class.
        //! \param exclusions The list of initial exclusions.
        US_ExcludeProfile(QList<int> exclusions);

        signals:
                //! \brief Signal emitted to update the exclusion profile.
                //! \param exclusions The updated list of exclusions.
                void update_exclude_profile(QList<int> exclusions);

        //! \brief Signal emitted when the exclusion profile is finished.
        //! \param exclusions The final list of exclusions.
        void finish_exclude_profile(QList<int> exclusions);

        //! \brief Signal emitted to cancel the exclusion profile.
        void cancel_exclude_profile(void);

    private:
        QwtCounter* ct_start; //!< Counter for the start value.
        QwtCounter* ct_stop; //!< Counter for the stop value.
        QwtCounter* ct_nth; //!< Counter for the nth value.

        QLineEdit* le_remaining; //!< Line edit for remaining scans.
        QLineEdit* le_excluded; //!< Line edit for excluded scans.

        int dbg_level; //!< Debug level.
        bool finished; //!< Flag to indicate if the profile is finished.
        QList<int> original; //!< Original list of exclusions.
        QList<int> excludes; //!< List of current exclusions.
        QList<int> current; //!< List of current selections.

        US_Help showHelp; //!< Help object.

    private slots:
        //! \brief Slot to update the exclusion profile.
        //! \param value The value to update (default is 0.0).
        void update(double value = 0.0);

        //! \brief Slot to update the start value.
        //! \param value The new start value.
        void update_start(double value);

        //! \brief Slot to update the stop value.
        //! \param value The new stop value.
        void update_stop(double value);

        //! \brief Slot to terminate the exclusion profile.
        void terminate(void);

        //! \brief Slot to apply the exclusion profile.
        void apply(void);

        //! \brief Slot to mark the profile as done.
        void done(void);

        //! \brief Slot to reset the exclusion profile.
        void reset(void);

        //! \brief Slot to show the help documentation.
        void help(void)
        {
            showHelp.show_help("manual/edit_exclude_profile.html");
        }
};

#endif // US_EXCLUDE_PROFILE_H
