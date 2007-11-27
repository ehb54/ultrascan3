#include "../include/us_export.h"

US_Export_Veloc::US_Export_Veloc(QWidget *p, const char *name) : Data_Control_W(3, p, name)
{
	pm = new US_Pixmap();
	setup_GUI();
}


US_Export_Veloc::~US_Export_Veloc()
{
}

void US_Export_Veloc::setup_GUI()
{
	int j=0;
	int rows = 9, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addWidget(pb_load,j,0);
	subGrid1->addWidget(pb_details,j,1);
	j++;
	subGrid1->addWidget(pb_print,j,0);
	subGrid1->addWidget(pb_reset,j,1);
	j++;
	subGrid1->addWidget(pb_help,j,0);
	subGrid1->addWidget(pb_close,j,1);
	j++;
	subGrid1->addMultiCellWidget(banner1,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_run_id1,j,0);
	subGrid1->addWidget(lbl_run_id2,j,1);
	j++;
	subGrid1->addWidget(lbl_temperature1,j,0);
	subGrid1->addWidget(lbl_temperature2,j,1);
	j++;
	subGrid1->addWidget(lbl_cell_info1,j,0);
	subGrid1->addWidget(lbl_cell_info2,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,1);
	j++;
	subGrid1->addWidget(cell_select,j,0);
	subGrid1->addWidget(lambda_select,j,1);
	
	rows = 4, columns = 2, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}

	subGrid2->addWidget(smoothing_lbl,j,0);
	subGrid2->addWidget(smoothing_counter,j,1);
	j++;
	subGrid2->addWidget(range_lbl,j,0);
	subGrid2->addWidget(range_counter,j,1);
	j++;
	subGrid2->addWidget(position_lbl,j,0);
	subGrid2->addWidget(position_counter,j,1);
	j++;
	subGrid2->addWidget(pb_exsingle,j,0);
	subGrid2->addWidget(cnt_exsingle,j,1);
	j++;
	subGrid2->addWidget(pb_exrange,j,0);
	subGrid2->addWidget(cnt_exrange,j,1);

	background->addLayout(subGrid1,0,0);
	background->addWidget(edit_plot,0,1);
	background->addLayout(subGrid2,1,0);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,750);
	
	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+6+750, r.height()+6);

}


void US_Export_Veloc::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	QString filestr, filename, temp;
	unsigned int i, j;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d%d", selected_cell + 1, selected_lambda + 1, selected_channel + 1);
	filestr = filename.copy();
	filestr.append(".raw");
	filestr.append(temp);
	QFile raw_f(filestr);
	
	if (raw_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&raw_f);
		ts << run_inf.scans[selected_cell][selected_lambda] << "\t" << points << "\t";
		for (j=0; j<(unsigned int)run_inf.scans[selected_cell][selected_lambda]-1; j++)
		{
			ts << (run_inf.time[selected_cell][selected_lambda][j]) << "\t";
		}
		ts << (run_inf.time[selected_cell][selected_lambda][run_inf.scans[selected_cell][selected_lambda]-1]) << "\n";
		for (i=0; i<points; i++)
		{
			ts << radius[i] << "\t";
			for (j=0; j<(unsigned int)run_inf.scans[selected_cell][selected_lambda]-1; j++)
			{
				ts << absorbance[j][i] << "\t";
			}
			ts << absorbance[run_inf.scans[selected_cell][selected_lambda]-1][i] << "\n";
		}
		raw_f.close();
	}
	qApp->processEvents();
	QPixmap p;
	QString fileName;
	fileName.sprintf(htmlDir + "/raw_%d%d.", selected_cell + 1, selected_lambda + 1);
	p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
	pm->save_file(fileName, p);
}

void US_Export_Veloc::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/export.html");
}

US_Export_Equil::US_Export_Equil(QWidget *p, const char *name) : Data_Control_W(4, p, name)
{
	pm = new US_Pixmap();
	setup_GUI();
}

US_Export_Equil::~US_Export_Equil()
{
}

