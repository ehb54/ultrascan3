//! \file us_properties.h
#ifndef US_PROPERTIES_H
#define US_PROPERTIES_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_femglobal_new.h"
//#include "us_help.h"
#include "us_predict1.h"
#include "us_analyte.h"
#include "us_buffer.h"

//! \brief Determine analyte properties

class US_Properties : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      US_Properties( const US_Buffer&, 
                     const US_FemGlobal_New::ModelSystem&,
                     int,
                     bool );

   signals:
      void valueChanged( US_Predict1::Hydrosim );

   private:
      // Passed parameters
      US_Buffer                     buffer;
      US_FemGlobal_New::ModelSystem model;
      int                           investigator;
      bool                          db_access;

      int                     oldRow;
      US_Analyte::analyteData analyte;


      enum { MW, S, D, F, F_F0 } check_type;

      QListWidget* lw_components;

      US_Predict1::Hydrosim          data;
      US_Predict1::Hydrosim          working_data;

      QPalette   gray;
      QPalette   normal;

      QLineEdit* le_density;
      QLineEdit* le_viscosity;
      QLineEdit* le_vbar;
      QLineEdit* le_mw;
      QLineEdit* le_s;
      QLineEdit* le_D;
      QLineEdit* le_f;
      QLineEdit* le_f_f0;
      QLineEdit* le_temperature;
      QLineEdit* le_extinction;
      QLineEdit* le_wavelength;
      QLineEdit* le_analyteConc;
      QLineEdit* le_molar;
      QLineEdit* le_sigma;
      QLineEdit* le_delta;
      QLineEdit* le_GUID;

      QCheckBox* cb_mw;
      QCheckBox* cb_s;
      QCheckBox* cb_D;
      QCheckBox* cb_f;
      QCheckBox* cb_f_f0;
      QCheckBox* cb_co_sed;

      QComboBox* cmb_shape;

      void setInvalid ( void );
      int  countChecks( void );
      void update_lw  ( void );
      void enable     ( QLineEdit*, bool, const QPalette& );

	private slots:
      void newAnalyte  ( void );
      void edit_analyte( void );
      void getBuffer   ( void );
      void getAnalyte  ( void );
      void setBuffer   ( US_Buffer );
      void update      ( int );
      void setAnalyte  ( US_Analyte::analyteData );
      void calculate   ( void );
      void checkbox    ( int = 0 );
      void acceptProp  ( void );
      void simulate    ( void );
      void new_hydro   ( void );
      void lambda_up   ( void );
      void lambda_down ( void );
      void update_analyte( US_Analyte::analyteData );
};
#endif
