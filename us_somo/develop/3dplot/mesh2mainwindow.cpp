#include <q3filedialog.h>
#include <q3frame.h>
#include <q3popupmenu.h>
#include <q3toolbar.h>
#include <q3whatsthis.h>
#include <qaction.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qcombobox.h>
#include <qfileinfo.h>
#include <qfontdialog.h>
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmenubar.h>
#include <qmetaobject.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qtimer.h>
#include <qtooltip.h>
// Added by qt3to4:
#include <qwt3d_io.h>
#include <qwt3d_io_gl2ps.h>
#include <qwt3d_io_reader.h>

#include <Q3ComboBox>
#include <Q3GridLayout>
#include <QCloseEvent>
#include <QMenuItem>

#include "colormapreader.h"
#include "femreader.h"
#include "functions.h"
#include "lightingdlg.h"
#include "mesh2mainwindow.h"

using namespace Qwt3D;
using namespace std;

Mesh2MainWindow::~Mesh2MainWindow() { delete dataWidget; }

Mesh2MainWindow::Mesh2MainWindow(bool* widget_flag, QString tmp_xtitle,
                                 QString tmp_ytitle,
                                 vector<struct element_3D> tmp_solutes,
                                 SA2d_control_variables* tmp_sa2d_ctrl_vars,
                                 QWidget* parent, const char* name,
                                 Qt::WFlags f)
    : Mesh2MainWindowBase(parent, name, f) {
  this->widget_flag = widget_flag;
  *widget_flag = true;
  col_ = 0;
  legend_ = false;
  redrawWait = 50;
  activeCoordSystem = None;
  setCaption("UltraScan 3D Viewer");
  sa2d_ctrl_vars = tmp_sa2d_ctrl_vars;
  solutes = tmp_solutes;
  xtitle = tmp_xtitle;
  ytitle = tmp_ytitle;
  x_scale = 1;
  y_scale = 1;
  z_scale = 1;

  /*
    for (unsigned int i=0; i<solutes.size(); i++)
    {
    cout << "i: " << i << ", " << solutes[i].x << ", " << solutes[i].y << ", "
    << solutes[i].z << endl;
    }
  */
  Q3GridLayout* grid = new Q3GridLayout(frame, 0, 0);

  dataWidget = new SurfacePlot(frame);
  grid->addWidget(dataWidget, 0, 0);

  connect(coord, SIGNAL(selected(QAction*)), this,
          SLOT(pickCoordSystem(QAction*)));
  connect(plotstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickPlotStyle(QAction*)));
  connect(axescolor, SIGNAL(activated()), this, SLOT(pickAxesColor()));
  connect(backgroundcolor, SIGNAL(activated()), this, SLOT(pickBgColor()));
  connect(floorstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickFloorStyle(QAction*)));
  connect(meshcolor, SIGNAL(activated()), this, SLOT(pickMeshColor()));
  connect(numbercolor, SIGNAL(activated()), this, SLOT(pickNumberColor()));
  connect(labelcolor, SIGNAL(activated()), this, SLOT(pickLabelColor()));
  connect(titlecolor, SIGNAL(activated()), this, SLOT(pickTitleColor()));
  connect(datacolor, SIGNAL(activated()), this, SLOT(pickDataColor()));
  connect(lighting, SIGNAL(clicked()), this, SLOT(pickLighting()));
  connect(resetcolor, SIGNAL(activated()), this, SLOT(resetColors()));
  connect(numberfont, SIGNAL(activated()), this, SLOT(pickNumberFont()));
  connect(labelfont, SIGNAL(activated()), this, SLOT(pickLabelFont()));
  connect(titlefont, SIGNAL(activated()), this, SLOT(pickTitleFont()));
  connect(resetfont, SIGNAL(activated()), this, SLOT(resetFonts()));
  connect(animation, SIGNAL(toggled(bool)), this, SLOT(toggleAnimation(bool)));
  connect(dump, SIGNAL(activated()), this, SLOT(dumpImage()));
  connect(openFile, SIGNAL(activated()), this, SLOT(open()));
  connect(openMeshFile, SIGNAL(activated()), this, SLOT(openMesh()));

  // only EXCLUSIVE groups emit selected :-/
  connect(left, SIGNAL(toggled(bool)), this, SLOT(setLeftGrid(bool)));
  connect(right, SIGNAL(toggled(bool)), this, SLOT(setRightGrid(bool)));
  connect(ceil, SIGNAL(toggled(bool)), this, SLOT(setCeilGrid(bool)));
  connect(floor, SIGNAL(toggled(bool)), this, SLOT(setFloorGrid(bool)));
  connect(back, SIGNAL(toggled(bool)), this, SLOT(setBackGrid(bool)));
  connect(front, SIGNAL(toggled(bool)), this, SLOT(setFrontGrid(bool)));

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(rotate()));

  resSlider->setRange(1, 70);
  connect(resSlider, SIGNAL(valueChanged(int)), dataWidget,
          SLOT(setResolution(int)));
  connect(dataWidget, SIGNAL(resolutionChanged(int)), resSlider,
          SLOT(setValue(int)));
  resSlider->setValue(1);

  connect(offsSlider, SIGNAL(valueChanged(int)), this,
          SLOT(setPolygonOffset(int)));

  connect(normButton, SIGNAL(clicked()), this, SLOT(setStandardView()));

  QString qwtstr(" 2D Spectrum Analysis ");
  qwtstr += QString::number(QWT3D_MAJOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_MINOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_PATCH_VERSION) + " ";

  QLabel* info = new QLabel(qwtstr, statusBar());
  info->setPaletteForegroundColor(Qt::darkBlue);
  statusBar()->addWidget(info, 0, false);
  filenameWidget =
      new QLabel("                                  ", statusBar());
  statusBar()->addWidget(filenameWidget, 0, false);
  dimWidget = new QLabel("", statusBar());
  statusBar()->addWidget(dimWidget, 0, false);
  rotateLabel = new QLabel("", statusBar());
  statusBar()->addWidget(rotateLabel, 0, false);
  shiftLabel = new QLabel("", statusBar());
  statusBar()->addWidget(shiftLabel, 0, false);
  scaleLabel = new QLabel("", statusBar());
  statusBar()->addWidget(scaleLabel, 0, false);
  zoomLabel = new QLabel("", statusBar());
  statusBar()->addWidget(zoomLabel, 0, false);

  connect(dataWidget, SIGNAL(rotationChanged(double, double, double)), this,
          SLOT(showRotate(double, double, double)));
  connect(dataWidget, SIGNAL(vieportShiftChanged(double, double)), this,
          SLOT(showShift(double, double)));
  connect(dataWidget, SIGNAL(scaleChanged(double, double, double)), this,
          SLOT(showScale(double, double, double)));
  connect(dataWidget, SIGNAL(zoomChanged(double)), this,
          SLOT(showZoom(double)));

  connect(projection, SIGNAL(toggled(bool)), this,
          SLOT(toggleProjectionMode(bool)));
  connect(colorlegend, SIGNAL(toggled(bool)), this,
          SLOT(toggleColorLegend(bool)));
  connect(autoscale, SIGNAL(toggled(bool)), this, SLOT(toggleAutoScale(bool)));
  connect(shader, SIGNAL(toggled(bool)), this, SLOT(toggleShader(bool)));
  connect(mouseinput, SIGNAL(toggled(bool)), dataWidget,
          SLOT(enableMouse(bool)));
  connect(lightingswitch, SIGNAL(toggled(bool)), this,
          SLOT(enableLighting(bool)));
  connect(normals, SIGNAL(toggled(bool)), this, SLOT(showNormals(bool)));
  connect(normalsquality, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalQuality(int)));
  connect(normalslength, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalLength(int)));

  setStandardView();

  dataWidget->coordinates()->setLineSmooth(true);
  dataWidget->coordinates()->setGridLinesColor(RGBA(0.35, 0.35, 0.35, 1));
  dataWidget->enableMouse(true);
  dataWidget->setKeySpeed(15, 20, 20);

  colormappv_ = new ColorMapPreview;
  datacolordlg_ = new Q3FileDialog(this);
  lightingdlg_ = new LightingDlg(this);
  lightingdlg_->assign(dataWidget);

  QDir dir("../../data/colormaps");
  if (dir.exists("../../data/colormaps")) {
    datacolordlg_->setDir("../../data/colormaps");
  }
  datacolordlg_->setFilter("Colormap files (*.map;*.MAP)");
  datacolordlg_->setContentsPreviewEnabled(TRUE);
  datacolordlg_->setContentsPreview(colormappv_, colormappv_);
  datacolordlg_->setPreviewMode(Q3FileDialog::Contents);

  connect(datacolordlg_, SIGNAL(fileHighlighted(const QString&)), this,
          SLOT(adaptDataColors(const QString&)));
  connect(filetypeCB, SIGNAL(activated(const QString&)), this,
          SLOT(setFileType(const QString&)));

  filetypeCB->clear();

  QStringList list = IO::outputFormatList();
  filetypeCB->insertStringList(list);

  filetype_ = "PNG";
  filetypeCB->setCurrentText("PNG");

  dataWidget->setTitleFont("Arial", 14, QFont::Normal);

  grids->setEnabled(false);

  PixmapWriter* pmhandler = (PixmapWriter*)IO::outputHandler("JPEG");
  if (pmhandler) pmhandler->setQuality(70);
  VectorWriter* handler = (VectorWriter*)IO::outputHandler("PDF");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS_GZ");
  if (handler)  // with zlib support only
    handler->setTextMode(VectorWriter::TEX);

  createFunction();
}

