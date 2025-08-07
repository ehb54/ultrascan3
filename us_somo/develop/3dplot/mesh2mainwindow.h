#ifndef mesh2mainwindow_h__2004_03_07_13_38_begin_guarded_code
#define mesh2mainwindow_h__2004_03_07_13_38_begin_guarded_code

#include <qwt3d_surfaceplot.h>

#include "../include/us_util.h"
#include "mesh2mainwindowbase.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QLabel>

class QLabel;
class QTimer;
class QAction;
class Q3FileDialog;
class ColorMapPreview;
class LightingDlg;

class Mesh2MainWindow : public Mesh2MainWindowBase {
  Q_OBJECT
 public:
  Mesh2MainWindow(bool*, QString, QString, vector<struct element_3D>,
                  SA2d_control_variables*, QWidget* parent = 0,
                  const char* name = 0, Qt::WFlags f = Qt::WType_TopLevel);
  Mesh2MainWindow(bool*, QString, QString, QString, double**,
                  SA2d_control_variables*, QWidget* parent = 0,
                  const char* name = 0, Qt::WFlags f = Qt::WType_TopLevel);
  Mesh2MainWindow(bool*, QString, QString, QString, QString, double**,
                  SA2d_control_variables*, QWidget* parent = 0,
                  const char* name = 0, Qt::WFlags f = Qt::WType_TopLevel);
  ~Mesh2MainWindow();

  Qwt3D::SurfacePlot* dataWidget;

 public slots:
  void open();
  void openMesh();

  void createFunction();
  void setFileType(QString const& name);

  void pickCoordSystem(QAction*);
  void pickPlotStyle(QAction*);
  void pickFloorStyle(QAction*);
  void pickAxesColor();
  void pickBgColor();
  void pickMeshColor();
  void pickNumberColor();
  void pickLabelColor();
  void pickTitleColor();
  void pickDataColor();
  void pickLighting();
  void resetColors();
  void pickNumberFont();
  void pickLabelFont();
  void pickTitleFont();
  void resetFonts();
  void setStandardView();
  void dumpImage();
  void dumpImage(QString, QString);
  void toggleAnimation(bool);
  void toggleProjectionMode(bool);
  void toggleColorLegend(bool);
  void toggleAutoScale(bool val);
  void toggleShader(bool val);
  void rotate();
  void setPolygonOffset(int);

  void setParameters(QString, QString, vector<struct element_3D>,
                     SA2d_control_variables*);
  void setParameters(QString, QString, QString, double**,
                     SA2d_control_variables*);
  void setParameters(QString title_3d, QString xtitle, QString ytitle,
                     QString ztitle, double** data,
                     SA2d_control_variables* sa2d_ctrl_vars);

  void showRotate(double x, double y, double z);
  void showShift(double x, double y);
  void showScale(double x, double y, double z);
  void showZoom(double z);
  void showNormals(bool val);
  void setNormalQuality(int);
  void setNormalLength(int);
  bool openColorMap(Qwt3D::ColorVector& cv, QString fname);
  void adaptDataColors(const QString&);
  void updateColorLegend(int majors, int minors);

  void setLeftGrid(bool b);
  void setRightGrid(bool b);
  void setCeilGrid(bool b);
  void setFloorGrid(bool b);
  void setFrontGrid(bool b);
  void setBackGrid(bool b);
  void setGrid(Qwt3D::SIDE, bool);

  void enableLighting(bool val);
  void closeEvent(QCloseEvent* e);

 public:
  bool* widget_flag;

 private:
  QLabel *filenameWidget, *dimWidget, *rotateLabel, *shiftLabel, *scaleLabel,
      *zoomLabel;
  SA2d_control_variables* sa2d_ctrl_vars;
  vector<struct element_3D> solutes;
  double** data;
  QString xtitle, ytitle, ztitle;
  double x_scale, y_scale, z_scale;

  QTimer* timer;
  int redrawWait;

  QAction* activeCoordSystem;

  bool legend_;
  Qwt3D::StandardColor* col_;

  Q3FileDialog* datacolordlg_;
  ColorMapPreview* colormappv_;
  LightingDlg* lightingdlg_;

  QString filetype_;
};

#endif /* include guarded */
