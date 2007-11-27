#include "../include/us_db_rtv_sample.h"

//! Constructor
/*!
	Constractor a new <var>US_DB_RtvSample</var> interface,
	with <var>p</var> as a parent and <var>us_rtvsample</var> as object name.
*/
US_DB_RtvSample::US_DB_RtvSample(QWidget *p, const char *name) : US_DB_RtvInvestigator( p, name)
{
	SampleID =0;
	QString str =tr("Show Investigator Sample Information\nfrom DB:");
	str.append(login_list.dbname);
	lbl_blank->setText(str);

	lb_name->disconnect();
	connect(lb_name, SIGNAL(highlighted(int)), SLOT(select_name(int)));
	connect(lb_name, SIGNAL(selected(int)), SLOT(check_sample(int)));

	pb_chkID->disconnect();
	connect(pb_chkID, SIGNAL(clicked()), SLOT(checkSample()));
	lb_data->disconnect();
	connect(lb_data, SIGNAL(highlighted(int)), SLOT(select_sample(int)));
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_sample(int)));

	pb_retrieve->disconnect();
	pb_retrieve->setText("Show Info");
	connect(pb_retrieve, SIGNAL(clicked()), SLOT(show_info()));
}

//! Destructor
/*! destroy the US_DB_RtvSample. */
US_DB_RtvSample::~US_DB_RtvSample()
{
}

void US_DB_RtvSample::check_sample(int item)
{
	select_name(item);
	checkSample();
}

void US_DB_RtvSample::checkSample()
{
	QString str, *item_description;
	int maxID = get_newID("tblSample", "SampleID");
	int count = 0;
	item_SampleID = new int[maxID];
	item_description = new QString[maxID];
	display_Str = new QString[maxID];

	if(check_ID)
	{
		str.sprintf("SELECT SampleID, Description FROM tblSample WHERE InvestigatorID = %d;", InvID);
		QSqlQuery query(str);
		if(query.isActive())
		{
			while(query.next())
			{
				item_SampleID[count] = query.value(0).toInt();
				item_description[count] = query.value(1).toString();
				display_Str[count] = "(" + QString::number( item_SampleID[count] ) + "), "+item_description[count];
				count++;
			}
		}

		if(count>0)
		{
			lb_data->clear();
			for( int i=0; i<count; i++)
			{
				lb_data->insertItem(display_Str[i]);
			}
			sel_data = true;
		}
		else
		{
			QString str;
			str = "No database records available for: " + name;
			lb_data->clear();
			lb_data->insertItem(str);
		}
	}
	else
	{
		QMessageBox::message(tr("Attention:"),
									tr("Please select an investigator first!\n"));
		return;
	}
}


void US_DB_RtvSample::select_sample(int item)
{
	QString str;
	if(sel_data)
	{
		SampleID = item_SampleID[item];
		Display = display_Str[item];
		str.sprintf("Retrieve: "+ Display);
		lbl_item->setText(str);
		retrieve_flag = true;
	}
	else
	{
		QMessageBox::message(tr("Attention:"),
									tr("No Sample Data has been selected\n"));
		return;
	}
}

void US_DB_RtvSample::show_info()
{
	if(!retrieve_flag)
	{
		QMessageBox::message(tr("Attention:"),
									tr("You have to select a dataset first!\n"));
		return;
	}

	US_DB_Sample *us_db_sample;
	us_db_sample = new US_DB_Sample(SampleID);
	us_db_sample->show();
}


/****************************************************************************************************/

