//! \file us_selectmodel3.h
#ifndef US_SELECTMODEL3_H
#define US_SELECTMODEL3_H

#include <QtGui>
#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_help.h"

//! \brief A window to get additioanl information when selecting a model
//!        with a fixed molecular weight distribution
class US_EXTERN US_SelectModel3 : public US_WidgetsDialog
{
   Q_OBJECT
   
   public:
      //! \param temp_mw_upperLimit - Location for mw upper limit
      //! \param temp_mw_lowerLimit - Location for mw lower limit
      //! \param temp_mw_slots      - Location for mw slots
      //! \param temp_model3_vbar   - Location for vbar setting
      //! \param temp_model3_vbar_flag - Location for flag indicating
      //!                               validity of vbar setting
      //! \param temp_InvID         - Investigator ID  
      //! \param p                  - Parent widget, normally unspecified
      //! \param f                  - Window flags, normally not specified
      US_SelectModel3( float&, float&, int&, float&, bool&, int,  
            QWidget* = 0, Qt::WindowFlags = 0 );

      //! \param temp_mw_slots      - Location for mw slots
      //! \param p                  - Parent widget, normally unspecified
      //! \param f                  - Window flags, normally not specified
      US_SelectModel3( int&, QWidget* = 0, Qt::WindowFlags = 0 );

   private:
      float& mwUpperLimit;
      float& mwLowerLimit;
      int&   mwSlots;
      bool&  model3VbarFlag;
      float& model3Vbar;

      int    InvID;

      QLineEdit* le_mwSlots;
      QLineEdit* le_mwLowerLimit;
      QLineEdit* le_mwUpperLimit;
      QLineEdit* le_vbar;

      QCheckBox* cb_commonVbar;

      US_Help    showhelp;

      //US_Vbar_DB* vbar_dlg;

   private slots:
      void help     ( void ){ showhelp.show_help( "manual/equil2.html" ); };
      void ok       ( void );
      void ok2      ( void );
      void cancel   ( void );
      void read_vbar( void );
};
#endif

