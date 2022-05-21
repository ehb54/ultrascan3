#include "us_mwl_sf_plot3d.h"

void SFDev::clear(){
    wavlth.clear();
    inclscns.clear();
    xvalues.clear();
    dev_snrpwl.clear();
    rmsd_scns.clear();
}

void SFDev::calc_rmsd(){
    if (dev_snrpwl.size() == 0)
        return;
    rmsd_scns.clear();
    int nscans = dev_snrpwl.size();
    rmsd_scns.fill(0, nscans);
    for (int i = 0; i < nscans; ++i){
        int npoints = dev_snrpwl.at(i).size();
        double rmsd_rp = 0;
        for (int j = 0; j < npoints; ++j){
            int nwl = dev_snrpwl.at(i).at(j).size();
            double rmsd_wl = 0;
            for (int k = 0; k < nwl; ++k){
                rmsd_wl += dev_snrpwl.at(i).at(j).at(k);
            }
            rmsd_wl /= nwl;
            rmsd_rp += rmsd_wl;
        }
        rmsd_rp /= npoints;
        rmsd_scns[i] = rmsd_rp;
    }
}



US_MWL_SF_PLOT3D::US_MWL_SF_PLOT3D(QWidget* w, const SFDev& FitDevIn): US_WidgetsDialog(w)
{
    setWindowTitle("Species Fit RMSD 3D Plot");
    setPalette( US_GuiSettings::frameColorDefault() );
    FitDev = &FitDevIn;
    nscans = FitDev->inclscns.size();
    scid = nscans - 1;
    npoints = FitDev->xvalues.size();
    nlambdas = FitDev->wavlth.size();
    gap = qRound((float) npoints / nlambdas) / 2;
    color_set = "G2R";

    surfaceProxy = new QSurfaceDataProxy();
    surfaceSeries = new QSurface3DSeries(surfaceProxy);
//    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurface);
    surfaceSeries->setFlatShadingEnabled(true);

    surface = new Q3DSurface();
    surface->setAxisX(new QValue3DAxis);
    surface->setAxisY(new QValue3DAxis);
    surface->setAxisZ(new QValue3DAxis);
    surface->addSeries(surfaceSeries);
    surface->setAspectRatio(1.5);
    surface->setHorizontalAspectRatio(1.5);
    surface->axisX()->setTitle("Radial Points (cm)");
    surface->axisX()->setTitleVisible(true);
    surface->axisX()->setLabelFormat("%.3f");
    surface->axisX()->setLabelAutoRotation(30);

    surface->axisY()->setTitle("RMSD");
    surface->axisY()->setTitleVisible(true);
    surface->axisY()->setLabelFormat("%.1e");
    surface->axisY()->setLabelAutoRotation(90);

    surface->axisZ()->setTitle("Wavelength (nm)");
    surface->axisZ()->setTitleVisible(true);
    surface->axisZ()->setLabelFormat("%d");
    surface->axisZ()->setLabelAutoRotation(30);
//    surface->axisZ()->setReversed(true);

    surface->scene()->activeCamera()->setWrapXRotation(true);
    surface->scene()->activeCamera()->setWrapYRotation(false);
    surface->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);


    QWidget *w_surface = QWidget::createWindowContainer(surface);
    QSize screenSize = surface->screen()->size();
    w_surface->setMinimumSize(QSize(screenSize.width() / 2, screenSize.height() / 1.6));
    w_surface->setMaximumSize(screenSize);
    w_surface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    w_surface->setFocusPolicy(Qt::StrongFocus);

    QLabel* lb_scan_ctrl = us_banner("Scan Control");
    QLabel* lb_scan = us_label("Scan:");
    cb_scan = us_comboBox();

    for (int i = 0; i < nscans; ++i){
        cb_scan->addItem(QString::number(FitDev->inclscns.at(i)));
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

    sl_min_point = new QSlider(Qt::Horizontal);
    sl_min_point->setMinimum(0);
    sl_min_point->setMaximum(npoints - 2);
    sl_min_point->setTickInterval(1);
    sl_min_point->setEnabled(true);

    sl_max_point = new QSlider(Qt::Horizontal);
    sl_max_point->setMinimum(1);
    sl_max_point->setMaximum(npoints - 1);
    sl_max_point->setTickInterval(1);
    sl_max_point->setEnabled(true);

    QLabel* lb_radl_rng = us_label("Radial Range");
    lb_radl_rng->setAlignment(Qt::AlignCenter);

    sl_min_wavl = new QSlider(Qt::Horizontal);
    sl_min_wavl->setMinimum(0);
    sl_min_wavl->setMaximum(nlambdas - 2);
    sl_min_wavl->setTickInterval(1);
    sl_min_wavl->setEnabled(true);

    sl_max_wavl = new QSlider(Qt::Horizontal);
    sl_max_wavl->setMinimum(1);
    sl_max_wavl->setMaximum(nlambdas - 1);
    sl_max_wavl->setTickInterval(1);
    sl_max_wavl->setEnabled(true);

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
    left_lyt->addWidget(sl_min_wavl);
    left_lyt->addWidget(sl_max_wavl);
    left_lyt->addSpacing(space);
    left_lyt->addWidget(lb_radl_rng);
    left_lyt->addWidget(sl_min_point);
    left_lyt->addWidget(sl_max_point);
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

    if (!surface->hasContext()) {
        QMessageBox::warning(this, "Error!", "Couldn't initialize the OpenGL context.");
        this->close();
    }



    connect(cb_scan, SIGNAL(currentIndexChanged(int)), this, SLOT(new_scan(int)));

    connect(cb_camera, SIGNAL(currentTextChanged(const QString&)),
            this, SLOT(set_camera(const QString&)));
    connect(cb_theme, SIGNAL(currentIndexChanged(int)), this, SLOT(set_theme(int)));
    connect(rb_surface, SIGNAL(clicked()), this, SLOT(mode_s()));
    connect(rb_surface_wire, SIGNAL(clicked()), this, SLOT(mode_sw()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_B2Y, SIGNAL(clicked()), this, SLOT(set_B2Y()));
    connect(pb_G2R, SIGNAL(clicked()), this, SLOT(set_G2R()));

    cb_scan->setCurrentIndex(scid);
}

US_MWL_SF_PLOT3D::~US_MWL_SF_PLOT3D()
{
    delete surface;
}

void US_MWL_SF_PLOT3D::set_theme(int theme)
{
    surface->activeTheme()->setType(Q3DTheme::Theme(theme));
}

void US_MWL_SF_PLOT3D::plot(){
    QSurfaceDataArray *dataArray = new QSurfaceDataArray;
    dataArray->reserve(npoints);
    qDebug() << FitDev->dev_snrpwl.size();
    qDebug() << FitDev->dev_snrpwl.at(scid).size();
    qDebug() << FitDev->dev_snrpwl.at(scid).at(0).size();
    float max_rmsd = -1e20;
    for (int i = 0 ; i < npoints ; i++) {
        if (i % gap != 0)
            continue;
        QSurfaceDataRow *newRow = new QSurfaceDataRow(nlambdas);
        int index = 0;
        float rp = FitDev->xvalues.at(i);
        for (int j = 0; j < nlambdas; j++) {
            float wl = FitDev->wavlth.at(j);
            float rmsd = FitDev->dev_snrpwl.at(scid).at(i).at(j);
            max_rmsd = qMax(rmsd, max_rmsd);
            (*newRow)[index++].setPosition(QVector3D(rp, rmsd, wl));
        }
        *dataArray << newRow;
    }
    surfaceProxy->resetArray(dataArray);

    float pad = 0.001;
    float min_rp = FitDev->xvalues.at(0) - pad;
    float max_rp = FitDev->xvalues.at(npoints - 1) + pad;
    float min_wl = FitDev->wavlth.at(0) - pad ;
    float max_wl = FitDev->wavlth.at(nlambdas - 1) + pad;
    max_rmsd += pad;
    surface->axisX()->setRange(min_rp, max_rp);
    surface->axisY()->setRange(0, max_rmsd);
    surface->axisZ()->setRange(min_wl, max_wl);
    if (color_set == "B2Y")
        set_B2Y();
    else if(color_set == "G2R")
        set_G2R();
}

//void US_MWL_SF_PLOT3D::fillSurface(){


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


void US_MWL_SF_PLOT3D::new_scan(int id){
    scid = id;
    plot();
}

void US_MWL_SF_PLOT3D::reset_camera(){
    surface->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);
}

void US_MWL_SF_PLOT3D::mode_s(){
    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurface);
}

void US_MWL_SF_PLOT3D::mode_sw(){
    surfaceSeries->setDrawMode(QSurface3DSeries::DrawSurfaceAndWireframe);
}

void US_MWL_SF_PLOT3D::set_B2Y(){
    QLinearGradient lg;
    lg.setColorAt(0.0, Qt::black);
    lg.setColorAt(0.33, Qt::blue);
    lg.setColorAt(0.67, Qt::red);
    lg.setColorAt(1.0, Qt::yellow);
    color_set = "B2Y";
    surface->seriesList().at(0)->setBaseGradient(lg);
    surface->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}

void US_MWL_SF_PLOT3D::set_G2R(){
    QLinearGradient lg;
    lg.setColorAt(0.0, Qt::darkGreen);
    lg.setColorAt(0.5, Qt::yellow);
    lg.setColorAt(0.8, Qt::red);
    lg.setColorAt(1.0, Qt::darkRed);
    color_set = "G2R";
    surface->seriesList().at(0)->setBaseGradient(lg);
    surface->seriesList().at(0)->setColorStyle(Q3DTheme::ColorStyleRangeGradient);
}


