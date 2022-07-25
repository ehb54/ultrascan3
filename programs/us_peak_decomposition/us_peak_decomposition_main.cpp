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

    ckb_xrange = new QCheckBox();
    QGridLayout *us_xrange = us_checkbox("Specify a Range",
                                                ckb_xrange);
    pb_pick_rp = us_pushbutton("Pick Two Points", false);
    QHBoxLayout *xrange_lyt = new QHBoxLayout();
    xrange_lyt->addLayout(us_xrange);
    xrange_lyt->addWidget(pb_pick_rp);


    ckb_rawData = new QCheckBox();
    QGridLayout *rawData_lyt = us_checkbox("Raw Data", ckb_rawData);
    ckb_rawData->setChecked(true);

    ckb_integral = new QCheckBox();
    QGridLayout *integral_lyt = us_checkbox("Integral", ckb_integral);
    ckb_integral->setChecked(true);

    ckb_scale = new QCheckBox();
    QGridLayout *scale_lyt = us_checkbox("Scale to 100%", ckb_scale);
    ckb_scale->setChecked(true);



    QHBoxLayout *intg_lyt = new QHBoxLayout();
    intg_lyt->addStretch(1);
    intg_lyt->addLayout(rawData_lyt);
    intg_lyt->addLayout(integral_lyt);
    intg_lyt->addLayout(scale_lyt);
    intg_lyt->addStretch(1);

    usplot = new US_Plot( plot, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot->setMinimumSize( 700, 400 );
    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft  , true );
    plot->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(plot);

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    QVBoxLayout* left_lyt = new QVBoxLayout();
    QVBoxLayout* right_lyt = new QVBoxLayout();

    left_lyt->addLayout(load_lyt);
    left_lyt->addWidget(lb_inpList);
    left_lyt->addWidget(lw_inpData);
    left_lyt->addWidget(lb_selList);
    left_lyt->addWidget(lw_selData);
    left_lyt->addLayout(sel_lyt);
    left_lyt->addLayout(xrange_lyt);
    left_lyt->addStretch(1);

    right_lyt->addLayout(intg_lyt);
    right_lyt->addLayout(usplot);

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
    set_sel_data();
}

void US_PeakDecomposition::slt_rmItem(void){
    int row = lw_selData->currentRow();
    if (row < 0)
        return;
    int rowInp = filenames.indexOf(lw_selData->item(row)->text());
    lw_inpData->item(rowInp)->setForeground(Qt::black);
    selFilenames.removeAt(row);
    lw_selData->takeItem(row);
    set_sel_data();
}

void US_PeakDecomposition::slt_clsList(void){
    for (int i = 0; i < lw_selData->count(); i++){
        int rowInp = filenames.indexOf(lw_selData->item(i)->text());
        lw_inpData->item(rowInp)->setForeground(Qt::black);
        selFilenames.removeAt(i);
    }
    lw_selData->clear();
    set_sel_data();
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
        QVector<QVector<double>> intgs;
        QVector<QVector<double>> intgs_s;
        int ns = rawData.scanCount();
        for (int j = 0; j < ns; j++){
            yvals << rawData.scanData.at(j).rvalues;
            QVector<QVector<double>> cumsum = trapz(rawData.xvalues,
                                                     rawData.scanData.at(j).rvalues);
            intgs << cumsum.at(0);
            intgs_s << cumsum.at(1);
        }
        yvalues << yvals;
        integral << intgs;
        integral_s << intgs_s;
        QVector<int> scrng;
        scrng << ns - 1 << ns << 1 << ns;
        scanRange << scrng;

    }
    if (badFiles.size() != 0){
        QMessageBox::warning(this, "Error!",
                             "These files could not be loaded!\n" +
                             badFiles.join("\n"));
    }

//    emit sig_plot1(true);
//    pb_load_data->setEnabled(false);
    pb_reset_data->setEnabled(true);
//    pb_reset->setEnabled(true);
}

QVector<QVector<double>> US_PeakDecomposition::trapz(QVector<double> xval,
                                                     QVector<double> yval){
    QVector<QVector<double>> out;
    QVector<double> intg;
    QVector<double> intg_s;

    double dx;
    double sum = 0;
    double *x = xval.data();
    double *y = yval.data();
    for (int i = 1; i < xval.size(); i++){
        dx = x[i] - x[i - 1];
        sum += dx * ( y[i] + y[i - 1] ) * 0.5;
        intg << sum;
    }

    for (int i = 0; i < intg.size(); i++){
        intg_s << intg.at(i) * 100 / sum;
    }

    out << intg << intg_s;
    return out;

}


