//! \file us_query_rmsd.cpp

#include <QApplication>

#include "us_query_rmsd.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_select_runs.h"


//! \brief Main program. Loads the Model RMSD values from DB
int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.
   
   US_QueryRmsd w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_QueryRmsd::US_QueryRmsd() : US_Widgets()
{
    setWindowTitle( tr( "Query Model RMSDs" ) );
    setPalette( US_GuiSettings::frameColor() );

    dbg_level = US_Settings::us_debug();
    dbCon = new US_DB2();

    QPushButton *pb_load_runid = us_pushbutton(tr("Load Run ID"));

    QLabel *lb_runid = us_label(tr("Run ID:"));
    le_runid = us_lineedit(tr(""), 0, true);

    QLabel *lb_edit = us_label(tr("Edit:"));
    cb_edit = us_comboBox();

    QLabel *lb_analysis = us_label(tr("Analysis:"));
    cb_analysis = us_comboBox();

    QLabel *lb_cell = us_label(tr("Cell:"));
    cb_cell = us_comboBox();

    QLabel *lb_channel = us_label(tr("Channel:"));
    cb_channel = us_comboBox();

    QLabel *lb_lambda = us_label(tr("Lambda:"));
    cb_lambda = us_comboBox();

    QLabel *lb_method = us_label(tr("Method:"));
    cb_method = us_comboBox();

    tw_rmsd = new QTableWidget();
    tw_rmsd->setRowCount(0);
    tw_rmsd->setColumnCount(2);
    tw_rmsd-> setHorizontalHeaderLabels(QStringList{"Triple_Method_Analysis", "RMSD"});
    tw_rmsd->setStyleSheet("background-color: white");
    QHeaderView *header = tw_rmsd->horizontalHeader();
    header->setSectionResizeMode(header->logicalIndexAt(0), QHeaderView::ResizeToContents);
    header->setSectionResizeMode(header->logicalIndexAt(1), QHeaderView::Stretch);

    QGridLayout *lyt_top = new QGridLayout();
    lyt_top->addWidget(pb_load_runid, 0, 0, 1, 2);
    lyt_top->addWidget(lb_runid,      0, 2, 1, 1);
    lyt_top->addWidget(le_runid,      0, 3, 1, 3);

    lyt_top->addWidget(lb_edit,       1, 0, 1, 1);
    lyt_top->addWidget(cb_edit,       1, 1, 1, 1);
    lyt_top->addWidget(lb_analysis,   1, 2, 1, 1);
    lyt_top->addWidget(cb_analysis,   1, 3, 1, 1);
    lyt_top->addWidget(lb_method,     1, 4, 1, 1);
    lyt_top->addWidget(cb_method,     1, 5, 1, 1);

    lyt_top->addWidget(lb_cell,       2, 0, 1, 1);
    lyt_top->addWidget(cb_cell,       2, 1, 1, 1);
    lyt_top->addWidget(lb_channel,    2, 2, 1, 1);
    lyt_top->addWidget(cb_channel,    2, 3, 1, 1);
    lyt_top->addWidget(lb_lambda,     2, 4, 1, 1);
    lyt_top->addWidget(cb_lambda,     2, 5, 1, 1);

    lyt_top->setColumnStretch(0, 0);
    lyt_top->setColumnStretch(1, 1);
    lyt_top->setColumnStretch(2, 0);
    lyt_top->setColumnStretch(3, 1);
    lyt_top->setColumnStretch(4, 0);
    lyt_top->setColumnStretch(5, 1);
    lyt_top->setMargin(0);
    lyt_top->setSpacing(1);

    QLabel *lb_file = us_label(tr("Output Filename:"));
    le_file = us_lineedit(tr(""), 0, false);
    QPushButton *pb_save = us_pushbutton(tr("Save Data"));

    QHBoxLayout *lyt_bottom = new QHBoxLayout();
    lyt_bottom->addWidget(lb_file,0);
    lyt_bottom->addWidget(le_file,5);
    lyt_bottom->addWidget(pb_save,1);
    lyt_bottom->setMargin(0);
    lyt_bottom->setSpacing(1);

    QVBoxLayout *lyt_main = new QVBoxLayout();
    lyt_main->addLayout(lyt_top);
    lyt_main->addWidget(tw_rmsd);
    lyt_main->addLayout(lyt_bottom);
    lyt_main->setMargin(1);
    lyt_main->setSpacing(1);

    this->setLayout(lyt_main);
    this->setMinimumSize(QSize(500,400));

    connect(pb_load_runid, SIGNAL(clicked()), this, SLOT(load_runid()));
    connect(pb_save, SIGNAL(clicked()), this, SLOT(save_data()));
}

void US_QueryRmsd::check_connection(){

    QString error;
    dbCon->connect(pw.getPasswd(), error);
    if (dbCon->isConnected()){
        QStringList DB   = US_Settings::defaultDB();
        DbgLv(1) << DB;
    }
    return;
}

