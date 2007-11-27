#include "../include/us_hydrodyn_addatom.h"

US_AddAtom::US_AddAtom(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
	this->widget_flag = widget_flag;
	*widget_flag = true;
	USglobal = new US_Config();
	atom_filename = USglobal->config_list.root_dir + "/etc/somolist.atom";
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("SoMo: Modify Atom Lookup Tables"));
	setupGUI();
	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
}	

US_AddAtom::~US_AddAtom()
{
}

void US_AddAtom::setupGUI()
{
	int minHeight1 = 30;

	lbl_info = new QLabel(tr("Add/Edit Atom Lookup Table:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setMinimumHeight(minHeight1);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1, QFont::Bold));

	pb_select_file = new QPushButton(tr("Load Atom Definition File"), this);
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

	cmb_atoms = new QComboBox(false, this, "Atom Listing" );
	cmb_atoms->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_atoms->setSizeLimit(5);
	cmb_atoms->setMinimumHeight(minHeight1);
	connect(cmb_atoms, SIGNAL(activated(int)), this, SLOT(select_atom(int)));

	lbl_mw = new QLabel(tr(" Molecular Weight:"), this);
	Q_CHECK_PTR(lbl_mw);
	lbl_mw->setMinimumHeight(minHeight1);
	lbl_mw->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_number_of_atoms = new QLabel(tr(" Number of Atoms in File: 0"), this);
	Q_CHECK_PTR(lbl_number_of_atoms);
	lbl_number_of_atoms->setMinimumHeight(minHeight1);
	lbl_number_of_atoms->setAlignment(AlignLeft|AlignVCenter);
	lbl_number_of_atoms->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_number_of_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_radius = new QLabel(tr(" Radius (A):"), this);
	Q_CHECK_PTR(lbl_radius);
	lbl_radius->setMinimumHeight(minHeight1);
	lbl_radius->setAlignment(AlignLeft|AlignVCenter);
	lbl_radius->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_name = new QLabel(tr(" Atom Name:"), this);
	Q_CHECK_PTR(lbl_name);
	lbl_name->setMinimumHeight(minHeight1);
	lbl_name->setAlignment(AlignLeft|AlignVCenter);
	lbl_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_chain = new QLabel(tr(" Atom Assignment:"), this);
	Q_CHECK_PTR(lbl_chain);
	lbl_chain->setAlignment(AlignLeft|AlignVCenter);
	lbl_chain->setMinimumHeight(minHeight1);
	lbl_chain->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_mw = new QLineEdit(this, "Molecular Weight Line Edit");
	le_mw->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_mw->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_mw->setMinimumHeight(minHeight1);
	connect(le_mw, SIGNAL(textChanged(const QString &)), SLOT(update_mw(const QString &)));	

	le_radius = new QLineEdit(this, "Radius Line Edit");
	le_radius->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_radius->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_radius->setMinimumHeight(minHeight1);
	connect(le_radius, SIGNAL(textChanged(const QString &)), SLOT(update_radius(const QString &)));	

	le_name = new QLineEdit(this, "Atom name Line Edit");
	le_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_name->setMinimumHeight(minHeight1);
	connect(le_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));	
				
	cmb_chain = new QComboBox(false, this, "chain Listing" );
	cmb_chain->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_chain->setSizeLimit(5);
	cmb_chain->insertItem("Main Chain Atom");
	cmb_chain->insertItem("Side Chain Atom");
	cmb_chain->insertItem("Prosthetic Group");
	cmb_chain->insertItem("Undefined");
	cmb_chain->insertItem("Other");
	cmb_chain->setMinimumHeight(minHeight1);
	connect(cmb_chain, SIGNAL(activated(int)), this, SLOT(select_chain(int)));

	pb_add = new QPushButton(tr("Add Atom to File"), this);
	Q_CHECK_PTR(pb_add);
	pb_add->setEnabled(false);
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
	background->addWidget(lbl_number_of_atoms, j, 0);
	background->addWidget(cmb_atoms, j, 1);
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
	background->addWidget(lbl_chain, j, 0);
	background->addWidget(cmb_chain, j, 1);
	j++;
	background->addWidget(pb_add, j, 0);
	background->addWidget(pb_close, j, 1);
	
}

void US_AddAtom::add()
{
	int item = -1;
	QString str1;
	for (int i=0; i<(int) atom_list.size(); i++)
	{
		if (atom_list[i].name.upper() == new_atom.name.upper())
		{
			item = i;
			atom_list[i].mw = new_atom.mw;
			atom_list[i].radius = new_atom.radius;
			atom_list[i].chain = new_atom.chain;
		} 
	}
	if (item < 0)
	{
		atom_list.push_back(new_atom);
	}
	QFile f(atom_filename);
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		cmb_atoms->clear();
		str1.sprintf(tr(" Number of Atoms in File: %d"), atom_list.size());
		QTextStream ts(&f);
		for (unsigned int i=0; i<atom_list.size(); i++)
		{
			ts << atom_list[i].name.upper() << "\t" << atom_list[i].mw << "\t" << atom_list[i].radius << "\t" << atom_list[i].chain << endl;
			str1.sprintf("%d: ", i+1);
			str1 += atom_list[i].name.upper();
			cmb_atoms->insertItem(str1);
		}
		f.close();
	}
}

void US_AddAtom::select_file()
{
	QString old_filename = atom_filename, str1, str2;
	atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
	if (atom_filename.isEmpty())
	{
		atom_filename = old_filename;
	}
	else
	{
		lbl_table->setText(atom_filename);
		QFile f(atom_filename);
		atom_list.clear();
		cmb_atoms->clear();
		unsigned int i=1;
		if (f.open(IO_ReadOnly|IO_Translate))
		{
			QTextStream ts(&f);
			while (!ts.atEnd())
			{
				ts >> new_atom.name;
				ts >> new_atom.mw;
				ts >> new_atom.radius;
				ts >> new_atom.chain;
				str2 = ts.readLine(); // read rest of line
				if (!new_atom.name.isEmpty() && new_atom.radius > 0.0 && new_atom.mw > 0.0)
				{
					atom_list.push_back(new_atom);
					str1.sprintf("%d: ", i);
					str1 += new_atom.name;
					cmb_atoms->insertItem(str1);
					i++;
				}
			}
			f.close();
		}
	}
	str1.sprintf(tr(" Number of Atoms in File: %d"), atom_list.size());
	lbl_number_of_atoms->setText(str1);
	pb_add->setEnabled(true);
}

void US_AddAtom::update_mw(const QString &str)
{
	new_atom.mw = str.toFloat();
}

void US_AddAtom::update_radius(const QString &str)
{
	new_atom.radius = str.toFloat();
}

void US_AddAtom::update_name(const QString &str)
{
	new_atom.name = str;
}

void US_AddAtom::select_atom(int val)
{
	QString str;
	str.sprintf("%3.4f", atom_list[val].mw);
	le_mw->setText(str);
	str.sprintf("%3.4f", atom_list[val].radius);
	le_radius->setText(str);
	le_name->setText(atom_list[val].name.upper());
	cmb_chain->setCurrentItem(atom_list[val].chain);
}

void US_AddAtom::select_chain(int val)
{
	new_atom.chain = (unsigned int) val;
}

void US_AddAtom::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;

	*widget_flag = false;
	e->accept();
}
