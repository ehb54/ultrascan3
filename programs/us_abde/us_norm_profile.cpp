#include <QApplication>
#include "us_license_t.h"
#include "us_license.h"
#include "us_norm_profile.h"
#include "us_load_auc.h"
#include "us_passwd.h"
#include <QFileInfo>

//Alt. constr.
US_Norm_Profile::US_Norm_Profile( QString auto_mode ): US_Widgets()
{
    us_auto_mode = true;
    us_auto_mode_report = false;
    //this->protocol_details = protocol_details_p;
    
    setWindowTitle("Buoyancy Equilibrium Data Analysis");
    QPalette p = US_GuiSettings::frameColorDefault();
    setPalette( p );

    QLabel* lb_runinfo = us_label("Data Info");
    le_runinfo = us_lineedit("", 0, true);
    QHBoxLayout *runinfo_lyt = new QHBoxLayout();
    runinfo_lyt->addWidget(lb_runinfo);
    runinfo_lyt->addWidget(le_runinfo);

    QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
//    connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
//    specs->addWidget( pb_investigator, s_row, 0 );

    if ( US_Settings::us_inv_level() < 1 )
       pb_investigator->setEnabled( false );

    
    int id = US_Settings::us_inv_ID();
    //int id = protocol_details[ "invID_passed" ].toInt();
    QString number  = ( id > 0 ) ?
                         QString::number( US_Settings::us_inv_ID() ) + ": "
                              : "";
    le_investigator = us_lineedit( number + US_Settings::us_inv_name(), -1, true );
    le_investigator->setMinimumWidth(150);
//    specs->addWidget( le_investigator, s_row++, 1, 1, 3 );
    QHBoxLayout *inv_lyt = new QHBoxLayout();
    inv_lyt->addWidget(pb_investigator);
    inv_lyt->addWidget(le_investigator);

    disk_controls = new US_Disk_DB_Controls;

    pb_load = us_pushbutton("Load Data");
    pb_reset = us_pushbutton("Reset Data");
    pb_save = us_pushbutton("Save");
    pb_close = us_pushbutton("Close");

    //chanels, next
    QLabel *lb_channList = us_banner("Channel Selection");
    QLabel *lb_chann = us_label("Channel");
    cb_chann      = us_comboBox();
    pb_next_chann = us_pushbutton("Next Channel");
    pb_prev_chann = us_pushbutton("Previous Channel");
    connect( cb_chann,  SIGNAL( currentIndexChanged( int ) ), 
	     SLOT  ( new_chann_auto         ( int ) ) );
    connect( pb_next_chann,   SIGNAL( clicked() ),
	     SLOT( next_chann_auto()   ) );
    connect( pb_prev_chann,   SIGNAL( clicked() ),
	     SLOT( prev_chann_auto()   ) );
    QGridLayout* chann_lyt = new QGridLayout();
    chann_lyt->addWidget(lb_chann,        0, 0, 1, 1);
    chann_lyt->addWidget(cb_chann,        0, 1, 1, 1);
    chann_lyt->addWidget(pb_prev_chann,   1, 0, 1, 1);
    chann_lyt->addWidget(pb_next_chann,   1, 1, 1, 1);
    
    QGridLayout* load_lyt = new QGridLayout();
    load_lyt->addWidget(pb_load,   0, 0, 1, 1);
    load_lyt->addWidget(pb_reset,  0, 1, 1, 1);
    load_lyt->addWidget(pb_close,       1, 0, 1, 1);
    load_lyt->addWidget(pb_save,        1, 1, 1, 1);

    QLabel *lb_inpList = us_banner("List of File(s)");
    lw_inpData = us_listwidget();

    QLabel *lb_selList = us_banner("Selected File(s)");
    lw_selData = us_listwidget();

    pb_rmItem = us_pushbutton("Remove Item");
    pb_cleanList = us_pushbutton("Clean List");

    ckb_xrange = new QCheckBox();
    QGridLayout *us_xrange = us_checkbox("Limit Radius",
                                                ckb_xrange);
    pb_pick_rp = us_pushbutton("Pick Two Points", false);

    ckb_norm_max = new QCheckBox();
    QGridLayout *us_norm_max = us_checkbox(" Normalize\n by Maximum",
                                         ckb_norm_max);
    pb_pick_norm = us_pushbutton("Pick a Point");

    QGridLayout *bottom_lyt = new QGridLayout();
    //bottom_lyt->addWidget(pb_rmItem,         0, 0, 1, 1);
    //bottom_lyt->addWidget(pb_cleanList,      0, 1, 1, 1);
    //bottom_lyt->addLayout(us_xrange,         1, 0, 1, 1);
    //bottom_lyt->addWidget(pb_pick_rp,        1, 1, 1, 1);
    bottom_lyt->addLayout(us_norm_max,       2, 0, 1, 1);
    bottom_lyt->addWidget(pb_pick_norm,      2, 1, 1, 1);

    //help && save btn
    int ihgt        = pb_pick_norm->height();
    QSpacerItem* spacer2 = new QSpacerItem( 10,25*ihgt, QSizePolicy::Expanding);
    bottom_lyt->addItem( spacer2,  3,  0, 1, 2 );

    QPushButton* pb_help   = us_pushbutton( tr( "Help" ) );
    pb_save_auto           = us_pushbutton( tr( "Save Profiles" ) );
    connect( pb_save_auto,  SIGNAL( clicked() ), 
	     this, SLOT  ( save_auto() ) );
    pb_save_auto->setEnabled(false);
    
    bottom_lyt->addWidget( pb_help,         4,   0, 1, 1 );
    bottom_lyt->addWidget( pb_save_auto,    4,   1, 1, 1 );

    ckb_rawData = new QCheckBox();
    QGridLayout *rawData_lyt = us_checkbox("Raw Data", ckb_rawData);
    ckb_rawData->setChecked(true);

    ckb_integral = new QCheckBox();
    QGridLayout *integral_lyt = us_checkbox("Integral", ckb_integral);
    ckb_integral->setChecked(true);

    ckb_norm = new QCheckBox();
    QGridLayout *norm_lyt = us_checkbox("Normalized", ckb_norm);
    ckb_norm->setChecked(true);

    ckb_legend = new QCheckBox();
    QGridLayout *legend_lyt = us_checkbox("Legend", ckb_legend);
    ckb_legend->setChecked(true);

    ckb_ranges = new QCheckBox();
    QGridLayout *ranges_lyt = us_checkbox("Ranges", ckb_ranges);
    ckb_ranges->setChecked(true);
    
    QHBoxLayout *intg_lyt = new QHBoxLayout();
    intg_lyt->addStretch(1);
    intg_lyt->addLayout(rawData_lyt);
    intg_lyt->addLayout(integral_lyt);
    intg_lyt->addLayout(norm_lyt);
    intg_lyt->addLayout(legend_lyt);
    intg_lyt->addLayout(ranges_lyt);
    intg_lyt->addStretch(1);

    usplot = new US_Plot( plot, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot->setMinimumSize( 700, 400 );
    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft  , true );
    //plot->enableAxis( QwtPlot::yRight  , true );
    plot->setCanvasBackground(QBrush(Qt::white));

    QVBoxLayout* main_lyt = new QVBoxLayout();
    QHBoxLayout* body_lyt = new QHBoxLayout();
    QVBoxLayout* left_lyt = new QVBoxLayout();
    QVBoxLayout* right_lyt = new QVBoxLayout();

    left_lyt->addLayout(inv_lyt);
    //left_lyt->addLayout(disk_controls);
    left_lyt->addWidget(lb_channList);
    left_lyt->addLayout(chann_lyt);
    left_lyt->addLayout(load_lyt);
    left_lyt->addWidget(lb_inpList);
    left_lyt->addWidget(lw_inpData);
    left_lyt->addWidget(lb_selList);
    left_lyt->addWidget(lw_selData);
    left_lyt->addLayout(bottom_lyt);
    left_lyt->addStretch(1);

    right_lyt->addLayout(intg_lyt);
    right_lyt->addLayout(usplot);

    left_lyt->setContentsMargins( 1, 1, 1, 1);
    left_lyt->setSpacing(1);
    right_lyt->setContentsMargins( 1, 1, 1, 1);
    right_lyt->setSpacing(1);

    body_lyt->addLayout(left_lyt, 1);
    body_lyt->addLayout(right_lyt, 3);
    body_lyt->setContentsMargins( 1, 1, 1, 1);
    body_lyt->setSpacing(0);

    main_lyt->addLayout(runinfo_lyt);
    main_lyt->addLayout(body_lyt);
    main_lyt->setContentsMargins( 0, 0, 0, 0);
    main_lyt->setSpacing(0);
    this->setLayout(main_lyt);

    picker = new US_PlotPicker(plot);
    picker->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );
    picker->setRubberBandPen(QPen(Qt::red));
    picker->setTrackerPen(QPen(Qt::red));
    plotData();
    picker_state = XNONE;

    connect(pb_load, SIGNAL(clicked()), this, SLOT(slt_loadAUC()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));
    connect(lw_inpData, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slt_addRmItem(QListWidgetItem *)));
    connect(lw_inpData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_inItemSel(int )));
    connect(lw_selData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_outItemSel(int )));
    connect(pb_rmItem, SIGNAL(clicked()), this, SLOT(slt_rmItem()));
    connect(pb_cleanList, SIGNAL(clicked()), this, SLOT(slt_cleanList()));

    connect(pb_pick_rp, SIGNAL(clicked()), this, SLOT(slt_pickRange()));
    connect(pb_pick_norm, SIGNAL(clicked()), this, SLOT(slt_pickPoint()));
    //connect(pb_pick_norm, SIGNAL(clicked()), this, SLOT(slt_pickPoint_auto()));
    connect(ckb_xrange, SIGNAL(stateChanged(int)), this, SLOT(slt_xrange(int)));
    connect(ckb_legend, SIGNAL(stateChanged(int)), this, SLOT(slt_legend(int)));
    connect(ckb_integral, SIGNAL(stateChanged(int)), this, SLOT(slt_integral(int)));
    connect(ckb_norm, SIGNAL(stateChanged(int)), this, SLOT(slt_norm(int)));
    connect(ckb_rawData, SIGNAL(stateChanged(int)), this, SLOT(slt_rawData(int)));
    connect(ckb_norm_max, SIGNAL(stateChanged(int)), this, SLOT(slt_norm_by_max(int)));
    connect(ckb_ranges, SIGNAL(stateChanged(int)), this, SLOT(slt_ranges(int)));
    connect(picker, SIGNAL(cMouseUp(const QPointF&)),
            this,   SLOT(slt_mouse(const QPointF&)));
    ckb_norm_max->setCheckState(Qt::Checked);

    //hide 
    pb_investigator ->hide();
    le_investigator ->hide();
    pb_load->hide();
    pb_reset->hide();
    pb_save->hide();
    pb_close->hide();
    pb_rmItem->hide();
    pb_cleanList->hide();
    ckb_xrange->hide();
    pb_pick_rp->hide();
    lb_inpList->hide(); 
    lw_inpData->hide(); 
    lb_selList->hide(); 
    lw_selData ->hide();
    
    //TEST: SWL, MWL
    // QMap<QString, QString> protocol_details;

    // //ABDE-MWL (CANADA-test)
    // protocol_details[ "invID_passed" ] = QString("2");
    // protocol_details[ "protocolName" ] = QString("GMP-test-ABDE-fromDisk");
    // protocol_details[ "aprofileguid" ] = QString("37a4c516-cf7f-4470-9102-8a98805c2cae");
    // protocol_details[ "filename" ]     = QString("AAV_GMP_test_030325-run2366-dataDiskRun-71");
    // protocol_details[ "analysisIDs"  ] = QString("");
    // protocol_details[ "expType" ]      = QString("ABDE");
    // protocol_details[ "dataSource" ]   = QString("dataDiskAUC");
    // protocol_details[ "statusID" ]     = QString("46");
    // protocol_details[ "autoflowID" ]   = QString("71");
    // //protocol_details["ssf_dir_name"]   = QString("/home/alexey/ultrascan/imports/SSF-AAV_GMP_test_030325-run2366-dataDiskRun-71");
    // /** When for REPORT: These next fields will be read from autoflowAnalysisABDE record: 
    // 	protocol_details["abde_etype"]     = QString("MWL");
    // 	protocol_details["directory_for_gmp"] = QString("");
    // 	protocol_details[ "baseline_corrections" ] = QString("");
    // 	protocol_details["filename_abde"]  = QString("SSF-AAV_GMP_test_030325-run2366-dataDiskRun-71");
    // 	protocol_details[ "channels_to_radial_ranges" ] = QString("2A:6.2-6.5,6.6-6.9;2B:5.8-7;4A:6.1-6.5,6.6-6.94;4B:6.25-6.55,6.65-7");
    // 	protocol_details[ "x_normalizations" ] = QString("");
    // /** plus info on x_norm form DB's JSON -- for MWL only**/

    // //ABDE-SWL (CANADA-test)
    // protocol_details[ "invID_passed" ] = QString("2");
    // protocol_details[ "protocolName" ] = QString("GMP-test-ABDE-Disk-SingleWVL");
    // protocol_details[ "aprofileguid" ] = QString("0a033cbb-8cc6-4efd-be99-81772f0c7d02");
    // protocol_details[ "filename" ]     = QString("McCue_AAVCsCl_25JAN25-run2255-dataDiskRun-72");
    // protocol_details[ "analysisIDs"  ] = QString("");
    // protocol_details[ "expType" ]      = QString("ABDE");
    // protocol_details[ "dataSource" ]   = QString("dataDiskAUC");
    // protocol_details[ "statusID" ]     = QString("47");
    // protocol_details[ "autoflowID" ]   = QString("72");
    // //protocol_details["ssf_dir_name"]   = QString("");
    // /** When for REPORT: These next fields will be read from autoflowAnalysisABDE record: 
    // 	protocol_details["abde_etype"]     = QString("SWL");
    // 	protocol_details["directory_for_gmp"] = QString("/home/alexey/ultrascan/results/McCue_AAVCsCl_25JAN25-run2255-dataDiskRun-72");
    // 	protocol_details[ "baseline_corrections" ] = QString("2A230:6.037,7.08,0.0175125,-0.034958");
    // 	protocol_details["filename_abde"]  = QString("McCue_AAVCsCl_25JAN25-run2255-dataDiskRun-72");
    // 	protocol_details[ "channels_to_radial_ranges" ] = QString("2A:6.2-6.5,6.6-6.9");
    // 	protocol_details[ "x_normalizations" ] = QString("");
    // /** plus info on x_norm form DB's JSON -- for MWL only**/
    
    // load_data_auto_report( protocol_details );     //<-- for REPORT
    //load_data_auto( protocol_details );             //<-- for ANALYSIS
    //END TEST
}


