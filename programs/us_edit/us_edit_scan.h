//! \file us_edit_scan.cpp
//! \brief Contains the implementation of the US_EditScan class and its members.

#ifndef US_EDIT_SCAN_H
#define US_EDIT_SCAN_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"
#include "us_dataIO.h"
#include "us_plot.h"

//! \class US_EditScan
//! \brief A class for editing scan data in UltraScan.
class US_EditScan : public US_WidgetsDialog
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_EditScan class.
        //! \param scan The scan data to be edited.
        //! \param radii The vector of radii values.
        //! \param invert The invert value.
        //! \param range_left The left range value.
        //! \param range_right The right range value.
        US_EditScan(US_DataIO::Scan& scan, const QVector<double>& radii,
                    double invert, double range_left, double range_right);

        signals:
                //! \brief Signal emitted when the scan is updated.
                //! \param points The updated points as a list of QPointF.
                void scan_updated(QList<QPointF> points);

    private:
        US_DataIO::Scan workingScan; //!< Working copy of the scan data.
        US_DataIO::Scan& originalScan; //!< Reference to the original scan data.
        QVector<double> allRadii; //!< Vector of all radii values.

        double invert; //!< Invert value.
        double range_left; //!< Left range value.
        double range_right; //!< Right range value.
        double* radii; //!< Pointer to the radii values.
        double* values; //!< Pointer to the values.
        bool dragging; //!< Flag to indicate if dragging is in progress.
        int point; //!< Current point index.
        int offset; //!< Offset value.
        int count; //!< Count of points.
        QList<QPointF> changes; //!< List of changes made to the scan data.

        QwtPlot* data_plot; //!< Plot to display the scan data.
        QwtPlotCurve* curve; //!< Curve to display the scan data.
        US_PlotPicker* pick; //!< Plot picker for selecting points.
        QwtSymbol fgSym; //!< Foreground symbol for the plot points.
        QwtSymbol bgSym; //!< Background symbol for the plot points.
        QPen bgPen; //!< Background pen for the plot.
        QPen fgPen; //!< Foreground pen for the plot.

        US_Help showHelp; //!< Help object.

        //! \brief Redraw the plot with the updated data.
        void redraw(void);

    private slots:
        //! \brief Slot to handle the done action.
        void done(void);

        //! \brief Slot to handle the reset action.
        void reset(void);

        //! \brief Slot to start dragging a point.
        //! \param event The mouse event that started the drag.
        void start_drag(QMouseEvent* event);

        //! \brief Slot to end dragging a point.
        //! \param point The point where the drag ended.
        void end_drag(const QwtDoublePoint& point);

        //! \brief Slot to handle dragging a point.
        //! \param point The point being dragged.
        void drag(const QwtDoublePoint& point);

        //! \brief Slot to show the help documentation.
        void help(void)
        {
            showHelp.show_help("manual/edit_scan.html");
        }
};

#endif // US_EDIT_SCAN_H
