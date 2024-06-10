//! \file us_modelmetrics.h
#ifndef US_MODELMETRICS_H
#define US_MODELMETRICS_H

#include <QApplication>
#include <QDomDocument>

#include "us_analysis_base2.h"
#include "us_spectrodata.h"
#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_editor.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_investigator.h"
#include "us_math2.h"
#include "us_util.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_model_loader.h"
#include "us_select_runs.h"
#include <qwt_legend.h>
#include "us_license_t.h"
#include "us_license.h"

//! \class HydroParm
//! \brief Class representing a hydrodynamic parameter with concentration.
class HydroParm
{
public:
    double parm;  //!< Parameter value
    double conc;  //!< Concentration value

    //! \brief Default constructor
    HydroParm() {};

    //! \brief Destructor
    ~HydroParm() {};

    //! \brief Equality operator
    //! \param objIn Another HydroParm object to compare
    //! \return True if the parameter values are equal
    bool operator==(const HydroParm& objIn)
    {
        return (parm == objIn.parm);
    }

    //! \brief Inequality operator
    //! \param objIn Another HydroParm object to compare
    //! \return True if the parameter values are not equal
    bool operator!=(const HydroParm& objIn)
    {
        return (parm != objIn.parm);
    }

    //! \brief Less-than operator
    //! \param objIn Another HydroParm object to compare
    //! \return True if the parameter value is less than the other
    bool operator < (const HydroParm& objIn) const
    {
        return (parm < objIn.parm);
    }
};

//! \struct reportItem
//! \brief Structure representing a report item with various metrics and data.
struct reportItem
{
    QString investigator;  //!< Investigator name
    QString runID;         //!< Run ID
    QString triple;        //!< Triple
    QString analysis;      //!< Analysis type
    QString iterations;    //!< Number of iterations
    QString edit;          //!< Edit type
    QString parameter;     //!< Parameter
    QString sigma;         //!< Sigma value
    QString d[3];          //!< Array of D values
    QString x[3];          //!< Array of X values
    QString span;          //!< Span value
    QString minimum;       //!< Minimum value
    QString maximum;       //!< Maximum value
    QString mean;          //!< Mean value
    QString mode;          //!< Mode value
    QString median;        //!< Median value
    QString skew;          //!< Skew value
    QString kurtosis;      //!< Kurtosis value
    QString span_label;    //!< Span label
    QString filename;      //!< Filename
    QString integral;      //!< Integral value
    QString totalc;        //!< Total concentration
    QString name;          //!< Name
    QString csv;           //!< CSV data
    QPixmap pixmap;        //!< Pixmap image
};

//! \brief Less-than function for sorting S_Solute values
bool distro_lessthan(const S_Solute&, const S_Solute&);

