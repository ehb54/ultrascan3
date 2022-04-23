#include "convert_scan.h"
#include "us_images.h"
#include "us_util.h"
#include "math.h"
#include "us_defines.h"

convertScan::convertScan() : US_Widgets()
{
    dbCon = new US_DB2();

    setWindowTitle( tr( "Convert Intensity to Pseudo-Absorbance" ) );
    setPalette( US_GuiSettings::frameColor() );

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    QVBoxLayout* left_layout = new QVBoxLayout();
    QVBoxLayout* right_layout = new QVBoxLayout();

    // Put the Run Info across the entire window
    QLabel* lb_runInfoInt  = us_banner(   tr( "Intensity Data Information" ) );
    pb_import = us_pushbutton(tr("Import Experimental Data"));
    QLabel* lb_runIdInt  = us_label(      tr( "Run ID:" ) );
    le_runIdInt          = us_lineedit(   "", 0, true );
    QLabel* lb_dirInt      = us_label(    tr( "Directory:" ) );
    le_dir              = us_lineedit( "", 0, true );
    QLabel* lb_desc  = us_label(    tr( "Description:" ), 0 );
    le_desc      = us_lineedit( "", 0, true);

    QGridLayout* runInfoIntLyt = new QGridLayout();
    runInfoIntLyt->addWidget(lb_dirInt,   0, 0, 1, 1);
    runInfoIntLyt->addWidget(le_dir,   0, 1, 1, 1);
    runInfoIntLyt->addWidget(lb_runIdInt, 1, 0, 1, 1);
    runInfoIntLyt->addWidget(le_runIdInt, 1, 1, 1, 1);
    runInfoIntLyt->addWidget(lb_desc,     2, 0, 1, 1);
    runInfoIntLyt->addWidget(le_desc,     2, 1, 1, 1);


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

    QGridLayout* mwlctrl_gl = new QGridLayout();
    mwlctrl_gl->addWidget(lb_lambstrt, 0, 0, 1, 1);
    mwlctrl_gl->addWidget(le_lambstrt, 0, 1, 1, 1);
    mwlctrl_gl->addWidget(lb_lambstop, 0, 2, 1, 1);
    mwlctrl_gl->addWidget(le_lambstop, 0, 3, 1, 1);

    mwlctrl_gl->addWidget(lb_lambplot, 1, 0, 1, 1);
    mwlctrl_gl->addWidget(cb_plot_id,  1, 1, 1, 1);
    mwlctrl_gl->addWidget(pb_prev_id,  1, 2, 1, 1);
    mwlctrl_gl->addWidget(pb_next_id,  1, 3, 1, 1);

    // Cell / Channel / Wavelength
    QLabel* lb_triple = us_banner(tr( "Cell / Channel / Wavelength" ), -1 );
    lw_triple = us_listwidget();
    QLabel* lb_ccw_info = us_label(tr("Manage Triples"));
    QPushButton* pb_del_ccw_item = us_pushbutton(tr("Delete Selected Item"));
    QPushButton* pb_sel_del_ccw = us_pushbutton(tr("Select and Drop CCWs"));
    QPushButton* pb_default_ccw = us_pushbutton(tr("Default"));
    QVBoxLayout* ccw_vbl = new QVBoxLayout();
    ccw_vbl->addWidget(lb_ccw_info);
    ccw_vbl->addWidget(pb_del_ccw_item);
    ccw_vbl->addWidget(pb_sel_del_ccw);
    ccw_vbl->addWidget(pb_default_ccw);
    ccw_vbl->addStretch(1);
    QHBoxLayout* ccw_hbl = new QHBoxLayout();
    ccw_hbl->addWidget(lw_triple);
    ccw_hbl->addLayout(ccw_vbl);

    QLabel* lb_runInfoAbs  = us_banner(   tr( "Absorbance Data Information" ) );
    diskDB_ctrl = new US_Disk_DB_Controls();
    pb_import_refScans = us_pushbutton(tr("Import Reference Data"), true, -1);
    pb_reset_refData = us_pushbutton(tr("Reset Reference Data"), false, -1);
    QHBoxLayout* rfs_lyt = new QHBoxLayout();
    rfs_lyt->addWidget(pb_import_refScans);
    rfs_lyt->addWidget(pb_reset_refData);

    QString ref_range("%1 Range:");
    QLabel* lb_ref_range = us_label(ref_range.arg(QChar(955)));
    le_ref_range = us_lineedit("");
    le_ref_range->setReadOnly(true);
    QLabel *lb_smooth = us_label("Smoothing Level:");
    ct_smooth = us_counter(1, 0, 10, 5);
    ct_smooth->setSingleStep(1);
    QHBoxLayout *ref_range_lyt = new QHBoxLayout();
    ref_range_lyt->addWidget(lb_ref_range);
    ref_range_lyt->addWidget(le_ref_range);
    ref_range_lyt->addWidget(lb_smooth);
    ref_range_lyt->addWidget(ct_smooth);

    QLabel* lb_runIdAbs  = us_label(      tr( "Run ID:" ) );
    le_runIdAbs          = us_lineedit(   "", 0, false );

    QGridLayout* runInfoAbsLyt = new QGridLayout();
    runInfoAbsLyt->addWidget(lb_runIdAbs, 0, 0, 1, 1);
    runInfoAbsLyt->addWidget(le_runIdAbs, 0, 1, 1, 1);

    QLabel *lb_buffer = us_label("Triple of the Buffer:");
    cb_buffer = us_comboBox();
    QHBoxLayout *buffer_lyt = new QHBoxLayout();
    buffer_lyt->addWidget(lb_buffer);
    buffer_lyt->addWidget(cb_buffer);

    ckb_shift_zero = new QCheckBox();
    QGridLayout *zero_min_lyt = us_checkbox("Shift Minimum to Zero",
                                                ckb_shift_zero);
    pb_pick_rp = us_pushbutton("Specify a Range", false);
    QHBoxLayout *shift_lyt = new QHBoxLayout();
    shift_lyt->addLayout(zero_min_lyt);
    shift_lyt->addWidget(pb_pick_rp);

    QLabel *lb_lower_x = us_label("Lower Range Value:");
    le_lower_x = us_lineedit("", 0, true);
    QLabel *lb_upper_x = us_label("Upper Range Value:");
    le_upper_x = us_lineedit("", 0, true);
    QHBoxLayout *xrange_lyt = new QHBoxLayout();
    xrange_lyt->addWidget(lb_lower_x);
    xrange_lyt->addWidget(le_lower_x);
    xrange_lyt->addWidget(lb_upper_x);
    xrange_lyt->addWidget(le_upper_x);

    QLabel* lb_status = us_label(tr("Status:"));
    le_status = us_lineedit(tr("(no data loaded)"), -1, true);
    QPalette stpal;
    stpal.setColor( QPalette::Text, Qt::white );
    stpal.setColor( QPalette::Base, Qt::blue  );
    le_status->setPalette( stpal );
    QHBoxLayout* status_l = new QHBoxLayout();
    status_l->addWidget(lb_status);
    status_l->addWidget(le_status);

    pb_save = us_pushbutton("Save Absorbance Data", false, 0 );
    pb_save->setDisabled(true);
    QPushButton* pb_close = us_pushbutton("Close", true, 0 );
    pb_reset = us_pushbutton("Reset", false, 0 );
    pb_reset->setDisabled(true);
    QHBoxLayout* closeLyt = new QHBoxLayout();
    closeLyt->addWidget(pb_reset);
    closeLyt->addWidget(pb_close);


    left_layout->addStretch(0);
    left_layout->setSpacing(5);
    right_layout->setSpacing(0);

    left_layout->addWidget(lb_runInfoInt);
    left_layout->addWidget(pb_import);
    left_layout->addLayout(runInfoIntLyt);
    left_layout->addWidget(lb_triple);
    left_layout->addLayout(ccw_hbl);
    left_layout->addWidget(lb_mwlctrl);
    left_layout->addLayout(mwlctrl_gl);

    left_layout->addWidget(lb_runInfoAbs);
    left_layout->addLayout(diskDB_ctrl);
    left_layout->addLayout(rfs_lyt);
    left_layout->addLayout(ref_range_lyt);
    left_layout->addLayout(buffer_lyt);
    left_layout->addLayout(shift_lyt);
    left_layout->addLayout(xrange_lyt);
    left_layout->addLayout(runInfoAbsLyt);
    left_layout->addWidget(pb_save);

    left_layout->addStretch(1);
    left_layout->addLayout(status_l);
    left_layout->addLayout(closeLyt);

    left_layout->addStretch(0);

//    //*****right*****//
//    plot_title = us_label("");

    plot_title = us_banner("");
    plot_title->setAlignment(Qt::AlignCenter);

    QwtText xLabel, yLabel;
    usplot_insty = new US_Plot( qwtplot_insty, tr( "" ),
                                tr( "Radius (in cm)" ), tr( "Intensity" ),
                                true, "", "rainbow" );
    qwtplot_insty->setMinimumSize( 700, 400 );
    qwtplot_insty->enableAxis( QwtPlot::xBottom, true );
    qwtplot_insty->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_insty->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_insty);

    usplot_abs = new US_Plot( qwtplot_abs, tr( "" ),
                              tr( "Radius (in cm)" ), tr( "Absorbance" ),
                              true, "", "rainbow" );
    qwtplot_abs->setMinimumSize( 700, 400 );
    qwtplot_abs->enableAxis( QwtPlot::xBottom, true );
    qwtplot_abs->enableAxis( QwtPlot::yLeft  , true );
    qwtplot_abs->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(qwtplot_abs);

    ct_lower = us_counter(3, 0, 0, 0);
    ct_lower->setSingleStep(1);
    ct_upper = us_counter(3, 0, 0, 0);
    ct_upper->setSingleStep(1);
    QLabel *lb_scan_ctrl = us_banner("Scans Control");
