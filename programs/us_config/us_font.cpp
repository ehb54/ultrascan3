//! \file us_font.cpp
#include "us_font.h"
#include "us_gui_settings.h"

US_Font::US_Font(QWidget *w, Qt::WindowFlags flags) : US_Widgets(true, w, flags) {
   // Frame layout
   setPalette(US_GuiSettings::frameColor());
   setWindowTitle("Font Selection Dialog");
   setAttribute(Qt::WA_DeleteOnClose);

   QBoxLayout *topbox = new QVBoxLayout(this);
   topbox->setSpacing(2);

   // Body
   pb_font = us_pushbutton(tr("Select Base Font"));
   topbox->addWidget(pb_font);
   connect(pb_font, SIGNAL(clicked()), SLOT(selectFont()));

   lbl_family = us_label(tr("Current Family:"), -1);
   lbl_family->setFixedHeight(BUTTON_H);

   QGridLayout *lineGrid = new QGridLayout();

   le_family = us_lineedit(US_GuiSettings::fontFamily(), -1, true);

   int row = 0;

   lineGrid->addWidget(lbl_family, row, 0);
   lineGrid->addWidget(le_family, row++, 1);

   size = us_label(tr("Point Size:"), -1);
   size->setFixedHeight(BUTTON_H);

   le_pointSize = us_lineedit(QString::number(US_GuiSettings::fontSize()), -1, true);

   lineGrid->addWidget(size, row, 0);
   lineGrid->addWidget(le_pointSize, row++, 1);

   samples = us_banner(tr("Selected Font Samples:"));
   lineGrid->addWidget(samples, row++, 0, 1, 2);

   small = us_label(tr("Small Font Sample"), -1);
   lineGrid->addWidget(small, row++, 0, 1, 2);

   regular = us_label(tr("Regular Font Sample"));
   lineGrid->addWidget(regular, row++, 0, 1, 2);

   regularBold = us_label(tr("Regular Font Sample, Bold"), 0, QFont::Bold);
   lineGrid->addWidget(regularBold, row++, 0, 1, 2);

   large = us_label(tr("Large Font Sample"), +1);
   lineGrid->addWidget(large, row++, 0, 1, 2);

   largeBold = us_label(tr("Large Font Sample, Bold"), +1, QFont::Bold);
   lineGrid->addWidget(largeBold, row++, 0, 1, 2);

   title = us_label(tr("Title Font Sample"), +2, QFont::Bold);
   lineGrid->addWidget(title, row++, 0, 1, 2);

   topbox->addLayout(lineGrid);

   pb_default = us_pushbutton(tr("Select Default"));
   connect(pb_default, SIGNAL(clicked()), SLOT(setDefault()));
   topbox->addWidget(pb_default);

   pb_save = us_pushbutton(tr("Save"));
   connect(pb_save, SIGNAL(clicked()), SLOT(save()));

   pb_help = us_pushbutton(tr("Help"));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_cancel = us_pushbutton(tr("Close"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(close()));

   QBoxLayout *buttons = new QHBoxLayout();

   buttons->addWidget(pb_save);
   buttons->addWidget(pb_help);
   buttons->addWidget(pb_cancel);

   topbox->addLayout(buttons);
}

void US_Font::setDefault(void) {
   le_family->setText("Helvetica");
   le_pointSize->setText(QString::number(10));

   redraw();
}


void US_Font::save() {
   US_GuiSettings::set_fontFamily(le_family->text());
   US_GuiSettings::set_fontSize(le_pointSize->text().toInt());

   QMessageBox::information(
      this, tr("Settings Saved"),
      tr("The settings were successfully saved.\n"
         "Changes will be applied in new windows."));
}

void US_Font::help() {
   US_Help *help = new US_Help();
   help->show_help("manual/usfont.html");
}

void US_Font::selectFont() {
   bool ok;

   QFont oldFont = QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize());
   QFont newFont = QFontDialog::getFont(&ok, oldFont, this, "Select a Font");

   if (!ok)
      return;

   le_family->setText(newFont.family());
   le_pointSize->setText(QString::number(newFont.pointSize()));

   redraw();
}

void US_Font::redraw(void) {
   QString family = le_family->text();
   int pointSize = le_pointSize->text().toInt();


   le_family->setFont(QFont(family, pointSize));
   le_pointSize->setFont(QFont(family, pointSize));
   lbl_family->setFont(QFont(family, pointSize));
   size->setFont(QFont(family, pointSize));
   samples->setFont(QFont(family, pointSize));

   small->setFont(QFont(family, pointSize - 1));
   regular->setFont(QFont(family, pointSize));
   regularBold->setFont(QFont(family, pointSize, QFont::Bold));
   large->setFont(QFont(family, pointSize + 1));
   largeBold->setFont(QFont(family, pointSize + 1, QFont::Bold));
   title->setFont(QFont(family, pointSize + 2, QFont::Bold));
   pb_default->setFont(QFont(family, pointSize + 1));
   pb_save->setFont(QFont(family, pointSize + 1));
   pb_font->setFont(QFont(family, pointSize + 1));
   pb_help->setFont(QFont(family, pointSize + 1));
   pb_cancel->setFont(QFont(family, pointSize + 1));

   this->repaint();
}
