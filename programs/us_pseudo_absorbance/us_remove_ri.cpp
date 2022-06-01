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
    pb_calc_res = us_pushbutton("Compute Residuals");
    QHBoxLayout *xrange_lyt = new QHBoxLayout();
    xrange_lyt->addWidget(pb_pick_rp);
    xrange_lyt->addWidget(le_xrange);
    xrange_lyt->addWidget(pb_calc_res);

    cb_autofit = new QCheckBox();
    pb_fit = us_pushbutton("Fit Polynomial");
    QGridLayout* uscb_autofit = us_checkbox("Auto Fit", cb_autofit, true);
    QHBoxLayout *fit_lyt = new QHBoxLayout();
    fit_lyt->addLayout(uscb_autofit);
    fit_lyt->addWidget(pb_fit);

    lb_maxorder = us_label("Max Order:");
    lb_maxorder->setAlignment(Qt::AlignRight);
    ct_max_order = us_counter(1, 1, max_ct, 10);
    lb_manorder = us_label("Order:");
    lb_manorder->setAlignment(Qt::AlignRight);
    ct_order = us_counter(1, 1, max_ct, 6);
    lb_fitorder = us_label("Order:");
    lb_fitorder->setAlignment(Qt::AlignRight);
    le_fitorder = us_lineedit("", 0, true);
    lb_fiterror = us_label("Error:");
    lb_fiterror->setAlignment(Qt::AlignRight);
    le_fiterror = us_lineedit("", 0, true);

    QHBoxLayout *fitctrl_lyt = new QHBoxLayout();
    fitctrl_lyt->addWidget(lb_maxorder);
    fitctrl_lyt->addWidget(ct_max_order);
    fitctrl_lyt->addWidget(lb_manorder);
    fitctrl_lyt->addWidget(ct_order);
    fitctrl_lyt->addWidget(lb_fitorder);
    fitctrl_lyt->addWidget(le_fitorder);
    fitctrl_lyt->addWidget(lb_fiterror);
    fitctrl_lyt->addWidget(le_fiterror);
//    le_fitorder->setMinimumWidth(20);
//    le_fiterror->setMinimumWidth(20);
    le_fitorder->setMaximumWidth(25);
    le_fiterror->setMaximumWidth(75);
    lb_manorder->hide();
    ct_order->hide();

    QLabel* lb_save = us_banner("Save Control");
    QLabel* lb_runIdOut  = us_label(tr("Output Run ID:"));
    le_runIdOut = us_lineedit("", 0, false);
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
    left_lyt->addLayout(fit_lyt);
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
                                true, "", "rainbow" );
    qwtplot_data->setMinimumSize( 650, 300 );
    qwtplot_data->enableAxis( QwtPlot::xBottom, true );
    qwtplot_data->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_data->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_data);

    usplot_fit = new US_Plot( qwtplot_fit, tr( "" ),
                              tr( "Radius (in cm)" ), tr( "Absorbance" ),
                              true, "", "rainbow" );
    qwtplot_fit->setMinimumSize( 650, 300 );
    qwtplot_fit->enableAxis( QwtPlot::xBottom, true );
    qwtplot_fit->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_fit->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_fit);


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

    connect(this, SIGNAL(sig_plot_data(bool)), this, SLOT(slt_plot_data(bool)));
    connect(pb_pick_rp, SIGNAL(clicked()),
                this, SLOT(slt_pick_point()));

    connect(cb_autofit, SIGNAL(stateChanged(int)), this, SLOT(slt_autofit(int)));
//    connect(ckb_xrange, SIGNAL(stateChanged(int)), this, SLOT(slt_xrange(int)));

//    connect(pb_reset_curr_scans, SIGNAL(clicked()), this, SLOT(slt_reset_scans()));
//    connect(pb_reset_allscans, SIGNAL(clicked()), this, SLOT(slt_reset_allscans()));
//    connect(pb_apply_allscans, SIGNAL(clicked()), this, SLOT(slt_apply_allscans()));
//    connect(pb_reset_refData, SIGNAL(clicked()), this, SLOT(slt_reset_refData()));
//    connect(this, SIGNAL(sig_save_button()), this, SLOT(slt_save_avail()));

//    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));

//    connect(cb_buffer, SIGNAL(currentIndexChanged(int)),
//            this, SLOT(slt_update_buffer(int)));

//    connect(ct_smooth, SIGNAL(valueChanged(double)),
//            this, SLOT(slt_update_smooth(double)));
//
//    connect(le_runIdAbs, SIGNAL(textEdited(QString)), this, SLOT(slt_edit_le(QString)));


}


