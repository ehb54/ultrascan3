#include "../include/us_equilspeed.h"

US_EquilSpeed::US_EquilSpeed(QWidget *parent, const char *name)
: QFrame( parent, name)
{
	USglobal = new US_Config();

	int bannerh=30;
	sigma = 2.5;
	mw = 35000;
	volume = 120;
	temperature = 20;
	vbar20      = (float) 0.72;
	vbar        = (float) 0.72;
	density_w20 = (float) DENS_20W;
	an50 = false;
	alu_centerpiece = false;
	flash_time1 = new QTimer(this);
	connect(flash_time1, SIGNAL(timeout()), SLOT(animate1()));
	flash_time2 = new QTimer(this);
	connect(flash_time2, SIGNAL(timeout()), SLOT(animate2()));
	blink1 = 1;
	blink2 = 1;
	setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

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

	lbl_rpm1 = new QLabel(tr(" Suggested Speed:"),this);
	lbl_rpm1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_rpm1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rpm1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_rpm2 = new QLabel(this);
	lbl_rpm2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_rpm2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_rpm2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));


	banner1 = new QLabel(tr(" Sample Settings:"),this);
	banner1->setFrameStyle(QFrame::WinPanel|Raised);
	banner1->setMinimumHeight(bannerh);
	banner1->setAlignment(AlignCenter|AlignVCenter);
	banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	banner2 = new QLabel(tr(" Experimental Settings:"),this);
	banner2->setFrameStyle(QFrame::WinPanel|Raised);
	banner2->setAlignment(AlignCenter|AlignVCenter);
	banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner2->setMinimumHeight(bannerh);
	banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw = new QLabel(tr(" Est. MW:"),this);
	lbl_mw->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	ct_mw = new QwtCounter(this);
	ct_mw->setRange(500, 1e7, 100);
	ct_mw->setValue(35000);
	ct_mw->setNumButtons(3);
	ct_mw->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//	connect(ct_mw, SIGNAL(buttonReleased(double)), SLOT(update_mw(double)));
	connect(ct_mw, SIGNAL(valueChanged(double)), SLOT(update_mw(double)));

	lbl_sigma = new QLabel(tr(" Sigma:"),this);
	lbl_sigma->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_sigma->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	ct_sigma = new QwtCounter(this);
	ct_sigma->setRange(0.1, 20, 0.01);
	ct_sigma->setValue(2.5);
	ct_sigma->setNumButtons(3);
	ct_sigma->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	ct_sigma->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
