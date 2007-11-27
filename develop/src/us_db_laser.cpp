#include "../include/us_db_laser.h"

US_DB_Laser::US_DB_Laser(QWidget *p, const char *name) : US_Laser( p, name)
{
	init();
	pb_load->setText("Load Data from HD");
	pb_load_db = new QPushButton(tr("Load Data from DB"), this);
	Q_CHECK_PTR(pb_load_db);
	pb_load_db->setAutoDefault(false);
	pb_load_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_load_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_load_db, SIGNAL(clicked()), SLOT(load_db()));

	instr_lbl = new QLabel(tr("Doubleclick to select:"),this);
	instr_lbl->setAlignment(AlignHCenter|AlignVCenter);
	instr_lbl->setPalette( QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	instr_lbl->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 2, QFont::Bold));


	lb_data = new QListBox(this, "datalist");
	lb_data->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	lb_data->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_data(int)));

	pb_save_db = new QPushButton(tr("Backup to DB"), this);
	Q_CHECK_PTR(pb_save_db);
	pb_save_db->setAutoDefault(false);
	pb_save_db->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_save_db->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_save_db, SIGNAL(clicked()), SLOT(save_db()));

	pb_del_db = new QPushButton(tr("Delete from DB"), this);
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


	pb_investigator = new QPushButton(tr("Select Investigator"), this);
	Q_CHECK_PTR(pb_investigator);
	pb_investigator->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_investigator->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_investigator, SIGNAL(clicked()), SLOT(sel_investigator()));

	le_investigator= new QLineEdit(this, "investigator");
	Q_CHECK_PTR(le_investigator);
	le_investigator->setAlignment(AlignLeft|AlignVCenter);
	le_investigator->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_investigator->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_investigator->setReadOnly(true);
	if(data_list.InvID>0)
	{
		le_investigator->setText(show_investigator(data_list.InvID));
	}
	else
	{
		le_investigator->setText(tr(" Not Selected"));
	}

	pb_buffer = new QPushButton(tr("Select Buffer"), this);
	Q_CHECK_PTR(pb_buffer);
	pb_buffer->setAutoDefault(false);
	pb_buffer->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_buffer->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_buffer, SIGNAL(clicked()), SLOT(sel_buffer()));

	le_buffer= new QLineEdit(this, "buffer");
	le_buffer->setAlignment(AlignLeft|AlignVCenter);
	le_buffer->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_buffer->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_buffer->setReadOnly(true);
	if(data_list.BuffID>0)
	{
		le_buffer->setText(show_buffer(data_list.BuffID));
	}
	else
	{
		le_buffer->setText(tr(" Not Selected"));
	}

	pb_peptide = new QPushButton(tr("Select Peptide"), this);
	Q_CHECK_PTR(pb_peptide);
	pb_peptide->setAutoDefault(false);
	pb_peptide->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_peptide->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_peptide, SIGNAL(clicked()), SLOT(sel_vbar()));

	le_peptide= new QLineEdit(this, "peptide");
	le_peptide->setAlignment(AlignLeft|AlignVCenter);
	le_peptide->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_peptide->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_peptide->setReadOnly(true);
	if(data_list.PepID>0)
	{
		le_peptide->setText(show_peptide(data_list.PepID));
	}
	else
	{
		le_peptide->setText(tr(" Not Selected"));
	}

	pb_DNA = new QPushButton(tr("Select DNA"), this);
	Q_CHECK_PTR(pb_DNA);
	pb_DNA->setAutoDefault(false);
	pb_DNA->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_DNA->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	connect(pb_DNA, SIGNAL(clicked()), SLOT(sel_DNA()));

	le_DNA= new QLineEdit(this, "DNA");
	le_DNA->setAlignment(AlignLeft|AlignVCenter);
	le_DNA->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_DNA->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_DNA->setReadOnly(true);
	if(data_list.DNAID>0)
	{
		le_DNA->setText(show_DNA(data_list.DNAID));
	}
	else
	{
		le_DNA->setText(tr(" Not Selected"));
	}

	global_Xpos += 30;
	global_Ypos += 30;
	setMinimumSize(860, 454);
	setGeometry(global_Xpos, global_Ypos, 860, 454);

}


US_DB_Laser::~US_DB_Laser()
{
}

