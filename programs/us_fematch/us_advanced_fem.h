//! \file us_advanced_fem.h
#ifndef US_ADVANCED_H
#define US_ADVANCED_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

#include <qwt_plot.h>

//! \brief A class to provide a window for advanced analysis controls

class US_AdvancedFem : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \brief US_AdvancedFem constructor
      //! \param amodel   Pointer to model
      //! \param adv_vals Reference to advanced values map
      //! \param p        Pointer to the parent of this widget
      US_AdvancedFem( US_Model*, QMap< QString, QString>&,
                      QWidget* p = 0 );

   private:
      US_Model*                 model;  //! \brief Pointer to model
      QMap< QString, QString >& parmap; //! \brief Reference to advanced dialog values

      QGridLayout*  mainLayout;    //! \brief Pointer to main layout

      QWidget*      parentw;       //! \brief Pointer to parent widget

      QwtCounter*   ct_simpoints;  //! \brief Number of simulation points
      QwtCounter*   ct_bandvolume; //! \brief Band-loading volume
      QwtCounter*   ct_parameter;  //! \brief Parameter

      QComboBox*    cb_mesh;      //! \brief Mesh type
      QComboBox*    cb_grid;      //! \brief Grid type

   protected:
      US_Help       showHelp;

   private slots:

      void done(           void );
      void help     ( void )
      { showHelp.show_help( "fe_match_adv.html" ); };
};
#endif

