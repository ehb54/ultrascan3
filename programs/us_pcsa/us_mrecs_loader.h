//! \file us_mrecs_loader.h
#ifndef US_MRECS_LOAD_H
#define US_MRECS_LOAD_H

#include "us_dataIO.h"
#include "us_extern.h"
#include "us_help.h"
#include "us_model.h"
#include "us_pcsa_modelrec.h"
#include "us_plot.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

#define VEC_MRECS QVector<US_ModelRecord>

//! \brief A dialog class to select a model and load its distribution data
class US_MrecsLoader : public US_WidgetsDialog {
      Q_OBJECT

   public:
      //! \brief Main constructor for dialog to select a model
      //!        records entry and load its mrecs data
      //! \param dbSrc    Flag: default models source to database
      //! \param search   List search filter string
      //! \param amrecs   Output loaded mrecs vector
      //! \param adescr   Output selected model description string
      //! \param eGUID    Edit GUID for filtering on Edit children
      //! \param arunID   Run ID for limiting local file directory
      US_MrecsLoader(bool, QString &, VEC_MRECS &, QString &, const QString = "", const QString = "");

   private:
      bool loadDB; // flag: true to load from DB; false=disk
      QString &dsearch; // reference to search string argument
      VEC_MRECS &omrecs; // reference to mrecs argument
      QString &odescr; // reference to description argument
      QString editGUID; // edit GUID to possibly match
      QString runID; // run ID to possibly limit local files
      US_DB2 *dbP; // pointer to a database connection

      US_Help showHelp;

      class MrecsDesc { // model description object
         public:
            QString description; // description string
            QString mrecsGUID; // mrecs GUID
            QString filename; // local file name
            QString DB_id; // database ID (-1 if local)
            QString editGUID; // edit GUID
            QString modelGUID; // model GUID
            int rec_index; // mrecs list index
      };

      QList<MrecsDesc> mrecs_descriptions; // listed mrecs entries
      QList<MrecsDesc> mrecs_descrs_recs; // mrecs list from records

      VEC_MRECS mrecs;
      US_ModelRecord mrec;
      QString mrdesc;

      QPushButton *pb_filtvmrecs;
      QPushButton *pb_delete;
      QPushButton *pb_details;
      QPushButton *pb_accept;

      QLineEdit *le_mfilter;

      QCheckBox *ck_edit;
      QCheckBox *ck_unasgn;

      US_ListWidget *lw_vmrecs;

      int db_id1;
      int db_id2;
      int sel_row;

      bool do_edit;
      bool do_unasgn;
      bool can_edit;

      QString reqGUID;

      QStringList mdescrs;

   private slots:
      //! \brief Load mrecs at a given index
      //! \param mrecs Mrecs to load from given index
      //! \param index Index in list of models to load (0,...,count-1)
      //! \return      status flag for load
      int load_mrecs(VEC_MRECS &, int);

      //! \brief Get a description of the mrecs at a given index
      //! \param index Index in list of mrecs entries loaded (0,...,count-1)
      //! \return      QString that concatenates MrecsDesc information.
      //!              First character is separator.
      //!              Remaining fields in the string, available with
      //!              QString::section(), are:
      //!                1,1 - Description string;
      //!                2,2 - Mrecs file name (empty if from db);
      //!                3,3 - GUID of mrecs;
      //!                4,4 - db ID (-1 if from disk).
      //!                5,5 - GUID of related Edit;
      QString concat_description(int);

      void list_vmrecs(void);
      void msearch(const QString &);
      void cancelled(void);
      void accepted(void);
      void change_edit(bool);
      void row_selected(int);
      void delete_mrecs(void);

      void help(void) { showHelp.show_help("load_modelrecs.html"); };

      int mrecsIndex(QString mdesc, QList<MrecsDesc>);
      void show_mrecs_info(void);
};
#endif
