#include "../include/us_eqmodelctrl.h"

US_EqModelControl::US_EqModelControl(vector <struct EquilScan_FitProfile> *temp_scanfit_vector,
                                     struct EquilRun_FitProfile *temp_runfit,vector <struct runinfo> *temp_runinfo_vector, int temp_model, bool *temp_model_widget, uint *temp_selected_scan,
                                     QWidget *p, const char *name) : QFrame( p, name)
{
   USglobal = new US_Config();

   int minHeight1 = 26;
   int minHeight2 = 30;
   buttonh = 26;
   border = 4;
   xpos = border;
   ypos = border;
   QString str;
   model_widget = temp_model_widget;
   *model_widget = true;
   model = temp_model;
   scanfit_vector = temp_scanfit_vector;
   runfit = temp_runfit;
   run_information= temp_runinfo_vector;
   selected_scan = temp_selected_scan;
   current_component = 0;

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   setCaption(tr("UltraScan: Equilibrium Model Control Window"));

   lbl_banner1 = new QLabel("", this);
   Q_CHECK_PTR(lbl_banner1);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner1->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner1->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner1->setGeometry(xpos, ypos, span, 2 * buttonh + spacing);
   lbl_banner1->setMinimumHeight(minHeight1*2);
   lbl_banner1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   ypos += 2 * buttonh + 4 * spacing;

   lbl_component1 = new QLabel(tr(" Global Run Parameters for Component:"), this);
   Q_CHECK_PTR(lbl_component1);
   lbl_component1->setAlignment(AlignLeft|AlignVCenter);
   lbl_component1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_component1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_component1->setGeometry(xpos, ypos, span, buttonh);
   lbl_component1->setMinimumHeight(minHeight1);

   xpos = span - column4 - 2 * column5 - column6 - 3 * spacing;

   cnt_component1= new QwtCounter(this);
   Q_CHECK_PTR(cnt_component1);
   cnt_component1->setRange(current_component + 1, (*runfit).components, 1);
   cnt_component1->setNumButtons(2);
   cnt_component1->setValue((double) current_component + 1);
   cnt_component1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_component1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_component1->setGeometry(xpos+3, ypos+1, span - xpos - column5 - spacing, buttonh-2);
   cnt_component1->setMinimumHeight(minHeight1);
   //   connect(cnt_component1, SIGNAL(buttonReleased(double)), SLOT(update_component1(double)));
   connect(cnt_component1, SIGNAL(valueChanged(double)), SLOT(update_component1(double)));

   xpos = border;
   ypos += buttonh;

   legend1 = new US_FitParameterLegend(this);
   legend1->setGeometry(xpos, ypos, span, buttonh);
   legend1->setMaximumHeight(minHeight2);

   xpos = border;
   ypos += buttonh + spacing;

   str.sprintf(tr(" Molecular Weight (%d):"), current_component + 1);
   lbl_mw = new QLabel(str, this);
   Q_CHECK_PTR(lbl_mw);
   lbl_mw->setAlignment(AlignLeft|AlignVCenter);
   lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_mw->setGeometry(xpos, ypos, column1, buttonh);
   lbl_mw->setMaximumHeight(minHeight1);

   xpos += column1 + spacing;

   fp_mw = new US_FitParameter(this);
   fp_mw->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_mw->setUnit("Dalton");
   fp_mw->updateValue((*runfit).mw[current_component]);
   fp_mw->updateRange((*runfit).mw_range[current_component]);
   connect(fp_mw->cb_float, SIGNAL(released()), SLOT(mw_float()));
   connect(fp_mw->cb_constrained, SIGNAL(released()), SLOT(mw_constrained()));
   connect(fp_mw->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_mw_value(const QString &)));
   connect(fp_mw->le_value, SIGNAL(textChanged(const QString &)), SLOT(update_sigma(const QString &)));
   connect(fp_mw->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_mw_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   str.sprintf("Vbar, 20ºC (%d):", current_component + 1);
   pb_vbar = new QPushButton(str, this);
   Q_CHECK_PTR(pb_vbar);
   pb_vbar->setAutoDefault(false);
   pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_vbar->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));

   xpos += column1 + spacing;

   fp_vbar = new US_FitParameter(this);
   fp_vbar->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_vbar->setUnit("ccm/g");
   fp_vbar->updateValue((*runfit).vbar20[current_component]);
   fp_vbar->updateRange((*runfit).vbar20_range[current_component]);
   connect(fp_vbar->cb_constrained, SIGNAL(released()), SLOT(vbar_constrained()));
   connect(fp_vbar->cb_float, SIGNAL(released()), SLOT(vbar_float()));
   connect(fp_vbar->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_vbar_value(const QString &)));
   connect(fp_vbar->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_vbar_range(const QString &)));

   /*
     xpos = border;
     ypos += buttonh + spacing;

     str.sprintf(tr(" Virial Coeff. (%d):"), current_component + 1);
     lbl_virial = new QLabel(str, this);
     Q_CHECK_PTR(lbl_virial);
     lbl_virial->setAlignment(AlignLeft|AlignVCenter);
     lbl_virial->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
     lbl_virial->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
     lbl_virial->setGeometry(xpos, ypos, column1, buttonh);

     xpos += column1 + spacing;

     fp_virial = new US_FitParameter(this);
     fp_virial->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
     fp_virial->setUnit("");
     fp_virial->updateValue((*runfit).virial[current_component]);
     fp_virial->updateRange((*runfit).virial_range[current_component]);
     connect(fp_virial->cb_constrained, SIGNAL(released()), SLOT(virial_constrained()));
     connect(fp_virial->cb_float, SIGNAL(released()), SLOT(virial_float()));
     connect(fp_virial->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_virial_value(const QString &)));
     connect(fp_virial->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_virial_range(const QString &)));
   */
   xpos = border;
   ypos += buttonh + spacing;

   eqconst1 = 1;
   pb_eqconst1 = new QPushButton(tr("ln(Assoc. Const. 1)"), this);
   Q_CHECK_PTR(pb_eqconst1);
   pb_eqconst1->setAutoDefault(false);
   pb_eqconst1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_eqconst1->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_eqconst1->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_eqconst1, SIGNAL(clicked()), SLOT(update_eqconst1()));

   xpos += column1 + spacing;

   fp_eqconst1 = new US_FitParameter(this);
   fp_eqconst1->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_eqconst1->updateValue((*runfit).eq[0]);
   fp_eqconst1->updateRange((*runfit).eq_range[0]);
   connect(fp_eqconst1->cb_constrained, SIGNAL(released()), SLOT(eqconst1_constrained()));
   connect(fp_eqconst1->cb_float, SIGNAL(released()), SLOT(eqconst1_float()));
   connect(fp_eqconst1->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst1_value(const QString &)));
   connect(fp_eqconst1->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst1_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   eqconst2 = 1;
   pb_eqconst2 = new QPushButton(tr("ln(Assoc. Const. 2)"), this);
   Q_CHECK_PTR(pb_eqconst2);
   pb_eqconst2->setAutoDefault(false);
   pb_eqconst2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_eqconst2->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_eqconst2->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_eqconst2, SIGNAL(clicked()), SLOT(update_eqconst2()));

   xpos += column1 + spacing;

   fp_eqconst2 = new US_FitParameter(this);
   fp_eqconst2->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_eqconst2->updateValue((*runfit).eq[1]);
   fp_eqconst2->updateRange((*runfit).eq_range[1]);
   connect(fp_eqconst2->cb_constrained, SIGNAL(released()), SLOT(eqconst2_constrained()));
   connect(fp_eqconst2->cb_float, SIGNAL(released()), SLOT(eqconst2_float()));
   connect(fp_eqconst2->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst2_value(const QString &)));
   connect(fp_eqconst2->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst2_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   eqconst3 = 1;
   pb_eqconst3 = new QPushButton(tr("ln(Assoc. Const. 3)"), this);
   Q_CHECK_PTR(pb_eqconst3);
   pb_eqconst3->setAutoDefault(false);
   pb_eqconst3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_eqconst3->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_eqconst3->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_eqconst3, SIGNAL(clicked()), SLOT(update_eqconst3()));

   xpos += column1 + spacing;

   fp_eqconst3 = new US_FitParameter(this);
   fp_eqconst3->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_eqconst3->updateValue((*runfit).eq[2]);
   fp_eqconst3->updateRange((*runfit).eq_range[2]);
   connect(fp_eqconst3->cb_constrained, SIGNAL(released()), SLOT(eqconst3_constrained()));
   connect(fp_eqconst3->cb_float, SIGNAL(released()), SLOT(eqconst3_float()));
   connect(fp_eqconst3->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst3_value(const QString &)));
   connect(fp_eqconst3->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst3_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   eqconst4 = 1;
   pb_eqconst4 = new QPushButton(tr("ln(Assoc. Const. 4)"), this);
   Q_CHECK_PTR(pb_eqconst4);
   pb_eqconst4->setAutoDefault(false);
   pb_eqconst4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_eqconst4->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_eqconst4->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_eqconst4, SIGNAL(clicked()), SLOT(update_eqconst4()));

   xpos += column1 + spacing;

   fp_eqconst4 = new US_FitParameter(this);
   fp_eqconst4->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_eqconst4->updateValue((*runfit).eq[3]);
   fp_eqconst4->updateRange((*runfit).eq_range[3]);
   connect(fp_eqconst4->cb_constrained, SIGNAL(released()), SLOT(eqconst4_constrained()));
   connect(fp_eqconst4->cb_float, SIGNAL(released()), SLOT(eqconst4_float()));
   connect(fp_eqconst4->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst4_value(const QString &)));
   connect(fp_eqconst4->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_eqconst4_range(const QString &)));

   xpos = border;
   ypos += buttonh + 4 * spacing;

   str.sprintf(tr("Local Scan Parameters for Scan %d"), *selected_scan + 1);
   lbl_banner2 = new QLabel(str, this);
   Q_CHECK_PTR(lbl_banner2);
   lbl_banner2->setFrameStyle(QFrame::WinPanel|Raised);
   lbl_banner2->setAlignment(AlignCenter|AlignVCenter);
   lbl_banner2->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_banner2->setGeometry(xpos, ypos, span, buttonh);
   lbl_banner2->setMinimumHeight(minHeight2);
   lbl_banner2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

   xpos = border;
   ypos += buttonh + 2 * spacing;

   lbl_description1 = new QLabel(tr(" Cell Description:"), this);
   Q_CHECK_PTR(lbl_description1);
   lbl_description1->setAlignment(AlignLeft|AlignVCenter);
   lbl_description1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_description1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_description1->setGeometry(xpos, ypos, column1, buttonh);

   xpos += column1 + spacing;

   lbl_description2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_description2);
   lbl_description2->setAlignment(AlignLeft|AlignVCenter);
   lbl_description2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_description2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_description2->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);

   xpos = border;
   ypos += buttonh + spacing;

   lbl_run_id1 = new QLabel(tr(" Run ID:"), this);
   Q_CHECK_PTR(lbl_run_id1);
   lbl_run_id1->setAlignment(AlignLeft|AlignVCenter);
   lbl_run_id1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_run_id1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_run_id1->setGeometry(xpos, ypos, column1, buttonh);

   xpos += column1 + spacing;

   lbl_run_id2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_run_id2);
   lbl_run_id2->setAlignment(AlignLeft|AlignVCenter);
   lbl_run_id2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_run_id2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_run_id2->setGeometry(xpos, ypos, column2, buttonh);

   xpos += column2 + spacing;

   lbl_temperature1 = new QLabel(tr(" Temperature:"), this);
   Q_CHECK_PTR(lbl_temperature1);
   lbl_temperature1->setAlignment(AlignLeft|AlignVCenter);
   lbl_temperature1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_temperature1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_temperature1->setGeometry(xpos, ypos, column3, buttonh);

   xpos += column3 + spacing;

   lbl_temperature2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_temperature2);
   lbl_temperature2->setAlignment(AlignLeft|AlignVCenter);
   lbl_temperature2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_temperature2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_temperature2->setGeometry(xpos, ypos, column2, buttonh);

   xpos = border;
   ypos += buttonh + spacing;

   lbl_speed1 = new QLabel(tr(" Rotor Speed:"), this);
   Q_CHECK_PTR(lbl_speed1);
   lbl_speed1->setAlignment(AlignLeft|AlignVCenter);
   lbl_speed1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_speed1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_speed1->setGeometry(xpos, ypos, column1, buttonh);

   xpos += column1 + spacing;

   lbl_speed2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_speed2);
   lbl_speed2->setAlignment(AlignLeft|AlignVCenter);
   lbl_speed2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_speed2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_speed2->setGeometry(xpos, ypos, column2, buttonh);

   xpos += column2 + spacing;

   lbl_wavelength1 = new QLabel(tr(" Wavelength:"), this);
   Q_CHECK_PTR(lbl_wavelength1);
   lbl_wavelength1->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_wavelength1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_wavelength1->setGeometry(xpos, ypos, column3, buttonh);

   xpos += column3 + spacing;

   lbl_wavelength2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_wavelength2);
   lbl_wavelength2->setAlignment(AlignLeft|AlignVCenter);
   lbl_wavelength2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_wavelength2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_wavelength2->setGeometry(xpos, ypos, column2, buttonh);

   xpos = border;
   ypos += buttonh + spacing;

   lbl_pathlength = new QLabel(tr(" Cell Pathlength (cm):"), this);
   Q_CHECK_PTR(lbl_pathlength);
   lbl_pathlength->setAlignment(AlignLeft|AlignVCenter);
   lbl_pathlength->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pathlength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_pathlength->setGeometry(xpos, ypos, column1, buttonh);

   xpos += column1 + spacing;

   le_pathlength = new QLineEdit(this);
   le_pathlength->setGeometry(xpos, ypos, column2, buttonh);
   le_pathlength->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pathlength->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pathlength, SIGNAL(textChanged(const QString &)), SLOT(update_pathlength2(const QString &)));

   xpos += column2 + spacing;

   pb_pathlength_apply = new QPushButton(tr("Apply to:"), this);
   Q_CHECK_PTR(pb_pathlength_apply);
   pb_pathlength_apply->setAutoDefault(false);
   pb_pathlength_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_pathlength_apply->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_pathlength_apply->setGeometry(xpos, ypos, column3,  buttonh);
   connect(pb_pathlength_apply, SIGNAL(clicked()), SLOT(apply_pathlength_range()));

   xpos += column3 + spacing;

   lbl_pathlength_apply1 = new QLabel(tr(" Scan"), this);
   Q_CHECK_PTR(lbl_pathlength_apply1);
   lbl_pathlength_apply1->setAlignment(AlignLeft|AlignVCenter);
   lbl_pathlength_apply1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_pathlength_apply1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_pathlength_apply1->setGeometry(xpos, ypos, 40, buttonh);

   xpos += 42;

   pathlength_range_start = 0;
   le_pathlength_range_start = new QLineEdit(this);
   le_pathlength_range_start->setGeometry(xpos, ypos, 32, buttonh);
   le_pathlength_range_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pathlength_range_start->setText(" 1");
   le_pathlength_range_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pathlength_range_start, SIGNAL(textChanged(const QString &)), SLOT(update_pathlength_range_start(const QString &)));

   xpos += 34;

   lbl_pathlength_apply2 = new QLabel(tr(" to Scan"), this);
   Q_CHECK_PTR(lbl_pathlength_apply2);
   lbl_pathlength_apply2->setAlignment(AlignLeft|AlignVCenter);
   lbl_pathlength_apply2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_pathlength_apply2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_pathlength_apply2->setGeometry(xpos, ypos, 53, buttonh);

   xpos += 55;

   pathlength_range_stop = 0;
   le_pathlength_range_stop = new QLineEdit(this);
   le_pathlength_range_stop->setGeometry(xpos, ypos, 32, buttonh);
   le_pathlength_range_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pathlength_range_stop->setText(" 1");
   le_pathlength_range_stop->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_pathlength_range_stop, SIGNAL(textChanged(const QString &)), SLOT(update_pathlength_range_stop(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   legend2 = new US_FitParameterLegend(this);
   legend2->setGeometry(xpos, ypos, span, buttonh);
   legend2->setMaximumHeight(minHeight2);

   xpos = border;
   ypos += buttonh + spacing;

   lbl_baseline = new QLabel(tr(" Baseline:"), this);
   Q_CHECK_PTR(lbl_baseline);
   lbl_baseline->setAlignment(AlignLeft|AlignVCenter);
   lbl_baseline->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_baseline->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_baseline->setGeometry(xpos, ypos, column1, buttonh);
   lbl_baseline->setMaximumHeight(minHeight1);

   xpos += column1 + spacing;

   fp_baseline = new US_FitParameter(this);
   fp_baseline->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_baseline->setUnit("OD/Fringes");
   connect(fp_baseline->cb_constrained, SIGNAL(released()), SLOT(baseline_constrained()));
   connect(fp_baseline->cb_float, SIGNAL(released()), SLOT(baseline_float()));
   connect(fp_baseline->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_baseline_value(const QString &)));
   connect(fp_baseline->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_baseline_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   pb_density = new QPushButton(tr("Density, 20ºC"), this);
   Q_CHECK_PTR(pb_density);
   pb_density->setAutoDefault(false);
   pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_density->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_density->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_density, SIGNAL(clicked()), SLOT(update_density1()));

   xpos += column1 + spacing;

   le_density = new QLineEdit(this);
   le_density->setGeometry(xpos, ypos, column2, buttonh);
   le_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_density->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_density, SIGNAL(textChanged(const QString &)), SLOT(update_density2(const QString &)));

   xpos += column2 + spacing;

   pb_density_apply = new QPushButton(tr("Apply to:"), this);
   Q_CHECK_PTR(pb_density_apply);
   pb_density_apply->setAutoDefault(false);
   pb_density_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_density_apply->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_density_apply->setGeometry(xpos, ypos, column3,  buttonh);
   connect(pb_density_apply, SIGNAL(clicked()), SLOT(apply_density_range()));

   xpos += column3 + spacing;

   lbl_density_apply1 = new QLabel(tr(" Scan"), this);
   Q_CHECK_PTR(lbl_density_apply1);
   lbl_density_apply1->setAlignment(AlignLeft|AlignVCenter);
   lbl_density_apply1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_density_apply1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_density_apply1->setGeometry(xpos, ypos, 40, buttonh);

   xpos += 42;

   scan_range_start = 0;
   le_scan_range_start = new QLineEdit(this);
   le_scan_range_start->setGeometry(xpos, ypos, 32, buttonh);
   le_scan_range_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scan_range_start->setText(" 1");
   le_scan_range_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_scan_range_start, SIGNAL(textChanged(const QString &)), SLOT(update_scan_range_start(const QString &)));

   xpos += 34;

   lbl_density_apply2 = new QLabel(tr(" to Scan"), this);
   Q_CHECK_PTR(lbl_density_apply2);
   lbl_density_apply2->setAlignment(AlignLeft|AlignVCenter);
   lbl_density_apply2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_density_apply2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_density_apply2->setGeometry(xpos, ypos, 53, buttonh);

   xpos += 55;

   scan_range_stop = 0;
   le_scan_range_stop = new QLineEdit(this);
   le_scan_range_stop->setGeometry(xpos, ypos, 32, buttonh);
   le_scan_range_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_scan_range_stop->setText(" 1");
   le_scan_range_stop->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_scan_range_stop, SIGNAL(textChanged(const QString &)), SLOT(update_scan_range_stop(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_component2 = new QLabel(tr(" Local Scan Parameters for Component:"), this);
   Q_CHECK_PTR(lbl_component2);
   lbl_component2->setAlignment(AlignLeft|AlignVCenter);
   lbl_component2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_component2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_component2->setGeometry(xpos, ypos, span, buttonh);

   xpos = span - column4 - 2 * column5 - column6 - 3 * spacing;

   cnt_component2= new QwtCounter(this);
   Q_CHECK_PTR(cnt_component2);
   cnt_component2->setRange(0, 0, 1);
   cnt_component2->setNumButtons(2);
   cnt_component2->setValue((double) (current_component + 1));
   cnt_component2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_component2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_component2->setGeometry(xpos+3, ypos+1, span - xpos - column5 - spacing, buttonh-2);
   //   connect(cnt_component2, SIGNAL(buttonReleased(double)), SLOT(update_component2(double)));
   connect(cnt_component2, SIGNAL(valueChanged(double)), SLOT(update_component2(double)));

   xpos = border;
   ypos += buttonh;

   legend3 = new US_FitParameterLegend(this);
   legend3->setGeometry(xpos, ypos, span, buttonh);
   legend3->setMaximumHeight(minHeight2);

   xpos = border;
   ypos += buttonh + spacing;

   str.sprintf(tr(" Amplitude (%d):"), current_component+1);
   lbl_amplitude = new QLabel(str, this);
   Q_CHECK_PTR(lbl_amplitude);
   lbl_amplitude->setAlignment(AlignLeft|AlignVCenter);
   lbl_amplitude->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_amplitude->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_amplitude->setGeometry(xpos, ypos, column1, buttonh);
   lbl_amplitude->setMaximumHeight(minHeight1);


   xpos += column1 + spacing;

   fp_amplitude = new US_FitParameter(this);
   fp_amplitude->setGeometry(xpos, ypos, span - column1 - spacing, buttonh);
   fp_amplitude->setUnit("OD/Fringes");
   connect(fp_amplitude->cb_constrained, SIGNAL(released()), SLOT(amplitude_constrained()));
   connect(fp_amplitude->cb_float, SIGNAL(released()), SLOT(amplitude_float()));
   connect(fp_amplitude->le_value, SIGNAL(textChanged(const QString &)), SLOT(get_amplitude_value(const QString &)));
   connect(fp_amplitude->le_range, SIGNAL(textChanged(const QString &)), SLOT(get_amplitude_range(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   str.sprintf("E (OD/(cm*mol)) (%d):", current_component+1);
   pb_extinction = new QPushButton(str, this);
   Q_CHECK_PTR(pb_extinction);
   pb_extinction->setAutoDefault(false);
   pb_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_extinction->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_extinction->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_extinction, SIGNAL(clicked()), SLOT(update_extinction1()));

   xpos += column1 + spacing;

   le_extinction = new QLineEdit(this);
   le_extinction->setGeometry(xpos, ypos, column2, buttonh);
   le_extinction->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_extinction->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_extinction, SIGNAL(textChanged(const QString &)), SLOT(update_extinction2(const QString &)));

   xpos += column2 + spacing;

   pb_extinction_apply = new QPushButton(tr("Apply to:"), this);
   Q_CHECK_PTR(pb_extinction_apply);
   pb_extinction_apply->setAutoDefault(false);
   pb_extinction_apply->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_extinction_apply->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_extinction_apply->setGeometry(xpos, ypos, column3,  buttonh);
   connect(pb_extinction_apply, SIGNAL(clicked()), SLOT(apply_extinction_range()));

   xpos += column3 + spacing;

   lbl_extinction_apply1 = new QLabel(tr(" Scan"), this);
   Q_CHECK_PTR(lbl_extinction_apply1);
   lbl_extinction_apply1->setAlignment(AlignLeft|AlignVCenter);
   lbl_extinction_apply1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_extinction_apply1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_extinction_apply1->setGeometry(xpos, ypos, 40, buttonh);

   xpos += 42;

   ext_range_start = 0;
   le_ext_range_start = new QLineEdit(this);
   le_ext_range_start->setGeometry(xpos, ypos, 32, buttonh);
   le_ext_range_start->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_ext_range_start->setText(" 1");
   le_ext_range_start->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_ext_range_start, SIGNAL(textChanged(const QString &)), SLOT(update_ext_range_start(const QString &)));

   xpos += 34;

   lbl_extinction_apply2 = new QLabel(tr(" to Scan"), this);
   Q_CHECK_PTR(lbl_extinction_apply2);
   lbl_extinction_apply2->setAlignment(AlignLeft|AlignVCenter);
   lbl_extinction_apply2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   lbl_extinction_apply2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_extinction_apply2->setGeometry(xpos, ypos, 53, buttonh);

   xpos += 55;

   ext_range_stop = 0;
   le_ext_range_stop = new QLineEdit(this);
   le_ext_range_stop->setGeometry(xpos, ypos, 32, buttonh);
   le_ext_range_stop->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_ext_range_stop->setText(" 1");
   le_ext_range_stop->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(le_ext_range_stop, SIGNAL(textChanged(const QString &)), SLOT(update_ext_range_stop(const QString &)));

   xpos = border;
   ypos += buttonh + spacing;

   lbl_sigma1 = new QLabel(tr(" Sigma for this Scan:"), this);
   Q_CHECK_PTR(lbl_sigma1);
   lbl_sigma1->setAlignment(AlignLeft|AlignVCenter);
   lbl_sigma1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sigma1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_sigma1->setGeometry(xpos, ypos, column1, buttonh);

   xpos += column1 + spacing;

   lbl_sigma2 = new QLabel("", this);
   Q_CHECK_PTR(lbl_sigma2);
   lbl_sigma2->setAlignment(AlignLeft|AlignVCenter);
   lbl_sigma2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
   lbl_sigma2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lbl_sigma2->setGeometry(xpos, ypos, column2, buttonh);

   xpos = border;
   ypos += buttonh + spacing;
   /*
     lbl_include = new QLabel(tr(" Include this Scan in the Fit:"), this);
     Q_CHECK_PTR(lbl_include);
     lbl_include->setAlignment(AlignLeft|AlignVCenter);
     lbl_include->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
     lbl_include->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
     lbl_include->setGeometry(xpos, ypos, span, buttonh);
   */
   xpos += column1 + spacing;

   cb_include = new QCheckBox(tr(" Include this Scan in the Fit:"),this);
   Q_CHECK_PTR(cb_include);
   cb_include->setGeometry(xpos + (unsigned int) column2/2, ypos+5, 14, 14);
   cb_include->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cb_include->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(cb_include, SIGNAL(clicked()), SLOT(change_include()));

   xpos += column2 + spacing;

   lbl_active_scan = new QLabel(tr(" Scan Selector:"), this);
   Q_CHECK_PTR(lbl_active_scan);
   lbl_active_scan->setAlignment(AlignLeft|AlignVCenter);
   lbl_active_scan->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_active_scan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
   lbl_active_scan->setGeometry(xpos, ypos, column3, buttonh);

   xpos = span - column4 - 2 * column5 - column6 - 3 * spacing;

   cnt_active_scan = new QwtCounter(this);
   Q_CHECK_PTR(cnt_active_scan);
   cnt_active_scan->setRange(1, (*scanfit_vector).size(), 1);
   cnt_active_scan->setNumButtons(3);
   cnt_active_scan->setValue((double) (*selected_scan + 1));
   cnt_active_scan->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   cnt_active_scan->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   cnt_active_scan->setGeometry(xpos+3, ypos+1, span - xpos, buttonh-2);

   xpos = border;
   ypos += buttonh + spacing;

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_help->setGeometry(xpos, ypos, column1,  buttonh);
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   xpos += column1 + spacing;

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   pb_close->setGeometry(xpos, ypos, column2,  buttonh);
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   /*   ypos += buttonh + border;
        global_Xpos += 30;
        global_Ypos += 30;
        setGeometry(global_Xpos, global_Ypos, span + 2 * border, ypos);
        setMinimumSize(span + 2 * border, ypos);
        setMaximumSize(span + 2 * border, ypos);
   */

   setup_GUI();


   eqconst1_float((*runfit).eq_fit[0]);
   eqconst2_float((*runfit).eq_fit[1]);
   eqconst3_float((*runfit).eq_fit[2]);
   eqconst4_float((*runfit).eq_fit[3]);

   setup_model();
   update_scan_info(*selected_scan);
   cb_include->setChecked((*scanfit_vector)[*selected_scan].FitScanFlag);
   //   virial_float((*runfit).virial_fit[current_component]);
   vbar_float((*runfit).vbar20_fit[current_component]);
   mw_float((*runfit).mw_fit[current_component]);
   baseline_float((*scanfit_vector)[*selected_scan].baseline_fit);

   amplitude_float((*scanfit_vector)[*selected_scan].amplitude_fit[current_component]);
   update_sigma(str);
}

US_EqModelControl::~US_EqModelControl()
{
}

void US_EqModelControl::setup_GUI()
{
   int j=0;
   int rows = 22, columns = 5, spacing = 2;

   QGridLayout * background = new QGridLayout(this,rows,columns,spacing);
   for(int i=0; i<22; i++)
   {
      background->setRowSpacing(i, 26);
   }
   background->addMultiCellWidget(lbl_banner1,j,j,0,4);
   j++;
   background->addMultiCellWidget(lbl_component1,j,j,0,3);
   background->addWidget(cnt_component1,j,4);
   j++;
   background->addMultiCellWidget(legend1,j,j,0,4);
   j++;
   background->addWidget(lbl_mw,j,0);
   background->addMultiCellWidget(fp_mw,j,j,1,4);
   j++;
   background->addWidget(pb_vbar,j,0);
   background->addMultiCellWidget(fp_vbar,j,j,1,4);
   j++;
   background->addWidget(pb_eqconst1,j,0);
   background->addMultiCellWidget(fp_eqconst1,j,j,1,4);
   j++;
   background->addWidget(pb_eqconst2,j,0);
   background->addMultiCellWidget(fp_eqconst2,j,j,1,4);
   j++;
   background->addWidget(pb_eqconst3,j,0);
   background->addMultiCellWidget(fp_eqconst3,j,j,1,4);
   j++;
   background->addWidget(pb_eqconst4,j,0);
   background->addMultiCellWidget(fp_eqconst4,j,j,1,4);
   j++;
   background->addMultiCellWidget(lbl_banner2,j,j,0,4);
   j++;
   background->addWidget(lbl_description1,j,0);
   background->addMultiCellWidget(lbl_description2,j,j,1,4);
   j++;
   background->addWidget(lbl_run_id1,j,0);
   background->addWidget(lbl_run_id2,j,1);
   background->addWidget(lbl_temperature1,j,2);
   background->addWidget(lbl_temperature2,j,3);
   j++;
   background->addWidget(lbl_speed1,j,0);
   background->addWidget(lbl_speed2,j,1);
   background->addWidget(lbl_wavelength1,j,2);
   background->addWidget(lbl_wavelength2,j,3);
   j++;
   background->addWidget(lbl_pathlength,j,0);
   background->addWidget(le_pathlength,j,1);
   background->addWidget(pb_pathlength_apply,j,2);
   QHBoxLayout *scan1= new QHBoxLayout();
   scan1->addWidget(lbl_pathlength_apply1);
   scan1->addWidget(le_pathlength_range_start);
   scan1->addWidget(lbl_pathlength_apply2);
   scan1->addWidget(le_pathlength_range_stop);
   background->addMultiCellLayout(scan1,j,j,3,4);
   j++;
   background->addMultiCellWidget(legend2,j,j,0,4);
   j++;
   background->addWidget(lbl_baseline,j,0);
   background->addMultiCellWidget(fp_baseline,j,j,1,4);
   j++;
   background->addWidget(pb_density,j,0);
   background->addWidget(le_density,j,1);
   background->addWidget(pb_density_apply,j,2);
   QHBoxLayout *scan2= new QHBoxLayout();
   scan2->addWidget(lbl_density_apply1);
   scan2->addWidget(le_scan_range_start);
   scan2->addWidget(lbl_density_apply2);
   scan2->addWidget(le_scan_range_stop);
   background->addMultiCellLayout(scan2,j,j,3,4);
   j++;
   background->addMultiCellWidget(lbl_component2,j,j,0,3);
   background->addWidget(cnt_component2,j,4);
   j++;
   background->addMultiCellWidget(legend3,j,j,0,4);
   j++;
   background->addWidget(lbl_amplitude,j,0);
   background->addMultiCellWidget(fp_amplitude,j,j,1,4);
   j++;
   background->addWidget(pb_extinction,j,0);
   background->addWidget(le_extinction,j,1);
   background->addWidget(pb_extinction_apply,j,2);
   QHBoxLayout *scan3= new QHBoxLayout();
   scan3->addWidget(lbl_extinction_apply1);
   scan3->addWidget(le_ext_range_start);
   scan3->addWidget(lbl_extinction_apply2);
   scan3->addWidget(le_ext_range_stop);
   background->addMultiCellLayout(scan3,j,j,3,4);
   j++;
   background->addWidget(lbl_sigma1,j,0);
   background->addWidget(lbl_sigma2,j,1);
   j++;
   background->addMultiCellWidget(cb_include,j,j,0,1);
   background->addMultiCellWidget(lbl_active_scan,j,j,2,3);
   background->addWidget(cnt_active_scan,j,4);
   j++;
   background->addWidget(pb_help,j,0);
   background->addWidget(pb_close,j,1);


   qApp->processEvents();
   QRect r = background->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());







}

