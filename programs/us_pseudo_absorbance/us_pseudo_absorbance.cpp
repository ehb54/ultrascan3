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
//    QTabWidget *tabs = new QTabWidget();
//    tabs->addTab(convertScan, "Convert Intensity to Absorbance");
//    tabs->addTab(addRefScan, "Add New Reference Scans");
//    tabs->setTabShape(QTabWidget::Triangular);
//    tabs->tabBar()->setMinimumWidth(800);
//    addRefScan->hide();

//    QStringList styleSheet;
//    styleSheet << "QTabWidget::pane {border-top: 2px solid #C2C7CB;}";
//    styleSheet << "QTabWidget::tab-bar {left: 5px;}";
//    styleSheet << "QTabBar::tab:selected {font: bold; color: black;}";
//    styleSheet << "QTabBar::tab:!selected {font: normal; color: gray;}";
//    styleSheet << "QTabBar::tab {background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                                    "stop: 0 #E1E1E1, stop: 0.4 #DDDDDD,"
//                                    "stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3);"
//                               "border: 2px solid #C4C4C3;"
//                               "border-bottom-color: #C2C7CB;"
//                               "border-top-left-radius: 4px;"
//                               "border-top-right-radius: 4px;"
//                               "min-width: 8ex;"
//                               "padding: 2px;}";
//    styleSheet << "QTabBar::tab:selected, QTabBar::tab:hover {"
//                     "background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,"
//                           "stop: 0 #fafafa, stop: 0.4 #f4f4f4,"
//                           "stop: 0.5 #e7e7e7, stop: 1.0 #fafafa);}";
//    styleSheet << "QTabBar::tab:selected {border-color: #9B9B9B; border-bottom-color: #C2C7CB;}";
//    styleSheet << "QTabBar::tab:!selected {margin-top: 2px;}";
//    styleSheet << "QTabBar::tab:selected {margin-left: -4px; margin-right: -4px;}";
//    styleSheet << "QTabBar::tab:first:selected {margin-left: 0;}";
//    styleSheet << "QTabBar::tab:last:selected {margin-right: 0;}";
//    styleSheet << "QTabBar::tab:only-one {margin: 0;};";

//    tabs->setStyleSheet(styleSheet.join(" "));


    pb_addRef = us_pushbutton("Add New Reference Scans");
    pb_cnvtSc = us_pushbutton("Convert Intensity to Absorbance");
    pb_addRef->setMinimumWidth(400);
    pb_cnvtSc->setMinimumWidth(400);
    QHBoxLayout* prog_lyt = new QHBoxLayout();
    prog_lyt->addStretch(1);
    prog_lyt->addWidget(pb_cnvtSc);
    prog_lyt->addWidget(pb_addRef);
    prog_lyt->addStretch(1);

    hline = new QLabel();
    hline->setMaximumHeight(2);
    hline->setStyleSheet("background-color: darkGray;");

    main_lyt = new QVBoxLayout(this);
    main_lyt->addLayout(prog_lyt);
    main_lyt->addWidget(hline);
    convertScan = new US_ConvertScan();
    main_lyt->addWidget(convertScan);
    curr_prgm = tr("convertScan");
    turn_on(pb_addRef, false);
    turn_on(pb_cnvtSc, true);
    main_lyt->setMargin(1);
    main_lyt->setSpacing(2);
//    main_lyt->setSizeConstraint(QLayout::SetMinimumSize);
    this->setLayout(main_lyt);

    connect(convertScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_addRef, SIGNAL(clicked()), this, SLOT(select_addRef()));
    connect(pb_cnvtSc, SIGNAL(clicked()), this, SLOT(select_cnvtSc()));
}

void US_PseudoAbsorbance::select_cnvtSc(){
    if (curr_prgm == "convertScan")
        return;
    if (addRefScan->hasData){
        int ck = QMessageBox::question(this, "Warning!",
                                       tr("Some data has been loaded into "
                                          "the current program. Switching to "
                                          "another program will close the current "
                                          "program.\nDo you want to proceed?"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
        if (ck == QMessageBox::No)
            return;
    }
    addRefScan->disconnect();
    addRefScan->close();
    main_lyt->removeWidget(addRefScan);
    convertScan = new US_ConvertScan();
    main_lyt->addWidget(convertScan);
    turn_on(pb_addRef, false);
    turn_on(pb_cnvtSc, true);
    connect(convertScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
    curr_prgm = tr("convertScan");
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(sizeHint());
    return;
}

void US_PseudoAbsorbance::select_addRef(){
    if (curr_prgm == "addRefScan")
        return;
    if (convertScan->hasData){
        int ck = QMessageBox::question(this, "Warning!",
                                       tr("Some data has been loaded into "
                                          "the current program. Switching to "
                                          "another program will close the current "
                                          "program.\nDo you want to proceed?"),
                                          QMessageBox::Yes | QMessageBox::No,
                                          QMessageBox::No);
        if (ck == QMessageBox::No)
            return;
    }
    convertScan->disconnect();
    convertScan->close();
    main_lyt->removeWidget(convertScan);
    addRefScan= new US_AddRefScan();
    main_lyt->addWidget(addRefScan);
    turn_on(pb_addRef, true);
    turn_on(pb_cnvtSc, false);
    connect(addRefScan->pb_close, SIGNAL(clicked()), this, SLOT(close()));
    curr_prgm = tr("addRefScan");
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(sizeHint());
    return;
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