void US_DB_Laser::resizeEvent(QResizeEvent *e)
{
	xpos = border;
	ypos = border;
	pb_load->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw + spacing;
	pb_reset->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_load_db->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw + spacing;
	pb_save_db->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	instr_lbl->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	pb_del_db->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh+spacing;
	lb_data->setGeometry(xpos, ypos, buttonw, buttonh*4+3*spacing);

	xpos+=buttonw + spacing;
	pb_param->setGeometry(xpos, ypos, buttonw, buttonh);

	ypos += buttonh + spacing;
	pb_channel->setGeometry(xpos, ypos, buttonw, buttonh);

	ypos += buttonh + spacing;
	pb_result->setGeometry(xpos, ypos, buttonw, buttonh);

	ypos += buttonh + spacing;
	pb_print->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_investigator->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_investigator->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_buffer->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_buffer->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_peptide->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_peptide->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_DNA->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_DNA->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_sample->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_sample->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_operator->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	le_operator->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_date1->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw+ spacing;;
	lbl_date2->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	lbl_time1->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos += buttonw + spacing;
	lbl_time2->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos = border;
	ypos += buttonh + spacing;
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos+=buttonw+spacing;
	pb_close->setGeometry(xpos, ypos, buttonw, buttonh);

	int dialogw = buttonw*2 + spacing + 2 * border;
	int plot_width = e->size().width() - dialogw - border;
	int plot_height = e->size().height()-2*border;
	data_plot->setGeometry(	dialogw, border, plot_width, plot_height);
}

/*!
	This event handler, for event <var>e</var>, to receive widget close events.
*/
void US_DB_Laser::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

/*!
	Open US_DB_TblInvestigator interface for selecting investigator.
*/
void US_DB_Laser::sel_investigator()
{
	US_DB_TblInvestigator *investigator_dlg;
	investigator_dlg = new US_DB_TblInvestigator();
	investigator_dlg->setCaption("Investigator Information");
	investigator_dlg->pb_exit->setText("Accept");
	investigator_dlg->show();
	connect(investigator_dlg, SIGNAL(valueChanged(QString, int)), SLOT(update_investigator_lbl(QString, int)));
}

/*!
	Update display with the selected investigator information.
	\param Display a string variable for show investigator info.
	\param InvID a integer variable for DB table: <tt>tblInvestigators</tt> index.
*/
void US_DB_Laser::update_investigator_lbl (QString Display, int InvID)
{
	QString str;
	data_list.InvID = InvID;
	str = Display;
	le_investigator->setText(str);
	if(str == "")
	{
		le_investigator->setText(" Not Selected");
	}
}

/*!
	Open a US_Buffer_DB interface for choosing Buffer information.
*/
void US_DB_Laser::sel_buffer()
{
	US_Buffer_DB *buffer_dlg;
	buffer_dlg = new US_Buffer_DB(false, data_list.InvID);
	buffer_dlg->setCaption("Buffer Information");
	buffer_dlg->show();
	connect(buffer_dlg, SIGNAL(IDChanged(int)), SLOT(update_buffer_lbl(int)));
}

/*!
	After get the buffer info, this function update <var>le_buffer</var>,
	show that Buffer's DB entry number.
*/
void US_DB_Laser::update_buffer_lbl(int BuffID)
{
	data_list.BuffID = BuffID;
	le_buffer->setText(show_buffer(BuffID));
	if(data_list.BuffID<=0)
	{
		le_buffer->setText(tr("Not Selected"));
	}
}

/*!
	Open a US_Vbar_DB interface for choosing Peptide information.
*/
void US_DB_Laser::sel_vbar()
{
	float vbar = .72, temp = 20, vbar20 = .72;
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true, false, data_list.InvID);
	vbar_dlg->setCaption("Peptide Information");
	vbar_dlg->show();
	connect(vbar_dlg, SIGNAL(idChanged(int)), SLOT(update_vbar_lbl(int)));
}

/*!
	After get the peptide info, this function update <var>lbl_peptide</var>,
	show that Peptide's DB entry number.
*/
void US_DB_Laser::update_vbar_lbl(int PepID)
{
	data_list.PepID = PepID;
	le_peptide->setText(show_peptide(PepID));

	if(data_list.PepID<=0)
	{
		le_peptide->setText(tr("Not Selected"));
	}
}

/*!
	Open a US_Nucleotide_DB interface for choosing DNA information.
*/
void US_DB_Laser::sel_DNA()
{
	US_Nucleotide_DB *DNA_dlg;
	DNA_dlg = new US_Nucleotide_DB(false, data_list.InvID);
	DNA_dlg->setCaption("DNA Information");
	DNA_dlg->pb_quit->setText("Accept");
	DNA_dlg->show();
	connect(DNA_dlg, SIGNAL(IdChanged(int)), SLOT(update_DNA_lbl(int)));
}

