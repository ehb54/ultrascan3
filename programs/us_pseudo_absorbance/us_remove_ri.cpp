#include "us_remove_ri.h"


US_RemoveRI::US_RemoveRI() : US_Widgets()
{
    setPalette( US_GuiSettings::frameColorDefault() );

    // Put the Run Info across the entire window
    QLabel* lb_runInfoInt  = us_banner(   tr( "Load Exprimental Data" ) );
    pb_import = us_pushbutton(tr("Import Experimental Data"));
    pb_reset = us_pushbutton("Reset", false, 0 );
    pb_reset->setDisabled(true);
    QHBoxLayout* import_lyt = new QHBoxLayout();
    import_lyt->addWidget(pb_import);
    import_lyt->addWidget(pb_reset);

    QLabel* lb_runIdIn  = us_label(tr("Run ID:"));
    lb_runIdIn->setAlignment(Qt::AlignRight);
    le_runIdIn          = us_lineedit(   "", 0, true );
    QLabel* lb_dirInt      = us_label(    tr( "Directory:" ) );
    lb_dirInt->setAlignment(Qt::AlignRight);
    le_dir              = us_lineedit( "", 0, true );
    QLabel* lb_desc  = us_label(    tr( "Description:" ), 0 );
    lb_desc->setAlignment(Qt::AlignRight);
    le_desc      = us_lineedit( "", 0, true);

    QGridLayout* info_lyt = new QGridLayout();
    info_lyt->addWidget(lb_dirInt,   0, 0, 1, 1);
    info_lyt->addWidget(le_dir,      0, 1, 1, 1);
    info_lyt->addWidget(lb_runIdIn,  1, 0, 1, 1);
    info_lyt->addWidget(le_runIdIn,  1, 1, 1, 1);
    info_lyt->addWidget(lb_desc,     2, 0, 1, 1);
    info_lyt->addWidget(le_desc,     2, 1, 1, 1);

    // Multi-Wavelength Lambda Controls
    QLabel* lb_mwlctrl = us_banner(tr( "Multi-Wavelength Lambda Controls" ), -1 );

    QLabel* lb_ccw  = us_label(tr("Cell / Channel / Wavelength:"));
    lb_ccw->setAlignment(Qt::AlignRight);
    cb_triples = us_comboBox();
    pb_prev_ccw  = us_pushbutton( "Previous CCW", true, 0 );
    pb_next_ccw  = us_pushbutton( "Next CCW",     true, 0 );
    pb_prev_ccw->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
    pb_next_ccw->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
    QGridLayout* ccw_lyt = new QGridLayout();
    ccw_lyt->addWidget(lb_ccw,      0, 0, 1, 1);
    ccw_lyt->addWidget(cb_triples,  0, 1, 1, 1);
    ccw_lyt->addWidget(pb_prev_ccw, 1, 0, 1, 1);
    ccw_lyt->addWidget(pb_next_ccw, 1, 1, 1, 1);

    // Multi-Wavelength Lambda Controls
    static QChar clambda( 955 );   // Lambda character
    QLabel* lb_lambstrt  = us_label   ( tr( "%1 Start:"    ).arg( clambda ) );
    lb_lambstrt->setAlignment(Qt::AlignRight);
    QLabel* lb_lambstop  = us_label   ( tr( "%1 End:"      ).arg( clambda ) );
    lb_lambstop->setAlignment(Qt::AlignRight);
    QLabel* lb_lambplot  = us_label   ( tr( "Plot %1:"     ).arg( clambda ) );
    lb_lambplot->setAlignment(Qt::AlignRight);
    le_lambstrt  = us_lineedit(tr(""), -1, true);
    le_lambstop  = us_lineedit(tr(""), -1, true);
    cb_plot_id  = us_comboBox();
    cb_plot_id->setEditable(true);
    pb_prev_id  = us_pushbutton( "Previous", true, 0 );
    pb_next_id  = us_pushbutton( "Next",     true, 0 );
    pb_prev_id->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
    pb_next_id->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );

    QHBoxLayout* wvl_rng_lyt = new QHBoxLayout();
    wvl_rng_lyt->addWidget(lb_lambstrt);
    wvl_rng_lyt->addWidget(le_lambstrt);
    wvl_rng_lyt->addWidget(lb_lambstop);
    wvl_rng_lyt->addWidget(le_lambstop);
    QHBoxLayout* wvl_plt_lyt = new QHBoxLayout();
    wvl_plt_lyt->addWidget(lb_lambplot);
    wvl_plt_lyt->addWidget(cb_plot_id);
    wvl_plt_lyt->addWidget(pb_prev_id);
    wvl_plt_lyt->addWidget(pb_next_id);

    QLabel* lb_fit_ctrl = us_banner("Polynomial Fit Control");
    pb_pick_rp = us_pushbutton("Pick Two Points", true);
    le_xrange = us_lineedit("", 0, true);
    QHBoxLayout *xrange_lyt = new QHBoxLayout();
    xrange_lyt->addWidget(pb_pick_rp);
    xrange_lyt->addWidget(le_xrange);
    pb_fit = us_pushbutton("Remove RI");
    xrange_lyt->addWidget(pb_fit);

    cb_autofit = new QCheckBox();
    QGridLayout* uscb_autofit = us_checkbox("Autofit", cb_autofit, true);
    QHBoxLayout *autofit_lyt = new QHBoxLayout();
    lb_maxorder = us_label("Max Order:");
    lb_maxorder->setAlignment(Qt::AlignRight);
    ct_max_order = us_counter(1, 1, max_ct, max_ct);
    ct_max_order->setSingleStep(1);
    autofit_lyt->addLayout(uscb_autofit);
    autofit_lyt->addWidget(lb_maxorder);
    autofit_lyt->addWidget(ct_max_order);

    lb_manorder = us_label("Order:");
    lb_manorder->setAlignment(Qt::AlignRight);
    ct_order = us_counter(1, 1, max_ct, 6);
    ct_order->setSingleStep(1);
    lb_fitorder = us_label("Order:");
    lb_fitorder->setAlignment(Qt::AlignRight);
    le_fitorder = us_lineedit("", 0, true);
    lb_fitrsqrd = us_label("R-Squared:");
    lb_fitrsqrd->setAlignment(Qt::AlignRight);
    le_fitrsqrd = us_lineedit("", 0, true);
    le_fitorder->setMaximumWidth(50);

    QHBoxLayout *fitctrl_lyt = new QHBoxLayout();
    fitctrl_lyt->addWidget(lb_manorder);
    fitctrl_lyt->addWidget(ct_order);
    fitctrl_lyt->addWidget(lb_fitorder);
    fitctrl_lyt->addWidget(le_fitorder);
    fitctrl_lyt->addWidget(lb_fitrsqrd);
    fitctrl_lyt->addWidget(le_fitrsqrd);
    lb_manorder->hide();
    ct_order->hide();

    QLabel* lb_save = us_banner("Save Control");
    QLabel* lb_runIdOut  = us_label(tr("Output Run ID:"));
    le_runIdOut = new US_LineEdit_RE("", 0, false);
    QHBoxLayout* runIdOut_lyt = new QHBoxLayout();
    runIdOut_lyt->addWidget(lb_runIdOut);
    runIdOut_lyt->addWidget(le_runIdOut);

    pb_save = us_pushbutton("Save Absorbance Data", false, 0 );
    pb_save->setDisabled(true);

    QLabel* lb_status = us_label(tr("Status:"));
    le_status = us_lineedit(tr(""), -1, true);
    QPalette stpal;
    stpal.setColor( QPalette::Text, Qt::white );
    stpal.setColor( QPalette::Base, Qt::blue  );
    le_status->setPalette( stpal );
    QHBoxLayout* status_lyt = new QHBoxLayout();
    status_lyt->addWidget(lb_status);
    status_lyt->addWidget(le_status);

    QPushButton* pb_help = us_pushbutton("Help", true, 0 );
    pb_close = us_pushbutton("Close", true, 0 );
    QHBoxLayout* close_lyt = new QHBoxLayout();
    close_lyt->addWidget(pb_help);
    close_lyt->addWidget(pb_close);

    QVBoxLayout* left_lyt = new QVBoxLayout();
    left_lyt->addStretch(0);
    left_lyt->setSpacing(1);
    left_lyt->addWidget(lb_runInfoInt);
    left_lyt->addLayout(import_lyt);
    left_lyt->addLayout(info_lyt);
    left_lyt->addWidget(lb_mwlctrl);
    left_lyt->addLayout(ccw_lyt);
    left_lyt->addLayout(wvl_rng_lyt);
    left_lyt->addLayout(wvl_plt_lyt);

    left_lyt->addWidget(lb_fit_ctrl);
    left_lyt->addLayout(xrange_lyt);
    left_lyt->addLayout(autofit_lyt);
    left_lyt->addLayout(fitctrl_lyt);
    left_lyt->addWidget(lb_save);
    left_lyt->addLayout(runIdOut_lyt);
    left_lyt->addWidget(pb_save);
    left_lyt->addStretch(1);
    left_lyt->addLayout(status_lyt);
    left_lyt->addLayout(close_lyt);
    left_lyt->addStretch(0);

