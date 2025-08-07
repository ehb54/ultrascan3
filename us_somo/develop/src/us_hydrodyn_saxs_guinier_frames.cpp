#include "../include/us_hydrodyn_saxs_guinier_frames.h"

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
#include "../include/us_saxs_util.h"
// Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

US_Hydrodyn_Saxs_Guinier_Frames::US_Hydrodyn_Saxs_Guinier_Frames(
    void *us_hydrodyn, map<QString, QString> parameters,
    map<QString, vector<vector<double> > > plots, QWidget *p, const char *)
    : QFrame(p) {
  this->us_hydrodyn = us_hydrodyn;
  this->parameters = parameters;
  this->plots = plots;

  USglobal = new US_Config();
  setPalette(PALET_FRAME);
  setWindowTitle(us_tr("US-SOMO: SAS Guinier frames"));

  plot_zoomer = (ScrollZoomer *)0;

  setupGUI();

  global_Xpos += 30;
  global_Ypos += 30;

  setGeometry(global_Xpos, global_Ypos, 0, 0);
  update_plot();
}

US_Hydrodyn_Saxs_Guinier_Frames::~US_Hydrodyn_Saxs_Guinier_Frames() {}

void US_Hydrodyn_Saxs_Guinier_Frames::setupGUI() {
  // fix up missing parameters
  {
    QStringList fixup;
    fixup << "title" << "unknown"
          << "xbottom" << "Time [a.u.]"
          << "yleft" << "unknown";

    for (int i = 0; i < (int)fixup.size(); i += 2) {
      if (!parameters.count(fixup[i])) {
        parameters[fixup[i]] = fixup[i + 1];
      }
    }
  }

  int minHeight1 = 30;

  // lbl_title =  new QLabel      ( us_tr( "US-SOMO: SAS Guinier residuals" ),
  // this ); lbl_title -> setFrameStyle   ( QFrame::WinPanel | QFrame::Raised );
  // lbl_title -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
  // lbl_title -> setMinimumHeight( minHeight1 );
  // lbl_title -> setPalette      ( PALET_FRAME );
  // lbl_title -> setFont         ( QFont( USglobal->config_list.fontFamily,
  // USglobal->config_list.fontSize + 1, QFont::Bold ) );

  // lbl_credits_1 =  new QLabel      ( "Cite: US-SOMO", this );
  // lbl_credits_1 -> setAlignment    ( Qt::AlignCenter | Qt::AlignVCenter );
  // lbl_credits_1 -> setMinimumHeight( minHeight1 );
  // lbl_credits_1 -> setPalette      ( PALET_LABEL );
  // lbl_credits_1 -> setFont         ( QFont( USglobal->config_list.fontFamily,
  // USglobal->config_list.fontSize+1, QFont::Bold ) );

  cb_yright1 = new QCheckBox(this);
  cb_yright1->setEnabled(true);
  cb_yright1->setChecked(false);
  cb_yright1->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_yright1->setPalette(PALET_NORMAL);
  connect(cb_yright1, SIGNAL(clicked()), this, SLOT(set_yright1()));
  cb_yright1->hide();

  cb_yright2 = new QCheckBox(this);
  cb_yright2->setEnabled(true);
  cb_yright2->setChecked(false);
  cb_yright2->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  cb_yright2->setPalette(PALET_NORMAL);
  connect(cb_yright2, SIGNAL(clicked()), this, SLOT(set_yright2()));
  cb_yright2->hide();

  pb_reffile = new QPushButton(us_tr("Load reference"), this);
  pb_reffile->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize + 1));
  pb_reffile->setMinimumHeight(minHeight1);
  pb_reffile->setPalette(PALET_PUSHB);
  connect(pb_reffile, SIGNAL(clicked()), SLOT(set_reffile()));

  le_reffile = new QLineEdit(this);
  le_reffile->setObjectName("reffile Line Edit");
  le_reffile->setText(parameters.count("reffile") ? parameters["reffile"] : "");
  le_reffile->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_reffile->setPalette(PALET_NORMAL);
  le_reffile->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  le_reffile->setMinimumHeight(minHeight1);
  le_reffile->setMinimumWidth(150);
  connect(le_reffile, SIGNAL(textChanged(const QString &)),
          SLOT(update_reffile(const QString &)));

  lbl_xbottom = new QLabel(us_tr("X axis"), this);
  lbl_xbottom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_xbottom->setMinimumHeight(minHeight1);
  lbl_xbottom->setPalette(PALET_LABEL);
  lbl_xbottom->setFont(QFont(USglobal->config_list.fontFamily,
                             USglobal->config_list.fontSize - 1, QFont::Bold));

  le_xbottom_min = new QLineEdit(this);
  le_xbottom_min->setObjectName("xbottom_min Line Edit");
  le_xbottom_min->setText("");
  le_xbottom_min->setMinimumHeight(minHeight1);
  le_xbottom_min->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xbottom_min->setPalette(PALET_NORMAL);
  le_xbottom_min->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xbottom_min, SIGNAL(textChanged(const QString &)),
          SLOT(update_xbottom_min(const QString &)));

  le_xbottom_max = new QLineEdit(this);
  le_xbottom_max->setObjectName("xbottom_max Line Edit");
  le_xbottom_max->setText("");
  le_xbottom_max->setMinimumHeight(minHeight1);
  le_xbottom_max->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_xbottom_max->setPalette(PALET_NORMAL);
  le_xbottom_max->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_xbottom_max, SIGNAL(textChanged(const QString &)),
          SLOT(update_xbottom_max(const QString &)));

  lbl_yleft = new QLabel(us_tr("Left Y axis"), this);
  lbl_yleft->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_yleft->setMinimumHeight(minHeight1);
  lbl_yleft->setPalette(PALET_LABEL);
  lbl_yleft->setFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize - 1, QFont::Bold));

  le_yleft_min = new QLineEdit(this);
  le_yleft_min->setObjectName("yleft_min Line Edit");
  le_yleft_min->setText("");
  le_yleft_min->setMinimumHeight(minHeight1);
  le_yleft_min->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_yleft_min->setPalette(PALET_NORMAL);
  le_yleft_min->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_yleft_min, SIGNAL(textChanged(const QString &)),
          SLOT(update_yleft_min(const QString &)));

  le_yleft_max = new QLineEdit(this);
  le_yleft_max->setObjectName("yleft_max Line Edit");
  le_yleft_max->setText("");
  le_yleft_max->setMinimumHeight(minHeight1);
  le_yleft_max->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_yleft_max->setPalette(PALET_NORMAL);
  le_yleft_max->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_yleft_max, SIGNAL(textChanged(const QString &)),
          SLOT(update_yleft_max(const QString &)));

  lbl_yright = new QLabel(us_tr("Right Y axis"), this);
  lbl_yright->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  lbl_yright->setMinimumHeight(minHeight1);
  lbl_yright->setPalette(PALET_LABEL);
  lbl_yright->setFont(QFont(USglobal->config_list.fontFamily,
                            USglobal->config_list.fontSize - 1, QFont::Bold));
  lbl_yright->hide();

  le_yright_min = new QLineEdit(this);
  le_yright_min->setObjectName("yright_min Line Edit");
  le_yright_min->setText("");
  le_yright_min->setMinimumHeight(minHeight1);
  le_yright_min->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_yright_min->setPalette(PALET_NORMAL);
  le_yright_min->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_yright_min, SIGNAL(textChanged(const QString &)),
          SLOT(update_yright_min(const QString &)));
  le_yright_min->hide();

  le_yright_max = new QLineEdit(this);
  le_yright_max->setObjectName("yright_max Line Edit");
  le_yright_max->setText("");
  le_yright_max->setMinimumHeight(minHeight1);
  le_yright_max->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);
  le_yright_max->setPalette(PALET_NORMAL);
  le_yright_max->setFont(
      QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
  connect(le_yright_max, SIGNAL(textChanged(const QString &)),
          SLOT(update_yright_max(const QString &)));
  le_yright_max->hide();

  pb_help = new QPushButton(us_tr("Help"), this);
  pb_help->setFont(QFont(USglobal->config_list.fontFamily,
                         USglobal->config_list.fontSize + 1));
  pb_help->setMinimumHeight(minHeight1);
  pb_help->setPalette(PALET_PUSHB);
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));

  pb_close = new QPushButton(us_tr("Close"), this);
  pb_close->setFont(QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize + 1));
  pb_close->setMinimumHeight(minHeight1);
  pb_close->setPalette(PALET_PUSHB);
  connect(pb_close, SIGNAL(clicked()), SLOT(cancel()));

  //   plot = new QwtPlot(this);
  usp_plot = new US_Plot(plot, "", "", "", this);
  connect((QWidget *)plot->titleLabel(),
          SIGNAL(customContextMenuRequested(const QPoint &)),
          SLOT(usp_config_plot(const QPoint &)));
  ((QWidget *)plot->titleLabel())->setContextMenuPolicy(Qt::CustomContextMenu);
  connect((QWidget *)plot->axisWidget(QwtPlot::yLeft),
          SIGNAL(customContextMenuRequested(const QPoint &)),
          SLOT(usp_config_plot(const QPoint &)));
  ((QWidget *)plot->axisWidget(QwtPlot::yLeft))
      ->setContextMenuPolicy(Qt::CustomContextMenu);
  connect((QWidget *)plot->axisWidget(QwtPlot::xBottom),
          SIGNAL(customContextMenuRequested(const QPoint &)),
          SLOT(usp_config_plot(const QPoint &)));
  ((QWidget *)plot->axisWidget(QwtPlot::xBottom))
      ->setContextMenuPolicy(Qt::CustomContextMenu);
