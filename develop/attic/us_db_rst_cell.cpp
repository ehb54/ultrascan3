#include "../include/us_db_rst_cell.h"

US_DB_RST_Cell::US_DB_RST_Cell(QString temp_run_id, QString temp_text, int temp_source_type, QWidget *p, const char *name) : US_DB(p, name)
{
	for(int i=0; i<4;i++)
	{
		EDID[i] = -1;
		VHWID[i] = -1;
		CSID[i] = -1;
		FEID[i] = -1;
		SMID[i] = -1;
		TDID[i] = -1;
		RDID[i] = -1;
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
		Num_Channel = cur.value("Num_Channel").toInt();
		for(int i=0; i<4; i++)
		{
			str.sprintf("PeptideID_%d", i+1);
			Pepid[i] = cur.value(str).toInt();
			str.sprintf("BufferID_%d", i+1);
			Bufid[i] = cur.value(str).toInt();
		}
	}

	if(source_type == 0)		//load from HD
	{
		htmlDir = USglobal->config_list.html_dir + "/" + run_id;
		baseName = USglobal->config_list.result_dir + "/" + run_id + ".";
	}
	display ="";
	cellGrid = new QGridLayout (this,1,4,2,2);
	cellTab = new QTabWidget(this);
	cellTab->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));

	l_expdata = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_expdata->get_channel(Num_Channel);
	l_expdata->lbl_report->setText("");			//no analysis report for Exp. data
	l_expdata->cb_report->hide();
	l_expdata->get_label1(" Experimental Data Plot :");
	l_expdata->get_label2(" ASCII File of Experimental Data :");
	l_expdata->get_label3(" Peptide Composition :");
	l_expdata->get_label4(" Buffer Composition :");
	l_expdata->lbl_label5->hide();
	l_expdata->cb_lbl5->hide();
	l_expdata->lbl_label6->hide();
	l_expdata->cb_lbl6->hide();
	l_expdata->get_display("Exp. Data");
	cellTab->addTab(l_expdata, "Exp. Data");
	
	l_vhw = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_vhw->get_channel(Num_Channel);
	l_vhw->get_label1(" Extrapolation Plot :");
	l_vhw->get_label2(" ASCII File of Extrapolation Plot Data :");
	l_vhw->get_label3(" Distribution Plot (G(S)) :");
	l_vhw->get_label4(" ASCII File of Distribution Plot Data :");
	l_vhw->get_label5(" S-value Histogram Plot (G(S)) :");
	l_vhw->get_label6(" Edited Data Plot :");
	l_vhw->get_display("vH-W");
	cellTab->addTab(l_vhw, " vH-W ");
	
	l_cs = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_cs->get_channel(Num_Channel);
	l_cs->get_label1(" C(s) Distribution Plot :");
	l_cs->get_label2(" C(s) Molecular Weight Distribution Plot :");
	l_cs->get_label3(" Residuals Plot :");
	l_cs->get_label4(" Residuals Pixel Map :");
	l_cs->get_label5(" ASCII File of Distribution Plot Data :");
	l_cs->get_label6(" Edited Data Plot :");
	l_cs->get_display("C(s)");
	cellTab->addTab(l_cs, " C(s) ");
	
	l_fe = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_fe->get_channel(Num_Channel);
	l_fe->get_label1(" Residuals from Scans :");
	l_fe->get_label2(" Overlays of Scans :");
	l_fe->get_label3(" ASCII File of Residuals :");
	l_fe->get_label4(" ASCII File of Experimental Data :");
	l_fe->get_label5(" ASCII File of Finite Element Solution Data :");
	l_fe->lbl_label6->hide();
	l_fe->cb_lbl6->hide();
	l_fe->get_display("Finite Element");
	cellTab->addTab(l_fe, "Finite Element");
	
	l_sm = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_sm->get_channel(Num_Channel);
	l_sm->get_label1(" Second Moment Analysis Plot :");
	l_sm->get_label2(" ASCII File of Second Moment Plot Data :");
	l_sm->get_label3(" Edited Data Plot :");
	l_sm->lbl_label4->hide();
	l_sm->cb_lbl4->hide();
	l_sm->lbl_label5->hide();
	l_sm->cb_lbl5->hide();
	l_sm->lbl_label6->hide();
	l_sm->cb_lbl6->hide();
	l_sm->get_display("Second Moment");
	cellTab->addTab(l_sm, "Second Moment");
	
	l_td = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_td->get_channel(Num_Channel);
	l_td->get_label1(" Time Derivative Analysis Plot :");
	l_td->get_label2(" ASCII File of Time Derivative Plot Data (Scans) :");
	l_td->get_label3(" ASCII File of Time Derivative Plot Data (S-values) :");
	l_td->get_label4(" Time Derivative Analysis Plot (Average g(S)):");
	l_td->get_label5(" ASCII File of Time Derivative Plot Data (Average g(S)) :");
	l_td->get_label6(" Edited Data Plot :");
	l_td->get_display("dC/dt");
	cellTab->addTab(l_td, "dC/dt");
	
	l_rd = new US_DB_RST_Cell_Layout(source_type, cellTab);
	l_rd->get_channel(Num_Channel);
	l_rd->get_label1(" Radial Derivative Analysis Plot :");
	l_rd->get_label2(" ASCII File of Radial Derivative Plot Data (Scans) :");
	l_rd->get_label3(" ASCII File of Radial Derivative Plot Data (S-values) :");
	l_rd->get_label4(" Radial Derivative Analysis Plot (Average g(S)):");
	l_rd->get_label5(" ASCII File of Radial Derivative Plot Data (Average g(S)) :");
	l_rd->get_label6(" Edited Data Plot :");
	l_rd->get_display("dC/dr");
	cellTab->addTab(l_rd, "dC/dr");

	cellGrid->addWidget(cellTab, 0,0);
	
	if(source_type == 0)			// from HD
	{	
		read_HD_expdata();
		read_HD_vhw();
		read_HD_cs();
		read_HD_fe();
		read_HD_sm();
		read_HD_td();
		read_HD_rd();
	}
	if(source_type == 1)			// from DB
	{
		QString str;
		QSqlCursor cur( "VelocCellResult" );
		str.sprintf("CellRstID = %d",cellid);
		cur.select(str);
		if(cur.next())
		{
			EDID[0] = cur.value("EDID").toInt();
			VHWID[0] = cur.value("VHWID").toInt();
			CSID[0] = cur.value("CSID").toInt();
			FEID[0] = cur.value("FEID").toInt();
			SMID[0] = cur.value("SMID").toInt();
			TDID[0] = cur.value("TDID").toInt();
			RDID[0] = cur.value("RDID").toInt();
			description = cur.value("Description").toString();
			read_DB_expdata();
			read_DB_vhw();
			read_DB_fe();
			read_DB_cs();
			read_DB_sm();
			read_DB_td();
			read_DB_rd();
		}
		else
		{
			QMessageBox::message(tr("Attention:"), 
									tr("No result data about this cell in database"));
			return;
		}
	}
	l_expdata->get_dbid("VelocExpData",EDID[0]);
	l_vhw->get_dbid("VelocVHW",VHWID[0]);
	l_cs->get_dbid("VelocCS",CSID[0]);
	l_fe->get_dbid("VelocFiniteElement",FEID[0]);
	l_sm->get_dbid("VelocSecondMoment",SMID[0]);
	l_td->get_dbid("VelocTimeDerivative",TDID[0]);
	l_rd->get_dbid("VelocRadialDerivative",RDID[0]);
}
US_DB_RST_Cell::~US_DB_RST_Cell()
{
}

