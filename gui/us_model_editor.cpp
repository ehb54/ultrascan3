//! \file us_model_editor.cpp

#include "us_model_editor.h"
#include "us_analyte_gui.h"
#include "us_constants.h"
#include "us_gui_settings.h"
#include "us_settings.h"

US_ModelEditor::US_ModelEditor(ModelSystem &current_model, QWidget *p, Qt::WindowFlags f) :
    US_WidgetsDialog(p, f), model(current_model) {
   setWindowTitle("Model Editor");
   setPalette(US_GuiSettings::frameColor());

   setWindowModality(Qt::WindowModal);
   setAttribute(Qt::WA_DeleteOnClose);

   QGridLayout *main = new QGridLayout(this);
   main->setSpacing(2);

   component = 0; // Initial component
   shape = PROLATE; // Inital shape (prolate)
   c0_file = "";

   // Convenience
   struct SimulationComponent *sc = &model.component_vector[ component ];
   vector<struct SimulationComponent> *scl = &model.component_vector;

   sc->density = DENS_20W;

   int row = 0;

   lb_header = us_banner(US_Constants::modelStrings()[ model.model ]);
   main->addWidget(lb_header, row++, 0, 1, 4);

   // Left Column

   // Label
   QLabel *lb_current = us_label(tr("Current Component:"));
   lb_current->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
   main->addWidget(lb_current, row++, 0, 1, 2);

   // Combo box
   cmb_component1 = us_comboBox();
   cmb_component1->setMaxVisibleItems(5);

   for (uint i = 0; i < scl->size(); i++)
      cmb_component1->addItem((*scl)[ i ].name);

   cmb_component1->setCurrentIndex(0);

   connect(cmb_component1, SIGNAL(currentIndexChanged(int)), SLOT(change_component1(int)));

   main->addWidget(cmb_component1, row++, 0, 1, 2);

   // Label + line edit
   QLabel *lb_sedCoef = us_label(tr("Sedimentation Coeff. (sec):"));
   main->addWidget(lb_sedCoef, row, 0);

   le_sed = us_lineedit(QString::number(sc->s, 'e', 4));
   connect(le_sed, SIGNAL(textEdited(const QString &)), SLOT(update_sed(const QString &)));
   main->addWidget(le_sed, row++, 1);

   // Label + line edit
   QLabel *lb_diffusion = us_label(tr("Diffusion Coeff. (cm<sup>2</sup>/sec):"));
   main->addWidget(lb_diffusion, row, 0);

   le_diff = us_lineedit(QString::number(sc->D, 'e', 4));
   connect(le_diff, SIGNAL(textEdited(const QString &)), SLOT(update_diff(const QString &)));
   main->addWidget(le_diff, row++, 1);

   // Label + line edit
   QLabel *lb_ExtCoef = us_label(tr("Extinction Coeff. (OD/mol):"));
   main->addWidget(lb_ExtCoef, row, 0);

   le_extinction = us_lineedit(QString::number(sc->extinction, 'e', 4));
   connect(le_extinction, SIGNAL(textEdited(const QString &)), SLOT(update_extinction(const QString &)));
   main->addWidget(le_extinction, row++, 1);

   // Pushbutton + line edit
   pb_vbar = us_pushbutton("vbar (ccm/g, 20C)");
   connect(pb_vbar, SIGNAL(clicked()), SLOT(get_vbar()));
   main->addWidget(pb_vbar, row, 0);

   le_vbar = us_lineedit(QString::number(sc->vbar20, 'e', 4));
   connect(le_vbar, SIGNAL(textEdited(const QString &)), SLOT(update_vbar(const QString &)));
   main->addWidget(le_vbar, row++, 1);

   // Label + line edit
   QLabel *lb_mw = us_label(tr("Molecular Weight (Da)"));
   main->addWidget(lb_mw, row, 0);

   le_mw = us_lineedit(QString::number(sc->mw, 'e', 4));
   connect(le_mw, SIGNAL(textEdited(const QString &)), SLOT(update_mw(const QString &)));
   main->addWidget(le_mw, row++, 1);

   // Label + line edit
   QLabel *lb_f_f0 = us_label(tr("Frictional Ratio (f/f0)"));
   main->addWidget(lb_f_f0, row, 0);

   le_f_f0 = us_lineedit(QString::number(sc->f_f0, 'e', 4));
   connect(le_f_f0, SIGNAL(textEdited(const QString &)), SLOT(update_f_f0(const QString &)));
   main->addWidget(le_f_f0, row++, 1);

   // Pushbutton + spin box
   QPushButton *pb_sim = us_pushbutton("Simulate s and D");
   connect(pb_sim, SIGNAL(clicked()), SLOT(simulate_component()));
   main->addWidget(pb_sim, row, 0);

   sb_count = new QSpinBox();
   sb_count->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()));
   sb_count->setRange(1, scl->size());
   sb_count->setValue(1);
   sb_count->setPalette(US_GuiSettings::editColor());

   connect(sb_count, SIGNAL(valueChanged(int)), SLOT(change_spinbox(int)));

   main->addWidget(sb_count, row++, 1);

   // Radio buttons

   QGridLayout *prolate = us_radiobutton("Prolate Ellipsoid", rb_prolate, true);
   rb_prolate->setEnabled(false);
   main->addLayout(prolate, row, 0);

   QGridLayout *oblate = us_radiobutton("Oblate Ellipsoid", rb_oblate);
   ;
   rb_oblate->setEnabled(false);
   main->addLayout(oblate, row++, 1);

   QGridLayout *rod = us_radiobutton("Long Rod", rb_rod);
   rb_rod->setEnabled(false);
   main->addLayout(rod, row, 0);

   QGridLayout *sphere = us_radiobutton("Sphere", rb_sphere);
   rb_sphere->setEnabled(false);
   main->addLayout(sphere, row++, 1);

   QButtonGroup *shapeButtons = new QButtonGroup(this);
   shapeButtons->addButton(rb_prolate, PROLATE);
   shapeButtons->addButton(rb_oblate, OBLATE);
   shapeButtons->addButton(rb_rod, ROD);
   shapeButtons->addButton(rb_sphere, SPHERE);
   connect(shapeButtons, SIGNAL(buttonClicked(int)), SLOT(select_shape(int)));

   // Right column

   row = 1;

   QLabel *lb_linked = us_label(tr("This component is linked to:"));
   main->addWidget(lb_linked, row++, 2, 1, 2);

   // Combo box
   cmb_component2 = us_listwidget();
   cmb_component2->setEnabled(false);
   cmb_component2->setMaximumHeight(( int ) (cmb_component1->height() * 1.7));

   for (uint i = 0; i < sc->show_component.size(); i++)
      cmb_component2->addItem((*scl)[ sc->show_component[ i ] ].name);

   main->addWidget(cmb_component2, row++, 2, 2, 2);
   row++;

   // Label + line edit
   QLabel *lb_conc = us_label(tr("Partial Conc. (in OD):"));
   main->addWidget(lb_conc, row, 2);

   le_conc = us_lineedit(QString::number(sc->concentration, 'e', 4));
   connect(le_conc, SIGNAL(textEdited(const QString &)), SLOT(update_conc(const QString &)));
   main->addWidget(le_conc, row++, 3);

   // Pushbutton + line edit
   pb_load_c0 = us_pushbutton("Load C0 File", false);
   connect(pb_load_c0, SIGNAL(clicked()), SLOT(load_c0()));
   main->addWidget(pb_load_c0, row, 2);

   le_c0 = us_lineedit("");
   le_c0->setEnabled(false);
   main->addWidget(le_c0, row++, 3);

   //label x 2
   QLabel *lb_adjust = us_label(tr("Please adjust for correct pathlength)"));
   lb_adjust->setAlignment(Qt::AlignCenter);
   main->addWidget(lb_adjust, row++, 2, 1, 2);

   // Label + line edit
   QLabel *lb_keq = us_label(tr("Equilibrium Const. (in OD):"));
   main->addWidget(lb_keq, row, 2);

   le_keq = us_lineedit("");
   le_keq->setEnabled(false);
   connect(le_keq, SIGNAL(textEdited(const QString &)), SLOT(update_keq(const QString &)));
   main->addWidget(le_keq, row++, 3);

   // Label + line edit
   QLabel *lb_koff = us_label(tr("K_off Rate Constant (1/sec):"));
   main->addWidget(lb_koff, row, 2);

   le_koff = us_lineedit("");
   le_koff->setEnabled(false);
   connect(le_koff, SIGNAL(textEdited(const QString &)), SLOT(update_koff(const QString &)));
   main->addWidget(le_koff, row++, 3);

   // Label + line edit
   QLabel *lb_stoich = us_label(tr("Stoichiometry:"));
   main->addWidget(lb_stoich, row, 2);

   le_stoich = us_lineedit("");
   le_stoich->setEnabled(false);
   main->addWidget(le_stoich, row++, 3);

   // Label + line edit
   QLabel *lb_sigma = us_label(tr("Conc. Dependency of s (sigma):"));
   main->addWidget(lb_sigma, row, 2);

   le_sigma = us_lineedit("");
   le_sigma->setEnabled(false);
   connect(le_sigma, SIGNAL(textEdited(const QString &)), SLOT(update_sigma(const QString &)));
   main->addWidget(le_sigma, row++, 3);

   // Label + line edit
   QLabel *lb_delta = us_label(tr("Conc. Dependency of D (delta):"));
   main->addWidget(lb_delta, row, 2);

   le_delta = us_lineedit("");
   le_delta->setEnabled(false);
   connect(le_delta, SIGNAL(textEdited(const QString &)), SLOT(update_delta(const QString &)));
   main->addWidget(le_delta, row++, 3);

   // empty row
   row++;

   // Pushbuttons
   QBoxLayout *buttonbox = new QHBoxLayout;

   QPushButton *pb_load = us_pushbutton(tr("Load Model File"));
   connect(pb_load, SIGNAL(clicked()), SLOT(load_model()));
   buttonbox->addWidget(pb_load);

   QPushButton *pb_save = us_pushbutton(tr("Save Model"));
   connect(pb_save, SIGNAL(clicked()), SLOT(save_model()));
   buttonbox->addWidget(pb_save);

   QPushButton *pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));
   buttonbox->addWidget(pb_help);

   QPushButton *pb_close = us_pushbutton(tr("Cancel"));
   buttonbox->addWidget(pb_close);
   connect(pb_close, SIGNAL(clicked()), SLOT(reject()));

   QPushButton *pb_accept = us_pushbutton(tr("Accept Model"));
   buttonbox->addWidget(pb_accept);
   connect(pb_accept, SIGNAL(clicked()), SLOT(accept_model()));

   main->addLayout(buttonbox, row++, 0, 1, 4);
}

