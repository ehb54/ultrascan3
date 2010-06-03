//! \file associationsi_gui.h
#ifndef US_ASSOCIATIONS_GUI_H
#define US_ASSOCIATIONS_GUI_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_widgets.h"
//#include "us_femglobal_new.h"
#include "us_model.h"
#include "us_help.h"

//! \brief A window to edit a model's associations

class US_AssociationsGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param current_model - Location of model structure for editing
      US_AssociationsGui( US_Model& );

   signals:
      
      //! \brief A signal that the dialog is complete
      void done( void );

   private:
      US_Model       model;
      US_Help        showhelp;
      US_ListWidget* lw_analytes;

   private slots:
      void help( void )
      { showhelp.show_help( "model_editor.html#associations" ); };
};
#endif
