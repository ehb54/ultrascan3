//! \file us_report_general_gui.h
#ifndef US_REPORT_GEN_GUI_H
#define US_REPORT_GEN_GUI_H

#include "us_extern.h"
#include "us_widgets.h"


//! This class provides a tabbed entry for solution selection
class US_GUI_EXTERN US_ReportGenGui: public US_Widgets
{
  Q_OBJECT

  public:

      US_ReportGenGui( QString );
      ~US_ReportGenGui() {};

      //abde
      bool abde_mode;
      void abde_mode_passed( void );

   signals:
      void update_details( QString& );
      
   private:
      QString reportMask;
      QJsonObject json;
      QTreeWidget     *treeWidget;
      QMap<QString, QTreeWidgetItem *> topItem;
      QMap<QString, QTreeWidgetItem *> solutionItem;
      QMap<QString, QTreeWidgetItem *> analysisItem;
      QMap<QString, QTreeWidgetItem *> analysisGenItem;
      QMap<QString, QTreeWidgetItem *> analysis2DSAItem;
      QMap<QString, QTreeWidgetItem *> analysisPCSAItem;
      QStringList topLevelItems;
      
      QStringList solutionItems;
      QStringList solutionItems_vals;
      
      QStringList analysisItems;

      QStringList analysisGenItems;
      QStringList analysisGenItems_vals;

      QStringList analysis2DSAItems;
      QStringList analysis2DSAItems_vals;

      QStringList analysisPCSAItems;
      QStringList analysisPCSAItems_vals;
      
      int row;
      
      QVBoxLayout* main;
      QGridLayout* genL;
      QScrollArea* scrollArea;
      QWidget*     containerWidget;
      //QWidget*     topContainerWidget;
      QHBoxLayout* lower_buttons;
      
      QPushButton*  pb_cancel;
      QPushButton*  pb_accept;

      void build_layout( void );
      void gui_to_parms( void );
      void get_item_childs( QList< QTreeWidgetItem* > &, QTreeWidgetItem* );
      void get_children_to_json( QString &, QTreeWidgetItem* );
      QString tree_to_json ( QMap < QString, QTreeWidgetItem * > );
      
   private slots:
      void changedItem   ( QTreeWidgetItem*, int );
      void cancel_selection( void );
      void update_selection( void );

     
   public slots:     

      
};

#endif
