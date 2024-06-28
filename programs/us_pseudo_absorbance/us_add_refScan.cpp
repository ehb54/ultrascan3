#include "us_add_refScan.h"
#include <QTextStream>

US_AddRefScan::US_AddRefScan() : US_Widgets()
{
    setPalette( US_GuiSettings::frameColorDefault() );
    dbCon = new US_DB2();

    // Put the Run Info across the entire window
    QLabel* lb_run = us_banner(tr("Load the Run(s)"));
    pb_import = us_pushbutton(tr("Import Data"));
    pb_reset = us_pushbutton(tr("Reset"), false);
    QHBoxLayout* import_lyt = new QHBoxLayout();
    import_lyt->addWidget(pb_import);
    import_lyt->addWidget(pb_reset);

    QLabel* lb_rid = us_label(tr("RunID(s):"));
    QHBoxLayout* rid_lyt = new QHBoxLayout();
    rid_lyt->addWidget(lb_rid);
    rid_lyt->addStretch(1);
    lb_runIDs = us_label("", -1);
    lb_runIDs->setWordWrap(true);
    lb_runIDs->setAlignment(Qt::AlignTop);
    lb_runIDs->setMinimumHeight(100);
    lb_runIDs->setStyleSheet(tr("border: 1px solid black;"
                                "border-radius: 5px;"
                                "padding: 2px;"
                                "color: black;"
                                "background-color: white;"));

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

    QHBoxLayout* wavl_rng_lyt = new QHBoxLayout();
    wavl_rng_lyt->addWidget(lb_lambstrt);
    wavl_rng_lyt->addWidget(le_lambstrt);
    wavl_rng_lyt->addWidget(lb_lambstop);
    wavl_rng_lyt->addWidget(le_lambstop);

    QHBoxLayout* wavl_plt_lyt = new QHBoxLayout();
    wavl_plt_lyt->addWidget(lb_lambplot);
    wavl_plt_lyt->addWidget(cb_plot_id);
    wavl_plt_lyt->addWidget(pb_prev_id);
    wavl_plt_lyt->addWidget(pb_next_id);

    // processing control
    QLabel* lb_cluster = us_banner(tr("Processing Control"));
    ckb_cluster = new QCheckBox();
    QGridLayout* ckb_cls_lyt = us_checkbox("Cluster Scans", ckb_cluster);
    ckb_cluster->setCheckState(Qt::Unchecked);
    ckb_align = new QCheckBox();
    QGridLayout* ckb_aln_lyt = us_checkbox("Align Scans", ckb_align);
    ckb_align->setCheckState(Qt::Unchecked);
    pb_clscltr = us_pushbutton("Clustering Control", false, 0 );
    pb_clscltr->setDisabled(true);
    // pb_clscltr->setFixedWidth(150);

    // QHBoxLayout* cls_aln_lyt = new QHBoxLayout();
    // cls_aln_lyt->addLayout(ckb_cls_lyt);
    // cls_aln_lyt->addLayout(ckb_aln_lyt);

    // Chromatic Aberration Correction
    QLabel* lb_CA = us_banner(tr("Chromatic Aberration Correction"));
    ckb_CA_state = new QCheckBox();
    QGridLayout* ckb_CA_state_lyt = us_checkbox("Do Not Correct", ckb_CA_state);
    ckb_CA_state->setCheckState(Qt::Checked);
    ckb_CA_local = new QCheckBox();
    QGridLayout* ckb_CA_source_lyt = us_checkbox("Local Disk", ckb_CA_local);
    ckb_CA_local->setCheckState(Qt::Unchecked);
    pb_loadCA = us_pushbutton("Load", true, 0 );
//    pb_loadCA->setStyleSheet("QPushButton { background-color: yellow};");
    ckb_CA_state->setDisabled(true);
    ckb_CA_local->setDisabled(true);
    pb_loadCA->setDisabled(true);
    // QHBoxLayout *CA_lyt = new QHBoxLayout();
    // CA_lyt->addLayout(ckb_CA_state_lyt);
    // CA_lyt->addLayout(ckb_CA_source_lyt);
    // CA_lyt->addWidget(pb_loadCA);

    lb_CA->hide();
    ckb_CA_state->hide();
    for (int ii = 0; ii < ckb_CA_state_lyt->count(); ii++) {
        ckb_CA_state_lyt->itemAt(ii)->widget()->hide();
    }
    for (int ii = 0; ii < ckb_CA_source_lyt->count(); ii++) {
        ckb_CA_source_lyt->itemAt(ii)->widget()->hide();
    }
    pb_loadCA->hide();


    // save refScan control
    // QLabel* lb_save = us_banner(tr("Saving Control"));
    // dkdb_ctrl = new US_Disk_DB_Controls();
    // dkdb_ctrl->set_disk();

    // lb_dir = us_label( tr( "Directory:" ), -1 );
    // le_dir = us_lineedit( US_Settings::workBaseDir(), -1, true );
    // lb_dbName   = us_label( tr( "Database:" ), -1 );
    // le_dbName  = us_lineedit( "", -1, true );
    pb_save = us_pushbutton("Save", false, 0 );
    pb_save->setDisabled(true);
    // pb_save->setFixedWidth(150);

    // if (dkdb_ctrl->db()){
    //     lb_dir->hide();
    //     le_dir->hide();
    // }else{
    //     lb_dbName->hide();
    //     le_dbName->hide();
    // }
    // QHBoxLayout* dir_lyt = new QHBoxLayout();
    // dir_lyt->addWidget(lb_dir);
    // dir_lyt->addWidget(le_dir);
    // QHBoxLayout* db_lyt = new QHBoxLayout();
    // db_lyt->addWidget(lb_dbName);
    // db_lyt->addWidget(le_dbName);


    QGridLayout* procctrl_lyt = new QGridLayout();
    procctrl_lyt->addLayout(ckb_cls_lyt, 0, 0, 1, 1);
    procctrl_lyt->addLayout(ckb_aln_lyt, 0, 1, 1, 1);
    procctrl_lyt->addWidget(pb_clscltr,  1, 0, 1, 1);
    procctrl_lyt->addWidget(pb_save,     1, 1, 1, 1);
    // status
    QLabel* lb_status = us_label(tr("Status:"));
    le_status = us_lineedit(tr(""), -1, true);
    QHBoxLayout* status_lyt = new QHBoxLayout();
    status_lyt->addWidget(lb_status);
    status_lyt->addWidget(le_status);
    QPalette stpal;
    stpal.setColor( QPalette::Text, Qt::white );
    stpal.setColor( QPalette::Base, Qt::blue  );
    le_status->setPalette( stpal );

    // close
    pb_close = us_pushbutton("Close", true, 0 );
    QPushButton* pb_help = us_pushbutton("Help", true, 0 );
    QHBoxLayout* close_lyt = new QHBoxLayout();
    close_lyt->addWidget(pb_help);
    close_lyt->addWidget(pb_close);

    // layout
    QVBoxLayout* left_lyt = new QVBoxLayout();
    left_lyt->addStretch(0);
    left_lyt->setSpacing(1);
    left_lyt->addWidget(lb_run);
    left_lyt->addLayout(import_lyt);
    left_lyt->addLayout(rid_lyt);
    left_lyt->addWidget(lb_runIDs);
    left_lyt->addWidget(lb_mwlctrl);
    left_lyt->addLayout(wavl_rng_lyt);
    left_lyt->addLayout(wavl_plt_lyt);
    left_lyt->addWidget(lb_cluster);
    left_lyt->addLayout(procctrl_lyt);
    left_lyt->addWidget(pb_clscltr, 0, Qt::AlignCenter);
    // left_lyt->addWidget(lb_CA);
    // left_lyt->addLayout(CA_lyt);
    // left_lyt->addWidget(lb_save);
    // left_lyt->addLayout(dkdb_ctrl);
    // left_lyt->addLayout(dir_lyt);
    // left_lyt->addLayout(db_lyt);
    left_lyt->addWidget(pb_save, 0, Qt::AlignCenter);
    //    left_lyt->setStretch(0);
    //    left_lyt->addSpacing(20);
    left_lyt->addStretch(1);
    left_lyt->addLayout(status_lyt);
    left_lyt->addLayout(close_lyt);


    //*****tabs*****//
    QwtText xLabel, yLabel;
    lb_wavl = us_label("Wavelength= ", 2);
    lb_wavl->setAlignment(Qt::AlignCenter);
    // tab 0
    US_Plot* tab0_usplotL = new US_Plot( tab0_plotLU, tr( "" ),
                                         tr( "Radius (in cm)" ), tr( "Intensity" ),
                                         true, "", "rainbow" );
//    tab0_plotLU->setMaximumSize(400, 300 );
    tab0_plotLU->setMinimumSize(400, 200);
    tab0_plotLU->enableAxis( QwtPlot::xBottom, true );
    tab0_plotLU->enableAxis( QwtPlot::yLeft  , true );
    tab0_plotLU->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(tab0_plotLU);

    US_Plot* tab0_usplotR = new US_Plot( tab0_plotRU, tr( "" ),
                                         tr( "Radius (in cm)" ), tr( "Intensity" ),
                                         true, "", "rainbow" );
//    tab0_plotRU->setMaximumSize( 400, 300 );
    tab0_plotRU->setMinimumSize( 400, 200 );
    tab0_plotRU->enableAxis( QwtPlot::xBottom, true );
    tab0_plotRU->enableAxis( QwtPlot::yLeft  , true );
    tab0_plotRU->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(tab0_plotRU);

    US_Plot* tab0_devplotL = new US_Plot(tab0_plotLD, tr(""),
                                         tr( "Radius (in cm)" ), tr( "Deviation" ));
//    tab0_plotLD->setMinimumSize( 600, 400 );
    tab0_plotLD->setMaximumHeight(250);
    tab0_plotLD->enableAxis( QwtPlot::xBottom, true );
    tab0_plotLD->enableAxis( QwtPlot::yLeft, true );
    tab0_plotLD->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(tab0_plotLD);

    US_Plot* tab0_devplotR = new US_Plot(tab0_plotRD, tr(""),
                                         tr( "Radius (in cm)" ), tr( "Deviation" ));
//    tab0_plotRD->setMinimumSize( 600, 400 );
    tab0_plotRD->setMaximumHeight(250);
    tab0_plotRD->enableAxis( QwtPlot::xBottom, true );
    tab0_plotRD->enableAxis( QwtPlot::yLeft, true );
    tab0_plotRD->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(tab0_plotRD);

    QWidget* tab0 = new QWidget();
    QGridLayout* tab0_lyt = new QGridLayout(tab0);
    tab0_lyt->addWidget(lb_wavl,       0, 0, 1, 2);
    tab0_lyt->addLayout(tab0_usplotL,  1, 0, 1, 1);
    tab0_lyt->addLayout(tab0_usplotR,  1, 1, 1, 1);
    tab0_lyt->addLayout(tab0_devplotL, 2, 0, 1, 1);
    tab0_lyt->addLayout(tab0_devplotR, 2, 1, 1, 1);
    tab0_lyt->setMargin(0);
    tab0_lyt->setSpacing(1);


    // tab 1
    lb_wavlBw = us_label(tr(""), 1);
    lb_wavlBw->setAlignment(Qt::AlignCenter);
    US_Plot* tab1_usplotLU = new US_Plot(tab1_plotLU, tr(""),
                                         tr("RMSD of Scans" ), tr("Distribution"));
    tab1_plotLU->setMinimumHeight(200);
    tab1_plotLU->enableAxis( QwtPlot::xBottom, true );
    tab1_plotLU->enableAxis( QwtPlot::yLeft, true );
    tab1_plotLU->setCanvasBackground( Qt::white );

    US_Plot* tab1_usplotRU = new US_Plot(tab1_plotRU, tr(""),
                                         tr("RMSD of Scans" ), tr("Intensity of Scans"));
    tab1_plotRU->setMinimumHeight(200);
    tab1_plotRU->enableAxis( QwtPlot::xBottom, true );
    tab1_plotRU->enableAxis( QwtPlot::yLeft, true );
    tab1_plotRU->setCanvasBackground( Qt::white );

    QLabel* lb_tab1_rmsd = us_label(tr("RMSD Overlaps"), 1);
    lb_tab1_rmsd->setAlignment(Qt::AlignCenter);
    US_Plot* tab1_usplotLD = new US_Plot(tab1_plotLD, tr(""),
                                         tr("RMSD of Scans" ), tr("Wavelength (in nm)"));
    tab1_plotLD->setMinimumHeight(200);
    tab1_plotLD->enableAxis( QwtPlot::xBottom, true );
    tab1_plotLD->enableAxis( QwtPlot::yLeft, true );
    tab1_plotLD->setCanvasBackground( Qt::white );

    QLabel* lb_tab1_mean = us_label(tr("Intensity Overlaps"), 1);
    lb_tab1_mean->setAlignment(Qt::AlignCenter);
    US_Plot* tab1_usplotRD = new US_Plot(tab1_plotRD, tr(""),
                                         tr("Intensity of Scans" ), tr("Wavelength (in nm)"));
    tab1_plotRD->setMinimumHeight(200);
    tab1_plotRD->enableAxis( QwtPlot::xBottom, true );
    tab1_plotRD->enableAxis( QwtPlot::yLeft, true );
    tab1_plotRD->setCanvasBackground( Qt::white );

    QHBoxLayout* tab1_plt_lyt1 = new QHBoxLayout();
    tab1_plt_lyt1->addLayout(tab1_usplotLU);
    tab1_plt_lyt1->addLayout(tab1_usplotRU);
    QHBoxLayout* tab1_plt_lyt2 = new QHBoxLayout();
    tab1_plt_lyt2->addWidget(lb_tab1_rmsd);
    tab1_plt_lyt2->addWidget(lb_tab1_mean);
    QHBoxLayout* tab1_plt_lyt3 = new QHBoxLayout();
    tab1_plt_lyt3->addLayout(tab1_usplotLD);
    tab1_plt_lyt3->addLayout(tab1_usplotRD);

    QSize sl;
    QLabel* lb_bws = us_label(tr("Bandwidth Scale:"));
    sl = lb_bws->size();
    ct_bws = us_counter(1, 1, 5, 2);
    ct_bws->setSingleStep(0.25);
    ckb_bws_all = new QCheckBox();
    QGridLayout *ckb_bws_lyt = us_checkbox("All Wavelength", ckb_bws_all);
    ckb_bws_all->setMinimumHeight(sl.height());
    pb_reset_bws = us_pushbutton(tr("Reset"), false);
    pb_reset_bws->setMinimumHeight(sl.height());

    QLabel* lb_merge = us_label(tr("Number of Neighbors:"));
    ct_winlen = us_counter(1, 0, winlen_max, winlen_dflt);
    ct_winlen->setSingleStep(1);
    pb_find_merge = us_pushbutton(QString("Find and Merge"), false);
    pb_find_merge->setMinimumHeight(sl.height());

    QHBoxLayout* tab1_plt_lyt4 = new QHBoxLayout();
    tab1_plt_lyt4->addStretch(1);
    tab1_plt_lyt4->addWidget(lb_bws);
    tab1_plt_lyt4->addWidget(ct_bws);
    tab1_plt_lyt4->addLayout(ckb_bws_lyt);
    tab1_plt_lyt4->addWidget(pb_reset_bws);
    tab1_plt_lyt4->addWidget(lb_merge);
    tab1_plt_lyt4->addWidget(ct_winlen);
    tab1_plt_lyt4->addWidget(pb_find_merge);
    tab1_plt_lyt4->addStretch(1);

    QWidget* tab1 = new QWidget();
    QVBoxLayout* tab1_lyt = new QVBoxLayout(tab1);
    tab1_lyt->addWidget(lb_wavlBw, 0);
    tab1_lyt->addLayout(tab1_plt_lyt1, 1);
    tab1_lyt->addLayout(tab1_plt_lyt2, 0);
    tab1_lyt->addLayout(tab1_plt_lyt3, 1);
    tab1_lyt->addLayout(tab1_plt_lyt4);
    tab1_lyt->setMargin(0);
    tab1_lyt->setSpacing(1);

    //**//
    tabs = new QTabWidget();
//    tabs->setAutoFillBackground(true);
    tabs->addTab(tab0, tr("Plots"));
    tabs->addTab(tab1, tr("Clustering Control"));
//    tabs->setTabShape(QTabWidget::Triangular);
    tabs->tabBar()->setMinimumWidth(300);
    QStringList styleSheet;
    styleSheet << "QTabWidget::pane {border-top: 2px solid #C2C7CB;}";
    styleSheet << "QTabBar::tab:selected {font-size: 10pt; font-weight: bold; color: black;}";
    styleSheet << "QTabBar::tab:!selected {font-size: 9pt; font-weight: normal; color: black;}";
    tabs->setStyleSheet(styleSheet.join(" "));

    QHBoxLayout* main_lyt = new QHBoxLayout(this);
    main_lyt->addLayout(left_lyt, 0);
    main_lyt->addWidget(tabs, 1);
    main_lyt->setSpacing(1);
    main_lyt->setMargin(1);
    setLayout(main_lyt);

    slt_reset();
    //
    connect(pb_import,  SIGNAL(clicked()), this, SLOT(slt_import()));
    connect(pb_reset,   SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_prev_id, SIGNAL(clicked()), this, SLOT(slt_prev_id()));
    connect(pb_next_id, SIGNAL(clicked()), this, SLOT(slt_next_id()));
    connect(pb_clscltr, SIGNAL(clicked()), this, SLOT(slt_cls_ctrl()));
    connect(pb_save,    SIGNAL(clicked()), this, SLOT(slt_save()));
    connect(pb_loadCA,    SIGNAL(clicked()), this, SLOT(slt_load_CA()));
    connect(ckb_CA_state, SIGNAL(stateChanged(int)), this, SLOT(slt_CA_state(int)));
    // connect(dkdb_ctrl,  SIGNAL(changed(bool)),  this, SLOT(slt_db_local(bool)));
    connect(this,  SIGNAL(sig_plot_l(bool)),    this, SLOT(slt_plot_l(bool)));
    connect(this,  SIGNAL(sig_plot_r(bool)),    this, SLOT(slt_plot_r(bool)));
    connect(this,  SIGNAL(sig_plot_dist(bool)), this, SLOT(slt_plot_dist(bool)));
    connect(this,  SIGNAL(sig_plot_inty(bool)), this, SLOT(slt_plot_inty(bool)));
    connect(this,  SIGNAL(sig_plot_ovlp(bool)), this, SLOT(slt_plot_ovlp(bool)));
}