void US_QueryRmsd::clear_data(){
    allRmsd.clear();
    allEdit.clear();
    allAnalysis.clear();
    allMethod.clear();
    allCell.clear();
    allChannel.clear();
    allLambda.clear();

    editList.clear();
    analysisList.clear();
    methodList.clear();
    cellList.clear();
    channelList.clear();
    lambdaList.clear();
}

void US_QueryRmsd::load_runid(){
    QStringList runList;
    US_SelectRuns *selrun = new US_SelectRuns(true, runList);
    selrun->setMinimumSize(QSize(500, 500));
    selrun->exec();
    if (runList.size() == 0)
        return;
    check_connection();

    QString runId = runList.at(0);

    QStringList q;
    q.clear();
    q << "get_model_desc_by_runID";
    q << QString::number(US_Settings::us_inv_ID());
    q << runId;
    qDebug() << q;

    dbCon->query( q );

    if ( dbCon->lastErrno() == US_DB2::OK ){
        clear_data();
        while (dbCon->next()) {
            QString desc = dbCon->value(2).toString();
            QStringList list1 = desc.split(u'.');
            QString cell = list1.at(1).at(0);
            QString channel = list1.at(1).at(1);
            QString lambda = list1.at(1).mid(2);
            QStringList list2 = list1.at(2).split(u'_');
            QString edit = list2.at(0);
            QString analysis = list2.at(1);
            QString method = list2.at(2);

            allCell << cell;
            allChannel << channel;
            allLambda << lambda;
            allEdit << edit;
            allAnalysis << analysis;
            allMethod << method;

            if (! editList.contains(edit))
                editList << edit;

            double var = dbCon->value(3).toDouble();
            allRmsd << qSqrt(var);
        }
    }else{
        QMessageBox::warning(this, "Error", dbCon->lastError());
        return;
    }

    editList.sort();

    le_runid->setText(runId);
    cb_edit->disconnect();
    cb_edit->clear();
    foreach (QString item, editList)
        cb_edit->addItem(item);
    cb_edit->setCurrentIndex(cb_edit->count() - 1);
    connect(cb_edit,     SIGNAL(currentIndexChanged(int)), this, SLOT(set_analysis(int)));
    set_analysis(cb_edit->count() - 1);
}

bool US_QueryRmsd::check_combo_content(QComboBox* combo, QString& text){
    int cc = combo->count();
    if (cc == 0)
        return false;
    if (cc == 1){
        if (text.compare(combo->currentText()) == 0)
            return true;
        else
            return false;
    } else {
        if (combo->currentIndex() == 0)
            return true;
        else{
            if (text.compare(combo->currentText()) == 0)
                return true;
            else
                return false;
        }

    }
}

void US_QueryRmsd::set_analysis(int){
    QString edit = cb_edit->currentText();
    analysisList.clear();
    for(int i = 0; i < allAnalysis.size(); i++) {
        if (edit.compare(allEdit.at(i)) == 0){
            QString analysis = allAnalysis.at(i);
            if (! analysisList.contains(analysis))
                analysisList << analysis;
        }
    }
    analysisList.sort();
    cb_analysis->disconnect();
    cb_analysis->clear();
    if (analysisList.size() > 1)
        cb_analysis->addItem("ALL");
    foreach (QString item, analysisList)
        cb_analysis->addItem(item);
    cb_analysis->setCurrentIndex(0);
    connect(cb_analysis, SIGNAL(currentIndexChanged(int)), this, SLOT(set_method(int)));
    set_method(0);
}

void US_QueryRmsd:: set_method(int){

    methodList.clear();
    for(int i = 0; i < allAnalysis.size(); i++) {
        QString edit = allEdit.at(i);
        QString analysis = allAnalysis.at(i);
        if (edit.compare(cb_edit->currentText()) != 0)
            continue;
        if (! check_combo_content(cb_analysis, analysis))
            continue;
        QString method = allMethod.at(i);
        if (! methodList.contains(method))
            methodList << method;
    }
    methodList.sort();
    cb_method->disconnect();
    cb_method->clear();
    if (methodList.size() > 1)
        cb_method->addItem("ALL");
    foreach (QString item, methodList)
        cb_method->addItem(item);
    cb_method->setCurrentIndex(0);
    connect(cb_method, SIGNAL(currentIndexChanged(int)), this, SLOT(set_triple(int)));
    set_triple(0);
}

