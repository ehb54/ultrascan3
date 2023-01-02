#include "us_mwl_sf_plot3d.h"

void SFData::clear(){
    wavelenghts.clear();
    includedScans.clear();
    xValues.clear();
    scansMSE.clear();
    allData.clear();
}

void SFData::computeMSE(){
    if (allData.size() == 0)
        return;
    scansMSE.clear();
    int nscans = allData.size();
    scansMSE.fill(0, nscans);
    int nwvl = wavelenghts.size();
    for (int i = 0; i < nscans; i++){
        int npoints = allData.at(i).size();
        double mse = 0;
        int n = 0;
        for (int j = 0; j < npoints; j++){
            for (int k = 0; k < nwvl; k++){
                QVector<double> orgSp = allData.at(i).at(j).at(k);
                double se = qPow(orgSp.last() - orgSp.first(), 2);
                mse += se;
                n++;
            }
        }
        mse /= n;
        scansMSE[i] = mse;
    }
}

US_MWL_SF_PLOT3D::US_MWL_SF_PLOT3D(QWidget* w, const SFData& spFitData): US_WidgetsDialog(w)
{
    setWindowTitle("Species Fit Deviation Plot");
    setPalette( US_GuiSettings::frameColorDefault() );
    nScans = spFitData.includedScans.size();
    scanId = nScans / 2;
    nWavelengths = spFitData.wavelenghts.size();
    nPoints = spFitData.xValues.size();
    double coeffER = 1.0;
    double coeffRP = 1.2;
    double coeffWL = 1.0;
    padding = 0.0001;
    idWL_l = 0;
    idWL_h = nWavelengths - 1;
    idRP_l = 0;
    idRP_h = nPoints - 1;

    allData.clear();
    double offset;
    double scale;
    double minValue = 1e99;
    double maxValue = -1e99;
    // scan < radial < lambda  < raw, fit, scaled SE > > > >
    for (int i = 0; i < nScans; i++){
        QVector< QVector < QVector < double > > > data_scan;
        for (int j = 0; j < nPoints; j++){
            QVector < QVector < double > > data_rp;
            for (int k = 0; k < nWavelengths; k++){
                QVector<double> data(3, 0);
                data[0] = spFitData.allData.at(i).at(j).at(k).first();
                data[1] = spFitData.allData.at(i).at(j).at(k).last();
                double sqe = qPow(data[0] - data[1], 2);
                data[2] = sqe;
                minValue = qMin(minValue, sqe);
                maxValue = qMax(maxValue, sqe);
                data_rp << data;
            }
            data_scan << data_rp;
        }
        allData << data_scan;
    }
    offset = minValue;
    scale = (maxValue - minValue);
    for (int i = 0; i < nScans; i++){
        for (int j = 0; j < nPoints; j++){
            for (int k = 0; k < nWavelengths; k++){
                double sqe = allData.at(i).at(j).at(k).last();
                sqe = (sqe - offset) / scale;
                sqe *= coeffER;
                allData[i][j][k][2] = sqe;
            }
        }
    }

    offset = spFitData.xValues.at(0);
    scale = spFitData.xValues.at(nPoints - 1) - spFitData.xValues.at(0);
    xvalsScaled.clear();
    xvals4ct.clear();
    for (int i = 0; i < nPoints; i++){
        double rp = spFitData.xValues.at(i);
        xvals4ct << (int) (rp * 1000);
        rp = (rp - offset) / scale;
        rp *= coeffRP;
        xvalsScaled << rp;
    }

    offset = spFitData.wavelenghts.at(0);
    scale = spFitData.wavelenghts.at(nWavelengths - 1) - spFitData.wavelenghts.at(0);
    lambdaScaled.clear();
    lambda4ct.clear();
    for (int i = 0; i < nWavelengths; i++){
        double wl = spFitData.wavelenghts.at(i);
        lambda4ct << (int) (wl * 10);
        wl = (wl - offset) / scale;
        wl *= coeffWL;
        lambdaScaled << wl;
    }

    dataProxy = new QSurfaceDataProxy();
    dataSeries = new QSurface3DSeries(dataProxy);
    dataSeries->setFlatShadingEnabled(true);
    colorId = G2R;

    xAngle = 30;
    yAngle = 90;
    zAngle = 30;
    graph = new Q3DSurface();
    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    graph->setAxisX(new QValue3DAxis);
    graph->setAxisY(new QValue3DAxis);
    graph->setAxisZ(new QValue3DAxis);
    graph->addSeries(dataSeries);
    graph->axisX()->setTitle("Radial Points (cm)");
    graph->axisX()->setTitleVisible(true);
    graph->axisX()->setLabelFormat("%.3f");
    graph->axisX()->setLabelAutoRotation(xAngle);

    graph->axisY()->setTitle("Squared Error");
    graph->axisY()->setTitleVisible(true);
    graph->axisY()->setLabelFormat("%.1e");
    graph->axisY()->setLabelAutoRotation(yAngle);

    graph->axisZ()->setTitle("Wavelength (nm)");
    graph->axisZ()->setTitleVisible(true);
    graph->axisZ()->setLabelFormat("%d");
    graph->axisZ()->setLabelAutoRotation(zAngle);
//    surface->axisZ()->setReversed(true);

    graph->scene()->activeCamera()->setWrapXRotation(true);
    graph->scene()->activeCamera()->setWrapYRotation(false);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);

    surfaceWgt = QWidget::createWindowContainer(graph);
    QSize screenSize = graph->screen()->size();
    surfaceWgt->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
    surfaceWgt->setMaximumSize(screenSize);
    surfaceWgt->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    surfaceWgt->setFocusPolicy(Qt::StrongFocus);

    QLabel* lb_scan_ctrl = us_banner("Scan Control");
    QLabel* lb_scan = us_label("Scan:");
    cb_scan = us_comboBox();
    for (int i = 0; i < nScans; ++i){
        cb_scan->addItem(QString::number(spFitData.includedScans.at(i) + 1));
    }
    cb_scan->setCurrentIndex(scanId);

    QHBoxLayout* scan_lyt1 = new QHBoxLayout();
    scan_lyt1->addWidget(lb_scan);
    scan_lyt1->addWidget(cb_scan);
    pb_prev = us_pushbutton(tr("Previous"));
    pb_prev->setIcon(US_Images::getIcon(US_Images::ARROW_LEFT));
    pb_next = us_pushbutton(tr("Next"));
    pb_next->setIcon(US_Images::getIcon(US_Images::ARROW_RIGHT));
    QHBoxLayout* scan_lyt2 = new QHBoxLayout();
    scan_lyt2->addWidget(pb_prev);
    scan_lyt2->addWidget(pb_next);

    QGridLayout* mse_lyt = new QGridLayout();
