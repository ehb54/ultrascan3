#include "../include/us_db_tbl_vbar.h"

//! Constructor
/*!
	Constractor a new <var>US_Vbar_DB</var> interface, which is a GUI wrapper for us_vbar.
	\param temp a float variable pass temperature value.
	\param temp_vbar a float point variable pass vbar value.
	\param temp_vbar20 a float point variable pass vbar20 value.
	\param temp_GUI <tt>true</tt> will show interface, <tt>false</tt> no interface show up and just pass value.
	\param from_cell <tt>true</tt> called from US_Cell_DB Table, <tt>false</tt> otherwise.
	\param parent Parent widget.
	\param name Widget name.
*/
US_Vbar_DB::US_Vbar_DB(float temp, float *temp_vbar, float *temp_vbar20, bool temp_GUI,
bool from_cell, int temp_InvID, QWidget *parent, const char *name) : US_DB(parent, name)
{
	temperature = temp;
	t1 = temp; t2 = *temp_vbar; t3 = *temp_vbar20;
	cell_flag = from_cell;
	vbar_info.PepID = -1;
	vbar_info.InvID = temp_InvID;
	sequence_loaded = false;
	select_flag = false;		// use for select query listbox
	from_HD = false;
	GUI = temp_GUI;
	if (GUI)
	{
		setup_GUI();
	}
}

//! Destructor
/*! destroy the <var>US_Vbar_DB</var>. */
US_Vbar_DB::~US_Vbar_DB()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*!
	If temp_GUI is <var>true</var>, This function will create an interface for US_Vbar_DB.
