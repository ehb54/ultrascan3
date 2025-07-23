//! \file us_choice.h
#ifndef US_CHOICE_H
#define US_CHOICE_H

#include "us_widgets_dialog.h"
#include "us_extern.h"
#include "us_solution.h"

//! A class to provide a choice of values.  It is 
//! generally used for selecting one analyte out of a solution with
//! multiple analytes.  

class US_GUI_EXTERN US_Choice : public US_WidgetsDialog
{
   Q_OBJECT

	public:
      //! \brief Constructor of a Choice dialog to act on a solution.
      //! \param solution The solution for which an analyte choice is made.
      US_Choice( const US_Solution& );

   signals:
      //! Return the user's choice
      void choice( int value );

   private:
      QList< QRadioButton* > radios;

   private slots:
      void done( void );
};
#endif