//    QLabel* lb_se = us_banner("Squared Error Data");
    QLabel* lb_mse = us_label("Mean Squared Error:");
    lb_mse->setAlignment(Qt::AlignRight);
    QLabel* lb_minse = us_label("Min of Squared Error:");
    lb_minse->setAlignment(Qt::AlignRight);
    QLabel* lb_maxse = us_label("Max of Squared Error:");
    lb_maxse->setAlignment(Qt::AlignRight);
    le_meanSE = us_lineedit(0, 0, true);
    le_minSE = us_lineedit(0, 0, true);
    le_maxSE = us_lineedit(0, 0, true);
    mse_lyt->addWidget(lb_mse,    0, 0, 1, 1);
    mse_lyt->addWidget(le_meanSE, 0, 1, 1, 1);
    mse_lyt->addWidget(lb_minse,  1, 0, 1, 1);
    mse_lyt->addWidget(le_minSE,  1, 1, 1, 1);
    mse_lyt->addWidget(lb_maxse,  2, 0, 1, 1);
    mse_lyt->addWidget(le_maxSE,  2, 1, 1, 1);

    QLabel* lb_plot_ctrl = us_banner("3D Plot Control");
    QLabel* lb_selmode = us_label("Selection Mode");
    lb_selmode->setAlignment(Qt::AlignCenter);

    rb_nosel = new QRadioButton();
    QGridLayout *nosel_gl = us_radiobutton("No Selection", rb_nosel, true);

    rb_point = new QRadioButton();
    QGridLayout *point_gl = us_radiobutton("Point", rb_point, false);

    rb_radial = new QRadioButton();
    QGridLayout *radial_gl = us_radiobutton("Radial Slice", rb_radial, false);

    rb_lambda = new QRadioButton();
    QGridLayout *wavlth_gl = us_radiobutton("Lambda Slice", rb_lambda, false);

    QGridLayout *select_lyt = new QGridLayout();
    select_lyt->addLayout(nosel_gl,  0, 0, 1, 1);
    select_lyt->addLayout(point_gl,  1, 0, 1, 1);
    select_lyt->addLayout(radial_gl, 2, 0, 1, 1);
    select_lyt->addLayout(wavlth_gl, 3, 0, 1, 1);

    QButtonGroup *select_btng = new QButtonGroup();
    select_btng->addButton(rb_nosel);
    select_btng->addButton(rb_point);
    select_btng->addButton(rb_radial);
    select_btng->addButton(rb_lambda);

    QLabel* lb_wavl_rng = us_label("Lambda Range (nm)");
    lb_wavl_rng->setAlignment(Qt::AlignCenter);
    QLabel* lb_wavl_fr = us_label("From:");
    lb_wavl_fr->setAlignment(Qt::AlignRight);
    double dwl = (double) (lambda4ct.at(1) - lambda4ct.at(0)) / 10.0;
    double wl1 = (double) lambda4ct.at(0) / 10.0;
    double wl2 = (double) lambda4ct.at(nWavelengths - 2) / 10.0;
    ct_min_wl = us_counter(3, wl1, wl2, wl1);
    ct_min_wl->setSingleStep(dwl);

    QLabel* lb_wavl_to = us_label("To:");
    lb_wavl_to->setAlignment(Qt::AlignRight);
    wl1 = (double) lambda4ct.at(1) / 10.0;
    wl2 = (double) lambda4ct.at(nWavelengths - 1) / 10.0;
    ct_max_wl = us_counter(3, wl1, wl2, wl2);
    ct_max_wl->setSingleStep(dwl);

    QGridLayout* wavl_lyt = new QGridLayout();
    wavl_lyt->addWidget(lb_wavl_fr, 0, 0, 1, 1);
    wavl_lyt->addWidget(ct_min_wl,  0, 1, 1, 1);
    wavl_lyt->addWidget(lb_wavl_to, 1, 0, 1, 1);
    wavl_lyt->addWidget(ct_max_wl,  1, 1, 1, 1);

    QLabel* lb_radl_rng = us_label("Radial Range (cm)");
    lb_radl_rng->setAlignment(Qt::AlignCenter);
    QLabel* lb_radl_fr = us_label("From:");
    lb_radl_fr->setAlignment(Qt::AlignRight);
    double drp = (double) (xvals4ct.at(1) - xvals4ct.at(0)) / 1000.0;
    double rp1 = (double) xvals4ct.at(0) / 1000.0;
    double rp2 = (double) xvals4ct.at(nPoints - 2) / 1000.0;
    ct_min_rp = us_counter(3, rp1, rp2, rp1);
    ct_min_rp->setSingleStep(drp);

    QLabel* lb_radl_to = us_label("To:");
    lb_radl_to->setAlignment(Qt::AlignRight);
    rp1 = (double) xvals4ct.at(1) / 1000.0;
    rp2 = (double) xvals4ct.at(nPoints - 1) / 1000.0;
    ct_max_rp = us_counter(3, rp1, rp2, rp2);
    ct_max_rp->setSingleStep(drp);

    QGridLayout* radl_lyt = new QGridLayout();
    radl_lyt->addWidget(lb_radl_fr, 0, 0, 1, 1);
    radl_lyt->addWidget(ct_min_rp,  0, 1, 1, 1);
    radl_lyt->addWidget(lb_radl_to, 1, 0, 1, 1);
    radl_lyt->addWidget(ct_max_rp,  1, 1, 1, 1);

    QLabel* lb_theme = us_label(tr("Theme %1 Camera").arg(QChar(38)));
    lb_theme->setAlignment(Qt::AlignCenter);

