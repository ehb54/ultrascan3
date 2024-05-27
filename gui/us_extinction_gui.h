//! \file us_extinction_gui.h
#ifndef US_EXTINCTION_GUI_H
#define US_EXTINCTION_GUI_H

#include <QApplication>
#include <QtGui>
#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"
#include "us_db2.h"
#include "us_eprofile.h"
#include "us_minimize.h"

class CustomListWidgetItem : public QListWidgetItem {
public:
    CustomListWidgetItem(QListWidget* parent = nullptr)
            : QListWidgetItem(parent) {}

    bool operator<(const QListWidgetItem& other) const;
};

//! \class US_Extinction
//! \brief A class to provide a GUI for managing extinction data.
class US_GUI_EXTERN US_Extinction : public US_Widgets
{
    Q_OBJECT

    public:

    /*!
     * \brief Constructor for US_Extinction.
     * \param type The type of extinction.
     * \param db_flag Flag indicating if database is used.
     * \param p_buffer Pointer to the buffer.
     * \param p_analyte Pointer to the analyte.
     * \param p_solution Pointer to the solution.
     */
    US_Extinction( QString type, bool db_flag, US_Buffer* p_buffer = 0, US_Analyte* p_analyte = 0, US_Solution* p_solution = 0 );

    /*!
     * \brief Destructor for US_Extinction.
     */
    ~US_Extinction() {};

    private:
        US_Buffer* buffer; /*!< Buffer object. */
        US_Analyte* analyte; /*!< Analyte object. */
        US_Solution* solution; /*!< Solution object. */
        QString type; /*!< Type of extinction. */
        bool db_flag; /*!< Database flag. */

        QPushButton* pb_cancel; /*!< Cancel button. */
        QPushButton* pb_apply; /*!< Apply button. */
        QPushButton* pb_help; /*!< Help button. */
        QPushButton* pb_close; /*!< Close button. */

        QGridLayout* mainLayout; /*!< Main layout. */
        QVBoxLayout* leftLayout; /*!< Left layout. */
        QVBoxLayout* rightLayout; /*!< Right layout. */

        /*!
         * \brief Initialize the GUI components.
         */
        void initGUI();

    private slots:

        /*!
         * \brief Slot to handle cancel button click.
         */
        void cancel();

        /*!
         * \brief Slot to handle apply button click.
         */
        void apply();

        /*!
         * \brief Slot to handle help button click.
         */
        void help();

        /*!
         * \brief Slot to handle close button click.
         */
        void close();

    signals:

        /*!
         * \brief Signal emitted when data is saved.
         */
        void dataSaved();

        /*!
         * \brief Signal emitted to get extinction data.
         * \param extinctionData The extinction data.
         */
        void get_extinction_data(QVector<double>& extinctionData);

        /*!
         * \brief Signal emitted to set extinction data.
         * \param extinctionData The extinction data.
         */
        void set_extinction_data(const QVector<double>& extinctionData);
};

#endif
