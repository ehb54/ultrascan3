#include <QApplication>
#include "us_pseudo_absorbance.h"
#include "us_license_t.h"
#include "us_license.h"
#include "us_extern.h"


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    #include "main1.inc"

    // License is OK.  Start up.

    US_PseudoAbsorbance w;
    w.show();                   //!< \memberof QWidget
    return application.exec();

};

US_PseudoAbsorbance::US_PseudoAbsorbance() : US_Widgets()
{
    setWindowTitle( tr( "Pseudo-Absorbance Program" ) );
    setPalette( US_GuiSettings::frameColorDefault() );

    pb_addRef = us_pushbutton("Reference Scan Calculation");
    pb_cnvtSc = us_pushbutton("Pseudo-Absorbance Conversion");
    pb_rmri   = us_pushbutton("Radial Invariant Noise Correction");
    int width = 350;
    pb_addRef->setMinimumWidth(width);
    pb_cnvtSc->setMinimumWidth(width);
    pb_rmri->setMinimumWidth(width);
    QHBoxLayout* prog_lyt = new QHBoxLayout();
    prog_lyt->addStretch(1);
    prog_lyt->addWidget(pb_cnvtSc);
    prog_lyt->addWidget(pb_addRef);
    prog_lyt->addWidget(pb_rmri);
    prog_lyt->addStretch(1);

    hline = new QLabel();
    hline->setMaximumHeight(2);
    hline->setStyleSheet("background-color: darkGray;");

    main_lyt = new QVBoxLayout(this);
    main_lyt->addLayout(prog_lyt);
    main_lyt->addWidget(hline);
    convertScan = new US_ConvertScan();
    main_lyt->addWidget(convertScan);
    program_state = CONVERT;
    turn_on(pb_addRef, false);
    turn_on(pb_cnvtSc, true);
    main_lyt->setMargin(1);
    main_lyt->setSpacing(2);
//    main_lyt->setSizeConstraint(QLayout::SetMinimumSize);
    this->setLayout(main_lyt);

    connect(convertScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_addRef, SIGNAL(clicked()), this, SLOT(select_addRef()));
    connect(pb_cnvtSc, SIGNAL(clicked()), this, SLOT(select_cnvtSc()));
    connect(pb_rmri, SIGNAL(clicked()), this, SLOT(select_rmRi()));
}

void US_PseudoAbsorbance::select_cnvtSc(){
    new_program(CONVERT);
}

void US_PseudoAbsorbance::select_addRef(){
    new_program(ADDREFSCAN);
}

void US_PseudoAbsorbance::select_rmRi(){
    new_program(REMOVERI);
}


void US_PseudoAbsorbance::turn_on(QPushButton *button, bool state){
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    QString on = tr("QPushButton { background-color: #00ff1e;"
                    "font-size: 11pt; font-weight: bold; color: #000000;}" );
    QString off = tr("QPushButton { background-color: %1;"
                     "font-size: 10pt; font-weight: normal; color: #2b2b2b;}").arg(color.name());
    if (state)
        button->setStyleSheet(on);
    else
        button->setStyleSheet(off);
    button->show();
    return;
}

template<class T> bool US_PseudoAbsorbance::cls_program(T *p){
    bool state;
    if (p->hasData){
        int mb = QMessageBox::question(this, "Warning!",
                                       tr("Some data has been loaded into "
                                          "the current program. Switching to "
                                          "another program will close the current "
                                          "program.\nDo you want to proceed?"),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);
        if (mb == QMessageBox::No)
            state = false;
        else
            state = true;
    } else
        state = true;
    if (state){
        p->disconnect();
        p->close();
        main_lyt->removeWidget(p);
    }
    return state;
}

void US_PseudoAbsorbance::new_program(int state){
    if (program_state == state)
        return;
    turn_on(pb_addRef, false);
    turn_on(pb_cnvtSc, false);
    turn_on(pb_rmri, false);
    if (program_state == ADDREFSCAN){
        if (! cls_program(addRefScan)){
            turn_on(pb_addRef, true);
            return;
        }
    } else if (program_state == CONVERT){
        if (! cls_program(convertScan)){
            turn_on(pb_cnvtSc, true);
            return;
        }
    } else if (program_state == REMOVERI){
        if (! cls_program(removeRi)){
            turn_on(pb_rmri, true);
            return;
        }
    }

    if (state == ADDREFSCAN){
        addRefScan = new US_AddRefScan();
        main_lyt->addWidget(addRefScan);
        connect(addRefScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
        program_state = ADDREFSCAN;
        turn_on(pb_addRef, true);
    } else if (state == CONVERT){
        convertScan = new US_ConvertScan();
        main_lyt->addWidget(convertScan);
        connect(convertScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
        program_state = CONVERT;
        turn_on(pb_cnvtSc, true);
    } else if (state == REMOVERI){
        removeRi = new US_RemoveRI();
        main_lyt->addWidget(removeRi);
        connect(removeRi->pb_close, SIGNAL(clicked()), this, SLOT(close()));
        program_state = REMOVERI;
        turn_on(pb_rmri, true);
    }

    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(sizeHint());
    return;
}