void US_EqModelControl::closeEvent(QCloseEvent *e)
{
   e->accept();
   global_Xpos -= 30;
   global_Ypos -= 30;
   *model_widget = false;
   emit ControlWindowClosed();
}

// first component counter was clicked - update pushbutton labels and current_component
void US_EqModelControl::update_component1(double val)
{
   current_component = (unsigned int) val - 1;
   update_component_labels();
   cnt_component2->setValue(val);
}

void US_EqModelControl::update_component_labels()
{
   QString str;
   eqconst1 = 3;
   eqconst2 = 3;
   eqconst3 = 3;
   eqconst4 = 3;
   str.sprintf(tr(" Molecular Weight (%d):"), current_component + 1);
   lbl_mw->setText(str);
   str.sprintf("%1.5e", (*runfit).mw[current_component]);
   fp_mw->le_value->setText(str);
   fp_mw->cb_float->setChecked((*runfit).mw_fit[current_component]);
   mw_float((*runfit).mw_fit[current_component]);

   str.sprintf("Vbar, 20ºC (%d):", current_component + 1);
   pb_vbar->setText(str);
   str.sprintf("%1.5e", (*runfit).vbar20[current_component]);
   fp_vbar->le_value->setText(str);
   fp_vbar->cb_float->setChecked((*runfit).vbar20_fit[current_component]);
   vbar_float((*runfit).vbar20_fit[current_component]);
   /*
     str.sprintf(tr(" Virial Coeff. (%d):"), current_component + 1);
     lbl_virial->setText(str);
     str.sprintf("%1.5e", (*runfit).virial[current_component]);
     fp_virial->le_value->setText(str);
     fp_virial->cb_float->setChecked((*runfit).virial_fit[current_component]);
     virial_float((*runfit).virial_fit[current_component]);
   */
   if (pb_eqconst1->isEnabled())
   {
      eqconst1 = 1;
      str.sprintf(tr("ln(Assoc. Const. 1):"));
      pb_eqconst1->setText(str);
      str.sprintf("%1.5e", (*runfit).eq[0]);
      fp_eqconst1->le_value->setText(str);
      fp_eqconst1->cb_float->setChecked((*runfit).eq_fit[0]);
      eqconst1_float((*runfit).eq_fit[0]);
   }

   if (pb_eqconst2->isEnabled())
   {
      eqconst2 = 1;
      str.sprintf(tr("ln(Assoc. Const. 2):"));
      pb_eqconst2->setText(str);
      str.sprintf("%1.5e", (*runfit).eq[1]);
      fp_eqconst2->le_value->setText(str);
      fp_eqconst2->cb_float->setChecked((*runfit).eq_fit[1]);
      eqconst2_float((*runfit).eq_fit[1]);
   }

   if (pb_eqconst3->isEnabled())
   {
      eqconst3 = 1;
      str.sprintf(tr("ln(Assoc. Const. 3):"));
      pb_eqconst3->setText(str);
      str.sprintf("%1.5e", (*runfit).eq[2]);
      fp_eqconst3->le_value->setText(str);
      fp_eqconst3->cb_float->setChecked((*runfit).eq_fit[2]);
      eqconst3_float((*runfit).eq_fit[2]);
   }

   if (pb_eqconst4->isEnabled())
   {
      eqconst4 = 1;
      str.sprintf(tr("ln(Assoc. Const. 4):"));
      pb_eqconst4->setText(str);
      str.sprintf("%1.5e", (*runfit).eq[3]);
      fp_eqconst4->le_value->setText(str);
      fp_eqconst4->cb_float->setChecked((*runfit).eq_fit[3]);
      eqconst4_float((*runfit).eq_fit[3]);
   }

   str.sprintf(tr(" Amplitude (%d):"), current_component + 1);
   lbl_amplitude->setText(str);
   str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].amplitude[current_component]);
   fp_amplitude->le_value->setText(str);
   fp_amplitude->cb_float->setChecked((*scanfit_vector)[*selected_scan].amplitude_fit[current_component]);
   if ((*scanfit_vector)[*selected_scan].FitScanFlag)
   {
      amplitude_float((*scanfit_vector)[*selected_scan].amplitude_fit[current_component]);
   }

   str.sprintf("E (OD/(cm*mol)) (%d):", current_component + 1);
   pb_extinction->setText(str);
   str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].extinction[current_component]);
   le_extinction->setText(str);

   update_sigma(str);
   update_density_label((*scanfit_vector)[*selected_scan].density, 0);
}

