#include "us_convert_scan.h"


US_ConvertScan::US_ConvertScan() : US_Widgets()
{
    dbCon = new US_DB2();
    setPalette( US_GuiSettings::frameColorDefault() );

    // Put the Run Info across the entire window
    QLabel* lb_runInfoInt  = us_banner(   tr( "Intensity Data Information" ) );
    pb_import = us_pushbutton(tr("Import Experimental Data"));
    pb_reset = us_pushbutton("Reset", false, 0 );
    pb_reset->setDisabled(true);
    QHBoxLayout* import_lyt = new QHBoxLayout();
    import_lyt->addWidget(pb_import);
    import_lyt->addWidget(pb_reset);

    QLabel* lb_runIdInt  = us_label(      tr( "Run ID:" ) );
    le_runIdInt          = us_lineedit(   "", 0, true );
    QLabel* lb_dirInt      = us_label(    tr( "Directory:" ) );
    le_dir              = us_lineedit( "", 0, true );
    QLabel* lb_desc  = us_label(    tr( "Description:" ), 0 );
    le_desc      = us_lineedit( "", 0, true);

    QGridLayout* insty_info_lyt = new QGridLayout();
    insty_info_lyt->addWidget(lb_dirInt,   0, 0, 1, 1);
    insty_info_lyt->addWidget(le_dir,      0, 1, 1, 1);
    insty_info_lyt->addWidget(lb_runIdInt, 1, 0, 1, 1);
    insty_info_lyt->addWidget(le_runIdInt, 1, 1, 1, 1);
    insty_info_lyt->addWidget(lb_desc,     2, 0, 1, 1);
    insty_info_lyt->addWidget(le_desc,     2, 1, 1, 1);

    // Multi-Wavelength Lambda Controls
    static QChar clambda( 955 );   // Lambda character
    QLabel* lb_mwlctrl   = us_banner  ( tr( "Multi-Wavelength Lambda Controls" ) );
    QLabel* lb_lambstrt  = us_label   ( tr( "%1 Start:"    ).arg( clambda ) );
    QLabel* lb_lambstop  = us_label   ( tr( "%1 End:"      ).arg( clambda ) );
    QLabel* lb_lambplot  = us_label   ( tr( "Plot %1:"     ).arg( clambda ) );
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

    // Cell / Channel / Wavelength
    QLabel* lb_triple = us_banner(tr( "Cell / Channel / Wavelength" ), -1 );
    lw_triple = us_listwidget();
    QLabel* lb_ccw_info = us_label(tr("Manage Triples"));
    lb_ccw_info->setAlignment(Qt::AlignCenter);
    QPushButton* pb_del_ccw_item = us_pushbutton(tr("Delete Selected CCW"));
    QPushButton* pb_default_ccw = us_pushbutton(tr("Default"));
    QVBoxLayout* ccw_vbl = new QVBoxLayout();
    ccw_vbl->addWidget(lb_ccw_info);
    ccw_vbl->addWidget(pb_del_ccw_item);
    ccw_vbl->addWidget(pb_default_ccw);
    ccw_vbl->addStretch(1);
    QHBoxLayout* ccw_hbl = new QHBoxLayout();
    ccw_hbl->addWidget(lw_triple);
    ccw_hbl->addLayout(ccw_vbl);

    QLabel* lb_refScan  = us_banner(   tr( "Reference Scan Control" ) );
    ckb_channelBlanking = new QCheckBox();
    QGridLayout *chbl_lyt = us_checkbox("Blank by Buffer Channel", ckb_channelBlanking);

    frm_refScan = new QFrame();
    frm_refScan->setContentsMargins(0, 0, 0, 0);
    diskDB_ctrl = new US_Disk_DB_Controls();
    pb_import_refScans = us_pushbutton(tr("Import"), true, 0);
    pb_reset_refData = us_pushbutton(tr("Reset"), false, 0);
    QString ref_range("%1 Range:");
    QLabel* lb_ref_range = us_label(ref_range.arg(QChar(955)));
    lb_ref_range->setMaximumWidth(80);
    le_ref_range = us_lineedit("");
    le_ref_range->setMaximumWidth(140);
    le_ref_range->setReadOnly(true);
    QHBoxLayout* rfs_imp_lyt = new QHBoxLayout();
    rfs_imp_lyt->addWidget(pb_import_refScans);
    rfs_imp_lyt->addWidget(pb_reset_refData);
    rfs_imp_lyt->addWidget(lb_ref_range);
    rfs_imp_lyt->addWidget(le_ref_range);

    ckb_ChroAberCorr = new QCheckBox();
    QGridLayout *cac_lyt = us_checkbox("Chromatic Aberration Correction", ckb_ChroAberCorr);
    QFont font = ckb_ChroAberCorr->font();
    font.setPointSize(font.pointSize() - 1);
    ckb_ChroAberCorr->setFont(font);

    QVBoxLayout* rfs_lyt = new QVBoxLayout();
    rfs_lyt->addLayout(diskDB_ctrl);
    rfs_lyt->addLayout(rfs_imp_lyt);
    rfs_lyt->addLayout(cac_lyt);
    rfs_lyt->setContentsMargins(0, 0, 0, 0);
    rfs_lyt->setSpacing(1);
    frm_refScan->setLayout(rfs_lyt);

    QLabel* lb_bufferBanner  = us_banner(   tr( "Buffer Control" ) );
    QLabel *lb_buffer = us_label("Buffer Triple:");
    lb_buffer->setMaximumWidth(108);
    cb_buffer = us_comboBox();
    QLabel *lb_smooth = us_label("Smooth Level:");
    lb_smooth->setMaximumWidth(120);
    ct_smooth = us_counter(1, 0, 10, 5);
    ct_smooth->setSingleStep(1);
    ct_smooth->setMaximumWidth(70);

    QHBoxLayout *buffer_lyt = new QHBoxLayout();
    buffer_lyt->addWidget(lb_buffer);
    buffer_lyt->addWidget(cb_buffer);
    buffer_lyt->addWidget(lb_smooth);
    buffer_lyt->addWidget(ct_smooth);

    QLabel* lb_runInfoAbs  = us_banner(   tr( "Absorbance Data Information" ) );
    QLabel* lb_runIdAbs  = us_label(      tr( "Run ID:" ) );
//    le_runIdAbs          = us_lineedit(   "", 0, false );
    le_runIdAbs = new US_LineEdit_RE("", 0, false );

    QGridLayout* abs_info_lyt = new QGridLayout();
    abs_info_lyt->addWidget(lb_runIdAbs, 0, 0, 1, 1);
    abs_info_lyt->addWidget(le_runIdAbs, 0, 1, 1, 1);

    ckb_zeroing = new QCheckBox();
    QGridLayout *us_zeroing = us_checkbox("Shift to Zero",
                                                ckb_zeroing);
    ckb_xrange = new QCheckBox();
    QGridLayout *us_xrange = us_checkbox("Limit Radius",
                                                ckb_xrange);
    pb_pick_rp = us_pushbutton("Pick Two Radial Points", false);
    QHBoxLayout *ckb_lyt = new QHBoxLayout();
    ckb_lyt->addLayout(us_zeroing);
    ckb_lyt->addLayout(us_xrange);
    ckb_lyt->addWidget(pb_pick_rp);

    QLabel *lb_r_rng = us_label("Radius Range:");
    le_xrange = us_lineedit("", 0, true);
    QHBoxLayout *xrange_lyt = new QHBoxLayout();
    xrange_lyt->addWidget(lb_r_rng);
    xrange_lyt->addWidget(le_xrange);

    QLabel* lb_status = us_label(tr("Status:"));
    le_status = us_lineedit(tr(""), -1, true);
    QPalette stpal;
    stpal.setColor( QPalette::Text, Qt::white );
    stpal.setColor( QPalette::Base, Qt::blue  );
    le_status->setPalette( stpal );
    QHBoxLayout* status_lyt = new QHBoxLayout();
    status_lyt->addWidget(lb_status);
    status_lyt->addWidget(le_status);

    pb_save = us_pushbutton("Save Absorbance Data", false, 0 );
    pb_save->setDisabled(true);

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
    left_lyt->addLayout(insty_info_lyt);
    left_lyt->addWidget(lb_triple);
    left_lyt->addLayout(ccw_hbl);
    left_lyt->addWidget(lb_mwlctrl);
    left_lyt->addLayout(wvl_rng_lyt);
    left_lyt->addLayout(wvl_plt_lyt);
    left_lyt->addWidget(lb_refScan);
    left_lyt->addLayout(chbl_lyt);
    left_lyt->addWidget(frm_refScan);
    left_lyt->addWidget(lb_bufferBanner);
    left_lyt->addLayout(buffer_lyt);
    left_lyt->addWidget(lb_runInfoAbs);
    left_lyt->addLayout(ckb_lyt);
    left_lyt->addLayout(xrange_lyt);
    left_lyt->addLayout(abs_info_lyt);
    left_lyt->addWidget(pb_save);
    left_lyt->addStretch(1);
    left_lyt->addLayout(status_lyt);
    left_lyt->addLayout(close_lyt);
    left_lyt->addStretch(0);

//    //*****right*****//
//    plot_title = us_label("");

    plot_title = us_label("");
    plot_title->setAlignment(Qt::AlignCenter);
    plot_title->setStyleSheet("background-color: white;"
                              "color:black; font-size: 11pt");

    QwtText xLabel, yLabel;
    usplot_insty = new US_Plot( qwtplot_insty, tr( "" ),
                                tr( "Radius (in cm)" ), tr( "Intensity" ),
                                true, "", "rainbow" );
    qwtplot_insty->setMinimumSize( 650, 300 );
    qwtplot_insty->enableAxis( QwtPlot::xBottom, true );
    qwtplot_insty->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_insty->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_insty);

    usplot_abs = new US_Plot( qwtplot_abs, tr( "" ),
                              tr( "Radius (in cm)" ), tr( "Absorbance" ),
                              true, "", "rainbow" );
    qwtplot_abs->setMinimumSize( 650, 300 );
    qwtplot_abs->enableAxis( QwtPlot::xBottom, true );
    qwtplot_abs->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_abs->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_abs);

    ct_scan_l = us_counter(3, 0, 0, 0);
    ct_scan_l->setSingleStep(1);
    ct_scan_u = us_counter(3, 0, 0, 0);
    ct_scan_u->setSingleStep(1);
    QLabel *lb_scan_ctrl = us_banner("Scans Control");
    QLabel *lb_scan_1 = us_label("Scan Focus From:");
    lb_scan_1->setAlignment(Qt::AlignRight);
    QLabel *lb_scan_2 = us_label("To:");
    lb_scan_2->setAlignment(Qt::AlignRight);
    QPushButton *pb_reset_curr_scans = us_pushbutton("Reset Current", true, 0);
    QPushButton *pb_reset_allscans = us_pushbutton("Reset All", true, 0);
    QPushButton *pb_apply_allscans = us_pushbutton("Apply All", true, 0);

    QGridLayout *scan_ctrl_lyt = new QGridLayout();
    scan_ctrl_lyt->addWidget(lb_scan_ctrl,        0, 0, 1, 4);
    scan_ctrl_lyt->addWidget(lb_scan_1,           1, 0, 1, 1);
    scan_ctrl_lyt->addWidget(ct_scan_l,           1, 1, 1, 1);
    scan_ctrl_lyt->addWidget(pb_reset_curr_scans, 1, 2, 1, 2);
    scan_ctrl_lyt->addWidget(lb_scan_2,           2, 0, 1, 1);
    scan_ctrl_lyt->addWidget(ct_scan_u,           2, 1, 1, 1);
    scan_ctrl_lyt->addWidget(pb_reset_allscans,   2, 2, 1, 1);
    scan_ctrl_lyt->addWidget(pb_apply_allscans,   2, 3, 1, 1);

    QVBoxLayout* right_lyt = new QVBoxLayout();
    right_lyt->setSpacing(0);
    right_lyt->addWidget(plot_title);
    right_lyt->addLayout(usplot_insty);
    right_lyt->addLayout(usplot_abs);
