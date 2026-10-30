//Added by qt3to4:
#include <QCloseEvent>
#include <QFrame>
#include <QLabel>
#ifndef US_HYDRODYN_XSR
#define US_HYDRODYN_XSR


#include "us_saxs_util.h"
#include "us_hydrodyn.h"
#include "us_hydrodyn_saxs.h"

class US_EXTERN  US_Hydrodyn_Xsr : public QFrame
{
   Q_OBJECT

      friend class US_Hydrodyn_Saxs;

   public:
      US_Hydrodyn_Xsr( 
                      US_Hydrodyn *us_hydrodyn,
                      QWidget     *p = 0,
                      const char   *name = 0
                      );
      ~US_Hydrodyn_Xsr();

   private:    

      US_Config               *USglobal;
      
      QLabel                  *lbl_title;

      QPushButton             *pb_start;
      QProgressBar            *progress;
      QPushButton             *pb_stop;

      QFont                   ft;
      QTextEdit               *editor;
      QMenuBar                *m;

      QPushButton             *pb_help;
      QPushButton             *pb_cancel;

      void                    editor_msg( QString color, QString msg );

      QString                 filename;
      saxs_options *          our_saxs_options;
      US_Hydrodyn *           us_hydrodyn;
      US_Hydrodyn_Saxs *      saxs_window;
      bool *                  saxs_widget;
      
      bool                    keep_files;

      bool                    running;
      
#ifdef WIN32
#endif

      vector < US_Saxs_Scan > data;

      bool    run(
                  QString                 out_filename,
                  vector < US_Saxs_Scan > data,
                  bool                    keep_files = false
                  );
#ifdef WIN32
#endif


      QString error_msg;
      
   private slots:

      void setupGUI();

      void start();
      void stop();

      void clear_display();
      void update_font();
      void save();

      void cancel();
      void help();

      void update_enables();

   protected slots:

      void closeEvent(QCloseEvent *);

};

#endif
