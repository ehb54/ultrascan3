//! \file us_report_gui.h
#ifndef US_REPORT_GUI_H
#define US_REPORT_GUI_H

#include "us_extern.h"
#include "us_widgets.h"
#include "us_widgets_dialog.h"

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

   signals:
      //void reportAccept( void );
      //void reportCancel( void );
      
   private:
      int row;
      QVBoxLayout* main;
      QGridLayout* genL;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;
      QHBoxLayout* lower_buttons;

      QPushButton*  pb_addRow;
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;

      void build_report_layout( void );
      
   private slots:
     void update_report( void );
     void add_row( void );
     
   public slots:
};

#endif
