#ifndef US_EXTINCTION_GUI_H
#define US_EXTINCTION_GUI_H
//#include <QApplication>
//#include <QtGui>

#include "us_widgets.h"
#include "us_plot.h"
#include "us_minimize.h"
#include "us_extinctfitter_gui.h"
#include "us_analyte_gui.h"


class CustomListWidgetItem : public QListWidgetItem {
   public:
   CustomListWidgetItem(QListWidget* parent = nullptr)
       : QListWidgetItem(parent) {}

   bool operator<(const QListWidgetItem& other) const;
};

class US_GUI_EXTERN US_Extinction : public US_Widgets
{
   Q_OBJECT

   public:
      US_Extinction();

      //US_Extinction(QString temp_mode, const QString &text, QWidget *parent);
      US_Extinction(QString temp_mode, const QString &text, const QString &text_e280, QWidget *parent);
      
      QVector <QString> filenames;
      US_ExtinctFitter *fitter;
      US_Disk_DB_Controls* disk_controls;
      US_Analyte currentAnalyte;
      QVector <double> lambda;
      QVector <double> extinction;
      float xmin, xmax;
      unsigned int order, parameters;
      double *fitparameters;
      bool fitted, fitting_widget;
      QString buffer_temp;
      
      QVector <QVector<double> > xfit_data;
      QVector <QVector<double> > yfit_data;

      QString current_path;

   private:
      QVector <WavelengthScan> v_wavelength;
      QVector <WavelengthScan> v_wavelength_original;
      QVector <QwtPlotCurve*>  v_curve;

      QWidget * parent;

      QString projectName, filename_one, filename_two;
      unsigned int maxrange;
      float odCutoff, lambdaLimitLeft, lambdaLimitRight,lambda_min, lambda_max,
            pathlength, extinction_coefficient, factor, selected_wavelength;
      QLabel*      lbl_gaussians;
      QLabel*      lbl_peptide;
      QLabel*      lbl_wvinfo;
      QLabel*      lbl_associate;
      QLabel*      lbl_cutoff;
      QLabel*      lbl_lambda1;
      QLabel*      lbl_lambda2;
      QLabel*      lbl_pathlength;
      QLabel*      lbl_coefficient;
      QLabel*      lbl_wavelengthref;
      QListWidget* lw_file_names;
      QPushButton* pb_addWavelength;
      QPushButton* pb_reset;
      QPushButton* pb_update;

      QPushButton* pb_perform;
      QPushButton* pb_perform_buffer;
      QPushButton* pb_perform_analyte;
      QPushButton* pb_perform_solution;
      QPushButton* pb_accept;

      QPushButton* pb_calculate;
      QPushButton* pb_save;
      QPushButton* pb_view;
      QPushButton* pb_help;
      QPushButton* pb_close;

      QLineEdit*   le_associate;
      QLineEdit*   le_associate_buffer;

      QLineEdit*   le_odCutoff;
      QLineEdit*   le_lambdaLimitLeft;
      QLineEdit*   le_lambdaLimitRight;
      QLineEdit*   le_pathlength;
      QLineEdit*   le_coefficient;
      QwtCounter*   ct_gaussian;
      QwtCounter*   ct_coefficient;
      QwtPlotCurve* changedCurve;
      US_Plot*      plotLayout;
      QwtPlot*      data_plot;
      QWidget*      p;

   private slots:
      bool    loadScan ( const QString& );
      bool    isComment( const QString& );
      void    add_wavelength( void );
      void    reading( QStringList );
      void    reset_scanlist( void );
      void    update_data( void );

      void    perform_global( void );
      void    perform_global_buffer( void );
      void    perform_global_analyte( void );
      void    perform_global_solution( void );
      void    accept( void );

      void    calculateE280( void );
      void    save( void );
      void    view_result( void );
      void    help( void );
      void    plot();
      void    calc_extinction();
      void    update_order( double );
      void    listToCurve();
      bool    deleteCurve();
      void    accessAnalyteExtinc( US_Analyte );

      void process_yfit( QVector <QVector<double> > &x, QVector <QVector<double> > &y );

    signals:
      void    get_results( QMap < double, double > &xyz );
      
};
#endif
