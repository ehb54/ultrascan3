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

   QLabel *lb_trsh = us_label(tr("RMSD Threshold:"));
   le_threshold = us_lineedit("", 0);

   QPushButton *pb_simulate = us_pushbutton("Simulate");

   tw_rmsd = new QTableWidget();
   tw_rmsd->setRowCount(0);
   tw_rmsd->setColumnCount(5);
   tw_rmsd-> setHorizontalHeaderLabels(QStringList{"Triple_Method_Analysis", "RMSD"});
   tw_rmsd-> setHorizontalHeaderLabels(QStringList{"Edit", "Analysis", "Method", "Triple", "RMSD"});
   hheader = tw_rmsd->horizontalHeader();
   tw_rmsd->setStyleSheet("background-color: white");
   QHeaderView *header = tw_rmsd->horizontalHeader();
//   header->setSectionResizeMode(header->logicalIndexAt(0), QHeaderView::Stretch);
//   header->setSectionResizeMode(header->logicalIndexAt(2), QHeaderView::Stretch);
//   header->setSectionResizeMode(header->logicalIndexAt(3), QHeaderView::ResizeToContents);
//   header->setSectionResizeMode(header->logicalIndexAt(4), QHeaderView::Stretch);

//   header->setSectionResizeMode(0, QHeaderView::Stretch);
//   header->setSectionResizeMode(2, QHeaderView::Stretch);
//   header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
//   header->setSectionResizeMode(4, QHeaderView::Stretch);

   header->setSectionResizeMode(QHeaderView::Stretch);
//   tw_rmsd->setColumnWidth(0, 100);
//   tw_rmsd->setColumnWidth(1, 150);
//   tw_rmsd->setColumnWidth(3, 120);

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

   lyt_top->addWidget(lb_trsh,       3, 0, 1, 2);
   lyt_top->addWidget(le_threshold,  3, 2, 1, 2);
   lyt_top->addWidget(pb_simulate,   3, 4, 1, 2);

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
   lyt_main->setSpacing(3);

   this->setLayout(lyt_main);
   setMinimumSize(QSize(500,400));

   fematch = new US_FeMatch();

   connect(pb_load_runid, SIGNAL(clicked()), this, SLOT(load_runid()));
   connect(pb_save, SIGNAL(clicked()), this, SLOT(save_data()));
   connect(pb_simulate, SIGNAL(clicked()), this, SLOT(simulate()));
   connect(le_threshold, SIGNAL(editingFinished()), this, SLOT(new_threshold()));
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
   allRmsd.clear();
   allEdit.clear();
   allAnalysis.clear();
   allMethod.clear();
   allCell.clear();
   allChannel.clear();
   allLambda.clear();
   n_data = 0;
   threshold = -1;

   for (auto it = Models.begin(); it != Models.end(); ++it) {
      delete it.value();
   }
   Models.clear();
   allModelIDs.clear();

   selIndex.clear();
//   allEditDataMap.clear();
   allEditIds.clear();
//   QMapIterator<int, US_DataIO::EditedData*> ite(editData);
//   while (ite.hasNext()) {
//      ite.next();
//      delete ite.value();
//   }
   editData.clear();
//   QMapIterator<int, US_DataIO::RawData*> itr(rawData);
//   while (itr.hasNext()) {
//      itr.next();
//      delete itr.value();
//   }
//   rawData.clear();




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
      US_Model *model = new US_Model();
      int mId = modelIDs_tmp.at(i).toInt();
      int state = model->load(modelIDs_tmp.at(i), dbCon);
      if (state == US_DB2::OK){
         QStringList list1 = model->description.split(u'.');
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

         allRmsd << qSqrt(model->variance);

         allModelIDs << mId;
         Models[mId] = model;
         allEditIds << editIDs_tmp.at(i);
      } else {
         delete model;
      }

   }

   n_data = allRmsd.size();
   loadData();
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

