#include "../include/us_db_rst_channel.h"

US_DB_RST_Channel::US_DB_RST_Channel( QString temp_run_id, QString temp_text, int temp_source_type, QWidget *p, const char *name) : US_DB(p, name)
{
	for(int i=0; i<4;i++)
	{
		EDID[i] = -1;
	}
	run_id = temp_run_id;
	text = temp_text;
	QString cell_str = text.mid(5,1);
	source_type = temp_source_type;
	cell = cell_str.toInt();
	int p1, p2, len;
	p1 = text.find("(", 0);
	p2 = text.find(")", 0);
	len = p2-p1-1;
	QString cellid_str = text.mid(p1+1,len);
	cellid = cellid_str.toInt();
	p1 = text.findRev("WL", -1);
	QString wl_str = text.mid(p1+2,1);
	wl = wl_str.toInt();
	p1 = text.findRev(":", -1);
	p2 = text.findRev(")",-1);
	len = p2-p1-2;
	QString sc_str = text.mid(p1+2, len);
	scans = sc_str.toInt();
	
	//cout<<"cell = "<<cell<<" cellid = "<<cellid<<" wl = "<<wl<<" scans = "<<scans<<endl;
	QString str;
	QSqlCursor cur( "tblCell" );
	str.sprintf("CellID = %d",cellid);
	cur.select(str);
	if(cur.next())
	{
		for(int i=0; i<4; i++)
		{
			str.sprintf("PeptideID_%d", i+1);
			Pepid[i] = cur.value(str).toInt();
			str.sprintf("BufferID_%d", i+1);
			Bufid[i] = cur.value(str).toInt();
		}
	}

	if(source_type == 0)		//from HD
	{
		htmlDir = USglobal->config_list.html_dir + "/" + run_id;
		baseName = USglobal->config_list.result_dir + "/" + run_id + ".";
	}
	channelGrid = new QGridLayout (this,1,4,2,2);
	channelTab = new QTabWidget(this);
	channelTab->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	channel1 = new US_DB_RST_Channel_Layout(source_type, channelTab);
	channel1->get_display("Channel 1");
	channelTab->addTab(channel1, "Channel 1");
	
	channel2 = new US_DB_RST_Channel_Layout(source_type, channelTab);
	channel2->get_display("Channel 2");
	channelTab->addTab(channel2, " Channel 2 ");
	
	channel3 = new US_DB_RST_Channel_Layout(source_type, channelTab);
	channel3->get_display("Channel 3");
	channelTab->addTab(channel3, " Channel 3 ");
	
	channel4 = new US_DB_RST_Channel_Layout(source_type, channelTab);
	channel4->get_display("Channel 4");
	channelTab->addTab(channel4, " Channel 4 ");
	
	channelGrid->addWidget(channelTab, 0,0);
	
	if(source_type == 0)			// from HD
	{
		for(int k=0; k<4; k++)
		{
			read_HD_expdata(k);
		}	
	}
	if(source_type == 1)			// from DB
	{
		QString str;
		QSqlCursor cur( "EquilCellResult" );
		str.sprintf("CellRstID = %d",cellid);
		cur.select(str);
		if(cur.next())
		{
			for(int j= 0; j<4; j++)
			{
				str.sprintf("EED%dID", j+1);
				EDID[j] = cur.value(str).toInt();
				read_DB_expdata(j);
			}
			description = cur.value("Description").toString();
		}
		else
		{
			QMessageBox::message(tr("Attention:"), 
									tr("No result data exists for this cell in the database"));
			return;
		}
	}
	channel1->get_dbid(EDID[0]);
	channel2->get_dbid(EDID[1]);
	channel3->get_dbid(EDID[2]);
	channel4->get_dbid(EDID[3]);
}
US_DB_RST_Channel::~US_DB_RST_Channel()
{
}

