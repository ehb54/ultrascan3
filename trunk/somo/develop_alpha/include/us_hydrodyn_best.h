#ifndef US_HYDRODYN_BEST_H
#define US_HYDRODYN_BEST_H

#include "us_hydrodyn.h"
#include "us_hydrodyn_saxs.h"
#include "us_csv.h"
//Added by qt3to4:
#include <Q3HBoxLayout>
#include <QCloseEvent>
#include <Q3Frame>
#include <QLabel>

using namespace std;

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Best : public Q3Frame
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

      QLabel *                                lbl_credits_1;
      QLabel *                                lbl_credits_2;

      mQLabel *                               lbl_input;
      Q3ListBox *                              lb_data;
      QPushButton *                           pb_load;
      QPushButton *                           pb_join_results;
      QPushButton *                           pb_save_results;
      QLineEdit *                             le_last_file;
      QCheckBox             *                 cb_plus_lm;

      // ------ editor section 

      mQLabel               *                 lbl_editor;
      QFont                                   ft;
      Q3TextEdit             *                 editor;
      QMenuBar              *                 mb_editor;

      // ------ plot section
      QwtPlot               *                 plot_data;
      ScrollZoomer          *                 plot_data_zoomer;
#ifdef QT4
      QwtPlotGrid           *                 grid_data;
#endif
      QCheckBox             *                 cb_errorlines;
      Q3HBoxLayout           *                 hbl_points;
      mQLabel               *                 lbl_points;
      vector < QCheckBox * >                  cb_points;
      Q3HBoxLayout           *                 hbl_points_ln;
      mQLabel               *                 lbl_points_ln;
      vector < QCheckBox * >                  cb_points_ln;
      Q3HBoxLayout           *                 hbl_points_exp;
      mQLabel               *                 lbl_points_exp;
      vector < QCheckBox * >                  cb_points_exp;

      // ------- bottom section

      QPushButton *                           pb_help;
      QPushButton *                           pb_close;

      // ------- out of gui area

      map < QString, set < int > >            cb_checked;
      map < QString, set < int > >            cb_checked_ln;
      map < QString, set < int > >            cb_checked_exp;

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

      double                                  last_a_ln;
      double                                  last_siga_ln;
      double                                  last_b_ln;
      double                                  last_sigb_ln;
      double                                  last_chi2_ln;
      QString                                 last_pts_removed_ln;

      bool                                    ln_plot_ok;

      double                                  last_a_exp;
      double                                  last_siga_exp;
      double                                  last_b_exp;
      double                                  last_sigb_exp;
      double                                  last_c_exp;
      double                                  last_sigc_exp;
      double                                  last_chi2_exp;
      QString                                 last_pts_removed_exp;

      bool                                    exp_plot_ok;

      QStringList                             tau_inputs;
      set < QString >                         tau_input_set;
      QStringList                             tau_msg;
      void                                    recompute_tau();
      map < QString, vector < double > >      last_lin_extrapolation;
      map < QString, vector < double > >      last_log_extrapolation;
      map < QString, vector < double > >      last_exp_extrapolation;
      QStringList                             tau_csv_addendum_tag;
      QStringList                             tau_csv_addendum_val;

      QString                                 save_last_file;

   private slots:

      // ------ data section 

      void                                    load();
      void                                    join_results();
      void                                    save_results();

      void                                    hide_input();

      // ------ editor section 

      void                                    clear_display();
      void                                    update_font();
      void                                    save();

      void                                    hide_editor();

      void                                    help();
      void                                    cancel();

      void                                    data_selected    ( bool do_recompute_tau = true );
      void                                    cb_changed       ( bool do_data = true );
      void                                    cb_changed_ln    ( bool do_data = true );
      void                                    cb_changed_exp   ( bool do_data = true );

      void                                    toggle_points    ();
      void                                    toggle_points_ln ();
      void                                    toggle_points_exp();

      void                                    set_last_file    ( const QString & str );

   protected slots:

      void                                    closeEvent( QCloseEvent * );
};

#ifdef WIN32
# if !defined( QT4 )
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