//***SLOTS***//

void US_AddRefScan::slt_import(){
    QFileDialog* fdialog;
    QStringList dir_list;
    fdialog = new QFileDialog(this, Qt::Dialog);
    fdialog->setDirectory(US_Settings::importDir());
    fdialog->setFileMode(QFileDialog::DirectoryOnly);
    fdialog->setOption(QFileDialog::DontUseNativeDialog, true);
    fdialog->resize(800, 600);
    fdialog->setWindowTitle("Choose Directories Containing AUC Data");

    QListView* fview = fdialog->findChild<QListView*>("");
    if (fview)
        fview->setSelectionMode(QAbstractItemView::MultiSelection);
    QTreeView* tview = fdialog->findChild<QTreeView*>();
    if (tview)
        tview->setSelectionMode(QAbstractItemView::MultiSelection);
    if (fdialog->exec())
        dir_list = fdialog->selectedFiles();
    delete fdialog;
    if (dir_list.size() > 1){
        QFileInfo dinfo(dir_list.at(1));
        if (dinfo.path() == dir_list.at(0))
            dir_list.removeAt(0);
    }

    if (dir_list.size() == 0)
        return;
    pb_import->setDisabled(true);
    this->setCursor(QCursor(Qt::BusyCursor));
    QString dir_name;
    QDir dir;
    dir.setSorting(QDir::Name);
    dir.setFilter(QDir::Files);
    dir.setNameFilters(QStringList("*.auc"));
    QStringList files_path;
    runIDs.clear();

    bool runID_changed = false;
    QString status;
    QStringList runID_old, runID_new;
    QStringList runTypeList;
    for (int i = 0; i < dir_list.size(); ++i){
        dir_name = dir_list.at(i);
        dir.setPath(dir_name);
        dir.makeAbsolute();
        QFileInfoList f_info = dir.entryInfoList();
        for (int j = 0; j < f_info.size(); ++j){
            files_path << f_info.at(j).absoluteFilePath();
            QString fname = f_info.at(j).fileName();
            QString rtp = fname.section(".", -5, -5);
            QString rid = fname.section(".", 0, -6);
            QString rido = fname.section(".", 0, -6);
            QRegExp re( "[^A-Za-z0-9_-]" );
            int reIdx = rid.indexOf(re, 0);
            if (reIdx >= 0) runID_changed = true;
            while (reIdx >= 0){
                rid = rid.replace(reIdx, 1, "_");
                reIdx = rid.indexOf(re, reIdx);
            }
            if (runID_changed)
                if (!runID_new.contains(rid)){
                    runID_new << rid;
                    runID_old << rido;
                }
            if (!runIDs.contains(rid))
                runIDs << rid;
            if (!runTypeList.contains(rtp))
                runTypeList << rtp;
        }
    }

    if (runID_new.size() > 0){
        QString mess = QString("The following runID(s) name changed:\n");
        for (int i = 0; i < runID_new.size(); ++i)
            mess = mess.append(runID_old.at(i)).append("->").
                    append(runID_new.at(i)).append("\n");

        QMessageBox::warning( this, tr( "Warning: runID(s) Name Changed !" ), mess);
    }

    if (files_path.size() == 0){
        QMessageBox::warning( this, tr( "Error" ), tr("AUC file(s) not found !"));
        status = "Error: AUC file(s) not found !";
        le_status->setText(status);
        pb_import->setEnabled(true);
        this->setCursor(QCursor(Qt::ArrowCursor));
        return;
    }

    if (runTypeList.size() > 1){
        QMessageBox::warning( this, tr( "Error" ), tr("Multiple run types found !"));
        status = "Error: multiple run types found !";
        le_status->setText(status);
        pb_import->setEnabled(true);
        this->setCursor(QCursor(Qt::ArrowCursor));
        return;
    }else
        runType = runTypeList.at(0);

    if (! parse_files(files_path)){
        le_status->setText("Wrong files");
        pb_import->setEnabled(true);
        this->setCursor(QCursor(Qt::ArrowCursor));
        return;
    }

    mean_rmsd();
    estimate_bw();
    status = "wavelengths= %1, Scans= %2";
    le_status->setText(status.arg(n_wavls).arg(n_scans));
    lb_runIDs->setText(runIDs.join(", "));

    connect(ckb_cluster,   SIGNAL(stateChanged(int)), this, SLOT(slt_cls_state(int)));
    connect(ckb_align,     SIGNAL(stateChanged(int)), this, SLOT(slt_aln_state(int)));
    connect(this,          SIGNAL(sig_cluster(int)),  this, SLOT(slt_cluster(int)));
    connect(ct_bws,        SIGNAL(valueChanged(double)), this, SLOT(slt_new_bws(double)));
    connect(pb_find_merge, SIGNAL(clicked()),            this, SLOT(slt_find_merge()));
    connect(ct_winlen,     SIGNAL(valueChanged(double)), this, SLOT(slt_new_wlen(double)));
    connect(pb_reset_bws,  SIGNAL(clicked()),            this, SLOT(slt_reset_bws()));

    set_wavl_ctrl();
    hasData = true;
    pb_reset->setEnabled(true);
    ckb_CA_state->setEnabled(true);
    ckb_CA_local->setEnabled(true);
    emit ckb_CA_state->stateChanged(ckb_CA_state->checkState());
    this->setCursor(QCursor(Qt::ArrowCursor));
    return;
}

void US_AddRefScan::slt_reset(){
    emit sig_cluster(cls_state::OFF);
    emit sig_plot_l(false);
    emit sig_plot_r(false);
    emit sig_plot_dist(false);
    emit sig_plot_inty(false);
    emit sig_plot_ovlp(false);
    cb_plot_id->disconnect();
    ckb_align->disconnect();
    ckb_cluster->disconnect();
    ct_winlen->disconnect();
    ct_bws->disconnect();
    pb_find_merge->disconnect();
    pb_reset_bws->disconnect();
    lb_runIDs->clear();

    hasData = false;
    wavl_id = 0;
    n_scans = 0;
    n_wavls = 0;
    n_points = 0;
    runIDs.clear();
    runType.clear();
    winlen = winlen_dflt;
    wavelength.clear();
    wavlScid.clear();
    wavlScid_S.clear();
    scanWavl.clear();
    scanWavl_W.clear();
    scanWavl_S.clear();
    xvalues.clear();
    wavlBw.clear();
    wavlBwS.clear();
    scanRvalues.clear();
    scanMean.clear();
    scanRmsd.clear();
    clusterIDs.clear();
    clusterRng.clear();
    le_status->clear();
    le_lambstrt->clear();
    le_lambstop->clear();
    cb_plot_id->clear();
    referenceScans.clear();
    CAValues.clear();
    pb_clscltr->setDisabled(true);
    pb_prev_id->setDisabled(true);
    pb_next_id->setDisabled(true);
    cb_plot_id->setDisabled(true);
    ckb_cluster->setCheckState(Qt::Unchecked);
    ckb_cluster->setDisabled(true);
    ckb_align->setCheckState(Qt::Unchecked);
    ckb_align->setDisabled(true);
    pb_save->setDisabled(true);
    ckb_bws_all->setCheckState(Qt::Unchecked);
    pb_reset->setDisabled(true);
    pb_import->setEnabled(true);
    ckb_CA_local->setCheckState(Qt::Unchecked);
    ckb_CA_state->setDisabled(true);
    ckb_CA_local->setDisabled(true);
    return;
}



void US_AddRefScan::slt_prev_id(void){
    --wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
    return;
}

void US_AddRefScan::slt_next_id(void){
    ++wavl_id;
    cb_plot_id->setCurrentIndex(wavl_id);
    return;
}

void US_AddRefScan::slt_set_id(int id){
    wavl_id = id;
    pb_prev_id->setDisabled(wavl_id <= 0);
    pb_next_id->setDisabled(wavl_id >= (n_wavls - 1));
    ct_bws->setValue(wavlBwS.at(wavl_id));
    get_current(wavl_id);
    emit sig_plot_l(true);
    emit sig_plot_r(true);
    emit sig_plot_dist(true);
    emit sig_plot_inty(true);
    emit sig_plot_ovlp(true);
    return;
}

