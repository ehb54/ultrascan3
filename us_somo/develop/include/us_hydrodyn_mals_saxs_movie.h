#ifndef US_HYDRODYN_MALS_SAXS_MOVIE_H
#define US_HYDRODYN_MALS_SAXS_MOVIE_H

// QT defs:

#include "../include/us_hydrodyn.h"
#include "../include/us_hydrodyn_saxs.h"
//Added by qt3to4:
#include <QPixmap>
#include <QLabel>
#include <QCloseEvent>

using namespace std;

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( disable: 4251 )
# endif
#endif

class US_EXTERN US_Hydrodyn_Mals_Saxs_Movie : public QDialog
{
   Q_OBJECT

   public:
      US_Hydrodyn_Mals_Saxs_Movie(
                                US_Hydrodyn_Mals_Saxs *mals_saxs_win, 
                                QWidget *p = 0, 
                                const char *name = 0
                                );

      ~US_Hydrodyn_Mals_Saxs_Movie();

   private:
      US_Config             * USglobal;

      US_Hydrodyn_Mals_Saxs * mals_saxs_win;
      US_Hydrodyn           * ush_win;

      // QwtWheel              * qwtw_wheel;

      QPushButton           * pb_front;
      QPushButton           * pb_prev;
      QPushButton           * pb_slower;
      QPushButton           * pb_start;
      QPushButton           * pb_faster;
      QPushButton           * pb_next;
      QPushButton           * pb_end;

      QLabel                * lbl_state;
      QLabel                * lbl_current;

      QCheckBox             * cb_save;
      QLineEdit             * le_save;

      QCheckBox             * cb_save_overwrite;

      QGroupBox          * bg_save;
      QRadioButton          * rb_save_png;
      QRadioButton          * rb_save_jpeg;
      QRadioButton          * rb_save_bmp;

      QCheckBox             * cb_show_gauss;
      QCheckBox             * cb_show_ref;
      QCheckBox             * cb_mono;

      QPushButton           * pb_help;
      QPushButton           * pb_cancel;

      vector < int >          mals_saxs_selected_files;

      int                     pos;
      int                     last_pos;
      void                    update_plot();

      QTimer                * timer;
      int                     timer_msec;
      bool                    last_show_gauss;
      bool                    last_show_ref;
      bool                    last_mono;

      void                    save_plot();
      void                    save_plot( QWidget *plot, QString tag );
      void                    save_plot( QWidget *plot, QWidget *plot2, QString tag );
      void                    save_plot( QWidget *plot, QWidget *plot2, QWidget *plot3, QString tag );
      void                    save_plot( QPixmap & m1, QString tag, int mypos );

      void                    join_maps( QPixmap & m1, QPixmap & m2 );

      vector < QColor >       plot_colors;

   private slots:

      void front();
      void prev();
      void slower();
      void start();
      void faster();
      void next();
      void end();

      void set_show_gauss();
      void set_show_ref();
      void set_mono();
      void set_save();

      void help();
      void cancel();

      void setupGUI();
      void update_enables();

      // void adjust_wheel ( double );
      
   protected slots:

      void closeEvent(QCloseEvent *);
   
};

#ifdef WIN32
# if QT_VERSION < 0x040000
  #pragma warning ( default: 4251 )
# endif
#endif

#endif