#if QT_VERSION < 0x040000
  plot->enableOutline(true);
  plot->setOutlinePen(Qt::white);
  plot->setOutlineStyle(Qwt::VLine);
  plot->enableGridXMin();
  plot->enableGridYMin();
  plot->setPalette(USglobal->global_colors.cg_plot);
  plot->setGridMajPen(QPen(USglobal->global_colors.major_ticks, 0, DotLine));
  plot->setGridMinPen(QPen(USglobal->global_colors.minor_ticks, 0, DotLine));
#else
  grid = new QwtPlotGrid;
  grid->enableXMin(true);
  grid->enableYMin(true);
  plot->setPalette(PALET_NORMAL);
  grid->setMajorPen(QPen(USglobal->global_colors.major_ticks, 0, Qt::DotLine));
  grid->setMinorPen(QPen(USglobal->global_colors.minor_ticks, 0, Qt::DotLine));
  grid->attach(plot);
#endif
  plot->setAxisTitle(QwtPlot::xBottom, parameters["xbottom"]);
  plot->setAxisTitle(QwtPlot::yLeft,
                     parameters["yleft"] + us_tr(" 'Diamonds'"));
  if (parameters.count("yright1")) {
    cb_yright1->setText(parameters["yright1"]);
    cb_yright1->setChecked(true);
    cb_yright1->show();
  }
  if (parameters.count("yright2")) {
    cb_yright2->setText(parameters["yright2"]);
    cb_yright2->show();
  }

