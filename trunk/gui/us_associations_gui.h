//! \file associations_gui.h
#ifndef US_ASSOCIATIONS_GUI_H
#define US_ASSOCIATIONS_GUI_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_widgets.h"
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
      US_Model&      model;
      US_Help        showhelp;
      US_ListWidget* lw_analytes;
      QTableWidget*  tw;

      QFontMetrics*  fm;

      QList< QwtCounter* > keq;
      QList< QwtCounter* > koff;

      void new_row      ( void );
      void populate     ( void );
      void set_component( int, int, int );

   private slots:
      void changed      ( int, int );
      void del          ( int      );
      void complete     ( void     );

      void help    ( void )
      { showhelp.show_help( "model_editor.html#associations" ); };
};

class US_PushButton : public QPushButton
{
  Q_OBJECT

public:
  US_PushButton( const QString&, int );
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
