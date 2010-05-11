//! \file us_model_editor_new2.h
#ifndef US_MODEL_EDITOR_H
#define US_MODEL_EDITOR_H

#include <QtGui>

#include "us_widgets.h"
#include "us_femglobal_new.h"
#include "us_help.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"

//! \brief A window to edit a model for simulation

class US_ModelEditor : public US_Widgets
{
	Q_OBJECT

	public:
      //! \param current_model - Location of model structure for editing
      //! \param p             - Parent widget, normally not specified
      //! \param f             - Window flags, normally not specified
		US_ModelEditor( US_FemGlobal_New::ModelSystem&, QWidget* = 0, 
            Qt::WindowFlags = 0 );

      enum optics_t { ABSORBANCE, INTERFERENCE, FLUORESCENCE };

   signals:
      
      //! \brief A signal that retruns the model loaded or created
      //! \param model - The parameters of the model
      void valueChanged( US_FemGlobal_New::ModelSystem );

   private:
      US_Help        showhelp;
                     
      int            prior_row;
      int            investigator;
                    
      class ModelDesc
      {
         public:
         QString description;
         QString filename;
         int     DB_id;
      };

      QList< ModelDesc > model_descriptions;

		QStringList                                      c0_files;
      QVector< US_FemGlobal_New::SimulationComponent > components;
      QVector< QMap< double, double >                > extinction_maps;
      QVector< QMap< double, double >                > refraction_maps;
      QVector< QMap< double, double >                > fluorescence_maps;
      QVector< US_Hydrosim                           > hydrosim_maps;
      QVector< bool                                  > hydrosim_valid;
      
      US_FemGlobal_New::ModelSystem                    model;
      US_FemGlobal_New::ModelSystem                    working_model;
      US_FemGlobal_New::SimulationComponent            local_component;
      US_Hydrosim                                      simcomp;
      US_Buffer                                        buffer;
      
      QLabel*        lb_coeff;

      QMap< double, double > extinction;

      QIcon          check;
                    
      QLineEdit*     le_description;              
		QLineEdit*     le_sed;
		QLineEdit*     le_diff;
		QLineEdit*     le_extinction;
		QLineEdit*     le_vbar;
		QLineEdit*     le_mw;
		QLineEdit*     le_f_f0;
		QLineEdit*     le_conc;
      QLineEdit*     le_coeff;
		QLineEdit*     le_c0;
		QLineEdit*     le_koff;
		QLineEdit*     le_keq;
		QLineEdit*     le_sigma;
		QLineEdit*     le_delta;
		QLineEdit*     le_stoich;
		QLineEdit*     le_investigator;
		QLineEdit*     le_buffer;
		QLineEdit*     le_density;
		QLineEdit*     le_viscosity;
		QLineEdit*     le_compressibility;
		QLineEdit*     le_temperature;


      QPushButton*   pb_load_c0;
                   
		US_ListWidget* lw_components;
		US_ListWidget* lw_models;

      QRadioButton*  rb_db;
      QRadioButton*  rb_disk;

      QwtCounter*    ct_wavelength;
                    
      QComboBox*     cb_shape;
      QComboBox*     cb_optics;
                    
      QCheckBox*     ck_coSed;
                    
		void show_component  ( void );

		//void update_sD       ( void );
      //bool verify_model    ( void );
      //void error           ( const QString& );
		//void update_component( void );
		void show_model_desc   ( void );

	private slots:

      void help( void ){ showhelp.show_help( "model_editor.html" ); };
      
		void manage_components ( void );
		void get_vbar          ( void );
      void update_analyte    ( US_Analyte );
      void change_optics     ( int  );
		void delete_component  ( void );
      void change_component  ( int  );

		void update_shape      ( void );
		//void simulate_component( void );

      void select_shape      ( int  );
		void load_c0           ( void );
		//void load_model        ( void );
		//void save_model        ( void );
		void accept_model      ( void );
		//void update_sim        ( US_Predict1::Hydrosim );
		void update_sim        ( void );

      void get_person        ( void );
      void update_person     ( int, const QString&, const QString& );

      void get_buffer        ( void );
      void update_buffer     ( US_Buffer );

		void new_model         ( void );
      //void update_sed        ( const QString& );
      //void update_diff       ( const QString& );
      //void update_extinction ( const QString& );
      //void update_vbar       ( const QString& );
      //void update_vbar       ( double );
      //void update_mw         ( const QString& );
      //void update_f_f0       ( const QString& );
      //void update_conc       ( const QString& );
      //void update_keq        ( const QString& );
      //void update_koff       ( const QString& );
      //void update_sigma      ( const QString& );
      //void update_delta      ( const QString& );
};
#endif