void US_Export_Equil::setup_GUI()
{
	int j=0;
	int rows = 13, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,1,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_reset,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_close,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(banner1,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_run_id1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_run_id2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_temperature1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_temperature2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_info1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_cell_info2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,3);
	j++;
	QGridLayout *listboxes = new QGridLayout(3,3,2);
	listboxes->addWidget(cell_select,0,0);
	listboxes->addWidget(lambda_select,0,1);
	listboxes->addWidget(channel_select,0,2);

	subGrid1->addMultiCell(listboxes,j,j,0,3);
	
	j++;	
	subGrid1->addMultiCellWidget(smoothing_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(smoothing_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(range_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(range_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(position_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(position_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_exsingle,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exsingle,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_exrange,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exrange,j,j,2,3);
			
			
	background->addLayout(subGrid1,0,0);
	background->addWidget(edit_plot,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,480);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	this->setGeometry(global_Xpos, global_Ypos, r.width()+480+spacing*3, r.height()-100);
}

void US_Export_Equil::save()
{
	if (step == 0)
	{
		QMessageBox::message(tr("Attention:\n"),tr("You need to load some data first!\n\n	   Click on \"Load Data\"\n"));
		return;
	}
	QString filestr, filename, temp;
	unsigned int i, j;
	filename = USglobal->config_list.result_dir.copy();
	filename.append("/");
	filename.append(run_inf.run_id);
	temp.sprintf(".%d%d%d", selected_cell+1, selected_lambda+1, selected_channel+1);
	filestr = filename.copy();
	filestr.append(".raw");
	filestr.append(temp);
	QFile raw_f(filestr);
	if (raw_f.open(IO_WriteOnly | IO_Translate))
	{
		QTextStream ts(&raw_f);
		for (i=0; i<points; i++)
		{
			ts << radius[i] << "\t";
			for (j=0; j<(unsigned int) run_inf.scans[selected_cell][selected_lambda]-1; j++)
			{
				ts << absorbance[j][i] << "\t";
			}
			ts << absorbance[run_inf.scans[selected_cell][selected_lambda]-1][i] << "\n";
		}
		raw_f.close();
	}
	qApp->processEvents();
	QPixmap p;
	QString fileName;
	fileName.sprintf(htmlDir + "/raw_%d%d%d.", selected_cell + 1, selected_lambda + 1, selected_channel + 1);
	p = QPixmap::grabWidget(edit_plot, 2, 2, edit_plot->width() - 4, edit_plot->height() - 4);
	pm->save_file(fileName, p);
}

void US_Export_Equil::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/export.html");
}

US_Print_Veloc::US_Print_Veloc(QWidget *p, const char *name) : Data_Control_W(5, p, name)
{
	setup_GUI();
}

US_Print_Veloc::~US_Print_Veloc()
{
}

void US_Print_Veloc::setup_GUI()
{
	int j=0;
	int rows = 9, columns = 2, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,2,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addWidget(pb_load,j,0);
	subGrid1->addWidget(pb_details,j,1);
	j++;
	subGrid1->addWidget(pb_print,j,0);
	subGrid1->addWidget(pb_reset,j,1);
	j++;
	subGrid1->addWidget(pb_help,j,0);
	subGrid1->addWidget(pb_close,j,1);
	j++;
	subGrid1->addMultiCellWidget(banner1,j,j,0,1);
	j++;
	subGrid1->addWidget(lbl_run_id1,j,0);
	subGrid1->addWidget(lbl_run_id2,j,1);
	j++;
	subGrid1->addWidget(lbl_temperature1,j,0);
	subGrid1->addWidget(lbl_temperature2,j,1);
	j++;
	subGrid1->addWidget(lbl_cell_info1,j,0);
	subGrid1->addWidget(lbl_cell_info2,j,1);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,1);
	j++;
	subGrid1->addWidget(cell_select,j,0);
	subGrid1->addWidget(lambda_select,j,1);
	
	rows = 4, columns = 2, spacing = 2, j=0;
	QGridLayout * subGrid2 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid2->setRowSpacing(i, 26);
	}

	subGrid2->addWidget(smoothing_lbl,j,0);
	subGrid2->addWidget(smoothing_counter,j,1);
	j++;
	subGrid2->addWidget(range_lbl,j,0);
	subGrid2->addWidget(range_counter,j,1);
	j++;
	subGrid2->addWidget(position_lbl,j,0);
	subGrid2->addWidget(position_counter,j,1);
	j++;
	subGrid2->addWidget(pb_exsingle,j,0);
	subGrid2->addWidget(cnt_exsingle,j,1);
	j++;
	subGrid2->addWidget(pb_exrange,j,0);
	subGrid2->addWidget(cnt_exrange,j,1);

	background->addLayout(subGrid1,0,0);
	background->addWidget(edit_plot,0,1);
	background->addLayout(subGrid2,1,0);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,750);
	
	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);

	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+6+750, r.height()+6);

}

void US_Print_Veloc::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/print.html");
}

US_Print_Equil::US_Print_Equil(QWidget *p, const char *name) : Data_Control_W(6, p, name)
{
	setup_GUI();
}

US_Print_Equil::~US_Print_Equil()
{
}

void US_Print_Equil::setup_GUI()
{
	int j=0;
	int rows = 14, columns = 4, spacing = 2;
	
	QGridLayout * background = new QGridLayout(this,1,2,spacing);	
	QGridLayout * subGrid1 = new QGridLayout(rows, columns, spacing);
	for (int i=0; i<rows; i++)
	{
		subGrid1->setRowSpacing(i, 26);
	}
	subGrid1->addMultiCellWidget(pb_load,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_details,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_print,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_reset,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_help,j,j,0,1);
	subGrid1->addMultiCellWidget(pb_close,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(banner1,j,j,0,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_run_id1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_run_id2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_temperature1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_temperature2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_info1,j,j,0,1);
	subGrid1->addMultiCellWidget(lbl_cell_info2,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(lbl_cell_descr,j,j,0,3);
	j++;
	QGridLayout *listboxes = new QGridLayout(3,3,2);
	listboxes->addWidget(cell_select,0,0);
	listboxes->addWidget(lambda_select,0,1);
	listboxes->addWidget(channel_select,0,2);

	subGrid1->addMultiCell(listboxes,j,j,0,3);
	
	j++;	
	subGrid1->addMultiCellWidget(smoothing_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(smoothing_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(range_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(range_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(position_lbl,j,j,0,1);
	subGrid1->addMultiCellWidget(position_counter,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_exsingle,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exsingle,j,j,2,3);
	j++;
	subGrid1->addMultiCellWidget(pb_exrange,j,j,0,1);
	subGrid1->addMultiCellWidget(cnt_exrange,j,j,2,3);
			
			
	background->addLayout(subGrid1,0,0);
	background->addWidget(edit_plot,0,1);
	background->setColStretch(0,1);
	background->setColStretch(1,4);
	background->setColSpacing(0,350);
	background->setColSpacing(1,480);

	qApp->processEvents();
	QRect r = background->cellGeometry(0, 0);
	
	global_Xpos += 30;
	global_Ypos += 30;
	
	this->setGeometry(global_Xpos, global_Ypos, r.width()+480+spacing*3, r.height()-100);

}

void US_Print_Equil::help()
{
	US_Help *online_help; online_help = new US_Help(this);
	online_help->show_help("manual/print.html");
}