US_Norm_Profile::US_Norm_Profile(): US_Widgets()
{
    us_auto_mode = false;
    us_auto_mode_report = false;
    
    setWindowTitle("Buoyancy Equilibrium Data Analysis");
    QPalette p = US_GuiSettings::frameColorDefault();
    setPalette( p );

    QLabel* lb_runinfo = us_label("Data Info");
    le_runinfo = us_lineedit("", 0, true);
    QHBoxLayout *runinfo_lyt = new QHBoxLayout();
    runinfo_lyt->addWidget(lb_runinfo);
    runinfo_lyt->addWidget(le_runinfo);

    QPushButton* pb_investigator = us_pushbutton( tr( "Select Investigator" ) );
//    connect( pb_investigator, SIGNAL( clicked() ), SLOT( sel_investigator() ) );
//    specs->addWidget( pb_investigator, s_row, 0 );

    if ( US_Settings::us_inv_level() < 1 )
       pb_investigator->setEnabled( false );

    int id = US_Settings::us_inv_ID();
    QString number  = ( id > 0 ) ?
                         QString::number( US_Settings::us_inv_ID() ) + ": "
                              : "";
    le_investigator = us_lineedit( number + US_Settings::us_inv_name(), -1, true );
    le_investigator->setMinimumWidth(150);
//    specs->addWidget( le_investigator, s_row++, 1, 1, 3 );
    QHBoxLayout *inv_lyt = new QHBoxLayout();
    inv_lyt->addWidget(pb_investigator);
    inv_lyt->addWidget(le_investigator);

    disk_controls = new US_Disk_DB_Controls;

    pb_load = us_pushbutton("Load Data");
    pb_reset = us_pushbutton("Reset Data");
    pb_save = us_pushbutton("Save");
    pb_close = us_pushbutton("Close");

    QGridLayout* load_lyt = new QGridLayout();
    load_lyt->addWidget(pb_load,   0, 0, 1, 1);
    load_lyt->addWidget(pb_reset,  0, 1, 1, 1);
    load_lyt->addWidget(pb_close,       1, 0, 1, 1);
    load_lyt->addWidget(pb_save,        1, 1, 1, 1);

    QLabel *lb_inpList = us_banner("List of File(s)");
    lw_inpData = us_listwidget();

    QLabel *lb_selList = us_banner("Selected File(s)");
    lw_selData = us_listwidget();

    pb_rmItem = us_pushbutton("Remove Item");
    pb_cleanList = us_pushbutton("Clean List");

    ckb_xrange = new QCheckBox();
    QGridLayout *us_xrange = us_checkbox("Limit Radius",
                                                ckb_xrange);
    pb_pick_rp = us_pushbutton("Pick Two Points", false);

    ckb_norm_max = new QCheckBox();
    QGridLayout *us_norm_max = us_checkbox("Normalize by Maximum",
                                         ckb_norm_max);
    pb_pick_norm = us_pushbutton("Pick a Point");

    QGridLayout *bottom_lyt = new QGridLayout();
    bottom_lyt->addWidget(pb_rmItem,         0, 0, 1, 1);
    bottom_lyt->addWidget(pb_cleanList,      0, 1, 1, 1);
    bottom_lyt->addLayout(us_xrange,         1, 0, 1, 1);
    bottom_lyt->addWidget(pb_pick_rp,        1, 1, 1, 1);
    bottom_lyt->addLayout(us_norm_max,       2, 0, 1, 1);
    bottom_lyt->addWidget(pb_pick_norm,      2, 1, 1, 1);

    ckb_rawData = new QCheckBox();
    QGridLayout *rawData_lyt = us_checkbox("Raw Data", ckb_rawData);
    ckb_rawData->setChecked(true);

    ckb_integral = new QCheckBox();
    QGridLayout *integral_lyt = us_checkbox("Integral", ckb_integral);
    ckb_integral->setChecked(true);

    ckb_norm = new QCheckBox();
    QGridLayout *norm_lyt = us_checkbox("Normalized", ckb_norm);
    ckb_norm->setChecked(true);

    ckb_legend = new QCheckBox();
    QGridLayout *legend_lyt = us_checkbox("Legend", ckb_legend);
    ckb_legend->setChecked(true);

    QHBoxLayout *intg_lyt = new QHBoxLayout();
    intg_lyt->addStretch(1);
    intg_lyt->addLayout(rawData_lyt);
    intg_lyt->addLayout(integral_lyt);
    intg_lyt->addLayout(norm_lyt);
    intg_lyt->addLayout(legend_lyt);
    intg_lyt->addStretch(1);

    usplot = new US_Plot( plot, tr( "" ),
                           tr( "Radius (in cm)" ), tr( "Absorbance" ),
                           true, "", "" );
    plot->setMinimumSize( 700, 400 );
    plot->enableAxis( QwtPlot::xBottom, true );
    plot->enableAxis( QwtPlot::yLeft  , true );
    plot->setCanvasBackground(QBrush(Qt::white));

    QVBoxLayout* main_lyt = new QVBoxLayout();
    QHBoxLayout* body_lyt = new QHBoxLayout();
    QVBoxLayout* left_lyt = new QVBoxLayout();
    QVBoxLayout* right_lyt = new QVBoxLayout();

    left_lyt->addLayout(inv_lyt);
    left_lyt->addLayout(disk_controls);
    left_lyt->addLayout(load_lyt);
    left_lyt->addWidget(lb_inpList);
    left_lyt->addWidget(lw_inpData);
    left_lyt->addWidget(lb_selList);
    left_lyt->addWidget(lw_selData);
    left_lyt->addLayout(bottom_lyt);
    left_lyt->addStretch(1);

    right_lyt->addLayout(intg_lyt);
    right_lyt->addLayout(usplot);

    left_lyt->setContentsMargins( 1, 1, 1, 1);
    left_lyt->setSpacing(1);
    right_lyt->setContentsMargins( 1, 1, 1, 1);
    right_lyt->setSpacing(1);

    body_lyt->addLayout(left_lyt, 1);
    body_lyt->addLayout(right_lyt, 3);
    body_lyt->setContentsMargins( 1, 1, 1, 1);
    body_lyt->setSpacing(0);

    main_lyt->addLayout(runinfo_lyt);
    main_lyt->addLayout(body_lyt);
    main_lyt->setContentsMargins( 0, 0, 0, 0);
    main_lyt->setSpacing(0);
    this->setLayout(main_lyt);

    picker = new US_PlotPicker(plot);
    picker->setRubberBand  ( QwtPicker::VLineRubberBand );
    picker->setMousePattern( QwtEventPattern::MouseSelect1,
                              Qt::LeftButton, Qt::ControlModifier );
    picker->setRubberBandPen(QPen(Qt::red));
    picker->setTrackerPen(QPen(Qt::red));
    plotData();
    picker_state = XNONE;

    connect(pb_load, SIGNAL(clicked()), this, SLOT(slt_loadAUC()));
    connect(pb_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(pb_reset, SIGNAL(clicked()), this, SLOT(slt_reset()));
    connect(pb_save, SIGNAL(clicked()), this, SLOT(slt_save()));
    connect(lw_inpData, SIGNAL(itemDoubleClicked(QListWidgetItem *)),
            this, SLOT(slt_addRmItem(QListWidgetItem *)));
    connect(lw_inpData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_inItemSel(int )));
    connect(lw_selData, SIGNAL(currentRowChanged(int)), this, SLOT(slt_outItemSel(int )));
    connect(pb_rmItem, SIGNAL(clicked()), this, SLOT(slt_rmItem()));
    connect(pb_cleanList, SIGNAL(clicked()), this, SLOT(slt_cleanList()));

    connect(pb_pick_rp, SIGNAL(clicked()), this, SLOT(slt_pickRange()));
    connect(pb_pick_norm, SIGNAL(clicked()), this, SLOT(slt_pickPoint()));
    connect(ckb_xrange, SIGNAL(stateChanged(int)), this, SLOT(slt_xrange(int)));
    connect(ckb_legend, SIGNAL(stateChanged(int)), this, SLOT(slt_legend(int)));
    connect(ckb_integral, SIGNAL(stateChanged(int)), this, SLOT(slt_integral(int)));
    connect(ckb_norm, SIGNAL(stateChanged(int)), this, SLOT(slt_norm(int)));
    connect(ckb_rawData, SIGNAL(stateChanged(int)), this, SLOT(slt_rawData(int)));
    connect(ckb_norm_max, SIGNAL(stateChanged(int)), this, SLOT(slt_norm_by_max(int)));
    connect(picker, SIGNAL(cMouseUp(const QPointF&)),
            this,   SLOT(slt_mouse(const QPointF&)));
    ckb_norm_max->setCheckState(Qt::Checked);
}



