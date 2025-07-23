//! \file us_model_loader.h
#ifndef US_MODEL_LOAD_H
#define US_MODEL_LOAD_H

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

/*! \brief A dialog class to select a model and load its distribution data
*/
class US_GUI_EXTERN US_ModelLoader : public US_WidgetsDialog
{
   Q_OBJECT

   public:
      /*! \brief Main constructor for dialog to select a model
                 and load its distribution data
          \param dbSrc    Flag: default models source to database
          \param search   List search filter string
          \param amodel   Output loaded model
          \param adescr   Output selected model description string
          \param eGUID    Edit GUID for filtering on Edit children
      */
      US_ModelLoader( bool, QString&, US_Model&, QString&,
                      const QString = "" );

      /*! \brief Alternate constructor for dialog to select a model
                 and load its distribution data (with pre-filter)
          \param dbSrc    Flag: default models source to database
          \param search   List search filter string
          \param amodel   Output loaded model
          \param adescr   Output selected model description string
          \param arunIDs  List of pre-filter runIDs (ID-db/GUID-file)
      */
      US_ModelLoader( bool, QString&, US_Model&, QString&,
                      QStringList& );

      /*! \brief Alternate constructor for dialog to select models
                 and load their distribution data
          \param dbSrc    Flag: default models source to database
          \param search   List search filter string
          \param amodels  Output loaded models
          \param adescrs  Output selected model description strings
          \param arunIDs  List of pre-filter runIDs (ID-db/GUID-file)
      */
      US_ModelLoader( bool, QString&, QList< US_Model >&, QStringList&,
                      QStringList& );

      US_ModelLoader( bool, QString&, QList< US_Model >&, QStringList&,
                      QStringList&, QString );

      /*! \brief Alternate constructor for dialog to select models
                 and load their distribution data (no runIDs list)
          \param dbSrc    Flag: default models source to database
          \param search   List search filter string
          \param amodels  Output loaded models
          \param adescrs  Output selected model description strings
      */
      US_ModelLoader( bool, QString&, QList< US_Model >&, QStringList& );

      void accepted_auto(  QStringList );

   signals:
      //! \brief A signal to the parent that disk/db selection has changed
      //! \param DB True if DB source is selected
      void changed( bool DB );

   private:
      bool                 loadDB;    // flag: true to load from DB; false=disk
      QString&             dsearch;   // reference to search string argument
      US_Model&            omodel;    // reference to model argument
      QString&             odescr;    // reference to description argument
      QList< US_Model >&   omodels;   // reference to model list argument
      QStringList&         odescrs;   // reference to description list argument
      QStringList&         runIDs;    // input runIDs list argument
      bool                 multi;     // flag if multiple models can be selected
      QString              editGUID;  // edit GUID to possibly match
      US_DB2*              dbP;       // pointer to a database connection

      US_Help showHelp;

      class ModelDesc
      {  // model description object
         public:
         QString description;   // description string
         QString filename;      // local file name
         QString modelGUID;     // model GUID
         QString DB_id;         // database ID (-1 if local)
         QString editGUID;      // edit GUID
         QString reqGUID;       // request GUID
         int     iterations;    // Monte Carlo iterations
         int     rec_index;     // model record list index
      };

      QList< ModelDesc >   model_descriptions; // listed models
      QList< ModelDesc >   model_descrs_recs;  // models from records
      QList< ModelDesc >   model_descrs_sings; // models with singles
      QList< ModelDesc >   model_descrs_ufilt; // models list unfiltered

      US_Model             model;
      QList< US_Model >    wmodels;

      QRadioButton*        rb_db;
      QRadioButton*        rb_disk;

      QPushButton*         pb_investigator;
      QPushButton*         pb_filtmodels;

      QLineEdit*           le_investigator;
      QLineEdit*           le_mfilter;

      QCheckBox*           ck_single;
      QCheckBox*           ck_edit;
      QCheckBox*           ck_unasgn;
      QCheckBox*           ck_edlast;

      US_Disk_DB_Controls* dkdb_cntrls;

      US_ListWidget*       lw_models;

      int                  modelsCount;
      int                  db_id1;
      int                  db_id2;

      bool                 singprev;
      bool                 do_single;
      bool                 do_run;
      bool                 do_edit;
      bool                 do_unasgn;
      bool                 do_manual;
      bool                 do_edlast;
      bool                 can_run;
      bool                 can_edit;

      QString              reqGUID;

      QStringList          mdescrs;
      QStringList          wrunIDs;

   private slots:
      /*! \brief Load model at a given index
          \param model Model to load from given index
          \param index Index in list of models to load (0,...,count-1)
          \return      status flag for load
      */
      int     load_model( US_Model&, int  );

      /*! \brief Get a description of the model at a given index
          \param index Index in list of models loaded (0,...,count-1)
          \return      QString that concatenates ModelDesc information.
                       First character is separator.
                       Remaining fields in the string, available with 
                       QString::section(), are:
                         1,1 - Description string;
                         2,2 - Models file name (empty if from db);
                         3,3 - GUID of model;
                         4,4 - db ID (-1 if from disk).
                         5,5 - GUID of related Edit;
      */
      QString description(       int  );

      void build_dialog(  void );
      void build_dialog_auto( QString );
      
      void select_diskdb( void );
      void get_person(    void );
      void update_person( int  );
      
      void list_models (  void );
      void list_models_auto (  QString );
      
      void msearch(       const QString& );
      void records_list ( void );
      void singles_list ( void );
      void cancelled(     void );
      
      void accepted(      void );
      

      void change_single( bool );
      void change_edit  ( bool );
      void change_unasgn( bool );
      void change_edlast( bool );
      void select_edlast( void );
      QString alt_description( QString&, const bool );

      void help  (        void )
      { showHelp.show_help( "load_model_distrib.html" ); };

      int     modelIndex(      QString mdesc, QList< ModelDesc > );
      void    show_model_info( QPoint );

   protected:
      /*! \brief Event filter to flag whether a mouse click is
                 with the right mouse button
          \param obj      Pointer to object on which event occurred.
          \param e        Pointer to event to filter.
          \return         Flag if event has been completely handled.
      */
      bool eventFilter(        QObject*, QEvent* );
};
#endif
