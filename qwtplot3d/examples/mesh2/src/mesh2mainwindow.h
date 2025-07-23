#ifndef mesh2mainwindow_h__2004_03_07_13_38_begin_guarded_code
#define mesh2mainwindow_h__2004_03_07_13_38_begin_guarded_code

#include "../../../include/qwt3d_surfaceplot.h"


#if QT_VERSION < 0x040000
#include "mesh2mainwindowbase.h"
#else
#include "ui_mesh2mainwindowbase4.h"
#include "designerworkaround.h"
#endif



//MOC_SKIP_BEGIN
#if QT_VERSION < 0x040000
  class DummyBase : public Mesh2MainWindowBase
  {
  public:
    DummyBase(QWidget* parent = 0) 
      : Mesh2MainWindowBase(parent) 
    {
    } 
  };
#else
  class DummyBase : public QMainWindow, protected Ui::MainWindow, protected DesignerWorkaround
  {
  public:
    DummyBase(QWidget* parent = 0) 
      : QMainWindow(parent) 
    {
    } 
  };
#endif
//MOC_SKIP_END

class QLabel;
class QTimer;
class QAction;
class QFileDialog;
class LightingDlg;
class ColorMapPreview;

class Mesh2MainWindow : public DummyBase
{
	Q_OBJECT
public:
	Mesh2MainWindow( QWidget* parent = 0 );
	~Mesh2MainWindow();

  Qwt3D::SurfacePlot* dataWidget;
	
public slots:
	void open();
	void openMesh();

	void createFunction(QString const& name);
	void createPSurface(QString const& name);
	void setFileType(QString const& name);

	void pickCoordSystem( QAction* );
	void pickPlotStyle( QAction* );
	void pickFloorStyle( QAction* );
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
	void toggleAnimation(bool);
	void toggleProjectionMode(bool);
	void toggleColorLegend(bool);
	void toggleAutoScale(bool val);
	void toggleShader(bool val);
	void rotate();
	void setPolygonOffset(int);

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

	void setLeftGrid( bool b );
	void setRightGrid( bool b );
	void setCeilGrid( bool b );
	void setFloorGrid( bool b );
	void setFrontGrid( bool b );
	void setBackGrid( bool b );
  void setGrid( Qwt3D::SIDE, bool );

  void enableLighting(bool val);

private:
	QLabel *filenameWidget, *dimWidget,
		*rotateLabel, *shiftLabel, *scaleLabel, *zoomLabel;
	
	QTimer* timer;
	int redrawWait;

	QAction* activeCoordSystem;

	bool legend_;
	Qwt3D::StandardColor* col_;

	QFileDialog* datacolordlg_;
    LightingDlg* lightingdlg_;
	QString filetype_;

    // convenience compatib. code
    bool connectA (const QObject* sender, const char * slot);
    bool connectAG (const QObject* sender, const char * slot);

#if QT_VERSION < 0x040000
	ColorMapPreview* colormappv_;
#endif
};

#endif /* include guarded */
