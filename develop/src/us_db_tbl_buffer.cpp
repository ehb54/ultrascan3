#include "../include/us_db_tbl_buffer.h"

//! Constructor
/*!
	Constractor a new <var>US_Buffer_DB</var> interface,
	\param from_cell <tt>true</tt> called from US_Cell_DB Table, <tt>false</tt> otherwise.
	\param parent Parent widget.
	\param temp_InvID the investigator ID for the buffer
	\param name Widget name.
*/
US_Buffer_DB::US_Buffer_DB(bool from_cell, int temp_InvID, QWidget *parent, const char *name)
: US_DB(parent, name)
{
	read_template_file();
	setup_GUI();
	cell_flag = from_cell;
	buf_init();
	if(temp_InvID > 0)
	{
		lbl_investigator->setText(show_investigator(temp_InvID));
		Buffer.investigatorID = temp_InvID;
	}
	else
	{
		lbl_investigator->setText(tr(" Not Selected"));
	}
}

//! Destructor
/*! destroy the <var>US_Buffer_DB</var>. */
US_Buffer_DB::~US_Buffer_DB()
{
}

void US_Buffer_DB::buf_init()
{
	Buffer.component.clear();
	Buffer.refractive_index = 0.0;
	Buffer.density = 0.0;
	Buffer.viscosity = 0.0;
	Buffer.description = "";
	Buffer.bufferID = -1;
	Buffer.investigatorID = -1;
	lbl_investigator->setText(" Not Selected");
}

/*!
	If temp_GUI is <var>true</var>, This function will create an interface for US_Buffer_DB.
*/
void US_Buffer_DB::setup_GUI()
{
	unsigned int i, j;

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	pb_load = new QPushButton(tr("Load Buffer from HD"), this);
	Q_CHECK_PTR(pb_load);
	pb_load->setAutoDefault(false);
	pb_load->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load, SIGNAL(clicked()), SLOT(read_buffer()));

	pb_save = new QPushButton(tr("Save Buffer to HD"), this);
	Q_CHECK_PTR(pb_save);
	pb_save->setAutoDefault(false);
	pb_save->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save, SIGNAL(clicked()), SLOT(save_buffer()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_load_db = new QPushButton(tr("Query Buffer from DB"), this);
	Q_CHECK_PTR(pb_load_db);
	pb_load_db->setAutoDefault(false);
	pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_db, SIGNAL(clicked()), SLOT(read_db()));

	lbl_banner1 = new QLabel(tr("Doubleclick on buffer data to select:"),this);
	lbl_banner1->setAlignment(AlignHCenter|AlignVCenter);
	lbl_banner1->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));

	lb_buffer_db = new QListBox(this, "Buffer files");
	lb_buffer_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_buffer_db->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_buffer_db, SIGNAL(selected(int)), SLOT(select_buff(int)));
