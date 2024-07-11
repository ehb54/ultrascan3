//! \file us_vhw_combo.h
#ifndef US_VHW_COMBO_H
#define US_VHW_COMBO_H

#include "us_editor.h"
#include "us_math2.h"
#include "us_run_details2.h"
#include "us_buffer_gui.h"
#include "us_plot3d.h"
#include "us_vhwc_pltctl.h"
#include "qwt_plot_marker.h"

#define PA_POINTS 20     // plateau average points to left and right

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug() //!< debug-level-conditioned qDebug()
#endif

/**
 * @class US_vHW_Combine
 * @brief The US_vHW_Combine class provides functionality for combining vHW distributions.
 */
class US_vHW_Combine : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_vHW_Combine.
         */
        US_vHW_Combine();

    private:

        /**
         * @class DistrDesc
         * @brief Class to describe distribution data.
         */
        class DistrDesc
        {
            public:
                QString            runID;    //!< Run ID
                QString            triple;   //!< Triple
                QString            tdescr;   //!< Triple description string
                QVector<double>    dsedcs;   //!< Distribution sedimentation coefficients
                QVector<double>    bfracs;   //!< Boundary fractions
                QVector<double>    esedcs;   //!< Envelope sedimentation coefficients
                QVector<double>    efreqs;   //!< Envelope frequencies
                QwtSymbol*         symbol;   //!< Curve symbol pointer
                QColor             color;    //!< Curve color
                double             totconc;  //!< Total concentration

                /**
                 * @brief Default constructor for DistrDesc.
                 */
                DistrDesc()
                {
                    runID    = "";
                    triple   = "";
                    tdescr   = "";
                    dsedcs.clear();
                    bfracs.clear();
                    esedcs.clear();
                    efreqs.clear();
                    color    = QColor(Qt::white);
                    symbol   = new QwtSymbol();
                    totconc  = 0.0;
                };

                /**
                 * @brief Copy constructor for DistrDesc.
                 * @param dd The DistrDesc object to copy
                 */
                DistrDesc(const DistrDesc& dd)
                {
                    runID    = dd.runID;
                    triple   = dd.triple;
                    tdescr   = dd.tdescr;
                    dsedcs   = dd.dsedcs;
                    bfracs   = dd.bfracs;
                    esedcs   = dd.esedcs;
                    efreqs   = dd.efreqs;
                    color    = dd.color;
                    totconc  = dd.totconc;
                    symbol   = new QwtSymbol(
                            dd.symbol->style(),
                            dd.symbol->brush(),
                            dd.symbol->pen(),
                            dd.symbol->size());
                };

                /**
                 * @brief Assignment operator for DistrDesc.
                 * @param dd The DistrDesc object to assign from
                 * @return A reference to the assigned DistrDesc object
                 */
                DistrDesc& operator=(const DistrDesc& dd)
                {
                    runID    = dd.runID;
                    triple   = dd.triple;
                    tdescr   = dd.tdescr;
                    dsedcs   = dd.dsedcs;
                    bfracs   = dd.bfracs;
                    esedcs   = dd.esedcs;
                    efreqs   = dd.efreqs;
                    color    = dd.color;
                    totconc  = dd.totconc;
                    symbol   = new QwtSymbol(
                            dd.symbol->style(),
                            dd.symbol->brush(),
                            dd.symbol->pen(),
                            dd.symbol->size());
                    return *this;
                };
        };

        QList<DistrDesc> distros;     //!< All distributions
        QList<DistrDesc> pdistrs;     //!< Plotted distributions

        QList<int> symbols;           //!< Possible symbol shapes
        QList<QColor> colors;         //!< Possible colors

        US_Disk_DB_Controls* dkdb_cntrls;  //!< Disk-or-DB controls

        QVector<QVector3D> xyzdat;    //!< XYZ data

        QPointer<US_VhwCPlotControl> p3d_ctld; //!< Pointer to 3D control
        QPointer<US_Plot3D>          p3d_pltw; //!< Pointer to 3D window

        QStringList distIDs;          //!< All distribution IDs
        QStringList pdisIDs;          //!< Plotted distribution IDs

        QPushButton* pb_saveda;       //!< Save button
        QPushButton* pb_resetd;       //!< Reset data button
        QPushButton* pb_resetp;       //!< Reset plot button
        QPushButton* pb_plot3d;       //!< Plot 3D button

        QLineEdit* le_runid;          //!< Run ID line edit

        QComboBox* cmb_svproj;        //!< Save project combo box

        QListWidget* lw_runids;       //!< Run IDs list widget
        QListWidget* lw_triples;      //!< Triples list widget

        QCheckBox* ck_distrib;        //!< Distribution checkbox
        QCheckBox* ck_envelope;       //!< Envelope checkbox
        QCheckBox* ck_intconc;        //!< Integrated concentration checkbox

        US_Help showHelp;             //!< Help display object

        QwtPlot* data_plot1;          //!< Data plot
        US_PlotPicker* gpick;         //!< Plot picker

        QString runID;                //!< Run ID
        QString triple;               //!< Triple
        QString dat1File;             //!< Data file
        QString env1File;             //!< Envelope file
        QString lis1File;             //!< List file

        int dbg_level;                //!< Debug level

    private slots:
        void load(void);                              //!< Slot to load data
        void save(void);                              //!< Slot to save data
        void reset_data(void);                        //!< Slot to reset data
        void reset_plot(void);                        //!< Slot to reset plot
        void plot_data(void);                         //!< Slot to plot data
        void plot_3d(void);                           //!< Slot to plot 3D data
        void plot_distr(DistrDesc, QString);          //!< Plot distribution data
        void runid_select(int);                       //!< Slot to handle run ID selection
        void triple_select(int);                      //!< Slot to handle triple selection
        void setDSymbol(DistrDesc&, int);             //!< Set the distribution symbol
        int envel_data(DistrDesc&);                   //!< Get envelope data
        void possibleSymbols(void);                   //!< Get possible symbols
        void control_closed(void);                    //!< Slot to handle control closed
        void update_disk_db(bool);                    //!< Slot to update disk/db choice
        void fill_in_desc(QTextStream&, QTextStream&, DistrDesc&, bool, int); //!< Fill in description
        void write_data(QString&, QString&, int&);    //!< Write data
        void write_denv(QString&, int&);              //!< Write envelope data
        QString expandedTriple(QString);              //!< Expand triple
        QString collapsedTriple(QString);             //!< Collapse triple
        int reportDocsFromFiles(QString&, QString&, QStringList&, US_DB2*, int&, QString&); //!< Report documents from files

        void help(void)
        {
            showHelp.show_help("vhw_combine.html");
        }
};

#endif