Mesh2MainWindow::Mesh2MainWindow(bool* widget_flag, QString tmp_xtitle,
                                 QString tmp_ytitle, QString tmp_ztitle,
                                 double** data,
                                 SA2d_control_variables* tmp_sa2d_ctrl_vars,
                                 QWidget* parent, const char* name,
                                 Qt::WFlags f)
    : Mesh2MainWindowBase(parent, name, f) {
  this->widget_flag = widget_flag;
  *widget_flag = true;
  col_ = 0;
  legend_ = false;
  redrawWait = 50;
  activeCoordSystem = None;
  setCaption("UltraScan 3D Viewer");
  sa2d_ctrl_vars = tmp_sa2d_ctrl_vars;
  this->data = data;
  xtitle = tmp_xtitle;
  ytitle = tmp_ytitle;
  ztitle = tmp_ztitle;

  Q3GridLayout* grid = new Q3GridLayout(frame, 0, 0);

  dataWidget = new SurfacePlot(frame);
  grid->addWidget(dataWidget, 0, 0);

  connect(coord, SIGNAL(selected(QAction*)), this,
          SLOT(pickCoordSystem(QAction*)));
  connect(plotstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickPlotStyle(QAction*)));
  connect(axescolor, SIGNAL(activated()), this, SLOT(pickAxesColor()));
  connect(backgroundcolor, SIGNAL(activated()), this, SLOT(pickBgColor()));
  connect(floorstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickFloorStyle(QAction*)));
  connect(meshcolor, SIGNAL(activated()), this, SLOT(pickMeshColor()));
  connect(numbercolor, SIGNAL(activated()), this, SLOT(pickNumberColor()));
  connect(labelcolor, SIGNAL(activated()), this, SLOT(pickLabelColor()));
  connect(titlecolor, SIGNAL(activated()), this, SLOT(pickTitleColor()));
  connect(datacolor, SIGNAL(activated()), this, SLOT(pickDataColor()));
  connect(lighting, SIGNAL(clicked()), this, SLOT(pickLighting()));
  connect(resetcolor, SIGNAL(activated()), this, SLOT(resetColors()));
  connect(numberfont, SIGNAL(activated()), this, SLOT(pickNumberFont()));
  connect(labelfont, SIGNAL(activated()), this, SLOT(pickLabelFont()));
  connect(titlefont, SIGNAL(activated()), this, SLOT(pickTitleFont()));
  connect(resetfont, SIGNAL(activated()), this, SLOT(resetFonts()));
  connect(animation, SIGNAL(toggled(bool)), this, SLOT(toggleAnimation(bool)));
  connect(dump, SIGNAL(activated()), this, SLOT(dumpImage()));
  connect(openFile, SIGNAL(activated()), this, SLOT(open()));
  connect(openMeshFile, SIGNAL(activated()), this, SLOT(openMesh()));

  // only EXCLUSIVE groups emit selected :-/
  connect(left, SIGNAL(toggled(bool)), this, SLOT(setLeftGrid(bool)));
  connect(right, SIGNAL(toggled(bool)), this, SLOT(setRightGrid(bool)));
  connect(ceil, SIGNAL(toggled(bool)), this, SLOT(setCeilGrid(bool)));
  connect(floor, SIGNAL(toggled(bool)), this, SLOT(setFloorGrid(bool)));
  connect(back, SIGNAL(toggled(bool)), this, SLOT(setBackGrid(bool)));
  connect(front, SIGNAL(toggled(bool)), this, SLOT(setFrontGrid(bool)));

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(rotate()));

  resSlider->setRange(1, 70);
  connect(resSlider, SIGNAL(valueChanged(int)), dataWidget,
          SLOT(setResolution(int)));
  connect(dataWidget, SIGNAL(resolutionChanged(int)), resSlider,
          SLOT(setValue(int)));
  resSlider->setValue(1);

  connect(offsSlider, SIGNAL(valueChanged(int)), this,
          SLOT(setPolygonOffset(int)));

  connect(normButton, SIGNAL(clicked()), this, SLOT(setStandardView()));

  QString qwtstr(" Multiwavelength Data ");
  qwtstr += QString::number(QWT3D_MAJOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_MINOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_PATCH_VERSION) + " ";

  QLabel* info = new QLabel(qwtstr, statusBar());
  info->setPaletteForegroundColor(Qt::darkBlue);
  statusBar()->addWidget(info, 0, false);
  filenameWidget =
      new QLabel("                                  ", statusBar());
  statusBar()->addWidget(filenameWidget, 0, false);
  dimWidget = new QLabel("", statusBar());
  statusBar()->addWidget(dimWidget, 0, false);
  rotateLabel = new QLabel("", statusBar());
  statusBar()->addWidget(rotateLabel, 0, false);
  shiftLabel = new QLabel("", statusBar());
  statusBar()->addWidget(shiftLabel, 0, false);
  scaleLabel = new QLabel("", statusBar());
  statusBar()->addWidget(scaleLabel, 0, false);
  zoomLabel = new QLabel("", statusBar());
  statusBar()->addWidget(zoomLabel, 0, false);

  connect(dataWidget, SIGNAL(rotationChanged(double, double, double)), this,
          SLOT(showRotate(double, double, double)));
  connect(dataWidget, SIGNAL(vieportShiftChanged(double, double)), this,
          SLOT(showShift(double, double)));
  connect(dataWidget, SIGNAL(scaleChanged(double, double, double)), this,
          SLOT(showScale(double, double, double)));
  connect(dataWidget, SIGNAL(zoomChanged(double)), this,
          SLOT(showZoom(double)));

  connect(projection, SIGNAL(toggled(bool)), this,
          SLOT(toggleProjectionMode(bool)));
  connect(colorlegend, SIGNAL(toggled(bool)), this,
          SLOT(toggleColorLegend(bool)));
  connect(autoscale, SIGNAL(toggled(bool)), this, SLOT(toggleAutoScale(bool)));
  connect(shader, SIGNAL(toggled(bool)), this, SLOT(toggleShader(bool)));
  connect(mouseinput, SIGNAL(toggled(bool)), dataWidget,
          SLOT(enableMouse(bool)));
  connect(lightingswitch, SIGNAL(toggled(bool)), this,
          SLOT(enableLighting(bool)));
  connect(normals, SIGNAL(toggled(bool)), this, SLOT(showNormals(bool)));
  connect(normalsquality, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalQuality(int)));
  connect(normalslength, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalLength(int)));

  setStandardView();

  dataWidget->coordinates()->setLineSmooth(true);
  dataWidget->coordinates()->setGridLinesColor(RGBA(0.35, 0.35, 0.35, 1));
  dataWidget->enableMouse(true);
  dataWidget->setKeySpeed(15, 20, 20);

  colormappv_ = new ColorMapPreview;
  datacolordlg_ = new Q3FileDialog(this);
  lightingdlg_ = new LightingDlg(this);
  lightingdlg_->assign(dataWidget);

  QDir dir("../../data/colormaps");
  if (dir.exists("../../data/colormaps")) {
    datacolordlg_->setDir("../../data/colormaps");
  }
  datacolordlg_->setFilter("Colormap files (*.map;*.MAP)");
  datacolordlg_->setContentsPreviewEnabled(TRUE);
  datacolordlg_->setContentsPreview(colormappv_, colormappv_);
  datacolordlg_->setPreviewMode(Q3FileDialog::Contents);

  connect(datacolordlg_, SIGNAL(fileHighlighted(const QString&)), this,
          SLOT(adaptDataColors(const QString&)));
  connect(filetypeCB, SIGNAL(activated(const QString&)), this,
          SLOT(setFileType(const QString&)));

  filetypeCB->clear();

  QStringList list = IO::outputFormatList();
  filetypeCB->insertStringList(list);

  filetype_ = "PNG";
  filetypeCB->setCurrentText("PNG");

  dataWidget->setTitleFont("Arial", 14, QFont::Normal);

  grids->setEnabled(false);

  PixmapWriter* pmhandler = (PixmapWriter*)IO::outputHandler("JPEG");
  if (pmhandler) pmhandler->setQuality(70);
  VectorWriter* handler = (VectorWriter*)IO::outputHandler("PDF");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS_GZ");
  if (handler)  // with zlib support only
    handler->setTextMode(VectorWriter::TEX);

  dataWidget->makeCurrent();

  dataWidget->legend()->setScale(LINEARSCALE);
  for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
    dataWidget->coordinates()->axes[i].setMajors(7);
    dataWidget->coordinates()->axes[i].setMinors(5);
  }

  double a = dataWidget->facets().first;
  double b = dataWidget->facets().second;

  dimWidget->setText(QString("Cells ") + QString::number(a * b) + " (" +
                     QString::number(a) + "x" + QString::number(b) + ")");

  updateColorLegend(7, 5);

  dataWidget->loadFromData(data, (*sa2d_ctrl_vars).meshx,
                           (*sa2d_ctrl_vars).meshy, (*sa2d_ctrl_vars).miny,
                           (*sa2d_ctrl_vars).maxy, (*sa2d_ctrl_vars).minx,
                           (*sa2d_ctrl_vars).maxx);
  if ((*sa2d_ctrl_vars).xscaling != 0) {
    x_scale = (*sa2d_ctrl_vars).xscaling;
  } else {
    x_scale = (*sa2d_ctrl_vars).maxx / (*sa2d_ctrl_vars).maxy;
  }
  if ((*sa2d_ctrl_vars).yscaling != 0) {
    y_scale = (*sa2d_ctrl_vars).yscaling;
  } else {
    y_scale = 1;
  }
  z_scale = (*sa2d_ctrl_vars).zscaling;

  dataWidget->coordinates()->axes[X1].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X2].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X3].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X4].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[Y1].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y2].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y3].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y4].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Z1].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z2].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z3].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z4].setLabelString(QString(ztitle));

  //  dataWidget->coordinates()->axes[X1].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X2].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X3].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X4].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  dataWidget->coordinates()->axes[Y1].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y2].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y3].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y4].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Z1].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z2].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z3].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z4].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  setStandardView();

  pickCoordSystem(activeCoordSystem);
  dataWidget->setTitle("Multiwavelength Data");
  //   dataWidget->setMesh((*sa2d_ctrl_vars).meshx, (*sa2d_ctrl_vars).meshy);
  //   dataWidget->setDomain((*sa2d_ctrl_vars).minx, (*sa2d_ctrl_vars).maxx,
  //   (*sa2d_ctrl_vars).miny, (*sa2d_ctrl_vars).maxy);
  //   dataWidget->setMinZ(-0.1);
}