*/
void US_Vbar_DB::setup_GUI()
{
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	pb_filename = new QPushButton(tr("Load Peptide from HD"), this);
	Q_CHECK_PTR(pb_filename);
	pb_filename->setAutoDefault(false);
	pb_filename->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_filename->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_filename, SIGNAL(clicked()), SLOT(read_file()));

	pb_load_db = new QPushButton(tr("Query Peptide from DB"), this);
	Q_CHECK_PTR(pb_load_db);
	pb_load_db->setAutoDefault(false);
	pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_db, SIGNAL(clicked()), SLOT(read_db()));

	pb_enter = new QPushButton(tr("Enter Peptide"), this);
	Q_CHECK_PTR(pb_enter);
	pb_enter->setAutoDefault(false);
	pb_enter->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_enter->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_enter, SIGNAL(clicked()), SLOT(enter_pep()));

	pb_download = new QPushButton(tr("Download Sequence"), this);
	Q_CHECK_PTR(pb_download);
	pb_download->setAutoDefault(false);
	pb_download->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_download->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_download, SIGNAL(clicked()), SLOT(download()));

	pb_sequence = new QPushButton(tr("View Sequence"), this);
	Q_CHECK_PTR(pb_download);
	pb_sequence->setAutoDefault(false);
	pb_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_sequence->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_sequence, SIGNAL(clicked()), SLOT(show_sequence()));

	pb_save_db = new QPushButton(tr("Save Peptide to DB"), this);
	Q_CHECK_PTR(pb_save_db);
	pb_save_db->setAutoDefault(false);
	pb_save_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_db, SIGNAL(clicked()), SLOT(save_db()));

	instr_lbl = new QLabel(tr("Doubleclick on peptide data to select:"),this);
	instr_lbl->setAlignment(AlignHCenter|AlignVCenter);
	instr_lbl->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	instr_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize-2, QFont::Bold));

	pb_del_db = new QPushButton(tr("Delete Peptide from DB"), this);
	Q_CHECK_PTR(pb_del_db);
	pb_del_db->setAutoDefault(false);
	pb_del_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_del_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_del_db, SIGNAL(clicked()), SLOT(check_permission()));

	pb_help = new QPushButton(tr("Help"), this);
	Q_CHECK_PTR(pb_help);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	pb_ok = new QPushButton(tr("Accept"), this);
	Q_CHECK_PTR(pb_ok);
	pb_ok->setAutoDefault(false);
	pb_ok->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_ok->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_ok, SIGNAL(clicked()), SLOT(ok()));

	lb_vbar = new QListBox(this, "Peptide files");
	lb_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_vbar->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_vbar, SIGNAL(selected(int)), SLOT(select_vbar(int)));

	pb_reset = new QPushButton(tr("Reset"), this);
	Q_CHECK_PTR(pb_reset);
	pb_reset->setAutoDefault(false);
	pb_reset->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_reset->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_reset, SIGNAL(clicked()), SLOT(reset()));

	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	pb_investigator->setAutoDefault(false);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	if(vbar_info.InvID>0)
	{
		pb_investigator->setEnabled(false);
	}
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	lbl_investigator= new QLabel("",this);
	lbl_investigator->setAlignment(AlignLeft|AlignVCenter);
	lbl_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	if(vbar_info.InvID>0)
	{
		lbl_investigator->setText(show_investigator(vbar_info.InvID));
	}
	else
	{
		lbl_investigator->setText(tr(" Not Selected"));
	}

	lbl_file1 = new QLabel(tr(" Selected File:"),this);
	lbl_file1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_file1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_file2 = new QLabel(tr(" No File Selected..."), this);
	lbl_file2->setAlignment(AlignLeft|AlignVCenter);
	lbl_file2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_file2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	lbl_desc1 = new QLabel(tr(" Protein Description:"),this);
	lbl_desc1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_desc1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_desc2 = new QLabel(tr(" Not Selected"), this);
	lbl_desc2->setAlignment(AlignLeft|AlignVCenter);
	lbl_desc2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_desc2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));


	lbl_temp1 = new QLabel(tr(" Temperature (in ºC):"),this);
	lbl_temp1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_temp1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));


	QString temp_str;
	temp_str.sprintf("%5.2f", t1);
	le_temperature = new QLineEdit(this);
	le_temperature->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_temperature->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	le_temperature->setText(temp_str);
	connect(le_temperature, SIGNAL(textChanged(const QString &)), SLOT(update_temp(const QString &)));
	connect(le_temperature, SIGNAL(returnPressed()), SLOT(update_vbar()));

	pb_info = new QPushButton(tr("More Information"), this);
	Q_CHECK_PTR(pb_info);
	pb_info->setAutoDefault(false);
	pb_info->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_info->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_info, SIGNAL(clicked()), SLOT(info()));


	lbl_vbar1 = new QLabel(tr(" vbar (temperature):"),this);
	lbl_vbar1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	QString str;
	lbl_vbar2 = new QLabel(this);
	lbl_vbar2->setText(str.sprintf("%7.5f ccm/g", t2));
	lbl_vbar2->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_vbar2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	lbl_vbar20_1 = new QLabel(tr(" vbar (20º C):"),this);
	lbl_vbar20_1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar20_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_vbar20_2 = new QLabel(this);
	lbl_vbar20_2->setText(str.sprintf("%7.5f ccm/g", t3));
	lbl_vbar20_2->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar20_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_vbar20_2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	lbl_e280_1 = new QLabel(tr(" E280 (denatured):"),this);
	lbl_e280_1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_e280_1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_e280_2 = new QLabel(this);
	lbl_e280_2->setText(str.sprintf("%7.1f OD/(mol*cm)", 1.0));
	lbl_e280_2->setAlignment(AlignLeft|AlignVCenter);
	lbl_e280_2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_e280_2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	lbl_resid1 = new QLabel(tr(" # of Residues:"),this);
	lbl_resid1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_resid1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_resid2 = new QLabel(tr(" Not Selected"), this);
	lbl_resid2->setAlignment(AlignLeft|AlignVCenter);
	lbl_resid2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_resid2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	lbl_mw1 = new QLabel(tr(" Molecular Weight:"),this);
	lbl_mw1->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_mw1->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	lbl_mw2 = new QLabel(tr(" Not Selected"), this);
	lbl_mw2->setAlignment(AlignLeft|AlignVCenter);
	lbl_mw2->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lbl_mw2->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, 0, 0);
	setup_GUI2();
}
void US_Vbar_DB::setup_GUI2()
{;
	int j=0;

	QGridLayout * background = new QGridLayout(this,2,1,2);
	QGridLayout * subGrid1 = new QGridLayout(8, 1, 2);
	for (int i=0; i<8; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addWidget(pb_filename,j,0);
	j++;
	subGrid1->addWidget(pb_load_db,j,0);
	j++;
	subGrid1->addWidget(instr_lbl,j,0);
	j++;
	subGrid1->addMultiCellWidget(lb_vbar,j,j+5,0,0);
	subGrid1->setRowStretch(j,5);

	j=0;
	QGridLayout * subGrid2 = new QGridLayout(8,1,2);
	for (int i=0; i<8; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}
	QBoxLayout *button=new QHBoxLayout(2);
	button->addWidget(pb_help);
	button->addWidget(pb_ok);
	subGrid2->addItem(button,j,0);
	j++;
	subGrid2->addWidget(pb_enter,j,0);
	j++;
	subGrid2->addWidget(pb_save_db,j,0);
	j++;
	subGrid2->addWidget(pb_del_db,j,0);
	j++;
	subGrid2->addWidget(pb_download,j,0);
	j++;
	subGrid2->addWidget(pb_sequence,j,0);
	j++;
	subGrid2->addWidget(pb_info,j,0);
	j++;
	subGrid2->addWidget(pb_reset,j,0);
	j++;
	subGrid2->setRowStretch(j,5);

	j=0;
	QGridLayout * subGrid3 = new QGridLayout(9, 2, 2);
	for (int i=0; i<9; i++)
	{
		subGrid3->setRowSpacing(i, 26);
	}

	subGrid3->addWidget(pb_investigator,j,0);
	subGrid3->addWidget(lbl_investigator,j,1);
	j++;
	subGrid3->addWidget(lbl_file1,j,0);
	subGrid3->addWidget(lbl_file2,j,1);
	j++;
	subGrid3->addWidget(lbl_desc1,j,0);
	subGrid3->addWidget(lbl_desc2,j,1);
	j++;
	subGrid3->addWidget(lbl_temp1,j,0);
	subGrid3->addWidget(le_temperature,j,1);
	j++;
	subGrid3->addWidget(lbl_vbar1,j,0);
	subGrid3->addWidget(lbl_vbar2,j,1);
	j++;
	subGrid3->addWidget(lbl_vbar20_1,j,0);
	subGrid3->addWidget(lbl_vbar20_2,j,1);
	j++;
	subGrid3->addWidget(lbl_e280_1,j,0);
	subGrid3->addWidget(lbl_e280_2,j,1);
	j++;
	subGrid3->addWidget(lbl_resid1,j,0);
	subGrid3->addWidget(lbl_resid2,j,1);
	j++;
	subGrid3->addWidget(lbl_mw1,j,0);
	subGrid3->addWidget(lbl_mw2,j,1);

	background->addLayout(subGrid1,0,0);
	background->addLayout(subGrid2,0,1);
	background->addMultiCellLayout(subGrid3,1,1,0,1);


	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());

}