void US_Norm_Profile::slt_xrange(int state){
    QString qs = "QPushButton { background-color: %1 }";
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    if (state == Qt::Checked){
        pb_pick_rp->setEnabled(true);
        if (x_min_picked == -1 && x_max_picked == -1) {
            pb_pick_rp->setStyleSheet(qs.arg("yellow"));
        } else {
            pb_pick_rp->setStyleSheet(qs.arg(color.name()));
        }
    }else{
        pb_pick_rp->setDisabled(true);
        pb_pick_rp->setStyleSheet(qs.arg(color.name()));
        enableWidgets(true);
    }
    selectData();
    return;
}

void US_Norm_Profile::slt_addRmItem(QListWidgetItem *item){
    QString text = item->text();
    int id = selFilenames.indexOf(text);
    if (id == -1){
        lw_selData->addItem(text);
        selFilenames.append(text);
        item->setForeground(Qt::red);
    } else {
        QListWidgetItem *selItem = lw_selData->item(id);
        delete selItem;
        selFilenames.removeAt(id);
        item->setForeground(Qt::black);
    }
    selectData();
}

void US_Norm_Profile::slt_rmItem(void){
    int row = lw_selData->currentRow();
    if (row < 0)
        return;
    int rowInp = filenames.indexOf(lw_selData->item(row)->text());
    lw_inpData->item(rowInp)->setForeground(Qt::black);
    selFilenames.removeAt(row);
    lw_selData->takeItem(row);
    selectData();
    if (lw_selData->count() == 0) {
        picker_state = XNONE;
    }
}

void US_Norm_Profile::slt_cleanList(void){
    picker_state = XNONE;
    for (int i = 0; i < lw_selData->count(); i++){
        int rowInp = filenames.indexOf(lw_selData->item(i)->text());
        lw_inpData->item(rowInp)->setForeground(Qt::black);
    }
    selFilenames.clear();
    lw_selData->clear();
    selectData();
}

void US_Norm_Profile::load_data_auto( QMap<QString,QString> & protocol_details )
{
  prot_details = protocol_details;
  slt_reset();
  data_per_channel. clear();
  data_per_channel_xnorm . clear();
  data_per_channel_meniscus. clear();
  data_per_channel_norm_cb. clear();
  data_per_channel_ranges_percents. clear();
  data_per_channel_processed. clear();
  data_per_channel_rmsd. clear();
  channels_ranges = protocol_details[ "channels_to_radial_ranges" ];
  abde_etype = protocol_details["abde_etype"];
  channels_rmsds = protocol_details["rmsds_for_gmp" ];
  slt_loadAUC_auto( protocol_details );
}

//for use in GMP REPORT
void US_Norm_Profile::load_data_auto_report( QMap<QString,QString> & protocol_details )
{
  us_auto_mode_report = true;
  
  //Clean
  slt_reset();
  data_per_channel. clear();
  data_per_channel_xnorm . clear();
  data_per_channel_meniscus. clear();
  data_per_channel_norm_cb. clear();
  data_per_channel_ranges_percents. clear();
  data_per_channel_processed. clear();
  data_per_channel_rmsd. clear();
  
  //First, read autoflowAnalysisABDE record
  QMap<QString, QString> abde_analysis_parms =
    read_autoflowAnalysisABDE_record( protocol_details["autoflowID"] );

  qDebug() << "ABDE analysis PARMS: -- "
	   << abde_analysis_parms["ID"]
	   << abde_analysis_parms["etype"]
	   << abde_analysis_parms[ "xnorms_percents" ]
	   << abde_analysis_parms[ "filename_blc" ];

  //parse respective JSONs
  parse_abde_analysis_jsons( abde_analysis_parms[ "xnorms_percents" ],
			     protocol_details,
			     data_per_channel_xnorm,
			     data_per_channel_norm_cb,
			     data_per_channel_ranges_percents,
			     data_per_channel_rmsd,
			     data_per_channel_meniscus );

  parse_abde_analysis_jsons( abde_analysis_parms[ "filename_blc" ],
			     protocol_details,
			     data_per_channel_xnorm,
			     data_per_channel_norm_cb,
			     data_per_channel_ranges_percents,
			     data_per_channel_rmsd,
			     data_per_channel_meniscus );
  
  //set some fields
  protocol_details["abde_etype"]     = abde_analysis_parms["etype"];
  prot_details = protocol_details;

  channels_ranges = protocol_details[ "channels_to_radial_ranges" ];
  abde_etype = protocol_details["abde_etype"];
  slt_loadAUC_auto_report( protocol_details );

  qDebug() << "[in load_for_REPORT]: us_auto_mode, abde_etype -- "
	   << us_auto_mode << abde_etype;

  emit pass_channels_info( channList );
  emit pass_rmsd_info( data_per_channel_rmsd );
  emit pass_menisc_info( data_per_channel_meniscus );
  emit pass_percents_info( data_per_channel_ranges_percents );
}

//return pointer to data_plot1
QwtPlot* US_Norm_Profile::rp_data_plot()
{
  return plot;
}

//For use in GMP REPORTing 
void US_Norm_Profile::slt_loadAUC_auto_report(QMap<QString,QString> & protocol_details)
{
  channList. clear();
  
  QVector< US_DataIO::RawData > allData;
  QStringList triples;
  QString workingDir;
  US_LoadAUC* dialog = new US_LoadAUC( false, protocol_details, allData, triples, workingDir );

  if (!us_auto_mode)
    if ( dialog->exec() == QDialog::Rejected )  return;
  
  QFileInfo finfo(workingDir);
  QString runid = finfo.baseName();
  QString dirname = finfo.dir().absolutePath();

  qDebug() << "[in slt_loadAUC_auto_report() ], triples -- " << triples; 

  char chtype[ 3 ] = { 'R', 'A', '\0' };
  strncpy( chtype, allData[ 0 ].type, 2 );
  QString dataType = QString( chtype ).left( 2 );
  qDebug() << "Data type -- " << dataType;
  
  for (int i = 0; i < triples.size(); i++)
    {
      QStringList triple_n_list = triples[i].split(u'/');
      channList << triple_n_list[0].trimmed() + triple_n_list[1].trimmed();
      
      QStringList ccw = triples.at(i).split(u'/');
      US_DataIO::RawData rawData = allData.at(i);
      qDebug() << "RawData desc -- " << rawData.description;

      QString fn = tr("%1.%2.%3.%4.%5").arg(runid, dataType, ccw.at(0).trimmed(),
					 ccw.at(1).trimmed(), ccw.at(2).trimmed());
      QString fp = tr("%1.%2.auc").arg(dirname, fn);
      
      filenames << fn;
      filePaths << fp;
      lw_inpData->addItem(fn);
      xvalues << rawData.xvalues;
      yvalues << rawData.scanData.last().rvalues;
    }
  
  //clean chanlist
  channList. removeDuplicates();
  qDebug() << "filenames -- " << filenames;
  qDebug() << "filePaths -- " << filePaths;
  qDebug() << "channList -- " << channList;

  //emit pass_channels_info( channList );
  
  //debug
  for (int i=0; i<channList.size(); ++i )
    {
      //set all channs as "processed"
      data_per_channel_processed[ channList[i] ] = true;
      
      qDebug() << "channel, " << channList[i]
	       << ": data_per_channel_xnorm, data_per_channel_norm_cb, data_per_channel_rmsd, "
	       << " data_per_channel_ranges_percents, data_per_channel_meniscus -- "
	       << data_per_channel_xnorm[channList[i]]
	       << data_per_channel_norm_cb[channList[i]]
	       << data_per_channel_rmsd[channList[i]]
	       << data_per_channel_ranges_percents[channList[i]]
	       << data_per_channel_meniscus[channList[i]];
	;
    }
  
  //Populate channels
  cb_chann -> addItems( channList );
  cb_chann->setCurrentIndex( 0 );
}

void US_Norm_Profile::slt_loadAUC_auto( QMap<QString,QString> & protocol_details)
{
  channList. clear();
  QString dirName;
  if ( abde_etype == "MWL" )
    dirName = protocol_details["ssf_dir_name"];
  else //SWL
    dirName = protocol_details["directory_for_gmp"];
  
  QDir runDir( dirName );
  QStringList fileList = runDir.entryList(QStringList() << "*.auc", QDir::Files, QDir::Name);
  QStringList fPath;
  for (int i=0; i<fileList.size(); ++i )
    {
      fPath << dirName + "/" + fileList[i];

      QStringList file_n_list = fileList[i].split(".");
      channList << file_n_list[2] + file_n_list[3];
    }
  
  if (fPath.size() == 0)
    return;

  qDebug() << "fPath -- " << fPath;
  
  QStringList badFiles;
  for (int i = 0; i < fPath.size(); i++){
    if (filePaths.contains(fPath.at(i)))
      continue;
    US_DataIO::RawData rawData;
    int state = US_DataIO::readRawData(fPath.at(i), rawData);
    QFileInfo finfo = QFileInfo(fPath.at(i));
    if (state != US_DataIO::OK){
      badFiles << finfo.fileName();
      continue;
    }
    
    xvalues << rawData.xvalues;
    QString fname = finfo.fileName();
    fname.chop(4);
    filenames << fname;
    filePaths << fPath.at(i);
    lw_inpData->addItem(fname);
    yvalues << rawData.scanData.last().rvalues;
  }
  if (badFiles.size() != 0){
    QMessageBox::warning(this, "Error!",
			 "These files could not be loaded!\n" +
			 badFiles.join("\n"));
  }

  //clean chanlist
  channList. removeDuplicates();

  //read ePRofiles if non-MWL
  qDebug() << "in ABDE_norm: reading eProfiles BC-- "
	       << protocol_details[ "baseline_corrections" ];
  qDebug() << "in ABDE_norm: reading meniscus info -- "
	       << protocol_details[ "meniscus_info" ];
    
  //set x_norm to "-1" for each channel, and RMSDs
  QStringList channels_rmsds_l = channels_rmsds.split(";");

  for (int i=0; i<channList.size(); ++i )
    {
      data_per_channel_xnorm[channList[i]]   = -1;
      data_per_channel_norm_cb[channList[i]] = Qt::Checked; //2

      //set all channels as unprocessed
      data_per_channel_processed[ channList[i] ] = false;

      //also fill in the RMSD map: MWL-deconv.
      if ( abde_etype == "MWL" )
	{
	  for (int j=0; j<channels_rmsds_l.size(); ++j)
	    {
	      QStringList c_chan_rmsd = channels_rmsds_l[j].split(":");
	      QString c_chan_rmsd_channel =  c_chan_rmsd[0];
	      QString c_chan_rmsd_rmsd    =  c_chan_rmsd[1];
	      if ( c_chan_rmsd_channel == channList[i] )
		data_per_channel_rmsd[channList[i]] = c_chan_rmsd_rmsd.toDouble();
	    }
	}

      //and also, fill meniscus info per channel
      QStringList channels_menisc_l = protocol_details["meniscus_info"].split(",");
      for (int j=0; j<channels_menisc_l.size(); ++j)
	{
	  QStringList c_chan_menisc = channels_menisc_l[j].split(":");
	  QString c_chan_menisc_channel =  c_chan_menisc[0];
	  QString c_chan_menisc_menisc  =  c_chan_menisc[1];
	  if ( c_chan_menisc_channel == channList[i] )
	    data_per_channel_meniscus[channList[i]] = c_chan_menisc_menisc.toDouble();
	}
    }
  // //TEST
  // /* x_norm point for channel  "2A" ,  6.79865
  //    x_norm point for channel  "4A" ,  6.84741
  //    x_norm point for channel  "4B" ,  6.85515
  // */
  // data_per_channel_xnorm["2A"]   = 6.79865;
  // data_per_channel_xnorm["4A"]   = 6.84741;
  // data_per_channel_xnorm["4B"]   = 6.85515;
  // data_per_channel_norm_cb["2A"] = Qt::Unchecked;
  // data_per_channel_norm_cb["4A"] = Qt::Unchecked;
  // data_per_channel_norm_cb["4B"] = Qt::Unchecked;
  // //END TEST
  
  
  //Populate channels
  //channList. removeDuplicates();
  cb_chann -> addItems( channList );
  cb_chann->setCurrentIndex( 0 );
}

