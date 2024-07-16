#include <QApplication>
#include "us_license_t.h"
#include "us_license.h"
#include "us_norm_profile.h"
#include "us_load_auc.h"
#include <QFileInfo>

US_Norm_Profile::US_Norm_Profile(): US_Widgets()
{
    setWindowTitle("Buoyancy Equilibrium Data Analysis");
    QPalette p = US_GuiSettings::frameColorDefault();
    setPalette( p );

    QLabel* lb_runinfo = us_label("Data Info");
    le_runinfo = us_lineedit("", 0, true);
    QHBoxLayout *runinfo_lyt = new QHBoxLayout();
    runinfo_lyt->addWidget(lb_runinfo);
    runinfo_lyt->addWidget(le_runinfo);

    QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
//    connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
//    specs->addWidget( pb_investigator, s_row, 0 );

    if ( US_Settings::us_inv_level() < 1 )
       pb_investigator->setEnabled( false );

    int id = US_Settings::us_inv_ID();
    QString number  = ( id > 0 ) ?
                         QString::number( US_Settings::us_inv_ID() ) + ": "
                              : "";
    le_investigator = us_lineedit( number + US_Settings::us_inv_name(), -1, true );
    le_investigator->setMinimumWidth(150);
//    specs->addWidget( le_investigator, s_row++, 1, 1, 3 );
    QHBoxLayout *inv_lyt = new QHBoxLayout();
    inv_lyt->addWidget(pb_investigator);
    inv_lyt->addWidget(le_investigator);

    disk_controls = new US_Disk_DB_Controls;

    pb_load = us_pushbutton("Load Data");
    pb_reset = us_pushbutton("Reset Data");
    pb_save = us_pushbutton("Save");
    pb_close = us_pushbutton("Close");

    QGridLayout* load_lyt = new QGridLayout();
    load_lyt->addWidget(pb_load,   0, 0, 1, 1);
    load_lyt->addWidget(pb_reset,  0, 1, 1, 1);
    load_lyt->addWidget(pb_close,       1, 0, 1, 1);
    load_lyt->addWidget(pb_save,        1, 1, 1, 1);

    QLabel *lb_inpList = us_banner("List of File(s)");
    lw_inpData = us_listwidget();

    QLabel *lb_selList = us_banner("Selected File(s)");
    lw_selData = us_listwidget();

    pb_rmItem = us_pushbutton("Remove Item");
    pb_cleanList = us_pushbutton("Clean List");

    ckb_xrange = new QCheckBox();
    QGridLayout *us_xrange = us_checkbox("Limit Radius",
                                                ckb_xrange);
    pb_pick_rp = us_pushbutton("Pick Two Points", false);

    ckb_norm_max = new QCheckBox();
    QGridLayout *us_norm_max = us_checkbox("Normalize by Maximum",
                                         ckb_norm_max);
    pb_pick_norm = us_pushbutton("Pick a Point");

    QGridLayout *bottom_lyt = new QGridLayout();
    bottom_lyt->addWidget(pb_rmItem,         0, 0, 1, 1);
    bottom_lyt->addWidget(pb_cleanList,      0, 1, 1, 1);
    bottom_lyt->addLayout(us_xrange,         1, 0, 1, 1);
    bottom_lyt->addWidget(pb_pick_rp,        1, 1, 1, 1);
    bottom_lyt->addLayout(us_norm_max,       2, 0, 1, 1);
    bottom_lyt->addWidget(pb_pick_norm,      2, 1, 1, 1);

    ckb_rawData = new QCheckBox();
    QGridLayout *rawData_lyt = us_checkbox("Raw Data", ckb_rawData);
    ckb_rawData->setChecked(true);

    ckb_integral = new QCheckBox();
    QGridLayout *integral_lyt = us_checkbox("Integral", ckb_integral);
    ckb_integral->setChecked(true);

    ckb_norm = new QCheckBox();
    QGridLayout *norm_lyt = us_checkbox("Normalized", ckb_norm);
    ckb_norm->setChecked(true);

    ckb_legend = new QCheckBox();
    QGridLayout *legend_lyt = us_checkbox("Legend", ckb_legend);
    ckb_legend->setChecked(true);

    QHBoxLayout *intg_lyt = new QHBoxLayout();
    intg_lyt->addStretch(1);
    intg_lyt->addLayout(rawData_lyt);
    intg_lyt->addLayout(integral_lyt);
    intg_lyt->addLayout(norm_lyt);
    intg_lyt->addLayout(legend_lyt);
    intg_lyt->addStretch(1);

    usplot = new US_Plot( plot, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot->setMinimumSize( 700, 400 );
    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft  , true );
    plot->setCanvasBackground(QBrush(Qt::white));

    QVBoxLayout* main_lyt = new QVBoxLayout();
    QHBoxLayout* body_lyt = new QHBoxLayout();
    QVBoxLayout* left_lyt = new QVBoxLayout();
    QVBoxLayout* right_lyt = new QVBoxLayout();

    left_lyt->addLayout(inv_lyt);
    left_lyt->addLayout(disk_controls);
    left_lyt->addLayout(load_lyt);
    left_lyt->addWidget(lb_inpList);
    left_lyt->addWidget(lw_inpData);
    left_lyt->addWidget(lb_selList);
    left_lyt->addWidget(lw_selData);
    left_lyt->addLayout(bottom_lyt);
    left_lyt->addStretch(1);

    right_lyt->addLayout(intg_lyt);
    right_lyt->addLayout(usplot);

    left_lyt->setMargin(1);
    left_lyt->setSpacing(1);
    right_lyt->setMargin(1);
    right_lyt->setSpacing(1);

    body_lyt->addLayout(left_lyt, 1);
    body_lyt->addLayout(right_lyt, 3);
    body_lyt->setMargin(1);
    body_lyt->setSpacing(0);

    main_lyt->addLayout(runinfo_lyt);
    main_lyt->addLayout(body_lyt);
    main_lyt->setMargin(0);
    main_lyt->setSpacing(0);
    this->setLayout(main_lyt);

    picker = new US_PlotPicker(plot);
    picker->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );
    picker->setRubberBandPen(QPen(Qt::red));
    picker->setTrackerPen(QPen(Qt::red));
    plotData();
    picker_state = XNONE;

    connect(pb_load, SIGNAL(clicked()), this, SLOT(slt_loadAUC()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));
    connect(lw_inpData, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slt_addRmItem(QListWidgetItem *)));
    connect(lw_inpData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_inItemSel(int )));
    connect(lw_selData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_outItemSel(int )));
    connect(pb_rmItem, SIGNAL(clicked()), this, SLOT(slt_rmItem()));
    connect(pb_cleanList, SIGNAL(clicked()), this, SLOT(slt_cleanList()));

    connect(pb_pick_rp, SIGNAL(clicked()), this, SLOT(slt_pickRange()));
    connect(pb_pick_norm, SIGNAL(clicked()), this, SLOT(slt_pickPoint()));
    connect(ckb_xrange, SIGNAL(stateChanged(int)), this, SLOT(slt_xrange(int)));
    connect(ckb_legend, SIGNAL(stateChanged(int)), this, SLOT(slt_legend(int)));
    connect(ckb_integral, SIGNAL(stateChanged(int)), this, SLOT(slt_integral(int)));
    connect(ckb_norm, SIGNAL(stateChanged(int)), this, SLOT(slt_norm(int)));
    connect(ckb_rawData, SIGNAL(stateChanged(int)), this, SLOT(slt_rawData(int)));
    connect(ckb_norm_max, SIGNAL(stateChanged(int)), this, SLOT(slt_norm_by_max(int)));
    connect(picker, SIGNAL(cMouseUp(const QwtDoublePoint&)),
            this,   SLOT(slt_mouse(const QwtDoublePoint&)));
    ckb_norm_max->setCheckState(Qt::Checked);
}

