//! \file us_report_gui.h
#ifndef US_REPORT_GUI_H
#define US_REPORT_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_run_protocol.h"
#include "us_report_gmp.h"

//! This class provides a tabbed entry for solution selection
class US_ReportGui: public US_Widgets
{
  Q_OBJECT

  public:

      //! brief Report Gui. To 
      //! instantiate the class a calling function must
      //! provide the ID of the investigator.
      US_ReportGui( US_ReportGMP* );
      ~US_ReportGui() {};

      US_ReportGMP*   report;
      US_ReportGMP    report_copy_original;

   signals:
      void cancel_changes( US_ReportGMP & );
      
      
   private:
      int row;
      QVBoxLayout* main;
      QGridLayout* params;
      QGridLayout* genL;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;
      QGridLayout* addRem_buttons;
      QHBoxLayout* lower_buttons;

      QLineEdit*   le_tot_conc;
      QLineEdit*   le_rmsd_limit;
      QLineEdit*   le_av_intensity;
      QLineEdit*   le_wvl;
            
      //duration
      QSpinBox*    sb_durat_dd;
      QSpinBox*    sb_durat_hh;
      QSpinBox*    sb_durat_mm;
      QSpinBox*    sb_durat_ss;

      QPushButton*  pb_addRow;
      QPushButton*  pb_removeRow;
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;

      void build_report_layout( void );
      void gui_to_report( void );

   private slots:
     void update_report( void );
     void add_row( void );
     void remove_row( void );
     void cancel_update( void );
     
   public slots:
};

#endif
