#include "us_convert_scan.h"
#include "us_load_auc.h"
#include "us_csv_loader.h"
#include <QGuiApplication>

US_ConvertScan::US_ConvertScan() : US_Widgets()
{
    setPalette( US_GuiSettings::frameColorDefault() );
    font = QFont( US_Widgets::fixedFont().family(), US_GuiSettings::fontSize() );
    // font.setPointSize(font.pointSize() - 1);

    disk_controls = new US_Disk_DB_Controls;
    QLabel* lb_runInfoInt  = us_banner("Intensity Data Control");
    pb_import = us_pushbutton("Import Run");
    pb_reset = us_pushbutton("Reset");
    pb_load_ref = us_pushbutton("Load Reference Scan");

    QHBoxLayout *imp_lyt_1 = new QHBoxLayout();
    imp_lyt_1->addWidget(pb_import);
    imp_lyt_1->addWidget(pb_load_ref);
    imp_lyt_1->addWidget(pb_reset);

    QLabel* lb_runIdInt = us_label("Run ID");
    lb_runIdInt->setAlignment(Qt::AlignCenter);
    le_runid = us_lineedit("", 0, true );
    QLabel* lb_desc  = us_label("Description", 0 );
    lb_desc->setAlignment(Qt::AlignCenter);
    le_desc      = us_lineedit( "", 0, true);

    QGridLayout *imp_lyt_2 = new QGridLayout();
    imp_lyt_2->addWidget(lb_runIdInt,  0, 0, 1, 1);
    imp_lyt_2->addWidget(le_runid,  0, 1, 1, 3);
    imp_lyt_2->addWidget(lb_desc,      1, 0, 1, 1);
    imp_lyt_2->addWidget(le_desc,      1, 1, 1, 3);

    QVBoxLayout* import_lyt = new QVBoxLayout();
    import_lyt->addWidget(lb_runInfoInt);
    import_lyt->addLayout(disk_controls);
    import_lyt->addLayout(imp_lyt_1);
    import_lyt->addLayout(imp_lyt_2);

    // Cell / Channel / Wavelength
    int width_lb = 150;
    int width_ct = 150;
    QLabel* lb_buffer = us_banner("Absorbance Data Control");

    QLabel* lb_scans = us_label("# Scans", 0);
    lb_scans->setAlignment(Qt::AlignCenter);
    lb_scans->setFixedWidth(width_lb);

    // QLabel *lb_smooth    = us_label("Smooth", 0);
    // lb_smooth->setAlignment(Qt::AlignCenter);
    // lb_smooth->setFixedWidth(width_lb);

    QLabel *lb_maxod    = us_label("Max OD", 0);
    lb_maxod->setAlignment(Qt::AlignCenter);
    lb_maxod->setFixedWidth(width_lb);

    pb_apply = us_pushbutton("Apply");

    nscans = 1;
    max_nscans = 1000;
    ct_scans = us_counter(2, 1, 1, nscans);
    ct_scans->setSingleStep(1);
    ct_scans->setFixedWidth(width_ct);
    align_center(ct_scans);

    smooth = 0;
    // ct_smooth = us_counter(2, 0, 10, smooth);
    // ct_smooth->setSingleStep(1);
    // ct_smooth->setFixedWidth(width_ct);
    // align_center(ct_smooth);

    ct_maxod = us_counter(2, 0.1, 10, 2.0);
    ct_maxod->setSingleStep(0.1);
    // ct_maxod->setFixedWidth(width_ct);
    align_center(ct_maxod);

    QLabel* lb_scan_from = us_label("Scans From");
    lb_scan_from->setAlignment(Qt::AlignCenter);
    QLabel* lb_scan_to =   us_label("Scans To");
    lb_scan_to->setAlignment(Qt::AlignCenter);

    ct_scan_from = us_counter(2, 1, 1, 1);
    ct_scan_from->setSingleStep(1);
    align_center(ct_scan_from);

    ct_scan_to = us_counter(2, 1, 1, 1);
    ct_scan_to->setSingleStep(1);
    align_center(ct_scan_to);

    QGridLayout* scan_lyt = new QGridLayout();
    scan_lyt->addWidget(lb_scans,     0, 0, 1, 1);
    scan_lyt->addWidget(ct_scans,     0, 1, 1, 1);
    // scan_lyt->addWidget(lb_smooth, 1, 0, 1, 1);
    // scan_lyt->addWidget(ct_smooth, 1, 1, 1, 1);
    scan_lyt->addWidget(pb_apply,     0, 2, 1, 1);
    scan_lyt->addWidget(lb_scan_from, 1, 0, 1, 1);
    scan_lyt->addWidget(ct_scan_from, 1, 1, 1, 2);
    scan_lyt->addWidget(lb_scan_to,   2, 0, 1, 1);
    scan_lyt->addWidget(ct_scan_to,   2, 1, 1, 2);
    scan_lyt->addWidget(lb_maxod,     3, 0, 1, 1);
    scan_lyt->addWidget(ct_maxod,     3, 1, 1, 2);

    tb_triple = new QTableWidget();
    tb_triple->setRowCount(0);
    tb_triple->setColumnCount(4);
    tb_triple-> setHorizontalHeaderLabels(QStringList{"Cell/Channel", "Lambda", "Reference", "Save"});
    tb_triple->setStyleSheet("QTableWidget { background-color: white; }");
    QHeaderView* header = tb_triple->horizontalHeader();
    header->setFont(font);
    header->setSectionResizeMode(QHeaderView::Fixed);
    header->resizeSection(header->logicalIndex(0), 130);
    header->resizeSection(header->logicalIndex(1), 145);
    header->resizeSection(header->logicalIndex(2), 123);
    header->resizeSection(header->logicalIndex(3), 50);

    tb_triple->verticalHeader()->setVisible(false);

    QVBoxLayout* ccw_lyt = new QVBoxLayout();
    ccw_lyt->addWidget(lb_buffer);
    ccw_lyt->addLayout(scan_lyt);
    ccw_lyt->addSpacing(1);
    ccw_lyt->addWidget(tb_triple);

    static QChar clambda( 955 );   // Lambda character
    QLabel* lb_lambstrt = us_label(tr( "%1 Start" ).arg( clambda ) );
    lb_lambstrt->setAlignment(Qt::AlignCenter);
    QLabel* lb_lambstop = us_label(tr( "%1 End" ).arg( clambda ) );
    lb_lambstop->setAlignment(Qt::AlignCenter);

    le_lambstrt = us_lineedit("", -1, true);
    le_lambstop = us_lineedit("", -1, true);

    cb_plot_id  = us_comboBox();
    cb_plot_id->setEditable(true);
    cb_plot_id->lineEdit()->setAlignment(Qt::AlignCenter);
    cb_plot_id->lineEdit()->setReadOnly(true);

    pb_prev_id  = us_pushbutton( "Previous", true, 0 );
    pb_next_id  = us_pushbutton( "Next",     true, 0 );
    pb_prev_id->setIcon( US_Images::getIcon( US_Images::ARROW_LEFT  ) );
    pb_next_id->setIcon( US_Images::getIcon( US_Images::ARROW_RIGHT ) );
    pb_prev_id->setFixedWidth(150);
    pb_next_id->setFixedWidth(150);

    QLabel *lb_zero = us_label("Shift to Zero");
    lb_zero->setAlignment(Qt::AlignCenter);
    pb_pick_rp = us_pushbutton("Pick Region");
    QPushButton *pb_default = us_pushbutton("Default");
    pb_save = us_pushbutton("Save Absorbance Data");

    QGridLayout* save_lyt = new QGridLayout();
    save_lyt->addWidget(pb_prev_id,   0, 1, 1, 2);
    save_lyt->addWidget(pb_next_id,   0, 3, 1, 2);

    save_lyt->addWidget(lb_lambstrt,  1, 0, 1, 1);
    save_lyt->addWidget(le_lambstrt,  1, 1, 1, 1);
    save_lyt->addWidget(cb_plot_id,   1, 2, 1, 2);
    save_lyt->addWidget(lb_lambstop,  1, 4, 1, 1);
    save_lyt->addWidget(le_lambstop,  1, 5, 1, 1);

    save_lyt->addWidget(lb_zero,      2, 0, 1, 2);
    save_lyt->addWidget(pb_pick_rp,   2, 2, 1, 2);
    save_lyt->addWidget(pb_default,   2, 4, 1, 2);

    save_lyt->addWidget(pb_save,      3, 1, 1, 4);

    for (int ii = 0; ii < save_lyt->columnCount(); ii++) {
        save_lyt->setColumnMinimumWidth(ii, 75);
    }

    QLabel* lb_status = us_label("Status:");
    lb_status->setAlignment(Qt::AlignCenter);
    le_status = us_lineedit("", -1, true);
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
    left_lyt->setSpacing(1);
    left_lyt->addLayout(import_lyt);
    left_lyt->addSpacing(3);
    left_lyt->addLayout(ccw_lyt);
    left_lyt->addSpacing(3);
    left_lyt->addLayout(save_lyt);
    left_lyt->addSpacing(3);
    left_lyt->addLayout(status_lyt);
    left_lyt->addLayout(close_lyt);
    left_lyt->setSpacing(1);
    left_lyt->setMargin(0);

    QFrame* frm_left = new QFrame();
    frm_left->setLayout(left_lyt);
    frm_left->setFrameShadow(QFrame::Plain);
    frm_left->setFrameShape(QFrame::NoFrame);
    frm_left->setMaximumWidth(450);

    //    //*****right*****//
    //    plot_title = us_label("");

    plot_title = us_label("");
    plot_title->setAlignment(Qt::AlignCenter);
    plot_title->setStyleSheet("background-color: white;"
                              "color:black; font-size: 11pt");

    chkb_abs_int = new QCheckBox();
    QGridLayout* chkb_lyt = us_checkbox("Plot Both Intensity and Reference scans", chkb_abs_int);
    chkb_lyt->setAlignment(Qt::AlignCenter);

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

    QVBoxLayout* right_lyt = new QVBoxLayout();
    right_lyt->setSpacing(0);
    right_lyt->addWidget(plot_title);
    right_lyt->addLayout(chkb_lyt);
    right_lyt->addLayout(usplot_insty);
    right_lyt->addLayout(usplot_abs);

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    main_lyt->addWidget(frm_left, 0);
    main_lyt->addLayout(right_lyt, 1);
    main_lyt->setSpacing(1);
    main_lyt->setMargin(1);
    setLayout(main_lyt);

    picker_abs = new US_PlotPicker(qwtplot_abs);
    picker_abs->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker_abs->setMousePattern( QwtEventPattern::MouseSelect1,
                                Qt::LeftButton, Qt::ControlModifier );
    picker_insty = new US_PlotPicker(qwtplot_insty);

    reset();

    connect(this, &US_ConvertScan::sig_plot, this, &US_ConvertScan::plot_all);
    connect(pb_import, &QPushButton::clicked, this, &US_ConvertScan::import_run);
    connect(pb_load_ref, &QPushButton::clicked, this, &US_ConvertScan::load_ref_scan);
    connect(pb_reset, &QPushButton::clicked, this, &US_ConvertScan::reset);
    connect(pb_prev_id, &QPushButton::clicked, this, &US_ConvertScan::prev_id);
    connect(pb_next_id, &QPushButton::clicked, this, &US_ConvertScan::next_id);
    connect(pb_save, &QPushButton::clicked, this, &US_ConvertScan::save_run);
    connect(pb_pick_rp, &QPushButton::clicked, this, &US_ConvertScan::pick_region);
    connect(pb_apply, &QPushButton::clicked, this, &US_ConvertScan::apply_nscans);
    connect(pb_default, &QPushButton::clicked, this, &US_ConvertScan::default_region);
    // connect(ct_smooth, &QwtCounter::valueChanged, this, &US_ConvertScan::update_scan_smooth);
    connect(ct_maxod, &QwtCounter::valueChanged, this, &US_ConvertScan::plot_absorbance);
    connect(chkb_abs_int, &QCheckBox::stateChanged, this, &US_ConvertScan::plot_ref_state);
}