//	connect(lb_buffer_db, SIGNAL(highlighted(int)), SLOT(select_buff(int)));

	pb_save_db = new QPushButton(tr("Backup Buffer to DB"), this);
	Q_CHECK_PTR(pb_save_db);
	pb_save_db->setAutoDefault(false);
	pb_save_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_db, SIGNAL(clicked()), SLOT(save_db()));

	pb_update_db = new QPushButton(tr("Update Buffer"), this);
	Q_CHECK_PTR(pb_update_db);
	pb_update_db->setAutoDefault(false);
	pb_update_db->setEnabled(false);
	pb_update_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_update_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_update_db, SIGNAL(clicked()), SLOT(update_db()));

	pb_del_db = new QPushButton(tr("Delete Buffer from DB"), this);
	Q_CHECK_PTR(pb_del_db);
	pb_del_db->setAutoDefault(false);
	pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_del_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));

	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_accept = new QPushButton(tr("Accept"), this);
	Q_CHECK_PTR(pb_accept);
	pb_accept->setAutoDefault(false);
	pb_accept->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_accept->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_accept, SIGNAL(clicked()), SLOT(accept()));

	lbl_density = new QLabel(tr(" Density (20ºC, g/ccm): "),this);
	lbl_density->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_density = new QLineEdit(this);
	le_density->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_density->setText(" 0.0");
	le_density->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));

	lbl_viscosity = new QLabel(tr(" Viscosity (20ºC, cp): "),this);
	lbl_viscosity->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_viscosity = new QLineEdit(this);
	le_viscosity->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_viscosity->setText(" 0.0");
	le_viscosity->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
	
	lbl_refractive_index = new QLabel(tr(" Refractive Index (20ºC): "),this);
	lbl_refractive_index->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_refractive_index->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_refractive_index = new QLineEdit(this);
	le_refractive_index->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_refractive_index->setText(" 0.0");
	le_refractive_index->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_refractive_index, SIGNAL(textChanged(const QString &)), this, SLOT(update_refractive_index(const QString &)));

	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	lbl_investigator= new QLabel("",this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	if(Buffer.investigatorID > 0)
	{
		lbl_investigator->setText(show_investigator(Buffer.investigatorID));
	}
	else
	{
		lbl_investigator->setText(tr(" Not Selected"));
	}

	lbl_description = new QLabel(tr(" Buffer Description:"),this);
	lbl_description->setAlignment(AlignLeft|AlignVCenter);
	lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_description = new QLineEdit(this);
	le_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_description->setText("");
	le_description->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_description, SIGNAL(textChanged(const QString &)), this, SLOT(update_description(const QString &)));

	lbl_buffer1 = new QLabel(tr(" Please select a Buffer Component: "),this);
	lbl_buffer1->setAlignment(AlignLeft|AlignVCenter);
	lbl_buffer1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_buffer1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_unit = new QLabel("",this);
	lbl_unit->setAlignment(AlignHCenter|AlignVCenter);
	lbl_unit->setPalette( QPalette(USglobal->global_colors.cg_red, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_unit->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_buffer2 = new QLabel("",this);
	lbl_buffer2->setAlignment(AlignLeft|AlignVCenter);
	lbl_buffer2->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_buffer2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize -1, QFont::Bold));

	le_concentration = new QLineEdit(this);
	le_concentration->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_concentration->setText("");
	le_concentration->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(le_concentration, SIGNAL(textChanged(const QString &)), this, SLOT(update_concentration(const QString &)));
	connect(le_concentration, SIGNAL(returnPressed()), this, SLOT(add_component()));

	lbl_banner2 = new QLabel(tr("Click on item to select:"),this);
	lbl_banner2->setAlignment(AlignHCenter|AlignVCenter);
	lbl_banner2->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_banner3 = new QLabel(tr("Doubleclick on item to remove:"),this);
	lbl_banner3->setAlignment(AlignHCenter|AlignVCenter);
	lbl_banner3->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_banner3->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lb_ingredients = new QListBox(this, "Buffer Components");
	lb_ingredients->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_ingredients->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	QString str1;
	for (i=0; i<component_list.size(); i++)
	{
		str1 = component_list[i].name + " (" + component_list[i].range + ")";
		lb_ingredients->insertItem(str1);
	}
	lb_ingredients->setSelected(0, TRUE);
	connect(lb_ingredients, SIGNAL(highlighted(int)), SLOT(list_component(int)));
	connect(lb_ingredients, SIGNAL(selected(int)), SLOT(list_component(int)));

	lb_current_buffer = new QListBox(this, "Buffer Components");
	lb_current_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_current_buffer->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_current_buffer, SIGNAL(selected(int)), SLOT(remove_component(int)));

	unsigned int rows=14, columns=3, margins=2, spacing=2;

	QGridLayout * Grid = new QGridLayout(this, rows, columns, margins, spacing);
	j=0;
	for (i=0; i<rows; i++)
	{
		Grid->setRowSpacing(i, 26);
	}
	Grid->addWidget(pb_load, j, 0);
	Grid->addWidget(pb_help, j, 1);
	Grid->addWidget(pb_accept, j, 2);
	j++;
	Grid->addWidget(pb_load_db, j, 0);
	Grid->addMultiCellWidget(pb_save, j, j, 1, 2);
	j++;
	Grid->addWidget(lbl_banner1, j, 0);
	Grid->addMultiCellWidget(pb_save_db, j, j, 1, 2);
	j++;
	Grid->addMultiCellWidget(lb_buffer_db, j, j+5, 0, 0);
	Grid->addMultiCellWidget(pb_update_db, j, j, 1, 2);
	j++;
	Grid->addMultiCellWidget(pb_del_db, j, j, 1, 2);
	j++;
	Grid->addMultiCellWidget(pb_reset, j, j, 1, 2);
	j++;
	Grid->addWidget(lbl_density, j, 1);
	Grid->addWidget(le_density, j, 2);
	j++;
	Grid->addWidget(lbl_viscosity, j, 1);
	Grid->addWidget(le_viscosity, j, 2);
	j++;
	Grid->addWidget(lbl_refractive_index, j, 1);
	Grid->addWidget(le_refractive_index, j, 2);
	j++;
	Grid->addWidget(pb_investigator, j, 0);
	Grid->addMultiCellWidget(lbl_investigator, j, j, 1, 2);
	j++;
	Grid->addWidget(lbl_description, j, 0);
	Grid->addMultiCellWidget(le_description, j, j, 1, 2);
	j++;
	Grid->addWidget(lbl_buffer1, j, 0);
	Grid->addMultiCellWidget(lbl_unit, j, j, 1, 2);
	j++;
	Grid->addWidget(lbl_buffer2, j, 0);
	Grid->addMultiCellWidget(le_concentration, j, j, 1, 2);
	j++;
	Grid->setRowSpacing(j, 30);
	Grid->addWidget(lbl_banner2, j, 0);
	Grid->addMultiCellWidget(lbl_banner3, j, j, 1, 2);
	j++;
	Grid->setRowSpacing(j, 200);
	Grid->addWidget(lb_ingredients, j, 0);
	Grid->addMultiCellWidget(lb_current_buffer, j, j, 1, 2);
	Grid->setColSpacing(2, 150);

	qApp->processEvents();
	QRect r = Grid->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

