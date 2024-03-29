//! \file us_associations_gui.h
#ifndef US_ASSOCIATIONS_GUI_H
#define US_ASSOCIATIONS_GUI_H

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_model.h"
#include "us_help.h"
#include "us_extern.h"

//! \brief A window to edit a model's associations

class US_GUI_EXTERN US_AssociationsGui : public US_WidgetsDialog
{
   Q_OBJECT
 
   public:
      //! \param current_model - Location of model structure for editing
      US_AssociationsGui( US_Model& );

   signals:
      
      //! \brief A signal that the dialog is complete
      void done( void );

   private:
      US_Model&      model;
      US_Help        showhelp;
      US_ListWidget* lw_analytes;
      QTableWidget*  tw;

      QFontMetrics*  fm;

      QPushButton* pb_accept;

      QList< QwtCounter* > keq;
      QList< QwtCounter* > koff;

      void new_row      ( void );
      void populate     ( void );
      void set_component( int, int, int );
      bool combine_reactants( US_Model::Association* );

   private slots:
      void changed      ( int, int );
      void del          ( int      );
      void complete     ( void     );
      void disable_gui   ( void );

      void help    ( void )
      { showhelp.show_help( "associations.html" ); };
};

//! \brief An overloaded QPushButton to allow an index to be set and a 
//! signal to be emitted with the index.  Allows the connected handler to 
//! tell which buttone was pushed.
class US_GUI_EXTERN US_PushButton : public QPushButton
{
  Q_OBJECT

public:
  //! Constructor for the pushbutton
  //! \param text Label for the pushbutton
  //! \param i Value to be used to identify the pushbutton
  US_PushButton( const QString&, int );
  //! \brief A utility to set the pushbutton's index
  //! \param i Value to be saved
  void setIndex( int i ){ index = i; }; 

signals:
  //! A signal to send with the index of the menu selected.
  void pushed( int );

private:
  int  index;

private slots:
  void mousePressEvent( QMouseEvent* );
};

#endif