void US_EqModelControl::update_component2(double val)
{
   current_component = (unsigned int) val - 1;
   update_component_labels();
   cnt_component1->setValue(val);
}

void US_EqModelControl::update_scan_range_start(const QString &str)
{
   QString str1;
   scan_range_start = str.toInt();
   if (scan_range_start > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      scan_range_start = 1;
      str1.sprintf(" %d", scan_range_start);
      le_scan_range_start->setText(str1);
      return;
   }
   if (scan_range_start < 1)
   {
      scan_range_start = 1;
      str1.sprintf(" %d", scan_range_start);
      le_scan_range_start->setText(str1);
      return;
   }
   scan_range_start--;
}

void US_EqModelControl::update_scan_range_stop(const QString &str)
{
   QString str1;
   scan_range_stop = str.toInt();
   if (scan_range_stop > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      scan_range_stop = (*scanfit_vector).size();
      str1.sprintf(" %d", scan_range_stop);
      le_scan_range_stop->setText(str1);
      return;
   }
   if (scan_range_stop < 1)
   {
      scan_range_stop = 1;
      str1.sprintf(" %d", scan_range_stop);
      le_scan_range_stop->setText(str1);
      return;
   }
   scan_range_stop--;
}

void US_EqModelControl::apply_density_range()
{
   if(scan_range_start > scan_range_stop)
   {
      QMessageBox::message(tr("Attention:"), tr("Sorry, invalid Range!"));
      scan_range_start = 0;
      scan_range_stop = 0;
      le_scan_range_start->setText(" 1");
      le_scan_range_stop->setText(" 1");
      return;
   }
   for (uint i=scan_range_start; i<=scan_range_stop; i++)
   {
      if (i != *selected_scan)
      {
         (*scanfit_vector)[i].density = (*scanfit_vector)[*selected_scan].density;
      }
   }
}

