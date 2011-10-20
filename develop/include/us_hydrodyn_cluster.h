#ifndef US_HYDRODYN_CLUSTER_H
#define US_HYDRODYN_CLUSTER_H

// QT defs:

#include <qlabel.h>
#include <qstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qframe.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qvalidator.h>

#include "us_util.h"
#include "us_hydrodyn_pdbdefs.h"
#include "us_hydrodyn_batch.h"

//standard C and C++ defs:

using namespace std;

class US_EXTERN US_Hydrodyn_Cluster : public QDialog
{
   Q_OBJECT

      friend class US_Hydrodyn_Batch;
      friend class US_Hydrodyn;

   public:
      US_Hydrodyn_Cluster(
                          void *us_hydrodyn,
                          QWidget *p = 0, 
                          const char *name = 0
                          );
      ~US_Hydrodyn_Cluster();

    private:
      void          *us_hydrodyn;
      
      US_Config     *USglobal;

      QLabel        *lbl_title;

      QLabel        *lbl_target;
      QPushButton   *pb_set_target;
      QLineEdit     *le_target_file;

      QLabel        *lbl_no_of_jobs;
      QLineEdit     *le_no_of_jobs;

      QLabel        *lbl_output_name;
      QLineEdit     *le_output_name;

      QPushButton   *pb_create_pkg;
      QPushButton   *pb_load_results;

      QFont         ft;
      QTextEdit     *editor;
      QMenuBar      *m;

      QPushButton   *pb_help;
      QPushButton   *pb_cancel;
      
      QStringList   selected_files;

      US_Hydrodyn_Batch *batch_window;

      saxs_options  *our_saxs_options;

      void          editor_msg( QString color, QString msg );

      QString       pkg_dir;

      bool          copy_files_to_pkg_dir( QStringList &filenames );
      bool          remove_files( QStringList &filenames );
      QString       errormsg;

   private slots:

      void setupGUI();
   
      void set_target();

      void update_output_name( const QString & );

      void create_pkg();
      void load_results();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#endif
