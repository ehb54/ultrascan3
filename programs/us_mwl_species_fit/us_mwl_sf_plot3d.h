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
#include "us_settings.h"

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
    enum colorSet{G2R, B2Y, DFLT};
    int colorId;
    int scanId;
    int nScans;
    int nPoints;
    int nWavelengths;
    QVector<double> lambdaScaled;
    QVector<int> lambda4ct;
    QVector<double> xvalsScaled;
    QVector<int> xvals4ct;
    QVector< QVector< QVector < double > > > allRmsdScaled;
    QVector< QVector< QVector < double > > > allRmsd;
    double padding;
    int idRP_l;
    int idRP_h;
    int idWL_l;
    int idWL_h;
    int xAngle;
    int yAngle;
    int zAngle;

    Q3DSurface *graph;
    QSurfaceDataProxy *dataProxy;
    QSurface3DSeries *dataSeries;

    QPushButton* pb_next;
    QPushButton* pb_prev;

    QSlider *sli_xAngle;
    QSlider *sli_yAngle;
    QSlider *sli_zAngle;


    QComboBox*   cb_scan;
    QComboBox *cb_theme;

    QwtCounter *ct_min_rp;
    QwtCounter *ct_max_rp;
    QwtCounter *ct_min_wl;
    QwtCounter *ct_max_wl;

    void plot(void);
    void reset_ct_rp(bool);
    void reset_ct_wl(bool);
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
    void set_DFLT(void);
    void adjustRpMin(double);
    void adjustRpMax(double);
    void adjustWlMin(double);
    void adjustWlMax(double);
    void toggleNone(bool);
    void togglePoint(bool);
    void toggleRadial(bool);
    void toggleLambda(bool);
    void saveImage(void);
    void new_xAngle(int);
    void new_yAngle(int);
    void new_zAngle(int);
    void reset_xAngle(void);
    void reset_yAngle(void);
    void reset_zAngle(void);

};

#endif // US_MWL_SF_PLOT3D_H