#if QT_VERSION < 0x040000
  plot->setTitleFont(QFont(USglobal->config_list.fontFamily,
                           USglobal->config_list.fontSize, QFont::Bold));
  plot->setAxisTitleFont(QwtPlot::yLeft,
                         QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize, QFont::Bold));
#endif
  plot->setAxisFont(QwtPlot::yLeft, QFont(USglobal->config_list.fontFamily,
                                          USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
  plot->setAxisTitleFont(QwtPlot::xBottom,
                         QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize, QFont::Bold));
#endif
  plot->setAxisFont(QwtPlot::xBottom,
                    QFont(USglobal->config_list.fontFamily,
                          USglobal->config_list.fontSize - 1));
#if QT_VERSION < 0x040000
  plot->setAxisTitleFont(QwtPlot::yRight,
                         QFont(USglobal->config_list.fontFamily,
                               USglobal->config_list.fontSize, QFont::Bold));
#endif
  plot->setAxisFont(QwtPlot::yRight, QFont(USglobal->config_list.fontFamily,
                                           USglobal->config_list.fontSize - 1));
  //    plot->setMargin(USglobal->config_list.margin);
  plot->setTitle(parameters["title"]);
  plot->setCanvasBackground(USglobal->global_colors.plot);

  QVBoxLayout *background = new QVBoxLayout(this);
  background->setContentsMargins(0, 0, 0, 0);
  background->setSpacing(0);
  // background->addSpacing(4);

  // background->addWidget( lbl_title );
  // background->addWidget( lbl_credits_1 );
  // background->addSpacing( 4 );
  background->addWidget(plot);

  {
    QGridLayout *gl = new QGridLayout(0);
    gl->setContentsMargins(0, 0, 0, 0);
    gl->setSpacing(0);
    gl->addWidget(lbl_xbottom, 0, 0);
    gl->addWidget(le_xbottom_min, 0, 1);
    gl->addWidget(le_xbottom_max, 0, 2);

    gl->addWidget(lbl_yleft, 1, 0);
    gl->addWidget(le_yleft_min, 1, 1);
    gl->addWidget(le_yleft_max, 1, 2);

    gl->addWidget(lbl_yright, 2, 0);
    gl->addWidget(le_yright_min, 2, 1);
    gl->addWidget(le_yright_max, 2, 2);
    background->addLayout(gl);
  }

  {
    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->setSpacing(0);
    // hbl->addSpacing( 4 );
    hbl->addWidget(cb_yright1);
    hbl->addWidget(cb_yright2);
    background->addLayout(hbl);
  }

  {
    QHBoxLayout *hbl = new QHBoxLayout();
    hbl->setContentsMargins(0, 0, 0, 0);
    hbl->setSpacing(0);
    // hbl->addSpacing( 4 );
    hbl->addWidget(pb_reffile);
    hbl->addWidget(le_reffile);
    background->addLayout(hbl);
  }

  QHBoxLayout *hbl_bottom = new QHBoxLayout();
  hbl_bottom->setContentsMargins(0, 0, 0, 0);
  hbl_bottom->setSpacing(0);
  //   hbl_bottom->addSpacing( 4 );
  hbl_bottom->addWidget(pb_help);
  //   hbl_bottom->addSpacing( 4 );
  hbl_bottom->addWidget(pb_close);
  //   hbl_bottom->addSpacing( 4 );

  background->addLayout(hbl_bottom);
  //   background->addSpacing( 4 );
}

