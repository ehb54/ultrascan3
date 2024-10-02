//! \file us_mwl_sf_plot3d.h
#ifndef US_MWL_SF_PLOT3D_H
#define US_MWL_SF_PLOT3D_H

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <cmath>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>
#include <QtDataVisualization/QHeightMapSurfaceDataProxy>
#include <QtDataVisualization/QSurface3DSeries>
#include <QtDataVisualization/QCustom3DLabel>
#include <QtDataVisualization/QValue3DAxisFormatter>
#include <QtDataVisualization/QValue3DAxis>
#include <QtWidgets/QSlider>
#include "us_widgets_dialog.h"
#include "us_gui_settings.h"
#include "us_images.h"
#include "us_settings.h"
#include "us_plot.h"
#include "qwt_legend.h"

#include <qwt_plot.h>
#include <qwt_plot_spectrogram.h>
#include <qapplication.h>
#include <qmainwindow.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qcombobox.h>
#include <qslider.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include "qwt_color_map.h"

#include <qprinter.h>
#include <qprintdialog.h>
#include <qnumeric.h>
#include <qwt_color_map.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_renderer.h>


using namespace QtDataVisualization;

//////
/// \brief The SFData class
///
class SFData {
public:
    QVector< double > wavelenghts;
    QVector< int > includedScans;
    QVector< double > xValues;
    //scan < radial < lambda  < spiecies > > > >
    // spiecies: raw_data, spiecies1, spiecies2, ..., sum_spiecies
    QVector< QVector< QVector < QVector < double > > > > allData;
    QVector< double > scansMSE;
    void clear(void);
    void computeMSE(void);
};

/////
/// \brief The US_MWL_SF_PLOT3D class
///
class US_MWL_SF_PLOT3D : public US_WidgetsDialog
{
    Q_OBJECT
public:
    US_MWL_SF_PLOT3D(QWidget* w, const SFData&);
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
   // scan < radial < lambda  < raw, fit, Err, sErr, absErr, sAbsErr > > > >
    QVector< QVector< QVector < QVector < double > > > > allData;
    QMap<int, QMap<int, QVector<int>>> lookupTable;
    double min_sAbsErr;
    double max_sAbsErr;
    double min_AbsErr;
    double max_AbsErr;
    double min_Err;
    double max_Err;
    double min_sErr;
    double max_sErr;
    double RMSE;
    double padding;
    int idRP_l;
    int idRP_h;
    int idWL_l;
    int idWL_h;
    int xAngle;
    int yAngle;
    int zAngle;
    bool renderLoopState;
    bool renderRunState;

    Q3DSurface *graph;
    Q3DCamera *camera;
    QWidget *surfaceWgt;
    QTabWidget *tabs;
    QSurfaceDataProxy *dataProxy;
    QSurface3DSeries *dataSeries;

    QRadioButton *rb_nosel;
    QRadioButton *rb_point;
    QRadioButton *rb_radial;
    QRadioButton *rb_lambda;

    QwtPlot *dataPlot;
    QwtPlot *errorPlot;
    QwtPlotGrid *grid;

    QPushButton *pb_next;
    QPushButton *pb_prev;
    QPushButton *pb_render;
    QPushButton *pb_G2R;
    QPushButton *pb_B2Y;
    QPushButton *pb_DFLT;
    QPushButton *pb_help;
    QPushButton *pb_close;
    QPushButton *pb_plotPixMap;

    QSlider *sli_xAngle;
    QSlider *sli_yAngle;
    QSlider *sli_zAngle;
    QSlider *sli_radial;

    QLineEdit *le_rt_mean;
    QLineEdit *le_rt_median;
    QLineEdit *le_rt_np;
    QLineEdit *le_rt_nruns;
    QLineEdit *le_rt_nneg;
    QLineEdit *le_rt_npos;
    QLineEdit *le_rt_zstat;
    QLineEdit *le_rt_pval;
    QLineEdit *le_rt_zcrit;
    QLineEdit *le_rt_region;
    QLabel *lb_rt_rstate;
    QLabel *lb_runs_test;

    QLineEdit *le_rpval;
    QLineEdit *le_rpid;
    QLineEdit *le_RMSE;
    QLineEdit *le_minErr;
    QLineEdit *le_maxErr;

    QComboBox *cb_scan;
    QComboBox *cb_theme;
    QComboBox *cb_camera;

    QwtCounter *ct_min_rp;
    QwtCounter *ct_max_rp;
    QwtCounter *ct_min_wl;
    QwtCounter *ct_max_wl;
    QwtCounter *ct_quality;
    QwtCounter *ct_scale;

    QCheckBox *ckb_rendall;
    QCheckBox *ckb_plotAbs;

    void fill_table(void);
    QMap<int, QVector<int>> new_table_row(QString);
    bool get_runs_test(QVector<double>, bool);
protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void setTheme(const QString);
    void newScan(int);
    void nextScan(void);
    void prevScan(void);
    void resetCamera(int);
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
    void renderImage(void);
    void new_xAngle(int);
    void new_yAngle(int);
    void new_zAngle(int);
    void new_rpoint(int);
    void reset_xAngle(void);
    void reset_yAngle(void);
    void reset_zAngle(void);
    void plot3d(void);
    void plot2d(void);
    void set_ct_rp(bool);
    void set_ct_wl(bool);
    void new_rpid();
    void get_minMaxMean(void);
    void render_option(int);
    void enable_wgt(bool);
    void cameraChanged(float);
    void set_radial_slider(void);
    void plotPixMap(void);
};

class CustomFormatter : public QValue3DAxisFormatter
{
    Q_OBJECT
//    Q_DECLARE_METATYPE(QValue3DAxisFormatter *)

public:
    explicit CustomFormatter(qreal i_minval = 0, qreal i_maxval = 1);
    virtual ~CustomFormatter();
protected:
    virtual QValue3DAxisFormatter *createNewInstance() const;
    virtual void populateCopy(QValue3DAxisFormatter &copy) const;
    virtual void recalculate();
    virtual QString stringForValue(qreal value, const QString &format) const;

private:
    Q_DISABLE_COPY(CustomFormatter)

    qreal minVal;
    qreal maxVal;

};

class SpectrogramData: public QwtRasterData
{
public:
    SpectrogramData(QVector<double> rp, QVector<double> sc,
                    QVector<QVector<double>> st);

    virtual double value( double x, double y ) const;

private:
    QVector<QVector<double>> zvals;
    QVector<double> xrange;
    QVector<double> yrange;
};

class RunsTestWidget: public QDialog
{
    Q_OBJECT
public:
    RunsTestWidget(QVector<double>, QVector<double>, QVector<QVector<double>>, QWidget * = NULL);

private:
    QwtPlot *d_plot;
    QwtPlotSpectrogram *d_spectrogram;
    QComboBox *cb_color_r;
    QComboBox *cb_color_nr;

private Q_SLOTS:
    void setColorMap(void);
    void showContour(bool);
    void printPlot(void);
    QColor getColor(QString);

};


#endif // US_MWL_SF_PLOT3D_H
