//! \file us_second_moment.h
#ifndef US_SECOND_MOMENT_H
#define US_SECOND_MOMENT_H

#include "us_analysis_base2.h"
#include "us_editor.h"
#include "us_math2.h"

/**
 * @class US_SecondMoment
 * @brief The US_SecondMoment class provides functionality for second moment analysis.
 */
class US_SecondMoment : public US_AnalysisBase2
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_SecondMoment.
         */
        US_SecondMoment();

    private:
        double     average_2nd;   //!< Average second moment value
        double*    smPoints;      //!< Pointer to second moment points
        double*    smSeconds;     //!< Pointer to second moment seconds
        US_Editor* te_results;    //!< Editor for displaying results
        QwtPlotGrid*  grid;       //!< Plot grid

        /**
         * @brief Function to plot the data.
         */
        void data_plot ( void );

    private slots:
        /**
         * @brief Slot to write the report.
         * @param stream The text stream to write the report to.
         */
        void write_report(QTextStream& stream);

        /**
         * @brief Slot to exclude data points.
         */
        void exclude(void);

        /**
         * @brief Slot to view the results.
         */
        void view(void);

        /**
         * @brief Slot to save the results.
         */
        void save(void);

        /**
         * @brief Slot to show help information.
         */
        void help(void)
        {
            showHelp.show_help("manual/second_moment.html");
        }
};

#endif