//    QPushButton *pb_exclude_scans = us_pushbutton("Exclude Scan(s)", false, -2 );
//    QPushButton *pb_include_all = us_pushbutton("Include All", false, -2 );
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
    scan_ctrl_lyt->addWidget(ct_lower,            1, 1, 1, 1);
    scan_ctrl_lyt->addWidget(pb_reset_curr_scans, 1, 2, 1, 2);
    scan_ctrl_lyt->addWidget(lb_scan_2,           2, 0, 1, 1);
    scan_ctrl_lyt->addWidget(ct_upper,            2, 1, 1, 1);
    scan_ctrl_lyt->addWidget(pb_reset_allscans,   2, 2, 1, 1);
    scan_ctrl_lyt->addWidget(pb_apply_allscans,   2, 3, 1, 1);

    right_layout->addWidget(plot_title);
    right_layout->addLayout(usplot_insty);
    right_layout->addLayout(usplot_abs);
    right_layout->addLayout(scan_ctrl_lyt);

    main_layout->addLayout(left_layout);
    main_layout->addLayout(right_layout);
    main_layout->setSpacing(1);
    main_layout->setMargin(1);
    setLayout(main_layout);

    picker = new US_PlotPicker(qwtplot_abs);
    picker->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );

    clear();

    connect(pb_import, SIGNAL(clicked()), this, SLOT(slt_import()));
    connect(pb_default_ccw, SIGNAL(clicked()), this, SLOT(slt_set_ccw_default()));
    connect(pb_del_ccw_item, SIGNAL(clicked()),
            this, SLOT(slt_del_item()));
    connect( pb_sel_del_ccw, SIGNAL( clicked() ),
            this, SLOT( slt_del_ccws() ) );

    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_prev_id,      SIGNAL(clicked()), this, SLOT(slt_prev_id()));
    connect(pb_next_id,      SIGNAL(clicked()), this, SLOT(slt_next_id()));

    connect(this, SIGNAL(sig_plot()), this, SLOT(slt_plot()));