void US_Norm_Profile::slt_xrange(int state){
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (state == Qt::Checked){
        pb_pick_rp->setEnabled(true);
        if (x_min_picked == -1 && x_max_picked == -1) {
            pb_pick_rp->setStyleSheet(qs.arg("yellow"));
        } else {
            pb_pick_rp->setStyleSheet(qs.arg(color.name()));
        }
    }else{
        pb_pick_rp->setDisabled(true);
        pb_pick_rp->setStyleSheet(qs.arg(color.name()));
        enableWidgets(true);
    }
    selectData();
    return;
}

void US_Norm_Profile::slt_addRmItem(QListWidgetItem *item){
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
    selectData();
}

void US_Norm_Profile::slt_rmItem(void){
    int row = lw_selData->currentRow();
    if (row < 0)
        return;
    int rowInp = filenames.indexOf(lw_selData->item(row)->text());
    lw_inpData->item(rowInp)->setForeground(Qt::black);
    selFilenames.removeAt(row);
    lw_selData->takeItem(row);
    selectData();
    if (lw_selData->count() == 0) {
        picker_state = XNONE;
    }
}

void US_Norm_Profile::slt_cleanList(void){
    picker_state = XNONE;
    for (int i = 0; i < lw_selData->count(); i++){
        int rowInp = filenames.indexOf(lw_selData->item(i)->text());
        lw_inpData->item(rowInp)->setForeground(Qt::black);
    }
    selFilenames.clear();
    lw_selData->clear();
    selectData();
}