void US_AddRefScan::slt_new_bws(double val){
    if (ckb_bws_all->isChecked())
        wavlBwS.fill(val);
    else
        wavlBwS[wavl_id] = val;
    emit sig_cluster(cls_state::ON);
    get_current(wavl_id);
    emit sig_plot_l(true);
    emit sig_plot_r(true);
    emit sig_plot_dist(true);
    return;
}

void US_AddRefScan::slt_new_wlen(double val){
    winlen = val;
    emit sig_cluster(cls_state::ON);
    emit sig_plot_ovlp(true);
    return;
}

void US_AddRefScan::slt_cls_state(int state){
    if (state == Qt::Checked)
        emit sig_cluster(cls_state::ON);
    else
        emit sig_cluster(cls_state::OFF);
    get_current(wavl_id);
    emit sig_plot_l(true);
    emit sig_plot_r(true);
    return;
}

void US_AddRefScan::slt_cluster(int state){
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    QString style = tr("background-color: %1;");
    referenceScans.clear();
    if (state == cls_state::ON){
        pb_clscltr->setEnabled(true);
        pb_clscltr->setStyleSheet(style.arg("yellow"));
        pb_find_merge->setEnabled(true);
        pb_find_merge->setStyleSheet(style.arg("yellow"));
        scanWavl_S.clear();
        wavlScid_S.clear();
        ct_bws->setEnabled(true);
        ct_winlen->setEnabled(true);
        ckb_bws_all->setEnabled(true);
        pb_reset_bws->setEnabled(true);
        pb_save->setEnabled(false);
    }else if (state == cls_state::OFF){
        pb_clscltr->setEnabled(false);
        pb_clscltr->setStyleSheet(style.arg(color.name()));
        pb_find_merge->setEnabled(false);
        pb_find_merge->setStyleSheet(style.arg(color.name()));
        ct_bws->setEnabled(false);
        ct_winlen->setEnabled(false);
        ckb_bws_all->setEnabled(false);
        pb_reset_bws->setEnabled(false);
        pb_save->setEnabled(true);
    }else if (state == cls_state::DONE){
        pb_clscltr->setStyleSheet(style.arg("green"));
        pb_find_merge->setStyleSheet(style.arg("green"));
        pb_save->setEnabled(true);
    }
    return;
}

void US_AddRefScan::slt_cls_ctrl(void){
    tabs->setCurrentIndex(1);
    return;
}

void US_AddRefScan::slt_aln_state(int){
    emit sig_plot_r(true);
    return;
}

void US_AddRefScan::slt_reset_bws(void){
    if (ckb_bws_all->isChecked())
        wavlBwS.fill(bws_dflt, n_wavls);
    else
        wavlBwS[wavl_id] = bws_dflt;
    ct_bws->setValue(bws_dflt);
    return;
}

void US_AddRefScan::slt_find_merge(void){
    this->setCursor(QCursor(Qt::BusyCursor));
    scanWavl_W.clear();
    scanWavl_W << scanWavl;
    find_clusters();
    merge_clusters();
    wavlScid_S.clear();
    QVector<int> ids_wl_i;
    for (int i = 0; i < n_wavls; ++i){
        ids_wl_i.clear();
        for (int j = 0; j < n_scans; ++j){
            if (wavelength.at(i) == scanWavl_S.at(j))
                ids_wl_i.append(j);
        }
        wavlScid_S.append(ids_wl_i);
    }
    get_current(wavl_id);
    emit sig_cluster(cls_state::DONE);
    emit sig_plot_r(true);
    this->setCursor(QCursor(Qt::ArrowCursor));
    return;
}

void US_AddRefScan::slt_save(void){
    referenceScans.clear();
    if (! ckb_CA_state->isChecked() && CAValues.size() == 0){
        QMessageBox::warning(this, tr("Error!"), tr("Chromatic aberration data "
                                                    "not found!"));
        return;
    }


    if (ckb_CA_state->isChecked()){
        referenceScans.CAState = false;
        referenceScans.CAValues.fill(0, n_wavls);
    } else {
        referenceScans.CAState = true;
        referenceScans.CAValues << CAValues;
    }

    this->setCursor(QCursor(Qt::BusyCursor));

    char ct[2] = {'R', 'I'};
    qstrncpy(referenceScans.type, ct, 3);
    referenceScans.nWavelength = n_wavls;
    referenceScans.nPoints = n_points;
    referenceScans.xValues << xvalues;

    QString status = "Preparing: %1 %2";
    QString percent;
    for (int i = 0; i < n_wavls; ++i){
        get_current(i);
        referenceScans.wavelength << wavelength.at(i) / 10.0;
        referenceScans.rValues << current.ref_S;
        referenceScans.std << get_std(current.dev_S_aln);
        percent = QString::number(100.0 * (i + 1) / n_wavls, 'f', 1);
        le_status->setText(status.arg(percent).arg(QChar(37)));
        qApp->processEvents();
    }
    this->setCursor(QCursor(Qt::ArrowCursor));

    // if (dkdb_ctrl->db())
    //     save_db(referenceScans);
    // else
    //     save_local(referenceScans);

    save_local(referenceScans);
    return;
}

// void US_AddRefScan::slt_db_local(bool status){
//     if (status){
//         lb_dir->hide();
//         le_dir->hide();
//         lb_dbName->show();
//         le_dbName->show();
//         check_connection();
//     }else{
//         lb_dir->show();
//         le_dir->show();
//         lb_dbName->hide();
//         le_dbName->hide();
//     }

//     return;
// }

void US_AddRefScan::slt_plot_l(bool state){
    tab0_plotLU->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    tab0_plotLD->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        lb_wavl->setText("Wavelength=");
        tab0_plotLU->setTitle(tr(""));
        grid = us_grid(tab0_plotLU);
        tab0_plotLU->replot();
        grid = us_grid(tab0_plotLD);
        tab0_plotLD->replot();
        return;
    }

    QPen refpen = QPen(Qt::red);
    refpen.setWidth(4);
    QPen nopen = QPen(Qt::NoBrush, 0, Qt::NoPen);
    //** left up raw scans plot
    double wl = wavelength.at(wavl_id) / 10.0;
//    QVector<double> xval = xvalues.at(lambda_id);
    double *x, *r;
    x = xvalues.data();
    QVector<double> rval;
    QVector<double> ref;
    QVector<double> dev;

    const double min_x = plotParams.value("min_x");
    const double max_x = plotParams.value("max_x");
    const double min_r = plotParams.value("min_r");
    const double max_r = plotParams.value("max_r");
    const double minmax_d = plotParams.value("minmax_d");
    double dx = 0.1 * (max_x - min_x);
    double dr = 0.1 * (max_r - min_r);
    double dv = 0.0 * minmax_d;

    int ns = current.scid.size();
    const int *idp = current.scid.data();
    for (int i = 0; i < ns; ++i){
        rval = scanRvalues.at(idp[i]);
        r = rval.data();
        QwtPlotCurve* curve1 = us_curve( tab0_plotLU,"");
        curve1->setSamples(x, r, n_points);
        rval.clear();
    }
    ref = current.ref;
    r = ref.data();
    QwtPlotCurve* curve2 = us_curve( tab0_plotLU,"");
    curve2->setPen(refpen);
    curve2->setSamples(x, r, n_points);

    tab0_plotLU->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    tab0_plotLU->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    tab0_plotLU->updateAxes();
    lb_wavl->setText(tr("wavelength= %1 nm").arg(wl));
    tab0_plotLU->setTitle(tr("number of scans= %1").arg(ns));
    grid = us_grid(tab0_plotLU);
    tab0_plotLU->replot();
    //** left down raw deviation plot
    for (int i = 0; i < ns; ++i){
        dev = current.dev.at(i);
        r = dev.data();
        QwtSymbol *symbol1 = new QwtSymbol(
                    QwtSymbol::Ellipse, QBrush(Qt::green),
                    QPen(Qt::green, 0), QSize(1, 1));
        QwtPlotCurve* curve3 = us_curve( tab0_plotLD,"");
        curve3->setPen(nopen);
        curve3->setSamples(x, r, n_points);
        curve3->setSymbol(symbol1);
        dev.clear();
    }
    tab0_plotLD->setAxisScale( QwtPlot::xBottom, min_x - dx / 10, max_x + dx / 10);
    tab0_plotLD->setAxisScale( QwtPlot::yLeft  , -minmax_d - dv, minmax_d + dv);
    tab0_plotLD->updateAxes();
    grid = us_grid(tab0_plotLD);
    tab0_plotLD->replot();
    return;
}

void US_AddRefScan::slt_plot_r(bool state){
    tab0_plotRU->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    tab0_plotRD->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    int ns = current.scid_S.size();
    if (! state || ns == 0){
        tab0_plotRU->setTitle(tr(""));
        grid = us_grid(tab0_plotRU);
        tab0_plotRU->replot();
        grid = us_grid(tab0_plotRD);
        tab0_plotRD->replot();
        return;
    }

    QPen refpen = QPen(Qt::red);
    refpen.setWidth(4);
    QPen nopen = QPen(Qt::NoBrush, 0, Qt::NoPen);
//    QVector<double> xval = xvalues.at(lambda_id);
    double *x, *r;
    x = xvalues.data();
    QVector<double> rval_s;
    QVector<double> ref_s;
    QVector<double> dev_s;

    const double min_x = plotParams.value("min_x");
    const double max_x = plotParams.value("max_x");
    const double min_r = plotParams.value("min_r");
    const double max_r = plotParams.value("max_r");
    const double minmax_ds  = plotParams.value("minmax_ds");
    const double minmax_dsa  = plotParams.value("minmax_dsa");
    double dx = 0.1 * (max_x - min_x);
    double dr = 0.1 * (max_r - min_r);
    double dvs = 0.0 * minmax_ds;
    double dvsa = 0.0 * minmax_dsa;

    //*** right up selected scans plot

    const int *idp = current.scid_S.data();
    for (int i = 0; i < ns; ++i){
        rval_s = scanRvalues.at(idp[i]);
        r = rval_s.data();
        if (ckb_align->isChecked()){
            for (int j = 0; j < n_points; ++j)
                r[j] -= current.mean_S.at(i) - current.aveMean_S;
        }
        QwtPlotCurve* curve4 = us_curve( tab0_plotRU,"");
        curve4->setSamples(x, r, n_points);
        rval_s.clear();
    }
    ref_s = current.ref_S;
    r = ref_s.data();
    {QwtPlotCurve* curve5 = us_curve( tab0_plotRU,"");
    curve5->setPen(refpen);
    curve5->setSamples(x, r, n_points);}
    tab0_plotRU->setAxisScale( QwtPlot::xBottom, min_x - dx, max_x + dx);
    tab0_plotRU->setAxisScale( QwtPlot::yLeft  , min_r - dr, max_r + dr);
    tab0_plotRU->updateAxes();
    tab0_plotRU->setTitle(tr("number of scans= %1").arg(ns));
    grid = us_grid(tab0_plotRU);
    tab0_plotRU->replot();
    //** right down selected deviation plot

    for (int i = 0; i < ns; ++i){
        if (ckb_align->isChecked())
            dev_s = current.dev_S_aln.at(i);
        else
            dev_s = current.dev_S.at(i);
        r = dev_s.data();
        QwtSymbol *symbol2 = new QwtSymbol(
                    QwtSymbol::Ellipse, QBrush(Qt::green),
                    QPen(Qt::green, 0), QSize(1, 1));
        QwtPlotCurve* curve6 = us_curve( tab0_plotRD,"");
        curve6->setPen(nopen);
        curve6->setSymbol(symbol2);
        curve6->setSamples(x, r, n_points);
        dev_s.clear();
    }
    tab0_plotRD->setAxisScale( QwtPlot::xBottom, min_x - dx / 10, max_x + dx / 10);
    if (ckb_align->isChecked())
        tab0_plotRD->setAxisScale( QwtPlot::yLeft  , -minmax_dsa - dvsa, minmax_dsa + dvsa);
    else
        tab0_plotRD->setAxisScale( QwtPlot::yLeft  , -minmax_ds - dvs, minmax_ds + dvs);
    tab0_plotRD->updateAxes();
    grid = us_grid(tab0_plotRD);
    tab0_plotRD->replot();
    return;
}

void US_AddRefScan::slt_plot_dist(bool state){
    tab1_plotLU->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        tab1_plotLU->replot();
        return;
    }
    QVector<QVector<double>> rmsd_kde;
    QMap<QString, QVector<int>> minmax_loc;
    double bw, bws;
    bw = wavlBw.at(wavl_id);
    bws = wavlBwS.at(wavl_id);
    rmsd_kde = kde(current.rmsd, bw * bws);
    double const *xx = rmsd_kde.at(0).data();
    double const *yy = rmsd_kde.at(1).data();
//    int sx = rmsd_kde.at(0).size();
    minmax_loc = loc_minmax(rmsd_kde.at(2));
    int np = rmsd_kde.at(0).size();
    //  curve 1: kde
    QwtPlotCurve* curve1 = new QwtPlotCurve();
    curve1->attach(tab1_plotLU);
    curve1->setSamples(xx, yy, np);
    QPen pen = QPen(Qt::SolidPattern, 4, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
    pen.setColor(Qt::black);
//    curve->setPen( Qt::blue, 2 );
    curve1->setPen(pen);
    curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true );
