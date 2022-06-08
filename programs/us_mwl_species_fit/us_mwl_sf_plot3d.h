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
#include "us_customformatter.h"

using namespace QtDataVisualization;

//////
/// \brief The SFDev class
///
class SFDev {
public:
    QVector< double > wavelenghts;
    QVector< int > includedScans;
    QVector< double > xValues;
    QVector< QVector< QVector < double > > > allDeviations;
    QVector< double > scansRmsd;
    void clear(void);
    void computeRmsd(void);
};

/////
/// \brief The US_MWL_SF_PLOT3D class
///
class US_MWL_SF_PLOT3D : public US_WidgetsDialog
{
    Q_OBJECT
public:
    US_MWL_SF_PLOT3D(QWidget* w, const SFDev&);
    ~US_MWL_SF_PLOT3D();

private:

    QString colorSet;
    int scanId;
    int nScans;
    int nPoints;
    int nWavelengths;
    QVector<double> lambdaScaled;
    QVector<int> lambda4ct;
    QVector<double> xvalsScaled;
    QVector<int> xvals4ct;
    QVector< QVector< QVector < double > > > rmsdScaled;
    QVector<double> offsetRmsd;
    QVector<double> scaleRmsd;
    double coeffRmsd;
    double offsetRP;
    double scaleRP;
    double coeffRP;
    double dRp;
    double offsetWL;
    double scaleWL;
    double coeffWL;
    double dWl;
    float padding;
    int idRP_l;
    int idRP_h;
    int idWL_l;
    int idWL_h;
//    float min_wl;
//    float max_wl;
//    float d_wl;
//    float min_rp;
//    float max_rp;
//    float d_rp;
//    float min_rmsd;
//    float max_rmsd;
//    float h_scale = 1.5;

//    const SFDev* allRMSDs;

    Q3DSurface *graph;
    QSurfaceDataProxy *dataProxy;
    QSurface3DSeries *dataSeries;

    QPushButton* pb_next;
    QPushButton* pb_prev;
    QPushButton *pb_B2Y;
    QPushButton *pb_G2R;
    QComboBox*   cb_scan;

//    QSlider *sl_min_rp;
//    QSlider *sl_max_rp;
//    QSlider *sl_min_wl;
//    QSlider *sl_max_wl;

    QwtCounter *ct_min_rp;
    QwtCounter *ct_max_rp;
    QwtCounter *ct_min_wl;
    QwtCounter *ct_max_wl;

    void plot(void);
private slots:
    void setTheme(int);
    void newScan(int);
    void nextScan(void);
    void prevScan(void);
    void resetCamera(void);
    void setSurfaceWire(void);
    void setSurface(void);
    void set_B2Y(void);
    void set_G2R(void);
    void adjustRpMin(double);
    void adjustRpMax(double);
    void adjustWlMin(double);
    void adjustWlMax(double);
};

#endif // US_MWL_SF_PLOT3D_H
