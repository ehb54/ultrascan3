//! \file us_legacy_converter.h
#ifndef US_LEGACY_CONVERTER
#define US_LEGACY_CONVERTER

#include <QApplication>
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_settings.h"
#include "us_archive.h"
// #include "us_tar.h"
// #include "us_gzip.h"
#include "../us_convert/us_convert.h"

//! \brief Class for converting legacy data to a modern format
class US_LegacyConverter : public US_Widgets
{
    Q_OBJECT

    public:
        //! \brief Constructor for US_LegacyConverter
        US_LegacyConverter();

    private:
        QMap< QString, QString > data_types;                //!< Map of data types
        QVector< US_DataIO::RawData > all_data;       //!< Map of all raw data
        QVector< US_Convert::TripleInfo > all_triples; //!< Map of all triples
        QHash< int, QHash< QString, QVector< int > > > output_index; // speed -> runType -> QVector(data index)
        QHash< int, QHash< QString, QString > > output_types; // speed -> runType -> runType out

        QLabel *lb_runid;           //!< Label for run ID
        QLineEdit *le_load;         //!< Line edit for load path
        QTextEdit *te_info;         //!< Text edit for information display
        QPushButton *pb_load;       //!< Button to load data
        QPushButton *pb_save;       //!< Button to save data
        US_LineEdit_RE *le_runid;   //!< Line edit for run ID with regular expression validation
        QLineEdit *le_dir;          //!< Line edit for directory
        US_Archive* archive;        //!< Archive object
        int counter;                //!< counter to update test edit
        bool exists;             //!< if it's true, runIDs are overwritten on disk

        //! \brief Resets the converter to its initial state
        void reset(void);

        //! \brief Parses all data from the given file
        //! \param filename Name of the file to parse
        void parse_all(const QString& filename);

        //! \brief Lists all files in the given directory
        //! \param dirname Name of the directory
        //! \param filelist List of files in the directory
        void list_files(const QString& dirname, QStringList& filelist);

        //! \brief Sorts the files based on the given criteria
        //! \param filelist List of files to be sorted
        //! \param criteria Criteria for sorting the files
        //! \return true if sorting is successful, false otherwise
        bool sort_files(const QStringList& filelist, const QString& criteria);

        //! \brief Reads Beckman files
        //! \param filepath Path to the Beckman files
        //! \param data Data read from the Beckman files
        //! \return true if reading is successful, false otherwise
        bool read_beckman_files(const QString& filepath, QString& data);

    private slots:
        //! \brief Slot to load data
        void load(void);

        //! \brief Slot to update the run ID
        void runid_updated(void);

        //! \brief Slot to save AUC data
        void save_auc(void);

        //! \brief Slot to update text edit when a file extracted from the archive file
        //! \param relative path
        //! \param absolute path
        void itemExtracted(const QString&, const QString&);
};

#endif // US_LEGACY_CONVERTER
