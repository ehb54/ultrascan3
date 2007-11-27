#include "../include/us_showdetails.h"

US_ShowDetails::US_ShowDetails(vector <QString> *temp_filenames, int *temp_dataset_selected, QWidget *p, const char *name) : QDialog( p, name, true)
{
	buttonh = 26;
	border = 4;
	int width = 300;
	unsigned int i;
	xpos = border;
	ypos = border;
	USglobal = new US_Config();

	setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	setCaption(tr("Equilibrium Run Details"));
	
	dataset_selected = temp_dataset_selected;
	*dataset_selected = 0;
	filenames = temp_filenames;
	
	lbl_info = new QLabel(tr("Double-click on desired Dataset:"), this);
	Q_CHECK_PTR(lbl_info);
	lbl_info->setFrameStyle(QFrame::WinPanel|QFrame::Raised);
	lbl_info->setAlignment(AlignCenter|AlignVCenter);
	lbl_info->setPalette(QPalette(USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame, USglobal->global_colors.cg_frame));
	lbl_info->setGeometry(xpos, ypos, width, buttonh);
	lbl_info->setFont(QFont(USglobal->config_list.fontFamily, USglobal->config_list.fontSize, QFont::Bold));
	
	ypos += buttonh + spacing;
	
	lb_datasets = new QListBox(this, "Datasets");
	lb_datasets->setGeometry(xpos, ypos, width, 4 * buttonh);
	lb_datasets->setSelected(0, true);
	lb_datasets->setPalette( QPalette(USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal, USglobal->global_colors.cg_normal));
	lb_datasets->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize - 1));
	for (i=0; i<(*filenames).size(); i++)
	{
		lb_datasets->insertItem((*filenames)[i]);
	}
	lb_datasets->setSelected(0, true);
	lb_datasets->setCurrentItem(0);
	connect(lb_datasets, SIGNAL(selected(int)), SLOT(show_dataset(int)));
	
	ypos += 4 * buttonh + spacing;
	xpos = border;

	pb_cancel = new QPushButton(tr("Cancel"), this);
	Q_CHECK_PTR(pb_cancel);
	pb_cancel->setAutoDefault(false);
	pb_cancel->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_cancel->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_cancel->setGeometry(xpos, ypos, 149, buttonh);
	connect(pb_cancel, SIGNAL(clicked()), SLOT(cancel()));

	xpos += 151;

	pb_show = new QPushButton(tr("Show Details"), this);
	Q_CHECK_PTR(pb_show);
	pb_show->setAutoDefault(false);
	pb_show->setFont(QFont( USglobal->config_list.fontFamily, USglobal->config_list.fontSize));
	pb_show->setPalette( QPalette(USglobal->global_colors.cg_pushb, USglobal->global_colors.cg_pushb_disabled, USglobal->global_colors.cg_pushb_active));
	pb_show->setGeometry(xpos, ypos, 149, buttonh);
	connect(pb_show, SIGNAL(clicked()), SLOT(show_dataset()));

	ypos += buttonh + border;

	global_Xpos += 30;
	global_Ypos += 30;
	
	setMinimumSize(width+8, ypos);
	setMaximumSize(width+8, ypos);
	setGeometry(global_Xpos, global_Ypos, width+8, ypos);
}

US_ShowDetails::~US_ShowDetails()
{
}

void US_ShowDetails::closeEvent(QCloseEvent *e)
{
	e->accept();
	global_Xpos -= 30;
	global_Ypos -= 30;
}

void US_ShowDetails::cancel()
{
	*dataset_selected = -1;
	reject();
}

void US_ShowDetails::show_dataset()
{
	int item;
	item = lb_datasets->currentItem();
	show_dataset(item); 
}

void US_ShowDetails::show_dataset(int item)
{
	*dataset_selected = item;
	accept();
}
