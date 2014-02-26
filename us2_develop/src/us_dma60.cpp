//
// C++ Implementation: us_dma60
//
// Description:
//
//
// Author: Borries Demeler <demeler@biochem.uthscsa.edu>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
/**************************   Class US_DMA60    *******************************/
#include "../include/us_dma60.h"

US_DMA60::US_DMA60(QWidget *parent, const char *name) : QWidget(parent, name)
{
   USglobal = new US_Config();

   description = "Sample";
   current_k = 0;
   dma.clear();
   recording.clear();

   setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

   lbl_banner1 = new QLabel(tr("Calibration Information:"),this);
   lbl_banner1->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner1->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_banner2 = new QLabel(tr("Sample Measurement:"),this);
   lbl_banner2->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner2->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_banner3 = new QLabel(tr("Density Extrapolation:"),this);
   lbl_banner3->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner3->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_banner4 = new QLabel(tr("Double-click to select Constant:"),this);
   lbl_banner4->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner4->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner4->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_banner5 = new QLabel(tr("Double-click to select Measurement:"),this);
   lbl_banner5->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner5->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner5->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_banner6 = new QLabel(tr("Extrapolation Range:"),this);
   lbl_banner6->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
   lbl_banner6->setAlignment(AlignHCenter|AlignVCenter);
   lbl_banner6->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

   lbl_temperature = new QLabel(tr(" Calibration Temperature:"),this);
   lbl_temperature->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_temperature = new QLineEdit("", this);
   le_temperature->setAlignment(AlignLeft|AlignVCenter);
   le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_temperature->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature(const QString &)));

   lbl_pressure = new QLabel(tr(" Air Pressure (inches):"),this);
   lbl_pressure->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_pressure->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_pressure = new QLineEdit("", this);
   le_pressure->setAlignment(AlignLeft|AlignVCenter);
   le_pressure->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_pressure->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_pressure, SIGNAL(textChanged(const QString &)), SLOT(update_pressure(const QString &)));

   lbl_T_air = new QLabel(tr(" T reading, dry air:"),this);
   lbl_T_air->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_T_air->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_T_air = new QLineEdit("", this);
   le_T_air->setAlignment(AlignLeft|AlignVCenter);
   le_T_air->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_T_air->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_T_air, SIGNAL(textChanged(const QString &)), SLOT(update_T_air(const QString &)));

   lbl_T_water = new QLabel(tr(" T reading, ddH2O:"),this);
   lbl_T_water->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_T_water->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_T_water = new QLineEdit("", this);
   le_T_water->setAlignment(AlignLeft|AlignVCenter);
   le_T_water->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_T_water->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_T_water, SIGNAL(textChanged(const QString &)), SLOT(update_T_water(const QString &)));

   lbl_description = new QLabel(tr(" Sample Description:"),this);
   lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_description = new QLineEdit("Sample", this);
   le_description->setAlignment(AlignLeft|AlignVCenter);
   le_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_description->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));

   lbl_units = new QLabel(tr(" Concentration Units:"),this);
   lbl_units->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_units->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_units = new QLineEdit("", this);
   le_units->setAlignment(AlignLeft|AlignVCenter);
   le_units->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_units->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_units, SIGNAL(textChanged(const QString &)), SLOT(update_units(const QString &)));

   lbl_concentration = new QLabel(tr(" Sample Concentration:"),this);
   lbl_concentration->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_concentration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_concentration = new QLineEdit("", this);
   le_concentration->setAlignment(AlignLeft|AlignVCenter);
   le_concentration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_concentration->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_concentration, SIGNAL(textChanged(const QString &)), SLOT(update_concentration(const QString &)));

   lbl_T_sample = new QLabel(tr(" T reading, Sample:"),this);
   lbl_T_sample->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_T_sample->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_T_sample = new QLineEdit("", this);
   le_T_sample->setAlignment(AlignLeft|AlignVCenter);
   le_T_sample->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_T_sample->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_T_sample, SIGNAL(textChanged(const QString &)), SLOT(update_T_sample(const QString &)));

   lbl_select_conc = new QLabel(tr(" Select a Concentration:"),this);
   lbl_select_conc->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_select_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_select_conc = new QLineEdit("", this);
   le_select_conc->setAlignment(AlignLeft|AlignVCenter);
   le_select_conc->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_select_conc->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_select_conc, SIGNAL(textChanged(const QString &)), SLOT(update_select_conc(const QString &)));

   lbl_select_density = new QLabel(tr(" Calculated Density:"),this);
   lbl_select_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_select_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_select_density = new QLineEdit("", this);
   le_select_density->setAlignment(AlignLeft|AlignVCenter);
   le_select_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_select_density->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_select_density->setReadOnly(true);
   connect(le_select_density, SIGNAL(textChanged(const QString &)), SLOT(update_select_density(const QString &)));

   pb_extrapolate_single_value = new QPushButton(tr("Update Density"), this);
   Q_CHECK_PTR(pb_extrapolate_single_value);
   pb_extrapolate_single_value->setAutoDefault(false);
   pb_extrapolate_single_value->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_extrapolate_single_value->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_extrapolate_single_value, SIGNAL(clicked()), SLOT(extrapolate_single_value()));

   lbl_conc_limit_low = new QLabel(tr(" Lower Concentration Limit:"),this);
   lbl_conc_limit_low->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_conc_limit_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_conc_limit_low = new QLineEdit("", this);
   le_conc_limit_low->setAlignment(AlignLeft|AlignVCenter);
   le_conc_limit_low->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_conc_limit_low->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_conc_limit_low, SIGNAL(textChanged(const QString &)), SLOT(update_conc_limit_low(const QString &)));

   lbl_conc_limit_high = new QLabel(tr(" Upper Concentration Limit:"),this);
   lbl_conc_limit_high->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_conc_limit_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_conc_limit_high = new QLineEdit("", this);
   le_conc_limit_high->setAlignment(AlignLeft|AlignVCenter);
   le_conc_limit_high->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_conc_limit_high->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   connect(le_conc_limit_high, SIGNAL(textChanged(const QString &)), SLOT(update_conc_limit_high(const QString &)));

   lbl_steps = new QLabel(tr(" Concentration Steps:"),this);
   lbl_steps->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_steps = new QLineEdit("", this);
   le_steps->setAlignment(AlignLeft|AlignVCenter);
   le_steps->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_steps->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_steps->setReadOnly(true);
   connect(le_steps, SIGNAL(textChanged(const QString &)), SLOT(update_steps(const QString &)));

   pb_calibrate = new QPushButton(tr("Calculate Calibration"), this);
   Q_CHECK_PTR(pb_calibrate);
   pb_calibrate->setAutoDefault(false);
   pb_calibrate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_calibrate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_calibrate, SIGNAL(clicked()), SLOT(calibrate()));

   pb_measurement = new QPushButton(tr("Calculate Density"), this);
   Q_CHECK_PTR(pb_measurement);
   pb_measurement->setAutoDefault(false);
   pb_measurement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_measurement->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_measurement, SIGNAL(clicked()), SLOT(measurement()));

   lbl_k = new QLabel(tr(" Calibration Constant K:"),this);
   lbl_k->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_k = new QLineEdit("", this);
   le_k->setAlignment(AlignLeft|AlignVCenter);
   le_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_k->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_k->setReadOnly(true);

   lbl_sample_density = new QLabel(tr(" Calculated Density:"),this);
   lbl_sample_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
   lbl_sample_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

   le_sample_density = new QLineEdit("", this);
   le_sample_density->setAlignment(AlignLeft|AlignVCenter);
   le_sample_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   le_sample_density->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
   le_sample_density->setReadOnly(true);

   pb_save_k = new QPushButton(tr("Save Calibration"), this);
   Q_CHECK_PTR(pb_save_k);
   pb_save_k->setAutoDefault(false);
   pb_save_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_k->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_k, SIGNAL(clicked()), SLOT(save_k()));

   pb_load_k = new QPushButton(tr("Load Calibrations"), this);
   Q_CHECK_PTR(pb_load_k);
   pb_load_k->setAutoDefault(false);
   pb_load_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_k->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_k, SIGNAL(clicked()), SLOT(load_k()));

   pb_save_measurement = new QPushButton(tr("Save Measurement"), this);
   Q_CHECK_PTR(pb_save_measurement);
   pb_save_measurement->setAutoDefault(false);
   pb_save_measurement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_save_measurement->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_save_measurement, SIGNAL(clicked()), SLOT(save_measurement()));

   pb_load_measurement = new QPushButton(tr("Load Measurement"), this);
   Q_CHECK_PTR(pb_load_measurement);
   pb_load_measurement->setAutoDefault(false);
   pb_load_measurement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_load_measurement->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_load_measurement, SIGNAL(clicked()), SLOT(load_measurement()));

   pb_extrapolate = new QPushButton(tr("Extrapolate"), this);
   Q_CHECK_PTR(pb_extrapolate);
   pb_extrapolate->setAutoDefault(false);
   pb_extrapolate->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_extrapolate->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_extrapolate, SIGNAL(clicked()), SLOT(extrapolate()));

   lb_k = new QListBox(this, "Calibration Records");
   lb_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_k->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(lb_k, SIGNAL(selected(int)), SLOT(select_k(int)));

   lb_measurement = new QListBox(this, "measurements");
   lb_measurement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_measurement->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
   connect(lb_measurement, SIGNAL(selected(int)), SLOT(select_measurement(int)));

   lb_extrapolation = new QListBox(this, "extrapolation");
   lb_extrapolation->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
   lb_extrapolation->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));

   pb_reset_k = new QPushButton(tr("Reset all Constants"), this);
   Q_CHECK_PTR(pb_reset_k);
   pb_reset_k->setAutoDefault(false);
   pb_reset_k->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset_k->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_reset_k, SIGNAL(clicked()), SLOT(reset_k()));

   pb_reset_measurement = new QPushButton(tr("Reset Measurements"), this);
   Q_CHECK_PTR(pb_reset_measurement);
   pb_reset_measurement->setAutoDefault(false);
   pb_reset_measurement->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_reset_measurement->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_reset_measurement, SIGNAL(clicked()), SLOT(reset_measurement()));

   pb_help = new QPushButton(tr("Help"), this);
   Q_CHECK_PTR(pb_help);
   pb_help->setAutoDefault(false);
   pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_help, SIGNAL(clicked()), SLOT(help()));

   pb_close = new QPushButton(tr("Close"), this);
   Q_CHECK_PTR(pb_close);
   pb_close->setAutoDefault(false);
   pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
   pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
   connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

   int j=0, rows=11, columns=6;

   QGridLayout * grid1 = new QGridLayout(this, rows, columns, 2);
   for (int i=1; i<rows; i++)
   {
      grid1->setRowSpacing(i, 26);
   }
   grid1->setRowSpacing(0, 30); // banner
   grid1->setRowSpacing(8, 30); // banner
   grid1->setRowSpacing(9, 90); // banner

   grid1->addMultiCellWidget(lbl_banner1, j, j, 0, 1);
   grid1->addMultiCellWidget(lbl_banner2, j, j, 2, 3);
   grid1->addMultiCellWidget(lbl_banner3, j, j, 4, 5);
   j++;
   grid1->addWidget(lbl_temperature, j, 0);
   grid1->addWidget(le_temperature, j, 1);
   grid1->addWidget(lbl_description, j, 2);
   grid1->addWidget(le_description, j, 3);
   grid1->addWidget(lbl_select_conc, j, 4);
   grid1->addWidget(le_select_conc, j, 5);
   j++;
   grid1->addWidget(lbl_pressure, j, 0);
   grid1->addWidget(le_pressure, j, 1);
   grid1->addWidget(lbl_units, j, 2);
   grid1->addWidget(le_units, j, 3);
   grid1->addWidget(lbl_select_density, j, 4);
   grid1->addWidget(le_select_density, j, 5);
   j++;
   grid1->addWidget(lbl_T_air, j, 0);
   grid1->addWidget(le_T_air, j, 1);
   grid1->addWidget(lbl_concentration, j, 2);
   grid1->addWidget(le_concentration, j, 3);
   grid1->addMultiCellWidget(pb_extrapolate_single_value, j, j, 4, 5);
   j++;
   grid1->addWidget(lbl_T_water, j, 0);
   grid1->addWidget(le_T_water, j, 1);
   grid1->addWidget(lbl_T_sample, j, 2);
   grid1->addWidget(le_T_sample, j, 3);
   grid1->addWidget(lbl_conc_limit_low, j, 4);
   grid1->addWidget(le_conc_limit_low, j, 5);
   j++;
   grid1->addMultiCellWidget(pb_calibrate, j, j, 0, 1);
   grid1->addMultiCellWidget(pb_measurement, j, j, 2, 3);
   grid1->addWidget(lbl_conc_limit_high, j, 4);
   grid1->addWidget(le_conc_limit_high, j, 5);
   j++;
   grid1->addWidget(lbl_k, j, 0);
   grid1->addWidget(le_k, j, 1);
   grid1->addWidget(lbl_sample_density, j, 2);
   grid1->addWidget(le_sample_density, j, 3);
   grid1->addWidget(lbl_steps, j, 4);
   grid1->addWidget(le_steps, j, 5);
   j++;
   grid1->addWidget(pb_save_k, j, 0);
   grid1->addWidget(pb_load_k, j, 1);
   grid1->addWidget(pb_save_measurement, j, 2);
   grid1->addWidget(pb_load_measurement, j, 3);
   grid1->addMultiCellWidget(pb_extrapolate, j, j, 4, 5);
   j++;
   grid1->addMultiCellWidget(lbl_banner4, j, j, 0, 1);
   grid1->addMultiCellWidget(lbl_banner5, j, j, 2, 3);
   grid1->addMultiCellWidget(lbl_banner6, j, j, 4, 5);
   j++;
   grid1->addMultiCellWidget(lb_k, j, j, 0, 1);
   grid1->addMultiCellWidget(lb_measurement, j, j, 2, 3);
   grid1->addMultiCellWidget(lb_extrapolation, j, j, 4, 5);
   j++;
   grid1->addMultiCellWidget(pb_reset_k, j, j, 0, 1);
   grid1->addMultiCellWidget(pb_reset_measurement, j, j, 2, 3);
   grid1->addWidget(pb_help, j, 4);
   grid1->addWidget(pb_close, j, 5);

   qApp->processEvents();

   QRect r = grid1->cellGeometry(0, 0);

   global_Xpos += 30;
   global_Ypos += 30;

   this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