//    //*****right*****//
//    plot_title = us_label("");

    QwtText xLabel, yLabel;
    usplot_data = new US_Plot( qwtplot_data, tr( "" ),
                                tr( "Radius (in cm)" ), tr( "Intensity" ),
                                true, "", "" );
    qwtplot_data->setMinimumSize( 650, 300 );
    qwtplot_data->enableAxis( QwtPlot::xBottom, true );
    qwtplot_data->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_data->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_data);

    usplot_fit = new US_Plot( qwtplot_fit, tr( "" ),
                              tr( "Scan Number" ), tr( "Integral" ),
                              true, "", "" );
    qwtplot_fit->setMinimumSize( 650, 300 );
    qwtplot_fit->enableAxis( QwtPlot::xBottom, true );
    qwtplot_fit->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_fit->setCanvasBackground(QBrush(Qt::black));
//    qwtplot_fit->setCanvasBackground(QBrush(QColor(254,242,228)));
    grid = us_grid(qwtplot_fit);
    QwtLegend *legend = new QwtLegend();
    qwtplot_fit->insertLegend( legend , QwtPlot::BottomLegend);

    QVBoxLayout* right_lyt = new QVBoxLayout();
    right_lyt->setSpacing(0);
    right_lyt->addLayout(usplot_data);
    right_lyt->addLayout(usplot_fit);

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    left_lyt->setSizeConstraint(QLayout::SetMinimumSize);
    main_lyt->addLayout(left_lyt, 0);