//    connect(le_runIdAbs,   SIGNAL(textEdited(QString)),
//            this, SLOT(slt_edit_runid(QString)));
//    connect(le_runIdAbs, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(slt_cpos(int,int)));

    connect(pb_import_refScans, SIGNAL(clicked()), this, SLOT(slt_load_refScans()));
    connect(ckb_shift_zero, SIGNAL(stateChanged(int)), this, SLOT(slt_check_box(int)));

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

void convertScan::clear(void){
    xvalues.clear();
    intensity.clear();
    absorbance.clear();
    absorbance_buffer.clear();
    allIntData.clear();
    allIntDataFiles.clear();
    ccwList.clear();
    ccwListMain.clear();
    ccwStrListMain.clear();
    ccwItemList.clear();
    scans_range.clear();
    le_cursor_pos = -1;
    wavelength.clear();
    refData.clear();
    picker->disconnect();
    x_min_picked = -1;
    x_max_picked = -1;
    return;
}

void convertScan::slt_import(void){
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

    QFileInfoList fileList = inDir.entryInfoList();
    runID = QString();
    runType = QString();
    allIntData.clear();
    QStringList runTypesList;
    runTypesList << "RI";
    QRegularExpression re( "[^A-Za-z0-9_-]" );
    bool runID_changed = false;
    QString ccw_str("%1 / %2 / %3");
    for (int i = 0; i < fileList.size(); ++i){
        QString fname = fileList.at(i).fileName();
        QString rtp = fname.section(".", -5, -5);
        QString rid = fname.section(".", 0, -6);
        QRegularExpressionMatch match = re.match(rid);
        if (match.hasMatch()){
            QStringList match_list = match.capturedTexts();
            runID_changed = true;
            for (int j = 0; j < match_list.size(); ++j)
                rid = rid.replace(match_list.at(j), "_");
        }
        if (runID.isNull())
            runID = rid;
        else
            if (runID != rid){
                QMessageBox::warning( this,
                      tr( "Error" ),
                      tr( "Multiple runIDs found in the directory" ));
                return;
            }
        if (! runTypesList.contains(rtp)){
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
        allIntData << rdata;
        allIntDataFiles << fileList.at(i);

        QVector<int> scl;
        int ns = rdata.scanCount();
        scl << 0 << ns << ns;
        scans_range << scl;

        int cell = rdata.cell;
        char channel = rdata.channel;
        double wl = rdata.scanData.at(0).wavelength;
        ccwStrListMain << ccw_str.arg(cell).arg(channel).arg(wl);

        ccwListMain.index << i;
        ccwListMain.cell << cell;
        ccwListMain.channel << channel;
        ccwListMain.wavelength << wl;

        ccwList.index << i;
        ccwList.cell << cell;
        ccwList.channel << channel;
        ccwList.wavelength << wl;
    }
    qDebug() << runID;

    if ( runID_changed )
    {
       QMessageBox::warning( this,
             tr( "RunID Name Changed" ),
             tr( "The runID name has been changed. It may consist only"
                 "of alphanumeric \n"
                 " characters, the underscore, and the hyphen. New runID: " )
             + runID );
    }
    le_runIdInt->setText(runID);
    le_dir->setText(US_Settings::importDir());
    re.setPattern("-run[0-9]+$");
    int idre = runID.indexOf(re);
    QString s1 = runID.left(idre);
    QString s2 = runID.right(runID.size() - idre);
    le_runIdAbs->setText(s1.append("_Absorbance").append(s2));
    make_ccwItemList();
    set_widgetList();
    set_lambda_ctrl();
    pb_import->setDisabled(true);
    pb_reset->setEnabled(true);
    emit sig_save_button();
    return;
}

void convertScan::slt_load_refScans(void){
    if (diskDB_ctrl->db()){
        le_status->setText("db connection failed!");
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
        int error = refScanDataIO::readRefData(fname, refData);
        if (error != refScanDataIO::OK){
            refData.clear();
            QString mess = refScanDataIO::errorString(error);
            le_status->setText(mess);
        }
        else{
            pb_import_refScans->setDisabled(true);
            pb_reset_refData->setEnabled(true);
            emit sig_save_button();
        }
        QString text("%1 - %2 nm");
        int nw = refData.nWavelength;
        double w1 = refData.wavelength.at(0);
        double w2 = refData.wavelength.at(nw - 1);
        le_ref_range->setText(text.arg(w1).arg(w2));
        plot_refscan();
        plot_absorbance();
        return;
    }
}

void convertScan::load_from_DB(){
    QString error;
    dbCon->connect(pw.getPasswd(), error);
    if (dbCon->isConnected()){
        QStringList DB  = US_Settings::defaultDB();
        if (DB.isEmpty()){
            le_status->setText("Database: Undefined");
            return;
        }
        else
            le_status->setText("Database: " + DB.at(3));
    }else
        return;

    LoadDBWidget *db_w = new LoadDBWidget(this, dbCon, refData);
    int state_db = db_w->exec();
    if (state_db != QDialog::Accepted)
        return;
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


void convertScan::make_ccwItemList(){
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
    return;
}

void convertScan::set_lambda_ctrl(){
    cb_plot_id->disconnect();
    cb_plot_id->clear();
    intDataId.clear();
    wavelength.clear();
    lambda_id = 0;
    n_wavelengths = 0;
    if (lw_triple->count() == 0){
        le_lambstrt->setText("");
        le_lambstop->setText("");
        offon_prev_next();
        emit sig_plot();
        return;
    }
    int ccw_id = lw_triple->currentRow();
    n_wavelengths = ccwItemList.n_wl.at(ccw_id);
    intDataId = ccwItemList.index.at(ccw_id);
    wavelength = ccwItemList.wavelength.at(ccw_id);
    le_lambstrt->setText(QString::number(wavelength.at(0)));
    le_lambstop->setText(QString::number(wavelength.at(n_wavelengths - 1)));
    QStringList items;
    for (int i = 0; i < n_wavelengths; ++i)
        items << QString::number(wavelength.at(i));
    cb_plot_id->addItems(items);
    offon_prev_next();
    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_id(int)));
    slt_set_id(0);
    return;
}

