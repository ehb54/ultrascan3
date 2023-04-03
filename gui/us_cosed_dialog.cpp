//! \file us_cosed_dialog.cpp

#include "us_cosed_dialog.h"
#include "us_editor_gui.h"
#include "us_settings.h"
#include "us_gui_settings.h"
#include "us_constants.h"

#include <qwt_legend.h>

US_BaseBufferRequester::US_BaseBufferRequester(QMap<QString, US_CosedComponent> &base_comps_, double* dens, double* visc)
        : US_WidgetsDialog(nullptr, nullptr), density(dens), viscosity(visc), base_comps(base_comps_) {
   if (base_comps.count() == 0) {
      this->close();
   }
   setWindowTitle(tr("Cosedimenting Buffer Loader Dialog"));
   setPalette(US_GuiSettings::frameColor());

   main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);
   // construct the base buffer description
   QString buffer_list;
   for (int i = 0; i < base_comps.count() - 1; i++) {
      US_CosedComponent cosed = base_comps.values().value(i);
      if (i != 0) { buffer_list += ", "; }
      buffer_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }
   US_CosedComponent cosed = base_comps.last();
   if (base_comps.count() > 1) { buffer_list += " and "; }
   buffer_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";

   QString description_text = tr("The base of the buffer is a solution, containing ") + buffer_list + ".\n\n" +
                              tr("Please enter the density and viscosity of this solution.\n") +
                              tr("Click the \"Finish\" button to load the entered values.\n") +
                              tr("Click the \"Cancel\" button to exit this dialog.");
   lb_description = us_banner(description_text);
   lb_description->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   lb_density = us_label(tr("Density (20") + DEGC + tr(", g/cm<sup>3</sup>):"));
   lb_viscosity = us_label(tr("Viscosity (20") + DEGC + tr(", cP):"));
   le_density = us_lineedit("");
   le_viscosity = us_lineedit("");
   int row = 0;
   main->addWidget(lb_description, row++, 0, 1, 4);
   main->addWidget(lb_density, row, 0, 1, 2);
   main->addWidget(le_density,row++, 2, 1, 2);
   main->addWidget(lb_viscosity,row, 0, 1, 2);
   main->addWidget(le_viscosity,row++, 2, 1, 2);


   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   pb_accept = us_pushbutton(tr("Accept"));
   main->addWidget(pb_cancel, row, 0, 1,2);
   main->addWidget(pb_accept, row, 2, 1, 2);
   pb_accept->setEnabled(false);
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancelled()));
   connect(pb_accept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(le_density, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity, SIGNAL(editingFinished()), this, SLOT(edit()));
}

void US_BaseBufferRequester::cancelled(void) {
   this->close();
}

void US_BaseBufferRequester::accept(void) {
   *viscosity = le_viscosity->text().toDouble();
   *density = le_density->text().toDouble();
   qDebug() << "Set base buffer properties: density " << QString::number(*density, 'f', 6)
            << ", viscosity: " << QString::number(*viscosity, 'f', 5);
   this->close();
}

void US_BaseBufferRequester::edit(void) {
   if (!le_viscosity->text().isEmpty() && !le_density->text().isEmpty()) {
      pb_accept->setEnabled(true);
   } else {
      pb_accept->setEnabled(false);
   }
}