/*!
	Read buffer.dat file and get buffer component value.
*/
bool US_Buffer_DB::read_template_file()
{
	QString str;
	QFile f(USglobal->config_list.system_dir + "/etc/buffer.dat");
	unsigned int i, j, components;
	if(f.open(IO_ReadOnly | IO_Translate))
	{
		QTextStream ts(&f);
		str = ts.readLine();
		components = str.toUInt();
		component_list.resize(components);
		for (i=0; i<components; i++)
		{
			component_list[i].name = ts.readLine();
			component_list[i].unit = ts.readLine();
			for (j=0; j<6; j++)
			{
				ts >> component_list[i].dens_coeff[j];
			}
			component_list[i].range = ts.readLine();
			for (j=0; j<6; j++)
			{
				ts >> component_list[i].visc_coeff[j];
			}
			ts.readLine();
		}
		f.close();
	}
	else
	{
		str = tr("UltraScan can not find the buffer definition file:\n\n"
				+ USglobal->config_list.system_dir + "/etc/buffer.dat"
				+  "\n\nPlease re-install the buffer definition file.");

		QMessageBox::message(tr("Attention:"), str);
		close();
		return false;
	}
	return true;
}

/*! Load buffer data from Hard Drive, add warning message when this widget is called by US_Cell_DB */
void US_Buffer_DB::read_buffer()
{
	if(cell_flag)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Please use the 'Query Buffer from DB' button\n"
 										"to select a buffer file from the database.\n\n"
										"If the desired file is not stored in the database,\n"
										"you have to click on 'Backup Buffer to DB' first, store\n"
										"a buffer file to the database, then select it from the database."));
	}
	if (Buffer.investigatorID < 1)
	{
		lbl_investigator->setText(" Not Selected");
	}
	unsigned int i, counter;
	QString str1, str2, filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.buf", 0);
	if (!filename.isEmpty())
	{
		lb_current_buffer->clear();
		lb_buffer_db->clear();
		lb_buffer_db->insertItem(filename);
		lb_buffer_db->insertItem("Current Buffer was loaded");
		lb_buffer_db->insertItem("from Hard Drive");
		QFile f(filename);
		if (f.open(IO_ReadOnly))
		{
			QTextStream ts (&f);
			Buffer.description = ts.readLine();
			le_description->setText(Buffer.description);
			ts >> counter;
			ts.readLine();
			Buffer.component.resize(counter);
			ts >> Buffer.density;
			ts.readLine();
			ts >> Buffer.viscosity;
			ts.readLine();
			for (i=0; i<counter; i++)
			{
				Buffer.component[i].name = ts.readLine();
				ts >> Buffer.component[i].partial_concentration;
				ts.readLine();
				str1.sprintf(Buffer.component[i].name + " (%.1f mM)", Buffer.component[i].partial_concentration);
				lb_current_buffer->insertItem(str1);
			}
			if (!ts.atEnd())
			{
				ts >> Buffer.refractive_index;
				ts.readLine();
			}
			Buffer.bufferID = -1; // this buffer is from disk, we flag this with a negative ID
			lb_buffer_db->disconnect();
			if (Buffer.component.size() > 0 && Buffer.density == 0.0)
			{
				recalc_density();
			}
			if (Buffer.component.size() > 0 && Buffer.viscosity == 0.0)
			{
				recalc_viscosity();
			}
			if (Buffer.component.size() == 0 && Buffer.viscosity == 0.0)
			{
				Buffer.viscosity = (float) (100.0 * VISC_20W); // assume water
			}
			if (Buffer.component.size() == 0 && Buffer.density == 0.0)
			{
				Buffer.density = (float) DENS_20W; // assume water
			}
			le_density->disconnect();
			le_viscosity->disconnect();
			le_density->setText(str1.sprintf(" %6.4f ", Buffer.density));
			le_viscosity->setText(str1.sprintf(" %6.4f ", Buffer.viscosity));
			connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
			connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));
			le_refractive_index->setText(str1.sprintf(" %6.4f ", Buffer.refractive_index));
			f.close();
		}
		else
		{
			QMessageBox::message(tr("Attention:"), tr("UltraScan can not open the selected buffer file:\n\n" \
					+ filename +  "\n\nPlease check for read permission on drive."));
			return;
		}
		if(Buffer.component.size() > 0) // don't let the user override density and viscosity calculations
		{
			le_viscosity->setReadOnly(true);
			le_density->setReadOnly(true);
		}
		else
		{
			le_viscosity->setReadOnly(false);
			le_density->setReadOnly(false);
		}
	}
	emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
	emit valueChanged(Buffer.density, Buffer.viscosity);
}

