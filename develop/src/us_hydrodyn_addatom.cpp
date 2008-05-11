#include "../include/us_hydrodyn_addatom.h"

US_AddAtom::US_AddAtom(bool *widget_flag, QWidget *p, const char *name) : QWidget( p, name)
{
	this->widget_flag = widget_flag;
	*widget_flag = true;
	USglobal = new US_Config();
	atom_filename = USglobal->config_list.system_dir + "/etc/somo.atom";
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

	pb_select_atom_file = new QPushButton(tr("Load Atom Definition File"), this);
	Q_CHECK_PTR(pb_select_atom_file);
	pb_select_atom_file->setMinimumHeight(minHeight1);
	pb_select_atom_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_atom_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_atom_file, SIGNAL(clicked()), SLOT(select_atom_file()));

	pb_select_hybrid_file = new QPushButton(tr("Load Hybridization File"), this);
	Q_CHECK_PTR(pb_select_hybrid_file);
	pb_select_hybrid_file->setMinimumHeight(minHeight1);
	pb_select_hybrid_file->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_select_hybrid_file->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_select_hybrid_file, SIGNAL(clicked()), SLOT(select_hybrid_file()));

	lbl_atom_table = new QLabel(tr(" not selected"),this);
	lbl_atom_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_atom_table->setAlignment(AlignCenter|AlignVCenter);
	lbl_atom_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_atom_table->setMinimumHeight(minHeight1);
	lbl_atom_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_hybrid_table = new QLabel(tr(" not selected"),this);
	lbl_hybrid_table->setFrameStyle(QFrame::WinPanel|QFrame::Sunken);
	lbl_hybrid_table->setAlignment(AlignCenter|AlignVCenter);
	lbl_hybrid_table->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_hybrid_table->setMinimumHeight(minHeight1);
	lbl_hybrid_table->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	cmb_atom = new QComboBox(false, this, "Atom Listing" );
	cmb_atom->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_atom->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_atom->setSizeLimit(5);
	cmb_atom->setMinimumHeight(minHeight1);
	connect(cmb_atom, SIGNAL(activated(int)), this, SLOT(select_atom(int)));

	cmb_hybrid = new QComboBox(false, this, "Hybrid Listing" );
	cmb_hybrid->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmb_hybrid->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	cmb_hybrid->setSizeLimit(5);
	cmb_hybrid->setMinimumHeight(minHeight1);
	connect(cmb_hybrid, SIGNAL(activated(int)), this, SLOT(select_hybrid(int)));

	lbl_mw1 = new QLabel(tr(" Molecular Weight:"), this);
	Q_CHECK_PTR(lbl_mw1);
	lbl_mw1->setMinimumHeight(minHeight1);
	lbl_mw1->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw2 = new QLabel("", this);
	Q_CHECK_PTR(lbl_mw2);
	lbl_mw2->setMinimumHeight(minHeight1);
	lbl_mw2->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_radius1 = new QLabel(tr(" Radius (A):"), this);
	Q_CHECK_PTR(lbl_radius1);
	lbl_radius1->setMinimumHeight(minHeight1);
	lbl_radius1->setAlignment(AlignLeft|AlignVCenter);
	lbl_radius1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_radius1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_radius2 = new QLabel("", this);
	Q_CHECK_PTR(lbl_radius2);
	lbl_radius2->setMinimumHeight(minHeight1);
	lbl_radius2->setAlignment(AlignLeft|AlignVCenter);
	lbl_radius2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_radius2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_hybrid1 = new QLabel(tr(" Hybridization:"), this);
	Q_CHECK_PTR(lbl_hybrid1);
	lbl_hybrid1->setMinimumHeight(minHeight1);
	lbl_hybrid1->setAlignment(AlignLeft|AlignVCenter);
	lbl_hybrid1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_hybrid1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_hybrid2 = new QLabel("", this);
	Q_CHECK_PTR(lbl_hybrid2);
	lbl_hybrid2->setMinimumHeight(minHeight1);
	lbl_hybrid2->setAlignment(AlignLeft|AlignVCenter);
	lbl_hybrid2->setPalette( QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit) );
	lbl_hybrid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));

	lbl_number_of_atoms = new QLabel(tr(" Number of Atoms in File: 0"), this);
	Q_CHECK_PTR(lbl_number_of_atoms);
	lbl_number_of_atoms->setMinimumHeight(minHeight1);
	lbl_number_of_atoms->setAlignment(AlignLeft|AlignVCenter);
	lbl_number_of_atoms->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_number_of_atoms->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_number_of_hybrids = new QLabel(tr(" Number of Hybridizations in File: 0"), this);
	Q_CHECK_PTR(lbl_number_of_hybrids);
	lbl_number_of_hybrids->setMinimumHeight(minHeight1);
	lbl_number_of_hybrids->setAlignment(AlignLeft|AlignVCenter);
	lbl_number_of_hybrids->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_number_of_hybrids->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_name = new QLabel(tr(" Atom Name:"), this);
	Q_CHECK_PTR(lbl_name);
	lbl_name->setMinimumHeight(minHeight1);
	lbl_name->setAlignment(AlignLeft|AlignVCenter);
	lbl_name->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_name = new QLineEdit(this, "Atom name Line Edit");
	le_name->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_name->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	le_name->setMinimumHeight(minHeight1);
	connect(le_name, SIGNAL(textChanged(const QString &)), SLOT(update_name(const QString &)));

	lbl_chain = new QLabel(tr(" Atom Assignment:"), this);
	Q_CHECK_PTR(lbl_chain);
	lbl_chain->setAlignment(AlignLeft|AlignVCenter);
	lbl_chain->setMinimumHeight(minHeight1);
	lbl_chain->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_chain->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

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

	pb_delete = new QPushButton(tr("Delete Atom"), this);
	Q_CHECK_PTR(pb_delete);
	pb_delete->setEnabled(false);
	pb_delete->setMinimumHeight(minHeight1);
	pb_delete->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize + 1));
	pb_delete->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_delete, SIGNAL(clicked()), SLOT(delete_atom()));

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
	background->addWidget(pb_select_atom_file, j, 0);
	background->addWidget(lbl_atom_table, j, 1);
	j++;
	background->addWidget(pb_select_hybrid_file, j, 0);
	background->addWidget(lbl_hybrid_table, j, 1);
	j++;
	background->addWidget(lbl_number_of_atoms, j, 0);
	background->addWidget(cmb_atom, j, 1);
	j++;
	background->addWidget(lbl_number_of_hybrids, j, 0);
	background->addWidget(cmb_hybrid, j, 1);
	j++;
	background->addWidget(lbl_name, j, 0);
	background->addWidget(le_name, j, 1);
	j++;
	background->addWidget(lbl_hybrid1, j, 0);
	background->addWidget(lbl_hybrid2, j, 1);
	j++;
	background->addWidget(lbl_mw1, j, 0);
	background->addWidget(lbl_mw2, j, 1);
	j++;
	background->addWidget(lbl_radius1, j, 0);
	background->addWidget(lbl_radius2, j, 1);
	j++;
	background->addWidget(lbl_chain, j, 0);
	background->addWidget(cmb_chain, j, 1);
	j++;
	background->addWidget(pb_add, j, 0);
	background->addWidget(pb_delete, j, 1);
	j++;
	background->addMultiCellWidget(pb_close, j, j, 0, 1);
}