void Mesh2MainWindow::setParameters(QString xtitle, QString ytitle,
                                    QString ztitle, double** data,
                                    SA2d_control_variables* sa2d_ctrl_vars) {
  this->sa2d_ctrl_vars = sa2d_ctrl_vars;
  this->solutes = solutes;
  this->xtitle = xtitle;
  this->ytitle = ytitle;
  this->ztitle = ztitle;
  dataWidget->setTitle("Multiwavelength Data");
  //   dataWidget->setMesh((*sa2d_ctrl_vars).meshx, (*sa2d_ctrl_vars).meshy);
  //   dataWidget->setDomain((*sa2d_ctrl_vars).minx, (*sa2d_ctrl_vars).maxx,
  //   (*sa2d_ctrl_vars).miny, (*sa2d_ctrl_vars).maxy);
  //   dataWidget->setMinZ(-0.1);
  dataWidget->loadFromData(data, (*sa2d_ctrl_vars).meshx,
                           (*sa2d_ctrl_vars).meshy, (*sa2d_ctrl_vars).miny,
                           (*sa2d_ctrl_vars).maxy, (*sa2d_ctrl_vars).minx,
                           (*sa2d_ctrl_vars).maxx);
  if ((*sa2d_ctrl_vars).xscaling != 0) {
    x_scale = (*sa2d_ctrl_vars).xscaling;
  } else {
    x_scale = (*sa2d_ctrl_vars).maxx / (*sa2d_ctrl_vars).maxy;
  }
  if ((*sa2d_ctrl_vars).yscaling != 0) {
    y_scale = (*sa2d_ctrl_vars).yscaling;
  } else {
    y_scale = 1;
  }
  z_scale = (*sa2d_ctrl_vars).zscaling;

  dataWidget->coordinates()->axes[X1].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X2].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X3].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X4].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[Y1].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y2].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y3].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y4].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Z1].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z2].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z3].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z4].setLabelString(QString(ztitle));

  //  dataWidget->coordinates()->axes[X1].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X2].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X3].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X4].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  dataWidget->coordinates()->axes[Y1].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y2].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y3].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y4].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Z1].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z2].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z3].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z4].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  setStandardView();

  pickCoordSystem(activeCoordSystem);
  dataWidget->setTitle("Multiwavelength Data");
}