/*! Save buffer data to disk drive in *.buf format. */
void US_Buffer_DB::save_buffer()
{
	unsigned int i;
	if (Buffer.description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
													  "your buffer before saving it!"));
		return;
	}
	QString filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.buf", 0);
	if (!filename.isEmpty())
	{
		if (filename.right(4) != ".buf")
		{
			filename.append(".buf");
		}
		switch(QMessageBox::information(this, tr("UltraScan - Buffer:"),
				tr("Click 'OK' to save buffer information to :\n" + filename),
				tr("OK"), tr("CANCEL"),	0, 1))
		{
			case 0:
			{
				QFile f(filename);
				if (f.open(IO_WriteOnly | IO_Translate))
				{
					QTextStream ts (&f);
					ts << Buffer.description << endl;
					ts << Buffer.component.size() << endl;
					ts << Buffer.density << endl;
					ts << Buffer.viscosity << endl;
					for (i=0; i<Buffer.component.size(); i++)
					{
						ts << Buffer.component[i].name << endl;
						ts << Buffer.component[i].partial_concentration << endl;
					}
					ts << Buffer.refractive_index;
					f.close();
				}
				else
				{
					QMessageBox::message(tr("Attention:"), tr("Unable to write the buffer to a file!\n"
						"Please check diskspace and make sure\nthe disk is not write-protected."));
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

/*! Load buffer data from database table tblBuffer and populate listbox. If
an investigator is defined, only select the buffer files from the investigator */
void US_Buffer_DB::read_db()
{
	pb_update_db->setEnabled(false);
	QString str;
	db_list.clear();
	lb_buffer_db->clear();
	struct BufferData temp_entry;
	if(Buffer.investigatorID > 0)
	{
		str.sprintf("SELECT BuffID, BufferDescription FROM tblBuffer WHERE InvestigatorID = %d ORDER BY BuffID DESC;", Buffer.investigatorID);
	}
	else
	{
		str.sprintf("SELECT BuffID, BufferDescription FROM tblBuffer ORDER BY BuffID DESC;");
	}
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			temp_entry.bufferID = query.value(0).toInt();
			temp_entry.description = query.value(1).toString();
			//cout << temp_entry.description << ", " << temp_entry.bufferID << endl;
			db_list.push_back(temp_entry);
			lb_buffer_db->insertItem(str.sprintf("%d: ", temp_entry.bufferID) + temp_entry.description);
		}
	}
	if (db_list.size() < 1 && Buffer.investigatorID > 0)
	{
		lb_buffer_db->insertItem(tr("No buffer file found for the"));
		lb_buffer_db->insertItem(tr("selected investigator,"));
		lb_buffer_db->insertItem(tr("You can click on \"Reset\","));
		lb_buffer_db->insertItem(tr("then query the DB to find buffers"));
		lb_buffer_db->insertItem(tr("from all Investigators."));
	}
	if (db_list.size() < 1)
	{
		lb_buffer_db->insertItem("There are no entries");
		lb_buffer_db->disconnect();
	}
	else
	{
		connect(lb_buffer_db, SIGNAL(selected(int)), SLOT(select_buff(int)));
//		connect(lb_buffer_db, SIGNAL(highlighted(int)), SLOT(select_buff(int)));
	}
	qApp->processEvents();
}

/*!
	If you find the Buffer name in the ListBox by read_buffer() or read_db().
	Doubleclick it, you will get all buffer data about this name.
	\param item The number of items in ListBox, count start from 0.
*/
void US_Buffer_DB::select_buff(int item)
{
	QString str;
	lb_current_buffer->clear();
	get_buffer(db_list[item].bufferID);
	lbl_investigator->setText(show_investigator(Buffer.investigatorID));
	le_description->setText(Buffer.description);
	le_density->disconnect();
	le_viscosity->disconnect();
	le_density->setText(str.sprintf(" %6.4f ", Buffer.density));
	le_viscosity->setText(str.sprintf(" %6.4f ", Buffer.viscosity));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
	connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));
	le_refractive_index->setText(str.sprintf(" %6.4f", Buffer.refractive_index));
	le_viscosity->setReadOnly(true);
	le_density->setReadOnly(true);
	for (unsigned int i=0; i<Buffer.component.size(); i++)
	{
		str.sprintf("%.1f ", Buffer.component[i].partial_concentration);
		lb_current_buffer->insertItem(Buffer.component[i].name + " (" + str + Buffer.component[i].unit +")");
	}
	pb_update_db->setEnabled(true); // allow modification of the just selected buffer from the DB
}