void US_ModelEditor::change_component1(int index) {
   component = index;

   sb_count->disconnect();
   sb_count->setValue(index + 1);
   connect(sb_count, SIGNAL(valueChanged(int)), SLOT(change_spinbox(int)));
   update_component();
}

void US_ModelEditor::change_spinbox(int value) {
   component = value - 1;

   cmb_component1->disconnect();
   cmb_component1->setCurrentIndex(component);

   connect(cmb_component1, SIGNAL(currentIndexChanged(int)), SLOT(change_component1(int)));

   update_component();
}

void US_ModelEditor::update_component(void) {
   rb_prolate->setEnabled(false);
   rb_oblate->setEnabled(false);
   rb_rod->setEnabled(false);
   rb_sphere->setEnabled(false);

   // Convenience
   struct SimulationComponent *sc = &model.component_vector[ component ];
   vector<struct SimulationComponent> *scl = &model.component_vector;

   le_sed->setText(QString::number(sc->s, 'e', 4));
   le_diff->setText(QString::number(sc->D, 'e', 4));
   le_extinction->setText(QString::number(sc->extinction, 'e', 4));
   le_vbar->setText(QString::number(sc->vbar20, 'e', 4));
   le_mw->setText(QString::number(sc->mw, 'e', 4));
   le_f_f0->setText(QString::number(sc->f_f0, 'e', 4));

   if (sc->shape == "sphere")
      rb_sphere->setDown(true);
   else if (sc->shape == "prolate")
      rb_prolate->setDown(true);
   else if (sc->shape == "oblate")
      rb_oblate->setDown(true);
   else if (sc->shape == "rod")
      rb_rod->setDown(true);

   // Find the associated components for component1 and enter them into the
   // linked component list:

   cmb_component2->clear();

   for (uint i = 0; i < sc->show_component.size(); i++)
      cmb_component2->addItem((*scl)[ sc->show_component[ i ] ].name);

   if (sc->show_conc) {
      le_conc->setEnabled(true);
      le_mw->setEnabled(true);
      le_vbar->setEnabled(true);
      pb_load_c0->setEnabled(true);
      pb_vbar->setEnabled(true);

      if (sc->concentration == -1.0) {
         le_conc->setText("from file");
         le_c0->setText(c0_file);
      }
      else {
         le_conc->setText(QString::number(sc->concentration, 'e', 4));
         sc->c0.radius.clear();
         sc->c0.concentration.clear();
      }
   }
   else {
      // Can't edit an associated species' mw or vbar
      le_mw->setEnabled(false);

      le_vbar->setEnabled(false);
      pb_vbar->setEnabled(false);

      le_conc->setEnabled(false);
      le_conc->setText("");

      pb_load_c0->setEnabled(false);
      le_c0->setText("");
   }

   // Convenience
   vector<struct Association> *assoc = &model.assoc_vector;

   if (sc->show_keq) {
      for (uint i = 0; i < assoc->size(); i++) { // only check the dissociating species
         if ((*assoc)[ i ].component2 == component || (*assoc)[ i ].component3 == component) {
            le_keq->setText(QString::number((*assoc)[ i ].keq, 'e', 4));
            le_keq->setEnabled(true);

            le_koff->setText(QString::number((*assoc)[ i ].k_off, 'e', 4));
            le_koff->setEnabled(true);
         }
      }
   }
   else {
      le_keq->setEnabled(false);
      le_keq->setText("");

      le_koff->setEnabled(false);
      le_koff->setText("");
   }

   if (sc->show_stoich > 0) {
      // This species is the dissociating species in a self-associating system

      // le_stoich->setEnabled( true );
      le_stoich->setText(QString::number(sc->show_stoich));
      sc->mw = (*scl)[ sc->show_component[ 0 ] ].mw * sc->show_stoich;


      le_mw->setText(QString::number(sc->mw, 'e', 4));


      sc->vbar20 = (*scl)[ sc->show_component[ 0 ] ].vbar20;

      le_vbar->setText(QString::number(sc->vbar20, 'e', 4));

      update_sD();
   }
   else if (sc->show_stoich == -1) {
      // This species is the dissociating species in a 2-component
      // hetero-associating system

      le_stoich->setText("hetero-associating");

      sc->mw = (*scl)[ sc->show_component[ 0 ] ].mw + (*scl)[ sc->show_component[ 1 ] ].mw;

      le_mw->setText(QString::number(sc->mw, 'e', 4));

      double fraction1 = (*scl)[ sc->show_component[ 0 ] ].mw / sc->mw;
      double fraction2 = (*scl)[ sc->show_component[ 1 ] ].mw / sc->mw;

      sc->vbar20 = (*scl)[ sc->show_component[ 0 ] ].vbar20 * fraction1
                   + (*scl)[ sc->show_component[ 1 ] ].vbar20 * fraction2;

      le_vbar->setText(QString::number(sc->vbar20, 'e', 4));

      update_sD();
   }
   else {
      le_stoich->setText("");
      le_stoich->setEnabled(false);
   }
}