void Mesh2MainWindow::setParameters(QString title_3d, QString xtitle,
                                    QString ytitle, QString ztitle,
                                    double** data,
                                    SA2d_control_variables* sa2d_ctrl_vars) {
  this->sa2d_ctrl_vars = sa2d_ctrl_vars;
  this->xtitle = xtitle;
  this->ytitle = ytitle;
  this->ztitle = ztitle;
  dataWidget->setTitle(title_3d);
  //   dataWidget->setMesh((*sa2d_ctrl_vars).meshx, (*sa2d_ctrl_vars).meshy);
  //   dataWidget->setDomain((*sa2d_ctrl_vars).minx, (*sa2d_ctrl_vars).maxx,
  //   (*sa2d_ctrl_vars).miny, (*sa2d_ctrl_vars).maxy);
  //   dataWidget->setMinZ(-0.1);
  dataWidget->loadFromData(data, (*sa2d_ctrl_vars).meshx,
                           (*sa2d_ctrl_vars).meshy, (*sa2d_ctrl_vars).miny,
                           (*sa2d_ctrl_vars).maxy, (*sa2d_ctrl_vars).minx,
                           (*sa2d_ctrl_vars).maxx);
  if ((*sa2d_ctrl_vars).xscaling != 0) {
    x_scale = (*sa2d_ctrl_vars).xscaling;
  } else {
    x_scale = (*sa2d_ctrl_vars).maxx / (*sa2d_ctrl_vars).maxy;
  }
  if ((*sa2d_ctrl_vars).yscaling != 0) {
    y_scale = (*sa2d_ctrl_vars).yscaling;
  } else {
    y_scale = 1;
  }
  z_scale = (*sa2d_ctrl_vars).zscaling;

  dataWidget->coordinates()->axes[X1].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X2].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X3].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X4].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[Y1].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y2].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y3].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y4].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Z1].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z2].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z3].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z4].setLabelString(QString(ztitle));

  //  dataWidget->coordinates()->axes[X1].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X2].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X3].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X4].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  dataWidget->coordinates()->axes[Y1].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y2].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y3].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y4].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Z1].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z2].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z3].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z4].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  setStandardView();

  pickCoordSystem(activeCoordSystem);
  dataWidget->setTitle(title_3d);
}