void US_EqModelControl::update_ext_range_start(const QString &str)
{
   QString str1;
   ext_range_start = str.toInt();
   if (ext_range_start > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      ext_range_start = 1;
      str1.sprintf(" %d", ext_range_start);
      le_ext_range_start->setText(str1);
      return;
   }
   if (ext_range_start < 1)
   {
      ext_range_start = 1;
      str1.sprintf(" %d", ext_range_start);
      le_ext_range_start->setText(str1);
      return;
   }
   ext_range_start--;
}

void US_EqModelControl::update_ext_range_stop(const QString &str)
{
   QString str1;
   ext_range_stop = str.toInt();
   if (ext_range_stop > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      ext_range_stop = (*scanfit_vector).size();
      str1.sprintf(" %d", ext_range_stop);
      le_ext_range_stop->setText(str1);
      return;
   }
   if (ext_range_stop < 1)
   {
      ext_range_stop = 1;
      str1.sprintf(" %d", ext_range_stop);
      le_ext_range_stop->setText(str1);
      return;
   }
   ext_range_stop--;
}

void US_EqModelControl::apply_extinction_range()
{
   if(ext_range_start > ext_range_stop)
   {
      QMessageBox::message(tr("Attention:"), tr("Sorry, invalid Range!"));
      ext_range_start = 0;
      ext_range_stop = 0;
      le_ext_range_start->setText(" 1");
      le_ext_range_stop->setText(" 1");
      return;
   }
   for (uint i=ext_range_start; i<=ext_range_stop; i++)
   {
      if (i != *selected_scan)
      {
         (*scanfit_vector)[i].extinction[current_component] = (*scanfit_vector)[*selected_scan].extinction[current_component];
      }
   }
}