bool US_Buffer_DB::get_buffer(int id)
{
	QString str;
	if (!DB_flag)
	{
		str = tr("The database is not yet opened!\n"
				"us_buffer.cpp cannot read a buffer from the database unless it is opened first!");
		QMessageBox::message(tr("Attention:"), str);
		return false;
	}
	str.sprintf("SELECT InvestigatorID, BufferData, Density20, Viscosity20, RefractiveIndex20, BufferDescription FROM tblBuffer WHERE BuffID = %d;", id);
	QSqlQuery query(str);
	struct BufferIngredient temp_component;
	if(query.isActive())
	{
		if(query.next())
		{
			Buffer.component.clear();
			Buffer.investigatorID = query.value(0).toInt();
			Buffer.data = query.value(1).toString();
			Buffer.density = query.value(2).toString().toFloat();
			Buffer.viscosity = query.value(3).toString().toFloat();
			Buffer.refractive_index = query.value(4).toString().toFloat();
			Buffer.description = query.value(5).toString();
			Buffer.bufferID = id;
			vector <QString> sl;
			str = Buffer.data;
			sl.clear();
			while (str.length() > 0)
			{
				sl.push_back(getToken(&str, "\n"));
			}
			for (vector <QString>::iterator it = sl.begin() + 1; it != sl.end(); ++it)
			{
				temp_component.name = *it;
				++it;
				temp_component.partial_concentration = (*it).toFloat();
				temp_component.unit = "mM";
				Buffer.component.push_back(temp_component);
			}
			if (Buffer.component.size() > 0 && Buffer.density == 0.0)
			{
				recalc_density();
			}
			if (Buffer.component.size() > 0 && Buffer.viscosity == 0.0)
			{
				recalc_viscosity();
			}
			if (Buffer.component.size() == 0 && Buffer.viscosity == 0.0)
			{
				Buffer.viscosity = (float) (100.0 * VISC_20W); // assume water
			}
			if (Buffer.component.size() == 0 && Buffer.density == 0.0)
			{
				Buffer.density = (float) DENS_20W; // assume water
			}
		}
	}
	else
	{
		str.sprintf(tr("Failed to retrieve the buffer %d from the database.\n"), id);
		str += tr("Unable to locate the requested buffer file in the "
				+  login_list.dbname + " database.\n"
				"Please make sure you are connecting to the correct database!");
		QMessageBox::message(tr("Attention:"), str);
		return false;
	}
	emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
	emit valueChanged(Buffer.density, Buffer.viscosity);
	return true;
}

/*!
	Save all new buffer data to DB table: tblBuffer and exit.
*/
void US_Buffer_DB::save_db()
{
	QString str;
	if(Buffer.investigatorID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please select an investigator first!"));
		return;
	}
	if(Buffer.description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a buffer description first!"));
		return;
	}

	Buffer.data.sprintf("%d", (int) Buffer.component.size());
	for (unsigned int i=0; i<Buffer.component.size(); i++)
	{
		Buffer.data += "\n" + Buffer.component[i].name;
		Buffer.data += str.sprintf("\n%6.1f", Buffer.component[i].partial_concentration);
	}
	switch(QMessageBox::information(this, tr("UltraScan - Buffer Database:"),
			tr("Click 'OK' to record the selected\nbuffer file into the database"),
			tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
			QSqlQuery target;
			str.sprintf("INSERT INTO tblBuffer(BufferDescription, BufferData, Density20, Viscosity20, RefractiveIndex20, InvestigatorID) VALUES('" + Buffer.description + "', '" + Buffer.data + "',%f, %f, %f, %d);", Buffer.density, Buffer.viscosity, Buffer.refractive_index, Buffer.investigatorID);
			target.exec(str);
			break;
		}
		case 1:
		{
			break;
		}
	}
}

