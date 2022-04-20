#include "select_program.h"
#include "us_gui_settings.h"

selectProgram::selectProgram(int& st):US_WidgetsDialog(nullptr)
{
    setWindowTitle( tr( "Pseudo-Absorbance Program" ) );
    setPalette( US_GuiSettings::frameColor() );

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *h1Layout = new QHBoxLayout();
    QHBoxLayout *h2Layout = new QHBoxLayout();

    pb_pAbs = us_pushbutton(tr("Convert Intensity \n to \n Pseudo-Absorbance"),
                            true, 1);
    pb_pAbs->setMinimumWidth(200);
    pb_refScan = us_pushbutton(tr("Add / Edit \n Reference \n Scans"), true, 1);
    pb_refScan->setMinimumWidth(200);

    pb_open = new QPushButton();
    pb_open->setText("Open");
    pb_open->setIcon(this->style()->standardIcon(QStyle::SP_DialogApplyButton));
    pb_close = new QPushButton();
    pb_close->setText("Close");
    pb_close->setIcon(this->style()->standardIcon(QStyle::SP_DialogCancelButton));
    pb_open->setDisabled(true);

    h1Layout->addWidget(pb_pAbs);
    h1Layout->addWidget(pb_refScan);
    h2Layout->addStretch(1);
    h2Layout->addWidget(pb_close);
    h2Layout->addWidget(pb_open);
    h2Layout->addStretch(1);
    mainLayout->addLayout(h1Layout);
    mainLayout->addLayout(h2Layout);

    setLayout(mainLayout);
    choice = &st;
    color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
//    color = QColor("yellow");
    connect(pb_close, SIGNAL(clicked()), this, SLOT(exit()));
    connect(pb_open, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_pAbs, SIGNAL(clicked()), this, SLOT(select_pAbs()));
    connect(pb_refScan, SIGNAL(clicked()), this, SLOT(select_refScan()));
}

void selectProgram::exit(){
    *choice = -1;
    this->close();
}

void selectProgram::select_refScan(){
    (*choice) = S_REF_SCAN;
    pb_open->setEnabled(true);
    turn_on(pb_refScan, true);
    turn_on(pb_pAbs, false);
    return;
}

void selectProgram::select_pAbs(){
    (*choice) = S_ABS;
    pb_open->setEnabled(true);
    turn_on(pb_refScan, false);
    turn_on(pb_pAbs, true);
    return;
}

void selectProgram::turn_on(QPushButton *button, bool status){
    QString qs = "QPushButton { background-color: %1 }"; //.arg(col.name());
    if (status)
        button->setStyleSheet(qs.arg("green"));
    else
        button->setStyleSheet(qs.arg(color.name()));
    button->show();
    return;
}