void US_Hydrodyn_Saxs_Guinier_Frames::cancel() { close(); }

void US_Hydrodyn_Saxs_Guinier_Frames::help() {
  US_Help *online_help;
  online_help = new US_Help(this);
  online_help->show_help("manual/somo/guinier_frames.html");
}

void US_Hydrodyn_Saxs_Guinier_Frames::closeEvent(QCloseEvent *e) {
  global_Xpos -= 30;
  global_Ypos -= 30;
  e->accept();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_plot() {
  // us_qdebug( "update plot" );
  // build up from plots map
  // it->first is axis name followed by color, x, y, and possibly error
  // later we could bring in more and allow any pair of selections

  // for ( map < QString, vector < vector < double > > >::iterator it =
  // plots.begin();
  //       it != plots.end();
  //       ++it ) {
  //    us_qdebug( US_Vector::qs_vector4( QString( "plot '%1'" ).arg( it->first
  //    ),
  //                                   it->second[ 0 ],
  //                                   it->second[ 1 ],
  //                                   it->second[ 2 ],
  //                                   it->second[ 3 ] ) );
  // }

  plot->detachItems(QwtPlotItem::Rtti_PlotCurve);
  plot->detachItems(QwtPlotItem::Rtti_PlotMarker);
  ;

  int use_line_width = 1;

  QwtSymbol sym;
  sym.setSize(use_line_width * 5 + 3);
  sym.setStyle(QwtSymbol::Diamond);

  double xb_min = 0e0;
  double xb_max = 0e0;
  double yl_min = 0e0;
  double yl_max = 0e0;
  double yr_min = 0e0;
  double yr_max = 0e0;

  if (plots.count("yleft")) {
    sym.setStyle(QwtSymbol::Diamond);
    vector<vector<double> > *this_plot = &plots["yleft"];

    for (int i = 0; i < (int)(*this_plot)[0].size(); ++i) {
      QString this_name = QString("%1 %2")
                              .arg(parameters[QString("%1-name").arg(i)])
                              .arg(parameters["yleft"]);

      if (i) {
        if (xb_min > (*this_plot)[1][i]) {
          xb_min = (*this_plot)[1][i];
        }

        if (xb_max < (*this_plot)[1][i]) {
          xb_max = (*this_plot)[1][i];
        }
      } else {
        xb_min = xb_max = (*this_plot)[1][i];
      }

      if (i) {
        if (yl_min > (*this_plot)[2][i] - (*this_plot)[3][i]) {
          yl_min = (*this_plot)[2][i] - (*this_plot)[3][i];
        }
        if (yl_max < (*this_plot)[2][i] + (*this_plot)[3][i]) {
          yl_max = (*this_plot)[2][i] + (*this_plot)[3][i];
        }
      } else {
        yl_min = (*this_plot)[2][i] - (*this_plot)[3][i];
        yl_max = (*this_plot)[2][i] + (*this_plot)[3][i];
      }

      int rgb = (int)(*this_plot)[0][i];
      int blue = rgb % 256;
      int green = (rgb / 256) % 256;
      int red = (rgb / (256 * 256)) % 256;
      QColor qc = QColor(red, green, blue);
      sym.setBrush(Qt::black);
      sym.setPen(qc);
      QPen use_pen = QPen(qc, use_line_width, Qt::SolidLine);

      // us_qdebug( QString( "%1 rgb %2 %3 %4" ).arg( rgb ).arg( red ).arg(
      // green ).arg( blue ) );

      {
#if QT_VERSION >= 0x040000
        QwtPlotCurve *curve = new QwtPlotCurve(this_name);
        curve->setStyle(QwtPlotCurve::NoCurve);
        curve->setSymbol(
            new QwtSymbol(sym.style(), sym.brush(), sym.pen(), sym.size()));
        curve->setSamples(&(*this_plot)[1][i], &(*this_plot)[2][i], 1);
        curve->attach(plot);
#else
        long curve = plot->insertCurve(this_name);
        plot->setCurveStyle(curve, QwtCurve::NoCurve);
        plot->setCurveSymbol(curve, sym);
        plot->setCurveData(curve, &(*this_plot)[1][i], &(*this_plot)[2][i], 1);
#endif
      }

      if ((*this_plot)[3][i] > 0e0) {
        double x[2];
        double y[2];

        x[0] = x[1] = (*this_plot)[1][i];
        y[0] = (*this_plot)[2][i] - (*this_plot)[3][i];
        y[1] = (*this_plot)[2][i] + (*this_plot)[3][i];
#if QT_VERSION >= 0x040000
        QwtPlotCurve *curve = new QwtPlotCurve(this_name + "_errorbar");
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setPen(use_pen);
        curve->setSamples(x, y, 2);
        curve->attach(plot);
#else
        long curve = plot->insertCurve(this_name + "_errorbar");
        plot->setCurveStyle(curve, QwtCurve::Lines);
        plot->setCurvePen(curve, use_pen);
        plot->setCurveData(curve, x, y, 2);
#endif
      }
    }

    {
      QString this_name = QString("%1 summary line").arg(parameters["yleft"]);

      QPen use_pen = QPen(Qt::cyan, use_line_width, Qt::DotLine);
#if QT_VERSION >= 0x040000
      QwtPlotCurve *curve = new QwtPlotCurve(this_name);
      curve->setStyle(QwtPlotCurve::Lines);
      curve->setPen(use_pen);
      curve->setSamples(&(*this_plot)[1][0], &(*this_plot)[2][0],
                        (*this_plot)[1].size());
      curve->attach(plot);
#else
      long curve = plot->insertCurve(this_name);
      plot->setCurveStyle(curve, QwtCurve::Lines);
      plot->setCurvePen(curve, use_pen);
      plot->setCurveData(curve, &(*this_plot)[1][0], &(*this_plot)[2][0],
                         (*this_plot)[1].size());
#endif
    }
  }

  {
    QString use_right;
    if (cb_yright1->isChecked()) {
      use_right = "yright1";
    }
    if (cb_yright2->isChecked()) {
      use_right = "yright2";
    }

    if (!use_right.isEmpty() && plots.count(use_right)) {
      lbl_yright->show();
      le_yright_min->show();
      le_yright_max->show();
      plot->enableAxis(QwtPlot::yRight, true);
      plot->setAxisTitle(QwtPlot::yRight,
                         parameters[use_right] + us_tr(" 'Squares'"));
      sym.setStyle(QwtSymbol::Rect);
      vector<vector<double> > *this_plot = &plots[use_right];

      for (int i = 0; i < (int)(*this_plot)[0].size(); ++i) {
        QString this_name = QString("%1 %2")
                                .arg(parameters[QString("%1-name").arg(i)])
                                .arg(parameters[use_right]);

        if (i) {
          if (yr_min > (*this_plot)[2][i] - (*this_plot)[3][i]) {
            yr_min = (*this_plot)[2][i] - (*this_plot)[3][i];
          }
          if (yr_max < (*this_plot)[2][i] + (*this_plot)[3][i]) {
            yr_max = (*this_plot)[2][i] + (*this_plot)[3][i];
          }
        } else {
          yr_min = (*this_plot)[2][i] - (*this_plot)[3][i];
          yr_max = (*this_plot)[2][i] + (*this_plot)[3][i];
        }

        int rgb = (int)(*this_plot)[0][i];
        int blue = rgb % 256;
        int green = (rgb / 256) % 256;
        int red = (rgb / (256 * 256)) % 256;
        QColor qc = QColor(red, green, blue);
        sym.setBrush(Qt::black);
        sym.setPen(qc);
        QPen use_pen = QPen(qc, use_line_width, Qt::SolidLine);

        // us_qdebug( QString( "%1 rgb %2 %3 %4" ).arg( rgb ).arg( red ).arg(
        // green ).arg( blue ) );

        {
#if QT_VERSION >= 0x040000
          QwtPlotCurve *curve = new QwtPlotCurve(this_name);
          curve->setStyle(QwtPlotCurve::NoCurve);
          curve->setSymbol(
              new QwtSymbol(sym.style(), sym.brush(), sym.pen(), sym.size()));
          curve->setSamples(&(*this_plot)[1][i], &(*this_plot)[2][i], 1);
          curve->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
          curve->attach(plot);
#else
          long curve =
              plot->insertCurve(this_name, QwtPlot::xBottom, QwtPlot::yRight);
          plot->setCurveStyle(curve, QwtCurve::NoCurve);
          plot->setCurveSymbol(curve, sym);
          plot->setCurveData(curve, &(*this_plot)[1][i], &(*this_plot)[2][i],
                             1);
#endif
        }
        {
          double x[2];
          double y[2];

          x[0] = x[1] = (*this_plot)[1][i];
          y[0] = (*this_plot)[2][i] - (*this_plot)[3][i];
          y[1] = (*this_plot)[2][i] + (*this_plot)[3][i];
#if QT_VERSION >= 0x040000
          QwtPlotCurve *curve = new QwtPlotCurve(this_name + "_errorbar");
          curve->setStyle(QwtPlotCurve::Lines);
          curve->setPen(use_pen);
          curve->setSamples(x, y, 2);
          curve->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
          curve->attach(plot);
#else
          long curve = plot->insertCurve(this_name + "_errorbar",
                                         QwtPlot::xBottom, QwtPlot::yRight);
          plot->setCurveStyle(curve, QwtCurve::Lines);
          plot->setCurvePen(curve, use_pen);
          plot->setCurveData(curve, x, y, 2);
#endif
          // us_qdebug( QString( "double check plot mw eb x %1 %2 y %3 %4"
          // ).arg( x[0] ).arg( x[1] ).arg( y[0] ).arg( y[1] ) );
        }
      }

      {
        QString this_name =
            QString("%1 summary line").arg(parameters[use_right]);

        QPen use_pen = QPen(Qt::red, use_line_width, Qt::DotLine);
#if QT_VERSION >= 0x040000
        QwtPlotCurve *curve = new QwtPlotCurve(this_name);
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setPen(use_pen);
        curve->setSamples(&(*this_plot)[1][0], &(*this_plot)[2][0],
                          (*this_plot)[1].size());
        curve->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
        curve->attach(plot);
#else
        long curve =
            plot->insertCurve(this_name, QwtPlot::xBottom, QwtPlot::yRight);
        plot->setCurveStyle(curve, QwtCurve::Lines);
        plot->setCurvePen(curve, use_pen);
        plot->setCurveData(curve, &(*this_plot)[1][0], &(*this_plot)[2][0],
                           (*this_plot)[1].size());
#endif
      }
    } else {
      plot->enableAxis(QwtPlot::yRight, false);
      lbl_yright->hide();
      le_yright_min->hide();
      le_yright_max->hide();
    }
  }

  if (ref_I.size()) {
    vector<double> y = ref_I;
    double min_y = 1e99;
    double max_y = -1e99;
    bool any_pts = false;
    for (int i = 0; i < (int)y.size(); ++i) {
      if (ref_t[i] >= xb_min && ref_t[i] <= xb_max) {
        any_pts = true;
        if (min_y > y[i]) {
          min_y = y[i];
        }
        if (max_y < y[i]) {
          max_y = y[i];
        }
      }
    }

    double range = max_y - min_y;
    double scale = 1e0;
    if (range > 0e0) {
      scale = (yl_max - yl_min) / range;
    }

    if (any_pts) {
      for (int i = 0; i < (int)y.size(); ++i) {
        y[i] -= min_y;
        y[i] *= scale;
        y[i] += yl_min;
      }
      {
        QString this_name = QString("reference curve");

        QPen use_pen = QPen(Qt::green, use_line_width, Qt::SolidLine);
#if QT_VERSION >= 0x040000
        QwtPlotCurve *curve = new QwtPlotCurve(this_name);
        curve->setStyle(QwtPlotCurve::Lines);
        curve->setPen(use_pen);
        curve->setSamples(&ref_t[0], &y[0], y.size());
        curve->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
        curve->attach(plot);
#else
        long curve =
            plot->insertCurve(this_name, QwtPlot::xBottom, QwtPlot::yLeft);
        plot->setCurveStyle(curve, QwtCurve::Lines);
        plot->setCurvePen(curve, use_pen);
        plot->setCurveData(curve, &ref_t[0], &y[0], y.size());
#endif
      }
    }
    // us_qdebug( US_Vector::qs_vector3( "ref domain scaled", ref_t, ref_I, y)
    // );
  }

  if (plot_zoomer) {
    delete plot_zoomer;
    plot_zoomer = (ScrollZoomer *)0;
  }

  double use_xbottom_min = xb_min - 1;
  double use_xbottom_max = xb_max + 1;
  double use_yleft_min = yl_min * .9;
  double use_yleft_max = yl_max * 1.1;
  double use_yright_min = yr_min * .9;
  double use_yright_max = yr_max * 1.1;

  if (!le_xbottom_min->text().isEmpty()) {
    bool ok;
    if (le_xbottom_min->text().toDouble(&ok)) {
      use_xbottom_min = le_xbottom_min->text().toDouble();
    }
  }
  if (!le_xbottom_max->text().isEmpty()) {
    bool ok;
    if (le_xbottom_max->text().toDouble(&ok)) {
      use_xbottom_max = le_xbottom_max->text().toDouble();
    }
  }

  if (!le_yleft_min->text().isEmpty()) {
    bool ok;
    if (le_yleft_min->text().toDouble(&ok)) {
      use_yleft_min = le_yleft_min->text().toDouble();
    }
  }
  if (!le_yleft_max->text().isEmpty()) {
    bool ok;
    if (le_yleft_max->text().toDouble(&ok)) {
      use_yleft_max = le_yleft_max->text().toDouble();
    }
  }

  if (!le_yright_min->text().isEmpty()) {
    bool ok;
    if (le_yright_min->text().toDouble(&ok)) {
      use_yright_min = le_yright_min->text().toDouble();
    }
  }
  if (!le_yright_max->text().isEmpty()) {
    bool ok;
    if (le_yright_max->text().toDouble(&ok)) {
      use_yright_max = le_yright_max->text().toDouble();
    }
  }

  plot->setAxisScale(QwtPlot::xBottom, use_xbottom_min, use_xbottom_max);
  plot->setAxisScale(QwtPlot::yLeft, use_yleft_min, use_yleft_max);
  plot->setAxisScale(QwtPlot::yRight, use_yright_min, use_yright_max);

  if (!plot_zoomer) {
    plot_zoomer = new ScrollZoomer(plot->canvas());
    plot_zoomer->setRubberBandPen(QPen(Qt::yellow, 0, Qt::DotLine));
#if QT_VERSION < 0x040000
    plot_zoomer->setCursorLabelPen(QPen(Qt::yellow));
#endif
  }

  plot->replot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::set_reffile() {
  QString tmp;
  update_reffile(tmp);
}

void US_Hydrodyn_Saxs_Guinier_Frames::set_yright1() {
  if (cb_yright1->isChecked() && cb_yright2->isChecked()) {
    cb_yright2->setChecked(false);
  }
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::set_yright2() {
  if (cb_yright1->isChecked() && cb_yright2->isChecked()) {
    cb_yright1->setChecked(false);
  }
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_xbottom_min(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_xbottom_max(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_yleft_min(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_yleft_max(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_yright_min(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_yright_max(const QString &) {
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::update_reffile(const QString &) {
  QString use_dir;
  ((US_Hydrodyn *)us_hydrodyn)->select_from_directory_history(use_dir, this);
  QString filename = QFileDialog::getOpenFileName(
      this, us_tr("Select a file for Reference concentration intensity curve"),
      use_dir, QString());

  disconnect(le_reffile, SIGNAL(textChanged(const QString &)), 0, 0);
  le_reffile->setText(filename);
  connect(le_reffile, SIGNAL(textChanged(const QString &)),
          SLOT(update_reffile(const QString &)));
  if (!filename.isEmpty()) {
    ((US_Hydrodyn *)us_hydrodyn)->add_to_directory_history(filename);
    QString error_msg;
    US_Saxs_Util::read_sas_data(filename, ref_t, ref_I, ref_e, error_msg);
    // us_qdebug( US_Vector::qs_vector3( filename + " " + error_msg, ref_t,
    // ref_I, ref_e ) );
  } else {
    ref_t.clear();
    ref_I.clear();
    ref_e.clear();
  }
  parameters["reffile"] = le_reffile->text();
  update_plot();
}

void US_Hydrodyn_Saxs_Guinier_Frames::usp_config_plot(const QPoint &) {
  US_PlotChoices *uspc = new US_PlotChoices(usp_plot);
  uspc->exec();
  delete uspc;
}