//    cb_theme = new QComboBox();
    cb_theme = us_comboBox();
    cb_theme->addItem(QStringLiteral("Qt"));
    cb_theme->addItem(QStringLiteral("Digia"));
    cb_theme->addItem(QStringLiteral("Stone Moss"));
    cb_theme->addItem(QStringLiteral("Army Blue"));
    cb_theme->addItem(QStringLiteral("Retro"));
    cb_theme->addItem(QStringLiteral("Isabelle"));

    pb_camera = us_pushbutton("Reset Camera");

    QHBoxLayout *theme_lyt = new QHBoxLayout();
    theme_lyt->addWidget(cb_theme);
    theme_lyt->addWidget(pb_camera);

    QLabel* lb_color = us_label("Color Map");
    lb_color->setAlignment(Qt::AlignCenter);

    pb_DFLT = us_pushbutton("Default");

    QLinearGradient lg_B2Y(0, 0, 75, 1);
    lg_B2Y.setColorAt(0.0, Qt::black);
    lg_B2Y.setColorAt(0.33, Qt::blue);
    lg_B2Y.setColorAt(0.67, Qt::red);
    lg_B2Y.setColorAt(1.0, Qt::yellow);
    QPixmap pm(75, 15);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(lg_B2Y));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 75, 15);
    pb_B2Y = new QPushButton();
    pb_B2Y->setIcon(QIcon(pm));
    pb_B2Y->setIconSize(QSize(75, 15));

    QLinearGradient lg_G2R(0, 0, 75, 1);
    lg_G2R.setColorAt(0.0, Qt::darkGreen);
    lg_G2R.setColorAt(0.5, Qt::yellow);
    lg_G2R.setColorAt(0.8, Qt::red);
    lg_G2R.setColorAt(1.0, Qt::darkRed);
    pmp.setBrush(QBrush(lg_G2R));
    pmp.drawRect(0, 0, 75, 15);
    pb_G2R = new QPushButton();
    pb_G2R->setIcon(QIcon(pm));
    pb_G2R->setIconSize(QSize(75, 15));

    QHBoxLayout *color_lyt = new QHBoxLayout;
    color_lyt->addWidget(pb_B2Y);
    color_lyt->addWidget(pb_G2R);
    color_lyt->addWidget(pb_DFLT);

    QLabel* lb_draw_mode = us_label("Draw Mode:");
    rb_surface = new QRadioButton();
    QGridLayout *surface_gl = us_radiobutton("Surface", rb_surface, true);
    rb_surface_wire = new QRadioButton();
    QGridLayout *surface_wire_gl = us_radiobutton("Surface-Wire", rb_surface_wire, false);
    QHBoxLayout* draw_mode_lyt = new QHBoxLayout();
    draw_mode_lyt->addWidget(lb_draw_mode);
    draw_mode_lyt->addLayout(surface_gl);
    draw_mode_lyt->addLayout(surface_wire_gl);
    setSurface();

    QButtonGroup *draw_btng = new QButtonGroup();
    draw_btng->addButton(rb_surface);
    draw_btng->addButton(rb_surface_wire);

    QLabel* lb_angle = us_label("Rotate Axis Labels");
    lb_angle->setAlignment(Qt::AlignCenter);

    QLabel* lb_xAngle = us_label("Radial:");
    lb_xAngle->setAlignment(Qt::AlignRight);
    sli_xAngle = new QSlider(Qt::Horizontal);
    sli_xAngle->setMinimum(0);
    sli_xAngle->setMaximum(90);
    sli_xAngle->setValue(xAngle);
    QPushButton* pb_xAngle = us_pushbutton("reset", true, -1);

    QLabel* lb_yAngle = us_label("Deviation:");
    lb_yAngle->setAlignment(Qt::AlignRight);
    sli_yAngle = new QSlider(Qt::Horizontal);
    sli_yAngle->setMinimum(0);
    sli_yAngle->setMaximum(90);
    sli_yAngle->setValue(yAngle);
    QPushButton* pb_yAngle = us_pushbutton("reset", true, -1);

    QLabel* lb_zAngle = us_label("Lambda:");
    lb_zAngle->setAlignment(Qt::AlignRight);
    sli_zAngle = new QSlider(Qt::Horizontal);
    sli_zAngle->setMinimum(0);
    sli_zAngle->setMaximum(90);
    sli_zAngle->setValue(zAngle);
    QPushButton* pb_zAngle = us_pushbutton("reset", true, -1);

    QGridLayout* angle_lyt = new QGridLayout();
    angle_lyt->addWidget(lb_xAngle,  0, 0, 1, 1);
    angle_lyt->addWidget(sli_xAngle, 0, 1, 1, 1);
    angle_lyt->addWidget(pb_xAngle,  0, 2, 1, 1);
    angle_lyt->addWidget(lb_yAngle,  1, 0, 1, 1);
    angle_lyt->addWidget(sli_yAngle, 1, 1, 1, 1);
    angle_lyt->addWidget(pb_yAngle,  1, 2, 1, 1);
    angle_lyt->addWidget(lb_zAngle,  2, 0, 1, 1);
    angle_lyt->addWidget(sli_zAngle, 2, 1, 1, 1);
    angle_lyt->addWidget(pb_zAngle,  2, 2, 1, 1);

    QLabel *lb_save = us_banner("Render Image");
    lb_save->setAlignment(Qt::AlignCenter);
    QLabel *lb_quality = us_label("Quality:");
    lb_quality->setAlignment(Qt::AlignRight);
    ct_quality = us_counter(2, 0, 100, 80);
    ct_quality->setSingleStep(1);
    QLabel *lb_scale = us_label("Magnify:");
    lb_scale->setAlignment(Qt::AlignRight);
    ct_scale = us_counter(1, 1, 10, 5);
    ct_scale->setSingleStep(1);
    ckb_rendall = new QCheckBox();
    QGridLayout *rendall_gl = us_checkbox("All Scans", ckb_rendall, false);
    pb_render = us_pushbutton("Save");

    QHBoxLayout* qual_lyt = new QHBoxLayout();
    qual_lyt->addWidget(lb_quality);
    qual_lyt->addWidget(ct_quality);
    qual_lyt->addWidget(lb_scale);
    qual_lyt->addWidget(ct_scale);
    QHBoxLayout* rend_lyt = new QHBoxLayout();
    rend_lyt->addLayout(rendall_gl);
    rend_lyt->addWidget(pb_render);
    QVBoxLayout* save_lyt = new QVBoxLayout();
    save_lyt->addLayout(qual_lyt);
    save_lyt->addLayout(rend_lyt);

    pb_help = us_pushbutton("Help");
    pb_close = us_pushbutton("Close");
    QHBoxLayout* close_lyt = new QHBoxLayout();
    close_lyt->addWidget(pb_help);
    close_lyt->addWidget(pb_close);

    US_Plot* us_dataPlot = new US_Plot(dataPlot, tr(""),
                                      tr( "Wavelength (nm)" ), tr( "Absorbance" ));
