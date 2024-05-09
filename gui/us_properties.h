//! \file us_properties.h
#ifndef US_PROPERTIES_H
#define US_PROPERTIES_H

#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_model.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"
#include "us_model_gui.h"
#include "us_help.h"
#include "us_extern.h"

//! \brief A class to manage analytes in a model

class US_GUI_EXTERN US_Properties : public US_WidgetsDialog
{
	Q_OBJECT

	public:
      //! Constructor.
      //! \param mod - The model to use
      //! \param access - A flag to determine if analyte data access should be
      //!                 disk or DB
      US_Properties( US_Model&,
                     int = US_Disk_DB_Controls::Default );
   signals:
      //! A signal that the class is complete
      //! \param hydro - this will probably change soon
      void valueChanged( US_Hydrosim hydro );

      //! A signal that the class is complete and the passed model reference 
      //! has been updated.
      void done( void );

      //! A signal to indicate that the current disk/db selection has changed.
      //! /param DB True if DB is the new selection
      void use_db( bool DB );

   private:
      // Passed parameters
      US_Model&  model;

      US_Help    showhelp;

      int        investigator;
      int        oldRow;

      int        db_access;
      bool       inUpdate;
      bool       chgStoi;
                
      US_Analyte analyte;

      enum { MW, S, D, F, F_F0 }  check_type;

      US_Hydrosim  hydro_data;
      US_Hydrosim  working_data;

      QIcon        check;
                   
      QPalette     gray;
      QPalette     normal;

      QPushButton* pb_accept;
      
      QListWidget* lw_components;
      QPushButton* pb_load_c0;
      QComboBox*   cb_shape;

      QLineEdit*   le_vbar;
      QLineEdit*   le_mw;
      QLineEdit*   le_s;
      QLineEdit*   le_D;
      QLineEdit*   le_f;
      QLineEdit*   le_f_f0;
      QLineEdit*   le_extinction;
      QLineEdit*   le_wavelength;
      QLineEdit*   le_sigConc;
      QLineEdit*   le_molConc;
      QLineEdit*   le_sigma;
      QLineEdit*   le_delta;
      QLineEdit*   le_guid;
      QLineEdit*   le_description;
                  
      QCheckBox*   ck_isProd;
      QCheckBox*   ck_sigConc;
      QCheckBox*   ck_molConc;
      QCheckBox*   ck_mw;
      QCheckBox*   ck_s;
      QCheckBox*   ck_D;
      QCheckBox*   ck_f;
      QCheckBox*   ck_f_f0;
      QCheckBox*   ck_co_sed;

      QwtCounter*  ct_oligomer;

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
      void set_oligomer  ( double );
      void source_changed( bool );
      void check_molar   ( bool );
      void check_signal  ( bool );
      void disable_gui   ( void );
           
      void del_component ( void );
      void edit_component( void );
      void edit_vbar     ( void );
      void edit_analyte  ( void );
      void help          ( void )
      { showhelp.show_help( "components.html" ); };
};
#endif