US_DMA60::~US_DMA60()
{
   global_Xpos -= 30;
   global_Ypos -= 30;
}

void US_DMA60::update_T_air(const QString &newText)
{
   T_air = newText.toDouble();
}

void US_DMA60::update_T_water(const QString &newText)
{
   T_water = newText.toDouble();
}

void US_DMA60::update_T_sample(const QString &newText)
{
   T_sample = newText.toDouble();
}

void US_DMA60::update_pressure(const QString &newText)
{
   pressure = newText.toDouble();
}

void US_DMA60::update_temperature(const QString &newText)
{
   temperature = newText.toDouble();
}

void US_DMA60::update_conc_limit_low(const QString &newText)
{
   conc_limit_low = newText.toFloat();
}

void US_DMA60::update_conc_limit_high(const QString &newText)
{
   conc_limit_high = newText.toFloat();
}

void US_DMA60::update_description(const QString &newText)
{
   description = newText;
}

void US_DMA60::update_units(const QString &newText)
{
   units = newText;
}

void US_DMA60::update_concentration(const QString &newText)
{
   concentration = newText.toDouble();
}

void US_DMA60::update_select_conc(const QString &newText)
{
   select_conc = newText.toFloat();
}

void US_DMA60::update_select_density(const QString &newText)
{
   select_density = newText.toFloat();
}