//bool US_QueryRmsd::load_model(QString mID, US_Model *model){
//    if (! check_connection())
//        return false;
//    int  rc      = 0;
//    qDebug() << "ModelID to retrieve: -- " << mID;
//    rc   = model->load( mID , dbCon );
//    qDebug() << "LdM:  model load rc" << rc;
////    qApp->processEvents();

////    model_loaded = model;   // Save model exactly as loaded
////    model_used   = model;   // Make that the working model
////    is_dmga_mc   = ( model.monteCarlo  &&
////                  model.description.contains( "DMGA" )  &&
////                  model.description.contains( "_mcN" ) );
////    qDebug() << "post-Load mC" << model.monteCarlo << "is_dmga_mc" << is_dmga_mc
////             << "description" << model.description;

//    if ( model->components.size() == 0 )
//    {
//        QMessageBox::critical( this, tr( "Empty Model" ),
//                              tr( "Loaded model has ZERO components!" ) );
//        return false;
//    }

////    ti_noise.count = 0;
////    ri_noise.count = 0;
////    ti_noise.values.clear();
////    ri_noise.values.clear();

////    //Load noise files
////    triple_information[ "mID" ] = QString::number( mID );

////    progress_msg->setValue( 4 );
////    qApp->processEvents();

////    loadNoises( triple_information );

//    return true;
//}


