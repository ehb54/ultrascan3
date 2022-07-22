//! \file us_peak_decomposition_main.cpp
//
#include <QApplication>
#include "us_license_t.h"
#include "us_license.h"
#include "us_peak_decomposition.h"

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_PeakDecomposition w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}

US_PeakDecomposition::US_PeakDecomposition(): US_Widgets()
{
    setWindowTitle("Peak Decomposition & Analysis");
    QPalette p = US_GuiSettings::frameColorDefault();
    setPalette( p );

//    QLabel* lb_load = us_banner("Load Data");
    pb_load_data = us_pushbutton("Load Data");
    pb_reset_data = us_pushbutton("Reset Data");
    pb_reset_plot = us_pushbutton("Reset Plot");
    pb_save = us_pushbutton("Save", false);
    pb_help = us_pushbutton("Help");
    pb_close = us_pushbutton("Close");

    QGridLayout* load_lyt = new QGridLayout();
    load_lyt->addWidget(pb_load_data,   0, 0, 1, 1);
    load_lyt->addWidget(pb_reset_data,  0, 1, 1, 1);
    load_lyt->addWidget(pb_reset_plot,  1, 0, 1, 1);
    load_lyt->addWidget(pb_save,        1, 1, 1, 1);
    load_lyt->addWidget(pb_help,        2, 0, 1, 1);
    load_lyt->addWidget(pb_close,       2, 1, 1, 1);

//    lb_descr = us_label("", -1);
//    lb_descr->setWordWrap(true);
//    lb_descr->setAlignment(Qt::AlignTop);
//    lb_descr->setMinimumHeight(50);
//    lb_descr->setStyleSheet(tr("border: 1px solid black;"
//                               "border-radius: 5px;"
//                               "padding: 2px;"
//                               "color: black;"
//                               "background-color: white;"));

    QLabel *lb_inpList = us_banner("List of File(s)");
    lw_inpData = us_listwidget();

//    "QListWidget QScrollBar{"
//    "background : lightblue;}"

    QLabel *lb_selList = us_banner("Selected File(s)");
    lw_selData = us_listwidget();

    QStringList cbList;
    cbList << "All" << "First" << "Last" ;
    QLabel *lb_scan = us_label("Scan(s):");
    lb_scan->setAlignment(Qt::AlignRight);
    cb_scan = us_comboBox();
    cb_scan->addItems(cbList);

    pb_rmItem = us_pushbutton("Remove Item");
    pb_clsList = us_pushbutton("Clean List");

    QHBoxLayout *sel_lyt = new QHBoxLayout();
    sel_lyt->addWidget(lb_scan);
    sel_lyt->addWidget(cb_scan);
    sel_lyt->addWidget(pb_rmItem);
    sel_lyt->addWidget(pb_clsList);



    usplot1 = new US_Plot( plot1, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot1->setMinimumSize( 700, 400 );
    plot1->enableAxis( QwtPlot::xBottom, true );
    plot1->enableAxis( QwtPlot::yLeft  , true );
    plot1->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(plot1);

    usplot2 = new US_Plot( plot2, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot2->setMinimumSize( 700, 400 );
    plot2->enableAxis( QwtPlot::xBottom, true );
    plot2->enableAxis( QwtPlot::yLeft  , true );
    plot2->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(plot2);

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    QVBoxLayout* left_lyt = new QVBoxLayout();
    QVBoxLayout* right_lyt = new QVBoxLayout();

    left_lyt->addLayout(load_lyt);
    left_lyt->addWidget(lb_inpList);
    left_lyt->addWidget(lw_inpData);
    left_lyt->addWidget(lb_selList);
    left_lyt->addWidget(lw_selData);
    left_lyt->addLayout(sel_lyt);
    left_lyt->addStretch(1);

    right_lyt->addLayout(usplot1);
    right_lyt->addLayout(usplot2);

    left_lyt->setMargin(1);
    left_lyt->setSpacing(1);
    right_lyt->setMargin(1);
    right_lyt->setSpacing(1);

    main_lyt->addLayout(left_lyt);
    main_lyt->addLayout(right_lyt);
    main_lyt->setMargin(1);
    main_lyt->setSpacing(0);

    this->setLayout(main_lyt);

    connect(pb_load_data, SIGNAL(clicked()), this, SLOT(slt_load_auc()));
    connect(this, SIGNAL(sig_plot1(bool)), this, SLOT(slt_plot1(bool)));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(lw_inpData, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slt_addRmItem(QListWidgetItem *)));
    connect(pb_rmItem, SIGNAL(clicked()), this, SLOT(slt_rmItem()));
    connect(pb_clsList, SIGNAL(clicked()), this, SLOT(slt_clsList()));
}

void US_PeakDecomposition::slt_addRmItem(QListWidgetItem *item){
    QString text = item->text();
    int id = selFilenames.indexOf(text);
    if (id == -1){
        lw_selData->addItem(text);
        selFilenames.append(text);
        item->setForeground(Qt::red);
    } else {
        QListWidgetItem *selItem = lw_selData->item(id);
        delete selItem;
        selFilenames.removeAt(id);
        item->setForeground(Qt::black);
    }
}

void US_PeakDecomposition::slt_rmItem(void){
    int row = lw_selData->currentRow();
    if (row < 0)
        return;
    int rowInp = filenames.indexOf(lw_selData->item(row)->text());
    lw_inpData->item(rowInp)->setForeground(Qt::black);
    selFilenames.removeAt(row);
    lw_selData->takeItem(row);
}

void US_PeakDecomposition::slt_clsList(void){
    for (int i = 0; i < lw_selData->count(); i++){
        int rowInp = filenames.indexOf(lw_selData->item(i)->text());
        lw_inpData->item(rowInp)->setForeground(Qt::black);
        selFilenames.removeAt(i);
    }
    lw_selData->clear();
}

//void US_PeakDecomposition::slt_selItem(int row){
//    QListWidgetItem *item = lw_inpData->item(row);
//    QColor fontC = item->foreground().color();
//    lw_inpData->setStyleSheet(tr("QListView::item:selected{"
//                              "border : 1px solid black;"
//                              "background : gray;"
//                              "color : %1;}").arg(fontC.name()));
//}

void US_PeakDecomposition::slt_load_auc(){

    QStringList fPath = QFileDialog::getOpenFileNames(this, tr("Open AUC File"),
                                                    US_Settings::importDir(),
                                                    tr(".auc (*.auc)"));
    if (fPath.size() == 0)
        return;

    QStringList badFiles;
    for (int i = 0; i < fPath.size(); i++){
        if (filePaths.contains(fPath.at(i)))
            continue;
        US_DataIO::RawData rawData;
        int state = US_DataIO::readRawData(fPath.at(i), rawData);
        QFileInfo finfo = QFileInfo(fPath.at(i));
        if (state != US_DataIO::OK){
            badFiles << finfo.fileName();
            continue;
        }
        xvalues << rawData.xvalues;
        filenames << finfo.fileName();
        filePaths << fPath.at(i);
        lw_inpData->addItem(finfo.fileName());
        QVector<QVector<double>> yvals;
        int ns = rawData.scanCount();
        for (int j = 0; j < ns; j++){
            yvals << rawData.scanData.at(j).rvalues;
        }
        yvalues << yvals;
        QVector<int> scrng;
        scrng << ns - 1 << ns << 1 << ns;
        scanRange << scrng;

    }
    if (badFiles.size() != 0){
        QMessageBox::warning(this, "Error!",
                             "These files could not be loaded!\n" +
                             badFiles.join("\n"));
    }

    emit sig_plot1(true);
//    pb_load_data->setEnabled(false);
    pb_reset_data->setEnabled(true);
//    pb_reset->setEnabled(true);
}

void US_PeakDecomposition::slt_plot1(bool state){
//    plot1->detachItems(QwtPlotItem::Rtti_PlotItem, false);
//    if (! state){
////        plot1->setTitle(tr(""));
//        grid = us_grid(plot1);
//        plot1->replot();
//        return;
//    }
//    QPen pen(Qt::yellow);

//    const double *xp, *yp;
//    xp = xvalues.at(scanid).data();
//    yp = yvalues.at(scanid).data();
////    QVector<double> xvec;
////    QVector<double> yvec;
//    int id_i = xlimits.at(scanid).at(0);
//    int id_f = xlimits.at(scanid).at(1);
//    double min_x  = xp[id_i];
//    double max_x  = xp[id_f - 1];
//    double min_y  =  1e20;
//    double max_y  = -1e20;
//    for (int i = id_i ; i < id_f; ++i){
//        min_y = qMin(min_y, yp[i]);
//        max_y = qMax(max_y, yp[i]);
//    }
//    QwtPlotCurve* curve = us_curve(plot1,"");
//    curve->setPen(pen);
//    curve->setSamples(xp + id_i, yp + id_i, id_f - id_i);

//    grid = us_grid(plot1);
//    double dx = (max_x - min_x) * 0.05;
//    double dr = (max_y - min_y) * 0.05;
//    plot1->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
//    plot1->setAxisScale( QwtPlot::yLeft  , min_y - dr, max_y + dr);
//    plot1->updateAxes();
//    plot1->replot();
}

void US_PeakDecomposition::slt_scan(double id){
    scanid = (int) id;
    emit sig_plot1(true);
}