//	connect(ct_sigma, SIGNAL(buttonReleased(double)), SLOT(update_sigma(double)));
	connect(ct_sigma, SIGNAL(valueChanged(double)), SLOT(update_sigma(double)));

	pb_density = new QPushButton(tr(" Density (20ºC):"), this);
	Q_CHECK_PTR(pb_density);
	pb_density->setAutoDefault(false);
	pb_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_density->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_density, SIGNAL(clicked()), SLOT(get_buffer()));

	le_density = new QLineEdit(this);
	le_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	QString str;
	le_density->setText( str.sprintf( "%8.6f", DENS_20W ) );
	connect(le_density, SIGNAL(textChanged(const QString &)), SLOT(update_density(const QString &)));

	lbl_ratio1 = new QLabel(tr(" Conc. Ratio Rb/Rm:"),this);
	lbl_ratio1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_ratio1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_ratio1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_ratio2 = new QLabel(this);
	lbl_ratio2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_ratio2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_ratio2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));

	pb_vbar = new QPushButton(tr(" Vbar (20º):"), this);
	Q_CHECK_PTR(pb_vbar);
	pb_vbar->setAutoDefault(false);
	pb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	pb_vbar->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_vbar, SIGNAL(clicked()), SLOT(read_vbar()));

	le_vbar = new QLineEdit(this);
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_vbar->setText(" 0.7200");
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

	lbl_column1 = new QLabel(tr(" Column Length:"),this);
	lbl_column1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_column1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_column1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_column2 = new QLabel(this);
	lbl_column2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_column2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_column2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));

	lbl_volume = new QLabel(tr(" Loading Volume (µl):"),this);
	lbl_volume->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_volume->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_volume = new QLineEdit(this);
	le_volume->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_volume->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_volume->setText( " 120");
	connect(le_volume, SIGNAL(textChanged(const QString &)), SLOT(update_volume(const QString &)));

	lbl_meniscus1 = new QLabel(tr(" Meniscus Position:"),this);
	lbl_meniscus1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_meniscus1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_meniscus1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_meniscus2 = new QLabel(this);
	lbl_meniscus2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_meniscus2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_meniscus2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_temperature = new QLabel(tr(" Temperature (ºC):"),this);
	lbl_temperature->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_temperature->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_temperature = new QLineEdit(this);
	le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_temperature->setText(" 20");
	le_temperature->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temperature(const QString &)));

	lbl_bottom1 = new QLabel(tr(" Bottom of Cell Position:"),this);
	lbl_bottom1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_bottom1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_bottom1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_bottom2 = new QLabel("", this);
	lbl_bottom2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_bottom2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_bottom2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));

	lbl_rotor = new QLabel(tr(" Rotor Type:"),this);
	lbl_rotor->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_rotor->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_rotor->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	bt_an50 = new QRadioButton(this);
	bt_an50->setText(tr("An 50 Ti"));
	bt_an50->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bt_an50->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_an50->setChecked(false);
	connect(bt_an50, SIGNAL(clicked()), SLOT(update_an50()));

	bt_an60 = new QRadioButton(this);
	bt_an60->setText(tr("An 60 Ti"));
	bt_an60->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bt_an60->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_an60->setChecked(true);
	connect(bt_an60, SIGNAL(clicked()), SLOT(update_an60()));

	lbl_overspeed1 = new QLabel(tr(" Rotor Overspeed Check:"), this);
	lbl_overspeed1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_overspeed1->setAlignment(AlignLeft|AlignVCenter);
	lbl_overspeed1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_lcd1 = new QLabel("", this);
	lbl_lcd1->setFixedSize(25,16);
	lbl_lcd1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lcd1->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );


	lbl_lcd2 = new QLabel("", this);
	lbl_lcd2->setFixedSize(25,16);
	lbl_lcd2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lcd2->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );

	lbl_centerpiece = new QLabel(tr(" Centerpiece:"), this);
	lbl_centerpiece->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_centerpiece->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_centerpiece->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	bt_aluminum = new QRadioButton(this);
	bt_aluminum->setText(tr("Aluminum"));
	bt_aluminum->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_aluminum->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bt_aluminum->setChecked(false);
	connect(bt_aluminum, SIGNAL(clicked()), SLOT(update_aluminum()));

	bt_epon = new QRadioButton(this);
	bt_epon->setText(tr("Epon"));
	bt_epon->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	bt_epon->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	bt_epon->setChecked(true);
	connect(bt_epon, SIGNAL(clicked()), SLOT(update_epon()));

	lbl_overspeed2 = new QLabel(tr(" Centerpiece Overspeed Check:"), this);
	lbl_overspeed2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label) );
	lbl_overspeed2->setAlignment(AlignLeft|AlignVCenter);
	lbl_overspeed2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_lcd3 = new QLabel("", this);
	lbl_lcd3->setFixedSize(25,16);
	lbl_lcd3->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lcd3->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );

	lbl_lcd4 = new QLabel("", this);
	lbl_lcd4->setFixedSize(25,16);
	lbl_lcd4->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_lcd4->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );


	banner3 = new QLabel(tr(" Estimated Maximum Number of Datapoints/Scan:"),this);
	banner3->setFrameStyle(QFrame::WinPanel|Raised);
	banner3->setAlignment(AlignCenter|AlignVCenter);
	banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame) );
	banner3->setMinimumHeight(bannerh);
	banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_intpoints1 = new QLabel(tr(" Interference Optics:"),this);
	lbl_intpoints1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_intpoints1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_intpoints1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_intpoints2 = new QLabel(this);
	lbl_intpoints2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_intpoints2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_intpoints2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));

	lbl_abspoints1 = new QLabel(tr(" Absorption Optics:"),this);
	lbl_abspoints1->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_abspoints1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_abspoints1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_abspoints2 = new QLabel(this);
	lbl_abspoints2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_abspoints2->setFrameStyle(QFrame::WinPanel|Sunken);
	lbl_abspoints2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));