US_CosedComponentRequester::US_CosedComponentRequester(QMap<QString, US_CosedComponent> &base_comps_,
                                                       QList<US_CosedComponent> &excess_comps_,
                                                       US_CosedComponent* comp_) : US_WidgetsDialog(nullptr, nullptr),
                                                                                   base_comps(base_comps_),
                                                                                   excess_comps(excess_comps_),
                                                                                   comp(comp_) {
   if (base_comps.count() == 0 || excess_comps.count() == 0) {
      this->close();
   }
   setWindowTitle(tr("Cosedimenting Component Loader Dialog"));
   setPalette(US_GuiSettings::frameColor());

   main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);
   // construct the base buffer description
   QString buffer_list;
   for (int i = 0; i < base_comps.count() - 1; i++) {
      US_CosedComponent cosed = base_comps.values().value(i);
      if (i != 0) { buffer_list += ", "; }
      buffer_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }
   {
      US_CosedComponent cosed = base_comps.last();
      if (base_comps.count() > 1) { buffer_list += " and "; }
      buffer_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }


   // construct the excess buffer description
   QString excess_list;
   for (int i = 0; i < excess_comps.count() - 1; i++) {
      US_CosedComponent cosed = excess_comps.value(i);
      if (i != 0) { excess_list += ", "; }
      excess_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }
   {
      US_CosedComponent cosed = excess_comps.last();
      if (excess_comps.count() > 1) { excess_list += " and "; }
      excess_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }

   QString description_text =
           tr("The cosedimenting component ") + comp->name + tr(" is cosedimenting in a solution out of ") +
           excess_list + ".\n\n" +
           tr("Please enter the sedimentation and diffusion coefficient as well as the vbar for the component in this solution at 20") +
           DEGC + ".\n" +
           tr("Additionally the density and viscosity coefficients for this component as a mixture with ") +
           buffer_list + tr(" should be entered.\n\n") +
           tr("Click the \"Finish\" button to load the entered values.\n") +
           tr("Click the \"Cancel\" button to exit this dialog.");
   lb_description = us_banner(description_text);
   lb_description->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   lb_sedimentation = us_label(tr("Sedimentation coefficient (20") + DEGC + tr(", S):"));
   lb_vbar = us_label(tr("Vbar (20") + DEGC + tr(", cm<sup>3</sup>/g):"));
   lb_diffusion = us_label(tr("Diffusion Coefficient (20") + DEGC + tr(", cm<sup>2</sup>/s):"));
   lb_visc = us_banner("Viscosity Coefficients");
   lb_dens = us_banner("Density Coefficients");
   if (!std::isnan(comp->s_coeff)&&comp->s_coeff != 0.0){
      le_sedimentation = us_lineedit(QString::number(comp->s_coeff*1E13, 'f', 6));
   }
   else le_sedimentation = us_lineedit("");
   if (!std::isnan(comp->vbar)&&comp->vbar != 0.0){
      le_vbar = us_lineedit(QString::number(comp->vbar, 'f', 6));
   }
   else le_vbar = us_lineedit("");
   if (!std::isnan(comp->d_coeff)&&comp->d_coeff != 0.0){
      le_diffusion = us_lineedit(QString::number(comp->d_coeff*1E13, 'f', 6));
   }
   else le_diffusion = us_lineedit("");
   le_density1 = us_lineedit("");
   le_density2 = us_lineedit("");
   le_density3 = us_lineedit("");
   le_density4 = us_lineedit("");
   le_density5 = us_lineedit("");
   le_viscosity1 = us_lineedit("");
   le_viscosity2 = us_lineedit("");
   le_viscosity3 = us_lineedit("");
   le_viscosity4 = us_lineedit("");
   le_viscosity5 = us_lineedit("");

   lb_density1 = us_label(tr("Density coefficient 1"));
   lb_density2 = us_label(tr("Density coefficient 2"));
   lb_density3 = us_label(tr("Density coefficient 3"));
   lb_density4 = us_label(tr("Density coefficient 4"));
   lb_density5 = us_label(tr("Density coefficient 5"));
   lb_viscosity1 = us_label(tr("Viscosity coefficient 1"));
   lb_viscosity2 = us_label(tr("Viscosity coefficient 2"));
   lb_viscosity3 = us_label(tr("Viscosity coefficient 3"));
   lb_viscosity4 = us_label(tr("Viscosity coefficient 4"));
   lb_viscosity5 = us_label(tr("Viscosity coefficient 5"));

   int row = 0;
   main->addWidget(lb_description, row++, 0, 1, 4);
   main->addWidget(lb_sedimentation, row, 0, 1, 2);
   main->addWidget(le_sedimentation, row++, 2, 1, 2);
   main->addWidget(lb_diffusion, row, 0, 1, 2);
   main->addWidget(le_diffusion, row++, 2, 1, 2);
   main->addWidget(lb_vbar, row, 0, 1, 2);
   main->addWidget(le_vbar, row++, 2, 1, 2);
   main->addWidget(lb_dens, row++, 0, 1, 4);
   main->addWidget(lb_density1, row, 0, 1, 2);
   main->addWidget(le_density1, row++, 2, 1, 2);
   main->addWidget(lb_density2, row, 0, 1, 2);
   main->addWidget(le_density2, row++, 2, 1, 2);
   main->addWidget(lb_density3, row, 0, 1, 2);
   main->addWidget(le_density3, row++, 2, 1, 2);
   main->addWidget(lb_density4, row, 0, 1, 2);
   main->addWidget(le_density4, row++, 2, 1, 2);
   main->addWidget(lb_density5, row, 0, 1, 2);
   main->addWidget(le_density5, row++, 2, 1, 2);
   main->addWidget(lb_visc, row++, 0, 1, 4);
   main->addWidget(lb_viscosity1, row, 0, 1, 2);
   main->addWidget(le_viscosity1, row++, 2, 1, 2);
   main->addWidget(lb_viscosity2, row, 0, 1, 2);
   main->addWidget(le_viscosity2, row++, 2, 1, 2);
   main->addWidget(lb_viscosity3, row, 0, 1, 2);
   main->addWidget(le_viscosity3, row++, 2, 1, 2);
   main->addWidget(lb_viscosity4, row, 0, 1, 2);
   main->addWidget(le_viscosity4, row++, 2, 1, 2);
   main->addWidget(lb_viscosity5, row, 0, 1, 2);
   main->addWidget(le_viscosity5, row++, 2, 1, 2);


   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   pb_accept = us_pushbutton(tr("Accept"));
   main->addWidget(pb_cancel, row, 0, 1, 2);
   main->addWidget(pb_accept, row, 2, 1, 2);
   pb_accept->setEnabled(false);
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancelled()));
   connect(pb_accept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(le_sedimentation, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_vbar, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_diffusion, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_density1, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_density2, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_density3, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_density4, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_density5, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity1, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity2, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity3, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity4, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_viscosity5, SIGNAL(editingFinished()), this, SLOT(edit()));
}