void US_ConvertScan::plot_ref_state() {
    plot_intensity();
    plot_refscan();
}

void US_ConvertScan::align_center(QwtCounter* ct) {
    QLineEdit *le = ct->findChild<QLineEdit*>();
    if (le) {
        le->setAlignment(Qt::AlignCenter);
    }
}

void US_ConvertScan::reset(){
    picker_abs->disconnect();
    tb_triple->disconnect();
    tb_triple->clearContents();
    tb_triple->setRowCount(0);
    set_ct_scans();
    hasData = false;
    ccw_items.clear();
    intensity_data.clear();
    absorbance_data.clear();
    absorbance_state.clear();
    absorbance_shifts.clear();
    refscan_data.clear();
    refscan_files.clear();
    le_lambstrt->clear();
    le_lambstop->clear();
    le_runid->clear();
    le_desc->clear();
    set_wavl_ctrl();
}

void US_ConvertScan::update_nscans() {
    pb_apply->setStyleSheet("QPushButton { background-color: red }");
}

void US_ConvertScan::apply_nscans() {
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    QString qs = "QPushButton { background-color: %1 }";
    pb_apply->setStyleSheet(qs.arg(color.name()));
    nscans = static_cast<int>(ct_scans->value());
    // smooth = static_cast<int>(ct_smooth->value());
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    for (int ii = 0; ii < ccw_items.size(); ii++) {
        calc_absorbance(ii);
    }
    QGuiApplication::restoreOverrideCursor();

    ct_scan_from->disconnect();
    ct_scan_to->disconnect();
    double cv = ct_scans->value();
    ct_scan_from->setMaximum(cv);
    ct_scan_to->setMaximum(cv);
    if (ct_scan_from->value() > cv) {
        ct_scan_from->setValue(cv);
    }
    if (ct_scan_to->value() > cv) {
        ct_scan_to->setValue(cv);
    }
    connect(ct_scan_from, &QwtCounter::valueChanged, this, &US_ConvertScan::lower_scan_range);
    connect(ct_scan_to, &QwtCounter::valueChanged, this, &US_ConvertScan::upper_scan_range);

    emit sig_plot();
}

