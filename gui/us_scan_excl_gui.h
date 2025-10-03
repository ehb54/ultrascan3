//! \file us_scan_excl_gui.h
#ifndef US_SCANEXCL_GUI_H
#define US_SCANEXCL_GUI_H

#include "us_extern.h"
#include "us_widgets.h"


//! This class provides a tabbed entry for solution selection
class US_GUI_EXTERN US_ScanExclGui: public US_Widgets
{
  Q_OBJECT

  public:

      US_ScanExclGui( QStringList, QList< int > , QList< int >, QList< int >, int, int  );
      ~US_ScanExclGui() {};

   signals:
      void update_aprofile_scans( QStringList& );
      
   private:

      QStringList channels_desc;
      QList < int >  scan_beg;
      QList < int >  scan_end;
      QList < int >  scan_nth;
      int scanCount;
      int scanCount_int;
      
      QMap< QString, int > maxScans_map;

      int  sb_beg_prior;
      int  sb_nth_prior;
      int  sb_end_prior;
      
      int row;
      QVBoxLayout* main;
      //QGridLayout* params; 
      QGridLayout* genL;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;
      QWidget*     topContainerWidget;
      QHBoxLayout* lower_buttons;

      //Begin | End scans counters
      QSpinBox*    sb_begin;
      QSpinBox*    sb_end;
           
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;
      QPushButton*  pb_applyall;
      
      void build_layout( void );
      void gui_to_parms( void );
            
   private slots:
      void cancel_update( void );
      void update_scans_excl( void );
      void applied_to_all( void );
      void scan_excl_changed( int );
     
   public slots:
};

#endif
