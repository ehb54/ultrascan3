#include "us_mwl_sf_plot3d.h"

void SFDev::clear(){
    wavelenghts.clear();
    includedScans.clear();
    xValues.clear();
    allDeviations.clear();
    scansRmsd.clear();
}

void SFDev::computeRmsd(){
    if (allDeviations.size() == 0)
        return;
    scansRmsd.clear();
    int nscans = allDeviations.size();
    scansRmsd.fill(0, nscans);
    for (int i = 0; i < nscans; ++i){
        int npoints = allDeviations.at(i).size();
        double rmsd_rp = 0;
        for (int j = 0; j < npoints; ++j){
            int nwl = allDeviations.at(i).at(j).size();
            double rmsd_wl = 0;
            for (int k = 0; k < nwl; ++k){
                rmsd_wl += allDeviations.at(i).at(j).at(k);
            }
            rmsd_wl /= nwl;
            rmsd_rp += rmsd_wl;
        }
        rmsd_rp /= npoints;
        scansRmsd[i] = rmsd_rp;
    }
}


US_MWL_SF_PLOT3D::US_MWL_SF_PLOT3D(QWidget* w, const SFDev& rmsdIn): US_WidgetsDialog(w)
{
    setWindowTitle("Species Fit RMSD Plot");
    setPalette( US_GuiSettings::frameColorDefault() );
    allRMSDs = &rmsdIn;
    nScans = allRMSDs->includedScans.size();
    scanId = nScans - 1;
    nPoints = allRMSDs->xValues.size();
    nWavelengths = allRMSDs->wavelenghts.size();
    getScaleOffset();
    pointsGap = qRound((float) nPoints / nWavelengths) / 2;
    colorSet = "G2R";
    padding = 0.001;


    dataProxy = new QSurfaceDataProxy();
    dataSeries = new QSurface3DSeries(dataProxy);
//    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    dataSeries->setDrawMode(QSurface3DSeries::DrawSurface);
    dataSeries->setFlatShadingEnabled(true);

    graph = new Q3DSurface();
    graph->setAxisX(new QValue3DAxis);
    graph->setAxisY(new QValue3DAxis);
    graph->setAxisZ(new QValue3DAxis);
    graph->addSeries(dataSeries);
//    graph->setAspectRatio(1.5);
//    graph->setHorizontalAspectRatio(1.5);
    graph->axisX()->setTitle("Radial Points (cm)");
    graph->axisX()->setTitleVisible(true);
    graph->axisX()->setLabelFormat("%.3f");
    graph->axisX()->setLabelAutoRotation(30);

    graph->axisY()->setTitle("RMSD");
    graph->axisY()->setTitleVisible(true);
    graph->axisY()->setLabelFormat("%.1e");
    graph->axisY()->setLabelAutoRotation(90);

    graph->axisZ()->setTitle("Wavelength (nm)");
    graph->axisZ()->setTitleVisible(true);
    graph->axisZ()->setLabelFormat("%d");
    graph->axisZ()->setLabelAutoRotation(30);
//    surface->axisZ()->setReversed(true);

    graph->scene()->activeCamera()->setWrapXRotation(true);
    graph->scene()->activeCamera()->setWrapYRotation(false);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);


    QWidget *w_surface = QWidget::createWindowContainer(graph);
    QSize screenSize = graph->screen()->size();
    w_surface->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
    w_surface->setMaximumSize(screenSize);
    w_surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w_surface->setFocusPolicy(Qt::StrongFocus);

    QLabel* lb_scan_ctrl = us_banner("Scan Control");
    QLabel* lb_scan = us_label("Scan:");
    cb_scan = us_comboBox();

    for (int i = 0; i < nScans; ++i){
        cb_scan->addItem(QString::number(allRMSDs->includedScans.at(i)));
    }
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

    QLabel* lb_plot_ctrl = us_banner("3D Plot Control");
    QLabel* lb_selmode = us_label("Selection Mode");
    lb_selmode->setAlignment(Qt::AlignCenter);

    QRadioButton *rb_nosel = new QRadioButton();
    QGridLayout *nosel_gl = us_radiobutton("No Selection", rb_nosel, false);

    QRadioButton *rb_point = new QRadioButton();
    QGridLayout *point_gl = us_radiobutton("Point", rb_point, false);

    QRadioButton *rb_radial = new QRadioButton();
    QGridLayout *radial_gl = us_radiobutton("Radial Slice", rb_radial, false);

    QRadioButton *rb_wavlth = new QRadioButton();
    QGridLayout *wavlth_gl = us_radiobutton("Lambda Slice", rb_wavlth, false);

    QLabel* lb_wavl_rng = us_label("Lambda Range");
    lb_wavl_rng->setAlignment(Qt::AlignCenter);

    sl_min_rp = new QSlider(Qt::Horizontal);
    sl_min_rp->setMinimum(0);
    sl_min_rp->setMaximum(nPoints - 2);
    sl_min_rp->setTickInterval(1);
    sl_min_rp->setEnabled(true);
    sl_min_rp->setValue(0);

    sl_max_rp = new QSlider(Qt::Horizontal);
    sl_max_rp->setMinimum(1);
    sl_max_rp->setMaximum(nPoints - 1);
    sl_max_rp->setTickInterval(1);
    sl_max_rp->setEnabled(true);
    sl_max_rp->setValue(nPoints - 1);

    QLabel* lb_radl_rng = us_label("Radial Range");
    lb_radl_rng->setAlignment(Qt::AlignCenter);

    sl_min_wl = new QSlider(Qt::Horizontal);
    sl_min_wl->setMinimum(0);
    sl_min_wl->setMaximum(nWavelengths - 2);
    sl_min_wl->setTickInterval(1);
    sl_min_wl->setEnabled(true);
    sl_min_wl->setValue(0);

    sl_max_wl = new QSlider(Qt::Horizontal);
    sl_max_wl->setMinimum(1);
    sl_max_wl->setMaximum(nWavelengths - 1);
    sl_max_wl->setTickInterval(1);
    sl_max_wl->setEnabled(true);
    sl_max_wl->setValue(nWavelengths - 1);

    QLabel* lb_theme = us_label("Theme");
    lb_theme->setAlignment(Qt::AlignCenter);

    QComboBox *cb_theme = new QComboBox();
    cb_theme->addItem(QStringLiteral("Qt"));
    cb_theme->addItem(QStringLiteral("Primary Colors"));
    cb_theme->addItem(QStringLiteral("Digia"));
    cb_theme->addItem(QStringLiteral("Stone Moss"));
    cb_theme->addItem(QStringLiteral("Army Blue"));
    cb_theme->addItem(QStringLiteral("Retro"));
    cb_theme->addItem(QStringLiteral("Ebony"));
    cb_theme->addItem(QStringLiteral("Isabelle"));

    QLabel* lb_color = us_label("Custom Gradient");
    lb_color->setAlignment(Qt::AlignCenter);

    QLinearGradient lg_B2Y(0, 0, 1, 100);
    lg_B2Y.setColorAt(1.0, Qt::black);
    lg_B2Y.setColorAt(0.67, Qt::blue);
    lg_B2Y.setColorAt(0.33, Qt::red);
    lg_B2Y.setColorAt(0.0, Qt::yellow);
    QPixmap pm(24, 100);
    QPainter pmp(&pm);
    pmp.setBrush(QBrush(lg_B2Y));
    pmp.setPen(Qt::NoPen);
    pmp.drawRect(0, 0, 24, 100);
    pb_B2Y = new QPushButton();
    pb_B2Y->setIcon(QIcon(pm));
    pb_B2Y->setIconSize(QSize(24, 100));

    QLinearGradient lg_G2R(0, 0, 1, 100);
    lg_G2R.setColorAt(1.0, Qt::darkGreen);
    lg_G2R.setColorAt(0.5, Qt::yellow);
    lg_G2R.setColorAt(0.2, Qt::red);
    lg_G2R.setColorAt(0.0, Qt::darkRed);
    pmp.setBrush(QBrush(lg_G2R));
    pmp.drawRect(0, 0, 24, 100);
    pb_G2R = new QPushButton();
    pb_G2R->setIcon(QIcon(pm));
    pb_G2R->setIconSize(QSize(24, 100));

    QHBoxLayout *color_lyt = new QHBoxLayout;
    color_lyt->addWidget(pb_B2Y);
    color_lyt->addWidget(pb_G2R);

    QLabel* lb_camera = us_label("CameraPreset");
    QComboBox* cb_camera = us_comboBox();
    QStringList camera_list;
    camera_list << "FrontLow" << "Front" << "FrontHigh" <<
                   "LeftLow" << "Left" << "LeftHigh" <<
                   "RightLow" << "Right" << "RightHigh" <<
                   "BehindLow" << "Behind" << "BehindHigh" <<
                   "IsometricLeft" << "IsometricLeftHigh" <<
                   "IsometricRight" << "IsometricRightHigh" <<
                   "DirectlyAbove" << "DirectlyAboveCW45" <<
                   "DirectlyAboveCCW45" << "FrontBelow" <<
                   "LeftBelow" << "RightBelow" << "BehindBelow" << "DirectlyBelow";
    for (int i = 0; i < camera_list.size(); ++i){
        cb_camera->addItem(camera_list.at(i));
    }
    cb_camera->setCurrentText("IsometricRightHigh");
    QHBoxLayout* camera_lyt = new QHBoxLayout();
    camera_lyt->addWidget(lb_camera);
    camera_lyt->addWidget(cb_camera);

    QLabel* lb_draw_mode = us_label("Draw Mode:");
    QRadioButton *rb_surface = new QRadioButton();
    QGridLayout *surface_gl = us_radiobutton("Surface", rb_surface, false);
    QRadioButton *rb_surface_wire = new QRadioButton();
    QGridLayout *surface_wire_gl = us_radiobutton("Surface-Wire", rb_surface_wire, false);
    QHBoxLayout* draw_mode_lyt = new QHBoxLayout();
    draw_mode_lyt->addWidget(lb_draw_mode);
    draw_mode_lyt->addLayout(surface_gl);
    draw_mode_lyt->addLayout(surface_wire_gl);

    QPushButton* pb_help = us_pushbutton("Help");
    QPushButton* pb_close = us_pushbutton("Close");
    QHBoxLayout* close_lyt = new QHBoxLayout();
    close_lyt->addWidget(pb_help);
    close_lyt->addWidget(pb_close);

    QVBoxLayout* left_lyt = new QVBoxLayout();
    int space = 5;
    left_lyt->addWidget(lb_scan_ctrl);
    left_lyt->addLayout(scan_lyt1);
    left_lyt->addLayout(scan_lyt2);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_plot_ctrl);
    left_lyt->addWidget(lb_selmode);
    left_lyt->addLayout(nosel_gl);
    left_lyt->addLayout(point_gl);
    left_lyt->addLayout(radial_gl);
    left_lyt->addLayout(wavlth_gl);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_wavl_rng);
    left_lyt->addWidget(sl_min_wl);
    left_lyt->addWidget(sl_max_wl);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_radl_rng);
    left_lyt->addWidget(sl_min_rp);
    left_lyt->addWidget(sl_max_rp);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_theme);
    left_lyt->addWidget(cb_theme);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_color);
    left_lyt->addLayout(color_lyt);
    left_lyt->addLayout(camera_lyt);
    left_lyt->addLayout(draw_mode_lyt);
    left_lyt->addStretch(1);
    left_lyt->addLayout(close_lyt);
    left_lyt->setSpacing(2);
    left_lyt->setMargin(1);

    QHBoxLayout* main_lyt = new QHBoxLayout();
    main_lyt->addLayout(left_lyt);
    main_lyt->addWidget(w_surface);
    main_lyt->setSpacing(2);
    main_lyt->setMargin(1);
    main_lyt->setSizeConstraint(QLayout::SetFixedSize);

    this->setLayout(main_lyt);

    if (!graph->hasContext()) {
        QMessageBox::warning(this, "Error!", "Couldn't initialize the OpenGL context.");
        this->close();
    }



    connect(cb_scan, SIGNAL(currentIndexChanged(int)), this, SLOT(newScan(int)));

    connect(cb_camera, SIGNAL(currentTextChanged(const QString&)),
            this, SLOT(setCamera(const QString&)));
    connect(cb_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(setTheme(int)));
    connect(rb_surface, SIGNAL(clicked()), this, SLOT(setSurface()));
    connect(rb_surface_wire, SIGNAL(clicked()), this, SLOT(setSurfaceWire()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_B2Y, SIGNAL(clicked()), this, SLOT(set_B2Y()));
    connect(pb_G2R, SIGNAL(clicked()), this, SLOT(set_G2R()));
    connect(sl_min_rp, SIGNAL(valueChanged(int)), this, SLOT(adjustRpMin(int)));
    connect(sl_max_rp, SIGNAL(valueChanged(int)), this, SLOT(adjustRpMax(int)));
    connect(sl_min_wl, SIGNAL(valueChanged(int)), this, SLOT(adjustWlMin(int)));
    connect(sl_max_wl, SIGNAL(valueChanged(int)), this, SLOT(adjustWlMax(int)));

    cb_scan->setCurrentIndex(scanId);
}