//    tab0_plotLD->setMinimumSize( 600, 400 );
    dataPlot->setMaximumHeight(250);
    dataPlot->enableAxis( QwtPlot::xBottom, true );
    dataPlot->enableAxis( QwtPlot::yLeft, true );
    dataPlot->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(dataPlot);
    QwtLegend *legend = new QwtLegend();
    dataPlot->insertLegend( legend , QwtPlot::TopLegend);

    US_Plot* us_devPlot = new US_Plot(errorPlot, tr(""),
                                      tr( "Wavelength (nm)" ), tr( "Deviation" ));
//    tab0_plotRD->setMinimumSize( 600, 400 );
    errorPlot->setMaximumHeight(250);
    errorPlot->enableAxis( QwtPlot::xBottom, true );
    errorPlot->enableAxis( QwtPlot::yLeft, true );
    errorPlot->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(errorPlot);

    QLabel* lb_radial = us_label("Radial Position (cm):");
    lb_radial->setAlignment(Qt::AlignRight);
    le_rpval = us_lineedit("", 0, true);
    le_rpval->setMaximumWidth(50);
    lb_radial->setAlignment(Qt::AlignLeft);
    sli_radial = new QSlider(Qt::Horizontal);
    sli_radial->setMinimum(0);
    sli_radial->setMaximum(nPoints - 1);
    sli_radial->setValue(nPoints / 2);
    sli_radial->setStyleSheet("background-color: white");
    sli_radial->setMinimumWidth(500);
    sli_radial->setTickPosition(QSlider::TicksBelow);
    le_rpid = us_lineedit("", 0, false);
    le_rpid->setMaximumWidth(50);

    QHBoxLayout* radial_lyt = new QHBoxLayout();
    radial_lyt->setSpacing(2);
    radial_lyt->addStretch(1);
    radial_lyt->addWidget(lb_radial);
    radial_lyt->addWidget(le_rpval);
    radial_lyt->addWidget(sli_radial);
    radial_lyt->addWidget(le_rpid);
    radial_lyt->addStretch(1);

    QWidget* tab1 = new QWidget();
    QVBoxLayout* tab1_lyt = new QVBoxLayout(tab1);
    tab1_lyt->setMargin(0);
    tab1_lyt->setSpacing(1);
    tab1_lyt->addLayout(us_dataPlot);
    tab1_lyt->addLayout(us_devPlot);
    tab1_lyt->addSpacing(10);
    tab1_lyt->addLayout(radial_lyt);
    tab1_lyt->addStretch(1);

    tabs = new QTabWidget();
//    tabs->setAutoFillBackground(true);
    tabs->addTab(surfaceWgt, tr("3D Plot"));
    tabs->addTab(tab1, tr("Inspect Deviation"));
    tabs->tabBar()->setMinimumWidth(300);
    QStringList styleSheet;
    styleSheet << "QTabWidget::pane {border-top: 2px solid #C2C7CB;}";
    styleSheet << "QTabBar::tab:selected {font-size: 10pt; font-weight: bold; color: black;}";
    styleSheet << "QTabBar::tab:!selected {font-size: 9pt; font-weight: normal; color: black;}";
    tabs->setStyleSheet(styleSheet.join(" "));

    QVBoxLayout* left_lyt = new QVBoxLayout();
    int space = 1;
    left_lyt->addWidget(lb_scan_ctrl);
    left_lyt->addLayout(scan_lyt1);
    left_lyt->addLayout(scan_lyt2);
    left_lyt->addLayout(mse_lyt);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_plot_ctrl);
    left_lyt->addWidget(lb_selmode);
    left_lyt->addLayout(select_lyt);
    left_lyt->addWidget(lb_wavl_rng);
    left_lyt->addLayout(wavl_lyt);
    left_lyt->addWidget(lb_radl_rng);
    left_lyt->addLayout(radl_lyt);
    left_lyt->addWidget(lb_theme);
    left_lyt->addLayout(theme_lyt);
    left_lyt->addWidget(lb_color);
    left_lyt->addLayout(color_lyt);
    left_lyt->addLayout(draw_mode_lyt);
    left_lyt->addWidget(lb_angle);
    left_lyt->addLayout(angle_lyt);
    left_lyt->addWidget(lb_save);
    left_lyt->addLayout(save_lyt);
    left_lyt->addStretch(1);
    left_lyt->addSpacing(1);
    left_lyt->addLayout(close_lyt);
    left_lyt->setSpacing(1);
    left_lyt->setMargin(1);

    QHBoxLayout* main_lyt = new QHBoxLayout();
    main_lyt->addLayout(left_lyt);
    main_lyt->addWidget(tabs);
    main_lyt->setSpacing(2);
    main_lyt->setMargin(1);
    main_lyt->setSizeConstraint(QLayout::SetFixedSize);

    this->setLayout(main_lyt);

    if (!graph->hasContext()) {
        QMessageBox::warning(this, "Error!", "Couldn't initialize tnew_rpidhe OpenGL context.");
        this->close();
    }

    connect(cb_scan, SIGNAL(currentIndexChanged(int)), this, SLOT(newScan(int)));
    connect(pb_next, SIGNAL(clicked()), this, SLOT(nextScan()));
    connect(pb_prev, SIGNAL(clicked()), this, SLOT(prevScan()));
    connect(pb_camera, SIGNAL(clicked()), this, SLOT(resetCamera()));
    connect(cb_theme, SIGNAL(currentTextChanged(QString)), this, SLOT(setTheme(QString)));
    connect(rb_surface, SIGNAL(clicked()), this, SLOT(setSurface()));
    connect(rb_surface_wire, SIGNAL(clicked()), this, SLOT(setSurfaceWire()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_render, SIGNAL(clicked()), this, SLOT(renderImage()));
    connect(pb_B2Y, SIGNAL(clicked()), this, SLOT(set_B2Y()));
    connect(pb_G2R, SIGNAL(clicked()), this, SLOT(set_G2R()));
    connect(pb_DFLT, SIGNAL(clicked()), this, SLOT(set_DFLT()));
    connect(ct_min_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMin(double)));
    connect(ct_max_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMax(double)));
    connect(ct_min_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMin(double)));
    connect(ct_max_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMax(double)));
    connect(rb_nosel, SIGNAL(toggled(bool)), this, SLOT(toggleNone(bool)));
    connect(rb_point, SIGNAL(toggled(bool)), this, SLOT(togglePoint(bool)));
    connect(rb_radial, SIGNAL(toggled(bool)), this, SLOT(toggleRadial(bool)));
    connect(rb_lambda, SIGNAL(toggled(bool)), this, SLOT(toggleLambda(bool)));
    connect(sli_xAngle, SIGNAL(valueChanged(int)), this, SLOT(new_xAngle(int)));
    connect(sli_yAngle, SIGNAL(valueChanged(int)), this, SLOT(new_yAngle(int)));
    connect(sli_zAngle, SIGNAL(valueChanged(int)), this, SLOT(new_zAngle(int)));
    connect(pb_xAngle, SIGNAL(clicked()), this, SLOT(reset_xAngle()));
    connect(pb_yAngle, SIGNAL(clicked()), this, SLOT(reset_yAngle()));
    connect(pb_zAngle, SIGNAL(clicked()), this, SLOT(reset_zAngle()));
    connect(sli_radial, SIGNAL(valueChanged(int)), this, SLOT(new_rpoint(int)));
    connect(le_rpid, SIGNAL(returnPressed()), this, SLOT(new_rpid()));
    connect(ckb_rendall, SIGNAL(stateChanged(int)), this, SLOT(render_option(int)));

    get_minMaxMean();