void US_DMA60::update_steps(const QString &newText)
{
   steps = newText.toUInt();
}

void US_DMA60::calibrate()
{
   if (T_air <= 0.0 || T_water <= 0.0 || pressure  <= 0.0)
   {
      return;
   }
   struct dma_constant temp_dma;
   QString str;
   QDateTime dt;
   dt = QDateTime::currentDateTime();
   //   temp_dma.dateTime = dt.toString(Qt::ISODate);
   temp_dma.dateTime = dt.toString("dd/MM/yyyy hh:mm:ss");
   temp_dma.dateTime = temp_dma.dateTime.stripWhiteSpace();
   double p = pressure * 25.4, density_air, density_water;
   density_air = 8.61966 - 0.565108 * temperature + 0.0148931 * temperature * temperature
      + 1.68258 * p + 9.77517e-06 * p * p - 0.00545161 * p * temperature;
   density_air *= 1e-6;
   density_water = 1.000028e-3 * ((999.83952 + 16.945176 * temperature) / (1 + 16.879850e-3 * temperature))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) temperature, (double) 2.0) + 46.170461e-6 * pow((double) temperature, (double) 3.0)) / (1.0 + 16.87985e-3 * temperature))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) temperature, (double) 4.0) - 280.54253e-12 * pow((double) temperature, (double) 5.0)) / (1.0 + 16.87985e-3 * temperature));
   constant = (density_water - density_air)/(pow((double) T_water, (double) 2.0) - pow((double) T_air, (double) 2.0));
   temp_dma.k = constant;
   temp_dma.T_water = T_water;
   temp_dma.T_air = T_air;
   temp_dma.temperature = temperature;
   temp_dma.pressure = pressure;
   dma.push_back(temp_dma);
   str.sprintf("%8.6e", constant);
   le_k->setText(str);
   str.sprintf("%8.6e (%6.3f C, %6.3f\"", constant, temperature, pressure);
   lb_k->insertItem(str + ", " + temp_dma.dateTime + ")");
}