//    right_lyt->addStretch(1);
    right_lyt->addLayout(scan_ctrl_lyt);

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    left_lyt->setSizeConstraint(QLayout::SetMinimumSize);
    main_lyt->addLayout(left_lyt, 0);
//    main_lyt->addStretch(1);
    main_lyt->addLayout(right_lyt, 1);
    main_lyt->setSpacing(1);
    main_lyt->setMargin(1);
    setLayout(main_lyt);

    picker_abs = new US_PlotPicker(qwtplot_abs);
    picker_abs->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker_abs->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );
    picker_insty = new US_PlotPicker(qwtplot_insty);

    slt_reset();

    connect(pb_import, SIGNAL(clicked()), this, SLOT(slt_import()));
    connect(pb_default_ccw, SIGNAL(clicked()), this, SLOT(slt_set_ccw_default()));
    connect(pb_del_ccw_item, SIGNAL(clicked()),
            this, SLOT(slt_del_item()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_prev_id,      SIGNAL(clicked()), this, SLOT(slt_prev_id()));
    connect(pb_next_id,      SIGNAL(clicked()), this, SLOT(slt_next_id()));

    connect(this, SIGNAL(sig_plot()), this, SLOT(slt_plot()));

    connect(ckb_channelBlanking, SIGNAL(stateChanged(int)),
            this, SLOT(slt_channelBlanking(int)));
    connect(pb_import_refScans, SIGNAL(clicked()), this, SLOT(slt_load_refScans()));
    connect(ckb_zeroing, SIGNAL(stateChanged(int)), this, SLOT(slt_zeroing(int)));
    connect(ckb_xrange, SIGNAL(stateChanged(int)), this, SLOT(slt_xrange(int)));

    connect(pb_reset_curr_scans, SIGNAL(clicked()), this, SLOT(slt_reset_scans()));
    connect(pb_reset_allscans, SIGNAL(clicked()), this, SLOT(slt_reset_allscans()));
    connect(pb_apply_allscans, SIGNAL(clicked()), this, SLOT(slt_apply_allscans()));
    connect(pb_reset_refData, SIGNAL(clicked()), this, SLOT(slt_reset_refData()));
    connect(this, SIGNAL(sig_save_button()), this, SLOT(slt_save_avail()));

    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));

    connect(cb_buffer, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slt_update_buffer(int)));

    connect(ct_smooth, SIGNAL(valueChanged(double)),
            this, SLOT(slt_update_smooth(double)));
    connect(pb_pick_rp, SIGNAL(clicked()),
            this, SLOT(slt_pick_point()));
}

void US_ConvertScan::slt_reset(){
    picker_abs->disconnect();
    hasData = false;
    n_scans = -1;
    intRunIds.clear();
    intDataId.clear();
    xvalues.clear();
    intensity.clear();
    absorbance.clear();
    absorbanceBuffer.clear();
    allIntData.clear();
    allIntDataFiles.clear();
    ccwList.clear();
    ccwListMain.clear();
    ccwStrListMain.clear();
    ccwItemList.clear();
    scansRange.clear();
    le_cursor_pos = -1;
    wavelength.clear();
    refData.clear();
    xvaluesRefCAC.clear();
    x_min_picked = -1;
    x_max_picked = -1;
    runIdAbs.clear();
    le_ref_range->clear();
    le_lambstrt->clear();
    le_lambstop->clear();
    le_runIdAbs->clear();
    le_runIdInt->clear();
    le_dir->clear();
    le_desc->clear();
    le_xrange->clear();
    set_listWidget();
    set_wavl_ctrl();
    pb_reset->setDisabled(true);
    pb_import->setEnabled(true);
    pb_import_refScans->setEnabled(true);
    pb_reset_refData->setDisabled(true);
    pb_save->setDisabled(true);
    ckb_zeroing->setCheckState(Qt::Unchecked);
    uncheck_CA_silently();
    return;
}