void US_DB_RST_Channel::read_HD_expdata(int k)
{
	QString str, str1;
	QFile testfile;
	
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	str.sprintf("channel%d.html", k+1);	
	expdata_file = make_tempFile(dirName, str);
	QFile f(expdata_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	str1.sprintf("<ul>\n   <b>Equilibrium Experimental Data for Channel %d: (from hard drive):</b>\n   <p>\n   <ul>\n", k+1);
	ts << str << str1;
	
	int i = cell-1;
	int j = wl-1;
	//int k =0;
	if (scans != 0)
	{			
	// Raw Experimental Data Plot Image:
		str.sprintf(htmlDir + "/raw_%d%d%d.png", i+1, j+1, k+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			if(k==0)
				channel1->cb_lbl1->setChecked(true);
			if(k==1)
				channel2->cb_lbl1->setChecked(true);
			if(k==2)
				channel3->cb_lbl1->setChecked(true);
			if(k==3)
				channel4->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Experimental Data Plot</a>\n");
			flag = true;
		}
		else
		{
			if(k==0)
				channel1->cb_lbl1->setEnabled(false);
			if(k==1)
				channel2->cb_lbl1->setEnabled(false);
			if(k==2)
				channel3->cb_lbl1->setEnabled(false);
			if(k==3)
				channel4->cb_lbl1->setEnabled(false);
		}
	// Raw Data Ascii Data:
		str.sprintf(baseName + "raw.%d%d%d", i+1, j+1, k+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			if(k==0)
				channel1->cb_lbl2->setChecked(true);
			if(k==1)
				channel2->cb_lbl2->setChecked(true);
			if(k==2)
				channel3->cb_lbl2->setChecked(true);
			if(k==3)
				channel4->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << ">ASCII File of Experimental Data</a>\n";
			flag = true;
		}
		else
		{
			if(k==0)
				channel1->cb_lbl2->setEnabled(false);
			if(k==1)
				channel2->cb_lbl2->setEnabled(false);
			if(k==2)
				channel3->cb_lbl2->setEnabled(false);
			if(k==3)
				channel4->cb_lbl2->setEnabled(false);
		}
	//Peptide Data:
		str = "";
		if(Pepid[k]>0)
		{
			str = USglobal->config_list.result_dir + "/" + QString::number(Pepid[k]) + ".pep_res";
		}
		testfile.setName(str);
		if (testfile.exists())
		{
			if(k==0)
				channel1->cb_lbl3->setChecked(true);
			if(k==1)
				channel2->cb_lbl3->setChecked(true);
			if(k==2)
				channel3->cb_lbl3->setChecked(true);
			if(k==3)
				channel4->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << ">Peptide Composition</a>\n";
			flag = true;
		}
		else
		{
			if(k==0)
				channel1->cb_lbl3->setEnabled(false);
			if(k==1)
				channel2->cb_lbl3->setEnabled(false);
			if(k==2)
				channel3->cb_lbl3->setEnabled(false);
			if(k==3)
				channel4->cb_lbl3->setEnabled(false);
		}
	//Buffer Data:
		str = "";
		if(Bufid[k]>0)
		{
			str = USglobal->config_list.result_dir + "/" + QString::number(Bufid[k]) + ".buf_res";
		}
		testfile.setName(str);
		if (testfile.exists())
		{
			if(k==0)
				channel1->cb_lbl4->setChecked(true);
			if(k==1)
				channel2->cb_lbl4->setChecked(true);
			if(k==2)
				channel3->cb_lbl4->setChecked(true);
			if(k==3)
				channel4->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str << ">Buffer Composition</a>\n";
			flag = true;
		}
		else
		{
			if(k==0)
				channel1->cb_lbl4->setEnabled(false);
			if(k==1)
				channel2->cb_lbl4->setEnabled(false);
			if(k==2)
				channel3->cb_lbl4->setEnabled(false);
			if(k==3)
				channel4->cb_lbl4->setEnabled(false);
		}
	}
	if(k==0)
		channel1->pb_display->setEnabled(flag);
	if(k==1)
		channel2->pb_display->setEnabled(flag);
	if(k==2)
		channel3->pb_display->setEnabled(flag);
	if(k==3)
		channel4->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Channel::read_DB_expdata(int k)
{
	QString str, str1, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool plot_flag = false;
	bool f_plot_flag = false;
	bool peptide_flag = false;
	bool buffer_flag = false;
	int i = cell-1;
	int j = wl-1;
	
	if(EDID[k]>0)		//expdata result exists.
	{	
		str.sprintf("channel%d.html", k+1);
		expdata_file = make_tempFile(dirName, str);
		QFile f(expdata_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		str1.sprintf("<ul>\n   <b>Equilibrium Experimental Data for Channel %d: (from database):</b>\n   <p>\n   <ul>\n", k+1);
		ts << str << str1;
			
		QSqlCursor cur( "EquilExpData" );
		str.sprintf("EEDID = %d",EDID[k]);
		cur.select(str);
		if(cur.next())
		{
			str.sprintf("EEDtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)	
			{
				QSqlCursor cur_t("EquilExpDataTable");
				str.sprintf("EEDtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
			// Raw Experimental Data Plot Image:
					str.sprintf("raw_%d%d%d.png", i+1, j+1, k+1);			// %d%d%d == cell/scans/channel
					filename = make_tempFile(dirName, str);
					plot_flag = read_blob("EDPlot", cur_t, filename);
					if(plot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Experimental Data Plot</a>\n");
						if(k==0)
							channel1->cb_lbl1->setChecked(true);
						if(k==1)
							channel2->cb_lbl1->setChecked(true);
						if(k==2)
							channel3->cb_lbl1->setChecked(true);
						if(k==3)
							channel4->cb_lbl1->setChecked(true);
					}
					else
					{
						if(k==0)
							channel1->cb_lbl1->setEnabled(false);
						if(k==1)
							channel2->cb_lbl1->setEnabled(false);
						if(k==2)
							channel3->cb_lbl1->setEnabled(false);
						if(k==3)
							channel4->cb_lbl1->setEnabled(false);
					}
			// Raw Data Ascii Data:
					str.sprintf("raw.%d%d%d", i+1, j+1, k+1);			
					filename = make_tempFile(dirName, str);
					f_plot_flag = read_blob("F_EDPlot", cur_t, filename);
					if(f_plot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">ASCII File of Experimental Data Plot</a>\n");
						if(k==0)
							channel1->cb_lbl2->setChecked(true);
						if(k==1)
							channel2->cb_lbl2->setChecked(true);
						if(k==2)
							channel3->cb_lbl2->setChecked(true);
						if(k==3)
							channel4->cb_lbl2->setChecked(true);
					}
					else
					{
						if(k==0)
							channel1->cb_lbl2->setEnabled(false);
						if(k==1)
							channel2->cb_lbl2->setEnabled(false);
						if(k==2)
							channel3->cb_lbl2->setEnabled(false);
						if(k==3)
							channel4->cb_lbl2->setEnabled(false);
					}
			// Peptide data:
					str.sprintf("pep_res.%d%d%d", i+1, j+1, k+1);			
					filename = make_tempFile(dirName, str);
					peptide_flag = read_blob("PepFile", cur_t, filename);
					if(peptide_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Peptide Composition</a>\n");
						if(k==0)
							channel1->cb_lbl3->setChecked(true);
						if(k==1)
							channel2->cb_lbl3->setChecked(true);
						if(k==2)
							channel3->cb_lbl3->setChecked(true);
						if(k==3)
							channel4->cb_lbl3->setChecked(true);
					}
					else
					{
						if(k==0)
							channel1->cb_lbl3->setEnabled(false);
						if(k==1)
							channel2->cb_lbl3->setEnabled(false);
						if(k==2)
							channel3->cb_lbl3->setEnabled(false);
						if(k==3)
							channel4->cb_lbl3->setEnabled(false);
					}
				// Buffer data:
					str.sprintf("buf_res.%d%d%d", i+1, j+1, k+1);			
					filename = make_tempFile(dirName, str);
					buffer_flag = read_blob("BufFile", cur_t, filename);
					if(buffer_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Buffer Composition</a>\n");
						if(k==0)
							channel1->cb_lbl4->setChecked(true);
						if(k==1)
							channel2->cb_lbl4->setChecked(true);
						if(k==2)
							channel3->cb_lbl4->setChecked(true);
						if(k==3)
							channel4->cb_lbl4->setChecked(true);
					}
					else
					{
						if(k==0)
							channel1->cb_lbl4->setEnabled(false);
						if(k==1)
							channel2->cb_lbl4->setEnabled(false);
						if(k==2)
							channel3->cb_lbl4->setEnabled(false);
						if(k==3)
							channel4->cb_lbl4->setEnabled(false);							
					}
				}
			}							
		}
		f.close();
	}
	else
	{
		if(k==0)
		{
			channel1->cb_lbl1->setEnabled(false);
			channel1->cb_lbl2->setEnabled(false);
			channel1->cb_lbl3->setEnabled(false);
			channel1->cb_lbl4->setEnabled(false);
			channel1->pb_display->setEnabled(false);
		}
		if(k==1)
		{
			channel2->cb_lbl1->setEnabled(false);
			channel2->cb_lbl2->setEnabled(false);
			channel2->cb_lbl3->setEnabled(false);
			channel2->cb_lbl4->setEnabled(false);
			channel2->pb_display->setEnabled(false);
		}
		if(k==2)
		{
			channel3->cb_lbl1->setEnabled(false);
			channel3->cb_lbl2->setEnabled(false);
			channel3->cb_lbl3->setEnabled(false);
			channel3->cb_lbl4->setEnabled(false);
			channel3->pb_display->setEnabled(false);
		}
		if(k==3)
		{
			channel4->cb_lbl1->setEnabled(false);
			channel4->cb_lbl2->setEnabled(false);
			channel4->cb_lbl3->setEnabled(false);
			channel4->cb_lbl4->setEnabled(false);
			channel4->pb_display->setEnabled(false);
		}
	}
}
/*****************************************************************************************************/
US_DB_RST_Channel_Layout::US_DB_RST_Channel_Layout(int temp_source_type, QWidget *parent, const char *name) : US_DB(parent, name)
{
	source_type = temp_source_type;
	int spacing=2;
	int xpos,ypos;
	int buttonw = 150, buttonh = 26;	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	xpos = spacing;
	ypos = spacing;
	lbl_DBID = new QLabel(" EquilExpDataTable Entry :", this);
	lbl_DBID->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_DBID->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_DBID->setGeometry(xpos, ypos, buttonw + buttonw/2, buttonh);
	
	xpos += buttonw + buttonw/2;
	lbl_dbid= new QLabel("",this);
	lbl_dbid->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dbid->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_dbid->setGeometry(xpos, ypos, buttonw/2, buttonh);
	
	xpos += buttonw*3/2;
	QString str;
	if(source_type == 0)
	{
		str = tr(" Available on HD :");
	}
	if(source_type == 1)
	{
		str = tr(" Available on DB :");
	}
	lbl_source = new QLabel(str, this);
	lbl_source->setAlignment(AlignHCenter|AlignVCenter);
	lbl_source->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_source->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label1 = new QLabel(tr(" Experimental Data Plot :"), this);
	lbl_label1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label1->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl1 = new QCheckBox(this);
	cb_lbl1->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label2 = new QLabel(tr(" ASCII File of Experimental Data :"), this);
	lbl_label2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label2->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl2 = new QCheckBox(this);
	cb_lbl2->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label3 = new QLabel(tr(" Peptide Composition :"), this);
	lbl_label3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label3->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl3 = new QCheckBox(this);
	cb_lbl3->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label4 = new QLabel(tr(" Buffer Composition :"), this);
	lbl_label4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label4->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl4 = new QCheckBox(this);
	cb_lbl4->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	

	xpos = spacing;
	ypos += buttonh + spacing;
	
	pb_display = new QPushButton("", this);
	pb_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_display->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_display->setGeometry(xpos+buttonw*3/2+buttonw*2/3, ypos, buttonw*2-buttonw/5, buttonh);
	connect(pb_display, SIGNAL(clicked()), SLOT(display()));

}

US_DB_RST_Channel_Layout::~US_DB_RST_Channel_Layout()
{
}

void US_DB_RST_Channel_Layout::get_dbid(int tableID)
{
	if(tableID == -1)
		lbl_dbid->setText("N/A");
	else
		lbl_dbid->setNum(tableID);
}

void US_DB_RST_Channel_Layout::get_display(QString text)
{
	QString str;	
	if(source_type == 0)
	{
		str = "Display  " + text + " Result from HD";
	}
	if(source_type == 1)
	{
		str = "Display  " + text + " Result from DB";
	}
	pb_display->setText(str);
	display_Str = text;
}

void US_DB_RST_Channel_Layout::display()
{
	QString dir, display_file;
	if(source_type == 0)
	{
		dir = "/temp/";
	}
	if(source_type == 1)
	{
		dir = "/db_temp/";
	}
	if(display_Str == "Channel 1")
	{
		display_file = USglobal->config_list.html_dir + dir +"channel1.html";
	}
	if(display_Str == "Channel 2")
	{
		display_file = USglobal->config_list.html_dir + dir +"channel2.html";
	}
	if(display_Str == "Channel 3")
	{
		display_file = USglobal->config_list.html_dir + dir + "channel3.html";
	}
	if(display_Str == "Channel 4")
	{
		display_file = USglobal->config_list.html_dir + dir +"channel4.html";
	}


	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_html_file(display_file);
}