void US_ConvertScan::load_ref_scan() {
    QString filter = "Text Files (*.csv *.dat);;All Files (*)";
    QString fpath = QFileDialog::getOpenFileName(this, "Load Reference Scans",
                                                 US_Settings::dataDir(), filter);
    if (fpath.isEmpty()) {
        return;
    }

    for (int ii = 0; ii < refscan_files.size(); ii++) {
        QString file_loaded = refscan_files.at(ii).filename;
        if (file_loaded.compare(fpath) == 0) {
            QMessageBox::warning(this, "Error!", "The following file has already been uploaded!\n" + fpath);
            return;
        }
    }


    QString note = "A Header Instance: cm;220nm;222nm;224nm";
    US_CSV_Loader *csv_loader = new US_CSV_Loader(fpath, note, true, this);
    int state = csv_loader->exec();
    if (state != QDialog::Accepted) return;
    double min_r = 5;
    double max_r = 8;
    QString mesg = tr("Reference scan files should have at least two columns with the following pattern. "
                      "The preferred data separator is semicolon. "
                      "The first column is the radial points where header name is (cm). "
                      "The radial point should be between %1 to %2 cm and equally distanced with 0.001 cm. "
                      "The rest of the columns are reference profiles at different wavelengths. "
                      "They should be positive values where the header specifies what wavelength they are for."
                      "Each wavelength header item should be number followed by (nm). "
                      "Here is an example:\n\n"
                      "cm;220nm;222nm;224nm;226nm;230nm\n"
                      "5.8;1629.81;1712.49;1579.66;1732.45;1632.21\n"
                      "5.801;1631.89;1713.16;1580.46;1733.07;1632.54\n"
                      "....\n"
                      "....\n").arg(min_r).arg(max_r);
    US_CSV_Data csv_data = csv_loader->data();
    if (csv_data.columnCount() < 2 ) {
        QMessageBox::warning(this, "Error!", mesg + "\n\nError in the number of columns!");
        return;
    }
    QStringList header = csv_data.header();
    QVector<double> wavl;
    QVector<QPair<double, int>> wavl_idx;
    for (int ii = 0; ii < header.size(); ii++) {
        QString name = header.at(ii).trimmed();
        if (ii == 0 && name.compare("cm", Qt::CaseSensitive) != 0) {
            QMessageBox::warning(this, "Error!", mesg + "\n\nError in the header!");
            return;
        } else if (ii > 0) {
            if (name.endsWith("nm")) {
                name.chop(2);
                bool ok;
                double val = name.toDouble(&ok);
                if (!ok) {
                    QMessageBox::warning(this, "Error!", mesg + "\n\nError in the header!");
                    return;
                }
                if (val < 180 || val > 800) {
                    QMessageBox::warning(this, "Error!", "Error in the header! Wavelengths are out of range. "
                                                         "They must be in a range of 180 to 800 nm.");
                    return;
                }
                if (wavl.contains(val)) {
                    QMessageBox::warning(this, "Error!", mesg + "\n\nError in the header! Wavelength redundancy!");
                    return;
                } else {
                    wavl << val;
                    wavl_idx << qMakePair(val, ii - 1);
                }
            } else {
                QMessageBox::warning(this, "Error!", mesg + "\n\nError in the header!");
                return;
            }
        }
    }
    QVector<double> xvalues = csv_data.columnAt(0);
    QString mesg_r = mesg + "\n\nError in the radial points!";
    if (xvalues.first() < min_r || xvalues.last() > max_r) {
        QMessageBox::warning(this, "Error!", mesg_r);
        return;
    }
    for(int ii = 1; ii < xvalues.size(); ii++) {
        double r = xvalues.at(ii);
        if (r < min_r || r > max_r) {
            QMessageBox::warning(this, "Error!", mesg_r);
            return;
        }
        double dr = xvalues.at(ii) - xvalues.at(ii - 1);
        if (dr < 0.0008 || dr > 0.003) {
            QMessageBox::warning(this, "Error!", mesg_r);
            return;
        }
    }
    mesg_r = mesg + "\n\nError in the data values!";
    QVector<QVector<double>> yvalues;
    for (int ii = 1; ii < csv_data.columnCount(); ii++) {
        QVector<double> col = csv_data.columnAt(ii);
        foreach (double d, col) {
            if (d <= 0) {
                QMessageBox::warning(this, "Error!", mesg_r + " Negative and zero values are not allowed!");
                return;
            }
        }
        yvalues << col;
    }

    std::sort(wavl_idx.begin(), wavl_idx.end(),
              [](auto a, auto b) { return a.first < b.first;});
    RefscanFile reffile;
    double min_wvl = 10000;
    double max_wvl = 0;
    reffile.xvalues << xvalues;
    for (int ii = 0; ii < wavl_idx.size(); ii++) {
        double wvl = wavl_idx.at(ii).first;
        double idx = wavl_idx.at(ii).second;
        min_wvl = qMin(min_wvl, wvl);
        max_wvl = qMax(max_wvl, wvl);
        reffile.wavelength << wvl;
        reffile.yvalues << yvalues.at(idx);
    }
    reffile.min_wvl = min_wvl;
    reffile.max_wvl = max_wvl;
    reffile.nwvl = reffile.wavelength.size();
    reffile.filename = fpath;
    reffile.name = tr("Reference %1").arg(refscan_files.size() + 1);
    reffile.wavl_id = 0;

    refscan_files << reffile;
    set_table();
    pb_reset->setEnabled(true);
    hasData = true;
    le_status->clear();

}

int US_ConvertScan::read_auc(QVector<US_DataIO::RawData>& rawdata,
                              QVector<QVector<QVector<double>>>& absorbance,
                              QVector<QVector<QVector<double>>>& refscans,
                              QVector<QVector<double>>& shifts, QString& working_dir) {

    QDir inDir = QDir( working_dir, "*.auc", QDir::Name, QDir::Files | QDir::Readable );
    if (inDir.count() == 0){
        le_status->setText("auc files not found!");
        return -1;
    }

    QFileInfo finfo(working_dir);
    QString run_id = finfo.baseName();
    QRegExp re( "[^A-Za-z0-9_-]" );
    bool run_id_changed = false;
    int reIdx = run_id.indexOf(re, 0);
    if (reIdx >=0) {
        run_id_changed = true;
    }
    while (reIdx >=0){
        run_id = run_id.replace(reIdx, 1, "_");
        reIdx = run_id.indexOf(re, reIdx);
    }

    QStringList loaded_runids;
    foreach (CellChannel cc, ccw_items) {
        loaded_runids << cc.runid;
    }
    if (loaded_runids.contains(run_id)) {
        QMessageBox::warning( this, tr( "Error" ),
                             tr( "The RunID (%1) is already loaded!" ).arg(run_id));
        return -1;
    }

    inDir.makeAbsolute();
    le_status->setText("parsing files!");
    QFileInfoList fileList = inDir.entryInfoList();
    QStringList bad_files;
    QString run_type;
    QStringList run_type_avail{"RI"};
    // QStringList fnames;
    int maxscans = 1000000;

    for (int ii = 0; ii < fileList.size(); ++ii){
        QString fn = fileList.at(ii).fileName();
        US_DataIO::RawData rdata = US_DataIO::RawData();
        int state = US_DataIO::readRawData(fileList.at(ii).absoluteFilePath(), rdata);
        if (state != US_DataIO::OK) {
            bad_files << fn;
            continue;
        }
        if (rdata.scanCount() == 0) {
            bad_files << fn;
            continue;
        }

        QString rtp = QChar::fromLatin1(rdata.type[0]);
        rtp.append(QChar::fromLatin1(rdata.type[1]));
        QString rid = fn.section(".", 0, -6);
        int reIdx = rid.indexOf(re, 0);
        while (reIdx >=0){
            rid = rid.replace(reIdx, 1, "_");
            reIdx = rid.indexOf(re, reIdx);
        }
        if (run_id != rid) {
            QMessageBox::warning( this, tr( "Warning!" ),
                                 tr( "Multiple RunIDs Found!"));
            return -1;
        }
        if (! run_type_avail.contains(rtp)){
            QMessageBox::warning( this, tr( "Error" ),
                                 tr( "The Run type (%1) is not supported!" ).arg(rtp));
            return -1;
        }
        if (run_type.isEmpty()){
            run_type = rtp;
        } else {
            if (run_type != rtp){
                QMessageBox::warning( this, tr( "Error" ),
                                     tr( "Multiple run types are found in the directory!" ));
                return -1;
            }
        }

        rawdata << rdata;
        // fnames << fn;
        int ns = rdata.scanCount();
        maxscans = qMin(maxscans, ns);
        int np = rdata.pointCount();
        QVector<QVector<double>> abs;
        QVector<QVector<double>> rf;
        for (int jj = 0; jj < ns; jj++) {
            QVector<double> a(np, 0);
            abs << a;
            rf << a;
        }
        absorbance << abs;
        refscans << rf;
        QVector<double> sh(ns, 0);
        shifts << sh;
    }

    if ( run_id_changed ) {
        QMessageBox::warning( this, tr( "Warning!" ),
                             tr( "The RunID changed. It can only have alphanumeric,"
                                "underscore, and hyphen characters."));
    }

    if (!bad_files.isEmpty()) {
        QMessageBox::warning( this, tr( "Warning!" ),
                             tr( "Unable to load %1 of the files").arg(bad_files.size()));
    }

    if (rawdata.isEmpty()) {
        return -1;
    }
    working_dir.clear();
    working_dir = run_id;
    return maxscans;

}