void US_DB_RST_Cell::read_HD_expdata()
{
	QString str,str1, str2;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	expdata_file = make_tempFile(dirName, "expdata.html");
	QFile f(expdata_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>Experimental Data: (from harddrive):</b>\n   <p>\n   <ul>\n");
	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{			
// Raw Experimental Data Plot Image:
		str.sprintf(htmlDir + "/raw_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_expdata->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Experimental Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_expdata->cb_lbl1->setEnabled(false);
		}
// Raw Data Ascii Data:
		str.sprintf(baseName + "raw.%d%d1", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_expdata->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << ">ASCII File of Experimental Data</a>\n";
			flag = true;
		}
		else
		{
			l_expdata->cb_lbl2->setEnabled(false);
		}
//Peptide Data:
		if(Pepid[0]>0)
		{
			str.sprintf(USglobal->config_list.result_dir + "/" + QString::number(Pepid[0]) + ".pep_res");
			testfile.setName(str);
			if (testfile.exists())
			{
				l_expdata->cb_lbl3->setChecked(true);
				ts << "      <li><a href=" << str << ">Peptide Composition</a>\n";
				flag = true;
			}
			else
			{
				l_expdata->cb_lbl3->setEnabled(false);
			}
		}
		else
		{
			l_expdata->cb_lbl3->setEnabled(false);
		}
//Buffer Data:
		if(Bufid[0]>0)
		{
			str.sprintf(USglobal->config_list.result_dir + "/" +  QString::number(Bufid[0]) + ".buf_res");
			testfile.setName(str);
			if (testfile.exists())
			{
				l_expdata->cb_lbl4->setChecked(true);
				ts << "      <li><a href=" << str << ">Buffer Composition</a>\n";
				flag = true;
			}
			else
			{
				l_expdata->cb_lbl4->setEnabled(false);
			}
		}
		else
		{
			l_expdata->cb_lbl4->setEnabled(false);
		}
	}
	l_expdata->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Cell::read_HD_vhw()
{
	QString str;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	vhw_file = make_tempFile(dirName, "vhw.html");
	QFile f(vhw_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>van Holde - Weischet Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");

	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// vHW Anaysis Report:
		str.sprintf(baseName + "vhw_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_vhw->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;
		}
		else
		{
			l_vhw->cb_report->setEnabled(false);
		}
// Extrapolation Plot Image:
		str.sprintf(htmlDir + "/vhw_ext_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_vhw->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Extrapolation Plot</a>\n");
			flag = true;
		}
		else
		{
			l_vhw->cb_lbl1->setEnabled(false);
		}
// Extrapolation Plot Data:
		str.sprintf(baseName + "vhw_ext.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_vhw->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Extrapolation Plot Data</a>\n");
			flag = true;				
		}
		else
		{
			l_vhw->cb_lbl2->setEnabled(false);
		}
// Distribution Plot Image:
		str.sprintf(htmlDir + "/vhw_dis_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_vhw->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Distribution Plot (G(S))</a>\n");
			flag = true;
		}
		else
		{
			l_vhw->cb_lbl3->setEnabled(false);
		}
// Distribution Plot Data:
		str.sprintf(baseName + "vhw_dis.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_vhw->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str<< tr(">ASCII File of Distribution Plot Data</a>\n");
			flag = true;					
		}
		else
		{
			l_vhw->cb_lbl4->setEnabled(false);
		}
// Histogram Plot Image:
		str.sprintf(htmlDir + "/vhw_his_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_vhw->cb_lbl5->setChecked(true);
			ts << "      <li><a href=" << str << tr(">S-value Histogram Plot (G(S))</a>\n");
			flag = true;
		}
		else
		{
			l_vhw->cb_lbl5->setEnabled(false);
		}
// Edited Data Plot Image:
		str.sprintf(htmlDir + "/vhw_edited_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_vhw->cb_lbl6->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Edited Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_vhw->cb_lbl6->setEnabled(false);
		}
	}
	l_vhw->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Cell::read_HD_cs()
{
	QString str;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	cs_file = make_tempFile(dirName, "cs.html");
	QFile f(cs_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>C(s) Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");

	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// C(s) Anaysis Report:
		str.sprintf(baseName + "cofs_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_cs->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;
		}
		else
		{
			l_cs->cb_report->setEnabled(false);
		}
// C(s) Distrubution Plot Image:
		str.sprintf(htmlDir + "/cofs_dis_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_cs->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">C(s) Distrubution Plot</a>\n");
			flag = true;
		}
		else
		{
			l_cs->cb_lbl1->setEnabled(false);
		}
// C(s) Molecular Weight Distrubution Plot Image:
		str.sprintf(htmlDir + "/cofs_mw_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_cs->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << tr(">C(s) Molecular Weight Distrubution Plot</a>\n");
			flag = true;				
		}
		else
		{
			l_cs->cb_lbl2->setEnabled(false);
		}
// Residuals Plot Image:
		str.sprintf(htmlDir + "/cofs_resid_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_cs->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Residuals Plot</a>\n");
			flag = true;
		}
		else
		{
			l_cs->cb_lbl3->setEnabled(false);
		}
// Residuals Pixel Map:
		str.sprintf(htmlDir + "/cofs_pixmap_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_cs->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str<< tr(">Residuals Pixel Map</a>\n");
			flag = true;					
		}
		else
		{
			l_cs->cb_lbl4->setEnabled(false);
		}
// ASCII File of Distrubution Plot Data:
		str.sprintf(baseName + "cofs_dis.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_cs->cb_lbl5->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Distribution Plot Data</a>\n");
			flag = true;
		}
		else
		{
			l_cs->cb_lbl5->setEnabled(false);
		}