QString US_Norm_Profile::select_channel_public( int index )
{
  qDebug() << "[in select_channel_public()]: channame, index, abde_etype, us_auto_mode -- "
	   << cb_chann->itemText( index ) << index
	   << abde_etype <<  us_auto_mode;
  cb_chann->setCurrentIndex( index );

  emit pass_data_per_channel( data_per_channel );
  
  return cb_chann->itemText( index );
}

//[AUTO]go over channels
void US_Norm_Profile::new_chann_auto( int index )
{
  slt_cleanList();
  
  QString channame = cb_chann->itemText( index );
  qDebug() << "Index, ChannName: " << index << "," << channame;

  ckb_norm_max->setChecked( data_per_channel_norm_cb[ channame ] );

  for(int i=0; i<filenames.size(); ++i )
    {
      QStringList file_n_list = filenames[i].split(".");

      qDebug() << "file_n_list -- " << file_n_list;
      
      if ( channame.contains( file_n_list[2] ) &&
	   channame.contains( file_n_list[3] ) )
	{
	  qDebug() << " selecting file " << filenames[i];
	  lw_selData->addItem( filenames[i] );
	  selFilenames.append( filenames[i] );
	  //selectData();
	  selectData_auto();
	}
    }

  //set channels as processed
  data_per_channel_processed[ channame ] = true;
  
  //save btn
  pb_save_auto->setEnabled( areAllProcessed_auto() );
}

bool US_Norm_Profile::areAllProcessed_auto( void )
{
  bool all_processed = true;
  for (int i=0; i<channList.size(); ++i )
    {
      if ( !data_per_channel_processed[ channList[i] ])
	{
	  all_processed = false;
	  break;
	}
    }

  return all_processed;
}

bool US_Norm_Profile::areAllNormalized_auto( QString& msg )
{
  msg. clear();
  bool all_processed = true;
  for (int i=0; i<channList.size(); ++i )
    {
      if ( data_per_channel_xnorm[ channList[i] ] == -1 )
	{
	  msg += "WARNING: Distributions for some channel(s) are not normalized!";
	  msg += "\n\nThis will result in the non-normalized distributions in the GMP Report...";
	  all_processed = false;
	  break;
	}
    }

  return all_processed;
}

//[AUTO] next channel button
void US_Norm_Profile::next_chann_auto( void )
{
  int row = cb_chann->currentIndex() + 1;
  if ( (row + 1 ) <= cb_chann->count() )
    cb_chann->setCurrentIndex( row );
}

//[AUTO] prev channel button
void US_Norm_Profile::prev_chann_auto( void )
{
  int row = cb_chann->currentIndex() - 1;
  if ( row  >= 0 )
    cb_chann->setCurrentIndex( row );
}

void US_Norm_Profile::slt_loadAUC(){

   bool isLocal = ! disk_controls->db();
   if (! isLocal){
      QVector< US_DataIO::RawData > allData;
      QStringList triples;
      QString workingDir;
      US_LoadAUC* dialog = new US_LoadAUC( isLocal, allData, triples, workingDir );

      if ( dialog->exec() == QDialog::Rejected )  return;

      QFileInfo finfo(workingDir);
      QString runid = finfo.baseName();
      QString dirname = finfo.dir().absolutePath();

      for (int i = 0; i < triples.size(); i++){
         QStringList ccw = triples.at(i).split(u'/');
         US_DataIO::RawData rawData = allData.at(i);
         QString fn = tr("%1.%2.%3.%4").arg(runid, ccw.at(0).trimmed(),
                                            ccw.at(1).trimmed(), ccw.at(2).trimmed());
         QString fp = tr("%1.%2.auc").arg(dirname, fn);
         filenames << fn;
         filePaths << fp;
         lw_inpData->addItem(fn);
         xvalues << rawData.xvalues;
         yvalues << rawData.scanData.last().rvalues;
      }
   }else {
      QStringList fPath = QFileDialog::getOpenFileNames(this, tr("Open AUC File"),
                                                    US_Settings::importDir(),
                                                    tr(".auc (*.auc)"));
      if (fPath.size() == 0)
         return;

      QStringList badFiles;
      for (int i = 0; i < fPath.size(); i++){
         if (filePaths.contains(fPath.at(i)))
            continue;
         US_DataIO::RawData rawData;
         int state = US_DataIO::readRawData(fPath.at(i), rawData);
         QFileInfo finfo = QFileInfo(fPath.at(i));
         if (state != US_DataIO::OK){
           badFiles << finfo.fileName();
           continue;
         }

         xvalues << rawData.xvalues;
         QString fname = finfo.fileName();
         fname.chop(4);
         filenames << fname;
         filePaths << fPath.at(i);
         lw_inpData->addItem(fname);
         yvalues << rawData.scanData.last().rvalues;
      }
      if (badFiles.size() != 0){
      QMessageBox::warning(this, "Error!",
                              "These files could not be loaded!\n" +
                                  badFiles.join("\n"));
      }
   }
}

QMap<QString, QVector<double>> US_Norm_Profile::trapz(
                           QVector<double> xval, QVector<double> yval){
    QMap<QString, QVector<double>> out;
    QVector<double> yvalN;
    QVector<double> midxval;
    QVector<double> integral;
    QVector<double> integralN;

    const double *x = xval.data();
    const double *y = yval.data();
    int np = xval.size();
    double dx;
    double sum = 0;
    double maxY = -1e99;
    bool flag_pnorm = false;

    QString channame;
    
    if ( us_auto_mode )
      {
	channame = cb_chann->currentText(); 
	if (! ckb_norm_max->isChecked() && data_per_channel_xnorm[channame] > 0) {
	  flag_pnorm = true;
	}
      }
    else
      {
	if (! ckb_norm_max->isChecked() && x_norm > 0) {
	  flag_pnorm = true;
	}
      }

    for (int i = 1; i < np; i++){
        dx = x[i] - x[i - 1];
        sum += dx * ( y[i] + y[i - 1] ) * 0.5;
        integral << sum;
        midxval << 0.5 * (x[i] + x[i - 1]);
        maxY = qMax(maxY, y[i]);
    }

    double normY = 1;
    if ( us_auto_mode )
      {
	if (flag_pnorm && data_per_channel_xnorm[channame] <= x[0])
	  {
	    normY = y[0];
	  }
	else if (flag_pnorm && data_per_channel_xnorm[channame] >= x[np - 1])
	  {
	    normY = y[np - 1];
	  }
	else if (flag_pnorm && data_per_channel_xnorm[channame] > x[0] && data_per_channel_xnorm[channame] < x[np - 1])
	  {
	    for (int i = 1; i < np - 1; i++)
	      {
		if (x[i] >= data_per_channel_xnorm[channame])
		  {
		    normY = y[i];
		    break;
		  }
	      }
	  }
	else
	  {
	    normY = maxY;
	  }
      }
    else
      {
	if (flag_pnorm && x_norm <= x[0])
	  {
	    normY = y[0];
	  }
	else if (flag_pnorm && x_norm >= x[np - 1])
	  {
	    normY = y[np - 1];
	  }
	else if (flag_pnorm && x_norm > x[0] && x_norm < x[np - 1])
	  {
	    for (int i = 1; i < np - 1; i++)
	      {
		if (x[i] >= x_norm)
		  {
		    normY = y[i];
		    break;
		  }
	      }
	  }
	else
	  {
	    normY = maxY;
	  }
      }

    for (int i = 0; i < integral.size(); i++){
        integralN << integral.at(i) * 100 / sum;
        yvalN << yval.at(i) / normY;
    }
    yvalN << yval.last() / normY;
    out["midxval"] = midxval;
    out["yvaluesN"] = yvalN;
    out["integral"] = integral;
    out["integralN"] = integralN;
    return out;

}


QPair<int, int> US_Norm_Profile::getXlimit(QVector<double> xval_in,
                                           double xmin, double xmax){
    const double *xp = xval_in.data();
    int id1 = -1;
    int id2 = -1;
    int np = xval_in.size();
    for (int i = 0; i < np; i++){
        if (id1 == -1){
            if (xp[i] >= xmin){
                id1 = i;
            }
        } else {
            if (xp[i] >= xmax){
                id2 = i + 1;
                break;
            }
        }
    }
    if (id2 == -1){
        id2 = np;
    }
    QPair<int, int> pair;
    pair.first = id1;
    pair.second = id2 - id1;
    return pair;
}

void US_Norm_Profile::selectData(void){
    xvalues_sel.clear();
    yvalues_sel.clear();
    yvaluesN_sel.clear();
    integral_sel.clear();
    integralN_sel.clear();
    midxval_sel.clear();
    bool flag_limit = false;
    if (ckb_xrange->isChecked() && x_min_picked != -1 && x_max_picked != -1) {
        flag_limit = true;
    }
    QVector<int> inpIds;
    for (int i = 0; i < lw_selData->count(); i++){
        inpIds << filenames.indexOf(lw_selData->item(i)->text());
    }
    for (int i = 0; i < inpIds.size(); i++){
        int id = inpIds.at(i);
        if (flag_limit){
            QPair<int, int> pair= getXlimit(xvalues.at(id), x_min_picked, x_max_picked);
            xvalues_sel << xvalues.at(id).mid(pair.first, pair.second);
            yvalues_sel << yvalues.at(id).mid(pair.first, pair.second);
        } else {
            xvalues_sel << xvalues.at(id);
            yvalues_sel << yvalues.at(id);
        }

        QMap<QString, QVector<double>> trapzOut;
        trapzOut = trapz(xvalues_sel.last(), yvalues_sel.last());
        midxval_sel << trapzOut["midxval"];
        yvaluesN_sel << trapzOut["yvaluesN"];
        integral_sel << trapzOut["integral"];
        integralN_sel << trapzOut["integralN"];
    }
    plotData();
}

