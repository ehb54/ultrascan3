//! \file us_combined_plots_gui.h
#ifndef US_COMBPLOTS_GUI_H
#define US_COMBPLOTS_GUI_H

#include "us_extern.h"
#include "us_widgets.h"


//! This class provides a tabbed entry for solution selection
class US_GUI_EXTERN US_CombPlotsGui: public US_Widgets
{
  Q_OBJECT

  public:

      US_CombPlotsGui( QString  );
      ~US_CombPlotsGui() {};

   signals:
       void update_combplots_parms( QString& );
      
   private:
       QString combPlotsMask;
       QJsonObject json;
       
       
       int row;
       QVBoxLayout* main;
       //QGridLayout* params; 
       QGridLayout* genL;
       QScrollArea* scrollArea;
       QWidget*     containerWidget;
       QWidget*     topContainerWidget;
       QHBoxLayout* lower_buttons;

       QMap < QString, QMap < QString, QString > >  combPlots_map;
             
       QPushButton*  pb_cancel;
       QPushButton*  pb_accept;
       QPushButton*  pb_applyall;

       void parse_json( void );
       void build_layout( void );
       void gui_to_parms( void );
       QString to_json( void );
            
   private slots:
      void cancel_update  ( void );
      void update_parms   ( void );
      void applied_to_all ( void );
           
   public slots:
};

#endif
