//! \file us_select_triples.cpp

#include "us_select_triples.h"

#include "us_gui_settings.h"
#include "us_settings.h"

//! Class to present a pair of lists (included, excluded) to allow the
//!  user to select triples to be excluded (deleted)
US_SelectTriples::US_SelectTriples(QStringList& triples)
    : US_WidgetsDialog(0, 0), triples(triples) {
  original = triples;  // List of original includes
  nbr_select = 0;
  nbr_orig = original.size();
  dbg_level = US_Settings::us_debug();

  setWindowTitle(tr("Triples Selector for Deletes"));
  setPalette(US_GuiSettings::frameColor());

  QVBoxLayout* main = new QVBoxLayout(this);
  main->setSpacing(2);
  main->setContentsMargins(2, 2, 2, 2);
  QVBoxLayout* left = new QVBoxLayout;
  QVBoxLayout* right = new QVBoxLayout;
  QHBoxLayout* lists = new QHBoxLayout;

  // Read-only triple list count text
  le_original = us_lineedit(tr("%1 original triples").arg(nbr_orig), -1, true);
  le_selected =
      us_lineedit(tr("%1 excluded triples").arg(nbr_select), -1, true);

  // Lambda list labels
  QLabel* lb_original = us_label(tr("Included Triples"));
  QLabel* lb_selected = us_label(tr("Excluded Triples"));

  // Lambda list widgets
  lw_original = us_listwidget();
  lw_selected = us_listwidget();
  lw_original->setSelectionMode(QAbstractItemView::ExtendedSelection);
  lw_selected->setSelectionMode(QAbstractItemView::ExtendedSelection);
  includes = original;
  excludes.clear();

  for (int ii = 0; ii < nbr_orig;
       ii++) {  // Add items to the original (included) list
    lw_original->addItem(original[ii]);
  }

  // Add (=>) and Remove (<=) buttons for lists
  pb_add = us_pushbutton(tr("Add  ===>"));
  pb_remove = us_pushbutton(tr("<===  Remove"));

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_reset = us_pushbutton(tr("Reset"));
  QPushButton* pb_help = us_pushbutton(tr("Help"));
  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  pb_accept = us_pushbutton(tr("Accept"));
  pb_accept->setEnabled(false);
  pb_add->setEnabled(includes.count() > 0);
  pb_remove->setEnabled(excludes.count() > 0);

  // Connections
  connect(pb_add, SIGNAL(clicked()), SLOT(add_selections()));
  connect(pb_remove, SIGNAL(clicked()), SLOT(rmv_selections()));
  connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));
  connect(pb_accept, SIGNAL(clicked()), SLOT(done()));

  // Complete layouts
  buttons->addWidget(pb_reset);
  buttons->addWidget(pb_help);
  buttons->addWidget(pb_cancel);
  buttons->addWidget(pb_accept);

  left->addWidget(lb_original);
  left->addWidget(lw_original);
  left->addWidget(pb_add);
  right->addWidget(lb_selected);
  right->addWidget(lw_selected);
  right->addWidget(pb_remove);
  lists->addLayout(left);
  lists->addLayout(right);
  main->addWidget(le_original);
  main->addWidget(le_selected);
  main->addLayout(lists);
  main->addLayout(buttons);

  resize(140, 700);
}

// Slot to add selections to the excluded list
void US_SelectTriples::add_selections() {
  DbgLv(0) << "AddSelections";
  // Get the list of selected items
  QList<QListWidgetItem*> selitems = lw_original->selectedItems();
  nbr_select = selitems.count();

  for (int ii = 0; ii < nbr_select;
       ii++) {  // Move selected items from included to excluded
    QListWidgetItem* l_item = selitems.at(ii);
    QString triple = l_item->text();
    lw_original->setCurrentItem(l_item, QItemSelectionModel::Deselect);
    excludes << triple;          // Add to excluded
    includes.removeOne(triple);  // Remove from included
  }

  excludes.sort();       // Sort new excluded list
  lw_original->clear();  // Clear list widgets
  lw_selected->clear();
  nbr_incl = includes.count();
  nbr_excl = excludes.count();

  for (int ii = 0; ii < nbr_incl; ii++)  // Repopulate included widget
    lw_original->addItem(includes[ii]);

  for (int ii = 0; ii < nbr_excl; ii++)  // Repopulate excluded widget
    lw_selected->addItem(excludes[ii]);

  // Report the new state of things
  le_original->setText((nbr_incl == 1)
                           ? tr("1 included triple")
                           : tr("%1 included triples").arg(nbr_incl) +
                                 tr(" (of %1 original)").arg(nbr_orig));
  le_selected->setText((nbr_excl == 1)
                           ? tr("1 excluded triple")
                           : tr("%1 excluded triples").arg(nbr_excl));

  // Enable/disable buttons appropriately
  pb_accept->setEnabled(nbr_excl > 0 && nbr_incl > 0);
  pb_add->setEnabled(includes.count() > 0);
  pb_remove->setEnabled(excludes.count() > 0);
}

// Slot to remove items from the excluded list
void US_SelectTriples::rmv_selections() {
  DbgLv(0) << "RemoveSelections";
  // Get the list of selected items
  QList<QListWidgetItem*> selitems = lw_selected->selectedItems();
  nbr_select = selitems.count();

  for (int ii = 0; ii < nbr_select;
       ii++) {  // Move selected items from excluded to included
    QListWidgetItem* l_item = selitems.at(ii);
    QString triple = l_item->text();
    lw_selected->setCurrentItem(l_item, QItemSelectionModel::Deselect);
    includes << triple;          // Add to included
    excludes.removeOne(triple);  // Remove from excluded
  }

  includes.sort();       // Sort new included list
  lw_original->clear();  // Clear list widgets
  lw_selected->clear();
  nbr_incl = includes.count();
  nbr_excl = excludes.count();

  for (int ii = 0; ii < nbr_incl; ii++)  // Repopulate included widget
    lw_original->addItem(includes[ii]);

  for (int ii = 0; ii < nbr_excl; ii++)  // Repopulate excluded widget
    lw_selected->addItem(excludes[ii]);

  // Report the new state of things
  le_original->setText((nbr_incl == 1)
                           ? tr("1 included triple")
                           : tr("%1 included triples").arg(nbr_incl) +
                                 tr(" (of %1 original)").arg(nbr_orig));
  le_selected->setText((nbr_excl == 1)
                           ? tr("1 excluded triple")
                           : tr("%1 excluded triples").arg(nbr_excl));

  // Enable/disable buttons appropriately
  pb_accept->setEnabled(nbr_excl > 0 && nbr_incl > 0);
  pb_add->setEnabled(includes.count() > 0);
  pb_remove->setEnabled(excludes.count() > 0);
}

// Reset the lists and buttons to their original state
void US_SelectTriples::reset(void) {
  lw_original->clear();
  lw_selected->clear();
  excludes.clear();
  includes = original;
  nbr_select = 0;

  for (int ii = 0; ii < includes.count(); ii++)
    lw_original->addItem(includes[ii]);

  le_original->setText(tr("%1 original triples").arg(nbr_orig));
  le_selected->setText(tr("0 selected triples"));
  pb_accept->setEnabled(false);
  pb_add->setEnabled(includes.count() > 0);
  pb_remove->setEnabled(excludes.count() > 0);
}

// Cancel button clicked:  returned delete-selections is empty
void US_SelectTriples::cancel(void) {
  triples.clear();

  reject();
  close();
}

// Accept button clicked:  returned delete-selections list is the excluded list
void US_SelectTriples::done(void) {
  triples = excludes;

  accept();
  close();
}
