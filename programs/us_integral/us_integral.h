//! \file us_integral.h
#ifndef US_INTEGRAL_H
#define US_INTEGRAL_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_model_loader.h"
#include "us_solute.h"
#include "us_spectrodata.h"
#include "us_plot.h"

#include "qwt_plot_marker.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_zoomer.h"
#include "qwt_plot_panner.h"
#include "qwt_scale_widget.h"
#include "qwt_scale_draw.h"

//! \brief Distribution structure
typedef struct distro_sys
{
    QList< S_Solute >   in_distro;      //!< Raw input distribution
    QList< S_Solute >   nm_distro;      //!< Normalized concentration distribution
    QList< S_Solute >   bo_distro;      //!< Boundary distribution with original points
    QList< S_Solute >   bf_distro;      //!< Boundary fractions distribution
    QString             run_name;       //!< Distribution run name
    QString             analys_name;    //!< Distribution analysis name
    QString             method;         //!< Model method (e.g., "2DSA")
    QString             editGUID;       //!< Associated edit GUID
    QString             solutionGUID;   //!< Associated solution GUID
    QString             label;          //!< Distribution label (channel description)
    int                 distro_type;    //!< Distribution type flag
    int                 solutionID;     //!< Associated solution database ID
    double              tot_conc;       //!< Total concentration
} DisSys;

//! \brief Less-than function for sorting distributions by S_Solute attribute
bool distro_lessthan_s(const S_Solute&, const S_Solute&);
bool distro_lessthan_k(const S_Solute&, const S_Solute&);
bool distro_lessthan_w(const S_Solute&, const S_Solute&);
bool distro_lessthan_d(const S_Solute&, const S_Solute&);

//! \brief Class for displaying models in pseudo-3D
class US_Integral : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Pseudo-3D Combination constructor
        US_Integral();

        //! \brief Function to get x-axis value
        //! \return x-axis value
        int get_x();

    private:

        //! \brief Enumeration for attribute types
        enum attr_type { ATTR_S, ATTR_K, ATTR_W, ATTR_D, ATTR_F };

        QLabel*       lb_division;       //!< Division label
        QTextEdit*    te_distr_info;     //!< Distribution information text edit
        QLineEdit*    le_prefilt;        //!< Pre-filter line edit
        US_Help       showHelp;          //!< Help object

        QwtCounter*   ct_division;       //!< Division counter
        QwtCounter*   ct_smoothing;      //!< Smoothing counter
        QwtCounter*   ct_boundaryPct;    //!< Boundary percentage counter
        QwtCounter*   ct_boundaryPos;    //!< Boundary position counter

        QwtPlot*      data_plot;         //!< Data plot

        QwtPlotPicker* pick;             //!< Plot picker

        US_Disk_DB_Controls* dkdb_cntrls; //!< Disk database controls

        QPushButton*  pb_refresh;        //!< Refresh button
        QPushButton*  pb_reset;          //!< Reset button
        QPushButton*  pb_prefilt;        //!< Pre-filter button
        QPushButton*  pb_lddistr;        //!< Load distribution button
        QPushButton*  pb_help;           //!< Help button
        QPushButton*  pb_close;          //!< Close button
        QPushButton*  pb_rmvdist;        //!< Remove distribution button
        QPushButton*  pb_mdlpars;        //!< Model parameters button
        QPushButton*  pb_save;           //!< Save button

        QRadioButton* rb_x_mass;         //!< x-axis mass radio button
        QRadioButton* rb_x_ff0;          //!< x-axis frictional ratio radio button
        QRadioButton* rb_x_vbar;         //!< x-axis vbar radio button
        QRadioButton* rb_x_s;            //!< x-axis sedimentation coefficient radio button
        QRadioButton* rb_x_d;            //!< x-axis diffusion coefficient radio button
        QRadioButton* rb_da_n;           //!< Distribution average n radio button
        QRadioButton* rb_da_s;           //!< Distribution average s radio button
        QRadioButton* rb_da_w;           //!< Distribution average w radio button

        QButtonGroup* bg_x_axis;         //!< x-axis button group
        QButtonGroup* bg_di_avg;         //!< Distribution average button group

        QVector< DisSys >             alldis;    //!< All distributions

        QVector< double >             v_bfracs;  //!< Boundary fraction vector
        QVector< QVector< double > >  v_vbars;   //!< Vector of vbars per fraction
        QVector< QVector< double > >  v_mmass;   //!< Vector of molar masses per fraction
        QVector< QVector< double > >  v_frats;   //!< Vector of friction ratios per fraction
        QVector< QVector< double > >  v_sedcs;   //!< Vector of sedimentation coefficient vectors per distribution
        QVector< QVector< double > >  v_difcs;   //!< Vector of diffusion coefficient vectors per distribution

        int           dbg_level;        //!< Debug level
        int           plot_x;           //!< x-axis plot selection

        QString       xa_title;         //!< x-axis title
        QString       ya_title;         //!< y-axis title
        QString       mfilter;          //!< Filter string

        QStringList   pfilts;           //!< Pre-filter string list
        QStringList   mdescs;           //!< Model descriptions string list

    private slots:

                //! \brief Slot to plot data
                //! \param index Index of the data to be plotted
                void plot_data(int index);

        //! \brief Slot to plot data
        void plot_data();

        //! \brief Slot to update disk database
        //! \param checked Whether the disk database is checked
        void update_disk_db(bool checked);

        //! \brief Slot to update division
        //! \param value New division value
        void update_divis(double value);

        //! \brief Slot to select pre-filter
        void select_prefilt();

        //! \brief Slot to load distribution
        void load_distro();

        //! \brief Slot to load distribution
        //! \param model Model to load
        //! \param method Method for loading
        void load_distro(US_Model model, QString method);

        //! \brief Slot to resort solution
        //! \param distributions Vector of distributions to be resorted
        void resort_sol(QVector< DisSys >& distributions);

        //! \brief Slot to reset the interface
        void reset();

        //! \brief Slot to save the current state
        void save();

        //! \brief Slot to sort distribution
        //! \param solutes List of solutes to be sorted
        //! \param ascending Whether to sort in ascending order
        void sort_distro(QList< S_Solute >& solutes, bool ascending);

        //! \brief Slot to remove distribution
        void remove_distro();

        //! \brief Slot to select x-axis
        //! \param index Index of the selected x-axis
        void select_x_axis(int index);

        //! \brief Slot to build boundary fraction distribution
        //! \param index Index of the boundary fraction
        void build_bf_distro(int index);

        //! \brief Slot to build boundary fraction distributions
        void build_bf_dists();

        //! \brief Slot to build boundary fraction vectors
        void build_bf_vects();

        //! \brief Slot to select x-axis for plotting
        //! \return Selected x-axis index
        int plot_x_select();

        //! \brief Function to get annotation title
        //! \param index Index of the annotation
        //! \return Annotation title
        QString anno_title(int index);

        //! \brief Function to get plot type text
        //! \param index Index of the plot type
        //! \return Plot type text
        QString ptype_text(int index);

        //! \brief Slot to display models summary
        void models_summary();

        //! \brief Function to write CSV file
        //! \param filename Name of the CSV file
        //! \param headers Headers for the CSV file
        //! \param values Vector of values to be written
        //! \param xheaders Headers for the x-axis
        //! \param xvalues Vector of x-axis values
        void write_csv(const QString filename, const QString headers, QVector< double >& values,
                       const QString xheaders, QVector< double >& xvalues);

        //! \brief Slot to display help information
        void help()
        { showHelp.show_help("integral.html"); }
};
#endif