void US_ConvertScan::import_run() {
    QVector<US_DataIO::RawData> rawdata;
    QVector<QVector<QVector<double>>> absorbance;
    QVector<QVector<QVector<double>>> refscans;
    QVector<QVector<double>> shifts;
    QString workingDir;

    if (disk_controls->db()) {
        QVector< US_DataIO::RawData > allData;
        QStringList triples;
        US_LoadAUC* dialog = new US_LoadAUC( false, allData, triples, workingDir );
        if ( dialog->exec() == QDialog::Rejected )  return;
    } else {
        workingDir = QFileDialog::getExistingDirectory( this,
                                                tr( "Open Run ID" ),
                                                US_Settings::importDir(),
                                                QFileDialog::DontResolveSymlinks );
        if (workingDir.isEmpty()) return;
    }

    int maxscn = read_auc(rawdata, absorbance, refscans, shifts, workingDir);
    if (maxscn == -1) {
        return;
    }

    QVector<int> states(rawdata.size(), -1);
    intensity_data << rawdata;
    absorbance_data << absorbance;
    absorbance_state << states;
    absorbance_shifts << shifts;
    refscan_data << refscans;

    list_ccw_items(workingDir);
    set_table();
    pb_reset->setEnabled(true);
    hasData = true;
    le_status->clear();

    maxscn = qMin(max_nscans, maxscn);
    set_ct_scans(maxscn);

}

void US_ConvertScan::set_ct_scans(int maxval) {
    ct_scans->disconnect();

    if (maxval == 0) {
        max_nscans = 1000;
        ct_scans->setValid(1);
        ct_scans->setMaximum(1);
        ct_scan_from->disconnect();
        ct_scan_to->disconnect();
        ct_scan_from->setMaximum(1);
        ct_scan_to->setMaximum(1);
    } else {
        int cv = ct_scans->value();
        max_nscans = maxval;
        ct_scans->setMaximum(max_nscans);
        if (cv > max_nscans) {
            cv = max_nscans;
            apply_nscans();
        }
        ct_scans->setValue(cv);
    }
    connect(ct_scans, &QwtCounter::valueChanged, this, &US_ConvertScan::update_nscans);
}

void US_ConvertScan::lower_scan_range(double value) {
    if (value > ct_scan_to->value()) {
        ct_scan_to->disconnect();
        ct_scan_to->setValue(value);
        connect(ct_scan_to, &QwtCounter::valueChanged, this, &US_ConvertScan::upper_scan_range);
    }
    emit sig_plot();
}

void US_ConvertScan::upper_scan_range(double value) {
    if (value < ct_scan_from->value()) {
        ct_scan_from->disconnect();
        ct_scan_from->setValue(value);
        connect(ct_scan_from, &QwtCounter::valueChanged, this, &US_ConvertScan::lower_scan_range);
    }
    emit sig_plot();
}

void US_ConvertScan::prev_id(){
    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    int wavl_id;
    if (role < 100) {
        wavl_id = ccw_items.at(role).wavl_id;
    } else {
        wavl_id = refscan_files.at(role - 100).wavl_id;
    }
    cb_plot_id->setCurrentIndex(--wavl_id);
}

void US_ConvertScan::next_id(){
    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    int wavl_id;
    if (role < 100) {
        wavl_id = ccw_items.at(role).wavl_id;
    } else {
        wavl_id = refscan_files.at(role - 100).wavl_id;
    }
    cb_plot_id->setCurrentIndex(++wavl_id);
}

void US_ConvertScan::offon_prev_next(){
    int n_wavls, wavl_id;
    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    if (role < 100) {
        n_wavls = ccw_items.at(role).wavelength.size();
        wavl_id = ccw_items.at(role).wavl_id;
    } else {
        n_wavls = refscan_files.at(role - 100).wavelength.size();
        wavl_id = refscan_files.at(role - 100).wavl_id;
    }
    pb_prev_id->setDisabled(wavl_id <= 0);
    pb_next_id->setDisabled(wavl_id >= (n_wavls - 1));
    return;
}

void US_ConvertScan::set_wavl_ctrl(){
    if (tb_triple->currentColumn() >= 2) return;

    cb_plot_id->disconnect();
    cb_plot_id->clear();
    if (ccw_items.isEmpty() && refscan_files.isEmpty()) {
        le_lambstrt->clear();
        le_lambstop->clear();
        le_runid->clear();
        le_desc->clear();
        emit sig_plot();
        return;
    }

    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    QVector<double> wavelength;
    int wavl_id;
    if (role < 100) {
        wavelength << ccw_items.at(row).wavelength;
        wavl_id = ccw_items.at(row).wavl_id;
    } else {
        wavelength << refscan_files.at(role - 100).wavelength;
        wavl_id = refscan_files.at(role - 100).wavl_id;
    }

    le_lambstrt->setText(tr("%1").arg(wavelength.first()));
    le_lambstrt->setAlignment(Qt::AlignCenter);
    le_lambstop->setText(tr("%1").arg(wavelength.last()));
    le_lambstop->setAlignment(Qt::AlignCenter);
    foreach (double wvl, wavelength) {
        cb_plot_id->addItem(QString::number(wvl));
    }
    for (int ii = 0; ii < cb_plot_id->count(); ii++) {
        cb_plot_id->setItemData(ii, Qt::AlignCenter, Qt::TextAlignmentRole);
    }
    cb_plot_id->setCurrentIndex(wavl_id);

    // connect(cb_plot_id, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         [=](int index){ select_id(index); });

    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(select_id(int)));
    select_id(wavl_id);
}

void US_ConvertScan::select_id(int id){
    disconnect_picker();
    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    if (role >= 100) {
        le_runid->clear();
        le_desc->setText(refscan_files.at(role - 100).filename);
        double wl = refscan_files.at(role - 100).wavelength.at(id);
        refscan_files[role - 100].wavl_id = id;
        // double wl = wavelength.at(wavl_id);
        QString title("Wavelength= %1 nm");
        plot_title->setText(title.arg(wl));
    } else {
        int raw_id = ccw_items.at(role).rawdata_ids.at(id);
        ccw_items[role].wavl_id = id;
        le_runid->setText(ccw_items.at(tb_triple->currentRow()).runid);
        le_desc->setText(intensity_data.at(raw_id).description);
        double wl = ccw_items.at(role).wavelength.at(id);
        int ns = intensity_data.at(raw_id).scanData.size();
        QString title("Wavelength= %1 nm; #Scans= %2");
        plot_title->setText(title.arg(wl).arg(ns));
    }
    offon_prev_next();
    emit sig_plot();
    return;
}

void US_ConvertScan::plot_all(){
    plot_intensity();
    plot_refscan();
    plot_absorbance();
    return;
}

void US_ConvertScan::select_refscan(int ref_row) {
    QComboBox* combo = qobject_cast<QComboBox*>(sender());
    if (combo) {
        int item_row = tb_triple->indexAt(combo->pos()).row();
        int role = combo->itemData(ref_row, Qt::UserRole).toInt();
        ccw_items[item_row].ref_id = role;

        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        calc_absorbance(item_row);
        QGuiApplication::restoreOverrideCursor();

        highlight();
        QModelIndex index = tb_triple->model()->index(item_row, 0);
        tb_triple->setCurrentIndex(index);
    }
}

void US_ConvertScan::disconnect_picker() {
    picker_abs->disconnect();
    pb_pick_rp->setEnabled(true);
}

void US_ConvertScan::default_region() {
    disconnect_picker();
    if (ccw_items.isEmpty()) return;
    ccw_items[tb_triple->currentRow()].minmax_x = qMakePair(-1, -1);
    plot_absorbance();
}

void US_ConvertScan::pick_region(){
    if (! abs_plt_on) {
        return;
    }
    disconnect_picker();
    ccw_items[tb_triple->currentRow()].minmax_x = qMakePair(-1, -1);
    pb_pick_rp->setEnabled(false);
    connect(picker_abs, &US_PlotPicker::cMouseUp, this, &US_ConvertScan::mouse_click);
    plot_absorbance();
}