void US_ModelEditor::update_sD(void) {
   // Convenience
   struct SimulationComponent *sc = &model.component_vector[ component ];

   double base = 0.75 / AVOGADRO * sc->mw * sc->vbar20 * M_PI * M_PI;

   sc->s = sc->mw * (1.0 - sc->vbar20 * DENS_20W) / (AVOGADRO * sc->f_f0 * 6.0 * VISC_20W * pow(base, 1.0 / 3.0));

   base = 2.0 * sc->s * sc->f_f0 * sc->vbar20 * VISC_20W / (1.0 - sc->vbar20 * DENS_20W);

   sc->D = R * K20 / (AVOGADRO * sc->f_f0 * 9.0 * VISC_20W * M_PI * pow(base, 0.5));

   le_sed->setText(QString::number(sc->s, 'e', 4));
   le_diff->setText(QString::number(sc->D, 'e', 4));
}

void US_ModelEditor::get_vbar(void) {
   US_AnalyteGui *vbar_dlg = new US_AnalyteGui(-1, true);
   connect(vbar_dlg, SIGNAL(valueChanged(double)), SLOT(update_vbar(double)));
   vbar_dlg->exec();
}

void US_ModelEditor::update_vbar(double vbar) {
   if (vbar > 0.0)
      le_vbar->setText(QString::number(vbar, 'f', 4));
}