//    curve->setCurveAttribute(QwtPlotCurve::Fitted, true);

    QPolygonF points;
    QPen nopen = QPen(Qt::NoBrush, 0, Qt::NoPen);
    // curve 2: rmsd vline
    QwtSymbol *symbol = new QwtSymbol(
                QwtSymbol::VLine, QBrush(Qt::yellow),
                QPen(Qt::darkGray, 2), QSize(1, 10));
    QwtPlotCurve* curve2 = new QwtPlotCurve();
    curve2->attach(tab1_plotLU);
    curve2->setPen(nopen);
    double maxpdf = *std::max_element(yy, yy + np);
    for (int i = 0; i < current.rmsd.size(); ++i)
        points << QPointF(current.rmsd.at(i), maxpdf * 0.01);
    curve2->setSamples(points);
    curve2->setSymbol(symbol);

    QVector<int> loc;
    // curve 3: min loc
    QwtSymbol *symbol_min = new QwtSymbol(
                QwtSymbol::VLine, QBrush(Qt::yellow),
                QPen(Qt::red, 3), QSize( 1, 15 ));
    QwtPlotCurve* curve3 = new QwtPlotCurve();
    curve3->attach(tab1_plotLU);
    curve3->setPen(nopen);
    loc.clear();
    loc = minmax_loc["minloc"];
    points.clear();
    points << QPointF(xx[0], 0) << QPointF(xx[np - 1], 0);
    for (int i = 0; i < loc.size(); ++i)
        points << QPointF(xx[loc.at(i)], 0);
    curve3->setSamples(points);
    curve3->setSymbol(symbol_min);

    // curve 4: max loc
    QwtSymbol *symbol_max = new QwtSymbol(
                QwtSymbol::Ellipse, QBrush(Qt::yellow),
                QPen(Qt::red, 2), QSize( 8, 8 ));
    QwtPlotCurve* curve4 = new QwtPlotCurve();
    curve4->attach(tab1_plotLU);
    curve4->setPen(nopen);
    points.clear();
    loc = minmax_loc["maxloc"];
    for (int i = 0; i < loc.size(); ++i)
        points << QPointF(xx[loc.at(i)], yy[loc.at(i)]);
    curve4->setSamples(points);
    curve4->setSymbol(symbol_max);

    QString title;
    double sk = skewness(current.rmsd);
    title = "Wavelength= %1 nm, skewness=%2, Bandwidth= %3";
    double wl = wavelength.at(wavl_id) / 10.0;
    QString s1 = QString::number(sk, 'f', 3);
    QString s2 = QString::number(bw * bws, 'f', 3);
    lb_wavlBw->setText(title.arg(wl).arg(s1, s2));
    lb_wavlBw->show();
    tab1_plotLU->replot();
    return;
}

//void US_AddRefScan::slt_plot_dr(bool state){
//    tab1_plotRU->detachItems(QwtPlotItem::Rtti_PlotItem, false);
//    if (! state){
//        tab1_plotRU->replot();
//        return;
//    }
//    if (current.rmsd_S.size() == 0){
//        tab1_plotRU->replot();
//        return;
//    }
//    QVector<QVector<double>> rmsd_kde;
//    double bw, bws;
//    bw = bwe_sj(current.rmsd_S);
//    bws = wavlBwS.at(wavl_id);
//    rmsd_kde = kde(current.rmsd, bw * bws);
//    double const *xx = rmsd_kde.at(0).data();
//    double const *yy = rmsd_kde.at(1).data();
//    int np = rmsd_kde.at(0).size();
//    //  curve 1: kde
//    QwtPlotCurve* curve1 = new QwtPlotCurve();
//    curve1->attach(tab1_plotRU);
//    curve1->setSamples(xx, yy, np);
//    QPen pen = QPen(Qt::SolidPattern, 4, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
//    pen.setColor(Qt::black);
////    curve->setPen( Qt::blue, 2 );
//    curve1->setPen(pen);
//    curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true );
////    curve->setCurveAttribute(QwtPlotCurve::Fitted, true);

//    QPolygonF points;
//    QPen nopen = QPen(Qt::NoBrush, 0, Qt::NoPen);
//    // curve 2: rmsd vline
//    QwtSymbol *symbol = new QwtSymbol(
//                QwtSymbol::VLine, QBrush(Qt::yellow),
//                QPen(Qt::darkGray, 2), QSize(1, 10));
//    QwtPlotCurve* curve2 = new QwtPlotCurve();
//    curve2->attach(tab1_plotRU);
//    curve2->setPen(nopen);
//    double maxpdf = *std::max_element(yy, yy + np);
//    for (int i = 0; i < current.rmsd.size(); ++i)
//        points << QPointF(current.rmsd.at(i), maxpdf * 0.01);
//    curve2->setSamples(points);
//    curve2->setSymbol(symbol);

//    QString title;
//    double sk = skewness(current.rmsd);
//    title = "Wavelength= %1 nm, skewness=%2, Bandwidth= %3";
//    double wl = wavelength.at(wavl_id) / 10.0;
//    QString s1 = QString::number(sk, 'f', 3);
//    QString s2 = QString::number(bw, 'f', 3);
//    lb_wavlBw->setText(title.arg(wl).arg(s1, s2));
//    lb_wavlBw->show();
//    tab1_plotRU->replot();
//    return;
//}

void US_AddRefScan::slt_plot_inty(bool state){
    tab1_plotRU->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        tab1_plotRU->replot();
        return;
    }

    int ns = current.rmsd.size();
    QVector<double> mean = current.mean;
    QVector<double> rmsd = current.rmsd;
    QVector<int> ids = arange(ns);
    //sort by mean
//    std::stable_sort(ids.begin(), ids.end(), [&mean](int i1, int i2){return  mean.at(i1) > mean.at(i2);});
    //sort by rmsd
    std::stable_sort(ids.begin(), ids.end(), [&rmsd](int i1, int i2){return  rmsd.at(i1) > rmsd.at(i2);});
    const double *cmp = current.mean.data();
    const double *crp = current.rmsd.data();
    double *rp = rmsd.data();
    double *mp = mean.data();
    for (int i = 0; i < ns; ++i){
        rp[i] = crp[i];
        mp[i] = cmp[i];
    }
    //  curve
    QwtSymbol *symbol = new QwtSymbol(
                QwtSymbol::Ellipse, QBrush(Qt::black),
                QPen(Qt::black, 2), QSize(4, 4));
    QwtPlotCurve* curve = new QwtPlotCurve();
    QPen pen = QPen(Qt::NoBrush, 0, Qt::NoPen);
    curve->attach(tab1_plotRU);
    curve->setPen(pen);
    curve->setSamples(rp, mp, ns);
    curve->setSymbol(symbol);
    tab1_plotRU->replot();
    return;
}

void US_AddRefScan::slt_plot_ovlp(bool state){
    tab1_plotLD->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    tab1_plotRD->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    if (! state){
        tab1_plotLD->replot();
        tab1_plotRD->replot();
        return;
    }
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
    color_list << color["black"] << color["blue"] << color["orange"];
    color_list << color["green"] << color["cyan"] << color["red"];
    color_list << color["purple"] << color["pink"] << color["yellow"];
    QPen pen = QPen(Qt::SolidPattern, 1, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);

    QVector<int> window_ids;
    int n = -1 * winlen;
    window_ids.append(wavl_id);
    int wl = wavelength.at(wavl_id);
    const int dwl = 10 * winlen;
    for (int i = 0; i < 2 * winlen + 1; ++i){
        if (n != 0 && 0 <= (wavl_id + n) && (wavl_id + n) < n_wavls){
            int delta = qAbs(wavelength.at(wavl_id + n) - wl);
            if (delta <= dwl)
                window_ids.append(wavl_id + n);
        }
        ++n;
    }

    const double *sc_rmsd = scanRmsd.data();
    const double *sc_mean = scanMean.data();
    double rmsd, mean;
    int min_wl   = 10000;
    int max_wl   = 0;
    double min_rmsd =  1e20;
    double max_rmsd = -1e20;
    double min_mean =  1e20;
    double max_mean = -1e20;
    QVector<int> scid;
    QVector<double> xr(2);
    QVector<double> xm(2);
    QVector<double> y(2);
    double *xrp = xr.data();
    double *xmp = xm.data();
    double *yp = y.data();
    int id;
    for (int i = 0; i < window_ids.size(); ++i){
        id = window_ids.at(i);
        scid = wavlScid.at(id);
        wl = wavelength.at(id);
        min_wl = qMin(min_wl, wl);
        max_wl = qMax(max_wl, wl);
        for (int j = 0; j < scid.size(); ++j){
            rmsd = sc_rmsd[scid.at(j)];
            mean = sc_mean[scid.at(j)];
            min_rmsd = qMin(min_rmsd, rmsd);
            max_rmsd = qMax(max_rmsd, rmsd);
            min_mean = qMin(min_mean, mean);
            max_mean = qMax(max_mean, mean);
            xr.fill(rmsd);
            xm.fill(mean);
            yp[0] = wl / 10.0 - 0.4;
            yp[1] = wl / 10.0 + 0.4;
            QwtPlotCurve* curve1 = new QwtPlotCurve();
            curve1->attach(tab1_plotLD);
            curve1->setSamples(xrp, yp, 2);
            pen.setColor(color_list.at(i));
            curve1->setPen(pen);
            curve1->setRenderHint(QwtPlotItem::RenderAntialiased, true );

            QwtPlotCurve* curve2 = new QwtPlotCurve();
            curve2->attach(tab1_plotRD);
            curve2->setSamples(xmp, yp, 2);
            pen.setColor(color_list.at(i));
            curve2->setPen(pen);
            curve2->setRenderHint(QwtPlotItem::RenderAntialiased, true );
        }
        scid.clear();
    }
    double dxr = (max_rmsd - min_rmsd) * 0.1;
    tab1_plotLD->setAxisScale( QwtPlot::xBottom, min_rmsd - dxr, max_rmsd + dxr);
    tab1_plotLD->setAxisScale( QwtPlot::yLeft  , min_wl / 10.0 - 1, max_wl / 10.0 + 1);
    tab1_plotLD->updateAxes();

    double dxm = (max_mean - min_mean) * 0.1;
    tab1_plotRD->setAxisScale( QwtPlot::xBottom, min_mean - dxm, max_mean + dxm);
    tab1_plotRD->setAxisScale( QwtPlot::yLeft  , min_wl / 10.0 - 1, max_wl / 10.0 + 1);
    tab1_plotRD->updateAxes();

    tab1_plotLD->replot();
    tab1_plotRD->replot();
    return;
}

void US_AddRefScan::slt_CA_state(int state){
    CAValues.clear();
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (state == Qt::Checked){
        pb_loadCA->setStyleSheet(qs.arg(color.name()));
        pb_loadCA->setEnabled(false);
    } else {
        pb_loadCA->setStyleSheet(qs.arg("yellow"));
        pb_loadCA->setEnabled(true);
    }
}

void US_AddRefScan::slt_load_CA(){
    QString qs = "QPushButton { background-color: %1 }";
    pb_loadCA->setStyleSheet(qs.arg("yellow"));
    CAValues.clear();
    if (ckb_CA_local->isChecked()){
        QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                        US_Settings::workBaseDir(),
                                                        tr("Text File (*.dat *.txt)"));
        if (filename.isEmpty())
            return;

        QFile file{filename};

        if (!file.open(QIODevice::ReadOnly)) {
          QMessageBox::warning(this, "Error!",
                               tr("Cannot open file for reading.\n%1").arg(filename));
          return ;
        }
        QVector<int> CA_lambda;
        QVector<double> CA_values;
        QTextStream inStream{&file};
        while (!inStream.atEnd()) {

          QString line = inStream.readLine();
          QStringList lsp = line.split(QRegExp("[\r\n\t,; ]+"), Qt::SkipEmptyParts);
          if (lsp.size() == 0) continue;
          if (lsp.size() != 2){
              QMessageBox::warning(this, "Error!",
                                   tr("Cannot parse the file! "
                                      "Each line must contain two values separated by "
                                      "one the following characters.\n"
                                      "space , tab , ',' , ';'\n"
                                      "\n%1").arg(filename));
              return ;
          }
          bool st1, st2;
          int x = qRound(lsp.at(0).trimmed().toDouble(&st1) * 10);
          double y = lsp.at(1).trimmed().toDouble(&st2);
          if (st1 && st2){
              CA_lambda << x;
              CA_values << y;
          }
        }
        QStringList badWavls;
        for (int i = 0; i < n_wavls; ++i){
            int wavl = wavelength.at(i);
            int id = CA_lambda.indexOf(wavl);
            if (id == -1)
                badWavls << QString::number(wavl / 10.0);
            else
                CAValues << CA_values.at(id);
        }
        if (badWavls.size() > 0){
            CAValues.clear();
            QMessageBox::warning(this, "Error!",
                                 tr("Data for correcting chromatic aberration not found "
                                    "for these wavelengths:\n") +
                                 badWavls.join(','));
            return ;
        }
        pb_loadCA->setStyleSheet(qs.arg("green"));
    } else {
        return;
    }

}


//*********//

QVector<double> US_AddRefScan::get_std(QVector<QVector<double>> dev){
    QVector<double> vecStd;
    double std;
    int ns = dev.size();
    for (int i = 0; i < n_points; ++i){
        std = 0;
        for (int j = 0; j < ns; ++j)
            std += qPow(dev.at(j).at(i), 2);
        vecStd << qSqrt(std / (ns - 1));
    }
    return vecStd;
}

