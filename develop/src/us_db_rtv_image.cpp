#include "../include/us_db_rtv_image.h"

//! Constructor
/*! 
	Constractor a new <var>US_DB_RtvImage</var> interface, 
	with <var>p</var> as a parent and <var>us_rtvimage</var> as object name. 
*/ 
US_DB_RtvImage::US_DB_RtvImage(QWidget *p, const char *name) : US_DB_RtvInvestigator( p, name)
{
	ImageID=0;
	QString str =tr("Show Investigator Submitted Images\nfrom DB:");
	str.append(login_list.dbname);
	lbl_blank->setText(str);
	
	lb_name->disconnect();
	connect(lb_name, SIGNAL(highlighted(int)), SLOT(select_name(int)));
	connect(lb_name, SIGNAL(selected(int)), SLOT(check_image(int)));

	pb_chkID->disconnect();
	connect(pb_chkID, SIGNAL(clicked()), SLOT(checkImage()));
	lb_data->disconnect();
	connect(lb_data, SIGNAL(highlighted(int)), SLOT(select_image(int)));
	connect(lb_data, SIGNAL(selected(int)), SLOT(select_image(int)));

	pb_retrieve->disconnect();
	pb_retrieve->setText("Show Image");
	connect(pb_retrieve, SIGNAL(clicked()), SLOT(show_image()));
}

//! Destructor
/*! destroy the US_DB_RtvImage. */
US_DB_RtvImage::~US_DB_RtvImage()
{
}

void US_DB_RtvImage::check_image(int item)
{
	select_name(item);
	checkImage();
}

void US_DB_RtvImage::checkImage()
{
	QString str, *item_description;
	int maxID = get_newID("tblImage", "GelID");
	int count = 0;
	item_ImageID = new int[maxID];
	item_description = new QString[maxID];
	display_Str = new QString[maxID];

	if(check_ID)
	{
		str.sprintf("SELECT GelID, Description FROM tblImage WHERE InvestigatorID = %d;", InvID);
		QSqlQuery query(str);
		if(query.isActive())
		{
			while(query.next())
			{
				item_ImageID[count] = query.value(0).toInt();
				item_description[count] = query.value(1).toString();
				display_Str[count] = "(" + QString::number( item_ImageID[count] ) + "), "+item_description[count];
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


void US_DB_RtvImage::select_image(int item)
{
	QString str;
	if(sel_data)
	{
		ImageID = item_ImageID[item];
		Display = display_Str[item];		
		str.sprintf("Retrieve: "+ Display);
		lbl_item->setText(str);
		retrieve_flag = true;			
	}
	else
	{
		QMessageBox::message(tr("Attention:"), 
									tr("No Image Data has been selected\n"));
		return;
	}
}

void US_DB_RtvImage::show_image()
{
	QString str;
	if(!retrieve_flag)
	{
		QMessageBox::message(tr("Attention:"), 
									tr("You have to select a dataset first!\n"));
		return;
	}

	
	QSqlCursor cur( "tblImage" );
	str.sprintf("GelID = %d",ImageID);
	cur.select(str);
	if(cur.next())
	{
		int size = cur.value("GelPicture").toByteArray().size();
		QByteArray da(size);
		da = cur.value("GelPicture").toByteArray();
		QString filename=USglobal->config_list.result_dir + "/temp_image.jpg";
		QFile f(filename);
		if (!f.open(IO_WriteOnly))
		{
			QMessageBox::message(tr("Warning:"), 
									tr("Problem to open file to write!\n"));
			return;
		}
		f.writeBlock(da.data(),size);
		f.close();
		view_image(filename);
		f.remove();
	}
	else
	{
		QMessageBox::message(tr("Warning:"), 
									tr("Problem to query tblImage!\n"));
		return;
	}			
}