/*!
	Update changed buffer data to DB table: tblBuffer
*/
void US_Buffer_DB::update_db()
{
	QString str;
	if(Buffer.bufferID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please select an existing Buffer first!"));
		return;
	}
	if(Buffer.investigatorID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please select an investigator first!"));
		return;
	}
	if(Buffer.description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a buffer description first!"));
		return;
	}

	Buffer.data.sprintf("%d", (int) Buffer.component.size());
	for (unsigned int i=0; i<Buffer.component.size(); i++)
	{
		Buffer.data += "\n" + Buffer.component[i].name;
		Buffer.data += str.sprintf("\n%6.1f", Buffer.component[i].partial_concentration);
	}
	switch(QMessageBox::information(this, tr("UltraScan - Buffer Database:"),
			str.sprintf(tr("Click 'OK' to update buffer %d\nin the database"), Buffer.bufferID),
			tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
			QSqlQuery target;
			str.sprintf("UPDATE tblBuffer SET BufferDescription='" + Buffer.description + "', BufferData='" + Buffer.data + "', Density20=%f, Viscosity20=%f, RefractiveIndex20=%f, InvestigatorID=%d WHERE BuffID=%d;", Buffer.density, Buffer.viscosity, Buffer.refractive_index, Buffer.investigatorID, Buffer.bufferID);
			target.exec(str);
			break;
		}
		case 1:
		{
			break;
		}
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_Buffer_DB::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(del_db(bool)));
}

/*! Delete selected entry of DB table: <tt>tblBuffer</tt>. */
void US_Buffer_DB::del_db(bool permission)
{
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Permission denied"));
		return;
	}
	if(Buffer.bufferID == -1  || lb_buffer_db->currentItem() == -1)
	{
		QMessageBox::message(tr("Attention:"),
									tr("First select the buffer which you\n"
										"want to delete from the database"));
		return;
	}
	else
	{
		switch(QMessageBox::information(this, tr("Confirm: Do you really want to delete this entry?"),
			tr("Clicking 'OK' will delete the selected buffer data from the database."),
			tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				QSqlQuery del;
				QString str;
				str.sprintf("DELETE FROM tblBuffer WHERE BuffID = %d;", Buffer.bufferID);
				if(!del.exec(str))
				{
					QSqlError sqlerr = del.lastError();
					QMessageBox::message(tr("Attention:"),
                           tr("Delete failed.\n"
                              "Attempted to execute this command:\n\n"
                              + str + "\n\n"
                              "Causing the following error:\n\n")
                              + sqlerr.text());
				}
				read_db();
				lb_buffer_db->clearSelection();
				reset();
				break;
			}
			case 1:
			{
				break;
			}
		}
	}
}

/*!
	After you select the buffer component in <var>lb_ingredients</var>,
	this component will display in <var>lbl_buffer2</var> and wait for inputting value.
*/
void US_Buffer_DB::list_component(int item)
{
	lbl_buffer2->setText(lb_ingredients->text(item));
	lbl_unit->setText(tr("Please enter with units in: " + component_list[item].unit));
	le_concentration->setFocus();
}

/*!
	Input the value of component which you selected in <var>lb_ingredients</var>.
	After 'Return' key was pressed,
	this function will dispaly the selected component value in <var>lb_current_buffer</var>
	and recalculate the <var>density</var> and <var>viscosity</var>.
*/
void US_Buffer_DB::add_component()
{
	QString str;
	lb_buffer_db->clearSelection(); // we are modifying the buffer, nothing should be selected in the database list
	BufferIngredient temp_ingredient;
	bool flag;
	if(lb_ingredients->currentItem() < 0)
	{
		QMessageBox::message(tr("Attention:"),
									tr("First select a buffer component!\n"));
		return;
	}
	if(partial_concentration <= 0.0)
	{
		return;
	}
	flag = false;
	int current = lb_ingredients->currentItem();
	for (unsigned int i=0; i<Buffer.component.size(); i++) // find out if this inredient already exists, otherwise add a new component
	{
		if (component_list[current].name == Buffer.component[i].name)
		{
			Buffer.component[i].partial_concentration = partial_concentration; // simply update the partial concentration of the existing ingredient
			str.sprintf(Buffer.component[i].name + " (%.1f " + Buffer.component[i].unit + ")", partial_concentration);
			lb_current_buffer->changeItem(str, i); // update the listbox with the current values
			flag = true;
			break;
		}
	}
	if (!flag) // add a new ingredient/component to this buffer
	{
		temp_ingredient.partial_concentration = partial_concentration;
		temp_ingredient.range = component_list[current].range;
		temp_ingredient.unit = component_list[current].unit;
		temp_ingredient.name = component_list[current].name;
		for (unsigned int i=0; i<6; i++)
		{
			temp_ingredient.dens_coeff[i] = component_list[current].dens_coeff[i];
			temp_ingredient.visc_coeff[i] = component_list[current].visc_coeff[i];
		}
		Buffer.component.push_back(temp_ingredient);
		str.sprintf(Buffer.component[Buffer.component.size()-1].name
						+ " (%.1f " + Buffer.component[Buffer.component.size()-1].unit
						+ ")", partial_concentration);
		lb_current_buffer->insertItem(str);
	}
	recalc_density();
	recalc_viscosity();
	le_density->disconnect();
	le_viscosity->disconnect();
	le_density->setText(str.sprintf(" %6.4f ", Buffer.density));
	le_viscosity->setText(str.sprintf(" %6.4f ", Buffer.viscosity));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
	connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));
	emit valueChanged(Buffer.density, Buffer.viscosity);
	le_concentration->setText("");
	if(Buffer.component.size() > 0)
	{
		le_viscosity->setReadOnly(true);
		le_density->setReadOnly(true);
	}
	else
	{
		le_viscosity->setReadOnly(false);
		le_density->setReadOnly(false);
	}

}


