//! \file us_sassoc.h
#ifndef US_SASSOC_H
#define US_SASSOC_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_plot.h"
#include "us_help.h"

#include <qwt_counter.h>
#include <qwt_plot_curve.h>

//! Define the number of points in a plot
#define ARRAY_SIZE 555

//! \brief Class to show results of Self-Association Equilibrium Simulation
//! This program allows you to simulate the relative amounts of monomer, dimer 
//! and tetramer species present in a self-association equilibrium.

class US_GUI_EXTERN US_Sassoc : public US_Widgets
{
   Q_OBJECT
   
   public:
   
      /*! Constructor
         \param eq0 Equilibrium constant for first component
         \param eq1 Equilibrium constant for second component
         \param stoich1 Stoichiometry for first component
         \param stoich2 Stoichiometry for second component
         \param t_project Project Name
         \param t_model Index into redefined model US_Constants::modelStrings()
         \param status Not used
         \param position_win If true, set the window position upper left
                        corner to 30, 30 from parent, otherwise system default
         \param parent  Parent window, normally 0
         \param flags   Window flags, normally 0 (default)
      */
      US_Sassoc( double eq0, double eq1, double stoich1, double stoich2,
                 const QString& t_project, int t_model, bool status,
                 bool position_win = true,
                 QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );
      
   private:
      double         eq    [ 2 ];
      double         stoich[ 2 ];
      QString        project;
      int            model;
                   
      double         x       [ ARRAY_SIZE ];
      double         species1[ ARRAY_SIZE ];
      double         species2[ ARRAY_SIZE ];
      double         species3[ ARRAY_SIZE ];
                    
      bool           updating;

      QwtPlot*       plot;
                    
      QwtPlotCurve*  curve1;
      QwtPlotCurve*  curve2;
      QwtPlotCurve*  curve3;
                    
      QwtCounter*    c_equil1;
      QwtCounter*    c_equil2;
                    
      QLineEdit*     le_species1;
      QLineEdit*     le_species2;
      QLineEdit*     le_species3;
      QLineEdit*     le_conc;
      QLineEdit*     le_assoc1;
      QLineEdit*     le_assoc2;
      QLineEdit*     le_equil1;
      QLineEdit*     le_equil2;
      
      US_Plot*       plot1;

      US_Help        showHelp;

      QwtPlotPicker* pick;

      void   recalc         ( void           );
      double monomer_root   ( double         );
      double polynomial     ( double, double );
      void   update_legend  ( double         );

   private slots:
      void   new_value      ( const QPointF& );
      void   mouseD         ( const QPointF& );
      void   mouseU         ( const QPointF& );
      void   update_stoich1 ( const QString&        );
      void   update_stoich2 ( const QString&        );
      void   update_eq1     ( const QString&        );
      void   update_eq2     ( const QString&        );
      void   update_eq1Count( double                );
      void   update_eq2Count( double                );
      void   save           ( void                  );

      void   help           ( void )
      { showHelp.show_help( "manual/sassoc.html" ); };
};
#endif

#ifdef NEVER
#include "us_util.h"
#include "us_pixmap.h"
#include "us_math.h"
#include "us_selectplot.h"
#include "us_printfilter.h"


class US_SassocLegend : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_SassocLegend(QWidget *p=0 , const char *name=0);
      ~US_SassocLegend();
      US_Config *USglobal;

      int xpos, buttonh, column1, column2, ypos, spacing, border;
      QLabel *lbl_species1a;
      QLabel *lbl_species2a;
      QLabel *lbl_species3a;
      QLabel *lbl_species4a;
      QLabel *lbl_species1b;
      QLabel *lbl_species2b;
      QLabel *lbl_species3b;
      QLabel *lbl_species4b;
      QLabel *lbl_instructions;
      void setup_GUI();
};

class US_GUI_EXTERN US_Sassoc : public QFrame
{
   Q_OBJECT
   
   public:
   
      US_Sassoc( float, float, float, float, QString, int, bool, QWidget *p=0 , const char *name=0);
      ~US_Sassoc();

      QwtPlot *data_plot;
      QLabel *lbl_banner1;
      QLabel *lbl_banner2;
      QLabel *lbl_banner3;
      QLabel *lbl_banner4;
      QLabel *lbl_banner5;
      QLabel *lbl_eq1;
      QLabel *lbl_eq2;
      QLabel *lbl_stoich1;
      QLabel *lbl_stoich2;
      QLabel *lbl_project;
      
      QTextEdit *mle_model;
      QPushButton *pb_cancel;
      QPushButton *pb_write;
      QPushButton *pb_help;
      QPushButton *pb_print;
      QPushButton *pb_recalc;
      QwtCounter *cnt_eq1;
      QwtCounter *cnt_eq2;
      QLineEdit *le_stoich1;
      QLineEdit *le_stoich2;
      QLineEdit *le_eq1;
      QLineEdit *le_eq2;
      QPoint point;
      US_SassocLegend *sas_l;
      US_Pixmap *pm;
      
   private:
   
      int xpos, buttonh, buttonw, ypos, spacing, border, model;
      unsigned int ARRAY_SIZE;
      unsigned int curve[3];
      double **data;
      QString htmlDir;
      
   protected slots:
      void setup_GUI();
      void closeEvent(QCloseEvent *);
//    void resizeEvent(QResizeEvent *e);
      void mouseMoved(const QMouseEvent &e);
      void mousePressed(const QMouseEvent &e);
      void mouseReleased(const QMouseEvent &e);
   
   public slots:

      void print();
      void recalc();
      bool createHtmlDir();
      void write_data();
      void help();
      void cancel();
      double monomer_root(double);
      double polynomial(double, double);
      void update_stoich1(const QString &);
      void update_stoich2(const QString &);
      void update_eq1(const QString &);
      void update_eq2(const QString &);
      void update_eq1Count(double);
      void update_eq2Count(double);
};

#endif

