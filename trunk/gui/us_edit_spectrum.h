//! \file us_solution_gui.h
#ifndef US_EDIT_SPECTRUM_H
#define US_EDIT_SPECTRUM_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"
#include "us_buffer.h"
#include "us_analyte.h"
#include "us_solution.h"
#include "us_help.h"

#include "us_db2.h"
#include "us_eprofile.h"

#include "us_minimize.h"
#include "us_extinction_gui.h"
#include "us_extinctfitter_gui.h"

class US_GUI_EXTERN US_EditSpectrum : public US_Widgets
{
   Q_OBJECT

   public:
   
   US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Buffer*);
   US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Analyte*);
   US_EditSpectrum(QString type, QString exists, const QString &text, const QString &text_e280, US_Solution*);

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
      QString        exists, tmp_exists;

   signals:
      void change_spectrum( void );
      void accept_enable  ( void );

   private:
      QPushButton*  pb_view;
      QPushButton*  pb_delete;
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
      void editmanually      ( void );
      void delete_spectrum   ( void );
      void view_spectrum     ( void );
};

#endif