//    cb_scan->setCurrentIndex(scanId);
    newScan(scanId);
    pb_render->setText(tr("Save Scan %1").arg(scanId + 1));
    renderLoopState = true;
    renderRunState = false;
}

US_MWL_SF_PLOT3D::~US_MWL_SF_PLOT3D()
{
    delete graph;
}


void US_MWL_SF_PLOT3D::setTheme(const QString theme)
{
    if (theme == "Qt")
        graph->activeTheme()->setType(Q3DTheme::ThemeQt);
    else if (theme == "Digia")
        graph->activeTheme()->setType(Q3DTheme::ThemeDigia);
    else if (theme == "Stone Moss")
        graph->activeTheme()->setType(Q3DTheme::ThemeStoneMoss);
    else if (theme == "Army Blue")
        graph->activeTheme()->setType(Q3DTheme::ThemeArmyBlue);
    else if (theme == "Retro")
        graph->activeTheme()->setType(Q3DTheme::ThemeRetro);
    else if (theme == "Isabelle")
        graph->activeTheme()->setType(Q3DTheme::ThemeIsabelle);

    if (colorId == B2Y)
        set_B2Y();
    else if(colorId == G2R)
        set_G2R();
}

void US_MWL_SF_PLOT3D::renderImage(){
    if (renderRunState){
        renderLoopState = false;
        return;
    }
    if (ckb_rendall->isChecked()){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Choose a Directory"),
                                                        US_Settings::resultDir(),
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::DontResolveSymlinks);
        if (dir.isEmpty())
            return;
        QDialog *bnameDialog = new QDialog();
        bnameDialog->setWindowTitle("Define the Image Files' Basenames");
        QLabel *lb_outDir = us_label(tr("Output Directory:"));
        lb_outDir->setAlignment(Qt::AlignRight);
        QLineEdit *le_outDir = us_lineedit(dir, 0, true);
        QLabel *lb_bname = us_label(tr("Basename:"));
        lb_bname->setAlignment(Qt::AlignRight);
        QLineEdit *le_bname = us_lineedit("", 0, false);
        QLabel *lb_format = us_label(tr("Format:"));
        lb_format->setAlignment(Qt::AlignRight);
        QComboBox *cb_format = us_comboBox();
        QStringList formats = { "png", "jpg", "jpeg", "bmp", "ppm", "xbm", "xpm"};
        cb_format->addItems(formats);
        QPushButton *pb_apply = us_pushbutton("Apply");
        QPushButton *pb_close = us_pushbutton("Close");
        QGridLayout *lyt = new QGridLayout(bnameDialog);
        lyt->addWidget(lb_outDir, 0, 0, 1, 1);
        lyt->addWidget(le_outDir, 0, 1, 1, 3);
        lyt->addWidget(lb_bname,  1, 0, 1, 1);
        lyt->addWidget(le_bname,  1, 1, 1, 1);
        lyt->addWidget(lb_format, 1, 2, 1, 1);
        lyt->addWidget(cb_format, 1, 3, 1, 1);
        lyt->addWidget(pb_close,  2, 2, 1, 1);
        lyt->addWidget(pb_apply,  2, 3, 1, 1);
        bnameDialog->setLayout(lyt);
        connect(pb_apply, SIGNAL(clicked()), bnameDialog, SLOT(accept()));
        connect(pb_close, SIGNAL(clicked()), bnameDialog, SLOT(reject()));
        int state = bnameDialog->exec();
        QString format = cb_format->currentText();
        QString bname = le_bname->text();
        QRegExp rx( "[A-Za-z0-9_-]" );
        QString baseName;
        int pos = 0;
        int idx = rx.indexIn(bname, pos);
        while (idx != -1) {
            baseName = baseName.append(bname.at(idx));
            pos = idx + 1;
            idx = rx.indexIn(bname, pos);
        }
        if (bname != baseName)
            QMessageBox::warning(this, tr("Warning!"),
                                 tr("The new basename is\n%1").arg(baseName));
        if (state == QDialog::Rejected){
            return;
        } else if (state == QDialog::Accepted){
            if (le_bname->text().isEmpty()){
                QMessageBox::warning(this, tr("Error!"),
                                     tr("Empty Basename Error!"));
                return;
            }
            enable_wgt(false);
            QString style = tr("background-color: %1;");
            QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
            pb_render->setText("Stop!");
            pb_render->setStyleSheet(style.arg("red"));
            renderRunState = true;
            QCoreApplication::processEvents();
            QString fn("%1-%2.%3");
            for (int i = 0; i < nScans; i++){
                cb_scan->setCurrentIndex(i);
                QString fnum = QString::number(i + 1).rightJustified(3, '0');
                QString fname = fn.arg(baseName, fnum, format);
                QFileInfo finfo = QFileInfo(dir, fname);
                qDebug() << finfo.absoluteFilePath();
                QImage image = graph->renderToImage(0, graph->size() * ct_scale->value());
                int quality = qRound(ct_quality->value());
                image.save(finfo.absoluteFilePath(), nullptr, quality);
                QCoreApplication::processEvents();
                if (! renderLoopState)
                    break;
            }
            renderRunState = false;
            renderLoopState = true;
            enable_wgt(true);
            pb_render->setStyleSheet(style.arg(color.name()));
            render_option(0);
            scanId = nScans / 2;
            cb_scan->setCurrentIndex(scanId);
            QCoreApplication::processEvents();
        }
    } else {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image (*.png *.xpm *.jpg)"),
                                   US_Settings::resultDir(),
                                   tr("Images (*.bmp *.jpg *.jpeg *.png *.ppm *.xbm *.xpm)"));
        if (fileName.size() == 0)
            return;
        QImage image = graph->renderToImage(0, graph->size() * ct_scale->value());
        int quality = qRound(ct_quality->value());
        bool state = image.save(fileName, nullptr, quality);
        if(! state)
            QMessageBox::warning(this, tr("Error!"),
                                 tr("File storage error!\n"
                                    "Image file format not supported.\n"
                                    "Currently supported file formats:\n"
                                    "(bmp, jpg, jpeg, png, ppm, xbm, xpm)"));
    }

}