void US_CosedComponentRequester::cancelled(void) {
   this->close();
}

void US_CosedComponentRequester::accept(void) {
   comp->s_coeff = le_sedimentation->text().toDouble() / 10E+13;
   comp->d_coeff = le_diffusion->text().toDouble();
   comp->vbar = le_vbar->text().toDouble();
   qDebug() << "Set cosed component properties: s" << QString::number(comp->s_coeff, 'f', 5)
            << ", D:" << QString::number(comp->d_coeff, 'f', 5) << ", vbar:" << QString::number(comp->vbar, 'f', 5) ;
   comp->dens_coeff[1] = le_density1->text().toDouble();
   comp->dens_coeff[2] = le_density2->text().toDouble();
   comp->dens_coeff[3] = le_density3->text().toDouble();
   comp->dens_coeff[4] = le_density4->text().toDouble();
   comp->dens_coeff[5] = le_density5->text().toDouble();
   comp->visc_coeff[1] = le_viscosity1->text().toDouble();
   comp->visc_coeff[2] = le_viscosity2->text().toDouble();
   comp->visc_coeff[3] = le_viscosity3->text().toDouble();
   comp->visc_coeff[4] = le_viscosity4->text().toDouble();
   comp->visc_coeff[5] = le_viscosity5->text().toDouble();
   this->close();
}

void US_CosedComponentRequester::edit(void) {
   if (le_diffusion->text().isEmpty() || le_sedimentation->text().isEmpty() || le_density1->text().isEmpty() ||
       le_density2->text().isEmpty() || le_density3->text().isEmpty() || le_density4->text().isEmpty() ||
       le_density5->text().isEmpty() || le_viscosity1->text().isEmpty() ||
       le_viscosity2->text().isEmpty() || le_viscosity3->text().isEmpty() || le_viscosity4->text().isEmpty() ||
       le_viscosity5->text().isEmpty() || le_vbar->text().isEmpty()) {
      pb_accept->setEnabled(false);
      return;
   }
   pb_accept->setEnabled(true);
}