//    main_lyt->addStretch(1);
    main_lyt->addLayout(right_lyt, 1);
    main_lyt->setSpacing(1);
    main_lyt->setMargin(1);
    setLayout(main_lyt);

    picker = new US_PlotPicker(qwtplot_data);
    picker->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );

    slt_reset();

    connect(pb_import, SIGNAL(clicked()), this, SLOT(slt_import()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_prev_id,      SIGNAL(clicked()), this, SLOT(slt_prev_id()));
    connect(pb_next_id,      SIGNAL(clicked()), this, SLOT(slt_next_id()));
    connect(pb_prev_ccw,     SIGNAL(clicked()), this, SLOT(slt_prev_ccw()));
    connect(pb_next_ccw,     SIGNAL(clicked()), this, SLOT(slt_next_ccw()));
    connect(this, SIGNAL(sig_plot(bool)), this, SLOT(slt_plot(bool)));
    connect(pb_pick_rp, SIGNAL(clicked()),
                this, SLOT(slt_pick_point()));
    connect(cb_autofit, SIGNAL(stateChanged(int)), this, SLOT(slt_autofit_state(int)));
    connect(pb_fit, SIGNAL(clicked()), this, SLOT(slt_polyfit()));
    connect(ct_max_order, SIGNAL(valueChanged(double)), this, SLOT(slt_rm_fit(double)));
    connect(ct_order, SIGNAL(valueChanged(double)), this, SLOT(slt_rm_fit(double)));
    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));
}


void US_RemoveRI::slt_reset(){
    picker->disconnect();
    cb_triples->disconnect();
    cb_plot_id->disconnect();
    hasData = false;
    allData.clear();
    allDataC.clear();
    allIntegrals.clear();
    allIntegralsC.clear();
    intgState.clear();
    fitState.clear();
    fitOrder.clear();
    fitRsqrd.clear();
    fitParam.clear();
    ccwList.clear();
    ccwStrList.clear();
    ccwItemList.clear();
    wavelength.clear();
    idMin.clear();
    idMax.clear();
    ccwFitState.clear();
    ccwIntgState.clear();
    le_runIdIn->clear();
    le_runIdOut->clear();
    le_lambstrt->clear();
    le_lambstop->clear();
    le_dir->clear();
    le_desc->clear();
    le_xrange->clear();
    le_fitorder->clear();
    le_fitrsqrd->clear();

    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    pb_fit->setStyleSheet(qs.arg(color.name()));
    pb_pick_rp->setStyleSheet(qs.arg(color.name()));

    pb_import->setEnabled(true);

    pb_reset->setDisabled(true);
    pb_save->setDisabled(true);
    pb_prev_id->setDisabled(true);
    pb_next_id->setDisabled(true);
    pb_prev_ccw->setDisabled(true);
    pb_next_ccw->setDisabled(true);
    pb_pick_rp->setDisabled(true);
    pb_fit->setDisabled(true);
    cb_plot_id->setDisabled(true);
    cb_triples->setDisabled(true);
    ct_max_order->setDisabled(true);
    ct_order->setDisabled(true);
    cb_autofit->setDisabled(true);
    emit sig_plot(false);
}