void US_ConvertScan::mouse_click(const QwtDoublePoint& point){
    double x = point.x();
    int row = tb_triple->currentRow();
    double min_x = intensity_data.at(ccw_items.at(row).rawdata_ids.at(0)).xvalues.first();
    double max_x = intensity_data.at(ccw_items.at(row).rawdata_ids.at(0)).xvalues.last();
    if (x < min_x || x > max_x) {
        QString mess("The picked point is beyond the data values. Please try again!\n"
                     "The picked point = %1 cm\n"
                     "Minimum radial point: %2 cm\n"
                     "Maximum radial point: %3 cm\n");
        QMessageBox::warning( this, tr( "Warning" ), mess.arg(x, min_x, max_x));
        return;
    }
    double first = ccw_items.at(tb_triple->currentRow()).minmax_x.first;
    if (first <= 0) {
        ccw_items[row].minmax_x.first = x;
        plot_absorbance();
    } else {
        if ( x <= first) {
            QString mess("The second point has to be greater than the first one. Please try again!\n"
                         "The picked point = %1 cm\n"
                         "The first point s= %2 cm\n");
            QMessageBox::warning( this, tr( "Warning" ), mess.arg(x, first));
            return;
        } else {
            ccw_items[row].minmax_x.second = x;
            update_shifts(row);
            plot_absorbance();
            disconnect_picker();
        }
    }
}

bool US_ConvertScan::set_abs_runid(QString& runid) {
    QDialog *dialog = new QDialog(this);
    dialog->setPalette( US_GuiSettings::frameColorDefault() );
    QHBoxLayout *lyt1 = new QHBoxLayout();
    QLabel *label = us_label("Absorbance RunID:");
    US_LineEdit_RE* wg_runid = new US_LineEdit_RE();
    wg_runid->setText(runid);
    lyt1->addWidget(label);
    lyt1->addWidget(wg_runid);

    QVBoxLayout *lyt2 = new QVBoxLayout();
    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Ok
                                                     | QDialogButtonBox::Cancel);
    buttons->setFont(font);

    lyt2->addLayout(lyt1);
    lyt2->addStretch(1);
    lyt2->addWidget(buttons);
    lyt2->setMargin(1);
    lyt2->setSpacing(2);
    lyt2->setMargin(2);
    dialog->setLayout(lyt2);
    dialog->setMinimumWidth(600);
    dialog->setFixedHeight(75);
    connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    int state = dialog->exec();
    if (state != QDialog::Accepted) {
        return false;
    }

    QString path = US_Settings::importDir();
    QDir dir(path);
    if (! dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    dir.setPath(dir.absoluteFilePath(wg_runid->text()));
    if (dir.exists()) {
        int ans = QMessageBox::question(this, "Absorbance RunID",
                                        "The output directory exists!\n\n"
                                            + dir.absolutePath() +
                                            "\n\nBy clicking on 'YES', "
                                            "all data will be overwritten! "
                                            + "Do you want to proceed?");
        if (ans == QMessageBox::No) {
            return false;
        } else {
            dir.removeRecursively();
        }
    }
    dir.mkpath(dir.absolutePath());
    runid.clear();
    runid = wg_runid->text();
    return true;
}

void US_ConvertScan::save_run() {
    if (ccw_items.isEmpty()) {
        return;
    }

    QStringList run_list;
    QVector<int> item_list;
    for (int ii = 0; ii < ccw_items.size(); ii++) {
        bool ok = false;
        QString runid = ccw_items.at(ii).runid;
        QWidget *wgt = tb_triple->cellWidget(ii, 3);
        if (wgt) {
            QCheckBox *chkb = wgt->findChild<QCheckBox *>();
            if (chkb && chkb->isChecked()) {
                ok = true;
            }
        }
        if (ok) {
            item_list << ii;
            if (! run_list.contains(runid)) {
                run_list << runid;
            }
        }
    }
    if (run_list.size() == 0) {
        QMessageBox::warning(this, "Warning!", "No triple selected to save!");
        return;
    } else if (run_list.size() > 1) {
        QMessageBox::warning(this, "Warning!", "Triples from multiple RunIDs are selected!\n"
                                               "Please save each RunID separately.");
        return;
    }

    QVector<US_DataIO::RawData> out_rawdata;
    QStringList miss_ccw;
    QString ccw_str("%1/%2/%3");
    foreach (int ii, item_list) {
        double x_1 = ccw_items.at(ii).minmax_x.first;
        double x_2 = ccw_items.at(ii).minmax_x.second;
        bool shift = false;
        if (x_1 > 0 && x_2 > 0) {
            shift = true;
        }
        for (int jj = 0; jj < ccw_items.at(ii).rawdata_ids.size(); jj++) {
            int rid = ccw_items.at(ii).rawdata_ids.at(jj);
            int cell = ccw_items.at(ii).cell;
            char channel = ccw_items.at(ii).channel;
            if (absorbance_state.at(rid) == -1) {
                double wvl = ccw_items.at(ii).wavelength.at(jj);
                miss_ccw << ccw_str.arg(cell).arg(channel).arg(wvl);
            } else {
                US_DataIO::RawData rawdata = intensity_data.at(rid);
                int N = absorbance_data.at(rid).size() - nscans;
                int pos = ct_scan_from->value() - 1 + N;
                int len = ct_scan_to->value() + N - pos;
                QVector<QVector<double>> absorbance = absorbance_data.at(rid).mid(pos, len);
                QVector<double> shifts = absorbance_shifts.at(rid).mid(pos, len);
                QVector<US_DataIO::Scan> scans = rawdata.scanData.mid(pos, len);
                trim_absorbance(absorbance, shifts, shift);
                for (int mm = 0; mm < absorbance.size(); mm++) {
                    scans[mm].stddevs.clear();
                    scans[mm].rvalues.clear();
                    scans[mm].rvalues << absorbance.at(mm);
                }
                rawdata.scanData.clear();
                rawdata.scanData << scans;
                rawdata.type[0] = 'R';
                rawdata.type[1] = 'A';
                uchar uuid[ 16 ];
                QString uuid_string = US_Util::new_guid();
                US_Util::uuid_parse( uuid_string, uuid );
                memcpy(rawdata.rawGUID, (char*) uuid, 16);
                out_rawdata << rawdata;
            }
        }
    }
    if (out_rawdata.isEmpty()) {
        QMessageBox::warning(this, "Error!", "No absorbance data found to save!");
        return;
    }
    if (! miss_ccw.isEmpty()) {
        int ans = QMessageBox::question(this, "Warning!",
                                        tr("There is (%1) triple(s) missing.\n"
                                           "Do you proceed to save the remaining (%2) "
                                           "triples ?").arg(miss_ccw.size()).arg(out_rawdata.size()));
        if (ans == QMessageBox::No) {
            return;
        }
    }
    QString abs_runid = "PABS_" + run_list.at(0);
    if (! set_abs_runid(abs_runid)) {
        return;
    }

    QString fileName("%1.RA.%2.%3.%4.auc");
    QDir dir = QDir(US_Settings::importDir());
    dir.setPath(dir.absoluteFilePath(abs_runid));
    for (int ii = 0; ii < out_rawdata.size(); ii++) {
        US_DataIO::RawData rawdata = out_rawdata.at(ii);
        int cell = rawdata.cell;
        char channel = rawdata.channel;
        double wvl = rawdata.scanData.at(0).wavelength;
        QString fn = fileName.arg(abs_runid).arg(cell).arg(channel).arg(wvl);
        QFileInfo fileInfo(dir, fn);
        US_DataIO::writeRawData(fileInfo.absoluteFilePath(), rawdata);
    }

}

