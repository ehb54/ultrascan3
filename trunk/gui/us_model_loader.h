//! \file us_model_loader.h
#ifndef US_MODEL_LOAD_H
#define US_MODEL_LOAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO.h"
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

   private:
      US_Help showHelp;

      class ModelDesc
      {
         public:
         QString description;
         QString filename;
         QString guid;
         QString DB_id;
      };

      QList< ModelDesc > model_descriptions;
      US_Model           model;

      QRadioButton*  rb_db;
      QRadioButton*  rb_disk;

      QPushButton*   pb_investigator;
      QPushButton*   pb_filtmodels;

      QLineEdit*     le_investigator;
      QLineEdit*     le_mfilter;
      US_ListWidget* lw_models;

      int            modelsCount;
      bool           multi;
      bool           ondisk;

      QString        dsearch;
      QString        dinvtext;

   private slots:
      void select_disk ( bool );
      void investigator( void );
      void list_models ( void );
      void cancelled(    void );
      void accepted(     void );
      void help  (       void )
      { showHelp.show_help( "load_model_distrib.html" ); };

      QString typeText(        US_Model::ModelType, int, int );
      int     modelIndex(      QString mdesc, QList< ModelDesc > );
      void    show_model_info( QPoint );

   protected:
      bool eventFilter(        QObject*, QEvent* );
};
#endif
