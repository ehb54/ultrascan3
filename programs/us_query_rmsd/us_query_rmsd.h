#ifndef US_QUERY_RMSD_H
#define US_QUERY_RMSD_H

#include "us_widgets.h"
#include "us_passwd.h"
#include "us_db2.h"

#ifndef DbgLv
#define DbgLv(a) if(dbg_level>=a)qDebug()
#endif

class US_QueryRmsd : public US_Widgets{
    Q_OBJECT

    public:
    US_QueryRmsd();

    private:
    QTableWidget *tw_rmsd;
    US_Passwd pw;
    US_DB2* dbCon;
    QStringList allCell;
    QStringList allChannel;
    QStringList allLambda;
    QStringList allEdit;
    QStringList allAnalysis;
    QStringList allMethod;
    QVector<double> allRmsd;

    QStringList methodList;
    QStringList editList;
    QStringList analysisList;
    QStringList channelList;
    QStringList cellList;
    QStringList lambdaList;

    QLineEdit *le_runid;
    QComboBox *cb_edit;
    QComboBox *cb_analysis;
    QComboBox *cb_cell;
    QComboBox *cb_channel;
    QComboBox *cb_lambda;
    QComboBox *cb_method;


    void check_connection(void);
    void clear_data(void);
    void fill_combos(void);

    private slots:
    void load_runid(void);
    void fill_table(int);

};
#endif