void US_RemoveRI::slt_import(void){

    QString dir;

    dir = QFileDialog::getExistingDirectory( this,
          tr( "Open Raw Data Directory" ),
          US_Settings::importDir(),
          QFileDialog::DontResolveSymlinks );

    if (dir.isEmpty()) return;

    QDir inDir = QDir( dir, "*.auc", QDir::Name, QDir::Files | QDir::Readable );
    if (inDir.count() == 0){
        le_status->setText("auc files not found");
        return;
    }
    inDir.makeAbsolute();
    le_status->setText("parsing error!");
    QFileInfoList fileList = inDir.entryInfoList();
    QString runId;
    QString runType;
    allData.clear();
    allDataC.clear();
    QStringList allowedRunTypes;
    allowedRunTypes << "RI" << "RA";
    QRegExp re( "[^A-Za-z0-9_-]" );
    bool runId_changed = false;
    QString ccw_str("%1 / %2 / %3");
    for (int i = 0; i < fileList.size(); ++i){
        QString fname = fileList.at(i).fileName();
        QString rtp = fname.section(".", -5, -5);
        QString rid = fname.section(".", 0, -6);
        int reIdx = rid.indexOf(re, 0);
        if (reIdx >=0) runId_changed = true;
        while (reIdx >=0){
            rid = rid.replace(reIdx, 1, "_");
            reIdx = rid.indexOf(re, reIdx);
        }
        if (runId.isNull())
            runId = rid;
        else
            if (runId != rid){
                QMessageBox::warning( this,
                      tr( "Error" ),
                      tr( "Multiple runIDs found in the directory" ));
                return;
            }
        if (! allowedRunTypes.contains(rtp)){
            QMessageBox::warning( this,
                  tr( "Error" ),
                  tr( "The Run type (%1) is not supported." ).arg(rtp));
            return;
        }
        if (runType.isNull())
            runType = rtp;
        else
            if (runType != rtp){
                QMessageBox::warning( this,
                      tr( "Error" ),
                      tr( "Multiple run types found in the directory" ));
                return;
            }

        US_DataIO::RawData rdata = US_DataIO::RawData();
        US_DataIO::readRawData(fileList.at(i).absoluteFilePath(), rdata);
        allData << rdata;
        uchar uuid[ 16 ];
        QString uuid_string = US_Util::new_guid();
        US_Util::uuid_parse( uuid_string, uuid );
        memcpy( rdata.rawGUID,   (char*) uuid, 16 );
        allDataC << rdata;
        allDataFInfo << fileList.at(i);
        QVector<double> intg(rdata.scanCount(), 0);
        allIntegrals << intg;
        allIntegralsC << intg;
        intgState << false;
        fitState << false;

        QVector<double> param(max_ct + 1, 0);
        fitParam << param;

        int cell = rdata.cell;
        char channel = rdata.channel;
        double wl = rdata.scanData.at(0).wavelength;
        ccwStrList << ccw_str.arg(cell).arg(channel).arg(wl);

        ccwList.index << i;
        ccwList.cell << cell;
        ccwList.channel << channel;
        ccwList.wavelength << wl;

    }
    fitOrder.fill(0, allData.size());
    fitRsqrd.fill(0, allData.size());

    if ( runId_changed )
    {
       QMessageBox::warning( this,
             tr( "RunID Name Changed" ),
             tr( "The runID name has been changed. It may consist only"
                 "of alphanumeric \n"
                 " characters, the underscore, and the hyphen. New runID: " )
             + runId );
    }
    le_runIdIn->setText(runId);
    le_runIdIn->setCursorPosition(0);
    le_dir->setText(US_Settings::importDir());
    le_dir->setCursorPosition(0);
    runIdOut = runId.prepend("RI_");
    le_runIdOut->setText(runIdOut);
    set_cb_triples();
    slt_new_ccw(0);
    pb_save_avail();
    hasData = true;
    le_status->clear();

    pb_import->setDisabled(true);

    pb_reset->setEnabled(true);
    pb_prev_id->setEnabled(true);
    pb_next_id->setEnabled(true);
    pb_prev_ccw->setEnabled(true);
    pb_next_ccw->setEnabled(true);
    pb_pick_rp->setEnabled(true);
    pb_fit->setEnabled(true);
    cb_plot_id->setEnabled(true);
    cb_triples->setEnabled(true);
    ct_max_order->setEnabled(true);
    ct_order->setEnabled(true);
    cb_autofit->setEnabled(true);
}

void US_RemoveRI::slt_prev_ccw(void){
    --ccw_id;
    cb_triples->setCurrentIndex(ccw_id);
}

void US_RemoveRI::slt_next_ccw(void){
    ++ccw_id;
    cb_triples->setCurrentIndex(ccw_id);
}

void US_RemoveRI::slt_prev_id(void){
    --wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
}

void US_RemoveRI::slt_next_id(void){
    ++wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
}

void US_RemoveRI::pn_id_avail(){
    pb_prev_id->setDisabled(wavl_id <= 0);
    pb_next_id->setDisabled(wavl_id >= (n_wavls - 1));
}

void US_RemoveRI::pn_ccw_avail(){
    pb_prev_ccw->setDisabled(ccw_id <= 0);
    pb_next_ccw->setDisabled(ccw_id >= (n_ccw - 1));
}

void US_RemoveRI::slt_new_ccw(int id){
    ccw_id = id;
    cb_plot_id->disconnect();
    cb_plot_id->clear();
    intDataId.clear();
    wavelength.clear();
    wavl_id = 0;
    n_wavls = 0;

    int ccw_id = id;
    n_wavls = ccwItemList.wavelength.at(ccw_id).size();
    intDataId = ccwItemList.index.at(ccw_id);
    wavelength = ccwItemList.wavelength.at(ccw_id);
    le_lambstrt->setText(QString::number(wavelength.at(0)));
    le_lambstop->setText(QString::number(wavelength.at(n_wavls - 1)));
    QStringList items;
    for (int i = 0; i < n_wavls; ++i)
        items << QString::number(wavelength.at(i));
    cb_plot_id->addItems(items);
    pn_id_avail();
    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_id(int)));

    pn_ccw_avail();

    QString qs = "QPushButton { background-color: %1 }";
    if (idMin.at(ccw_id) < 0 || idMax.at(ccw_id) < 0){
        idMin[ccw_id] = -1;
        idMax[ccw_id] = -1;
        le_xrange->setText("");
        pb_pick_rp->setStyleSheet(qs.arg("yellow"));
    } else {
        QString str = tr("%1 - %2 cm");
        int index = ccwItemList.index.at(ccw_id).at(0);
        double lx = allData.at(index).xvalues.at(idMin.at(ccw_id));
        double hx = allData.at(index).xvalues.at(idMax.at(ccw_id));
        le_xrange->setText(str.arg(lx, 0, 'f', 3).arg(hx, 0, 'f', 3));
//        le_xrange->setText(str.arg(idMin.at(ccw_id), 0, 'f', 3).
//                           arg(idMax.at(ccw_id), 0, 'f', 3));

        pb_pick_rp->setStyleSheet(qs.arg("green"));
    }
    pb_fit_avail();

    slt_set_id(0);
}

void US_RemoveRI::slt_set_id(int id){
    wavl_id = id;
    pn_id_avail();
    le_desc->setText(allData.at(id).description);
    le_desc->setCursorPosition(0);
    emit sig_plot(true);
}

