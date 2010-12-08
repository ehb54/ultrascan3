//! \file us_model_loader.h
#ifndef US_MODEL_LOAD_H
#define US_MODEL_LOAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_model.h"
#include "us_plot.h"
#include "us_help.h"

/*! \brief A dialog class to select a model and load its distribution data
*/
class US_EXTERN US_ModelLoader : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      /*! \brief Main constructor for dialog to select a model
                 and load its distribution data
          \param multisel Flag: allow multiple model selections
          \param local    Flag: default models source to local disk
          \param search   Default list search string (list filter)
          \param invtext  Default investigator search text or "USER"
      */
      US_ModelLoader( bool, bool, QString, QString );

      /*! \brief Get count of models selected
          \return Count of number of models selected
      */
      int     models_count(     void );

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

      /*! \brief Get current list search filter (for use in next call).
          \return      QString current contents of list search line edit.
      */
      QString search_filter(     void );

      /*! \brief Get current investigator text (for use in next call).
          \return      QString current contents of investigator line edit.
      */
      QString investigator_text( void );

      /*! \brief Set Edit GUID for possible use in list filtering.
          \param guid Edit GUID string to set for possible filtering.
      */
      void    set_edit_guid(     QString );

   private:
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
         int     asd_index;     // all-single-descriptions index
      };

      QList< ModelDesc > model_descriptions; // listed models
      QList< ModelDesc > all_model_descrips; // all models/groups
      QList< ModelDesc > all_single_descrs;  // all single models

      US_Model           model;

      QRadioButton*  rb_db;
      QRadioButton*  rb_disk;

      QPushButton*   pb_investigator;
      QPushButton*   pb_filtmodels;

      QLineEdit*     le_investigator;
      QLineEdit*     le_mfilter;
      US_ListWidget* lw_models;

      int            modelsCount;
      int            db_id1;
      int            db_id2;

      bool           multi;
      bool           ondisk;

      QString        dsearch;
      QString        dinvtext;
      QString        editGUID;
      QString        reqGUID;

   private slots:
      void select_disk (  bool );
      void investigator(  void );
      void get_person(    void );
      void update_person( int, const QString&, const QString& );
      void list_models (  void );
      void compress_list( void );
      void dup_singles(   void );
      void cancelled(     void );
      void accepted(      void );
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
