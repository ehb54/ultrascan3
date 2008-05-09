#include "../include/us_hydrodyn_addhybrid.h"

US_AddHybridization::US_AddHybridization(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
	this->widget_flag = widget_flag;
	*widget_flag = true;
	USglobal = new US_Config();
	hybrid_filename = USglobal->config_list.system_dir + "/etc/somo.hybrid";
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SoMo: Modify Hybridization Lookup Tables"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}	

US_AddHybridization::~US_AddHybridization()
{
}

void US_AddHybridization::setupGUI()
{
	int minHeight1 = 30;

	lbl_info = new QLabel(tr("Add/Edit Hybridization Lookup Table:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_select_file = new QPushButton(tr("Load Hybridization Definition File"), this);
	Q_CHECK_PTR(pb_select_file);
	pb_select_file->setMinimumHeight(minHeight1);
	pb_select_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_file, SIGNAL(clicked()), SLOT(select_file()));

	lbl_table = new QLabel(tr(" not selected"),this);
	lbl_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_table->setAlignment(AlignCenter|AlignVCenter);
	lbl_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_table->setMinimumHeight(minHeight1);
	lbl_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	cmb_hybrid = new QComboBox(false, this, "Hybridization Listing" );
	cmb_hybrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_hybrid->setSizeLimit(5);
	cmb_hybrid->setMinimumHeight(minHeight1);
	connect(cmb_hybrid, SIGNAL(activated(int)), this, SLOT(select_hybrid(int)));

	lbl_mw = new QLabel(tr(" Molecular Weight:"), this);
	Q_CHECK_PTR(lbl_mw);
	lbl_mw->setMinimumHeight(minHeight1);
	lbl_mw->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_mw = new QLineEdit(this, "Molecular Weight Line Edit");
	le_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_mw->setMinimumHeight(minHeight1);
	connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));

	lbl_number_of_hybrids = new QLabel(tr(" Number of Hybridizations in File: 0"), this);
	Q_CHECK_PTR(lbl_number_of_hybrids);
	lbl_number_of_hybrids->setMinimumHeight(minHeight1);
	lbl_number_of_hybrids->setAlignment(AlignLeft|AlignVCenter);
	lbl_number_of_hybrids->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_number_of_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_radius = new QLabel(tr(" Radius (A):"), this);
	Q_CHECK_PTR(lbl_radius);
	lbl_radius->setMinimumHeight(minHeight1);
	lbl_radius->setAlignment(AlignLeft|AlignVCenter);
	lbl_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_radius = new QLineEdit(this, "Radius Line Edit");
	le_radius->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_radius->setMinimumHeight(minHeight1);
	connect(le_radius, SIGNAL(textChanged(const QString &)), SLOT(update_radius(const QString &)));

	lbl_name = new QLabel(tr(" Hybridization Name:"), this);
	Q_CHECK_PTR(lbl_name);
	lbl_name->setMinimumHeight(minHeight1);
	lbl_name->setAlignment(AlignLeft|AlignVCenter);
	lbl_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_name = new QLineEdit(this, "Hybridization name Line Edit");
	le_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_name->setMinimumHeight(minHeight1);
	connect(le_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));	
				
	pb_add = new QPushButton(tr("Add Hybridization to File"), this);
	Q_CHECK_PTR(pb_add);
	pb_add->setEnabled(true);
	pb_add->setMinimumHeight(minHeight1);
	pb_add->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_add->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_add, SIGNAL(clicked()), SLOT(add()));

	pb_close = new QPushButton(tr("Close"), this);
	Q_CHECK_PTR(pb_close);
	pb_close->setMinimumHeight(minHeight1);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_close->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_close, SIGNAL(clicked()), SLOT(close()));

	int rows=3, columns = 2, spacing = 2, j=0, margin=4;
	QGridLayout *background = new QGridLayout(this, rows, columns, margin, spacing);

	background->addMultiCellWidget(lbl_info, j, j, 0, 1);
	j++;
	background->addWidget(pb_select_file, j, 0);
	background->addWidget(lbl_table, j, 1);
	j++;
	background->addWidget(lbl_number_of_hybrids, j, 0);
	background->addWidget(cmb_hybrid, j, 1);
	j++;
	background->addWidget(lbl_name, j, 0);
	background->addWidget(le_name, j, 1);
	j++;
	background->addWidget(lbl_mw, j, 0);
	background->addWidget(le_mw, j, 1);
	j++;
	background->addWidget(lbl_radius, j, 0);
	background->addWidget(le_radius, j, 1);
	j++;
	background->addWidget(pb_add, j, 0);
	background->addWidget(pb_close, j, 1);
	
}