void US_AddRefScan::write2txt(const QString& file, US_RefScanDataIO::RefData& data){
    QFile out_file(file);
    QString sval;
    double dval;
    int nw = data.nWavelength;
    int np = data.nPoints;
    if (out_file.open(QIODevice::WriteOnly)) {
        QTextStream out_str{&out_file};
        out_str << "Type=" << data.type << Qt::endl;
        out_str << "nWavelength=" << nw << Qt::endl;
        out_str << "nPoints=" << np << Qt::endl;
        out_str << "Wavelength=";
        for (int i = 0; i < nw; ++i){
            dval = data.wavelength.at(i);
            sval = QString::number(dval, 'f', 1);
            if (i < nw - 1)
                out_str << sval << ",";
            else
                out_str << sval << "\n";
        }
        for (int i = 0; i < nw; ++i){
            out_str << "xValues=";
            for (int j = 0; j < np; ++j){
                dval = data.xValues.at(j);
                sval = QString::number(dval, 'f', 4);
                if (j < np - 1)
                    out_str << sval << ",";
                else
                    out_str << sval << "\n";
            }
        }
        for (int i = 0; i < nw; ++i){
            out_str << "rValues=";
            for (int j = 0; j < np; ++j){
                dval = data.rValues.at(i).at(j);
                sval = QString::number(dval, 'e', 6);
                if (j < np - 1)
                    out_str << sval << ",";
                else
                    out_str << sval << "\n";
            }
        }
        for (int i = 0; i < nw; ++i){
            out_str << "std=";
            for (int j = 0; j < np; ++j){
                dval = data.std.at(i).at(j);
                sval = QString::number(dval, 'e', 6);
                if (j < np - 1)
                    out_str << sval << ",";
                else
                    out_str << sval << "\n";
            }
        }
        }
    return;
}

void US_AddRefScan::set_wavl_ctrl(void){
    cb_plot_id->disconnect();
    cb_plot_id->clear();
    wavl_id = 0;
    le_lambstrt->setText(QString::number(wavelength.at(0) / 10.0));
    le_lambstop->setText(QString::number(wavelength.at(n_wavls - 1) / 10.0));
    QString clamb;
    for (int i = 0; i < n_wavls; ++i){
        clamb = QString::number(wavelength.at(i) / 10.0);
        cb_plot_id->addItem(clamb);
    }
    cb_plot_id->setCurrentIndex(wavl_id);
    cb_plot_id->setEnabled(true);
    ckb_align->setEnabled(true);
    ckb_cluster->setEnabled(true);
    ct_winlen->setValue(winlen_dflt);
    pb_save->setEnabled(true);
    connect(cb_plot_id, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_id(int)));
    slt_set_id(wavl_id);
    return;
}

bool US_AddRefScan::parse_files(QStringList files_path){
    int n_files = files_path.size();
    QString status = "Parsing Files: %1 %2";
    QString percent;
    xvalues.clear();
    QVector<double> xval;
    int min_x = -1, max_x = -1;
    bool flag_check_xval = true;
    for (int i = 0; i < n_files; ++i){
        US_DataIO::RawData raw_data;
        US_DataIO::readRawData(files_path.at(i), raw_data);
        percent = QString::number(100.0 * (i + 1) / n_files, 'f', 1);
        le_status->setText(status.arg(percent).arg(QChar(37)));
        qApp->processEvents();
        QString rawT;
        rawT.append(raw_data.type[0]).append(raw_data.type[1]);
        if (rawT != runType){
            QMessageBox::information( this,
                                      tr( "Error" ), tr("The run type of the following binary data "
                                                        "does not match the filename !\n%1").arg(
                                                        files_path.at(0)));
            return false;
        }

        if (flag_check_xval){
            int x1 = qRound(raw_data.xvalues.first() * 1000);
            int x2 = qRound(raw_data.xvalues.last() * 1000);
            if (min_x == -1 && max_x == -1){
                min_x = x1;
                max_x = x2;
            }
            if (min_x != x1 || max_x != x2){

                QMessageBox msgBox;
                QString text = tr( "Radial point arrays do not match. "
                                  "This might be due to the chromatic "
                                  "aberration correction!\n"
                                  "If \"Yes\" is clicked, the program proceeds "
                                  "with the radial points of the first parsed file.\n\n"
                                  "First file:\nName:%1\n\n"
                                  "Current file:\nName:%2\n").arg(
                                       files_path.at(0), files_path.at(i));
                msgBox.setText(text);
                msgBox.setInformativeText("Do you want to proceed?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::No);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes){
                    flag_check_xval = false;
                } else {
                    return false;
                }
            }
        }

        double *xp = raw_data.xvalues.data();
        int id_b, id_e;
        int jj = 0;
        while ( jj < raw_data.pointCount() - 2){
            if ( qRound(xp[jj] * 1000) < 5800 ) ++jj;
            else break;
        }
        if ( qRound(xp[jj] * 1000) != 5800){
            QMessageBox::information( this,
                                      tr( "Error" ),
                                      tr( "Radial point at 5.8 cm not found in this file: \n%1\n"
                                          "This might be due to chromatic aberration correction!").arg(
                                                      files_path.at(i)));
            return false;
        }
        id_b = jj;
        int np = 0;
        while ( jj < raw_data.pointCount() - 2){
            if ( qRound(xp[jj] * 1000) < 7200 ){
                ++np;
                if (xvalues.size() == 0)
                    xval.append(xp[jj]);
                ++jj;
            }
            else break;
        }
        if ( qRound(xp[jj] * 1000) != 7200) {
            QMessageBox::information( this,
                                      tr( "Error" ),
                                      tr( "Radial point at 7.2 cm not found in this file: \n%1\n"
                                          "This might be due to chromatic aberration correction!").arg(
                                                      files_path.at(i)));
            return false;
        }
        ++np;
        id_e = jj + 1;
        if (xvalues.size() == 0){
            xval.append(xp[jj]);
            xvalues << xval;
            xval.clear();
            n_points = xvalues.size();
        }
        if (np != n_points){
            QMessageBox::information( this,
                                      tr( "Error" ),
                                      tr( "The current file is not compatible with the first parsed file."
                                          "This might be due to chromatic aberration correction!\n\n"
                                          "First file:\nName:%1\n"
                                          "Number of points in the range of 5.8 to 7.2 (cm): %2\n\n"
                                          "Current file:\nName:%3\n"
                                          "Number of points in the range of 5.8 to 7.2 (cm): %4\n").arg(
                                          files_path.at(0)).arg(n_points).arg(files_path.at(i)).arg(np));
            return false;
        }

        for (int j = 0; j < raw_data.scanData.size(); ++j){
            QVector<double> rval;
            rval.clear();
            const double *rp = raw_data.scanData.at(j).rvalues.data();
            for (int k = id_b; k < id_e; ++k)
                rval << rp[k];
            scanRvalues << rval;
            int wl = qRound(raw_data.scanData.at(j).wavelength * 10);
            scanWavl << wl;
            if (! wavelength.contains(wl))
                wavelength << wl;
        }
    }

    std::sort(wavelength.begin(), wavelength.end());
    n_scans = scanWavl.size();
    n_wavls = wavelength.size();
    QVector<int> ids_wl_i;
    for (int i = 0; i < n_wavls; ++i){
        ids_wl_i.clear();
        for (int j = 0; j < n_scans; ++j)
            if (wavelength.at(i) == scanWavl.at(j))
                ids_wl_i.append(j);
        wavlScid << ids_wl_i;
    }
    return true;
}


void US_AddRefScan::mean_rmsd(void){
    int i, j, k, ns, id;
    double mean, rmsd;
    QVector<double> rval;
    const double *r;
    scanMean.resize(n_scans);
    scanRmsd.resize(n_scans);
    double *avep = scanMean.data();
    double *msdp = scanRmsd.data();
    QString status = "Calculating Mean %1 RMSD: %2 %3";
    QString percent;
    for (i = 0; i < n_wavls; ++i){
        percent = QString::number(100.0 * (i + 1) / n_wavls, 'f', 1);
        le_status->setText(status.arg(QChar(38)).arg(percent).arg(QChar(37)));
        qApp->processEvents();
        ns = wavlScid.at(i).size();
        for (j = 0; j < ns; ++j){
            mean = 0;
            id = wavlScid.at(i).at(j);
            rval = scanRvalues.at(id);
            r = rval.data();
            for (k = 0; k < n_points; ++k)
                mean += r[k];
            mean /= n_points;
            avep[id] = mean;

            rmsd = 0;
            for (k = 0; k < n_points; ++k)
                rmsd += qPow(r[k] - mean, 2);
            rmsd /= n_points;
            msdp[id] = qSqrt(rmsd);
        }
    }
    return;
}

QVector<int> US_AddRefScan::arange(int start, int stop, int step){
    QVector<int> vector;
    while (start < stop) {
        vector.append(start);
        start += step;
    }
    return vector;
}

QVector<int> US_AddRefScan::arange(int start, int stop){
    QVector<int> vector;
    int step = 1;
    while (start < stop) {
        vector.append(start);
        start += step;
    }
    return vector;
}

QVector<int> US_AddRefScan::arange(int stop){
    QVector<int> vector;
    int start = 0;
    int step = 1;
    while (start < stop) {
        vector.append(start);
        start += step;
    }
    return vector;
}

QVector<double> US_AddRefScan::linspace(double start, double stop, int num){
    QVector<double> linspaced;
    if (num == 0)
        return linspaced;
    else if (num == 1){
        linspaced.resize(1);
        linspaced[0] = start;
        return linspaced;
    }
    else
        linspaced.resize(num);
    double delta = (stop - start) / (num - 1);
    QVector<double>::iterator it;
    int n = 0;
    for(it = linspaced.begin(); it != linspaced.end() - 1; ++it)
        (*it) = (start + delta * n++);
    (*it) = stop;
    return linspaced;
}

double US_AddRefScan::pdf(double x, double mu, double sigma){
    const double pi = 3.14159265358979323846;
    double coeff = 1 / (sigma * std::sqrt(2 * pi));
    double y = coeff * std::exp(-0.5 * std::pow((x - mu) / sigma, 2));
    return y;
}

double US_AddRefScan::quantile(QVector<double> x, double q){
    double qnt;
    int size_x = x.size();
    std::sort(x.begin(), x.end());
    double id = (size_x - 1) * q;
    int id1 = (int) std::floor(id);
    int id2 = (int) std::ceil(id);
    if (id1 == id2)
        qnt = x.at(id1);
    else{
        double a = (x.at(id2) - x.at(id1)) / (id2 - id1);
        double b = x.at(id1) - id1 * a;
        qnt = a * id + b;
    }
    return qnt;
}

double US_AddRefScan::skewness(QVector<double> x){
    double q1 = quantile(x, 0.25);
    double q2 = quantile(x, 0.50);
    double q3 = quantile(x, 0.75);
    double left = q2 - q1;
    double right = q3 - q2;
    double sk = (right - left) / (right + left);
    return sk;
}

double US_AddRefScan::variance(QVector<double> x){
    double var = 0;
    double mean = 0;
    int size_x = x.size();
    for (int i = 0; i < size_x; ++i)
        mean += x.at(i);
    mean /= size_x;
    for (int i = 0; i < size_x; ++i)
        var += std::pow(x.at(i) - mean, 2);
    var /= (size_x - 1);
    return var;
}

double US_AddRefScan::bwe_norm(QVector<double> x){
    int size_x = x.size();
    double sv = std::sqrt(variance(x));
    sv = sv * std::pow(4.0 / (3.0 * size_x), (1.0 / 5.0));
    return sv;
}


double US_AddRefScan::sj_phi6(double x){
    double y;
    y = std::pow(x, 6) - 15 * std::pow(x, 4) + 45 * std::pow(x, 2) - 15;
    y *= pdf(x, 0, 1);
    return y;
}

double US_AddRefScan::sj_phi4(double x){
    double y;
    y = std::pow(x, 4) - 6 * std::pow(x, 2) + 3;
    y *= pdf(x, 0, 1);
    return y;
}

double US_AddRefScan::sj_param(QVector<QVector<double>> *W, int size_x,
                             double h, double tdb, double sda){
    int i, j;
    double alpha2, w2, sdalpha2, v;
    alpha2 = 1.357 * std::pow(std::abs(sda / tdb), 1 / 7.0) * std::pow(h, 5 / 7.0);
    sdalpha2 = 0;
    for (j = 0; j < size_x; ++j){
        w2 = 0;
        for (i = 0; i < size_x; ++i)
            w2 += sj_phi4(W->at(i).at(j) / alpha2);
        sdalpha2 += w2;
    }
    sdalpha2 = sdalpha2 / (size_x * (size_x - 1) * std::pow(alpha2, 5));
    v = pdf(0, 0, std::sqrt(2)) / (size_x * std::abs(sdalpha2));
    v = std::pow(v, 0.2) - h;
    return v;
}