/*!
	After get the DNA info, this function update <var>lbl_DNA</var>,
	show that DNA's DB entry number.
*/
void US_DB_Laser::update_DNA_lbl(int DNAID)
{
	data_list.DNAID = DNAID;
	le_DNA->setText(show_DNA(DNAID));

	if(data_list.DNAID<=0)
	{
		le_DNA->setText(tr("Not Selected"));
	}
}

/*!
	Save the experimental data into DB table: <tt>tblLaser</tt>.
*/
void US_DB_Laser::save_db()
{
	if(!load_flag)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Load Data from HD' first!"));
		return;
	}
	if(data_list.InvID <= 0)
	{
		QMessageBox::message(tr("Attention:"), tr("Please 'Select Investigator' first!"));
		return;
	}
	QSqlQuery target1, target2;
	data_list.LaserID = get_newID("tblLaser","LaserID");
	QString  STR ="INSERT INTO tblLaser (LaserID, SampleName, Operator, Date, ";
	if(data_list.BuffID>0)
		STR += "BufferID, ";
	if(data_list.PepID>0)
		STR += "PeptideID, ";
	if(data_list.DNAID>0)
		STR += "DNAID, ";
	STR +="InvestigatorID) VALUES (";
	STR += QString::number(data_list.LaserID) + ", '";
	STR += data_list.sampleID + "', '";
	STR += data_list.operatorID+ "', '";
	STR += data_list.date + "', ";
	if(data_list.BuffID>0)
		STR += QString::number(data_list.BuffID)+", ";
	if(data_list.PepID>0)
		STR += QString::number(data_list.PepID)+", ";
	if(data_list.DNAID>0)
		STR += QString::number(data_list.DNAID) + ", ";
	STR += QString::number(data_list.InvID)+");";
	//cout<<STR<<endl;
	bool flg1=target1.exec(STR);
	if(!flg1)
	{
		QSqlError sqlerr1 = target1.lastError();
      QMessageBox::message(tr("Attention:"),
                           tr("Saving to DB table 'tblLaser' failed.\n"
										 "Attempted to execute this command:\n\n"
										 + STR + "\n\n"
										 "Causing the following error:\n\n")
										 + sqlerr1.text());
		return;
	}


	QFile f(fn);
	QByteArray myFile;
	if(f.exists())
	{
		f.open(IO_ReadOnly);
		unsigned int SIZE = f.size();
		if(SIZE>24000000)
		{
			QMessageBox::message(tr("Attention:"),
									tr("The file's size is too large to store\n"));
			return;
		}
		myFile=f.readAll();
		f.close();
	}

	STR = "INSERT INTO tblLaserData (LaserID, DataFile) VALUES (";
	STR += QString::number(data_list.LaserID) + ", :file);";
	target2.prepare(STR);
	target2.bindValue(":file", QVariant(myFile));
	bool flg2 = target2.exec();
	if(!flg2)
	{
		QSqlError sqlerr2 = target2.lastError();
      QMessageBox::message(tr("Attention:"),
                           tr("Saving to DB table 'tblLaserData' failed.\n"
										 "Attempted to execute this command:\n\n"
										 + STR + "\n\n"
										 "Causing the following error:\n\n")
										 + sqlerr2.text());

		return;
	}
	pb_save_db->setEnabled(false);
	lb_data->clear();
	QString str;
	str.sprintf("Laser Exp. Data (ID:%d) ", data_list.LaserID);
	lb_data->insertItem(str);
	lb_data->insertItem("has been saved to DB.");
}

/*!
	List all laser experimental data from DB table: <tt>tblLaser</tt>.
*/
void US_DB_Laser::load_db()
{
	QString str;
	int maxID = get_newID("tblLaser","LaserID");
	int count = 0;
	item_LaserID = new int[maxID];
	item_Sample = new QString[maxID];
	display_Str = new QString[maxID];

	if(data_list.InvID >0)
	{
		str.sprintf("SELECT LaserID, SampleName FROM tblLaser WHERE InvestigatorID = %d ORDER BY LaserID;", data_list.InvID);
	}
	else
	{
		str.sprintf("SELECT LaserID, SampleName FROM tblLaser ORDER BY LaserID;");

	}
	QSqlQuery query(str);
	if(query.isActive())
	{
		while(query.next())
		{
			item_LaserID[count] = query.value(0).toInt();
			item_Sample[count] = query.value(1).toString();
			display_Str[count] = "("+  QString::number( item_LaserID[count] ) + "): "+item_Sample[count];
			count++;
		}
	}
	lb_data->clear();
	if(count>0)
	{
		for( int i=0; i<count; i++)
		{
			lb_data->insertItem(display_Str[i]);
		}
		sel_flag=true;
	}
	else
	{
		if(data_list.InvID>0)
		{
			QMessageBox::message(tr("Warning:"),
									tr("No laser data found for the selected investigator,\n You can 'Reset' then load DB to list all Laser files."));
		}
		else
		{
			QMessageBox::message(tr("Warning:"),
									tr("No laser data found in DB"));
		}
	}
}