void US_Norm_Profile::selectData_auto(void){
    xvalues_sel.clear();
    yvalues_sel.clear();
    yvaluesN_sel.clear();
    integral_sel.clear();
    integralN_sel.clear();
    midxval_sel.clear();
    bool flag_limit = false;
    if (ckb_xrange->isChecked() && x_min_picked != -1 && x_max_picked != -1) {
        flag_limit = true;
    }
    QString channame = cb_chann->currentText();
    //data_per_channel_xnorm[ channame ] = x_norm;
    
    QVector<int> inpIds;
    for (int i = 0; i < lw_selData->count(); i++){
        inpIds << filenames.indexOf(lw_selData->item(i)->text());
	//get channame
	qDebug() << "in selectData_auto(): filename -- "
		 << lw_selData->item(i)->text();
	qDebug() << "current chann -- "
		 << channame;
    }
    for (int i = 0; i < inpIds.size(); i++){
        int id = inpIds.at(i);
	if ( abde_etype == "MWL" )
	  {
	    if (flag_limit){
	      QPair<int, int> pair= getXlimit(xvalues.at(id), x_min_picked, x_max_picked);
	      xvalues_sel << xvalues.at(id).mid(pair.first, pair.second);
	      yvalues_sel << yvalues.at(id).mid(pair.first, pair.second);
	    } else {
	      xvalues_sel << xvalues.at(id);
	      yvalues_sel << yvalues.at(id);
	    }
	  }
	else //SWL
	  {
	    //set x_min_picked, x_max_picked to data ranges in eProfile
	    double bl_slope  = 0;
	    double bl_interp = 0;
	    QString chann_blc = prot_details[ "baseline_corrections" ];
	    QStringList chann_blc_list = chann_blc.split(";");
	    qDebug() << "chann_blc_list -- " << chann_blc_list;
	    for (int bl=0; bl < chann_blc_list.size(); ++bl)
	      {
		QString chanblcinfo = chann_blc_list[bl];
		QString triple_name = chanblcinfo.split(":")[0];
		if ( triple_name.contains( channame ) )
		  {
		    QStringList edit_parms = chanblcinfo.split(":")[1].split(",");
		    x_min_picked = edit_parms[0].toDouble();
		    x_max_picked = edit_parms[1].toDouble();
		    bl_slope     = edit_parms[2].toDouble();
		    bl_interp    = edit_parms[3].toDouble();
		  }
	      }
	    
	    QPair<int, int> pair= getXlimit(xvalues.at(id), x_min_picked, x_max_picked);
	    xvalues_sel << xvalues.at(id).mid(pair.first, pair.second);
	    //yvalues_sel << yvalues.at(id).mid(pair.first, pair.second);
	    
	    //correct for base-lein first
	    QVector<double> yval_corrected;
	    for( int yv=0; yv<yvalues.at(id).size(); ++yv )
	      yval_corrected.push_back( yvalues.at(id)[yv] - (xvalues.at(id)[yv]*bl_slope + bl_interp));
	    
	    yvalues_sel << yval_corrected.mid(pair.first, pair.second);
	  }
	  

        QMap<QString, QVector<double>> trapzOut;
        trapzOut = trapz(xvalues_sel.last(), yvalues_sel.last());
        midxval_sel << trapzOut["midxval"];
        yvaluesN_sel << trapzOut["yvaluesN"];
        integral_sel << trapzOut["integral"];
        integralN_sel << trapzOut["integralN"];
    }
    //Add to qMap < channame, qMap < property, value> >;
    data_per_channel[ channame ]["midxval"]    = midxval_sel;
    data_per_channel[ channame ]["yvaluesN"]   = yvaluesN_sel;
    data_per_channel[ channame ]["integral"]   = integral_sel;
    data_per_channel[ channame ]["integralN"]  = integralN_sel;
    data_per_channel[ channame ]["xvalues"]    = xvalues_sel;
    data_per_channel[ channame ]["yvalues"]    = yvalues_sel;
        
    plotData();
}


void US_Norm_Profile::plotData(void){
    plot->detachItems(QwtPlotItem::Rtti_PlotItem, false);
    plot->enableAxis(QwtPlot::yRight, false);
    plot->enableAxis(QwtPlot::yLeft, false);

    QString channame;
    if (us_auto_mode)
      {
	channame = cb_chann->currentText();
	qDebug() << "in plotData(), channame -- " << channame;
      }
    
    if ( !us_auto_mode )
      {
	grid = us_grid(plot);
	QPen pen_mj = grid->majorPen();
	QPen pen_mn = grid->majorPen();
	pen_mj.setColor(Qt::black);
	pen_mn.setColor(Qt::black);
	grid->setMajorPen(pen_mj);
	grid->setMinorPen(pen_mn);
      }
    
    int tpncy = 255;
    QMap<QString, QColor> color;
    color["blue"]    = QColor(0  , 0  , 255, tpncy);
    color["orange"]  = QColor(255, 128, 0  , tpncy);
    color["green"]   = QColor(0  , 255, 0  , tpncy);
    color["cyan"]    = QColor(0  , 255, 255, tpncy);
    color["red"]     = QColor(255, 0  , 0  , tpncy);
    color["purple"]  = QColor(153, 51 , 255, tpncy);
    color["pink"]    = QColor(255, 0  , 255, tpncy);
    color["yellow"]  = QColor(200, 200, 0 , tpncy);
    color["black"]   = QColor(0  , 0  , 0  , 255);
    QVector<QColor> color_list;
    color_list <<  color["blue"] << color["orange"] << color["green"];
    color_list << color["cyan"] << color["red"] << color["purple"] ;
    color_list << color["pink"] << color["yellow"] << color["black"];
    int sz_clist = color_list.size();
    QPen pen = QPen(Qt::SolidPattern, 1, Qt::SolidLine, Qt::RoundCap, Qt::BevelJoin);
    pen.setWidth(2);

    QVector<Qt::PenStyle> penstyles_list;
    penstyles_list << Qt::DashLine << Qt::DotLine << Qt::DashDotLine << Qt::DashDotDotLine;

    //symbols
    QVector<QwtSymbol*> line_symbols;
    QwtSymbol *s_cross      = new QwtSymbol(QwtSymbol::Cross, QBrush(Qt::black), QPen(Qt::black), QSize(4, 4));
    QwtSymbol *s_xcross     = new QwtSymbol(QwtSymbol::XCross, QBrush(Qt::black), QPen(Qt::black), QSize(4, 4));
    QwtSymbol *s_hline      = new QwtSymbol(QwtSymbol::HLine, QBrush(Qt::black), QPen(Qt::black), QSize(4, 4));
    QwtSymbol *s_utriangle  = new QwtSymbol(QwtSymbol::UTriangle, QBrush(Qt::black), QPen(Qt::black), QSize(4, 4));
    QwtSymbol *s_dtriangle  = new QwtSymbol(QwtSymbol::DTriangle, QBrush(Qt::black), QPen(Qt::black), QSize(4, 4));
    line_symbols <<  s_cross << s_xcross << s_hline << s_utriangle << s_dtriangle;
    

    QwtText yTitle = plot->axisTitle(QwtPlot::yLeft);

    int nd = selFilenames.size();
    qDebug() << "[in plotData(): ] selFilenames -- " << selFilenames;
        
    bool plt_state = ckb_rawData->isChecked() || ckb_integral->isChecked();

    if (! plt_state || nd == 0){
        yTitle.setText("");
        plot->setAxisTitle(QwtPlot::yLeft, yTitle);
        plot->setAxisTitle(QwtPlot::yRight, yTitle);
        plot->replot();
        return;
    }

    const double *xp, *yp;
    QVector<double> xp_intN_protein, yp_intN_protein;

    double minX =  1e99;
    double maxX = -1e99;
    double minY =  1e99;
    double maxY = -1e99;

    double minY_global = minY;
    double maxY_global = maxY;

    if (ckb_rawData->isChecked()){
        bool norm = ckb_norm->isChecked();
        if (norm)
            yTitle.setText("Absorbance (normalized)");
        else
            yTitle.setText("Absorbance");

	if ( us_auto_mode )
	  {
	    plot->setAxisTitle(QwtPlot::yRight, yTitle);
	    plot->enableAxis(QwtPlot::yRight, true);
	  }
	else
	  {
	    plot->setAxisTitle(QwtPlot::yLeft, yTitle);
	    plot->enableAxis(QwtPlot::yLeft, true);
	  }
	
        for (int i = 0; i < nd; i++){
            int np = xvalues_sel.at(i).size();
            xp = xvalues_sel.at(i).data();
            pen.setColor(color_list.at(i % sz_clist));
	    
            //QString legend = tr("(D)_") + selFilenames.at(i);
	    QString legend;
	    if ( us_auto_mode )
	      {
		if (abde_etype == "MWL")
		  {
		    if ( selFilenames.at(i). contains("_002") ||
			 selFilenames.at(i). contains(".002") ||
			 selFilenames.at(i). endsWith(".2")   ||
			 selFilenames.at(i). contains("protein", Qt::CaseInsensitive) ||
			 selFilenames.at(i). contains("prot", Qt::CaseInsensitive) )
		      {
			legend = tr("(D)_") + channame + "-protein";
			//pen.setColor("red");
			pen.setColor("magenta");
		      }
		    else if ( selFilenames.at(i). contains("_001") ||
			      selFilenames.at(i). contains(".001") ||
			      selFilenames.at(i). endsWith(".1")   ||
			      selFilenames.at(i). contains("dna", Qt::CaseInsensitive) )
		      {
			legend = tr("(D)_") + channame + "-DNA";
			//pen.setColor("blue");
			pen.setColor("cyan");
		      }
		    else
		      legend = channame;
		  }
		else // SWL
		  {
		    legend = tr("(D)_") + channame;
		    pen.setColor("cyan");
		  }
	      }
	    else
	      legend = tr("(D)_") + selFilenames.at(i);

	    if (norm)
	      {
		if ( us_auto_mode )
		  yp = data_per_channel[ channame ]["yvaluesN"].at(i).data();
		else
		  yp = yvaluesN_sel.at(i).data();
	      }
	    else
	      yp = yvalues_sel.at(i).data();
            QwtPlotCurve* curve = us_curve(plot, legend);

	    if ( us_auto_mode )
	      {
		curve->setYAxis(QwtPlot::yRight);
		// if ( ckb_integral->isChecked() )
		//   curve->setItemAttribute( QwtPlotItem::Legend, false );
	      }
	    
            curve->setPen(pen);
            curve->setSamples(xp, yp, np);

            for (int j = 0; j < np; j++){
                minX = qMin(minX, xp[j]);
                maxX = qMax(maxX, xp[j]);
                minY = qMin(minY, yp[j]);
                maxY = qMax(maxY, yp[j]);
            }
        }
        double dy = (maxY - minY) * 0.05;
	
	if ( us_auto_mode )
	  plot->setAxisScale( QwtPlot::yRight, minY - dy, maxY + dy);
	else
	  plot->setAxisScale( QwtPlot::yLeft, minY - dy, maxY + dy);

	minY_global = minY;
	maxY_global = maxY;

	qDebug() << "[plotting rawData: minY_global, maxY_global ] -- "
		 << minY_global << maxY_global;
    }

    minY =  1e99;
    maxY = -1e99;

    if (ckb_integral->isChecked())
      {
	bool norm = ckb_norm->isChecked();
        if (norm)
	  yTitle.setText("Integral (%)");
        else
	  yTitle.setText("Integral");
	
	if ( us_auto_mode )
	  {
	    plot->setAxisTitle(QwtPlot::yLeft, yTitle);
	    plot->enableAxis(QwtPlot::yLeft, true);
	  }
	else
	  {
	    if (ckb_rawData->isChecked()){
	      plot->setAxisTitle(QwtPlot::yRight, yTitle);
	      plot->enableAxis(QwtPlot::yRight, true);
	    }
	    else {
	      plot->setAxisTitle(QwtPlot::yLeft, yTitle);
	      plot->enableAxis(QwtPlot::yLeft, true);
	    }
	  }
	
        for (int i = 0; i < nd; i++){
	  
	    int np;
	    if ( us_auto_mode )
	      {
		np = data_per_channel[ channame ]["midxval"].at(i).size();
		xp = data_per_channel[ channame ]["midxval"].at(i).data();
	      }
	    else
	      {
		np = midxval_sel.at(i).size();
		xp = midxval_sel.at(i).data();
	      }
	    
	    pen.setColor(color_list.at(i % sz_clist));

	    QString legend;

	    if ( us_auto_mode )
	      {
		if (abde_etype == "MWL")
		  {
		    if ( selFilenames.at(i). contains("_002") ||
			 selFilenames.at(i). contains(".002") ||
			 selFilenames.at(i). endsWith(".2")   ||
			 selFilenames.at(i). contains("protein", Qt::CaseInsensitive) ||
			 selFilenames.at(i). contains("prot", Qt::CaseInsensitive))
		      {
			legend = tr("(I)_") + channame + "-protein";
			pen.setColor("red");
						
			xp_intN_protein = data_per_channel[ channame ]["midxval"][i];
			yp_intN_protein = data_per_channel[ channame ]["integralN"][i];
			//qDebug() << "yp_intN_protein, size() " << yp_intN_protein << ", size: " << yp_intN_protein.size();
		      }
		    else if ( selFilenames.at(i). contains("_001") ||
			      selFilenames.at(i). contains(".001") ||
			      selFilenames.at(i). endsWith(".1")   ||
			      selFilenames.at(i). contains("dna", Qt::CaseInsensitive) )
		      {
			legend = tr("(I)_") + channame + "-DNA";
			pen.setColor("blue");
		      }
		    else
		      legend = channame;
		  }
		else //SWL or other
		  {
		    xp_intN_protein = data_per_channel[ channame ]["midxval"][i];
		    yp_intN_protein = data_per_channel[ channame ]["integralN"][i];
		    legend = tr("(I)_") + channame;
		    pen.setColor("darkBlue");
		  }
	      }
	    else
	      legend = tr("(I)_") + selFilenames.at(i);
	    
            if (ckb_norm->isChecked())
	      {
		if ( us_auto_mode )
		  yp = data_per_channel[ channame ]["integralN"].at(i).data();
		else
		  yp = integralN_sel.at(i).data();
	      }
            else
	      {
		if ( us_auto_mode )
		  yp = data_per_channel[ channame ]["integral"].at(i).data();
		else
		  yp = integral_sel.at(i).data();
	      }
            QwtPlotCurve* curve = us_curve(plot, legend);

	    if ( us_auto_mode )
	      curve->setYAxis(QwtPlot::yLeft);
	    else
	      {
		if (ckb_rawData->isChecked())
		  curve->setYAxis(QwtPlot::yRight);
	      }
	    
	    curve->setPen(pen);
            curve->setSamples(xp, yp, np);

            for (int j = 0; j < np; j++){
                minX = qMin(minX, xp[j]);
                maxX = qMax(maxX, xp[j]);
                minY = qMin(minY, yp[j]);
                maxY = qMax(maxY, yp[j]);
            }
        }

        double dy = (maxY - minY) * 0.05;

	if ( us_auto_mode ) 
	  {
	    plot->setAxisScale( QwtPlot::yLeft, minY - dy, maxY + dy);
	  }
	else
	  {
	    if (ckb_rawData->isChecked()){
	      plot->setAxisScale( QwtPlot::yRight, minY - dy, maxY + dy);
	    }
	    else {
	      plot->setAxisScale( QwtPlot::yLeft, minY - dy, maxY + dy);
	    }
	  }
	
	minY_global = minY;
	maxY_global = maxY;

	qDebug() << "[plotting Integral: minY_global, maxY_global ] -- "
		 << minY_global << maxY_global;
      } //end of in ckb_integral is Checked!!!
    
    //if [AUTO-ABDE] mode, add ranges
    if ( us_auto_mode && ckb_ranges->isChecked() )
      {
	QStringList c_ranges;
	QString channame = cb_chann->currentText();
	//e.g. "2A:6.2-6.5;2B:5.8-7;4A:6.1-6.5;4B:6.1-6.5"
	QString chann_ranges = channels_ranges;
	QStringList chann_ranges_l = chann_ranges.split(";");
	for (int i=0; i<chann_ranges_l.size(); ++i)
	  {
	    QString chann_ranges_c = chann_ranges_l[i];
	    QStringList chann_ranges_c_l = chann_ranges_c.split(":");
	    if ( chann_ranges_c_l[0] == channame )
	      {
		qDebug() << "channel " << channame << ", has range(s): "
			 << chann_ranges_c_l[1];
		qDebug() << "List -- " << chann_ranges_c_l[1].split(",");
		c_ranges = chann_ranges_c_l[1].split(",");
		break;
	      }
	  }
	
	for (int i=0; i<c_ranges.size(); ++i )
	  {
	    QString point1_s = c_ranges[ i ].split("-")[0];
	    QString point2_s = c_ranges[ i ].split("-")[1];
	    double point1 = point1_s.toDouble();
	    double point2 = point2_s.toDouble();
	    
	    // find intercection of point1 & 2 with the 'yp_intN_protein'
	    // use xp_intN_protein as index
	    // yp_intN_protein = data_per_channel[ channame ]["integralN"].at(i).data();
	    if ( !us_auto_mode_report )
	      find_percent_from_range( channame, point1_s, point2_s, xp_intN_protein, yp_intN_protein );
	    
	    QwtPlotCurve* v_line_peak1;
	    double r1[ 2 ];
	    r1[ 0 ] = point1;
	    r1[ 1 ] = point1;
	    
	    QwtPlotCurve* v_line_peak2;
	    double r2[ 2 ];
	    r2[ 0 ] = point2;
	    r2[ 1 ] = point2;
	    
	   	   
#if QT_VERSION < 0x050000
	    QwtScaleDiv* y_axis = plot->axisScaleDiv( QwtPlot::yLeft );
#else
	    QwtScaleDiv* y_axis = (QwtScaleDiv*)&(plot->axisScaleDiv( QwtPlot::yLeft ));
#endif
	    
	    double padding = ( y_axis->upperBound() - y_axis->lowerBound() ) / 30.0;

	    qDebug() << "In Plotting ranges: y_axis->upperBound(), y_axis->lowerBound(), padding --- "
		     << y_axis->upperBound() << y_axis->lowerBound() << padding;
	    qDebug() << "In Plotting ranges: maxY, minY ---  "
		     << maxY << minY;
	    qDebug() << "In Plotting ranges: maxY_global, minY_global ---  "
		     << maxY_global << minY_global;
	    
	    double v[ 2 ];
	    // v [ 0 ] = y_axis->upperBound() - padding;
	    // v [ 1 ] = y_axis->lowerBound();// + padding;

	    double dy = (maxY_global - minY_global) * 0.05;

	    if ( ckb_integral->isChecked() )
	      {
		v [ 0 ] = maxY_global;// + dy;
		v [ 1 ] = minY_global;// - dy;
	      }
	    else
	      {
		v [ 0 ] = y_axis->upperBound() - padding;
		v [ 1 ] = y_axis->lowerBound() + padding;
	      }

	    //[TEST]get new samples
	    QVector<double> x_vals_1(60, point1);
	    QVector<double> x_vals_2(60, point2);
	    QVector<double> y_vals_1, y_vals_2;
	    double delta_v = (v [ 0 ] - v [ 1 ]) / double(x_vals_1.size());
	    for ( int x1=0; x1<x_vals_1.size(); ++x1)
	      {
		double c_val = v [ 1 ] + x1* delta_v;
		y_vals_1.push_back( c_val );
		y_vals_2.push_back( c_val );
	      }
	    
	    
	    QString vline_name1 = "Low "  + QString::number( i );
	    QString vline_name2 = "High " + QString::number( i );
	    
	    // int color_index = i;
	    // while ( color_index >= sz_clist )
	    //   color_index -= sz_clist;
	    
	    // QPen pen1 = QPen( QBrush( color_list[ color_index ] ), 2.0, Qt::DotLine );
	    // pen1.setColor("black");
	    // int style_num = 0;
	    // if ( i < penstyles_list.size() )
	    //   style_num = i;
	    // pen1.setStyle(penstyles_list[ style_num ]);
	    QPen pen1;
	    pen1.setColor(Qt::black);
	    pen1.setWidth(1);
	    pen1.setStyle(Qt::NoPen);
	    //pen1.setStyle(Qt::SolidLine);

	    int symbol_number = 0;
	    if ( i < line_symbols.size() )
	      symbol_number = i;
	    
	    v_line_peak1 = us_curve( plot, vline_name1 );
	    //v_line_peak1 ->setSamples( r1, v, 2 );
	    v_line_peak1 ->setSamples(x_vals_1, y_vals_1);
	    
	    v_line_peak2 = us_curve( plot, vline_name2 );
	    //v_line_peak2 ->setSamples( r2, v, 2 );
	    v_line_peak2 ->setSamples(x_vals_2, y_vals_2);
	    
	    v_line_peak1->setPen( pen1 );
	    v_line_peak2->setPen( pen1 );
	    v_line_peak1->setRenderHint(QwtPlotCurve::RenderAntialiased, true);
	    v_line_peak2->setRenderHint(QwtPlotCurve::RenderAntialiased, true);
	    v_line_peak1->setSymbol( line_symbols[ symbol_number ]);
	    v_line_peak2->setSymbol( line_symbols[ symbol_number ]);
	    
	    v_line_peak1->setItemAttribute( QwtPlotItem::Legend, false );
	    v_line_peak2->setItemAttribute( QwtPlotItem::Legend, false );

	    qDebug() << "Finish plotting vertical ranges";
	  }
	//plot->replot();
      }

    if (ckb_legend->isChecked())
      {
	QwtLegend* legend = new QwtLegend();
	legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
	plot->insertLegend( legend, QwtPlot::BottomLegend   );
      }
    else
      {
        plot->insertLegend( NULL, QwtPlot::BottomLegend );
      }
    
    double dx = (maxX - minX) * 0.05;
    plot->setAxisScale( QwtPlot::xBottom, minX - dx, maxX + dx);
    plot->replot();
}

