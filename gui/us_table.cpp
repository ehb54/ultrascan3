//! \file us_table.cpp
#include "us_table.h"
#include "us_gui_settings.h"

US_Table::US_Table(QMap<double, double> &map, const QString &type, bool &change, QWidget *parent, Qt::WindowFlags f) :
    US_WidgetsDialog(parent, f), values(map), changed(change) {
   setPalette(US_GuiSettings::frameColor());
   setAttribute(Qt::WA_DeleteOnClose);

   local = values;

   QVBoxLayout *main = new QVBoxLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   QHBoxLayout *edits = new QHBoxLayout;

   QLabel *lb_wl = us_label(tr("Wavelength:"));
   edits->addWidget(lb_wl);

   le_wl = us_lineedit();
   connect(le_wl, SIGNAL(editingFinished()), SLOT(edited()));
   edits->addWidget(le_wl);

   QLabel *lb_value = us_label(type);
   edits->addWidget(lb_value);

   le_value = us_lineedit();
   connect(le_value, SIGNAL(editingFinished()), SLOT(edited()));
   edits->addWidget(le_value);

   main->addLayout(edits);

   // Create and fill in list widget
   lw_table = us_listwidget();

   main->addWidget(lw_table);
   connect(lw_table, SIGNAL(itemSelectionChanged(void)), SLOT(new_row(void)));

   connect(lw_table, SIGNAL(itemDoubleClicked(QListWidgetItem *)), SLOT(delete_row(QListWidgetItem *)));

   QBoxLayout *buttons = new QHBoxLayout;

   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
   buttons->addWidget(pb_cancel);

   QPushButton *pb_accept = us_pushbutton(tr("Accept"));
   connect(pb_accept, SIGNAL(clicked()), SLOT(done()));
   buttons->addWidget(pb_accept);

   main->addLayout(buttons);

   update();
}

void US_Table::done(void) {
   if (values != local) {
      values = local;
      changed = true;
   }

   close();
}

void US_Table::edited(void) {
   if (le_wl->text().isEmpty() || le_value->text().isEmpty())
      return;

   double wl = le_wl->text().toDouble();
   double value = le_value->text().toDouble();

   if (wl < 190.0 || wl > 750.0) {
      QMessageBox::warning(this, tr("Error"), tr("The wavelength must be in the range of 190-750."));
      le_wl->clear();
      le_value->clear();
      return;
   }

   if (value < 0.0) {
      QMessageBox::warning(this, tr("Error"), tr("The value entered cannot be negative."));
      le_wl->clear();
      le_value->clear();
      return;
   }

   local.insert(wl, value);
   update();

   lw_table->setCurrentRow(-1);
}

void US_Table::update(void) {
   lw_table->clear();

   QList<double> keys = local.keys();
   qSort(keys);

   for (int i = 0; i < keys.size(); i++) {
      QString wavelength = QString::number(keys[ i ], 'f', 1);
      QString value = QString::number(local[ keys[ i ] ], 'f', 4);

      lw_table->addItem(wavelength + " / " + value);
   }
}

void US_Table::new_row(void) {
   QStringList v = lw_table->currentItem()->text().split(" / ");
   le_wl->setText(v[ 0 ]);
   le_value->setText(v[ 1 ]);
}

void US_Table::delete_row(QListWidgetItem *item) {
   int response = QMessageBox::question(
      this, tr("Delete Entry?"), tr("Delete the current entry?"), QMessageBox::Yes, QMessageBox::Cancel);

   if (response == QMessageBox::Yes) {
      qDebug() << "Delete_1";
      QStringList v = item->text().split(" / ");
      qDebug() << "Delete_1a";
      local.remove(v[ 0 ].toDouble());
      qDebug() << "Delete_1b";
      qDebug() << "item: " << item;
      delete item;
      qDebug() << "Delete_2";
   }
}
