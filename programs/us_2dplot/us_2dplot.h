//! \file us_2dplot.h
//! \brief Contains the declaration of the US_2dPlot class and its members.
#ifndef US_2DPLOT_H
#define US_2DPLOT_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_plot.h"

#if QT_VERSION < 0x050000
#include <QApplication>
#else
#include <QtWidgets/QApplication>
#endif

//! \class US_2dPlot
//! \brief A class for creating 2D plots with user-defined parameters.
class US_2dPlot : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_2dPlot class.
        US_2dPlot();

    private:
        QVector<double> x; //!< X-axis data points.
        QVector<double> y; //!< Y-axis data points.

        double resolution; //!< Resolution for the plot.
        double s_min; //!< Minimum value for the s parameter.
        double s_max; //!< Maximum value for the s parameter.
        double f_min; //!< Minimum value for the f parameter.
        double f_max; //!< Maximum value for the f parameter.
        double par1; //!< First parameter for the plot.
        double par2; //!< Second parameter for the plot.
        double par3; //!< Third parameter for the plot.
        double par4; //!< Fourth parameter for the plot.
        int model; //!< Model selection for the plot.

        QPushButton* pb_calculate; //!< Button to trigger the calculation.
        QPushButton* pb_close; //!< Button to close the plot window.

        QLabel* lbl_par1; //!< Label for the first parameter.
        QLabel* lbl_par2; //!< Label for the second parameter.
        QLabel* lbl_par3; //!< Label for the third parameter.
        QLabel* lbl_par4; //!< Label for the fourth parameter.
        QLabel* lbl_resolution; //!< Label for the resolution.
        QLabel* lbl_model; //!< Label for the model selection.

        QwtPlot* data_plot; //!< QwtPlot object for data plotting.
        QwtPlotCurve* f_curve; //!< QwtPlotCurve object for the plot curve.
        QwtCounter* ct_par1; //!< Counter for the first parameter.
        QwtCounter* ct_par2; //!< Counter for the second parameter.
        QwtCounter* ct_par3; //!< Counter for the third parameter.
        QwtCounter* ct_par4; //!< Counter for the fourth parameter.
        QwtCounter* ct_resolution; //!< Counter for the resolution.
        QwtCounter* ct_model; //!< Counter for the model selection.

        US_Plot* plot; //!< US_Plot object for additional plotting functionalities.

    private slots:
                //! \brief Slot to calculate the plot based on current parameters.
                void calculate();

        //! \brief Slot to update the first parameter.
        //! \param value New value for the first parameter.
        void update_par1(double value);

        //! \brief Slot to update the second parameter.
        //! \param value New value for the second parameter.
        void update_par2(double value);

        //! \brief Slot to update the third parameter.
        //! \param value New value for the third parameter.
        void update_par3(double value);

        //! \brief Slot to update the fourth parameter.
        //! \param value New value for the fourth parameter.
        void update_par4(double value);

        //! \brief Slot to update the resolution.
        //! \param value New value for the resolution.
        void update_resolution(double value);

        //! \brief Slot to update the model.
        //! \param value New value for the model.
        void update_model(double value);
};

#endif // US_2DPLOT_H