Mesh2MainWindow::Mesh2MainWindow(bool* widget_flag, QString title_3d,
                                 QString tmp_xtitle, QString tmp_ytitle,
                                 QString tmp_ztitle, double** data,
                                 SA2d_control_variables* tmp_sa2d_ctrl_vars,
                                 QWidget* parent, const char* name,
                                 Qt::WFlags f)
    : Mesh2MainWindowBase(parent, name, f) {
  this->widget_flag = widget_flag;
  *widget_flag = true;
  col_ = 0;
  legend_ = false;
  redrawWait = 50;
  activeCoordSystem = None;
  setCaption(title_3d);
  sa2d_ctrl_vars = tmp_sa2d_ctrl_vars;
  this->data = data;
  xtitle = tmp_xtitle;
  ytitle = tmp_ytitle;
  ztitle = tmp_ztitle;

  Q3GridLayout* grid = new Q3GridLayout(frame, 0, 0);

  dataWidget = new SurfacePlot(frame);
  grid->addWidget(dataWidget, 0, 0);

  connect(coord, SIGNAL(selected(QAction*)), this,
          SLOT(pickCoordSystem(QAction*)));
  connect(plotstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickPlotStyle(QAction*)));
  connect(axescolor, SIGNAL(activated()), this, SLOT(pickAxesColor()));
  connect(backgroundcolor, SIGNAL(activated()), this, SLOT(pickBgColor()));
  connect(floorstyle, SIGNAL(selected(QAction*)), this,
          SLOT(pickFloorStyle(QAction*)));
  connect(meshcolor, SIGNAL(activated()), this, SLOT(pickMeshColor()));
  connect(numbercolor, SIGNAL(activated()), this, SLOT(pickNumberColor()));
  connect(labelcolor, SIGNAL(activated()), this, SLOT(pickLabelColor()));
  connect(titlecolor, SIGNAL(activated()), this, SLOT(pickTitleColor()));
  connect(datacolor, SIGNAL(activated()), this, SLOT(pickDataColor()));
  connect(lighting, SIGNAL(clicked()), this, SLOT(pickLighting()));
  connect(resetcolor, SIGNAL(activated()), this, SLOT(resetColors()));
  connect(numberfont, SIGNAL(activated()), this, SLOT(pickNumberFont()));
  connect(labelfont, SIGNAL(activated()), this, SLOT(pickLabelFont()));
  connect(titlefont, SIGNAL(activated()), this, SLOT(pickTitleFont()));
  connect(resetfont, SIGNAL(activated()), this, SLOT(resetFonts()));
  connect(animation, SIGNAL(toggled(bool)), this, SLOT(toggleAnimation(bool)));
  connect(dump, SIGNAL(activated()), this, SLOT(dumpImage()));
  connect(openFile, SIGNAL(activated()), this, SLOT(open()));
  connect(openMeshFile, SIGNAL(activated()), this, SLOT(openMesh()));

  // only EXCLUSIVE groups emit selected :-/
  connect(left, SIGNAL(toggled(bool)), this, SLOT(setLeftGrid(bool)));
  connect(right, SIGNAL(toggled(bool)), this, SLOT(setRightGrid(bool)));
  connect(ceil, SIGNAL(toggled(bool)), this, SLOT(setCeilGrid(bool)));
  connect(floor, SIGNAL(toggled(bool)), this, SLOT(setFloorGrid(bool)));
  connect(back, SIGNAL(toggled(bool)), this, SLOT(setBackGrid(bool)));
  connect(front, SIGNAL(toggled(bool)), this, SLOT(setFrontGrid(bool)));

  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(rotate()));

  resSlider->setRange(1, 70);
  connect(resSlider, SIGNAL(valueChanged(int)), dataWidget,
          SLOT(setResolution(int)));
  connect(dataWidget, SIGNAL(resolutionChanged(int)), resSlider,
          SLOT(setValue(int)));
  resSlider->setValue(1);

  connect(offsSlider, SIGNAL(valueChanged(int)), this,
          SLOT(setPolygonOffset(int)));

  connect(normButton, SIGNAL(clicked()), this, SLOT(setStandardView()));

  QString qwtstr(title_3d + " ");
  qwtstr += QString::number(QWT3D_MAJOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_MINOR_VERSION) + ".";
  qwtstr += QString::number(QWT3D_PATCH_VERSION) + " ";

  QLabel* info = new QLabel(qwtstr, statusBar());
  info->setPaletteForegroundColor(Qt::darkBlue);
  statusBar()->addWidget(info, 0, false);
  filenameWidget =
      new QLabel("                                  ", statusBar());
  statusBar()->addWidget(filenameWidget, 0, false);
  dimWidget = new QLabel("", statusBar());
  statusBar()->addWidget(dimWidget, 0, false);
  rotateLabel = new QLabel("", statusBar());
  statusBar()->addWidget(rotateLabel, 0, false);
  shiftLabel = new QLabel("", statusBar());
  statusBar()->addWidget(shiftLabel, 0, false);
  scaleLabel = new QLabel("", statusBar());
  statusBar()->addWidget(scaleLabel, 0, false);
  zoomLabel = new QLabel("", statusBar());
  statusBar()->addWidget(zoomLabel, 0, false);

  connect(dataWidget, SIGNAL(rotationChanged(double, double, double)), this,
          SLOT(showRotate(double, double, double)));
  connect(dataWidget, SIGNAL(vieportShiftChanged(double, double)), this,
          SLOT(showShift(double, double)));
  connect(dataWidget, SIGNAL(scaleChanged(double, double, double)), this,
          SLOT(showScale(double, double, double)));
  connect(dataWidget, SIGNAL(zoomChanged(double)), this,
          SLOT(showZoom(double)));

  connect(projection, SIGNAL(toggled(bool)), this,
          SLOT(toggleProjectionMode(bool)));
  connect(colorlegend, SIGNAL(toggled(bool)), this,
          SLOT(toggleColorLegend(bool)));
  connect(autoscale, SIGNAL(toggled(bool)), this, SLOT(toggleAutoScale(bool)));
  connect(shader, SIGNAL(toggled(bool)), this, SLOT(toggleShader(bool)));
  connect(mouseinput, SIGNAL(toggled(bool)), dataWidget,
          SLOT(enableMouse(bool)));
  connect(lightingswitch, SIGNAL(toggled(bool)), this,
          SLOT(enableLighting(bool)));
  connect(normals, SIGNAL(toggled(bool)), this, SLOT(showNormals(bool)));
  connect(normalsquality, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalQuality(int)));
  connect(normalslength, SIGNAL(valueChanged(int)), this,
          SLOT(setNormalLength(int)));

  setStandardView();

  dataWidget->coordinates()->setLineSmooth(true);
  dataWidget->coordinates()->setGridLinesColor(RGBA(0.35, 0.35, 0.35, 1));
  dataWidget->enableMouse(true);
  dataWidget->setKeySpeed(15, 20, 20);

  colormappv_ = new ColorMapPreview;
  datacolordlg_ = new Q3FileDialog(this);
  lightingdlg_ = new LightingDlg(this);
  lightingdlg_->assign(dataWidget);

  QDir dir("../../data/colormaps");
  if (dir.exists("../../data/colormaps")) {
    datacolordlg_->setDir("../../data/colormaps");
  }
  datacolordlg_->setFilter("Colormap files (*.map;*.MAP)");
  datacolordlg_->setContentsPreviewEnabled(TRUE);
  datacolordlg_->setContentsPreview(colormappv_, colormappv_);
  datacolordlg_->setPreviewMode(Q3FileDialog::Contents);

  connect(datacolordlg_, SIGNAL(fileHighlighted(const QString&)), this,
          SLOT(adaptDataColors(const QString&)));
  connect(filetypeCB, SIGNAL(activated(const QString&)), this,
          SLOT(setFileType(const QString&)));

  filetypeCB->clear();

  QStringList list = IO::outputFormatList();
  filetypeCB->insertStringList(list);

  filetype_ = "PNG";
  filetypeCB->setCurrentText("PNG");

  dataWidget->setTitleFont("Arial", 14, QFont::Normal);

  grids->setEnabled(false);

  PixmapWriter* pmhandler = (PixmapWriter*)IO::outputHandler("JPEG");
  if (pmhandler) pmhandler->setQuality(70);
  VectorWriter* handler = (VectorWriter*)IO::outputHandler("PDF");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS");
  handler->setTextMode(VectorWriter::TEX);
  handler = (VectorWriter*)IO::outputHandler("EPS_GZ");
  if (handler)  // with zlib support only
    handler->setTextMode(VectorWriter::TEX);

  dataWidget->makeCurrent();

  dataWidget->legend()->setScale(LINEARSCALE);
  for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
    dataWidget->coordinates()->axes[i].setMajors(7);
    dataWidget->coordinates()->axes[i].setMinors(5);
  }

  double a = dataWidget->facets().first;
  double b = dataWidget->facets().second;

  dimWidget->setText(QString("Cells ") + QString::number(a * b) + " (" +
                     QString::number(a) + "x" + QString::number(b) + ")");

  updateColorLegend(7, 5);

  dataWidget->loadFromData(data, (*sa2d_ctrl_vars).meshx,
                           (*sa2d_ctrl_vars).meshy, (*sa2d_ctrl_vars).miny,
                           (*sa2d_ctrl_vars).maxy, (*sa2d_ctrl_vars).minx,
                           (*sa2d_ctrl_vars).maxx);

  if ((*sa2d_ctrl_vars).xscaling != 0) {
    x_scale = (*sa2d_ctrl_vars).xscaling;
  } else {
    x_scale = (*sa2d_ctrl_vars).maxx / (*sa2d_ctrl_vars).maxy;
  }
  if ((*sa2d_ctrl_vars).yscaling != 0) {
    y_scale = (*sa2d_ctrl_vars).yscaling;
  } else {
    y_scale = 1;
  }

  z_scale = (*sa2d_ctrl_vars).zscaling;

  dataWidget->coordinates()->axes[X1].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X2].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X3].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X4].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[Y1].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y2].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y3].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y4].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Z1].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z2].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z3].setLabelString(QString(ztitle));
  dataWidget->coordinates()->axes[Z4].setLabelString(QString(ztitle));

  //  dataWidget->coordinates()->axes[X1].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X2].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X3].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  //  dataWidget->coordinates()->axes[X4].setTicLength(1/(10*x_scale),
  //  1/(20*x_scale));
  dataWidget->coordinates()->axes[Y1].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y2].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y3].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Y4].setTicLength(1 / (15 * y_scale),
                                                   1 / (30 * y_scale));
  dataWidget->coordinates()->axes[Z1].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z2].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z3].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  dataWidget->coordinates()->axes[Z4].setTicLength(1 / (2 * z_scale),
                                                   1 / (5 * z_scale));
  setStandardView();

  pickCoordSystem(activeCoordSystem);
  dataWidget->setTitle(title_3d);
  //   dataWidget->setMesh((*sa2d_ctrl_vars).meshx, (*sa2d_ctrl_vars).meshy);
  //   dataWidget->setDomain((*sa2d_ctrl_vars).minx, (*sa2d_ctrl_vars).maxx,
  //   (*sa2d_ctrl_vars).miny, (*sa2d_ctrl_vars).maxy);
  //   dataWidget->setMinZ(-0.1);
}

