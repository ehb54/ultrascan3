//! \file us_data_loader.h
#ifndef US_DATA_LOAD_H
#define US_DATA_LOAD_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets_dialog.h"
#include "us_widgets.h"
#include "us_dataIO2.h"
#include "us_db2.h"
#include "us_help.h"

//! \brief A dialog class to select and load data (edited or raw)
class US_EXTERN US_DataLoader : public US_WidgetsDialog
{
   Q_OBJECT

   public:

      //! \brief Main constructor for dialog to select edit or raw
      //!        data and load it
      //! \param editsel  Flag: load edit (true) or raw (false)
      //! \param late     Flag: present only latest if editsel=true
      //! \param local    Flag: default data source to local disk
      //! \param search   Default list search string (list filter)
      //! \param invtext  Default investigator search text or "USER"
      //! \param dbP      Database pointer to opened connection
      US_DataLoader( bool, bool, bool, QString, QString, US_DB2* );

      //! \brief Load the vector of raw data sets for selected runID
      //! \param rawList A reference to a vector of rawData objects
      //! \param triples A reference to a list of triples to fill
      //! \return        The count of objects returned
      int     load_raw(   QVector< US_DataIO2::RawData >&,
                          QStringList& );

      //! \brief Load the vector of edited data sets for selected edit
      //!        along with parallel AUC data sets
      //! \param dataList A reference to a vector of editedData objects
      //! \param rawList  A reference to a vector of rawData objects
      //! \param triples  A reference to a list of triples to fill
      //! \return         The count of objects returned
      int     load_edit(  QVector< US_DataIO2::EditedData >&,
                          QVector< US_DataIO2::RawData    >&,
                          QStringList& );

      //! \brief Get a description of the data last selected
      //! \return      QString that concatenates DataDesc information.
      //!              First character is separator.
      //!              Remaining fields in the string, available with 
      //!              QString::section(), are:
      //!                1,1 - Label string;
      //!                2,2 - Description string;
      //!                3,3 - db ID (-1 if from disk).
      //!                4,4 - Data file name
      //!                5,5 - GUID of data;
      //!                6,6 - GUID of related AUC;
      QString description(       void );

      //! \brief Get current settings to use as defaults in next call
      //! \param local   Reference to boolean: is local selected?
      //! \param invtext Reference to current investigator text.
      //! \param search  Reference to current search string.
      //! \return        Pointer to any opened database connection.
      US_DB2* settings( bool&, QString&, QString& );

   private:
      US_Help showHelp;

      class DataDesc   // description of each data set in the list presented
      {
         public:
         QString label;           // identifying label for choice list
         QString descript;        // full descriptive string
         QString filename;        // file name
         QString dataGUID;        // data Global Identifier
         QString aucGUID;         // AUC GUID (same as dataGUID for raw)
         int     DB_id;           // database ID number
         bool    isEdit;          // flag:  is this edit? False->raw
         bool    isLatest;        // flag:  are edits latest ones
      };

      QMap< QString, DataDesc >  datamap;  // map of labels,data-desc-objs

      DataDesc       ddesc;       // current selected data's descripton object

      QRadioButton*  rb_db;       // radio button: database load
      QRadioButton*  rb_disk;     // radio button: local disk load

      QPushButton*   pb_invest;   // investigator button
      QPushButton*   pb_filtdata; // data search filter button

      QLineEdit*     le_invest;   // investigator text entry
      QLineEdit*     le_dfilter;  // data search filter text entry

      US_ListWidget* lw_data;     // data list widget

      US_DB2*        db;          // pointer to opened DB connection

      bool           ldedit;      // current load-edits flag (F -> raw)
      bool           latest;      // current use-lastest-edit flag
      bool           ondisk;      // current from-disk flag (F -> DB)

      QString        dsearch;     // current data search string
      QString        dinvtext;    // current data investigator string

   private slots:
      void select_disk (    bool );
      void investigator(    void );
      void list_data   (    void );
      void cancelled(       void );
      void accepted(        void );
      int  scan_dbase_edit( void );
      int  scan_dbase_raw(  void );
      int  scan_local_edit( void );
      int  scan_local_raw(  void );
      int  pare_to_latest(  void );
      void show_data_info(  QPoint );
      void help  (          void )
      { showHelp.show_help( "data_loader.html" ); };


   protected:
      //! \brief Event filter to flag whether a mouse click is
      //!        with the right mouse button
      //! \param obj Pointer to object on which event occurred.
      //! \param e   Pointer to event to filter.
      //! \return    Flag if event has been completely handled.
      bool eventFilter(        QObject*, QEvent* );
};
#endif