void US_Norm_Profile::find_percent_from_range( QString channame, QString point1_s, QString point2_s,
					       QVector< double> xp_intN_protein, QVector<double> yp_intN_protein )
{
  QString range = point1_s + "-" + point2_s;
  double p1 = point1_s.toDouble();
  double p2 = point2_s.toDouble();
  int p1_ind=0, p2_ind=0;

  qDebug() << "p1, p2 -- " << p1 << p2;
  //qDebug() << "[in find_percent_from_range() ] xp_intN_protein, size() " << xp_intN_protein << ", size: " << xp_intN_protein.size();
  for (int i=0; i<xp_intN_protein.size(); ++i )
    {
      if ( xp_intN_protein[i] <= p1 )
	p1_ind = i;
    }
  
  for (int i=0; i<xp_intN_protein.size(); ++i )
    {
      if ( xp_intN_protein[i] <= p2 )
	p2_ind = i;
    }
  qDebug() << "p1_ind " << p1_ind << ", y-value: " << yp_intN_protein[p1_ind];
  qDebug() << "p2_ind " << p2_ind << ", y-value: " << yp_intN_protein[p2_ind];
  
  data_per_channel_ranges_percents[ channame ][ range ] = double(yp_intN_protein[p2_ind] - yp_intN_protein[p1_ind]);
  qDebug() << "channel " << channame << ", range " << range << ", percent: "
    	   <<  double(yp_intN_protein[p2_ind] - yp_intN_protein[p1_ind]);
}

void US_Norm_Profile::slt_legend(int state) {

    if (state == Qt::Checked) {
        QwtLegend* legend = new QwtLegend();
        legend->setFrameStyle( QFrame::Box | QFrame::Sunken );
        plot->insertLegend( legend, QwtPlot::BottomLegend   );
    } else {
        plot->insertLegend( NULL, QwtPlot::BottomLegend );
    }
    plot->replot();
}

void US_Norm_Profile::slt_ranges(int) {
    plotData();
}

void US_Norm_Profile::slt_rawData(int) {
    plotData();
}

void US_Norm_Profile::slt_integral(int) {
    plotData();
}

void US_Norm_Profile::slt_norm(int) {
  qDebug() << "ckb_ranges() checked ? " << ckb_ranges->isChecked();
    plotData();
}

void US_Norm_Profile::slt_pickRange(){
    x_min_picked = -1;
    x_max_picked = -1;
    if (selFilenames.size() == 0) return;
    if (picker_state == XNORM) return;

    picker_state = XRANGE;
    pb_pick_rp->setStyleSheet("QPushButton { background-color: red }");
    selectData();
    enableWidgets(false);
}