void Mesh2MainWindow::setParameters(QString xtitle, QString ytitle,
                                    vector<struct element_3D> solutes,
                                    SA2d_control_variables* sa2d_ctrl_vars) {
  this->sa2d_ctrl_vars = sa2d_ctrl_vars;
  this->solutes = solutes;
  this->xtitle = xtitle;
  this->ytitle = ytitle;
  createFunction();
}

void Mesh2MainWindow::open() {
  QString s(Q3FileDialog::getOpenFileName(
      "../../data", "GridData Files (*.mes *.MES)", this));

  if (s.isEmpty() || !dataWidget) return;

  QFileInfo fi(s);
  QString ext = fi.extension(false);  // ext = "gz"

  QToolTip::add(filenameWidget, s);
  filenameWidget->setText(fi.fileName());

  if (IO::load(dataWidget, s, ext)) {
    double a = dataWidget->facets().first;
    double b = dataWidget->facets().second;

    dimWidget->setText(QString("Cells ") + QString::number(a * b) + " (" +
                       QString::number(a) + "x" + QString::number(b) + ")");

    dataWidget->setResolution(3);
  }

  for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
    dataWidget->coordinates()->axes[i].setMajors(4);
    dataWidget->coordinates()->axes[i].setMinors(5);
    dataWidget->coordinates()->axes[i].setLabelString("");
  }

  updateColorLegend(4, 5);
  pickCoordSystem(activeCoordSystem);
  dataWidget->showColorLegend(legend_);
}

void Mesh2MainWindow::closeEvent(QCloseEvent* e) {
  *widget_flag = false;
  e->accept();
}

void Mesh2MainWindow::createFunction() {
  C_s_k* c_s_k = (C_s_k*)0;
  dataWidget->makeCurrent();

  dataWidget->legend()->setScale(LINEARSCALE);
  for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
    dataWidget->coordinates()->axes[i].setMajors(7);
    dataWidget->coordinates()->axes[i].setMinors(5);
  }

  c_s_k = new C_s_k(sa2d_ctrl_vars, solutes, *dataWidget);
  c_s_k->create();

  double a = dataWidget->facets().first;
  double b = dataWidget->facets().second;

  dimWidget->setText(QString("Cells ") + QString::number(a * b) + " (" +
                     QString::number(a) + "x" + QString::number(b) + ")");

  updateColorLegend(7, 5);

  dataWidget->coordinates()->axes[X1].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X2].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X3].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[X4].setLabelString(QString(xtitle));
  dataWidget->coordinates()->axes[Y1].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y2].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y3].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Y4].setLabelString(QString(ytitle));
  dataWidget->coordinates()->axes[Z1].setLabelString(QString("Concentration"));
  dataWidget->coordinates()->axes[Z2].setLabelString(QString("Concentration"));
  dataWidget->coordinates()->axes[Z3].setLabelString(QString("Concentration"));
  dataWidget->coordinates()->axes[Z4].setLabelString(QString("Concentration"));
  pickCoordSystem(activeCoordSystem);
  dataWidget->setTitle("2-D Spectrum Analysis");
}