void US_RemoveRI::slt_pick_point(){
    picker->disconnect();
    idMin[ccw_id] = -1;
    idMax[ccw_id] = -1;
    le_xrange->setText("");
    if (allData.size() == 0)
        return;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    pb_next_ccw->setDisabled(true);
    pb_prev_ccw->setDisabled(true);
    cb_triples->setDisabled(true);
    clean_states(INTG_FIT_S);
    ccwIntgState[ccw_id] = false;
    ccwFitState[ccw_id] = false;
    pb_fit_avail();
    emit sig_plot(true);
    pb_save_avail();
    connect(picker, SIGNAL(cMouseUp(const QwtDoublePoint&)),
            this, SLOT(slt_mouse(const QwtDoublePoint&)));
    return;
}

int US_RemoveRI::get_id(QVector<double> vec, double val){
    int id = 0;
    for (int i = 0; i < vec.size(); i++){
        id = i;
        if(vec.at(i) > val)
            break;
    }
    return id;
}

void US_RemoveRI::slt_mouse(const QwtDoublePoint& point){
    double x = point.x();
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
    US_DataIO::RawData data = allData.at(index);
    int np = data.pointCount();
    double min_x = data.xvalues.at(0);
    double max_x = data.xvalues.at(np - 1);
    QString str;
    if (x > min_x && x < max_x){
        if (idMin.at(ccw_id) == -1){
            idMin[ccw_id] = get_id(data.xvalues, x);
            str = tr("%1 -");
            le_xrange->setText(str.arg(x, 0, 'f', 3));
            emit sig_plot(true);
        } else {
            double lx = data.xvalues.at(idMin.at(ccw_id));
            if (x <= lx + 0.01){
                QString mess("Pick a radial point greater than: %1 cm");
                QMessageBox::warning( this, tr( "Warning" ), mess.arg(lx));
                return;
            }
            idMax[ccw_id] = get_id(data.xvalues, x);
            picker->disconnect();
            str = tr("%1 - %2 cm");
            double hx = data.xvalues.at(idMax.at(ccw_id));
            le_xrange->setText(str.arg(lx, 0, 'f', 3).arg(hx, 0, 'f', 3));

//            le_xrange->setText(str.arg(idMin.at(ccw_id), 0, 'f', 3).
//                               arg(idMax.at(ccw_id), 0, 'f', 3));
            pb_pick_rp->setStyleSheet("QPushButton { background-color: green }");
            pn_ccw_avail();
            cb_triples->setEnabled(true);
            integrate(RDATA_S);
            ccwIntgState[ccw_id] = true;
            pb_fit_avail();
            emit sig_plot(true);
        }
    }else{
        QString mess("Pick a point between the minimum and maximum"
                     "values of the radial points.\n"
                     "Minimum= %1 cm, Maximum= %2 cm");
        QMessageBox::warning( this, tr( "Warning" ), mess.arg(min_x).arg(max_x));
    }
    return;
}

void US_RemoveRI::slt_save(void){
    QDir dir = QDir(le_dir->text());
    if (dir.cd(runIdOut)){
        QString absPath = dir.absolutePath();
        int ck = QMessageBox::question(this, tr( "Warning!" ),
                                       tr( "Output directory exists!\n" ) + runIdOut +
                                       tr( "\n\n Do you really want to replace it?"));
        if (ck == QMessageBox::Yes){
            dir.removeRecursively();
            dir.mkpath(absPath);
            dir.setPath(absPath);
        }else return;
    }else{
        dir.mkdir(runIdOut);
        dir.cd(runIdOut);
    }
    dir.makeAbsolute();
    qDebug() << dir.path();
    int nwl_tot = 0;
    for (int i = 0; i < n_ccw; ++i) {
        nwl_tot += ccwItemList.wavelength.at(i).size();
    }

    int n = 1;
    QString status = tr("writting: %1 %2");
    QString percent;
    QString fileName("%1.RA.%2.%3.%4.auc");
    for (int i = 0; i < n_ccw; ++i){
        for (int j = 0; j < ccwItemList.wavelength.at(i).size(); ++j){
            percent = QString::number(100.0 * n / nwl_tot, 'f', 1);
            le_status->setText(status.arg(percent).arg(QChar(37)));
            qApp->processEvents();
            n++;
            int dataId = ccwItemList.index.at(i).at(j);
            double wavelength = ccwItemList.wavelength.at(i).at(j);

            US_DataIO::RawData rawData = allDataC.at(dataId);
            int cell = rawData.cell;
            char channel = rawData.channel;
            QString fn = fileName.arg(runIdOut).arg(cell).arg(channel).arg(wavelength);
            QFileInfo fileInfo(dir, fn);
            US_DataIO::writeRawData(fileInfo.absoluteFilePath(), rawData);
        }
    }
    le_status->setText("written on the local disk !");

    return;
}