//	setFixedSize(556, ypos + buttonh + 4);
	calc();

	setup_GUI();
}

US_EquilSpeed::~US_EquilSpeed()
{
}

void US_EquilSpeed::setup_GUI()
{
	int j=0;
	int rows = 11, columns = 4, spacing = 2;
	QGridLayout * background = new QGridLayout(this,rows,columns,spacing);
	for (int i=0; i<rows; i++)
	{
		background->setRowSpacing(i, 26);
	}
	background->setColStretch(2,2);
	background->setColStretch(5,2);

	QHBoxLayout *button = new QHBoxLayout(2);
	button->addWidget(pb_help);
	button->addWidget(pb_close);
	background->addMultiCellLayout(button,j,j,0,2);
	background->addMultiCellWidget(lbl_rpm1,j,j,3,4);
	background->addWidget(lbl_rpm2,j,5);
	j++;
	background->addMultiCellWidget(banner1,j,j,0,2);
	background->addMultiCellWidget(banner2,j,j,3,5);
	j++;
	background->addWidget(lbl_mw,j,0);
	background->addMultiCellWidget(ct_mw,j,j,1,2);
	background->addWidget(lbl_sigma,j,3);
	background->addMultiCellWidget(ct_sigma,j,j,4,5);
	j++;
	background->addMultiCellWidget(pb_density,j,j,0,1);
	background->addWidget(le_density,j,2);
	background->addMultiCellWidget(lbl_ratio1,j,j,3,4);
	background->addWidget(lbl_ratio2,j,5);
	j++;
	background->addMultiCellWidget(pb_vbar,j,j,0,1);
	background->addWidget(le_vbar,j,2);
	background->addMultiCellWidget(lbl_column1,j,j,3,4);
	background->addWidget(lbl_column2,j,5);
	j++;
	background->addMultiCellWidget(lbl_volume,j,j,0,1);
	background->addWidget(le_volume,j,2);
	background->addMultiCellWidget(lbl_meniscus1,j,j,3,4);
	background->addWidget(lbl_meniscus2,j,5);
	j++;
	background->addMultiCellWidget(lbl_temperature,j,j,0,1);
	background->addWidget(le_temperature,j,2);
	background->addMultiCellWidget(lbl_bottom1,j,j,3,4);
	background->addWidget(lbl_bottom2,j,5);
	j++;
	background->addWidget(lbl_rotor,j,0);
	QHBoxLayout *radio1 = new QHBoxLayout(0);
	radio1->addWidget(bt_an50);
	radio1->addWidget(bt_an60);
	background->addMultiCellLayout(radio1,j,j,1,2);
	background->addMultiCellWidget(lbl_overspeed1,j,j,3,4);
	QHBoxLayout *lcd1 = new QHBoxLayout(5);
	lcd1->addWidget(lbl_lcd1);
	lcd1->addWidget(lbl_lcd2);
	background->addLayout(lcd1,j,5);
	j++;
	background->addWidget(lbl_centerpiece,j,0);
	QHBoxLayout *radio2 = new QHBoxLayout(0);
	radio2->addWidget(bt_aluminum);
	radio2->addWidget(bt_epon);
	background->addMultiCellLayout(radio2,j,j,1,2);
	background->addMultiCellWidget(lbl_overspeed2,j,j,3,4);
	QHBoxLayout *lcd2 = new QHBoxLayout(5);
	lcd2->addWidget(lbl_lcd3);
	lcd2->addWidget(lbl_lcd4);
	background->addLayout(lcd2,j,5);
	j++;
	background->addMultiCellWidget(banner3,j,j,0,5);
	j++;
	background->addMultiCellWidget(lbl_intpoints1,j,j,0,1);
	background->addWidget(lbl_intpoints2,j,2);
	background->addMultiCellWidget(lbl_abspoints1,j,j,3,4);
	background->addWidget(lbl_abspoints2,j,5);

}