void US_ModelEditor::simulate_component(void) {
   US_Predict1 *hydro = new US_Predict1(simcomp);
   connect(hydro, SIGNAL(changed()), SLOT(update_shape()));
   hydro->exec();

   rb_prolate->setEnabled(true);
   rb_oblate->setEnabled(true);
   rb_rod->setEnabled(true);
   rb_sphere->setEnabled(true);
}

void US_ModelEditor::select_shape(int new_shape) {
   shape = new_shape;
   update_shape();
}

void US_ModelEditor::update_shape(void) {
   struct SimulationComponent *sc = &model.component_vector[ component ];

   switch (shape) {
      case PROLATE:
         sc->s = simcomp.prolate.s;
         sc->D = simcomp.prolate.D;
         sc->f_f0 = simcomp.prolate.f_f0;
         sc->shape = "prolate";
         break;

      case OBLATE:
         sc->s = simcomp.oblate.s;
         sc->D = simcomp.oblate.D;
         sc->f_f0 = simcomp.oblate.f_f0;
         sc->shape = "oblate";
         break;

      case ROD:
         sc->s = simcomp.rod.s;
         sc->D = simcomp.rod.D;
         sc->f_f0 = simcomp.rod.f_f0;
         sc->shape = "rod";
         break;
      case SPHERE:

         sc->s = simcomp.sphere.s;
         sc->D = simcomp.sphere.D;
         sc->f_f0 = simcomp.sphere.f_f0;
         sc->shape = "sphere";
         break;
   }

   sc->mw = simcomp.mw;
   sc->vbar20 = simcomp.vbar;
   sc->density = simcomp.density;

   update_component();

   rb_prolate->setEnabled(true);
   rb_oblate->setEnabled(true);
   rb_rod->setEnabled(true);
   rb_sphere->setEnabled(true);
}