double US_AddRefScan::bwe_sj(QVector<double> x){
    int i, j;
    int size_x = x.size();
    double lam, a, b, hstep, tdb, sda, w1, w2, h0, v0, h1, v1, h;
    QVector<QVector<double>> W(size_x);
    lam = quantile(x, 0.75) - quantile(x, 0.25);
    a = 0.92 * lam * std::pow((double)size_x, -1 / 7.0);
    b = 0.912 * lam * std::pow((double)size_x, -1 / 9.0);
    for (i = 0; i < size_x; ++i){
        W[i].resize(size_x);
        for (j = 0; j < size_x; ++j)
            W[i][j] = x.at(j) - x.at(i);
    }

    tdb = 0;
    sda = 0;
    for (j = 0; j < size_x; ++j){
        w1 = 0;
        w2 = 0;
        for (i = 0; i < size_x; ++i){
            w1 += sj_phi6(W.at(i).at(j) / b);
            w2 += sj_phi4(W.at(i).at(j) / a);
        }
        tdb += w1;
        sda += w2;
    }
    tdb = -tdb / (size_x * (size_x - 1) * std::pow(b, 7.0));
    sda = sda / (size_x * (size_x - 1) * std::pow(a, 5.0));
    h0 = bwe_norm(x);
    v0 = sj_param(&W, size_x, h0, tdb, sda);

    if (v0 > 0)
        hstep = 1.1;
    else
        hstep = 0.9;

    h1 = h0 * hstep;
    v1 = sj_param(&W, size_x, h1, tdb, sda);

    while (v1 * v0 > 0){
        h0 = h1;
        v0 = v1;
        h1 = h0 * hstep;
        v1 = sj_param(&W, size_x, h1, tdb, sda);
    }
    h = h0 + (h1 - h0) * std::abs(v0) / (std::abs(v0) + std::abs(v1));
    return h;

}

QVector<QVector<double>> US_AddRefScan::kde(QVector<double> x, double bw){
    int size_x = x.size(), np = 2000, i, j;
    double diff;
    QVector<double> xx(np);
    QVector<double> yy(np, 0);
    QVector<double> sdiff(np);
    QVector<QVector<double>> out;
    double min = *std::min_element(x.constBegin(), x.constEnd());
    double max = *std::max_element(x.constBegin(), x.constEnd());
    double dx = (max - min) * 0.2;
    double start = min - dx;
    double stop = max + dx;
    double delta = (stop - start) / (np - 1);

    double *xp = x.data();
    double *xxp = xx.data();
    double *yyp = yy.data();
    double *dfp = sdiff.data();
    for (i = 0; i < size_x; ++i)
        for (j = 0; j < np; ++j){
            if (i == 0){
                if (j < np - 1)
                    xxp[j] = start + delta * j;
                else
                    xxp[j] = stop;
            }
            yyp[j] += pdf(xxp[j], xp[i], bw) / size_x;
        }
    for (i = 1; i < np; ++i){
        diff = yyp[i] - yyp[i - 1];
        if (diff > 0)
            dfp[i - 1] = 1;
        else if (diff == 0)
            dfp[i - 1] = 0;
        else
            dfp[i - 1] = -1;
    }
    dfp[np - 1] = dfp[np - 2];
    out << xx;
    out << yy;
    out << sdiff;
    return out;
}


QMap<QString, QVector<int>> US_AddRefScan::loc_minmax(QVector<double> sdiff){
    int i;
    double s1, s2;
    QMap<QString, QVector<int>> out;
    QVector<int> zc, minloc, maxloc;
    for (i = 1; i < sdiff.size(); ++i){
        if (sdiff.at(i) != sdiff.at(i - 1)){
            zc << i - 1;
        }
    }
    for (i = 0; i < zc.size(); ++i){
        s1 = sdiff.at(zc.at(i));
        s2 = sdiff.at(zc.at(i) + 1);
        if (s1 > 0 and s2 < 0)
            maxloc.append(zc.at(i));
        else if (s1 < 0 and s2 > 0)
            minloc.append(zc.at(i));
    }
    out["zc"] = zc;
    out["minloc"] = minloc;
    out["maxloc"] = maxloc;
    return out;
}


void US_AddRefScan::estimate_bw(void){
    if (wavlBw.size() > 0 && wavlBwS.size() > 0)
        return;
    wavlBw.fill(0, n_wavls);
    wavlBwS.fill(bws_dflt, n_wavls);
    QVector<double> rmsd_i;
    QVector<int> ids_i;
    int i, j;
    double *bwp = wavlBw.data();
    double *msdp;
    double *sc_msdp = scanRmsd.data();
    QString status = "Bandwidth estimation: %1 %2";
    QString percent;
    for (i = 0; i < n_wavls; ++i){
        rmsd_i.clear();
        ids_i.clear();
        ids_i = wavlScid.at(i);
        rmsd_i.fill(0, ids_i.size());
        msdp = rmsd_i.data();
        for (j = 0; j < ids_i.size(); ++j)
            msdp[j] = sc_msdp[ids_i.at(j)];
        bwp[i] = bwe_sj(rmsd_i);
        percent = QString::number(100.0 * (i + 1) / n_wavls, 'f', 1);
        le_status->setText(status.arg(percent).arg(QChar(37)));
        qApp->processEvents();
    }
    return;
}

void US_AddRefScan::find_clusters(void){
    scanWavl_S.fill(0, n_scans);
    QVector<QVector<double>> rmsd_i_kde;
    QMap<QString, QVector<int>> minmax_loc;
    double bw;
    clusterIDs.clear();
    clusterRng.clear();
    QString status = "Finding clusters: %1 %2";
    QString percent;
    for (int i = 0; i < n_wavls; ++i){
        rmsd_i_kde.clear();
        minmax_loc.clear();
        get_current(i);
        bw = wavlBw.at(i) * wavlBwS.at(i);
        rmsd_i_kde = kde(current.rmsd, bw);
        minmax_loc = loc_minmax(rmsd_i_kde.at(2));
        get_clusters_i(current.rmsd, current.scid, wavelength.at(i), rmsd_i_kde, minmax_loc);
        percent = QString::number(100.0 * (i + 1) / n_wavls, 'f', 1);
        le_status->setText(status.arg(percent).arg(QChar(37)));
        qApp->processEvents();
    }
    return;
}

void US_AddRefScan::get_clusters_i(QVector<double> rmsd_i, QVector<int> ids_i, double wl,
                                 QVector<QVector<double>> kde_out,
                                 QMap<QString, QVector<int>> minmax_loc){
    QVector<double> X = kde_out.at(0);
    QVector<double> Y = kde_out.at(1);
    double min_pdf;
    double r;
    int i, j, k, n;
    int n_ranges = minmax_loc["minloc"].size() + 1;
    QVector<double> ranges_x;
    QVector<double> ranges_y;
    QVector<double> maxloc_y;
    ranges_x.append(0);
    ranges_y.append(0);
    for (i = 0; i < n_ranges - 1; ++i){
        j = minmax_loc["minloc"].at(i);
        ranges_x.append(X.at(j));
        ranges_y.append(Y.at(j));
    }
    ranges_x.append(1e10);
    ranges_y.append(0);
    for (i = 0; i < minmax_loc["maxloc"].size(); ++i){
        j = minmax_loc["maxloc"].at(i);
        maxloc_y.append(Y.at(j));
    }
    QVector<int> ranges_cls_num;
    QVector<QVector<int>> ranges_ids;
    QVector<QVector<double>> ranges_rmsd;
    QVector<int> temp_ids;
    QVector<double> temp_rmsd;

    n = 0;
    for (i = 0; i < n_ranges; ++i){
        temp_ids.clear();
        temp_rmsd.clear();
        for (j = 0; j < rmsd_i.size(); ++j){
            r = rmsd_i.at(j);
            if (r >= ranges_x.at(i) && r < ranges_x.at(i + 1)){
                temp_ids.append(ids_i.at(j));
                temp_rmsd.append(r);
            }
        }
        if (i > 0){
            min_pdf = maxloc_y.at(i - 1) * 1.0;
            if (ranges_y.at(i) < min_pdf)
                ++n;
        }
        ranges_cls_num.append(n);
        ranges_ids.append(temp_ids);
        ranges_rmsd.append(temp_rmsd);
    }
    QVector<double> temp_cls_ids;
    QVector<double> temp_cls_rmsd;
    QVector<QVector<double>> cls_ids;
    QVector<QVector<double>> cls_minmax;
    QVector<double> minmax;
    for (i = 0; i < n + 1; ++i){
        temp_cls_ids.clear();
        temp_cls_rmsd.clear();
        minmax.clear();
        for (j = 0; j < n_ranges; ++j){
            if (ranges_cls_num.at(j) == i){
                temp_rmsd.clear();
                temp_ids.clear();
                temp_ids = ranges_ids.at(j);
                temp_rmsd = ranges_rmsd.at(j);
                for (k = 0; k < temp_ids.size(); ++k){
                    temp_cls_ids.append(temp_ids.at(k));
                    temp_cls_rmsd.append(temp_rmsd.at(k));
                }
            }
        }
        std::sort(temp_cls_ids.begin(), temp_cls_ids.end());
        cls_ids.append(temp_cls_ids);
        minmax.append(*std::min_element(temp_cls_rmsd.constBegin(), temp_cls_rmsd.constEnd()));
        minmax.append(*std::max_element(temp_cls_rmsd.constBegin(), temp_cls_rmsd.constEnd()));
        cls_minmax.append(minmax);
    }
    QVector<int> idx = arange(n + 1);
    std::stable_sort(idx.begin(), idx.end(), [&cls_ids](int i1, int i2){return  cls_ids.at(i1).size() > cls_ids.at(i2).size();});
    QVector<QVector<double>> cls_ids_s;
    QVector<QVector<double>> cls_minmax_s;
    for (i = 0; i < n + 1; ++i){
        cls_ids_s.append(cls_ids.at(idx.at(i)));
        cls_minmax_s.append(cls_minmax.at(idx.at(i)));
        if (i == 0){
            for (j = 0; j < cls_ids_s.at(i).size(); ++j){
                scanWavl_W[cls_ids_s.at(i).at(j)] = 0;
                scanWavl_S[cls_ids_s.at(i).at(j)] = wl;
            }
        }
    }
    clusterIDs.append(cls_ids_s);
    clusterRng.append(cls_minmax_s);
    return ;
}