void US_ConvertScan::slt_import(void){
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
    QStringList runTypesList;
    runTypesList << "RI";
    QRegExp re( "[^A-Za-z0-9_-]" );
    bool runId_changed = false;
    bool multiple_runid = false;
    QString ccw_str("%1 / %2 / %3");
    bool truncate = false;
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
        if (runId.isEmpty()) {
            runId = rid;
        } else {
            if (runId != rid) multiple_runid = true;
        }
        if (! runTypesList.contains(rtp)){
            QMessageBox::warning( this, tr( "Error" ),
                  tr( "The Run type (%1) is not supported." ).arg(rtp));
            return;
        }
        if (runType.isEmpty()) runType = rtp;
        else {
            if (runType != rtp){
                QMessageBox::warning( this, tr( "Error" ),
                                     tr( "Multiple run types found in the directory" ));
                return;
            }
        }
        if (intRunIds.contains(rid)) {
            QMessageBox::warning( this, tr( "Error" ),
                                 tr( "The RunID Already Loaded!\n(%1)" ).arg(rid));
            return;
        }

        US_DataIO::RawData rdata = US_DataIO::RawData();
        US_DataIO::readRawData(fileList.at(i).absoluteFilePath(), rdata);
        QVector<int> scl;
        int ns = rdata.scanCount();
        if (n_scans == -1) n_scans = ns;
        if (ns < n_scans) {
            QMessageBox::warning(this, "Error!", tr("The number of scans in the following run (%1) "
                                                    "is less than the previously loaded one(s) (%2).\n\n%3")
                                                    .arg(ns).arg(n_scans).arg(fileList.at(i).fileName()));
            return;
        } else if (ns > n_scans && !truncate ) {
            int chk = QMessageBox::question(this, "Warning!",
                                            tr("The number of scans in the following runID (%1) "
                                               "is greater than the previously loaded one(s) (%2)."
                                               "Do you want to proceed with truncating the last (%2) scans?\n\n%3")
                                               .arg(ns).arg(n_scans).arg(rid));
            if (chk == QMessageBox::No) return;
            truncate = true;
        }
        if (truncate) {
            QVector<US_DataIO::Scan> scans;
            scans << rdata.scanData;
            rdata.scanData.clear();
            rdata.scanData << scans.mid(ns - n_scans);
        }
        scl << 0 << n_scans << n_scans;
        scansRange << scl;
        allIntData << rdata;

        allIntDataFiles << fileList.at(i);

        int cell = rdata.cell;
        char channel = rdata.channel;
        double wl = rdata.scanData.at(0).wavelength;
        ccwStrListMain << ccw_str.arg(cell).arg(channel).arg(wl);

        ccwListMain.index << allIntData.size() - 1;
        ccwListMain.cell << cell;
        ccwListMain.channel << channel;
        ccwListMain.wavelength << wl;
        ccwListMain.runId << rid;

        ccwList.index << allIntData.size() - 1;
        ccwList.cell << cell;
        ccwList.channel << channel;
        ccwList.wavelength << wl;
        ccwList.runId << rid;
    }

    if ( runId_changed ) {
       QMessageBox::warning( this, tr( "Warning!" ),
             tr( "The RunID changed. It can only be alphanumeric,"
                    "underscore, and hyphen characters."));
    }
    if ( multiple_runid ) {
        QMessageBox::warning( this, tr( "Warning!" ),
                             tr( "Multiple RunIDs Found!"));
    }

    intRunIds.clear();
    for (int ii = 0; ii < ccwListMain.size(); ii++){
        QString rundId = ccwListMain.runId.at(ii);
        if (! intRunIds.contains(rundId)) {
            intRunIds << rundId;
        }
    }

    // le_runIdInt->setText(runId);
    le_dir->setText(US_Settings::importDir());
//    re.setPattern("-run[0-9]+$");
//    int reIdx = runId.indexOf(re);
//    QString s1 = runId.left(reIdx);
//    QString s2 = runId.right(runId.size() - reIdx);
//    runIdAbs = s1.append("_Absorbance").append(s2);
    runIdAbs = runId.prepend("P-ABS_");
    le_runIdAbs->setText(runIdAbs);
    make_ccwItemList();
    set_listWidget();
    set_wavl_ctrl();
    // pb_import->setDisabled(true);
    pb_reset->setEnabled(true);
    emit sig_save_button();
    hasData = true;
    le_status->clear();
    return;
}

void US_ConvertScan::set_wavl_ctrl(){
    cb_plot_id->disconnect();
    cb_plot_id->clear();
    intDataId.clear();
    wavelength.clear();
    wavl_id = 0;
    n_wavls = 0;
    if (lw_triple->count() == 0){
        le_lambstrt->clear();
        le_lambstop->clear();
        le_runIdInt->clear();
        offon_prev_next();
        emit sig_plot();
        return;
    }
    int ccw_id = lw_triple->currentRow();
    QString runId = intRunIds.at(ccwItemList.runClass.at(ccw_id));
    le_runIdInt->setText(runId);
    n_wavls = ccwItemList.wavelength.at(ccw_id).size();
    intDataId = ccwItemList.index.at(ccw_id);
    wavelength = ccwItemList.wavelength.at(ccw_id);
    le_lambstrt->setText(QString::number(wavelength.first()));
    le_lambstop->setText(QString::number(wavelength.last()));
    foreach (double wvl, wavelength) {
        cb_plot_id->addItem(QString::number(wvl));
    }
    offon_prev_next();
    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_id(int)));
    slt_set_id(0);
    return;
}

void US_ConvertScan::slt_prev_id(void){
    --wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
    return;
}

void US_ConvertScan::slt_next_id(void){
    ++wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
    return;
}

void US_ConvertScan::offon_prev_next(){
    pb_prev_id->setDisabled(wavl_id <= 0);
    pb_next_id->setDisabled(wavl_id >= (n_wavls - 1));
    return;
}

void US_ConvertScan::slt_set_ccw_default(){
    ccwList.clear();
    ccwList.index << ccwListMain.index;
    ccwList.cell << ccwListMain.cell;
    ccwList.channel << ccwListMain.channel;
    ccwList.wavelength << ccwListMain.wavelength;
    make_ccwItemList();
    set_listWidget();
    set_wavl_ctrl();
    emit sig_save_button();
    return;
}

void US_ConvertScan::slt_del_item(){
    le_desc->setText("");
    if (lw_triple->count() == 0)
        return;
    int row = lw_triple->currentRow();
    ccwItemList.index.removeAt(row);
    ccwItemList.cell.removeAt(row);
    ccwItemList.channel.removeAt(row);
    ccwItemList.wavelength.removeAt(row);
    set_listWidget();
    set_wavl_ctrl();
    emit sig_save_button();
    return;
}

void US_ConvertScan::slt_wavl_ctrl(int){
    set_wavl_ctrl();
    return;
}