void convertScan::slt_set_ccw_default(){
    ccwList.clear();
    ccwList.index << ccwListMain.index;
    ccwList.cell << ccwListMain.cell;
    ccwList.channel << ccwListMain.channel;
    ccwList.wavelength << ccwListMain.wavelength;
    make_ccwItemList();
    set_widgetList();
    set_lambda_ctrl();
    emit sig_save_button();
    return;
}

void convertScan::set_widgetList(){
    lw_triple->disconnect();
    lw_triple->clear();
    if (ccwItemList.size() == 0)
        return;
    QString item("%1 / %2 / %3-%4 (%5)");
    for (int i = 0; i < ccwItemList.size(); ++i){
        int cell = ccwItemList.cell.at(i);
        char channel = ccwItemList.channel.at(i);
        double min_wl = ccwItemList.min_wl.at(i);
        double max_wl = ccwItemList.max_wl.at(i);
        int nwl = ccwItemList.n_wl.at(i);
        QString item_i = item.arg(cell).arg(channel)
                .arg(min_wl).arg(max_wl).arg(nwl);
        lw_triple->addItem(item_i);
    }
    lw_triple->setCurrentRow(0);
    set_buffer_list();
    connect( lw_triple, SIGNAL( currentRowChanged(int) ),
            this, SLOT( slt_new_lambda_ctrl(int) ) );
    return;
}