void US_Norm_Profile::slt_loadAUC(){

   bool isLocal = ! disk_controls->db();
   if (! isLocal){
      QVector< US_DataIO::RawData > allData;
      QStringList triples;
      QString workingDir;
      US_LoadAUC* dialog = new US_LoadAUC( isLocal, allData, triples, workingDir );

      if ( dialog->exec() == QDialog::Rejected )  return;

      QFileInfo finfo(workingDir);
      QString runid = finfo.baseName();
      QString dirname = finfo.dir().absolutePath();

      for (int i = 0; i < triples.size(); i++){
         QStringList ccw = triples.at(i).split(u'/');
         US_DataIO::RawData rawData = allData.at(i);
         QString fn = tr("%1.%2.%3.%4").arg(runid, ccw.at(0).trimmed(),
                                            ccw.at(1).trimmed(), ccw.at(2).trimmed());
         QString fp = tr("%1.%2.auc").arg(dirname, fn);
         filenames << fn;
         filePaths << fp;
         lw_inpData->addItem(fn);
         xvalues << rawData.xvalues;
         yvalues << rawData.scanData.last().rvalues;
      }
   }else {
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
         QString fname = finfo.fileName();
         fname.chop(4);
         filenames << fname;
         filePaths << fPath.at(i);
         lw_inpData->addItem(fname);
         yvalues << rawData.scanData.last().rvalues;
      }
      if (badFiles.size() != 0){
      QMessageBox::warning(this, "Error!",
                              "These files could not be loaded!\n" +
                                  badFiles.join("\n"));
      }
   }
}