void US_RemoveRI::set_cb_triples(){
    ccwItemList.clear();
    if (ccwList.size() == 0){
        return;
    }

    QVector<int> cell_list;
    for (int i = 0; i < ccwList.size(); ++i){
        int cell = ccwList.cell.at(i);
        if (cell_list.size() == 0){
            cell_list << cell;
            continue;
        }
        if (! cell_list.contains(cell))
            cell_list << cell;
    }
    std::sort(cell_list.begin(), cell_list.end());

    QVector<QVector<char>> channel_list;
    for (int i = 0; i < cell_list.size(); ++i){
        QVector<char> channel_list_i;
        int cell = cell_list.at(i);
        for (int j = 0; j < ccwList.size(); ++j){
            char channel = ccwList.channel.at(j);
            if (ccwList.cell.at(j) == cell){
                if (channel_list_i.size() == 0)
                    channel_list_i << channel;
                else
                    if (! channel_list_i.contains(channel))
                        channel_list_i << channel;
            }
        }
        std::sort(channel_list_i.begin(), channel_list_i.end());
        channel_list << channel_list_i;
    }

    for (int i = 0; i < cell_list.size(); ++i){
        int cell = cell_list.at(i);
        for (int j = 0; j < channel_list.at(i).size(); ++j){
            char channel = channel_list.at(i).at(j);
            ccwItemList.cell << cell;
            ccwItemList.channel << channel;
            QVector<double> wl_list;
            QVector<int> index;
            for (int k = 0; k < ccwList.size(); ++k){
                double wl = ccwList.wavelength.at(k);
                if (ccwList.cell.at(k) == cell && ccwList.channel.at(k) == channel){
                    wl_list << wl;
                    index << ccwList.index.at(k);
                }
            }
            std::sort(wl_list.begin(), wl_list.end());
            ccwItemList.index << index;
            ccwItemList.wavelength << wl_list;
        }
    }

    cb_triples->disconnect();
    cb_triples->clear();
    if (ccwItemList.size() == 0)
        return;
    QString item("%1 / %2 / %3-%4 (%5)");
    n_ccw = ccwItemList.size();
    for (int i = 0; i < n_ccw; ++i){
        int cell = ccwItemList.cell.at(i);
        char channel = ccwItemList.channel.at(i);
        double min_wl = ccwItemList.wavelength.at(i).first();
        double max_wl = ccwItemList.wavelength.at(i).last();
        int nwl = ccwItemList.wavelength.at(i).size();
        QString item_i = item.arg(cell).arg(channel)
                .arg(min_wl).arg(max_wl).arg(nwl);
        cb_triples->addItem(item_i);
    }
    ccw_id = 0;
    idMin.fill(-1, n_ccw);
    idMax.fill(-1, n_ccw);
    ccwFitState.fill(false, n_ccw);
    ccwIntgState.fill(false, n_ccw);
    connect( cb_triples, SIGNAL( currentIndexChanged(int) ),
            this, SLOT( slt_new_ccw(int) ) );
    cb_triples->setCurrentIndex(ccw_id);
}

void US_RemoveRI::slt_plot(bool state){
    plot_data(state);
    plot_fit(state);
}