void US_ModelEditor::load_c0(void) {
   QMessageBox::information(
      this, tr("UltraScan Information"),
      tr("Please note:\n\n"
         "The initial concentration file should have\n"
         "the following format:\n\n"
         "radius_value1 concentration_value1\n"
         "radius_value2 concentration_value2\n"
         "radius_value3 concentration_value3\n"
         "etc...\n\n"
         "radius values smaller than the meniscus or\n"
         "larger than the bottom of the cell will be\n"
         "excluded from the concentration vector."));

   QString fn = QFileDialog::getOpenFileName(this, US_Settings::resultDir(), "*");

   if (!fn.isEmpty()) {
      le_c0->setText(fn);

      QFile f(fn);
      ;

      if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
         c0_file = fn;
         QTextStream ts(&f);

         struct SimulationComponent *sc = &model.component_vector[ component ];

         sc->c0.radius.clear();
         sc->c0.concentration.clear();

         // Sets concentration for this component to -1 to signal that we are
         // using a concentration vector

         le_conc->setText("from file");

         double val1;
         double val2;

         while (!ts.atEnd()) {
            ts >> val1;
            ts >> val2;

            if (val1 > 0.0) // ignore radius pairs that aren't positive
            {
               sc->c0.radius.push_back(val1);
               sc->c0.concentration.push_back(val2);
            }
         }

         f.close();
      }
      else {
         QMessageBox::warning(
            this, tr("UltraScan Warning"),
            tr("Please note:\n\n"
               "UltraScan could not open the file specified\n")
               + fn);
      }
   }
}