void US_ConvertScan::slt_set_id(int id){
    wavl_id = id;
    offon_prev_next();
    le_desc->setText(allIntData.at(id).description);
    set_scan_ct();
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_plot(){
    le_status->clear();
    if (lw_triple->count() == 0)
        plot_title->setText("");
    else{
        QString title("Wavelength= %1 nm");
        double wl = wavelength.at(wavl_id);
        plot_title->setText(title.arg(wl));
    }
    plot_intensity();
    plot_refscan();
    plot_absorbance();
    return;
}

void US_ConvertScan::slt_update_scrng(double){
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(wavl_id);
    int lower = qRound(ct_scan_l->value());
    int upper = qRound(ct_scan_u->value());
    scansRange[index][0] = lower;
    scansRange[index][1] = upper;
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_zeroing(int){
    emit sig_plot();
    emit sig_save_button();
    return;
}

void US_ConvertScan::slt_channelBlanking(int state){
    if (state == Qt::Checked){
        frm_refScan->setDisabled(true);
    }
    else {
        frm_refScan->setEnabled(true);
    }
    slt_reset_refData();
    emit sig_save_button();
    return;
}

void US_ConvertScan::slt_xrange(int state){
    x_min_picked = -1;
    x_max_picked = -1;
    le_xrange->setText("");
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (state == Qt::Checked){
        pb_pick_rp->setEnabled(true);
        pb_pick_rp->setStyleSheet(qs.arg("yellow"));
    }else{
        pb_pick_rp->setDisabled(true);
        pb_pick_rp->setStyleSheet(qs.arg(color.name()));
    }
    emit sig_plot();
    emit sig_save_button();
    return;
}

void US_ConvertScan::slt_pick_point(){
    picker_abs->disconnect();
    x_min_picked = -1;
    x_max_picked = -1;
    le_xrange->setText("");
    if (absorbance.size() == 0)
        return;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    emit sig_plot();
    emit sig_save_button();
    connect(picker_abs, SIGNAL(cMouseUp(const QwtDoublePoint&)),
            this,   SLOT(slt_mouse(const QwtDoublePoint&)));
    return;
}

void US_ConvertScan::slt_mouse(const QwtDoublePoint& point){
    double x = point.x();
    int np = xvalues.size();
    double min_x = xvalues.at(0);
    double max_x = xvalues.at(np - 1);
    QString str;
    if (x > min_x && x < max_x){
        if (x_min_picked == -1){
            x_min_picked = x;
            str = tr("%1 -");
            le_xrange->setText(str.arg(x_min_picked, 0, 'f', 3));
            emit sig_plot();
        } else {
            if (x <= x_min_picked){
                QString mess("Pick a radial point greater than: %1 cm");
                QMessageBox::warning( this, tr( "Warning" ), mess.arg(x_min_picked));
                return;
            }
            x_max_picked = x;
            picker_abs->disconnect();
            str = tr("%1 - %2 cm");
            le_xrange->setText(str.arg(x_min_picked, 0, 'f', 3).
                               arg(x_max_picked, 0, 'f', 3));
            pb_pick_rp->setStyleSheet("QPushButton { background-color: green }");
            emit sig_plot();
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

void US_ConvertScan::slt_reset_allscans(){
    if (scansRange.size() == 0 || lw_triple->count() == 0)
        return;
    for (int i = 0; i < scansRange.size(); ++i){
        scansRange[i][0] = 0;
        scansRange[i][1] = scansRange.at(i).at(2);
    }
    set_scan_ct();
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_apply_allscans(){
    if (scansRange.size() == 0 || lw_triple->count() == 0)
        return;
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(wavl_id);
    QString curr_ccw = ccwStrListMain.at(index);
    int lower = scansRange.at(index).at(0);
    int upper = scansRange.at(index).at(1);
    QVector<int> mismatch;
    for (int i = 0; i < ccwItemList.size(); ++i){
        for (int j = 0; j < ccwItemList.index.at(i).size(); ++j){
            int id = ccwItemList.index.at(i).at(j);
            int max = scansRange.at(id).at(2);
            if (lower <= max && upper <= max){
                scansRange[id][0] = lower;
                scansRange[id][1] = upper;
            } else
                mismatch << id;
        }
    }

    if (mismatch.size() > 0){
        QListWidget* wl_mm = new QListWidget();
        QString item("%1- %2; # scans= %3");
        for (int i = 0; i < mismatch.size(); ++i){
            int id = mismatch.at(i);
            QString ccw = ccwStrListMain.at(id);
            int ns = scansRange.at(id).at(2);
            wl_mm->addItem(item.arg(i + 1).arg(ccw).arg(ns));
        }

        QLabel* mess = us_label("The number of scans in the CCW list "
                                "below does not match the current CCW= "
                                + curr_ccw + "\nPlease modify them separately.");
        mess->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
        mess->setWordWrap(true);
        mess->setStyleSheet(tr("border: 1px solid white;"
                               "color: black;"
                               "background-color: white;"));
        QLabel* lb_wrn = new QLabel();
        lb_wrn->setPixmap(this->style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(50));
        QHBoxLayout* mess_lyt = new QHBoxLayout();
        mess_lyt->addWidget(lb_wrn,0 );
        mess_lyt->addWidget(mess, 1);

        QPushButton* ok = new QPushButton("&Ok");
        ok->setIcon(this->style()->standardIcon(QStyle::SP_DialogOkButton));
        QHBoxLayout* ok_lyt = new QHBoxLayout();
        ok_lyt->addStretch(1);
        ok_lyt->addWidget(ok);
        ok_lyt->addStretch(1);

        QVBoxLayout* lyt = new QVBoxLayout();
        lyt->addLayout(mess_lyt);
        lyt->addWidget(wl_mm);
        lyt->addLayout(ok_lyt);

        QDialog *qd = new QDialog(this);
        qd->setWindowTitle("Warning !");
        qd->setWindowIcon(this->style()->standardIcon(QStyle::SP_MessageBoxWarning));
        qd->setLayout(lyt);
        connect(ok, SIGNAL(clicked()), qd, SLOT(close()));
        qd->exec();
    }
}

void US_ConvertScan::slt_reset_scans(){
    if (scansRange.size() == 0 || lw_triple->count() == 0)
        return;

    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(wavl_id);
    scansRange[index][0] = 0;
    scansRange[index][1] = scansRange.at(index).at(2);
    set_scan_ct();
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_reset_refData(){
    refId = -1;
    refData.clear();
    le_ref_range->setText("");
    pb_import_refScans->setEnabled(true);
    pb_reset_refData->setDisabled(true);
    pb_save->setDisabled(true);
    uncheck_CA_silently();
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_load_refScans(void){
    if (diskDB_ctrl->db()){
        load_from_DB();
        return;
    }else{
        QString fname;
        fname = QFileDialog::getOpenFileName( this,
              tr( "Open Reference Scan File" ),
              US_Settings::importDir(), tr("*.aucrs"));
        if (fname.isEmpty()) return;
        qDebug() << fname;
        refData.clear();
        xvaluesRefCAC.clear();
        int error = US_RefScanDataIO::readRefData(fname, refData);
        if (error != US_RefScanDataIO::OK){
            refData.clear();
            QString mess = US_RefScanDataIO::errorString(error);
            le_status->setText(mess);
            uncheck_CA_silently();
        } else{
            pb_import_refScans->setDisabled(true);
            pb_reset_refData->setEnabled(true);
            bool state;
            xvaluesRefCAC = refData.get_CA_corrected(state);
            if (! state){
                xvaluesRefCAC.clear();
                uncheck_CA_silently();
                le_status->setText("Error in chromatic aberration correction!");
            } else
                ckb_ChroAberCorr->setEnabled(true);
            emit sig_save_button();
        }
        QString text("%1 - %2 nm");
        int nw = refData.nWavelength;
        double w1 = refData.wavelength.at(0);
        double w2 = refData.wavelength.at(nw - 1);
        le_ref_range->setText(text.arg(w1).arg(w2));
        plot_refscan();
        plot_absorbance();
        le_status->clear();
        return;
    }
}

void US_ConvertScan::slt_save_avail(void){
    if (lw_triple->count() == 0 ||
        ( refData.nWavelength == 0 &&
         !ckb_channelBlanking->isChecked()) )
    {
        pb_save->setDisabled(true);
        return;
    }

    if (ckb_channelBlanking->isChecked() &&
        cb_buffer->currentIndex() == 0){
        pb_save->setDisabled(true);
        return;
    }

    if (ckb_xrange->isChecked()){
        if (x_min_picked != -1 && x_max_picked != -1)
            pb_save->setEnabled(true);
        else
            pb_save->setDisabled(true);
    }else{
        pb_save->setEnabled(true);
    }
    return;
}

void US_ConvertScan::select_CCW_save(QVector<int>& selist) {
    selist.clear();
    QDialog *dialog = new QDialog(this);
    QList<QCheckBox*> chk_list;
    QVBoxLayout *lyt = new QVBoxLayout();
    QLabel *label = us_banner("Select Triple(s) To Save");
    lyt->addWidget(label);
    for (int ii = 0; ii < lw_triple->count(); ii++) {
        QCheckBox* chk = new QCheckBox(lw_triple->item(ii)->text());
        QGridLayout *gl = us_checkbox(lw_triple->item(ii)->text(), chk);
        chk_list << chk;
        lyt->addLayout(gl);
    }
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                                     | QDialogButtonBox::Cancel);
    lyt->addStretch(1);
    lyt->addWidget(buttons);
    lyt->setMargin(1);
    lyt->setSpacing(1);
    dialog->setLayout(lyt);
    dialog->setMaximumWidth(500);
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    int state = dialog->exec();
    if (state != QDialog::Accepted) return;
    for (int ii = 0; ii < chk_list.size(); ii++) {
        if (chk_list.at(ii)->isChecked()) selist << ii;
    }
}

void US_ConvertScan::slt_save(void){
    QVector<int> selist;
    select_CCW_save(selist);
    if (selist.size() == 0) return;

    runIdAbs = le_runIdAbs->text();
    QDir dir = QDir(le_dir->text());
    if (dir.cd(runIdAbs)){
        QString absPath = dir.absolutePath();
        int ck = QMessageBox::question(this, tr( "Warning!" ),
                                       tr( "Output directory exists!\n" ) + runIdAbs +
                                       tr( "\n\n Do you really want to replace it?"));
        if (ck == QMessageBox::Yes){
            dir.removeRecursively();
            dir.mkpath(absPath);
            dir.setPath(absPath);
        }else return;
    }else{
        dir.mkdir(runIdAbs);
        dir.cd(runIdAbs);
    }
    dir.makeAbsolute();

    int nwl_tot = 0;
    foreach (int ii, selist) {
        nwl_tot += ccwItemList.wavelength.at(ii).size();
    }

    int cb_buffer_row = cb_buffer->currentIndex();
    int n = 1;
    QString status = tr("writting: %1 %2");
    QString percent;
    QString fileName("%1.RA.%2.%3.%4.auc");
    foreach (int ii, selist){
        for (int jj = 0; jj < ccwItemList.wavelength.at(ii).size(); ++jj){
            percent = QString::number(100.0 * n / nwl_tot, 'f', 1);
            le_status->setText(status.arg(percent).arg(QChar(37)));
            qApp->processEvents();
            n++;
            int dataId = ccwItemList.index.at(ii).at(jj);
            double wavelength = ccwItemList.wavelength.at(ii).at(jj);

            int dataId_buff = -1;
            if (cb_buffer_row > 0){
                dataId_buff = ccwItemList.index.at(cb_buffer_row - 1).at(jj);
            }

            if (ckb_channelBlanking->isChecked()){
                get_absorbance(dataId, dataId_buff);
            } else{
                if (! get_refId(wavelength)){
                    qDebug() << tr("Not found corrosponding reference data for: ") <<
                        allIntDataFiles.at(dataId).fileName();
                    continue;
                }
                get_pseudo_absorbance(refId, dataId, false);
                if (dataId_buff != -1){
                    get_relative_absorbance(dataId_buff);
                }
            }
            trim_absorbance();
            int scan_l1 = scansRange.at(dataId).at(0);
            int scan_l2 = scansRange.at(dataId).at(1);
            QVector<US_DataIO::Scan> absorbance_sel;
            for (int k = scan_l1; k <scan_l2; ++k)
                absorbance_sel << absorbance.at(k);
            if (absorbance_sel.size() == 0)
                continue;

            US_DataIO::RawData rawData = allIntData.at(dataId);
            rawData.type[0] = 'R';
            rawData.type[1] = 'A';
            uchar uuid[ 16 ];
            QString uuid_string = US_Util::new_guid();
            US_Util::uuid_parse( uuid_string, uuid );
            memcpy( rawData.rawGUID,   (char*) uuid, 16 );
            rawData.scanData.clear();
            rawData.scanData << absorbance_sel;
            int cell = rawData.cell;
            char channel = rawData.channel;
            QString fn = fileName.arg(runIdAbs).arg(cell).arg(channel).arg(wavelength);
            QFileInfo fileInfo(dir, fn);
            US_DataIO::writeRawData(fileInfo.absoluteFilePath(), rawData);
        }
    }
    le_status->setText("written on the local disk !");

    return;
}

void US_ConvertScan::slt_update_buffer(int index){
    if (index == 0)
        ckb_xrange->setEnabled(true);
    else{
            ckb_xrange->setCheckState(Qt::Checked);
            ckb_xrange->setDisabled(true);
    }
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_update_smooth(double){
    emit sig_plot();
    return;
}

void US_ConvertScan::slt_cac(int){
    slt_plot();
}

void US_ConvertScan::set_listWidget(){
    lw_triple->disconnect();
    lw_triple->clear();
    if (ccwItemList.size() == 0) return;
    bool uniqueRun = true;
    int r0 = ccwItemList.runClass.at(0);
    for (int i = 1; i < ccwItemList.runClass.size(); i++) {
        if (ccwItemList.runClass.at(i) != r0) {
            uniqueRun = false;
            break;
        }
    }
    QString item_uq("%1 / %2 / %3-%4 (%5)");
    QString item("[%1] %2 / %3 / %4-%5 (%6)");
    for (int i = 0; i < ccwItemList.size(); ++i){
        int run = ccwItemList.runClass.at(i);
        int cell = ccwItemList.cell.at(i);
        char channel = ccwItemList.channel.at(i);
        double min_wl = ccwItemList.wavelength.at(i).first();
        double max_wl = ccwItemList.wavelength.at(i).last();
        int nwl = ccwItemList.wavelength.at(i).size();
        QString item_i;
        if (uniqueRun) {
            item_i = item_uq.arg(cell).arg(channel).arg(min_wl).arg(max_wl).arg(nwl);
        } else {
            item_i = item.arg(run).arg(cell).arg(channel).arg(min_wl).arg(max_wl).arg(nwl);
        }

        lw_triple->addItem(item_i);
    }
    lw_triple->setCurrentRow(0);
    set_buffer_list();
    int *scp;
    for (int i = 0; i < scansRange.size(); ++i){
        scp = scansRange[i].data();
        scp[0] = 0;
        scp[1] = scp[2];
    }
    connect( lw_triple, SIGNAL( currentRowChanged(int) ),
            this, SLOT( slt_wavl_ctrl(int) ) );
    return;
}

void US_ConvertScan::set_buffer_list(){
    cb_buffer->disconnect();
    cb_buffer->clear();
    QStringList list;
    list << "None";
    for (int i = 0; i < lw_triple->count(); ++i){
        list << lw_triple->item(i)->text();
    }
    cb_buffer->addItems(list);
    cb_buffer->setCurrentIndex(0);
    absorbanceBuffer.clear();
    connect(cb_buffer, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slt_update_buffer(int)));
    emit sig_save_button();
    return;
}

void US_ConvertScan::load_from_DB(){
    QString error;
    dbCon->connect(pw.getPasswd(), error);
    if (dbCon->isConnected()){
        QStringList DB  = US_Settings::defaultDB();
        if (DB.isEmpty()){
            le_status->setText("database: Undefined");
            return;
        }
        else
            le_status->setText("database: " + DB.at(3));
    }else{
        le_status->setText("database connection failed!");
        return;
    }

    LoadDBWidget *db_w = new LoadDBWidget(this, dbCon, refData);
    int state_db = db_w->exec();
    if (state_db != QDialog::Accepted){
        le_status->setText("Load Reference data!");
        return;
    }else
        le_status->clear();
    xvaluesRefCAC.clear();
    bool state;
    xvaluesRefCAC = refData.get_CA_corrected(state);
    if (! state){
        xvaluesRefCAC.clear();
        le_status->setText("Error in chromatic aberration correction!");
    }
    pb_import_refScans->setDisabled(true);
    pb_reset_refData->setEnabled(true);
    emit sig_save_button();
    QString text("%1 - %2 nm");
    int nw = refData.nWavelength;
    double w1 = refData.wavelength.at(0);
    double w2 = refData.wavelength.at(nw - 1);
    le_ref_range->setText(text.arg(w1).arg(w2));
    plot_refscan();
    plot_absorbance();

    return;
}

void US_ConvertScan::make_ccwItemList(){
    ccwItemList.clear();
    if (ccwList.size() == 0){
        return;
    }

    QMap<QString, QVector<QPair<double, int>>> wavl_idx_map;
    QString key_str("%1-%2-%3");
    for (int ii = 0; ii < ccwList.size(); ii++){
        int runId = intRunIds.indexOf(ccwList.runId.at(ii));
        int cell = ccwList.cell.at(ii);
        char channel = ccwList.channel.at(ii);
        QString key = key_str.arg(runId).arg(cell).arg(channel);
        wavl_idx_map[key] << qMakePair(ccwList.wavelength.at(ii), ccwList.index.at(ii));
    }

    QStringList keys = wavl_idx_map.keys();
    std::sort(keys.begin(), keys.end());

    foreach (QString key, keys) {
        QStringList ksp = key.split("-");
        int runClass = ksp.at(0).toInt();
        int cell = ksp.at(1).toInt();
        char channel = ksp.at(2).at(0).toLatin1();
        QVector<QPair<double, int>> wavl_idx = wavl_idx_map.value(key);
        std::sort(wavl_idx.begin(), wavl_idx.end(),
                  [](auto a, auto b) { return a.first < b.first;});
        QVector<double> wavls;
        QVector<int> idx;
        for (int ii = 0; ii < wavl_idx.size(); ii++) {
            wavls << wavl_idx.at(ii).first;
            idx << wavl_idx.at(ii).second;
        }
        ccwItemList.runClass << runClass;
        ccwItemList.cell << cell;
        ccwItemList.channel << channel;
        ccwItemList.index << idx;
        ccwItemList.wavelength << wavls;
    }
    return;
}

void US_ConvertScan::set_scan_ct(){
    ct_scan_l->disconnect();
    ct_scan_u->disconnect();
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(wavl_id);
    int lower = scansRange.at(index).at(0);
    int upper = scansRange.at(index).at(1);
    int max = scansRange.at(index).at(2);
    ct_scan_l->setRange(0, max);
    ct_scan_l->setValue(lower);
    ct_scan_u->setRange(0, max);
    ct_scan_u->setValue(upper);
    connect(ct_scan_l, SIGNAL(valueChanged(double)),
            this, SLOT(slt_update_scrng(double)));
    connect(ct_scan_u, SIGNAL(valueChanged(double)),
            this, SLOT(slt_update_scrng(double)));
    return;
}

void US_ConvertScan::plot_intensity(void){
    qwtplot_insty->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (lw_triple->count() == 0){
        grid = us_grid(qwtplot_insty);
        qwtplot_insty->replot();
        return;
    }

    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(wavl_id);
//    QVector<double> xvalues = allIntData.at(index).xvalues;
    get_intensity(index);
    int ns = intensity.size();
    if (ns == 0){
        grid = us_grid(qwtplot_insty);
        qwtplot_insty->replot();
        return;
    }

    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(1);
    double rb = qCeil(ns / 50.0);
    int nc = qCeil(ns / rb);
    HSVcolormap colormap(nc);
    QVector<QColor> colorList;
    int error = colormap.get_colorlist(colorList);

    const double *x, *r;
    x = xvalues.data();

    double min_x  =  1e20;
    double max_x  = -1e20;
    double min_r  =  1e20;
    double max_r  = -1e20;
    int np = xvalues.size();
    for (int i = 0; i < ns; ++i){
        r = intensity.at(i).rvalues.data();
        for (int j = 0; j < np; ++j){
            min_x = qMin(min_x, x[j]);
            max_x = qMax(max_x, x[j]);
            min_r = qMin(min_r, r[j]);
            max_r = qMax(max_r, r[j]);
        }
        QwtPlotCurve* curve = us_curve( qwtplot_insty,"");
        if (error == 0)
            pen_plot.setColor(colorList[ i % nc ]);
        curve->setPen( pen_plot );
        curve->setSamples(x, r, np);
    }

    grid = us_grid(qwtplot_insty);
    double dx = (max_x - min_x) * 0.05;
    double dr = (max_r - min_r) * 0.05;
    qwtplot_insty->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    qwtplot_insty->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_insty->updateAxes();
    qwtplot_insty->replot();
    return;
}

void US_ConvertScan::plot_refscan(void){
    if (refData.xValues.size() == 0 || lw_triple->count() == 0){
        refId = -1;
        return;
    }
    if (intensity.size() == 0)
        return;

    QPen pen = QPen( QBrush( Qt::white ), 2.0 );
    double min_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double max_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).upperBound();

    if (! get_refId(wavelength.at(wavl_id))){
        le_status->setText("reference data: wavelength not found !");
        return;
    }
    le_status->clear();
    const double *xp;
    if (ckb_ChroAberCorr->isChecked() && refData.CAState){
        xp = xvaluesRefCAC.at(refId).data();
    } else {
        xp = refData.xValues.data();
    }
    const double *rp = refData.rValues.at(refId).data();

    for (int i = 0; i < refData.nPoints; ++i){
        min_r = qMin(min_r, rp[i]);
        max_r = qMax(max_r, rp[i]);
    }
    QwtPlotCurve* curve = us_curve( qwtplot_insty,"");
    curve->setPen(pen);
    curve->setSamples(xp, rp, refData.nPoints);
    double dr = (max_r - min_r) * 0.05;
    qwtplot_insty->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_insty->updateAxes();
    qwtplot_insty->replot();

    return;
}

void US_ConvertScan::plot_absorbance(void){
    qwtplot_abs->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    grid = us_grid(qwtplot_abs);
    qwtplot_abs->replot();
    bool quit = false;
    if (lw_triple->count() == 0){
        quit = true;
    }
    if (ckb_channelBlanking->isChecked()){
        if (cb_buffer->count() == 0 || cb_buffer->currentIndex() == 0){
            quit = true;
        }
    } else{
        if (refData.xValues.size() == 0 || refId == -1){
            quit = true;
        }
    }
    if (quit) return;

    double min_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    double max_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).upperBound();
    double min_r  =  1e20;
    double max_r  = -1e20;

    int row = lw_triple->currentRow();
    int dataId = ccwItemList.index.at(row).at(wavl_id);

    row = cb_buffer->currentIndex();
    int dataId_buff = -1;
    if (row > 0){
        dataId_buff = ccwItemList.index.at(row - 1).at(wavl_id);
    }

    if (ckb_channelBlanking->isChecked()){
        if (! get_absorbance(dataId, dataId_buff)) return;
    } else{
        get_pseudo_absorbance(refId, dataId, false);
        if (dataId_buff != -1){
            get_relative_absorbance(dataId_buff);
        }
    }
    trim_absorbance();

    int scan_l1 = scansRange.at(dataId).at(0);
    int scan_l2 = scansRange.at(dataId).at(1);
    QVector<US_DataIO::Scan> absorbance_sel;
    for (int i = scan_l1; i <scan_l2; ++i)
        absorbance_sel << absorbance.at(i);
    if (absorbance_sel.size() == 0){
        grid = us_grid(qwtplot_abs);
        qwtplot_abs->replot();
        return;
    }

    int ns = absorbance_sel.size();
    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(1);
    double rb = qCeil(ns / 50.0);
    int nc = qCeil(ns / rb);
    HSVcolormap colormap(nc);
    QVector<QColor> colorList;
    int error = colormap.get_colorlist(colorList);

    const double *xp = allIntData.at(dataId).xvalues.data();
    const double *rp;

    int np = allIntData.at(dataId).xvalues.size();
    int l1 = np * 0.2;
    int l2 = np * 0.8;
    for (int i = 0; i < ns; ++i){
        rp = absorbance_sel.at(i).rvalues.data();
        for (int j = 0; j < np; ++j)
            if (j > l1 && j < l2){
                min_r = qMin(min_r, rp[j]);
                max_r = qMax(max_r, rp[j]);
            }
        QwtPlotCurve* curve = us_curve( qwtplot_abs,"");
        if (error == 0)
            pen_plot.setColor(colorList[ i % nc ]);
        curve->setPen( pen_plot );
        curve->setSamples(xp, rp, np);
    }
    grid = us_grid(qwtplot_abs);
    double dr = (max_r - min_r) * 0.05;
    min_r -= dr;
    max_r += dr;
    if (x_min_picked != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << x_min_picked;
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_abs,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    if (x_max_picked != -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double x0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << x_max_picked;
            yy << x0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_abs,"");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    qwtplot_abs->setAxisScale( QwtPlot::xBottom, min_x, max_x);
    qwtplot_abs->setAxisScale( QwtPlot::yLeft  , min_r, max_r);
    qwtplot_abs->updateAxes();
    qwtplot_abs->replot();
    return;
}

void US_ConvertScan::get_intensity(int data_id){
    intensity.clear();
    int scan_l1 = scansRange.at(data_id).at(0);
    int scan_l2 = scansRange.at(data_id).at(1);
    xvalues.clear();
    xvalues << allIntData.at(data_id).xvalues;
    for (int i = scan_l1; i < scan_l2; ++i)
        intensity << allIntData.at(data_id).scanData.at(i);
    return;
}

bool US_ConvertScan::get_refId(double wavelength){
    refId = -1;
    if (refData.xValues.size() == 0 || lw_triple->count() == 0)
        return false;
    bool flag = false;
    int wl = qRound(wavelength * 10);
    double *wp = refData.wavelength.data();
    int wlref;
    for (int i = 0; i < refData.nWavelength; ++i){
        wlref = qRound(wp[i] * 10);
        if (wlref == wl){
            refId = i;
            flag = true;
            break;
        }
    }
    return flag;
}

void US_ConvertScan::get_pseudo_absorbance(int id_ref, int id_data, bool buffer){
    if (buffer)
        absorbanceBuffer.clear();
    else
        absorbance.clear();
    const double *xp_ref;
    if (ckb_ChroAberCorr->isChecked() && refData.CAState){
        xp_ref = xvaluesRefCAC.at(id_ref).data();
    } else {
        xp_ref = refData.xValues.data();
    }
    const double *rp_ref = refData.rValues.at(id_ref).data();
    const double *xp_dta = allIntData.at(id_data).xvalues.data();
    const double *rp_dta;
    int xref, xdta, id;
    double *rp_abs;
    double rdta;
    int np = allIntData.at(id_data).xvalues.size();
    int np_ref = refData.nPoints;
    int ns = allIntData.at(id_data).scanData.size();
    for (int i = 0; i < ns; ++i){
        US_DataIO::Scan scan = allIntData.at(id_data).scanData.at(i);
        rp_dta = scan.rvalues.data();
        QVector<double> rval_abs(np);
        rp_abs = rval_abs.data();
        id = 0;
        for (int j = 0; j < np; ++j){
            xdta = qRound(xp_dta[j] * 1000);
            xref = qRound(xp_ref[id] * 1000);
            if (xref < xdta){
                while(xref < xdta  && id < (np_ref - 1))
                    xref = qRound(xp_ref[++id] * 1000);
                if (xref > xdta){
                    qDebug() << "Radial Steps Not Matched!";
                    le_status->setText("Radial Steps Not Matched!");
                    absorbance.clear();
                    return ;
                }
            }
            rdta = rp_dta[j];
            if (rdta <= 0)
                rdta = 1e-5;
            rp_abs[j] = std::log10(rp_ref[id] / rdta);
        }
        scan.rvalues.clear();
        scan.rvalues = rval_abs;
        scan.stddevs.clear();
        scan.nz_stddev = false;
        if (buffer)
            absorbanceBuffer << scan;
        else
            absorbance << scan;
        rval_abs.clear();
    }
    return;
}

bool US_ConvertScan::get_absorbance(int id_data, int id_buff){
    int ns = allIntData.at(id_data).scanData.size();
    if (ns != allIntData.at(id_buff).scanData.size()){
        qDebug() << "Number Scans Not Matched!";
        le_status->setText("Number Scans Not Matched!");
        absorbance.clear();
        // QMessageBox::warning(this, "Error!", "Number Scans Not Matched!");
        return false;
    }
    absorbance.clear();

    int smooth_l = ct_smooth->value();

    QVector< QVector<double>> buffer_rvalues;
    for (int i = 0; i < ns; i++){
        if (smooth_l > 0){
            QVector<double> rval = get_smooth(allIntData.at(id_buff).scanData.at(i).rvalues,
                                              smooth_l, true, true);
            buffer_rvalues << rval;
        } else {
            buffer_rvalues << allIntData.at(id_buff).scanData.at(i).rvalues;
        }
    }

    for (int ii = 0; ii < ns; ++ii){
        US_DataIO::Scan scan = allIntData.at(id_data).scanData.at(ii);
        QVector<double> rval_data(scan.rvalues);
        QVector<double> xval_data(allIntData.at(id_data).xvalues);
        QVector<double> xval_buff(allIntData.at(id_buff).xvalues);

        if( !linear_interpolation(xval_buff, xval_data, rval_data)) {
            QMessageBox::warning(this, "Error!", "len(x) != len(y)");
            return false;
        }
        for (int jj = 0; jj < rval_data.size(); ++jj){
            double val = buffer_rvalues.at(ii).at(jj) / rval_data.at(jj);
            if (val <= 0) val = 1e-5;
            rval_data[jj] = std::log10(val);
        }
        scan.rvalues.clear();
        scan.rvalues << rval_data;
        scan.stddevs.clear();
        scan.nz_stddev = false;
        absorbance << scan;
    }
    return true;
}

void US_ConvertScan::get_relative_absorbance(int bufferId){
    get_pseudo_absorbance(refId, bufferId, true);

    if (absorbanceBuffer.size() == 0)
        return;
    if (absorbance.size() != absorbanceBuffer.size()){
        qDebug() << "Number Scans Not Matched!";
        QMessageBox::warning(this, "Error!", "Number Scans Not Matched!");
        return;
    }
    int smooth_l = ct_smooth->value();
    if (smooth_l > 1){
        for (int i = 0; i < absorbanceBuffer.size(); ++i){
            QVector<double> rval =  absorbanceBuffer.at(i).rvalues;
            QVector<double> rval_s = get_smooth(rval, smooth_l, true, false);
            absorbanceBuffer[i].rvalues.clear();
            absorbanceBuffer[i].rvalues << rval_s;
        }
    }
    for (int i = 0; i < absorbance.size(); ++i){
        int np = absorbance.at(i).rvalues.size();
        int np_b = absorbanceBuffer.at(i).rvalues.size();
        if (np != np_b){
            QMessageBox::warning(this, "Error!", "Number Radial Points Not Matched!");
            return;
        }
        double *rp = absorbance[i].rvalues.data();
        const double *rbp = absorbanceBuffer.at(i).rvalues.data();
        for (int j = 0; j < np; ++j)
            if (rp[j] < maxAbs - 0.1)
                rp[j] -= rbp[j];
        for (int j = 0; j < np; ++j){
            if (rp[j] > maxAbs)
                rp[j] = maxAbs;
            else if (rp[j] < -maxAbs)
                rp[j] = -maxAbs;
        }
    }
    return;
}

void US_ConvertScan::trim_absorbance(){
    bool cutting = true;
    bool zeroing = true;
    if (x_min_picked == -1 || x_max_picked == -1)
        cutting = false;
    if (!ckb_zeroing->isChecked())
        zeroing = false;
    const double *xp = xvalues.data();
    int id_min = 0;
    int id_max = xvalues.size();
    if (cutting){
        id_min = -1;
        id_max = -1;
        bool check_min = true;
        for (int i = 0; i < xvalues.size(); ++i){
            if (check_min){
                if (xp[i] >= x_min_picked){
                    id_min = i;
                    check_min = false;
                }
            } else{
                if (xp[i] > x_max_picked){
                    id_max = i;
                    break;
                }
            }
        }
        if (id_max == -1)
            id_max = xvalues.size();
    }


    int row = lw_triple->currentRow();
    int dataId = ccwItemList.index.at(row).at(wavl_id);
    int scan_l1 = scansRange.at(dataId).at(0);
    int scan_l2 = scansRange.at(dataId).at(1);

    double min_r = 1e20;
    bool zr = false;
    for (int i = 0; i < absorbance.size(); ++i){
        int np = absorbance.at(i).rvalues.size();
        double *rp = absorbance[i].rvalues.data();
        for (int j = 0; j < np; ++j){
            if (j < id_min || j > id_max)
                rp[j] = 0;
            else if (zeroing){
                if (i >= scan_l1 && i < scan_l2){
                    min_r = qMin(min_r, rp[j]);
                    zr = true;
                }
            }
        }
    }
    qDebug() << min_r << scan_l1 << scan_l2;
    if (zeroing && zr){
        for (int i = 0; i < absorbance.size(); ++i){
            double *rp = absorbance[i].rvalues.data();
            for (int j = id_min; j < id_max; ++j)
                rp[j] -= min_r;
        }
    }
    return;
}

QVector<double> US_ConvertScan::get_smooth(QVector<double> array, int winlen,
                                           bool ave, bool intsy){
    if (ave){
        int np = array.size();
        QVector<double> array_s(np, 0);
        const double *rp = array.data();
        double *rsp = array_s.data();
        int N = 2 * winlen + 1;
        for (int i = 0; i < np; ++i){
            if (i < winlen || (np - i - 1) < winlen){
                rsp[i] = rp[i];
                continue;
            }
            bool flag = true;
            double yy = 0;
            for (int j = -winlen; j <= winlen; ++j){
                yy += rp[i + j];
                if (intsy){
                    continue;
                }
                if (rp[i + j] > (maxAbs - 0.1)){
                    flag = false;
                    rsp[i] = rp[i];
                    break;
                }
            }
            if (flag){
                yy /= N;
                rsp[i] = yy;
            }
        }
        return array_s;
    }
    US_Math2::gaussian_smoothing(array, winlen);
    return array;
}

bool US_ConvertScan::linear_interpolation(QVector<double>& xt_vals,
                                          QVector<double>& x_vals,
                                          QVector<double>& y_vals) {
    if (x_vals.size() != y_vals.size()) return false;
    double *xtp = xt_vals.begin();
    double *xp = x_vals.begin();
    if (x_vals.size() == xt_vals.size()) {
        bool eq = true;
        for (int ii = 0; ii < x_vals.size(); ii++) {
            int a = qRound(xp[ii] * 1000);
            int b = qRound(xtp[ii] * 1000);
            if (a != b) {
                eq = false;
                break;
            }
        }
        if (eq) return true;
    }

    double *yp = y_vals.begin();
    int np_x = x_vals.size();
    int np_xt = xt_vals.size();
    QVector<double> yo_vals;
    int nn = 0;
    for (int ii = 0; ii < np_xt; ii++) {
        int xt = qRound(xtp[ii] * 1000);
        int x = qRound(xp[nn] * 1000);
        if (xt < x && nn == 0) {
            yo_vals << yp[nn];
            continue;
        } else if (xt == x) {
            yo_vals << yp[nn];
            continue;
        } else if (xt > x && nn >= (np_x - 1)) {
            yo_vals << yp[np_x - 1];
            continue;
        }
        double x1 = xp[nn];
        double y1 = yp[nn];
        while (true) {
            if (xt == x) {
                yo_vals << yp[nn];
                break;
            } else if (xt < x) {
                if (xt >= qRound(xp[nn - 1] * 1000)) {
                    double m = (yp[nn] - yp[nn - 1]) / (xp[nn] - xp[nn - 1]);
                    double yy = yp[nn - 1] + m * (xt - xp[nn - 1]);
                    yo_vals << yy;
                    break;
                } else {
                    double m = (yp[nn] - y1) / (xp[nn] - x1);
                    double yy = yp[nn - 1] + m * (xt - xp[nn - 1]);
                    yo_vals << yy;
                    break;
                }
            }
            if (nn < (np_x - 1)) {
                x = qRound(xp[++nn] * 1000);
            } else {
                yo_vals << yp[nn];
                break;
            }
        }
    }
    x_vals.clear();
    x_vals << xt_vals;
    y_vals.clear();
    y_vals << yo_vals;
    return true;
}

void US_ConvertScan::uncheck_CA_silently(){
    ckb_ChroAberCorr->disconnect();
    ckb_ChroAberCorr->setCheckState(Qt::Unchecked);
    connect(ckb_ChroAberCorr, SIGNAL(stateChanged(int)), this, SLOT(slt_cac(int)));
    ckb_ChroAberCorr->setDisabled(true);
}

////
///
LoadDBWidget::LoadDBWidget(QWidget* w, US_DB2 *dbCon,
                           US_RefScanDataIO::RefData &refDataIn): US_WidgetsDialog(w, Qt::Dialog)
{
    setPalette( US_GuiSettings::frameColor() );
    db = dbCon;
    refData = &refDataIn;
    QStringList DB  = US_Settings::defaultDB();
    setWindowTitle( "Database: " + DB.at(3));

    QStringList q( "" );
    q.clear();
    q  << QString( "get_instrument_names" )
       << QString::number( 1 );
    db->query( q );

    if ( db->lastErrno() == US_DB2::OK ){
        // If not, no instruments defined
        QList< int > instIDs;
        while (db->next()) {
            int ID = db->value( 0 ).toString().toInt();
            instIDs << ID;
            qDebug() << "InstID: " << ID;
        }

        foreach ( int ID, instIDs ){
            q.clear();
            q  << QString( "get_instrument_info_new" )
               << QString::number( ID );
            db->query( q );
            db->next();
            QString name = db->value( 0 ).toString();

            if ( name.contains("Optima")){
                instrumentIDs << ID;
                instrumentNames << name;
            }
        }
    }

    // Let's see if we can find the run ID
    q.clear();
    q << "get_referenceScan_info";
    db->query( q );
    qDebug() << db->numRows();
    if ( db->lastErrno() == US_DB2::NOROWS ){
        qDebug() << "reference table is empty!";
    }
    while (db->next()){
        refScanTableInfo refScanInfo;
        refScanInfo.id = db->value(0).toString().toInt();
        refScanInfo.instrumentID = db->value(1).toString().toInt();
        refScanInfo.personID = db->value(2).toString().toInt();
        refScanInfo.type = db->value(3).toString();
        refScanInfo.exprimentIds = db->value(4).toString().split(",");
        refScanInfo.referenceTime = str2date(db->value(5).toString());
        refScanInfo.nWavelength = db->value(6).toString().toInt();
        refScanInfo.nPoints = db->value(7).toString().toInt();
        refScanInfo.startWavelength = db->value(8).toString().toDouble();
        refScanInfo.stopWavelength = db->value(9).toString().toDouble();
        int nb = db->value(10).toString().toInt();
        if (nb == 0)
            refScanInfo.null_blob = false;
        else
            refScanInfo.null_blob = true;
        refScanInfo.lastUpdated = str2date(db->value(11).toString());
        if (! refScanInfo.null_blob)
            refTable << refScanInfo;
    }

    for (int i = 0; i < instrumentNames.size(); ++i){
        QVector<QDate> refTime;
        QVector<int> ids_i;
        for (int j = 0; j < refTable.size(); ++j)
            if (refTable.at(j).instrumentID == instrumentIDs.at(i)){
                refTime << refTable.at(j).referenceTime;
                ids_i << j;
            }
        if (ids_i.size() > 1)
            std::stable_sort(ids_i.begin(), ids_i.end(), [&refTime](int i1, int i2)
                {return  refTime.at(i1) > refTime.at(i2);});
        instrumentRefList << ids_i;
    }
    ////
    QLabel *lb_instrument = us_label("Instruments List", 2, 1);
    lb_instrument->setAlignment(Qt::AlignCenter);
    tw_instruments = new QTreeWidget();
    tw_instruments->setColumnCount(1);
    tw_instruments->setHeaderLabels(QStringList() << "ID" << "Name");
    for (int i = 0; i < instrumentNames.size(); ++i){
        QTreeWidgetItem *twi = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
        twi->setText(0, QString::number(i + 1));
        twi->setText(1, instrumentNames.at(i));
        tw_instruments->addTopLevelItem(twi);
    }
    tw_instruments->setStyleSheet("background-color: white");
    tw_instruments->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    QLabel *lb_refTable = us_label("Available Reference Scans", 2, 1);
    lb_refTable->setAlignment(Qt::AlignCenter);
    tw_refData = new QTreeWidget();
    tw_refData->setColumnCount(5);
    QStringList sl;
    sl << "ID" << "Date" << tr("#( ") + QString(QChar(955)) + " )";
    sl << tr("Min( ") + QString(QChar(955)) + tr(" )");
    sl << tr("Max( ") + QString(QChar(955)) + tr(" )");
    tw_refData->setHeaderLabels(sl);
    tw_refData->setStyleSheet("background-color: white");
    tw_refData->header()->setSectionResizeMode(QHeaderView::ResizeToContents);

    tw_instruments->setMaximumWidth(200);
    tw_refData->setMinimumWidth(600);
    QGridLayout *widget_lyt = new QGridLayout();
    widget_lyt->addWidget(lb_instrument,  0, 0, 1, 1);
    widget_lyt->addWidget(lb_refTable,    0, 1, 1, 1);
    widget_lyt->addWidget(tw_instruments, 1, 0, 1, 1);
    widget_lyt->addWidget(tw_refData,     1, 1, 1, 1);

    QPushButton *pb_apply = new QPushButton();
    pb_apply->setText("Apply");
    pb_apply->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
    QPushButton *pb_cancel = new QPushButton();
    pb_cancel->setText("Cancel");
    pb_cancel->setIcon(this->style()->standardIcon(QStyle::SP_DialogCancelButton));

    QHBoxLayout *close_lyt = new QHBoxLayout();
    close_lyt->addStretch();
    close_lyt->addWidget(pb_cancel);
    close_lyt->addWidget(pb_apply);

    QVBoxLayout *main_lyt = new QVBoxLayout(this);
    main_lyt->addLayout(widget_lyt);
    main_lyt->addSpacing(10);
    main_lyt->addLayout(close_lyt);
    this->setLayout(main_lyt);

    connect(pb_cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_apply, SIGNAL(clicked()), this, SLOT(slt_apply()));
    connect(tw_instruments, SIGNAL(itemSelectionChanged()),
            this, SLOT(slt_set_refTable()));

}

void LoadDBWidget::slt_set_refTable(){
    tw_refData->clear();
    QTreeWidgetItem *item = tw_instruments->currentItem();
    int row = item->text(0).toInt() - 1;
    QVector<int> ids;
    ids << instrumentRefList.at(row);
    for (int i = 0; i < ids.size(); ++i){
        int refId = ids.at(i);
        QTreeWidgetItem *twi = new QTreeWidgetItem(static_cast<QTreeWidget *>(nullptr));
        twi->setText(0, QString::number(i + 1));
        twi->setText(1, refTable.at(refId).referenceTime.toString("MMM dd, yyyy"));
        twi->setText(2, QString::number(refTable.at(refId).nWavelength));
        twi->setText(3, QString::number(refTable.at(refId).startWavelength));
        twi->setText(4, QString::number(refTable.at(refId).stopWavelength));
        tw_refData->addTopLevelItem(twi);
    }
    return;
}

QDate LoadDBWidget::str2date(QString date){
    QRegExp re("\\d+:\\d+:\\d+");
    QStringList match = date.remove(re).simplified().split("-");
    if (match.size() == 3) {
        int year = match.at(0).toInt();
        int month = match.at(1).toInt();
        int day = match.at(2).toInt();
        return QDate(year, month, day);
    }else
        return QDate();
}

void LoadDBWidget::slt_apply(){
    QTreeWidgetItem *item_inst = tw_instruments->currentItem();
    int id_inst = item_inst->text(0).toInt() - 1;
    QVector<int> ids;
    ids << instrumentRefList.at(id_inst);
    QTreeWidgetItem *item_table = tw_refData->currentItem();
    int id_widgetItem = item_table->text(0).toInt() - 1;
    int id_refTable = ids.at(id_widgetItem);
    int refScanId = refTable.at(id_refTable).id;

    QDir tempDir = QDir::temp();
    QString fname("reference_scans_%1.aucrs");
    qint64 ms = QDateTime::currentMSecsSinceEpoch();
    QFileInfo finfo(tempDir, fname.arg(ms));
    QString fpath = finfo.absoluteFilePath();

    int db_read = db->readBlobFromDB(fpath, QString( "download_referenceScanData" ),
                                           refScanId );
    if (db_read == US_DB2::DBERROR){
        QMessageBox::warning(this, "Error", "Error processing file:\n"
                             + fpath + "\n" + db->lastError() +
                             "\n" + "Could not open file or no data \n");
    }else if (db_read != US_DB2::OK){
        QMessageBox::warning(this, "Error", "returned processing file:\n" +
                             fpath + "\n" + db->lastError() + "\n");
    }else{
        US_RefScanDataIO::RefData data;
        int referr = US_RefScanDataIO::readRefData(fpath, data);
        if (referr == US_RefScanDataIO::OK){
            refData->clear();
            refData->type[0] = data.type[0];
            refData->type[1] = data.type[1];
            refData->nWavelength = data.nWavelength;
            refData->nPoints = data.nPoints;
            refData->wavelength << data.wavelength;
            refData->xValues << data.xValues;
            refData->rValues << data.rValues;
            refData->std << data.std;
        }
    }
    QFile f(fpath);
    if (f.exists()){
        f.remove();
    }
    this->accept();
}