void US_RemoveRI::slt_reset(){
    picker->disconnect();
    hasData = false;
    allData.clear();
    allDataC.clear();
    allResiduals.clear();
    fitOrder.clear();
    fitError.clear();
    fitParam.clear();
    ccwList.clear();
    ccwStrList.clear();
    ccwItemList.clear();
//    le_cursor_pos = -1;
    wavelength.clear();
    pmin.clear();
    pmax.clear();
    le_runIdIn->clear();
    le_runIdOut->clear();
    le_lambstrt->clear();
    le_lambstop->clear();
    le_dir->clear();
    le_desc->clear();
    le_xrange->clear();
    pb_reset->setDisabled(true);
    pb_import->setEnabled(true);
    pb_save->setDisabled(true);
    emit sig_plot_data(false);
}

void US_RemoveRI::slt_import(void){
    slt_reset();

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
        QVector<double> res(rdata.scanCount(), -1e20);
        allResiduals << res;

        QVector<double> param(max_ct, 0);
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
    fitError.fill(0, allData.size());

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
    le_dir->setText(US_Settings::importDir());
    runIdOut = runId.prepend("RI_");
    le_runIdOut->setText(runIdOut);
    set_cb_triples();
    slt_new_ccw(0);
    pb_import->setDisabled(true);
    pb_reset->setEnabled(true);
    emit sig_save_button();
    hasData = true;
    le_status->clear();
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

void US_RemoveRI::offon_prev_next(){
    pb_prev_id->setDisabled(wavl_id <= 0);
    pb_next_id->setDisabled(wavl_id >= (n_wavls - 1));
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
    n_wavls = ccwItemList.n_wl.at(ccw_id);
    intDataId = ccwItemList.index.at(ccw_id);
    wavelength = ccwItemList.wavelength.at(ccw_id);
    le_lambstrt->setText(QString::number(wavelength.at(0)));
    le_lambstop->setText(QString::number(wavelength.at(n_wavls - 1)));
    QStringList items;
    for (int i = 0; i < n_wavls; ++i)
        items << QString::number(wavelength.at(i));
    cb_plot_id->addItems(items);
    offon_prev_next();
    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_id(int)));

    pb_prev_ccw->setDisabled(ccw_id <= 0);
    pb_next_ccw->setDisabled(ccw_id >= (n_ccw - 1));

    QString qs = "QPushButton { background-color: %1 }";
    if (pmin.at(ccw_id) < 0 || pmax.at(ccw_id) < 0){
        pmin[ccw_id] = -1;
        pmax[ccw_id] = -1;
        le_xrange->setText("");
        pb_pick_rp->setStyleSheet(qs.arg("yellow"));
    } else {
        QString str = tr("%1 - %2 cm");
        le_xrange->setText(str.arg(pmin.at(ccw_id), 0, 'f', 3).
                           arg(pmax.at(ccw_id), 0, 'f', 3));
        pb_pick_rp->setStyleSheet(qs.arg("green"));
    }

    slt_set_id(0);
}

void US_RemoveRI::slt_set_id(int id){
    wavl_id = id;
    offon_prev_next();
    le_desc->setText(allData.at(id).description);
    emit sig_plot_data(true);
}

void US_RemoveRI::slt_pick_point(){
    picker->disconnect();
    pmin[ccw_id] = -1;
    pmax[ccw_id] = -1;
//    x_min_picked = -1;
//    x_max_picked = -1;
    le_xrange->setText("");
    if (allData.size() == 0)
        return;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    emit sig_plot_data(true);
    emit sig_save_button();
    connect(picker, SIGNAL(cMouseUp(const QwtDoublePoint&)),
            this,   SLOT(slt_mouse(const QwtDoublePoint&)));
    return;
}

void US_RemoveRI::slt_mouse(const QwtDoublePoint& point){
    double x = point.x();
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
    US_DataIO::RawData data = allDataC.at(index);
    int np = data.pointCount();
    double min_x = data.xvalues.at(0);
    double max_x = data.xvalues.at(np - 1);
    QString str;
    if (x > min_x && x < max_x){
        if (pmin.at(ccw_id) == -1){
            pmin[ccw_id] = x;
            str = tr("%1 -");
            le_xrange->setText(str.arg(x, 0, 'f', 3));
            emit sig_plot_data(true);
        } else {
            if (x <= pmax.at(ccw_id)){
                QString mess("Pick a radial point greater than: %1 cm");
                QMessageBox::warning( this, tr( "Warning" ), mess.arg(x));
                return;
            }
            pmax[ccw_id] = x;
            picker->disconnect();
            str = tr("%1 - %2 cm");
            le_xrange->setText(str.arg(pmin.at(ccw_id), 0, 'f', 3).
                               arg(pmax.at(ccw_id), 0, 'f', 3));
            pb_pick_rp->setStyleSheet("QPushButton { background-color: green }");
            emit sig_plot_data(true);
        }
    }else{
        QString mess("Pick a point between the minimum and maximum"
                     "values of the radial points.\n"
                     "Minimum= %1 cm, Maximum= %2 cm");
        QMessageBox::warning( this, tr( "Warning" ), mess.arg(min_x).arg(max_x));
    }
    emit sig_save_button();
    return;
}