/*! Load peptide data from Hard Drive, add warning message when this widget is called by US_Cell_DB. */
void US_Vbar_DB::read_file()
{
	QString str, newstr, test, sequence;
	sequence_loaded = true;
	vbar_info.e280 = 0.0;
	vbar_info.vbar = 0.0;

	if(cell_flag)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Please use the 'Query Peptide from DB' button\n"
 										"to select a peptide file from the database.\n\n"
										"If the desired sequence is not stored in the database,\n"
										"you have to click on 'Save Peptide to DB' first, store\n"
										"a sequence to the database, then select it from the database."));

	}
	filename = QFileDialog::getOpenFileName(USglobal->config_list.root_dir, "*.pep", 0);
	vbar_info.PepFileName = filename;
	vbar_info.PepFileName = vbar_info.PepFileName.remove(0,(vbar_info.PepFileName.findRev("/", -1,false)+1));
	if ( !filename.isEmpty() && GUI)
	{
		lb_vbar->clear();
		lb_vbar->insertItem("Showing Data from Harddrive:");
		lbl_file2->setText(vbar_info.PepFileName);
		QFile fr(filename);
		fr.open(IO_ReadOnly);
		QTextStream ts (&fr);
		while (test != "DE")
		{
			ts >> test;
			if (fr.atEnd())
			{
				QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
												 "The description identifier \"DE\" is missing!\n\n"
												 "Please fix this error and try again..."));
				return;
			}
		}
		newstr = ts.readLine();
		vbar_info.Description = newstr.stripWhiteSpace();
		if(GUI)
		{
			lbl_desc2->setText(vbar_info.Description);
		}
		while (test != "SQ")
		{
			ts >> test;
			if (fr.atEnd())
			{
				QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
														"The sequence identifier \"SQ\" is missing!\n\n"
														"Please fix this error and try again..."));
				return;
			}
		}
		ts.readLine();
		sequence = "";
		test = sequence.copy();
		while (test != "//")
		{
			sequence.append(test);
			test = ts.readLine();
			if (fr.atEnd() && test.stripWhiteSpace() != "//")
			{
				QMessageBox::message(tr("Attention:"), tr("The peptide file is not in the proper format!\n"
												  "The sequence end identifier \"//\" is missing!\n\n"
													"Please fix this error and try again..."));
				return;
			}
		}
		if (!fr.atEnd())
		{
			test = ts.readLine();
			vbar_info.vbar = test.toFloat();
		}
		if (!fr.atEnd())
		{
			test = ts.readLine();
			vbar_info.e280 = test.toFloat();
		}
		fr.close();
		vbar_info.Sequence = sequence;
		calc_vbar(&pep, &sequence, &temperature);
		if (vbar_info.e280 > 0.0)
		{
			pep.e280 = vbar_info.e280;
		}
		else
		{
			vbar_info.e280 = pep.e280;
		}
		if (vbar_info.vbar > 0.0)
		{
			pep.vbar20 = vbar_info.vbar;
			pep.vbar = adjust_vbar20(pep.vbar20, temperature);
		}
		else
		{
			vbar_info.vbar = pep.vbar20;
		}
		str = filename;
		str.truncate(str.find("."));
		int pos = str.findRev("/");
		str = str.remove(0,pos+1);
		res_file = USglobal->config_list.result_dir +"/" + str + ".pep_res";
		result_output(res_file);
		from_HD = true;
	}
	this->raise();
}
/*!
	Update <var>temperature</var> with the argument <var>str</var>.
*/
void US_Vbar_DB::update_temp(const QString &str)
{
	if (sequence_loaded)
	{
		temperature = str.toFloat();
	}
	else
	{
		temperature = 20.0;
	}
}