void US_EquilSpeed::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_EquilSpeed::update_an50()
{
	bt_an60->setChecked(FALSE);
	an50 = true;
	calc();
}

void US_EquilSpeed::update_an60()
{
	bt_an50->setChecked(FALSE);
	an50 = false;
	calc();
}

void US_EquilSpeed::update_aluminum()
{
	bt_epon->setChecked(FALSE);
	alu_centerpiece = true;
	calc();
}

void US_EquilSpeed::update_epon()
{
	bt_aluminum->setChecked(FALSE);
	alu_centerpiece = false;
	calc();
}


void US_EquilSpeed::read_vbar()
{

	vbar_from_seq = true;
	vbar_dlg = new US_Vbar_DB(temperature, &vbar, &vbar20, true, false, -1);
	vbar_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	vbar_dlg->setCaption(tr("V-bar Calculation"));
	vbar_dlg->show();
	connect(vbar_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_vbar_lbl(float, float)));
}

void US_EquilSpeed::update_vbar(const QString &val)
{
	vbar20 = val.toFloat();
	vbar = vbar20 + (4.25e-4 * (temperature - 20));
	calc();
}

void US_EquilSpeed::get_buffer()
{

	buffer_dlg = new US_Buffer_DB(false, -1);
	buffer_dlg->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	buffer_dlg->setCaption(tr("Buffer Calculation"));
	buffer_dlg->show();
	connect(buffer_dlg, SIGNAL(valueChanged(float, float)), SLOT(update_buffer_lbl(float, float)));
}

void US_EquilSpeed::update_mw(double val)
{
	mw = (float) val;
	calc();
}

void US_EquilSpeed::update_sigma(double val)
{
	sigma = (float) val;
	calc();
}

void US_EquilSpeed::update_density(const QString &val)
{
	density_w20 = val.toFloat();
	calc();
}

void US_EquilSpeed::update_volume(const QString &val)
{
	volume = val.toFloat();
	calc();
}

void US_EquilSpeed::update_temperature(const QString &val)
{
	temperature = val.toFloat();
	calc();
}

void US_EquilSpeed::update_vbar_lbl(float val, float val20)
{
	vbar = val;
	vbar20 = val20;
	QString str;
	str.sprintf("%6.4f", vbar20);
	le_vbar->setText(str);
	calc();
}

void US_EquilSpeed::update_buffer_lbl(float dens, float)
{
// visc is not used for equilibrium experiments

	QString str, str2;
	density_w20 = dens;
	str.sprintf("%6.4f", density_w20);
	le_density->setText(str);
	calc();
}

void US_EquilSpeed::calc_correction()
{
	float t, density_wt;
	t			    = temperature;
	density_wt	 = 1.000028e-3 * ((999.83952 + 16.945176 * t) / (1 + 16.879850e-3 * t))
					 - 1.000028e-3 * ((7.9870401e-3 * pow(t, 2) + 46.170461e-6 * pow(t, 3))
										/ (1 + 16.87985e-3 * t))
					 + 1.000028e-3 * ((105.56302e-9 * pow(t, 4) - 280.54253e-12 * pow(t, 5))
										/ (1 + 16.87985e-3 * t));

	density_tb   = density_w20 * density_wt / DENS_20W;
	buoyancyb	 = 1 - vbar * density_tb;
	buoyancyw	 = 1 - vbar20 * DENS_20W;
	correction   = (buoyancyw / buoyancyb);
}