void US_AddAtom::add()
{
	int item = -1;
	for (int i=0; i<(int) atom_list.size(); i++) //check if the same atom was defined previously
	{
		if (atom_list[i].name.upper() == current_atom.name.upper()
		&&  atom_list[i].hybrid.name.upper() == current_atom.hybrid.name.upper())
		{
			item = i;
			atom_list[i].hybrid.mw = current_atom.hybrid.mw;
			atom_list[i].hybrid.radius = current_atom.hybrid.radius;
			atom_list[i].chain = current_atom.chain;
		}
	}
	if (item < 0)
	{
		atom_list.push_back(current_atom);
	}
	write_atom_file();
	pb_delete->setEnabled(true);
}

void US_AddAtom::write_atom_file()
{
	QString str1;
	QFile f(atom_filename);
	if (f.open(IO_WriteOnly|IO_Translate))
	{
		cmb_atom->clear();
		str1.sprintf(tr(" Number of Atoms in File: %d"), atom_list.size());
		QTextStream ts(&f);
		for (unsigned int i=0; i<atom_list.size(); i++)
		{
			ts << atom_list[i].name.upper() << "\t" << atom_list[i].hybrid.name.upper() << "\t" <<
					atom_list[i].hybrid.mw << "\t" << atom_list[i].hybrid.radius << "\t" << atom_list[i].chain << endl;
			str1.sprintf("%d: ", i+1);
			str1 += atom_list[i].name.upper();
			str1 += " (";
			str1 += atom_list[i].hybrid.name.upper();
			str1 += ")";
			cmb_atom->insertItem(str1);
		}
		f.close();
	}
	else
	{
		QMessageBox::message("Attention:", "Could not open the atom file:\n\n" + atom_filename);
	}
}