/*!
	Update <var>pep.vbar</var>by recalculating <var>pep.vbar20</var> and <var>temperature</var>.
*/
void US_Vbar_DB::update_vbar()
{
	if(GUI)
	{
		QString test;
		test.sprintf("%5.2f", temperature);
		le_temperature->setText(test);
		if (sequence_loaded)
		{
			pep.vbar = adjust_vbar20(pep.vbar20, temperature);
			lbl_vbar2->setText(test.sprintf("%7.5f ccm/g", pep.vbar));
	//		emit valueChanged(pep.vbar, pep.vbar20);
		}
		else
		{
			pep.vbar = 0.72;
			lbl_vbar2->setText(test.sprintf("%7.5f ccm/g", pep.vbar));
	//		emit valueChanged(pep.vbar, pep.vbar20);
		}
	}
	else
	return;
}

/*! Open a netscape browser to load help page.*/
void US_Vbar_DB::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/vbar.html");
}

/*! Open a netscape browser to download website.*/
void US_Vbar_DB::download()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/download_pep.html");
}
/*! Open a text edit for showing peptide information.*/
void US_Vbar_DB::info()
{
	if (filename.isEmpty())
	{
		QMessageBox::message(tr("Attention:\n"),
					tr("You need to load a peptide sequence file first!\n\nClick on \"Load Peptide from HD or from DB\"\n"));
		return;
	}
	else
	{
		//view_file(res_file);
		TextEdit *e;
		e = new TextEdit();
		e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
		e->load(res_file);
		e->show();
	}
}

/*! Open a text edit for showing sequence.*/
void US_Vbar_DB::show_sequence()
{
	if(from_HD||select_flag)
	{
		QString str, str_sequence;
		TextEdit *e;
		e = new TextEdit();
		e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
		if(from_HD)			//from HD
		{
			TextEdit *e;
			e = new TextEdit();
			e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
			e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);

			e->load(filename);
			e->show();
		}
		if(select_flag)	//from DB
		{
			str.sprintf("SELECT Sequence FROM tblPeptide WHERE PepID = %d;", vbar_info.PepID);
			QSqlQuery query(str);
			if(query.isActive())
			{
				if(query.next())
				{
					str_sequence = query.value(0).toString();
				}

				TextEdit *e;
				e = new TextEdit();
				e->setPalette(QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
				e->setGeometry(global_Xpos + 30, global_Ypos + 30, 685, 600);
				e->load_text(str_sequence);
				e->show();

			}
		}
	}
	else
	{
		QMessageBox::message(tr("Attention:\n"),
					tr("You need to load a peptide sequence file first!\n\nClick on \"Load Peptide from HD or from DB\"\n"));
		return;
	}

}

/*! Load peptide data from database table: tblPeptide. */
void US_Vbar_DB::read_db()
{
	QString str;
	int maxID = get_newID("tblPeptide", "PepID");
	int count = 0;
	item_PepID = new int[maxID];
	item_Description = new QString[maxID];
	display_Str = new QString[maxID];
	if(vbar_info.InvID >0)
	{
		str.sprintf("SELECT PepID, Description FROM tblPeptide WHERE InvestigatorID = %d ORDER BY PepID DESC;", vbar_info.InvID);
	}
	else
	{
		str.sprintf("SELECT PepID, Description FROM tblPeptide ORDER BY PepID DESC;");

	}
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			item_PepID[count] = query.value(0).toInt();
			item_Description[count] = query.value(1).toString();
			display_Str[count] = "PepID ("+  QString::number( item_PepID[count] ) + "): "+item_Description[count];
			count++;
		}
	}
	if(GUI)
	{
		lb_vbar->clear();
		if(count>0)
		{
			for( int i=0; i<count; i++)
			{
				lb_vbar->insertItem(display_Str[i]);
			}
			select_flag = true;
		}
		else
		{
			lb_vbar->insertItem("No peptide file found for the selected investigator,");
			lb_vbar->insertItem("You can 'Reset' then query DB to list all Peptide files.");
		}
	}
}

/*!
	If you find the Peptide name in the ListBox by read_file() or read_db().
	Doubleclick it, you will get all vbar data about this name.
	\param item The number of items in ListBox, count start from 0.
*/
void US_Vbar_DB::select_vbar(int item)
{
	if(select_flag && GUI)
	{
		sequence_loaded = true;
		Item = item;
		vbar_info.PepID = item_PepID[item];
		lbl_file2->setText("");
		emit idChanged(vbar_info.PepID);

		retrieve_vbar(vbar_info.PepID);
	}
	else if (GUI)
	{
		QMessageBox::message(tr("Attention:"),
									tr("No available peptide files\n"));
	}

}