void US_ConvertScan::set_table(){
    tb_triple->disconnect();
    tb_triple->clearContents();
    int nrows = ccw_items.size() + refscan_files.size();
    if (nrows == 0) {
        return;
    }
    tb_triple->setRowCount(nrows);

    QString runid;
    int rcode = 1;
    QVector<int> rcode_list;
    for (int ii = 0; ii < ccw_items.size(); ii++) {
        QString ccw_runid = ccw_items.at(ii).runid;
        if (runid.isEmpty()) {
            runid = ccw_runid;
        } else if ( runid.compare(ccw_runid) != 0) {
            rcode += 1;
            runid = ccw_runid;
        }
        rcode_list << rcode;
    }

    for (int ii = 0; ii < ccw_items.size(); ii++){
        rcode = rcode_list.at(ii);
        int cell = ccw_items.at(ii).cell;
        char channel = ccw_items.at(ii).channel;
        double min_wl = ccw_items.at(ii).wavelength.first();
        double max_wl = ccw_items.at(ii).wavelength.last();
        int nwl = ccw_items.at(ii).wavelength.size();
        QString item_cc;
        if (rcode_list.last() == 1) {
            item_cc = tr("%1 / %2").arg(cell).arg(channel);
        } else {
            item_cc = tr("[%1] %2 / %3").arg(rcode).arg(cell).arg(channel);
        }
        QTableWidgetItem *twi_cc;
        twi_cc = new QTableWidgetItem(item_cc);
        twi_cc->setFlags(twi_cc->flags() & ~Qt::ItemIsEditable);
        twi_cc->setFont(font);
        twi_cc->setData(Qt::UserRole, QVariant(ii));
        twi_cc->setTextAlignment(Qt::AlignCenter);
        tb_triple->setItem(ii, 0, twi_cc);

        QString item_wvl = tr("%1-%2 (%3)").arg(min_wl).arg(max_wl).arg(nwl);
        QTableWidgetItem *twi_wvl;
        twi_wvl = new QTableWidgetItem(item_wvl);
        twi_wvl->setFlags(twi_wvl->flags() & ~Qt::ItemIsEditable);
        twi_wvl->setTextAlignment(Qt::AlignCenter);
        twi_wvl->setFont(font);
        tb_triple->setItem(ii, 1, twi_wvl);
    }

    for (int ii = 0; ii < refscan_files.size(); ii++) {
        QString item_cc = refscan_files.at(ii).name;
        QTableWidgetItem *twi_cc;
        twi_cc = new QTableWidgetItem(item_cc);
        twi_cc->setFlags(twi_cc->flags() & ~Qt::ItemIsEditable);
        twi_cc->setData(Qt::UserRole, QVariant(ii + 100));
        twi_cc->setTextAlignment(Qt::AlignCenter);
        twi_cc->setFont(font);
        int II = ccw_items.size() + ii;
        tb_triple->setItem(II, 0, twi_cc);

        QString item_wvl = tr("%1-%2 (%3)").arg(refscan_files.at(ii).min_wvl)
                               .arg(refscan_files.at(ii).max_wvl).arg(refscan_files.at(ii).nwvl);
        QTableWidgetItem *twi_wvl;
        twi_wvl = new QTableWidgetItem(item_wvl);
        twi_wvl->setFlags(twi_wvl->flags() & ~Qt::ItemIsEditable);
        twi_wvl->setFont(font);
        twi_wvl->setTextAlignment(Qt::AlignCenter);
        tb_triple->setItem(II, 1, twi_wvl);

        QTableWidgetItem *twi_2 = new QTableWidgetItem();
        twi_2->setFlags(twi_2->flags() & ~Qt::ItemIsEditable);
        tb_triple->setItem(II, 2, twi_2);

        QTableWidgetItem *twi_3 = new QTableWidgetItem();
        twi_3->setFlags(twi_3->flags() & ~Qt::ItemIsEditable);
        tb_triple->setItem(II, 3, twi_3);
    }

    for (int ii = 0; ii < ccw_items.size(); ii++) {
        QComboBox *combo = us_comboBox();
        combo->setEditable(true);
        combo->lineEdit()->setAlignment(Qt::AlignCenter);
        combo->lineEdit()->setReadOnly(true);
        combo->addItem("none");
        for (int jj = 0; jj < nrows; jj++) {
            combo->addItem(tb_triple->item(jj, 0)->text());
        }

        int cid = 0;
        int re_fid = ccw_items.at(ii).ref_id;
        for (int jj = 0; jj < combo->count(); jj++) {
            combo->setItemData(jj, Qt::AlignCenter, Qt::TextAlignmentRole);
            if (jj == 0) {
                combo->setItemData(jj, -1, Qt::UserRole);
                continue;
            }
            int role = tb_triple->item(jj - 1, 0)->data(Qt::UserRole).toInt();
            combo->setItemData(jj, role, Qt::UserRole);
            if (role == re_fid) {
                cid = jj;
            }
        }
        combo->setCurrentIndex(cid);

        tb_triple->setCellWidget(ii, 2, combo);
        connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(select_refscan(int)));

        // connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        //         [=](int index){ select_refscan(index); });

        QCheckBox* checkbox = new QCheckBox();
        QWidget *wgt = new QWidget();
        QHBoxLayout *lyt = new QHBoxLayout(wgt);
        lyt->addWidget(checkbox);
        lyt->setAlignment(Qt::AlignCenter);
        lyt->setContentsMargins(0, 0, 0, 0);
        wgt->setLayout(lyt);
        tb_triple->setCellWidget(ii, 3, wgt);

    }
    connect( tb_triple, &QTableWidget::currentCellChanged, this, &US_ConvertScan::set_wavl_ctrl);
    QModelIndex index = tb_triple->model()->index(0, 0);
    tb_triple->setCurrentIndex(index);
    highlight();
}

void US_ConvertScan::list_ccw_items(QString& runid){
    if (intensity_data.size() == 0) {
        return;
    }
    int II = -1;
    for (int ii = 0; ii < ccw_items.size(); ii++) {
        for (int jj = 0; jj < ccw_items.at(ii).rawdata_ids.size(); jj++) {
            II = qMax(II, ccw_items.at(ii).rawdata_ids.at(jj));
        }
    }
    II++;

    QMap<QString, QVector<QPair<double, int>>> wavl_idx_map;
    QString key_str("%1-%2");
    for (int ii = II; ii < intensity_data.size(); ii++){
        int cell = intensity_data.at(ii).cell;
        char channel = intensity_data.at(ii).channel;
        double wavelength = intensity_data.at(ii).scanData.at(0).wavelength;
        QString key = key_str.arg(cell).arg(channel);
        wavl_idx_map[key] << qMakePair(wavelength, ii);
    }

    QStringList keys = wavl_idx_map.keys();
    std::sort(keys.begin(), keys.end());

    foreach (QString key, keys) {
        CellChannel cc;
        QStringList ksp = key.split("-");
        cc.cell = ksp.at(0).toInt();
        cc.channel = ksp.at(1).at(0).toLatin1();
        cc.minmax_x = qMakePair(-1, -1);
        cc.ref_id = -1;
        cc.runid = runid;
        cc.wavl_id = 0;
        QVector<QPair<double, int>> wavl_idx = wavl_idx_map.value(key);
        std::sort(wavl_idx.begin(), wavl_idx.end(),
                  [](auto a, auto b) { return a.first < b.first;});
        for (int ii = 0; ii < wavl_idx.size(); ii++) {
            cc.wavelength << wavl_idx.at(ii).first;
            cc.rawdata_ids << wavl_idx.at(ii).second;
        }
        ccw_items << cc;
    }
    qDebug();
}

