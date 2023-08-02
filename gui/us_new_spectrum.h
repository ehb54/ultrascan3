//! \file us_solution_gui.h
#ifndef US_NEW_SPECTRUM_H
#define US_NEW_SPECTRUM_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"


class US_GUI_EXTERN US_NewSpectrum : public US_Widgets
{
   Q_OBJECT

   public:
   
   US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Buffer*);
   US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Analyte*);
   US_NewSpectrum(QString type, const QString &text, const QString &text_e280, US_Solution*);

      US_Buffer*   buffer;
      US_Buffer*   tmp_buffer;

      US_Analyte*   analyte;
      US_Analyte*   tmp_analyte; 
      
      US_Solution*   solution;
      US_Solution*   tmp_solution;

      US_Extinction*  w_spec;
      QString        text, tmp_text;
      QString        type, tmp_type;
      QString        text_e280, tmp_text_e280;

   signals:
      void change_prot_e280( void );

   private:
      QPushButton*  pb_cancel;
      QPushButton*  pb_manual;
      QPushButton*  pb_uploadDisk;
      QPushButton*  pb_uploadFit;
      
      QMap< double, double > loc_extinct;
      
   private slots:    
      void cancel ( void );
      void uploadDisk ( void );
      void uploadFit  ( void );
      void process_results( QMap < double, double > &xyz );
      void add_spectrumDisk   ( void );
      void readingspectra     (const QString&);
      void entermanually      ( void );
};

// class to view spectrum
class US_ViewSpectrum : public US_Widgets
{
   Q_OBJECT

   public:

        US_ViewSpectrum(QMap<double,double>& tmp_extinciton);
	
	QMap <double, double> extinction;
	US_Plot*	plotLayout;
	QwtPlot	 	*data_plot;
	
	void plot_extinction();
	
   private slots:
    void save_csv();
	
};

#endif