void US_AddRefScan::merge_clusters(void){
    int n, id, id_j, chk_id;
    double i1, i2, j1, j2;
    int wl_i, wl_j, chk_wl;
    double chk_rmsd, chk_mean;
    double rmsd, mean;
    double rmsd_i_min, rmsd_i_max, dr;
    double mean_i_min, mean_i_max, dm;
    bool merge;
    QVector<double> rmsd_i;
    QVector<double> mean_i;
    QVector<QVector<double>> cls_rng_j;
    QVector<double> cls_ids_j_k;
    QVector<int> window_ids;
    QString status = "Merging clusters: %1 %2";
    QString percent;
    if (winlen == 0){
        le_status->setText(status.arg(100.0).arg(QChar(37)));
        return;
    }
    for (int i = 0; i < n_wavls; ++i){
        percent = QString::number(100.0 * (i + 1) / n_wavls, 'f', 1);
        le_status->setText(status.arg(percent).arg(QChar(37)));
        qApp->processEvents();
        window_ids.clear();
        wl_i = wavelength.at(i);
        n = -1 * winlen;
        const int dwl = 10 * winlen;
        for (int j = 0; j < 2 * winlen + 1; ++j){
            if (n != 0 && 0 <= (i + n) && (i + n) < n_wavls){
                int delta = qAbs(wavelength.at(i + n) - wl_i);
                if (delta <= dwl)
                    window_ids.append(i + n);
            }
            ++n;
        }
        if (window_ids.size() == 0)
            continue;
        i1 = clusterRng.at(i).at(0).at(0);
        i2 = clusterRng.at(i).at(0).at(1);
        rmsd_i.clear();
        mean_i.clear();
        rmsd_i_min =  1e20;
        rmsd_i_max = -1e20;
        mean_i_min =  1e20;
        mean_i_max = -1e20;
        for (int j = 0; j < clusterIDs.at(i).at(0).size(); ++j){
            id = clusterIDs.at(i).at(0).at(j);
            rmsd = scanRmsd.at(id);
            mean = scanMean.at(id);
            rmsd_i.append(rmsd);
            mean_i.append(mean);
            rmsd_i_min = qMin(rmsd_i_min, rmsd);
            rmsd_i_max = qMax(rmsd_i_max, rmsd);
            mean_i_min = qMin(mean_i_min, mean);
            mean_i_max = qMax(mean_i_max, mean);
        }
        dr = (rmsd_i_max - rmsd_i_min) * 0.0;
        rmsd_i_min -= dr;
        rmsd_i_max += dr;
        dm = (mean_i_max - mean_i_min) * 0.0;
        mean_i_min -= dm;
        mean_i_max += dm;
        for (int j = 0; j < window_ids.size(); ++j){
            cls_rng_j.clear();
            id_j = window_ids.at(j);
            cls_rng_j = clusterRng.at(id_j);
            if (cls_rng_j.size() > 1){
                wl_j = wavelength.at(id_j);
                for (int k = 1; k < cls_rng_j.size(); ++k){
                    j1 = cls_rng_j.at(k).at(0);
                    j2 = cls_rng_j.at(k).at(1);
                    if ((j1 <= i1 && i1 < j2) || (i1 <= j1 && j1 < i2)){
                        cls_ids_j_k.clear();
                        cls_ids_j_k = clusterIDs.at(id_j).at(k);
                        for (int l = 0; l < cls_ids_j_k.size(); ++l){
                            chk_id = cls_ids_j_k.at(l);
                            chk_wl = scanWavl_W.at(chk_id);
                            if (chk_wl == wl_j){
                                chk_rmsd = scanRmsd.at(chk_id);
                                chk_mean = scanMean.at(chk_id);
                                merge = chk_rmsd >= rmsd_i_min;
                                merge = merge && (chk_rmsd <= rmsd_i_max);
                                merge = merge && (chk_mean >= mean_i_min);
                                merge = merge && (chk_mean<= mean_i_max);
                                if (merge){
                                    scanWavl_W[chk_id] = 0;
                                    scanWavl_S[chk_id] = wl_i;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return;
}

QVector<bool> US_AddRefScan::trimming(QVector<double> rmsd, double threshold){
    int ns = rmsd.size();
    QVector<bool> tsk(ns, true);
    bool *bp = tsk.data();
    QVector<int> sorted_ids = arange(ns);
    std::stable_sort(sorted_ids.begin(), sorted_ids.end(), [&rmsd](int i1, int i2){return  rmsd.at(i1) < rmsd.at(i2);});
    std::stable_sort(rmsd.begin(), rmsd.end());
    double sk = skewness(rmsd);
    while (std::abs(sk) > threshold){
        if (sk < 0){
            bp[sorted_ids.at(0)] = false;
            sorted_ids.removeFirst();
            rmsd.removeFirst();
            sk = skewness(rmsd);
        } else{
            bp[sorted_ids.at(sorted_ids.size() - 1)] = false;
            sorted_ids.removeLast();
            rmsd.removeLast();
            sk = skewness(rmsd);
        }
    }

    return tsk;
}

QVector<bool> US_AddRefScan::cropping(QVector<double> rmsd, QVector<bool>tsk, double margin){
    int ns = rmsd.size();
    QVector<bool> crop(ns, true);
    bool *bp = tsk.data();
    bool flag;
    QVector<double> rmsd_t;
    for (int i = 0; i < ns; ++i){
        flag = tsk.at(i);
        bp[i] = flag;
        if (flag)
            rmsd_t << rmsd.at(i);
    }
    double q1 = quantile(rmsd_t, margin);
    double q2 = quantile(rmsd_t, 1 - margin);
    for (int i = 0; i < ns; ++i){
        if (rmsd.at(i) < q1)
            bp[i] = false;
        else if(rmsd.at(i) > q2)
            bp[i] = false;
    }
    return crop;
}

void US_AddRefScan::get_current(int id){
    current.clear();

    QVector<int> scid;
    scid << wavlScid.at(id);
    int ns = scid.size();
    QVector<double> mean(ns, 0);
    QVector<double> rmsd(ns, 0);
    const int *idp = scid.data();
    double *mp = mean.data();
    double *rp = rmsd.data();
    const double* scmp = scanMean.data();
    const double* scrp = scanRmsd.data();
    double aveMean = 0;
    double sm;
    for (int i = 0; i < ns; ++i){
        rp[i] = scrp[idp[i]];
        sm = scmp[idp[i]];
        mp[i] = sm;
        aveMean += sm;
    }
    aveMean /= ns;
    QVector<double> ref;
    ref = get_ref_rval(scid);
    QVector<QVector<QVector<double>>> dev;
    dev = get_ref_dev(scid, ref, mean, aveMean);
    current.aveMean = aveMean;
    current.scid << scid;
    current.mean << mean;
    current.rmsd << rmsd;
    current.ref << ref;
    current.dev << dev.at(0);
    current.dev_aln << dev.at(1);
    ////
    if (wavlScid_S.size() == 0){
        if (! ckb_cluster->isChecked()){
            current.aveMean_S = aveMean;
            current.scid_S << scid;
            current.mean_S << mean;
            current.rmsd_S << rmsd;
            current.ref_S << ref;
            current.dev_S << dev.at(0);
            current.dev_S_aln << dev.at(1);
        }
    }else{
        QVector<int> scid_S;
        scid_S << wavlScid_S.at(id);
        ns = scid_S.size();
        QVector<double> rmsd_S(ns, 0);
        idp = scid_S.data();
        rp = rmsd_S.data();
        for (int i = 0; i < ns; ++i)
            rp[i] = scrp[idp[i]];
        QVector<bool> including_ids;
        including_ids = trimming(rmsd_S, 0.15);
        const bool *bp = including_ids.data();

        QVector<double> mean_S;
        rmsd_S.clear();
        scid_S.clear();
        double aveMean_S = 0;

        for (int i = 0; i < ns; ++i){
            if (bp[i]){
                scid_S.append(idp[i]);
                rmsd_S.append(scrp[idp[i]]);
                sm = scmp[idp[i]];
                mean_S.append(sm);
                aveMean_S += sm;
            }
        }
        aveMean_S /= scid_S.size();
        QVector<double> ref_S;
        ref_S = get_ref_rval(scid_S);
        QVector<QVector<QVector<double>>> dev_S;
        dev_S = get_ref_dev(scid_S, ref_S, mean_S, aveMean_S);
        current.aveMean_S = aveMean_S;
        current.scid_S << scid_S;
        current.mean_S << mean_S;
        current.rmsd_S << rmsd_S;
        current.ref_S << ref_S;
        current.dev_S << dev_S.at(0);
        current.dev_S_aln << dev_S.at(1);
    }
    get_plot_params();
    return;
}

void US_AddRefScan::get_plot_params(void){
    double max_x    = -1e20;
    double min_x    =  1e20;
    double max_r    = -1e20;
    double min_r    =  1e20;
    double max_d    = -1e20;
    double min_d    =  1e20;
    double max_ds   = -1e20;
    double min_ds   =  1e20;
    double max_dsa  = -1e20;
    double min_dsa  =  1e20;
    int strt_id = 20;
//    QVector<double> xval = xvalues.at(lambda_id);
    QVector<double> rval;
    QVector<double> dev;
    QVector<double> dev_aln;
    const double *xp = xvalues.data();
    const double *rp, *dp, *dap;
    const int *idp = current.scid.data();
    for (int i = 0; i < current.scid.size(); ++i){
        rval = scanRvalues.at(idp[i]);
        dev = current.dev.at(i);
        rp = rval.data();
        dp = dev.data();
        for (int j = 0; j < n_points; ++j){
            if (i == 0){
                max_x = qMax(max_x, xp[j]);
                min_x = qMin(min_x, xp[j]);
            }
            if (j >= strt_id && j < (n_points - strt_id)){
                max_r = qMax(max_r, rp[j]);
                min_r = qMin(min_r, rp[j]);
                max_d = qMax(max_d, dp[j]);
                min_d = qMin(min_d, dp[j]);
            }
        }
        rval.clear();
        dev.clear();
    }
    /////
    for (int i = 0; i < current.scid_S.size(); ++i){
        dev = current.dev_S.at(i);
        dev_aln = current.dev_S_aln.at(i);
        dp = dev.data();
        dap = dev_aln.data();
        for (int j = 0; j < n_points; ++j){
            if (j >= strt_id && j < (n_points - strt_id)){
                max_ds = qMax(max_ds, dp[j]);
                min_ds = qMin(min_ds, dp[j]);
                max_dsa = qMax(max_dsa, dap[j]);
                min_dsa = qMin(min_dsa, dap[j]);
            }
        }
        dev.clear();
        dev_aln.clear();
    }
    double minmax_d = qMax(qAbs(min_d), qAbs(max_d));
    double minmax_ds = qMax(qAbs(min_ds), qAbs(max_ds));
    double minmax_dsa = qMax(qAbs(min_dsa), qAbs(max_dsa));
    plotParams["min_x"] = min_x;
    plotParams["max_x"] = max_x;
    plotParams["min_r"] = min_r;
    plotParams["max_r"] = max_r;
    plotParams["minmax_d"] = minmax_d;
    plotParams["minmax_ds"] = minmax_ds;
    plotParams["minmax_dsa"] = minmax_dsa;
    return;
}

QVector<double> US_AddRefScan::get_ref_rval(QVector<int> ids){
    int i, j, ns;
    ns = ids.size();
    int *idp = ids.data();
    QVector<double> rval;
    QVector<double> ref_rval(n_points, 0);
    double *rf = ref_rval.data();
    const double *r;
    for (i = 0; i < ns; ++i){
        rval = scanRvalues.at(idp[i]);
        r = rval.data();
        for (j = 0; j < n_points; ++j){
            rf[j] += r[j] / ns;
        }
        rval.clear();
    }
    return ref_rval;
}

QVector<QVector<QVector<double>>> US_AddRefScan::get_ref_dev(QVector<int> ids, QVector<double> ref,
                                                           QVector<double> mean, double aveMean){
    QVector<double> rval;
    QVector<QVector<double>> ref_dev;
    QVector<QVector<double>> ref_dev_aln;
    QVector<double> dev;
    QVector<double> dev_aln;
    int ns = ids.size();
    const int *idp = ids.data();
    const double *mp = mean.data();
    double *rp;
    double r;
    const double *rf = ref.data();
    for (int i = 0; i < ns; ++i){
        rval = scanRvalues.at(idp[i]);
        rp = rval.data();
        for (int j = 0; j < n_points; ++j){
            r = rp[j];
            dev << ( r - rf[j] );
            dev_aln << ( r - mp[i] + aveMean - rf[j] );
        }
        ref_dev << dev;
        ref_dev_aln << dev_aln;
        rval.clear();
        dev.clear();
        dev_aln.clear();
    }
    QVector<QVector<QVector<double>>> output;
    output << ref_dev << ref_dev_aln;
    return output;
}

bool US_AddRefScan::check_runIDs(US_DB2* db, QVector<int>& experimentIDs,
                             QVector<QDate>& runIDs_date, int& instrumentID){
    // Let's see if we can find the run ID
    QString procedure("get_info_for_referenceScan");
    QStringList badRunIDs;
    QStringList badExpType;
    badRunIDs.clear();
    experimentIDs.clear();
    runIDs_date.clear();
    instrumentID = -1;
    QString experimentType;
    for (int i = 0; i < runIDs.size(); ++i){
        QStringList q;
        q.clear();
        q << procedure << runIDs.at(i);
        db->query( q );
        qDebug() << db->numRows();
        if (db->lastErrno() == US_DB2::NOTPERMITTED){
            QString mess("Only admin could add new reference data!");
            QMessageBox::information( this, tr( "Error" ), mess);
            return false;
        }
        if ( db->lastErrno() == US_DB2::NOROWS )
            badRunIDs << runIDs.at(i);
        else{
            db->next();
            experimentIDs << db->value( 0 ).toString().toInt();

            experimentType = db->value( 5 ).toString();
            if (experimentType != "calibration") badExpType << runIDs.at(i);

            int insId = db->value( 3 ).toString().toInt();
            if (instrumentID == -1) instrumentID = insId;
            else{
                if (instrumentID != insId){
                    QString mess("The instrument ID of all RunIDs must be same!");
                    QMessageBox::information( this, tr( "Error" ), mess);
                    return false;
                }
            }
            QString dt_str = db->value( 7 ).toString();
            QDate date_i = str2date(dt_str);
            runIDs_date << date_i;
        }
    }
    qDebug() << experimentIDs << "\n" << instrumentID << experimentType << "\n" << runIDs_date;
    if (badRunIDs.size() > 0){
        QString mess("runID(s) not found in the database:\n");
        for (int i = 0; i < badRunIDs.size(); ++i){
            mess.append("'").append(badRunIDs.at(i)).append("'");
            if (i < badRunIDs.size() - 1)
                mess.append(";\n");
        }
        QMessageBox::information( this, tr( "Error" ), mess);
        return false;
    }
    if (badExpType.size() > 0){
        QString mess("The type of these runID(s) is not 'calibration':\n");
        for (int i = 0; i < badExpType.size(); ++i){
            mess.append("'").append(badExpType.at(i)).append("'");
            if (i < badExpType.size() - 1)
                mess.append(";\n");
        }
        QMessageBox::information( this, tr( "Error" ), mess);
        return false;
    }
    return true;

}

void US_AddRefScan::save_local(US_RefScanDataIO::RefData &refScans){
    // QString outFileName;
    // FileNameWidget FileName(outFileName);
    // FileName.show();
    // int s = FileName.exec();
    // if (s == QDialog::Accepted){
    //     qDebug() << outFileName;
    //     int error = US_RefScanDataIO::writeRefData(outFileName, refScans);
    //     if (error != US_RefScanDataIO::OK)
    //         le_status->setText(US_RefScanDataIO::errorString(error));
    //     else{
    //         le_status->setText("Written on the local disk");
    //         US_RefScanDataIO::RefData refScans2;
    //         US_RefScanDataIO::readRefData(outFileName, refScans2);
    //         qDebug() << "";
    //     }
    // }

    QString fpath = QFileDialog::getSaveFileName(this, "Save Reference Scan File",
                                                       US_Settings::dataDir(), tr("dat (*.dat)"));
    if (fpath.isEmpty()) {
        return;
    }
    if (!fpath.endsWith(".dat", Qt::CaseInsensitive)) {
        fpath.append(".dat");
    }
    QFile file(fpath);

    if (file.open(QIODevice::WriteOnly)) {
        QTextStream textout{&file};
        // textout << "Points(cm)";
        textout << "cm";
        for (int ii = 0; ii < refScans.nWavelength; ii++) {
            // textout << tr(";Lambda(%1)").arg(refScans.wavelength.at(ii));
            textout << tr(";%1nm").arg(refScans.wavelength.at(ii));
        }
        textout << "\n";
        for (int ii = 0; ii < refScans.nPoints; ii++) {
            textout << QString::number(refScans.xValues.at(ii));
            for (int jj = 0; jj < refScans.nWavelength; jj++) {
                textout << ";" << QString::number(refScans.rValues.at(jj).at(ii));
            }
            textout << "\n";
        }
    }
    return;
}

void US_AddRefScan::save_db(US_RefScanDataIO::RefData &refScans){
    QVector<int> experimentIDs;
    QVector<QDate> runIDs_date;
    int instrumentID;
    bool checked = check_runIDs(dbCon, experimentIDs,
                               runIDs_date, instrumentID);
    if (! checked){
        le_status->setText("DB upload failed !");
        return;
    }
    std::sort(experimentIDs.begin(), experimentIDs.end());
    QVector<refScanTableInfo> refScansDBinfo;
    get_refScanDBinfo(dbCon, refScansDBinfo);

    // check for existing experiment ids
    if (refScansDBinfo.size() > 0)
        for (int i = 0; i < refScansDBinfo.size(); ++i){
            QVector<int> expIDs_i = refScansDBinfo.at(i).experimentIDs;
            int id = refScansDBinfo.at(i).id;
            QDate date = refScansDBinfo.at(i).referenceTime;
            bool existing = true;
            for (int j = 0; j < experimentIDs.size(); ++j)
                existing = existing && expIDs_i.contains(experimentIDs.at(j));
            if (existing){
                QMessageBox::warning(this, "Error", tr( "Current data already exists in the database!\n" ) +
                                     tr( "Table name: referenceScan\n" ) +
                                     tr( "id: " ) + QString::number(id) + tr( "\n" ) +
                                     tr( "date: " ) + date.toString());
                return;
            }
        }

    QString expIDs_str;
    int N = experimentIDs.size();
    for (int i = 0; i < N; ++i){
        expIDs_str.append(QString::number(experimentIDs.at(i)));
        if (i < N - 1)
            expIDs_str.append(",");
    }
    QDate refTime(runIDs_date.at(0));
    for (int i = 0; i < runIDs_date.size(); ++i)
        if (runIDs_date.at(i) >= refTime)
            refTime = runIDs_date.at(i);

    setRefTime refTimeWidget(this, refTime);
    int state_refDate = refTimeWidget.exec();
    if (state_refDate != QDialog::Accepted){
        le_status->setText("Data were not saved on the DB!");
        return;
    }

    QStringList q;
    q.clear();
    q << "new_referenceScanRecord";
    q << QString::number(instrumentID);
    q << QString::number(US_Settings::us_inv_ID());
    q << runType;
    q << expIDs_str;
    q << refTime.toString("yyyy-MM-dd");
    q << QString::number(refScans.nWavelength);
    q << QString::number(refScans.nPoints);
    int np_wl = refScans.nWavelength;
    q << QString::number(refScans.wavelength.at(0), 'f', 1);
    q << QString::number(refScans.wavelength.at(np_wl - 1), 'f', 1);
//    qDebug()<<q;

    int state_newRef = dbCon->statusQuery( q );
    QString staterr_newRef = dbCon->lastError();
    int refDataID = dbCon->lastInsertID();
    bool clear_last = false;
    if ( state_newRef == US_DB2::OK ){
        QDir tempDir = QDir::temp();
        QString fname("reference_scans_%1.aucrs");
        qint64 ms = QDateTime::currentMSecsSinceEpoch();
        QFileInfo finfo(tempDir, fname.arg(ms));
        QString fpath = finfo.absoluteFilePath();
        qDebug() << fpath;
        int referr = US_RefScanDataIO::writeRefData(fpath, refScans);
        if (referr == US_RefScanDataIO::OK){
            int db_write = dbCon->writeBlobToDB(fpath, QString( "upload_referenceScanData" ),
                                                   refDataID );
            if (db_write == US_DB2::DBERROR){
                QMessageBox::warning(this, "Error", "Error processing file:\n"
                                     + fpath + "\n" + dbCon->lastError() +
                                     "\n" + "Could not open file or no data \n");
                clear_last = true;
            }else if (db_write != US_DB2::OK){
                QMessageBox::warning(this, "Error", "returned processing file:\n" +
                                     fpath + "\n" + dbCon->lastError() + "\n");
                clear_last = true;
            }
        } else
            clear_last = true;
//        qDebug() << fpath;
        QFile f(fpath);
        if (f.exists())
            f.remove();

        if (clear_last){
            q.clear();
            q << "clear_referenceScanRecord" << QString::number(refDataID);
            dbCon->query(q);
        }
    }else{
        QMessageBox::warning(this, "Error", staterr_newRef);
        return;
    }
    return;
}

void US_AddRefScan::check_connection(){
//    if (dbCon->isConnected())
//        return;

    QString error;
    dbCon->connect(pw.getPasswd(), error);
    // First row
    if (dbCon->isConnected()){
        QStringList DB   = US_Settings::defaultDB();
        // if (DB.isEmpty())
        //     le_dbName->setText("Undefined");
        // else
        //     le_dbName->setText(DB.at(3));
    }
    return;
}

void US_AddRefScan::get_refScanDBinfo(US_DB2* db, QVector<refScanTableInfo>& refTable){
    // Let's see if we can find the run ID
    refTable.clear();
    QStringList q;
    q.clear();
    q << "get_referenceScan_info";
    db->query( q );
    qDebug() << db->numRows();
    if ( db->lastErrno() == US_DB2::NOROWS ){
        qDebug() << "reference table is empty!";
        return;
    }
    while (db->next()){
        refScanTableInfo refScanInfo;
        refScanInfo.id = db->value(0).toString().toInt();
        refScanInfo.instrumentID = db->value(1).toString().toInt();
        refScanInfo.personID = db->value(2).toString().toInt();
        refScanInfo.type = db->value(3).toString();
        QStringList expIDs = db->value(4).toString().split(",");
        QVector<int> expIDs_int;
        foreach (QString str, expIDs)
            expIDs_int << str.toInt();
        refScanInfo.experimentIDs << expIDs_int;
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
    return;
}

QDate US_AddRefScan::str2date(QString date){
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


////////////
/// \brief FileNameWidget::FileNameWidget
/// \param inputName
///

FileNameWidget::FileNameWidget(QString &inputName):US_WidgetsDialog(nullptr, Qt::Dialog){
    setWindowTitle( tr( "Set Reference Scans File Name" ) );
    fileName = &inputName;
    QDateTime currDateTime = QDateTime::currentDateTimeUtc();
    QDate currDate = currDateTime.date();
    QStringList StrList;
    StrList << QString::number(currDate.month()) << QString::number(currDate.day());
    StrList << QString::number(currDate.year());
    importDir = QDir(US_Settings::importDir());

    caution = new QLineEdit();
    caution->setText("");
    caution->setStyleSheet("color: red;  background-color: white");
    caution->setFrame(false);
    caution->setAlignment(Qt::AlignCenter);

    QLabel *lb_dir = us_label("Directory:");
    QLineEdit *le_dir = us_lineedit(US_Settings::importDir(), 1, true);
    QLabel *lb_fname = us_label("File name:");
    le_prep = us_lineedit("ReferenceScan", 1, true);
    base_prev = tr("optima");
    le_base = us_lineedit(base_prev, 1, false);
    le_apen = us_lineedit(StrList.join("-"), 1, true);
    le_ext = us_lineedit(QString(".aucrs"), 1, true);

    le_final = us_lineedit("", 1, true);
    le_final->setStyleSheet("color: black;  background-color: white");
    le_final->setFrame(false);

    QPushButton *pb_ok = us_pushbutton("Apply");
    QPushButton *pb_cancel = us_pushbutton("Cancel");

    QGridLayout *gl = new QGridLayout();
    gl->addWidget(lb_dir,    0, 0, 1, 1);
    gl->addWidget(le_dir,    0, 1, 1, 4);
    gl->addWidget(lb_fname,  1, 0, 1, 1);
    gl->addWidget(le_prep,   1, 1, 1, 1);
    gl->addWidget(le_base,   1, 2, 1, 1);
    gl->addWidget(le_apen,   1, 3, 1, 1);
    gl->addWidget(le_ext,    1, 4, 1, 1);
    gl->addWidget(le_final,  2, 1, 1, 4);

    QHBoxLayout *hb = new QHBoxLayout();
    hb->addStretch(1);
    hb->addWidget(pb_cancel);
    hb->addWidget(pb_ok);

    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->addWidget(caution);
    vb->addLayout(gl);
    vb->addLayout(hb);
    setLayout(vb);

    le_final->setText(check_fname().fileName());

    connect(pb_cancel, SIGNAL(clicked()), this, SLOT(slt_cancel()));
    connect(pb_ok,     SIGNAL(clicked()), this, SLOT(slt_ok()));
    connect(le_base,   SIGNAL(textEdited(QString)), this, SLOT(slt_edit(QString)));
}

void FileNameWidget::slt_ok(void){
    QString fpath = check_fname().absoluteFilePath();
    *fileName = fpath;
    this->accept();
}

void FileNameWidget::slt_cancel(void){
    *fileName = "";
    this->reject();
}

void FileNameWidget::slt_edit(QString text){
    if (text.size() < base_prev.size()){
        base_prev = text;
    }else{
        QRegExp re( "[^a-zA-Z0-9-]" );
        int reIdx = text.indexOf(re, 0);
        if (reIdx >= 0){
            le_base->setText(base_prev);
            le_base->setCursorPosition(reIdx);
        }else
            base_prev = text;
    }
    le_final->setText(check_fname().fileName());
    return;
}

QFileInfo FileNameWidget::check_fname(void){
    QString fn;
    fn += le_prep->text();
    if (le_base->text().size() > 0)
        fn += "_" + le_base->text();
    fn += "_" + le_apen->text() + le_ext->text();
    QFileInfo fpath(importDir, fn);
    fpath.makeAbsolute();
    if (QFile::exists(fpath.absoluteFilePath()))
        caution->setText("Caution! found the same file name in the directory.");
    else
        caution->setText("");
    return fpath;
}


setRefTime::setRefTime(QWidget* w, QDate& date):
    US_WidgetsDialog(w, Qt::Dialog)
{

    setPalette( US_GuiSettings::frameColor() );
    refDate = &date;
    QFont font = QFont();
    font.setBold(true);

    QStringList months;
    months << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun"
           << "Jul" << "Aug" << "Sep" << "Oct" << "Nov" << "Dec";

    setWindowTitle( "Set Reference Time");
    QDate today = QDate::currentDate();

    cb_year = us_comboBox();
    cb_year->setFont(font);
    int idx = -1;
    for (int i = 2000; i <= today.year(); ++i){
        cb_year->addItem(QString::number(i));
        if (i == refDate->year())
            idx = i - 2000;
    }
    if (idx != -1)
        cb_year->setCurrentIndex(idx);
    else
        cb_year->setCurrentIndex(0);


    cb_month = us_comboBox();
    cb_month->setFont(font);
    idx = -1;
    for (int i = 1; i <= 12; ++i){
        cb_month->addItem(months.at(i - 1));
        if (refDate->month() == i)
            idx = i - 1;
    }
    if (idx != -1)
        cb_month->setCurrentIndex(idx);
    else
        cb_month->setCurrentIndex(0);

    cb_day = us_comboBox();
    slt_set_days(cb_month->currentIndex());
    idx = -1;
    for (int i = 0; i < cb_day->count(); ++i){
        int day = cb_day->itemText(i).toInt();
        if (refDate->day() == day)
            idx = i;
    }
    if (idx != -1)
        cb_day->setCurrentIndex(idx);

    QLabel *text = new QLabel();
    text->setText("The recommended time for reference scans \n based on the runID time is as follows.");
    text->setStyleSheet("color: red;  background-color: white");
//    text->setFont(font);
    text->setAlignment(Qt::AlignCenter);
    text->setLineWidth(2);
    text->setMidLineWidth(2);
    text->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    QPushButton *pb_apply = new QPushButton();
    pb_apply->setText("Apply");
    pb_apply->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
    QPushButton *pb_cancel = new QPushButton();
    pb_cancel->setText("Cancel");
    pb_cancel->setIcon(this->style()->standardIcon(QStyle::SP_DialogCancelButton));

    QHBoxLayout *combo_lyt = new QHBoxLayout();
    combo_lyt->addWidget(cb_month);
    combo_lyt->addWidget(cb_day);
    combo_lyt->addWidget(cb_year);

    QHBoxLayout *close_lyt = new QHBoxLayout();
    close_lyt->addStretch(1);
    close_lyt->addWidget(pb_cancel);
    close_lyt->addWidget(pb_apply);

    QVBoxLayout *main_lyt = new QVBoxLayout(this);
    main_lyt->addWidget(text);
    main_lyt->addLayout(combo_lyt);
    main_lyt->addLayout(close_lyt);

    this->setLayout(main_lyt);

    connect(cb_month, SIGNAL(currentIndexChanged(int)), this, SLOT(slt_set_days(int)));
    connect(pb_cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_apply, SIGNAL(clicked()), this, SLOT(slt_apply()));
}

void setRefTime::slt_set_days(int month){
    month += 1;
    int year = cb_year->currentText().toInt();
    QCalendar cal;
    int n_days = cal.daysInMonth(month, year);
    cb_day->clear();
    for (int i = 1; i <= n_days; ++i)
        cb_day->addItem(QString::number(i));
    cb_day->setCurrentIndex(0);
    QFont font = QFont();
    font.setBold(true);
    cb_day->setFont(font);
    return;
}

void setRefTime::slt_apply(){
    int year = cb_year->currentText().toInt();
    int month = cb_month->currentIndex() + 1;
    int day = cb_day->currentText().toInt();
    refDate->setDate(year, month, day);
    this->accept();
}