void US_DMA60::measurement()
{
   QString str;
   struct measurement temp_recording;
   QDateTime dt;
   dt = QDateTime::currentDateTime();
   //   temp_dma.dateTime = dt.toString(Qt::ISODate);
   temp_recording.dateTime = dt.toString("dd/MM/yyyy hh:mm:ss");
   temp_recording.dateTime = temp_recording.dateTime.stripWhiteSpace();
   temp_recording.temperature = dma[current_k].temperature;
   temp_recording.k = dma[current_k].k;
   temp_recording.pressure = dma[current_k].pressure;
   temp_recording.T_water = dma[current_k].T_water;
   temp_recording.T_air = dma[current_k].T_air;
   temp_recording.T_sample = T_sample;
   temp_recording.units = units;
   temp_recording.description = description;
   temp_recording.concentration = concentration;
   temperature = dma[current_k].temperature;
   float density_w = 1.000028e-3 * ((999.83952 + 16.945176 * temperature) / (1 + 16.879850e-3 * temperature))
      - 1.000028e-3 * ((7.9870401e-3 * pow((double) temperature, (double) 2.0) + 46.170461e-6 * pow((double) temperature, (double) 3.0)) / (1.0 + 16.87985e-3 * temperature))
      + 1.000028e-3 * ((105.56302e-9 * pow((double) temperature, (double) 4.0) - 280.54253e-12 * pow((double) temperature, (double) 5.0)) / (1.0 + 16.87985e-3 * temperature));
   temp_recording.density = temp_recording.k * (pow(temp_recording.T_sample, 2.0) - pow(temp_recording.T_water, 2.0)) + density_w;
   recording.push_back(temp_recording);
   str.sprintf("%8.6e", temp_recording.density);
   le_sample_density->setText(str);
   str.sprintf("%8.6e (%6.3f ", temp_recording.density, temp_recording.concentration);
   lb_measurement->insertItem(str + temp_recording.units + " " + temp_recording.description + ")");
}