void US_EquilSpeed::calc()
{
	QString str;
	float omega_s;
	float column;
	calc_correction();
	rpm = (uint) (30/M_PI * pow((sigma * R * 2 * (temperature + K0))/(mw * buoyancyb), 0.5) + 0.5);
	omega_s = pow(M_PI/30 * rpm, 2);
	lbl_rpm2->setText(str.sprintf(" %d rpm", rpm));
	if (an50)
	{
		bottom = 7.754e-5 - 1.546e-8 * rpm + 9.601e-12 * pow((double) rpm, (double) 2)
				 - 5.800e-17 * pow((double) rpm, (double) 3) + 6.948e-22 * pow((double) rpm, (double) 4);
	}
	else
	{
		bottom = 3.128e-5 - 6.620e-9 * rpm + 7.264e-12 * pow((double) rpm, (double) 2)
				 - 6.152e-17 * pow((double) rpm, (double) 3) + 5.760e-22 * pow((double) rpm, (double) 4);
	}
	if (alu_centerpiece)
	{
		bottom += (float) 7.15;
	}
	else
	{
		bottom += (float) 7.195;
	}
	lbl_bottom2->setText(str.sprintf(" %1.3f cm", bottom));
//
//  square_root[ Rbottom^2 - (volume * 360 degrees * 0.001 µl/ccm / M_PI * 1.2 cm * 2.5 degrees)]
//
	meniscus = pow( (pow(bottom, 2) - ((volume * 0.360) / (M_PI * 3))), 0.5);
	lbl_meniscus2->setText(str.sprintf(" %1.3f cm", meniscus));
	column = 10*(bottom - meniscus);
	lbl_column2->setText(str.sprintf(" %2.2f mm", column));
	ratio = exp((omega_s * mw * buoyancyb * (pow(bottom, 2) - pow(meniscus, 2))
					/ (2 * R * (temperature + K0))));
	lbl_ratio2->setText(str.sprintf(" %1.4e", ratio));
	lbl_intpoints2->setText(str.sprintf(" %d/scan", (uint) (column *100)));
	lbl_abspoints2->setText(str.sprintf(" %d/scan", (uint) (0.5+column*100/1.6)));
	valid();
}

void US_EquilSpeed::valid()
{

	if ((an50 && rpm > 50000) || (rpm > 60000))
	{
		lbl_lcd1->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );
		flash_time1->start(200);
	}
	else
	{
		if (flash_time1->isActive())
		{
			flash_time1->stop();
		}
		lbl_lcd1->setPalette( QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green) );
		lbl_lcd2->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );
	}
	if ((!alu_centerpiece && rpm > 42000) || (rpm > 60000))
	{
		lbl_lcd3->setPalette( QPalette(USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green, USglobal->global_colors.cg_dk_green) );
		flash_time2->start(200);
	}
	else
	{
		if (flash_time2->isActive())
		{
			flash_time2->stop();
		}
		lbl_lcd3->setPalette( QPalette(USglobal->global_colors.cg_green, USglobal->global_colors.cg_green, USglobal->global_colors.cg_green) );
		lbl_lcd4->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );
	}
}

void US_EquilSpeed::animate1()
{

	blink1 = -blink1;
	if (blink1 > 0)
	{
		lbl_lcd2->setPaletteForegroundColor (Qt::red);
		lbl_lcd2->setPaletteBackgroundColor (Qt::red);
	}
	else
	{
		lbl_lcd2->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );
	}
}

void US_EquilSpeed::animate2()
{

	blink2 = -blink2;
	if (blink2 > 0)
	{
		lbl_lcd4->setPaletteForegroundColor (Qt::red);
		lbl_lcd4->setPaletteBackgroundColor (Qt::red);
	}
	else
	{
		lbl_lcd4->setPalette( QPalette(USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red, USglobal->global_colors.cg_dk_red) );
	}
}

void US_EquilSpeed::help()
{
	US_Help *online_help;
	online_help = new US_Help(this);
	online_help->show_help("manual/calc_equil.html");
}

void US_EquilSpeed::quit()
{
	close();
}


