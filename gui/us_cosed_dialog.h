//! \file us_cosed_dialog.h

#ifndef US_COSED_DIALOG_H
#define US_COSED_DIALOG_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_plot.h"
#include "us_help.h"
#include "us_buffer.h"

//! \brief A class for requesting additional information for cosedimenting components

class US_GUI_EXTERN US_BaseBufferRequester : public US_WidgetsDialog {
Q_OBJECT

public:
   US_BaseBufferRequester(QMap<QString, US_CosedComponent> &base_comps_, double* dens, double* visc);

private:
   double* density;
   double* viscosity;
   QGridLayout *main;
   QMap<QString, US_CosedComponent> base_comps;

   QLabel *lb_description;
   QLabel *lb_density;
   QLabel *lb_viscosity;
   QLineEdit *le_density;
   QLineEdit *le_viscosity;
   QPushButton *pb_accept;

private slots:

   void cancelled(void);

   void accept(void);

   void edit(void);
};

class US_GUI_EXTERN US_CosedComponentRequester : public US_WidgetsDialog {
Q_OBJECT

public:
   US_CosedComponentRequester(QMap<QString, US_CosedComponent> &base_comps_, QList<US_CosedComponent> &excess_comps_,
                              US_CosedComponent* comp_);

private:
   QGridLayout* main;
   QMap<QString, US_CosedComponent> base_comps;
   QList<US_CosedComponent> excess_comps;
   US_CosedComponent* comp;

   QLabel *lb_description;
   QLabel *lb_dens;
   QLabel *lb_visc;
   QLabel *lb_density1;
   QLabel *lb_viscosity1;
   QLineEdit *le_density1;
   QLineEdit *le_viscosity1;
   QLabel *lb_density2;
   QLabel *lb_viscosity2;
   QLineEdit *le_density2;
   QLineEdit *le_viscosity2;
   QLabel *lb_density3;
   QLabel *lb_viscosity3;
   QLineEdit *le_density3;
   QLineEdit *le_viscosity3;
   QLabel *lb_density4;
   QLabel *lb_viscosity4;
   QLineEdit *le_density4;
   QLineEdit *le_viscosity4;
   QLabel *lb_density5;
   QLabel *lb_viscosity5;
   QLineEdit *le_density5;
   QLineEdit *le_viscosity5;
   QLabel *lb_sedimentation;
   QLabel *lb_diffusion;
   QLabel *lb_vbar;
   QLineEdit *le_vbar;
   QLineEdit *le_sedimentation;
   QLineEdit *le_diffusion;
   QPushButton *pb_accept;

private slots:

   void cancelled(void);

   void accept(void);

   void edit(void);
};

class US_GUI_EXTERN US_LowerCosedComponentRequester : public US_WidgetsDialog {
Q_OBJECT

public:
   US_LowerCosedComponentRequester(QList<US_CosedComponent> &excess_comps_,
                              US_CosedComponent* comp_);

private:
   QGridLayout* main;
   QMap<QString, US_CosedComponent> base_comps;
   QList<US_CosedComponent> excess_comps;
   US_CosedComponent* comp;

   QLabel *lb_description;
   QLabel *lb_dens;
   QLabel *lb_visc;
   QLabel *lb_density1;
   QLabel *lb_viscosity1;
   QLineEdit *le_density1;
   QLineEdit *le_viscosity1;
   QLabel *lb_density2;
   QLabel *lb_viscosity2;
   QLineEdit *le_density2;
   QLineEdit *le_viscosity2;
   QLabel *lb_density3;
   QLabel *lb_viscosity3;
   QLineEdit *le_density3;
   QLineEdit *le_viscosity3;
   QLabel *lb_density4;
   QLabel *lb_viscosity4;
   QLineEdit *le_density4;
   QLineEdit *le_viscosity4;
   QLabel *lb_density5;
   QLabel *lb_viscosity5;
   QLineEdit *le_density5;
   QLineEdit *le_viscosity5;
   QLabel *lb_sedimentation;
   QLabel *lb_diffusion;
   QLabel *lb_vbar;
   QLineEdit *le_vbar;
   QLineEdit *le_sedimentation;
   QLineEdit *le_diffusion;
   QPushButton *pb_accept;

private slots:

   void cancelled(void);

   void accept(void);

   void edit(void);
};


#endif //US_COSED_DIALOG_H