US_LowerCosedComponentRequester::US_LowerCosedComponentRequester(QList<US_CosedComponent> &excess_comps_,
                                                       US_CosedComponent* comp_) : US_WidgetsDialog(nullptr, nullptr),
                                                                                   excess_comps(excess_comps_),
                                                                                   comp(comp_) {
   if (base_comps.count() == 0 || excess_comps.count() == 0) {
      this->close();
   }
   setWindowTitle(tr("Cosedimenting Component Loader Dialog"));
   setPalette(US_GuiSettings::frameColor());

   main = new QGridLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);


   // construct the excess buffer description
   QString excess_list;
   for (int i = 0; i < excess_comps.count() - 1; i++) {
      US_CosedComponent cosed = excess_comps.value(i);
      if (i != 0) { excess_list += ", "; }
      excess_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }
   {
      US_CosedComponent cosed = excess_comps.last();
      if (excess_comps.count() > 1) { excess_list += " and "; }
      excess_list += cosed.name + " (" + QString::number(cosed.conc * 1000, 'f', 3) + " mM)";
   }

   QString description_text =
           tr("The cosedimenting component ") + comp->name + tr(" is cosedimenting in a solution out of ") +
           excess_list + ".\n\n" +
           tr("Please enter the sedimentation and diffusion coefficient as well as the vbar for the component in this solution at 20") +
           DEGC + ". In case this component is neither diffusing nor sedimentating, just press \"Finish\"\n" +
           tr("Click the \"Finish\" button to load the entered values.\n") +
           tr("Click the \"Cancel\" button to exit this dialog.");
   lb_description = us_banner(description_text);
   lb_description->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
   lb_sedimentation = us_label(tr("Sedimentation coefficient (20") + DEGC + tr(", S):"));
   lb_vbar = us_label(tr("Vbar (20") + DEGC + tr(", cm<sup>3</sup>/g):"));
   lb_diffusion = us_label(tr("Diffusion Coefficient (20") + DEGC + tr(", cm<sup>2</sup>/s):"));
   le_vbar = us_lineedit("0.0");
   le_sedimentation = us_lineedit("0.0");
   le_diffusion = us_lineedit("0.0");

   int row = 0;
   main->addWidget(lb_description, row++, 0, 1, 4);
   main->addWidget(lb_sedimentation, row, 0, 1, 2);
   main->addWidget(le_sedimentation, row++, 2, 1, 2);
   main->addWidget(lb_diffusion, row, 0, 1, 2);
   main->addWidget(le_diffusion, row++, 2, 1, 2);
   main->addWidget(lb_vbar, row, 0, 1, 2);
   main->addWidget(le_vbar, row++, 2, 1, 2);


   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   pb_accept = us_pushbutton(tr("Finish"));
   main->addWidget(pb_cancel, row, 0, 1, 2);
   main->addWidget(pb_accept, row, 2, 1, 2);
   pb_accept->setEnabled(true);
   connect(pb_cancel, SIGNAL(clicked()), this, SLOT(cancelled()));
   connect(pb_accept, SIGNAL(clicked()), this, SLOT(accept()));
   connect(le_sedimentation, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_vbar, SIGNAL(editingFinished()), this, SLOT(edit()));
   connect(le_diffusion, SIGNAL(editingFinished()), this, SLOT(edit()));
}

void US_LowerCosedComponentRequester::cancelled(void) {
   this->close();
}

void US_LowerCosedComponentRequester::accept(void) {
   comp->s_coeff = le_sedimentation->text().toDouble() / 10E+13;
   comp->d_coeff = le_diffusion->text().toDouble();
   comp->vbar = le_vbar->text().toDouble();
   qDebug() << "Set cosed component properties: s" << QString::number(comp->s_coeff, 'f', 5)
            << ", D:" << QString::number(comp->d_coeff, 'f', 5) << ", vbar:" << QString::number(comp->vbar, 'f', 5) ;
   this->close();
}

void US_LowerCosedComponentRequester::edit(void) {
   if (le_diffusion->text().isEmpty() || le_sedimentation->text().isEmpty() || le_vbar->text().isEmpty()) {
      pb_accept->setEnabled(false);
      return;
   }
   pb_accept->setEnabled(true);
}