void US_Norm_Profile::slt_pickPoint() {
    if (selFilenames.size() == 0)  return;
    if (picker_state == XRANGE) return;
    pb_pick_norm->setStyleSheet("QPushButton { background-color: red }");
    if( us_auto_mode ) 
      selectData_auto();
    else
      selectData();
    enableWidgets(false);
    picker_state = XNORM;
}

// void US_Norm_Profile::slt_pickPoint_auto() {
//   qDebug() << "in slt_pickPoint_auto(): selFilenames -- "
// 	   << selFilenames;
//     if (selFilenames.size() == 0)  return;
//     // if (picker_state == XRANGE) return;
//     pb_pick_norm->setStyleSheet("QPushButton { background-color: red }");
//     selectData();
//     enableWidgets(false);
//     picker_state = XNORM;
// }


void US_Norm_Profile::slt_mouse(const QPointF& point){
    if (selFilenames.size() == 0) return;
    if (picker_state == XNONE) return;
    QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
    QString bkgc = tr("QPushButton { background-color: %1 }").arg(color.name());

    if (picker_state == XRANGE) {
        double x = point.x();
        if (x_min_picked == -1){
            x_min_picked = x;
            double miny = plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
            double maxy = plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
            QVector<double> xx;
            QVector<double> yy;
            int np = 50;
            double dyy = (maxy - miny) / np;
            double y0 = miny;
            for (int i = 0; i < np; ++i){
                xx << x_min_picked;
                yy << y0 + i * dyy;
            }
            QPen pen(Qt::red);
            pen.setWidth(3);
            QwtPlotCurve* curve = us_curve( plot,"");
            curve->setStyle(QwtPlotCurve::Dots);
            curve->setPen(pen);
            curve->setSamples(xx.data(), yy.data(), np);
            //        grid = us_grid(plot);
            plot->replot();
        } else {
            if (x <= x_min_picked){
                QString mess("Pick a radial point greater than: %1 cm");
                QMessageBox::warning( this, tr( "Warning" ), mess.arg(x_min_picked));
                return;
            }
            x_max_picked = x;
            pb_pick_rp->setStyleSheet(bkgc);
            selectData();
            enableWidgets(true);
            picker_state = XNONE;
        }
    } else if (picker_state == XNORM) {
        x_norm = point.x();
        enableWidgets(true);
        pb_pick_norm->setStyleSheet(bkgc);
        picker_state = XNONE;
	if ( us_auto_mode )
	  {
	    QString channame = cb_chann->currentText();
	    data_per_channel_xnorm[ channame ] = point.x();
	    selectData_auto();
	  }
	else
	  selectData();
    }
}

void US_Norm_Profile::slt_reset(){
    slt_cleanList();
    lw_inpData->clear();
    filenames.clear();
    filePaths.clear();
    xvalues.clear();
    yvalues.clear();
    x_min_picked = -1;
    x_max_picked = -1;
    ckb_xrange->setCheckState(Qt::Unchecked);

    if ( us_auto_mode )
      {
	qDebug() << "[AUTO-mode] -- resetting!";
	
	channList. clear();
	cb_chann->clear();
	//cb_chann->disconnect();
	channels_ranges.clear();
	channels_rmsds .clear();
      }

}

void US_Norm_Profile::enableWidgets(bool state){
    pb_load->setEnabled(state);
    pb_reset->setEnabled(state);
    pb_save->setEnabled(state);
    pb_rmItem->setEnabled(state);
    pb_cleanList->setEnabled(state);
    lw_inpData->setEnabled(state);
    lw_selData->setEnabled(state);
    ckb_rawData->setEnabled(state);
    ckb_integral->setEnabled(state);
    ckb_legend->setEnabled(state);
    ckb_norm->setEnabled(state);
}

void US_Norm_Profile::slt_save(){
    int nd = selFilenames.size();
    if (nd == 0)
        return;

    QString fname = QFileDialog::getSaveFileName(this, tr("Save File"),
                               US_Settings::reportDir(),
                               tr("(*.csv)"));
    if (fname.size() == 0)
        return;

    QFileInfo inFInfo = QFileInfo(fname);
    QFileInfo outFInfo;
    if (inFInfo.suffix() == "csv")
        outFInfo = QFileInfo(inFInfo);
    else{
        fname = inFInfo.completeBaseName();
        fname.append(".csv");
        outFInfo = QFileInfo(inFInfo.dir(), fname);
    }

    qDebug() << outFInfo.absoluteFilePath();

    QFile file{outFInfo.absoluteFilePath()};
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream outStream{&file};
        QVector<int> nPoints;
        for (int i = 0; i < nd; i++){
            nPoints << xvalues_sel.at(i).size();
            outStream << tr("Filename,X_scan,Scan,Scan_norm,");
            outStream << tr("X_integral,Integral,Integral_norm");
            if (i != nd - 1)
                outStream << tr(",");
            else
                outStream << "\n";
        }

        bool newLine = true;
        int line = 0;
        while (newLine){
            newLine = false;
            for (int i = 0; i < nd; i++){
//                outStream << tr("Filename,X_scan,X_integral,");
                int np = nPoints.at(i);

                if (line == 0) {           //Filename
                    outStream << selFilenames.at(i) << ",";
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //X_scan
                    outStream << QString::number(xvalues_sel.at(i).at(line), 'f', 4) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //Scan
                    outStream << QString::number(yvalues_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np){        //Scan_norm
                    outStream << QString::number(yvaluesN_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np - 1){    //X_integral
                    outStream << QString::number(midxval_sel.at(i).at(line), 'f', 4) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }

                if (line < np - 1){        //Integral
                    outStream << QString::number(integral_sel.at(i).at(line), 'f', 6) << ",";
                    newLine = true;
                } else {
                    outStream <<  " ,";
                }
                if (line < np - 1){        //Integral_norm
                    outStream << QString::number(integralN_sel.at(i).at(line), 'f', 6);
                    newLine = true;
                }

                if (i != nd - 1)
                    outStream << tr(",");
                else
                    outStream << "\n";
            }
            line ++;
        }
    }
    qDebug() << "Saved the csv file!";
    file.close();
}

void US_Norm_Profile::closeEvent(QCloseEvent *event)
{
    emit widgetClosed();
    event->accept();
}

void US_Norm_Profile::slt_inItemSel(int row)
{
    if (row < 0){
        return;
    } else {
        le_runinfo->setText(lw_inpData->item(row)->text());
    }
}

void US_Norm_Profile::slt_outItemSel(int row)
{
    if (row < 0){
        return;
    } else {
        le_runinfo->setText(lw_selData->item(row)->text());
    }
}

void US_Norm_Profile::slt_norm_by_max(int state)
{
  if (us_auto_mode)
    {
      QString channame = cb_chann->currentText();
      data_per_channel_norm_cb[ channame ] = state;
    }
  
  QString qs = "QPushButton { background-color: %1 }";
  QColor color = US_GuiSettings::pushbColor().color(QPalette::Active, QPalette::Button);
  if (state == Qt::Checked) {
    pb_pick_norm->setStyleSheet(qs.arg(color.name()));
    pb_pick_norm->setDisabled(true);
  } else {
    pb_pick_norm->setDisabled(false);
    if (x_norm == -1) {
      pb_pick_norm->setStyleSheet(qs.arg("yellow"));
    } else {
      pb_pick_norm->setStyleSheet(qs.arg(color.name()));
    }
  }
  if ( us_auto_mode ) 
    selectData_auto();
  else
    selectData();
}

void US_Norm_Profile::save_auto( void )
{
  QString msg_u;
  if (!areAllNormalized_auto( msg_u ) && abde_etype == "MWL" )
    {
      qDebug() << "NOT all channels NORMALIZED!!!";
      int status = QMessageBox::warning( this,
					 tr( "Channel Normalization" ),
					 msg_u,
					 QMessageBox::Ok, QMessageBox::Cancel );

      if ( status != 0 ) return;
    }
  //construct JSON to be saved && passed
  /***
      {
        "2A":{
               "x_norm":"6.79841",
	       "percents": {
	                      "6.2-6.5":"48.6361",
			      "6.6-6.9":"39.3836"
			   }
	    },
	"4A":{
	       "x_norm":"6.8476",
	       "percents":{
	                      "6.1-6.5":"68.876",
			      "6.6-6.94":"27.369"
			   }
	      },
	 "4B":{
	        "x_norm":"6.86425",
		"percents":{
		               "6.25-6.55":"89.7197",
			       "6.65-7":"6.36435"
			    }
	      }
       }
      
  ***/
  
  QString json_p = "{";
  for ( int i=0; i< channList.size(); ++i )
    {
      QString channame = channList[ i ];
      QString x_normal = QString::number(data_per_channel_xnorm[ channame ]);
      qDebug() << "x_norm point for channel "
	       <<  channame << ", "
	       <<  x_normal;
      //x_normalization
      json_p += "\"" + channame + "\":{\"x_norm\":\"" + x_normal + "\",";

      //meniscus
      json_p += "\"meniscus\":\"" + QString::number(data_per_channel_meniscus[channame]) + "\",";

      //add rmsds
      if ( abde_etype=="MWL" )
	json_p += "\"rmsd\":\"" + QString::number(data_per_channel_rmsd[channame]) + "\",";
      
      //now over ranges:percents
      QMap < QString, double> ranges_percents = data_per_channel_ranges_percents[ channame ];
      json_p += "\"percents\":{";
      QMap < QString, double >::iterator rp;
      for ( rp = ranges_percents.begin(); rp != ranges_percents.end(); ++rp )
	{
	  json_p += "\"" + rp.key() + "\":\"" + QString::number(rp.value()) + "\",";
	}
      json_p.chop(1);
      json_p += "}},";
    }
  json_p.chop(1);
  json_p += "}";

  qDebug() << "JSON: " << json_p;

  // // // TEST 
  // return;

  //Determine filename (MWL or SWL)
  QString filename_p;
  if ( abde_etype == "MWL" )
    filename_p = prot_details["ssf_dir_name"];
  else //SWL
    filename_p = prot_details["directory_for_gmp"];

  QStringList filename_p_list = filename_p.split("/");
  if ( !filename_p_list.isEmpty() )
    filename_p = filename_p_list.takeLast();
  qDebug() << "Passed filename_p -- " << filename_p; 

  //JSON filename_bcl (_blc for SWL only...)
  QString filename_blc_p = "{";
  filename_blc_p += "\"filename\":\"" + filename_p + "\"";
  if ( abde_etype == "SWL" )
    {
      //For SWL, pass baseline_corrections
      QString bl_corrs = prot_details[ "baseline_corrections" ];
      filename_blc_p += ",\"blcorrs\":\"" + bl_corrs + "\"";
    }
  filename_blc_p += "}";
  qDebug() << "filename_blc_p JSON -- " << filename_blc_p;
  

  // Determine if we are using the database
   US_DB2* dbP  = NULL;

   if ( disk_controls->db() )
   {
      US_Passwd pw;
      dbP            = new US_DB2( pw.getPasswd() );

      if ( dbP->lastErrno() != US_DB2::OK )
      {
         QMessageBox::warning( this, tr( "Connection Problem" ),
           tr( "Could not connect to database: \n" ) + dbP->lastError() );
         return;
      }
   }

   /*************************************************************************************************************/
   // We need to  insert submission form dialog (with password...)
   /*************************************************************************************************************/
   QStringList qry1;
   qry1 <<  QString( "get_user_info" );
   dbP-> query( qry1 );
   dbP-> next();
   int u_ID        = dbP-> value( 0 ).toInt();
   QString u_fname = dbP-> value( 1 ).toString();
   QString u_lname = dbP-> value( 2 ).toString();
   int u_lev       = dbP-> value( 5 ).toInt();
   
   QString user_submitter = u_lname + ", " + u_fname;
   
   US_Passwd   pw_at;
   QMap < QString, QString > gmp_submitter_map  =
     pw_at.getPasswd_auditTrail( "GMP Run ABDE Form", "Please fill out GMP run ABDE-Analysis form:", user_submitter );
   
   int gmp_submitter_map_size = gmp_submitter_map.keys().size();
   qDebug() << "Submitter map: "
	    << gmp_submitter_map.keys()  << gmp_submitter_map.keys().size() << gmp_submitter_map_size
	    << gmp_submitter_map.keys().isEmpty() 
	    << gmp_submitter_map[ "User:" ]
	    << gmp_submitter_map[ "Comment:" ];
   //<< gmp_submitter_map[ "Master Password:" ];
   
   if ( gmp_submitter_map_size == 0 ||  gmp_submitter_map.keys().isEmpty() )
     {
       //revert_autoflowAnalysisABDEstages_record( prot_details["autoflowID"] );
       return;
     }
   else
     {
       //check if saving already initiated /////////////////////////////////////////////////
       int status_analysis_abde_unique;
       status_analysis_abde_unique = read_autoflowAnalyisABDEstages_record( prot_details["autoflowID"] );
       
       qDebug() << "status_analysis_abde_unique -- " << status_analysis_abde_unique;
       
       if ( !status_analysis_abde_unique )
         {
       
           QMessageBox::information( this,
       				tr( "The Program State Updated / Being Updated" ),
       				tr( "The program advanced or is advancing to the next stage!\n\n"
       				    "This happened because you or different user "
       				    "has already saved ABDE analysis profiles into DB using different program "
       				    "session, and the program is proceeding to the next stage. \n\n"
       				    "The program will return to the autoflow runs dialog where "
       				    "you can re-attach to the actual current stage of the run. "
       				    "Please allow some time for the status to be updated.") );
       
       
           //slt_reset(); //TEMP
           emit back_to_runManager();
           return;
         }
     }
   /*************************************************************************************************************/

   //NOW, save ABDE profiles
   qry1.clear();
   qry1 << "update_autoflowAnalysisABDE_record"
	<< json_p
        << filename_blc_p
	<< prot_details[ "autoflowID" ];

   int status = dbP->statusQuery( qry1 );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
    {
      QMessageBox::warning( this,
			    tr( "AutoflowAnalysisABDE Record Not Updated" ),
			    tr( "No AutoflowAnalysisABDE record\n"
				"associated with this experiment." ) );
      return;
    }

   //Also, update autoflowStatus's 'analysisABDE' && 'analysisABDEts' (new fields) with info from gmp_submitter_map:
   record_AnalysisABDE_status( gmp_submitter_map );
 
   /***  FOR TEST */
   //Now, update parent autoflow record with 'REPORT' stage
   update_autoflow_record_atAnalysisABDE();  //TEST (will turn ON!)
   
   //Finally, switch to 6. REPORT -- need to communicate with parent autoflow_Analysis....
   emit abde_to_report( prot_details );
   /***/
}

void US_Norm_Profile::record_AnalysisABDE_status( QMap<QString,QString> gmp_form )
{
  // Check DB connection
  US_Passwd pw;
  QString masterpw = pw.getPasswd();
  US_DB2* db = new US_DB2( masterpw );
  
  if ( db->lastErrno() != US_DB2::OK )
    {
      QMessageBox::warning( this, tr( "Connection Problem" ),
			    tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
      return;
    }
  
  QStringList qry;

  //get user info
  qry.clear();
  qry <<  QString( "get_user_info" );
  db->query( qry );
  db->next();

  int ID        = db->value( 0 ).toInt();
  QString fname = db->value( 1 ).toString();
  QString lname = db->value( 2 ).toString();
  QString email = db->value( 4 ).toString();
  int     level = db->value( 5 ).toInt();
  
  QString AnalysisABDE_Json;
  AnalysisABDE_Json. clear();
  AnalysisABDE_Json += "{ \"Person\": ";

  AnalysisABDE_Json += "[{";
  AnalysisABDE_Json += "\"ID\":\""     + QString::number( ID )     + "\",";
  AnalysisABDE_Json += "\"fname\":\""  + fname                     + "\",";
  AnalysisABDE_Json += "\"lname\":\""  + lname                     + "\",";
  AnalysisABDE_Json += "\"email\":\""  + email                     + "\",";
  AnalysisABDE_Json += "\"level\":\""  + QString::number( level )  + "\"";
  AnalysisABDE_Json += "}],";
  
  AnalysisABDE_Json += "\"Comment\": \""   + gmp_form[ "Comment:" ]   + "\"";
  
  AnalysisABDE_Json += "}";
  
  //Record to autoflowStatus:
  qry.clear();

  int autoflowStatusID = prot_details[ "statusID" ].toInt();
  
  if ( autoflowStatusID )
    {
      //update
      qry << "update_autoflowStatusAnalysisABDE_record"    // to implement: add 'analysisABDE' && 'analysisABDEts' to autoflowStatus!!!!!
	  << QString::number( autoflowStatusID )
	  << prot_details[ "autoflowID" ] 
	  << AnalysisABDE_Json;
      
	  db->query( qry );
	  delete db;
    }
  else
    {
      QMessageBox::warning( this, tr( "AutoflowStatus Record Problem" ),
			    tr( "autoflowStatus (analysisABDE): There was a problem with identifying "
				"a record in autoflowStatus table for a given run! \n" ) );

      delete db;
      return;
    }

}


//Read autoflowABDEAnalysisStages record
int US_Norm_Profile::read_autoflowAnalyisABDEstages_record( QString autoflowID  )
{
   int status = 0;
  
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       delete db;
       return status;
     }


   //qDebug() << "BEFORE query ";
   QStringList qry;
   qry << "autoflow_abde_analysis_status"
       << autoflowID;
   
   status = db->statusQuery( qry );
   //qDebug() << "AFTER query ";

   delete db;
   return status;
}

//Set autoflowABDEAnalysisStages record back to "unlnown"
void US_Norm_Profile::revert_autoflowAnalysisABDEstages_record( QString autoflowID )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       delete db;
       return;
     }
   
   QStringList qry;
   qry << "autoflow_abde_analysis_status_revert"
       << autoflowID;

   qDebug() << "[in revert_autoflowAnalysisABDEstages_record(), query] -- "
	    << qry;
   
   db->query( qry );

   delete db;
}