void US_QueryRmsd::set_triple(int){
    cellList.clear();
    channelList.clear();
    lambdaList.clear();

    for (int i = 0; i < allRmsd.size(); i++){
        QString edit = allEdit.at(i);
        QString analysis = allAnalysis.at(i);
        QString method = allMethod.at(i);
        if (edit.compare(cb_edit->currentText()) != 0)
            continue;
        if (! check_combo_content(cb_analysis, analysis))
            continue;
        if (! check_combo_content(cb_method, method))
            continue;

        QString cell = allCell.at(i);
        QString channel = allChannel.at(i);
        QString lambda = allLambda.at(i);
        if (! cellList.contains(cell))
            cellList << cell;
        if (! channelList.contains(channel))
            channelList << channel;
        if (! lambdaList.contains(lambda))
            lambdaList << lambda;
    }
    cellList.sort();
    channelList.sort();
    lambdaList.sort();
    cb_cell->disconnect();
    cb_channel->disconnect();
    cb_lambda->disconnect();
    cb_cell->clear();
    cb_channel->clear();
    cb_lambda->clear();
    if (cellList.size() > 1)
        cb_cell->addItem("ALL");
    if (channelList.size() > 1)
        cb_channel->addItem("ALL");
    if (lambdaList.size() > 1)
        cb_lambda->addItem("ALL");
    foreach (QString item, cellList)
        cb_cell->addItem(item);
    cb_cell->setCurrentIndex(0);

    foreach (QString item, channelList)
        cb_channel->addItem(item);
    cb_channel->setCurrentIndex(0);

    foreach (QString item, lambdaList)
        cb_lambda->addItem(item);
    cb_lambda->setCurrentIndex(0);

    connect(cb_cell,     SIGNAL(currentIndexChanged(int)), this, SLOT(fill_table(int)));
    connect(cb_channel,  SIGNAL(currentIndexChanged(int)), this, SLOT(fill_table(int)));
    connect(cb_lambda,   SIGNAL(currentIndexChanged(int)), this, SLOT(fill_table(int)));
    fill_table(0);

}

void US_QueryRmsd::fill_table(int){
    QFont tw_font( US_Widgets::fixedFont().family(),
                  US_GuiSettings::fontSize() );
    QFontMetrics* fm = new QFontMetrics( tw_font );
    int rowht        = fm->height() + 2;
    tw_rmsd->clearContents();
    tw_rmsd->setSortingEnabled( true );
    tw_rmsd->setRowCount(allRmsd.size());

    int n = 0;
    for (int i = 0; i < allRmsd.size(); i++){
        double rmsd = allRmsd.at(i);
        QString edit = allEdit.at(i);
        QString analysis = allAnalysis.at(i);
        QString method = allMethod.at(i);
        QString cell = allCell.at(i);
        QString channel = allChannel.at(i);
        QString lambda = allLambda.at(i);
        QString desc = tr("%1%2%3_%4_%5").arg(cell, channel,lambda, method, analysis);
        if (edit.compare(cb_edit->currentText()) != 0)
            continue;
        if (! check_combo_content(cb_analysis, analysis))
            continue;
        if (! check_combo_content(cb_method, method))
            continue;
        if (! check_combo_content(cb_cell, cell))
            continue;
        if (! check_combo_content(cb_channel, channel))
            continue;
        if (! check_combo_content(cb_lambda, lambda))
            continue;

        QTableWidgetItem *twi;

        twi = new QTableWidgetItem(desc);
        twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
        twi->setFont(tw_font);
        tw_rmsd->setItem(n, 0, twi);

        twi = new QTableWidgetItem(QString::number(rmsd));
        twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
        twi->setFont(tw_font);
        tw_rmsd->setItem(n, 1, twi);

        tw_rmsd->setRowHeight(n, rowht);
        n++;
    }
    tw_rmsd->setRowCount(n);
    tw_rmsd->verticalHeader()->setFont(tw_font);
    tw_rmsd->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    tw_rmsd->sortItems(2, Qt::DescendingOrder);
    QTableWidgetItem *item = tw_rmsd->item(0, 0);
    item->setBackground(Qt::yellow);
    item = tw_rmsd->item(0, 1);
    item->setBackground(Qt::yellow);

    QString fname("RMSD_%1_%2_%3_%4-%5-%6.dat");
    le_file->setText(fname.arg(cb_edit->currentText(), cb_analysis->currentText(),
                               cb_method->currentText(), cb_cell->currentText(),
                               cb_channel->currentText(), cb_lambda->currentText()));

}

void US_QueryRmsd::save_data(){
    int nRows = tw_rmsd->rowCount();
    if( nRows == 0){
        QMessageBox::warning(this, tr("Warning!"), tr("RMSD data not found!"));
        return;
    }
    if (le_file->text().isEmpty()){
        QMessageBox::warning(this, tr("Warning!"), tr("Give a name to the file, then try again!"));
        return;
    }

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    US_Settings::reportDir(),
                                                    QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
        return;

    QFileInfo finfo = QFileInfo(QDir(dir), le_file->text());
    QFile file{finfo.absoluteFilePath()};
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream outStream{&file};
        outStream << tr("Triple_Method_Analysis,RMSD\n");
        for (int i = 0; i < nRows; i++){
            QString desc = tw_rmsd->item(i, 0)->text();
            QString rmsd = tw_rmsd->item(i, 1)->text();
            outStream << desc << "," << rmsd << "\n";
        }
    }
    file.close();

}