void US_RemoveRI::plot_data(bool state){
    qwtplot_data->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        grid = us_grid(qwtplot_data);
        qwtplot_data->replot();
        return;
    }
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
    bool fit_state = fitState.at(index);
    US_DataIO::RawData data;
    QString title("wavelength= ");
    if (fit_state){
        data = allDataC.at(index);
        title += QString::number(data.scanData.at(0).wavelength);
        title += tr(" nm, corrected data");
    } else {
        data = allData.at(index);
        title += QString::number(data.scanData.at(0).wavelength);
        title += tr(" nm, raw data");
    }

    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(1);
    int ns = data.scanCount();
    double rb = qCeil(ns / 50.0);
    int nc = qCeil(ns / rb);
    HSVcolormap colormap(nc);
    QVector<QColor> colorList;
    int error = colormap.get_colorlist(colorList);

    const double *x, *r;
    x = data.xvalues.data();
    int np = data.pointCount();

    double min_x  =  1e20;
    double max_x  = -1e20;
    double min_r  =  1e20;
    double max_r  = -1e20;
    for (int i = 0; i < ns; ++i){
        r = data.scanData.at(i).rvalues.data();
        for (int j = 0; j < np; ++j){
            min_x = qMin(min_x, x[j]);
            max_x = qMax(max_x, x[j]);
            min_r = qMin(min_r, r[j]);
            max_r = qMax(max_r, r[j]);
        }
        QwtPlotCurve* curve = us_curve( qwtplot_data,"");
        if (error == 0)
            pen_plot.setColor(colorList[ i % nc ]);
        curve->setPen( pen_plot );
        curve->setSamples(x, r, np);
    }
    double dx = (max_x - min_x) * 0.05;
    double dr = (max_r - min_r) * 0.05;

    if (idMin.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        double lx = data.xvalues.at(idMin.at(ccw_id));
        for (int i = 0; i < np; ++i){
            xx << lx;
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_data,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    if (idMax.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        double hx = data.xvalues.at(idMax.at(ccw_id));
        for (int i = 0; i < np; ++i){
            xx << hx;
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_data,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    grid = us_grid(qwtplot_data);

    qwtplot_data->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    qwtplot_data->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_data->updateAxes();

    qwtplot_data->setTitle(title);
    qwtplot_data->replot();
    return;
}

void US_RemoveRI::plot_fit(bool state){
    qwtplot_fit->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        grid = us_grid(qwtplot_fit);
        qwtplot_fit->replot();
        return;
    }
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
    bool intg_state = intgState.at(index);
    bool fit_state = fitState.at(index);
    le_fitrsqrd->clear();
    le_fitorder->clear();
    if (! intg_state){
        grid = us_grid(qwtplot_fit);
        qwtplot_fit->replot();
        return;
    }
    QVector<double> integralC = allIntegralsC.at(index);
    QVector<double> integral = allIntegrals.at(index);
    int ns = integral.size();
    QVector<double> xvec(ns);
    double min_r  =  1e20;
    double max_r  = -1e20;
    for (int i = 0; i < ns; i++){
        min_r = qMin(integral.at(i), min_r);
        max_r = qMax(integral.at(i), max_r);
        xvec[i] = i;
    }
    double dr = (max_r - min_r) * 0.05;
    QPen pen_plot(Qt::yellow);

    // raw
    QwtPlotCurve* curve = us_curve( qwtplot_fit,"raw data");
    pen_plot.setColor(Qt::yellow);
    pen_plot.setWidth(1);
    curve->setPen( pen_plot );
    curve->setSamples(xvec.data(), integral.data(), ns);

    // fitted line
    if (fit_state){
        le_fitrsqrd->setText(QString::number(fitRsqrd.at(index)));
        le_fitorder->setText(QString::number(fitOrder.at(index) - 1));

        QVector<double> fit;
        const double *xp = xvec.data();
        const double *cp = fitParam.at(index).data();
        for (int i = 0; i < ns; i++){
            double yf = 0;
            for (int j = 0; j < fitOrder.at(index); j++)
                yf += cp[j] * qPow(xp[i], j);
            fit << yf;
        }
        pen_plot.setColor(Qt::green);
        pen_plot.setWidth(3);
        QwtPlotCurve* curve = us_curve( qwtplot_fit,"fitted line");
        curve->setPen( pen_plot );
        curve->setSamples(xp, fit.data(), ns);
    }

    // corrected
    if (fit_state){
        QwtSymbol *symbol = new QwtSymbol(
                    QwtSymbol::Ellipse, QBrush(Qt::red),
                    QPen(Qt::red, 1), QSize( 3, 3 ));
        QwtPlotCurve* curve = us_curve( qwtplot_fit,"corrected data");
        pen_plot.setColor(Qt::red);
        pen_plot.setWidth(0);
        curve->setPen( pen_plot );
        curve->setSymbol(symbol);
        curve->setSamples(xvec.data(), integralC.data(), ns);
    }

    grid = us_grid(qwtplot_fit);
    qwtplot_fit->setAxisScale( QwtPlot::xBottom, 0, ns + 1);
    qwtplot_fit->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_fit->updateAxes();
    qwtplot_fit->replot();
    return;
}



void US_RemoveRI::slt_autofit_state(int state){
    if (state == Qt::Checked){
        lb_maxorder->show();
        ct_max_order->show();
        lb_manorder->hide();
        ct_order->hide();
        lb_fitorder->show();
        le_fitorder->show();
        lb_fitrsqrd->show();
        le_fitrsqrd->show();
    } else {
        lb_maxorder->hide();
        ct_max_order->hide();
        lb_manorder->show();
        ct_order->show();
        lb_fitorder->hide();
        le_fitorder->hide();
        lb_fitrsqrd->show();
        le_fitrsqrd->show();
    }
    le_fitrsqrd->setText("");
    clean_states(FIT_S);
    ccwFitState[ccw_id] = false;
    pb_fit_avail();
    pb_save_avail();
    emit sig_plot(true);
}

void US_RemoveRI::integrate(int state){
    QVector<int> index = ccwItemList.index.at(ccw_id);
    int lxid = idMin.at(ccw_id);
    int hxid = idMax.at(ccw_id);
    bool *sp = intgState.data();
    for (int i = 0; i < index.size(); i++){
        int id = index.at(i);
        sp[id] = true;
        int ns = allData.at(id).scanData.size();
        QVector<double> xvalues;
        xvalues << allData.at(id).xvalues;
        const double *x = xvalues.data();
        double *intg;
        if (state == RDATA_S)
            intg = allIntegrals[id].data();
        else if (state == CDATA_S)
            intg = allIntegralsC[id].data();

        for (int j = 0; j < ns; j++){
            QVector<double> rvalues;
            if (state == RDATA_S)
                rvalues << allData.at(id).scanData.at(j).rvalues;
            else if (state == CDATA_S)
                rvalues << allDataC.at(id).scanData.at(j).rvalues;
            const double *y = rvalues.data();
            double sum = trapz(x, y, lxid, hxid);
            intg[j] = sum;
        }
    }
}

void US_RemoveRI::clean_states(int state){
    QVector<int> index = ccwItemList.index.at(ccw_id);
    bool *ints = intgState.data();
    bool *fits = fitState.data();
    for (int i = 0; i < index.size(); i++){
        if (state == INTG_S)
            ints[index.at(i)] = false;
        else if (state == FIT_S)
            fits[index.at(i)] = false;
        else if (state == INTG_FIT_S){
            ints[index.at(i)] = false;
            fits[index.at(i)] = false;
        }
    }
}

double US_RemoveRI::trapz(const double* x, const double* y,
                          int lxid, int hxid){
    double dx;
    double sum = 0;
    for (int i = lxid + 1; i <= hxid; i++){
        dx = x[i] - x[i - 1];
        sum += dx * ( y[i] + y[i - 1] ) * 0.5;
    }
    return sum;
}

void US_RemoveRI::slt_polyfit(){
    QVector<int> index = ccwItemList.index.at(ccw_id);
    int nwl = index.size();
    for (int i = 0; i < nwl; i++){
        int id = index.at(i);
        QVector<double> y = allIntegrals.at(id);
        double *yp = y.data();
        int ns = y.size();
        QVector<double> x(ns);
        double *xp = x.data();
        for (int j = 0; j < ns; j++)
            xp[j] = (double) j;
        if (cb_autofit->isChecked()){
            QVector<double> coeff;
            int order = -1;
            double max_rsqrd = 0;
            int ctval = (int) ct_max_order->value();
            for (int j = 1; j <= ctval; j++ ){
                int order_tst = j + 1;
                QVector<double> coeff_tst(max_ct + 1, 0);
                bool st = US_Matrix::lsfit(coeff_tst.data(), xp, yp, ns, order_tst);
                if (st){
                    double rsqrd = get_rsqrd(xp, yp, ns, coeff_tst);
                    if (rsqrd > max_rsqrd){
                        max_rsqrd = rsqrd;
                        order = order_tst;
                        coeff.clear();
                        coeff << coeff_tst;
                    }
                }
            }
            if (order == -1){
                QMessageBox::warning(this, tr("Error!"),
                                     tr("Autofit couldn't found the proper polynomial order"));
                clean_states(FIT_S);
                return;
            }
            fitOrder[id] = order;
            fitRsqrd[id] = max_rsqrd;
            fitState[id] = true;
            double *cp = fitParam[id].data();
            for (int j = 0; j < coeff.size(); j++)
                cp[j] = coeff.at(j);
        } else {
            int order = (int) ct_order->value() + 1;
            QVector<double> coeff(max_ct + 1, 0);
            bool st = US_Matrix::lsfit(coeff.data(), xp, yp, ns, order);
            if (st){
                double rsqrd = get_rsqrd(xp, yp, ns, coeff);
                fitOrder[id] = order;
                fitRsqrd[id] = rsqrd;
                fitState[id] = true;
                double *cp = fitParam[id].data();
                for (int j = 0; j < coeff.size(); j++)
                    cp[j] = coeff.at(j);
            } else {
                QMessageBox::warning(this, tr("Error!"),
                                     tr("Couldn't fit the polynomial function with order of: ") +
                                     QString::number(order));
                clean_states(FIT_S);
                return;
            }
        }
    }
    correct_data();
    ccwFitState[ccw_id] = true;
    pb_fit_avail();
    pb_save_avail();
    emit sig_plot(true);
}

double US_RemoveRI::get_rsqrd(double* x, double* y, int np, QVector<double> coeff){
    double res = 0;
    double ave = 0;
    for (int i = 0; i < np; i++){
        double yf = 0;
        for (int j = 0; j < coeff.size(); j++)
            yf += coeff.at(j) * qPow(x[i], j);
        res += qPow(y[i] - yf, 2);
        ave += y[i];
    }
    ave /= np;
    double var = 0;
    for (int i = 0; i < np; i++)
        var += qPow(y[i] - ave, 2);
    double rs = 1 - (res / var);
    return rs;
}

void US_RemoveRI::correct_data(){
    QVector<int> index = ccwItemList.index.at(ccw_id);
    int nwl = index.size();
    for (int i = 0; i < nwl; i++){
        int id = index.at(i);
        QVector<double> coeff = fitParam.at(id);
        int order = fitOrder.at(id);
        int ns = allData.at(id).scanData.size();
        int np = allData.at(id).xvalues.size();
        double lx = allData.at(id).xvalues.at(idMin.at(ccw_id));
        double hx = allData.at(id).xvalues.at(idMax.at(ccw_id));
        double dr = hx - lx;
        for (int j = 0; j < ns; j++){
            double yf = 0;
            for (int k = 0; k < order; k++)
                yf += coeff.at(k) * qPow(j, k);
            double dA = yf - allIntegrals.at(id).at(j);
            double shift = dA / dr;
            const double *rp = allData.at(id).scanData.at(j).rvalues.data();
            double *rcp = allDataC[id].scanData[j].rvalues.data();
            for (int k = 0; k < np; k++)
                rcp[k] = rp[k] + shift;
        }
    }
    integrate(CDATA_S);
}

void US_RemoveRI::pb_save_avail(){
    bool state = true;
    for (int i = 0; i < fitState.size(); i++)
        state = state && fitState.at(i);
    if (state)
        pb_save->setEnabled(true);
    else
        pb_save->setDisabled(true);
}

void US_RemoveRI::pb_fit_avail(){
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (! ccwIntgState.at(ccw_id)){
        pb_fit->setStyleSheet(qs.arg(color.name()));
        pb_fit->setDisabled(true);
        return;
    }
    pb_fit->setEnabled(true);
    if (ccwFitState.at(ccw_id))
        pb_fit->setStyleSheet(qs.arg("green"));
    else
        pb_fit->setStyleSheet(qs.arg("yellow"));
}

void US_RemoveRI::slt_rm_fit(double){
    clean_states(FIT_S);
    ccwFitState[ccw_id] = false;
    pb_fit_avail();
    pb_save_avail();
    le_fitrsqrd->setText("");
    le_fitorder->setText("");
    emit sig_plot(true);
}