void Mesh2MainWindow::pickCoordSystem(QAction* action) {
  if (!action || !dataWidget) return;

  activeCoordSystem = action;

  dataWidget->setTitle(
      "QwtPlot3D (Use Ctrl-Alt-Shift-LeftBtn-Wheel or keyboard)");

  if (!dataWidget->hasData()) {
    double l = 0.6;
    dataWidget->createCoordinateSystem(Triple(-l, -l, -l), Triple(l, l, l));
    for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
      dataWidget->coordinates()->axes[i].setMajors(4);
      dataWidget->coordinates()->axes[i].setMinors(5);
    }
  }

  if (action == Box || action == Frame) {
    if (action == Box) dataWidget->setCoordinateStyle(BOX);
    if (action == Frame) dataWidget->setCoordinateStyle(FRAME);
    grids->setEnabled(true);
  } else if (action == None) {
    dataWidget->setTitle(
        "QwtPlot3D (Use Ctrl-Alt-Shift-LeftBtn-Wheel or keyboard)");
    dataWidget->setCoordinateStyle(NOCOORD);
    grids->setEnabled(false);
  }
}

void Mesh2MainWindow::pickPlotStyle(QAction* action) {
  if (!action || !dataWidget) return;

  if (action == polygon) {
    dataWidget->setPlotStyle(FILLED);
  } else if (action == filledmesh) {
    dataWidget->setPlotStyle(FILLEDMESH);
  } else if (action == wireframe) {
    dataWidget->setPlotStyle(WIREFRAME);
  } else if (action == hiddenline) {
    dataWidget->setPlotStyle(HIDDENLINE);
  } else if (action == pointstyle) {
    dataWidget->setPlotStyle(Qwt3D::POINTS);
    //    Cone d(len,32);
    //    CrossHair d(0.003,0,true,false);
    //    dataWidget->setPlotStyle(d);
  } else {
    dataWidget->setPlotStyle(NOPLOT);
  }
  dataWidget->updateData();
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickFloorStyle(QAction* action) {
  if (!action || !dataWidget) return;

  if (action == floordata) {
    dataWidget->setFloorStyle(FLOORDATA);
  } else if (action == flooriso) {
    dataWidget->setFloorStyle(FLOORISO);
  } else {
    dataWidget->setFloorStyle(NOFLOOR);
  }

  dataWidget->updateData();
  dataWidget->updateGL();
}

void Mesh2MainWindow::setLeftGrid(bool b) { setGrid(Qwt3D::LEFT, b); }
void Mesh2MainWindow::setRightGrid(bool b) { setGrid(Qwt3D::RIGHT, b); }
void Mesh2MainWindow::setCeilGrid(bool b) { setGrid(Qwt3D::CEIL, b); }
void Mesh2MainWindow::setFloorGrid(bool b) { setGrid(Qwt3D::FLOOR, b); }
void Mesh2MainWindow::setFrontGrid(bool b) { setGrid(Qwt3D::FRONT, b); }
void Mesh2MainWindow::setBackGrid(bool b) { setGrid(Qwt3D::BACK, b); }

void Mesh2MainWindow::setGrid(Qwt3D::SIDE s, bool b) {
  if (!dataWidget) return;

  int sum = dataWidget->coordinates()->grids();

  if (b)
    sum |= s;
  else
    sum &= ~s;

  dataWidget->coordinates()->setGridLines(sum != Qwt3D::NOSIDEGRID,
                                          sum != Qwt3D::NOSIDEGRID, sum);
  dataWidget->updateGL();
}

void Mesh2MainWindow::resetColors() {
  if (!dataWidget) return;

  const RGBA axc = RGBA(0, 0, 0, 1);
  const RGBA bgc = RGBA(1.0, 1.0, 1.0, 1.0);
  const RGBA msc = RGBA(0, 0, 0, 1);
  const RGBA nuc = RGBA(0, 0, 0, 1);
  const RGBA lbc = RGBA(0, 0, 0, 1);
  const RGBA tc = RGBA(0, 0, 0, 1);

  dataWidget->coordinates()->setAxesColor(axc);
  dataWidget->setBackgroundColor(bgc);
  dataWidget->setMeshColor(msc);
  dataWidget->updateData();
  dataWidget->coordinates()->setNumberColor(nuc);
  dataWidget->coordinates()->setLabelColor(lbc);
  dataWidget->setTitleColor(tc);

  col_ = new StandardColor(dataWidget);
  dataWidget->setDataColor(col_);
  dataWidget->updateData();
  dataWidget->updateNormals();
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickAxesColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->coordinates()->setAxesColor(rgb);
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickBgColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->setBackgroundColor(rgb);
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickMeshColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->setMeshColor(rgb);
  dataWidget->updateData();
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickNumberColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->coordinates()->setNumberColor(rgb);
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickLabelColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->coordinates()->setLabelColor(rgb);
  dataWidget->updateGL();
}
void Mesh2MainWindow::pickTitleColor() {
  QColor c = QColorDialog::getColor(Qt::white, this);
  if (!c.isValid()) return;
  RGBA rgb = Qt2GL(c);
  dataWidget->setTitleColor(rgb);
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickDataColor() { datacolordlg_->show(); }

void Mesh2MainWindow::pickLighting() { lightingdlg_->show(); }

void Mesh2MainWindow::adaptDataColors(const QString& fileName) {
  ColorVector cv;

  if (!openColorMap(cv, fileName)) return;

  col_ = new StandardColor(dataWidget);
  col_->setColorVector(cv);

  dataWidget->setDataColor(col_);
  dataWidget->updateData();
  dataWidget->updateNormals();
  dataWidget->showColorLegend(legend_);
  dataWidget->updateGL();
}

void Mesh2MainWindow::pickNumberFont() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, this);
  if (!ok) {
    return;
  }
  dataWidget->coordinates()->setNumberFont(font);
  dataWidget->updateGL();
}
void Mesh2MainWindow::pickLabelFont() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, this);
  if (!ok) {
    return;
  }
  dataWidget->coordinates()->setLabelFont(font);
  dataWidget->updateGL();
}
void Mesh2MainWindow::pickTitleFont() {
  bool ok;
  QFont font = QFontDialog::getFont(&ok, this);
  if (!ok) {
    return;
  }
  dataWidget->setTitleFont(font.family(), font.pointSize(), font.weight(),
                           font.italic());
}

