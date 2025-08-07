//! \file us_get_edit.cpp

#include "us_get_edit.h"
#include "us_gui_settings.h"
#include "us_settings.h"
#include "us_widgets.h"

US_GetEdit::US_GetEdit(int &i, QStringList &filenames) : US_WidgetsDialog(0, 0), index(i) {
   setWindowTitle(tr("Load Edits From DB"));
   setPalette(US_GuiSettings::frameColor());

   QVBoxLayout *main = new QVBoxLayout(this);
   main->setSpacing(2);
   main->setContentsMargins(2, 2, 2, 2);

   lw = new US_ListWidget;
   lw->addItems(filenames);
   main->addWidget(lw);

   // Button Row
   QHBoxLayout *buttons = new QHBoxLayout;

   QPushButton *pb_cancel = us_pushbutton(tr("Cancel"));
   connect(pb_cancel, SIGNAL(clicked()), SLOT(reject()));
   buttons->addWidget(pb_cancel);

   QPushButton *pb_accept = us_pushbutton(tr("Select"));
   connect(pb_accept, SIGNAL(clicked()), SLOT(select()));
   buttons->addWidget(pb_accept);

   main->addLayout(buttons);
   resize(250, 150);
}

void US_GetEdit::select(void) {
   index = lw->currentRow();
   accept();
}