US_DB_Sample::US_DB_Sample(int temp_sampleID, QWidget *p, const char *name) : US_DB(p, name)
{
	int border=4, spacing=2;
	int xpos = border, ypos = border;
	int buttonw = 180, buttonh = 26;

	BufferID=0;
	PeptideID=0;
	DNAID=0;
	ImageID=0;
	InvID=0;

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));


	lbl_sample = new QLabel(tr(" Sample :"),this);
	lbl_sample->setAlignment(AlignLeft|AlignVCenter);
	lbl_sample->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_sample->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_sample->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos +=buttonw+spacing;

	le_sample= new QLineEdit(this, "sample");
	le_sample->setAlignment(AlignLeft|AlignVCenter);
	le_sample->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_sample->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_sample->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_sample->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	lbl_STemp = new QLabel(tr(" Storage Temperature :"),this);
	lbl_STemp->setAlignment(AlignLeft|AlignVCenter);
	lbl_STemp->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_STemp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_STemp->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos +=buttonw+spacing;

	le_STemp= new QLineEdit(this, "STemp");
	le_STemp->setAlignment(AlignLeft|AlignVCenter);
	le_STemp->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_STemp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_STemp->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_STemp->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	lbl_vbar = new QLabel(tr(" VBar :"),this);
	lbl_vbar->setAlignment(AlignLeft|AlignVCenter);
	lbl_vbar->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_vbar->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_vbar->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos +=buttonw+spacing;

	le_vbar= new QLineEdit(this, "vbar");
	le_vbar->setAlignment(AlignLeft|AlignVCenter);
	le_vbar->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_vbar->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_vbar->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_vbar->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	lbl_RTemp = new QLabel(tr(" Run Temperature :"),this);
	lbl_RTemp->setAlignment(AlignLeft|AlignVCenter);
	lbl_RTemp->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_RTemp->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_RTemp->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos +=buttonw+spacing;

	le_RTemp= new QLineEdit(this, "RTemp");
	le_RTemp->setAlignment(AlignLeft|AlignVCenter);
	le_RTemp->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_RTemp->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_RTemp->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_RTemp->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	pb_buff = new QPushButton(tr("Buffer :"), this);
	pb_buff->setAutoDefault(false);
	pb_buff->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_buff->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_buff->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_buff->setEnabled(false);
	connect(pb_buff, SIGNAL(clicked()), SLOT(view_buff()));

	xpos +=buttonw+spacing;

	le_buff= new QLineEdit(this, "buffer");
	le_buff->setAlignment(AlignLeft|AlignVCenter);
	le_buff->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_buff->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_buff->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_buff->setReadOnly(true);


	xpos = border;
	ypos+= buttonh+spacing;

	pb_pep = new QPushButton(tr("Peptide :"), this);
	pb_pep->setAutoDefault(false);
	pb_pep->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_pep->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_pep->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_pep->setEnabled(false);
	connect(pb_pep, SIGNAL(clicked()), SLOT(view_pep()));

	xpos +=buttonw+spacing;

	le_pep= new QLineEdit(this, "peptide");
	le_pep->setAlignment(AlignLeft|AlignVCenter);
	le_pep->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_pep->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_pep->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_pep->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	pb_dna = new QPushButton(tr("DNA :"), this);
	pb_dna->setAutoDefault(false);
	pb_dna->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_dna->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_dna->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_dna->setEnabled(false);
	connect(pb_dna, SIGNAL(clicked()), SLOT(view_DNA()));

	xpos +=buttonw+spacing;

	le_dna= new QLineEdit(this, "dna");
	le_dna->setAlignment(AlignLeft|AlignVCenter);
	le_dna->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_dna->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_dna->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_dna->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	pb_image = new QPushButton(tr("Image :"), this);
	pb_image->setAutoDefault(false);
	pb_image->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_image->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_image->setGeometry(xpos, ypos, buttonw, buttonh);
	pb_image->setEnabled(false);
	connect(pb_image, SIGNAL(clicked()), SLOT(view_image()));

	xpos +=buttonw+spacing;

	le_image= new QLineEdit(this, "image");
	le_image->setAlignment(AlignLeft|AlignVCenter);
	le_image->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_image->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_image->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_image->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	lbl_note = new QLabel(tr(" Special Instructions :"),this);
	lbl_note->setAlignment(AlignLeft|AlignVCenter);
	lbl_note->setPalette( QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_note->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_note->setGeometry(xpos, ypos, buttonw, buttonh);

	xpos +=buttonw+spacing;

	le_note= new QLineEdit(this, "note");
	le_note->setAlignment(AlignLeft|AlignVCenter);
	le_note->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	le_note->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	le_note->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	le_note->setReadOnly(true);

	xpos = border;
	ypos+= buttonh+spacing;

	pb_help = new QPushButton(tr("Help"), this);
	pb_help->setAutoDefault(false);
	pb_help->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_help->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_help->setGeometry(xpos, ypos, buttonw, buttonh);
	connect(pb_help, SIGNAL(clicked()), SLOT(help()));

	xpos +=buttonw+spacing;

	pb_close = new QPushButton(tr("Close"), this);
	pb_close->setAutoDefault(false);
	pb_close->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_close->setPalette(QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_close->setGeometry(xpos, ypos, buttonw*3/2, buttonh);
	connect(pb_close, SIGNAL(clicked()), SLOT(quit()));

	xpos = buttonw*5/2+2*border+2*spacing;
	ypos += buttonh + border;
	setMinimumSize(xpos, ypos);

	global_Xpos += 30;
	global_Ypos += 30;
	setGeometry(global_Xpos, global_Ypos, xpos, ypos);

	load(temp_sampleID);
}
US_DB_Sample::~US_DB_Sample()
{
}

void US_DB_Sample::load(int sampleID)
{
	QString str;
	QString description, vbar, note;
	int stemp, rtemp;

	str.sprintf("SELECT Description, BufferID, PeptideID, DNAID, ImageID, StorageTemperature, Vbar, RunTemperature, Notes, InvestigatorID FROM tblSample WHERE SampleID = %d;", sampleID);
	QSqlQuery query(str);
	if(query.isActive())
	{
		if(query.next())
		{
			description = query.value(0).toString();
			BufferID=query.value(1).toInt();
			PeptideID=query.value(2).toInt();
			DNAID=query.value(3).toInt();
			ImageID=query.value(4).toInt();
			stemp=query.value(5).toInt();
			vbar=query.value(6).toString();
			rtemp=query.value(7).toInt();
			note=query.value(8).toString();
			InvID=query.value(9).toInt();
			str="("+QString::number(sampleID)+"), "+description;
			le_sample->setText(str);
			le_STemp->setText(QString::number(stemp)+" ºC");
			le_vbar->setText(vbar+" ccm/g");
			le_RTemp->setText(QString::number(rtemp)+" ºC");
			le_buff->setText(show_buffer(BufferID));
			le_pep->setText(show_peptide(PeptideID));
			le_dna->setText(show_DNA(DNAID));
			le_image->setText(show_image(ImageID));
			le_note->setText(note);
		}
		else
		{
			QMessageBox::message(tr("Attention:"), tr("SQL query failed to retrieve the sample ID"));
			exit(-1);
		}
	}
	if(BufferID>0)
	{
		pb_buff->setEnabled(true);
	}
	if(PeptideID>0)
	{
		pb_pep->setEnabled(true);
	}

	if(DNAID>0)
	{
		pb_dna->setEnabled(true);
	}
	if(ImageID>0)
	{
		pb_image->setEnabled(true);
	}


}

void US_DB_Sample::view_buff()
{
	US_Buffer_DB *buffer_dlg;
	buffer_dlg = new US_Buffer_DB(false, InvID);
	buffer_dlg->setCaption("Buffer Information");
	buffer_dlg->get_buffer(BufferID);
	buffer_dlg->show();
}

void US_DB_Sample::view_pep()
{
	float vbar = .72, temp = 20, vbar20 = .72;
	US_Vbar_DB *vbar_dlg;
	vbar_dlg = new US_Vbar_DB(temp, &vbar, &vbar20, true, false, InvID);
	vbar_dlg->setCaption("Peptide Information");
	vbar_dlg->retrieve_vbar(PeptideID);
	vbar_dlg->show();
}

void US_DB_Sample::view_DNA()
{
	US_Nucleotide_DB *DNA_dlg;
	DNA_dlg = new US_Nucleotide_DB(false, InvID);
	DNA_dlg->setCaption("DNA Information");
	DNA_dlg->retrieve_DNA(DNAID);
	DNA_dlg->show();
}

void US_DB_Sample::view_image()
{
	US_DB_RtvImage *image_dlg;
	image_dlg = new US_DB_RtvImage();
	image_dlg->retrieve_flag=true;
	image_dlg->ImageID=ImageID;
	image_dlg->show_image();
}

void US_DB_Sample::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/db_sample.html");
}

void US_DB_Sample::quit()
{
	close();
}