void US_AddAtom::select_atom_file()
{
	QString old_filename = atom_filename, str1, str2;
	atom_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.atom *.ATOM", this);
	if (atom_filename.isEmpty())
	{
		atom_filename = old_filename;
	}
	else
	{
		lbl_atom_table->setText(atom_filename);
		QFile f(atom_filename);
		atom_list.clear();
		cmb_atom->clear();
		unsigned int i=1;
		if (f.open(IO_ReadOnly|IO_Translate))
		{
			QTextStream ts(&f);
			while (!ts.atEnd())
			{
				ts >> current_atom.name;
				ts >> current_atom.hybrid.name;
				ts >> current_atom.hybrid.mw;
				ts >> current_atom.hybrid.radius;
				ts >> current_atom.chain;
				str2 = ts.readLine(); // read rest of line
				if (!current_atom.name.isEmpty() && current_atom.hybrid.radius > 0.0 && current_atom.hybrid.mw > 0.0)
				{
					atom_list.push_back(current_atom);
					str1.sprintf("%d: ", i);
					str1 += current_atom.name;
					str1 += " (";
					str1 += current_atom.hybrid.name.upper();
					str1 += ")";
					cmb_atom->insertItem(str1);
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

void US_AddAtom::select_hybrid_file()
{
	QString old_filename = hybrid_filename, str1, str2;
	hybrid_filename = QFileDialog::getOpenFileName(USglobal->config_list.system_dir + "/etc", "*.hybrid *.HYBRID", this);
	if (hybrid_filename.isEmpty())
	{
		hybrid_filename = old_filename;
	}
	else
	{
		lbl_hybrid_table->setText(hybrid_filename);
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

void US_AddAtom::select_hybrid(int i)
{
	QString str;
	current_atom.hybrid.mw = hybrid_list[i].mw;
	current_atom.hybrid.name = hybrid_list[i].name;
	current_atom.hybrid.radius = hybrid_list[i].radius;
	str.sprintf("%f: ", current_atom.hybrid.mw);
	lbl_mw2->setText(str);
	str.sprintf("%f: ", current_atom.hybrid.radius);
	lbl_radius2->setText(str);
	lbl_hybrid2->setText(current_atom.hybrid.name);
}

void US_AddAtom::update_name(const QString &str)
{
	current_atom.name = str;
}

void US_AddAtom::update_hybridization_name(const QString &str)
{
	current_atom.hybrid.name = str;
}

void US_AddAtom::select_atom(int val)
{
	QString str;
	current_atom.name = atom_list[val].name.upper();
	current_atom.hybrid.mw = atom_list[val].hybrid.mw;
	current_atom.hybrid.name = atom_list[val].hybrid.name;
	current_atom.hybrid.radius = atom_list[val].hybrid.radius;
	current_atom.chain = atom_list[val].chain;

	str.sprintf("%3.4f", atom_list[val].hybrid.mw);
	lbl_mw2->setText(str);
	str.sprintf("%3.4f", atom_list[val].hybrid.radius);
	lbl_radius2->setText(str);
	le_name->setText(atom_list[val].name.upper());
	lbl_hybrid2->setText(atom_list[val].hybrid.name.upper());
	cmb_chain->setCurrentItem(atom_list[val].chain);
	pb_delete->setEnabled(true);
}

void US_AddAtom::select_chain(int val)
{
	current_atom.chain = (unsigned int) val;
}

void US_AddAtom::closeEvent(QCloseEvent *e)
{
	global_Xpos -= 30;
	global_Ypos -= 30;

	*widget_flag = false;
	e->accept();
}

void US_AddAtom::delete_atom()
{
	vector <struct atom>::iterator it;
	for (it=atom_list.begin(); it != atom_list.end(); it++)
	{
		if ((*it).name == current_atom.name && (*it).hybrid.name == current_atom.hybrid.name)
		{
			atom_list.erase(it);
			break;
		}
	}
	write_atom_file();
}

