//! \file us_eqmodel_control.h
//! \brief Contains the declaration of the US_EqModelControl class and its members.
#ifndef US_EQMODCTRL_H
#define US_EQMODCTRL_H

#include "us_extern.h"
#include "us_globeq_data.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_analyte.h"
#include "us_buffer.h"
#include "us_help.h"

//! \class US_EqModelControl
//! \brief A class for controlling equilibrium model parameters in UltraScan.
class US_EqModelControl : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EqModelControl class.
        //! \param scan_fits Reference to the vector of scan fits.
        //! \param run_fit Reference to the run fit parameters structure.
        //! \param edata Pointer to the edited data.
        //! \param model_index Selected model index.
        //! \param model_titles List of model titles.
        //! \param model_widget_created Flag indicating if the model widget is created.
        //! \param selected_scan Current selected scan.
        US_EqModelControl(QVector< EqScanFit >& scan_fits, EqRunFit& run_fit,
                          US_DataIO::EditedData* edata, int model_index, QStringList model_titles, bool& model_widget_created, int& selected_scan);

        //! \brief Slot to handle new scan.
        //! \param scan_index The new scan index.
        void new_scan(int scan_index);

        //! \brief Slot to handle new components.
        void new_components(void);

        //! \brief Slot to set float flag.
        //! \param is_float Boolean indicating if the flag is float.
        void set_float(bool is_float);

        signals:
                //! \brief Signal to update scan.
                //! \param scan_index The scan index to update.
                void update_scan(int scan_index);

    private:
        QVector< EqScanFit >&   scanfits;  //!< Reference to the vector of scan fits.
        EqRunFit&               runfit;    //!< Reference to the run fit parameters structure.
        US_DataIO::EditedData*  edata;     //!< Pointer to the edited data.
        int                     modelx;    //!< Selected model index.
        QStringList             models;    //!< List of model titles.
        bool&                   mWidget;   //!< Flag indicating if the model widget is created.
        int&                    selscan;   //!< Current selected scan.

        QLabel*            lb_gbanner;    //!< Label for global banner.
        QLabel*            lb_lbanner;    //!< Label for local banner.
        QLabel*            lb_molecwt;    //!< Label for molecular weight.
        QLabel*            lb_amplitu;    //!< Label for amplitude.

        QLineEdit*         le_mwguess;    //!< Line edit for molecular weight guess.
        QLineEdit*         le_mwbound;    //!< Line edit for molecular weight bound.
        QLineEdit*         le_vbguess;    //!< Line edit for vbar guess.
        QLineEdit*         le_vbbound;    //!< Line edit for vbar bound.
        QLineEdit*         le_l1guess;    //!< Line edit for length 1 guess.
        QLineEdit*         le_l1bound;    //!< Line edit for length 1 bound.
        QLineEdit*         le_l2guess;    //!< Line edit for length 2 guess.
        QLineEdit*         le_l2bound;    //!< Line edit for length 2 bound.
        QLineEdit*         le_l3guess;    //!< Line edit for length 3 guess.
        QLineEdit*         le_l3bound;    //!< Line edit for length 3 bound.
        QLineEdit*         le_l4guess;    //!< Line edit for length 4 guess.
        QLineEdit*         le_l4bound;    //!< Line edit for length 4 bound.
        QLineEdit*         le_cdescr;     //!< Line edit for component description.
        QLineEdit*         le_runid;      //!< Line edit for run ID.
        QLineEdit*         le_tempera;    //!< Line edit for temperature.
        QLineEdit*         le_speed;      //!< Line edit for speed.
        QLineEdit*         le_wavelen;    //!< Line edit for wavelength.
        QLineEdit*         le_pathlen;    //!< Line edit for path length.
        QLineEdit*         le_plenscn;    //!< Line edit for path length scan.
        QLineEdit*         le_blguess;    //!< Line edit for baseline guess.
        QLineEdit*         le_blbound;    //!< Line edit for baseline bound.
        QLineEdit*         le_density;    //!< Line edit for density.
        QLineEdit*         le_densscn;    //!< Line edit for density scan.
        QLineEdit*         le_amguess;    //!< Line edit for amplitude guess.
        QLineEdit*         le_ambound;    //!< Line edit for amplitude bound.
        QLineEdit*         le_extinct;    //!< Line edit for extinction.
        QLineEdit*         le_extiscn;    //!< Line edit for extinction scan.
        QLineEdit*         le_sigma;      //!< Line edit for sigma.

        QRadioButton*      rb_mwfloat;    //!< Radio button for molecular weight float.
        QRadioButton*      rb_mwlock;     //!< Radio button for molecular weight lock.
        QRadioButton*      rb_vbfloat;    //!< Radio button for vbar float.
        QRadioButton*      rb_vblock;     //!< Radio button for vbar lock.
        QRadioButton*      rb_l1float;    //!< Radio button for length 1 float.
        QRadioButton*      rb_l1lock;     //!< Radio button for length 1 lock.
        QRadioButton*      rb_l2float;    //!< Radio button for length 2 float.
        QRadioButton*      rb_l2lock;     //!< Radio button for length 2 lock.
        QRadioButton*      rb_l3float;    //!< Radio button for length 3 float.
        QRadioButton*      rb_l3lock;     //!< Radio button for length 3 lock.
        QRadioButton*      rb_l4float;    //!< Radio button for length 4 float.
        QRadioButton*      rb_l4lock;     //!< Radio button for length 4 lock.
        QRadioButton*      rb_blfloat;    //!< Radio button for baseline float.
        QRadioButton*      rb_bllock;     //!< Radio button for baseline lock.
        QRadioButton*      rb_amfloat;    //!< Radio button for amplitude float.
        QRadioButton*      rb_amlock;     //!< Radio button for amplitude lock.

        QCheckBox*         ck_mwbound;    //!< Checkbox for molecular weight bound.
        QCheckBox*         ck_vbbound;    //!< Checkbox for vbar bound.
        QCheckBox*         ck_l1bound;    //!< Checkbox for length 1 bound.
        QCheckBox*         ck_l2bound;    //!< Checkbox for length 2 bound.
        QCheckBox*         ck_l3bound;    //!< Checkbox for length 3 bound.
        QCheckBox*         ck_l4bound;    //!< Checkbox for length 4 bound.
        QCheckBox*         ck_blbound;    //!< Checkbox for baseline bound.
        QCheckBox*         ck_ambound;    //!< Checkbox for amplitude bound.
        QCheckBox*         ck_inclfit;    //!< Checkbox for include fit.

        QPushButton*       pb_vbar20;     //!< Push button for vbar at 20°C.
        QPushButton*       pb_lnasc1;     //!< Push button for length aspect 1.
        QPushButton*       pb_lnasc2;     //!< Push button for length aspect 2.
        QPushButton*       pb_lnasc3;     //!< Push button for length aspect 3.
        QPushButton*       pb_lnasc4;     //!< Push button for length aspect 4.
        QPushButton*       pb_plenapp;    //!< Push button for path length apply.
        QPushButton*       pb_density;    //!< Push button for density.
        QPushButton*       pb_densapp;    //!< Push button for density apply.
        QPushButton*       pb_extinct;    //!< Push button for extinction.
        QPushButton*       pb_extiapp;    //!< Push button for extinction apply.

        QwtCounter*        ct_grunpar;    //!< Counter for global run parameters.
        QwtCounter*        ct_lrunpar;    //!< Counter for local run parameters.
        QwtCounter*        ct_scansel;    //!< Counter for scan selection.

        US_Help            showHelp;      //!< Help object.

        bool               send_signal;   //!< Flag indicating if a signal should be sent.
        bool               chng_floats;   //!< Flag indicating if float values changed.

    private slots:
        //! \brief Slot to handle scan change.
        //! \param value The new scan value.
        void scan_changed(double value);

        //! \brief Slot to handle global component change.
        //! \param value The new global component value.
        void global_comp_changed(double value);

        //! \brief Slot to handle local component change.
        //! \param value The new local component value.
        void local_comp_changed(double value);

        //! \brief Slot to update sigma value.
        void update_sigma(void);

        //! \brief Slot to update float values.
        void update_floats(void);

        //! \brief Slot to update global values.
        void update_gvals(void);

        //! \brief Slot to update local values.
        void update_lvals(void);

        //! \brief Slot to disconnect global component.
        void disconnect_global(void);

        //! \brief Slot to disconnect local component.
        void disconnect_local(void);

        //! \brief Slot to connect global component.
        void connect_global(void);

        //! \brief Slot to connect local component.
        void connect_local(void);

        //! \brief Slot to apply path length.
        void pathlen_applyto(void);

        //! \brief Slot to apply density.
        void density_applyto(void);

        //! \brief Slot to apply extinction.
        void extinct_applyto(void);

        //! \brief Slot to set vbar value.
        void set_vbar(void);

        //! \brief Slot to set density value.
        void set_density(void);

        //! \brief Slot to assign vbar.
        //! \param analyte The analyte object.
        void assignVbar(US_Analyte analyte);

        //! \brief Slot to assign density.
        //! \param buffer The buffer object.
        void assignDensity(US_Buffer buffer);

        //! \brief Creates a horizontal box layout with radio buttons.
        //! \param grid1 The first grid layout.
        //! \param rb1 The first radio button.
        //! \param grid2 The second grid layout.
        //! \param rb2 The second radio button.
        //! \return The created horizontal box layout.
        QHBoxLayout* radiobox(QGridLayout* grid1, QRadioButton* rb1, QGridLayout* grid2, QRadioButton* rb2);

        //! \brief Slot to handle selection.
        void selected(void);

        //! \brief Slot to show help.
        void help(void) { showHelp.show_help("global_equil-modctrl.html"); };
};

#endif // US_EQMODCTRL_H
