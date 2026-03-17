//! \file us_query_rmsd.cpp

#include <QApplication>

#include "us_query_rmsd.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_select_runs.h"
#include <QTemporaryDir>


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
   setMinimumSize(QSize(600,400));
   
   dbg_level = US_Settings::us_debug();
   dbCon = new US_DB2();
   
   QLabel *lb_runid = us_label(tr("Run ID"));
   QLabel *lb_edit = us_label(tr("Edit"));
   QLabel *lb_analysis = us_label(tr("Analysis"));
   QLabel *lb_cell = us_label(tr("Cell"));
   QLabel *lb_channel = us_label(tr("Channel"));
   QLabel *lb_lambda = us_label(tr("Lambda"));
   QLabel *lb_method = us_label(tr("Method"));
   QLabel *lb_trsh = us_label(tr("RMSD Threshold"));
   QLabel *lb_file = us_label(tr("Output Filename"));
   QLabel* lb_progress = us_label(tr("Progress"));

   lb_runid->setAlignment(Qt::AlignCenter);
   lb_edit->setAlignment(Qt::AlignCenter);
   lb_analysis->setAlignment(Qt::AlignCenter);
   lb_cell->setAlignment(Qt::AlignCenter);
   lb_channel->setAlignment(Qt::AlignCenter);
   lb_lambda->setAlignment(Qt::AlignCenter);
   lb_method->setAlignment(Qt::AlignCenter);
   lb_trsh->setAlignment(Qt::AlignCenter);
   lb_file->setAlignment(Qt::AlignCenter);
   lb_progress->setAlignment(Qt::AlignCenter);

   cb_edit = us_comboBox();
   cb_analysis = us_comboBox();
   cb_cell = us_comboBox();
   cb_channel = us_comboBox();
   cb_lambda = us_comboBox();
   cb_method = us_comboBox();
   
   le_runid = us_lineedit(tr(""), 0, true);
   le_threshold = us_lineedit("", 0);
   le_file = us_lineedit(tr(""), 0, false);
   
   QPushButton *pb_load_runid = us_pushbutton(tr("Load Run ID"));
   QPushButton *pb_save = us_pushbutton(tr("Save Data"));
   QPushButton *pb_help = us_pushbutton(tr("Help"));
   pb_simulate = us_pushbutton("Simulate");
   
   progress = us_progressBar( 0, 100, 0 );

   tw_rmsd = new QTableWidget();
   tw_rmsd->setRowCount(0);
   tw_rmsd->setColumnCount(5);
   tw_rmsd-> setHorizontalHeaderLabels(QStringList{"Triple_Method_Analysis", "RMSD"});
   tw_rmsd-> setHorizontalHeaderLabels(QStringList{"Edit", "Analysis", "Method", "Triple", "RMSD"});
   hheader = tw_rmsd->horizontalHeader();
   tw_rmsd->setStyleSheet("background-color: white");
   QHeaderView *header = tw_rmsd->horizontalHeader();
   header->setSectionResizeMode(QHeaderView::Stretch);

   QGridLayout *layout = new QGridLayout();
   int row = 0;
   layout->addWidget(pb_load_runid, row,   0, 1, 2);
   layout->addWidget(lb_runid,      row,   2, 1, 1);
   layout->addWidget(le_runid,      row++, 3, 1, 3);

   layout->addWidget(lb_edit,       row,   0, 1, 1);
   layout->addWidget(cb_edit,       row,   1, 1, 1);
   layout->addWidget(lb_analysis,   row,   2, 1, 1);
   layout->addWidget(cb_analysis,   row,   3, 1, 1);
   layout->addWidget(lb_method,     row,   4, 1, 1);
   layout->addWidget(cb_method,     row++, 5, 1, 1);

   layout->addWidget(lb_cell,       row,   0, 1, 1);
   layout->addWidget(cb_cell,       row,   1, 1, 1);
   layout->addWidget(lb_channel,    row,   2, 1, 1);
   layout->addWidget(cb_channel,    row,   3, 1, 1);
   layout->addWidget(lb_lambda,     row,   4, 1, 1);
   layout->addWidget(cb_lambda,     row++, 5, 1, 1);

   layout->addWidget(tw_rmsd,       row++, 0, 1, 6);
   
   layout->addWidget(pb_simulate,   row,   0, 1, 2);
   layout->addWidget(lb_trsh,       row,   2, 1, 1);
   layout->addWidget(le_threshold,  row++, 3, 1, 3);

   layout->addWidget(pb_save,       row,   0, 1, 2);
   layout->addWidget(lb_file,       row,   2, 1, 1);
   layout->addWidget(le_file,       row++, 3, 1, 3);

   layout->addWidget(pb_help,       row,   0, 1, 2);
   layout->addWidget(lb_progress,   row,   2, 1, 1);
   layout->addWidget(progress,      row++, 3, 1, 3);

   layout->setColumnStretch(0, 1);
   layout->setColumnStretch(1, 2);
   layout->setColumnStretch(2, 1);
   layout->setColumnStretch(3, 2);
   layout->setColumnStretch(4, 1);
   layout->setColumnStretch(5, 2);
   layout->setContentsMargins(1, 1, 1, 1);
   layout->setSpacing(1);

   this->setLayout(layout);

   fematch = new US_FeMatch();

   connect(pb_load_runid, SIGNAL(clicked()), this, SLOT(load_runid()));
   connect(pb_save, SIGNAL(clicked()), this, SLOT(save_data()));
   connect(pb_help, SIGNAL(clicked()), this, SLOT(help()));
   connect(pb_simulate, SIGNAL(clicked()), this, SLOT(simulate()));
   connect(le_threshold, SIGNAL(editingFinished()), this, SLOT(new_threshold()));
   connect(fematch, SIGNAL(astfem_cmp(int)), SLOT(update_progress(int)));
}