void US_Norm_Profile::update_autoflow_record_atAnalysisABDE( void )
{
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       delete db;
       return;
     }

   QStringList qry;
   qry << "update_autoflow_at_analysis"
       << prot_details[ "autoflowID" ];

   //db->query( qry );

   int status = db->statusQuery( qry );
   
   if ( status == US_DB2::NO_AUTOFLOW_RECORD )
     {
       QMessageBox::warning( this,
			     tr( "Autoflow Record Not Updated" ),
			     tr( "No autoflow record\n"
				 "associated with this experiment." ) );
       delete db;
       return;
     }
   delete db;
}


QMap <QString, QString> US_Norm_Profile::read_autoflowAnalysisABDE_record( QString aID )
{
  QMap <QString, QString> abde_analysis_parms;
  
   // Check DB connection
   US_Passwd pw;
   QString masterpw = pw.getPasswd();
   US_DB2* db = new US_DB2( masterpw );

   if ( db->lastErrno() != US_DB2::OK )
     {
       QMessageBox::warning( this, tr( "Connection Problem" ),
			     tr( "Read protocol: Could not connect to database \n" ) + db->lastError() );
       delete db;
       return abde_analysis_parms;
     }
   
   QStringList qry;
   qry << "read_autoflowAnalysisABDE_record"
       << aID;
   db->query(qry);
   
   if ( db->lastErrno() == US_DB2::OK )      // Autoflow record exists
     {
       while ( db->next() )
	 {
	   abde_analysis_parms[ "ID" ]                = db->value( 0 ).toString();
	   abde_analysis_parms[ "etype" ]             = db->value( 1 ).toString();
	   abde_analysis_parms[ "xnorms_percents" ]   = db->value( 2 ).toString();
	   abde_analysis_parms[ "filename_blc" ]      = db->value( 3 ).toString();
	 }
     }
   
   delete db;
   return abde_analysis_parms;
}

void US_Norm_Profile::parse_abde_analysis_jsons( QString abde_analysis_parms_string,
						 QMap <QString, QString>& protocol_details,
						 QMap <QString, double>&  data_chann_x_norm,
						 QMap< QString, int >& data_chann_x_norm_cb,
						 QMap< QString, QMap < QString, double>>& data_chann_range_percent,
						 QMap <QString, double>&  data_chann_rmsd,
						 QMap <QString, double>&  data_chann_menisc )
{
  QString channels_to_radial_ranges;
  
  if ( !abde_analysis_parms_string.isEmpty() )
    {
      QJsonDocument jsonDoc = QJsonDocument::fromJson( abde_analysis_parms_string.toUtf8() );
      QJsonObject json_obj = jsonDoc.object();
      
      foreach(const QString& key, json_obj.keys())
	{
	  QJsonValue value = json_obj.value(key);
	  qDebug() << "XNORMS, PERCENTS key, value: " << key << value;

	  if ( key == "filename" )
	    {
	      qDebug() << "PARSED filename_abde -- " << value.toString();
	      protocol_details[ "filename_abde" ] = value.toString();
	    }
	  else if ( key == "blcorrs" )
	    {
	      qDebug() << "PARSED blccorrs -- " << value.toString();
	      protocol_details[ "baseline_corrections" ] = value.toString();
	    }
	  else //channNames...
	    {
	      channels_to_radial_ranges += key + ":";
	      
	      QJsonObject json_obj_1 = value.toObject();
	      foreach(const QString& key_1, json_obj_1.keys())
		{
		  QJsonValue value_1 = json_obj_1.value(key_1);
		  qDebug() << "XNORMS-1, PERCENTS-1 key-1, value-1: " << key_1 << value_1;
		  if ( key_1 == "x_norm" )
		    {
		      double x_norm_val      = value_1.toString().toDouble();
		      data_chann_x_norm[key] = x_norm_val;
		      data_chann_x_norm_cb[key] = (x_norm_val != -1) ? Qt::Unchecked : Qt::Checked ;
		    }
		  else if ( key_1 == "meniscus" )
		    {
		      double menisc_val = value_1.toString().toDouble();
		      data_chann_menisc[key] = menisc_val;
		    }
		  else if ( key_1 == "rmsd" )
		    {
		       double rmsd_val      = value_1.toString().toDouble();
		       data_chann_rmsd[key] = rmsd_val;
		    }
		  else if ( key_1 == "percents" )
		    {
		      QJsonObject json_obj_2 = value_1.toObject();
		      foreach(const QString& key_2, json_obj_2.keys())
			{
			  QJsonValue value_2 = json_obj_2.value(key_2);
			  //for ranges-to-percents
			  double percent_c = value_2.toString().toDouble();
			  data_chann_range_percent[key][key_2] = percent_c;
			  
			  //ned to make somethimg like
			  //protocol_details[ "channels_to_radial_ranges" ]
			  //   = QString("2A:6.2-6.5,6.6-6.9;4A:6.1-6.5,6.6-6.94;4B:6.25-6.55,6.65-7");
			  channels_to_radial_ranges += key_2 + ",";
			}
		      channels_to_radial_ranges.chop(1);
		    }
		}
	      channels_to_radial_ranges += ";";
	    }
	}
      channels_to_radial_ranges.chop(1);
    }
  
  if ( !channels_to_radial_ranges.isEmpty() )
    {
      qDebug() << "[PARSE], channels_to_radial_ranges -- " << channels_to_radial_ranges;
      protocol_details[ "channels_to_radial_ranges" ] = channels_to_radial_ranges;
    }
}