void US_MWL_SF_PLOT3D::plot3d(){
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    int np = idRP_h - idRP_l + 1;
    int nw = idWL_h - idWL_l + 1;
    dataArray->reserve(nw);

    for (int j = idWL_l ; j <= idWL_h ; j++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(np);
        int index = 0;
        double wl = lambdaScaled.at(j);
        for (int i = idRP_l; i <= idRP_h; i++) {
            double rp = xvalsScaled.at(i);
            double sse = allData.at(scanId).at(i).at(j).last();
            (*newRow)[index++].setPosition(QVector3D((float)rp, (float)sse, (float)wl));
        }
        *dataArray << newRow;
    }
    dataProxy->resetArray(dataArray);

    double min_wl = lambdaScaled.at(idWL_l);
    double max_wl = lambdaScaled.at(idWL_h);
    double min_rp = xvalsScaled.at(idRP_l);
    double max_rp = xvalsScaled.at(idRP_h);

    graph->axisX()->setRange(min_rp -padding, max_rp + padding);
    graph->axisY()->setRange(minSSE -padding, maxSSE + padding);
    graph->axisZ()->setRange(min_wl -padding, max_wl + padding);

    min_wl = ct_min_wl->value();
    max_wl = ct_max_wl->value();
    min_rp = ct_min_rp->value();
    max_rp = ct_max_rp->value();

    CustomFormatter *formatX = new CustomFormatter(min_rp, max_rp);
    graph->axisX()->setFormatter(formatX);
    CustomFormatter *formatY = new CustomFormatter(minSE, maxSE);
    graph->axisY()->setFormatter(formatY);
    CustomFormatter *formatZ = new CustomFormatter(min_wl, max_wl);
    graph->axisZ()->setFormatter(formatZ);

    int theme = cb_theme->currentIndex();
    graph->activeTheme()->setType(Q3DTheme::Theme(theme));
    if (colorId == B2Y)
        set_B2Y();
    else if(colorId == G2R)
        set_G2R();
}

void US_MWL_SF_PLOT3D::newScan(int id){
    scanId = id;
    plot2d();
    plot3d();
    pb_prev->setDisabled(scanId <= 0);
    pb_next->setDisabled(scanId >= (nScans - 1));
    if (! renderRunState)
        render_option(0);
}

void US_MWL_SF_PLOT3D::nextScan(){
    cb_scan->setCurrentIndex(scanId + 1);
}

void US_MWL_SF_PLOT3D::prevScan(){
    cb_scan->setCurrentIndex(scanId - 1);
}

void US_MWL_SF_PLOT3D::resetCamera(){
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);
}

void US_MWL_SF_PLOT3D::setSurface(){
    dataSeries->setDrawMode(QSurface3DSeries::DrawSurface);
}

void US_MWL_SF_PLOT3D::setSurfaceWire(){
    dataSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
}

void US_MWL_SF_PLOT3D::set_B2Y(){
    QLinearGradient lg;
    lg.setColorAt(0.0, Qt::black);
    lg.setColorAt(0.33, Qt::blue);
    lg.setColorAt(0.67, Qt::red);
    lg.setColorAt(1.0, Qt::yellow);
    colorId = B2Y;
    graph->seriesList().at(0)->setBaseGradient(lg);
    graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void US_MWL_SF_PLOT3D::set_G2R(){
    QLinearGradient lg;
    lg.setColorAt(0.0, Qt::darkGreen);
    lg.setColorAt(0.5, Qt::yellow);
    lg.setColorAt(0.8, Qt::red);
    lg.setColorAt(1.0, Qt::darkRed);
    colorId = G2R;
    graph->seriesList().at(0)->setBaseGradient(lg);
    graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void US_MWL_SF_PLOT3D::set_DFLT(){
    colorId = DFLT;
    graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleUniform);
}

void US_MWL_SF_PLOT3D::set_ct_rp(bool ascent){
    ct_min_rp->disconnect();
    ct_max_rp->disconnect();
    double min = ct_min_rp->value();
    int mini = qRound(min * 1000.0);
    int id_min = xvals4ct.indexOf(mini);
    double max = ct_max_rp->value();
    int maxi = qRound(max * 1000.0);
    int id_max = xvals4ct.indexOf(maxi);
    if (ascent) {
        if (id_min >= id_max){
            if (id_min < nPoints - 2)
                id_max = id_min + 1;
            else
                id_max = nPoints - 1;
        }
    } else {
        if (id_max <= id_min){
            if (id_max > 1)
                id_min = id_max - 1;
            else
                id_min = 0;
        }
    }

    idRP_l = id_min;
    min = (double) xvals4ct.at(idRP_l) / 1000.0;
    ct_min_rp->setValue(min);
    idRP_h = id_max;
    max = (double) xvals4ct.at(idRP_h) / 1000.0;
    ct_max_rp->setValue(max);
    get_minMaxMean();
    plot3d();
    connect(ct_min_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMin(double)));
    connect(ct_max_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMax(double)));
}

