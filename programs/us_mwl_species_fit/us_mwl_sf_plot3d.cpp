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
    nScans = rmsdIn.includedScans.size();
    scanId = nScans / 2;
    nWavelengths = rmsdIn.wavelenghts.size();
    nPoints = rmsdIn.xValues.size();
    double coeffRmsd = 1.0;
    double coeffRP = 2.0;
    double coeffWL = 1.0;
    idWL_l = 0;
    idWL_h = nWavelengths - 1;
    idRP_l = 0;
    idRP_h = nPoints - 1;

    allRmsdScaled.clear();
    allRmsd.clear();
    double offset;
    double scale;
    for (int i = 0; i < nScans; i++){
        double minValue = 1e20;
        double maxValue = -1e20;
        for (int j = 0; j < nPoints; j++){
            for (int k = 0; k < nWavelengths; k++){
                double rmsd = rmsdIn.allDeviations.at(i).at(j).at(k);
                minValue = qMin(minValue, rmsd);
                maxValue = qMax(maxValue, rmsd);
            }
        }
        offset = minValue;
        scale = (maxValue - minValue);
        QVector< QVector < double > > dev_rpwl;
        for (int j = 0; j < nPoints; j++){
            QVector < double > dev_wl;
            for (int k = 0; k < nWavelengths; k++){
                double rmsd = rmsdIn.allDeviations.at(i).at(j).at(k);
                rmsd = (rmsd - offset) / scale;
                rmsd *= coeffRmsd;
                dev_wl << rmsd;
            }
            dev_rpwl << dev_wl;
        }
        allRmsdScaled << dev_rpwl;
        allRmsd << rmsdIn.allDeviations.at(i);
    }

    offset = rmsdIn.xValues.at(0);
    scale = rmsdIn.xValues.at(nPoints - 1) - rmsdIn.xValues.at(0);
    xvalsScaled.clear();
    xvals4ct.clear();
    for (int i = 0; i < nPoints; i++){
        double rp = rmsdIn.xValues.at(i);
        xvals4ct << (int) (rp * 1000);
        rp = (rp - offset) / scale;
        rp *= coeffRP;
        xvalsScaled << rp;
    }

    offset = rmsdIn.wavelenghts.at(0);
    scale = rmsdIn.wavelenghts.at(nWavelengths - 1) - rmsdIn.wavelenghts.at(0);
    lambdaScaled.clear();
    lambda4ct.clear();
    for (int i = 0; i < nWavelengths; i++){
        double wl = rmsdIn.wavelenghts.at(i);
        lambda4ct << (int) (wl * 10);
        wl = (wl - offset) / scale;
        wl *= coeffWL;
        lambdaScaled << wl;
    }

    colorSet = "G2R";
    padding = 0.0001;

    dataProxy = new QSurfaceDataProxy();
    dataSeries = new QSurface3DSeries(dataProxy);
//    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    dataSeries->setDrawMode(QSurface3DSeries::DrawSurface);
    dataSeries->setFlatShadingEnabled(true);

    graph = new Q3DSurface();
    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
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
        cb_scan->addItem(QString::number(rmsdIn.includedScans.at(i)));
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
    left_lyt->addLayout(wavl_lyt);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_radl_rng);
    left_lyt->addLayout(radl_lyt);
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
    connect(pb_next, SIGNAL(clicked()), this, SLOT(nextScan()));
    connect(pb_prev, SIGNAL(clicked()), this, SLOT(prevScan()));

//    connect(cb_camera, SIGNAL(currentTextChanged(const QString&)),
//            this, SLOT(setCamera(const QString&)));
    connect(cb_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(setTheme(int)));
    connect(rb_surface, SIGNAL(clicked()), this, SLOT(setSurface()));
    connect(rb_surface_wire, SIGNAL(clicked()), this, SLOT(setSurfaceWire()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_B2Y, SIGNAL(clicked()), this, SLOT(set_B2Y()));
    connect(pb_G2R, SIGNAL(clicked()), this, SLOT(set_G2R()));

    connect(ct_min_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMin(double)));
    connect(ct_max_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMax(double)));
    connect(ct_min_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMin(double)));
    connect(ct_max_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMax(double)));

    cb_scan->setCurrentIndex(scanId);
}

