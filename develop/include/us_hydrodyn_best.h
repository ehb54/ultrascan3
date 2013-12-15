#ifndef US_HYDRODYN_BEST_H
#define US_HYDRODYN_BEST_H

#include "us_hydrodyn.h"
#include "us_hydrodyn_saxs.h"
#include "us_csv.h"

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Best : public QFrame
{
   Q_OBJECT

   public:
      US_Hydrodyn_Best(
                       void                     *              us_hydrodyn,
                       bool                     *              best_widget,
                       QWidget *                               p = 0,
                       const char *                            name = 0
                       );

      ~US_Hydrodyn_Best();

   private:

      US_Config *                             USglobal;

      // ------ input section 

      mQLabel *                               lbl_input;
      QListBox *                              lb_data;
      QPushButton *                           pb_load;
      QPushButton *                           pb_save_results;

      // ------ editor section 

      mQLabel               *                 lbl_editor;
      QFont                                   ft;
      QTextEdit             *                 editor;
      QMenuBar              *                 mb_editor;

      // ------ plot section
      QwtPlot               *                 plot_data;
      ScrollZoomer          *                 plot_data_zoomer;
#ifdef QT4
      QwtPlotGrid           *                 grid_data;
#endif
      QHBoxLayout           *                 hbl_points;
      vector < QCheckBox * >                  cb_points;

      // ------- bottom section

      QPushButton *                           pb_help;
      QPushButton *                           pb_close;


      // ------- out of gui area

      map < QString, set < int > >            cb_checked;

      QColorGroup                             cg_red;

      void                                    editor_msg( QString color, QString msg );

      vector < QWidget * >                    input_widgets;
      vector < QWidget * >                    editor_widgets;

      void                                    hide_widgets( vector < QWidget *> widgets, bool hide );


      void                     *              us_hydrodyn;
      bool                     *              best_widget;


      void                                    setupGUI();

      void                                    clear();

      int                                     points; // how many different triangle points plotted
      vector < double >                       one_over_triangles;
      map < QString, vector < double > >      parameter_data;

      QString                                 loaded_csv_filename;
      QStringList                             loaded_csv_trimmed;

      double                                  last_a;
      double                                  last_siga;
      double                                  last_b;
      double                                  last_sigb;
      double                                  last_chi2;
      QString                                 last_pts_removed;

   private slots:

      // ------ data section 

      void                                    load();
      void                                    save();

      void                                    hide_input();

      // ------ editor section 

      void                                    clear_display();
      void                                    update_font();
      void                                    save_results();

      void                                    hide_editor();

      void                                    help();
      void                                    cancel();

      void                                    data_selected();
      void                                    cb_changed();

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
