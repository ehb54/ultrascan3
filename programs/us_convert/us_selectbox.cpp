//! \file us_selectbox.cpp

#include "us_selectbox.h"
#include "us_gui_settings.h"

// A class for creating widgets with options referred to
//   by a logical ID
US_SelectBase::US_SelectBase() {}

// The implementation routines for a combo box
US_SelectBox::US_SelectBox(QWidget *parent) : QComboBox(parent), US_SelectBase() {
   this->setPalette(US_GuiSettings::normalColor());
   this->setAutoFillBackground(true);
   this->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize()));

   this->load();
}

void US_SelectBox::addOption(listInfo &option) {
   this->widgetList << option;
}

void US_SelectBox::addOptions(QList<listInfo> &options) {
   // Initialize combo box or other type of control
   this->widgetList.clear();
   foreach (listInfo option, options) {
      this->widgetList << option;
   }

   this->setCurrentIndex(0);
}

void US_SelectBox::load(void) {
   this->clear();

   foreach (listInfo option, widgetList)
      this->addItem(option.text);

   this->setCurrentIndex(0);
}

// Function to update a combobox so that the current choice is selected
void US_SelectBox::setLogicalIndex(int ID) {
   for (int i = 0; i < this->widgetList.size(); i++) {
      if (this->widgetList[ i ].ID.toInt() == ID) {
         this->setCurrentIndex(i);
         return;
      }
   }

   // If here, index was not found
   this->setCurrentIndex(0);
}

int US_SelectBox::getLogicalID(void) {
   int ndx = this->currentIndex();

   return (
      (ndx == -1) ? -1 // Combo box is empty or not set
                  : this->widgetList[ ndx ].ID.toInt());
}

// Implementation routines for a ListWidget
US_ListwidgetBox::US_ListwidgetBox(QWidget *parent, int fontAdjust) : QListWidget(parent), US_SelectBase() {
   this->setAutoFillBackground(true);
   this->setPalette(US_GuiSettings::editColor());
   this->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() + fontAdjust));

   this->load();
}

void US_ListwidgetBox::addOption(listInfo &option) {
   this->widgetList << option;
}

void US_ListwidgetBox::addOptions(QList<listInfo> &options) {
   // Initialize list widget
   this->widgetList.clear();
   foreach (listInfo option, options)
      this->widgetList << option;

   this->setCurrentRow(0);
}

void US_ListwidgetBox::load(void) {
   this->clear();

   foreach (listInfo option, widgetList)
      this->addItem(option.text);

   this->setCurrentRow(0);
}

// Function to update a combobox so that the current choice is selected
void US_ListwidgetBox::setLogicalIndex(int ID) {
   for (int i = 0; i < this->widgetList.size(); i++) {
      if (this->widgetList[ i ].ID.toInt() == ID) {
         this->setCurrentRow(i);
         return;
      }
   }

   // If here, index was not found
   this->setCurrentRow(0);
}

int US_ListwidgetBox::getLogicalID(void) {
   int ndx = this->currentRow();

   for (int i = 0; i < this->widgetList.size(); i++) {
      if (this->widgetList[ i ].ID.toInt() == ndx)
         return (this->widgetList[ i ].ID.toInt());
   }

   // If here, index was not found
   return (0);
}