void US_EqModelControl::update_pathlength_range_start(const QString &str)
{
   QString str1;
   pathlength_range_start = str.toUInt();
   if (pathlength_range_start > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      pathlength_range_start = 1;
      str1.sprintf(" %d", pathlength_range_start);
      le_pathlength_range_start->setText(str1);
      return;
   }
   if (pathlength_range_start < 1)
   {
      pathlength_range_start = 1;
      str1.sprintf(" %d", pathlength_range_start);
      le_pathlength_range_start->setText(str1);
      return;
   }
   pathlength_range_start--;
}

void US_EqModelControl::update_pathlength_range_stop(const QString &str)
{
   QString str1;
   pathlength_range_stop = str.toUInt();
   if (pathlength_range_stop > (*scanfit_vector).size())
   {
      QMessageBox::message(tr("Attention:"), tr("Not that many scans available!"));
      pathlength_range_stop = (*scanfit_vector).size();
      str1.sprintf(" %d", pathlength_range_stop);
      le_pathlength_range_stop->setText(str1);
      return;
   }
   if (pathlength_range_stop < 1)
   {
      pathlength_range_stop = 1;
      str1.sprintf(" %d", pathlength_range_stop);
      le_pathlength_range_stop->setText(str1);
      return;
   }
   pathlength_range_stop--;
}

void US_EqModelControl::apply_pathlength_range()
{
   if(pathlength_range_start > pathlength_range_stop)
   {
      QMessageBox::message(tr("Attention:"), tr("Sorry, invalid Range!"));
      pathlength_range_start = 0;
      pathlength_range_stop = 0;
      le_pathlength_range_start->setText(" 1");
      le_pathlength_range_stop->setText(" 1");
      return;
   }
   for (uint i=pathlength_range_start; i<=pathlength_range_stop; i++)
   {
      if (i != *selected_scan)
      {
         (*scanfit_vector)[i].pathlength = (*scanfit_vector)[*selected_scan].pathlength;
      }
   }
}

void US_EqModelControl::get_mw_value(const QString &str)
{
   (*runfit).mw[current_component] = str.toFloat();
}

void US_EqModelControl::get_mw_range(const QString &str)
{
   (*runfit).mw_range[current_component] = str.toFloat();
}

void US_EqModelControl::mw_constrained()
{
   if (fp_mw->cb_constrained->isChecked())
   {
      (*runfit).mw_bound[current_component] = true;
   }
   else
   {
      (*runfit).mw_bound[current_component] = false;
   }
}

void US_EqModelControl::mw_float()
{
   if (fp_mw->cb_float->isChecked())
   {
      mw_float(true);
   }
   else
   {
      mw_float(false);
   }
}

void US_EqModelControl::mw_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).mw_fit[current_component] = true;
      if ((*runfit).mw_range[current_component] == 0.0)
      {
         (*runfit).mw_range[current_component] = (*runfit).mw[current_component] / 5.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).mw_range[current_component]);
      fp_mw->le_range->setText(str);
      fp_mw->le_range->setEnabled(true);
      fp_mw->cb_float->setChecked(true);
      if ((*runfit).vbar20_fit[current_component])
      {
         (*runfit).vbar20_fit[current_component] = false;   // we can't float both
         (*runfit).vbar20_range[current_component] = 0.0;
         str.sprintf("%1.5e", 0.0);
         fp_vbar->le_range->setText(str);
         fp_vbar->le_range->setEnabled(false);
         fp_vbar->cb_float->setChecked(false);
      }
   }
   else
   {
      (*runfit).mw_fit[current_component] = false;   // we can't float both
      (*runfit).mw_range[current_component] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_mw->le_range->setText(str);
      fp_mw->le_range->setEnabled(false);
      fp_mw->cb_float->setChecked(false);
   }
}

void US_EqModelControl::vbar_constrained()
{
   if (fp_vbar->cb_constrained->isChecked())
   {
      (*runfit).vbar20_bound[current_component] = true;
   }
   else
   {
      (*runfit).vbar20_bound[current_component] = false;
   }
}

void US_EqModelControl::vbar_float()
{
   if (fp_vbar->cb_float->isChecked())
   {
      vbar_float(true);
   }
   else
   {
      vbar_float(false);
   }
}