void US_Vbar_DB::retrieve_vbar(int PepID)
{
	if(PepID<=0)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Peptide tableid Error\n"));
		return;
	}
	QString str, sequence;
	str.sprintf("SELECT Description, Sequence, InvestigatorID, vbar, e280 FROM tblPeptide WHERE PepID = %d", PepID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			vbar_info.Description = query.value(0).toString();
			sequence = query.value(1).toString();
			vbar_info.InvID = query.value(2).toInt();
			vbar_info.vbar = (float) query.value(3).toDouble();
			vbar_info.e280 = (float) query.value(4).toDouble();
			calc_vbar(&pep, &sequence, &temperature);
			if (vbar_info.e280 > 0.0)
			{
				pep.e280 = vbar_info.e280;
			}
			else
			{
				vbar_info.e280 = pep.e280;
			}
			if (vbar_info.vbar > 0.0)
			{
				pep.vbar20 = vbar_info.vbar;
				pep.vbar = adjust_vbar20(pep.vbar20, temperature);
			}
			else
			{
				vbar_info.vbar = pep.vbar20;
			}
		}
	}
	lbl_desc2->setText(vbar_info.Description);
	lbl_investigator->setText(show_investigator(vbar_info.InvID));
	pb_investigator->setEnabled(false);
	//filename.truncate(filename.find("."));
	filename = QString::number(vbar_info.PepID);
	res_file = USglobal->config_list.result_dir +"/" + filename + ".pep_res";
	result_output(res_file);
	from_HD = false;
}

/*!
	Save all new peptide data to DB table: tblPeptide and exit.
*/
void US_Vbar_DB::save_db()
{
	if(vbar_info.InvID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
		return;
	}

	switch(QMessageBox::information(this, tr("UltraScan - Peptide Database:"), tr("Store this peptide info into the database?"),
										tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
//			int newPepID = get_newID("tblPeptide", "PepID");
			QSqlQuery target;
			QString str;
//			str.sprintf("INSERT INTO tblPeptide(PepID, PepFileName, Description, Sequence, InvestigatorID) VALUES(%d, '"
//						+ vbar_info.PepFileName + "', '" + vbar_info.Description +"', '"+ vbar_info.Sequence + "', %d);",
//							newPepID, vbar_info.InvID);
				str.sprintf("INSERT INTO tblPeptide(PepFileName, Description, Sequence, InvestigatorID) VALUES('"
							+ vbar_info.PepFileName + "', '" + vbar_info.Description +"', '"+ vbar_info.Sequence + "', %d);", vbar_info.InvID);

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
void US_Vbar_DB::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(delete_db(bool)));
}

/*! Delete selected entry of DB table: <tt>tblPeptide</tt>. */
void US_Vbar_DB::delete_db(bool permission)
{
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Permission denied"));
		return;
	}
	int j = Item;
	int id = vbar_info.PepID;
	if(id == -1)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Please select an entry which you\n"
										"want to be deleted from the database"));

	}
	else
	{
	/*
		QSqlCursor cur( "tblPeptide");
		cur.setMode( QSqlCursor::Delete);
		QSqlIndex filter = cur.index("PepID");
		cur.setValue("PepID", id);
		cur.select(filter);
		cur.primeDelete();
	*/
		switch(QMessageBox::information(this, tr("Attention:"),
										tr("Clicking 'OK' will delete the selected sequence from the database"),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				//cur.del();
				QSqlQuery del;
				QString str;
				str.sprintf("DELETE FROM tblPeptide WHERE PepID = %d;", id);
				bool done=del.exec(str);
				if(!done)
				{
					QSqlError sqlerr = del.lastError();
					QMessageBox::message(tr("Attention:"),
                           tr("Delete failed.\n"
                              "Attempted to execute this command:\n\n"
                              + str + "\n\n"
                              "Causing the following error:\n\n")
                              + sqlerr.text());
				}

				if(GUI)
				{
					lb_vbar->removeItem(j);
				}
				clear();
				break;
			}
			case 1:
			{
				break;
			}
		}
	}
}

/*! Initialize all variables. */
void US_Vbar_DB::clear()
{
	if(GUI)
	{
		QString str;
		pb_investigator->setEnabled(true);
		lbl_investigator->setText(tr(" Not Selected"));
		lbl_file2 ->setText(tr(" No File Selected..."));
		lbl_desc2 ->setText(tr(" Not Selected"));
		le_temperature->setText(str.sprintf("%5.2f",t1));
		lbl_vbar2->setText(str.sprintf("%7.5f ccm/g", t2));
		lbl_vbar20_2->setText(str.sprintf("%7.5f ccm/g",t3));
		lbl_e280_2->setText(str.sprintf("%7.1f OD/(mol*cm)", 1.0));
		lbl_resid2->setText(tr("Not Selected"));
		lbl_mw2->setText(tr("Not Selected"));
	}
	vbar_info.PepID = -1;
	vbar_info.InvID = -1;
	vbar_info.PepFileName = "";
	vbar_info.Description = "";
	vbar_info.Sequence = "";
	emit idChanged(vbar_info.PepID);
	select_flag = false;
	from_HD = false;
}
/*! Reset all variables to start values. */
void US_Vbar_DB::reset()
{
	lb_vbar->clear();
	filename = "";
	temperature = t1;
	pep.vbar = t2;
	pep.vbar20 = t3;
	clear();
}

