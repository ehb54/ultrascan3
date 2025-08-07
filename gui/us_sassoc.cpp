//! \file us_sassoc.cpp
#include "us_sassoc.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#if QT_VERSION < 0x050000
#define setSamples(a, b, c) setData(a, b, c)
#else
#include "qwt_point_data.h"
#endif

#include "qwt_scale_engine.h"

US_Sassoc::US_Sassoc(
   double eq0, double eq1, double stoich1, double stoich2, const QString &t_project, int t_model, bool status,
   bool position_win, QWidget *parent, Qt::WindowFlags flags) : US_Widgets(position_win, parent, flags) {
   eq[ 0 ] = eq0;
   eq[ 1 ] = eq1;
   model = t_model;
   stoich[ 0 ] = stoich1;
   stoich[ 1 ] = stoich2;
   project = t_project;
   updating = false;

   setPalette(US_GuiSettings::frameColor());
   setWindowTitle(tr("Equilibrium Concentration Distribution"));

   QBoxLayout *main = new QHBoxLayout(this);
   main->setContentsMargins(2, 2, 2, 2);
   main->setSpacing(2);

   QBoxLayout *controls = new QVBoxLayout;

   QLabel *lb_header = us_banner(
      tr("Species Distribution in\n"
         "Self-Associating System"),
      -2);
   lb_header->setMargin(10);
   controls->addWidget(lb_header);

   QLabel *lb_stoich = us_banner(tr("Stoichiometries:"), -2);
   controls->addWidget(lb_stoich);

   // Association constants
   QGridLayout *params = new QGridLayout;
   int row = 0;

   QLabel *lb_assoc1 = us_label("Association1:");
   params->addWidget(lb_assoc1, row, 0);

   le_assoc1 = us_lineedit(QString::number(stoich1, 'f', 4));
   connect(le_assoc1, SIGNAL(textChanged(const QString &)), SLOT(update_stoich1(const QString &)));
   params->addWidget(le_assoc1, row++, 1);

   QLabel *lb_assoc2 = us_label("Association2:");
   params->addWidget(lb_assoc2, row, 0);

   le_assoc2 = us_lineedit(QString::number(stoich2, 'f', 4));
   connect(le_assoc2, SIGNAL(textChanged(const QString &)), SLOT(update_stoich2(const QString &)));
   params->addWidget(le_assoc2, row++, 1);

   // Equlibrium constants
   QLabel *lb_equil = us_banner(tr("Equilibrium Constants:"), -2);
   params->addWidget(lb_equil, row++, 0, 1, 2);

   QLabel *lb_equil1 = us_label("ln( Constant1 ):");
   params->addWidget(lb_equil1, row, 0);

   le_equil1 = us_lineedit(QString::number(eq0, 'g', 3));
   connect(le_equil1, SIGNAL(textChanged(const QString &)), SLOT(update_eq1(const QString &)));
   params->addWidget(le_equil1, row++, 1);

   c_equil1 = us_counter(3, -100.0, 100.0, eq0);
   c_equil1->setSingleStep(0.01);
   connect(c_equil1, SIGNAL(valueChanged(double)), SLOT(update_eq1Count(double)));
   params->addWidget(c_equil1, row++, 0, 1, 2);

   QLabel *lb_equil2 = us_label("ln( Constant2 ):");
   params->addWidget(lb_equil2, row, 0);

   le_equil2 = us_lineedit(QString::number(eq1, 'g', 3));
   connect(le_equil2, SIGNAL(textChanged(const QString &)), SLOT(update_eq2(const QString &)));
   params->addWidget(le_equil2, row++, 1);

   c_equil2 = us_counter(3, -100.0, 100.0, eq1);
   c_equil2->setSingleStep(0.01);
   connect(c_equil2, SIGNAL(valueChanged(double)), SLOT(update_eq2Count(double)));
   params->addWidget(c_equil2, row++, 0, 1, 2);

   controls->addLayout(params);

   // Other data
   QLabel *lb_model = us_banner(tr("Equilibrium Model:"), -2);
   controls->addWidget(lb_model);

   QStringList models = US_Constants::modelStrings();

   QTextEdit *te_model = us_textedit();
   te_model->setText(models[ model ]);
   te_model->setReadOnly(true);
   te_model->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
   te_model->setMaximumHeight(100);
   QPalette p = US_GuiSettings::editColor();
   p.setColor(QPalette::Base, Qt::lightGray);
   te_model->setPalette(p);
   controls->addWidget(te_model);

   QLabel *lb_project = us_banner(tr("Project Name:"), -2);
   controls->addWidget(lb_project);

   QLineEdit *le_project = us_lineedit(project);
   le_project->setReadOnly(true);
   le_project->setPalette(p);
   controls->addWidget(le_project);

   controls->addStretch();

   // Buttons
   QPushButton *pb_save = us_pushbutton(tr("Save Data to File"));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));
   controls->addWidget(pb_save);

   QPushButton *pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   controls->addWidget(pb_help);

   QPushButton *pb_close = us_pushbutton(tr("Close"));
   connect(pb_close, SIGNAL(clicked()), SLOT(close()));
   controls->addWidget(pb_close);

   main->addLayout(controls);
   QBoxLayout *rightSide = new QVBoxLayout;

   // Graph
   plot1 = new US_Plot(
      plot, tr("Self-Association Profile for") + project, tr("Total Concentration"), tr("% of Total Concentration"));

