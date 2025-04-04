#include "us_abde_main.h"
#include "us_license_t.h"
#include "us_license.h"

//! \brief Main program for US_Buoyancy. Loads translators and starts
//         the class US_FitMeniscus.

int main( int argc, char* argv[] )
{
   QApplication application( argc, argv );

   #include "main1.inc"

   // License is OK.  Start up.

   US_ABDE_MW w;
   w.show();                   //!< \memberof QWidget
   return application.exec();  //!< \memberof QApplication
}


US_ABDE_MW::US_ABDE_MW() : US_Widgets()
{
    setPalette( US_GuiSettings::frameColorDefault() );

    int width = 350;
    pb_fitter = us_pushbutton("Peak Decomposition");
    pb_fitter->setMinimumWidth(width);
    pb_normalizer = us_pushbutton("Profile Normalization");
    pb_normalizer->setMinimumWidth(width);

    QHBoxLayout* prog_lyt = new QHBoxLayout();
    prog_lyt->addStretch(1);
    prog_lyt->addWidget(pb_fitter);
    prog_lyt->addWidget(pb_normalizer);
    prog_lyt->addStretch(1);

    hline = new QLabel();
    hline->setMaximumHeight(2);
    hline->setStyleSheet("background-color: darkGray;");

    main_lyt = new QVBoxLayout(this);
    main_lyt->addLayout(prog_lyt);
    main_lyt->addWidget(hline);
    fitter = new US_Buoyancy("AUTO");
    normalizer = new US_Norm_Profile();

    //TEST
    // QMap<QString, QString> protocol_details;
    // //ABDE-MWL
    // protocol_details[ "invID_passed" ] = QString("165");
    // protocol_details[ "protocolName" ] = QString("GMP-test-ABDE-fromDisk");
    // protocol_details[ "aprofileguid" ] = QString("6c376179-6eda-47e9-b699-3eef63c6fe6e");
    // protocol_details[ "filename" ]     = QString("AAV_GMP_test_030325-run2366-dataDiskRun-1515");
    // protocol_details[ "analysisIDs"  ] = QString("");
    // protocol_details[ "expType" ]      = QString("ABDE");
    // protocol_details[ "dataSource" ]   = QString("dataDiskAUC");
    // protocol_details[ "statusID" ]     = QString("588");
    // protocol_details[ "autoflowID" ]   = QString("1515");
    // normalizer = new US_Norm_Profile( protocol_details );
    // normalizer = new US_Norm_Profile( "AUTO" );
    //END TEST

    
    main_lyt->addWidget(fitter);
    main_lyt->addWidget(normalizer);
    normalizer->setVisible(false);
    turn_on(pb_fitter, true);
    turn_on(pb_normalizer, false);
    setWindowTitle(fitter->windowTitle());
    main_lyt->setMargin(1);
    main_lyt->setSpacing(2);
//    main_lyt->setSizeConstraint(QLayout::SetMinimumSize);
    this->setLayout(main_lyt);

    connect(fitter,     SIGNAL(widgetClosed()), this, SLOT(close()));
    connect(normalizer, SIGNAL(widgetClosed()), this, SLOT(close()));
    connect(pb_fitter,     SIGNAL(clicked()), this, SLOT(sel_fitter()));
    connect(pb_normalizer, SIGNAL(clicked()), this, SLOT(sel_normalizer()));
}

void US_ABDE_MW::sel_fitter(){
    turn_on(pb_fitter, true);
    turn_on(pb_normalizer, false);
    fitter->setVisible(true);
    normalizer->setVisible(false);
    setWindowTitle(fitter->windowTitle());
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(sizeHint());
}

void US_ABDE_MW::sel_normalizer(){
    turn_on(pb_fitter, false);
    turn_on(pb_normalizer, true);
    fitter->setVisible(false);
    normalizer->setVisible(true);
    setWindowTitle(normalizer->windowTitle());
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    resize(sizeHint());
}

void US_ABDE_MW::turn_on(QPushButton *button, bool state){
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
