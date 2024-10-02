//! \file us_rasmol_ctrl.h
#ifndef US_RASMOL_CTRL_H
#define US_RASMOL_CTRL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_window_message.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_plot.h"

#define DbgLv(a) if(dbg_level>=a)qDebug()  //!< dbg_level-conditioned qDebug()

/**
 * @class US_RasmolControl
 * @brief The US_RasmolControl class provides a user interface to control Rasmol visualization.
 */
class US_RasmolControl : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_RasmolControl.
         */
        US_RasmolControl();

    private:
        US_WindowMessage*    winmsgs;       //!< Window messages

        QComboBox*           cb_intname;    //!< Combo box for interpreter names
        QComboBox*           cb_commcmd;    //!< Combo box for command commands

        QTextEdit*           te_status;     //!< Text edit for status

        QLineEdit*           le_sendcmd;    //!< Line edit for sending commands

        QPushButton*         pb_listints;   //!< Button to list interpreters
        QPushButton*         pb_close;      //!< Close button
        QPushButton*         pb_sendcmd;    //!< Button to send commands

        int                  dbg_level;     //!< Debug level

    private slots:
                /**
                 * @brief Slot to list interpreters.
                 */
                void list_interps(void);

        /**
         * @brief Slot to send a command.
         */
        void send_command(void);

        /**
         * @brief Slot to choose a command from the combo box.
         * @param command The command chosen
         */
        void choose_command(const QString& command);
};

#endif // US_RASMOL_CTRL_H
