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

    QLabel* lb_load = us_banner("Load Data");
    pb_load_auc = us_pushbutton("Load OpenAuc");
    pb_load_txt = us_pushbutton("Load TXT");
    pb_save = us_pushbutton("Save", false);
    pb_reset = us_pushbutton("Reset", false);
    QGridLayout* load_lyt = new QGridLayout();
    load_lyt->addWidget(lb_load,      0, 0, 1, 2);
    load_lyt->addWidget(pb_load_auc,  1, 0, 1, 1);
    load_lyt->addWidget(pb_load_txt,  1, 1, 1, 1);
    load_lyt->addWidget(pb_reset,     2, 0, 1, 1);
    load_lyt->addWidget(pb_save,      2, 1, 1, 1);

    lb_descr = us_label("", -1);
    lb_descr->setWordWrap(true);
    lb_descr->setAlignment(Qt::AlignTop);
    lb_descr->setMinimumHeight(50);
    lb_descr->setStyleSheet(tr("border: 1px solid black;"
                               "border-radius: 5px;"
                               "padding: 2px;"
                               "color: black;"
                               "background-color: white;"));

    QLabel* lb_scno = us_label("Scan:");
    ct_scans = us_counter(2, 0, 0, 0);
    QHBoxLayout* scno_lyt = new QHBoxLayout();
    scno_lyt->addWidget(lb_scno);
    scno_lyt->addWidget(ct_scans);



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
    left_lyt->addWidget(lb_descr);
    left_lyt->addLayout(scno_lyt);
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

    connect(pb_load_auc, SIGNAL(clicked()), this, SLOT(slt_load_auc()));
    connect(this, SIGNAL(sig_plot1(bool)), this, SLOT(slt_plot1(bool)));
}

void US_PeakDecomposition::slt_load_auc(){

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open AUC File"),
                                                    US_Settings::importDir(),
                                                    tr(".auc (*.auc)"));
    if (fileName.size() == 0)
        return;
    US_DataIO::RawData rawData;
    int state = US_DataIO::readRawData(fileName, rawData);
    if (state != US_DataIO::OK){
        QString msg = US_DataIO::errorString(state);
        QMessageBox::warning(this, "Error !", msg);
        return;
    }
    nscans = rawData.scanCount();
    int np = rawData.pointCount();
    for (int i= 0; i < nscans; ++i){
        xvalues << rawData.xvalues;
        yvalues << rawData.scanData.at(i).rvalues;
        descr << rawData.description + tr("_scan-%1").arg(i + 1);
        QVector<int> xlim;
        xlim << 0 << np;
        xlimits << xlim;
    }
    scanid = 0;
    ct_scans->disconnect();
    ct_scans->setRange(0, nscans - 1);
    ct_scans->setValue(scanid);
    ct_scans->setSingleStep(1);
    connect(ct_scans, SIGNAL(valueChanged(double)),
            this, SLOT(slt_scan(double)));
    emit sig_plot1(true);
    pb_load_auc->setEnabled(false);
    pb_load_txt->setEnabled(false);
    pb_reset->setEnabled(true);
}

void US_PeakDecomposition::slt_plot1(bool state){
    plot1->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
//        plot1->setTitle(tr(""));
        grid = us_grid(plot1);
        plot1->replot();
        return;
    }
    QPen pen(Qt::yellow);

    const double *xp, *yp;
    xp = xvalues.at(scanid).data();
    yp = yvalues.at(scanid).data();
//    QVector<double> xvec;
//    QVector<double> yvec;
    int id_i = xlimits.at(scanid).at(0);
    int id_f = xlimits.at(scanid).at(1);
    double min_x  = xp[id_i];
    double max_x  = xp[id_f - 1];
    double min_y  =  1e20;
    double max_y  = -1e20;
    for (int i = id_i ; i < id_f; ++i){
        min_y = qMin(min_y, yp[i]);
        max_y = qMax(max_y, yp[i]);
    }
    QwtPlotCurve* curve = us_curve(plot1,"");
    curve->setPen(pen);
    curve->setSamples(xp + id_i, yp + id_i, id_f - id_i);

    grid = us_grid(plot1);
    double dx = (max_x - min_x) * 0.05;
    double dr = (max_y - min_y) * 0.05;
    plot1->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    plot1->setAxisScale( QwtPlot::yLeft  , min_y - dr, max_y + dr);
    plot1->updateAxes();
    plot1->replot();
}

void US_PeakDecomposition::slt_scan(double id){
    scanid = (int) id;
    emit sig_plot1(true);
}