bool US_QueryRmsd::loadData(){

   if (! check_connection())
      return false;

   QTemporaryDir temp_dir;
   if (! temp_dir.isValid())
      return false;
   QDir dir(temp_dir.path());
   QString efn = "sample.000.RI.1.A.280.xml";
   QString rfn = "sample.RI.1.A.280.auc";
   QFileInfo efile(dir, efn);
   QFileInfo rfile(dir, rfn);
   editData.clear();
   QVector<int> elist;
   foreach (int eId, allEditIds) {
      if (elist.contains(eId))
         continue;
      elist << eId;
      QStringList query;
      query << "get_editedData" << QString::number(eId);
      dbCon->query(query);
      int rId = -1;
      if ( dbCon->lastErrno() == US_DB2::OK ){
         if (dbCon->next()) rId = dbCon->value(0).toInt();
      }else{
         QMessageBox::warning(this, "Error", dbCon->lastError());
//         rawData[eId] = NULL;
         continue;
      }
      dbCon->readBlobFromDB(efile.absoluteFilePath(), "download_editData", eId);
      dbCon->readBlobFromDB(rfile.absoluteFilePath(), "download_aucData", rId);
//      US_DataIO::RawData* rdata;
      US_DataIO::EditedData edata;
      US_DataIO::loadData(dir.absolutePath(), efn, edata);
      editData[eId] = edata;
   }

   dir.removeRecursively();

//    int rID=0;
//    QString rfilename;
//    int eID=0;
//    QString efilename;

//    //get EditedData filename && editedDataID for current triple, then infer rawDataID
//    QStringList query;

//    query << "get_editedDataFilenamesIDs" << triple_information["filename"];
//    db->query( query );

//    qDebug() << "In loadData() Query: " << query;
//    qDebug() << "In loadData() Query: triple_information[ \"triple_name\" ]  -- " << triple_information[ "triple_name" ];

//    int latest_update_time = 1e100;

//    QString triple_name_actual = triple_information[ "triple_name" ];

//    if ( triple_name_actual.contains("Interference") )
//        triple_name_actual.replace( "Interference", "660" );

//    while ( db->next() )
//    {
//        QString  filename            = db->value( 0 ).toString();
//        int      editedDataID        = db->value( 1 ).toInt();
//        int      rawDataID           = db->value( 2 ).toInt();
//        //QString  date                = US_Util::toUTCDatetimeText( db->value( 3 ).toDateTime().toString( "yyyy/MM/dd HH:mm" ), true );
//        QDateTime date               = db->value( 3 ).toDateTime();

//        QDateTime now = QDateTime::currentDateTime();

//        if ( filename.contains( triple_name_actual ) )
//        {
//            int time_to_now = date.secsTo(now);
//            if ( time_to_now < latest_update_time )
//            {
//                latest_update_time = time_to_now;
//                //qDebug() << "Edited profile MAX, NOW, DATE, sec-to-now -- " << latest_update_time << now << date << date.secsTo(now);

//                rID       = rawDataID;
//                eID       = editedDataID;
//                efilename = filename;
//            }
//        }
//    }

//    qDebug() << "In loadData() after Query ";

//    QString edirpath  = US_Settings::resultDir() + "/" + triple_information[ "filename" ];
//    QDir edir( edirpath );
//    if (!edir.exists())
//        edir.mkpath( edirpath );

//    QString efilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + efilename;

//    qDebug() << "In loadData() efilename: " << efilename;


    // Can check here if such filename exists
    // QFileInfo check_file( efilepath );
    // if ( check_file.exists() && check_file.isFile() )
    //   qDebug() << "EditProfile file: " << efilepath << " exists";
    // else
//    db->readBlobFromDB( efilepath, "download_editData", eID );

//    qDebug() << "In loadData() after readBlobFromDB ";

//    //Now download rawData corresponding to rID:
//    QString efilename_copy = efilename;
//    QStringList efilename_copy_list = efilename_copy.split(".");

//    rfilename = triple_information[ "filename" ] + "." + efilename_copy_list[2] + "."
//                + efilename_copy_list[3] + "."
//                + efilename_copy_list[4] + "."
//                + efilename_copy_list[5] + ".auc";

//    QString rfilepath = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/" + rfilename;
//    //do we need to check for existance ?
//    db->readBlobFromDB( rfilepath, "download_aucData", rID );

//    qApp->processEvents();

//    qDebug() << "Loading eData, rawData: efilepath, rfilepath, eID, rID --- " << efilepath << rfilepath << eID << rID;

//    //Put downloaded data in memory:
//    QString uresdir = US_Settings::resultDir() + "/" + triple_information[ "filename" ] + "/";
//    US_DataIO::loadData( uresdir, efilename, editedData, rawData );

//    eID_global = eID;

//    qDebug() << "END of loadData(), eID_global: " << eID_global;

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

   for(int i = 0; i < n_data; i++) {
      if (! check_combo_content(cb_edit, edit))
         continue;
      QString analysis = allAnalysis.at(i);
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

   for(int i = 0; i < n_data; i++) {
      QString edit = allEdit.at(i);
      QString analysis = allAnalysis.at(i);
      if (! check_combo_content(cb_edit, edit))
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

   for (int i = 0; i < n_data; i++){
      QString edit = allEdit.at(i);
      QString analysis = allAnalysis.at(i);
      QString method = allMethod.at(i);
      if (! check_combo_content(cb_edit, edit))
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
   tw_rmsd->setRowCount(allRmsd.size());

   selIndex.clear();
   int n = 0;
   for (int i = 0; i < n_data; i++){
      double rmsd = allRmsd.at(i);
      QString edit = allEdit.at(i);
      QString analysis = allAnalysis.at(i);
      QString method = allMethod.at(i);
      QString cell = allCell.at(i);
      QString channel = allChannel.at(i);
      QString lambda = allLambda.at(i);
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
      dtwi->setData(Qt::EditRole, QVariant(rmsd));
      dtwi->setFlags(twi->flags() & ~Qt::ItemIsEditable);
      dtwi->setFont(tw_font);
      tw_rmsd->setItem(n, 4, dtwi);

      tw_rmsd->setRowHeight(n, rowht);
      n++;
   }
   tw_rmsd->setRowCount(n);
   tw_rmsd->verticalHeader()->setFont(tw_font);
   tw_rmsd->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
   tw_rmsd->setSortingEnabled( true );
   tw_rmsd->sortItems(4, Qt::DescendingOrder);
   if (threshold == -1){
      DoubleTableWidgetItem *item = static_cast<DoubleTableWidgetItem*>(tw_rmsd->item(0, 4));
      threshold = item->get_value();
      le_threshold->setText(QString::number(threshold));
   }
   highlight();

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
   DbgLv(0) << tw_rmsd->currentColumn();
   DbgLv(0) << tw_rmsd->currentRow();
   fematch->show();

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
