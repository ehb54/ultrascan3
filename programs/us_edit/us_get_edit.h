//! \file us_get_edit.h
//! \brief Contains the declaration of the US_GetEdit class and its members.
#ifndef US_GET_EDIT_H
#define US_GET_EDIT_H

#include "us_extern.h"
#include "us_widgets_dialog.h"

//! \class US_GetEdit
//! \brief A class for selecting an edit profile in UltraScan.
class US_GetEdit : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_GetEdit class.
        //! \param editIndex Reference to the index of the selected edit.
        //! \param editList List of edit profiles.
        US_GetEdit(int& editIndex, QStringList& editList);

    private:
        QListWidget* lw; //!< List widget to display edit profiles.
        int& index; //!< Reference to the index of the selected edit.

    private slots:
        //! \brief Slot to handle the selection of an edit profile.
        void select(void);
};

#endif // US_GET_EDIT_H

