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
    this->setWindowFlags(Qt::Window);
    setWindowTitle("Spectral Decomposition Residual Plot");
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
    double minErr = 1e99;
    double maxErr = -1e99;
    double minAbsErr = 1e99;
    double maxAbsErr = -1e99;
    // scan < radial < lambda  < raw, fit, Err, sErr, absErr, sAbsErr > > > >
    for (int i = 0; i < nScans; i++){
        QVector< QVector < QVector < double > > > data_scan;
        for (int j = 0; j < nPoints; j++){
            QVector < QVector < double > > data_rp;
            for (int k = 0; k < nWavelengths; k++){
                QVector<double> data(6, 0);
                data[0] = spFitData.allData.at(i).at(j).at(k).first();
                data[1] = spFitData.allData.at(i).at(j).at(k).last();
                double err = data[0] - data[1];
                double absErr = qAbs(err);
                minErr = qMin(minErr, err);
                maxErr = qMax(maxErr, err);
                minAbsErr = qMin(minAbsErr, absErr);
                maxAbsErr = qMax(maxAbsErr, absErr);
                data[2] = err;
                data[4] = absErr;
                data_rp << data;
            }
            data_scan << data_rp;
        }
        allData << data_scan;
    }
    double offset = minErr;
    double scale = maxErr - minErr;
    double offset2 = minAbsErr;
    double scale2 = maxAbsErr - minAbsErr;
    for (int i = 0; i < nScans; i++){
        for (int j = 0; j < nPoints; j++){
            for (int k = 0; k < nWavelengths; k++){
                double err = allData.at(i).at(j).at(k).at(2);
                err = (err - offset) / scale;
                err *= coeffER;
                allData[i][j][k][3] = err;
                double absErr = allData.at(i).at(j).at(k).at(4);
                absErr = (absErr - offset2) / scale2;
                absErr *= coeffER;
                allData[i][j][k][5] = absErr;
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
    dataSeries->setDrawMode(QSurface3DSeries::DrawSurface);
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
    graph->axisX()->setTitle("Radius (cm)");
    graph->axisX()->setTitleVisible(true);
    graph->axisX()->setLabelFormat("%.3f");
    graph->axisX()->setLabelAutoRotation(xAngle);

    graph->axisY()->setTitle("Residual (OD)");
    graph->axisY()->setTitleVisible(true);
    graph->axisY()->setLabelFormat("%.1e");
    graph->axisY()->setLabelAutoRotation(yAngle);

    graph->axisZ()->setTitle("Wavelength (nm)");
    graph->axisZ()->setTitleVisible(true);
    graph->axisZ()->setLabelFormat("%d");
    graph->axisZ()->setLabelAutoRotation(zAngle);
//    surface->axisZ()->setReversed(true);

    camera = graph->scene()->activeCamera();
    camera->setWrapXRotation(true);
    camera->setWrapYRotation(false);
    camera->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);

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
    QLabel* lb_mse = us_label("Root-Mean-Square Deviation:");
    lb_mse->setAlignment(Qt::AlignRight);
    QLabel* lb_minse = us_label("The Largest Negative Residual:");
    lb_minse->setAlignment(Qt::AlignRight);
    QLabel* lb_maxse = us_label("The Largest Positive Residual:");
    lb_maxse->setAlignment(Qt::AlignRight);
    le_RMSE = us_lineedit(0, 0, true);
    le_minErr = us_lineedit(0, 0, true);
    le_maxErr = us_lineedit(0, 0, true);
    mse_lyt->addWidget(lb_mse,    0, 0, 1, 1);
    mse_lyt->addWidget(le_RMSE, 0, 1, 1, 1);
    mse_lyt->addWidget(lb_minse,  1, 0, 1, 1);
    mse_lyt->addWidget(le_minErr,  1, 1, 1, 1);
    mse_lyt->addWidget(lb_maxse,  2, 0, 1, 1);
    mse_lyt->addWidget(le_maxErr,  2, 1, 1, 1);

    QLabel* lb_plot_ctrl = us_banner("3D Plot Control");
    ckb_plotAbs = new QCheckBox();
    QGridLayout *plotAbs_gl = us_checkbox("Plot Magnitude of Residuals", ckb_plotAbs, false);

    QLabel* lb_selmode = us_label("Selection Mode");
    lb_selmode->setAlignment(Qt::AlignCenter);

    rb_nosel = new QRadioButton();
    QGridLayout *nosel_gl = us_radiobutton("No Selection", rb_nosel, true);

    rb_point = new QRadioButton();
    QGridLayout *point_gl = us_radiobutton("Point", rb_point, false);

    rb_radial = new QRadioButton();
    QGridLayout *radial_gl = us_radiobutton("Radius Slice", rb_radial, false);

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

    QLabel* lb_radl_rng = us_label("Radius Range (cm)");
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

//    pb_camera = us_pushbutton("Reset Camera");
    cb_camera = us_comboBox();
    QStringList camera_list = {"Custom"};
    camera_list << "IsometricRight" << "IsometricRightHigh";
    camera_list << "IsometricLeft" << "IsometricLeftHigh";
    camera_list << "FrontLow" << "Front" << "FrontHigh";
    camera_list << "BehindLow" << "Behind" << "BehindHigh";
    camera_list << "LeftLow" << "Left" << "LeftHigh";
    camera_list << "RightLow" << "Right" << "RightHigh";
    camera_list << "FrontBelow";
    camera_list << "LeftBelow" << "RightBelow" << "BehindBelow" << "DirectlyBelow";
    camera_list << "DirectlyAbove" << "DirectlyAboveCW45" << "DirectlyAboveCCW45";
    cb_camera->addItems(camera_list);
    cb_camera->setCurrentIndex(1);

    QHBoxLayout *theme_lyt = new QHBoxLayout();
    theme_lyt->addWidget(cb_theme);
    theme_lyt->addWidget(cb_camera);

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

    QLabel* lb_angle = us_label("Rotate Axis Labels");
    lb_angle->setAlignment(Qt::AlignCenter);

    QLabel* lb_xAngle = us_label("Radius:");
    lb_xAngle->setAlignment(Qt::AlignRight);
    sli_xAngle = new QSlider(Qt::Horizontal);
    sli_xAngle->setMinimum(0);
    sli_xAngle->setMaximum(90);
    sli_xAngle->setValue(xAngle);
    QPushButton* pb_xAngle = us_pushbutton("reset", true, -1);

    QLabel* lb_yAngle = us_label("Residual:");
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
//    dataPlot->setMaximumHeight(250);
    dataPlot->enableAxis( QwtPlot::xBottom, true );
    dataPlot->enableAxis( QwtPlot::yLeft, true );
    dataPlot->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(dataPlot);
    QwtLegend *legend = new QwtLegend();
    dataPlot->insertLegend( legend , QwtPlot::TopLegend);

    US_Plot* us_devPlot = new US_Plot(errorPlot, tr(""),
                                      tr( "Wavelength (nm)" ), tr( "Residual" ));
//    tab0_plotRD->setMinimumSize( 600, 400 );
//    errorPlot->setMaximumHeight(250);
    errorPlot->enableAxis( QwtPlot::xBottom, true );
    errorPlot->enableAxis( QwtPlot::yLeft, true );
    errorPlot->setCanvasBackground(QBrush(Qt::black));
    grid = us_grid(errorPlot);

    QLabel* lb_radial = us_label("Radius (cm):");
    lb_radial->setAlignment(Qt::AlignRight);
    le_rpval = us_lineedit("", 0, true);
    le_rpval->setMaximumWidth(50);
    lb_radial->setAlignment(Qt::AlignLeft);
    sli_radial = new QSlider(Qt::Horizontal);
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

    lb_runs_test = us_banner("Runs Test Statistics for the Radius");
    QLabel *lb_rt_mean = us_label("Average Value of Residuals:");
//    lb_rt_mean->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_median = us_label("Median Value of Residuals:");
//    lb_rt_median->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_np = us_label("Number of Residuals:");
//    lb_rt_np->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_nruns = us_label("No. Runs:");
//    lb_rt_nruns->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_nneg = us_label("No. Negatives:");
//    lb_rt_nneg->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_npos = us_label("No. Positives:");
//    lb_rt_npos->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_zstat = us_label("Test Statistics:");
//    lb_rt_zstat->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_pval = us_label("Test p-value:");
//    lb_rt_pval->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_zcrit = us_label("Critical Value:");
//    lb_rt_zcrit->setAlignment(Qt::AlignRight);
    QLabel *lb_rt_region = us_label("Random Region of the No. Runs:");
//    lb_rt_region->setAlignment(Qt::AlignRight);
    le_rt_mean   = us_lineedit("", 0, true);
    le_rt_median = us_lineedit("", 0, true);
    le_rt_np = us_lineedit("", 0, true);
    le_rt_nruns  = us_lineedit("", 0, true);
    le_rt_nneg   = us_lineedit("", 0, true);
    le_rt_npos   = us_lineedit("", 0, true);
    le_rt_zstat  = us_lineedit("", 0, true);
    le_rt_pval   = us_lineedit("", 0, true);
    le_rt_zcrit  = us_lineedit("", 0, true);
    le_rt_region  = us_lineedit("", 0, true);
    pb_plotPixMap = us_pushbutton("Plot PixelMap (All Scans)");

    QGridLayout *lyt_zstat = new QGridLayout();
    lyt_zstat->addWidget(lb_rt_mean,   0, 0,  1, 4);
    lyt_zstat->addWidget(le_rt_mean,   0, 4,  1, 4);
    lyt_zstat->addWidget(lb_rt_median, 0, 8,  1, 4);
    lyt_zstat->addWidget(le_rt_median, 0, 12, 1, 4);
    lyt_zstat->addWidget(lb_rt_np,     0, 16, 1, 4);
    lyt_zstat->addWidget(le_rt_np,     0, 20, 1, 4);

    lyt_zstat->addWidget(lb_rt_nruns , 1, 0,  1, 4);
    lyt_zstat->addWidget(le_rt_nruns , 1, 4,  1, 4);
    lyt_zstat->addWidget(lb_rt_nneg  , 1, 8,  1, 4);
    lyt_zstat->addWidget(le_rt_nneg  , 1, 12, 1, 4);
    lyt_zstat->addWidget(lb_rt_npos  , 1, 16, 1, 4);
    lyt_zstat->addWidget(le_rt_npos  , 1, 20, 1, 4);

    lyt_zstat->addWidget(lb_rt_zstat, 2, 0,  1, 4);
    lyt_zstat->addWidget(le_rt_zstat, 2, 4,  1, 4);
    lyt_zstat->addWidget(lb_rt_pval , 2, 8,  1, 4);
    lyt_zstat->addWidget(le_rt_pval , 2, 12, 1, 4);
    lyt_zstat->addWidget(lb_rt_zcrit, 2, 16, 1, 4);
    lyt_zstat->addWidget(le_rt_zcrit, 2, 20, 1, 4);

    lyt_zstat->addWidget(lb_rt_region,  3, 0,  1, 8);
    lyt_zstat->addWidget(le_rt_region,  3, 8,  1, 8);
    lyt_zstat->addWidget(pb_plotPixMap, 3, 16, 1, 8);

    lb_rt_rstate = us_label("Randomness State", 2);
    lb_rt_rstate->setMinimumWidth(250);
    lb_rt_rstate->setAlignment(Qt::AlignCenter);
    QHBoxLayout *lyt_rstate = new QHBoxLayout();
    lyt_rstate->addStretch(1);
    lyt_rstate->addWidget(lb_rt_rstate);
    lyt_rstate->addStretch(1);

    QWidget* tab1 = new QWidget();
    QVBoxLayout* tab1_lyt = new QVBoxLayout(tab1);
    tab1_lyt->setMargin(0);
    tab1_lyt->setSpacing(1);
    tab1_lyt->addLayout(us_dataPlot, 1);
    tab1_lyt->addLayout(us_devPlot, 1);
    tab1_lyt->addSpacing(10);
//    tab1_lyt->addStretch(1);
    tab1_lyt->addLayout(radial_lyt, 0);
    tab1_lyt->addWidget(lb_runs_test, 0);
    tab1_lyt->addLayout(lyt_zstat, 0);
    tab1_lyt->addLayout(lyt_rstate, 0);
//    tab1_lyt->addStretch(1);

    tabs = new QTabWidget();
//    tabs->setAutoFillBackground(true);
    tabs->addTab(surfaceWgt, tr("3D Plot"));
    tabs->addTab(tab1, tr("Inspect Residual"));
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
    left_lyt->addLayout(plotAbs_gl);
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
    main_lyt->addLayout(left_lyt, 0);
    main_lyt->addWidget(tabs, 1);
    main_lyt->setSpacing(2);
    main_lyt->setMargin(1);
//    main_lyt->setSizeConstraint(QLayout::SetFixedSize);

    this->setLayout(main_lyt);

    if (!graph->hasContext()) {
        QMessageBox::warning(this, "Error!", "Couldn't initialize the OpenGL context.");
        this->close();
    }

    connect(cb_scan, SIGNAL(currentIndexChanged(int)), this, SLOT(newScan(int)));
    connect(pb_next, SIGNAL(clicked()), this, SLOT(nextScan()));
    connect(pb_prev, SIGNAL(clicked()), this, SLOT(prevScan()));
    connect(cb_camera, SIGNAL(currentIndexChanged(int)), this, SLOT(resetCamera(int)));
    connect(cb_theme, SIGNAL(currentTextChanged(QString)), this, SLOT(setTheme(QString)));
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
    connect(ckb_plotAbs, SIGNAL(stateChanged(int)), this, SLOT(plot3d()));
    connect(ckb_rendall, SIGNAL(stateChanged(int)), this, SLOT(render_option(int)));
    connect(camera,   SIGNAL(xRotationChanged(float)), this, SLOT(cameraChanged(float)));
    connect(camera,   SIGNAL(yRotationChanged(float)), this, SLOT(cameraChanged(float)));
    connect(pb_plotPixMap,   SIGNAL(clicked()), this, SLOT(plotPixMap()));

    fill_table();
    get_minMaxMean();
    set_radial_slider();
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
            double data;
            if (ckb_plotAbs->isChecked())
                data = allData.at(scanId).at(i).at(j).at(5);
            else
                data = allData.at(scanId).at(i).at(j).at(3);
            (*newRow)[index++].setPosition(QVector3D((float)rp, (float)data, (float)wl));
        }
        *dataArray << newRow;
    }
    dataProxy->resetArray(dataArray);

    double min_wl = lambdaScaled.at(idWL_l);
    double max_wl = lambdaScaled.at(idWL_h);
    double min_rp = xvalsScaled.at(idRP_l);
    double max_rp = xvalsScaled.at(idRP_h);

    graph->axisX()->setRange(min_rp - padding, max_rp + padding);
    graph->axisZ()->setRange(min_wl - padding, max_wl + padding);

    min_wl = ct_min_wl->value();
    max_wl = ct_max_wl->value();
    min_rp = ct_min_rp->value();
    max_rp = ct_max_rp->value();

    CustomFormatter *formatX = new CustomFormatter(min_rp, max_rp);
    graph->axisX()->setFormatter(formatX);
    CustomFormatter *formatZ = new CustomFormatter(min_wl, max_wl);
    graph->axisZ()->setFormatter(formatZ);
    CustomFormatter *formatY;
    if (ckb_plotAbs->isChecked()){
        graph->axisY()->setTitle("|| Residual (OD) ||");
        graph->axisY()->setRange(min_sAbsErr - padding, max_sAbsErr + padding);
        formatY = new CustomFormatter(min_AbsErr, max_AbsErr);
    } else {
        graph->axisY()->setTitle("Residual (OD)");
        graph->axisY()->setRange(min_sErr - padding, max_sErr + padding);
        formatY = new CustomFormatter(min_Err, max_Err);
    }
    graph->axisY()->setFormatter(formatY);

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