void convertScan::set_buffer_list(){
    cb_buffer->disconnect();
    cb_buffer->clear();
    QStringList list;
    list << "None";
    for (int i = 0; i < lw_triple->count(); ++i){
        list << lw_triple->item(i)->text();
    }
    cb_buffer->addItems(list);
    cb_buffer->setCurrentIndex(0);
    absorbance_buffer.clear();
    connect(cb_buffer, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slt_update_buffer(int)));
    return;
}

void convertScan::slt_del_item(){
    le_desc->setText("");
    if (lw_triple->count() == 0)
        return;
    int row = lw_triple->currentRow();
    ccwItemList.index.removeAt(row);
    ccwItemList.cell.removeAt(row);
    ccwItemList.channel.removeAt(row);
    ccwItemList.wavelength.removeAt(row);
    ccwItemList.min_wl.removeAt(row);
    ccwItemList.max_wl.removeAt(row);
    ccwItemList.n_wl.removeAt(row);
    set_widgetList();
    set_lambda_ctrl();
    emit sig_save_button();
    return;
}

void convertScan::slt_del_ccws(){
    QStringList ccwStrList;
    QStringList exludeList;
    QString ccw_str("%1 / %2 / %3");
    for (int i = 0; i < ccwItemList.size(); ++i){
        int cell = ccwItemList.cell.at(i);
        char channel = ccwItemList.channel.at(i);
        QVector<double> wavelength = ccwItemList.wavelength.at(i);
        for (int j = 0; j < wavelength.size(); ++j){
            double wl = wavelength.at(j);
            ccwStrList << ccw_str.arg(cell).arg(channel).arg(wl);
            exludeList << ccw_str.arg(cell).arg(channel).arg(wl);
        }
    }

    US_SelectTriples* seldiag = new US_SelectTriples( exludeList );
    seldiag->show();
    int code = seldiag->exec();
    if (code == 0)
        return;
    int id;
    for (int i = 0; i < exludeList.size(); ++i){
        id = ccwStrList.indexOf(exludeList.at(i));
        ccwStrList.removeAt(id);
    }

    ccwList.clear();
    for (int i = 0; i < ccwStrList.size(); ++i){
        id = ccwStrListMain.indexOf(ccwStrList.at(i));
        ccwList.cell << ccwListMain.cell.at(id);
        ccwList.channel << ccwListMain.channel.at(id);
        ccwList.index << ccwListMain.index.at(id);
        ccwList.wavelength << ccwListMain.wavelength.at(id);
    }
    make_ccwItemList();
    set_widgetList();
    set_lambda_ctrl();

    return;
}

void convertScan::slt_new_lambda_ctrl(int){
    set_lambda_ctrl();
    return;
}

void convertScan::slt_reset(){
    clear();
    le_ref_range->setText("");
    le_lambstrt->setText("");
    le_lambstop->setText("");
    le_runIdAbs->setText("");
    le_runIdInt->setText("");
    le_dir->setText("");
    le_desc->setText("");
    le_lower_x->setText("");
    le_upper_x->setText("");
    set_widgetList();
    set_lambda_ctrl();
    pb_reset->setDisabled(true);
    pb_import->setEnabled(true);
    pb_import_refScans->setEnabled(true);
    pb_reset_refData->setDisabled(true);
    pb_save->setDisabled(true);
    ckb_shift_zero->setCheckState(Qt::Unchecked);
    return;
}

void convertScan::slt_set_id(int id){
    lambda_id = id;
    offon_prev_next();
    le_desc->setText(allIntData.at(id).description);
    set_scan_range();
    emit sig_plot();
    return;
}

void convertScan::slt_prev_id(void){
    --lambda_id;
    cb_plot_id->setCurrentIndex(lambda_id);
    return;
}

void convertScan::slt_next_id(void){
    ++lambda_id;
    cb_plot_id->setCurrentIndex(lambda_id);
    return;
}

void convertScan::slt_plot(){
    if (lw_triple->count() == 0)
        plot_title->setText("");
    else{
        QString title("Wavelength= %1 nm");
        double wl = wavelength.at(lambda_id);
        plot_title->setText(title.arg(wl));
    }
    plot_intensity();
    plot_refscan();
    plot_absorbance();
    return;
}

void convertScan::offon_prev_next(){
    pb_prev_id->setDisabled(lambda_id <= 0);
    pb_next_id->setDisabled(lambda_id >= (n_wavelengths - 1));
    return;

}

