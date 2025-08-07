//! \file us_model_adpars.cpp

#include "us_model_adpars.h"

#include "us_gui_settings.h"
#include "us_settings.h"

// Main constructor with addition-parameters count and reference to params list
US_ModelAdPars::US_ModelAdPars(int a_napars, QList<double>& a_udpars)
    : US_WidgetsDialog(0, 0), napars(a_napars), aud_pars(a_udpars) {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle(tr("Model Additional Parameters"));
  setPalette(US_GuiSettings::frameColor());
  setMinimumSize(200, 100);

  // Main layout
  QGridLayout* main = new QGridLayout(this);
  main->setContentsMargins(2, 2, 2, 2);
  main->setSpacing(2);

  // Main banner
  QLabel* lb_mbanner = us_banner(tr("Please provide additional information:"));
  int wwid = 0;
  int whgt = 0;
  int row = 0;

  if (napars == 1) {  // Single parameter:  stoichiometry
    lb_mbanner->setText(
        tr("Please provide the Stoichiometry\n"
           "for the Monomer - N-mer Equilibrium:"));
    QLabel* lb_stoich1 = us_label(tr("N-Mer:"));
    le_stoich1 = us_lineedit();
    main->addWidget(lb_mbanner, row, 0, 2, 6);
    row += 2;
    main->addWidget(lb_stoich1, row, 0, 1, 4);
    main->addWidget(le_stoich1, row++, 4, 1, 2);
    lb_mbanner->adjustSize();
    QSize wsize = lb_mbanner->frameSize();
    wwid = wsize.width() / 3 + 8;
    whgt = wsize.height() / 2;

    le_stoich1->setText("2");
  }

  else if (napars == 2) {  // Dual parameters:  stoichiometries
    lb_mbanner->setText(
        tr("Please provide the Stoichiometry\n"
           "for the Monomer - N-mer - M-mer Equilibrium:"));
    QLabel* lb_stoich1 = us_label(tr("N-Mer:"));
    le_stoich1 = us_lineedit();
    QLabel* lb_stoich2 = us_label(tr("M-Mer:"));
    le_stoich2 = us_lineedit();
    main->addWidget(lb_mbanner, row, 0, 2, 6);
    row += 2;
    main->addWidget(lb_stoich1, row, 0, 1, 4);
    main->addWidget(le_stoich1, row++, 4, 1, 2);
    main->addWidget(lb_stoich2, row, 0, 1, 4);
    main->addWidget(le_stoich2, row++, 4, 1, 2);
    lb_mbanner->adjustSize();
    QSize wsize = lb_mbanner->frameSize();
    wwid = wsize.width() / 3 + 8;
    whgt = wsize.height() / 2;

    le_stoich1->setText("2");
    le_stoich2->setText("3");
  }

  else if (napars == 4) {  // 4 parameters; MW parameters and common vbar
    QLabel* lb_nmwslot = us_label(tr("Number of MW Slots:"));
    le_nmwslot = us_lineedit();
    QLabel* lb_llimmwr = us_label(tr("Lower Limit of MW Range:"));
    le_llimmwr = us_lineedit();
    QLabel* lb_ulimmwr = us_label(tr("Upper Limit of MW Range:"));
    le_ulimmwr = us_lineedit();
    QLayout* lo_cmnvbar =
        us_checkbox(tr("Use common vbar value"), ck_cmnvbar, true);
    le_cmnvbar = us_lineedit();
    main->addWidget(lb_mbanner, row++, 0, 1, 6);
    main->addWidget(lb_nmwslot, row, 0, 1, 4);
    main->addWidget(le_nmwslot, row++, 4, 1, 2);
    main->addWidget(lb_llimmwr, row, 0, 1, 4);
    main->addWidget(le_llimmwr, row++, 4, 1, 2);
    main->addWidget(lb_ulimmwr, row, 0, 1, 4);
    main->addWidget(le_ulimmwr, row++, 4, 1, 2);
    main->addLayout(lo_cmnvbar, row, 0, 1, 4);
    main->addWidget(le_cmnvbar, row++, 4, 1, 2);
    ck_cmnvbar->adjustSize();
    QSize wsize = ck_cmnvbar->frameSize();
    wwid = wsize.width() / 2 + 8;
    whgt = wsize.height();

    le_nmwslot->setText("100");
    le_llimmwr->setText("1.000e+04");
    le_ulimmwr->setText("5.000e+05");
    ck_cmnvbar->setChecked(true);
    le_cmnvbar->setText("0.7200");
  }

  // Button Row
  QHBoxLayout* buttons = new QHBoxLayout;

  QPushButton* pb_help = us_pushbutton(tr("Help"));
  connect(pb_help, SIGNAL(clicked()), SLOT(help()));
  buttons->addWidget(pb_help);

  QPushButton* pb_cancel = us_pushbutton(tr("Cancel"));
  connect(pb_cancel, SIGNAL(clicked()), SLOT(cancelled()));
  buttons->addWidget(pb_cancel);

  QPushButton* pb_accept = us_pushbutton(tr("OK"));
  connect(pb_accept, SIGNAL(clicked()), SLOT(selected()));
  buttons->addWidget(pb_accept);

  main->addLayout(buttons, row++, 0, 1, 6);

  // Resize to fit elements added
  adjustSize();
  QSize wsize = QSize(wwid * 3 + 4, whgt * row + 32);
  resize(wsize);
}

// Cancel button:  no parameters returned
void US_ModelAdPars::cancelled() {
  napars = -1;
  aud_pars.clear();

  reject();
  close();
}

// Select Model button:  set up to return data information
void US_ModelAdPars::selected() {
  aud_pars.clear();

  if (napars == 1 ||
      napars == 2) {  // Get stoichiometr(y/ies) if 1 or 2 parameters
    aud_pars << le_stoich1->text().toDouble();

    if (napars == 2) aud_pars << le_stoich2->text().toDouble();
  }

  else if (napars == 4) {  // Get MW/vbar parameters if 4 parameters
    aud_pars << le_nmwslot->text().toDouble();
    aud_pars << le_llimmwr->text().toDouble();
    aud_pars << le_ulimmwr->text().toDouble();

    if (ck_cmnvbar->isChecked()) {
      aud_pars << le_cmnvbar->text().toDouble();
    }

    else {
      aud_pars << 0.0;
    }
  }

  accept();  // Signal that selection was accepted
  close();
}