QMap<QString, QVector<double>> US_Norm_Profile::trapz(
                           QVector<double> xval, QVector<double> yval){
    QMap<QString, QVector<double>> out;
    QVector<double> yvalN;
    QVector<double> midxval;
    QVector<double> integral;
    QVector<double> integralN;

    const double *x = xval.data();
    const double *y = yval.data();
    int np = xval.size();
    double dx;
    double sum = 0;
    double maxY = -1e99;
    bool flag_pnorm = false;
    if (! ckb_norm_max->isChecked() && x_norm > 0) {
        flag_pnorm = true;
    }

    for (int i = 1; i < np; i++){
        dx = x[i] - x[i - 1];
        sum += dx * ( y[i] + y[i - 1] ) * 0.5;
        integral << sum;
        midxval << 0.5 * (x[i] + x[i - 1]);
        maxY = qMax(maxY, y[i]);
    }

    double normY = 1;
    if (flag_pnorm && x_norm <= x[0]) {
        normY = y[0];
    } else if (flag_pnorm && x_norm >= x[np - 1]) {
        normY = y[np - 1];
    } else if (flag_pnorm && x_norm > x[0] && x_norm < x[np - 1]) {
        for (int i = 1; i < np - 1; i++) {
            if (x[i] >= x_norm) {
                normY = y[i];
                break;
            }
        }
    } else {
        normY = maxY;
    }

    for (int i = 0; i < integral.size(); i++){
        integralN << integral.at(i) * 100 / sum;
        yvalN << yval.at(i) / normY;
    }
    yvalN << yval.last() / normY;
    out["midxval"] = midxval;
    out["yvaluesN"] = yvalN;
    out["integral"] = integral;
    out["integralN"] = integralN;
    return out;

}

QPair<int, int> US_Norm_Profile::getXlimit(QVector<double> xval_in,
                                           double xmin, double xmax){
    const double *xp = xval_in.data();
    int id1 = -1;
    int id2 = -1;
    int np = xval_in.size();
    for (int i = 0; i < np; i++){
        if (id1 == -1){
            if (xp[i] >= xmin){
                id1 = i;
            }
        } else {
            if (xp[i] >= xmax){
                id2 = i + 1;
                break;
            }
        }
    }
    if (id2 == -1){
        id2 = np;
    }
    QPair<int, int> pair;
    pair.first = id1;
    pair.second = id2 - id1;
    return pair;
}

void US_Norm_Profile::selectData(void){
    xvalues_sel.clear();
    yvalues_sel.clear();
    yvaluesN_sel.clear();
    integral_sel.clear();
    integralN_sel.clear();
    midxval_sel.clear();
    bool flag_limit = false;
    if (ckb_xrange->isChecked() && x_min_picked != -1 && x_max_picked != -1) {
        flag_limit = true;
    }
    QVector<int> inpIds;
    for (int i = 0; i < lw_selData->count(); i++){
        inpIds << filenames.indexOf(lw_selData->item(i)->text());
    }
    for (int i = 0; i < inpIds.size(); i++){
        int id = inpIds.at(i);
        if (flag_limit){
            QPair<int, int> pair= getXlimit(xvalues.at(id), x_min_picked, x_max_picked);
            xvalues_sel << xvalues.at(id).mid(pair.first, pair.second);
            yvalues_sel << yvalues.at(id).mid(pair.first, pair.second);
        } else {
            xvalues_sel << xvalues.at(id);
            yvalues_sel << yvalues.at(id);
        }

        QMap<QString, QVector<double>> trapzOut;
        trapzOut = trapz(xvalues_sel.last(), yvalues_sel.last());
        midxval_sel << trapzOut["midxval"];
        yvaluesN_sel << trapzOut["yvaluesN"];
        integral_sel << trapzOut["integral"];
        integralN_sel << trapzOut["integralN"];
    }
    plotData();
}

