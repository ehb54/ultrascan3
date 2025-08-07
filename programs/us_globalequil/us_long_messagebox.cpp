//! \file us_long_messagebox.cpp

#include "us_long_messagebox.h"
#include "us_gui_settings.h"
#include "us_settings.h"

// Main constructor with title and message
US_LongMessageBox::US_LongMessageBox(const QString &title, const QString &message, QWidget *parent, Qt::WindowFlags f) :
    US_WidgetsDialog(parent, f) {
   setAttribute(Qt::WA_DeleteOnClose);
   setWindowTitle(title);
   setPalette(US_GuiSettings::frameColor());

   // Main layout
   QVBoxLayout *main = new QVBoxLayout(this);
   main->setContentsMargins(2, 2, 2, 2);
   main->setSpacing(2);

   // Create the text box
   textbox = us_textedit();
   textbox->setFrameStyle(QFrame::NoFrame || QFrame::Plain);
   textbox->setFont(QFont(US_GuiSettings::fontFamily(), US_GuiSettings::fontSize() - 2));
   textbox->setPlainText(message);
   adjustSize();
   textbox->show();

   // Create a button box with single OK button
   QDialogButtonBox *btnbox = new QDialogButtonBox(QDialogButtonBox::Ok);
   btnbox->setCenterButtons(true);
   connect(btnbox, SIGNAL(accepted()), this, SLOT(close_diag()));

   main->addWidget(textbox);
   main->addWidget(btnbox);
}

// Set the title string
void US_LongMessageBox::setTitle(const QString &title) {
   setWindowTitle(title);
}

// Set new plain text
void US_LongMessageBox::setText(const QString &message) {
   textbox->setPlainText(message);
   adjustSize();
}

// Set new rich text
void US_LongMessageBox::setHtml(const QString &message) {
   textbox->setAcceptRichText(true);
   textbox->setHtml(message);
   adjustSize();
}

// Close
void US_LongMessageBox::close_diag(void) {
   accept();
}

// Overloaded method to adjust size of message box
void US_LongMessageBox::adjustSize() {
   textbox->adjustSize();
   QWidget::adjustSize();
   QSize wsize = frameSize(); // message box size, currently
   QSize tsize = textbox->frameSize(); // text box size
   QSize bsize = wsize - tsize; // border dimensions

   if (tsize.width() > wsize.width() || tsize.height() > wsize.height())
      bsize = QSize(4, 32); // minimum border dimensions

   QStringList alltext = textbox->toPlainText().split("\n"); // text lines
   int nline = alltext.size();
   int longx = -1;
   int maxc = -1;

   for (int ii = 0; ii < nline; ii++) { // find the longest line and save its index
      int llen = alltext[ ii ].length();

      if (llen > maxc) {
         maxc = llen;
         longx = ii;
      }
   }

   // Determine the size needed for current text in current font
   QFontMetrics fm(textbox->font());
   int fontw = fm.width(alltext[ longx ]) + fm.width("WW");
   int fonth = fm.lineSpacing() * (nline + 3);
   fontw = ((fontw / 20 + 1) * 20);
   fonth = ((fonth / 20 + 1) * 20);

   tsize = QSize(fontw, fonth);
   wsize = tsize + bsize;

   textbox->resize(tsize); // re-size text box
   resize(wsize); // re-size dialog
}