// Edited Data Plot Image:
		str.sprintf(htmlDir + "/cofs_edited_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_cs->cb_lbl6->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Edited Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_cs->cb_lbl6->setEnabled(false);
		}
	}
	l_cs->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Cell::read_HD_fe()
{
	QString str,str1;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	fe_file = make_tempFile(dirName, "fe.html");
	QFile f(fe_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>Finite Element Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");	
			
	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// Finite Element Analysis Report:
		str.sprintf(baseName + "fef_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_fe->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;					
		}
		else
		{
			l_fe->cb_report->setEnabled(false);
		}
				
		ts << tr("      <p>\n      Residual Plots and Overlays:\n      <p>\n      <ul>\n");
		for (int k=0; k<scans; k+=5)
		{
// Residuals Plot Image:
			str.sprintf(htmlDir + "/fef_%d%d_%d.png", i+1, j+1, k+1);
			testfile.setName(str);
			if (testfile.exists())
			{	
				l_fe->cb_lbl1->setChecked(true);
				str1.sprintf(tr("Residuals from Scan %d to Scan %d"), k+1, k+5);
				ts << "         <li><a href=" << str << ">" << str1 <<"</a>\n";
				flag = true;
			}
			else
			{
				l_fe->cb_lbl1->setEnabled(false);
			}
// Overlays Plot Image:
			str.sprintf(htmlDir + "/fef_edited_%d%d_%d.png", i+1, j+1, k+1);
			testfile.setName(str);
			if (testfile.exists())
			{	
				l_fe->cb_lbl2->setChecked(true);
				str1.sprintf(tr("Overlays (Scans %d - %d are highlighted)"), k+1, k+5);
				ts << "         <li><a href=" << str << ">" << str1 <<"</a>\n";
				flag = true;
			}
			else
			{
				l_fe->cb_lbl2->setEnabled(false);
			}
		}
// Residuals ASCII Data:
		str.sprintf(baseName + "fef_residuals.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_fe->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Residuals</a>\n");
			flag = true;
		}
		else
		{
			l_fe->cb_lbl3->setEnabled(false);
		}
// Raw ASCII Data:
		str.sprintf(baseName + "fef_raw.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_fe->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Experimental Data</a>\n");
			flag = true;
		}
		else
		{
			l_fe->cb_lbl4->setEnabled(false);
		}
// Finite Element ASCII Data:
		str.sprintf(baseName + "fef_dat.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_fe->cb_lbl5->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File Finite Element Solution Data</a>\n");
			flag = true;
		}
		else
		{
			l_fe->cb_lbl5->setEnabled(false);
		}
	}
	l_fe->pb_display->setEnabled(flag);
	f.close();
}	

void US_DB_RST_Cell::read_HD_sm()
{
	QString str,str1;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	sm_file = make_tempFile(dirName, "sm.html");
	QFile f(sm_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>Second Moment Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");
			
	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// Second Moment Analysis Report:
		str.sprintf(baseName + "sm_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_sm->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;
		}
		else
		{
			l_sm->cb_report->setEnabled(false);
		}
// Second Moment Plot Image:
		str.sprintf(htmlDir + "/sm_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_sm->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Second Moment Analysis Plot</a>\n");
			flag = true;
		}
		else
		{
			l_sm->cb_lbl1->setEnabled(false);
		}
// Second Moment Plot Data:
		str.sprintf(baseName + "sm_dat.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_sm->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Second Moment Plot Data</a>\n");
			flag = true;	
		}		
		else
		{
			l_sm->cb_lbl2->setEnabled(false);
		}
//Second Moment Edited Data Plot Image:
		str.sprintf(htmlDir + "/sm_edited_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_sm->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Edited Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_sm->cb_lbl3->setEnabled(false);
		}
	}
	l_sm->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Cell::read_HD_td()
{
	QString str,str1;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	td_file = make_tempFile(dirName, "td.html");
	QFile f(td_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>Time Derivative Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");
			
	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// Time Derivative Analysis Report:
		str.sprintf(baseName + "dcdt_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_report->setEnabled(false);
		}
// Time Derivative Plot Image:
		str.sprintf(htmlDir + "/dcdt_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Time Derivative Analysis Plot</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl1->setEnabled(false);
		}
// Time Derivative Scan Plot Data:
		str.sprintf(baseName + "dcdt_scans.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Time Derivative Plot Data (Scans)</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl2->setEnabled(false);
		}
// Time Derivative S-value Plot Data:
		str.sprintf(baseName + "dcdt_sval.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Time Derivative Plot Data (S-values)</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl3->setEnabled(false);
		}
// Time Derivative Average Plot Image:
		str.sprintf(htmlDir + "/dcdt_avg_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Time Derivative Analysis Plot (Average g(S))</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl4->setEnabled(false);
		}
// Time Derivative S-value Average Plot Data:
		str.sprintf(baseName + "dcdt_avg.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl5->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Time Derivative Plot Data (Average g(S))</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl5->setEnabled(false);
		}
// Edited TD Data Plot Image:
		str.sprintf(htmlDir + "/dcdt_edited_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_td->cb_lbl6->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Edited Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_td->cb_lbl6->setEnabled(false);
		}
	}
	l_td->pb_display->setEnabled(flag);
	f.close();
}

void US_DB_RST_Cell::read_HD_rd()
{
	QString str,str1;
	QFile testfile;
	bool flag = false;
	QString dirName = USglobal->config_list.html_dir + "/temp/";
	rd_file = make_tempFile(dirName, "rd.html");
	QFile f(rd_file);
	f.open(IO_WriteOnly | IO_Translate);
	QTextStream ts(&f);
	str.sprintf("<b>" + text + "</b>\n<p>\n");
	ts << str << tr("<ul>\n   <b>Radial Derivative Analysis (from harddrive):</b>\n   <p>\n   <ul>\n");
			
	int i = cell-1;
	int j = wl-1;
	if (scans != 0)
	{
// Radial Derivative Analysis Report:
		str.sprintf(baseName + "dcdr_res.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_report->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Analysis Report</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_report->setEnabled(false);
		}
// Radial Derivative Plot Image:
		str.sprintf(htmlDir + "/dcdr_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_lbl1->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Radial Derivative Analysis Plot</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_lbl1->setEnabled(false);
		}
// Radial Derivative Scan Plot Data:
		str.sprintf(baseName + "dcdr_scans.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_lbl2->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Radial Derivative Plot Data (Scans)</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_lbl2->setEnabled(false);
		}
// Radial Derivative S-value Plot Data:
		str.sprintf(baseName + "dcdr_sval.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_lbl3->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Radial Derivative Plot Data (S-values)</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_lbl3->setEnabled(false);
		}
// Radial Derivative Average Plot Image:
		str.sprintf(htmlDir + "/dcdr_avg_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{
			l_rd->cb_lbl4->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Radial Derivative Analysis Plot (Average g(S))</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_lbl4->setEnabled(false);
		}
	// Radial Derivative S-value Average Plot Data:
		str.sprintf(baseName + "dcdr_avg.%d%d", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_lbl5->setChecked(true);
			ts << "      <li><a href=" << str << tr(">ASCII File of Radial Derivative Plot Data (Average g(S))</a>\n");	
			flag = true;
		}
		else
		{
			l_rd->cb_lbl5->setEnabled(false);
		}

// Edited RD Data Plot Image:
		str.sprintf(htmlDir + "/dcdr_edited_%d%d.png", i+1, j+1);
		testfile.setName(str);
		if (testfile.exists())
		{	
			l_rd->cb_lbl6->setChecked(true);
			ts << "      <li><a href=" << str << tr(">Edited Data Plot</a>\n");
			flag = true;
		}
		else
		{
			l_rd->cb_lbl6->setEnabled(false);
		}
	}
	l_rd->pb_display->setEnabled(flag);
	f.close();
}	

