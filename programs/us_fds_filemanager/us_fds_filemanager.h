//! \file us_fds_filemanager.h
//! \brief Contains the declaration of the US_FDS_FileManager class and its members.
#ifndef US_FDS_FILEMANAGER_H
#define US_FDS_FILEMANAGER_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_help.h"
#include "us_plot.h"

#include "qwt_plot_marker.h"

//! \struct ScanInfo
//! \brief Structure to hold information about a scan.
struct ScanInfo
{
    int voltage; //!< Voltage value.
    int gain; //!< Gain value.
    int range; //!< Range value.
    int rpm; //!< RPM value.
    int cell; //!< Cell number.
    int lambda; //!< Lambda value.
    QString filename; //!< Filename of the scan.
    QString triple; //!< Triple name.
    QString date; //!< Date of the scan.
    QString time; //!< Time of the scan.
    QString channel; //!< Channel name.
    QString gainset; //!< Gain set.
    double omega_s; //!< Omega S value.
    double seconds; //!< Seconds value.
    bool include; //!< Flag to indicate if the scan is included.
    QVector<double> x; //!< X values of the scan.
    QVector<double> y; //!< Y values of the scan.
};

//! \struct TripleIndex
//! \brief Structure to hold the index of a triple.
struct TripleIndex
{
    QString name; //!< Name of the triple.
    int index; //!< Index of the triple.
};

//! \class US_FDS_FileManager
//! \brief A class for managing FDS files in UltraScan.
class US_FDS_FileManager : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for the US_FDS_FileManager class.
        US_FDS_FileManager();

    private:
        QProgressBar* progress; //!< Progress bar.
        QStringList files; //!< List of files.
        QString source_dir; //!< Source directory.
        QString undo_triple; //!< Undo triple.
        QString prefix; //!< Prefix string.
        QVector<ScanInfo> scaninfo; //!< Vector of scan information.
        QList<int> scanindex; //!< List of scan indices corresponding to scaninfo objects.
        QList<int> tmp_scanindex; //!< Temporary copy of scanindex for undo.
        QList<TripleIndex> tripleCounts; //!< List of triple counts.
        ScanInfo tmp_scaninfo; //!< Temporary scan information.
        int current_rpm; //!< Current RPM value.
        int current_triple; //!< Current triple index.
        int current_scan; //!< Current scan index.
        int current_gain; //!< Current gain value.
        int from; //!< Start index for scan range.
        int to; //!< End index for scan range.

        US_Help showHelp; //!< Help object.
        QString workingDir; //!< Working directory.
        QwtPlot* data_plot; //!< Data plot.
        US_Plot* plot; //!< Plot object.

        QLabel* lbl_rpms; //!< Label for RPMs.
        QLabel* lbl_triple; //!< Label for triple.
        QLabel* lbl_gains; //!< Label for gains.
        QLabel* lbl_progress; //!< Label for progress.
        QLabel* lbl_from; //!< Label for start index.
        QLabel* lbl_to; //!< Label for end index.
        QLabel* lbl_scans; //!< Label for scans.
        QLabel* lbl_prefix; //!< Label for prefix.

        QLineEdit* le_info; //!< Line edit for information.
        QLineEdit* le_directory; //!< Line edit for directory.
        QLineEdit* le_scans; //!< Line edit for scans.

        QComboBox* cb_triple; //!< Combo box for triples.
        QComboBox* cb_rpms; //!< Combo box for RPMs.
        QComboBox* cb_gains; //!< Combo box for gains.

        QPushButton* pb_write; //!< Button to write data.
        QPushButton* pb_exclude; //!< Button to exclude scans.
        QPushButton* pb_undo; //!< Button to undo last action.
        QPushButton* pb_delete_all; //!< Button to delete all scans.
        QPushButton* pb_delete_triple; //!< Button to delete triple scans.
        QPushButton* pb_save_first; //!< Button to save first scan.
        QPushButton* pb_save_last; //!< Button to save last scan.
        QPushButton* pb_save_first_and_last; //!< Button to save first and last scans.

        QwtCounter* ct_to; //!< Counter for end index.
        QwtCounter* ct_from; //!< Counter for start index.
        QwtCounter* ct_prefix; //!< Counter for prefix.

    private slots:
        //! \brief Load files and scan data.
        void load(void);

        //! \brief Select a triple from the list.
        //! \param index The index of the selected triple.
        void select_triple(int index);

        //! \brief Select an RPM value.
        //! \param index The index of the selected RPM.
        void select_rpm(int index);

        //! \brief Select a gain value.
        //! \param index The index of the selected gain.
        void select_gain(int index);

        //! \brief Focus on the start index.
        //! \param value The start index value.
        void focus_from(double value);

        //! \brief Focus on the end index.
        //! \param value The end index value.
        void focus_to(double value);

        //! \brief Update the prefix value.
        //! \param value The new prefix value.
        void update_prefix(double value);

        //! \brief Focus on a range of scans.
        //! \param from The start index.
        //! \param to The end index.
        void focus(int from, int to);

        //! \brief Reset the form.
        void reset(void);

        //! \brief Write data to file.
        void write(void);

        //! \brief Undo the last action.
        void undo(void);

        //! \brief Exclude selected scans.
        void exclude_scans(void);

        //! \brief Delete all scans.
        void delete_all(void);

        //! \brief Delete scans for the selected triple.
        void delete_triple(void);

        //! \brief Save the first scan.
        void save_first(void);

        //! \brief Save the last scan.
        void save_last(void);

        //! \brief Save the first and last scans.
        void save_first_and_last(void);

        //! \brief Plot the scans.
        void plot_scans(void);

        //! \brief Parse the files to extract scan data.
        void parse_files(void);

        //! \brief Activate the undo function.
        void activate_undo(void);

        //! \brief Activate the undo function with a message.
        //! \param message The undo message.
        void activate_undo(QString message);

        //! \brief Show help documentation.
        void help(void)
        {
            showHelp.show_help("manual/us_fds_filemanager.html");
        };
};

#endif // US_FDS_FILEMANAGER_H