/*! Add warning message when peptide data is not loaded from DB and emit <var>PepID</var> = -1 to US_Cell_DB. */
void US_Vbar_DB::ok()
{
	if(cell_flag && from_HD)
	{
		emit idChanged(-1);
		QMessageBox::message(tr("Attention:"),
									tr("Selecting a peptide file from the harddrive will not\n"
										"associate the data from this channel with this file.\n\n"
										"In order to associate a peptide file with your data,\n"
										"you need to select it from the database."));
		return;
	}
	accept();
}
/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_Vbar_DB::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*!
	Export the struct peptideDetails values according to Peptide <var>id</var>.
*/
struct peptideDetails US_Vbar_DB::export_vbar(int id)
{
	//QString newstr, test, sequence;
	QString sequence;
	QSqlCursor cur( "tblPeptide" );
	QString chk_pepid;
	chk_pepid.sprintf("PepID = %d",id);
	cur.select(chk_pepid);
	if(cur.next())
	{
	//	int size = cur.value("File").toByteArray().size();
	//	QByteArray da(size);
	//	da = cur.value("File").toByteArray();
	//	sequence = get_sequence(da);
		sequence = cur.value("Sequence").toString();
		calc_vbar(&pep, &sequence, &temperature);
		Vbar.vbar = pep.vbar;
		Vbar.vbar20 = pep.vbar20;
		Vbar.e280 = pep.e280;
		QString res_file = USglobal->config_list.result_dir +"/" + QString::number(id) + ".pep_res";
		result_output(res_file);
	}
	else
	{
		QString str = tr("Unable to locate the requested peptide file in the ");
		str.append(login_list.dbname);
		str.append(" database. Please make sure you are connecting\nto the correct database!");
		QMessageBox::message(tr("Attention:"), str);
		Vbar.vbar = -1;
		Vbar.vbar20 = -1;
		Vbar.e280 = -1;
	}
	return Vbar;
}
/*!
	Extract sequence from peptide file.
	/param filename the source file.
	/param flag <tt>true</tt> will remove the source file after getting sequence, <tt>false</tt> otherwise.
	/param sequence for pure sequence.
*/
/*
QString US_Vbar_DB::get_sequence(QByteArray da)
{
	QString file, sequence = "";
	int DE_index, SQ_index, End_index;

	for(unsigned int i=0; i<da.size(); i++)
	{
		file[i] = da[i];
	}
	DE_index = file.find("DE",0, true);
	if(DE_index<0)
	{
		QMessageBox::message(tr("Attention:"), tr("The database peptide file is not in the correct format\n"
												 "The description identifier \"DE\" is missing!\n\n"
												 "Please fix this error and try again..."));
		return(sequence);

	}
	else
	{
		file.remove(0, file.find("DE")+3);
		description = file.left(file.find("\n"));
		description.stripWhiteSpace();
		if(GUI)
		{
			lbl_desc2->setText(description);
		}
	}
	SQ_index = file.find("SQ", 0, true);
	if(SQ_index<0)
	{
		QMessageBox::message(tr("Attention:"), tr("The database peptide file is not in the correct format\n"
														"The sequence identifier preceded by \"SQ\" is missing!\n\n"
														"Please fix this error and try again..."));
				return(sequence);
	}
	else
	{
		file.remove(0, file.find("SQ")+3);
		End_index = file.find("//");
		if(End_index<0)
		{
			QMessageBox::message(tr("Attention:"), tr("The database peptide file is not in the correct format\n"
												  "The sequence end identifier \"//\" is missing!\n\n"
													"Please fix this error and try again..."));
					return(sequence);
		}
		else
		{
			sequence = file.left(file.find("//"));
			sequence.stripWhiteSpace();
			return (sequence);
		}
	}
	return(sequence);
}
*/
/*!
	Write the peptide analysis results to a text format display.
*/
void US_Vbar_DB::result_output(QString res_file)
{
	QString tempstr;
	tempstr.sprintf("%5.3f", temperature);
	QFile result(res_file);
	result.open(IO_WriteOnly | IO_Translate);
	QTextStream  res_io(&result);
	res_io << "***************************************************\n";
	res_io << tr("*            Peptide Analysis Results             *\n");
	res_io << "***************************************************\n\n\n";
	res_io << tr("Report for:         ") << description << "\n\n";
	res_io << tr("Number of Residues: ") << pep.residues << " AA\n";
	res_io << tr("Molecular Weight:   ") << pep.mw << tr(" Dalton\n");
	res_io << tr("V-bar at 20 ºC:     ") << pep.vbar20 << " ccm/g\n";
	res_io << tr("V-bar at ") << tempstr << " ºC: " << pep.vbar << " ccm/g\n";
	res_io << tr("Extinction coefficient for the denatured\npeptide at 280 nm: ") << pep.e280 << " OD/(mol*cm)\n\n";
	res_io << tr("Composition: \n\n");
	res_io << tr("Alanine:\t") << pep.a << tr("\tArginine:\t") << pep.r << "\n";
	res_io << tr("Asparagine:\t") << pep.n << tr("\tAspartate:\t") << pep.d << "\n";
	res_io << tr("Asparagine or\nAspartate:\t") << pep.b << "\n";
	res_io << tr("Cysteine:\t") << pep.c << tr("\tGlutamate:\t") << pep.e << "\n";
	res_io << tr("Glutamine:\t") << pep.q << tr("\tGlycine:\t") << pep.g << "\n";
	res_io << tr("Glutamine or\nGlutamate:\t") << pep.z << "\n";
	res_io << tr("Histidine:\t") << pep.h << tr("\tIsoleucine:\t") << pep.i << "\n";
	res_io << tr("Leucine:\t") << pep.l << tr("\tLysine:\t\t") << pep.k << "\n";
	res_io << tr("Methionine:\t") << pep.m << tr("\tPhenylalanine:\t") << pep.f << "\n";
	res_io << tr("Proline:\t") << pep.p << tr("\tSerine:\t\t") << pep.s << "\n";
	res_io << tr("Threonine:\t") << pep.t << tr("\tTryptophan:\t") << pep.w << "\n";
	res_io << tr("Tyrosine:\t") << pep.y << tr("\tValine:\t\t") << pep.v << "\n";
	res_io << tr("Unknown:\t") << pep.x << tr("\tHao:\t\t") << pep.j << "\n";
	res_io << tr("Delta-linked Ornithine:\t") << pep.o << endl;
	result.close();
	emit valueChanged(pep.vbar, pep.vbar20);
	emit e280Changed(pep.e280);
	emit mwChanged(pep.mw);
	if(GUI)
	{
		lbl_vbar2->setText(tempstr.sprintf("%7.5f ccm/g", pep.vbar));
		lbl_vbar20_2->setText(tempstr.sprintf("%7.5f ccm/g", pep.vbar20));
		lbl_resid2->setText(tempstr.sprintf("%d AA", pep.residues));
		lbl_e280_2->setText(tempstr.sprintf("%7.1f OD/(mol*cm)", pep.e280));
		lbl_mw2->setText(tempstr.sprintf("%6.4e Dalton", pep.mw));
	}
}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_Vbar_DB::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
	investigator_dlg->exec();
}

