//! \file us_dcdt.h
//! \brief Contains the declaration of the US_Dcdt class and its members.
#ifndef US_SECOND_MOMENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

//! \class US_Dcdt
//! \brief A class for performing second moment (dc/dt) analysis.
class US_Dcdt : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_Dcdt class.
        US_Dcdt();

    private:
        int graphType; //!< Type of graph to display.
        int previousScanCount; //!< Total number of scans before skipping and exclusion.
        double sMax; //!< Maximum s-value.

        QVector<int> arraySizes; //!< Sizes of arrays for dcdt data.
        QVector<int> arrayStart; //!< Start indices of arrays for dcdt data.
        QVector<double> avgDcdt; //!< Holds the average of all dcdt scans.
        QVector<double> avgS; //!< Holds the transformation to s of avgDcdt.

        static const int arrayLength = 400; //!< Length of arrays.

        US_Editor* te_results; //!< Text editor for results.

        QRadioButton* rb_radius; //!< Radio button for radius graph type.
        QRadioButton* rb_sed; //!< Radio button for sedimentation graph type.
        QRadioButton* rb_avg; //!< Radio button for average graph type.

        QwtCounter* ct_sValue; //!< Counter for s-value.

        QList<QVector<double>> dcdt; //!< Holds all the dcdt scans.
        QList<QVector<double>> sValues; //!< Holds s-value transformations from the dcdt scans.

        //! \brief Plot the data.
        void data_plot(void);

        //! \brief Get the results as a string.
        //! \return Results as a QString.
        QString results(void);

    private slots:
        //! \brief View the data.
        void view(void);

        //! \brief Save the data.
        void save(void);

        //! \brief Set the graph type.
        //! \param type The graph type to set.
        void set_graph(int type);

        //! \brief Reset the analysis.
        void reset(void);

        //! \brief Apply a smoothing function to the data.
        void smooth10(void);

        //! \brief Update the maximum s-value.
        //! \param value The new maximum s-value.
        void sMaxChanged(double value);

        //! \brief Exclude selected scans.
        void exclude(void);

        //! \brief Reset the excluded scans.
        void reset_excludes(void);

        //! \brief Write the report to a text stream.
        //! \param stream The text stream to write to.
        void write_report(QTextStream& stream);

        //! \brief Show help documentation.
        void help(void)
        {
            showHelp.show_help("manual/time_derivative.html");
        }
};

#endif // US_SECOND_MOMENT_H