//! \class US_ModelMetrics
//! \brief Class for managing and analyzing model metrics.
class US_ModelMetrics : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_ModelMetrics
        US_ModelMetrics();

        //! \enum hydro_parms
        //! \brief Enumeration for hydrodynamic parameters
        enum hydro_parms { HPs, HPd, HPm, HPk, HPf, HPv, HPr };

    private:
        reportItem         report_entry; //!< Report entry
        QString            report;       //!< Report string
        QString            xtitle;       //!< X-axis title
        QTextStream        report_ts;    //!< Report text stream
        QFile              report_file;  //!< Report file
        int                dbg_level;    //!< Debug level
        int                mc_iters;     //!< Monte Carlo iterations
        int                fixed;        //!< Fixed parameter
        int                calc_val;     //!< Calculated value
        int                model_count;  //!< Model count
        bool               monte_carlo;  //!< Monte Carlo flag
        bool               saved;        //!< Saved flag
        bool               xautoscale;   //!< X-axis autoscale flag
        double             plotxmin;     //!< X-axis minimum plot value
        double             plotxmax;     //!< X-axis maximum plot value
        double*            xx;           //!< X data array
        double*            yy;           //!< Y data array
        double             sigma;        //!< Sigma value
        double             tc;           //!< Total concentration
        double             cmin;         //!< Minimum concentration
        double             cmax;         //!< Maximum concentration
        double             smin;         //!< Minimum S value
        double             smax;         //!< Maximum S value
        double             kmin;         //!< Minimum K value
        double             kmax;         //!< Maximum K value
        double             wmin;         //!< Minimum W value
        double             wmax;         //!< Maximum W value
        double             vmin;         //!< Minimum V value
        double             vmax;         //!< Maximum V value
        double             dmin;         //!< Minimum D value
        double             dmax;         //!< Maximum D value
        double             fmin;         //!< Minimum F value
        double             fmax;         //!< Maximum F value
        double             xmin;         //!< Minimum X value
        double             xmax;         //!< Maximum X value
        double             rmin;         //!< Minimum R value
        double             rmax;         //!< Maximum R value
        double             dval1;        //!< D value 1
        double             xval1;        //!< X value 1
        double             dval2;        //!< D value 2
        double             xval2;        //!< X value 2
        double             dval3;        //!< D value 3
        double             xval3;        //!< X value 3
        double             total_conc;   //!< Total concentration
        QList< HydroParm > hp_distro;    //!< Hydrodynamic parameter distribution
        QList< HydroParm > orig_list;    //!< Original list
        US_Help            showHelp;     //!< Help display
        US_Editor*         te;           //!< Text editor
        US_Model*          model;        //!< Model object
        US_PlotPicker*     pick;         //!< Plot picker
        QPushButton*       pb_load_model; //!< Load model button
        QPushButton*       pb_prefilter; //!< Prefilter button
        QPushButton*       pb_report;    //!< Report button
        QPushButton*       pb_write;     //!< Write button
        QLabel*            lbl_dval1;    //!< D value 1 label
        QLabel*            lbl_dval2;    //!< D value 2 label
        QLabel*            lbl_dval3;    //!< D value 3 label
        QLabel*            lbl_span;     //!< Span label
        QLabel*            lbl_minimum;  //!< Minimum value label
        QLabel*            lbl_maximum;  //!< Maximum value label
        QLabel*            lbl_mean;     //!< Mean value label
        QLabel*            lbl_mode;     //!< Mode value label
        QLabel*            lbl_median;   //!< Median value label
        QLabel*            lbl_kurtosis; //!< Kurtosis value label
        QLabel*            lbl_skew;     //!< Skew value label
        QLabel*            lbl_sigma;    //!< Sigma value label
        QLabel*            lbl_integral; //!< Integral value label
        QLabel*            lbl_name;     //!< Name label
        QLabel*            lbl_plotxmin; //!< X-axis minimum plot value label
        QLabel*            lbl_plotxmax; //!< X-axis maximum plot value label
        QwtCounter*        ct_dval1;     //!< D value 1 counter
        QwtCounter*        ct_dval2;     //!< D value 2 counter
        QwtCounter*        ct_dval3;     //!< D value 3 counter
        QwtCounter*        ct_sigma;     //!< Sigma counter
        QButtonGroup*      bg_hp;        //!< Hydrodynamic parameter button group
        QButtonGroup*      bg_plotlimits; //!< Plot limits button group
        QRadioButton*      rb_s;         //!< S value radio button
        QRadioButton*      rb_d;         //!< D value radio button
        QRadioButton*      rb_f;         //!< F value radio button
        QRadioButton*      rb_k;         //!< K value radio button
        QRadioButton*      rb_m;         //!< M value radio button
        QRadioButton*      rb_v;         //!< V value radio button
        QRadioButton*      rb_r;         //!< R value radio button
        QStringList        pfilts;       //!< Prefilter list
        QString            mfilter;      //!< Model filter
        QString            run_name;     //!< Run name
        QString            method;       //!< Method
        QString            analysis_name; //!< Analysis name
        QString            editGUID;     //!< Edit GUID
        QLineEdit*         le_model;     //!< Model line edit
        QLineEdit*         le_investigator; //!< Investigator line edit
        QLineEdit*         le_prefilter; //!< Prefilter line edit
        QLineEdit*         le_experiment; //!< Experiment line edit
        QLineEdit*         le_dval1;     //!< D value 1 line edit
        QLineEdit*         le_dval2;     //!< D value 2 line edit
        QLineEdit*         le_dval3;     //!< D value 3 line edit
        QLineEdit*         le_span;      //!< Span line edit
        QLineEdit*         le_minimum;   //!< Minimum value line edit
        QLineEdit*         le_maximum;   //!< Maximum value line edit
        QLineEdit*         le_mean;      //!< Mean value line edit
        QLineEdit*         le_mode;      //!< Mode value line edit
        QLineEdit*         le_median;    //!< Median value line edit
        QLineEdit*         le_kurtosis;  //!< Kurtosis value line edit
        QLineEdit*         le_skew;      //!< Skew value line edit
        QLineEdit*         le_integral;  //!< Integral value line edit
        QLineEdit*         le_name;      //!< Name line edit
        QLineEdit*         le_plotxmin;  //!< X-axis minimum plot value line edit
        QLineEdit*         le_plotxmax;  //!< X-axis maximum plot value line edit
        QwtPlot*           data_plot;    //!< Data plot
        QList< S_Solute >  sk_distro;    //!< Skew distribution
        QList< S_Solute >  xy_distro;    //!< XY distribution
        QList< S_Solute >* sdistro;      //!< Solute distribution

        US_Disk_DB_Controls* disk_controls; //!< Radiobuttons for disk/db choice

    private slots:
        //! \brief Create a table row with 5 columns
        //! \param row Row number
        //! \param col1 Column 1 value
        //! \param col2 Column 2 value
        //! \param col3 Column 3 value
        //! \param col4 Column 4 value
        //! \return Formatted table row string
        QString table_row(const int row, const QString &col1, const QString &col2, const QString &col3, const QString &col4) const;

        //! \brief Create a table row with 6 columns
        //! \param row Row number
        //! \param col1 Column 1 value
        //! \param col2 Column 2 value
        //! \param col3 Column 3 value
        //! \param col4 Column 4 value
        //! \param col5 Column 5 value
        //! \return Formatted table row string
        QString table_row(const int row, const QString &col1, const QString &col2, const QString &col3, const QString &col4, const QString &col5, const QString &col6) const;

        //! \brief Indent a string
        //! \param spaces Number of spaces to indent
        //! \return Indented string
        QString indent(const int spaces) const;

        //! \brief Select investigator
        void sel_investigator(void);

        //! \brief Add report item
        void addReportItem(void);

        //! \brief Update disk/db selection
        //! \param disk Flag indicating disk selection
        void update_disk_db(bool disk);

        //! \brief Load model
        void load_model(void);

        //! \brief Select prefilter
        void select_prefilter(void);

        //! \brief Reset the state
        void reset(void);

        //! \brief Calculate metrics
        void calc(void);

        //! \brief Plot data
        void plot_data(void);

        //! \brief Update sigma value
        void update_sigma(void);

        //! \brief Update name
        //! \param name New name
        void update_name(const QString &name);

        //! \brief Select hydrodynamic parameter
        //! \param index Parameter index
        void select_hp(int index);

        //! \brief Write report
        void write_report(void);

        //! \brief Sort distribution
        //! \param distro Distribution list
        //! \param ascend Flag indicating ascending order
        void sort_distro(QList< S_Solute > &distro, bool ascend);

        //! \brief Check if two values are equivalent
        //! \param val1 First value
        //! \param val2 Second value
        //! \param tol Tolerance
        //! \return True if values are equivalent within tolerance
        bool equivalent(double val1, double val2, double tol);

        //! \brief Set D value 1
        //! \param value New value
        void set_dval1(double value);

        //! \brief Set D value 2
        //! \param value New value
        void set_dval2(double value);

        //! \brief Set D value 3
        //! \param value New value
        void set_dval3(double value);

        //! \brief Set X-axis minimum plot value
        //! \param value New value
        void set_plotxmin(const QString &value);

        //! \brief Set X-axis maximum plot value
        //! \param value New value
        void set_plotxmax(const QString &value);

        //! \brief Set D value labels
        //! \param update Flag indicating if labels should be updated
        void set_dval_labels(bool update = false);

        //! \brief Set sigma value
        //! \param value New value
        void set_sigma(double value);

        //! \brief Show help
        void help(void)
        {
            showHelp.show_help("manual/us_modelmetrics.html");
        };
};

#endif