void US_QueryRmsd::help() {
   showHelp.show_help("manual/rmsd_query.html");
}

bool US_QueryRmsd::check_connection(){

   QString error;
   dbCon->connect(pw.getPasswd(), error);
   if (dbCon->isConnected()){
      QStringList DB   = US_Settings::defaultDB();
      DbgLv(1) << DB;
      return true;
   } else{
      QApplication::restoreOverrideCursor();
      QMessageBox::information( this,
                               tr( "DB Connection Problem" ),
                               tr( "There was an error connecting to the database:\n" )
                                   + dbCon->lastError() );
      return false;
   }
}

void US_QueryRmsd::clear_data(){
   allData.clear();
   threshold = -1;
   Models.clear();
   selIndex.clear();
   editData.clear();
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
   if (! check_connection())
      return;

   QString runId = runList.at(0);

   QStringList q;
   q.clear();
   q << "get_model_desc_by_runID";
   q << QString::number(US_Settings::us_inv_ID());
   q << runId;
   qDebug() << q;

   dbCon->query( q );

   QStringList modelIDs_tmp;
   QVector<int> editIDs_tmp;
   if ( dbCon->lastErrno() == US_DB2::OK ){
      while (dbCon->next()){
         modelIDs_tmp << dbCon->value(0).toString();
         editIDs_tmp << dbCon->value(6).toInt();
      }
   }else{
      QMessageBox::warning(this, "Error", dbCon->lastError());
      return;
   }
   clear_data();

   for (int i = 0; i < modelIDs_tmp.size(); i++){
      US_Model model = US_Model();
      int mId = modelIDs_tmp.at(i).toInt();
      int state = model.load(modelIDs_tmp.at(i), dbCon);
      if (state == US_DB2::OK){
         DataBundle data;
         QStringList list1 = model.description.split(u'.');
         data.cell = list1.at(1).at(0);
         data.channel = list1.at(1).at(1);
         data.lamda = list1.at(1).mid(2);
         QStringList list2 = list1.at(2).split(u'_');
         data.edit = list2.at(0);
         data.analysis = list2.at(1);
         data.method = list2.at(2);
         data.editID = editIDs_tmp.at(i);
         data.modelID = mId;
         data.rmsd = qSqrt(model.variance);

         QString mesg;
         if (! get_metadata(data, mesg)) {
            qDebug() << mesg;
            continue;
         }
         if (! editList.contains(data.edit))
            editList << data.edit;
         allData << data;
         Models.insert(mId, model);
      }
   }

   // loadData();
   editList.sort();

   le_runid->setText(runId);
   cb_edit->disconnect();
   cb_edit->clear();
   if (editList.size() > 1)
      cb_edit->addItem("ALL");
   foreach (QString item, editList)
      cb_edit->addItem(item);
   cb_edit->setCurrentIndex(0);
   connect(cb_edit,     SIGNAL(currentIndexChanged(int)), this, SLOT(set_analysis(int)));
   set_analysis(cb_edit->count() - 1);
}

