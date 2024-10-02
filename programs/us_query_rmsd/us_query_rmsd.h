//! \file us_query_rmsd.h
#ifndef US_QUERY_RMSD_H
#define US_QUERY_RMSD_H

#include "us_widgets.h"
#include "us_passwd.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_dataIO.h"
#include "us_noise.h"
#include "../us_fematch/us_fematch.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

/**
 * @class US_QueryRmsd
 * @brief The US_QueryRmsd class provides functionality to query and analyze RMSD values from the database.
 */
class US_QueryRmsd : public US_Widgets {
    Q_OBJECT

    public:
        /**
         * @brief Constructor for US_QueryRmsd.
         */
        US_QueryRmsd();

    private:
        /**
         * @struct DataBundle
         * @brief Struct to hold data bundle information.
         */
        struct DataBundle {
            QString cell;        //!< Cell identifier
            QString channel;     //!< Channel identifier
            QString lamda;       //!< Wavelength
            QString edit;        //!< Edit identifier
            QString analysis;    //!< Analysis method
            QString method;      //!< Method used
            QString editFile;    //!< Edit file name
            QString rdataFile;   //!< Raw data file name
            int editID;          //!< Edit ID
            int rdataID;         //!< Raw data ID
            int modelID;         //!< Model ID
            int expID;           //!< Experiment ID
            double rmsd;         //!< RMSD value
        };

        QPushButton *pb_simulate;      //!< Simulate button
        int dbg_level;                 //!< Debug level
        double threshold;              //!< RMSD threshold
        QTableWidget *tw_rmsd;         //!< Table widget for RMSD values
        QHeaderView *hheader;          //!< Header view for table
        US_Passwd pw;                  //!< Password object
        US_DB2* dbCon;                 //!< Database connection
        QVector<DataBundle> allData;   //!< Vector of all data bundles
        QVector<int> selIndex;         //!< Vector of selected indices
        QMap<int, US_Model> Models;    //!< Map of DB model ID to Model
        QMap<int, US_DataIO::EditedData> editData;  //!< Map of DB edit ID to EditedData
        QMap<int, US_DataIO::RawData> rawData;      //!< Map of DB raw data ID to RawData

        QStringList methodList;        //!< List of methods
        QStringList editList;          //!< List of edits
        QStringList analysisList;      //!< List of analyses
        QStringList channelList;       //!< List of channels
        QStringList cellList;          //!< List of cells
        QStringList lambdaList;        //!< List of wavelengths

        QLineEdit *le_runid;           //!< Line edit for run ID
        QLineEdit *le_file;            //!< Line edit for file name
        QComboBox *cb_edit;            //!< Combo box for edit
        QComboBox *cb_analysis;        //!< Combo box for analysis
        QComboBox *cb_cell;            //!< Combo box for cell
        QComboBox *cb_channel;         //!< Combo box for channel
        QComboBox *cb_lambda;          //!< Combo box for wavelength
        QComboBox *cb_method;          //!< Combo box for method
        QLineEdit *le_threshold;       //!< Line edit for threshold

        QProgressBar *progress;        //!< Progress bar

        US_FeMatch *fematch;           //!< Pointer to FeMatch object

        /**
         * @brief Check the database connection.
         * @return True if connected, false otherwise
         */
        bool check_connection(void);

        /**
         * @brief Clear all data.
         */
        void clear_data(void);

        /**
         * @brief Check the content of a combo box.
         * @param comboBox Pointer to the combo box
         * @param content String content to check
         * @return True if content exists, false otherwise
         */
        bool check_combo_content(QComboBox* comboBox, QString& content);

        /**
         * @brief Highlight selected rows in the table.
         */
        void highlight(void);

        /**
         * @brief Get metadata for a data bundle.
         * @param dataBundle Reference to the data bundle
         * @param message Reference to the message string
         * @return True if successful, false otherwise
         */
        bool get_metadata(DataBundle& dataBundle, QString& message);

        /**
         * @brief Load data for a specific ID.
         * @param id Data ID
         * @param message Reference to the message string
         * @return True if successful, false otherwise
         */
        bool load_data(int id, QString& message);

    protected:
        /**
         * @brief Event handler for close events.
         * @param event Pointer to the close event
         */
        void closeEvent(QCloseEvent* event) override;

    private slots:
                void load_runid(void);
        void fill_table(int);
        void set_analysis(int);
        void set_method(int);
        void set_triple(int);
        void save_data(void);
        void simulate(void);
        void new_threshold(void);
        void update_progress(int);
};

/**
 * @class DoubleTableWidgetItem
 * @brief The DoubleTableWidgetItem class provides a table widget item for double values.
 */
class DoubleTableWidgetItem : public QTableWidgetItem {
    public:
        /**
         * @brief Constructor for DoubleTableWidgetItem.
         * @param value The double value
         */
        DoubleTableWidgetItem(double value) : QTableWidgetItem(QString::number(value, 'f', 8)), m_value(value) {}

        /**
         * @brief Less than operator for comparing table widget items.
         * @param other Reference to the other table widget item
         * @return True if less than, false otherwise
         */
        bool operator<(const QTableWidgetItem &other) const override
        {
            return m_value < other.data(Qt::EditRole).toDouble();
        }

        /**
         * @brief Get the double value.
         * @return The double value
         */
        double get_value()
        {
            return m_value;
        }

    private:
        double m_value; //!< The double value
};

#endif // US_QUERY_RMSD_H