void US_EqModelControl::vbar_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).vbar20_fit[current_component] = true;
      if ((*runfit).vbar20_range[current_component] == 0.0)
      {
         (*runfit).vbar20_range[current_component] = (*runfit).vbar20[current_component] / 5.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).vbar20_range[current_component]);
      fp_vbar->le_range->setText(str);
      fp_vbar->le_range->setEnabled(true);
      fp_vbar->cb_float->setChecked(true);
      if ((*runfit).mw_fit[current_component])
      {
         (*runfit).mw_fit[current_component] = false;   // we can't float both
         (*runfit).mw_range[current_component] = 0.0;
         str.sprintf("%1.5e", 0.0);
         fp_mw->le_range->setText(str);
         fp_mw->le_range->setEnabled(false);
         fp_mw->cb_float->setChecked(false);
      }
   }
   else
   {
      (*runfit).vbar20_fit[current_component] = false;   // we can't float both
      (*runfit).vbar20_range[current_component] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_vbar->le_range->setText(str);
      fp_vbar->le_range->setEnabled(false);
      fp_vbar->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_vbar_value(const QString &str)
{
   (*runfit).vbar20[current_component] = str.toFloat();
}

void US_EqModelControl::get_vbar_range(const QString &str)
{
   (*runfit).vbar20_range[current_component] = str.toFloat();
}

void US_EqModelControl::virial_constrained()
{
   /*
     if (fp_virial->cb_constrained->isChecked())
     {
     (*runfit).virial_bound[current_component] = true;
     }
     else
     {
     (*runfit).virial_bound[current_component] = false;
     }
   */
}

void US_EqModelControl::virial_float()
{
   if (fp_virial->cb_float->isChecked())
   {
      virial_float(true);
   }
   else
   {
      virial_float(false);
   }
}

void US_EqModelControl::virial_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).virial_fit[current_component] = true;
      if((*runfit).virial_range[current_component] == 0.0)
      {
         (*runfit).virial_range[current_component] = (*runfit).virial[current_component] / 5.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).virial_range[current_component]);
      fp_virial->le_range->setText(str);
      fp_virial->le_range->setEnabled(true);
      fp_virial->cb_float->setChecked(true);
   }
   else
   {
      (*runfit).virial_fit[current_component] = false;   // we can't float both
      (*runfit).virial_range[current_component] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_virial->le_range->setText(str);
      fp_virial->le_range->setEnabled(false);
      fp_virial->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_virial_value(const QString &str)
{
   (*runfit).virial[current_component] = str.toFloat();
}

void US_EqModelControl::get_virial_range(const QString &str)
{
   (*runfit).virial_range[current_component] = str.toFloat();
}

void US_EqModelControl::eqconst1_constrained()
{
   if (fp_eqconst1->cb_constrained->isChecked())
   {
      (*runfit).eq_bound[0] = true;
   }
   else
   {
      (*runfit).eq_bound[0] = false;
   }
}

void US_EqModelControl::eqconst1_float()
{
   if (fp_eqconst1->cb_float->isChecked())
   {
      eqconst1_float(true);
   }
   else
   {
      eqconst1_float(false);
   }
}

void US_EqModelControl::eqconst1_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).eq_fit[0] = true;
      if ((*runfit).eq_range[0] == 0.0)
      {
         (*runfit).eq_range[0] = (*runfit).eq[0] / 2.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).eq_range[0]);
      fp_eqconst1->le_range->setText(str);
      fp_eqconst1->le_range->setEnabled(true);
      fp_eqconst1->cb_float->setChecked(true);
   }
   else
   {
      (*runfit).eq_fit[0] = false;
      (*runfit).eq_range[0] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_eqconst1->le_range->setText(str);
      fp_eqconst1->le_range->setEnabled(false);
      fp_eqconst1->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_eqconst1_value(const QString &str)
{
   (*runfit).eq[0] = str.toFloat();
}

void US_EqModelControl::get_eqconst1_range(const QString &str)
{
   (*runfit).eq_range[0] = str.toFloat();
}

void US_EqModelControl::eqconst2_constrained()
{
   if (fp_eqconst1->cb_constrained->isChecked())
   {
      (*runfit).eq_bound[1] = true;
   }
   else
   {
      (*runfit).eq_bound[1] = false;
   }
}

void US_EqModelControl::eqconst2_float()
{
   if (fp_eqconst2->cb_float->isChecked())
   {
      eqconst2_float(true);
   }
   else
   {
      eqconst2_float(false);
   }
}

void US_EqModelControl::eqconst2_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).eq_fit[1] = true;
      if ((*runfit).eq_range[1] == 0.0)
      {
         (*runfit).eq_range[1] = (*runfit).eq[1] / 2.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).eq_range[1]);
      fp_eqconst2->le_range->setText(str);
      fp_eqconst2->le_range->setEnabled(true);
      fp_eqconst2->cb_float->setChecked(true);
   }
   else
   {
      (*runfit).eq_fit[1] = false;
      (*runfit).eq_range[1] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_eqconst2->le_range->setText(str);
      fp_eqconst2->le_range->setEnabled(false);
      fp_eqconst2->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_eqconst2_value(const QString &str)
{
   (*runfit).eq[1] = str.toFloat();
}

void US_EqModelControl::get_eqconst2_range(const QString &str)
{
   (*runfit).eq_range[1] = str.toFloat();
}

void US_EqModelControl::eqconst3_constrained()
{
   if (fp_eqconst1->cb_constrained->isChecked())
   {
      (*runfit).eq_bound[2] = true;
   }
   else
   {
      (*runfit).eq_bound[2] = false;
   }
}

void US_EqModelControl::eqconst3_float()
{
   if (fp_eqconst3->cb_float->isChecked())
   {
      eqconst3_float(true);
   }
   else
   {
      eqconst3_float(false);
   }
}

void US_EqModelControl::eqconst3_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).eq_fit[2] = true;
      if ((*runfit).eq_range[2] == 0.0)
      {
         (*runfit).eq_range[2] = (*runfit).eq[2] / 2.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).eq_range[2]);
      fp_eqconst3->le_range->setText(str);
      fp_eqconst3->le_range->setEnabled(true);
      fp_eqconst3->cb_float->setChecked(true);
   }
   else
   {
      (*runfit).eq_fit[2] = false;
      (*runfit).eq_range[2] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_eqconst3->le_range->setText(str);
      fp_eqconst3->le_range->setEnabled(false);
      fp_eqconst3->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_eqconst3_value(const QString &str)
{
   (*runfit).eq[2] = str.toFloat();
}

void US_EqModelControl::get_eqconst3_range(const QString &str)
{
   (*runfit).eq_range[2] = str.toFloat();
}

void US_EqModelControl::eqconst4_constrained()
{
   if (fp_eqconst1->cb_constrained->isChecked())
   {
      (*runfit).eq_bound[3] = true;
   }
   else
   {
      (*runfit).eq_bound[3] = false;
   }
}

void US_EqModelControl::eqconst4_float()
{
   if (fp_eqconst4->cb_float->isChecked())
   {
      eqconst4_float(true);
   }
   else
   {
      eqconst4_float(false);
   }
}

void US_EqModelControl::eqconst4_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*runfit).eq_fit[3] = true;
      if ((*runfit).eq_range[3] == 0.0)
      {
         (*runfit).eq_range[3] = (*runfit).eq[3] / 2.0;      // 20% of total
      }
      str.sprintf("%1.5e", (*runfit).eq_range[3]);
      fp_eqconst4->le_range->setText(str);
      fp_eqconst4->le_range->setEnabled(true);
      fp_eqconst4->cb_float->setChecked(true);
   }
   else
   {
      (*runfit).eq_fit[3] = false;
      (*runfit).eq_range[3] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_eqconst4->le_range->setText(str);
      fp_eqconst4->le_range->setEnabled(false);
      fp_eqconst4->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_eqconst4_value(const QString &str)
{
   (*runfit).eq[3] = str.toFloat();
}

void US_EqModelControl::get_eqconst4_range(const QString &str)
{
   (*runfit).eq_range[3] = str.toFloat();
}

void US_EqModelControl::baseline_constrained()
{
   if (fp_baseline->cb_constrained->isChecked())
   {
      (*scanfit_vector)[*selected_scan].baseline_bound = true;
   }
   else
   {
      (*scanfit_vector)[*selected_scan].baseline_bound = false;
   }
}

void US_EqModelControl::baseline_float()
{
   if (fp_baseline->cb_float->isChecked())
   {
      baseline_float(true);
   }
   else
   {
      baseline_float(false);
   }
}

void US_EqModelControl::baseline_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*scanfit_vector)[*selected_scan].baseline_fit = true;
      if((*scanfit_vector)[*selected_scan].baseline_range == 0.0)
      {
         (*scanfit_vector)[*selected_scan].baseline_range = (float) 0.05;
      }
      str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].baseline_range);
      fp_baseline->le_range->setText(str);
      fp_baseline->le_range->setEnabled(true);
      fp_baseline->cb_float->setChecked(true);
   }
   else
   {
      (*scanfit_vector)[*selected_scan].baseline_fit = false;   // we can't float both
      (*scanfit_vector)[*selected_scan].baseline_range = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_baseline->le_range->setText(str);
      fp_baseline->le_range->setEnabled(false);
      fp_baseline->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_baseline_value(const QString &str)
{
   (*scanfit_vector)[*selected_scan].baseline = str.toFloat();
}

void US_EqModelControl::get_baseline_range(const QString &str)
{
   (*scanfit_vector)[*selected_scan].baseline_range = str.toFloat();
}

void US_EqModelControl::amplitude_constrained()
{
   if (fp_amplitude->cb_constrained->isChecked())
   {
      (*scanfit_vector)[*selected_scan].amplitude_bound[current_component] = true;
   }
   else
   {
      (*scanfit_vector)[*selected_scan].amplitude_bound[current_component] = false;
   }
}

void US_EqModelControl::amplitude_float()
{
   if (fp_amplitude->cb_float->isChecked())
   {
      amplitude_float(true);
   }
   else
   {
      amplitude_float(false);
   }
}

void US_EqModelControl::amplitude_float(bool choice)
{
   QString str;
   if (choice)
   {
      (*scanfit_vector)[*selected_scan].amplitude_fit[current_component] = true;
      if ((*scanfit_vector)[*selected_scan].amplitude_range[current_component] == 0.0)
      {
         (*scanfit_vector)[*selected_scan].amplitude_range[current_component] =
            (*scanfit_vector)[*selected_scan].amplitude[current_component] / 5.0;
      }
      str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].amplitude_range[current_component]);
      fp_amplitude->le_range->setText(str);
      fp_amplitude->le_range->setEnabled(true);
      fp_amplitude->cb_float->setChecked(true);
   }
   else
   {
      (*scanfit_vector)[*selected_scan].amplitude_fit[current_component] = false;   // we can't float both
      (*scanfit_vector)[*selected_scan].amplitude_range[current_component] = 0.0;
      str.sprintf("%1.5e", 0.0);
      fp_amplitude->le_range->setText(str);
      fp_amplitude->le_range->setEnabled(false);
      fp_amplitude->cb_float->setChecked(false);
   }
}

void US_EqModelControl::get_amplitude_value(const QString &str)
{
   //cout << (*scanfit_vector).size() << ", scan: " << *selected_scan << ", string: " << str << ", comp: " << current_component << endl;
   if (!str.isEmpty())
      (*scanfit_vector)[*selected_scan].amplitude[current_component] = str.toFloat();
}

void US_EqModelControl::get_amplitude_range(const QString &str)
{
   (*scanfit_vector)[*selected_scan].amplitude_range[current_component] = str.toFloat();
}

void US_EqModelControl::setup_model()
{
   lbl_banner1->setText(modelString[model]);
   cnt_component1->setRange(1, (*runfit).components, 1);
   cnt_component1->setValue(1);
   cnt_component2->setRange(1, (*runfit).components, 1);
   cnt_component2->setValue(1);
   switch ((*runfit).association_constants)
   {
   case 0:
      {
         fp_eqconst1->setEnabled(false);
         fp_eqconst2->setEnabled(false);
         fp_eqconst3->setEnabled(false);
         fp_eqconst4->setEnabled(false);
         pb_eqconst1->setEnabled(false);
         pb_eqconst2->setEnabled(false);
         pb_eqconst3->setEnabled(false);
         pb_eqconst4->setEnabled(false);
         break;
      }
   case 1:
      {
         fp_eqconst1->setEnabled(true);
         fp_eqconst2->setEnabled(false);
         fp_eqconst3->setEnabled(false);
         fp_eqconst4->setEnabled(false);
         pb_eqconst1->setEnabled(true);
         pb_eqconst2->setEnabled(false);
         pb_eqconst3->setEnabled(false);
         pb_eqconst4->setEnabled(false);
         break;
      }
   case 2:
      {
         fp_eqconst1->setEnabled(true);
         fp_eqconst2->setEnabled(true);
         fp_eqconst3->setEnabled(false);
         fp_eqconst4->setEnabled(false);
         pb_eqconst1->setEnabled(true);
         pb_eqconst2->setEnabled(true);
         pb_eqconst3->setEnabled(false);
         pb_eqconst4->setEnabled(false);
         break;
      }
   case 3:
      {
         fp_eqconst1->setEnabled(true);
         fp_eqconst2->setEnabled(true);
         fp_eqconst3->setEnabled(true);
         fp_eqconst4->setEnabled(false);
         pb_eqconst1->setEnabled(true);
         pb_eqconst2->setEnabled(true);
         pb_eqconst3->setEnabled(true);
         pb_eqconst4->setEnabled(false);
         break;
      }
   case 4:
      {
         fp_eqconst1->setEnabled(true);
         fp_eqconst2->setEnabled(true);
         fp_eqconst3->setEnabled(true);
         fp_eqconst4->setEnabled(true);
         pb_eqconst1->setEnabled(true);
         pb_eqconst2->setEnabled(true);
         pb_eqconst3->setEnabled(true);
         pb_eqconst4->setEnabled(true);
         break;
      }
   }
}

void US_EqModelControl::read_vbar()
{
   float vbar = (*runfit).vbar20[current_component];
   vbar_dlg = new US_Vbar_DB(20.0, &vbar, &(*runfit).vbar20[current_component],true, false, (*run_information)[0].investigator);
   vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   vbar_dlg->setCaption(tr("V-bar Calculation"));
   vbar_dlg->show();
   connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
}

void US_EqModelControl::update_vbar_lbl(float, float val20)
{
   (*runfit).vbar20[current_component] = val20;
   QString str;
   str.sprintf("%6.4f", val20);
   fp_vbar->le_value->setText(str);
   (*runfit).vbar20_range[current_component] = (*runfit).vbar20[current_component] / 5.0;      // 20% of total
   if ((*runfit).vbar20_fit[current_component])
   {
      str.sprintf("%1.5e", (*runfit).vbar20_range[current_component]);
      fp_vbar->le_range->setText(str);
   }
   else
   {
      str.sprintf("%1.5e", 0.0);
      fp_vbar->le_range->setText(str);
   }
}

void US_EqModelControl::update_scan_number(int scan)
{
   cnt_active_scan->setValue((double) (scan + 1));
   update_scan_info(scan);
}

