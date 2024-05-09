//! \file us_model_gui.h
#ifndef US_MODEL_GUI_H
#define US_MODEL_GUI_H

#include "us_widgets.h"
#include "us_model.h"
#include "us_help.h"
#include "us_predict1.h"
#include "us_analyte_gui.h"
#include "us_buffer.h"
#include "us_extern.h"

//! \brief A window to edit a model for simulation

class US_GUI_EXTERN US_ModelGui : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      //! \param current_model - Location of model structure for editing
      US_ModelGui( US_Model& );

      //! The optical type of sensor in the experiment
      enum optics_t { ABSORBANCE, INTERFERENCE, FLUORESCENCE };

   signals:
      
      //! \brief A signal that returns the model loaded or created
      //! \param model - The parameters of the model
      void valueChanged( US_Model model );
      void disable_components_gui( void );

   private:
      int       investigator;
      int       recent_row;

      bool      newFile;
      bool      model_saved;

      US_Model  model;
      US_Model  working_model;
      US_Buffer buffer;
      US_Help   showhelp;
                    
      class US_GUI_EXTERN ModelDesc
      {
         public:
         QString description;
         QString filename;
         QString modelGUID;
         QString DB_id;
         QString editGUID;
      };

      QList< ModelDesc > model_descriptions;
      QString modelID_global;
      
      QLineEdit*     le_mlfilt;              
      QLineEdit*     le_nlines;              
      QLineEdit*     le_description;              
      QLineEdit*     le_investigator;
      QLineEdit*     le_wavelength;
      QLineEdit*     le_guid;

      QListWidget*   lw_models;

      QPushButton*   pb_save;
      QPushButton*   pb_delete;
      QPushButton*   pb_accept;

      QComboBox*     cb_optics;
 
      US_Disk_DB_Controls* dkdb_cntrls; //!< Radiobuttons for disk/db choice

      bool    verify_model   ( void );
      bool    status_query   ( const QStringList& );
      bool    database_ok    ( US_DB2& );
      bool    ignore_changes ( void );
      void    show_model_desc( void );
      void    connect_error  ( const QString& );

   private slots:
      void manage_components ( void );
      void list_models       ( void );
      void select_model      ( QListWidgetItem* );
      void save_model        ( void );
      void accept_model      ( void );
      void update_sim        ( void );
      void edit_description  ( void );
      void get_person        ( void );
      void check_db          ( void );
      void new_model         ( void );
      void delete_model      ( void );
      bool is_modelIDs_from_autoflow  ( QString );
      bool parse_models_desc_json( QStringList, QString );
      void associations      ( void );
      void update_assoc      ( void );
      void source_changed    ( bool );
      void filter_changed    ( void );
      void upd_disk_db       ( bool );
      int  modelIndex        ( QString, QList< ModelDesc > );

      void help( void ){ showhelp.show_help( "model_editor.html" ); };
};
#endif