#if QT_VERSION < 0x050000
   plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLog10ScaleEngine);
#else
   plot->setAxisScaleEngine(QwtPlot::xBottom, new QwtLogScaleEngine);
#endif
   pick = new US_PlotPicker(plot);
   pick->setRubberBand(QwtPicker::VLineRubberBand);
   connect(pick, SIGNAL(selected(const QwtDoublePoint &)), SLOT(new_value(const QwtDoublePoint &)));
   connect(pick, SIGNAL(moved(const QwtDoublePoint &)), SLOT(new_value(const QwtDoublePoint &)));
   connect(pick, SIGNAL(mouseDown(const QwtDoublePoint &)), SLOT(mouseD(const QwtDoublePoint &)));
   connect(pick, SIGNAL(mouseUp(const QwtDoublePoint &)), SLOT(mouseU(const QwtDoublePoint &)));

   // Initialize x
   double c = 1.0e-10; // Lower concentration range limit
   const double inc = 20.0;

   for (int i = 0; i < ARRAY_SIZE; i++) {
      x[ i ] = c;
      c += c / inc;
   }

   curve1 = us_curve(plot, tr("Species 1"));
   curve2 = us_curve(plot, tr("Species 2"));
   curve3 = us_curve(plot, tr("Species 3"));

   curve1->setPen(QPen(QBrush(Qt::green), 2.0));
   curve2->setPen(QPen(QBrush(Qt::yellow), 2.0));
   curve3->setPen(QPen(QBrush(Qt::cyan), 2.0));

   recalc();

   QGridLayout *legend = new QGridLayout;

   p = US_GuiSettings::labelColor();
   QLabel *lb_species1 = us_label(tr("Species 1:"));
   p.setColor(QPalette::WindowText, Qt::green);
   lb_species1->setPalette(p);
   legend->addWidget(lb_species1, 0, 0);

   le_species1 = us_lineedit("0 %");
   le_species1->setReadOnly(true);
   legend->addWidget(le_species1, 0, 1);

   QLabel *lb_species2 = us_label(tr("Species 2:"));
   p.setColor(QPalette::WindowText, Qt::yellow);
   lb_species2->setPalette(p);
   legend->addWidget(lb_species2, 0, 2);

   le_species2 = us_lineedit("0 %");
   le_species2->setReadOnly(true);
   legend->addWidget(le_species2, 0, 3);

   QLabel *lb_species3 = us_label(tr("Species 3:"));
   p.setColor(QPalette::WindowText, Qt::cyan);
   lb_species3->setPalette(p);
   legend->addWidget(lb_species3, 0, 4);

   le_species3 = us_lineedit("0 %");
   le_species3->setReadOnly(true);
   legend->addWidget(le_species3, 0, 5);

   QLabel *conc = us_label(tr("Total Concentration:"));
   legend->addWidget(conc, 1, 0);

   le_conc = us_lineedit("0 M");
   le_species3->setReadOnly(true);
   legend->addWidget(le_conc, 1, 1);

   QLineEdit *le_conc = us_lineedit(tr("Drag mouse through plot area to see relative concentrations"));
   le_species3->setReadOnly(true);
   legend->addWidget(le_conc, 1, 2, 1, 4);

   rightSide->addLayout(plot1);
   rightSide->addLayout(legend);
   main->addLayout(rightSide);
   status = !status; // Avoid warning for now
}

void US_Sassoc::recalc(void) {
   for (int i = 0; i < ARRAY_SIZE; i++) {
      species1[ i ] = monomer_root(x[ i ]);
      species2[ i ] = stoich[ 0 ] * pow(species1[ i ], stoich[ 0 ]) * exp(eq[ 0 ]);
      species3[ i ] = stoich[ 1 ] * pow(species1[ i ], stoich[ 1 ]) * exp(eq[ 1 ]);

      species1[ i ] *= 100 / x[ i ];
      species2[ i ] *= 100 / x[ i ];
      species3[ i ] *= 100 / x[ i ];
   }

   curve1->setSamples(x, species1, ARRAY_SIZE);
   curve2->setSamples(x, species2, ARRAY_SIZE);
   curve3->setSamples(x, species3, ARRAY_SIZE);
   plot->replot();
}