void US_DMA60::extrapolate_single_value()
{
}

void US_DMA60::extrapolate()
{
}

void US_DMA60::save_k()
{
   QString filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.dma_cal", 0);
   if (!filename.isEmpty())
   {
      if (filename.right(8) != ".dma_cal")
      {
         filename.append(".dma_cal");
      }
      switch(QMessageBox::information(this, tr("UltraScan - DMA 60 Density Measurements:"),
                                      tr("Click 'OK' to save array with Instrument Constants to :\n"
                                         + filename),
                                      tr("OK"), tr("CANCEL"),   0,1))
      {
      case 0:
         {
            QFile f(filename);
            if (f.open(IO_WriteOnly | IO_Translate))
            {
               QTextStream ts (&f);
               ts.precision(16);
               for (unsigned int i=0; i<dma.size(); i++)
               {
                  ts << dma[i].k << " " << dma[i].temperature << " " << dma[i].pressure << " "
                     << dma[i].T_water << " " << dma[i].T_air << " " << dma[i].dateTime << endl;
               }
               f.close();
            }
            break;
         }
      case 1:
         {
            break;
         }
      }
   }
}

void US_DMA60::save_measurement()
{
   QString filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.dma", 0);
   if (!filename.isEmpty())
   {
      if (filename.right(4) != ".dma")
      {
         filename.append(".dma");
      }
      switch(QMessageBox::information(this, tr("UltraScan - DMA 60 Density Measurements:"),
                                      tr("Click 'OK' to save measurement data to :\n"
                                         + filename),
                                      tr("OK"), tr("CANCEL"),   0,1))
      {
      case 0:
         {
            QFile f(filename);
            if (f.open(IO_WriteOnly | IO_Translate))
            {
               QTextStream ts (&f);
               ts.precision(16);
               for (unsigned int i=0; i<recording.size(); i++)
               {
                  ts << recording[i].density << " "
                     << recording[i].temperature << " "
                     << recording[i].pressure << " "
                     << recording[i].T_water << " "
                     << recording[i].T_air << " "
                     << recording[i].T_sample << " "
                     << recording[i].k << " "
                     << recording[i].concentration << " "
                     << recording[i].units << " "
                     << recording[i].description << " "
                     << recording[i].dateTime << endl;
               }
               f.close();
            }
            break;
         }
      case 1:
         {
            break;
         }
      }
   }
}

