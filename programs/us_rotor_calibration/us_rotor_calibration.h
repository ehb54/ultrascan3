//! \file us_rotor_calibration.h
#ifndef US_ROTOR_CALIBRATION_H
#define US_ROTOR_CALIBRATION_H

#include <QApplication>
#include <QDomDocument>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"
#include "us_dataIO.h"
#include "us_matrix.h"
#include "us_editor.h"

#include <qwt_plot_marker.h>

/**
 * @struct Average
 * @brief Structure to hold average values for rotor calibration.
 */
struct Average
{
    double top;           //!< Top value
    double bottom;        //!< Bottom value
    int    cell;          //!< Cell number
    int    rpm;           //!< RPM value
    int    channel;       //!< Channel number
    int    top_count;     //!< Top count
    int    bottom_count;  //!< Bottom count
};

/**
 * @struct Average_multi
 * @brief Structure to hold average values for multiple rotor calibration scans.
 */
struct Average_multi
{
    double avg;           //!< Radial average of all points belonging to this scan
    int    cell;          //!< Cell number
    int    rpm;           //!< RPM value
    int    channel;       //!< Channel number
    int    index;         //!< Index of the division, zero is the first on the left
    double sigma;         //!< Standard deviation
};

/**
 * @struct SpeedEntry
 * @brief Structure to hold speed entries for rotor calibration.
 */
struct SpeedEntry
{
    QVector <double> diff; //!< Differences
    double speed;          //!< Speed value
    int channel;           //!< Channel number
};

/**
 * @struct Limit
 * @brief Structure to hold limits for rotor calibration.
 */
struct Limit
{
    QwtDoubleRect  rect[2]; //!< Top and bottom of the channel
    bool           used[2]; //!< Used flags for top and bottom
    int            cell;    //!< Cell number
    QString        channel; //!< Channel name
};

/**
 * @class US_RotorCalibration
 * @brief The US_RotorCalibration class provides a user interface for rotor calibration.
 */
class US_RotorCalibration : public US_Widgets
{
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_RotorCalibration.
         */
        US_RotorCalibration();

        US_Disk_DB_Controls* disk_controls;     //!< Radiobuttons for disk/db choice

    private:
        double left;                 //!< Left limit
        double right;                //!< Right limit
        double top;                  //!< Top limit
        double bottom;               //!< Bottom limit
        double coef[3];              //!< Coefficients for calibration
        QVector<double> x;           //!< X values
        QVector<double> y;           //!< Y values
        QVector<double> sd1;         //!< Standard deviations 1
        QVector<double> sd2;         //!< Standard deviations 2
        int maxcell;                 //!< Maximum cell number
        int maxchannel;              //!< Maximum channel number
        int current_triple;          //!< Current triple
        int current_cell;            //!< Current cell
        int minrpm;                  //!< Minimum RPM
        bool top_of_cell;            //!< Top of cell flag
        bool newlimit;               //!< New limit flag
        bool zoomed;                 //!< Zoomed flag

        QString rotor;               //!< Rotor name
        QString fileText;            //!< File text
        QString current_channel;     //!< Current channel

        US_Help showHelp;            //!< Help display object

        QPushButton* pb_reset;       //!< Reset button
        QPushButton* pb_accept;      //!< Accept button
        QPushButton* pb_calculate;   //!< Calculate button
        QPushButton* pb_save;        //!< Save button
        QPushButton* pb_load;        //!< Load button
        QPushButton* pb_view;        //!< View button

        QString workingDir;          //!< Working directory
        QString runID;               //!< Run ID
        QString editID;              //!< Edit ID
        QString dataType;            //!< Data type
        QStringList files;           //!< List of files
        QStringList triples;         //!< List of triples

        QwtPlot* data_plot;          //!< Data plot
        QwtPlotCurve* fit_curve;     //!< Fit curve
        QwtPlotCurve* v_line;        //!< Vertical line
        QwtPlotCurve* minimum_curve; //!< Minimum curve
        QwtPlotGrid* grid;           //!< Plot grid
        QwtPlotMarker* marker;       //!< Plot marker
        QwtCounter* ct_cell;         //!< Cell counter
        QwtCounter* ct_channel;      //!< Channel counter

        US_PlotPicker* pick;         //!< Plot picker
        US_Plot* plot;               //!< Plot

        QLineEdit* le_instructions;  //!< Instructions line edit

        QRadioButton* rb_channel;    //!< Channel radio button
        QRadioButton* rb_top;        //!< Top radio button
        QRadioButton* rb_bottom;     //!< Bottom radio button

        QCheckBox* cb_assigned;      //!< Assigned checkbox
        QCheckBox* cb_6channel;      //!< 6-channel checkbox
        QComboBox* cb_wavelengths;   //!< Wavelengths combo box
        QComboBox* cb_minrpm;        //!< Minimum RPM combo box

        US_DataIO::RawData data;                       //!< Raw data
        QVector<US_DataIO::RawData> allData;           //!< All raw data
        QVector<Average> avg;                          //!< Averages
        QVector<Average_multi> avg_multi;              //!< Multiple averages
        QVector<QVector<double>> reading;              //!< Reading values
        QVector<double> stretch_factors, std_dev;      //!< Stretch factors and standard deviations
        QVector<Limit> limit;                          //!< Limits
        QVector<double> bounds;                        //!< X-limits for multi-channel calibration mask
        QVector<QwtDoubleRect> bounds_rect;            //!< Limits for multi-channel calibration mask

        QwtDoubleRect zoom_mask;                       //!< Zoomed rectangle for multi-channel calibration mask
        QStringList wavelengths;                       //!< List of wavelengths
        int current_wavelength;                        //!< Current wavelength

    private slots:
        void reset(void);                              //!< Slot to reset the calibration
        void changeLambda(int index);                  //!< Slot to change the wavelength
        void changeminrpm(QString minrpm);             //!< Slot to change the minimum RPM
        void source_changed(bool disk);                //!< Slot to handle source change
        void update_disk_db(bool disk);                //!< Slot to update disk/db choice
        void load(void);                               //!< Slot to load data
        void loadDB(void);                             //!< Slot to load data from the database
        void loadDisk(void);                           //!< Slot to load data from disk
        void plotAll(void);                            //!< Slot to plot all data
        void currentRect(QwtDoubleRect rect);          //!< Slot to handle current rectangle selection
        void currentRectf(QRectF rect);                //!< Slot to handle current rectangle selection (QRectF)
        void divide(QwtDoubleRect rect);               //!< Slot to divide the selected rectangle
        void mouse(const QwtDoublePoint& point);       //!< Slot to handle mouse click
        void findTriple(void);                         //!< Slot to find triple
        void next(void);                               //!< Slot to go to next item
        void calculate(void);                          //!< Slot to calculate the calibration
        void calc_6channel(void);                      //!< Slot to calculate the 6-channel calibration
        double findAverage(QwtDoubleRect rect, US_DataIO::RawData data, int channel); //!< Find the average
        void save(void);                               //!< Slot to save the calibration
        void view(void);                               //!< Slot to view the calibration
        void update_used(void);                        //!< Slot to update the used flag
        void use_6channel(void);                       //!< Slot to use the 6-channel calibration
        void update_cell(double cell);                 //!< Slot to update the cell
        void update_channel(double channel);           //!< Slot to update the channel
        void update_position(void);                    //!< Slot to update the position
        void update_plot(void);                        //!< Slot to update the plot
        void help(void)
        {
            showHelp.show_help("manual/rotor_calibration.html");
        }
};

#endif