/*!
	Double click the select item in <var>lb_current_buffer</var>,
	this function will remove the selected component and recalculate
	the <var>density</var> and <var>viscosity</var>.
*/
void US_Buffer_DB::remove_component(int i)
{
	QString str;
	Buffer.component.erase(Buffer.component.begin() + i);
	recalc_viscosity();
	recalc_density();
	le_density->disconnect();
	le_viscosity->disconnect();
	le_density->setText(str.sprintf(" %6.4f ", Buffer.density));
	le_viscosity->setText(str.sprintf(" %6.4f ", Buffer.viscosity));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
	connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));
	lb_current_buffer->removeItem(i);
	if(Buffer.component.size() == 0)
	{
		le_viscosity->setReadOnly(false);
		le_density->setReadOnly(false);
	}
	else
	{
		le_viscosity->setReadOnly(true);
		le_density->setReadOnly(true);
	}
}

/*!
	Update <var>partial_conc</var> with LineEdit input.
*/
void US_Buffer_DB::update_concentration(const QString &val)
{
	partial_concentration = val.toFloat();
}

/*!
	Update <var>description</var> with LineEdit input.
*/
void US_Buffer_DB::update_description(const QString &str)
{
	Buffer.description = str;
}

/*!
	Update <var>viscosity</var> with LineEdit input.
*/
void US_Buffer_DB::update_viscosity(const QString &str)
{
	Buffer.viscosity = str.toFloat();
}

/*!
	Update <var>density</var> with LineEdit input.
*/
void US_Buffer_DB::update_density(const QString &str)
{
	Buffer.density = str.toFloat();
}

/*!
	Update <var>refractive index</var> with LineEdit input.
*/
void US_Buffer_DB::update_refractive_index(const QString &str)
{
	Buffer.refractive_index = str.toFloat();
}

/*!Reset some variables to initialization. */
void US_Buffer_DB::reset()
{
	QString str;
	le_viscosity->setReadOnly(false);
	le_density->setReadOnly(false);
	lb_current_buffer->clear();
	buf_init();
	le_density->disconnect();
	le_viscosity->disconnect();
	le_density->setText(str.sprintf(" %6.4f ", Buffer.density));
	le_viscosity->setText(str.sprintf(" %6.4f ", Buffer.viscosity));
	connect(le_viscosity, SIGNAL(textChanged(const QString &)), this, SLOT(update_viscosity(const QString &)));
	connect(le_density, SIGNAL(textChanged(const QString &)), this, SLOT(update_density(const QString &)));
	emit valueChanged(Buffer.density, Buffer.viscosity);
	lbl_buffer2->setText("");
	lbl_unit->setText("");
	le_description->setText("");
	le_concentration->setText("");
	lbl_investigator->setText(" Not Selected");
}

/*! Emit buffer values and exit. */
void US_Buffer_DB::accept()
{
	if(cell_flag && Buffer.bufferID < 1)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Buffer definitions need to be selected from the database\n"
										"in order to be associated with the experimental data.\n\n"
										"If the desired file is not stored in the database,\n"
										"you have to click on 'Backup Buffer to DB' first, store\n"
										"a buffer file to the database, then select it from the database."));
		return;
	}
	if (Buffer.density != 0.0 && Buffer.viscosity != 0.0)
	{
		emit valueChanged(Buffer.density, Buffer.viscosity);
	}
	if (Buffer.density != 0.0 && Buffer.viscosity != 0.0 && Buffer.refractive_index != 0.0)
	{
		emit valueChanged(Buffer.density, Buffer.viscosity, Buffer.refractive_index);
	}
	if (Buffer.bufferID > 0)
	{
		emit IDChanged(Buffer.bufferID);
	}
	close();
}