void US_Norm_Profile::plotData(void){
    plot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    plot->enableAxis(QwtPlot::yRight, false);
    plot->enableAxis(QwtPlot::yLeft, false);
    grid = us_grid(plot);
    QPen pen_mj = grid->majorPen();
    QPen pen_mn = grid->majorPen();
    pen_mj.setColor(Qt::black);
    pen_mn.setColor(Qt::black);
    grid->setMajorPen(pen_mj);
    grid->setMinorPen(pen_mn);

    int tpncy = 255;
    QMap<QString, QColor> color;
    color["blue"]    = QColor(0  , 0  , 255, tpncy);
    color["orange"]  = QColor(255, 128, 0  , tpncy);
    color["green"]   = QColor(0  , 255, 0  , tpncy);
    color["cyan"]    = QColor(0  , 255, 255, tpncy);
    color["red"]     = QColor(255, 0  , 0  , tpncy);
    color["purple"]  = QColor(153, 51 , 255, tpncy);
    color["pink"]    = QColor(255, 0  , 255, tpncy);
    color["yellow"]  = QColor(200, 200, 0 , tpncy);
    color["black"]   = QColor(0  , 0  , 0  , 255);
    QVector<QColor> color_list;
    color_list <<  color["blue"] << color["orange"] << color["green"];
    color_list << color["cyan"] << color["red"] << color["purple"] ;
    color_list << color["pink"] << color["yellow"] << color["black"];
    int sz_clist = color_list.size();
    QPen pen = QPen(Qt::SolidPattern, 1, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
    pen.setWidth(2);

    QwtText yTitle = plot->axisTitle(QwtPlot::yLeft);

    int nd = selFilenames.size();
    bool plt_state = ckb_rawData->isChecked() || ckb_integral->isChecked();

    if (! plt_state || nd == 0){
        yTitle.setText("");
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        plot->setAxisTitle(QwtPlot::yRight, yTitle);
        plot->replot();
        return;
    }

    const double *xp, *yp;

    double minX =  1e99;
    double maxX = -1e99;
    double minY =  1e99;
    double maxY = -1e99;

    if (ckb_rawData->isChecked()){
        bool norm = ckb_norm->isChecked();
        if (norm)
            yTitle.setText("Absorbance (normalized)");
        else
            yTitle.setText("Absorbance");
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        plot->enableAxis(QwtPlot::yLeft, true);
        for (int i = 0; i < nd; i++){
            int np = xvalues_sel.at(i).size();
            xp = xvalues_sel.at(i).data();
            pen.setColor(color_list.at(i % sz_clist));
            QString legend = tr("(D)_") + selFilenames.at(i);
            if (norm)
                yp = yvaluesN_sel.at(i).data();
            else
                yp = yvalues_sel.at(i).data();
            QwtPlotCurve* curve = us_curve(plot, legend);
            curve->setPen(pen);
            curve->setSamples(xp, yp, np);

            for (int j = 0; j < np; j++){
                minX = qMin(minX, xp[j]);
                maxX = qMax(maxX, xp[j]);
                minY = qMin(minY, yp[j]);
                maxY = qMax(maxY, yp[j]);
            }
        }
        double dy = (maxY - minY) * 0.05;
        plot->setAxisScale( QwtPlot::yLeft, minY - dy, maxY + dy);
    }

    minY =  1e99;
    maxY = -1e99;

    if (ckb_integral->isChecked()){

        bool norm = ckb_norm->isChecked();
        if (norm)
            yTitle.setText("Integral (%)");
        else
            yTitle.setText("Integral");

        if (ckb_rawData->isChecked()){
            plot->setAxisTitle(QwtPlot::yRight, yTitle);
            plot->enableAxis(QwtPlot::yRight, true);
        }
        else {
            plot->setAxisTitle(QwtPlot::yLeft, yTitle);
            plot->enableAxis(QwtPlot::yLeft, true);
        }

        for (int i = 0; i < nd; i++){
            int np = midxval_sel.at(i).size();
            xp = midxval_sel.at(i).data();
            pen.setColor(color_list.at(i % sz_clist));
            QString legend = tr("(I)_") + selFilenames.at(i);
            if (ckb_norm->isChecked())
                yp = integralN_sel.at(i).data();
            else
                yp = integral_sel.at(i).data();
            QwtPlotCurve* curve = us_curve(plot, legend);
            if (ckb_rawData->isChecked())
                curve->setYAxis(QwtPlot::yRight);
            curve->setPen(pen);
            curve->setSamples(xp, yp, np);

            for (int j = 0; j < np; j++){
                minX = qMin(minX, xp[j]);
                maxX = qMax(maxX, xp[j]);
                minY = qMin(minY, yp[j]);
                maxY = qMax(maxY, yp[j]);
            }
        }

        double dy = (maxY - minY) * 0.05;
        if (ckb_rawData->isChecked()){
            plot->setAxisScale( QwtPlot::yRight, minY - dy, maxY + dy);
        }
        else {
            plot->setAxisScale( QwtPlot::yLeft, minY - dy, maxY + dy);
        }
    }

    if (ckb_legend->isChecked()) {
        QwtLegend* legend = new QwtLegend();
        legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
        plot->insertLegend( legend, QwtPlot::BottomLegend   );
    } else {
        plot->insertLegend( NULL, QwtPlot::BottomLegend );
    }

    double dx = (maxX - minX) * 0.05;
    plot->setAxisScale( QwtPlot::xBottom, minX - dx, maxX + dx);
    plot->replot();
}

void US_Norm_Profile::slt_legend(int state) {

    if (state == Qt::Checked) {
        QwtLegend* legend = new QwtLegend();
        legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
        plot->insertLegend( legend, QwtPlot::BottomLegend   );
    } else {
        plot->insertLegend( NULL, QwtPlot::BottomLegend );
    }
    plot->replot();
}

void US_Norm_Profile::slt_rawData(int) {
    plotData();
}

void US_Norm_Profile::slt_integral(int) {
    plotData();
}

void US_Norm_Profile::slt_norm(int) {
    plotData();
}

void US_Norm_Profile::slt_pickRange(){
    x_min_picked = -1;
    x_max_picked = -1;
    if (selFilenames.size() == 0) return;
    if (picker_state == XNORM) return;

    picker_state = XRANGE;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    selectData();
    enableWidgets(false);
}

void US_Norm_Profile::slt_pickPoint() {
    if (selFilenames.size() == 0)  return;
    if (picker_state == XRANGE) return;
    pb_pick_norm->setStyleSheet("QPushButton { background-color: red }");
    selectData();
    enableWidgets(false);
    picker_state = XNORM;
}

void US_Norm_Profile::slt_mouse(const QwtDoublePoint& point){
    if (selFilenames.size() == 0) return;
    if (picker_state == XNONE) return;
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    QString bkgc = tr("QPushButton { background-color: %1 }").arg(color.name());

    if (picker_state == XRANGE) {
        double x = point.x();
        if (x_min_picked == -1){
            x_min_picked = x;
            double miny = plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
            double maxy = plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
            QVector<double> xx;
            QVector<double> yy;
            int np = 50;
            double dyy = (maxy - miny) / np;
            double y0 = miny;
            for (int i = 0; i < np; ++i){
                xx << x_min_picked;
                yy << y0 + i * dyy;
            }
            QPen pen(Qt::red);
            pen.setWidth(3);
            QwtPlotCurve* curve = us_curve( plot,"");
            curve->setStyle(QwtPlotCurve::Dots);
            curve->setPen(pen);
            curve->setSamples(xx.data(), yy.data(), np);
            //        grid = us_grid(plot);
            plot->replot();
        } else {
            if (x <= x_min_picked){
                QString mess("Pick a radial point greater than: %1 cm");
                QMessageBox::warning( this, tr( "Warning" ), mess.arg(x_min_picked));
                return;
            }
            x_max_picked = x;
            pb_pick_rp->setStyleSheet(bkgc);
            selectData();
            enableWidgets(true);
            picker_state = XNONE;
        }
    } else if (picker_state == XNORM) {
        x_norm = point.x();
        enableWidgets(true);
        pb_pick_norm->setStyleSheet(bkgc);
        picker_state = XNONE;
        selectData();
    }
}

void US_Norm_Profile::slt_reset(){
    slt_cleanList();
    lw_inpData->clear();
    filenames.clear();
    filePaths.clear();
    xvalues.clear();
    yvalues.clear();
    x_min_picked = -1;
    x_max_picked = -1;
    ckb_xrange->setCheckState(Qt::Unchecked);

}

void US_Norm_Profile::enableWidgets(bool state){
    pb_load->setEnabled(state);
    pb_reset->setEnabled(state);
    pb_save->setEnabled(state);
    pb_rmItem->setEnabled(state);
    pb_cleanList->setEnabled(state);
    lw_inpData->setEnabled(state);
    lw_selData->setEnabled(state);
    ckb_rawData->setEnabled(state);
    ckb_integral->setEnabled(state);
    ckb_legend->setEnabled(state);
    ckb_norm->setEnabled(state);
}

void US_Norm_Profile::slt_save(){
    int nd = selFilenames.size();
    if (nd == 0)
        return;

    QString fname = QFileDialog::getSaveFileName(this, tr("Save File"),
                               US_Settings::reportDir(),
                               tr("(*.csv)"));
    if (fname.size() == 0)
        return;

    QFileInfo inFInfo = QFileInfo(fname);
    QFileInfo outFInfo;
    if (inFInfo.suffix() == "csv")
        outFInfo = QFileInfo(inFInfo);
    else{
        fname = inFInfo.completeBaseName();
        fname.append(".csv");
        outFInfo = QFileInfo(inFInfo.dir(), fname);
    }

    qDebug() << outFInfo.absoluteFilePath();

    QFile file{outFInfo.absoluteFilePath()};
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream outStream{&file};
        QVector<int> nPoints;
        for (int i = 0; i < nd; i++){
            nPoints << xvalues_sel.at(i).size();
            outStream << tr("Filename,X_scan,Scan,Scan_norm,");
            outStream << tr("X_integral,Integral,Integral_norm");
            if (i != nd - 1)
                outStream << tr(",");
            else
                outStream << "\n";
        }

        bool newLine = true;
        int line = 0;
        while (newLine){
            newLine = false;
            for (int i = 0; i < nd; i++){
//                outStream << tr("Filename,X_scan,X_integral,");
                int np = nPoints.at(i);

                if (line == 0) {           //Filename
                    outStream << selFilenames.at(i) << ",";
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //X_scan
                    outStream << QString::number(xvalues_sel.at(i).at(line), 'f', 4) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //Scan
                    outStream << QString::number(yvalues_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //Scan_norm
                    outStream << QString::number(yvaluesN_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np - 1){    //X_integral
                    outStream << QString::number(midxval_sel.at(i).at(line), 'f', 4) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np - 1){        //Integral
                    outStream << QString::number(integral_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }
                if (line < np - 1){        //Integral_norm
                    outStream << QString::number(integralN_sel.at(i).at(line), 'f', 6);
                    newLine = true;
                }

                if (i != nd - 1)
                    outStream << tr(",");
                else
                    outStream << "\n";
            }
            line ++;
        }
    }
    qDebug() << "Saved the csv file!";
    file.close();
}

void US_Norm_Profile::closeEvent(QCloseEvent *event)
{
    emit widgetClosed();
    event->accept();
}

void US_Norm_Profile::slt_inItemSel(int row)
{
    if (row < 0){
        return;
    } else {
        le_runinfo->setText(lw_inpData->item(row)->text());
    }
}

void US_Norm_Profile::slt_outItemSel(int row)
{
    if (row < 0){
        return;
    } else {
        le_runinfo->setText(lw_selData->item(row)->text());
    }
}

void US_Norm_Profile::slt_norm_by_max(int state) {
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (state == Qt::Checked) {
        pb_pick_norm->setStyleSheet(qs.arg(color.name()));
        pb_pick_norm->setDisabled(true);
    } else {
        pb_pick_norm->setDisabled(false);
        if (x_norm == -1) {
            pb_pick_norm->setStyleSheet(qs.arg("yellow"));
        } else {
            pb_pick_norm->setStyleSheet(qs.arg(color.name()));
        }
    }
    selectData();
}