double US_Sassoc::monomer_root(double total) {
   double monomer = total / 2;
   double diff = monomer / 2;
   double diff1 = monomer - diff; // = total / 4
   double test = polynomial(monomer, total);

   while (fabs(diff1) > 1.0e-15) {
      double monomer_old = monomer;

      if (test > 0)
         monomer = monomer - diff;
      else
         monomer = monomer + diff;

      test = polynomial(monomer, total);
      diff = fabs(monomer - monomer_old) / 2;
      diff1 = monomer_old - monomer;
   }

   return monomer;
}

double US_Sassoc::polynomial(double monomer, double total) {
   return monomer + stoich[ 0 ] * pow(monomer, stoich[ 0 ]) * exp(eq[ 0 ])
          + stoich[ 1 ] * pow(monomer, stoich[ 1 ]) * exp(eq[ 1 ]) - total;
}

void US_Sassoc::new_value(const QwtDoublePoint &p) {
   update_legend(p.x());
}

void US_Sassoc::mouseU(const QwtDoublePoint & /* p */) {
   update_legend(-1.0);
}

void US_Sassoc::mouseD(const QwtDoublePoint &p) {
   updating = true;
   update_legend(p.x());
}

void US_Sassoc::update_legend(const double total) {
   if (total >= 1.0e-10 && total <= 100.0 && updating) {
      int i = 0;
      while (x[ i ] < total)
         i++;

#if QT_VERSION < 0x050000
      le_species1->setText(QString::number(curve1->y(i)) + " %");
      le_species2->setText(QString::number(curve2->y(i)) + " %");
      le_species3->setText(QString::number(curve3->y(i)) + " %");
#else
      QwtSeriesData<QPointF> *cdata1 = curve1->data();
      QwtSeriesData<QPointF> *cdata2 = curve2->data();
      QwtSeriesData<QPointF> *cdata3 = curve3->data();

      //le_species1->setText( QString::number( curve1->y( i ) ) + " %" );
      //le_species2->setText( QString::number( curve2->y( i ) ) + " %" );
      //le_species3->setText( QString::number( curve3->y( i ) ) + " %" );
      le_species1->setText(QString::number(cdata1->sample(i).y()) + " %");
      le_species2->setText(QString::number(cdata2->sample(i).y()) + " %");
      le_species3->setText(QString::number(cdata3->sample(i).y()) + " %");
#endif
      le_conc->setText(QString::number(total) + " M");
   }
   else {
      updating = false;
      le_species1->setText(" 0 %");
      le_species2->setText(" 0 %");
      le_species3->setText(" 0 %");
      le_conc->setText(" 0 M");
   }
}

void US_Sassoc::update_stoich1(const QString &s) {
   stoich[ 0 ] = s.toDouble();
   recalc();
}

void US_Sassoc::update_stoich2(const QString &s) {
   stoich[ 1 ] = s.toDouble();
   recalc();
}

void US_Sassoc::update_eq1(const QString &s) {
   eq[ 0 ] = s.toDouble();
   c_equil1->setValue(eq[ 0 ]);
   recalc();
}

void US_Sassoc::update_eq2(const QString &s) {
   eq[ 1 ] = s.toDouble();
   c_equil2->setValue(eq[ 1 ]);
   recalc();
}

void US_Sassoc::update_eq1Count(double value) {
   eq[ 0 ] = value;
   le_equil1->setText(QString::number(value));
   // No need to call recalc(), since this will signal textChanged() in lineedit
}

void US_Sassoc::update_eq2Count(double value) {
   eq[ 1 ] = value;
   le_equil2->setText(QString::number(value));
   // No need to call recalc(), since this will signal textChanged() in lineedit
}

void US_Sassoc::save(void) {
   QString filename = US_Settings::resultDir() + "/" + project + "-" + QString::number(model) + ".dis";

   QFile f(filename);

   if (f.exists()) {
      if (
         QMessageBox::warning(
            this, tr("Warning"),
            tr("Attention:\nThis file exists already!\n\n") + filename + tr("\n\nDo you want to overwrite it?"),
            QMessageBox::Yes, QMessageBox::No)
         == QMessageBox::No) {
         return;
      }
   }

   // Save the data
   if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream ts(&f);
      ts << tr("\"Total Concentration\"\t\"% Monomer\"\t\"% ") << stoich[ 0 ] << "-mer\"\t\"% " << stoich[ 1 ]
         << "-mer\"\n";

      for (int i = 0; i < ARRAY_SIZE; i++) {
         ts << x[ i ] << "\t" << species1[ i ] << "\t" << species2[ i ] << "\t" << species3[ i ] << endl;
      }
   }

   f.close();

   // Save a picture
   QDir dir;
   QString reportDir = US_Settings::reportDir();
   if (!dir.exists(reportDir))
      dir.mkpath(reportDir);

   filename = reportDir + "/" + project + "-" + QString::number(model) + ".distribution.png";

   QRect r = QRect(2, 2, plot->width() - 4, plot->height() - 4);
#if QT_VERSION < 0x050000
   QPixmap p = QPixmap::grabWidget(plot, r);
#else
   QPixmap p = (( QWidget * ) plot)->grab(r);
#endif

   p.save(filename);
}