void US_AddHybridization::add()
{
	int item = -1;
	QString str1;
	for (int i=0; i<(int) hybrid_list.size(); i++)
	{
		if (hybrid_list[i].name.upper() == current_hybrid.name.upper())
		{
			item = i;
			hybrid_list[i].mw = current_hybrid.mw;
			hybrid_list[i].radius = current_hybrid.radius;
		}
	}
	if (item < 0)
	{
		hybrid_list.push_back(current_hybrid);
	}
	QFile f(hybrid_filename);
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		cmb_hybrid->clear();
		str1.sprintf(tr(" Number of Hybridizations in File: %d"), hybrid_list.size());
		QTextStream ts(&f);
		for (unsigned int i=0; i<hybrid_list.size(); i++)
		{
			ts << hybrid_list[i].name.upper() << "\t" << hybrid_list[i].mw << "\t" << hybrid_list[i].radius << endl;
			cout << "item: " << item << ", " << hybrid_list[i].name.upper() << "\t" << hybrid_list[i].mw << "\t" << hybrid_list[i].radius << ", " <<  hybrid_filename << endl;
			str1.sprintf("%d: ", i+1);
			str1 += hybrid_list[i].name.upper();
			cmb_hybrid->insertItem(str1);
		}
		f.close();
	}
	else
	{
		QMessageBox::message("Attention:", "Could not open the hybridization file:\n\n" + hybrid_filename);
	}
}

void US_AddHybridization::select_file()
{
	QString old_filename = hybrid_filename, str1, str2;
	hybrid_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.hybrid *.HYBRID", this);
	if (hybrid_filename.isEmpty())
	{
		hybrid_filename = old_filename;
	}
	else
	{
		lbl_table->setText(hybrid_filename);
		QFile f(hybrid_filename);
		hybrid_list.clear();
		cmb_hybrid->clear();
		unsigned int i=1;
		if (f.open(IO_ReadOnly|IO_Translate))
		{
			QTextStream ts(&f);
			while (!ts.atEnd())
			{
				ts >> current_hybrid.name;
				ts >> current_hybrid.mw;
				ts >> current_hybrid.radius;
				str2 = ts.readLine(); // read rest of line
				if (!current_hybrid.name.isEmpty() && current_hybrid.radius > 0.0 && current_hybrid.mw > 0.0)
				{
					hybrid_list.push_back(current_hybrid);
					str1.sprintf("%d: ", i);
					str1 += current_hybrid.name;
					cmb_hybrid->insertItem(str1);
					i++;
				}
			}
			f.close();
		}
	}
	str1.sprintf(tr(" Number of Hybridizations in File: %d"), hybrid_list.size());
	lbl_number_of_hybrids->setText(str1);
	pb_add->setEnabled(true);
}

void US_AddHybridization::update_mw(const QString &str)
{
	current_hybrid.mw = str.toFloat();
}

void US_AddHybridization::update_radius(const QString &str)
{
	current_hybrid.radius = str.toFloat();
}

void US_AddHybridization::update_name(const QString &str)
{
	current_hybrid.name = str;
}

void US_AddHybridization::select_hybrid(int val)
{
	QString str;
	str.sprintf("%3.4f", hybrid_list[val].mw);
	le_mw->setText(str);
	str.sprintf("%3.4f", hybrid_list[val].radius);
	le_radius->setText(str);
	le_name->setText(hybrid_list[val].name.upper());
}

void US_AddHybridization::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;

	*widget_flag = false;
	e->accept();
}