void US_EqModelControl::update_scan_info(int scan)
{
   QString str;
   str.sprintf(tr("Local Scan Parameters for Scan %d"), scan + 1);
   lbl_banner2->setText(str);
   lbl_description2->setText(" " + (*scanfit_vector)[*selected_scan].description);
   lbl_run_id2->setText(" " + (*scanfit_vector)[*selected_scan].run_id);
   str.sprintf(" %4.2f ºC", (*scanfit_vector)[*selected_scan].temperature);
   lbl_temperature2->setText(str);
   str.sprintf(" %u rpm", (*scanfit_vector)[*selected_scan].rpm);
   lbl_speed2->setText(str);
   str.sprintf(" %d nm", (*scanfit_vector)[*selected_scan].wavelength);
   lbl_wavelength2->setText(str);
   update_include();   // disable the fields if the scan isn't included, enable, if it is.
   if ((*scanfit_vector)[*selected_scan].FitScanFlag)
   {
      fp_baseline->updateValue((*scanfit_vector)[*selected_scan].baseline);
      fp_baseline->updateRange((*scanfit_vector)[*selected_scan].baseline_range);
      fp_amplitude->updateValue((*scanfit_vector)[*selected_scan].amplitude[current_component]);
      fp_amplitude->updateRange((*scanfit_vector)[*selected_scan].amplitude_range[current_component]);
      cb_include->setChecked(true);
      baseline_float((*scanfit_vector)[*selected_scan].baseline_fit);
      amplitude_float((*scanfit_vector)[*selected_scan].amplitude_fit[current_component]);
   }
   else
   {
      cb_include->setChecked(false);
   }
   str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].extinction[current_component]);
   le_extinction->setText(str);
   str.sprintf("%2.2f", (*scanfit_vector)[*selected_scan].pathlength);
   le_pathlength->setText(str);
   str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].density);
   le_density->setText(str);
   update_sigma(str); // sigma label needs to be updated in case the buffer is different for some scans
}

void US_EqModelControl::update_eqconst1()
{
   QString str;
   eqconst1++;
   float eq = (*runfit).eq[0];   // we need to save the original equilibrium constant in a temporary
   if (eqconst1 == 4)            // since it will be reassigned in the le_value->setText() ca
   {
      eqconst1 = 1;
   }
   switch(eqconst1)
   {
   case 1:
      {
         pb_eqconst1->setText(tr("ln(Assoc. Const. 1)"));
         if((*runfit).stoich1 == 2)
         {
            fp_eqconst1->setUnit("1/C");
         }
         else
         {
            fp_eqconst1->setUnit("1/C^" + str.sprintf("%1.1f", (*runfit).stoich1-1));
         }
         fp_eqconst1->le_value->setText(str.sprintf("%2.4e", eq));
         break;
      }
   case 2:
      {
         pb_eqconst1->setText(tr("Assoc. Const. 1"));
         if((*runfit).stoich1 == 2)
         {
            fp_eqconst1->setUnit("1/mol");
         }
         else
         {
            fp_eqconst1->setUnit("1/mol^" + str.sprintf("%1.1f", (*runfit).stoich1-1));
         }
         fp_eqconst1->le_value->setText(str.sprintf("%2.4e", exp(eq)));
         break;
      }
   case 3:
      {
         pb_eqconst1->setText(tr("Dissoc. Const. 1"));
         if((*runfit).stoich1 == 2)
         {
            fp_eqconst1->setUnit("mol");
         }
         else
         {
            fp_eqconst1->setUnit("mol^" + str.sprintf("%1.1f", (*runfit).stoich1-1));
         }
         fp_eqconst1->le_value->setText(str.sprintf("%2.4e", 1/exp(eq)));
         break;
      }
   }
   (*runfit).eq[0] = eq;
}

void US_EqModelControl::update_eqconst2()
{
   QString str;
   eqconst2++;
   float eq = (*runfit).eq[1];
   if (eqconst2 == 4)
   {
      eqconst2 = 1;
   }
   switch(eqconst2)
   {
   case 1:
      {
         pb_eqconst2->setText(tr("ln(Assoc. Const. 2)"));
         if((*runfit).stoich2 == 2)
         {
            fp_eqconst2->setUnit("1/C");
         }
         else
         {
            fp_eqconst2->setUnit("1/C^" + str.sprintf("%1.1f", (*runfit).stoich2-1));
         }
         fp_eqconst2->le_value->setText(str.sprintf("%2.4e", eq));
         break;
      }
   case 2:
      {
         pb_eqconst2->setText(tr("Assoc. Const. 2"));
         if((*runfit).stoich2 == 2)
         {
            fp_eqconst2->setUnit("1/mol");
         }
         else
         {
            fp_eqconst2->setUnit("1/mol^" + str.sprintf("%1.1f", (*runfit).stoich2-1));
         }
         fp_eqconst2->le_value->setText(str.sprintf("%2.4e", exp(eq)));
         break;
      }
   case 3:
      {
         pb_eqconst2->setText(tr("Dissoc. Const. 2"));
         if((*runfit).stoich2 == 2)
         {
            fp_eqconst2->setUnit("mol");
         }
         else
         {
            fp_eqconst2->setUnit("mol^" + str.sprintf("%1.1f", (*runfit).stoich2-1));
         }
         fp_eqconst2->le_value->setText(str.sprintf("%2.4e", 1/exp(eq)));
         break;
      }
   }
   (*runfit).eq[1] = eq;
}

void US_EqModelControl::update_eqconst3()
{
   QString str;
   eqconst3++;
   float eq = (*runfit).eq[2];
   if (eqconst3 == 4)
   {
      eqconst3 = 1;
   }
   switch(eqconst3)
   {
   case 1:
      {
         pb_eqconst3->setText(tr("ln(Assoc. Const. 3)"));
         if((*runfit).stoich3 == 2)
         {
            fp_eqconst3->setUnit("1/C");
         }
         else
         {
            fp_eqconst3->setUnit("1/C^" + str.sprintf("%1.1f", (*runfit).stoich3-1));
         }
         fp_eqconst3->le_value->setText(str.sprintf("%2.4e", eq));
         break;
      }
   case 2:
      {
         pb_eqconst3->setText(tr("Assoc. Const. 3"));
         if((*runfit).stoich3 == 2)
         {
            fp_eqconst3->setUnit("1/mol");
         }
         else
         {
            fp_eqconst3->setUnit("1/mol^" + str.sprintf("%1.1f", (*runfit).stoich3-1));
         }
         fp_eqconst3->le_value->setText(str.sprintf("%2.4e", exp(eq)));
         break;
      }
   case 3:
      {
         pb_eqconst3->setText(tr("Dissoc. Const. 3"));
         if((*runfit).stoich3 == 2)
         {
            fp_eqconst3->setUnit("mol");
         }
         else
         {
            fp_eqconst3->setUnit("mol^" + str.sprintf("%1.1f", (*runfit).stoich3-1));
         }
         fp_eqconst3->le_value->setText(str.sprintf("%2.4e", 1/exp(eq)));
         break;
      }
   }
   (*runfit).eq[2] = eq;
}

void US_EqModelControl::update_eqconst4()
{
   QString str;
   eqconst4++;
   float eq = (*runfit).eq[3];
   if (eqconst4 == 4)
   {
      eqconst4 = 1;
   }
   switch(eqconst4)
   {
   case 1:
      {
         pb_eqconst4->setText(tr("ln(Assoc. Const. 4)"));
         if((*runfit).stoich4 == 2)
         {
            fp_eqconst4->setUnit("1/C");
         }
         else
         {
            fp_eqconst4->setUnit("1/C^" + str.sprintf("%1.1f", (*runfit).stoich4-1));
         }
         fp_eqconst4->le_value->setText(str.sprintf("%2.4e", eq));
         break;
      }
   case 2:
      {
         pb_eqconst4->setText(tr("Assoc. Const. 4"));
         if((*runfit).stoich4 == 2)
         {
            fp_eqconst4->setUnit("1/mol");
         }
         else
         {
            fp_eqconst4->setUnit("1/mol^" + str.sprintf("%1.1f", (*runfit).stoich4-1));
         }
         fp_eqconst4->le_value->setText(str.sprintf("%2.4e", exp(eq)));
         break;
      }
   case 3:
      {
         pb_eqconst4->setText(tr("Dissoc. Const. 4"));
         if((*runfit).stoich4 == 2)
         {
            fp_eqconst4->setUnit("mol");
         }
         else
         {
            fp_eqconst4->setUnit("mol^" + str.sprintf("%1.1f", (*runfit).stoich4-1));
         }
         fp_eqconst4->le_value->setText(str.sprintf("%2.4e", 1/exp(eq)));
         break;
      }
   }
   (*runfit).eq[3] = eq;
}

void US_EqModelControl::update_density1()
{
   QString dummy = "";
   buffer_dlg = new US_Buffer_DB(false, -1);
   buffer_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
   buffer_dlg->setCaption(tr("Buffer Calculation"));
   buffer_dlg->show();
   connect(buffer_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_density_label(float, float)));
   update_sigma(dummy);
}

void US_EqModelControl::update_density_label(float density, float)
{
   QString str;
   str.sprintf("%6.4f", density);
   le_density->setText(str);
}

float US_EqModelControl::calc_density_tb(float density, float t)
{
   // density of parameter list is density of buffer at 20C
   // t of parameter list is temperature in Celsius

   float density_wt, density_wt_20, density_tb, rt=20.0;
   density_wt    = 1.000028e-3 * ((999.83952 + 16.945176 * t) / (1.0 + 16.879850e-3 * t))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) t, (double) 2.0) + 46.170461e-6 * pow((double) t, (double) 3.0))
                       / (1.0 + 16.87985e-3 * t))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) t, (double) 4.0) - 280.54253e-12 * pow((double) t, (double) 5.0))
                       / (1.0 + 16.87985e-3 * t));
   density_wt_20    = 1.000028e-3 * ((999.83952 + 16.945176 * rt) / (1.0 + 16.879850e-3 * rt))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) rt, 2.0) + 46.170461e-6 * pow((double) rt, 3.0))
                       / (1.0 + 16.87985e-3 * rt))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) rt, (double) 4.0) - 280.54253e-12 * pow((double) rt, (double) 5.0))
                       / (1.0 + 16.87985e-3 * rt));
   density_tb = density * density_wt / density_wt_20;
   return(density_tb);
}