US_MWL_SF_PLOT3D::~US_MWL_SF_PLOT3D()
{
    delete graph;
}

void US_MWL_SF_PLOT3D::getScaleOffset(){
    offsetRmsd.clear();
    scaleRmsd.clear();
    for (int i = 0; i < nScans; i++){
        double minValue = 1e20;
        double maxValue = -1e20;
        for (int j = 0; j < nPoints; j++){
            for (int k = 0; k < nWavelengths; k++){
                double rmsd = allRMSDs->allDeviations.at(i).at(j).at(k);
                minValue = qMin(minValue, rmsd);
                maxValue = qMax(maxValue, rmsd);
            }
        }
        offsetRmsd << minValue;
        scaleRmsd << (maxValue - minValue);
    }
    offsetRP = allRMSDs->xValues.at(0);
    scaleRP = allRMSDs->xValues.at(nPoints - 1) - allRMSDs->xValues.at(0);
    offsetWL = allRMSDs->wavelenghts.at(0);
    scaleWL = allRMSDs->wavelenghts.at(nWavelengths - 1) - allRMSDs->wavelenghts.at(0);
}

void US_MWL_SF_PLOT3D::setTheme(int theme)
{
    graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void US_MWL_SF_PLOT3D::fillProxy(){
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(nPoints);
    qDebug() << allRMSDs->allDeviations.size();
    qDebug() << allRMSDs->allDeviations.at(scanId).size();
    qDebug() << allRMSDs->allDeviations.at(scanId).at(0).size();
//    min_rmsd =  1e20;
//    max_rmsd = -1e20;
//    int rp_id1 = sl_min_rp->value();
//    int rp_id2 = sl_max_rp->value();
//    int wl_id1 = sl_min_wl->value();
//    int wl_id2 = sl_max_wl->value();
    for (int i = 0 ; i < nPoints ; i++) {
//        if (i % gap != 0)
//            continue;
        QSurfaceDataRow *newRow = new QSurfaceDataRow(nWavelengths);
        int index = 0;
        double rp = (allRMSDs->xValues.at(i) - offsetRP) / scaleRP;
        rp *= 2.0;
        for (int j = 0; j < nWavelengths; j++) {
            double wl = (allRMSDs->wavelenghts.at(j) - offsetWL) / scaleWL;
            double rmsd = allRMSDs->allDeviations.at(scanId).at(i).at(j);
            rmsd = (rmsd - offsetRmsd.at(scanId)) / scaleRmsd.at(scanId);
            (*newRow)[index++].setPosition(QVector3D((float)rp, (float)rmsd, (float)wl));
        }
        *dataArray << newRow;
    }
    dataProxy->resetArray(dataArray);

}

void US_MWL_SF_PLOT3D::plot(){
//    min_rmsd -= padding;
//    max_rmsd += padding;
//    min_rp = allRMSDs->xValues.at(rp_id1) - padding;
//    max_rp = allRMSDs->xValues.at(rp_id2) + padding;
//    d_rp = (float) (allRMSDs->xValues.at(1) - allRMSDs->xValues.at(0));
//    min_wl = allRMSDs->wavelenghts.at(wl_id1) - padding ;
//    max_wl = allRMSDs->wavelenghts.at(wl_id2) + padding;
//    d_wl = (float) (allRMSDs->wavelenghts.at(1) - allRMSDs->wavelenghts.at(0));
    graph->axisX()->setRange(-padding, 2 + padding);
    graph->axisY()->setRange(-padding, 1 + padding);
    graph->axisZ()->setRange(-padding, 1 + padding);
    if (colorSet == "B2Y")
        set_B2Y();
    else if(colorSet == "G2R")
        set_G2R();

    CustomFormatter *formatX = new CustomFormatter(0, offsetRP, scaleRP);
    graph->axisX()->setFormatter(formatX);
    CustomFormatter *formatY = new CustomFormatter(0, offsetRmsd.at(scanId), scaleRmsd.at(scanId));
    graph->axisY()->setFormatter(formatY);
    CustomFormatter *formatZ = new CustomFormatter(0, offsetWL, scaleWL);
    graph->axisZ()->setFormatter(formatZ);

//    float frac = (float) nPoints / (float) nWavelengths;
//    int new_np = rp_id2 - rp_id1 + 1;
//    int new_nwl = wl_id2 - wl_id1 + 1;
//    float new_frac = (float) new_np / (float) new_nwl;
//    float new_scale = (new_frac / frac * h_scale);
//    graph->setAspectRatio(1.5);
//    graph->setHorizontalAspectRatio(new_scale);


}

///void US_MWL_SF_PLOT3D::fillSurface(){


//    surface->axisX()->setLabelFormat("%.2f");
//    surface->axisZ()->setLabelFormat("%.2f");
////    surface->axisX()->setRange(sampleMin, sampleMax);
//    surface->axisY()->setRange(0.0f, 2.0f);
////    surface->axisZ()->setRange(sampleMin, sampleMax);
//    surface->axisX()->setLabelAutoRotation(30);
//    surface->axisY()->setLabelAutoRotation(90);
//    surface->axisZ()->setLabelAutoRotation(30);

////    surface->selec
////    m_graph->removeSeries(m_heightMapSeries);
////    surface->addSeries(surfaceSeries);
////    //! [3]

////    //! [8]
////    // Reset range sliders for Sqrt&Sin
////    m_rangeMinX = sampleMin;
////    m_rangeMinZ = sampleMin;
////    m_stepX = (sampleMax - sampleMin) / float(sampleCountX - 1);
////    m_stepZ = (sampleMax - sampleMin) / float(sampleCountZ - 1);
////    m_axisMinSliderX->setMaximum(sampleCountX - 2);
////    m_axisMinSliderX->setValue(0);
////    m_axisMaxSliderX->setMaximum(sampleCountX - 1);
////    m_axisMaxSliderX->setValue(sampleCountX - 1);
////    m_axisMinSliderZ->setMaximum(sampleCountZ - 2);
////    m_axisMinSliderZ->setValue(0);
////    m_axisMaxSliderZ->setMaximum(sampleCountZ - 1);
////    m_axisMaxSliderZ->setValue(sampleCountZ - 1);

//}


void US_MWL_SF_PLOT3D::newScan(int id){
    scanId = id;
    fillProxy();
    plot();
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
    colorSet = "B2Y";
    graph->seriesList().at(0)->setBaseGradient(lg);
    graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void US_MWL_SF_PLOT3D::set_G2R(){
    QLinearGradient lg;
    lg.setColorAt(0.0, Qt::darkGreen);
    lg.setColorAt(0.5, Qt::yellow);
    lg.setColorAt(0.8, Qt::red);
    lg.setColorAt(1.0, Qt::darkRed);
    colorSet = "G2R";
    graph->seriesList().at(0)->setBaseGradient(lg);
    graph->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void US_MWL_SF_PLOT3D::adjustRpMin(int min)
{
    int max = sl_max_rp->value();
    if (min >= max) {
        max = min + 1;
        sl_max_rp->setValue(max);
    }
//    plot();
}

void US_MWL_SF_PLOT3D::adjustRpMax(int max)
{
    int min = sl_min_rp->value();
    if (max <= min) {
        min = max - 1;
        sl_min_rp->setValue(min);
    }
//    plot();
}

void US_MWL_SF_PLOT3D::adjustWlMin(int min)
{
    int max = sl_max_wl->value();
    if (min >= max) {
        max = min + 1;
        sl_max_wl->setValue(max);
    }
//    plot();
}

void US_MWL_SF_PLOT3D::adjustWlMax(int max)
{
    int min = sl_min_wl->value();
    if (max <= min) {
        min = max - 1;
        sl_min_wl->setValue(min);
    }
//    plot();
}