void US_MWL_SF_PLOT3D::adjustRpMin(double){
    set_ct_rp(true);
}

void US_MWL_SF_PLOT3D::adjustRpMax(double){
    set_ct_rp(false);
}

void US_MWL_SF_PLOT3D::set_ct_wl(bool ascent){
    ct_min_wl->disconnect();
    ct_max_wl->disconnect();
    double min = ct_min_wl->value();
    int mini = qRound(min * 10.0);
    int id_min = lambda4ct.indexOf(mini);
    double max = ct_max_wl->value();
    int maxi = qRound(max * 10.0);
    int id_max = lambda4ct.indexOf(maxi);
    if (ascent) {
        if (id_min >= id_max){
            if (id_min < nWavelengths - 2)
                id_max = id_min + 1;
            else
                id_max = nWavelengths - 1;
        }
    } else {
        if (id_max <= id_min){
            if (id_max > 1)
                id_min = id_max - 1;
            else
                id_min = 0;
        }
    }

    idWL_l = id_min;
    min = (double) lambda4ct.at(idWL_l) / 10.0;
    ct_min_wl->setValue(min);
    idWL_h = id_max;
    max = (double) lambda4ct.at(idWL_h) / 10.0;
    ct_max_wl->setValue(max);
    get_minMaxMean();
    plot3d();
    connect(ct_min_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMin(double)));
    connect(ct_max_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMax(double)));
}

void US_MWL_SF_PLOT3D::adjustWlMin(double){
    set_ct_wl(true);
}

void US_MWL_SF_PLOT3D::adjustWlMax(double){
    set_ct_wl(false);
}

void US_MWL_SF_PLOT3D::toggleNone(bool) {
    graph->setSelectionMode(QAbstract3DGraph::SelectionNone);
}
void US_MWL_SF_PLOT3D::togglePoint(bool) {
    graph->setSelectionMode(QAbstract3DGraph::SelectionItem);
}
void US_MWL_SF_PLOT3D::toggleLambda(bool) {
    graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndColumn
                            | QAbstract3DGraph::SelectionSlice);
}
void US_MWL_SF_PLOT3D::toggleRadial(bool) {
    graph->setSelectionMode(QAbstract3DGraph::SelectionItemAndRow
                            | QAbstract3DGraph::SelectionSlice);
}

void US_MWL_SF_PLOT3D::new_xAngle(int value){
    graph->axisX()->setLabelAutoRotation(value);
}

void US_MWL_SF_PLOT3D::new_yAngle(int value){
    graph->axisY()->setLabelAutoRotation(value);
}

void US_MWL_SF_PLOT3D::new_zAngle(int value){
    graph->axisZ()->setLabelAutoRotation(value);
}

void US_MWL_SF_PLOT3D::reset_xAngle(){
    sli_xAngle->setValue(xAngle);
}

void US_MWL_SF_PLOT3D::reset_yAngle(){
    sli_yAngle->setValue(yAngle);
}

void US_MWL_SF_PLOT3D::reset_zAngle(){
    sli_zAngle->setValue(zAngle);
}

void US_MWL_SF_PLOT3D::new_rpoint(int){
    plot2d();
}

void US_MWL_SF_PLOT3D::new_rpid(){
    bool state;
    int rpId = le_rpid->text().toInt(&state);
    if (! state || rpId <= 0)
        rpId = 0;
    else if (rpId > nPoints)
        rpId = nPoints - 1;
    else
        rpId--;
    le_rpid->setText(QString::number(rpId + 1));
    sli_radial->setValue(rpId);

}

void US_MWL_SF_PLOT3D::plot2d(){
    errorPlot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    dataPlot->detachItems(QwtPlotItem::Rtti_PlotItem, false);

    QVector<double> dev(nWavelengths, 0);
    QVector<double> lambdas(nWavelengths, 0);
    double *yp = dev.data();
    double *xp = lambdas.data();
    double minDev =  1e20;
    double maxDev = -1e20;
    int rpId = sli_radial->value();
    le_rpid->setText(QString::number(rpId + 1));
    double xvalue = (double) xvals4ct.at(rpId) / 1000.0;
    le_rpval->setText(QString::number(xvalue, 'f', 3));
    for (int i = 0; i < nWavelengths; ++i){
        double raw = allData.at(scanId).at(rpId).at(i).at(0);
        double fit = allData.at(scanId).at(rpId).at(i).at(1);
        double error = fit - raw;
        minDev = qMin(minDev, error);
        maxDev = qMax(maxDev, error);
        xp[i] = (double) lambda4ct.at(i) / 10.0;
        yp[i] = error;
    }
    double minWl = xp[0] - 0.1;
    double maxWl = xp[nWavelengths - 1] + 0.1;
    QwtPlotCurve* curve = us_curve( errorPlot,"");
    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(2);
    curve->setPen( pen_plot );
    curve->setSamples(xp, yp, nWavelengths);
    double dy = (maxDev - minDev) * 0.01;
    errorPlot->setAxisScale( QwtPlot::xBottom, minWl, maxWl);
    errorPlot->setAxisScale( QwtPlot::yLeft  , minDev - dy, maxDev + dy);
    errorPlot->updateAxes();
    grid = us_grid(errorPlot);
    errorPlot->replot();

    QVector<double> od_raw(nWavelengths, 0);
    QVector<double> od_fit(nWavelengths, 0);
    double *rp = od_raw.data();
    double *fp = od_fit.data();
    double minVal =  1e20;
    double maxVal = -1e20;
    for (int i = 0; i < nWavelengths; i++){
        double raw = allData.at(scanId).at(rpId).at(i).at(0);
        rp[i] = raw;
        double fit = allData.at(scanId).at(rpId).at(i).at(1);
        fp[i] = fit;
        double minV = qMin(raw, fit);
        double maxV = qMax(raw, fit);
        minVal = qMin(minVal, minV);
        maxVal = qMax(maxVal, maxV);
    }
    QwtPlotCurve* curve_org = us_curve( dataPlot,"Raw Data");
    pen_plot.setColor(Qt::green);
    curve_org->setPen( pen_plot );
    curve_org->setSamples(xp, rp, nWavelengths);

    QPen nopen = QPen(Qt::red, 0, Qt::NoPen);
    QwtSymbol *symbol = new QwtSymbol(
                QwtSymbol::Ellipse, QBrush(Qt::red),
                QPen(Qt::red, 3), QSize( 5, 5 ));
    QwtPlotCurve* curve_fit = us_curve( dataPlot,"Fitted Data");
    curve_fit->setSamples(xp, fp, nWavelengths);
    curve_fit->setPen(nopen);
    curve_fit->setSymbol(symbol);

    dy = (maxVal - minVal) * 0.01;
    dataPlot->setAxisScale( QwtPlot::xBottom, minWl, maxWl);
    dataPlot->setAxisScale( QwtPlot::yLeft  , minVal - dy, maxVal + dy);
    dataPlot->updateAxes();
    grid = us_grid(dataPlot);
    dataPlot->replot();
}