void US_DMA60::load_k()
{
   bool flag;
   QString str, filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.dma_cal", 0);
   struct dma_constant temp_dma;
   if (!filename.isEmpty())
   {
      QFile fr(filename);
      fr.open(IO_ReadOnly);
      QTextStream ts (&fr);
      while (!fr.atEnd())
      {
         ts >> temp_dma.k;
         ts >> temp_dma.temperature;
         ts >> temp_dma.pressure;
         ts >> temp_dma.T_water;
         ts >> temp_dma.T_air;
         temp_dma.dateTime = ts.readLine();
         flag = false;
         for (unsigned int i=0; i<dma.size(); i++)
         {
            if (temp_dma.dateTime.stripWhiteSpace() == dma[i].dateTime.stripWhiteSpace())
               flag = true;
         }
         if (!flag) //only add if the entry doesn't already exist
         {
            dma.push_back(temp_dma);
            str.sprintf("%8.6e (%6.3f C, %6.3f\"", temp_dma.k, temp_dma.temperature, temp_dma.pressure);
            lb_k->insertItem(str + ", " + temp_dma.dateTime.stripWhiteSpace() + ")");
         }
      }
      fr.close();
   }
}

void US_DMA60::load_measurement()
{
   bool flag;
   QString str, filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.dma", 0);
   struct measurement temp_recording;
   if (!filename.isEmpty())
   {
      QFile fr(filename);
      fr.open(IO_ReadOnly);
      QTextStream ts (&fr);
      while (!fr.atEnd())
      {
         ts >> temp_recording.density;
         ts >> temp_recording.temperature;
         ts >> temp_recording.pressure;
         ts >> temp_recording.T_water;
         ts >> temp_recording.T_air;
         ts >> temp_recording.T_sample;
         ts >> temp_recording.k;
         ts >> temp_recording.concentration;
         ts >> temp_recording.units;
         ts >> temp_recording.description;
         temp_recording.dateTime = ts.readLine();
         flag = false;
         for (unsigned int i=0; i<recording.size(); i++)
         {
            if (temp_recording.dateTime.stripWhiteSpace() == recording[i].dateTime.stripWhiteSpace())
               flag = true;
         }
         if (!flag) //only add if the entry doesn't already exist
         {
            recording.push_back(temp_recording);
            str.sprintf("%8.6e (%6.3f ", temp_recording.density, temp_recording.concentration);
            lb_measurement->insertItem(str + temp_recording.units + " " + temp_recording.description + ")");
         }
      }
      fr.close();
   }
}

void US_DMA60::reset_k()
{
   lb_k->clear();
   le_k->setText("");
   dma.clear();
}

void US_DMA60::select_k(int item)
{
   QString str;
   current_k = item;
   str.sprintf("%8.6e", dma[item].k);
   le_k->setText(str);
   str.sprintf("%5.3f", dma[item].temperature);
   le_temperature->setText(str);
   str.sprintf("%5.3f", dma[item].pressure);
   le_pressure->setText(str);
   str.sprintf("%15.6f", dma[item].T_air);
   le_T_air->setText(str);
   str.sprintf("%15.6f", dma[item].T_water);
   le_T_water->setText(str);
}

void US_DMA60::reset_measurement()
{
   lb_measurement->clear();
   le_sample_density->setText("");
   recording.clear();
}

void US_DMA60::select_measurement(int item)
{
   QString str;
   current_measurement = item;
   le_units->setText(recording[item].units);
   le_description->setText(recording[item].description);
   str.sprintf("%15.6f", recording[item].T_sample);
   le_T_sample->setText(str);
   str.sprintf("%9.7e", recording[item].density);
   le_sample_density->setText(str);
   str.sprintf("%6.4e", recording[item].concentration);
   le_concentration->setText(str);
}

void US_DMA60::help()
{
   US_Help *online_help;
   online_help = new US_Help(this);
   online_help->show_help("manual/enter_dma60.html");
}

void US_DMA60::quit()
{
   close();
}