//void US_RemoveRI::slt_save_avail(void){
//    if (lw_triple->count() > 0 && refData.nWavelength > 0){
//        if (ckb_xrange->isChecked()){
//            if (x_min_picked != -1 && x_max_picked != -1)
//                pb_save->setEnabled(true);
//            else
//                pb_save->setDisabled(true);
//        }else
//            pb_save->setEnabled(true);
//    }else
//        pb_save->setDisabled(true);
//    return;
//}

//void US_RemoveRI::slt_save(void){
//    QDir dir = QDir(le_dir->text());
//    if (dir.cd(runIdAbs)){
//        QString absPath = dir.absolutePath();
//        int ck = QMessageBox::question(this, tr( "Warning!" ),
//                                       tr( "Output directory exists!\n" ) + runIdAbs +
//                                       tr( "\n\n Do you really want to replace it?"));
//        if (ck == QMessageBox::Yes){
//            dir.removeRecursively();
//            dir.mkpath(absPath);
//            dir.setPath(absPath);
//        }else return;
//    }else{
//        dir.mkdir(runIdAbs);
//        dir.cd(runIdAbs);
//    }
//    dir.makeAbsolute();
//    qDebug() << dir.path();
//    int nwl_tot = 0;
//    int nrows = lw_triple->count();
//    for (int i = 0; i < nrows; ++i)
//        nwl_tot += ccwItemList.n_wl.at(i);

//    int id_buff = cb_buffer->currentIndex();
//    bool rm_buffer = false;
//    if (id_buff > 0)
//        rm_buffer = true;

//    int n = 1;
//    QString status = tr("writting: %1 %2");
//    QString percent;
//    QString fileName("%1.RA.%2.%3.%4.auc");
//    for (int i = 0; i < nrows; ++i){
//        for (int j = 0; j < ccwItemList.n_wl.at(i); ++j){
//            percent = QString::number(100.0 * n / nwl_tot, 'f', 1);
//            le_status->setText(status.arg(percent).arg(QChar(37)));
//            qApp->processEvents();
//            n++;
//            int dataId = ccwItemList.index.at(i).at(j);
//            double wavelength = ccwItemList.wavelength.at(i).at(j);
//            if (! get_refId(wavelength)){
//                qDebug() << tr("Not found corrosponding reference data for: ") <<
//                            allIntDataFiles.at(dataId).fileName();
//                continue;
//            }

//            get_absorbance(refId, dataId, false);
//            if (rm_buffer){
//                int dataId_buff = ccwItemList.index.at(id_buff - 1).at(j);
//                get_relative_absorbance(dataId_buff);
//            }
//            trim_absorbance();
//            int scan_l1 = scansRange.at(dataId).at(0);
//            int scan_l2 = scansRange.at(dataId).at(1);
//            QVector<US_DataIO::Scan> absorbance_sel;
//            for (int k = scan_l1; k <scan_l2; ++k)
//                absorbance_sel << absorbance.at(k);
//            if (absorbance_sel.size() == 0)
//                continue;

//            US_DataIO::RawData rawData = allIntData.at(dataId);
//            rawData.type[0] = 'R';
//            rawData.type[1] = 'A';
//            uchar uuid[ 16 ];
//            QString uuid_string = US_Util::new_guid();
//            US_Util::uuid_parse( uuid_string, uuid );
//            memcpy( rawData.rawGUID,   (char*) uuid, 16 );
//            rawData.scanData.clear();
//            rawData.scanData << absorbance_sel;
//            int cell = rawData.cell;
//            char channel = rawData.channel;
//            QString fn = fileName.arg(runIdAbs).arg(cell).arg(channel).arg(wavelength);
//            QFileInfo fileInfo(dir, fn);
//            US_DataIO::writeRawData(fileInfo.absoluteFilePath(), rawData);
//        }
//    }
//    le_status->setText("written on the local disk !");

//    return;
//}