/*!
	Update display with the selected investigator information.
	\param Display a string variable for show investigator info.
	\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_Vbar_DB::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	vbar_info.InvID = InvID;
	str = Display;
	lbl_investigator->setText(str);
	if(str == "")
	{
		lbl_investigator->setText(" Not Selected");
	}
}

void US_Vbar_DB::enter_pep ()
{
	if(vbar_info.InvID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
		return;
	}
	enterPepDlg = new US_Enter_Vbar_DB(vbar_info.InvID);
	enterPepDlg->exec();
}


/**************************   Class US_Enter_Vbar_DB    *******************************/
US_Enter_Vbar_DB::US_Enter_Vbar_DB(int temp_InvID, QWidget *parent, const char *name) : US_DB(parent, name)
{

	InvID = temp_InvID;
	filename = "";
	description = "";
	sequence = "";
	vbar = 0.0;
	e280 = 0.0;
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));

	lbl_bar = new QLabel(tr(" Enter Peptide Information:"),this);
	lbl_bar->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_bar->setAlignment(AlignHCenter|AlignVCenter);
	lbl_bar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize , QFont::Bold));

	lbl_description = new QLabel(tr(" Peptide Description:"),this);
	lbl_description->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_description = new QLineEdit("", this);
	le_description->setAlignment(AlignLeft|AlignVCenter);
	le_description->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_description->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	connect(le_description, SIGNAL(textChanged(const QString &)), SLOT(update_description(const QString &)));

	lbl_vbar = new QLabel(tr(" Peptide vbar (optional):"),this);
	lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_vbar = new QLineEdit("", this);
	le_vbar->setAlignment(AlignLeft|AlignVCenter);
	le_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	le_vbar->setText("0.0");
	connect(le_vbar, SIGNAL(textChanged(const QString &)), SLOT(update_vbar(const QString &)));

	lbl_e280 = new QLabel(tr(" Peptide E280 (optional):"),this);
	lbl_e280->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_e280 = new QLineEdit("", this);
	le_e280->setAlignment(AlignLeft|AlignVCenter);
	le_e280->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_e280->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	le_e280->setText("0.0");
	connect(le_e280, SIGNAL(textChanged(const QString &)), SLOT(update_e280(const QString &)));

	QString str1;
	str1 = tr(" Peptide Sequence:\n"
				 " Please Note: If the vbar \n"
				 " or E280 value should be \n"
				 " determined from sequence,\n"
				 " the vbar or E280 value \n"
				 " above should be zero ");
	lbl_sequence = new QLabel(str1, this);
	lbl_sequence->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));

	le_sequence = new QTextEdit(this, "sequence_editor");
	le_sequence->setAlignment(AlignLeft|AlignVCenter);
	le_sequence->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sequence->setPalette( QPalette(USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit,USglobal->global_colors.cg_edit));
	connect(le_sequence, SIGNAL(textChanged()), SLOT(update_sequence()));

	pb_save_HD = new QPushButton(tr("Save to Hard Drive"), this);
	Q_CHECK_PTR(pb_save_HD);
	pb_save_HD->setAutoDefault(false);
	pb_save_HD->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_HD->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_HD, SIGNAL(clicked()), SLOT(save_HD()));

	pb_save_DB = new QPushButton(tr("Save to Database"), this);
	Q_CHECK_PTR(pb_save_DB);
	pb_save_DB->setAutoDefault(false);
	pb_save_DB->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_DB->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_DB, SIGNAL(clicked()), SLOT(save_DB()));

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

	int j=0;

	QGridLayout * grid1 = new QGridLayout(this, 7, 2, 2);
	for (int i=1; i<7; i++)
	{
		grid1->setRowSpacing(i, 26);
	}
	grid1->setRowSpacing(0, 30);
	grid1->addMultiCellWidget(lbl_bar, j, j, 0, 1);
	j++;
	grid1->addWidget(lbl_description, j, 0);
	grid1->addWidget(le_description, j, 1);
	j++;
	grid1->addWidget(lbl_vbar, j, 0);
	grid1->addWidget(le_vbar, j, 1);
	j++;
	grid1->addWidget(lbl_e280, j, 0);
	grid1->addWidget(le_e280, j, 1);
	j++;
	grid1->addWidget(lbl_sequence, j, 0);
	grid1->addWidget(le_sequence, j, 1);
	j++;
	grid1->addWidget(pb_save_HD, j, 0);
	grid1->addWidget(pb_save_DB, j, 1);
	j++;
	grid1->addWidget(pb_help, j, 0);
	grid1->addWidget(pb_close, j, 1);

	qApp->processEvents();

	QRect r = grid1->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;

	this->setGeometry(global_Xpos, global_Ypos, r.width(), r.height());
}