void US_MWL_SF_PLOT3D::resetCamera(int){
    camera->disconnect();
    QString state = cb_camera->currentText();
    if (state == "FrontLow")
        camera->setCameraPreset(Q3DCamera::CameraPresetFrontLow);
    else if (state == "Front")
        camera->setCameraPreset(Q3DCamera::CameraPresetFront);
    else if (state == "FrontHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetFrontHigh);
    else if (state == "LeftLow")
        camera->setCameraPreset(Q3DCamera::CameraPresetLeftLow);
    else if (state == "Left")
        camera->setCameraPreset(Q3DCamera::CameraPresetLeft);
    else if (state == "LeftHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetLeftHigh);
    else if (state == "RightLow")
        camera->setCameraPreset(Q3DCamera::CameraPresetRightLow);
    else if (state == "Right")
        camera->setCameraPreset(Q3DCamera::CameraPresetRight);
    else if (state == "RightHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetRightHigh);
    else if (state == "BehindLow")
        camera->setCameraPreset(Q3DCamera::CameraPresetBehindLow);
    else if (state == "Behind")
        camera->setCameraPreset(Q3DCamera::CameraPresetBehind);
    else if (state == "BehindHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetBehindHigh);
    else if (state == "IsometricLeft")
        camera->setCameraPreset(Q3DCamera::CameraPresetIsometricLeft);
    else if (state == "IsometricLeftHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetIsometricLeftHigh);
    else if (state == "IsometricRight")
        camera->setCameraPreset(Q3DCamera::CameraPresetIsometricRight);
    else if (state == "IsometricRightHigh")
        camera->setCameraPreset(Q3DCamera::CameraPresetIsometricRightHigh);
    else if (state == "DirectlyAbove")
        camera->setCameraPreset(Q3DCamera::CameraPresetDirectlyAbove);
    else if (state == "DirectlyAboveCW45")
        camera->setCameraPreset(Q3DCamera::CameraPresetDirectlyAboveCW45);
    else if (state == "DirectlyAboveCCW45")
        camera->setCameraPreset(Q3DCamera::CameraPresetDirectlyAboveCCW45);
    else if (state == "FrontBelow")
        camera->setCameraPreset(Q3DCamera::CameraPresetFrontBelow);
    else if (state == "LeftBelow")
        camera->setCameraPreset(Q3DCamera::CameraPresetLeftBelow);
    else if (state == "RightBelow")
        camera->setCameraPreset(Q3DCamera::CameraPresetRightBelow);
    else if (state == "BehindBelow")
        camera->setCameraPreset(Q3DCamera::CameraPresetBehindBelow);
    else if (state == "DirectlyBelow")
        camera->setCameraPreset(Q3DCamera::CameraPresetDirectlyBelow);
    connect(camera,   SIGNAL(xRotationChanged(float)), this, SLOT(cameraChanged(float)));
    connect(camera,   SIGNAL(yRotationChanged(float)), this, SLOT(cameraChanged(float)));
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
    set_radial_slider();
    plot2d();
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
    set_radial_slider();
    plot2d();
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
    int npts = idRP_h - idRP_l;
    int rpId = le_rpid->text().toInt(&state);
    if (! state || rpId <= 0)
        rpId = 0;
    else if (rpId > npts + 1)
        rpId = npts - 1;
    else
        rpId--;
    le_rpid->setText(QString::number(rpId + 1));
    sli_radial->setValue(rpId);

}

void US_MWL_SF_PLOT3D::plot2d(){
    errorPlot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    dataPlot->detachItems(QwtPlotItem::Rtti_PlotItem, false);

    int nwl = idWL_h - idWL_l + 1;
    QVector<double> dev(nwl, 0);
    QVector<double> lambdas(nwl, 0);
    double *yp = dev.data();
    double *xp = lambdas.data();
    double minDev =  1e20;
    double maxDev = -1e20;
    int rpId = sli_radial->value();
    int rpId_org = rpId + idRP_l;
    le_rpid->setText(QString::number(rpId + 1));
    double xvalue = (double) xvals4ct.at(rpId_org) / 1000.0;
    QString rpval = QString::number(xvalue, 'f', 3);
    le_rpval->setText(rpval);
    lb_runs_test->setText(tr("Runs Test Statistics for the Radius %1 (cm)").arg(rpval));
    for (int i = 0; i < nwl; ++i){
        int id_wl = i + idWL_l;
        double error = allData.at(scanId).at(rpId_org).at(id_wl).at(2);
        minDev = qMin(minDev, error);
        maxDev = qMax(maxDev, error);
        xp[i] = (double) lambda4ct.at(id_wl) / 10.0;
        yp[i] = error;
    }
    double minWl = xp[0] - 0.1;
    double maxWl = xp[nwl - 1] + 0.1;
    QwtPlotCurve* curve = us_curve( errorPlot, "");
    QPen pen_plot(Qt::yellow);
    pen_plot.setWidth(2);
    curve->setPen( pen_plot );
    curve->setSamples(xp, yp, nwl);
    double dy = (maxDev - minDev) * 0.01;
    errorPlot->setAxisScale( QwtPlot::xBottom, minWl, maxWl);
    errorPlot->setAxisScale( QwtPlot::yLeft  , minDev - dy, maxDev + dy);
    errorPlot->updateAxes();
    grid = us_grid(errorPlot);
    errorPlot->replot();


    get_runs_test(dev, true);

    QVector<double> od_raw(nwl, 0);
    QVector<double> od_fit(nwl, 0);
    double *rp = od_raw.data();
    double *fp = od_fit.data();
    double minVal =  1e20;
    double maxVal = -1e20;
    for (int i = 0; i < nwl; i++){
        int id_wl = i + idWL_l;
        double raw = allData.at(scanId).at(rpId_org).at(id_wl).at(0);
        rp[i] = raw;
        double fit = allData.at(scanId).at(rpId_org).at(id_wl).at(1);
        fp[i] = fit;
        double minV = qMin(raw, fit);
        double maxV = qMax(raw, fit);
        minVal = qMin(minVal, minV);
        maxVal = qMax(maxVal, maxV);
    }
    QwtPlotCurve* curve_org = us_curve( dataPlot, "Raw Data");
    pen_plot.setColor(Qt::green);
    curve_org->setPen( pen_plot );
    curve_org->setSamples(xp, rp, nwl);

    QPen nopen = QPen(Qt::red, 0, Qt::NoPen);
    QwtSymbol *symbol = new QwtSymbol(
                QwtSymbol::Ellipse, QBrush(Qt::red),
                QPen(Qt::red, 3), QSize( 5, 5 ));
    QwtPlotCurve* curve_fit = us_curve( dataPlot, "Fitted Data");
    curve_fit->setSamples(xp, fp, nwl);
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

    min_Err   =  1e99;
    max_Err   = -1e99;
    min_sErr  =  1e99;
    max_sErr  = -1e99;
    min_AbsErr  =  1e99;
    max_AbsErr  = -1e99;
    min_sAbsErr =  1e99;
    max_sAbsErr = -1e99;
    RMSE = 0;
    int n = 0;
    for (int i = 0; i < nScans; i++){
        for (int j = idRP_l; j <= idRP_h ; j++){
            for (int k = idWL_l; k <= idWL_h; k++) {
                double err = allData.at(i).at(j).at(k).at(2);
                double sErr = allData.at(i).at(j).at(k).at(3);
                double absErr = allData.at(i).at(j).at(k).at(4);
                double sAbsErr = allData.at(i).at(j).at(k).at(5);
                RMSE += qPow(err, 2);
                min_Err   = qMin(min_Err,   err);
                min_sErr  = qMin(min_sErr,  sErr);
                min_AbsErr  = qMin(min_AbsErr,  absErr);
                min_sAbsErr = qMin(min_sAbsErr, sAbsErr);

                max_Err   = qMax(max_Err,   err);
                max_sErr  = qMax(max_sErr,  sErr);
                max_AbsErr  = qMax(max_AbsErr,  absErr);
                max_sAbsErr = qMax(max_sAbsErr, sAbsErr);
                n++;
            }
        }
    }
    RMSE = qSqrt(RMSE / n);
    le_RMSE->setText(QString::number(RMSE));
    le_minErr->setText(QString::number(min_Err));
    le_maxErr->setText(QString::number(max_Err));
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

void US_MWL_SF_PLOT3D::cameraChanged(float){
    cb_camera->setCurrentIndex(0);
}

void US_MWL_SF_PLOT3D::set_radial_slider(void){
    sli_radial->disconnect();
    le_rpid->disconnect();
    int npts = idRP_h - idRP_l;
    sli_radial->setMinimum(0);
    sli_radial->setMaximum(npts);
    sli_radial->setValue(npts / 2);
    connect(sli_radial, SIGNAL(valueChanged(int)), this, SLOT(new_rpoint(int)));
    connect(le_rpid, SIGNAL(returnPressed()), this, SLOT(new_rpid()));
}

bool US_MWL_SF_PLOT3D::get_runs_test(QVector<double> sample, bool display){
    QVector<double> sample_sort;
    sample_sort << sample;
    std::sort(sample_sort.begin(), sample_sort.end());

    double median, mean, rhat, s2r, zstat, pval;
    int nneg, npos, nruns, lower, upper;
    bool randstate, lookup;
    int size = sample.size();
    if (size % 2 != 0)
        median = sample_sort[size / 2];
    else
        median = (sample_sort[(size - 1) / 2] + sample_sort[size / 2]) / 2.0;
    mean = 0;
    nneg = 0;
    npos = 0;
    nruns = 0;
    double *sp = sample.data();
    for (int i = 0; i < size; i++){
        double val = sp[i];
        mean += val;
        if (i > 0){
            double valp = sp[i - 1];
            if ((val >= median && valp <  median) ||
                (val <  median && valp >= median))
                nruns++;
        }
        if (val >= median)
            npos++;
        else
            nneg++;
    }
    nruns++;
    mean /= size;
    double npn = nneg + npos;
    double ntn = 2 * nneg * npos;
    rhat = ntn / npn + 1.0;
    s2r = (ntn * (ntn - npn)) / (npn * npn * (npn + 1.0));
    zstat = qAbs(((double) nruns - rhat) / qSqrt(s2r));
    double cdf = std::erfc(-1 * zstat / std::sqrt(2.0)) / 2.0;
    pval = 2 * (1 - cdf);
    int n1, n2;
    if (npos >= nneg){
        n1 = npos;
        n2 = nneg;
    } else {
        n1 = nneg;
        n2 = npos;
    }

    if (n1 > 20){
        lookup = false;
        lower = -1;
        upper = -1;
        if (qAbs(zstat) >= 1.96)
            randstate = false;
        else
            randstate = true;
    } else {
        lookup = true;
        if (lookupTable.contains(n1)){
            QMap<int, QVector<int>> row;
            row = lookupTable.value(n1);
            if (row.contains(n2)){
                lower = row.value(n2).at(0);
                upper = row.value(n2).at(1);
                if (nruns >= lower && nruns  <= upper)
                    randstate = true;
                else
                    randstate = false;
            } else {
                lower = -1;
                upper = -1;
                randstate = false;
            }
        } else {
            lower = -1;
            upper = -1;
            randstate = false;
        }
    }

    if (display){
        le_rt_mean->setText(QString::number(mean));
        le_rt_median->setText(QString::number(median));
        le_rt_np->setText(QString::number(sample.size()));
        le_rt_nruns->setText(QString::number(nruns));
        le_rt_nneg->setText(QString::number(nneg));
        le_rt_npos->setText(QString::number(npos));
        QString l2u = tr("%1 to %2").arg(lower).arg(upper);
        if (lookup){
            le_rt_region->setText(l2u);
            if (n1 > 10){
                le_rt_zstat->setText(QString::number(zstat));
                le_rt_pval->setText(QString::number(pval));
                le_rt_zcrit->setText(QString::number(1.96));
            } else {
                le_rt_zstat->clear();
                le_rt_pval->clear();
                le_rt_zcrit->clear();
            }
        } else {
            le_rt_zstat->setText(QString::number(zstat));
            le_rt_pval->setText(QString::number(pval));
            le_rt_zcrit->setText(QString::number(1.96));
            le_rt_region->setText(l2u);
        }

        if (randstate){
            QColor color = QColor(144, 238, 144);
            lb_rt_rstate->setText("Random");
            lb_rt_rstate->setStyleSheet(tr("QLabel { background-color : %1; color : black; }").arg(color.name()));
        } else {
            QColor color = QColor(255, 87, 51);
            lb_rt_rstate->setText("Non-random");
            lb_rt_rstate->setStyleSheet(tr("QLabel { background-color : %1; color : black; }").arg(color.name()));
        }
    }
    return randstate;
}

void US_MWL_SF_PLOT3D::fill_table(){
    lookupTable.clear();
    QStringList tableRows;
    tableRows << "20,15,27,19,14,26,18,14,26,17,14,25,16,13,24,15,13,24,14,12,23,13,11,22,12,11,21,11,10,20,10,10,19,9,9,19,8,8,16,7,7,-1,6,7,-1,5,6,-1,4,5,-1,3,4,-1,2,3,-1";
    tableRows << "19,14,26,18,14,25,17,13,25,16,13,24,15,12,23,14,12,22,13,11,21,12,11,21,11,10,20,10,9,19,9,9,19,8,8,16,7,7,-1,6,7,-1,5,6,-1,4,5,-1,3,4,-1,2,3,-1";
    tableRows << "18,13,25,17,13,24,16,12,24,15,12,23,14,11,22,13,11,21,12,10,21,11,10,19,10,9,18,9,9,19,8,8,16,7,7,-1,6,6,-1,5,6,-1,4,5,-1,3,4,-1,2,3,-1";
    tableRows << "17,12,24,16,12,23,15,12,22,14,11,22,13,11,21,12,10,20,11,10,19,10,9,18,9,8,19,8,8,16,7,7,-1,6,6,-1,5,5,-1,4,5,-1,3,4,-1,2,3,-1";
    tableRows << "16,12,22,15,11,22,14,11,21,13,10,21,12,10,21,11,9,19,10,9,18,9,8,19,8,7,16,7,7,-1,6,6,-1,5,5,-1,4,5,-1,3,4,-1,2,3,-1";
    tableRows << "15,11,21,14,10,21,13,10,20,12,9,19,11,9,18,10,8,17,9,8,19,8,7,15,7,7,14,6,6,-1,5,5,-1,4,4,-1,3,4,-1,2,3,-1";
    tableRows << "14,10,20,13,10,19,12,9,19,11,9,18,10,8,17,9,8,16,8,7,15,7,6,14,6,6,-1,5,5,-1,4,4,-1,3,3,-1,2,3,-1";
    tableRows << "13,9,19,12,8,19,11,8,18,10,8,17,9,7,16,8,7,15,7,6,14,6,6,-1,5,5,-1,4,4,-1,3,3,-1,2,3,-1";
    tableRows << "12,8,18,11,8,17,10,8,16,9,7,15,8,7,15,7,6,13,6,5,12,5,5,-1,4,4,-1,3,3,-1,2,3,-1";
    tableRows << "11,8,16,10,7,16,9,7,15,8,6,14,7,6,13,6,5,12,5,5,-1,4,4,-1,3,3,-1,2,-1,-1";
    tableRows << "10,7,15,9,6,15,8,6,14,7,6,13,6,5,12,5,4,-1,4,4,-1,3,3,-1,2,-1,-1";
    tableRows << "9,6,14,8,6,13,7,5,13,6,5,12,5,4,-1,4,4,-1,3,3,-1,2,-1,-1";
    tableRows << "8,5,13,7,5,12,6,4,11,5,4,10,4,4,-1,3,3,-1,2,-1,-1";
    tableRows << "7,4,12,6,4,11,5,4,10,4,3,-1,3,3,-1,2,-1,-1";
    tableRows << "6,4,10,5,4,9,4,3,8,3,3,-1,2,-1,-1";
    tableRows << "5,3,9,4,3,8,3,-1,-1,2,-1,-1";
    int key = 20;
    for (int i = 0; i < tableRows.size(); i++){
        QMap<int, QVector<int>> row;
        row = new_table_row(tableRows.at(i));
        lookupTable.insert(key, row);
        key--;
    }
}

QMap<int, QVector<int>> US_MWL_SF_PLOT3D::new_table_row(QString rowStr){
    QMap<int, QVector<int>> rowMap;
    QStringList rlist = rowStr.split(u',');
    for (int i = 0; i < rlist.size() / 3; i++){
        int n2 = rlist.at(3 * i).toInt();
        int l = rlist.at(3 * i + 1).toInt();
        int u = rlist.at(3 * i + 2).toInt();
        QVector<int> lu;
        lu << l << u;
        rowMap.insert(n2, lu);
    }
    return rowMap;
}

void US_MWL_SF_PLOT3D::plotPixMap(){
    QVector<QVector<double>> allStates;

    QVector<double> scans;
    for (int i = 0; i < nScans; i++)
        scans << i + 1;
    QVector<double> rpoints;
    for (int i = idRP_l; i <= idRP_h; i++)
        rpoints << (double) xvals4ct.at(i) / 1000.0;

    for (int i = 0; i < nScans; i++){
        QVector<double> rnd_sc;
        for (int j = idRP_l; j <= idRP_h; j++){
            QVector<double> err;
            for (int k = idWL_l; k <= idWL_h; k++){
                err << allData.at(i).at(j).at(k).at(2);
            }
            bool state = get_runs_test(err, false);
            if (state)
                rnd_sc << 1.0;
            else
                rnd_sc << -1.0;
        }
        allStates << rnd_sc;
    }

    RunsTestWidget wd(rpoints, scans, allStates);
//    wd.resize( 600, 400 );
    wd.exec();
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

    qreal delta = 1.0 / segmentCount;
    for (int i = 0; i < segmentCount; i++) {
        values[i] = delta * i;
    }
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

//////

class MyZoomer: public QwtPlotZoomer
{
public:
    MyZoomer( QWidget *canvas ):
        QwtPlotZoomer( canvas )
    {
        setTrackerMode( AlwaysOn );
    }

    virtual QwtText trackerTextF( const QPointF &pos ) const
    {
        QColor bg( Qt::white );
        bg.setAlpha( 200 );

        QwtText text = QwtPlotZoomer::trackerTextF( pos );
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }
};

SpectrogramData::SpectrogramData(QVector<double> points, QVector<double> scans,
                                 QVector<QVector<double>> states):QwtRasterData(){
    xrange.clear();
    for (int i = 0; i < points.size() - 1; i++){
        double mid = 0.5 * (points.at(i) + points.at(i + 1));
        if (i == 0)
            xrange << points.first() - (mid - points.first());
        xrange << mid;
    }
    xrange << points.last() + (points.last() - xrange.last());

    yrange.clear();
    for (int i = 0; i < scans.size() - 1; i++){
        double mid = 0.5 * (scans.at(i) + scans.at(i + 1));
        if (i == 0)
            yrange << scans.first() - (mid - scans.first());
        yrange << mid;
    }
    yrange << scans.last() + (scans.last() - yrange.last());

    setInterval( Qt::XAxis, QwtInterval( xrange.first(), xrange.last() ) );
    setInterval( Qt::YAxis, QwtInterval( yrange.first(), yrange.last() ) );
    setInterval( Qt::ZAxis, QwtInterval( 0, 1 ) );

    zvals << states;
}

double SpectrogramData::value( double x, double y ) const{
    int idx = -1;
    for (int i = 0; i < xrange.size() - 1; i++){
        double x1 = xrange.at(i);
        double x2 = xrange.at(i + 1);
        if (x >= x1 && x < x2){
            idx = i;
            break;
        }
    }
    if (idx == -1)
        idx = xrange.size() - 2;


    int idy = -1;
    for (int i = 0; i < yrange.size() - 1; i++){
        double y1 = yrange.at(i);
        double y2 = yrange.at(i + 1);
        if (y >= y1 && y < y2){
            idy = i;
            break;
        }
    }
    if (idy == -1)
        idy = yrange.size() - 2;
    double z = zvals.at(idy).at(idx);
    return z;
}

RunsTestWidget::RunsTestWidget(QVector<double> points, QVector<double> scans,
                               QVector<QVector<double>> states,  QWidget *parent ):
    QDialog( parent )
{

    this->setWindowTitle("Runs Test PixelMap");
    Qt::WindowFlags flags;
    flags = flags | Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint
            | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
    this->setMinimumSize(1000, 800);

    QToolBar *toolBar = new QToolBar( this );
    QToolButton *pb_print = new QToolButton( toolBar );
    pb_print->setText( "Print" );
    pb_print->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( pb_print );
    connect( pb_print, SIGNAL( clicked() ),
        this, SLOT( printPlot() ) );

    QToolButton *pb_image = new QToolButton( toolBar );
    pb_image->setText( "Save" );
    pb_image->setToolButtonStyle( Qt::ToolButtonTextUnderIcon );
    toolBar->addWidget( pb_image );
//    connect( btnPrint, SIGNAL( clicked() ),
//        d_plot, SLOT( printPlot() ) );

    toolBar->addSeparator();

    QStringList colorList;
    colorList << "white"<< "black"<< "red"<< "darkRed"<< "green"<<
                 "darkGreen"<< "blue"<< "darkBlue"<< "cyan"<<
                 "darkCyan"<< "magenta"<< "darkMagenta"<< "yellow"<<
                 "darkYellow"<< "gray"<< "darkGray"<< "lightGray";
    toolBar->addWidget( new QLabel("Random Points:" ) );
    cb_color_r = new QComboBox( toolBar );
    cb_color_r->addItems( colorList );
    cb_color_r->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    cb_color_r->setCurrentText("green");
    toolBar->addWidget( cb_color_r );
    connect( cb_color_r, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( setColorMap(  ) ) );

    toolBar->addWidget( new QLabel("Non-random Points:" ) );
    cb_color_nr = new QComboBox( toolBar );
    cb_color_nr->addItems( colorList );
    cb_color_nr->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
    cb_color_nr->setCurrentText("darkBlue");
    toolBar->addWidget( cb_color_nr );
    connect( cb_color_nr, SIGNAL( currentIndexChanged( int ) ),
             this, SLOT( setColorMap(  ) ) );

    toolBar->addSeparator();

//    QCheckBox *ckb_cont = new QCheckBox( "Contour", toolBar );
//    toolBar->addWidget( ckb_cont );
//    ckb_cont->setChecked( false );
//    connect( ckb_cont, SIGNAL( toggled( bool ) ),
//        this, SLOT( showContour( bool ) ) );

    d_plot = new QwtPlot();

    d_spectrogram = new QwtPlotSpectrogram();
    d_spectrogram->setRenderThreadCount( 0 ); // use system specific thread count
    d_spectrogram->setCachePolicy( QwtPlotRasterItem::PaintCache );
//    QList<double> contourLevels{-1, 1};
//    d_spectrogram->setContourLevels( contourLevels );
    d_spectrogram->setData( new SpectrogramData(points, scans, states) );
    d_spectrogram->attach(d_plot);
    d_plot->setAxisTitle(QwtPlot::xBottom, "Radial Points (cm)");
    d_plot->setAxisTitle(QwtPlot::yLeft, "Scan Number");
    QFont font;
    QwtText qwtText;
    qwtText = d_plot->axisTitle(QwtPlot::xBottom);
    qwtText.setText("Radial Point (cm)");
    font = qwtText.font();
    font.setPointSize(20);
    font.setBold(true);
    qwtText.setFont(font);
    d_plot->setAxisTitle(QwtPlot::xBottom, qwtText);
    font.setPointSize(18);
    font.setBold(true);
    d_plot->setAxisFont(QwtPlot::xBottom, font);
    
    qwtText = d_plot->axisTitle(QwtPlot::xBottom);
    qwtText.setText("Scan Number");
    font.setPointSize(20);
    font.setBold(true);
    qwtText.setFont(font);
    d_plot->setAxisTitle(QwtPlot::yLeft, qwtText);
    font = d_plot->axisFont(QwtPlot::yLeft);
    font.setPointSize(18);
    font.setBold(true);
    d_plot->setAxisFont(QwtPlot::yLeft, font);
//    const QwtInterval zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
//    // A color bar on the right axis
//    QwtScaleWidget *rightAxis = d_plot->axisWidget( QwtPlot::yRight );
//    rightAxis->setTitle( "Randomness" );
//    rightAxis->setColorBarEnabled( true );
//    d_plot->setAxisScale( QwtPlot::yRight, zInterval.minValue(), zInterval.maxValue() );
//    d_plot->enableAxis( QwtPlot::yRight );

    d_plot->plotLayout()->setAlignCanvasToScales( true );

    setColorMap();

    QwtPlotZoomer* zoomer = new MyZoomer( d_plot->canvas() );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect2,
        Qt::RightButton, Qt::ControlModifier );
    zoomer->setMousePattern( QwtEventPattern::MouseSelect3,
        Qt::RightButton );

    QwtPlotPanner *panner = new QwtPlotPanner( d_plot->canvas() );
    panner->setAxisEnabled( QwtPlot::yRight, false );
    panner->setMouseButton( Qt::MidButton );

    // LeftButton for the zooming
    // MidButton for the panning
    // RightButton: zoom out by 1
    // Ctrl+RighButton: zoom out to full size

//    const QFontMetrics fm( d_plot->axisWidget( QwtPlot::yLeft )->font() );
//    QwtScaleDraw *sd = d_plot->axisScaleDraw( QwtPlot::yLeft );
//    sd->setMinimumExtent( fm.width( "100.00" ) );

    const QColor c( Qt::darkBlue );
    zoomer->setRubberBandPen( c );
    zoomer->setTrackerPen( c );

    QVBoxLayout *main_lyt = new QVBoxLayout();
    main_lyt->addWidget(toolBar);
//    main_lyt->addStretch(1);
    main_lyt->addWidget(d_plot);
    this->setLayout(main_lyt);

}

QColor RunsTestWidget::getColor(QString cname){
    QColor color;
    if (cname == "white")
        color = QColor(Qt::white);
    else if (cname == "black")
        color = QColor(Qt::black);
    else if (cname == "red")
        color = QColor(Qt::red);
    else if (cname == "darkRed")
        color = QColor(Qt::darkRed);
    else if (cname == "green")
        color = QColor(Qt::green);
    else if (cname == "darkGreen")
        color = QColor(Qt::darkGreen);
    else if (cname == "blue")
        color = QColor(Qt::blue);
    else if (cname == "darkBlue")
        color = QColor(Qt::darkBlue);
    else if (cname == "cyan")
        color = QColor(Qt::cyan);
    else if (cname == "darkCyan")
        color = QColor(Qt::darkCyan);
    else if (cname == "magenta")
        color = QColor(Qt::magenta);
    else if (cname == "darkMagenta")
        color = QColor(Qt::darkMagenta);
    else if (cname == "yellow")
        color = QColor(Qt::yellow);
    else if (cname == "darkYellow")
        color = QColor(Qt::darkYellow);
    else if (cname == "gray")
        color = QColor(Qt::gray);
    else if (cname == "darkGray")
        color = QColor(Qt::darkGray);
    else if (cname == "lightGray")
        color = QColor(Qt::lightGray);
    return color;
}

void RunsTestWidget::setColorMap(){
    QColor color_r = getColor(cb_color_r->currentText());
    QColor color_nr = getColor(cb_color_nr->currentText());
//    QwtScaleWidget *axis = d_plot->axisWidget( QwtPlot::yRight );
//    const QwtInterval zInterval = d_spectrogram->data()->interval( Qt::ZAxis );
    d_spectrogram->setColorMap( new QwtLinearColorMap( color_nr, color_r, QwtColorMap::RGB ) );
//    axis->setColorMap( zInterval, new QwtLinearColorMap( color_1, color_2, QwtColorMap::RGB ) );
    int alpha = 250;
    d_spectrogram->setAlpha( alpha );
    d_plot->replot();
}

void RunsTestWidget::showContour( bool on )
{
    d_spectrogram->setDisplayMode( QwtPlotSpectrogram::ContourMode, on );
    d_plot->replot();
}

void RunsTestWidget::printPlot()
{
    QPrinter printer( QPrinter::HighResolution );
    printer.setOrientation( QPrinter::Landscape );
    printer.setOutputFileName( "spectrogram.pdf" );

    QPrintDialog dialog( &printer );
    if ( dialog.exec() )
    {
        QwtPlotRenderer renderer;

        if ( printer.colorMode() == QPrinter::GrayScale )
        {
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasBackground );
            renderer.setDiscardFlag( QwtPlotRenderer::DiscardCanvasFrame );
            renderer.setLayoutFlag( QwtPlotRenderer::FrameWithScales );
        }

        renderer.renderTo( d_plot, printer );
    }
}
