//! \file us_properties.h
#ifndef US_PROPERTIES_H
#define US_PROPERTIES_H

#include <QtGui>

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_model.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"
#include "us_model_gui.h"

//! \brief A class to manage analytes in a model

class US_Properties : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      //! Constructor.
      //! \param buf - The buffer to use in the analyte calculations
      //! \param mod - The model to use
      //! \param invID - The investigator ID in the database (-1 if not defined)
      //! \param access - A flag to determine if analyte data access should be
      //!          disk (false) or DB (true)
      US_Properties( const US_Buffer&, 
                     US_Model&,
                     int  = -1,
                     bool = false );
   signals:
      //! A signal that the class is complete
      //! \param hydro - this will probably change soon
      void valueChanged( US_Hydrosim hydro );

      //! A signal that the class is complete and the passed model reference 
      //! has been updated.
      void done( void );

   private:
      // Passed parameters
      US_Buffer  buffer;
      US_Model&  model;

      int        investigator;
      int        oldRow;

      bool       db_access;
      bool       inUpdate;
      bool       chgStoi;
                
      US_Analyte analyte;

      enum { MW, S, D, F, F_F0 }  check_type;

      US_Hydrosim  hydro_data;
      US_Hydrosim  working_data;

      QIcon        check;
                   
      QPalette     gray;
      QPalette     normal;

      QListWidget* lw_components;
      QPushButton* pb_load_c0;
      QComboBox*   cmb_shape;

      QLineEdit*   le_vbar;
      QLineEdit*   le_mw;
      QLineEdit*   le_s;
      QLineEdit*   le_D;
      QLineEdit*   le_f;
      QLineEdit*   le_f_f0;
      QLineEdit*   le_extinction;
      QLineEdit*   le_wavelength;
      QLineEdit*   le_analyteConc;
      QLineEdit*   le_molar;
      QLineEdit*   le_sigma;
      QLineEdit*   le_delta;
      QLineEdit*   le_guid;
      QLineEdit*   le_description;
                  
      QCheckBox*   cb_mw;
      QCheckBox*   cb_s;
      QCheckBox*   cb_D;
      QCheckBox*   cb_f;
      QCheckBox*   cb_f_f0;
      QCheckBox*   cb_co_sed;

      QwtCounter*  ct_stoich;

      void setInvalid    ( void );
      int  countChecks   ( void );
      void update_lw     ( void );
      void enable        ( QLineEdit*, bool, const QPalette& );
      void clear_guid    ( void );
      void clear_entries ( void );
      void save_changes  ( int );
      void checkbox      ( void );
      bool keep_standard ( void );

	private slots:
      void newAnalyte    ( void );
      void update        ( int );
      void update_analyte( US_Analyte );
      void calculate     ( void );
      void calculate     ( bool ) { calculate(); };
      void acceptProp    ( void );
      void simulate      ( void );
      void new_hydro     ( US_Analyte );
      void select_shape  ( int );
      void load_c0       ( void );
      void co_sed        ( int );
      void set_molar     ( void );
      void set_stoich    ( double );
           
      void del_component ( void );
      void edit_component( void );
      void edit_vbar     ( void );
};
#endif
