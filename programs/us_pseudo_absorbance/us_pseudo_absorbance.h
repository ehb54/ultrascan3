//! \file us_pseudo_absorbance.h
#ifndef US_PSEUDO_ABSORBANCE_H
#define US_PSEUDO_ABSORBANCE_H

#include <us_widgets.h>
#include "us_add_refScan.h"
#include "us_convert_scan.h"
#include "us_remove_ri.h"

/**
 * @class US_PseudoAbsorbance
 * @brief The US_PseudoAbsorbance class provides a user interface for managing pseudo-absorbance data.
 */
class US_PseudoAbsorbance : public US_Widgets
{
    Q_OBJECT
    public:
        /**
         * @brief Constructor for US_PseudoAbsorbance.
         */
        US_PseudoAbsorbance();

    private slots:
                /**
                 * @brief Slot for selecting the add reference scan program.
                 */
                void select_addRef(void);

        /**
         * @brief Slot for selecting the convert scan program.
         */
        void select_cnvtSc(void);

        /**
         * @brief Slot for selecting the remove RI program.
         */
        void select_rmRi(void);

        /**
         * @brief Slot to turn on a QPushButton.
         * @param button The QPushButton to turn on or off
         * @param on Boolean value to turn the button on (true) or off (false)
         */
        void turn_on(QPushButton* button, bool on);

    private:
        /**
         * @enum programs
         * @brief Enum representing the different programs available.
         */
        enum programs { ADDREFSCAN, CONVERT, REMOVERI };

        US_AddRefScan *addRefScan; ///< Pointer to the add reference scan program
        US_ConvertScan *convertScan; ///< Pointer to the convert scan program
        US_RemoveRI *removeRi; ///< Pointer to the remove RI program
        QPushButton *pb_addRef; ///< QPushButton for add reference scan
        QPushButton *pb_cnvtSc; ///< QPushButton for convert scan
        QPushButton *pb_rmri; ///< QPushButton for remove RI
        QVBoxLayout *main_lyt; ///< Main layout for the widget
        int program_state; ///< Current program state
        QLabel* hline; ///< Horizontal line label

        /**
         * @brief Start a new program.
         * @param program The program to start
         */
        void new_program(int program);

        /**
         * @brief Close a program.
         * @tparam T The type of the program to close
         * @param program The program to close
         * @return Boolean indicating if the program was successfully closed
         */
        template<class T> bool cls_program(T* program);
};

#endif // US_PSEUDO_ABSORBANCE_H