US_MWL_SF_PLOT3D::~US_MWL_SF_PLOT3D()
{
    delete graph;
}


void US_MWL_SF_PLOT3D::setTheme(int theme)
{
    graph->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void US_MWL_SF_PLOT3D::plot(){
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    int np = idRP_h - idRP_l + 1;
    int nw = idWL_h - idWL_l + 1;
    dataArray->reserve(np);
    double min_dv =  1e20;
    double max_dv = -1e20;
    double min_dvns =  1e20;
    double max_dvns = -1e20;

    for (int i = idRP_l ; i <= idRP_h ; i++) {
        QSurfaceDataRow *newRow = new QSurfaceDataRow(nw);
        int index = 0;
        double rp = xvalsScaled.at(i);
        for (int j = idWL_l; j <= idWL_h; j++) {
            double wl = lambdaScaled.at(j);
            double rmsd = allRmsdScaled.at(scanId).at(i).at(j);
            double rmsd_ns = allRmsd.at(scanId).at(i).at(j);
            min_dv = qMin(min_dv, rmsd);
            max_dv = qMax(max_dv, rmsd);
            min_dvns = qMin(min_dvns, rmsd_ns);
            max_dvns = qMax(max_dvns, rmsd_ns);
            (*newRow)[index++].setPosition(QVector3D((float)rp, (float)rmsd, (float)wl));
        }
        *dataArray << newRow;
    }
    dataProxy->resetArray(dataArray);

    double min_wl = lambdaScaled.at(idWL_l);
    double max_wl = lambdaScaled.at(idWL_h);
    double min_rp = xvalsScaled.at(idRP_l);
    double max_rp = xvalsScaled.at(idRP_h);

    graph->axisX()->setRange(min_rp -padding, max_rp + padding);
    graph->axisY()->setRange(min_dv -padding, max_dv + padding);
    graph->axisZ()->setRange(min_wl -padding, max_wl + padding);
    if (colorSet == "B2Y")
        set_B2Y();
    else if(colorSet == "G2R")
        set_G2R();

    min_wl = ct_min_wl->value();
    max_wl = ct_max_wl->value();
    min_rp = ct_min_rp->value();
    max_rp = ct_max_rp->value();

    CustomFormatter *formatX = new CustomFormatter(min_rp, max_rp);
    graph->axisX()->setFormatter(formatX);
    CustomFormatter *formatY = new CustomFormatter(min_dvns, max_dvns);
    graph->axisY()->setFormatter(formatY);
    CustomFormatter *formatZ = new CustomFormatter(min_wl, max_wl);
    graph->axisZ()->setFormatter(formatZ);

}


void US_MWL_SF_PLOT3D::newScan(int id){
    scanId = id;
    plot();
    pb_prev->setDisabled(scanId <= 0);
    pb_next->setDisabled(scanId >= (nScans - 1));
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

void US_MWL_SF_PLOT3D::reset_ct_rp(bool ascent){
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
    plot();
    connect(ct_min_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMin(double)));
    connect(ct_max_rp, SIGNAL(valueChanged(double)), this, SLOT(adjustRpMax(double)));
}

void US_MWL_SF_PLOT3D::adjustRpMin(double){
    reset_ct_rp(true);
}

void US_MWL_SF_PLOT3D::adjustRpMax(double){
    reset_ct_rp(false);
}

void US_MWL_SF_PLOT3D::reset_ct_wl(bool ascent){
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
    plot();
    connect(ct_min_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMin(double)));
    connect(ct_max_wl, SIGNAL(valueChanged(double)), this, SLOT(adjustWlMax(double)));
}

void US_MWL_SF_PLOT3D::adjustWlMin(double){
    reset_ct_wl(true);
}

void US_MWL_SF_PLOT3D::adjustWlMax(double){
    reset_ct_wl(false);
}