void US_MWL_SF_PLOT3D::get_minMaxMean(void){
    meanSE = 0;
    minSE  =  1e99;
    minSSE =  1e99;
    maxSE  = -1e99;
    maxSSE = -1e99;
    int n = 0;
    for (int i = 0; i < nScans; i++){
        for (int j = idRP_l; j <= idRP_h ; j++){
            for (int k = idWL_l; k <= idWL_h; k++) {
                double d1 = allData.at(i).at(j).at(k).at(0);
                double d2 = allData.at(i).at(j).at(k).at(1);
                double se = qPow(d1 - d2, 2);
                double sse = allData.at(i).at(j).at(k).at(2);
                meanSE += se;
                maxSE = qMax(maxSE, se);
                minSE = qMin(minSE, se);
                maxSSE = qMax(maxSSE, sse);
                minSSE = qMin(minSSE, sse);
                n++;
            }
        }
    }
    meanSE /= n;
    le_meanSE->setText(QString::number(meanSE));
    le_minSE->setText(QString::number(minSE));
    le_maxSE->setText(QString::number(maxSE));
}


void US_MWL_SF_PLOT3D::render_option(int){
    int state = ckb_rendall->checkState();
    if (state == Qt::Checked){
        pb_render->setText("Render All Scans");
    } else {
        pb_render->setText(tr("Render Scan %1").arg(scanId + 1));
    }
}

void US_MWL_SF_PLOT3D::enable_wgt(bool state){
//    surfaceWgt->setEnabled(state);
    tabs->setEnabled(state);
    rb_surface->setEnabled(state);
    rb_surface_wire->setEnabled(state);
    rb_nosel->setEnabled(state);
    rb_point->setEnabled(state);
    rb_radial->setEnabled(state);
    rb_lambda->setEnabled(state);
    pb_next->setEnabled(state);
    pb_prev->setEnabled(state);
    pb_G2R->setEnabled(state);
    pb_B2Y->setEnabled(state);
    pb_DFLT->setEnabled(state);
    pb_help->setEnabled(state);
    pb_camera->setEnabled(state);
    sli_xAngle->setEnabled(state);
    sli_yAngle->setEnabled(state);
    sli_zAngle->setEnabled(state);
    sli_radial->setEnabled(state);
    cb_scan->setEnabled(state);
    cb_theme->setEnabled(state);
    ct_min_rp->setEnabled(state);
    ct_max_rp->setEnabled(state);
    ct_min_wl->setEnabled(state);
    ct_max_wl->setEnabled(state);
    ct_quality->setEnabled(state);
    ct_scale->setEnabled(state);
    ckb_rendall->setEnabled(state);

}

void US_MWL_SF_PLOT3D::closeEvent(QCloseEvent *event){
    renderLoopState = false;
    event->accept();
}
/////
/////

CustomFormatter::CustomFormatter(qreal i_minval, qreal i_maxval) :
    QValue3DAxisFormatter()
{
    qRegisterMetaType<QValue3DAxisFormatter *>();
    minVal = i_minval;
    maxVal = i_maxval;
}

CustomFormatter::~CustomFormatter(){}

QValue3DAxisFormatter *CustomFormatter::createNewInstance() const {
    return new CustomFormatter();
}

void CustomFormatter::populateCopy(QValue3DAxisFormatter &copy) const {
    QValue3DAxisFormatter::populateCopy(copy);

    CustomFormatter *customFormatter = static_cast<CustomFormatter *>(&copy);
    customFormatter->minVal = minVal;
    customFormatter->maxVal = maxVal;
}

void CustomFormatter::recalculate() {
    int segmentCount = axis()->segmentCount();
    int subGridCount = axis()->subSegmentCount() - 1;
    QString labelFormat =  axis()->labelFormat();

    gridPositions().resize(segmentCount + 1);
    subGridPositions().resize(segmentCount * subGridCount);

    labelPositions().resize(segmentCount + 1);
    labelStrings().clear();
    labelStrings().reserve(segmentCount + 1);

    qreal segmentStep = 1.0 / qreal(segmentCount);
    qreal subSegmentStep = 0;
    if (subGridCount > 0)
        subSegmentStep = segmentStep / qreal(subGridCount + 1);

    qreal labelValue;
    QVector<qreal> values(segmentCount + 1);

//    qreal delta = (maxVal - minVal) / segmentCount;
    qreal delta = 1 / segmentCount;
    for (int i = 0; i < segmentCount; i++) {
//        values[i] = minVal + i * delta;
        values[i] = i * delta;
    }
    values[segmentCount] = maxVal;
    values[segmentCount] = 1.0;

    for (int i = 0; i < segmentCount; i++) {
        qreal gridValue = segmentStep * qreal(i);
        gridPositions()[i] = float(gridValue);
        labelPositions()[i] = float(gridValue);
        labelValue = values.at(i);
        labelStrings() << stringForValue(labelValue, labelFormat);
        if (subGridPositions().size()) {
            for (int j = 0; j < subGridCount; j++)
                subGridPositions()[i * subGridCount + j] = gridValue + subSegmentStep * (j + 1);
        }
    }
    gridPositions()[segmentCount] = 1.0f;
    labelPositions()[segmentCount] = 1.0f;
    labelValue = values.at(segmentCount);
    labelStrings() << stringForValue(labelValue, labelFormat);
}

QString CustomFormatter::stringForValue(qreal value, const QString &format) const {
//    Q_UNUSED(format)
    qreal newValue = (maxVal - minVal) * value + minVal;
    return QValue3DAxisFormatter::stringForValue(newValue, format);
}
