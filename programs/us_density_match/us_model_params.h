//! \file us_model_params.h
#ifndef US_MDLPARAM_H
#define US_MDLPARAM_H

#include "us_density_match.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_widgets_dialog.h"

//! \brief A class to provide a window for setting parameters for each model

class US_ModelParams : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \brief US_ModelParams constructor
      //! \param adistros Pointer to model distributions list
      //! \param p        Pointer to the parent of this widget
      US_ModelParams(QVector<DisSys> &, QWidget *p = 0);

   private:
      QVector<DisSys> &distros; // Reference to model distributions vector

      int nd_orig; // Number of distributions in original
      int dbg_level;

      QGridLayout *mainLayout; //

      QList<QLabel *> labels;
      QList<double> d2opcts;
      QList<double> bdensts;
      QStringList modelids;
      QStringList mlabels;
      QStringList mdescrs;

      QStringList mdesc_orig; // List of descriptions from original
      QStringList mdesc_list; // List of descriptions now listed

      QPushButton *pb_help; // Help    button
      QPushButton *pb_cancel; // Cancel  button
      QPushButton *pb_accept; // Accept  button
      QPushButton *pb_compute; // Compute button

   protected:
      US_Help showHelp;

   private slots:

      void canceled(void);
      void accepted(void);
      void compute_densities(void);
      bool values_filled(void);
      bool all_percents(void);
      void lnedChanged(const QString &);
      void help(void) { showHelp.show_help("dens_match_mparam.html"); };
};
#endif