void Mesh2MainWindow::resetFonts() {
  dataWidget->coordinates()->setNumberFont(QFont("Courier", 12));
  dataWidget->coordinates()->setLabelFont(QFont("Courier", 14, QFont::Bold));
  dataWidget->setTitleFont("Arial", 14, QFont::Normal);
  dataWidget->updateGL();
}

void Mesh2MainWindow::setStandardView() {
  dataWidget->setRotation(30, 0, 15);
  dataWidget->setViewportShift(0.05, 0);
  dataWidget->setScale(x_scale, y_scale, z_scale);
  dataWidget->setZoom(1);
}

void Mesh2MainWindow::dumpImage() {
  static int counter = 0;
  if (!dataWidget) return;
  QString name;

  name = QString("dump_") + QString::number(counter++) + ".";

  if (filetype_ == "PS_GZ")
    name += "ps.gz";
  else if (filetype_ == "EPS_GZ")
    name += "eps.gz";
  else
    name += filetype_;

  IO::save(dataWidget, name.lower(), filetype_);
  // dataWidget->savePixmap(name.lower(), filetype_);
}

void Mesh2MainWindow::dumpImage(QString filename, QString imagetype) {
  if (!dataWidget) return;
  IO::save(dataWidget, filename, imagetype);
}

/*!
  Turns animation on or off
*/
void Mesh2MainWindow::toggleAnimation(bool val) {
  if (val) {
    timer->start(redrawWait, false);  // Wait this many msecs before redraw
  } else {
    timer->stop();
  }
}

void Mesh2MainWindow::rotate() {
  if (dataWidget) {
    dataWidget->setRotation(int(dataWidget->xRotation() + 1) % 360,
                            int(dataWidget->yRotation() + 1) % 360,
                            int(dataWidget->zRotation() + 1) % 360);
  }
}

void Mesh2MainWindow::toggleProjectionMode(bool val) {
  dataWidget->setOrtho(val);
}

void Mesh2MainWindow::toggleColorLegend(bool val) {
  legend_ = val;
  dataWidget->showColorLegend(val);
}

void Mesh2MainWindow::toggleAutoScale(bool val) {
  dataWidget->coordinates()->setAutoScale(val);
  dataWidget->updateGL();
}

void Mesh2MainWindow::toggleShader(bool val) {
  if (val)
    dataWidget->setShading(GOURAUD);
  else
    dataWidget->setShading(FLAT);
}

void Mesh2MainWindow::setPolygonOffset(int val) {
  dataWidget->setPolygonOffset(val / 10.0);
  dataWidget->updateData();
  dataWidget->updateGL();
}

void Mesh2MainWindow::showRotate(double x, double y, double z) {
  rotateLabel->setText(" Angles (" + QString::number(x, 'g', 3) + " ," +
                       QString::number(y, 'g', 3) + " ," +
                       QString::number(z, 'g', 3) + ")");
}
void Mesh2MainWindow::showShift(double x, double y) {
  shiftLabel->setText(" Shifts (" + QString::number(x, 'g', 3) + " ," +
                      QString::number(y, 'g', 3) + " )");
}
void Mesh2MainWindow::showScale(double x, double y, double z) {
  scaleLabel->setText(" Scales (" + QString::number(x, 'g', 3) + " ," +
                      QString::number(y, 'g', 3) + " ," +
                      QString::number(z, 'g', 3) + ")");
}
void Mesh2MainWindow::showZoom(double z) {
  zoomLabel->setText(" Zoom " + QString::number(z, 'g', 3));
}

void Mesh2MainWindow::openMesh() {
  QString data(
      Q3FileDialog::getOpenFileName("../../data", "nodes (*.nod)", this));
  QString edges(Q3FileDialog::getOpenFileName("../../data",
                                              "connectivities (*.cel)", this));

  if (data.isEmpty() || edges.isEmpty() || !dataWidget) return;

  TripleField vdata;
  CellField vpoly;

  readNodes(vdata, data, NodeFilter());
  readConnections(vpoly, edges, CellFilter());

  dataWidget->loadFromData(vdata, vpoly);

  dimWidget->setText(QString("Cells ") +
                     QString::number(dataWidget->facets().first));

  for (unsigned i = 0; i != dataWidget->coordinates()->axes.size(); ++i) {
    dataWidget->coordinates()->axes[i].setMajors(4);
    dataWidget->coordinates()->axes[i].setMinors(5);
    dataWidget->coordinates()->axes[i].setLabelString(QString(""));
  }

  updateColorLegend(4, 5);
  pickCoordSystem(activeCoordSystem);
}

void Mesh2MainWindow::showNormals(bool val) {
  dataWidget->showNormals(val);
  dataWidget->updateNormals();
  dataWidget->updateGL();
}

void Mesh2MainWindow::setNormalLength(int val) {
  dataWidget->setNormalLength(val / 400.);
  dataWidget->updateNormals();
  dataWidget->updateGL();
}

void Mesh2MainWindow::setNormalQuality(int val) {
  dataWidget->setNormalQuality(val);
  dataWidget->updateNormals();
  dataWidget->updateGL();
}

bool Mesh2MainWindow::openColorMap(ColorVector& cv, QString fname) {
  ifstream file((const char*)fname.local8Bit());

  if (!file) return false;

  RGBA rgb;
  cv.clear();

  while (file) {
    file >> rgb.r >> rgb.g >> rgb.b;
    file.ignore(1000, '\n');
    if (!file.good())
      break;
    else {
      rgb.a = 1;
      rgb.r /= 255;
      rgb.g /= 255;
      rgb.b /= 255;
      cv.push_back(rgb);
    }
  }

  return true;
}

void Mesh2MainWindow::updateColorLegend(int majors, int minors) {
  dataWidget->legend()->setMajors(majors);
  dataWidget->legend()->setMinors(minors);
  double start, stop;
  dataWidget->coordinates()->axes[Z1].limits(start, stop);
  dataWidget->legend()->setLimits(start, stop);
}

void Mesh2MainWindow::setFileType(QString const& name) { filetype_ = name; }

void Mesh2MainWindow::enableLighting(bool val) {
  dataWidget->enableLighting(val);
  dataWidget->illuminate(0);
  dataWidget->updateGL();
}