// update density variable from line edit
void US_EqModelControl::update_density2(const QString &text)
{
   (*scanfit_vector)[*selected_scan].density = text.toFloat();
   QString dummy = "";
   update_sigma(dummy);
}

void US_EqModelControl::update_extinction1()
{
   US_Config *USglobal;
   unsigned int i, j;
   USglobal = new US_Config();

   QMessageBox mb(tr("UltraScan"),
                  tr("Do you want to load an existing extinction profile or generate\n"
                     "a new profile with the global extinction coefficient fitter?\n\n"
                     "When you load an existing scan, all extinction coefficients\n"
                     "for the current component will be replaced with the extinction\n"
                     "profile stored in the loaded dataset. If you generate a new\n"
                     "profile, you will have the choice to apply the profile to the\n"
                     "scans of your run after fitting is completed."),
                  QMessageBox::Information,
                  QMessageBox::Yes,
                  QMessageBox::No,
                  QMessageBox::Cancel | QMessageBox::Escape | QMessageBox::Default);
   mb.setButtonText(QMessageBox::Yes, "Load");
   mb.setButtonText(QMessageBox::No, "Generate");
   switch(mb.exec())
   {
   case QMessageBox::Yes:
      {
         QFile f;
         QString extName, str;
         extName = QFileDialog::getOpenFileName(USglobal->config_list.result_dir, "*.extinction.dat", 0, 0);
         if(extName != NULL || extName.stripWhiteSpace() != "")
         {
            f.setName(extName);
            extinction.X.clear();
            extinction.Y.clear();
            if(f.open(IO_ReadOnly))
            {
               QTextStream ts(&f);
               str = ts.readLine();
               i = 0;
               while (!ts.eof())
               {
                  ts >> str;
                  extinction.X.push_back(str.toFloat());
                  ts >> str;
                  extinction.Y.push_back(str.toFloat());
               }
            }
            if ((*runfit).components > 1)
            {
               i = QInputDialog::getInteger(tr("UltraScan Notice:"),
                                            tr("You have more than one component in the current model definition.\n"
                                               "Currently component ") + str.sprintf("%d", current_component+1) + tr(" is selected.\n\n"
                                                                                                                     "If you want to adjust the extinction coefficient for a different\n"
                                                                                                                     "component, please enter the number of the desired component now:\n"),
                                            current_component+1, 1, (*runfit).components, 1, 0, 0, "");
               cnt_component2->setValue((double) i);
               update_component2((double) i);
            }
            for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
            {
               //               if ((*scanfit_vector)[i].FitScanFlag)
               //               {
               j = 0;
               while ((unsigned int) extinction.X[j] < (*scanfit_vector)[i].wavelength)
               {
                  j++;
               }
               if (j == 0 && (unsigned int) extinction.X[j] > (*scanfit_vector)[i].wavelength)
               {
                  str.sprintf(tr("I couldn't find a wavelength definition matching\n the wavelength of scan %d (%d nm)!\n\n"
                                 "Please adjust this coefficient manually!"), i+1, (unsigned int) (*scanfit_vector)[i].wavelength);
                  QMessageBox::message(tr("UltraScan Warning:"), str);
               }
               else
               {
                  (*scanfit_vector)[i].extinction[current_component] = extinction.Y[j];
               }
               //               }
            }
            update_scan_info(*selected_scan);
            break;
         }
         else
         {
            return;
         }
      }
   case QMessageBox::No:
      {
         US_Extinction *ex_control;
         projectName = (*scanfit_vector)[*selected_scan].run_id;
         ex_control = new US_Extinction(projectName, (*run_information)[0].investigator, 0, "Extinction Control");
         ex_control->show();
         connect(ex_control, SIGNAL(fitUpdated(double *, double *, unsigned int)), this,
                 SLOT(update_extinction3(double *, double *, unsigned int)));
         connect(ex_control, SIGNAL(projectChanged(const QString &)),
                 SLOT(update_projectName(const QString &)));
         // ask user if he wants all wavelengths checked and apply extinction correction to all scans
         connect(ex_control, SIGNAL(extinctionClosed()), SLOT(update_extinction4()));
         break;
      }
   case QMessageBox::Cancel:
      {
         return;
      }
   default:
      {
         return;
      }
   }
}

void US_EqModelControl::update_extinction4()
{
   QString str;
   int i = 0;
   unsigned int j;
   QMessageBox mb(tr("UltraScan"),
                  tr("Do you want to automatically update all scans with\n"
                     "the Extinction Coefficients corresponding to each\n"
                     "scan's wavelength for run ") + projectName + "?\n",
                  QMessageBox::Information,
                  QMessageBox::Yes | QMessageBox::Default,
                  QMessageBox::No,
                  QMessageBox::Cancel | QMessageBox::Escape);
   mb.setButtonText(QMessageBox::Yes, "Yes");
   mb.setButtonText(QMessageBox::No, "No");
   switch(mb.exec())
   {
   case QMessageBox::Yes:
      {
         if ((*runfit).components > 1)
         {
            i = QInputDialog::getInteger(tr("UltraScan Notice:"),
                                         tr("You have more than one component in the current model definition.\n"
                                            "Currently component ") + str.sprintf("%d", current_component+1) + tr(" is selected.\n\n"
                                                                                                                  "If you want to adjust the extinction coefficient for a different\n"
                                                                                                                  "component, please enter the number of the desired component now:\n"),
                                         current_component+1, 1, (*runfit).components, 1, 0, 0, "");
            cnt_component2->setValue((double) i);
            update_component2((double) i);
         }
         for (unsigned int i=0; i<(*scanfit_vector).size(); i++)
         {
            //            if ((*scanfit_vector)[i].FitScanFlag)
            //            {
            j = 0;
            while ((unsigned int) extinction.X[j] < (*scanfit_vector)[i].wavelength)
            {
               j++;
            }
            if (j == 0 && (unsigned int) extinction.X[j] > (*scanfit_vector)[i].wavelength)
            {
               str.sprintf(tr("I couldn't find a wavelength definition matching\n the wavelength of scan %d (%d nm)!\n\n"
                              "Please adjust this coefficient manually!"), i+1, (unsigned int) (*scanfit_vector)[i].wavelength);
               QMessageBox::message(tr("UltraScan Warning:"), str);
            }
            else
            {
               (*scanfit_vector)[i].extinction[current_component] = extinction.Y[j];
               //cout << "Current_component: " << current_component << ", scan: " << i << ", extinction: " << (*scanfit_vector)[i].extinction[current_component] << endl;
            }
            //            }
         }
         update_scan_info(*selected_scan);
         break;
      }
   }
}

void US_EqModelControl::update_projectName(const QString &str)
{
   projectName = str;
}

void US_EqModelControl::update_extinction3(double *x, double *y, unsigned int size)
{
   unsigned int i;
   extinction.X.clear();
   extinction.Y.clear();
   for(i=0; i<size; i++)
   {
      extinction.X.push_back((float) x[i]);
      extinction.Y.push_back((float) y[i]);
      //         cout << extinction.X[i] << ", " << extinction.Y[i] << endl;
   }
}

void US_EqModelControl::update_sigma(const QString &)
{
   float vbar = adjust_vbar20((*runfit).vbar20[current_component], (*scanfit_vector)[*selected_scan].temperature);
   float omega = (*scanfit_vector)[*selected_scan].rpm * M_PI / 30.0;
   float temperature = (*scanfit_vector)[*selected_scan].temperature + K0;
   float density_tb = calc_density_tb((*scanfit_vector)[*selected_scan].density, (*scanfit_vector)[*selected_scan].temperature);
   float sigma = ((1.0 - vbar * density_tb) * pow((double) omega, (double) 2.0) * (*runfit).mw[current_component]) / (2.0 * R * temperature);
   QString str;
   str.sprintf(" %1.5e", sigma);
   lbl_sigma2->setText(str);
}

void US_EqModelControl::update_extinction2(const QString &text)
{
   (*scanfit_vector)[*selected_scan].extinction[current_component] = text.toFloat();
}

void US_EqModelControl::update_pathlength2(const QString &text)
{
   (*scanfit_vector)[*selected_scan].pathlength = text.toFloat();
}

//updates the enabled/disabled settings for selected scans:
void US_EqModelControl::update_include()
{
   if ((*scanfit_vector)[*selected_scan].FitScanFlag)
   {
      (*scanfit_vector)[*selected_scan].FitScanFlag = true;
      fp_baseline->setEnabled(true);
      le_density->setEnabled(true);
      fp_amplitude->setEnabled(true);
      le_extinction->setEnabled(true);
   }
   else
   {
      (*scanfit_vector)[*selected_scan].FitScanFlag = false;
      fp_baseline->setEnabled(false);
      le_density->setEnabled(false);
      fp_amplitude->setEnabled(false);
      le_extinction->setEnabled(false);
   }
}

void US_EqModelControl::change_include()
{
   QString str;
   if ((*scanfit_vector)[*selected_scan].FitScanFlag)
   {
      (*scanfit_vector)[*selected_scan].FitScanFlag = false;
      fp_baseline->setEnabled(false);
      le_density->setEnabled(false);
      fp_amplitude->setEnabled(false);
      le_extinction->setEnabled(false);
   }
   else
   {
      str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].baseline);
      fp_baseline->le_value->setText(str);
      str.sprintf("%1.5e", (*scanfit_vector)[*selected_scan].amplitude[current_component]);
      fp_amplitude->le_value->setText(str);
      (*scanfit_vector)[*selected_scan].FitScanFlag = true;
      fp_baseline->setEnabled(true);
      le_density->setEnabled(true);
      fp_amplitude->setEnabled(true);
      le_extinction->setEnabled(true);
   }
}

void US_EqModelControl::help()
{
   US_Help *online_help; online_help = new US_Help(this);
   online_help->show_help("manual/equil5.html");
}

void US_EqModelControl::quit()
{
   close();
}