void US_DB_RST_Cell::read_DB_expdata()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool plot_flag = false;
	bool f_plot_flag = false;
	bool peptide_flag = false;
	bool buffer_flag = false;

	if(EDID[0]>0)		//expdata result exists.
	{
		expdata_file = make_tempFile(dirName, "expdata.html");
		QFile f(expdata_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>Experimental Data: (from database):</b>\n   <p>\n   <ul>\n");
			
		QSqlCursor cur( "VelocExpData" );
		str.sprintf("EDID = %d",EDID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("EDtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)	
			{
				QSqlCursor cur_t("VelocExpDataTable");
				str.sprintf("EDtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
				// Raw Experimental Data Plot Image:
					str.sprintf("raw_%d%d.png", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					plot_flag = read_blob("EDPlot", cur_t, filename);
					if(plot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Experimental Data Plot</a>\n");
						l_expdata->cb_lbl1->setChecked(true);
					}
					else
					{
						l_expdata->cb_lbl1->setEnabled(false);
					}
				// Raw Data Ascii Data:
					str.sprintf("raw.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					f_plot_flag = read_blob("F_EDPlot", cur_t, filename);
					if(f_plot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">ASCII File of Experimental Data Plot</a>\n");
						l_expdata->cb_lbl2->setChecked(true);
					}
					else
					{
						l_expdata->cb_lbl2->setEnabled(false);
					}
				// Peptide data:
					str.sprintf("pep_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					peptide_flag = read_blob("PepFile", cur_t, filename);
					if(peptide_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Peptide Composition</a>\n");
						l_expdata->cb_lbl3->setChecked(true);
					}
					else
					{
						l_expdata->cb_lbl3->setEnabled(false);
					}
				// Buffer data:
					str.sprintf("buf_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					buffer_flag = read_blob("BufFile", cur_t, filename);
					if(buffer_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Buffer Composition</a>\n");
						l_expdata->cb_lbl4->setChecked(true);
					}
					else
					{
						l_expdata->cb_lbl4->setEnabled(false);
					}
				}
			}
			f.close();						
		}
	}
	else
	{
		l_expdata->cb_lbl1->setEnabled(false);
		l_expdata->cb_lbl2->setEnabled(false);
		l_expdata->cb_lbl3->setEnabled(false);
		l_expdata->cb_lbl4->setEnabled(false);
		l_expdata->pb_display->setEnabled(false);
	}
}
void US_DB_RST_Cell::read_DB_vhw()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	
	int tableId;
	bool report_flag = false;
	bool extraPlot_flag = false;
	bool f_extraPlot_flag = false;
	bool distrPlot_flag = false;
	bool f_distrPlot_flag = false;
	bool hisPlot_flag = false;
	bool editedPlot_flag = false;	
						
	if(VHWID[0]>0)		//VHW result exists.
	{
		vhw_file = make_tempFile(dirName, "vhw.html");
		QFile f(vhw_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>van Holde - Weischet Analysis (from database):</b>\n   <p>\n   <ul>\n");

		QSqlCursor cur( "VelocVHW" );
		str.sprintf("VHWID = %d",VHWID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("VHWtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)	
			{
				QSqlCursor cur_t("VelocVHWTable");
				str.sprintf("VHWtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
			// vhw analysis report
					str.sprintf("vhw_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);
					if(report_flag)
					{
						ts << "      <li><a href=" << filename << ">Analysis Report</a>\n";
						l_vhw->cb_report->setChecked(true);
					}
					else
					{
						l_vhw->cb_report->setEnabled(false);
					}	
				// Extrapolation Plot Image:
					str.sprintf("vhw_ext_%d%d.png",i+1, j+1);
					filename = make_tempFile(dirName, str);
					extraPlot_flag = read_blob("ExtraPlot", cur_t, filename);
					if(extraPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Extrapolation Plot</a>\n");
						l_vhw->cb_lbl1->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl1->setEnabled(false);
					}
				// Extrapolation Plot Data:
					str.sprintf("vhw_ext.%d%d", i+1, j+1);
					filename = make_tempFile(dirName, str);
					f_extraPlot_flag = read_blob("F_ExtraPlot", cur_t, filename);
					if(f_extraPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">ASCII File of Extrapolation Plot Data</a>\n");
						l_vhw->cb_lbl2->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl2->setEnabled(false);
					}
				// Distribution Plot Image:
					str.sprintf("vhw_dis_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					distrPlot_flag = read_blob("DistrPlot", cur_t, filename);
					if(distrPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Distribution Plot (G(S))</a>\n");
						l_vhw->cb_lbl3->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl3->setEnabled(false);
					}
				// Distribution Plot Data:
					str.sprintf("vhw_dis.%d%d", i+1, j+1);
					filename = make_tempFile(dirName, str);
					f_distrPlot_flag = read_blob("F_DistrPlot", cur_t, filename);
					if(f_distrPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">ASCII File of Distribution Plot Data</a>\n");
						l_vhw->cb_lbl4->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl4->setEnabled(false);
					}
				// Histogram Plot Image:
					str.sprintf("vhw_his_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					hisPlot_flag = read_blob("HisPlot", cur_t, filename);
					if(hisPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">S-value Histogram Plot (G(S))</a>\n");
						l_vhw->cb_lbl5->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl5->setEnabled(false);
					}
				// Edited Data Plot Image:
					str.sprintf("vhw_edited_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					editedPlot_flag = read_blob("EditedPlot", cur_t, filename);
					if(editedPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Edited Data Plot</a>\n");
						l_vhw->cb_lbl6->setChecked(true);
					}
					else
					{
						l_vhw->cb_lbl6->setEnabled(false);
					}
				}
			}
			f.close();
		}
	}
	else			//vhw result data not exist
	{
		l_vhw->cb_report->setEnabled(false);
		l_vhw->cb_lbl1->setEnabled(false);
		l_vhw->cb_lbl2->setEnabled(false);
		l_vhw->cb_lbl3->setEnabled(false);
		l_vhw->cb_lbl4->setEnabled(false);
		l_vhw->cb_lbl5->setEnabled(false);
		l_vhw->cb_lbl6->setEnabled(false);
		l_vhw->pb_display->setEnabled(false);
	}
}

void US_DB_RST_Cell::read_DB_cs()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	
	int tableId;
	bool report_flag = false;
	bool distrPlot_flag = false;
	bool mw_distrPlot_flag = false;
	bool residPlot_flag = false;
	bool residPixmap_flag = false;
	bool f_distrPlot_flag = false;
	bool editedPlot_flag = false;	
	
	if(CSID[0]>0)		//CofS result exists.
	{
		cs_file = make_tempFile(dirName, "cs.html");
		QFile f(cs_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>C(s) Analysis (from database):</b>\n   <p>\n   <ul>\n");

		QSqlCursor cur( "VelocCS" );
		str.sprintf("CSID = %d",CSID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("CStableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)	
			{
				QSqlCursor cur_t("VelocCSTable");
				str.sprintf("CStableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
				// cs analysis report
					str.sprintf("cs_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);
					if(report_flag)
					{
						ts << "      <li><a href=" << filename << ">Analysis Report</a>\n";
						l_cs->cb_report->setChecked(true);
					}
					else
					{
						l_cs->cb_report->setEnabled(false);
					}	
				// Distribution Plot Image:
					str.sprintf("cs_dis_%d%d.png",i+1, j+1);
					filename = make_tempFile(dirName, str);
					distrPlot_flag = read_blob("DistrPlot", cur_t, filename);
					if(distrPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">C(s) Distribution Plot</a>\n");
						l_cs->cb_lbl1->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl1->setEnabled(false);
					}
				// Molecular Weight Distribution Plot Image:
					str.sprintf("cs_mw_dis_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					mw_distrPlot_flag = read_blob("MW_DistrPlot", cur_t, filename);
					if(mw_distrPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">C(s) Molecular Weight Distribution Plot</a>\n");
						l_cs->cb_lbl2->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl2->setEnabled(false);
					}
				// Residuals Plot Image:
					str.sprintf("cs_resid_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					residPlot_flag = read_blob("ResidPlot", cur_t, filename);
					if(residPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Residuals Plot</a>\n");
						l_cs->cb_lbl3->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl3->setEnabled(false);
					}
				// Residuals Pixmap Image:
					str.sprintf("cs_residpix_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					residPixmap_flag = read_blob("ResidPixmap", cur_t, filename);
					if(residPixmap_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Residuals Pixel Map</a>\n");
						l_cs->cb_lbl4->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl4->setEnabled(false);
					}

				// ASCII File of Distribution Plot Data:
					str.sprintf("cs_dis.%d%d", i+1, j+1);
					filename = make_tempFile(dirName, str);
					f_distrPlot_flag = read_blob("F_DistrPlot", cur_t, filename);
					if(f_distrPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">ASCII File of Distribution Plot Data</a>\n");
						l_cs->cb_lbl5->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl5->setEnabled(false);
					}
				// Edited Data Plot Image:
					str.sprintf("cs_edited_%d%d.png", i+1, j+1);
					filename = make_tempFile(dirName, str);
					editedPlot_flag = read_blob("EditedPlot", cur_t, filename);
					if(editedPlot_flag)
					{
						ts << "      <li><a href=" << filename << tr(">Edited Data Plot</a>\n");
						l_cs->cb_lbl6->setChecked(true);
					}
					else
					{
						l_cs->cb_lbl6->setEnabled(false);
					}
				}
			}
			f.close();
		}
	}
	else			//cs result data not exist
	{
		l_cs->cb_report->setEnabled(false);
		l_cs->cb_lbl1->setEnabled(false);
		l_cs->cb_lbl2->setEnabled(false);
		l_cs->cb_lbl3->setEnabled(false);
		l_cs->cb_lbl4->setEnabled(false);
		l_cs->cb_lbl5->setEnabled(false);
		l_cs->cb_lbl6->setEnabled(false);
		l_cs->pb_display->setEnabled(false);
	}
}

void US_DB_RST_Cell::read_DB_fe()
{
	QString str,str1,str2, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool report_flag = false;
	bool residuals_flag = false;
	bool overlays_flag = false;
	bool f_residuals_flag = false;
	bool f_raw_flag = false;
	bool f_solution_flag = false;	
			
	if(FEID[0]>0)		//FE result exists.
	{
		fe_file = make_tempFile(dirName, "fe.html");
		QFile f(fe_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>Finite Element Analysis (from database):</b>\n   <p>\n   <ul>\n");	
		QSqlCursor cur( "VelocFiniteElement" );
		str.sprintf("FEID = %d",FEID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("FEtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)		//Wavelength  exists.
			{
				QSqlCursor cur_t("VelocFiniteElementTable");
				str.sprintf("FEtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
				// Finite Element analysis report
					str.sprintf("fef_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);
					if(report_flag)
					{
						ts << "      <li><a href=" <<filename<< tr(">Analysis Report</a>\n");
						l_fe->cb_report->setChecked(true);
					}
					else
					{
						l_fe->cb_report->setEnabled(false);
					}
				// Residuals Plot Image:		
					filename = make_tempFile(dirName, "fe_residuals.tar");
					residuals_flag = read_blob("Residuals", cur_t, filename);								
					QProcess *r_proc;
					r_proc = new QProcess(this);
					r_proc->clearArguments();
					QDir work_dir;
					str = USglobal->config_list.html_dir + "/db_temp/";
					work_dir.setPath(str);
					r_proc->setWorkingDirectory(work_dir);
					QStringList r_cmd;
					r_cmd.append("tar");
					r_cmd.append("-xvvf");
					r_cmd.append("fe_residuals.tar");
					r_proc->setArguments(r_cmd);
					if(!r_proc->start())
					{
						QMessageBox::message("UltraScan Error:", "Unable to start process to uncompress residuals image files.");
						exit(0);
					}
					if(residuals_flag)
					{
						l_fe->cb_lbl1->setChecked(true);
						for(int r=0; r<scans; r+=5)
						{
							str1.sprintf("fef_%d%d_%d.png",i+1,j+1,r+1);
							str2.sprintf(tr("Residuals from Scan %d to Scan %d"), r+1, r+5);
							ts << "         <li><a href=" << str1 << ">" << str2 <<"</a>\n";
						}
					}
					else
					{
						l_fe->cb_lbl1->setEnabled(false);
					}
					
				// Overlays Plot Image:	
					filename = make_tempFile(dirName, "fe_overlays.tar");
					overlays_flag = read_blob("Overlays", cur_t, filename);	
					QProcess *o_proc;
					o_proc = new QProcess(this);
					o_proc->clearArguments();
					str = USglobal->config_list.html_dir + "/db_temp/";
					work_dir.setPath(str);
					o_proc->setWorkingDirectory(work_dir);
					QStringList o_cmd;
					o_cmd.append("tar");
					o_cmd.append("-xvvf");
					o_cmd.append("fe_overlays.tar");
					o_proc->setArguments(o_cmd);
					if(!o_proc->start())
					{
						QMessageBox::message("UltraScan Error:", "Unable to start process to uncompress overlays image files.");
						exit(0);
					}
					if(overlays_flag)
					{
						l_fe->cb_lbl2->setChecked(true);
						for(int r=0; r<scans; r+=5)
						{
							str1.sprintf("fef_edited_%d%d_%d.png",i+1,j+1,r+1);
							str2.sprintf(tr("Overlays (Scans %d - %d are highlighted)"), r+1, r+5);
							ts << "         <li><a href=" << str1 << ">" << str2 <<"</a>\n";
						}
					}
					else
					{
						l_fe->cb_lbl2->setEnabled(false);
					}
					
				// Residuals ASCII Data:
					str.sprintf("fef_residuals.%d%d", i+1, j+1);	
					filename = make_tempFile(dirName, str);
					f_residuals_flag = read_blob("F_Residuals", cur_t, filename);
					if(f_residuals_flag)	
					{
						l_fe->cb_lbl3->setChecked(true);
						ts << "      <li><a href=" << filename << tr(">ASCII File of Residuals</a>\n");
					}
					else
					{
						l_fe->cb_lbl3->setEnabled(false);
					}
				// Raw ASCII Data:
					str.sprintf("fef_raw.%d%d", i+1, j+1);	
					filename = make_tempFile(dirName, str);
					f_raw_flag = read_blob("F_Raw", cur_t, filename);
					if(f_raw_flag)
					{
						l_fe->cb_lbl4->setChecked(true);
						ts << "      <li><a href=" << filename << tr(">ASCII File of Experimental Data</a>\n");
					}
					else
					{
						l_fe->cb_lbl4->setEnabled(false);
					}	
					// Finite Element ASCII Data:
					str.sprintf("fef_dat.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					f_solution_flag = read_blob("F_Solution", cur_t, filename);	
					if(f_solution_flag)
					{
						l_fe->cb_lbl5->setChecked(true);
						ts << "      <li><a href=" << filename << tr(">ASCII File Finite Element Solution Data</a>\n");
					}
					else
					{
						l_fe->cb_lbl5->setEnabled(false);
					}	
				}
			}
			f.close();
		}
	}
	else			// NO Finite Element result
	{
		l_fe->cb_report->setEnabled(false);
		l_fe->cb_lbl1->setEnabled(false);
		l_fe->cb_lbl2->setEnabled(false);
		l_fe->cb_lbl3->setEnabled(false);
		l_fe->cb_lbl4->setEnabled(false);
		l_fe->cb_lbl5->setEnabled(false);
		l_fe->pb_display->setEnabled(false);
	}
}
void US_DB_RST_Cell::read_DB_sm()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool report_flag = false;
	bool plot_flag = false;
	bool f_plot_flag = false;
	bool editedPlot_flag = false;

	if(SMID[0]>0)		//SM result exists.
	{
		sm_file = make_tempFile(dirName, "sm.html");
		QFile f(sm_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>Scond Moment Analysis (from database):</b>\n   <p>\n   <ul>\n");
			
		QSqlCursor cur( "VelocSecondMoment" );
		str.sprintf("SMID = %d",SMID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("SMtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)		//Wavelength j exists.
			{
				QSqlCursor cur_t("VelocSecondMomentTable");
				str.sprintf("SMtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
				// Second Moment Anaysis Report:
					str.sprintf("sm_res.%d%d", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);	
					if(report_flag)
					{
						ts << "      <li><a href=" << filename << ">Analysis Report</a>\n";
						l_sm->cb_report->setChecked(true);
					}
					else
					{
						l_sm->cb_report->setEnabled(false);
					}	

				// Second Moment Plot Image:
					str.sprintf("sm_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					plot_flag = read_blob("SMPlot", cur_t, filename);
					if(plot_flag)
					{
						ts << "      <li><a href=" << filename << ">Second Moment Analysis Plot</a>\n";
						l_sm->cb_lbl1->setChecked(true);
					}
					else
					{
						l_sm->cb_lbl1->setEnabled(false);
					}		
				// Second Moment Plot Data:
					str.sprintf("sm_dat.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					f_plot_flag = read_blob("F_SMPlot", cur_t, filename);
					if(f_plot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Second Moment Plot Data</a>\n";
						l_sm->cb_lbl2->setChecked(true);
					}
					else
					{
						l_sm->cb_lbl2->setEnabled(false);
					}	
	
				// Second Moment Edited Data Plot Image:
					str.sprintf("sm_edited_%d%d.png", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					editedPlot_flag = read_blob("EditedPlot", cur_t, filename);	
					if(editedPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">Edited Data Plot</a>\n";
						l_sm->cb_lbl3->setChecked(true);
					}
					else
					{
						l_sm->cb_lbl3->setEnabled(false);
					}	
				}
			}
			f.close();
		}
	}
	else		//no second moment result
	{
		l_sm->cb_report->setEnabled(false);
		l_sm->cb_lbl1->setEnabled(false);
		l_sm->cb_lbl2->setEnabled(false);
		l_sm->cb_lbl3->setEnabled(false);
		l_sm->pb_display->setEnabled(false);
	}
}
void US_DB_RST_Cell::read_DB_td()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool report_flag = false;
	bool plot_flag = false;
	bool f_scanPlot_flag = false;
	bool f_svalPlot_flag = false;
	bool avgPlot_flag = false;
	bool f_avgPlot_flag = false;
	bool editedPlot_flag = false;	
	
	if(TDID[0]>0)		//Time Derivative result exists.
	{
		td_file = make_tempFile(dirName, "td.html");
		QFile f(td_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>Time Derivative Analysis (from database):</b>\n   <p>\n   <ul>\n");
			
		QSqlCursor cur( "VelocTimeDerivative" );
		str.sprintf("TDID = %d",TDID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("TDtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)						
			{	
				QSqlCursor cur_t("VelocTimeDerivativeTable");
				str.sprintf("TDtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
			// Time Derivative Anaysis Report:
					str.sprintf("dcdt_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);	
					if(report_flag)
					{
						ts << "      <li><a href=" << filename << ">Analysis Report</a>\n";
						l_td->cb_report->setChecked(true);
					}
					else
					{
						l_td->cb_report->setEnabled(false);
					}	
			// Time Derivative Plot Image:
					str.sprintf("dcdt_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					plot_flag = read_blob("TDPlot", cur_t, filename);
					if(plot_flag)
					{
						ts << "      <li><a href=" << filename << ">Time Derivative Analysis Plot</a>\n";
						l_td->cb_lbl1->setChecked(true);
					}
					else
					{
						l_td->cb_lbl1->setEnabled(false);
					}		
			// Time Derivative Scan Plot Data:
					str.sprintf("dcdt_scans.%d%d", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					f_scanPlot_flag = read_blob("F_ScanPlot", cur_t, filename);	
					if(f_scanPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Time Derivative Plot Data (Scans)</a>\n";
						l_td->cb_lbl2->setChecked(true);
					}
					else
					{
						l_td->cb_lbl2->setEnabled(false);
					}	
			// Time Derivative S-value Plot Data:
					str.sprintf("dcdt_sval.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					f_svalPlot_flag = read_blob("F_SvaluePlot", cur_t, filename);
					if(f_svalPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Time Derivative Plot Data (S-values)</a>\n";
						l_td->cb_lbl3->setChecked(true);
					}
					else
					{
						l_td->cb_lbl3->setEnabled(false);
					}		
			// Time Derivative Average Plot Image:
					str.sprintf("dcdt_avg_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					avgPlot_flag = read_blob("AvgPlot", cur_t, filename);	
					if(avgPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">Time Derivative Analysis Plot (Average g(S))</a>\n";
						l_td->cb_lbl4->setChecked(true);
					}
					else
					{
						l_td->cb_lbl4->setEnabled(false);
					}		
			// Time Derivative S-value Average Plot Data:
					str.sprintf("dcdt_avg.%d%d", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					f_avgPlot_flag = read_blob("F_AvgPlot", cur_t, filename);
					if(f_avgPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Time Derivative Plot Data (Average g(S))</a>\n";
						l_td->cb_lbl5->setChecked(true);
					}
					else
					{
						l_td->cb_lbl5->setEnabled(false);
					}			
			// Time Derivative Edited Data Plot Image:
					str.sprintf("dcdt_edited_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);	
					editedPlot_flag = read_blob("EditedPlot", cur_t, filename);
					if(editedPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">Edited Data Plot</a>\n";
						l_td->cb_lbl6->setChecked(true);
					}
					else
					{
						l_td->cb_lbl6->setEnabled(false);
					}		
				}
			}
			f.close();
		}
	}
	else   	//NO Time Derivative result exists.
	{
		l_td->cb_report->setEnabled(false);
		l_td->cb_lbl1->setEnabled(false);
		l_td->cb_lbl2->setEnabled(false);
		l_td->cb_lbl3->setEnabled(false);
		l_td->cb_lbl4->setEnabled(false);
		l_td->cb_lbl5->setEnabled(false);
		l_td->cb_lbl6->setEnabled(false);
		l_td->pb_display->setEnabled(false);
	}
}
void US_DB_RST_Cell::read_DB_rd()
{
	QString str, filename;
	QString dirName = USglobal->config_list.html_dir + "/db_temp/";
	int tableId;
	bool report_flag = false;
	bool plot_flag = false;
	bool f_scanPlot_flag = false;
	bool f_svalPlot_flag = false;
	bool avgPlot_flag = false;
	bool f_avgPlot_flag = false;
	bool editedPlot_flag = false;	
			
	if(RDID[0]>0)		//Radial Derivative result exists.
	{
		rd_file = make_tempFile(dirName, "rd.html");
		QFile f(rd_file);
		f.open(IO_WriteOnly | IO_Translate);
		QTextStream ts(&f);
		str.sprintf("<b>" + text + "</b>\n<p>\n");
		ts << str << tr("<ul>\n   <b>Radial Derivative Analysis (from database):</b>\n   <p>\n   <ul>\n");
			
		QSqlCursor cur( "VelocRadialDerivative" );
		str.sprintf("RDID = %d",RDID[0]);
		cur.select(str);
		if(cur.next())
		{
			int i = cell-1;
			int j = wl-1;
			str.sprintf("RDtableID_WL%d", j+1);
			tableId= cur.value(str).toInt();
			if(tableId>0)						
			{
				QSqlCursor cur_t("VelocRadialDerivativeTable");
				str.sprintf("RDtableID=%d", tableId);
				cur_t.select(str);
				if(cur_t.next())
				{
			// Radial Derivative Anaysis Report:
					str.sprintf("dcdr_res.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					report_flag = read_blob("Report", cur_t, filename);	
					if(report_flag)
					{
						ts << "      <li><a href=" << filename << ">Analysis Report</a>\n";
						l_rd->cb_report->setChecked(true);
					}
					else
					{
						l_rd->cb_report->setEnabled(false);
					}	
			// Radial Derivative Plot Image:
					str.sprintf("dcdr_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					plot_flag = read_blob("RDPlot", cur_t, filename);
					if(plot_flag)
					{
						ts << "      <li><a href=" << filename << ">Radial Derivative Analysis Plot</a>\n";
						l_rd->cb_lbl1->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl1->setEnabled(false);
					}		
				// Radial Derivative Scan Plot Data:
					str.sprintf("dcdr_scans.%d%d", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					f_scanPlot_flag = read_blob("F_ScanPlot", cur_t, filename);	
					if(f_scanPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Radial Derivative Plot Data (Scans)</a>\n";
						l_rd->cb_lbl2->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl2->setEnabled(false);
					}	
				// Radial Derivative S-value Plot Data:
					str.sprintf("dcdr_sval.%d%d", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					f_svalPlot_flag = read_blob("F_SvaluePlot", cur_t, filename);
					if(f_svalPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Radial Derivative Plot Data (S-values)</a>\n";
						l_rd->cb_lbl3->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl3->setEnabled(false);
					}		
				// Radial Derivative Average Plot Image:
					str.sprintf("dcdr_avg_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);
					avgPlot_flag = read_blob("AvgPlot", cur_t, filename);	
					if(avgPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">Radial Derivative Analysis Plot (Average g(S))</a>\n";
						l_rd->cb_lbl4->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl4->setEnabled(false);
					}		
				// Radial Derivative S-value Average Plot Data:
					str.sprintf("dcdr_avg.%d%d", i+1, j+1);		
					filename = make_tempFile(dirName, str);
					f_avgPlot_flag = read_blob("F_AvgPlot", cur_t, filename);
					if(f_avgPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">ASCII File of Radial Derivative Plot Data (Average g(S))</a>\n";
						l_rd->cb_lbl5->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl5->setEnabled(false);
					}			
				// Radial Derivative Edited Data Plot Image:
					str.sprintf("dcdr_edited_%d%d.png", i+1, j+1);			
					filename = make_tempFile(dirName, str);	
					editedPlot_flag = read_blob("EditedPlot", cur_t, filename);
					if(editedPlot_flag)
					{
						ts << "      <li><a href=" << filename << ">Edited Data Plot</a>\n";
						l_rd->cb_lbl6->setChecked(true);
					}
					else
					{
						l_rd->cb_lbl6->setEnabled(false);
					}		
				}
			}
			f.close();
		}
	}
	else   	//NO Radial Derivative result exists.
	{
		l_rd->cb_report->setEnabled(false);
		l_rd->cb_lbl1->setEnabled(false);
		l_rd->cb_lbl2->setEnabled(false);
		l_rd->cb_lbl3->setEnabled(false);
		l_rd->cb_lbl4->setEnabled(false);
		l_rd->cb_lbl5->setEnabled(false);
		l_rd->cb_lbl6->setEnabled(false);
		l_rd->pb_display->setEnabled(false);
	}
}


void US_DB_RST_Cell::clear()
{

}
/*****************************************************************************************************/
US_DB_RST_Cell_Layout::US_DB_RST_Cell_Layout(int temp_source_type, QWidget *parent, const char *name) : US_DB(parent, name)
{
	source_type = temp_source_type;
	int spacing=2;
	int xpos,ypos;
	int buttonw = 150, buttonh = 26;	
	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	
	xpos = spacing;
	ypos = spacing;
	lbl_DBID = new QLabel("", this);
	lbl_DBID->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_DBID->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_DBID->setGeometry(xpos, ypos, buttonw, buttonh);
	
	xpos += buttonw;
	lbl_dbid= new QLabel("",this);
	lbl_dbid->setPalette(QPalette(USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit, USglobal->global_colors.cg_edit));
	lbl_dbid->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_dbid->setGeometry(xpos, ypos, buttonw/2, buttonh);
	
	xpos += buttonw/2 + spacing;
	
	lbl_channel = new QLabel(tr(" Channel :"), this);
	lbl_channel->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_channel->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold));
	lbl_channel->setGeometry(xpos, ypos, buttonw*2/3, buttonh);
	
	xpos += buttonw*2/3;
	cmbb_channel = new QComboBox(false, this);
	cmbb_channel->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	cmbb_channel->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	cmbb_channel->setGeometry(xpos, ypos, buttonw*2/3, buttonh);

	xpos += buttonw*2/3;
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
	lbl_report = new QLabel(tr(" Analysis Report :"), this);
	lbl_report->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_report->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_report->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);

	xpos += buttonw*3;
	cb_report = new QCheckBox(this);
	cb_report->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_report->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label1 = new QLabel(tr(""), this);
	lbl_label1->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label1->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label1->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl1 = new QCheckBox(this);
	cb_lbl1->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl1->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label2 = new QLabel(tr(""), this);
	lbl_label2->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label2->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label2->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl2 = new QCheckBox(this);
	cb_lbl2->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl2->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label3 = new QLabel(tr(""), this);
	lbl_label3->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label3->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label3->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl3 = new QCheckBox(this);
	cb_lbl3->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl3->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label4 = new QLabel(tr(""), this);
	lbl_label4->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label4->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label4->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);
	
	xpos += buttonw*3;
	cb_lbl4 = new QCheckBox(this);
	cb_lbl4->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl4->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	

	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label5 = new QLabel(tr(""), this);
	lbl_label5->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label5->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label5->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);

	xpos += buttonw*3;
	cb_lbl5 = new QCheckBox(this);
	cb_lbl5->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl5->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
		
	xpos = spacing;
	ypos += buttonh + spacing;
	
	lbl_label6 = new QLabel(tr(""), this);
	lbl_label6->setPalette(QPalette(USglobal->global_colors.cg_label, USglobal->global_colors.cg_label, USglobal->global_colors.cg_label));
	lbl_label6->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1, QFont::Bold)); 
	lbl_label6->setGeometry(xpos, ypos, buttonw*4+spacing, buttonh);

	xpos += buttonw*3;
	cb_lbl6 = new QCheckBox(this);
	cb_lbl6->setGeometry(xpos + buttonw/3, ypos+5, 14, 14);
	cb_lbl6->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	
	
	xpos = spacing;
	ypos += buttonh + spacing;
	
	pb_display = new QPushButton("", this);
	pb_display->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_display->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_display->setGeometry(xpos+buttonw*3/2+buttonw*2/3, ypos, buttonw*2-buttonw/5, buttonh);
	connect(pb_display, SIGNAL(clicked()), SLOT(display()));

}

US_DB_RST_Cell_Layout::~US_DB_RST_Cell_Layout()
{
}

void US_DB_RST_Cell_Layout::get_dbid(QString tableName, int tableID)
{
	QString str;
	str = tableName + " ID :"; 
	lbl_DBID->setText(str);
	if(tableID == -1)
		lbl_dbid->setText("N/A");
	else
		lbl_dbid->setNum(tableID);
}

void US_DB_RST_Cell_Layout::get_channel(int number)
{
	if(number == 1)
	{
		cmbb_channel->insertItem("Channel 1");
	}
	if(number == 3)
	{
		cmbb_channel->insertItem("Channel 1");
		cmbb_channel->insertItem("Channel 2");
		cmbb_channel->insertItem("Channel 3");
	}
}
void US_DB_RST_Cell_Layout::get_label1(QString text)
{
	lbl_label1->setText(text);
}

void US_DB_RST_Cell_Layout::get_label2(QString text)
{
	lbl_label2->setText(text);
}

void US_DB_RST_Cell_Layout::get_label3(QString text)
{
	lbl_label3->setText(text);
}

void US_DB_RST_Cell_Layout::get_label4(QString text)
{
	lbl_label4->setText(text);
}

void US_DB_RST_Cell_Layout::get_label5(QString text)
{
	lbl_label5->setText(text);
}
void US_DB_RST_Cell_Layout::get_label6(QString text)
{
	lbl_label6->setText(text);
}

void US_DB_RST_Cell_Layout::get_display(QString text)
{
	QString str;	
	if(source_type == 0)
	{
		str = "Display  " + text + " from HD";
	}
	if(source_type == 1)
	{
		str = "Display  " + text + " from DB";
	}
	pb_display->setText(str);
	display_Str = text;
}

void US_DB_RST_Cell_Layout::display()
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
	if(display_Str == "Exp. Data")
	{
		display_file = USglobal->config_list.html_dir + dir +"expdata.html";
	}
	if(display_Str == "vH-W")
	{
		display_file = USglobal->config_list.html_dir + dir +"vhw.html";
	}
	if(display_Str == "C(s)")
	{
		display_file = USglobal->config_list.html_dir + dir +"cs.html";
	}
	if(display_Str == "Finite Element")
	{
		display_file = USglobal->config_list.html_dir + dir + "fe.html";
	}
	if(display_Str == "Second Moment")
	{
		display_file = USglobal->config_list.html_dir + dir +"sm.html";
	}
	if(display_Str == "dC/dt")
	{
		display_file = USglobal->config_list.html_dir + dir +"td.html";
	}
	if(display_Str == "dC/dr")
	{
		display_file = USglobal->config_list.html_dir + dir +"rd.html";
	}
	US_Help *online_help; 
	online_help = new US_Help(this);
	online_help->show_html_file(display_file);
}