void US_ModelEditor::load_model() {
   QString fn = QFileDialog::getOpenFileName(
      this, tr("Load Model File"), US_Settings::resultDir(), "*.model.?? *.model-?.??  *model-??.??");

   if (!fn.isEmpty()) {
      int code = US_FemGlobal::read_modelSystem(model, fn);

      if (code == 0) {
         // Successfully loaded a new model
         QMessageBox::information(this, tr("Simulation Module"), tr("Successfully loaded Model:"));

         lb_header->setText(US_Constants::modelStrings()[ model.model ]);

         cmb_component1->disconnect();
         cmb_component1->clear();

         vector<struct SimulationComponent> *scl = &model.component_vector;

         for (uint i = 0; i < (*scl).size(); i++) {
            cmb_component1->addItem((*scl)[ i ].name);
         }
         cmb_component1->setCurrentIndex(0);
         connect(cmb_component1, SIGNAL(currentIndexChanged(int)), SLOT(change_component1(int)));


         component = 0;
         sb_count->setRange(1, (*scl).size());
         sb_count->setValue(1); // Resets screen

         //select_component((int) 0);
      }

      else if (code == -40) {
         QMessageBox::warning(
            this, tr("UltraScan Warning"),
            tr("Please note:\n\n"
               "UltraScan could not open\n"
               "the selected Model File!"));
      }

      else if (code == 1) {
         QMessageBox::warning(
            this, tr("UltraScan Warning"),
            tr("Sorry, the old-style models are no longer supported.\n\n"
               "Please load a different model or create a new Model"));
      }

      else if (code < 0 && code > -40) {
         QMessageBox::warning(
            this, tr("UltraScan Warning"),
            tr("Please note:\n\n"
               "There was an error reading\n"
               "the selected Model File!\n\n"
               "This file appears to be corrupted"));
      }
   }
}

void US_ModelEditor::save_model(void) {
   if (!verify_model())
      return;

   QString fn = QFileDialog::getSaveFileName(
      this, tr("Save model as"), US_Settings::resultDir(), "*.model.?? *.model-?.?? *model-??.??");

   if (!fn.isEmpty()) {
      int k = fn.lastIndexOf(".");

      // If an extension was given, strip it.
      if (k != -1)
         fn.resize(k);

      fn += ".model-" + QString::number(model.model) + ".11";

      QFile f(fn);

      if (f.exists()) {
         int answer = QMessageBox::question(
            this, tr("Warning"),
            tr("Attention:\n"
               "This file exists already!\n\n"
               "Do you want to overwrite it?"),
            QMessageBox::Yes, QMessageBox::No);

         if (answer == QMessageBox::No)
            return;
      }

      int result = US_FemGlobal::write_modelSystem(model, fn);
      if (result != 0)
         error(tr("Could not write file."));
   }
}

void US_ModelEditor::accept_model(void) {
   if (verify_model())
      accept();
}

// The next five methods are virtually identical.  It would be nice
// to combine them.  It would require a custom class and signal.
void US_ModelEditor::update_sed(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->s = text.toDouble();
}

void US_ModelEditor::update_diff(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->D = text.toDouble();
}

void US_ModelEditor::update_extinction(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->extinction = text.toDouble();
}

void US_ModelEditor::update_sigma(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->sigma = text.toDouble();
}

void US_ModelEditor::update_delta(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->delta = text.toDouble();
}

// The next two methods are virtually identical.  It would be nice
// to combine them.
void US_ModelEditor::update_vbar(const QString &text) {
   struct SimulationComponent *sc = &model.component_vector[ component ];
   double vbar = text.toDouble();

   if (vbar <= 0.0) {
      error(tr("The vbar value must be greater than 0.0"));
      le_vbar->setText(QString::number(sc->vbar20, 'e', 4));
      return;
   }

   sc->vbar20 = vbar;
}

void US_ModelEditor::update_mw(const QString &text) {
   struct SimulationComponent *sc = &model.component_vector[ component ];
   double mw = text.toDouble();

   if (mw <= 0.0) {
      error(tr("The Molecular weight must be greater than 0.0"));
      le_mw->setText(QString::number(sc->mw, 'e', 4));
      return;
   }

   sc->mw = mw;
}

void US_ModelEditor::update_f_f0(const QString &text) {
   if (text == "")
      return;

   struct SimulationComponent *sc = &model.component_vector[ component ];
   sc->f_f0 = text.toDouble();
}

void US_ModelEditor::update_conc(const QString &text) {
   struct SimulationComponent *sc = &model.component_vector[ component ];

   if (text == "")
      return;

   if (text == "from file")
      sc->concentration = -1.0;
   else {
      bool ok;
      double concentration;
      concentration = text.toDouble(&ok);
      if (ok) {
         sc->concentration = concentration;
         sc->c0.radius.clear();
         sc->c0.concentration.clear();

         le_c0->setText("");
      }
      else
         error(tr("The Partial Concentration field is invalid."));
   }
}

