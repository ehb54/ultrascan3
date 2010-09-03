#ifndef US_DATA_MODEL_H
#define US_DATA_MODEL_H

#include <QtGui>

#include "us_extern.h"
#include "us_widgets.h"
#include "us_db2.h"
#include "us_model.h"
#include "us_noise.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_help.h"
#include "us_dataIO2.h"

class US_EXTERN US_DataModel : public QObject
{
   Q_OBJECT

   public:
      US_DataModel( QWidget* = 0 );

      enum State { NOSTAT=0,  REC_DB=1,  REC_LO=2, PAR_DB=4, PAR_LO=8,
                   HV_DET=16, IS_CON=32, ALL_OK=64 };

      class DataDesc
      {
         public:
         int       recordID;          // record DB Identifier
         int       recType;           // record type (1-4)=Raw/Edit/Model/Noise
         int       parentID;          // parent's DB Identifier
         int       recState;          // record state flag
         QString   subType;           // sub-type (e.g., TI,RI for noises)
         QString   dataGUID;          // this record data Global Identifier
         QString   parentGUID;        // parent's GUID
         QString   filename;          // file name if on local disk
         QString   contents;          // md5sum() and length of data
         QString   label;             // record identifying label
         QString   description;       // record description string
         QString   lastmodDate;       // last modification date/time
      };

      void      setDatabase( US_DB2*,         QString      );
      void      setProgress( QProgressBar*,   QLabel*      );
      void      setSiblings( QObject*,        QObject*     );
      US_DB2*       dbase   (      void );
      QString       invtext (      void );
      QProgressBar* progrBar(      void );
      QLabel*       statlab (      void );
      QObject*      procobj (      void );
      QObject*      treeobj (      void );
      void      browse_data(       void );
      void      scan_data(         bool );
      void      dummy_data(        void );
      void      set_investigator(  QString );
      QString   investigator_text( void );
      DataDesc  row_datadesc(      int  );
      DataDesc  current_datadesc(  void );
      void      setCurrent(        int  );
      int       recCount(          void );
      int       recCountDB(        void );
      int       recCountLoc(       void );

   private:
      US_DB2*       db;               // pointer to opened DB connection
      QString       investig;         // investigator text
      QProgressBar* progress;         // progress bar on main window
      QLabel*       lb_status;        // status label on main window
      QWidget*      parentw;          // parent widget (main window)

      QTreeWidget*        tw_recs;    // tree widget
      QTreeWidgetItem*    tw_item;    // current tree widget item

      DataDesc            cdesc;      // current record description
      QVector< DataDesc > ddescs;     // DB descriptions
      QVector< DataDesc > ldescs;     // local-disk descriptions
      QVector< DataDesc > adescs;     // all (merged) descriptions

      QObject*            ob_process; // data processor
      QObject*            ob_tree;    // data tree handler
      QObject*            ob_exper;   // experiment synchronizer

      US_Buffer     buffer;
      US_Analyte    analyte;
 

      int           personID;

      bool          details;

      QString       invID;
      QString       run_name;
      QString       cell;

      QPoint        cur_pos;

   private slots:

      void scan_dbase(     void );
      void scan_local(     void );
      void merge_dblocal(  void );

      void sort_descs(     QVector< DataDesc >& descs );
      bool review_descs(   QStringList&, QVector< DataDesc >& );

      int         index_substring(  QString, int, QStringList& );
      QStringList filter_substring( QString, int, QStringList& );
      QStringList list_orphans(     QStringList&, QStringList& );

      int         record_state_flag( DataDesc, int );
      QString     sort_string(       DataDesc, int );
      QString     model_type(        int, int, int );
      QString     model_type(        US_Model      );
      QString     model_type(        QString       );
      QString     expGUIDauc(        QString       );

};
#endif