//void convertScan::slt_cpos(int oldPos, int newPos ){
//    cpos = newPos;
//    qDebug() << cpos;
//    return;
//}

//void convertScan::slt_edit_runid(QString text){
//    le_runIdAbs->disconnect();
//    QRegularExpression re;
//    re.setPattern("[^a-zA-Z0-9-]+");
//    QRegularExpressionMatch match = re.match(text);
//    if (match.hasMatch()){
//        text.replace(re, "");
////        --cpos;
//    }else
//        ++cpos;
//    le_runIdAbs->setText(text);
//    le_runIdAbs->setCursorPosition(cpos);
//    connect(le_runIdAbs,   SIGNAL(textEdited(QString)),
//            this, SLOT(slt_edit_runid(QString)));
//    connect(le_runIdAbs, SIGNAL(cursorPositionChanged(int,int)), this, SLOT(slt_cpos(int,int)));
//    return;
//}

void convertScan::plot_intensity(void){
    qwtplot_insty->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (lw_triple->count() == 0){
        grid = us_grid(qwtplot_insty);
        qwtplot_insty->replot();
        return;
    }

    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(lambda_id);
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

void convertScan::plot_refscan(void){    
    if (refData.xValues.size() == 0 || lw_triple->count() == 0){
        refId = -1;
        return;
    }
    if (intensity.size() == 0)
        return;

    QPen pen = QPen( QBrush( Qt::white ), 2.0 );
    double min_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double max_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).upperBound();

    if (! get_refId(wavelength.at(lambda_id))){
        le_status->setText("reference data: not found lambda!");
        return;
    }

    const double *xp = refData.xValues.data();
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

void convertScan::plot_absorbance(void){
    qwtplot_abs->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (refData.xValues.size() == 0 ||
            lw_triple->count() == 0 || refId == -1){
        grid = us_grid(qwtplot_abs);
        qwtplot_abs->replot();
        return;
    }

    double min_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    double max_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).upperBound();
    double min_r  =  1e20;
    double max_r  = -1e20;

    int row = lw_triple->currentRow();
    int dataId = ccwItemList.index.at(row).at(lambda_id);

    get_absorbance(refId, dataId, false);

    int id_buff = cb_buffer->currentIndex();
    if (id_buff > 0){
        int dataId_buff = ccwItemList.index.at(id_buff - 1).at(lambda_id);
        get_relative_absorbance(dataId_buff);
    }
    shift_absorbance();

    int scan_l1 = scans_range.at(dataId).at(0);
    int scan_l2 = scans_range.at(dataId).at(1);
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
        curve->setSamples(xp, rp, refData.nPoints);
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

void convertScan::get_intensity(int data_id){
    intensity.clear();
    int scan_l1 = scans_range.at(data_id).at(0);
    int scan_l2 = scans_range.at(data_id).at(1);
    xvalues.clear();
    xvalues << allIntData.at(data_id).xvalues;
    for (int i = scan_l1; i < scan_l2; ++i)
        intensity << allIntData.at(data_id).scanData.at(i);
    return;
}


void convertScan::get_absorbance(int id_ref, int id_data, bool buffer){
    if (buffer)
        absorbance_buffer.clear();
    else
        absorbance.clear();
    const double *xp_ref = refData.xValues.data();
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
                    qDebug() << "incompatible steps";
                    le_status->setText("incompatible steps");
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
            absorbance_buffer << scan;
        else
            absorbance << scan;
        rval_abs.clear();
    }
    return;
}

void convertScan::set_scan_range(){
    ct_lower->disconnect();
    ct_upper->disconnect();
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(lambda_id);
    int lower = scans_range.at(index).at(0);
    int upper = scans_range.at(index).at(1);
    int max = scans_range.at(index).at(2);
    ct_lower->setRange(0, max);
    ct_lower->setValue(lower);
    ct_upper->setRange(0, max);
    ct_upper->setValue(upper);
    connect(ct_lower, SIGNAL(valueChanged(double)),
            this, SLOT(slt_new_scan_range(double)));
    connect(ct_upper, SIGNAL(valueChanged(double)),
            this, SLOT(slt_new_scan_range(double)));
    return;
}

void convertScan::slt_new_scan_range(double){
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(lambda_id);
    int lower = qRound(ct_lower->value());
    int upper = qRound(ct_upper->value());
    scans_range[index][0] = lower;
    scans_range[index][1] = upper;
    emit sig_plot();
    return;
}

