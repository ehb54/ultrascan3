//! \file us_eqhistogram.h
//! \brief Contains the declaration of the US_EqHistogram class and its members.
#ifndef US_HISTOGRAM_H
#define US_HISTOGRAM_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_globeq_data.h"
#include "us_plot.h"

//! \class US_EqHistogram
//! \brief A simple dialog to display a histogram plot.
class US_EqHistogram : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EqHistogram class.
        //! \param od_limit OD limit value.
        //! \param scan_fits Vector of scan fits.
        //! \param parent Pointer to the parent widget.
        //! \param flags Window flags.
        US_EqHistogram(double od_limit, QVector< EqScanFit >& scan_fits, QWidget* parent = 0, Qt::WindowFlags flags = 0);

    private:
        double                 od_limit;  //!< OD Limit value.
        QVector< EqScanFit >&  scanfits;  //!< Vector of scan fits.

        US_Plot*    hplot;      //!< Histogram plot layout.
        QwtPlot*    hist_plot;  //!< The histogram plot.

    private slots:
};

#endif

