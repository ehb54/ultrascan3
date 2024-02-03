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

class US_QueryRmsd : public US_Widgets{
    Q_OBJECT

    public:
    US_QueryRmsd();

    private:
    int dbg_level;
    double threshold;
    QTableWidget *tw_rmsd;
    QHeaderView *hheader;
    US_Passwd pw;
    US_DB2* dbCon;
    QStringList allCell;
    QStringList allChannel;
    QStringList allLambda;
    QStringList allEdit;
    QVector<int> allEditIds;
    QStringList allAnalysis;
    QStringList allMethod;
    QVector<double> allRmsd;
    QVector<int> allModelIDs;
    QVector<int> selIndex;
    QMap<int, US_Model *> Models;  //DB model id -> Model
    QMap<int, US_Noise *> TI_Noise;  //DB model id -> TI_Noise
    QMap<int, US_Noise *> RI_Noise;  //DB model id -> RI_Noise
    QMap<int, US_DataIO::EditedData> editData;  //DB edit id -> EditedData
//    QMap<int, US_DataIO::RawData*> rawData;      //DB edit id -> RawData
    int n_data;

    QStringList methodList;
    QStringList editList;
    QStringList analysisList;
    QStringList channelList;
    QStringList cellList;
    QStringList lambdaList;

    QLineEdit *le_runid;
    QLineEdit *le_file;
    QComboBox *cb_edit;
    QComboBox *cb_analysis;
    QComboBox *cb_cell;
    QComboBox *cb_channel;
    QComboBox *cb_lambda;
    QComboBox *cb_method;
    QLineEdit *le_threshold;

    US_FeMatch *fematch;

    bool check_connection(void);
    void clear_data(void);
    bool check_combo_content(QComboBox*, QString&);
    void highlight(void);
//    bool load_model(QString, US_Model*);
    bool loadData(void);
    bool loadNoises(void);

    private slots:
    void load_runid(void);
    void fill_table(int);
    void set_analysis(int);
    void set_method(int);
    void set_triple(int);
    void save_data(void);
    void simulate(void);
    void new_threshold(void);
};

class DoubleTableWidgetItem : public QTableWidgetItem
{
public:
    DoubleTableWidgetItem(double value) : QTableWidgetItem(QString::number(value, 'f', 8)), m_value(value) {}

    bool operator<(const QTableWidgetItem &other) const override
    {
        return m_value < other.data(Qt::EditRole).toDouble();
    }

    double get_value()
    {
        return m_value;
    }


private:
    double m_value;

};
#endif