/*! Open a netscape browser to load help page.*/
void US_Buffer_DB::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/hydro.html");
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_Buffer_DB::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_Buffer_DB::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
	investigator_dlg->exec();
	lb_current_buffer->clear();
	lbl_buffer2->setText("");
	le_description->setText("");
	le_density->setText(" 0.0");
	le_viscosity->setText(" 0.0");
	le_refractive_index->setText(" 0.0");
	Buffer.component.clear();
	read_db();
}

/*!
	Update display with the selected investigator information.
	\param Name a string variable for the investigator name.
	\param InvID an integer variable for the index from DB table: <tt>tblInvestigators</tt>.
*/
void US_Buffer_DB::update_investigator_lbl (QString Name, int InvID)
{
	Buffer.investigatorID = InvID;
	lbl_investigator->setText(Name);
	if(Name == "")
	{
		lbl_investigator->setText(" Not Selected");
	}
}

void US_Buffer_DB::setInvestigator(const int investigatorID) // if called without invID in constructor
{
	Buffer.investigatorID = investigatorID;
}

/*!
	Recalculate the density of the buffer based on the information in the template file
 */
void US_Buffer_DB::recalc_density()
{
	Buffer.density = (float) DENS_20W;
	for (unsigned int i=0; i<Buffer.component.size(); i++) // iterate over all components in this buffer
	{
		for (unsigned int j=0; j<component_list.size(); j++) // find the component in the Buffer database file
		{
			if(Buffer.component[i].name == component_list[j].name) // if we find it, assign the values
			{
				Buffer.component[i].unit = component_list[j].unit;
				Buffer.component[i].range = component_list[j].range;
				for (unsigned int k=0; k<6; k++)
				{
					Buffer.component[i].dens_coeff[k] = component_list[j].dens_coeff[k];
				}
				if (Buffer.component[i].unit == "mM" && Buffer.component[i].partial_concentration > 0.0)
				{
					Buffer.density += Buffer.component[i].dens_coeff[0]
					+ Buffer.component[i].dens_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 0.5)
					+ Buffer.component[i].dens_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration/1000
					+ Buffer.component[i].dens_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 2)
					+ Buffer.component[i].dens_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 3)
					+ Buffer.component[i].dens_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 4) - DENS_20W;
				}
				else if (Buffer.component[i].unit == "M" && Buffer.component[i].partial_concentration > 0.0)
				{
					Buffer.density += Buffer.component[i].dens_coeff[0]
					+ Buffer.component[i].dens_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 0.5)
					+ Buffer.component[i].dens_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration
					+ Buffer.component[i].dens_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 2)
					+ Buffer.component[i].dens_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration), (double) 3)
					+ Buffer.component[i].dens_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration), (double) 4) - DENS_20W;
				}
			}
		}
	}
}

/*!
	Recalculate the viscosity of the buffer based on the information in the template file
 */
void US_Buffer_DB::recalc_viscosity()
{
	Buffer.viscosity = (float) (100.0 * VISC_20W);
	for (unsigned int i=0; i<Buffer.component.size(); i++) // iterate over all components in this buffer
	{
		for (unsigned int j=0; j<component_list.size(); j++) // find the component in the Buffer database file
		{
			if(Buffer.component[i].name == component_list[j].name) // if we find it, assign the values
			{
				Buffer.component[i].unit = component_list[j].unit;
				Buffer.component[i].range = component_list[j].range;
				for (unsigned int k=0; k<6; k++)
				{
					Buffer.component[i].visc_coeff[k] = component_list[j].visc_coeff[k];
				}
				if (Buffer.component[i].unit == "mM")
				{
					Buffer.viscosity += Buffer.component[i].visc_coeff[0]
					+ Buffer.component[i].visc_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 0.5)
					+ Buffer.component[i].visc_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration/1000
					+ Buffer.component[i].visc_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 2)
					+ Buffer.component[i].visc_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 3)
					+ Buffer.component[i].visc_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration/1000), (double) 4) - (100.0 * VISC_20W);
				}
				else if (Buffer.component[i].unit == "M")
				{
					Buffer.viscosity += Buffer.component[i].visc_coeff[0]
					+ Buffer.component[i].visc_coeff[1] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 0.5)
					+ Buffer.component[i].visc_coeff[2] * 1e-2 *
					Buffer.component[i].partial_concentration
					+ Buffer.component[i].visc_coeff[3] * 1e-3 * pow((double) (Buffer.component[i].partial_concentration), (double) 2)
					+ Buffer.component[i].visc_coeff[4] * 1e-4 * pow((double) (Buffer.component[i].partial_concentration), (double) 3)
					+ Buffer.component[i].visc_coeff[5] * 1e-6 * pow((double) (Buffer.component[i].partial_concentration), (double) 4) - (100.0 * VISC_20W);
				}
			}
		}
	}
}