bool US_QueryRmsd::get_metadata(DataBundle& data, QString& mesg) {
   if (! check_connection()){
      mesg = "Error: DB connection";
      return false;
   }
   QStringList query;
   query << "get_editedData" << QString::number(data.editID);
   dbCon->query(query);
   if ( dbCon->lastErrno() == US_DB2::OK ){
      if (dbCon->next()){
         data.rdataID = dbCon->value(0).toInt();
         data.editFile = dbCon->value(3).toString();
      }
   }else{
      mesg = tr("Error: get_editedData: ID: %1\n%2").
             arg(data.editID).arg(dbCon->lastError());
      return false;
   }

   query.clear();
   query << "get_rawData" << QString::number(data.rdataID);
   dbCon->query(query);
   if ( dbCon->lastErrno() == US_DB2::OK ){
      if (dbCon->next()){
         data.rdataFile = dbCon->value(2).toString();
         data.expID = dbCon->value(4).toInt();
      }
   }else{
      mesg = tr("Error: get_rawData: ID: %1\n%2").
             arg(data.rdataID).arg(dbCon->lastError());
      return false;
   }
   return true;
}


bool US_QueryRmsd::load_data(int index, QString &mesg) {
   if (! check_connection())
      return false;

   QTemporaryDir temp_dir;
   if (! temp_dir.isValid())
      return false;
   int state;
   QString edit_file = allData.at(index).editFile;
   QString rdata_file = allData.at(index).rdataFile;
   int editId = allData.at(index).editID;
   int rdataId = allData.at(index).rdataID;

   dbCon->readBlobFromDB(temp_dir.filePath(edit_file), "download_editData", editId);
   if (dbCon->lastErrno() != US_DB2::OK) {
      mesg = tr("Error: download_editData: ID: %1\n%2").arg(editId).arg(dbCon->lastError());
      return false;
   }

   if (rawData.contains(rdataId)) {
      US_DataIO::RawData rdata = rawData.value(rdataId);
      state = US_DataIO::writeRawData(temp_dir.filePath(rdata_file), rdata);
      if (state != US_DataIO::OK) {
         mesg = tr("Error: writeRawData: ID: %1\n%2").arg(editId).
                arg(US_DataIO::errorString(state));
         return false;
      }
   } else {
      dbCon->readBlobFromDB(temp_dir.filePath(rdata_file), "download_aucData", rdataId);
      if (dbCon->lastErrno() != US_DB2::OK) {
         mesg = tr("Error: download_aucData: ID: %1\n%2").arg(rdataId).arg(dbCon->lastError());
         return false;
      }
   }
   QVector< US_DataIO::EditedData > edata;
   QVector< US_DataIO::RawData > rdata;
   state = US_DataIO::loadData(temp_dir.path(), edit_file, edata, rdata);
   if (state != US_DataIO::OK) {
      mesg = tr("Error: US_DataIO::loadData:%1").arg(US_DataIO::errorString(state));
      return false;
   }

   editData.insert(editId, edata.at(0));
   if (! rawData.contains(rdataId)) {
      rawData.insert(rdataId, rdata.at(0));
   }
   return true;

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

   for(int i = 0; i < allData.size(); i++) {
      if (! check_combo_content(cb_edit, edit))
         continue;
      QString analysis = allData.at(i).analysis;
      if (! analysisList.contains(analysis))
         analysisList << analysis;
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

   for(int i = 0; i < allData.size(); i++) {
      QString edit = allData.at(i).edit;
      QString analysis = allData.at(i).analysis;
      if (! check_combo_content(cb_edit, edit))
         continue;
      if (! check_combo_content(cb_analysis, analysis))
         continue;
      QString method = allData.at(i).method;
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

   for (int i = 0; i < allData.size(); i++){
      QString edit = allData.at(i).edit;
      QString analysis = allData.at(i).analysis;
      QString method = allData.at(i).method;
      if (! check_combo_content(cb_edit, edit))
         continue;
      if (! check_combo_content(cb_analysis, analysis))
         continue;
      if (! check_combo_content(cb_method, method))
         continue;

      QString cell = allData.at(i).cell;
      QString channel = allData.at(i).channel;
      QString lambda = allData.at(i).lamda;
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
   tw_rmsd->setRowCount(allData.size());
   tw_rmsd->setSortingEnabled( false );
   selIndex.clear();
   int n = 0;
   double max_rmsd = 0;
   for (int i = 0; i < allData.size(); i++){
      double rmsd = allData.at(i).rmsd;
      max_rmsd = qMax(max_rmsd, rmsd);
      QString edit = allData.at(i).edit;
      QString analysis = allData.at(i).analysis;
      QString method = allData.at(i).method;
      QString cell = allData.at(i).cell;
      QString channel = allData.at(i).channel;
      QString lambda = allData.at(i).lamda;
      QString triple = tr("%1%2%3").arg(cell, channel,lambda);
      if (! check_combo_content(cb_edit, edit))
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
      selIndex << i;
      QTableWidgetItem *twi;
      twi = new QTableWidgetItem(edit);
      twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      twi->setFont(tw_font);
      tw_rmsd->setItem(n, 0, twi);

      twi = new QTableWidgetItem(analysis);
      twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      twi->setFont(tw_font);
      tw_rmsd->setItem(n, 1, twi);

      twi = new QTableWidgetItem(method);
      twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      twi->setFont(tw_font);
      tw_rmsd->setItem(n, 2, twi);

      twi = new QTableWidgetItem(triple);
      twi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      twi->setFont(tw_font);
      tw_rmsd->setItem(n, 3, twi);

      DoubleTableWidgetItem *dtwi = new DoubleTableWidgetItem(rmsd);
      dtwi->setData(Qt::UserRole, i);
      dtwi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      dtwi->setFont(tw_font);
      tw_rmsd->setItem(n, 4, dtwi);

      tw_rmsd->setRowHeight(n, rowht);
      n++;
   }
   tw_rmsd->setRowCount(n);
   tw_rmsd->verticalHeader()->setFont(tw_font);
   tw_rmsd->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   
   if (threshold == -1){
      threshold = max_rmsd;
      le_threshold->setText(QString::number(threshold));
   }
   highlight();
   
   tw_rmsd->setSortingEnabled( true );
   if ( hheader->sortIndicatorSection() > 4 ) {
      hheader->setSortIndicator( 4, Qt::DescendingOrder );
   }

   QString fname("RMSD_%1_%2_%3_%4-%5-%6.dat");
   le_file->setText(fname.arg(cb_edit->currentText(), cb_analysis->currentText(),
                              cb_method->currentText(), cb_cell->currentText(),
                              cb_channel->currentText(), cb_lambda->currentText()));

}

void US_QueryRmsd::save_data(){
   int nRows = tw_rmsd->rowCount();
   int nCol = tw_rmsd->columnCount();
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
      for (int j = 0; j < nCol; j++){
         QString header = tw_rmsd->horizontalHeaderItem(j)->text();
         outStream << header;
         if (j < nCol - 1)
            outStream << ",";
         else
            outStream << "\n";
      }
      for (int i = 0; i < nRows; i++){
         for (int j = 0; j < nCol; j++){
            outStream << tw_rmsd->item(i, j)->text();
            if (j < nCol - 1)
               outStream << ",";
            else
               outStream << "\n";
         }
      }
   }
   file.close();
}

void US_QueryRmsd::simulate(){
   int row = tw_rmsd->currentRow();
   if (row < 0) return;
   pb_simulate->setDisabled(true);
   int index = tw_rmsd->item(row, 4)->data(Qt::UserRole).toInt();
   int editId = allData.at(index).editID;
   US_Model model = Models.value(allData.at(index).modelID);
   progress->setRange(1, model.components.size());
   progress->reset();
   if (!editData.contains(editId)) {
      QString mesg;
      if (! load_data(index, mesg)){
         QMessageBox::warning(this, tr("Error!"), mesg);
         pb_simulate->setEnabled(true);
         progress->reset();
         return;
      }
   }
   US_DataIO::RawData rdata = rawData.value(allData.at(index).rdataID);
   US_DataIO::EditedData edata = editData.value(allData.at(index).editID);
   int expId = allData.at(index).expID;
   QPoint pos = this->pos();

   fematch->auto_load_simulate(rdata, edata, model, expId, pos);
   pb_simulate->setEnabled(true);
}

void US_QueryRmsd::highlight(){
   QTableWidgetItem *twi;
   DoubleTableWidgetItem *dtwi;
   QColor color;
   for (int i = 0; i < tw_rmsd->rowCount(); i++){
      dtwi = static_cast<DoubleTableWidgetItem*>(tw_rmsd->item(i, 4));
      if (threshold == -1){
         color = QColor(152,251,152);
      } else{
         if (dtwi->get_value() >= threshold)
            color = QColor(255,250,205);
         else
            color = QColor(152,251,152);
      }
      dtwi->setBackground(color);
      for (int j = 0; j < 4; j++){
         twi = tw_rmsd->item(i, j);
         twi->setBackground(color);
      }
   }
}

void US_QueryRmsd::new_threshold(){
   QString text = le_threshold->text();
   if (text.isEmpty()){
      threshold = 1e99;
      highlight();
      return;
   }
   bool ok;
   double th = text.toDouble(&ok);
   if (ok){
      threshold = th;
   }else{
      le_threshold->setText(QString::number(threshold));
      return;
   }
   highlight();
}

void US_QueryRmsd::update_progress(int val) {
   progress->setValue(val);
}

void US_QueryRmsd::closeEvent(QCloseEvent *event) {
   fematch->close();
   event->accept();
}