void US_DB_Laser::select_data(int t)
{
	QString str;
	QString dirName = USglobal->config_list.data_dir + "/db_temp/";
	if(sel_flag)
	{
		Item = t;
		data_list.LaserID = item_LaserID[t];
		str.sprintf("SELECT BufferID, PeptideID, DNAID, InvestigatorID FROM tblLaser WHERE LaserID = %i;", data_list.LaserID);
		QSqlQuery query(str);
		if(query.isActive())
		{
			if(query.next())
			{
				data_list.BuffID = query.value(0).toInt();
				data_list.PepID = query.value(1).toInt();
				data_list.DNAID = query.value(2).toInt();
				data_list.InvID = query.value(3).toInt();
			}
		}

		le_buffer->setText(show_buffer(data_list.BuffID));
		le_peptide->setText(show_peptide(data_list.PepID));
		le_DNA->setText(show_DNA(data_list.DNAID));
		le_investigator->setText(show_investigator(data_list.InvID));
		pb_investigator->setEnabled(false);
		pb_buffer->setEnabled(false);
		pb_peptide->setEnabled(false);
		pb_DNA->setEnabled(false);

		QSqlCursor cur_t("tblLaserData");
		str.sprintf("LaserID=%d", data_list.LaserID);
		cur_t.select(str);
		if(cur_t.next())
		{
			from_HD=false;
			fn = make_tempFile(dirName, item_Sample[t]+".dat");
			bool flag = read_blob("DataFile", cur_t, fn);
			if(!flag)
			{
				QMessageBox::message(tr("Warning:"),
									tr("There has some problem to load data from DB table 'tblLaserData'"));
				return;
			}
			load();
		}


	}
	else
	{
		QMessageBox::message(tr("Attention:"),
									tr("No file was selected\n"));
		return;
	}
}

/*!
	Open US_DB_Admin to check delete permission.
*/
void US_DB_Laser::check_permission()
{
	US_DB_Admin *db_admin;
	db_admin = new US_DB_Admin("");
	db_admin->show();
	connect(db_admin, SIGNAL(issue_permission(bool)), SLOT(del_db(bool)));
}

/*!
	Delete selected entry of DB table: <tt>tblLaser and tblLaserData</tt>.
*/
void US_DB_Laser::del_db(bool permission)
{
	if(!permission)
	{
		QMessageBox::message(tr("Attention:"),
									tr("Permission denied"));
		return;
	}
	if(data_list.LaserID <=0)
	{
		QMessageBox::message(tr("Attention:"),
									tr("First select the Laser which you\n"
										"want to delete from the database"));
		return;
	}
	else
	{

		switch(QMessageBox::information(this, tr("Confirm: Do you really want to delete this entry?"),
										tr("Clicking 'OK' will delete the selected laser data from the database."),
										tr("OK"), tr("CANCEL"),	0,1))
		{
			case 0:
			{
				QSqlQuery del1, del2;
				QString str;
				str.sprintf("DELETE FROM tblLaser WHERE LaserID = %i;", data_list.LaserID);
				del1.exec(str);
				str.sprintf("DELETE FROM tblLaserData WHERE LaserID = %i;", data_list.LaserID);
				del2.exec(str);
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


void US_DB_Laser::init()
{
	data_list.LaserID=0;
	data_list.InvID=0;
	data_list.BuffID=0;
	data_list.PepID=0;
	data_list.DNAID=0;
	Item=0;
	sel_flag=false;
	from_HD=true;
}

void US_DB_Laser::reset()
{
	init();
	le_investigator->setText("Not Selected");
	le_buffer->setText("Not Selected");
	le_peptide->setText("Not Selected");
	le_DNA->setText("Not Selected");
	le_sample->setText("");
	le_operator->setText("");
	lbl_date2->setText("");
	lbl_time2->setText("");
	lb_data->clear();
	data_plot->clear();
	data_plot->replot();
	pb_param->setEnabled(false);
	pb_channel->setEnabled(false);
	pb_result->setEnabled(false);
	pb_print->setEnabled(false);
	pb_investigator->setEnabled(true);
	pb_buffer->setEnabled(true);
	pb_peptide->setEnabled(true);
	pb_DNA->setEnabled(true);

}
