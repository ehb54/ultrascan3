#ifndef US_MWL_SF_PLOT3D_H
#define US_MWL_SF_PLOT3D_H

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QCustom3DLabel>
#include <QtWidgets/QSlider>
#include "us_widgets_dialog.h"
#include "us_gui_settings.h"
#include "us_images.h"

using namespace QtDataVisualization;

class SFDev {
public:
    QVector< int > wavlth;
    QVector< int > inclscns;
    QVector< double > xvalues;
    QVector< QVector< QVector < double > > > dev_snrpwl;
    QVector< double > rmsd_scns;
    void clear(void);
    void calc_rmsd(void);
};

class US_MWL_SF_PLOT3D : public US_WidgetsDialog
{
    Q_OBJECT
public:
    US_MWL_SF_PLOT3D(QWidget* w, const SFDev&);
    ~US_MWL_SF_PLOT3D();

private:

    QString color_set;
    int scid;
    int nscans;
    int npoints;
    int nlambdas;
    int gap;
    double zoom;

    const SFDev* FitDev;

    Q3DSurface *surface;
    QSurfaceDataProxy *surfaceProxy;
    QSurface3DSeries *surfaceSeries;

    QPushButton* pb_next;
    QPushButton* pb_prev;
    QPushButton *pb_B2Y;
    QPushButton *pb_G2R;
    QComboBox*   cb_scan;

    QSlider *sl_min_point;
    QSlider *sl_max_point;
    QSlider *sl_min_wavl;
    QSlider *sl_max_wavl;

    void plot(void);
private slots:
    void set_theme(int);
    void new_scan(int);
    void reset_camera(void);
    void mode_sw(void);
    void mode_s(void);
    void set_B2Y(void);
    void set_G2R(void);
};

#endif // US_MWL_SF_PLOT3D_H
