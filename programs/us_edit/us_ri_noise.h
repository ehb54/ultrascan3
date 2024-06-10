//! \file us_ri_noise.h
//! \brief Contains the declaration of the US_RiNoise class and its members.
#ifndef US_RI_NOISE_H
#define US_RI_NOISE_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_dataIO.h"
#include "us_plot.h"

#include <qwt_counter.h>

//! \class US_RiNoise
//! \brief A class for handling radially invariant noise in UltraScan.
class US_RiNoise : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_RiNoise class.
        //! \param data The raw data.
        //! \param includes The list of included scans.
        //! \param range_left The left range value.
        //! \param range_right The right range value.
        //! \param label The label for the noise.
        //! \param order The order of the noise.
        //! \param residuals The list of residuals.
        US_RiNoise(const US_DataIO::RawData& data,
                   const QList<int>& includes,
                   double range_left,
                   double range_right,
                   const QString& label,
                   int& order,
                   QList<double>& residuals);

        //! \brief Static method to calculate residuals.
        //! \param data The raw data.
        //! \param includes The list of included scans.
        //! \param range_left The left range value.
        //! \param range_right The right range value.
        //! \param order The order of the noise.
        //! \param residuals The list of residuals.
        static void calc_residuals(const US_DataIO::RawData& data,
                                   const QList<int>& includes,
                                   double range_left,
                                   double range_right,
                                   int order,
                                   QList<double>& residuals);

    private:
        const US_DataIO::RawData& data; //!< Reference to the raw data.
        const QList<int>& includes; //!< Reference to the list of included scans.
        double range_left; //!< Left range value.
        double range_right; //!< Right range value.
        int& order; //!< Reference to the order of the noise.
        QList<double>& residuals; //!< Reference to the list of residuals.

        QTextEdit* te_details; //!< Text edit for details.

        QwtPlot* data_plot; //!< Plot for the data.
        QwtPlotCurve* curve; //!< Curve for the data plot.
        QwtCounter* ct_order; //!< Counter for the order.

    private slots:
        //! \brief Slot to draw the fit.
        //! \param value The value to draw the fit.
        void draw_fit(double value);
};

#endif // US_RI_NOISE_H
