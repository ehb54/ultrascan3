#ifndef US_EXTINCTION_H
#define US_EXTINCTION_H

// QT includes:
#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qfile.h>
#include <qlabel.h>
#include <qtextstream.h>
#include <qfont.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpoint.h>
#include <qpixmap.h>
#include <qcheckbox.h>
#include <qmessagebox.h>
#include <qmultilineedit.h>
#include <qfiledialog.h>
#include <qstringlist.h>

// QWT includes:
#include <qwt_plot.h>
#include <qwt_curve.h>
#include <qwt_symbol.h>
#include <qwt_counter.h>

// UltraScan includes:
#include "us_util.h"
#include "us_pixmap.h"
#include "us_editor.h"
#include "us_extinctfitter.h"
#include "us_db_tbl_vbar.h"

extern int global_Xpos;
extern int global_Ypos;

class US_EXTERN US_ExtinctionLegend : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_ExtinctionLegend(QWidget *p=0 , const char *name=0);
      ~US_ExtinctionLegend();
      US_Config *USglobal;

      int xpos, buttonh, column1, column2, ypos, spacing, border;
      QLabel *lbl_lambda1;
      QLabel *lbl_lambda2;
      QLabel *lbl_extinction1;
      QLabel *lbl_extinction2;
      QLabel *lbl_instructions;
      QRadioButton *rb_pathlength;
      QRadioButton *rb_normalized;
      bool normalized;

   public slots:
      void setup_GUI();
      void set_pathlength();
      void set_normalized();
   
   signals:
   
      void scaleChanged();
};

class US_EXTERN US_Extinction : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_Extinction(QString, int, QWidget *p=0 , const char *name=0);
      ~US_Extinction();
      US_ExtinctionLegend *ext_l;
      US_Pixmap *pm;
      US_Config *USglobal;
      US_ExtinctionFitter *fitter;
      QString projectName;
      int investigatorID;
      
#ifdef WIN32
#pragma warning ( disable: 4251 )
#endif

    vector <struct WavelengthScan> wavelengthScan_vector;
      vector <QString> filenames;
      vector <float> parameter_vector;
      vector <float> lambda;
      vector <float> extinction;

#ifdef WIN32
#pragma warning ( default: 4251 )
#endif

      float  xmin, xmax, lambda_min, lambda_max;
      double *fitparameters; 
      unsigned int order, parameters;
      bool fitted, fitting_widget;
      QwtPlot *data_plot;
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_odCutoff;
      QLabel *lbl_lambdaCutoff;
      QLabel *lbl_pathlength;
      QLabel *lbl_extinction;
      QLabel *lbl_project;
      QLabel *lbl_order;
      QwtCounter *cnt_order;
      QwtCounter *cnt_wavelength;
      QPushButton *pb_selectScans;
      QPushButton *pb_reset;
      QPushButton *pb_plot;
      QPushButton *pb_fit;
      QPushButton *pb_selectPeptide;
      QPushButton *pb_save;
      QPushButton *pb_print;
      QPushButton *pb_view;
      QPushButton *pb_help;
      QPushButton *pb_close;
      QLineEdit *le_odCutoff;
      QLineEdit *le_lambdaCutoff;
      QLineEdit *le_pathlength;
      QLineEdit *le_extinction;
      QLineEdit *le_project;
      QListBox *lb_scans;
      QPoint point;
      
   private:
   
      int xpos, buttonh, buttonw, ypos, spacing, border, model;
      unsigned int maxrange;
      bool print_plot;
      float odCutoff, lambdaCutoff, pathlength, extinction_coefficient, factor, selected_wavelength;
      QString htmlDir;
      
   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
//      void resizeEvent(QResizeEvent *e);
      void mouseMoved(const QMouseEvent &e);
      void mousePressed(const QMouseEvent &e);
      void mouseReleased(const QMouseEvent &e);
   
   public slots:

      void selectScans();
      void reset();
      void plot();
      void fit();
      void initialize(double **);
      void selectPeptide();
      void save();
      void print();
      void view();
      void help();
      void cancel();
      bool loadScan(const QString &);
      void update_odCutoff(const QString &);
      void update_lambdaCutoff(const QString &);
      void update_pathlength(const QString &);
      void update_extinction(const QString &);
      void update_extinction_coefficient(float);
      void calc_extinction();
      void update_project(const QString &);
      void update_scale();
      void update_order(double);
      void update_wavelength(double);
      bool createHtmlDir();

   signals:
   
      void fitUpdated(double *, double *, unsigned int);
      void projectChanged(const QString &);
      void extinctionClosed();
};


#endif