void US_ConvertScan::plot_intensity(){
    qwtplot_insty->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (ccw_items.isEmpty()){
        grid = us_grid(qwtplot_insty);
        qwtplot_insty->replot();
        return;
    }

    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    if (role >= 100) {
        return;
    }

    int wavl_id = ccw_items.at(role).wavl_id;
    int raw_id = ccw_items.at(role).rawdata_ids.at(wavl_id);
    bool flag = chkb_abs_int->isChecked();
    flag = flag && (absorbance_state.at(raw_id) >= 0 && absorbance_state.at(raw_id) < 100);
    flag = !flag;

    US_DataIO::RawData raw_data = intensity_data.at(raw_id);
    int N = raw_data.scanCount() - nscans;
    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(1);
    double rb = qCeil(nscans / 50.0);
    int nc = qCeil(nscans / rb);
    HSVcolormap colormap(nc);
    QVector<QColor> colorList;
    int error = colormap.get_colorlist(colorList);

    const double *x, *r;
    x = raw_data.xvalues.data();

    double min_x  =  1e20;
    double max_x  = -1e20;
    double min_r  =  1e20;
    double max_r  = -1e20;
    int np = raw_data.pointCount();
    int ii_0 = ct_scan_from->value() - 1;
    int ii_1 = ct_scan_to->value();
    int cnt = 0;
    for (int ii = ii_0; ii < ii_1; ++ii){
        r = raw_data.scanData.at(N + ii).rvalues.data();
        for (int j = 0; j < np; ++j){
            min_x = qMin(min_x, x[j]);
            max_x = qMax(max_x, x[j]);
            min_r = qMin(min_r, r[j]);
            max_r = qMax(max_r, r[j]);
        }
        QwtPlotCurve* curve = us_curve( qwtplot_insty, tr("Intensity %1").arg(cnt + 1));
        pen_plot.setColor(QColor(Qt::red));
        if (flag && error == 0) {
            pen_plot.setColor(colorList[ cnt % nc ]);
        }
        curve->setPen( pen_plot );
        curve->setSamples(x, r, np);
        cnt++;
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

void US_ConvertScan::plot_refscan(){
    if (ccw_items.isEmpty() && refscan_files.isEmpty()) {
        return;
    }

    double min_r = 1e99;
    double max_r = -1e99;
    QVector<double> xvalues;
    QVector<QVector<double>> yvalues;
    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    int wavl_id;
    if (role >= 100) {
        wavl_id = refscan_files.at(role - 100).wavl_id;
        xvalues << refscan_files.at(role - 100).xvalues;
        yvalues << refscan_files.at(role - 100).yvalues.at(wavl_id);
    } else {
        wavl_id = ccw_items.at(tb_triple->currentRow()).wavl_id;
        int raw_id = ccw_items.at(tb_triple->currentRow()).rawdata_ids.at(wavl_id);
        if (absorbance_state.at(raw_id) == -1) {
            return;
        }
        if (absorbance_state.at(raw_id) < 100 && !chkb_abs_int->isChecked()) {
            return;
        }
        min_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).lowerBound();
        max_r = qwtplot_insty->axisScaleDiv(QwtPlot::yLeft).upperBound();
        xvalues << intensity_data.at(raw_id).xvalues;
        int N = absorbance_data.at(raw_id).size() - nscans;
        QVector<QVector<double>> refscan = refscan_data.at(raw_id).mid(N);
        int ii_0 = ct_scan_from->value() - 1;
        int ii_1 = ct_scan_to->value();
        yvalues << refscan.at(ii_0);
        ii_0++;
        if (absorbance_state.at(raw_id) < 100) {
            for (int ii = ii_0; ii < ii_1; ++ii){
                yvalues << refscan.at(ii);
            }
        }
    }

    grid = us_grid(qwtplot_insty);
    QPen pen = QPen( QBrush( Qt::white ), 1. );
    const double *xp, *rp;
    xp = xvalues.data();
    int np = xvalues.size();

    for (int ii = 0; ii < yvalues.size(); ii++) {
        rp = yvalues.at(ii).data();
        for (int jj = 0; jj < np; ++jj){
            min_r = qMin(min_r, rp[jj]);
            max_r = qMax(max_r, rp[jj]);
        }
        QwtPlotCurve* curve = us_curve( qwtplot_insty, tr("Reference %1").arg(ii + 1));
        curve->setPen(pen);
        curve->setSamples(xp, rp, np);
    }
    double dr = (max_r - min_r) * 0.05;
    qwtplot_insty->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    qwtplot_insty->updateAxes();
    qwtplot_insty->replot();
}

void US_ConvertScan::plot_absorbance(){
    abs_plt_on = false;
    qwtplot_abs->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    grid = us_grid(qwtplot_abs);
    qwtplot_abs->replot();
    if (ccw_items.isEmpty()) {
        return;
    }

    int row = tb_triple->currentRow();
    int role = tb_triple->item(row, 0)->data(Qt::UserRole).toInt();
    if (role >= 100) {
        return;
    }
    int wavl_id = ccw_items.at(role).wavl_id;
    int raw_id = ccw_items.at(role).rawdata_ids.at(wavl_id);
    if (absorbance_state.at(raw_id) == -1) {
        return;
    }

    double x_1 = ccw_items.at(row).minmax_x.first;
    double x_2 = ccw_items.at(row).minmax_x.second;

    int N = absorbance_data.at(raw_id).size() - nscans;

    QVector<QVector<double>> absorbance = absorbance_data.at(raw_id).mid(N);
    QVector<double> shift = absorbance_shifts.at(raw_id).mid(N);
    bool state = false;
    if (x_1 > 0 && x_2 > 0) {
        state = true;
    }
    trim_absorbance(absorbance, shift, state);

    QVector<double> xvalues = intensity_data.at(raw_id).xvalues;

    double min_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    double max_x = qwtplot_insty->axisScaleDiv(QwtPlot::xBottom).upperBound();
    double min_r  =  1e20;
    double max_r  = -1e20;

    int ns = absorbance.size();
    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(1);
    double rb = qCeil(ns / 50.0);
    int nc = qCeil(ns / rb);
    HSVcolormap colormap(nc);
    QVector<QColor> colorList;
    int error = colormap.get_colorlist(colorList);

    const double *xp = xvalues.data();
    const double *rp;

    int np = xvalues.size();
    int l1 = np * 0.2;
    int l2 = np * 0.8;

    int ii_0 = ct_scan_from->value() - 1;
    int ii_1 = ct_scan_to->value();
    int cnt = 0;
    for (int ii = ii_0; ii < ii_1; ++ii){
        rp = absorbance.at(ii).data();
        for (int jj = 0; jj < np; ++jj)
            if (jj > l1 && jj < l2){
                min_r = qMin(min_r, rp[jj]);
                max_r = qMax(max_r, rp[jj]);
            }
        QwtPlotCurve* curve = us_curve( qwtplot_abs, tr("Absorbance %1").arg(cnt + 1));
        if (error == 0)
            pen_plot.setColor(colorList[ cnt % nc ]);
        curve->setPen( pen_plot );
        curve->setSamples(xp, rp, np);
        cnt++;
    }
    grid = us_grid(qwtplot_abs);
    double dr = (max_r - min_r) * 0.05;
    min_r -= dr;
    max_r += dr;
    if (x_1 > 0){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double y0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << x_1;
            yy << y0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_abs,"left");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    if (x_2 > -1){
        QVector<double> xx;
        QVector<double> yy;
        int np = 50;
        double dyy = (max_r - min_r + 2 * dr) / np;
        double y0 = min_r - dr;
        for (int i = 0; i < np; ++i){
            xx << x_2;
            yy << y0 + i * dyy;
        }
        pen_plot.setWidth(3);
        pen_plot.setColor(QColor(Qt::yellow));
        QwtPlotCurve* curve = us_curve( qwtplot_abs,"right");
        curve->setStyle(QwtPlotCurve::Dots);
        curve->setPen(pen_plot);
        curve->setSamples(xx.data(), yy.data(), np);
    }

    qwtplot_abs->setAxisScale( QwtPlot::xBottom, min_x, max_x);
    qwtplot_abs->setAxisScale( QwtPlot::yLeft  , min_r, max_r);
    qwtplot_abs->updateAxes();
    qwtplot_abs->replot();
    abs_plt_on = true;
}

bool US_ConvertScan::get_refval_file(int ref_id, int raw_id) {

    ref_id -= 100;
    US_DataIO::RawData rawdata = intensity_data.at(raw_id);
    int iwvl_tgt = static_cast<int>(qRound(rawdata.scanData.at(0).wavelength * 10));
    QVector<double> xvals_tgt = rawdata.xvalues;

    bool ok = false;
    for (int ii = 0; ii < refscan_files.at(ref_id).wavelength.size(); ii++) {
        int iwvl = static_cast<int>(qRound(refscan_files.at(ref_id).wavelength.at(ii) * 10));
        if (iwvl == iwvl_tgt) {
            QVector<double> ref_xvals = refscan_files.at(ref_id).xvalues;
            QVector<double> ref_yvals = refscan_files.at(ref_id).yvalues.at(ii);
            if (linear_interpolation(xvals_tgt, ref_xvals, ref_yvals)) {
                if (smooth > 0){
                    double max_OD = ct_maxod->value();
                    QVector<double> yvals = smooth_refscan(ref_yvals, smooth, true, true, max_OD);
                    ref_yvals.clear();
                    ref_yvals << yvals;
                }
                int N = rawdata.scanCount() - nscans;
                for (int jj = N; jj < rawdata.scanCount(); jj++) {
                    for (int kk = 0; kk < ref_yvals.size(); kk++) {
                        refscan_data[raw_id][jj][kk] = ref_yvals.at(kk);
                    }
                }
            } else {
                return false;
            }
            ok = true;
            break;
        }
    }
    return ok;
}

bool US_ConvertScan::get_refval_buffer(int ref_row, int raw_id) {

    int iwvl_tgt = static_cast<int>(qRound(intensity_data.at(raw_id).scanData.at(0).wavelength * 10));
    QVector<double> xvals_tgt = intensity_data.at(raw_id).xvalues;
    int N_tgt = intensity_data[raw_id].scanCount() - nscans;

    bool ok = false;
    foreach (int rid, ccw_items.at(ref_row).rawdata_ids) {
        US_DataIO::RawData rawdata = intensity_data.at(rid);
        int iwvl = static_cast<int>(qRound(rawdata.scanData.at(0).wavelength * 10));
        if (iwvl_tgt == iwvl) {
            QVector<double> ref_xvals = rawdata.xvalues;
            int N_ref = rawdata.scanCount() - nscans;
            for (int ii = 0; ii < nscans; ii++) {
                int II_ref = ii + N_ref;
                int II_tgt = ii + N_tgt;
                QVector<double> ref_yvals = rawdata.scanData.at(II_ref).rvalues;
                if (linear_interpolation(xvals_tgt, ref_xvals, ref_yvals)) {
                    if (smooth > 0){
                        double max_OD = ct_maxod->value();
                        QVector<double> yvals = smooth_refscan(ref_yvals, smooth, true, true, max_OD);
                        ref_yvals.clear();
                        ref_yvals << yvals;
                    }
                    for (int jj = 0; jj < ref_yvals.size(); jj++) {
                        refscan_data[raw_id][II_tgt][jj] = ref_yvals.at(jj);
                    }
                } else {
                    return false;
                }
            }
            ok = true;
            break;
        }
    }
    return ok;
}

void US_ConvertScan::trim_absorbance(QVector<QVector<double>>& absorbance,
                                     QVector<double>& shift, bool state) {
    double max_OD = ct_maxod->value();
    int ns = absorbance.size();
    for (int ii = 0; ii < ns; ii++) {
        int np = absorbance.at(ii).size();
        for (int jj = 0; jj < np; jj++) {
            double val = absorbance.at(ii).at(jj);
            if (state) {
                val += shift.at(ii);
            }
            if (val > max_OD) {
                val = max_OD;
            } else if (val < -max_OD) {
                val = -max_OD;
            }
            absorbance[ii][jj] = val;
        }
    }
}

void US_ConvertScan::calc_absorbance(int item_row){
    if (ccw_items.isEmpty()) return;

    QVector<int> rawdata_ids = ccw_items.at(item_row).rawdata_ids;
    const int ref_id = ccw_items.at(item_row).ref_id;
    bool buffer = true;
    if ( ref_id == -1) {
        foreach (int rid, rawdata_ids) {
            absorbance_state[rid] = -1;
        }
        return;
    } else if (ref_id >= 100) {
        buffer = false;
    }

    double x_1 = ccw_items.at(item_row).minmax_x.first;
    double x_2 = ccw_items.at(item_row).minmax_x.second;
    bool shift = false;
    if (x_1 > 0 && x_2 > 0) {
        shift = true;
    }

    QVector<double> not_found_wvl;
    for (int ii = 0; ii < rawdata_ids.size(); ii++) {
        int rid = rawdata_ids.at(ii);
        US_DataIO::RawData rawdata = intensity_data.at(rid);
        double wvl = ccw_items.at(item_row).wavelength.at(ii);
        bool ok = false;
        if (buffer) {
            ok = get_refval_buffer(ref_id, rid);
        } else {
            ok = get_refval_file(ref_id, rid);
        }
        absorbance_state[rid] = -1;
        if (! ok) {
            not_found_wvl << wvl;
            continue;
        }

        int N = rawdata.scanCount() - nscans;
        QVector<double> xvalues = rawdata.xvalues;
        for (int jj = N; jj < rawdata.scanCount(); jj++) {
            double miny = 1e99;
            for (int kk = 0; kk < rawdata.pointCount(); kk++){
                double x = xvalues.at(kk);
                double rfv = refscan_data.at(rid).at(jj).at(kk);
                double val = rfv / rawdata.reading(jj, kk);
                if (val <= 0) {
                    val = 1e-5;
                }
                val = std::log10(val);
                absorbance_data[rid][jj][kk] = val;
                if (shift && x >= x_1 && x <= x_2) {
                    miny = qMin(miny, val);
                }
            }
            if (shift) {
                absorbance_shifts[rid][jj] = -1 * miny;
            }
        }
        absorbance_state[rid] = ref_id;
    }
}

void US_ConvertScan::highlight() {
    for (int ii = 0; ii < ccw_items.size(); ii++) {
        QVector<int> wvls;
        tb_triple->item(ii, 0)->setBackground(QBrush(Qt::white));
        tb_triple->item(ii, 1)->setBackground(QBrush(Qt::white));
        int ref_id = ccw_items.at(ii).ref_id;
        if ( ref_id == -1) {
            continue;
        } else if ( ref_id < 100 ) {
            foreach (double d, ccw_items.at(ref_id).wavelength) {
                wvls << qRound(d * 10);
            }
        } else {
            foreach (double d, refscan_files.at(ref_id - 100).wavelength) {
                wvls << qRound(d * 10);
            }
        }
        bool ok = true;
        foreach (double val, ccw_items.at(ii).wavelength) {
            int ival = qRound(val * 10);
            if (! wvls.contains(ival)) {
                ok = false;
                break;
            }
        }
        if (! ok) {
            tb_triple->item(ii, 0)->setBackground(QBrush(Qt::yellow));
            tb_triple->item(ii, 1)->setBackground(QBrush(Qt::yellow));
        }
    }
}

void US_ConvertScan::update_shifts(int item_row){
    if (ccw_items.isEmpty()) return;

    QVector<int> rawdata_ids = ccw_items.at(item_row).rawdata_ids;
    double x_1 = ccw_items.at(item_row).minmax_x.first;
    double x_2 = ccw_items.at(item_row).minmax_x.second;
    if (x_1 <= 0 || x_2 <= 0) {
        return;
    }

    for (int ii = 0; ii < rawdata_ids.size(); ii++) {
        int rid = rawdata_ids.at(ii);
        if (absorbance_state.at(rid) == -1) {
            continue;
        }
        US_DataIO::RawData rawdata = intensity_data.at(rid);
        int nn = rawdata.scanCount() - nscans;
        if (nn < 0) {
            nn = 0;
        }
        QVector<double> xvalues = rawdata.xvalues;
        for (int jj = nn; jj < rawdata.scanCount(); jj++) {
            double miny = 1e99;
            for (int kk = 0; kk < rawdata.pointCount(); kk++){
                double x = xvalues.at(kk);
                double val = absorbance_data.at(rid).at(jj).at(kk);
                if (x >= x_1 && x <= x_2) {
                    miny = qMin(miny, val);
                }
            }
            absorbance_shifts[rid][jj] = -1 * miny;
        }
    }
}

QVector<double> US_ConvertScan::smooth_refscan(QVector<double> array, int winlen,
                                               bool ave, bool intsy, double maxVal){
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
                if (rp[i + j] > (maxVal - 0.1)){
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

bool US_ConvertScan::linear_interpolation(const QVector<double>& xt_vals,
                                          QVector<double>& x_vals,
                                          QVector<double>& y_vals) {
    if (x_vals.size() != y_vals.size()) return false;
    const double *xtp = xt_vals.constBegin();
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
    // x_vals.clear();
    // x_vals << xt_vals;
    y_vals.clear();
    y_vals << yo_vals;
    return true;
}