//void US_RemoveRI::slt_edit_le(QString text){
//    if (text.size() < runIdAbs.size()){
//        runIdAbs = text;
//    }else{
//        QRegExp re( "[^a-zA-Z0-9_-]" );
//        int reIdx = text.indexOf(re, 0);
//        if (reIdx >= 0){
//            le_runIdAbs->setText(runIdAbs);
//            le_runIdAbs->setCursorPosition(reIdx);
//        }else
//            runIdAbs = text;
//    }
//    return;
//}

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
            int n_wl = wl_list.size();
            ccwItemList.index << index;
            ccwItemList.min_wl << wl_list.at(0);
            ccwItemList.max_wl << wl_list.at(n_wl - 1);
            ccwItemList.wavelength << wl_list;
            ccwItemList.n_wl << n_wl;
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
        double min_wl = ccwItemList.min_wl.at(i);
        double max_wl = ccwItemList.max_wl.at(i);
        int nwl = ccwItemList.n_wl.at(i);
        QString item_i = item.arg(cell).arg(channel)
                .arg(min_wl).arg(max_wl).arg(nwl);
        cb_triples->addItem(item_i);
    }
    ccw_id = 0;
    pmin.fill(-1, n_ccw);
    pmax.fill(-1, n_ccw);
    connect( cb_triples, SIGNAL( currentIndexChanged(int) ),
            this, SLOT( slt_new_ccw(int) ) );
    cb_triples->setCurrentIndex(ccw_id);
}


void US_RemoveRI::slt_plot_data(bool state){
    qwtplot_data->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        grid = us_grid(qwtplot_data);
        qwtplot_data->replot();
        return;
    }

//    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
//    QVector<double> xvalues = allIntData.at(index).xvalues;
    US_DataIO::RawData data = allDataC.at(index);

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

    if (pmin.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << pmin.at(ccw_id);
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_data,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    if (pmax.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << pmax.at(ccw_id);
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
    QString title = tr("wavelength= % nm");
    qwtplot_data->setTitle(title.arg(QString::number(data.scanData.at(0).wavelength)));
    qwtplot_data->replot();
    return;
}

void US_RemoveRI::slt_plot_fit(bool state){
    qwtplot_fit->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        grid = us_grid(qwtplot_fit);
        qwtplot_fit->replot();
        return;
    }

//    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(ccw_id).at(wavl_id);
//    QVector<double> xvalues = allIntData.at(index).xvalues;
    QVector<double> residual = allResiduals.at(index);
    US_DataIO::RawData data = allDataC.at(index);

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
        QwtPlotCurve* curve = us_curve( qwtplot_fit,"");
        if (error == 0)
            pen_plot.setColor(colorList[ i % nc ]);
        curve->setPen( pen_plot );
        curve->setSamples(x, r, np);
    }
    double dx = (max_x - min_x) * 0.05;
    double dr = (max_r - min_r) * 0.05;

    if (pmin.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << pmin.at(ccw_id);
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_fit,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    if (pmax.at(ccw_id) != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << pmax.at(ccw_id);
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_fit,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    grid = us_grid(qwtplot_fit);

    qwtplot_fit->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    qwtplot_fit->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_fit->updateAxes();
    QString title = tr("wavelength= % nm");
    qwtplot_fit->setTitle(title.arg(QString::number(data.scanData.at(0).wavelength)));
    qwtplot_fit->replot();
    return;
}



void US_RemoveRI::slt_autofit(int state){
    if (state == Qt::Checked){
        lb_maxorder->show();
        ct_max_order->show();
        lb_manorder->hide();
        ct_order->hide();
        lb_fitorder->show();
        le_fitorder->show();
        lb_fiterror->show();
        le_fiterror->show();
    } else {
        lb_maxorder->hide();
        ct_max_order->hide();
        lb_manorder->show();
        ct_order->show();
        lb_fitorder->hide();
        le_fitorder->hide();
        lb_fiterror->show();
        le_fiterror->show();
    }
}

void US_RemoveRI::slt_residual(void){
    QVector<int> index = ccwItemList.index.at(ccw_id);
    double xmin = pmin.at(ccw_id);
    double xmax = pmax.at(ccw_id);
    for (int i = 0; i < index.size(); i++){
        int id = index.at(i);
        US_DataIO::RawData data = allData.at(id);
        const double *x = data.xvalues.data();
        double *res = allResiduals[id].data();
        for (int j = 0; j < data.scanCount(); j++){
            const double *y = data.scanData.at(j).rvalues.data();
            double sum = trapz(x, y, xmin, xmax);
            res[j] = sum;
        }
    }
    emit sig_plot_fit(true);

}

double US_RemoveRI::trapz(const double* x, const double* y,
                          double xmin, double xmax){
    int n = 0;
    while (x[n] < xmin)
        n++;
    double dx;
    double sum = 0;
    while (x[++n] <= xmax){
        dx = x[n] - x[n - 1];
        sum += dx * ( y[n] + y[n - 1] ) / 2.0;
    }
    return sum;
}