// The next two methods are virtually identical.  It would be nice
// to combine them
void US_ModelEditor::update_keq(const QString &text) {
   if (text == "")
      return;

   vector<struct Association> *av = &model.assoc_vector;

   // Check to see if the current component is a dissociation component

   for (uint i = 0; i < (*av).size(); i++) {
      struct Association *as = &model.assoc_vector[ i ];

      // Check to see if there is an dissociation linked to this component
      if (as->component2 == component) {
         // Check to make sure this component is not an irreversible component
         if (
            as->stoichiometry1 != as->stoichiometry2 // Self-association
            || (as->stoichiometry1 == 0 && as->stoichiometry2 == 0)) // Hetero-association
         {
            as->keq = text.toDouble();
         }
      }
   }
}

void US_ModelEditor::update_koff(const QString &text) {
   if (text == "")
      return;

   vector<struct Association> *av = &model.assoc_vector;

   // Check to see if the current component is a dissociation component
   for (uint i = 0; i < (*av).size(); i++) {
      struct Association *as = &model.assoc_vector[ i ];

      // Check to see if there is an dissociation linked to this component
      if (as->component2 == component) {
         // Check to make sure this component is not an irreversible component
         if (
            as->stoichiometry1 != as->stoichiometry2 // Self-association
            || (as->stoichiometry1 == 0 && as->stoichiometry2 == 0)) // Hetero-association
         {
            as->k_off = text.toDouble();
         }
      }
   }
}

void US_ModelEditor::error(const QString &message) {
   QMessageBox::warning(this, tr("Model Error"), message);
}

bool US_ModelEditor::verify_model(void) {
   bool flag = true;

   QString str;

   vector<struct Association> *av = &model.assoc_vector;
   vector<struct SimulationComponent> *cv = &model.component_vector;

   for (uint i = 0; i < (*av).size(); i++) {
      struct Association *as = &model.assoc_vector[ i ];

      // See if we need to check if the MWs match
      if (as->stoichiometry2 > 0 && as->stoichiometry3 != 1) {
         if (
            fabs((*cv)[ as->component2 ].mw - ((*cv)[ as->component1 ].mw * as->stoichiometry2 / as->stoichiometry1))
            > 1.0) // MWs don't match within 1 dalton
         {
            str = tr("The molecular weights of the reacting species\n"
                     "in reaction ")
                  + QString::number(i + 1) + tr("do not agree:\n\n")

                  + tr("Molecular weight of species 1: ") + QString::number((*cv)[ as->component1 ].mw, 'e', 4) + "\n"

                  + tr("Molecular weight of species 2:") + QString::number((*cv)[ as->component2 ].mw, 'e', 4) + "\n"

                  + tr("Stoichiometry of reaction ") + QString::number(i + 1) + tr(": MW(1) *")
                  + QString::number(as->stoichiometry2) + tr(" = MW(2)\n\n")

                  + tr("Please adjust either MW(1) or MW(2) before proceeding...");

            QMessageBox::warning(this, tr("Model Definition Error"), str);
            flag = false;
         }
      }

      // See if we need to check if the sum of MW(1) + MW(2) = MW(3)
      if (as->stoichiometry3 == 1) {
         if (
            fabs(
               model.component_vector[ as->component3 ].mw - model.component_vector[ as->component2 ].mw
               - model.component_vector[ as->component1 ].mw)
            > 1.0)
         // MWs don't match within 10 dalton
         {
            str = tr("The molecular weights of the reacting species\n"
                     "in reaction ")
                  + QString::number(i + 1) + tr("do not agree:\n\n")

                  + tr("Molecular weight of species 1: ") + QString::number((*cv)[ as->component1 ].mw, 'e', 4) + "\n"

                  + tr("Molecular weight of species 2: ") + QString::number((*cv)[ as->component2 ].mw, 'e', 4) + "\n"

                  + tr("Molecular weight of species 3: ") + QString::number((*cv)[ as->component3 ].mw, 'e', 4) + "\n"

                  + tr("Stoichiometry of reaction ") + QString::number(i + 1) + tr(": MW(1) + MW(2) = MW(3)\n\n")

                  + tr("Please adjust the molecular weight of the appropriate\n"
                       "component before proceeding...");

            QMessageBox::warning(this, tr("Model Definition Error"), str);
            flag = false;
         }
      }
   }

   return (flag);
}