US_Enter_Vbar_DB::~US_Enter_Vbar_DB()
{
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_Enter_Vbar_DB::update_description(const QString &newText)
{
	description = newText;
}

void US_Enter_Vbar_DB::update_sequence()
{
	sequence = le_sequence->text();
}

void US_Enter_Vbar_DB::update_vbar(const QString & str)
{
	vbar = str.toFloat();
}

void US_Enter_Vbar_DB::update_e280(const QString & str)
{
	e280 = str.toFloat();
}

void US_Enter_Vbar_DB::save_HD()
{
	if (description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
													  "your peptide before saving it!"));
		return;
	}
	if (sequence.isEmpty() && vbar == 0.0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a sequence or vbar valuefor\n"
													  "your peptide before saving it!"));
		return;
	}

	filename = QFileDialog::getSaveFileName(USglobal->config_list.root_dir, "*.pep", 0);
	if (!filename.isEmpty())
	{
		if (filename.right(4) != ".pep")
		{
			filename.append(".pep");
		}
		switch(QMessageBox::information(this, tr("UltraScan - Peptide:"),
										tr("Click 'OK' to save peptide information to :\n"
											+ filename),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				QFile f(filename);
				f.open(IO_WriteOnly | IO_Translate);
				QTextStream ts (&f);
				ts << "DE" << "\t";
				ts << description << "\n";
				ts << "SQ" << "\n";
				ts << sequence << "\n"<<"//\n";
				ts << vbar << endl;
				ts << e280;
				f.close();
				break;
			}
			case 1:
			{
				break;
			}
		}
	}


}

void US_Enter_Vbar_DB::save_DB()
{
	if(InvID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' int the main interface!"));
		exit(0);
	}
	if (description.isEmpty())
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a description for\n"
													  "your peptide before saving it!"));
		return;
	}
	if (sequence.isEmpty() && vbar == 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please enter a sequence for\n"
													  "your peptide before saving it!"));
		return;
	}
	switch(QMessageBox::information(this, tr("UltraScan - Peptide Database:"), tr("Store this peptide info into the database?"),
										tr("OK"), tr("CANCEL"),	0,1))
	{
		case 0:
		{
			QSqlQuery target;
			QString str;
			str.sprintf("INSERT INTO tblPeptide(PepFileName, Description, Sequence, vbar, e280, InvestigatorID) VALUES('"
			+ filename + "', '" + description +"', '"+ sequence + "', %f, %f, %d);", vbar, e280, InvID);
			target.exec(str);
			break;
		}
		case 1:
		{
			break;
		}
	}
}

void US_Enter_Vbar_DB::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/enter_vbar.html");
}

void US_Enter_Vbar_DB::quit()
{
	close();
}
