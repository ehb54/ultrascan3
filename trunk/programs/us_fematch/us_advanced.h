//! \file us_advanced.h
#ifndef US_ADVANCED_H
#define US_ADVANCED_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"
#include <qwt_plot.h>

//! \brief A class to provide a window for advanced analysis controls

class US_Advanced : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_Advanced constructor
      //! \param model Pointer to model
      //! \param       Pointer to the parent of this widget
      US_Advanced( US_Model*, QMap< QString, QString>&, QWidget* p = 0 );

   private:
      US_Model*                 model;
      QMap< QString, QString >& parmap;

      int           ncomp;

      QVBoxLayout*  mainLayout;
      QHBoxLayout*  upperLayout;
      QHBoxLayout*  lowerLayout;
      QGridLayout*  analysisLayout;
      QGridLayout*  modelcomLayout;

      QWidget*      parentw;

      QwtCounter*   ct_simpoints;
      QwtCounter*   ct_bldvolume;
      QwtCounter*   ct_parameter;
      QwtCounter*   ct_modelnbr;
      QwtCounter*   ct_component;

      QLineEdit*    le_sedcoeff;
      QLineEdit*    le_difcoeff;
      QLineEdit*    le_moweight;
      QLineEdit*    le_friratio;
      QLineEdit*    le_partconc;

      QGroupBox*    gb_modelsim;

      QRadioButton* rb_curmod;
      QRadioButton* rb_mode;
      QRadioButton* rb_mean;
      QRadioButton* rb_median;

      QComboBox*    cb_mesh;
      QComboBox*    cb_grid;

   protected:
      US_Help       showHelp;

   private slots:

      void done(           void );
      void next_component( void );
      void set_component(  double );
      void help     ( void )
      { showHelp.show_help( "fem_advanced.html" ); };
};
#endif

