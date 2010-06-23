//! \file us_model_gui.h
#ifndef US_MODEL_GUI_H
#define US_MODEL_GUI_H

#include <QtGui>

#include "us_widgets.h"
#include "us_model.h"
#include "us_help.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"

//! \brief A window to edit a model for simulation

class US_ModelGui : public US_Widgets
{
   Q_OBJECT

   public:
      //! \param current_model - Location of model structure for editing
      US_ModelGui( US_Model& );

      enum optics_t { ABSORBANCE, INTERFERENCE, FLUORESCENCE };

   signals:
      
      //! \brief A signal that retruns the model loaded or created
      //! \param model - The parameters of the model
      void valueChanged( US_Model );

   private:
      int       investigator;
      int       recent_row;
      bool      newFile;
      US_Model  model;
      US_Model  working_model;
      US_Buffer buffer;
      US_Help   showhelp;
                    
      class ModelDesc
      {
         public:
         QString description;
         QString filename;
         QString guid;
         QString DB_id;
         QString editguid;
      };

      QList< ModelDesc > model_descriptions;

      QLineEdit*     le_description;              
      QLineEdit*     le_investigator;
      QLineEdit*     le_buffer;
      QLineEdit*     le_density;
      QLineEdit*     le_viscosity;
      QLineEdit*     le_compressibility;
      QLineEdit*     le_wavelength;
      QLineEdit*     le_temperature;
      QLineEdit*     le_guid;

      US_ListWidget* lw_models;

      QRadioButton*  rb_db;
      QRadioButton*  rb_disk;

      QComboBox*     cb_optics;
                    
      bool    verify_model   ( void );
      bool    status_query   ( const QStringList& );
      bool    database_ok    ( US_DB2& );
      bool    ignore_changes ( void );
      void    show_model_desc( void );
      void    connect_error  ( const QString& );
      QString get_filename   ( const QString&, const QString& );

   private slots:
      void manage_components ( void );
      void list_models       ( void );
      void select_model      ( QListWidgetItem* );
      void save_model        ( void );
      void accept_model      ( void );
      void update_sim        ( void );
      void edit_description  ( void );
      void get_person        ( void );
      void update_person     ( int, const QString&, const QString& );
      void get_buffer        ( void );
      void update_buffer     ( US_Buffer );
      void new_model         ( void );
      void delete_model      ( void );
      void associations      ( void );
      void update_assoc      ( void );

      void help( void ){ showhelp.show_help( "model_editor.html" ); };
};
#endif