void convertScan::slt_check_box(int state){
    x_min_picked = -1;
    x_max_picked = -1;
    le_lower_x->setText("");
    le_upper_x->setText("");
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

void convertScan::slt_pick_point(){
    x_min_picked = -1;
    x_max_picked = -1;
    le_lower_x->setText("");
    le_upper_x->setText("");
    if (absorbance.size() == 0)
        return;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    connect(picker, SIGNAL(cMouseUp(const QwtDoublePoint&)),
            this  , SLOT(slt_mouse(const QwtDoublePoint&)));
    emit sig_plot();
    emit sig_save_button();
    return;
}

void convertScan::slt_mouse(const QwtDoublePoint& point){
    double x = point.x();
    int np = xvalues.size();
    double min_x = xvalues.at(0);
    double max_x = xvalues.at(np - 1);
    if (x > min_x && x < max_x){
        if (x_min_picked == -1){
            x_min_picked = x;
            le_lower_x->setText(QString::number(x, 'f', 3));
            emit sig_plot();
        } else {
            x_max_picked = x;
            picker->disconnect();
            le_upper_x->setText(QString::number(x, 'f', 3));
            pb_pick_rp->setStyleSheet("QPushButton { background-color: green }");
            emit sig_plot();
        }
    }else{
        QString mess("Pick a point between the minimum and maximum"
                     "values of the radial points.\n"
                     "Minimum= %1, Maximum= %2");
        QMessageBox::warning( this, tr( "Warning" ), mess.arg(min_x).arg(max_x));
    }
    emit sig_save_button();
    return;
}

void convertScan::slt_reset_allscans(){
    if (scans_range.size() == 0 || lw_triple->count() == 0)
        return;
    for (int i = 0; i < scans_range.size(); ++i){
        scans_range[i][0] = 0;
        scans_range[i][1] = scans_range.at(i).at(2);
    }
    set_scan_range();
    emit sig_plot();
    return;
}

void convertScan::slt_apply_allscans(){
    if (scans_range.size() == 0 || lw_triple->count() == 0)
        return;
    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(lambda_id);
    int lower = scans_range[index][0];
    int upper = scans_range[index][1];
    for (int i = 0; i < scans_range.size(); ++i){
        int max = scans_range.at(i).at(2);
        if (lower <= max && upper <= max){
            scans_range[i][0] = lower;
            scans_range[i][1] = upper;
        }
    }
}

void convertScan::slt_reset_scans(){
    if (scans_range.size() == 0 || lw_triple->count() == 0)
        return;

    int row = lw_triple->currentRow();
    int index = ccwItemList.index.at(row).at(lambda_id);
    scans_range[index][0] = 0;
    scans_range[index][1] = scans_range.at(index).at(2);
    set_scan_range();
    emit sig_plot();
    return;
}

void convertScan::slt_reset_refData(){
    refId = -1;
    refData.clear();
    le_ref_range->setText("");
    pb_import_refScans->setEnabled(true);
    pb_reset_refData->setDisabled(true);
    pb_save->setDisabled(true);
    emit sig_plot();
    return;
}

void convertScan::slt_save_avail(void){
    if (lw_triple->count() > 0 && refData.nWavelength > 0){
        if (ckb_shift_zero->isChecked()){
            if (x_min_picked != -1 && x_max_picked != -1)
                pb_save->setEnabled(true);
            else
                pb_save->setDisabled(true);
        }else
            pb_save->setEnabled(true);
    }else
        pb_save->setDisabled(true);
    return;
}

void convertScan::slt_save(void){
    QString runIdOld = le_runIdAbs->text();
    QString runId(runIdOld);
    QRegularExpression re;
    re.setPattern("[^a-zA-Z0-9_-]+");
    bool flag = false;
    QRegularExpressionMatch match = re.match(runId);
    if (match.hasMatch()){
        runId.replace(re, "");
        flag = true;
    }
    if (flag)
        QMessageBox::warning( this,
              tr( "Absorbance RunID Changed" ),
              tr( "The runID name has been changed. It may consist only"
                  "of alphanumeric \n"
                  " characters, the underscore, and the hyphen. New runID: " )
              + runID );
    QDir dir = QDir(le_dir->text());
    if (dir.cd(runId)){
        QString absPath = dir.absolutePath();
        int ck = QMessageBox::question(this, tr( "Warning!" ),
                                       tr( "Output directory exists!\n" ) + runID +
                                       tr( "\n\n Do you really want to replace it?"));
        if (ck == QMessageBox::Yes){
            dir.removeRecursively();
            dir.mkpath(absPath);
            dir.setPath(absPath);
        }else return;
    }else{
        dir.mkdir(runId);
        dir.cd(runId);
    }
    dir.makeAbsolute();
    qDebug() << dir.path();
    int nwl_tot = 0;
    int nrows = lw_triple->count();
    for (int i = 0; i < nrows; ++i)
        nwl_tot += ccwItemList.n_wl.at(i);

    int id_buff = cb_buffer->currentIndex();
    bool rm_buffer = false;
    if (id_buff > 0)
        rm_buffer = true;

    int n = 1;
    status = tr("Writting: %1 %2");
    QString fileName("%1.RA.%2.%3.%4.auc");
    for (int i = 0; i < nrows; ++i){
        for (int j = 0; j < ccwItemList.n_wl.at(i); ++j){
            percent = QString::number(100.0 * n / nwl_tot, 'f', 1);
            le_status->setText(status.arg(percent).arg(QChar(37)));
            qApp->processEvents();
            n++;
            int dataId = ccwItemList.index.at(i).at(j);
            double wavelength = ccwItemList.wavelength.at(i).at(j);
            if (! get_refId(wavelength)){
                qDebug() << tr("Not found corrosponding reference data for: ") <<
                            allIntDataFiles.at(dataId).fileName();
                continue;
            }

            get_absorbance(refId, dataId, false);
            if (rm_buffer){
                int dataId_buff = ccwItemList.index.at(id_buff - 1).at(j);
                get_relative_absorbance(dataId_buff);
            }
            shift_absorbance();
            int scan_l1 = scans_range.at(dataId).at(0);
            int scan_l2 = scans_range.at(dataId).at(1);
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
            QString fn = fileName.arg(runId).arg(cell).arg(channel).arg(wavelength);
            QFileInfo fileInfo(dir, fn);
            US_DataIO::writeRawData(fileInfo.absoluteFilePath(), rawData);
        }
    }
    le_status->setText("Done !");

    return;
}

bool convertScan::get_refId(double wavelength){
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

void convertScan::slt_update_buffer(int){
    emit sig_plot();
    return;
}

void convertScan::get_relative_absorbance(int bufferId){
    get_absorbance(refId, bufferId, true);

    if (absorbance_buffer.size() == 0)
        return;
    if (absorbance.size() != absorbance_buffer.size()){
        QMessageBox::warning( this,
              tr( "Error" ),
              tr( "Mismatch nscan" ));
        return;
    }
    int smooth_l = ct_smooth->value();
    if (smooth_l > 0){
        for (int i = 0; i < absorbance_buffer.size(); ++i){
            QVector<double> rval =  absorbance_buffer.at(i).rvalues;
            QVector<double> rval_s = get_smooth(rval, smooth_l);
            absorbance_buffer[i].rvalues.clear();
            absorbance_buffer[i].rvalues << rval_s;
        }
    }
    for (int i = 0; i < absorbance.size(); ++i){
        int np = absorbance.at(i).rvalues.size();
        int np_b = absorbance_buffer.at(i).rvalues.size();
        if (np != np_b){
            QMessageBox::warning( this,
                  tr( "Error" ),
                  tr( "Mismatch npoints" ));
            return;
        }
        double *rp = absorbance[i].rvalues.data();
        const double *rbp = absorbance_buffer.at(i).rvalues.data();
        for (int j = 0; j < np; ++j)
            if (rp[j] < abs_max - 0.1)
                rp[j] -= rbp[j];
        for (int j = 0; j < np; ++j){
            if (rp[j] > abs_max)
                rp[j] = abs_max;
            else if (rp[j] < -abs_max)
                rp[j] = -abs_max;
        }
    }
    return;
}

void convertScan::shift_absorbance(){
    if (x_min_picked == -1 || x_max_picked == -1)
        return;
    const double *xp = xvalues.data();
    int id_min = -1;
    int id_max = -1;
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
    for (int i = 0; i < absorbance.size(); ++i){
        int np = absorbance.at(i).rvalues.size();
        double *rp = absorbance[i].rvalues.data();
        double min_r =  1e20;
        for (int j = id_min; j < id_max; ++j)
            min_r = qMin(min_r, rp[j]);
        for (int j = 0; j < np; ++j)
            rp[j] -= min_r;
    }

    return;
}

QVector<double> convertScan::get_smooth(QVector<double> values, int winlen){
    int np = values.size();
    QVector<double> values_smooth(np, 0);
    const double *rp = values.data();
    double *rsp = values_smooth.data();
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
            if (rp[i + j] > (abs_max - 0.1)){
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
    return values_smooth;
}

void convertScan::slt_update_smooth(double){
    emit sig_plot();
    return;
}


LoadDBWidget::LoadDBWidget(QWidget* w, US_DB2 *dbCon,
                           refScanDataIO::RefData &refDataIn): US_WidgetsDialog(w, Qt::Dialog)
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

QDate LoadDBWidget::str2date(QString date_str){
    QRegularExpression re_d("^(\\d+)-(\\d+)-(\\d+)");
    QRegularExpressionMatch match = re_d.match(date_str);
    QStringList match_list = match.capturedTexts();
    int day = 0, month = 0, year = 0;
    if (match_list.size() > 0) {
        year = match_list.at(1).toInt();
        month = match_list.at(2).toInt();
        day = match_list.at(3).toInt();
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
        refScanDataIO::RefData data;
        int referr = refScanDataIO::readRefData(fpath, data);
        if (referr == refScanDataIO::OK){
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