void US_PeakDecomposition::set_sel_data(void){
    xvalues_sel.clear();
    yvalues_sel.clear();
    integral_sel.clear();
    integral_s_sel.clear();
    midxval_sel.clear();
    QVector<int> inpIds;
    for (int i = 0; i < lw_selData->count(); i++){
        inpIds << filenames.indexOf(lw_selData->item(i)->text());
    }
    for (int i = 0; i < inpIds.size(); i++){
        int id = inpIds.at(i);
        xvalues_sel << xvalues.at(id);
        yvalues_sel << yvalues.at(id);
        integral_sel << integral.at(id);
        integral_s_sel << integral_s.at(id);
        QVector<double> midx;
        const double *xp = xvalues.at(id).data();
        for (int j = 1; j < xvalues.at(id).size(); j++){
            midx << 0.5 * (xp[j] + xp[j - 1]);
        }
        midxval_sel << midx;
    }
    slt_plot(true);
}

void US_PeakDecomposition::slt_plot(bool state){
    plot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
//        plot1->setTitle(tr(""));
        grid = us_grid(plot);
        plot->replot();
        return;
    }
    QwtText yTitle = plot->axisTitle(QwtPlot::yLeft);
    QString plt_state;
    if (ckb_rawData->isChecked())
        plt_state.append("D");
    if (ckb_integral->isChecked()){
        if (ckb_scale->isChecked())
            plt_state.append("S");
        else
            plt_state.append("U");
    }

    if (plt_state.size() == 0){
        yTitle.setText("");
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        plot->setAxisTitle(QwtPlot::yRight, yTitle);
        return;
    }

    QPen pen(Qt::yellow);

    int nd = selFilenames.size();
    const double *xp, *yp;

    if (plt_state.contains("D")){
        yTitle.setText("Absorbance");
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        for (int i = 0; i < nd; i++){
            int ns = yvalues_sel.at(i).size();
            int np = xvalues_sel.at(i).size();
            xp = xvalues_sel.at(i).data();
            if (cb_scan->currentText() == "All"){
                for (int j = 0; j < ns; j++){
                    yp = yvalues_sel.at(i).at(j).data();
                    QwtPlotCurve* curve = us_curve(plot,"");
                    curve->setPen(pen);
                    curve->setSamples(xp, yp, np);
                }
            } else {
                if (cb_scan->currentText() == "First")
                    yp = yvalues_sel.at(i).at(0).data();
                else //  "Last"
                    yp = yvalues_sel.at(i).at(ns - 1).data();
                QwtPlotCurve* curve = us_curve(plot,"");
                curve->setPen(pen);
                curve->setSamples(xp, yp, np);
            }

        }
    }

    if (plt_state.contains("U") || plt_state.contains("S")){
        for (int i = 0; i < nd; i++){
            int ns = integral_sel.at(i).size();
            int np = midxval_sel.at(i).size();
            xp = midxval_sel.at(i).data();
            if (cb_scan->currentText() == "All"){
                for (int j = 0; j < ns; j++){
                    if (plt_state.contains("S"))
                        yp = integral_s_sel.at(i).at(j).data();
                    else // (plt_state.contains("U")
                        yp = integral_sel.at(i).at(j).data();
                    QwtPlotCurve* curve = us_curve(plot,"");
                    if (plt_state.size() == 2)
                        curve->setYAxis(QwtPlot::yRight);
                    curve->setPen(pen);
                    curve->setSamples(xp, yp, np);
                }
            } else {
                if (cb_scan->currentText() == "First"){
                    if (plt_state.contains("S"))
                        yp = integral_s_sel.at(i).at(0).data();
                    else // (plt_state.contains("U")
                        yp = integral_sel.at(i).at(0).data();
                } else // "Last"
                    if (plt_state.contains("S"))
                        yp = integral_s_sel.at(i).at(ns - 1).data();
                    else // (plt_state.contains("U")
                        yp = integral_sel.at(i).at(ns - 1).data();
                QwtPlotCurve* curve = us_curve(plot,"");
                if (plt_state.size() == 2)
                    curve->setYAxis(QwtPlot::yRight);
                curve->setPen(pen);
                curve->setSamples(xp, yp, np);
            }
        }

        if (plt_state.contains("S"))
            yTitle.setText("Integral (%)");
        else
            yTitle.setText("Integral");
        if (plt_state.size() == 2)
            plot->setAxisTitle(QwtPlot::yRight, yTitle);
        else
            plot->setAxisTitle(QwtPlot::yLeft, yTitle);
    }



//    plot->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
//    plot->setAxisScale( QwtPlot::yLeft  , min_y - dr, max_y + dr);
//    plot->updateAxes();
    plot->replot();
}

void US_PeakDecomposition::slt_scan(double id){
    scanid = (int) id;
    emit sig_plot1(true);
}

