//! \file us_combined_plots_gui.h
#ifndef US_COMBPLOTS_GUI_H
#define US_COMBPLOTS_GUI_H

#include "us_extern.h"
#include "us_widgets.h"

//! \class US_CombPlotsGui
//! \brief This class provides a tabbed entry for solution selection.
class US_GUI_EXTERN US_CombPlotsGui : public US_Widgets
{
Q_OBJECT

public:

/*!
 * \brief Constructor for US_CombPlotsGui.
 * \param plotMask A string defining the plot mask.
 * \param typeMethodList A list of type methods.
 */
US_CombPlotsGui( QString plotMask, QStringList typeMethodList );

/*!
 * \brief Destructor for US_CombPlotsGui.
 */
~US_CombPlotsGui() {};

signals:

/*!
 * \brief Signal emitted to update combined plots parameters.
 * \param jsonParms The JSON string of parameters.
 */
void update_combplots_parms( QString& jsonParms );

private:
    QString     combPlotsMask;   /*!< Plot mask string */
    QStringList type_method_list; /*!< List of type methods */
    QJsonObject json; /*!< JSON object for parameters */

    int row; /*!< Current row for layout */
    QVBoxLayout* main; /*!< Main layout */
    QGridLayout* genL; /*!< General layout */
    QScrollArea* scrollArea; /*!< Scroll area */
    QWidget* containerWidget; /*!< Container widget */
    QWidget* topContainerWidget; /*!< Top container widget */
    QHBoxLayout* lower_buttons; /*!< Layout for lower buttons */

    QMap< QString, QMap< QString, QString > > combPlots_map; /*!< Map for combined plots parameters */

    QPushButton* pb_cancel; /*!< Cancel button */
    QPushButton* pb_accept; /*!< Accept button */
    QPushButton* pb_applyall; /*!< Apply to all button */

    /*!
     * \brief Parse the JSON parameters.
     */
    void parse_json( void );

    /*!
     * \brief Build the GUI layout.
     */
    void build_layout( void );

    /*!
     * \brief Convert GUI inputs to parameters.
     */
    void gui_to_parms( void );

    /*!
     * \brief Convert parameters to JSON string.
     * \return JSON string of parameters.
     */
    QString to_json( void );

private slots:

    /*!
     * \brief Slot to handle cancel update action.
     */
    void cancel_update( void );

    /*!
     * \brief Slot to handle update parameters action.
     */
    void update_parms( void );

    /*!
     * \brief Slot to handle apply to all action.
     */
    void applied_to_all( void );

public slots:

};

#endif
